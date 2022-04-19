/*
 *
 * This file implements the hw session functionality specific to LSM HW
 * sessions that use GCS.
 *
 * Copyright (c) 2016-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#define LOG_TAG "sound_trigger_hw"
#define ATRACE_TAG (ATRACE_TAG_HAL)
/* #define LOG_NDEBUG 0 */
#define LOG_NDDEBUG 0

#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <cutils/trace.h>
#include <fcntl.h>
#include <stdlib.h>
#include "st_hw_session_gcs.h"
#include "sound_trigger_platform.h"
#include "sound_trigger_hw.h"
#include "st_graphite_api.h"
#include "gcs_api.h"
#include "st_hw_common.h"

#define GCS_LIB "libgcs.so"
#define GCS_CONCURRENT_READS_CNT 2
#define WDSP_SYSFS_NAME "/dev/wcd_dsp0_control"

static int reg_sm(st_hw_session_t *p_ses,
    void *sm_data,
    sound_trigger_sound_model_type_t sm_type);
static int reg_sm_params(st_hw_session_t *p_ses,
    unsigned int recognition_mode,
    bool capture_requested,
    struct sound_trigger_recognition_config *rc_config,
    sound_trigger_sound_model_type_t sm_type,
    void *sm_data);
static int read_pcm(st_hw_session_t *p_ses,
    unsigned char *buf,
    unsigned int bytes);
static void process_lab_capture(st_hw_session_t *p_ses);
static int dereg_sm(st_hw_session_t *p_ses, bool capture_requested);
static int dereg_sm_params(st_hw_session_t *p_ses);
static int start(st_hw_session_t *p_ses);
static int restart(st_hw_session_t *p_ses,
    unsigned int recognition_mode, bool capture_requested,
    struct sound_trigger_recognition_config *rc_config __unused,
    sound_trigger_sound_model_type_t sm_type, void *sm_data);
static int stop(st_hw_session_t *p_ses);
static int stop_buffering(st_hw_session_t *p_ses, bool capture_requested);
static int set_device(st_hw_session_t *p_ses,
    bool enable);
static int disable_device(st_hw_session_t *p_ses, bool setting_device);
static int enable_device(st_hw_session_t *p_ses, bool setting_device);
static int send_detection_request(st_hw_session_t *p_ses);

static int dbg_trace_lab_buf_cnt = -1;

static struct pcm_config stdev_cpe_pcm_config = {
    .channels = SOUND_TRIGGER_CHANNEL_MODE_MONO,
    .rate = SOUND_TRIGGER_SAMPLING_RATE_16000,
    .period_size = ST_GRAPHITE_LAB_PERIOD_SIZE_IN_SAMPLES,
    .period_count = ST_GRAPHITE_LAB_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

static struct st_session_fptrs fptrs_gcs = {
    .reg_sm = reg_sm,
    .reg_sm_params = reg_sm_params,
    .dereg_sm = dereg_sm,
    .dereg_sm_params = dereg_sm_params,
    .start = start,
    .restart = restart,
    .stop = stop,
    .stop_buffering = stop_buffering,
    .disable_device = disable_device,
    .enable_device = enable_device,
    .set_device = set_device,
    .read_pcm = read_pcm,
    .process_lab_capture = process_lab_capture,
    .send_detection_request = send_detection_request,
};

/* gcs functions loaded from dynamic library */
static int32_t(*gcs_init_fn)(void);
static int32_t(*gcs_open_fn)(uint32_t UID,
    uint32_t DID,
    uint32_t *graph_handle);
static int32_t(*gcs_enable_fn)(uint32_t graph_handle,
    void *non_persist_ucal,
    uint32_t size_ucal);
static int32_t(*gcs_disable_fn)(uint32_t graph_handle);
static int32_t(*gcs_close_fn)(uint32_t graph_handle);
static int32_t(*gcs_load_data_fn)(uint32_t graph_handle,
    void *data,
    uint32_t data_size,
    uint32_t *data_handle);
static int32_t(*gcs_unload_data_fn)(uint32_t graph_handle, uint32_t data_handle);
static int32_t(*gcs_deinit_fn)(void);
static int32_t(*gcs_register_for_event_fn)(uint32_t graph_handle,
    struct gcs_module_param_info *module_param_info,
    event_cb_ptr cb_handler, void *cookie);

static int32_t(*gcs_register_data_cmd_handler_fn)(uint32_t graph_handle,
    void *data_cmd_cb_handler,
    void *cookie);
static int32_t(*gcs_start_buff_xfer_fn)(uint32_t graph_handle, enum gcs_data_xfer dir);
static int32_t(*gcs_stop_buff_xfer_fn)(uint32_t graph_handle, enum gcs_data_xfer dir);
static int32_t(*gcs_send_data_cmd_fn)(uint32_t graph_handle,
    int8_t *payload,
    uint32_t payload_size);

static int32_t(*gcs_enable_device_fn)(uint32_t graph_handle,
    uint32_t UID, int8_t *payload, uint32_t payload_size);
static int32_t(*gcs_disable_device_fn)(uint32_t graph_handle);
static int32_t (*gcs_set_config_fn)(uint32_t graph_handle, void *payload,
    uint32_t payload_size);

/* used to output pcm to file for debugging */
ST_DBG_DECLARE(static int lab_fp_gcs_cnt = 0);
ST_DBG_DECLARE(static int lab_fp_client_cnt = 0);

struct st_hw_gcs_data {
    void *lib_handle;
    int sysfs_fd;
};

static struct st_hw_gcs_data gcs_data = {.lib_handle = NULL, .sysfs_fd = 0 };

static inline void wdsp_debug_dump(int fd)
{
    ssize_t len = (ssize_t)sizeof("DEBUG_DUMP");
    ssize_t ret_bytes = write(fd, "DEBUG_DUMP", len);

    if (ret_bytes != len) {
        //ALOGE("%s: write failed %zd < %zd, %s", __func__, ret_bytes,
          //  len, strerror(errno));
    }
}

static int32_t gcs_event_cb(uint32_t graph_hdl,
    struct gcs_event_rsp *ev,
    void *private_data)
{
    st_hw_session_gcs_t *p_ses = NULL;
    int32_t status = 0;
    int mutex_ret = 0;

    ALOGD("%s: Enter...", __func__);

    if (!private_data || !ev) {
        ALOGE("%s: received invalid params", __func__);
        status = -EINVAL;
        goto exit;
    }

    p_ses = (st_hw_session_gcs_t *)private_data;

    if (p_ses->graph_handle != graph_hdl) {
        ALOGE("%s: graph_hdl mismatch param has %d but private data has %d",
            __func__, graph_hdl, p_ses->graph_handle);
        status = -EINVAL;
        goto exit;
    }

    if (ev->payload_size == 0 ||
        ev->payload_size > MAX_DETECTION_PAYLOAD_SZ) {
        ALOGE("%s: received invalid detection payload size %d",
            __func__, ev->payload_size);
        status = -EINVAL;
        goto exit;
    }

    if (ev->module_param_info.PID !=
        p_ses->gcs_usecase->params[DETECTION_EVENT].param_id) {
        ALOGE("%s: Detection event PID does not match the requested PID, exiting",
            __func__);
        status = -EINVAL;
        goto exit;
    }

    /*
     * We use try_lock here, if a detection event is currently being processed, we
     * will ignore incoming detections until processing the current one is completed.
     * Anyway we are not expected to get any detection notification from FW until the
     * previous one is processed and the application restarts recognition
     */
    mutex_ret = pthread_mutex_trylock(&p_ses->callback_thread_lock);
    if (!mutex_ret) {
        /* The duration of this trace log indicates the detection latency. */
        ATRACE_ASYNC_BEGIN("sthal: detection success",
            p_ses->common.sm_handle);
        ATRACE_ASYNC_BEGIN("sthal: detection reject",
            p_ses->common.sm_handle);
        ALOGV("%s: got mutex lock", __func__);
        p_ses->detect_payload_size = ev->payload_size;
        memcpy(p_ses->detect_payload, ev->payload, p_ses->detect_payload_size);
        p_ses->detection_signaled = true;
        pthread_cond_signal(&p_ses->callback_thread_cond);
        pthread_mutex_unlock(&p_ses->callback_thread_lock);
    } else {
        ALOGD("%s: previous event in-progress, ignore event", __func__);
    }

exit:
    return status;
}

static int32_t gcs_data_cmdrsp_cb(uint32_t graph_handle,
    void *rsp,
    size_t rsp_size,
    void *cookie, int32_t cmd_status)
{
    int status = 0;
    st_hw_session_gcs_t *p_gcs_ses = NULL;
    struct graphite_data_cmdrsp_hdr *hdr = NULL;
    struct gcs_cmd_readrsp_payload_t *payload = NULL;
    uint32_t buff_sz = 0;
    uint8_t *buff = NULL;
    struct st_vendor_info *v_info = NULL ;
    st_hw_session_t *p_ses = NULL;
    sound_trigger_device_t *  stdev = NULL;
    struct listnode *node = NULL, *tmp_node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;

    p_gcs_ses = (st_hw_session_gcs_t *)cookie;
    ST_DBG_ATRACE_ASYNC_END_IF(++dbg_trace_lab_buf_cnt == dbg_trace_max_lab_reads,
                    "sthal:gcs: gcs_buffer_read",p_gcs_ses->common.sm_handle);
    if (!p_gcs_ses) {
        ALOGE("%s: received NULL cookie", __func__);
        return -EINVAL;
    }

    if (p_gcs_ses->graph_handle != graph_handle) {
        ALOGE("%s: graph_hdl mismatch param has %d but private data has %d",
            __func__, graph_handle, p_gcs_ses->graph_handle);
        return -EINVAL;
    }

    if (0 > cmd_status) {
        ALOGE("%s: received failed cmdrsp status: %d", __func__, cmd_status);
        return status;
    }

    if (0 == rsp_size) {
        ALOGE("%s: received response size of 0", __func__);
        return -EINVAL;
    }

    p_gcs_ses->frame_receive_time = get_current_time_ns();
    p_ses = (st_hw_session_t *)p_gcs_ses;
    v_info = p_ses->vendor_uuid_info;
    stdev = p_ses->stdev;

    /* parse CMDRSP message */
    hdr = (struct graphite_data_cmdrsp_hdr *)rsp;

    if (hdr->module_id != p_gcs_ses->gcs_usecase->params[READ_RSP].module_id ||
        hdr->instance_id != p_gcs_ses->gcs_usecase->params[READ_RSP].instance_id ||
        hdr->cmd_id != p_gcs_ses->gcs_usecase->params[READ_RSP].param_id) {
        ALOGE("%s: received unexpected parameters module_id %d, instance_id %d"
            ", cmd_id %d expected module_id %d, instance_id %d, cmd_id %d",
            __func__, hdr->module_id, hdr->instance_id, hdr->cmd_id,
            p_gcs_ses->gcs_usecase->params[READ_RSP].module_id,
            p_gcs_ses->gcs_usecase->params[READ_RSP].instance_id,
            p_gcs_ses->gcs_usecase->params[READ_RSP].param_id);
        return -EINVAL;
    }

    /* received response for READ CMD */
    payload = (struct gcs_cmd_readrsp_payload_t *)((uint8_t *)rsp
        + sizeof(struct graphite_data_cmdrsp_hdr));

    if (GCS_READ_CMDRSP_STATUS_SUCCESS != payload->status) {
        ALOGE("%s: cmdrsp_status %d", __func__, payload->status);
        return -EINVAL;
    }

    buff_sz = hdr->size_in_bytes - sizeof(struct gcs_cmd_readrsp_payload_t);
    buff = (uint8_t *)payload + sizeof(struct gcs_cmd_readrsp_payload_t);

    ST_DBG_FILE_WRITE(p_gcs_ses->lab_fp_gcs, buff, buff_sz);

    if (buff_sz > ST_GCS_READ_BUF_SIZE) {
        ALOGW("%s: received size %d more than requested %d, truncate",
              __func__, buff_sz, ST_GCS_READ_BUF_SIZE);
        buff_sz = ST_GCS_READ_BUF_SIZE;
    }

    pthread_mutex_lock(&p_gcs_ses->lock);
    if (!p_gcs_ses->exit_buffering) {

        if ((v_info->kw_capture_format & MULAW) &&
            stdev->mulaw_dec_process) {
            /* Produces 1:2 decoded data for any input size */
            ATRACE_BEGIN("sthal:gcs: mulaw_dec_process");
            status = stdev->mulaw_dec_process((short *)p_gcs_ses->mulaw_op_buf,
                                              (char *)buff, buff_sz);
            ATRACE_END();
            if (status) {
                ALOGE("%s:[%d] mulaw error %d ", __func__, p_ses->sm_handle,
                      status);
                p_gcs_ses->exit_buffering = true;
                goto exit_unlock;
            }
            /* don't need input buff anymore */
            buff = p_gcs_ses->mulaw_op_buf;
            buff_sz <<= 1; /* expected decoded output size */
        }

        status = st_buffer_write(p_ses->buffer, buff, buff_sz);
        if (status) {
            p_gcs_ses->exit_buffering = true;
            if (p_ses->enable_second_stage) {
                list_for_each_safe(node, tmp_node, p_ses->second_stage_list) {
                    st_sec_stage = node_to_item(node, st_arm_second_stage_t,
                        list_node);
                    pthread_mutex_lock(&st_sec_stage->ss_session->lock);
                    ALOGW("%s: Exit 2nd stage processing due to buf overflow",
                        __func__);
                    st_sec_stage->ss_session->exit_buffering = true;
                    pthread_cond_signal(&st_sec_stage->ss_session->cond);
                    pthread_mutex_unlock(&st_sec_stage->ss_session->lock);
                }
            }
            goto exit_unlock;
        }

        p_gcs_ses->unread_bytes += buff_sz;
        p_gcs_ses->bytes_written += buff_sz;

        if (p_ses->enable_second_stage) {
            list_for_each_safe(node, tmp_node, p_ses->second_stage_list) {
                st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);

                /*
                 * When the current written frame passes the number of bytes indicated
                 * in buf_start, give the second stage session its read pointer.
                 */
                pthread_mutex_lock(&st_sec_stage->ss_session->lock);
                if ((p_gcs_ses->bytes_written >= st_sec_stage->ss_session->buf_start) &&
                    !st_sec_stage->ss_session->start_processing) {
                    st_sec_stage->ss_session->hw_rd_ptr =
                        st_buffer_get_wr_ptr(p_ses->buffer);
                    st_sec_stage->ss_session->hw_rd_ptr -= buff_sz;
                    st_sec_stage->ss_session->start_processing = true;
                }

                if (st_sec_stage->ss_session->start_processing) {
                    st_sec_stage->ss_session->unread_bytes += buff_sz;
                    pthread_cond_signal(&st_sec_stage->ss_session->cond);
                }
                pthread_mutex_unlock(&st_sec_stage->ss_session->lock);
            }
        }
        /*
         * wakeup both reader thread to read and buffering thread to
         * request more data
         */
        ALOGVV("%s: signaling cond", __func__);
        ++p_gcs_ses->read_rsp_cnt;
        pthread_cond_broadcast(&p_gcs_ses->cond);
    }

exit_unlock:
    pthread_mutex_unlock(&p_gcs_ses->lock);

