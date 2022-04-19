/* st_hw_common.c
 *
 * This file contains common functionality between
 * sound trigger hw and sound trigger extension interface.
 *
 * Copyright (c) 2016, 2018-2019, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "sound_trigger_hw_common"
/* #define LOG_NDEBUG 0 */
#define LOG_NDDEBUG 0

/* #define VERY_VERBOSE_LOGGING */
#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

#include <cutils/log.h>
#include <hardware/sound_trigger.h>

#include "st_hw_common.h"

/*
 * Maximum number of sessions supported in LPI mode:
 * here each stages are counted individually,
 * so session count for dual stage lsm session would be set to 2.
 */
#define LSM_MAX_LPI_BUDGET_AVAILABLE 2
#define LSM_MAX_SS_USECASE_SUPPORTED 1

#ifdef AUDIO_FEATURE_ENABLED_GCOV
extern void  __gcov_flush();
static void enable_gcov()
{
    __gcov_flush();
}
#else
static void enable_gcov()
{
}
#endif

bool st_hw_check_ses_ss_usecase_allowed(st_session_t *st_ses)
{
    struct sound_trigger_device *stdev;
    struct listnode *node;
    st_session_t *p_ses;
    int lsm_ss_uc_count = 0;
    /* TODO: get max_lsm_ss_uc_count from platform config xml */
    int max_lsm_ss_uc_count = LSM_MAX_SS_USECASE_SUPPORTED;

    /*
     * Second stage is only supported on an adsp session,
     * and when multi-stage support is available in lsm drivers.
     */
    if (!st_ses || !st_ses->hw_proxy_ses ||
        !st_ses->hw_proxy_ses->hw_ses_adsp ||
        !st_hw_check_multi_stage_lsm_support()) {
        return false;
    }

    stdev = st_ses->stdev;
    list_for_each(node, &stdev->sound_model_list) {
        p_ses = node_to_item(node, st_session_t, list_node);
        if (p_ses == NULL || p_ses == st_ses ||
            p_ses->exec_mode != ST_EXEC_MODE_ADSP)
            continue;
        pthread_mutex_lock(&p_ses->hw_proxy_ses->lock);
        if (p_ses->hw_proxy_ses->hw_ses_adsp &&
            !list_empty(&p_ses->hw_proxy_ses->hw_ses_adsp->lsm_ss_cfg_list))
            lsm_ss_uc_count++;
        if (lsm_ss_uc_count >= max_lsm_ss_uc_count) {
            ALOGD("%s: max supported ss usecase count(%d) already active,"
                  "not allowing further", __func__, max_lsm_ss_uc_count);
            pthread_mutex_unlock(&p_ses->hw_proxy_ses->lock);
            return false;
        }
        pthread_mutex_unlock(&p_ses->hw_proxy_ses->lock);
    }
    ALOGD("%s: ss usecase allowed", __func__);
    return true;
}

static int get_session_lpi_cost(st_session_t *ses)
{
    /* TODO: check if this works for all sound models, and update logic if required */
    return ses->num_phrases > 1 ? ses->num_phrases : 1;
}

/*
 * Check below cases to do LPI budgeting:
 * 1. when asked for a session:
 *   - check if the session can be allowed with lpi mode
 * 2. when session is NULL:
 *   - check if sessions in global list meets lpi constraint
 * 3. for NULL case without any other active session:
 *   - return false as lpi mode is false by default
 */
