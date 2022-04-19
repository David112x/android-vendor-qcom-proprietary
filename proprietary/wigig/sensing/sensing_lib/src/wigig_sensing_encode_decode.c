/******************************************************************************
  @file    wigig_sensing_encode_decode.c
  @brief   The WiGig Sensing QMI Library Protobuf Handling C file.

  DESCRIPTION
  Encode and decode wigig sensing protobufs

  ---------------------------------------------------------------------------
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
*******************************************************************************/

#include "wigig_sensing_encode_decode.h"
#include "wigig_sensing_data.h"

/**
 * Copy an already encoded payload into a message field.  For the purpose of
 * this function, an encoded submessage is identical to a character string.
 */
bool wigig_sensing_encode_payload
(
    pb_ostream_t *stream,
    const pb_field_t *field,
    void *const *arg
)
{
    sns_buffer_arg *info = (sns_buffer_arg*)*arg;

    if (!pb_encode_tag_for_field(stream, field))
        return false;
    if (!pb_encode_string(stream, info->buf, info->buf_len))
        return false;

    return true;
}

/**
 * Create a SUID lookup request for the specified data type.
 * Allocates memory for the encoded message encoded_req.
 * encoded_req needs to be free later after usage.
 *
 * @param[i] data_type Sensor type to search for
 * @param[o] encoded_req Generated encoded request
 *
 * @return Length of encoded_req
 */
size_t wigig_sensing_get_encoded_suid_req
(
    char const *data_type,
    void **encoded_req
)
{
    size_t encoded_req_size;
    sns_suid_req suid_req = sns_suid_req_init_default;

    *encoded_req = NULL;

    suid_req.data_type.funcs.encode = &wigig_sensing_encode_payload;
    suid_req.data_type.arg = &((sns_buffer_arg)
    {
        .buf = data_type, .buf_len = strlen(data_type)
    });
    suid_req.has_register_updates = true;
    suid_req.register_updates = true;

    if (!pb_get_encoded_size(&encoded_req_size, sns_suid_req_fields, &suid_req)) {
        syslog(LOG_ERR, "%s: pb_get_encoded_size error", __FUNCTION__);
    }
    else {
        void *encoded_buffer = malloc(encoded_req_size);
        if (!encoded_buffer) {
            syslog(LOG_ERR, "%s: failed to allocate encoded_buffer", __FUNCTION__);
            return 0;
        }

        pb_ostream_t stream = pb_ostream_from_buffer(encoded_buffer, encoded_req_size);

        if (!pb_encode(&stream, sns_suid_req_fields, &suid_req))
            syslog(LOG_ERR, "%s: Error Encoding suid request: %s", __FUNCTION__, PB_GET_ERROR(&stream));
        else
            *encoded_req = encoded_buffer;
    }

    return NULL == *encoded_req ? 0 : encoded_req_size;
}

/**
 * Create a set recovery request.
 * Allocates memory for the encoded message encoded_req.
 * encoded_req needs to be free later after usage.
 *
 * @param[o] encoded_req Generated encoded request
 *
 * @return Length of encoded_req
 */
size_t wigig_sensing_get_encoded_auto_recovery_req
(
    void **encoded_req
)
{
    size_t encoded_req_size;
    sns_wigig_auto_recovery_req recovery_req = sns_wigig_auto_recovery_req_init_default;

    *encoded_req = NULL;

    if (!pb_get_encoded_size(&encoded_req_size, sns_wigig_auto_recovery_req_fields, &recovery_req)) {
        syslog(LOG_ERR, "%s: pb_get_encoded_size error", __FUNCTION__);
    }
    else {
        void *encoded_buffer = malloc(encoded_req_size);
        if (!encoded_buffer) {
            syslog(LOG_ERR, "%s: failed to allocate encoded_buffer", __FUNCTION__);
            return 0;
        }

        pb_ostream_t stream = pb_ostream_from_buffer(encoded_buffer, encoded_req_size);

        if (!pb_encode(&stream, sns_wigig_auto_recovery_req_fields, &recovery_req))
            syslog(LOG_ERR, "%s: Error Encoding set recovery request: %s", __FUNCTION__, PB_GET_ERROR(&stream));
        else
            *encoded_req = encoded_buffer;
    }

    return NULL == *encoded_req ? 0 : encoded_req_size;
}

