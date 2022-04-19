/**********************************************************************
*  Copyright (c) 2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "module_imgbase.h"
#include "module_thirdpartyeis.h"

#define WAIT_TIMEOUT 500000000
#define PTHREAD_COND_WAIT_TIME(cond_ptr, mutex_ptr, \
    timeout_ptr, ns, rc) \
  { \
    clock_gettime(CLOCK_REALTIME, (timeout_ptr)); \
    (timeout_ptr)->tv_nsec += ns;\
    if ((timeout_ptr)->tv_nsec > 999999999) { \
      (timeout_ptr)->tv_nsec -= 999999999; \
      (timeout_ptr)->tv_sec += 1; \
    } \
    rc = pthread_cond_timedwait(cond_ptr, \
      mutex_ptr, timeout_ptr);\
  }

/**
 *  Static functions
 **/
static int32_t module_thirdpartyeis_client_created(imgbase_client_t *p_client);
static int32_t module_thirdpartyeis_client_destroy(imgbase_client_t *p_client);
static int32_t module_thirdpartyeis_client_streamon(imgbase_client_t * p_client);
static int32_t module_thirdpartyeis_client_prestreamoff( imgbase_client_t * p_client,
  uint32_t identity);
static int32_t module_thirdpartyeis_flush_frame( imgbase_client_t * p_client,
  uint32_t event_id);
static int32_t module_thirdpartyeis_client_flush_done( imgbase_client_t * p_client,
  imgbase_stream_t *p_stream);
static int32_t module_thirdpartyeis_update_meta(imgbase_client_t *p_client,
  img_meta_t *p_meta);
//static boolean module_thirdpartyeis_handle_stream_crop_event(uint32_t identity,
//  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
//  img_core_ops_t *p_core_ops, boolean *is_evt_handled);
static boolean module_thirdpartyeis_handle_isp_out_dim_event(uint32_t identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client, img_core_ops_t *p_core_ops, boolean *is_evt_handled);
static boolean module_thirdpartyeis_handle_dis_config_event(uint32_t identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client, img_core_ops_t *p_core_ops, boolean *is_evt_handled);
static boolean module_thirdpartyeis_handle_dis_update_event(uint32_t identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client, img_core_ops_t *p_core_ops, boolean *is_evt_handled);

static boolean module_thirdpartyeis_handle_skew_event(uint32_t identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client, img_core_ops_t *p_core_ops, boolean *is_evt_handled);
static boolean module_thirdpartyeis_handle_exposure_time_event(uint32_t identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client, img_core_ops_t *p_core_ops, boolean *is_evt_handled);
static boolean module_thirdpartyeis_handle_accel_update_event(uint32_t identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client, img_core_ops_t *p_core_ops, boolean *is_evt_handled);
static boolean module_thirdpartyeis_handle_gyro_update_event(uint32_t identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client, img_core_ops_t *p_core_ops, boolean *is_evt_handled);

static boolean module_thirdpartyeis_handle_iface_request_pp_divert_event(
  uint32_t identity, mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled);
static int32_t module_thirdpartyeis_handle_meta_stream_info(mct_event_control_parm_t
  *p_ctrl_parm, imgbase_client_t *p_client, img_core_ops_t *p_core_ops);


/** g_caps:
 *
 *  Set the capabilities for thirdpartyeis module
*/
static img_caps_t g_caps_p = {
  .num_input = 1,
  .num_output = 1,
  .num_meta = 1,
  .inplace_algo = 0,
  .num_release_buf = 0,
  .hold_meta = 0,
  .num_hold_outbuf = 0,
  .use_internal_bufs = 1,
  .internal_buf_cnt = 35,
};
static img_caps_t g_caps_v = {
  .num_input = 1,
  .num_output = 1,
  .num_meta = 1,
  .inplace_algo = 0,
  .num_release_buf = 0,
  .hold_meta = 0,
  .num_hold_outbuf = 0,
  .use_internal_bufs = 1,
  .internal_buf_cnt = 38,
};

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_client_created = module_thirdpartyeis_client_created,
  .imgbase_client_destroy = module_thirdpartyeis_client_destroy,
  .imgbase_client_pre_streamoff = module_thirdpartyeis_client_prestreamoff,
  .imgbase_client_flush_done = module_thirdpartyeis_client_flush_done,
  .imgbase_client_streamon = module_thirdpartyeis_client_streamon,
  .imgbase_client_flush_frame = module_thirdpartyeis_flush_frame,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_SENSOR_ROLLING_SHUTTER_SKEW] =
    module_thirdpartyeis_handle_skew_event,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_SENSOR_GET_EXPOSURE_TIME] =
    module_thirdpartyeis_handle_exposure_time_event,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_ISP_OUTPUT_DIM] =
    module_thirdpartyeis_handle_isp_out_dim_event,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_ISP_DIS_CONFIG] =
    module_thirdpartyeis_handle_dis_config_event,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_STATS_DIS_UPDATE] =
    module_thirdpartyeis_handle_dis_update_event,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_STATS_GYRO_STATS] =
    module_thirdpartyeis_handle_gyro_update_event,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_ACCEL_VECTOR_UPDATE] =
    module_thirdpartyeis_handle_accel_update_event,
  .imgbase_handle_module_event[ MCT_EVENT_MODULE_IFACE_REQUEST_PP_DIVERT] =
    module_thirdpartyeis_handle_iface_request_pp_divert_event,
  .imgbase_handle_ctrl_parm[CAM_INTF_META_STREAM_INFO] =
    module_thirdpartyeis_handle_meta_stream_info,
  .imgbase_client_update_meta = module_thirdpartyeis_update_meta,
  .exec_mode = IMG_EXECUTION_SW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .force_cache_op = FALSE,
};

