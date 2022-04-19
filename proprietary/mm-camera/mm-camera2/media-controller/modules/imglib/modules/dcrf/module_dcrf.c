/**********************************************************************
*  Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "module_imgbase.h"
#include "dual_frameproc_comp.h"
#include "chromatix_3a.h"

#define MAX_EXTRA_BUF 2

/**DEFAULT_ROI_TO_CENTER
*
* Defines if the ROI sent to DCRF lib needs to
* be set to the center
*/
#define DEFAULT_ROI_TO_CENTER 1

/**ROI_SIZE_FACTOR
*
* Defines the size of the ROI as a factor of
* image size
*/
#define ROI_SIZE_FACTOR 3

/**
 *  Static functions
 **/
static boolean module_dcrf_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* /*p_mod*/,unsigned int sessionid);
static int32_t module_dcrf_client_created(imgbase_client_t *p_client);
static int32_t module_dcrf_client_destroy(imgbase_client_t *p_client);
static int module_dcrf_handle_module_event(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled);
static int module_dcrf_event_handler(imgbase_client_t *p_client,
  img_event_t *p_event);
static int module_dcrf_client_update_meta(imgbase_client_t *p_client,
  img_meta_t *p_meta);

/** g_caps:
 *
 *  Set the capabilities for DCRF module
*/
static img_caps_t g_caps = {
  .num_input = 1,
  .num_output = 0,
  .num_meta = 1,
  .inplace_algo = 1,
  .ack_required = 1,
  .share_client_per_session = 1,
};

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_query_mod = module_dcrf_query_mod,
  .imgbase_client_created = module_dcrf_client_created,
  .imgbase_client_destroy = module_dcrf_client_destroy,
  .imgbase_client_update_meta = module_dcrf_client_update_meta,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_SET_STREAM_CONFIG] =
    module_dcrf_handle_module_event,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_ISP_OUTPUT_DIM] =
    module_dcrf_handle_module_event,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_IMGLIB_AF_CONFIG] =
    module_dcrf_handle_module_event,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_SET_DUAL_OTP_PTR] =
    module_dcrf_handle_module_event,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_IMGLIB_DCRF_CFG] =
    module_dcrf_handle_module_event,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_SET_CHROMATIX_PTR] =
    module_dcrf_handle_module_event,
  .imgbase_client_event_handler[QIMG_EVT_DCRF_DONE] =
    module_dcrf_event_handler,
  .exec_mode = IMG_EXECUTION_SW,
  .access_mode = IMG_ACCESS_READ,
  .force_cache_op = FALSE,
};

/** module_dcrf_private_data_t
*   @cur_dciaf_cfg: current dciaf configure data
*   @dcrf_init_params: DCRF init data
*   @request_crop: sensor request crop
*   @af_monitor_fps: AF fps in monitoring mode
*   @af_search_fps: AF fps in searching mode
*   @dcrf_fps: DCRF fps
*   @is_dcrf_cfg_received: DCRF config received flag
*   @dcrf_ratio: size conversion ratio isp_out/camif
*
*   IMGLIB_BASE client structure
**/
typedef struct {
  mct_imglib_dciaf_config_t cur_dciaf_cfg;
  img_dual_cam_init_params_t dcrf_init_params;
  sensor_request_crop_t request_crop;
  uint32_t af_monitor_fps;
  uint32_t af_search_fps;
  uint32_t dcrf_fps;
  img_dcrf_input_runtime_param_t dcrf_runtime_param;
  boolean is_dcrf_cfg_received;
  float dcrf_ratio;
} module_dcrf_private_data_t;

/**
 * Function: module_dcrf_query_mod
 *
 * Description: This function is used to query DCRF caps
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
boolean module_dcrf_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* p_mod,unsigned int sessionid)
{
  IMG_UNUSED(p_mod);
  IMG_UNUSED(sessionid);
  if (!p_mct_cap) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return FALSE;
  }

  return TRUE;
}

/**
 * Function: module_dcrf_client_created
 *
 * Description: function called after client creation
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_NO_MEMORY
 *
 * Notes: none
 **/
