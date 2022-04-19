/*
 *
 * This file implements the hw session functionality specific to LSM HW
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

#include <stdarg.h>
#include <unistd.h>

#include "st_common_defs.h"
#include "sound_trigger_platform.h"
#include "st_hw_session_lsm.h"
#include "sound_trigger_hw.h"
#include "st_hw_common.h"

#define XSTR(x) STR(x)
#define STR(x) #x

#define MAX_DOA_TRACKING_ANGLES 2
#define DOA_POLAR_ACTIVITY_INDICATORS 360

static int ape_reg_sm(st_hw_session_t* p_ses, void *sm_data,
           sound_trigger_sound_model_type_t sm_type);
static int ape_reg_sm_params(st_hw_session_t* p_ses, unsigned int recognition_mode,
    bool capture_requested, struct sound_trigger_recognition_config *rc_config,
    sound_trigger_sound_model_type_t sm_type, void *sm_data);

static int ape_dereg_sm(st_hw_session_t* p_ses, bool capture_requested);
static int ape_dereg_sm_params(st_hw_session_t* p_ses);
static int ape_start(st_hw_session_t* p_ses);
static int ape_stop(st_hw_session_t* p_ses);
static int ape_stop_buffering(st_hw_session_t* p_ses);

static int cpe_reg_sm(st_hw_session_t* p_ses,void *sm_data, sound_trigger_sound_model_type_t sm_type);
static int cpe_reg_sm_params(st_hw_session_t* p_ses, unsigned int recognition_mode,
    bool capture_requested, struct sound_trigger_recognition_config *rc_config,
    sound_trigger_sound_model_type_t sm_type, void *sm_data);

static int cpe_dereg_sm(st_hw_session_t* p_ses, bool capture_requested);
static int cpe_dereg_sm_params(st_hw_session_t* p_ses);
static int cpe_start(st_hw_session_t* p_ses);
static int cpe_stop(st_hw_session_t* p_ses);
static int cpe_stop_buffering(st_hw_session_t *p_ses, bool capture_requested);

/* Routing layer functions to route to either ADSP or CPE */
static int route_reg_sm_ape(st_hw_session_t *p_ses,
    void *sm_data, sound_trigger_sound_model_type_t sm_type);
static int route_reg_sm_params_ape(st_hw_session_t* p_ses,
    unsigned int recognition_mode, bool capture_requested,
    struct sound_trigger_recognition_config *rc_config,
    sound_trigger_sound_model_type_t sm_type, void *sm_data);
static int route_dereg_sm_ape(st_hw_session_t* p_ses, bool capture_requested);
static int route_dereg_sm_params_ape(st_hw_session_t* p_ses);
static int route_restart_ape(st_hw_session_t* p_ses,
                             unsigned int recognition_mode,
                             bool capture_requested,
                             struct sound_trigger_recognition_config *rc_config,
                             sound_trigger_sound_model_type_t sm_type,
                             void *sm_data);
static int route_start_ape(st_hw_session_t* p_ses);
static int route_stop_ape(st_hw_session_t* p_ses);
static int route_stop_buffering_ape(st_hw_session_t* p_ses, bool capture_requested);
static int route_set_device_ape(st_hw_session_t* p_ses,
                                bool enable);
static int route_read_pcm_ape(st_hw_session_t *p_ses,
                              unsigned char *buf,
                              unsigned int bytes);
static void route_audio_capture_ape(st_hw_session_t* p_ses);
static int route_send_custom_chmix_coeff_ape(st_hw_session_t *p_ses, char *str);

static int route_reg_sm_cpe(st_hw_session_t *p_ses,
    void *sm_data, sound_trigger_sound_model_type_t sm_type);
static int route_reg_sm_params_cpe(st_hw_session_t* p_ses,
    unsigned int recognition_mode, bool capture_requested,
    struct sound_trigger_recognition_config *rc_config,
    sound_trigger_sound_model_type_t sm_type, void *sm_data);
static int route_dereg_sm_cpe(st_hw_session_t* p_ses, bool capture_requested);
static int route_dereg_sm_params_cpe(st_hw_session_t* p_ses);
static int route_start_cpe(st_hw_session_t* p_ses);
static int route_restart(st_hw_session_t* p_ses, unsigned int recognition_mode,
   bool capture_requested, struct sound_trigger_recognition_config *rc_config __unused,
   sound_trigger_sound_model_type_t sm_type, void *sm_data);
static int route_stop_cpe(st_hw_session_t* p_ses);
static int route_stop_buffering_cpe(st_hw_session_t* p_ses, bool capture_requested);
static int route_set_device_cpe(st_hw_session_t* p_ses,
                                bool enable);
static int route_read_pcm_cpe(st_hw_session_t *p_ses,
                              unsigned char *buf,
                              unsigned int bytes);
static void route_audio_capture_cpe(st_hw_session_t* p_ses);
static int route_send_custom_chmix_coeff_cpe(st_hw_session_t *p_ses, char *str);
static int route_disable_device(st_hw_session_t *p_ses, bool setting_device);
static int route_enable_device(st_hw_session_t *p_ses, bool setting_device);
static void request_exit_callback_thread(st_hw_session_lsm_t *p_lsm_ses);
static int get_param_data(st_hw_session_t* p_ses, const char *param,
    void *payload, size_t payload_size, size_t *param_data_size);
static int send_detection_request(st_hw_session_t *p_ses);

static int dbg_trace_lab_buf_cnt = -1;

typedef struct {
    int16_t target_angle_L16[MAX_DOA_TRACKING_ANGLES];
    int16_t interf_angle_L16[MAX_DOA_TRACKING_ANGLES];
    int8_t polarActivityGUI[DOA_POLAR_ACTIVITY_INDICATORS];
} st_ffv_doa_tracking_monitor_t;

static struct pcm_config stdev_ape_pcm_config = {
    .channels = SOUND_TRIGGER_CHANNEL_MODE_MONO,
    .rate = SOUND_TRIGGER_SAMPLING_RATE_16000,
    .period_size = SOUND_TRIGGER_APE_PERIOD_SIZE,
    .period_count = SOUND_TRIGGER_APE_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

static struct pcm_config stdev_cpe_pcm_config = {
    .channels = SOUND_TRIGGER_CHANNEL_MODE_MONO,
    .rate = SOUND_TRIGGER_SAMPLING_RATE_16000,
    .period_size = SOUND_TRIGGER_CPE_PERIOD_SIZE,
    .period_count = SOUND_TRIGGER_CPE_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct st_session_fptrs ape_fptrs = {

    .reg_sm = route_reg_sm_ape ,
    .reg_sm_params = route_reg_sm_params_ape,
    .dereg_sm = route_dereg_sm_ape ,
    .dereg_sm_params = route_dereg_sm_params_ape,
    .start = route_start_ape,
    .restart = route_restart_ape,
    .stop = route_stop_ape,
    .stop_buffering = route_stop_buffering_ape,
    .set_device = route_set_device_ape,
    .read_pcm = route_read_pcm_ape,
    .process_lab_capture = route_audio_capture_ape,
    .send_custom_chmix_coeff = route_send_custom_chmix_coeff_ape,
    .disable_device = route_disable_device,
    .enable_device = route_enable_device,
    .get_param_data = get_param_data,
    .send_detection_request = send_detection_request,
};

struct st_session_fptrs cpe_fptrs = {
    .reg_sm = route_reg_sm_cpe ,
    .reg_sm_params = route_reg_sm_params_cpe,
    .dereg_sm = route_dereg_sm_cpe  ,
    .dereg_sm_params = route_dereg_sm_params_cpe,
    .start = route_start_cpe,
    .restart = route_restart,
    .stop = route_stop_cpe,
    .stop_buffering = route_stop_buffering_cpe,
    .set_device = route_set_device_cpe,
    .read_pcm = route_read_pcm_cpe,
    .process_lab_capture = route_audio_capture_cpe,
    .send_custom_chmix_coeff = route_send_custom_chmix_coeff_cpe,
    .disable_device = route_disable_device,
    .enable_device = route_enable_device,
    .get_param_data = get_param_data,
    .send_detection_request = send_detection_request,
};

int pcm_ioctl(struct pcm *pcm, int request, ...)
{
    va_list ap;
    void * arg;
    int pcm_fd = *(int*)pcm;

    va_start(ap, request);
    arg = va_arg(ap, void *);
    va_end(ap);

    return ioctl(pcm_fd, request, arg);
}

#if (SNDRV_LSM_VERSION >= SNDRV_PROTOCOL_VERSION(0, 3, 1))
typedef struct lsm_params_info_v2 lsm_param_info_t;
typedef struct lsm_param_payload_v2 lsm_param_payload_t;

static int lsm_set_session_data_v2(st_hw_session_t *p_ses)
{
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t*)p_ses;
    struct st_vendor_info *v_info = p_ses->vendor_uuid_info;
    struct snd_lsm_session_data_v2 ses_data = {0};
    uint16_t stage_idx = LSM_STAGE_INDEX_FIRST;
    struct listnode *node;
    st_lsm_ss_config_t *ss_cfg;
    int status = 0;

    ses_data.app_id = LSM_VOICE_WAKEUP_APP_ID_V2;
    ses_data.num_stages = p_lsm_ses->num_stages;
    ses_data.stage_info[stage_idx].app_type =
        (v_info->app_type == 0) ?
        p_lsm_ses->lsm_usecase->app_type : v_info->app_type;
    ses_data.stage_info[stage_idx].lpi_enable = p_ses->lpi_enable;
    ALOGD("%s: Sending lpi_enable = %s to LSM", __func__,
        p_ses->lpi_enable ? "true" : "false");

    list_for_each(node, &p_ses->lsm_ss_cfg_list) {
        ss_cfg = node_to_item(node, st_lsm_ss_config_t, list_node);
        stage_idx++;
        ses_data.stage_info[stage_idx].app_type = ss_cfg->params->app_type;
        ses_data.stage_info[stage_idx].lpi_enable = ss_cfg->params->lpi_enable;
        ALOGD("%s: Sending lpi_enable = %s to LSM for stage_idx %d", __func__,
            ss_cfg->params->lpi_enable ? "true" : "false", stage_idx);
    }

    ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_set_session_data_v2");
    status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_SET_SESSION_DATA_V2, &ses_data);
    ATRACE_END();
    if (status)
            ALOGE("%s: ERROR. SNDRV_LSM_SET_SESSION_DATA_V2 failed status(%d)",
                  __func__, status);
    return status;
}

static void lsm_fill_param_info
(
    uint32_t param_type,
    lsm_param_info_t *p_info,
    struct st_module_param_info *mparams,
    uint16_t stage_idx
)
{
    p_info->param_type = param_type;
    p_info->module_id = mparams->module_id;
    p_info->instance_id = mparams->instance_id;
    p_info->param_id = mparams->param_id;
    p_info->stage_idx = stage_idx;
}

static int lsm_set_module_params
(
    st_hw_session_lsm_t *p_lsm_ses,
    struct snd_lsm_module_params *lsm_params
)
{
    int status = 0;

    ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_set_module_params_v2");
    status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_SET_MODULE_PARAMS_V2, lsm_params);
    ATRACE_END();

    if (status)
        ALOGE("%s: ERROR. SNDRV_LSM_SET_MODULE_PARAMS_V2 count(%d), status(%d)",
              __func__, lsm_params->num_params, status);
    return status;
}

bool st_hw_check_multi_stage_lsm_support()
{
    return true;
}

static void lsm_fill_param_header
(
    lsm_param_payload_t *custom_conf,
    uint32_t payload_size,
    struct st_module_param_info *mparams
)
{
    custom_conf->module_id = mparams->module_id;
    custom_conf->instance_id = mparams->instance_id;
    custom_conf->reserved = 0;
    custom_conf->param_id = mparams->param_id;
    custom_conf->p_size = payload_size;
}
#else
#define LSM_MAX_STAGES_PER_SESSION 1
#define LSM_STAGE_INDEX_FIRST 0
#define LSM_LAB_CONTROL -1
typedef struct lsm_params_info lsm_param_info_t;
typedef struct lsm_param_payload lsm_param_payload_t;

static int lsm_set_session_data_v2(st_hw_session_t *p_ses __unused)
{
    ALOGE("%s: ERROR: unexpected call, check support from \"%s\" before using",
          __func__, "st_hw_check_multi_stage_lsm_support");
    return -ENOSYS;
}

static void lsm_fill_param_info
(
    uint32_t param_type,
    lsm_param_info_t *p_info,
    struct st_module_param_info *mparams,
    uint16_t stage_idx __unused
)
{
    p_info->param_type = param_type;
    p_info->module_id = mparams->module_id;
    p_info->param_id = mparams->param_id;
}

static int lsm_set_module_params
(
    st_hw_session_lsm_t *p_lsm_ses,
    struct snd_lsm_module_params *lsm_params
)
{
    int status = 0;

    ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_set_module_params");
    status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_SET_MODULE_PARAMS, lsm_params);
    ATRACE_END();

    if (status)
        ALOGE("%s: ERROR. SNDRV_LSM_SET_MODULE_PARAMS count(%d), status(%d)",
              __func__, lsm_params->num_params, status);
    return status;
}

bool st_hw_check_multi_stage_lsm_support()
{
    return false;
}

static void lsm_fill_param_header
(
    lsm_param_payload_t *custom_conf,
    uint32_t payload_size,
    struct st_module_param_info *mparams
)
{
    custom_conf->module_id = mparams->module_id;
    custom_conf->param_id = mparams->param_id;
    custom_conf->p_size = payload_size;
}
#endif

#ifdef LSM_EVENT_TIMESTAMP_MODE_SUPPORT
static int set_lsm_fwk_mode(st_hw_session_lsm_t *p_lsm_ses)
{
    int status;
    struct st_vendor_info *v_info = p_lsm_ses->common.vendor_uuid_info;
    unsigned int fwk_mode = LSM_EVENT_NON_TIME_STAMP_MODE;

    /* set fwk mode is not supported in CPE mode */
    if (p_lsm_ses->common.exec_mode == ST_EXEC_MODE_CPE)
        return 0;

    if (v_info && (v_info->fwk_mode == SOUND_TRIGGER_EVENT_TIME_STAMP_MODE))
        fwk_mode = LSM_EVENT_TIME_STAMP_MODE;

    status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_SET_FWK_MODE_CONFIG,
                       &fwk_mode);
    if (status)
        ALOGE("%s: SNDRV_LSM_SET_FWK_MODE_CONFIG status=%d", __func__, status);

    return status;
}

static void update_lsm_event_status_info(st_hw_session_lsm_t *p_lsm_ses,
                                         int *request,
                                         char **st_lsm_event_cmd)
{
#ifdef LSM_DET_EVENT_TYPE_GENERIC
    if (p_lsm_ses->common.is_generic_event) {
        *request = SNDRV_LSM_GENERIC_DET_EVENT;
        *st_lsm_event_cmd = strdup("SNDRV_LSM_GENERIC_DET_EVENT");
    }
    else
#endif
    if (!p_lsm_ses->common.is_generic_event) {
        *request = SNDRV_LSM_EVENT_STATUS_V3;
        *st_lsm_event_cmd = strdup("SNDRV_LSM_EVENT_STATUS_V3");
    }
}

static uint64_t get_event_timestamp(st_lsm_event_status_t *params)
{
    uint64_t timestamp;

    timestamp = ((uint64_t)params->timestamp_msw << 32) |
                           params->timestamp_lsw;
    return timestamp;
}
#else
static int set_lsm_fwk_mode(st_hw_session_lsm_t *p_lsm_ses __unused)
{
    /* set fwk mode not supported */
    return 0;
}

static void update_lsm_event_status_info(st_hw_session_lsm_t *p_lsm_ses,
                                         int *request,
                                         char **st_lsm_event_cmd)
{
#ifdef LSM_DET_EVENT_TYPE_GENERIC
    if (p_lsm_ses->common.is_generic_event) {
        *request = SNDRV_LSM_GENERIC_DET_EVENT;
        *st_lsm_event_cmd = strdup("SNDRV_LSM_GENERIC_DET_EVENT");
    }
    else
#endif
    if (!p_lsm_ses->common.is_generic_event) {
        *request = SNDRV_LSM_EVENT_STATUS;
        *st_lsm_event_cmd = strdup("SNDRV_LSM_EVENT_STATUS");
    }
}

static uint64_t get_event_timestamp(st_lsm_event_status_t *params __unused)
{
    /* timestamp mode not supported */
    return 0;
}
#endif

#ifdef LSM_POLLING_ENABLE_SUPPORT
static int lsm_set_port(st_hw_session_lsm_t *p_lsm_ses)
{
    int status = 0;

    if (p_lsm_ses->common.stdev->lpi_enable &&
        p_lsm_ses->common.vendor_uuid_info->lab_dam_cfg_payload.token_id) {
        status = platform_stdev_set_shared_buf_fmt(
            p_lsm_ses->common.stdev->platform, p_lsm_ses->pcm_id,
            p_lsm_ses->common.vendor_uuid_info->shared_buf_fmt);
        if (status)
            return status;
    }

    status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_SET_PORT);
    if (status)
        ALOGE("%s: ERROR. SNDRV_LSM_SET_PORT, status=%d", __func__, status);
    return status;
}

static bool fill_lsm_poll_enable_params
(
    struct st_vendor_info *v_info,
    st_lsm_poll_enable_t *poll_enable,
    lsm_param_info_t *poll_en_params,
    struct st_module_param_info *mparams,
    uint16_t stage_idx
)
{
    poll_enable->poll_en = v_info->profile_type == ST_PROFILE_TYPE_NONE;

    poll_en_params->param_size = sizeof(*poll_enable);
    poll_en_params->param_data = (unsigned char *)poll_enable;
    lsm_fill_param_info(LSM_POLLING_ENABLE, poll_en_params,
                        &mparams[POLLING_ENABLE], stage_idx);
    return true;
}
#else
static int lsm_set_port(st_hw_session_lsm_t *p_lsm_ses __unused)
{
    return 0;
}

static bool fill_lsm_poll_enable_params
(
    struct st_vendor_info *v_info __unused,
    st_lsm_poll_enable_t *poll_enable __unused,
    lsm_param_info_t *poll_en_params __unused,
    struct st_module_param_info *mparams __unused,
    uint16_t stage_idx __unused
)
{
    return false;
}
#endif

#if (SNDRV_LSM_VERSION >= SNDRV_PROTOCOL_VERSION(0, 3, 0))
static int send_lsm_input_hw_params(st_hw_session_t *p_ses)
{
    struct st_vendor_info *v_info = p_ses->vendor_uuid_info;
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t*)p_ses;
    struct snd_lsm_input_hw_params params;
    int status = 0;

    params.sample_rate = v_info->sample_rate;
    params.bit_width = pcm_format_to_bits(v_info->format);
    if (platform_get_lpi_mode(p_ses->stdev->platform))
        params.num_channels = p_lsm_ses->lsm_usecase->in_channels_lpi;
    else
        params.num_channels = p_lsm_ses->lsm_usecase->in_channels;

    ALOGV("%s: set SNDRV_LSM_SET_INPUT_HW_PARAMS sr=%d bw=%d ch=%d ", __func__,
          params.sample_rate, params.bit_width, params.num_channels);

    status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_SET_INPUT_HW_PARAMS,
                       &params);
    if (status) {
        ALOGE("%s: SNDRV_LSM_SET_INPUT_HW_PARAMS failed, status [%d] - %s",
              __func__, status, strerror(errno));
    }
    return status;
}
#else
#define send_lsm_input_hw_params(a) (0)
#endif

static void fill_set_params_payload(struct lsm_setparam_payload *custom_conf_setparam,
               uint32_t data_payload_size, uint32_t data_payload_addr_lsw,
               uint32_t data_payload_addr_msw, uint32_t mem_map_handle)
{
    custom_conf_setparam->data_payload_size = data_payload_size;
    custom_conf_setparam->data_payload_addr_lsw = data_payload_addr_lsw;
    custom_conf_setparam->data_payload_addr_msw = data_payload_addr_msw;
    custom_conf_setparam->mem_map_handle = mem_map_handle;
}

