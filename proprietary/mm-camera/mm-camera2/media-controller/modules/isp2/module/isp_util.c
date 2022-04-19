 /* isp_util.c
 *
 * Copyright (c) 2012-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
/* std headers */
#include <stdio.h>

/* kernel headers */
#include <media/msmb_ispif.h>

/* mctl headers */
#include "mct_object.h"

/* isp headers */
#include "isp_module.h"
#include "isp_resource.h"
#include "isp_log.h"
#include "isp_util.h"
#include "isp_hw_update_util.h"

#include "camera_dbg.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COMMON, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COMMON, fmt, ##args)

/* 2= rolloff, 4= gamma, 8= linearization, 16=la , 32= ltm, 64= abf
   128=pedestal 256=gic 512=gtm BF_STATS (1 << 17 | 1 << 18)*/
#define ISP_PRINT_DMI_METADUMP 0

#define PPROC_MAX_SCALERS_RATIO   16 // todo query by PPROC

#define IS_SNAP_STREAM(type) ( \
  ((type) == CAM_STREAM_TYPE_SNAPSHOT) ? TRUE : FALSE)

#define IS_PP_MASK_FIT(mask1, mask2) ( \
  (mask1 == CAM_QCOM_FEATURE_NONE && mask2 == CAM_QCOM_FEATURE_NONE) || \
  (mask1 != CAM_QCOM_FEATURE_NONE && mask2 != CAM_QCOM_FEATURE_NONE))

#define IS_PP_MASK_NONE(mask1, mask2) ( \
  (mask1 == CAM_QCOM_FEATURE_NONE && mask2 == CAM_QCOM_FEATURE_NONE))


#define QCIF_WIDTH 176
#define QCIF_HEIGHT 144

#define MAX_STREAMS_PER_PORT 4
#define PRINT_STREAM_MAPPING 0

/* configurable register dump range*/
static uint32_t reg_dump_range[][2] = {
  {0x0, 0xbb0}, /*1st dump range*/
  {0x0, 0x0}, /*2nd dump range*/
  {0x0, 0x0},
  {0x0, 0x0},
  {0x0, 0x0},
  {0x0, 0x0},
  };

/** isp_util_compare_sessionid_session_param:
 *
 *  @data1: session params handle
 *  @data2: session id
 *
 *  Returns TRUE if session id of session params matches with
 *  requested session id, FALSE otherwise
 **/
boolean isp_util_compare_sessionid_from_session_param(void *data1, void *data2)
{
  isp_session_param_t *session_param = (isp_session_param_t *)data1;
  uint32_t            *session_id = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }

  if (session_param->session_id == *session_id) {
    return TRUE;
  }

  return FALSE;
}

/** isp_util_update_stream_info_dims_for_rotation:
 *    @stream_info: stream info
 *
 *  This function runs in MCTL thread context.
 *
 *  This function swaps width and height in case of 90/270 degree rotation.
 *
 *  Return: 0 - Success
 *         -1 - Port is connected to its max number of streams
 **/
void isp_util_update_stream_info_dims_for_rotation(
    mct_stream_info_t *stream_info)
{
    switch (stream_info->pp_config.rotation) {
      case ROTATE_90:
      case ROTATE_270: {
        int32_t swap;

        swap = stream_info->dim.width;
        stream_info->dim.width = stream_info->dim.height;
        stream_info->dim.height = swap;
      }
        break;
      case ROTATE_0:
      case ROTATE_180:
      default:
        break;
    }
}

/** isp_util_compare_stream_type_from_stream_param:
 *
 *  @data1: stream params handle
 *  @data2: stream_type
 *
 *  Returns TRUE if identity of stream params matches with
 *  requested identity, FALSE otherwise
 **/
boolean isp_util_compare_stream_type_from_stream_param(void *data1,
  void *data2)
{
  isp_stream_param_t *stream_param = (isp_stream_param_t *)data1;
  cam_stream_type_t  *stream_type = (cam_stream_type_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }

  if (stream_param->stream_info.stream_type == *stream_type) {
    return TRUE;
  }

  return FALSE;
}

/** isp_util_compare_identity_from_stream_param:
 *
 *  @data1: stream params handle
 *  @data2: identity
 *
 *  Returns TRUE if identity of stream params matches with
 *  requested identity, FALSE otherwise
 **/
boolean isp_util_compare_identity_from_stream_param(void *data1,
  void *data2)
{
  isp_stream_param_t *stream_param = (isp_stream_param_t *)data1;
  uint32_t            *identity = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }

  if (stream_param->stream_info.identity == *identity) {
    return TRUE;
  }

  return FALSE;
}

/** isp_util_compare_identity:
 *
 *  @data1: identity1
 *  @data2: identity2
 *
 *  Return TRUE if identity matches, FALSE otherwise
 **/
boolean isp_util_compare_identity(void *data1, void *data2)
{
  uint32_t *identity1 = (uint32_t *)data1;
  uint32_t *identity2 = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  if (*identity1 == *identity2) {
    return TRUE;
  }

  return FALSE;
}

/** isp_util_compare_non_identity:
 *
 *  @data1: identity1
 *  @data2: identity2
 *
 *  Return TRUE if identity does not match, FALSE otherwise
 **/
boolean isp_util_compare_non_identity(void *data1, void *data2)
{
  uint32_t                  i = 0;
  uint32_t                 *identity1 = (uint32_t *)data1;
  isp_util_identity_list_t *identity_list = (isp_util_identity_list_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }

  for (i = 0; i < identity_list->num_identity; i++) {
    ISP_DBG("ide %x list ide %x", *identity1, identity_list->identity[i]);
    if (*identity1 == identity_list->identity[i]) {
      return FALSE;
    }
  }

  return TRUE;
}

/** isp_util_compare_non_hal_stream_type:
 *
 *  @data: stream_param handle
 *  @user_data: NULL
 *
 *  If current stream is HAL created stream, return TRUE,
 *  else return FALSE
 **/
boolean isp_util_compare_hal_stream_type(
    void *data,
    void *userdata __unused)
{
  isp_stream_param_t *stream_param = (isp_stream_param_t *)data;

  if (!stream_param) {
    ISP_ERR("failed: stream_param %p", NULL);
    return FALSE;
  }

  if (stream_param->stream_info.stream_type != CAM_STREAM_TYPE_PARM) {
    return TRUE;
  }

  return FALSE;
}

/** isp_util_compare_stream_type:
 *
 *  @data: stream_param handle
 *  @user_data: NULL
 *
 *  If current stream is HAL created stream, return TRUE,
 *  else return FALSE
 **/
boolean isp_util_compare_stream_type(void *data, void *user_data)
{
  isp_stream_param_t *stream_param = (isp_stream_param_t *)data;
  cam_stream_type_t  *stream_type = (cam_stream_type_t *)user_data;

  if (!stream_param) {
    ISP_ERR("failed: stream_param %p", NULL);
    return FALSE;
  }

  if (stream_param->stream_info.stream_type == *stream_type) {
    return TRUE;
  }

  return FALSE;
}

/** isp_util_get_session_params:
 *
 *  @module: mct module handle
 *  @session_id: requested session id
 *  @session_param: handle to store session param
 *
 *  Find session param corresponding to requested session id
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_get_session_params(mct_module_t *module, uint32_t session_id,
  isp_session_param_t **session_param)
{
  boolean             ret = TRUE;
  isp_t               *isp = NULL;
  mct_list_t          *l_session_params = NULL;

  if (!module || !session_param) {
    ISP_ERR("failed: module %p session_param %p", module, session_param);
    return FALSE;
  }

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  //PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));
  l_session_params = mct_list_find_custom(isp->l_session_params, &session_id,
    isp_util_compare_sessionid_from_session_param);
  if (!l_session_params) {
    ISP_ERR("failed: l_session_params %p for session %x", l_session_params,
      session_id);
    //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
    return FALSE;
  }

  *session_param = (isp_session_param_t *)l_session_params->data;
  if (!(*session_param)) {
    ISP_ERR("failed: session_param %p", *session_param);
    //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
    return FALSE;
  }
  //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  return ret;
}

/** isp_util_set_isp_frame_skip:
 *
 *  @session_param: handle to session param
 *  @isp_split_out_info: risp split info
 *
 *  Find isp frame skip needed
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_set_isp_frame_skip(mct_module_t *module,
  isp_session_param_t             *session_param,
  uint8_t                         *frame_skip,
  uint32_t                         hw_id,
  uint32_t                         stream_identity)
{
  boolean                   ret = TRUE;
  mct_event_t               event;
  isp_private_event_t       private_event;

  ISP_DBG("E");
  if (!session_param || !frame_skip || !module) {
    ISP_ERR("failed: session_param %p frame_skip %p module %p", session_param,
      frame_skip, module);
    return FALSE;
  }

  /*split case fill in isp split info base on ispif stripe info*/
  memset(&event, 0, sizeof(event));
  memset(&private_event, 0, sizeof(private_event));
  memset(frame_skip, 0, sizeof(*frame_skip));

#ifdef _ANDROID_
  int                       disable_skip = 0;
  int                       exp_time;
  char                      value[PROPERTY_VALUE_MAX];
  stats_get_data_t          stats_get;
  property_get("persist.vendor.camera.isp.noskip", value, "0");
  disable_skip = atoi(value);
  if (disable_skip) {
    /* ISP frame skip needs to be disbaled based on exp value*/
    property_get("persist.vendor.camera.isp.exptime", value, "200");
    exp_time = atoi(value);
    /*Get AEC Data*/
    memset(&stats_get, 0, sizeof(stats_get));
    event.type = MCT_EVENT_MODULE_EVENT;
    event.identity = stream_identity;
    event.direction = MCT_EVENT_DOWNSTREAM;
    event.u.module_event.type = MCT_EVENT_MODULE_STATS_GET_DATA;
    event.u.module_event.module_event_data = (void *)&stats_get;
    /* Forward to stats port */
    ret = isp_util_forward_event_downstream_to_type(module, &event,
      MCT_PORT_CAPS_STATS);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event_downstream_to_type type %d",
        event.type);
    }
    if ((stats_get.aec_get.exp_time * 1000) > (float)exp_time) {
      ISP_INFO("Disabling ISP SKIP as exposure time is above threshold");
      *frame_skip = NO_SKIP;
      return ret;
    }
    memset(&event, 0, sizeof(event));
  }
#endif
  private_event.type = ISP_PRIVATE_REQUEST_FRAME_SKIP;
  private_event.data = frame_skip;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = stream_identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.u.module_event.module_event_data = &private_event;

  ret = isp_util_forward_event_to_internal_pipeline(
    session_param, &event, hw_id);
  if (ret == FALSE) {
    ISP_ERR("failed: ISP_PRIVATE_REQUEST_STREAM_SPLIT_INFO hw_id %d",
      hw_id);
    *frame_skip = NO_SKIP;
  }
  ISP_DBG("ISP_PRIVATE_REQUEST_FRAME_SKIP %x", *frame_skip);
  return ret;
}

/**
* Function: isp_util_get_offline_meta_buffer
*
* Description: Function used as callback to find
*   metadata buffer wiht corresponding index
*
* Input parameters:
*   @data - MCT stream buffer list
*   @user_data - Pointer to searched buffer index
*
* Return values:
*     true/false
*
* Notes: none
**/
static boolean isp_util_get_offline_meta_buffer(void *data, void *user_data)
{
  mct_stream_map_buf_t *buf = NULL;(mct_stream_map_buf_t *)data;
  uint8_t              *buf_index = NULL;(uint8_t *)user_data;

  if (!data || !user_data) {
    ISP_ERR("failed: NULL pointer! %p %p", data, user_data);
    return FALSE;
  }

  buf = (mct_stream_map_buf_t *)data;
  buf_index = (uint8_t *)user_data;

  if (buf->buf_type != CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF)
    return FALSE;

  return ((uint8_t)buf->buf_index == *buf_index);
}

/** isp_util_extract_metadata_buffer:
 *
 */
metadata_buffer_t *isp_util_extract_metadata_buffer(mct_module_t *module,
  isp_session_param_t *session_param, isp_stream_param_t *stream_param,
  cam_stream_parm_buffer_t *stream_parm_buf)
{
  boolean               ret = TRUE;
  mct_list_t           *temp_list = NULL;
  mct_stream_info_t    *mct_stream_info = NULL;
  metadata_buffer_t    *metadata_buf = NULL;
  mct_stream_map_buf_t *buff_holder = NULL;
  ISP_HIGH("fe_dbg extract meta buf stream_id %x", stream_param->stream_info.identity);

  if (!module || !session_param || !stream_param || !stream_parm_buf) {
    ISP_ERR("failed: fe_dbg NULL pointer %p %p %p %p", module,
      session_param, stream_param, stream_parm_buf);
    return FALSE;
  }

  mct_stream_info = &stream_param->stream_info;

  if (stream_param->stream_info.reprocess_config.pp_type ==
      CAM_ONLINE_REPROCESS_TYPE) {
    ISP_DBG("online mode stream id = %d",
      stream_parm_buf->reprocess.meta_stream_handle);
    metadata_buf = (metadata_buffer_t *)mct_module_get_buffer_ptr(
      stream_parm_buf->reprocess.meta_buf_index,
      module, session_param->session_id,
      stream_parm_buf->reprocess.meta_stream_handle);
  } else {
    ISP_DBG("offline mode index %d mct_stream_id %d list %p",
    stream_parm_buf->reprocess.meta_buf_index,
    mct_stream_info->identity, mct_stream_info->stream->buffers.img_buf);
     temp_list = mct_list_find_custom(mct_stream_info->stream->buffers.img_buf,
       &stream_parm_buf->reprocess.meta_buf_index,
       isp_util_get_offline_meta_buffer);
     if (temp_list && temp_list->data) {
       buff_holder = temp_list->data;
       metadata_buf = buff_holder->buf_planes[0].buf;
     }
  }

  if (metadata_buf == NULL) {
      ISP_ERR("failed: fe_dbg metadata NULL\n");
      return NULL;
  }

  return metadata_buf;
}

/** isp_util_prepare_offline_cfg_data:
 *
 *  @session_param: handle to session param
 *  @fetch_eng_cfg_data: fetch engine cfg data
 *
 *  set fetch pipeline setting
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_prepare_offline_cfg_data(
  isp_session_param_t *session_param,
  metadata_buffer_t *metadata, isp_fetch_eng_cfg_data_t *fetch_eng_cfg_data,
  iface_fetch_engine_cfg_t *iface_fetch_engine_cfg)
{
  boolean ret = TRUE;
  mct_stream_session_metadata_info *session_meta = NULL;
  mct_bus_msg_sensor_metadata_t    *sensor_meta = NULL;
  stats_get_data_t                 *stats_metadata = NULL;
  cam_awb_params_t                 *awb_meta = NULL;
  mct_bus_msg_awb_immediate_t      *awb_msg = NULL;

  if (!session_param || !metadata || !fetch_eng_cfg_data ||
      !iface_fetch_engine_cfg) {
    ISP_ERR("failed: NULL pointer %p %p %p %p",
      session_param, metadata, fetch_eng_cfg_data,
      iface_fetch_engine_cfg);
    return FALSE;
  }
  session_meta = (mct_stream_session_metadata_info *)
    POINTER_OF_META(CAM_INTF_META_PRIVATE_DATA, metadata);

  if (!session_meta) {
    ISP_ERR("failed: NULL pointer %p",
      session_meta);
    return FALSE;
  }

  awb_msg = (mct_bus_msg_awb_immediate_t *)
    session_meta->stats_awb_data.private_data;
  if (!awb_msg) {
    ISP_ERR("failed: NULL pointer %p",
      awb_msg);
    return FALSE;
  }

  awb_meta = (cam_awb_params_t *)&(awb_msg->awb_info);
  if (!awb_meta) {
    ISP_ERR("failed: NULL pointer %p",
      awb_meta);
    return FALSE;
  }
  /* if no special chromatxi required
       Fill in chromatix params from meta data*/
  if (fetch_eng_cfg_data->is_chromatix_overwrite == FALSE) {
    ISP_DBG("use online header");
    sensor_meta = &session_meta->sensor_data;
    fetch_eng_cfg_data->offline_chromatix.chromatixComPtr =
      sensor_meta->common_chromatix_ptr;
    fetch_eng_cfg_data->offline_chromatix.chromatixPtr    =
      sensor_meta->chromatix_ptr;
    fetch_eng_cfg_data->offline_chromatix.chromatixCppPtr =
      sensor_meta->cpp_chromatix_ptr;
    fetch_eng_cfg_data->offline_chromatix.chromatix3APtr  =
      sensor_meta->a3_chromatix_ptr;
  }  else {
    ISP_DBG("use offline header");
    fetch_eng_cfg_data->offline_chromatix.chromatixComPtr =
      session_param->fetch_eng_cfg_data.offline_chromatix.chromatixComPtr;
    fetch_eng_cfg_data->offline_chromatix.chromatixPtr    =
      session_param->fetch_eng_cfg_data.offline_chromatix.chromatixPtr;
    fetch_eng_cfg_data->offline_chromatix.chromatixCppPtr =
      session_param->fetch_eng_cfg_data.offline_chromatix.chromatixCppPtr;
    fetch_eng_cfg_data->offline_chromatix.chromatix3APtr  =
      session_param->fetch_eng_cfg_data.offline_chromatix.chromatix3APtr;
  }

  stats_metadata =
    (stats_get_data_t *)&session_meta->stats_aec_data.private_data;
  if (stats_metadata) {
    fetch_eng_cfg_data->aec_update.lux_idx =
      stats_metadata->aec_get.lux_idx;
    fetch_eng_cfg_data->aec_update.real_gain =
      stats_metadata->aec_get.real_gain[0];
    fetch_eng_cfg_data->aec_update.total_drc_gain =
      stats_metadata->aec_get.total_drc_gain;
    fetch_eng_cfg_data->aec_update.color_drc_gain =
      stats_metadata->aec_get.color_drc_gain;
    fetch_eng_cfg_data->aec_update.gtm_ratio =
      stats_metadata->aec_get.gtm_ratio;
    fetch_eng_cfg_data->aec_update.ltm_ratio =
      stats_metadata->aec_get.ltm_ratio;
    fetch_eng_cfg_data->aec_update.la_ratio =
       stats_metadata->aec_get.la_ratio;
    fetch_eng_cfg_data->aec_update.gamma_ratio =
       stats_metadata->aec_get.gamma_ratio;
    fetch_eng_cfg_data->aec_update.hdr_sensitivity_ratio =
      stats_metadata->aec_get.hdr_sensitivity_ratio;
    fetch_eng_cfg_data->aec_update.hdr_exp_time_ratio =
      stats_metadata->aec_get.hdr_exp_time_ratio;
    fetch_eng_cfg_data->aec_update.sensor_gain =
      stats_metadata->aec_get.real_gain[0];
    ISP_DBG("OFFDRC gain %f lux idx %f real gain %f",
      fetch_eng_cfg_data->aec_update.total_drc_gain,
      fetch_eng_cfg_data->aec_update.lux_idx,
      fetch_eng_cfg_data->aec_update.real_gain);
  }

  /*fill in AWB udpate from meta*/
  memcpy(&fetch_eng_cfg_data->awb_update,
    &session_meta->isp_stats_awb_data, sizeof(awb_update_t));

  fetch_eng_cfg_data->awb_update.color_temp = awb_meta->cct_value;
  fetch_eng_cfg_data->awb_update.gain.g_gain = awb_meta->rgb_gains.g_gain;
  fetch_eng_cfg_data->awb_update.gain.b_gain = awb_meta->rgb_gains.b_gain;
  fetch_eng_cfg_data->awb_update.gain.r_gain = awb_meta->rgb_gains.r_gain;

  fetch_eng_cfg_data->awb_update.ccm_update.awb_ccm_enable =
    awb_meta->ccm_update.awb_ccm_enable;
  fetch_eng_cfg_data->awb_update.ccm_update.hard_awb_ccm_flag=
    awb_meta->ccm_update.hard_awb_ccm_flag;
  fetch_eng_cfg_data->awb_update.ccm_update.ccm_update_flag =
    awb_meta->ccm_update.ccm_update_flag;
  memcpy(fetch_eng_cfg_data->awb_update.ccm_update.ccm,
    awb_meta->ccm_update.ccm, sizeof(float)*AWB_NUM_CCM_ROWS*AWB_NUM_CCM_COLS);
  memcpy(fetch_eng_cfg_data->awb_update.ccm_update.ccm_offset,
    awb_meta->ccm_update.ccm_offset, sizeof(float)*AWB_NUM_CCM_ROWS);

  /* fill in needed sensor output info!!*/
  fetch_eng_cfg_data->sensor_out_info.request_crop.first_pixel = 0;
  fetch_eng_cfg_data->sensor_out_info.request_crop.last_pixel =
    iface_fetch_engine_cfg->input_dim.width - 1;
  fetch_eng_cfg_data->sensor_out_info.request_crop.first_line = 0;
  fetch_eng_cfg_data->sensor_out_info.request_crop.last_line =
    iface_fetch_engine_cfg->input_dim.height - 1;

  fetch_eng_cfg_data->sensor_out_info.dim_output.width =
    iface_fetch_engine_cfg->input_dim.width ;
  fetch_eng_cfg_data->sensor_out_info.dim_output.height =
    iface_fetch_engine_cfg->input_dim.height;
  fetch_eng_cfg_data->sensor_out_info.fmt =
    iface_fetch_engine_cfg->input_fmt;

  ISP_HIGH("dump_offline input stream dim W %d H %d",
    iface_fetch_engine_cfg->input_dim.width ,
    iface_fetch_engine_cfg->input_dim.height);
  ISP_HIGH("dump_offline sensor_meta lux %f, real_gain %f",
    fetch_eng_cfg_data->aec_update.lux_idx,
    fetch_eng_cfg_data->aec_update.real_gain);
  ISP_HIGH("dump_offline AWB: color_temp %d g_gain %f, b_gain %f, r_gain %f",
    fetch_eng_cfg_data->awb_update.color_temp,
    fetch_eng_cfg_data->awb_update.gain.g_gain,
    fetch_eng_cfg_data->awb_update.gain.b_gain,
    fetch_eng_cfg_data->awb_update.gain.r_gain);

  return ret;
}
/** isp_util_set_offline_cfg_event:
 *
 *  @session_param: handle to session param
 *  @fetch_eng_cfg_data: fetch engine cfg data
 *
 *  set fetch pipeline setting
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_set_offline_cfg_event(isp_t *isp,
  isp_session_param_t *session_param,
  isp_fetch_eng_cfg_data_t *fetch_eng_cfg_data,
  isp_hw_id_t hw_id, uint32_t identity)
{
  boolean                ret = TRUE;
  mct_event_t            internel_event;
  stats_update_t         stats_update;
  isp_pipeline_t        *pipeline;
  isp_private_event_t    private_event;

  memset(&internel_event, 0, sizeof(internel_event));

  internel_event.type = MCT_EVENT_MODULE_EVENT;
  internel_event.identity = identity;
  internel_event.direction = MCT_EVENT_DOWNSTREAM;

  ISP_DBG("dump_offline send set stream config ... ");
  /* forward set stream_config event*/
  internel_event.u.module_event.type = MCT_EVENT_MODULE_SET_STREAM_CONFIG;
  internel_event.u.module_event.module_event_data =
    (void *)&fetch_eng_cfg_data->sensor_out_info;
  ret = isp_util_forward_event_to_internal_pipeline(session_param,
    &internel_event, hw_id);
  if (ret == FALSE) {
    ISP_ERR("failed: ret %d", ret);
  }

  ISP_DBG("dump_offline send offline overwrite ... ");
  /* disable trigger enable if special chromatix needed */
  /*internel_event.u.module_event.type =
    MCT_EVENT_MODULE_ISP_OFFLINE_CONFIG_OVERWRITE;
  internel_event.u.module_event.module_event_data =
    (void *)fetch_eng_cfg_data;
  ret = isp_util_forward_event_to_internal_pipeline(session_param,
    &internel_event, hw_id);
  if (ret == FALSE) {
    ISP_ERR("failed: ret %d", ret);
  }*/

  ISP_DBG("dump_offline send chromatix ptr event ... ");
  /* forward set chromatix event*/
  internel_event.u.module_event.type = MCT_EVENT_MODULE_SET_CHROMATIX_PTR;
  internel_event.u.module_event.module_event_data =
    (void *)&fetch_eng_cfg_data->offline_chromatix;
  ret = isp_util_forward_event_to_internal_pipeline(session_param,
    &internel_event, hw_id);
  if (ret == FALSE) {
    ISP_ERR("failed: ret %d", ret);
  }

  memset(&private_event, 0, sizeof(private_event));
  private_event.type = ISP_PRIVATE_CURRENT_SUBMOD_ENABLE;

  internel_event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  internel_event.u.module_event.module_event_data = (void *)&private_event;
  pipeline = isp->isp_resource.isp_resource_info[hw_id].isp_pipeline;
  private_event.data = pipeline->submod_enable;
  ret = isp_util_forward_event_to_internal_pipeline(session_param,
    &internel_event, hw_id);
  if (ret == FALSE) {
    ISP_ERR("failed: ret %d", ret);
  }

  return ret;
}

boolean isp_util_forward_stats_update_to_pipeline(
  isp_session_param_t *session_param,
  isp_hw_id_t hw_id, uint32_t identity)
{
  boolean                ret = TRUE;
  mct_event_t            internel_event;
  stats_update_t         stats_update;
  isp_pipeline_t        *pipeline;
  isp_private_event_t    private_event;

  memset(&internel_event, 0, sizeof(internel_event));

  internel_event.type = MCT_EVENT_MODULE_EVENT;
  internel_event.identity = identity;
  internel_event.direction = MCT_EVENT_DOWNSTREAM;

  ISP_DBG("dump_offline send AWB udpate event ... ");
  /* forward AWB update event*/
  PTHREAD_MUTEX_LOCK(&session_param->mutex);
  stats_update.awb_update = session_param->fetch_eng_cfg_data.awb_update;
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  if (stats_update.awb_update.gain.r_gain != 0 ||
    stats_update.awb_update.gain.b_gain != 0 ||
    stats_update.awb_update.gain.g_gain != 0 ||
    stats_update.awb_update.color_temp != 0) {
    internel_event.u.module_event.type = MCT_EVENT_MODULE_STATS_AWB_UPDATE;
    internel_event.u.module_event.module_event_data =
      (void *)&stats_update;
    ret = isp_util_forward_event_to_internal_pipeline(session_param,
      &internel_event, hw_id);
    if (ret == FALSE) {
     ISP_ERR("failed: ret %d", ret);
    }
  } else {
    ISP_ERR("Invalid AWB stats");
    ret = FALSE;
    return ret;
  }

    ISP_DBG("dump_offline send AEC udpate event ... ");
  /* forward AEC udpate event*/
  PTHREAD_MUTEX_LOCK(&session_param->mutex);
  stats_update.aec_update = session_param->fetch_eng_cfg_data.aec_update;
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  if (stats_update.aec_update.lux_idx >= 0 ||
    stats_update.aec_update.sensor_gain != 0) {
    internel_event.u.module_event.type = MCT_EVENT_MODULE_STATS_AEC_UPDATE;
    internel_event.u.module_event.module_event_data =
      (void *)&stats_update;
    ret = isp_util_forward_event_to_internal_pipeline(session_param,
      &internel_event, hw_id);
    if (ret == FALSE) {
      ISP_ERR("failed: ret %d", ret);
    }
  } else {
    ISP_ERR("Invalid AEC stats");
    ret = FALSE;
    return ret;
  }
  return ret;
}

/** isp_util_set_offline_cfg_overwrite:
 *
 *  @session_param: handle to session param
 *  @fetch_eng_cfg_data: fetch engine cfg data
 *
 *  set fetch pipeline setting
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_set_offline_cfg_overwrite(isp_t *isp __unused,
  isp_session_param_t *session_param,
  isp_fetch_eng_cfg_data_t *fetch_eng_cfg_data,
  isp_hw_id_t hw_id, uint32_t identity)
{
  boolean                ret = TRUE;
  mct_event_t            internel_event;
  stats_update_t         stats_update;
  isp_pipeline_t        *pipeline;
  isp_private_event_t    private_event;

  memset(&internel_event, 0, sizeof(internel_event));

  internel_event.type = MCT_EVENT_MODULE_EVENT;
  internel_event.identity = identity;
  internel_event.direction = MCT_EVENT_DOWNSTREAM;

  /* disable trigger enable if special chromatix needed */
  internel_event.u.module_event.type =
    MCT_EVENT_MODULE_ISP_OFFLINE_CONFIG_OVERWRITE;
  internel_event.u.module_event.module_event_data =
    (void *)fetch_eng_cfg_data;
  ret = isp_util_forward_event_to_internal_pipeline(session_param,
    &internel_event, hw_id);
  if (ret == FALSE) {
    ISP_ERR("failed: ret %d", ret);
  }
  return ret;
}

/** isp_util_set_split_output_info_per_stream:
 *
 *  @session_param: handle to session param
 *  @isp_split_out_info: risp split info
 *
 *  Find isp split info
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_set_split_output_info_per_stream(
  isp_session_param_t *session_param, isp_out_info_t *isp_split_out_info,
  uint32_t hw_id, uint32_t stream_identity)
{
  boolean                   ret = TRUE;
  mct_event_t               event;
  isp_private_event_t       private_event;

  if (!session_param || !isp_split_out_info) {
    ISP_ERR("failed: session_param %p stream param %p", session_param,
      isp_split_out_info);
    return FALSE;
  }

  /*split case fill in isp split info base on ispif stripe info*/
  memset(&event, 0, sizeof(event));
  memset(&private_event, 0, sizeof(private_event));
  memset(isp_split_out_info, 0, sizeof(isp_out_info_t));

  private_event.type = ISP_PRIVATE_REQUEST_STREAM_SPLIT_INFO;
  private_event.data = isp_split_out_info;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = stream_identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.u.module_event.module_event_data = &private_event;

  ret = isp_util_forward_event_to_internal_pipeline(
    session_param, &event, hw_id);
  if (ret == TRUE) {
    if (hw_id == ISP_HW_0) {
      isp_split_out_info->stripe_id = ISP_STRIPE_LEFT;
    } else if (hw_id == ISP_HW_1) {
      isp_split_out_info->stripe_id = ISP_STRIPE_RIGHT;
    }

    memset(&event, 0, sizeof(event));
    memset(&private_event, 0, sizeof(private_event));
    private_event.type = ISP_PRIVATE_SET_STREAM_SPLIT_INFO;
    private_event.data = isp_split_out_info;
    event.type = MCT_EVENT_MODULE_EVENT;
    event.identity = stream_identity;
    event.direction = MCT_EVENT_DOWNSTREAM;
    event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
    event.u.module_event.module_event_data = &private_event;

    ret = isp_util_forward_event_to_internal_pipeline(
      session_param, &event, hw_id);
    if (ret == FALSE) {
      ISP_ERR("failed: SET_STREAM_SPLIT_INFO hw_id %d", hw_id);
      memset(isp_split_out_info, 0, sizeof(isp_out_info_t));
    }
  } else {
    ISP_ERR("failed: ISP_PRIVATE_REQUEST_STREAM_SPLIT_INFO hw_id %d",
      hw_id);
    memset(isp_split_out_info, 0, sizeof(isp_out_info_t));
  }

  return ret;
}

/** isp_util_set_offline_split_output_info_per_stream:
 *
 *  @session_param: handle to session param
 *  @isp_split_out_info: risp split info
 *
 *  Find isp split info
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_set_offline_split_output_info_per_stream(
  isp_session_param_t *session_param, isp_out_info_t *isp_split_out_info,
  uint32_t hw_id, uint32_t stream_identity, boolean firstpass)
{
  boolean                   ret = TRUE;
  mct_event_t               event;
  isp_private_event_t       private_event;

  if (!session_param || !isp_split_out_info) {
    ISP_ERR("failed: session_param %p stream param %p", session_param,
      isp_split_out_info);
    return FALSE;
  }

  /*split case fill in isp split info base on ispif stripe info*/
  if (firstpass) {
      memset(&event, 0, sizeof(event));
      memset(&private_event, 0, sizeof(private_event));
      memset(isp_split_out_info, 0, sizeof(isp_out_info_t));

      private_event.type = ISP_PRIVATE_REQUEST_STREAM_SPLIT_INFO;
      private_event.data = isp_split_out_info;
      event.type = MCT_EVENT_MODULE_EVENT;
      event.identity = stream_identity;
      event.direction = MCT_EVENT_DOWNSTREAM;
      event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
      event.u.module_event.module_event_data = &private_event;

      ret = isp_util_forward_event_to_internal_pipeline(
        session_param, &event, hw_id);
  }
  if (ret == TRUE) {
    if (firstpass) {
      isp_split_out_info->stripe_id = ISP_STRIPE_LEFT;
    } else {
      isp_split_out_info->stripe_id = ISP_STRIPE_RIGHT;
    }

    memset(&event, 0, sizeof(event));
    memset(&private_event, 0, sizeof(private_event));
    private_event.type = ISP_PRIVATE_SET_STREAM_SPLIT_INFO;
    private_event.data = isp_split_out_info;
    event.type = MCT_EVENT_MODULE_EVENT;
    event.identity = stream_identity;
    event.direction = MCT_EVENT_DOWNSTREAM;
    event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
    event.u.module_event.module_event_data = &private_event;
    ret = isp_util_forward_event_to_internal_pipeline(
      session_param, &event, hw_id);
    if (ret == FALSE) {
      ISP_ERR("failed: SET_STREAM_SPLIT_INFO hw_id %d", hw_id);
      memset(isp_split_out_info, 0, sizeof(isp_out_info_t));
    }
  } else {
    ISP_ERR("failed: ISP_PRIVATE_REQUEST_STREAM_SPLIT_INFO hw_id %d",
      hw_id);
    memset(isp_split_out_info, 0, sizeof(isp_out_info_t));
  }

  return ret;
}