/**
 * Function: module_thirdpartyeis_client_created
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
int32_t module_thirdpartyeis_client_created(imgbase_client_t *p_client)
{
  IDBG_LOW("%s:%d: E", __func__, __LINE__);
  p_client->rate_control = TRUE;
  p_client->exp_frame_delay = 0LL;
  p_client->ion_fd = open("/dev/ion", O_RDONLY);
  p_client->before_cpp = TRUE;
  p_client->feature_mask = CAM_QTI_FEATURE_VENDOR_EIS;

  /* process thirdpartyeis only when bufs received on port where
    video stream is mapped */
  p_client->streams_to_process =
    (1 << CAM_STREAM_TYPE_VIDEO) | (1 << CAM_STREAM_TYPE_PREVIEW) ;

  p_client->process_all_frames = FALSE;

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

  /* Requires extra buffers . Will update after input from IS module*/
  p_client->isp_extra_native_buf += 0;

  /*Specifying that module will change dimension of buffer for consideration
       during output buffer operations */
  p_client->is_dim_change = TRUE;

  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  img_thirdpartyeis_module_t *p_thirdpartyeis_mod =
    (img_thirdpartyeis_module_t *)p_mod->mod_private;
  img_thirdpartyeis_client_t *p_thirdpartyeis_client;

  /* Create ThirdPartyEIS private client and set default params */
  p_thirdpartyeis_client = calloc(1, sizeof(img_thirdpartyeis_client_t));
  if (!p_thirdpartyeis_client) {
    IDBG_ERROR("Failed, ThirdParty EIS client is NULL");
    return IMG_ERR_NO_MEMORY;
  }

  p_thirdpartyeis_client->p_session_data =
    &p_thirdpartyeis_mod->session_data[p_client->session_id - 1];
  p_client->p_private_data = p_thirdpartyeis_client;
  p_thirdpartyeis_client->p_client = p_client;

  /* Set default params for imglib base client */
  p_client->before_cpp = p_mod->caps.before_cpp;
  p_client->feature_mask = p_mod->feature_mask;
  IDBG_LOW("p_mode feature_mask: %d", p_mod->feature_mask);

  p_client->processing_disabled = FALSE;
  if (p_mod->hal_version == CAM_HAL_V3) {
    p_client->process_all_frames = FALSE;
  } else {
    p_client->process_all_frames = TRUE;
  }

  IDBG_MED("Streams to process %d", p_client->streams_to_process);
  IDBG_MED("Does algo change dimension %d", p_client->is_dim_change);
  IDBG_MED("Is algo before CPP %d", p_mod->caps.before_cpp);
  IDBG_MED("Session id %d", p_client->session_id);

  IDBG_MED(":X");

  return IMG_SUCCESS;
}

/**
 * Function: module_thirdpartyeis_client_streamon
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
static int32_t module_thirdpartyeis_client_streamon(
  imgbase_client_t * p_client)
{
  IDBG_LOW("%s:%d: E", __func__, __LINE__);
  int rc = IMG_ERR_GENERAL;
  p_client->processing_disabled = FALSE;
  /*Use the deault framproc callback*/
  rc = IMG_COMP_SET_PARAM(&p_client->comp, QIMG_PARAM_SET_LIB_CALLBACK,
       NULL);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Fail to set callback");
    return IMG_ERR_INVALID_OPERATION;
  }
  return IMG_SUCCESS;
}
/**
 * Function: module_thirdpartyeis_flush_frame
 *
 * Description: function called on receiving flush frame
 *               event from MCT
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int32_t module_thirdpartyeis_flush_frame(
  imgbase_client_t * p_client, uint32_t event_id)
{
  int stream_idx = 0;
  imgbase_stream_t *p_stream = NULL;
  img_component_ops_t *p_comp = &p_client->comp;
  stream_idx = module_imgbase_find_stream_by_identity(p_client, event_id);
  if (stream_idx < 0) {
    IDBG_ERROR(" Error, cannot find identity %x", event_id);
    return IMG_ERR_INVALID_INPUT;
  }
  p_stream = &p_client->stream[stream_idx];
  if (NULL == p_stream) {
    IDBG_ERROR("%s %d] Cannot find stream mapped to client", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  if (p_client->streams_to_process & (1 << p_stream->stream_info->stream_type)) {
    IDBG_LOW("Stream to process type %d ", p_stream->stream_info->stream_type);
  } else {
    IDBG_ERROR("Stream NOT to process type %d return", p_stream->stream_info->stream_type);
    return IMG_ERR_GENERAL;
  }

  IDBG_LOW("%s sending flush for video stream",__func__);
  IMG_COMP_NOTIFY(p_comp, IMG_COMP_EVENT_FLUSH);
  return IMG_SUCCESS;
}

/**
 * Function: module_thirdpartyeis_client_prestreamoff
 *
 * Description: function called before streamoff
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
static int32_t module_thirdpartyeis_client_prestreamoff(
  imgbase_client_t * p_client, uint32_t identity)
{
  int stream_idx = 0;
  imgbase_stream_t *p_stream = NULL;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;

  stream_idx = module_imgbase_find_stream_by_identity(p_client, identity);
  if (stream_idx < 0) {
    IDBG_ERROR("[%s] Cannot find stream mapped to idx %x",p_mod->name,
      p_client->divert_identity);
    goto error;
  }

  p_stream = &p_client->stream[stream_idx];

  p_client->processing_disabled = TRUE;
  p_stream->streamon = FALSE;

  IDBG_LOW("id %x Stream to process type %d stream type %d ", identity,
    p_client->streams_to_process, p_stream->stream_info->stream_type);
  if (p_client->streams_to_process & (1 << p_stream->stream_info->stream_type)) {
    IDBG_LOW("Stream to process type %d ", p_stream->stream_info->stream_type);
  } else {
    IDBG_ERROR("Stream NOT to process type %d return", p_stream->stream_info->stream_type);
    return IMG_ERR_GENERAL;
  }
  return IMG_SUCCESS;

  error:
  return IMG_ERR_GENERAL;

}

/**
 * Function: module_thirdpartyeis_client_flush_done
 *
 * Description: function called when flush is complete
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *   @p_stream - IMGLIB_BASE stream
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_thirdpartyeis_client_flush_done(
  imgbase_client_t * p_client, imgbase_stream_t *p_stream)
{
  IDBG_LOW("Flush done indication!");
  p_stream->eis_config.flush_done = 1;
  pthread_cond_broadcast(&(p_client->cond));
  IDBG_LOW("Released streamoff!");

  return IMG_SUCCESS;
}


/**
 * Function: module_thirdpartyeis_client_destroy
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
int32_t module_thirdpartyeis_client_destroy(
  imgbase_client_t *p_client)
{
  IDBG_MED(": E");
  if (p_client->ion_fd >= 0) {
    close(p_client->ion_fd);
    p_client->ion_fd = -1;
  }

  if (p_client->p_private_data) {
    free(p_client->p_private_data);
    p_client->p_private_data = NULL;
  }

  IDBG_MED(": X");
  return IMG_SUCCESS;
}
/**
 * Function: module_thirdpartyeis_handle_meta_stream_info
 *
 * Description: This function handles the Meta stream Info Ctrl
 * param
 *
 * Arguments:
 *   p_mod_event - pointer of module event
 *   p_client - pointer to imgbase client
 *   p_core_ops - pointer to imgbase module ops
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int32_t module_thirdpartyeis_handle_meta_stream_info(
  mct_event_control_parm_t *p_ctrl_parm,
  imgbase_client_t *p_client, img_core_ops_t *p_core_ops)
{
  cam_stream_size_info_t *meta_stream_info;
  int rc = IMG_SUCCESS;
  uint32_t i = 0;
  bool video_4k = 0;

  IDBG_LOW("Got CAM_INTF_META_STREAM_INFO");

  if (!p_ctrl_parm || !p_client || !p_core_ops) {
    IDBG_ERROR(" Error input");
    return IMG_ERR_INVALID_INPUT;
  }

  meta_stream_info = (cam_stream_size_info_t *)(p_ctrl_parm->parm_data);
  if (!meta_stream_info) {
    IDBG_ERROR("Meta stream info is NULL");
    return IMG_ERR_INVALID_INPUT;
  }

  for (i = 0; i < meta_stream_info->num_streams; i++) {
    IDBG_LOW("META_STREAM_INFO type = %d w x h : %d x %d pp_mask = 0x%x",
      meta_stream_info->type[i],
      meta_stream_info->stream_sizes[i].width,
    meta_stream_info->stream_sizes[i].height,
    meta_stream_info->postprocess_mask[i]);
    if (meta_stream_info->type[i] == CAM_STREAM_TYPE_VIDEO) {
      video_4k = IS_4k_VIDEO(meta_stream_info->type[i],
        meta_stream_info->stream_sizes[i].width,
        meta_stream_info->stream_sizes[i].height,
        meta_stream_info->postprocess_mask[i]);
      break;
    }
  }

  if (video_4k) {
    IDBG_LOW("Video is 4k");
    /* send preferred stream mapping requesting for preview and
      video on the same port */
    p_client->set_preferred_mapping = TRUE;
    p_client->preferred_mapping_single.stream_num = 2;
    p_client->preferred_mapping_single.streams[0].max_streams_num = 1;
    p_client->preferred_mapping_single.streams[0].stream_mask =
      (1 << CAM_STREAM_TYPE_PREVIEW);
    p_client->preferred_mapping_single.streams[1].max_streams_num = 2;
    p_client->preferred_mapping_single.streams[1].stream_mask =
      (1 << CAM_STREAM_TYPE_VIDEO) | (1 << CAM_STREAM_TYPE_SNAPSHOT);

    p_client->preferred_mapping_multi.stream_num = 2;
    p_client->preferred_mapping_multi.streams[0].max_streams_num = 1;
    p_client->preferred_mapping_multi.streams[0].stream_mask =
      (1 << CAM_STREAM_TYPE_PREVIEW);
    p_client->preferred_mapping_multi.streams[1].max_streams_num = 2;
    p_client->preferred_mapping_multi.streams[1].stream_mask =
      (1 << CAM_STREAM_TYPE_VIDEO) | (1 << CAM_STREAM_TYPE_SNAPSHOT);
  }


  return rc;
}