/**
 * Create a change mode request.
 * Allocates memory for the encoded message encoded_req.
 * encoded_req needs to be free later after usage.
 *
 * @param[o] encoded_req Generated encoded request
 * @param[i] mode Sensing mode
 * @param[i] channel Sensing channel
 *
 * @return Length of encoded_req
 */
size_t wigig_sensing_get_encoded_change_mode_req
(
    void **encoded_req,
    sns_wigig_mode mode,
    uint32_t channel
)
{
    size_t encoded_req_size;
    sns_wigig_change_mode_req change_req = sns_wigig_change_mode_req_init_default;
    change_req.mode = mode;

    if (mode != SNS_WIGIG_MODE_STOP) {
        change_req.has_channel = true;
        change_req.channel = channel;
    }

    *encoded_req = NULL;

    if (!pb_get_encoded_size(&encoded_req_size, sns_wigig_change_mode_req_fields, &change_req)) {
        syslog(LOG_ERR, "%s: pb_get_encoded_size error", __FUNCTION__);
    }
    else {
        void *encoded_buffer = malloc(encoded_req_size);
        if (!encoded_buffer) {
            syslog(LOG_ERR, "%s: failed to allocate encoded_buffer", __FUNCTION__);
            return 0;
        }

        pb_ostream_t stream = pb_ostream_from_buffer(encoded_buffer, encoded_req_size);

        if (!pb_encode(&stream, sns_wigig_change_mode_req_fields, &change_req))
            syslog(LOG_ERR, "%s: Error Encoding change mode request: %s", __FUNCTION__, PB_GET_ERROR(&stream));
        else
            *encoded_req = encoded_buffer;
    }

    return NULL == *encoded_req ? 0 : encoded_req_size;
}

/**
 * Create a get parameters request.
 * Allocates memory for the encoded message encoded_req.
 * encoded_req needs to be free later after usage.
 *
 * @param[o] encoded_req Generated encoded request
 *
 * @return Length of encoded_req
 */
size_t wigig_sensing_get_encoded_get_params_req
(
    void **encoded_req
)
{
    size_t encoded_req_size;
    sns_wigig_get_params_req params_req = sns_wigig_get_params_req_init_default;

    *encoded_req = NULL;

    if (!pb_get_encoded_size(&encoded_req_size, sns_wigig_get_params_req_fields, &params_req)) {
        syslog(LOG_ERR, "%s: pb_get_encoded_size error", __FUNCTION__);
    }
    else {
        void *encoded_buffer = malloc(encoded_req_size);
        if (!encoded_buffer) {
            syslog(LOG_ERR, "%s: failed to allocate encoded_buffer", __FUNCTION__);
            return 0;
        }

        pb_ostream_t stream = pb_ostream_from_buffer(encoded_buffer, encoded_req_size);

        if (!pb_encode(&stream, sns_wigig_get_params_req_fields, &params_req))
            syslog(LOG_ERR, "%s: Error Encoding get params request: %s", __FUNCTION__, PB_GET_ERROR(&stream));
        else
            *encoded_req = encoded_buffer;
    }

    return NULL == *encoded_req ? 0 : encoded_req_size;
}

/**
 * Create a debug request.
 * Allocates memory for the encoded message encoded_req.
 * encoded_req needs to be free later after usage.
 *
 * @param[o] encoded_req Generated encoded request
 * @param[i] req_id Debug request id
 * @param[i] req_buf Debug request buffer
 * @param[i] buf_len Debug request buffer length
 *
 * @return Length of encoded_req
 */