#ifdef LSM_DET_EVENT_TYPE_GENERIC
static bool fill_lsm_det_event_type_params
(
   st_lsm_det_event_type_t *det_event_type,
   lsm_param_info_t *det_event_type_params,
   struct st_module_param_info *mparams,
   uint16_t stage_idx
)
{
    /* fill event type params */
    det_event_type->event_type = LSM_DET_EVENT_TYPE_GENERIC;
    /* request for confidence level and timestamp */
    det_event_type->mode = ACD_CONFIDENCE_LEVELS_BIT | ACD_TIME_STAMP_INFO_BIT;

    det_event_type_params->param_size = sizeof(*det_event_type);
    det_event_type_params->param_data = (unsigned char *)det_event_type;
    lsm_fill_param_info(LSM_DET_EVENT_TYPE, det_event_type_params,
                        &mparams[DET_EVENT_TYPE], stage_idx);
    return true;
}
#else
static bool fill_lsm_det_event_type_params
(
   st_lsm_det_event_type_t *det_event_type __unused,
   lsm_param_info_t *det_event_type_params __unused,
   struct st_module_param_info *mparams __unused,
   uint16_t stage_idx __unused
)
{
    return false;
}
#endif

static st_profile_type_t get_profile_type(st_hw_session_t *p_ses)
{
    st_profile_type_t profile_type;

    profile_type = (p_ses->vendor_uuid_info && !p_ses->stdev->lpi_enable) ?
                   p_ses->vendor_uuid_info->profile_type : ST_PROFILE_TYPE_NONE;
    return profile_type;
}

void check_and_exit_lab(st_hw_session_t *p_ses)
{
    st_hw_session_lsm_t *p_lsm_ses =
        (st_hw_session_lsm_t *)p_ses;

    if ((p_lsm_ses->lab_buffers_allocated && !p_lsm_ses->exit_lab_processing) &&
        CHECK_STATE(p_lsm_ses->common.state, SES_BUFFERING)) {

        pthread_mutex_lock(&p_lsm_ses->lock);
        p_lsm_ses->exit_lab_processing = true;
        pthread_cond_signal(&p_lsm_ses->cond);
        pthread_mutex_unlock(&p_lsm_ses->lock);
    }
}

static void ape_enable_use_case(bool enable, st_hw_session_t *p_ses)
{
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;
    st_profile_type_t profile_type = get_profile_type(p_ses);
    char use_case[USECASE_STRING_SIZE];

    if (enable) {
        strlcpy(use_case,
                p_ses->stdev->ape_pcm_use_cases[p_ses->use_case_idx].use_case,
                USECASE_STRING_SIZE);
        platform_stdev_check_and_append_usecase(p_ses->stdev->platform,
                                               use_case, profile_type);
        ALOGD("%s: enable use case = %s", __func__, use_case);
        platform_stdev_send_stream_app_type_cfg(p_ses->stdev->platform,
                                   p_lsm_ses->pcm_id, p_ses->st_device,
                                   p_ses->exec_mode, profile_type);

        platform_stdev_send_ec_ref_cfg(p_ses->stdev->platform, profile_type, true);


        ATRACE_BEGIN("sthal:lsm: audio_route_apply_and_update_path");
        audio_route_apply_and_update_path(p_ses->stdev->audio_route, use_case);
        ATRACE_END();
        if (p_lsm_ses->use_case)
            free(p_lsm_ses->use_case);

        p_lsm_ses->use_case = strdup(use_case);
    } else {
        if (!p_lsm_ses->use_case) {
            ALOGE("%s: Invalid sound trigger usecase control", __func__);
            return;
        }

        ALOGD("%s: disable use case = %s", __func__, p_lsm_ses->use_case);
        ATRACE_BEGIN("sthal:lsm: audio_route_reset_and_update_path");
        audio_route_reset_and_update_path(p_ses->stdev->audio_route, p_lsm_ses->use_case);
        ATRACE_END();

        platform_stdev_send_ec_ref_cfg(p_ses->stdev->platform, profile_type, false);
        free(p_lsm_ses->use_case);
        p_lsm_ses->use_case = NULL;
    }
}

static int ape_enable_port_control(bool enable, st_hw_session_t *p_ses)
{
    int ret = 0;
    char port_ctrl[USECASE_STRING_SIZE] = {0};
    st_profile_type_t profile_type = get_profile_type(p_ses);
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;

    if (enable) {
        strlcpy(port_ctrl,
                p_ses->stdev->ape_pcm_use_cases[p_ses->use_case_idx].use_case,
                USECASE_STRING_SIZE);
        platform_stdev_check_and_append_usecase(p_ses->stdev->platform,
                                                port_ctrl, profile_type);
        strlcat(port_ctrl, " port", USECASE_STRING_SIZE);

        ALOGV("%s: enable = %s", __func__, port_ctrl);
        ret = audio_route_apply_and_update_path(p_ses->stdev->audio_route, port_ctrl);
        if (!ret) {
            if (p_lsm_ses->port_ctrl)
                free(p_lsm_ses->port_ctrl);
            p_lsm_ses->port_ctrl = strdup(port_ctrl);
        }
    } else {
        if (!p_lsm_ses->port_ctrl) {
            ALOGW("%s: Invalid sound trigger port control", __func__);
            ret = -EINVAL;
        } else {
            ALOGV("%s: disable = %s", __func__, p_lsm_ses->port_ctrl);
            /*
             * Do not send reset controls as driver is not maintaining
             * state per session since current implementation limits port
             * settings to be shared globaly and cannot vary per session.
             * After first session resets port value as part of its disable
             * sequence, driver is missing info if the next session to be
             * disabled was using adm/non-adm path.
             * Port update if applicable would be done as part of enable
             * sequence of any one of the sessions.
             */
            free(p_lsm_ses->port_ctrl);
            p_lsm_ses->port_ctrl = NULL;
        }
    }

    return ret;
}

static int read_pcm_data(st_hw_session_lsm_t *p_ses,
                     unsigned char *buf,
                     unsigned int bytes)
{
    unsigned int read_bytes = 0, move_bytes = 0, delay_bytes = 0;
    unsigned int requested_bytes = 0;
    struct timespec tspec;
    int ret = 0;

    pthread_mutex_lock(&p_ses->lock);
    ALOGVV("%s: bytes=%d, unread_bytes=%d", __func__, bytes,
        p_ses->unread_bytes);

    if (p_ses->move_client_ptr) {
        /*
         * This logic is needed if LAB was enabled due to second stage being enabled,
         * but the client did not request LAB. The client read pointer will be shifted
         * to the keyword end index in this usecase.
         */
        delay_bytes = p_ses->common.kw_end_idx;
        move_bytes = MIN(delay_bytes, p_ses->unread_bytes);
        ALOGD("%s: Moving client ptr by %d bytes", __func__, move_bytes);
        st_buffer_flush(p_ses->common.buffer, move_bytes);
        p_ses->unread_bytes -= move_bytes;
        p_ses->move_client_ptr = false;
    }

    requested_bytes = bytes;
    while (!p_ses->exit_lab_processing && (bytes > 0)) {
        if (!p_ses->unread_bytes) {
            ALOGVV("%s: waiting on cond, bytes=%d", __func__, bytes);
            /* Time out to unblock read thread in case if write thread is
               stuck filling the buffers */
            GET_WAIT_TIMESPEC(tspec, convert_bytes_to_ms((requested_bytes * 2),
                &p_ses->common.config) * NSECS_PER_MSEC);
            ret = pthread_cond_timedwait(&p_ses->cond, &p_ses->lock, &tspec);
            ALOGVV("%s: done waiting on cond", __func__);
            if (ret) {
                ALOGE("%s: ERROR. read wait timed out, ret %d", __func__, ret);
                p_ses->exit_lab_processing = true;
                ret = -EIO;
                goto exit;
            }
            if (p_ses->exit_lab_processing) {
                 ALOGVV("%s: buffering stopped while waiting on cond, exiting",
                    __func__);
                ret = -EIO;
                goto exit;
            }
        }
        read_bytes = MIN(bytes, p_ses->unread_bytes);

        ret = st_buffer_read(p_ses->common.buffer, buf, read_bytes,
            NULL, true);
        if (ret) {
            ALOGE("%s: st_buffer_read failed, status %d", __func__, ret);
            goto exit;
        }

        pthread_cond_signal(&p_ses->cond);
        p_ses->unread_bytes -= read_bytes;
        bytes -= read_bytes;
        buf += read_bytes;
    }

exit:
    pthread_mutex_unlock(&p_ses->lock);
    return 0;
}

static int write_pcm_data_cpe(st_hw_session_lsm_t *p_lsm_ses,
                     unsigned char *buf,
                     unsigned int bytes)
{
    unsigned int free_bytes = 0, write_bytes = 0;
    int status = 0;
    struct listnode *node = NULL, *tmp_node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;

    pthread_mutex_lock(&p_lsm_ses->lock);

    while (!p_lsm_ses->exit_lab_processing && (bytes > 0)) {
        free_bytes = p_lsm_ses->common.buffer->size -
            p_lsm_ses->common.buffer->unread_bytes;
        if (!free_bytes) {
            ALOGVV("%s: waiting on cond, bytes=%d", __func__, bytes);
            /* wait till space is avaialable to write */
            pthread_cond_wait(&p_lsm_ses->cond, &p_lsm_ses->lock);
            if (p_lsm_ses->exit_lab_processing) {
                 ALOGVV("%s: buffering stopped while waiting on cond, exiting", __func__);
                pthread_mutex_unlock(&p_lsm_ses->lock);
                return -EIO;
            }
        }
        write_bytes = MIN(bytes, free_bytes);

        status = st_buffer_write(p_lsm_ses->common.buffer, buf, write_bytes);
        if (status) {
            p_lsm_ses->exit_lab_processing = true;
            if (p_lsm_ses->common.enable_second_stage) {
                list_for_each_safe(node, tmp_node, p_lsm_ses->common.second_stage_list) {
                    st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
                    pthread_mutex_lock(&st_sec_stage->ss_session->lock);
                    ALOGW("%s: Exit 2nd stage processing due to buf overflow",
                        __func__);
                    st_sec_stage->ss_session->exit_buffering = true;
                    pthread_cond_signal(&st_sec_stage->ss_session->cond);
                    pthread_mutex_unlock(&st_sec_stage->ss_session->lock);
                }
            }
            goto exit;
        }
        ALOGV("%s: about to signal condition", __func__);
        pthread_cond_signal(&p_lsm_ses->cond);
        bytes -= write_bytes;
        buf += write_bytes;
        p_lsm_ses->unread_bytes += write_bytes;
        p_lsm_ses->bytes_written += write_bytes;
    }

    if (p_lsm_ses->common.enable_second_stage) {
        list_for_each_safe(node, tmp_node, p_lsm_ses->common.second_stage_list) {
            st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);

            /*
             * When the current written frame passes the number of bytes indicated
             * in buf_start, give the second stage session its read pointer.
             */
            pthread_mutex_lock(&st_sec_stage->ss_session->lock);
            if ((p_lsm_ses->bytes_written >= st_sec_stage->ss_session->buf_start) &&
                !st_sec_stage->ss_session->start_processing) {
                st_sec_stage->ss_session->hw_rd_ptr =
                    st_buffer_get_wr_ptr(p_lsm_ses->common.buffer);
                st_sec_stage->ss_session->hw_rd_ptr -= bytes;
                st_sec_stage->ss_session->start_processing = true;
            }

            if (st_sec_stage->ss_session->start_processing) {
                st_sec_stage->ss_session->unread_bytes += bytes;
                pthread_cond_signal(&st_sec_stage->ss_session->cond);
            }
            pthread_mutex_unlock(&st_sec_stage->ss_session->lock);
        }
    }

exit:
    pthread_mutex_unlock(&p_lsm_ses->lock);
    return 0;
}

static int write_pcm_data_ape(st_hw_session_lsm_t *p_lsm_ses,
                     unsigned char *buf,
                     unsigned int bytes)
{
    int status = 0;
    struct listnode *node = NULL, *tmp_node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;

    status = st_buffer_write(p_lsm_ses->common.buffer, buf, bytes);
    if (status) {
        p_lsm_ses->exit_lab_processing = true;
        if (p_lsm_ses->common.enable_second_stage) {
            list_for_each_safe(node, tmp_node, p_lsm_ses->common.second_stage_list) {
                st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
                pthread_mutex_lock(&st_sec_stage->ss_session->lock);
                ALOGW("%s: Exit 2nd stage processing due to buf overflow",
                    __func__);
                st_sec_stage->ss_session->exit_buffering = true;
                pthread_cond_signal(&st_sec_stage->ss_session->cond);
                pthread_mutex_unlock(&st_sec_stage->ss_session->lock);
            }
        }
        goto exit;
    }
    p_lsm_ses->unread_bytes += bytes;
    p_lsm_ses->bytes_written += bytes;

    if (p_lsm_ses->common.enable_second_stage) {
        list_for_each_safe(node, tmp_node, p_lsm_ses->common.second_stage_list) {
            st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);

            /*
             * When the current written frame passes the number of bytes indicated
             * in buf_start, give the second stage session its read pointer.
             */
            pthread_mutex_lock(&st_sec_stage->ss_session->lock);
            if ((p_lsm_ses->bytes_written >= st_sec_stage->ss_session->buf_start) &&
                !st_sec_stage->ss_session->start_processing) {
                st_sec_stage->ss_session->hw_rd_ptr =
                    st_buffer_get_wr_ptr(p_lsm_ses->common.buffer);
                st_sec_stage->ss_session->hw_rd_ptr -= bytes;
                st_sec_stage->ss_session->start_processing = true;
            }

            if (st_sec_stage->ss_session->start_processing) {
                st_sec_stage->ss_session->unread_bytes += bytes;
                pthread_cond_signal(&st_sec_stage->ss_session->cond);
            }
            pthread_mutex_unlock(&st_sec_stage->ss_session->lock);
        }
    }

exit:
    ALOGVV("%s: about to signal condition", __func__);
    pthread_cond_signal(&p_lsm_ses->cond);
    return status;
}

static bool search_for_syncword(st_hw_session_lsm_t *p_ses,
                                unsigned char *in_buf,
                                unsigned int in_size,
                                unsigned int *pos)
{

    unsigned char *buf_ptr = in_buf;
    unsigned int inc_bytes = 2;

    /* S16_LE is sampled at 16bit boundary, 24_LE and 32_LE will be
       sampled at 4byte boundary */
    if ((p_ses->lab_config.format == PCM_FORMAT_S32_LE) ||
        (p_ses->lab_config.format == PCM_FORMAT_S24_LE)) {
        inc_bytes += 2;
    }
    *pos = 0;

    while (buf_ptr < (in_buf + in_size)) {
        if (*buf_ptr == CPE_PACKET_SYNC_WORD) {
            *pos = buf_ptr - in_buf;
            ALOGVV("%s: first syncword found at idx %d for format %d",
                   __func__, *pos, p_ses->lab_config.format);
            return true;
        }
        buf_ptr += inc_bytes;
    }
    return false;
}

static void get_ftrt_processing_data(st_hw_session_lsm_t *p_ses,
                              unsigned char *in_buf,
                              unsigned int in_size,
                              unsigned char **out_buf,
                              unsigned int *out_size)
{
    unsigned char *src, *dst;

    if (p_ses->lab_config.format == PCM_FORMAT_S24_LE) {
        /* Construct ftrt packed buffer without gaps by skipping MSB byte */
        src = dst = in_buf;
        while (src < (in_buf + in_size)) {
            memmove(dst, src, 3);
            src += 4;
            dst += 3;
        }
        *out_buf = in_buf;
        *out_size = (dst - *out_buf);
    } else {
        *out_buf = in_buf;
        *out_size = in_size;
    }
}