static boolean module_thirdpartyeis_handle_skew_event(
  uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  IDBG_LOW(": E");
  int rc = TRUE;
  img_thirdpartyeis_client_t *p_thirdpartyeis_client = NULL;
  img_thirdpartyeis_session_data_t *p_session_data = NULL;

  int64_t frame_readout_time = *((int64_t*)(p_mod_event->module_event_data));

  p_thirdpartyeis_client = (img_thirdpartyeis_client_t *)(p_client->p_private_data);
  if (NULL == p_thirdpartyeis_client ) {
    /* Error printed in the macro*/
    IDBG_ERROR("input is NULL");
    return IMG_ERR_INVALID_INPUT;
  }
  p_session_data = (img_thirdpartyeis_session_data_t *)(p_thirdpartyeis_client->p_session_data);
  if (NULL == p_session_data) {
    /* Error printed in the macro*/
    IDBG_ERROR("input is NULL");
    return IMG_ERR_INVALID_INPUT;
  }
  p_session_data->thirdpartyeis_cfg.rolling_shutter_skew = frame_readout_time;
  IDBG_LOW("stream_type %d frame_readout_time %lld",
    p_client->isp_output_dim_stream_info.stream_type,
    frame_readout_time);

  IDBG_LOW(": X");
  return rc;
}

static boolean module_thirdpartyeis_handle_exposure_time_event(
  uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  IDBG_LOW(": E");
  int rc = TRUE;
  img_thirdpartyeis_client_t *p_thirdpartyeis_client = NULL;
  img_thirdpartyeis_session_data_t *p_session_data = NULL;

  int64_t exposure_time = *((int64_t*)(p_mod_event->module_event_data));

  p_thirdpartyeis_client = (img_thirdpartyeis_client_t *)(p_client->p_private_data);
  if (NULL == p_thirdpartyeis_client) {
    /* Error printed in the macro*/
    IDBG_ERROR("input is NULL");
    return IMG_ERR_INVALID_INPUT;
  }
  p_session_data = (img_thirdpartyeis_session_data_t *)(p_thirdpartyeis_client->p_session_data);
  if (NULL == p_session_data) {
    /* Error printed in the macro*/
    IDBG_ERROR("input is NULL");
    return IMG_ERR_INVALID_INPUT;
  }
  p_session_data->thirdpartyeis_cfg.exposure_time = exposure_time;
  IDBG_INFO("stream_type %d exposure_time %lld",
    p_client->isp_output_dim_stream_info.stream_type,
    exposure_time);

  IDBG_LOW(": X");
  return rc;
}

/**
 * Function: module_thirdpartyeis_handle_isp_out_dim_event
 *
 * Description: This function handles isp out dim event
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
boolean module_thirdpartyeis_handle_isp_out_dim_event(
  uint32_t identity, mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client, img_core_ops_t *p_core_ops, boolean *is_evt_handled)
{
  boolean fwd_event = FALSE;

  IDBG_LOW("E");

  if (!p_mod_event || !p_client || !p_core_ops) {
    IDBG_ERROR("Error input");
    return fwd_event;
  }

  //Base module needs information for i/p buffer
  *is_evt_handled = FALSE;
  //Do not forward, since DIS config will forward it.
  fwd_event = FALSE;

  IDBG_LOW("X");
  return fwd_event;
}



/**
 * Function: thirdpartyeis_update_stream_dimensions
 *
 * Description: This function updates stream dimensions
 *
 * Arguments:
 *   stream - Stream
 *   dis_config - dis config event payload
 *
 * Return values:
 *     int32_t - error code
 *
 * Notes: none
 **/
