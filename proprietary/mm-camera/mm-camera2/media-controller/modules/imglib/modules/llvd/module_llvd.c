/**********************************************************************
*  Copyright (c) 2014-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
#include "module_imgbase.h"

/**
 *  Static functions
 **/

static int32_t module_llvd_client_created(imgbase_client_t *p_client);
static void module_llvd_set_seemore_tune_params();
static int32_t module_llvd_client_destroy(imgbase_client_t *p_client);
static int32_t module_llvd_client_process_done(imgbase_client_t *p_client,
  img_frame_t *p_frame);
static int32_t module_llvd_client_streamon(imgbase_client_t * p_client);
static int32_t module_llvd_client_handle_ssr(imgbase_client_t *p_client);
static int32_t module_llvd_session_start(void *p_imgbasemod,
  uint32_t sessionid);
static int32_t module_llvd_update_meta(imgbase_client_t *p_client,
  img_meta_t *p_meta);
static int32_t module_llvd_process_param(mct_event_control_parm_t *ctrl_parm,
  imgbase_client_t *p_client, img_core_ops_t *p_core_ops);
static boolean module_llvd_handle_aec(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled);
static boolean module_llvd_handle_buffer_divert(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled);

/**
 * Macros
 */
#define LLVD_MIN_DELTA_HYST 20
#define LLVD_MAX_DELTA_HYST 40

/** LLVD_CMP_AND_APPLY_RET:
 *
 *  macro to compare string and apply parameters
 */
#define LLVD_CMP_AND_APPLY_RET(s_param, key, param, value) ({ \
  if (!strncmp((s_param), (key), strlen(s_param))) { \
    param = atof(value); \
    return; \
  } \
})

/** g_cfg:
 *
 *  Set the tuning parameters for seemore (LLVD) module.
*/
static img_seemore_cfg_t g_cfg = {
  .br_intensity = 0.8f,
  .br_color = 0.6f,
  .enable_LTM = 1,
  .enable_TNR = 1,
  .tnr_intensity = 0.6f, /* default value when hysteresis is not used */
  .mot_det_sensitivity = 0.4f, /* default value when hysteresis is not used */
};

/** g_tuning:
 *
 *  Set the tuning parameters for hysteresis
*/
static img_seemore_tuning_t g_tuning = {
  .num_regions = 3,
  .type = IMG_TRIGGER_LUX_IDX,
  .delta_hyst = 30,
  .trigger[0] =
  {
    .t_start = 200,
    .tnr_intensity = 0.9f,
    .mot_det_sensitivity = 0.5f,
  },
  .trigger[1] =
  {
    .t_start = 300,
    .tnr_intensity = 0.9f,
    .mot_det_sensitivity = 0.5f,
  },
  .trigger[2] =
  {
    .tnr_intensity = 0.9f,
    .mot_det_sensitivity = 0.5f,
  }
};

/** g_caps:
 *
 *  Set the capabilities for SeeMore (LLVD) module
*/
static img_caps_t g_caps = {
  .num_input = 2,
  .num_output = 0,
  .num_meta = 1,
  .inplace_algo = 1,
  .num_release_buf = 0,
  .num_overlap = 1,
};

/**
 * Store the default configuration
 */
static img_seemore_cfg_t g_init_cfg;

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_client_created = module_llvd_client_created,
  .imgbase_client_destroy = module_llvd_client_destroy,
  .imgbase_client_streamon = module_llvd_client_streamon,
  .imgbase_client_process_done = module_llvd_client_process_done,
  .imgbase_client_handle_ssr = module_llvd_client_handle_ssr,
  .imgbase_session_start = module_llvd_session_start,
  .imgbase_client_update_meta = module_llvd_update_meta,
  .imgbase_handle_ctrl_parm[CAM_INTF_META_NOISE_REDUCTION_MODE] =
    module_llvd_process_param,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_STATS_AEC_UPDATE] =
    module_llvd_handle_aec,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_BUF_DIVERT] =
    module_llvd_handle_buffer_divert,
  .exec_mode = IMG_EXECUTION_HW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .force_cache_op = FALSE,
};


/** img_llvd_session_data_t:
*
*   @disable_preview: disable llvd for preview
*  .@noisered_mode: noise reduction mode
*
*   Session based parameters for llvd module
*/
typedef struct {
  bool disable_preview;
  uint8_t noisered_mode;
  img_aec_info_t aec_info;
} img_llvd_session_data_t;


