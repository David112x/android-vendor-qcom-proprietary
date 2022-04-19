/*
 * This file contains the state machine for a single sound trigger
 * user session. This state machine implements logic for handling all user
 * interactions, detectinos, SSR and Audio Concurencies.
 *
 * Copyright (c) 2016-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "sound_trigger_hw"
#define ATRACE_TAG (ATRACE_TAG_HAL)
/* #define LOG_NDEBUG 0 */
#define LOG_NDDEBUG 0

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <cutils/log.h>
#include <cutils/trace.h>
#include "st_session.h"
#include "st_hw_session.h"
#include "st_hw_session_lsm.h"
#include "st_hw_session_gcs.h"
#include "sound_trigger_hw.h"
#include "st_hw_session_pcm.h"
#include "st_hw_extn.h"
#include "st_hw_common.h"
#include "st_second_stage.h"

#ifdef LINUX_ENABLED
#define ST_SES_DEFERRED_STOP_DELAY_MS 0
#else
#define ST_SES_DEFERRED_STOP_DELAY_MS 1000
#endif

#define IS_SS_DETECTION_PENDING(det)\
    (det & (KEYWORD_DETECTION_PENDING | USER_VERIFICATION_PENDING))
#define IS_SS_DETECTION_SUCCESS(det)\
    !(det & (KEYWORD_DETECTION_REJECT | USER_VERIFICATION_REJECT))

/* below enum used in cleanup in error scenarios */
enum hw_session_err_mask {
    HW_SES_ERR_MASK_DEVICE_SET = 0x1,
    HW_SES_ERR_MASK_REG_SM = 0x2,
    HW_SES_ERR_MASK_REG_SM_PARAM = 0x4,
    HW_SES_ERR_MASK_STARTED = 0x8,
    HW_SES_ERR_MASK_BUFFERING = 0x10,
};

