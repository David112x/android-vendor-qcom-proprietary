/****************************************************************************
* Copyright (c) 2016-2017 Qualcomm Technologies, Inc.                       *
* All Rights Reserved.                                                      *
* Confidential and Proprietary - Qualcomm Technologies, Inc.                *
****************************************************************************/

#include "module_imgbase.h"
#include "chromatix_common.h"
#include "chromatix_iot.h"

/**
 *  Static functions
 **/
static int32_t module_bincorr_client_created(imgbase_client_t *p_client);
static int32_t module_bincorr_client_destroy(imgbase_client_t *p_client);
static boolean module_bincorr_handle_isp_output_dim(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client,  img_core_ops_t *p_coreops,
  boolean *is_evt_handled);
static int module_bincorr_handle_out_buf_done_event(imgbase_client_t *p_client,
  img_event_t *p_event);
static int module_bincorr_handle_buf_done(imgbase_client_t *p_client,
  img_event_t *p_event);
static boolean module_bincorr_handle_buf_divert(uint32_t event_identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled);
boolean module_bincorr_client_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void *p_mod, unsigned int sessionid);
static boolean module_bincorr_handle_set_stream_cfg(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client, img_core_ops_t *p_coreops,
  boolean *is_event_handled);
static int module_bincorr_client_set_in_dim(imgbase_client_t *p_client,
  uint32_t identity);
static int module_bincorr_client_get_out_dim(imgbase_client_t *p_client,
  mct_stream_info_t *p_out_stream_info);
static boolean module_bincorr_client_map_buf(uint32_t event_identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled);
int module_bincorr_client_prepare_native_bufs(
  imgbase_client_t *p_client, mct_stream_info_t *p_stream_info);
static boolean module_bincorr_handle_chrx_event(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled);
static boolean module_bincorr_handle_buf_divert_ack(uint32_t event_identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled);
static int32_t module_bincorr_client_prestreamon(imgbase_client_t * p_client,
  uint32_t identity);

/** g_caps:
 *
 *  Set the capabilities for bincorr module
*/
static img_caps_t g_caps = {
  .num_input = 1,
  .num_output = 1,
  .num_meta = 1,
  .share_client_per_session = 0,
  .use_internal_bufs = 1,
  .internal_buf_cnt = 4,
};

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_query_mod = module_bincorr_client_query_mod,
  .imgbase_client_created = module_bincorr_client_created,
  .imgbase_client_destroy = module_bincorr_client_destroy,
  .imgbase_client_pre_streamon = module_bincorr_client_prestreamon,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_SET_CHROMATIX_PTR] =
     module_bincorr_handle_chrx_event,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_SET_STREAM_CONFIG] =
     module_bincorr_handle_set_stream_cfg,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_ISP_OUTPUT_DIM] =
     module_bincorr_handle_isp_output_dim,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_BUF_DIVERT] =
     module_bincorr_handle_buf_divert,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_BUF_DIVERT_ACK] =
     module_bincorr_handle_buf_divert_ack,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_OUTPUT_BUFF_LIST] =
     module_bincorr_client_map_buf,
  .imgbase_client_event_handler[QIMG_EVT_IMG_BUF_DONE] =
     module_bincorr_handle_buf_done,
  .imgbase_client_event_handler[QIMG_EVT_IMG_OUT_BUF_DONE] =
     module_bincorr_handle_out_buf_done_event,
  .exec_mode = IMG_EXECUTION_HW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .force_cache_op = FALSE,
};

/** img_bincorr_session_data_t:
 *
 *   @chromatix_data: disable bincorr for preview
 *   @sensor_output_dim: Sensor output dimesions
 *
 *   Session based parameters for bincorr module
 */
typedef struct {
  modulesChromatix_t chromatix_data;
  sensor_out_info_t sensor_output_dim;
} img_bincorr_session_data_t;

/** img_bincorr_client_t:
 *
 *   @native_frame: Img frame data
 *
 *   binning correction client private structure
 */
typedef struct {
  img_frame_t native_frame;
} img_bincorr_client_t;

