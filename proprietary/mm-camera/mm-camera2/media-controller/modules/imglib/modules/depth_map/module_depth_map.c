/**********************************************************************
*  Copyright (c) 2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
#include "module_imgbase.h"

/** img_depthmap_session_data_t:
 *   Session based parameters for depthmap module
 */
typedef struct {
  cam_format_t sensor_img_format;
  cam_fps_range_t fps_range;
  cam_ae_mode_type depthmap_exp_mode;
  int64_t          depthmap_exp_val;
} img_depthmap_session_data_t;


/** img_depthmapmod_priv_t:
 *  @session_data: depthmapmod session data
 *  depthmapmodule private structure
 */
typedef struct {
  img_depthmap_session_data_t session_data;
} img_depthmapmod_priv_t;

/**
 *  Static functions
 **/
static int32_t module_depth_map_client_created(imgbase_client_t *p_client);
static int32_t module_depth_map_client_destroy(imgbase_client_t *p_client);
static int32_t module_depth_map_client_streamon(imgbase_client_t * p_client);
static boolean module_depth_map_handle_isp_out_dim_event(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled);
static int32_t module_depth_map_handle_ctrl_parm(mct_event_control_parm_t
  *p_ctrl_parm, imgbase_client_t *p_client, img_core_ops_t *p_core_ops);
static void module_depth_map_set_config_param(img_depthmap_session_data_t
  *session_data, cam_intf_parm_type_t param_type, void *data);
static boolean module_depth_map_send_config_data(imgbase_client_t *p_client);

/** g_caps: TODO
 *
 *  Set the capabilities for depth_map  module
*/
static img_caps_t g_caps = {
  .num_input = 1,
  .num_output = 1,
  .num_meta = 1,
  .inplace_algo = 0,
  .num_release_buf = 0,
  .num_overlap = 0,
};

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_client_created = module_depth_map_client_created,
  .imgbase_client_destroy = module_depth_map_client_destroy,
  .imgbase_client_streamon = module_depth_map_client_streamon,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_ISP_OUTPUT_DIM] =
    module_depth_map_handle_isp_out_dim_event,
  .imgbase_handle_ctrl_parm[CAM_INTF_PARM_FPS_RANGE] =
    module_depth_map_handle_ctrl_parm,
  .imgbase_handle_ctrl_parm[CAM_INTF_META_AEC_MODE] =
    module_depth_map_handle_ctrl_parm,
  .imgbase_handle_ctrl_parm[CAM_INTF_META_SENSOR_EXPOSURE_TIME] =
    module_depth_map_handle_ctrl_parm,
  .exec_mode = IMG_EXECUTION_HW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .force_cache_op = FALSE,
};

/**
 * Function: module_depth_map_client_created
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
int32_t module_depth_map_client_created(imgbase_client_t *p_client)
{
  IDBG_LOW("E");
  module_imgbase_t *p_mod =
    (module_imgbase_t *)p_client->p_mod;

  p_client->exp_frame_delay = 0LL;
  p_client->ion_fd = open("/dev/ion", O_RDONLY);
  p_client->before_cpp = FALSE;
  p_client->feature_mask = CAM_QTI_FEATURE_DEPTH_MAP;

  /* process depth only when bufs received on port where depth stream mapped */
  p_client->streams_to_process = 1 << CAM_STREAM_TYPE_DEPTH;

  /* Do not process all depth map rames until HAL3 has a
    request on it */
  p_client->process_all_frames = FALSE;

  /* overlap batch input requires extra buffers */
  p_client->isp_extra_native_buf += p_mod->caps.num_overlap;

  IDBG_LOW("X");

  return IMG_SUCCESS;
}

