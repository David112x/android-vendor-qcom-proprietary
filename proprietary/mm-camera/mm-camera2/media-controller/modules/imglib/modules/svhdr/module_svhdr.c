/****************************************************************************
* Copyright (c) 2016,2019 Qualcomm Technologies, Inc.                       *
* All Rights Reserved.                                                      *
* Confidential and Proprietary - Qualcomm Technologies, Inc.                *
****************************************************************************/

#include "module_imgbase.h"
#include "chromatix_common.h"
#include "chromatix_iot.h"

/**
 *  Static functions
 **/
static int32_t module_svhdr_client_created(imgbase_client_t *p_client);
static int32_t module_svhdr_client_destroy(imgbase_client_t *p_client);
static int32_t module_svhdr_client_prestreamon(imgbase_client_t * p_client,
  uint32_t identity);
static boolean module_svhdr_handle_chrx_event(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client, img_core_ops_t *p_coreops,
  boolean *is_evt_handled);
static boolean module_svhdr_handle_aec_update(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled);
static boolean module_svhdr_handle_awb_update(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled);
static boolean module_svhdr_handle_isp_output_dim(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client,  img_core_ops_t *p_coreops,
  boolean *is_evt_handled);
static int module_svhdr_handle_out_buf_done_event(imgbase_client_t *p_client,
  img_event_t *p_event);
static int module_svhdr_handle_buf_done(imgbase_client_t *p_client,
  img_event_t *p_event);
static boolean module_svhdr_handle_buf_divert(uint32_t event_identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled);
static boolean module_svhdr_handle_buf_divert_ack(uint32_t event_identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled);
boolean module_svhdr_client_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void *p_mod);
static boolean module_svhdr_handle_set_stream_cfg(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client, img_core_ops_t *p_coreops,
  boolean *is_event_handled);
static int module_svhdr_client_get_out_dim(imgbase_client_t *p_client,
  mct_stream_info_t *p_out_stream_info);
static int module_svhdr_client_set_in_dim(imgbase_client_t *p_client,
  uint32_t identity);
static int module_svhdr_client_prepare_native_bufs(
  imgbase_client_t *p_client, mct_stream_info_t *p_stream_info);
static boolean module_svhdr_client_update_map(void *data, void *user);
static int module_svhdr_client_stats_data(
  imgbase_client_t *p_client, bayerproc_prepare_info_t *p_info);
static boolean module_svhdr_client_map_buf(uint32_t event_identity,
    mct_event_module_t *p_mod_event,
    imgbase_client_t *p_client,
    img_core_ops_t *p_coreops, boolean *is_event_handled);

/** g_caps:
 *
 *  Set the capabilities for (SVHDR) module
*/
static img_caps_t g_caps = {
  .num_input = 1,
  .num_output = 1,
  .num_meta = 1,
  .internal_buf_cnt = 4,
  .use_internal_bufs = 1,
  .share_client_per_session = 0,
};

/** img_svhdr_session_data_t:
*
*   @chromatix_data: svhdr chromatix data variable
*   @sensor_output_dim: sensor output dimensions
*     information
*   @hdr_ratios: AEC hdr ratios values
*
*   Session based parameters for svhdr module
*/
typedef struct {
  modulesChromatix_t chromatix_data;
  sensor_out_info_t sensor_output_dim;
  mct_aec_video_hdr_input_t hdr_ratios;
} img_svhdr_session_data_t;

/** img_svhdrmod_priv_t:
*
*  @session_data: svhdrmod session data
*
*  svhdrmodule private structure
*/
typedef struct {
  img_svhdr_session_data_t session_data;
} img_svhdrmod_priv_t;

/** img_svhdrclient_priv_t:
*
*  @native_frame: Img native frame
*
*  svhdr module client private structure
*/
typedef struct {
  img_frame_t native_frame;
} img_svhdrclient_priv_t;

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_client_created = module_svhdr_client_created,
  .imgbase_client_destroy = module_svhdr_client_destroy,
  .imgbase_client_pre_streamon = module_svhdr_client_prestreamon,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_SET_CHROMATIX_PTR] =
     module_svhdr_handle_chrx_event,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_STATS_AEC_UPDATE] =
     module_svhdr_handle_aec_update,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_STATS_AWB_UPDATE] =
     module_svhdr_handle_awb_update,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_SET_STREAM_CONFIG] =
     module_svhdr_handle_set_stream_cfg,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_ISP_OUTPUT_DIM] =
     module_svhdr_handle_isp_output_dim,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_BUF_DIVERT] =
     module_svhdr_handle_buf_divert,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_BUF_DIVERT_ACK] =
     module_svhdr_handle_buf_divert_ack,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_OUTPUT_BUFF_LIST] =
     module_svhdr_client_map_buf,
  .imgbase_client_event_handler[QIMG_EVT_IMG_BUF_DONE] =
     module_svhdr_handle_buf_done,
  .imgbase_client_event_handler[QIMG_EVT_IMG_OUT_BUF_DONE] =
     module_svhdr_handle_out_buf_done_event,
  .exec_mode = IMG_EXECUTION_HW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .force_cache_op = FALSE,
};


/**
 * Function: module_svhdr_client_query_mod
 *
 * Description: This function is used to query the imgbase module
 * info
 *
 * Arguments:
 *   @p_mct_cap: pipeline capability
 *
 * Return values:
 *     success/failure
 *
 * Notes: none
 **/
boolean module_svhdr_client_query_mod(mct_pipeline_cap_t *p_mct_cap, void *p_mod)
{
  mct_pipeline_imaging_cap_t *imaging_cap = &p_mct_cap->imaging_cap;
  /* We have one frame delay because of storing frame in memory and
   * one frame delay because of HDR processing. */
  imaging_cap->bayer_procecessing_delay = 2;

  return TRUE;
}

/**
 * Function: module_svhdr_client_created
 *
 * Description: function called after client creation
 *
 * Arguments:
 *   @p_client - IMG_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_svhdr_client_created(imgbase_client_t *p_client)
{
  int i;
  IDBG_MED(": E");
  p_client->ion_fd = open("/dev/ion", O_RDONLY);
  p_client->before_cpp = TRUE;
  p_client->bypass_enable = FALSE;
  p_client->is_bayer =  TRUE;
  img_svhdrclient_priv_t *p_client_priv = NULL;
  p_client_priv =
    (img_svhdrclient_priv_t *)calloc(1, sizeof(img_svhdrclient_priv_t));
  if (p_client_priv == NULL) {
    IDBG_ERROR("Unable to create client priv data");
    return IMG_ERR_NO_MEMORY;
  }
  p_client->p_private_data = p_client_priv;

  p_client->output_stream_mask = 0;
  for(i = 0; i < CAM_STREAM_TYPE_MAX; i++) {
    if (CAM_STREAM_TYPE_ANALYSIS == i)
      continue;
    p_client->output_stream_mask |= 1 << i;
  }
  IDBG_INFO("output_stream_mask %x", p_client->output_stream_mask);

  p_client->streams_to_process = (1 << CAM_STREAM_TYPE_PREVIEW)|
    (1 << CAM_STREAM_TYPE_VIDEO);

  /* process all buffers */
  p_client->process_all_frames = TRUE;

  return IMG_SUCCESS;
}