/** img_bincorrmod_priv_t:
 *
 *  @session_data: bincorrmod session data
 *
 *  bincorrmodule private structure
 */
typedef struct {
  img_bincorr_session_data_t session_data;
} img_bincorrmod_priv_t;


/**
 * Function: module_bincorr_client_query_mod
 *
 * Description: This function is used to query the imgbase module
 * info
 *
 * Arguments:
 *   @p_mct_cap: pipeline capability
 *   @p_mod: Module pointer
 *   @session_id: Session id
 *
 * Return values:
 *    true/false
 *
 * Notes: none
 **/
boolean module_bincorr_client_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void *p_mod, unsigned int session_id)
{
  IMG_UNUSED(session_id);
  mct_pipeline_pp_cap_t *pp_cap =  NULL;
  if (!p_mct_cap || !p_mod) {
    IDBG_ERROR("Invalid pointers p_mct_cap %p p_mod %p",
      p_mct_cap, p_mod);
    return FALSE;
  }
  pp_cap = &p_mct_cap->pp_cap;
  pp_cap->bincorr_modes[0] = CAM_BINNING_CORRECTION_MODE_OFF;
  pp_cap->bincorr_modes[1] = CAM_BINNING_CORRECTION_MODE_ON;
  pp_cap->supported_binning_correction_mode_cnt = 2;
  pp_cap->feature_mask |= CAM_QTI_FEATURE_BINNING_CORRECTION;

  return TRUE;
}

/**
 * Function: module_bincorr_client_created
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
int32_t module_bincorr_client_created(imgbase_client_t *p_client)
{
  int i;
  IDBG_MED(": E");
  p_client->ion_fd = open("/dev/ion", O_RDONLY);
  p_client->before_cpp = TRUE;
  p_client->bypass_enable = FALSE;
  p_client->is_bayer =  TRUE;
  img_bincorr_client_t *p_client_priv = NULL;
  p_client_priv =
    (img_bincorr_client_t *)calloc(1, sizeof(img_bincorr_client_t));
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

  /* The output buffer dimensions will be different from stream dimensions*/
  p_client->is_dim_change = TRUE;

  return IMG_SUCCESS;
}

/**
 * Function: module_bincorr_client_destroy
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
int32_t module_bincorr_client_destroy(imgbase_client_t *p_client)
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
 * Function: module_bincorr_client_process_done
 *
 * Description: function called after frame is processed
 *
 * Arguments:
 *   @p_client - IMG_BASE client
 *   @p_frame: output frame
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int32_t module_bincorr_client_prestreamon(imgbase_client_t * p_client,
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
    IDBG_ERROR("Cannot find stream mapped to idx %x", p_client->divert_identity);
    rc = IMG_ERR_INVALID_INPUT;
    goto error;
  }

  p_stream = &p_client->stream[stream_idx];

  // internal buffers for bincorr module are allocated on receiving
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
 * Function: module_bincorr_handle_buf_divert
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
static boolean module_bincorr_handle_buf_divert(uint32_t event_identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  IDBG_MED(": E");

  isp_buf_divert_t *p_buf_divert =
    (isp_buf_divert_t *)p_mod_event->module_event_data;
  IDBG_MED("MCT_EVENT_MODULE_BUF_DIVERT %x is_bayer = %d", event_identity,
    p_buf_divert->bayerdata);

  /* If this is a bayer buf divert, handle the event by setting
     is_evt_handled to FALSE
     and  return FALSE to avaoid sending this event downstream */
  if(p_buf_divert->bayerdata == TRUE) {
    *is_event_handled = FALSE;
    return FALSE;
  } else {
  /* If it is a yuv divert, send it to downstream by returning TRUE */
    *is_event_handled = TRUE;
    return TRUE;
  }
}

