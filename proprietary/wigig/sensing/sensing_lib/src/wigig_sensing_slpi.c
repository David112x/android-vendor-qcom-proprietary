/******************************************************************************
  @file    wigig_sensing_slpi.c
  @brief   The WiGig Sensing Library SLPI Flow C File.

  DESCRIPTION
  Implementation of APIs used for SLPI flow in sensing lib

  ---------------------------------------------------------------------------
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include "wigig_sensing_slpi.h"

extern struct wigig_sensing_lib sensing_lib;
static sns_std_suid suid_lookup = sns_suid_sensor_init_default;
sns_std_suid wigig_sensing_suid = (sns_std_suid) { .suid_low = 0, .suid_high = 0 };

uint32_t wigig_sensing_get_curr_req()
{
    return sensing_lib.current_req;
}

void check_and_signal
(
    bool match_curr_req
)
{
    pthread_mutex_lock(&sensing_lib.async_response_mutex);

    /* only signal if event matches current request and if current request is nonzero,
       if current request is 0 it means there is no wait thus shouldn't signal */
    if (match_curr_req && sensing_lib.current_req) {
        sensing_lib.async_response_wait_flag = 1;
        pthread_cond_signal(&sensing_lib.async_response_cond_var);
    }

    pthread_mutex_unlock(&sensing_lib.async_response_mutex);
}

void wigig_sensing_set_fw_ready
(
    bool ready
)
{
    if (!sensing_lib.is_fw_ready)
        check_and_signal(true);
    sensing_lib.is_fw_ready = ready;
}

void wigig_sensing_process_generic_event
(
    uint32_t error,
    bool match_curr_req
)
{
    sensing_lib.last_error = error;

    check_and_signal(match_curr_req);
}

void wigig_sensing_process_get_params_resp
(
    uint32_t   error,
    const void *params_buf,
    uint32_t   params_buf_len,
    bool match_curr_req
)
{
    sensing_lib.last_error = error;
    if (params_buf_len > sensing_lib.params_buf.buf_len) {
        syslog(LOG_ERR, "%s: copy error given length %d is smaller than received %d\n", __FUNCTION__, sensing_lib.params_buf.buf_len, params_buf_len);
    }
    else {
        sensing_lib.params_buf.recv_len = params_buf_len;
        memcpy(sensing_lib.params_buf.buf, params_buf, params_buf_len);
    }

    check_and_signal(match_curr_req);
}

void wigig_sensing_process_debug_resp
(
    uint32_t   error,
    uint32_t   req_id,
    const void *debug_buf,
    uint32_t   debug_buf_len,
    bool match_curr_req
)
{
    sensing_lib.last_error = error;
    sensing_lib.debug_buf.req_id = req_id;
    if (debug_buf_len > sensing_lib.debug_buf.buf_len) {
        syslog(LOG_ERR, "%s: copy error given length %d is smaller than received %d\n", __FUNCTION__, sensing_lib.debug_buf.buf_len, debug_buf_len);
    }
    else {
        sensing_lib.debug_buf.recv_len = debug_buf_len;
        memcpy(sensing_lib.debug_buf.buf, debug_buf, debug_buf_len);
    }

    check_and_signal(match_curr_req);
}

void wigig_sensing_process_get_mode_resp
(
    sns_wigig_get_mode_resp *resp_msg,
    bool match_curr_req
)
{
    sensing_lib.current_mode = (uint32_t)resp_msg->mode;
    syslog(LOG_INFO, "Got mode from SLPI: %d\n", sensing_lib.current_mode);

    check_and_signal(match_curr_req);
}

void wigig_sensing_process_burst_size_event
(
    sns_wigig_burst_size_event *event_msg,
    bool match_curr_req
)
{
    sensing_lib.burst_size = (uint32_t)event_msg->burst_size;
    syslog(LOG_INFO, "Burst size is %d\n", sensing_lib.burst_size);

    check_and_signal(match_curr_req);
}

void wigig_sensing_process_data_ready_event
(
    uint64_t timestamp,
    bool mf,
    uint32_t data_len,
    uint8_t *data
)
{
    pthread_mutex_lock(&sensing_lib.async_response_mutex);

    /* TODO: this is copying twice, to burst buffer then to real buffer.
       Maybe change cyclic buffer to an array of a struct that includes burst buffer and mf flag? */
    wigig_sensing_process_single_burst(timestamp, data_len, data);
    if (!mf) {
        wigig_sensing_process_data_ready();
        if (!sensing_lib.data_response_wait_flag) {
            sensing_lib.data_response_wait_flag = 1;
            pthread_cond_signal(&sensing_lib.data_response_cond_var);
        }
    }

    pthread_mutex_unlock(&sensing_lib.async_response_mutex);
}