int32_t thirdpartyeis_update_stream_dimensions(
  imgbase_stream_t *stream,
  isp_dis_config_info_t *dis_config)
{
  uint32_t i = 0;
  int color_fmt = 0;
  if (!stream || !dis_config) {
    IDBG_ERROR("failed stream_params %p, dis_config %p",
       stream, dis_config);
    return -EFAULT;
  }

  IDBG_LOW("B id %x stream op %dX%d plane[0] %dX%d len %d"
    " plane[1] %dX%d len %d frame_len %d\n",
    stream->identity,
    stream->stream_info->dim.width,
    stream->stream_info->dim.height,
    stream->stream_info->buf_planes.plane_info.mp[0].stride,
    stream->stream_info->buf_planes.plane_info.mp[0].scanline,
    stream->stream_info->buf_planes.plane_info.mp[0].len,
    stream->stream_info->buf_planes.plane_info.mp[1].stride,
    stream->stream_info->buf_planes.plane_info.mp[1].scanline,
    stream->stream_info->buf_planes.plane_info.mp[1].len,
    stream->stream_info->buf_planes.plane_info.frame_len);

  stream->stream_info->dim.width = dis_config->width;
  stream->stream_info->dim.height = dis_config->height;

  stream->stream_info->buf_planes.plane_info.frame_len = 0;
  color_fmt =
    module_imglib_common_get_venus_color_format(stream->stream_info->fmt);

  stream->stream_info->buf_planes.plane_info.frame_len =
    VENUS_BUFFER_SIZE(color_fmt, stream->stream_info->dim.width,
      stream->stream_info->dim.height);

  for (i = 0; i < stream->stream_info->buf_planes.plane_info.num_planes; i++) {
    if (i == 0) { // Y Plane
      stream->stream_info->buf_planes.plane_info.mp[i].stride =
        VENUS_Y_STRIDE(color_fmt, stream->stream_info->dim.width);
      stream->stream_info->buf_planes.plane_info.mp[i].scanline =
        VENUS_Y_SCANLINES(color_fmt, stream->stream_info->dim.height);
      if (stream->stream_info->fmt == CAM_FORMAT_YUV_420_NV12_UBWC) {
        stream->stream_info->buf_planes.plane_info.mp[i].len =
          (uint32_t)(MSM_MEDIA_ALIGN(
                    (stream->stream_info->buf_planes.plane_info.mp[i].stride *
                     stream->stream_info->buf_planes.plane_info.mp[i].scanline),
                     4096) +
          stream->stream_info->buf_planes.plane_info.mp[i].meta_len);
      } else {
        stream->stream_info->buf_planes.plane_info.mp[i].len =
          (stream->stream_info->buf_planes.plane_info.mp[i].stride *
          stream->stream_info->buf_planes.plane_info.mp[i].scanline);
      }
    } else {
      stream->stream_info->buf_planes.plane_info.mp[i].stride =
        VENUS_UV_STRIDE(color_fmt, stream->stream_info->dim.width);
      stream->stream_info->buf_planes.plane_info.mp[i].scanline =
        VENUS_UV_SCANLINES(color_fmt, stream->stream_info->dim.height);

    stream->stream_info->buf_planes.plane_info.mp[i].len =
        stream->stream_info->buf_planes.plane_info.frame_len -
        stream->stream_info->buf_planes.plane_info.mp[0].len;
    }
  }

  IDBG_LOW(",A id %x stream op %dX%d plane[0] %dX%d len %d"
    " plane[1] %dX%d len %d frame_len %d\n",
    stream->identity,
    stream->stream_info->dim.width,
    stream->stream_info->dim.height,
    stream->stream_info->buf_planes.plane_info.mp[0].stride,
    stream->stream_info->buf_planes.plane_info.mp[0].scanline,
    stream->stream_info->buf_planes.plane_info.mp[0].len,
    stream->stream_info->buf_planes.plane_info.mp[1].stride,
    stream->stream_info->buf_planes.plane_info.mp[1].scanline,
    stream->stream_info->buf_planes.plane_info.mp[1].len,
    stream->stream_info->buf_planes.plane_info.frame_len);
  return 0;
}