static void adjust_ss_buff_end(st_hw_session_t *p_ses,
    uint32_t cnn_append_bytes, uint32_t vop_append_bytes)
{
    st_hw_session_lsm_t *p_hw_ses = (st_hw_session_lsm_t *)p_ses;
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

void process_raw_lab_data_cpe(st_hw_session_lsm_t *p_lsm_ses)
{
    int status = 0;
    struct listnode *node = NULL, *tmp_node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;
    unsigned int prepend_bytes = 0, cnn_append_bytes = 0, vop_append_bytes = 0;
    unsigned int kw_duration_bytes = 0;
    bool real_time_check = false;
    uint64_t frame_receive_time = 0, frame_send_time = 0;
    uint64_t frame_read_time = 0;

    ST_DBG_DECLARE(FILE *fptr_drv = NULL; static int file_cnt = 0);
    ST_DBG_FILE_OPEN_WR(fptr_drv, ST_DEBUG_DUMP_LOCATION,
                        "st_lab_drv_data_cpe", "pcm", file_cnt++);

    p_lsm_ses->lab_processing_active = true;
    p_lsm_ses->unread_bytes = 0;
    p_lsm_ses->bytes_written = 0;

    st_buffer_reset(p_lsm_ses->common.buffer);

    if (p_lsm_ses->common.enable_second_stage) {
        prepend_bytes =
            convert_ms_to_bytes(
                p_lsm_ses->common.vendor_uuid_info->kw_start_tolerance,
                &p_lsm_ses->common.config);
        if (p_lsm_ses->common.client_req_hist_buf) {
            kw_duration_bytes =
                convert_ms_to_bytes(p_lsm_ses->common.client_req_hist_buf,
                    &p_lsm_ses->common.config);
        } else {
            kw_duration_bytes =
                convert_ms_to_bytes(p_lsm_ses->common.vendor_uuid_info->kw_duration,
                    &p_lsm_ses->common.config);
        }

        list_for_each_safe(node, tmp_node, p_lsm_ses->common.second_stage_list) {
            st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);

            pthread_mutex_lock(&st_sec_stage->ss_session->lock);
            /*
             * In the generic detection event usecase, the start of the buffer
             * sent to 2nd stage is determined by the 1st stage keyword start
             * index. This index can have some error, so the start of the buffer
             * is moved forward to ensure there are no resulting missed
             * detections. Similarly, error tolerance is added to the end of the
             * buffer for generic and non generic detection event usecases.
             */
            if (p_lsm_ses->common.kw_start_idx > prepend_bytes) {
                st_sec_stage->ss_session->buf_start =
                    p_lsm_ses->common.kw_start_idx - prepend_bytes;
            } else {
                st_sec_stage->ss_session->buf_start = 0;
            }

            if (st_sec_stage->ss_info->sm_detection_type ==
                ST_SM_TYPE_KEYWORD_DETECTION) {
                cnn_append_bytes =
                    convert_ms_to_bytes(
                        (p_lsm_ses->common.vendor_uuid_info->kw_end_tolerance +
                         st_sec_stage->ss_info->data_after_kw_end),
                        &p_lsm_ses->common.config);

                if (p_lsm_ses->common.kw_end_idx < kw_duration_bytes) {
                    st_sec_stage->ss_session->buf_end =
                        p_lsm_ses->common.kw_end_idx + cnn_append_bytes;
                } else {
                    st_sec_stage->ss_session->buf_end = kw_duration_bytes +
                        cnn_append_bytes;
                }
                st_sec_stage->ss_session->buff_sz = p_lsm_ses->lab_drv_buf_size;
                st_sec_stage->ss_session->det_status = KEYWORD_DETECTION_PENDING;
            } else if (st_sec_stage->ss_info->sm_detection_type ==
                ST_SM_TYPE_USER_VERIFICATION) {
                vop_append_bytes =
                    convert_ms_to_bytes(
                        p_lsm_ses->common.vendor_uuid_info->kw_end_tolerance,
                        &p_lsm_ses->common.config);

                if ((p_lsm_ses->common.kw_end_idx + vop_append_bytes) <
                    kw_duration_bytes) {
                    st_sec_stage->ss_session->buf_end =
                        p_lsm_ses->common.kw_end_idx + vop_append_bytes;
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

        if (p_lsm_ses->common.enable_second_stage &&
            !p_lsm_ses->common.client_req_hist_buf)
            p_lsm_ses->move_client_ptr = true;
        else
            p_lsm_ses->move_client_ptr = false;

        if (!p_lsm_ses->common.is_generic_event)
            real_time_check = true;
    }


    ST_DBG_ATRACE_ASYNC_BEGIN_IF(!(dbg_trace_lab_buf_cnt = 0),"sthal:lsm:cpe: lsm_buffer_read",
                                    p_lsm_ses->common.sm_handle);
    while (!p_lsm_ses->exit_lab_processing) {
        ALOGVV("%s: pcm_read reading bytes=%d", __func__, p_lsm_ses->lab_drv_buf_size);
        frame_send_time = get_current_time_ns();
        status = pcm_read(p_lsm_ses->pcm, p_lsm_ses->lab_drv_buf, p_lsm_ses->lab_drv_buf_size);
        ST_DBG_ATRACE_ASYNC_END_IF((++dbg_trace_lab_buf_cnt == dbg_trace_max_lab_reads),
                                    "sthal:lsm:cpe: lsm_buffer_read", p_lsm_ses->common.sm_handle);
        frame_receive_time = get_current_time_ns();

        ALOGVV("%s: pcm_read done", __func__);

        ST_DBG_FILE_WRITE(fptr_drv, p_lsm_ses->lab_drv_buf,
            p_lsm_ses->lab_drv_buf_size);

        if (status) {
            ALOGE("%s: pcm read failed status %d - %s", __func__, status,
                  pcm_get_error(p_lsm_ses->pcm));
            p_lsm_ses->exit_lab_processing = true;
            p_lsm_ses->common.fptrs->stop_buffering(&p_lsm_ses->common, true);
            pcm_stop(p_lsm_ses->pcm);
            pcm_start(p_lsm_ses->pcm);
            break;
        }

        status = write_pcm_data_cpe(p_lsm_ses, p_lsm_ses->lab_drv_buf, p_lsm_ses->lab_drv_buf_size);
        if (status)
            break;
        frame_read_time = frame_receive_time - frame_send_time;
        if (real_time_check &&
            (frame_read_time > CPE_MAX_LAB_FTRT_FRAME_RD_TIME_NS)) {
            uint32_t bytes_written_ms =
                convert_bytes_to_ms(p_lsm_ses->bytes_written,
                    &p_lsm_ses->common.config);

            ALOGD("%s: Real time frame received after %dms took %llums",
                __func__, bytes_written_ms,
                (frame_read_time / NSECS_PER_MSEC));
            adjust_ss_buff_end(&p_lsm_ses->common, cnn_append_bytes,
                vop_append_bytes);
            real_time_check = false;
        }
    }

    p_lsm_ses->lab_processing_active = false;
    ST_DBG_FILE_CLOSE(fptr_drv);
    ALOGVV("%s: Exit status=%d", __func__, status);
}

void process_packetized_lab_data(st_hw_session_lsm_t *p_ses)
{
    struct cpe_packet_hdr *prev_packet = NULL;
    bool first_sync_word_found = false;
    int sync_max_retry_cnt = SOUND_TRIGGER_SYNC_WORD_MAX_RETRY_CNT;
    int status = 0, dec_status = 0, out_samples = 0;
    unsigned int copy_bytes, dec_bytes, tmp_size;
    unsigned char *driver_rd_buf;
    unsigned int driver_rd_bytes, dr_buf_idx = 0;
    unsigned char *ftrt_buf_ptr;
    unsigned int ftrt_buf_size, ftrt_rem_bytes, ftrt_buf_idx = 0;
    unsigned char *frame_buf_ptr;
    unsigned int frame_buf_idx = 0;
    unsigned char *dec_out_buf;
    unsigned char packet_hdr[sizeof(struct cpe_packet_hdr)];
    unsigned int hdr_size = sizeof(struct cpe_packet_hdr);
    unsigned int hdr_buf_idx = 0;

    ST_DBG_DECLARE(FILE *fptr_drv = NULL, *fptr_pcm = NULL, *fptr_dec = NULL;
                   static int fcnt = 0, dec_fcnt = 0;);
    ST_DBG_FILE_OPEN_WR(fptr_drv, ST_DEBUG_DUMP_LOCATION,
                        "st_lab_drv_data_cpe", "bin", fcnt);
    ST_DBG_FILE_OPEN_WR(fptr_pcm, ST_DEBUG_DUMP_LOCATION,
                        "st_lab_output", "pcm", fcnt++);

    p_ses->lab_processing_active = true;
    p_ses->unread_bytes = 0;
    p_ses->bytes_written = 0;

    st_buffer_reset(p_ses->common.buffer);

    driver_rd_buf = p_ses->lab_drv_buf;
    driver_rd_bytes = p_ses->lab_drv_buf_size;

    /*
    1. Read from driver
    2. Find first sync word
    3. Skip MSB
    4. Skip Null packets
    5. Store in buffer
    */
    ST_DBG_ATRACE_ASYNC_BEGIN_IF(!(dbg_trace_lab_buf_cnt = 0),"sthal:lsm:cpe: lsm_buffer_read",
                                p_ses->common.sm_handle);
    while (!p_ses->exit_lab_processing) {

        ALOGVV("%s: pcm_read reading bytes=%d", __func__, driver_rd_bytes);
        status = pcm_read(p_ses->pcm, driver_rd_buf, driver_rd_bytes);
        ST_DBG_ATRACE_ASYNC_END_IF(++dbg_trace_lab_buf_cnt == dbg_trace_max_lab_reads,
                                    "sthal:lsm:cpe: lsm_buffer_read", p_ses->common.sm_handle);
        ALOGVV("%s: pcm_read done", __func__);

        ST_DBG_FILE_WRITE(fptr_drv, driver_rd_buf, driver_rd_bytes);

        if (status) {
            ALOGE("%s: pcm read failed status %d - %s", __func__, status,
                  pcm_get_error(p_ses->pcm));
            pcm_stop(p_ses->pcm);
            pcm_start(p_ses->pcm);
            break;
        }

        if (!first_sync_word_found) {
            first_sync_word_found = search_for_syncword(p_ses, driver_rd_buf,
               driver_rd_bytes, &dr_buf_idx);
            if (!first_sync_word_found) {
                if (--sync_max_retry_cnt <= 0) {
                    ALOGE("%s: max tries for finding sync word reached", __func__);
                    goto exit;
                } else {
                    /* Read next buffer from driver */
                    dr_buf_idx = 0;
                    continue;
                }
            } else {
                /* adpcm decoder needs to be initialized upon every utterance.
                   TODO: format check if compressed/uncompressed switch */
                if (CHECK_BIT(p_ses->common.vendor_uuid_info->kw_capture_format,
                              ADPCM)) {
                    ATRACE_BEGIN("sthal:lsm: adpcm_dec_init");
                    dec_status = p_ses->common.stdev->adpcm_dec_init(p_ses->adpcm_dec_state);
                    ATRACE_END();
                    if (dec_status) {
                        ALOGE("%s: ERROR. ADPCM decoder init failed status %d",
                               __func__, dec_status);
                        goto exit;
                    }
                    ST_DBG_FILE_OPEN_WR(fptr_dec, ST_DEBUG_DUMP_LOCATION,
                                        "st_lab_adpcm_input", "bin", dec_fcnt++);
                }
            }
        }
        get_ftrt_processing_data(p_ses, &driver_rd_buf[dr_buf_idx],
                                 driver_rd_bytes - dr_buf_idx,
                                 &ftrt_buf_ptr, &ftrt_buf_size);

        while (!p_ses->exit_lab_processing) {

            if (!prev_packet) {
                /* construct new packet */
                tmp_size = MIN((hdr_size - hdr_buf_idx), (ftrt_buf_size - ftrt_buf_idx));
                memcpy(&packet_hdr[hdr_buf_idx], &ftrt_buf_ptr[ftrt_buf_idx], tmp_size);
                hdr_buf_idx += tmp_size;
                ftrt_buf_idx += tmp_size;
                if (hdr_buf_idx == hdr_size) {
                    /* We have a new packet now, parse it */
                    prev_packet = (struct cpe_packet_hdr *)packet_hdr;
                    hdr_buf_idx = 0;
                    /* sync word must be present at packet boundary */
                    if (prev_packet->sync_word != CPE_PACKET_SYNC_WORD) {
                        ALOGE("%s: ERROR. sync word not present", __func__);
                        goto exit;
                    }
                    if (prev_packet->format == CPE_PACKET_FORMAT_EOS) {
                        ALOGVV("%s: EOS ??", __func__);
                        goto exit; /* TODO-V: Not expected now. FTRT to RT switch */
                    }
                }
            }

            if (prev_packet) {
                if (prev_packet->format == CPE_PACKET_FORMAT_NULL) {
                    /* Skip the NULL packets */
                    tmp_size = MIN(prev_packet->size, (ftrt_buf_size - ftrt_buf_idx));
                    ALOGVV("%s: null packet skip: size=%d ftrt_buf_idx=%d ftrt_buf_size=%d prev_packet->size=%d",
                          __func__, tmp_size, ftrt_buf_idx, ftrt_buf_size, prev_packet->size);
                    prev_packet->size -= tmp_size;
                    ftrt_buf_idx += tmp_size;
                } else if (prev_packet->format == CPE_PACKET_FORMAT_PCM) {
                    copy_bytes = MIN(prev_packet->size, (ftrt_buf_size - ftrt_buf_idx));
                    status = write_pcm_data_cpe(p_ses, &ftrt_buf_ptr[ftrt_buf_idx], copy_bytes);
                    if (status)
                        goto exit;

                    ST_DBG_FILE_WRITE(fptr_pcm, &ftrt_buf_ptr[ftrt_buf_idx], copy_bytes);
                    ftrt_buf_idx += copy_bytes;
                    prev_packet->size -= copy_bytes;
                } else if (prev_packet->format == CPE_PACKET_FORMAT_ADPCM) {
                    dec_out_buf = p_ses->dec_buf + ADPCM_MAX_IN_FRAME_SIZE;

                    while (!p_ses->exit_lab_processing &&
                           (ftrt_buf_idx < ftrt_buf_size) &&
                           (prev_packet->size > 0)) {

                        dec_bytes = MIN(prev_packet->size, ADPCM_MAX_IN_FRAME_SIZE);
                        ftrt_rem_bytes = ftrt_buf_size - ftrt_buf_idx;

                        if (!frame_buf_idx && (ftrt_rem_bytes >= dec_bytes)) {
                            /* There are no cached partial frame samples in frame buf.
                               We have full frame in ftrt buf for decoding. Use ftrt
                               buf directly as decoder input */
                            frame_buf_ptr = &ftrt_buf_ptr[ftrt_buf_idx];
                        } else {
                            /* Compute least of all buffer sizes to not to overflow */
                            dec_bytes = MIN(MIN(dec_bytes, ftrt_rem_bytes),
                                (ADPCM_MAX_IN_FRAME_SIZE - frame_buf_idx));
                            memcpy(&p_ses->dec_buf[frame_buf_idx], &ftrt_buf_ptr[ftrt_buf_idx], dec_bytes);
                            frame_buf_ptr = p_ses->dec_buf;
                        }
                        frame_buf_idx += dec_bytes;
                        ftrt_buf_idx += dec_bytes;
                        prev_packet->size -= dec_bytes;

                        ST_DBG_FILE_WRITE(fptr_dec, frame_buf_ptr, frame_buf_idx);
                        if (!prev_packet->size || (frame_buf_idx == ADPCM_MAX_IN_FRAME_SIZE)) {
                            /* if packet size is zero, we may have partial frame to be decoded */
                            ALOGVV("%s: enter for decode- frame_buf_idx=%d ftrt_buf_idx=%d prev_packet->size=%d",
                                   __func__, frame_buf_idx, ftrt_buf_idx, prev_packet->size);

                            /* Decode ADPCM frame and copy the data to pcm out buffer */
                            ATRACE_BEGIN("sthal:lsm: adpcm_dec_process");
                            dec_status = p_ses->common.stdev->adpcm_dec_process((short *)frame_buf_ptr,
                                                                  (short *)dec_out_buf,
                                                                  frame_buf_idx,
                                                                  &out_samples,
                                                                  p_ses->adpcm_dec_state);
                            ATRACE_END();
                            if (dec_status) {
                                ALOGE("%s: ERROR. adpcm dec failed status %d", __func__, dec_status);
                                goto exit;
                            }
                            ALOGVV("%s: adpcm_dec_process done. frame_buf_idx=%d out_samples=%d",
                                   __func__, frame_buf_idx, out_samples);
                            if (out_samples) {
                                ST_DBG_FILE_WRITE(fptr_pcm, dec_out_buf, out_samples << 1);
                                status = write_pcm_data_cpe(p_ses, dec_out_buf, out_samples << 1);
                                if (status)
                                    goto exit;
                            }
                            frame_buf_idx = 0;
                        }
                    }
                } else {
                    ALOGE("%s: Uknown packet format %d ", __func__, prev_packet->format);
                    goto exit;
                }

                if (!prev_packet->size) {
                    /* packet is processed, remove its reference */
                    prev_packet = NULL;
                }
            }

            if (ftrt_buf_idx == ftrt_buf_size) {
                ALOGVV("%s: ftrt buffer exhausted, read next buffer ftrt_buf_idx=%d",
                       __func__, ftrt_buf_idx);
                ftrt_buf_idx = dr_buf_idx = 0;
                break;
            }
        }
    }

exit:
    if (!p_ses->exit_lab_processing) {
        p_ses->exit_lab_processing = true;
        p_ses->common.fptrs->stop_buffering(&p_ses->common, true);
    }

    ST_DBG_FILE_CLOSE(fptr_drv);
    ST_DBG_FILE_CLOSE(fptr_dec);
    ST_DBG_FILE_CLOSE(fptr_pcm);

    ALOGVV("%s: Exit status=%d", __func__, status);
}

void process_raw_lab_data_ape(st_hw_session_lsm_t *p_lsm_ses)
{
    int status = 0;
    struct listnode *node = NULL, *tmp_node = NULL;
    st_arm_second_stage_t *st_sec_stage = NULL;
    unsigned int prepend_bytes = 0, cnn_append_bytes = 0, vop_append_bytes = 0;
    unsigned int kw_duration_bytes = 0;
    bool real_time_check = false;
    uint64_t frame_receive_time = 0, frame_send_time = 0;
    uint64_t frame_read_time = 0;

    ST_DBG_DECLARE(FILE *fptr_drv = NULL; static int file_cnt = 0);
    ST_DBG_FILE_OPEN_WR(fptr_drv, ST_DEBUG_DUMP_LOCATION,
                        "st_lab_drv_data_ape", "pcm", file_cnt++);

    pthread_mutex_lock(&p_lsm_ses->lock);
    p_lsm_ses->lab_processing_active = true;
    p_lsm_ses->unread_bytes = 0;
    p_lsm_ses->bytes_written = 0;

    st_buffer_reset(p_lsm_ses->common.buffer);

    if (p_lsm_ses->common.enable_second_stage) {
        prepend_bytes =
            convert_ms_to_bytes(
                p_lsm_ses->common.vendor_uuid_info->kw_start_tolerance,
                &p_lsm_ses->common.config);
        if (p_lsm_ses->common.client_req_hist_buf) {
            kw_duration_bytes =
                convert_ms_to_bytes(p_lsm_ses->common.client_req_hist_buf,
                    &p_lsm_ses->common.config);
        } else {
            kw_duration_bytes =
                convert_ms_to_bytes(p_lsm_ses->common.vendor_uuid_info->kw_duration,
                    &p_lsm_ses->common.config);
        }

        list_for_each_safe(node, tmp_node, p_lsm_ses->common.second_stage_list) {
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
            if (p_lsm_ses->common.kw_start_idx > prepend_bytes) {
                st_sec_stage->ss_session->buf_start =
                    p_lsm_ses->common.kw_start_idx - prepend_bytes;
            } else {
                st_sec_stage->ss_session->buf_start = 0;
            }

            if (st_sec_stage->ss_info->sm_detection_type ==
                ST_SM_TYPE_KEYWORD_DETECTION) {
                cnn_append_bytes =
                    convert_ms_to_bytes(
                        (p_lsm_ses->common.vendor_uuid_info->kw_end_tolerance +
                         st_sec_stage->ss_info->data_after_kw_end),
                        &p_lsm_ses->common.config);

                if (p_lsm_ses->common.kw_end_idx < kw_duration_bytes) {
                    st_sec_stage->ss_session->buf_end =
                        p_lsm_ses->common.kw_end_idx + cnn_append_bytes;
                } else {
                    st_sec_stage->ss_session->buf_end = kw_duration_bytes +
                        cnn_append_bytes;
                }
                st_sec_stage->ss_session->buff_sz = p_lsm_ses->lab_drv_buf_size;
                st_sec_stage->ss_session->det_status = KEYWORD_DETECTION_PENDING;
            } else if (st_sec_stage->ss_info->sm_detection_type ==
                ST_SM_TYPE_USER_VERIFICATION) {
                vop_append_bytes =
                    convert_ms_to_bytes(
                        p_lsm_ses->common.vendor_uuid_info->kw_end_tolerance,
                        &p_lsm_ses->common.config);

                if ((p_lsm_ses->common.kw_end_idx + vop_append_bytes) <
                    kw_duration_bytes) {
                    st_sec_stage->ss_session->buf_end =
                        p_lsm_ses->common.kw_end_idx + vop_append_bytes;
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

        if (p_lsm_ses->common.enable_second_stage &&
            !p_lsm_ses->common.client_req_hist_buf)
            p_lsm_ses->move_client_ptr = true;
        else
            p_lsm_ses->move_client_ptr = false;

        if (!p_lsm_ses->common.is_generic_event)
            real_time_check = true;
    }

    ST_DBG_ATRACE_ASYNC_BEGIN_IF(!(dbg_trace_lab_buf_cnt = 0),"sthal:lsm:ape: lsm_buffer_read",
                                    p_lsm_ses->common.sm_handle);

    while (!p_lsm_ses->exit_lab_processing) {
        ALOGVV("%s: pcm_read reading bytes=%d", __func__, p_lsm_ses->lab_drv_buf_size);
        pthread_mutex_unlock(&p_lsm_ses->lock);
        frame_send_time = get_current_time_ns();
        status = pcm_read(p_lsm_ses->pcm, p_lsm_ses->lab_drv_buf, p_lsm_ses->lab_drv_buf_size);
        pthread_mutex_lock(&p_lsm_ses->lock);
        ST_DBG_ATRACE_ASYNC_END_IF((++dbg_trace_lab_buf_cnt == dbg_trace_max_lab_reads),
                                    "sthal:lsm:ape: lsm_buffer_read", p_lsm_ses->common.sm_handle);
        frame_receive_time = get_current_time_ns();

        ALOGVV("%s: pcm_read done", __func__);
        ST_DBG_FILE_WRITE(fptr_drv, p_lsm_ses->lab_drv_buf,
            p_lsm_ses->lab_drv_buf_size);

        if (status) {
            ALOGE("%s: pcm read failed status %d - %s", __func__, status,
                  pcm_get_error(p_lsm_ses->pcm));
            pcm_stop(p_lsm_ses->pcm);
            pcm_start(p_lsm_ses->pcm);
            break;
        }

        write_pcm_data_ape(p_lsm_ses, p_lsm_ses->lab_drv_buf, p_lsm_ses->lab_drv_buf_size);
        frame_read_time = frame_receive_time - frame_send_time;
        if (real_time_check &&
            (frame_read_time > APE_MAX_LAB_FTRT_FRAME_RD_TIME_NS)) {
            uint32_t bytes_written_ms =
                convert_bytes_to_ms(p_lsm_ses->bytes_written,
                    &p_lsm_ses->common.config);

            ALOGD("%s: Real time frame received after %dms took %llums",
                __func__, bytes_written_ms,
                (frame_read_time / NSECS_PER_MSEC));
            adjust_ss_buff_end(&p_lsm_ses->common, cnn_append_bytes,
                vop_append_bytes);
            real_time_check = false;
        }
    }

    ape_stop_buffering(&p_lsm_ses->common);
    p_lsm_ses->lab_processing_active = false;
    pthread_cond_broadcast(&p_lsm_ses->cond);
    pthread_mutex_unlock(&p_lsm_ses->lock);
    ST_DBG_FILE_CLOSE(fptr_drv);
    ALOGVV("%s: Exit status=%d", __func__, status);
}

static void *callback_thread_loop(void *context)
{
    st_hw_session_lsm_t *p_lsm_ses =
                                 (st_hw_session_lsm_t *)context;
    st_lsm_event_status_t *params;
    char *st_lsm_event_cmd = NULL;
    unsigned int payload_alloc_size = SOUND_TRIGGER_MAX_EVNT_PAYLOAD_SIZE;
    int status = 0;
    int event_status, request;
    st_hw_sess_event_t hw_sess_event; /* used to report event to st_session */

    if (p_lsm_ses == NULL) {
        ALOGE("%s: ERROR. null context.. exiting", __func__);
        return NULL;
    }
    ALOGD("%s:[%d] Enter", __func__, p_lsm_ses->common.sm_handle);

    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_DEFAULT);
    prctl(PR_SET_NAME, (unsigned long)"sound trigger callback", 0, 0, 0);

    pthread_mutex_lock(&p_lsm_ses->callback_thread_lock);
    params = calloc(1, sizeof(*params) + payload_alloc_size);
    if (params == NULL) {
        ALOGE("%s: ERROR. insufficient memory for payload", __func__);
        goto exit;
    }

    set_lsm_fwk_mode(p_lsm_ses);
    update_lsm_event_status_info(p_lsm_ses, &request, &st_lsm_event_cmd);

    while (!p_lsm_ses->exit_callback_thread) {
        params->payload_size = payload_alloc_size;
        ALOGI("%s:[%d] Waiting for %s",
               __func__, p_lsm_ses->common.sm_handle, st_lsm_event_cmd);
        pthread_mutex_unlock(&p_lsm_ses->callback_thread_lock);
        if (p_lsm_ses->common.is_generic_event)
            status = pcm_ioctl(p_lsm_ses->pcm, request, &params->status);
        else
            status = pcm_ioctl(p_lsm_ses->pcm, request, params);
        pthread_mutex_lock(&p_lsm_ses->callback_thread_lock);

        ALOGI("%s:[%d] Received %s status=%d",
              __func__, p_lsm_ses->common.sm_handle, st_lsm_event_cmd, status);
        if (status < 0) {
            if (errno == ENOMEM) {
                payload_alloc_size = payload_alloc_size << 1;
                params = realloc(params, sizeof(*params) + payload_alloc_size);

                if (params == NULL) {
                    ALOGE("%s: ERROR. Not enough memory for payload",
                          __func__);
                    p_lsm_ses->exit_callback_thread = true;
                    break;
                }
                continue;
            } else {
                ALOGE("%s: ERROR. %s failed status %d",
                    __func__, st_lsm_event_cmd, status);
                p_lsm_ses->exit_callback_thread = true;
                break;
            }
        }
        if (p_lsm_ses->exit_callback_thread)
            break;

        ALOGD("%s: params status %d", __func__, params->status);
        switch (params->status) {
        case LSM_VOICE_WAKEUP_STATUS_RUNNING:
            continue;
        case LSM_VOICE_WAKEUP_STATUS_DETECTED:
            /*
             * Currently, DSP does not support the inclusion of detection
             * timestamp within the payload. So the timestamp is filled here
             * instead.
             */
            p_lsm_ses->common.first_stage_det_event_time = get_current_time_ns();
            /* The duration of this trace log indicates the detection latency. */
            ATRACE_ASYNC_BEGIN("sthal: detection success",
                p_lsm_ses->common.sm_handle);
            ATRACE_ASYNC_BEGIN("sthal: detection reject",
                p_lsm_ses->common.sm_handle);
            event_status = RECOGNITION_STATUS_SUCCESS;
            /*
             * note: since now we are dispatching the detected event to the
             * state machine there is no need to check if buffering enabled or
             * concurrency is active here, it will now be handled by the st_session
             * state machine
             */
            break;

        case LSM_VOICE_WAKEUP_STATUS_END_SPEECH:
        case LSM_VOICE_WAKEUP_STATUS_REJECTED:
            event_status = RECOGNITION_STATUS_FAILURE;
            break;
        default:
            ALOGW("%s: Unknown status %d", __func__, params->status);
            continue;
        }

        /* inform st_sessoin of the event */
        hw_sess_event.event_id = ST_HW_SESS_EVENT_DETECTED;
        hw_sess_event.payload.detected.timestamp = get_event_timestamp(params);
        hw_sess_event.payload.detected.detect_status = event_status;
        if (p_lsm_ses->common.is_generic_event &&
            p_lsm_ses->common.vendor_uuid_info->is_qcmd_uuid) {
            unsigned int payload_offset = sizeof(struct st_param_header) +
                                          GENERIC_DET_EVENT_HEADER_SIZE;
            hw_sess_event.payload.detected.detect_payload =
                (void*)((unsigned char *)params->payload + payload_offset);
            hw_sess_event.payload.detected.payload_size =
                params->payload_size - payload_offset;
        } else {
            hw_sess_event.payload.detected.detect_payload =
                (void*)params->payload;
            hw_sess_event.payload.detected.payload_size = params->payload_size;
        }

        ST_DBG_DECLARE(FILE *detect_fd = NULL; static int detect_fd_cnt = 0);
        ST_DBG_FILE_OPEN_WR(detect_fd, ST_DEBUG_DUMP_LOCATION,
                            "lsm_detection_event", "bin", detect_fd_cnt++);
        ST_DBG_FILE_WRITE(detect_fd, hw_sess_event.payload.detected.detect_payload,
                          hw_sess_event.payload.detected.payload_size);
        ST_DBG_FILE_CLOSE(detect_fd);

        pthread_mutex_unlock(&p_lsm_ses->callback_thread_lock);
        p_lsm_ses->common.callback_to_st_session(&hw_sess_event,
                p_lsm_ses->common.cookie);
        pthread_mutex_lock(&p_lsm_ses->callback_thread_lock);
    }

    if (st_lsm_event_cmd)
        free(st_lsm_event_cmd);

    if (params)
        free(params);

exit:
    pthread_cond_signal(&p_lsm_ses->callback_thread_cond);
    pthread_mutex_unlock(&p_lsm_ses->callback_thread_lock);
    ALOGD("%s:[%d] Exit", __func__, p_lsm_ses->common.sm_handle);
    return NULL;
}

static int deallocate_lab_buffers_ape(st_hw_session_lsm_t* p_ses)
{
    ALOGVV("%s:[%d] Enter", __func__, p_ses->common.sm_handle);
    st_buffer_deinit(p_ses->common.buffer);
    p_ses->common.buffer = NULL;
    if (p_ses->lab_drv_buf) {
        free(p_ses->lab_drv_buf);
        p_ses->lab_drv_buf = NULL;
    }
    p_ses->lab_buffers_allocated = false;

    return 0;
}

static int deallocate_lab_buffers_cpe(st_hw_session_lsm_t* p_ses)
{
    ALOGVV("%s:[%d] Enter", __func__, p_ses->common.sm_handle);
    if (p_ses->adpcm_dec_state) {
        free(p_ses->adpcm_dec_state);
        p_ses->adpcm_dec_state = NULL;
    }
    if (p_ses->dec_buf) {
        free(p_ses->dec_buf);
        p_ses->dec_buf = NULL;
    }
    st_buffer_deinit(p_ses->common.buffer);
    p_ses->common.buffer = NULL;
    if (p_ses->lab_drv_buf) {
        free(p_ses->lab_drv_buf);
        p_ses->lab_drv_buf = NULL;
    }
    p_ses->lab_buffers_allocated = false;

    return 0;
}

static int configure_lab(st_hw_session_lsm_t* p_ses, bool capture_requested)
{
    int status = 0, buf_en = 0;
    struct snd_lsm_output_format_cfg media_cfg;
    struct st_vendor_info *v_info = p_ses->common.vendor_uuid_info;

    media_cfg.format = LSM_OUT_FORMAT_PCM;
    media_cfg.packing = LSM_OUT_DATA_RAW;
    media_cfg.mode = LSM_OUT_TRANSFER_MODE_RT;
    media_cfg.events = LSM_OUT_DATA_EVENTS_DISABLED;

    if (v_info && (v_info->kw_transfer_mode == FTRT_TRANSFER_MODE)) {
        media_cfg.packing = LSM_OUT_DATA_PACKED;
        media_cfg.mode = LSM_OUT_TRANSFER_MODE_FTRT;

        if (CHECK_BIT(v_info->kw_capture_format, ADPCM))
            media_cfg.format = LSM_OUT_FORMAT_ADPCM;
    }

    ALOGD("%s: media_cfg format=%d, packing=%d, events=%d, mode=%d", __func__,
       media_cfg.format, media_cfg.packing, media_cfg.events, media_cfg.mode);
    ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_out_format_cfg)");
    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_OUT_FORMAT_CFG, &media_cfg);
    ATRACE_END();
    if (status) {
        ALOGE("%s: ERROR. SNDRV_LSM_OUT_FORMAT_CFG, status=%d", __func__,
           status);
        return status;
    }

    buf_en = capture_requested ? 1 : 0;
    ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_lab_control");
    status = pcm_ioctl(p_ses->pcm, SNDRV_LSM_LAB_CONTROL, &buf_en);
    ATRACE_END();
    if (status) ALOGE("%s: ERROR. SNDRV_LSM_LAB_CONTROL failed, status=%d",
        __func__, status);

    return status;
}

static int allocate_lab_buffers_cpe(st_hw_session_lsm_t* p_ses)
{
    int status = 0, circ_buff_sz = 0;
    int adpcm_dec_scratch_size;
    unsigned int kw_tranfer_duration, rt_bytes_one_sec, ftrt_to_rt_factor;
    struct st_vendor_info *v_info = p_ses->common.vendor_uuid_info;

    ALOGVV("%s:[%d] Enter", __func__, p_ses->common.sm_handle);

    p_ses->lab_drv_buf_size = pcm_frames_to_bytes(p_ses->pcm, p_ses->lab_config.period_size);
    p_ses->lab_drv_buf = (unsigned char *)calloc(1, p_ses->lab_drv_buf_size);
    if (!p_ses->lab_drv_buf) {
        ALOGE("%s: ERROR. Can not allocate lab buffer size %d", __func__, p_ses->lab_drv_buf_size);
        status = -ENOMEM;
        goto error_exit;
    }
    ALOGV("%s: Allocated lab buffer period size bytes =%d",
          __func__, p_ses->lab_drv_buf_size);

    rt_bytes_one_sec = (p_ses->common.config.rate * p_ses->common.config.channels *
                        (pcm_format_to_bits(p_ses->common.config.format) >> 3));

    circ_buff_sz = (v_info->client_capture_read_delay * rt_bytes_one_sec) / 1000;
    if (v_info->kw_transfer_mode == FTRT_TRANSFER_MODE) {
        ftrt_to_rt_factor = (p_ses->lab_config.rate * pcm_format_to_bits(p_ses->lab_config.format)) /
                            (p_ses->common.config.rate * pcm_format_to_bits(p_ses->common.config.format));

        kw_tranfer_duration = v_info->kw_duration / ftrt_to_rt_factor;
        ALOGV("%s: kw_tranfer_duration %d, kw_duration %d, ftrt_to_rt_factor %d",
               __func__, kw_tranfer_duration, v_info->kw_duration, ftrt_to_rt_factor);
        if (kw_tranfer_duration < v_info->client_capture_read_delay) {
            /* Full keyword gets transmitted within App delay, accomodate full
               keyword duration size plus the client read delay time of real time data size.
               Ex: 2sec keyword duration at 384KHz/32bit FTRT speed gets transfered in ~40ms.
               If configured App delay is 1sec, accomdate realtime data for 1sec delay in
               addition to 2sec keyword data. So the buffer size becomes
               (2000ms + 1000ms) * 32KB/1000ms = ~96KB */
            circ_buff_sz = (v_info->kw_duration + v_info->client_capture_read_delay) *
                                      rt_bytes_one_sec / 1000;
        } else {
            /* Accomdate client read delay worth of ftrt keyword data.
               Ex: 2sec keyword duration at 16KHz/32bit FTRT speed gets transfered
               in 1000ms. If configured App delay is 500ms, all that we need to store
               is 500ms worth for FTRT data. So the buffer size becomes
               (500ms * 2) * 32KB/1000ms = 32KB */
            circ_buff_sz *= ftrt_to_rt_factor;
        }
    }

    p_ses->common.buffer = st_buffer_init(circ_buff_sz);
    if (!p_ses->common.buffer) {
        ALOGE("%s: failed to allocate circ buffer", __func__);
        status = -ENOMEM;
        goto error_exit;
    }

    ALOGV("%s: Allocated out buffer size=%d", __func__, circ_buff_sz);

    if (CHECK_BIT(v_info->kw_capture_format, ADPCM)) {
        p_ses->dec_buf = (unsigned char *)calloc(1, ADPCM_MAX_IN_FRAME_SIZE +
                                                  ADPCM_MAX_OUT_FRAME_SIZE);
        if (!p_ses->dec_buf) {
            ALOGE("%s: ERROR. Can not allocate decoder buffer size %d",
                  __func__, ADPCM_MAX_IN_FRAME_SIZE + ADPCM_MAX_OUT_FRAME_SIZE);
            status = -ENOMEM;
            goto error_exit;
        }
        ALOGV("%s: Allocated decoder buffers size=%d", __func__,
               ADPCM_MAX_IN_FRAME_SIZE + ADPCM_MAX_OUT_FRAME_SIZE);

        /* Instantiate the decoder */
        status = p_ses->common.stdev->adpcm_dec_get_scratch_size(&adpcm_dec_scratch_size);
        if (status) {
            ALOGE("%s: ERROR. Can not get adpcm dec scratch", __func__);
            status = -EINVAL;
            goto error_exit;
        }

        p_ses->adpcm_dec_state = (void *)calloc(1, adpcm_dec_scratch_size);
        if (!p_ses->adpcm_dec_state)
        {
            ALOGE("%s: ERROR. Can not allocate decoder scratch", __func__);
            status = -ENOMEM;
            goto error_exit;
        }
        ALOGV("%s: Allocated adpcm_dec_state size=%d", __func__,
              adpcm_dec_scratch_size);
    }
    p_ses->lab_buffers_allocated = true;

    return 0;

error_exit:
    deallocate_lab_buffers_cpe(p_ses);
    return status;
}

static int allocate_lab_buffers_ape(st_hw_session_lsm_t* p_lsm_ses)
{
    int status = 0, circ_buff_sz = 0;
    struct st_vendor_info *v_info = p_lsm_ses->common.vendor_uuid_info;
    unsigned int rt_bytes_one_sec;

    p_lsm_ses->lab_drv_buf_size = pcm_frames_to_bytes(p_lsm_ses->pcm,
                                                      p_lsm_ses->common.config.period_size);
    p_lsm_ses->lab_drv_buf = (unsigned char *)calloc(1, p_lsm_ses->lab_drv_buf_size);
    if (!p_lsm_ses->lab_drv_buf) {
        ALOGE("%s: ERROR. Can not allocate lab buffer size %d", __func__,
            p_lsm_ses->lab_drv_buf_size);
        status = -ENOMEM;
        goto error_exit;
    }
    ALOGV("%s: Allocated lab buffer period size bytes =%d",
        __func__, p_lsm_ses->lab_drv_buf_size);

    rt_bytes_one_sec = (p_lsm_ses->common.config.rate *
                        p_lsm_ses->common.config.channels *
                        (pcm_format_to_bits(p_lsm_ses->common.config.format) >> 3));

    if ((p_lsm_ses->common.client_req_hist_buf +
         p_lsm_ses->common.client_req_preroll) > v_info->kw_duration) {
        circ_buff_sz = ((p_lsm_ses->common.client_req_hist_buf +
            p_lsm_ses->common.client_req_preroll +
            v_info->client_capture_read_delay) * rt_bytes_one_sec) / 1000;
    } else {
        circ_buff_sz = ((v_info->kw_duration +
            v_info->client_capture_read_delay) * rt_bytes_one_sec) / 1000;
    }
    p_lsm_ses->common.buffer = st_buffer_init(circ_buff_sz);
    if (!p_lsm_ses->common.buffer) {
        ALOGE("%s: failed to allocate circ buffer", __func__);
        status = -ENOMEM;
        goto error_exit;
    }

    ALOGV("%s: Allocated out buffer size=%d", __func__, circ_buff_sz);
    p_lsm_ses->lab_buffers_allocated = true;

    return status;

error_exit:
    deallocate_lab_buffers_ape(p_lsm_ses);
    return status;
}

static int sound_trigger_set_device
(
   st_hw_session_t* p_ses,
   bool enable
)
{
    char st_device_name[DEVICE_NAME_MAX_SIZE] = { 0 };
    int ref_cnt_idx = 0, ref_cnt = 0;
    int status = 0;
    st_device_t st_device = 0;
    audio_devices_t capture_device = 0;
    int app_type = 0;
    st_profile_type_t profile_type = 0;
    bool is_hwmad_device = false;
    struct st_vendor_info *v_info = p_ses->vendor_uuid_info;
    bool backend_cfg_change = true;

    is_hwmad_device = platform_stdev_is_hwmad_backend(p_ses->stdev->platform,
        p_ses->st_device, p_ses->exec_mode);

    profile_type = get_profile_type(p_ses);
    if (enable) {
        status = platform_stdev_check_and_set_codec_backend_cfg(
                                                 p_ses->stdev->platform,
                                                 v_info,
                                                 &backend_cfg_change,
                                                 p_ses->stdev->lpi_enable,
                                                 p_ses->stdev->vad_enable,
                                                 p_ses->client_req_preroll);
        if (status) {
            ALOGE("%s: ERROR. codec backend config update failed, status=%d",
                __func__, status);
            return status;
        }

        capture_device = platform_stdev_get_capture_device(p_ses->stdev->platform);
        st_device = platform_stdev_get_device(p_ses->stdev->platform,
            p_ses->vendor_uuid_info, capture_device, p_ses->exec_mode);

        if (platform_stdev_get_device_name(p_ses->stdev->platform, p_ses->exec_mode,
                st_device, st_device_name) < 0) {
            ALOGE("%s: Invalid sound trigger device name returned", __func__);
            return -EINVAL;
        }

        /*
         * Send device cal irrespective of ref cnt as driver expects
         * this to be sent per instance.
         */

        pthread_mutex_lock(&p_ses->stdev->ref_cnt_lock);
        ref_cnt_idx = (p_ses->exec_mode * ST_DEVICE_MAX) + st_device;
        ref_cnt = ++(p_ses->stdev->dev_ref_cnt[ref_cnt_idx]);
        app_type = platform_stdev_get_device_app_type(p_ses->stdev->platform,
                                                      profile_type);

        status = platform_stdev_send_calibration(p_ses->stdev->platform,
            capture_device,
            p_ses->exec_mode,
            p_ses->vendor_uuid_info,
            app_type,
            false, ST_DEVICE_CAL);

        if (!status) {
            if (1 == ref_cnt) {
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
                    ALOGV("%s: conc capture supported, reset device controls (%x) = %s",
                           __func__, p_ses->st_device, p_ses->st_device_name);
                    audio_route_reset_and_update_path(p_ses->stdev->audio_route,
                        st_device_name);
                }

                ALOGD("%s: enable device (%x) = %s", __func__, st_device,
                      st_device_name);
                ATRACE_BEGIN("sthal:lsm: audio_route_apply_and_update_path");
                audio_route_apply_and_update_path(p_ses->stdev->audio_route,
                                                  st_device_name);
                ATRACE_END();
                update_hw_mad_exec_mode(p_ses->exec_mode, profile_type);
            }
        } else {
            --(p_ses->stdev->dev_ref_cnt[ref_cnt_idx]);
            ALOGE("%s: failed to send calibration %d", __func__, status);
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
                ATRACE_BEGIN("sthal:lsm: audio_route_reset_and_update_path");
                audio_route_reset_and_update_path(p_ses->stdev->audio_route,
                                                  p_ses->st_device_name);
                ATRACE_END();
                update_hw_mad_exec_mode(ST_EXEC_MODE_NONE, profile_type);
            } else {
                ALOGD("%s: Non-hwmad device, concurrent capture on, do not disable", __func__);
            }
        }
        pthread_mutex_unlock(&p_ses->stdev->ref_cnt_lock);
        free(p_ses->st_device_name);
    }
    return status;
}

static int ape_reg_sm(st_hw_session_t *p_ses, void *sm_data,
    sound_trigger_sound_model_type_t sm_type)
{
    int status = 0, param_count = 0, stage_idx = 0;
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t*)p_ses;
    struct st_vendor_info *v_info = p_ses->vendor_uuid_info;
    struct sound_trigger_phrase_sound_model *phrase_sm =
       (struct sound_trigger_phrase_sound_model*)sm_data;
    struct sound_trigger_sound_model *common_sm=
       (struct sound_trigger_sound_model*)sm_data;
    struct snd_lsm_session_data ses_data;
    struct snd_lsm_module_params lsm_params;
    lsm_param_info_t param_info[LSM_MAX_STAGES_PER_SESSION];
    st_lsm_det_event_type_t det_event_type;
    pthread_attr_t attr;
    struct listnode *node = NULL;
    st_lsm_ss_config_t *ss_cfg = NULL;
    struct st_module_param_info *mparams = NULL;
    audio_devices_t capture_device = 0;

    ALOGD("%s:[%d] Enter vinfo %p", __func__, p_ses->sm_handle, v_info);

    p_lsm_ses->pcm_id = platform_ape_get_pcm_device_id(p_ses->stdev->platform,
        &p_ses->use_case_idx);
    if (p_lsm_ses->pcm_id < 0)
        return -ENODEV;

    platform_get_lsm_usecase(p_ses->stdev->platform, v_info,
                             &p_lsm_ses->lsm_usecase, p_ses->exec_mode);
    if (!p_lsm_ses->lsm_usecase) {
        ALOGE("%s: couldn't get lsm usecase", __func__);
        goto sm_error;
    }

    p_lsm_ses->num_stages = 1;
    list_for_each(node, &p_ses->lsm_ss_cfg_list) {
        p_lsm_ses->num_stages++;
    }
    if ((p_lsm_ses->num_stages > 1) &&
        !st_hw_check_multi_stage_lsm_support()) {
        ALOGE("%s: lsm driver/dsp support for mult-stage(%d) session is missing",
              __func__, p_lsm_ses->num_stages);
        goto sm_error;
    }

    /* Backward compatibility with previous XML version <= 0x0102 */
    int app_type = (v_info->app_type == 0) ?
        p_lsm_ses->lsm_usecase->app_type : v_info->app_type;

    capture_device = platform_stdev_get_capture_device(p_ses->stdev->platform);
    status = platform_stdev_send_calibration(p_ses->stdev->platform,
                                              capture_device,
                                              p_ses->exec_mode,
                                              p_ses->vendor_uuid_info,
                                              app_type, true,
                                              ST_SESSION_CAL);

    if (status) {
        ALOGE("%s: ERROR. sending calibration failed status %d, idx 0",
              __func__, status);
        goto sm_error;
    }

    if (p_lsm_ses->num_stages > 1) {
        /*
         * Send cal for additional stages, if any,
         * Start cal index from One as first index is already taken care.
         */
        int index = ST_CAL_INDEX_0;
        list_for_each(node, &p_ses->lsm_ss_cfg_list) {
            index++;
            ss_cfg = node_to_item(node, st_lsm_ss_config_t, list_node);
            status = platform_stdev_send_calibration_v2(p_ses->stdev->platform,
                         capture_device, p_ses->exec_mode,
                         p_ses->vendor_uuid_info, ss_cfg->params->app_type,
                         true, ST_SESSION_CAL, index);
            if (status) {
                ALOGE("%s: ERROR. sending calibration failed status %d idx %d",
                      __func__, status, index);
                goto sm_error;
            }
            ALOGV("%s: sent lsm cal for app_type=%d idx %d cal status=%d",
                __func__, ss_cfg->params->app_type, index, status);
        }
    }

    p_lsm_ses->common.config = stdev_ape_pcm_config;

    platform_stdev_check_and_update_pcm_config(p_ses->stdev->platform,
                                               &p_lsm_ses->common.config,
                                               v_info, p_ses->exec_mode);

    ALOGD("%s: opening pcm device=%d", __func__, p_lsm_ses->pcm_id);
    ALOGV("%s: config: channels=%d rate=%d, period_size=%d, period_cnt=%d, format=%d",
          __func__, p_lsm_ses->common.config.channels, p_lsm_ses->common.config.rate,
          p_lsm_ses->common.config.period_size, p_lsm_ses->common.config.period_count,
          p_lsm_ses->common.config.format);
    ATRACE_BEGIN("sthal:lsm: pcm_open");
    p_lsm_ses->pcm = pcm_open(p_ses->stdev->snd_card, p_lsm_ses->pcm_id,
                          PCM_IN, &p_lsm_ses->common.config);
    ATRACE_END();
    if (!p_lsm_ses->pcm) {
        ALOGE("%s: ERROR. pcm_open failed", __func__);
        status = -ENODEV;
        goto sm_error;
    }
    if (!pcm_is_ready(p_lsm_ses->pcm)) {
        ALOGE("%s: ERROR. pcm_is_ready failed err=%s", __func__,
              pcm_get_error(p_lsm_ses->pcm));
        status = -ENODEV;
        goto sm_error;
    }

    /*
     * Using V2 APIs when multi-stage support is available in driver,
     * this would avoid confusion on code flow,
     * as only either legacy or V2 APIs would be used for a particular platform.
     */
    if (st_hw_check_multi_stage_lsm_support()) {
        status = lsm_set_session_data_v2(p_ses);
        if (status)
            goto sm_error;
    } else {
        /* Although app_id is valid only for non-topology solution. The driver
           opens DSP LSM session with this ioctl cmd */
        ses_data.app_id = LSM_VOICE_WAKEUP_APP_ID_V2;
        ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_set_session_data");
        status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_SET_SESSION_DATA, &ses_data);
        ATRACE_END();
        if (status) {
            ALOGE("%s: ERROR. SNDRV_LSM_SET_SESSION_DATA failed status %d",
                  __func__, status);
            goto sm_error;
        }
    }

    /* Send Reg SM param for each stages */
    param_count = 0;
    stage_idx = LSM_STAGE_INDEX_FIRST;
    lsm_params.params = (unsigned char*)&param_info[0];
    mparams = p_lsm_ses->lsm_usecase->params;

    if (sm_type == SOUND_MODEL_TYPE_KEYPHRASE) {
        param_info[param_count].param_size = phrase_sm->common.data_size;
        param_info[param_count].param_data =
            (unsigned char*)phrase_sm + phrase_sm->common.data_offset;
    } else {
        param_info[param_count].param_size = common_sm->data_size;
        param_info[param_count].param_data =
            (unsigned char*)common_sm + common_sm->data_offset;
    }
    lsm_fill_param_info(LSM_REG_SND_MODEL, &param_info[param_count++],
                        &mparams[LOAD_SOUND_MODEL], stage_idx);

    list_for_each(node, &p_ses->lsm_ss_cfg_list) {
        ss_cfg = node_to_item(node, st_lsm_ss_config_t, list_node);
        mparams = ss_cfg->params->params;
        stage_idx++;

        param_info[param_count].param_size = ss_cfg->sm_size;
        param_info[param_count].param_data = ss_cfg->sm_data;
        lsm_fill_param_info(LSM_REG_SND_MODEL, &param_info[param_count++],
                            &mparams[LOAD_SOUND_MODEL], stage_idx);
    }

    lsm_params.num_params = param_count;
    lsm_params.data_size =
        lsm_params.num_params * sizeof(lsm_param_info_t);
    status = lsm_set_module_params(p_lsm_ses, &lsm_params);
    if (status) {
        ALOGE("%s: ERROR. registering sound models. status %d",
              __func__, status);
        goto sm_error;
    }

    /* Send detection event type for last stage only, if params set in config */
    mparams = NULL;
    if ((p_lsm_ses->num_stages == 1) &&
        (p_lsm_ses->lsm_usecase->param_tag_tracker & PARAM_DET_EVENT_TYPE_BIT)) {
        mparams = p_lsm_ses->lsm_usecase->params;
    } else if (p_lsm_ses->num_stages > 1) {
        node = list_tail(&p_ses->lsm_ss_cfg_list);
        ss_cfg = node_to_item(node, st_lsm_ss_config_t, list_node);
        if (ss_cfg->params->param_tag_tracker & PARAM_DET_EVENT_TYPE_BIT)
            mparams = ss_cfg->params->params;
    }
    if (mparams) {
        stage_idx = LSM_STAGE_INDEX_FIRST + p_lsm_ses->num_stages - 1;
        if (fill_lsm_det_event_type_params(&det_event_type,
                    &param_info[0], mparams, stage_idx)) {
            p_ses->is_generic_event = true;

            lsm_params.num_params = 1;
            lsm_params.params = (unsigned char*)&param_info[0];
            lsm_params.data_size = sizeof(lsm_param_info_t);
            status = lsm_set_module_params(p_lsm_ses, &lsm_params);
            if (status) {
                ALOGE("%s: ERROR. setting detection event type. status %d",
                      __func__, status);
                goto sm_error;
            }
        }
    }

    /* Callback thread to wait on event detection */
    p_lsm_ses->exit_callback_thread = false;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&p_lsm_ses->callback_thread, &attr,
                        callback_thread_loop, p_lsm_ses);

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_lsm_ses->common.sm_handle,
        status);
    return 0;