/**
 * Function: module_svhdr_client_destroy
 *
 * Description: function called before client is destroyed
 *
 * Arguments:
 *   @p_client - IMG_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_svhdr_client_destroy(imgbase_client_t *p_client)
{
  IDBG_MED(": E");
  if (p_client->p_private_data != NULL) {
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
 * Function: module_svhdr_handle_chrx_event
 *
 * Description: Function to handle chromatix event.
 *
 * Arguments:
 *   @event_identity: Event identity
 *   @p_mod_event: Pointer to module event
 *   @p_client: IMG_BASE client
 *   @p_coreops: img core operations
 *   @is_event_handled: Event handled flag
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: No need of error check since base module is protecting input
arguments.
 **/
static boolean module_svhdr_handle_chrx_event(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  IDBG_LOW(": E");
  boolean ret = TRUE;
  float ratio = 8.0f;

  module_imgbase_t *p_mod = NULL;
  if (!p_client) {
    IDBG_ERROR("Invalid client pointer");
    return IMG_ERR_INVALID_INPUT;
  }
  p_mod = (module_imgbase_t *)p_client->p_mod;
  if (!p_mod) {
    IDBG_ERROR("Invalid module pointer");
    return IMG_ERR_INVALID_INPUT;
  }
  img_svhdrmod_priv_t *p_svhdrmod_priv =
    (img_svhdrmod_priv_t *)p_mod->mod_private;
  if (!p_svhdrmod_priv) {
    IDBG_ERROR("Invalid private module data");
    return IMG_ERR_INVALID_INPUT;
  }

  modulesChromatix_t *chromatix_param =
    (modulesChromatix_t*)p_mod_event->module_event_data;
  p_svhdrmod_priv->session_data.chromatix_data =
    *chromatix_param;

  /* Query this info from from the library */
  p_svhdrmod_priv->session_data.hdr_ratios.number_of_frames = 2;
  p_svhdrmod_priv->session_data.hdr_ratios.frame_exp_compensation[0]
    = 1.0f / sqrt(ratio);
  p_svhdrmod_priv->session_data.hdr_ratios.frame_exp_compensation[1]
    = sqrt(ratio);
  IDBG_LOW(": X");
  return ret;
}

/**
 * Function: module_svhdr_handle_aec_update
 *
 * Description: Function to handle aec event.
 *
 * Arguments:
 *   @event_identity: Event identity
 *   @p_mod_event: Pointer to module event
 *   @p_client: IMG_BASE client
 *   @p_coreops: img core operations
 *   @is_event_handled: Event handled flag
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: No need of error check since base module is protecting input
 *        arguments.
 **/
static boolean module_svhdr_handle_aec_update(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  IDBG_LOW(": E");
  int rc = IMG_SUCCESS;
  boolean ret = TRUE;
  aec_update_t* aec_update;
  img_aec_info_t aec_info;
  stats_update_t *stats_update;

  stats_update = p_mod_event->module_event_data;
  if (stats_update) {
    aec_update = &stats_update->aec_update;
    aec_info.real_gain = aec_update->real_gain;
    aec_info.linecount = aec_update->linecount;
    aec_info.exp_time = aec_update->exp_time;
    aec_info.lux_idx = aec_update->lux_idx;
    aec_info.iso = aec_update->exif_iso;
    aec_info.hdr_gtm_gamma = aec_update->vhdr_update.hdr_gtm_gamma;
    aec_info.hdr_exp_time_ratio = aec_update->vhdr_update.hdr_exp_time_ratio;
  }

  rc = img_set_meta(&p_client->current_meta, IMG_META_AEC_INFO, &aec_info);
  if(IMG_SUCCEEDED(rc)) {
    *is_event_handled = TRUE;
  } else {
    *is_event_handled = FALSE;
  }
  IDBG_LOW(": X");
  return ret;
}

/**
 * Function: module_svhdr_handle_awb_update
 *
 * Description: Function to handle awb event.
 *
 * Arguments:
 *   @event_identity: Event identity
 *   @p_mod_event: Pointer to module event
 *   @p_client: IMG_BASE client
 *   @p_coreops: img core operations
 *   @is_event_handled: Event handled flag
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: No need of error check since base module is protecting input
 *        arguments.
 **/
static boolean module_svhdr_handle_awb_update(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  IDBG_LOW(": E");
  int rc = IMG_SUCCESS;
  boolean ret = TRUE;
  awb_update_t *p_awb_update;
  img_awb_info_t awb_info;
  stats_update_t *p_stats_update;

  p_stats_update = p_mod_event->module_event_data;
  if (p_stats_update) {
    p_awb_update = &p_stats_update->awb_update;
    awb_info.r_gain = p_awb_update->gain.r_gain;
    awb_info.b_gain = p_awb_update->gain.b_gain;
  }

  rc = img_set_meta(&p_client->current_meta, IMG_META_AWB_INFO, &awb_info);
  if(IMG_SUCCEEDED(rc)) {
    *is_event_handled = TRUE;
  } else {
    *is_event_handled = FALSE;
  }
  IDBG_LOW(": X");
  return ret;
}

/**
 * Function: module_svhdr_handle_buf_divert
 *
 * Description: Function to handle buf divert event.
 *
 * Arguments:
 *   @event_identity: Event identity
 *   @p_mod_event: Pointer to module event
 *   @p_client: IMG_BASE client
 *   @p_coreops: img core operations
 *   @is_event_handled: Event handled flag
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: No need of error check since base module is protecting input
 *        arguments.
 **/
static boolean module_svhdr_handle_buf_divert(uint32_t event_identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  IDBG_LOW(": E");

  isp_buf_divert_t *p_buf_divert =
    (isp_buf_divert_t *)p_mod_event->module_event_data;
  IDBG_LOW("MCT_EVENT_MODULE_BUF_DIVERT %x", event_identity);

  /* If this is a bayer buf divert, handle the event by setting
     is_evt_handled to FALSE
     and  return FALSE to avaoid sending this event upstream */
  if(p_buf_divert->bayerdata == TRUE) {
    *is_event_handled = FALSE;
    return FALSE;
  } else {
  /* If it is a yuv divert, send it to downstream by returning TRUE */
    *is_event_handled = TRUE;
    return TRUE;
  }
  IDBG_LOW(": X");
}

/**
 * Function: module_svhdr_handle_buf_divert_ack
 *
 * Description: Function to handle buf divert ack event.
 *
 * Arguments:
 *   @event_identity: Event identity
 *   @p_mod_event: Pointer to module event
 *   @p_client: IMG_BASE client
 *   @p_coreops: img core operations
 *   @is_event_handled: Event handled flag
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: No need of error check since base module is protecting input
 *        arguments.
 **/
static boolean module_svhdr_handle_buf_divert_ack(uint32_t event_identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  IDBG_LOW(": E");

  isp_buf_divert_ack_t *p_buf_divert_ack =
    (isp_buf_divert_ack_t *)p_mod_event->module_event_data;
  IDBG_LOW("MCT_EVENT_MODULE_BUF_DIVERT_ACK %x bayer = %d", event_identity,
    p_buf_divert_ack->bayerdata);

  /* If this is a bayer buf divert, handle the event by setting
     is_evt_handled to FALSE
     and  return FALSE to avoid sending this event upstream */
  if(p_buf_divert_ack->bayerdata == TRUE) {
    *is_event_handled = FALSE;
    return FALSE;
  } else {
  /* If it is a yuv divert, send it to downstream by returning TRUE */
    *is_event_handled = TRUE;
    return TRUE;
  }
}