boolean module_thirdpartyeis_handle_dis_config_event(
  uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled)
{
  boolean fwd_event = TRUE;
  *is_evt_handled = TRUE;
  int32_t stream_idx;
  uint32_t i = 0 ;
  imgbase_stream_t *p_stream;
  mct_stream_info_t *stream_info = NULL;
  int rc = IMG_SUCCESS;
  img_lib_config_t lib_config;
  img_component_ops_t *p_comp = &p_client->comp;
  img_multicam_dim_info_t dim_info;

  isp_dis_config_info_t *dis_config =
    (isp_dis_config_info_t *) (p_mod_event->module_event_data);
  if (!dis_config){
    IDBG_ERROR("failed\n");
    return fwd_event;
  }

  IDBG_LOW(" MCT_EVENT_MODULE_ISP_DIS_CONFIG , sid = %u, strid = %x, "
    " col_num = %u, row_num = %u, w = %u, h = %u",
    dis_config->session_id, dis_config->stream_id,
    dis_config->col_num, dis_config->row_num,
    dis_config->width, dis_config->height);

  /*If the module is sink then output dim is same as stream_info,
    so don't handle*/
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  if (p_mod->module_type == MCT_MODULE_FLAG_SINK){
    IDBG_LOW("not handling MCT_EVENT_MODULE_ISP_DIS_CONFIG ");
    return fwd_event;
  }
  //Update the internal stream dimensions i.e this modules o/p dimensions
  stream_idx = module_imgbase_find_stream_by_identity(p_client,
    identity);
  if (stream_idx < 0) {
    IDBG_ERROR(" Cannot find stream mapped to idx %x")
    return IMG_ERR_NOT_FOUND;
  }

  p_stream = &p_client->stream[stream_idx];
  if (NULL == p_stream) {
    IDBG_ERROR(" Cannot find stream mapped to client");
    return IMG_ERR_NOT_FOUND;
  }
  IDBG_LOW("B Strm Info dim %dX%d strXsca %dX%d \n", p_stream->stream_info->dim.width,
    p_stream->stream_info->dim.height, p_stream->stream_info->buf_planes.plane_info.mp[0].stride,
    p_stream->stream_info->buf_planes.plane_info.mp[0].scanline,
    p_stream->stream_info->buf_planes.plane_info.frame_len);

  //Update internal stream dimension information
  thirdpartyeis_update_stream_dimensions(p_stream, dis_config);

  IDBG_LOW("A Strm Info dim %dX%d strXsca %dX%d \n", p_stream->stream_info->dim.width,
    p_stream->stream_info->dim.height, p_stream->stream_info->buf_planes.plane_info.mp[0].stride,
    p_stream->stream_info->buf_planes.plane_info.mp[0].scanline,
    p_stream->stream_info->buf_planes.plane_info.frame_len);

  memset(&dim_info, 0x0, sizeof(img_multicam_dim_info_t));
          dim_info.input_size.width =
            p_stream->isp_stream_info.dim.width;
          dim_info.input_size.height =
            p_stream->isp_stream_info.dim.height;
          dim_info.input_size.stride =
            p_stream->isp_stream_info.buf_planes.plane_info.mp[0].stride;
          dim_info.input_size.scanline =
            p_stream->isp_stream_info.buf_planes.plane_info.mp[0].scanline;
          dim_info.stream_type = p_stream->isp_stream_info.stream_type;
          dim_info.output_size.width =
            p_stream->stream_info->dim.width;
          dim_info.output_size.height =
            p_stream->stream_info->dim.height;
          dim_info.output_size.stride =
            p_stream->stream_info->buf_planes.plane_info.mp[0].stride;
          dim_info.output_size.scanline =
            p_stream->stream_info->buf_planes.plane_info.mp[0].scanline;
          dim_info.fps = dis_config->fps;

  lib_config.lib_data = &dim_info;
  lib_config.lib_param = IMG_ALGO_IN_FRAME_DIM;
  rc = IMG_COMP_GET_PARAM(p_comp, QIMG_PARAM_LIB_CONFIG, &lib_config);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("get param for stream info failed");
  }

  //Send ISP output dim event to CPP
  stream_info = p_stream->stream_info;

  mct_stream_info_t new_out_stream_info;
  new_out_stream_info.fmt = p_stream->isp_stream_info.fmt;
  new_out_stream_info.stream_type = stream_info->stream_type;
  new_out_stream_info.dim.width = stream_info->dim.width;
  new_out_stream_info.dim.height = stream_info->dim.height;
  new_out_stream_info.buf_planes.plane_info.num_planes =
    stream_info->buf_planes.plane_info.num_planes;
  new_out_stream_info.buf_planes.plane_info.frame_len = 0;

  IDBG_LOW("stream type %d, width %d, height %d num planes %d",
    stream_info->stream_type, new_out_stream_info.dim.width,
    new_out_stream_info.dim.height,
    new_out_stream_info.buf_planes.plane_info.num_planes);

  for (i = 0; i < stream_info->buf_planes.plane_info.num_planes; i++) {
    new_out_stream_info.buf_planes.plane_info.mp[i].stride =
      stream_info->buf_planes.plane_info.mp[i].stride;
    new_out_stream_info.buf_planes.plane_info.mp[i].scanline =
      stream_info->buf_planes.plane_info.mp[i].scanline;
    new_out_stream_info.buf_planes.plane_info.mp[i].offset =
      stream_info->buf_planes.plane_info.mp[i].offset;
    new_out_stream_info.buf_planes.plane_info.mp[i].offset_x =
      stream_info->buf_planes.plane_info.mp[i].offset;
    new_out_stream_info.buf_planes.plane_info.mp[i].offset_y =
      stream_info->buf_planes.plane_info.mp[i].offset;
    new_out_stream_info.buf_planes.plane_info.mp[i].len =
      stream_info->buf_planes.plane_info.mp[i].len;
    new_out_stream_info.buf_planes.plane_info.frame_len +=
      stream_info->buf_planes.plane_info.mp[i].len;
    IDBG_LOW("new plane %d, stride %d, scanline %d,"
      "offset_x %d, offset_y %d, len %d, offset:%d", i,
      new_out_stream_info.buf_planes.plane_info.mp[i].stride,
      new_out_stream_info.buf_planes.plane_info.mp[i].scanline,
      new_out_stream_info.buf_planes.plane_info.mp[i].offset_x,
      new_out_stream_info.buf_planes.plane_info.mp[i].offset_y,
      new_out_stream_info.buf_planes.plane_info.mp[i].len,
      new_out_stream_info.buf_planes.plane_info.mp[i].offset);
  }

  rc = mod_imgbase_send_event(identity, FALSE,
    MCT_EVENT_MODULE_ISP_OUTPUT_DIM, new_out_stream_info);

  if (!rc) {
    IDBG_ERROR(" Sending ISP OUT DIM failed %d", rc);
    return IMG_ERR_GENERAL;
  }

  return fwd_event;
}