sm_error:
    platform_ape_free_pcm_device_id(p_ses->stdev->platform, p_lsm_ses->pcm_id);
    if (p_lsm_ses->pcm) {
        pcm_close(p_lsm_ses->pcm);
        p_lsm_ses->pcm = NULL;
    }

    ALOGD("%s:[%d] Exit, status=%d", __func__,
        p_lsm_ses->common.sm_handle, status);
    return status;
}

static int ape_dereg_sm(st_hw_session_t *p_ses, bool capture_requested)
{
    int status = 0, buf_en = 0;
    struct snd_lsm_module_params lsm_params;
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;
    struct st_module_param_info *mparams = NULL;
    lsm_param_info_t param_info[LSM_MAX_STAGES_PER_SESSION];
    struct listnode *node = NULL;
    st_lsm_ss_config_t *ss_cfg = NULL;
    int stage_idx, param_count;

    ALOGD("%s:[%d] Enter", __func__, p_lsm_ses->common.sm_handle);

    if (p_ses->conf_levels_info) {
        free(p_ses->conf_levels_info);
        p_ses->conf_levels_info = NULL;
    }

    if (p_ses->conf_levels) {
        free(p_ses->conf_levels);
        p_ses->conf_levels = NULL;
    }

    p_ses->rc_config = NULL;

    if (!p_lsm_ses->pcm) {
        ALOGV("%s: pcm NULL", __func__);
        return status;
    }

    /* Exit the callback thread waiting on event detection */
    request_exit_callback_thread(p_lsm_ses);

    pthread_join(p_lsm_ses->callback_thread, (void **) NULL);

    /*
     * Disable lab capture requests.
     * Note: lab for internal stages gets enabled even when lab capture is not required.
     */
    stage_idx = LSM_STAGE_INDEX_FIRST;
    param_count = 0;
    lsm_params.params = (unsigned char*)&param_info[0];
    mparams = p_lsm_ses->lsm_usecase->params;

    /* reset last stage only if lab capture was set */
    if ((stage_idx == p_lsm_ses->num_stages - 1) && capture_requested &&
        (p_lsm_ses->lsm_usecase->param_tag_tracker & PARAM_LAB_CONTROL_BIT)) {
        param_info[param_count].param_size = sizeof(buf_en);
        param_info[param_count].param_data = (unsigned char *)&buf_en;
        lsm_fill_param_info(LSM_LAB_CONTROL, &param_info[param_count++],
                            &mparams[LAB_CONTROL], stage_idx);
    }

    list_for_each(node, &p_ses->lsm_ss_cfg_list) {
        stage_idx++;
        ss_cfg = node_to_item(node, st_lsm_ss_config_t, list_node);
        mparams = ss_cfg->params->params;
        /* reset last stage only if lab capture was set */
        if ((stage_idx == p_lsm_ses->num_stages) && (!capture_requested))
            break;

            param_info[param_count].param_size = sizeof(buf_en);
            param_info[param_count].param_data = (unsigned char *)&buf_en;
            lsm_fill_param_info(LSM_LAB_CONTROL, &param_info[param_count++],
                                &mparams[LAB_CONTROL], stage_idx);
    }

    lsm_params.num_params = param_count;
    if (lsm_params.num_params) {
        lsm_params.data_size =
                lsm_params.num_params * sizeof(lsm_param_info_t);
         status = lsm_set_module_params(p_lsm_ses, &lsm_params);
         if (status)
             ALOGE("%s: ERROR. Sending LAB_CONTROL module params, status=%d",
                    __func__, status);
    }

    if (capture_requested) {
        /*
         * Check and reset lab if ses was non-multi-stage,
         * and lab control param bit was not set.
         * LAB_CONTROL params is optional only for single stage session.
         */
        if ((p_lsm_ses->num_stages == 1) &&
            !(p_lsm_ses->lsm_usecase->param_tag_tracker & PARAM_LAB_CONTROL_BIT)) {
            ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_lab_control");
            status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_LAB_CONTROL, &buf_en);
            ATRACE_END();
            if (status)
                ALOGE("%s: ERROR. SNDRV_LSM_LAB_CONTROL failed, status=%d",
                      __func__, status);
        }

        /* Deallocate lab buffes if allocated during start_recognition */
        if (p_lsm_ses->lab_buffers_allocated)
            deallocate_lab_buffers_ape(p_lsm_ses);
    }

    /* Dereg Sound Models */
    stage_idx = LSM_STAGE_INDEX_FIRST;
    param_count = 0;
    lsm_params.params = (unsigned char*)&param_info[0];
    mparams = p_lsm_ses->lsm_usecase->params;

    param_info[param_count].param_size = 0;
    lsm_fill_param_info(LSM_DEREG_SND_MODEL, &param_info[param_count++],
                        &mparams[UNLOAD_SOUND_MODEL], stage_idx);

    list_for_each(node, &p_ses->lsm_ss_cfg_list) {
        stage_idx++;
        ss_cfg = node_to_item(node, st_lsm_ss_config_t, list_node);
        mparams = ss_cfg->params->params;
        param_info[param_count].param_size = 0;
        lsm_fill_param_info(LSM_DEREG_SND_MODEL, &param_info[param_count++],
                            &mparams[UNLOAD_SOUND_MODEL], stage_idx);
    }

    lsm_params.num_params = param_count;
    lsm_params.data_size =
        lsm_params.num_params * sizeof(lsm_param_info_t);
    status = lsm_set_module_params(p_lsm_ses, &lsm_params);
    if (status)
        ALOGE("%s: ERROR. dereg sound model module params, status %d",
              __func__, status);

    ATRACE_BEGIN("sthal:lsm: pcm_close");
    pcm_close(p_lsm_ses->pcm);
    ATRACE_END();
    p_lsm_ses->pcm = NULL;

    platform_ape_free_pcm_device_id(p_ses->stdev->platform, p_lsm_ses->pcm_id);

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_lsm_ses->common.sm_handle,
        status);
    return status;
}