static bool is_projected_lpi_budget_available
(
    struct sound_trigger_device *stdev,
    st_session_t *st_ses
)
{
    st_session_t *p_ses;
    struct listnode *node;
    int ses_lpi_cost = 0, sys_lpi_cost = 0;
    /* TODO: max lpi budget from platform config xml */
    int max_lpi_budget_available = LSM_MAX_LPI_BUDGET_AVAILABLE;

    /*
     * LPI mode is only allowed in following ADSP use cases:
     * 1. Single session with single keyword and 2 stages, both in ADSP
     * 2. Single session with two keywords but single stages in ADSP
     * 2. Upto 2 sessions with single keyword each
     */

    /* first check current session for LPI criteria if st_ses is available */
    ses_lpi_cost = st_ses ? get_session_lpi_cost(st_ses) : 0;
    if (ses_lpi_cost > max_lpi_budget_available) {
        ALOGD("%s: ses lpi cost(%d) exceeds budget(%d), enable=false",
              __func__, ses_lpi_cost, max_lpi_budget_available);
        return false;
    }
    /* check for presence of any other session with remaining LPI budget*/
    sys_lpi_cost += ses_lpi_cost;
    list_for_each(node, &stdev->sound_model_list) {
        p_ses = node_to_item(node, st_session_t, list_node);
        if (p_ses == NULL || p_ses == st_ses || p_ses->exec_mode != ST_EXEC_MODE_ADSP)
            continue;
        /*
         * Check for all sessions present in sound model list as
         * LPI is load time decision.
         */
        if (p_ses->vendor_uuid_info->lpi_enable) {
            sys_lpi_cost += get_session_lpi_cost(p_ses);
            if (sys_lpi_cost > max_lpi_budget_available) {
                ALOGD("%s: projected lpi cost exceeds budget(%d), enable=false",
                      __func__, max_lpi_budget_available);
                return false;
            }
        }
    }

    /* system lpi cost 0 at this point means no valid lpi session, return false */
    if (sys_lpi_cost == 0) {
        ALOGV("%s: no session with lpi support, enable=false", __func__);
        return false;
    }

    ALOGD("%s: projected lpi cost(%d) within budget(%d), enable=true",
          __func__, sys_lpi_cost, max_lpi_budget_available);
    return true;
}

void st_hw_check_and_update_lpi
(
    struct sound_trigger_device *stdev,
    st_session_t *st_ses
)
{
    st_session_t *ses = NULL;
    struct listnode *ses_node = NULL;

    stdev->lpi_enable = false;
    stdev->barge_in_mode = true;
    /*
     * ST_PLATFORM_LPI_NONE is used for backward compatibility. With this
     * setting, the st_vendor_uuid->lpi_enable flag will be used.
     */
    if (stdev->platform_lpi_enable == ST_PLATFORM_LPI_DISABLE) {
        ALOGD("%s: lpi NOT enabled in platform setting", __func__);
        return;
    } else if ((stdev->platform_lpi_enable == ST_PLATFORM_LPI_NONE) &&
               st_ses && !st_ses->vendor_uuid_info->lpi_enable) {
        ALOGD("%s: lpi NOT enabled for ses %d", __func__, st_ses->sm_handle);
        return;
    }

    if (stdev->rx_concurrency_active || stdev->conc_voice_active ||
        stdev->conc_voip_active) {
        ALOGD("%s: lpi NOT supported due to concurrency", __func__);
        return;
    }

    if (stdev->is_charging &&
        stdev->transit_to_non_lpi_on_battery_charging) {
        ALOGD("%s: lpi NOT supported. battery status %d", __func__,
            stdev->is_charging);
        if (stdev->support_barge_in_mode)
            stdev->barge_in_mode = false;
        return;
    }

    list_for_each(ses_node, &stdev->sound_model_list) {
        ses = node_to_item(ses_node, st_session_t, list_node);

        if (ses->client_req_det_mode == ST_DET_HIGH_PERF_MODE) {
            ALOGD("%s:[%d] lpi NOT supported due to high perf mode", __func__,
                ses->sm_handle);
            if (stdev->support_barge_in_mode)
                stdev->barge_in_mode = false;
            return;
        }
    }

    if (!stdev->screen_off && stdev->support_barge_in_mode) {
        ALOGD("%s: lpi NOT supported. Screen is on", __func__);
        stdev->barge_in_mode = false;
        return;
    }

    if (stdev->platform_lpi_enable == ST_PLATFORM_LPI_NONE)
        stdev->lpi_enable = is_projected_lpi_budget_available(stdev, st_ses);
    else
        stdev->lpi_enable = true;
    stdev->barge_in_mode = !stdev->lpi_enable;
}

bool st_hw_check_vad_support
(
    struct sound_trigger_device *stdev,
    st_session_t *st_ses,
    bool lpi_enable
)
{
    st_session_t *p_ses;
    struct listnode *node;
    bool vad_enable = false;
    st_profile_type_t profile_type;


    if (stdev->rx_concurrency_active || stdev->tx_concurrency_active) {
        ALOGD("%s: VAD NOT supported due to concurrency", __func__);
        return false;
    }

    /*
     * check for all sessions, unless vad check is requested for a particular session.
     * For a session, return true if vad is enabled in platform config and
     * profile type is NONE or lpi is disabled.
     */
    if (st_ses) {
        profile_type = st_ses->vendor_uuid_info->profile_type;
        vad_enable = st_ses->vendor_uuid_info->vad_enable &&
                     ((profile_type == ST_PROFILE_TYPE_NONE) || lpi_enable);
    } else {
        list_for_each(node, &stdev->sound_model_list) {
            p_ses = node_to_item(node, st_session_t, list_node);
            profile_type = p_ses->vendor_uuid_info->profile_type;
            if (p_ses->exec_mode != ST_EXEC_MODE_ADSP) {
                continue;
            } else if (!p_ses->vendor_uuid_info->vad_enable ||
                      ((profile_type != ST_PROFILE_TYPE_NONE) && !lpi_enable)) {
                ALOGD("%s: disable vad, session does not meet requirement", __func__);
                vad_enable = false;
                break;
            } else {
                    vad_enable = true;
            }
        }
    }

    ALOGV("%s: vad_enable = %d", __func__, vad_enable);
    return vad_enable;
}