/** isp_util_forward_offline_event_modules:
 *
 *  @stream_param: handle to stream param
 *  @metadata: handle to metadata
 *  @session_param: handle to session param
 *  @event: mct event
 *  @hw_id: hw_id
 *
 *  forward events to pipeline
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_forward_offline_event_modules(
  isp_stream_param_t *stream_param, metadata_buffer_t *metadata,
  isp_session_param_t *session_param, mct_event_t *event, isp_hw_id_t hw_id)
{
  boolean                   ret = TRUE;
  mct_event_t               internel_event;
  cam_zoom_info_t           *zoom_value = NULL;
  int32_t                   *user_zoom_value = NULL;
  cam_effect_mode_type      *special_effect = NULL;
  int32_t                   *contrast = NULL;
  int32_t                   *saturation = NULL;
  mct_stream_info_t         *offline_stream_info = NULL;
  mct_event_control_parm_t  hal_param;
  int32_t                   *tintless_enable = NULL;
  mct_event_t               copy_event;

  if (!session_param || !metadata || !stream_param) {
    ISP_ERR("failed: session_param %p stream param %p", session_param,
      metadata);
    return FALSE;
  }

  if (session_param->hal_version == CAM_HAL_V3) {
    cam_crop_region_t* crop_region = (cam_crop_region_t *)
      POINTER_OF_META(CAM_INTF_META_SCALER_CROP_REGION, metadata);
    if (crop_region) {
      hal_param.parm_data = crop_region;
      hal_param.type = CAM_INTF_META_SCALER_CROP_REGION;
      isp_util_fill_hal_params(MCT_EVENT_CONTROL_SET_PARM, &internel_event,
        &hal_param, event->identity);
      ret = isp_util_forward_event_to_internal_pipeline(session_param,
        &internel_event, hw_id);
      if (ret == FALSE) {
        ISP_ERR("failed: ret %d", ret);
        return FALSE;
      }
    }
  } else {
    zoom_value = (cam_zoom_info_t *)
    POINTER_OF_META(CAM_INTF_PARM_USERZOOM, metadata);
    /* forward zoom value */
    if (zoom_value) {
      hal_param.parm_data = zoom_value;
      hal_param.type = CAM_INTF_PARM_USERZOOM;
      isp_util_fill_hal_params(MCT_EVENT_CONTROL_SET_PARM, &internel_event,
        &hal_param, event->identity);
      ret = isp_util_forward_event_to_internal_pipeline(session_param,
        &internel_event, hw_id);
      if (ret == FALSE) {
        ISP_ERR("failed: ret %d", ret);
        return FALSE;
      }
    }
  }

  special_effect = (cam_effect_mode_type *)
  POINTER_OF_META(CAM_INTF_PARM_EFFECT, metadata);
  /* forward special effect */
  if (special_effect) {
    hal_param.parm_data = special_effect;
    hal_param.type = CAM_INTF_PARM_EFFECT;
    isp_util_fill_hal_params(MCT_EVENT_CONTROL_SET_PARM, &internel_event,
      &hal_param, event->identity);
    ret = isp_util_forward_event_to_internal_pipeline(session_param,
      &internel_event, hw_id);
    if (ret == FALSE) {
      ISP_ERR("failed: ret %d", ret);
      return FALSE;
    }
  }

  contrast = (int32_t *)
  POINTER_OF_META(CAM_INTF_PARM_CONTRAST, metadata);
  /* forward contrast value */
  if (contrast && *contrast) {
    hal_param.parm_data = contrast;
    hal_param.type = CAM_INTF_PARM_CONTRAST;
    isp_util_fill_hal_params(MCT_EVENT_CONTROL_SET_PARM, &internel_event,
      &hal_param, event->identity);
    ret = isp_util_forward_event_to_internal_pipeline(session_param,
      &internel_event, hw_id);
    if (ret == FALSE) {
      ISP_ERR("failed: ret %d", ret);
      return FALSE;
    }
  }

  saturation = (int32_t *)
  POINTER_OF_META(CAM_INTF_PARM_SATURATION, metadata);
  /* forward saturation value */
  if (saturation && *saturation) {
    hal_param.parm_data = saturation;
    hal_param.type = CAM_INTF_PARM_SATURATION;
    isp_util_fill_hal_params(MCT_EVENT_CONTROL_SET_PARM, &internel_event,
      &hal_param, event->identity);
    ret = isp_util_forward_event_to_internal_pipeline(session_param,
      &internel_event, hw_id);
    if (ret == FALSE) {
      ISP_ERR("failed: ret %d", ret);
      return FALSE;
    }
  }

  tintless_enable = (int32_t *)
  POINTER_OF_META(CAM_INTF_PARM_TINTLESS, metadata);
  /* forward  tintless value */
  if (tintless_enable && *tintless_enable) {
    hal_param.parm_data = tintless_enable;
    hal_param.type = CAM_INTF_PARM_TINTLESS;
    isp_util_fill_hal_params(MCT_EVENT_CONTROL_SET_PARM, &internel_event,
      &hal_param, event->identity);
    ret = isp_util_forward_event_to_internal_pipeline(session_param,
      &internel_event, hw_id);
    if (ret == FALSE) {
      ISP_ERR("failed: ret %d", ret);
      return FALSE;
    }
    if (*tintless_enable) {
      /*Sending Tintless ouput from online*/
      if (session_param->saved_events.
        module_events[ISP_MODULE_EVENT_TINTLESS_ALGO_UPDATE]) {
        copy_event = *(session_param->
          saved_events.module_events[ISP_MODULE_EVENT_TINTLESS_ALGO_UPDATE]);
        copy_event.identity = event->identity;
        memcpy(&session_param->offline_tinltess_params,
          session_param->saved_events.
          module_events[ISP_MODULE_EVENT_TINTLESS_ALGO_UPDATE]->u.
          module_event.module_event_data,
          sizeof(isp_saved_tintless_params_t));
        copy_event.u.module_event.module_event_data =
          &session_param->offline_tinltess_params;
        ret = isp_util_forward_event_to_internal_pipeline(session_param,
          &copy_event, hw_id);
      }
    }
  }

  return ret;
}

/** isp_util_fill_hal_params:
 *
 *  @type: handle to event type
 *  @internel_event: internal event
 *  @hal_param: hal data structure to fill in
 *  @event_identity: event identity
 *
 *  fill hal structure
 *
 **/
void isp_util_fill_hal_params(mct_event_control_type_t type,
  mct_event_t *internel_event, void *hal_param,
  uint32_t event_identity)
{
  memset(internel_event, 0, sizeof(mct_event_t));

    internel_event->type = MCT_EVENT_CONTROL_CMD;
    internel_event->identity = event_identity;
    internel_event->direction = MCT_EVENT_DOWNSTREAM;
    internel_event->u.ctrl_event.type = type;
    internel_event->u.ctrl_event.control_event_data =
    (void *)hal_param;

}

/** isp_util_get_session_stream_params:
 *
 *  @module: mct module handle
 *  @identity: requested identity
 *  @session_param: handle to store session param
 *  @stream_param: handle to store stream param
 *
 *  Find session param and stream param corresponding to
 *  requested identity
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_get_session_stream_params(mct_module_t *module,
  uint32_t identity, isp_session_param_t **session_param,
  isp_stream_param_t **stream_param)
{
  boolean     ret = TRUE;
  uint32_t    session_id = 0;
  isp_t      *isp = NULL;
  mct_list_t *l_session_params = NULL;
  mct_list_t *l_stream_params = NULL;

  if (!module || !session_param || !stream_param) {
    ISP_ERR("failed: module %p session_param %p stream param %p", module,
      session_param, stream_param);
    return FALSE;
  }

  session_id = ISP_GET_SESSION_ID(identity);

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  //PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));
  l_session_params = mct_list_find_custom(isp->l_session_params, &session_id,
    isp_util_compare_sessionid_from_session_param);
  if (!l_session_params) {
    ISP_ERR("failed: l_session_params %p for session %d", l_session_params,
      session_id);
    //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
    return FALSE;
  }

  *session_param = (isp_session_param_t *)l_session_params->data;
  if (!(*session_param)) {
    ISP_ERR("failed: session_param %p", *session_param);
    //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
    return FALSE;
  }

  l_stream_params = mct_list_find_custom((*session_param)->l_stream_params,
    &identity, isp_util_compare_identity_from_stream_param);
  if (!l_stream_params) {
    ISP_ERR("failed: l_stream_params %p for session %d", l_stream_params,
      session_id);
    //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
    return FALSE;
  }

  *stream_param = (isp_stream_param_t *)l_stream_params->data;
  if (!(*stream_param)) {
    ISP_ERR("failed: stream_param %p", *stream_param);
    //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
    return FALSE;
  }

  //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  return ret;
}

/** isp_util_get_isp_version:
 *
 *  @module: isp mct module
 *  @identity: requested identity
 *  @stat_mod_mask: stats module mask
 *
 *  Retrive info which stats modules are enabled
 *
 *  Return TRUE on success and FALSE on failure
 **/

boolean isp_util_get_isp_version(mct_module_t *module,
  uint32_t identity, uint32_t *version)
{
  boolean                         ret = TRUE;
  isp_t                          *isp = NULL;
  isp_session_param_t            *session_param = NULL;
  uint8_t                         num_isp;

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  ret = isp_resource_get_vfe_version(&isp->isp_resource,
    session_param->hw_id[0], version);
  if (ret == FALSE) {
    ISP_ERR("failed to get vfe version info");
    return ret;
  }
  ISP_DBG("isp version %d", *version);
  return ret;
}

/** isp_util_get_stream_params:
 *
 *  @session_param: handle to session param
 *  @identity: requested identity
 *  @stream_param: handle to store stream param
 *
 *  Find session param and stream param corresponding to
 *  requested identity
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_get_stream_params(isp_session_param_t *session_param,
  uint32_t identity, isp_stream_param_t **stream_param)
{
  boolean       ret = TRUE;
  uint32_t      session_id = 0;
  isp_t        *isp = NULL;
  mct_list_t   *l_stream_params = NULL;

  if (!session_param || !stream_param) {
    ISP_ERR("failed: session_param %p stream param %p", session_param,
      stream_param);
    return FALSE;
  }

  session_id = ISP_GET_SESSION_ID(identity);

  l_stream_params = mct_list_find_custom(session_param->l_stream_params,
    &identity, isp_util_compare_identity_from_stream_param);
  if (!l_stream_params) {
    ISP_ERR("failed: l_stream_params %p for session %d", l_stream_params,
      session_id);
    return FALSE;
  }

  *stream_param = (isp_stream_param_t *)l_stream_params->data;
  if (!(*stream_param)) {
    ISP_ERR("failed: stream_param %p", *stream_param);
    return FALSE;
  }

  return ret;
}

/** isp_util_find_port_based_on_identity:
 *
 *  @data1: mct port handle
 *  @data2: identity
 *
 *  Retrieve port based on identity
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_find_port_based_on_identity(void *data1, void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  uint32_t   *identity = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }

  if (mct_list_find_custom(MCT_PORT_CHILDREN(port), identity,
    isp_util_compare_identity)) {
    return TRUE;
  }

  return FALSE;
}

/** isp_util_get_port_from_module:
 *
 *  @module: mct module handle
 *  @port: port to return
 *  @direction: direction of port to be retrieved
 *  @identity: identity to match against
 *
 *  Find port based on direction and identity and return it
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_get_port_from_module(mct_module_t *module,
  mct_port_t **port, mct_port_direction_t direction, uint32_t identity)
{
  mct_list_t *l_port = NULL;

  if (!module || !port) {
    ISP_ERR("failed: module %p sink_port %p", module, port);
    return FALSE;
  }

  /* Reset output param */
  *port = NULL;

  if ((direction != MCT_PORT_SRC) && (direction != MCT_PORT_SINK)) {
    ISP_ERR("failed: invalid direction %d", direction);
    return FALSE;
  }

  if (direction == MCT_PORT_SINK) {
    l_port = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &identity,
      isp_util_find_port_based_on_identity);
    if (!l_port) {
      ISP_ERR("failed: mct_list_find_custom for ide %x", identity);
      return FALSE;
    }
  } else {
    l_port = mct_list_find_custom(MCT_MODULE_SRCPORTS(module), &identity,
      isp_util_find_port_based_on_identity);
    if (!l_port) {
      ISP_ERR("failed: mct_list_find_custom for ide %x", identity);
      return FALSE;
    }
  }

  *port = (mct_port_t *)l_port->data;

  return TRUE;
}

/** isp_util_forward_event:
 *
 *  @port: port on which event is received
 *  @event: event to be forwarded
 *
 *  Forward event upstream / downstream based on event's
 *  direction
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_forward_event(mct_port_t *port, mct_event_t *event)
{
  boolean               ret = TRUE;
  mct_port_t           *out_port = NULL;
  uint32_t              hw_stream = 0;
  mct_list_t           *l_identity = NULL;
  mct_module_t         *module = NULL;
  isp_session_param_t  *session_param = NULL;
  isp_module_ports_t   *isp_ports = NULL;

  if (!port || !event) {
    ISP_ERR("failed: port %p event %p", port, event);
    return FALSE;
  }

  if ((event->direction != MCT_EVENT_UPSTREAM) &&
      (event->direction != MCT_EVENT_DOWNSTREAM)) {
     ISP_ERR("failed: invalid event dir %d", event->direction);
     return FALSE;
  }

  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  isp_ports = &session_param->isp_ports;

  if (event->direction == MCT_EVENT_UPSTREAM) {
    /* Upstream event */
    if (port->direction != MCT_PORT_SRC) {
      if (event->type == MCT_EVENT_MODULE_EVENT) {
        ISP_ERR("failed: mod port dir %d event dir %d ide %x type %d",
          port->direction, event->direction, event->identity,
          event->u.module_event.type);
      } else if (event->type == MCT_EVENT_CONTROL_CMD) {
        ISP_ERR("failed: ctrl port dir %d event dir %d ide %x type %d",
          port->direction, event->direction, event->identity,
          event->u.ctrl_event.type);
      }
      return FALSE;
    }
    out_port = isp_ports->isp_sink_port;
    /* forward event upstream */
    if (out_port && out_port->direction == MCT_PORT_SINK) {
      ret = mct_port_send_event_to_peer(out_port, event);
      if (ret == FALSE) {
        ISP_ERR("failed: mct_port_send_event_to_peer event->type %d",
          event->type);
        return ret;
      }
    }
  } else {
    /* downstream event */
    if (port->direction != MCT_PORT_SINK) {
      if (event->type == MCT_EVENT_MODULE_EVENT) {
        ISP_ERR("failed: mod port dir %d event dir %d ide %x type %d",
          port->direction, event->direction, event->identity,
          event->u.module_event.type);
      } else if (event->type == MCT_EVENT_CONTROL_CMD) {
        ISP_ERR("failed: ctrl port dir %d event dir %d ide %x type %d",
          port->direction, event->direction, event->identity,
          event->u.ctrl_event.type);
      }
      return FALSE;
    }
    /* Forward event to stats port */
    out_port = isp_ports->isp_source_stats_port;
    /* forward event downstream */
    if (out_port && out_port->direction == MCT_PORT_SRC) {
      ret = mct_port_send_event_to_peer(out_port, event);
      if (ret == FALSE) {
        ISP_ERR("failed: mct_port_send_event_to_peer event->type %d",
          event->type);
      }
    }
    /* Forward event to frame port */
    for (hw_stream = 0;
      hw_stream < ARRAY_SIZE(isp_ports->isp_source_frame_port); hw_stream++) {
      out_port = isp_ports->isp_source_frame_port[hw_stream];
      /* forward event downstream */
      if (out_port && out_port->direction == MCT_PORT_SRC) {
        /* Find whether this port is used for requested identity */
        l_identity = mct_list_find_custom(MCT_PORT_CHILDREN(out_port),
          &event->identity, isp_util_compare_identity);
        if (l_identity) {
          ret = mct_port_send_event_to_peer(out_port, event);
          if (ret == FALSE) {
            ISP_ERR("failed: mct_port_send_event_to_peer event->type %d",
              event->type);
          }
        }
      }
    }
  }
  return ret;
}

/** isp_util_forward_event_from_module:
 *
 *  @module: mct module handle
 *  @event: event to be forwarded
 *
 *  Forward event upstream / downstream based on event's
 *  direction
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_forward_event_from_module(mct_module_t *module,
  mct_event_t *event)
{
  boolean               ret = TRUE;
  mct_port_t           *out_port = NULL;
  uint32_t              hw_stream = 0;
  mct_list_t           *l_identity = NULL;
  isp_session_param_t  *session_param = NULL;
  isp_module_ports_t   *isp_ports = NULL;

  if (!module || !event) {
    ISP_ERR("failed: module %p event %p", module, event);
    return FALSE;
  }

  if ((event->direction != MCT_EVENT_UPSTREAM) &&
      (event->direction != MCT_EVENT_DOWNSTREAM)) {
     ISP_ERR("failed: invalid event dir %d", event->direction);
     return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  isp_ports = &session_param->isp_ports;

  if (event->direction == MCT_EVENT_UPSTREAM) {
    out_port = isp_ports->isp_sink_port;
    /* forward event upstream */
    if (out_port && out_port->direction == MCT_PORT_SINK) {
      ret = mct_port_send_event_to_peer(out_port, event);
      if (ret == FALSE) {
        ISP_ERR("failed: mct_port_send_event_to_peer event->type %d",
          event->type);
        return ret;
      }
    }
  } else {
    /* Forward event to stats port */
    out_port = isp_ports->isp_source_stats_port;
    /* forward event downstream */
    if (out_port && out_port->direction == MCT_PORT_SRC) {
      ret = mct_port_send_event_to_peer(out_port, event);
      if (ret == FALSE) {
        ISP_ERR("failed: mct_port_send_event_to_peer event->type %d",
          event->type);
      }
    }
    /* Forward event to frame port */
    for (hw_stream = 0;
      hw_stream < ARRAY_SIZE(isp_ports->isp_source_frame_port); hw_stream++) {
      out_port = isp_ports->isp_source_frame_port[hw_stream];
      /* forward event downstream */
      if (out_port && out_port->direction == MCT_PORT_SRC) {
        /* Find whether this port is used for requested identity */
        l_identity = mct_list_find_custom(MCT_PORT_CHILDREN(out_port),
          &event->identity, isp_util_compare_identity);
        if (l_identity) {
          ret = mct_port_send_event_to_peer(out_port, event);
          if (ret == FALSE) {
            ISP_ERR("failed: mct_port_send_event_to_peer event->type %d",
              event->type);
          }
        }
      }
    }
  }
  return ret;
}

/** isp_util_forward_event_downstream_to_type:
 *
 *  @port: port on which event is received
 *  @event: event to be forwarded
 *  @port_type: type of port on which event shall be forwarded
 *
 *  Forward event downstream based on port type
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_forward_event_downstream_to_type(mct_module_t *module,
  mct_event_t *event, mct_port_caps_type_t port_type)
{
  boolean               ret = TRUE;
  isp_port_data_t      *port_data = NULL;
  mct_port_t           *out_port = NULL;
  uint32_t              hw_stream = 0;
  mct_list_t           *l_identity = NULL;
  isp_session_param_t  *session_param = NULL;
  isp_module_ports_t   *isp_ports = NULL;

  if (!module || !event) {
    ISP_ERR("failed: module %p event %p", module, event);
    return FALSE;
  }

  /* Change event direction to downstream */
  event->direction = MCT_EVENT_DOWNSTREAM;

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  isp_ports = &session_param->isp_ports;

  if (port_type == MCT_PORT_CAPS_STATS) {
    /* Forward event to stats port */
    out_port = isp_ports->isp_source_stats_port;
    /* forward event downstream */
    if (out_port && out_port->direction == MCT_PORT_SRC) {
      ret = mct_port_send_event_to_peer(out_port, event);
      if (ret == FALSE) {
        ISP_ERR("failed: mct_port_send_event_to_peer event->type %d",
          event->type);
      }
    }
  } else if (port_type == MCT_PORT_CAPS_FRAME) {
    /* Forward event to frame port */
    for (hw_stream = 0;
      hw_stream < ARRAY_SIZE(isp_ports->isp_source_frame_port); hw_stream++) {
      out_port = isp_ports->isp_source_frame_port[hw_stream];
      /* forward event downstream */
      if (out_port && out_port->direction == MCT_PORT_SRC) {
        /* Find whether this port is used for requested identity */
        l_identity = mct_list_find_custom(MCT_PORT_CHILDREN(out_port),
          &event->identity, isp_util_compare_identity);
        if (l_identity) {
          ret = mct_port_send_event_to_peer(out_port, event);
          if (ret == FALSE) {
            ISP_ERR("failed: mct_port_send_event_to_peer event->type %d",
              event->type);
          }
        }
      }
    }
  } else {
    ISP_ERR("failed: invalid port type %d", port_type);
    ret = FALSE;
  }
  return ret;
}

/** isp_util_forward_event_downstream_to_all_types:
 *
 *  @port: port on which event is received
 *  @event: event to be forwarded
 *  @port_type: type of port on which event shall be forwarded
 *
 *  Forward event downstream based on port type
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_forward_event_downstream_to_all_types(mct_module_t *module,
  mct_event_t *event)
{
  boolean               ret = TRUE;
  isp_port_data_t      *port_data = NULL;
  mct_port_t           *out_port = NULL;
  uint32_t              hw_stream = 0;
  mct_list_t           *l_identity = NULL;
  isp_session_param_t  *session_param = NULL;
  isp_module_ports_t   *isp_ports = NULL;

  if (!module || !event) {
    ISP_ERR("failed: module %p event %p", module, event);
    return FALSE;
  }

  /* Change event direction to downstream */
  event->direction = MCT_EVENT_DOWNSTREAM;

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  isp_ports = &session_param->isp_ports;

  /* Forward event to stats port */
  out_port = isp_ports->isp_source_stats_port;
  /* forward event downstream */
  if (out_port && out_port->direction == MCT_PORT_SRC) {
    ret = mct_port_send_event_to_peer(out_port, event);
    if (ret == FALSE) {
      if (event->type == MCT_EVENT_CONTROL_CMD) {
        ISP_ERR("failed: mct_port_send_event_to_peer event->type %d",
          event->u.ctrl_event.type);
      } else if (event->type == MCT_EVENT_MODULE_EVENT) {
        ISP_ERR("failed: mct_port_send_event_to_peer event->type %d",
          event->u.module_event.type);
      }
    }
  }

  /* Forward event to frame port */
  for (hw_stream = 0; hw_stream < ARRAY_SIZE(isp_ports->isp_source_frame_port);
    hw_stream++) {
    out_port = isp_ports->isp_source_frame_port[hw_stream];
    /* forward event downstream */
    if (out_port && out_port->direction == MCT_PORT_SRC) {
      /* Find whether this port is used for requested identity */
      l_identity = mct_list_find_custom(MCT_PORT_CHILDREN(out_port),
        &event->identity, isp_util_compare_identity);
      if (l_identity) {
        ret = mct_port_send_event_to_peer(out_port, event);
        if (ret == FALSE) {
          if (event->type == MCT_EVENT_CONTROL_CMD) {
            ISP_ERR("failed: mct_port_send_event_to_peer event->type %d",
              event->u.ctrl_event.type);
          } else if (event->type == MCT_EVENT_MODULE_EVENT) {
            ISP_ERR("failed: mct_port_send_event_to_peer event->type %d",
              event->u.module_event.type);
          }
        }
      }
    }
  }

  return ret;
}

/** isp_util_send_crop_bus_message:
 *
 *  @module: ISP module
 *  @stream_crop: stream_crop to be sent
 *
 *  Send stream_crop to mct bus
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_util_send_crop_bus_message(mct_module_t *module,
  isp_session_param_t *session_param,
  mct_bus_msg_session_crop_info_t *session_crop,
  boolean is_online)
{
  boolean               ret = TRUE;
  mct_bus_msg_t         bus_msg;

  if (!module || !session_param || !session_crop) {
    ISP_ERR("failed: %p %p %p", module, session_param, session_crop);
    return FALSE;
  }
  bus_msg.type = MCT_BUS_MSG_ISP_SESSION_CROP;
  bus_msg.msg = (void *)session_crop;
  bus_msg.sessionid = session_param->session_id;
  bus_msg.size = sizeof(mct_bus_msg_session_crop_info_t);
  if (FALSE == is_online)
    bus_msg.metadata_collection_type= MCT_BUS_OFFLINE_METADATA;
  else
    bus_msg.metadata_collection_type= MCT_BUS_ONLINE_METADATA;


  ret = isp_util_send_metadata_entry(module, (mct_bus_msg_t *)&bus_msg,
    session_param->curr_frame_id);

  return ret;
}

/** isp_util_forward_event_to_internal_pipeline:
 *
 *  @session_param: session param
 *  @event: event to be handled
 *  @hw_id: pipeline to which this event shall be passed
 *
 *  Forward event to pipelines
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_forward_event_to_internal_pipeline(
  isp_session_param_t *session_param, mct_event_t *event, isp_hw_id_t hw_id)
{
  boolean                ret = TRUE;
  mct_port_t            *int_hw_port = NULL;
  isp_stream_param_t    *stream_param = NULL;

  if (!session_param || !event || (hw_id >= ISP_HW_MAX)) {
    ISP_ERR("failed: session_param %p event %p hw id %d", session_param, event,
      hw_id);
    return FALSE;
  }

  ret = isp_util_get_stream_params(session_param, event->identity,
    &stream_param);
  if (ret == FALSE || !stream_param) {
    ISP_ERR("failed: isp_util_get_stream_params ret %d stream param %p", ret,
      stream_param);
    return TRUE;
  }

  if (stream_param->is_pipeline_supported == FALSE) {
    ISP_ERR("isp pipeline not support this stream identity %x",
      event->identity);
    return TRUE;
  }

  int_hw_port = session_param->isp_ports.isp_submod_ports[hw_id];
  ISP_HIGH("DEBUGPORT int_hw_port %p event ide %x", int_hw_port,
    event->identity);
  /* Forward event to pipeline */
  if (int_hw_port && int_hw_port->event_func) {
    ISP_HIGH("DEBUGPORT int_hw_port->event_func");
    ret = int_hw_port->event_func(int_hw_port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: event type %d hw %d", event->type, hw_id);
    }
  }
  return ret;
}

/** isp_util_forward_event_to_all_internal_pipelines:
 *
 *  @session_param: session param
 *  @event: event to be handled
 *
 *  Forward event to pipelines
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_forward_event_to_all_internal_pipelines(
  isp_session_param_t *session_param, mct_event_t *event)
{
  boolean     ret = TRUE;
  isp_hw_id_t hw_index = 0;

  if (!session_param || !event) {
    ISP_ERR("failed: session_param %p event %p", session_param, event);
    return FALSE;
  }

  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    isp_hw_id_t session_hw_id = session_param->hw_id[hw_index];
    if (session_param->offline_num_isp > 1 &&
      session_param->offline_hw_id[0] == session_hw_id)
      continue;
    if (session_hw_id >= ISP_HW_MAX)
      continue;
    ret = isp_util_forward_event_to_internal_pipeline(session_param, event,
      session_hw_id);
    if (ret == FALSE) {
      ISP_ERR("failed: ret %d", ret);
    }
  }

  return ret;
}

/** isp_util_forward_event_to_int_pipeline_to_fill_cfg:
 *
 *  @session_param: session param
 *  @event: event to be handled
 *  @isp_resource: isp_resource
 *  Forward event to pipelines
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_forward_event_to_int_pipeline_to_fill_cfg(
  isp_session_param_t *session_param,
  isp_resource_t      *isp_resource,
  uint32_t             identity,
  cam_stream_type_t    stream_type __unused)
{
  boolean                      ret = TRUE;
  isp_hw_id_t                  hw_index = 0;
  isp_module_enable_LPM_info_t enable_bit_info;
  mct_event_t                  int_event;
  isp_private_event_t          private_event;
  int i;

  memset(&private_event, 0, sizeof(isp_private_event_t));
  private_event.type = ISP_PRIVATE_EVENT_GET_MOD_CFG_MASK;
  private_event.data = (void *)&enable_bit_info;
  private_event.data_size = sizeof(enable_bit_info);

  if (!session_param || !isp_resource) {
    ISP_ERR("failed: session_param %p event %p", session_param, isp_resource);
    return FALSE;
  }

  memset(&enable_bit_info, 0, sizeof(isp_module_enable_info_t));
  memset(&int_event, 0, sizeof(mct_event_t));
  int_event.direction = MCT_EVENT_DOWNSTREAM;
  int_event.identity = identity;
  int_event.type = MCT_EVENT_MODULE_EVENT;
  int_event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  int_event.u.module_event.module_event_data = &private_event;

  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    isp_hw_id_t session_hw_id;
    session_hw_id = session_param->hw_id[hw_index];
    if (session_hw_id >= ISP_HW_MAX)
      continue;

    ret = isp_util_forward_event_to_internal_pipeline(session_param, &int_event,
      session_hw_id);
    if (ret == FALSE) {
      ISP_ERR("failed: ret %d", ret);
      continue;
    }
    session_param->lpm_info.enableLPM = enable_bit_info.enableLPM;
    ISP_DBG("enableLPM = %d", session_param->lpm_info.enableLPM);
     for (i = 0; i < ISP_MOD_MAX_NUM; i++) {
       if (enable_bit_info.cfg_info.submod_enable[i] == 1) {
       ISP_DBG(" enable_bit_info cfb %d index %d",enable_bit_info.cfg_info.submod_enable[i], i);
       ISP_DBG(" enable_bit_info cfb %d ",enable_bit_info.cfg_info.submod_mask[i]);
       }
     }
    ret = isp_resource_update_module_cfg(isp_resource, session_hw_id,
      session_param->hw_update_params.hw_update_list_params[session_hw_id].fd,
      &(enable_bit_info.cfg_info), session_param->sensor_fmt);
    if (ret != TRUE) {
      ISP_ERR("failed: isp_resource_update_module_cfg hw_id %d fd %d, hw_index %d %p\n",
              session_hw_id,
              session_param->hw_update_params.hw_update_list_params[session_hw_id].fd,
              hw_index);
    }
  }

  return TRUE;
}

/** isp_util_forward_event_to_all_pipeline_per_stream:
 *
 *  @data: handle to isp_stream_param_t
 *  @user_data: handle to isp_util_forward_event_info_t
 *
 *  Forward event to all internal pipelines for current
 *  identity
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_forward_event_to_all_pipeline_per_stream(void *data,
   void *user_data)
{
  boolean             ret = TRUE;
  isp_stream_param_t *stream_param = (isp_stream_param_t *)data;
  isp_util_forward_event_info_t *forward_event_info =
    (isp_util_forward_event_info_t *)user_data;
  isp_session_param_t *session_param = NULL;
  mct_event_t         *event = NULL;
  isp_hw_id_t         hw_index = 0;

  if (!stream_param || !forward_event_info) {
    ISP_ERR("failed: stream_param %p forward_event_info %p", stream_param,
      forward_event_info);
    /* Return TRUE since its called as part of mct_list_traverse */
    return TRUE;
  }

  if (stream_param->is_pipeline_supported == FALSE) {
    ISP_ERR("stream id %x is not supported by ISP pipeline",
      stream_param->stream_info.identity);
    return TRUE;
  }
  session_param = forward_event_info->session_param;
  event = forward_event_info->event;
  if (!session_param || !event) {
    ISP_ERR("failed: session_param %p event %p", session_param, event);
    /* Return TRUE since its called as part of mct_list_traverse */
    return TRUE;
  }

  event->identity = stream_param->stream_info.identity;
  for (hw_index = 0; hw_index < session_param->num_isp; hw_index++) {
    ret = isp_util_forward_event_to_internal_pipeline(session_param, event,
      session_param->hw_id[hw_index]);
    if (ret == FALSE) {
      ISP_ERR("failed: ret %d", ret);
    }
  }

  /* Return TRUE since its called as part of mct_list_traverse */
  return TRUE;
}

/** isp_util_forward_event_to_all_streams_in_internal_pipelines:
 *
 *  @session_param: session param
 *  @event: event to be handled
 *
 *  Forward event to pipelines
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_forward_event_to_all_streams_all_internal_pipelines(
  isp_session_param_t *session_param, mct_event_t *event)
{
  boolean                       ret = TRUE;
  uint32_t                      orig_identity = 0;
  isp_util_forward_event_info_t forward_event_info;

  if (!session_param || !event) {
    ISP_ERR("failed: session_param %p event %p", session_param, event);
    return FALSE;
  }

  orig_identity = event->identity;
  forward_event_info.event = event;
  forward_event_info.session_param = session_param;

  mct_list_traverse(session_param->l_stream_params,
    isp_util_forward_event_to_all_pipeline_per_stream, &forward_event_info);

  event->identity = orig_identity;
  return ret;
}

/** isp_util_forward_event_to_session_based_stream_all_int_pipelines:
 *
 *  @session_param: session param
 *  @event: event to be handled
 *
 *  Forward event to pipelines
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_forward_event_to_session_based_stream_all_int_pipelines(
  isp_session_param_t *session_param, mct_event_t *event)
{
  boolean                       ret = TRUE;
  uint32_t                      orig_identity = 0;
  isp_util_forward_event_info_t forward_event_info;
  isp_hw_id_t                   hw_index = 0;

  if (!session_param || !event) {
    ISP_ERR("failed: session_param %p event %p", session_param, event);
    return FALSE;
  }

  orig_identity = event->identity;

  event->identity = session_param->session_based_ide;
  for (hw_index = 0; hw_index < session_param->num_isp; hw_index++) {
    ret = isp_util_forward_event_to_internal_pipeline(session_param, event,
      session_param->hw_id[hw_index]);
    if (ret == FALSE) {
      ISP_ERR("failed: ret %d", ret);
    }
  }

  event->identity = orig_identity;
  return ret;
}

/** isp_util_forward_event_to_stream:
 *
 *  @data: handle to isp_stream_param_t
 *  @user_data: handle to isp_util_forward_event_info_t
 *
 *  Forward event to all internal pipelines for current
 *  identity
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_forward_event_to_stream(void *data, void *user_data)
{
  boolean                        ret = TRUE;
  isp_stream_param_t            *stream_param = (isp_stream_param_t *)data;
  isp_util_forward_event_info_t *forward_event_info =
    (isp_util_forward_event_info_t *)user_data;
  isp_session_param_t           *session_param = NULL;
  mct_event_t                   *event = NULL;
  isp_hw_id_t                    hw_id = 0;

  if (!stream_param || !forward_event_info) {
    ISP_ERR("failed: stream_param %p forward_event_info %p", stream_param,
      forward_event_info);
    /* Return TRUE since its called as part of mct_list_traverse */
    return TRUE;
  }

  if (stream_param->is_pipeline_supported == FALSE) {
    ISP_ERR("stream id %x is not supported by ISP pipeline!",
      stream_param->stream_info.identity);
    return TRUE;
  }

  session_param = forward_event_info->session_param;
  event = forward_event_info->event;
  hw_id = forward_event_info->hw_id;
  if (!session_param || !event) {
    ISP_ERR("failed: session_param %p event %p", session_param, event);
    /* Return TRUE since its called as part of mct_list_traverse */
    return TRUE;
  }

  if (hw_id >= ISP_HW_MAX) {
    ISP_ERR("failed: invalid hw_id %d", hw_id);
    /* Return TRUE since its called as part of mct_list_traverse */
    return TRUE;
  }

  event->identity = stream_param->stream_info.identity;
  ret = isp_util_forward_event_to_internal_pipeline(session_param, event,
    hw_id);
  if (ret == FALSE) {
    ISP_ERR("failed: ret %d", ret);
  }

  /* Return TRUE since its called as part of mct_list_traverse */
  return TRUE;
}

/** isp_util_forward_event_to_all_streams:
 *
 *  @session_param: session param
 *  @event: event to be handled
 *  @hw_id: ISP hw id
 *
 *  Forward event to pipelines
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_forward_event_to_all_streams(
  isp_session_param_t *session_param, mct_event_t *event, isp_hw_id_t hw_id)
{
  boolean                       ret = TRUE;
  uint32_t                      orig_identity = 0;
  isp_util_forward_event_info_t forward_event_info;

  if (!session_param || !event) {
    ISP_ERR("failed: session_param %p event %p", session_param, event);
    return FALSE;
  }

  if (hw_id >= ISP_HW_MAX) {
    ISP_ERR("failed: invalid hw_id %d", hw_id);
    return FALSE;
  }

  orig_identity = event->identity;
  forward_event_info.event = event;
  forward_event_info.session_param = session_param;
  forward_event_info.hw_id = hw_id;

  mct_list_traverse(session_param->l_stream_params,
    isp_util_forward_event_to_stream, &forward_event_info);

  event->identity = orig_identity;
  return ret;
}


/** isp_util_report_meta_dump:
 *
 *  @isp_module: isp module
 *  @session_param : isp session parameters
 *  @event: event
 *  @is_applied: applied update/current update
 *
 *  Report meta_data dump when dump is enabled
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_report_meta_dump(mct_module_t *module,
  isp_session_param_t *session_param)
{
  boolean                      ret = TRUE;
  isp_hw_update_params_t      *hw_update_params = NULL;
  isp_hw_id_t                  hw_id = 0;
  uint8_t                      i = 0;
  uint8_t                      j = 0;

  if (module == NULL || session_param == NULL) {
    ISP_ERR("NULL pointer,isp_module %p, session parm = %p",
      module, session_param);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;

  ISP_HIGH("post isp metadata dump");
  isp_util_prepare_meta_dump(module, session_param);

   /*free all valid dmi table, vfe0 and vfe1 if dual vfe*/
   for (i = 0; i < session_param->num_isp; i++) {
     for (j = 0; j < ISP_META_MAX; j++) {
       if (hw_update_params->hw_update_list_params[session_param->hw_id[i]].
           applied_meta_dump_parms.meta_entry[j].len == 0) {
         continue;
       } else {
         memset(&hw_update_params->hw_update_list_params
           [session_param->hw_id[i]].applied_meta_dump_parms.meta_entry[j],
           0 , sizeof(isp_meta_entry_t));
       }
     }
   }

  return ret;
}

