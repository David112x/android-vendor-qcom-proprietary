/* st_session.c
 *
 * This file contains the state machine for a single sound trigger
 * user session. This state machine implements logic for handling all user
 * interactions, detectinos, SSR and Audio Concurencies.
 *
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
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
#define ST_SES_DEFERRED_STOP_SS_DELAY_MS 0
#else
#define ST_SES_DEFERRED_STOP_DELAY_MS 1000
#define ST_SES_DEFERRED_STOP_SS_DELAY_MS 250
#endif

#define IS_SS_DETECTION_PENDING(det)\
    (det & (KEYWORD_DETECTION_PENDING | USER_VERIFICATION_PENDING))
#define IS_SS_DETECTION_SUCCESS(det)\
    !(det & (KEYWORD_DETECTION_REJECT | USER_VERIFICATION_REJECT))

#define IS_KEYWORD_DETECTION_MODEL(sm_id) (sm_id & ST_SM_ID_SVA_KWD)

#define IS_USER_VERIFICATION_MODEL(sm_id) (sm_id & ST_SM_ID_SVA_VOP)

#define IS_SECOND_STAGE_MODEL(sm_id)\
    ((sm_id & ST_SM_ID_SVA_KWD) || (sm_id & ST_SM_ID_SVA_VOP))

#define IS_MATCHING_SS_MODEL(usecase_sm_id, levels_sm_id)\
    ((usecase_sm_id & levels_sm_id) ||\
    ((usecase_sm_id & ST_SM_ID_SVA_RNN) && (levels_sm_id & ST_SM_ID_SVA_CNN)))

#define STATE_TRANSITION(st_session, new_state_fn)\
do {\
    if (st_session->current_state != new_state_fn) {\
        st_session->current_state = new_state_fn;\
        ALOGD("session[%d]: %s ---> %s", st_session->sm_handle, __func__, \
            #new_state_fn);\
    }\
} while(0)

#define DISPATCH_EVENT(st_session, event, status)\
do {\
    status = st_session->current_state(st_session, &event);\
} while (0)

#define REG_SM_RETRY_CNT 5
#define REG_SM_WAIT_TIME_MS 100

#define MAX_CONF_LEVEL_VALUE (100)
#define MAX_KW_USERS_NAME_LEN (2 * MAX_STRING_LEN)

/* below enum used in cleanup in error scenarios */
enum hw_session_err_mask {
    HW_SES_ERR_MASK_DEVICE_SET = 0x1,
    HW_SES_ERR_MASK_REG_SM = 0x2,
    HW_SES_ERR_MASK_REG_SM_PARAM = 0x4,
    HW_SES_ERR_MASK_STARTED = 0x8,
    HW_SES_ERR_MASK_BUFFERING = 0x10,
};

typedef struct st_session_loadsm_payload {
    struct sound_trigger_phrase_sound_model *phrase_sm;
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
    st_session_t *stc_ses;
};

static int idle_state_fn(st_proxy_session_t *st_ses, st_session_ev_t *ev);
static int loaded_state_fn(st_proxy_session_t *st_ses, st_session_ev_t *ev);
static int active_state_fn(st_proxy_session_t *st_ses, st_session_ev_t *ev);
static int detected_state_fn(st_proxy_session_t *st_ses, st_session_ev_t *ev);
static int buffering_state_fn(st_proxy_session_t *st_ses, st_session_ev_t *ev);
static int ssr_state_fn(st_proxy_session_t *st_ses, st_session_ev_t *ev);

static inline int process_detection_event
(
    st_proxy_session_t *st_ses, uint64_t timestamp, int detect_status,
    void *payload, size_t payload_size,
    struct sound_trigger_recognition_event **event
);

ST_DBG_DECLARE(static int file_cnt = 0);

