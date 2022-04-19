/**********************************************************************
* Copyright (c) 2013-2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include <linux/media.h>
#include "mct_module.h"
#include "module_cac.h"
#include "mct_stream.h"
#include "pthread.h"
#include "chromatix.h"
#include "mct_stream.h"

static int file_index = 0;

/*Hardcode for enabling CDS be default*/
//#define TEST_CDS

#define MODULE_CAC_PROPERTY_DUMP_DISABLE "no"
#define MODULE_CAC_PROPERTY_IN_DUMP_ENABLE "in"
#define MODULE_CAC_PROPERTY_OUT_DUMP_ENABLE "out"
#define MODULE_CAC_PROPERTY_IN_OUT_DUMP_ENABLE "inout"
 /**
 * Function: module_cac_client_allocate_cac_buffer
 *
 * Description: Pre Allocate CAC buffers
 *
 * Arguments:
 *   userdata : Current Session ID
 *   data - CAC Module
 *
 * Return values:
 *  None
 *
 **/
void module_cac_client_allocate_cac_buffer(void *userdata, void *data)
{
  int rc = IMG_SUCCESS;
  module_cac_t *p_mod = (module_cac_t *)data;
  uint32_t* p_session_id = (uint32_t*)userdata;
  cac_session_params_t *session_params;
  uint32_t session_id;

  if ((NULL == p_mod) || (NULL == p_session_id)) {
    IDBG_ERROR("%s:%d] Input is null, Failed to allocate memory",
      __func__, __LINE__);
    return;
  }
  session_id = *p_session_id;
  session_params = (cac_session_params_t *)
    module_cac_get_session_params(p_mod, session_id);
  if (!session_params) {
    IDBG_ERROR("%s:%d: Session params are null. Cannot allocate memory",
      __func__, __LINE__);
    return;
  }

  IDBG_MED("%s:%d] E ref_count %d", __func__, __LINE__,
    session_params->mem_alloc_ref_count);

  if(session_params->mem_alloc_ref_count == 0) {
    img_core_ops_t *p_core_ops = (img_core_ops_t*)&p_mod->core_ops;
    rc = IMG_CORE_PRELOAD(p_core_ops, NULL);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Memory allocation failed rc %d", rc);
    }
  }
  session_params->mem_alloc_ref_count++;

  //signal for buffer release
  pthread_cond_signal(&p_mod->cond);

  IDBG_MED("X alloc_ref_count %d", session_params->mem_alloc_ref_count);

  return;
}

/**
 * Function: module_cac_client_release_cac_buffer
 *
 * Description: Pre Allocate CAC buffers
 *
 * Arguments:
 *   userdata: Current session id
 *   data - CAC Module
 *
 * Return values:
 *  None
 *
 **/
void module_cac_client_release_cac_buffer(void *userdata, void *data)
{
  module_cac_t *p_mod = (module_cac_t *)data;
  uint32_t* p_session_id = (uint32_t*)userdata;
  cac_session_params_t *session_params;
  uint32_t session_id;
  int32_t rc;

  if ((NULL == p_mod) || (NULL == p_session_id)) {
    IDBG_ERROR("%s:%d] Input is null, Failed to deallocate memory",
      __func__, __LINE__);
    return;
  }

  session_id = *p_session_id;
  session_params = (cac_session_params_t *)
    module_cac_get_session_params(p_mod, session_id);
  if (!session_params) {
    IDBG_ERROR("%s:%d: Session params are null. Cannot release memory",
      __func__, __LINE__);
    return;
  }

  IDBG_MED("%s:%d] E, ref_count %d", __func__, __LINE__,
    session_params->mem_alloc_ref_count);

  pthread_mutex_lock(&p_mod->mutex);

  //check for allocation status before releasing buffers
  if (session_params->mem_alloc_ref_count == 0) {
    IDBG_ERROR("allocation pending, wait for completion %d",
      session_params->mem_alloc_ref_count);

    //wait for allocation to complete
    pthread_cond_wait(&p_mod->cond, &p_mod->mutex);
  }

  session_params->mem_alloc_ref_count--;

  pthread_mutex_unlock(&p_mod->mutex);

  if (session_params->mem_alloc_ref_count == 0) {
    img_core_ops_t *p_core_ops = (img_core_ops_t*)&p_mod->core_ops;
    rc = IMG_CORE_SHUTDOWN(p_core_ops);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Shutdown failed %d", rc);
    }
  }

  IDBG_MED("X alloc_ref_count %d",
    session_params->mem_alloc_ref_count);

  return;
}