/**
 * Function: module_bincorr_handle_buf_divert_ack
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
static boolean module_bincorr_handle_buf_divert_ack(uint32_t event_identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  IDBG_MED(": E");

  isp_buf_divert_ack_t *p_buf_divert_ack =
    (isp_buf_divert_ack_t *)p_mod_event->module_event_data;
  IDBG_MED("MCT_EVENT_MODULE_BUF_DIVERT_ACK %x bayer = %d", event_identity,
    p_buf_divert_ack->bayerdata);

  /* If this is a bayer buf divert, handle the event by setting
     is_evt_handled to FALSE
     and  return FALSE to avoid sending this event downstream */
  if(p_buf_divert_ack->bayerdata == TRUE) {
    *is_event_handled = FALSE;
    return FALSE;
  } else {
  /* If it is a yuv divert, send it to downstream by returning TRUE */
    *is_event_handled = TRUE;
    return TRUE;
  }
  IDBG_MED(": X");
}


/**
 * Function: module_bincorr_handle_isp_output_dim
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
static boolean module_bincorr_handle_isp_output_dim(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  IDBG_HIGH(": E event_identity %x", event_identity);
  uint32_t output_identity = 0, ret = TRUE;
  int  rc = 0;
  mct_stream_info_t *stream_info =
    (mct_stream_info_t *)(p_mod_event->module_event_data);

  /* ISP sends two events with info from RDI stream
   and CAMIF and distinguished between them
   using bayerdata flag. */
  if (stream_info) {
    if (stream_info->bayerdata == 0) {
      IDBG_MED("Isp out dim evt bayerdata = 0, fwd'ing it without handling");
      return TRUE;
    } else {
      pthread_mutex_lock(&p_client->mutex);
      p_client->isp_output_dim_stream_info = *stream_info;
      p_client->isp_output_dim_stream_info_valid = TRUE;
      pthread_mutex_unlock(&p_client->mutex);
    }
    IDBG_HIGH("MCT_EVENT_MODULE_ISP_OUTPUT_DIM %dX%d fmt %d bayerdata %d",
      stream_info->dim.width,
      stream_info->dim.height,
      stream_info->fmt,
      stream_info->bayerdata);

    if (p_client->bypass_enable == FALSE &&
      stream_info->bayerdata == TRUE) {
      output_identity = p_client->output_identity;
      IDBG_HIGH("output_identity %x", output_identity);
      rc = module_bincorr_client_set_in_dim(p_client, output_identity);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("Fail to prepary buffs for 0x%x", event_identity);
      }
    }
  }
  *is_event_handled = TRUE;
  /* Do not forward event downstream as this is bayer dim info*/
  ret = FALSE;

  IDBG_HIGH(": X");
  return ret;
}

