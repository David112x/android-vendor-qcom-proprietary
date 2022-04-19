/*
 * This file contrains the thread for a single second stage sound
 * trigger user session. This thread implements logic for passing
 * buffered data to various modules on ARM via capi wrappers. It also
 * retrieves the detection results via capi wrappers and notifies the
 * sound trigger state machine.
 *
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "sound_trigger_hw"
#define ATRACE_TAG (ATRACE_TAG_HAL)
/* #define LOG_NDEBUG 0 */
#define LOG_NDDEBUG 0

#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <cutils/trace.h>
#include <system/thread_defs.h>
#include <hardware/sound_trigger.h>

#include "st_second_stage.h"
#include "st_common_defs.h"
#include "sound_trigger_platform.h"
#include "sound_trigger_hw.h"
#include "st_session.h"
#include "st_hw_common.h"
#include "capi_v2_extn.h"
#include "st_hw_session_gcs.h"

ST_DBG_DECLARE(static int ss_fd_cnt_kw_det = 0);
ST_DBG_DECLARE(static int ss_fd_cnt_user_ver = 0);

static int process_frame_keyword_detection(st_arm_ss_session_t *ss_session,
    uint8_t *frame, capi_v2_stream_data_t *stream_input,
    sva_result_t *result_cfg_ptr, unsigned int *det_status)
{
    capi_v2_buf_t capi_result_ptr;
    capi_v2_err_t rc;
    int ret = 0;

    stream_input->bufs_num = 1; /* 1 channel */
    stream_input->buf_ptr->max_data_len = ss_session->buff_sz;
    stream_input->buf_ptr->actual_data_len = ss_session->buff_sz;
    stream_input->buf_ptr->data_ptr = (int8_t *)frame;

    ALOGV("%s: Issuing capi_process", __func__);
    ATRACE_BEGIN("sthal:second_stage: process keyword detection (CNN)");
    rc = ss_session->capi_handle->vtbl_ptr->process(ss_session->capi_handle,
        &stream_input, NULL);
    ATRACE_END();
    if (CAPI_V2_EFAILED == rc) {
        ALOGD("%s: capi process function failed, result = %d", __func__, rc);
        ret = -EINVAL;
        goto exit;
    }

    capi_result_ptr.data_ptr = (int8_t *)result_cfg_ptr;
    capi_result_ptr.actual_data_len = sizeof(sva_result_t);
    capi_result_ptr.max_data_len = sizeof(sva_result_t);

    ALOGV("%s: Issuing capi_get_param for param %d", __func__, SVA_ID_RESULT);
    rc = ss_session->capi_handle->vtbl_ptr->get_param(ss_session->capi_handle,
        SVA_ID_RESULT, NULL, &capi_result_ptr);
    if (CAPI_V2_EOK != rc) {
        ALOGE("%s: get_param failed, result = %d", __func__, rc);
        ret = -EINVAL;
        goto exit;
    }

    if (result_cfg_ptr->is_detected) {
        *det_status = KEYWORD_DETECTION_SUCCESS;
        ss_session->exit_buffering = true;
        ss_session->kw_start_idx = (result_cfg_ptr->start_position *
            CNN_FRAME_SIZE) + ss_session->buf_start;
        ss_session->kw_end_idx = (result_cfg_ptr->end_position *
            CNN_FRAME_SIZE) + ss_session->buf_start;
        ss_session->confidence_score = result_cfg_ptr->best_confidence;
    }

exit:
    return ret;
}