/**
 * Function: module_svhdr_client_prestreamon
 *
 * Description: function called before streamon
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *   @identity: input identity
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int32_t module_svhdr_client_prestreamon(imgbase_client_t * p_client,
  uint32_t identity)
{
  int rc = IMG_SUCCESS;
  imgbase_stream_t *p_stream = NULL;
  int stream_idx = 0;

  if (!p_client) {
    IDBG_ERROR("Invalid client handle %p", p_client);
    rc = IMG_ERR_INVALID_INPUT;
    goto error;
  }

  stream_idx = module_imgbase_find_stream_by_identity(p_client, identity);
  if (stream_idx < 0) {
    IDBG_ERROR("Cannot find stream mapped to idx %x",
      p_client->divert_identity);
    rc = IMG_ERR_INVALID_INPUT;
    goto error;
  }

  p_stream = &p_client->stream[stream_idx];

  // Internal buffers for svhdr module are allocated on receiving
  // MCT_EVENT_MODULE_OUTPUT_BUFF_LIST event, hence disable
  // internal buffers allocation during client start or streamon
  p_client->defer_intbuff_alloc = TRUE;

  IDBG_LOW("stream_type %d defer_intbuff_alloc %d",
    p_stream->stream_info->stream_type,
    p_client->defer_intbuff_alloc);

error:
  return rc;
}

/**
 * Function: module_svhdr_handle_isp_output_dim
 *
 * Description: Function to handle isp output dim event.
 *
 * Arguments:
 *   @event_identity: Event identity
 *   @p_mod_event: Pointer to module event
 *   @p_client: IMG_BASE client
 *   @p_coreops: img core operations
 *   @is_event_handled: Event handled flag
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: No need of error check since base module is protecting input
 *        arguments.
 **/
static boolean module_svhdr_handle_isp_output_dim(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  IDBG_HIGH(": E %x", event_identity);
  uint32_t output_identity = 0, rc = 0;
  mct_stream_info_t *stream_info =
    (mct_stream_info_t *)(p_mod_event->module_event_data);
  if(!stream_info) {
    IDBG_ERROR("Invalid stream info");
    return FALSE;
  }

  /* ISP sends two events with info from RDI stream
   and CAMIF and distinguished between them
   using bayerdata flag. */
  if(stream_info->bayerdata == 0) {
    IDBG_MED("Isp out dim evt bayerdata = 0, fwd'ing it without handling");
    return TRUE;
  }

  if (stream_info) {
    pthread_mutex_lock(&p_client->mutex);

    if(stream_info->bayerdata) {
      p_client->isp_output_dim_stream_info = *stream_info;
      p_client->isp_output_dim_stream_info_valid = TRUE;
    }
    pthread_mutex_unlock(&p_client->mutex);

    IDBG_HIGH("MCT_EVENT_MODULE_ISP_OUTPUT_DIM %dX%d fmt %d bayerdata %d",
      stream_info->dim.width,
      stream_info->dim.height,
      stream_info->fmt,
      stream_info->bayerdata);

    if ((p_client->bypass_enable == FALSE) &&
        (stream_info->bayerdata == TRUE)) {
        output_identity = p_client->output_identity;
      IDBG_HIGH("output_identity %x", __func__, __LINE__, output_identity);
      rc = module_svhdr_client_set_in_dim(p_client, output_identity);
      if (IMG_ERROR(rc)) {
        IDBG_HIGH("Fail to prepary buffs for 0x%x", event_identity);
      }
    }
  }
  *is_event_handled = 1;

  /* Do not forward event downstream as this is bayer dim info*/
  rc = FALSE;

  IDBG_HIGH(": X");
  return rc;
}

/**
 * Function: module_svhdr_handle_out_buf_done_event
 *
 * Description: Function to handle output buf done event
 *
 * Arguments:
 *   @p_client - IMG_BASE client
 *   @p_event  - Img event
 *
 * Return values:
 *   IMG_ERROR type
 *
 **/
int module_svhdr_handle_out_buf_done_event(imgbase_client_t *p_client,
  img_event_t *p_event)
{
  int rc = IMG_ERR_EAGAIN;
  int stream_idx = 0;
  uint32_t identity, frame_id;
  imgbase_stream_t *p_stream = NULL;
  imgbase_buf_t *p_imgbase_buf = NULL;
  img_frame_t *p_frame = NULL;
  isp_buf_divert_t *p_buf_divert = NULL, img_buf_divert;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  p_frame = p_event->d.p_frame;
  p_imgbase_buf = (imgbase_buf_t *)p_frame->private_data;

  IDBG_HIGH("buf_done for frame id %d", p_frame->frame_id);
  stream_idx = module_imgbase_find_stream_by_identity(p_client,
    p_client->divert_identity);
  if (stream_idx < 0) {
    IDBG_ERROR("[%s] Cannot find stream mapped to idx %x",p_mod->name,
      p_client->divert_identity);
    goto end;
  }

  p_stream = &p_client->stream[stream_idx];

  p_buf_divert = &p_imgbase_buf->buf_divert;
  identity = p_imgbase_buf->event_identity ? p_imgbase_buf->event_identity :
    p_buf_divert->identity;
  IDBG_MED("[%s] identity %x E", p_mod->name, identity);

  if (p_stream->p_srcport || p_stream->p_sinkport) {
    /* If src port mapped, send op buffer downstream via buf divert */
    frame_id = p_buf_divert->buffer.sequence;

    IDBG_MED("[%s] [IMG_BUF_DBG_OUT_f%d_b%d_i%x] send buf_div timestamp %llu",
      p_mod->name, frame_id, p_frame->idx, identity,
      p_imgbase_buf->buf_divert.buffer.timestamp.tv_sec
      * 1000000LL + p_imgbase_buf->buf_divert.buffer.timestamp.tv_usec);

    /* Always send buffer divert when native output is used */
    if(p_mod->caps.use_internal_bufs) {
      IDBG_ERROR("Sending the buffer divert to upstream iface since native");
      /* Fill buffer divert from frame */
      memset(&img_buf_divert, 0x00, sizeof(img_buf_divert));
      img_buf_divert.fd = p_frame->frame[0].plane[0].fd;
      img_buf_divert.vaddr = p_frame->frame[0].plane[0].addr;
      img_buf_divert.buffer.sequence = p_frame->frame_id;
      img_buf_divert.buffer.index = p_frame->idx;
      img_buf_divert.native_buf = TRUE;
      img_buf_divert.identity = p_client->divert_identity;
      img_buf_divert.channel_id = 1;
      img_buf_divert.buffer_access = p_buf_divert->buffer_access;

      if (p_stream->p_sinkport) {
        rc = mod_imgbase_send_event(p_client->divert_identity,
          TRUE, MCT_EVENT_MODULE_BUF_DIVERT, img_buf_divert);
        if (rc == FALSE) {
          rc = module_imgbase_client_handle_buffer_ack(p_client,
            IMG_EVT_ACK_FREE_INTERNAL_BUF, &img_buf_divert);
          IDBG_ERROR("%s] Error sending output buffer %d",
            p_mod->name, p_frame->idx);
          goto end;
        }
        /* free in task */
        p_frame = NULL;
        return rc;
      }
    }
  }
  return rc;

  end:
  return rc;
}