void st_hw_check_and_set_lpi_mode(st_session_t *stc_ses)
{
    st_proxy_session_t *st_ses = NULL;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return;

    st_ses = stc_ses->hw_proxy_ses;

    pthread_mutex_lock(&st_ses->lock);
    if (st_ses->hw_ses_adsp) {
        if (st_ses->stdev->platform_lpi_enable == ST_PLATFORM_LPI_NONE) {
            st_ses->hw_ses_adsp->lpi_enable =
                (st_ses->vendor_uuid_info->lpi_enable &&
                is_projected_lpi_budget_available(st_ses->stdev, stc_ses));
        } else {
            st_ses->hw_ses_adsp->lpi_enable = st_ses->stdev->lpi_enable;
            st_ses->hw_ses_adsp->barge_in_mode =
                st_ses->stdev->barge_in_mode;
        }
    }
    pthread_mutex_unlock(&st_ses->lock);
}

int stop_other_sessions(struct sound_trigger_device *stdev,
                         st_session_t *cur_ses)
{
    st_session_t *p_ses;
    struct listnode *p_ses_node;
    int status = 0;

    ALOGV("%s: list empty %s", __func__,
          list_empty(&stdev->sound_model_list) ? "true" : "false");

    if (!stdev->session_allowed)
        return 0;

    list_for_each(p_ses_node, &stdev->sound_model_list) {
        p_ses = node_to_item(p_ses_node, st_session_t, list_node);

        /* Current session can already be in the list during SSR */
        if (p_ses == cur_ses)
            continue;

        status = st_session_pause(p_ses);
        if (status) {
            ALOGE("%s: error stopping session", __func__);
            return -EIO;
        }
    }
    return 0;
}

int start_other_sessions(struct sound_trigger_device *stdev,
                         st_session_t *cur_ses)
{
    st_session_t *p_ses;
    struct listnode *p_ses_node;
    int status = 0;

    ALOGV("%s: list empty %s", __func__,
       list_empty(&stdev->sound_model_list) ? "true" : "false");

    if (!stdev->session_allowed)
        return 0;

    list_for_each(p_ses_node, &stdev->sound_model_list) {
        p_ses = node_to_item(p_ses_node, st_session_t, list_node);

        /* Current session can already be in the list during SSR */
        if (p_ses == cur_ses)
            continue;
        status = st_session_resume(p_ses);
        if (status) {
            ALOGE("%s: error restarting session", __func__);
            return -EIO;
        }
    }
    return 0;
}

st_session_t* get_sound_trigger_session(
                    struct sound_trigger_device *stdev,
                    sound_model_handle_t sound_model_handle)

{
    st_session_t *st_session = NULL;
    struct listnode *node;

    list_for_each(node, &stdev->sound_model_list) {
        st_session = node_to_item(node, st_session_t, list_node);
        if (st_session->sm_handle == sound_model_handle)
            return st_session;

    }
    return NULL;
}

/*
 * This function is used to prepare the detection engine custom config payload for
 * sound trigger sessions that have second stage sessions. If history buffering is not
 * requested by the client, add it into the payload here. Second stage sessions require
 * the keyword to be buffered.
 */