/**
 * Function: module_bincorr_handle_out_buf_done_event
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
int module_bincorr_handle_out_buf_done_event(imgbase_client_t *p_client,
  img_event_t *p_event)
{
  int rc = IMG_ERR_GENERAL, ret;
  int stream_idx = 0;
  uint32_t identity, frame_id;
  imgbase_stream_t *p_stream = NULL;
  imgbase_buf_t *p_imgbase_buf = NULL;
  img_frame_t *p_frame = NULL;
  isp_buf_divert_t *p_buf_divert = NULL, img_buf_divert;
  if (!p_client || !p_event) {
    IDBG_ERROR("Invalid input pointers");
    return IMG_ERR_INVALID_INPUT;
  }
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  p_frame = p_event->d.p_frame;

  if (!p_frame || !p_frame->private_data) {
    IDBG_ERROR("Invalid frame pointer");
    return IMG_ERR_INVALID_INPUT;
  }
  p_imgbase_buf = (imgbase_buf_t *)p_frame->private_data;

  IDBG_HIGH("buf_done for frame id %d", p_frame->frame_id);
  stream_idx = module_imgbase_find_stream_by_identity(p_client,
    p_client->divert_identity);
  if (stream_idx < 0) {
    IDBG_ERROR("[%s] Cannot find stream mapped to idx %x",p_mod->name,
      p_client->divert_identity);
    return rc;
  }

  p_stream = &p_client->stream[stream_idx];

  p_buf_divert = &p_imgbase_buf->buf_divert;
  identity = p_imgbase_buf->event_identity ? p_imgbase_buf->event_identity :
    p_buf_divert->identity;
  IDBG_ERROR("[%s] identity %x E", p_mod->name, identity);

  if (p_stream->p_sinkport) {
    /* If sink port mapped, send op buffer upstream via buf divert */
    frame_id = p_buf_divert->buffer.sequence;

    IDBG_MED("[%s] [IMG_BUF_DBG_OUT_f%d_b%d_i%x] send buf_div timestamp %llu",
      p_mod->name, frame_id, p_frame->idx, identity,
      p_imgbase_buf->buf_divert.buffer.timestamp.tv_sec
      * 1000000LL + p_imgbase_buf->buf_divert.buffer.timestamp.tv_usec);

    /* Fill buffer divert from frame */
    memset(&img_buf_divert, 0x00, sizeof(img_buf_divert));
    img_buf_divert.fd = p_frame->frame[0].plane[0].fd;
    img_buf_divert.vaddr = p_frame->frame[0].plane[0].addr;
    img_buf_divert.buffer.sequence = p_frame->frame_id;
    img_buf_divert.buffer.index = p_frame->idx;
    img_buf_divert.native_buf = TRUE;
    img_buf_divert.identity = p_client->divert_identity;
    img_buf_divert.channel_id = 1;

    ret = mod_imgbase_send_event(p_client->divert_identity, TRUE,
      MCT_EVENT_MODULE_BUF_DIVERT, img_buf_divert);
    rc = (ret == TRUE) ? IMG_SUCCESS :  IMG_ERR_GENERAL;

    if (ret == FALSE) {
      IDBG_MED("%s] Ack flag %d or error %d return buffer to the queue",
        p_mod->name, img_buf_divert.ack_flag, ret);
      isp_buf_divert_ack_t ack;
      ack.identity = img_buf_divert.identity;
      ack.frame_id = img_buf_divert.buffer.sequence;
      /*Enque buffer back*/
      rc = module_imgbase_client_handle_buffer_ack(p_client,
        IMG_EVT_ACK_FREE_INTERNAL_BUF, &ack);
      return rc;
    }
    IDBG_LOW("Sent the buffer divert to upstream iface since native");
    p_frame = NULL;
  }
  return rc;
}

/**
 * Function: module_imgbase_client_handle_buf_done
 *
 * Description: Function to handle input buf done event
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *   @p_frame - frame for buf done
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
int module_bincorr_handle_buf_done(imgbase_client_t *p_client,
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
    IDBG_ERROR("Error invalid inputs");
    return rc;
  }
  p_mod = (module_imgbase_t *)p_client->p_mod;
  if (!p_mod) {
    IDBG_ERROR("Error module pointer");
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
      IDBG_WARN("Unable to handle buf_done caps.num_output %d acks_needed %d",
        p_mod->caps.num_output,
        p_mod->caps.ack_required);
      return rc;
    }
  }
  /* if overlap batch processing, this is freed in handle_overlap_input */
  if (!p_mod->caps.num_overlap && p_imgbase_buf && free_frame) {
    free(p_imgbase_buf);
    p_imgbase_buf = NULL;
  }
  return rc;

end:
  return rc;
}