int32_t module_dcrf_client_created(imgbase_client_t *p_client)
{
  IDBG_MED("%s %d: E", __func__, __LINE__);

  module_dcrf_private_data_t *p_private_data =
    calloc(1, sizeof(module_dcrf_private_data_t));

  if (!p_private_data) {
    IDBG_ERROR("%s:%d] Failed alloc private data", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }
  p_client->p_private_data = p_private_data;
  p_client->before_cpp = TRUE;
  p_client->streams_to_process = 1 << CAM_STREAM_TYPE_ANALYSIS;

  /* DCRF ping-pong buffer requires extra buffers */
  p_client->isp_extra_native_buf += MAX_EXTRA_BUF;
  return IMG_SUCCESS;
}

/**
 * Function: module_dcrf_client_destroy
 *
 * Description: function called after client destroy
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *
 * Notes: none
 **/
int32_t module_dcrf_client_destroy(imgbase_client_t *p_client)
{
  IDBG_MED("%s %d: E", __func__, __LINE__);

  if (p_client->p_private_data) {
    free(p_client->p_private_data);
  }
  return IMG_SUCCESS;
}

/**
 * Function: module_dcrf_get_frame_format
 *
 * Description: Get the dcrf frame format
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *   @fmt - Cam frame format
 *
 * Return values:
 *   dcrf frame format
 *
 **/
static int module_dcrf_get_frame_format(imgbase_client_t *p_client,
  cam_format_t fmt)
{
  dcrf_frame_format_type dcrf_fmt;

  switch (fmt) {
  case CAM_FORMAT_YUV_420_NV12:
  case CAM_FORMAT_YUV_420_NV21:
  case CAM_FORMAT_YUV_420_NV21_ADRENO:
  case CAM_FORMAT_YUV_420_YV12:
  case CAM_FORMAT_YUV_422_NV16:
  case CAM_FORMAT_YUV_422_NV61:
  case CAM_FORMAT_YUV_420_NV12_VENUS:
  case CAM_FORMAT_YUV_420_NV21_VENUS:
  case CAM_FORMAT_Y_ONLY:
    dcrf_fmt = DCRF_FORMAT_YUV;
    p_client->plane_type[0] = PLANE_Y;
    break;
  case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
    dcrf_fmt = DCRF_FORMAT_YUYV_YVYU;
    p_client->plane_type[0] = PLANE_Y_CB_Y_CR;
    break;
  case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
    dcrf_fmt = DCRF_FORMAT_YUYV_YVYU;
    p_client->plane_type[0] = PLANE_Y_CR_Y_CB;
    break;
  case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
    dcrf_fmt = DCRF_FORMAT_UYVY_VYUY;
    p_client->plane_type[0] = PLANE_CB_Y_CR_Y;
    break;
  case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
    dcrf_fmt = DCRF_FORMAT_UYVY_VYUY;
    p_client->plane_type[0] = PLANE_CR_Y_CB_Y;
    break;
  default:
    IDBG_ERROR("%s:%d] Format not supported %d", __func__, __LINE__, fmt);
    return DCRF_FORMAT_MAX;
  }

  return dcrf_fmt;
}

/**
 * Function: module_dcrf_handle_module_event
 *
 * Description: This function handles module event
 *
 * Arguments:
 *   p_mod_event - pointer of module event
 *   p_client - pointer to imgbase client
 *   p_core_ops - pointer to imgbase module ops
 *
 * Return values:
 *     TRUE/FALSE
 *
 * Notes: none
 **/
boolean module_dcrf_handle_module_event(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled)
{
  int rc = IMG_SUCCESS;
  boolean fwd_event = FALSE;
  imgbase_session_data_t *p_session_data;
  module_imgbase_t *p_mod;


  if (!p_mod_event || !p_client || !p_core_ops) {
    IDBG_ERROR("%s:%d] Error input", __func__, __LINE__);
    return fwd_event;
  }
  img_component_ops_t *p_comp = &p_client->comp;
  module_dcrf_private_data_t *p_private_data =
    (module_dcrf_private_data_t *)p_client->p_private_data;
  img_dual_cam_init_params_t *p_dcrf_init_params =
    &p_private_data->dcrf_init_params;

  p_mod = (module_imgbase_t *)p_client->p_mod;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return fwd_event;
  }

  p_session_data = IMGBASE_SSP(p_mod, p_client->session_id);
  if (NULL == p_session_data) {
    /* Error printed in the macro*/
    return fwd_event;
  }

  switch (p_mod_event->type) {
  case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
    sensor_out_info_t *sensor_info;
    sensor_info = (sensor_out_info_t *)p_mod_event->module_event_data;
    IDBG_MED("%s:%d] MCT_EVENT_MODULE_SET_STREAM_CONFIG, w = %u, h = %u",
      __func__, __LINE__,
      sensor_info->dim_output.width,
      sensor_info->dim_output.height);
    p_dcrf_init_params->main.sensor_w =
      (int32_t)sensor_info->dim_output.width;
    p_dcrf_init_params->main.sensor_h =
      (int32_t)sensor_info->dim_output.height;
    p_dcrf_init_params->main.sensor_x_offset = 0;
    p_dcrf_init_params->main.sensor_y_offset = 0;
    p_private_data->request_crop = sensor_info->request_crop;

    break;
  }
  case MCT_EVENT_MODULE_ISP_OUTPUT_DIM: {
    mct_stream_info_t *stream_info =
      (mct_stream_info_t *)(p_mod_event->module_event_data);

    if (stream_info) {
      p_client->isp_output_dim_stream_info = *stream_info;
      p_client->isp_output_dim_stream_info_valid = TRUE;
      p_dcrf_init_params->main.width =
        stream_info->dim.width;
      p_dcrf_init_params->main.height =
        stream_info->dim.height;
      p_dcrf_init_params->main.stride =
        stream_info->buf_planes.plane_info.mp[0].stride;
      p_dcrf_init_params->main.format =
        module_dcrf_get_frame_format(p_client, stream_info->fmt);

      IDBG_MED("%s:%d] MCT_EVENT_MODULE_ISP_OUTPUT_DIM "
        "p_client [%p]  sensor_info %d  port %p",
        __func__, __LINE__, p_client,
        p_session_data->multi_cam_info.cam_type,
        p_client->p_intra_port);

      /* send intra port event to master camera */
      if (p_session_data->multi_cam_info.peer_session_ids[0] &&
        CAM_TYPE_AUX == p_session_data->multi_cam_info.cam_type) {
        mct_event_t event;
        boolean status = FALSE;
        mct_port_t *p_intra_port = p_client->p_intra_port;
        /* Pack into an mct_event object */
        event.direction = MCT_EVENT_INTRA_MOD;
        event.identity =  p_session_data->multi_cam_info.peer_session_ids[0];
        event.type = MCT_EVENT_MODULE_EVENT;
        event.u.module_event.current_frame_id = p_client->frame_id;
        event.u.module_event.type = MCT_EVENT_MODULE_IMGLIB_DCRF_CFG;
        event.u.module_event.module_event_data = (void *)p_client;
        status = mct_port_send_intra_module_event(p_intra_port, &event);
        if (!status) {
          IDBG_ERROR("%s:%d] Sending intraport event failed",
            __func__, __LINE__);
        }
      }
      /* send init params to DCRF comp if we haven't done so */
      else if (CAM_TYPE_MAIN == p_session_data->multi_cam_info.cam_type &&
        p_private_data->is_dcrf_cfg_received == TRUE) {
        p_private_data->is_dcrf_cfg_received = FALSE;
        rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_DCRF, p_dcrf_init_params);
        if (IMG_ERROR(rc)) {
          IDBG_ERROR("%s:%d] Set dcrf init params failed %d",
            __func__, __LINE__, rc);
        }
      }
    }
    break;
  }
  case MCT_EVENT_MODULE_IMGLIB_AF_CONFIG : {
    mct_imglib_af_config_t *cfg =
      (mct_imglib_af_config_t *)p_mod_event->module_event_data;
    IDBG_MED("%s:%d] MCT_EVENT_MODULE_IMGLIB_AF_CONFIG cfg_type %d",
      __func__, __LINE__, cfg->type);
    if (NULL == cfg || AF_CFG_DCIAF != cfg->type) {
      break;
    }

    mct_imglib_dciaf_config_t *p_cfg = &(cfg->u.dciaf);
    imgbase_client_t *p_peer_client = p_client->p_peer_client;

    if (p_cfg) {
      p_private_data->cur_dciaf_cfg = *p_cfg;
       IDBG_MED("%s:%d] MCT_EVENT_MODULE_IMGLIB_AF_CONFIG, DCRF enabled %d",
      __func__, __LINE__, p_private_data->cur_dciaf_cfg.requireDistanceInfo);
      /* get DCRF FPS from 3A chromatix based on AF status */
      if (p_private_data->cur_dciaf_cfg.requireDistanceInfo) {
        pthread_mutex_lock(&p_client->mutex);
        p_client->processing_disabled = FALSE;
        //update peer client processing flag
        if (p_peer_client) {
          p_peer_client->processing_disabled = FALSE;
        }
        pthread_mutex_unlock(&p_client->mutex);
        p_private_data->dcrf_fps =
          p_private_data->af_search_fps;
      } else {
        pthread_mutex_lock(&p_client->mutex);
        if (!p_client->processing_disabled) {
          p_client->processing_disabled = TRUE;
          //update peer client processing flag
          if (p_peer_client) {
            p_peer_client->processing_disabled = TRUE;
          }
        }
        pthread_mutex_unlock(&p_client->mutex);
        p_private_data->dcrf_fps =
          p_private_data->af_monitor_fps;
      }
      IDBG_MED("%s:%d]  AF status %d ROI (%d,%d) %dx%d  zoom %f",
        __func__, __LINE__,
        p_cfg->is_af_search_active, p_cfg->roi.left, p_cfg->roi.top,
        p_cfg->roi.width, p_cfg->roi.height,
        p_cfg->focal_length_ratio);
    }
    break;
  }

  case MCT_EVENT_MODULE_SET_DUAL_OTP_PTR : {
    cam_related_system_calibration_data_t *p_calib_data =
      (cam_related_system_calibration_data_t *)p_mod_event->module_event_data;
    IDBG_MED("%s:%d] MCT_EVENT_MODULE_SET_DUAL_OTP_PTR ", __func__, __LINE__);
    if (NULL == p_calib_data) {
      IDBG_ERROR("%s %d: calib_data is null", __func__, __LINE__);
      break;
    }
    if (p_dcrf_init_params) {
      memcpy(&p_dcrf_init_params->dcrf_calib_data,
        p_calib_data, sizeof(img_dcrf_calib_t));
      IDBG_MED("%s:%d] calibration_format_version %d baseline dis %f",
        __func__, __LINE__,
        p_dcrf_init_params->dcrf_calib_data.calibration_format_version,
        p_dcrf_init_params->dcrf_calib_data.relative_baseline_distance);

    }
    break;
  }

  case MCT_EVENT_MODULE_SET_CHROMATIX_PTR: {
    modulesChromatix_t *chromatix_param =
      (modulesChromatix_t*)p_mod_event->module_event_data;
    if (chromatix_param) {
      chromatix_3a_parms_type *p3a_chromatix =
        chromatix_param->chromatix3APtr;
      IDBG_MED("%s %d: MCT_EVENT_MODULE_SET_CHROMATIX_PTR %p", __func__,
        __LINE__, p3a_chromatix);
      p_private_data->af_monitor_fps =
        p3a_chromatix->AF_algo_data.af_algo.af_haf.dciaf.monitor_freq;
      p_private_data->af_search_fps =
        p3a_chromatix->AF_algo_data.af_algo.af_haf.dciaf.search_freq;
      IDBG_MED("%s %d: AF monitor fps %d AF search fps %d", __func__,
        __LINE__, p_private_data->af_monitor_fps,
        p_private_data->af_search_fps);
    } else {
      IDBG_ERROR("%s %d: MCT_EVENT_MODULE_SET_CHROMATIX_PTR"
       "chromatix param is null", __func__, __LINE__);
    }
    break;
  }

  case MCT_EVENT_MODULE_IMGLIB_DCRF_CFG: {
    /* Master cam client receive aux cam client info */
    imgbase_client_t *p_peer_client = p_mod_event->module_event_data;
    img_component_ops_t *p_peer_comp = &p_peer_client->comp;
    IDBG_MED("%s:%d] p_peer_client %p p_peer_comp %p", __func__, __LINE__,
      p_peer_client, p_peer_comp);

    module_dcrf_private_data_t *p_peer_module_private_data =
      (module_dcrf_private_data_t *)p_peer_client->p_private_data;

    p_client->p_peer_comp = p_peer_comp;
    p_client->p_peer_client = (void *)p_peer_client;
    img_dual_cam_init_params_t aux_dcrf_init_params =
      p_peer_module_private_data->dcrf_init_params;
    p_dcrf_init_params->aux.width = aux_dcrf_init_params.main.width;
    p_dcrf_init_params->aux.height = aux_dcrf_init_params.main.height;
    p_dcrf_init_params->aux.stride = aux_dcrf_init_params.main.stride;
    p_dcrf_init_params->aux.format = aux_dcrf_init_params.main.format;
    p_dcrf_init_params->aux.sensor_w = aux_dcrf_init_params.main.sensor_w;
    p_dcrf_init_params->aux.sensor_h = aux_dcrf_init_params.main.sensor_h;
    p_dcrf_init_params->aux.sensor_x_offset =
      aux_dcrf_init_params.main.sensor_x_offset;
    p_dcrf_init_params->aux.sensor_y_offset =
      aux_dcrf_init_params.main.sensor_y_offset;

    /* Todo: fill dcrf init params from chromatix */
    p_dcrf_init_params->alignment_check_enable = 0;
    p_dcrf_init_params->aux_direction = IMG_AF_CAM_ORIENT_LEFT;
    p_dcrf_init_params->baseline_mm = 20.0f;
    p_dcrf_init_params->focus_preference = 0x110;
    p_dcrf_init_params->macro_est_limit_cm = 10;

    /* Bind dual camera */
    if (!p_client->is_binded && p_client->p_peer_comp) {
      if (p_core_ops->bind) {
        IDBG_MED("%s:%d] Bind comp %p peer_comp %p is_binded %d",
          __func__, __LINE__,
          p_comp, p_client->p_peer_comp, p_client->is_binded);
        rc = IMG_COMP_BIND(p_core_ops, p_comp, p_client->p_peer_comp);
        p_client->is_binded = TRUE;
        if (IMG_ERROR(rc)) {
          IDBG_ERROR("%s:%d] Bind failed %d", __func__, __LINE__, rc);
        }
      }
    }

    if (p_private_data->dcrf_init_params.main.sensor_w != 0) {
      rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_DCRF, p_dcrf_init_params);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("%s:%d] Set dcrf init params failed %d",
          __func__, __LINE__, rc);
      }
    } else {  //we still have not received the config info from main sensor
      p_private_data->is_dcrf_cfg_received = TRUE;
    }
    break;
  }
  default:
    break;
  }

  return fwd_event;
}