size_t wigig_sensing_get_encoded_debug_req
(
    void **encoded_req,
    uint32_t req_id,
    void *req_buf,
    uint32_t buf_len
)
{
    size_t encoded_req_size;
    sns_wigig_debug_req debug_req = sns_wigig_debug_req_init_default;
    debug_req.req_id = req_id;

    /* only when debug response has a payload */
    if (buf_len) {
        debug_req.data.funcs.encode = &wigig_sensing_encode_payload;
        debug_req.data.arg = &((sns_buffer_arg)
        {
            .buf = req_buf, .buf_len = buf_len
        });
    }

    *encoded_req = NULL;

    if (!pb_get_encoded_size(&encoded_req_size, sns_wigig_debug_req_fields, &debug_req)) {
        syslog(LOG_ERR, "%s: pb_get_encoded_size error", __FUNCTION__);
    }
    else {
        void *encoded_buffer = malloc(encoded_req_size);
        if (!encoded_buffer) {
            syslog(LOG_ERR, "%s: failed to allocate encoded_buffer", __FUNCTION__);
            return 0;
        }

        pb_ostream_t stream = pb_ostream_from_buffer(encoded_buffer, encoded_req_size);

        if (!pb_encode(&stream, sns_wigig_debug_req_fields, &debug_req))
            syslog(LOG_ERR, "%s: Error Encoding debug request: %s", __FUNCTION__, PB_GET_ERROR(&stream));
        else
            *encoded_req = encoded_buffer;
    }

    return NULL == *encoded_req ? 0 : encoded_req_size;
}

/**
 * Create a get mode request.
 * Allocates memory for the encoded message encoded_req.
 * encoded_req needs to be free later after usage.
 *
 * @param[o] encoded_req Generated encoded request
 *
 * @return Length of encoded_req
 */
size_t wigig_sensing_get_encoded_get_mode_req
(
    void **encoded_req
)
{
    size_t encoded_req_size = 0;
    sns_wigig_get_mode_req mode_req = sns_wigig_get_mode_req_init_default;

    *encoded_req = NULL;

    if (!pb_get_encoded_size(&encoded_req_size, sns_wigig_get_mode_req_fields, &mode_req)) {
        syslog(LOG_ERR, "%s: pb_get_encoded_size error", __FUNCTION__);
    }
    else {
        void *encoded_buffer = malloc(encoded_req_size);
        if (!encoded_buffer) {
            syslog(LOG_ERR, "%s: failed to allocate encoded_buffer", __FUNCTION__);
            return 0;
        }

        pb_ostream_t stream = pb_ostream_from_buffer(encoded_buffer, encoded_req_size);

        if (!pb_encode(&stream, sns_wigig_get_mode_req_fields, &mode_req))
            syslog(LOG_ERR, "%s: Error Encoding get mode request: %s", __FUNCTION__, PB_GET_ERROR(&stream));
        else
            *encoded_req = encoded_buffer;
    }

    return NULL == *encoded_req ? 0 : encoded_req_size;
}

/**
 * Get the message ID of the encoded event residing on stream.
 *
 * @param[i] stream Protobuf stream
 * @return Message ID
 */
static uint32_t get_msg_id
(
    pb_istream_t *stream
)
{
    sns_client_event_msg_sns_client_event event =
        sns_client_event_msg_sns_client_event_init_default;

    if (!pb_decode(stream, sns_client_event_msg_sns_client_event_fields, &event)) {
        syslog(LOG_ERR, "%s: event: %s", __FUNCTION__, PB_GET_ERROR(stream));
    }
    else {
        syslog(LOG_DEBUG, "%s: Decoding event with message ID %i, timestamp %"PRIu64, __FUNCTION__,
            event.msg_id, event.timestamp);
        return event.msg_id;
    }

    /* 0 for invalid msg id: wigig events are >512 and framework events are >128 */
    return 0;
}

/*
 * Decode an element of sns_client_event_msg::events.  This function will be
 * called by nanopb once per each element in the array.
 *
 * @param[i] stream Protobuf stream
 * @param[i] field Protobuf fields to decode
 * @param[i] arg Sensor-specific decode function
 */
bool wigig_sensing_decode_events
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg
)
{
    bool rv = true;
    sns_client_event_msg_sns_client_event event =
        sns_client_event_msg_sns_client_event_init_default;
    pb_istream_t stream_cpy = *stream;

    uint32_t msg_id = get_msg_id(&stream_cpy);

    /* "arg" points to wigig_sensing_decode_sensing_events() that determines the specific wigig event */
    event.payload.funcs.decode = *arg;
    event.payload.arg = &msg_id;

    if (!pb_decode(stream, sns_client_event_msg_sns_client_event_fields, &event)) {
        syslog(LOG_ERR, "%s: Error decoding Event: %s", __FUNCTION__, PB_GET_ERROR(stream));
        rv = false;
    }

    return rv;
}