/**
 * Function: module_bincorr_handle_chrx_event
 *
 * Description: Founction to handle chromatix event.
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
static boolean module_bincorr_handle_chrx_event(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  IDBG_LOW(": E");
  boolean ret = TRUE;
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
  img_bincorrmod_priv_t *p_bincorrmod_priv =
    (img_bincorrmod_priv_t *)p_mod->mod_private;
  if (!p_bincorrmod_priv) {
    IDBG_ERROR("Invalid private module data");
    return IMG_ERR_INVALID_INPUT;
  }
  modulesChromatix_t *chromatix_param =
    (modulesChromatix_t*)p_mod_event->module_event_data;
  p_bincorrmod_priv->session_data.chromatix_data =
    *chromatix_param;
  IDBG_LOW(": X");

  return ret;
}


/**
 * Function: module_bincorr_handle_set_stream_cfg
 *
 * Description: Founction to handle chromatix event.
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
static boolean module_bincorr_handle_set_stream_cfg(uint32_t event_identity,
  mct_event_module_t *p_mod_event,
  imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  IDBG_LOW(": E");
  boolean ret = TRUE;
  module_imgbase_t *p_mod = NULL;
  sensor_out_info_t *p_sensor_out_dim = NULL;

  p_mod = (module_imgbase_t *)p_client->p_mod;
  if (NULL == p_mod || p_mod_event == NULL) {
    IDBG_ERROR("imgbase module or module evt payload NULL");
    *is_event_handled = FALSE;
    return FALSE;
  }
  img_bincorrmod_priv_t *p_bincorrmod_priv =
    (img_bincorrmod_priv_t *)p_mod->mod_private;
  if (p_bincorrmod_priv == NULL) {
    IDBG_ERROR("NULL private module data pointer");
    *is_event_handled = FALSE;
    return FALSE;
  }
  sensor_out_info_t *p_output_info =
    (sensor_out_info_t *)p_mod_event->module_event_data;
  p_sensor_out_dim =
    &p_bincorrmod_priv->session_data.sensor_output_dim;

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
   IDBG_LOW(": X");
  return ret;
}


/**
 * Function: module_bincorr_client_update_map
 *
 * Description: Callback function used in list traverse
 * to map buffers from stream info.
 *
 * Input parameters:
 *   @data - MCT stream buffer mapping
 *   @user - IMG frame map structure
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_bincorr_client_update_map(void *data, void *user)
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
 * Function: module_bincorr_client_map_buf
 *
 * Description: This function is used to map stream buffers to component.
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
 * Notes: none
 **/