static int start_keyword_detection(st_arm_second_stage_t *st_sec_stage)
{
    struct timespec tspec;
    int ret = 0;
    uint8_t *process_input_buff = NULL;
    capi_v2_err_t rc;
    st_arm_ss_session_t *ss_session = st_sec_stage->ss_session;
    capi_v2_stream_data_t *stream_input = NULL;
    sva_result_t *result_cfg_ptr = NULL;
    unsigned int det_status = KEYWORD_DETECTION_PENDING;

    ALOGV("%s: Enter", __func__);

    process_input_buff = calloc(1, ss_session->buff_sz);
    if (!process_input_buff) {
        ALOGE("%s: failed to allocate process_input_buff", __func__);
        ret = -ENOMEM;
        goto exit;
    }

    stream_input = calloc(1, sizeof(capi_v2_stream_data_t));
    if (!stream_input) {
        ALOGE("%s: failed to allocate stream_input", __func__);
        ret = -ENOMEM;
        goto exit;
    }

    stream_input->buf_ptr = calloc(1, sizeof(capi_v2_buf_t));
    if (!stream_input->buf_ptr) {
        ALOGE("%s: failed to allocate stream_input->buf_ptr", __func__);
        ret = -ENOMEM;
        goto exit;
    }

    result_cfg_ptr = calloc(1, sizeof(sva_result_t));
    if (!result_cfg_ptr) {
        ALOGE("%s: failed to allocate result_cfg_ptr", __func__);
        ret = -ENOMEM;
        goto exit;
    }

    while (!ss_session->exit_buffering &&
           (ss_session->bytes_processed <
            (ss_session->buf_end - ss_session->buf_start))) {
        if (ss_session->unread_bytes < ss_session->buff_sz) {
            ALOGV("%s: waiting on cond, unread_bytes = %d", __func__,
                ss_session->unread_bytes);
            /* Time out to unblock read thread in case if write thread is
               stuck filling the buffers */
            clock_gettime(CLOCK_MONOTONIC, &tspec);
            tspec.tv_sec += ST_READ_WAIT_TIME_OUT_SEC;
            ret = pthread_cond_timedwait(&ss_session->cond, &ss_session->lock,
                &tspec);
            ALOGV("%s: done waiting on cond", __func__);
            if (ret) {
                ALOGE("%s: ERROR. read wait timed out, ret %d", __func__, ret);
                break;
            }
            if (ss_session->exit_buffering ||
                (ss_session->bytes_processed >=
                 (ss_session->buf_end - ss_session->buf_start))) {
                ALOGW("%s: buffering stopped while waiting on cond, exiting",
                    __func__);
                break;
            }
            if (ss_session->unread_bytes < ss_session->buff_sz)
                continue;
        }

        ret = st_buffer_read(ss_session->st_ses->hw_ses_current->buffer,
            process_input_buff, ss_session->buff_sz, &ss_session->hw_rd_ptr,
            false);
        if (ret) {
            ALOGE("%s: st_buffer_read failed, status %d", __func__, ret);
            goto exit;
        }
        ss_session->unread_bytes -= ss_session->buff_sz;

        ST_DBG_FILE_WRITE(st_sec_stage->dump_fp, process_input_buff,
            ss_session->buff_sz);
        pthread_mutex_unlock(&ss_session->lock);
        ret = process_frame_keyword_detection(ss_session, process_input_buff,
            stream_input, result_cfg_ptr, &det_status);
        pthread_mutex_lock(&ss_session->lock);
        if (ret) {
            ALOGE("%s: Processing through capi wrapper failed", __func__);
            break;
        }
        ss_session->bytes_processed += ss_session->buff_sz;
    }

exit:
    /*
     * The CNN algorithm doesn't set reject because it is continuously called
     * until the keyword has passed. So if a detection success has not been
     * declared inside the above loop, it is set to detection reject.
     */
    pthread_mutex_unlock(&ss_session->lock);
    pthread_mutex_lock(&ss_session->st_ses->ss_detections_lock);
    if (det_status == KEYWORD_DETECTION_SUCCESS) {
        uint32_t kw_start_ms = 0, kw_end_ms = 0;

        ss_session->det_status = KEYWORD_DETECTION_SUCCESS;
        kw_start_ms = convert_bytes_to_ms(ss_session->kw_start_idx,
            &ss_session->st_ses->hw_ses_current->config);
        kw_end_ms = convert_bytes_to_ms(ss_session->kw_end_idx,
            &ss_session->st_ses->hw_ses_current->config);
        ALOGD("%s: Detection success, confidence level = %d", __func__,
            ss_session->confidence_score);
        ALOGD("%s: 2nd stage kw_start = %dms, kw_end = %dms", __func__,
            kw_start_ms, kw_end_ms);
    } else {
        ss_session->det_status = KEYWORD_DETECTION_REJECT;
        ALOGD("%s: Detection reject", __func__);
    }
    /* Signal aggregator thread with detection result */
    pthread_cond_signal(&ss_session->st_ses->ss_detections_cond);
    pthread_mutex_unlock(&ss_session->st_ses->ss_detections_lock);
    pthread_mutex_lock(&ss_session->lock);

    /* Reinit the module */
    ALOGV("%s: Issuing capi_set_param for param %d", __func__,
        SVA_ID_REINIT_ALL);
    rc = ss_session->capi_handle->vtbl_ptr->set_param(ss_session->capi_handle,
        SVA_ID_REINIT_ALL, NULL, NULL);
    if (CAPI_V2_EOK != rc) {
        ret = -EINVAL;
        ALOGE("%s: Failed to reinit capi module, result = %d", __func__, rc);
    }

    if (result_cfg_ptr)
        free(result_cfg_ptr);
    if (stream_input) {
        if (stream_input->buf_ptr)
            free(stream_input->buf_ptr);
        free(stream_input);
    }
    if (process_input_buff)
        free(process_input_buff);

    ALOGV("%s: Exiting", __func__);
    return ret;
}