/** img_llvd_client_t:
*
*   @p_session_data: pointer to the session based data
*   @ignore_noisered: flag to ignore noise reduction mode
*   @prev_noisered: previous noise reduction mode flag
*   @p_client: imgbase client
*   @prev_reg_idx: index of the previous region
*   @aec_info: AEC information
*   @llvd_cfg: llvd configuration
*   llvd client private structure
*/
typedef struct {
  img_llvd_session_data_t *p_session_data;
  uint8_t ignore_noisered;
  uint8_t prev_noisered;
  imgbase_client_t *p_client;
  uint32_t prev_reg_idx;
  img_aec_info_t aec_info;
  img_seemore_cfg_t llvd_cfg;
} img_llvd_client_t;


/** img_llvdmod_priv_t:
*
*  @session_data: llvdmod session data
*
*  llvdmodule private structure
*/
typedef struct {
  img_llvd_session_data_t session_data[MAX_IMGLIB_SESSIONS];
} img_llvdmod_priv_t;

/**
 * Function: module_llvd_parse
 *
 * Description: Callback function for the seemore parser
 *
 * Input parameters:
 *   p_userdata - test object
 *   key - attribute
 *   value - value of the attribute
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static void module_llvd_parse(void *p_userdata, char *key, char *value)
{
  uint32_t i;
  char buf[64];

  IDBG_MED("[SEEMORE_PARSE_DBG] attr %s val %s ", key, value);
  LLVD_CMP_AND_APPLY_RET("br_intensity", key, g_cfg.br_intensity, value);
  LLVD_CMP_AND_APPLY_RET("br_color", key, g_cfg.br_color, value);
  LLVD_CMP_AND_APPLY_RET("num_regions", key, g_tuning.num_regions, value);
  LLVD_CMP_AND_APPLY_RET("delta_hyst", key, g_tuning.delta_hyst, value);

  for (i = 0; i < IMGLIB_ARRAY_SIZE(g_tuning.trigger); i++) {
    snprintf(buf, sizeof(buf), "trigger%d_t_start", i);
    LLVD_CMP_AND_APPLY_RET(buf, key, g_tuning.trigger[i].t_start, value);
    snprintf(buf, sizeof(buf), "trigger%d_tnr_intensity", i);
    LLVD_CMP_AND_APPLY_RET(buf, key, g_tuning.trigger[i].tnr_intensity, value);
    snprintf(buf, sizeof(buf), "trigger%d_mot_det_sensitivity", i);
    LLVD_CMP_AND_APPLY_RET(buf, key, g_tuning.trigger[i].mot_det_sensitivity,
      value);
  }
}

/**
 * Function: module_llvd_client_created
 *
 * Description: function called after client creation
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_llvd_client_created(imgbase_client_t *p_client)
{
  IDBG_MED("%s:%d: E", __func__, __LINE__);
  bool disable_preview = 0;
  img_llvd_client_t *p_llvd_client;
  module_imgbase_t *p_mod =
    (module_imgbase_t *)p_client->p_mod;
  img_llvdmod_priv_t *p_llvdmod_priv =
    (img_llvdmod_priv_t *)p_mod->mod_private;

  p_client->rate_control = TRUE;
  p_client->exp_frame_delay = 0LL;
  p_client->ion_fd = open("/dev/ion", O_RDONLY);
  p_client->before_cpp = TRUE;
  p_client->feature_mask = CAM_QCOM_FEATURE_LLVD;

  /* alloc llvd client private data */
  p_llvd_client = calloc(1, sizeof(img_llvd_client_t));
  if (!p_llvd_client) {
    IDBG_ERROR("%s:%d] llvd client data alloc failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  /* update llvd client priv data */
  p_llvd_client->p_session_data =
    &p_llvdmod_priv->session_data[p_client->session_id - 1];
  p_client->p_private_data = p_llvd_client;
  p_llvd_client->p_client = p_client;

  disable_preview =
    p_llvdmod_priv->session_data[p_client->session_id - 1].disable_preview;

#ifdef _LE_CAMERA_
  p_llvd_client->ignore_noisered = FALSE;
  p_client->processing_disabled = TRUE;
#else
  p_llvd_client->ignore_noisered = TRUE;
  p_client->processing_disabled = FALSE;
#endif

  IDBG_MED("ignore_noisered %d processing_disabled %d",
    p_llvd_client->ignore_noisered, p_client->processing_disabled);

  /* process seemore only when bufs received on port where
    video stream is mapped */
  p_client->streams_to_process = 1 << CAM_STREAM_TYPE_VIDEO;
  if (!disable_preview) {
    p_client->streams_to_process |= 1 << CAM_STREAM_TYPE_PREVIEW;
  }

  IDBG_HIGH("%s:%d: disable_preview %d streams_to_process %x",
    __func__, __LINE__, disable_preview, p_client->streams_to_process);

  /* process seemore on all frames regardless of if HAL3 has a
    request on it */
  p_client->process_all_frames = TRUE;

  /* send preferred stream mapping requesting for preview and
    video on the same port */
  p_client->set_preferred_mapping = TRUE;
  p_client->preferred_mapping_single.stream_num = 1;
  p_client->preferred_mapping_single.streams[0].max_streams_num = 2;
  p_client->preferred_mapping_single.streams[0].stream_mask =
    (1 << CAM_STREAM_TYPE_PREVIEW) | (1 << CAM_STREAM_TYPE_VIDEO);
  p_client->preferred_mapping_multi.stream_num = 1;
  p_client->preferred_mapping_multi.streams[0].max_streams_num = 2;
  p_client->preferred_mapping_multi.streams[0].stream_mask =
    (1 << CAM_STREAM_TYPE_VIDEO) | (1 << CAM_STREAM_TYPE_PREVIEW);

  /* overlap batch input requires extra buffers */
  p_client->isp_extra_native_buf += p_mod->caps.num_overlap;

  /* seemore/swtnr requires 128 byte aligned buffers */
  p_client->isp_native_buf_alignment = 128;

  return IMG_SUCCESS;
}

/**
 * Function: module_llvd_set_seemore_tune_params
 *
 * Description: function to get seemore params from setprop
 *
 * Arguments:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static void module_llvd_set_seemore_tune_params()
{
#ifdef _ANDROID_
  char *property_token = NULL;
  char prop[PROPERTY_VALUE_MAX];
  char *saveptr;
  property_get("persist.vendor.camera.imglib.cmor_tune",
    prop, "0.8,0.6,1,1,0.6,0.4");
  property_token = strtok_r(prop, ",", &saveptr);

  if ((property_token != NULL) && ((atof(property_token) >= 0.0)
    && (atof(property_token) <= 1.0))) {
    g_cfg.br_intensity = atof(property_token);
  } else if (property_token != NULL) {
    IDBG_ERROR("br_intensity not in range. Range: [0.0 , 1.0]");
  }
  property_token = strtok_r(NULL, ",", &saveptr);

  if ((property_token != NULL) && ((atof(property_token) >= 0.0)
    && (atof(property_token) <= 1.0))) {
    g_cfg.br_color = atof(property_token);
  } else if (property_token != NULL) {
    IDBG_ERROR("br_color not in range. Range: [0.0 , 1.0]");
  }
  property_token = strtok_r(NULL, ",", &saveptr);

  if ((property_token != NULL) && ((atoi(property_token) == 0)
    || (atoi(property_token) == 1))) {
    g_cfg.enable_LTM = atoi(property_token);
  } else if (property_token != NULL) {
    IDBG_ERROR("enable_LTM is toggle bit either 1 or 0");
  }
  property_token = strtok_r(NULL, ",", &saveptr);

  if ((property_token != NULL) && ((atoi(property_token) == 0)
    || (atoi(property_token) == 1))) {
    g_cfg.enable_TNR = atoi(property_token);
  } else if (property_token != NULL) {
    IDBG_ERROR("enable_TNR is toggle bit either 1 or 0");
  }
  property_token = strtok_r(NULL, ",", &saveptr);

  if ((property_token != NULL) && ((atof(property_token) >= 0.0)
    && (atof(property_token) <= 1.0))) {
    g_cfg.tnr_intensity = atof(property_token);
  } else if (property_token != NULL) {
    IDBG_ERROR("tnr_intensity not in range. Range: [0.0 , 1.0]");
  }
  property_token = strtok_r(NULL, ",", &saveptr);

  if ((property_token != NULL) && ((atof(property_token) >= 0.0)
    && (atof(property_token) <= 1.0))) {
    g_cfg.mot_det_sensitivity = atof(property_token);
  } else if (property_token != NULL) {
    IDBG_ERROR("mod_det_sensitivity is not in range. Range: [0.0 , 1.0]");
  }

  IDBG_MED("tnr_intensity = %f\n mod_det_sensitivity = %f\n"
    "br_color = %f \n br_intensity = %f\n LTM = %d\n"
    "TNR = %d",
    g_cfg.tnr_intensity, g_cfg.mot_det_sensitivity,
    g_cfg.br_color, g_cfg.br_intensity,
    g_cfg.enable_LTM, g_cfg.enable_TNR);
#endif
}

/**
 * Function: module_llvd_session_start
 *
 * Description: function called after session start
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
 int32_t module_llvd_session_start(void *p_imgbasemod, uint32_t sessionid)
{
  module_imgbase_t *p_mod = (module_imgbase_t *)p_imgbasemod;
  img_llvdmod_priv_t *p_llvdmod_priv;
  img_llvd_session_data_t *p_session_data;

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  module_llvd_set_seemore_tune_params();

  if (!p_mod || sessionid > MAX_IMGLIB_SESSIONS) {
    IDBG_ERROR("%s:%d: Invalid input, %p %d", __func__, __LINE__,
      p_mod, sessionid);
    return IMG_ERR_INVALID_INPUT;
  }
  p_llvdmod_priv = (img_llvdmod_priv_t *)p_mod->mod_private;

  if (!p_llvdmod_priv) {
    IDBG_ERROR("%s:%d: Invalid llvd private data, %p", __func__, __LINE__,
      p_llvdmod_priv);
    return IMG_ERR_INVALID_INPUT;
  }
  p_session_data = &p_llvdmod_priv->session_data[sessionid - 1];
  p_session_data->disable_preview =  module_imglib_common_get_prop(
    "camera.llvd.preview.disable", "0");

  IDBG_HIGH("%s:%d] sessionid %d disable_preview %d", __func__, __LINE__,
    sessionid, p_session_data->disable_preview);

  IDBG_MED("%s:%d] X", __func__, __LINE__);

  return IMG_SUCCESS;
}

/**
 * Function: module_llvd_dump_tuning
 *
 * Description: function to dump tuning values
 *
 * Arguments:
 *   None
 *
 * Return values:
 *   None
 *
 * Notes: none
 **/
static void module_llvd_dump_tuning()
{
  uint32_t i;
  IDBG_MED("[SEEMORE_PARAMS_TUNE] br_color %f", g_cfg.br_color);
  IDBG_MED("[SEEMORE_PARAMS_TUNE] br_intensity %f", g_cfg.br_intensity);
  IDBG_MED("[SEEMORE_PARAMS_TUNE] enable_LTM %d", g_cfg.enable_LTM);
  IDBG_MED("[SEEMORE_PARAMS_TUNE] enable_TNR %d", g_cfg.enable_TNR);
  IDBG_MED("[SEEMORE_PARAMS_TUNE] num_regions %d", g_tuning.num_regions);
  IDBG_MED("[SEEMORE_PARAMS_TUNE] delta_hyst %f", g_tuning.delta_hyst);
  for (i = 0; i < g_tuning.num_regions; i++) {
    if (i < g_tuning.num_regions - 1) {
      IDBG_MED("[SEEMORE_PARAMS_TUNE] trigger[%d].t_start %f",
        i, g_tuning.trigger[i].t_start);
    }
    IDBG_MED("[SEEMORE_PARAMS_TUNE] trigger[%d].tnr_intensity %f",
      i, g_tuning.trigger[i].tnr_intensity);
    IDBG_MED("[SEEMORE_PARAMS_TUNE] trigger[%d].mot_det_sensitivity %f",
      i, g_tuning.trigger[i].mot_det_sensitivity);
  }
}

/**
 * Function: module_llvd_client_streamon
 *
 * Description: function called after stream on
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int32_t module_llvd_client_streamon(imgbase_client_t * p_client)
{
  module_imgbase_t *p_mod =
    (module_imgbase_t *)p_client->p_mod;
  img_llvdmod_priv_t *p_llvdmod_priv =
    (img_llvdmod_priv_t *)p_mod->mod_private;
  img_llvd_client_t *p_llvdclient =
    (img_llvd_client_t *)p_client->p_private_data;
  bool disable_preview =
    p_llvdmod_priv->session_data[p_client->session_id - 1].disable_preview;
  imgbase_buf_t *p_temp_buf = NULL;
  int i =0;

  p_client->streams_to_process = 0;
  if (CAM_HAL_V1 == p_mod->hal_version) {
    p_client->streams_to_process = 1 << CAM_STREAM_TYPE_VIDEO;
    if (!disable_preview) {
      p_client->streams_to_process |= 1 << CAM_STREAM_TYPE_PREVIEW;
    }
  }

  /* update */
  g_cfg = g_init_cfg;
  if (module_imglib_common_get_prop("persist.vendor.camera.seemore.tuning", "0")) {
    img_parse_main(QCAMERA_DUMP_FRM_LOCATION"seemore_tuning.txt", NULL,
      module_llvd_parse);
    module_llvd_dump_tuning();
  }

  /*reset*/
  p_llvdclient->prev_reg_idx = 0;

  /*updated client llvd cfg */
  p_llvdclient->llvd_cfg = g_cfg;

  /* reinitialize llvd on streamon */
  p_llvdclient->llvd_cfg.re_initialize = true;

  /* reset buf list to handle usecases like svhdr that
   involves streamon and off without link and unlink */
  for (i = 0; i < MAX_IMGLIB_BATCH_SIZE; i++) {
    p_temp_buf = p_client->buf_list[i];
    if (p_temp_buf) {
      free(p_temp_buf);
      p_temp_buf = NULL;
    }
    p_client->buf_list[i] = NULL;
  }
  p_client->cur_buf_cnt = 0;
  p_client->buf_list_idx = 0;

  return IMG_SUCCESS;
}

