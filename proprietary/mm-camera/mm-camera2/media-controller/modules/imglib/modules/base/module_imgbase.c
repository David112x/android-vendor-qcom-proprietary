/**********************************************************************
*  Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "mct_module.h"
#include "module_imgbase.h"
#include "img_thread_ops.h"
#include "img_thread.h"

/**
 * STATIC function declarations
 **/
static mct_port_t *module_imgbase_create_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir);
static int32_t module_imgbase_handle_dyn_img_data(
  imgbase_client_t *p_client,
  cam_dyn_img_data_t *p_dyn_data,
  uint32_t identity);

/** module_imglib_free_func
 *    @data: node data
 *    @user_data: not used. Null pointer
 *
 * Memory release traverse function for flushing queue
 *
 * Returns TRUE
 **/
static boolean module_imgbase_free_config_data(void *data, void *user_data)
{
  if (!data) {
    return FALSE;
  }

  imgbase_frame_config_t *frame_config = (imgbase_frame_config_t *)data;
  free(frame_config->config_data.parm_data);
  free(data);
  return TRUE;
}

/**
 * Function: module_imgbase_find_session_data
 *
 * Description: This method is used to find the session data
 *
 * Arguments:
 *   @p_module: mct module pointer
 *   @sessionid: session id
 *
 * Return values:
 *     pointer to session data entry or null
 *
 * Notes: none
 **/
static imgbase_session_data_t* module_imgbase_find_session_data(
  void* p_module, uint32_t sessionid)
{
  mct_module_t *module = (mct_module_t *)p_module;
  module_imgbase_t *p_mod;

  if (!module || !module->module_private) {
    IDBG_ERROR("failed module %p or module private for sessionid %d",
      module, sessionid);
    return NULL;
  }

  p_mod = (module_imgbase_t *)module->module_private;
  return IMGBASE_SSP(p_mod, sessionid);
}

/**
 * Function: module_imgbase_get_zoom_ratio
 *
 * Description: This method is used to find the session data
 *
 * Arguments:
 *   @p_module: mct module pointer
 *   @sessionid: session id
 *   @zoom_level: zoom level to get zoom ratio
 *
 * Return values:
 *     zoom ratio: float
 *
 * Notes: none
 **/
float module_imgbase_get_zoom_ratio(void* p_module,
  uint32_t sessionid, uint8_t zoom_level)
{
  imgbase_session_data_t* p_session_data = NULL;

  p_session_data = module_imgbase_find_session_data(p_module, sessionid);
  if (!p_session_data) {
    IDBG_ERROR("failed, p_session_data = %p", p_session_data);
    return 0;
  }

  if (zoom_level > p_session_data->zoom_ratio_tbl_cnt) {
    return 0;
  }

  return (float)p_session_data->zoom_ratio_tbl[zoom_level] /
    MOD_IMGLIB_ZOOM_DENUMINATOR;
}
/**
 * Function: module_imgbase_find_identity
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_data: data in the mct list
 *   @p_input: input data to be seeked
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean __unused module_imgbase_find_identity(void *p_data,
  void *p_input)
{
  uint32_t *p_identity = (uint32_t *)p_data;
  uint32_t identity = *((uint32_t *)p_input);

  return (*p_identity == identity);
}

/**
 * Function: module_imgbase_find_stream_by_identity
 *
 * Description: This method is used to find the client based
 *              on stream identity
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @identity: input identity
 *
 * Return values:
 *     Index of the stream in the stream port mapping of the
 *     client
 *
 * Notes: none
 **/
int module_imgbase_find_stream_by_identity(imgbase_client_t *p_client,
  uint32_t identity)
{
  uint32_t i = 0;
  int s_idx = -1;
  for (i = 0; i < p_client->stream_cnt; i++) {
    if (identity == p_client->stream[i].identity) {
      s_idx = i;
      break;
    }
  }
  return s_idx;
}

/**
 * Function: module_imgbase_find_active_shared_stream
 *
 * Description: This method is used to find the active client based
 *                    for shared streams
 *
 * Arguments:
 *   @p_client:  client
 *   @idx: active stream index
 *
 * Return values:
 *     identity of the active stream in the shared streams
 *
 * Notes: none
 **/

uint32_t module_imgbase_find_active_shared_stream_idx(imgbase_client_t *p_client)
{
  uint32_t i = 0;
  int32_t s_idx = -1;

  for (i = 0; i < p_client->stream_cnt; i++) {
    if (p_client->stream[i].streamon) {
      s_idx = i;
      IDBG_MED("Active id %x idx %d", p_client->stream[i].identity, s_idx);
      break;
    }
  }
  return s_idx;
}

/**
 * Function: module_imgbase_find_stream_by_type
 *
 * Description: This method is used to find the client based
 *              on stream type
 *
 * Arguments:
 *   @p_client: cac client
 *   @stream_type: input stream type
 *
 * Return values:
 *     Index of the stream in the stream port mapping of the
 *     client
 *
 * Notes: none
 **/
int module_imgbase_find_stream_idx_by_type(imgbase_client_t *p_client,
  uint32_t stream_type)
{
  uint32_t i = 0;
  int32_t s_idx = -1;
  for (i = 0; i < p_client->stream_cnt; i++) {
    IDBG_MED("%s %d stream_type %d identity %x stream_cnt %d i %d", __func__,
      __LINE__, p_client->stream[i].stream_info->stream_type,
      p_client->stream[i].identity, p_client->stream_cnt,
      i);
    if (stream_type == p_client->stream[i].stream_info->stream_type) {
      s_idx = i;
      break;
    }
  }
  return s_idx;
}

/**
 * Function: module_imgbase_find_session_str_client
 *
 * Description: This method is used to find the client
 * associated with the session stream
 *
 * Arguments:
 *   @p_fp_data: imgbase client
 *   @p_input: input data
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_imgbase_find_session_str_client(void *p_imgbase_data,
  void *p_input)
{
  imgbase_client_t *p_client = (imgbase_client_t *)p_imgbase_data;
  uint32_t *p_session_id = (uint32_t *)p_input;
  boolean flag = FALSE;

  if ((p_client->session_client) && (p_client->session_id == *p_session_id))
    flag = TRUE;

  return flag;
}

/**
 * Function: module_imgbase_find_client
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_fp_data: imgbase client
 *   @p_input: input data
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_imgbase_find_client(void *p_imgbase_data, void *p_input)
{
  imgbase_client_t *p_client = (imgbase_client_t *)p_imgbase_data;
  uint32_t identity = *((uint32_t *)p_input);
  boolean flag = FALSE;
  uint32_t i = 0;

  for (i = 0; i < p_client->stream_cnt; i++) {
    if (p_client->stream[i].identity == identity) {
      flag = TRUE;
      p_client->cur_index = i;
      break;
    }
  }
  return flag;
}

/**
 * Function: module_imgbase_find_client_by_session
 *
 * Description: This method is used to find the client by
 *               session id
 *
 * Arguments:
 *   @p_fp_data: imgbase client
 *   @p_input: input data
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_imgbase_find_client_by_session(void *p_imgbase_data,
  void *p_input)
{
  imgbase_client_t *p_client = (imgbase_client_t *)p_imgbase_data;
  uint32_t identity = *((uint32_t *)p_input);
  boolean flag = FALSE;
  uint32_t i = 0;

  for (i = 0; i < p_client->stream_cnt; i++) {
    if (IMGLIB_SESSIONID(p_client->stream[i].identity) ==
      IMGLIB_SESSIONID(identity)) {
      flag = TRUE;
      break;
    }
  }
  return flag;
}

/**
 * Function: module_imgbase_add_stream_to_mask
 *
 * Description: This method is used to update the stream mask
 *
 * Arguments:
 *   @mask: current stream mask
 *   @stream: stream to add to mask
 *
 * Return values:
 *     error values
 *
 * Notes: none
 **/
static int module_imgbase_add_stream_to_mask(uint32_t *mask,
  uint32_t stream)
{
  if (IMGLIB_OVERFLOW(UINT32_MAX, 1U << stream)) {
    return IMG_ERR_OUT_OF_BOUNDS;
  }
  *mask |= (1 << stream);
  return IMG_SUCCESS;
}

/**
 * Function: module_imgbase_set_mod
 *
 * Description: This function is used to set the module type
 *
 * Arguments:
 *   @module: mct module pointer
 *   @module_type: module type
 *   @identity: id of the stream/session
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void module_imgbase_set_mod(mct_module_t *module,
  unsigned int module_type,
  unsigned int identity)
{
  module_imgbase_t *p_mod;

  if (!(module && module->module_private)) {
    IDBG_ERROR("%s:%d failed, module=%p", __func__, __LINE__, module);
    return;
  }

  /* Do not touch module mode since currently is only one instance module */
  p_mod = (module_imgbase_t *)module->module_private;
  p_mod->module_type = module_type;

  if (mct_module_find_type(module, identity) != MCT_MODULE_FLAG_INVALID) {
    mct_module_remove_type(module, identity);
  }
  mct_module_add_type(module, module_type, identity);

  return;
}

/**
 * Function: module_imgbase_query_mod
 *
 * Description: This function is used to query the imgbase module
 * info
 *
 * Arguments:
 *   @module: mct module pointer
 *   @query_buf: pipeline capability
 *   @sessionid: session identity
 *
 * Return values:
 *     success/failure
 *
 * Notes: none
 **/
boolean module_imgbase_query_mod(mct_module_t *module, void *buf,
  unsigned int sessionid)
{

  mct_pipeline_cap_t *p_mct_cap = (mct_pipeline_cap_t *)buf;
  mct_pipeline_pp_cap_t *p_cap;
  module_imgbase_t *p_mod;

  if (!p_mct_cap || !module || !module->module_private) {
    IDBG_ERROR("%s:%d failed, p_mct_cap=%p, module=%p",
      __func__, __LINE__, p_mct_cap, module);
    goto error;
  }

  p_mod = (module_imgbase_t *)module->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    goto error;
  }

  IDBG_MED("%s_%s:%d: E", __func__, p_mod->name, __LINE__);
  p_cap = &p_mct_cap->pp_cap;
  if (INT64_MAX < p_mod->feature_mask) {
    IDBG_ERROR("%s:%d] feature_mask %lld Overflow error",
      __func__, __LINE__, p_mod->feature_mask);
    goto error;
  }
  p_cap->feature_mask = p_cap->feature_mask | p_mod->feature_mask;

  if (p_mod->modparams.imgbase_query_mod)
    p_mod->modparams.imgbase_query_mod(p_mct_cap, p_mod,sessionid);

  return TRUE;
error:
  return FALSE;
}

/**
 * Function: module_imgbase_set_session_data
 *
 * Description: This function provides session data that has per frame
 *  contorl parameters
 *
 * Arguments:
 *  @module: imglib base module handle
 *  @set_buf: set buffer handle that has session data
 *  @sessionid: session id for which session data shall be
 *            applied
 *
 * Return values:
 *  TRUE on success and FALSE on failure
 *
 * Notes: none
 **/
static boolean module_imgbase_set_session_data(mct_module_t *module,
  void *set_buf, unsigned int sessionid)
{
  imgbase_session_data_t *p_session_data;
  mct_pipeline_session_data_t *session_data =
    (mct_pipeline_session_data_t *)set_buf;

  if (!(module && module->module_private && set_buf && sessionid)) {
    IDBG_ERROR("%s:%d Invalid input %p %p 0x%x", __func__, __LINE__, module,
      set_buf, sessionid);
    return FALSE;
  }

  p_session_data = module_imgbase_find_session_data(module, sessionid);
  if (!p_session_data) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  /* save zoom caps in session data */
  p_session_data->zoom_ratio_tbl_cnt =
    session_data->zoom_ratio_tbl_cnt;
  memcpy(p_session_data->zoom_ratio_tbl, session_data->zoom_ratio_tbl,
    sizeof(session_data->zoom_ratio_tbl));

  p_session_data->max_apply_delay =
    session_data->max_pipeline_frame_applying_delay;
  p_session_data->max_reporting_delay =
    session_data->max_pipeline_meta_reporting_delay;

  return TRUE;
}

/**
 * Function: module_imgbase_forward_port_event
 *
 * Description: This method is used to forward an event
 * depending on the direction.
 *
 * Arguments:
 *   @p_client: Imblib base client
 *   @port: MCT port pointer
 *   @event: MCT event pointer
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_imgbase_forward_port_event(imgbase_client_t *p_client,
  mct_port_t *port,
  mct_event_t *event)
{
  boolean rc = FALSE;
  mct_port_t *p_adj_port = NULL;
  imgbase_stream_t *p_stream;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  int stream_idx;

  stream_idx = module_imgbase_find_stream_by_identity(p_client,
    event->identity);
  if (stream_idx < 0) {
    IDBG_ERROR("%s_%s %d] Cannot find stream mapped to event id %x",
       __func__, p_mod->name, __LINE__, event->identity);
    return FALSE;
  }

  p_stream = &p_client->stream[stream_idx];
  if (NULL == p_stream) {
    IDBG_ERROR("%s_%s:%d] Cannot find stream mapped to client",
      __func__, p_mod->name, __LINE__);
    return FALSE;
  }

  if (MCT_PORT_IS_SINK(port)) {
    p_adj_port = p_stream->p_srcport;
    if (NULL == p_adj_port) {
      /* Not an error for sink module */
      IDBG_LOW("Not an error for sink module !!!");
      return TRUE;
    }
    switch(event->direction) {
      case MCT_EVENT_UPSTREAM : {
        rc =  mct_port_send_event_to_peer(port, event);
        if (rc == FALSE) {
          IDBG_ERROR("%s] Fowarding event %d from sink port failed",
            p_mod->name, event->type);
        }
        break;
      }
      case MCT_EVENT_BOTH:
      case MCT_EVENT_DOWNSTREAM: {
        rc =  mct_port_send_event_to_peer(p_adj_port, event);
        if (rc == FALSE) {
          IDBG_ERROR("%s] Fowarding event %d from sink port failed",
            p_mod->name, event->type);
        }
        break;
      }
      default:
        IDBG_ERROR("%s] Invalid port direction for event %d",
          p_mod->name, event->type);
        break;
    }
  } else if (MCT_PORT_IS_SRC(port)) {
    p_adj_port = p_stream->p_sinkport;
    if (NULL == p_adj_port) {
       IDBG_HIGH("%s_%s:%d] Invalid port", __func__, p_mod->name, __LINE__);
       return FALSE;
    }
    switch(event->direction) {
      case MCT_EVENT_DOWNSTREAM : {
        IDBG_ERROR("%s_%s:%d] Error Downstream event on Src port %d",
          __func__, p_mod->name, __LINE__, event->type);
        break;
      }
      case MCT_EVENT_BOTH:
      case MCT_EVENT_UPSTREAM: {
       rc =  mct_port_send_event_to_peer(p_adj_port, event);
       if (rc == FALSE) {
         IDBG_ERROR("%s_%s:%d] Fowarding event %d from src port failed",
           __func__, p_mod->name, __LINE__, event->type);
       }
       break;
     }
     default:
       IDBG_ERROR("%s_%s:%d] Invalid port direction for event %d",
         __func__, p_mod->name, __LINE__, event->type);
       break;
    }
  }
  return rc;
}