static int process_frame_user_verification(st_arm_ss_session_t *ss_session,
    uint8_t *frame, capi_v2_stream_data_t *stream_input,
    voiceprint2_result_t *result_cfg_ptr, unsigned int *det_status)
{
    capi_v2_buf_t capi_result_ptr;
    capi_v2_err_t rc;
    int ret = 0;

    stream_input->bufs_num = 1; /* 1 channel */
    stream_input->buf_ptr->max_data_len = ss_session->buff_sz;
    stream_input->buf_ptr->actual_data_len = ss_session->buff_sz;
    stream_input->buf_ptr->data_ptr = (int8_t *)frame;

    ALOGV("%s: Issuing capi_process", __func__);
    ATRACE_BEGIN("sthal:second_stage: process user verification (VOP)");
    rc = ss_session->capi_handle->vtbl_ptr->process(ss_session->capi_handle,
        &stream_input, NULL);
    ATRACE_END();
    if (CAPI_V2_EFAILED == rc) {
        ALOGD("%s: capi process function failed, result = %d", __func__, rc);
        ret = -EINVAL;
        goto exit;
    }

    capi_result_ptr.data_ptr = (int8_t *)result_cfg_ptr;
    capi_result_ptr.actual_data_len = sizeof(voiceprint2_result_t);
    capi_result_ptr.max_data_len = sizeof(voiceprint2_result_t);

    ALOGV("%s: Issuing capi_get_param for param %d", __func__,
        VOICEPRINT2_ID_RESULT);
    rc = ss_session->capi_handle->vtbl_ptr->get_param(ss_session->capi_handle,
        VOICEPRINT2_ID_RESULT, NULL, &capi_result_ptr);
    if (CAPI_V2_EOK != rc) {
        ALOGE("%s: get_param failed, result = %d", __func__, rc);
        ret = -EINVAL;
        goto exit;
    }

    if (result_cfg_ptr->is_detected) {
        *det_status = USER_VERIFICATION_SUCCESS;
        ss_session->confidence_score =
            (int32_t)result_cfg_ptr->combined_user_score;
    }

exit:
    return ret;
}