/**
 * Function: module_llvd_client_destroy
 *
 * Description: function called before client is destroyed
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_llvd_client_destroy(imgbase_client_t *p_client)
{
  IDBG_MED("%s:%d: E", __func__, __LINE__);

  /* free llvd client data */
  if (p_client->p_private_data) {
    free(p_client->p_private_data);
    p_client->p_private_data = NULL;
  }

  if (p_client->ion_fd >= 0) {
    close(p_client->ion_fd);
    p_client->ion_fd = -1;
  }

  return IMG_SUCCESS;
}

/**
 * Function: module_llvd_client_process_done
 *
 * Description: function called after frame is processed
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *   @p_frame: output frame
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_llvd_client_process_done(imgbase_client_t *p_client,
  img_frame_t *p_frame)
{
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  int rc = IMG_SUCCESS;

  if (p_mod->feature_mask == CAM_QCOM_FEATURE_LLVD) {
    img_component_ops_t *p_comp = &p_client->comp;
    img_lib_config_t lib_config;
    img_comp_execution_mode_t exec_mode = IMG_EXECUTION_UNKNOWN;

    lib_config.lib_param = IMG_ALGO_EXEC_MODE;
    lib_config.lib_data = &exec_mode;

    // Seemore exec mode can dynamically change. So get the last frame mode.
    rc = IMG_COMP_GET_PARAM(p_comp, QIMG_PARAM_LIB_CONFIG,
      &lib_config);
    if (IMG_SUCCEEDED(rc)) {
      p_mod->modparams.exec_mode = exec_mode;
    }
  }

  if (p_mod->modparams.force_cache_op == TRUE) {
    rc = img_cache_ops_external(IMG_ADDR(p_frame),
      IMG_FRAME_LEN(p_frame), 0, IMG_FD(p_frame),
      IMG_CACHE_CLEAN_INV, p_mod->ion_fd);

    if (rc == IMG_SUCCESS) {
      imgbase_buf_t *p_imgbase_buf = p_frame->private_data;
      p_imgbase_buf->buf_divert.buffer_access = 0;
    }
  }

  return rc;
}

/**
 * Function: module_llvd_client_handle_ssr
 *
 * Description: function called after DSP SSR occured
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_llvd_client_handle_ssr(imgbase_client_t *p_client)
{
  IDBG_MED("%s:%d] E", __func__, __LINE__);

  /* In the event of a SSR, SeeMore uses deferred reload. Set
    reload needed to true here and begin reloading during streamoff */
  img_dsp_dl_mgr_set_reload_needed(TRUE);

  return IMG_SUCCESS;
}

