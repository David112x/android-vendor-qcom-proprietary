/**********************************************************************
*  Copyright (c) 2014-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "module_imgbase.h"

/** STILLMORE_MAX_BURST_LVLS:
 *
 *  Burst count levels
 **/
#define STILLMORE_BURST_LVLS 4

/** STILLMORE_BURST_CNT:
 *
 *  enum for various burst counts
 **/
typedef enum {
  STILLMORE_BURST_CNT_ZERO,
  STILLMORE_BURST_CNT_MIN,
  STILLMORE_BURST_CNT_MID = 3,
  STILLMORE_BURST_CNT_MAX = 5,
} img_stillmore_burst_cnt;

/** g_cfg:
 *
 *  if g_override_cf_default_params is set to TRUE,
 *  use these values to override the chromaflash default
 *  parameters
*/
static img_stillmore_cfg_t g_cfg = {
  .br_intensity = 1.0,
  .br_color = 0.6,
#if HVX_ENABLED
  .smoothing_intensity = 0.5,
#else
  .smoothing_intensity = 0.0,
#endif
};

/**
 *  Static functions
 **/
static boolean module_stillmore_query_mod(mct_pipeline_cap_t*, void* /*pmod*/,unsigned int sessionid);
static int32_t module_stillmore_client_created(imgbase_client_t *p_client);
static int32_t module_stillmore_client_destroy(imgbase_client_t *p_client);
static int32_t module_stillmore_session_start(void *p_imgbasemod,
  uint32_t sessionid);
static int32_t module_stillmore_session_stop(void *p_imgbasemod,
  uint32_t sessionid);

static img_caps_t g_caps = {
#if HVX_ENABLED
  .num_input = STILLMORE_BURST_CNT_MID,
#else
  .num_input = STILLMORE_BURST_CNT_MAX,
#endif
  .num_output = 1,
  .num_meta = 1,
  .inplace_algo = 0,
  .num_release_buf = 1,
};

/**
 * Function: module_stillmore_update_meta
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
int32_t module_stillmore_update_meta(imgbase_client_t *p_client,
  img_meta_t *p_meta)
{
  int rc;
  if (!p_client || !p_meta) {
    IDBG_ERROR("%s:%d] invalid input %p %p", __func__, __LINE__,
      p_client, p_meta);
    rc = IMG_ERR_INVALID_INPUT;
    goto error;
  }

  rc = img_set_meta(p_meta, IMG_META_STILLMORE_CFG, &g_cfg);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
    goto error;
  }

  IDBG_HIGH("%s:%d] Success ", __func__, __LINE__);
  return IMG_SUCCESS;

error:
  return rc;
}


/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_query_mod = module_stillmore_query_mod,
  .imgbase_client_created = module_stillmore_client_created,
  .imgbase_client_destroy = module_stillmore_client_destroy,
  .imgbase_session_start = module_stillmore_session_start,
  .imgbase_session_stop = module_stillmore_session_stop,
  .imgbase_client_update_meta = module_stillmore_update_meta,
  .exec_mode = IMG_EXECUTION_SW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .streams_to_process = IMG_2_MASK(CAM_STREAM_TYPE_OFFLINE_PROC),
  .force_cache_op = FALSE,
};

/** img_stillmore_session_data_t:
 *
 *   @burst_cnt: stillmore burst cnt
 *   @sd_client_id: scene detect client id
 *   @dyn_burst_cnt_flag: dynamic burst count flag
 *
 *   Session based parameters for edge alignment module
 */
typedef struct {
  int32_t burst_cnt;
  uint32_t sd_client_id;
  uint32_t dyn_burst_cnt_flag;
} img_stillmore_session_data_t;

/** img_stillmore_client_t:
 *
 *   @p_session_data: pointer to the session based data
 *   @p_client: imgbase client
 *   @cur_gain: current real gain
 *   @cur_lux_idx: current lux idx
 *
 *   edge client private structure
 */
typedef struct {
  img_stillmore_session_data_t *p_session_data;
  imgbase_client_t *p_client;
  float cur_gain;
  float cur_lux_idx;
} img_stillmore_client_t;

/** img_stillmore_module_t:
 *
 *   @session_data: stillmore session data
 *
 *   stillmore private structure
 */
typedef struct {
  img_stillmore_session_data_t session_data[MAX_IMGLIB_SESSIONS];
} img_stillmore_module_t;