/**
 * Function: module_depth_map_client_streamon
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
static int32_t module_depth_map_client_streamon(imgbase_client_t * p_client)
{
  IDBG_LOW("E");
  p_client->processing_disabled = FALSE;
  return IMG_SUCCESS;
}

/**
 * Function: module_depth_map_client_destroy
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
int32_t module_depth_map_client_destroy(imgbase_client_t *p_client)
{
  IDBG_LOW(" E");
  if (p_client->ion_fd >= 0) {
    close(p_client->ion_fd);
    p_client->ion_fd = -1;
  }
  IDBG_LOW(" X");
  return IMG_SUCCESS;
}

/**
  * Function: module_depth_map_send_config_data
  *
  * Description: function to send config data to lib
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
boolean module_depth_map_send_config_data(imgbase_client_t *p_client) {
  boolean rc = FALSE;
  mct_stream_info_t *stream_info = NULL;
  if ( !p_client ) {
    IDBG_ERROR("Error input");
    return rc;
  }

  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  img_component_ops_t *p_comp = &p_client->comp;

  if ( !p_comp || !p_mod ) {
    IDBG_ERROR("Error input");
    return rc;
  }

  int idx_depth = module_imgbase_find_stream_idx_by_type(p_client,
    CAM_STREAM_TYPE_DEPTH);
  if (idx_depth > -1) {
    p_comp = &p_client->comp;
    if (p_comp && p_comp->handle) {

      img_depthmapmod_priv_t *p_depthmapmod_priv =
        (img_depthmapmod_priv_t *)p_mod->mod_private;
      if (!p_depthmapmod_priv) {
        IDBG_ERROR("Invalid private module data");
        return rc;
      }

      img_lib_depth_map_config_t lib_config;
      lib_config.lib_param = IMG_ALGO_PRESTART;

      img_depthmap_session_data_t *p_depthmap_session_data =
        &(p_depthmapmod_priv->session_data);

      if (!p_depthmap_session_data) {
        IDBG_ERROR("Invalid priavte session data");
        return rc;
      }

      stream_info = p_client->stream[idx_depth].stream_info;
      p_depthmap_session_data->sensor_img_format = stream_info->fmt;

      lib_config.depth_map_sensor_img_format =
        p_depthmap_session_data->sensor_img_format;
      lib_config.depth_map_fps_range.min_fps       =
       p_depthmap_session_data->fps_range.min_fps;
      lib_config.depth_map_fps_range.max_fps       =
        p_depthmap_session_data->fps_range.max_fps;
      lib_config.depth_map_fps_range.video_min_fps =
        p_depthmap_session_data->fps_range.video_min_fps;
      lib_config.depth_map_fps_range.video_max_fps =
        p_depthmap_session_data->fps_range.video_max_fps;
      lib_config.depth_map_exp_mode                =
        p_depthmap_session_data->depthmap_exp_mode;
      lib_config.depth_map_exp_val                 =
        p_depthmap_session_data->depthmap_exp_val;

      IDBG_HIGH("Sending depth_map_sensor_img_format =%d, min_fps= %.2f ,\
        max_fps=%.2f, video_min_fps= %.2f, video_max_fps = %.2f,\
        exposure mode = %d exposure val = %lld",
        lib_config.depth_map_sensor_img_format,
        lib_config.depth_map_fps_range.min_fps,
        lib_config.depth_map_fps_range.max_fps,
        lib_config.depth_map_fps_range.video_min_fps,
        lib_config.depth_map_fps_range.video_max_fps,
        lib_config.depth_map_exp_mode,
        lib_config.depth_map_exp_val);

      rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_LIB_CONFIG, &lib_config);
      if (IMG_ERROR(rc)) {
          IDBG_ERROR("Fail :QIMG_PARAM_LIB_CONFIG");
      } else {
        IDBG_HIGH("Send to lib-Success");
        rc = TRUE;
      }
    }
  } else {
    IDBG_ERROR("CAM_STREAM_TYPE_DEPTH not set !!");
  }
  return rc;
}

/**
 * Function: module_depth_map_handle_isp_out_dim_event
 *
 * Description: module_depth_map_handle_isp_out_dim_event
 *
 * Arguments:
 *   @p_client
 *   dentity - IMGLIB_BASE client
 *   @p_mod_event:
 *   @p_core_ops:
 *   @is_evt_handled:
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
boolean module_depth_map_handle_isp_out_dim_event(uint32_t identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client, img_core_ops_t *p_core_ops,
  boolean *is_evt_handled)
{
  boolean fwd_event = true;
  IDBG_LOW("E");

  if (!p_mod_event || !p_client || !p_core_ops ) {
    IDBG_ERROR("Error input");
    return fwd_event;
  }

  if (!module_depth_map_send_config_data(p_client)) {
    IDBG_ERROR("Error module_depth_map_send_config_data");
    return fwd_event;
  }

  //Base module needs information for i/p buffer
  *is_evt_handled = FALSE;

  IDBG_LOW("X");
  return fwd_event;
}

/**
 * Function: module_depth_map_handle_ctrl_parm
 *
 * Description: This function handles the ctrl parms
 *
 * Arguments:
 *   p_ctrl_parm - pointer of ctrl parm
 *   p_client - pointer to imgbase client
 *   p_core_ops - pointer to imgbase module ops
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int32_t module_depth_map_handle_ctrl_parm(mct_event_control_parm_t
  *p_ctrl_parm, imgbase_client_t *p_client, img_core_ops_t *p_core_ops)
{

  IDBG_ERROR("E");

  module_imgbase_t *p_mod;
  int rc = IMG_SUCCESS;
  if (!p_ctrl_parm || !p_client || !p_core_ops ) {
    IDBG_ERROR("Error input");
    return IMG_ERR_INVALID_OPERATION;
  }
  p_mod = (module_imgbase_t *)p_client->p_mod;

  if ( !p_mod ) {
    IDBG_ERROR("Error input");
    return IMG_ERR_INVALID_OPERATION;
  }

  img_depthmapmod_priv_t *p_depthmapmod_priv =
    (img_depthmapmod_priv_t *)p_mod->mod_private;
  if (!p_depthmapmod_priv) {
    IDBG_ERROR("Invalid private module data");
    return IMG_ERR_INVALID_OPERATION;
  }

  img_depthmap_session_data_t *p_depthmap_session_data =
    &(p_depthmapmod_priv->session_data);

  if (!p_depthmap_session_data) {
    IDBG_ERROR("Invalid private session data");
    return IMG_ERR_INVALID_OPERATION;
  }

  IDBG_LOW("E. param_type %d", p_ctrl_parm->type);

  module_depth_map_set_config_param(p_depthmap_session_data,
    p_ctrl_parm->type, p_ctrl_parm->parm_data);

  IDBG_LOW("UPDATED depth_map_sensor_img_format = %d, min_fps= %.2f, \
    max_fps=%.2f, video_min_fps= %.2f, video_max_fps = %.2f, \
    exposure mode = %d  exposure val = %lld",
    p_depthmap_session_data->sensor_img_format,
    p_depthmap_session_data->fps_range.min_fps,
    p_depthmap_session_data->fps_range.max_fps,
    p_depthmap_session_data->fps_range.video_min_fps,
    p_depthmap_session_data->fps_range.video_max_fps,
    p_depthmap_session_data->depthmap_exp_mode,
    p_depthmap_session_data->depthmap_exp_val);

  if (!module_depth_map_send_config_data(p_client)) {
    IDBG_ERROR("Error module_depth_map_send_config_data");
     rc = IMG_ERR_INVALID_OPERATION;
  }

  IDBG_ERROR("X");
  return rc;
}

/**
 * Function: module_depth_map_set_config_param
 *
 * Description: store latets config param
 *
 * Arguments:
 *   session_data - pointer to  img_depthmap_session_data_t
 *   param_type - of type cam_intf_parm_type_t
 *   lib_config - pointer to img_lib_depth_map_config_t
 *
 * Return values:
 *     None
 *
 * Notes: none
 **/