static qmi_client_error_type
create_client_conn
(
    qmi_client_type *qmi_handle,
    uint32_t        timeout
);

/**
 * QMI Error callback.  This would be our notification if the QMI connection
 * was lost.  A typical implementation would re-establish the connection, and
 * resend any active requests.
 *
 * See qmi_client_error_cb. */
static void error_cb
(
    qmi_client_type         user_handle,
    qmi_client_error_type   error,
    void                    *err_cb_data
)
{
    syslog(LOG_DEBUG, "QMI error callback %i\n", error);
    if (error == QMI_SERVICE_ERR)
        create_client_conn(&sensing_lib.qmi_user_handle, sensing_lib.cmd_timeout_ms);
}

static void handle_event_msg
(
    sns_std_suid const *suid,
    void const *event_msg,
    size_t event_msg_len
)
{
    pb_istream_t stream;
    sns_client_event_msg event = sns_client_event_msg_init_default;

    syslog(LOG_DEBUG, "%s: Processing events from SUID %"PRIx64" %"PRIx64, __FUNCTION__,
        suid->suid_low, suid->suid_high);

    if (suid->suid_high == suid_lookup.suid_high && suid->suid_low == suid_lookup.suid_low) {
        event.events.funcs.decode = &wigig_sensing_decode_events;
        event.events.arg = &wigig_sensing_decode_suid_event;
    }
    else {
        wigig_sensing_suid = *suid;

        event.events.funcs.decode = &wigig_sensing_decode_events;
        event.events.arg = &wigig_sensing_decode_sensing_events;
    }

    stream = pb_istream_from_buffer(event_msg, event_msg_len);
    if (!pb_decode(&stream, sns_client_event_msg_fields, &event)) {
        syslog(LOG_ERR, "%s: Error decoding event list: %s", __FUNCTION__, PB_GET_ERROR(&stream));
    }
}

/**
 * Handle an incoming QMI response message from the Sensors Service.
 */
static void resp_msg_cb
(
    qmi_client_type user_handle,
    unsigned int msg_id,
    void *resp_c_struct,
    unsigned int resp_c_struct_len,
    void *resp_cb_data,
    qmi_client_error_type transp_err
)
{
    syslog(LOG_DEBUG, "%s: Received response: error %i, msg_id %i, size %i",
        __FUNCTION__, transp_err, msg_id, resp_c_struct_len);

    if (QMI_NO_ERR == transp_err) {
        sns_client_resp_msg_v01 *resp = (sns_client_resp_msg_v01*)resp_c_struct;

        syslog(LOG_DEBUG, "%s: Received client ID %" PRId64 "result %i",
            __FUNCTION__, resp->client_id, resp->result);
    }

    free(resp_c_struct);
}

/* QMI indication callback.  See qmi_client_ind_cb. */
static void ind_msg_cb
(
    qmi_client_type user_handle,
    unsigned int msg_id,
    void *ind_buf,
    unsigned int ind_buf_len,
    void *ind_cb_data)
{
    size_t ind_len = sizeof(sns_client_report_ind_msg_v01);
    sns_client_report_ind_msg_v01 *ind = malloc(ind_len);
    if (!ind) {
        syslog(LOG_ERR, "%s: failed to allocate sns_client_report_ind_msg_v01", __FUNCTION__);
        return;
    }

    int32_t qmi_err;

    syslog(LOG_DEBUG, "%s: Received Indication; len %i", __FUNCTION__, ind_buf_len);

    // Extract the Protocol Buffer encoded message from the outer QMI/IDL message
    qmi_err = qmi_idl_message_decode(SNS_CLIENT_SVC_get_service_object_v01(),
        QMI_IDL_INDICATION, msg_id, ind_buf, ind_buf_len, ind, ind_len);
    if (QMI_IDL_LIB_NO_ERR != qmi_err) {
        syslog(LOG_ERR, "%s: QMI decode error %i", __FUNCTION__, qmi_err);
        free(ind);
        return;
    }

    sns_client_event_msg event = sns_client_event_msg_init_default;
    syslog(LOG_DEBUG, "%s: Indication from client ID %"PRIu64, __FUNCTION__, ind->client_id);

    // Decode just the sns_client_event_msg in order to get the SUID
    pb_istream_t stream = pb_istream_from_buffer(ind->payload, ind->payload_len);

    if (pb_decode(&stream, sns_client_event_msg_fields, &event)) {
        handle_event_msg(&event.suid, ind->payload, ind->payload_len);
    }
    else {
        syslog(LOG_ERR, "%s: Error decoding Event Message: %s", __FUNCTION__, PB_GET_ERROR(&stream));
    }

    free(ind);
}