static int ape_reg_sm_params(st_hw_session_t* p_ses, unsigned int recognition_mode,
    bool capture_requested, struct sound_trigger_recognition_config *rc_config,
    sound_trigger_sound_model_type_t sm_type __unused, void *sm_data __unused)
{
    int status = 0, buf_en = 1, retry_num = 0, offset = 0;
    int param_tag_tracker;
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t*)p_ses;
    uint32_t custom_payload_size, smm_th_conf_param_size;
    uint32_t data_payload_size, lsm_param_payload_size;
    uint32_t data_payload_addr_lsw = 0, data_payload_addr_msw = 0, mem_map_handle = 0;
    uint32_t lab_dam_payload_size = 0;
    unsigned char *custom_payload = NULL, *smm_th_conf_param = NULL;
    unsigned char *lab_dam_payload = NULL;
    struct st_vendor_info *v_info = p_lsm_ses->common.vendor_uuid_info;
    struct snd_lsm_module_params lsm_params;
    lsm_param_info_t param_info[LSM_SM_PARAMS_INFO_IDX];
    lsm_param_info_t *cfl_params;
    lsm_param_info_t *op_params;
    lsm_param_info_t *cus_params;
    lsm_param_info_t *poll_en_params;
    lsm_param_info_t *lab_params;
    lsm_param_info_t *lab_dam_cfg_params;
    struct snd_lsm_detect_mode det_mode;
    st_lsm_poll_enable_t poll_enable;
    bool disable_custom_config = false;
    struct listnode *node = NULL;
    struct st_module_param_info *mparams = NULL;
    st_lsm_ss_config_t *ss_cfg = NULL;
    int param_count = 0, stage_idx = 0;
    struct lsm_param_custom_config custom_conf_params;
    lsm_param_payload_t custom_conf_params_v2 = {0};
    lsm_param_payload_t cus_dam_cfg_params = {0};

    ALOGD("%s:[%d] Enter", __func__, p_lsm_ses->common.sm_handle);
    if (!p_lsm_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }

    /*
     * While dynamically switching ports,
     * port info needs to be sent to driver before enabling usecase.
     * If port controls are available seprately,
     * set port control and ioctl before enabling usecase.
     * Note: error from enable port control,
     *       implies no support for separate port controls.
     */
    if (ape_enable_port_control(true, p_ses) == 0) {
        status = lsm_set_port(p_lsm_ses);
        if (!status)
            ape_enable_use_case(true, p_ses);
    } else {
        ape_enable_use_case(true, p_ses);
        status = lsm_set_port(p_lsm_ses);
    }
    if (status) {
        ALOGE("%s: ERROR. set port failed, returned status %d",
              __func__, status);
        goto error_exit;
    }

    status = send_lsm_input_hw_params(p_ses);
    if (status)
        goto error_exit;

    if ((rc_config->data_size > CUSTOM_CONFIG_OPAQUE_DATA_SIZE) &&
        p_ses->vendor_uuid_info->is_qcva_uuid && !capture_requested)
        disable_custom_config = true;

    ATRACE_BEGIN("sthal:lsm: pcm_start");
    status = pcm_start(p_lsm_ses->pcm);
    while (status && (retry_num < SOUND_TRIGGER_PCM_MAX_RETRY)) {
        usleep(SOUND_TRIGGER_PCM_SLEEP_WAIT);
        retry_num++;
        ALOGI("%s: pcm_start retrying..status %d errno %d, retry cnt %d",
              __func__, status, errno, retry_num);
        status = pcm_start(p_lsm_ses->pcm);
    }
    ATRACE_END();
    if (status) {
        ALOGE("%s: ERROR. pcm_start failed, returned status %d",
              __func__, status);
        goto error_exit;
    }

    /* SVA doesn't support per keyword recogntion mode.
       Use the per soundmodel recognition mode */
    if (recognition_mode & RECOGNITION_MODE_VOICE_TRIGGER){
        det_mode.mode= LSM_MODE_KEYWORD_ONLY_DETECTION;
        if (recognition_mode & RECOGNITION_MODE_USER_IDENTIFICATION)
            det_mode.mode = LSM_MODE_USER_KEYWORD_DETECTION;
    } else {
        ALOGE("%s: Unknown recognition mode %d", __func__, recognition_mode);
        goto error_exit_1;
    }

    stage_idx = LSM_STAGE_INDEX_FIRST;
    param_count = 0;
    lsm_params.params = (unsigned char*)&param_info[0];
    param_tag_tracker = p_lsm_ses->lsm_usecase->param_tag_tracker;
    mparams = p_lsm_ses->lsm_usecase->params;

    /*
     * If smlib is not present, pass only the opaque data as custom params and
     * ignore sending all other params
     */
    if (v_info->smlib_handle) {
        det_mode.detect_failure = p_ses->stdev->detect_failure;
        ALOGV("%s: dm %d df %d lab %d", __func__,
               det_mode.mode, det_mode.detect_failure, capture_requested);
        if (param_tag_tracker & PARAM_CONFIDENCE_LEVELS_BIT) {
            /* fill confidence level params */
            cfl_params = &param_info[param_count++];
            cfl_params->param_size = p_ses->num_conf_levels;
            cfl_params->param_data = p_ses->conf_levels;
            lsm_fill_param_info(LSM_MIN_CONFIDENCE_LEVELS, cfl_params,
                                &mparams[CONFIDENCE_LEVELS], stage_idx);
            {
                unsigned int i;
                ALOGV("%s: ncl %d", __func__, cfl_params->param_size);
                for (i = 0; i < cfl_params->param_size; i++) {
                    ALOGD("%s: First stage conf_levels[%d] = %d",
                        __func__, i, cfl_params->param_data[i]);
                }
            }
        }
        if (param_tag_tracker & PARAM_OPERATION_MODE_BIT) {
            /* fill operation mode params */
            op_params = &param_info[param_count++];
            op_params->param_size = sizeof(det_mode);
            op_params->param_data = (unsigned char *)&det_mode;
            lsm_fill_param_info(LSM_OPERATION_MODE, op_params,
                                &mparams[OPERATION_MODE], stage_idx);
        }
        if (param_tag_tracker & PARAM_POLLING_ENABLE_BIT) {
            poll_en_params = &param_info[param_count];
            if (fill_lsm_poll_enable_params(v_info,
                    &poll_enable, poll_en_params, mparams, stage_idx)) {
                param_count++;
            }
        }
    }
    /*
     * Custom config is mandatory for adsp multi-stage session,
     * Default config would be sent if not explicitly set from client applicaiton.
     */
    if ((rc_config->data_size && !disable_custom_config) ||
        !list_empty(&p_ses->lsm_ss_cfg_list)) {
        /* fill opaque data as custom params */
        cus_params = &param_info[param_count++];
        if (param_tag_tracker & PARAM_CUSTOM_CONFIG_BIT) {
            /*
             * For both cases below, fill module_id and param_id obtained from platform info
             * xml. Also fill custom_config set_param. Package this with recognition config
             * opaque data in the required format and send it.
             */
            if (platform_stdev_get_xml_version(p_ses->stdev->platform) >
                PLATFORM_XML_VERSION_0x0102) {
                int lsm_param_payload_size;
                /*
                 * lsm_param_payload_size = size of opaque data.
                 * custom_payload_size = size of param data + size of opaque data.
                 */
                lsm_param_payload_size = sizeof(struct st_hist_buffer_info);
                lsm_fill_param_header(&custom_conf_params_v2,
                    lsm_param_payload_size, &mparams[CUSTOM_CONFIG]);

                custom_payload_size = sizeof(custom_conf_params_v2) + lsm_param_payload_size;
                custom_payload = (unsigned char *)calloc(1, custom_payload_size);
                if (!custom_payload) {
                    ALOGE("%s: ERROR. Cannot allocate memory for custom_payload", __func__);
                    goto error_exit_1;
                }
                /* copy custom config params to payload */
                memcpy(custom_payload, &custom_conf_params_v2, sizeof(custom_conf_params_v2));
                offset = sizeof(custom_conf_params_v2);

            } else {
                cus_params->param_type = LSM_CUSTOM_PARAMS;
                /*
                 * p_size = size of opaque data.
                 * data_payload_size = size of payload data + size of opaque data.
                 */
                lsm_param_payload_size = sizeof(custom_conf_params) -
                    sizeof(custom_conf_params.set_param);
                data_payload_size = lsm_param_payload_size + sizeof(struct st_hist_buffer_info);
                fill_set_params_payload(&custom_conf_params.set_param, data_payload_size,
                         data_payload_addr_lsw, data_payload_addr_msw, mem_map_handle);

                lsm_fill_param_header((lsm_param_payload_t *)&custom_conf_params.common,
                    lsm_param_payload_size, &mparams[CUSTOM_CONFIG]);

                custom_payload_size = sizeof(struct lsm_param_custom_config) +
                    sizeof(struct st_hist_buffer_info);
                custom_payload = (unsigned char *)calloc(1, custom_payload_size);
                if (!custom_payload) {
                    ALOGE("%s: ERROR. Cannot allcoate memory for custom_payload", __func__);
                    goto error_exit_1;
                }
                /* copy custom config params to payload */
                memcpy(custom_payload, &custom_conf_params, sizeof(struct lsm_param_custom_config));
                offset += sizeof(struct lsm_param_custom_config);
            }

            /* copy opaque data from recognition config to payload */
            if (((rc_config->data_size == 0) ||
                 (rc_config->data_size > CUSTOM_CONFIG_OPAQUE_DATA_SIZE)) &&
                p_ses->vendor_uuid_info->is_qcva_uuid) {
                st_hw_ses_get_hist_buff_payload(p_ses, (uint8_t *)custom_payload + offset,
                                                custom_payload_size - offset);
            } else {
                /* copy opaque data from recognition config to payload */
                memcpy((char *)custom_payload + offset,
                    (char *)rc_config + rc_config->data_offset, rc_config->data_size);
            }
        } else {
            /*
             * Send opaque data as it is,
             * Using legacy custom param where app needs to form appropriate payload.
             */
            custom_payload_size = rc_config->data_size;
            custom_payload = (unsigned char *)calloc(1, custom_payload_size);
            if (!custom_payload) {
                ALOGE("%s: ERROR. Cannot allocate memory for custom_payload", __func__);
                goto error_exit_1;
            }
            memcpy(custom_payload, (char *)rc_config + rc_config->data_offset,
                rc_config->data_size);
        }
        cus_params->param_size = custom_payload_size;
        cus_params->param_data = (unsigned char *)custom_payload;
        lsm_fill_param_info(LSM_CUSTOM_PARAMS, cus_params,
                            &mparams[CUSTOM_CONFIG], stage_idx);
    }

    /* set lab for last stage, only when capturing lab */
    if ((param_tag_tracker & PARAM_LAB_CONTROL_BIT) &&
        ((stage_idx != p_lsm_ses->num_stages - 1) || capture_requested)) {
        lab_params = &param_info[param_count++];

        lab_params->param_size = sizeof(buf_en);
        lab_params->param_data = (unsigned char *)&buf_en;
        lsm_fill_param_info(LSM_LAB_CONTROL, lab_params,
                            &mparams[LAB_CONTROL], stage_idx);
    }

    /*
     * If shared buffering is supported and LPI mode is enabled, send the DAM
     * driver param to DSP to set the shared buffer token for this session.
     */
    if (capture_requested && p_ses->stdev->lpi_enable &&
        p_ses->vendor_uuid_info->lab_dam_cfg_payload.token_id &&
        (p_lsm_ses->lsm_usecase->param_tag_tracker &
         PARAM_LAB_DAM_CFG_BIT)) {

        lab_dam_cfg_params = &param_info[param_count++];
        p_ses->vendor_uuid_info->lab_dam_cfg_payload.minor_version = 0x1;

        lsm_param_payload_size = sizeof(struct lab_dam_cfg_payload);

        lsm_fill_param_header(&cus_dam_cfg_params,
            lsm_param_payload_size, &mparams[LAB_DAM_CFG]);

        lab_dam_payload_size = sizeof(cus_dam_cfg_params) +
            lsm_param_payload_size;
        lab_dam_payload = (unsigned char *)calloc(1, lab_dam_payload_size);
        if (!lab_dam_payload) {
            ALOGE("%s: ERROR. Cannot allocate memory for lab_dam_payload",
                __func__);
            goto error_exit_1;
        }
        /* copy custom config params to payload */
        memcpy(lab_dam_payload, &cus_dam_cfg_params,
            sizeof(cus_dam_cfg_params));
        memcpy(lab_dam_payload + sizeof(cus_dam_cfg_params),
            &p_ses->vendor_uuid_info->lab_dam_cfg_payload,
            lsm_param_payload_size);

        lab_dam_cfg_params->param_size = lab_dam_payload_size;
        lab_dam_cfg_params->param_data = (unsigned char *)lab_dam_payload;
        lsm_fill_param_info(LSM_CUSTOM_PARAMS, lab_dam_cfg_params,
                            &mparams[LAB_DAM_CFG], stage_idx);
    }

    /* Send all applicable module params for this(first) stage */
    lsm_params.num_params = param_count;
    if (lsm_params.num_params) {
        lsm_params.data_size =
             lsm_params.num_params * sizeof(lsm_param_info_t);

        ST_DBG_DECLARE(FILE *lsm_params_fd = NULL; static int lsm_params_cnt = 0);
        ST_DBG_FILE_OPEN_WR(lsm_params_fd, ST_DEBUG_DUMP_LOCATION,
                            "lsm_params_data", "bin", lsm_params_cnt++);
        ST_DBG_FILE_WRITE(lsm_params_fd, lsm_params.params, lsm_params.data_size);
        ST_DBG_FILE_CLOSE(lsm_params_fd);

        status = lsm_set_module_params(p_lsm_ses, &lsm_params);
        if (status) {
            ALOGE("%s: ERROR. sending sm_params, status %d, stage 0",
                  __func__, status);
                goto error_exit_1;
        }
    }

    /* Send sm params for additional stages if available */
    list_for_each(node, &p_ses->lsm_ss_cfg_list) {
        /* reset param_count for each stage, but not stage_idx */
        stage_idx++;
        param_count = 0;
        lsm_params.params = (unsigned char*)&param_info[0];
        ss_cfg = node_to_item(node, st_lsm_ss_config_t, list_node);
        mparams = ss_cfg->params->params;
        param_tag_tracker = ss_cfg->params->param_tag_tracker;

        /*
         * Multi stage usecase must have PARAM_LAB_CONTROL_BIT set,
         * As legacy ioctl do not support multi-stage.
         */
        if (!(param_tag_tracker & PARAM_LAB_CONTROL_BIT)) {
            ALOGE("%s: ERROR: lab control param not set for multi-stage ses %p, stage %d",
                  __func__, p_ses, stage_idx);
            goto error_exit_1;
        }

        if (param_tag_tracker & PARAM_CONFIDENCE_LEVELS_BIT) {
            struct lsm_param_smm_th_config *smm_th_conf = NULL;
            smm_th_conf_param_size =
                sizeof(lsm_param_payload_t) + sizeof(*smm_th_conf);

            smm_th_conf_param = (unsigned char *)calloc(1, smm_th_conf_param_size);
            if (!smm_th_conf_param) {
                ALOGE("%s: ERROR. Cannot allocate memory for smm_th_config", __func__);
                goto error_exit_1;
            }

            lsm_fill_param_header((lsm_param_payload_t *)smm_th_conf_param,
                                  sizeof(*smm_th_conf), &mparams[CONFIDENCE_LEVELS]);

            ALOGV("%s: confidence threshold %d", __func__, ss_cfg->confidence_threshold);
            smm_th_conf = (struct lsm_param_smm_th_config *)
                          (smm_th_conf_param + sizeof(lsm_param_payload_t));
            smm_th_conf->minor_version = 1;
            smm_th_conf->smm_threshold = ss_cfg->confidence_threshold;

            cfl_params = &param_info[param_count++];
            cfl_params->param_size = smm_th_conf_param_size;
            cfl_params->param_data = smm_th_conf_param;
            /*
             * Cannot use LSM_MIN_CONFIDENCE_LEVELS type to send confidence levels
             * for non-GMM sound models, as this param type have special handling
             * in driver to form payload having multiple keyword and user conf levels
             * which is not applicable for GMM sound models.
             * Params can be sent using custom param type without extending driver,
             * as custom param can take any param payload.
             */
            lsm_fill_param_info(LSM_CUSTOM_PARAMS, cfl_params,
                                &mparams[CONFIDENCE_LEVELS], stage_idx);
        }

        if (param_tag_tracker & PARAM_OPERATION_MODE_BIT) {
            op_params = &param_info[param_count++];

            /* CNN supports only keyword detection */
            if (ss_cfg->params->common_params.sm_id == ST_SM_ID_SVA_CNN)
                det_mode.mode = LSM_MODE_KEYWORD_ONLY_DETECTION;

            op_params->param_size = sizeof(det_mode);
            op_params->param_data = (unsigned char *)&det_mode;

            lsm_fill_param_info(LSM_OPERATION_MODE, op_params,
                                &mparams[OPERATION_MODE], stage_idx);
        }

        /*
         * Reusing custom config payload from first stage if available,
         * MultiStage SM is expected to use same custom config,
         * Since same keyword/history buffering is applicable to all stages.
         */
        if ((param_tag_tracker & PARAM_CUSTOM_CONFIG_BIT) && custom_payload) {
            cus_params = &param_info[param_count++];

            /* keep the payload as is but need to update param info */
            lsm_fill_param_header((lsm_param_payload_t *)custom_payload,
                                  (custom_payload_size - sizeof(lsm_param_payload_t)),
                                  &mparams[CUSTOM_CONFIG]);

            cus_params->param_size = custom_payload_size;
            cus_params->param_data = (unsigned char *)custom_payload;

            lsm_fill_param_info(LSM_CUSTOM_PARAMS, cus_params,
                                &mparams[CUSTOM_CONFIG], stage_idx);
        }

        /* set lab for last stage, only when capturing lab */
        if ((stage_idx != p_lsm_ses->num_stages - 1) || capture_requested) {
            lab_params = &param_info[param_count++];

            lab_params->param_size = sizeof(buf_en);
            lab_params->param_data = (unsigned char *)&buf_en;

            lsm_fill_param_info(LSM_LAB_CONTROL, lab_params,
                                &mparams[LAB_CONTROL], stage_idx);
        }

        /* Send all applicable module params for this stage */
        lsm_params.num_params = param_count;
        if (lsm_params.num_params) {
            lsm_params.data_size =
               lsm_params.num_params * sizeof(lsm_param_info_t);

            ST_DBG_DECLARE(FILE *lsm_params_fd = NULL; static int lsm_params_cnt = 0);
            ST_DBG_FILE_OPEN_WR(lsm_params_fd, ST_DEBUG_DUMP_LOCATION,
                            "lsm_params_data", "bin", lsm_params_cnt++);
            ST_DBG_FILE_WRITE(lsm_params_fd, lsm_params.params, lsm_params.data_size);
            ST_DBG_FILE_CLOSE(lsm_params_fd);

            status = lsm_set_module_params(p_lsm_ses, &lsm_params);
            if (status) {
                ALOGE("%s: ERROR. sending reg_sm_params, status %d stage %d",
                      __func__, status, stage_idx);
                goto error_exit_1;
            }
        }
    }

    if (capture_requested) {
        /*
         * Check and set lab if ses was non-multi-stage,
         * and lab control param bit was not set.
         * LAB_CONTROL params is optional only for single stage session.
         */
        if (p_lsm_ses->num_stages == 1 &&
            !(p_lsm_ses->lsm_usecase->param_tag_tracker & PARAM_LAB_CONTROL_BIT)) {
            ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_lab_control");
            status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_LAB_CONTROL, &buf_en);
            ATRACE_END();
            if (status) {
                ALOGE("%s: ERROR. SNDRV_LSM_LAB_CONTROL failed, status=%d",
                      __func__, status);
                goto error_exit_1;
            }
        }

        if (!p_lsm_ses->lab_buffers_allocated) {
            status = allocate_lab_buffers_ape(p_lsm_ses);
            if (status)
                goto error_exit_1;
        }
    }

    return status;