void hw_sess_cb(st_hw_sess_event_t *hw_event, void *cookie)
{
    st_proxy_session_t *st_ses = (st_proxy_session_t *)cookie;
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
            /*
             * TODO: Add RECOGNITION_STATUS_GET_STATE_RESPONSE to
             * the SoundTrigger API header.
             */
            if (st_ses->detection_requested)
                ev.payload.detected.detect_status = 3;

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

static inline void free_array_ptrs(char **arr, unsigned int arr_len)
{
    int i = 0;

    if (!arr)
        return;

    for (i = 0; i < arr_len; i++) {
        if (arr[i]) {
            free(arr[i]);
            arr[i] = NULL;
        }
    }
    free(arr);
    arr = NULL;
}

static inline void alloc_array_ptrs(char ***arr, unsigned int arr_len,
    unsigned int elem_len)
{
    char **str_arr = NULL;
    int i = 0;

    str_arr = (char **) calloc(arr_len, sizeof(char *));

    if (!str_arr) {
        *arr = NULL;
        return;
    }

    for (i = 0; i < arr_len; i++) {
         str_arr[i] = (char *) calloc(elem_len, sizeof(char));
         if (str_arr[i] == NULL) {
             free_array_ptrs(str_arr, i);
             *arr = NULL;
             return;
         }
    }
    *arr = str_arr;
    ALOGV("%s: string array %p", __func__, *arr);
    for (i = 0; i < arr_len; i++)
        ALOGV("%s: string array[%d] %p", __func__, i, (*arr)[i]);
}

static int merge_sound_models(struct sound_trigger_device *stdev,
    unsigned int num_models, listen_model_type *in_models[],
    listen_model_type *out_model)
{
    listen_status_enum sm_ret = 0;
    int status = 0;

    ALOGV("%s: num_models to merge %d", __func__, num_models);

    if (!stdev->smlib_handle) {
        ALOGE("%s: NULL sound model lib handle", __func__);
        return -ENOSYS;
    }

    sm_ret = stdev->smlib_getMergedModelSize(num_models, in_models,
        &out_model->size);
    if ((sm_ret != kSucess) || !out_model->size) {
        ALOGE("%s: smlib_getMergedModelSize failed, err %d, size %d", __func__,
            sm_ret, out_model->size);
        return -EINVAL;
    }
    ALOGD("%s: merge sound model size %d", __func__, out_model->size);

    out_model->data = calloc(1, out_model->size * sizeof(char));
    if (!out_model->data) {
        ALOGE("%s: Merged sound model allocation failed", __func__);
        return -ENOMEM;
    }

    sm_ret = stdev->smlib_mergeModels(num_models, in_models, out_model);
    if (sm_ret != kSucess) {
        ALOGE("%s: smlib_mergeModels failed, err %d", __func__, sm_ret);
        status = -EINVAL;
        goto cleanup;
    }
    if (!out_model->data || !out_model->size) {
        ALOGE("%s: MergeModels returned NULL data or size %d", __func__,
              out_model->size);
        status = -EINVAL;
        goto cleanup;
    }
    if (stdev->enable_debug_dumps) {
        ST_DBG_DECLARE(FILE *sm_fd = NULL; static int sm_cnt = 0);
        ST_DBG_FILE_OPEN_WR(sm_fd, ST_DEBUG_DUMP_LOCATION,
            "st_smlib_output_merged_sm", "bin", sm_cnt++);
        ST_DBG_FILE_WRITE(sm_fd, out_model->data, out_model->size);
        ST_DBG_FILE_CLOSE(sm_fd);
    }
    ALOGV("%s: Exit", __func__);
    return 0;

cleanup:
    if (out_model->data) {
        free(out_model->data);
        out_model->data = NULL;
        out_model->size = 0;
    }
    return status;
}

static int delete_from_merged_sound_model(struct sound_trigger_device *stdev,
    char **keyphrases, unsigned int num_keyphrases,
    listen_model_type *in_model, listen_model_type *out_model)
{
    listen_model_type merge_model = {0};
    listen_status_enum sm_ret = 0;
    unsigned int out_model_sz = 0;
    int status = 0, i = 0;

    out_model->data = NULL;
    out_model->size = 0;
    merge_model.data = in_model->data;
    merge_model.size = in_model->size;

    for (i = 0; i < num_keyphrases; i++) {
        sm_ret = stdev->smlib_getSizeAfterDeleting(&merge_model, keyphrases[i],
                                                   NULL, &out_model_sz);
        if (sm_ret != kSucess) {
            ALOGE("%s: smlib_getSizeAfterDeleting failed %d", __func__, sm_ret);
            status = -EINVAL;
            goto cleanup;
        }
        if (out_model_sz >= in_model->size) {
            ALOGE("%s: unexpected, smlib_getSizeAfterDeleting returned size %d"
                  "not less than merged model size %d", __func__,
                  out_model_sz, in_model->size);
            status = -EINVAL;
            goto cleanup;
        }
        ALOGV("%s: Size after deleting kw[%d] = %d", __func__, i, out_model_sz);
        if (!out_model->data) {
            /* Valid if deleting multiple keyphrases one after other */
            free (out_model->data);
            out_model->size = 0;
        }
        out_model->data = calloc(1, out_model_sz * sizeof(char));
        if (!out_model->data) {
            ALOGE("%s: Merge sound model allocation failed, size %d ", __func__,
                  out_model_sz);
            status = -ENOMEM;
            goto cleanup;
        }
        out_model->size = out_model_sz;

        sm_ret = stdev->smlib_deleteFromModel(&merge_model, keyphrases[i],
                                              NULL, out_model);
        if (sm_ret != kSucess) {
            ALOGE("%s: smlib_getSizeAfterDeleting failed %d", __func__, sm_ret);
            status = -EINVAL;
            goto cleanup;
        }
        if (out_model->size != out_model_sz) {
            ALOGE("%s: unexpected, out_model size %d != expected size %d",
                  __func__, out_model->size, out_model_sz);
            status = -EINVAL;
            goto cleanup;
        }
        /* Used if deleting multiple keyphrases one after other */
        merge_model.data = out_model->data;
        merge_model.size = out_model->size;
    }
    if (stdev->enable_debug_dumps && out_model->data && out_model->size) {
        ST_DBG_DECLARE(FILE *sm_fd = NULL; static int sm_cnt = 0);
        ST_DBG_FILE_OPEN_WR(sm_fd, ST_DEBUG_DUMP_LOCATION,
            "st_smlib_output_deleted_sm", "bin", sm_cnt++);
        ST_DBG_FILE_WRITE(sm_fd, out_model->data, out_model->size);
        ST_DBG_FILE_CLOSE(sm_fd);
    }
    return 0;

cleanup:
    if (out_model->data) {
        free(out_model->data);
        out_model->data = NULL;
    }
    return status;
}

static void release_sound_model_info(struct sound_model_info *sm_info)
{
    ALOGV("%s", __func__);

    if (sm_info->cf_levels) {
        free(sm_info->cf_levels);
        sm_info->cf_levels = NULL;
        sm_info->det_cf_levels = NULL;
    }
    free_array_ptrs(sm_info->keyphrases, sm_info->num_keyphrases);
    sm_info->keyphrases = NULL;

    free_array_ptrs(sm_info->users, sm_info->num_users);
    sm_info->users = NULL;

    free_array_ptrs(sm_info->cf_levels_kw_users, sm_info->cf_levels_size);
    sm_info->cf_levels_kw_users = NULL;
}

static inline void stdbg_print_sound_model_header(
   listen_sound_model_header *smh)
{
    int i = 0, j = 0;

    ALOGV("%s", __func__);
    ALOGV("numKeywords = %d", smh->numKeywords);
    ALOGV("numUsers = %d", smh->numUsers);
    ALOGV("numActiveUserKeywordPairs = %d", smh->numActiveUserKeywordPairs);
    ALOGV("isStripped = %d", smh->isStripped);
    ALOGV("model_indicator = %d", smh->model_indicator);

    for (i = 0; i < smh->numKeywords; i++) {
        ALOGV("kw-%d langPerKw = %d", i, smh->langPerKw[i]);
        ALOGV("kw-%d numUsersSetPerKw = %d", i, smh->numUsersSetPerKw[i]);
        ALOGV("kw-%d isUserDefinedKeyword = %d", i,
              smh->isUserDefinedKeyword[i]);
    }
    if (smh->userKeywordPairFlags) {
        for (i = 0; i < smh->numUsers; i++) {
            for (j = 0; j < smh->numKeywords; j++)
                ALOGV("userKeywordPairFlags[%d][%d] = %d", i, j,
                    smh->userKeywordPairFlags[i][j]);
        }
    }
}

static int query_sound_model(struct sound_trigger_device *stdev,
    struct sound_model_info *sm_info, unsigned char *sm_data,
    unsigned int sm_size)
{
    listen_sound_model_header sm_header = {0};
    listen_model_type model = {0};
    listen_status_enum sm_ret = 0;
    int status = 0, i = 0, j = 0, k = 0;
    uint16_t tmp = 0;

    ALOGV("%s: enter sm_size %d", __func__, sm_size);

    if (!stdev->smlib_handle) {
        ALOGE("%s: NULL sound model lib handle", __func__);
        return -ENOSYS;
    }

    model.data = sm_data;
    model.size = sm_size;

    sm_ret = stdev->smlib_getSoundModelHeader(&model, &sm_header);
    if (sm_ret != kSucess) {
        ALOGE("%s: smlib_getSoundModelHeader failed, err %d ", __func__, sm_ret);
        return -EINVAL;
    }
    if (sm_header.numKeywords == 0) {
        ALOGE("%s: num keywords zero!!", __func__);
        return -EINVAL;
    }
    if (sm_header.numActiveUserKeywordPairs < sm_header.numUsers) {
        ALOGE("%s: smlib activeUserKwPairs(%d) < total users (%d)", __func__,
                sm_header.numActiveUserKeywordPairs, sm_header.numUsers);
        goto cleanup;
    }
    if (sm_header.numUsers && !sm_header.userKeywordPairFlags) {
        ALOGE("%s: userKeywordPairFlags is NULL, numUsers (%d)", __func__,
                sm_header.numUsers);
        goto cleanup;
    }
    stdbg_print_sound_model_header(&sm_header);

    /* MAX_STRING_LEN is part of listen sound model header file */
    alloc_array_ptrs(&sm_info->keyphrases, sm_header.numKeywords,
        MAX_STRING_LEN);
    if (!sm_info->keyphrases) {
        ALOGE("%s: keyphrases allocation failed", __func__);
        status = -ENOMEM;
        goto cleanup;
    }
    sm_info->num_keyphrases = sm_header.numKeywords;
    sm_info->num_users = sm_header.numUsers;

    tmp = sm_header.numKeywords;
    ALOGV("%s: stdb: model.data %pK, model.size %d", __func__, model.data,
            model.size);
    sm_ret = stdev->smlib_getKeywordPhrases(&model, &tmp, sm_info->keyphrases);
    if (sm_ret) {
        ALOGE("%s: smlib_getKeywordPhrases failed, err %d ", __func__, sm_ret);
        goto cleanup;
    }
    if (tmp != sm_header.numKeywords) {
        ALOGE("%s: smlib_getkeywordPhrases(%d) != sml header (%d)", __func__,
              tmp, sm_header.numKeywords);
        goto cleanup;
    }
    for (i = 0; i < sm_header.numKeywords; i++)
        ALOGV("%s: keyphrases names = %s", __func__, sm_info->keyphrases[i]);

    if (sm_header.numUsers) {
        alloc_array_ptrs(&sm_info->users, sm_header.numUsers, MAX_STRING_LEN);
        if (!sm_info->users) {
            ALOGE("%s: users allocation failed", __func__);
            status = -ENOMEM;
            goto cleanup;
        }

        tmp = sm_header.numUsers;
        sm_ret = stdev->smlib_getUserNames(&model, &tmp, sm_info->users);
        if (sm_ret) {
            ALOGE("%s: smlib_getUserNames failed, err %d ", __func__, sm_ret);
            goto cleanup;
        }
        if (tmp != sm_header.numUsers) {
            ALOGE("%s: smlib_getUserNames(%d) != sml header (%d)", __func__,
                tmp, sm_header.numUsers);
            status = -EINVAL;
            goto cleanup;
        }
        for (i = 0; i < sm_header.numUsers; i++)
            ALOGV("%s: users names = %s", __func__, sm_info->users[i]);
    }

    sm_info->cf_levels_size = sm_header.numKeywords +
        sm_header.numActiveUserKeywordPairs;
    alloc_array_ptrs(&sm_info->cf_levels_kw_users, sm_info->cf_levels_size,
                     MAX_KW_USERS_NAME_LEN);
    if (!sm_info->cf_levels_kw_users) {
        ALOGE("%s: cf_levels_kw_users allocation failed", __func__);
        status = -ENOMEM;
        goto cleanup;
    }

    /* Used later for mapping client to/from merged DSP confidence levels */
    sm_info->cf_levels = calloc(1, 2 * sm_info->cf_levels_size);
    if (!sm_info->cf_levels) {
        ALOGE("%s: cf_levels allocation failed", __func__);
        status = -ENOMEM;
        goto cleanup;
    }
    /*
     * Used for updating detection confidence level values from DSP merged
     * detection conf levels
     */
    sm_info->det_cf_levels = sm_info->cf_levels + sm_info->cf_levels_size;

    /*
     * Used for conf level setting to DSP. Reset the conf value to max value,
     * so that the keyword of a loaded and in-active model in a merged model
     * doesn't detect.
     */
    memset(sm_info->cf_levels, MAX_CONF_LEVEL_VALUE, sm_info->cf_levels_size);

    /*
     * Derive the confidence level payload for keyword and user pairs.
     * Store the user-keyword pair names in an array that will be used for
     * mapping the DSP detection and confidence levels to the client.
     */
    char **kw_names = sm_info->cf_levels_kw_users;
    char **ukw_names = &sm_info->cf_levels_kw_users[sm_header.numKeywords];
    int ukw_idx = 0;

    for (i = 0; i < sm_header.numKeywords; i++) {
        strlcpy(kw_names[i], sm_info->keyphrases[i], MAX_KW_USERS_NAME_LEN);
        if (!sm_header.numUsersSetPerKw)
            continue;
        for (j = 0, k = 0; j < sm_header.numUsers; j++) {
            if (k >= sm_header.numUsersSetPerKw[i])
                break;
            if (sm_header.userKeywordPairFlags[j][i]) {
                strlcpy(ukw_names[ukw_idx], sm_info->users[j],
                    MAX_KW_USERS_NAME_LEN);
                strlcat(ukw_names[ukw_idx], sm_info->keyphrases[i],
                    MAX_KW_USERS_NAME_LEN);
                ukw_idx++;
                k++;
            }
        }
    }
    for (i = 0; i < sm_info->cf_levels_size; i++)
        ALOGV("%s: cf_levels_kw_users = %s", __func__,
                sm_info->cf_levels_kw_users[i]);

    sm_ret = stdev->smlib_releaseSoundModelHeader(&sm_header);
    if (sm_ret != kSucess) {
        ALOGE("%s: smlib_releaseSoundModelHeader failed, err %d ", __func__,
                sm_ret);
        status = -EINVAL;
        goto cleanup_1;
    }
    ALOGV("%s: exit", __func__);
    return 0;

cleanup:
    sm_ret = stdev->smlib_releaseSoundModelHeader(&sm_header);
    if (sm_ret != kSucess)
        ALOGE("%s: smlib_releaseSoundModelHeader failed, err %d ", __func__,
                sm_ret);

cleanup_1:
    release_sound_model_info(sm_info);

    return status;
}

static int add_sound_model(st_session_t *stc_ses, unsigned char *sm_data,
    unsigned int sm_size)
{
    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;
    listen_model_type **in_models = NULL;
    listen_model_type out_model = {0};
    struct sound_model_info sm_info = {0};
    int status = 0, num_models = 0;

    ALOGV("%s:[c%d]", __func__, stc_ses->sm_handle);
    if (stc_ses->sm_info.sm_data) {
        ALOGD("%s:[c%d] Already added", __func__, stc_ses->sm_handle);
        return 0;
    }
    if (!st_ses->vendor_uuid_info->merge_fs_soundmodels) {
        stc_ses->sm_info.sm_data = sm_data;
        stc_ses->sm_info.sm_size = sm_size;
        st_ses->sm_info.sm_data = sm_data;
        st_ses->sm_info.sm_size = sm_size;
        st_ses->sm_info.sm_type = stc_ses->sm_type;
        ALOGD("%s:[c%d] no merge", __func__, stc_ses->sm_handle);
        return 0;
    }
    /* get sound model header information for client model */
    status = query_sound_model(st_ses->stdev, &stc_ses->sm_info,
                               sm_data, sm_size);
    if (status)
        return status;

    stc_ses->sm_info.sm_data = sm_data;
    stc_ses->sm_info.sm_size = sm_size;
    ALOGV("%s: stc_ses %pK - sm_data %pK, sm_size %d", __func__,
          stc_ses, stc_ses->sm_info.sm_data,
          stc_ses->sm_info.sm_size);

    /* Check for remaining client sound models to merge */
    list_for_each(node, &st_ses->clients_list) {
        c_ses = node_to_item(node, st_session_t, hw_list_node);
        if ((c_ses != stc_ses) && c_ses->sm_info.sm_data)
            num_models++;
    }
    if (!num_models) {
        if (st_ses->sm_info.sm_merged && st_ses->sm_info.sm_data) {
            free(st_ses->sm_info.sm_data);
        }
        /* Only one current client model, just re-use it */
        st_ses->recognition_mode = stc_ses->recognition_mode;
        stc_ses->sm_info.sm_type = stc_ses->sm_type;
        st_ses->sm_info = stc_ses->sm_info;
        st_ses->sm_info.sm_merged = false;
        ALOGD("%s: re-use single client c%d model, size %d", __func__,
              stc_ses->sm_handle, stc_ses->sm_info.sm_size);
        return 0;
    }
    ALOGV("%s: num existing models %d", __func__, num_models);
    /*
     * Merge this client model with already existing merged model due to other
     * clients models.
     */
    if (!st_ses->sm_info.sm_data) {
        if (num_models == 1) {
            /*
             * Its not a merged model yet, but proxy ses sm_data is valid and
             * must be pointing to client sm_data
             */
            ALOGE("%s: Unexpected, sm_info.sm_data NULL, num current"
                  "models %d", __func__, num_models);
            status = -EINVAL;
            goto cleanup;
        } else if (!st_ses->sm_info.sm_merged) {
            ALOGE("%s: Unexpected, no pre-existing merged model, num current"
                  "models %d", __func__, num_models);
            status = -EINVAL;
            goto cleanup;
        }
    }

    /* Merge this client model with remaining clients models */
    num_models = 2;/* re-use */
    alloc_array_ptrs((char***)&in_models, num_models, sizeof(listen_model_type));
    if (!in_models) {
        ALOGE("%s: in_models allocation failed", __func__);
        status = -ENOMEM;
        goto cleanup;
    }
    /* Add existing model */
    in_models[0]->data = st_ses->sm_info.sm_data;
    in_models[0]->size = st_ses->sm_info.sm_size;
    /* Add this client model */
    in_models[1]->data = sm_data;
    in_models[1]->size = sm_size;

    status = merge_sound_models(st_ses->stdev, 2, in_models, &out_model);
    if (status)
        goto cleanup;

    sm_info.sm_data = out_model.data;
    sm_info.sm_size = out_model.size;
    sm_info.sm_merged = true;

    status = query_sound_model(st_ses->stdev, &sm_info,
                               out_model.data, out_model.size);
    if (status)
        goto cleanup;

    if (out_model.size < st_ses->sm_info.sm_size) {
        ALOGE("%s: Unexpected, merged model sz %d < current sz %d",
            __func__, out_model.size, st_ses->sm_info.sm_size);
        release_sound_model_info(&sm_info);
        status = -EINVAL;
        goto cleanup;
    }
    free_array_ptrs((char **)in_models, num_models);
    in_models = NULL;

    /* Update the new merged model */
    if (st_ses->sm_info.sm_merged && st_ses->sm_info.sm_data) {
        release_sound_model_info(&st_ses->sm_info);
        free(st_ses->sm_info.sm_data);
    }
    ALOGD("%s: Updated sound model: current size %d, new size %d", __func__,
        st_ses->sm_info.sm_size, out_model.size);
    st_ses->sm_info = sm_info;

    /*
     * If any of the clients has user identificaiton enabled, underlying
     * hw session has to operate with user identification enabled.
     */
    if (stc_ses->recognition_mode & RECOGNITION_MODE_USER_IDENTIFICATION)
        st_ses->recognition_mode |= stc_ses->recognition_mode;

    return 0;

cleanup:
    release_sound_model_info(&stc_ses->sm_info);
    stc_ses->sm_info.sm_data = NULL;

    if (out_model.data)
        free(out_model.data);

    if (in_models)
        free_array_ptrs((char **)in_models, num_models);

    return status;
}

static int delete_sound_model(st_session_t *stc_ses)
{
    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL, *c_ses_rem = NULL;
    listen_model_type in_model = {0};
    listen_model_type out_model = {0};
    struct sound_model_info sm_info = {0};
    int status = 0, num_models = 0;
    unsigned int rec_mode = RECOGNITION_MODE_VOICE_TRIGGER;

    ALOGV("%s:[c%d]", __func__, stc_ses->sm_handle);
    if (!stc_ses->sm_info.sm_data) {
        ALOGD("%s:[c%d] Already deleted", __func__, stc_ses->sm_handle);
        return 0;
    }
    if (!st_ses->vendor_uuid_info->merge_fs_soundmodels) {
        /*
         * As it directly points to client model, just set as NULL
         * without freeing
         */
        st_ses->sm_info.sm_data = NULL;
        stc_ses->sm_info.sm_data = NULL;
        ALOGD("%s:[c%d] no merge", __func__, stc_ses->sm_handle);
        return 0;
    }

    ALOGV("%s: stc_ses %pK - sm_data %pK, sm_size %d", __func__,
          stc_ses, stc_ses->sm_info.sm_data,
          stc_ses->sm_info.sm_size);

    /* Check for remaining clients sound models to merge */
    list_for_each(node, &st_ses->clients_list) {
        c_ses = node_to_item(node, st_session_t, hw_list_node);
        if ((c_ses != stc_ses) && c_ses->sm_info.sm_data) {
            c_ses_rem = c_ses;
            num_models++;
        }
    }
    if (num_models == 0) {
        ALOGD("%s: No remaining models", __func__);
        /* Delete current client model */
        release_sound_model_info(&stc_ses->sm_info);
        stc_ses->sm_info.sm_data = NULL;
        return 0;
    }

    if (num_models == 1) {
        ALOGD("%s: reuse only remaining client model, size %d", __func__,
            c_ses_rem->sm_info.sm_size);
        /* If only one remaining client model exists, re-use it */
        if (st_ses->sm_info.sm_merged) {
            release_sound_model_info(&st_ses->sm_info);
            if (st_ses->sm_info.sm_data)
                free(st_ses->sm_info.sm_data);
        }
        st_ses->sm_info = c_ses_rem->sm_info;
        st_ses->sm_info.sm_merged = false;
        st_ses->hw_ses_current->sthw_cfg.conf_levels =
            st_ses->sm_info.cf_levels;
        st_ses->hw_ses_current->sthw_cfg.num_conf_levels =
            st_ses->sm_info.cf_levels_size;
        st_ses->recognition_mode = c_ses_rem->recognition_mode;
        /* Delete current client model */
        release_sound_model_info(&stc_ses->sm_info);
        stc_ses->sm_info.sm_data = NULL;
        return 0;
    }
    /* Update overall recogniton mode from remaining clients */
    list_for_each(node, &st_ses->clients_list) {
        c_ses = node_to_item(node, st_session_t, hw_list_node);
        if ((c_ses != stc_ses) && c_ses->sm_info.sm_data) {
            if (c_ses->recognition_mode & RECOGNITION_MODE_USER_IDENTIFICATION)
                rec_mode |=  RECOGNITION_MODE_USER_IDENTIFICATION;
        }
    }

    /*
     * Delete this client model with already existing merged model due to other
     * clients models.
     */
    if (!st_ses->sm_info.sm_merged || !st_ses->sm_info.sm_data) {
        ALOGE("%s: Unexpected, no pre-existing merged model to delete from,"
              "num current models %d", __func__, num_models);
        goto cleanup;
    }

    /* Existing merged model from which the current client model to be deleted */
    in_model.data = st_ses->sm_info.sm_data;
    in_model.size = st_ses->sm_info.sm_size;

    status = delete_from_merged_sound_model(st_ses->stdev,
        stc_ses->sm_info.keyphrases, stc_ses->sm_info.num_keyphrases,
        &in_model, &out_model);

    if (status)
        goto cleanup;

    sm_info.sm_data = out_model.data;
    sm_info.sm_size = out_model.size;
    sm_info.sm_merged = true;

    /* Update existing merged model info with new merged model */
    status = query_sound_model(st_ses->stdev, &sm_info, out_model.data,
                               out_model.size);
    if (status)
        goto cleanup;

    if (out_model.size > st_ses->sm_info.sm_size) {
        ALOGE("%s: Unexpected, merged model sz %d > current sz %d",
            __func__, out_model.size, st_ses->sm_info.sm_size);
        release_sound_model_info(&sm_info);
        status = -EINVAL;
        goto cleanup;
    }

    if (st_ses->sm_info.sm_merged && st_ses->sm_info.sm_data) {
        release_sound_model_info(&st_ses->sm_info);
        free(st_ses->sm_info.sm_data);
    }

    ALOGD("%s: Updated sound model: current size %d, new size %d", __func__,
        st_ses->sm_info.sm_size, out_model.size);
    st_ses->sm_info = sm_info;
    /*
     * If any of the remaining clients has user identificaiton enabled,
     * underlying hw session has to operate with user identificaiton enabled.
     */
    st_ses->recognition_mode = rec_mode;

    /* Release current client model */
    release_sound_model_info(&stc_ses->sm_info);
    stc_ses->sm_info.sm_data = NULL;

    return 0;

cleanup:
    release_sound_model_info(&stc_ses->sm_info);
    stc_ses->sm_info.sm_data = NULL;

    if (out_model.data)
        free(out_model.data);

    return status;
}

static int update_sound_model(st_session_t *stc_ses, bool add)
{
    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    struct sound_trigger_phrase_sound_model *phrase_sm =  stc_ses->phrase_sm;
    struct sound_trigger_sound_model *common_sm =
        (struct sound_trigger_sound_model*)stc_ses->phrase_sm;
    unsigned char *sm_data = NULL;
    unsigned int sm_size = 0;
    int status = 0;

    ALOGV("%s: Enter", __func__);

    if (stc_ses->sm_type == SOUND_MODEL_TYPE_KEYPHRASE) {
        sm_data = (unsigned char*)phrase_sm + phrase_sm->common.data_offset;
        sm_size = phrase_sm->common.data_size;
    } else {
        sm_data = (unsigned char*)common_sm + common_sm->data_offset;
        sm_size = common_sm->data_size;
    }

    pthread_mutex_lock(&st_ses->lock);
    if (add)
        status = add_sound_model(stc_ses, sm_data, sm_size);
    else
        status = delete_sound_model(stc_ses);
    pthread_mutex_unlock(&st_ses->lock);

    ALOGV("%s: Exit", __func__);
    return status;
}

static int update_merge_conf_levels_payload(st_proxy_session_t *st_ses,
    struct sound_model_info *src_sm_info, unsigned char *src,
    unsigned int src_size, bool set)
{
    int i = 0, j = 0;

    if (!st_ses || !src) {
        ALOGE("%s: NULL pointer", __func__);
        return -EINVAL;
    }

    if (!st_ses->sm_info.sm_merged)
        return 0;

    if (src_size > st_ses->sm_info.cf_levels_size) {
        ALOGE("%s:[%d] Unexpected, client conf levels %d > "
            "merged conf levels %d", __func__, st_ses->sm_handle,
            src_size, st_ses->sm_info.cf_levels_size);
        return -EINVAL;
    }

    for (i = 0; i < src_size; i++)
        ALOGV("%s: src cf_levels[%d]=%d", __func__, i, src[i]);

    /* Populate DSP merged sound model conf levels */
    for (i = 0; i < src_size; i++) {
        for (j = 0; j < st_ses->sm_info.cf_levels_size; j++) {
            if (!strcmp(st_ses->sm_info.cf_levels_kw_users[j],
                        src_sm_info->cf_levels_kw_users[i])) {
                if (set) {
                    st_ses->sm_info.cf_levels[j] = src[i];
                    ALOGV("%s: set: sm_info.cf_levels[%d]=%d", __func__,
                          j, st_ses->sm_info.cf_levels[j]);
                } else {
                    st_ses->sm_info.cf_levels[j] = MAX_CONF_LEVEL_VALUE;
                    ALOGV("%s: reset: sm_info.cf_levels[%d]=%d", __func__,
                          j, st_ses->sm_info.cf_levels[j]);
                }
            }
        }
    }
    return 0;
}

static int update_merge_conf_levels_payload_with_active_clients(
   st_proxy_session_t *st_ses)
{
    int status = 0;
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;

    list_for_each(node, &st_ses->clients_list) {
        c_ses = node_to_item(node, st_session_t, hw_list_node);
        if (c_ses->state == ST_STATE_ACTIVE) {
            ALOGV("%s: update merge conf levels with other active"
                  "client %d ", __func__, c_ses->sm_handle);
            status = update_merge_conf_levels_payload(st_ses,
                &c_ses->sm_info, c_ses->sm_info.cf_levels,
                c_ses->sm_info.cf_levels_size, true);
            if (status)
                return status;
        }
    }
    return status;
}

static void check_and_extract_det_conf_levels_payload(
    st_proxy_session_t *st_ses,
    unsigned char *src, unsigned int src_size,
    unsigned char **dst, unsigned int *dst_size)
{
    st_session_t *stc_ses = st_ses->det_stc_ses;
    int i = 0, j = 0;

    *dst = src;
    *dst_size = src_size;

    if (!st_ses->vendor_uuid_info->merge_fs_soundmodels ||
        !st_ses->sm_info.sm_merged) {
        ALOGV("%s:[%d] not merged", __func__, st_ses->sm_handle);
        return;
    }

    if (src_size < st_ses->sm_info.cf_levels_size) {
        ALOGE("%s:[%d] Unexpected, detection conf payload size %d < %d",
              __func__, st_ses->sm_handle, src_size,
              st_ses->sm_info.cf_levels_size);
        return;
    }

    /* Reset any cached previous detection values */
    memset(stc_ses->sm_info.det_cf_levels, 0, stc_ses->sm_info.cf_levels_size);

    /* Extract the client conf level values from DSP payload */
    for(i = 0; i < st_ses->sm_info.cf_levels_size; i++) {
        if (!src[i])
            continue;
        for(j = 0; j < stc_ses->sm_info.cf_levels_size; j++) {
            if (!strcmp(stc_ses->sm_info.cf_levels_kw_users[j],
                        st_ses->sm_info.cf_levels_kw_users[i])) {
                stc_ses->sm_info.det_cf_levels[j] = src[i];
            }
        }
    }
    for (i = 0; i < stc_ses->sm_info.cf_levels_size; i++)
        ALOGD("%s: c%d det_cf_levels[%d]=%d", __func__, stc_ses->sm_handle, i,
              stc_ses->sm_info.det_cf_levels[i]);

    *dst = stc_ses->sm_info.det_cf_levels;
    *dst_size = stc_ses->sm_info.cf_levels_size;
}

static inline bool check_for_multi_clients(st_proxy_session_t *st_ses)
{
    struct listnode *node = NULL;
    int cnt = 0;

    list_for_each(node, &st_ses->clients_list) {
        if (++cnt > 1)
            return true;
    }
    return false;
}

static inline bool is_other_client_attached(st_proxy_session_t *st_ses,
    st_session_t *stc_ses)
{
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;

    list_for_each(node, &st_ses->clients_list) {
        c_ses = node_to_item(node, st_session_t, hw_list_node);
        if (c_ses != stc_ses)
            return true;
    }
    return false;
}

static inline void reset_clients_pending_load(st_proxy_session_t *st_ses)
{
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;

    list_for_each(node, &st_ses->clients_list) {
        c_ses = node_to_item(node, st_session_t, hw_list_node);
        c_ses->pending_load = false;
    }
}

static inline int is_any_client_not_pending_load(st_proxy_session_t *st_ses)
{
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;

    list_for_each(node, &st_ses->clients_list) {
        c_ses = node_to_item(node, st_session_t, hw_list_node);
        if (!c_ses->pending_load)
            return true;
    }
    return false;
}

static inline int is_any_client_not_pending_set_device(
   st_proxy_session_t *st_ses)
{
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;

    list_for_each(node, &st_ses->clients_list) {
        c_ses = node_to_item(node, st_session_t, hw_list_node);
        if (!c_ses->pending_set_device)
            return true;
    }
    return false;
}

static inline void reset_clients_pending_set_device(st_proxy_session_t *st_ses)
{
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;

    list_for_each(node, &st_ses->clients_list) {
        c_ses = node_to_item(node, st_session_t, hw_list_node);
        c_ses->pending_set_device = false;
    }
}

static inline bool is_any_client_paused(st_proxy_session_t *st_ses)
{
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;

    list_for_each(node, &st_ses->clients_list) {
        c_ses = node_to_item(node, st_session_t, hw_list_node);
        if (c_ses->paused)
            return true;
    }
    return false;
}

static inline bool is_any_client_in_state(st_proxy_session_t *st_ses,
    enum client_states_t state)
{
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;

    list_for_each(node, &st_ses->clients_list) {
        c_ses = node_to_item(node, st_session_t, hw_list_node);
        if (c_ses->state == state)
            return true;
    }
    return false;
}

static void update_hw_config_on_restart(st_proxy_session_t *st_ses,
    st_session_t *stc_ses)
{
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    struct st_hw_ses_config *sthw_cfg = &hw_ses->sthw_cfg;
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;
    int hb_sz = 0, pr_sz = 0;
    bool enable_lab = false;

    if (!st_ses->vendor_uuid_info->merge_fs_soundmodels ||
        !st_ses->sm_info.sm_merged)
        return;

    /*
     * Adjust history buffer and preroll durations to highest of
     * all clients, including current restarting client.
     * If any of the clients requested capture or enabled the
     * second stage, the underlying hw session buffering needs to be
     * enabled.
     */
    list_for_each(node, &st_ses->clients_list) {
        c_ses = node_to_item(node, st_session_t, hw_list_node);
        if ((c_ses == stc_ses) || (c_ses->state == ST_STATE_ACTIVE)) {
            if (hb_sz < c_ses->hist_buf_duration)
                hb_sz = c_ses->hist_buf_duration;
            if (pr_sz < c_ses->preroll_duration)
                pr_sz = c_ses->preroll_duration;
            if (!enable_lab)
                enable_lab = (c_ses->rc_config &&
                              c_ses->rc_config->capture_requested) ||
                             !list_empty(&c_ses->second_stage_list);
        }
    }

    sthw_cfg->client_req_hist_buf = hb_sz;
    sthw_cfg->client_req_preroll = pr_sz;
    st_ses->lab_enabled = enable_lab;

    update_merge_conf_levels_payload(st_ses, &stc_ses->sm_info,
                                     stc_ses->sm_info.cf_levels,
                                     stc_ses->sm_info.cf_levels_size,
                                     true);
    hw_ses->sthw_cfg_updated = true;

    ALOGV("%s:[%d] lab_enabled %d, hb_sz %d, pr_sz %d", __func__,
        st_ses->sm_handle, st_ses->lab_enabled,
        sthw_cfg->client_req_hist_buf, sthw_cfg->client_req_preroll);
}

static bool update_hw_config_on_stop(st_proxy_session_t *st_ses,
    st_session_t *stc_ses)
{
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    struct st_hw_ses_config *sthw_cfg = &hw_ses->sthw_cfg;
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;
    int hb_sz = 0, pr_sz = 0;
    bool active = false, enable_lab = false;

    if (!st_ses->vendor_uuid_info->merge_fs_soundmodels) {
        if (sthw_cfg->conf_levels) {
            ALOGV("%s: free hw conf_levels", __func__);
            free(sthw_cfg->conf_levels);
            sthw_cfg->conf_levels = NULL;
        }
        return false;
    }
    /*
     * Adjust history buffer and preroll durations to highest of
     * remaining clients.
     * If any of the remaining clients requested capture or enabled the
     * second stage, the underlying hw session buffering needs to be
     * enabled.
     */
    list_for_each(node, &st_ses->clients_list) {
        c_ses = node_to_item(node, st_session_t, hw_list_node);
        if ((c_ses != stc_ses) && (c_ses->state == ST_STATE_ACTIVE)) {
            active = true;
            if (hb_sz < c_ses->hist_buf_duration)
                hb_sz = c_ses->hist_buf_duration;
            if (pr_sz < c_ses->preroll_duration)
                pr_sz = c_ses->preroll_duration;
            if (!enable_lab)
                enable_lab = c_ses->rc_config->capture_requested ||
                             !list_empty(&c_ses->second_stage_list);
        }
    }
    if (!active) {
        sthw_cfg->client_req_hist_buf = 0;
        sthw_cfg->client_req_preroll = 0;
        st_ses->lab_enabled = 0;
        sthw_cfg->custom_data = NULL;
        sthw_cfg->custom_data_size = 0;
        hw_ses->sthw_cfg_updated = true;
        ALOGV("%s:[%d] no active client hw cfg is reset", __func__,
              st_ses->sm_handle);
        return false;
    }

    sthw_cfg->client_req_hist_buf = hb_sz;
    sthw_cfg->client_req_preroll = pr_sz;
    st_ses->lab_enabled = enable_lab;

    update_merge_conf_levels_payload(st_ses, &stc_ses->sm_info,
                                     stc_ses->sm_info.cf_levels,
                                     stc_ses->sm_info.cf_levels_size,
                                     false);
    hw_ses->sthw_cfg_updated = true;

    ALOGV("%s:[%d] lab_enabled %d, hb_sz %d, pr_sz %d", __func__,
        st_ses->sm_handle, st_ses->lab_enabled,
        sthw_cfg->client_req_hist_buf, sthw_cfg->client_req_preroll);

    return active;
}

static void get_conf_levels_from_dsp_payload(st_proxy_session_t *st_ses,
    unsigned char *payload, unsigned int payload_size,
    unsigned char **conf_levels, unsigned int *conf_levels_size)
{
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    uint32_t key_id = 0, key_payload_size = 0;
    unsigned int i = 0;

    if (hw_ses->is_generic_event) {
        while (i < payload_size) {
            key_id = *(uint32_t *)payload;
            key_payload_size = *((uint32_t *)payload + 1);

            if (key_id == KEY_ID_CONFIDENCE_LEVELS) {
                *conf_levels = payload + (4 * sizeof(uint32_t));
                *conf_levels_size = *((uint32_t *)payload + 3);;
                ALOGV("%s: generic_event: DSP num conf levels %d", __func__,
                      *conf_levels_size);
                break;
            }
            i += GENERIC_DET_EVENT_HEADER_SIZE + key_payload_size;
            payload += i;
        }
    } else {
        if ((st_ses->exec_mode == ST_EXEC_MODE_CPE) && st_ses->stdev->is_gcs) {
            *conf_levels = payload + 2;
            *conf_levels_size = payload_size - 2;
        } else {
            *conf_levels = payload;
            *conf_levels_size = payload_size;
        }
    }
}

st_session_t* get_detected_client(st_proxy_session_t *st_ses,
    unsigned char *payload, unsigned int payload_size)
{
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;
    unsigned char *conf_levels = NULL;
    unsigned int conf_levels_size = 0;
    int i = 0, j = 0;

    if (list_empty(&st_ses->clients_list)) {
        ALOGE("%s:[%d] no clients attached!!", __func__, st_ses->sm_handle);
        return NULL;
    }
    /*
     * If only single client exist, this detection is not for merged
     * sound model, hence return this as only available client
     */
    if (!check_for_multi_clients(st_ses)) {
        ALOGV("%s:[%d] single client detection", __func__, st_ses->sm_handle);
        node = list_head(&st_ses->clients_list);
        c_ses = node_to_item(node, st_session_t, hw_list_node);
        if (c_ses->state == ST_STATE_ACTIVE) {
            ALOGD("%s: detected c%d", __func__, c_ses->sm_handle);
            return c_ses;
        } else {
            ALOGE("%s: detected c%d is not active", __func__, c_ses->sm_handle);
            return NULL;
        }
    }

    get_conf_levels_from_dsp_payload(st_ses, payload, payload_size,
                                     &conf_levels, &conf_levels_size);
    if (!conf_levels) {
        ALOGE("%s:[%d] no conf levels payload found!!", __func__,
            st_ses->sm_handle);
        return NULL;
    }
    if (conf_levels_size < st_ses->sm_info.num_keyphrases) {
        ALOGE("%s:[%d] detection conf levels size %d < num of keywords %d",
            __func__, st_ses->sm_handle, conf_levels_size,
            st_ses->sm_info.num_keyphrases);
        return NULL;
    }

    /*
     * The DSP payload contains the keyword conf levels from the beginning.
     * Only one keyword conf level is expected to be non-zero from keyword
     * detection. Find non-zero conf level up to number of keyphrases and if
     * one is found, match it to the corresponding keyphrase from list of
     * clients to obtain the detected client.
     */
    for (i = 0; i < st_ses->sm_info.num_keyphrases; i++) {
        if (!conf_levels[i])
            continue;
        list_for_each(node, &st_ses->clients_list) {
            c_ses = node_to_item(node, st_session_t, hw_list_node);
            for (j = 0; j < c_ses->sm_info.num_keyphrases; j++) {
                if (!strcmp(st_ses->sm_info.keyphrases[i],
                            c_ses->sm_info.keyphrases[j])) {
                    if (c_ses->state == ST_STATE_ACTIVE) {
                        ALOGV("%s: detected c%d", __func__, c_ses->sm_handle);
                        return c_ses;
                    } else {
                        ALOGE("%s: detected c%d is not active", __func__,
                            c_ses->sm_handle);
                        return NULL;
                    }
                }
            }
        }
    }
    return c_ses;
}

static int fill_conf_levels_payload_from_rc_config
(
   const struct sound_trigger_phrase_sound_model *phrase_sm,
   const struct sound_trigger_recognition_config *rc_config,
   unsigned char *conf_levels,
   unsigned int total_num_users
)
{
    int status = 0;
    unsigned int user_level, user_id;
    unsigned int i = 0, j = 0;
    unsigned int num_conf_levels = 0;
    unsigned char *user_id_tracker;

    if (!phrase_sm || !rc_config || !conf_levels) {
        ALOGE("%s: ERROR. Invalid inputs",__func__);
        return -EINVAL;
    }

    if ((UINT32_MAX - total_num_users) > rc_config->num_phrases)
        num_conf_levels = total_num_users + rc_config->num_phrases;

    if (!num_conf_levels) {
        ALOGE("%s: ERROR. Invalid num_conf_levels input", __func__);
        return -EINVAL;
    }

    /*  Example: Say the recognition structure has 3 keywords with users
     *  |kid|
     *  [0] k1 |uid|
     *         [3] u1 - 1st trainer
     *         [4] u2 - 4th trainer
     *         [6] u3 - 3rd trainer
     *  [1] k2
     *         [5] u2 - 2nd trainer
     *         [7] u3 - 5th trainer
     *  [2] k3
     *         [8] u4 - 6th trainer
     *
     *  Output confidence level array will be
     *  [k1, k2, k3, u1k1, u2k1, u2k2, u3k1, u3k2, u4k3]
     */

    user_id_tracker = calloc(1, num_conf_levels);
    if (!user_id_tracker) {
        ALOGE("%s: failed to allocate user_id_tracker", __func__);
        return -ENOMEM;
    }

    for (i = 0; i < rc_config->num_phrases; i++) {
        ALOGV("%s: [%d] kw level %d", __func__, i,
              rc_config->phrases[i].confidence_level);
        for (j = 0; j < rc_config->phrases[i].num_levels; j++) {
            ALOGV("%s: [%d] user_id %d level %d ", __func__, i,
                  rc_config->phrases[i].levels[j].user_id,
                  rc_config->phrases[i].levels[j].level);
        }
    }

    for (i = 0; i < rc_config->num_phrases; i++) {
        if (i < num_conf_levels) {
            conf_levels[i] = rc_config->phrases[i].confidence_level;
        } else {
            ALOGE("%s: ERROR. Invalid number of phrases", __func__);
            status = -EINVAL;
            goto exit;
        }
        for (j = 0; j < rc_config->phrases[i].num_levels; j++) {
            user_level = rc_config->phrases[i].levels[j].level;
            user_id = rc_config->phrases[i].levels[j].user_id;
            if ((user_id < rc_config->num_phrases) ||
                (user_id >= num_conf_levels)) {
                ALOGE("%s: ERROR. Invalid params user id %d>%d",
                      __func__, user_id, total_num_users);
                status = -EINVAL;
                goto exit;
            } else {
                if (user_id_tracker[user_id] == 1) {
                    ALOGE("%s: ERROR. Duplicate user id %d",
                          __func__, user_id);
                    status = -EINVAL;
                    goto exit;
                }
                conf_levels[user_id] = (user_level < 100) ? user_level: 100;
                user_id_tracker[user_id] = 1;
                ALOGV("%s: user_conf_levels[%d] = %d", __func__,
                                    user_id, conf_levels[user_id]);
            }
        }
    }

exit:
    free(user_id_tracker);
    return status;
}

int generate_conf_levels_payload_from_rc_config
(
   const struct sound_trigger_phrase_sound_model *phrase_sm,
   const struct sound_trigger_recognition_config *rc_config,
   unsigned char **out_payload,
   unsigned int *out_payload_size
)
{
    int status = 0;
    unsigned int total_num_users = 0, num_conf_levels = 0;
    unsigned int i = 0, j = 0;
    unsigned char *conf_levels = NULL;

    if (!phrase_sm || !rc_config || !out_payload || !out_payload_size) {
        ALOGE("%s: ERROR. Invalid inputs",__func__);
        status = -EINVAL;
        goto exit;
    }
    *out_payload = NULL;
    *out_payload_size = 0;

    if((rc_config->num_phrases == 0) ||
       (rc_config->num_phrases > phrase_sm->num_phrases)) {
        ALOGE("%s: ERROR. Invalid phrases %d!=%d",__func__,
              rc_config->num_phrases, phrase_sm->num_phrases);
        status = -EINVAL;
        goto exit;
    }
    for (i = 0; i < rc_config->num_phrases; i++) {
        for (j = 0; j < rc_config->phrases[i].num_levels; j++)
            total_num_users++;
    }

    num_conf_levels = total_num_users + rc_config->num_phrases;
    conf_levels = calloc(1, num_conf_levels);
    if (!conf_levels) {
        ALOGE("%s: ERROR. conf levels alloc failed",__func__);
        status = -ENOMEM;
        goto exit;
    }

    status = fill_conf_levels_payload_from_rc_config(phrase_sm, rc_config,
       conf_levels, total_num_users);
    if (status) {
        ALOGE("%s: fill config payload failed, error %d", __func__, status);
        goto exit;
    }

    *out_payload = conf_levels;
    *out_payload_size = num_conf_levels;

    return status;

exit:
    if (conf_levels)
        free(conf_levels);
    return status;
}

int generate_conf_levels_payload_from_rc_config_v2
(
   const struct sound_trigger_phrase_sound_model *phrase_sm,
   const struct sound_trigger_recognition_config *rc_config,
   unsigned char **out_payload,
   unsigned int *out_payload_size
)
{
    int status = 0;
    unsigned int total_num_users = 0, num_conf_levels = 0;
    unsigned char *conf_levels = NULL;
    unsigned int i = 0, j = 0;

    ALOGV("%s: Enter...", __func__);

    if (!phrase_sm || !rc_config || !out_payload || !out_payload_size) {
        ALOGE("%s: ERROR. Invalid inputs",__func__);
        status = -EINVAL;
        goto exit;
    }
    *out_payload = NULL;
    *out_payload_size = 0;

    if((rc_config->num_phrases == 0) ||
       (rc_config->num_phrases > phrase_sm->num_phrases)) {
        ALOGE("%s: ERROR. Invalid phrases %d!=%d",__func__,
              rc_config->num_phrases, phrase_sm->num_phrases);
        status = -EINVAL;
        goto exit;
    }
    for (i = 0; i < rc_config->num_phrases; i++) {
        for (j = 0; j < rc_config->phrases[i].num_levels; j++)
            total_num_users++;
    }

    num_conf_levels = total_num_users + rc_config->num_phrases;
    /*
     * allocate dsp payload w/additional 2 bytes for minor_version and
     * num_active_models and additional num_conf_levels for KW enable
     * fields
     */
    conf_levels = calloc(1, 2 + 2 * num_conf_levels);
    if (!conf_levels) {
        ALOGE("%s: ERROR. conf levels alloc failed",__func__);
        status = -ENOMEM;
        goto exit;
    }

    conf_levels[0] = 1; /* minor version */
    conf_levels[1] = num_conf_levels; /* num_active_models */
    status = fill_conf_levels_payload_from_rc_config(phrase_sm, rc_config,
        conf_levels + 2, total_num_users);
    if (status) {
        ALOGE("%s: fill config payload failed, error %d", __func__, status);
        goto exit;
    }

    /*
     * set KW enable fields to 1 for now
     * TODO: set appropriately based on what client is passing in rc_config
     */
    memset(&conf_levels[num_conf_levels + 2], 0x1, num_conf_levels);
    ALOGV("%s: here", __func__);
    *out_payload = conf_levels;
    /* add size of minor version and num_active_models */
    *out_payload_size = 2 + 2 * num_conf_levels;
    return status;

exit:
    if (conf_levels)
        free(conf_levels);
    return status;
}

static int fill_sound_trigger_recognition_config_payload
(
   const void *sm_levels_generic,
   unsigned char *conf_levels,
   unsigned int total_num_users,
   uint32_t version
)
{
    int status = 0;
    unsigned int user_level = 0, user_id = 0;
    unsigned int i = 0, j = 0;
    unsigned int num_conf_levels = 0;
    unsigned char *user_id_tracker = NULL;
    struct st_sound_model_conf_levels *sm_levels = NULL;
    struct st_sound_model_conf_levels_v2 *sm_levels_v2 = NULL;

    /*  Example: Say the recognition structure has 3 keywords with users
     *  |kid|
     *  [0] k1 |uid|
     *         [3] u1 - 1st trainer
     *         [4] u2 - 4th trainer
     *         [6] u3 - 3rd trainer
     *  [1] k2
     *         [5] u2 - 2nd trainer
     *         [7] u3 - 5th trainer
     *  [2] k3
     *         [8] u4 - 6th trainer
     *
     *  Output confidence level array will be
     *  [k1, k2, k3, u1k1, u2k1, u2k2, u3k1, u3k2, u4k3]
     */

    if (version != CONF_LEVELS_INTF_VERSION_0002) {
        sm_levels = (struct st_sound_model_conf_levels *)sm_levels_generic;
        if (!sm_levels || !conf_levels) {
            ALOGE("%s: ERROR. Invalid inputs", __func__);
            return -EINVAL;
        }

        if ((UINT32_MAX - total_num_users) > sm_levels->num_kw_levels)
            num_conf_levels = total_num_users + sm_levels->num_kw_levels;

        if (!num_conf_levels) {
            ALOGE("%s: ERROR. Invalid num_conf_levels input", __func__);
            return -EINVAL;
        }

        user_id_tracker = calloc(1, num_conf_levels);
        if (!user_id_tracker) {
            ALOGE("%s: failed to allocate user_id_tracker", __func__);
            return -ENOMEM;
        }

        for (i = 0; i < sm_levels->num_kw_levels; i++) {
            ALOGV("%s: [%d] kw level %d", __func__, i,
                sm_levels->kw_levels[i].kw_level);
            for (j = 0; j < sm_levels->kw_levels[i].num_user_levels; j++) {
                ALOGV("%s: [%d] user_id %d level %d ", __func__, i,
                    sm_levels->kw_levels[i].user_levels[j].user_id,
                    sm_levels->kw_levels[i].user_levels[j].level);
            }
        }

        for (i = 0; i < sm_levels->num_kw_levels; i++) {
            if (i < num_conf_levels) {
                conf_levels[i] = sm_levels->kw_levels[i].kw_level;
            } else {
                ALOGE("%s: ERROR. Invalid numver of kw levels", __func__);
                status = -EINVAL;
                goto exit;
            }
            for (j = 0; j < sm_levels->kw_levels[i].num_user_levels; j++) {
                user_level = sm_levels->kw_levels[i].user_levels[j].level;
                user_id = sm_levels->kw_levels[i].user_levels[j].user_id;
                if ((user_id < sm_levels->num_kw_levels) ||
                    (user_id >= num_conf_levels)) {
                    ALOGE("%s: ERROR. Invalid params user id %d>%d",
                        __func__, user_id, total_num_users);
                    status = -EINVAL;
                    goto exit;
                } else {
                    if (user_id_tracker[user_id] == 1) {
                        ALOGE("%s: ERROR. Duplicate user id %d",
                            __func__, user_id);
                        status = -EINVAL;
                        goto exit;
                    }
                    conf_levels[user_id] = (user_level < 100) ?
                                           user_level: 100;
                    user_id_tracker[user_id] = 1;
                    ALOGV("%s: user_conf_levels[%d] = %d", __func__,
                        user_id, conf_levels[user_id]);
                }
            }
        }
    } else {
        sm_levels_v2 =
            (struct st_sound_model_conf_levels_v2 *)sm_levels_generic;
        if (!sm_levels_v2 || !conf_levels) {
            ALOGE("%s: ERROR. Invalid inputs", __func__);
            return -EINVAL;
        }

        if ((UINT32_MAX - total_num_users) > sm_levels_v2->num_kw_levels)
            num_conf_levels = total_num_users + sm_levels_v2->num_kw_levels;

        if (!num_conf_levels) {
            ALOGE("%s: ERROR. Invalid num_conf_levels input", __func__);
            return -EINVAL;
        }

        user_id_tracker = calloc(1, num_conf_levels);
        if (!user_id_tracker) {
            ALOGE("%s: failed to allocate user_id_tracker", __func__);
            return -ENOMEM;
        }

        for (i = 0; i < sm_levels_v2->num_kw_levels; i++) {
            ALOGV("%s: [%d] kw level %d", __func__, i,
                sm_levels_v2->kw_levels[i].kw_level);
            for (j = 0; j < sm_levels_v2->kw_levels[i].num_user_levels; j++) {
                ALOGV("%s: [%d] user_id %d level %d ", __func__, i,
                     sm_levels_v2->kw_levels[i].user_levels[j].user_id,
                     sm_levels_v2->kw_levels[i].user_levels[j].level);
            }
        }

        for (i = 0; i < sm_levels_v2->num_kw_levels; i++) {
            if (i < num_conf_levels) {
                conf_levels[i] = sm_levels_v2->kw_levels[i].kw_level;
            } else {
                ALOGE("%s: ERROR. Invalid numver of kw levels", __func__);
                status = -EINVAL;
                goto exit;
            }
            for (j = 0; j < sm_levels_v2->kw_levels[i].num_user_levels; j++) {
                user_level = sm_levels_v2->kw_levels[i].user_levels[j].level;
                user_id = sm_levels_v2->kw_levels[i].user_levels[j].user_id;
                if ((user_id < sm_levels_v2->num_kw_levels) ||
                    (user_id >= num_conf_levels)) {
                    ALOGE("%s: ERROR. Invalid params user id %d>%d",
                         __func__, user_id, total_num_users);
                    status = -EINVAL;
                    goto exit;
                } else {
                    if (user_id_tracker[user_id] == 1) {
                        ALOGE("%s: ERROR. Duplicate user id %d",
                            __func__, user_id);
                        status = -EINVAL;
                        goto exit;
                    }
                    conf_levels[user_id] = (user_level < 100) ?
                                            user_level: 100;
                    user_id_tracker[user_id] = 1;
                    ALOGV("%s: user_conf_levels[%d] = %d", __func__,
                        user_id, conf_levels[user_id]);
                }
            }
        }
    }

exit:
    free(user_id_tracker);
    return status;
}

static int generate_sound_trigger_recognition_config_payload
(
   const void *sm_levels_generic,
   unsigned char **out_payload,
   unsigned int *out_payload_size,
   uint32_t version
)
{
    int status = 0;
    unsigned int total_num_users = 0, num_conf_levels = 0;
    unsigned char *conf_levels = NULL;
    unsigned int i = 0, j = 0;
    struct st_sound_model_conf_levels *sm_levels = NULL;
    struct st_sound_model_conf_levels_v2 *sm_levels_v2 = NULL;

    ALOGV("%s: Enter...", __func__);

    if (version != CONF_LEVELS_INTF_VERSION_0002) {
        sm_levels = (struct st_sound_model_conf_levels *)sm_levels_generic;
        if (!sm_levels || !out_payload || !out_payload_size) {
            ALOGE("%s: ERROR. Invalid inputs", __func__);
            status = -EINVAL;
            goto exit;
        }
        *out_payload = NULL;
        *out_payload_size = 0;

        if (sm_levels->num_kw_levels == 0) {
            ALOGE("%s: ERROR. No confidence levels present", __func__);
            status = -EINVAL;
            goto exit;
        }
        for (i = 0; i < sm_levels->num_kw_levels; i++) {
            for (j = 0; j < sm_levels->kw_levels[i].num_user_levels; j++)
                total_num_users++;
        }

        num_conf_levels = total_num_users + sm_levels->num_kw_levels;
        conf_levels = calloc(1, num_conf_levels);
        if (!conf_levels) {
            ALOGE("%s: ERROR. conf levels alloc failed", __func__);
            status = -ENOMEM;
            goto exit;
        }
    } else {
        sm_levels_v2 =
            (struct st_sound_model_conf_levels_v2 *)sm_levels_generic;
        if (!sm_levels_v2 || !out_payload || !out_payload_size) {
            ALOGE("%s: ERROR. Invalid inputs", __func__);
            status = -EINVAL;
            goto exit;
        }
        *out_payload = NULL;
        *out_payload_size = 0;

        if (sm_levels_v2->num_kw_levels == 0) {
            ALOGE("%s: ERROR. No confidence levels present", __func__);
            status = -EINVAL;
            goto exit;
        }
        for (i = 0; i < sm_levels_v2->num_kw_levels; i++) {
            for (j = 0; j < sm_levels_v2->kw_levels[i].num_user_levels; j++)
                total_num_users++;
        }

        num_conf_levels = total_num_users + sm_levels_v2->num_kw_levels;
        conf_levels = calloc(1, num_conf_levels);
        if (!conf_levels) {
            ALOGE("%s: ERROR. conf levels alloc failed", __func__);
            status = -ENOMEM;
            goto exit;
        }
    }

    status = fill_sound_trigger_recognition_config_payload(sm_levels_generic,
        conf_levels, total_num_users, version);
    if (status) {
        ALOGE("%s: fill config payload failed, error %d", __func__, status);
        goto exit;
    }

    *out_payload = conf_levels;
    *out_payload_size = num_conf_levels;

    return status;

exit:
    if (conf_levels)
        free(conf_levels);

    return status;
}

static int generate_sound_trigger_recognition_config_payload_v2
(
   const void *sm_levels_generic,
   unsigned char **out_payload,
   unsigned int *out_payload_size,
   uint32_t version
)
{
    int status = 0;
    unsigned int total_num_users = 0, num_conf_levels = 0;
    unsigned char *conf_levels = NULL;
    unsigned int i = 0, j = 0;
    struct st_sound_model_conf_levels *sm_levels = NULL;
    struct st_sound_model_conf_levels_v2 *sm_levels_v2 = NULL;

    ALOGV("%s: Enter...", __func__);

    if (version != CONF_LEVELS_INTF_VERSION_0002) {
        sm_levels = (struct st_sound_model_conf_levels *)sm_levels_generic;
        if (!sm_levels || !out_payload || !out_payload_size) {
            ALOGE("%s: ERROR. Invalid inputs", __func__);
            status = -EINVAL;
            goto exit;
        }
        *out_payload = NULL;
        *out_payload_size = 0;

        if (sm_levels->num_kw_levels == 0) {
            ALOGE("%s: ERROR. No confidence levels present", __func__);
            status = -EINVAL;
            goto exit;
        }
        for (i = 0; i < sm_levels->num_kw_levels; i++) {
            for (j = 0; j < sm_levels->kw_levels[i].num_user_levels; j++)
                total_num_users++;
        }

        num_conf_levels = total_num_users + sm_levels->num_kw_levels;
    } else {
        sm_levels_v2 =
            (struct st_sound_model_conf_levels_v2 *)sm_levels_generic;
        if (!sm_levels_v2 || !out_payload || !out_payload_size) {
            ALOGE("%s: ERROR. Invalid inputs", __func__);
            status = -EINVAL;
            goto exit;
        }
        *out_payload = NULL;
        *out_payload_size = 0;

        if (sm_levels_v2->num_kw_levels == 0) {
            ALOGE("%s: ERROR. No confidence levels present", __func__);
            status = -EINVAL;
            goto exit;
        }
        for (i = 0; i < sm_levels_v2->num_kw_levels; i++) {
            for (j = 0; j < sm_levels_v2->kw_levels[i].num_user_levels; j++)
                total_num_users++;
        }
        num_conf_levels = total_num_users + sm_levels_v2->num_kw_levels;
    }

    /*
     * allocate dsp payload w/additional 2 bytes for minor_version and
     * num_active_models and additional num_conf_levels for KW enable
     * fields
     */
    conf_levels = calloc(1, 2 + 2 * num_conf_levels);
    if (!conf_levels) {
        ALOGE("%s: ERROR. conf levels alloc failed", __func__);
        status = -ENOMEM;
        goto exit;
    }

    conf_levels[0] = 1; /* minor version */
    conf_levels[1] = num_conf_levels; /* num_active_models */
    status = fill_sound_trigger_recognition_config_payload(sm_levels_generic,
        conf_levels + 2, total_num_users, version);
    if (status) {
        ALOGE("%s: fill config payload failed, error %d", __func__, status);
        goto exit;
    }

    /* set KW enable fields to 1 for now
     * TODO set appropriately based on what client is passing in rc_config
     */
    memset(&conf_levels[num_conf_levels + 2], 0x1, num_conf_levels);
    ALOGV("%s: here", __func__);
    *out_payload = conf_levels;
    /* add size of minor version and num_active_models */
    *out_payload_size = 2 + 2 * num_conf_levels;

    return status;

exit:
    if (conf_levels)
        free(conf_levels);

    return status;
}

static int parse_rc_config_key_conf_levels
(
    st_session_t *stc_ses,
    st_hw_session_t *st_hw_ses,
    void *opaque_conf_levels,
    unsigned char **out_conf_levels,
    unsigned int *out_num_conf_levels
)
{
    struct st_confidence_levels_info *conf_levels = NULL;
    struct st_confidence_levels_info_v2 *conf_levels_v2 = NULL;
    struct st_sound_model_conf_levels *sm_levels = NULL;
    struct st_sound_model_conf_levels_v2 *sm_levels_v2 = NULL;
    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    struct listnode *node = NULL;
    st_lsm_ss_config_t *ss_cfg = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;
    int status = 0;
    uint32_t i = 0;
    bool gmm_conf_found = false;
    uint8_t confidence_level = 0;
    int32_t confidence_level_v2 = 0;
    bool arm_second_stage = !list_empty(&stc_ses->second_stage_list);
    bool adsp_second_stage = (st_hw_ses == st_ses->hw_ses_adsp &&
                              !list_empty(&st_hw_ses->lsm_ss_cfg_list));

    if (arm_second_stage || adsp_second_stage) {
        if (stc_ses->rc_config->num_phrases > 1) {
            ALOGE("%s: Multi keyword is unsupported with 2nd stage detection",
                  __func__);
            return -EINVAL;
        }

        if (stc_ses->rc_config->phrases[0].num_levels > 1) {
            ALOGE("%s: Multi user is unsupported with 2nd stage detection",
                  __func__);
            return -EINVAL;
        }
    }

    if (stc_ses->st_conf_levels) {
        free(stc_ses->st_conf_levels);
        stc_ses->st_conf_levels = NULL;
    }

    if (stc_ses->conf_levels_intf_version != CONF_LEVELS_INTF_VERSION_0002) {
        conf_levels = (struct st_confidence_levels_info *)
            ((char *)opaque_conf_levels + sizeof(struct st_param_header));

        stc_ses->st_conf_levels =
            calloc(1, sizeof(struct st_confidence_levels_info));
        if (!stc_ses->st_conf_levels) {
            ALOGE("%s: failed to alloc st_conf_levels", __func__);
            return -ENOMEM;
        }
        /* Cache to use during detection event processing */
        memcpy(stc_ses->st_conf_levels, (char *)conf_levels,
            sizeof(struct st_confidence_levels_info));

        for (i = 0; i < conf_levels->num_sound_models; i++) {
            sm_levels = &conf_levels->conf_levels[i];
            if (sm_levels->sm_id == ST_SM_ID_SVA_GMM) {
                if ((st_ses->stdev->is_gcs) && (st_hw_ses == st_ses->hw_ses_cpe))
                    status =
                        generate_sound_trigger_recognition_config_payload_v2(
                        (void *)sm_levels, out_conf_levels, out_num_conf_levels,
                        stc_ses->conf_levels_intf_version);
                else
                    status =
                        generate_sound_trigger_recognition_config_payload(
                        (void *)sm_levels, out_conf_levels, out_num_conf_levels,
                        stc_ses->conf_levels_intf_version);
                gmm_conf_found = true;
            } else if (IS_SECOND_STAGE_MODEL(sm_levels->sm_id)) {
                confidence_level = IS_KEYWORD_DETECTION_MODEL(sm_levels->sm_id) ?
                    sm_levels->kw_levels[0].kw_level:
                    sm_levels->kw_levels[0].user_levels[0].level;
                if (arm_second_stage) {
                    list_for_each(node, &stc_ses->second_stage_list) {
                        st_sec_stage = node_to_item(node, st_arm_second_stage_t,
                            list_node);
                        if (IS_MATCHING_SS_MODEL(st_sec_stage->ss_info->sm_id,
                                                 sm_levels->sm_id))
                            st_sec_stage->ss_session->confidence_threshold =
                                confidence_level;
                    }
                } else if (adsp_second_stage) {
                    list_for_each(node, &st_hw_ses->lsm_ss_cfg_list) {
                        ss_cfg = node_to_item(node, st_lsm_ss_config_t,
                            list_node);
                        if (IS_MATCHING_SS_MODEL(ss_cfg->ss_info->sm_id,
                                                 sm_levels->sm_id))
                            ss_cfg->confidence_threshold = confidence_level;
                    }
                }
            } else {
                ALOGE("%s: Unsupported sm id (%d), exiting", __func__,
                    sm_levels->sm_id);
                status = -EINVAL;
                break;
            }
        }
    } else {
        conf_levels_v2 = (struct st_confidence_levels_info_v2 *)
            ((char *)opaque_conf_levels + sizeof(struct st_param_header));

        stc_ses->st_conf_levels =
            calloc(1, sizeof(struct st_confidence_levels_info_v2));
        if (!stc_ses->st_conf_levels) {
            ALOGE("%s: failed to alloc st_conf_levels", __func__);
            return -ENOMEM;
        }
                /* Cache to use during detection event processing */
        memcpy(stc_ses->st_conf_levels, (char *)conf_levels_v2,
            sizeof(struct st_confidence_levels_info_v2));

        for (i = 0; i < conf_levels_v2->num_sound_models; i++) {
            sm_levels_v2 = &conf_levels_v2->conf_levels[i];
            if (sm_levels_v2->sm_id == ST_SM_ID_SVA_GMM) {
                if ((st_ses->stdev->is_gcs) &&
                    (st_hw_ses == st_ses->hw_ses_cpe))
                    status =
                        generate_sound_trigger_recognition_config_payload_v2(
                        (void *)sm_levels_v2, out_conf_levels, out_num_conf_levels,
                        stc_ses->conf_levels_intf_version);
                else
                    status =
                        generate_sound_trigger_recognition_config_payload(
                        (void *)sm_levels_v2, out_conf_levels,
                        out_num_conf_levels, stc_ses->conf_levels_intf_version);
                gmm_conf_found = true;
            } else if (IS_SECOND_STAGE_MODEL(sm_levels_v2->sm_id)) {
                confidence_level_v2 =
                    (IS_KEYWORD_DETECTION_MODEL(sm_levels_v2->sm_id)) ?
                    sm_levels_v2->kw_levels[0].kw_level:
                    sm_levels_v2->kw_levels[0].user_levels[0].level;
                if (arm_second_stage) {
                    list_for_each(node, &stc_ses->second_stage_list) {
                        st_sec_stage = node_to_item(node, st_arm_second_stage_t,
                            list_node);
                        if (IS_MATCHING_SS_MODEL(st_sec_stage->ss_info->sm_id,
                                                 sm_levels_v2->sm_id))
                            st_sec_stage->ss_session->confidence_threshold =
                                confidence_level_v2;
                    }
                } else if (adsp_second_stage) {
                    list_for_each(node, &st_hw_ses->lsm_ss_cfg_list) {
                        ss_cfg = node_to_item(node, st_lsm_ss_config_t,
                            list_node);
                        if (IS_MATCHING_SS_MODEL(ss_cfg->ss_info->sm_id,
                                                 sm_levels_v2->sm_id))
                            ss_cfg->confidence_threshold = confidence_level_v2;
                    }
                }
            } else {
                ALOGE("%s: Unsupported sm id (%d), exiting", __func__,
                    sm_levels_v2->sm_id);
                status = -EINVAL;
                break;
            }
        }
    }

    if (!gmm_conf_found) {
        ALOGE("%s: Did not receive GMM confidence threshold, error!", __func__);
        status  = -EINVAL;
    }

    if (status && stc_ses->st_conf_levels) {
        free(stc_ses->st_conf_levels);
        stc_ses->st_conf_levels = NULL;
    }
    return status;
}

static int update_hw_config_on_start(st_session_t *stc_ses,
    st_hw_session_t *st_hw_ses)
{
    struct st_param_header *param_hdr = NULL;
    struct st_hist_buffer_info *hist_buf = NULL;
    struct st_det_perf_mode_info *det_perf_mode = NULL;
    struct sound_trigger_recognition_config *rc_config = stc_ses->rc_config;
    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    struct st_vendor_info *v_info = NULL;
    struct st_hw_ses_config *sthw_cfg = NULL;
    unsigned char *conf_levels = NULL;
    unsigned int num_conf_levels = 0;
    uint8_t *opaque_ptr = NULL;
    unsigned int opaque_size = 0, conf_levels_payload_size = 0;
    int status = 0;
    bool enable_lab = false;

    if (st_ses->stdev->enable_debug_dumps) {
        ST_DBG_DECLARE(FILE *rc_opaque_fd = NULL;
            static int rc_opaque_cnt = 0);
        ST_DBG_FILE_OPEN_WR(rc_opaque_fd, ST_DEBUG_DUMP_LOCATION,
            "rc_config_opaque_data", "bin", rc_opaque_cnt++);
        ST_DBG_FILE_WRITE(rc_opaque_fd,
            (uint8_t *)rc_config + rc_config->data_offset,
            rc_config->data_size);
        ST_DBG_FILE_CLOSE(rc_opaque_fd);
    }

    if (!st_hw_ses) {
        ALOGE("%s: NULL hw session !!!", __func__);
        return -EINVAL;
    }

    v_info = st_hw_ses->vendor_uuid_info;

    if ((rc_config->data_size > CUSTOM_CONFIG_OPAQUE_DATA_SIZE) &&
        v_info->is_qcva_uuid) {
        stc_ses->client_req_det_mode = ST_DET_UNKNOWN_MODE;

        opaque_ptr = (uint8_t *)rc_config + rc_config->data_offset;
        while (opaque_size < rc_config->data_size) {
            param_hdr = (struct st_param_header *)opaque_ptr;
            ALOGV("%s: key %d, payload size %d", __func__,
                  param_hdr->key_id, param_hdr->payload_size);

            switch(param_hdr->key_id) {
            case ST_PARAM_KEY_CONFIDENCE_LEVELS:
                stc_ses->conf_levels_intf_version =
                    *(uint32_t *)(opaque_ptr + sizeof(struct st_param_header));

                if (stc_ses->conf_levels_intf_version !=
                    CONF_LEVELS_INTF_VERSION_0002) {
                    conf_levels_payload_size =
                        sizeof(struct st_confidence_levels_info);
                } else {
                    conf_levels_payload_size =
                        sizeof(struct st_confidence_levels_info_v2);
                }
                if (param_hdr->payload_size != conf_levels_payload_size) {
                    ALOGE("%s: Conf level format error, exiting", __func__);
                    return -EINVAL;
                }
                status = parse_rc_config_key_conf_levels(stc_ses, st_hw_ses,
                    opaque_ptr, &conf_levels, &num_conf_levels);
                opaque_size += sizeof(struct st_param_header) +
                    conf_levels_payload_size;
                opaque_ptr += sizeof(struct st_param_header) +
                    conf_levels_payload_size;
                if (status) {
                    ALOGE("%s: parsing conf levels failed(status=%d)",
                        __func__, status);
                    return -EINVAL;
                }
                break;
            case ST_PARAM_KEY_HISTORY_BUFFER_CONFIG:
                if (param_hdr->payload_size !=
                    sizeof(struct st_hist_buffer_info)) {
                    ALOGE("%s: History buffer config format error, exiting",
                          __func__);
                    return -EINVAL;
                }
                hist_buf = (struct st_hist_buffer_info *)(opaque_ptr +
                    sizeof(struct st_param_header));
                stc_ses->hist_buf_duration =
                    hist_buf->hist_buffer_duration_msec;
                stc_ses->preroll_duration = hist_buf->pre_roll_duration_msec;
                ALOGV("%s: recognition config history buf len = %d, "
                      "preroll len = %d, minor version = %d",
                      __func__, hist_buf->hist_buffer_duration_msec,
                      hist_buf->pre_roll_duration_msec, hist_buf->version);
                opaque_size += sizeof(struct st_param_header) +
                    sizeof(struct st_hist_buffer_info);
                opaque_ptr += sizeof(struct st_param_header) +
                    sizeof(struct st_hist_buffer_info);
                break;
            case ST_PARAM_KEY_DETECTION_PERF_MODE:
                if (param_hdr->payload_size !=
                    sizeof(struct st_det_perf_mode_info)) {
                    ALOGE("%s: Opaque data format error, exiting", __func__);
                    return -EINVAL;
                }
                det_perf_mode = (struct st_det_perf_mode_info *)(opaque_ptr +
                    sizeof(struct st_param_header));
                ALOGV("set perf mode to %d", det_perf_mode->mode);
                stc_ses->client_req_det_mode = det_perf_mode->mode;
                opaque_size += sizeof(struct st_param_header) +
                    sizeof(struct st_det_perf_mode_info);
                opaque_ptr += sizeof(struct st_param_header) +
                    sizeof(struct st_det_perf_mode_info);
                break;
            default:
                ALOGE("%s: Unsupported opaque data key id, exiting", __func__);
                return -EINVAL;
            }
        }
    } else if (stc_ses->sm_type == SOUND_MODEL_TYPE_KEYPHRASE) {
        struct sound_trigger_phrase_sound_model *phrase_sm = stc_ses->phrase_sm;

        ALOGV("%s: num_phrases=%d, id=%d", __func__,
               rc_config->num_phrases, rc_config->phrases[0].id);

        if (st_ses->vendor_uuid_info->is_qcva_uuid ||
            st_ses->vendor_uuid_info->is_qcmd_uuid) {
            if (st_ses->stdev->is_gcs && st_hw_ses == st_ses->hw_ses_cpe)
                status = generate_conf_levels_payload_from_rc_config_v2(
                    phrase_sm, rc_config, &conf_levels, &num_conf_levels);
            else
                status = generate_conf_levels_payload_from_rc_config(
                    phrase_sm, rc_config, &conf_levels, &num_conf_levels);
            if (status || !conf_levels) {
                ALOGE("%s: failed to get conf levels from lib handle",
                      __func__);
                return status;
            }
        } else {
            ALOGD("%s: No smlib, opaque data would be sent as is", __func__);
        }
    }

    sthw_cfg = &st_hw_ses->sthw_cfg;
    enable_lab = stc_ses->rc_config->capture_requested ||
                 !list_empty(&stc_ses->second_stage_list);

    if (v_info->merge_fs_soundmodels) {
        /* merge_fs_soundmodels is true only for QC SVA UUID */

         /*
          * Note:
          * For ADSP case, the generated conf levles size must be equal to
          * SML queried conf levels.
          * For WDSP gcs case, there is additional payload for KW enable
          * fields in generated conf_levels. If merge sound model is supported
          * on WDSP case, update logic here accordingly.
          */
        if (num_conf_levels != stc_ses->sm_info.cf_levels_size) {
            ALOGE("%s: Unexpected, client cf levels %d != sm_info cf levels %d",
                __func__, num_conf_levels, stc_ses->sm_info.cf_levels_size);
            return -EINVAL;
        }

        /*
         * If any of the active clients requested capture or enabled the
         * second stage, the underlying hw session buffering needs to
         * be enabled. Ignore if it is already enabled.
         */
        if (!st_ses->lab_enabled && enable_lab)
            st_ses->lab_enabled = true;

        /* Aggregate DSP configuration for highest client configuration */

        /* SVA2.0 sound models */
        if (!stc_ses->hist_buf_duration &&
            stc_ses->rc_config->capture_requested &&
            (stc_ses->rc_config->data_size > 0)) {
            stc_ses->hist_buf_duration = st_ses->vendor_uuid_info->kw_duration;
            stc_ses->preroll_duration = 0;
        }

        if (stc_ses->hist_buf_duration > sthw_cfg->client_req_hist_buf)
            sthw_cfg->client_req_hist_buf = stc_ses->hist_buf_duration;
        if (stc_ses->preroll_duration > sthw_cfg->client_req_preroll)
            sthw_cfg->client_req_preroll = stc_ses->preroll_duration;

        ALOGV("%s: client hb_sz %d pr_sz %d, sthw lab %d hb_sz %d "
              "pr_sz %d", __func__, stc_ses->hist_buf_duration,
              stc_ses->preroll_duration, st_ses->lab_enabled,
              sthw_cfg->client_req_hist_buf, sthw_cfg->client_req_preroll);

        /* Cache it to use when client restarts without config update or
         * during only one remaining client model as there won't be a
         * merged model yet.
         */
        memcpy(stc_ses->sm_info.cf_levels, conf_levels,
               stc_ses->sm_info.cf_levels_size);

        status = update_merge_conf_levels_payload(st_ses, &stc_ses->sm_info,
            conf_levels, num_conf_levels, true);
        free(conf_levels); /* Merged model conf levels will be used further */
        if (status)
            return status;

        sthw_cfg->conf_levels = st_ses->sm_info.cf_levels;
        sthw_cfg->num_conf_levels = st_ses->sm_info.cf_levels_size;
        st_hw_ses->sthw_cfg_updated = true;

        /*
         * Merging further unknown custom data is not needed, as
         * SVA doesn't support unkown custom data. if required in future,
         * handle here.
         * For now just copy the the current client data which is same
         * across SVA engines.
         * Update the custom data for the case in which one client session
         * does not have custom data and another one does.
         */
        if (rc_config->data_size > sthw_cfg->custom_data_size) {
            sthw_cfg->custom_data = (char *)rc_config + rc_config->data_offset;
            sthw_cfg->custom_data_size =  rc_config->data_size;
        }

    } else {
        st_ses->recognition_mode = stc_ses->recognition_mode;
        st_ses->lab_enabled = enable_lab;

        sthw_cfg->client_req_hist_buf = stc_ses->hist_buf_duration;
        sthw_cfg->client_req_preroll = stc_ses->preroll_duration;

        if (sthw_cfg->conf_levels)
            free(sthw_cfg->conf_levels);
        sthw_cfg->conf_levels = conf_levels;
        sthw_cfg->num_conf_levels = num_conf_levels;

        sthw_cfg->custom_data = (char *)rc_config + rc_config->data_offset;
        sthw_cfg->custom_data_size =  rc_config->data_size;
    }
    ALOGD("%s:[%d] lab enabled %d", __func__, st_ses->sm_handle,
          st_ses->lab_enabled);

    return status;
}

static void do_hw_sess_cleanup(st_proxy_session_t *st_ses,
    st_hw_session_t *hw_ses, enum hw_session_err_mask err)
{
    if (err & HW_SES_ERR_MASK_BUFFERING)
        hw_ses->fptrs->stop_buffering(hw_ses);

    if (err & HW_SES_ERR_MASK_STARTED) {
        hw_ses->fptrs->stop(hw_ses);
        st_ses->hw_session_started = false;
    }

    if (err & HW_SES_ERR_MASK_REG_SM_PARAM)
        hw_ses->fptrs->dereg_sm_params(hw_ses);

    if (err & HW_SES_ERR_MASK_DEVICE_SET)
        hw_ses->fptrs->set_device(hw_ses, false);

    if (err & HW_SES_ERR_MASK_REG_SM)
        hw_ses->fptrs->dereg_sm(hw_ses);
}

static void reg_hal_event_session(st_session_t *stc_ses,
    st_hw_session_t *hw_ses)
{
    struct sound_trigger_event_info event_info;
    /* Pass the pcm information to audio hal for capturing LAB */
    if ((stc_ses->rc_config &&
         stc_ses->rc_config->capture_requested) &&
        stc_ses->stdev->audio_hal_cb) {
        ALOGD("%s:[c%d] ST_EVENT_SESSION_REGISTER capture_handle %d",
           __func__, stc_ses->sm_handle, stc_ses->capture_handle);
        event_info.st_ses.p_ses = (void *)stc_ses;
        event_info.st_ses.config = hw_ses->config;
        event_info.st_ses.capture_handle = stc_ses->capture_handle;
        /*
         * set pcm to NULL as this version of st_hal doesn't pass pcm to
         * audio HAL
         */
        event_info.st_ses.pcm = NULL;
        stc_ses->stdev->audio_hal_cb(ST_EVENT_SESSION_REGISTER, &event_info);
    }
}

static void dereg_hal_event_session(st_session_t *stc_ses)
{
    struct sound_trigger_event_info event_info;
    /* Indicate to audio hal that to stop reading LAB data */
    if ((stc_ses->rc_config &&
         stc_ses->rc_config->capture_requested) &&
        stc_ses->stdev->audio_hal_cb) {
        ALOGD("%s:[c%d] ST_EVENT_SESSION_DEREGISTER capture_handle %d",
           __func__, stc_ses->sm_handle, stc_ses->capture_handle);
        event_info.st_ses.p_ses = (void *)stc_ses;
        event_info.st_ses.capture_handle = stc_ses->capture_handle;
        event_info.st_ses.pcm = NULL;
        stc_ses->stdev->audio_hal_cb(ST_EVENT_SESSION_DEREGISTER, &event_info);
    }
}

static bool check_gcs_usecase_switch
(
    st_proxy_session_t *st_ses
)
{
    struct st_vendor_info *v_info = NULL;
    st_hw_session_t *p_ses = NULL;
    st_hw_session_gcs_t *p_gcs_ses = NULL;
    int st_device = 0;
    unsigned int device_acdb_id = 0;
    int capture_device;

    if (!st_ses || st_ses->exec_mode != ST_EXEC_MODE_CPE) {
        ALOGE("%s: Invalid session or non CPE session!", __func__);
        return false;
    }

    p_ses = st_ses->hw_ses_cpe;
    p_gcs_ses = (st_hw_session_gcs_t *)p_ses;
    v_info = p_ses->vendor_uuid_info;

    if (list_empty(&v_info->gcs_usecase_list)) {
        ALOGE("%s: gcs usecase not available", __func__);
        return false;
    }

    /* check if need to switch gcs usecase for new capture device */
    capture_device = platform_stdev_get_capture_device(p_ses->stdev->platform);
    st_device = platform_stdev_get_device(p_ses->stdev->platform,
        v_info, capture_device, p_ses->exec_mode);
    device_acdb_id = platform_stdev_get_acdb_id(st_device,
        p_ses->exec_mode);
    if (platform_stdev_get_xml_version(p_ses->stdev->platform) >=
        PLATFORM_XML_VERSION_0x0102) {
        int i = 0;
        while ((i < MAX_GCS_USECASE_ACDB_IDS) &&
                p_gcs_ses->gcs_usecase->acdb_ids[i]) {
            if (p_gcs_ses->gcs_usecase->acdb_ids[i] == device_acdb_id)
                return false;
            i++;
        }
        ALOGD("%s: gcs usecase doesn't match for new device", __func__);
        return true;
    } else {
        return false;
    }
}

static int start_hw_session(st_proxy_session_t *st_ses, st_hw_session_t *hw_ses,
    bool load_sm)
{
    int status = 0, err = 0;
    bool do_unload = false;

    /*
     * It is possible the BE LPI mode has been updated, but not the FE mode.
     * DSP requires both FE and BE to be in the same mode for any configuration
     * changes between LPI and non-LPI switch, so update the FE mode to the
     * same as BE mode by re-opening LSM session. This is also used for
     * other transition usecases which require dereg_sm and reg_sm.
     */
    if (hw_ses->lpi_enable != hw_ses->stdev->lpi_enable ||
        (hw_ses->barge_in_mode != hw_ses->stdev->barge_in_mode &&
         !hw_ses->stdev->support_dynamic_ec_update)) {
        hw_ses->lpi_enable = hw_ses->stdev->lpi_enable;
        hw_ses->barge_in_mode = hw_ses->stdev->barge_in_mode;
        do_unload = true;
    }

    /*
     * For gcs sessions, uid may be changed for new capture device,
     * in this case, sm must be dereg and reg again.
     */
    if (check_gcs_usecase_switch(st_ses))
        do_unload = true;

    if (do_unload) {
        if (!load_sm) {
            load_sm = true;
            status = hw_ses->fptrs->dereg_sm(hw_ses);
            if (status)
                ALOGW("%s:[%d] failed to dereg_sm err %d", __func__,
                    st_ses->sm_handle, status);
        }
    }

    if (load_sm) {
        status = hw_ses->fptrs->reg_sm(hw_ses, st_ses->sm_info.sm_data,
            st_ses->sm_info.sm_size, st_ses->sm_info.sm_type);
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

    status = hw_ses->fptrs->reg_sm_params(hw_ses, st_ses->recognition_mode,
        st_ses->lab_enabled, st_ses->rc_config, st_ses->sm_info.sm_type,
        st_ses->sm_info.sm_data);
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

static int stop_hw_session(st_proxy_session_t *st_ses, st_hw_session_t *hw_ses,
    bool unload_sm)
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
        status = hw_ses->fptrs->dereg_sm(hw_ses);
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

static int start_session(st_proxy_session_t *st_ses, st_hw_session_t *hw_ses,
    bool load_sm)
{
    int status = 0;

    if (st_ses->hw_session_started) {
        ALOGE("%s:[%d] already started", __func__, st_ses->sm_handle);
        return -1;
    }

    status = start_hw_session(st_ses, hw_ses, load_sm);
    hw_ses->sthw_cfg_updated = false;

    return status;
}

static int restart_session(st_proxy_session_t *st_ses, st_hw_session_t *hw_ses)
{
    int status = 0;

    status = hw_ses->fptrs->restart(hw_ses, st_ses->recognition_mode,
            st_ses->rc_config, st_ses->sm_info.sm_type,
            st_ses->sm_info.sm_data);
    if (status == 0) {
        st_ses->hw_session_started = true;
    } else {
        ALOGE("%s:[%d] failed to restart", __func__, st_ses->sm_handle);
        st_ses->hw_session_started = false;
    }

    return status;
}

static int stop_session(st_proxy_session_t *st_ses,
    st_hw_session_t *hw_ses, bool unload_sm)
{
    if (!st_ses->hw_session_started) {
        ALOGV("%s:[%d] already stopped", __func__, st_ses->sm_handle);
        return 0;
    }
    st_ses->detection_requested = false;
    return stop_hw_session(st_ses, hw_ses, unload_sm);
}

/*
 * This function gets the first stage detection keyword indices, which are
 * needed by the second stage sessions. If the legacy DSP is used, which does
 * not provide keyword indices, set the indices to include the entire keyword
 * duration. This function also gets the user confidence level if there is an
 * active voiceprint session.
 */
static int get_first_stage_detection_params(st_proxy_session_t *st_ses,
    void *payload, size_t payload_size)
{
    size_t count_size = 0;
    uint8_t *payload_ptr = (uint8_t *)payload;
    uint8_t *cf_levels = NULL;
    uint32_t key_id = 0, key_payload_size = 0, cf_levels_size = 0;
    uint32_t kw_start_ms = 0, kw_end_ms = 0;
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    struct listnode *node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;
    st_session_t *stc_ses = st_ses->det_stc_ses;
    bool is_active_vop_session = false;

    list_for_each(node, &stc_ses->second_stage_list) {
        st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
        if (st_sec_stage->ss_info->sm_detection_type ==
            ST_SM_TYPE_USER_VERIFICATION) {
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
                    /*
                     * It is expected that VoP is supported with single KW/user
                     * SVA3.0 model, hence get it directly with hard offset.
                     */
                    if (!st_ses->sm_info.sm_merged) {
                        hw_ses->user_level = (int32_t)(*(payload_ptr +
                            GENERIC_DET_EVENT_USER_LEVEL_OFFSET));
                    } else {
                        /* Extract from first stage merged conf levels */
                        check_and_extract_det_conf_levels_payload(st_ses,
                            payload_ptr + (4 * sizeof(uint32_t)),
                            *((uint32_t *)payload_ptr + 3),
                            &cf_levels, &cf_levels_size);
                        if (!cf_levels || !cf_levels_size)
                            break;
                        hw_ses->user_level = cf_levels[1];
                        ALOGV("%s:hw_ses->user_level %d at cf_levels[1]",
                              __func__, hw_ses->user_level);
                    }
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
        if (hw_ses->sthw_cfg.client_req_hist_buf) {
            hw_ses->kw_end_idx =
                convert_ms_to_bytes(hw_ses->sthw_cfg.client_req_hist_buf,
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
    ALOGD("%s:[%d] 1st stage kw_start = %dms, kw_end = %dms,"
          "is_generic_event %d", __func__, st_ses->sm_handle,
          kw_start_ms, kw_end_ms, hw_ses->is_generic_event);

    return 0;
}

static inline int prepare_second_stage_for_client(st_session_t *stc_ses)
{
    struct listnode *node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;
    int status = 0;

    ALOGV("%s:[c%d]", __func__, stc_ses->sm_handle);

    list_for_each(node, &stc_ses->second_stage_list) {
        st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
        status = st_second_stage_prepare_session(st_sec_stage);
    }
    return status;
}

static inline int start_second_stage_for_client(st_session_t *stc_ses)
{
    struct listnode *node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;
    int status = 0;

    ALOGV("%s:[c%d]", __func__, stc_ses->sm_handle);

    list_for_each(node, &stc_ses->second_stage_list) {
        st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
        status = st_second_stage_start_session(st_sec_stage);
    }
    return status;
}

static inline void stop_second_stage_for_client(st_session_t *stc_ses)
{
    struct listnode *node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;

    ALOGV("%s:[c%d]", __func__, stc_ses->sm_handle);

    list_for_each(node, &stc_ses->second_stage_list) {
        st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
        st_second_stage_stop_session(st_sec_stage);
    }
}

static int generate_legacy_st_phrase_recognition_event
(
    const struct sound_trigger_phrase_sound_model *phrase_sm,
    const struct sound_trigger_recognition_config *rc_config,
    const void *payload,
    unsigned int payload_size,
    struct sound_trigger_phrase_recognition_event **out_rc_event
)
{
    struct sound_trigger_phrase_recognition_event *event;
    unsigned int i = 0, j = 0, user_id = 0;

    ALOGD("%s: Enter payload_size %d", __func__, payload_size);

    if(!payload || !phrase_sm || !rc_config || !out_rc_event) {
        ALOGE("%s: Null params", __func__);
        return -EINVAL;
    }

    *out_rc_event = NULL;
    event = calloc(1, sizeof(*event) + payload_size);
    if (!event) {
        ALOGE("%s: event allocation failed size %d", __func__, payload_size);
        return -ENODEV;
    }

    event->num_phrases = rc_config->num_phrases;
    event->common.data_offset = sizeof(*event);
    event->common.data_size = payload_size;
    memcpy((char *)event + event->common.data_offset, payload, payload_size);

    /* fill confidence levels */
    for (i = 0; i < rc_config->num_phrases; i++) {
        event->phrase_extras[i].id = rc_config->phrases[i].id;
        event->phrase_extras[i].recognition_modes =
                                 phrase_sm->phrases[0].recognition_mode;
        event->phrase_extras[i].confidence_level = ((char *)payload)[i];
        event->phrase_extras[i].num_levels =  rc_config->phrases[i].num_levels;
        for (j = 0; j < rc_config->phrases[i].num_levels; j++) {
            user_id = rc_config->phrases[i].levels[j].user_id;
            event->phrase_extras[i].levels[j].user_id = user_id;
            event->phrase_extras[i].levels[j].level =
                                             ((char *)payload)[user_id];
        }
    }

    *out_rc_event = event;
    return 0;
}

/*
 * This function sets the opaque data size for the DSP's generic detection
 * events. This opaque data can now have varying size based on the requested
 * params.
 */
static size_t set_opaque_data_size(char *payload, size_t payload_size,
    uint32_t version)
{
    size_t count_size = 0, opaque_size = 0;
    uint32_t key_id = 0, key_payload_size = 0;

    while (count_size < payload_size) {
        key_id = *(uint32_t *)payload;
        key_payload_size = *((uint32_t *)payload + 1);

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
            payload += count_size;
            break;

        case KEY_ID_KEYWORD_POSITION_STATS:
            opaque_size += sizeof(struct st_param_header) +
                sizeof(struct st_keyword_indices_info);
            count_size += GENERIC_DET_EVENT_HEADER_SIZE + key_payload_size;
            payload += count_size;
            break;

        default:
            ALOGE("%s: Unsupported generic detection event key id", __func__);
        }
    }

    opaque_size += sizeof(struct st_param_header) +
        sizeof(struct st_timestamp_info);

    return opaque_size;
}

/*
 * This function packs the updated opaque data confidence levels which are
 * passed to the client via callback.
 */
static int pack_opaque_data_conf_levels(
    st_proxy_session_t *st_ses, void *opaque_data,
    uint8_t *payload,
    unsigned int payload_size)
{
    uint8_t *payload_ptr = payload;
    unsigned int i = 0, j = 0, k = 0, user_id = 0;
    st_arm_second_stage_t *st_sec_stage = NULL;
    struct listnode *node = NULL;
    struct st_confidence_levels_info *conf_levels = NULL;
    struct st_confidence_levels_info_v2 *conf_levels_v2 = NULL;
    st_session_t *stc_ses = st_ses->det_stc_ses;
    int32_t kw_level = 0, user_level = 0;

    list_for_each(node, &stc_ses->second_stage_list) {
        st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
        if (IS_KEYWORD_DETECTION_MODEL(st_sec_stage->ss_info->sm_id)) {
            kw_level = st_sec_stage->ss_session->confidence_score;
        } else if (IS_USER_VERIFICATION_MODEL(st_sec_stage->ss_info->sm_id)) {
            user_level = st_sec_stage->ss_session->confidence_score;
        }
    }

    if (stc_ses->conf_levels_intf_version != CONF_LEVELS_INTF_VERSION_0002) {
        conf_levels = (struct st_confidence_levels_info *)opaque_data;
        for (i = 0; i < conf_levels->num_sound_models; i++) {
            if (conf_levels->conf_levels[i].sm_id == ST_SM_ID_SVA_GMM) {
                for (j = 0;
                     j < conf_levels->conf_levels[i].num_kw_levels; j++) {
                    if (j <= payload_size)
                            conf_levels->conf_levels[i].kw_levels[j].kw_level =
                                    payload_ptr[j];
                    else
                        ALOGE("%s: unexpected conf size %d < %d", __func__,
                            payload_size, j);
                    for (k = 0;
                         k < conf_levels->conf_levels[i].kw_levels[j].num_user_levels;
                         k++) {
                        user_id =
                            conf_levels->conf_levels[i].kw_levels[j].
                                user_levels[k].user_id;
                        if (user_id <= payload_size)
                            conf_levels->conf_levels[i].kw_levels[j].
                                user_levels[k].level = payload_ptr[user_id];
                        else
                            ALOGE("%s: Unexpected conf size %d < %d", __func__,
                                payload_size, user_id);
                    }
                }
            } else if (IS_KEYWORD_DETECTION_MODEL(conf_levels->conf_levels[i].sm_id)) {
                conf_levels->conf_levels[i].kw_levels[0].kw_level = kw_level;
            } else if (IS_USER_VERIFICATION_MODEL(conf_levels->conf_levels[i].sm_id)) {
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
                    if (j <= payload_size)
                        conf_levels_v2->conf_levels[i].kw_levels[j].kw_level =
                            payload_ptr[j];
                    else
                        ALOGE("%s: unexpected conf size %d < %d", __func__,
                              payload_size, j);

                    for (k = 0;
                         k < conf_levels_v2->conf_levels[i].kw_levels[j].num_user_levels;
                         k++) {
                        user_id =
                            conf_levels_v2->conf_levels[i].kw_levels[j].
                                user_levels[k].user_id;
                        if (user_id <= payload_size)
                            conf_levels_v2->conf_levels[i].kw_levels[j].
                                    user_levels[k].level = payload_ptr[user_id];
                        else
                            ALOGE("%s: Unexpected conf size %d < %d", __func__,
                                payload_size, user_id);
                    }
                }
            } else if (IS_KEYWORD_DETECTION_MODEL(conf_levels_v2->conf_levels[i].sm_id)) {
                conf_levels_v2->conf_levels[i].kw_levels[0].kw_level = kw_level;
            } else if (IS_USER_VERIFICATION_MODEL(conf_levels_v2->conf_levels[i].sm_id)) {
                /*
                 * Fill both the keyword and user confidence level with the
                 * confidence score returned from the voiceprint algorithm.
                 */
                conf_levels_v2->conf_levels[i].kw_levels[0].kw_level =
                    user_level;
                conf_levels_v2->conf_levels[i].kw_levels[0].user_levels[0].level =
                    user_level;
            }
        }
    }

    return 0;
}

/* This function packs the sound trigger API confidence levels */
static int pack_recognition_event_conf_levels(
    st_proxy_session_t *st_ses, uint8_t *payload,
    unsigned int payload_size,
    struct sound_trigger_phrase_recognition_event *local_event)
{
    unsigned int j = 0, k = 0, user_id = 0;
    st_arm_second_stage_t *st_sec_stage = NULL;
    struct listnode *node = NULL;
    st_session_t *stc_ses = st_ses->det_stc_ses;
    struct sound_trigger_phrase_sound_model *phrase_sm =
        (struct sound_trigger_phrase_sound_model *)stc_ses->phrase_sm;

    /*
     * Fill in the GMM confidence levels to the sound trigger recognition event
     * APIs first. If any second stage session is enabled, overwrite the APIs
     * with the second stage confidence levels.
     */
    for (j = 0; j < stc_ses->rc_config->num_phrases; j++) {
        local_event->phrase_extras[j].id = stc_ses->rc_config->phrases[j].id;
        local_event->phrase_extras[j].recognition_modes =
            phrase_sm->phrases[j].recognition_mode;
        local_event->phrase_extras[j].num_levels =
            stc_ses->rc_config->phrases[j].num_levels;
                if (j <= payload_size)
                    local_event->phrase_extras[j].confidence_level = payload[j];
                else
                    ALOGE("%s: unexpected conf size %d < %d", __func__,
                        payload_size, j);

        for (k = 0; k < stc_ses->rc_config->phrases[j].num_levels; k++) {
            user_id = stc_ses->rc_config->phrases[j].levels[k].user_id;
            if (user_id <= payload_size) {
                local_event->phrase_extras[j].levels[k].user_id = user_id;
                local_event->phrase_extras[j].levels[k].level =
                    payload[user_id];
            } else {
                ALOGE("%s: Unexpected conf size %d < %d", __func__,
                    payload_size, user_id);
            }
        }
    }

    list_for_each(node, &stc_ses->second_stage_list) {
        st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
        if (IS_KEYWORD_DETECTION_MODEL(st_sec_stage->ss_info->sm_id)) {
            local_event->phrase_extras[0].confidence_level =
                (uint8_t)st_sec_stage->ss_session->confidence_score;
        } else if (IS_USER_VERIFICATION_MODEL(st_sec_stage->ss_info->sm_id)) {
            local_event->phrase_extras[0].levels[0].level =
                (uint8_t)st_sec_stage->ss_session->confidence_score;
        }
    }
    return 0;
}

static int parse_generic_event_and_pack_opaque_data(
    st_proxy_session_t *st_ses, uint8_t *opaque_data,
    uint8_t *payload, size_t payload_size,
    struct sound_trigger_phrase_recognition_event *local_event)
{
    uint32_t key_id = 0, key_payload_size = 0;
    struct st_param_header *param_hdr = NULL;
    struct st_keyword_indices_info *kw_indices = NULL;
    struct st_timestamp_info *timestamps = NULL;
    size_t count_size = 0;
    st_arm_second_stage_t *st_sec_stage = NULL;
    struct listnode *node = NULL;
    st_session_t *stc_ses = st_ses->det_stc_ses;
    int status = 0;
    unsigned char *cf_levels = NULL;
    unsigned int cf_levels_size = 0;

    while (count_size < payload_size) {
        key_id = *(uint32_t *)payload;
        key_payload_size = *((uint32_t *)payload + 1);

        switch (key_id) {
        case KEY_ID_CONFIDENCE_LEVELS:
            /* Pack the opaque data confidence levels structure */
            param_hdr = (struct st_param_header *)(opaque_data);
            param_hdr->key_id = ST_PARAM_KEY_CONFIDENCE_LEVELS;
            opaque_data += sizeof(struct st_param_header);
            if (stc_ses->conf_levels_intf_version !=
                CONF_LEVELS_INTF_VERSION_0002) {
                param_hdr->payload_size =
                    sizeof(struct st_confidence_levels_info);
            } else {
                param_hdr->payload_size =
                    sizeof(struct st_confidence_levels_info_v2);
            }
            check_and_extract_det_conf_levels_payload(st_ses,
                payload + (4 * sizeof(uint32_t)), *((uint32_t *)payload + 3),
                &cf_levels, &cf_levels_size);
            if (!cf_levels || !cf_levels_size) {
                status = -EINVAL;
                goto exit;
            }
            memcpy(opaque_data, stc_ses->st_conf_levels,
                param_hdr->payload_size);
            pack_opaque_data_conf_levels(st_ses, opaque_data,
                cf_levels, cf_levels_size);
            pack_recognition_event_conf_levels(st_ses, cf_levels,
                cf_levels_size, local_event);
            opaque_data += param_hdr->payload_size;
            break;

        case KEY_ID_KEYWORD_POSITION_STATS:
            /* Pack the opaque data keyword indices structure */
            param_hdr = (struct st_param_header *)(opaque_data);
            param_hdr->key_id = ST_PARAM_KEY_KEYWORD_INDICES;
            param_hdr->payload_size = sizeof(struct st_keyword_indices_info);
            opaque_data += sizeof(struct st_param_header);
            kw_indices = (struct st_keyword_indices_info *)(opaque_data);
            kw_indices->version = 0x1;
            kw_indices->start_index = *((uint32_t *)payload + 3);
            kw_indices->end_index = *((uint32_t *)payload + 4);

            list_for_each(node, &stc_ses->second_stage_list) {
                st_sec_stage = node_to_item(node, st_arm_second_stage_t,
                                            list_node);
                if (IS_KEYWORD_DETECTION_MODEL(st_sec_stage->ss_info->sm_id)) {
                    kw_indices->start_index =
                        st_sec_stage->ss_session->kw_start_idx;
                    kw_indices->end_index =
                        st_sec_stage->ss_session->kw_end_idx;
                }
            }
            opaque_data += sizeof(struct st_keyword_indices_info);
            break;

        default:
            ALOGE("%s: Unsupported generic detection event key id", __func__);
            status = -EINVAL;
            goto exit;
        }
        count_size += GENERIC_DET_EVENT_HEADER_SIZE + key_payload_size;
        payload += count_size;
    }

    /* Pack the opaque data detection timestamp structure */
    param_hdr = (struct st_param_header *)(opaque_data);
    param_hdr->key_id = ST_PARAM_KEY_TIMESTAMP;
    param_hdr->payload_size = sizeof(struct st_timestamp_info);
    opaque_data += sizeof(struct st_param_header);
    timestamps = (struct st_timestamp_info *)(opaque_data);
    timestamps->version = 0x1;
    timestamps->first_stage_det_event_time =
    st_ses->hw_ses_current->first_stage_det_event_time;
    if (!list_empty(&stc_ses->second_stage_list))
        timestamps->second_stage_det_event_time =
            st_ses->hw_ses_current->second_stage_det_event_time;
    opaque_data += sizeof(struct st_timestamp_info);

exit:
    return status;
}

static int parse_generic_event_without_opaque_data(
    st_proxy_session_t *st_ses, uint8_t *payload, size_t payload_size,
    struct sound_trigger_phrase_recognition_event *local_event)
{
    uint32_t key_id = 0, key_payload_size = 0;
    size_t count_size = 0;
    int status = 0;
    unsigned char *cf_levels = NULL;
    unsigned int cf_levels_size = 0;;

    while (count_size < payload_size) {
        key_id = *(uint32_t *)payload;
        key_payload_size = *((uint32_t *)payload + 1);

        switch (key_id) {
        case KEY_ID_CONFIDENCE_LEVELS:
            check_and_extract_det_conf_levels_payload(st_ses,
                payload + (4 * sizeof(uint32_t)), *((uint32_t *)payload + 3),
                &cf_levels, &cf_levels_size);
            if (!cf_levels || !cf_levels_size) {
                status = -EINVAL;
                return status;
            }
            pack_recognition_event_conf_levels(st_ses, cf_levels,
                cf_levels_size, local_event);
            return status;

        case KEY_ID_KEYWORD_POSITION_STATS:
            count_size += GENERIC_DET_EVENT_HEADER_SIZE + key_payload_size;
            payload += count_size;
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
    st_proxy_session_t *st_ses, int detect_status,
    void *payload, size_t payload_size,
    struct sound_trigger_phrase_recognition_event **event)
{
    st_hw_session_t *st_hw_ses = st_ses->hw_ses_current;
    st_session_t *stc_ses = st_ses->det_stc_ses;
    unsigned int i = 0, j = 0;
    int status = 0;
    uint8_t *opaque_data = NULL;
    size_t opaque_size = 0;
    struct sound_trigger_phrase_recognition_event *local_event = NULL;

    if (st_ses->vendor_uuid_info->is_qcva_uuid)
        opaque_size = set_opaque_data_size(payload, payload_size,
            stc_ses->conf_levels_intf_version);
    else
        opaque_size = payload_size;

    local_event = calloc(1,
        sizeof(struct sound_trigger_phrase_recognition_event) + opaque_size);
    if (!local_event) {
        ALOGE("%s: local_event allocation failed, opaque data size = %d",
              __func__, (unsigned int)opaque_size);
        return -ENOMEM;
    }

    local_event->num_phrases = stc_ses->rc_config->num_phrases;
    local_event->common.data_offset =
        sizeof(struct sound_trigger_phrase_recognition_event);
    local_event->common.data_size = opaque_size;
    opaque_data = (uint8_t *)local_event + local_event->common.data_offset;

    if (st_ses->vendor_uuid_info->is_qcva_uuid) {
        if (stc_ses->rc_config->data_size > CUSTOM_CONFIG_OPAQUE_DATA_SIZE) {
            status = parse_generic_event_and_pack_opaque_data(st_ses,
                opaque_data, payload, payload_size, local_event);
            if (status) {
                ALOGE("%s: Failed to parse generic detection event with opaque"
                      "data %d", __func__, status);
                goto exit;
            }

            if (st_ses->stdev->enable_debug_dumps) {
                ST_DBG_DECLARE(FILE *opaque_fd = NULL;
                    static int opaque_cnt = 0);
                ST_DBG_FILE_OPEN_WR(opaque_fd, ST_DEBUG_DUMP_LOCATION,
                    "detection_opaque_data", "bin", opaque_cnt++);
                ST_DBG_FILE_WRITE(opaque_fd, opaque_data, opaque_size);
                ST_DBG_FILE_CLOSE(opaque_fd);
            }
        } else {
            status = parse_generic_event_without_opaque_data(st_ses, payload,
                payload_size, local_event);
            if (status) {
                ALOGE("%s: Failed to parse generic detection event without"
                      "opaque data %d", __func__, status);
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
            stc_ses->rc_config->phrases, stc_ses->rc_config->num_phrases *
            sizeof(struct sound_trigger_phrase_recognition_extra));
        local_event->num_phrases = stc_ses->rc_config->num_phrases;
        local_event->common.data_offset = sizeof(*local_event);
        local_event->common.data_size = opaque_size;
        memcpy(opaque_data, payload, opaque_size);
        opaque_data += opaque_size;
    }

    /*
     * fill the remaining recognition event parameters not specific
     * to soundmodel lib
     */
    local_event->common.status = detect_status;
    local_event->common.type = stc_ses->phrase_sm->common.type;
    local_event->common.model = stc_ses->sm_handle;
    local_event->common.capture_available =
        stc_ses->rc_config->capture_requested;
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

    ALOGI("%s:[c%d]", __func__, stc_ses->sm_handle);

    ALOGV("%s:[c%d] status=%d, type=%d, model=%d, capture_avaiable=%d, "
          "num_phrases=%d id=%d", __func__, stc_ses->sm_handle,
          local_event->common.status, local_event->common.type,
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
 * TODO: Deprecate this when DSP for all shared targets of this component
 * move to generic event.
 */
static int process_detection_event_keyphrase(
    st_proxy_session_t *st_ses, int detect_status,
    void *payload, size_t payload_size,
    struct sound_trigger_phrase_recognition_event **event)
{
    st_hw_session_t *st_hw_ses = st_ses->hw_ses_current;
    st_session_t *stc_ses = st_ses->det_stc_ses;
    unsigned int i = 0, j = 0;
    int status = 0;
    struct sound_trigger_phrase_recognition_event *local_event = NULL;
    size_t opaque_size = 0;
    uint8_t *opaque_data = NULL, *payload_ptr = NULL;
    struct st_param_header *param_hdr = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;
    struct listnode *node = NULL;
    struct st_keyword_indices_info *kw_indices = NULL;
    struct st_timestamp_info *timestamps = NULL;
    bool enable_kw_indices = false;
    unsigned char *cf_levels = NULL;
    unsigned int cf_levels_size = 0;

    if ((stc_ses->rc_config->data_size > CUSTOM_CONFIG_OPAQUE_DATA_SIZE) &&
        st_ses->vendor_uuid_info->is_qcva_uuid) {
        /*
         * This logic is for the updated opaque data format. Sound trigger
         * recognition event APIs are filled along with the opaque data's
         * confidence levels, keyword indices, and timestamp parameters.
         */
        opaque_size = (2 * sizeof(struct st_param_header)) +
            sizeof(struct st_timestamp_info);
        if (stc_ses->conf_levels_intf_version != CONF_LEVELS_INTF_VERSION_0002)
            opaque_size += sizeof(struct st_confidence_levels_info);
        else
            opaque_size += sizeof(struct st_confidence_levels_info_v2);

        list_for_each(node, &stc_ses->second_stage_list) {
            st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
            if (IS_KEYWORD_DETECTION_MODEL(st_sec_stage->ss_info->sm_id)) {
                enable_kw_indices = true;
                opaque_size += sizeof(struct st_param_header) +
                    sizeof(struct st_keyword_indices_info);
                break;
            }
        }

        local_event = calloc(1,
            sizeof(struct sound_trigger_phrase_recognition_event) +
            opaque_size);
        if (!local_event) {
            ALOGE("%s: local_event allocation failed, opaque data size = %d",
                  __func__, (unsigned int)opaque_size);
            return -ENOMEM;
        }

        local_event->num_phrases = stc_ses->rc_config->num_phrases;
        local_event->common.data_offset =
            sizeof(struct sound_trigger_phrase_recognition_event);
        local_event->common.data_size = opaque_size;
        opaque_data = (uint8_t *)local_event + local_event->common.data_offset;
        if ((st_ses->exec_mode == ST_EXEC_MODE_CPE) && st_ses->stdev->is_gcs) {
            payload_ptr = (uint8_t *)payload + 2;
            payload_size -= 2; /* Re-use */
        } else if ((st_ses->exec_mode == ST_EXEC_MODE_ADSP) ||
                   !st_ses->stdev->is_gcs) {
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
        if (stc_ses->conf_levels_intf_version !=
            CONF_LEVELS_INTF_VERSION_0002) {
            param_hdr->payload_size =
                sizeof(struct st_confidence_levels_info);
        } else {
            param_hdr->payload_size =
                sizeof(struct st_confidence_levels_info_v2);
        }
        check_and_extract_det_conf_levels_payload(st_ses, payload_ptr,
            payload_size, &cf_levels, &cf_levels_size);
        if (!cf_levels || !cf_levels_size) {
            status = -EINVAL;
            goto err_exit;
        }
        memcpy(opaque_data, stc_ses->st_conf_levels, param_hdr->payload_size);
        pack_opaque_data_conf_levels(st_ses, opaque_data, cf_levels,
            cf_levels_size);
        pack_recognition_event_conf_levels(st_ses, cf_levels, cf_levels_size,
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
            list_for_each(node, &stc_ses->second_stage_list) {
                st_sec_stage = node_to_item(node, st_arm_second_stage_t,
                list_node);
                if (IS_KEYWORD_DETECTION_MODEL(st_sec_stage->ss_info->sm_id)) {
                    kw_indices->start_index =
                        st_sec_stage->ss_session->kw_start_idx;
                    kw_indices->end_index =
                        st_sec_stage->ss_session->kw_end_idx;
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
            st_hw_ses->first_stage_det_event_time;
        if (!list_empty(&stc_ses->second_stage_list))
            timestamps->second_stage_det_event_time =
                st_hw_ses->second_stage_det_event_time;
        opaque_data += sizeof(struct st_timestamp_info);

        if (st_ses->stdev->enable_debug_dumps) {
            ST_DBG_DECLARE(FILE *opaque_fd = NULL; static int opaque_cnt = 0);
            ST_DBG_FILE_OPEN_WR(opaque_fd, ST_DEBUG_DUMP_LOCATION,
                                "detection_opaque_data", "bin", opaque_cnt++);
            ST_DBG_FILE_WRITE(opaque_fd, (opaque_data - opaque_size),
                              opaque_size);
            ST_DBG_FILE_CLOSE(opaque_fd);
        }

    } else {
        if (st_ses->vendor_uuid_info->is_qcva_uuid ||
            st_ses->vendor_uuid_info->is_qcmd_uuid) {
            if (st_ses->stdev->is_gcs &&
                ST_EXEC_MODE_CPE == st_ses->exec_mode &&
                !st_hw_ses->is_generic_event) {
                payload_ptr = payload;
                payload_ptr += 2; /* Skip minor_version and num_active_models */
                payload_size -= 2;
            } else {
                payload_ptr = payload;
            }
            status = generate_legacy_st_phrase_recognition_event(
                stc_ses->phrase_sm, stc_ses->rc_config, payload_ptr,
                payload_size, &local_event);

            if (status)
                goto exit;
        } else {
            ALOGD("%s: Send detection payload as is", __func__);

            local_event = calloc(1, sizeof(*local_event) + payload_size);
            if (!local_event) {
                ALOGE("%s: event allocation failed, size %zd", __func__,
                    payload_size);
                status = -ENOMEM;
                goto exit;
            }
            memcpy(local_event->phrase_extras,
                stc_ses->rc_config->phrases, stc_ses->rc_config->num_phrases *
                sizeof(struct sound_trigger_phrase_recognition_extra));
            local_event->num_phrases = stc_ses->rc_config->num_phrases;
            local_event->common.data_offset = sizeof(*local_event);
            local_event->common.data_size = payload_size;
            memcpy((char *)local_event + local_event->common.data_offset,
                   payload, payload_size);
        }
    }

    /* fill the remaining recognition event parameters not specific
       to soundmodel lib */
    local_event->common.status = detect_status;
    local_event->common.type = stc_ses->phrase_sm->common.type;
    local_event->common.model = stc_ses->sm_handle;
    local_event->common.capture_available =
        stc_ses->rc_config->capture_requested;
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

    ALOGI("%s:[c%d]", __func__, stc_ses->sm_handle);

    ALOGV("%s:[c%d] status=%d, type=%d, model=%d, capture_avaiable=%d, "
          "num_phrases=%d id=%d", __func__, stc_ses->sm_handle,
          local_event->common.status, local_event->common.type,
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

static int process_detection_event_generic(st_proxy_session_t *st_ses,
    int detect_status,
    void *payload, size_t payload_size,
    struct sound_trigger_recognition_event **event)
{
    st_hw_session_t *st_hw_ses = st_ses->hw_ses_current;
    st_session_t *stc_ses = st_ses->det_stc_ses;
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
    local_event->type = stc_ses->sm_type;
    local_event->model = stc_ses->sm_handle;
    local_event->capture_available = stc_ses->rc_config->capture_requested;
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

    ALOGI("%s:[%d]", __func__, stc_ses->sm_handle);
    ALOGV("%s:[c%d] status=%d, type=%d, model=%d, capture_avaiable=%d",
        __func__, stc_ses->sm_handle, local_event->status,
        local_event->type, local_event->model,
        local_event->capture_available);

    *event = local_event;

exit:
    return status;
}

static inline int process_detection_event(st_proxy_session_t *st_ses,
    uint64_t timestamp __unused,
    int detect_status,
    void *payload, size_t payload_size,
    struct sound_trigger_recognition_event **event)
{
    int ret;
    struct sound_trigger_phrase_recognition_event *phrase_event = NULL;

    *event = NULL;
    if (st_ses->sm_info.sm_type == SOUND_MODEL_TYPE_KEYPHRASE) {
        if (sthw_extn_check_process_det_ev_support())
            ret = sthw_extn_process_detection_event_keyphrase(st_ses,
                timestamp, detect_status, payload, payload_size, &phrase_event);
        else if (st_ses->hw_ses_current->is_generic_event &&
                 !st_ses->vendor_uuid_info->is_qcmd_uuid)
            ret = process_detection_event_keyphrase_v2(st_ses, detect_status,
                payload, payload_size, &phrase_event);
        else
            ret = process_detection_event_keyphrase(st_ses, detect_status,
                payload, payload_size, &phrase_event);
        if (phrase_event)
            *event = &phrase_event->common;
    } else {
        ret = process_detection_event_generic(st_ses, detect_status, payload,
            payload_size, event);
    }
    return ret;
}


/*
 * If the keyword detection session detects before the user verification
 * session, signal to process user verification. If the keyword detection
 * session rejects before the user verification session, signal to stop
 * processing user verification.
 */
static void handle_vop_pending_detection(st_arm_ss_session_t *ss_session,
    unsigned int det_status, unsigned int kw_det_buff_sz)
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
    st_proxy_session_t *st_ses = (st_proxy_session_t *)st_session;
    st_session_t *stc_ses = NULL;
    recognition_callback_t callback = NULL;
    void *cookie = NULL;
    struct listnode *node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;
    int status = 0, lock_status = 0;
    unsigned int kw_det_buff_sz = 0, det_status = 0;
    struct timespec tspec = {0};
    struct sound_trigger_recognition_event *event = NULL;
    bool capture_requested = false;
    uint64_t callback_time = 0;

    ALOGV("%s: Enter", __func__);

    /*
     * For multi-clients it is expected only one of the clients detection
     * happens at a time. Continue processing on a run time detected client
     */
    pthread_mutex_lock(&st_ses->ss_detections_lock);
    while (!st_ses->exit_aggregator_loop) {
        det_status = 0;
        lock_status = 0;
        ALOGV("%s: waiting on cond", __func__);
        pthread_cond_wait(&st_ses->ss_detections_cond,
            &st_ses->ss_detections_lock);
        ALOGV("%s: done waiting on cond", __func__);
        if (st_ses->exit_aggregator_loop) {
            ALOGV("%s: exit", __func__);
            pthread_mutex_unlock(&st_ses->ss_detections_lock);
            return NULL;
        }
        if (!st_ses->det_stc_ses)
            continue;
        stc_ses = st_ses->det_stc_ses;

        list_for_each(node, &stc_ses->second_stage_list) {
            st_sec_stage = node_to_item(node, st_arm_second_stage_t,
                list_node);

            pthread_mutex_lock(&st_sec_stage->ss_session->lock);
            det_status |= st_sec_stage->ss_session->det_status;
            if (st_sec_stage->ss_session->det_status ==
                KEYWORD_DETECTION_SUCCESS)
                kw_det_buff_sz = st_sec_stage->ss_session->bytes_processed;
            pthread_mutex_unlock(&st_sec_stage->ss_session->lock);
        }

        list_for_each(node, &stc_ses->second_stage_list) {
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
                    /*
                     * Stop buffering if this is not a successful detection and
                     * LAB is triggered in hw automatically
                     */
                    st_ses->hw_ses_current->fptrs->stop_buffering(
                        st_ses->hw_ses_current);

                    pthread_mutex_unlock(&st_ses->lock);
                    if (event) {
                        free(event);
                        event = NULL;
                    }
                    goto exit;
                }
                stc_ses->detection_sent = true;
                callback = stc_ses->callback;
                capture_requested = stc_ses->rc_config->capture_requested;
                cookie = stc_ses->cookie;
                callback_time = get_current_time_ns();
                ALOGD("%s:[c%d] Second stage detected successfully, "
                    "calling client callback", __func__, stc_ses->sm_handle);
                ALOGD("%s: Total sthal processing time: %llums", __func__,
                    (callback_time - st_ses->detection_event_time) /
                    NSECS_PER_MSEC);
                pthread_mutex_unlock(&st_ses->lock);
                ATRACE_BEGIN("sthal: client detection callback");
                callback(event, cookie);
                free(event);
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
                    goto exit;
                }
                /*
                 * If client has not requested capture data,
                 * stop hw session buffering here to resume next
                 * detection
                 */
                if (!capture_requested)
                    st_ses->hw_ses_current->fptrs->stop_buffering(
                        st_ses->hw_ses_current);
            } else {
                ATRACE_ASYNC_END("sthal: detection reject",
                    st_ses->sm_handle);
                ALOGD("%s: Second stage did NOT detect, restarting st_session",
                    __func__);
                st_ses->hw_ses_current->fptrs->stop_buffering(
                    st_ses->hw_ses_current);
                start_second_stage_for_client(stc_ses);
                st_session_ev_t ev = {.ev_id = ST_SES_EV_RESTART,
                    .stc_ses = stc_ses};
                DISPATCH_EVENT(st_ses, ev, status);
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

static void init_det_event_aggregator(st_proxy_session_t *st_ses)
{
    int status = 0;
    pthread_condattr_t attr;

    ALOGV("%s", __func__);

    st_ses->exit_aggregator_loop = false;
    pthread_mutex_init(&st_ses->ss_detections_lock, NULL);
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&st_ses->ss_detections_cond, &attr);
    pthread_condattr_destroy(&attr);
    status = pthread_create(&st_ses->aggregator_thread, NULL,
        aggregator_thread_loop, st_ses);
    if (status) {
        ALOGE("%s: Error creating aggregator thread. status = %d",
              __func__, status);
    } else {
        st_ses->aggregator_thread_created = true;
    }
}

static void destroy_det_event_aggregator(st_proxy_session_t *st_ses)
{
    int status = 0;

    ALOGV("%s", __func__);

    st_ses->exit_aggregator_loop = true;
    pthread_mutex_lock(&st_ses->ss_detections_lock);
    pthread_cond_signal(&st_ses->ss_detections_cond);
    pthread_mutex_unlock(&st_ses->ss_detections_lock);
    status = pthread_join(st_ses->aggregator_thread, NULL);
    if (status)
        ALOGE("%s: Error joining aggregator thread. status = %d",
              __func__, status);
    pthread_cond_destroy(&st_ses->ss_detections_cond);
    pthread_mutex_destroy(&st_ses->ss_detections_lock);
    st_ses->aggregator_thread_created = false;
}

/* This function is called for multi-client */
static int handle_load_sm(st_proxy_session_t *st_ses, st_session_t *stc_ses)
{
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    int status = 0;

    ALOGV("%s:[c%d-%d]", __func__, stc_ses->sm_handle, st_ses->sm_handle);
    if (!stc_ses->phrase_sm) {
        ALOGE("%s:[c%d] sound model data is not initialzed", __func__,
              stc_ses->sm_handle);
        return -EINVAL;
    }

    if (!is_other_client_attached(st_ses, stc_ses)) {
        ALOGE("%s:[c%d] Unexpected without multi-clients", __func__,
              stc_ses->sm_handle);
        return -EINVAL;
    }

    if (st_ses->current_state == buffering_state_fn)
        hw_ses->fptrs->stop_buffering(hw_ses);

    if (st_ses->current_state == active_state_fn ||
        st_ses->current_state == detected_state_fn ||
        st_ses->current_state == buffering_state_fn) {
        status = stop_session(st_ses, hw_ses, false);
        if (status)
            ALOGE("%s:[%d] stop_session failed %d", __func__, st_ses->sm_handle,
                  status);
        STATE_TRANSITION(st_ses, loaded_state_fn);
    }

    status = hw_ses->fptrs->dereg_sm(hw_ses);
    if (status) {
        ALOGE("%s:[%d] dereg_sm failed %d", __func__,
            st_ses->sm_handle, status);
    }
    /* Continue updating sound model resulting in merged model */
    status = update_sound_model(stc_ses, true);
    if (status) {
        ALOGE("%s:[c%d] update_sound_model add failed %d", __func__,
              stc_ses->sm_handle, status);
        goto exit;
    }
    hw_ses->sthw_cfg.conf_levels = st_ses->sm_info.cf_levels;
    hw_ses->sthw_cfg.num_conf_levels = st_ses->sm_info.cf_levels_size;
    hw_ses->sthw_cfg_updated = true;
    /*
     * Sound model merge would have changed the order of merge conf levels,
     * which need to be re-updated for all current active clients, if any.
     */
    status = update_merge_conf_levels_payload_with_active_clients(st_ses);
    if (status)
        goto exit_1;

    /* Load merged sound model */
    status = hw_ses->fptrs->reg_sm(hw_ses, st_ses->sm_info.sm_data,
            st_ses->sm_info.sm_size, st_ses->sm_info.sm_type);
    if (status) {
        ALOGE("%s:[%d] reg_sm failed %d", __func__,
            st_ses->sm_handle, status);
        goto exit_1;
    }

    if (st_ses->current_state == active_state_fn ||
        st_ses->current_state == detected_state_fn ||
        st_ses->current_state == buffering_state_fn) {

        status = start_session(st_ses, hw_ses, false);
        if (status)
            goto exit_2;
        STATE_TRANSITION(st_ses, active_state_fn);
    }

    return 0;

exit_2:
    if (!st_ses->stdev->ssr_offline_received)
        hw_ses->fptrs->dereg_sm(hw_ses);

exit_1:
    if (!st_ses->stdev->ssr_offline_received) {
        update_sound_model(stc_ses, false);
        update_merge_conf_levels_payload_with_active_clients(st_ses);
    }

exit:
    if (st_ses->stdev->ssr_offline_received) {
        STATE_TRANSITION(st_ses, ssr_state_fn);
        status = 0;
    }
    return status;
}

/* This function is called for multi-client */
static int handle_unload_sm(st_proxy_session_t *st_ses, st_session_t *stc_ses)
{
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    int status = 0;

    ALOGV("%s:[c%d-%d]", __func__, stc_ses->sm_handle, st_ses->sm_handle);

    if (!is_other_client_attached(st_ses, stc_ses)) {
        ALOGE("%s:[c%d] Unexpected without multi-clients", __func__,
            stc_ses->sm_handle);
        return -EINVAL;
    }

    if (st_ses->current_state == buffering_state_fn)
        hw_ses->fptrs->stop_buffering(hw_ses);

    if (st_ses->current_state == active_state_fn ||
        st_ses->current_state == detected_state_fn ||
        st_ses->current_state == buffering_state_fn) {
        status = stop_session(st_ses, hw_ses, false);
        if (status)
            ALOGE("%s:[%d] stop_session failed %d", __func__,
                st_ses->sm_handle, status);
        STATE_TRANSITION(st_ses, loaded_state_fn);
    }

    status = hw_ses->fptrs->dereg_sm(hw_ses);
    if (status)
        ALOGE("%s:[%d] dereg_sm failed %d", __func__, st_ses->sm_handle, status);

    /* Continue deleting this model */
    status = update_sound_model(stc_ses, false);
    if (status)
        ALOGE("%s:[c%d] update_sound_model delete failed %d", __func__,
            stc_ses->sm_handle, status);

    hw_ses->sthw_cfg.conf_levels = st_ses->sm_info.cf_levels;
    hw_ses->sthw_cfg.num_conf_levels = st_ses->sm_info.cf_levels_size;
    hw_ses->sthw_cfg_updated = true;
    /*
     * Sound model merge would have changed the order of merge conf levels,
     * which need to be re-updated for all current active clients, if any.
     */
    update_merge_conf_levels_payload_with_active_clients(st_ses);

    /* Load remaining merged sound model */
    status = hw_ses->fptrs->reg_sm(hw_ses, st_ses->sm_info.sm_data,
        st_ses->sm_info.sm_size, st_ses->sm_info.sm_type);
    if (status) {
        ALOGE("%s:[%d] reg_sm failed %d", __func__,
            st_ses->sm_handle, status);
        goto exit;
    }

    if (st_ses->current_state == active_state_fn ||
        st_ses->current_state == detected_state_fn ||
        st_ses->current_state == buffering_state_fn) {

        status = start_session(st_ses, hw_ses, false);
        if (status)
            goto exit;
        STATE_TRANSITION(st_ses, active_state_fn);
    }
    return 0;

exit:
    if (st_ses->stdev->ssr_offline_received) {
        STATE_TRANSITION(st_ses, ssr_state_fn);
        status = 0;
    }
    return status;
}

static int idle_state_fn(st_proxy_session_t *st_ses, st_session_ev_t *ev)
{
    int status = 0;
    int ret = 0;
    st_session_t *stc_ses = ev->stc_ses;
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;

    /* skip parameter check as this is an internal funciton */
    ALOGD("%s:[c%d-%d] handle event id %d", __func__, stc_ses->sm_handle,
        st_ses->sm_handle, ev->ev_id);

    switch (ev->ev_id) {
    case ST_SES_EV_LOAD_SM:
        if (!stc_ses->phrase_sm) {
            ALOGE("%s: sound model data is not initialzed", __func__);
            status = -EINVAL;
            break;
        }
        status = update_sound_model(stc_ses, true);
        if (status) {
            ALOGE("%s:[c%d] update sound model add failed %d", __func__,
                stc_ses->sm_handle, status);
            status = -EINVAL;
            break;
        }

        /*
         * Do retry to handle a corner case that when ADSP SSR ONLINE is
         * received, sometimes ADSP is still not ready to receive cmd from HLOS
         * and thus fails, so try more times to recover the session from SSR
         * state.
         */
        for (int i = 0; i < REG_SM_RETRY_CNT; i++) {
            status = ret = hw_ses->fptrs->reg_sm(hw_ses, st_ses->sm_info.sm_data,
                st_ses->sm_info.sm_size, st_ses->sm_info.sm_type);
            if (ret) {
                if (st_ses->stdev->ssr_offline_received) {
                    STATE_TRANSITION(st_ses, ssr_state_fn);
                    status = 0;
                    break;
                } else {
                    ALOGE("%s:[%d] failed to reg sm, err %d, retry cnt %d",
                          __func__, st_ses->sm_handle, status, i);
                    usleep(REG_SM_WAIT_TIME_MS * 1000);
                }
            } else {
                break;
            }
        }
        if (ret)
            break;

        STATE_TRANSITION(st_ses, loaded_state_fn);
        break;

    case ST_SES_EV_SET_EXEC_MODE:
        stc_ses->exec_mode = ev->payload.exec_mode;
        if (ev->payload.exec_mode == st_ses->exec_mode)
            break;

        st_ses->exec_mode = ev->payload.exec_mode;
        if (ST_EXEC_MODE_CPE == st_ses->exec_mode)
            st_ses->hw_ses_current = st_ses->hw_ses_cpe;
        else if (ST_EXEC_MODE_ADSP == st_ses->exec_mode)
            st_ses->hw_ses_current = st_ses->hw_ses_adsp;
        /* remain in current state */
        break;

    case ST_SES_EV_PAUSE:
        stc_ses->paused = true;
        break;

    case ST_SES_EV_RESUME:
        stc_ses->paused = false;
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

static int loaded_state_fn(st_proxy_session_t *st_ses, st_session_ev_t *ev)
{
    int status = 0;
    st_session_t *stc_ses = ev->stc_ses;
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    st_hw_session_t *new_hw_ses = NULL;
    st_exec_mode_t new_exec_mode = 0;

    /* skip parameter check as this is an internal function */
    ALOGD("%s:[c%d-%d] handle event id %d", __func__, stc_ses->sm_handle,
                st_ses->sm_handle, ev->ev_id);

    switch (ev->ev_id) {
    case ST_SES_EV_LOAD_SM:
        /* Valid only in multi-client session usecase */
        status = handle_load_sm(st_ses, stc_ses);
        break;

    case ST_SES_EV_UNLOAD_SM:
        if (is_other_client_attached(st_ses, stc_ses)) {
            status = handle_unload_sm(st_ses, stc_ses);
            break;
        }

        status = hw_ses->fptrs->dereg_sm(hw_ses);
        if (status)
            ALOGE("%s:[%d] dereg_sm failed %d", __func__,
                st_ses->sm_handle, status);

        status = update_sound_model(stc_ses, false);
        if (status)
            ALOGE("%s:[c%d] update_sound_model failed %d", __func__,
                stc_ses->sm_handle, status);

        /* since this is a teardown scenario dont fail here */
        status = 0;
        STATE_TRANSITION(st_ses, idle_state_fn);
        break;

    case ST_SES_EV_RESUME:
        stc_ses->paused = false;
        if (!is_any_client_in_state(st_ses, ST_STATE_ACTIVE)) {
            /*
             * When a transition is needed due to lpi mode or barge-in mode,
             * call dereg_sm and reg_sm to select the updated lsm_usecase.
             */
            if (hw_ses->lpi_enable != hw_ses->stdev->lpi_enable ||
                (hw_ses->barge_in_mode != hw_ses->stdev->barge_in_mode &&
                 !hw_ses->stdev->support_dynamic_ec_update)) {

                hw_ses->lpi_enable = hw_ses->stdev->lpi_enable;
                hw_ses->barge_in_mode = hw_ses->stdev->barge_in_mode;

                status = hw_ses->fptrs->dereg_sm(hw_ses);
                if (status) {
                    ALOGE("%s:[%d] failed to dereg_sm err %d", __func__,
                        st_ses->sm_handle, status);
                    break;
                }

                status = hw_ses->fptrs->reg_sm(hw_ses, st_ses->sm_info.sm_data,
                    st_ses->sm_info.sm_size, st_ses->sm_info.sm_type);
                if (status) {
                    ALOGE("%s:[%d] failed to reg_sm err %d", __func__,
                        st_ses->sm_handle, status);
                    STATE_TRANSITION(st_ses, idle_state_fn);
                }
            }
            break;
        }
        /* Fall through */
    case ST_SES_EV_START:
    case ST_SES_EV_RESTART:
        if (ev->ev_id == ST_SES_EV_RESTART)
            update_hw_config_on_restart(st_ses, stc_ses);

        /*
        * During Resume, the first active client will start the hw sesison.
        * During Start, check for any paused sessions to delay actual start
        * to Resume.
        */
        if ((ev->ev_id != ST_SES_EV_RESUME) && is_any_client_paused(st_ses))
            break;

        status = start_session(st_ses, hw_ses, false);
        if (status) {
            if (st_ses->stdev->ssr_offline_received) {
                hw_ses->fptrs->dereg_sm(hw_ses);
                STATE_TRANSITION(st_ses, ssr_state_fn);
                status = 0;
            } else {
                ALOGE("%s:[%d] failed to start session, err %d", __func__,
                    st_ses->sm_handle, status);
            }
            break;
        }
        STATE_TRANSITION(st_ses, active_state_fn);
        break;

    case ST_SES_EV_STOP:
        /*
         * Valid in multi-client case.
         * Reconfig based off other active clients, if any, so that RESUME
         * can apply this reconfig.
         */
        update_hw_config_on_stop(st_ses, stc_ses);
        break;

    case ST_SES_EV_SSR_OFFLINE:
        /* exec mode can be none if ssr occurs during a transition */
        if (st_ses->exec_mode != ST_EXEC_MODE_NONE)
            hw_ses->fptrs->dereg_sm(hw_ses);
        STATE_TRANSITION(st_ses, ssr_state_fn);
        break;

    case ST_SES_EV_PAUSE:
        stc_ses->paused = true;
        break;

    case ST_SES_EV_SET_EXEC_MODE:
        new_exec_mode = ev->payload.exec_mode;

        if (new_exec_mode == st_ses->exec_mode) {
            stc_ses->exec_mode = st_ses->exec_mode;
            break;
        }

        if (st_ses->exec_mode != ST_EXEC_MODE_NONE) {
            st_ses->exec_mode = ST_EXEC_MODE_NONE;
            list_for_each(node, &st_ses->clients_list) {
                c_ses = node_to_item(node, st_session_t, hw_list_node);
                c_ses->exec_mode = ST_EXEC_MODE_NONE;
            }
            /* unload sm for current hw session */
            status = hw_ses->fptrs->dereg_sm(hw_ses);
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
        } else if (ST_EXEC_MODE_ADSP == new_exec_mode) {
            new_hw_ses = st_ses->hw_ses_adsp;
        } else {
            ALOGE("%s: unknown execution mode %d", __func__,
                new_exec_mode);
            status = -EINVAL;
            break;
        }

        status = new_hw_ses->fptrs->reg_sm(new_hw_ses,
            st_ses->sm_info.sm_data, st_ses->sm_info.sm_size,
            st_ses->sm_info.sm_type);
        if (status) {
            ALOGE("%s:[%d] reg_sm failed with err %d", __func__,
                st_ses->sm_handle, status);
            break;
        }
        /* switch hw sessions only if successful*/
        list_for_each(node, &st_ses->clients_list) {
            c_ses = node_to_item(node, st_session_t, hw_list_node);
            c_ses->exec_mode = new_exec_mode;
            if (c_ses->state == ST_STATE_ACTIVE) {
                dereg_hal_event_session(c_ses);
                reg_hal_event_session(c_ses, new_hw_ses);
            }
        }
        st_ses->exec_mode = new_exec_mode;
        st_ses->hw_ses_current = new_hw_ses;
        /* remain in current state */
        break;

    case ST_SES_EV_SET_DEVICE:
        /*
         * This event handling is needed for certain graphs which
         * have multiple buffering modules with a single voice wakeup
         * module in each usecase.
         */
        if (!ev->payload.enable) {
            status = hw_ses->fptrs->disable_device(hw_ses, false);
        } else {
            status = hw_ses->fptrs->enable_device(hw_ses, false);
            /*
             * Device switch might happen during active buffering.
             * If any client is active, start hw session.
             */
            if (is_any_client_in_state(st_ses, ST_STATE_ACTIVE)) {
                st_session_ev_t start_ev = {.ev_id = ST_SES_EV_START,
                    .stc_ses = stc_ses};
                DISPATCH_EVENT(st_ses, start_ev, status);
            }
        }

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

static int active_state_fn(st_proxy_session_t *st_ses, st_session_ev_t *ev)
{
    int status = 0;
    st_session_t *stc_ses = ev->stc_ses;
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    st_hw_session_t *new_hw_ses = NULL;
    st_exec_mode_t new_exec_mode;
    st_arm_second_stage_t *st_sec_stage = NULL;
    struct sound_trigger_recognition_event *event = NULL;
    recognition_callback_t callback = NULL;
    void *cookie = NULL;
    bool lab_enabled = false, enable_second_stage = false, active = false;

    /* skip parameter check as this is an internal funciton */
    ALOGD("%s:[%d] handle event id %d", __func__, st_ses->sm_handle, ev->ev_id);

    switch (ev->ev_id) {
    case ST_SES_EV_LOAD_SM:
        /* Valid in multi-client usecase */
        status = handle_load_sm(st_ses, stc_ses);
        break;

    case ST_SES_EV_UNLOAD_SM:
        /* Valid in multi-client usecase */
        status = handle_unload_sm(st_ses, stc_ses);
        break;

    case ST_SES_EV_RESTART:
        /* Valid in multi-client usecase */
        update_hw_config_on_restart(st_ses, stc_ses);
        /* Fall through */
    case ST_SES_EV_START:
        /* Valid in multi-client usecase */
        status = stop_session(st_ses, hw_ses, false);
        if (!status) {
            status = start_session(st_ses, hw_ses, false);
            if (status)
                ALOGE("%s:[%d] start_session failed %d", __func__,
                    st_ses->sm_handle, status);
        } else {
            ALOGE("%s:[%d] stop_session failed %d", __func__,
                st_ses->sm_handle, status);
        }
        if (status & st_ses->stdev->ssr_offline_received) {
            hw_ses->fptrs->dereg_sm(hw_ses);
            STATE_TRANSITION(st_ses, ssr_state_fn);
            status = 0;
        }
        break;

    case ST_SES_EV_SET_EXEC_MODE:
        new_exec_mode = ev->payload.exec_mode;

        if (new_exec_mode == st_ses->exec_mode) {
            stc_ses->exec_mode = st_ses->exec_mode;
            break;
        }

        if (st_ses->exec_mode != ST_EXEC_MODE_NONE) {
            ALOGV("%s: disable current session", __func__);
            st_ses->exec_mode = ST_EXEC_MODE_NONE;
            status = stop_session(st_ses, hw_ses, true);
            list_for_each(node, &st_ses->clients_list) {
                c_ses = node_to_item(node, st_session_t, hw_list_node);
                c_ses->exec_mode = ST_EXEC_MODE_NONE;
            }
            if (status)
                break;
        }

        if (new_exec_mode == ST_EXEC_MODE_NONE)
            break;

        if (ST_EXEC_MODE_CPE == new_exec_mode) {
            new_hw_ses = st_ses->hw_ses_cpe;
        } else if (ST_EXEC_MODE_ADSP == new_exec_mode) {
            new_hw_ses = st_ses->hw_ses_adsp;
        } else {
            ALOGE("%s: unknown execution mode %d", __func__,
                new_exec_mode);
            status = -EINVAL;
            break;
        }
        /*
         * hw session changed to/from WDSP/ADSP, hence update the
         * related config.
         * Not applicable for LPI<->non-LPI transtions as hw session
         * doesn't change.
         */
        status = update_hw_config_on_start(stc_ses, new_hw_ses);
        if (status) {
            ALOGE("%s: Update_hw_config_on_start failed %d",
                __func__, status);
            break;
        }

        ALOGV("%s: enable current session", __func__);
        status = start_session(st_ses, new_hw_ses, true);
        if (status)
            break;

        /* set new exec mode and current session */
        list_for_each(node, &st_ses->clients_list) {
            c_ses = node_to_item(node, st_session_t, hw_list_node);
            c_ses->exec_mode = new_exec_mode;
            if (c_ses->state == ST_STATE_ACTIVE) {
                dereg_hal_event_session(c_ses);
                reg_hal_event_session(c_ses, new_hw_ses);
            }
        }
        st_ses->exec_mode = new_exec_mode;
        st_ses->hw_ses_current = new_hw_ses;

        ALOGV("%s: end transition", __func__);
        break;

    case ST_SES_EV_PAUSE:
        /*
         * For multi-client, the first active pausing client stops the hw
         * session and moves to loaded state.
         */
        stc_ses->paused = true;
        if (stc_ses->state != ST_STATE_ACTIVE)
                break;

        status = stop_session(st_ses, hw_ses, false);

        if (status) {
            if (st_ses->stdev->ssr_offline_received) {
                STATE_TRANSITION(st_ses, ssr_state_fn);
                hw_ses->fptrs->dereg_sm(hw_ses);
                status = 0;
            } else {
                ALOGE("%s:[%d] failed to stop session, err %d", __func__,
                    st_ses->sm_handle, status);
                /* Move anyway to loaded state */
                STATE_TRANSITION(st_ses, loaded_state_fn);
            }
            break;
        }
        STATE_TRANSITION(st_ses, loaded_state_fn);
        break;

    case ST_SES_EV_RESUME:
        if (stc_ses->paused == true)
            stc_ses->paused = false;
        break;

    case ST_SES_EV_STOP:
        status = stop_session(st_ses, hw_ses, false);
        if (status)
            ALOGE("%s:[%d] start_session failed %d", __func__,
                st_ses->sm_handle, status);

        /* Continue to reconfig based off other active clients, if any */
        active = update_hw_config_on_stop(st_ses, stc_ses);

        if (!status) {
            if (active) {
                ALOGD("%s: client c%d stopped, start %d due to reconfig",
                    __func__, stc_ses->sm_handle, st_ses->sm_handle);
                status = start_session(st_ses, hw_ses, false);
                if (status)
                    ALOGE("%s:[%d] start_session failed %d", __func__,
                        st_ses->sm_handle, status);
                /* Stay in active state */
            } else {
                STATE_TRANSITION(st_ses, loaded_state_fn);
            }
        }

        if (status) {
            if (st_ses->stdev->ssr_offline_received) {
                hw_ses->fptrs->dereg_sm(hw_ses);
                STATE_TRANSITION(st_ses, ssr_state_fn);
                status = 0;
            } else {
                STATE_TRANSITION(st_ses, loaded_state_fn);
            }
        }

        break;

    case ST_SES_EV_DETECTED:

        st_ses->detection_event_time = get_current_time_ns();
        /*
         * Find which client is this detection for.
         * Note that only one keyword detection can happen at a time.
         */
        stc_ses = get_detected_client(st_ses,
            ev->payload.detected.detect_payload,
            ev->payload.detected.payload_size);

        if (!stc_ses) {
            ALOGW("%s:[%d] Couldn't find a matching client for detection",
                __func__, st_ses->sm_handle);
            /*
             * Though we set higest conf level 100 for inactive client in merged
             * sound model, it may be possible it still detects. In case the lab
             * is enabled due to other active client, stop hw buffering.
             */
            if (st_ses->lab_enabled)
                hw_ses->fptrs->stop_buffering(hw_ses);
            break;
        }
        st_ses->det_stc_ses = stc_ses;
        st_ses->hw_ses_current->enable_second_stage = false; /* Initialize */
        stc_ses->detection_sent = false;

        if (list_empty(&stc_ses->second_stage_list) ||
            st_ses->detection_requested) {
            st_ses->detection_requested = false;
            status = process_detection_event(st_ses,
                ev->payload.detected.timestamp,
                ev->payload.detected.detect_status,
                ev->payload.detected.detect_payload,
                ev->payload.detected.payload_size,
                &event);
            if (status || !event) {
                ALOGE("%s:[%d] process_detection_event failed err %d", __func__,
                    st_ses->sm_handle, status);
                /*
                 * Stop buffering if this is not a successful detection and
                 * LAB is triggered in hw automatically
                 */
                hw_ses->fptrs->stop_buffering(hw_ses);
                if (event)
                    free(event);
                break;
            }
        } else {
            ALOGV("%s:[c%d] second stage enabled, list_empty %d,"
                  "det_requested %d", __func__, stc_ses->sm_handle,
                  list_empty(&stc_ses->second_stage_list),
                  st_ses->detection_requested);

            enable_second_stage = true;
            /*
             * Before first stage starts buffering, update the second stage info
             * to first stage layer for further communication between first and
             * second stage layers.
             */
            st_ses->hw_ses_current->enable_second_stage = true;
            st_ses->hw_ses_current->second_stage_list =
                &(stc_ses->second_stage_list);

            list_for_each(node, &stc_ses->second_stage_list) {
                st_sec_stage = node_to_item(node, st_arm_second_stage_t,
                                            list_node);
                st_sec_stage->ss_session->st_ses = st_ses;
            }
            get_first_stage_detection_params(st_ses,
                ev->payload.detected.detect_payload,
                ev->payload.detected.payload_size);
            memcpy(st_ses->det_session_ev, ev, sizeof(st_session_ev_t));
        }
        /*
         * change to new state before invoking user callback, this will
         * ensure that if user calls start_recognition immediately from the
         * callback it will be handled by one of the two states below
         */
        if (!status && st_ses->lab_enabled) {
            if (stc_ses->rc_config->capture_requested ||
                !list_empty(&stc_ses->second_stage_list)) {
                if (st_ses->stdev->enable_debug_dumps) {
                    ST_DBG_FILE_OPEN_WR(st_ses->lab_fp, ST_DEBUG_DUMP_LOCATION,
                        "lab_capture", "bin", file_cnt++);
                }
                STATE_TRANSITION(st_ses, buffering_state_fn);
                lab_enabled = true;
            } else {
                /*
                 * for merged model case the client detected may not have
                 * requested the capture nor enabled the second stage, but the
                 * hw lab could be enabled due to other client requested capture
                 * or enabled second stage.
                 */
                ALOGV("%s: stop buffering as c%d doesn't need", __func__,
                    stc_ses->sm_handle);
                hw_ses->fptrs->stop_buffering(hw_ses);
                STATE_TRANSITION(st_ses, detected_state_fn);
                lab_enabled = false;
            }
        } else {
            STATE_TRANSITION(st_ses, detected_state_fn);
        }

        if (!stc_ses->callback) {
            ALOGE("%s:[c%d] received detection event but no callback",
                __func__, stc_ses->sm_handle);
            status = -EINVAL;
            if (event)
                free(event);
            break;
        }
        /*
         * callback to user, assumption is that client does not
         * block in the callback waiting for data otherwise will be a deadlock.
         * If second stage is enabled, the detection will be sent later when
         * second stage successfully detects.
         */
        if (!enable_second_stage) {
            stc_ses->detection_sent = true;
            callback = stc_ses->callback;
            cookie = stc_ses->cookie;
            ALOGD("%s:[c%d] invoking the client callback",
                __func__, stc_ses->sm_handle);
            ATRACE_ASYNC_END("sthal: detection success",
                st_ses->sm_handle);
            pthread_mutex_unlock(&st_ses->lock);
            ATRACE_BEGIN("sthal: client detection callback");
            callback(event, cookie);
            ATRACE_END();
        } else {
            pthread_mutex_unlock(&st_ses->lock);
        }
        if (event)
            free(event);

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
         * For multi-client scenario, only one client is assumed to be
         * detected/bufffering, so the logic remains same.
         */
         do {
             status = pthread_mutex_trylock(&st_ses->lock);
         } while (status && ((st_ses->current_state == detected_state_fn) ||
                  (st_ses->current_state == buffering_state_fn)) &&
                  !st_ses->stdev->ssr_offline_received);

        if (st_ses->current_state != detected_state_fn) {
            ALOGV("%s:[%d] client not in detected state, lock status %d",
                __func__, st_ses->sm_handle, status);
            if (!status) {
                /*
                 * If detection is sent to client while in buffering state,
                 * and if internal buffering is stopped due to errors, stop
                 * session internally as client is expected to restart the
                 * detection if required.
                 * Note: It is possible that detection event is not sent to
                 * client if second stage is not yet detected during internal
                 * buffering stop, in which case restart is posted from second
                 * stage thread for further detections.
                 */
                if ((st_ses->current_state == buffering_state_fn) &&
                    !stc_ses->pending_stop && stc_ses->detection_sent) {
                    ALOGD("%s:[%d] buffering stopped internally, post c%d stop",
                        __func__, st_ses->sm_handle,
                        st_ses->det_stc_ses->sm_handle);
                    status = hw_session_notifier_enqueue(stc_ses->sm_handle,
                        ST_SES_EV_DEFERRED_STOP,
                        ST_SES_DEFERRED_STOP_SS_DELAY_MS);
                    if (!status)
                        stc_ses->pending_stop = true;
                }
                pthread_mutex_unlock(&st_ses->lock);
            }
            status = 0;
            break;
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
            st_session_ev_t deferred_ev = { .ev_id = ST_SES_EV_DEFERRED_STOP,
                .stc_ses = stc_ses};
            DISPATCH_EVENT(st_ses, deferred_ev, status);
        } else {
            ALOGE("%s:[%d] capture is requested but state is still detected!?",
                __func__, st_ses->sm_handle);
        }
        break;

    case ST_SES_EV_SSR_OFFLINE:
        /* exec mode can be none if ssr occurs during a transition */
        if (st_ses->exec_mode != ST_EXEC_MODE_NONE)
            stop_session(st_ses, hw_ses, true);
        STATE_TRANSITION(st_ses, ssr_state_fn);
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

        if (status && st_ses->stdev->ssr_offline_received) {
            STATE_TRANSITION(st_ses, ssr_state_fn);
            status = 0;
        }
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
        if (!list_empty(&stc_ses->second_stage_list)) {
            ALOGE("%s:[%d] Event not supported with second stage enabled",
                __func__, st_ses->sm_handle);
            status = -EINVAL;
            break;
        }
        status = hw_ses->fptrs->send_detection_request(hw_ses);
        if (!status)
            st_ses->detection_requested = true;
        break;

    default:
        ALOGD("%s:[%d] unhandled event", __func__, st_ses->sm_handle);
        break;
    };

    return status;
}

static int detected_state_fn(st_proxy_session_t *st_ses, st_session_ev_t *ev)
{
    int status = 0;
    st_exec_mode_t new_exec_mode = ST_EXEC_MODE_NONE;
    st_session_t *stc_ses = ev->stc_ses;
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    st_hw_session_t *new_hw_ses = NULL;

    /* skip parameter check as this is an internal funciton */
    ALOGD("%s:[c%d-%d] handle event id %d", __func__, stc_ses->sm_handle,
        st_ses->sm_handle, ev->ev_id);

    switch (ev->ev_id) {
    case ST_SES_EV_LOAD_SM:
        /* Valid event only in multi-client usecase */
        /*
         * If a detected client deffered stop event is not handled yet,
         * handle here before moving out of detected state
         */
        if (st_ses->det_stc_ses && !st_ses->det_stc_ses->pending_stop) {
            ALOGD("%s:[%d] post deferred stop for c%d", __func__,
                st_ses->sm_handle, st_ses->det_stc_ses->sm_handle);
            status = hw_session_notifier_enqueue(
               st_ses->det_stc_ses->sm_handle, ST_SES_EV_DEFERRED_STOP,
               ST_SES_DEFERRED_STOP_DELAY_MS);
            if (!status)
                st_ses->det_stc_ses->pending_stop = true;
        }
        status = handle_load_sm(st_ses, stc_ses);
        break;

    case ST_SES_EV_UNLOAD_SM:
        /* Valid event only in multi-client usecase */
        /*
         * If a detected client deffered stop event is not handled yet,
         * handle here before moving out of detected state
         */
        if (st_ses->det_stc_ses && !st_ses->det_stc_ses->pending_stop) {
            ALOGD("%s:[%d] post deferred stop for client c%d", __func__,
                st_ses->sm_handle, st_ses->det_stc_ses->sm_handle);
            status = hw_session_notifier_enqueue(
               st_ses->det_stc_ses->sm_handle, ST_SES_EV_DEFERRED_STOP,
               ST_SES_DEFERRED_STOP_DELAY_MS);
            if (!status)
                st_ses->det_stc_ses->pending_stop = true;
        }
        status = handle_unload_sm(st_ses, stc_ses);
        break;

    case ST_SES_EV_START:
        /* For multi-client, other loaded client may start */
        STATE_TRANSITION(st_ses, active_state_fn);
        DISPATCH_EVENT(st_ses, *ev, status);
        break;

    case ST_SES_EV_RESTART:
        status = restart_session(st_ses, hw_ses);
        if (status && !st_ses->stdev->ssr_offline_received)
            ALOGE("%s:[%d] failed to start session, err %d", __func__,
                st_ses->sm_handle, status);

        if (st_ses->stdev->ssr_offline_received) {
            stop_session(st_ses, hw_ses, true);
            STATE_TRANSITION(st_ses, ssr_state_fn);
            status = 0;
        } else {
            /* Move anyways to allow client unload */
            STATE_TRANSITION(st_ses, active_state_fn);
        }
        break;

    case ST_SES_EV_PAUSE:
    case ST_SES_EV_STOP:
        /*
         * It is possible that the client can issue stop after detection
         * callback. This even can be issued internally as part of
         * deferred stop as well. For multi-client, it could be current
         * detected  client stop or other client stop.
         */
        STATE_TRANSITION(st_ses, active_state_fn);
        DISPATCH_EVENT(st_ses, *ev, status);
        break;

    case ST_SES_EV_SSR_OFFLINE:
        /*
         * Ignore return status during SSR handling
         * as the ADSP or CPE might be down so these
         * calls would fail. Exec mode can be none if
         * ssr occurs during a transition.
         */
        if (st_ses->exec_mode != ST_EXEC_MODE_NONE)
            stop_session(st_ses, hw_ses, true);
        STATE_TRANSITION(st_ses, ssr_state_fn);
        break;

    case ST_SES_EV_SET_EXEC_MODE:
        new_exec_mode = ev->payload.exec_mode;

        if (new_exec_mode == st_ses->exec_mode) {
            stc_ses->exec_mode = st_ses->exec_mode;
            break;
        }

        if (st_ses->exec_mode != ST_EXEC_MODE_NONE) {
            st_ses->exec_mode = ST_EXEC_MODE_NONE;
            status = stop_session(st_ses, hw_ses, true);
            list_for_each(node, &st_ses->clients_list) {
                c_ses = node_to_item(node, st_session_t, hw_list_node);
                c_ses->exec_mode = new_exec_mode;
            }
            if (status)
                break;
        }

        if (new_exec_mode == ST_EXEC_MODE_NONE)
            break;

        /* switch to new hw session */
        if (ST_EXEC_MODE_CPE == new_exec_mode) {
            new_hw_ses = st_ses->hw_ses_cpe;
        } else if (ST_EXEC_MODE_ADSP == new_exec_mode) {
            new_hw_ses = st_ses->hw_ses_adsp;
        } else {
            ALOGE("%s: unknown execution mode %d", __func__,
                new_exec_mode);
            status = -EINVAL;
            break;
        }

        /*
         * hw session changed to/from WDSP/ADSP, hence update the
         * related config.
         * Not applicable for LPI<->non-LPI transtions as hw session
         * doesn't change.
         */
        status = update_hw_config_on_start(stc_ses, new_hw_ses);
        if (status) {
            ALOGE("%s: Update_hw_config_on_start failed %d",
                __func__, status);
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

        list_for_each(node, &st_ses->clients_list) {
            c_ses = node_to_item(node, st_session_t, hw_list_node);
            c_ses->exec_mode = new_exec_mode;
            if (c_ses->state == ST_STATE_ACTIVE) {
                dereg_hal_event_session(c_ses);
                reg_hal_event_session(c_ses, new_hw_ses);
            }
        }
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
        status = hw_session_notifier_enqueue(stc_ses->sm_handle,
            ST_SES_EV_DEFERRED_STOP, ST_SES_DEFERRED_STOP_DELAY_MS);
        if (!status)
            stc_ses->pending_stop = true;
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

static int buffering_state_fn(st_proxy_session_t *st_ses, st_session_ev_t *ev)
{
    int status = 0;
    st_session_t *stc_ses = ev->stc_ses;
    struct listnode *node = NULL;
    st_session_t *c_ses = NULL;
    st_hw_session_t *hw_ses = st_ses->hw_ses_current;
    st_exec_mode_t new_exec_mode = ST_EXEC_MODE_NONE;
    st_hw_session_t *new_hw_ses = NULL;

    /* skip parameter check as this is an internal function */
    ALOGVV("%s:[c%d-%d] handle event id %d", __func__, stc_ses->sm_handle,
         st_ses->sm_handle, ev->ev_id);

    switch (ev->ev_id) {
    case ST_SES_EV_LOAD_SM:
        /* Valid in multi-client usecase */
        ALOGD("%s:[c%d-%d] load sm", __func__, stc_ses->sm_handle,
            st_ses->sm_handle);

        status = handle_load_sm(st_ses, stc_ses);
        break;

    case ST_SES_EV_UNLOAD_SM:
        ALOGD("%s:[c%d-%d] unload sm", __func__, stc_ses->sm_handle,
            st_ses->sm_handle);

        status = handle_unload_sm(st_ses, stc_ses);
        break;

    case ST_SES_EV_READ_PCM:
        /* Note: this function may block if there is no PCM data ready*/
        hw_ses->fptrs->read_pcm(hw_ses, ev->payload.readpcm.out_buff,
            ev->payload.readpcm.out_buff_size);
        if (st_ses->stdev->enable_debug_dumps) {
            ST_DBG_FILE_WRITE(st_ses->lab_fp, ev->payload.readpcm.out_buff,
                ev->payload.readpcm.out_buff_size);
        }
        break;
    case ST_SES_EV_END_BUFFERING:
        if (stc_ses == st_ses->det_stc_ses) {
            hw_ses->fptrs->stop_buffering(hw_ses);
            if (!stc_ses->pending_stop) {
                ALOGD("%s:[c%d] post deferred stop on buffering end", __func__,
                    stc_ses->sm_handle);
                status = hw_session_notifier_enqueue(stc_ses->sm_handle,
                    ST_SES_EV_DEFERRED_STOP, ST_SES_DEFERRED_STOP_DELAY_MS);
                if (!status)
                    stc_ses->pending_stop = true;
            } else {
                ALOGD("%s:[c%d] skip deferred stop on buffering as already set",
                      __func__, stc_ses->sm_handle);
            }
        }
        break;

    case ST_SES_EV_STOP:
         ALOGD("%s:[c%d-%d] handle event STOP", __func__, stc_ses->sm_handle,
             st_ses->sm_handle);
        if (stc_ses != st_ses->det_stc_ses) {
            ALOGD("%s: c%d buffering, delay c%d stop", __func__,
                st_ses->det_stc_ses->sm_handle, stc_ses->sm_handle);
            update_hw_config_on_stop(st_ses, stc_ses);
            break;
        }
        /* Fall through */
    case ST_SES_EV_PAUSE:
        hw_ses->fptrs->stop_buffering(hw_ses);
        STATE_TRANSITION(st_ses, active_state_fn);
        DISPATCH_EVENT(st_ses, *ev, status);
        if (st_ses->stdev->enable_debug_dumps)
            ST_DBG_FILE_CLOSE(st_ses->lab_fp);
        break;

    case ST_SES_EV_SET_DEVICE:
        ALOGD("%s:[c%d-%d] handle SET_DEVICE", __func__, stc_ses->sm_handle,
            st_ses->sm_handle);
        /*
         * Device switch will not wait for buffering to finish. It will instead
         * interrupt and stop the buffering and transition to the loaded state.
         * The loaded state will then take care of the device switch.
         */
        hw_ses->fptrs->stop_buffering(hw_ses);
        status = stop_session(st_ses, hw_ses, false);
        if (status && !st_ses->stdev->ssr_offline_received) {
            ALOGE("%s:[%d] failed to stop session, err %d", __func__,
                  st_ses->sm_handle, status);
            break;
        }
        STATE_TRANSITION(st_ses, loaded_state_fn);
        DISPATCH_EVENT(st_ses, *ev, status);

        /*
         * set_device event can be dispatched with any one of attached
         * multi-clients. For current detected client, the App may or may
         * not start next detection, so handle the state accordingly.
         */
        if (st_ses->det_stc_ses->pending_stop) {
            ALOGD("%s:[c%d] cancel ST_SES_EV_DEFERRED_STOP", __func__,
                st_ses->det_stc_ses->sm_handle);
            hw_session_notifier_cancel(stc_ses->sm_handle,
                ST_SES_EV_DEFERRED_STOP);
            stc_ses->pending_stop = false;
        }
        st_ses->det_stc_ses->state = ST_STATE_LOADED;
        break;

    case ST_SES_EV_START:
    case ST_SES_EV_RESTART:
        ALOGD("%s:[c%d-%d] handle event START/RESTART", __func__,
            stc_ses->sm_handle, st_ses->sm_handle);

        if (stc_ses != st_ses->det_stc_ses) {
            ALOGD("%s: c%d buffering, delay c%d start", __func__,
                st_ses->det_stc_ses->sm_handle, stc_ses->sm_handle);
            break;
        }
        /*
         * Client starts detection again.
         * This implies a previous deferred stop hasn't completed yet as
         * stop would have changed state to loaded.
         * For a restart event, issue stop buffering and restart the session
         * For a start event, stop buffering then stop and start the session
         * so that any new parameters take effect.
         * For multi-client case while the detected is buffering,
         * the other client stop/start would have been deferred by updating
         * the config, and later when current detected client restarts after
         * buffreing is completed, check if hw config is updated due to other
         * client and stop->start the hw session to apply updated config.
         */
        hw_ses->fptrs->stop_buffering(hw_ses);
        if (hw_ses->sthw_cfg_updated || ev->ev_id == ST_SES_EV_START) {
            status = stop_session(st_ses, hw_ses, false);
            STATE_TRANSITION(st_ses, loaded_state_fn);
            if (status) {
                ALOGE("%s:[%d] failed to stop session, err %d", __func__,
                    st_ses->sm_handle, status);
            } else {
                status = start_session(st_ses, hw_ses, false);
                if (status) {
                    ALOGE("%s:[%d] failed to start session, err %d", __func__,
                        st_ses->sm_handle, status);
                }
            }
        } else {
            status = restart_session(st_ses, hw_ses);
        }

        if (status) {
            if (st_ses->stdev->ssr_offline_received) {
                hw_ses->fptrs->dereg_sm(hw_ses);
                STATE_TRANSITION(st_ses, ssr_state_fn);
                status = 0;
            } else {
                /* move to active anyways to allow unload sm */
                STATE_TRANSITION(st_ses, active_state_fn);
            }
        } else {
            STATE_TRANSITION(st_ses, active_state_fn);
        }
        break;

    case ST_SES_EV_SSR_OFFLINE:
        ALOGD("%s:[c%d-%d] handle event id %d", __func__, stc_ses->sm_handle,
            st_ses->sm_handle, ev->ev_id);
        /*
         *  Ignore return status during SSR handling
         *  as the ADSP or CPE might be down so these
         *  calls would fail. Exec mode can be none if
         *  ssr occurs during a transition.
         */
        if (st_ses->exec_mode != ST_EXEC_MODE_NONE) {
            hw_ses->fptrs->stop_buffering(hw_ses);
            stop_session(st_ses, hw_ses, true);
        }
        STATE_TRANSITION(st_ses, ssr_state_fn);
        break;

    case ST_SES_EV_SET_EXEC_MODE:
        ALOGD("%s:[c%d-%d] handle event id %d", __func__, stc_ses->sm_handle,
            st_ses->sm_handle, ev->ev_id);

        new_exec_mode = ev->payload.exec_mode;
        if (new_exec_mode == st_ses->exec_mode) {
            stc_ses->exec_mode = st_ses->exec_mode;
            break;
        }

        if (st_ses->exec_mode != ST_EXEC_MODE_NONE) {
            st_ses->exec_mode = ST_EXEC_MODE_NONE;
            status = hw_ses->fptrs->stop_buffering(hw_ses);
            if (status) {
                ALOGE("%s:[%d] failed to stop_buffering err %d", __func__,
                    st_ses->sm_handle, status);
                break;
            }
            status = stop_session(st_ses, hw_ses, true);
            list_for_each(node, &st_ses->clients_list) {
                c_ses = node_to_item(node, st_session_t, hw_list_node);
                c_ses->exec_mode = new_exec_mode;
            }
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
        } else if (ST_EXEC_MODE_ADSP == new_exec_mode) {
            new_hw_ses = st_ses->hw_ses_adsp;
        } else {
            ALOGE("%s: unknown execution mode %d", __func__,
                new_exec_mode);
            status = -EINVAL;
            break;
        }
        /*
         * hw session changed to/from WDSP/ADSP, hence update the
         * related config.
         * Not applicable for LPI<->non-LPI transtions as hw session
         * doesn't change.
         */
        status = update_hw_config_on_start(stc_ses, new_hw_ses);
        if (status) {
            ALOGE("%s: Update_hw_config_on_start failed %d",
                __func__, status);
            break;
        }
        status = start_session(st_ses, new_hw_ses, true);
        if (status) {
            ALOGE("%s:[%d] failed to start hw ses, err %d", __func__,
                st_ses->sm_handle, status);
            break;
        }
        list_for_each(node, &st_ses->clients_list) {
            c_ses = node_to_item(node, st_session_t, hw_list_node);
            c_ses->exec_mode = new_exec_mode;
            if (c_ses->state == ST_STATE_ACTIVE) {
                dereg_hal_event_session(c_ses);
                reg_hal_event_session(c_ses, new_hw_ses);
            }
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

static int ssr_state_fn(st_proxy_session_t *st_ses, st_session_ev_t *ev)
{
    int status = 0;
    st_session_t *stc_ses = ev->stc_ses;
    st_session_ev_t load_ev =  {.ev_id = ST_SES_EV_LOAD_SM};
    st_session_ev_t start_ev =  {.ev_id = ST_SES_EV_START};
    st_session_ev_t exec_mode_ev =  {.ev_id = ST_SES_EV_SET_EXEC_MODE};
    bool active = false;

    /* skip parameter check as this is an internal function */
    ALOGD("%s:[c%d-%d] handle event id %d", __func__, stc_ses->sm_handle,
        st_ses->sm_handle, ev->ev_id);

    switch (ev->ev_id) {
    case ST_SES_EV_SSR_ONLINE:
        ALOGV("%s:[%d] SSR ONLINE received", __func__, st_ses->sm_handle);
        /*
         * Load and start all clients at once instead of unload/loading
         * due to each subsequent client dispatch. It is expected the
         * upper layer calls SSR_ONLINE for all clients.
         */
        stc_ses->pending_load = true;
        if (is_any_client_not_pending_load(st_ses))
            break;

        reset_clients_pending_load(st_ses);

        STATE_TRANSITION(st_ses, idle_state_fn);

        if ((stc_ses->ssr_transit_exec_mode == ST_EXEC_MODE_CPE) ||
            (stc_ses->ssr_transit_exec_mode == ST_EXEC_MODE_ADSP)) {
            exec_mode_ev.stc_ses = stc_ses;
            exec_mode_ev.payload.exec_mode = stc_ses->ssr_transit_exec_mode;
            DISPATCH_EVENT(st_ses, exec_mode_ev, status);
            if (status)
                break;
            stc_ses->ssr_transit_exec_mode = ST_EXEC_MODE_NONE;
        }
        active = is_any_client_in_state(st_ses, ST_STATE_ACTIVE);
        if (active || is_any_client_in_state(st_ses, ST_STATE_LOADED)) {
            load_ev.stc_ses = stc_ses;
            DISPATCH_EVENT(st_ses, load_ev, status);
            if (status)
                break;
        }
        if (active) {
            start_ev.stc_ses = stc_ses;
            DISPATCH_EVENT(st_ses, start_ev, status);
        }

        break;

    case ST_SES_EV_LOAD_SM:
        if (ST_STATE_IDLE == stc_ses->state) {
            status = update_sound_model(stc_ses, true);
            if (status) {
                ALOGE("%s:[c%d] update sound model add failed %d", __func__,
                    stc_ses->sm_handle, status);
                status = -EINVAL;
                break;
            }
            stc_ses->state = ST_STATE_LOADED;
        } else {
            ALOGE("%s: received unexpected event, client state = %d",
                __func__, stc_ses->state);
        }
        break;

    case ST_SES_EV_UNLOAD_SM:
        if (ST_STATE_LOADED == stc_ses->state) {
            status = update_sound_model(stc_ses, false);
            if (status)
                ALOGE("%s:[c%d] update sound_model failed %d", __func__,
                    stc_ses->sm_handle, status);
            stc_ses->state = ST_STATE_IDLE;
        } else {
            ALOGE("%s: received unexpected event, client state = %d",
                __func__, stc_ses->state);
        }
        break;

    case ST_SES_EV_START:
    case ST_SES_EV_RESTART:
        if (ST_STATE_LOADED == stc_ses->state) {
            if (ev->ev_id == ST_SES_EV_RESTART)
                update_hw_config_on_restart(st_ses, stc_ses);
            stc_ses->state = ST_STATE_ACTIVE;
        } else {
            ALOGE("%s: received unexpected event, client state = %d",
                __func__, stc_ses->state);
        }
        break;

    case ST_SES_EV_STOP:
        if (ST_STATE_ACTIVE == stc_ses->state) {
            update_hw_config_on_stop(st_ses, stc_ses);
            stc_ses->state = ST_STATE_LOADED;
        } else {
            ALOGE("%s: received unexpected event, client state = %d",
                __func__, stc_ses->state);
        }
        break;

    case ST_SES_EV_PAUSE:
        stc_ses->paused = true;
        break;

    case ST_SES_EV_RESUME:
        stc_ses->paused = false;
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
        stc_ses->exec_mode = ev->payload.exec_mode;
        if (ev->payload.exec_mode == st_ses->exec_mode)
            break;

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

int st_session_load_sm(st_session_t *stc_ses)
{
    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_session_loadsm_payload_t payload = { .phrase_sm = stc_ses->phrase_sm };
    st_session_ev_t ev = { .ev_id = ST_SES_EV_LOAD_SM,
        .payload.loadsm = payload, .stc_ses = stc_ses };

    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    if (!status) {
        prepare_second_stage_for_client(stc_ses);
        stc_ses->state = ST_STATE_LOADED;
    }
    pthread_mutex_unlock(&st_ses->lock);

    return status;
}

int st_session_unload_sm(st_session_t *stc_ses)
{
    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_session_ev_t ev = { .ev_id = ST_SES_EV_UNLOAD_SM, .stc_ses = stc_ses };

    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    stop_second_stage_for_client(stc_ses);
    stc_ses->state = ST_STATE_IDLE;
    pthread_mutex_unlock(&st_ses->lock);

    return status;
}

int st_session_start(st_session_t *stc_ses)
{
    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_session_ev_t ev = { .ev_id = ST_SES_EV_START, .stc_ses = stc_ses };

    pthread_mutex_lock(&st_ses->lock);
    if (stc_ses->pending_stop) {
        ALOGV("%s:[c%d] cancel ST_SES_EV_DEFERRED_STOP", __func__,
            stc_ses->sm_handle);
        hw_session_notifier_cancel(stc_ses->sm_handle, ST_SES_EV_DEFERRED_STOP);
        stc_ses->pending_stop = false;
    }

    DISPATCH_EVENT(st_ses, ev, status);
    if (!status) {
        reg_hal_event_session(stc_ses, st_ses->hw_ses_current);
        start_second_stage_for_client(stc_ses);
        stc_ses->state = ST_STATE_ACTIVE;
    }
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_stop(st_session_t *stc_ses)
{
    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_session_ev_t ev = { .ev_id = ST_SES_EV_STOP, .stc_ses = stc_ses };

    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    dereg_hal_event_session(stc_ses);
    stc_ses->pending_stop = false;
    stc_ses->state = ST_STATE_LOADED;
    pthread_mutex_unlock(&st_ses->lock);

    return status;
}

int st_session_restart(st_session_t *stc_ses)
{
    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_session_ev_t ev = { .ev_id = ST_SES_EV_RESTART, .stc_ses = stc_ses };

    pthread_mutex_lock(&st_ses->lock);
    if (stc_ses->pending_stop) {
        ALOGV("%s:[c%d] cancel ST_SES_EV_DEFERRED_STOP", __func__,
            stc_ses->sm_handle);
        hw_session_notifier_cancel(stc_ses->sm_handle, ST_SES_EV_DEFERRED_STOP);
        stc_ses->pending_stop = false;
    }

    DISPATCH_EVENT(st_ses, ev, status);
    if (!status) {
        start_second_stage_for_client(stc_ses);
        stc_ses->state = ST_STATE_ACTIVE;
    } else {
        dereg_hal_event_session(stc_ses);
        stc_ses->state = ST_STATE_LOADED;
    }
    pthread_mutex_unlock(&st_ses->lock);

    return status;
}

int st_session_ssr_offline(st_session_t *stc_ses,
    enum ssr_event_status ssr_type)
{
    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_session_ev_t ev = { .ev_id = ST_SES_EV_SSR_OFFLINE,
        .payload.ssr = ssr_type, .stc_ses = stc_ses };

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
         (((ST_EXEC_MODE_CPE == stc_ses->ssr_transit_exec_mode) &&
           (SND_CARD_STATUS_OFFLINE == ssr_type)) ||
          ((ST_EXEC_MODE_ADSP == stc_ses->ssr_transit_exec_mode) &&
           (CPE_STATUS_OFFLINE == ssr_type)))) ||
        ((ST_EXEC_MODE_CPE == st_ses->exec_mode) &&
         (SND_CARD_STATUS_OFFLINE == ssr_type) &&
         (st_ses->stdev->bg_kwd)))
        DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);

    return status;
}

int st_session_ssr_online(st_session_t *stc_ses,
    enum ssr_event_status ssr_type)
{
    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_session_ev_t ev = { .ev_id = ST_SES_EV_SSR_ONLINE,
        .payload.ssr = ssr_type, .stc_ses = stc_ses };

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
         (((ST_EXEC_MODE_CPE == stc_ses->ssr_transit_exec_mode) &&
           (SND_CARD_STATUS_ONLINE == ssr_type)) ||
          ((ST_EXEC_MODE_ADSP == stc_ses->ssr_transit_exec_mode) &&
           (CPE_STATUS_ONLINE == ssr_type)))) ||
        ((ST_EXEC_MODE_CPE == st_ses->exec_mode) &&
         (SND_CARD_STATUS_ONLINE == ssr_type) &&
         (st_ses->stdev->bg_kwd)))
        DISPATCH_EVENT(st_ses, ev, status);

    pthread_mutex_unlock(&st_ses->lock);

    return status;
}

int st_session_pause(st_session_t *stc_ses)
{
    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_session_ev_t ev = { .ev_id = ST_SES_EV_PAUSE, .stc_ses = stc_ses };

    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_resume(st_session_t *stc_ses)
{
    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_session_ev_t ev = { .ev_id = ST_SES_EV_RESUME, .stc_ses = stc_ses };

    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_disable_device(st_session_t *stc_ses)
{
    int status = 0;
    st_session_event_id_t ev_id = ST_SES_EV_SET_DEVICE;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    pthread_mutex_lock(&st_ses->lock);
    if (check_gcs_usecase_switch(stc_ses->hw_proxy_ses))
        ev_id = ST_SES_EV_PAUSE;

    st_session_ev_t ev = {.ev_id = ev_id,
        .payload.enable = false, .stc_ses = stc_ses};

    /*
     * Avoid dispatching for each attached multi-client, instead
     * defer it until last client
     */
    stc_ses->pending_set_device = true;
    if (is_any_client_not_pending_set_device(st_ses)) {
        pthread_mutex_unlock(&st_ses->lock);
        return status;
    }
    reset_clients_pending_set_device(st_ses);

    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_enable_device(st_session_t *stc_ses)
{
    int status = 0;
    st_session_event_id_t ev_id = ST_SES_EV_SET_DEVICE;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    pthread_mutex_lock(&st_ses->lock);
    if (check_gcs_usecase_switch(stc_ses->hw_proxy_ses))
        ev_id = ST_SES_EV_RESUME;

    st_session_ev_t ev = { .ev_id = ev_id,
        .payload.enable = true, .stc_ses = stc_ses };

    /*
     * Avoid dispatching for each attached multi-client, instead
     * defer it until last client
     */
    stc_ses->pending_set_device = true;
    if (is_any_client_not_pending_set_device(st_ses)) {
        pthread_mutex_unlock(&st_ses->lock);
        return status;
    }
    reset_clients_pending_set_device(st_ses);

    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

bool st_session_is_detected(st_session_t *stc_ses)
{
    bool ret = false;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return ret;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;

    pthread_mutex_lock(&st_ses->lock);
    ret = (st_ses->current_state == detected_state_fn) ? true : false;
    pthread_mutex_unlock(&st_ses->lock);

    return ret;
}

bool st_session_is_active(st_session_t *stc_ses)
{
    bool ret = false;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return ret;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;

    pthread_mutex_lock(&st_ses->lock);
    ret = (st_ses->current_state == active_state_fn) ? true : false;
    pthread_mutex_unlock(&st_ses->lock);

    return ret;
}

bool st_session_is_buffering(st_session_t *stc_ses)
{
    bool ret = false;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return ret;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;

    pthread_mutex_lock(&st_ses->lock);
    ret = (st_ses->current_state == buffering_state_fn) ? true : false;
    pthread_mutex_unlock(&st_ses->lock);

    return ret;
}

bool st_session_is_ssr_state(st_session_t *stc_ses)
{
    bool ret = false;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return ret;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;

    pthread_mutex_lock(&st_ses->lock);
    ret = (st_ses->current_state == ssr_state_fn) ? true : false;
    pthread_mutex_unlock(&st_ses->lock);

    return ret;
}

int st_session_read_pcm(st_session_t *stc_ses, uint8_t *buff,
    size_t buff_size, size_t *read_size)
{
    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses || !buff || buff_size == 0 ||
        read_size == 0)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_session_readpcm_payload_t payload = {.out_buff = buff,
        .out_buff_size = buff_size, .actual_read_size = read_size};

    st_session_ev_t ev = {.ev_id = ST_SES_EV_READ_PCM,
        .payload.readpcm = payload, .stc_ses = stc_ses};

    /*
     * Do not lock when handling this event, this event
     * can go in parallel with other events as multiple
     * sessions can buffer in parallel.
     */
    DISPATCH_EVENT(st_ses, ev, status);
    return status;
}

int st_session_stop_lab(st_session_t *stc_ses)
{
    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_session_ev_t ev = {.ev_id = ST_SES_EV_END_BUFFERING, .stc_ses = stc_ses};

    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_set_exec_mode(st_session_t *stc_ses, st_exec_mode_t exec)
{
    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    ALOGV("%s: exec mode %d", __func__, exec);

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_session_ev_t ev = {.ev_id = ST_SES_EV_SET_EXEC_MODE,
        .payload.exec_mode = exec, .stc_ses = stc_ses};

    pthread_mutex_lock(&st_ses->lock);
    if (st_ses->enable_trans)
        DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_update_recongition_config(st_session_t *stc_ses)
{

    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;

    pthread_mutex_lock(&st_ses->lock);
    status = update_hw_config_on_start(stc_ses, st_ses->hw_ses_current);
    pthread_mutex_unlock(&st_ses->lock);
    return status;

}

int st_session_get_preroll(st_session_t *stc_ses)
{
    int val = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return 0;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;

    pthread_mutex_lock(&st_ses->lock);
    val = st_ses->hw_ses_current->sthw_cfg.client_req_preroll;
    pthread_mutex_unlock(&st_ses->lock);

    return val;
}

int st_session_send_custom_chmix_coeff(st_session_t *stc_ses, char *str)
{
    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_session_ev_t ev = { .ev_id = ST_SES_EV_SEND_CHMIX_COEFF,
        .payload.chmix_coeff_str = str, .stc_ses = stc_ses};

    pthread_mutex_lock(&st_ses->lock);
    if (ST_EXEC_MODE_ADSP == st_ses->exec_mode)
        DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_get_config(st_session_t *stc_ses, struct pcm_config *config)
{
    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;

    pthread_mutex_lock(&st_ses->lock);
    memcpy(config, &st_ses->hw_ses_current->config, sizeof(struct pcm_config));
    pthread_mutex_unlock(&st_ses->lock);

    return 0;
}

int st_session_get_param_data(st_session_t *stc_ses, const char *param,
    void *payload, size_t payload_size, size_t *param_data_size)
{
    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_session_getparam_payload_t getparam_payload = { .param = param,
        .payload = payload,
        .payload_size = payload_size,
        .param_data_size = param_data_size};
    st_session_ev_t ev = { .ev_id = ST_SES_EV_GET_PARAM_DATA,
        .payload.getparam = getparam_payload, .stc_ses = stc_ses};

    pthread_mutex_lock(&st_ses->lock);
    /* Currently get param data supported for ARM & ADSP mode */
    if ((ST_EXEC_MODE_ARM == st_ses->exec_mode) ||
        (ST_EXEC_MODE_ADSP == st_ses->exec_mode))
        DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);

    return status;
}

int st_session_ss_init(st_session_t *stc_ses)
{
    int status = 0;
    struct listnode *node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;
    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;

    list_for_each(node, &stc_ses->second_stage_list) {
        st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
        status = st_second_stage_module_init(st_sec_stage,
            (void *)st_sec_stage->ss_info->lib_name);
        if (status) {
            ALOGE("%s:[c%d] initializing second stage session failed %d",
                __func__, stc_ses->sm_handle, status);
            goto ss_cleanup;
        }
    }

    pthread_mutex_lock(&st_ses->lock);
    if (st_ses->aggregator_thread_created) {
        pthread_mutex_unlock(&st_ses->lock);
        return 0;
    }
    /*
     * Aggregator is not maintatined per client as there is only one
     * client keyword detection happens at a time in multi-client scenario.
     * Instead use single aggregator thread at proxy level, processing the
     * second stage for detected client at run time.
     */
    init_det_event_aggregator(st_ses);

    st_ses->det_session_ev = calloc(1, sizeof(st_session_ev_t));
    if (!st_ses->det_session_ev) {
        ALOGE("%s: Failed to allocate st_session_ev_t, exiting", __func__);
        status = -ENOMEM;
        goto ss_cleanup;
    }
    pthread_mutex_unlock(&st_ses->lock);
    return 0;

ss_cleanup:
    destroy_det_event_aggregator(st_ses);
    list_for_each(node, &stc_ses->second_stage_list) {
        st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
        st_second_stage_module_deinit(st_sec_stage);
    }
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_ss_deinit(st_session_t *stc_ses)
{
    struct listnode *node = NULL;
    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_arm_second_stage_t *st_sec_stage = NULL;
    st_session_t *c_ses = NULL;
    bool aggregator_needed = false;

    list_for_each(node, &stc_ses->second_stage_list) {
        st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
        st_second_stage_module_deinit(st_sec_stage);
    }

    pthread_mutex_lock(&st_ses->lock);
    if (!st_ses->aggregator_thread_created) {
        pthread_mutex_unlock(&st_ses->lock);
        return 0;
    }
    /* If other client has second stage enabled, keep the aggregator */
    list_for_each(node, &st_ses->clients_list) {
        c_ses = node_to_item(node, st_session_t, hw_list_node);
        if (c_ses != stc_ses && !list_empty(&c_ses->second_stage_list)) {
            aggregator_needed = true;
            break;
        }
    }
    if (aggregator_needed) {
        pthread_mutex_unlock(&st_ses->lock);
        return 0;
    }

    destroy_det_event_aggregator(st_ses);

    if (st_ses->det_session_ev)
        free(st_ses->det_session_ev);
    pthread_mutex_unlock(&st_ses->lock);

    return 0;
}

int st_session_request_detection(st_session_t *stc_ses)
{
    int status = 0;

    if (!stc_ses || !stc_ses->hw_proxy_ses)
        return -EINVAL;

    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;
    st_session_ev_t ev = {.ev_id = ST_SES_EV_REQUEST_DET, .stc_ses = stc_ses};

    /* lock to serialize event handling */
    pthread_mutex_lock(&st_ses->lock);
    DISPATCH_EVENT(st_ses, ev, status);
    pthread_mutex_unlock(&st_ses->lock);
    return status;
}

int st_session_init(st_session_t *stc_ses, struct sound_trigger_device *stdev,
    st_exec_mode_t exec_mode, sound_model_handle_t sm_handle)
{
    int status = 0;
    struct st_vendor_info *v_info = NULL;
    st_proxy_session_t *st_ses = NULL;
    struct listnode *node = NULL;
    struct st_session *c_ses = NULL;
    pthread_mutexattr_t attr;

    if (!stc_ses || !stdev)
        return -EINVAL;

    st_ses = calloc(1, sizeof(st_proxy_session_t));
    if (!st_ses) {
        ALOGE("%s: hw_proxy_ses allocation failed", __func__);
        return -ENOMEM;
    }
    st_ses->stdev = stc_ses->stdev = stdev;
    v_info = stc_ses->vendor_uuid_info;

    if (v_info && (EXEC_MODE_CFG_DYNAMIC == v_info->exec_mode_cfg)) {
        st_ses->enable_trans = true;
        if (stdev->is_gcs) {
            /* alloc and init cpe session*/
            st_ses->hw_ses_cpe =
                (st_hw_session_t *)calloc(1, sizeof(st_hw_session_gcs_t));
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
            st_ses->hw_ses_adsp =
                (st_hw_session_t *)calloc(1, sizeof(st_hw_session_lsm_t));
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
            st_ses->hw_ses_cpe =
                (st_hw_session_t *)calloc(1, sizeof(st_hw_session_lsm_t));
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
            st_ses->hw_ses_adsp =
                (st_hw_session_t *)calloc(1, sizeof(st_hw_session_lsm_t));
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
            st_ses->hw_ses_cpe =
                (st_hw_session_t *)calloc(1, sizeof(st_hw_session_gcs_t));
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
            st_ses->hw_ses_cpe =
                (st_hw_session_t *)calloc(1, sizeof(st_hw_session_lsm_t));
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
        /*
         * Check for merge sound model support and return the existing hw
         * session. If any other clients have already created it.
         */
        if (v_info->merge_fs_soundmodels) {
            if (!v_info->is_qcva_uuid) {
               ALOGE("%s: merge sound model not supported for non SVA engines",
                     __func__);
               status = -ENOSYS;
               goto cleanup;
            }
            list_for_each(node, &stdev->sound_model_list) {
                c_ses = node_to_item(node, st_session_t, list_node);
                if ((c_ses != stc_ses) &&
                    c_ses->vendor_uuid_info->is_qcva_uuid &&
                    c_ses->vendor_uuid_info->merge_fs_soundmodels) {
                    stc_ses->hw_proxy_ses = c_ses->hw_proxy_ses;
                    list_add_tail(&stc_ses->hw_proxy_ses->clients_list,
                        &stc_ses->hw_list_node);
                    ALOGD("%s: another client attached: h%d <-- c%d", __func__,
                        stc_ses->hw_proxy_ses->sm_handle, sm_handle);
                    free(st_ses);
                    st_ses = NULL;
                    break;
                }
            }
         }
         if (st_ses) { /* If no other client exist */
             st_ses->hw_ses_adsp =
                 (st_hw_session_t *)calloc(1, sizeof(st_hw_session_lsm_t));
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
         }
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
        st_ses->hw_ses_cpe =
            (st_hw_session_t *)calloc(1, sizeof(st_hw_session_lsm_t));
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
    pthread_mutex_init(&stc_ses->lock, (const pthread_mutexattr_t *)&attr);

    stc_ses->exec_mode = exec_mode;
    stc_ses->sm_handle = sm_handle;
    stc_ses->ssr_transit_exec_mode = ST_EXEC_MODE_NONE;
    stc_ses->client_req_det_mode = ST_DET_UNKNOWN_MODE;
    stc_ses->state = ST_STATE_IDLE;

    if (st_ses) { /* Could get freed if other client exists */
        st_ses->vendor_uuid_info = v_info;
        st_ses->exec_mode = exec_mode;
        st_ses->sm_handle = sm_handle;
        st_ses->lab_fp = NULL;
        pthread_mutex_init(&st_ses->lock, (const pthread_mutexattr_t *)&attr);

        stc_ses->hw_proxy_ses = st_ses;
        list_init(&st_ses->clients_list);
        list_add_tail(&st_ses->clients_list, &stc_ses->hw_list_node);
        ALOGD("%s: client attached: h%d <-- c%d", __func__,
            st_ses->sm_handle, sm_handle);

        if (!stdev->ssr_offline_received) {
            STATE_TRANSITION(st_ses, idle_state_fn);
        } else {
            STATE_TRANSITION(st_ses, ssr_state_fn);
            status = 0;
        }
    }
    return status;

cleanup:
    if (st_ses) {
        if (st_ses->hw_ses_cpe)
            free(st_ses->hw_ses_cpe);
        if (st_ses->hw_ses_adsp)
            free(st_ses->hw_ses_adsp);
        free(st_ses);
    }
    return status;
}

int st_session_deinit(st_session_t *stc_ses)
{
    st_proxy_session_t *st_ses = stc_ses->hw_proxy_ses;

    pthread_mutex_lock(&st_ses->lock);
    list_remove(&stc_ses->hw_list_node);
    ALOGV("%s: client detatched: h%d <-- c%d", __func__, st_ses->sm_handle,
        stc_ses->sm_handle);
    if (stc_ses == st_ses->det_stc_ses)
        st_ses->det_stc_ses = NULL;

    if (!list_empty(&st_ses->clients_list)) {
        pthread_mutex_unlock(&st_ses->lock);
        return 0;
    }
    /* deinit cpe session */
    if (st_ses->hw_ses_cpe) {
        if (st_ses->stdev->is_gcs)
            st_hw_sess_gcs_deinit(st_ses->hw_ses_cpe);
        else
            st_hw_sess_lsm_deinit(st_ses->hw_ses_cpe);
        free(st_ses->hw_ses_cpe);
        st_ses->hw_ses_cpe = NULL;
    }
    /* deinit adsp session */
    if (st_ses->hw_ses_adsp) {
        st_hw_sess_lsm_deinit(st_ses->hw_ses_adsp);
        free(st_ses->hw_ses_adsp);
        st_ses->hw_ses_adsp = NULL;
    }
    pthread_mutex_unlock(&st_ses->lock);
    pthread_mutex_destroy(&st_ses->lock);
    free(stc_ses->hw_proxy_ses);
    stc_ses->hw_proxy_ses = NULL;

    return 0;
}