/**
 * Function: module_svhdr_client_handle_buf_done
 *
 * Description: Function to handle input buf done event
 *
 * Arguments:
 *   @p_client - IMG_BASE client
 *   @p_event  - Frameproc event structure
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
int module_svhdr_handle_buf_done(imgbase_client_t *p_client,
  img_event_t *p_event)
{
  img_frame_t *p_frame = NULL;
  int rc = IMG_ERR_EAGAIN;
  isp_buf_divert_t *p_buf_divert;
  imgbase_buf_t *p_imgbase_buf = NULL;
  module_imgbase_t *p_mod;
  imgbase_stream_t *p_stream = NULL;
  int32_t free_frame = TRUE;
  int stream_idx;

  p_frame =   p_event->d.p_frame;
  if (!p_client || !p_frame) {
    IDBG_ERROR("Error");
    return rc;
  }

  p_mod = (module_imgbase_t *)p_client->p_mod;
  if (!p_mod) {
    IDBG_ERROR("Error");
    return rc;
  }

  stream_idx = module_imgbase_find_stream_by_identity(p_client,
    p_client->divert_identity);
  if (stream_idx < 0) {
    IDBG_ERROR(":%s] Cannot find stream mapped to idx %x",
      p_mod->name, p_client->divert_identity);
    goto end;
  }

  p_stream = &p_client->stream[stream_idx];
  if (NULL == p_stream) {
    IDBG_ERROR(":%s] Cannot find stream mapped to client %d",
      p_mod->name, p_client->stream_cnt);
    goto end;
  }
  p_imgbase_buf = (imgbase_buf_t *)p_frame->private_data;
  if (!p_imgbase_buf) {
    IDBG_HIGH("imgbasebuf NULL, dummy frame");
    goto end;
  }
  p_buf_divert = &p_imgbase_buf->buf_divert;
  IDBG_MED("buffer idx %d port %p %p frame_id %d",
    p_frame->idx, p_stream->p_sinkport, p_stream->p_srcport,
    p_buf_divert->buffer.sequence);

   if (p_stream->p_sinkport) {
    if (p_mod->caps.num_output || p_mod->caps.ack_required) {
      /* Generate ACK */
      rc = module_imgbase_client_handle_buffer_ack(p_client,
        IMG_EVT_ACK_FORCE_RELEASE, p_buf_divert);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("%s:%d] Error in sending ack", __func__, __LINE__);
      }
    } else {
      rc = module_imglib_common_release_buffer(p_mod->subdevfd,
        p_client->divert_identity,
        p_frame->idx,
        p_frame->frame_id,
        TRUE,
        p_frame,
        p_client->ion_fd,
        p_buf_divert->buffer_access);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("Error in releasing buffer");
      }
    }
  }
  return rc;

end:
  /* if overlap batch processing, this is freed in handle_overlap_input */
  if (!p_mod->caps.num_overlap && p_imgbase_buf && free_frame) {
    free(p_imgbase_buf);
    p_imgbase_buf = NULL;
  }
  return rc;
}

/**
 * Function: module_svhdr_handle_set_stream_cfg
 *
 * Description: Function to handle set stream cfg event.
 *
 * Arguments:
 *   @event_identity: Event identity
 *   @p_mod_event: Pointer to module event
 *   @p_client: IMG_BASE client
 *   @p_coreops: img core operations
 *   @is_event_handled: Event handled flag
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: No need of error check since base module is protecting input
 *        arguments.
 **/
static boolean module_svhdr_handle_set_stream_cfg(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  IDBG_HIGH(": E");
  boolean ret = TRUE;
  module_imgbase_t *p_mod = NULL;
  sensor_out_info_t *p_sensor_out_dim = NULL;

  p_mod = (module_imgbase_t *)p_client->p_mod;
  if (NULL == p_mod || p_mod_event == NULL) {
    IDBG_ERROR("imgbase module or module evt payload NULL");
    *is_event_handled = FALSE;
    return FALSE;
  }
  img_svhdrmod_priv_t *p_svhdrmod_priv =
    (img_svhdrmod_priv_t *)p_mod->mod_private;
  if (p_svhdrmod_priv == NULL) {
    IDBG_ERROR("NULL private module data pointer");
    *is_event_handled = FALSE;
    return FALSE;
  }
  sensor_out_info_t *p_output_info =
    (sensor_out_info_t *)p_mod_event->module_event_data;
  p_sensor_out_dim =
    &p_svhdrmod_priv->session_data.sensor_output_dim;

  p_sensor_out_dim->dim_output =
    p_output_info->dim_output;
  p_sensor_out_dim->custom_format =
    p_output_info->custom_format;
  p_sensor_out_dim->custom_format.enable =
    p_output_info->custom_format.enable;
  p_sensor_out_dim->custom_format.width =
    p_output_info->dim_output.width;
  p_sensor_out_dim->custom_format.height =
    p_output_info->dim_output.height;
  p_sensor_out_dim->custom_format.start_x =
    p_output_info->custom_format.start_x;
  p_sensor_out_dim->custom_format.start_y =
    p_output_info->custom_format.start_y;
  p_sensor_out_dim->custom_format.subframes_cnt =
    p_output_info->custom_format.subframes_cnt;
  IDBG_INFO("custom_format.enable %d custom_format.subframes_cnt %d",
    p_output_info->custom_format.enable,
    p_output_info->custom_format.subframes_cnt);
  IDBG_INFO("start_y %d start_x %d", p_output_info->custom_format.start_y,
    p_output_info->custom_format.start_x);
  IDBG_INFO("custom fmt W x H %d x %d",
    p_output_info->custom_format.width,
    p_output_info->custom_format.height);
  IDBG_INFO("dim_output W x H %d x %d",
    p_output_info->dim_output.width,
    p_output_info->dim_output.height);
  *is_event_handled = TRUE;

  return ret;
}