/**
 * Function: module_dcrf_handle_dcrf_done
 *
 * Description: Function to handle dcrf done event
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static void module_dcrf_handle_dcrf_done(imgbase_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  boolean ret;
  img_dcrf_output_result_t result;
  img_dcrf_output_result_t *p_result = &result;
  mct_event_t mct_event;
  int str_idx;
  mct_module_t *p_mct_mod;
  unsigned int sessionid;
  imgbase_stream_t *p_stream = NULL;

  if (!p_client) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return;
  }

  if (p_client->processing_disabled) {
    IDBG_HIGH("%s:%d] DCRF Disabled, Not posting result",
      __func__, __LINE__);
    return;
  }

  img_component_ops_t *p_comp;
  p_comp = &p_client->comp;

  rc = IMG_COMP_GET_PARAM(p_comp, QIMG_DCRF_RESULT, (void *)p_result);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return;
  }

  /* create MCT event and send */
  mct_imglib_dcrf_result_t dcrf_out;
  memset(&mct_event, 0x0, sizeof(mct_event_t));
  memset(&dcrf_out, 0x0, sizeof(mct_imglib_dcrf_result_t));

  module_dcrf_private_data_t *p_private_data =
    (module_dcrf_private_data_t *)p_client->p_private_data;
  img_dcrf_input_runtime_param_t *p_dcrf_runtime_param =
    &p_private_data->dcrf_runtime_param;

  dcrf_out.id = p_result->frame_id;
  dcrf_out.timestamp = p_result->timestamp;
  dcrf_out.distance_in_mm = p_result->distance_in_mm;
  dcrf_out.confidence = p_result->confidence;
  dcrf_out.status = p_result->status;
  if (p_private_data->dcrf_ratio > 0) {
    dcrf_out.focused_roi.width = p_result->focused_roi.size.width /
      p_private_data->dcrf_ratio;
    dcrf_out.focused_roi.height = p_result->focused_roi.size.height /
      p_private_data->dcrf_ratio;
    dcrf_out.focused_roi.left = (p_result->focused_roi.pos.x /
      p_private_data->dcrf_ratio) +
      p_dcrf_runtime_param->fov_params_main[3].offset_x;
    dcrf_out.focused_roi.top = (p_result->focused_roi.pos.y /
      p_private_data->dcrf_ratio) +
      p_dcrf_runtime_param->fov_params_main[3].offset_y;
    dcrf_out.focused_x = (p_result->focused_x /
      p_private_data->dcrf_ratio) +
      p_dcrf_runtime_param->fov_params_main[3].offset_x;
    dcrf_out.focused_y = (p_result->focused_y /
      p_private_data->dcrf_ratio) +
      p_dcrf_runtime_param->fov_params_main[3].offset_y;
  }

  /* report 0 distance when special effect on or DCRF error cases */
  if (p_client->effect != 0 || IMG_ERROR(dcrf_out.status)) {
    dcrf_out.distance_in_mm = 0;
    dcrf_out.confidence = 0;
  }

  /* from current stream to send event*/
  str_idx = module_imgbase_find_stream_by_identity(p_client,
    p_client->divert_identity);
  if (str_idx < 0) {
    IDBG_ERROR("%s:%d] Invalid Stream index", __func__, __LINE__);
    return;
  }

  p_stream = &p_client->stream[str_idx];
  if (NULL == p_stream) {
    IDBG_ERROR("%s:%d] Cannot find stream at index %d", __func__,
      __LINE__, str_idx);
    return;
  }
  ret = mod_imgbase_send_event(p_client->divert_identity, TRUE,
    MCT_EVENT_MODULE_IMGLIB_DCRF_OUTPUT, dcrf_out);
  if (!ret) {
    IDBG_ERROR("%s:%d] send mct event fail", __func__, __LINE__);
    return;
  }

  cam_dcrf_result_t dcrf_result;
  p_mct_mod =
    MCT_MODULE_CAST(MCT_PORT_PARENT(p_client->stream[str_idx].p_sinkport)->data);
  sessionid = IMGLIB_SESSIONID(p_client->divert_identity);

  dcrf_result.id = dcrf_out.id;
  dcrf_result.timestamp = dcrf_out.timestamp;
  dcrf_result.distance_in_mm = dcrf_out.distance_in_mm;
  dcrf_result.confidence = dcrf_out.confidence;
  dcrf_result.status = dcrf_out.status;
  dcrf_result.focused_roi.width = dcrf_out.focused_roi.width;
  dcrf_result.focused_roi.height = dcrf_out.focused_roi.height;
  dcrf_result.focused_roi.left = dcrf_out.focused_roi.left;
  dcrf_result.focused_roi.top = dcrf_out.focused_roi.top;
  dcrf_result.focused_x = dcrf_out.focused_x;
  dcrf_result.focused_y = dcrf_out.focused_y;

  IDBG_HIGH("%s:%d] DCRF result frameID:%d timestamp %lld dis %d "
    "confidence %d status %d roi %dx%d (%d,%d) focus(%d,%d) ",
    __func__, __LINE__,
    dcrf_result.id,
    dcrf_result.timestamp,
    dcrf_result.distance_in_mm,
    dcrf_result.confidence,
    dcrf_result.status,
    dcrf_result.focused_roi.width,
    dcrf_result.focused_roi.height,
    dcrf_result.focused_roi.left,
    dcrf_result.focused_roi.top,
    dcrf_result.focused_x,
    dcrf_result.focused_y);

  module_imgbase_post_bus_msg(p_mct_mod, sessionid,
    MCT_BUS_MSG_DCRF_RESULT, &dcrf_result, sizeof(cam_dcrf_result_t));

}