/**
 * Function: module_llvd_handle_aec
 *
 * Description: This function handles module event
 *
 * Arguments:
 *   @identity: identity of the event
 *   @p_mod_event - pointer of module event
 *   @p_client - pointer to imgbase client
 *   @p_core_ops - pointer to imgbase module ops
 *   @is_evt_handled: output data to check if the event is
 *                  handled or not.
 *
 * Return values:
 *     TRUE/FALSE
 *
 * Notes: none
 **/
static boolean module_llvd_handle_aec(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled)
{
  stats_update_t *stats_update =
    (stats_update_t *)p_mod_event->module_event_data;
  aec_update_t* aec_update = &stats_update->aec_update;
  img_llvd_client_t *p_llvdclient =
    (img_llvd_client_t *)p_client->p_private_data;
  img_llvd_session_data_t *p_session_data = p_llvdclient->p_session_data;
  img_aec_info_t *p_aec_info = &p_session_data->aec_info;

  p_aec_info->real_gain = aec_update->real_gain;
  p_aec_info->linecount = aec_update->linecount;
  p_aec_info->exp_time  = aec_update->exp_time;
  p_aec_info->lux_idx   = aec_update->lux_idx;
  p_aec_info->iso       = aec_update->exif_iso;
  return true; /* Fwd event */
}