/**
 * Function: module_cac_client_post_mct_msg
 *
 * Description: Post message to the MCT bus
 *
 * Arguments:
 *   p_client - CAC client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: Used only during pipelining for continuous burst shot
 **/
int module_cac_client_post_mct_msg(cac_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  cam_cac_info_t cac_info;
  mct_bus_msg_t bus_msg;
  mct_module_t *p_mct_mod = p_client->parent_mod;
  int str_idx = 0;

  str_idx = module_cac_get_stream_by_id(p_client,
    p_client->event_identity);
  if (str_idx < 0) {
    IDBG_ERROR("%s %d: Cannot find stream with identity 0x%x mapped to the "
      "client", __func__, __LINE__, p_client->p_buf_divert_data->identity);
    return IMG_ERR_GENERAL;
  }
  //Fill in cac info data
  cac_info.frame_id = p_client->p_buf_divert_data->buffer.sequence;
  cac_info.buf_idx = p_client->p_buf_divert_data->buffer.index;
  //Compose Bus message
  memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.type = MCT_BUS_MSG_CAC_STAGE_DONE;
  bus_msg.msg = (void *)&cac_info;
  bus_msg.size = sizeof(cam_cac_info_t);
  bus_msg.sessionid = IMGLIB_SESSIONID(p_client->stream[str_idx].identity);
  IDBG_MED("%s:%d] session id %d mct_mod %p", __func__, __LINE__,
    bus_msg.sessionid, p_mct_mod);
  rc = mct_module_post_bus_msg(p_mct_mod, &bus_msg);
  if (!rc) {
    IDBG_ERROR("%s:%d] Post MCT bus msg error", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }
  return IMG_SUCCESS;
}

/**
 * Function: module_cac_client_event_handler
 *
 * Description: event handler for FaceProc client
 *
 * Arguments:
 *   p_appdata - CAC test object p_event - pointer to the event
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int module_cac_client_event_handler(void* p_appdata,
  img_event_t *p_event)
{
  cac_client_t *p_client;
  img_component_ops_t *p_comp;
  int rc = IMG_SUCCESS;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  if ((NULL == p_event) || (NULL == p_appdata)) {
    IDBG_ERROR("%s:%d] invalid event", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  p_client = (cac_client_t *)p_appdata;
  p_comp = &p_client->comp;
  IDBG_LOW("%s:%d] type %d", __func__, __LINE__, p_event->type);

  switch (p_event->type) {
  case QIMG_EVT_BUF_DONE:
    //CAC processes one frame at a time currently. Nothing to be done.
    break;
  case QIMG_EVT_ERROR:
    IDBG_HIGH("%s %d: CAC Error", __func__, __LINE__);
  case QIMG_EVT_DONE:
    pthread_cond_signal(&p_client->cond);
    break;
  default:
    break;
  }
  return rc;

}

/**
 * Function: module_cac_client_getbuf
 *
 * Description: This function is to open the imaging buffer mgr
 *              and queing and dequeing the buffer.
 * Arguments:
 *   @p_client: cac client
 *   @pframe: frame pointer
 *   @native_buf: flag to indicate if its a native buffer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_cac_client_getbuf(cac_client_t *p_client,
  img_frame_t *pframe, int native_buf)
{
  int rc = IMG_SUCCESS;
  uint32_t i = 0;
  uint32_t buf_idx;
  uint32_t size;
  uint8_t *p_addr;
  mct_module_t *p_mct_mod;
  uint32_t padded_size;
  int fd = -1;
  int stride, scanline;
  isp_buf_divert_t *buf_divert = p_client->p_buf_divert_data;
  mct_stream_map_buf_t *buf_holder;
  mct_stream_info_t *stream_info;
  int str_idx = 0;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  if (!buf_divert) {
    IDBG_ERROR("%s:%d] Invalid inputs", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  str_idx = module_cac_get_stream_by_id(p_client, p_client->event_identity);
  if (str_idx < 0) {
    IDBG_ERROR("%s:%d] Cannot find stream info for identity 0x%x",
      __func__, __LINE__, p_client->event_identity);
    return IMG_ERR_GENERAL;
  }

  stream_info = p_client->stream[str_idx].stream_info;
  if (!stream_info) {
    IDBG_ERROR("%s:%d] stream info for identity 0x%x is NULL",
      __func__, __LINE__, p_client->event_identity);
    return IMG_ERR_GENERAL;
  }

  pframe->frame_cnt = 1;
  pframe->idx = 0;
  pframe->frame_id = buf_divert ->buffer.sequence;

  pframe->info.width = (uint32_t)p_client->stream[str_idx].in_dim.width;
  pframe->info.height = (uint32_t)p_client->stream[str_idx].in_dim.height;
  stride = p_client->stream[str_idx].in_dim.stride;
  scanline = p_client->stream[str_idx].in_dim.scanline;

  size = pframe->info.width * pframe->info.height;
  padded_size = (uint32_t)(stride * scanline);

  /* check actually how many */
  pframe->frame[0].plane_cnt = stream_info->buf_planes.plane_info.num_planes;
  pframe->idx = buf_idx = buf_divert->buffer.index;

  if (NULL == p_client->stream[str_idx].p_sinkport) {
    IDBG_ERROR("%s:%d] NULL Sink port", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  p_mct_mod =
    MCT_MODULE_CAST((MCT_PORT_PARENT(p_client->stream[str_idx].p_sinkport))->data);
  IDBG_MED("%s:%d] buf_idx %d identity %x mod %p port %p pproc %p",
    __func__, __LINE__, buf_idx, stream_info->identity, p_mct_mod,
    p_client->stream[str_idx].p_sinkport, p_client->parent_mod);

  if (!native_buf) {
    buf_holder = mct_module_get_buffer(buf_idx,
      p_client->parent_mod,
      IMGLIB_SESSIONID(stream_info->identity),
      IMGLIB_STREAMID(stream_info->identity));

    if (NULL == buf_holder) {
      IDBG_ERROR("%s:%d] NULL buff holder", __func__, __LINE__);
      return IMG_ERR_INVALID_OPERATION;
    }
    p_addr = buf_holder->buf_planes[0].buf;
    fd = buf_holder->buf_planes[0].fd;
  } else {
    /* vaddr for offline proc and other streams have different meaning */
    if (stream_info->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
      p_addr = buf_divert->vaddr;
    } else {
      p_addr = (void *)((unsigned long *)buf_divert->vaddr)[0];
    }
    fd = p_client->p_buf_divert_data->fd;
    IDBG_MED("%s:%d] Native Buffer addr = %p, fd = %d",
     __func__, __LINE__, p_addr, fd);
  }

  if (NULL == p_addr) {
    IDBG_ERROR("%s:%d] NULL address", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  if ((CAM_FORMAT_YUV_420_NV21 != stream_info->fmt) &&
    (CAM_FORMAT_YUV_420_NV12 != stream_info->fmt) &&
    (CAM_FORMAT_YUV_422_NV16 != stream_info->fmt) &&
    (CAM_FORMAT_YUV_422_NV61 != stream_info->fmt)) {
    IDBG_ERROR("%s:%d] Wrong image format, fmt=%d", __func__, __LINE__,
      stream_info->fmt);
    return IMG_ERR_INVALID_INPUT;
  }
  for (i = 0; i < pframe->frame[0].plane_cnt; i++) {
    pframe->frame[0].plane[i].plane_type = i;
    pframe->frame[0].plane[i].fd = fd;

    pframe->frame[0].plane[i].offset =
      stream_info->buf_planes.plane_info.mp[i].offset;
    pframe->frame[0].plane[i].addr = (i == 0) ? p_addr
      : p_addr + padded_size;
    pframe->frame[0].plane[i].stride = (uint32_t)stride;
    pframe->frame[0].plane[i].width = pframe->info.width;
    pframe->frame[0].plane[i].height = pframe->info.height;

    if (CAM_FORMAT_YUV_420_NV21 == stream_info->fmt)
      pframe->frame[0].plane[i].height /= (i + 1);
    if (i == 0) { //Y plane can have scanline different from height
      pframe->frame[0].plane[i].scanline = (uint32_t)scanline;
    } else {
      pframe->frame[0].plane[i].scanline = pframe->frame[0].plane[i].height;
    }

    pframe->frame[0].plane[i].length =
      pframe->frame[0].plane[i].height * pframe->frame[0].plane[i].stride;
  }
  if (CAM_FORMAT_YUV_420_NV21 == stream_info->fmt) {
    pframe->info.ss = IMG_H2V2;
  } else {
    pframe->info.ss = IMG_H2V1;
  }
  pframe->info.analysis = 0;

  IDBG_MED("%s:%d] Dimension %dx%d y stridexscanline %dx%d"
    "c stridexscanline %dx%d", __func__, __LINE__,
    pframe->info.width, pframe->info.height,
    stride, scanline, pframe->frame[0].plane[1].stride,
    pframe->frame[0].plane[1].scanline);

  if (p_client->dump_input_frame) {
    img_dump_frame(p_client->frame, "cac_input", (uint32_t)++file_index,
      NULL);
  }

  return rc;
}

/**
 * Function: module_cac_client_set_sw_cds
 *
 * Description: This function is for checking whether SW CDS
 * needs to be applied
 *
 * Arguments:
 *   @p_client: cac client
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_cac_client_set_sw_cds(cac_client_t *p_client)
{
  int32_t i;
  mct_stream_info_t *p_stream_info;
  p_client->use_sw_cds = 0;
  for (i = 0; i < p_client->stream_cnt; i++) {
    p_stream_info = p_client->stream[i].stream_info;
#ifdef TEST_CDS
    p_stream_info->reprocess_config.pp_feature_config.feature_mask |=
      CAM_QCOM_FEATURE_CDS;
#endif
    if (p_stream_info) {
      IDBG_MED("%s:%d] Stream identity %x type %d feat_mask %llx",
        __func__, __LINE__,
        p_stream_info->identity,
        p_stream_info->stream_type,
        p_stream_info->reprocess_config.pp_feature_config.feature_mask);
      if ((p_stream_info->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) &&
        ((p_stream_info->reprocess_config.pp_feature_config.feature_mask &
        CAM_QCOM_FEATURE_CDS) != 0)) {
        IDBG_HIGH("%s:%d] Use SW CDS identity %x CDS enabled %d",
          __func__, __LINE__,
          p_stream_info->identity,
          p_client->cac_cfg_info.cds_enable_flag);
        p_client->use_sw_cds = TRUE;
        break;
      }
    }
  }
}



/**
 * Function: module_cac_client_preprocess_performcache
 *
 * Description: This function is for handling the buffers
 *            sent from the peer modules
 *
 * Arguments:
 *   @p_client: cac client
 *   @p_frame: pointer to frame
 *   @p_buffer_access: pointer to buffer access flag
 *   @p_invalidate_cache_required: pointer to cache invalidate
 *     required flag
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
void module_cac_client_preprocess_performcache(cac_client_t *p_client,
  img_frame_t *p_frame, uint32_t *p_buffer_access,
  boolean *p_invalidate_cache_required)
{
  uint32_t buffer_access =
    p_client->p_buf_divert_data->buffer_access;
  int flush_cache = 0;
  boolean invalidate_cache_required = 0;
  boolean defer_invalidate_cache = 0;

  if (!p_client || !p_frame ||
    !p_buffer_access || !p_invalidate_cache_required) {
    IDBG_ERROR("Error: p_client=%p, p_frame=%p, "
      "p_buffer_access=%p, p_invalidate_cache_required=%p",
      p_client, p_frame, p_buffer_access,
      p_invalidate_cache_required);
    return;
  }

  void *v_addr = IMG_ADDR(p_frame);
  int32_t fd = IMG_FD(p_frame);
  int32_t buffer_size = IMG_FRAME_LEN(p_frame);

  // If any of CAC modifications happening through HW (exa GPU),
  //   We need to Flush if buffer_access has WRITE to make sure
  //   HW (GPU) reads latest data
  // Better not to invalidate the cache here if atleast one of cac operation
  // happening in SW as those algos can still read from cache.
  // and can still read from cache.
  // If we invalidate now here, even CPU reads go to main memory.

  // If atleast one HW operation
  if ((p_client->exec_mode == IMG_EXECUTION_HW) ||
    (p_client->exec_mode == IMG_EXECUTION_SW_HW)) {
    if ((buffer_access & CPU_HAS_WRITTEN) == CPU_HAS_WRITTEN) {
      // need flush
      flush_cache = 1;
    }

    if (buffer_access) {
      // cache invalidate is required as HW writes memory and
      // memory is cached because of earlier SW module READ.
      invalidate_cache_required = 1;
    }
  }

  // If atleast one sw
  if (((p_client->exec_mode == IMG_EXECUTION_SW) ||
    (p_client->exec_mode == IMG_EXECUTION_SW_HW)) &&
    (invalidate_cache_required == 1)) {
    // since there is a cpu operation, we can defer invalidation of cache
    defer_invalidate_cache = 1;
  }

  if ((flush_cache == 1) && (invalidate_cache_required == 1) &&
    (defer_invalidate_cache == 0)) {
    // do flush+invalidate
    img_cache_ops_external(v_addr, buffer_size, 0, fd,
      IMG_CACHE_CLEAN_INV, p_client->ion_fd);
    buffer_access = 0;
    invalidate_cache_required = 0;
  } else if (flush_cache == 1) {
    // do Flush
    img_cache_ops_external(v_addr, buffer_size, 0, fd,
      IMG_CACHE_CLEAN, p_client->ion_fd);

    buffer_access &= ~CPU_HAS_WRITTEN;
    // Indicate that the buffer is still cached
    buffer_access |= CPU_HAS_READ;
  }

  // With this we have valid data in main memroy and
  // as well in cache (if atleast one cac algo is running in SW).
  // So cac(cpu), rnrn(gpu) both will work.

  *p_buffer_access = buffer_access;
  *p_invalidate_cache_required = invalidate_cache_required;
}

/**
 * Function: module_cac_client_exec
 *
 * Description: This function is for handling the buffers
 *            sent from the peer modules
 *
 * Arguments:
 *   @p_client: cac client
 *   @p_buffer_access: pointer to buffer access flag
 *   @p_invalidate_cache_required: pointer to cache invalidate
 *     required flag
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_cac_client_exec(cac_client_t *p_client,
  uint32_t *p_buffer_access,
  boolean *p_invalidate_cache_required)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_client->comp;
  img_frame_t *p_frame = &p_client->frame[0];

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  // Get Frame first since we need dimension to calculate some rnr params
  rc = module_cac_client_getbuf(p_client, p_frame,
    p_client->p_buf_divert_data->native_buf);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s : Error: Cannot get frame", __func__);
    return IMG_ERR_GENERAL;
  }
  IDBG_HIGH("%s:%d] dim %dx%d frame %p", __func__, __LINE__,
    p_frame[0].info.width, p_frame[0].info.height, &p_frame[0]);

  /* set SW CDS flag for component configurations*/
  module_cac_client_set_sw_cds(p_client);

  rc = module_cac_config_client(p_client);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Error: Not Executing CAC", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  module_cac_client_preprocess_performcache(p_client, &p_frame[0],
    p_buffer_access, p_invalidate_cache_required);

  // Queuing only 1 buffer at a time.
  rc = IMG_COMP_Q_BUF(p_comp, &p_frame[0], IMG_IN);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_START(p_comp, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }
  p_client->state = IMGLIB_STATE_STARTED;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  return rc;
}

/**
 * Function: module_cac_client_send_buf_divert_ack
 *
 * Description: This function sends the ack back to upstream modules
 *
 * Arguments:
 *   @p_client: cac client
 *   @p_stream: current stream
 *
 * Return values:
 *     TRUE/FALSE
 *
 * Notes: none
 **/
boolean module_cac_client_send_buf_divert_ack(cac_client_t *p_client,
  stream_port_mapping_t *p_stream)
{
  boolean ret;
  isp_buf_divert_ack_t buff_divert_ack;

  memset(&buff_divert_ack, 0, sizeof(buff_divert_ack));
  buff_divert_ack.buf_idx = p_client->p_buf_divert_data->buffer.index;
  buff_divert_ack.is_buf_dirty = 1;
  buff_divert_ack.buffer_access = p_client->p_buf_divert_data->buffer_access;
  buff_divert_ack.identity = p_client->event_identity;
  buff_divert_ack.frame_id = p_client->p_buf_divert_data->buffer.sequence;
  buff_divert_ack.channel_id = p_client->p_buf_divert_data->channel_id;
  buff_divert_ack.meta_data = p_client->p_buf_divert_data->meta_data;
  buff_divert_ack.timestamp = p_client->p_buf_divert_data->buffer.timestamp;

  mct_port_t *p_port;
  mct_event_t event;
  p_port = p_stream->p_sinkport;
  memset(&event, 0x0, sizeof(mct_event_t));
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = buff_divert_ack.identity;
  event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT_ACK;
  event.u.module_event.module_event_data = &buff_divert_ack;

  ret =  mct_port_send_event_to_peer(p_port, &event);
  if (!ret) {
    IDBG_ERROR("%s:%d] cac2 Error, mct_port_send_event_to_peer failed %d",
    __func__, __LINE__, ret);
  }

  return ret;
}

/**
 * Function: module_cac_client_divert_exec
 *
 * Description: This function is for handling the buffers
 *            sent from the peer modules
 *
 * Arguments:
 *   @userdata: pointer to user data
 *   @data: pointer to data
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
void module_cac_client_divert_exec(void *userdata, void *data)
{
  int rc = IMG_SUCCESS;
  cac_client_t *p_client = (cac_client_t *)userdata;
  mod_img_msg_buf_divert_t *p_divert = (mod_img_msg_buf_divert_t *)data;
  mct_event_t buff_divert_event;
  int str_idx;
  bool algo_applied = true;
  uint32_t buffer_access = 0;
  boolean invalidate_cache_required = 0;

  if (!p_divert) {
    IDBG_ERROR("%s %d: bufer divert data is null", __func__, __LINE__);
    return;
  }
  p_client->p_buf_divert_data = &p_divert->buf_divert;
  str_idx = module_cac_get_stream_by_id(p_client, p_divert->identity);
  if (str_idx < 0) {
    IDBG_ERROR("%s %d: Cannot find stream with identity 0x%x mapped to the"
      "client", __func__, __LINE__, p_divert->identity);
    return;
  }

  pthread_mutex_lock(&p_client->mutex);

#ifdef TEST_CDS
  p_divert->buf_divert.is_uv_subsampled = TRUE;
#endif
  IDBG_HIGH("%s:%d] Start subsample %d", __func__, __LINE__,
    p_divert->buf_divert.is_uv_subsampled);

  if (p_client->stream[str_idx].stream_off) {
    IDBG_HIGH("%s:%d] streamoff called return", __func__, __LINE__);
    pthread_mutex_unlock(&p_client->mutex);
    return;
  }
  /* do not perform CAC if buffer is chroma subsampled */
  if (p_divert->buf_divert.is_uv_subsampled) {
    p_client->cac_cfg_info.cds_enable_flag = TRUE;
  } else {
    p_client->cac_cfg_info.cds_enable_flag = FALSE;
  }

  /*Get meta data ptr from buf_divert */
  meta_data_container *p_md_container = p_divert->buf_divert.meta_data;

  if (!p_md_container && (CAM_STREAM_TYPE_OFFLINE_PROC ==
    p_client->stream[str_idx].stream_info->stream_type)) {
    IDBG_ERROR("%s:%d] Invalid metadata buffer", __func__, __LINE__);
    pthread_mutex_unlock(&p_client->mutex);
    return;
  }

  if (p_md_container) {
    p_client->debug_meta_info.meta_data = p_md_container->pproc_meta_data;
  }

  /*reset early cb*/
  p_client->cb_sent = FALSE;

  rc = module_cac_client_exec(p_client, &buffer_access,
    &invalidate_cache_required);

  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] CAC Not Successful, rc = %d", __func__, __LINE__, rc);
    algo_applied = false;
  }
  //Currently CAC is in Syn mode. Will not wait here
  if ((p_client->mode == IMG_ASYNC_MODE) && (rc == IMG_SUCCESS)) {
    IDBG_HIGH("%s:%d] before wait rc %d", __func__, __LINE__, rc);
    rc = img_wait_for_completion(&p_client->cond, &p_client->mutex,
      10000);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] CAC Not Successful rc %d", __func__, __LINE__, rc);
      algo_applied = false;
    }
  }
  IDBG_HIGH("%s:%d] after wait rc %d", __func__, __LINE__, rc);

  if (p_client->use_sw_cds) {
    /* Reset UV subsample based on the algo result, so as to prevent
       CPP upscaling if CDS is not applied by SW
    */
    p_divert->buf_divert.is_uv_subsampled =
      p_client->cac_cfg_info.cds_enable_flag && algo_applied;

  }

  IDBG_MED("cds_enable %d lds_enable %d",
    p_client->cac_cfg_info.cds_enable_flag,
    p_client->cac_cfg_info.lds_enable_flag);

  /* do not perform LDS if CDS is disabled */
  if (!p_client->cac_cfg_info.cds_enable_flag) {
    p_client->cac_cfg_info.lds_enable_flag = FALSE;
  }

  p_client->p_buf_divert_data->lds_enabled =
    p_client->cac_cfg_info.lds_enable_flag;

  IDBG_MED("lds_enabled %d",
    p_client->p_buf_divert_data->lds_enabled);

  if ((p_client->exec_mode == IMG_EXECUTION_SW) ||
    (p_client->exec_mode == IMG_EXECUTION_SW_HW)) {
    buffer_access |= CPU_HAS_READ_WRITTEN;
  }

  // previous module has red the data through cache, and we wrote something
  // through HW in main memory. So we must invalidate memory.
  // But make sure we flush, if any of CAC algos wrote to cache (sw algos)
  if (invalidate_cache_required == 1) {
    img_frame_t *p_frame = &p_client->frame[0];
    void *v_addr = IMG_ADDR(p_frame);
    int32_t fd = IMG_FD(p_frame);
    int32_t buffer_size = IMG_FRAME_LEN(p_frame);

    if ((buffer_access & CPU_HAS_WRITTEN) == CPU_HAS_WRITTEN) {
      // we just wrote something through CPU into cache.
      // ---> Flush + Invalidate
      img_cache_ops_external(v_addr, buffer_size, 0, fd,
        IMG_CACHE_CLEAN_INV, p_client->ion_fd);
      buffer_access = 0;
    } else {
      // --> Invalidate
      img_cache_ops_external(v_addr, buffer_size, 0, fd,
        IMG_CACHE_INV, p_client->ion_fd);
      buffer_access = 0;
    }
  }

  pthread_mutex_unlock(&p_client->mutex);

  IDBG_MED("%s:%d] cb_sent %d", __func__, __LINE__, p_client->cb_sent);
  if (p_client->dump_output_frame) {
    buffer_access |= CPU_HAS_READ;
    img_dump_frame(p_client->frame, "cac_output", (uint32_t)++file_index,
      NULL);
  }

  /* Send event to CPP */
  if (!p_client->stream[str_idx].stream_off) {
    //Post to metadata
    if (!p_client->cb_sent) {
      rc = module_cac_client_post_mct_msg(p_client);
      if (rc != IMG_SUCCESS) {
        IDBG_ERROR("%s %d] Error Posting message to MCT bus",
          __func__, __LINE__);
      }
    }
    memset(&buff_divert_event, 0x0, sizeof(mct_event_t));
    buff_divert_event.type = MCT_EVENT_MODULE_EVENT;
    buff_divert_event.identity = p_client->event_identity;
    buff_divert_event.direction = MCT_EVENT_DOWNSTREAM;
    buff_divert_event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
    buff_divert_event.u.module_event.module_event_data =
      p_client->p_buf_divert_data;
    p_client->p_buf_divert_data->ack_flag = FALSE;
    p_client->p_buf_divert_data->buffer_access = buffer_access;
    rc =  mct_port_send_event_to_peer(p_client->stream[str_idx].p_srcport,
      &buff_divert_event);

    /* if ack is piggy backed, we can safely send ack to upstream */
    if (p_client->p_buf_divert_data->ack_flag == TRUE) {
      IDBG_HIGH("%s:%d] Got piggy-back ack from downstream module %d",
        __func__, __LINE__, rc);
      module_cac_client_send_buf_divert_ack(p_client,
        &p_client->stream[str_idx]);
    }
  }

  /* Stop the client*/
  if (p_client->state == IMGLIB_STATE_STARTED) {
    module_cac_client_stop(p_client);
  }
  IDBG_HIGH("%s:%d] End", __func__, __LINE__);
  return;
}