error_exit_1:

    if (p_lsm_ses->lab_buffers_allocated)
        deallocate_lab_buffers_ape(p_lsm_ses);

    if (smm_th_conf_param)
        free(smm_th_conf_param);

    if (lab_dam_payload)
        free(lab_dam_payload);

    if (custom_payload)
        free(custom_payload);

    pcm_stop(p_lsm_ses->pcm);

error_exit:

    ape_enable_use_case(false, p_ses);
    ape_enable_port_control(false, p_ses);

    ALOGD("%s:[%d] Exit, status=%d", __func__,
        p_lsm_ses->common.sm_handle, status);

    return status;
}

static int ape_dereg_sm_params(st_hw_session_t* p_ses)
{
    st_hw_session_lsm_t *p_lsm_ses =
       (st_hw_session_lsm_t *)p_ses;

    ALOGD("%s:[%d] Enter", __func__, p_lsm_ses->common.sm_handle);
    if (p_lsm_ses->pcm){
        ATRACE_BEGIN("sthal:lsm: pcm_stop");
        pcm_stop(p_lsm_ses->pcm);
        ATRACE_END();
    }
    ape_enable_use_case(false, p_ses);
    ape_enable_port_control(false, p_ses);

    return 0;
}

static int ape_start(st_hw_session_t* p_ses)
{
    int status = 0;
    st_hw_session_lsm_t *p_lsm_ses =
       (st_hw_session_lsm_t *)p_ses;

    ALOGD("%s:[%d] Enter", __func__, p_lsm_ses->common.sm_handle);
    if (!p_lsm_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }

    p_lsm_ses->exit_lab_processing = false;

    ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_start");
    status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_START);
    ATRACE_END();
    if (status) {
        ALOGE("%s: ERROR. SNDRV_LSM_START failed, status=%d", __func__, status);
    }

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_lsm_ses->common.sm_handle,
        status);
    return status;
}

static int ape_stop(st_hw_session_t* p_ses)
{
    int status = 0;
    st_hw_session_lsm_t *p_lsm_ses =
       (st_hw_session_lsm_t *)p_ses;

    ALOGD("%s:[%d] Enter", __func__, p_lsm_ses->common.sm_handle);
    if (!p_lsm_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }

    ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_stop");
    status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_STOP);
    ATRACE_END();
    if (status)
        ALOGE("%s: ERROR. SNDDRV_LSM_STOP failed, status=%d", __func__, status);

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_lsm_ses->common.sm_handle,
        status);
    return status;
}

static int ape_stop_buffering(st_hw_session_t* p_ses)
{
    int status = 0;
    st_hw_session_lsm_t *p_lsm_ses =
       (st_hw_session_lsm_t *)p_ses;

    ALOGD("%s:[%d] Enter pcm %p", __func__, p_lsm_ses->common.sm_handle,
        p_lsm_ses->pcm);
    if (!p_lsm_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }

    ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_stop_lab");
    status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_STOP_LAB);
    ATRACE_END();
    if (status) {
        ALOGE("%s: ERROR. SNDRV_LSM_STOP_BUFFERING failed status %d", __func__,
           status);
    } else {
        ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_pcm_ioctl_reset");
        status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_PCM_IOCTL_RESET);
        ATRACE_END();
        if (status) ALOGE("%s: ERROR. SNDRV_PCM_IOCTL_RESET failed status %d", __func__,
               status);
    }

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return status;
}

static int cpe_reg_sm(st_hw_session_t *p_ses, void* sm_data,
    sound_trigger_sound_model_type_t sm_type)
{
    int status = 0, fd;
    struct sound_trigger_phrase_sound_model *phrase_sm = sm_data;
    struct sound_trigger_sound_model *common_sm= sm_data;
    struct snd_lsm_module_params lsm_params;
    struct lsm_params_info param_info;
    struct st_vendor_info *v_info = p_ses->vendor_uuid_info;
    struct pcm_config  *config;
    pthread_attr_t attr;
    st_hw_session_lsm_t *p_lsm_ses =  (st_hw_session_lsm_t *)p_ses;
    struct st_module_param_info *mparams = NULL;

    ALOGD("%s:[%d] Enter", __func__, p_lsm_ses->common.sm_handle);

    p_lsm_ses->pcm_id = platform_cpe_get_pcm_device_id(p_ses->stdev->platform,
        v_info->sample_rate, &p_ses->use_case_idx);
    if (p_lsm_ses->pcm_id < 0)
        return -ENODEV;

    platform_get_lsm_usecase(p_ses->stdev->platform, v_info,
                             &p_lsm_ses->lsm_usecase, p_ses->exec_mode);
    if (!p_lsm_ses->lsm_usecase) {
        ALOGE("%s: couldn't get lsm usecase", __func__);
        goto sm_error;
    }
    /* Backward compatibility with previous XML version <= 0x0102 */
    int app_type = (v_info->app_type == 0) ?
        p_lsm_ses->lsm_usecase->app_type : v_info->app_type;

    status = platform_stdev_send_calibration(p_ses->stdev->platform,
                                              AUDIO_DEVICE_NONE,
                                              p_ses->exec_mode,
                                              p_ses->vendor_uuid_info,
                                              app_type, true,
                                              ST_SESSION_CAL);

    if (status) {
        ALOGE("%s: ERROR. sending session calibration failed status %d",
               __func__, status);
        goto sm_error;
    }

    if (strcmp(v_info->cpe_firmware_image,"")) {
        ALOGD("%s: cpe firmware image %s", __func__, v_info->cpe_firmware_image);
        /* Write the vendor firmware image name in sysfs entry */
        fd = open("/sys/kernel/wcd_cpe0/fw_name", O_WRONLY);
        if (fd >= 0) {
            write(fd, v_info->cpe_firmware_image,
               strlen(v_info->cpe_firmware_image));
            close(fd);
        }
        else {
            ALOGE("%s: ERROR. sysfs open to write firmware name failed : %s",
                  __func__, strerror(errno));
            goto sm_error;
        }
    }
    p_lsm_ses->common.config = stdev_cpe_pcm_config;
    p_lsm_ses->lab_config = stdev_cpe_pcm_config;
    config = &p_lsm_ses->lab_config;

    p_lsm_ses->lab_config.rate = v_info->sample_rate;
    if (v_info->kw_transfer_mode == FTRT_TRANSFER_MODE) {
        platform_stdev_check_and_update_pcm_config(p_ses->stdev->platform,
                                                   &p_lsm_ses->lab_config,
                                                   v_info,
                                                   p_ses->exec_mode);

        p_lsm_ses->lab_config.period_size =
           CALCULATE_PERIOD_SIZE(SOUND_TRIGGER_CPE_LAB_DRV_BUF_DURATION_MS,
                                 p_lsm_ses->lab_config.rate,
                                 SOUND_TRIGGER_CPE_PERIOD_COUNT, 32);

        ALOGV("%s: ftrt rate=%d format=%d", __func__,
            p_lsm_ses->lab_config.rate, p_lsm_ses->lab_config.format);

        if (CHECK_BIT(v_info->kw_capture_format, ADPCM))
           p_lsm_ses->common.config.period_size = ADPCM_MAX_OUT_SAMPLES_PER_FRAME;
    }

    ALOGD("%s: opening pcm device=%d", __func__, p_lsm_ses->pcm_id);
    ALOGD("%s: config: channels=%d rate=%d, period_size=%d, "
          "period_cnt=%d, format=%d", __func__, config->channels,
          config->rate, config->period_size,
          config->period_count, config->format);

    ATRACE_BEGIN("sthal:lsm: pcm_open");
    p_lsm_ses->pcm = pcm_open(p_ses->stdev->snd_card, p_lsm_ses->pcm_id,
                          PCM_IN, config);
    ATRACE_END();

    if (!p_lsm_ses->pcm) {
        ALOGE("%s: ERROR. pcm_open failed", __func__);
        status = -ENODEV;
        goto sm_error;
    }
    if (!pcm_is_ready(p_lsm_ses->pcm)) {
        ALOGE("%s: ERROR. pcm_is_ready failed err=%s", __func__,
              pcm_get_error(p_lsm_ses->pcm));
        status = -ENODEV;
        goto sm_error;
    }

    mparams = p_lsm_ses->lsm_usecase->params;
    param_info.param_type = LSM_REG_SND_MODEL;
    param_info.module_id = mparams[LOAD_SOUND_MODEL].module_id;
    param_info.param_id = mparams[LOAD_SOUND_MODEL].param_id;

    if (sm_type == SOUND_MODEL_TYPE_KEYPHRASE) {
        param_info.param_size = phrase_sm->common.data_size;
        param_info.param_data =
            (unsigned char*)phrase_sm + phrase_sm->common.data_offset;
    } else {
        param_info.param_size = common_sm->data_size;
        param_info.param_data =
            (unsigned char*)common_sm + common_sm->data_offset;
    }

    lsm_params.num_params = 1;
    lsm_params.params = (unsigned char*)&param_info;
    lsm_params.data_size = lsm_params.num_params * sizeof(struct lsm_params_info);

    ALOGV("%s: sm data size=%d", __func__, param_info.param_size);

    ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_set_module_params");
    status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_SET_MODULE_PARAMS,
        &lsm_params);
    ATRACE_END();

    if (status) {
        ALOGE("%s: ERROR. SNDRV_LSM_SET_MODULE_PARAMS->reg status=%d",
              __func__, status);
        goto sm_error;
    }

    /* Callback thread to wait on event detection */
    p_lsm_ses->exit_callback_thread = false;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(&p_lsm_ses->callback_thread, &attr,
                        callback_thread_loop, p_ses);

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_lsm_ses->common.sm_handle,
        status);
    return 0;