/**
 * Create a QMI connection to QWG
 *
 * @param[i] qmi_handle QMI Handle created in wait_for_service
 * @param[i] timeout Time-out in milliseconds. 0 = no timeout
 *
 * @return True on success; false otherwise
 */
static wigig_sensing_lib_error_type create_client_conn
(
    qmi_client_type *qmi_handle,
    uint32_t        timeout
)
{
    qmi_idl_service_object_type service_obj = SNS_CLIENT_SVC_get_service_object_v01();
    qmi_client_error_type qmi_err;
    qmi_cci_os_signal_type os_params;

    syslog(LOG_INFO, "Creating client connection\n");

    if (!qmi_handle) {
        syslog(LOG_ERR, "qmi_handle invalid\n");
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }
    if (!service_obj) {
        syslog(LOG_ERR, "service_obj invalid\n");
        return WIGIG_SENSING_LIB_INTERNAL_ERR;
    }

    qmi_err = qmi_client_init_instance(service_obj,
        QMI_CLIENT_INSTANCE_ANY, ind_msg_cb, NULL, &os_params, timeout, qmi_handle);

    if (QMI_NO_ERR != qmi_err) {
        syslog(LOG_ERR, "qmi_client_init_instance error %i\n", qmi_err);
        return WIGIG_SENSING_LIB_SERVICE_ERR;
    }

    qmi_err = qmi_client_register_error_cb(*qmi_handle, error_cb, NULL);

    if (QMI_NO_ERR != qmi_err) {
        syslog(LOG_ERR, "qmi_client_register_error_cb error %i\n", qmi_err);
        return WIGIG_SENSING_LIB_SERVICE_ERR;
    }

    return qmi_err;
}

static qmi_client_error_type send_async_sensing_request
(
    qmi_client_type                 qmi_handle,
    uint32_t                        msg_id,
    sns_std_suid                    suid,
    void                            *encoded_req,
    size_t                          encoded_req_len
)
{
    sns_buffer_arg payload = (sns_buffer_arg)
    {
        .buf = encoded_req, .buf_len = encoded_req_len
    };
    sns_client_request_msg pb_req_msg = sns_client_request_msg_init_default;
    sns_client_req_msg_v01 req_msg;
    pb_ostream_t stream = pb_ostream_from_buffer(req_msg.payload, SNS_CLIENT_REQ_LEN_MAX_V01);
    qmi_client_error_type qmi_err;

    pb_req_msg.suid = suid;
    pb_req_msg.msg_id = msg_id;
    pb_req_msg.request.has_batching = false;
    pb_req_msg.request.payload.funcs.encode = &wigig_sensing_encode_payload;
    pb_req_msg.request.payload.arg = (void*)&payload;

    syslog(LOG_INFO, "%s: sending msg_id %d", __FUNCTION__, msg_id);

    if (!pb_encode(&stream, sns_client_request_msg_fields, &pb_req_msg)) {
        syslog(LOG_ERR, "%s: Error Encoding request: %s", __FUNCTION__, PB_GET_ERROR(&stream));
        return QMI_INTERNAL_ERR;
    }

    req_msg.payload_len = stream.bytes_written;
    void *resp = malloc(sizeof(sns_client_resp_msg_v01));
    if (!resp) {
        syslog(LOG_ERR, "%s: Error allocating sns_client_resp_msg_v01", __FUNCTION__);
        return QMI_INTERNAL_ERR;
    }
    qmi_txn_handle txn_handle;

    qmi_err = qmi_client_send_msg_async(
        qmi_handle, SNS_CLIENT_REQ_V01, (void*)&req_msg, sizeof(req_msg),
        resp, sizeof(sns_client_resp_msg_v01), resp_msg_cb, &resp, &txn_handle);

    if (QMI_NO_ERR != qmi_err) {
        syslog(LOG_ERR, "%s: qmi_client_send_msg_async error %i", __FUNCTION__, qmi_err);
        free(resp);
        return QMI_INTERNAL_ERR;
    }
    syslog(LOG_INFO, "%s: qmi_client_send_msg_async successfully %i", __FUNCTION__, qmi_err);

    return qmi_err;
}