/**
 * Function: module_cac_client_stop
 *
 * Description: This function is used to stop the CAC
 *              client
 *
 * Arguments:
 *   @p_client: CAC client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_cac_client_stop(cac_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_client->comp;

  pthread_mutex_lock(&p_client->mutex);
  rc = IMG_COMP_ABORT(p_comp, NULL);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] CAC comp abort failed %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_client->mutex);
    return rc;
  }
  p_client->state = IMGLIB_STATE_INIT;
  pthread_mutex_unlock(&p_client->mutex);
  return rc;
}

/**
 * Function: module_cac_client_destroy
 *
 * Description: This function is used to destroy the cac client
 *
 * Arguments:
 *   @p_client: cac client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
void module_cac_client_destroy(cac_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = NULL;

  if (NULL == p_client) {
    return;
  }

  p_comp = &p_client->comp;
  IDBG_MED("%s:%d] state %d", __func__, __LINE__, p_client->state);

  if (IMGLIB_STATE_STARTED == p_client->state) {
    module_cac_client_stop(p_client);
  }

  if (IMGLIB_STATE_INIT == p_client->state) {
    rc = IMG_COMP_DEINIT(p_comp);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] deinit failed %d", __func__, __LINE__, rc);
    }
    p_client->state = IMGLIB_STATE_IDLE;
  }

  if (p_client->ion_fd >= 0) {
    close(p_client->ion_fd);
    p_client->ion_fd = -1;
  }

  if (IMGLIB_STATE_IDLE == p_client->state) {
    pthread_mutex_destroy(&p_client->mutex);
    pthread_cond_destroy(&p_client->cond);

    free(p_client);
    p_client = NULL;
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);

}

/**
 * Function: module_cac_client_early_cb
 *
 * Description: This function is used to send early callback from cac
 *
 * Arguments:
 *   @p_client: CAC client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_cac_client_early_cb(cac_client_t *p_client)
{
  IDBG_HIGH("%s:%d] p_client %p", __func__, __LINE__, p_client);
  p_client->cb_sent = TRUE;
  int rc = module_cac_client_post_mct_msg(p_client);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s %d] Error Posting message to MCT bus", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }
  return IMG_SUCCESS;
}

/** Function: module_cac_client_create
 *
 * Description: This function is used to create the CAC client
 *
 * Arguments:
 *   @p_mct_mod: mct module pointer
 *   @p_port: mct port pointer
 *   @identity: identity of the stream
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_cac_client_create(mct_module_t *p_mct_mod, mct_port_t *p_port,
  mct_stream_info_t *stream_info)
{
  int rc = IMG_SUCCESS;
  cac_client_t *p_client = NULL;
  img_component_ops_t *p_comp = NULL;
  img_core_ops_t *p_core_ops = NULL;
  module_cac_t *p_mod = (module_cac_t *)p_mct_mod->module_private;
  mct_list_t *p_temp_list = NULL;
#ifdef _ANDROID_
  char value[PROPERTY_VALUE_MAX];
#endif

  p_core_ops = &p_mod->core_ops;

  IDBG_MED("%s:%d]", __func__, __LINE__);
  p_client = (cac_client_t *)malloc(sizeof(cac_client_t));
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] CAC client alloc failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  /* initialize the variables */
  memset(p_client, 0x0, sizeof(cac_client_t));

  p_comp = &p_client->comp;
  pthread_mutex_init(&p_client->mutex, NULL);
  pthread_cond_init(&p_client->cond, NULL);
  p_client->state = IMGLIB_STATE_IDLE;
  memset(p_client->stream,  0x0, sizeof(stream_port_mapping_t) * MAX_CAC_STREAMS);
  p_client->stream[0].stream_info = stream_info;

  rc = IMG_COMP_CREATE(p_core_ops, p_comp);
  if (IMG_ERROR(rc)) {
   IDBG_ERROR("%s:%d] create failed %d", __func__, __LINE__, rc);
    goto error;
  }

  rc = IMG_COMP_INIT(p_comp, p_client, module_cac_client_early_cb);

  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] init failed %d", __func__, __LINE__, rc);
    goto error;
  }
  p_client->state = IMGLIB_STATE_INIT;

  rc = IMG_COMP_SET_CB(p_comp, module_cac_client_event_handler);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    goto error;
  }

    /* add the client to the list */
  p_temp_list = mct_list_append(p_mod->cac_client, p_client, NULL, NULL);
  if (NULL == p_temp_list) {
    IDBG_ERROR("%s:%d] list append failed", __func__, __LINE__);
    rc = IMG_ERR_NO_MEMORY;
    goto error;
  }
  p_mod->cac_client = p_temp_list;
  p_client->stream[0].p_sinkport = p_port;
  p_client->parent_mod = p_mod->parent_mod;
  p_client->p_mod = p_mod;
  p_client->exec_mode = IMG_EXECUTION_SW_HW;
  p_port->port_private = p_client;
  memset(p_client->frame, 0x0, sizeof(img_frame_t) * MAX_NUM_FRAMES);

  p_client->ion_fd = open("/dev/ion", O_RDONLY);

  p_client->dump_input_frame = FALSE;
  p_client->dump_output_frame = FALSE;