sm_error:
    platform_cpe_free_pcm_device_id(p_ses->stdev->platform, p_lsm_ses->pcm_id);
    if (p_lsm_ses->pcm) {
        pcm_close(p_lsm_ses->pcm);
        p_lsm_ses->pcm = NULL;
    }

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_lsm_ses->common.sm_handle,
        status);
    return status;
}

static int cpe_dereg_sm(st_hw_session_t *p_ses, bool capture_requested)
{
    int status = 0, buf_en;
    struct snd_lsm_module_params lsm_params;
    struct lsm_params_info param_info;
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;
    struct st_module_param_info *mparams = p_lsm_ses->lsm_usecase->params;

    ALOGD("%s:[%d] Enter", __func__, p_lsm_ses->common.sm_handle);

    if (p_ses->conf_levels_info) {
        free(p_ses->conf_levels_info);
        p_ses->conf_levels_info = NULL;
    }

    if (p_ses->conf_levels) {
        free(p_ses->conf_levels);
        p_ses->conf_levels = NULL;
    }

    p_ses->rc_config = NULL;

    if (!p_lsm_ses->pcm) {
        ALOGV("%s: pcm NULL", __func__);
        return status;
    }

    /* Exit the callback thread waiting on event detection */
    request_exit_callback_thread(p_lsm_ses);

    /* note see note in ape_dereg_sm */
    pthread_join(p_lsm_ses->callback_thread, (void **) NULL);

    if (capture_requested) {
        buf_en = 0;
        ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_lab_control");
        status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_LAB_CONTROL, &buf_en);
        ATRACE_END();
        if (status)
            ALOGE("%s: ERROR. SNDRV_LSM_LAB_CONTROL failed, status=%d",
            __func__, status);

        if (p_lsm_ses->lab_buffers_allocated) {
            /* Deallocate lab buffes allocated during start_recognition */
            deallocate_lab_buffers_cpe(p_lsm_ses);
        }
    }

    param_info.param_type = LSM_DEREG_SND_MODEL;
    param_info.module_id = mparams[UNLOAD_SOUND_MODEL].module_id;
    param_info.param_id = mparams[UNLOAD_SOUND_MODEL].param_id;
    param_info.param_size = 0;

    lsm_params.num_params = 1;
    lsm_params.params = (unsigned char*)&param_info;
    lsm_params.data_size =
        lsm_params.num_params * sizeof(struct lsm_params_info);

    ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_set_module_params");
    status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_SET_MODULE_PARAMS,
       &lsm_params);
    ATRACE_END();

    if (status)
        ALOGE("%s: ERROR. SNDRV_LSM_SET_MODULE_PARAMS->dereg status=%d",
              __func__, status);

    ATRACE_BEGIN("sthal:lsm: pcm_close");
    pcm_close(p_lsm_ses->pcm);
    ATRACE_END();
    p_lsm_ses->pcm = NULL;
    platform_cpe_free_pcm_device_id(p_ses->stdev->platform, p_lsm_ses->pcm_id);

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_lsm_ses->common.sm_handle,
        status);
    return status;
}

static int cpe_reg_sm_params(st_hw_session_t* p_ses, unsigned int recognition_mode,
    bool capture_requested, struct sound_trigger_recognition_config *rc_config,
    sound_trigger_sound_model_type_t sm_type, void *sm_data __unused)
{
    int status = 0, idx = 0;
    int retry_num = 0, offset = 0;
    bool disable_custom_config = false;
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t*)p_ses;
    struct st_vendor_info *v_info = p_lsm_ses->common.vendor_uuid_info;
    struct snd_lsm_module_params lsm_params;
    struct lsm_params_info param_info[3];
    struct lsm_params_info *cfl_params;
    struct lsm_params_info *op_params;
    struct lsm_params_info *cus_params;
    struct snd_lsm_detect_mode det_mode;
    int param_tag_tracker;
    unsigned char *custom_payload = NULL;
    struct lsm_param_payload custom_conf_params;
    uint32_t custom_payload_size;
    struct st_module_param_info *mparams = p_lsm_ses->lsm_usecase->params;

    ALOGD("%s:[%d] Enter", __func__, p_lsm_ses->common.sm_handle);
    if (!p_lsm_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }

    if (v_info == NULL) {
        ALOGE("%s: ERROR. v_info is null", __func__);
        return -EINVAL;
    }

    status = lsm_set_port(p_lsm_ses);
    if (status) {
        ALOGE("%s: ERROR. set port failed, returned status %d",
              __func__, status);
        return status;
    }

    if ((rc_config->data_size > CUSTOM_CONFIG_OPAQUE_DATA_SIZE) &&
        p_ses->vendor_uuid_info->is_qcva_uuid && !capture_requested)
        disable_custom_config = true;

    ATRACE_BEGIN("sthal:lsm: pcm_start");
    status = pcm_start(p_lsm_ses->pcm);
    while (status && (retry_num < SOUND_TRIGGER_PCM_MAX_RETRY)) {
        usleep(SOUND_TRIGGER_PCM_SLEEP_WAIT);
        retry_num++;
        ALOGI("%s: pcm_start retrying..status %d errno %d, retry cnt %d",
              __func__, status, errno, retry_num);
        status = pcm_start(p_lsm_ses->pcm);
    }
    ATRACE_END();
    if (status) {
        ALOGE("%s: ERROR. pcm_start failed, returned status %d",
              __func__, status);
        return status;
    }

    memset(&det_mode, 0, sizeof(struct snd_lsm_detect_mode));
    if (sm_type == SOUND_MODEL_TYPE_KEYPHRASE) {
        /* SVA doesn't support per keyword recogntion mode.
         * Use the per soundmodel recognition mode
         */
        if (recognition_mode & RECOGNITION_MODE_VOICE_TRIGGER){
            det_mode.mode= LSM_MODE_KEYWORD_ONLY_DETECTION;
            if (recognition_mode & RECOGNITION_MODE_USER_IDENTIFICATION)
                det_mode.mode = LSM_MODE_USER_KEYWORD_DETECTION;
        } else {
            ALOGE("%s: Unknown recognition mode %d", __func__, recognition_mode);
            goto err_exit_1;
        }
    }

    lsm_params.num_params = 0;
    param_tag_tracker = p_lsm_ses->lsm_usecase->param_tag_tracker;

    /*
     * If this is generic sound model or smlib is not present, pass only the
     * opaque data as custom params and ignore sending all other params.
     */
    if (v_info->smlib_handle &&
        (sm_type == SOUND_MODEL_TYPE_KEYPHRASE)) {
        det_mode.detect_failure = p_ses->stdev->detect_failure;
        ALOGV("%s: dm %d df %d lab %d", __func__,
               det_mode.mode, det_mode.detect_failure, capture_requested);

        if ((param_tag_tracker & PARAM_CONFIDENCE_LEVELS_BIT) &&
             p_ses->conf_levels) {
            /* fill confidence level params if present */
            cfl_params = &param_info[idx++];
            cfl_params->param_type = LSM_MIN_CONFIDENCE_LEVELS;
            cfl_params->module_id = mparams[CONFIDENCE_LEVELS].module_id;
            cfl_params->param_id = mparams[CONFIDENCE_LEVELS].param_id;
            cfl_params->param_size = p_ses->num_conf_levels;
            cfl_params->param_data = p_ses->conf_levels;
            lsm_params.num_params++;
            {
                unsigned int i;
                ALOGD("%s: ncl %d", __func__, cfl_params->param_size);
                for (i = 0; i < cfl_params->param_size; i++) {
                    ALOGV("%s: conf_levels[%d] = %d", __func__,
                                        i, cfl_params->param_data[i]);
                }
            }
        }
        if (param_tag_tracker & PARAM_OPERATION_MODE_BIT) {
            /* fill operation mode params */
            op_params = &param_info[idx++];
            op_params->param_type = LSM_OPERATION_MODE;
            op_params->module_id = mparams[OPERATION_MODE].module_id;
            op_params->param_id = mparams[OPERATION_MODE].param_id;
            op_params->param_size = sizeof(det_mode);
            op_params->param_data = (unsigned char *)&det_mode;
            lsm_params.num_params++;
        }
    }

    if (rc_config->data_size && !disable_custom_config) {
        /* fill opaque data as custom params */
        cus_params = &param_info[idx++];
        if (param_tag_tracker & PARAM_CUSTOM_CONFIG_BIT) {
            /* Fill custom config params, module id,
               param id and size. Append it with payload data. */
            custom_conf_params.module_id = mparams[CUSTOM_CONFIG].module_id;
            custom_conf_params.param_id = mparams[CUSTOM_CONFIG].param_id;
            custom_conf_params.p_size = sizeof(struct st_hist_buffer_info);

            custom_payload_size = sizeof(struct lsm_param_payload) +
                    sizeof(struct st_hist_buffer_info);
            custom_payload = (unsigned char *)calloc(1, custom_payload_size);
            if (!custom_payload) {
                  ALOGE("%s: ERROR. Cannot allcoate memory for custom_payload", __func__);
                  goto error_exit;
            }
            /* copy custom config params to payload */
            memcpy(custom_payload, &custom_conf_params, sizeof(struct lsm_param_payload));
            offset += sizeof(struct lsm_param_payload);
            /* copy opaque data from recognition config to payload */
            if (((rc_config->data_size == 0) ||
               (rc_config->data_size > CUSTOM_CONFIG_OPAQUE_DATA_SIZE)) &&
               p_ses->vendor_uuid_info->is_qcva_uuid) {
               st_hw_ses_get_hist_buff_payload(p_ses, (uint8_t *)custom_payload + offset,
                                               custom_payload_size - offset);
            } else {
              /* copy opaque data from recognition config to payload */
              memcpy((char *)custom_payload + offset,
                   (char *)rc_config + rc_config->data_offset, rc_config->data_size);
            }
        } else {
             /*
             * Send opaque data as it is,
             * Using legacy custom param where app needs to form appropriate payload.
             */
             custom_payload_size = rc_config->data_size;
             custom_payload = (unsigned char *)calloc(1, custom_payload_size);
             if (!custom_payload) {
                 ALOGE("%s: ERROR. Cannot allocate memory for custom_payload", __func__);
                 goto error_exit;
             }
             memcpy(custom_payload, (char *)rc_config + rc_config->data_offset,
             rc_config->data_size);
        }
        cus_params->param_type = LSM_CUSTOM_PARAMS;
        cus_params->param_size = custom_payload_size;
        cus_params->param_data = (unsigned char *)custom_payload;
        lsm_params.num_params++;
    }

    if (lsm_params.num_params) {
        lsm_params.params = (unsigned char*)param_info;
        lsm_params.data_size =
            lsm_params.num_params * sizeof(struct lsm_params_info);

        ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_set_module_params");
        status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_SET_MODULE_PARAMS, &lsm_params);
        ATRACE_END();
        if (status) {
            ALOGE("%s: ERROR. SNDRV_LSM_SET_MODULE_PARAMS->reg_params %d, status=%d",
                  __func__, lsm_params.num_params, status);
            goto err_exit_1;
        }
    }

    if (capture_requested) {
        if (!p_lsm_ses->lab_buffers_allocated) {
            /* Instead of allocating and deallocating during frequent
               start/stop_recogntion, allocate once and dellocate during
               unload_sound_model */
            status = allocate_lab_buffers_cpe(p_lsm_ses);
            if (status)
                goto err_exit_1;
        }
        status = configure_lab(p_lsm_ses, capture_requested);
        if (status)
            goto err_exit_1;
    }

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_lsm_ses->common.sm_handle, status);
    return 0;
error_exit:
    if (custom_payload)
        free(custom_payload);
err_exit_1:
    if (p_lsm_ses->lab_buffers_allocated)
        deallocate_lab_buffers_cpe(p_lsm_ses);

    pcm_stop(p_lsm_ses->pcm);

    return status;
}

static int cpe_dereg_sm_params(st_hw_session_t* p_ses)
{
    int status = 0;
    st_hw_session_lsm_t *p_lsm_ses =
       (st_hw_session_lsm_t *)p_ses;

    ALOGD("%s:[%d] Enter", __func__, p_lsm_ses->common.sm_handle);
    if (p_lsm_ses->pcm) {
        ATRACE_BEGIN("sthal:lsm: pcm_stop");
        status = pcm_stop(p_lsm_ses->pcm);
        ATRACE_END();
        if (status)
            ALOGE("%s: ERROR. pcm_stop failed: %s", __func__,
               pcm_get_error(p_lsm_ses->pcm));

    }

    ALOGD("%s:[%d] Exit", __func__, p_lsm_ses->common.sm_handle);
    return status;
}

static int cpe_start(st_hw_session_t* p_ses)
{
    int status = 0;
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;

    ALOGD("%s:[%d] Enter", __func__, p_lsm_ses->common.sm_handle);
    if (!p_lsm_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }

    ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_start");
    status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_START);
    ATRACE_END();
    if (status) {
        ALOGE("%s: ERROR. SNDRV_LSM_START failed, status=%d", __func__, status);
    }

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_lsm_ses->common.sm_handle,
        status);

    SET_STATE(p_lsm_ses->common.state, SES_STARTED);
    return status;
}

static int cpe_stop(st_hw_session_t* p_ses)
{
    int status = 0;
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;

    ALOGD("%s:[%d] Enter", __func__, p_lsm_ses->common.sm_handle);
    if (!p_lsm_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }

    ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_stop");
    status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_STOP);
    ATRACE_END();
    if (status)
        ALOGE("%s: ERROR. SNDDRV_LSM_STOP failed, status=%d", __func__, status);

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_lsm_ses->common.sm_handle,
        status);
    return status;
}

static int cpe_stop_buffering(st_hw_session_t* p_ses, bool capture_requested)
{
    int status = 0;
    st_hw_session_lsm_t *p_lsm_ses =
       (st_hw_session_lsm_t *)p_ses;

    ALOGD("%s:[%d] Enter pcm %p", __func__, p_lsm_ses->common.sm_handle,
        p_lsm_ses->pcm);
    if (!p_lsm_ses->pcm) {
        ALOGW("%s: pcm NULL", __func__);
        return status;
    }

    ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_stop_lab");
    status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_STOP_LAB);
    ATRACE_END();
    if (status) {
        ALOGE("%s: ERROR. SNDRV_LSM_STOP_BUFFERING failed status %d", __func__,
           status);
    } else {
        ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_pcm_ioctl_reset");
        status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_PCM_IOCTL_RESET);
        ATRACE_END();
        if (status)
            ALOGE("%s: ERROR. SNDRV_PCM_IOCTL_RESET failed status %d",
               __func__, status);
    }
    if (capture_requested) {
        pthread_mutex_lock(&p_lsm_ses->lock);
        pthread_cond_broadcast(&p_lsm_ses->cond);
        pthread_mutex_unlock(&p_lsm_ses->lock);
    }

    ALOGD("%s:[%d] Exit, status=%d", __func__, p_ses->sm_handle, status);
    return status;
}

static int route_reg_sm_ape(st_hw_session_t *p_ses,void *sm_data,
        sound_trigger_sound_model_type_t sm_type)
{
    return ape_reg_sm(p_ses, sm_data, sm_type);
}

static int route_reg_sm_params_ape(st_hw_session_t* p_ses,
    unsigned int recognition_mode, bool capture_requested,
    struct sound_trigger_recognition_config *rc_config,
    sound_trigger_sound_model_type_t sm_type, void *sm_data)
{
    return ape_reg_sm_params(p_ses, recognition_mode, capture_requested,
        rc_config, sm_type, sm_data);
}

static int route_dereg_sm_params_ape(st_hw_session_t* p_ses)
{
    return ape_dereg_sm_params(p_ses);
}

static int route_dereg_sm_ape(st_hw_session_t* p_ses,
    bool capture_requested)
{
    int status = 0;

    status = ape_dereg_sm(p_ses, capture_requested);

    return status;
}

static int route_start_ape(st_hw_session_t* p_ses)
{

    return ape_start(p_ses);
}

static int route_restart_ape(st_hw_session_t* p_ses,
                             unsigned int recognition_mode __unused,
                             bool capture_requested __unused,
                             struct sound_trigger_recognition_config *rc_config __unused,
                             sound_trigger_sound_model_type_t sm_type __unused,
                             void *sm_data __unused)
{
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;

    ALOGD("%s:[%d] Enter", __func__, p_lsm_ses->common.sm_handle);
    p_lsm_ses->exit_lab_processing = false;
    return 0;
}

static int route_stop_ape(st_hw_session_t* p_ses)
{

    return ape_stop(p_ses);
}

static int route_stop_buffering_ape(st_hw_session_t* p_ses, bool capture_requested __unused)
{
    int status = 0;
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;
    struct timespec tspec;
    st_arm_second_stage_t *st_sec_stage;
    struct listnode *node = NULL, *tmp_node = NULL;
    bool restart_pcm_on_timeout = true;

    if (p_ses->enable_second_stage) {
        list_for_each_safe(node, tmp_node, p_lsm_ses->common.second_stage_list) {
            st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
            pthread_mutex_lock(&st_sec_stage->ss_session->lock);
            st_sec_stage->ss_session->exit_buffering = true;
            pthread_cond_signal(&st_sec_stage->ss_session->cond);
            pthread_mutex_unlock(&st_sec_stage->ss_session->lock);
        }
    }

    p_lsm_ses->exit_lab_processing = true;
    pthread_mutex_lock(&p_lsm_ses->lock);

    /* wait for buffering thread to exit */
    while (p_lsm_ses->lab_processing_active) {
        GET_WAIT_TIMESPEC(tspec,
            convert_bytes_to_ms((p_lsm_ses->lab_drv_buf_size * 2),
            &p_ses->config) * NSECS_PER_MSEC);
        ALOGV("%s: waiting on exit cond", __func__);
        status = pthread_cond_timedwait(&p_lsm_ses->cond, &p_lsm_ses->lock,
            &tspec);
        ALOGV("%s: done waiting on exit cond", __func__);
        if (status) {
            ALOGE("%s: ERROR. wait timed out, ret %d, retry = %d",
                  __func__, status, restart_pcm_on_timeout);
            /*
             * retry once more after aborting ongoing read with
             * pcm_stop/start
             */
            if (restart_pcm_on_timeout && p_lsm_ses->pcm) {
                pcm_stop(p_lsm_ses->pcm);
                pcm_start(p_lsm_ses->pcm);
                restart_pcm_on_timeout = false;
                continue;
            }
            status = -status;
            break;
        }
    }
    pthread_mutex_unlock(&p_lsm_ses->lock);

    return status;
}