/**
 * Function: module_svhdr_deinit
 *
 * Description: Function used to deinit svhdr module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_svhdr_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_t *p_mod = NULL;
  img_svhdrmod_priv_t *p_svhdrmod_data = NULL;
  if(!p_mct_mod) {
    IDBG_ERROR("Invalid module %p", p_mct_mod);
    return;
  }
  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (!p_mod) {
    IDBG_ERROR("Invalid base module %p", p_mod);
    return;
  }
  p_svhdrmod_data = p_mod->mod_private;
  if (p_svhdrmod_data) {
    free(p_svhdrmod_data);
  }
  module_imgbase_deinit(p_mct_mod);
}

/** module_svhdr_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: Function used to initialize the SVHDR module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_svhdr_init(const char *name)
{
  img_svhdrmod_priv_t *p_svhdr_priv_data = NULL;
  p_svhdr_priv_data = calloc(1, sizeof(img_svhdrmod_priv_t));

  if(p_svhdr_priv_data == NULL) {
    IDBG_ERROR("Unable allocate module private data");
    return NULL;
  }
  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    p_svhdr_priv_data,
    &g_caps,
    "libmmcamera_bp_svhdr.so",
    BAYERPROC_FEATURE_SVHDR,
    &g_params);
}

/**
 * Function: module_svhdr_client_set_in_dim
 *
 * Description: This function is set bayerproc input dimension.
 *
 * Arguments:
 *   @p_client: IMG_BASE client
 *   @identity: Stream identity
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_svhdr_client_set_in_dim(imgbase_client_t *p_client,
  uint32_t identity)
{
  imgbase_stream_t *p_stream;
  mct_stream_info_t out_stream_info;
  boolean is_upstream, ret_event;
  int stream_idx, rc;

  if (!p_client) {
    IDBG_ERROR("Invalid input");
    return IMG_ERR_INVALID_INPUT;
  }
  pthread_mutex_lock(&p_client->mutex);

  rc = module_svhdr_client_get_out_dim(p_client, &out_stream_info);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Cannot get output dim");
    pthread_mutex_unlock(&p_client->mutex);
    return IMG_ERR_GENERAL;
  }
  memcpy(&p_client->out_stream_info,
    &out_stream_info, sizeof(mct_stream_info_t));

  stream_idx = module_imgbase_find_stream_by_identity(p_client, identity);
  if (stream_idx < 0) {
    IDBG_ERROR("Cannot find stream mapped to idx %x", identity);
    pthread_mutex_unlock(&p_client->mutex);
    return IMG_ERR_GENERAL;
  }
  p_stream = &p_client->stream[stream_idx];
  pthread_mutex_unlock(&p_client->mutex);
  IDBG_INFO("sending MCT_EVENT_MODULE_BAYERPROC_OUTPUT_DIM evt to IFACE");

  is_upstream = TRUE;
  ret_event = mod_imgbase_send_event(identity, is_upstream,
    MCT_EVENT_MODULE_BAYERPROC_OUTPUT_DIM, out_stream_info);
  if (ret_event == FALSE) {
    IDBG_ERROR("Fail to send bayeproc event %d", is_upstream);
    return IMG_ERR_GENERAL;
  }

  return IMG_SUCCESS;
}

/**
 * Function: module_svhdr_client_get_out_dim
 *
 * Description: This function is used to get the output dims.
 *
 * Arguments:
 *   @p_client: IMG_BASE client
 *   @p_native_stream_info: Pointer to stream info filled with output dims.
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_svhdr_client_get_out_dim(imgbase_client_t *p_client,
  mct_stream_info_t *p_out_stream_info)
{
  mct_stream_info_t *p_stream_info;
  img_component_ops_t *p_comp;
  img_format_t fmt;
  bayerproc_in_out_frame_info_t info;
  uint32_t buffer_size, stride_bytes, stride_pixels;
  uint32_t scanline, width, height;
  int rc;
  img_lib_config_t lib_config;
  module_imgbase_t *p_mod = NULL;

  if (!p_client || !p_out_stream_info) {
    IDBG_ERROR("Invalid input");
    return IMG_ERR_INVALID_INPUT;
  }
  p_comp = &p_client->comp;

  p_mod = (module_imgbase_t *)p_client->p_mod;
  if (!p_mod) {
    IDBG_ERROR("Invalid module pointer");
    return IMG_ERR_INVALID_INPUT;
  }
  img_svhdrmod_priv_t *p_svhdrmod_priv =
    (img_svhdrmod_priv_t *)p_mod->mod_private;
  if (!p_svhdrmod_priv) {
    IDBG_ERROR("Invalid private module data");
    return IMG_ERR_INVALID_INPUT;
  }
  sensor_out_info_t *out_dim =
    &p_svhdrmod_priv->session_data.sensor_output_dim;
  if (!out_dim) {
    IDBG_ERROR("Invalid sensor data pointer");
    return IMG_ERR_INVALID_INPUT;
  }

  if (FALSE == p_client->isp_output_dim_stream_info_valid) {
    IDBG_ERROR("Isp output dim not available can not allocate buffers");
    return IMG_ERR_GENERAL;
  }
  p_stream_info = &p_client->isp_output_dim_stream_info;

  rc = module_imglib_common_get_frame_fmt(p_stream_info->fmt, &fmt);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Fail frame format");
    return IMG_ERR_INVALID_OPERATION;
  }
  IDBG_LOW("Got frame fmt %d", fmt);

  memset(&info, 0x00, sizeof(info));
  info.in_info.fmt = fmt;
  info.in_info.width = out_dim->dim_output.width;
  info.in_info.height = out_dim->dim_output.height;
  info.in_info.custom_format.enable =
    out_dim->custom_format.enable;
  info.in_info.custom_format.subframes_cnt =
    out_dim->custom_format.subframes_cnt;
  info.in_info.custom_format.start_x =
    out_dim->custom_format.start_x;
  info.in_info.custom_format.start_y =
    out_dim->custom_format.start_y;
  info.in_info.custom_format.width =
    out_dim->custom_format.width;
  info.in_info.custom_format.height =
    out_dim->custom_format.height;

  info.out_info = info.in_info;
  info.out_info.analysis = info.in_info.analysis;
  info.out_info.client_id = info.in_info.client_id;
  info.out_info.custom_format = info.in_info.custom_format;
  info.out_info.fmt = info.in_info.fmt;
  info.out_info.height = info.in_info.height;
  info.out_info.width = info.in_info.width;
  info.out_info.stride = info.in_info.stride;
  info.out_info.num_planes = info.in_info.num_planes;
  info.out_info.ss = info.in_info.ss;

  lib_config.lib_data = &info;
  lib_config.lib_param = IMG_ALGO_OUT_FRAME_DIM;

  /* Algo will update output params based on input fmt */
  rc = IMG_COMP_GET_PARAM(p_comp, QIMG_PARAM_LIB_CONFIG,
    &lib_config);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Fail output frame info");
    return IMG_ERR_GENERAL;
  }
  width = info.out_info.width;
  height = info.out_info.height;
  scanline = info.out_info.height;
  stride_bytes = info.out_info.stride;

  if (!stride_bytes) {
    IDBG_ERROR("info.out_info.fmt %d width %d",info.out_info.fmt, width);
    stride_bytes = module_imglib_common_get_stride_in_bytes(width,
      info.out_info.fmt);
    if (!stride_bytes) {
      IDBG_ERROR("Can not get stride in bytes");
      return IMG_ERR_GENERAL;
    }
  }
  IDBG_LOW("stride_bytes %d scanline %d", stride_bytes, scanline);

  stride_pixels = module_imglib_common_get_stride_in_pix(stride_bytes,
    info.out_info.fmt);
  if (!stride_pixels) {
    IDBG_ERROR("Can not get stride in pixels");
    return IMG_ERR_GENERAL;
  }

  width = info.out_info.width;
  height = info.out_info.height;
  scanline = info.out_info.height;
  buffer_size = stride_bytes * scanline;
  IDBG_LOW("W %d H %d Stride %d Scanline %d StridInPix %d StridInByt %d",
    width, height, scanline, stride_bytes, stride_pixels);
  IDBG_LOW("out_info fmt %d ", info.out_info.fmt);

  /* Initialize all fields with isp stream info first */
  *p_out_stream_info = *p_stream_info;

  /* If library format is different convert to cam type */
  if (info.out_info.fmt != info.in_info.fmt) {
    rc = module_imglib_common_get_cam_fmt(info.out_info.fmt,
      &p_out_stream_info->fmt);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Cam format unsupported");
      return IMG_ERR_NO_MEMORY;
    }
  }
  p_out_stream_info->dim.width = width;
  p_out_stream_info->dim.height = height;
  p_out_stream_info->num_bufs = 0;
  p_out_stream_info->img_buffer_list = NULL;
  p_out_stream_info->buf_planes.plane_info.num_planes = 1;
  p_out_stream_info->buf_planes.plane_info.frame_len = buffer_size;
  p_out_stream_info->buf_planes.plane_info.mp[0].len = buffer_size;
  p_out_stream_info->buf_planes.plane_info.mp[0].width = width;
  p_out_stream_info->buf_planes.plane_info.mp[0].height = height;
  p_out_stream_info->buf_planes.plane_info.mp[0].stride =
    stride_pixels;
  p_out_stream_info->buf_planes.plane_info.mp[0].stride_in_bytes =
    stride_bytes;
  p_out_stream_info->buf_planes.plane_info.mp[0].scanline = scanline;

  return IMG_SUCCESS;
}