    return status;
}

static void* callback_thread_loop(void *context)
{
    st_hw_session_gcs_t *p_ses = (st_hw_session_gcs_t *)context;
    st_hw_sess_event_t hw_ses_event; /* used to report event to st_session */
    struct gcs_det_engine_event *p_det = NULL;
    struct gcs_det_engine_extended_event *p_det_ext = NULL;
    uint8_t *ev_payload = NULL;
    size_t ev_payload_size = 0;
    uint32_t event_status = 0;

    if (!p_ses) {
        ALOGE("%s: Received null session ptr", __func__);
        goto exit;
    }
    ALOGV("%s: callback thread started...", __func__);
    pthread_mutex_lock(&p_ses->callback_thread_lock);
    while (!p_ses->detection_signaled && !p_ses->exit_detection) {
        pthread_cond_wait(&p_ses->callback_thread_cond,
            &p_ses->callback_thread_lock);
        /*
         * Currently, DSP does not support the inclusion of detection
         * timestamp within the payload. So the timestamp is filled here
         * instead.
         */
        p_ses->common.first_stage_det_event_time = get_current_time_ns();
        ALOGV("%s: came out of cond_wait...", __func__);

        if (p_ses->exit_detection)
            break;

        if (!p_ses->detection_signaled)
            continue;

        ALOGV("%s: detection signaled", __func__);
        p_ses->detection_signaled = false;
        p_det = (struct gcs_det_engine_event *)p_ses->detect_payload;

        ST_DBG_DECLARE(FILE *detect_fd = NULL; static int detect_fd_cnt = 0);
        ST_DBG_FILE_OPEN_WR(detect_fd, ST_DEBUG_DUMP_LOCATION,
                            "gcs_detection_event", "bin", detect_fd_cnt++);
        ST_DBG_FILE_WRITE(detect_fd, p_ses->detect_payload,
                          p_ses->detect_payload_size);
        ST_DBG_FILE_CLOSE(detect_fd);

        if (p_ses->start_engine_cal) {
            p_det_ext = (struct gcs_det_engine_extended_event *)p_det;
            /* This is a special case of handling multiple VA engines in single
             * DSP VA module with HAL providing a customized interface to
             * VA module.
             * gcs sends one VA engine event to another VA engine session if
             * both sessions are registered for same DSP module containing both
             * VA engines. Ignore the unwanted event by validating the token in
             * the payload
             */
            if (p_det_ext->start_params.token !=
                p_ses->start_engine_cal->token) {
                ALOGV("%s: Ignore unwanted event, gh %d token %d != %d",
                      __func__, p_ses->graph_handle,
                      p_det_ext->start_params.token,
                      p_ses->start_engine_cal->token);
                continue;
            }
            /* Get to actual VA engine specific payload */
            ev_payload = p_ses->detect_payload +
                         sizeof(p_det_ext->start_params);
            ev_payload_size = p_det->custom_payload_sz -
                              sizeof(p_det_ext->start_params);
        } else {
            unsigned int payload_offset = 0;
            if (p_ses->common.vendor_uuid_info->is_qcmd_uuid)
                payload_offset = sizeof(struct st_param_header) +
                                 GENERIC_DET_EVENT_HEADER_SIZE;

            ev_payload = p_ses->detect_payload + sizeof(*p_det) +
                         payload_offset;
            ev_payload_size = p_det->custom_payload_sz - payload_offset;
        }

        /* get detection status */
        if (p_det->status == GCS_DETECTION_ENGINE_EVENT_DETECTED)
            event_status = RECOGNITION_STATUS_SUCCESS;
        else
            event_status = RECOGNITION_STATUS_FAILURE;

        /*
         * inform st_session of the event, st_session knows how to parse list
         * of conf levels
         */
        ALOGV("%s: notifying st_session of the detection event, status = %d",
            __func__, event_status);

        hw_ses_event.event_id = ST_HW_SESS_EVENT_DETECTED;
        hw_ses_event.payload.detected.timestamp = 0;
        hw_ses_event.payload.detected.detect_status = event_status;
        hw_ses_event.payload.detected.payload_size = ev_payload_size;
        hw_ses_event.payload.detected.detect_payload = ev_payload;

        p_ses->common.callback_to_st_session(&hw_ses_event,
                                             p_ses->common.cookie);
    }

    pthread_mutex_unlock(&p_ses->callback_thread_lock);
    ALOGV("%s: callback thread ending...", __func__);
exit:
    return NULL;
}