/**
 * Decode a string of bytes from a field. This function is intended to be
 * used as a callback function during the decode process.
 *
 * @see pb_callback_s::decode
 */
static bool decode_payload
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg
)
{
    sns_buffer_arg *data = (sns_buffer_arg*)*arg;

    data->buf_len = stream->bytes_left;
    data->buf = stream->state;
    return pb_read(stream, (pb_byte_t*)NULL, stream->bytes_left);
}

extern sns_std_suid wigig_sensing_suid;

/**
 * Each SUID event contains an array of SUIDs.  This function will be called
 * once per each SUID in the array.
 *
 * At this time we will send an attributes request to the SUID.
 */
static bool decode_suid
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg
)
{
    sns_std_suid uid;

    if (!pb_decode(stream, sns_std_suid_fields, &uid)) {
        syslog(LOG_ERR, "%s: Error decoding SUID Event: %s", __FUNCTION__, PB_GET_ERROR(stream));
        return false;
    }

    syslog(LOG_DEBUG, "%s: Received SUID Event with SUID %"PRIx64" %"PRIx64, __FUNCTION__,
        uid.suid_low, uid.suid_high);

    wigig_sensing_suid = uid;

    return true;
}

/**
 * Decode the payload of the event message, i.e. the SUID Event
 *
 * @param[i] stream Protobuf stream
 * @param[i] field Protobuf fields to decode
 * @param[i] arg Message ID
 */
bool wigig_sensing_decode_suid_event
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg
)
{
    sns_suid_event suid_event;
    sns_buffer_arg data;

    suid_event.suid.funcs.decode = &decode_suid;
    suid_event.data_type.funcs.decode = &decode_payload;
    suid_event.data_type.arg = &data;

    if (!pb_decode(stream, sns_suid_event_fields, &suid_event)) {
        syslog(LOG_ERR, "%s: Error decoding SUID Event: %s", __FUNCTION__, PB_GET_ERROR(stream));
        return false;
    }
    else {
        char data_type[data.buf_len + 1];
        strlcpy(data_type, (char*)data.buf, data.buf_len + 1);
    }

    bool match_curr_req = false;
    if (wigig_sensing_get_curr_req() == SNS_SUID_MSGID_SNS_SUID_REQ)
        match_curr_req = true;

    wigig_sensing_process_generic_event(0, match_curr_req);

    return true;
}

/**
 * Decode the sns error.
 */
static bool decode_sns_error
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg
)
{
    sns_std_error_event event_msg = sns_std_error_event_init_default;

    if (!pb_decode(stream, sns_std_error_event_fields, &event_msg)) {
        syslog(LOG_ERR, "%s: Error decoding event: %s",  __FUNCTION__, PB_GET_ERROR(stream));
        return false;
    }

    syslog(LOG_ERR, "%s: error received from sns: %d", __FUNCTION__, event_msg.error);
    /* setting error here causes send_request_wait() to finish wait and return with error */
    wigig_sensing_process_generic_event(event_msg.error, true);

    return true;
}

/**
 * Decode the set recovery sensing resp.
 */
static bool decode_recovery_resp
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg,
    bool match_curr_req
)
{
    sns_wigig_auto_recovery_resp resp_msg = sns_wigig_auto_recovery_resp_init_default;

    if (!pb_decode(stream, sns_wigig_auto_recovery_resp_fields, &resp_msg)) {
        syslog(LOG_ERR, "%s: Error decoding event: %s", __FUNCTION__, PB_GET_ERROR(stream));
        return false;
    }

    wigig_sensing_process_generic_event(resp_msg.error, match_curr_req);

    return true;
}

/**
 * Decode the get params sensing resp.
 */