/**
 * Function: module_imgbase_find_stream_handler_by_identity
 *
 * Description: find stream by identity
 *
 * Arguments:
 *   @p_client: imgbase client
 *   @identity: identity
 *
 * Return values:
 *     stream handler or null
 *
 * Notes: none
 **/
imgbase_stream_t* module_imgbase_find_stream_handler_by_identity(
  imgbase_client_t *p_client, uint32_t identity)
{
  uint32_t i;

  for (i = 0; i < p_client->stream_cnt; i++) {
    if (p_client->stream[i].identity == identity) {
      return &p_client->stream[i];
    }
  }

  return NULL;
}

/**
 * Function: module_imgbase_send_preferred_stream_mapping
 *
 * Description: Request a preferred stream-to-port mapping
 *   from ISP
 *
 * Arguments:
 *   @meta_stream_info: stream info
 *   @p_client: imgbase client
 *   @event_idx: event indedx on which to send event
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
uint32_t module_imgbase_send_preferred_stream_mapping(cam_stream_size_info_t
  *meta_stream_info, imgbase_client_t *p_client, uint32_t event_idx)
{
  int rc = IMG_SUCCESS;
  uint32_t i = 0;
  uint32_t number_of_streams_to_process = 0;
  imgbase_stream_t *p_stream = NULL;
  int stream_idx;
  isp_preferred_streams *preferred_streams_map;

  /* check if module which requires special stream mapping is present */
  for (i = 0; i < meta_stream_info->num_streams; i++) {
    if ((meta_stream_info->postprocess_mask[i] & p_client->feature_mask) &&
        ((uint32_t)1 << meta_stream_info->type[i] &
            p_client->streams_to_process)) {
      number_of_streams_to_process++;
    }
  }

  if (number_of_streams_to_process) {

    if (number_of_streams_to_process > 1) {
      preferred_streams_map = &p_client->preferred_mapping_multi;
    } else {
      preferred_streams_map = &p_client->preferred_mapping_single;
    }

    stream_idx = module_imgbase_find_stream_by_identity(p_client,
      event_idx);
    if (stream_idx < 0) {
      IDBG_ERROR("%s:%d] Cannot find stream mapped to idx %x",
        __func__, __LINE__, p_client->divert_identity);
      rc = IMG_ERR_GENERAL;
      goto error;
    }

    p_stream = &p_client->stream[stream_idx];
    if (NULL == p_stream) {
      IDBG_ERROR("%s:%d] Cannot find stream mapped to client %d",
        __func__, __LINE__, p_client->stream_cnt);
      rc = IMG_ERR_GENERAL;
      goto error;
    }

    if (!preferred_streams_map->stream_num) {
      IDBG_ERROR("%s:%d] Failed to set preferred stream mapping",
        __func__, __LINE__);
      rc = IMG_ERR_GENERAL;
      goto error;
    }

    rc = mod_imgbase_send_event(event_idx, TRUE,
      MCT_EVENT_MODULE_SET_PREFERRED_STREAMS_MAPPING, *preferred_streams_map);
    if (!rc) {
      IDBG_ERROR("%s:%d] mod_imgbase_send_event failed %d",
        __func__, __LINE__, rc);
      rc = IMG_ERR_GENERAL;
      goto error;
    }
    rc = IMG_SUCCESS;
    IDBG_MED("%s:%d] Sending preferred stream map to ISP successful",
      __func__, __LINE__);
  }

  return rc;

error:
  return rc;
}

/*
 * Function: module_imgbase_get_process_param_size
 *
 * Description: get param size
 *
 * Arguments:
 *   @type: param type
 *
 * Return values:
 *    param size
 *
 * Notes: none
 **/
static uint32_t module_imgbase_get_process_param_size(
  cam_intf_parm_type_t type)
{
  uint32_t rc = 0;

  switch (type) {
  case CAM_INTF_PARM_ROTATION: {
    rc = sizeof(cam_rotation_info_t);
    break;
  }
  case CAM_INTF_META_NOISE_REDUCTION_MODE: {
    rc = sizeof(cam_noise_reduction_mode_t);
    break;
  }
  case CAM_INTF_PARM_DIS_ENABLE:
  case CAM_INTF_PARM_HAL_VERSION: {
    rc = sizeof(int32_t);
    break;
  }
  case CAM_INTF_PARAM_BOKEH_BLUR_LEVEL: {
    rc = sizeof(cam_rtb_blur_info_t);
    break;
  }
  case CAM_INTF_META_DC_CAPTURE: {
    rc = sizeof(uint8_t);
    break;
  }
  case CAM_INTF_PARM_USERZOOM: {
    rc = sizeof(cam_zoom_info_t);
    break;
  }
  case CAM_INTF_META_USERZOOM: {
    rc = sizeof(cam_zoom_info_t);
    break;
  }
  case CAM_INTF_META_STREAM_ID: {
    rc = sizeof(cam_stream_ID_t);
    break;
  }
  case CAM_INTF_META_STREAM_INFO: {
    rc = sizeof(cam_stream_size_info_t);
    break;
  }
  case CAM_INTF_PARM_FPS_RANGE: {
    rc = sizeof(cam_fps_range_t);
    break;
  }
  case CAM_INTF_META_AEC_MODE: {
    rc = sizeof(uint32_t);
    break;
  }
  case CAM_INTF_META_SENSOR_EXPOSURE_TIME: {
    rc = sizeof(int64_t);
    break;
  }
  default:
    break;
  }

  return rc;
}

/**
 * Function: module_imgbase_fill_preload_dim
 *
 * Description: Calculate max dim for preload
 *
 * Arguments:
 *   @p_client: Imgbase client
 *   @p_meta_stream_info: Ptr to meta stream info structure
 *
 * Return values:
 *    None
 *
 * Notes: none
 **/
static void module_imgbase_fill_preload_dim(imgbase_client_t *p_client,
  cam_stream_size_info_t *p_meta_stream_info)
{
  module_imgbase_t *p_mod;
  imgbase_preload_t *p_preload;
  imgbase_session_data_t* p_session_data;
  uint32_t i;

  p_mod = (module_imgbase_t *)p_client->p_mod;
  p_session_data = IMGBASE_SSP(p_mod, p_client->session_id);
  if (NULL == p_session_data) {
    IDBG_ERROR("[%s] Session Data is NULL. Cannot calculate preload dim",
      p_mod->name);
    return;
  }

  p_preload = &p_session_data->preload_params;

  for (i = 0; i < p_meta_stream_info->num_streams; i++) {
    if ((uint32_t)IMG_2_MASK(p_meta_stream_info->type[i]) &
      p_client->streams_to_process) {

      if ((uint32_t)p_meta_stream_info->stream_sizes[i].width >
        p_preload->param.max_dim.width) {
        p_preload->param.max_dim.width =
          p_meta_stream_info->stream_sizes[i].width;
      }

      if ((uint32_t)p_meta_stream_info->stream_sizes[i].height >
        p_preload->param.max_dim.height) {
        p_preload->param.max_dim.height =
          p_meta_stream_info->stream_sizes[i].height;
      }
    }
  }
  p_preload->param.max_dim.stride = p_preload->param.max_dim.width;
  p_preload->param.max_dim.scanline = p_preload->param.max_dim.height;

  return;
}

/**
 * Function: module_imgbase_process_param
 *
 * Description: param handler function
 *
 * Arguments:
 *   @port: mct port pointer
 *   @ctrl_parm: ctrl data
 *   @identity: identity
 *
 * Return values:
 *    standard image lib return codes
 *
 * Notes: none
 **/
static int32_t module_imgbase_process_param(mct_port_t *port,
  mct_event_control_parm_t *ctrl_parm, uint32_t identity, uint32_t frame_id)
{
  mct_module_t *p_mct_mod;
  module_imgbase_t *p_mod;
  imgbase_client_t *p_client;
  img_core_ops_t *p_core_ops;
  int32_t rc;

  if (!port || !ctrl_parm) {
    IDBG_ERROR("invalid input port=%p, ctrl_parm=%p", port, ctrl_parm);
    return IMG_ERR_INVALID_INPUT;
  }

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("invalid module");
    return IMG_ERR_INVALID_INPUT;
  }

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("imgbase module NULL");
    return IMG_ERR_INVALID_INPUT;
  }

  p_client = (imgbase_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("[%s] imgbase client NULL", p_mod->name);
    return FALSE;
  }

  p_core_ops = &p_mod->core_ops;

  if (ctrl_parm->type < CAM_INTF_PARM_MAX) {
    if (p_mod->modparams.imgbase_handle_ctrl_parm[ctrl_parm->type]) {
      rc = p_mod->modparams.imgbase_handle_ctrl_parm[ctrl_parm->type](
        ctrl_parm, p_client, p_core_ops);
      if ((rc != IMG_ERR_EAGAIN) && (rc != IMG_SUCCESS)) {
        IDBG_ERROR("[%s] handle_ctrl_parm error : type=%d, rc=%d",
          p_mod->name, ctrl_parm->type, rc);
      }
      if (rc != IMG_ERR_EAGAIN) {
        return rc;
      }
    }
  }

  rc = IMG_SUCCESS;

  switch (ctrl_parm->type) {
  case CAM_INTF_PARM_ROTATION: {
    cam_rotation_info_t rotation =
      *(cam_rotation_info_t *)ctrl_parm->parm_data;
    rc = module_imglib_common_meta_set_rotation(&rotation,
      &p_client->current_meta);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("[%s] CAM_INTF_PARM_ROTATION error : rc=%d",
        p_mod->name, rc);
    }
    break;
  }
  case CAM_INTF_PARM_HAL_VERSION: {
    p_mod->hal_version = *((int32_t *)ctrl_parm->parm_data);
    break;
  }
  case CAM_INTF_PARM_DIS_ENABLE: {
    int32_t dis_enable = *(int32_t *)(ctrl_parm->parm_data);
    p_client->dis_enable = !!dis_enable;
    break;
  }
  case CAM_INTF_META_STREAM_ID: {
    uint32_t q_idx = frame_id % PER_FRAME_QUEUE_SIZE;
    imgbase_session_data_t *p_session_data;

    p_session_data = IMGBASE_SSP(p_mod,p_client->session_id);
    if (p_session_data == NULL) {
       IDBG_ERROR("CAM_INTF_META_STREAM_ID failed, session data null \n");
       return IMG_ERR_INVALID_INPUT;
    }

    p_session_data->stream_id_requested[q_idx] = *(cam_stream_ID_t *)(ctrl_parm->parm_data);
    IDBG_HIGH("meta_stream_id identity %x frameid %d", identity, frame_id);
    break;
  }
  case CAM_INTF_META_STREAM_INFO: {
    cam_stream_size_info_t *meta_stream_info =
      (cam_stream_size_info_t *)(ctrl_parm->parm_data);

    IDBG_MED("[%s] CAM_INTF_META_STREAM_INFO, identity=0x%x, %d",
      p_mod->name, identity,
      p_client->set_preferred_mapping);

    if (p_client->set_preferred_mapping) {
      rc = module_imgbase_send_preferred_stream_mapping(
        meta_stream_info, p_client, identity);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("[%s] Send preferred stream mapping failed", p_mod->name);
      }
    }
    imgbase_session_data_t* p_session_data = IMGBASE_SSP(p_mod,
        p_client->session_id);
    if (p_session_data &&
      (IMG_PRELOAD_PER_SESSION_MODE == p_mod->caps.preload_mode) &&
      (!p_session_data->preload_params.preload_done)) {
      if ((!p_session_data->preload_params.param.max_dim.width) ||
        (!p_session_data->preload_params.param.max_dim.height)) {
         IDBG_MED("[%s] Calculate default pre-load params", p_mod->name);
         module_imgbase_fill_preload_dim(p_client, meta_stream_info);
      }
      IDBG_MED("[%s] Do preload", p_mod->name);
      rc = module_imgbase_client_preload(p_session_data);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("[%s] Preload failed", p_mod->name);
      }
    }
    break;
  }
  case CAM_INTF_PARM_EFFECT: {
    p_client->effect = *(uint32_t *)ctrl_parm->parm_data;
    IDBG_MED("[%s] CAM_INTF_PARM_EFFECT client %p effect %d",
      p_mod->name, p_client, p_client->effect);
    break;
  }
  case CAM_INTF_META_IMG_DYN_FEAT: {
    if (!p_client->lock_dyn_update) {
      cam_dyn_img_data_t *p_img_data =
        (cam_dyn_img_data_t *)ctrl_parm->parm_data;
      module_imgbase_handle_dyn_img_data(p_client, p_img_data, identity);
      IDBG_MED("[%s] CAM_INTF_META_IMG_DYN_FEAT mask %llx identity %x",
        p_mod->name, p_img_data->dyn_feature_mask, identity);
    }
    break;
  }
  case CAM_INTF_PARM_LONGSHOT_ENABLE: {
    int8_t enable = *((int8_t *)ctrl_parm->parm_data);
    IDBG_HIGH("[%s] CAM_INTF_PARM_LONGSHOT_ENABLE enable %d",
      p_mod->name, enable);
    p_client->lock_dyn_update = enable;
    break;
  }
  default:
    break;
  }

  return rc;
}

/**
 * Function: module_imgbase_process_super_params
 *
 * Description: Super params handler function
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: mct event
 *
 * Return values:
 *    standard image lib return codes
 *
 * Notes: none
 **/
int32_t module_imgbase_process_super_params(mct_port_t *port,
  mct_event_t *event, bool buf_divert, uint32_t current_frameid)
{
  mct_module_t *p_mct_mod;
  module_imgbase_t *p_mod;
  imgbase_client_t *p_client;
  int32_t rc = IMG_SUCCESS;
  uint32_t sessionid;
  imgbase_session_data_t *p_session_data;

  if (!port || !event) {
    IDBG_ERROR("invalid input, port=%p, event=%p", port, event);
    return IMG_ERR_INVALID_INPUT;
  }

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("invalid module");
    return IMG_ERR_INVALID_INPUT;
  }

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("imgbase module NULL");
    return IMG_ERR_INVALID_INPUT;
  }

  p_client = (imgbase_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("[%s] imgbase client NULL", p_mod->name);
    return FALSE;
  }

  sessionid = IMGLIB_SESSIONID(event->identity);
  p_session_data = IMGBASE_SSP(p_mod, sessionid);
  if (NULL == p_session_data) {
    /* Error printed in the macro*/
    return IMG_ERR_INVALID_INPUT;
  }

  uint32_t frame_id;
  if(buf_divert == TRUE) {
    mct_event_module_t *p_mod_event = &event->u.module_event;
    isp_buf_divert_t *p_buf_divert =
      (isp_buf_divert_t *)p_mod_event->module_event_data;
    frame_id = p_buf_divert->buffer.sequence;
  } else {
    frame_id = current_frameid;
  }

  imgbase_frame_config_t *p_frame_config;

  pthread_mutex_lock(&p_session_data->q_mutex);
  p_frame_config = (imgbase_frame_config_t *)
    mct_queue_look_at_head(&p_session_data->frame_config_q);

  uint32_t q_length = p_session_data->frame_config_q.length;

  while (q_length) {
    p_frame_config = mct_queue_pop_head(&p_session_data->frame_config_q);
    if (p_frame_config->frame_id == frame_id) {
      rc = module_imgbase_process_param(port, &p_frame_config->config_data,
        p_frame_config->identity, frame_id);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("[%s] Cannot process param %d", p_mod->name, rc);
        break;
      }
      module_imgbase_free_config_data(p_frame_config, NULL);
    } else {
        mct_queue_push_tail(&p_session_data->frame_config_q,
          (void *)p_frame_config);
    }
    q_length--;
  }
  pthread_mutex_unlock(&p_session_data->q_mutex);

  return rc;
}