/** isp_util_update_hw_param:
 *
 *  @isp_module: isp module
 *  @session_param : isp session parameters
 *  @event: event
   update hw parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_update_hw_param(mct_module_t *module,
  isp_session_param_t *session_param)
{
  boolean                      ret = TRUE;
  isp_hw_update_params_t      *hw_update_params = NULL;
  isp_parser_params_t         *parser_params = NULL;
  isp_hw_update_list_params_t *hw_update_list_params = NULL;
  isp_hw_id_t                  hw_id = 0;
  uint8_t                      i = 0;
  uint8_t                      j = 0;
  isp_hw_read_info_t           *dmi_info;
  isp_t                       *isp = NULL;
  uint32_t                     num_reg = 0;

  if (module == NULL || session_param == NULL) {
    ISP_ERR("NULL pointer,isp_module %p, session parm = %p",
      module, session_param);
    return FALSE;
  }

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  isp = MCT_OBJECT_PRIVATE(module);

  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;
  if (hw_update_params == NULL) {
    ISP_ERR("NULL pointer, hw_update_params = %p", hw_update_params);
    return FALSE;
  }

  parser_params = &session_param->parser_params;
  PTHREAD_MUTEX_LOCK(&parser_params->mutex);

  /* 1. Save a copy of settings applied in previous frame
        metadata reporting
     2. Save a copy of stats and algo params applied in previous
        frame to parse the stats
   */
  for (i = 0; i < session_param->num_isp; i++) {
    hw_id = session_param->hw_id[i];
    hw_update_list_params = &hw_update_params->hw_update_list_params[hw_id];

    /* Save the settings applied for metadata reporting */
    hw_update_list_params->applied_meta_dump_parms =
      hw_update_list_params->meta_dump_parms;
    hw_update_list_params->applied_frame_meta =
      hw_update_list_params->frame_meta;
    memcpy(&session_param->hdr_data_stored,
      &session_param->hdr_data_cur, sizeof(session_param->hdr_data_cur));

    /* DMI info, being used for gamma alone  */
    for (j = 0; j < ISP_METADUMP_MAX_NUM; j++) {
      dmi_info = &hw_update_list_params->dmi_tbl.dmi_info[j];
      if (dmi_info->read_length == 0) {
        continue;
      } else {
        hw_update_list_params->applied_dmi_tbl.dmi_info[j] = *dmi_info;
        memset(dmi_info, 0, sizeof(isp_hw_read_info_t));
      }
    }

    /* update parser stats params */
    parser_params->stats_params[hw_id] =
      *hw_update_list_params->stats_params;

    /* update parser algo params */
    parser_params->algo_parm[hw_id] =
      hw_update_list_params->algo_parm;
  }

  if (session_param->hw_update_params.metadata_dump_enable == 1) {
    hw_update_list_params = &hw_update_params->hw_update_list_params[session_param->hw_id[0]];
    num_reg = isp->isp_resource.isp_resource_info[session_param->hw_id[0]].
      isp_pipeline->num_register;

      /* prepare metadump: register dump */
    hw_update_list_params->applied_meta_dump_parms.meta_entry[ISP_META_REGISTER_DUMP].dump_type
      = ISP_META_REGISTER_DUMP;
    hw_update_list_params->applied_meta_dump_parms.meta_entry[ISP_META_REGISTER_DUMP].len
      = num_reg * sizeof(uint32_t);
    hw_update_list_params->applied_meta_dump_parms.meta_entry[ISP_META_REGISTER_DUMP].start_addr
      = 0;

    ret = isp_util_read_reg_dump(hw_update_list_params->fd,
      hw_update_list_params->applied_meta_dump_parms.meta_entry[ISP_META_REGISTER_DUMP].isp_meta_dump,
      VFE_READ, num_reg * sizeof(uint32_t), 0);
  }

  PTHREAD_MUTEX_UNLOCK(&parser_params->mutex);

  return ret;
}
/** isp_util_post_cds_update:
 *
 *  @isp_module: isp module
 *  @session_param : isp session parameters
 *  @event: event
 *  @is_applied: applied update/current update
 *
 *  update hw parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_post_cds_update(mct_module_t *module,
  isp_session_param_t *session_param)
{
  boolean                      ret = TRUE;
  isp_hw_update_params_t      *hw_update_params = NULL;
  isp_hw_update_list_params_t *hw_update_list_params = NULL;
  isp_hw_id_t                  hw_id = 0;
  uint8_t                      i = 0;
  uint8_t                      j = 0;
  mct_event_t                  cds_update_event;
  isp_cds_request_t            isp_cds_request;


  if (module == NULL || session_param == NULL) {
    ISP_ERR("NULL pointer,isp_module %p, session parm = %p",
      module, session_param);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;
  if (hw_update_params == NULL) {
    ISP_ERR("NULL pointer, hw_update_params = %p", hw_update_params);
    return FALSE;
  }

  if(hw_update_params->skip_hw_update == FALSE) {
    /* update hw update parm*/
    for (i = 0; i < session_param->num_isp; i++) {
      hw_id = session_param->hw_id[i];
      /* Update dmi table meta*/
      hw_update_list_params = &hw_update_params->hw_update_list_params[hw_id];
      /* update cds request info*/
      if (hw_update_list_params->new_cds_update == TRUE) {
        hw_update_list_params->cur_cds_update =
          hw_update_list_params->new_cds_update;
        hw_update_list_params->new_cds_update = FALSE;
        if (hw_update_list_params->cur_cds_update == TRUE) {
           hw_update_params->cds_updating = TRUE;
        }
      }
    }
  }

  /* if cds is updating this frame, notify iface for the cds update*/
  if (hw_update_params->cds_updating) {
    hw_update_params->skip_hw_update = FALSE;
    memset(&cds_update_event, 0, sizeof(mct_event_t));
    memset(&isp_cds_request, 0, sizeof(isp_cds_request_t));

    /* only tell iface isp starting cds*/
    isp_cds_request.cds_request_done = TRUE;

    cds_update_event.type = MCT_EVENT_MODULE_EVENT;
    cds_update_event.identity = session_param->session_based_ide;//event->identity;
    cds_update_event.direction = MCT_EVENT_UPSTREAM;
    cds_update_event.u.module_event.type = MCT_EVENT_MODULE_ISP_CDS_REQUEST;
    cds_update_event.u.module_event.module_event_data = &isp_cds_request;
    ret = isp_util_forward_event_from_module(module, &cds_update_event);

    if (ret == FALSE) {
      hw_update_params->skip_hw_update = TRUE;
      ISP_ERR("failed: <cds_dbg> MCT_EVENT_MODULE_ISP_CDS_REQUEST failed!");
    } else {
      hw_update_params->cds_updating = FALSE;
    }
  }

  return ret;
}

/** isp_util_dump_register:
 *
 *  @isp_module: isp module
 *  @session_param : isp session parameters
 *
 *  update hw parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_dump_register(mct_module_t *isp_module,
  isp_session_param_t *session_param, uint32_t frame_id, uint32_t num_register)
{
  boolean                  ret = TRUE;
  isp_hw_update_params_t  *hw_update_params = NULL;
  isp_t                   *isp = NULL;
  static uint32_t          dump_end = 0;
  static uint32_t          offline_dump_end = 0;
  uint32_t                 dump_count = 0, i = 0;;
  uint32_t                 offline_dump_count = 0;
  char                     value[PROPERTY_VALUE_MAX];
  uint32_t                 register_dump[num_register];
  isp_meta_t               *applied_meta_dump_parms = NULL;
  uint32_t                 *dmi_dump = NULL;
  uint64_t                 *dmi64_dump = NULL;
  isp_meta_dump_t          dmi_modules;
  isp_meta_entry_t         *meta_entry = NULL;


  /*===== Register Dump Iinstruction: ==========
    1. --- adb root;
       --- adb shell setprop persist.vendor.camera.isp.regdump 30;
           (dump 30 frames, configurable)

    2. after the first dump is done,
       if want to get second set dump
       --- adb shell setprop persist.vendor.camera.isp.regdump 0; (reset)
       --- adb shell setprop persist.vendor.camera.isp.regdump 30; (30 dump)

    3. every time we start camera, the dump count will also be reset.
  ========================================*/

#ifdef _ANDROID_
  property_get("persist.vendor.camera.isp.regdump", value, "0");
  dump_count = atoi(value);
  property_get("persist.vendor.camera.isp.offregdump", value, "0");
  offline_dump_count = atoi(value);
#endif

  if (dump_count == 0 && offline_dump_count == 0) {
    return ret;
  }

  if (isp_module == NULL ||session_param == NULL) {
    ISP_ERR("NULL pointer,isp_module %p, session parm = %p",
      isp_module, session_param);
    return FALSE;
  }

  isp = MCT_OBJECT_PRIVATE(isp_module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;
  if (hw_update_params == NULL) {
    ISP_ERR("NULL pointer, hw_update_params = %p", hw_update_params);
    return FALSE;
  }

  applied_meta_dump_parms = &hw_update_params->hw_update_list_params[session_param->hw_id[0]].
           applied_meta_dump_parms;


  /*when disable dump, reset dump end to indicate restart pending*/
  if (!dump_count || (frame_id == 1)) {
    dump_end = 0;
  }

  if (!offline_dump_count || (frame_id == 1)) {
    offline_dump_end = 0;
  }
  /* if
    1. dump count enable
    2. not passed dump end yet or restart pending(dump_end = 0)
    then dump register */
  if ((dump_count > 0) &&
    ((dump_end == 0) || (frame_id < dump_end))) {
    /*dump_end = 0 means restart pending*/
    if (dump_end == 0) {
      dump_end = frame_id + dump_count;
      ISP_ERR("dump register start! frame %d, dump end %d",
        frame_id, dump_end);
    }

    memset(&register_dump[0], 0, sizeof(uint32_t) * num_register);
    /*start dump*/
    ISP_ERR("===========ISP REG DUMP (reg_dump frame %d)===========",frame_id);
    ret = isp_util_read_reg_dump(hw_update_params->
      hw_update_list_params[session_param->hw_id[0]].fd,
      &register_dump[0], VFE_READ, num_register * sizeof(uint32_t), 1);
    if (ret == FALSE) {
      ISP_ERR("read isp register failed, skip meta dump");
      return FALSE;
    }

    if (hw_update_params->metadata_dump_enable == 1) {
      for (dmi_modules = ISP_META_ROLLOFF_TBL ; dmi_modules < ISP_META_MAX; dmi_modules++) {
        meta_entry = &applied_meta_dump_parms->meta_entry[dmi_modules];
        if (meta_entry->len && ISP_PRINT_DMI_METADUMP & (1 << dmi_modules)) {
          if (dmi_modules == ISP_META_LINEARIZATION_TBL || dmi_modules == ISP_META_GTM_TBL
            || dmi_modules == ISP_META_STATS_BF_RGN_IND_LUT) {
            ISP_ERR("DMI_type %d length = %d ", dmi_modules, meta_entry->len /sizeof(uint64_t));
            dmi64_dump =
             (uint64_t *)meta_entry->isp_meta_dump;
             /* read_lengh = by words (4 bytes)*/
            for (i = 0; i < (meta_entry->len /
             sizeof(uint64_t)); i=i+8) {
              ISP_ERR("%d DMI table[%d] %08llx %08llx %08llx %08llx %08llx %08llx %08llx %08llx",
                dmi_modules, i, dmi64_dump[i], dmi64_dump[i+1], dmi64_dump[i+2], dmi64_dump[i+3],
                dmi64_dump[i+4], dmi64_dump[i+5], dmi64_dump[i+6], dmi64_dump[i+7]);
            }
          } else {
            ISP_ERR("DMI_type %d length = %d ", dmi_modules, meta_entry->len /sizeof(uint32_t));
            dmi_dump =
             (uint32_t *)meta_entry->isp_meta_dump;
             /* read_lengh = by words (4 bytes)*/
            for (i = 0; i < (meta_entry->len /
              sizeof(uint32_t)); i=i+8) {
                ISP_ERR("%d DMI table[%d] %08x %08x %08x %08x %08x %08x %08x %08x",
                 dmi_modules, i, dmi_dump[i], dmi_dump[i], dmi_dump[i+1], dmi_dump[i+2],
                 dmi_dump[i+4], dmi_dump[i+5], dmi_dump[i+6], dmi_dump[i+7]);
            }
          }
        }
      }
    }

  }

  if ((offline_dump_count > 0) &&
     ((offline_dump_end == 0) || (frame_id < offline_dump_end)) &&
     (hw_update_params->hw_update_list_params[
      session_param->offline_hw_id[0]].fd > 0)) {
    /*offline dump_end = 0 means restart pending*/
    if (offline_dump_end == 0) {
      offline_dump_end = frame_id + offline_dump_count;
      ISP_ERR("dump offline register start! frame %d, offline dump end %d",
        frame_id, offline_dump_end);
    }

    memset(&register_dump[0], 0, sizeof(uint32_t) * num_register);
    /*start offline dump*/
    ISP_ERR("===========ISP OFFLINE REG DUMP (reg_dump frame %d)===========",frame_id);
    ret = isp_util_read_reg_dump(hw_update_params->
      hw_update_list_params[session_param->offline_hw_id[0]].fd,
      &register_dump[0], VFE_READ, num_register * sizeof(uint32_t), 1);
    if (ret == FALSE) {
      ISP_ERR("read isp register failed, skip meta dump");
      return FALSE;
    }
  }

  return ret;
}
/** isp_util_prepare_meta_dump:
 *
 *  @isp_module: isp module
 *  @session_param : isp session parameters
 *
 *  update hw parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_prepare_meta_dump(mct_module_t *isp_module,
  isp_session_param_t *session_param)
{
  boolean                      ret                     = TRUE;
  uint32_t                     i                       = 0;
  uint32_t                     j                       = 0;
  uint32_t                     num_reg                 = 0;
  cam_format_t                 fmt                     = 0xFFFFFFFF;
  isp_hw_id_t                  hw_id                   = 0;
  cam_stream_type_t            raw_stream_type         = CAM_STREAM_TYPE_RAW;
  isp_meta_t                  *applied_meta_dump_parms = NULL;
  isp_hw_read_info_t          *dmi_info                = NULL;
  isp_hw_update_params_t      *hw_update_params        = NULL;
  isp_t                       *isp                     = NULL;
  mct_list_t                  *l_stream_params         = NULL;
  isp_stream_param_t          *stream_param            = NULL;
  isp_hw_update_list_params_t *hw_list_parms           = NULL;

  enum msm_vfe_axi_stream_src axi_src = VFE_AXI_SRC_MAX;

  if (isp_module == NULL ||session_param == NULL) {
    ISP_ERR("NULL pointer,isp_module %p, session parm = %p",
      isp_module, session_param);
    return FALSE;
  }

  isp = MCT_OBJECT_PRIVATE(isp_module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;
  if (hw_update_params == NULL) {
    ISP_ERR("NULL pointer, hw_update_params = %p", hw_update_params);
    return FALSE;
  }
  hw_list_parms =
    &hw_update_params->hw_update_list_params[session_param->hw_id[0]];
  ISP_DBG("max_reporting_delay %d", session_param->max_reporting_delay);
  if (session_param->max_reporting_delay == 0) {
    applied_meta_dump_parms = &hw_list_parms->applied_meta_dump_parms;
  } else if (session_param->max_reporting_delay == 1) {
    applied_meta_dump_parms = hw_list_parms->temp_meta_dump_parms;
  } else {
    ISP_ERR("failed: meta data max reporting delay %d not supported",
      session_param->max_reporting_delay);
    return FALSE;
  }

  l_stream_params = mct_list_find_custom(session_param->l_stream_params,
    &raw_stream_type, isp_util_compare_stream_type_from_stream_param);
  if (!l_stream_params) {
    ISP_ERR("failed: l_stream_params %p for session %d", l_stream_params,
      raw_stream_type);
  } else {
    stream_param = (isp_stream_param_t *)l_stream_params->data;
    if (stream_param) {
      /* Sensor always streams in MIPI format. The setprop persist.vendor.camera.raw.format
         can be used to specify any RAW dump format e.g. camif raw. Update the meta dump
         info with the format used to dump the frame to memory */
      fmt = stream_param->stream_info.fmt;
      applied_meta_dump_parms->frame_meta.addlinfo.raw_width =
        stream_param->stream_info.dim.width;
      applied_meta_dump_parms->frame_meta.addlinfo.raw_height =
        stream_param->stream_info.dim.height;
      applied_meta_dump_parms->frame_meta.addlinfo.raw_stride =
        stream_param->stream_info.buf_planes.plane_info.mp[0].stride;
      applied_meta_dump_parms->frame_meta.addlinfo.raw_scanline =
        stream_param->stream_info.buf_planes.plane_info.mp[0].scanline;
    }
  }

  applied_meta_dump_parms->frame_meta.isp_version = GET_ISP_MAIN_VERSION(isp->isp_resource.
     isp_resource_info[session_param->hw_id[0]].isp_pipeline->isp_version);

  ISP_ERR("isp_metadump: ver = %d, R/G/B = %f / %f / %f, lux = %f"
    ", se_lux = %f, color_tem = %d raw_stride %d raw_scanline %d",
    applied_meta_dump_parms->frame_meta.isp_version,
    applied_meta_dump_parms->frame_meta.awb_gain.r_gain,
    applied_meta_dump_parms->frame_meta.awb_gain.g_gain,
    applied_meta_dump_parms->frame_meta.awb_gain.b_gain,
    applied_meta_dump_parms->frame_meta.lux_idx,
    applied_meta_dump_parms->frame_meta.se_lux_idx,
    applied_meta_dump_parms->frame_meta.color_temp,
    applied_meta_dump_parms->frame_meta.addlinfo.raw_stride,
    applied_meta_dump_parms->frame_meta.addlinfo.raw_scanline);


  applied_meta_dump_parms->frame_meta.addlinfo.reserved[2] =
    session_param->snapshot_path + 1;
  isp_fill_frame_format_param(fmt,
   &applied_meta_dump_parms->frame_meta.addlinfo.reserved[9]);

  applied_meta_dump_parms->frame_meta.num_entry++;
  mct_bus_msg_t bus_msg_meta_dump;
  memset(&bus_msg_meta_dump, 0, sizeof(bus_msg_meta_dump));
  bus_msg_meta_dump.type = MCT_BUS_MSG_ISP_META;
  bus_msg_meta_dump.size = sizeof(isp_meta_t);
  bus_msg_meta_dump.msg = applied_meta_dump_parms;
  bus_msg_meta_dump.sessionid = session_param->session_id;
  if (TRUE != mct_module_post_bus_msg(isp_module, &bus_msg_meta_dump))
    ISP_ERR("meta dump to bus error\n");

  if (session_param->max_reporting_delay == 1) {
    *hw_list_parms->temp_meta_dump_parms =
      hw_list_parms->applied_meta_dump_parms;
  }

  return ret;
}

/** isp_util_send_zoom_crop:
 *
 *  @isp_module: isp module
 *  @session_param : isp session parameters
 *
 *  Post offline stream crop information for 2 step CPP of offline yuv
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_send_zoom_crop(mct_module_t *isp_module,
  isp_session_param_t *session_param, isp_zoom_params_t *zoom_params_arr,
  cam_zoom_info_t *zoom)
{
  uint32_t        frame_id;
  boolean         ret = TRUE;
  struct timeval  timestamp;
  isp_t *isp = MCT_OBJECT_PRIVATE(isp_module);

  RETURN_IF_NULL(isp);
  frame_id = session_param->offline_frame_id + isp->isp_resource.
    isp_resource_info[session_param->offline_hw_id[0]].
    isp_pipeline->pipeline_applying_delay;
    ret = isp_util_broadcast_crop_info(isp_module, session_param,
      frame_id, &timestamp, zoom_params_arr, zoom, FALSE);
  return ret;
}

/** isp_util_send_gamma:
 *
 *  @isp_module: isp module
 *  @session_param : isp session parameters
 *
 *  update hw parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_send_gamma(mct_module_t *isp_module,
  isp_session_param_t *session_param, boolean is_online)
{
  boolean                  ret = TRUE;
  mct_bus_msg_isp_gamma_t  bus_msg_dmi;
  isp_dmi_tbl_meta_t      *applied_dmi_tbl = NULL;
  uint32_t                *reg_dump = NULL;
  uint32_t                 i = 0;
  uint32_t                 j = 0;
  isp_hw_read_info_t      *dmi_info = NULL;
  isp_hw_update_params_t  *hw_update_params = NULL;
  isp_hw_id_t              hw_index = 0;
  isp_t                    *isp = NULL;

  if (isp_module == NULL ||session_param == NULL) {
    ISP_ERR("NULL pointer,isp_module %p, session parm = %p",
      isp_module, session_param);
    return FALSE;
  }

  isp = MCT_OBJECT_PRIVATE(isp_module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;
  if (hw_update_params == NULL) {
    ISP_ERR("NULL pointer, hw_update_params = %p", hw_update_params);
    return FALSE;
  }

  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    if (session_param->hw_id[hw_index] != ISP_HW_MAX) {
      break;
    }
  }
  if (hw_index == ISP_HW_MAX) {
    ISP_HIGH(" Error! no isp found");
    return FALSE;
  }

  if (is_online) {
    applied_dmi_tbl = &hw_update_params->
    hw_update_list_params[session_param->hw_id[hw_index]].applied_dmi_tbl;
    RETURN_IF_NULL(applied_dmi_tbl);
  }
  else {
    applied_dmi_tbl = &hw_update_params->
    hw_update_list_params[session_param->offline_hw_id[0]].applied_dmi_tbl;
    RETURN_IF_NULL(applied_dmi_tbl);
  }

  memset(&bus_msg_dmi, 0, sizeof(bus_msg_dmi));
  if (ISP_PRINT_DMI_METADUMP & (1 << ISP_META_GAMMA_TBL)) {
     reg_dump =
       (uint32_t *)applied_dmi_tbl->dmi_info[ISP_METADUMP_GAMMA].hw_dmi_tbl;
     /* read_lengh = by words (4 bytes)*/
     for (i = 0; i < (applied_dmi_tbl->dmi_info[ISP_METADUMP_GAMMA].read_length /
       sizeof(uint32_t)); i++) {
       ISP_ERR("GAMMA DMI table[%d] %08x", i, reg_dump[i]);
     }
  }
  bus_msg_dmi.isp_version =
    GET_ISP_MAIN_VERSION(isp->isp_resource.
    isp_resource_info[session_param->hw_id[hw_index]].isp_pipeline->isp_version);
  bus_msg_dmi.num_entries =
    applied_dmi_tbl->dmi_info[ISP_METADUMP_GAMMA].num_entries;
  bus_msg_dmi.entry_size =
    applied_dmi_tbl->dmi_info[ISP_METADUMP_GAMMA].entry_size;
  bus_msg_dmi.pack_bits =
    applied_dmi_tbl->dmi_info[ISP_METADUMP_GAMMA].pack_bits;
  if (bus_msg_dmi.num_entries && bus_msg_dmi.entry_size) {
    bus_msg_dmi.num_tables =
        applied_dmi_tbl->dmi_info[ISP_METADUMP_GAMMA].read_length /
        (bus_msg_dmi.num_entries * bus_msg_dmi.entry_size);
  }

  if (sizeof(bus_msg_dmi.private_data) <
    applied_dmi_tbl->dmi_info[ISP_METADUMP_GAMMA].read_length) {
    ISP_ERR("gamma table size is bigger than bus msg private data");
    return FALSE;
  }

  ISP_DBG("num_entries: %d entry size %d isp version %d "
    "num_entries %d pack_bits %d num_tables %d",
    bus_msg_dmi.num_entries, bus_msg_dmi.entry_size,
    bus_msg_dmi.isp_version, bus_msg_dmi.num_entries,
    bus_msg_dmi.pack_bits, bus_msg_dmi.num_tables);

  if (bus_msg_dmi.num_entries > 0 ) {
    memcpy(bus_msg_dmi.private_data, (char *)
      applied_dmi_tbl->dmi_info[ISP_METADUMP_GAMMA].hw_dmi_tbl,
      sizeof(bus_msg_dmi.private_data));

    mct_bus_msg_t bus_msg_dmi_meta;
    bus_msg_dmi_meta.type = MCT_BUS_MSG_SET_ISP_GAMMA_INFO;
    bus_msg_dmi_meta.size = sizeof(bus_msg_dmi);
    bus_msg_dmi_meta.msg = (void *)&bus_msg_dmi;
    bus_msg_dmi_meta.sessionid = session_param->session_id;
    if (is_online)
      bus_msg_dmi_meta.metadata_collection_type = MCT_BUS_ONLINE_METADATA;
    else
      bus_msg_dmi_meta.metadata_collection_type = MCT_BUS_OFFLINE_METADATA;
    if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg_dmi_meta,
      session_param->curr_frame_id)) {
      ISP_ERR("meta dump to bus error\n");
    }
  }
  return ret;
}


/** isp_util_send_hdr_curves:
 *
 *  @isp_module: isp module
 *  @session_param : isp session parameters
 *
 *  update hw parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_send_hdr_curves(mct_module_t *isp_module,
  isp_session_param_t *session_param, boolean is_online)
{
  boolean                  ret = TRUE;
  mct_hdr_data             hdr_data;

  if (isp_module == NULL ||session_param == NULL) {
    ISP_ERR("NULL pointer,isp_module %p, session parm = %p",
      isp_module, session_param);
    return FALSE;
  }

 memset(&hdr_data, 0, sizeof(hdr_data));
  mct_bus_msg_t bus_msg_dmi_meta;
  bus_msg_dmi_meta.type = MCT_BUS_MSG_SET_HDR_CURVES_INFO;
  bus_msg_dmi_meta.size = sizeof(hdr_data);
  hdr_data = session_param->hdr_data_stored;
  bus_msg_dmi_meta.msg = (void *)&hdr_data;
  bus_msg_dmi_meta.sessionid = session_param->session_id;

  if (is_online)
    bus_msg_dmi_meta.metadata_collection_type = MCT_BUS_ONLINE_METADATA;
  else
    bus_msg_dmi_meta.metadata_collection_type = MCT_BUS_OFFLINE_METADATA;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg_dmi_meta,
    session_param->curr_frame_id)) {
    ISP_ERR("meta dump to bus error\n");
  }

  return ret;
}


/** isp_util_send_awb_stats_info:
 *
 *  @isp_module: isp module
 *  @applied_frame_meta: metadata applied to current frame id
 *  @session_param : isp session parameters
 *
 *  send awb stats information of current frame
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_util_send_awb_stats_info(mct_module_t *isp_module,
  isp_per_frame_meta_t *applied_frame_meta,
  isp_session_param_t *session_param, boolean is_online)
{
  boolean                               ret = TRUE;
  mct_bus_msg_isp_stats_awb_metadata_t *awb_info= NULL;
  mct_bus_msg_t                         bus_msg_awb_info;
  awb_update_t                          awb_update;

  RETURN_IF_NULL(session_param);
  RETURN_IF_NULL(applied_frame_meta);

  awb_update = applied_frame_meta->awb_update;
  ISP_DBG("%d %d", sizeof(mct_bus_msg_isp_stats_awb_metadata_t),
    sizeof(awb_update));

  if (sizeof(mct_bus_msg_isp_stats_awb_metadata_t) <
    sizeof(awb_update)) {
    ISP_ERR("Can not post awb info as sizeof mct struct is smaller");
    return FALSE;
  }
  bus_msg_awb_info.type = MCT_BUS_MSG_SET_ISP_STATS_AWB_INFO;
  bus_msg_awb_info.size = sizeof(awb_update);
  bus_msg_awb_info.msg = (void *)&awb_update;
  bus_msg_awb_info.sessionid = session_param->session_id;
  if (is_online)
    bus_msg_awb_info.metadata_collection_type = MCT_BUS_ONLINE_METADATA;
  else
    bus_msg_awb_info.metadata_collection_type = MCT_BUS_OFFLINE_METADATA;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg_awb_info,
    session_param->curr_frame_id)) {
    ISP_ERR("isp_util_send_awb_stats_info error");
  }
  return ret;
}

/** isp_util_send_current_scene:
 *
 *  @isp_module: isp module
 *  @applied_frame_meta: metadata applied to current frame id
 *  @session_param : isp session parameters
 *
 *  send bestshot/scene info to HAl for scene detection feature
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_util_send_current_scene(mct_module_t *isp_module,
  isp_per_frame_meta_t *applied_frame_meta,
  isp_session_param_t *session_param, boolean is_online)
{
  boolean                  ret = TRUE;
  cam_scene_mode_type      bestshot_mode = CAM_SCENE_MODE_OFF;
  mct_bus_msg_t            bus_msg_scene_meta;

  RETURN_IF_NULL(session_param);
  RETURN_IF_NULL(applied_frame_meta);

  bestshot_mode = applied_frame_meta->bestshot_mode;
  bus_msg_scene_meta.type = MCT_BUS_MSG_META_CURRENT_SCENE;
  bus_msg_scene_meta.size = sizeof(bestshot_mode);
  bus_msg_scene_meta.msg = (void *)&bestshot_mode;
  bus_msg_scene_meta.sessionid = session_param->session_id;
  if (is_online)
    bus_msg_scene_meta.metadata_collection_type = MCT_BUS_ONLINE_METADATA;
  else
    bus_msg_scene_meta.metadata_collection_type = MCT_BUS_OFFLINE_METADATA;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg_scene_meta,
    session_param->curr_frame_id)) {
    ISP_ERR("send_current_scene error");
  }
  return ret;
}

/** isp_util_prepare_vfe_diag:
 *
 *  @isp_module: isp module
 *  @applied_frame_meta: metadata applied to current frame id
 *  @session_param : isp session parameters
 *
 *  send vfe diagnostics info to eztune. use for mobicat as well.
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_util_prepare_vfe_diag(mct_module_t *isp_module,
  isp_per_frame_meta_t *applied_frame_meta,
  isp_session_param_t *session_param, boolean is_online)
{
  boolean                  ret = TRUE;
  vfe_diagnostics_t       *vfe_diag = NULL;
  mct_bus_msg_t            bus_msg_vfe_diag;

  RETURN_IF_NULL(session_param);
  RETURN_IF_NULL(applied_frame_meta);

  vfe_diag = &applied_frame_meta->vfe_diag;
  if (applied_frame_meta->vfe_diag_enable) {
    ISP_HIGH("vfe diag_enable %d",
      applied_frame_meta->vfe_diag_enable);
    bus_msg_vfe_diag.type = MCT_BUS_MSG_ISP_CHROMATIX_LITE;
    bus_msg_vfe_diag.size = sizeof(vfe_diagnostics_t);
    bus_msg_vfe_diag.msg = (void *)vfe_diag;
    bus_msg_vfe_diag.sessionid = session_param->session_id;
    if (is_online)
      bus_msg_vfe_diag.metadata_collection_type = MCT_BUS_ONLINE_METADATA;
    else
      bus_msg_vfe_diag.metadata_collection_type = MCT_BUS_OFFLINE_METADATA;
    if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg_vfe_diag,
      session_param->curr_frame_id)) {
      ISP_ERR("isp_util_prepare_vfe_diag");
    }
  }
  return ret;
}

/** isp_util_generate_neutral_point
 *    @cfg: applied configurations for the frame
 *
 *   Neutral point
 *   Inverse(CC*AB) * (1, 1, 1), where CC is VFE color
 *   correction matrix (fully interpolated based on AE and AWB),
 *   and AB is diagonal matrix with normalized white balance
 *   gain.
 *
 *  Return none
 **/
static boolean isp_util_generate_neutral_point(isp_per_frame_meta_t *cfg)
{
    boolean  ret = TRUE;
    float cc_matrix[3][3],inv_matrix[3][3],tmp_mat[3][3], result[3][1];
    float tmp[3][1] = {{1},
                       {1},
                       {1}};

    float wb_gain[3][3] = {{0,0,0},
                           {0,0,0},
                           {0,0,0}};
    float *tmp1, *tmp2;

    wb_gain[0][0] = cfg->cc_gain.gains[0]/cfg->cc_gain.gains[1];
    wb_gain[1][1] = cfg->cc_gain.gains[1]/cfg->cc_gain.gains[1];
    wb_gain[2][2] = cfg->cc_gain.gains[3]/cfg->cc_gain.gains[1];

    RATIONAL_TO_FLOAT(cfg->color_correction_matrix.transform_matrix, cc_matrix, 3, 3);

    /* (CC*AB)*/
    MATRIX_MULT(cc_matrix, wb_gain, tmp_mat, 3, 3, 3);

    /* Inverse(CC*AB)*/
    tmp1 = (float*) tmp_mat;
    tmp2 = (float*) inv_matrix;

    MATRIX_INVERSE_3x3(tmp1, tmp2);

    /* Inverse(CC*AB) * (1, 1, 1)*/
    MATRIX_MULT(inv_matrix, tmp,result,3,3,1);

    /* Convert to rational format */
    cfg->neutral_color_point.neutral_col_point[0].numerator =
      FLOAT_TO_Q(7, result[0][0]);
    cfg->neutral_color_point.neutral_col_point[0].denominator = 1 << 7;

    cfg->neutral_color_point.neutral_col_point[1].numerator =
      FLOAT_TO_Q(7, result[1][0]);
     cfg->neutral_color_point.neutral_col_point[1].denominator = 1 << 7;

    cfg->neutral_color_point.neutral_col_point[2].numerator =
      FLOAT_TO_Q(7, result[2][0]);
    cfg->neutral_color_point.neutral_col_point[2].denominator = 1 << 7;
    return ret;
}