static int reg_sm(st_hw_session_t *p_ses,
    void *sm_data,
    sound_trigger_sound_model_type_t sm_type)
{
    int status = 0;
    uint8_t *load_sm_msg = NULL;
    struct graphite_cal_header *sm_msg_hdr = NULL;
    size_t load_sm_msg_sz = 0;
    struct st_vendor_info *v_info = p_ses->vendor_uuid_info;
    struct gcs_module_param_info gcs_module_info; /* used to register for ev*/
    struct sound_trigger_phrase_sound_model *phrase_sm =
        (struct sound_trigger_phrase_sound_model *)sm_data;
    struct sound_trigger_sound_model *common_sm =
        (struct sound_trigger_sound_model *)sm_data;
    size_t sm_data_size = 0;
    uint32_t sm_data_offset = 0;
    st_hw_session_gcs_t *p_gcs_ses = (st_hw_session_gcs_t *)p_ses;
    int st_device = 0, device_acdb_id = 0;
    int capture_device;

    /* BG target doesn't have wdsp */
    if (!p_ses->stdev->bg_kwd) {
        status = st_hw_gcs_load_wdsp(true);
        if (status)
            return status;
    }

    /* get the acdb device id to use when opening a graph */
    capture_device = platform_stdev_get_capture_device(p_ses->stdev->platform);
    st_device = platform_stdev_get_device(p_ses->stdev->platform,
        v_info, capture_device, p_ses->exec_mode);
    if (st_device == ST_DEVICE_NONE) {
        ALOGE("%s: Could not find valid device",__func__);
        status = -EINVAL;
        goto cleanup2;
    }

    device_acdb_id = platform_stdev_get_acdb_id(st_device,
        p_ses->exec_mode);
    if (0 > device_acdb_id) {
        ALOGE("%s: Could not get device ACDB ID", __func__);
        status = -EINVAL;
        goto cleanup2;
    }

    /* allocate a gcs use-case for this session from vendor info */
    platform_alloc_gcs_usecase(p_ses->stdev->platform, v_info,
        &p_gcs_ses->gcs_usecase, device_acdb_id);
    if (!p_gcs_ses->gcs_usecase) {
        ALOGE("%s: failed to allocate gcs usecase for the session", __func__);
        status = -ENOMEM;
        goto cleanup2;
    }

    p_ses->config = stdev_cpe_pcm_config;
    platform_stdev_check_and_update_pcm_config(p_ses->stdev->platform,
                                               &p_ses->config, v_info,
                                               p_ses->exec_mode);

    ALOGD("%s:[%d] calling gcs_open with uid %d, did %d", __func__,
        p_ses->sm_handle, p_gcs_ses->gcs_usecase->uid, device_acdb_id);
    ATRACE_BEGIN("sthal:gcs: gcs_open");
    status = gcs_open_fn(p_gcs_ses->gcs_usecase->uid, device_acdb_id,
        &p_gcs_ses->graph_handle);
    ATRACE_END();
    if (status) {
        ALOGE("%s: gcs_open failed with status %d", __func__, status);
        wdsp_debug_dump(gcs_data.sysfs_fd);
        goto cleanup1;
    }

    if (sm_type == SOUND_MODEL_TYPE_KEYPHRASE) {
        sm_data_size = phrase_sm->common.data_size;
        sm_data_offset = phrase_sm->common.data_offset;
    } else {
        sm_data_size = common_sm->data_size;
        sm_data_offset = common_sm->data_offset;
    }

    /* calculate size of load sm msg */
    load_sm_msg_sz = sizeof(struct graphite_cal_header); /* param header for sound model param */
    load_sm_msg_sz += sm_data_size; /* SM opaque data size from upper layers */
    load_sm_msg_sz = ALIGN(load_sm_msg_sz, 4);

    load_sm_msg = calloc(load_sm_msg_sz, sizeof(uint8_t));
    if (!load_sm_msg) {
        ALOGE("%s: failed to allocate memory for sm msg, size = %zu", __func__,
            load_sm_msg_sz);
        status = -ENOMEM;
        goto cleanup1;
    }
    sm_msg_hdr = (struct graphite_cal_header *)load_sm_msg;
    sm_msg_hdr->module_id = p_gcs_ses->gcs_usecase->
        params[LOAD_SOUND_MODEL].module_id;
    sm_msg_hdr->instance_id = p_gcs_ses->gcs_usecase->
        params[LOAD_SOUND_MODEL].instance_id;
    sm_msg_hdr->param_id = p_gcs_ses->gcs_usecase->
        params[LOAD_SOUND_MODEL].param_id;
    sm_msg_hdr->size = sm_data_size;

    ALOGV("%s: sm cal header MID %x, IID %x, PID %x \n sm data %p, sm size %zu,"
        " alloc size %zu", __func__, sm_msg_hdr->module_id,
        sm_msg_hdr->instance_id, sm_msg_hdr->param_id, sm_data,
        sm_data_size, load_sm_msg_sz);

    memcpy(load_sm_msg + sizeof(struct graphite_cal_header),
        (uint8_t *)sm_data + sm_data_offset, sm_data_size);


    ST_DBG_DECLARE(FILE *load_fd = NULL; static int load_fd_cnt = 0);
    ST_DBG_FILE_OPEN_WR(load_fd, ST_DEBUG_DUMP_LOCATION, "load_sm", "bin", load_fd_cnt++);
    ST_DBG_FILE_WRITE(load_fd, load_sm_msg, load_sm_msg_sz);
    ST_DBG_FILE_CLOSE(load_fd);

    ALOGD("%s:[%d] calling gcs_load_data with graph_handle %d, load_sm_msg %p, "
        "load_sm_msg_sz %zu", __func__, p_ses->sm_handle, p_gcs_ses->graph_handle,
        load_sm_msg, load_sm_msg_sz);
    ATRACE_BEGIN("sthal:gcs: gcs_load_data");
    status = gcs_load_data_fn(p_gcs_ses->graph_handle, load_sm_msg,
        (uint32_t)load_sm_msg_sz, &p_gcs_ses->loaded_sm_handle);
    ATRACE_END();
    if (status) {
        ALOGE("%s: gcs_load_data failed with status %d", __func__, status);
        wdsp_debug_dump(gcs_data.sysfs_fd);
        goto cleanup1;
    }

    free(load_sm_msg);
    load_sm_msg = NULL;

    /* register callback for event handling */
    gcs_module_info.module_info.MID =
        p_gcs_ses->gcs_usecase->params[DETECTION_EVENT].module_id;
    gcs_module_info.module_info.IID =
        p_gcs_ses->gcs_usecase->params[DETECTION_EVENT].instance_id;
    gcs_module_info.PID =
        p_gcs_ses->gcs_usecase->params[DETECTION_EVENT].param_id;

    ALOGD("%s:[%d] calling gcs_register_for_event with MID %x, IID %x, PID %x", __func__,
        p_ses->sm_handle, gcs_module_info.module_info.MID,
        gcs_module_info.module_info.IID, gcs_module_info.PID);
    ATRACE_BEGIN("sthal:gcs: gcs_register_for_event");
    status = gcs_register_for_event_fn(p_gcs_ses->graph_handle, &gcs_module_info,
        gcs_event_cb, p_gcs_ses);
    ATRACE_END();
    if (status) {
        ALOGE("%s: gcs_register_for_event failed with status %d", __func__, status);
        wdsp_debug_dump(gcs_data.sysfs_fd);
        goto cleanup1;
    }

    /* register callback for data cmd handling */
    ALOGD("%s:[%d] calling gcs_register_data_cmd_handler with handle %d, cb %p, "
        "cookie %p", __func__, p_ses->sm_handle,
        p_gcs_ses->graph_handle, gcs_data_cmdrsp_cb, p_gcs_ses);
    ATRACE_BEGIN("sthal:gcs: gcs_register_data_cmd_handler");
    status = gcs_register_data_cmd_handler_fn(p_gcs_ses->graph_handle,
        (data_cmd_cb_ptr)gcs_data_cmdrsp_cb, p_gcs_ses);
    ATRACE_END();
    if (status) {
        ALOGE("%s: gcs_register_data_cmd_handler failed with status %d",
            __func__, status);
        wdsp_debug_dump(gcs_data.sysfs_fd);
        goto cleanup1;
    }

    return status;

cleanup1:
    if (load_sm_msg)
        free(load_sm_msg);
    platform_free_gcs_usecase(v_info, p_gcs_ses->gcs_usecase);
cleanup2:
    /* unload WDSP image */
    if (gcs_data.sysfs_fd >= 0)
        write(gcs_data.sysfs_fd, "0", 2);

    return status;
}