/**
 * Function: module_imgbase_handle_frame_skip
 *
 * Description: Frame skip traverse configuration function
 *
 * Arguments:
 *   @data: mct port pointer
 *   @user_data: pointer to skip frame value
 *
 * Return values:
 *    true/false
 *
 * Notes: none
 **/
static boolean module_imgbase_handle_frame_skip(void* data, void* user_data)
{
  imgbase_frame_config_t *frame_config;
  uint32_t skip_frame_id;

  if (!data || !user_data) {
    IDBG_ERROR("%s:%d, failed, data=%p user data=%p\n", __func__, __LINE__,
      data, user_data);
    return FALSE;
  }

  frame_config = (imgbase_frame_config_t *)data;
  skip_frame_id = *(uint32_t *)user_data;
  if (skip_frame_id <= frame_config->frame_id) {
    frame_config->frame_id++;
  }

  return TRUE;
}


/**
 * Function: module_imgbase_extract_from_reprocess_meta
 *
 * Description: Function to extract from reprocess metadata
 *
 * Arguments:
 *   @p_client: imgbase client
 *   @metadata: metadata pointer
 *   @identity: event identity
 *
 * Return values:
 *   standard image lib return codes
 *
 * Notes: none
 **/
static int32_t __unused module_imgbase_extract_from_reprocess_meta(
  imgbase_client_t *p_client,
  metadata_buffer_t* metadata,
  uint32_t identity)
{
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  cam_feature_mask_t pp_feature_mask;
  imgbase_stream_t *p_stream = NULL;
  int stream_idx;

  stream_idx = module_imgbase_find_stream_by_identity(p_client, identity);
  if (stream_idx < 0) {
    IDBG_ERROR("%s_%s %d] Error, cannot find identity %x", __func__,
      p_mod->name, __LINE__, identity);
    return IMG_ERR_INVALID_INPUT;
  }
  p_stream = &p_client->stream[stream_idx];
  pp_feature_mask =
    p_stream->stream_info->reprocess_config.pp_feature_config.feature_mask;

  IF_META_AVAILABLE(cam_dyn_img_data_t, p_dyn_data, CAM_INTF_META_IMG_DYN_FEAT,
    metadata) {
    /* Disable processing if 1) the current feature has ability to be
      disabled dynamically and 2) the dynamic feature mask does not
      include current feature */
    if ((pp_feature_mask & (CAM_OEM_FEATURE_1 | CAM_OEM_FEATURE_2
      | CAM_QCOM_FEATURE_STILLMORE)) &&
      !(p_dyn_data->dyn_feature_mask & p_mod->feature_mask)) {
      p_client->processing_disabled = TRUE;
    } else {
      p_client->processing_disabled = FALSE;
    }

    IDBG_MED("%s_%s:%d] dyn mask %llx, module mask %x, pp mask %llx, "
      "disabled %d", __func__, p_mod->name, __LINE__,
      p_dyn_data->dyn_feature_mask, p_mod->feature_mask, pp_feature_mask,
      p_client->processing_disabled);
  } else {
    IDBG_ERROR("%s_%s:%d: Dynamic scene data not available in metadata",
      __func__, p_mod->name, __LINE__);
  }


  return IMG_SUCCESS;
}

/**
 * Function: module_imgbase_handle_dyn_img_data
 *
 * Description: Function to handle dynamic img data
 *
 * Arguments:
 *   @p_client: imgbase client
 *   @p_dyn_data: dynamic image data pointer
 *   @identity: event identity
 *
 * Return values:
 *   standard image lib return codes
 *
 * Notes: none
 **/
static int32_t module_imgbase_handle_dyn_img_data(
  imgbase_client_t *p_client,
  cam_dyn_img_data_t *p_dyn_data,
  uint32_t identity)
{
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  cam_feature_mask_t pp_feature_mask;
  imgbase_stream_t *p_stream = NULL;
  int stream_idx;

  stream_idx = module_imgbase_find_stream_by_identity(p_client, identity);
  if (stream_idx < 0) {
    IDBG_ERROR("%s_%s %d] Error, cannot find identity %x", __func__,
      p_mod->name, __LINE__, identity);
    return IMG_ERR_INVALID_INPUT;
  }
  p_stream = &p_client->stream[stream_idx];
  pp_feature_mask =
    p_stream->stream_info->reprocess_config.pp_feature_config.feature_mask;

  if ((pp_feature_mask & (CAM_OEM_FEATURE_1 | CAM_OEM_FEATURE_2 |
    CAM_QCOM_FEATURE_STILLMORE)) && p_dyn_data->dyn_feature_mask &&
    !(p_dyn_data->dyn_feature_mask & p_mod->feature_mask)) {
    p_client->processing_disabled = TRUE;
  } else {
    p_client->processing_disabled = FALSE;
  }
  IDBG_MED("%s_%s:%d] dyn mask %llx, module mask %llx, pp mask %llx, "
    "disabled %d", __func__, p_mod->name, __LINE__,
    p_dyn_data->dyn_feature_mask, p_mod->feature_mask, pp_feature_mask,
    p_client->processing_disabled);

  return IMG_SUCCESS;
}

/**
 * Function: module_imgbase_handle_control_parm_stream_buf
 *
 * Description: Function to handle control_parm_stream_buf event
 *
 * Arguments:
 *   @p_client: imgbase client
 *   @parm_buf: ptr to parm buf
 *   @event_id: event identity
 *   @p_mct_mod: prt to mct module
 *
 * Return values:
 *   standard image lib return codes
 *
 * Notes: none
 **/