static boolean module_bincorr_client_map_buf(uint32_t event_identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_coreops, boolean *is_event_handled)
{
  img_component_ops_t *p_comp;
  mct_stream_info_t *p_in_stream;
  bc_prestart_info_t prepare_info;
  img_lib_config_t lib_config;
  uint32_t i, j, out_id;
  int stream_idx;
  int rc = IMG_ERR_GENERAL;
  imgbase_stream_t *p_stream = NULL;
  out_id = event_identity;
  chromatix_VFE_common_type *pchromatix_common = NULL;
  module_imgbase_t *p_mod = NULL;
  mct_list_t *p_img_buffer_list = NULL;
  modulesChromatix_t *p_chromatix =  NULL;
  img_bincorr_client_t *p_client_priv = NULL;
  IDBG_LOW("E:");

  p_img_buffer_list = (mct_list_t *)p_mod_event->module_event_data;
  /* Dont forward this event downstream*/
  if (!p_client || !p_img_buffer_list) {
    IDBG_ERROR("Invalid input");
    return FALSE;
  }
  p_comp = &p_client->comp;

  p_mod = (module_imgbase_t *)p_client->p_mod;
  img_bincorrmod_priv_t *p_bincorrmod_priv =
    (img_bincorrmod_priv_t *)p_mod->mod_private;
  p_client_priv =
    (img_bincorr_client_t *)p_client->p_private_data;
  if (!p_bincorrmod_priv || !p_client_priv) {
    IDBG_ERROR("Invalid private module/client data");
    return IMG_ERR_INVALID_INPUT;
  }

  sensor_out_info_t *out_dim =
    &p_bincorrmod_priv->session_data.sensor_output_dim;
  p_chromatix = &p_bincorrmod_priv->session_data.chromatix_data;
  if(out_dim == NULL || p_chromatix == NULL) {
    IDBG_ERROR("NULL sensor or chromatix pointer");
    return IMG_ERR_INVALID_INPUT;
  }
  pthread_mutex_lock(&p_client->mutex);
  out_id = p_client->output_identity;
  stream_idx = module_imgbase_find_stream_by_identity(p_client, out_id);

  IDBG_LOW("stream_idx %x", stream_idx);
  if (stream_idx < 0) {
    IDBG_ERROR("Cannot find stream mapped to idx %x", out_id);
    rc = IMG_ERR_INVALID_OPERATION;
    goto out_and_unlock;
  }
  p_stream = &p_client->stream[stream_idx];
  memcpy(p_stream->stream_info, &p_client->out_stream_info,
    sizeof(mct_stream_info_t));

  rc = module_bincorr_client_prepare_native_bufs(p_client,
    &p_client->out_stream_info);

  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Cannot prepare native buffers ");
    pthread_mutex_unlock(&p_client->mutex);
    return IMG_ERR_GENERAL;
  }

  p_in_stream = NULL;
  if (p_client->isp_output_dim_stream_info_valid) {
    p_in_stream = &p_client->isp_output_dim_stream_info;
    IDBG_INFO("stride from ISP \"bayer = 1\"  %d",
      p_in_stream->buf_planes.plane_info.mp[0].stride);
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
      p_in_stream->buf_planes.plane_info.mp[0].width, fmt);
    if (!stride_bytes) {
      IDBG_ERROR("Can not get stride in bytes");
      rc = IMG_ERR_INVALID_OPERATION;
      goto out_and_unlock;
    }
    IDBG_INFO("Got stride in bytes %d", stride_bytes);
    mct_list_traverse(p_img_buffer_list,
      module_bincorr_client_update_map, &prepare_info.in_buf_map);

    /* Width and height are not part of map info we need to fill them here */
    for (i = 0; i < prepare_info.in_buf_map.frame_cnt; i++) {
      for (j = 0; j < prepare_info.in_buf_map.frames[i].plane_cnt; j++) {
        prepare_info.in_buf_map.frames[i].plane[j].width =
          p_in_stream->dim.width;
        prepare_info.in_buf_map.frames[i].plane[j].height =
          p_in_stream->dim.height;
        prepare_info.in_buf_map.frames[i].plane[j].stride = stride_bytes;
      }
      IDBG_ERROR("buffer_%d plane_cnt %d width %d height %d stride %d", i,
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
      module_bincorr_client_update_map, &prepare_info.out_buf_map);
    /* Width and height are not part of map info we need to fill them here */
    for (i = 0; i < prepare_info.out_buf_map.frame_cnt; i++) {
      for (j = 0; j < prepare_info.out_buf_map.frames[i].plane_cnt; j++) {
        prepare_info.out_buf_map.frames[i].plane[j].width =
          p_client_priv->native_frame.frame[0].plane[0].width;
        prepare_info.out_buf_map.frames[i].plane[j].height =
          p_client_priv->native_frame.frame[0].plane[0].height;
      }
      IDBG_ERROR("buffer_%d plane_cnt %d width %d height %d stride %d", i,
        prepare_info.out_buf_map.frames[i].plane_cnt,
        prepare_info.out_buf_map.frames[i].plane[0].width,
        prepare_info.out_buf_map.frames[i].plane[0].height,
        prepare_info.out_buf_map.frames[i].plane[0].stride);
    }
  }
  IDBG_INFO("Mapped output buf list");

  pchromatix_common = p_chromatix->chromatixComPtr;
  if (!pchromatix_common) {
    rc = IMG_ERR_INVALID_OPERATION;
    IDBG_ERROR("Null chromatix common");
    goto out_and_unlock;
  }

  if (prepare_info.in_buf_map.frame_cnt || prepare_info.out_buf_map.frame_cnt) {
    rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_SET_LIB_CALLBACK,
      NULL); /*Use the deault framproc callback*/
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Fail to map stream buffers");
      rc = IMG_ERR_INVALID_OPERATION;
      goto out_and_unlock;
    }