static int reg_sm_params(st_hw_session_t *p_ses,
    unsigned int recognition_mode,
    bool capture_requested,
    struct sound_trigger_recognition_config *rc_config,
    sound_trigger_sound_model_type_t sm_type __unused,
    void *sm_data __unused)
{
    st_hw_session_gcs_t *p_hw_ses = (st_hw_session_gcs_t *)p_ses;
    uint8_t *msg_offset = NULL;
    uint32_t rt_bytes_one_sec;
    unsigned int i;
    int status = 0;
    size_t det_config_size = 0, custom_config_size = 0, start_custom_config_size = 0;
    size_t det_event_type_size = 0;
    struct gcs_det_engine_config_param *p_msg = NULL;
    struct gcs_det_engine_custom_config_param *cc_msg = NULL;
    struct gcs_det_engine_start_custom_config *stcfg_msg = NULL;
    struct gcs_det_event_type_custom_config *det_event_msg = NULL;
    struct st_vendor_info *v_info = p_ses->vendor_uuid_info;
    bool disable_custom_config = false;

    if (NULL != p_hw_ses->nonpersistent_cal) {
        ALOGE("%s: nonpersistent cal data already cached! failing.", __func__);
        return -EINVAL;
    }

    /* The nonpersistant_cal to be sent to WDSP can contain mulitple param
     * payloads with corresponding headers. For example
     *     |Param Header1|
     *         |Param Payload1|
     *     |Param Header2|
     *         |Param Payload2|
     *     ...
     * Each header and its payload size must be 4bytes aligned.
     *
     * These params can be combination DETECTION_ENGINE_CONFIG,
     * DETECTION_ENGINE_CUSTOM_CONFIG and START_ENGINE params.
     *
     * Set custom_config flag if mID and pID have been set in platform xml
     * file. If the flag is set, the opaque data from client will be wrapped
     * in a header containing mID, pID, etc. Else, the opaque data is sent as
     * it is, and it is assumed that it is formatted from within.
     */

    if ((rc_config->data_size > CUSTOM_CONFIG_OPAQUE_DATA_SIZE) &&
        p_ses->vendor_uuid_info->is_qcva_uuid) {
        if (!capture_requested)
            disable_custom_config = true;

        det_config_size = sizeof(struct gcs_det_engine_config_param) +
                          p_ses->num_conf_levels;
        det_config_size = ALIGN(det_config_size, 4);
        p_hw_ses->nonpersistent_cal_size += det_config_size;

        if (!disable_custom_config)
            p_hw_ses->nonpersistent_cal_size +=
                sizeof(struct gcs_det_engine_custom_config_param) +
                sizeof(struct st_hist_buffer_info);
    } else {
        /*
         * This logic is for the legacy opaque data or no opaque data at all.
         * At most, the custom config detection engine payload will be filled
         * from the opaque data.
         */
        if (p_hw_ses->gcs_usecase->params[CUSTOM_CONFIG].module_id &&
            p_hw_ses->gcs_usecase->params[CUSTOM_CONFIG].param_id &&
            rc_config->data_size) {
            custom_config_size =
                sizeof(struct gcs_det_engine_custom_config_param) +
                rc_config->data_size;
            custom_config_size = ALIGN(custom_config_size, 4);
            p_hw_ses->nonpersistent_cal_size += custom_config_size;
        }

        if (v_info->smlib_handle) {
            det_config_size = sizeof(struct gcs_det_engine_config_param) +
                              p_ses->num_conf_levels;
            /* If not using custom config param, send opaque data as part of
             * DETECTION_ENGINE_CONFIG. Opaque data will be put after confidence
             * level payload data.
             */
            if (rc_config->data_size && !custom_config_size)
                det_config_size += rc_config->data_size;

            det_config_size = ALIGN(det_config_size, 4);
            p_hw_ses->nonpersistent_cal_size += det_config_size;
        }
    }

    if (p_hw_ses->gcs_usecase->params[DET_EVENT_TYPE].module_id &&
        p_hw_ses->gcs_usecase->params[DET_EVENT_TYPE].param_id) {
        /* MD module on DSP by default sends generic event.
         * So no need to explicitly request the DSP.
         * Just set this flag to handle generic detection event coming from DSP
         */
        p_ses->is_generic_event = true;
        if (p_ses->vendor_uuid_info->is_qcva_uuid) {
            det_event_type_size +=
                sizeof(struct gcs_det_event_type_custom_config);
            det_event_type_size = ALIGN(det_event_type_size, 4);
            p_hw_ses->nonpersistent_cal_size += det_event_type_size;
        }
    }

    if (p_hw_ses->gcs_usecase->params[START_ENGINE].module_id &&
        p_hw_ses->gcs_usecase->params[START_ENGINE].param_id) {
        start_custom_config_size +=
            sizeof(struct gcs_det_engine_start_custom_config);
        /* API size is already 4 byte aligned */
        p_hw_ses->nonpersistent_cal_size += start_custom_config_size;
    }

    if (p_hw_ses->nonpersistent_cal_size) {
        p_hw_ses->nonpersistent_cal = calloc(1, p_hw_ses->nonpersistent_cal_size);
        if (!p_hw_ses->nonpersistent_cal) {
            ALOGE("%s: failed to alloc nonpersistent cal ", __func__);
            status = -ENOMEM;
            goto err_exit;
        }
        msg_offset = p_hw_ses->nonpersistent_cal;

        if (det_config_size) {
            p_msg = (struct gcs_det_engine_config_param *)msg_offset;
            p_msg->cal_hdr.module_id =
                p_hw_ses->gcs_usecase->params[CONFIDENCE_LEVELS].module_id;
            p_msg->cal_hdr.instance_id =
                p_hw_ses->gcs_usecase->params[CONFIDENCE_LEVELS].instance_id;
            p_msg->cal_hdr.param_id =
                p_hw_ses->gcs_usecase->params[CONFIDENCE_LEVELS].param_id;

            ALOGV("%s: nonpersistent cal header MID %x, IID %x, PID %x",
                  __func__, p_msg->cal_hdr.module_id,
                  p_msg->cal_hdr.instance_id, p_msg->cal_hdr.param_id);

            p_msg->cal_hdr.size = det_config_size -
                sizeof(struct graphite_cal_header);
            /*
             * SVA doesn't support per keyword recogntion mode.
             * use the per soundmodel recognition mode
             */
            if (recognition_mode & RECOGNITION_MODE_VOICE_TRIGGER) {
                p_msg->mode = GCS_DETECTION_ENGINE_CONFIG_KW_DET_ENABLE;

                if (recognition_mode & RECOGNITION_MODE_USER_IDENTIFICATION)
                    p_msg->mode |= GCS_DETECTION_ENGINE_CONFIG_USER_VER_ENABLE;

                if (p_ses->stdev->detect_failure)
                    p_msg->mode |=
                        GCS_DETECTION_ENGINE_CONFIG_FAILURE_DET_ENABLE;
            } else {
                    ALOGE("%s: Unknown recognition mode %d", __func__,
                        recognition_mode);
                    status = -EINVAL;
                    goto err_exit;
            }

            msg_offset += sizeof(struct gcs_det_engine_config_param);
            if (p_ses->conf_levels) {
                memcpy(msg_offset, (uint8_t *)p_ses->conf_levels, p_ses->num_conf_levels);
                msg_offset += p_ses->num_conf_levels;
                p_msg->custom_payload_sz = p_ses->num_conf_levels;
                /*
                 * The detection_engine_config struct has 2 bytes for minor
                 * version and num_active_models before the confidence levels.
                 * There is also 1 byte added for each confidence level as an
                 * enable/disable flag.
                 */
                for (i = 0; i < (p_ses->num_conf_levels - 2) / 2; i++) {
                    ALOGD("%s: First stage conf_levels[%d] = %d",
                        __func__, i, *(p_ses->conf_levels + 2 + i));
                }
            }
        }

        /* Start at 4 byte aligned custom config param header */
        msg_offset = (uint8_t *)ALIGN((size_t)msg_offset, 4);

        if (rc_config->data_size && !disable_custom_config) {
            if (p_hw_ses->gcs_usecase->params[CUSTOM_CONFIG].module_id &&
                p_hw_ses->gcs_usecase->params[CUSTOM_CONFIG].param_id) {

                cc_msg = (struct gcs_det_engine_custom_config_param *)msg_offset;
                cc_msg->cal_hdr.module_id =
                    p_hw_ses->gcs_usecase->params[CUSTOM_CONFIG].module_id;
                cc_msg->cal_hdr.instance_id =
                    p_hw_ses->gcs_usecase->params[CUSTOM_CONFIG].instance_id;
                cc_msg->cal_hdr.param_id =
                    p_hw_ses->gcs_usecase->params[CUSTOM_CONFIG].param_id;

                ALOGV("%s: custom config header MID %x, IID %x, PID %x", __func__,
                    cc_msg->cal_hdr.module_id, cc_msg->cal_hdr.instance_id,
                    cc_msg->cal_hdr.param_id);

                if ((rc_config->data_size > CUSTOM_CONFIG_OPAQUE_DATA_SIZE) &&
                    p_ses->vendor_uuid_info->is_qcva_uuid) {
                    /* Custom config for updated opaque data structure for SVA 3.0. */
                    cc_msg->cal_hdr.size = sizeof(struct st_hist_buffer_info);
                    msg_offset += sizeof(struct gcs_det_engine_custom_config_param);
                    st_hw_ses_get_hist_buff_payload(p_ses, (uint8_t *)msg_offset,
                                                    sizeof(struct st_hist_buffer_info));
                    msg_offset += sizeof(struct st_hist_buffer_info);

                } else {
                    /* Custom config for legacy opaque data. */
                    ALOGV("%s: copying opaque data, size = %d", __func__,
                        rc_config->data_size);
                    cc_msg->cal_hdr.size = custom_config_size -
                        sizeof(struct graphite_cal_header);
                    msg_offset += sizeof(struct gcs_det_engine_custom_config_param);
                    memcpy(msg_offset, (unsigned char *)rc_config +
                        rc_config->data_offset, rc_config->data_size);
                    msg_offset += rc_config->data_size;
                }
            } else {
                /*
                 * Copy opaque data as part of det_engine_config if there's no custom
                 * config IDs.
                 */
                ALOGV("%s: copying opaque data, size = %d", __func__,
                    rc_config->data_size);
                memcpy(msg_offset, (unsigned char *)rc_config +
                    rc_config->data_offset, rc_config->data_size);
                msg_offset += rc_config->data_size;
            }
        }

        if (start_custom_config_size) {
            /* Start at 4 byte aligned start engine param header */
            msg_offset = (uint8_t *)ALIGN((size_t)msg_offset, 4);
            stcfg_msg = (struct gcs_det_engine_start_custom_config *)msg_offset;
            stcfg_msg->cal_hdr.module_id =
                p_hw_ses->gcs_usecase->params[START_ENGINE].module_id;
            stcfg_msg->cal_hdr.instance_id =
                p_hw_ses->gcs_usecase->params[START_ENGINE].instance_id;
            stcfg_msg->cal_hdr.param_id =
                p_hw_ses->gcs_usecase->params[START_ENGINE].param_id;

            stcfg_msg->cal_hdr.size = start_custom_config_size -
                                   sizeof(struct graphite_cal_header);
            stcfg_msg->minor_version = 0x01;
            stcfg_msg->enable = 1;
            stcfg_msg->token = android_atomic_inc(&p_ses->stdev->gcs_token);
            p_hw_ses->start_engine_cal = stcfg_msg;
            p_hw_ses->start_engine_cal_size = start_custom_config_size;
            ALOGV("%s: start VA engine MID 0x%x, IID 0x%x, PID 0x%x,"
                  "enable %d, token %d", __func__, stcfg_msg->cal_hdr.module_id,
                  stcfg_msg->cal_hdr.instance_id, stcfg_msg->cal_hdr.param_id,
                  stcfg_msg->enable, stcfg_msg->token);
            msg_offset += sizeof(struct gcs_det_engine_start_custom_config);
        }

        if (det_event_type_size) {
            /* Start at 4 byte aligned start engine param header */
            msg_offset = (uint8_t *)ALIGN((size_t)msg_offset, 4);
            det_event_msg = (struct gcs_det_event_type_custom_config *)msg_offset;
            det_event_msg->cal_hdr.module_id =
                p_hw_ses->gcs_usecase->params[DET_EVENT_TYPE].module_id;
            det_event_msg->cal_hdr.instance_id =
                p_hw_ses->gcs_usecase->params[DET_EVENT_TYPE].instance_id;
            det_event_msg->cal_hdr.param_id =
                p_hw_ses->gcs_usecase->params[DET_EVENT_TYPE].param_id;

            det_event_msg->cal_hdr.size = det_event_type_size -
                                   sizeof(struct graphite_cal_header);

            det_event_msg->minor_version = 0x01;
            det_event_msg->event_type = 1;
            if (v_info->is_qcmd_uuid)
                det_event_msg->mode = 1;
            else
                det_event_msg->mode = 3;
            ALOGV("%s: Detection event type MID 0x%x, IID 0x%x, PID 0x%x,"
                  "event_type %d, mode %d", __func__, det_event_msg->cal_hdr.module_id,
                  det_event_msg->cal_hdr.instance_id, det_event_msg->cal_hdr.param_id,
                  det_event_msg->event_type, det_event_msg->mode);
        }
    }

    if (capture_requested) {
        int circ_buff_sz = 0;
        /* allocate buffers used for LAB transfer*/
        rt_bytes_one_sec = (p_ses->config.rate * p_ses->config.channels *
            (pcm_format_to_bits(p_ses->config.format) >> 3));

        circ_buff_sz = ((v_info->kw_duration +
            v_info->client_capture_read_delay) * rt_bytes_one_sec) / 1000;
        p_ses->buffer = st_buffer_init(circ_buff_sz);
        if (!p_ses->buffer) {
            ALOGE("%s: failed to allocate circ buffer", __func__);
            status = -ENOMEM;
            goto err_exit;
        }

        if (v_info->kw_capture_format & MULAW) {
            /* Allocate 2 times the requested compressed input size */
            p_hw_ses->mulaw_op_buf = calloc(1, ST_GCS_READ_BUF_SIZE << 1);
            if (!p_hw_ses->mulaw_op_buf) {
                ALOGE("%s: failed to allocate mulaw buffer sz %d", __func__,
                      ST_GCS_READ_BUF_SIZE << 1);
                status = -ENOMEM;
                goto err_exit;
            }
        }
    }

    return status;

err_exit:

    if (p_hw_ses->nonpersistent_cal) {
        free(p_hw_ses->nonpersistent_cal);
        p_hw_ses->nonpersistent_cal = NULL;
        p_hw_ses->start_engine_cal = NULL;
    }

    return status;
}

