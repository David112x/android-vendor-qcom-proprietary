/* st_hw_common.c
 *
 * This library contains common functionality between
 * sound trigger hw and sound trigger extension interface.

 * Copyright (c) 2016, 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
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
    if (!st_ses || !st_ses->hw_ses_adsp || !st_hw_check_multi_stage_lsm_support())
        return false;

    stdev = st_ses->stdev;
    list_for_each(node, &stdev->sound_model_list) {
        p_ses = node_to_item(node, st_session_t, list_node);
        if (p_ses == NULL || p_ses == st_ses || p_ses->exec_mode != ST_EXEC_MODE_ADSP)
            continue;
        if (p_ses->hw_ses_adsp && !list_empty(&p_ses->hw_ses_adsp->lsm_ss_cfg_list))
            lsm_ss_uc_count++;
        if (lsm_ss_uc_count >= max_lsm_ss_uc_count) {
            ALOGD("%s: max supported ss usecase count(%d) already active, not allowing further",
                  __func__, max_lsm_ss_uc_count);
            return false;
        }
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

bool st_hw_check_lpi_support
(
    struct sound_trigger_device *stdev,
    st_session_t *st_ses
)
{
    st_session_t *ses = NULL;
    struct listnode *ses_node = NULL;

    /*
     * ST_PLATFORM_LPI_NONE is used for backward compatibility. With this
     * setting, the st_vendor_uuid->lpi_enable flag will be used.
     */
    if (stdev->platform_lpi_enable == ST_PLATFORM_LPI_DISABLE) {
        ALOGD("%s: lpi NOT enabled in platform setting", __func__);
        return false;
    } else if ((stdev->platform_lpi_enable == ST_PLATFORM_LPI_NONE) &&
               st_ses && !st_ses->vendor_uuid_info->lpi_enable) {
        ALOGD("%s: lpi NOT enabled for ses %d", __func__, st_ses->sm_handle);
        return false;
    }

    if (stdev->rx_concurrency_active || stdev->tx_concurrency_active) {
        ALOGD("%s: lpi NOT supported due to concurrency", __func__);
        return false;
    }

    if (stdev->is_charging &&
        stdev->transit_to_non_lpi_on_battery_charging) {
        ALOGD("%s: lpi NOT supported. battery status %d", __func__,
            stdev->is_charging);
        return false;
    }

    list_for_each(ses_node, &stdev->sound_model_list) {
        ses = node_to_item(ses_node, st_session_t, list_node);

        if (ses->hw_ses_current->client_req_det_mode ==
            ST_HW_SESS_DET_HIGH_PERF_MODE) {
            ALOGD("%s:[%d] lpi NOT supported due to high perf mode", __func__,
                ses->sm_handle);
            return false;
        }
    }

    if (stdev->platform_lpi_enable == ST_PLATFORM_LPI_NONE)
        return is_projected_lpi_budget_available(stdev, st_ses);
    else
        return true;
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

void st_hw_check_and_set_lpi_mode(st_session_t *st_ses)
{
    if (st_ses && st_ses->hw_ses_adsp) {
        if (st_ses->stdev->platform_lpi_enable == ST_PLATFORM_LPI_NONE) {
            st_ses->hw_ses_adsp->lpi_enable =
                (st_ses->vendor_uuid_info->lpi_enable &&
                is_projected_lpi_budget_available(st_ses->stdev, st_ses));
        } else {
            st_ses->hw_ses_adsp->lpi_enable =
                (st_ses->stdev->platform_lpi_enable ==
                 ST_PLATFORM_LPI_ENABLE) ? true: false;
        }
    }
}