static struct timespec get_time_to_wait(int timeout_ms)
{
    struct timespec timeToWait = { 0 };
    clock_gettime(CLOCK_REALTIME, &timeToWait);
    timeToWait.tv_nsec += timeToWait.tv_nsec + timeout_ms * 1000UL * 1000UL; /* convert to nanosec  */
    timeToWait.tv_sec += timeToWait.tv_nsec / 1000000000UL;
    timeToWait.tv_nsec = timeToWait.tv_nsec % 1000000000UL;

    return timeToWait;
}

static wigig_sensing_lib_error_type wait_for_event_slpi()
{
    struct timespec timeToWait = get_time_to_wait(sensing_lib.cmd_timeout_ms);

    while (!sensing_lib.async_response_wait_flag) {
        int timeoutRes = pthread_cond_timedwait(&sensing_lib.async_response_cond_var, &sensing_lib.async_response_mutex, &timeToWait);
        if (timeoutRes == ETIMEDOUT) {
            syslog(LOG_ERR, "Timed out waiting for resp \n");
            sensing_lib.current_req = 0;
            return WIGIG_SENSING_LIB_TIMEOUT_ERR;
        }
        else if (timeoutRes) {
            syslog(LOG_ERR, "Enable mutex error = %d \n", timeoutRes);
            sensing_lib.current_req = 0;
            return WIGIG_SENSING_LIB_MUTEX_ERR;
        }
    }

    return WIGIG_SENSING_LIB_NO_ERR;
}

static int wigig_sensing_data_response_mutex_init()
{
    int rc;
    rc = pthread_cond_init(&sensing_lib.data_response_cond_var, NULL);
    if (rc) {
        syslog(LOG_ERR, "data_response_cond_var could not be created, %d\n", rc);
        return WIGIG_SENSING_LIB_MUTEX_ERR;
    }

    rc = pthread_mutex_init(&sensing_lib.data_response_mutex, NULL);
    if (rc) {
        syslog(LOG_ERR, "data_response_mutex could not be created, %d\n", rc);
        pthread_cond_destroy(&sensing_lib.data_response_cond_var);
        return WIGIG_SENSING_LIB_MUTEX_ERR;
    }

    /* initialize to 1 and only to be cleared when a read_data is called */
    sensing_lib.data_response_wait_flag = 1;
    return WIGIG_SENSING_LIB_NO_ERR;
}

static void wigig_sensing_data_response_mutex_uninit()
{
    int rc = pthread_cond_destroy(&sensing_lib.data_response_cond_var);
    if (rc)
        syslog(LOG_ERR, "data_response_cond_var could not be destroyed, %d\n", rc);

    rc = pthread_mutex_destroy(&sensing_lib.data_response_mutex);
    if (rc)
        syslog(LOG_ERR, "data_response_mutex could not be destroyed, %d\n", rc);
}


static wigig_sensing_lib_error_type wait_for_data_slpi()
{
    pthread_mutex_lock(&sensing_lib.data_response_mutex);
    sensing_lib.data_response_wait_flag = 0;

    struct timespec timeToWait = get_time_to_wait(sensing_lib.cmd_timeout_ms);

    while (!sensing_lib.data_response_wait_flag) {
        int timeoutRes = pthread_cond_timedwait(&sensing_lib.data_response_cond_var, &sensing_lib.data_response_mutex, &timeToWait);
        if (timeoutRes == ETIMEDOUT) {
            syslog(LOG_ERR, "Timed out waiting for data \n");
            pthread_mutex_unlock(&sensing_lib.data_response_mutex);
            return WIGIG_SENSING_LIB_TIMEOUT_ERR;
        }
        else if (timeoutRes) {
            syslog(LOG_ERR, "Enable mutex error = %d \n", timeoutRes);
            pthread_mutex_unlock(&sensing_lib.data_response_mutex);
            return WIGIG_SENSING_LIB_MUTEX_ERR;
        }
    }

    pthread_mutex_unlock(&sensing_lib.data_response_mutex);
    return WIGIG_SENSING_LIB_NO_ERR;
}