static int32_t module_imgbase_handle_control_parm_stream_buf(
  imgbase_client_t *p_client, cam_stream_parm_buffer_t *parm_buf,
  uint32_t event_id, mct_module_t *p_mct_mod)
{
  int32_t rc = IMG_SUCCESS;

  if (!p_client) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;

  if (!parm_buf) {
    IDBG_ERROR("%s_%s:%d invalid input", __func__, p_mod->name, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  switch (parm_buf->type) {
  case CAM_STREAM_PARAM_TYPE_GET_IMG_PROP: {
    cam_stream_parm_buffer_t *out_buf;
    out_buf = img_q_dequeue(&p_client->stream_parm_q);
    if (out_buf) {
      *parm_buf = *out_buf;
      free(out_buf);
    }
    break;
  }
  case CAM_STREAM_PARAM_TYPE_SET_FLIP: {
    int32_t flip_mask = parm_buf->flipInfo.flip_mask;
    IDBG_MED("%s_%s:%d, flip received %d", __func__, p_mod->name,
      __LINE__, flip_mask);
    rc = module_imglib_common_meta_set_flip(flip_mask,
      &p_client->current_meta);
    break;
  }
  case CAM_STREAM_PARAM_TYPE_DO_REPROCESS: {
    break;
  }
  case CAM_STREAM_PARAM_TYPE_FLUSH_FRAME: {
    IDBG_INFO("%s_%s:%d Got CAM_STREAM_PARAM_TYPE_FLUSH_FRAME pclient %p", __func__, p_mod->name, __LINE__, p_client);
    if(p_mod->modparams.imgbase_client_flush_frame) {
      rc = p_mod->modparams.imgbase_client_flush_frame( p_client , event_id ) ;
    }
    break;
  }
  default:
    break;
  }
  return rc;
}

/**
 * Function: module_imgbase_port_event_func
 *
 * Description: Event handler function for the imgbase port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: mct event
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_imgbase_port_event_func(mct_port_t *port,
  mct_event_t *event)
{
  mct_module_t *p_mct_mod;
  module_imgbase_t *p_mod;
  imgbase_client_t *p_client;
  int rc = IMG_SUCCESS;
  boolean fwd_event = TRUE;
  imgbase_stream_t* p_stream = NULL;
  uint32_t i;
  img_core_ops_t *p_core_ops = NULL;
  uint32_t sessionid;
  imgbase_session_data_t *p_session_data;

  if (!port || !event) {
    IDBG_ERROR("invalid input, port=%p, event=%p", port, event);
    return FALSE;
  }
  IDBG_LOW("port %p E", port);

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("invalid module");
    return FALSE;
  }

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("imgbase module NULL");
    return FALSE;
  }

  p_client = (imgbase_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("[%s] imgbase client NULL", p_mod->name);
    return FALSE;
  }

  p_core_ops = &p_mod->core_ops;

  sessionid = IMGLIB_SESSIONID(event->identity);
  p_session_data = IMGBASE_SSP(p_mod, sessionid);
  if (NULL == p_session_data) {
    /* Error printed in the macro*/
    return IMG_ERR_INVALID_INPUT;
  }

  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    mct_event_control_t *p_ctrl_event = &event->u.ctrl_event;
    int stream_idx = module_imgbase_find_stream_by_identity(p_client, event->identity);
    if (stream_idx < 0) {
      IDBG_ERROR("[%s] Cannot find stream mapped to idx %x",p_mod->name,
        event->identity);
      break;
    }

    if (p_ctrl_event->type < MCT_EVENT_CONTROL_MAX) {
      boolean is_evt_handled = TRUE;
      if (p_mod->modparams.imgbase_handle_ctrl_event[p_ctrl_event->type]) {
        fwd_event =
          p_mod->modparams.imgbase_handle_ctrl_event[p_ctrl_event->type](
            p_ctrl_event, p_core_ops, p_client, &is_evt_handled);
        if (is_evt_handled) {
          break;
        }
      }
    }


    p_stream = &p_client->stream[stream_idx];

    switch (p_ctrl_event->type) {
    case MCT_EVENT_CONTROL_STREAMON: {
      p_session_data->stream_on_cnt++;
      p_stream->cur_frame_id = 0;
      p_stream->frames_pending_for_ack = 0;
      p_stream->streamon = true;
      IDBG_LOW("[%s_%x] IMGLIB_BASE STREAMON %d", p_mod->name,
        event->identity, p_session_data->stream_on_cnt);

      mct_stream_info_t *stream_info =
        (mct_stream_info_t *)p_ctrl_event->control_event_data;

      if (p_mod->modparams.imgbase_client_pre_streamon) {
        p_mod->modparams.imgbase_client_pre_streamon(p_client,
          stream_info->identity);
      }

      ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_IMGLIB_STREAMON);
      /*If the session is linked, find peer client in linked sessions*/
      if (p_session_data->multi_cam_info.is_linked) {
        if (p_mod->modparams.imgbase_client_link_peer) {
          rc = p_mod->modparams.imgbase_client_link_peer
            (p_client, event->identity);
          if (!IMG_ERROR(rc)) {
           /* assuming all streams on the client share the same sink port*/
            mct_port_t *peer =
              ((imgbase_client_t *)
              p_client->p_peer_client)->stream[0].p_sinkport;
            MCT_PORT_INTRALINKFUNC(port)(event->identity, port, peer);
            p_client->p_intra_port = port;
          }
        }
      }
      module_imgbase_client_start(p_client, event->identity);
      ATRACE_CAMSCOPE_END(CAMSCOPE_IMGLIB_STREAMON);

      /* If module is only enabled on a particular stream, check if
          current stream is compatible */
      if (p_client->streams_to_process & (1 << stream_info->stream_type)) {
        uint32_t imglib_streamid = IMGLIB_STREAMID(stream_info->identity);
        IDBG_MED("Adding streamid %x for stream type %d to mask, identity %x",
          imglib_streamid, stream_info->stream_type, stream_info->identity);
        rc = module_imgbase_add_stream_to_mask(&p_client->processing_mask,
          imglib_streamid);
        if (IMG_ERROR(rc)) {
          IDBG_ERROR("[%s] Error, rc=%d", p_mod->name, rc);
          return IMG_ERR_OUT_OF_BOUNDS;
        }
      }
      IDBG_MED("[%s] processing mask %x", p_mod->name,
        p_client->processing_mask);

      break;
    }
    case MCT_EVENT_CONTROL_STREAMOFF: {
      p_session_data->stream_on_cnt--;
      IDBG_HIGH("[%s_%x] IMGLIB_BASE STREAMOFF %d", p_mod->name,
        event->identity, p_session_data->stream_on_cnt);
      /*Drop the pending Framedrop queue buffer*/
      module_imgbase_client_handle_framedrop(port, event, 0 , TRUE);

      mct_stream_info_t *stream_info =
        (mct_stream_info_t *)p_ctrl_event->control_event_data;
      uint32_t imglib_streamid = IMGLIB_STREAMID(stream_info->identity);

      IDBG_MED("[%s] processing mask %x", p_mod->name,
        p_client->processing_mask);

      if(p_client->processing_mask) {
        if (p_mod->modparams.imgbase_client_pre_streamoff) {
          p_mod->modparams.imgbase_client_pre_streamoff(
            p_client, stream_info->identity);
        }
      } else {
        IDBG_HIGH("[%s] Processing mask absent!", p_mod->name);
      }
      p_stream->streamon = false;
      //incase of internal output buffers, issue stream off to downstream first
      if (p_mod->caps.use_internal_bufs && p_mod->caps.num_output) {
        p_client->state = IMGLIB_STATE_STOP_PENDING;
        rc = module_imgbase_forward_port_event(p_client, port, event);
        fwd_event = FALSE;
      }

      // no more buffer diverts, flush queue
      if (p_session_data->stream_on_cnt == 0) {
        pthread_mutex_lock(&p_session_data->q_mutex);
        mct_queue_flush(&p_session_data->frame_config_q,
          module_imgbase_free_config_data);
        memset(p_session_data->stream_id_requested, 0,
          PER_FRAME_QUEUE_SIZE * sizeof(cam_stream_ID_t));
        pthread_mutex_unlock(&p_session_data->q_mutex);
      }

      p_client->processing_mask &= ~(1 << imglib_streamid);

      IDBG_MED("[%s] processing mask %x", p_mod->name,
        p_client->processing_mask);

      rc = module_imgbase_client_stop(p_client);

      if (p_session_data->multi_cam_info.is_linked &&
        (!p_client->stream_on) &&
        p_client->p_peer_client) {
        MCT_PORT_INTRAUNLINKFUNC(port);
        p_client->p_peer_client = NULL;
        p_client->p_peer_comp = NULL;
        p_client->p_intra_port = NULL;
      }
      break;
    }
    case MCT_EVENT_CONTROL_PARM_STREAM_BUF: {
      cam_stream_parm_buffer_t *parm_buf =
        event->u.ctrl_event.control_event_data;
      IDBG_MED("[%s] MCT_EVENT_CONTROL_PARM_STREAM_BUF %d",
        p_mod->name, parm_buf ? parm_buf->type : 0xffff);

      if (parm_buf) {
        rc = module_imgbase_handle_control_parm_stream_buf(p_client,
          parm_buf, event->identity, p_mct_mod);
        if (!IMG_SUCCEEDED(rc)) {
          IDBG_ERROR("[%s] Handle control_parm_stream_buf failed",
            p_mod->name);
        }
      }
      break;
    }
    case MCT_EVENT_CONTROL_SET_PARM: {
      mct_event_control_parm_t *ctrl_parm =
        (mct_event_control_parm_t *) event->u.ctrl_event.control_event_data;
      uint32_t frame_id = event->u.ctrl_event.current_frame_id;
      rc = module_imgbase_process_param(port, ctrl_parm, event->identity, frame_id);
      break;
    }
    case MCT_EVENT_CONTROL_SET_SUPER_PARM: {
      imgbase_session_data_t* session_data = module_imgbase_find_session_data(
        p_mct_mod, IMGLIB_SESSIONID(event->identity));

      if (!session_data) {
        IDBG_ERROR("[%s] session_data %p", p_mod->name, session_data);
        return FALSE;
      }

      if (p_session_data->stream_on_cnt == 0) {
        IDBG_MED("[%s] Ignore super_param event, session %p",
          p_mod->name, p_session_data);
        break;
      }

      uint32_t q_idx = 0;
      uint32_t num_configs = ((mct_event_super_control_parm_t *)
        event->u.ctrl_event.control_event_data)->num_of_parm_events;
      mct_event_control_parm_t *config = ((mct_event_super_control_parm_t *)
        event->u.ctrl_event.control_event_data)->parm_events;
      for (i = 0; i < num_configs; i++) {
        uint32_t size = module_imgbase_get_process_param_size(config[i].type);

        if (size) {
          imgbase_frame_config_t *frame_config = (imgbase_frame_config_t *)
            calloc(1, sizeof(imgbase_frame_config_t));
          void *data = malloc(size);

          if (!frame_config || !data) {
            IDBG_ERROR("[%s] frame_config %p data %p",
              p_mod->name, frame_config, data);
            if (frame_config)
              free(frame_config);
            if (data)
              free(data);
            return FALSE;
          }

          frame_config->config_data = config[i];
          frame_config->config_data.parm_data = data;
          memcpy(data, config[i].parm_data, size);

          frame_config->identity = event->identity;
          frame_config->frame_id = session_data->max_apply_delay +
            event->u.ctrl_event.current_frame_id;
          if (frame_config->config_data.type == CAM_INTF_META_STREAM_ID) {
            q_idx = frame_config->frame_id % PER_FRAME_QUEUE_SIZE;
            p_session_data->master_info[q_idx] = 1;
            IDBG_HIGH("META_STREAM_ID mod: %s identity %x frame id %d",
              p_mod->name, frame_config->identity, frame_config->frame_id);
          }

          pthread_mutex_lock(&p_session_data->q_mutex);
          mct_queue_push_tail(&p_session_data->frame_config_q,
            (void *)frame_config);
          pthread_mutex_unlock(&p_session_data->q_mutex);
        }
      }
      break;
    }
    case MCT_EVENT_CONTROL_SOF: {
      if (p_mod->feature_mask & CAM_QTI_FEATURE_RTB) {
        IDBG_MED("[%s] sof tardy update event",p_mod->name);
        rc = module_imgbase_client_handle_sof_notify(port, event);
        if (rc < 0) {
          IDBG_ERROR("[%s] sof tardy update not properly",__func__);
        }
      }
      break;
    }
    case MCT_EVENT_CONTROL_LINK_INTRA_SESSION: {
      uint32_t identity;
      mct_port_t *peer;
      imgbase_client_t *p_peer_port_client;
      IDBG_MED("[%s] MCT_EVENT_CONTROL_LINK_INTRA_SESSION", p_mod->name);
      cam_sync_related_sensors_event_info_t* linking_params =
        (cam_sync_related_sensors_event_info_t*)
        (p_ctrl_event->control_event_data);
      identity = linking_params->related_sensor_session_id;
      peer = mct_port_find_intra_port_across_session(MCT_PORT_SINK,
        port, identity);

      IDBG_MED("[%s] port %p intrapeer %p  identity 0x%x"
        "sessionid %d streamid %d dual cam sensor info %d, camera role %d",
        p_mod->name, port, peer,
        identity, IMGLIB_SESSIONID(identity), IMGLIB_STREAMID(identity),
        linking_params->type, linking_params->cam_role);
      if (peer && port != peer) {
        MCT_PORT_INTRALINKFUNC(port)(identity, port, peer);
        pthread_mutex_lock(&p_session_data->q_mutex);
        p_session_data->multi_cam_info.peer_session_ids[0] = identity;
        p_session_data->multi_cam_info.num_peer_sessions++;
        p_session_data->multi_cam_info.cam_type =
          linking_params->type;
        p_session_data->multi_cam_info.cam_mode = linking_params->mode;
        p_session_data->multi_cam_info.cam_role = linking_params->cam_role;
        p_session_data->multi_cam_info.is_linked = TRUE;
        pthread_mutex_unlock(&p_session_data->q_mutex);
        //Update peer port client info
        p_peer_port_client = (imgbase_client_t *)peer->port_private;
        p_client->p_peer_client = p_peer_port_client;
        p_client->p_peer_comp = &p_peer_port_client->comp;
        p_client->p_intra_port = port;
        IDBG_MED("[%s] sessionid [%d], p_client [%p] sensor_info [%d] port [%p]"
          "camera role [%d], camera mode [%d], is_linked [%d]",
          p_mod->name, sessionid, p_client,
          p_session_data->multi_cam_info.cam_type, port,
          p_session_data->multi_cam_info.cam_role,
          p_session_data->multi_cam_info.cam_mode,
          p_session_data->multi_cam_info.is_linked);
        int index;
        for (index = 0; index < MAX_PORT_CONNECTIONS; index++) {
          IDBG_MED("[%s] peer ports for %p is fileld %d iden %d %p",
            p_mod->name, port,
            port->inter_session_peer[index].is_filled,
            port->inter_session_peer[index].identity,
            port->inter_session_peer[index].port);
        }
      }
      break;
    }
    case MCT_EVENT_CONTROL_HW_SLEEP: {
      cam_dual_camera_perf_control_t *dual_cam_lpm =
        (cam_dual_camera_perf_control_t*)p_ctrl_event->control_event_data;
      if(dual_cam_lpm == NULL) {
        IDBG_ERROR("Invalid input for Dual Cam LPM Info");
        break;
      }
      pthread_mutex_lock(&p_session_data->q_mutex);
      p_session_data->multi_cam_info.low_power_mode = dual_cam_lpm->enable;
      p_session_data->multi_cam_info.perf_mode = dual_cam_lpm->perf_mode;
      pthread_mutex_unlock(&p_session_data->q_mutex);
      break;
    }
    case MCT_EVENT_CONTROL_HW_WAKEUP: {
      cam_dual_camera_perf_control_t *dual_cam_lpm =
        (cam_dual_camera_perf_control_t*)p_ctrl_event->control_event_data;
      if(dual_cam_lpm == NULL) {
        IDBG_ERROR("Invalid input for Dual Cam LPM Info");
        break;
      }
      pthread_mutex_lock(&p_session_data->q_mutex);
      p_session_data->multi_cam_info.low_power_mode = dual_cam_lpm->enable;
      p_session_data->multi_cam_info.perf_mode = dual_cam_lpm->perf_mode;
      pthread_mutex_unlock(&p_session_data->q_mutex);
      break;
    }
    case MCT_EVENT_CONTROL_MASTER_INFO: {
      cam_dual_camera_master_info_t *dual_master_info =
        (cam_dual_camera_master_info_t *)p_ctrl_event->control_event_data;
      if(dual_master_info == NULL) {
        IDBG_ERROR("Invalid input for DUAL CAM MASTER Info");
        break;
      }
      p_session_data->multi_cam_info.cam_mode = dual_master_info->mode;
      break;
    }
    default:
      break;
    }
    break;
  }
  case MCT_EVENT_MODULE_EVENT: {
    mct_event_module_t *p_mod_event = &event->u.module_event;

    p_stream = module_imgbase_find_stream_handler_by_identity(p_client,
      event->identity);
    if (NULL == p_stream) {
      IDBG_ERROR("[%s] stream NULL", p_mod->name);
      return FALSE;
    }

    if (p_mod_event->type < MCT_EVENT_MODULE_MAX) {
      boolean is_evt_handled = TRUE;
      if (p_mod->modparams.imgbase_handle_module_event[p_mod_event->type]) {
        fwd_event =
          p_mod->modparams.imgbase_handle_module_event[p_mod_event->type](
            event->identity, p_mod_event, p_client, p_core_ops, &is_evt_handled);
        if (is_evt_handled) {
          break;
        }
      }
    }

    switch (p_mod_event->type) {
    case MCT_EVENT_MODULE_FRAME_DROP_NOTIFY: {
      p_client->divert_identity = event->identity;
      uint32_t frame_id = p_mod_event->current_frame_id;
      if (p_mod->hal_version == CAM_HAL_V3 &&
        (p_mod->feature_mask & CAM_QTI_FEATURE_RTB)) {
        if (p_stream->stream_info->stream_type == CAM_STREAM_TYPE_PARM)
          break;
        IDBG_HIGH("[%s] FRAME_DROP_NOTIFY identity %x frame id %d stream_type %d",
          p_mod->name,
          p_client->divert_identity, p_mod_event->current_frame_id,
          p_stream->stream_info->stream_type);
        imgbase_framedrop_t *p_drop_data = NULL;
        uint32_t is_master = (p_session_data->multi_cam_info.cam_mode ==
          CAM_MODE_PRIMARY)? 1:0;

        img_component_ops_t *p_comp;
        p_comp = &p_client->comp;
        if (is_master) {
          p_drop_data = calloc(1, sizeof(imgbase_framedrop_t));
          p_drop_data->frameid = frame_id;
          rc = img_q_enqueue(&p_session_data->framedrop_hold_q, p_drop_data);
          if (IMG_ERROR(rc)) {
            IDBG_ERROR("%s:%d] Cannot enqueue bundle", __func__, __LINE__);
          }
        } else {
          rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_FRAME_DROP,
            (void *)(&frame_id));
          if (IMG_ERROR(rc)) {
            IDBG_ERROR("Set QIMG_PARAM_IS_MASTER failed rc %d", rc);
          }
        }
      } else if ((p_mod->hal_version == CAM_HAL_V3 &&
          (p_mod->feature_mask & CAM_QTI_FEATURE_SAT))) {
          if (p_stream->stream_info->stream_type == CAM_STREAM_TYPE_PARM)
            break;
          IDBG_HIGH("[%s] FRAME_DROP_NOTIFY identity %x frame id %d stream_type %d",
            p_mod->name,
            p_client->divert_identity, p_mod_event->current_frame_id,
            p_stream->stream_info->stream_type);
          /*
           * To remove stall stream_id_requested data.
           */
          rc = module_imgbase_process_super_params(port, event, FALSE, frame_id);
          if (IMG_ERROR(rc)) {
            IDBG_ERROR("[%s] cannot process super params rc %d",
            p_mod->name, rc);
          }
          module_imgbase_client_get_and_update_buf_index(p_mod, p_client,
            frame_id, TRUE);

          /* Immediate frame drop notification to component layer */
          img_component_ops_t *p_comp;
          p_comp = &p_client->comp;
          rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_FRAME_DROP,
            (void *)(&frame_id));
          if (IMG_ERROR(rc)) {
            IDBG_ERROR("Set QIMG_PARAM_FRAME_DROP failed rc %d", rc);
          }
      }
      break;
    }
    case MCT_EVENT_MODULE_BUF_DIVERT: {
      uint32_t is_master;
      imgbase_stream_t *p_stream;
      isp_buf_divert_t *p_buf_divert =
        (isp_buf_divert_t *)p_mod_event->module_event_data;
      boolean process_frame = FALSE;

      if (p_client->state == IMGLIB_STATE_ERROR ||
        p_client->state == IMGLIB_STATE_STOP_PENDING) {
        IDBG_ERROR("[%s] Invalid state to handle divert state %d pending 5 error 6",
          p_mod->name, p_client->state);
        if (!(p_client->caps.inplace_algo) ||
          (p_client->caps.will_dim_change)) {
          p_buf_divert->ack_flag = TRUE;
          fwd_event = FALSE;
          break;
        }
      }

      int32_t active_idx = module_imgbase_find_active_shared_stream_idx(p_client);
      if (active_idx < 0) {
        IDBG_MED("[%s] Cannot find stream mapped to idx %x",p_mod->name,
          event->identity);
        p_buf_divert->ack_flag = TRUE;
        fwd_event = FALSE;
        break;
      }

      p_stream = &p_client->stream[active_idx];

      p_client->divert_identity = event->identity;
      IDBG_MED("[%s] [IMG_BUF_DBG_IN_f%d_b%d_i%x] event identity %x"
        " buf_div str_to_proc %x",
        p_mod->name,
        p_buf_divert->buffer.sequence,
        p_buf_divert->buffer.index,
        p_buf_divert->identity,
        p_client->divert_identity,
        p_client->streams_to_process);

      is_master = (p_session_data->multi_cam_info.cam_mode ==
         CAM_MODE_PRIMARY)? 1:0;
      if ((p_mod->feature_mask & CAM_QTI_FEATURE_RTB) && is_master) {
        rc = module_imgbase_client_handle_update_superparms_drop_queue
          (port, event, p_buf_divert->buffer.sequence);
        if (IMG_ERROR(rc)) {
          IDBG_ERROR("[%s] cannot process super params rc %d",
            p_mod->name, rc);
        }
      } else {
        rc = module_imgbase_process_super_params(port, event, TRUE, 0);
        if (IMG_ERROR(rc)) {
        IDBG_ERROR("[%s] cannot process super params rc %d",
          p_mod->name, rc);
        }
      }

      uint32_t apply_frame_id = p_buf_divert->buffer.sequence;
      uint32_t q_idx = 0;


      if (CAM_HAL_V1 == p_mod->hal_version ||
        TRUE == p_client->process_all_frames) {
        process_frame = TRUE;
      } else if (CAM_HAL_V3 == p_mod->hal_version) {
        imgbase_session_data_t *p_session_data;
        p_session_data = IMGBASE_SSP(p_mod,IMGLIB_SESSIONID(event->identity));
        if (p_session_data == NULL) {
         IDBG_ERROR("CAM_INTF_META_STREAM_ID failed, session data null \n");
         return IMG_ERR_INVALID_INPUT;
        }
        q_idx = apply_frame_id % PER_FRAME_QUEUE_SIZE;

        for (i = 0; i < p_session_data->stream_id_requested[q_idx].num_streams; i++) {
          if (p_session_data->stream_id_requested[q_idx].stream_request[i].streamID ==
              IMGLIB_STREAMID(p_client->divert_identity)) {
            process_frame = TRUE;
            p_client->valid_pcr[q_idx] = 1;
            break;
          }
        }

        if (p_mod->feature_mask & CAM_QTI_FEATURE_RTB) {
          /* In HAL3, PCR will be sent for master frame alone
           * for slave frame PCR is added here
           */
          process_frame = is_master ? process_frame : TRUE;
        } else if (p_mod->caps.dual_camfeature && !process_frame) {
          process_frame = TRUE;
        }

        /*
         * For HAL3, SAT module master switch event will be based book-keeping
         * to avoid event timing issue
         */
        if (p_mod->feature_mask & CAM_QTI_FEATURE_SAT) {
            p_mod->modparams.imgbase_handle_master_switch(port,
              p_client, apply_frame_id);
        }
      }
      IDBG_HIGH("[%s] [IMG_BUF_DBG_IN_f%d_b%d_i%x] event identity %x"
        " buf_div str_to_proc %x process_frame %d",
        p_mod->name,
        p_buf_divert->buffer.sequence,
        p_buf_divert->buffer.index,
        p_buf_divert->identity,
        p_client->divert_identity,
        p_client->streams_to_process,
        process_frame);

      rc = module_imglib_common_meta_fd_info_apply_rotation(
        p_mod->caps.face_detect_tilt_cut_off, &p_client->current_meta);
      if (IMG_ERROR(rc)) {
        IDBG_LOW("[%s] cannot apply fd/rotation info rc %d",
          p_mod->name, rc);
        rc = IMG_SUCCESS;
      }

      if (process_frame &&
        ((p_client->processing_mask & p_client->stream_mask) ||
        (!p_client->streams_to_process)) &&
        (!p_client->processing_disabled)) {
        rc = module_imgbase_client_handle_buffer(p_client, p_buf_divert);

        /* indicate that the buffer is consumed */
        if (IMG_SUCCEEDED(rc)) {
          p_buf_divert->is_locked = FALSE;
          p_buf_divert->ack_flag = FALSE;
          fwd_event = FALSE;
        } else if (!(p_client->caps.inplace_algo) ||
            (p_client->caps.will_dim_change)) {
          p_buf_divert->ack_flag = TRUE;
          fwd_event = FALSE;
        }
      } else {
        p_client->p_current_buf_div = p_buf_divert;
        if (p_mod->modparams.imgbase_client_handle_frame_skip)
          p_mod->modparams.imgbase_client_handle_frame_skip(p_client);
        p_buf_divert->is_locked = FALSE;

        /* If the module is a sink module, do a piggy back ack */
       if ((p_mod->module_type == MCT_MODULE_FLAG_SINK) ||
          !p_stream->streamon  || p_client->piggy_back_ack || !process_frame) {
          IDBG_MED("Stream not on acking back %s id %d", p_mod->name,
            p_buf_divert->identity);
          p_buf_divert->ack_flag = TRUE;
          fwd_event = FALSE;
        } else {
          p_buf_divert->ack_flag = FALSE;
        }
        IDBG_HIGH("[%s] [IMG_BUF_DBG_IN_f%d_b%d_i%x] FWD  str_to_proc %x"
          "process_frame %d proc_mask %x stream_mask %x processing disabled %d"
          "streamon %d, ack_flag %d, fwd_event %d",
          p_mod->name,
          p_buf_divert->buffer.sequence,
          p_buf_divert->buffer.index,
          p_buf_divert->identity,
          p_client->streams_to_process,
          process_frame,
          p_client->processing_mask,
          p_client->stream_mask,
          p_client->processing_disabled,
          p_stream->streamon,
          p_buf_divert->ack_flag,
          fwd_event);
      }

      break;
    }
    case MCT_EVENT_MODULE_BUF_DIVERT_ACK: {
      imgbase_stream_t *p_stream;
      isp_buf_divert_ack_t *p_ack =
        (isp_buf_divert_ack_t *)p_mod_event->module_event_data;
      uint32_t out_buffer_access = 0;

      if (p_client->state == IMGLIB_STATE_ERROR ||
        p_client->state == IMGLIB_STATE_INIT) {
        IDBG_HIGH("[%s] Invalid state to handle ACK", p_mod->name);
        break;
      }

      if (p_ack) {

        int stream_idx = module_imgbase_find_stream_by_identity(p_client, p_ack->identity);
        if (stream_idx < 0) {
          IDBG_ERROR("[%s] Cannot find stream mapped to idx %x",p_mod->name,
            p_ack->identity);
          break;
        }

        p_stream = &p_client->stream[stream_idx];

        pthread_mutex_lock(&p_client->mutex);
        IDBG_MED("[%s] Received ACK : frames_pending_for_ack=%d",
          p_mod->name, p_stream->frames_pending_for_ack);
        if (p_stream->frames_pending_for_ack > 0) {
          p_stream->frames_pending_for_ack--;
        }
        pthread_mutex_unlock(&p_client->mutex);

        if (!p_stream->streamon &&
          (p_stream->cur_frame_id <= p_ack->frame_id) &&
          (p_stream->frames_pending_for_ack == 0)) {
          /* override*/
          IDBG_MED("[%s] calling flush done, curr_frame=%d, ack_frame=%d",
            p_mod->name, p_stream->cur_frame_id, p_ack->frame_id);
          if (p_mod->modparams.imgbase_client_flush_done) {
            p_mod->modparams.imgbase_client_flush_done(p_client, p_stream);
          }
        }
        uint32_t buf_mask = IMG_2_MASK(IMGLIB_STREAMID(p_ack->identity));
        IDBG_MED("[%s]: identity %x, buf mask %x",
          p_mod->name, p_ack->identity, buf_mask);
        IDBG_MED("[%s] [IMG_BUF_DBG_%s_f%d_b%d_i%x] buf_ack buf_mask %x",
          p_mod->name,
          (p_mod->caps.num_output) ? "OUT" : "IN",
          p_ack->frame_id,
          p_ack->buf_idx,
          p_ack->identity,
          buf_mask);
        IDBG_HIGH("Received ack for bayerdata = %d", p_ack->bayerdata);

        if (p_mod->caps.num_overlap && !p_client->processing_disabled) {
          rc = module_imgbase_client_handle_buffer_ack(p_client,
            IMG_EVT_ACK_HOLD, p_ack);
          fwd_event = IMG_SUCCEEDED(rc) ? FALSE : TRUE;
        }
        IDBG_MED("[%s]: Internal buf %d, processing_mask %x, buf mask %x",
          p_mod->name, p_mod->caps.use_internal_bufs, p_client->processing_mask,
          buf_mask);
        if (p_mod->caps.use_internal_bufs &&
          (p_client->processing_mask & buf_mask)) {
          rc = module_imgbase_client_handle_buffer_ack(p_client,
            IMG_EVT_ACK_FREE_INTERNAL_BUF, p_ack);
          if (rc == IMG_ERR_NOT_FOUND) {
            fwd_event = TRUE;
          } else {
            fwd_event = FALSE;
          }
          pthread_mutex_lock(&p_client->buf_mutex);
          IDBG_HIGH("Trying recovery pending %d",
            p_client->output_hold_q_buf_pending_count);
          if (p_client->output_hold_q_buf_pending_count) {
            if (p_mod->caps.num_hold_outbuf) {
              img_frame_t *p_out_frame = NULL;
              img_component_ops_t *p_comp = &p_client->comp;
              p_out_frame = img_q_dequeue(&p_client->output_hold_q);
              if (!p_out_frame) {
                p_client->cur_outbuf_cnt--;
                IDBG_LOW(" cur_outbuf_cnt %d", p_client->cur_outbuf_cnt);
                pthread_mutex_unlock(&p_client->buf_mutex);
                } else {
                pthread_mutex_unlock(&p_client->buf_mutex);
                rc = module_imgbase_client_get_outputbuf(p_client, p_out_frame,
                  p_mod->caps.use_internal_bufs, true, &out_buffer_access);
                if (IMG_ERROR(rc)) {
                   pthread_mutex_lock(&p_client->buf_mutex);
                   p_client->cur_outbuf_cnt--;
                   IDBG_LOW(" cur_outbuf_cnt %d", p_client->cur_outbuf_cnt);
                   pthread_mutex_unlock(&p_client->buf_mutex);
                   if (p_out_frame->private_data) {
                     free(p_out_frame->private_data);
                   }
                } else {
                  if (p_mod->modparams.access_mode == IMG_ACCESS_READ_WRITE) {
                    // This module writes into output frames (not read-only module like paaf)
                    // Handle output buffer cache operations.
                    img_common_handle_output_frame_cache_op(p_mod->modparams.exec_mode,
                      p_out_frame, p_mod->ion_fd,
                      (p_mod->modparams.force_cache_op | p_mod->force_cache_op),
                      &out_buffer_access);
                  }
                  ((imgbase_buf_t *)(p_out_frame->private_data))->buf_divert
                    .buffer_access = out_buffer_access;

                  /* queue the buffer */
                  rc = IMG_COMP_Q_BUF(p_comp, p_out_frame, IMG_OUT);
                  if (IMG_SUCCEEDED(rc)) {
                    p_client->output_hold_q_buf_pending_count--;
                    IDBG_ERROR("Recovery Success! frame id %d pending %d",
                      p_out_frame->frame_id,
                      p_client->output_hold_q_buf_pending_count);
                  } else {
                    IDBG_ERROR("%s:%d] Error, failed to qbuf %d",
                      __func__, __LINE__, rc);
                    pthread_mutex_lock(&p_client->buf_mutex);
                    p_client->cur_outbuf_cnt--;
                    IDBG_LOW(" cur_outbuf_cnt %d", p_client->cur_outbuf_cnt);
                    pthread_mutex_unlock(&p_client->buf_mutex);
                    if (p_out_frame->private_data) {
                      free(p_out_frame->private_data);
                    }
                  }
                 }
               }
            }else {
              pthread_mutex_unlock(&p_client->buf_mutex);
            }
          }else {
            pthread_mutex_unlock(&p_client->buf_mutex);
          }
        }
      }
      break;
    }
    case MCT_EVENT_MODULE_EXTENDED_FACE_INFO: {
      rc = module_imglib_common_meta_set_fd_info(p_mod_event->module_event_data,
        &p_client->current_meta);

      break;
    }
    case MCT_EVENT_MODULE_STATS_DIS_UPDATE: {
      is_update_t *is_update =
        (is_update_t *) p_mod_event->module_event_data;

      if (is_update) {
        p_client->is_update = *is_update;
        p_client->is_update_valid = TRUE;
      }

      break;
    }
    case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
      sensor_out_info_t *p_sensor_info =
        (sensor_out_info_t *)p_mod_event->module_event_data;

      if (p_sensor_info) {
        p_session_data->sensor_max_dim.width =
          p_sensor_info->dim_output.width;
        p_session_data->sensor_max_dim.height =
          p_sensor_info->dim_output.height;
        p_session_data->sensor_crop.first_line =
          p_sensor_info->request_crop.first_line;
        p_session_data->sensor_crop.last_line =
          p_sensor_info->request_crop.last_line;
        p_session_data->sensor_crop.first_pixel =
          p_sensor_info->request_crop.first_pixel;
        p_session_data->sensor_crop.last_pixel =
          p_sensor_info->request_crop.last_pixel;
      }
      break;
    }
    case MCT_EVENT_MODULE_STREAM_CROP: {
      mct_bus_msg_stream_crop_t *stream_crop =
        (mct_bus_msg_stream_crop_t *) p_mod_event->module_event_data;

      if (stream_crop) {
        p_client->stream_crop = *stream_crop;
        p_client->stream_crop_valid = TRUE;
        IDBG_MED("[%s] MCT_EVENT_MODULE_STREAM_CROP o %dX%d m_x%d, m_y%d",
          p_mod->name,
          p_client->stream_crop.crop_out_x,
          p_client->stream_crop.crop_out_y,
          p_client->stream_crop.x_map,
          p_client->stream_crop.y_map);
      }

      break;
    }
    case MCT_EVENT_MODULE_ISP_OUTPUT_DIM: {
      mct_stream_info_t *stream_info =
        (mct_stream_info_t *)(p_mod_event->module_event_data);

      if (stream_info) {
        p_client->isp_output_dim_stream_info = *stream_info;
        p_client->isp_output_dim_stream_info_valid = TRUE;
        p_stream->isp_stream_info = *stream_info;

        IDBG_MED("[%s] MCT_EVENT_MODULE_ISP_OUTPUT_DIM %x, %dX%d",
          p_mod->name, event->identity,
          stream_info->dim.width,
          stream_info->dim.height);
      }
      break;
    }
    case MCT_EVENT_MODULE_IFACE_REQUEST_PP_DIVERT: {
      pp_buf_divert_request_t *divert_request =
        (pp_buf_divert_request_t *)(p_mod_event->module_event_data);

      if (divert_request) {
        IDBG_MED("[%s] Request extra native buffers %d, buffer alignment %d",
          p_mod->name, p_client->isp_extra_native_buf,
          p_client->isp_native_buf_alignment);

        //update client specific buffer alignments if any
        divert_request->buf_alignment = mct_util_calculate_lcm(
          divert_request->buf_alignment, p_client->isp_native_buf_alignment);

        //update if any additional native buffers required
        divert_request->num_additional_buffers +=
          p_client->isp_extra_native_buf;

        //If module is after cpp and has its own output buffer, CPP needs to
        // Allocate native buffers
        divert_request->need_divert =
          (!p_client->before_cpp && p_mod->caps.num_output)? 1: 0;
      }
      break;
    }
    case MCT_EVENT_MODULE_QUERY_DIVERT_TYPE: {
      if (!p_client->before_cpp) {
        uint32_t *divert_mask = (uint32_t *)p_mod_event->module_event_data;
        *divert_mask |= PPROC_DIVERT_PROCESSED;
        p_client->divert_mask = *divert_mask;
      }
      break;
    }
    case MCT_EVENT_MODULE_SET_META_PTR:
      if (CAM_STREAM_TYPE_OFFLINE_PROC == p_stream->stream_info->stream_type) {
        p_client->hal_meta_data_list = mct_list_append(
          p_client->hal_meta_data_list, p_mod_event->module_event_data, NULL,
          NULL);
      }
      break;
    case MCT_EVENT_MODULE_STATS_AEC_UPDATE:
      if (CAM_STREAM_TYPE_OFFLINE_PROC == p_stream->stream_info->stream_type) {
        rc = module_imglib_common_meta_set_aec(&p_client->meta_data_list,
          p_mod_event->module_event_data, p_mod_event->current_frame_id);
      }
      rc = module_imglib_common_meta_set_aec_info(
        p_mod_event->module_event_data, &p_session_data->session_meta);
      if (rc) {
        IDBG_WARN("Failed in set aec meta");
      }
      break;
    case MCT_EVENT_MODULE_ISP_AWB_UPDATE:
      if (CAM_STREAM_TYPE_OFFLINE_PROC == p_stream->stream_info->stream_type) {
        rc = module_imglib_common_meta_set_awb(&p_client->meta_data_list,
          p_mod_event->module_event_data, p_mod_event->current_frame_id);
      }
      break;
    case MCT_EVENT_MODULE_STATS_AWB_UPDATE:
      rc = module_imglib_common_meta_set_awb_info(
        p_mod_event->module_event_data, &p_session_data->session_meta);
      if (rc) {
        IDBG_WARN("Failed in set awb meta");
      }
      break;
    case MCT_EVENT_MODULE_ISP_GAMMA_UPDATE:
      rc = module_imglib_common_meta_set_gamma_info(
        p_mod_event->module_event_data, &p_session_data->session_meta);
      if (rc) {
        IDBG_WARN("Failed in set gamma meta");
      }
      break;
    case MCT_EVENT_MODULE_FRAME_SKIP_NOTIFY: {
      uint32_t skip_frame_id =
        *(uint32_t *)event->u.module_event.module_event_data;
      mct_queue_traverse(&p_session_data->frame_config_q,
        module_imgbase_handle_frame_skip, &skip_frame_id);
      break;
    }
    case MCT_EVENT_MODULE_SENSOR_QUERY_OUTPUT_SIZE: {
      sensor_isp_stream_sizes_t *p_isp_stream_sizes =
        (sensor_isp_stream_sizes_t *)(p_mod_event->module_event_data);
      if (p_mod->modparams.imgbase_client_sensor_query_out_size) {
        rc = p_mod->modparams.imgbase_client_sensor_query_out_size(p_client,
          event, p_isp_stream_sizes, &fwd_event);
        if (IMG_ERROR(rc)) {
          IDBG_ERROR("Sensor query output size failed");
        }
      }
      break;
    }
    case CAM_DUAL_CAMERA_LOW_POWER_MODE: {
      cam_dual_camera_perf_control_t *dual_cam_lpm = p_mod_event->module_event_data;
      if(dual_cam_lpm == NULL) {
        IDBG_ERROR("Invalid input for Dual Cam LPM Info");
        break;
      }
      p_session_data->multi_cam_info.low_power_mode = dual_cam_lpm->enable;
      break;
    }
    case CAM_DUAL_CAMERA_MASTER_INFO: {
      cam_dual_camera_master_info_t *dual_master_info = p_mod_event->module_event_data;
      if(dual_master_info == NULL) {
        IDBG_ERROR("Invalid input for DUAL CAM MASTER Info");
        break;
      }
      p_session_data->multi_cam_info.cam_mode = dual_master_info->mode;
      break;
    }
    default:
      break;
    }
    break;
  }
  default:
   break;
  }

  if (fwd_event) {
    boolean brc = module_imgbase_forward_port_event(p_client, port, event);
    rc = (brc) ? IMG_SUCCESS : IMG_ERR_GENERAL;
  }

  return GET_STATUS(rc);
}