void module_depth_map_set_config_param(img_depthmap_session_data_t
  *session_data, cam_intf_parm_type_t param_type, void *data) {

  IDBG_LOW("E. param_type %d", param_type);

  switch (param_type) {
  case CAM_INTF_PARM_FPS_RANGE:
    session_data->fps_range.min_fps = ((cam_fps_range_t *)(data))->min_fps;
    session_data->fps_range.max_fps = ((cam_fps_range_t *)(data))->max_fps;
    session_data->fps_range.video_min_fps =
      ((cam_fps_range_t *)(data))->video_min_fps;
    session_data->fps_range.video_max_fps =
      ((cam_fps_range_t *)(data))->video_max_fps;
    IDBG_LOW("CAM_INTF_PARM_FPS_RANGE min_fps= %.2f , max_fps=%.2f,\
      video_min_fps= %.2f, video_max_fps = %.2f",
      session_data->fps_range.min_fps,
      session_data->fps_range.max_fps,
      session_data->fps_range.video_min_fps,
      session_data->fps_range.video_max_fps);
    break;

  case CAM_INTF_META_AEC_MODE:
    session_data->depthmap_exp_mode = *((uint32_t *)(data));
    IDBG_LOW("CAM_INTF_META_AEC_MODE exposure mode = %d",
      session_data->depthmap_exp_mode);
    break;

  case CAM_INTF_META_SENSOR_EXPOSURE_TIME:
    session_data->depthmap_exp_val = *((int64_t *)(data));
    IDBG_LOW("CAM_INTF_META_SENSOR_EXPOSURE_TIME exposure val = %lld",
      session_data->depthmap_exp_val);
  break;

  default:
    break;
  }

  IDBG_ERROR("X");
}