/** isp_util_clear_all_stored_metadata:
 *  @session_param : isp session parameters
 *  @frame_id: current frame id
 *
 *  Send stored meta data for frame id "frame_id".
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_clear_all_stored_metadata(isp_session_param_t *session_param)
{
  if (!session_param->max_reporting_delay)
    return TRUE;

  memset(&session_param->metadata, 0, sizeof(session_param->metadata));

  return TRUE;
}

/** isp_util_report_stored_metadata:
 *  @module: mct module handle
 *  @session_param : isp session parameters
 *  @frame_id: current frame id
 *
 *  Send stored meta data for frame id "frame_id".
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_report_stored_metadata(mct_module_t *module,
  isp_session_param_t *session_param, uint32_t frame_id)
{
  mct_bus_msg_type_t bus_msg_type;
  mct_bus_msg_t bus_msg;
  uint32_t i,idx = 0;
  boolean ret = TRUE;
  boolean send_err = TRUE;
  uint8_t index = 0;

  idx = frame_id % ISP_MAX_META_REPORTING_QUEUE_DEPTH;
  memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.sessionid = session_param->session_id;

  PTHREAD_MUTEX_LOCK(&session_param->metadata_mutex);

  for (bus_msg_type = 0; bus_msg_type < MCT_BUS_MSG_MAX; bus_msg_type++) {
    switch (bus_msg_type) {
    case MCT_BUS_MSG_SET_ISP_GAMMA_INFO:
      if (!session_param->metadata[idx].valid_set_isp_gamma_info)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_gamma_info;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_gamma_info);
      break;
    case MCT_BUS_MSG_SET_ISP_LPM_INFO:
      if (!session_param->metadata[idx].valid_set_isp_lpm_info)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_lpm_info;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_lpm_info);
      break;
    case MCT_BUS_MSG_META_CURRENT_SCENE:
      if (!session_param->metadata[idx].valid_meta_current_scene)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_meta_current_scene;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_meta_current_scene);
      break;
    case MCT_BUS_MSG_ISP_CHROMATIX_LITE:
      if (!session_param->metadata[idx].valid_isp_chromatix_lite)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_isp_chromatix_lite;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_isp_chromatix_lite);
      break;
    case MCT_BUS_MSG_SET_ISP_STATS_AWB_INFO:
      if (!session_param->metadata[idx].valid_set_isp_stats_awb_info)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_stats_awb_info;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_stats_awb_info);
      break;
    case MCT_BUS_MSG_SET_ISP_LENS_SHADING_INFO:
      if (!session_param->metadata[idx].valid_set_isp_lens_shading_info)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg =
        &session_param->metadata[idx].meta_set_isp_lens_shading_info;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_lens_shading_info);
      break;
    case MCT_BUS_MSG_SET_ISP_LENS_SHADING_MODE:
      if (!session_param->metadata[idx].valid_set_isp_lens_shading_mode)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg =
        &session_param->metadata[idx].meta_set_isp_lens_shading_mode;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_lens_shading_mode);
      break;
    case MCT_BUS_MSG_SET_ISP_LENS_SHADING_MAP_MODE:
      if (!session_param->metadata[idx].valid_set_isp_lens_shading_map_mode)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg =
        &session_param->metadata[idx].meta_set_isp_lens_shading_map_mode;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_lens_shading_map_mode);
      break;

    case MCT_BUS_MSG_ISP_GAIN:
      if (!session_param->metadata[idx].valid_isp_dig_gain)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg =
        &session_param->metadata[idx].isp_dig_gain;
      bus_msg.size =
        sizeof(session_param->metadata[idx].isp_dig_gain);
      break;

    case MCT_BUS_MSG_SET_ISP_TONE_MAP_MODE:
      if (!session_param->metadata[idx].valid_set_isp_tone_map_mode)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_tone_map_mode;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_tone_map_mode);
      break;
    case MCT_BUS_MSG_SET_ISP_TONE_MAP:
      if (!session_param->metadata[idx].valid_set_isp_tone_map)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_tone_map;
      bus_msg.size = sizeof(session_param->metadata[idx].meta_set_isp_tone_map);
      break;
    case MCT_BUS_MSG_SET_ISP_CC_MODE:
      if (!session_param->metadata[idx].valid_set_isp_cc_mode)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_cc_mode;
      bus_msg.size = sizeof(session_param->metadata[idx].meta_set_isp_cc_mode);
      break;
    case MCT_BUS_MSG_SET_ISP_CC_TRANSFORM:
      if (!session_param->metadata[idx].valid_set_isp_cc_transform)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_cc_transform;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_cc_transform);
      break;
    case MCT_BUS_MSG_SET_ISP_CC_GAIN:
      if (!session_param->metadata[idx].valid_set_isp_cc_gain)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_cc_gain;
      bus_msg.size = sizeof(session_param->metadata[idx].meta_set_isp_cc_gain);
      break;
    case MCT_BUS_MSG_SET_ISP_CONTROL_MODE:
      if (!session_param->metadata[idx].valid_set_isp_control_mode)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_control_mode;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_control_mode);
      break;
    case MCT_BUS_MSG_ISP_CROP_REGION:
        if (!session_param->metadata[idx].valid_isp_crop_region)
          continue;
        bus_msg.type = bus_msg_type;
        bus_msg.msg = &session_param->metadata[idx].meta_isp_crop_region;
        bus_msg.size =
          sizeof(session_param->metadata[idx].meta_isp_crop_region);
        break;
    case MCT_BUS_MSG_SET_ISP_ABF_MODE:
      if (!session_param->metadata[idx].valid_set_isp_abf_mode)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_abf_mode;
      bus_msg.size = sizeof(session_param->metadata[idx].meta_set_isp_abf_mode);
      break;
    case MCT_BUS_MSG_SET_CAC_MODE:
      if (!session_param->metadata[idx].valid_set_isp_cac_mode)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_cac_mode;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_cac_mode);
      break;
    case MCT_BUS_MSG_SET_ISP_HOT_PIX_MODE:
      if (!session_param->metadata[idx].valid_set_isp_hot_pix_mode)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_hot_pix_mode;
      bus_msg.size = sizeof(session_param->metadata[idx].meta_set_isp_hot_pix_mode);
      break;
    case MCT_BUS_MSG_SET_ISP_BLACK_LEVEL_LOCK:
      if (!session_param->metadata[idx].valid_set_isp_black_level_lock)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_black_level_lock;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_black_level_lock);
      break;
    case MCT_BUS_MSG_SET_ISP_CAPTURE_INTENT:
      if (!session_param->metadata[idx].valid_set_isp_capture_intent)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_capture_intent;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_capture_intent);
      break;
    case MCT_BUS_MSG_SET_ISP_ZOOM_VALUE:
      if (!session_param->metadata[idx].valid_set_isp_zoom_value)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_zoom_value;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_zoom_value);
      break;
    case MCT_BUS_MSG_SET_ISP_CONTRAST:
      if (!session_param->metadata[idx].valid_set_isp_contrast)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_contrast;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_contrast);
      break;
    case MCT_BUS_MSG_SET_ISP_SATURATION:
      if (!session_param->metadata[idx].valid_set_isp_saturation)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_saturation;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_saturation);
      break;
    case MCT_BUS_MSG_SET_EFFECT:
      if (!session_param->metadata[idx].valid_set_isp_special_effect)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_isp_special_effect;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_set_isp_special_effect);
      break;
     case MCT_BUS_MSG_SET_TINTLESS:
      if (!session_param->metadata[idx].valid_tintless)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_tintless_enable;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_tintless_enable);
      break;
    case MCT_BUS_MSG_PROFILE_TONE_CURVE:
      if (!session_param->metadata[idx].valid_profile_tone_curve)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_profile_tone_curve;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_profile_tone_curve);
      break;
    case MCT_BUS_MSG_ISP_META:
      if (!session_param->metadata[idx].valid_isp_meta)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_isp_meta;
      bus_msg.size = sizeof(session_param->metadata[idx].meta_isp_meta);
      break;
    case MCT_BUS_MSG_SET_SENSOR_HDR_MODE:
      if (!session_param->metadata[idx].valid_set_sensor_hdr_info)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_set_sensor_hdr_info;
      bus_msg.size = sizeof(session_param->metadata[idx].meta_set_sensor_hdr_info);
      break;
    case MCT_BUS_MSG_ISP_SESSION_CROP: {
      if (session_param->metadata[idx].valid_isp_stream_crop) {
        mct_bus_msg_session_crop_info_t *session_crop =
          &session_param->metadata[idx].session_crop;
        ISP_DBG("num_of_streams = %d", session_crop->num_of_streams);
        for (index=0; index < session_crop->num_of_streams; index ++) {
          ISP_DBG("crop_info[%d]: stream id %d: (%d, %d) -- (%d, %d)",
            index, session_crop->crop_info[index].stream_id,
            session_crop->crop_info[index].x,
            session_crop->crop_info[index].y,
            session_crop->crop_info[index].crop_out_x,
            session_crop->crop_info[index].crop_out_y);
        }

        bus_msg.type = bus_msg_type;
        bus_msg.msg = &session_param->metadata[idx].session_crop;
        bus_msg.size = sizeof(mct_bus_msg_session_crop_info_t);
        send_err = mct_module_post_bus_msg(module, &bus_msg);
        if (send_err == FALSE) {
          ret = FALSE;
          ISP_ERR("failed: post to bus");
        }
      }
      continue;
    }

    case MCT_BUS_MSG_HIST_STATS_INFO:
      if (!session_param->metadata[idx].valid_hist_stats_info)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_hist_stats_info;
      bus_msg.size = sizeof(session_param->metadata[idx].meta_hist_stats_info);
      break;
    case MCT_BUS_MSG_NEUTRAL_COL_POINT:
      if (!session_param->metadata[idx].valid_neutral_col_point)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_neutral_col_point;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_neutral_col_point);
      break;
    case MCT_BUS_MSG_SNAP_CROP_INFO_ISP:
      if (!session_param->metadata[idx].valid_isp_snap_crop)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_isp_snap_crop;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_isp_snap_crop);
      break;
    case MCT_BUS_MSG_SNAP_CROP_INFO_PP:
      if (!session_param->metadata[idx].valid_pp_snap_crop)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_pp_snap_crop;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_pp_snap_crop);
      break;
    case MCT_BUS_MSG_SET_HDR_CURVES_INFO:
      if (!session_param->metadata[idx].valid_hdr_data)
        continue;
      bus_msg.type = bus_msg_type;
      bus_msg.msg = &session_param->metadata[idx].meta_hdr_data;
      bus_msg.size =
        sizeof(session_param->metadata[idx].meta_hdr_data);
      break;
    default:
      continue;
    }

    send_err = mct_module_post_bus_msg(module, &bus_msg);
    if (send_err == FALSE) {
      ret = FALSE;
      ISP_ERR("failed: post to bus");
    }
  }

  memset(&session_param->metadata[idx], 0, sizeof(session_param->metadata[0]));

  PTHREAD_MUTEX_UNLOCK(&session_param->metadata_mutex);

  return ret;
}

/** isp_util_store_metadata:
 *  @session_param : isp session parameters
 *  @bus_msg: bus message
 *  @frame_id: meta data should this frame on this frame id
 *
 *  Store meta data.
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_store_metadata(isp_session_param_t *session_param,
  mct_bus_msg_t *bus_msg, uint32_t frame_id)
{
  boolean ret = TRUE;
  uint32_t i, idx = 0;

  idx = frame_id % ISP_MAX_META_REPORTING_QUEUE_DEPTH;

  PTHREAD_MUTEX_LOCK(&session_param->metadata_mutex);

  switch (bus_msg->type) {
  case MCT_BUS_MSG_SET_CAC_MODE:
    session_param->metadata[idx].meta_set_isp_cac_mode =
      *((cam_aberration_mode_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_cac_mode = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_GAMMA_INFO:
    session_param->metadata[idx].meta_set_isp_gamma_info =
      *((mct_bus_msg_isp_gamma_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_gamma_info = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_LPM_INFO:
    session_param->metadata[idx].meta_set_isp_lpm_info =
      *((mct_bus_msg_isp_low_power_mode_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_lpm_info = TRUE;
    break;
  case MCT_BUS_MSG_META_CURRENT_SCENE:
    session_param->metadata[idx].meta_meta_current_scene =
      *((cam_scene_mode_type *)bus_msg->msg);
    session_param->metadata[idx].valid_meta_current_scene = TRUE;
    break;
  case MCT_BUS_MSG_ISP_CHROMATIX_LITE:
    session_param->metadata[idx].meta_isp_chromatix_lite =
      *((vfe_diagnostics_t *)bus_msg->msg);
    session_param->metadata[idx].valid_isp_chromatix_lite = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_STATS_AWB_INFO:
    session_param->metadata[idx].meta_set_isp_stats_awb_info =
      *((awb_update_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_stats_awb_info = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_LENS_SHADING_INFO:
    session_param->metadata[idx].meta_set_isp_lens_shading_info =
      *((cam_lens_shading_map_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_lens_shading_info = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_LENS_SHADING_MODE:
    session_param->metadata[idx].meta_set_isp_lens_shading_mode =
      *((cam_lens_shading_mode_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_lens_shading_mode = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_LENS_SHADING_MAP_MODE:
    session_param->metadata[idx].meta_set_isp_lens_shading_map_mode =
      *((cam_lens_shading_map_mode_t*)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_lens_shading_map_mode = TRUE;
    break;
  case MCT_BUS_MSG_ISP_GAIN:
    session_param->metadata[idx].isp_dig_gain =
      *((int32_t*)bus_msg->msg);
    session_param->metadata[idx].valid_isp_dig_gain = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_TONE_MAP_MODE:
    session_param->metadata[idx].meta_set_isp_tone_map_mode =
      *((cam_tonemap_mode_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_tone_map_mode = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_TONE_MAP:
    session_param->metadata[idx].meta_set_isp_tone_map =
      *((cam_rgb_tonemap_curves *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_tone_map = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_CC_MODE:
    session_param->metadata[idx].meta_set_isp_cc_mode =
      *((cam_color_correct_mode_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_cc_mode = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_CC_TRANSFORM:
    session_param->metadata[idx].meta_set_isp_cc_transform =
      *((cam_color_correct_matrix_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_cc_transform = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_CC_GAIN:
    session_param->metadata[idx].meta_set_isp_cc_gain =
      *((cam_color_correct_gains_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_cc_gain = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_CONTROL_MODE:
    session_param->metadata[idx].meta_set_isp_control_mode =
      *((uint8_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_control_mode = TRUE;
    break;
  case MCT_BUS_MSG_ISP_CROP_REGION:
    session_param->metadata[idx].meta_isp_crop_region =
      *((cam_crop_region_t *)bus_msg->msg);
    session_param->metadata[idx].valid_isp_crop_region = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_ABF_MODE:
    session_param->metadata[idx].meta_set_isp_abf_mode =
      *((uint8_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_abf_mode = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_HOT_PIX_MODE:
    session_param->metadata[idx].meta_set_isp_hot_pix_mode =
      *((uint8_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_hot_pix_mode = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_BLACK_LEVEL_LOCK:
    session_param->metadata[idx].meta_set_isp_black_level_lock =
      *((uint8_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_black_level_lock = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_CAPTURE_INTENT:
    session_param->metadata[idx].meta_set_isp_capture_intent =
      *((uint8_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_capture_intent = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_ZOOM_VALUE:
    session_param->metadata[idx].meta_set_isp_zoom_value =
      *((int32_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_zoom_value = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_CONTRAST:
    session_param->metadata[idx].meta_set_isp_contrast =
      *((int32_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_contrast = TRUE;
    break;
  case MCT_BUS_MSG_SET_ISP_SATURATION:
    session_param->metadata[idx].meta_set_isp_saturation =
      *((int32_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_saturation = TRUE;
    break;
  case MCT_BUS_MSG_SET_EFFECT:
    session_param->metadata[idx].meta_set_isp_special_effect =
      *((cam_effect_mode_type *)bus_msg->msg);
    session_param->metadata[idx].valid_set_isp_special_effect = TRUE;
    break;
  case MCT_BUS_MSG_PROFILE_TONE_CURVE:
    session_param->metadata[idx].meta_profile_tone_curve =
      *((cam_profile_tone_curve *)bus_msg->msg);
    session_param->metadata[idx].valid_profile_tone_curve = TRUE;
    break;
  case MCT_BUS_MSG_ISP_META:
    session_param->metadata[idx].meta_isp_meta = *((isp_meta_t *)bus_msg->msg);
    session_param->metadata[idx].valid_isp_meta = TRUE;
    break;

  case MCT_BUS_MSG_ISP_SESSION_CROP:
    session_param->metadata[idx].session_crop =
      *((mct_bus_msg_session_crop_info_t*)bus_msg->msg);
    session_param->metadata[idx].valid_isp_stream_crop = TRUE;
    break;
  case MCT_BUS_MSG_HIST_STATS_INFO:
    session_param->metadata[idx].meta_hist_stats_info =
      *((mct_bus_msg_isp_bhist_stats_t *)bus_msg->msg);
    session_param->metadata[idx].valid_hist_stats_info = TRUE;
    break;
  case MCT_BUS_MSG_NEUTRAL_COL_POINT:
    session_param->metadata[idx].meta_neutral_col_point =
      *((cam_neutral_col_point_t *)bus_msg->msg);
    session_param->metadata[idx].valid_neutral_col_point = TRUE;
    break;
  case MCT_BUS_MSG_SNAP_CROP_INFO_ISP:
    session_param->metadata[idx].meta_isp_snap_crop =
      *((cam_stream_crop_info_t *)bus_msg->msg);
    session_param->metadata[idx].valid_isp_snap_crop = TRUE;
    break;
  case MCT_BUS_MSG_SNAP_CROP_INFO_PP:
    session_param->metadata[idx].meta_pp_snap_crop =
      *((cam_stream_crop_info_t *)bus_msg->msg);
    session_param->metadata[idx].valid_pp_snap_crop = TRUE;
    break;
  case MCT_BUS_MSG_SET_SENSOR_HDR_MODE:
    session_param->metadata[idx].meta_set_sensor_hdr_info =
      *((cam_sensor_hdr_type_t *)bus_msg->msg);
    session_param->metadata[idx].valid_set_sensor_hdr_info = TRUE;
    break;
  case MCT_BUS_MSG_SET_HDR_CURVES_INFO:
    session_param->metadata[idx].meta_hdr_data =
    *((mct_bus_msg_hdr_data *)bus_msg->msg);
    session_param->metadata[idx].valid_hdr_data = TRUE;
    break;
  case MCT_BUS_MSG_SET_TINTLESS:
    session_param->metadata[idx].meta_tintless_enable =
      *((int32_t *)bus_msg->msg);
    session_param->metadata[idx].valid_tintless = TRUE;
    break;
  default:
    ISP_ERR("Not supported meta entry!");
    ret = FALSE;
    break;
  }

  PTHREAD_MUTEX_UNLOCK(&session_param->metadata_mutex);

  return ret;
}

boolean isp_util_post_hdr_data(mct_port_t *port,
  isp_session_param_t *session_param, uint32_t frame_id)
{
  mct_event_t             mct_event;

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.u.module_event.type = MCT_EVENT_MODULE_POST_HDR_CURVES;
  mct_event.u.module_event.module_event_data = (void *)&(session_param->hdr_data_stored);
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = session_param->session_based_ide;
  mct_event.direction = MCT_EVENT_DOWNSTREAM;
  mct_event.u.module_event.current_frame_id =
    frame_id - (session_param->max_apply_delay - 1);

  if (isp_util_forward_event(port, &mct_event) == FALSE ){
    ISP_ERR("failed: isp_util_forward_event");
    PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);
    return FALSE;
  }

  return TRUE;
}

/** isp_util_send_metadata_entry:
 *  @module: mct module handle
 *  @bus_msg: bus message
 *  @frame_id: curr frame id
 *
 *  Send meta data to MCT or store it for later.
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_send_metadata_entry(mct_module_t *module,
  mct_bus_msg_t *bus_msg, uint32_t frame_id)
{
  boolean ret = FALSE;
  isp_session_param_t *session_param = NULL;

  ret = isp_util_get_session_params(module, bus_msg->sessionid,
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("cannot find session: %d ", bus_msg->sessionid);
    return FALSE;
  }

  if ((session_param->max_reporting_delay) &&
    (bus_msg->metadata_collection_type != MCT_BUS_OFFLINE_METADATA)) {
    ret = isp_util_store_metadata(session_param, bus_msg,
      frame_id + session_param->max_reporting_delay);
    if (ret == FALSE)
      ISP_ERR("failed: store meta data");
  } else {
    ret = mct_module_post_bus_msg(module, bus_msg);
    if (ret == FALSE)
      ISP_ERR("failed: post to bus");
  }

  return ret;
}

/** isp_util_send_metadata:
 *
 * @isp_module: isp module
 * @session_param : isp session parameters
 *
 * This function is used to post metadata of current frame.
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean isp_util_send_metadata(mct_module_t *isp_module,
  isp_session_param_t *session_param, boolean is_online)
{
  boolean                  ret = TRUE;
  cam_scene_mode_type      bus_msg_frame_meta;
  isp_per_frame_meta_t    *applied_frame_meta = NULL;
  uint32_t                *reg_dump = NULL;
  isp_hw_update_params_t  *hw_update_params = NULL;
  isp_hw_id_t              hw_id = 0;
  isp_t                   *isp = NULL;
  mct_bus_msg_t           bus_msg;
  uint32_t                frame_id;
  mct_bus_msg_isp_low_power_mode_t lpm_meta;
  mct_event_control_parm_t  *online_param;
  cam_zoom_info_t         *zoom = NULL;

  if (isp_module == NULL || session_param == NULL) {
    ISP_ERR("NULL pointer,isp_module %p, session parm = %p",
      isp_module, session_param);
    return FALSE;
  }

  frame_id = session_param->curr_frame_id;

  isp = MCT_OBJECT_PRIVATE(isp_module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;
  if (hw_update_params == NULL) {
    ISP_ERR("NULL pointer, hw_update_params = %p", hw_update_params);
    return FALSE;
  }

  if (is_online == TRUE) {
    applied_frame_meta = &hw_update_params->
    hw_update_list_params[session_param->hw_id[0]].applied_frame_meta;
    bus_msg.metadata_collection_type = MCT_BUS_ONLINE_METADATA;
    RETURN_IF_NULL(applied_frame_meta);
  }  else {
    applied_frame_meta = &hw_update_params->
    hw_update_list_params[session_param->offline_hw_id[0]].applied_frame_meta;
    bus_msg.metadata_collection_type = MCT_BUS_OFFLINE_METADATA;
    RETURN_IF_NULL(applied_frame_meta);

    /* Post zoom crop params for 2 step CPP processing of offline buffer */
    isp_zoom_params_t *zoom_params_arr = &hw_update_params->
      hw_update_list_params[session_param->offline_hw_id[0]].
      applied_stats_params->zoom_params[0];
    zoom = &hw_update_params->
      hw_update_list_params[session_param->offline_hw_id[0]].
      applied_stats_params->zoom;
    ret = isp_util_send_zoom_crop(isp_module, session_param, zoom_params_arr,
      zoom);
    if (ret == FALSE)
      ISP_ERR("falied: isp_util_send_zoom_crop");
  }

  ret = isp_util_send_gamma(isp_module, session_param, is_online);
  if (ret == FALSE)
    ISP_ERR("falied: isp_util_send_gamma");
  ret = isp_util_send_current_scene(isp_module, applied_frame_meta,
    session_param, is_online);
  if (ret == FALSE)
    ISP_ERR("falied: isp_util_send_current_scene");
  ret = isp_util_prepare_vfe_diag(isp_module, applied_frame_meta,
    session_param, is_online);
  if (ret == FALSE)
    ISP_ERR("falied: isp_util_send_vfe_diag");
  ret = isp_util_send_awb_stats_info(isp_module, applied_frame_meta,
    session_param, is_online);
  if (ret == FALSE)
    ISP_ERR("falied: isp_util_send_awb_stats_info");
  ret = isp_util_send_hdr_curves(isp_module,
    session_param, is_online);
  if (ret == FALSE)
    ISP_ERR("falied: isp_util_send_awb_stats_info");

  /* Send the Lens shading mode */
  bus_msg.type = MCT_BUS_MSG_SET_ISP_LENS_SHADING_MODE;
  bus_msg.size = sizeof(cam_lens_shading_mode_t);
  bus_msg.msg  = (void *)&applied_frame_meta->lens_shading_mode;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("lens_shading_mode error");


  /* Update the metadata with lens roll off table when the shading
     mode is set to on by app */
  if (applied_frame_meta->lens_shading_map_mode == CAM_LENS_SHADING_MAP_MODE_ON) {
    bus_msg.type = MCT_BUS_MSG_SET_ISP_LENS_SHADING_INFO;
    bus_msg.size = sizeof(cam_lens_shading_map_t);
    bus_msg.msg  = (void *)&applied_frame_meta->rolloff_tbl;
    bus_msg.sessionid = session_param->session_id;
    if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
      ISP_ERR("lens_shading_map error");
  }

  /* Send the Lens shading mapmode */
  bus_msg.type = MCT_BUS_MSG_SET_ISP_LENS_SHADING_MAP_MODE;
  bus_msg.size = sizeof(cam_lens_shading_map_mode_t);
  bus_msg.msg  = (void *)&applied_frame_meta->lens_shading_map_mode;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("lens_shading_map_mod error");

  /* Send the isp manual digital gain */
  bus_msg.type = MCT_BUS_MSG_ISP_GAIN;
  bus_msg.size = sizeof(int32_t);
  bus_msg.msg  = (void *)&applied_frame_meta->isp_gain;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("isp_iso_digital_gain error");

  /* Send the Tone map mode */
  bus_msg.type = MCT_BUS_MSG_SET_ISP_TONE_MAP_MODE;
  bus_msg.size = sizeof(cam_tonemap_mode_t);
  bus_msg.msg  = (void *)&applied_frame_meta->tonemap_mode;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("lens_shading_map_mod error");

  /* Send the tone map */
  bus_msg.type = MCT_BUS_MSG_SET_ISP_TONE_MAP;
  bus_msg.size = sizeof(cam_rgb_tonemap_curves);
  bus_msg.msg  = (void *)&applied_frame_meta->tone_map;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("lens_shading_map_mod error");


  /* Profile Tone Curve
    For now, we just use the Gamma Green curve */
  bus_msg.type = MCT_BUS_MSG_PROFILE_TONE_CURVE;
  bus_msg.size = sizeof(cam_profile_tone_curve);
  bus_msg.msg  = (void *)&applied_frame_meta->tone_curve;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("profile_tone_curve error");

  /* Send out the Color Correction mode */
  bus_msg.type = MCT_BUS_MSG_SET_ISP_CC_MODE;
  bus_msg.size = sizeof(cam_color_correct_mode_t);
  bus_msg.msg  = (void *)&applied_frame_meta->cc_mode;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("color_correct_mode error");

  /* Send out the configured CC matrix */
  bus_msg.type = MCT_BUS_MSG_SET_ISP_CC_TRANSFORM;
  bus_msg.size = sizeof(cam_color_correct_matrix_t);
  bus_msg.msg  = (void *)&applied_frame_meta->color_correction_matrix;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("color_correct_matrix error");

  /* Send out the configured CC Gain */
  bus_msg.type = MCT_BUS_MSG_SET_ISP_CC_GAIN;
  bus_msg.size = sizeof(cam_color_correct_gains_t);
  bus_msg.msg  = (void *)&applied_frame_meta->cc_gain;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("color_correct_gains error");

  /* Noise reduction mode  */
  if (applied_frame_meta->abf_mode !=
    CAM_NOISE_REDUCTION_MODE_OFF ||
    bus_msg.metadata_collection_type !=
      MCT_BUS_OFFLINE_METADATA) {
    bus_msg.type = MCT_BUS_MSG_SET_ISP_ABF_MODE;
    bus_msg.size = sizeof(uint8_t);
    bus_msg.msg  = (void *)&applied_frame_meta->abf_mode;
    bus_msg.sessionid = session_param->session_id;
    if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
      ISP_ERR("abf_mode error");
  }

   /* Noise reduction mode  */
  bus_msg.type = MCT_BUS_MSG_SET_ISP_HOT_PIX_MODE;
  bus_msg.size = sizeof(uint8_t);
  bus_msg.msg  = (void *)&applied_frame_meta->hot_pix_mode;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("hot_pix_mode error");

  /* Control mode  */
  bus_msg.type = MCT_BUS_MSG_SET_ISP_CONTROL_MODE;
  bus_msg.size = sizeof(uint8_t);
  bus_msg.msg  = (void *)&applied_frame_meta->app_ctrl_mode;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("app_ctrl_mode error");

  bus_msg.type = MCT_BUS_MSG_ISP_CROP_REGION;
  bus_msg.size = sizeof(cam_crop_region_t);
  bus_msg.msg  = (void *)&applied_frame_meta->crop_window;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("crop_region error");

  /* Update Black level lock state */
  bus_msg.type = MCT_BUS_MSG_SET_ISP_BLACK_LEVEL_LOCK;
  bus_msg.size = sizeof(uint8_t);
  bus_msg.msg  = (void *)&applied_frame_meta->black_level_lock;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("black_level_lock error");

  /* Update cac mode */
  bus_msg.type = MCT_BUS_MSG_SET_CAC_MODE;
  bus_msg.size = sizeof(cam_aberration_mode_t);
  bus_msg.msg  = (void *)&applied_frame_meta->cac_mode;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("cac_mode error");

  bus_msg.type = MCT_BUS_MSG_SET_ISP_CAPTURE_INTENT;
  bus_msg.size = sizeof(uint8_t);
  bus_msg.msg  = (void *)&applied_frame_meta->capture_intent;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("capture_intent error");

  bus_msg.type = MCT_BUS_MSG_SET_ISP_ZOOM_VALUE;
  bus_msg.size = sizeof(cam_zoom_info_t);
  bus_msg.msg  = (void *)&applied_frame_meta->zoom_value;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("zoom value error");

  bus_msg.type = MCT_BUS_MSG_SET_ISP_CONTRAST;
  bus_msg.size = sizeof(int32_t);
  bus_msg.msg  = (void *)&applied_frame_meta->contrast;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("conrast error");

  bus_msg.type = MCT_BUS_MSG_SET_ISP_SATURATION;
  bus_msg.size = sizeof(int32_t);
  bus_msg.msg  = (void *)&applied_frame_meta->saturation;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("saturation error");

  bus_msg.type = MCT_BUS_MSG_SET_EFFECT;
  bus_msg.size = sizeof(cam_effect_mode_type);
  bus_msg.msg  = (void *)&applied_frame_meta->special_effect;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("special_effect error");

  /* Neutral Point */
  if (isp_util_generate_neutral_point(applied_frame_meta)) {
    bus_msg.type = MCT_BUS_MSG_NEUTRAL_COL_POINT;
    bus_msg.size = sizeof(cam_neutral_col_point_t);
    bus_msg.msg  = (void *)&applied_frame_meta->neutral_color_point;
    bus_msg.sessionid = session_param->session_id;
    if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
      ISP_ERR("cam_neutral_col_point error");
  }

  /* Send the low power mode info */
  if (session_param->lpm_info.meta_report_pending == TRUE) {
    bus_msg.type = MCT_BUS_MSG_SET_ISP_LPM_INFO;
    bus_msg.size = sizeof(mct_bus_msg_isp_low_power_mode_t);
    lpm_meta.is_lpm_enabled = session_param->lpm_info.enableLPM;
    bus_msg.msg  = (void *)&lpm_meta;
    bus_msg.sessionid = session_param->session_id;
    session_param->lpm_info.meta_report_pending =  FALSE;
    if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
      ISP_ERR("lpm meta info error");
  }

  bus_msg.type = MCT_BUS_MSG_SET_SENSOR_HDR_MODE;
  bus_msg.size = sizeof(cam_sensor_hdr_type_t);
  bus_msg.msg  = (void *)&applied_frame_meta->sensor_hdr;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("set sensor hdr error");

  bus_msg.type = MCT_BUS_MSG_SET_TINTLESS;
  bus_msg.size = sizeof(int32_t);
  bus_msg.msg  = (void *)&applied_frame_meta->tintless_enable;
  bus_msg.sessionid = session_param->session_id;
  if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
    ISP_ERR("Tintless error");
  return ret;
}

boolean isp_util_send_rdi_only_metadata(mct_module_t *isp_module,
  isp_session_param_t *session_param)
{
  mct_bus_msg_t             bus_msg;
  uint32_t                  frame_id;
  mct_event_control_parm_t  *online_param;

  if (isp_module == NULL || session_param == NULL) {
    ISP_ERR("NULL pointer,isp_module %p, session parm = %p",
      isp_module, session_param);
    return FALSE;
  }

  frame_id = session_param->curr_frame_id;

  bus_msg.metadata_collection_type = MCT_BUS_ONLINE_METADATA;

  bus_msg.type = MCT_BUS_MSG_ISP_CROP_REGION;
  bus_msg.size = sizeof(cam_crop_region_t);
  bus_msg.msg = NULL;
  /*Find the crop region from the saved  events*/
  if (session_param->saved_events.set_params[ISP_SET_META_SCALER_CROP_REGION]) {
    online_param = (mct_event_control_parm_t *)session_param->saved_events.
      set_params[ISP_SET_META_SCALER_CROP_REGION]->u.ctrl_event.control_event_data;
    if (online_param)
      bus_msg.msg = online_param->parm_data;
  }
  bus_msg.sessionid = session_param->session_id;
  if (bus_msg.msg) {
    if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
      ISP_ERR("crop_region error");
  }

  bus_msg.type = MCT_BUS_MSG_SET_ISP_ZOOM_VALUE;
  bus_msg.size = sizeof(cam_zoom_info_t);
  bus_msg.msg = NULL;
  /*Find the zoom from the saved event*/
  if (session_param->saved_events.set_params[ISP_SET_PARM_ZOOM]) {
    online_param = (mct_event_control_parm_t *)session_param->saved_events.
      set_params[ISP_SET_PARM_ZOOM]->u.ctrl_event.control_event_data;
    if (online_param)
      bus_msg.msg = online_param->parm_data;
  }
  bus_msg.sessionid = session_param->session_id;
  if (bus_msg.msg)
    if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
      ISP_ERR("zoom value error");

  bus_msg.type = MCT_BUS_MSG_SET_ISP_CONTRAST;
  bus_msg.size = sizeof(int32_t);
  bus_msg.msg  = NULL;
  /*Find the Contrast from the saved  events*/
  if (session_param->saved_events.set_params[ISP_SET_PARM_CONTRAST]) {
    online_param = (mct_event_control_parm_t *)session_param->saved_events.
      set_params[ISP_SET_PARM_CONTRAST]->u.ctrl_event.control_event_data;
    if (online_param)
      bus_msg.msg = online_param->parm_data;
  }
  bus_msg.sessionid = session_param->session_id;
  if (bus_msg.msg)
    if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
      ISP_ERR("conrast error");

  bus_msg.type = MCT_BUS_MSG_SET_ISP_SATURATION;
  bus_msg.size = sizeof(int32_t);
  bus_msg.msg  = NULL;
  /*Find the Saturation from the saved  events*/
  if (session_param->saved_events.set_params[ISP_SET_PARM_SATURATION]) {
    online_param = (mct_event_control_parm_t *)session_param->saved_events.
      set_params[ISP_SET_PARM_SATURATION]->u.ctrl_event.control_event_data;
    if (online_param)
      bus_msg.msg = online_param->parm_data;
  }
  bus_msg.sessionid = session_param->session_id;
  if (bus_msg.msg)
    if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
      ISP_ERR("saturation error");

  bus_msg.type = MCT_BUS_MSG_SET_EFFECT;
  bus_msg.size = sizeof(cam_effect_mode_type);
  bus_msg.msg  = NULL;
  /*Find the Contrast from the saved  events*/
  if (session_param->saved_events.set_params[ISP_SET_PARM_EFFECT]) {
    online_param = (mct_event_control_parm_t *)session_param->saved_events.
      set_params[ISP_SET_PARM_EFFECT]->u.ctrl_event.control_event_data;
    if (online_param)
      bus_msg.msg = online_param->parm_data;
  }
  bus_msg.sessionid = session_param->session_id;
  if (bus_msg.msg)
    if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
      ISP_ERR("special_effect error");

  bus_msg.type = MCT_BUS_MSG_SET_TINTLESS;
  bus_msg.size = sizeof(int32_t);
  bus_msg.msg  = NULL;
  /*Find the zoom from the saved  events*/
  if (session_param->saved_events.set_params[ISP_SET_PARM_TINTLESS]) {
    online_param = (mct_event_control_parm_t *)session_param->saved_events.
      set_params[ISP_SET_PARM_TINTLESS]->u.ctrl_event.control_event_data;
    if (online_param)
      bus_msg.msg = online_param->parm_data;
  }
  bus_msg.sessionid = session_param->session_id;
  if (bus_msg.msg)
    if (TRUE != isp_util_send_metadata_entry(isp_module, &bus_msg, frame_id))
      ISP_ERR("Tintless error");

  return TRUE;
}



/** isp_util_read_reg_dump
 *    @vfe_fd: fd to talk to kernel
 *    @dump_entry: pointer for the dump
 *    @read_type: how to read from kernel
 *    @read_len: how much to read from kernel
 *
 *    read register dump when receive SOF evt will be the most
 *    acurate timing to dump the register
 *
 *   Return boolean
 **/