/**
 * Function:  module_imgbase_port_intra_event_func:
 *
 * Description: Intra_event handler function for the imgbase port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: mct event
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_imgbase_port_intra_event_func(
  mct_port_t *port, mct_event_t *event)
{
  int rc = IMG_SUCCESS;
  mct_module_t *p_mct_mod = NULL;
  module_imgbase_t *p_mod = NULL;
  imgbase_client_t *p_client;
  img_core_ops_t *p_core_ops = NULL;

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d] invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_core_ops = &p_mod->core_ops;
  if (NULL == p_core_ops) {
    IDBG_ERROR("%s:%d] p_core_ops NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (imgbase_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s_%s:%d] imgbase client NULL", __func__, p_mod->name,
      __LINE__);
    return FALSE;
  }

  IDBG_LOW("%s_%s:%d] type %d", __func__, p_mod->name, __LINE__, event->type);

  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    break;
  }

  case MCT_EVENT_MODULE_EVENT: {
    mct_event_module_t *p_mod_event = &event->u.module_event;
    IDBG_MED("%s_%s:%d] Mod type %d", __func__, p_mod->name, __LINE__,
      p_mod_event->type);

    if (p_mod_event->type < MCT_EVENT_MODULE_MAX) {
      boolean is_evt_handled = TRUE;
      if (p_mod->modparams.imgbase_handle_module_event[p_mod_event->type]) {
        p_mod->modparams.imgbase_handle_module_event[p_mod_event->type](
        event->identity, p_mod_event, p_client, p_core_ops, &is_evt_handled);
      }
    }
  }

  default:
    break;
  }

  return GET_STATUS(rc);
}

/**
 * Function: module_imgbase_is_stream_compatible
 *
 * Description: This method is used to check if the stream is
 * compatible with the port.
 *
 * Arguments:
 *   @stream_info: Stream requesting port
 *   @p_client: Client associated with the port
 *   @port: mct port pointer
 *
 * Return values:
 *     TRUE if compatible
 *     FALSE otherwise
 *
 * Notes: none
 **/