static int dereg_sm(st_hw_session_t *p_ses, bool capture_requested __unused)
{
    int status = 0, rc = 0;
    st_hw_session_gcs_t *p_gcs_ses = (st_hw_session_gcs_t *)p_ses;

    ALOGD("%s:[%d] calling gcs_unload_data with handle %d, loaded data handle %d",
        __func__, p_ses->sm_handle, p_gcs_ses->graph_handle,
        p_gcs_ses->loaded_sm_handle);
    ATRACE_BEGIN("sthal:gcs: gcs_unload_data");
    status = gcs_unload_data_fn(p_gcs_ses->graph_handle,
        p_gcs_ses->loaded_sm_handle);
    ATRACE_END();
    if (status) {
        rc = status;
        ALOGE("%s: gcs_unload_data failed with status %d", __func__, status);
        wdsp_debug_dump(gcs_data.sysfs_fd);
    }

    ALOGD("%s:[%d] calling gcs_close on handle %d", __func__, p_ses->sm_handle,
        p_gcs_ses->graph_handle);
    ATRACE_BEGIN("sthal:gcs: gcs_close");
    status = gcs_close_fn(p_gcs_ses->graph_handle);
    ATRACE_END();
    if (status) {
        rc = status;
        ALOGE("%s: gcs_close failed with status %d", __func__, status);
        wdsp_debug_dump(gcs_data.sysfs_fd);
    }

    platform_free_gcs_usecase(p_ses->vendor_uuid_info, p_gcs_ses->gcs_usecase);

    /* BG target doesn't have wdsp */
    if (!p_ses->stdev->bg_kwd) {
        status = st_hw_gcs_load_wdsp(false);
        if (status)
            rc = status;
    }

    if (p_ses->conf_levels_info) {
        free(p_ses->conf_levels_info);
        p_ses->conf_levels_info = NULL;
    }

    if (p_ses->conf_levels) {
        free(p_ses->conf_levels);
        p_ses->conf_levels = NULL;
    }

    p_ses->rc_config = NULL;

    return rc;
}

static int dereg_sm_params(st_hw_session_t *p_ses)
{
    int status = 0;
    st_hw_session_gcs_t *p_gcs_ses = (st_hw_session_gcs_t *)p_ses;

    if (p_gcs_ses->nonpersistent_cal)
        free(p_gcs_ses->nonpersistent_cal);
    p_gcs_ses->nonpersistent_cal = NULL;
    p_gcs_ses->nonpersistent_cal_size = 0;
    p_gcs_ses->start_engine_cal = NULL;

    if (p_gcs_ses->mulaw_op_buf) {
        free(p_gcs_ses->mulaw_op_buf);
        p_gcs_ses->mulaw_op_buf = NULL;
    }

    st_buffer_deinit(p_ses->buffer);
    p_ses->buffer = NULL;

    return status;
}

static int start(st_hw_session_t *p_ses)
{
    int status = 0;
    st_hw_session_gcs_t *p_gcs_ses = (st_hw_session_gcs_t *)p_ses;

    ST_DBG_DECLARE(FILE *nonpersist_fd = NULL; static int nonpersist_fd_cnt = 0);
    ST_DBG_FILE_OPEN_WR(nonpersist_fd, ST_DEBUG_DUMP_LOCATION, "nonpersist_params",
        "bin", nonpersist_fd_cnt++);
    ST_DBG_FILE_WRITE(nonpersist_fd, p_gcs_ses->nonpersistent_cal,
        p_gcs_ses->nonpersistent_cal_size);
    ST_DBG_FILE_CLOSE(nonpersist_fd);

    p_gcs_ses->exit_buffering = false;
    /* During start and stop of VA engines update enable param.
     * start_engine_cal points to part of nonpersistent_cal */
    if (p_gcs_ses->start_engine_cal) {
        p_gcs_ses->start_engine_cal->enable = 1;
        ALOGD("%s:[%d] gcs start VA engine, enable %d token %d", __func__,
              p_ses->sm_handle, p_gcs_ses->start_engine_cal->enable,
              p_gcs_ses->start_engine_cal->token);
    }

    ALOGD("%s:[%d] calling gcs_enable with handle %d, non-persist cal %p, sz %zu",
        __func__, p_ses->sm_handle, p_gcs_ses->graph_handle,
        p_gcs_ses->nonpersistent_cal, p_gcs_ses->nonpersistent_cal_size);
    ATRACE_BEGIN("sthal:gcs: gcs_enable");
    status = gcs_enable_fn(p_gcs_ses->graph_handle, p_gcs_ses->nonpersistent_cal,
        (uint32_t)p_gcs_ses->nonpersistent_cal_size);
    ATRACE_END();
    if (status) {
        ALOGE("%s: gcs_enable failed with status %d", __func__, status);
        wdsp_debug_dump(gcs_data.sysfs_fd);
    }

    return status;
}

static int restart(st_hw_session_t *p_ses,
    unsigned int recognition_mode __unused,
    bool capture_requested __unused,
    struct sound_trigger_recognition_config *rc_config,
    sound_trigger_sound_model_type_t sm_type, void *sm_data)
{
    st_hw_session_gcs_t *p_gcs_ses = (st_hw_session_gcs_t *)p_ses;
    struct graphite_cal_header restart_cfg;
    int status = 0;

    p_gcs_ses->exit_buffering = false;

    if (p_gcs_ses->gcs_usecase->params[RESTART_ENGINE].module_id &&
        p_gcs_ses->gcs_usecase->params[RESTART_ENGINE].param_id) {
        restart_cfg.module_id = p_gcs_ses->gcs_usecase->
            params[LOAD_SOUND_MODEL].module_id;
        restart_cfg.instance_id = p_gcs_ses->gcs_usecase->
            params[LOAD_SOUND_MODEL].instance_id;
        restart_cfg.reserved = 0;
        restart_cfg.param_id =
            p_gcs_ses->gcs_usecase->params[RESTART_ENGINE].param_id;
        restart_cfg.size = 0;
        ALOGD("%s:[%d] gcs restart engine", __func__, p_ses->sm_handle);
        ATRACE_BEGIN("sthal:gcs: gcs_set_config");
        status = gcs_set_config_fn(p_gcs_ses->graph_handle, &restart_cfg,
                                   sizeof(restart_cfg));
        ATRACE_END();
        if (status) {
            ALOGE("%s: gcs_set_cfg failed status %d", __func__, status);
            wdsp_debug_dump(gcs_data.sysfs_fd);
        }
        return status;
    } else {
        status = stop(p_ses);
        if (status) {
            ALOGE("%s: failed to stop err %d", __func__, status);
            goto cleanup_reg_sm_params;
        }

        status = dereg_sm_params(p_ses);
        if (status) {
            ALOGE("%s: failed to dereg_sm_params err %d", __func__, status);
            goto exit;
        }

        status = reg_sm_params(p_ses, recognition_mode, capture_requested,
            rc_config, sm_type, sm_data);
        if (status) {
            ALOGE("%s: failed to reg_sm_params err %d", __func__, status);
            goto exit;
        }

        status = start(p_ses);
        if (status) {
            ALOGE("%s: failed to start err %d", __func__, status);
            goto cleanup_start;
        }

        return status;

    cleanup_start:
        stop(p_ses);
    cleanup_reg_sm_params:
        dereg_sm_params(p_ses);
    exit:
        return status;
    }
}

static int stop(st_hw_session_t *p_ses)
{
    int status = 0;
    st_hw_session_gcs_t *p_gcs_ses = (st_hw_session_gcs_t *)p_ses;

    /* Send stop to the VA engine if other VA engines are contained in the
     * same module. This way only the specific VA engine in a VA module can
     * be stopped leaving the other VA engines active.
     */
    if (p_gcs_ses->start_engine_cal) {
        p_gcs_ses->start_engine_cal->enable = 0;
        ALOGD("%s:[%d] gcs stop VA engine, size=%zu", __func__,
              p_ses->sm_handle, p_gcs_ses->start_engine_cal_size);
        ATRACE_BEGIN("sthal:gcs: gcs_set_config");
        status = gcs_set_config_fn(p_gcs_ses->graph_handle,
                                   p_gcs_ses->start_engine_cal,
                                   p_gcs_ses->start_engine_cal_size);
        ATRACE_END();
        if (status) {
            ALOGE("%s: gcs_set_config failed status %d", __func__, status);
            wdsp_debug_dump(gcs_data.sysfs_fd);
        }
    }

    ALOGD("%s:[%d] calling gcs_disable with handle %d", __func__,
        p_ses->sm_handle, p_gcs_ses->graph_handle);
    ATRACE_BEGIN("sthal:gcs: gcs_disable");
    status = gcs_disable_fn(p_gcs_ses->graph_handle);
    ATRACE_END();
    if (status) {
        ALOGE("%s: gcs_disable failed with status %d", __func__, status);
        wdsp_debug_dump(gcs_data.sysfs_fd);
    }

    return status;
}

static int stop_buffering(st_hw_session_t *p_ses,
    bool capture_requested)
{
    int status = 0;
    st_hw_session_gcs_t *p_hw_ses = (st_hw_session_gcs_t *)p_ses;
    struct timespec tspec;
    struct listnode *node = NULL, *tmp_node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;

    if (p_ses->enable_second_stage) {
        list_for_each_safe(node, tmp_node, p_ses->second_stage_list) {
            st_sec_stage = node_to_item(node, st_arm_second_stage_t,
                list_node);
            pthread_mutex_lock(&st_sec_stage->ss_session->lock);
            st_sec_stage->ss_session->exit_buffering = true;
            pthread_cond_signal(&st_sec_stage->ss_session->cond);
            pthread_mutex_unlock(&st_sec_stage->ss_session->lock);
        }
    }

    if (capture_requested) {
        /* signal stop of bufferinng */
        ALOGV("%s: acquirung lock", __func__);
        pthread_mutex_lock(&p_hw_ses->lock);
        p_hw_ses->exit_buffering = true;
        ALOGV("%s: signaling cond", __func__);
        pthread_cond_broadcast(&p_hw_ses->cond);

        /* wait for buffering thread to exit */
        while (p_hw_ses->lab_processing_active) {
            GET_WAIT_TIMESPEC(tspec,
                convert_bytes_to_ms((ST_GCS_READ_BUF_SIZE * 2),
                &p_ses->config) * NSECS_PER_MSEC);
            ALOGV("%s: waiting on exit cond", __func__);
            status = pthread_cond_timedwait(&p_hw_ses->cond,
                &p_hw_ses->lock, &tspec);
            ALOGV("%s: done waiting on exit cond", __func__);
            if (status) {
                ALOGE("%s: ERROR. wait timed out, ret %d", __func__, status);
                status = -status;
                break;
            }
        }
        pthread_mutex_unlock(&p_hw_ses->lock);
    }

    return status;
}