boolean isp_util_read_reg_dump(int vfe_fd,
  void *dump_entry, uint32_t read_type,
  uint32_t read_len, uint8_t print_dump)
{
  int rc = 0;
  uint32_t i;
  uint32_t j = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd;
  memset(&cfg_cmd, 0, sizeof(cfg_cmd));
  memset(&reg_cfg_cmd, 0, sizeof(reg_cfg_cmd));
  memset(dump_entry, 0, read_len);

  cfg_cmd.cfg_data = dump_entry;
  cfg_cmd.cmd_len = read_len;
  cfg_cmd.cfg_cmd = (void *)&reg_cfg_cmd;
  cfg_cmd.num_cfg = 1;
  reg_cfg_cmd.u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd.cmd_type = read_type;
  reg_cfg_cmd.u.rw_info.len = read_len;
  reg_cfg_cmd.u.rw_info.reg_offset = 0;
  rc = ioctl(vfe_fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
  if (rc < 0) {
    ISP_ERR("isp red register error = %d %s\n", rc, strerror(errno));
    return FALSE;
  }

  if (print_dump == 1) {
    uint32_t *reg_dump;
    uint32_t num_dump_register =
      read_len / sizeof(uint32_t);
    reg_dump = (uint32_t *)dump_entry;

    for(j = 0; j < (int)(sizeof(reg_dump_range) / sizeof(reg_dump_range[0]));
      j++) {
      /* safety check, if go over register boundry then dont do,
         or it can crash device*/
      if ((reg_dump_range[j][0]/4) > num_dump_register ||
        (reg_dump_range[j][1]/4) > num_dump_register ||
        ((reg_dump_range[j][1]/4) == 0))
        continue;

      ISP_ERR("===== start 0x%x, end 0x%x ======",
        reg_dump_range[j][0] , reg_dump_range[j][1]);

     /* (i % 4): 4 register per line
      *  reg_dump_range[j][0] / 4: number of register
      * (i * 16/4): *16 = to be HEX, /4 = 4 byte per register
      */
      for (i = (reg_dump_range[j][0] / 4); i < (reg_dump_range[j][1] / 4) ;
        i++) {
        if (i % 4  == 0) {
          ISP_ERR("0x%x: %08x  %08x  %08x  %08x",(i * 16) / 4,
            reg_dump[i], reg_dump[i+1], reg_dump[i+2], reg_dump[i+3]);
        }
      }
    }
  }

  return TRUE;
}

/** isp_util_send_submod_enable_to_pipeline:
 *
 *  @session_param: session param handle
 *  @frame_id: frame id
 *
 *
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_send_submod_enable_to_pipeline(isp_resource_t *isp_resource,
  isp_session_param_t *session_param)
{
  boolean    ret = TRUE;
  mct_event_t    event;
  isp_private_event_t          private_event;
  isp_pipeline_t              *pipeline = NULL;
  uint32_t                     hw_id;

  if (!isp_resource || !session_param) {
    ISP_ERR("failed! NULL input isp_resource %p session_param %p",
      isp_resource, session_param);
    return FALSE;
  }
  memset(&private_event, 0, sizeof(private_event));
  private_event.type = ISP_PRIVATE_CURRENT_SUBMOD_ENABLE;

  memset(&event, 0, sizeof(event));
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.identity =  session_param->session_based_ide;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.u.module_event.module_event_data = (void *)&private_event;

  for (hw_id = 0; hw_id < isp_resource->num_isp; hw_id++) {
    pipeline = isp_resource->isp_resource_info[hw_id].isp_pipeline;
    private_event.data = pipeline->submod_enable;

    if (session_param->session_based_ide) {
      ret = isp_util_forward_event_to_internal_pipeline(session_param, &event,
            hw_id);
      if (ret == FALSE) {
        ISP_ERR("failed: ret %d", ret);
        continue;
      }
    } else {
      ret = isp_util_forward_event_to_all_streams(session_param, &event, hw_id);
      if (ret == FALSE) {
        ISP_ERR("failed: ret %d", ret);
        continue;
      }
    }
  }

  return TRUE;
}

/** isp_util_trigger_internal_hw_update_event:
 *
 *  @session_param: session param handle
 *  @frame_id: frame id
 *
 *  Trigger internal isp event to get hw update list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_trigger_internal_hw_update_event(
 isp_session_param_t *session_param, uint32_t identity, uint32_t frame_id,
 isp_hw_id_t *hw_ids, uint32_t num_isp, isp_saved_events_t *saved_events)
{
  boolean                      ret = TRUE;
  mct_event_t                  event;
  isp_hw_id_t                  hw_id = 0;
  struct msm_vfe_cfg_cmd_list *hw_update_list[ISP_HW_MAX];
  isp_meta_t                   meta_dump_params[ISP_HW_MAX];
  isp_dmi_tbl_meta_t           dmi_tbl[ISP_HW_MAX];
  isp_algo_params_t            algo_parm[ISP_HW_MAX];
  boolean                      is_cds_update[ISP_HW_MAX];
  isp_per_frame_meta_t         frame_meta[ISP_HW_MAX];
  isp_hw_update_params_t      *hw_update_params = NULL;
  isp_private_event_t          private_event;
  isp_sub_module_output_t      output;
  uint32_t                     i;

  if (!session_param) {
    ISP_ERR("failed: session_param %p", session_param);
    return FALSE;
  }
  hw_update_params = &session_param->hw_update_params;
  memset(&private_event, 0, sizeof(private_event));
  private_event.type = ISP_PRIVATE_SET_TRIGGER_UPDATE;
  private_event.data = &output;

  memset(&event, 0, sizeof(event));
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.identity = identity;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.u.module_event.module_event_data = (void *)&private_event;

  memset(hw_update_list, 0, sizeof(hw_update_list));

  /* Call internal isp event for all ISP hw */
  for (i = 0; i < num_isp; i++) {
    hw_id = hw_ids[i];
    if (hw_id >= ISP_HW_MAX) {
      ISP_ERR("failed: index out of range: %d", hw_id);
      return FALSE;
    }
    memset(hw_update_params->hw_update_list_params[hw_id].stats_params, 0,
      sizeof(isp_saved_stats_params_t));
    memset(&meta_dump_params[hw_id], 0, sizeof(meta_dump_params[hw_id]));
    memset(&algo_parm[hw_id], 0, sizeof(algo_parm[hw_id]));
    memset(&dmi_tbl[hw_id], 0, sizeof(dmi_tbl[hw_id]));
    memset(&is_cds_update[hw_id], 0, sizeof(is_cds_update[hw_id]));
    memset(&frame_meta[hw_id], 0, sizeof(frame_meta[hw_id]));
    memset(&output, 0, sizeof(output));
    output.stats_params =
      hw_update_params->hw_update_list_params[hw_id].stats_params;
    output.meta_dump_params = &meta_dump_params[hw_id];
    output.dmi_tbl = &dmi_tbl[hw_id];
    output.algo_params = &algo_parm[hw_id];
    output.is_cds_update = &is_cds_update[hw_id];
    output.frame_meta = &frame_meta[hw_id];
    output.metadata_dump_enable =
      session_param->hw_update_params.metadata_dump_enable;
    output.frame_id = frame_id;
    output.saved_events = saved_events;

    memset(&session_param->hdr_data_cur, 0, sizeof(session_param->hdr_data_cur));
    output.hdr_data = &session_param->hdr_data_cur;

    if (session_param->session_based_ide) {
      ret = isp_util_forward_event_to_internal_pipeline(session_param, &event,
        hw_id);
      if (ret == FALSE) {
        ISP_ERR("failed: ret %d", ret);
        continue;
      }
    } else {
      ret = isp_util_forward_event_to_all_streams(session_param, &event, hw_id);
      if (ret == FALSE) {
        ISP_ERR("failed: ret %d", ret);
        continue;
      }
    }
    session_param->lpm_info.enableLPM = output.enableLPM;
    hw_update_list[hw_id] = output.hw_update_list;
  }

  /* Enqueue in hw update list */
  for (i = 0; i < num_isp; i++) {
    hw_id = hw_ids[i];
    if (hw_update_list[hw_id]) {
      ret = isp_hw_update_util_enqueue(hw_update_params,
        hw_update_list[hw_id], hw_id);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_hw_update_list_enqueue hw_id %d", hw_id);
      }
    }
    hw_update_params->hw_update_list_params[hw_id].stats_params->is_valid =
      TRUE;
    hw_update_params->hw_update_list_params[hw_id].meta_dump_parms =
      meta_dump_params[hw_id];
    hw_update_params->hw_update_list_params[hw_id].dmi_tbl=
      dmi_tbl[hw_id];
    hw_update_params->hw_update_list_params[hw_id].algo_parm =
      algo_parm[hw_id];
    hw_update_params->hw_update_list_params[hw_id].frame_meta =
      frame_meta[hw_id];
    ISP_DBG_BUCKET_SWAP("frame_meta local to new %p",
      hw_update_params->hw_update_list_params[hw_id].frame_meta);

    /*per user stream event will go to the same hw stream twice,
      and overwrite the param*/
    if (is_cds_update[hw_id] == TRUE) {
       hw_update_params->hw_update_list_params[hw_id].new_cds_update =
         is_cds_update[hw_id];
    }
  }
  return TRUE;
}

/** isp_util_get_vfe_id:
 *
 *  @hw_id: hw id
 *
 *  Convert isp_hw_id_t to msm_ispif_vfe_intf
 **/
boolean isp_util_get_vfe_id(isp_hw_id_t hw_id)
{
  switch (hw_id) {
  case ISP_HW_0:
    return VFE0;
  case ISP_HW_1:
    return VFE1;
  default:
    return VFE_MAX;
  }
}

/** isp_util_get_stream_src:
 *
 *  @hw_stream: hw stream id
 *
 *  Convert isp_hw_streamid_t to msm_vfe_axi_stream_src
 **/
boolean isp_util_get_stream_src(isp_hw_streamid_t hw_stream)
{
  switch (hw_stream) {
  case ISP_HW_STREAM_VIEWFINDER:
    return PIX_VIEWFINDER;
  case ISP_HW_STREAM_ENCODER:
    return PIX_ENCODER;
  case ISP_HW_STREAM_VIDEO:
    return PIX_VIDEO;
  default:
    return VFE_AXI_SRC_MAX;
  }
}

/** isp_util_get_axi_src_type:
 *
 *    @fmt: format
 *    @is_encoder:
 *
 *    Get stream format to axi path conversion
 *
 *    Return stream format
 **/
enum msm_vfe_axi_stream_src isp_util_get_axi_src_type(cam_format_t output_fmt,
  uint8_t stream_id)
{
  ISP_DBG("<camif_raw> fmt %d stream id %d", output_fmt, stream_id);
  switch (output_fmt) {
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
  case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GBRG:
  case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GRBG:
  case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_RGGB:
  case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_BGGR:
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GREY:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GREY:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GREY:
    return CAMIF_RAW;

  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GREY:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GREY:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GREY:
    return IDEAL_RAW;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG:
  case CAM_FORMAT_JPEG_RAW_8BIT:
    return RDI_INTF_0;

  case CAM_FORMAT_META_RAW_8BIT:
  case CAM_FORMAT_META_RAW_10BIT:
    return RDI_INTF_1;

  default: {
    if (stream_id == ISP_HW_STREAM_ENCODER)
      return PIX_ENCODER;
    else if (stream_id == ISP_HW_STREAM_VIEWFINDER)
      return PIX_VIEWFINDER;
    else if (stream_id == ISP_HW_STREAM_VIDEO)
      return PIX_VIDEO;
  }
    break;
  }

  return VFE_AXI_SRC_MAX;
}

/** isp_util_send_BE_info:
 *
 *  @module: mct module handle
 *  @session_param: handle to isp_session_param_t
 *  @sof_event: sof event
 *  @saved_params: saved params
 *
 *  Broadcast stream crop event downstream
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_send_BE_info(mct_module_t *module,
  isp_session_param_t *session_param, uint32_t identity,
  mct_bus_msg_isp_sof_t *sof_event, isp_saved_stats_params_t *stats_params)
{
  q3a_be_info_t               be_info;
  mct_event_t                 be_stats_event;
  int                         i = 0;
  isp_hw_id_t                 hw_index = 0;
  boolean                     ret;
  isp_saved_stats_params_t   *used_stats_params = NULL;

  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    if (session_param->hw_id[hw_index] != ISP_HW_MAX) {
      break;
    }
  }
  if (hw_index == ISP_HW_MAX) {
    ISP_ERR(" Error! no isp found");
    return FALSE;
  }
  used_stats_params = &stats_params[session_param->hw_id[hw_index]];
  be_info = used_stats_params->be_info;

  memset(&be_stats_event, 0, sizeof(mct_event_t));
  be_stats_event.u.module_event.current_frame_id = sof_event->frame_id;
  be_stats_event.direction = MCT_EVENT_DOWNSTREAM;
  be_stats_event.type = MCT_EVENT_MODULE_EVENT;
  be_stats_event.identity = identity;
  be_stats_event.u.module_event.type = MCT_EVENT_MODULE_BE_STATS_INFO;
  be_stats_event.u.module_event.module_event_data = (void *)&be_info;
  ret =  isp_util_forward_event_downstream_to_type(module, &be_stats_event,
    MCT_PORT_CAPS_STATS);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_port_send_event_to_peer event->type %d",
      be_stats_event.type);
    return FALSE;
  }
  return TRUE;
}

/** isp_util_broadcast_crop_info:
 *
 *  @module: mct module handle
 *  @session_param: handle to isp_session_param_t
 *  @sof_event: sof event
 *  @saved_params: saved params
 *
 *  Broadcast stream crop event downstream
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_broadcast_crop_info(mct_module_t *module,
  isp_session_param_t *session_param, unsigned int frame_id,
  struct timeval *timestamp, isp_zoom_params_t *zoom_params_arr,
  cam_zoom_info_t *zoom, boolean is_online)
{
  uint32_t                         i = 0, j = 0;
  boolean                          ret = TRUE;
  mct_bus_msg_stream_crop_t        stream_crop;
  mct_event_t                      mct_event;
  isp_crop_window_info_t          *crop_window = NULL;
  isp_t                           *isp = NULL;
  isp_hw_id_t                      isp_id = 0;
  uint8_t                          pipeline_applying_delay = 0;
  mct_bus_msg_session_crop_info_t  session_crop_info;
  isp_stream_param_t              *stream_param = NULL;
  isp_pipeline_t                  *isp_pipeline = NULL;

  if (!module || !session_param || !timestamp || !zoom_params_arr || !zoom) {
    ISP_ERR("failed: %p %p %p %p %p", module, session_param, timestamp,
      zoom_params_arr, zoom);

    return FALSE;
  }

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  isp_pipeline = isp->isp_resource.isp_resource_info[session_param->hw_id[0]].
    isp_pipeline;
  if (!isp_pipeline){
    ISP_ERR("failed: isp_pipeline is %p ", isp_pipeline);
    return FALSE;
  }

  memset(&mct_event, 0, sizeof(mct_event));
  memset(&stream_crop, 0, sizeof(stream_crop));
  memset(&session_crop_info, 0, sizeof(session_crop_info));

  mct_event.u.module_event.type = MCT_EVENT_MODULE_STREAM_CROP;
  mct_event.u.module_event.module_event_data = (void *)&stream_crop;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.direction = MCT_EVENT_DOWNSTREAM;
  /* Since crop info is sent after the HW update is done, it needs to carry
     a frame ID with respect to applying dealy */
  pipeline_applying_delay = isp_pipeline->pipeline_applying_delay;

  if (frame_id >= pipeline_applying_delay) {
    mct_event.u.module_event.current_frame_id = frame_id -
      pipeline_applying_delay;
  }
  stream_crop.frame_id = frame_id;
  stream_crop.timestamp = *timestamp;

  isp_id = session_param->hw_id[0];

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (!zoom_params_arr[i].identity) {
      break;
    }
    mct_event.identity = zoom_params_arr[i].identity;

    stream_param = NULL;
    ret = isp_util_get_stream_params(session_param, zoom_params_arr[i].identity,
      &stream_param);
    if ((ret == FALSE ) || !stream_param) {
      ISP_ERR("failed: ret %d stream_param %p", ret, stream_param);
      continue;
    }

    stream_crop.session_id = ISP_GET_SESSION_ID(zoom_params_arr[i].identity);
    stream_crop.stream_id = ISP_GET_STREAM_ID(zoom_params_arr[i].identity);

    if (zoom->is_stream_zoom_info_valid) {
      for (j = 0; j < zoom->num_streams; j++) {
        if (stream_param->stream_info.stream_type ==
          zoom->stream_zoom_info[j].stream_type) {
          stream_crop.stream_zoom = zoom->stream_zoom_info[j].stream_zoom;
          break;
         }
      }
      crop_window = &zoom_params_arr[i].crop_window_calculated;
    } else {
      stream_crop.stream_zoom = zoom->user_zoom;
      crop_window = &zoom_params_arr[i].crop_window;
    }

    if(!crop_window) {
      ISP_ERR("failed: crop_window");
      continue;
    }
    stream_crop.crop_out_x = crop_window->crop_out_x;
    stream_crop.crop_out_y = crop_window->crop_out_y;
    stream_crop.x = crop_window->x;
    stream_crop.y = crop_window->y;

    stream_crop.user_zoom = zoom->user_zoom;
    stream_crop.scale_ratio = zoom_params_arr[i].scaling_ratio;

    ret = isp_resource_calculate_roi_map(&isp->isp_resource, isp_id,
      zoom_params_arr, &stream_crop, zoom_params_arr[i].identity);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_resource_calculate_roi_map");
    }

    ISP_DBG("x %d, y %d crop x %d crop y %d frame id %d xmap %d,"
        "ymap %d, xmap w %d ymap h %d user zoom %d stream zoom %d"
        " stream_ type %d isp zoom %d scaling ratio %f",
        stream_crop.x, stream_crop.y, stream_crop.crop_out_x,
        stream_crop.crop_out_y, stream_crop.frame_id, stream_crop.x_map,
        stream_crop.y_map, stream_crop.width_map, stream_crop.height_map,
        stream_crop.user_zoom, stream_crop.stream_zoom,
        zoom->stream_zoom_info[j].stream_type,
        zoom->stream_zoom_info[j].isp_zoom,
        stream_crop.scale_ratio);

    ISP_DBG("ide %x camif %d %d scaler %d %d fov %d %d %d %d",
      mct_event.identity,
      zoom_params_arr[i].camif_output.width,
      zoom_params_arr[i].camif_output.height,
      zoom_params_arr[i].scaler_output.width,
      zoom_params_arr[i].scaler_output.height,
      zoom_params_arr[i].fov_output.x,
      zoom_params_arr[i].fov_output.y,
      zoom_params_arr[i].fov_output.crop_out_x,
      zoom_params_arr[i].fov_output.crop_out_y);

    ISP_DBG("ide %x scale ratio %f stream crop %d %d %d %d frame_id= %d",
      mct_event.identity, zoom_params_arr[i].scaling_ratio,
      stream_crop.x, stream_crop.y, stream_crop.crop_out_x,
      stream_crop.crop_out_y, stream_crop.frame_id);

    ret = isp_util_forward_event_downstream_to_all_types(module, &mct_event);
    if (ret == FALSE) {
      ISP_ERR("failed: ide %x MCT_EVENT_MODULE_STREAM_CROP",
        mct_event.identity);
    }

    if ((!stream_param->need_divert || !is_online) &&
         stream_param->stream_state == ISP_STREAM_STATE_ACTIVE) {
      session_crop_info.crop_info[session_crop_info.num_of_streams] =
        stream_crop;
      session_crop_info.num_of_streams++;
      ISP_DBG("session_crop_info.num_of_streams = %d",
          session_crop_info.num_of_streams);
    }

    /* Output ISP & PostProc crop to metadata.
     * Required by dual-camera to calculate FOV changes.
     */
    if(stream_param->stream_info.stream_type == CAM_STREAM_TYPE_SNAPSHOT) {
      uint32_t                  rv;
      mct_bus_msg_t             bus_msg;
      cam_stream_crop_info_t    crop_info;

      /* ISP_CROP is the "map" information sent to CPP,
       * we require an extra message as we need to know start resolution
       */
      memset(&crop_info, 0, sizeof(crop_info));
      crop_info.stream_id       = stream_crop.stream_id;
      crop_info.roi_map.left    = 0;
      crop_info.roi_map.top     = 0;
      crop_info.roi_map.width   = zoom_params_arr[i].scaler_output.width;
      crop_info.roi_map.height  = zoom_params_arr[i].scaler_output.height;
      crop_info.crop.left       = zoom_params_arr[i].fov_output.x;
      crop_info.crop.top        = zoom_params_arr[i].fov_output.y;
      crop_info.crop.width      = zoom_params_arr[i].fov_output.crop_out_x;
      crop_info.crop.height     = zoom_params_arr[i].fov_output.crop_out_y;
      crop_info.user_zoom       = zoom->user_zoom;
      crop_info.scale_ratio     = zoom_params_arr[i].scaling_ratio;

      for (j = 0; j < zoom->num_streams; j++) {
        if (zoom->stream_zoom_info[j].stream_type ==
          CAM_STREAM_TYPE_SNAPSHOT)
        crop_info.stream_zoom = zoom->stream_zoom_info[j].stream_zoom;
      }

      ISP_DBG("CROP_INFO_ISP: frame_id %d str_id %d crop (%d, %d, %d,%d)"
        "==> (%d,%d,%d,%d) uzoom %d strmzoom %d scaling ration %f",
        frame_id,
        crop_info.stream_id,
        crop_info.roi_map.left, crop_info.roi_map.top,
        crop_info.roi_map.width, crop_info.roi_map.height,
        crop_info.crop.left, crop_info.crop.top,
        crop_info.crop.width, crop_info.crop.height,
        crop_info.user_zoom, crop_info.stream_zoom,
        crop_info.scale_ratio);

      memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
      bus_msg.sessionid = stream_crop.session_id;
      bus_msg.type = MCT_BUS_MSG_SNAP_CROP_INFO_ISP;
      bus_msg.size = sizeof(crop_info);
      bus_msg.msg = &crop_info;

      rv = isp_util_send_metadata_entry(module, &bus_msg, frame_id);
      if (rv == FALSE) {
        ISP_ERR("Error: isp_util_send_metadata_entry failed MCT_BUS_MSG_CROP_INFO_ISP");
      }

      /* Post-proc crop info to support cases where CPP
       * is not used.  Will be overwritten by CPP, as it
       * does not include:
       *   crop for aspect ratio changes (calculated in CPP)
       *   crop for image-stabilization (not typical for snapshots)
       */
      memset(&crop_info, 0, sizeof(crop_info));
      crop_info.stream_id       = stream_crop.stream_id;
      crop_info.roi_map.left    = 0;
      crop_info.roi_map.top     = 0;
      crop_info.roi_map.width   = zoom_params_arr[i].isp_output.width;
      crop_info.roi_map.height  = zoom_params_arr[i].isp_output.height;
      crop_info.crop.left       = stream_crop.x;
      crop_info.crop.top        = stream_crop.y;
      crop_info.crop.width      = stream_crop.crop_out_x;
      crop_info.crop.height     = stream_crop.crop_out_y;

      ISP_DBG("CROP_INFO_PP: str_id %d crop (%d, %d, %d,%d) ==> (%d,%d,%d,%d)",
        crop_info.stream_id,
        crop_info.roi_map.left, crop_info.roi_map.top,
        crop_info.roi_map.width, crop_info.roi_map.height,
        crop_info.crop.left, crop_info.crop.top,
        crop_info.crop.width, crop_info.crop.height);

      memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
      bus_msg.sessionid = stream_crop.session_id;
      bus_msg.type = MCT_BUS_MSG_SNAP_CROP_INFO_PP;
      bus_msg.size = sizeof(crop_info);
      bus_msg.msg = &crop_info;

      rv = isp_util_send_metadata_entry(module, &bus_msg, frame_id);
      if (rv == FALSE) {
        ISP_ERR("Error: isp_util_send_metadata_entry failed MCT_BUS_MSG_CROP_INFO_CPP");
      }
    }
  }
  for (i = 0; i < session_crop_info.num_of_streams; i++) {
    ISP_DBG("i %d crop %d %d %d %d", i,
       session_crop_info.crop_info[i].x, session_crop_info.crop_info[i].y,
       session_crop_info.crop_info[i].crop_out_x,
       session_crop_info.crop_info[i].crop_out_y);
  }
  isp_util_send_crop_bus_message(module, session_param, &session_crop_info,
    is_online);

  return ret;
}

/** isp_util_free_all_meta_dump_entry:
 *
 *  @module: isp module
 *  @session_param : isp session parameters
 *
 *  update hw parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_free_all_meta_dump_entry(mct_module_t *module,
  isp_session_param_t *session_param)
{
  boolean                      ret = TRUE;
  isp_hw_update_params_t      *hw_update_params = NULL;
  isp_parser_params_t         *parser_params = NULL;
  isp_hw_update_list_params_t *hw_update_list_params = NULL;
  uint32_t                     i = 0;
  uint32_t                     j = 0;

  if (!module || !session_param) {
    ISP_ERR("failed: module %p session_param %p", module, session_param);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;
  for (i = 0; i < session_param->num_isp; i++) {
    for (j = 0; j < ISP_METADUMP_MAX_NUM; j++) {
      hw_update_list_params =
        &hw_update_params->hw_update_list_params[session_param->hw_id[i]];
      hw_update_list_params->applied_meta_dump_parms.
        meta_entry[j].len = 0;
      hw_update_list_params->meta_dump_parms.
        meta_entry[j].len = 0;
    }
  }

  return ret;
}

/** isp_util_free_control_event_data:
 *
 *  @ctrl_event: control event handle
 *
 *  Free control event data
 *
 *  Return void
 **/
static void isp_util_free_control_event_data(mct_event_control_t *ctrl_event)
{
  uint32_t i = 0;

  if (!ctrl_event) {
    ISP_ERR("failed: ctrl_event %p", ctrl_event);
    return;
  }
  switch (ctrl_event->type) {
  case MCT_EVENT_CONTROL_SET_PARM: {
    mct_event_control_parm_t *control_parm =
      (mct_event_control_parm_t *)ctrl_event->control_event_data;
    if (control_parm) {
      free(control_parm->parm_data);
    }
  }
    break;

  case MCT_EVENT_CONTROL_SET_SUPER_PARM: {
    mct_event_super_control_parm_t *control_parm =
      (mct_event_super_control_parm_t *)ctrl_event->control_event_data;
    if (control_parm) {
      for (i = 0; i < control_parm->num_of_parm_events; i++) {
        if (control_parm->parm_events[i].parm_data)
          free(control_parm->parm_events[i].parm_data);
      }
      free(control_parm->parm_events);
    }
  }
    break;

  default:
    break;
  }
} /* isp_util_free_control_event_data */

/** isp_util_free_module_event_data:
 *
 *  @module_event: module event handle
 *
 *  Free module event data
 *
 *  Return void
 **/
static void isp_util_free_module_event_data(mct_event_module_t *module_event)
{
  if (!module_event) {
    ISP_ERR("failed: module_event %p", module_event);
    return;
  }

  switch (module_event->type) {
  case MCT_EVENT_MODULE_SET_AF_ROLLOFF_PARAMS: {
    af_rolloff_info_t *af_rolloff_info =
      (af_rolloff_info_t *)module_event->module_event_data;
    if (af_rolloff_info) {
      free(af_rolloff_info->rolloff_tables_macro);
    }
  }
    break;

  default:
    break;
  }
} /* isp_util_free_module_event_data */

/** isp_util_free_event:
 *
 *  @event: event handle
 *
 *  Free event
 *
 *  Return TRUE
 **/
static boolean isp_util_free_event(mct_event_t *event)
{
  if (!event) {
    ISP_ERR("failed: event %p", event);
    return TRUE;
  }

  if (event->type == MCT_EVENT_CONTROL_CMD) {
    isp_util_free_control_event_data(&event->u.ctrl_event);
    if (event->u.ctrl_event.control_event_data != NULL) {
      free(event->u.ctrl_event.control_event_data);
    }
  } else if (event->type == MCT_EVENT_MODULE_EVENT) {
    isp_util_free_module_event_data(&event->u.module_event);
    if (event->u.module_event.module_event_data != NULL) {
      free(event->u.module_event.module_event_data);
    }
  }
  free(event);
  return TRUE;
} /* isp_util_free_event */

/** isp_util_free_saved_events:
 *
 *  @saved_events: saved events
 *
 *  Free saved params
 *
 *  Return void
 **/
void isp_util_free_saved_events(isp_saved_events_t *saved_events)
{
  uint32_t i = 0;

  if (!saved_events) {
    ISP_ERR("failed: %p", saved_events);
    return;
  }

  /* Free set params */
  for (i = 0; i < ISP_SET_MAX; i++) {
    if (saved_events->set_params[i]) {
      isp_util_free_event(saved_events->set_params[i]);
      saved_events->set_params[i] = NULL;
      saved_events->set_params_valid[i] = FALSE;
    }
  }

  /* Free module events */
  for (i = 0; i < ISP_MODULE_EVENT_MAX; i++) {
    if (saved_events->module_events[i]) {
      isp_util_free_event(saved_events->module_events[i]);
      saved_events->module_events[i] = NULL;
      saved_events->module_events_valid[i] = FALSE;
    }
  }
}


/** isp_util_saved_event_reset_valid_flag:
 *
 *  @saved_events: saved events
 *
 *  Reset valid flag of all saved events.=
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_saved_event_reset_valid_flag(isp_saved_events_t *saved_events)
{
  uint32_t            i = 0;

  if (!saved_events) {
    ISP_ERR("failed: %p", saved_events);
    return FALSE;
  }

  /* Reset set params */
  for (i = 0; i < ISP_SET_MAX; i++) {
    if (saved_events->set_params[i]) {
      saved_events->set_params_valid[i] = TRUE;
    }
  }

  /* Reset module events */
  for (i = 0; i < ISP_MODULE_EVENT_MAX; i++) {
    if (saved_events->module_events[i]) {
      saved_events->module_events_valid[i] = TRUE;
    }
  }

  return TRUE;
}

/** isp_util_forward_saved_events:
 *
 *  @session_param: session param
 *  @saved_events: handle to saved events
 *  @reset_valid_flag: flag to indicate whether event valid flag
 *                   should be reset
 *
 *  Forward saved module and set params downstream to sub
 *  modules
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_clear_saved_events(isp_saved_events_t *saved_events)
{
  boolean             ret = TRUE;
  uint32_t            i = 0;

  if (!saved_events) {
    ISP_ERR("failed: %p", saved_events);
    return FALSE;
  }

  /* Forward set params */
  for (i = 0; i < ISP_SET_MAX; i++) {
    saved_events->set_params_valid[i] = FALSE;
  }

  /* Forward module events */
  for (i = 0; i < ISP_MODULE_EVENT_MAX; i++) {
    saved_events->module_events_valid[i] = FALSE;
  }

  return ret;
}

/** isp_util_forward_saved_events:
 *
 *  @session_param: session param
 *  @saved_events: handle to saved events
 *  @reset_valid_flag: flag to indicate whether event valid flag
 *                   should be reset
 *
 *  Forward saved module and set params downstream to sub
 *  modules
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_forward_saved_events(isp_session_param_t *session_param,
  isp_saved_events_t *saved_events, boolean reset_valid_flag)
{
  boolean             ret = TRUE;
  uint32_t            i = 0;

  if (!session_param || !saved_events) {
    ISP_ERR("failed: %p %p", session_param, saved_events);
    return FALSE;
  }

  /* Forward set params */
  for (i = 0; i < ISP_SET_MAX; i++) {
    if (saved_events->set_params[i] &&
      (saved_events->set_params_valid[i] == TRUE)) {
        ATRACE_BEGIN_SNPRINTF(30, "ISP:SET_PARAM %d",i);
      if (session_param->session_based_ide) {
        ret = isp_util_forward_event_to_session_based_stream_all_int_pipelines(
          session_param, saved_events->set_params[i]);
        ATRACE_END();
        if (ret == FALSE) {
          ISP_ERR("failed: set parm type %d", i);
        } else {
          saved_events->set_params_valid[i] = FALSE;
        }
      } else {
        ret = isp_util_forward_event_to_all_streams_all_internal_pipelines(
          session_param, saved_events->set_params[i]);
        if (ret == FALSE) {
          ISP_ERR("failed: set parm type %d", i);
        } else if (reset_valid_flag == TRUE) {
          saved_events->set_params_valid[i] = FALSE;
        }
      }
    }
  }

  /* Forward module events */
  for (i = 0; i < ISP_MODULE_EVENT_MAX; i++) {
    if (saved_events->module_events[i] &&
      (saved_events->module_events_valid[i] == TRUE)) {
      if (session_param->session_based_ide) {
        ret = isp_util_forward_event_to_session_based_stream_all_int_pipelines(
          session_param, saved_events->module_events[i]);
        if (ret == FALSE) {
          ISP_ERR("failed: module event type %d", i);
        } else {
          saved_events->module_events_valid[i] = FALSE;
        }
      } else {
        ret = isp_util_forward_event_to_all_streams_all_internal_pipelines(
          session_param, saved_events->module_events[i]);
        if (ret == FALSE) {
          ISP_ERR("failed: module event type %d", i);
        } else if (reset_valid_flag == TRUE) {
          saved_events->module_events_valid[i] = FALSE;
        }
      }
    }
  }

  return ret;
}

/** isp_util_convert_set_parm_event_type:
 *
 *  @mct_type: HAL set parm event type
 *  @isp_type: isp internal event type
 *  @set_parm_size: set parm size
 *
 *  Convert HAL set parm to isp internal event
 *
 *  Return TRUE on Success and FALSE on failure
 **/