static int start_user_verification(st_arm_second_stage_t *st_sec_stage)
{
    struct timespec tspec;
    int ret = 0;
    uint8_t *process_input_buff = NULL;
    capi_v2_err_t rc;
    st_arm_ss_session_t *ss_session = st_sec_stage->ss_session;
    capi_v2_stream_data_t *stream_input = NULL;
    capi_v2_buf_t capi_uv_ptr;
    voiceprint2_result_t *result_cfg_ptr = NULL;
    voiceprint2_sva_uv_score_t *uv_cfg_ptr = NULL;
    unsigned int det_status = USER_VERIFICATION_PENDING;

    ALOGV("%s: Enter", __func__);

    process_input_buff = calloc(1, ss_session->buff_sz);
    if (!process_input_buff) {
        ALOGE("%s: failed to allocate process_input_buff", __func__);
        ret = -ENOMEM;
        goto exit;
    }

    stream_input = calloc(1, sizeof(capi_v2_stream_data_t));
    if (!stream_input) {
        ALOGE("%s: failed to allocate stream_input", __func__);
        ret = -ENOMEM;
        goto exit;
    }

    stream_input->buf_ptr = calloc(1, sizeof(capi_v2_buf_t));
    if (!stream_input->buf_ptr) {
        ALOGE("%s: failed to allocate stream_input->buf_ptr", __func__);
        ret = -ENOMEM;
        goto exit;
    }

    result_cfg_ptr = calloc(1, sizeof(voiceprint2_result_t));
    if (!result_cfg_ptr) {
        ALOGE("%s: failed to allocate result_cfg_ptr", __func__);
        ret = -ENOMEM;
        goto exit;
    }

    uv_cfg_ptr = calloc(1, sizeof(voiceprint2_sva_uv_score_t));
    if (!uv_cfg_ptr) {
        ALOGE("%s: failed to allocate uv_cfg_ptr", __func__);
        ret = -ENOMEM;
        goto exit;
    }

    uv_cfg_ptr->sva_uv_confidence_score =
        ss_session->st_ses->hw_ses_current->user_level;
    capi_uv_ptr.data_ptr = (int8_t *)uv_cfg_ptr;
    capi_uv_ptr.actual_data_len = sizeof(voiceprint2_sva_uv_score_t);
    capi_uv_ptr.max_data_len = sizeof(voiceprint2_sva_uv_score_t);

    ALOGV("%s: Issuing capi_set_param for param %d", __func__,
        VOICEPRINT2_ID_SVA_UV_SCORE);
    rc = ss_session->capi_handle->vtbl_ptr->set_param(ss_session->capi_handle,
        VOICEPRINT2_ID_SVA_UV_SCORE, NULL, &capi_uv_ptr);
    if (CAPI_V2_EOK != rc) {
        ALOGE("%s: set_param VOICEPRINT2_ID_SVA_UV_SCORE failed, result = %d",
            __func__, rc);
        ret = -EINVAL;
        goto exit;
    }

    while (!ss_session->exit_buffering) {
        if (ss_session->unread_bytes < ss_session->buff_sz) {
            ALOGV("%s: waiting on cond, unread_bytes = %d", __func__,
                ss_session->unread_bytes);
            /* Time out to unblock read thread in case if write thread is
               stuck filling the buffers */
            clock_gettime(CLOCK_MONOTONIC, &tspec);
            tspec.tv_sec += ST_READ_WAIT_TIME_OUT_SEC;
            ret = pthread_cond_timedwait(&ss_session->cond, &ss_session->lock,
                &tspec);
            ALOGV("%s: done waiting on cond", __func__);
            if (ret) {
                ALOGE("%s: ERROR. read wait timed out, ret %d", __func__, ret);
                break;
            }
            if (ss_session->exit_buffering) {
                ALOGW("%s: buffering stopped while waiting on cond, exiting",
                    __func__);
                break;
            }
            if (ss_session->unread_bytes < ss_session->buff_sz)
                continue;
        }

        ret = st_buffer_read(ss_session->st_ses->hw_ses_current->buffer,
            process_input_buff, ss_session->buff_sz, &ss_session->hw_rd_ptr,
            false);
        if (ret) {
            ALOGE("%s: st_buffer_read failed, status %d", __func__, ret);
            goto exit;
        }
        ss_session->unread_bytes -= ss_session->buff_sz;

        ST_DBG_FILE_WRITE(st_sec_stage->dump_fp, process_input_buff,
            ss_session->buff_sz);
        pthread_mutex_unlock(&ss_session->lock);
        ret = process_frame_user_verification(ss_session, process_input_buff,
            stream_input, result_cfg_ptr, &det_status);
        pthread_mutex_lock(&ss_session->lock);
        if (ret) {
            ALOGE("%s: Processing through capi wrapper failed", __func__);
            break;
        }
        ss_session->exit_buffering = true;
    }

exit:

    pthread_mutex_unlock(&ss_session->lock);
    pthread_mutex_lock(&ss_session->st_ses->ss_detections_lock);
    if (det_status == USER_VERIFICATION_SUCCESS) {
        ss_session->det_status = USER_VERIFICATION_SUCCESS;
        ALOGD("%s: Detection success, confidence level = %d", __func__,
            ss_session->confidence_score);
    } else {
        ss_session->det_status = USER_VERIFICATION_REJECT;
        ALOGD("%s: Detection reject", __func__);
    }
    /* Signal aggregator thread with detection result */
    pthread_cond_signal(&ss_session->st_ses->ss_detections_cond);
    pthread_mutex_unlock(&ss_session->st_ses->ss_detections_lock);
    pthread_mutex_lock(&ss_session->lock);

    /* Reinit the module */
    ALOGV("%s: Issuing capi_set_param for param %d", __func__,
        VOICEPRINT2_ID_REINIT);
    rc = ss_session->capi_handle->vtbl_ptr->set_param(ss_session->capi_handle,
        VOICEPRINT2_ID_REINIT, NULL, NULL);
    if (CAPI_V2_EOK != rc) {
        ALOGE("%s: Failed to reinit capi module, result = %d", __func__, rc);
        ret = -EINVAL;
    }

    if (result_cfg_ptr)
        free(result_cfg_ptr);
    if (uv_cfg_ptr)
        free(uv_cfg_ptr);
    if (stream_input) {
        if (stream_input->buf_ptr)
            free(stream_input->buf_ptr);
        free(stream_input);
    }
    if (process_input_buff)
        free(process_input_buff);

    ALOGV("%s: Exiting", __func__);
    return ret;
}