int st_hw_ses_get_hist_buff_payload
(
    st_hw_session_t *p_ses,
    uint8_t *payload_buf,
    size_t buff_size
)
{
    struct st_hist_buffer_info *hist_buf = NULL;

    if (!payload_buf || buff_size < sizeof(*hist_buf)) {
        ALOGE("%s: buffer size(%zd) too small to fill payload(%zd)",
              __func__, buff_size, sizeof(*hist_buf));
        return -EINVAL;
    }

    hist_buf = (struct st_hist_buffer_info *) payload_buf;
    hist_buf->version = DEFAULT_CUSTOM_CONFIG_MINOR_VERSION;

    if (p_ses->sthw_cfg.client_req_hist_buf > 0) {
        hist_buf->hist_buffer_duration_msec =
            p_ses->sthw_cfg.client_req_hist_buf;
        hist_buf->pre_roll_duration_msec = p_ses->sthw_cfg.client_req_preroll;

        if (p_ses->is_generic_event) {
            if (p_ses->sthw_cfg.client_req_hist_buf <=
                (p_ses->sthw_cfg.client_req_preroll + KW_LEN_WARNING))
                ALOGW("%s: Client hist buf and preroll lens leave only"
                      "%dms for keyword", __func__,
                      (p_ses->sthw_cfg.client_req_hist_buf -
                       p_ses->sthw_cfg.client_req_preroll));

            if (p_ses->sthw_cfg.client_req_preroll < PREROLL_LEN_WARNING)
                ALOGW("%s: Client requested small preroll length %dms",
                    __func__, p_ses->sthw_cfg.client_req_preroll);
        }
    } else {
        hist_buf->hist_buffer_duration_msec =
            p_ses->vendor_uuid_info->kw_duration;
        hist_buf->pre_roll_duration_msec = 0;
    }
    ALOGD("%s: history buf duration %d, preroll %d", __func__,
          hist_buf->hist_buffer_duration_msec,
          hist_buf->pre_roll_duration_msec);
    return 0;
}

/* ---------------- hw session notify thread --------------- */
#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

typedef struct {
    sound_model_handle_t handle;
    st_session_event_id_t event;
    struct listnode node; /* membership in queue or pool */
    uint64_t when;
} request_t;

static struct {
    pthread_t thread;
    bool done;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    struct listnode queue;
    struct listnode pool;
    request_t requests[32];
    hw_session_notify_callback_t callback;
    bool inited;
} hw_session_notifier;

static inline int hw_session_notifier_lock()
{
    return pthread_mutex_lock(&hw_session_notifier.lock);
}

static inline int hw_session_notifier_unlock()
{
    return pthread_mutex_unlock(&hw_session_notifier.lock);
}

static inline int hw_session_notifier_wait_l()
{
    int ret = -EINVAL;

    if (list_empty(&hw_session_notifier.queue)) {
        pthread_cond_wait(&hw_session_notifier.cond,
                          &hw_session_notifier.lock);
        return EINTR; /* wait can only be interrupted */
    }

    struct listnode *node = list_head(&hw_session_notifier.queue);
    request_t *r = node_to_item(node, request_t, node);
    int64_t interval = r->when - get_current_time_ns();

    if (interval <= 0) {
        ALOGV("early exit\n");
        return 0;
    }

    struct timespec timeout;
    GET_WAIT_TIMESPEC(timeout, interval);
    ret = pthread_cond_timedwait(&hw_session_notifier.cond,
                                 &hw_session_notifier.lock, &timeout);
    switch (ret) {
        case 0:
            ret = EINTR; /* wait was interrupted */
            break;
        case ETIMEDOUT:
            ret = 0; /* time passed expired, proceed with removing first entry from list */
            break;
        default:
            ALOGE("%s: wait failed w/ ret %s\n", __func__, strerror(ret));
            break;
    }
    return ret;
}

static inline int hw_session_notifier_signal()
{
    return pthread_cond_signal(&hw_session_notifier.cond);
}

int hw_session_notifier_enqueue(sound_model_handle_t handle,
                                st_session_event_id_t event,
                                uint64_t delay_ms)
{
    int ret = 0;

    if (!hw_session_notifier.inited)
        return -ENODEV;

    hw_session_notifier_lock();
    if (list_empty(&hw_session_notifier.pool)) {
        ALOGE("%s: No space to queue request, try again", __func__);
        ret = -EAGAIN;
        goto exit;
    }

    struct listnode *req_node = list_head(&hw_session_notifier.pool);
    list_remove(req_node);
    request_t *r = node_to_item(req_node, request_t, node);
    r->handle = handle;
    r->event = event;
    r->when = get_current_time_ns() + (delay_ms * NSECS_PER_MSEC);

    struct listnode *node = list_head(&hw_session_notifier.queue);
    while (node != list_tail(&hw_session_notifier.queue)) {
        request_t *er = node_to_item(node, request_t, node);
        if (r->when >= er->when) {
            /* continue to next node */
            node = list_head(node);
        } else {
            /* insert the element before this node */
            break;
        }
    }
    /*
     * queue element before element "node". This is implicitly same
     * as list->push_back if the element has to be pushed to the
     * end of the list as in that case, node == head
     */
    list_add_tail(node, req_node);
    hw_session_notifier_signal();
exit:
    hw_session_notifier_unlock();
    return ret;
}