boolean isp_util_convert_set_parm_event_type(cam_intf_parm_type_t hal_type,
  isp_set_param_type_t *isp_type, uint32_t *set_parm_size)
{
  if (!isp_type || !set_parm_size) {
    ISP_ERR("failed: isp_type %p set_parm_size %p", isp_type, set_parm_size);
    return FALSE;
  }

  switch (hal_type) {
  case CAM_INTF_PARM_CONTRAST:
    *isp_type = ISP_SET_PARM_CONTRAST;
    *set_parm_size = sizeof(int32_t);
    break;

  case CAM_INTF_PARM_SATURATION:
    *isp_type = ISP_SET_PARM_SATURATION;
    *set_parm_size = sizeof(int32_t);
    break;

  case CAM_INTF_PARM_USERZOOM:
    *isp_type = ISP_SET_PARM_ZOOM;
    *set_parm_size = sizeof(cam_zoom_info_t);
    break;

  case CAM_INTF_META_SCALER_CROP_REGION:
    *isp_type = ISP_SET_META_SCALER_CROP_REGION;
    *set_parm_size = sizeof(cam_crop_region_t);
    break;

  case CAM_INTF_PARM_EFFECT:
    *isp_type = ISP_SET_PARM_EFFECT;
    *set_parm_size = sizeof(int32_t);
    break;

  case CAM_INTF_META_MODE:
    *isp_type = ISP_SET_META_MODE;
    *set_parm_size = sizeof(uint8_t);
    break;

  case CAM_INTF_META_AEC_MODE:
    *isp_type = ISP_SET_AEC_MODE;
    *set_parm_size = sizeof(uint8_t);
    break;

  case CAM_INTF_META_ISP_SENSITIVITY:
    *isp_type = ISP_SET_MANUAL_GAIN;
    *set_parm_size = sizeof(int32_t);
    break;

  case CAM_INTF_PARM_WHITE_BALANCE:
    *isp_type = ISP_SET_PARM_WHITE_BALANCE;
    *set_parm_size = sizeof(uint8_t);
    break;

  case CAM_INTF_META_NOISE_REDUCTION_MODE:
    *isp_type = ISP_SET_META_NOISE_REDUCTION_MODE;
    *set_parm_size = sizeof(uint8_t);
    break;

  case CAM_INTF_META_HOTPIXEL_MODE:
    *isp_type = ISP_SET_INTF_HOTPIXEL_MODE;
    *set_parm_size = sizeof(uint8_t);
    break;

  case CAM_INTF_META_LENS_SHADING_MAP_MODE:
    *isp_type = ISP_SET_META_LENS_SHADING_MAP_MODE;
    *set_parm_size = sizeof(uint8_t);
    break;

  case CAM_INTF_META_SHADING_MODE:
    *isp_type = ISP_SET_META_LENS_SHADING_MODE;
    *set_parm_size = sizeof(uint8_t);
    break;

  case CAM_INTF_META_TONEMAP_MODE:
    *isp_type = ISP_SET_META_TONEMAP_MODE;
    *set_parm_size = sizeof(uint8_t);
    break;

  case CAM_INTF_PARM_CAC:
    *isp_type = ISP_SET_INTF_PARM_CAC;
    *set_parm_size = sizeof(uint8_t);
    break;

  case CAM_INTF_META_TONEMAP_CURVES:
    *isp_type = ISP_SET_META_TONEMAP_CURVES;
    *set_parm_size = sizeof(cam_rgb_tonemap_curves);
    break;

  case CAM_INTF_META_COLOR_CORRECT_MODE:
    *isp_type = ISP_SET_META_COLOR_CORRECT_MODE;
    *set_parm_size = sizeof(uint8_t);
    break;

  case CAM_INTF_META_COLOR_CORRECT_GAINS:
    *isp_type = ISP_SET_META_COLOR_CORRECT_GAINS;
    *set_parm_size = sizeof(cam_color_correct_gains_t);
    break;

  case CAM_INTF_META_COLOR_CORRECT_TRANSFORM:
    *isp_type = ISP_SET_META_COLOR_CORRECT_TRANSFORM;
    *set_parm_size = sizeof(cam_color_correct_matrix_t);
    break;

  case CAM_INTF_META_BLACK_LEVEL_LOCK:
    *isp_type = ISP_SET_META_BLACK_LEVEL_LOCK;
    *set_parm_size = sizeof(uint8_t);
    break;

  case CAM_INTF_PARM_BESTSHOT_MODE:
    *isp_type = ISP_SET_PARM_BESTSHOT_MODE;
    *set_parm_size = sizeof(cam_scene_mode_type);
    break;

  case CAM_INTF_PARM_SCE_FACTOR:
    *isp_type = ISP_SET_PARM_SCE_FACTOR;
    *set_parm_size = sizeof(int32_t);
    break;

  case CAM_INTF_PARM_HFR:
    *isp_type = ISP_SET_PARM_HFR;
    *set_parm_size = sizeof(int32_t);
    break;

  case CAM_INTF_PARM_DIS_ENABLE:
    *isp_type = ISP_SET_PARM_DIS_ENABLE;
    *set_parm_size = sizeof(int32_t);
    break;

  case CAM_INTF_PARM_ZSL_MODE:
    *isp_type = ISP_SET_PARM_ZSL_MODE;
    *set_parm_size = sizeof(int32_t);
    break;

  case CAM_INTF_PARM_SENSOR_HDR:
    *isp_type = ISP_SET_PARM_SENSOR_HDR_MODE;
    *set_parm_size = sizeof(cam_sensor_hdr_type_t);
    break;

  case CAM_INTF_PARM_FRAMESKIP:
    *isp_type = ISP_SET_PARM_FRAMESKIP;
    *set_parm_size = sizeof(int32_t);
    break;

  case CAM_INTF_PARM_RECORDING_HINT:
    *isp_type = ISP_SET_PARM_RECORDING_HINT;
    *set_parm_size = sizeof(boolean);
    break;

  case CAM_INTF_PARM_TINTLESS:
    *isp_type = ISP_SET_PARM_TINTLESS;
    *set_parm_size = sizeof(int32_t);
    break;

  case CAM_INTF_PARM_SET_VFE_COMMAND:
    *isp_type = ISP_SET_PARM_SET_VFE_COMMAND;
    *set_parm_size = sizeof(tune_cmd_t);
    break;

  case CAM_INTF_PARM_VT:
    *isp_type = ISP_SET_PARM_VT;
    *set_parm_size = sizeof(int32_t);
    break;

  case CAM_INTF_META_USE_AV_TIMER:
    *isp_type = ISP_SET_PARM_VT;
    *set_parm_size = sizeof(int32_t);
    break;

  case CAM_INTF_PARM_HDR:
    *isp_type = ISP_SET_PARM_HDR;
    *set_parm_size = sizeof(cam_exp_bracketing_t);
    break;

  case CAM_INTF_PARM_UPDATE_DEBUG_LEVEL:
    *isp_type = ISP_SET_PARM_UPDATE_DEBUG_LEVEL;
    *set_parm_size = sizeof(uint32_t);
    break;

  case CAM_INTF_PARM_TONE_MAP_MODE:
    *isp_type = ISP_SET_INTF_PARM_LTM_MODE;
    *set_parm_size = sizeof(uint32_t);
    break;

  case CAM_INTF_PARM_DUAL_LED_CALIBRATION:
    *isp_type = ISP_SET_DUAL_LED_CALIBRATION;
    *set_parm_size = sizeof(uint32_t);
    break;

  case CAM_INTF_PARM_LONGSHOT_ENABLE:
    *isp_type = ISP_SET_PARM_LONGSHOT_ENABLE;
    *set_parm_size = sizeof(int8_t);
    break;

  case CAM_INTF_META_STATS_HISTOGRAM_MODE:
  case CAM_INTF_PARM_HISTOGRAM:
      *isp_type = ISP_SET_PARM_BHIST_META_ENABLE;
      *set_parm_size = sizeof(int32_t);
    break;

  case CAM_INTF_PARM_BOKEH_MODE:
      *isp_type = ISP_SET_PARM_BOKEH_MODE;
      *set_parm_size = sizeof(int32_t);
    break;

  default:
    *isp_type = ISP_SET_MAX;
    *set_parm_size = 0;
    break;
  }

  return TRUE;
}

/** isp_util_clear_invalid_saved_event:
 *
 *  @saved_events: handle to saved_events
 *
 *  stats_config_update event should not be restored during
 *  mode change. Free it if it is saved
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_clear_invalid_saved_event(isp_saved_events_t *saved_events)
{
  if (!saved_events) {
    ISP_ERR("failed: saved_events %p", saved_events);
    return FALSE;
  }

  if (saved_events->module_events[ISP_MODULE_EVENT_STATS_AEC_CONFIG_UPDATE]) {
    isp_util_free_event(
      saved_events->module_events[ISP_MODULE_EVENT_STATS_AEC_CONFIG_UPDATE]);
    saved_events->module_events[ISP_MODULE_EVENT_STATS_AEC_CONFIG_UPDATE] =
      NULL;
  }
  saved_events->module_events_valid[ISP_MODULE_EVENT_STATS_AEC_CONFIG_UPDATE] =
    FALSE;

  if (saved_events->module_events[ISP_MODULE_EVENT_STATS_AWB_CONFIG_UPDATE]) {
    isp_util_free_event(
      saved_events->module_events[ISP_MODULE_EVENT_STATS_AWB_CONFIG_UPDATE]);
    saved_events->module_events[ISP_MODULE_EVENT_STATS_AWB_CONFIG_UPDATE] =
      NULL;
  }
  saved_events->module_events_valid[ISP_MODULE_EVENT_STATS_AWB_CONFIG_UPDATE] =
    FALSE;

  if (saved_events->module_events[ISP_MODULE_EVENT_STATS_AF_CONFIG_UPDATE]) {
    isp_util_free_event(
      saved_events->module_events[ISP_MODULE_EVENT_STATS_AF_CONFIG_UPDATE]);
    saved_events->module_events[ISP_MODULE_EVENT_STATS_AF_CONFIG_UPDATE] =
      NULL;
  }
  saved_events->module_events_valid[ISP_MODULE_EVENT_STATS_AF_CONFIG_UPDATE] =
    FALSE;

  if (saved_events->module_events[ISP_MODULE_EVENT_AF_EXP_COMPENSATE]) {
    isp_util_free_event(
      saved_events->module_events[ISP_MODULE_EVENT_AF_EXP_COMPENSATE]);
    saved_events->module_events[ISP_MODULE_EVENT_AF_EXP_COMPENSATE] =
      NULL;
  }
  saved_events->module_events_valid[ISP_MODULE_EVENT_AF_EXP_COMPENSATE] =
    FALSE;


  if (saved_events->module_events[ISP_MODULE_EVENT_STATS_RS_CONFIG_UPDATE]) {
    isp_util_free_event(
      saved_events->module_events[ISP_MODULE_EVENT_STATS_RS_CONFIG_UPDATE]);
    saved_events->module_events[ISP_MODULE_EVENT_STATS_RS_CONFIG_UPDATE] =
      NULL;
  }
  saved_events->module_events_valid[ISP_MODULE_EVENT_STATS_RS_CONFIG_UPDATE] =
    FALSE;

  if (saved_events->module_events[ISP_MODULE_EVENT_CDS_REQUEST]) {
    isp_util_free_event(
      saved_events->module_events[ISP_MODULE_EVENT_CDS_REQUEST]);
    saved_events->module_events[ISP_MODULE_EVENT_CDS_REQUEST] = NULL;
  }
  saved_events->module_events_valid[ISP_MODULE_EVENT_CDS_REQUEST] = FALSE;

  if (saved_events->module_events[ISP_MODULE_EVENT_SET_CHROMATIX_PTR]) {
    isp_util_free_event(
      saved_events->module_events[ISP_MODULE_EVENT_SET_CHROMATIX_PTR]);
    saved_events->module_events[ISP_MODULE_EVENT_SET_CHROMATIX_PTR] = NULL;
  }
  saved_events->module_events_valid[ISP_MODULE_EVENT_SET_CHROMATIX_PTR] = FALSE;

  if (saved_events->module_events[ISP_MODULE_EVENT_ISP_DISABLE_MODULE]) {
    isp_util_free_event(
      saved_events->module_events[ISP_MODULE_EVENT_ISP_DISABLE_MODULE]);
    saved_events->module_events[ISP_MODULE_EVENT_ISP_DISABLE_MODULE] = NULL;
  }
  saved_events->module_events_valid[ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
    FALSE;

  if (saved_events->module_events[ISP_MODULE_EVENT_SET_FLASH_MODE]) {
    isp_util_free_event(
      saved_events->module_events[ISP_MODULE_EVENT_SET_FLASH_MODE]);
    saved_events->module_events[ISP_MODULE_EVENT_SET_FLASH_MODE] = NULL;
  }
  saved_events->module_events_valid[ISP_MODULE_EVENT_SET_FLASH_MODE] = FALSE;

  if (saved_events->module_events[ISP_MODULE_EVENT_STATS_FOVC_MAGNIFICATION_FACTOR]) {
    isp_util_free_event(
      saved_events->module_events[ISP_MODULE_EVENT_STATS_FOVC_MAGNIFICATION_FACTOR]);
    saved_events->module_events[ISP_MODULE_EVENT_STATS_FOVC_MAGNIFICATION_FACTOR] = NULL;
  }
  saved_events->module_events_valid[ISP_MODULE_EVENT_STATS_FOVC_MAGNIFICATION_FACTOR] = FALSE;

  if(saved_events->set_params[ISP_SET_META_SCALER_CROP_REGION]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_META_SCALER_CROP_REGION]);
      saved_events->set_params[ISP_SET_META_SCALER_CROP_REGION] = NULL;
      saved_events->set_params_valid[ISP_SET_META_SCALER_CROP_REGION] = FALSE;
  }

  if(saved_events->set_params[ISP_SET_META_COLOR_CORRECT_GAINS]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_META_COLOR_CORRECT_GAINS]);
      saved_events->set_params[ISP_SET_META_COLOR_CORRECT_GAINS] = NULL;
      saved_events->set_params_valid[ISP_SET_META_COLOR_CORRECT_GAINS] = FALSE;
  }

  if(saved_events->set_params[ISP_SET_META_NOISE_REDUCTION_MODE]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_META_NOISE_REDUCTION_MODE]);
      saved_events->set_params[ISP_SET_META_NOISE_REDUCTION_MODE] = NULL;
      saved_events->set_params_valid[ISP_SET_META_NOISE_REDUCTION_MODE] = FALSE;
  }

  if(saved_events->set_params[ISP_SET_META_LENS_SHADING_MODE]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_META_LENS_SHADING_MODE]);
      saved_events->set_params[ISP_SET_META_LENS_SHADING_MODE] = NULL;
      saved_events->set_params_valid[ISP_SET_META_LENS_SHADING_MODE] = FALSE;
  }

  if(saved_events->set_params[ISP_SET_META_LENS_SHADING_MAP_MODE]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_META_LENS_SHADING_MAP_MODE]);
      saved_events->set_params[ISP_SET_META_LENS_SHADING_MAP_MODE] = NULL;
      saved_events->set_params_valid[ISP_SET_META_LENS_SHADING_MAP_MODE] = FALSE;
  }

  if(saved_events->set_params[ISP_SET_META_TONEMAP_MODE]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_META_TONEMAP_MODE]);
      saved_events->set_params[ISP_SET_META_TONEMAP_MODE] = NULL;
      saved_events->set_params_valid[ISP_SET_META_TONEMAP_MODE] = FALSE;
  }

  if(saved_events->set_params[ISP_SET_META_TONEMAP_CURVES]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_META_TONEMAP_CURVES]);
      saved_events->set_params[ISP_SET_META_TONEMAP_CURVES] = NULL;
      saved_events->set_params_valid[ISP_SET_META_TONEMAP_CURVES] = FALSE;
  }

  if(saved_events->set_params[ISP_SET_META_COLOR_CORRECT_MODE]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_META_COLOR_CORRECT_MODE]);
      saved_events->set_params[ISP_SET_META_COLOR_CORRECT_MODE] = NULL;
      saved_events->set_params_valid[ISP_SET_META_COLOR_CORRECT_MODE] = FALSE;
  }

  if(saved_events->set_params[ISP_SET_META_COLOR_CORRECT_TRANSFORM]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_META_COLOR_CORRECT_TRANSFORM]);
      saved_events->set_params[ISP_SET_META_COLOR_CORRECT_TRANSFORM] = NULL;
      saved_events->set_params_valid[ISP_SET_META_COLOR_CORRECT_TRANSFORM] = FALSE;
  }

  if(saved_events->set_params[ISP_SET_META_BLACK_LEVEL_LOCK]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_META_BLACK_LEVEL_LOCK]);
      saved_events->set_params[ISP_SET_META_BLACK_LEVEL_LOCK] = NULL;
      saved_events->set_params_valid[ISP_SET_META_BLACK_LEVEL_LOCK] = FALSE;
  }

  if(saved_events->set_params[ISP_SET_META_MODE]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_META_MODE]);
      saved_events->set_params[ISP_SET_META_MODE] = NULL;
      saved_events->set_params_valid[ISP_SET_META_MODE] = FALSE;
  }

  if(saved_events->set_params[ISP_SET_AEC_MODE]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_AEC_MODE]);
      saved_events->set_params[ISP_SET_AEC_MODE] = NULL;
      saved_events->set_params_valid[ISP_SET_AEC_MODE] = FALSE;
  }

  if(saved_events->set_params[ISP_SET_MANUAL_GAIN]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_MANUAL_GAIN]);
      saved_events->set_params[ISP_SET_MANUAL_GAIN] = NULL;
      saved_events->set_params_valid[ISP_SET_MANUAL_GAIN] = FALSE;
  }

  if(saved_events->set_params[ISP_SET_INTF_PARM_CAC]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_INTF_PARM_CAC]);
      saved_events->set_params[ISP_SET_INTF_PARM_CAC] = NULL;
      saved_events->set_params_valid[ISP_SET_INTF_PARM_CAC] = FALSE;
  }

  if(saved_events->set_params[ISP_SET_PARM_WHITE_BALANCE]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_PARM_WHITE_BALANCE]);
      saved_events->set_params[ISP_SET_PARM_WHITE_BALANCE] = NULL;
      saved_events->set_params_valid[ISP_SET_PARM_WHITE_BALANCE] = FALSE;
  }

  if(saved_events->set_params[ISP_SET_INTF_HOTPIXEL_MODE]){
      isp_util_free_event(saved_events->
        set_params[ISP_SET_INTF_HOTPIXEL_MODE]);
      saved_events->set_params[ISP_SET_INTF_HOTPIXEL_MODE] = NULL;
      saved_events->set_params_valid[ISP_SET_INTF_HOTPIXEL_MODE] = FALSE;
  }

  return TRUE;
}

/** isp_util_free_dmi_meta_tbl:
 *
 *  @module: isp module
 *  @session_param : isp session parameters
 *
 *  update hw parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_free_dmi_meta_tbl(mct_module_t *module,
  isp_session_param_t *session_param)
{
  boolean                      ret = TRUE;
  isp_hw_update_params_t      *hw_update_params = NULL;
  isp_parser_params_t         *parser_params = NULL;
  isp_hw_update_list_params_t *hw_update_list_params = NULL;
  uint32_t                     i = 0;
  uint32_t                     j = 0;

  if (!module || !session_param) {
    ISP_ERR("failed: module %p session_param %p", module, session_param);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;
  for (i = 0; i < session_param->num_isp; i++) {
    for (j = 0; j < ISP_METADUMP_MAX_NUM; j++) {
      hw_update_list_params =
        &hw_update_params->hw_update_list_params[session_param->hw_id[i]];
      hw_update_list_params->applied_dmi_tbl.
        dmi_info[j].read_length = 0;
      hw_update_list_params->dmi_tbl.
        dmi_info[j].read_length = 0;
    }
  }
  return ret;
}

/** isp_util_free_per_frame_meta:
 *
 *  @module: isp module
 *  @session_param : isp session parameters
 *
 *  update hw parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_free_per_frame_meta(mct_module_t *module,
  isp_session_param_t *session_param)
{
  boolean                      ret = TRUE;
  isp_hw_update_params_t      *hw_update_params = NULL;
  isp_parser_params_t         *parser_params = NULL;
  isp_hw_update_list_params_t *hw_update_list_params = NULL;
  uint32_t                     i = 0;
  uint32_t                     j = 0;

  if (!module || !session_param) {
    ISP_ERR("failed: module %p session_param %p", module, session_param);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;
  for (i = 0; i < session_param->num_isp; i++) {
    for (j = 0; j < ISP_METADUMP_MAX_NUM; j++) {
      hw_update_list_params =
        &hw_update_params->hw_update_list_params[session_param->hw_id[i]];
      hw_update_list_params->applied_frame_meta.bestshot_mode= 0;
      hw_update_list_params->frame_meta.bestshot_mode= 0;
    }
  }
  return ret;
}

/** isp_fast_aec_mode:
 *
 *@module: iface instance
 *@stream_id : stream info
 *@session_id : session info
 *@fast_aec : fast aec mode info
 *
 * Stores fast aec information in session params
 *
 *  Return: TRUE/FALSE
 **/
boolean isp_util_set_fast_aec_mode(mct_module_t *module, mct_port_t *port,
  mct_event_t *event, mct_fast_aec_mode_t *fast_aec)
{
  boolean              ret = TRUE;
  isp_t               *isp = NULL;
  isp_session_param_t *session_param = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }
  /* Forward event to pipelines */
  if (session_param->session_based_ide) {
    ret = isp_util_forward_event_to_session_based_stream_all_int_pipelines(
      session_param, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event_to_pipelines");
    }
  } else {
    ret = isp_util_forward_event_to_all_streams_all_internal_pipelines(
      session_param, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event_to_pipelines");
    }
  }

  session_param->fast_aec_mode = fast_aec->enable;
  ISP_HIGH("fast_aec %d", session_param->fast_aec_mode);
  /* Forward event to downstream */
  ret = isp_util_forward_event(port, event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_forward_event");
  }
  return ret;
}

/** isp_util_get_stream_param_from_type:
 *
 *  @module: mct module
 *  @stream_type: stream type
 *  @session_param: handle to store session param
 *  @stream_param: handle to store stream param
 *
 *  Retrieve session and stream param based on stream type
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_get_stream_param_from_type(mct_module_t *module,
  cam_stream_type_t stream_type, isp_session_param_t *session_param,
  isp_stream_param_t **stream_param)
{
  boolean     ret = TRUE;
  isp_t      *isp = NULL;
  mct_list_t *l_session_params = NULL;
  mct_list_t *l_stream_params = NULL;

  if (!module || !session_param || !stream_param) {
    ISP_ERR("failed: module %p session_param %p stream param %p", module,
      session_param, stream_param);
    return FALSE;
  }

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  l_stream_params = mct_list_find_custom(session_param->l_stream_params,
    &stream_type, isp_util_compare_stream_type);
  if (!l_stream_params) {
    ISP_ERR("failed: l_stream_params %p for stream type %d", l_stream_params,
      stream_type);
    return FALSE;
  }

  *stream_param = (isp_stream_param_t *)l_stream_params->data;
  if (!(*stream_param)) {
    ISP_ERR("failed: stream_param %p", *stream_param);
    return FALSE;
  }

  return ret;
}

/** isp_util_sort_streams
 *    @streams_desc: description of all streams sent by HAL
 *    @sorted_streams: output array of sorted indexes
 *    @sensor_dim: sensor output/ISP input
 *
 *  Sort all streams by size, except of RAW streams. It put all
 *  RAW streams at the end. This function does not sort array of
 *  stream.
 *
 * Return: none
 **/
static void isp_util_sort_streams(cam_stream_size_info_t *streams_desc,
  isp_util_stream_desc *sorted_streams, cam_dimension_t *sensor_dim)
{
  cam_dimension_t norm_stream_sizes[MAX_NUM_STREAMS];
  isp_util_stream_desc tmp;
  int32_t i, j;
  float sen_ar, curr_ar;

  /* copy to local array */
  for (i = 0; i < (int32_t)streams_desc->num_streams; i++) {
    sorted_streams[i].type = streams_desc->type[i];
    sorted_streams[i].dim = streams_desc->stream_sz_plus_margin[i];
    sorted_streams[i].orig_dim = streams_desc->stream_sizes[i];
    sorted_streams[i].pp_mask = streams_desc->postprocess_mask[i];
  }

  /* Put all raw streams at the end */
  for (i = 1; i < (int32_t)streams_desc->num_streams; i++) {
    for (j = i; j > 0; j--) {
      if (sorted_streams[j - 1].type == CAM_STREAM_TYPE_RAW ||
          sorted_streams[j - 1].type == CAM_STREAM_TYPE_OFFLINE_PROC) {
        tmp = sorted_streams[j];
        sorted_streams[j] = sorted_streams[j - 1];
        sorted_streams[j - 1] = tmp;
        break;
      }
    }
  }

  /* Calculate stream size before FOV module */
  sen_ar = (float)sensor_dim->width / sensor_dim->height;
  for (i = 0; i < (int32_t)streams_desc->num_streams; i++) {
    sorted_streams[i].norm_dim.width = sorted_streams[i].dim.width;
    sorted_streams[i].norm_dim.height = sorted_streams[i].dim.height;
    /* We do not need to recalculate dimensions of RAW streams. */
    if (sorted_streams[i].type == CAM_STREAM_TYPE_RAW ||
        sorted_streams[i].type == CAM_STREAM_TYPE_OFFLINE_PROC) {
      break;
    }


    if (sensor_dim->width * sorted_streams[i].dim.height !=
        sensor_dim->height * sorted_streams[i].dim.width) {
      curr_ar = (float)sorted_streams[i].dim.width /
          sorted_streams[i].dim.height;
      if (curr_ar > sen_ar) {
        sorted_streams[i].norm_dim.height =
          sorted_streams[i].norm_dim.width / sen_ar;
        sorted_streams[i].norm_dim.height =
          FLOOR2(sorted_streams[i].norm_dim.height);
      } else {
        sorted_streams[i].norm_dim.width =
          sorted_streams[i].norm_dim.height * sen_ar;
        sorted_streams[i].norm_dim.width =
          CEILING2(sorted_streams[i].norm_dim.width);
      }
    }
  }

  /* Sort streams by type and size. */
  for (i = 1; i < (int32_t)streams_desc->num_streams; i++) {
    for (j = i; j > 0; j--) {
      /* Put all raw streams to the end. */
      if (sorted_streams[j].type == CAM_STREAM_TYPE_RAW ||
          sorted_streams[j].type == CAM_STREAM_TYPE_OFFLINE_PROC) {
        break;
      }

      if ((sorted_streams[j].norm_dim.width *
           sorted_streams[j].norm_dim.height) >
          (sorted_streams[j - 1].norm_dim.width *
           sorted_streams[j - 1].norm_dim.height)) {
        /* Sort by size. The biggest first */
        tmp = sorted_streams[j];
        sorted_streams[j] = sorted_streams[j - 1];
        sorted_streams[j - 1] = tmp;
      } else if ((sorted_streams[j].norm_dim.width *
                  sorted_streams[j].norm_dim.height) ==
                 (sorted_streams[j - 1].norm_dim.width *
                  sorted_streams[j - 1].norm_dim.height) &&
                 (sorted_streams[j].pp_mask >
                  sorted_streams[j - 1].pp_mask)) {
        /* If size is same sort by pp_mask */
        tmp = sorted_streams[j];
        sorted_streams[j] = sorted_streams[j - 1];
        sorted_streams[j - 1] = tmp;
      } else {
        break;
      }
    }
  }

#if PRINT_STREAM_MAPPING
  ISP_ERR("=============== sorted streams ====================");
  for (i = 0; i < (int32_t)streams_desc->num_streams; i++) {
    ISP_ERR("Stream size: %dx%d Normalize: %dx%d type: %d pp_mask: 0x%x",
      sorted_streams[i].dim.height, sorted_streams[i].dim.width,
      sorted_streams[i].norm_dim.height, sorted_streams[i].norm_dim.width,
      sorted_streams[i].type, sorted_streams[i].pp_mask);
  }
  ISP_ERR("===================================================");
#endif
}

/** isp_util_sort_isp_outputs
 *    @isp_hw_limit: description of HW limitations
 *
 *  Sort HW limit array by size
 *
 * Return: none
 **/
static void isp_util_sort_isp_outputs(isp_submod_hw_limit_t *isp_hw_limit)
{
  int32_t i, j, tmp;
  isp_submod_hw_limit_desc_t *hw_limits;
  isp_submod_hw_limit_desc_t hw_limits_tmp;

  hw_limits = isp_hw_limit->hw_limits;

  for (i = 1; i < (int32_t)isp_hw_limit->num_hw_streams; i++) {
    for (j = i; j > 0; j--) {

      /* Sort by size. The biggest first */
      if ((hw_limits[j].max_width * hw_limits[j].max_height) >
          (hw_limits[j - 1].max_width * hw_limits[j - 1].max_height)) {
        hw_limits_tmp = hw_limits[j];
        hw_limits[j] = hw_limits[j - 1];
        hw_limits[j - 1] = hw_limits_tmp;
      }
    }
  }

#if PRINT_STREAM_MAPPING
  ISP_ERR("============ sorted ISP output ====================");
  ISP_ERR("num_hw_streams %d", isp_hw_limit->num_hw_streams);
  for (i = 0; i < (int32_t)isp_hw_limit->num_hw_streams; i++) {
    ISP_ERR("i %d hw stream id %d max w %d h %d scale ratio %d", i,
      isp_hw_limit->hw_limits[i].hw_stream_id,
      isp_hw_limit->hw_limits[i].max_width,
      isp_hw_limit->hw_limits[i].max_height,
      isp_hw_limit->hw_limits[i].max_scale_ratio);
  }
  ISP_ERR("==================================================");
#endif
}

/** isp_util_check_hw_limitation
 *    @isp_hw_limit: ISP HW limitations
 *    @sorted_streams: array of sorted streams
 *    @num_pix_streams: number of stream that need to be sorted
 *    @num_isp: number of ISPs
 *    @threshold: store witch is the biggest stream per HW output
 *
 *  Check which streams fit in HW outputs
 *
 * Return: none
 **/
static void isp_util_check_hw_limitation(isp_submod_hw_limit_t *isp_hw_limit,
  isp_util_stream_desc *sorted_streams, int32_t num_pix_streams,
  uint32_t num_isp, int32_t *threshold)
{
  int32_t strm, out, i = 0;
  int32_t stream_out_w, stream_out_h, hw_max_w, hw_max_h;

  for (out = 0; out < (int32_t)isp_hw_limit->num_hw_streams; out++) {
    hw_max_w = isp_hw_limit->hw_limits[out].max_width;
    hw_max_h = isp_hw_limit->hw_limits[out].max_height;
    for (strm = num_pix_streams - 1; strm >= 0; strm--) {
      stream_out_w = sorted_streams[strm].norm_dim.width;
      if (num_isp > 1) {
        /* width / num_isp + 20% */
        stream_out_w = ((stream_out_w / num_isp) * 6) / 5;
      }
      stream_out_h = sorted_streams[strm].norm_dim.height;

      if (hw_max_w < stream_out_w || hw_max_h < stream_out_h) {
        threshold[out] = strm + 1;
        break;
      }
    }
  }

#if PRINT_STREAM_MAPPING
  ISP_ERR("==================== threshold =====================");
  for (i = 0; i < (int32_t)isp_hw_limit->num_hw_streams; i++) {
    ISP_ERR("threshold[%d] %d", i, threshold[i]);
  }
  ISP_ERR("====================================================");

#endif
}

/** isp_util_check_fit_in_isp_ratio
 *    @camif_dim: isp in dimension
 *    @out_dim: isp out dimension
 *    @changed_dim: isp changed out dimension
 *    @isp_ratio: max ISP downscale ratio
 * ... @pp_mask: pp_mask
 *  Check if ISP could provide output dimension
 *
 * Return: TRUE on fit and FALSE if does not fit in ratio
 **/
static boolean isp_util_check_fit_in_isp_ratio(cam_dimension_t *camif_dim,
  cam_dimension_t *out_dim, cam_dimension_t *changed_dim,
  int isp_ratio, boolean *changed, uint32_t pp_mask)
{
  int32_t tmp_out_w, tmp_out_h, tmp_in_w, tmp_in_h;
  float camif_aspect_ratio = 0;
  float out_aspect_ratio = 0;
  int32_t fit = 0;

  tmp_out_w = changed_dim->width = out_dim->width;
  tmp_out_h = changed_dim->height = out_dim->height;
  *changed = FALSE;
  if (camif_dim->width * out_dim->height !=
    out_dim->width * camif_dim->height) {
    camif_aspect_ratio = (float)camif_dim->width / camif_dim->height;
    out_aspect_ratio = (float)out_dim->width / out_dim->height;
    if (camif_aspect_ratio < out_aspect_ratio) {
      tmp_out_h = tmp_out_w / camif_aspect_ratio;
      tmp_out_h = FLOOR16(tmp_out_h);
    } else {
      tmp_out_w = tmp_out_h * camif_aspect_ratio;
      tmp_out_w = CEILING16(tmp_out_w);
    }
  }


  tmp_in_w = CEILING16(camif_dim->width / isp_ratio);
  tmp_in_h = CEILING16(camif_dim->height / isp_ratio);
  if (((tmp_out_w != changed_dim->width) ||
    tmp_out_h != changed_dim->height) &&
    (out_dim->width < tmp_in_w || out_dim->height < tmp_in_h) &&
    pp_mask) {
    changed_dim->width = tmp_out_w;
    changed_dim->height = tmp_out_h;
    *changed = TRUE;
  }
  if (tmp_out_w < tmp_in_w || tmp_out_h < tmp_in_h){
    if (pp_mask) {
      changed_dim->width = tmp_in_w;
      changed_dim->height = tmp_in_h;
      *changed = TRUE;
    }
    if (out_dim->width == QCIF_WIDTH && out_dim->height == QCIF_HEIGHT)
      fit = TRUE;
    else
      fit = FALSE;
  }
  else
    fit = TRUE;
  return fit;
}

/** check_dim_fit_in_ratio
 *    @dim1: dimension 1
 *    @dim2: dimension 2
 *    @ratio: max expected difference
 *
 *  Check if two dimensions fit in ratio.
 *
 * Return: TRUE on fit and FALSE if does not fit in ratio
 **/
static boolean check_dim_fit_in_ratio(cam_dimension_t *dim1,
  cam_dimension_t *dim2, int ratio)
{
  int32_t tmp1_w, tmp1_h, tmp2_w, tmp2_h;
  int32_t fit = 0;

  tmp1_w = CEILING16(dim1->width / ratio);
  tmp1_h = CEILING16(dim1->height / ratio);
  tmp2_w = CEILING16(dim2->width / ratio);
  tmp2_h = CEILING16(dim2->height / ratio);

  if (dim1->width  < tmp2_w ||
      dim1->height < tmp2_h ||
      dim2->width  < tmp1_w ||
      dim2->height < tmp1_h )
    fit = FALSE;
  else
    fit = TRUE;

  return fit;
}

/** isp_util_compare_dim
 *    @dim1: dimension 1
 *    @dim2: dimension 2
 *
 *  Check if two dimensions are equal.
 *
 * Return: TRUE on same dimension and FALSE if different dimension
 **/
static inline boolean isp_util_compare_dim(cam_dimension_t *dim1,
  cam_dimension_t *dim2)
{
  return ((dim1->width == dim2->width) && (dim1->height == dim2->height));
}

/** isp_util_calc_max_stream_num_per_hw_output
 *    @sorted_streams: description of streams sent by HAL sorted by size
 *    @mapped_streams: container of stream mapping
 *    @num_hw_outputs: number of ISP HW streams
 *    @threshold: point witch streams fit in each output
 *    @num_pix_streams: number of pix streams
 *    @num_prior_streams: number of priority streams
 *    @prior_hw_stream_mask: mask of HW output for priority streams
 *
 *  Calculate what is maximum number of user streams which may be linked to
 *  one ISP HW stream. This value apply only for non priority streams.
 *
 * Return number of user stream which may be link to one HW stream
 **/