static void *buffer_thread_loop(void *st_second_stage)
{
    st_arm_second_stage_t *st_sec_stage;
    st_arm_ss_session_t *ss_session = NULL;

    if (st_second_stage == NULL) {
        ALOGE("%s: input is NULL, exiting", __func__);
        return NULL;
    }

    st_sec_stage = (st_arm_second_stage_t *)st_second_stage;
    if (st_sec_stage->ss_session) {
        ss_session = st_sec_stage->ss_session;
    } else {
        ALOGE("%s: st_arm_ss_session_t is NULL, exiting", __func__);
        return NULL;
    }

    pthread_mutex_lock(&ss_session->lock);
    while (!ss_session->exit_thread) {
        ss_session->exit_buffering = true;
        ALOGV("%s: waiting on cond", __func__);
        /* Wait for keyword buffer data from DSP */
        pthread_cond_wait(&ss_session->cond, &ss_session->lock);
        ALOGV("%s: done waiting on cond, exit_buffering = %d", __func__,
            ss_session->exit_buffering);
        if (ss_session->exit_thread) {
            pthread_mutex_unlock(&ss_session->lock);
            return NULL;
        }

        /*
         * If 1st stage buffering overflows before 2nd stage starts processing,
         * the below functions need to be called to reset the 1st stage session
         * for the next detection.
         */
        if (ss_session->exit_buffering &&
            (((st_sec_stage->ss_info->sm_detection_type ==
               ST_SM_TYPE_KEYWORD_DETECTION) &&
              (ss_session->det_status != KEYWORD_DETECTION_PENDING)) ||
             ((st_sec_stage->ss_info->sm_detection_type ==
               ST_SM_TYPE_USER_VERIFICATION) &&
              (ss_session->det_status != USER_VERIFICATION_PENDING)))) {
            continue;
        }

        if (st_sec_stage->ss_info->sm_detection_type ==
            ST_SM_TYPE_KEYWORD_DETECTION) {
            ST_DBG_FILE_OPEN_WR(st_sec_stage->dump_fp, ST_DEBUG_DUMP_LOCATION,
                "ss_buf_kw_det", "bin", ss_fd_cnt_kw_det++);
            start_keyword_detection(st_sec_stage);
            ST_DBG_FILE_CLOSE(st_sec_stage->dump_fp);
        } else {
            ST_DBG_FILE_OPEN_WR(st_sec_stage->dump_fp, ST_DEBUG_DUMP_LOCATION,
                "ss_buf_user_ver", "bin", ss_fd_cnt_user_ver++);
            start_user_verification(st_sec_stage);
            ST_DBG_FILE_CLOSE(st_sec_stage->dump_fp);
        }
    }
    pthread_mutex_unlock(&ss_session->lock);
    return NULL;
}