/**
 * Function: module_svhdr_client_prepare_native_bufs
 *
 * Description: This function is used to allocate bayerproc native buffers.
 *
 * Arguments:
 *   @p_client: IMG_BASE client
 *   @p_stream_info: Pointer to stream info containing native buffer dimensions.
 *     It can be NULL if stream info is not needed.
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_svhdr_client_prepare_native_bufs(
  imgbase_client_t *p_client, mct_stream_info_t *p_stream_info)
{
  uint32_t buffer_size, stride_bytes, scanline, out_id, stream_idx;
  img_frame_info_t frame_info;
  module_imgbase_t *p_mod = NULL;
  img_svhdrclient_priv_t *p_client_priv = NULL;
  int rc;

  /* W,H will be saved in priv data of client so it should be valid */
  if (!p_client || !p_stream_info || !p_client->p_private_data) {
    IDBG_ERROR("Invalid input");
    return IMG_ERR_INVALID_INPUT;
  }
  p_mod = (module_imgbase_t *)p_client->p_mod;
  if (p_mod == NULL) {
    IDBG_ERROR("Invalid module pointer");
    return IMG_ERR_INVALID_INPUT;
  }
  if (p_mod->caps.use_internal_bufs == 0) {
    IDBG_ERROR("No need of internal buffers");
    return IMG_SUCCESS;
  }
  p_client_priv = (img_svhdrclient_priv_t *)p_client->p_private_data;

  memset(&frame_info, 0x00, sizeof(frame_info));
  frame_info.client_id = -1;
  frame_info.width = p_stream_info->dim.width;
  frame_info.height = p_stream_info->dim.height;
  IDBG_INFO("p_stream_info->dim.width %d", p_stream_info->dim.width);
  IDBG_INFO("p_stream_info->dim.height %d", frame_info.height);

  rc = module_imglib_common_get_frame_fmt(p_stream_info->fmt,
    &frame_info.fmt);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Cam format unsupported");
    return IMG_ERR_NO_MEMORY;
  }
  scanline = p_stream_info->buf_planes.plane_info.mp[0].scanline;
  stride_bytes = p_stream_info->buf_planes.plane_info.mp[0].stride_in_bytes;
  buffer_size = stride_bytes * scanline;

  /* If any buffer already allocated free them first */
  out_id = p_client->output_identity;
  stream_idx = module_imgbase_find_stream_by_identity(p_client, out_id);
  IDBG_LOW("out_id %x", out_id);

  rc = module_imgbase_client_alloc_int_buf(p_client, out_id);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Unable to allocate internal buffers");
    return IMG_ERR_NO_MEMORY;
  }
  p_client_priv->native_frame.frame[0].plane[0].width = frame_info.width;
  p_client_priv->native_frame.frame[0].plane[0].height = frame_info.height;
  rc = module_imgbase_client_pack_buffer(p_client, stride_bytes, scanline);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Unable to pack buffers");
    return IMG_ERR_NO_MEMORY;
  }

  return IMG_SUCCESS;
}

/**
 * Function: module_svhdr_client_update_map
 *
 * Description: Callback function used in list traverse
 * to map buffers from stream info.
 *
 * Input parameters:
 *   @data - MCT stream buffer mapping
 *   @user - Bayerproc frame map structure
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_svhdr_client_update_map(void *data, void *user)
{
  mct_stream_map_buf_t *p_buf = (mct_stream_map_buf_t *)data;
  bayerproc_frame_map_t *p_frame_map = (bayerproc_frame_map_t *)user;
  uint32_t i;

  if (!p_buf || !p_frame_map) {
    IDBG_ERROR("failed");
    return FALSE;
  }

  if (p_buf->buf_type != CAM_MAPPING_BUF_TYPE_STREAM_BUF) {
    return TRUE;
  }

  if (p_frame_map->frame_cnt >= BAYERPROC_MAX_USED_BUFS) {
    return FALSE;
  }

  p_frame_map->frames[p_frame_map->frame_cnt].plane_cnt =  p_buf->num_planes;
  for (i = 0; i < p_buf->num_planes; i++) {
    p_frame_map->frames[p_frame_map->frame_cnt].plane[i].addr =
      p_buf->buf_planes[i].buf;
    p_frame_map->frames[p_frame_map->frame_cnt].plane[i].fd =
      p_buf->buf_planes[i].fd;
    p_frame_map->frames[p_frame_map->frame_cnt].plane[i].stride =
      p_buf->buf_planes[i].stride;
    p_frame_map->frames[p_frame_map->frame_cnt].plane[i].length =
      p_buf->buf_planes[i].size;
    p_frame_map->frames[p_frame_map->frame_cnt].plane[i].offset =
      p_buf->buf_planes[i].offset;
  }
  p_frame_map->frame_cnt++;
  return TRUE;
}

/**
 * Function: module_svhdr_client_map_buf
 *
 * Description: Function to handle OUTPUT_BUFF_LIST event
 *
 * Arguments:
 *   @event_identity: Event identity
 *   @p_mod_event: Pointer to module event
 *   @p_client: IMG_BASE client
 *   @p_coreops: img core operations
 *   @is_event_handled: Event handled flag
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: No need of error check since base module is protecting input
 *        arguments.
 **/