/**
 * Send an encoded protobuf inside a QMI request to QMI server
 * then wait for the response.
 * Frees encoded_req after sending.
 *
 * @param[i] msg_id Protobuf request id
 * @param[i] suid Target sensor id
 * @param[i] encoded_req Encoded protobuf request
 * @param[i] encoded_req_len Encoded protobuf request length
 * @return wigig_sensing_lib_error_type
 */
static wigig_sensing_lib_error_type send_request_wait
(
    uint32_t                        msg_id,
    sns_std_suid                    suid,
    void                            *encoded_req,
    size_t                          encoded_req_len
)
{
    if (!encoded_req)
        return WIGIG_SENSING_LIB_PARAM_ERR;

    sensing_lib.last_error = 0;
    sensing_lib.current_req = msg_id;
    qmi_client_error_type qmi_err = send_async_sensing_request(sensing_lib.qmi_user_handle, msg_id, suid, encoded_req, encoded_req_len);
    sensing_lib.qmi_error = qmi_err;
    free(encoded_req);
    if (qmi_err) {
        sensing_lib.current_req = 0;
        return WIGIG_SENSING_LIB_SERVICE_ERR;
    }

    wigig_sensing_lib_error_type rc = wait_for_event_slpi();
    if (rc)
        return rc;

    sensing_lib.current_req = 0;

    if (sensing_lib.last_error) {
        syslog(LOG_ERR, "%s failed, sensing error code %d\n", __FUNCTION__, sensing_lib.last_error);
        return WIGIG_SENSING_LIB_INTERNAL_ERR;
    }

    return WIGIG_SENSING_LIB_NO_ERR;
}

static wigig_sensing_lib_error_type send_stop_mode_slpi()
{
    if (sensing_lib.current_mode == WIGIG_SENSING_LIB_STOP_MODE) {
        syslog(LOG_INFO, "Already in stop mode\n");
        return WIGIG_SENSING_LIB_NO_ERR;
    }

    /* encoded_req allocation in wigig_sensing_get_encoded_change_mode_req() and
       free in send_request_wait() */
    void *encoded_req = NULL;
    size_t encoded_req_len;

    pthread_mutex_lock(&sensing_lib.async_response_mutex);
    sensing_lib.async_response_wait_flag = 0;

    encoded_req_len = wigig_sensing_get_encoded_change_mode_req(&encoded_req, SNS_WIGIG_MODE_STOP, 0);

    if (!encoded_req) {
        pthread_mutex_unlock(&sensing_lib.async_response_mutex);
        return WIGIG_SENSING_LIB_PARAM_ERR;
    }

    wigig_sensing_lib_error_type send_err = send_request_wait(SNS_WIGIG_MSGID_SNS_WIGIG_CHANGE_MODE_REQ, wigig_sensing_suid, encoded_req, encoded_req_len);
    sensing_lib.last_error = send_err;
    if (send_err) {
        pthread_mutex_unlock(&sensing_lib.async_response_mutex);
        return send_err;
    }

    sensing_lib.current_mode = WIGIG_SENSING_LIB_STOP_MODE;
    wigig_sensing_release_buffers();

    if (sensing_lib.last_error) {
        syslog(LOG_ERR, "%s failed, sensing error code %d\n", __FUNCTION__, sensing_lib.last_error);
        pthread_mutex_unlock(&sensing_lib.async_response_mutex);
        return WIGIG_SENSING_LIB_INTERNAL_ERR;
    }

    pthread_mutex_unlock(&sensing_lib.async_response_mutex);

    return WIGIG_SENSING_LIB_NO_ERR;
}

/**
 * Send a request to the SUID Lookup Sensor for the complete list of SUIDs
 *
 * @param[i] qmi_handle QMI connecton created in create_client_conn
 * @param[i] data_type Sensor type to search for
 *
 * @return true if request was sent successfully, false otherwise
 */
static wigig_sensing_lib_error_type send_suid_req
(
    qmi_client_type qmi_handle,
    char const *data_type
)
{
    /* encoded_req allocation in wigig_sensing_get_encoded_suid_req() and
       free in send_request_wait() */
    void *encoded_req = NULL;
    size_t encoded_req_len;

    pthread_mutex_lock(&sensing_lib.async_response_mutex);
    sensing_lib.async_response_wait_flag = 0;

    encoded_req_len = wigig_sensing_get_encoded_suid_req(data_type, &encoded_req);

    wigig_sensing_lib_error_type res = send_request_wait(SNS_SUID_MSGID_SNS_SUID_REQ, suid_lookup, encoded_req, encoded_req_len);

    pthread_mutex_unlock(&sensing_lib.async_response_mutex);

    return res;
}