#define STATE_TRANSITION(st_session, new_state_fn)\
do {\
        st_session->current_state = new_state_fn;\
        ALOGD("session[%d]: %s ---> %s %s", st_session->sm_handle, __func__, \
            #new_state_fn, st_session->paused ? "(paused)" : "");\
} while(0)

#define DISPATCH_EVENT(ST_SESSION, EVENT, STATUS)\
do {\
    STATUS = ST_SESSION->current_state(ST_SESSION, &EVENT);\
} while (0)

#define REG_SM_RETRY_CNT 5
#define REG_SM_WAIT_TIME_MS 100

static inline int process_detection_event
(
    st_session_t *st_ses, uint64_t timestamp, int detect_status,
    void *payload, size_t payload_size,
    struct sound_trigger_recognition_event **event
);

static inline void enable_second_stage_processing
(
    st_session_t *st_ses,
    st_hw_session_t *hw_ses
)
{
    hw_ses->enable_second_stage = st_ses->enable_second_stage;
    st_ses->lab_enabled =
        (st_ses->capture_requested || st_ses->enable_second_stage);
}

static inline void disable_second_stage_processing
(
    st_session_t *st_ses,
    st_hw_session_t *hw_ses
)
{
    hw_ses->enable_second_stage = false;
    st_ses->lab_enabled = st_ses->capture_requested;
}

static int idle_state_fn(st_session_t *st_ses, st_session_ev_t *ev);
static int loaded_state_fn(st_session_t *st_ses, st_session_ev_t *ev);
static int active_state_fn(st_session_t *st_ses, st_session_ev_t *ev);
static int detected_state_fn(st_session_t *st_ses, st_session_ev_t *ev);
static int buffering_state_fn(st_session_t *st_ses, st_session_ev_t *ev);
static int ssr_state_fn(st_session_t *st_ses, st_session_ev_t *ev);

typedef struct st_session_loadsm_payload {
    struct sound_trigger_phrase_sound_model *sm_data;
} st_session_loadsm_payload_t;

typedef struct st_session_start_payload {
    void *config;
    size_t config_size;
    recognition_callback_t callback;
    void *cookie;
} st_session_start_payload_t;

typedef struct st_session_read_pcm_payload {
    void *out_buff;
    size_t out_buff_size;
    size_t *actual_read_size;
} st_session_readpcm_payload_t;

typedef struct st_session_get_param_payload {
    const char *param;
    void *payload;
    size_t payload_size;
    size_t *param_data_size;
} st_session_getparam_payload_t;

struct st_session_ev {
    st_session_event_id_t ev_id;
    union {
        st_session_loadsm_payload_t loadsm;
        st_session_start_payload_t start;
        st_hw_sess_detected_ev_t detected;
        st_exec_mode_t exec_mode;
        st_session_readpcm_payload_t readpcm;
        enum ssr_event_status ssr;
        char *chmix_coeff_str;
        bool enable;
        st_session_getparam_payload_t getparam;
    } payload;
};

ST_DBG_DECLARE(static int file_cnt = 0);

void hw_sess_cb(st_hw_sess_event_t *hw_event, void *cookie)
{
    st_session_t *st_ses = (st_session_t *)cookie;
    int status = 0;
    int lock_status = 0;

    if (!hw_event || !cookie) {
        ALOGE("%s: received NULL params", __func__);
        return;
    }

    switch (hw_event->event_id) {
    case ST_HW_SESS_EVENT_DETECTED:
        {
            st_session_ev_t ev;
            ev.ev_id = ST_SES_EV_DETECTED;
            ev.payload.detected = hw_event->payload.detected;

            do {
                lock_status = pthread_mutex_trylock(&st_ses->lock);
            } while (lock_status && !st_ses->device_disabled &&
                     (st_ses->exec_mode != ST_EXEC_MODE_NONE) &&
                     (st_ses->current_state != ssr_state_fn));

            /*
             * TODO: Add RECOGNITION_STATUS_GET_STATE_RESPONSE to
             * the SoundTrigger API header.
             */
            if (st_ses->detection_requested)
                ev.payload.detected.detect_status = 3;

            if (st_ses->device_disabled) {
                ALOGV("%s:[%d] device switch in progress, ignore event",
                      __func__, st_ses->sm_handle);
            } else if (st_ses->exec_mode == ST_EXEC_MODE_NONE) {
                ALOGV("%s:[%d] transition in progress, ignore event",
                      __func__, st_ses->sm_handle);
            } else if (st_ses->current_state == ssr_state_fn) {
                ALOGV("%s:[%d] SSR handling in progress, ignore event",
                      __func__, st_ses->sm_handle);
            } else if (!lock_status) {
                DISPATCH_EVENT(st_ses, ev, status);
            }

            if (!lock_status)
                pthread_mutex_unlock(&st_ses->lock);
            break;
        }

    default:
        ALOGD("%s:[%d] unhandled event", __func__, st_ses->sm_handle);
        break;
    };

}

static void do_hw_sess_cleanup(st_session_t *st_ses, st_hw_session_t *hw_ses,
    enum hw_session_err_mask err)
{
    if (err & HW_SES_ERR_MASK_BUFFERING)
        hw_ses->fptrs->stop_buffering(hw_ses, st_ses->lab_enabled);

    if (err & HW_SES_ERR_MASK_STARTED) {
        hw_ses->fptrs->stop(hw_ses);
        st_ses->hw_session_started = false;
    }

    if (err & HW_SES_ERR_MASK_REG_SM_PARAM)
        hw_ses->fptrs->dereg_sm_params(hw_ses);

    if (err & HW_SES_ERR_MASK_DEVICE_SET)
        hw_ses->fptrs->set_device(hw_ses, false);

    if (err & HW_SES_ERR_MASK_REG_SM)
        hw_ses->fptrs->dereg_sm(hw_ses, st_ses->lab_enabled);
}

static void reg_hal_event_session(st_session_t *p_ses, st_hw_session_t *hw_ses)
{
    struct sound_trigger_event_info event_info;
    /* Pass the pcm information to audio hal for capturing LAB */
    if (p_ses->lab_enabled && p_ses->stdev->audio_hal_cb) {
        ALOGD("%s: ST_EVENT_SESSION_REGISTER capture_handle %d p_ses %p",
              __func__, p_ses->capture_handle, (void *)p_ses);
        event_info.st_ses.p_ses = (void *)p_ses;
        event_info.st_ses.config = hw_ses->config;
        event_info.st_ses.capture_handle = p_ses->capture_handle;
        /*
         * set pcm to NULL as this version of st_hal doesn't pass pcm to
         * audio HAL
         */
        event_info.st_ses.pcm = NULL;
        p_ses->stdev->audio_hal_cb(ST_EVENT_SESSION_REGISTER, &event_info);
    }
}

static void dereg_hal_event_session(st_session_t *p_ses)
{
    struct sound_trigger_event_info event_info;
    /* Indicate to audio hal that buffering is stopped to stop reading LAB data */
    if (p_ses->lab_enabled && p_ses->stdev->audio_hal_cb) {
        ALOGD("%s: ST_EVENT_SESSION_DEREGISTER capture_handle %d p_ses %p",
           __func__, p_ses->capture_handle, p_ses);
        event_info.st_ses.p_ses = (void *)p_ses;
        event_info.st_ses.capture_handle = p_ses->capture_handle;
        event_info.st_ses.pcm = NULL;
        p_ses->stdev->audio_hal_cb(ST_EVENT_SESSION_DEREGISTER, &event_info);
    }
}

static int start_hw_session(st_session_t *st_ses, st_hw_session_t *hw_ses, bool load_sm)
{
    int status = 0, err = 0;

    /*
     * It is possible the BE LPI mode has been updated, but not the FE mode.
     * DSP requires both FE and BE to be in the same mode for any configuration
     * changes between LPI and non-LPI switch, so update the FE mode to the
     * same as BE mode by re-opening LSM session.
     */
    if (hw_ses->lpi_enable != hw_ses->stdev->lpi_enable) {
        hw_ses->lpi_enable = hw_ses->stdev->lpi_enable;
        if (!load_sm) {
            load_sm = true;
            status = hw_ses->fptrs->dereg_sm(hw_ses, st_ses->lab_enabled);
            if (status)
                ALOGW("%s:[%d] failed to dereg_sm err %d", __func__,
                    st_ses->sm_handle, status);
        }
    }

    if (load_sm) {
        status = hw_ses->fptrs->reg_sm(hw_ses, st_ses->sm_data,
            st_ses->sm_type);
        if (status) {
            ALOGE("%s:[%d] failed to reg_sm err %d", __func__,
                st_ses->sm_handle, status);
            goto cleanup;
        }
        err |= HW_SES_ERR_MASK_REG_SM;
    }

    status = hw_ses->fptrs->set_device(hw_ses, true);
    if (status) {
        ALOGE("%s:[%d] failed to set_device err %d", __func__,
            st_ses->sm_handle, status);
        goto cleanup;
    }
    err |= HW_SES_ERR_MASK_DEVICE_SET;

    /*
     * Check for rc_config update before reg_sm_param,
     * as hw session can change from transitions,
     * and hence related config might also need to be updated.
     */
    if (hw_ses->rc_config != st_ses->rc_config ||
        hw_ses->rc_config_update_counter != st_ses->rc_config_update_counter) {
        status = st_hw_ses_update_config(st_ses, hw_ses);
        if (status) {
            ALOGE("%s: ERROR. updating rc_config, returned status %d",
                  __func__, status);
            goto cleanup;
        }
    }

    status = hw_ses->fptrs->reg_sm_params(hw_ses,
        st_ses->recognition_mode, st_ses->lab_enabled,
        st_ses->rc_config, st_ses->sm_type, st_ses->sm_data);
    if (status) {
        ALOGE("%s:[%d] failed to reg_sm_params err %d", __func__,
            st_ses->sm_handle, status);
        goto cleanup;
    }
    err |= HW_SES_ERR_MASK_REG_SM_PARAM;

    status = hw_ses->fptrs->start(hw_ses);
    if (status) {
        ALOGE("%s:[%d] failed to start err %d", __func__,
            st_ses->sm_handle, status);
        goto cleanup;
    }
    err |= HW_SES_ERR_MASK_STARTED;

    st_ses->hw_session_started = true;
    return status;

cleanup:
    do_hw_sess_cleanup(st_ses, hw_ses, err);
    return status;
}

static int stop_hw_session(st_session_t *st_ses, st_hw_session_t *hw_ses, bool unload_sm)
{
    int status = 0;
    int rc = 0;

    status = hw_ses->fptrs->stop(hw_ses);
    if (status) {
        ALOGE("%s:[%d] failed to stop err %d", __func__,
            st_ses->sm_handle, status);
        rc = status;
    }

    status = hw_ses->fptrs->dereg_sm_params(hw_ses);
    if (status) {
        ALOGE("%s:[%d] failed to dereg_sm_params err %d", __func__,
            st_ses->sm_handle, status);
        rc = status;
    }

    status = hw_ses->fptrs->set_device(hw_ses, false);
    if (status) {
        ALOGE("%s:[%d] failed to set_device err %d", __func__,
            st_ses->sm_handle, status);
        rc = status;
    }
    if (unload_sm) {
        status = hw_ses->fptrs->dereg_sm(hw_ses, st_ses->lab_enabled);
        if (status) {
            ALOGE("%s:[%d] failed to dereg_sm, err %d", __func__,
                st_ses->sm_handle, status);
            rc = status;
        }
    }

    /* This must be set to false irrespective as the above calls may
     * return error (esp for SSR)
     */
    st_ses->hw_session_started = false;
    return rc;
}

static int start_session(st_session_t *st_ses, st_hw_session_t *hw_ses, bool load_sm)
{
    int status = 0;
    if (st_ses->hw_session_started) {
        ALOGE("%s:[%d] already started", __func__, st_ses->sm_handle);
        return -1;
    }
    /*
     * The reg_hal_event_session call must be after start_hw_session. This is
     * important for when load_sm is true, because reg_sm sets the correct pcm
     * config for the current hw session. That pcm config is then sent to audio hal.
     */
    status = start_hw_session(st_ses, hw_ses, load_sm);
    if (!status)
        reg_hal_event_session(st_ses, hw_ses);
    return status;
}

static int restart_session(st_session_t *st_ses, st_hw_session_t *hw_ses)
{
    int status = hw_ses->fptrs->restart(hw_ses, st_ses->recognition_mode,
                        st_ses->lab_enabled, st_ses->rc_config,
                        st_ses->sm_type, st_ses->sm_data);
    if (status == 0) {
        st_ses->hw_session_started = true;
    } else {
        ALOGE("%s:[%d] failed to restart, stop session", __func__, st_ses->sm_handle);
        /*
         * lower layers like gcs/lsm need to handle double stop calls properly
         * to avoid possible crash, as some of the clean ups are already issued
         * during fptrs->restart() when it's failed.
         */
        stop_hw_session(st_ses, hw_ses, true);
    }
    return status;
}

static int stop_session(st_session_t *st_ses, st_hw_session_t *hw_ses, bool unload_sm)
{
    if (!st_ses->hw_session_started) {
        ALOGV("%s:[%d] already stopped", __func__, st_ses->sm_handle);
        return 0;
    }
    dereg_hal_event_session(st_ses);
    if (st_ses->detection_requested) {
        st_ses->detection_requested = false;
        enable_second_stage_processing(st_ses, hw_ses);
    }
    return stop_hw_session(st_ses, hw_ses, unload_sm);
}

/*
 * This function gets the first stage detection keyword indices, which are needed
 * by the second stage sessions. If the legacy DSP is used, which does not provide
 * keyword indices, set the indices to include the entire keyword duration. This
 * function also gets the user confidence level if there is an active voiceprint
 * session.
 */
static int get_first_stage_detection_params(st_session_t *st_ses, void *payload,
                                            size_t payload_size)
{
    size_t count_size = 0;
    uint8_t *payload_ptr = (uint8_t *)payload;
    uint32_t key_id = 0, key_payload_size = 0;
    uint32_t kw_start_ms = 0, kw_end_ms = 0;
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    struct listnode *node = NULL, *tmp_node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;
    bool is_active_vop_session = false;

    list_for_each_safe(node, tmp_node, &st_ses->second_stage_list) {
        st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
        if (st_sec_stage->ss_info->sm_detection_type == ST_SM_TYPE_USER_VERIFICATION) {
            is_active_vop_session = true;
            break;
        }
    }

    if (hw_ses->is_generic_event) {
        /*
         * This case is for the generic detection event from the DSP. Set the
         * keyword start and end indices and user confidence level based on key
         * id, if applicable.
         */
        while (count_size < payload_size) {
            key_id = *(uint32_t *)payload_ptr;
            key_payload_size = *((uint32_t *)payload_ptr + 1);

            switch (key_id) {
            case KEY_ID_CONFIDENCE_LEVELS:
                if (is_active_vop_session) {
                    hw_ses->user_level = (int32_t)(*(payload_ptr +
                        GENERIC_DET_EVENT_USER_LEVEL_OFFSET));
                }
                break;

            case KEY_ID_KEYWORD_POSITION_STATS:
                hw_ses->kw_start_idx = *((uint32_t *)payload_ptr +
                    GENERIC_DET_EVENT_KW_START_OFFSET);
                hw_ses->kw_end_idx = *((uint32_t *)payload_ptr +
                    GENERIC_DET_EVENT_KW_END_OFFSET);
                break;

            default:
                ALOGW("%s: Unsupported generic detection event key id",
                    __func__);
                break;
            }
            count_size += GENERIC_DET_EVENT_HEADER_SIZE + key_payload_size;
            payload_ptr += count_size;
        }
    } else {
        /*
         * This case is for the DSP detection events which are not the generic
         * detection event. There will be no keyword indices from first stage
         * detection, so the start index will be 0 and the end index will be the
         * buffer duration sent from the app. If this is not sent, the keyword
         * duration from platform xml will be used.
         */
        hw_ses->kw_start_idx = 0;
        if (hw_ses->client_req_hist_buf) {
            hw_ses->kw_end_idx =
                convert_ms_to_bytes(hw_ses->client_req_hist_buf,
                    &hw_ses->config);
        } else {
            hw_ses->kw_end_idx =
                convert_ms_to_bytes(st_ses->vendor_uuid_info->kw_duration,
                    &hw_ses->config);
        }

        if (is_active_vop_session) {
            if ((st_ses->exec_mode == ST_EXEC_MODE_CPE) &&
                 st_ses->stdev->is_gcs) {
                hw_ses->user_level = (int32_t)(*(payload_ptr +
                    GCS_NON_GENERIC_USER_LEVEL_OFFSET));
            } else if ((st_ses->exec_mode == ST_EXEC_MODE_ADSP) ||
                      !st_ses->stdev->is_gcs) {
                hw_ses->user_level = (int32_t)(*(payload_ptr +
                    LSM_NON_GENERIC_USER_LEVEL_OFFSET));
            }
        }
    }

    kw_start_ms = convert_bytes_to_ms(hw_ses->kw_start_idx, &hw_ses->config);
    kw_end_ms = convert_bytes_to_ms(hw_ses->kw_end_idx, &hw_ses->config);
    ALOGD("%s:[%d] 1st stage kw_start = %dms, kw_end = %dms, is_generic_event %d",
        __func__, st_ses->sm_handle, kw_start_ms, kw_end_ms,
        hw_ses->is_generic_event);

    return 0;
}

static inline void stop_second_stage_session(st_session_t *st_ses)
{
    struct listnode *node = NULL, *tmp_node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;

    list_for_each_safe(node, tmp_node, &st_ses->second_stage_list) {
        st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
        st_second_stage_stop_session(st_sec_stage);
    }
}

static int idle_state_fn(st_session_t *st_ses, st_session_ev_t *ev)
{
    int status = 0;
    int ret = 0;
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    struct listnode *node = NULL, *tmp_node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;

    /* skip parameter check as this is an internal funciton */
    ALOGD("%s:[%d] handle event id %d", __func__, st_ses->sm_handle, ev->ev_id);

    switch (ev->ev_id) {
    case ST_SES_EV_LOAD_SM:
        if (!st_ses->sm_data) {
            ALOGE("%s: sound model data is not initialzed", __func__);
            status = -EINVAL;
            break;
        }

        /*
         * Do retry to handle a corner case that when ADSP SSR ONLINE is received,
         * sometimes ADSP is still not ready to receive cmd from HLOS and thus
         * fails, so try more times to recover the session from SSR state.
         */
        for (int i = 0; i < REG_SM_RETRY_CNT; i++) {
            status = ret = hw_ses->fptrs->reg_sm(hw_ses, st_ses->sm_data,
                st_ses->sm_type);
            if (ret) {
                if (st_ses->stdev->ssr_offline_received) {
                    st_ses->client_req_state = ST_STATE_LOADED;
                    STATE_TRANSITION(st_ses, ssr_state_fn);
                    /*
                     * Send success to client because the failure is recovered
                     * internally from SSR.
                     */
                    status = 0;
                    break;
                } else {
                    ALOGE("%s:[%d] failed to reg sm, err %d, retry cnt %d", __func__,
                        st_ses->sm_handle, status, i);
                    usleep(REG_SM_WAIT_TIME_MS * 1000);
                }
            } else {
                break;
            }
        }
        if (ret)
            break;

        if (st_ses->enable_second_stage) {
            hw_ses->enable_second_stage = true;
            hw_ses->second_stage_list = &(st_ses->second_stage_list);
            list_for_each_safe(node, tmp_node, &st_ses->second_stage_list) {
                st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
                st_sec_stage->ss_session->st_ses = st_ses;
                st_second_stage_prepare_session(st_sec_stage);
            }
        }

        STATE_TRANSITION(st_ses, loaded_state_fn);
        break;

    case ST_SES_EV_SET_EXEC_MODE:
        st_ses->exec_mode = ev->payload.exec_mode;
        if (ST_EXEC_MODE_CPE == st_ses->exec_mode)
            st_ses->hw_ses_current = st_ses->hw_ses_cpe;
        else if (ST_EXEC_MODE_ADSP == st_ses->exec_mode)
            st_ses->hw_ses_current = st_ses->hw_ses_adsp;
        /* remain in current state */
        break;

    case ST_SES_EV_PAUSE:
        st_ses->paused = true;
        break;

    case ST_SES_EV_RESUME:
        st_ses->paused = false;
        break;

    case ST_SES_EV_SSR_OFFLINE:
        STATE_TRANSITION(st_ses, ssr_state_fn);
        break;

    case ST_SES_EV_SEND_CHMIX_COEFF:
        status = -EIO;
        break;

    case ST_SES_EV_GET_PARAM_DATA:
        status = -EIO;
        break;

    case ST_SES_EV_REQUEST_DET:
        ALOGE("%s:[%d] Event not supported in this state",
            __func__, st_ses->sm_handle);
        status = -EINVAL;
        break;

    default:
        ALOGD("%s:[%d] unhandled event", __func__, st_ses->sm_handle);
        break;
    };

    return status;
}

static int loaded_state_fn(st_session_t *st_ses, st_session_ev_t *ev)
{
    int status = 0;
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    st_hw_session_t *new_hw_ses = NULL;
    st_exec_mode_t new_exec_mode = 0;
    struct listnode *node = NULL, *tmp_node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;

    /* skip parameter check as this is an internal funciton */
    ALOGD("%s:[%d] handle event id %d", __func__, st_ses->sm_handle, ev->ev_id);

    switch (ev->ev_id) {
    case ST_SES_EV_RESUME:
        if (!st_ses->paused)
            break;
        st_ses->paused = false;
        if (st_ses->client_req_state != ST_STATE_ACTIVE)
            break;
        /* If the session is paused and client_req_state is active, fall through
         * and handle similarly to start/restart.
         */
    case ST_SES_EV_START:
    case ST_SES_EV_RESTART:
        st_ses->client_req_state = ST_STATE_ACTIVE;
        if (!st_ses->paused) {
            /*
             *  There is a need to be able to differentiate between LAB due to a client
             *  request, and LAB due to second stage enablement.
             */
            st_ses->capture_requested = st_ses->rc_config->capture_requested;
            st_ses->lab_enabled =
                (st_ses->capture_requested || st_ses->enable_second_stage);

            status = start_session(st_ses, hw_ses, false);
            if (status) {
                if (st_ses->stdev->ssr_offline_received) {
                    if (st_ses->enable_second_stage)
                        stop_second_stage_session(st_ses);
                    hw_ses->fptrs->dereg_sm(hw_ses, st_ses->lab_enabled);
                    STATE_TRANSITION(st_ses, ssr_state_fn);
                    /* Send success to client because the failure is recovered
                     * internally from SSR.
                     */
                    status = 0;
                } else {
                    ALOGE("%s:[%d] failed to start session, err %d", __func__,
                        st_ses->sm_handle, status);
                }
                break;
            }

            if (st_ses->enable_second_stage) {
                list_for_each_safe(node, tmp_node, &st_ses->second_stage_list) {
                    st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
                    status = st_second_stage_start_session(st_sec_stage);
                    if (status) {
                        ALOGE("%s: Failed to start second stage session, exiting", __func__);
                        status = -EINVAL;
                        break;
                    }
                }
            }
            STATE_TRANSITION(st_ses, active_state_fn);
        }
        break;

    case ST_SES_EV_UNLOAD_SM:
        if (st_ses->enable_second_stage)
            stop_second_stage_session(st_ses);
        status = hw_ses->fptrs->dereg_sm(hw_ses, st_ses->lab_enabled);
        if (status) {
            /* since this is a teardown scenario dont fail here */
            ALOGE("%s:[%d] dereg_sm failed with err %d", __func__,
                st_ses->sm_handle, status);
            status = 0;
        }
        STATE_TRANSITION(st_ses, idle_state_fn);
        break;

    case ST_SES_EV_SSR_OFFLINE:
        if (st_ses->enable_second_stage)
            stop_second_stage_session(st_ses);
        /* exec mode can be none if ssr occurs during a transition */
        if (st_ses->exec_mode != ST_EXEC_MODE_NONE)
            hw_ses->fptrs->dereg_sm(hw_ses, st_ses->lab_enabled);
        /*
         * When the session is first loaded, the client_req_state remains
         * in idle state. In this case, client_req_state must be set to
         * loaded before the SSR handling. In other usecases,
         * client_req_state can be either loaded or active, so it should
         * not be changed here.
         */
        if (st_ses->client_req_state == ST_STATE_IDLE)
            st_ses->client_req_state = ST_STATE_LOADED;
        STATE_TRANSITION(st_ses, ssr_state_fn);
        break;

    case ST_SES_EV_PAUSE:
        st_ses->paused = true;
        break;

    case ST_SES_EV_STOP:
        st_ses->client_req_state = ST_STATE_LOADED;
        break;

    case ST_SES_EV_SET_EXEC_MODE:
        new_exec_mode = ev->payload.exec_mode;

        if ((st_ses->exec_mode != new_exec_mode) &&
            st_ses->enable_trans) {

            if (st_ses->exec_mode != ST_EXEC_MODE_NONE) {
                st_ses->exec_mode = ST_EXEC_MODE_NONE;
                /* unload sm for current hw session */
                status = hw_ses->fptrs->dereg_sm(hw_ses, st_ses->lab_enabled);
                if (status) {
                    ALOGE("%s:[%d] dereg_sm failed with err %d", __func__,
                        st_ses->sm_handle, status);
                    break;
                }
            }

            if (new_exec_mode == ST_EXEC_MODE_NONE)
                break;

            /* load sm to new hw_ses */
            if (ST_EXEC_MODE_CPE == new_exec_mode) {
                new_hw_ses = st_ses->hw_ses_cpe;
                st_ses->hw_ses_cpe->enable_second_stage =
                    st_ses->hw_ses_adsp->enable_second_stage;
                st_ses->hw_ses_cpe->second_stage_list =
                    st_ses->hw_ses_adsp->second_stage_list;
            } else if (ST_EXEC_MODE_ADSP == new_exec_mode) {
                new_hw_ses = st_ses->hw_ses_adsp;
                st_ses->hw_ses_adsp->enable_second_stage =
                    st_ses->hw_ses_cpe->enable_second_stage;
                st_ses->hw_ses_adsp->second_stage_list =
                    st_ses->hw_ses_cpe->second_stage_list;
            } else {
                ALOGE("%s: unknown execution mode %d", __func__,
                    new_exec_mode);
                status = -EINVAL;
                break;
            }

            status = new_hw_ses->fptrs->reg_sm(new_hw_ses,
                        st_ses->sm_data, st_ses->sm_type);
            if (status) {
                ALOGE("%s:[%d] reg_sm failed with err %d", __func__,
                       st_ses->sm_handle, status);
                break;
            }
            /* switch hw sessions only if successful*/
            st_ses->exec_mode = new_exec_mode;
            st_ses->hw_ses_current = new_hw_ses;
            /* remain in current state */
        }
        break;

    case ST_SES_EV_SET_DEVICE:
        /*
         * This event handling is needed for certain graphs which
         * have multiple buffering modules with a single voice wakeup
         * module in each usecase.
         */
        if (!ev->payload.enable)
            status = hw_ses->fptrs->disable_device(hw_ses, false);
        else
            status = hw_ses->fptrs->enable_device(hw_ses, false);

        break;

    case ST_SES_EV_READ_PCM:
        /*
         * set status to failure this will tell AHAL to
         * provide zero buffers to client
         */
        status = -EIO;
        break;

    case ST_SES_EV_SEND_CHMIX_COEFF:
        status = -EIO;
        break;

    case ST_SES_EV_GET_PARAM_DATA:
        status = hw_ses->fptrs->get_param_data(hw_ses,
                      ev->payload.getparam.param, ev->payload.getparam.payload,
                      ev->payload.getparam.payload_size,
                      ev->payload.getparam.param_data_size);
        break;

    case ST_SES_EV_REQUEST_DET:
        ALOGE("%s:[%d] Event not supported in this state",
            __func__, st_ses->sm_handle);
        status = -EINVAL;
        break;

    default:
        ALOGD("%s:[%d] unhandled event", __func__, st_ses->sm_handle);
        break;

    };

    return status;
}

static int active_state_fn(st_session_t *st_ses, st_session_ev_t *ev)
{
    int status = 0;
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    st_session_ev_t deferred_ev =  { .ev_id = ST_SES_EV_DEFERRED_STOP };
    st_hw_session_t *new_hw_ses = NULL;
    st_exec_mode_t new_exec_mode;

    /* skip parameter check as this is an internal funciton */
    ALOGD("%s:[%d] handle event id %d", __func__, st_ses->sm_handle, ev->ev_id);

    switch (ev->ev_id) {
    case ST_SES_EV_SET_EXEC_MODE:
        new_exec_mode = ev->payload.exec_mode;

        /* if no change in mode or dynamic transition not enabled then noop */
        if ((new_exec_mode == st_ses->exec_mode) || !st_ses->enable_trans)
            break;

        if (st_ses->exec_mode != ST_EXEC_MODE_NONE) {
            ALOGV("%s: disable current session", __func__);
            st_ses->exec_mode = ST_EXEC_MODE_NONE;
            status = stop_session(st_ses, hw_ses, true);
            if (status)
                break;
        }

        if (new_exec_mode == ST_EXEC_MODE_NONE)
            break;

        if (ST_EXEC_MODE_CPE == new_exec_mode) {
            new_hw_ses = st_ses->hw_ses_cpe;
            st_ses->hw_ses_cpe->enable_second_stage =
                st_ses->hw_ses_adsp->enable_second_stage;
            st_ses->hw_ses_cpe->second_stage_list =
                st_ses->hw_ses_adsp->second_stage_list;
        } else if (ST_EXEC_MODE_ADSP == new_exec_mode) {
            new_hw_ses = st_ses->hw_ses_adsp;
            st_ses->hw_ses_adsp->enable_second_stage =
                st_ses->hw_ses_cpe->enable_second_stage;
            st_ses->hw_ses_adsp->second_stage_list =
                st_ses->hw_ses_cpe->second_stage_list;
        } else {
            ALOGE("%s: unknown execution mode %d", __func__,
                new_exec_mode);
            status = -EINVAL;
            break;
        }

        ALOGV("%s: enable current session", __func__);
        status = start_session(st_ses, new_hw_ses, true);
        if (status)
            break;

        /* set new exec mode and current session */
        st_ses->exec_mode = new_exec_mode;
        st_ses->hw_ses_current = new_hw_ses;
        ALOGV("%s: end transition", __func__);
        break;

    case ST_SES_EV_PAUSE:
        st_ses->paused = true;
        /* Fall through to handle pause events similarly to stop events. */
    case ST_SES_EV_STOP:
        if (st_ses->paused)
            st_ses->client_req_state = ST_STATE_ACTIVE;
        else
            st_ses->client_req_state = ST_STATE_LOADED;
        status = stop_session(st_ses, hw_ses, false);
        if (status) {
            if (st_ses->stdev->ssr_offline_received) {
                STATE_TRANSITION(st_ses, ssr_state_fn);
                if (st_ses->enable_second_stage)
                    stop_second_stage_session(st_ses);
                hw_ses->fptrs->dereg_sm(hw_ses, st_ses->lab_enabled);
                /* Send success to client because the failure is recovered
                 * internally from SSR.
                 */
                status = 0;
                break;
            } else {
                ALOGE("%s:[%d] failed to stop session, err %d", __func__,
                    st_ses->sm_handle, status);
            }
        }

        STATE_TRANSITION(st_ses, loaded_state_fn);
        break;

    case ST_SES_EV_DETECTED:
        {
            size_t payload_size = ev->payload.detected.payload_size;
            struct sound_trigger_recognition_event *event = NULL;
            recognition_callback_t callback;
            bool lab_enabled = false, enable_second_stage = false;

            if (!st_ses->enable_second_stage ||
                st_ses->detection_requested) {
                status = process_detection_event(st_ses,
                    ev->payload.detected.timestamp,
                    ev->payload.detected.detect_status,
                    ev->payload.detected.detect_payload,
                    payload_size, &event);
                if (status || !event) {
                    ALOGE("%s:[%d] process_detection_event failed err %d", __func__,
                        st_ses->sm_handle, status);
                    /* Stop buffering if this is not a successful detection and
                        LAB is triggered in hw automatically */
                    hw_ses->fptrs->stop_buffering(hw_ses, st_ses->lab_enabled);

                    if (event)
                        free(event);
                    break;
                }
            } else {
                enable_second_stage = true;
                st_ses->sent_detection_to_client = false;
                get_first_stage_detection_params(st_ses,
                    ev->payload.detected.detect_payload, payload_size);
                memcpy(st_ses->det_session_ev, ev, sizeof(st_session_ev_t));
            }

            /*
             * change to new state before invoking user callback, this will
             * ensure that if user calls start_recognition immediately from the
             * callback it will be handled by one of the two states below
             */
            if (!status && st_ses->lab_enabled) {
                ST_DBG_FILE_OPEN_WR(st_ses->lab_fp, ST_DEBUG_DUMP_LOCATION, "lab_capture",
                    "bin", file_cnt++);
                STATE_TRANSITION(st_ses, buffering_state_fn);
            } else {
                STATE_TRANSITION(st_ses, detected_state_fn);
            }

            if (!st_ses->callback) {
                ALOGE("%s:[%d] received detection event but no callback",
                    __func__, st_ses->sm_handle);
                status = -EINVAL;
                if (event && !enable_second_stage)
                    free(event);
                break;
            }
            callback = st_ses->callback;

            /*
             * store the current capture requested in-case a new start comes
             * once we exist the critical-section.
             * In this case we continue to operate based on previous capture requested
             * setting untill the new session start is processed and resets the state
             */
            lab_enabled = st_ses->lab_enabled;
            pthread_mutex_unlock(&st_ses->lock);

            /*
             * callback to user, assumption is that client does not
             * block in the callback waiting for data otherwise will be a deadlock
             */
            if (!enable_second_stage) {
                ALOGD("%s:[%d] invoking the client callback",
                    __func__, st_ses->sm_handle);
                ATRACE_ASYNC_END("sthal: detection success",
                    st_ses->sm_handle);
                ATRACE_BEGIN("sthal: client detection callback");
                callback(event, st_ses->cookie);
                ATRACE_END();
            }

            /*
             * TODO: Add RECOGNITION_STATUS_GET_STATE_RESPONSE to
             * the SoundTrigger API header.
             */
            if (lab_enabled &&
                ((ev->payload.detected.detect_status ==
                  RECOGNITION_STATUS_SUCCESS) ||
                 (ev->payload.detected.detect_status == 3))) {
                /* Cache lab data to internal buffers (blocking call) */
                hw_ses->fptrs->process_lab_capture(hw_ses);
            }

            /*
             * It is possible that the client may start/stop/unload the session
             * with the same lock held, before we aqcuire lock here.
             * We need further processing only if client starts in detected state
             * or buffering state if lab was enabled, else return gracefully.
             */
             do {
                status = pthread_mutex_trylock(&st_ses->lock);
             } while (status && ((st_ses->current_state == detected_state_fn) ||
                      (st_ses->current_state == buffering_state_fn)));

            if (st_ses->current_state != detected_state_fn) {
                ALOGV("%s:[%d] client not in detected state, lock status %d",
                      __func__, st_ses->sm_handle, status);
                if (!status) {
                    /*
                     * Stop session if still in buffering state and no pending
                     * stop to be handled i.e. internally buffering was stopped.
                     * This is required to avoid further detections in wrong state.
                     * Client is expected to issue start recognition for current
                     * detection event which will restart the session.
                     */
                    if ((st_ses->current_state == buffering_state_fn) &&
                        !st_ses->pending_stop) {
                        ALOGD("%s:[%d] buffering stopped internally, stop session",
                              __func__, st_ses->sm_handle);
                        stop_session(st_ses, hw_ses, false);
                        STATE_TRANSITION(st_ses, loaded_state_fn);
                    }

                    pthread_mutex_unlock(&st_ses->lock);
                }

                if (!st_ses->enable_second_stage)
                    free(event);
                status = 0;
                if (st_ses->detection_requested) {
                    st_ses->detection_requested = false;
                    enable_second_stage_processing(st_ses, hw_ses);
                }
                break;
            }

            if (st_ses->detection_requested) {
                st_ses->detection_requested = false;
                enable_second_stage_processing(st_ses, hw_ses);
            }
            /*
             * If we are not buffering (i.e capture is not requested), then
             * trigger a deferred stop. Most applications issue (re)start
             * almost immediately. Delaying stop allows unnecessary teardown
             * and reinitialization of backend.
             */
            if (!lab_enabled) {
                /*
                 * Note that this event will only be posted to the detected state
                 * The current state may switch to active if the client
                 * issues start/restart before control of the callback thread
                 * reaches this point.
                 */
                DISPATCH_EVENT(st_ses, deferred_ev, status);
            } else {
                ALOGE("%s:[%d] capture is requested but state is still detected!?",
                      __func__, st_ses->sm_handle);
            }

            if (!enable_second_stage)
                free(event);
            break;
        }

    case ST_SES_EV_SSR_OFFLINE:
        STATE_TRANSITION(st_ses, ssr_state_fn);
        if (st_ses->enable_second_stage)
            stop_second_stage_session(st_ses);
        /* exec mode can be none if ssr occurs during a transition */
        if (st_ses->exec_mode != ST_EXEC_MODE_NONE)
            stop_hw_session(st_ses, hw_ses, true /* unload_sm */);

        st_ses->client_req_state = ST_STATE_ACTIVE;
        break;

    case ST_SES_EV_SEND_CHMIX_COEFF:
        status = hw_ses->fptrs->send_custom_chmix_coeff(hw_ses,
                                  ev->payload.chmix_coeff_str);
        break;

    case ST_SES_EV_SET_DEVICE:
        if (!ev->payload.enable)
            status = hw_ses->fptrs->disable_device(hw_ses, true);
        else
            status = hw_ses->fptrs->enable_device(hw_ses, true);

        break;

    case ST_SES_EV_READ_PCM:
        /*
         * buffering could have been stopped internally
         * and switched to active state ex: transitions.
         * set status to failure this will tell AHAL to
         * provide zero buffers to client
         */
        status = -EIO;
        break;

    case ST_SES_EV_GET_PARAM_DATA:
        status = hw_ses->fptrs->get_param_data(hw_ses,
                      ev->payload.getparam.param, ev->payload.getparam.payload,
                      ev->payload.getparam.payload_size,
                      ev->payload.getparam.param_data_size);
        break;

    case ST_SES_EV_REQUEST_DET:
        status = hw_ses->fptrs->send_detection_request(hw_ses);
        if (!status) {
            st_ses->detection_requested = true;
            /*
             * Disable second stage processing if a forced detection is
             * requested.
             */
            disable_second_stage_processing(st_ses, hw_ses);
        }
        break;

    default:
        ALOGD("%s:[%d] unhandled event", __func__, st_ses->sm_handle);
        break;

    };

    return status;
}

static int detected_state_fn(st_session_t *st_ses, st_session_ev_t *ev)
{
    int status = 0;
    st_exec_mode_t new_exec_mode;
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    st_hw_session_t *new_hw_ses = NULL;

    /* skip parameter check as this is an internal funciton */
    ALOGD("%s:[%d] handle event id %d", __func__, st_ses->sm_handle, ev->ev_id);

    switch (ev->ev_id) {
    case ST_SES_EV_START:
        /* session already started but config has changed stop and restart */
        status = stop_session(st_ses, hw_ses, false);
        if (status)
            break;
        status = start_session(st_ses, hw_ses, false);
        if (status)
            break;
        STATE_TRANSITION(st_ses, active_state_fn);
        break;

    case ST_SES_EV_RESTART:
        /* session already restarted without any config changes */
        restart_session(st_ses, hw_ses);
        STATE_TRANSITION(st_ses, active_state_fn);
        break;
    case ST_SES_EV_PAUSE:
        st_ses->paused = true;
        st_ses->client_req_state = ST_STATE_LOADED;
        /* Fall through to handle pause events similarly to stop event. */
    case ST_SES_EV_STOP:
        /*
         * It is possible that the client can issue stop after detection
         * callback. This even can be issued internally as part of
         * deferred stop as well.
         */
        status = stop_session(st_ses, hw_ses, false);
        if (status) {
            if (st_ses->stdev->ssr_offline_received) {
                STATE_TRANSITION(st_ses, ssr_state_fn);
                if (st_ses->enable_second_stage)
                    stop_second_stage_session(st_ses);
                hw_ses->fptrs->dereg_sm(hw_ses, st_ses->lab_enabled);
                st_ses->client_req_state = ST_STATE_LOADED;
                /* Send success to client because the failure is recovered
                 * internally from SSR.
                 */
                status = 0;
            } else {
                ALOGE("%s:[%d] failed to stop session, err %d", __func__,
                    st_ses->sm_handle, status);
            }
            break;
        }

        STATE_TRANSITION(st_ses, loaded_state_fn);
        break;
    case ST_SES_EV_SSR_OFFLINE:
        if (st_ses->enable_second_stage)
            stop_second_stage_session(st_ses);
        /*
         * Ignore return status during SSR handling
         * as the ADSP or CPE might be down so these
         * calls would fail. Exec mode can be none if
         * ssr occurs during a transition.
         */
        if (st_ses->exec_mode != ST_EXEC_MODE_NONE)
            stop_hw_session(st_ses, hw_ses, true /* unload sm */);

        st_ses->client_req_state = ST_STATE_ACTIVE;
        STATE_TRANSITION(st_ses, ssr_state_fn);
        break;

    case ST_SES_EV_SET_EXEC_MODE:
        new_exec_mode = ev->payload.exec_mode;

        /* if no change in mode or dynamic transition not enabled then noop */
        if ((new_exec_mode == st_ses->exec_mode) || !st_ses->enable_trans)
            break;

        if (st_ses->exec_mode != ST_EXEC_MODE_NONE) {
            st_ses->exec_mode = ST_EXEC_MODE_NONE;
            status = stop_session(st_ses, hw_ses, true);
            if (status)
                break;
        }

        if (new_exec_mode == ST_EXEC_MODE_NONE)
            break;

        /* switch to new hw session */
        if (ST_EXEC_MODE_CPE == new_exec_mode) {
            new_hw_ses = st_ses->hw_ses_cpe;
            st_ses->hw_ses_cpe->enable_second_stage =
                st_ses->hw_ses_adsp->enable_second_stage;
            st_ses->hw_ses_cpe->second_stage_list =
                st_ses->hw_ses_adsp->second_stage_list;
        } else if (ST_EXEC_MODE_ADSP == new_exec_mode) {
            new_hw_ses = st_ses->hw_ses_adsp;
            st_ses->hw_ses_adsp->enable_second_stage =
                st_ses->hw_ses_cpe->enable_second_stage;
            st_ses->hw_ses_adsp->second_stage_list =
                st_ses->hw_ses_cpe->second_stage_list;
        } else {
            ALOGE("%s: unknown execution mode %d", __func__,
                new_exec_mode);
            status = -EINVAL;
            break;
        }

        /*
         * start new hw session and stay in detected state as
         * client restart and stop concurrency scenarios are handled
         * in this state
         */
        status = start_session(st_ses, new_hw_ses, true);
        if (status)
            break;

        st_ses->exec_mode = new_exec_mode;
        st_ses->hw_ses_current = new_hw_ses;
        break;

    case ST_SES_EV_SEND_CHMIX_COEFF:
        status = -EINVAL;
        break;

    case ST_SES_EV_SET_DEVICE:
        /*
         * set device is a no-op in detected state due to the following reasons
         * A set device is a sequence of disable and enable device commands.
         * set device sequence is triggered with dev lock held. Therefore there
         * cannot be a concurrency with other client issued events.
         * As a deferred stop is posted prior to entering detected state,
         * one of the two events are possible
         *     1) timer expires and stop is issued : this implies stop_session
         *     2) timer is cancelled and start is issued by client: this implies
         *        new device is set as part of start_session
         */
        break;

    case ST_SES_EV_GET_PARAM_DATA:
        status = hw_ses->fptrs->get_param_data(hw_ses,
                      ev->payload.getparam.param, ev->payload.getparam.payload,
                      ev->payload.getparam.payload_size,
                      ev->payload.getparam.param_data_size);
        break;
    case ST_SES_EV_DEFERRED_STOP:
        ALOGD("%s:[%d] post deferred stop from detected state", __func__,
              st_ses->sm_handle);
        status = hw_session_notifier_enqueue(st_ses->sm_handle,
                                             ST_SES_EV_DEFERRED_STOP,
                                             ST_SES_DEFERRED_STOP_DELAY_MS);
        if (!status)
            st_ses->pending_stop = true;
        break;
    case ST_SES_EV_REQUEST_DET:
        ALOGE("%s:[%d] Event not supported in this state",
            __func__, st_ses->sm_handle);
        status = -EINVAL;
        break;
    default:
        ALOGD("%s:[%d] unhandled event", __func__, st_ses->sm_handle);
        break;

    };

    return status;
}

static int buffering_state_fn(st_session_t *st_ses, st_session_ev_t *ev)
{
    int status = 0;
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    st_exec_mode_t new_exec_mode = 0;
    st_hw_session_t *new_hw_ses = NULL;
    st_session_ev_t set_dev_ev =  { .ev_id = ST_SES_EV_SET_DEVICE };

    /* skip parameter check as this is an internal function */
    ALOGVV("%s:[%d] handle event id %d", __func__, st_ses->sm_handle,
        ev->ev_id);
    switch (ev->ev_id) {
    case ST_SES_EV_READ_PCM:

        /* Note: this function may block if there is no PCM data ready*/
        hw_ses->fptrs->read_pcm(hw_ses, ev->payload.readpcm.out_buff,
            ev->payload.readpcm.out_buff_size);
        ST_DBG_FILE_WRITE(st_ses->lab_fp, ev->payload.readpcm.out_buff,
            ev->payload.readpcm.out_buff_size);
        break;
    case ST_SES_EV_END_BUFFERING:
        hw_ses->fptrs->stop_buffering(hw_ses, st_ses->lab_enabled);
        if (!st_ses->pending_stop) {
            ALOGD("%s:[%d] post deferred stop on buffering end", __func__,
                  st_ses->sm_handle);
            status = hw_session_notifier_enqueue(st_ses->sm_handle,
                                                 ST_SES_EV_DEFERRED_STOP,
                                                 ST_SES_DEFERRED_STOP_DELAY_MS);
            if (!status)
                st_ses->pending_stop = true;
        } else {
            ALOGD("%s:[%d] skip deferred stop on buffering as already set", __func__,
                  st_ses->sm_handle);
        }
        break;
    case ST_SES_EV_PAUSE:
        st_ses->paused = true;
        /* Fall through to handle pause events similarly to stop event. */
    case ST_SES_EV_STOP:
        ALOGD("%s:[%d] handle event STOP %s", __func__, st_ses->sm_handle,
              st_ses->paused ? "(paused)" : "");
        /*
         * These events are related to a tear down sequence, so transition to
         * loaded state even if there is a failure.
         */
        status = hw_ses->fptrs->stop_buffering(hw_ses, st_ses->lab_enabled);
        if (status)
            ALOGE("%s:[%d] failed to stop_buffering err %d", __func__,
                st_ses->sm_handle, status);
        /*
         * We treat pause during buffering with the same semantics as stop.
         * With second stage enabled, the session can be in buffering state
         * without the app requesting it. So if the PAUSE event comes when 
         * session is in buffering state, client_req_state needs to be set to
         * active to ensure the session can make it back to the active state.
         */
        if (st_ses->paused)
            st_ses->client_req_state = ST_STATE_ACTIVE;
        else
            st_ses->client_req_state = ST_STATE_LOADED;
        status = stop_session(st_ses, hw_ses, false);
        if (status)
            ALOGE("%s:[%d] failed to stop session, err %d", __func__,
                st_ses->sm_handle, status);

        ST_DBG_FILE_CLOSE(st_ses->lab_fp);
        STATE_TRANSITION(st_ses, loaded_state_fn);
        break;

    case ST_SES_EV_SET_DEVICE:
        /*
         * Device switch will not wait for buffering to finish. It will instead
         * interrupt and stop the buffering and transition to the loaded state.
         * The loaded state will then take care of the device switch.
         */
        hw_ses->fptrs->stop_buffering(hw_ses, st_ses->lab_enabled);
        status = stop_session(st_ses, hw_ses, false);
        if (status && !st_ses->stdev->ssr_offline_received) {
            ALOGE("%s:[%d] failed to stop session, err %d", __func__,
                  st_ses->sm_handle, status);
            break;
        }
        STATE_TRANSITION(st_ses, loaded_state_fn);
        DISPATCH_EVENT(st_ses, set_dev_ev, status);
        break;

    case ST_SES_EV_START:
    case ST_SES_EV_RESTART:
        ALOGD("%s:[%d] handle event id %d", __func__, st_ses->sm_handle,
            ev->ev_id);
        /*
         * Client starts detection again.
         * This implies a previous deferred stop hasn't completed yet as
         * stop would have changed state to loaded.
         * For a restart event, issue stop buffering and restart the session
         * For a start event, stop buffering then stop and start the session
         * so that any new parameters take effect.
         */
        hw_ses->fptrs->stop_buffering(hw_ses, st_ses->lab_enabled);
        if (ev->ev_id == ST_SES_EV_START) {
            status = stop_session(st_ses, hw_ses, false);
            if (status && !st_ses->stdev->ssr_offline_received) {
                ALOGE("%s:[%d] failed to stop session, err %d", __func__,
                      st_ses->sm_handle, status);
                break;
            }
            status = start_session(st_ses, hw_ses, false);
        } else {
            status = restart_session(st_ses, hw_ses);
        }

        if (status) {
            if (st_ses->stdev->ssr_offline_received) {
                if (st_ses->enable_second_stage)
                    stop_second_stage_session(st_ses);
                hw_ses->fptrs->dereg_sm(hw_ses, st_ses->lab_enabled);
                st_ses->client_req_state = ST_STATE_ACTIVE;
                STATE_TRANSITION(st_ses, ssr_state_fn);
                /* Send success to client because the failure is recovered
                 * internally from SSR.
                 */
                status = 0;
            } else {
                ALOGE("%s:[%d] failed to start session, err %d", __func__,
                      st_ses->sm_handle, status);
                /* move to active anyways to allow unload sm */
                STATE_TRANSITION(st_ses, active_state_fn);
            }
        } else {
            /* Move state back to active for restart and start events */
            STATE_TRANSITION(st_ses, active_state_fn);
        }
        break;

    case ST_SES_EV_SSR_OFFLINE:
        ALOGD("%s:[%d] handle event id %d", __func__, st_ses->sm_handle,
            ev->ev_id);

        if (st_ses->enable_second_stage) {
            /*
             * Second stage sessions will always buffer in order to be
             * able to make detections. If the SSR occurs before the
             * client is notified of a detection, it needs to recover to
             * the active state. If the SSR occurs after this, or if the
             * second stage detection is rejected, then recovering to
             * the loaded state is sufficient.
             */
            if (!st_ses->sent_detection_to_client || st_ses->paused)
                st_ses->client_req_state = ST_STATE_ACTIVE;
            else
                st_ses->client_req_state = ST_STATE_LOADED;
        } else {
            if (st_ses->paused)
                st_ses->client_req_state = ST_STATE_ACTIVE;
            else
                st_ses->client_req_state = ST_STATE_LOADED;
        }

        /*
         *  Ignore return status during SSR handling
         *  as the ADSP or CPE might be down so these
         *  calls would fail. Exec mode can be none if
         *  ssr occurs during a transition.
         */
        if (st_ses->exec_mode != ST_EXEC_MODE_NONE) {
            hw_ses->fptrs->stop_buffering(hw_ses,
                                          st_ses->lab_enabled);
            if (st_ses->enable_second_stage)
                stop_second_stage_session(st_ses);
            stop_hw_session(st_ses, hw_ses, true /* unload sm */);
        }
        STATE_TRANSITION(st_ses, ssr_state_fn);
        break;

    case ST_SES_EV_SET_EXEC_MODE:
        ALOGD("%s:[%d] handle event id %d", __func__, st_ses->sm_handle,
            ev->ev_id);

        new_exec_mode = ev->payload.exec_mode;

        /* if no change in mode or dynamic transition not enabled then noop */
        if ((new_exec_mode == st_ses->exec_mode) || !st_ses->enable_trans)
            break;

        if (st_ses->exec_mode != ST_EXEC_MODE_NONE) {
            st_ses->exec_mode = ST_EXEC_MODE_NONE;
            status = hw_ses->fptrs->stop_buffering(hw_ses, st_ses->lab_enabled);
            if (status) {
                ALOGE("%s:[%d] failed to stop_buffering err %d", __func__,
                    st_ses->sm_handle, status);
                break;
            }

            status = stop_session(st_ses, hw_ses, true);
            if (status) {
                ALOGE("%s:[%d] failed to stop session, err %d", __func__,
                    st_ses->sm_handle, status);
                break;
            }
        }

        if (new_exec_mode == ST_EXEC_MODE_NONE)
            break;

        /* switch to new hw session */
        if (ST_EXEC_MODE_CPE == new_exec_mode) {
            new_hw_ses = st_ses->hw_ses_cpe;
            st_ses->hw_ses_cpe->enable_second_stage =
                st_ses->hw_ses_adsp->enable_second_stage;
            st_ses->hw_ses_cpe->second_stage_list =
                st_ses->hw_ses_adsp->second_stage_list;
        } else if (ST_EXEC_MODE_ADSP == new_exec_mode) {
            new_hw_ses = st_ses->hw_ses_adsp;
            st_ses->hw_ses_adsp->enable_second_stage =
                st_ses->hw_ses_cpe->enable_second_stage;
            st_ses->hw_ses_adsp->second_stage_list =
                st_ses->hw_ses_cpe->second_stage_list;
        } else {
            ALOGE("%s: unknown execution mode %d", __func__,
                new_exec_mode);
            status = -EINVAL;
            break;
        }

        status = start_session(st_ses, new_hw_ses, true);
        if (status) {
            ALOGE("%s:[%d] failed to start hw ses, err %d", __func__,
                st_ses->sm_handle, status);
            break;
        }
        st_ses->exec_mode = new_exec_mode;
        st_ses->hw_ses_current = new_hw_ses;
        STATE_TRANSITION(st_ses, active_state_fn);
        break;

    case ST_SES_EV_SEND_CHMIX_COEFF:
        status = hw_ses->fptrs->send_custom_chmix_coeff(hw_ses,
                                  ev->payload.chmix_coeff_str);
        break;

    case ST_SES_EV_GET_PARAM_DATA:
        status = hw_ses->fptrs->get_param_data(hw_ses,
                      ev->payload.getparam.param, ev->payload.getparam.payload,
                      ev->payload.getparam.payload_size,
                      ev->payload.getparam.param_data_size);
        break;

    case ST_SES_EV_REQUEST_DET:
        ALOGE("%s:[%d] Event %d not supported in this state",
            __func__, st_ses->sm_handle, ev->ev_id);
        status = -EINVAL;
        break;

    default:
        ALOGD("%s:[%d] unhandled event, id %d", __func__, st_ses->sm_handle,
            ev->ev_id);
        break;

    };

    return status;
}

static int ssr_state_fn(st_session_t *st_ses, st_session_ev_t *ev)
{
    int status = 0;
    st_session_ev_t load_ev =  { .ev_id = ST_SES_EV_LOAD_SM };
    st_session_ev_t start_ev =  { .ev_id = ST_SES_EV_START };
    st_session_ev_t exec_mode_ev =  { .ev_id = ST_SES_EV_SET_EXEC_MODE };

    /* skip parameter check as this is an internal funciton */
    ALOGD("%s:[%d] handle event id %d", __func__, st_ses->sm_handle, ev->ev_id);

    switch (ev->ev_id) {
    case ST_SES_EV_SSR_ONLINE:
        ALOGV("%s:[%d] SSR ONLINE received", __func__, st_ses->sm_handle);

        STATE_TRANSITION(st_ses, idle_state_fn);

        if ((st_ses->ssr_transit_exec_mode == ST_EXEC_MODE_CPE) ||
            (st_ses->ssr_transit_exec_mode == ST_EXEC_MODE_ADSP)) {
            exec_mode_ev.payload.exec_mode = st_ses->ssr_transit_exec_mode;
            DISPATCH_EVENT(st_ses, exec_mode_ev, status);
            st_ses->ssr_transit_exec_mode = ST_EXEC_MODE_NONE;
        }

        if ((ST_STATE_ACTIVE == st_ses->client_req_state) ||
            (ST_STATE_LOADED == st_ses->client_req_state)) {
            DISPATCH_EVENT(st_ses, load_ev, status);
            if (status)
                break;
        }

        if ((ST_STATE_ACTIVE == st_ses->client_req_state) &&
            !st_ses->paused) {
            DISPATCH_EVENT(st_ses, start_ev, status);
            if (status)
                break;
        }

        break;

    case ST_SES_EV_LOAD_SM:
        if (ST_STATE_IDLE == st_ses->client_req_state) {
            st_ses->client_req_state = ST_STATE_LOADED;
        } else {
            ALOGE("%s: received unexpected event, client_req_state = %d",
                __func__, st_ses->client_req_state);
        }
        break;

    case ST_SES_EV_UNLOAD_SM:
        if (ST_STATE_LOADED == st_ses->client_req_state) {
            st_ses->client_req_state = ST_STATE_IDLE;
        } else {
            ALOGE("%s: received unexpected event, client_req_state = %d",
                __func__, st_ses->client_req_state);
        }
        break;

    case ST_SES_EV_START:
    case ST_SES_EV_RESTART:
        if (ST_STATE_LOADED == st_ses->client_req_state) {
            st_ses->client_req_state = ST_STATE_ACTIVE;
        } else {
            ALOGE("%s: received unexpected event, client_req_state = %d",
                __func__, st_ses->client_req_state);
        }
        break;

    case ST_SES_EV_STOP:
        if (ST_STATE_ACTIVE == st_ses->client_req_state) {
            st_ses->client_req_state = ST_STATE_LOADED;
        } else {
            ALOGE("%s: received unexpected event, client_req_state = %d",
                __func__, st_ses->client_req_state);
        }
        break;

    case ST_SES_EV_PAUSE:
        st_ses->paused = true;
        break;

    case ST_SES_EV_RESUME:
        st_ses->paused = false;
        break;

    case ST_SES_EV_READ_PCM:
        status = -EIO;
        break;

    case ST_SES_EV_SEND_CHMIX_COEFF:
        status = -EIO;
        break;

    case ST_SES_EV_GET_PARAM_DATA:
        status = -EIO;
        break;

    case ST_SES_EV_SET_EXEC_MODE:
        st_ses->exec_mode = ev->payload.exec_mode;
        if (ST_EXEC_MODE_CPE == st_ses->exec_mode)
            st_ses->hw_ses_current = st_ses->hw_ses_cpe;
        else if (ST_EXEC_MODE_ADSP == st_ses->exec_mode)
            st_ses->hw_ses_current = st_ses->hw_ses_adsp;
        /* remain in current state */
        break;

    case ST_SES_EV_REQUEST_DET:
        ALOGE("%s:[%d] Event not supported in this state",
            __func__, st_ses->sm_handle);
        status = -EINVAL;
        break;

    default:
        ALOGD("%s:[%d] unhandled event", __func__, st_ses->sm_handle);
        break;
    };

    return status;
}

/*
 * This function sets the opaque data size for the DSP's generic detection
 * events. This opaque data can now have varying size based on the requested
 * params.
 */
static size_t set_opaque_data_size(void *payload, size_t payload_size,
    uint32_t version)
{
    size_t count_size = 0, opaque_size = 0;
    uint8_t *payload_ptr = (uint8_t *)payload;
    uint32_t key_id = 0, key_payload_size = 0;

    while (count_size < payload_size) {
        key_id = *(uint32_t *)payload_ptr;
        key_payload_size = *((uint32_t *)payload_ptr + 1);

        switch (key_id) {
        case KEY_ID_CONFIDENCE_LEVELS:
            opaque_size += sizeof(struct st_param_header);
            if (version != CONF_LEVELS_INTF_VERSION_0002) {
                opaque_size +=
                    sizeof(struct st_confidence_levels_info);
            } else {
                opaque_size +=
                    sizeof(struct st_confidence_levels_info_v2);
            }
            count_size += GENERIC_DET_EVENT_HEADER_SIZE + key_payload_size;
            payload_ptr += count_size;
            break;

        case KEY_ID_KEYWORD_POSITION_STATS:
            opaque_size += sizeof(struct st_param_header) +
                sizeof(struct st_keyword_indices_info);
            count_size += GENERIC_DET_EVENT_HEADER_SIZE + key_payload_size;
            payload_ptr += count_size;
            break;

        default:
            ALOGE("%s: Unsupported generic detection event key id", __func__);
        }
    }

    opaque_size += sizeof(struct st_param_header) + sizeof(struct st_timestamp_info);

    return opaque_size;
}

/*
 * This function packs the updated opaque data confidence levels which are
 * passed to the client via callback.
 */
static int pack_opaque_data_conf_levels(
    st_session_t *st_ses, void *opaque_data,
    uint8_t *payload)
{
    uint8_t *payload_ptr = payload;
    unsigned int i, j, k, user_id;
    st_arm_second_stage_t *st_sec_stage;
    struct listnode *node = NULL, *tmp_node = NULL;
    struct st_confidence_levels_info *conf_levels = NULL;
    struct st_confidence_levels_info_v2 *conf_levels_v2 = NULL;
    int32_t kw_level = 0, user_level = 0;

    if (st_ses->enable_second_stage) {
        list_for_each_safe(node, tmp_node, &st_ses->second_stage_list) {
            st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
            if (st_sec_stage->ss_info->sm_id == ST_SM_ID_SVA_CNN) {
                kw_level = st_sec_stage->ss_session->confidence_score;
            } else if (st_sec_stage->ss_info->sm_id == ST_SM_ID_SVA_VOP) {
                user_level = st_sec_stage->ss_session->confidence_score;
            }
        }
    }
    if (st_ses->conf_levels_intf_version != CONF_LEVELS_INTF_VERSION_0002) {
        conf_levels = (struct st_confidence_levels_info *)opaque_data;
        for (i = 0; i < conf_levels->num_sound_models; i++) {
            if (conf_levels->conf_levels[i].sm_id == ST_SM_ID_SVA_GMM) {
                for (j = 0;
                     j < conf_levels->conf_levels[i].num_kw_levels; j++) {
                    conf_levels->conf_levels[i].kw_levels[j].kw_level =
                        payload_ptr[j];
                    for (k = 0;
                         k < conf_levels->conf_levels[i].kw_levels[j].num_user_levels;
                         k++) {
                        user_id =
                            conf_levels->conf_levels[i].kw_levels[j].
                                user_levels[k].user_id;
                        conf_levels->conf_levels[i].kw_levels[j].
                            user_levels[k].level = payload_ptr[user_id];
                    }
                }
            } else if (conf_levels->conf_levels[i].sm_id == ST_SM_ID_SVA_CNN) {
                conf_levels->conf_levels[i].kw_levels[0].kw_level = kw_level;
            } else if (conf_levels->conf_levels[i].sm_id == ST_SM_ID_SVA_VOP) {
                /*
                 * Fill both the keyword and user confidence level with the
                 * confidence score returned from the voiceprint algorithm.
                 */
                conf_levels->conf_levels[i].kw_levels[0].kw_level =
                    (uint8_t)user_level;
                conf_levels->conf_levels[i].kw_levels[0].user_levels[0].level =
                    (uint8_t)user_level;
            }
        }
    } else {
        conf_levels_v2 = (struct st_confidence_levels_info_v2 *)opaque_data;
        for (i = 0; i < conf_levels_v2->num_sound_models; i++) {
            if (conf_levels_v2->conf_levels[i].sm_id == ST_SM_ID_SVA_GMM) {
                for (j = 0;
                     j < conf_levels_v2->conf_levels[i].num_kw_levels; j++) {
                    conf_levels_v2->conf_levels[i].kw_levels[j].kw_level =
                        payload_ptr[j];
                    for (k = 0;
                         k < conf_levels_v2->conf_levels[i].kw_levels[j].num_user_levels;
                         k++) {
                        user_id =
                            conf_levels_v2->conf_levels[i].kw_levels[j].
                                user_levels[k].user_id;
                        conf_levels_v2->conf_levels[i].kw_levels[j].
                            user_levels[k].level = payload_ptr[user_id];
                    }
                }
            } else if (conf_levels_v2->conf_levels[i].sm_id == ST_SM_ID_SVA_CNN) {
                conf_levels_v2->conf_levels[i].kw_levels[0].kw_level = kw_level;
            } else if (conf_levels_v2->conf_levels[i].sm_id == ST_SM_ID_SVA_VOP) {
                /*
                 * Fill both the keyword and user confidence level with the
                 * confidence score returned from the voiceprint algorithm.
                 */
                conf_levels_v2->conf_levels[i].kw_levels[0].kw_level = user_level;
                conf_levels_v2->conf_levels[i].kw_levels[0].user_levels[0].level =
                    user_level;
            }
        }
    }

    return 0;
}

/* This function packs the sound trigger API confidence levels */
static int pack_recognition_event_conf_levels(
    st_session_t *st_ses, uint8_t *payload_ptr,
    struct sound_trigger_phrase_recognition_event *local_event)
{
    unsigned int j = 0, k = 0, user_id = 0;
    st_arm_second_stage_t *st_sec_stage;
    struct listnode *node = NULL, *tmp_node = NULL;
    struct sound_trigger_phrase_sound_model *phrase_sm =
        (struct sound_trigger_phrase_sound_model *)st_ses->sm_data;

    /*
     * Fill in the GMM confidence levels to the sound trigger recognition event
     * APIs first. If any second stage session is enabled, overwrite the APIs
     * with the second stage confidence levels.
     */
    for (j = 0; j < st_ses->rc_config->num_phrases; j++) {
        local_event->phrase_extras[j].id = st_ses->rc_config->phrases[j].id;
        local_event->phrase_extras[j].recognition_modes =
            phrase_sm->phrases[j].recognition_mode;
        local_event->phrase_extras[j].num_levels =
            st_ses->rc_config->phrases[j].num_levels;
        local_event->phrase_extras[j].confidence_level = payload_ptr[j];
        for (k = 0; k < st_ses->rc_config->phrases[j].num_levels; k++) {
            user_id = st_ses->rc_config->phrases[j].levels[k].user_id;
            local_event->phrase_extras[j].levels[k].user_id = user_id;
            local_event->phrase_extras[j].levels[k].level =
                payload_ptr[user_id];
        }
    }

    if (st_ses->enable_second_stage) {
        list_for_each_safe(node, tmp_node, &st_ses->second_stage_list) {
            st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
            if (st_sec_stage->ss_info->sm_id == ST_SM_ID_SVA_CNN) {
                local_event->phrase_extras[0].confidence_level =
                    (uint8_t)st_sec_stage->ss_session->confidence_score;
            } else if (st_sec_stage->ss_info->sm_id == ST_SM_ID_SVA_VOP) {
                local_event->phrase_extras[0].levels[0].level =
                    (uint8_t)st_sec_stage->ss_session->confidence_score;
            }
        }
    }

    return 0;
}

static int parse_generic_event_and_pack_opaque_data(
    st_session_t *st_ses, uint8_t **opaque_data,
    uint8_t *payload_ptr, size_t payload_size,
    struct sound_trigger_phrase_recognition_event *local_event)
{
    uint32_t key_id = 0, key_payload_size = 0;
    struct st_param_header *param_hdr = NULL;
    struct st_keyword_indices_info *kw_indices = NULL;
    struct st_timestamp_info *timestamps = NULL;
    size_t count_size = 0;
    st_arm_second_stage_t *st_sec_stage;
    struct listnode *node = NULL, *tmp_node = NULL;
    st_hw_session_t *st_hw_ses = st_ses->hw_ses_current;
    int status = 0;
    uint8_t *conf_payload_ptr = NULL;

    while (count_size < payload_size) {
        key_id = *(uint32_t *)payload_ptr;
        key_payload_size = *((uint32_t *)payload_ptr + 1);

        switch (key_id) {
        case KEY_ID_CONFIDENCE_LEVELS:
            /* Pack the opaque data confidence levels structure */
            param_hdr = (struct st_param_header *)(*opaque_data);
            param_hdr->key_id = ST_PARAM_KEY_CONFIDENCE_LEVELS;
            *opaque_data += sizeof(struct st_param_header);
            if (st_ses->conf_levels_intf_version !=
                CONF_LEVELS_INTF_VERSION_0002) {
                param_hdr->payload_size =
                    sizeof(struct st_confidence_levels_info);
            } else {
                param_hdr->payload_size =
                    sizeof(struct st_confidence_levels_info_v2);
            }
            memcpy(*opaque_data, st_hw_ses->conf_levels_info,
                param_hdr->payload_size);
            conf_payload_ptr = payload_ptr + (4 * sizeof(uint32_t));
            pack_opaque_data_conf_levels(st_ses, (void *)*opaque_data,
                conf_payload_ptr);
            pack_recognition_event_conf_levels(st_ses, conf_payload_ptr,
                local_event);
            *opaque_data += param_hdr->payload_size;
            break;

        case KEY_ID_KEYWORD_POSITION_STATS:
            /* Pack the opaque data keyword indices structure */
            param_hdr = (struct st_param_header *)(*opaque_data);
            param_hdr->key_id = ST_PARAM_KEY_KEYWORD_INDICES;
            param_hdr->payload_size = sizeof(struct st_keyword_indices_info);
            *opaque_data += sizeof(struct st_param_header);
            kw_indices = (struct st_keyword_indices_info *)(*opaque_data);
            kw_indices->version = 0x1;
            kw_indices->start_index = *((uint32_t *)payload_ptr + 3);
            kw_indices->end_index = *((uint32_t *)payload_ptr + 4);

            if (st_ses->enable_second_stage) {
                list_for_each_safe(node, tmp_node, &st_ses->second_stage_list) {
                    st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
                    if (st_sec_stage->ss_info->sm_id == ST_SM_ID_SVA_CNN) {
                        kw_indices->start_index = st_sec_stage->ss_session->kw_start_idx;
                        kw_indices->end_index = st_sec_stage->ss_session->kw_end_idx;
                    }
                }
            }

            *opaque_data += sizeof(struct st_keyword_indices_info);
            break;

        default:
            ALOGE("%s: Unsupported generic detection event key id", __func__);
            status = -EINVAL;
            goto exit;
        }
        count_size += GENERIC_DET_EVENT_HEADER_SIZE + key_payload_size;
        payload_ptr += count_size;
    }

    /* Pack the opaque data detection timestamp structure */
    param_hdr = (struct st_param_header *)(*opaque_data);
    param_hdr->key_id = ST_PARAM_KEY_TIMESTAMP;
    param_hdr->payload_size = sizeof(struct st_timestamp_info);
    *opaque_data += sizeof(struct st_param_header);
    timestamps = (struct st_timestamp_info *)(*opaque_data);
    timestamps->version = 0x1;
    timestamps->first_stage_det_event_time =
        st_ses->hw_ses_current->first_stage_det_event_time;
    if (st_ses->enable_second_stage)
        timestamps->second_stage_det_event_time =
            st_ses->hw_ses_current->second_stage_det_event_time;
    *opaque_data += sizeof(struct st_timestamp_info);

exit:
    return status;
}

static int parse_generic_event_without_opaque_data(
    st_session_t *st_ses, uint8_t *payload_ptr, size_t payload_size,
    struct sound_trigger_phrase_recognition_event *local_event)
{
    uint32_t key_id = 0, key_payload_size = 0;
    size_t count_size = 0;
    int status = 0;
    uint8_t *conf_payload_ptr;

    while (count_size < payload_size) {
        key_id = *(uint32_t *)payload_ptr;
        key_payload_size = *((uint32_t *)payload_ptr + 1);

        switch (key_id) {
        case KEY_ID_CONFIDENCE_LEVELS:
            conf_payload_ptr = payload_ptr + (4 * sizeof(uint32_t));
            pack_recognition_event_conf_levels(st_ses, conf_payload_ptr,
                local_event);
            return status;

        case KEY_ID_KEYWORD_POSITION_STATS:
            count_size += GENERIC_DET_EVENT_HEADER_SIZE + key_payload_size;
            payload_ptr += count_size;
            break;

        default:
            ALOGE("%s: Unsupported generic detection event key id", __func__);
            status = -EINVAL;
            return status;
        }
    }

    return status;
}

/*
 * This function handles detection payloads in the format of the DSP's
 * generic detection event.
 */
int process_detection_event_keyphrase_v2(
    st_session_t *st_ses, int detect_status,
    void *payload, size_t payload_size,
    struct sound_trigger_phrase_recognition_event **event)
{
    st_hw_session_t *st_hw_ses = st_ses->hw_ses_current;
    unsigned int i, j;
    int status = 0;
    uint8_t *opaque_data = NULL, *payload_ptr = NULL;
    size_t opaque_size = 0;
    struct sound_trigger_phrase_recognition_event *local_event = NULL;

    if (st_ses->vendor_uuid_info->is_qcva_uuid)
        opaque_size = set_opaque_data_size(payload, payload_size,
            st_ses->conf_levels_intf_version);
    else
        opaque_size = payload_size;

    local_event = calloc(1, sizeof(struct sound_trigger_phrase_recognition_event) + opaque_size);
    if (!local_event) {
        ALOGE("%s: local_event allocation failed, opaque data size = %d", __func__,
            (unsigned int)opaque_size);
        return -ENOMEM;
    }

    local_event->num_phrases = st_ses->rc_config->num_phrases;
    local_event->common.data_offset = sizeof(struct sound_trigger_phrase_recognition_event);
    local_event->common.data_size = opaque_size;
    opaque_data = (uint8_t *)local_event + local_event->common.data_offset;
    payload_ptr = (uint8_t *)payload;

    if (st_ses->vendor_uuid_info->is_qcva_uuid) {
        if (st_ses->rc_config->data_size > CUSTOM_CONFIG_OPAQUE_DATA_SIZE) {
            status = parse_generic_event_and_pack_opaque_data(st_ses, &opaque_data,
                payload_ptr, payload_size, local_event);
            if (status) {
                ALOGE("%s: Failed to parse generic detection event with opaque data %d",
                    __func__, status);
                goto exit;
            }

            ST_DBG_DECLARE(FILE *opaque_fd = NULL; static int opaque_cnt = 0);
            ST_DBG_FILE_OPEN_WR(opaque_fd, ST_DEBUG_DUMP_LOCATION,
                                "detection_opaque_data", "bin", opaque_cnt++);
            ST_DBG_FILE_WRITE(opaque_fd, (uint8_t *)local_event +
                local_event->common.data_offset, opaque_size);
            ST_DBG_FILE_CLOSE(opaque_fd);
        } else {
            status = parse_generic_event_without_opaque_data(st_ses, payload_ptr,
                payload_size, local_event);
            if (status) {
                ALOGE("%s: Failed to parse generic detection event without opaque data %d",
                    __func__, status);
                goto exit;
            }
        }

    } else {
        local_event = calloc(1, sizeof(*local_event) + payload_size);
        if (!local_event) {
            ALOGE("%s: event allocation failed, size %zd", __func__,
                payload_size);
            status = -ENOMEM;
            goto exit;
        }
        memcpy(local_event->phrase_extras,
            st_ses->rc_config->phrases, st_ses->rc_config->num_phrases *
            sizeof(struct sound_trigger_phrase_recognition_extra));
        local_event->num_phrases = st_ses->rc_config->num_phrases;
        local_event->common.data_offset = sizeof(*local_event);
        local_event->common.data_size = payload_size;
        memcpy((char *)local_event + local_event->common.data_offset, payload,
            payload_size);
        opaque_data += opaque_size;
    }

    /* fill the remaining recognition event parameters not specific
       to soundmodel lib */
    local_event->common.status = detect_status;
    local_event->common.type = st_ses->sm_data->common.type;
    local_event->common.model = st_ses->sm_handle;
    local_event->common.capture_available = st_ses->capture_requested;
    local_event->common.capture_delay_ms = 0;
    local_event->common.capture_preamble_ms = 0;
    local_event->common.audio_config.sample_rate =
        SOUND_TRIGGER_SAMPLING_RATE_16000;
    local_event->common.audio_config.format = AUDIO_FORMAT_PCM_16_BIT;
    local_event->common.audio_config.channel_mask =
        audio_channel_in_mask_from_count(st_hw_ses->config.channels);

    for (i = 0; i < local_event->num_phrases; ++i) {
        ALOGV("%s: [%d] kw_id %d level %d", __func__, i,
            local_event->phrase_extras[i].id,
            local_event->phrase_extras[i].confidence_level);
        for (j = 0; j < local_event->phrase_extras[i].num_levels; ++j) {
            ALOGV("%s: [%d] user_id %d level %d ", __func__, i,
                local_event->phrase_extras[i].levels[j].user_id,
                local_event->phrase_extras[i].levels[j].level);
        }
    }

    ALOGI("%s:[%d]", __func__, st_ses->sm_handle);

    ALOGV("%s:[%d] status=%d, type=%d, model=%d, capture_avaiable=%d, "
        "num_phrases=%d id=%d", __func__,
        st_ses->sm_handle, local_event->common.status, local_event->common.type,
        local_event->common.model, local_event->common.capture_available,
        local_event->num_phrases, local_event->phrase_extras[0].id);

    *event = local_event;
    return 0;

exit:

    if (local_event)
        free(local_event);
    return status;
}

/*
 * This function handles detection payloads in the format of the DSP's
 * legacy (non-generic) detection event.
 */
static int process_detection_event_keyphrase(
    st_session_t *st_ses, int detect_status,
    void *payload, size_t payload_size,
    struct sound_trigger_phrase_recognition_event **event)
{
    st_hw_session_t *st_hw_ses = st_ses->hw_ses_current;
    unsigned int i, j;
    int status = 0;
    struct sound_trigger_phrase_recognition_event *local_event = NULL;
    size_t opaque_size = 0;
    uint8_t *opaque_data = NULL, *payload_ptr = NULL;
    struct st_param_header *param_hdr = NULL;
    st_arm_second_stage_t *st_sec_stage;
    struct listnode *node = NULL, *tmp_node = NULL;
    struct st_keyword_indices_info *kw_indices = NULL;
    struct st_timestamp_info *timestamps = NULL;
    bool enable_kw_indices = false;

    if ((st_ses->rc_config->data_size > CUSTOM_CONFIG_OPAQUE_DATA_SIZE) &&
            st_ses->vendor_uuid_info->is_qcva_uuid) {
        /*
         * This logic is for the updated opaque data format. Sound trigger recognition
         * event APIs are filled along with the opaque data's confidence levels, keyword
         * indices, and timestamp parameters.
         */
        opaque_size = (2 * sizeof(struct st_param_header)) +
            sizeof(struct st_timestamp_info);
        if (st_ses->conf_levels_intf_version != CONF_LEVELS_INTF_VERSION_0002)
            opaque_size += sizeof(struct st_confidence_levels_info);
        else
            opaque_size += sizeof(struct st_confidence_levels_info_v2);

        if (st_ses->enable_second_stage) {
            list_for_each_safe(node, tmp_node, &st_ses->second_stage_list) {
                st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
                if (st_sec_stage->ss_info->sm_id == ST_SM_ID_SVA_CNN) {
                    enable_kw_indices = true;
                    opaque_size += sizeof(struct st_param_header) +
                        sizeof(struct st_keyword_indices_info);
                    break;
                }
            }
        }

        local_event = calloc(1,
            sizeof(struct sound_trigger_phrase_recognition_event) + opaque_size);
        if (!local_event) {
            ALOGE("%s: local_event allocation failed, opaque data size = %d", __func__,
                (unsigned int)opaque_size);
            return -ENOMEM;
        }

        local_event->num_phrases = st_ses->rc_config->num_phrases;
        local_event->common.data_offset = sizeof(struct sound_trigger_phrase_recognition_event);
        local_event->common.data_size = opaque_size;
        opaque_data = (uint8_t *)local_event + local_event->common.data_offset;
        if ((st_ses->exec_mode == ST_EXEC_MODE_CPE) && st_ses->stdev->is_gcs) {
            payload_ptr = (uint8_t *)payload + 2;
        } else if ((st_ses->exec_mode == ST_EXEC_MODE_ADSP) || !st_ses->stdev->is_gcs) {
            payload_ptr = (uint8_t *)payload;
        } else {
            ALOGE("%s: Invalid execution mode, exiting", __func__);
            status = -EINVAL;
            goto err_exit;
        }

        /* Pack the opaque data confidence levels structure */
        param_hdr = (struct st_param_header *)opaque_data;
        param_hdr->key_id = ST_PARAM_KEY_CONFIDENCE_LEVELS;
        opaque_data += sizeof(struct st_param_header);
        if (st_ses->conf_levels_intf_version != CONF_LEVELS_INTF_VERSION_0002) {
            param_hdr->payload_size =
                sizeof(struct st_confidence_levels_info);
        } else {
            param_hdr->payload_size =
                sizeof(struct st_confidence_levels_info_v2);
        }
        memcpy(opaque_data, st_hw_ses->conf_levels_info,
            param_hdr->payload_size);
        pack_opaque_data_conf_levels(st_ses, (void *)opaque_data, payload_ptr);
        pack_recognition_event_conf_levels(st_ses, payload_ptr,
            local_event);
        opaque_data += param_hdr->payload_size;

        /* Pack the opaque data keyword indices structure */
        if (enable_kw_indices) {
            param_hdr = (struct st_param_header *)opaque_data;
            param_hdr->key_id = ST_PARAM_KEY_KEYWORD_INDICES;
            param_hdr->payload_size = sizeof(struct st_keyword_indices_info);
            opaque_data += sizeof(struct st_param_header);
            kw_indices = (struct st_keyword_indices_info *)opaque_data;
            kw_indices->version = 0x1;

            if (st_ses->enable_second_stage) {
                list_for_each_safe(node, tmp_node, &st_ses->second_stage_list) {
                    st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
                    if (st_sec_stage->ss_info->sm_id == ST_SM_ID_SVA_CNN) {
                        kw_indices->start_index = st_sec_stage->ss_session->kw_start_idx;
                        kw_indices->end_index = st_sec_stage->ss_session->kw_end_idx;
                    }
                }
            }
            opaque_data += sizeof(struct st_keyword_indices_info);
        }

        /* Pack the opaque data detection timestamp structure */
        param_hdr = (struct st_param_header *)opaque_data;
        param_hdr->key_id = ST_PARAM_KEY_TIMESTAMP;
        param_hdr->payload_size = sizeof(struct st_timestamp_info);
        opaque_data += sizeof(struct st_param_header);
        timestamps = (struct st_timestamp_info *)opaque_data;
        timestamps->version = 0x1;
        timestamps->first_stage_det_event_time =
            st_ses->hw_ses_current->first_stage_det_event_time;
        if (st_ses->enable_second_stage)
            timestamps->second_stage_det_event_time =
                st_ses->hw_ses_current->second_stage_det_event_time;
        opaque_data += sizeof(struct st_timestamp_info);

        ST_DBG_DECLARE(FILE *opaque_fd = NULL; static int opaque_cnt = 0);
        ST_DBG_FILE_OPEN_WR(opaque_fd, ST_DEBUG_DUMP_LOCATION,
                            "detection_opaque_data", "bin", opaque_cnt++);
        ST_DBG_FILE_WRITE(opaque_fd, (opaque_data - opaque_size), opaque_size);
        ST_DBG_FILE_CLOSE(opaque_fd);

    } else {
        /* This logic is for the legacy opaque data format or third party vendors */
        if (st_ses->vendor_uuid_info &&
            st_ses->vendor_uuid_info->smlib_handle) {
            /* if smlib is present, get the event from it else send the
               DSP recieved payload as it is to App */
            /* TODO: checking is_gcs should be avoided here */
            if (st_ses->stdev->is_gcs &&
                ST_EXEC_MODE_CPE == st_ses->exec_mode &&
                !st_ses->hw_ses_current->is_generic_event) {
                ALOGD("%s: about to call generate_st_phrase_recognition_event_v2",
                      __func__);
                status = st_ses->vendor_uuid_info->
                    generate_st_phrase_recognition_event_v2(st_ses->sm_data,
                    st_ses->rc_config, payload, payload_size, &local_event);
            } else {
                status = st_ses->vendor_uuid_info->
                    generate_st_phrase_recognition_event(st_ses->sm_data,
                    st_ses->rc_config, payload, payload_size, &local_event);
            }

            if (status) {
                ALOGW("%s: smlib fill recognition event failed, status %d",
                    __func__, status);
                goto exit;
            }
        } else if (!st_ses->vendor_uuid_info &&
                   st_ses->stdev->smlib_handle) {
            /* This is SVA non topology solution */
            /* TODO: checking is_gcs should be avoided here */
            if (st_ses->stdev->is_gcs) {
                status = st_ses->stdev->generate_st_phrase_recognition_event_v2(
                    st_ses->sm_data, st_ses->rc_config, payload, payload_size,
                    &local_event);
            } else {
                status = st_ses->stdev->generate_st_phrase_recognition_event(
                    st_ses->sm_data, st_ses->rc_config, payload,  payload_size,
                    &local_event);
            }

            if (status) {
                ALOGW("%s: SVA smlib fill recognition event failed, status\
                    %d", __func__, status);
                goto exit;
            }
        } else {
            local_event = calloc(1, sizeof(*local_event) + payload_size);
            if (!local_event) {
                ALOGE("%s: event allocation failed, size %zd", __func__,
                    payload_size);
                status = -ENOMEM;
                goto exit;
            }
            memcpy(local_event->phrase_extras,
                st_ses->rc_config->phrases, st_ses->rc_config->num_phrases *
                sizeof(struct sound_trigger_phrase_recognition_extra));
            local_event->num_phrases = st_ses->rc_config->num_phrases;
            local_event->common.data_offset = sizeof(*local_event);
            local_event->common.data_size = payload_size;
            memcpy((char *)local_event + local_event->common.data_offset, payload,
                payload_size);
        }
    }

    /* fill the remaining recognition event parameters not specific
       to soundmodel lib */
    local_event->common.status = detect_status;
    local_event->common.type = st_ses->sm_data->common.type;
    local_event->common.model = st_ses->sm_handle;
    local_event->common.capture_available = st_ses->capture_requested;
    local_event->common.capture_delay_ms = 0;
    local_event->common.capture_preamble_ms = 0;
    local_event->common.audio_config.sample_rate =
        SOUND_TRIGGER_SAMPLING_RATE_16000;
    local_event->common.audio_config.format = AUDIO_FORMAT_PCM_16_BIT;
    local_event->common.audio_config.channel_mask =
        audio_channel_in_mask_from_count(st_hw_ses->config.channels);

    for (i = 0; i < local_event->num_phrases; ++i) {
        ALOGV("%s: [%d] kw_id %d level %d", __func__, i,
            local_event->phrase_extras[i].id,
            local_event->phrase_extras[i].confidence_level);
        for (j = 0; j < local_event->phrase_extras[i].num_levels; ++j) {
            ALOGV("%s: [%d] user_id %d level %d ", __func__, i,
                local_event->phrase_extras[i].levels[j].user_id,
                local_event->phrase_extras[i].levels[j].level);
        }
    }

    ALOGI("%s:[%d]", __func__, st_ses->sm_handle);

    ALOGV("%s:[%d] status=%d, type=%d, model=%d, capture_avaiable=%d, "
        "num_phrases=%d id=%d", __func__,
        st_ses->sm_handle, local_event->common.status, local_event->common.type,
        local_event->common.model, local_event->common.capture_available,
        local_event->num_phrases, local_event->phrase_extras[0].id);

    *event = local_event;
    return 0;

err_exit:
    if (local_event)
        free(local_event);

exit:
    return status;
}

static int process_detection_event_generic(st_session_t *st_ses,
    int detect_status,
    void *payload, size_t payload_size,
    struct sound_trigger_recognition_event **event)
{
    st_hw_session_t *st_hw_ses = st_ses->hw_ses_current;
    struct st_vendor_info *v_info = st_ses->vendor_uuid_info;
    int status = 0;
    struct sound_trigger_recognition_event *local_event = NULL;

    local_event = calloc(1, sizeof(*local_event) + payload_size);
    if (!local_event) {
        ALOGE("%s: event allocation failed, size %zd", __func__,
            payload_size);
        status = -ENOMEM;
        goto exit;
    }

    local_event->status = detect_status;
    local_event->type = st_ses->sm_type;
    local_event->model = st_ses->sm_handle;
    local_event->capture_available = st_ses->capture_requested;
    local_event->capture_delay_ms = 0;
    local_event->capture_preamble_ms = 0;
    local_event->audio_config.sample_rate = v_info ?
        v_info->sample_rate : SOUND_TRIGGER_SAMPLING_RATE_16000;
    local_event->audio_config.format = AUDIO_FORMAT_PCM_16_BIT;
    local_event->audio_config.channel_mask =
        audio_channel_in_mask_from_count(st_hw_ses->config.channels);

    local_event->data_offset = sizeof(*local_event);
    local_event->data_size = payload_size;
    memcpy((char *)local_event + local_event->data_offset,
        payload, payload_size);

    ALOGI("%s:[%d]", __func__, st_ses->sm_handle);
    ALOGV("%s:[%d] status=%d, type=%d, model=%d, capture_avaiable=%d",
        __func__, st_ses->sm_handle, local_event->status,
        local_event->type, local_event->model,
        local_event->capture_available);

    *event = local_event;

exit:
    return status;
}

static inline int process_detection_event(st_session_t *st_ses,
    uint64_t timestamp __unused,
    int detect_status,
    void *payload, size_t payload_size,
    struct sound_trigger_recognition_event **event)
{
    int ret;
    struct sound_trigger_phrase_recognition_event *phrase_event = NULL;

    *event = NULL;
    if (st_ses->sm_type == SOUND_MODEL_TYPE_KEYPHRASE) {
        if (sthw_extn_check_process_det_ev_support())
            ret = sthw_extn_process_detection_event_keyphrase(st_ses,
                                      timestamp, detect_status,
                                      payload, payload_size, &phrase_event);
        else if (st_ses->hw_ses_current->is_generic_event &&
                 !st_ses->vendor_uuid_info->is_qcmd_uuid)
            ret = process_detection_event_keyphrase_v2(st_ses, detect_status,
                                                    payload, payload_size,
                                                    &phrase_event);
        else
            ret = process_detection_event_keyphrase(st_ses, detect_status,
                                                    payload, payload_size,
                                                    &phrase_event);
        if (phrase_event)
            *event = &phrase_event->common;
    } else {
        ret = process_detection_event_generic(st_ses, detect_status, payload,
                                            payload_size, event);
    }
    return ret;
}

int st_session_load_sm(st_session_t *st_ses)
{
    int status = 0;
    if (!st_ses)
        return -EINVAL;

    /* SM is stored in the session by st_device, hence set NULL below */
    st_session_loadsm_payload_t payload = { NULL };
    st_session_ev_t ev = { .ev_id = ST_SES_EV_LOAD_SM,
        .payload.loadsm = payload };

    /*
     * no need to lock mutex when loading sm as session is just being
     * being created and handle not returned to caller yet
     */
    DISPATCH_EVENT(st_ses, ev, status);
    return status;
}

int st_session_unload_sm(st_session_t *st_ses)
{
    int status = 0;
    if (!st_ses)
        return -EINVAL;
    st_session_ev_t ev = { .ev_id = ST_SES_EV_UNLOAD_SM };

    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);

    return status;
}