/** img_stillmore_dyn_burst_info:
 *
 *   @threshold: Lux index threshold
 *   @burst_cnt: burst count based on lux range
 *
 *   Lux range and stillmore burst count based on range
 */
typedef struct {
  float threshold;
  uint32_t burst_cnt;
} img_stillmore_dyn_burst_info;

//Array of all lux index thrasholds and corresponding burst count
static img_stillmore_dyn_burst_info dyn_burst_confs[STILLMORE_BURST_LVLS] = {
  {
    .threshold  = 120.0,
    .burst_cnt = STILLMORE_BURST_CNT_ZERO,
  },
  {
    .threshold  = 220.0,
    .burst_cnt = STILLMORE_BURST_CNT_MIN,
  },
  {
    .threshold  = 320.0,
    .burst_cnt = STILLMORE_BURST_CNT_MID,
  },
  {
    .threshold  = 420.0,
    .burst_cnt = STILLMORE_BURST_CNT_MAX,
  },
};

/**
 * Function: module_stillmore_get_burst_cnt
 *
 * Description: This function return the burst count based on
 *   lux idx value
 *
 * Arguments:
 *   lux_idx - lux index
 *
 * Return values:
 *   Burst count based on lux index
 *
 * Notes: none
 **/
uint32_t module_stillmore_get_burst_cnt(float lux_idx)
{
  uint32_t idx;

  for (idx = 0; idx < STILLMORE_BURST_LVLS; idx++) {
    if ((lux_idx <= dyn_burst_confs[idx].threshold)) {
      return dyn_burst_confs[idx].burst_cnt;
    }
  }

  return STILLMORE_BURST_CNT_MAX;
}