/**
 * Function: module_thirdpartyeis_handle_dis_update_event
 *
 * Description: This function handles dis update event
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
boolean module_thirdpartyeis_handle_dis_update_event(
  uint32_t identity, mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client, img_core_ops_t *p_core_ops, boolean *is_evt_handled)
{
  int rc = IMG_SUCCESS;
  img_meta_t *p_meta = NULL;
  img_meta_t *p_peek_meta = NULL;
  int stream_idx = 0;
  imgbase_stream_t *p_stream = NULL;

  boolean fwd_event = FALSE;
  *is_evt_handled = TRUE;

   is_update_t *is_update =
    (is_update_t *)(p_mod_event->module_event_data);
  if(!is_update) {
    IDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return FALSE;
  }

  stream_idx = module_imgbase_find_stream_by_identity(p_client,
                 is_update->id);
  if (stream_idx < 0) {
    IDBG_ERROR(" Cannot find stream mapped to idx %x", is_update->id);
    return FALSE;
  }

  p_stream = &p_client->stream[stream_idx];

  IDBG_LOW("Got DIS update event for id %x frameid %d ",identity, is_update->frame_id);

  if (img_q_count(&p_client->meta_hold_q) > 0) {
    p_peek_meta = img_q_peek(&p_client->meta_hold_q);
    if (!p_peek_meta) {
      IDBG_ERROR("No meta queued in peek");
      goto error;
    }
  } else {
    module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
    if (CAM_HAL_V3 == p_mod->hal_version) {
      // For HAL3, this is not an error. There could be possibility that there
      // is no frame request for this frame_id, so we skipped frame and then
      // getting DIS update event.
      IDBG_MED("No meta queued in peek");
      return fwd_event;
    }
    IDBG_MED("p_peek_meta %p",p_peek_meta);
    return fwd_event;
  }

  if (!p_peek_meta) {
      IDBG_ERROR("No meta queued in peek");
      goto error;
  }

  IDBG_LOW("peek event frame id %d meta frameid %d meta count %d ",
    is_update->frame_id, p_peek_meta->frame_id, p_client->meta_hold_q.count);

  //Error handling for mismatched event
  if(p_peek_meta->frame_id < is_update->frame_id) {
    IDBG_ERROR("MISMATCH Frame queued, but matrix for future frame. Need to flush");
    while(p_peek_meta->frame_id < is_update->frame_id) {
      //dequeue the buffer
      p_meta = img_q_dequeue(&p_client->meta_hold_q);
      if (!p_meta) {
        IDBG_ERROR("No meta queued");
        goto error;
      }

      //Update meta informaiton
      memcpy(p_meta->eis_cfg.transform_matrix, p_stream->eis_config.transform_matrix,
        sizeof(p_meta->eis_cfg.transform_matrix));
      p_meta->eis_cfg.num_mesh_x = p_stream->eis_config.eis_num_mesh_x;
      p_meta->eis_cfg.num_mesh_y = p_stream->eis_config.eis_num_mesh_y;
      p_meta->eis_cfg.last_frame = FALSE;

      //queue the buffer
      rc = IMG_COMP_Q_META_BUF(&p_client->comp, p_meta);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("Q_meta failed rc %d", rc);
        goto error;
      }
      p_client->num_meta_queued++;

      //Update peek to decide additional flush needed or not
      p_peek_meta = img_q_peek(&p_client->meta_hold_q);
      if (!p_peek_meta) {
        IDBG_ERROR("No meta queued in peek");
        goto error;
      }
      IDBG_LOW("flush peek event frame id %d meta frameid %d",
        is_update->frame_id, p_peek_meta->frame_id);
    }
    if(p_peek_meta->frame_id == is_update->frame_id) {
      IDBG_LOW("Flush complete. State restored.");
    } else {
      IDBG_LOW("Flush. State Error.event frame id %d meta frameid %d",
        is_update->frame_id, p_peek_meta->frame_id);
      goto error;
    }
  } else if (p_peek_meta->frame_id > is_update->frame_id) {
    IDBG_ERROR("MISMATCH Old matrix. Ignore event, since no frame for it!");
    //Update matrix information since its latest
    memcpy(p_stream->eis_config.transform_matrix, is_update->transform_matrix,
      sizeof(is_update->transform_matrix));
    goto error;
  } else {
    IDBG_LOW("MATCH Matrix for the frame queued.");
  }

  IDBG_LOW("Getting held meta from count %d", p_client->meta_hold_q.count);
  p_meta = img_q_dequeue(&p_client->meta_hold_q);
  if (!p_meta) {
    IDBG_ERROR("No meta queued");
    goto error;
  }

  //Modify meta buffer
  IDBG_LOW("Event frame id %d meta frameid %d", is_update->frame_id,
    p_meta->frame_id);

  //Add event information to meta
  memcpy(p_meta->eis_cfg.transform_matrix, is_update->transform_matrix,
    sizeof(is_update->transform_matrix));
  //Store for dropped/streamoff cases
  memcpy(p_stream->eis_config.transform_matrix, is_update->transform_matrix,
    sizeof(is_update->transform_matrix));
  p_meta->eis_cfg.num_mesh_x = p_stream->eis_config.eis_num_mesh_x;
  p_meta->eis_cfg.num_mesh_y = p_stream->eis_config.eis_num_mesh_y;
  p_meta->eis_cfg.last_frame = FALSE;

  //queue the buffer
  rc = IMG_COMP_Q_META_BUF(&p_client->comp, p_meta);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Q_meta failed rc %d", rc);
    goto error;
  }
  p_client->num_meta_queued++;
  IDBG_LOW(" p_client->num_meta_queued %d", p_client->num_meta_queued);

  return fwd_event;

  error:
  IDBG_ERROR("Error path!");
  return fwd_event;
}

/**
 * Function: module_thirdpartyeis_handle_accel_update_event
 *
 * Description: This function handles accel update event
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
boolean module_thirdpartyeis_handle_accel_update_event(
  uint32_t identity, mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client, img_core_ops_t *p_core_ops, boolean *is_evt_handled)
{
  int rc = TRUE;
  img_thirdpartyeis_client_t *p_thirdpartyeis_client = NULL;
  img_thirdpartyeis_session_data_t *p_session_data = NULL;
  mct_event_accel_stats_t *accel_stats;

  if (!p_client) {
    IDBG_ERROR("Invalid p_client input %p", p_client);
    return IMG_ERR_INVALID_INPUT;
  }

  accel_stats = (mct_event_accel_stats_t *)(p_mod_event->module_event_data);
  if(!accel_stats) {
    IDBG_ERROR(" accel stats NULL, failed\n");
    return FALSE;
  }

  p_thirdpartyeis_client = (img_thirdpartyeis_client_t *)(p_client->p_private_data);
  p_session_data = (img_thirdpartyeis_session_data_t *)(p_thirdpartyeis_client->p_session_data);
  if ( NULL == p_thirdpartyeis_client || NULL == p_session_data) {
    /* Error printed in the macro*/
    IDBG_ERROR("input is NULL");
    return IMG_ERR_INVALID_INPUT;
  }

//time interval check -E
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t get_time = ((int64_t)tv.tv_sec * 1000000) + tv.tv_usec;
  IDBG_LOW("ACCEL gettime: %lld", get_time);
  int32_t diff = get_time - ts[0];
  if(diff > 33000)
    IDBG_LOW("accel update interval %ld is too long", diff);
  ts[0] = get_time;
//time interval check -X

  memset(&p_session_data->is_accel_data, 0, sizeof(mct_event_accel_data_t));
  memcpy(&p_session_data->is_accel_data, &accel_stats->is_accel_data, sizeof(mct_event_accel_data_t));

  return rc;
}