/**
 * Function: module_llvd_handle_buffer_divert
 *
 * Description: Function used to handle buffer divert
 * module event to process frames based on noise
 * reduction mode.
 *
 * Arguments:
 *   @identity: identity of the event
 *   @p_mod_event - pointer of module event
 *   @p_client - pointer to imgbase client
 *   @p_core_ops - pointer to imgbase module ops
 *   @is_evt_handled: output data to check if the event is
 *                  handled or not.
 *
 * Return values:
 *     TRUE/FALSE
 *
 * Notes: none
 **/
static boolean module_llvd_handle_buffer_divert(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled)
{
  module_imgbase_t *p_mod;
  img_llvd_client_t *p_llvd_client;
  img_llvd_session_data_t *p_session_data;
  uint8_t noisered_mode;
  isp_buf_divert_t *p_buf_divert;

  if (!p_mod_event || !p_client || !p_core_ops) {
    IDBG_ERROR("%s:%d] Error input", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_imgbase_t *)p_client->p_mod;
  p_llvd_client = (img_llvd_client_t *)p_client->p_private_data;
  p_buf_divert = (isp_buf_divert_t *)p_mod_event->module_event_data;
  if (!p_mod || !p_llvd_client || !p_buf_divert) {
    IDBG_ERROR("Invalid module or client or event!!!");
    return FALSE;
  }

  p_session_data = p_llvd_client->p_session_data;
  if (!p_session_data) {
    IDBG_ERROR("Invalid session handle %d",p_client->session_id);
    return FALSE;
  }

  // handles only noise reduction part,  rest buffer handling in imgbase
  *is_evt_handled = FALSE;

  // ignore bayer buffer processing, return inplace ack
  if(p_buf_divert->bayerdata == TRUE) {
    p_buf_divert->ack_flag = TRUE;
    *is_evt_handled = TRUE;
    return FALSE;
  }

  //handle noise reduction mode incase of LE and HAL3
  if (CAM_HAL_V3 != p_mod->hal_version || p_llvd_client->ignore_noisered) {
    IDBG_MED("hal_version %d ignore_noisered %d",
      p_mod->hal_version, p_llvd_client->ignore_noisered);
    return TRUE;
  }

  //get noise reduction mode
  noisered_mode = p_session_data->noisered_mode;
  IDBG_MED("noisered_mode %d", noisered_mode);

  if (CAM_NOISE_REDUCTION_MODE_HIGH_QUALITY == noisered_mode) {
    p_client->processing_disabled = FALSE;
  } else {
    if (p_llvd_client->prev_noisered ==
      CAM_NOISE_REDUCTION_MODE_HIGH_QUALITY) {
      IDBG_MED("NR mode disabled, flush triggered");
      module_imgbase_client_flush(p_client);
    }
    p_client->processing_disabled = TRUE;
  }
  p_llvd_client->prev_noisered = noisered_mode;
  return TRUE;
}


/**
 * Function: module_llvd_mp_hysteresis
 *
 * Description: API to perform multi point hysteresis
 *
 * Arguments:
 *   @p_tuning: tuning parameters
 *   @prev_reg_idx: region index for the previous state
 *   @cur_value: Current lux index/gain
 *
 * Return values:
 *    Region index for the new state
 *
 **/
static uint32_t module_llvd_mp_hysteresis(img_seemore_tuning_t *p_tuning,
  uint32_t prev_reg_idx, float cur_value)
{
  uint32_t i;
  /* set last region as default */
  uint32_t reg_idx = p_tuning->num_regions - 1;

  for (i = 0; i < (p_tuning->num_regions - 1); i++) {
    if (cur_value < p_tuning->trigger[i].t_start) {
      reg_idx = i;
      break;
    } else if (cur_value < (p_tuning->trigger[i].t_start +
        p_tuning->delta_hyst)) {
      reg_idx = IMG_MIN(prev_reg_idx, i + 1);
      break;
    }
  }
  return reg_idx;
}

/**
 * Function: module_llvd_update_meta
 *
 * Description: This function is used to called when the base
 *                       module updates the metadata
 *
 * Arguments:
 *   @p_client - pointer to imgbase client
 *   @p_meta: pointer to the image meta
 *
 * Return values:
 *     error values
 *
 * Notes: none
 **/
static int32_t module_llvd_update_meta(imgbase_client_t *p_client,
  img_meta_t *p_meta)
{
  int rc;
  img_llvd_client_t *p_llvdclient = NULL;
  img_seemore_cfg_t *pllvd_cfg = NULL;
  img_llvd_session_data_t *p_session_data;

  uint32_t new_reg_idx = 0;
  if (!p_client || !p_meta) {
    IDBG_ERROR("%s:%d] invalid input %p %p", __func__, __LINE__,
      p_client, p_meta);
    rc = IMG_ERR_INVALID_INPUT;
    goto error;
  }

  p_llvdclient = (img_llvd_client_t *)p_client->p_private_data;
  if (!p_llvdclient) {
    IDBG_ERROR("invalid private data %p", p_llvdclient);
    rc = IMG_ERR_INVALID_INPUT;
    goto error;
  }

  pllvd_cfg = &p_llvdclient->llvd_cfg;
  p_session_data = p_llvdclient->p_session_data;
  memcpy(&p_llvdclient->aec_info, &p_session_data->aec_info,
    sizeof(img_aec_info_t));

  /* Use tuning header and hysteresis when num of regions is set*/
  if ((g_tuning.num_regions > 0) &&
    (g_tuning.num_regions < IMG_MAX_HYST_REGIONS) &&
    (IMG_TRIGGER_LUX_IDX == g_tuning.type) &&
    (g_tuning.delta_hyst >= LLVD_MIN_DELTA_HYST) &&
    (g_tuning.delta_hyst <= LLVD_MAX_DELTA_HYST)) {

    /* get the new region */
    new_reg_idx = module_llvd_mp_hysteresis(&g_tuning,
      p_llvdclient->prev_reg_idx,
      p_llvdclient->aec_info.lux_idx);

    if (new_reg_idx != p_llvdclient->prev_reg_idx) {
      if ((pllvd_cfg->mot_det_sensitivity !=
        g_tuning.trigger[new_reg_idx].mot_det_sensitivity) ||
        (pllvd_cfg->tnr_intensity != g_tuning.trigger[new_reg_idx].tnr_intensity)) {
        pllvd_cfg->re_initialize = true;
      }
      IDBG_MED("[SEEMORE_TUN] Reinit %d Update Trigger region %d %d lux %f",
        pllvd_cfg->re_initialize,
        p_llvdclient->prev_reg_idx, new_reg_idx,
        p_llvdclient->aec_info.lux_idx);
    }

    pllvd_cfg->mot_det_sensitivity =
      g_tuning.trigger[new_reg_idx].mot_det_sensitivity;
    pllvd_cfg->tnr_intensity = g_tuning.trigger[new_reg_idx].tnr_intensity;
  }
  rc = img_set_meta(p_meta, IMG_META_SEEMORE_CFG, pllvd_cfg);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
    goto error;
  }

  IDBG_HIGH("%s:%d] Lux %f region %u %u re_initialize %d", __func__, __LINE__,
    p_llvdclient->aec_info.lux_idx, p_llvdclient->prev_reg_idx,
    new_reg_idx, pllvd_cfg->re_initialize);

  pllvd_cfg->re_initialize = false;

  /* switch to new region */
  p_llvdclient->prev_reg_idx = new_reg_idx;
  return IMG_SUCCESS;

error:
  return rc;
}