static bool decode_get_params_resp
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg,
    bool match_curr_req
)
{
    sns_wigig_get_params_resp resp_msg = sns_wigig_get_params_resp_init_default;
    sns_buffer_arg data_buf;
    resp_msg.data.funcs.decode = &decode_payload;
    resp_msg.data.arg = &data_buf;

    if (!pb_decode(stream, sns_wigig_get_params_resp_fields, &resp_msg)) {
        syslog(LOG_ERR, "%s: Error decoding event: %s", __FUNCTION__, PB_GET_ERROR(stream));
        return false;
    }

    wigig_sensing_process_get_params_resp(resp_msg.error, data_buf.buf, data_buf.buf_len, match_curr_req);

    return true;
}

/**
 * Decode the debug sensing resp.
 */
static bool decode_debug_resp
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg,
    bool match_curr_req
)
{
    sns_wigig_debug_resp resp_msg = sns_wigig_debug_resp_init_default;
    sns_buffer_arg data_buf;
    resp_msg.data.funcs.decode = &decode_payload;
    resp_msg.data.arg = &data_buf;

    if (!pb_decode(stream, sns_wigig_debug_resp_fields, &resp_msg)) {
        syslog(LOG_ERR, "%s: Error decoding event: %s", __FUNCTION__, PB_GET_ERROR(stream));
        return false;
    }

    wigig_sensing_process_debug_resp(resp_msg.error, resp_msg.req_id, data_buf.buf, data_buf.buf_len, match_curr_req);

    return true;
}

/**
 * Decode the get_mode sensing resp.
 */
static bool decode_get_mode_resp
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg,
    bool match_curr_req
)
{
    sns_wigig_get_mode_resp resp_msg = sns_wigig_get_mode_resp_init_default;

    if (!pb_decode(stream, sns_wigig_get_mode_resp_fields, &resp_msg)) {
        syslog(LOG_ERR, "%s: Error decoding event: %s", __FUNCTION__, PB_GET_ERROR(stream));
        return false;
    }

    wigig_sensing_process_get_mode_resp(&resp_msg, match_curr_req);

    return true;
}

/**
 * Decode the ready sensing event.
 */
static bool decode_ready_event
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg
)
{
    sns_wigig_ready_event event_msg = sns_wigig_ready_event_init_default;

    if (!pb_decode(stream, sns_wigig_ready_event_fields, &event_msg)) {
        syslog(LOG_ERR, "%s: Error decoding event: %s", __FUNCTION__, PB_GET_ERROR(stream));
        return false;
    }

    wigig_sensing_set_fw_ready(true);
    return true;
}

/**
 * Decode the reset sensing event.
 */
static bool decode_reset_event
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg
)
{
    sns_wigig_reset_event event_msg = sns_wigig_reset_event_init_default;

    if (!pb_decode(stream, sns_wigig_reset_event_fields, &event_msg)) {
        syslog(LOG_ERR, "%s: Error decoding event: %s", __FUNCTION__, PB_GET_ERROR(stream));
        return false;
    }

    wigig_sensing_set_fw_ready(false);
    return true;
}

/**
 * Decode the data ready sensing event.
 */
static bool decode_data_ready_event
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg
)
{
    sns_wigig_cir_data_ready_event event_msg = sns_wigig_cir_data_ready_event_init_default;
    sns_buffer_arg data_buf;

    event_msg.data.funcs.decode = &decode_payload;
    event_msg.data.arg = &data_buf;

    if (!pb_decode(stream, sns_wigig_cir_data_ready_event_fields, &event_msg)) {
        syslog(LOG_ERR, "%s: Error decoding event: %s", __FUNCTION__, PB_GET_ERROR(stream));
        return false;
    }

    syslog(LOG_DEBUG, "%s: timestamp: %" PRIu64 ", MF: %d, data_len: %zu ", __FUNCTION__, event_msg.timestamp, event_msg.mf, data_buf.buf_len);
    wigig_sensing_process_data_ready_event(event_msg.timestamp, event_msg.mf, data_buf.buf_len, (uint8_t*)data_buf.buf);

    return true;
}

/**
 * Decode the burst size sensing event.
 */
static bool decode_burst_size_event
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg,
    bool match_curr_req
)
{
    sns_wigig_burst_size_event event_msg = sns_wigig_burst_size_event_init_default;

    if (!pb_decode(stream, sns_wigig_burst_size_event_fields, &event_msg)) {
        syslog(LOG_ERR, "%s: Error decoding event: %s", __FUNCTION__, PB_GET_ERROR(stream));
        return false;
    }

    wigig_sensing_process_burst_size_event(&event_msg, match_curr_req);
    return true;
}