/**
 * Function: module_thirdpartyeis_handle_gyro_update_event
 *
 * Description: This function handles gyro update event
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
boolean module_thirdpartyeis_handle_gyro_update_event(
  uint32_t identity, mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client, img_core_ops_t *p_core_ops, boolean *is_evt_handled)
{
  uint32_t rc = TRUE;

  img_thirdpartyeis_client_t *p_thirdpartyeis_client = NULL;
  img_thirdpartyeis_session_data_t *p_session_data = NULL;
  mct_event_gyro_stats_t *gyro_stats;

  if (!p_client) {
    IDBG_ERROR("Invalid p_client input %p", p_client);
    return IMG_ERR_INVALID_INPUT;
  }

   gyro_stats =
    (mct_event_gyro_stats_t *)(p_mod_event->module_event_data);
  if(!gyro_stats) {
    IDBG_ERROR("l.%d> Gyro stats NULL, failed\n", __LINE__);
    return FALSE;
  }

  p_thirdpartyeis_client = (img_thirdpartyeis_client_t *)(p_client->p_private_data);
  p_session_data = (img_thirdpartyeis_session_data_t *)(p_thirdpartyeis_client->p_session_data);
  if ( NULL == p_thirdpartyeis_client || NULL == p_session_data) {
    /* Error printed in the macro*/
    IDBG_ERROR("input is NULL");
    return IMG_ERR_INVALID_INPUT;
  }

//time interval check -E
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t get_time = ((int64_t)tv.tv_sec * 1000000) + tv.tv_usec;
  IDBG_LOW("gyro gettime: %lld", get_time);
  int32_t diff = get_time - ts[1];
  if(diff > 33000)
    IDBG_LOW("gyro update interval %d is too long", diff);
  ts[1] = get_time;
//time interval check -X

  memset(&p_session_data->is_gyro_data, 0, sizeof(mct_event_gyro_data_t));
  memcpy(&p_session_data->is_gyro_data, &gyro_stats->is_gyro_data, sizeof(mct_event_gyro_data_t));

  return rc;
}

boolean module_thirdpartyeis_handle_iface_request_pp_divert_event(
  uint32_t identity, mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled)
{
  int rc = IMG_SUCCESS;
  boolean fwd_event = TRUE;
  int stream_idx = 0;
  imgbase_stream_t *p_stream = NULL;

  IDBG_LOW("PP divert request event for %x", identity);
  *is_evt_handled = FALSE;

  mct_event_is_config_t is_config;
  memset(&is_config, 0, sizeof(mct_event_is_config_t));

  stream_idx = module_imgbase_find_stream_by_identity(p_client, identity);

  if (stream_idx < 0) {
    IDBG_ERROR("%s %d] Cannot find stream mapped to idx %x", __func__, __LINE__,
      identity);
    return IMG_ERR_GENERAL;
  }

  p_stream = &p_client->stream[stream_idx];
  rc = mod_imgbase_send_event(identity, TRUE,
    MCT_EVENT_MODULE_IS_CONFIG, is_config);

  if (!rc) {
    IDBG_ERROR("%s:%d] Sending ISP OUT DIM failed %d",
    __func__, __LINE__, rc);
    return IMG_ERR_GENERAL;
  }
  memset(&p_stream->eis_config, 0, sizeof(img_eis_config_t));
  p_stream->eis_config.eis_num_mesh_x = is_config.num_mesh_x;
  p_stream->eis_config.eis_num_mesh_y = is_config.num_mesh_y;
  IDBG_LOW("Got IS input %f %d %d %d", is_config.margin_value,
        is_config.num_buffers, is_config.num_mesh_x, is_config.num_mesh_y);
  //Update the number of extra native bufs needed.
  uint32_t additional_isp_extra_native_buf = 0;
  if(p_stream->stream_info->stream_type == CAM_STREAM_TYPE_PREVIEW){
    additional_isp_extra_native_buf =
    (uint32_t)property_get_int32(
    "persist.vendor.camera.thirdparty.eis.additional_isp_extra_native_buf.preview",
    0);
  }
  else if(p_stream->stream_info->stream_type == CAM_STREAM_TYPE_VIDEO){
    additional_isp_extra_native_buf = (uint32_t)property_get_int32(
    "persist.vendor.camera.thirdparty.eis.additional_isp_extra_native_buf.video",
    18);
  }
  p_client->isp_extra_native_buf +=
    is_config.num_buffers + additional_isp_extra_native_buf;
  IDBG_INFO("isp_extra_native_buf:%d", p_client->isp_extra_native_buf);

  pp_buf_divert_request_t *divert_request;
  divert_request =
   (pp_buf_divert_request_t *)(p_mod_event->module_event_data);
  divert_request->buf_alignment = 128;
  divert_request->need_divert = TRUE;

  return fwd_event;
}