/*  TODO: Read tuning values from chromatix
    if(p_chromatix->chromatixIotPtr) {
      chromatix_iot_parms_type* p_chromatix_iot =
        (chromatix_iot_parms_type*)p_chromatix->chromatixIotPtr;
      IDBG_LOW("Loaded IOT Chromatix pointer");
    }
*/
    prepare_info.alpha_col = 0.875f;
    prepare_info.alpha_row = 0.875f;
    prepare_info.beta_row = 0.125f;
    prepare_info.beta_col = 0.125f;
    prepare_info.hor_correction_mode = 1;
    prepare_info.ver_correction_mode = 1;

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
  p_stream = &p_client->stream[stream_idx];

  /* Set upstream event if source port is not available */
  if (p_client->native_buffer_list) {
    boolean is_upstream, ret_event;
    is_upstream = TRUE;
    ret_event = mod_imgbase_send_event(event_identity, is_upstream,
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
  IDBG_LOW("X:");
  return FALSE;

out_and_unlock:
  pthread_mutex_unlock(&p_client->mutex);
  return FALSE;
}

/**
 * Function: module_bincorr_deinit
 *
 * Description: Function used to deinit Bincorr module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_bincorr_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_t *p_mod = NULL;
  img_bincorrmod_priv_t *p_bincorrmod_data = NULL;
  if(!p_mct_mod) {
    IDBG_ERROR("Invalid module %p", p_mct_mod);
    return;
  }

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (!p_mod) {
    IDBG_ERROR("Invalid base module %p", p_mod);
    return;
  }
  p_bincorrmod_data = p_mod->mod_private;
  if (p_bincorrmod_data) {
    free(p_bincorrmod_data);
  }

  module_imgbase_deinit(p_mct_mod);
}

/** module_bincorr_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: Function used to initialize the Bioncorr module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_bincorr_init(const char *name)
{
  img_bincorrmod_priv_t *p_bincorr_priv_data = NULL;
  p_bincorr_priv_data = calloc(1, sizeof(img_bincorrmod_priv_t ));

  if(p_bincorr_priv_data == NULL) {
    IDBG_ERROR("Unable allocate module private data");
    return NULL;
  }

  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    p_bincorr_priv_data,
    &g_caps,
    "libmmcamera_bincorr.so",
    BAYERPROC_FEATURE_BINCORR,
    &g_params);

}

/**
 * Function: module_bincorr_client_prepare_native_bufs
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
int module_bincorr_client_prepare_native_bufs(
  imgbase_client_t *p_client, mct_stream_info_t *p_stream_info)
{
  uint32_t buffer_size, stride_bytes, scanline, out_id;
  int stream_idx;
  img_frame_info_t frame_info;
  imgbase_stream_t *p_stream = NULL;
  module_imgbase_t *p_mod = NULL;
  img_bincorr_client_t *p_priv_client = NULL;
  int rc;
  /* This module expects client private data pointer */
  if (!p_client || !p_stream_info || !p_client->p_private_data) {
    IDBG_ERROR("Invalid input");
    return IMG_ERR_INVALID_INPUT;
  }

  p_mod = (module_imgbase_t *)p_client->p_mod;
  if (!p_mod) {
    IDBG_ERROR("%s:%d] Error invalid module", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  p_priv_client = (img_bincorr_client_t *)p_client->p_private_data;

  if (p_mod->caps.internal_buf_cnt == 0) {
    IDBG_WARN("No need of internal buffers %d",
      p_mod->caps.internal_buf_cnt);
    return IMG_SUCCESS;
  }

  memset(&frame_info, 0x00, sizeof(frame_info));
  frame_info.client_id = -1;
  frame_info.width = p_stream_info->dim.width;
  frame_info.height = p_stream_info->dim.height;
  IDBG_INFO("p_stream_info->dim.width %d height %d",
    frame_info.width, frame_info.height);

  rc = module_imglib_common_get_frame_fmt(p_stream_info->fmt,
    &frame_info.fmt);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Cam format unsupported");
    return IMG_ERR_NO_MEMORY;
  }
  scanline = p_stream_info->buf_planes.plane_info.mp[0].scanline;
  stride_bytes = p_stream_info->buf_planes.plane_info.mp[0].stride_in_bytes;
  buffer_size = stride_bytes * scanline;

  out_id = p_client->output_identity;
  stream_idx = module_imgbase_find_stream_by_identity(p_client, out_id);
  IDBG_LOW("out_id %x", out_id);
  if (stream_idx < 0) {
    IDBG_ERROR("Cannot find stream mapped to idx %x", stream_idx);
    return IMG_ERR_INVALID_OPERATION;
  }
  p_stream = &p_client->stream[stream_idx];

  rc = module_imgbase_client_alloc_int_buf(p_client, out_id);
  if (!IMG_SUCCEEDED(rc)) {
    IDBG_ERROR("Cannot allocate internal buffers rc = %d", rc);
    return rc;
  }

  p_priv_client->native_frame.frame[0].plane[0].width = frame_info.width;
  p_priv_client->native_frame.frame[0].plane[0].height = frame_info.height;
  rc = module_imgbase_client_pack_buffer(p_client, stride_bytes, scanline);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Unable to pack buffers");
    return IMG_ERR_NO_MEMORY;
  }

  return IMG_SUCCESS;
}