int hw_session_notifier_cancel(sound_model_handle_t handle,
                               st_session_event_id_t event) {
    int ret = -1;
    struct listnode *node, *tmp_node;

    if (!hw_session_notifier.inited)
        return -1;

    hw_session_notifier_lock();
    list_for_each_safe(node, tmp_node, &hw_session_notifier.queue) {
        request_t *r = node_to_item(node, request_t, node);
        if (r->handle == handle && r->event == event) {
            ALOGV("%s: found req with handle %d and ev %d to cancel",
                  __func__, handle, event);
            r->handle = -1;
            list_remove(node);
            list_add_tail(&hw_session_notifier.pool, node);
            ret = 0;
        }
    }
    hw_session_notifier_signal();
    hw_session_notifier_unlock();
    return ret;
}

static void hw_session_notify_process_once_l() {
    struct listnode *req_node = list_head(&hw_session_notifier.queue);
    request_t *r = node_to_item(req_node, request_t, node);

    list_remove(req_node);
    hw_session_notifier_unlock();
    switch (r->event) {
        case ST_SES_EV_DEFERRED_STOP:
            ALOGI("%s:[%d] hw notify deferred stop", __func__, r->handle);
            hw_session_notifier.callback(r->handle, ST_SES_EV_DEFERRED_STOP);
            break;
        default:
            break;
    }
    hw_session_notifier_lock();
    list_add_tail(&hw_session_notifier.pool, req_node);
}

void *hw_session_notifier_loop(void *arg __unused) {
    hw_session_notifier_lock();
    while (!hw_session_notifier.done) {
        int ret = hw_session_notifier_wait_l();
        switch (ret) {
            case 0: /* timer expired */
                hw_session_notify_process_once_l();
                break;
            case EINTR: /* timer interrupted due to next enqueue or some other event */
                continue;
            default:
                ALOGE("%s: wait_l returned err %d, exit loop!", __func__, ret);
                hw_session_notifier.done =  true;
                break;
        }
    }
    hw_session_notifier_unlock();
    ALOGI("%s thread loop exiting", __func__);
    return NULL;
}

int hw_session_notifier_init(hw_session_notify_callback_t cb) {
    pthread_attr_t attr;
    pthread_condattr_t c_attr;

    if (hw_session_notifier.inited)
        return -EINVAL;

    hw_session_notifier.inited = false;
    hw_session_notifier.done = false;

    pthread_mutex_init(&hw_session_notifier.lock, NULL);
    pthread_condattr_init(&c_attr);
    pthread_condattr_setclock(&c_attr, CLOCK_MONOTONIC);
    pthread_cond_init(&hw_session_notifier.cond, &c_attr);
    pthread_condattr_destroy(&c_attr);

    list_init(&hw_session_notifier.queue);
    list_init(&hw_session_notifier.pool);
    for (uint32_t i = 0; i < ARRAY_SIZE(hw_session_notifier.requests); i++) {
        list_init(&hw_session_notifier.requests[i].node);
        hw_session_notifier.requests[i].handle = -1;
        list_add_tail(&hw_session_notifier.pool,
                      &hw_session_notifier.requests[i].node);
    }
    pthread_attr_init(&attr);
    if (pthread_create(&hw_session_notifier.thread, &attr,
                       hw_session_notifier_loop, NULL)) {
        ALOGE("%s: Failed to create hw_notify thread w/ err %s",
              __func__,
              strerror(errno));
        return -1;
    }
    hw_session_notifier.callback = cb;
    hw_session_notifier.inited = true;
    ALOGV("%s: completed", __func__);
    enable_gcov();
    return 0;
}

void hw_session_notifier_deinit()
{
    if (hw_session_notifier.inited) {
        hw_session_notifier_lock();
        hw_session_notifier.done = true;
        hw_session_notifier_signal();
        hw_session_notifier_unlock();
        pthread_join(hw_session_notifier.thread, NULL);
        hw_session_notifier.inited = false;
        pthread_mutex_destroy(&hw_session_notifier.lock);
        pthread_cond_destroy(&(hw_session_notifier.cond));
        ALOGV("%s: completed", __func__);
    }
    enable_gcov();
}