int st_second_stage_start_session(st_arm_second_stage_t *st_sec_stage)
{
    int ret = 0;
    capi_v2_err_t rc = CAPI_V2_EOK;
    int8_t *cfg_ptr_threshold = NULL;
    capi_v2_buf_t capi_buf;
    st_arm_ss_session_t *ss_session;


    if (!st_sec_stage || !st_sec_stage->ss_info || !st_sec_stage->ss_session) {
        ALOGE("%s: Receivd null params, exiting", __func__);
        return -EINVAL;
    }

    ss_session = st_sec_stage->ss_session;

    if (st_sec_stage->ss_info->sm_detection_type ==
        ST_SM_TYPE_KEYWORD_DETECTION) {
        sva_threshold_config_t *threshold_cfg;

        cfg_ptr_threshold = calloc(1, sizeof(sva_threshold_config_t));
        if (!cfg_ptr_threshold) {
            ALOGE("%s: failed to allocate cfg_ptr_threshold", __func__);
            ret = -ENOMEM;
            goto exit;
        }

        capi_buf.data_ptr = cfg_ptr_threshold;
        capi_buf.actual_data_len = sizeof(sva_threshold_config_t);
        capi_buf.max_data_len = sizeof(sva_threshold_config_t);
        threshold_cfg = (sva_threshold_config_t *)capi_buf.data_ptr;
        threshold_cfg->smm_threshold = ss_session->confidence_threshold;
        ALOGD("%s: Keyword detection (CNN) confidence level = %d", __func__,
            ss_session->confidence_threshold);

        ALOGV("%s: Issuing capi_set_param for param %d", __func__,
            SVA_ID_THRESHOLD_CONFIG);
        rc = ss_session->capi_handle->vtbl_ptr->set_param(
            ss_session->capi_handle, SVA_ID_THRESHOLD_CONFIG, NULL, &capi_buf);
        if (CAPI_V2_EOK != rc) {
            ret = -EINVAL;
            ALOGE("%s: set_param SVA_ID_THRESHOLD_CONFIG failed, result = %d",
                __func__, rc);
            goto exit;
        }

        ALOGV("%s: Issuing capi_set_param for param %d", __func__,
            SVA_ID_REINIT_ALL);
        rc = ss_session->capi_handle->vtbl_ptr->set_param(
            ss_session->capi_handle, SVA_ID_REINIT_ALL, NULL, NULL);
        if (CAPI_V2_EOK != rc) {
            ret = -EINVAL;
            ALOGE("%s: set_param SVA_ID_REINIT_ALL failed, result = %d",
                __func__, rc);
            goto exit;
        }
    } else if (st_sec_stage->ss_info->sm_detection_type ==
        ST_SM_TYPE_USER_VERIFICATION) {
        voiceprint2_threshold_config_t *threshold_cfg;

        ALOGV("%s: Issuing capi_set_param for param %d", __func__,
            VOICEPRINT2_ID_REINIT);
        rc = ss_session->capi_handle->vtbl_ptr->set_param(
            ss_session->capi_handle, VOICEPRINT2_ID_REINIT, NULL, NULL);
        if (CAPI_V2_EOK != rc) {
            ret = -EINVAL;
            ALOGE("%s: set_param VOICEPRINT2_ID_REINIT failed, result = %d",
                __func__, rc);
            goto exit;
        }

        cfg_ptr_threshold = calloc(1, sizeof(voiceprint2_threshold_config_t));
        if (!cfg_ptr_threshold) {
            ALOGE("%s: failed to allocate cfg_ptr_threshold", __func__);
            ret = -ENOMEM;
            goto exit;
        }

        capi_buf.data_ptr = cfg_ptr_threshold;
        capi_buf.actual_data_len = sizeof(voiceprint2_threshold_config_t);
        capi_buf.max_data_len = sizeof(voiceprint2_threshold_config_t);
        threshold_cfg = (voiceprint2_threshold_config_t *)capi_buf.data_ptr;
        threshold_cfg->user_verification_threshold =
            ss_session->confidence_threshold;
        ALOGD("%s: User verification (VOP) confidence level = %d", __func__,
            ss_session->confidence_threshold);

        ALOGV("%s: Issuing capi_set_param for param %d", __func__,
            VOICEPRINT2_ID_THRESHOLD_CONFIG);
        rc = ss_session->capi_handle->vtbl_ptr->set_param(
            ss_session->capi_handle, VOICEPRINT2_ID_THRESHOLD_CONFIG, NULL,
            &capi_buf);
        if (CAPI_V2_EOK != rc) {
            ret = -EINVAL;
            ALOGE("%s: set_param VOICEPRINT2_ID_THRESHOLD_CONFIG failed"
                ", result = %d", __func__, rc);
            goto exit;
        }
    }

exit:
    if (cfg_ptr_threshold)
        free(cfg_ptr_threshold);
    return ret;
}