void wigig_sensing_lib_unregister_slpi()
{
    int mres;

    wigig_sensing_lib_error_type stop_res = send_stop_mode_slpi();
    if (stop_res)
        syslog(LOG_ERR, "%s: change to stop failed = %d, continue to unregister..\n", __FUNCTION__, stop_res);

    mres = qmi_client_release(sensing_lib.qmi_user_handle);
    if (mres)
        syslog(LOG_ERR, "qmi_client_release failed, %d\n", mres);

    mres = pthread_cond_destroy(&sensing_lib.async_response_cond_var);
    if (mres)
        syslog(LOG_ERR, "async_response_cond_var could not be destroyed, %d\n", mres);

    mres = pthread_mutex_destroy(&sensing_lib.async_response_mutex);
    if (mres)
        syslog(LOG_ERR, "async_response_mutex could not be destroyed, %d\n", mres);

    mres = wigig_sensing_data_mutex_uninit(&sensing_lib.async_response_mutex);
    if (mres)
        syslog(LOG_ERR, "data_mutex could not be destroyed, %d\n", mres);

    wigig_sensing_data_response_mutex_uninit();
}

wigig_sensing_lib_error_type
wigig_sensing_lib_change_mode_slpi
(
    wigig_sensing_lib_mode_type     mode,
    wigig_sensing_channel           channel,
    uint32_t                        *burst_size
)
{
    if (mode == WIGIG_SENSING_LIB_STOP_MODE) {
        wigig_sensing_lib_error_type stop_res = send_stop_mode_slpi();
        *burst_size = 0;
        syslog(LOG_INFO, "change_mode to stop complete status = %d\n", stop_res);
        return stop_res;
    }

    pthread_mutex_lock(&sensing_lib.async_response_mutex);
    sensing_lib.async_response_wait_flag = 0;

    /* Release previous buffers before changing to nonstop mode */
    wigig_sensing_release_buffers();

    /* encoded_req allocation in wigig_sensing_get_encoded_change_mode_req() and
       free in send_request_wait() */
    void *encoded_req = NULL;
    size_t encoded_req_len;

    encoded_req_len = wigig_sensing_get_encoded_change_mode_req(&encoded_req, (sns_wigig_mode)mode, channel);

    wigig_sensing_lib_error_type res = send_request_wait(SNS_WIGIG_MSGID_SNS_WIGIG_CHANGE_MODE_REQ, wigig_sensing_suid, encoded_req, encoded_req_len);
    if (res) {
        syslog(LOG_ERR, "%s: error sending req, %d\n", __FUNCTION__, res);
        pthread_mutex_unlock(&sensing_lib.async_response_mutex);
        return res;
    }

    syslog(LOG_INFO, "got burst size in main thread %d\n", sensing_lib.burst_size);

    int allocate_err = allocate_cyclic_buffer(sensing_lib.burst_size);
    if (allocate_err) {
        syslog(LOG_ERR, "%s: Cyclic buffer allocation error = %d\n", __FUNCTION__, allocate_err);
        sensing_lib.burst_size = 0;
        *burst_size = 0;
        pthread_mutex_unlock(&sensing_lib.async_response_mutex);
        return allocate_err;
    }

    *burst_size = sensing_lib.burst_size;
    sensing_lib.current_mode = mode;

    syslog(LOG_INFO, "%s succeeded\n", __FUNCTION__);

    pthread_mutex_unlock(&sensing_lib.async_response_mutex);

    return WIGIG_SENSING_LIB_NO_ERR;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_get_parameters_slpi
(
    void                            *params_buf,
    uint32_t                        params_buf_len,
    uint32_t                        *params_recv_len
)
{
    /* encoded_req allocation in wigig_sensing_get_encoded_get_params_req() and
       free in send_request_wait() */
    void *encoded_req = NULL;
    size_t encoded_req_len;

    pthread_mutex_lock(&sensing_lib.async_response_mutex);
    sensing_lib.async_response_wait_flag = 0;

    sensing_lib.params_buf.buf = params_buf;
    sensing_lib.params_buf.buf_len = params_buf_len;
    sensing_lib.params_buf.recv_len = 0;

    encoded_req_len = wigig_sensing_get_encoded_get_params_req(&encoded_req);

    wigig_sensing_lib_error_type res = send_request_wait(SNS_WIGIG_MSGID_SNS_WIGIG_GET_PARAMS_REQ, wigig_sensing_suid, encoded_req, encoded_req_len);

    pthread_mutex_unlock(&sensing_lib.async_response_mutex);

    if (res) {
        syslog(LOG_ERR, "%s: error sending req, %d\n", __FUNCTION__, res);
        sensing_lib.params_buf.buf = NULL;
        return res;
    }

    syslog(LOG_INFO, "%s succeeded\n", __FUNCTION__);

    *params_recv_len = sensing_lib.params_buf.recv_len;
    sensing_lib.params_buf.buf = NULL;

    return WIGIG_SENSING_LIB_NO_ERR;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_send_debug_msg_slpi
(
    uint32_t                        req_id,
    bool                            data_valid,
    void                            *req_buf,
    uint32_t                        req_buf_len,
    void                            *resp_buf,
    uint32_t                        resp_buf_len,
    uint32_t                        *resp_buf_recv_len,
    uint32_t                        timeout_msecs
)
{
    /* encoded_req allocation in wigig_sensing_get_encoded_debug_req() and
       free in send_request_wait() */
    void *encoded_req = NULL;
    size_t encoded_req_len;

    pthread_mutex_lock(&sensing_lib.async_response_mutex);
    sensing_lib.async_response_wait_flag = 0;

    sensing_lib.debug_buf.buf = resp_buf;
    sensing_lib.debug_buf.buf_len = resp_buf_len;
    sensing_lib.debug_buf.recv_len = 0;

    encoded_req_len = wigig_sensing_get_encoded_debug_req(&encoded_req, req_id, req_buf, req_buf_len);

    wigig_sensing_lib_error_type res = send_request_wait(SNS_WIGIG_MSGID_SNS_WIGIG_DEBUG_REQ, wigig_sensing_suid, encoded_req, encoded_req_len);

    pthread_mutex_unlock(&sensing_lib.async_response_mutex);

    if (res) {
        syslog(LOG_ERR, "%s: error sending req, %d\n", __FUNCTION__, res);
        sensing_lib.debug_buf.buf = NULL;
        return res;
    }

    syslog(LOG_INFO, "%s succeeded\n", __FUNCTION__);

    *resp_buf_recv_len = sensing_lib.debug_buf.recv_len;
    sensing_lib.debug_buf.buf = NULL;

    return WIGIG_SENSING_LIB_NO_ERR;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_read_data_slpi
(
    uint32_t                        *data_buf,
    uint32_t                        data_buf_len,
    uint32_t                        *data_buf_recv_len,
    uint64_t                        *dri_tsf_arr,
    uint32_t                        dri_tsf_arr_size,
    uint32_t                        *num_of_bursts
)
{
    wigig_sensing_lib_error_type rc = WIGIG_SENSING_LIB_NO_ERR;

    while (!wigig_sensing_get_buffer_size()) {
        rc = wait_for_data_slpi();
        if (rc) {
            syslog(LOG_ERR, "%s: error waiting for data = %d", __FUNCTION__, rc);
            return rc;
        }
    }

    rc = wigig_sensing_process_read(data_buf, data_buf_len, data_buf_recv_len, dri_tsf_arr, dri_tsf_arr_size,
        num_of_bursts);
    if (rc) {
        syslog(LOG_ERR, "%s: error reading data", __FUNCTION__);
        return rc;
    }

    return WIGIG_SENSING_LIB_NO_ERR;
}


wigig_sensing_lib_error_type
wigig_sensing_lib_get_mode_slpi
(
    wigig_sensing_lib_mode_type     *mode
)
{
    /* encoded_req allocation in wigig_sensing_get_encoded_get_mode_req() and
    free in send_request_wait() */
    void *encoded_req = NULL;
    size_t encoded_req_len;

    pthread_mutex_lock(&sensing_lib.async_response_mutex);
    sensing_lib.async_response_wait_flag = 0;

    encoded_req_len = wigig_sensing_get_encoded_get_mode_req(&encoded_req);

    wigig_sensing_lib_error_type res = send_request_wait(SNS_WIGIG_MSGID_SNS_WIGIG_GET_MODE_REQ, wigig_sensing_suid, encoded_req, encoded_req_len);

    pthread_mutex_unlock(&sensing_lib.async_response_mutex);

    if (res) {
        syslog(LOG_ERR, "%s: error sending req, %d\n", __FUNCTION__, res);
        return res;
    }

    syslog(LOG_INFO, "%s succeeded\n", __FUNCTION__);

    *mode = sensing_lib.current_mode;

    return WIGIG_SENSING_LIB_NO_ERR;
}


wigig_sensing_lib_error_type
wigig_sensing_lib_register_slpi
(
    bool                            auto_recovery
)
{
    wigig_sensing_lib_error_type register_err = create_client_conn(&sensing_lib.qmi_user_handle, sensing_lib.cmd_timeout_ms);
    if (register_err) {
        syslog(LOG_ERR, "%s: could not register %d\n", __FUNCTION__, register_err);
        goto fail;
    }

    register_err = send_suid_req(sensing_lib.qmi_user_handle, "radar");
    if (register_err) {
        syslog(LOG_ERR, "%s: error sending suid req, %d\n", __FUNCTION__, register_err);
        goto fail;
    }

    int mres;
    mres = pthread_cond_init(&sensing_lib.async_response_cond_var, NULL);
    if (mres) {
        syslog(LOG_ERR, "async_response_cond_var could not be created, %d\n", mres);
        register_err = WIGIG_SENSING_LIB_MUTEX_ERR;
        goto fail;
    }

    mres = pthread_mutex_init(&sensing_lib.async_response_mutex, NULL);
    if (mres) {
        syslog(LOG_ERR, "async_response_mutex could not be created, %d\n", mres);
        register_err = WIGIG_SENSING_LIB_MUTEX_ERR;
        goto destroy_async_cond;
    }

    mres = wigig_sensing_data_response_mutex_init();
    if (mres) {
        register_err = WIGIG_SENSING_LIB_MUTEX_ERR;
        goto destroy_async_mutex;
    }

    mres = wigig_sensing_data_mutex_init();
    if (mres) {
        syslog(LOG_ERR, "data_mutex could not be created, %d\n", mres);
        register_err = WIGIG_SENSING_LIB_MUTEX_ERR;
        goto destroy_all_mutex;
    }

    /* encoded_req allocation in wigig_sensing_get_encoded_auto_recovery_req() and
       free in send_request_wait() */
    if (auto_recovery) {
        void *encoded_req = NULL;
        size_t encoded_req_len;

        pthread_mutex_lock(&sensing_lib.async_response_mutex);
        sensing_lib.async_response_wait_flag = 0;

        encoded_req_len = wigig_sensing_get_encoded_auto_recovery_req(&encoded_req);
        wigig_sensing_lib_error_type res = send_request_wait(SNS_WIGIG_MSGID_SNS_WIGIG_AUTO_RECOVERY_REQ, wigig_sensing_suid, encoded_req, encoded_req_len);

        pthread_mutex_unlock(&sensing_lib.async_response_mutex);

        if (res)
            syslog(LOG_ERR, "%s: error sending auto recovery req, %d\n", __FUNCTION__, res);
    }

    return WIGIG_SENSING_LIB_NO_ERR;

destroy_all_mutex:
    wigig_sensing_data_response_mutex_uninit();
destroy_async_mutex:
    pthread_mutex_destroy(&sensing_lib.async_response_mutex);
destroy_async_cond:
    pthread_cond_destroy(&sensing_lib.async_response_cond_var);
fail:
    if (NULL != sensing_lib.qmi_user_handle) {
        qmi_client_release(sensing_lib.qmi_user_handle);
        sensing_lib.qmi_user_handle = NULL;
    }
    return register_err;
}

wigig_sensing_lib_error_type
wigig_sensing_lib_read_stats_slpi
(
    uint32_t                        *dropped_cnt_from_last_read,
    uint32_t                        *remaining_bursts
)
{
    return wigig_sensing_process_stats(dropped_cnt_from_last_read, remaining_bursts);
}

wigig_sensing_lib_error_type
wigig_sensing_lib_cancel_read_slpi()
{
    if (sensing_lib.data_response_wait_flag) {
        pthread_mutex_lock(&sensing_lib.async_response_mutex);
        sensing_lib.data_response_wait_flag = false;
        sensing_lib.async_response_wait_flag = 1;
        pthread_cond_signal(&sensing_lib.async_response_cond_var);
        pthread_mutex_unlock(&sensing_lib.async_response_mutex);
    }

    return WIGIG_SENSING_LIB_NO_ERR;
}