/**
 * Function: module_bincorr_client_set_in_dim
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
static int module_bincorr_client_set_in_dim(imgbase_client_t *p_client,
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

  rc = module_bincorr_client_get_out_dim(p_client, &out_stream_info);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Cannot get output dim");
    pthread_mutex_unlock(&p_client->mutex);
    return IMG_ERR_GENERAL;
  }
  /*Set bayer dimesions as stream dimensions*/
  memcpy(&p_client->out_stream_info, &out_stream_info,
    sizeof(mct_stream_info_t));

  stream_idx = module_imgbase_find_stream_by_identity(p_client, identity);
  if (stream_idx < 0) {
    IDBG_ERROR("Cannot find stream mapped to idx %x", identity);
    pthread_mutex_unlock(&p_client->mutex);
    return IMG_ERR_GENERAL;
  }
  p_stream = &p_client->stream[stream_idx];
  pthread_mutex_unlock(&p_client->mutex);
  IDBG_INFO("Sending MCT_EVENT_MODULE_BAYERPROC_OUTPUT_DIM evt to IFACE\n");
  IDBG_INFO("Stride %d Stride_in_bytes %d Scanline %d\n",
    out_stream_info.buf_planes.plane_info.mp[0].stride,
    out_stream_info.buf_planes.plane_info.mp[0].stride_in_bytes,
    out_stream_info.buf_planes.plane_info.mp[0].scanline);

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
 * Function: module_bincorr_client_get_out_dim
 *
 * Description: This function is used to get the output dims.
 *
 * Arguments:
 *   @p_client: IMG_BASE client
 *   @p_native_stream_info: Pointer to stream info filled with output dims.
 *
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_bincorr_client_get_out_dim(imgbase_client_t *p_client,
  mct_stream_info_t *p_out_stream_info)
{
  mct_stream_info_t *p_stream_info;
  img_component_ops_t *p_comp;
  img_format_t fmt;
  bayerproc_in_out_frame_info_t info;
  uint32_t buffer_size, stride_bytes, stride_pixels;
  uint32_t scanline, width, height;
  int rc;
  module_imgbase_t *p_mod = NULL;
  img_lib_config_t lib_config;

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
  img_bincorrmod_priv_t *p_bincorrmod_priv =
    (img_bincorrmod_priv_t *)p_mod->mod_private;
  if (!p_bincorrmod_priv) {
    IDBG_ERROR("Invalid private module data");
    return IMG_ERR_INVALID_INPUT;
  }
  sensor_out_info_t *out_dim =
    &p_bincorrmod_priv->session_data.sensor_output_dim;

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
  IDBG_LOW("Got frame imglib fmt %d for cam fmt %d",
    fmt, p_stream_info->fmt);

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
    IDBG_LOW("info.out_info.fmt %d width %d",info.out_info.fmt, width);
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

  /*Fix me: ISP configuring to different dimension than expected*/
  buffer_size = stride_bytes * scanline * 2;
  IDBG_INFO("W %d H %d Scanline %d StridInPix %d StridInByt %d",
    width, height, scanline, stride_pixels, stride_bytes);
  IDBG_INFO("out_info fmt %d ", info.out_info.fmt);

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