int st_second_stage_module_init(st_arm_second_stage_t *st_sec_stage,
    void *lib_name)
{
    int ret = 0;
    const char *lib = (const char *)lib_name;
    capi_v2_proplist_t init_set_proplist;
    capi_v2_prop_t sm_prop_ptr;
    capi_v2_err_t rc;
    st_arm_ss_session_t *ss_session = NULL;

    if (!st_sec_stage || !st_sec_stage->ss_session) {
        ALOGE("%s: Received NULL params, exiting", __func__);
        return -EINVAL;
    }
    ss_session = st_sec_stage->ss_session;
    st_sec_stage->dump_fp = NULL;

    /* Allocate extra pointers needed in the capi wrappers */
    if (st_sec_stage->ss_info->sm_detection_type ==
        ST_SM_TYPE_KEYWORD_DETECTION)
        ss_session->capi_handle = calloc(1,
            sizeof(capi_v2_t) + sizeof(char *));
    else if (st_sec_stage->ss_info->sm_detection_type ==
        ST_SM_TYPE_USER_VERIFICATION)
        ss_session->capi_handle = calloc(1,
            sizeof(capi_v2_t) + (3 * sizeof(char *)));
    /* else TODO: ST_SM_TYPE_CUSTOM_DETECTION */

    if (!ss_session->capi_handle) {
        ALOGE("%s: failed to allocate ss_session->capi_handle", __func__);
        ret = -ENOMEM;
        goto err_exit;
    }

    ss_session->capi_lib_handle = dlopen(lib, RTLD_NOW);
    if (!ss_session->capi_lib_handle) {
        ALOGE("%s: ERROR. %s", __func__, dlerror());
        ret = -ENODEV;
        goto err_exit;
    }
    dlerror();

    DLSYM(ss_session->capi_lib_handle, ss_session->capi_init,
          capi_v2_init, ret);
    if (ret)
        goto err_exit;

    sm_prop_ptr.id = CAPI_V2_CUSTOM_INIT_DATA;
    sm_prop_ptr.payload.data_ptr = (int8_t *)ss_session->sound_model;
    sm_prop_ptr.payload.actual_data_len = (uint32_t)ss_session->sm_size;
    sm_prop_ptr.payload.max_data_len = (uint32_t)ss_session->sm_size;
    init_set_proplist.props_num = 1;
    init_set_proplist.prop_ptr = &sm_prop_ptr;

    ALOGV("%s: Issuing capi_init", __func__);
    rc = ss_session->capi_init(ss_session->capi_handle, &init_set_proplist);
    if (rc != CAPI_V2_EOK) {
        ALOGE("%s: capi_init result is %d, exiting", __func__, rc);
        ret = -EINVAL;
        goto err_exit;
    }
    if (NULL == ss_session->capi_handle) {
        ALOGE("%s: capi_handle is NULL, exiting", __func__);
        ret = -EINVAL;
        goto err_exit;
    }
    if (NULL == ss_session->capi_handle->vtbl_ptr) {
        ALOGE("%s: capi_handle->vtbl_ptr is NULL, exiting", __func__);
        ret = -EINVAL;
        goto err_exit;
    }

    return 0;

err_exit:
    if (ss_session->capi_handle) {
        free(ss_session->capi_handle);
        ss_session->capi_handle = NULL;
    }
    if (ss_session->capi_lib_handle) {
        dlclose(ss_session->capi_lib_handle);
        ss_session->capi_lib_handle = NULL;
    }
    return ret;
}