static int parse_config_key_conf_levels
(
    st_session_t *st_ses,
    st_hw_session_t* st_hw_ses,
    void *opaque_conf_levels
)
{
    struct st_confidence_levels_info *conf_levels = NULL;
    struct st_confidence_levels_info_v2 *conf_levels_v2 = NULL;
    struct st_sound_model_conf_levels *sm_levels = NULL;
    struct st_sound_model_conf_levels_v2 *sm_levels_v2 = NULL;
    struct listnode *node = NULL, *tmp_node = NULL;
    st_lsm_ss_config_t *ss_cfg = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;
    int status = 0;
    uint32_t i = 0;
    bool gmm_conf_found = false;
    uint8_t confidence_level = 0;
    int32_t confidence_level_v2 = 0;
    bool arm_second_stage = st_hw_ses->enable_second_stage;
    bool adsp_second_stage = (st_hw_ses == st_ses->hw_ses_adsp &&
                              !list_empty(&st_hw_ses->lsm_ss_cfg_list));

    if (arm_second_stage || adsp_second_stage) {
        if (st_ses->rc_config->num_phrases > 1) {
            ALOGE("%s: Multi keyword is unsupported with 2nd stage detection",
                  __func__);
            return -EINVAL;
        }

        if (st_ses->rc_config->phrases[0].num_levels > 1) {
            ALOGE("%s: Multi user is unsupported with 2nd stage detection",
                  __func__);
            return -EINVAL;
        }
    }

    if (st_ses->conf_levels_intf_version != CONF_LEVELS_INTF_VERSION_0002) {
        conf_levels = (struct st_confidence_levels_info *)
            ((char *)opaque_conf_levels + sizeof(struct st_param_header));
        st_hw_ses->conf_levels_info =
            calloc(1, sizeof(struct st_confidence_levels_info));
        if (!st_hw_ses->conf_levels_info) {
            ALOGE("%s: failed to alloc conf_levels_info", __func__);
            return -ENOMEM;
        }
        memcpy(st_hw_ses->conf_levels_info, (char *)conf_levels,
            sizeof(struct st_confidence_levels_info));

        for (i = 0; i < conf_levels->num_sound_models; i++) {
            sm_levels = (struct st_sound_model_conf_levels *)
                &conf_levels->conf_levels[i];
            if (sm_levels->sm_id == ST_SM_ID_SVA_GMM) {
                if ((st_ses->stdev->is_gcs) && (st_hw_ses == st_ses->hw_ses_cpe))
                    status =
                        generate_sound_trigger_recognition_config_payload_v2(
                        (void *)sm_levels, &st_hw_ses->conf_levels,
                        &st_hw_ses->num_conf_levels,
                        st_ses->conf_levels_intf_version);
                else
                    status =
                        generate_sound_trigger_recognition_config_payload(
                        (void *)sm_levels, &st_hw_ses->conf_levels,
                        &st_hw_ses->num_conf_levels,
                        st_ses->conf_levels_intf_version);
                gmm_conf_found = true;
            } else if ((sm_levels->sm_id == ST_SM_ID_SVA_CNN) ||
                       (sm_levels->sm_id == ST_SM_ID_SVA_VOP)) {
                confidence_level = (sm_levels->sm_id == ST_SM_ID_SVA_CNN) ?
                    sm_levels->kw_levels[0].kw_level:
                    sm_levels->kw_levels[0].user_levels[0].level;
                if (arm_second_stage) {
                    list_for_each_safe(node, tmp_node,
                        st_hw_ses->second_stage_list) {
                        st_sec_stage = node_to_item(node, st_arm_second_stage_t,
                            list_node);
                        if (st_sec_stage->ss_info->sm_id == sm_levels->sm_id)
                            st_sec_stage->ss_session->confidence_threshold =
                                confidence_level;
                    }
                } else if (adsp_second_stage) {
                    list_for_each_safe(node, tmp_node,
                        &st_hw_ses->lsm_ss_cfg_list) {
                        ss_cfg = node_to_item(node, st_lsm_ss_config_t,
                            list_node);
                        if (ss_cfg->ss_info->sm_id == sm_levels->sm_id)
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
        st_hw_ses->conf_levels_info =
            calloc(1, sizeof(struct st_confidence_levels_info_v2));
        if (!st_hw_ses->conf_levels_info) {
            ALOGE("%s: failed to alloc conf_levels_info", __func__);
            return -ENOMEM;
        }
        memcpy(st_hw_ses->conf_levels_info, (char *)conf_levels_v2,
            sizeof(struct st_confidence_levels_info_v2));

        for (i = 0; i < conf_levels_v2->num_sound_models; i++) {
            sm_levels_v2 = (struct st_sound_model_conf_levels_v2 *)
                &conf_levels_v2->conf_levels[i];
            if (sm_levels_v2->sm_id == ST_SM_ID_SVA_GMM) {
                if ((st_ses->stdev->is_gcs) &&
                    (st_hw_ses == st_ses->hw_ses_cpe))
                    status =
                        generate_sound_trigger_recognition_config_payload_v2(
                        (void *)sm_levels_v2, &st_hw_ses->conf_levels,
                        &st_hw_ses->num_conf_levels,
                        st_ses->conf_levels_intf_version);
                else
                    status =
                        generate_sound_trigger_recognition_config_payload(
                        (void *)sm_levels_v2, &st_hw_ses->conf_levels,
                        &st_hw_ses->num_conf_levels,
                        st_ses->conf_levels_intf_version);
                gmm_conf_found = true;
            } else if ((sm_levels_v2->sm_id == ST_SM_ID_SVA_CNN) ||
                       (sm_levels_v2->sm_id == ST_SM_ID_SVA_VOP)) {
                confidence_level_v2 =
                    (sm_levels_v2->sm_id == ST_SM_ID_SVA_CNN) ?
                    sm_levels_v2->kw_levels[0].kw_level:
                    sm_levels_v2->kw_levels[0].user_levels[0].level;
                if (arm_second_stage) {
                    list_for_each_safe(node, tmp_node,
                        st_hw_ses->second_stage_list) {
                        st_sec_stage = node_to_item(node, st_arm_second_stage_t,
                            list_node);
                        if (st_sec_stage->ss_info->sm_id ==
                            sm_levels_v2->sm_id)
                            st_sec_stage->ss_session->confidence_threshold =
                                confidence_level_v2;
                    }
                } else if (adsp_second_stage) {
                    list_for_each_safe(node, tmp_node,
                        &st_hw_ses->lsm_ss_cfg_list) {
                        ss_cfg = node_to_item(node, st_lsm_ss_config_t,
                            list_node);
                        if (ss_cfg->ss_info->sm_id == sm_levels_v2->sm_id)
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

    if (status && st_hw_ses->conf_levels_info) {
        free(st_hw_ses->conf_levels_info);
        st_hw_ses->conf_levels_info = NULL;
    }

    return status;
}

int st_hw_ses_update_config(st_session_t *st_ses, st_hw_session_t* st_hw_ses)
{
    int status = 0;
    uint8_t *opaque_ptr = NULL;
    unsigned int opaque_size = 0, conf_levels_payload_size = 0;
    struct st_param_header *param_hdr = NULL;
    struct st_hist_buffer_info *hist_buf = NULL;
    struct st_det_perf_mode_info *det_perf_mode = NULL;
    struct sound_trigger_recognition_config *rc_config = st_ses->rc_config;

    ST_DBG_DECLARE(FILE *rc_opaque_fd = NULL; static int rc_opaque_cnt = 0);
    ST_DBG_FILE_OPEN_WR(rc_opaque_fd, ST_DEBUG_DUMP_LOCATION,
                        "rc_config_opaque_data", "bin", rc_opaque_cnt++);
    ST_DBG_FILE_WRITE(rc_opaque_fd, (uint8_t *)rc_config + rc_config->data_offset,
                      rc_config->data_size);
    ST_DBG_FILE_CLOSE(rc_opaque_fd);

    /* First release memory and reset allocation from previous rc_config update */
    if (st_hw_ses->conf_levels_info) {
        free(st_hw_ses->conf_levels_info);
        st_hw_ses->conf_levels_info = NULL;
    }
    if (st_hw_ses->conf_levels) {
        free(st_hw_ses->conf_levels);
        st_hw_ses->conf_levels = NULL;
    }
    st_hw_ses->client_req_hist_buf = 0;
    st_hw_ses->client_req_preroll = 0;
    st_hw_ses->client_req_det_mode = ST_HW_SESS_DET_UNKNOWN_MODE;

    if ((rc_config->data_size > CUSTOM_CONFIG_OPAQUE_DATA_SIZE) &&
        st_hw_ses->vendor_uuid_info->is_qcva_uuid) {
        opaque_ptr = (uint8_t *)rc_config + rc_config->data_offset;
        while (opaque_size < rc_config->data_size) {
            param_hdr = (struct st_param_header *)opaque_ptr;
            ALOGV("%s: key %d, payload size %d", __func__,
                  param_hdr->key_id, param_hdr->payload_size);

            switch(param_hdr->key_id) {
            case ST_PARAM_KEY_CONFIDENCE_LEVELS:
                memcpy((char *)&st_ses->conf_levels_intf_version,
                    (char *)(opaque_ptr + sizeof(struct st_param_header)),
                    sizeof(uint32_t));
                if (st_ses->conf_levels_intf_version !=
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
                status = parse_config_key_conf_levels(st_ses, st_hw_ses,
                    opaque_ptr);
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
                if (param_hdr->payload_size != sizeof(struct st_hist_buffer_info)) {
                    ALOGE("%s: History buffer config format error, exiting", __func__);
                    return -EINVAL;
                }
                hist_buf = (struct st_hist_buffer_info *)(opaque_ptr +
                    sizeof(struct st_param_header));
                st_hw_ses->client_req_hist_buf = hist_buf->hist_buffer_duration_msec;
                st_hw_ses->client_req_preroll = hist_buf->pre_roll_duration_msec;
                ALOGV("%s: recognition config history buf len = %d, preroll len = %d, minor version = %d",
                      __func__, hist_buf->hist_buffer_duration_msec,
                      hist_buf->pre_roll_duration_msec, hist_buf->version);
                opaque_size += sizeof(struct st_param_header) + sizeof(struct st_hist_buffer_info);
                opaque_ptr += sizeof(struct st_param_header) + sizeof(struct st_hist_buffer_info);
                break;
            case ST_PARAM_KEY_DETECTION_PERF_MODE:
                if (param_hdr->payload_size != sizeof(struct st_det_perf_mode_info)) {
                    ALOGE("%s: Opaque data format error, exiting", __func__);
                    return -EINVAL;
                }
                det_perf_mode = (struct st_det_perf_mode_info *)(opaque_ptr +
                    sizeof(struct st_param_header));
                ALOGV("set perf mode to %d", det_perf_mode->mode);
                st_hw_ses->client_req_det_mode = det_perf_mode->mode;
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
    } else if (st_ses->sm_type == SOUND_MODEL_TYPE_KEYPHRASE) {
        struct sound_trigger_phrase_sound_model *phrase_sm = st_ses->sm_data;
        struct st_vendor_info *v_info = st_hw_ses->vendor_uuid_info;

        ALOGV("%s: num_phrases=%d, id=%d", __func__,
               rc_config->num_phrases, rc_config->phrases[0].id);

        /*
         * Can be QC SVA or ISV vendor. Get from corresponding smlib defined
         * in platform file. if soundmodel library for ISV vendor uuid is
         * mentioned, use it. If not ignore, in this case opaque data would be
         * send from HAL to DSP.
         */
        if (v_info && v_info->smlib_handle) {
            if (st_ses->stdev->is_gcs && st_hw_ses == st_ses->hw_ses_cpe)
                status = v_info->generate_st_recognition_config_payload_v2(
                                                 phrase_sm, rc_config,
                                                 &st_hw_ses->conf_levels,
                                                 &st_hw_ses->num_conf_levels);
            else
                status = v_info->generate_st_recognition_config_payload(
                                                 phrase_sm, rc_config,
                                                 &st_hw_ses->conf_levels,
                                                 &st_hw_ses->num_conf_levels);
            if (status || !st_hw_ses->conf_levels)
                ALOGE("%s: failed to get conf levels from lib handle", __func__);
        } else {
            ALOGD("%s: No smlib, opaque data would be sent as is", __func__);
        }
    }

    st_hw_ses->rc_config = st_ses->rc_config;
    st_hw_ses->rc_config_update_counter = st_ses->rc_config_update_counter;
    return status;
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

/*
 * This function is used to prepare the detection engine custom config payload for
 * sound trigger sessions that have second stage sessions. If history buffering is not
 * requested by the client, add it into the payload here. Second stage sessions require
 * the keyword to be buffered.
 */
int st_hw_ses_get_hist_buff_payload
(
    st_hw_session_t *p_ses,
    uint8_t *payload_buff,
    size_t buff_size
)
{
    struct st_hist_buffer_info *hist_buf = NULL;

    if (!payload_buff || buff_size < sizeof(*hist_buf)) {
        ALOGE("%s: buffer size(%zd) too small to fill payload(%zd)",
              __func__, buff_size, sizeof(*hist_buf));
        return -EINVAL;
    }

    hist_buf = (struct st_hist_buffer_info *) payload_buff;
    hist_buf->version = DEFAULT_CUSTOM_CONFIG_MINOR_VERSION;

    if (p_ses->client_req_hist_buf > 0) {
        hist_buf->hist_buffer_duration_msec = p_ses->client_req_hist_buf;
        hist_buf->pre_roll_duration_msec = p_ses->client_req_preroll;

        if (p_ses->is_generic_event) {
            if (p_ses->client_req_hist_buf <= p_ses->client_req_preroll + KW_LEN_WARNING)
                ALOGW("%s: Warning: Client hist buf and preroll lens leave only %dms for keyword",
                    __func__, (p_ses->client_req_hist_buf - p_ses->client_req_preroll));

            if (p_ses->client_req_preroll < PREROLL_LEN_WARNING)
                ALOGW("%s: Warning: Client requested small preroll length %dms",
                    __func__, p_ses->client_req_preroll);
        }

    } else {
        hist_buf->hist_buffer_duration_msec = p_ses->vendor_uuid_info->kw_duration;
        hist_buf->pre_roll_duration_msec = 0;
    }

    return 0;
}

static int fill_sound_trigger_recognition_config_payload
(
   const void *sm_levels_generic,
   unsigned char *conf_levels,
   unsigned int num_conf_levels,
   unsigned int total_num_users,
   uint32_t version
)
{
    int status = 0;
    unsigned int user_level, user_id;
    unsigned int i, j;
    unsigned char *user_id_tracker;
    struct st_sound_model_conf_levels *sm_levels = NULL;
    struct st_sound_model_conf_levels_v2 *sm_levels_v2 = NULL;

    /*  Example: Say the recognition structure has 3 keywords with users
     *  |kid|
     *  [0] k1 |uid|
     *         [0] u1 - 1st trainer
     *         [1] u2 - 4th trainer
     *         [3] u3 - 3rd trainer
     *  [1] k2
     *         [2] u2 - 2nd trainer
     *         [4] u3 - 5th trainer
     *  [2] k3
     *         [5] u4 - 6th trainer
     *
     *  Output confidence level array will be
     *  [k1, k2, k3, u1k1, u2k1, u2k2, u3k1, u3k2, u4k3]
     */

    if (version != CONF_LEVELS_INTF_VERSION_0002) {
        sm_levels = (struct st_sound_model_conf_levels *)sm_levels_generic;
        if (!sm_levels || !conf_levels || !num_conf_levels) {
            ALOGE("%s: ERROR. Invalid inputs", __func__);
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
            conf_levels[i] = sm_levels->kw_levels[i].kw_level;
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
        if (!sm_levels_v2 || !conf_levels || !num_conf_levels) {
            ALOGE("%s: ERROR. Invalid inputs", __func__);
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
            conf_levels[i] = sm_levels_v2->kw_levels[i].kw_level;
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

int generate_sound_trigger_recognition_config_payload
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
        conf_levels, num_conf_levels, total_num_users, version);
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

int generate_sound_trigger_recognition_config_payload_v2
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
        conf_levels + 2, num_conf_levels, total_num_users, version);
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
