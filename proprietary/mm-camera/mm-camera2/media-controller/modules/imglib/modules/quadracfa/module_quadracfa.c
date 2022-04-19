/**********************************************************************
*  Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "module_imgbase.h"

/**
 *  Static functions
 **/
static int32_t module_quadracfa_client_created(imgbase_client_t *p_client);
static boolean module_quadracfa_query_mod(mct_pipeline_cap_t* p_mct_cap,
  void* p_mod,unsigned int sessionid);
static boolean module_quadracfa_handle_set_stream_config(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled);

/** g_caps:
 *
 *  Set the capabilities for quadracfa (quadracfa) module
*/
static img_caps_t g_caps = {
  .num_input = 1,
  .num_output = 1,
  .num_meta = 1,
  .inplace_algo = 0,
  .num_release_buf = 1,
  .use_internal_bufs = 0,
  .preload_mode = IMG_PRELOAD_COMMON_STREAM_CFG_MODE
};

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_client_created = module_quadracfa_client_created,
  .imgbase_query_mod = module_quadracfa_query_mod,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_SET_STREAM_CONFIG] =
    module_quadracfa_handle_set_stream_config,
  .exec_mode = IMG_EXECUTION_SW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .force_cache_op = FALSE,
};

/**
 * Function: module_quadracfa_client_created
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
int32_t module_quadracfa_client_created(imgbase_client_t *p_client)
{
   /* process seemore on all frames regardless of if HAL3 has a
    request on it */
  p_client->process_all_frames = TRUE;
  return IMG_SUCCESS;
}

/** module_quadracfa_init:
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
mct_module_t *module_quadracfa_init(const char *name)
{
  IDBG_LOW("  E loading");
  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    NULL,
    &g_caps,
    "libmmcamera_quadracfa.so",
    CAM_QCOM_FEATURE_QUADRA_CFA,
    &g_params);
}

/**
 * Function: module_quadracfa_query_mod
 *
 * Description: This function is used to query Quadra CFA caps
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
boolean module_quadracfa_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* p_mod,unsigned int sessionid)
{
  mct_pipeline_pp_cap_t *buf;
  IDBG_LOW("  E ");

  IMG_UNUSED(p_mod);
  IMG_UNUSED(sessionid);
  if (!p_mct_cap) {
    IDBG_ERROR("Error");
    return FALSE;
  }

  buf = &(p_mct_cap->pp_cap);
  buf->is_remosaic_lib_present = TRUE;

  buf->quadra_cfa_format = CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG;

  IDBG_HIGH("is_remosaic_lib_present %d",
    buf->is_remosaic_lib_present);

  return TRUE;
}

/**
 * Function: module_quadracfa_handle_set_stream_config
 *
 * Description: This function handles module event
 *
 * Arguments:
 *   identity - stream idenity
 *   p_mod_event - pointer of module event
 *   p_client - pointer to imgbase client
 *   p_core_ops - pointer to imgbase module ops
 *   is_evt_handled - if evt is handled
 *
 * Return values:
 *     TRUE/FALSE
 *
 * Notes: none
 **/
boolean module_quadracfa_handle_set_stream_config(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled)
{
  int32_t rc = IMG_SUCCESS;
  module_imgbase_t *p_mod;
  sensor_out_info_t *p_sensor_info;

  if (!p_mod_event || !p_client || !p_core_ops) {
    IDBG_ERROR("Error input");
    return TRUE;
  }
  p_sensor_info = (sensor_out_info_t *)p_mod_event->module_event_data;
  if (!p_sensor_info) {
    IDBG_ERROR("Error input %p", p_sensor_info);
    return TRUE;
  }
  p_mod = (module_imgbase_t *)p_client->p_mod;
  IDBG_HIGH("MCT_EVENT_MODULE_SET_STREAM_CONFIG, w = %u, h = %u",
    p_sensor_info->dim_output.width, p_sensor_info->dim_output.height);

  imgbase_session_data_t* p_session_data = IMGBASE_SSP(p_mod,
    p_client->session_id);
  if (!p_session_data) {
    IDBG_ERROR("Error no session data");
    return TRUE;
  }

  p_session_data->preload_params.param.custom_cal_data =
    p_sensor_info->custom_cal_data;
  p_session_data->preload_params.param.full_s_dim.height =
    p_sensor_info->full_height;
  p_session_data->preload_params.param.full_s_dim.width =
    p_sensor_info->full_width;
  p_session_data->preload_params.param.filter_arrangement =
    p_sensor_info->filter_arrangement;
  p_session_data->preload_params.param.pedestal =
    p_sensor_info->pedestal;

  IDBG_MED("[%s] trying preload session_cnt %d, sesspreload done%d,"
    " mod preloaddone %d",
    p_mod->name, p_mod->session_cnt,
    p_session_data->preload_params.preload_done,
    p_mod->preload_done);

  if ((IMG_PRELOAD_COMMON_STREAM_CFG_MODE == p_mod->caps.preload_mode) &&
    (!p_mod->preload_done)) {
    IDBG_MED("[%s] Do preload", p_mod->name);
    rc = module_imgbase_client_preload(p_session_data);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("[%s] Preload failed", p_mod->name);
    }
  }

  return TRUE;
}