static boolean module_svhdr_client_map_buf(uint32_t event_identity,
    mct_event_module_t *p_mod_event,
    imgbase_client_t *p_client,
    img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  img_component_ops_t *p_comp;
  mct_stream_info_t *p_in_stream;
  bayerproc_prepare_info_t prepare_info;
  img_lib_config_t lib_config;
  uint32_t i, j, out_id;
  int stream_idx;
  int rc = IMG_ERR_GENERAL;
  imgbase_stream_t *p_stream = NULL;
  chromatix_VFE_common_type *pchromatix_common = NULL;
  chromatix_L_type          *pchromatix_L = NULL;
  int shift_bits = 0;
  module_imgbase_t *p_mod = NULL;
  mct_list_t *p_img_buffer_list = NULL;
  modulesChromatix_t *p_chromatix =  NULL;
  p_img_buffer_list = (mct_list_t *)p_mod_event->module_event_data;
  img_svhdrclient_priv_t *p_client_priv = NULL;
  IDBG_ERROR("event_identity %x", event_identity);

  if (!p_client || !p_img_buffer_list) {
    *is_event_handled = FALSE;
    IDBG_ERROR("Invalid input");
    return FALSE;
  }
  p_comp = &p_client->comp;

  p_mod = (module_imgbase_t *)p_client->p_mod;
  img_svhdrmod_priv_t *p_svhdrmod_priv =
    (img_svhdrmod_priv_t *)p_mod->mod_private;
  p_client_priv =
    (img_svhdrclient_priv_t *)p_client->p_private_data;
  if (!p_svhdrmod_priv || !p_client_priv) {
    IDBG_ERROR("Invalid private module or client pointer");
    return IMG_ERR_INVALID_INPUT;
  }

  sensor_out_info_t *out_dim =
    &p_svhdrmod_priv->session_data.sensor_output_dim;
  p_chromatix = &p_svhdrmod_priv->session_data.chromatix_data;
  if(out_dim == NULL || p_chromatix == NULL) {
    IDBG_ERROR("NULL sensor or chromatix pointer");
    return IMG_ERR_INVALID_INPUT;
  }
  pthread_mutex_lock(&p_client->mutex);

  out_id = p_client->output_identity;
  stream_idx = module_imgbase_find_stream_by_identity(p_client, out_id);
  IDBG_LOW("out_id %x", out_id);

  /* over write out_id with actual prev stream identity */
  IDBG_LOW("stream_idx of preview %x", stream_idx);
  if (stream_idx < 0) {
    IDBG_ERROR("Cannot find stream mapped to idx %x", out_id);
    rc = IMG_ERR_INVALID_OPERATION;
    goto out_and_unlock;
  }
  p_stream = &p_client->stream[stream_idx];
  p_client->is_dim_change =  TRUE; /* Output dimension has changed */

  memcpy(p_stream->stream_info, &p_client->out_stream_info,
    sizeof(mct_stream_info_t));

  rc = module_svhdr_client_prepare_native_bufs(p_client,
    &p_client->out_stream_info);

  p_client->event_identity = event_identity;
  p_in_stream = NULL;
  if (p_client->isp_output_dim_stream_info_valid) {
    p_in_stream = &p_client->isp_output_dim_stream_info;
    p_in_stream->buf_planes.plane_info.mp[0].stride =
      out_dim->custom_format.width;
  }

  memset(&prepare_info, 0x00, sizeof(prepare_info));
  if (p_in_stream) {
    img_format_t fmt;
    uint32_t stride_bytes;

    prepare_info.in_buf_map.frame_cnt = 0;
    rc = module_imglib_common_get_frame_fmt(p_in_stream->fmt, &fmt);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Fail frame format");
      rc = IMG_ERR_INVALID_OPERATION;
      goto out_and_unlock;
    }

    stride_bytes = module_imglib_common_get_stride_in_bytes(
      p_in_stream->dim.width, fmt);
    if (!stride_bytes) {
      IDBG_ERROR("Can not get stride in bytes");
      rc = IMG_ERR_INVALID_OPERATION;
      goto out_and_unlock;
    }
    IDBG_INFO("Got stride in bytes %d", stride_bytes);
    mct_list_traverse(p_img_buffer_list,
      module_svhdr_client_update_map, &prepare_info.in_buf_map);

    /* Width and height are not part of map info we need to fill them here */
    for (i = 0; i < prepare_info.in_buf_map.frame_cnt; i++) {
      for (j = 0; j < prepare_info.in_buf_map.frames[i].plane_cnt; j++) {
        prepare_info.in_buf_map.frames[i].plane[j].width =
          p_in_stream->dim.width;
        prepare_info.in_buf_map.frames[i].plane[j].height =
          p_in_stream->dim.height;
        prepare_info.in_buf_map.frames[i].plane[j].stride = stride_bytes;
      }
      IDBG_LOW("buffer_%d plane_cnt %d width %d height %d stride %d", i,
        prepare_info.in_buf_map.frames[i].plane_cnt,
        prepare_info.in_buf_map.frames[i].plane[0].width,
        prepare_info.in_buf_map.frames[i].plane[0].height,
        prepare_info.in_buf_map.frames[i].plane[0].stride);
    }
  }
  IDBG_INFO("Mapped input buf list");

  if (p_client->native_buffer_list) {
    prepare_info.out_buf_map.frame_cnt = 0;
    mct_list_traverse(p_client->native_buffer_list,
      module_svhdr_client_update_map, &prepare_info.out_buf_map);
    /* Width and height are not part of map info we need to fill them here */
    for (i = 0; i < prepare_info.out_buf_map.frame_cnt; i++) {
      for (j = 0; j < prepare_info.out_buf_map.frames[i].plane_cnt; j++) {
        prepare_info.out_buf_map.frames[i].plane[j].width =
          p_client_priv->native_frame.frame[0].plane[0].width;
        prepare_info.out_buf_map.frames[i].plane[j].height =
          p_client_priv->native_frame.frame[0].plane[0].height;
      }
      IDBG_LOW("buffer_%d plane_cnt %d width %d height %d stride %d", i,
        prepare_info.out_buf_map.frames[i].plane_cnt,
        prepare_info.out_buf_map.frames[i].plane[0].width,
        prepare_info.out_buf_map.frames[i].plane[0].height,
        prepare_info.out_buf_map.frames[i].plane[0].stride);
    }
  }
  IDBG_INFO("Mapped output buf list");

  if (!p_client) {
    IDBG_ERROR("Null client pointer");
    rc = IMG_ERR_INVALID_OPERATION;
    goto out_and_unlock;
  }
  rc = module_imglib_common_get_black_level(&shift_bits,
    p_client->isp_output_dim_stream_info.fmt);

  pchromatix_common = p_chromatix->chromatixComPtr;
  if (!pchromatix_common) {
    IDBG_ERROR("Null chromatix common");
    rc = IMG_ERR_INVALID_OPERATION;
    goto out_and_unlock;
  }
  pchromatix_L = &pchromatix_common->chromatix_L;
  prepare_info.black_level_b =
    pchromatix_L->linear_table_normal.b_lut_p[0] >> shift_bits;
  prepare_info.black_level_r =
    pchromatix_L->linear_table_normal.r_lut_p[0] >> shift_bits;
  prepare_info.black_level_gr =
    pchromatix_L->linear_table_normal.gr_lut_p[0] >> shift_bits;
  prepare_info.black_level_gb =
    pchromatix_L->linear_table_normal.gb_lut_p[0] >> shift_bits;

  if (IMG_ERROR(rc)) {
    prepare_info.black_level_b = BAYER_PROC_DEF_SENSOR_BLACK_LEVEL;
    prepare_info.black_level_r = BAYER_PROC_DEF_SENSOR_BLACK_LEVEL;
    prepare_info.black_level_gb = BAYER_PROC_DEF_SENSOR_BLACK_LEVEL;
    prepare_info.black_level_gr = BAYER_PROC_DEF_SENSOR_BLACK_LEVEL;
  }
  IDBG_MED("Chromatix normal data pedestal R %d GR %d B %d GB %d",
    prepare_info.black_level_r,
    prepare_info.black_level_gr,
    prepare_info.black_level_b,
    prepare_info.black_level_gb);
  IDBG_INFO("Got black level");

  rc = module_svhdr_client_stats_data(p_client, &prepare_info);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Can not get stats data set to default");
  }

  if (prepare_info.in_buf_map.frame_cnt || prepare_info.out_buf_map.frame_cnt) {
    rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_SET_LIB_CALLBACK,
      NULL); /*Use the deault framproc callback*/
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Fail to map stream buffers");
      rc = IMG_ERR_INVALID_OPERATION;
      goto out_and_unlock;
    }
    /*Set tuning info from chromatix pointer*/
    if(p_chromatix->chromatixIotPtr) {
      chromatix_iot_parms_type* p_chromatix_iot =
        (chromatix_iot_parms_type*)p_chromatix->chromatixIotPtr;
      IDBG_LOW("Loaded IOT Chromatix pointer");
      prepare_info.adc_bit_depth =
        p_chromatix_iot->shdr_mode_data.shdr_bayer_proc_params.adc_bit_depth;
      prepare_info.tm_out_bit_depth =
        p_chromatix_iot->shdr_mode_data.shdr_bayer_proc_params.tm_out_bit_depth;
      prepare_info.bayer_gtm_gamma =
        p_chromatix_iot->shdr_mode_data.shdr_bayer_proc_params.bayer_gtm_gamma;
      prepare_info.hdr_dark_n1 =
        p_chromatix_iot->shdr_mode_data.shdr_bayer_proc_params.hdr_dark_n1;
      prepare_info.hdr_dark_n2_minus_n1_normalization_factor =
        p_chromatix_iot->shdr_mode_data.shdr_bayer_proc_params.\
        hdr_dark_n2_minus_n1_normalization_factor;
      prepare_info.hdr_max_weight =
        p_chromatix_iot->shdr_mode_data.shdr_bayer_proc_params.hdr_max_weight;
      prepare_info.tm_gain =
        p_chromatix_iot->shdr_mode_data.shdr_bayer_proc_params.tm_gain;
      prepare_info.perf_hint =
        p_chromatix_iot->shdr_mode_data.shdr_bayer_proc_params.perf_hint;
      prepare_info.num_gpu_passes =
        p_chromatix_iot->shdr_mode_data.shdr_bayer_proc_params.num_gpu_passes;
      prepare_info.hdr_ratio =
        p_chromatix_iot->shdr_mode_data.shdr_bayer_proc_params.hdr_ratio;
    }

    lib_config.lib_data = &prepare_info;
    lib_config.lib_param = IMG_ALGO_PRESTART;
    rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_LIB_CONFIG,
      &lib_config);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Fail to map stream buffers");
      rc = IMG_ERR_INVALID_OPERATION;
      goto out_and_unlock;
    }
  }

  out_id = p_client->output_identity;
  stream_idx = module_imgbase_find_stream_by_identity(p_client, out_id);
  if (stream_idx < 0) {
    IDBG_ERROR("Cannot find stream mapped to idx %x", out_id);
    rc = IMG_ERR_INVALID_OPERATION;
    goto out_and_unlock;
  }
  IDBG_LOW("out_id %x", out_id);

  /* over write out_id with actual prev stream identity */
  out_id = p_client->stream[stream_idx].identity;
  IDBG_INFO("stream_idx of preview %x", stream_idx);
  p_stream = &p_client->stream[stream_idx];

  /* Set upstream event if source port is not available */
  if (p_client->native_buffer_list) {
    boolean is_upstream, ret_event;
    is_upstream = TRUE;
    ret_event = mod_imgbase_send_event(out_id, is_upstream,
      MCT_EVENT_MODULE_OUTPUT_BUFF_LIST, (*p_client->native_buffer_list));
    if (ret_event == FALSE) {
      IDBG_ERROR("Fail to send bayeproc event %d", is_upstream);
      rc = IMG_ERR_INVALID_OPERATION;
      goto out_and_unlock;

    }
  } else {
    IDBG_ERROR("p_client->native_buffer_list is NULL");
    rc = IMG_ERR_INVALID_OPERATION;
    goto out_and_unlock;
  }

  pthread_mutex_unlock(&p_client->mutex);
  *is_event_handled = TRUE;
  return TRUE;