int st_session_start(st_session_t *st_ses)
{
    int status = 0;
    if (!st_ses)
        return -EINVAL;

    st_session_ev_t ev = { .ev_id = ST_SES_EV_START };

    /* lock to serialize event handling */
    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_stop(st_session_t *st_ses)
{
    int status = 0;
    if (!st_ses)
        return -EINVAL;
    st_session_ev_t ev = { .ev_id = ST_SES_EV_STOP };

    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);

    return status;
}

int st_session_restart(st_session_t *st_ses)
{
    int status = 0;
    if (!st_ses)
        return -EINVAL;

    st_session_ev_t ev = { .ev_id = ST_SES_EV_RESTART };

    /* lock to serialize event handling */
    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_ssr_offline(st_session_t *st_ses,
    enum ssr_event_status ssr_type)
{
    int status = 0;

    if (!st_ses)
        return -EINVAL;

    st_session_ev_t ev = { .ev_id = ST_SES_EV_SSR_OFFLINE,
        .payload.ssr = ssr_type };

    pthread_mutex_lock(&st_ses->lock);
    /*
     *  In typical usecases, handle SSR only if it occured on the core we are
     *  currently using. In cases that have an SSR event during transitions,
     *  the exec_mode can be NONE. For these cases, handle SSR on the core
     *  which was in use prior to the transition. For example, if the
     *  ssr_transit_exec_mode is ADSP, then the core prior to the transition
     *  is CPE, so we handle the CPE SSR event.
     *
     *  On 8909w BG uses CPE mode for detection. So add BG specific
     *  conditon check to handle SSR event.
     */
    if (((ST_EXEC_MODE_CPE == st_ses->exec_mode) &&
         (CPE_STATUS_OFFLINE == ssr_type)) ||
        ((ST_EXEC_MODE_ADSP == st_ses->exec_mode) &&
         (SND_CARD_STATUS_OFFLINE == ssr_type)) ||
        ((ST_EXEC_MODE_NONE == st_ses->exec_mode) &&
         (((ST_EXEC_MODE_CPE == st_ses->ssr_transit_exec_mode) &&
           (SND_CARD_STATUS_OFFLINE == ssr_type)) ||
          ((ST_EXEC_MODE_ADSP == st_ses->ssr_transit_exec_mode) &&
           (CPE_STATUS_OFFLINE == ssr_type)))) ||
        ((ST_EXEC_MODE_CPE == st_ses->exec_mode) &&
         (SND_CARD_STATUS_OFFLINE == ssr_type) &&
         (st_ses->stdev->bg_kwd)))
        DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);

    return status;
}