/**
 * Function: module_depth_map_deinit
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
void module_depth_map_deinit(mct_module_t *p_mct_mod)
{
  IDBG_LOW("E");

  module_imgbase_t *p_mod = NULL;
  img_depthmapmod_priv_t *p_depthmapmod_priv = NULL;

  if(!p_mct_mod) {
    IDBG_ERROR("Invalid module %p", p_mct_mod);
    return;
  }

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (!p_mod) {
    IDBG_ERROR("Invalid base module %p", p_mod);
    return;
  }

  p_depthmapmod_priv = p_mod->mod_private;
  if (p_depthmapmod_priv) {
    free(p_depthmapmod_priv);
    p_depthmapmod_priv = NULL;
  }

  module_imgbase_deinit(p_mct_mod);
  IDBG_LOW("X");
}

/** module_depth_map_init:
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
mct_module_t *module_depth_map_init(const char *name)
{
  IDBG_LOW("E");

  img_depthmapmod_priv_t *p_depthmap_priv_data = NULL;
  p_depthmap_priv_data = calloc(1, sizeof(img_depthmapmod_priv_t ));

  if(p_depthmap_priv_data == NULL) {
    IDBG_ERROR("Unable allocate module private data");
    return NULL;
  }

  p_depthmap_priv_data->session_data.depthmap_exp_mode = CAM_AE_MODE_OFF;
  p_depthmap_priv_data->session_data.depthmap_exp_val = -1;
  p_depthmap_priv_data->session_data.fps_range.max_fps = 0.0f;
  p_depthmap_priv_data->session_data.fps_range.min_fps = 0.0f;
  p_depthmap_priv_data->session_data.fps_range.video_max_fps = 0.0f;
  p_depthmap_priv_data->session_data.fps_range.video_min_fps = 0.0f;
  p_depthmap_priv_data->session_data.sensor_img_format = CAM_FORMAT_DEPTH8;


  IDBG_LOW("DEFAULT depth_map_sensor_img_format =%d, min_fps= %.2f ,\
    max_fps=%.2f, video_min_fps= %.2f, video_max_fps = %.2f,\
    exposure mode = %d exposure val = %lld",
    p_depthmap_priv_data->session_data.sensor_img_format,
    p_depthmap_priv_data->session_data.fps_range.min_fps,
    p_depthmap_priv_data->session_data.fps_range.max_fps,
    p_depthmap_priv_data->session_data.fps_range.video_min_fps,
    p_depthmap_priv_data->session_data.fps_range.video_max_fps ,
    p_depthmap_priv_data->session_data.depthmap_exp_mode,
    p_depthmap_priv_data->session_data.depthmap_exp_val);

  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    p_depthmap_priv_data,
    &g_caps,
    "libmmcamera_depth_map.so",
    CAM_QTI_FEATURE_DEPTH_MAP,
    &g_params);
}

/** module_depth_map_set_parent:
 *
 *  Arguments:
 *  @p_parent - parent module pointer
 *
 * Description: This function is used to set the parent pointer
 *              of the depth_map  module
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_depth_map_set_parent(mct_module_t *p_mct_mod, mct_module_t *p_parent)
{
  IDBG_LOW("E");
  return module_imgbase_set_parent(p_mct_mod, p_parent);
}