static int set_device(st_hw_session_t *p_ses, bool enable)
{
    char st_device_name[DEVICE_NAME_MAX_SIZE] = { 0 };
    int ref_cnt_idx = 0, ref_cnt = 0;
    int status = 0;
    st_device_t st_device = 0;
    audio_devices_t capture_device = 0;
    bool is_hwmad_device = false;

    is_hwmad_device = platform_stdev_is_hwmad_backend(p_ses->stdev->platform,
        p_ses->st_device, p_ses->exec_mode);

    if (enable) {
        capture_device = platform_stdev_get_capture_device(p_ses->stdev->platform);
        st_device = platform_stdev_get_device(p_ses->stdev->platform,
            p_ses->vendor_uuid_info, capture_device, p_ses->exec_mode);

        if (platform_stdev_get_device_name(p_ses->stdev->platform, p_ses->exec_mode,
                st_device, st_device_name) < 0) {
            ALOGE("%s: Invalid sound trigger device returned", __func__);
            return -EINVAL;
        }

        pthread_mutex_lock(&p_ses->stdev->ref_cnt_lock);
        ref_cnt_idx = (p_ses->exec_mode * ST_DEVICE_MAX) + st_device;
        ref_cnt = ++(p_ses->stdev->dev_ref_cnt[ref_cnt_idx]);
        if (1 == ref_cnt) {
            status = platform_stdev_send_calibration(p_ses->stdev->platform,
                capture_device,
                p_ses->exec_mode,
                p_ses->vendor_uuid_info,
                ACDB_LSM_APP_TYPE_NO_TOPOLOGY,
                false, ST_DEVICE_CAL);

            if (!status) {
                /*
                 * If no dedicated path, make sure to disable device first and then
                 * enable with conc enabled to handle below scenario:
                 * 1. STHAL and AHAL runs concurrently with conc capture enabled
                 * 2. STHAL session closed then it will not disable device as AHAL
                 * usecase is going on. Then AHAL disables device when it closes session
                 * 3. If STHAL is started again then device will not be enabled as stdev
                 * mixer value in audio route is not reset, so detections won't work
                 * 4. To fix this call audio reset then enable device
                 */
                if (!p_ses->stdev->dedicated_sva_path &&
                    p_ses->stdev->conc_capture_supported &&
                    p_ses->stdev->reset_backend &&
                    !is_hwmad_device) {
                    audio_route_reset_and_update_path(p_ses->stdev->audio_route,
                        st_device_name);
                }
                ALOGD("%s: enable device (%x) = %s", __func__, st_device,
                      st_device_name);
                ATRACE_BEGIN("sthal:gcs: audio_route_apply_and_update_path");
                audio_route_apply_and_update_path(p_ses->stdev->audio_route,
                                                  st_device_name);
                ATRACE_END();
           } else {
                ALOGE("%s: failed to send calibration %d", __func__, status);
                --(p_ses->stdev->dev_ref_cnt[ref_cnt_idx]);
            }
        }
        pthread_mutex_unlock(&p_ses->stdev->ref_cnt_lock);
        p_ses->st_device = st_device;
        p_ses->st_device_name = strdup(st_device_name);
    } else {
        if (!p_ses->st_device_name) {
            ALOGE("%s: Invalid sound trigger device name", __func__);
            return -EINVAL;
        }

        ref_cnt_idx = (p_ses->exec_mode * ST_DEVICE_MAX) + p_ses->st_device;
        pthread_mutex_lock(&p_ses->stdev->ref_cnt_lock);
        ref_cnt = p_ses->stdev->dev_ref_cnt[ref_cnt_idx];
        if (0 < ref_cnt) {
            ref_cnt = --(p_ses->stdev->dev_ref_cnt[ref_cnt_idx]);
        } else {
            ALOGV("%s: ref_cnt = %d", __func__, ref_cnt);
            pthread_mutex_unlock(&p_ses->stdev->ref_cnt_lock);
            return status;
        }

        if (0 == ref_cnt) {
            if (p_ses->stdev->reset_backend || is_hwmad_device) {
                ALOGD("%s: disable device (%x) = %s", __func__, p_ses->st_device,
                      p_ses->st_device_name);
                ATRACE_BEGIN("sthal:gcs: audio_route_reset_and_update_path");
                audio_route_reset_and_update_path(p_ses->stdev->audio_route,
                                                  p_ses->st_device_name);
                ATRACE_END();
            } else {
                ALOGD("%s: Non-hwmad device, concurrent capture on, do not disable", __func__);
            }
        }
        pthread_mutex_unlock(&p_ses->stdev->ref_cnt_lock);
        free(p_ses->st_device_name);
    }
    return status;
}

static int disable_device(st_hw_session_t *p_ses, bool setting_device)
{
    int status = 0, rc = 0;
    st_hw_session_gcs_t *p_gcs_ses = (st_hw_session_gcs_t *)p_ses;

    ALOGD("%s:[%d] calling gcs_disable_device with handle %d", __func__,
        p_ses->sm_handle, p_gcs_ses->graph_handle);
    ATRACE_BEGIN("sthal:gcs: gcs_disable_device");
    status = gcs_disable_device_fn(p_gcs_ses->graph_handle);
    ATRACE_END();
    if (status) {
        ALOGE("%s: gcs_disable_device failed status %d", __func__, status);
        wdsp_debug_dump(gcs_data.sysfs_fd);
        rc = status;
    }

    if (setting_device) {
        status = set_device(p_ses, false);
        if (status) {
            ALOGE("%s: set_device disable failed status %d", __func__, status);
            rc = status;
        }
    }

    return rc;
}

static int enable_device(st_hw_session_t *p_ses, bool setting_device)
{
    int status = 0, acdb_id = 0;
    st_hw_session_gcs_t *p_gcs_ses = (st_hw_session_gcs_t *)p_ses;
    st_device_t st_device;
    audio_devices_t capture_device;
    char st_device_name[DEVICE_NAME_MAX_SIZE] = { 0 };

    if (setting_device) {
        status = set_device(p_ses, true);
        if (status) {
            ALOGE("%s: set_device enable failed status %d", __func__, status);
            return status;
        }
        st_device = p_ses->st_device;
    } else {
        capture_device = platform_stdev_get_capture_device(p_ses->stdev->platform);
        st_device = platform_stdev_get_device(p_ses->stdev->platform,
            p_ses->vendor_uuid_info, capture_device, p_ses->exec_mode);

        if (platform_stdev_get_device_name(p_ses->stdev->platform, p_ses->exec_mode,
            st_device, st_device_name) < 0) {
            ALOGE("%s: Invalid sound trigger device returned", __func__);
            return -EINVAL;
        }
    }

    acdb_id = platform_stdev_get_acdb_id(st_device, p_ses->exec_mode);
    if (acdb_id < 0) {
        status = -EINVAL;
        goto exit;
    }

    ALOGD("%s:[%d] calling gcs_enable_device with handle %d, acdb_id %d",
          __func__, p_ses->sm_handle, p_gcs_ses->graph_handle, acdb_id);
    ATRACE_BEGIN("sthal:gcs: gcs_enable_device");
    status = gcs_enable_device_fn(p_gcs_ses->graph_handle, acdb_id, NULL, 0);
    ATRACE_END();
    if (status) {
        ALOGE("%s: gcs_enable_device failed status %d", __func__, status);
        wdsp_debug_dump(gcs_data.sysfs_fd);
        goto exit;
    }
    return status;

exit:
    if (setting_device)
        set_device(p_ses, false);
    return status;
}

static void adjust_ss_buff_end(st_hw_session_t *p_ses,
    uint32_t cnn_append_bytes, uint32_t vop_append_bytes)
{
    st_hw_session_gcs_t *p_hw_ses = (st_hw_session_gcs_t *)p_ses;
    struct listnode *node = NULL, *tmp_node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;

    list_for_each_safe(node, tmp_node, p_ses->second_stage_list) {
        st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);

        pthread_mutex_lock(&st_sec_stage->ss_session->lock);
        if ((st_sec_stage->ss_info->sm_detection_type ==
             ST_SM_TYPE_KEYWORD_DETECTION) &&
            ((p_hw_ses->bytes_written + cnn_append_bytes) <
             st_sec_stage->ss_session->buf_end)) {
            ALOGV("%s: Adjusting CNN buf_end from %d to %d bytes", __func__,
                st_sec_stage->ss_session->buf_end,
                p_hw_ses->bytes_written + cnn_append_bytes);
            st_sec_stage->ss_session->buf_end = p_hw_ses->bytes_written +
                cnn_append_bytes;
        } else if ((st_sec_stage->ss_info->sm_detection_type ==
                    ST_SM_TYPE_USER_VERIFICATION) &&
                   ((p_hw_ses->bytes_written + vop_append_bytes) <
                    st_sec_stage->ss_session->buff_sz)) {
            ALOGV("%s: Adjusting VOP buff_sz from %d to %d bytes", __func__,
                st_sec_stage->ss_session->buff_sz,
                (p_hw_ses->bytes_written + vop_append_bytes));
            st_sec_stage->ss_session->buff_sz =
                (p_hw_ses->bytes_written + vop_append_bytes);
        }
        pthread_mutex_unlock(&st_sec_stage->ss_session->lock);
    }
}