/**
 * Function: module_stillmore_deinit
 *
 * Description: This function is used to deinit StillMore module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_stillmore_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}

/**
 * Function: module_stillmore_query_mod
 *
 * Description: This function is used to query StillMore caps
 *
 * Arguments:
 *   @p_mct_cap - capababilities
 *   @p_mod - pointer to the module
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_stillmore_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* p_mod,unsigned int sessionid)
{
  mct_pipeline_imaging_cap_t *buf;

  IMG_UNUSED(p_mod);
  IMG_UNUSED(sessionid);
  if (!p_mct_cap) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return FALSE;
  }

  buf = &p_mct_cap->imaging_cap;

#if HVX_ENABLED
  buf->stillmore_settings.burst_count = STILLMORE_BURST_CNT_MID;
  buf->stillmore_settings.max_burst_count = STILLMORE_BURST_CNT_MID;
#else
  buf->stillmore_settings.burst_count = STILLMORE_BURST_CNT_MAX;
  buf->stillmore_settings.max_burst_count = STILLMORE_BURST_CNT_MAX;
#endif
  buf->stillmore_settings.min_burst_count = STILLMORE_BURST_CNT_MIN;

  return TRUE;
}

/**
 * Function: module_stillmore_config_handle_hysteresis
 *
 * Description: function called by scene detect manager to
 *   handle hysteresis
 *
 * Arguments:
 *  @p_userdata: pointer to the userdata
 *  @session_id: session id
 *  @p_scenelist: scene data list
 *  @p_dyn_data: output dynamic data set by the client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_stillmore_config_handle_hysteresis(
  void *p_userdata,
  uint32_t session_id __unused,
  img_scene_detect_list_t *p_scenelist,
  cam_dyn_img_data_t *p_dyn_data)
{
  img_scene_detect_data_t *p_sd_data;
  uint32_t burst_cnt;
  img_stillmore_session_data_t *p_session_data =
    (img_stillmore_session_data_t *) p_userdata;

  if (p_session_data && p_session_data->dyn_burst_cnt_flag) {
    p_sd_data = &p_scenelist->sd_data[p_scenelist->cur_idx];
    burst_cnt = module_stillmore_get_burst_cnt(p_sd_data->lux_idx);

    if (burst_cnt) {
      //enable stillmore dynamic feature mask
      p_dyn_data->dyn_feature_mask |= CAM_QCOM_FEATURE_STILLMORE;
      p_dyn_data->input_count = burst_cnt;
    } else {
      p_dyn_data->input_count = 1;
    }

    if (p_session_data->dyn_burst_cnt_flag > 1) {
      IDBG_INFO("dyn_feature_mask %llx lux_idx = %f and burst cnt = %d",
        p_dyn_data->dyn_feature_mask, p_sd_data->lux_idx,
        p_dyn_data->input_count);
    } else {
      IDBG_LOW("dyn_feature_mask %llx lux_idx = %f and burst cnt = %d",
        p_dyn_data->dyn_feature_mask, p_sd_data->lux_idx,
        p_dyn_data->input_count);
    }

  } else {
    p_dyn_data->dyn_feature_mask |= CAM_QCOM_FEATURE_STILLMORE;
    p_dyn_data->input_count = STILLMORE_BURST_CNT_MAX;
    IDBG_LOW("dyn stillmore disabled");
  }
  return IMG_SUCCESS;
}

/**
 * Function: module_stillmore_client_created
 *
 * Description: function called after client creation
 *
 * Arguments:
 *   @p_client: imgbase client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_stillmore_client_created(imgbase_client_t *p_client)
{
  int32_t rc = IMG_SUCCESS;
  img_stillmore_client_t *p_stillmore_client;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  img_stillmore_module_t *p_stillmore_mod =
    (img_stillmore_module_t *)p_mod->mod_private;

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  p_stillmore_client = calloc(1, sizeof(img_stillmore_client_t));
  if (!p_stillmore_client) {
    IDBG_ERROR("%s:%d] Failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  p_client->process_all_frames = TRUE;
  p_stillmore_client->p_session_data =
    &p_stillmore_mod->session_data[p_client->session_id - 1];
  p_client->p_private_data = p_stillmore_client;
  p_stillmore_client->p_client = p_client;

  return rc;
}

/**
 * Function: module_stillmore_client_destroy
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
int32_t module_stillmore_client_destroy(imgbase_client_t *p_client)
{
  IDBG_MED("%s %d: E", __func__, __LINE__);
  if (p_client->p_private_data) {
    free(p_client->p_private_data);
    p_client->p_private_data = NULL;
  }
  return IMG_SUCCESS;
}

/**
 * Function: module_stillmore_session_start
 *
 * Description: function called after session start
 *
 * Arguments:
 *   @p_imgbasemod - IMGLIB_BASE module
 *   @sessionid: session ID
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_stillmore_session_start(void *p_imgbasemod, uint32_t sessionid)
{
  img_sd_client_data_t client_data;
  img_stillmore_session_data_t *p_session_data;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_imgbasemod;
  img_stillmore_module_t *p_stillmore_mod =
    (img_stillmore_module_t *)p_mod->mod_private;

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  /* register the client with scene mgr */
  p_session_data = &p_stillmore_mod->session_data[sessionid - 1];
  p_session_data->dyn_burst_cnt_flag =
    module_imglib_common_get_prop("persist.vendor.camera.imglib.dyn_sm", "0");
  client_data.p_appdata = p_session_data;
  client_data.p_detect = module_stillmore_config_handle_hysteresis;

  p_session_data->sd_client_id =
    img_scene_mgr_register(get_scene_mgr(), &client_data);
  if (!p_session_data->sd_client_id) {
    /* non fatal error */
    IDBG_WARN("%s:%d] register error %d", __func__, __LINE__,
      p_session_data->sd_client_id);
  }

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return IMG_SUCCESS;
}


/**
 * Function: module_edge_smooth_session_stop
 *
 * Description: function called after session start
 *
 * Arguments:
 *   @p_imgbasemod: IMGLIB_BASE module
 *   @sessionid: session ID
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_stillmore_session_stop(void *p_imgbasemod, uint32_t sessionid)
{
  int32_t rc;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_imgbasemod;
  img_stillmore_module_t *p_stillmoremod = p_mod->mod_private;
  img_stillmore_session_data_t *p_session_data;

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  /* unregister the client */
  p_session_data = &p_stillmoremod->session_data[sessionid - 1];
  rc = img_scene_mgr_unregister(get_scene_mgr(), p_session_data->sd_client_id);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] img_scene_mgr_unregister failed %d",
      __func__, __LINE__, rc);
    /* non fatal - pass through */
  }

  IDBG_MED("%s:%d] rc %d", __func__, __LINE__, rc);
  return rc;
}

/** module_stillmore_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the stillmore
 *              module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_stillmore_init(const char *name)
{
  img_stillmore_module_t *p_stillmore_mod = calloc(1,
    sizeof(img_stillmore_module_t));
  if (!p_stillmore_mod) {
    IDBG_ERROR("%s:%d] Failed", __func__, __LINE__);
    return NULL;
  }

  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    p_stillmore_mod,
    &g_caps,
    "libmmcamera_stillmore_lib.so",
    CAM_QCOM_FEATURE_STILLMORE,
    &g_params);
}