/**
 * Decode an event message received from the wigig sensing Sensor.
 */
bool wigig_sensing_decode_sensing_events
(
    pb_istream_t *stream,
    const pb_field_t *field,
    void **arg
)
{
    uint32_t *msg_id = (uint32_t*)*arg;
    bool match_curr_req = false;

    syslog(LOG_INFO, "%s: Begin decoding wigig sensing event with msg: %d", __FUNCTION__, *msg_id);

    switch (*msg_id)
    {
      case SNS_STD_MSGID_SNS_STD_ERROR_EVENT:
        {
            syslog(LOG_INFO, "SNS_STD_MSGID_SNS_STD_ERROR_EVENT");
            return decode_sns_error(stream, field, arg);
        }
        break;
      case SNS_WIGIG_MSGID_SNS_WIGIG_AUTO_RECOVERY_RESP:
         {
            syslog(LOG_INFO, "WIGIG_SENSING_AUTO_RECOVERY");
            if (wigig_sensing_get_curr_req() == SNS_WIGIG_MSGID_SNS_WIGIG_AUTO_RECOVERY_REQ)
                match_curr_req = true;
            return decode_recovery_resp(stream, field, arg, match_curr_req);
         }
         break;
      case SNS_WIGIG_MSGID_SNS_WIGIG_GET_PARAMS_RESP:
         {
            syslog(LOG_INFO, "WIGIG_SENSING_GET_PARAMS");
            if (wigig_sensing_get_curr_req() == SNS_WIGIG_MSGID_SNS_WIGIG_GET_PARAMS_REQ)
                match_curr_req = true;
            return decode_get_params_resp(stream, field, arg, match_curr_req);
         }
         break;
      case SNS_WIGIG_MSGID_SNS_WIGIG_DEBUG_RESP:
         {
            syslog(LOG_INFO, "WIGIG_SENSING_DEBUG");
            if (wigig_sensing_get_curr_req() == SNS_WIGIG_MSGID_SNS_WIGIG_DEBUG_REQ)
                match_curr_req = true;
            return decode_debug_resp(stream, field, arg, match_curr_req);
         }
         break;
      case SNS_WIGIG_MSGID_SNS_WIGIG_GET_MODE_RESP:
         {
            syslog(LOG_INFO, "WIGIG_SENSING_GET_MODE");
            if (wigig_sensing_get_curr_req() == SNS_WIGIG_MSGID_SNS_WIGIG_GET_MODE_REQ)
                match_curr_req = true;
            return decode_get_mode_resp(stream, field, arg, match_curr_req);
         }
         break;
      case SNS_WIGIG_MSGID_SNS_WIGIG_READY_EVENT:
         {
            syslog(LOG_INFO, "WIGIG_SENSING_READY_EVENT");
            return decode_ready_event(stream, field, arg);
         }
         break;
      case SNS_WIGIG_MSGID_SNS_WIGIG_RESET_EVENT:
         {
            syslog(LOG_INFO, "WIGIG_SENSING_RESET_EVENT");
            return decode_reset_event(stream, field, arg);
         }
         break;
      case SNS_WIGIG_MSGID_SNS_WIGIG_CIR_DATA_READY_EVENT:
         {
            syslog(LOG_INFO, "WIGIG_SENSING_CIR_DATA_READY_EVENT");
            decode_data_ready_event(stream, field, arg);
         }
         break;
      case SNS_WIGIG_MSGID_SNS_WIGIG_BURST_SIZE_EVENT:
         {
            syslog(LOG_INFO, "WIGIG_SENSING_BURST_SIZE_EVENT");
            if (wigig_sensing_get_curr_req() == SNS_WIGIG_MSGID_SNS_WIGIG_CHANGE_MODE_REQ)
                match_curr_req = true;
            decode_burst_size_event(stream, field, arg, match_curr_req);
         }
         break;
      default:
         syslog(LOG_INFO, "Unhandled protobuf message received.\n");
         return false;
   }

    syslog(LOG_INFO, "%s Indication processing completed", __FUNCTION__);

    return true;
}