static void process_lab_capture(st_hw_session_t *p_ses)
{
    int status = 0;
    st_hw_session_gcs_t *p_hw_ses = (st_hw_session_gcs_t *)p_ses;
    struct gcs_data_cmd_t read_cmd;
    bool dbg_trace_lab_read_en = true, real_time_check = false;
    struct listnode *node = NULL, *tmp_node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;
    uint32_t prepend_bytes = 0, cnn_append_bytes = 0, vop_append_bytes = 0;
    uint32_t kw_duration_bytes = 0;
    uint64_t frame_read_time = 0;

    dbg_trace_lab_buf_cnt = 0;

    read_cmd.hdr.cmd_id = p_hw_ses->gcs_usecase->params[READ_REQ].param_id;
    read_cmd.hdr.module_id = p_hw_ses->gcs_usecase->params[READ_REQ].module_id;
    read_cmd.hdr.instance_id = p_hw_ses->gcs_usecase->params[READ_REQ].instance_id;
    read_cmd.hdr.reserved = 0;
    read_cmd.hdr.token = 0;
    read_cmd.hdr.size_in_bytes = sizeof(struct gcs_cmd_read_payload_t);

    read_cmd.payload.read.size_in_bytes = ST_GCS_READ_BUF_SIZE;

    ST_DBG_DECLARE(FILE *read_fp = NULL; static int read_fp_cnt = 0);
    ST_DBG_FILE_OPEN_WR(read_fp, ST_DEBUG_DUMP_LOCATION, "read_msg", "bin", read_fp_cnt++);
    ST_DBG_FILE_WRITE(read_fp, (char *)&read_cmd, sizeof(struct graphite_data_cmd_hdr) +
        sizeof(struct gcs_cmd_read_payload_t));
    ST_DBG_FILE_CLOSE(read_fp);

    ALOGV("%s:[%d] read_cmd module_id 0x%x, instance_id 0x%x, cmd_id 0x%x, "
        "read_sz %d", __func__, p_ses->sm_handle, read_cmd.hdr.module_id,
        read_cmd.hdr.instance_id, read_cmd.hdr.cmd_id,
        read_cmd.payload.read.size_in_bytes);

    pthread_mutex_lock(&p_hw_ses->lock);
    /*
     * Check needed in-case the client called stop_buffering before we came
     * here
     */
    if (p_hw_ses->exit_buffering) {
        pthread_mutex_unlock(&p_hw_ses->lock);
        return;
    }

    p_hw_ses->read_rsp_cnt = GCS_CONCURRENT_READS_CNT;
    p_hw_ses->lab_processing_active = true;
    p_hw_ses->unread_bytes = 0;
    p_hw_ses->bytes_written = 0;
    p_hw_ses->frame_send_time = 0;
    p_hw_ses->frame_receive_time = 0;

    st_buffer_reset(p_ses->buffer);

    if (p_ses->enable_second_stage) {
        prepend_bytes =
            convert_ms_to_bytes(p_ses->vendor_uuid_info->kw_start_tolerance,
                &p_ses->config);
        if (p_ses->client_req_hist_buf) {
            kw_duration_bytes =
                convert_ms_to_bytes(p_ses->client_req_hist_buf, &p_ses->config);
        } else {
            kw_duration_bytes =
                convert_ms_to_bytes(p_ses->vendor_uuid_info->kw_duration,
                    &p_ses->config);
        }

        list_for_each_safe(node, tmp_node, p_ses->second_stage_list) {
            st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
            /*
             * At the start of buffering, initialize the variables needed by the
             * second stage sessions.
             */
            pthread_mutex_lock(&st_sec_stage->ss_session->lock);
            /*
             * In the generic detection event usecase, the start of the buffer
             * sent to 2nd stage is determined by the 1st stage keyword start
             * index. This index can have some error, so the start of the buffer
             * is moved forward to ensure there are no resulting missed
             * detections. Similarly, error tolerance is added to the end of the
             * buffer for generic and non generic detection event usecases.
             */
            if (p_ses->kw_start_idx > prepend_bytes) {
                st_sec_stage->ss_session->buf_start =
                    p_ses->kw_start_idx - prepend_bytes;
            } else {
                st_sec_stage->ss_session->buf_start = 0;
            }

            if (st_sec_stage->ss_info->sm_detection_type ==
                ST_SM_TYPE_KEYWORD_DETECTION) {
                cnn_append_bytes =
                    convert_ms_to_bytes((p_ses->vendor_uuid_info->kw_end_tolerance +
                        st_sec_stage->ss_info->data_after_kw_end), &p_ses->config);

                if (p_ses->kw_end_idx < kw_duration_bytes) {
                    st_sec_stage->ss_session->buf_end = p_ses->kw_end_idx +
                        cnn_append_bytes;
                } else {
                    st_sec_stage->ss_session->buf_end = kw_duration_bytes +
                        cnn_append_bytes;
                }
                st_sec_stage->ss_session->buff_sz = ST_GCS_READ_BUF_SIZE;
                st_sec_stage->ss_session->det_status = KEYWORD_DETECTION_PENDING;
            } else if (st_sec_stage->ss_info->sm_detection_type ==
                ST_SM_TYPE_USER_VERIFICATION) {

                vop_append_bytes =
                    convert_ms_to_bytes(p_ses->vendor_uuid_info->kw_end_tolerance,
                        &p_ses->config);

                if ((p_ses->kw_end_idx + vop_append_bytes) < kw_duration_bytes) {
                    st_sec_stage->ss_session->buf_end = p_ses->kw_end_idx +
                        vop_append_bytes;
                } else {
                    st_sec_stage->ss_session->buf_end = kw_duration_bytes;
                }

                st_sec_stage->ss_session->buff_sz = (st_sec_stage->ss_session->buf_end -
                    st_sec_stage->ss_session->buf_start);
                st_sec_stage->ss_session->det_status = USER_VERIFICATION_PENDING;
            }
            st_sec_stage->ss_session->unread_bytes = 0;
            st_sec_stage->ss_session->exit_buffering = false;
            st_sec_stage->ss_session->bytes_processed = 0;
            st_sec_stage->ss_session->start_processing = false;
            pthread_mutex_unlock(&st_sec_stage->ss_session->lock);
        }

        if (p_ses->enable_second_stage && !p_ses->client_req_hist_buf)
            p_hw_ses->move_client_ptr = true;
        else
            p_hw_ses->move_client_ptr = false;

        if (!p_ses->is_generic_event)
            real_time_check = true;
    }

    ALOGD("%s: issuing gcs_start_buff_xfer", __func__);
    ATRACE_BEGIN("sthal:gcs: gcs_start_buff_xfer");
    status = gcs_start_buff_xfer_fn(p_hw_ses->graph_handle, GCS_XFER_READ);
    ATRACE_END();
    if (status) {
        ALOGE("%s: failed to start buffer xfer, err %d", __func__, status);
        wdsp_debug_dump(gcs_data.sysfs_fd);
        pthread_mutex_unlock(&p_hw_ses->lock);
        return;
    }

    ST_DBG_FILE_OPEN_WR(p_hw_ses->lab_fp_gcs, ST_DEBUG_DUMP_LOCATION, "lab_gcs_to_sthal",
        "bin", lab_fp_gcs_cnt++);
    ST_DBG_FILE_OPEN_WR(p_hw_ses->lab_fp_client, ST_DEBUG_DUMP_LOCATION, "lab_sthal_to_client",
        "bin", lab_fp_client_cnt++);

    while (p_hw_ses->read_rsp_cnt && !p_hw_ses->exit_buffering) {

        pthread_mutex_unlock(&p_hw_ses->lock);
        ALOGVV("%s: issuing read cmd to gcs", __func__);
        ST_DBG_ATRACE_ASYNC_BEGIN_IF(dbg_trace_lab_read_en,"sthal:gcs: gcs_buffer_read",
                                    p_hw_ses->common.sm_handle);
        dbg_trace_lab_read_en = false;
        p_hw_ses->frame_send_time = get_current_time_ns();
        status = gcs_send_data_cmd_fn(p_hw_ses->graph_handle, (int8_t *)&read_cmd,
                sizeof(struct graphite_data_cmd_hdr) +
                sizeof(struct gcs_cmd_read_payload_t));
        pthread_mutex_lock(&p_hw_ses->lock);

        /* break from loop in case read failed or exit requested */
        if (status) {
            ALOGE("%s: failed to send read cmd, err %d", __func__, status);
            wdsp_debug_dump(gcs_data.sysfs_fd);
            break;
        }

        if (p_hw_ses->exit_buffering)
            break;

        /* wait for read response */
        if (--p_hw_ses->read_rsp_cnt == 0) {
            ALOGVV("%s: waiting on cond", __func__);
            pthread_cond_wait(&p_hw_ses->cond, &p_hw_ses->lock);
            ALOGVV("%s: ended wait on cond", __func__);
            frame_read_time = (p_hw_ses->frame_receive_time -
                p_hw_ses->frame_send_time);
            if (real_time_check && (frame_read_time >
                GCS_MAX_LAB_FTRT_FRAME_RD_TIME_NS) &&
                !p_hw_ses->exit_buffering) {
                uint32_t bytes_written_ms =
                    convert_bytes_to_ms(p_hw_ses->bytes_written,
                        &p_ses->config);

                ALOGD("%s: Real time frame received after %dms took %llums",
                    __func__, bytes_written_ms,
                    (frame_read_time / NSECS_PER_MSEC));
                adjust_ss_buff_end(p_ses, cnn_append_bytes,
                    vop_append_bytes);
                real_time_check = false;
            }
        }
    }
    ALOGV("%s: exited buffering loop ", __func__);
    pthread_mutex_unlock(&p_hw_ses->lock);

    /*
     * Below call will cause the DSP to flush any pending reads and
     * return a read rsp, this read rsp will be ignored in the callback
     * because the exit_buffering flag is set
     */
    ALOGD("%s:[%d] calling gcs_stop_buff_xfer ", __func__, p_ses->sm_handle);
    ATRACE_BEGIN("sthal:gcs: gcs_stop_buff_xfer");
    status = gcs_stop_buff_xfer_fn(p_hw_ses->graph_handle, GCS_XFER_READ);
    ATRACE_END();
    if (status) {
        ALOGE("%s: failed to stop buffer xfer, err %d", __func__, status);
        wdsp_debug_dump(gcs_data.sysfs_fd);
    }

    ST_DBG_FILE_CLOSE(p_hw_ses->lab_fp_gcs);
    ST_DBG_FILE_CLOSE(p_hw_ses->lab_fp_client);

    /*
     * Signal back to thread calling stop_buffering that
     * buffering has exited
     */
    pthread_mutex_lock(&p_hw_ses->lock);
    p_hw_ses->lab_processing_active = false;
    pthread_cond_signal(&p_hw_ses->cond);
    pthread_mutex_unlock(&p_hw_ses->lock);
    return;
}

static int read_pcm(st_hw_session_t *p_ses,
    unsigned char *client_buf,
    unsigned int bytes)
{
    int status = 0;
    unsigned int requested_bytes = 0;
    st_hw_session_gcs_t *p_gcs_ses = (st_hw_session_gcs_t *)p_ses;
    struct timespec tspec;
    uint32_t copy_bytes = 0, move_bytes = 0, delay_bytes = 0;

    ALOGV("%s: Enter, bytes requested %d", __func__, bytes);

    pthread_mutex_lock(&p_gcs_ses->lock);

    if (p_gcs_ses->move_client_ptr) {
        /*
         * This logic is needed if LAB was enabled due to second stage being enabled,
         * but the client did not request LAB. The client read pointer will be shifted
         * to the keyword end index in this usecase.
         */
        delay_bytes = p_ses->kw_end_idx;
        move_bytes = MIN(delay_bytes, p_gcs_ses->unread_bytes);
        ALOGD("%s: Moving client ptr by %d bytes", __func__, move_bytes);
        st_buffer_flush(p_ses->buffer, move_bytes);
        p_gcs_ses->unread_bytes -= move_bytes;
        p_gcs_ses->move_client_ptr = false;
    }

    requested_bytes = bytes;
    while (bytes > 0 && !p_gcs_ses->exit_buffering) {

        if (0 == p_gcs_ses->unread_bytes) {
            /* not enough data, block and wait for more data */
            GET_WAIT_TIMESPEC(tspec, convert_bytes_to_ms((requested_bytes * 2),
                &p_ses->config) * NSECS_PER_MSEC);
            ALOGVV("%s: waiting on cond, bytes=%d", __func__, bytes);
            status = pthread_cond_timedwait(&p_gcs_ses->cond,
                &p_gcs_ses->lock, &tspec);
            ALOGVV("%s: done waiting on cond, bytes=%d", __func__, bytes);
            if (status) {
                ALOGE("%s: ERROR. read wait timed out, ret %d", __func__,
                      status);
                p_gcs_ses->exit_buffering = true;
                status = -status;
                break;
            }
            if (p_gcs_ses->exit_buffering) {
                ALOGV("%s: exiting buffering ", __func__);
                status = -EIO;
                break;
            }
        }

        if (p_gcs_ses->unread_bytes > 0) {

            copy_bytes = MIN(p_gcs_ses->unread_bytes, bytes);
            status = st_buffer_read(p_ses->buffer, client_buf, copy_bytes,
                NULL, true);
            if (status) {
                ALOGE("%s: st_buffer_read failed, status %d", __func__,
                    status);
                goto exit;
            }

            p_gcs_ses->unread_bytes -= copy_bytes;

            ST_DBG_FILE_WRITE(p_gcs_ses->lab_fp_client, client_buf, copy_bytes);

            bytes -= copy_bytes;
            client_buf += copy_bytes;
        }
    }

exit:
    pthread_mutex_unlock(&p_gcs_ses->lock);

    ALOGVV("%s: Exit...", __func__);
    return status;
}