static boolean module_imgbase_is_stream_compatible(
  mct_stream_info_t *stream_info, imgbase_client_t *p_client, mct_port_t *port)
{
  uint32_t i;
  uint32_t port_sess_id;
  uint32_t is_compatible;
  imgbase_client_t *p_temp_client = port->port_private;

  if (!p_temp_client) {
    IDBG_ERROR("%s:%d] cannot create temp client", __func__, __LINE__);
    return FALSE;
  }

  /* check if port is already connected in a different session */
  for (i = 0; i < p_temp_client->stream_cnt; i++) {
    port_sess_id = IMGLIB_SESSIONID(p_temp_client->stream[i].identity);
    if (port_sess_id != IMGLIB_SESSIONID(stream_info->identity)) {
      IDBG_HIGH("%s %d]: Port already connected on sessionid %d, "
        "Current sessionid %d", __func__, __LINE__, port_sess_id,
        IMGLIB_SESSIONID(stream_info->identity));
      return FALSE;
    }
  }

  /* Check if stream mask includes current stream, ie if the
    client has mapped the current stream type to its sink port */
  is_compatible = p_temp_client->stream_mask &
    (1 << IMGLIB_STREAMID(stream_info->identity));
  IDBG_MED("%s:%d] found client %p, stream mask %x. curr type %d, "
    "idx %x, compatible %d", __func__, __LINE__, p_temp_client,
    p_temp_client->stream_mask, stream_info->stream_type,
    stream_info->identity, is_compatible);

  return is_compatible;
}

/**
 * Function: module_imgbase_fill_stream
 *
 * Description: This function is used to fill the stream details
 *
 * Arguments:
 *   @p_client: imgbase client
 *   @stream_info: pointer to stream info
 *   @port: port
 *
 * Return values:
 *     error values
 *
 * Notes: none
 **/
int module_imgbase_fill_stream(imgbase_client_t *p_client,
  mct_stream_info_t *stream_info,
  mct_port_t *port)
{
  imgbase_stream_t *p_stream;
  int32_t status = IMG_ERR_NOT_FOUND;
  module_imgbase_t *p_mod;

  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] Error invalid client",
      __func__, __LINE__);
    return IMG_ERR_OUT_OF_BOUNDS;
  }
  p_mod = (module_imgbase_t *)p_client->p_mod;
  IDBG_MED("%s] Stream %d, port %s", p_mod->name,
    stream_info->stream_type, MCT_PORT_NAME(port));
  if (p_client->stream_cnt >= MAX_IMGLIB_BASE_MAX_STREAM) {
    IDBG_ERROR("%s_%s:%d] Error max ports reached",
      __func__, p_mod->name, __LINE__);
    return IMG_ERR_OUT_OF_BOUNDS;
  }

  if (MCT_PORT_IS_SINK(port)) {
    IDBG_MED("%s]: Port %s is sink port", p_mod->name,
      MCT_PORT_NAME(port));
    p_stream = &p_client->stream[p_client->stream_cnt];
    p_stream->orig_stream_info = stream_info;

    /* copy stream info */
    p_stream->stream_info = NULL;
    p_stream->stream_info =
      (mct_stream_info_t *) calloc (1, sizeof(mct_stream_info_t));
    if (p_stream->stream_info == NULL) {
      IDBG_ERROR("cannot alloc mem for Out of memory");
      return IMG_ERR_NO_MEMORY;
    }
    memcpy(p_stream->stream_info, stream_info, sizeof(mct_stream_info_t));
    p_stream->identity = stream_info->identity;
    p_stream->p_sinkport = port;
    status = IMG_SUCCESS;
    p_stream->p_srcport = NULL;
    p_client->stream_cnt++;

    cam_pp_feature_config_t *p_config = NULL;
    p_config = &p_stream->stream_info->reprocess_config.pp_feature_config;
    if (p_config->burst_cnt > 0) {
       p_mod->caps.num_input = p_config->burst_cnt;
       p_client->caps.num_input = p_config->burst_cnt;
       p_client->caps.num_output = p_mod->caps.num_output;
       IDBG_MED("%s:%d] pp_config burst count %d", __func__,
         __LINE__, p_config->burst_cnt);
    }

    /* If module is only enabled on a particular stream, check if
       current stream is compatible */
    if (p_client->output_stream_mask & (1 << stream_info->stream_type)) {
      p_client->output_identity = stream_info->identity;
    }
    IDBG_MED("processing mask %x", p_client->processing_mask);
  } else { /* src port */
    IDBG_MED("%s_%s %d]: Port %s is src port",  __func__,  p_mod->name,
      __LINE__, MCT_PORT_NAME(port));
    uint32_t i = 0;
    for (i = 0; i < p_client->stream_cnt; i++) {
      IDBG_MED("%s_%s:%d] Src %x %x", __func__, p_mod->name, __LINE__,
        p_client->stream[i].identity, stream_info->identity);
      if (p_client->stream[i].identity ==
        stream_info->identity) {
        p_client->stream[i].p_srcport = port;
        status = IMG_SUCCESS;
        break;
      }
    }
  }

  return status;
}

/**
 * Function: module_imgbase_port_acquire
 *
 * Description: This function is used to acquire the port
 *
 * Arguments:
 *   @p_mct_mod: mct module pointer
 *   @port: mct port pointer
 *   @stream_info: stream information
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_imgbase_port_acquire(mct_module_t *p_mct_mod,
  mct_port_t *port,
  mct_stream_info_t *stream_info)
{
  int rc = IMG_SUCCESS;
  unsigned int p_identity;
  mct_list_t *p_temp_list = NULL;
  imgbase_client_t *p_client = NULL;
  module_imgbase_t *p_mod = NULL;
  boolean is_compatible = TRUE;
  uint32_t imglib_streamid;
  boolean new_client_flag = TRUE;

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return FALSE;
  }
  IDBG_MED("%s_%s:%d] E", __func__, p_mod->name, __LINE__);
  p_identity =  stream_info->identity;

  /* check if its sink port*/
  if (MCT_PORT_IS_SINK(port)) {
    if (p_mod->caps.share_client_per_session) {
      /* create imgbase client only for new sessions */
      mct_list_t *p_temp_list = mct_list_find_custom(p_mod->imgbase_client,
        &stream_info->identity,
        module_imgbase_find_client_by_session);
      IDBG_MED("%s_%s:%d] client entry %p", __func__, p_mod->name, __LINE__,
        p_temp_list);
      if (NULL != p_temp_list) {
        port->port_private = p_temp_list->data;
        if (((imgbase_client_t *)(port->port_private))->p_mod) {
          IDBG_MED("%s:%d] No need to create new client, use client %p",
            __func__, __LINE__, port->port_private);
          new_client_flag = FALSE;
        }
      }
    }
    if (TRUE == new_client_flag) {
      IDBG_LOW("%s_%s:%d] create imgbase client for %s", __func__, p_mod->name,
        __LINE__, MCT_PORT_NAME(port));
      /* create imgbase client */
      rc = module_imgbase_client_create(p_mct_mod, port,
        IMGLIB_SESSIONID(stream_info->identity),
        stream_info);
      if (IMG_SUCCEEDED(rc)) {
        p_client = port->port_private;
        if (p_mod->caps.share_client_per_session) {
          /* add the client to the list */
          p_temp_list = mct_list_append(p_mod->imgbase_client, p_client,
            NULL, NULL);
          if (NULL == p_temp_list) {
            IDBG_ERROR("%s:%d] list append failed", __func__, __LINE__);
            rc = IMG_ERR_GENERAL;
            goto error;
          }
          p_mod->imgbase_client = p_temp_list;
          p_mod->imgbase_client_cnt++;
        }
      }
    }
    p_client = port->port_private;
    imglib_streamid = IMGLIB_STREAMID(stream_info->identity);
    rc = module_imgbase_add_stream_to_mask(&p_client->stream_mask,
      imglib_streamid);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s_%s:%d] Error", __func__, p_mod->name, __LINE__);
      goto error;
    }
    IDBG_MED("%s:%d] client %p, stream mask %x, type %d, idx %x",
      __func__, __LINE__, p_client, p_client->stream_mask,
      stream_info->stream_type, stream_info->identity);

  } else {
    /*The port may already be used by another stream.
        Check if current stream is compatible*/
    if (port->object.refcount > 0) {
      is_compatible =
        module_imgbase_is_stream_compatible(stream_info, p_client, port);
    }
    if (is_compatible) {
        /* update the internal connection with source port */
        p_temp_list = mct_list_find_custom(p_mod->imgbase_client, &p_identity,
          module_imgbase_find_client);
        if (NULL != p_temp_list) {
          p_client = p_temp_list->data;
          port->port_private = p_client;
          IDBG_MED("%s_%s:%d] found client %p", __func__, p_mod->name, __LINE__,
            p_client);
        } else {
          IDBG_ERROR("%s_%s:%d] cannot find the client", __func__, p_mod->name,
            __LINE__);
          goto error;
        }
    } else {
      IDBG_MED("[%s] Port already reserved", p_mod->name);
      goto error;
    }
  }

  if (IMG_SUCCEEDED(rc)) {
    rc = module_imgbase_fill_stream(p_client, stream_info, port);
  }

  IDBG_MED("%s_%s:%d] port %p rc %dX", __func__, p_mod->name, __LINE__, port, rc);
  return GET_STATUS(rc);