/**
 * Function: module_dcrf_event_handler
 *
 * Description: event handler for dcrf module
 *
 * Arguments:
 *   @p_client - pointer to IMGLIB_BASE client
 *   @p_event - pointer to the event
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int module_dcrf_event_handler(imgbase_client_t *p_client,
  img_event_t *p_event)
{
  int rc = IMG_SUCCESS;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  if ((NULL == p_event) || (NULL == p_client)) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  switch (p_event->type) {
  case QIMG_EVT_DCRF_DONE: {
    module_dcrf_handle_dcrf_done(p_client);
    break;
  }
  default:
    break;
  }

  IDBG_LOW("%s:%d] type %d X", __func__, __LINE__, p_event->type);
  return rc;

}

/**
 * Function: module_dcrf_fill_runtime_params
 *
 * Description: helper to fill runtime params for DCRF
 *
 * Arguments:
 *   @p_client - pointer to IMGLIB_BASE client
 *
 * Return values:
 * None
 *
 * Notes: none
 **/
void module_dcrf_fill_runtime_params(imgbase_client_t *p_client)
{
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return;
  }
  module_dcrf_private_data_t *p_private_data =
    (module_dcrf_private_data_t *)p_client->p_private_data;

  img_dcrf_input_runtime_param_t *p_dcrf_runtime_param =
    &p_private_data->dcrf_runtime_param;

  /* for YUYV format the actual width is 1/2 of the sensor width */
  uint32_t width_factor = 1;
  if (p_private_data->dcrf_init_params.main.format ==
    DCRF_FORMAT_YUYV_YVYU) {
    IDBG_MED("%s:%d] DCRF_FORMAT_YUYV_YVYU format", __func__, __LINE__);
    width_factor = 2;
  }

  /* Fill FOVs */
  p_dcrf_runtime_param->fov_params_main[0].module = SENSOR_FOV;
  p_dcrf_runtime_param->fov_params_main[0].input_width =
    p_private_data->dcrf_init_params.main.sensor_w  / width_factor;
  p_dcrf_runtime_param->fov_params_main[0].input_height =
    p_private_data->dcrf_init_params.main.sensor_h;
  p_dcrf_runtime_param->fov_params_main[0].offset_x = 0;
  p_dcrf_runtime_param->fov_params_main[0].offset_y = 0;
  p_dcrf_runtime_param->fov_params_main[0].fetch_window_width =
    p_private_data->dcrf_init_params.main.sensor_w / width_factor;
  p_dcrf_runtime_param->fov_params_main[0].fetch_window_height =
    p_private_data->dcrf_init_params.main.sensor_h;
  p_dcrf_runtime_param->fov_params_main[0].output_window_width =
    p_private_data->dcrf_init_params.main.sensor_w  / width_factor;
  p_dcrf_runtime_param->fov_params_main[0].output_window_height =
    p_private_data->dcrf_init_params.main.sensor_h;

  p_dcrf_runtime_param->fov_params_main[1].module = ISPIF_FOV;
  p_dcrf_runtime_param->fov_params_main[1].input_width =
    p_dcrf_runtime_param->fov_params_main[0].output_window_width;
  p_dcrf_runtime_param->fov_params_main[1].input_height =
    p_dcrf_runtime_param->fov_params_main[0].output_window_height;
  p_dcrf_runtime_param->fov_params_main[1].offset_x = 0;
  p_dcrf_runtime_param->fov_params_main[1].offset_y = 0;
  p_dcrf_runtime_param->fov_params_main[1].fetch_window_width =
    p_dcrf_runtime_param->fov_params_main[1].input_width;
  p_dcrf_runtime_param->fov_params_main[1].fetch_window_height =
    p_dcrf_runtime_param->fov_params_main[1].input_height;
  p_dcrf_runtime_param->fov_params_main[1].output_window_width =
    p_dcrf_runtime_param->fov_params_main[1].input_width;
  p_dcrf_runtime_param->fov_params_main[1].output_window_height =
    p_dcrf_runtime_param->fov_params_main[1].input_height;

  p_dcrf_runtime_param->fov_params_main[2].module = CAMIF_FOV;
  p_dcrf_runtime_param->fov_params_main[2].input_width =
    p_dcrf_runtime_param->fov_params_main[1].output_window_width;
  p_dcrf_runtime_param->fov_params_main[2].input_height =
    p_dcrf_runtime_param->fov_params_main[1].output_window_height;
  p_dcrf_runtime_param->fov_params_main[2].offset_x =
    p_private_data->request_crop.first_pixel;
  p_dcrf_runtime_param->fov_params_main[2].offset_y =
    p_private_data->request_crop.first_line;
  p_dcrf_runtime_param->fov_params_main[2].fetch_window_width =
    (p_private_data->request_crop.last_pixel -
    p_private_data->request_crop.first_pixel + 1) / width_factor;
  p_dcrf_runtime_param->fov_params_main[2].fetch_window_height =
    p_private_data->request_crop.last_line -
    p_private_data->request_crop.first_line + 1;
  p_dcrf_runtime_param->fov_params_main[2].output_window_width =
    p_dcrf_runtime_param->fov_params_main[2].fetch_window_width;
  p_dcrf_runtime_param->fov_params_main[2].output_window_height =
    p_dcrf_runtime_param->fov_params_main[2].fetch_window_height;

  p_dcrf_runtime_param->fov_params_main[3].module = ISP_OUT_FOV;
  p_dcrf_runtime_param->fov_params_main[3].input_width =
    p_dcrf_runtime_param->fov_params_main[2].output_window_width;
  p_dcrf_runtime_param->fov_params_main[3].input_height =
    p_dcrf_runtime_param->fov_params_main[2].output_window_height;
  p_dcrf_runtime_param->fov_params_main[3].offset_x =
    p_client->stream_crop.x_map;
  p_dcrf_runtime_param->fov_params_main[3].offset_y =
    p_client->stream_crop.y_map;
  p_dcrf_runtime_param->fov_params_main[3].fetch_window_width =
    p_client->stream_crop_valid ?
    p_client->stream_crop.width_map :
    (uint32_t)p_client->isp_output_dim_stream_info.dim.width;
  p_dcrf_runtime_param->fov_params_main[3].fetch_window_height =
    p_client->stream_crop_valid ?
    p_client->stream_crop.height_map :
    (uint32_t)p_client->isp_output_dim_stream_info.dim.height;
  p_dcrf_runtime_param->fov_params_main[3].output_window_width =
    p_client->isp_output_dim_stream_info.dim.width;
  p_dcrf_runtime_param->fov_params_main[3].output_window_height =
    p_client->isp_output_dim_stream_info.dim.height;

  if (p_dcrf_runtime_param->fov_params_main[3].fetch_window_width) {
    p_private_data->dcrf_ratio =
      (float)p_client->isp_output_dim_stream_info.dim.width /
      (float)p_dcrf_runtime_param->fov_params_main[3].fetch_window_width;
  }

  IDBG_MED("%s:%d] dcrf_ratio %f", __func__, __LINE__,
    p_private_data->dcrf_ratio);

  p_dcrf_runtime_param->lens_zoom_ratio =
    p_private_data->cur_dciaf_cfg.focal_length_ratio;