int st_session_ssr_online(st_session_t *st_ses,
    enum ssr_event_status ssr_type)
{
    int status = 0;
    if (!st_ses)
        return -EINVAL;

    st_session_ev_t ev = { .ev_id = ST_SES_EV_SSR_ONLINE,
        .payload.ssr = ssr_type };

    pthread_mutex_lock(&st_ses->lock);
    /*
     *  In typical usecases, handle SSR only if it occured on the core we are
     *  currently using. In cases that have an SSR event during transitions,
     *  the exec_mode can be NONE. For these cases, handle SSR on the core
     *  which was in use prior to the transition. For example, if the
     *  ssr_transit_exec_mode is ADSP, then the core prior to the transition
     *  is CPE, so we handle the CPE SSR event.
     *
     *  On 8909w BG uses CPE mode for detection. So add BG specific
     *  conditon check to handle SSR event.
     */
    if (((ST_EXEC_MODE_CPE == st_ses->exec_mode) &&
         (CPE_STATUS_ONLINE == ssr_type)) ||
        ((ST_EXEC_MODE_ADSP == st_ses->exec_mode) &&
         (SND_CARD_STATUS_ONLINE == ssr_type)) ||
        ((ST_EXEC_MODE_NONE == st_ses->exec_mode) &&
         (((ST_EXEC_MODE_CPE == st_ses->ssr_transit_exec_mode) &&
           (SND_CARD_STATUS_ONLINE == ssr_type)) ||
          ((ST_EXEC_MODE_ADSP == st_ses->ssr_transit_exec_mode) &&
           (CPE_STATUS_ONLINE == ssr_type)))) ||
        ((ST_EXEC_MODE_CPE == st_ses->exec_mode) &&
         (SND_CARD_STATUS_ONLINE == ssr_type) &&
         (st_ses->stdev->bg_kwd)))
        DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);

    return status;
}