/**
 * Function: module_llvd_process_param
 *
 * Description: Function used to handle control
 * param events
 *
 * Arguments:
 *   ctrl_parm - pointer to control param
 *   p_client - pointer to client
 *   p_core_ops - pointer to imgbase module ops
 *
 * Return values:
 *     IMG_SUCCESS/IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
static int32_t module_llvd_process_param(mct_event_control_parm_t *ctrl_parm,
  imgbase_client_t *p_client, img_core_ops_t *p_core_ops)
{
  int rc = IMG_SUCCESS;
  img_llvd_client_t *p_llvd_client;
  img_llvd_session_data_t *p_session_data;

  if (!ctrl_parm || !p_client || !p_core_ops) {
    rc = IMG_ERR_INVALID_INPUT;
    IDBG_ERROR("Invalid Input!!!");
    return rc;
  }

  p_llvd_client = (img_llvd_client_t *)p_client->p_private_data;
  if (!p_llvd_client) {
    rc = IMG_ERR_INVALID_INPUT;
    IDBG_ERROR("Invalid llvd client!!!");
    return rc;
  }

  p_session_data = p_llvd_client->p_session_data;
  if (!p_session_data) {
    rc = IMG_ERR_INVALID_INPUT;
    IDBG_ERROR("Invalid session handle %d",p_client->session_id);
    return rc;
  }

  switch (ctrl_parm->type) {
    case CAM_INTF_META_NOISE_REDUCTION_MODE: {
      if (p_llvd_client->ignore_noisered) {
        IDBG_MED("NR mode Ignored");
        break;
      }
      p_session_data->noisered_mode = *((uint8_t *)ctrl_parm->parm_data);
      IDBG_MED("Updated NR Mode %d", p_session_data->noisered_mode);
      if (CAM_NOISE_REDUCTION_MODE_HIGH_QUALITY ==
        p_session_data->noisered_mode) {
        p_client->processing_disabled = FALSE;
        IDBG_MED("SWTNR Enabled!!!");
      } else {
        if (p_llvd_client->prev_noisered ==
          CAM_NOISE_REDUCTION_MODE_HIGH_QUALITY) {
          IDBG_MED("NR mode disabled, flush triggered");
          module_imgbase_client_flush(p_client);
        }
        p_client->processing_disabled = TRUE;
        IDBG_MED("SWTNR Disabled!!!");
      }
      p_llvd_client->prev_noisered = p_session_data->noisered_mode;
      break;
    }
    default:
      IDBG_MED("default case, type %d", ctrl_parm->type);
      break;
  }
  return rc;
}

/**
 * Function: module_llvd_deinit
 *
 * Description: Function used to deinit SeeMore module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_llvd_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_t *p_mod;
  img_llvdmod_priv_t *p_llvdmod_priv;

  if(!p_mct_mod) {
    IDBG_ERROR("%s:%d] Invalid module %p", __func__, __LINE__, p_mct_mod);
    return;
  }

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (!p_mod) {
    IDBG_ERROR("%s:%d] Invalid base module %p", __func__, __LINE__, p_mod);
    return;
  }
  p_llvdmod_priv = p_mod->mod_private;
  //free llvd mod priv data
  if (p_llvdmod_priv) {
    free(p_llvdmod_priv);
  }

  module_imgbase_deinit(p_mct_mod);
}

/** module_llvd_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: Function used to initialize the SeeMore module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_llvd_init(const char *name)
{
  img_llvdmod_priv_t *p_llvdmod_priv = calloc(1, sizeof(img_llvdmod_priv_t));
  if (!p_llvdmod_priv) {
    IDBG_ERROR("%s:%d] llvd private data alloc failed!!!", __func__, __LINE__);
    return NULL;
  }

  g_init_cfg = g_cfg;
  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    p_llvdmod_priv,
    &g_caps,
    "libmmcamera_llvd.so",
    CAM_QCOM_FEATURE_LLVD,
    &g_params);
}

/** module_llvd_set_parent:
 *
 *  Arguments:
 *  @p_parent - parent module pointer
 *
 * Description: This function is used to set the parent pointer
 *              of the LLVD (SeeMore) module
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_llvd_set_parent(mct_module_t *p_mct_mod, mct_module_t *p_parent)
{
  return module_imgbase_set_parent(p_mct_mod, p_parent);
}

/**
 * Function: module_sw_tnr_deinit
 *
 * Description: Function used to deinit SeeMore module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_sw_tnr_deinit(mct_module_t *p_mct_mod)
{
  module_llvd_deinit(p_mct_mod);
}

/** module_sw_tnr_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: Function used to initialize the SW TNR module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_sw_tnr_init(const char *name)
{
  img_llvdmod_priv_t *p_swtnrmod_priv = calloc(1, sizeof(img_llvdmod_priv_t));
  if (!p_swtnrmod_priv) {
    IDBG_ERROR("%s:%d] swtnr private data alloc failed!!", __func__, __LINE__);
    return NULL;
  }

  g_init_cfg = g_cfg;
  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    p_swtnrmod_priv,
    &g_caps,
    "libmmcamera_sw_tnr.so",
    CAM_QTI_FEATURE_SW_TNR,
    &g_params);
}

/** module_sw_tnr_set_parent:
 *
 *  Arguments:
 *  @p_parent - parent module pointer
 *
 * Description: This function is used to set the parent pointer
 *              of the SW TNR module
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_sw_tnr_set_parent(mct_module_t *p_mct_mod, mct_module_t *p_parent)
{
  return module_llvd_set_parent(p_mct_mod, p_parent);
}