#ifdef DEFAULT_ROI_TO_CENTER
  uint32_t roi_width, roi_height;
  roi_width = p_private_data->dcrf_init_params.main.width /
    ROI_SIZE_FACTOR;
  roi_height = p_private_data->dcrf_init_params.main.height /
    ROI_SIZE_FACTOR;
  if (p_client->stream_crop_valid && p_client->stream_crop.x &&
    p_client->stream_crop.y && p_client->stream_crop.crop_out_x &&
    p_client->stream_crop.crop_out_y) {
    if ((roi_width > p_client->stream_crop.crop_out_x) ||
      (roi_height > p_client->stream_crop.crop_out_y)) {
      roi_width = p_client->stream_crop.width_map;
      roi_height = p_client->stream_crop.height_map;
    }
    p_dcrf_runtime_param->roi_of_main.pos.x = p_client->stream_crop.x +
      ((p_client->stream_crop.crop_out_x / 2) - (roi_width / 2));
    p_dcrf_runtime_param->roi_of_main.pos.y = p_client->stream_crop.y +
      ((p_client->stream_crop.crop_out_y / 2) - (roi_height / 2));
  } else {
    p_dcrf_runtime_param->roi_of_main.pos.x =
      (p_private_data->dcrf_init_params.main.width / 2) -
      (roi_width / 2);
    p_dcrf_runtime_param->roi_of_main.pos.y =
      (p_private_data->dcrf_init_params.main.height / 2) -
      (roi_height / 2);
  }
  p_dcrf_runtime_param->roi_of_main.size.width = roi_width;
  p_dcrf_runtime_param->roi_of_main.size.height = roi_height;