out_and_unlock:
  pthread_mutex_unlock(&p_client->mutex);
  *is_event_handled = FALSE;
  return FALSE;
}

/**
 * Function: module_svhdr_client_stats_data
 *
 * Description: This function queries initial stats configuration
 *
 * Arguments:
 *   @p_client: IMG_BASE client.
 *   @p_info: Prepare info struct where black level need to be filled.
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_svhdr_client_stats_data(
  imgbase_client_t *p_client, bayerproc_prepare_info_t *p_info)
{
  imgbase_stream_t *p_stream;
  stats_get_data_t stats_get;
  boolean is_upstream, ret_event;
  int32_t i;
  int stream_idx;
  module_imgbase_t *p_mod = NULL;
  mct_aec_video_hdr_input_t hdr_ratios;

  if (!p_client || !p_info) {
    IDBG_ERROR("Invalid input");
    return IMG_ERR_INVALID_INPUT;
  }
  p_mod = (module_imgbase_t *)p_client->p_mod;
  if (!p_mod) {
    IDBG_ERROR("Invalid module pointer");
    return IMG_ERR_INVALID_INPUT;
  }
  img_svhdrmod_priv_t *p_svhdrmod_priv =
    (img_svhdrmod_priv_t *)p_mod->mod_private;
  if (!p_svhdrmod_priv) {
    IDBG_ERROR("Invalid private module data");
    return IMG_ERR_INVALID_INPUT;
  }
  sensor_out_info_t *out_dim =
    &p_svhdrmod_priv->session_data.sensor_output_dim;
  if (!out_dim) {
    IDBG_ERROR("Invalid sensor data pointer");
    return IMG_ERR_INVALID_INPUT;
  }
  memset(&stats_get, 0x00, sizeof(stats_get));

  stream_idx = module_imgbase_find_stream_by_identity(p_client,
    p_client->event_identity);
  if (stream_idx < 0) {
    IDBG_ERROR("Cannot find stream mapped to idx %x", p_client->event_identity);
    return IMG_ERR_GENERAL;
  }
  p_stream = &p_client->stream[stream_idx];

  /* Set upstream event if source port is not available */
  is_upstream = (p_stream->p_srcport == NULL) ? TRUE : FALSE;
  ret_event = mod_imgbase_send_event(p_client->event_identity,
    is_upstream, MCT_EVENT_MODULE_STATS_GET_DATA, stats_get);
  if (ret_event == FALSE) {
    IDBG_ERROR("Fail to send bayeproc event %d", is_upstream);
    return IMG_ERR_GENERAL;
  }
  p_info->wb_r_gain = stats_get.awb_get.r_gain;
  p_info->wb_g_gain = stats_get.awb_get.g_gain;
  p_info->wb_b_gain = stats_get.awb_get.b_gain;
  p_info->interlaced = out_dim->custom_format.enable;

  hdr_ratios = p_svhdrmod_priv->session_data.hdr_ratios;
  if (hdr_ratios.number_of_frames) {
    if (stats_get.aec_get.valid_entries != hdr_ratios.number_of_frames) {
      IDBG_ERROR("HDR frames does not fit %d != %d",
        stats_get.aec_get.valid_entries, hdr_ratios.number_of_frames);
    /* Todo */
//      return IMG_ERR_GENERAL;
    }

    p_info->number_of_hdr_frames = hdr_ratios.number_of_frames;
    p_info->hdr_ratios.number_of_frames = hdr_ratios.number_of_frames;
    for (i = 0; i < p_info->number_of_hdr_frames; i++) {
      p_info->linecount[i] = stats_get.aec_get.linecount[i];
      p_info->exposure_time[i] = 1000000LL * (stats_get.aec_get.exp_time *
        hdr_ratios.frame_exp_compensation[i]);
      p_info->sensor_gain[i] = stats_get.aec_get.real_gain[i];
    }
    for (i = 0; i < p_info->number_of_hdr_frames; i++) {
      p_info->hdr_ratios.frame_exp_compensation[i] =
        hdr_ratios.frame_exp_compensation[i];
    }
  }

  return IMG_SUCCESS;
}