inline static int32_t isp_util_calc_max_stream_num_per_hw_output(
  isp_util_stream_desc *sorted_streams,
  isp_stream_port_map_info_t *mapped_streams, uint32_t num_hw_outputs,
  int32_t *thresholds, uint32_t num_pix_streams, uint32_t num_prior_streams,
  uint32_t prior_hw_stream_mask, uint32_t prior_user_stream_mask,
  uint32_t prior_hw_stream_cnt)
{
  int32_t num_mapped_streams;
  int32_t  i,j, max_streams_per_out, curr_abs_max, max, out;
  int32_t  max_num_per_out[ISP_HW_STREAM_MAX];

  int32_t normal_stream      = 0;
  int32_t pp_stream          = 0;
  int32_t pending_hw_outputs = 0;
  int32_t count              = 0;
  uint32_t pp_mask_count     = 0;
  isp_util_stream_desc local_table[ISP_MAX_STREAMS];

  /* Calculate how many non-priority streams fit in each HW output */
  memset(max_num_per_out, 0, sizeof(max_num_per_out));
  for (out = num_hw_outputs - 1; out >= 0; out--) {
    /* Skip if output is use for priority stream */
    if ((1 << out) & prior_hw_stream_mask) {
      continue;
    }
    for (i = num_pix_streams - 1; i >= 0; i--) {
      if (mapped_streams[i].hw_stream != ISP_HW_STREAM_MAX) {
        /* if stream is mapped, don't add it */
        continue;
      }
      if (thresholds[out] <= i) {
        max_num_per_out[out]++;
      } else {
        break;
      }
    }
  }
  /* Filter all the stream with pp_mask 0 and store in local table */
  for (i = 0; i < (int32_t)num_pix_streams; i++) {
    if (sorted_streams[i].pp_mask == CAM_QCOM_FEATURE_NONE) {
      local_table[count].dim.width = sorted_streams[i].dim.width;
      local_table[count].dim.height = sorted_streams[i].dim.height;
      count++;
    } else
      continue;
  }
  /* if only one stream has 0 pp_mask the reduce num_hw_outputs
   * to calculate proper max_streams_per_out */
  if (count == 1) {
    pp_mask_count++;
  }
  /* check any duplicate stream with pp_mask 0. in that case
   * both stream goes to same port */
  for (j = 1; j < count; j++) {
    if (local_table[j].dim.width == local_table[j-1].dim.width &&
       local_table[j].dim.height == local_table[j-1].dim.height) {
      continue;
    } else {
       pp_mask_count++;
    }
  }
  /* Find maximum number of user streams per HW output. We pick up the smallest
   * maximum of user stream per output. We count how many streams could be map
   * with this maximum. We need to count it because it is not linear function of
   * max. This is not linear function because of HW limitations. Once we find
   * maximum number of streams we check if this number is enough to map all
   * streams. */
  max_streams_per_out = 0;
  for (max = 1; max < (int32_t)(MAX_STREAMS_PER_PORT + 1); max++) {
    num_mapped_streams = 0;
    for (out = num_hw_outputs - 1; out >= 0; out--) {
      /* Skip if output is use for priority stream */
      if ((1 << out) & prior_hw_stream_mask) {
        continue;
      }
      /* calc absolute maximum for this output */
      curr_abs_max = max_num_per_out[out] - num_mapped_streams;
      if (curr_abs_max < 0) {
        curr_abs_max = 0;
      }
      /* Clip to chosen maximum and add to number of mapped streams */
      num_mapped_streams += curr_abs_max > max ? max : curr_abs_max;
    }

    /* Check if current max is enough to fit all streams */
    if (num_mapped_streams >= (int32_t)(num_pix_streams - num_prior_streams)) {
      max_streams_per_out = max;
      break;
    }
  }

  /* If result is 1 but we have two streams with same dimensions and PP mask set
   * maximum number of streams per output to 2. This is kind of work around but
   * we still don not have a better solution how calculate it more generic */
  if (max_streams_per_out == 1) {
    for (i = 1; i < (int32_t)(num_pix_streams - num_prior_streams); i++) {
      if ((sorted_streams[i].dim.width  == sorted_streams[i - 1].dim.width) &&
          (sorted_streams[i].dim.height == sorted_streams[i - 1].dim.height) &&
          (sorted_streams[i].pp_mask    == sorted_streams[i - 1].pp_mask) &&
          (mapped_streams[i].hw_stream == ISP_HW_STREAM_MAX) &&
          (mapped_streams[i - 1].hw_stream == ISP_HW_STREAM_MAX)) {
        max_streams_per_out = 2;
        break;
      }
    }
  }
  if (!num_prior_streams)
    prior_hw_stream_cnt = pp_mask_count;

  pending_hw_outputs = num_hw_outputs - prior_hw_stream_cnt;

  if (((int32_t)(num_pix_streams - num_prior_streams) >=
      max_streams_per_out * pending_hw_outputs)) {
    for (i = 0; i < (int32_t)(num_pix_streams); i++) {
      if (prior_user_stream_mask & 1 << sorted_streams[i].type)
        continue;
      if (sorted_streams[i].pp_mask == CAM_QCOM_FEATURE_NONE){
        normal_stream++;
      } else {
        pp_stream++;
      }
    }
    max_streams_per_out = MAX(normal_stream, pp_stream);
    if (max_streams_per_out > MAX_STREAMS_PER_PORT)
      max_streams_per_out = MAX_STREAMS_PER_PORT;
  }
  return max_streams_per_out;
}

/** isp_util_map_streams
 *    @session_param: handle to store session parameters
 *    @streams_desc: description of all streams sent by HAL
 *    @isp_hw_limit: ISP HW limitations
 *    @priority_stream: priority streams
 *    @controllable_out_needed: controllable output needed
 *
 *  Decide the mapping of the streams to the HW streams. If priority stream
 *  type is present, we map streams with this type on separate HW stream.
 *
 * Return TRUE on success and FALSE if streams could not be mapped
 **/
static boolean isp_util_map_streams(isp_session_param_t *session_param,
  cam_stream_size_info_t *streams_desc, isp_submod_hw_limit_t *isp_hw_limit,
  isp_preferred_streams *priority_streams, boolean controllable_out_needed)
{
  isp_stream_port_map_info_t *stream_map;
  cam_dimension_t sensor_dim, *dim1, *dim2;
  isp_hw_streamid_t hw_stream_id;
  boolean fit_in_port, fit_in_isp_ratio[ISP_HW_STREAM_MAX];
  boolean changed[ISP_MAX_STREAMS];
  boolean controllable_output[ISP_HW_STREAM_MAX];
  boolean stream_mapped;
  isp_util_stream_desc sorted_streams[ISP_MAX_STREAMS];
  int32_t num_pix_streams, num_prior_streams;
  int32_t smaller_stream[ISP_HW_STREAM_MAX];
  int32_t threshold[ISP_HW_STREAM_MAX], stream_num_per_out[ISP_HW_STREAM_MAX];
  int32_t strm, out, i, j, mapped_stream_cnt;
  uint32_t pp_mask1, pp_mask2;
  uint32_t priority_user_streams_mask, priority_hw_streams_mask;
  int32_t max_streams_per_out;
  int32_t additional_hw_stream_required;
  int32_t num_free_pix_streams = 0, num_free_hw_stream = 0;
  boolean same_dim = FALSE;

  stream_map = session_param->stream_port_map.streams;
  sensor_dim.width = session_param->sensor_output_info.dim_output.width;
  sensor_dim.height = session_param->sensor_output_info.dim_output.height;

  /* Validate input */
  if (!sensor_dim.width || !sensor_dim.height) {
    ISP_ERR("failed: sensor resolution: %dx%d", sensor_dim.width,
      sensor_dim.height);
    return FALSE;
  }
  for (i = 0; i < (int32_t)streams_desc->num_streams; i++) {
    if (streams_desc->stream_sizes[i].width <= 0 ||
        streams_desc->stream_sizes[i].height <= 0) {
      ISP_ERR("failed: stream type: %d dim: %dx%d sensor dim: %dx%d",
        streams_desc->type[i], streams_desc->stream_sizes[i].width,
        streams_desc->stream_sizes[i].height,
        sensor_dim.width, sensor_dim.height);
      return FALSE;
    }
  }

  /* Initialize local variables */
  memset(fit_in_isp_ratio, 0, sizeof(fit_in_isp_ratio));
  memset(controllable_output, 0, sizeof(controllable_output));
  memset(stream_num_per_out, 0, sizeof(*stream_num_per_out) *
    ISP_HW_STREAM_MAX);
  memset(threshold, 0, sizeof(*threshold) * ISP_HW_STREAM_MAX);
  memset(smaller_stream, 0, sizeof(*smaller_stream) * ISP_HW_STREAM_MAX);
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    stream_map[i].hw_stream = ISP_HW_STREAM_MAX;
  }

  mapped_stream_cnt = 0;
  num_prior_streams = 0;
  num_pix_streams = 0;
  priority_user_streams_mask = 0;
  priority_hw_streams_mask = 0;
  for (i = 0; i < (int32_t)streams_desc->num_streams; i++) {
    if (streams_desc->type[i] != CAM_STREAM_TYPE_RAW &&
        streams_desc->type[i] != CAM_STREAM_TYPE_OFFLINE_PROC) {
      num_pix_streams++;
    }
    for (j = 0; j < (int32_t)priority_streams->stream_num; j++) {
      if ((1 << streams_desc->type[i]) &
          priority_streams->streams[j].stream_mask) {
        num_prior_streams++;
        priority_user_streams_mask |= priority_streams->streams[j].stream_mask;
      }
    }
  }
  num_free_hw_stream   = isp_hw_limit->num_hw_streams;
  /* Check if we have enought outputs */
  if ((priority_streams->stream_num > isp_hw_limit->num_hw_streams) ||
      ((priority_streams->stream_num == isp_hw_limit->num_hw_streams) &&
       (num_pix_streams - num_prior_streams))) {
    ISP_ERR("ISP cann't support %d priority outputs.",
      priority_streams->stream_num);
    ISP_ERR("ISP outputs: %d Priority outputs: %d non-priority streams: %d",
      isp_hw_limit->num_hw_streams, priority_streams->stream_num,
      num_pix_streams - num_prior_streams);
    return FALSE;
  }

  /* Sort ISP output by size */
  isp_util_sort_isp_outputs(isp_hw_limit);

  /* Sort streams for non snapshot case and raw streams mapping */
  isp_util_sort_streams(streams_desc, sorted_streams, &sensor_dim);

  /* Find witch streams fit in output limitations per output */
  isp_util_check_hw_limitation(isp_hw_limit, sorted_streams, num_pix_streams,
    session_param->num_isp, threshold);

  /* Map first priority stream. */
  for (j = 0; j < (int32_t)priority_streams->stream_num; j++) {
    stream_mapped = FALSE;
    memset(smaller_stream, 0, sizeof(*smaller_stream) * ISP_HW_STREAM_MAX);
    for (out = isp_hw_limit->num_hw_streams - 1; out >= 0; out--) {
      if (priority_hw_streams_mask & (1 << out)) {
        // do not map any other streams to prior HW stream
        continue;
      }
      hw_stream_id = isp_hw_limit->hw_limits[out].hw_stream_id;
      for (i = 0; i < num_pix_streams; i++) {
        /* Check if user stream type is same as priority stream type */
        if ((1 << sorted_streams[i].type) &
            (~priority_streams->streams[j].stream_mask)) {
          continue;
        }

        /* Check if current user stream is mapped. */
        if (stream_map[i].hw_stream != ISP_HW_STREAM_MAX) {
          continue; // stream is already mapped
        }

        /* Check if user stream fit in HW output limitations */
        if (threshold[out] > i) {
          break; // does not fit in port limitation, go to next port
        }

        /* Check if all streams need or do not need post process */
        if (stream_num_per_out[hw_stream_id] > 0) {
          pp_mask1 = sorted_streams[i].pp_mask;
          pp_mask2 = sorted_streams[smaller_stream[hw_stream_id]].pp_mask;
          if (!IS_PP_MASK_FIT(pp_mask1, pp_mask2)) {
            break; // can not fit which need pp and stream that do not need pp
          }
        }

        /* Check if ISP can provide required resolution */
        fit_in_isp_ratio[hw_stream_id] = isp_util_check_fit_in_isp_ratio(
                  &sensor_dim, &sorted_streams[i].dim,
                  &sorted_streams[i].changed_dim,
                  isp_hw_limit->hw_limits[out].max_scale_ratio, &changed[i],
                  sorted_streams[i].pp_mask);

        stream_mapped = TRUE;
        priority_hw_streams_mask |= (1 << out);

        stream_map[i].hw_stream = hw_stream_id;
        stream_map[i].stream_type = sorted_streams[i].type;
        stream_map[i].stream_sizes = sorted_streams[i].dim;
        stream_map[i].orig_stream_sizes = sorted_streams[i].orig_dim;
        stream_map[i].changed_stream_sizes =
          sorted_streams[i].changed_dim;
        stream_map[i].shared_output = FALSE;
        stream_map[i].controlable_output = FALSE;
        stream_map[i].is_changed = changed[i];
        if (sorted_streams[i].pp_mask == CAM_QCOM_FEATURE_NONE &&
          controllable_out_needed) {
          stream_map[i].controlable_output = TRUE;
          controllable_output[out] = TRUE;
        }

        session_param->hw_stream_linked[hw_stream_id] = FALSE;
        if (sorted_streams[i].pp_mask != CAM_QCOM_FEATURE_NONE &&
          sorted_streams[i].pp_mask != CAM_QCOM_FEATURE_EZTUNE) {
          session_param->hw_stream_linked[hw_stream_id] = TRUE;
          stream_map[i].native_buffer = TRUE;
        }
        else
          stream_map[i].native_buffer = FALSE;

        if (stream_num_per_out[hw_stream_id] == 0) {
          smaller_stream[hw_stream_id] = i;
        }

        mapped_stream_cnt++;
        stream_num_per_out[hw_stream_id]++;
      }

      /* All priority streams which belong to one group should be mapped on
       * same HW stream. */
      if (stream_mapped) {
        /* If there are more than one controllable stream set all user streams
         * mapped to this hw stream as shared */
        if (stream_num_per_out[hw_stream_id] > 1 && controllable_output[out]) {
          for (i = 0; i < num_pix_streams; i++) {
            if (stream_map[i].hw_stream == hw_stream_id) {
              stream_map[i].shared_output = TRUE;
            }
          }
        }
        /* One group of priority streams are mapped successfully */
        break;
      }
    }
  }

  /* Check if all priority streams are mapped */
  if (mapped_stream_cnt != num_prior_streams) {
    ISP_ERR("failed: Cannot map all priority streams");
    return FALSE;
  }

  num_free_pix_streams = num_pix_streams - mapped_stream_cnt;
  for (out = isp_hw_limit->num_hw_streams - 1; out >= 0; out--) {
    if (priority_hw_streams_mask & (1 << out)) {
      num_free_hw_stream--;
    }
  }

  ISP_DBG("num_pix_streams %d mapped_stream_cnt %d"
          "free_pix_stream %d free_hw_stream %d"
          "priority_hw_streams_mask %x \n", num_pix_streams, mapped_stream_cnt,
          num_free_pix_streams, num_free_hw_stream, priority_hw_streams_mask);

  max_streams_per_out = isp_util_calc_max_stream_num_per_hw_output(
      sorted_streams, stream_map, isp_hw_limit->num_hw_streams, threshold,
      num_pix_streams, num_prior_streams, priority_hw_streams_mask,
      priority_user_streams_mask, priority_streams->stream_num);
  if (!max_streams_per_out || max_streams_per_out > MAX_STREAMS_PER_PORT) {
    ISP_ERR("failed: max_streams_per_out is: %d", max_streams_per_out);
    return FALSE;
  }

  /* Map PIX streams */
  for (out = isp_hw_limit->num_hw_streams - 1; out >= 0; out--) {
    hw_stream_id = isp_hw_limit->hw_limits[out].hw_stream_id;
    if (priority_hw_streams_mask & (1 << out)) {
      // do not map any other user streams to prior HW stream
      continue;
    }

    additional_hw_stream_required = 0;
    for (i = num_pix_streams - 1; i >= 0; i--) {
      /* Check if current user stream is mapped */
      if (stream_map[i].hw_stream != ISP_HW_STREAM_MAX) {
        continue; // stream is already mapped
      }

      /* Check if current is HW stream fully occupied*/
      if (stream_num_per_out[hw_stream_id] >= max_streams_per_out) {
        break; // port is full, go to next port
      }

      /* Check if user stream fit in HW output limitations */
      if (threshold[out] > i) {
        break; // does not fit in port limitation, go to next port
      }


      /* Check if num_free_pix_stream(streams that is yet to map)
         is less than free_hw_stream, and if we try to map Second stream
         to same port.
      */
      if ((num_free_pix_streams <= num_free_hw_stream) &&
         (stream_num_per_out[hw_stream_id] > 0) && out > 0 ) {

        same_dim = isp_util_compare_dim(&sorted_streams[i].dim,
                     &sorted_streams[smaller_stream[hw_stream_id]].dim);

       /* If current stream is not same as next one
          and if current strem is same and pp mask is not fit
          goto next port */
        if ((!same_dim || (same_dim &&
           !IS_PP_MASK_FIT(sorted_streams[i].pp_mask,
                           sorted_streams[smaller_stream[hw_stream_id]].pp_mask)))) {
           break; //goto next port
        }
     }

      /* Check if all streams need or do not need post process */
      if (stream_num_per_out[hw_stream_id] > 0) {
        pp_mask1 = sorted_streams[i].pp_mask;
        pp_mask2 = sorted_streams[smaller_stream[hw_stream_id]].pp_mask;
        if (!IS_PP_MASK_FIT(pp_mask1, pp_mask2)) {
          additional_hw_stream_required++;
          /* Can not map pp_mask streams with non_pp streams
             Continue mapping next user stream to same HW output */
          continue;
        }
      }

      /* Check if current user stream could fit with existing */
      if (stream_num_per_out[hw_stream_id] > 0) {
        fit_in_port = check_dim_fit_in_ratio(&sorted_streams[i].dim,
                  &sorted_streams[smaller_stream[hw_stream_id]].dim,
                  PPROC_MAX_SCALERS_RATIO);
        if (fit_in_port == FALSE) {
          break; // does not fit with previous stream, try with next output
        }
      }

      /* Map user streams with same size to same HW stream.
       * Check if next stream is not already mapped (priority stream case) */
      if (stream_num_per_out[hw_stream_id] > 0 && out > 0 && i > 0 &&
          stream_map[i - 1].hw_stream == ISP_HW_STREAM_MAX) {
        dim1 = &sorted_streams[i].dim;
        dim2 = &sorted_streams[smaller_stream[hw_stream_id]].dim;
        /* Try mapping same stream dimensions together,
           - Check if current and previous(already mapped stream) are of same
             dimension, if so go ahead and map on same HW output.
           - If not, check current and next stream are of same dimension,
             if so and also if we have sufficent HW outputs, map the current
             stream to next HW output.
           - If not map current user stream to current HW output */
        if (!isp_util_compare_dim(dim1, dim2)) {
          dim1 = &sorted_streams[i].orig_dim;
          dim2 = &sorted_streams[i - 1].orig_dim;
          pp_mask1 = sorted_streams[i].pp_mask;
          pp_mask2 = sorted_streams[i - 1].pp_mask;
          /* Check if current stream is same as next one and pp mask fit */
          if (isp_util_compare_dim(dim1, dim2) &&
              IS_PP_MASK_FIT(pp_mask1, pp_mask2)) {
            /* Check for additional Streams which needs HW outputs */
            for (j = (i -1); j > 0; j--) {
                fit_in_port = check_dim_fit_in_ratio(&sorted_streams[j].dim,
                  &sorted_streams[j -1].dim,
                  PPROC_MAX_SCALERS_RATIO);
                if(!fit_in_port || !IS_PP_MASK_FIT(sorted_streams[j].pp_mask,
                   sorted_streams[j -1].pp_mask)) {
                  additional_hw_stream_required++;
                }
            }
            if (out > additional_hw_stream_required) {
              break; // go to next port
            }
          }
        }
      }

      /* Take decision based on the biggest stream per HW output */
      fit_in_isp_ratio[hw_stream_id] = isp_util_check_fit_in_isp_ratio(
          &sensor_dim, &sorted_streams[i].dim,
          &sorted_streams[i].changed_dim,
          isp_hw_limit->hw_limits[out].max_scale_ratio, &changed[i],
          sorted_streams[i].pp_mask);

      stream_map[i].hw_stream = hw_stream_id;
      stream_map[i].stream_type = sorted_streams[i].type;
      stream_map[i].stream_sizes = sorted_streams[i].dim;
      stream_map[i].orig_stream_sizes = sorted_streams[i].orig_dim;
      stream_map[i].changed_stream_sizes =
        sorted_streams[i].changed_dim;
      stream_map[i].shared_output = FALSE;
      stream_map[i].controlable_output = FALSE;
      stream_map[i].is_changed = changed[i];
      if (sorted_streams[i].pp_mask == CAM_QCOM_FEATURE_NONE &&
        controllable_out_needed) {
        stream_map[i].controlable_output = TRUE;
        controllable_output[out] = TRUE;
      }

      session_param->hw_stream_linked[hw_stream_id] = FALSE;
      if (sorted_streams[i].pp_mask != CAM_QCOM_FEATURE_NONE) {
        session_param->hw_stream_linked[hw_stream_id] = TRUE;
        stream_map[i].native_buffer = TRUE;
      }
      else
        stream_map[i].native_buffer = FALSE;

      /* Store smaller stream for feature checks */
      if (stream_num_per_out[hw_stream_id] == 0) {
        smaller_stream[hw_stream_id] = i;
      }
      mapped_stream_cnt++;
      stream_num_per_out[hw_stream_id]++;
    }

    /* If there are more than one controllable stream set all user streams
     * mapped to this hw stream as shared */
    if (stream_num_per_out[hw_stream_id] > 1 && controllable_output[out]) {
      for (i = 0; i < num_pix_streams; i++) {
        if (stream_map[i].hw_stream == hw_stream_id) {
          stream_map[i].shared_output = TRUE;
        }
      }
    }
    for (i = num_pix_streams - 1; i >= 0; i--) {
      if (stream_map[i].stream_type == CAM_STREAM_TYPE_SNAPSHOT) {
        session_param->snapshot_path = stream_map[i].hw_stream;
        break;
      }
    }
  }
  if (mapped_stream_cnt != num_pix_streams) {
    ISP_ERR("failed: not able to map all streams!");
    return FALSE;
  }

  /* Check if native buffer is needed */
  for (i = num_pix_streams - 1; i >= 0; i--) {
    if (stream_map[i].controlable_output)
      continue;

    if (priority_user_streams_mask & (1 << stream_map[i].stream_type))
      continue;

    if ((fit_in_isp_ratio[stream_map[i].hw_stream] == FALSE &&
        sorted_streams[i].pp_mask != 0)||
        stream_num_per_out[stream_map[i].hw_stream] > 1) {
      stream_map[i].native_buffer = TRUE;
    }
  }

  /* RAW streams don't need port */
  for (i = num_pix_streams; i < (int32_t)streams_desc->num_streams; i++) {
    stream_map[i].hw_stream = ISP_HW_STREAM_IDEAL_RAW;
    stream_map[i].stream_type = sorted_streams[i].type;
    stream_map[i].stream_sizes = sorted_streams[i].dim;
    stream_map[i].native_buffer = FALSE;
  }

  return TRUE;
}

/** isp_util_bundle_stream_preferrences
 *    @priority_streams:
 *    @preferred_streams:
 *
 *  Merge ISP priority streams with downstream preference streams
 *
 * Return TRUE on success and FALSE if fails
 **/
static boolean isp_util_merge_stream_preferrences(
  isp_preferred_streams *priority_streams,
  isp_preferred_streams *preferred_streams)
{
  uint32_t i, j;

  if (!preferred_streams->stream_num) {
    return TRUE;
  } else if (!priority_streams->stream_num) {
    *priority_streams = *preferred_streams;
    return TRUE;
  }

  /* if priority mask is same as preferred mask we remove preferred */
  for (i = 0; i < priority_streams->stream_num; i++) {
    for (j = 0; j < preferred_streams->stream_num; j++) {
      if ((priority_streams->streams[i].stream_mask ==
           preferred_streams->streams[j].stream_mask) &&
          (priority_streams->streams[i].max_streams_num <=
           preferred_streams->streams[j].max_streams_num)) {
        preferred_streams->streams[j].stream_mask = 0;
        preferred_streams->streams[j].max_streams_num = 0;
      }
    }
  }

  /* Amend preferences which are not merged */
  for (i = 0; i < preferred_streams->stream_num; i++) {
    /* check if already merged */
    if (!preferred_streams->streams[i].stream_mask) {
      continue;
    }

    priority_streams->streams[priority_streams->stream_num].max_streams_num =
      preferred_streams->streams[i].max_streams_num;
    priority_streams->streams[priority_streams->stream_num].stream_mask =
      preferred_streams->streams[i].stream_mask;
    priority_streams->stream_num++;
  }

  /* Check if we have one stream set to two HW outputs */
  for (i = 0; i < priority_streams->stream_num - 1; i++) {
    for (j = i + 1; j < priority_streams->stream_num; j++) {
      if (priority_streams->streams[i].stream_mask &
          priority_streams->streams[j].stream_mask) {
        ISP_ERR("fail to merge stream preferences");
        return FALSE;
      }
    }
  }

  return TRUE;
}

/** isp_util_merge_non_prioirty_streams
 *    @session_param:
 *    @streams_desc:
 *    @isp_hw_limit:
 *    @priority_streams:
 *
 *  Merge ISP non prioity streams with preferred streams
 *  based on pp mask, if preferred streams exists
 *
 * Return TRUE on success and FALSE if fails
 **/
static void isp_util_merge_non_prioirty_streams(
  isp_session_param_t *session_param, cam_stream_size_info_t *streams_desc,
  isp_submod_hw_limit_t *isp_hw_limit, isp_preferred_streams *priority_streams)
{
  uint32_t combined_stream_mask = 0;
  uint32_t i,j;

  /* Generate complete prioirity streams mask */
  for (i = 0; i < priority_streams->stream_num; i++) {
      combined_stream_mask |= priority_streams->streams[i].stream_mask;
  }
  /* Combine Priority streams mask with Preferred streams mask */
  for (i = 0; i < session_param->preferred_mapping.stream_num; i++) {
      combined_stream_mask |= session_param->preferred_mapping.streams[i].stream_mask;
  }

  if (isp_hw_limit->num_hw_streams > 2) {
    /* If both priority and preferred streams exists merge non preferred, non prioity
       streams with preferred streams, if they have pp_mask*/
    if (priority_streams->stream_num && session_param->preferred_mapping.stream_num) {
      for (i = 0; i <  session_param->preferred_mapping.stream_num; i++) {
        if (session_param->preferred_mapping.streams[i].stream_mask) {
          for (j = 0; j < streams_desc->num_streams; j++) {
            if ((streams_desc->postprocess_mask[j]) &&
              !((1 << streams_desc->type[j]) & combined_stream_mask)) {
              session_param->preferred_mapping.streams[i].stream_mask |=
              (1 << streams_desc->type[j]);
            }
          }
          break;
        }
      }
    }
  } else {
    if (session_param->preferred_mapping.stream_num) {
      for (i = 0; i <  session_param->preferred_mapping.stream_num; i++) {
        if (session_param->preferred_mapping.streams[i].stream_mask) {
          for (j = 0; j < streams_desc->num_streams; j++) {
            if (streams_desc->postprocess_mask[j] &&
              !((1 << streams_desc->type[j]) & combined_stream_mask)) {
              session_param->preferred_mapping.streams[i].stream_mask |=
              (1 << streams_desc->type[j]);
            }
          }
          break;
        }
      }
    }
  }
}

/** isp_util_map_streams_to_hw_hal1
 *    @session_param: handle to store session param
 *    @streams_desc: description of all streams sent by HAL
 *    @isp_hw_limit: ISP HW limitations
 *
 *  Decide the mapping of the streams to the HW streams for HAL1.
 *
 * Return TRUE on success and FALSE if streams could not be mapped
 **/
static boolean isp_util_map_streams_to_hw_hal1(
  isp_session_param_t *session_param, cam_stream_size_info_t *streams_desc,
  isp_submod_hw_limit_t *isp_hw_limit)
{
  isp_preferred_streams priority_streams;
  boolean is_video = FALSE, prior_analysis = FALSE;
  boolean ret;
  uint32_t i;

  if (session_param->preferred_mapping.stream_num >
      isp_hw_limit->num_hw_streams) {
    ISP_ERR("ISP cann't support %d preferred outputs.",
      session_param->preferred_mapping.stream_num);
  }

  /* check is it a video mode */
  for (i = 0; i < streams_desc->num_streams; i++) {
    if (streams_desc->type[i] == CAM_STREAM_TYPE_VIDEO) {
      is_video = TRUE;
      break;
    }
  }

  /* check if analysis stream should be priority stream */
  if (isp_hw_limit->num_hw_streams > 2) {
    prior_analysis = TRUE;
  }

  memset(&priority_streams, 0, sizeof(priority_streams));
  for (i = 0; i < streams_desc->num_streams; i++) {
    if ((streams_desc->type[i] == CAM_STREAM_TYPE_ANALYSIS && prior_analysis) ||
        (streams_desc->type[i] == CAM_STREAM_TYPE_SNAPSHOT && !is_video)) {
      priority_streams.streams[priority_streams.stream_num].max_streams_num++;
      priority_streams.streams[priority_streams.stream_num].stream_mask |=
        1 << streams_desc->type[i];
      priority_streams.stream_num++;
    }
  }

  isp_util_merge_non_prioirty_streams(session_param, streams_desc,
    isp_hw_limit, &priority_streams);

  /* Amend stream preferences to priority streams */
  ret = isp_util_merge_stream_preferrences(&priority_streams,
    &session_param->preferred_mapping);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_merge_stream_preferrences");
    return FALSE;
  }

  for (i = 0; i < priority_streams.stream_num; i++) {
    ISP_DBG("INFO: priority stream: %d num: %d mask: 0x%x", i,
      priority_streams.streams[i].max_streams_num,
      priority_streams.streams[i].stream_mask);
  }

  ret = isp_util_map_streams(session_param, streams_desc, isp_hw_limit,
    &priority_streams, FALSE);
  if (ret == FALSE) {
    ISP_ERR("failed: stream mapping");
  }

  return ret;
}

/** isp_util_map_streams_to_hw_hal3
 *    @session_param: handle to store session param
 *    @streams_desc: description of all streams sent by HAL
 *    @isp_hw_limit: ISP HW limitations
 *
 *  Decide the mapping of the streams to the HW streams for HAL3.
 *
 * Return TRUE on success and FALSE if streams could not be mapped
 **/
static boolean isp_util_map_streams_to_hw_hal3(
  isp_session_param_t *session_param, cam_stream_size_info_t *streams_desc,
  isp_submod_hw_limit_t *isp_hw_limit)
{
  isp_preferred_streams priority_streams;
  boolean ret;
  boolean is_analysis = FALSE;
  boolean is_raw = FALSE;
  boolean is_snap = FALSE;
  uint32_t i;

  if (session_param->preferred_mapping.stream_num >
      isp_hw_limit->num_hw_streams) {
    ISP_ERR("ISP cann't support %d preferred outputs.",
      session_param->preferred_mapping.stream_num);
  }

  /* check if we have analysis stream */
  for (i = 0; i < streams_desc->num_streams; i++) {
    if (streams_desc->type[i] == CAM_STREAM_TYPE_ANALYSIS) {
      is_analysis = TRUE;
      break;
    }
  }

#ifdef _DRONE_
  /* check if we have raw and snapshot streams - in this case if 3 hw streams
   * are available and Analysis needs 2 planes we need to map it to preview
   * otherwise number of VFE WMs won't be enough */
  if (is_analysis) {
    if (streams_desc->format[i] != CAM_FORMAT_Y_ONLY) {
      for (i = 0; i < streams_desc->num_streams; i++) {
        if (streams_desc->type[i] == CAM_STREAM_TYPE_RAW) {
          is_raw = TRUE;
        } else if (streams_desc->type[i] == CAM_STREAM_TYPE_SNAPSHOT) {
          is_snap = TRUE;
        }
      }
    }
  }
#endif

  memset(&priority_streams, 0, sizeof(priority_streams));
  if (!(is_raw && is_snap) && isp_hw_limit->num_hw_streams > 2) {
    for (i = 0; i < streams_desc->num_streams; i++) {
      if (streams_desc->type[i] == CAM_STREAM_TYPE_ANALYSIS) {
        priority_streams.streams[priority_streams.stream_num].max_streams_num++;
        priority_streams.streams[priority_streams.stream_num].stream_mask |=
          1 << streams_desc->type[i];
        priority_streams.stream_num++;
      }
    }
  } else if (is_analysis) {
    for (i = 0; i <  session_param->preferred_mapping.stream_num; i++) {
      if (session_param->preferred_mapping.streams[i].stream_mask &
         (1 << CAM_STREAM_TYPE_PREVIEW)) {
         session_param->preferred_mapping.streams[i].stream_mask |=
         (1 << CAM_STREAM_TYPE_ANALYSIS);
         break;
      }
    }
  }

  isp_util_merge_non_prioirty_streams(session_param, streams_desc,
    isp_hw_limit, &priority_streams);

  ret = isp_util_merge_stream_preferrences(&priority_streams,
    &session_param->preferred_mapping);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_merge_stream_preferrences");
    return FALSE;
  }

  for (i = 0; i < priority_streams.stream_num; i++) {
    ISP_DBG("INFO: priority stream: %d num: %d mask: 0x%x", i,
      priority_streams.streams[i].max_streams_num,
      priority_streams.streams[i].stream_mask);
  }

  ret = isp_util_map_streams(session_param, streams_desc, isp_hw_limit,
    &priority_streams, TRUE);
  if (ret == FALSE) {
    ISP_ERR("failed: stream mapping");
  }

  return ret;
}

/** isp_util_decide_stream_mapping
 *    @session_param: handle to store session param
 *    @streams_desc: description of all streams sent by HAL
 *    @isp:
 *
 *  Save the dimension info of all the streams, and use
 *  to decide the mapping of the user streams to HW streams.
 *
 * Return TRUE on success and FALSE if streams could not be mapped
 **/
static boolean isp_util_decide_stream_mapping(
  isp_session_param_t *session_param, cam_stream_size_info_t *streams_desc,
  isp_t *isp)
{
  uint32_t               i;
  boolean                ret = FALSE;

  if (!session_param || !streams_desc) {
    ISP_ERR("failed: %p %p", session_param, streams_desc);
    return FALSE;
  }

  ret = isp_resource_get_hw_limitations(&isp->isp_resource,
    &session_param->isp_hw_limit, session_param->num_isp, session_param->hw_id);
  if (!ret) {
     ISP_ERR("failed");
     return FALSE;
  }

  if (session_param->hal_version == CAM_HAL_V3) {
    ret = isp_util_map_streams_to_hw_hal3(session_param, streams_desc,
      &session_param->isp_hw_limit);
  } else {
    ret = isp_util_map_streams_to_hw_hal1(session_param, streams_desc,
      &session_param->isp_hw_limit);
  }
  if (ret == FALSE) {
    ISP_ERR("failed: stream mapping");
    /* clean up stream mapping */
    session_param->stream_port_map.num_streams = 0;
    return ret;
  }

  ISP_INFO("stream_port_map num streams %d",
    session_param->stream_port_map.num_streams);
  for (i = 0; i < session_param->stream_port_map.num_streams; i++) {
    ISP_INFO("INFO: type %d resolution %dx%d hw_stream %d need_native_buff %d "
      "controllable_output %d shared_output %d"
      "is_changed %d changed_dim %dx%d",
      session_param->stream_port_map.streams[i].stream_type,
      session_param->stream_port_map.streams[i].stream_sizes.width,
      session_param->stream_port_map.streams[i].stream_sizes.height,
      session_param->stream_port_map.streams[i].hw_stream,
      session_param->stream_port_map.streams[i].native_buffer,
      session_param->stream_port_map.streams[i].controlable_output,
      session_param->stream_port_map.streams[i].shared_output,
      session_param->stream_port_map.streams[i].is_changed,
      session_param->stream_port_map.streams[i].changed_stream_sizes.width,
      session_param->stream_port_map.streams[i].changed_stream_sizes.height);
  }

  return ret;
}

static boolean isp_util_is_mipi_stream(cam_format_t fmt)
{
  switch(fmt) {
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
      return TRUE;
      break;

    default:
      return FALSE;
      break;
  }

}

static boolean isp_util_is_camif_or_ideal_stream(cam_format_t fmt)
{
  switch(fmt) {
    /* CAMIF RAW Format*/
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_BGGR:
    /* IDEAL RAW Format*/
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR:

      return TRUE;
      break;

    default:
      return FALSE;
      break;
  }

}

/** isp_util_print_meta_stream_info
 *    @session_param: handle to store session param
 *    @streams_desc: description of all streams sent by HAL
 *
 *  Print stream description for HAL and sensor output
 *
 *  None
 **/