#else
  p_dcrf_runtime_param->roi_of_main.size.width =
    p_private_data->cur_dciaf_cfg.roi.width *
    p_private_data->dcrf_ratio;
  p_dcrf_runtime_param->roi_of_main.size.height=
    p_private_data->cur_dciaf_cfg.roi.height *
    p_private_data->dcrf_ratio;

  int32_t shift = p_private_data->cur_dciaf_cfg.roi.left -
    p_dcrf_runtime_param->fov_params_main[3].offset_x;
  p_dcrf_runtime_param->roi_of_main.pos.x = shift > 0 ?
    shift * p_private_data->dcrf_ratio : 0;

  shift = p_private_data->cur_dciaf_cfg.roi.top -
    p_dcrf_runtime_param->fov_params_main[3].offset_y;
  p_dcrf_runtime_param->roi_of_main.pos.y = shift > 0 ?
    shift * p_private_data->dcrf_ratio : 0;
#endif

  p_dcrf_runtime_param->af_fps = p_private_data->dcrf_fps;
  /*hack lens zoom ratio */
  if (p_dcrf_runtime_param->lens_zoom_ratio == 0.0f) {
    p_dcrf_runtime_param->lens_zoom_ratio = 1.0f;
  }

}

/**
 * Function: module_dcrf_client_update_meta
 *
 * Description: This function is used to called when the base
 *                       module updates the metadata
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @p_meta: pointer to the image meta
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_dcrf_client_update_meta(imgbase_client_t *p_client,
  img_meta_t *p_meta)
{
  int rc = IMG_SUCCESS;
  module_dcrf_private_data_t *p_private_data =
    (module_dcrf_private_data_t *)p_client->p_private_data;

  IDBG_LOW("%s:%d] ", __func__, __LINE__);
  module_dcrf_fill_runtime_params(p_client);

  /* Add DCRF runtime cfg to meta data */
  IDBG_MED("%s:%d] Set meta for DCRF", __func__, __LINE__);
  rc = img_set_meta(p_meta, IMG_META_DCRF_RUNTIME_PARAM,
    &p_private_data->dcrf_runtime_param);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
  }

  return rc;
}

/**
 * Function: module_dcrf_deinit
 *
 * Description: This function is used to deinit DCRF module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_dcrf_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}

/** module_dcrf_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize DCRF module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_dcrf_init(const char *name)
{
  IDBG_HIGH("%s:%d] E %s", __func__, __LINE__, name);
   return module_imgbase_init(name,
    IMG_COMP_DUAL_FRAME_PROC,
    "qti.dual_frameproc",
    NULL,
    &g_caps,
    "libmmcamera_dcrf_lib.so",
    CAM_QCOM_FEATURE_DCRF,
    &g_params);
}