static int route_set_device_ape(st_hw_session_t *p_ses,
                                bool enable)
{
    return sound_trigger_set_device(p_ses, enable);
}

static int route_read_pcm_ape(st_hw_session_t *p_ses,
                              unsigned char *buf,
                              unsigned int bytes)
{
    int status = 0;
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;

    ATRACE_BEGIN("sthal:lsm: read_pcm_data");
    status = read_pcm_data(p_lsm_ses, buf, bytes);
    ATRACE_END();

    return status;
}

static void route_audio_capture_ape(st_hw_session_t *p_ses)
{
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;

    ATRACE_BEGIN("sthal:lsm: process_raw_lab_data_ape");
    process_raw_lab_data_ape(p_lsm_ses);
    ATRACE_END();
}

int route_send_custom_chmix_coeff_ape(st_hw_session_t *p_ses, char *str)
{
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;

    return platform_stdev_send_custom_channel_mixing_coefficients(p_ses->stdev->platform,
                                                                  p_ses->vendor_uuid_info,
                                                                  p_lsm_ses->pcm_id,
                                                                  str);
}

static int route_reg_sm_cpe(st_hw_session_t *p_ses, void* sm_data,
    sound_trigger_sound_model_type_t sm_type)
{
    return cpe_reg_sm(p_ses, sm_data, sm_type);
}

static int route_reg_sm_params_cpe(st_hw_session_t* p_ses,
    unsigned int recognition_mode, bool capture_requested,
    struct sound_trigger_recognition_config *rc_config,
    sound_trigger_sound_model_type_t sm_type, void *sm_data)
{
    return cpe_reg_sm_params(p_ses, recognition_mode, capture_requested,
        rc_config, sm_type, sm_data);
}

static int route_dereg_sm_params_cpe(st_hw_session_t* p_ses)
{
    return cpe_dereg_sm_params(p_ses);
}

static int route_dereg_sm_cpe(st_hw_session_t* p_ses, bool capture_requested)
{
    return cpe_dereg_sm(p_ses, capture_requested);
}

static int route_start_cpe(st_hw_session_t* p_ses)
{
    return cpe_start(p_ses);
}

static int route_restart(st_hw_session_t* p_ses __unused, unsigned int recognition_mode __unused,
   bool capture_requested __unused, struct sound_trigger_recognition_config *rc_config __unused,
   sound_trigger_sound_model_type_t sm_type __unused, void *sm_data __unused)
{
    /*
     * this is a noop, in the case of LSM if session is already
     * started there is no need to stop/start even after detection
     */
    return 0;
}

static int route_stop_cpe(st_hw_session_t* p_ses)
{
    return cpe_stop(p_ses);
}

static int route_stop_buffering_cpe(st_hw_session_t* p_ses,
    bool capture_requested)
{
    int status = 0;
    st_arm_second_stage_t *st_sec_stage;
    struct listnode *node = NULL, *tmp_node = NULL;

    if (!CHECK_STATE(p_ses->state, SES_BUFFERING)) {
        return status;
    }

    if (p_ses->enable_second_stage) {
        list_for_each_safe(node, tmp_node, p_ses->second_stage_list) {
            st_sec_stage = node_to_item(node, st_arm_second_stage_t, list_node);
            pthread_mutex_lock(&st_sec_stage->ss_session->lock);
            st_sec_stage->ss_session->exit_buffering = true;
            pthread_cond_signal(&st_sec_stage->ss_session->cond);
            pthread_mutex_unlock(&st_sec_stage->ss_session->lock);
        }
    }

    check_and_exit_lab(p_ses);
    status = cpe_stop_buffering(p_ses, capture_requested);

    CLEAR_STATE(p_ses->state, SES_BUFFERING);
    return status;
}

static int route_set_device_cpe(st_hw_session_t *p_ses,
                                bool enable)
{
    return sound_trigger_set_device(p_ses, enable);
}

static int route_read_pcm_cpe(st_hw_session_t *p_ses,
                              unsigned char *buf,
                              unsigned int bytes)
{
    int status = 0;
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;

    pthread_mutex_lock(&p_lsm_ses->lock);
    if (p_lsm_ses->exit_lab_processing) {
        ALOGE("%s: No active buffering", __func__);
        pthread_mutex_unlock(&p_lsm_ses->lock);
        return -EIO;
    }
    pthread_mutex_unlock(&p_lsm_ses->lock);

    status = read_pcm_data(p_lsm_ses, buf, bytes);

    return status;
}

static void route_audio_capture_cpe(st_hw_session_t *p_ses)
{
    SET_STATE(p_ses->state, SES_BUFFERING);
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;

    pthread_mutex_lock(&p_lsm_ses->lock);
    p_lsm_ses->exit_lab_processing = false;
    pthread_mutex_unlock(&p_lsm_ses->lock);

    if (p_ses->vendor_uuid_info &&
        p_ses->vendor_uuid_info->kw_transfer_mode == FTRT_TRANSFER_MODE) {
        process_packetized_lab_data(p_lsm_ses);
    } else {
        process_raw_lab_data_cpe(p_lsm_ses);
    }
}

int route_send_custom_chmix_coeff_cpe(st_hw_session_t *p_ses __unused, char *str __unused)
{
    /* custom chmix coeff not supported for CPE */
    return 0;
}

static int route_disable_device(st_hw_session_t *p_ses, bool setting_device)
{
    int status = 0;
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t*)p_ses;

    if (!setting_device || (p_ses->exec_mode == ST_EXEC_MODE_CPE))
        return status;

    ALOGD("%s: Enter", __func__);

    status = ape_stop(p_ses);
    if (status)
        return status;

    if (p_lsm_ses->pcm)
        pcm_stop(p_lsm_ses->pcm);

    ape_enable_use_case(false, p_ses);
    ape_enable_port_control(false, p_ses);

    status = sound_trigger_set_device(p_ses, false);
    if (status)
        return status;

    ALOGD("%s: Exit", __func__);
    return status;
}

static int route_enable_device(st_hw_session_t *p_ses, bool setting_device)
{
    int status = 0;
    int retry_num = 0;
    struct st_vendor_info *v_info = p_ses->vendor_uuid_info;
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t*)p_ses;
    audio_devices_t capture_device = 0;

    if (p_ses->exec_mode == ST_EXEC_MODE_CPE)
        return status;

    ALOGD("%s: Enter", __func__);

    capture_device = platform_stdev_get_capture_device(p_ses->stdev->platform);

    platform_get_lsm_usecase(p_ses->stdev->platform, v_info,
        &p_lsm_ses->lsm_usecase, p_ses->exec_mode);
    if (!p_lsm_ses->lsm_usecase) {
        ALOGE("%s: failed to allocate lsm usecase for the session", __func__);
        return -ENODEV;
    }

    status = platform_stdev_send_calibration(p_ses->stdev->platform,
                                             capture_device,
                                             p_ses->exec_mode,
                                             p_ses->vendor_uuid_info,
                                             v_info->app_type, true,
                                             ST_SESSION_CAL);

    status = send_lsm_input_hw_params(p_ses);
    if (status)
        return status;

    /*
     * If the setting_device flag is true, the session is in the active state.
     * Otherwise, session is in the loaded state and bringup will happen
     * when the session is started/resumed.
     */
    if (!setting_device)
        return status;

    status = sound_trigger_set_device(p_ses, true);
    if (status)
        return status;

    if (ape_enable_port_control(true, p_ses) == 0) {
        status = lsm_set_port(p_lsm_ses);
        if (!status)
            ape_enable_use_case(true, p_ses);
    } else {
        ape_enable_use_case(true, p_ses);
        status = lsm_set_port(p_lsm_ses);
    }
    if (status) {
        ALOGE("%s: ERROR. set port failed, returned status %d",
              __func__, status);
        goto exit_1;
    }

    status = pcm_start(p_lsm_ses->pcm);
    while (status && (retry_num < SOUND_TRIGGER_PCM_MAX_RETRY)) {
        usleep(SOUND_TRIGGER_PCM_SLEEP_WAIT);
        retry_num++;
        ALOGE("%s: pcm_start retrying..status %d: %s, retry cnt %d",
              __func__, status, strerror(errno), retry_num);
        status = pcm_start(p_lsm_ses->pcm);
    }
    if (status) {
        ALOGE("%s: ERROR. pcm_start failed, %s",
              __func__, strerror(errno));
        goto exit_1;
    }

    status = ape_start(p_ses);
    if (status)
        goto exit_2;

    ALOGD("%s: Exit", __func__);
    return status;

exit_2:
    pcm_stop(p_lsm_ses->pcm);

exit_1:
    ape_enable_use_case(false, p_ses);
    ape_enable_port_control(false, p_ses);
    sound_trigger_set_device(p_ses, false);
    return status;
}

static void request_exit_callback_thread(st_hw_session_lsm_t *p_lsm_ses)
{
    int status;
    int wait_result;
    struct timespec timeout;

    pthread_mutex_lock(&p_lsm_ses->callback_thread_lock);
    p_lsm_ses->exit_callback_thread = true;
    for (int i = 0; i < LSM_ABORT_RETRY_COUNT; i++) {
        ATRACE_BEGIN("sthal:lsm: pcm_ioctl sndrv_lsm_abort_event");
        status = pcm_ioctl(p_lsm_ses->pcm, SNDRV_LSM_ABORT_EVENT);
        ATRACE_END();
        GET_WAIT_TIMESPEC(timeout, LSM_ABORT_WAIT_TIMEOUT_NS);
        wait_result = pthread_cond_timedwait(&p_lsm_ses->callback_thread_cond,
                                                 &p_lsm_ses->callback_thread_lock, &timeout);
        if (wait_result == ETIMEDOUT) {
            ALOGW("%s: wait for callback thread exit timeout, retry...", __func__);
            continue;
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&p_lsm_ses->callback_thread_lock);

    if (status)
        ALOGE("%s: ERROR. SNDRV_LSM_ABORT_EVENT failed, status=%d", __func__,
              status);
}

static int get_param_data(st_hw_session_t *p_ses, const char *param,
                          void *payload, size_t payload_size,
                          size_t *param_data_size)
{
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;
    int ret = -EINVAL, i = 0;
    unsigned int count = 0;
    size_t param_size = 0;
    char doa_mixer_ctl_name[MIXER_PATH_MAX_LENGTH] = "Doa Tracking Monitor Listen";
    struct mixer_ctl *ctl = NULL;
    st_ffv_doa_tracking_monitor_t doa_data = {{0}, {0}, {0}};
    struct qsthw_source_tracking_param st_params = {{0}, {0}, {0}};

    if (!param || !payload || !param_data_size) {
        ALOGE("%s: ERROR. Invalid params", __func__);
        goto exit;
    }

    if (p_lsm_ses->common.exec_mode == ST_EXEC_MODE_CPE) {
        ALOGE("%s: ERROR. GetParam unsupported for CPE", __func__);
        goto exit;
    }

    ALOGD("%s:[%d] Enter param %s", __func__, p_lsm_ses->common.sm_handle, param);
    if (!strncmp(param, QSTHW_PARAMETER_DIRECTION_OF_ARRIVAL,
            sizeof(QSTHW_PARAMETER_DIRECTION_OF_ARRIVAL))) {
        param_size = sizeof(struct qsthw_source_tracking_param);
        if (payload_size < param_size) {
            ALOGE("%s: ERROR. Invalid payload size %zu", __func__, payload_size);
            goto exit;
        }

        ret = platform_stdev_derive_mixer_ctl_from_backend(p_ses->stdev->platform,
                                                    doa_mixer_ctl_name);
        if (ret) {
            ALOGE("%s: ERROR. Could not derive mixer ctl from backend", __func__);
            goto exit;
        }

        ctl = mixer_get_ctl_by_name(p_ses->stdev->mixer, doa_mixer_ctl_name);
        if (!ctl) {
            ALOGE("%s: ERROR. Could not get ctl for mixer cmd - %s",
                   __func__, doa_mixer_ctl_name);
            goto exit;
        }

        mixer_ctl_update(ctl);
        count = mixer_ctl_get_num_values(ctl);
        if (count != sizeof(st_ffv_doa_tracking_monitor_t)) {
            ALOGE("%s: ERROR. Invalid doa data size %d", __func__, count);
            goto exit;
        }

        ret = mixer_ctl_get_array(ctl, (void *)&doa_data, count);
        if (!ret) {
            /* copy adsp doa payload to client requested format */
            for (i = 0; i < MAX_DOA_TRACKING_ANGLES; i++) {
                st_params.target_angle_L16[i] = doa_data.target_angle_L16[i];
                ALOGV("%s: target angle[%d] = %d", __func__, i,
                       st_params.target_angle_L16[i]);
            }

            for (i = 0; i < MAX_DOA_TRACKING_ANGLES; i++) {
                st_params.interf_angle_L16[i] = doa_data.interf_angle_L16[i];
                ALOGV("%s: interference angle[%d] = %d", __func__, i,
                       st_params.interf_angle_L16[i]);
            }

            memcpy(&st_params.polarActivityGUI, &doa_data.polarActivityGUI,
                   DOA_POLAR_ACTIVITY_INDICATORS);

            memcpy(payload, &st_params, param_size);
            *param_data_size = param_size;
        } else {
            ALOGE("%s: ERROR. Invalid doa data returned", __func__);
        }
    } else {
        ALOGE("%s: ERROR. Unsupported param %s", __func__, param);
        goto exit;
    }

exit:
    return ret;
}

static int send_detection_request(st_hw_session_t *p_ses)
{
    int status = 0, stage_idx = 0;
    st_hw_session_lsm_t *p_lsm_ses =
       (st_hw_session_lsm_t *)p_ses;
    struct lsm_param_custom_config custom_conf_params = {{0}, {0}};
    struct snd_lsm_module_params lsm_params = {0};
    struct st_module_param_info *mparams = NULL;
    lsm_param_info_t param_info = {0};
    lsm_param_payload_t lsm_param_payload = {0};
    unsigned char *custom_payload = NULL;
    uint32_t data_payload_size = 0, lsm_param_payload_size = 0;
    uint32_t data_payload_addr_lsw = 0, data_payload_addr_msw = 0;
    uint32_t custom_payload_size = 0, mem_map_handle = 0;

    if (!(p_lsm_ses->lsm_usecase->param_tag_tracker &
          PARAM_REQUEST_DETECTION_BIT)) {
        ALOGE("%s: No request detection IDs set, exiting", __func__);
        return -EINVAL;
    }

    mparams = p_lsm_ses->lsm_usecase->params;
    stage_idx = LSM_STAGE_INDEX_FIRST;
    if (platform_stdev_get_xml_version(p_ses->stdev->platform) >
        PLATFORM_XML_VERSION_0x0102) {
        lsm_fill_param_header(&lsm_param_payload,
            lsm_param_payload_size, &mparams[REQUEST_DETECTION]);

        custom_payload_size = sizeof(lsm_param_payload);
        custom_payload = (unsigned char *)calloc(1, custom_payload_size);
        if (!custom_payload) {
            ALOGE("%s: ERROR. Cannot allocate memory for custom_payload",
                __func__);
            status = -ENOMEM;
            goto error_exit;
        }

        memcpy(custom_payload, &lsm_param_payload,
            sizeof(lsm_param_payload));
    } else {
        param_info.param_type = LSM_CUSTOM_PARAMS;
        lsm_param_payload_size = sizeof(custom_conf_params) -
            sizeof(custom_conf_params.set_param);
        data_payload_size = lsm_param_payload_size;
        fill_set_params_payload(&custom_conf_params.set_param,
            data_payload_size, data_payload_addr_lsw, data_payload_addr_msw,
            mem_map_handle);

        lsm_fill_param_header((lsm_param_payload_t *)&custom_conf_params.common,
            lsm_param_payload_size, &mparams[REQUEST_DETECTION]);

        custom_payload_size = sizeof(struct lsm_param_custom_config);
        custom_payload = (unsigned char *)calloc(1, custom_payload_size);
        if (!custom_payload) {
            ALOGE("%s: ERROR. Cannot allcoate memory for custom_payload",
                __func__);
            status = -ENOMEM;
            goto error_exit;
        }

        memcpy(custom_payload, &custom_conf_params,
            sizeof(struct lsm_param_custom_config));
    }

    param_info.param_size = custom_payload_size;
    param_info.param_data = (unsigned char *)custom_payload;
    lsm_fill_param_info(LSM_CUSTOM_PARAMS, &param_info,
                        &mparams[REQUEST_DETECTION], stage_idx);

    lsm_params.params = (unsigned char*)&param_info;
    lsm_params.num_params = 1;
    lsm_params.data_size = sizeof(lsm_param_info_t);

    ST_DBG_DECLARE(FILE *req_event_fd = NULL;
        static int req_event_cnt = 0);
    ST_DBG_FILE_OPEN_WR(req_event_fd, ST_DEBUG_DUMP_LOCATION,
                        "requested_event_lsm", "bin", req_event_cnt++);
    ST_DBG_FILE_WRITE(req_event_fd, param_info.param_data,
                      param_info.param_size);
    ST_DBG_FILE_CLOSE(req_event_fd);

    status = lsm_set_module_params(p_lsm_ses, &lsm_params);
    if (status)
        ALOGE("%s: ERROR. setting module params. status %d",
              __func__, status);

error_exit:
    if (custom_payload)
        free(custom_payload);

    return status;
}

int st_hw_sess_lsm_init(st_hw_session_t *const p_ses,
    hw_ses_event_callback_t cb, void* cookie, st_exec_mode_t exec_mode,
    struct st_vendor_info* v_info, sound_model_handle_t sm_handle,
    sound_trigger_device_t *stdev)
{
    int status = 0;
    pthread_condattr_t c_attr;
    pthread_condattr_t c_attr_buf;
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;

    p_ses->exec_mode = exec_mode;
    if (ST_EXEC_MODE_CPE == exec_mode) {
        ALOGV("%s: CPE mode", __func__);
        p_ses->fptrs = &cpe_fptrs;
    } else if (ST_EXEC_MODE_ADSP == exec_mode){
        ALOGV("%s: APE mode", __func__);
        p_ses->fptrs = &ape_fptrs;
    }

    p_ses->callback_to_st_session = cb;
    p_ses->cookie = cookie;
    p_ses->vendor_uuid_info = v_info;
    p_ses->sm_handle = sm_handle;
    p_ses->stdev = stdev;
    p_ses->is_generic_event = false;
    p_ses->client_req_det_mode = ST_HW_SESS_DET_UNKNOWN_MODE;

    p_lsm_ses->exit_lab_processing = false;
    p_lsm_ses->lab_processing_active = false;
    list_init(&p_ses->lsm_ss_cfg_list);

    pthread_condattr_init(&c_attr);
    pthread_condattr_setclock(&c_attr, CLOCK_MONOTONIC);
    pthread_cond_init(&p_lsm_ses->callback_thread_cond, &c_attr);
    pthread_mutex_init(&p_lsm_ses->callback_thread_lock, NULL);

    pthread_mutex_init(&p_lsm_ses->lock,
        (const  pthread_mutexattr_t *) NULL);
    pthread_condattr_init(&c_attr_buf);
    pthread_condattr_setclock(&c_attr_buf, CLOCK_MONOTONIC);
    pthread_cond_init(&p_lsm_ses->cond, &c_attr_buf);

    return status;
}

void st_hw_sess_lsm_deinit(st_hw_session_t *const p_ses)
{
    st_hw_session_lsm_t *p_lsm_ses = (st_hw_session_lsm_t *)p_ses;

    pthread_cond_destroy(&p_lsm_ses->callback_thread_cond);
    pthread_mutex_destroy(&p_lsm_ses->callback_thread_lock);
    pthread_cond_destroy(&p_lsm_ses->cond);
    pthread_mutex_destroy(&p_lsm_ses->lock);
}