error:

  IDBG_MED("%s_%s:%d] Error X", __func__, p_mod->name, __LINE__);
  return FALSE;

}

/**
 * Function: module_imgbase_port_check_caps_reserve
 *
 * Description: This function is used to reserve the port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @peer_caps: pointer to peer capabilities
 *   @stream_info: stream information
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_imgbase_port_check_caps_reserve(mct_port_t *port,
  void *caps,
  void *vstream_info)
{
  boolean rc = FALSE;
  mct_port_caps_t *peer_caps = caps;
  mct_module_t *p_mct_mod = NULL;
  module_imgbase_t *p_mod = NULL;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)vstream_info;
  int ret = IMG_SUCCESS;
  uint32_t imglib_streamid;
  imgbase_client_t *p_client = NULL;

  if (!port || !stream_info) {
    CDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  if (peer_caps && (peer_caps->port_caps_type != port->caps.port_caps_type)) {
    CDBG_ERROR("%s:%d peer caps missmatch", __func__, __LINE__);
    return FALSE;
  }

  IDBG_MED("%s:%d] E %s %d", __func__, __LINE__, MCT_PORT_NAME(port),
    stream_info->stream_type);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    CDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return FALSE;
  }

  /* lock the module */
  pthread_mutex_lock(&p_mod->mutex);
  int ref_count = MCT_OBJECT_REFCOUNT(port);

  if (port->port_private && (0 == ref_count)) {
    /* port is already reserved */
    IDBG_MED("%s_%s:%d] Error port is already reserved %d p_client %p",
      __func__, p_mod->name, __LINE__, ref_count, port->port_private);
    pthread_mutex_unlock(&p_mod->mutex);
    return FALSE;
  }

  IDBG_MED("%s_%s:%d] port %s ref_count %d port_private %p", __func__, p_mod->name,
    __LINE__, MCT_PORT_NAME(port), ref_count, port->port_private);
  if ((ref_count > 0) && MCT_PORT_IS_SINK(port) && port->port_private) {
    IDBG_LOW("%s_%s:%d] port %s is sink, ref_count %d", __func__, p_mod->name,
      __LINE__, MCT_PORT_NAME(port), ref_count);
    if (!p_mod->caps.disable_shared_port) {
       imgbase_client_t *p_client = port->port_private;
       ret = module_imgbase_fill_stream(p_client, stream_info, port);

       if (IMG_SUCCEEDED(ret)) {
         imglib_streamid = IMGLIB_STREAMID(stream_info->identity);
         rc = module_imgbase_add_stream_to_mask(&p_client->stream_mask,
           imglib_streamid);
         if (IMG_ERROR(rc)) {
           IDBG_ERROR("%s_%s:%d] Error", __func__, p_mod->name, __LINE__);
           pthread_mutex_unlock(&p_mod->mutex);
           return FALSE;
         }
         IDBG_MED("%s] stream mask %x", p_mod->name, p_client->stream_mask);
         rc = TRUE;
       }
    } else {
       IDBG_ERROR("%s_%s:%d] Error port is already reserved and shared port is disabled"
         "%d p_client %p, disabled_shared_port %d",
         __func__, p_mod->name, __LINE__, ref_count, port->port_private,
         p_mod->caps.disable_shared_port);
       pthread_mutex_unlock(&p_mod->mutex);
       return FALSE;
    }
  } else {
    IDBG_LOW("%s_%s:%d] acquire port for %s", __func__, p_mod->name,
      __LINE__, MCT_PORT_NAME(port));
    rc = module_imgbase_port_acquire(p_mct_mod, port, stream_info);
    if (FALSE == rc) {
      IDBG_MED("[%s] Error acquiring port %d", p_mod->name, ref_count);
      pthread_mutex_unlock(&p_mod->mutex);
      return FALSE;
    }
  }

  p_client = (imgbase_client_t *)port->port_private;
  IDBG_MED("%s_%s:%d] client ref_count %d p_client %p", __func__, p_mod->name,
      __LINE__, p_client->stream_cnt, p_client);

  MCT_OBJECT_REFCOUNT(port)++;
  IDBG_MED("%s_%s:%d] count %d X", __func__, p_mod->name, __LINE__,
    MCT_OBJECT_REFCOUNT(port));
  pthread_mutex_unlock(&p_mod->mutex);
  return TRUE;

}

/**
 * Function: module_imgbase_port_release_client
 *
 * Description: This method is used to release the client after all the
 *                 ports are destroyed
 *
 * Arguments:
 *   @p_mod: pointer to the imgbase module
 *   @identity: stream/session id
 *   @p_client: imgbase client
 *   @identity: port identity
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_imgbase_port_release_client(module_imgbase_t *p_mod,
  mct_port_t *port,
  imgbase_client_t *p_client,
  unsigned int identity)
{
  IMG_UNUSED(port);

  mct_list_t *p_temp_list = NULL;
  p_temp_list = mct_list_find_custom(p_mod->imgbase_client, &identity,
    module_imgbase_find_client);
  if (NULL != p_temp_list) {
    IDBG_MED("%s_%s:%d] ", __func__, p_mod->name, __LINE__);
    p_mod->imgbase_client = mct_list_remove(p_mod->imgbase_client,
      p_temp_list->data);
  }
  module_imgbase_client_destroy(p_client);
}

/**
 * Function: module_imgbase_port_check_caps_unreserve
 *
 * Description: This method is used to unreserve the port
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_imgbase_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  int rc = IMG_SUCCESS;
  mct_module_t *p_mct_mod = NULL;
  module_imgbase_t *p_mod = NULL;
  imgbase_client_t *p_client = NULL;
  uint32_t imglib_streamid;
  mct_list_t *p_temp_list = NULL;
  int idx;
  uint32_t i;
  boolean release_client_flag = TRUE;

  if (!port) {
    IDBG_ERROR("%s:%d invalid input ", __func__, __LINE__);
    return FALSE;
  }

  IDBG_MED("%s:%d] E %p", __func__, __LINE__, port);

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (imgbase_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s_%s:%d] imgbase client NULL", __func__, p_mod->name, __LINE__);
    return FALSE;
  }

  /* lock the module */
  pthread_mutex_lock(&p_mod->mutex);
  MCT_OBJECT_REFCOUNT(port)--;

  if (MCT_PORT_IS_SRC(port)) {
    IDBG_LOW("%s:%d] src port %s, ref %d", __func__, __LINE__,
      MCT_PORT_NAME(port), MCT_OBJECT_REFCOUNT(port));
    if (MCT_OBJECT_REFCOUNT(port) == 0) {
      port->port_private = NULL;
    }
  } else {
    IDBG_LOW("%s:%d] sink port %s, ref %d", __func__, __LINE__,
      MCT_PORT_NAME(port), port->object.refcount);

    /* remove stream from stream mask */
    imglib_streamid = IMGLIB_STREAMID(identity);
    p_client->stream_mask &= ~(1 << imglib_streamid);

    IDBG_MED("%s:%d] sink refcount %d, stream mask %x",
      __func__, __LINE__, MCT_OBJECT_REFCOUNT(port), p_client->stream_mask);

    idx = module_imgbase_find_stream_by_identity(p_client, identity);

    if (p_mod->caps.share_client_per_session) {
      IDBG_MED("%s:%d] stream cnt %d", __func__, __LINE__,p_client->stream_cnt);
      if (p_client->stream_cnt <= 1) {
        /* First remove client from module list */
        p_temp_list = mct_list_find_custom(p_mod->imgbase_client, &identity,
          module_imgbase_find_client_by_session);
        IDBG_MED("%s:%d] p_temp_list %p", __func__, __LINE__, p_temp_list);
        if (NULL != p_temp_list) {
          p_mod->imgbase_client = mct_list_remove(p_mod->imgbase_client,
            p_temp_list->data);
          /* Update module client count */
          if (p_mod->imgbase_client_cnt) {
            p_mod->imgbase_client_cnt--;
            if (!p_mod->imgbase_client_cnt) {
              mct_list_free_list(p_mod->imgbase_client);
              p_mod->imgbase_client = NULL;
            }
          } else {
            IDBG_ERROR("%s:%d] Invalid client count", __func__, __LINE__);
          }
          IDBG_MED("%s:%d] Module client count %d", __func__, __LINE__,
            p_mod->imgbase_client_cnt);
        }
      } else {
        release_client_flag = FALSE;
      }
      if (idx >= 0) {
        for (i = idx; i < p_client->stream_cnt; i++) {
          p_client->stream[i].identity = p_client->stream[i + 1].identity;
          p_client->stream[i].p_sinkport = p_client->stream[i + 1].p_sinkport;
          p_client->stream[i].p_srcport = p_client->stream[i + 1].p_srcport;
          p_client->stream[i].stream_info =
            p_client->stream[i + 1].stream_info;
        }
        p_client->stream_cnt--;
      } else {
        IDBG_HIGH("%s_%s:%d] Cannot find the stream", __func__, p_mod->name,
          __LINE__);
      }
    }

    if (p_client->stream[idx].stream_info != NULL) {
      free(p_client->stream[idx].stream_info);
    }

    if (TRUE == release_client_flag) {
      if (port->object.refcount == 0) {
        module_imgbase_port_release_client(p_mod, port, p_client,
          identity);
      }
    }
  }
  if (MCT_OBJECT_REFCOUNT(port) == 0) {
    port->port_private = NULL;
    MCT_PORT_PEER(port) = NULL;
  }
  pthread_mutex_unlock(&p_mod->mutex);

  /*Todo: free port??*/
  IDBG_MED("%s_%s:%d] count %d X", __func__, p_mod->name, __LINE__,
    MCT_OBJECT_REFCOUNT(port));
  return GET_STATUS(rc);
}

/**
 * Function: module_imgbase_port_ext_link
 *
 * Description: This method is called when the user establishes
 *              link.
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_imgbase_port_ext_link(unsigned int identity,
  mct_port_t* port, mct_port_t *peer)
{
  IMG_UNUSED(identity);

  mct_module_t *p_mct_mod = NULL;
  module_imgbase_t *p_mod = NULL;
  imgbase_client_t *p_client = NULL;

  if (!port || !peer) {
    IDBG_ERROR("%s:%d invalid input, port=%p, peer=%p",
      __func__, __LINE__, port, peer);
    return FALSE;
  }

  IDBG_MED("%s:%d] port %p E", __func__, __LINE__, port);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (imgbase_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s_%s:%d] invalid client", __func__, p_mod->name, __LINE__);
    return FALSE;
  }

  if (MCT_PORT_PEER(port)) {
    IDBG_MED("[%s] link already established", p_mod->name);
    return TRUE;
  }

  MCT_PORT_PEER(port) = peer;

  /* check if its sink port*/
  if (MCT_PORT_IS_SINK(port)) {
    /* start imgbase client in case of dynamic module */
  } else {
    /* do nothing for source port */
  }

  mct_port_add_child(identity, port);
  IDBG_MED("%s_%s:%d] add identity %x session %d stream %d port %p ",
    __func__, p_mod->name, __LINE__, identity,
    IMGLIB_SESSIONID(identity), IMGLIB_STREAMID(identity), port);

  IDBG_MED("%s_%s:%d] X", __func__, p_mod->name, __LINE__);
  return TRUE;
}

/**
 * Function: module_imgbase_port_unlink
 *
 * Description: This method is called when the user disconnects
 *              the link.
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_imgbase_port_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  IMG_UNUSED(identity);

  mct_module_t *p_mct_mod = NULL;
  module_imgbase_t *p_mod = NULL;
  imgbase_client_t *p_client = NULL;

  if (!port || !peer) {
    IDBG_ERROR("%s:%d invalid input, port=%p, peer=%p",
      __func__, __LINE__, port, peer);
    return;
  }

  IDBG_MED("%s:%d] E %p", __func__, __LINE__, port);

  if (MCT_PORT_PEER(port) != peer) {
    IDBG_ERROR("%s:%d invalid peer %p %p", __func__, __LINE__,
      peer, MCT_PORT_PEER(port));
    return;
  }

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return;
  }

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return;
  }

  p_client = (imgbase_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] imgbase client NULL", __func__, __LINE__);
    return;
  }

  if (MCT_PORT_IS_SINK(port)) {
    /* stop the client in case of dynamic module */
  } else {
    /* do nothing for source port*/
  }

  mct_port_remove_child(identity, port);

  IDBG_MED("%s_%s:%d] X", __func__, p_mod->name, __LINE__);
  return;
}

/**
 * Function: module_imgbase_port_set_caps
 *
 * Description: This method is used to set the capabilities
 *
 * Arguments:
 *   @port: mct port pointer
 *   @caps: mct port capabilities
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_imgbase_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  IMG_UNUSED(port);
  IMG_UNUSED(caps);

  return TRUE;
}

/**
 * Function: module_imgbase_free_port
 *
 * Description: This function is used to free the imgbase ports
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static boolean module_imgbase_free_port(void *data, void *user_data)
{
  mct_port_t *p_port = (mct_port_t *)data;
  mct_module_t *p_mct_mod = (mct_module_t *)user_data;
  boolean rc = FALSE;

  if (!p_port || !p_mct_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return TRUE;
  }
  IDBG_MED("%s:%d port %p p_mct_mod %p", __func__, __LINE__, p_port,
    p_mct_mod);

  rc = mct_module_remove_port(p_mct_mod, p_port);
  if (rc == FALSE) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
  }
  mct_port_destroy(p_port);
  return TRUE;
}

/**
 * Function: module_imgbase_create_port
 *
 * Description: This function is used to create a port and link with the
 *              module
 *
 * Arguments:
 *   none
 *
 * Return values:
 *     MCTL port pointer
 *
 * Notes: none
 **/
mct_port_t *module_imgbase_create_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir)
{
  char portname[PORT_NAME_LEN];
  mct_port_t *p_port = NULL;
  int status = IMG_SUCCESS;
  int index = 0;

  if (!p_mct_mod || (MCT_PORT_UNKNOWN == dir)) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return NULL;
  }

  index = (MCT_PORT_SINK == dir) ? p_mct_mod->numsinkports :
    p_mct_mod->numsrcports;
  /*portname <mod_name>_direction_portIndex*/
  snprintf(portname, sizeof(portname), "%s_d%d_i%d",
    MCT_MODULE_NAME(p_mct_mod), dir, index);
  p_port = mct_port_create(portname);
  if (NULL == p_port) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return NULL;
  }
  IDBG_MED("%s:%d portname %s", __func__,  __LINE__, portname);

  p_port->direction = dir;
  p_port->port_private = NULL;
  p_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;

  /* override the function pointers */
  p_port->check_caps_reserve    = module_imgbase_port_check_caps_reserve;
  p_port->check_caps_unreserve  = module_imgbase_port_check_caps_unreserve;
  p_port->ext_link              = module_imgbase_port_ext_link;
  p_port->un_link               = module_imgbase_port_unlink;
  p_port->set_caps              = module_imgbase_port_set_caps;
  p_port->event_func            = module_imgbase_port_event_func;
  p_port->intra_event_func      = module_imgbase_port_intra_event_func;
   /* add port to the module */
  if (!mct_module_add_port(p_mct_mod, p_port)) {
    IDBG_ERROR("%s:%d] add port failed", __func__, __LINE__);
    status = IMG_ERR_GENERAL;
    goto error;
  }

  if (MCT_PORT_SRC == dir)
    p_mct_mod->numsrcports++;
  else
    p_mct_mod->numsinkports++;

  IDBG_MED("%s:%d ", __func__, __LINE__);
  return p_port;