int st_second_stage_module_deinit(st_arm_second_stage_t *st_sec_stage)
{
    if (st_sec_stage) {
        if (st_sec_stage->ss_session) {
            dlclose(st_sec_stage->ss_session->capi_lib_handle);
            st_sec_stage->ss_session->capi_lib_handle = NULL;
            if (st_sec_stage->ss_session->capi_handle) {
                st_sec_stage->ss_session->capi_handle->vtbl_ptr = NULL;
                free(st_sec_stage->ss_session->capi_handle);
                st_sec_stage->ss_session->capi_handle = NULL;
            }
        }
    }

    return 0;
}

int st_second_stage_prepare_session(st_arm_second_stage_t *st_sec_stage)
{
    int status = 0;
    pthread_condattr_t attr;

    pthread_mutex_init(&st_sec_stage->ss_session->lock, NULL);
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&st_sec_stage->ss_session->cond, &attr);
    pthread_condattr_destroy(&attr);
    st_sec_stage->ss_session->exit_thread = false;

    status = pthread_create(&st_sec_stage->ss_session->thread, NULL,
        buffer_thread_loop, st_sec_stage);
    if (status)
        ALOGE("%s: Error creating second stage buffer thread. status = %d",
            __func__, status);

    return status;
}

int st_second_stage_stop_session(st_arm_second_stage_t *st_sec_stage)
{
    int status = 0;
    capi_v2_err_t rc;

    if ((st_sec_stage->ss_info->sm_detection_type ==
         ST_SM_TYPE_USER_VERIFICATION) &&
        !st_sec_stage->ss_session->st_ses->stdev->ssr_offline_received) {
        ALOGV("%s: Issuing capi_end", __func__);
        rc = st_sec_stage->ss_session->capi_handle->vtbl_ptr->end(
            st_sec_stage->ss_session->capi_handle);
        if (rc != CAPI_V2_EOK)
            ALOGE("%s: Capi end function failed, result = %d",
                __func__, rc);
    }

    st_sec_stage->ss_session->exit_thread = true;
    st_sec_stage->ss_session->exit_buffering = true;
    pthread_mutex_lock(&st_sec_stage->ss_session->lock);
    pthread_cond_signal(&st_sec_stage->ss_session->cond);
    pthread_mutex_unlock(&st_sec_stage->ss_session->lock);
    status = pthread_join(st_sec_stage->ss_session->thread, NULL);
    if (status)
        ALOGE("%s: Error joining second stage thread, status = %d",
            __func__, status);
    pthread_cond_destroy(&st_sec_stage->ss_session->cond);
    pthread_mutex_destroy(&st_sec_stage->ss_session->lock);

    return 0;
}