static void isp_util_print_meta_stream_info(
  cam_stream_size_info_t *streams_desc, boolean *is_mipi_stream)
{
  uint32_t i = 0;

  if (!streams_desc) {
    ISP_ERR("failed: streams_desc %p", streams_desc);
    return;
  }

  if (streams_desc->num_streams) {
    for (i = 0; i < streams_desc->num_streams && i < ISP_MAX_STREAMS; i++) {
      if (streams_desc->margins[i].widthMargins > 0.0f ||
        streams_desc->margins[i].heightMargins > 0.0f) {
        ISP_INFO("Margin: Stream type %d Resolution: %dx%d pp_mask: 0x%x",
          streams_desc->type[i], streams_desc->stream_sz_plus_margin[i].width,
          streams_desc->stream_sz_plus_margin[i].height,
          streams_desc->postprocess_mask[i]);
      } else {
        if ((streams_desc->type[i] == CAM_STREAM_TYPE_RAW) &&
          (streams_desc->num_streams == 1)) {
          /* Here the assumption is we have only one continous *
           * RDI Stream: for example used for depth calculation*
           * by HAL.                                           */
           *is_mipi_stream = TRUE;
        }
        ISP_INFO("Stream type %d Resolution: %dx%d pp_mask: 0x%x",
          streams_desc->type[i], streams_desc->stream_sizes[i].width,
          streams_desc->stream_sizes[i].height,
          streams_desc->postprocess_mask[i]);
      }
    }
  } else {
    ISP_INFO("Deallocate resources");
  }
}

/** isp_util_unlink_session_stream
 *    @module: mct module handle
 *    @isp:
 *    @session_param: handle to store session param
 *
 *  Unlink session stream for ISP internal modules
 *
 * Return TRUE on success and FALSE if streams could not be mapped
 **/
boolean isp_util_unlink_session_stream(mct_module_t *module,
  isp_session_param_t *session_param)
{
  boolean             ret;
  isp_stream_param_t *stream_param = NULL;
  mct_list_t         *l_port = NULL;
  mct_port_t         *port = NULL;

  /* Get session based stream */
  ret = isp_util_get_stream_param_from_type(module, CAM_STREAM_TYPE_PARM,
    session_param, &stream_param);
  if ((ret == FALSE) || !stream_param) {
    ISP_ERR("failed: to get session based stream %d %p", ret,
      stream_param);
    return FALSE;
  }

  /* Find sinc port */
  l_port = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
    &stream_param->stream_info.identity,
    isp_util_find_port_based_on_identity);
  if (!l_port || !l_port->data) {
    ISP_ERR("failed: sinkport for session based stream");
    return FALSE;
  }
  port = (mct_port_t *)l_port->data;

  /* Internal link session based stream */
  ret = isp_resource_destroy_internal_link(port,
    stream_param->stream_info.identity);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_resource_destroy_internal_link");
  }

  return ret;
}

/** isp_util_link_session_stream
 *    @module: mct module handle
 *    @isp:
 *    @session_param: handle to store session param
 *
 *  Link session stream for ISP internal modules
 *
 * Return TRUE on success and FALSE if streams could not be mapped
 **/
static boolean isp_util_link_session_stream(mct_module_t *module, isp_t *isp,
  isp_session_param_t *session_param)
{
  boolean             ret;
  isp_stream_param_t *stream_param = NULL;
  mct_list_t         *l_port = NULL;
  mct_port_t         *port = NULL;

  /* Get session based stream */
  ret = isp_util_get_stream_param_from_type(module, CAM_STREAM_TYPE_PARM,
    session_param, &stream_param);
  if ((ret == FALSE) || !stream_param) {
    ISP_ERR("failed: to get session based stream %d %p", ret,
      stream_param);
    return FALSE;
  }

  /* Find sinc port */
  l_port = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
    &stream_param->stream_info.identity,
    isp_util_find_port_based_on_identity);
  if (!l_port || !l_port->data) {
    ISP_ERR("failed: sinkport for session based stream");
    return FALSE;
  }
  port = (mct_port_t *)l_port->data;

  /* Internal link session based stream */
  ret = isp_resource_create_link(module, port, &isp->isp_resource,
    &stream_param->stream_info);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_pipeline_create_link");
  }

  return ret;
}

/** isp_handler_set_sensor_dim
 *    @module: mct module handle
 *    @identity: identity
 *    @hw_id: array of ISP hw IDs
 *    @num_isp: num of ISPs
 *
 *  Send stream ifno to internal modules
 *
 * Return TRUE on success and FALSE if streams could not be mapped
 **/
static boolean isp_handler_set_sensor_dim(isp_session_param_t *session_param,
  unsigned int identity, isp_hw_id_t *hw_id, uint32_t num_isp)
{
  mct_event_t             event;
  isp_private_event_t     private_event;
  uint32_t                i;
  boolean                 ret = TRUE;

  /* Set Sensor dimension to Scalar */
  memset(&event, 0, sizeof(event));
  memset(&private_event, 0, sizeof(private_event));

  private_event.type = ISP_PRIVATE_SET_SENSOR_DIM;
  private_event.data = &session_param->sensor_output_info;

  for (i = 0; i < num_isp; i++) {
    if (session_param->offline_num_isp > 0 &&
      session_param->offline_hw_id[0] == hw_id[i]) {
      /* Offline ISP case, Overwrite with  Offline Input Cfg*/
      private_event.data = &session_param->offline_input_cfg;
    }
  }

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.u.module_event.module_event_data = &private_event;

  for (i = 0; i < num_isp; i++) {
    ret = isp_util_forward_event_to_internal_pipeline(session_param, &event,
      hw_id[i]);
    if (ret == FALSE) {
      ISP_ERR("failed: ret %d", ret);
    }
  }

  return ret;
}

/** isp_util_send_hw_limit_to_internal_pipeline
 *    @session_param: handle to store session param
 *    @identity: identity
 *    @hw_id: array of ISP hw IDs
 *    @num_isp: num of ISPs
 *
 *  Send information which HW stream is linked to internal pipeline
 *
 * Return TRUE on success and FALSE if streams could not be mapped
 **/
boolean isp_util_send_hw_limit_to_internal_pipeline(
  isp_session_param_t *session_param, uint32_t identity, isp_hw_id_t *hw_id,
  uint32_t num_isp)
{
  mct_event_t           event;
  isp_private_event_t   private_event;
  isp_scaler_hw_limit_t isp_hw_limit;
  uint32_t              i;
  boolean               ret = TRUE;

  memset(&event, 0, sizeof(event));
  memset(&private_event, 0, sizeof(private_event));

  isp_hw_limit.num_hw_streams = session_param->isp_hw_limit.num_hw_streams;
  for (i = 0; i < isp_hw_limit.num_hw_streams; i++) {
    isp_hw_limit.hw_limits[i].hw_stream_id =
      session_param->isp_hw_limit.hw_limits[i].hw_stream_id;
    isp_hw_limit.hw_limits[i].max_width =
      session_param->isp_hw_limit.hw_limits[i].max_width;
    isp_hw_limit.hw_limits[i].max_height =
      session_param->isp_hw_limit.hw_limits[i].max_height;
    isp_hw_limit.hw_limits[i].max_scale_ratio =
      session_param->isp_hw_limit.hw_limits[i].max_scale_ratio;
  }

  private_event.type = ISP_PRIVATE_HW_LIMITATIONS;
  private_event.data = &isp_hw_limit;

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.u.module_event.module_event_data = &private_event;

  for (i = 0; i < num_isp; i++) {
    ret = isp_util_forward_event_to_internal_pipeline(session_param, &event,
      hw_id[i]);
    if (ret == FALSE) {
      ISP_ERR("failed: ret %d", ret);
    }
  }

  return ret;
}

/** isp_util_update_internal_pipeline
 *    @session_param: handle to store session param
 *    @identity: identitysession_param->hw_id[hw_index]hw_id: array of ISP hw IDs
 *    @num_isp: num of ISPs
 *
 *  Update internal pipeline
 *
 * Return TRUE on success and FALSE if streams could not be mapped
 **/
boolean isp_util_update_internal_pipeline(isp_session_param_t *session_param,
  uint32_t identity, isp_hw_id_t *hw_id, uint32_t num_isp)
{
  boolean  ret = TRUE;

  ret = isp_handler_set_sensor_dim(session_param, identity, hw_id, num_isp);
  if (ret == FALSE) {
    ISP_ERR("failed: to set sensor dimension");
    return ret;
  }

  ret = isp_util_send_hw_limit_to_internal_pipeline(session_param, identity,
    hw_id, num_isp);
  if (ret == FALSE) {
    ISP_ERR("failed: to send hw limitations");
    return ret;
  }

  return ret;
}

static int isp_util_calc_num_plane(cam_stream_type_t stream_type)
{
  int plane_num = 0;

  switch (stream_type) {
  case CAM_STREAM_TYPE_ANALYSIS:
    plane_num = 1;
    break;
  default:
    plane_num = 2;
    break;
  }

  return plane_num;
}


/** isp_util_handle_stream_info
 *    @module: mct module handle
 *    @session_param: handle to store session param
 *    @streams_desc: description of all streams sent by HAL
 *
 *  Handle stream info
 *
 * Return TRUE on success and FALSE if streams could not be mapped
 **/
boolean isp_util_handle_stream_info(mct_module_t *module,
  isp_session_param_t *session_param, cam_stream_size_info_t *streams_desc)
{
  isp_t        *isp = NULL;
  boolean      ret  = TRUE;
  boolean      stream_mapped  = FALSE;
  unsigned int i    = 0;
  boolean  is_mipi_stream = FALSE;
  int vfe1_reserved_rdi = 0;

  if (!module || !session_param || !streams_desc){
    ISP_ERR("failed: module %p session_param %p streams_desc %p",
      module, session_param, streams_desc);
    return FALSE;
  }
  isp_util_print_meta_stream_info(streams_desc, &is_mipi_stream);
  session_param->is_depth = FALSE;
  if (streams_desc->type[i] == CAM_STREAM_TYPE_DEPTH) {
    session_param->is_depth = TRUE;
    ISP_ERR("<non-fatal> we dont need an ISP resource for Depth sensor");
    return TRUE;
  }
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));

  isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
    return FALSE;
  }
  /* do not clean up if num_isp is 0*/
  if (session_param->num_isp) {
     session_param->rdi_only = FALSE;
     ret = isp_util_unlink_session_stream(module, session_param);
     if (ret == FALSE) {
         ISP_ERR("failed: isp_util_link_session_stream");
         goto ERROR;
     }
     /* Check if only session stream is linked to ISP module */
     if (session_param->l_stream_params &&
         session_param->l_stream_params->next_num) {
         ISP_ERR("failed: Cannot free ISP resource. %d streams are unlinked.",
           session_param->l_stream_params->next_num);
         isp_stream_param_t *stream_param = NULL;
         if (session_param->l_stream_params->next_num == 1) {
         /* Online streams streamed off, offline will be done indepedendently */
             ret = isp_util_get_stream_param_from_type(module,
               CAM_STREAM_TYPE_OFFLINE_PROC, session_param, &stream_param);
             if ((ret == FALSE) || !stream_param) {
               ISP_ERR("failed: 1 pending unlink and its not offline %d %p",
                       ret, stream_param);
               PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
               return FALSE;
             } else {
               ISP_ERR("A pending unlink and its offline %d continue", ret);
             }
         } else {
              ISP_ERR("failed: 1 pending unlink and its not offline %d %p",
                       ret, stream_param);
              PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
              return FALSE;
         }
     }
     ret = isp_resource_deallocate(module, &isp->isp_resource,
           session_param->session_id, &session_param->num_isp,
           session_param->hw_id);
     if (ret == FALSE) {
      ISP_ERR("failed: isp resource deallocate");
      PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
      return FALSE;
     }
  }

 if (!streams_desc->num_streams) {
    ISP_INFO("streams_desc num_streams is 0 ");
    PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
    return TRUE;
  }

  session_param->stream_port_map.raw_stream_exists = FALSE;
  if (streams_desc->num_streams > ISP_MAX_STREAMS) {
      ISP_ERR("failed: Max supported pix supported streams %d, current %d",
         ISP_MAX_STREAMS, streams_desc->num_streams);
      PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
      return FALSE;
  }
  session_param->rdi_only = TRUE;
  /* Filling number of streams in session info */
  session_param->stream_port_map.num_streams = streams_desc->num_streams;
  /* check if we have a camif raw stream, we need this info to veto usage of dual VFE*/
  for(i = 0; i < streams_desc->num_streams; i++) {
    if((streams_desc->type[i] == CAM_STREAM_TYPE_RAW) &&
        (isp_util_is_camif_or_ideal_stream(streams_desc->format[i]) == TRUE)){
      session_param->stream_port_map.raw_stream_exists = TRUE;
      break;
    }
  }

  for (i = 0; i < streams_desc->num_streams; i++) {
    if (streams_desc->type[i] != CAM_STREAM_TYPE_RAW){
      session_param->rdi_only = FALSE;
      break;
    }
  }

  session_param->sync_type = streams_desc->sync_type;

  if (is_mipi_stream == TRUE &&
      !session_param->sensor_output_info.need_camif_raw) {
    ISP_ERR("<non-fatal> failed: isp resource allocate for mipi stream");
    PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
    return TRUE;
  }
  for (i =0; i < ISP_HW_MAX; i++) {
    ret = isp_resource_allocate(module, &isp->isp_resource,
       session_param->session_id, &session_param->num_isp,
       session_param->hw_id, 0, i);
    if (ret == FALSE) {
      ISP_ERR("failed: isp resource allocate");
      PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
      return FALSE;
    }

    ret = isp_util_decide_stream_mapping(session_param, streams_desc, isp);
    if (ret == TRUE) {
       stream_mapped = TRUE;
       break;
    }
    ret = isp_resource_deallocate(module, &isp->isp_resource,
           session_param->session_id, &session_param->num_isp,
           session_param->hw_id);
    if (ret == FALSE) {
       PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
       ISP_ERR("failed: isp resource deallocate");
       return FALSE;
    }
    /*set num_stream to map again, could be reset in decide_mapping*/
    session_param->stream_port_map.num_streams = streams_desc->num_streams;
 }
 if(stream_mapped == FALSE) {
   PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
   ISP_ERR("failed: stream mapping");
   return FALSE;
 }

  vfe1_reserved_rdi = isp->isp_resource.isp_hw_limt.vfe1_reserved_rdi;
  /* 0 <= vfe1_reserved_rdi <= 3, -1 means no RDI is reserved, set to 0*/
  if (vfe1_reserved_rdi < 0 || vfe1_reserved_rdi > 3)
    vfe1_reserved_rdi = 0;

  /* only enter when the smaller vfe is allocated before*/
  if (session_param->num_isp == 1 && vfe1_reserved_rdi != 0 &&
    session_param->hw_id[0] == isp->isp_resource.sorted_hw_ids[0]) {
    int               num_wm[3] = {0,0,0};
    isp_hw_streamid_t hw_stream = 0;
    int               num_used_rdi = 0;
    cam_stream_type_t stream_type;

    /* estimate the number of wm would be used*/
    for (i = 0; i < session_param->stream_port_map.num_streams; i++) {
      hw_stream = session_param->stream_port_map.streams[i].hw_stream;
      stream_type = session_param->stream_port_map.streams[i].stream_type;
      if (hw_stream < ISP_HW_STREAM_IDEAL_RAW) {
        num_wm[hw_stream] = num_wm[hw_stream] > isp_util_calc_num_plane(stream_type) ?
          num_wm[hw_stream] : isp_util_calc_num_plane(stream_type);
      }
    }
    num_wm[0] = num_wm[0] + num_wm[1] + num_wm[2];
    if (session_param->sensor_output_info.need_camif_raw)
      num_wm[0] += 1;

    /* requested wms + reserved rdi shoudl not exceed the wm limitation*/
    if (vfe1_reserved_rdi + num_wm[0] >
      (int)isp->isp_resource.isp_resource_info[VFE1].isp_pipeline->num_wms) {
      ISP_HIGH("max_rdi_allowed %d, num_wm %d",vfe1_reserved_rdi, num_wm[0]);

      /* return back the smaller VFE*/
      isp_resource_deallocate(module, &isp->isp_resource,
        session_param->session_id, &session_param->num_isp,
        session_param->hw_id);

      /* reallocate the bigger VFE*/
      session_param->num_isp = 1;
      session_param->hw_id[0] = ISP_HW_0;
      ret = isp_resource_allocate(module, &isp->isp_resource,
        session_param->session_id, &session_param->num_isp,
        session_param->hw_id, 0, 0);
      if (ret == FALSE) {
        ISP_ERR("failed: isp resource allocate");
        PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
        return FALSE;
      }

      ret = isp_util_decide_stream_mapping(session_param, streams_desc, isp);
      if (ret == FALSE) {
        ISP_ERR("failed: stream mapping");
        goto ERROR;
      }
    }
  }

  // set secure mode bit for VFE topology.
  for (i = 0; i < session_param->num_isp; i++)
  {
     if (session_param->hw_id[i]) {
       streams_desc->topology |= 1 << 9;
     } else {
       streams_desc->topology |= 1 << 8;
     }
     ISP_INFO("INFO: set VFE %d in secure mode value %x\n",
       session_param->hw_id[i], streams_desc->topology);
  }

  ret = isp_util_link_session_stream(module, isp, session_param);
  if (ret == FALSE) {
     ISP_ERR("failed: isp_util_link_session_stream");
     goto ERROR;
  }

  ret = isp_util_update_internal_pipeline(session_param,
     session_param->session_based_ide, session_param->hw_id,
     session_param->num_isp);
  if (ret == FALSE) {
     ISP_ERR("failed: isp_util_update_internal_pipeline");
     goto ERROR;
  }
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return ret;
ERROR:
  ret = isp_resource_deallocate(module, &isp->isp_resource,
    session_param->session_id, &session_param->num_isp,
    session_param->hw_id);
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return ret;
}

/** isp_util_set_meta_hw_data_overwrite
 *    @isp:
 *    @session_id:
 *    @hal_version: input data
 *
 *  save the hal version for feature use.
 *
 * return: 0 on success and -1 if streams could not be mapped
 **/
boolean isp_util_set_meta_hw_data_overwrite(
  isp_session_param_t *session_param, void *hal_overwrite_data)
{
  boolean                 ret = true;
  boolean                 is_offline_overwrite;
  cam_hw_data_overwrite_t hw_overwrite_data;

  if (!session_param || !hal_overwrite_data) {
    ISP_ERR("failed: %p %p", session_param, hal_overwrite_data);
    return false;
  }

  memset(&hw_overwrite_data, 0, sizeof(cam_hw_data_overwrite_t));
  hw_overwrite_data = *((cam_hw_data_overwrite_t *)hal_overwrite_data);

  switch (hw_overwrite_data.overwrite_type) {
  case CAM_INTF_OVERWRITE_MINI_CHROMATIX_OFFLINE:
    ret = isp_util_set_chromatix_meta(session_param,
      (mct_bus_msg_sensor_metadata_t*)hw_overwrite_data.chromatix_data_overwrite);
    break;

  case CAM_INTF_OVERWRITE_ISP_HW_DATA_OFFLINE:
    ret = isp_util_set_meta_hw_update_list(session_param,
      hw_overwrite_data.isp_hw_data_list);
    break;

  case CAM_INTF_OVERWRITE_MINI_CHROMATIX_ONLINE:
    break;

  case CAM_INTF_OVERWRITE_ISP_HW_DATA_ONLINE:
    break;

  default:
    ISP_ERR(" invalid overwrite type %d", hw_overwrite_data.overwrite_type);
    break;
  }

  if (ret == FALSE) {
    ISP_ERR("failed: handle HW overwrite failed: type %d",
      hw_overwrite_data.overwrite_type);
  }

  return ret;
}

/** isp_util_set_meta_hw_update_list
 *    @isp:
 *    @session_id:
 *    @hal_version: input data
 *
 *  save the hal version for feature use.
 *
 * return: 0 on success and -1 if streams could not be mapped
 **/
boolean isp_util_set_meta_hw_update_list(
  isp_session_param_t *session_param, void *meta_hw_update_list)
{
  boolean ret = true;

  if (!session_param || !meta_hw_update_list) {
    ISP_ERR("failed: %p %p", session_param, meta_hw_update_list);
    return false;
  }

  session_param->fetch_eng_cfg_data.offline_hw_update_list =
    meta_hw_update_list;
  session_param->fetch_eng_cfg_data.is_hw_update_list_overwrite = true;

  return ret;
}

/** isp_util_set_preferred_mapping
 *    @module: mct module handle
 *    @event: module event
 *
 *  Handle preferred stream mapping
 *
 * Return TRUE on success and FALSE if fails
 **/
boolean isp_util_set_preferred_mapping(mct_module_t *module, mct_event_t *event)
{
  isp_session_param_t      *session_param;
  isp_preferred_streams    *preferred_mapping;
  uint32_t                  i, curr_num, j;
  boolean                   ret;

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  if (session_param->stream_port_map.num_streams) {
    ISP_ERR("failed: Stream mapping is already done.");
    return FALSE;
  }

  preferred_mapping = (isp_preferred_streams *)
    event->u.module_event.module_event_data;

  /* Amend new preferred streams */
  curr_num = session_param->preferred_mapping.stream_num;

  for (i = 0; i < preferred_mapping->stream_num; i++) {
    for (j = 0; j < (i + curr_num); j++) {
      if (session_param->preferred_mapping.streams[j].stream_mask &
         preferred_mapping->streams[i].stream_mask) {
        ISP_ERR("error: Requested same stream type on different ports\n");
        return FALSE;
      }
    }
  }
  for (i = 0; i < preferred_mapping->stream_num; i++) {
    session_param->preferred_mapping.streams[i + curr_num] =
      preferred_mapping->streams[i];
  }
  session_param->preferred_mapping.stream_num += preferred_mapping->stream_num;

  return TRUE;
}

/** isp_util_set_chromatix_meta
 *    @isp:
 *    @session_id:
 *    @hal_version: input data
 *
 *  Save the HAL version for feature use.
 *
 * Return: 0 on success and -1 if streams could not be mapped
 **/
boolean isp_util_set_chromatix_meta(
  isp_session_param_t           *session_param,
  mct_bus_msg_sensor_metadata_t *chromatix_meta)
{
  boolean ret = TRUE;

  if (!session_param || !chromatix_meta) {
    ISP_ERR("failed: %p %p", session_param, chromatix_meta);
    return FALSE;
  }

  session_param->fetch_eng_cfg_data.offline_chromatix.chromatixComPtr =
    chromatix_meta->common_chromatix_ptr;
  session_param->fetch_eng_cfg_data.offline_chromatix.chromatixPtr =
    chromatix_meta->chromatix_ptr;
  session_param->fetch_eng_cfg_data.offline_chromatix.chromatixCppPtr =
    chromatix_meta->cpp_chromatix_ptr;
  session_param->fetch_eng_cfg_data.is_chromatix_overwrite = TRUE;

  return ret;
}

/** isp_util_set_hal_version
 *    @session_param: handle to store session param
 *    @hal_version: HAL version
 *
 *  Save the HAL version
 *
 * Return TRUE on success and FALSE if streams could not be mapped
 **/
boolean isp_util_set_hal_version(isp_session_param_t *session_param,
  cam_hal_version_t *hal_version)
{

  if (!session_param || !hal_version) {
    ISP_ERR("failed: %p %p", session_param, hal_version);
    return FALSE;
  }
  session_param->hal_version = *hal_version;

  return TRUE;
}

boolean isp_util_handle_vhdr(mct_module_t *module,
  isp_session_param_t *session_param, cam_sensor_hdr_type_t *video_hdr_mode)
{
  if (module == NULL || session_param == NULL || video_hdr_mode == NULL) {
    ISP_ERR("NULL pointer,isp_module %p, session parm = %p, video_hdr_mode %p",
      module, session_param, video_hdr_mode);
    return FALSE;
  }

  ISP_DBG("video hdr mode = %d", *video_hdr_mode);
  if (*video_hdr_mode == CAM_SENSOR_HDR_STAGGERED) {
    session_param->svhdr_enb = 1;

  } else {
    session_param->svhdr_enb = 0;
  }

  return TRUE;
}

/** isp_util_free_offline_shared_queue:
 *  @offline_trigger_param_q: shared param queue
 *
 *  Free shared param queue
 *
 *  Return TRUE on success and FALSE on failure
 **/
void isp_util_free_offline_shared_queue(mct_queue_t *offline_trigger_param_q)
{
  isp_saved_events_t          *new_events = NULL;
  uint32_t                     i = 0;

  do {
    new_events = mct_queue_pop_head(offline_trigger_param_q);
    if (new_events) {
      for (i = 0; i < ISP_SET_MAX; i++) {
        if (new_events->set_params[i] &&
          (new_events->set_params_valid[i] == TRUE)) {
          free(new_events->set_params[i]->u.ctrl_event.control_event_data);
          free(new_events->set_params[i]);
        }
      }
      for (i = 0; i < ISP_MODULE_EVENT_MAX; i++) {
        if (new_events->module_events[i] &&
          (new_events->module_events_valid[i] == TRUE)) {
          free(new_events->module_events[i]->u.module_event.module_event_data);
          free(new_events->module_events[i]);
        }
      }
      free(new_events);
    }
  } while (new_events);
}
/** isp_util_get_session_stream_param_from_type:
 *
 *  @module: mct module
 *  @stream_type: stream type
 *  @session_param: handle to store session param
 *  @stream_param: handle to store stream param
 *  @session_id: session id
 *
 *  Retrieve session and stream param based on stream type
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_get_session_stream_param_from_type(mct_module_t *module,
  cam_stream_type_t stream_type, isp_session_param_t **session_param,
  isp_stream_param_t **stream_param, uint32_t session_id)
{
  boolean     ret = TRUE;
  isp_t      *isp = NULL;
  mct_list_t *l_session_params = NULL;
  mct_list_t *l_stream_params = NULL;

  if (!module || !session_param || !stream_param) {
    ISP_ERR("failed: module %p session_param %p stream param %p", module,
      session_param, stream_param);
    return FALSE;
  }

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  l_session_params = mct_list_find_custom(isp->l_session_params, &session_id,
    isp_util_compare_sessionid_from_session_param);
  if (!l_session_params) {
    ISP_ERR("failed: l_session_params %p for session %d", l_session_params,
      session_id);
    return FALSE;
  }

  *session_param = (isp_session_param_t *)l_session_params->data;
  if (!(*session_param)) {
    ISP_ERR("failed: session_param %p", *session_param);
    return FALSE;
  }

  l_stream_params = mct_list_find_custom((*session_param)->l_stream_params,
    &stream_type, isp_util_compare_stream_type);
  if (!l_stream_params) {
    ISP_ERR("failed: l_stream_params %p for stream type %d", l_stream_params,
      stream_type);
    return FALSE;
  }

  *stream_param = (isp_stream_param_t *)l_stream_params->data;
  if (!(*stream_param)) {
    ISP_ERR("failed: stream_param %p", *stream_param);
    return FALSE;
  }

  return ret;
}

/** isp_util_update_hw_param_hw_id:
 *
 *  @isp_module: isp module
 *  @session_param : isp session parameters
   @is_applied: applied update/current update
 *
 *  update hw parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_util_update_hw_param_offline(mct_module_t *module,
  isp_session_param_t *session_param, isp_hw_id_t hw_id)
{
  boolean                      ret = TRUE;
  isp_hw_update_params_t      *hw_update_params = NULL;
  isp_parser_params_t         *parser_params = NULL;
  isp_hw_update_list_params_t *hw_update_list_params = NULL;
  uint8_t                      i = 0;
  uint8_t                      j = 0;
  isp_hw_read_info_t           *dmi_info;

  if (module == NULL || session_param == NULL) {
    ISP_ERR("NULL pointer,isp_module %p, session parm = %p,",
      module, session_param);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;
  if (hw_update_params == NULL) {
    ISP_ERR("NULL pointer, hw_update_params = %p", hw_update_params);
    return FALSE;
  }

  /* 1. Save a copy of settings applied in previous frame
        metadata reporting
     2. Save a copy of stats and algo params applied in previous
        frame to parse the stats
   */
  parser_params = &session_param->parser_params;
  PTHREAD_MUTEX_LOCK(&parser_params->mutex);

  for (i = 0; i < session_param->offline_num_isp; i++) {
    hw_update_list_params = &hw_update_params->
      hw_update_list_params[session_param->offline_hw_id[i]];

    /* Save the settings applied for metadata reporting */
    hw_update_list_params->applied_meta_dump_parms =
      hw_update_list_params->meta_dump_parms;
    hw_update_list_params->applied_frame_meta =
      hw_update_list_params->frame_meta;

    /* DMI info, being used for gamma alone  */
    for (j = 0; j < ISP_METADUMP_MAX_NUM; j++) {
      dmi_info = &hw_update_list_params->dmi_tbl.dmi_info[j];
      if (dmi_info->read_length == 0) {
        continue;
      } else {
        /*free valid dmi table, vfe0 and vfe1 if dual vfe*/
        hw_update_list_params->applied_dmi_tbl.dmi_info[j] = *dmi_info;
        memset(dmi_info, 0, sizeof(isp_hw_read_info_t));
      }
    }

    /* update stats params */
    parser_params->stats_params[hw_id] =
      *hw_update_list_params->stats_params;

    /* Passing & update Algo params */
    parser_params->algo_parm[hw_id] =
      hw_update_list_params->algo_parm;
  }

  PTHREAD_MUTEX_UNLOCK(&parser_params->mutex);

  return ret;
}

/** isp_set_pipeline_delay:
 *
 *  This function runs in MCTL thread context.
 *
 * This store pipeline delays
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean isp_set_pipeline_delay(mct_module_t *module, uint32_t session_id,
  mct_pipeline_session_data_t *session_data)
{
  isp_session_param_t *session_param;
  boolean              ret = TRUE;

  ret = isp_util_get_session_params(module, session_id, &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("cannot find session (%d)", session_id);
    return FALSE;
  }

  session_param->max_apply_delay =
    session_data->max_pipeline_frame_applying_delay;
  session_param->max_reporting_delay =
      session_data->max_pipeline_meta_reporting_delay;

  return TRUE;
}
/** isp_fill_frame_format_param:
 *
 *    @fmt: format
 *    @is_encoder:
 *
 *    Get stream format to axi path conversion
 *
 *    Return stream format
 **/
boolean isp_fill_frame_format_param(cam_format_t fmt,
  uint32_t *output)
{
  uint32_t bpp = 0, bayer_format = 0, pack_type = 0;

  if (!output) {
    ISP_ERR("out param invalid");
    return FALSE;
  }

  *output = 0;

  switch (fmt) {
    case CAM_FORMAT_YUV_RAW_8BIT_YUYV: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_YCBYCR;
      pack_type = ISP_CAM_PACK_PLAIN8;
      break;
    }
    case CAM_FORMAT_YUV_RAW_8BIT_YVYU: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_YCRYCB;
      pack_type = ISP_CAM_PACK_PLAIN8;
      break;
    }
    case CAM_FORMAT_YUV_RAW_8BIT_UYVY: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_CRYCBY;
      pack_type = ISP_CAM_PACK_PLAIN8;
      break;
    }
    case CAM_FORMAT_YUV_RAW_8BIT_VYUY: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_CBYCRY;
      pack_type = ISP_CAM_PACK_PLAIN8;
      break;
    }
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_QCOM;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_MIPI;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_PLAIN8;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_PLAIN8;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_PLAIN8;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_PLAIN8;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_PLAIN16;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_PLAIN16;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_PLAIN16;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR: {
      bpp = ISP_CAM_BPP_8;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_PLAIN16;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_PLAIN16;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_PLAIN16;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_PLAIN16;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR: {
      bpp = ISP_CAM_BPP_10;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_PLAIN16;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_GB;
      pack_type = ISP_CAM_PACK_PLAIN16;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_GR;
      pack_type = ISP_CAM_PACK_PLAIN16;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_RG;
      pack_type = ISP_CAM_PACK_PLAIN16;
      break;
    }
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR: {
      bpp = ISP_CAM_BPP_12;
      bayer_format = ISP_CAM_BAYER_BG;
      pack_type = ISP_CAM_PACK_PLAIN16;
      break;
    }
    default: {
      ISP_ERR("Invalid format");
    }
  }

  *output = (bpp) | (bayer_format << 8) | (pack_type << 16);

  return TRUE;
}

  /** isp_util_send_adrc_hw_module_mask:
 *
 *
 * This func send ADRC module mask to 3A.
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean isp_util_send_adrc_hw_module_mask(mct_module_t *module,
  isp_session_param_t *session_param)
{
  boolean                      ret = TRUE;
  isp_t                       *isp = NULL;
  isp_hw_update_params_t      *hw_update_params = NULL;
  isp_hw_update_list_params_t *hw_update_list_params = NULL;
  isp_pipeline_t              *isp_pipeline = NULL;
  mct_event_t                  mct_event;
  mct_event_stats_isp_adrc_hw_module_t isp_adrc_hw_module;
  uint32_t                     i = 0, hw_index = 0;

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;
  if (hw_update_params == NULL) {
    ISP_ERR("NULL pointer, hw_update_params = %p", hw_update_params);
    return FALSE;
  }

  /* Call internal isp event for all ISP hw */
  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    if (session_param->hw_id[hw_index] != ISP_HW_MAX) {
      break;
    }
  }
  if (hw_index == ISP_HW_MAX) {
    ISP_ERR(" Error! no isp found");
    return FALSE;
  }

  isp_pipeline = isp->isp_resource.isp_resource_info[session_param->hw_id[hw_index]].isp_pipeline;
  if (!isp_pipeline) {
    ISP_ERR("failed: isp_pipeline %p", isp_pipeline);
    return FALSE;
  }

  if (isp_pipeline->func_table->adrc_hw_module_id_mask) {
    isp_adrc_hw_module.isp_adrc_hw_module_id_mask =
      isp_pipeline->func_table->adrc_hw_module_id_mask();

    /* Send event to 3A to get stats needed by 3A */
    memset(&mct_event, 0, sizeof(mct_event));

    mct_event.direction = MCT_EVENT_DOWNSTREAM;
    mct_event.type = MCT_EVENT_MODULE_EVENT;
    mct_event.identity = session_param->session_based_ide;
    mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_ADRC_MODULE_MASK;
    mct_event.u.module_event.module_event_data = (void *)&isp_adrc_hw_module;

    ret = isp_util_forward_event_downstream_to_type(module, &mct_event,
      MCT_PORT_CAPS_STATS);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_ISP_ADRC_MODULE_MASK to 3A");
    }
  }
  return ret;
}

/** isp_util_setloglevel:
 *
 *  @name: ISP hw module unique name
 *  @isp_log_sub_modules_t: module id of the hardware module
 *
 *  This function sets the log level of the hardware module id
 *  passed based on the setprop.
 *  persist.vendor.camera.debug.xxx (xxx = hw module name)
 *  It also arbitrates betweeen the global setprop
 *  persist.vendor.camera.global.debug
 *  The final log level is decided on whichever is higher
 *  0 - ERR Logs
 *  1 - HIGH Logs + ERR logs
 *  2 - DBG logs + HIGH Logs + ERR Logs
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean isp_util_setloglevel(const char *name,
  isp_log_sub_modules_t mod_id)
{
  uint32_t      globalloglevel = 0;
  char          prop[PROPERTY_VALUE_MAX];
  char          buf[255];
  uint32_t      isp_module_loglevel = 0,
                isploglevel;

  if (!name) {
    ISP_ERR("failed: %p", name);
    return FALSE;
  }

  strlcpy(buf, "persist.vendor.camera.debug.", sizeof(buf));
  strlcat(buf, name, sizeof(buf));

  property_get(buf, prop, "0");
  isp_module_loglevel = atoi(prop);
  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.camera.global.debug", prop, "0");
  globalloglevel = atoi(prop);
  if (globalloglevel > isp_module_loglevel) {
    isp_module_loglevel = globalloglevel;
  }
  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.camera.isp.debug", prop, "0");
  isploglevel = atoi(prop);
  if (isploglevel > isp_module_loglevel) {
    isp_module_loglevel = isploglevel;
  }
  isp_modules_loglevel[mod_id] = isp_module_loglevel;
  ISP_DBG("%s: ###ISP_Loglevel %d", buf, isp_module_loglevel);
  return TRUE;
}