/**
 * Function: module_thirdpartyeis_update_meta
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
int32_t module_thirdpartyeis_update_meta(imgbase_client_t *p_client,
  img_meta_t *p_meta)
{
  IDBG_LOW("%s:%d: E", __func__, __LINE__);

  int rc = IMG_SUCCESS;
  img_thirdpartyeis_client_t *p_thirdpartyeis_client;
  img_thirdpartyeis_session_data_t *p_session_data;
  module_imgbase_t *p_base_mod;
  uint32_t meta_type;
  img_thirdparty_eis_cfg_t eis_cfg;

  if (!p_client || !p_meta) {
    IDBG_ERROR("Invalid input %p %p", p_client, p_meta);
    rc = IMG_ERR_INVALID_INPUT;
    return rc;
  }

  p_base_mod = (module_imgbase_t *)p_client->p_mod;

  /* Get Session Data */
  p_thirdpartyeis_client = (img_thirdpartyeis_client_t *)p_client->p_private_data;
  p_session_data = p_thirdpartyeis_client->p_session_data;

  /* Set CFG meta */
  memset(&eis_cfg, 0, sizeof(img_thirdparty_eis_cfg_t));

  switch (p_base_mod->feature_mask) {
    case CAM_QTI_FEATURE_VENDOR_EIS:
      meta_type = IMG_META_GYRO;
      memcpy(&eis_cfg.is_gyro_stats,
        &p_session_data->is_gyro_data, sizeof(mct_event_gyro_data_t));
      rc = img_set_meta(p_meta, meta_type, &eis_cfg.is_gyro_stats);

      meta_type = IMG_META_ACCEL;
      memcpy( &eis_cfg.is_accel_stats,
        &p_session_data->is_accel_data, sizeof(mct_event_accel_data_t) );
      rc = img_set_meta(p_meta, meta_type, &eis_cfg.is_accel_stats);

      for(uint32_t i=0;i<p_session_data->is_gyro_data.sample_len;i++){
        IDBG_LOW("p_session_data Gyro samples [%d] I %llu: G %llu, (%f, %f, %f)",
          i, p_session_data->is_gyro_data.frame_time,
          p_session_data->is_gyro_data.sample[i].timestamp,
          p_session_data->is_gyro_data.sample[i].value[0],
          p_session_data->is_gyro_data.sample[i].value[1],
          p_session_data->is_gyro_data.sample[i].value[2]);
      }
      for(uint32_t i=0;i<p_session_data->is_accel_data.sample_len;i++){
        IDBG_LOW("p_session_data Accel samples [%d] I %llu: G %llu, (%f, %f, %f)",
          i, p_session_data->is_accel_data.frame_time,
          p_session_data->is_accel_data.sample[i].timestamp,
          p_session_data->is_accel_data.sample[i].value[0],
          p_session_data->is_accel_data.sample[i].value[1],
          p_session_data->is_accel_data.sample[i].value[2]);
      }

      IDBG_LOW("stream_type %d rolling_shutter_skew %lld exposure_time %lld",
        p_client->isp_output_dim_stream_info.stream_type,
        p_session_data->thirdpartyeis_cfg.rolling_shutter_skew,
        p_session_data->thirdpartyeis_cfg.exposure_time);

      p_meta->thirdparty_eis_cfg.exposure_time = p_session_data->thirdpartyeis_cfg.exposure_time;
      p_meta->thirdparty_eis_cfg.rolling_shutter_skew =
        p_session_data->thirdpartyeis_cfg.rolling_shutter_skew;

      if (rc != IMG_SUCCESS) {
        IDBG_ERROR("Error updating meta cfg %d rc %d", meta_type, rc);
      }
    break;
  }

  p_meta->hal_verison = p_base_mod->hal_version;

  IDBG_LOW(": X");

  return 0;
}

/**
 * Function: module_thirdpartyeis_deinit
 *
 * Description: Function used to deinit thirdpartyeis module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_thirdpartyprevieweis_deinit(mct_module_t *p_mct_mod)
{
  IDBG_MED(": E");

  module_imgbase_t *p_mod = NULL;
  img_thirdpartyeis_module_t *p_thirdpartyeis_mod = NULL;

  p_mod = p_mct_mod->module_private;
  if (p_mod && p_mod->mod_private) {
    p_thirdpartyeis_mod = p_mod->mod_private;
    pthread_mutex_destroy(&p_thirdpartyeis_mod->session_data->session_mutex);
    free(p_thirdpartyeis_mod);
  }

  module_imgbase_deinit(p_mct_mod);
  IDBG_MED(": X");
}

void module_thirdpartyvideoeis_deinit(mct_module_t *p_mct_mod)
{
  IDBG_MED(": E");

  module_imgbase_t *p_mod = NULL;
  img_thirdpartyeis_module_t *p_thirdpartyeis_mod = NULL;

  p_mod = p_mct_mod->module_private;
  if (p_mod && p_mod->mod_private) {
    p_thirdpartyeis_mod = p_mod->mod_private;
    pthread_mutex_destroy(&p_thirdpartyeis_mod->session_data->session_mutex);
    free(p_thirdpartyeis_mod);
  }

  module_imgbase_deinit(p_mct_mod);
  IDBG_MED(": X");
}

/** module_thirdpartyeis_init:
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
mct_module_t *module_thirdpartyprevieweis_init(const char *name)
{
  IDBG_LOW("%s:%d: E", __func__, __LINE__);
  char value[PROPERTY_VALUE_MAX];
  uint32_t eis_post_cpp = 0;
  property_get("persist.vendor.camera.pproc.eispostcpp", value, "0");
  eis_post_cpp  = atoi(value);

  img_thirdpartyeis_module_t *p_thirdpartyeis_mod = NULL;
  p_thirdpartyeis_mod = calloc(1, sizeof(img_thirdpartyeis_module_t));
  if (!p_thirdpartyeis_mod) {
    IDBG_ERROR("%s:%d] Failed", __func__, __LINE__);
    return NULL;
  }

  if (eis_post_cpp > 0 ) {
    g_caps_p.internal_buf_cnt = 0;
    g_caps_p.use_internal_bufs = 0;
    g_caps_p.num_release_buf = 1;
  }

  g_caps_p.internal_buf_cnt = (uint32_t)property_get_int32(
    "persist.vendor.camera.thirdparty.eis.internal_buf_cnt.preview", 8);
  IDBG_ERROR("internal_buf_cnt:%u", g_caps_p.internal_buf_cnt);

  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    p_thirdpartyeis_mod,
    &g_caps_p,
    "libmmcamera_thirdpartyeis.so",
    CAM_QTI_FEATURE_VENDOR_EIS,
    &g_params);
}
mct_module_t *module_thirdpartyvideoeis_init(const char *name)
{
  IDBG_LOW("%s:%d: E", __func__, __LINE__);

  img_thirdpartyeis_module_t *p_thirdpartyeis_mod = NULL;
  p_thirdpartyeis_mod = calloc(1, sizeof(img_thirdpartyeis_module_t));
  if (!p_thirdpartyeis_mod) {
    IDBG_ERROR("%s:%d] Failed", __func__, __LINE__);
    return NULL;
  }

  g_caps_v.internal_buf_cnt = (uint32_t)property_get_int32(
    "persist.vendor.camera.thirdparty.eis.internal_buf_cnt.video", 35);
  IDBG_ERROR("internal_buf_cnt:%u", g_caps_v.internal_buf_cnt);

  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    p_thirdpartyeis_mod,
    &g_caps_v,
    "libmmcamera_thirdpartyeis.so",
    CAM_QTI_FEATURE_VENDOR_EIS,
    &g_params);
}

/** module_thirdpartyeis_set_parent:
 *
 *  Arguments:
 *  @p_parent - parent module pointer
 *
 * Description: This function is used to set the parent pointer
 *              of the thirdpartyeis (SeeMore) module
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_thirdpartyeis_set_parent(
  mct_module_t *p_mct_mod, mct_module_t *p_parent)
{
  IDBG_LOW("%s:%d: E", __func__, __LINE__);
  return module_imgbase_set_parent(p_mct_mod, p_parent);
}