error:

  IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
  if (p_port) {
    mct_port_destroy(p_port);
    p_port = NULL;
  }
  return NULL;
}

/**
 * Function: module_imgbase_deinit
 *
 * Description: This function is used to free the imgbase module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_imgbase_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_t *p_mod = NULL;
  img_core_ops_t *p_core_ops = NULL;
  mct_list_t* p_list;
  uint32_t i = 0;
  imgbase_session_data_t *p_session_data;

  if (NULL == p_mct_mod) {
    IDBG_ERROR("%s:%d] MCTL module NULL", __func__, __LINE__);
    return;
  }

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return;
  }

  do {
    p_list = mct_list_find_custom(MCT_MODULE_SINKPORTS(p_mct_mod), p_mct_mod,
      module_imglib_get_next_from_list);
    if (p_list)
      module_imgbase_free_port(p_list->data, p_mct_mod);
  } while (p_list);

  do {
    p_list = mct_list_find_custom(MCT_MODULE_SRCPORTS(p_mct_mod), p_mct_mod,
      module_imglib_get_next_from_list);
    if (p_list)
      module_imgbase_free_port(p_list->data, p_mct_mod);
  } while (p_list);

  if (p_mod->comp_ops.handle)
    free(p_mod->comp_ops.handle);

  p_core_ops = &p_mod->core_ops;
  IDBG_MED("%s_%s:%d lib_ref_cnt %d", __func__, p_mod->name, __LINE__,
    p_mod->lib_ref_count);
  if (p_mod->lib_ref_count) {
    IMG_COMP_UNLOAD(p_core_ops);
  }
  p_mod->imgbase_client_cnt = 0;
  img_thread_mgr_destroy_pool();

  /* destroy mutexes */
  pthread_mutex_destroy(&p_mod->mutex);
  for (i = 0; i < IMGLIB_ARRAY_SIZE(p_mod->session_data_l); i++) {
    p_session_data = &p_mod->session_data_l[i];
    pthread_mutex_destroy(&p_session_data->q_mutex);
  }
  pthread_cond_destroy(&p_mod->cond);

  free(p_mod);
  mct_module_destroy(p_mct_mod);
}

/**
 * Function: module_imgbase_start_session
 *
 * Description: This function is called when a new camera
 *              session is started
 *
 * Arguments:
 *   @module: mct module pointer
 *   @sessionid: session id
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
static boolean module_imgbase_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  module_imgbase_t *p_mod;
  imgbase_session_data_t *p_session_data;

  if (!module || !module->module_private) {
    IDBG_ERROR("failed module %p or module private for sessionid %d",
      module, sessionid);
    rc = IMG_ERR_INVALID_INPUT;
    goto end;
  }

  p_mod = (module_imgbase_t *)module->module_private;
  p_session_data = IMGBASE_SSP(p_mod, sessionid);
  if (NULL == p_session_data) {
    /* Error printed in the macro*/
    rc = IMG_ERR_INVALID_INPUT;
    goto end;
  }

  p_session_data->sessionid = sessionid;
  p_session_data->p_mod = (void *)p_mod;

  QIMG_LOCK(&p_mod->mutex);

  if (!p_mod->session_cnt) {
    p_mod->ion_fd = img_buffer_open();
    if (p_mod->ion_fd < 0) {
      IDBG_ERROR("%s:%d] ion client invalid %d", __func__, __LINE__,
        p_mod->ion_fd);
      QIMG_UNLOCK(&p_mod->mutex);
      rc = IMG_ERR_GENERAL;
      goto end;
    }

    rc = module_imglib_common_get_bfr_mngr_subdev(&p_mod->subdevfd);
    if (!rc || p_mod->subdevfd < 0) {
      IDBG_ERROR("%s_%s:%d] invalid subdev fd %d", __func__, p_mod->name,
        __LINE__, p_mod->subdevfd);
      rc = IMG_ERR_GENERAL;
      QIMG_UNLOCK(&p_mod->mutex);
      goto end;
    }
  }

  /* set session count */
  p_mod->session_cnt++;
  /* init frame config */
  mct_queue_init(&p_session_data->frame_config_q);
  img_q_init(&p_session_data->framedrop_hold_q, "DropQ");

  IDBG_HIGH("calling img_dsp_reload");
  img_dsp_dl_requestall_to_close_and_reopen();

  if (p_mod->modparams.imgbase_session_start) {
    rc = p_mod->modparams.imgbase_session_start(p_mod, sessionid);
  }

  if ((p_mod->session_cnt == 1) &&
    (IMG_PRELOAD_COMMON_START_SESSION_MODE == p_mod->caps.preload_mode)) {
    rc = module_imgbase_client_preload(p_session_data);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("[%s] Preload failed", p_mod->name);
      QIMG_UNLOCK(&p_mod->mutex);
      goto end;
    }
  }

  QIMG_UNLOCK(&p_mod->mutex);

  IDBG_HIGH("%s_%s:%d] session_cnt %d", __func__, p_mod->name, __LINE__,
    p_mod->session_cnt);

end:
  return GET_STATUS(rc);
}

/**
 * Function: module_imgbase_stop_session
 *
 * Description: This function is called when the camera
 *              session is stopped
 *
 * Arguments:
 *   @module: mct module pointer
 *   @sessionid: session id
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
static boolean module_imgbase_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  module_imgbase_t *p_mod;
  imgbase_session_data_t *p_session_data;

  if (!module || !module->module_private || !sessionid ||
    (sessionid > MAX_IMGLIB_SESSIONS)) {
    IDBG_ERROR("failed module %p or mod_private for sessionid %d",
      module, sessionid);
    rc = IMG_ERR_INVALID_INPUT;
    goto end;
  }

  p_mod = (module_imgbase_t *)module->module_private;
  p_session_data = IMGBASE_SSP(p_mod, sessionid);
  if (NULL == p_session_data) {
    /* Error printed in the macro*/
    rc = IMG_ERR_INVALID_INPUT;
    goto end;
  }

  img_q_flush_and_destroy(&p_session_data->framedrop_hold_q);
  img_q_deinit(&p_session_data->framedrop_hold_q);

  p_session_data->sessionid = 0;

  QIMG_LOCK(&p_mod->mutex);

  if (p_mod->modparams.imgbase_session_stop) {
    p_mod->modparams.imgbase_session_stop(p_mod, sessionid);
  }
  pthread_mutex_lock(&p_session_data->q_mutex);
  mct_queue_flush(&p_session_data->frame_config_q,
    module_imgbase_free_config_data);
  pthread_mutex_unlock(&p_session_data->q_mutex);

  IDBG_MED("[%s] trying shutdown session_cnt %d, sessionid%d",
    p_mod->name, p_mod->session_cnt, sessionid);

  /* If the module does a preload per session, do a shutdown*/
  if (IMG_PRELOAD_PER_SESSION_MODE == p_mod->caps.preload_mode) {
    IDBG_MED("[%s] shutdown per session called for session %d",
      p_mod->name, sessionid);
    rc = IMG_CORE_SHUTDOWN(&p_mod->core_ops);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("[%s] shutdown failed", p_mod->name);
    } else {
      p_session_data->preload_params.preload_done = false;
    }
  }

  /* set session count */
  p_mod->session_cnt--;

  if (!(p_mod->session_cnt) ) {
    if (p_mod->ion_fd >= 0) {
      img_buffer_close();
      p_mod->ion_fd = -1;
    }
    if (p_mod->subdevfd >= 0) {
      close(p_mod->subdevfd);
      p_mod->subdevfd = -1;
    }

    /* For modules that do a pre-load once for all sessions,
    do a shutdown when session count is 0 */
    if ((IMG_PRELOAD_COMMON_START_SESSION_MODE == p_mod->caps.preload_mode ||
      IMG_PRELOAD_COMMON_STREAM_CFG_MODE == p_mod->caps.preload_mode) &&
      (p_mod->preload_done == true)) {
      rc = IMG_CORE_SHUTDOWN(&p_mod->core_ops);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("[%s] shutdown failed", p_mod->name);
      } else {
        p_mod->preload_done = false;
      }
    }
  }
  QIMG_UNLOCK(&p_mod->mutex);

  IDBG_HIGH("%s_%s:%d] session_cnt %d", __func__, p_mod->name, __LINE__,
    p_mod->session_cnt);

end:
  return GET_STATUS(rc);
}

/** module_imgbase_set_parent:
 *
 *  Arguments:
 *  @p_parent - parent module pointer
 *
 * Description: This function is used to set the parent pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_imgbase_set_parent(mct_module_t *p_mct_mod, mct_module_t *p_parent)
{
  module_imgbase_t *p_mod = NULL;

  if (!p_mct_mod || !p_parent)
    return;

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;

  if (!p_mod)
    return;

  p_mod->parent_mod = p_parent;
}

/**
 * Function: module_imgbase_post_bus_msg
 *
 * Description: post a particular message to media bus
 *
 * Arguments:
 *   @p_mct_mod - media controller module
 *   @sessionid - session id
 *   @msg_id - bus message id
 *   @msg_data - bus message data
 *   @size - bus message size
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_imgbase_post_bus_msg(mct_module_t *p_mct_mod,
  unsigned int sessionid, mct_bus_msg_type_t msg_id, void *msg_data, uint32_t size)
{
  mct_bus_msg_t bus_msg;
  memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.type = msg_id;
  bus_msg.msg = msg_data;
  bus_msg.size = size;
  bus_msg.sessionid = sessionid;
  IDBG_MED("%s:%d] session id %d mct_mod %p", __func__, __LINE__,
    bus_msg.sessionid, p_mct_mod);
  mct_module_post_bus_msg(p_mct_mod, &bus_msg);
}

/** module_imgbase_init:
 *
 *  Arguments:
 *  @name - name of the module
 *  @comp_role: imaging component role
 *  @comp_name: imaging component name
 *  @mod_private: derived structure pointer
 *  @p_caps: imaging capability
 *  @lib_name: library name
 *  @feature_mask: feature mask of imaging algo
 *  @p_modparams: module parameters
 *
 * Description: This function is used to initialize the imgbase
 * module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_imgbase_init(const char *name,
  img_comp_role_t comp_role,
  char *comp_name,
  void *mod_private,
  img_caps_t *p_caps,
  char *lib_name,
  cam_feature_mask_t feature_mask,
  module_imgbase_params_t *p_modparams)
{
  mct_module_t *p_mct_mod = NULL;
  module_imgbase_t *p_mod = NULL;
  img_core_ops_t *p_core_ops = NULL;
  mct_port_t *p_sinkport = NULL, *p_sourceport = NULL;
  int rc = 0;
  uint32_t i = 0;
  imgbase_session_data_t *p_session_data;

  if (!name || !comp_name || (comp_role >= IMG_COMP_ROLE_MAX)
    || !p_caps || !lib_name) {
    IDBG_ERROR("invalid input");
    return NULL;
  }

  IDBG_MED("%s] ", name);

  /*create a thread pool shared by all imglib modules */
  rc = img_thread_mgr_create_pool();
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Error create thread pool %d", rc);
    return NULL;
  }

  p_mct_mod = mct_module_create(name);
  if (NULL == p_mct_mod) {
    IDBG_ERROR("%s] cannot allocate mct module", name);
    // img_thread_mgr_destroy_pool();
    return NULL;
  }

  p_mod = malloc(sizeof(module_imgbase_t));
  if (NULL == p_mod) {
    IDBG_ERROR("%s] failed", name);
    goto error;
  }

  p_mct_mod->module_private = (void *)p_mod;
  memset(p_mod, 0, sizeof(module_imgbase_t));

  pthread_mutex_init(&p_mod->mutex, NULL);
  pthread_cond_init(&p_mod->cond, NULL);
  p_core_ops = &p_mod->core_ops;
  /* initialize per frame mutex */
  for (i = 0; i < IMGLIB_ARRAY_SIZE(p_mod->session_data_l); i++) {
    p_session_data = &p_mod->session_data_l[i];
    pthread_mutex_init(&p_session_data->q_mutex, NULL);
  }

  IDBG_MED("%s] ", name);
  /* check if the imgbase module is present */
  rc = img_core_get_comp(comp_role, comp_name, p_core_ops);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s] Error rc %d", name, rc);
    goto error;
  }

 /* try to load the component */
  rc = IMG_COMP_LOAD(p_core_ops, lib_name);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s] Error rc %d", name, rc);
    goto error;
  }
  p_mod->lib_ref_count++;
  p_mod->imgbase_client = NULL;
  p_mod->mod_private = mod_private;
  p_mod->p_mct_mod = p_mct_mod;
  p_mod->caps = *p_caps;
  p_mod->subdevfd = -1;
  p_mod->ion_fd = -1;
  p_mod->feature_mask = feature_mask;
  p_mod->name = name;
  p_mod->ssid_delta = 1;
  p_mod->force_cache_op = module_imglib_common_get_prop(
    "persist.vendor.camera.force_cache_ops", "0");
  if (p_modparams)
    p_mod->modparams = *p_modparams;

  /* create static ports */
  for (i = 0; i < MAX_IMGLIB_BASE_STATIC_PORTS; i++) {
    p_sinkport = module_imgbase_create_port(p_mct_mod, MCT_PORT_SINK);
    if (NULL == p_sinkport) {
      IDBG_ERROR("%s] create SINK port failed", p_mod->name);
      goto error;
    }
    p_sourceport = module_imgbase_create_port(p_mct_mod, MCT_PORT_SRC);
    if (NULL == p_sourceport) {
      IDBG_ERROR("%s] create SINK port failed", p_mod->name);
      goto error;
    }
  }

  p_mct_mod->set_mod          = module_imgbase_set_mod;
  p_mct_mod->start_session    = module_imgbase_start_session;
  p_mct_mod->stop_session     = module_imgbase_stop_session;
  p_mct_mod->query_mod        = module_imgbase_query_mod;
  p_mct_mod->set_session_data = module_imgbase_set_session_data;

  IDBG_MED("%s] %p", p_mod->name, p_mct_mod);
  return p_mct_mod;

error:
  if (p_mod) {
    module_imgbase_deinit(p_mct_mod);
  } else if (p_mct_mod) {
    mct_module_destroy(p_mct_mod);
    p_mct_mod = NULL;
  }
  // img_thread_mgr_destroy_pool();
  return NULL;
}