static int send_detection_request(st_hw_session_t *p_ses)
{
    int status = 0, cal_hdr_size = sizeof(struct graphite_cal_header);
    st_hw_session_gcs_t *p_gcs_ses = (st_hw_session_gcs_t *)p_ses;
    struct graphite_cal_header cal_hdr = {0};

    cal_hdr.module_id = p_gcs_ses->gcs_usecase->
        params[REQUEST_DETECTION].module_id;
    cal_hdr.instance_id = p_gcs_ses->gcs_usecase->
        params[REQUEST_DETECTION].instance_id;
    cal_hdr.param_id = p_gcs_ses->gcs_usecase->
        params[REQUEST_DETECTION].param_id;
    cal_hdr.size = 0;

    ST_DBG_DECLARE(FILE *req_event_fd = NULL; static int req_event_cnt = 0);
    ST_DBG_FILE_OPEN_WR(req_event_fd, ST_DEBUG_DUMP_LOCATION,
        "requested_event_gcs", "bin", req_event_cnt++);
    ST_DBG_FILE_WRITE(req_event_fd, &cal_hdr, cal_hdr_size);
    ST_DBG_FILE_CLOSE(req_event_fd);

    ALOGD("%s:[%d] calling gcs_set_config with graph_handle %d, msg_size %d",
        __func__, p_ses->sm_handle, p_gcs_ses->graph_handle, cal_hdr_size);
    ATRACE_BEGIN("sthal:gcs: gcs_set_config");
    status = gcs_set_config_fn(p_gcs_ses->graph_handle, &cal_hdr,
        cal_hdr_size);
    ATRACE_END();
    if (status) {
        status = -EINVAL;
        ALOGE("%s: gcs_set_config failed with status %d", __func__, status);
        wdsp_debug_dump(gcs_data.sysfs_fd);
    }

    return status;
}

int st_hw_sess_gcs_init(st_hw_session_t *const p_ses,
    hw_ses_event_callback_t cb,
    void *cookie, st_exec_mode_t exec_mode,
    struct st_vendor_info *v_info,
    sound_model_handle_t sm_handle,
    sound_trigger_device_t *stdev)
{
    int status = 0;
    pthread_attr_t attr;
    pthread_condattr_t cond_attr, cond_attr2;
    st_hw_session_gcs_t *p_hw_ses = (st_hw_session_gcs_t *)p_ses;

    if (!v_info) {
        ALOGE("%s: received null v_info", __func__);
        return -EINVAL;
    }

    ALOGV("%s:[%d] Enter, exec_mode %d, callback %p, cookie %p",
        __func__, sm_handle, exec_mode, cb, cookie);

    p_ses->exec_mode = exec_mode;
    p_ses->callback_to_st_session = cb;
    p_ses->cookie = cookie;
    p_ses->vendor_uuid_info = v_info;
    p_ses->sm_handle = sm_handle;
    p_ses->fptrs = &fptrs_gcs;
    p_ses->stdev = stdev;
    p_ses->is_generic_event = false;
    p_ses->client_req_det_mode = ST_HW_SESS_DET_UNKNOWN_MODE;
    p_hw_ses->nonpersistent_cal = NULL;
    p_hw_ses->detect_payload_size = 0;
    p_hw_ses->detection_signaled = false;
    p_hw_ses->exit_detection = false;
    p_hw_ses->exit_buffering = false;
    p_hw_ses->lab_processing_active = false;
    p_hw_ses->lab_fp_gcs = NULL;
    p_hw_ses->lab_fp_client = NULL;
    pthread_condattr_init(&cond_attr);
    pthread_condattr_setclock(&cond_attr, CLOCK_MONOTONIC);
    pthread_cond_init(&p_hw_ses->callback_thread_cond, &cond_attr);
    pthread_condattr_destroy(&cond_attr);

    pthread_mutex_init(&p_hw_ses->callback_thread_lock, NULL);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    status = pthread_create(&p_hw_ses->callback_thread, &attr,
        callback_thread_loop, p_ses);
    if (status) {
        ALOGE("%s failed to create cb thread, error %d", __func__, status);
        goto error;
    }
    pthread_attr_destroy(&attr);

    /* initialize lab transfer objects */
    pthread_mutex_init(&p_hw_ses->lock, NULL);

    pthread_condattr_init(&cond_attr2);
    pthread_condattr_setclock(&cond_attr2, CLOCK_MONOTONIC);
    pthread_cond_init(&p_hw_ses->cond, &cond_attr2);
    pthread_condattr_destroy(&cond_attr2);

    return 0;

error:
    pthread_attr_destroy(&attr);
    pthread_cond_destroy(&p_hw_ses->callback_thread_cond);
    pthread_mutex_destroy(&p_hw_ses->callback_thread_lock);
    pthread_condattr_destroy(&cond_attr);
    pthread_condattr_destroy(&cond_attr2);

    return status;
}

void st_hw_sess_gcs_deinit(st_hw_session_t *const p_ses)
{
    st_hw_session_gcs_t *p_hw_sess = (st_hw_session_gcs_t *)p_ses;

    /* signal detection callback thread to exit */
    pthread_mutex_lock(&p_hw_sess->callback_thread_lock);
    p_hw_sess->exit_detection = true;
    pthread_cond_signal(&p_hw_sess->callback_thread_cond);
    pthread_mutex_unlock(&p_hw_sess->callback_thread_lock);

    ALOGV("%s: waiting for callback thread to join", __func__);
    pthread_join(p_hw_sess->callback_thread, (void **)NULL);
    ALOGV("%s: callback thread exited", __func__);

    pthread_cond_destroy(&p_hw_sess->callback_thread_cond);
    pthread_mutex_destroy(&p_hw_sess->callback_thread_lock);
    pthread_mutex_destroy(&p_hw_sess->lock);
    pthread_cond_destroy(&p_hw_sess->cond);
}

int st_hw_gcs_init(void)
{
    int status = 0;

    /* load GCS library */
    gcs_data.lib_handle = dlopen(GCS_LIB, RTLD_NOW);
    if (!gcs_data.lib_handle) {
        ALOGE("%s: Unable to open %s, error %s", __func__, GCS_LIB,
            dlerror());
        status = -ENOENT;
        goto exit;
    }

    DLSYM(gcs_data.lib_handle, gcs_init_fn, gcs_init, status);
    if (status)
        goto exit;
    DLSYM(gcs_data.lib_handle, gcs_deinit_fn, gcs_deinit, status);
    if (status)
        goto exit;
    DLSYM(gcs_data.lib_handle, gcs_open_fn, gcs_open, status);
    if (status)
        goto exit;
    DLSYM(gcs_data.lib_handle, gcs_close_fn, gcs_close, status);
    if (status)
        goto exit;
    DLSYM(gcs_data.lib_handle, gcs_load_data_fn, gcs_load_data, status);
    if (status)
        goto exit;
    DLSYM(gcs_data.lib_handle, gcs_unload_data_fn, gcs_unload_data, status);
    if (status)
        goto exit;
    DLSYM(gcs_data.lib_handle, gcs_enable_fn, gcs_enable, status);
    if (status)
        goto exit;
    DLSYM(gcs_data.lib_handle, gcs_disable_fn, gcs_disable, status);
    if (status)
        goto exit;
    DLSYM(gcs_data.lib_handle, gcs_register_for_event_fn,
          gcs_register_for_event, status);
    if (status)
        goto exit;
    DLSYM(gcs_data.lib_handle, gcs_register_data_cmd_handler_fn,
          gcs_register_data_cmd_handler, status);
    if (status)
        goto exit;
    DLSYM(gcs_data.lib_handle, gcs_start_buff_xfer_fn, gcs_start_buff_xfer,
          status);
    if (status)
        goto exit;
    DLSYM(gcs_data.lib_handle, gcs_stop_buff_xfer_fn, gcs_stop_buff_xfer,
          status);
    if (status)
        goto exit;
    DLSYM(gcs_data.lib_handle, gcs_send_data_cmd_fn, gcs_send_data_cmd, status);
    if (status)
        goto exit;
    DLSYM(gcs_data.lib_handle, gcs_enable_device_fn, gcs_enable_device, status);
    if (status)
        goto exit;
    DLSYM(gcs_data.lib_handle, gcs_disable_device_fn, gcs_disable_device,
          status);
    if (status)
        goto exit;
    DLSYM(gcs_data.lib_handle, gcs_set_config_fn, gcs_set_config, status);
    if (status)
        goto exit;

    /* initialize GCS */
    ALOGD("%s: calling gcs_init", __func__);
    ATRACE_BEGIN("sthal:gcs: gcs_init");
    status = gcs_init_fn();
    ATRACE_END();
    if (status) {
        ALOGE("%s: gcs_init failed with status %d", __func__, status);
        wdsp_debug_dump(gcs_data.sysfs_fd);
        goto exit;
    }

    /* open handle used to load WDSP image */
    gcs_data.sysfs_fd = open(WDSP_SYSFS_NAME, O_WRONLY);
    if (gcs_data.sysfs_fd < 0) {
        ALOGW("%s: Failed to open %s with error: %s open will retried",
            __func__, WDSP_SYSFS_NAME, strerror(errno));
        wdsp_debug_dump(gcs_data.sysfs_fd);
    }

    return status;

exit:
    if (gcs_data.lib_handle)
        dlclose(gcs_data.lib_handle);
    return status;
}

void st_hw_gcs_deinit(void)
{
    if (gcs_data.sysfs_fd >= 0)
        close(gcs_data.sysfs_fd);
    ALOGD("%s: calling gcs_deinit", __func__);
    ATRACE_BEGIN("sthal:gcs: gcs_deinit");
    gcs_deinit_fn();
    ATRACE_END();
    if (gcs_data.lib_handle)
        dlclose(gcs_data.lib_handle);
}

/*
 * Both current gcs session and LMPA extension call this API
 * with stdev lock held.
 */
int st_hw_gcs_load_wdsp(bool load)
{
    int status = 0;

    ALOGV("%s: %d", __func__, load);
    if (load) {
        /* load WDSP image */
        if (gcs_data.sysfs_fd >= 0) {
            ATRACE_BEGIN("sthal:gcs: load WDSP image");
            write(gcs_data.sysfs_fd, "1", 2);
            ATRACE_END();
        } else {
            ALOGE("%s: sysfs fd invalid %d", __func__, gcs_data.sysfs_fd);
            wdsp_debug_dump(gcs_data.sysfs_fd);
            status = -EIO;
        }
    } else {
        /* Unload WDSP image */
        if (gcs_data.sysfs_fd >= 0) {
            ATRACE_BEGIN("sthal:gcs: unload WDSP image");
            write(gcs_data.sysfs_fd, "0", 2);
            ATRACE_END();
        } else {
            ALOGE("%s: sysfs fd invalid %d ", __func__, gcs_data.sysfs_fd);
            wdsp_debug_dump(gcs_data.sysfs_fd);
            status = -EIO;
        }
    }
    ALOGV("%s: exit status %d", __func__, status);
    return status;
}