#ifdef _ANDROID_
  property_get("persist.vendor.camera.imglib.cac.dump",
    value,
    MODULE_CAC_PROPERTY_DUMP_DISABLE);

  if (!strncmp(MODULE_CAC_PROPERTY_IN_DUMP_ENABLE,
    value,
    sizeof(MODULE_CAC_PROPERTY_IN_DUMP_ENABLE))) {
      p_client->dump_input_frame = TRUE;
  } else if (!strncmp(MODULE_CAC_PROPERTY_OUT_DUMP_ENABLE,
    value,
    sizeof(MODULE_CAC_PROPERTY_OUT_DUMP_ENABLE))) {
      p_client->dump_output_frame = TRUE;
  } else if (!strncmp(MODULE_CAC_PROPERTY_IN_OUT_DUMP_ENABLE,
    value,
    sizeof(MODULE_CAC_PROPERTY_IN_OUT_DUMP_ENABLE))) {
      p_client->dump_input_frame = TRUE;
      p_client->dump_output_frame = TRUE;
  }

  property_get("persist.vendor.camera.dumpmetadata", value, 0);
  p_client->debug_meta_info.camera_dump_enabled = (uint8_t)atoi(value);
#endif

  IDBG_MED("%s:%d] port %p client %p X", __func__, __LINE__, p_port, p_client);
  return rc;

error:
  if (p_client) {
    module_cac_client_destroy(p_client);
    p_client = NULL;
  }
  return rc;
}