int st_session_pause(st_session_t *st_ses)
{
    int status = 0;
    if (!st_ses)
        return -EINVAL;
    st_session_ev_t ev = { .ev_id = ST_SES_EV_PAUSE };
    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_resume(st_session_t *st_ses)
{
    int status = 0;
    if (!st_ses)
        return -EINVAL;
    st_session_ev_t ev = { .ev_id = ST_SES_EV_RESUME };

    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_disable_device(st_session_t *st_ses)
{
    int status = 0;

    if (!st_ses)
        return -EINVAL;

    st_session_ev_t ev = { .ev_id = ST_SES_EV_SET_DEVICE,
        .payload.enable = false };

    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_enable_device(st_session_t *st_ses)
{
    int status = 0;

    if (!st_ses)
        return -EINVAL;

    st_session_ev_t ev = { .ev_id = ST_SES_EV_SET_DEVICE,
        .payload.enable = true };

    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

bool st_session_is_detected(st_session_t *st_ses)
{
    bool ret;

    pthread_mutex_lock(&st_ses->lock);
    ret = (st_ses->current_state == detected_state_fn) ? true : false;
    pthread_mutex_unlock(&st_ses->lock);

    return ret;
}

bool st_session_is_active(st_session_t *st_ses)
{
    bool ret;

    pthread_mutex_lock(&st_ses->lock);
    ret = (st_ses->current_state == active_state_fn) ? true : false;
    pthread_mutex_unlock(&st_ses->lock);

    return ret;
}

bool st_session_is_buffering(st_session_t *st_ses)
{
    bool ret;

    pthread_mutex_lock(&st_ses->lock);
    ret = (st_ses->current_state == buffering_state_fn) ? true : false;
    pthread_mutex_unlock(&st_ses->lock);

    return ret;
}

bool st_session_is_ssr_state(st_session_t *st_ses)
{
    bool ret;

    pthread_mutex_lock(&st_ses->lock);
    ret = (st_ses->current_state == ssr_state_fn) ? true : false;
    pthread_mutex_unlock(&st_ses->lock);

    return ret;
}

int st_session_read_pcm(st_session_t *st_ses, uint8_t *buff,
    size_t buff_size, size_t *read_size)
{
    int status = 0;
    if (!st_ses || !buff || buff_size == 0 || read_size == 0)
        return -EINVAL;

    st_session_readpcm_payload_t payload = { .out_buff = buff,
        .out_buff_size = buff_size, .actual_read_size = read_size };

    st_session_ev_t ev = { .ev_id = ST_SES_EV_READ_PCM,
        .payload.readpcm = payload };

    /* Do not lock when handling this event, this event
     can go in parallel with other events */
    DISPATCH_EVENT(st_ses, ev, status);
    return status;
}

int st_session_stop_lab(st_session_t *st_ses)
{
    int status = 0;
    if (!st_ses)
        return -EINVAL;
    st_session_ev_t ev = { .ev_id = ST_SES_EV_END_BUFFERING };

    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_set_exec_mode(st_session_t *st_ses, st_exec_mode_t exec)
{
    int status = 0;
    if (!st_ses)
        return -EINVAL;

    ALOGV("%s: exec mode %d", __func__, exec);

    st_session_ev_t ev = { .ev_id = ST_SES_EV_SET_EXEC_MODE,
        .payload.exec_mode = exec };

    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_send_custom_chmix_coeff(st_session_t *st_ses, char *str)
{
    int status = 0;

    if (!st_ses)
        return -EINVAL;

    st_session_ev_t ev = { .ev_id = ST_SES_EV_SEND_CHMIX_COEFF,
        .payload.chmix_coeff_str = str};

    pthread_mutex_lock(&st_ses->lock);
    if (ST_EXEC_MODE_ADSP == st_ses->exec_mode)
        DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_get_config(st_session_t *st_ses, struct pcm_config *config)
{
    if (!st_ses)
        return -EINVAL;

    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    memcpy(config, &hw_ses->config, sizeof(struct pcm_config));

    return 0;
}

int st_session_get_param_data(st_session_t *st_ses, const char *param,
                              void *payload, size_t payload_size,
                              size_t *param_data_size)
{
    int status = 0;

    if (!st_ses)
        return -EINVAL;

    st_session_getparam_payload_t getparam_payload = { .param = param,
        .payload = payload,
        .payload_size = payload_size,
        .param_data_size = param_data_size};
    st_session_ev_t ev = { .ev_id = ST_SES_EV_GET_PARAM_DATA,
        .payload.getparam = getparam_payload};

    pthread_mutex_lock(&st_ses->lock);
    /* Currently get param data supported for ARM & ADSP mode */
    if ((ST_EXEC_MODE_ARM == st_ses->exec_mode) ||
        (ST_EXEC_MODE_ADSP == st_ses->exec_mode))
        DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);

    return status;
}

/*
 * If the keyword detection session detects before the user verification
 * session, signal to process user verification. If the keyword detection
 * session rejects before the user verification session, signal to stop
 * processing user verification.
 */
static void handle_vop_pending_detection(st_arm_ss_session_t *ss_session,
                                   unsigned int det_status,
                                   unsigned int kw_det_buff_sz)
{
    if (det_status & KEYWORD_DETECTION_SUCCESS) {
        if (kw_det_buff_sz > ss_session->unread_bytes)
            ss_session->buff_sz = kw_det_buff_sz;
        else
            ss_session->buff_sz = ss_session->unread_bytes;

        /*
         * It is possible that VOP started processing by already consuming
         * data from unread_bytes while CNN detects. In this case, it does
         * not need to be signaled.
         */
        if (ss_session->unread_bytes >= ss_session->buff_sz) {
            ALOGD("%s: Processing UV due to KW detection success", __func__);
            pthread_cond_signal(&ss_session->cond);
        }
    } else if (det_status & KEYWORD_DETECTION_REJECT) {
        ss_session->exit_buffering = true;
        ALOGD("%s: Exiting from UV due to KW detection rejection", __func__);
        pthread_cond_signal(&ss_session->cond);
    }
}

/*
 * If the user verification session rejects before the keyword detection
 * session, signal to stop processing keyword detection.
 */
static void handle_cnn_pending_detection(st_arm_ss_session_t *ss_session,
                                   unsigned int det_status)
{
    if (det_status & USER_VERIFICATION_REJECT) {
        ss_session->exit_buffering = true;
        ALOGD("%s: Exiting from KW detection due to UV rejection", __func__);
        pthread_cond_signal(&ss_session->cond);
    }
}

/*
 * This thread handles detection events from the second stage sessions
 * and aggregates them into 1 final decision. It will call the client callback
 * or restart the first stage session based on this decision.
 */
static void *aggregator_thread_loop(void *st_session)
{
    st_session_t *st_ses = (st_session_t *)st_session;
    recognition_callback_t callback = NULL;
    struct listnode *node = NULL, *tmp_node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;
    int status = 0, lock_status = 0;
    unsigned int kw_det_buff_sz = 0, det_status = 0;
    struct timespec tspec = {0};
    struct sound_trigger_recognition_event *event = NULL;
    st_session_ev_t restart_ev = { .ev_id = ST_SES_EV_RESTART };

    ALOGV("%s: Enter", __func__);

    pthread_mutex_lock(&st_ses->ss_detections_lock);
    while (!st_ses->exit_aggregator_loop) {
        det_status = 0;
        lock_status = 0;
        ALOGV("%s: waiting on cond", __func__);
        pthread_cond_wait(&st_ses->ss_detections_cond,
            &st_ses->ss_detections_lock);
        ALOGV("%s: done waiting on cond", __func__);
        if (st_ses->exit_aggregator_loop) {
            pthread_mutex_unlock(&st_ses->ss_detections_lock);
            return NULL;
        }

        list_for_each_safe(node, tmp_node, &st_ses->second_stage_list) {
            st_sec_stage = node_to_item(node, st_arm_second_stage_t,
                list_node);

            pthread_mutex_lock(&st_sec_stage->ss_session->lock);
            det_status |= st_sec_stage->ss_session->det_status;
            if (st_sec_stage->ss_session->det_status ==
                KEYWORD_DETECTION_SUCCESS)
                kw_det_buff_sz = st_sec_stage->ss_session->bytes_processed;
            pthread_mutex_unlock(&st_sec_stage->ss_session->lock);
        }

        list_for_each_safe(node, tmp_node, &st_ses->second_stage_list) {
            st_sec_stage = node_to_item(node, st_arm_second_stage_t,
                list_node);

            pthread_mutex_lock(&st_sec_stage->ss_session->lock);
            if ((st_sec_stage->ss_info->sm_detection_type ==
                    ST_SM_TYPE_USER_VERIFICATION) &&
                (det_status & USER_VERIFICATION_PENDING)) {
                handle_vop_pending_detection(st_sec_stage->ss_session,
                    det_status, kw_det_buff_sz);
            } else if ((st_sec_stage->ss_info->sm_detection_type ==
                    ST_SM_TYPE_KEYWORD_DETECTION) &&
                (det_status & KEYWORD_DETECTION_PENDING)) {
                handle_cnn_pending_detection(st_sec_stage->ss_session,
                    det_status);
            }
            pthread_mutex_unlock(&st_sec_stage->ss_session->lock);
        }

        if (!IS_SS_DETECTION_PENDING(det_status)) {
            pthread_mutex_lock(&st_ses->lock);
            /*
             * If the client stops before 2nd stage finishes processing, or a
             * transition is in progress, the detection event should not be
             * handled.
             */
            if ((st_ses->current_state != buffering_state_fn) ||
                (st_ses->exec_mode == ST_EXEC_MODE_NONE)) {
                ALOGW("%s: First stage is not in a valid state, continuing",
                    __func__);
                pthread_mutex_unlock(&st_ses->lock);
                continue;
            }
            if (IS_SS_DETECTION_SUCCESS(det_status)) {
                clock_gettime(CLOCK_MONOTONIC, &tspec);
                st_ses->hw_ses_current->second_stage_det_event_time =
                    get_current_time_ns();
                ATRACE_ASYNC_END("sthal: detection success",
                    st_ses->sm_handle);
                status = process_detection_event(st_ses,
                    st_ses->det_session_ev->payload.detected.timestamp,
                    st_ses->det_session_ev->payload.detected.detect_status,
                    st_ses->det_session_ev->payload.detected.detect_payload,
                    st_ses->det_session_ev->payload.detected.payload_size,
                    &event);
                if (status || !event) {
                    ALOGE("%s:[%d] process_detection_event failed err %d",
                        __func__, st_ses->sm_handle, status);
                    /* Stop buffering if this is not a successful detection and
                        LAB is triggered in hw automatically */
                    st_ses->hw_ses_current->fptrs->stop_buffering(
                        st_ses->hw_ses_current, st_ses->lab_enabled);

                    pthread_mutex_unlock(&st_ses->lock);
                    if (event) {
                        free(event);
                        event = NULL;
                    }
                    goto exit;
                }
                callback = st_ses->callback;
                ALOGD("%s: Second stage detected successfully"
                    ", calling client callback", __func__);
                st_ses->sent_detection_to_client = true;
                pthread_mutex_unlock(&st_ses->lock);
                ATRACE_BEGIN("sthal: client detection callback");
                callback(event, st_ses->cookie);
                ATRACE_END();

                /*
                 * The client could unload the sound model during the callback,
                 * which would join this thread and wait for this thread exit
                 * as part of st_session_deinit() with st_session_lock held. By
                 * this time, the state is also moved to idle. To avoid
                 * deadlock, upon return from client callback, try acquiring
                 * lock only if not in idle state, else exit right away.
                 */
                do {
                    lock_status = pthread_mutex_trylock(&st_ses->lock);
                } while (lock_status && (st_ses->current_state !=
                         idle_state_fn));

                if (st_ses->current_state == idle_state_fn) {
                    ALOGV("%s:[%d] client unloaded after callback"
                        ", lock status %d", __func__, st_ses->sm_handle,
                        lock_status);
                    if (!lock_status)
                        pthread_mutex_unlock(&st_ses->lock);
                    free(event);
                    event = NULL;
                    goto exit;
                }

                if (!st_ses->capture_requested)
                    st_ses->hw_ses_current->fptrs->stop_buffering(
                        st_ses->hw_ses_current, st_ses->lab_enabled);
                free(event);
                event = NULL;
            } else {
                ATRACE_ASYNC_END("sthal: detection reject",
                    st_ses->sm_handle);
                ALOGD("%s: Second stage did NOT detect, restarting st_session",
                    __func__);
                st_ses->hw_ses_current->fptrs->stop_buffering(
                    st_ses->hw_ses_current, st_ses->lab_enabled);
                DISPATCH_EVENT(st_ses, restart_ev, status);
            }
            pthread_mutex_unlock(&st_ses->lock);
        } else {
            ALOGV("%s: There is a second stage session pending, continuing",
                __func__);
        }
    }
exit:
    pthread_mutex_unlock(&st_ses->ss_detections_lock);
    ALOGV("%s: Exit", __func__);
    return NULL;
}

static void init_det_event_aggregator(st_session_t *st_ses)
{
    int status = 0;
    pthread_condattr_t attr;

    st_ses->exit_aggregator_loop = false;
    pthread_mutex_init(&(st_ses->ss_detections_lock), NULL);
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&(st_ses->ss_detections_cond), &attr);
    pthread_condattr_destroy(&attr);
    status = pthread_create(&st_ses->aggregator_thread, NULL,
        aggregator_thread_loop, st_ses);
    if (status)
        ALOGE("%s: Error creating aggregator thread. status = %d",
            __func__, status);

}

static void destroy_det_event_aggregator(st_session_t *st_ses)
{
    int status = 0;

    st_ses->exit_aggregator_loop = true;
    pthread_mutex_lock(&st_ses->ss_detections_lock);
    pthread_cond_signal(&st_ses->ss_detections_cond);
    pthread_mutex_unlock(&st_ses->ss_detections_lock);
    status = pthread_join(st_ses->aggregator_thread, NULL);
    if (status)
        ALOGE("%s: Error joining aggregator thread. status = %d",
            __func__, status);
    pthread_cond_destroy(&(st_ses->ss_detections_cond));
    pthread_mutex_destroy(&(st_ses->ss_detections_lock));
}

int st_session_ss_init(st_session_t *st_ses)
{
    int status = 0;
    struct listnode *node = NULL, *tmp_node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;

    init_det_event_aggregator(st_ses);
    list_for_each_safe(node, tmp_node, &st_ses->second_stage_list) {
        st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
        status = st_second_stage_module_init(st_sec_stage,
            (void *)st_sec_stage->ss_info->lib_name);
        if (status) {
            ALOGE("%s: initializing second stage session failed %d",
                __func__, status);
            goto ss_cleanup;
        }
    }

    st_ses->det_session_ev = calloc(1, sizeof(st_session_ev_t));
    if (!st_ses->det_session_ev) {
        ALOGE("%s: Failed to allocate st_session_ev_t, exiting", __func__);
        status = -ENOMEM;
        goto ss_cleanup;
    }
    return 0;

ss_cleanup:
    destroy_det_event_aggregator(st_ses);
    list_for_each_safe(node, tmp_node, &st_ses->second_stage_list) {
        st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
        st_second_stage_module_deinit(st_sec_stage);
    }
    return status;
}

int st_session_ss_deinit(st_session_t *st_ses)
{
    struct listnode *node = NULL, *tmp_node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;

    destroy_det_event_aggregator(st_ses);
    list_for_each_safe(node, tmp_node, &st_ses->second_stage_list) {
        st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
        st_second_stage_module_deinit(st_sec_stage);
    }

    if (st_ses->det_session_ev)
        free(st_ses->det_session_ev);

    return 0;
}

int st_session_request_detection(st_session_t *st_ses)
{
    int status = 0;

    if (!st_ses)
        return -EINVAL;

    st_session_ev_t ev = { .ev_id = ST_SES_EV_REQUEST_DET };

    /* lock to serialize event handling */
    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_init(st_session_t *st_ses, struct sound_trigger_device *stdev,
    st_exec_mode_t exec_mode, sound_model_handle_t sm_handle)
{
    int status = 0;
    struct st_vendor_info *v_info;
    pthread_mutexattr_t attr;

    if (!st_ses || !stdev) {
        status = -EINVAL;
        return status;
    }
    st_ses->stdev = stdev;

    /* caller must set vendor_uuid_info directly if present */
    v_info = st_ses->vendor_uuid_info;

    if (v_info && (EXEC_MODE_CFG_DYNAMIC == v_info->exec_mode_cfg)) {
        st_ses->enable_trans = true;

        if (stdev->is_gcs) {
            /* alloc and init cpe session*/
            st_ses->hw_ses_cpe = (st_hw_session_t *)calloc(1, sizeof(st_hw_session_gcs_t));
            if (!st_ses->hw_ses_cpe) {
                status = -ENOMEM;
                goto cleanup;
            }
            status = st_hw_sess_gcs_init(st_ses->hw_ses_cpe, hw_sess_cb,
                (void *)st_ses, ST_EXEC_MODE_CPE, v_info, sm_handle, stdev);
            if (status) {
                ALOGE("%s: initializing gcs hw session failed %d", __func__,
                    status);
                goto cleanup;
            }

            /* alloc and init adsp session*/
            st_ses->hw_ses_adsp = (st_hw_session_t *)calloc(1, sizeof(st_hw_session_lsm_t));
            if (!st_ses->hw_ses_adsp) {
                st_hw_sess_gcs_deinit(st_ses->hw_ses_cpe);
                status = -ENOMEM;
                goto cleanup;
            }

            status = st_hw_sess_lsm_init(st_ses->hw_ses_adsp, hw_sess_cb,
                (void *)st_ses, ST_EXEC_MODE_ADSP, v_info, sm_handle, stdev);
            if (status) {
                ALOGE("%s: initializing lsm session failed", __func__);
                st_hw_sess_gcs_deinit(st_ses->hw_ses_cpe);
                goto cleanup;
            }

        } else {
            /* alloc and init cpe session*/
            st_ses->hw_ses_cpe = (st_hw_session_t *)calloc(1, sizeof(st_hw_session_lsm_t));
            if (!st_ses->hw_ses_cpe) {
                status = -ENOMEM;
                goto cleanup;
            }
            status = st_hw_sess_lsm_init(st_ses->hw_ses_cpe, hw_sess_cb,
                (void *)st_ses, ST_EXEC_MODE_CPE, v_info, sm_handle, stdev);
            if (status) {
                ALOGE("%s: initialzing lsm hw session failed %d", __func__,
                    status);
                goto cleanup;
            }
            /* alloc and init adsp session*/
            st_ses->hw_ses_adsp = (st_hw_session_t *)calloc(1, sizeof(st_hw_session_lsm_t));
            if (!st_ses->hw_ses_adsp) {
                status = -ENOMEM;
                st_hw_sess_lsm_deinit(st_ses->hw_ses_cpe);
                goto cleanup;
            }

            status = st_hw_sess_lsm_init(st_ses->hw_ses_adsp, hw_sess_cb,
                (void *)st_ses, ST_EXEC_MODE_ADSP, v_info, sm_handle, stdev);
            if (status) {
                ALOGE("%s: initializing lsm session failed", __func__);
                st_hw_sess_lsm_deinit(st_ses->hw_ses_cpe);
                goto cleanup;
            }
        }

        /* set current hw_session */
        if (exec_mode == ST_EXEC_MODE_CPE)
            st_ses->hw_ses_current = st_ses->hw_ses_cpe;
        else if (exec_mode == ST_EXEC_MODE_ADSP)
            st_ses->hw_ses_current = st_ses->hw_ses_adsp;

    } else if (v_info && (EXEC_MODE_CFG_CPE == v_info->exec_mode_cfg)) {
        st_ses->enable_trans = false;
        if (stdev->is_gcs) {
            ALOGD("%s: initializing gcs hw session", __func__);
            st_ses->hw_ses_cpe = (st_hw_session_t *)calloc(1, sizeof(st_hw_session_gcs_t));
            if (!st_ses->hw_ses_cpe) {
                status = -ENOMEM;
                goto cleanup;
            }
            status = st_hw_sess_gcs_init(st_ses->hw_ses_cpe, hw_sess_cb,
                (void *)st_ses, exec_mode, v_info, sm_handle, stdev);
            if (status) {
                ALOGE("%s: initializing gcs hw session failed %d",
                    __func__, status);
                goto cleanup;
            }
        } else {
            st_ses->hw_ses_cpe = (st_hw_session_t *)calloc(1, sizeof(st_hw_session_lsm_t));
            if (!st_ses->hw_ses_cpe) {
                status = -ENOMEM;
                goto cleanup;
            }
            status = st_hw_sess_lsm_init(st_ses->hw_ses_cpe, hw_sess_cb,
                (void *)st_ses, exec_mode, v_info, sm_handle, stdev);
            if (status) {
                ALOGE("%s: initializing lsm hw session failed %d",
                    __func__, status);
                goto cleanup;
            }
        }
        st_ses->hw_ses_current = st_ses->hw_ses_cpe;

    } else if (v_info && (EXEC_MODE_CFG_APE == v_info->exec_mode_cfg)) {
        st_ses->enable_trans = false;
        st_ses->hw_ses_adsp = (st_hw_session_t *)calloc(1, sizeof(st_hw_session_lsm_t));
        if (!st_ses->hw_ses_adsp) {
            status = -ENOMEM;
            goto cleanup;
        }
        status = st_hw_sess_lsm_init(st_ses->hw_ses_adsp, hw_sess_cb,
            (void *)st_ses, exec_mode, v_info, sm_handle, stdev);
        if (status) {
            ALOGE("%s: initializing lsm hw session failed %d",
                __func__, status);
            goto cleanup;
        }
        st_ses->hw_ses_current = st_ses->hw_ses_adsp;
    } else if (v_info && (EXEC_MODE_CFG_ARM == v_info->exec_mode_cfg)) {
        st_ses->enable_trans = false;
        st_ses->hw_ses_arm = calloc(1, sizeof(st_hw_session_pcm_t));
        if (!st_ses->hw_ses_arm) {
            status = -ENOMEM;
            goto cleanup;
        }
        status = st_hw_sess_pcm_init(st_ses->hw_ses_arm, hw_sess_cb,
            (void *)st_ses, exec_mode, v_info, sm_handle, stdev);
        if (status) {
            ALOGE("%s: initializing pcm hw session failed %d",
                __func__, status);
            goto cleanup;
        }
        st_ses->hw_ses_current = st_ses->hw_ses_arm;
    } else if (!v_info) {
        st_ses->hw_ses_cpe = (st_hw_session_t *)calloc(1, sizeof(st_hw_session_lsm_t));
        if (!st_ses->hw_ses_cpe) {
            status = -ENOMEM;
            goto cleanup;
        }
        status = st_hw_sess_lsm_init(st_ses->hw_ses_cpe, hw_sess_cb,
            (void *)st_ses, exec_mode, v_info, sm_handle, stdev);
        if (status) {
            ALOGE("%s: initializing lsm hw session failed %d",
                __func__, status);
            goto cleanup;
        }
    }

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&st_ses->lock, (const pthread_mutexattr_t *)&attr);

    st_ses->exec_mode = exec_mode;
    st_ses->sm_handle = sm_handle;
    st_ses->ssr_transit_exec_mode = ST_EXEC_MODE_NONE;
    st_ses->lab_fp = NULL;

    /* start in idle state */
    STATE_TRANSITION(st_ses, idle_state_fn);

    return status;

cleanup:
    if (st_ses->hw_ses_cpe)
        free(st_ses->hw_ses_cpe);
    if (st_ses->hw_ses_adsp)
        free(st_ses->hw_ses_adsp);
    return status;
}

int st_session_deinit(st_session_t *st_ses)
{
    /* deinit cpe session */
    if (st_ses->hw_ses_cpe) {
        if (st_ses->stdev->is_gcs)
            st_hw_sess_gcs_deinit(st_ses->hw_ses_cpe);
        else
            st_hw_sess_lsm_deinit(st_ses->hw_ses_cpe);
        free((void *)st_ses->hw_ses_cpe);
        st_ses->hw_ses_cpe = NULL;
    }

    /* deinit adsp session */
    if (st_ses->hw_ses_adsp) {
        st_hw_sess_lsm_deinit(st_ses->hw_ses_adsp);
        free((void *)st_ses->hw_ses_adsp);
        st_ses->hw_ses_adsp = NULL;
    }

    pthread_mutex_destroy(&st_ses->lock);

    return 0;
}
