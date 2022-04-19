/**********************************************************************
*  Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include <linux/media.h>
#include "mct_module.h"
#include "module_imgbase.h"
#include "mct_stream.h"
#include "pthread.h"
#include "chromatix.h"
#include "mct_stream.h"
#include "module_imglib_common.h"
#include "chromatix_common.h"

#define DEBUG_BATCH 0
#define DEBUG_INTERNAL_BUF_LIST 0

/**
 * MACRO: PRINT_BUF_LIST
 *
 * Description:
 *   Print contents of overlap buffers list
 *
 * Arguments:
 *   @p_client: imgbase client
 *
 **/
#define PRINT_BUF_LIST(p_client) ({ \
if (DEBUG_BATCH) { \
  const int size = 500; \
  char dst [size]; \
  char scratch [size]; \
  dst[0] = '\0'; \
  imgbase_buf_t *p_buf = NULL; \
  strlcat(dst, "BUF LIST ", size); \
  for (i = 0; i < MAX_IMGLIB_BATCH_SIZE; i++) { \
    p_buf = p_client->buf_list[i]; \
    if (p_buf) { \
      snprintf (scratch, size, "[%d, %d %d %d, %d]", \
      p_buf->frame.frame_id, \
      p_buf->queued_cnt, \
      p_buf->dequeued_cnt, \
      p_buf->divert_done, \
      p_buf->ack.ack_received_cnt); \
    } else { \
      snprintf (scratch, size, "[0, 0 0 0, 0]"); \
    } \
    strlcat(dst, scratch, size); \
  } \
  IDBG_MED("%s:%d] %s \n", __func__, __LINE__, dst); \
}\
})

/**
 * MACRO: PRINT_INTERNAL_BUF_LIST
 *
 * Description:
 *   Print contents of internal buf list
 *
 * Arguments:
 *   @p_client: imgbase client
 *
 **/
#define PRINT_INTERNAL_BUF_LIST(p_client) ({ \
if (DEBUG_INTERNAL_BUF_LIST) { \
  const int size = 500; \
  char dst [size]; \
  char scratch [size]; \
  img_frame_t *p_frame; \
  dst[0] = '\0'; \
  strlcat(dst, "INTERNAL BUF ", size); \
  for (i = 0; i < p_client->internal_buf.buf_cnt; i++) { \
    p_frame = p_client->internal_buf.buf[i].p_frame; \
    snprintf (scratch, size, "[%d, %d %d]", \
      p_frame ? (int32_t )p_frame->frame_id : -1, \
      p_client->internal_buf.buf[i].ref_cnt, \
      p_client->internal_buf.buf[i].input_ack_held); \
    strlcat(dst, scratch, size); \
  } \
  IDBG_MED("%s \n", dst); \
}\
})

/**
 *  STATIC FUNCTIONS
 */

static int module_imgbase_client_handle_src_divert_wrapper(
  void *userdata);

/**
 * Function: module_imgbase_client_preload_exec
 *
 * Description: function called to execute the preload
 *
 * Arguments:
 *   @p_appdata - pointer to base module
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int32_t module_imgbase_client_preload_exec(void *p_appdata)
{
  imgbase_session_data_t *p_session_data = (imgbase_session_data_t *)p_appdata;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_session_data->p_mod;
  IDBG_MED(" E ");
  img_preload_param_t preload_param;
  preload_param = p_session_data->preload_params.param;

  pthread_mutex_lock(&p_mod->mutex);
  int32_t rc = IMG_CORE_PRELOAD(&p_mod->core_ops, &preload_param);
  pthread_mutex_unlock(&p_mod->mutex);

  if (IMG_ERROR(rc)) {
    p_mod->last_error = IMG_ERR_GENERAL;
    IDBG_ERROR("IMG_CORE_PRELOAD failed %d", rc);
  } else {
    p_session_data->preload_params.preload_done = true;
    p_mod->preload_done = true;
  }

  IDBG_MED("X rc %d", rc);
  return IMG_SUCCESS;
}
/**
 * Function: module_imgbase_client_preload
 *
 * Description: function called for base client preload
 *
 * Arguments:
 *   @p_session_data - Sesion data
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_imgbase_client_preload(imgbase_session_data_t *p_session_data)
{
  module_imgbase_t *p_mod;
  img_core_type_t thread_affinity[1] = { IMG_CORE_ARM };
  img_thread_job_params_t thread_job;
  uint32_t job_id;
  int32_t rc = IMG_SUCCESS;

  p_mod = (module_imgbase_t *)p_session_data->p_mod;
  p_mod->last_error = IMG_SUCCESS;

  if (!IMG_COMP_PRELOAD_SUPPORTED(&p_mod->core_ops)) {
    p_session_data->preload_params.preload_done = true;
    p_mod->preload_done = true;
    p_mod->th_client_id = 0;
    IDBG_HIGH("[%s] Preload not needed", p_mod->name);
    return IMG_SUCCESS;
  }

  IDBG_MED("Preload needed %dx%d",
    p_session_data->preload_params.param.max_dim.width,
    p_session_data->preload_params.param.max_dim.height);

  /* reserve thread */
  if (!p_mod->th_client_id) {
    p_mod->th_client_id =
      img_thread_mgr_reserve_threads(1, thread_affinity);
  }

  if (!p_mod->th_client_id) {
    IDBG_ERROR("Error reserve thread ");
    rc = IMG_ERR_GENERAL;
    goto end;
  }
  thread_job.client_id = p_mod->th_client_id;
  thread_job.core_affinity = IMG_CORE_ARM;
  thread_job.delete_on_completion = TRUE;
  thread_job.execute = module_imgbase_client_preload_exec;

  thread_job.dep_job_count = 0;
  thread_job.args = p_session_data;
  thread_job.dep_job_ids = NULL;
  job_id = img_thread_mgr_schedule_job(&thread_job);
  if (job_id) {
    IDBG_MED("scheduled job id %x client %x", job_id, p_mod->th_client_id);
  } else {
    IDBG_ERROR("Error cannot schedule job ");
    rc = IMG_ERR_GENERAL;
  }

end:
  return rc;
}

/**
 * Function: module_imgbase_client_forward_ack
 *
 * Description: Function to forward the ack upstream
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *   @p_ack - ack to be forwarded
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
static int module_imgbase_client_forward_ack(imgbase_client_t *p_client,
  isp_buf_divert_ack_t *p_ack)
{
  int rc = IMG_SUCCESS;
  int stream_idx;
  imgbase_stream_t *p_stream;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  if (!p_ack) {
    IDBG_ERROR("%s:%d] Error, null ack ptr %p", __func__, __LINE__,
      p_ack);
    return IMG_ERR_GENERAL;
  }

  stream_idx = module_imgbase_find_stream_by_identity(p_client,
    p_client->divert_identity);
  if (stream_idx < 0) {
    IDBG_ERROR("%s:%d] Cannot find stream mapped to idx %x", __func__,
      __LINE__, p_client->divert_identity);
    return IMG_ERR_GENERAL;
  }
  p_stream = &p_client->stream[stream_idx];

  IDBG_MED("[%s] [IMG_BUF_DBG_IN_f%d_b%d_i%x] buf_ack",
    p_mod->name,
    p_ack->frame_id,
    p_ack->buf_idx,
    p_ack->identity);

  isp_buf_divert_ack_t divert_ack;
  memset(&divert_ack, 0, sizeof(divert_ack));
  divert_ack.buf_idx = p_ack->buf_idx;
  divert_ack.is_buf_dirty = 1;
  divert_ack.identity = p_ack->identity;
  divert_ack.buf_identity = p_ack->buf_identity;
  divert_ack.frame_id = p_ack->frame_id;
  divert_ack.channel_id = p_ack->channel_id;
  divert_ack.meta_data = p_ack->meta_data;
  divert_ack.bayerdata = p_ack->bayerdata;
  divert_ack.buffer_access = p_ack->buffer_access;

  IDBG_HIGH("CAMCACHE : ack buffer_access=%s(0x%x)",
    (p_ack->buffer_access == 0) ? "NONE" :
    (p_ack->buffer_access == CPU_HAS_READ) ? "READ" :
    (p_ack->buffer_access == CPU_HAS_WRITTEN) ? "WRITTEN" :
    "READ_WRITTEN", p_ack->buffer_access);

  IDBG_MED("send buff divert ACK on event identity = 0x%x,"
    "for buf identity = 0x%x", divert_ack.identity,
    divert_ack.buf_identity);
  rc = mod_imgbase_send_event(p_client->divert_identity, TRUE,
    MCT_EVENT_MODULE_BUF_DIVERT_ACK, divert_ack);
  if (!rc) {
    IDBG_ERROR("[%s] Error Sending ACK upstream failed %d",
      p_mod->name, rc);
    return IMG_ERR_GENERAL;
  }

  return IMG_SUCCESS;
}

/**
 * Function: module_imgbase_client_forward_buffer_divert
 *
 * Description: Function to forward the buffer divert downstream
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *   @p_buf_divert - divert to be forwarded
 *   @p_stream - stream info on which to send divert
 *   @is_input_buf - flag to indicate if buffer was
 *     imglib input buffer or imglib output buffer
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
static int module_imgbase_client_forward_buffer_divert(
  imgbase_client_t *p_client, isp_buf_divert_t *p_buf_divert,
  imgbase_stream_t *p_stream, boolean is_input_buf)
{
  int rc = IMG_SUCCESS;
  boolean brc = TRUE;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;

  IDBG_MED("[%s] [IMG_BUF_DBG_%s_f%d_b%d_i%x] forward buf_div to %s",
    p_mod->name,
    is_input_buf ? "IN" : "OUT",
    p_buf_divert->buffer.sequence,
    p_buf_divert->buffer.index,
    p_buf_divert->identity,
    MCT_MODULE_NAME(p_stream->p_srcport));

  /* divert buffer downsteam */
  p_buf_divert->ack_flag = FALSE;
  IDBG_MED("Sending divert on identity 0x%x", p_client->divert_identity);
  brc = mod_imgbase_send_event(p_client->divert_identity, FALSE,
    MCT_EVENT_MODULE_BUF_DIVERT, *p_buf_divert);
  if (!brc) {
    IDBG_ERROR("%s_%s:%d] Error, send divert event failed %d",
      __func__, p_mod->name, __LINE__, brc);
    rc = IMG_ERR_GENERAL;
    goto end;
  }

  /* check if inplace ack was issued for divert */
  if (!p_buf_divert->ack_flag) {
    // Increment pending_ack counter
    pthread_mutex_lock(&p_client->mutex);
    p_stream->frames_pending_for_ack++;
    IDBG_MED("[%s] After DIVERT : frames_pending_for_ack=%d",
      p_mod->name, p_stream->frames_pending_for_ack);
    pthread_mutex_unlock(&p_client->mutex);

    rc = IMG_SUCCESS;
    goto end;
  }

  IDBG_MED("[%s] [IMG_BUF_DBG_%s_f%d_b%d_i%x] inplace buf_ack",
    p_mod->name,
    is_input_buf ? "IN" : "OUT",
    p_buf_divert->buffer.sequence,
    p_buf_divert->buffer.index,
    p_buf_divert->identity);

  if (is_input_buf) {
    /* buffer was imglib input buffer */
    /* generate ACK. Hold if overlap buffers are used. Forward if not */
    rc = module_imgbase_client_handle_buffer_ack(p_client,
      IMG_EVT_ACK_FORCE_RELEASE, p_buf_divert);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s_%s:%d] Error in handling ack", __func__,
        p_mod->name, __LINE__);
    }
  } else {
    /* buffer was imglib output buffer */
    /* if internal buffers are used for the output buf, free it now */
    if (p_mod->caps.use_internal_bufs) {
      isp_buf_divert_ack_t buff_divert_ack;
      memset(&buff_divert_ack, 0x0, sizeof(isp_buf_divert_ack_t));
      buff_divert_ack.buf_idx = p_buf_divert->buffer.index;
      buff_divert_ack.is_buf_dirty = p_buf_divert->is_buf_dirty;
      buff_divert_ack.identity = p_client->divert_identity;
      buff_divert_ack.buf_identity = p_buf_divert->identity;
      buff_divert_ack.frame_id = p_buf_divert->buffer.sequence;
      buff_divert_ack.channel_id = p_buf_divert->channel_id;
      buff_divert_ack.meta_data = p_buf_divert->meta_data;
      buff_divert_ack.buffer_access = p_buf_divert->buffer_access;

      IDBG_HIGH("CAMCACHE : forward buf_divert buffer_access=%s(0x%x)",
        (p_buf_divert->buffer_access == 0) ? "NONE" :
        (p_buf_divert->buffer_access == CPU_HAS_READ) ? "READ" :
        (p_buf_divert->buffer_access == CPU_HAS_WRITTEN) ? "WRITTEN" :
        "READ_WRITTEN", p_buf_divert->buffer_access);

      rc = module_imgbase_client_handle_buffer_ack(p_client,
        IMG_EVT_ACK_FREE_INTERNAL_BUF, &buff_divert_ack);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("[%s] [IMG_BUF_DBG_OUT] Error, internal buf failed %d",
          p_mod->name, rc);
      }
    } else if(p_mod->caps.num_output) {
        struct msm_buf_mngr_info buff_info;
        int32_t cmd;
        int ret;
        IDBG_MED("buf done on ack %x", p_buf_divert->identity);

        buff_info.index = p_buf_divert->buffer.index;
        buff_info.session_id = IMGLIB_SESSIONID(p_buf_divert->identity);
        buff_info.stream_id = IMGLIB_STREAMID(p_buf_divert->identity);
        buff_info.frame_id = p_buf_divert->buffer.sequence;

        IDBG_MED("prepare buf done on identity %x frame_id %d buf_idx %d",
          p_buf_divert->identity,buff_info.frame_id,buff_info.index );

        cmd = (int32_t)VIDIOC_MSM_BUF_MNGR_BUF_DONE;
        ret = ioctl(p_mod->subdevfd, cmd, &buff_info);

        if (ret < 0) {
          IDBG_MED("Failed to do buf_done");
          return IMG_ERR_GENERAL;
        }
    }
  }

  IDBG_MED("[%s] inplace ACK. streamon=%d, curr_frame=%d, ack_frame=%d, "
    "pending_for_ack=%d",
    p_mod->name, p_stream->streamon,
    p_stream->cur_frame_id, p_buf_divert->buffer.sequence,
    p_stream->frames_pending_for_ack);

  // If in case we get an inplace ack for the last frame, inform that flush is done
  if (!p_stream->streamon &&
    (p_stream->cur_frame_id <= p_buf_divert->buffer.sequence) &&
    (p_stream->frames_pending_for_ack == 0)) {
    if (p_mod->modparams.imgbase_client_flush_done) {
      p_mod->modparams.imgbase_client_flush_done(p_client, p_stream);
    }
  }

end:
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
static void module_imgbase_client_handle_buf_done(
  imgbase_client_t *p_client,
  img_frame_t *p_frame)
{
  isp_buf_divert_t *p_buf_divert;
  int rc, i;
  imgbase_buf_t *p_imgbase_buf = NULL;
  module_imgbase_t *p_mod;
  imgbase_stream_t *p_stream = NULL;
  int32_t free_frame = TRUE;
  int stream_idx;
  img_core_type_t thread_affinity[1] = {
    IMG_CORE_ARM,
  };
  int client_id;
  bool release_ack = FALSE;

  if (!p_client || !p_frame) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return;
  }

  p_mod = (module_imgbase_t *)p_client->p_mod;
  if (!p_mod) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return;
  }

  IDBG_MED("%s_%s:%d] bufdone, buffer idx %d, frameid %d, streamon %d",
    __func__, p_mod->name, __LINE__, p_frame->idx, p_frame->frame_id,
    p_client->stream_on);

  stream_idx = module_imgbase_find_stream_by_identity(p_client,
    p_client->divert_identity);
  if (stream_idx < 0) {
    IDBG_ERROR("%s_%s %d] Cannot find stream mapped to idx %x", __func__,
      p_mod->name, __LINE__, p_client->divert_identity);
    goto end;
  }

  p_stream = &p_client->stream[stream_idx];
  if (NULL == p_stream) {
    IDBG_ERROR("%s_%s:%d] Cannot find stream mapped to client %d", __func__,
      p_mod->name, __LINE__, p_client->stream_cnt);
    goto end;
  }

  p_imgbase_buf = (imgbase_buf_t *)p_frame->private_data;
  if (!p_imgbase_buf) {
    IDBG_HIGH("%s:%d] imgbasebuf NULL, dummy frame", __func__, __LINE__);
    goto end;
  }

  p_buf_divert = &p_imgbase_buf->buf_divert;

  IDBG_MED("%s:%d] buffer idx %d port %p %p frame_id %d", __func__, __LINE__,
    p_frame->idx, p_stream->p_sinkport, p_stream->p_srcport,
    p_buf_divert->buffer.sequence);

  if (p_mod->caps.num_overlap) {
    pthread_mutex_lock(&p_client->buf_mutex);
    p_imgbase_buf->dequeued_cnt++;
    PRINT_BUF_LIST(p_client);

    /* Release ACK if it is no longer needed for a batch process */
    if ((p_imgbase_buf->queued_cnt == p_mod->caps.num_input) &&
      (p_imgbase_buf->dequeued_cnt == p_mod->caps.num_input) &&
      (p_imgbase_buf->ack.ack_received_cnt == p_client->num_ack)) {
      release_ack = TRUE;
    }
    pthread_mutex_unlock(&p_client->buf_mutex);

    if (release_ack) {
      rc = module_imgbase_client_handle_buffer_ack(p_client,
        IMG_EVT_ACK_TRY_RELEASE, p_imgbase_buf);
      if (IMG_ERROR(rc)) {
       IDBG_ERROR("%s:%d] Error in releasing ack", __func__, __LINE__);
      }
      goto end;
    }
  }

  /* release buffer to kernel directly on streamoff */
  if (!p_client->stream_on && !p_buf_divert->native_buf) {
    rc = module_imglib_common_release_buffer(p_mod->subdevfd,
      p_stream->identity,
      p_frame->idx,
      p_frame->frame_id,
      FALSE,
      p_frame,
      p_mod->ion_fd,
      p_buf_divert->buffer_access);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] Error in releasing buffer", __func__, __LINE__);
    }
    goto end;
  }

  IDBG_MED("from src port1 %p %d",p_stream->p_srcport,p_mod->caps.num_output);
  if (p_stream->p_srcport && !p_mod->caps.num_output) {
    isp_buf_divert_t lbuf_divert;
    memcpy(&lbuf_divert, p_buf_divert, sizeof(isp_buf_divert_t));
    IDBG_MED("from src port2 %p %d",p_stream->p_srcport,p_mod->caps.num_output);
    if (!p_client->rate_control) { /*FIXME*/
      IDBG_MED("%s:%d] forwarding from src port %x %s", __func__, __LINE__,
        p_buf_divert->identity,p_mod->name);

      /* divert the buffer only if not already forwarded */
      if (p_mod->caps.num_overlap && p_imgbase_buf->divert_done == TRUE) {
        IDBG_HIGH("%s:%d] frame %d was already diverted", __func__,
          __LINE__, p_frame->frame_id);
        goto end;
      }

      img_common_handle_output_frame_cache_op(p_mod->modparams.exec_mode,
        p_frame, p_mod->ion_fd,
        (p_mod->modparams.force_cache_op | p_mod->force_cache_op),
        &lbuf_divert.buffer_access);

      rc = module_imgbase_client_forward_buffer_divert(p_client,
        &lbuf_divert, p_stream, TRUE);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("%s:%d] Error, send divert event from src port failed %d",
          __func__, __LINE__, rc);
      }
      p_imgbase_buf->divert_done = TRUE;
    } else {
      if (!p_client->thread_job.client_id) {
        client_id = img_thread_mgr_reserve_threads(1, thread_affinity);
        if (client_id == 0) {
          IDBG_ERROR("%s:%d] Error reserve thread ", __func__, __LINE__);
          goto end;
        }
        p_client->thread_ops.client_id = client_id;
        IDBG_MED("%s:%d] Reserved thread with client id %d ", __func__,
          __LINE__, p_client->thread_ops.client_id);
      }
      imgbase_client_divert_wrapper_params_t
        *p_src_divert_wrapper_params =
        (imgbase_client_divert_wrapper_params_t *)calloc(1,
        sizeof(imgbase_client_divert_wrapper_params_t));
      if (NULL == p_src_divert_wrapper_params) {
        IDBG_ERROR("%s:%d] Error alloc", __func__, __LINE__);
        goto end;
      }
      p_src_divert_wrapper_params->p_client = p_client;
      p_src_divert_wrapper_params->p_frame = p_frame;
      p_client->thread_job.client_id = p_client->thread_ops.client_id;
      p_client->thread_job.core_affinity = IMG_CORE_ARM;
      p_client->thread_job.delete_on_completion = TRUE;
      p_client->thread_job.execute =
        module_imgbase_client_handle_src_divert_wrapper;
      p_client->thread_job.dep_job_count = 0;
      p_client->thread_job.args = (void *)p_src_divert_wrapper_params;
      p_client->thread_job.dep_job_ids = NULL;

      uint32_t job_id =
        p_client->thread_ops.schedule_job(&p_client->thread_job);
      IDBG_MED("%s:%d] scheduled job id %d", __func__, __LINE__, job_id);
      if (!job_id) {
        IDBG_ERROR("%s:%d] Error schedule job", __func__, __LINE__);
      }
      free_frame = FALSE;
    }
  } else if (p_stream->p_sinkport) {
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
        p_mod->ion_fd,
        p_buf_divert->buffer_access);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("%s:%d] Error in releasing buffer", __func__, __LINE__);
      }
    }
  }

end:
  /* if overlap batch processing, this is freed in handle_overlap_input */
  if (!p_mod->caps.num_overlap && p_imgbase_buf && free_frame) {
    free(p_imgbase_buf);
    p_imgbase_buf = NULL;
  }
}

/**
 * Function: module_imgbase_client_handle_src_divert_wrapper
 *
 * Description: Function to forward divert after it is posted
 *   to the msg thread
 *
 * Arguments:
 *   @p_userdata - Pointer to src divert wrapper params
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
int module_imgbase_client_handle_src_divert_wrapper(
  void *p_userdata)
{
  imgbase_client_divert_wrapper_params_t *p_params =
    (imgbase_client_divert_wrapper_params_t *)p_userdata;
  img_frame_t *p_frame = p_params->p_frame;
  imgbase_client_t *p_client = p_params->p_client;
  imgbase_stream_t *p_stream = NULL;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  uint64_t time_delta;
  int stream_idx;
  isp_buf_divert_t l_buf_divert;
  imgbase_buf_t *p_imgbase_buf = NULL;
  int rc;

  stream_idx = module_imgbase_find_stream_by_identity(p_client,
    p_client->divert_identity);
  if (stream_idx < 0) {
    IDBG_ERROR("%s_%s %d] Cannot find stream mapped to idx %x", __func__,
      p_mod->name, __LINE__, p_client->divert_identity);
    goto end;
  }

  p_stream = &p_client->stream[stream_idx];
  if (NULL == p_stream) {
    IDBG_ERROR("%s_%s %d] Cannot find stream mapped to client", __func__,
      p_mod->name, __LINE__);
    goto end;
  }

  if (!p_client->first_frame) {
    time_delta = IMG_DBG_TIMER_END(p_client->start_time, p_client->end_time,
      p_mod->name, IMG_TIMER_MODE_US);
  } else {
    p_client->first_frame = FALSE;
    time_delta = p_client->exp_frame_delay;
  }

  if (time_delta < p_client->exp_frame_delay) {
    usleep(p_client->exp_frame_delay - time_delta);
  }

  if (p_mod->modparams.imgbase_client_process_done) {
    p_mod->modparams.imgbase_client_process_done(p_client, p_frame);
  }

  p_imgbase_buf = p_frame->private_data;
  l_buf_divert = p_imgbase_buf->buf_divert;

  if (p_mod->modparams.access_mode == IMG_ACCESS_READ_WRITE) {
    img_common_handle_output_frame_cache_op(p_mod->modparams.exec_mode,
      p_frame, p_mod->ion_fd,
      (p_mod->modparams.force_cache_op | p_mod->force_cache_op),
      &l_buf_divert.buffer_access);
  }

  /* divert the buffer only if not already forwarded */
  if (p_mod->caps.num_overlap && p_imgbase_buf->divert_done == TRUE) {
    IDBG_HIGH("%s:%d] frame %d was already diverted", __func__,
      __LINE__, p_frame->frame_id);
    goto end;
  }

  IDBG_HIGH("%s:%d] forward frame %d downstream", __func__,
    __LINE__, p_frame->frame_id);
  p_imgbase_buf->divert_done = TRUE;

  /* divert the buffer downstream */
  rc = module_imgbase_client_forward_buffer_divert(p_client,
    &l_buf_divert, p_stream, TRUE);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] Error, send divert event failed %d",
      __func__, __LINE__, rc);
  }

  /* store new time */
  IMG_DBG_TIMER_START(p_client->start_time);

end:
  /* if overlap batch processing, this is freed in handle_overlap_input */
  if (!p_mod->caps.num_overlap && p_imgbase_buf) {
    free(p_imgbase_buf);
    p_imgbase_buf = NULL;
  }
  free(p_params);
  return IMG_SUCCESS;
}

/**
 * Function: module_imgbase_client_store_frame_id
 *
 * Description: Function to store the frameId of all the input
 *   buffers. The frameIds will be added to the output buffers
 *   released to HAL.
 *
 *   If the num of bufs released to HAL is greater than the num
 *   of input bufs, we need to fabricate frameIds for the extra
 *   buffers to ensure that a 0 frameId zero is not sent to HAL.
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *   @frame_id - frameId of input buf
 *   @input_buf_cnt - input buf count
 *
 * Return values:
 *   inaging error values
 *
 * Notes: none
 **/
static int module_imgbase_client_store_frame_id(imgbase_client_t *p_client,
  uint32_t frame_id, int input_buf_cnt)
{
  int rc = IMG_SUCCESS;
  int extra_cnt = 1;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;

  if (input_buf_cnt >= MAX_IMGLIB_BASE_RELEASED_BUFS) {
    rc = IMG_ERR_INVALID_INPUT;
    goto error;
  }

  /* Store the frameId of input buffers */
  p_client->input_frame_id[input_buf_cnt] = frame_id;
  IDBG_MED("%s:%d] Store frameId %d at idx %d", __func__,
    __LINE__, p_client->input_frame_id[input_buf_cnt], input_buf_cnt);
  input_buf_cnt++;

  /* If num bufs to be released is greater than num input bufs, fabricate
    frameIds; create a frameId which is incrementally greater than the
    last input buffer's frameId */
  if (input_buf_cnt >= p_mod->caps.num_input &&
    input_buf_cnt < p_mod->caps.num_release_buf) {
    while ((input_buf_cnt < p_mod->caps.num_release_buf) &&
      (input_buf_cnt < MAX_IMGLIB_BASE_RELEASED_BUFS)) {
      p_client->input_frame_id[input_buf_cnt] = frame_id + extra_cnt;
      extra_cnt++;
      IDBG_MED("%s:%d] Store frameId %d at idx %d", __func__,
        __LINE__, p_client->input_frame_id[input_buf_cnt], input_buf_cnt);
      input_buf_cnt++;
    }
  }
  return rc;

error:
  return rc;
}

/**
 * Function: module_imgbase_client_retrieve_frame_id
 *
 * Description: Function to retrieve the frameId to associate
 *   with the output buffer which is sent to HAL
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *   @p_frame: frame pointer
 *
 * Return values:
 *   frameId of buffer
 *
 * Notes: none
 **/
static uint32_t module_imgbase_client_retrieve_frame_id(
  imgbase_client_t *p_client,
  img_frame_t *p_frame)
{
  uint32_t frame_id = 0;
  int release_index = 0;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;

  if (p_mod->caps.num_release_buf > p_mod->caps.num_input) {
    /* If the number of released bufs is greater than the number
    of input bufs, set the released bufs frameId to the input buf's
    frameIds and fabricated frameIds */
    release_index = p_client->release_buf_cnt;
    if (release_index >= p_mod->caps.num_release_buf ||
      release_index >= MAX_IMGLIB_BASE_RELEASED_BUFS) {
      IDBG_ERROR("%s:%d] Invalid index %d", __func__, __LINE__, release_index);
      goto error;
    }
    frame_id = p_client->input_frame_id[release_index];
  } else if (!p_frame->frame_id) {
    /* If the number of released bufs is <= the number of input
      bufs, set the released buffer's frameId to the most recently
      received input buf's frameIds */
    release_index = p_mod->caps.num_input -
      p_mod->caps.num_output + p_client->release_buf_cnt;

    if (release_index >= p_mod->caps.num_input ||
      release_index >= MAX_IMGLIB_BASE_RELEASED_BUFS) {
      IDBG_ERROR("%s:%d] Invalid index %d", __func__, __LINE__, release_index);
      goto error;
    }
    frame_id = p_client->input_frame_id[release_index];
  } else {
    frame_id = p_frame->frame_id;
  }

  p_client->release_buf_cnt++;

  /* Ensure that released buf cnt does not exceed the intended
    number of buffers to be released */
  if (p_client->release_buf_cnt >= p_mod->caps.num_release_buf) {
    p_client->release_buf_cnt = 0;
  }

  return frame_id;

error:
  return 0;
}

/**
 * Function: module_imgbase_client_handle_outbuf_done
 *
 * Description: Function to handle output buf done event
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *   @p_frame - frame for buf done
 *   @buf_done - flag to indicate if the buf done needs to be
 *             called
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_imgbase_client_handle_outbuf_done(
  imgbase_client_t *p_client,
  img_frame_t *p_frame,
  int8_t buf_done)
{
  int rc = IMG_SUCCESS;
  uint32_t frame_id = 0;
  imgbase_stream_t *p_stream = NULL;
  isp_buf_divert_t *p_buf_divert;
  int stream_idx;
  imgbase_buf_t *p_imgbase_buf;
  uint32_t identity;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;

  if (!p_frame) {
    IDBG_ERROR("[%s] Error, p_frame null", p_mod->name);
    return;
  }

  /* Retrieve divert info from private data */
  p_imgbase_buf = (imgbase_buf_t *)p_frame->private_data;

  if (!p_imgbase_buf) {
    IDBG_ERROR("[%s] Error, img_base null", p_mod->name);
    return;
  }

  p_buf_divert = &p_imgbase_buf->buf_divert;
  identity = p_imgbase_buf->event_identity ? p_imgbase_buf->event_identity :
    p_buf_divert->identity;
  IDBG_MED("[%s] identity %x E", p_mod->name, identity);

  stream_idx = module_imgbase_find_stream_by_identity(p_client, identity);
  if (stream_idx < 0) {
    IDBG_ERROR("[%s] Cannot find stream mapped to idx %x",
      p_mod->name, identity);
    goto end;
  }

  p_stream = &p_client->stream[stream_idx];

  if (p_stream->p_srcport && p_mod->caps.use_internal_bufs) {
    /* If src port mapped, send op buffer downstream via buf divert or do buffer done*/
    frame_id = p_buf_divert->buffer.sequence;

    if(p_mod->caps.use_internal_bufs && p_buf_divert->ack_flag) {
      isp_buf_divert_ack_t l_buf_divert_ack;
      memset(&l_buf_divert_ack, 0, sizeof(isp_buf_divert_ack_t));
      l_buf_divert_ack.buf_idx = p_buf_divert->buffer.index;
      l_buf_divert_ack.is_buf_dirty = 1;
      l_buf_divert_ack.identity = p_client->divert_identity;
      l_buf_divert_ack.buf_identity = p_buf_divert->identity;
      l_buf_divert_ack.frame_id = p_buf_divert->buffer.sequence;
      l_buf_divert_ack.channel_id = p_buf_divert->channel_id;
      l_buf_divert_ack.meta_data = p_buf_divert->meta_data;
      l_buf_divert_ack.buffer_access = p_buf_divert->buffer_access;
      IDBG_HIGH("[%s] [IMG_BUF_DBG_OUT_f%d_b%d_i%x] queue to buffer to internal queue",
        p_mod->name, frame_id, p_frame->idx, identity,
        p_imgbase_buf->buf_divert.buffer.timestamp.tv_sec
        * 1000000LL + p_imgbase_buf->buf_divert.buffer.timestamp.tv_usec);
      rc = module_imgbase_client_handle_buffer_ack(p_client,
        IMG_EVT_ACK_FREE_INTERNAL_BUF, &l_buf_divert_ack);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("[%s] [IMG_BUF_DBG_OUT] Error, internal buf failed %d",
          p_mod->name, rc);
      }
    } else {
      IDBG_HIGH("[%s] [IMG_BUF_DBG_OUT_f%d_b%d_i%x] send buf_div timestamp %llu",
        p_mod->name, frame_id, p_frame->idx, identity,
        p_imgbase_buf->buf_divert.buffer.timestamp.tv_sec
        * 1000000LL + p_imgbase_buf->buf_divert.buffer.timestamp.tv_usec);

      /* Send divert downstream */
      rc = module_imgbase_client_forward_buffer_divert(p_client,
         p_buf_divert, p_stream, FALSE);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("[%s] [IMG_BUF_DBG_OUT_f%d_b%d_i%x] Error FWD failed %d",
          p_mod->name, frame_id, p_frame->idx, identity, rc);
      }
    }
  } else if (!p_mod->caps.inplace_algo) {
    /* Release output buffer back to kernel */
    frame_id = module_imgbase_client_retrieve_frame_id(p_client, p_frame);
    if (frame_id == 0) {
      rc = IMG_ERR_NOT_FOUND;
      IDBG_ERROR("[%s] [IMG_BUF_DBG_OUT_f%d_b%d_i%x] frameID 0 %d",
        p_mod->name, frame_id, p_frame->idx, identity, rc);
      goto end;
    }

    if(!p_mod->caps.use_internal_bufs) {
      buf_done = (p_imgbase_buf->put_buf == 0) ? TRUE : FALSE;
    }

    mct_event_t event;
    event.identity = p_stream->identity;
    module_imgbase_client_handle_framedrop(p_stream->p_sinkport,
      &event, frame_id, FALSE);
    IDBG_HIGH("[%s] [IMG_BUF_DBG_OUT_f%d_b%d_i%x] release fd %d buf_done %d",
      p_mod->name, frame_id, p_frame->idx, identity,
      p_mod->subdevfd, buf_done);

    rc = module_imglib_common_release_buffer(p_mod->subdevfd,
      p_stream->identity, p_frame->idx, frame_id, buf_done,
      p_frame, p_mod->ion_fd, p_buf_divert->buffer_access);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("[%s] [IMG_BUF_DBG_OUT_f%d_b%d_i%x] Error getting outbuf %d",
        p_mod->name, frame_id, p_frame->idx, identity, rc);
    }
  }

end:
  if (!p_mod->caps.use_internal_bufs && p_imgbase_buf) {
    free(p_imgbase_buf);
    p_imgbase_buf = NULL;
  }

}

/**
 * Function: module_imgbase_client_enqueue_streambuf
 *
 * Description: function to enqueue stream buffer
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *   @p_meta - pointer to the meta
 *   @prop: image properties
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static void module_imgbase_client_enqueue_streambuf(
  imgbase_client_t *p_client,
  img_meta_t *p_meta,
  cam_stream_img_prop_t *prop)
{
  if (!p_client->stream_on) {
    IDBG_LOW("%s:%d] skip create streambuf", __func__, __LINE__);
    return;
  }

  cam_stream_parm_buffer_t *p_stream_buf =
    calloc(1, sizeof(cam_stream_parm_buffer_t));
  if (p_stream_buf) {
    p_stream_buf->imgProp = *prop;
    p_stream_buf->type = CAM_STREAM_PARAM_TYPE_GET_IMG_PROP;
    IDBG_HIGH("%s:%d] (%d %d %d %d) (%d %d) (%d %d)",
      __func__, __LINE__,
      p_stream_buf->imgProp.crop.left,
      p_stream_buf->imgProp.crop.top,
      p_stream_buf->imgProp.crop.width,
      p_stream_buf->imgProp.crop.height,
      p_stream_buf->imgProp.input.width,
      p_stream_buf->imgProp.input.height,
      p_stream_buf->imgProp.output.width,
      p_stream_buf->imgProp.output.height);
    if (IMG_ERROR(img_q_enqueue(&p_client->stream_parm_q, p_stream_buf))) {
      free(p_stream_buf);
      p_stream_buf = NULL;
    }
  }
}

/**
 * Function: module_imgbase_client_event_handler
 *
 * Description: event handler for Imglib base client
 *
 * Arguments:
 *   @p_appdata - IMGLIB_BASE client
 *   @p_event - pointer to the event
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int module_imgbase_client_event_handler(void* p_appdata,
  img_event_t *p_event)
{
  imgbase_client_t *p_client;
  img_component_ops_t *p_comp;
  int rc = IMG_SUCCESS;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  if ((NULL == p_event) || (NULL == p_appdata)) {
    IDBG_ERROR("%s:%d] invalid event", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  p_client = (imgbase_client_t *)p_appdata;
  p_comp = &p_client->comp;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  IDBG_LOW("%s:%d] type %d", __func__, __LINE__, p_event->type);

  p_mod = (module_imgbase_t *)p_client->p_mod;
  if (p_event->type < QIMG_EVT_MAX) {
    if (p_mod->modparams.imgbase_client_event_handler[p_event->type]) {
      rc =
        p_mod->modparams.imgbase_client_event_handler[p_event->type](
        p_client, p_event);
      //If the submodule has over ridden the execution, return
      if (rc != IMG_ERR_EAGAIN) {
        return rc;
      }
    }
  }

  switch (p_event->type) {
  case QIMG_EVT_IMG_BUF_DONE: {
    module_imgbase_client_handle_buf_done(p_client, p_event->d.p_frame);
    break;
  }
  case QIMG_EVT_IMG_OUT_BUF_DONE: {
    module_imgbase_client_handle_outbuf_done(p_client, p_event->d.p_frame,
      TRUE);
    IDBG_LOW("%s:%d] resume QIMG_EVT_IMG_OUT_BUF_DONE %d", __func__, __LINE__,p_mod->caps.num_hold_outbuf);
    if (p_mod->caps.num_hold_outbuf) {
      pthread_mutex_lock(&p_client->buf_mutex);
      img_frame_t *p_out_frame = NULL;
      uint32_t out_buffer_access = 0;

      p_out_frame = img_q_peek(&p_client->output_hold_q);
      if (!p_out_frame) {
        p_client->cur_outbuf_cnt--;
        IDBG_LOW(" cur_outbuf_cnt %d", p_client->cur_outbuf_cnt);
        pthread_mutex_unlock(&p_client->buf_mutex);
      } else {
        pthread_mutex_unlock(&p_client->buf_mutex);
        rc = module_imgbase_client_get_outputbuf(p_client, p_out_frame,
          p_mod->caps.use_internal_bufs, true, &out_buffer_access);
        if (IMG_SUCCEEDED(rc)) {
          /* dequeue the buffer */
          pthread_mutex_lock(&p_client->buf_mutex );
          p_out_frame = img_q_dequeue(&p_client->output_hold_q);
          pthread_mutex_unlock(&p_client->buf_mutex);

          if (!p_out_frame) {
            p_client->cur_outbuf_cnt--;
            IDBG_LOW("peek ok dq fail cur_outbuf_cnt %d", p_client->cur_outbuf_cnt);
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
               IDBG_LOW(" Success! frame id %d ", p_out_frame->frame_id);
              break; //SUCCESS
            } else {
              p_client->cur_outbuf_cnt--;
              if (p_out_frame->private_data) {
                free(p_out_frame->private_data);
              }
              IDBG_ERROR("Error, failed to qbuf %d", rc);
            }
          }
        } else {
          p_client->output_hold_q_buf_pending_count++;
          IDBG_LOW(" Will retry when buffer available pending %d", p_client->output_hold_q_buf_pending_count);
        }
        pthread_mutex_lock(&p_client->buf_mutex);
        IDBG_LOW(" cur_outbuf_cnt %d", p_client->cur_outbuf_cnt);
        pthread_mutex_unlock(&p_client->buf_mutex);
      }
    }

    break;
  }
  case QIMG_EVT_META_BUF_DONE: {
    img_meta_t *p_meta = p_event->d.p_meta;
    IDBG_MED("%s:%d] QIMG_EVT_META_BUF_DONE p_meta %p",
      __func__, __LINE__, p_meta);

    p_client->num_meta_queued--;

    if (p_meta) {
      /* Only enqueue the last metadata buf since the relevant
        information is only available within this */
      if (p_mod->caps.is_offline_proc && p_client->num_meta_queued == 0) {
        cam_stream_img_prop_t prop;
        memset(&prop, 0x0, sizeof(cam_stream_img_prop_t));
        cam_rect_t *p_rect = &prop.crop;
        img_rect_t *output_crop = (img_rect_t *)img_get_meta(p_meta,
          IMG_META_OUTPUT_ROI);
        if (output_crop) {
          p_rect->left = (int32_t)output_crop->pos.x;
          p_rect->top = (int32_t)output_crop->pos.y;
          p_rect->width = (int32_t)output_crop->size.width;
          p_rect->height = (int32_t)output_crop->size.height;
        }

        img_misc_t *misc_data = (img_misc_t *)img_get_meta(p_meta,
          IMG_META_MISC_DATA);
        if (p_client->p_current_misc_data && misc_data) {
          p_client->p_current_misc_data->result = misc_data->result;
          p_client->p_current_misc_data->header_size = misc_data->header_size;
          p_client->p_current_misc_data->width = misc_data->width;
          p_client->p_current_misc_data->height = misc_data->height;
        }
        if (output_crop || misc_data) {
          module_imgbase_client_enqueue_streambuf(p_client, p_meta, &prop);
        }
      }
      free(p_event->d.p_meta);
      p_meta = NULL;
    }
    p_client->p_current_meta = NULL;
    break;
  }
  case QIMG_EVT_ERROR: {
    IDBG_HIGH("%s %d: IMGLIB_BASE Error", __func__, __LINE__);
    bool error_handled = FALSE;

    switch (p_event->d.status) {
    case IMG_ERR_SSR:
      IDBG_HIGH("%s:%d] SSR occured", __func__, __LINE__);
      if (p_mod->modparams.imgbase_client_handle_ssr) {
        p_mod->modparams.imgbase_client_handle_ssr(p_client);
        error_handled = TRUE;
      } else {
        IDBG_HIGH("%s:%d] Error, SSR is not handled", __func__, __LINE__ );
      }
      break;
    default:
      break;
    }

    if (!error_handled) {
      /* flush any outstanding acks */
      if (p_mod->caps.num_overlap) {
        rc = module_imgbase_client_handle_buffer_ack(p_client,
          IMG_EVT_ACK_FLUSH, NULL);
        if (IMG_ERROR(rc)) {
          IDBG_ERROR("%s:%d] Error in flushing acks", __func__, __LINE__);
        }
      }
      p_client->state = IMGLIB_STATE_ERROR;
    }
    break;
  }
  default:
    break;
  }
  IDBG_LOW("%s:%d] type %d X", __func__, __LINE__, p_event->type);
  return rc;
}

/**
 * Function: module_imgbase_client_extract_isp_stream_info
 *
 * Description: This function is exercised if the module is
 *           before cpp, in which case the frame dimensions,
 *           padding, and offset need to be extracted from isp's
 *           output dimension
 * Arguments:
 *   @p_client: imgbase client
 *   @pframe: frame pointer
 *   @stride: frame's stride
 *   @scanline: frame's scanline
 *   @offset: frame's offset
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void module_imgbase_client_get_isp_stream_info(
  imgbase_client_t *p_client, img_frame_t *pframe, int *stride,
  int *scanline, int *offset)
{
  int dis_x;
  int dis_y;
  int dis_width;
  int dis_height;
  int crop_x;
  int crop_y;
  int crop_width;
  int crop_height;
  int start_x;
  int start_y;

  if (p_client->dis_enable && p_client->is_update_valid) {
    dis_width = p_client->is_update.width;
    dis_height = p_client->is_update.height;
    dis_x = p_client->is_update.x;
    dis_y = p_client->is_update.y;
  } else {
    dis_width = p_client->isp_output_dim_stream_info.dim.width;
    dis_height = p_client->isp_output_dim_stream_info.dim.height;
    dis_x = 0;
    dis_y = 0;
  }

  if (p_client->stream_crop_valid && p_client->stream_crop.x &&
      p_client->stream_crop.y && p_client->stream_crop.crop_out_x &&
      p_client->stream_crop.crop_out_y && p_client->crop_required) {
    crop_width = p_client->stream_crop.crop_out_x;
    crop_height = p_client->stream_crop.crop_out_y;
    crop_x = p_client->stream_crop.x;
    crop_y = p_client->stream_crop.y;
  } else {
    crop_width = p_client->isp_output_dim_stream_info.dim.width;
    crop_height = p_client->isp_output_dim_stream_info.dim.height;
    crop_x = 0;
    crop_y = 0;
  }

  pframe->info.width = crop_width * dis_width /
      p_client->isp_output_dim_stream_info.dim.width;
  pframe->info.height = crop_height * dis_height /
      p_client->isp_output_dim_stream_info.dim.height;
  module_imglib_common_get_frame_fmt(
    p_client->isp_output_dim_stream_info.fmt,
    &pframe->info.fmt);

  *stride =
    p_client->isp_output_dim_stream_info.buf_planes.plane_info.mp[0].stride;
  *scanline =
    p_client->isp_output_dim_stream_info.buf_planes.plane_info.mp[0].scanline;

  start_x = dis_x + (crop_x * dis_width /
      p_client->isp_output_dim_stream_info.dim.width);
  start_y = dis_y + (crop_y * dis_height /
      p_client->isp_output_dim_stream_info.dim.height);

  *offset = start_y * (*stride) + start_x;
}

/**
 * Function: module_imgbase_client_getbuf
 *
 * Description: This function is to fetching the input buffer
 *           based in buffer info
 * Arguments:
 *   @p_client: imgbase client
 *   @p_buf_divert: ISP buffer divert event structure
 *   @pframe: frame pointer
 *   @native_buf: flag to indicate if its a native buffer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_imgbase_client_getbuf(imgbase_client_t *p_client,
  isp_buf_divert_t *p_buf_divert,
  img_frame_t *pframe,
  int native_buf)
{
  int rc = IMG_SUCCESS;
  uint32_t i = 0;
  uint32_t buf_idx;
  uint32_t size;
  uint8_t *p_addr = NULL;
  mct_module_t *p_mct_mod;
  uint32_t padded_size;
  int fd = -1;
  int stride = 0;
  int scanline = 0;
  int offset = 0;
  imgbase_stream_t *p_stream = NULL;
  mct_stream_map_buf_t *p_buf_holder;
  int stream_idx;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  int32_t *p_rotation = (int32_t *)img_get_meta(&p_client->current_meta,
    IMG_META_ROTATION);
  int32_t rotation;

  if (!p_rotation) {
    IDBG_HIGH("%s:%d] Rotation not set", __func__, __LINE__);
    rotation = 0;
  } else {
    rotation = *p_rotation;
  }

  stream_idx = module_imgbase_find_stream_by_identity(p_client,
    p_client->divert_identity);
  if (stream_idx < 0) {
    IDBG_ERROR("%s_%s %d] Cannot find stream mapped to idx %x", __func__,
      p_mod->name, __LINE__, p_client->divert_identity);
    return IMG_ERR_GENERAL;
  }
  p_stream = &p_client->stream[stream_idx];
  if (NULL == p_stream) {
    IDBG_ERROR("%s_%s %d] Cannot find stream mapped to client", __func__,
      p_mod->name, __LINE__);
    return IMG_ERR_GENERAL;
  }

  if (NULL == p_stream->p_sinkport) {
    IDBG_ERROR("%s:%d] NULL Sink port", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_LOW("mod %s is before cpp %d, client %0x native %d",
    p_mod->name, p_client->before_cpp, p_client, native_buf);
  if (p_client->before_cpp) {
    IDBG_LOW("%s:%d] mod %s is before cpp", __func__, __LINE__, p_mod->name);
    if (!p_client->isp_output_dim_stream_info_valid) {
      IDBG_ERROR("%s:%d] ISP output dim info required", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }

    module_imgbase_client_get_isp_stream_info(p_client, pframe,
      &stride, &scanline, &offset);
  } else {
    IDBG_HIGH("%s:%d] rotation %d", __func__, __LINE__, rotation);
    if (CAM_STREAM_TYPE_OFFLINE_PROC == p_stream->stream_info->stream_type) {
      if ((0 == rotation) || (180 == rotation)) {
        pframe->info.width = (uint32_t)p_stream->stream_info->dim.width;
        pframe->info.height = (uint32_t)p_stream->stream_info->dim.height;
        stride = p_stream->stream_info->buf_planes.plane_info.mp[0].stride;
        scanline = p_stream->stream_info->buf_planes.plane_info.mp[0].scanline;
      } else {
        pframe->info.width = (uint32_t)p_stream->stream_info->dim.height;
        pframe->info.height = (uint32_t)p_stream->stream_info->dim.width;
        stride = p_stream->stream_info->buf_planes.plane_info.mp[0].scanline;
        scanline = p_stream->stream_info->buf_planes.plane_info.mp[0].stride;
      }
    } else {
      pframe->info.width = (uint32_t)p_stream->isp_stream_info.dim.width;
      pframe->info.height = (uint32_t)p_stream->isp_stream_info.dim.height;
      stride = p_stream->isp_stream_info.buf_planes.plane_info.mp[0].stride;
      scanline = p_stream->isp_stream_info.buf_planes.plane_info.mp[0].scanline;
    }

    module_imglib_common_get_frame_fmt(
      p_stream->stream_info->fmt,
      &pframe->info.fmt);
    offset = 0;
  }

  if ((CAM_STREAM_TYPE_OFFLINE_PROC == p_stream->stream_info->stream_type) &&
    ((CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG ==
    p_stream->stream_info->fmt) ||
    (CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG ==
    p_stream->stream_info->fmt) ||
    (CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB ==
    p_stream->stream_info->fmt) ||
    (CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR ==
    p_stream->stream_info->fmt))) {
    pframe->frame[0].plane_cnt = 1;
    size = QIMG_CALC_10BPP_PACKED_SIZE(pframe->info.width, pframe->info.height);
    padded_size = QIMG_CALC_10BPP_PACKED_SIZE(stride, scanline);
  } else if ((CAM_STREAM_TYPE_DEPTH == p_stream->stream_info->stream_type) &&
    ((CAM_FORMAT_DEPTH16 == p_stream->stream_info->fmt) ||
    (CAM_FORMAT_DEPTH8 == p_stream->stream_info->fmt) ||
    (CAM_FORMAT_DEPTH_POINT_CLOUD == p_stream->stream_info->fmt))) {
    pframe->frame[0].plane_cnt = 1;
    size = pframe->info.width * pframe->info.height;
    padded_size = (uint32_t)stride * (uint32_t)scanline;
    IDBG_HIGH("plane_cnt %d, size %d, padded_size %d",
      pframe->frame[0].plane_cnt, size, padded_size);
  } else {
    pframe->frame[0].plane_cnt = 2;
    size = pframe->info.width * pframe->info.height;
    padded_size = (uint32_t)stride * (uint32_t)scanline;
  }
  pframe->frame_cnt = 1;
  if (p_buf_divert->bayerdata) {
    pframe->frame[0].plane_cnt = 1;
  }

  pframe->idx = buf_idx = p_buf_divert->buffer.index;
  pframe->frame_id = p_buf_divert->buffer.sequence;
  pframe->timestamp = (uint64_t)p_buf_divert->buffer.timestamp.tv_sec
    * 1000000LL + (uint64_t)p_buf_divert->buffer.timestamp.tv_usec;
  p_stream->cur_frame_id = pframe->frame_id;

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(p_stream->p_sinkport))->data);
  IDBG_MED("[%s] [IMG_BUF_DBG_IN_f%d_b%d_i%x] Dimension %dx%d, "
    " mod %p, port %p, pproc %p, pad %dx%d, offset %d time %llu",
    p_mod->name,
    p_buf_divert->buffer.sequence,
    buf_idx,
    p_stream->identity,
    pframe->info.width, pframe->info.height,
    p_mct_mod,
    p_stream->p_sinkport,
    p_client->parent_mod,
    stride,
    scanline,
    offset,
    pframe->timestamp);

  if (!native_buf) {
    p_buf_holder = mct_module_get_buffer(buf_idx,
      (p_client->parent_mod) ? p_client->parent_mod : p_mct_mod,
      IMGLIB_SESSIONID(p_stream->identity),
      IMGLIB_STREAMID(p_stream->identity));
    if (!p_buf_holder) {
      IDBG_ERROR("%s:%d] Buffer with id %d from stream 0x%x is not found",
        __func__, __LINE__, buf_idx, p_stream->identity);
      return IMG_ERR_INVALID_INPUT;
    }
    p_addr = p_buf_holder->buf_planes[0].buf;
    fd = p_buf_holder->buf_planes[0].fd;
  } else {

    p_addr = (uint8_t*)p_buf_divert->plane_vaddr[0];

    fd = p_buf_divert->fd;

    IDBG_MED("%s:%d] Native Buffer addr = %p, fd = %d",
     __func__, __LINE__, p_addr, fd);
  }

  if (NULL == p_addr) {
    IDBG_ERROR("%s:%d] NULL address", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  for (i = 0; i < pframe->frame[0].plane_cnt; i++) {
    pframe->frame[0].plane[i].fd = fd;
    pframe->frame[0].plane[i].offset = offset;
    if (i == 0) { /* Y plane */
      pframe->frame[0].plane[i].plane_type = p_client->plane_type[0];
      pframe->frame[0].plane[i].addr = p_addr;
      pframe->frame[0].plane[i].width = pframe->info.width;
      pframe->frame[0].plane[i].height = pframe->info.height;
      pframe->frame[0].plane[i].stride = (uint32_t)stride;
      pframe->frame[0].plane[i].scanline = (uint32_t)scanline;
      pframe->frame[0].plane[i].fd = fd;
    } else { /* Chroma plane */
      pframe->frame[0].plane[i].addr = p_addr + padded_size;
      pframe->frame[0].plane[i].width = pframe->info.width;
      pframe->frame[0].plane[i].height = pframe->info.height/2;
      pframe->frame[0].plane[i].stride = (uint32_t)stride;
      pframe->frame[0].plane[i].scanline = (uint32_t)scanline/2;
    }
    if (native_buf) {
      if ((CAM_STREAM_TYPE_OFFLINE_PROC ==
        p_stream->stream_info->stream_type) &&
        ((CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG ==
        p_stream->stream_info->fmt) ||
        (CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG ==
        p_stream->stream_info->fmt) ||
        (CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB ==
        p_stream->stream_info->fmt) ||
        (CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR ==
        p_stream->stream_info->fmt))) {
        pframe->frame[0].plane[i].length =
          QIMG_CALC_10BPP_PACKED_SIZE(pframe->frame[0].plane[i].stride,
          pframe->frame[0].plane[i].scanline);
        pframe->frame[0].plane[i].plane_type = PLANE_BAYER_MIPI_RAW_10BPP;

      } else {
        pframe->frame[0].plane[i].length = pframe->frame[0].plane[i].stride *
          pframe->frame[0].plane[i].scanline;
      }
    } else {
      pframe->frame[0].plane[i].length =
        p_stream->stream_info->buf_planes.plane_info.mp[i].len;
    }
  }
  return rc;
}

/**
 * Function: module_imgbase_client_fetch_outbuf
 *
 * Description: This function is to fetch outbuf buffer
 *    and update the buffer holder
 *
 * Arguments:
 *   @p_client: imgbase client
 *   @pframe: frame pointer
 *   @internal_buf: flag to indicate whether internal buf is
 *     used
 *   @p_buffer_access: buffer access pointer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int32_t module_imgbase_client_fetch_outbuf(imgbase_client_t *p_client,
  img_frame_t *pframe,
  bool internal_buf,
  uint32_t *p_buffer_access,
  uint32_t requested_buf)
{
  uint8_t *p_addr;
  int fd = -1;
  uint32_t buf_idx;
  int32_t out_idx;
  uint32_t i = 0;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  imgbase_buf_t *p_imgbase_buf = (imgbase_buf_t *)pframe->private_data;
  img_internal_bufnode_t *p_intbuf;
  mct_stream_map_buf_t *p_buf_holder;
  isp_buf_divert_t *p_buf_divert = &p_imgbase_buf->buf_divert;
  uint32_t identity = p_imgbase_buf->event_identity ?
    p_imgbase_buf->event_identity : p_buf_divert->identity;

  IDBG_MED("[%s] identity %x internal %d %d", p_mod->name, identity,
    p_mod->caps.use_internal_bufs, internal_buf);
  if (!internal_buf) {
    out_idx = module_imglib_common_get_buffer(p_mod->subdevfd, identity,
      p_buffer_access, pframe->req_buf_idx);
    if (out_idx < 0) {
      IDBG_ERROR("[%s] cannot get buffer id %x", p_mod->name, identity);
      return IMG_ERR_GENERAL;
    }

    buf_idx = (uint32_t)out_idx;
    p_buf_holder = mct_module_get_buffer(buf_idx,
      (p_client->parent_mod) ? p_client->parent_mod : p_mod->p_mct_mod,
      IMGLIB_SESSIONID(identity),
      IMGLIB_STREAMID(identity));

    if (NULL == p_buf_holder) {
      IDBG_ERROR("[%s] NULL holder id %x", p_mod->name, identity);
      return IMG_ERR_GENERAL;
    }
    p_addr = p_buf_holder->buf_planes[0].buf;
    fd = p_buf_holder->buf_planes[0].fd;
  } else { /* get output buffer - internal */
    /* Use the preallocated internal buffers for output */
    QIMG_LOCK(&p_client->buf_mutex);

    p_intbuf = NULL;
    /* Find unused internal buffer */
    for (i = 0; i < p_client->internal_buf.buf_cnt; i++) {
      if ((p_client->internal_buf.buf[i].ref_cnt == 0) &&
        (p_client->internal_buf.buf[i].is_init == TRUE)) {
        p_intbuf = &p_client->internal_buf.buf[i];
        break;
      }
    }
    if (!p_intbuf) {
      IDBG_ERROR("[%s] [IMG_BUF_DBG_OUT] cannot find buffer", p_mod->name);
      QIMG_UNLOCK(&p_client->buf_mutex);
      return IMG_ERR_GENERAL;
    }
    p_intbuf->p_frame = pframe;
    p_intbuf->ref_cnt++;
    p_intbuf->input_ack_held = TRUE;
    p_addr = p_intbuf->mem_handle.vaddr;
    fd = p_intbuf->mem_handle.fd;
    buf_idx = i;
    QIMG_UNLOCK(&p_client->buf_mutex);
    *p_buffer_access = p_intbuf->exists_in_cache ? CPU_HAS_READ : 0;
  }

  pframe->idx = buf_idx;
  for (i = 0; i < pframe->frame[0].plane_cnt; i++) {
    QIMG_FD(pframe, i) = fd;
    QIMG_ADDR(pframe, i) = (uint8_t *)(p_addr + ((i > 0) ?
      QIMG_LEN(pframe, i - 1) : 0));
    /* update buf divert */
    p_buf_divert->plane_vaddr[i] = (uint32_t)QIMG_ADDR(pframe, i);
    IDBG_LOW("[%s] plane[%d] va %p", p_mod->name, i, QIMG_ADDR(pframe, i));
    IDBG_LOW("[%s] plane[%d] fd %d", p_mod->name, i, QIMG_FD(pframe, i));
    IDBG_LOW("[%s] plane[%d] W %d" , p_mod->name, i, QIMG_WIDTH(pframe, i));
    IDBG_LOW("[%s] plane[%d] H %d" , p_mod->name, i, QIMG_HEIGHT(pframe, i));
    IDBG_LOW("[%s] plane[%d] St %d", p_mod->name, i, QIMG_STRIDE(pframe, i));
    IDBG_LOW("[%s] plane[%d] Sc %d", p_mod->name, i, QIMG_SCANLINE(pframe, i));
    IDBG_LOW("[%s] plane[%d] L %d" , p_mod->name, i, QIMG_LEN(pframe, i));
  }
  p_buf_divert->fd = fd;
  p_buf_divert->vaddr = (void *)p_buf_divert->plane_vaddr[0];
  p_buf_divert->buffer.sequence = pframe->frame_id;
  p_buf_divert->buffer.index = pframe->idx;

  IDBG_MED("[%s] [IMG_BUF_DBG_OUT_f%d_b%d_i%x] Dimension %dx%d "
    " mod %p %p pad %dx%d len %d bc %d",
    p_mod->name,
    pframe->frame_id,
    pframe->idx,
    identity,
    QIMG_WIDTH(pframe, 0),
    QIMG_HEIGHT(pframe, 0),
    p_mod->p_mct_mod,
    p_client->parent_mod,
    QIMG_STRIDE(pframe, 0),
    QIMG_SCANLINE(pframe, 0),
    QIMG_LEN(pframe, 0),
    p_client->before_cpp);

  IDBG_HIGH("[%s] : buf_index=%d, fd=%d, vaddr=%p",
    p_mod->name, pframe->idx, p_buf_divert->fd, p_buf_divert->vaddr);

  return IMG_SUCCESS;
}

/**
 * Function: module_imgbase_client_get_outputbuf
 *
 * Description: This function is to fetching the output buffer
 *           based in buffer info
 * Arguments:
 *   @p_client: imgbase client
 *   @pframe: frame pointer
 *   @internal_buf: flag to indicate whether internal buf is
 *                used
 *   @fetch_buf: Flag to indicate whether the buffer needs to be
 *             fetched. if flag is false, the only the buffer
 *             holder will be returned
 *   @p_buffer_access: buffer access pointer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_get_outputbuf(imgbase_client_t *p_client,
  img_frame_t *pframe,
  bool internal_buf,
  bool fetch_buf,
  uint32_t *p_buffer_access)
{
  int rc = IMG_SUCCESS;
  uint32_t size;
  uint32_t i;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  imgbase_stream_t *p_stream = NULL;
  int stream_idx;
  int32_t *p_rotation;
  int32_t  rotation;
  cam_mp_len_offset_t *p_mp;
  mct_stream_info_t *p_stream_info;
  cam_dimension_t *p_dim;
  bool swap_dim;
  uint32_t l_width, l_height, l_stride, l_scanline;
  double v_scale_factor[MAX_PLANE_CNT] = {0};
  imgbase_buf_t *p_imgbase_buf = (imgbase_buf_t *)pframe->private_data;
  isp_buf_divert_t *p_buf_divert = &p_imgbase_buf->buf_divert;

  IDBG_LOW("[%s] Enter", p_mod->name);

  stream_idx = module_imgbase_find_stream_by_identity(p_client,
    p_client->divert_identity);
  if (stream_idx < 0) {
    IDBG_ERROR("[%s] Cannot find stream mapped to idx %x", p_mod->name,
      p_client->divert_identity);
    goto error;
  }

  p_rotation = (int32_t *)img_get_meta(&p_client->current_meta,
    IMG_META_ROTATION);

  if (!p_rotation) {
    IDBG_HIGH("%s:%d] Rotation not set", __func__, __LINE__);
    rotation = 0;
  } else {
    rotation = *p_rotation;
  }

  p_stream = &p_client->stream[stream_idx];

  if (p_client->before_cpp) {
    if (p_client->is_dim_change) {
      p_stream_info = p_stream->stream_info;
    } else {
      p_stream_info = &p_stream->isp_stream_info;
    }
  } else {
    p_stream_info = p_stream->stream_info;
  }

  p_mp = &p_stream_info->buf_planes.plane_info.mp[0];
  p_dim = &p_stream_info->dim;

  IDBG_LOW("Conneted to stream_type %d", p_stream->stream_info->stream_type);

  swap_dim =
    ((CAM_STREAM_TYPE_OFFLINE_PROC == p_stream->stream_info->stream_type) &&
    ((90 == rotation) || (270 == rotation)));

  IDBG_LOW("bufseq %d",p_buf_divert->buffer.sequence);
  pframe->frame_id = p_buf_divert->buffer.sequence;
  pframe->frame_cnt = 1;
  size = pframe->info.width * pframe->info.height;

  /* swap the dimensions */
  if (!swap_dim) {
    l_width    = p_dim->width;
    l_height   = p_dim->height;
    l_stride   = p_mp->stride;
    l_scanline = p_mp->scanline;
  } else {
    l_height   = p_dim->width;
    l_width    = p_dim->height;
    l_scanline = p_mp->stride;
    l_stride   = p_mp->scanline;
  }

  IDBG_LOW("[%s] Enter format %d, ispfmt %d strfmt %d ", p_mod->name,
    p_stream_info->fmt,
    p_stream->isp_stream_info.fmt, p_stream->stream_info->fmt );

  switch (p_stream_info->fmt) {
  case CAM_FORMAT_YUV_420_NV12:
  case CAM_FORMAT_YUV_420_NV21:
  case CAM_FORMAT_YUV_420_NV21_ADRENO:
  case CAM_FORMAT_YUV_420_NV12_VENUS:
  case CAM_FORMAT_YUV_420_NV12_UBWC:
  case CAM_FORMAT_YUV_420_NV21_VENUS:
    pframe->frame[0].plane_cnt = 2;
    pframe->info.ss = IMG_H2V2;
    v_scale_factor[0] = 1.0f;
    v_scale_factor[1] = 0.5f;
    break;

  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
    pframe->frame[0].plane_cnt = 1;
    v_scale_factor[0] = 1.0f;
    pframe->frame[0].plane[0].plane_type =
      PLANE_BAYER_IDEAL_RAW_PLAIN16_12BPP;
    break;
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
    pframe->frame[0].plane_cnt = 1;
    v_scale_factor[0] = 1.0f;
    pframe->frame[0].plane[0].plane_type =
      PLANE_BAYER_MIPI_RAW_10BPP;
    break;
  case CAM_FORMAT_DEPTH16:
  case CAM_FORMAT_DEPTH8:
  case CAM_FORMAT_DEPTH_POINT_CLOUD:
    pframe->frame[0].plane_cnt = 1;
    v_scale_factor[0] = 1.0f;
    pframe->frame[0].plane[0].plane_type = PLANE_Y;
    break;
  default:
    IDBG_ERROR("[%s] Not supported format %d", p_mod->name,
      p_stream_info->fmt);
    goto error;
  }

  for (i = 0; i < pframe->frame[0].plane_cnt; i++) {
    p_mp = &p_stream_info->buf_planes.plane_info.mp[i];
    pframe->frame[0].plane[i].offset = 0;
    QIMG_WIDTH(pframe, i)    = l_width;
    QIMG_HEIGHT(pframe, i)   = l_height * v_scale_factor[i];
    QIMG_STRIDE(pframe, i)   = l_stride;
    QIMG_SCANLINE(pframe, i) = l_scanline * v_scale_factor[i];

    if (pframe->frame[0].plane[i].plane_type ==
      PLANE_BAYER_IDEAL_RAW_PLAIN16_12BPP) {
      QIMG_LEN(pframe, i) =
        QIMG_CALC_PLAIN16_UNPACKED_SIZE(l_stride, l_scanline);
    } else {
      QIMG_LEN(pframe, i) = p_mp->len;
    }
    QIMG_FD(pframe, i)       = -1;
    QIMG_ADDR(pframe, i)     = NULL;
  }
  pframe->info.width  = QIMG_WIDTH(pframe, 0);
  pframe->info.height = QIMG_HEIGHT(pframe, 0);
  module_imglib_common_get_frame_fmt(p_stream_info->fmt,
    &pframe->info.fmt);

  if (fetch_buf) {
    IDBG_MED("[%s] fetch buffer frame_id %d req_buf_idx %d %x", p_mod->name,
      pframe->frame_id,pframe->req_buf_idx,p_client->divert_identity);
    rc = module_imgbase_client_fetch_outbuf(p_client, pframe, internal_buf,
      p_buffer_access, pframe->req_buf_idx);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("[%s] fetch buffer failed rc %d", p_mod->name, rc);
    }
  }
  return rc;

error:
  IDBG_ERROR("[%s] Cannot get output buffer rc %d", p_mod->name, rc);
  return rc;
}

/**
 * Function: module_imgbase_client_handle_offline_meta
 *
 * Description: This function is used to update the
 *   offline metadata
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @p_meta: meta pointer
 *   @current_frame_id: current frame id
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_imgbase_client_update_offline_meta(
  imgbase_client_t *p_client,
  img_meta_t *p_meta,
  uint32_t current_frame_id)
{
  int rc = IMG_SUCCESS;
  img_aec_info_t *p_aec_info;
  img_awb_info_t *p_awb_info;
  img_meta_t *p_frame_meta;

  /* retreive per-frame meta */
  rc = module_imglib_common_meta_get_by_frameid(current_frame_id,
    &p_client->meta_data_list, &p_frame_meta);

  p_aec_info = (img_aec_info_t *)img_get_meta(p_frame_meta,
    IMG_META_AEC_INFO);
  if (p_aec_info) {
    rc = img_set_meta(p_meta, IMG_META_AEC_INFO, p_aec_info);
    if (IMG_ERROR(rc)) {
      IDBG_WARN("AEC get rc %d", rc);
    }
  }
  p_awb_info = (img_awb_info_t *)img_get_meta(p_frame_meta,
    IMG_META_AWB_INFO);
  if (p_awb_info) {
    rc = img_set_meta(p_meta, IMG_META_AWB_INFO, p_awb_info);
    if (IMG_ERROR(rc)) {
      IDBG_WARN("AWB get rc %d", rc);
    }
  }
  return IMG_SUCCESS;
}

/**
 * Function: module_imgbase_client_update_session_meta
 *
 * Description: This function is used to update the
 *   session metadata into current meta data
 *
 * Arguments:
 *   @p_in_meta: input session meta pointer
 *   @p_out_meta: output meta pointer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_imgbase_client_update_session_meta(
  img_meta_t *p_in_meta,
  img_meta_t *p_out_meta)
{
  int rc = IMG_SUCCESS;
  img_aec_info_t *p_aec_info;
  img_awb_info_t *p_awb_info;
  void *p_gamma;

  p_aec_info = (img_aec_info_t *)img_get_meta(p_in_meta,
    IMG_META_AEC_INFO);
  if (p_aec_info) {
    rc = img_set_meta(p_out_meta, IMG_META_AEC_INFO, p_aec_info);
    if (IMG_ERROR(rc)) {
      IDBG_WARN("AEC set failed rc %d", rc);
    }
  }
  p_awb_info = (img_awb_info_t *)img_get_meta(p_in_meta,
    IMG_META_AWB_INFO);
  if (p_awb_info) {
    rc = img_set_meta(p_out_meta, IMG_META_AWB_INFO, p_awb_info);
    if (IMG_ERROR(rc)) {
      IDBG_WARN("AWB set failed rc %d", rc);
    }
  }

  p_gamma = img_get_meta(p_in_meta, IMG_META_R_GAMMA);
  if (p_gamma) {
    rc = img_set_meta(p_out_meta, IMG_META_R_GAMMA, p_gamma);
    if (IMG_ERROR(rc)) {
      IDBG_WARN("R_GAMMA set failed rc %d", rc);
    }
  }

  p_gamma = img_get_meta(p_in_meta, IMG_META_G_GAMMA);
  if (p_gamma) {
    rc = img_set_meta(p_out_meta, IMG_META_G_GAMMA, p_gamma);
    if (IMG_ERROR(rc)) {
      IDBG_WARN("G_GAMMA set failed rc %d", rc);
    }
  }

  p_gamma = img_get_meta(p_in_meta, IMG_META_B_GAMMA);
  if (p_gamma) {
    rc = img_set_meta(p_out_meta, IMG_META_B_GAMMA, p_gamma);
    if (IMG_ERROR(rc)) {
      IDBG_WARN("B_GAMMA set failed rc %d", rc);
    }
  }

  return IMG_SUCCESS;
}

static boolean module_imgbase_client_update_frameid_frameconfigq_traverse(void* qdata,
  void* userdata)
{
  if (!qdata || !userdata) {
    IDBG_ERROR("Invalid qdata %p userdata %p",
      qdata, userdata);
    return FALSE;
  }
  imgbase_frame_config_t *frame_ctrl_data = (imgbase_frame_config_t *)qdata;
  void** input = (void**)userdata;
  uint32_t tardy = *(uint32_t*) input[0];
  uint32_t remap_frame_id = *(uint32_t*) input[1];

  if (frame_ctrl_data->frame_id >= remap_frame_id) {
    frame_ctrl_data->frame_id += tardy;
  }

  return TRUE;
}


static int module_imgbase_client_update_perframe_ctrlq
  (imgbase_session_data_t *p_session_data, void * input){

  if (!p_session_data) {
    IDBG_ERROR("invalid input, p_session_data=%p", p_session_data);
    return IMG_ERR_INVALID_INPUT;
  }
  uint32_t is_master = (p_session_data->multi_cam_info.cam_mode ==
    CAM_MODE_PRIMARY)? 1:0;

  if (!is_master)
    return IMG_SUCCESS;

  mct_queue_traverse(&p_session_data->frame_config_q,
    module_imgbase_client_update_frameid_frameconfigq_traverse, input);

   return IMG_SUCCESS;
}

/**
 * Function: module_imgbase_client_handle_sof_notify
 *
 * Description: This function is used to update tardy
 * delay to frame_config_q queue
 *
 * Arguments:
 *   @p_port: mct port pointer
 *   @event :  mct event pointer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/

int module_imgbase_client_handle_sof_notify(mct_port_t *port,
  mct_event_t *event) {

  mct_module_t *p_mct_mod;
  module_imgbase_t *p_mod;
  mct_bus_msg_isp_sof_t *sof_event = NULL;
  uint32_t tardy;
  uint32_t remap_start_id;
  imgbase_client_t *p_client;
  int32_t rc = IMG_SUCCESS;
  uint32_t sessionid;
  imgbase_session_data_t *p_session_data;
  void*  input[3];

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

  sof_event = event->u.ctrl_event.control_event_data;
  if (!sof_event) {
    IDBG_ERROR("%s:%d] Error: invalid Sof event", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  pthread_mutex_lock(&p_session_data->q_mutex);
  tardy = sof_event->tardy;
  remap_start_id = sof_event->remap_start_id;
  input[0] = &tardy;
  input[1] = &remap_start_id;

  if (tardy) {
    rc = module_imgbase_client_update_perframe_ctrlq(p_session_data, input);
    if (rc < 0) {
      IDBG_ERROR("%s:%d] Error: tardy delay not updated", __func__, __LINE__);
      pthread_mutex_unlock(&p_session_data->q_mutex);
      return IMG_ERR_GENERAL;
    }
  }
  pthread_mutex_unlock(&p_session_data->q_mutex);
  return IMG_SUCCESS;
}

/**
 * Function: module_imgbase_client_handle_meta
 *
 * Description: This function is used to queue the metadata
 *   buffer
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @p_stream: stream
 *   @current_frame_id: current frame id
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_imgbase_client_handle_meta(imgbase_client_t *p_client,
  imgbase_stream_t *p_stream, uint32_t current_frame_id)
{
  int rc = IMG_SUCCESS;
  img_meta_t             *p_meta;
  uint32_t               session_id = 0;
  float                  zoom_factor = 0;
  img_component_ops_t    *p_comp;
  module_imgbase_t       *p_mod;
  cam_pp_feature_config_t *p_config;
  imgbase_session_data_t *p_session_data;

  p_comp = &p_client->comp;
  p_mod = (module_imgbase_t *)p_client->p_mod;
  p_config = &p_stream->stream_info->reprocess_config.pp_feature_config;

  p_meta = (img_meta_t *)calloc(1, sizeof(img_meta_t));
  if (!p_meta) {
    IDBG_ERROR("%s:%d] Error: Cannot allocate memory", __func__, __LINE__);
    rc = IMG_ERR_NO_MEMORY;
    goto error;
  }
  IDBG_MED("p_meta %p hold_meta %d", p_meta, p_client->caps.hold_meta);

  /* get zoom factor from session data */
  session_id = IMGLIB_SESSIONID(p_stream->stream_info->identity);
  zoom_factor = module_imgbase_get_zoom_ratio(
    MCT_MODULE_CAST((MCT_PORT_PARENT(p_stream->p_sinkport))->data),
    session_id, p_config->zoom_level);

  p_session_data = IMGBASE_SSP(p_mod, session_id);

  rc = img_set_meta(&p_client->current_meta, IMG_META_ZOOM_FACTOR,
    &zoom_factor);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    goto error;
  }

  rc = img_set_meta(&p_client->current_meta, IMG_META_NUM_INPUT,
    &p_mod->caps.num_input);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    goto error;
  }

  *p_meta = p_client->current_meta;
  p_client->p_current_meta = p_meta;

  module_imgbase_client_update_session_meta(
    &p_session_data->session_meta, p_meta);

  img_fov_cfg_t fov_cfg;
  memset(&fov_cfg, 0x0, sizeof(fov_cfg));

  module_imglib_common_fill_fov_params(
    &p_session_data->sensor_max_dim,
    &p_session_data->sensor_crop,
    p_client->stream_crop_valid,
    &p_client->stream_crop,
    p_client->isp_output_dim_stream_info_valid,
    &p_client->isp_output_dim_stream_info,
    &p_stream->stream_info->dim,
    &fov_cfg.fov_params[0], 5);
  rc = img_set_meta(p_meta, IMG_META_FOV_CONFIG, &fov_cfg);
  if (IMG_ERROR(rc)) {
    IDBG_WARN("[%s] error updating fov cfg rc %d", p_mod->name, rc);
  }

  if (p_mod->modparams.imgbase_client_update_meta) {
    p_mod->modparams.imgbase_client_update_meta(p_client, p_meta);
  }

  p_meta->frame_id = current_frame_id;

  p_client->p_current_misc_data = module_imglib_common_get_miscdata(
    p_stream->stream_info, p_config->misc_buf_param.misc_buffer_index);

  if (p_client->p_current_misc_data) {
    img_misc_t misc_data;
    misc_data.data = p_client->p_current_misc_data->data;
    rc = img_set_meta(p_meta, IMG_META_MISC_DATA, &misc_data);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      goto error;
    }
  }

  if (CAM_STREAM_TYPE_OFFLINE_PROC == p_stream->stream_info->stream_type) {
    rc = module_imgbase_client_update_offline_meta(p_client, p_meta,
      current_frame_id);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Update offline meta failed rc %d", rc);
      goto error;
    }
  }

  /* queue the meta buffer */
  if (!p_mod->caps.hold_meta) {
    rc = IMG_COMP_Q_META_BUF(p_comp, p_meta);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Q_meta failed rc %d", rc);
      goto error;
    }
    p_client->num_meta_queued++;
  } else {
    rc = img_q_enqueue(&p_client->meta_hold_q, p_meta);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("hold Q_meta failed rc %d", rc);
      goto error;
    }
  }

  return IMG_SUCCESS;

error:
  if (p_meta) {
    free(p_meta);
  }
  return rc;
}

/**
 * Function: module_imgbase_client_handle_overlap_batch_input
 *
 * Description: This function is used to handle the overlap
 *   batch input buffers. Batch processing refers to queueuing
 *   the current frame and queueing n previous frames. Retreive
 *   the previous n frames from the list of frames previously
 *   received as a buf divert into imglib and queue it to the
 *   component
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @p_buf_divert: buffer divert structure
 *   @p_frame: current frame ptr
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
static int module_imgbase_client_handle_overlap_batch_input(
  imgbase_client_t *p_client, isp_buf_divert_t *p_buf_divert,
  img_frame_t *p_frame)
{
  int32_t rc;
  int i;
  uint32_t frame_id;
  int batch_curr_idx;
  int batch_start_idx;
  int batch_next_idx;
  imgbase_buf_t *p_buf = NULL;
  imgbase_buf_t *p_temp_buf = NULL;
  img_frame_t *p_temp_frame = NULL;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  img_component_ops_t *p_comp = &p_client->comp;

  p_buf = p_frame->private_data;
  frame_id = p_buf_divert->buffer.sequence;
  IDBG_HIGH("%s:%d] curr frame %d", __func__, __LINE__, frame_id);

  batch_curr_idx = p_client->buf_list_idx;
  batch_start_idx = (batch_curr_idx - p_mod->caps.num_overlap) < 0 ?
    (batch_curr_idx - p_mod->caps.num_overlap + MAX_IMGLIB_BATCH_SIZE) :
    (batch_curr_idx - p_mod->caps.num_overlap);

  pthread_mutex_lock(&p_client->buf_mutex);

  /* store current frame details within buf list */
  p_temp_buf = p_client->buf_list[batch_curr_idx];
  if (p_temp_buf) {
    if (p_temp_buf->divert_done == FALSE || p_temp_buf->ack.frame_id > 0) {
      IDBG_ERROR("%s:%d] Error, frame %d not yet forwarded, but attempting "
        "to overwrite %d %d", __func__, __LINE__, p_temp_buf->frame.frame_id,
        p_temp_buf->divert_done, p_temp_buf->ack.frame_id);
      pthread_mutex_unlock(&p_client->buf_mutex);
      return IMG_ERR_INVALID_OPERATION;
    }
    free(p_temp_buf);
    p_temp_buf = NULL;
  }
  p_client->buf_list[batch_curr_idx] = p_buf;
  p_client->buf_list_idx = (p_client->buf_list_idx + 1) % MAX_IMGLIB_BATCH_SIZE;

  /* queue batch overlap frames by finding previous frames within buf list.
    If previous frame is not found, queue dummy frame */
  for (i = 0; i < p_mod->caps.num_overlap; i++) {
    p_temp_frame = NULL;
    batch_next_idx = (batch_start_idx + i) % MAX_IMGLIB_BATCH_SIZE;
    p_temp_buf = p_client->buf_list[batch_next_idx];

    if (p_temp_buf) {
      IDBG_HIGH("%s:%d] curr frame %d, found previous frame %d",
        __func__, __LINE__, frame_id, p_temp_buf->frame.frame_id);
      p_temp_buf->queued_cnt++;
      p_temp_frame = &p_temp_buf->frame;
    } else {
      IDBG_HIGH("%s:%d] prev frame not found, use dummy", __func__, __LINE__);
      p_temp_frame = &p_client->dummy_frame;
    }

#if 0
    img_common_handle_input_frame_cache_op(p_mod->modparams.exec_mode, p_temp_frame,
      p_mod->ion_fd,
      (p_mod->modparams.force_cache_op | p_mod->force_cache_op),
      &p_temp_buf->buf_divert.buffer_access);
#endif

    /* queue frame */
    rc = IMG_COMP_Q_BUF(p_comp, p_temp_frame, IMG_IN);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] Error, failed to qbuf %d", __func__, __LINE__, rc);
      pthread_mutex_unlock(&p_client->buf_mutex);
      return rc;
    }
    p_client->cur_buf_cnt++;
  }
  PRINT_BUF_LIST(p_client);

  pthread_mutex_unlock(&p_client->buf_mutex);
  return IMG_SUCCESS;
}

/**
 * Function: imgbase_client_handle_input_buf
 *
 * Description: This function is used to handle the input buffer
 *   from the upstream module
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @p_buf_divert: Buffer divert structure
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
static int module_imgbase_client_handle_input_buf(imgbase_client_t *p_client,
  isp_buf_divert_t *p_buf_divert)
{
  int32_t rc;
  uint32_t frame_id;
  img_frame_t *p_frame;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  img_component_ops_t *p_comp = &p_client->comp;
  imgbase_buf_t *p_buf = (imgbase_buf_t *)calloc(1, sizeof(imgbase_buf_t));

  if (!p_buf) {
    IDBG_ERROR("%s:%d] Error: Cannot allocate memory", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  p_frame = &p_buf->frame;
  p_buf->buf_divert = *p_buf_divert;
  p_frame->private_data = p_buf;

  /* Get input buffer */
  rc = module_imgbase_client_getbuf(p_client, p_buf_divert, p_frame,
    p_buf_divert->native_buf);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s_%s:%d] Error: Cannot get frame", __func__, p_mod->name,
      __LINE__);
    rc = IMG_ERR_GENERAL;
    goto error;
  }

  img_common_handle_input_frame_cache_op(p_mod->modparams.exec_mode, p_frame,
    p_mod->ion_fd,
    (p_mod->modparams.force_cache_op | p_mod->force_cache_op),
    &p_buf->buf_divert.buffer_access);

  if ((p_mod->modparams.access_mode == IMG_ACCESS_READ_WRITE) &&
    (p_mod->caps.inplace_algo == TRUE) &&
    (p_mod->caps.num_output == 0)) {
    // This module writes into output frames (not read-only module like paaf)
    // Handle output buffer cache operations.
    img_common_handle_output_frame_cache_op(p_mod->modparams.exec_mode,
      p_frame, p_mod->ion_fd,
      (p_mod->modparams.force_cache_op | p_mod->force_cache_op),
      &p_buf->buf_divert.buffer_access);
  }

  frame_id = p_buf_divert->buffer.sequence;
  IDBG_HIGH(" dim %dx%d id %d", p_frame->info.width,
    p_frame->info.height, frame_id);

  /* Store input frame frameId */
  rc = module_imgbase_client_store_frame_id(p_client, frame_id,
    p_client->cur_buf_cnt);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s_%s:%d] Error storing frameIds", __func__, p_mod->name,
      __LINE__);
    goto error;
  }

  /* Queue input buffer */
  p_buf->queued_cnt++;
  rc = IMG_COMP_Q_BUF(p_comp, p_frame, IMG_IN);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] Error, failed to qbuf %d", __func__, __LINE__, rc);
    goto error;
  }
  p_client->cur_buf_cnt++;

  /* Queue overlap batch buffers */
  if (p_mod->caps.num_overlap) {
    rc = module_imgbase_client_handle_overlap_batch_input(p_client,
      p_buf_divert, p_frame);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      goto error_overlap;
    }
  }

  IDBG_LOW("%s_%s:%d] bufs queued %d, bufs needed %d, bufs overlap %d",
    __func__, p_mod->name, __LINE__, p_client->cur_buf_cnt,
    p_mod->caps.num_input, p_mod->caps.num_overlap);

  return IMG_SUCCESS;

error:
  if (p_buf) {
    free(p_buf);
  }
error_overlap:
  IDBG_ERROR("%s_%s:%d] Error rc %d", __func__, p_mod->name, __LINE__, rc);
  return rc;
}

/**
 * Function: module_imgbase_client_handle_dualcam_outputbuf
 *
 * Description: This function is used to capture number of
 *            output buffers assigned to master and slave.
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
static int module_imgbase_client_handle_dualcam_outputbuf
  (imgbase_client_t *p_client)
{
  int8_t num_output;
  imgbase_session_data_t *p_base_sess_data;
  uint32_t is_master =0;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  if (NULL == p_mod) {
    IDBG_ERROR("Imgbase module NULL");
    return IMG_ERR_INVALID_INPUT;
  }

  if (!p_mod->caps.dual_camfeature) {
    return IMG_SUCCESS;
  }

  p_base_sess_data = IMGBASE_SSP(p_mod, p_client->session_id);
  if (NULL == p_base_sess_data) {
    IDBG_ERROR("Imgbase session data is NULL");
    return IMG_ERR_INVALID_INPUT;
  }

  is_master = (p_base_sess_data->multi_cam_info.cam_mode ==
    CAM_MODE_PRIMARY)? 1:0;

  num_output = is_master ? p_mod->caps.num_master_output
    : p_mod->caps.num_slave_output;

  IDBG_LOW("Session id %x is_master %d num_output %d",
    p_client->session_id,is_master,num_output);

  return num_output ? IMG_SUCCESS : IMG_ERR_GENERAL;
}

/**
 * Function: module_imgbase_client_get_and_update_buf_index
 *
 * Description: This function is used to get HAL3 requested
 * buffer index
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @frame_id: Frame id
 *   @reset : flag to reset queue index
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/

int32_t module_imgbase_client_get_and_update_buf_index(module_imgbase_t *p_mod,
  imgbase_client_t *p_client, uint32_t frame_id, int reset)
{
  uint32_t index = 0xFFFFFFFF;
  uint32_t stream_id = IMGLIB_STREAMID(p_client->divert_identity);
  uint32_t i;
  imgbase_session_data_t *p_session_data;

  p_session_data = IMGBASE_SSP(p_mod, p_client->session_id);
  if (p_session_data == NULL) {
     IDBG_ERROR("CAM_INTF_META_STREAM_ID failed, session data null \n");
     return IMG_ERR_INVALID_INPUT;
  }

  /* get the buffer index from per frame queue */
  uint32_t q_idx = frame_id % PER_FRAME_QUEUE_SIZE;

  for (i = 0; i < p_session_data->stream_id_requested[q_idx].
    num_streams; i++) {
    if (p_session_data->stream_id_requested[q_idx].
      stream_request[i].streamID ==stream_id) {
      index = p_session_data->stream_id_requested[q_idx].
        stream_request[i].buf_index;
      if (reset == 1) {
        p_session_data->stream_id_requested[q_idx].
          stream_request[i].streamID = 0;
      }
      break;
    }
  }

  return (p_mod->hal_version == CAM_HAL_V3) ?
    index : 0xFFFFFFFF;
}


/**
 * Function: module_imgbase_client_handle_output_buf
 *
 * Description: This function is used to queue the output
 *   buffer to the component. For the output buffer, either
 *   fetch the output buffer OR use the preallocated internal
 *   buffers
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @p_buf_divert: Buffer divert structure
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
static int module_imgbase_client_handle_output_buf(imgbase_client_t *p_client,
  isp_buf_divert_t *p_buf_divert)
{
  int32_t rc = IMG_SUCCESS;
  img_frame_t *p_out_frame;
  imgbase_buf_t *p_buf = NULL;
  img_internal_bufnode_t *p_intbuf = NULL;

  img_component_ops_t *p_comp = &p_client->comp;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  uint32_t out_buffer_access = 0;
  imgbase_stream_t *p_stream = NULL;
  int32_t stream_idx = 0;
  uint32_t idx;

  if (p_mod->caps.num_output == 0) {
    IDBG_HIGH("%s:%d] num output buf is 0", __func__, __LINE__);
    return rc;
  }

  rc = module_imgbase_client_handle_dualcam_outputbuf(p_client);
  if (rc != IMG_SUCCESS) {
    IDBG_HIGH("%s:%d] Dual Cam num output buf is 0", __func__, __LINE__);
    return IMG_SUCCESS;
  }

  /* Fetch output buffer */
  p_buf = (imgbase_buf_t *)calloc(1, sizeof(imgbase_buf_t));
  if (!p_buf) {
    IDBG_ERROR("[%s] Error: Cannot allocate memory", p_mod->name);
    rc = IMG_ERR_NO_MEMORY;
    goto error;
  }

  stream_idx = module_imgbase_find_stream_by_identity(p_client,
    p_client->divert_identity);
  if (stream_idx < 0) {
    IDBG_ERROR("[%s] Cannot find stream mapped to idx %x", p_mod->name,
      p_client->divert_identity);
    goto error;
  }
  p_stream = &p_client->stream[stream_idx];

  idx = module_imgbase_client_get_and_update_buf_index(p_mod, p_client,
    p_buf_divert->buffer.sequence, 1);
  IDBG_LOW("buff index %d", idx);

  p_out_frame = &p_buf->frame;
  p_buf->buf_divert = *p_buf_divert;
  p_buf->event_identity = p_client->divert_identity;
  p_out_frame->private_data = p_buf;
  p_out_frame->req_buf_idx = idx;

  pthread_mutex_lock(&p_client->buf_mutex);
  if ((0 == p_mod->caps.num_hold_outbuf) ||
    (p_client->cur_outbuf_cnt < p_mod->caps.num_hold_outbuf &&
    (!img_q_count(&p_client->output_hold_q)))) {

    IDBG_MED("%s] OUTPUTBUF Not Delayed for %p cur_outbuf_cnt %d frame id %d",
      p_mod->name, p_out_frame, p_client->cur_outbuf_cnt,
      p_buf->buf_divert.buffer.sequence);
    p_client->cur_outbuf_cnt++;
    IDBG_LOW("cur_outbuf_cnt %d", p_client->cur_outbuf_cnt);
    pthread_mutex_unlock(&p_client->buf_mutex);

    rc = module_imgbase_client_get_outputbuf(p_client, p_out_frame,
      p_mod->caps.use_internal_bufs, true, &out_buffer_access);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Error, rc %d", rc);
      pthread_mutex_lock(&p_client->buf_mutex);
      for (uint32_t i = 0; i < p_client->internal_buf.buf_cnt; i++) {
        p_intbuf = &p_client->internal_buf.buf[i];
        if (p_intbuf && (p_intbuf->p_frame == p_out_frame)) {
          IDBG_ERROR("[%s] get_buf error for identity %x frame_id %d",
            p_mod->name, p_client->divert_identity, p_out_frame->frame_id);
          p_intbuf->ref_cnt--;
          p_intbuf->input_ack_held = FALSE;
          break;
        }
      }
      p_client->cur_outbuf_cnt--;
      IDBG_LOW("cur_outbuf_cnt %d", p_client->cur_outbuf_cnt);
      pthread_mutex_unlock(&p_client->buf_mutex);
      goto error;
    }

    if (p_mod->modparams.access_mode == IMG_ACCESS_READ_WRITE) {
      // This module writes into output frames (not read-only module like paaf)
      // Handle output buffer cache operations.
      img_common_handle_output_frame_cache_op(p_mod->modparams.exec_mode,
        p_out_frame, p_mod->ion_fd,
        (p_mod->modparams.force_cache_op | p_mod->force_cache_op),
        &out_buffer_access);
    }

    p_buf->buf_divert.buffer_access = out_buffer_access;

    /* queue the buffer */
    rc = IMG_COMP_Q_BUF(p_comp, p_out_frame, IMG_OUT);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Error, failed to qbuf %d", rc);
      pthread_mutex_lock(&p_client->buf_mutex);
      p_client->cur_outbuf_cnt--;
      IDBG_LOW("cur_outbuf_cnt %d", p_client->cur_outbuf_cnt);
      pthread_mutex_unlock(&p_client->buf_mutex);
      goto error;
    }
  } else {
    if (IMG_ERROR(img_q_enqueue(&p_client->output_hold_q,
      (void*)p_out_frame))) {
      IDBG_ERROR("%s:%d] Error, failed to qbuf to local queue%d",
        __func__, __LINE__, rc);
      pthread_mutex_unlock(&p_client->buf_mutex);
      goto error;
    } else {
      IDBG_MED("%s:%d] OUTPUTBUF Delayed for  %p cur_outbuf_cnt %d",
        __func__, __LINE__, p_out_frame, p_client->cur_outbuf_cnt);
    }
    pthread_mutex_unlock(&p_client->buf_mutex);
  }

  return rc;

error:
  if (p_buf) {
    free(p_buf);
  }
  return rc;
}

/**
 * Function: module_imgbase_client_handle_update_superparms_drop_queue
 *
 * Description: Super params handler function to drop queue
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: mct event
 *   @current_frame_id: current frameid
 *
 * Return values:
 *    standard image lib return codes
 *
 * Notes: none
 **/

int module_imgbase_client_handle_update_superparms_drop_queue
  (mct_port_t *port,
  mct_event_t *event, uint32_t current_frameid)
{
  mct_module_t *p_mct_mod = NULL;
  module_imgbase_t *p_mod = NULL;
  int rc = IMG_SUCCESS;
  uint32_t sessionid;
  imgbase_session_data_t *p_session_data;
  img_queue_t *p_q = NULL;
  img_node_t* node = NULL;
  imgbase_framedrop_t *p_drop_data = NULL;

  if (!port || !event) {
    IDBG_ERROR("invalid input, port=%p, event=%p", port, event);
    return FALSE;
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

  sessionid = IMGLIB_SESSIONID(event->identity);
  p_session_data = IMGBASE_SSP(p_mod, sessionid);

  p_q = &p_session_data->framedrop_hold_q;
  pthread_mutex_lock(&p_q->mutex);

  for (node = p_q->p_front; node; node = node->next) {
    p_drop_data = (imgbase_framedrop_t *)node->data;
    if (p_drop_data != NULL) {
      if (p_drop_data->frameid < current_frameid) {
        rc = module_imgbase_process_super_params(port, event,
          FALSE, p_drop_data->frameid);
        if (IMG_ERROR(rc)) {
          IDBG_ERROR("[%s] cannot process super params rc %d",
            p_mod->name, rc);
        }
      }
    }
    p_drop_data = NULL;
  }
  pthread_mutex_unlock(&p_q->mutex);

  rc = module_imgbase_process_super_params(port, event, TRUE, 0);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("[%s] cannot process super params rc %d", p_mod->name, rc);
    return IMG_ERR_INVALID_INPUT;
  }

  module_imgbase_client_handle_framedrop(port, event, 0, FALSE);

  return IMG_SUCCESS;
}

int module_imgbase_client_handle_drop_queue_traverse
  (void *p_nodedata, void *p_userdata)
{
  imgbase_framedrop_t* master_drop = p_nodedata;
  uint32_t *current_frame_id = p_userdata;

  if (!p_nodedata || !p_userdata) {
    IDBG_ERROR("NULL pointer %p %p", p_nodedata, p_userdata);
    return 0;
  }

  if (master_drop->frameid <= *current_frame_id) {
    return 1;
  }

  return 0;
}

/**
 * Function: module_imgbase_client_handle_framedrop
 *
 * Description: This function is used to handle the framedrop
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: mct event
 *   @frame_id: frame id
 *   @streamoff: streamoff
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/

int module_imgbase_client_handle_framedrop(mct_port_t *port,
  mct_event_t *event, uint32_t frameid, uint32_t streamoff)
{
  mct_module_t *p_mct_mod;
  module_imgbase_t *p_mod;
  imgbase_client_t *p_client;
  uint32_t sessionid;
  imgbase_session_data_t *p_session_data;
  uint32_t identity = event->identity;
  struct msm_buf_mngr_info buff_info;
  int32_t cmd;
  int ret;
  uint32_t index, count =0, current_frame_id =0;
  int32_t out_idx;
  mct_event_module_t *p_mod_event= NULL;
  imgbase_framedrop_t *p_drop_data = NULL;

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

  if (p_mod->feature_mask != CAM_QTI_FEATURE_RTB) {
    return IMG_SUCCESS;
  }

  p_client = (imgbase_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("[%s] imgbase client NULL", p_mod->name);
    return IMG_SUCCESS;
  }

  sessionid = IMGLIB_SESSIONID(event->identity);
  p_session_data = IMGBASE_SSP(p_mod, sessionid);
  if (NULL == p_session_data) {
    /* Error printed in the macro*/
    return IMG_ERR_INVALID_INPUT;
  }

  if (frameid || streamoff) {
    current_frame_id = frameid;
  } else {
    p_mod_event = &event->u.module_event;
    if (p_mod_event == NULL) {
      IDBG_ERROR("[%s] imgbase p_mod_event NULL", p_mod->name);
      goto error;
    }

    isp_buf_divert_t *p_buf_divert =
      (isp_buf_divert_t *)p_mod_event->module_event_data;
    if (p_buf_divert == NULL) {
      IDBG_ERROR("[%s] imgbase p_buf_divert NULL", p_mod->name);
      goto error;
    }
    current_frame_id = p_buf_divert->buffer.sequence;
  }

  count = img_q_count(&p_session_data->framedrop_hold_q);
  IDBG_HIGH("[%s] identity %x current frame id %d count %d streamoff %d",
    p_mod->name, identity,
    current_frame_id, count, streamoff);
  while(count) {
    if (streamoff) {
      p_drop_data = img_q_dequeue(&p_session_data->framedrop_hold_q);
    } else {
      p_drop_data  = (imgbase_framedrop_t *)img_q_traverse_dequeue(
        &p_session_data->framedrop_hold_q,
        module_imgbase_client_handle_drop_queue_traverse,
        &current_frame_id);
    }
    if (p_drop_data != NULL) {
      index = module_imgbase_client_get_and_update_buf_index(p_mod, p_client,
        p_drop_data->frameid, 1);
      if (index == 0xFFFFFFFF) {
        IDBG_LOW("[%s] framedrop not required identity %x frame id %d",
          p_mod->name, identity, p_drop_data->frameid);
        free(p_drop_data);
        p_drop_data = NULL;
        count--;
        continue;
      }

      out_idx = module_imglib_common_get_buffer(p_mod->subdevfd, identity,
        NULL, index);
      if (out_idx < 0) {
        IDBG_ERROR("[%s] cannot get buffer id %x", p_mod->name, identity);
        goto error;
      }

      IDBG_HIGH("[%s] identity %x frame id %d", p_mod->name, identity,
        p_drop_data->frameid);

      buff_info.index = out_idx;
      buff_info.session_id = IMGLIB_SESSIONID(identity);
      buff_info.stream_id = IMGLIB_STREAMID(identity);
      buff_info.frame_id = p_drop_data->frameid;
      buff_info.timestamp.tv_sec = 0;
      buff_info.timestamp.tv_usec = 0;
      cmd = VIDIOC_MSM_BUF_MNGR_BUF_ERROR;

      ret = ioctl(p_mod->subdevfd, cmd, &buff_info);
      if (ret < 0) {
        IDBG_ERROR("%s:%d] Failed to do buf_done id 0x%x out_idx %d frameid %d",
          __func__, __LINE__, identity, out_idx, p_drop_data->frameid);
        goto error;
      }
      free(p_drop_data);
      p_drop_data = NULL;
    }
    count--;
  }
  return IMG_SUCCESS;

error:
  if (p_drop_data) {
    free(p_drop_data);
    p_drop_data = NULL;
  }
  return IMG_ERR_GENERAL;
}

/**
 * Function: module_imgbase_client_handle_buffer
 *
 * Description: This function is used to handle the divert
 *            buffer event
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @p_buf_divert: Buffer divert structure
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_handle_buffer(imgbase_client_t *p_client,
  isp_buf_divert_t *p_buf_divert)
{
  int rc = IMG_SUCCESS;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  imgbase_stream_t *p_stream = NULL;
  int stream_idx;
  uint32_t frame_id = p_buf_divert->buffer.sequence;
  void *p_dq_frame = NULL;

  stream_idx = module_imgbase_find_stream_by_identity(p_client,
    p_client->divert_identity);
  if (stream_idx < 0) {
    IDBG_ERROR("%s_%s %d] Cannot find stream mapped to idx %x", __func__,
      p_mod->name, __LINE__, p_client->divert_identity);
    return IMG_ERR_GENERAL;
  }

  p_stream = &p_client->stream[stream_idx];
  if (NULL == p_stream) {
    IDBG_ERROR("%s_%s:%d] Cannot find stream mapped to client", __func__,
      p_mod->name, __LINE__);
    return IMG_ERR_GENERAL;
  }

  IDBG_LOW("%s_%s:%d] E", __func__, p_mod->name, __LINE__);

  pthread_mutex_lock(&p_client->mutex);
  if (!p_client->stream_on) {
    IDBG_LOW("%s_%s:%d] Not streaming, discard buffers", __func__,
      p_mod->name, __LINE__);
    pthread_mutex_unlock(&p_client->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }
  pthread_mutex_unlock(&p_client->mutex);

  p_client->p_current_buf_div = p_buf_divert;

  /* Queue input bufs */
  rc = module_imgbase_client_handle_input_buf(p_client,
    p_buf_divert);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s_%s:%d] Error: cannot handle input buf",
      __func__, p_mod->name, __LINE__);
    goto error;
  }

  /* Queue each meta along with the input buf if there are
    an equal number of meta bufs and input bufs */
  if (p_mod->caps.num_meta &&
    (p_mod->caps.num_meta == p_mod->caps.num_input)) {
    rc = module_imgbase_client_handle_meta(p_client, p_stream, frame_id);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s_%s:%d] Error rc %d", __func__, p_mod->name, __LINE__, rc);
      goto meta_error;
    }
    IDBG_LOW("%s_%s:%d] queue meta with frame %d",
      __func__, p_mod->name, __LINE__, frame_id);
  }

  IDBG_LOW("%s] curbufcnt %d , num input %d", p_mod->name,
    p_client->cur_buf_cnt,p_mod->caps.num_input );
  if (p_client->cur_buf_cnt >= p_mod->caps.num_input) {
    p_client->cur_buf_cnt = 0;

    /* Queue the output buffer */
    rc = module_imgbase_client_handle_output_buf(p_client, p_buf_divert);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s_%s:%d] Error queuing the output buffer %d", __func__,
        p_mod->name, __LINE__, rc);
      goto output_buf_error;
    }

    /* Add imglib debug data to hal meta */
    if (CAM_STREAM_TYPE_OFFLINE_PROC == p_stream->stream_info->stream_type) {
      rc = module_imglib_common_add_imglib_debug_meta(
        p_client->hal_meta_data_list, p_client->meta_data_list);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("%s_%s:%d] Cannot add imglib debug\n",
          __func__, p_mod->name, __LINE__);
      }
    }

    /* If num meta != num input, it was not queued with the input bufs.
      Queue it now with the output buf */
    if (p_mod->caps.num_meta &&
      (p_mod->caps.num_meta != p_mod->caps.num_input)) {
      rc = module_imgbase_client_handle_meta(p_client, p_stream, frame_id);
        if (rc != IMG_SUCCESS) {
          IDBG_ERROR("%s_%s:%d] Error rc %d", __func__, p_mod->name,
            __LINE__, rc);
          goto output_meta_error;
       }
      IDBG_LOW("%s_%s:%d] queue meta with last frame %d",
        __func__, p_mod->name, __LINE__, frame_id);
    }
  }

  IDBG_MED("%s_%s:%d] X", __func__, p_mod->name, __LINE__);
  return 0;

output_meta_error:
  /* dequeue output buffer base on frame id*/
  IMG_COMP_DQ_BUF_BY_FID(&p_client->comp, &p_dq_frame, frame_id, IMG_OUT);
  if (p_dq_frame && ((img_frame_t *)p_dq_frame)->private_data) {
    free(((img_frame_t *)p_dq_frame)->private_data);
    ((img_frame_t *)p_dq_frame)->private_data = NULL;
    p_dq_frame = NULL;
  }

output_buf_error:
  /* dequeue meta buffer base on frame id*/
  if (p_mod->caps.num_meta &&
    (p_mod->caps.num_meta == p_mod->caps.num_input)) {
    IMG_COMP_DQ_BUF_BY_FID(&p_client->comp, &p_dq_frame, frame_id, IMG_META);
    if (p_dq_frame) {
      free((img_meta_t *)p_dq_frame);
      p_dq_frame = NULL;
    }
  }

meta_error:
  /* dequeue input buffer base on frame id*/
  IMG_COMP_DQ_BUF_BY_FID(&p_client->comp, &p_dq_frame, frame_id, IMG_IN);
  if (p_dq_frame && ((img_frame_t *)p_dq_frame)->private_data) {
    free(((img_frame_t *)p_dq_frame)->private_data);
    ((img_frame_t *)p_dq_frame)->private_data = NULL;
    p_dq_frame = NULL;
  }

error:
  return rc;
}

/**
 * Function: module_imgbase_client_get_frame
 *
 * Description: This function is to fetching the output buffer
 *           based in buffer info
 * Arguments:
 *   @p_appdata: imgbase client
 *   @pframe: frame pointer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_get_frame(void *p_appdata,
  img_frame_t **pp_frame)
{
  int rc = IMG_SUCCESS;
  imgbase_buf_t *p_buf;
  imgbase_client_t *p_client = (imgbase_client_t *)p_appdata;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  img_frame_t *p_out_frame;

  assert(p_appdata != NULL);
  assert(pp_frame != NULL);
  *pp_frame = NULL;

  IDBG_HIGH("[%s] E", p_mod->name);
  p_buf = (imgbase_buf_t *)calloc(1, sizeof(imgbase_buf_t));
  if (!p_buf) {
    IDBG_ERROR("[%s] Error: Cannot allocate memory", p_mod->name);
    rc = IMG_ERR_NO_MEMORY;
    goto error;
  }

  p_out_frame = &p_buf->frame;
  p_out_frame->private_data = p_buf;
  /* get first stream identity since there is no preference */
  p_buf->buf_divert.identity = p_client->stream[0].identity;

  /* get output buffer */
  rc = module_imgbase_client_get_outputbuf(p_client, p_out_frame,
    false /* external */,
    true /* fetch */,
    NULL);

  if (IMG_ERROR(rc)) {
    IDBG_ERROR("[%s] rc %d", p_mod->name, rc);
    goto error;
  }
  *pp_frame = p_out_frame;
  return rc;

error:
  if (p_buf) {
    free(p_buf);
  }
  return rc;
}

/**
 * Function: module_imgbase_client_release_frame
 *
 * Description: This function is to release the output buffer
 *           based in buffer info
 * Arguments:
 *   @p_appdata: imgbase client
 *   @pframe: frame pointer
 *   @is_dirty: flag to indicate whether to do buf done
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_release_frame(void *p_appdata,
  img_frame_t *p_frame,
  int is_dirty)
{
  imgbase_client_t *p_client = (void *)p_appdata;

  if (!p_frame) {
    IDBG_ERROR("%s:%d] Error invalid release", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_HIGH("%s:%d] p_client %p is_dirty %d", __func__, __LINE__, p_frame,
    is_dirty);

  if (p_client->p_current_meta) {
    cam_stream_img_prop_t prop;
    memset(&prop, 0x0, sizeof(cam_stream_img_prop_t));
    cam_rect_t *p_rect = &prop.crop;
    img_rect_t *output_crop = (img_rect_t *)img_get_meta(
      p_client->p_current_meta, IMG_META_OUTPUT_ROI);
    if (output_crop) {
      p_rect->left = (int32_t)output_crop->pos.x;
      p_rect->top = (int32_t)output_crop->pos.y;
      p_rect->width = (int32_t)output_crop->size.width;
      p_rect->height = (int32_t)output_crop->size.height;
    } else {
      IDBG_ERROR("%s:%d failed: Output crop not set\n", __func__, __LINE__);
    }
    prop.input.width = (int32_t)p_frame->info.width;
    prop.input.height = (int32_t)p_frame->info.height;
    module_imgbase_client_enqueue_streambuf(p_client,
      p_client->p_current_meta, &prop);
  }

  module_imgbase_client_handle_outbuf_done(p_client, p_frame, (int8_t)is_dirty);

  return IMG_SUCCESS;
}

/**
 * Function: module_imgbase_client_pack_buffer
 *
 * Description: This function is used to pack internal
 * buffers into stream mapped buffer structure for offline
 * isp usecases
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @stride_in_bytes: Stride in bytes
 *   @scanline: Scanline size
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_pack_buffer(imgbase_client_t *p_client,
  int32_t stride_in_bytes, int32_t scanline) {
  uint32_t i;
  img_mem_handle_t native_buf_mem;
  if (!p_client || !stride_in_bytes || !scanline) {
    IDBG_ERROR("Invalid input");
    return IMG_ERR_INVALID_INPUT;
  }

  for (i = 0; i < p_client->internal_buf.buf_cnt; i++)  {
   /* Pack imglib internal allocated buffers into necessary structures*/
    native_buf_mem =
      p_client->internal_buf.buf[i].mem_handle;
    p_client->native_buffs_map[i].buf_index = i;
    p_client->native_buffs_map[i].buf_size = stride_in_bytes * scanline;
    p_client->native_buffs_map[i].buf_type = CAM_MAPPING_BUF_TYPE_STREAM_BUF;
    p_client->native_buffs_map[i].num_planes = 1;
    p_client->native_buffs_map[i].buf_planes[0].fd =
      native_buf_mem.fd;
    p_client->native_buffs_map[i].buf_planes[0].buf =
      native_buf_mem.vaddr;
    p_client->native_buffs_map[i].buf_planes[0].size =
      native_buf_mem.length;
    p_client->native_buffs_map[i].buf_planes[0].stride = stride_in_bytes;
    p_client->native_buffs_map[i].buf_planes[0].scanline = scanline;
    p_client->native_buffs_map[i].buf_planes[0].offset = 0;
    p_client->native_buffs_map[i].common_fd = FALSE;
    p_client->native_buffs_map[i].container_fd = -1;
    p_client->native_buffs_map[i].buf_container_info = NULL;

    p_client->native_buffer_list =
      mct_list_append(p_client->native_buffer_list,
      &p_client->native_buffs_map[i], NULL, NULL);
    IDBG_INFO("native_buffer_list %p", p_client->native_buffer_list);
  }
  return IMG_SUCCESS;
}

/**
 * Function: module_imgbase_client_alloc_int_buf
 *
 * Description: This function is used to allocate internal
 * buffers
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @identity: identity of the stream
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_alloc_int_buf(imgbase_client_t *p_client,
  uint32_t identity)
{
  int rc = IMG_SUCCESS;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  int32_t stream_idx;
  imgbase_stream_t *p_stream;
  uint32_t frame_len;
  uint32_t i;

  stream_idx = module_imgbase_find_stream_by_identity(p_client,
    identity);
  if (stream_idx < 0) {
    IDBG_ERROR("[%s] Cannot find stream mapped to idx %x", p_mod->name,
      identity);
    return IMG_ERR_NOT_FOUND;
  }

  p_stream = &p_client->stream[stream_idx];

  if (NULL == p_stream) {
    IDBG_ERROR("[%s] Cannot find stream mapped to client", p_mod->name);
    return IMG_ERR_NOT_FOUND;
  }

  if ( p_client->before_cpp) {
    if (p_client->is_dim_change) {
      frame_len = p_stream->stream_info->buf_planes.plane_info.frame_len;
    } else {
      frame_len = p_stream->isp_stream_info.buf_planes.plane_info.frame_len;
    }
  } else {
    frame_len = p_stream->stream_info->buf_planes.plane_info.frame_len;
  }

  IDBG_INFO("[%s] Id %x Buffer cnt %d length %u", p_mod->name, identity,
    p_client->internal_buf.buf_cnt, frame_len);
  if (frame_len <= 0) {
    IDBG_ERROR("[%s] Error, invalid length for buffer", p_mod->name);
    return IMG_ERR_NOT_FOUND;
  }

  for (i = 0; i < p_client->internal_buf.buf_cnt; i++) {
    rc = img_buffer_get(p_client->internal_buf.type, -1, TRUE, frame_len,
      &p_client->internal_buf.buf[i].mem_handle);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("[%s] Error, img buffer get failed, %d, %dx%d",
        p_mod->name, p_client->internal_buf.type,
        p_stream->stream_info->dim.width, p_stream->stream_info->dim.height);
      goto error;
    }
    p_client->internal_buf.buf[i].is_init = TRUE;
    p_client->internal_buf.buf[i].exists_in_cache = FALSE;

    IDBG_HIGH("[%s] : buf_index=%d, fd=%d, vaddr=%p, length=%d",
      p_mod->name, i, p_client->internal_buf.buf[i].mem_handle.fd,
      p_client->internal_buf.buf[i].mem_handle.vaddr,
      p_client->internal_buf.buf[i].mem_handle.length);
  }
  IDBG_MED("[%s] Allocate Internal Buf successful type %d, %dx%d",
    p_mod->name, p_client->internal_buf.type,
    p_stream->stream_info->dim.width, p_stream->stream_info->dim.height);
  PRINT_INTERNAL_BUF_LIST(p_client);
  return rc;

error:
  pthread_mutex_lock(&p_client->buf_mutex);
  for (i = 0; i < p_client->internal_buf.buf_cnt; i++) {
    if (p_client->internal_buf.buf[i].mem_handle.handle) {
      img_buffer_release(&p_client->internal_buf.buf[i].mem_handle);
      p_client->internal_buf.buf[i].is_init = FALSE;
    }
  }
  pthread_mutex_unlock(&p_client->buf_mutex);
  IDBG_ERROR("Error");
  return rc;
}

/**
 * Function: module_imgbase_client_start
 *
 * Description: This function is used to start the IMGLIB_BASE
 *              client
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @identity: identity of the stream
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_start(imgbase_client_t *p_client, uint32_t identity)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_client->comp;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  imgbase_session_data_t *p_session_data = IMGBASE_SSP(p_mod,
    p_client->session_id);

  if (NULL == p_session_data) {
    /* Error printed in the macro*/
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_LOW("[%s] E", p_mod->name);
  pthread_mutex_lock(&p_client->mutex);

  if (!p_client->stream_on) {
    p_client->first_frame = TRUE;
    if (p_client->caps.num_input > 0) {
      img_caps_t caps;
      memset(&caps, 0x0, sizeof(img_caps_t));
      caps.num_input = p_client->caps.num_input;
      caps.num_output = p_client->caps.num_output;
      rc = IMG_COMP_START(p_comp, &caps);
    } else {
      rc = IMG_COMP_START(p_comp, NULL);
    }

    IDBG_LOW("[%s]use_internal_bufs %d defer_intbuff_alloc %d",
      p_mod->name, p_mod->caps.use_internal_bufs,
      p_client->defer_intbuff_alloc);

    /* Allocate internal buffers if required */
    if (p_mod->caps.use_internal_bufs && !p_client->defer_intbuff_alloc) {
      if (IMG_ERROR(module_imgbase_client_alloc_int_buf(p_client, identity))) {
        IDBG_ERROR("[%s] alloc failed %d", p_mod->name, rc);
        goto error;
      }
    }

    if (IMG_ERROR(rc)) {
      IDBG_ERROR("[%s] start failed %d", p_mod->name, rc);
      goto error;
    }
    p_client->state = IMGLIB_STATE_STARTED;
  }


  p_client->stream_on++;
  p_client->dummy_frame.private_data = NULL;
  p_client->dummy_frame.frame[0].plane[0].addr = NULL;
  p_client->dummy_frame.frame[0].plane[1].addr = NULL;
  pthread_mutex_unlock(&p_client->mutex);

  p_mod = (module_imgbase_t *)p_client->p_mod;
  if (p_mod->modparams.imgbase_client_streamon) {
    p_mod->modparams.imgbase_client_streamon(p_client);
  }

  IDBG_LOW("[%s] X", p_mod->name);
  return rc;

error:
  pthread_mutex_unlock(&p_client->mutex);

  IDBG_ERROR("%s:%d] Error, X", __func__, __LINE__);
  return rc;
}


/**
 * Function: module_imgbase_client_flush
 *
 * Description: This function is used to flush the
 *   buffers with component and imgbase client
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_flush(imgbase_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_client->comp;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;

  IDBG_LOW("E");

  pthread_mutex_lock(&p_client->mutex);

  IMG_COMP_NOTIFY(p_comp, IMG_COMP_EVENT_FLUSH);

  /* flush buffers with component */
  rc = IMG_COMP_FLUSH(p_comp, 0);

  /* flush any outstanding acks */
  if (p_mod->caps.num_overlap) {
    rc = module_imgbase_client_handle_buffer_ack(p_client,
      IMG_EVT_ACK_FLUSH, NULL);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Error in flushing acks %d", rc);
    }
  }

  img_q_flush(&p_mod->msg_thread.msg_q);
  img_q_flush(&p_client->stream_parm_q);
  img_q_flush(&p_client->meta_hold_q);

  pthread_mutex_unlock(&p_client->mutex);

  IDBG_LOW("X %d", rc);

  return rc;
}

/**
 * Function: module_imgbase_client_stop
 *
 * Description: This function is used to stop the IMGLIB_BASE
 *              client
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_stop(imgbase_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  uint32_t i;
  img_component_ops_t *p_comp = &p_client->comp;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  img_core_ops_t *p_core_ops = NULL;
  img_frame_t *p_out_frame = NULL;
  imgbase_session_data_t *p_session_data;

  p_session_data = IMGBASE_SSP(p_mod, p_client->session_id);
  if (NULL == p_session_data) {
    /* Error printed in the macro*/
    return IMG_ERR_INVALID_INPUT;
  }

  pthread_mutex_lock(&p_client->mutex);
  p_client->stream_on--;
  pthread_mutex_unlock(&p_client->mutex);

  if (!p_client->stream_on) {
    IDBG_MED("%s: Calling abort in component", __func__);
    rc = IMG_COMP_ABORT(p_comp, NULL);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] abort failed %d", __func__, __LINE__, rc);
      return rc;
    }
    pthread_mutex_lock(&p_client->mutex);

    /* Unbind dual camera */
    imgbase_client_t *p_peer_client = (imgbase_client_t *)p_client->p_peer_client;
    if (p_peer_client && !p_peer_client->stream_on) {
      if (p_client->is_binded && p_client->p_peer_comp ) {
        p_core_ops = &p_mod->core_ops;
        if (p_core_ops->unbind) {
          IDBG_MED("%s] unbind p_comp %p peer_comp %p for session %d",
            p_mod->name, p_comp, p_client->p_peer_comp, p_client->session_id);
          rc = IMG_COMP_UNBIND(p_core_ops, p_comp, p_client->p_peer_comp);
          p_client->is_binded = FALSE;
          ((imgbase_client_t *)p_client->p_peer_client)->is_binded = FALSE;
          if (IMG_ERROR(rc)) {
            IDBG_ERROR("%s:%d] unbind failed %d", __func__, __LINE__, rc);
          }
        }
      }
    }

    /* flush any outstanding acks */
    if (p_mod->caps.num_overlap) {
      rc = module_imgbase_client_handle_buffer_ack(p_client,
        IMG_EVT_ACK_FLUSH, NULL);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("%s:%d] Error in flushing acks", __func__, __LINE__);
      }
    }

    /* Release internal buffers */
    pthread_mutex_lock(&p_client->buf_mutex);
    if (p_mod->caps.use_internal_bufs) {
      for (i = 0; i < p_client->internal_buf.buf_cnt; i++) {
        if (p_client->internal_buf.buf[i].mem_handle.handle) {
          if (p_client->internal_buf.buf[i].ref_cnt > 0) {
            IDBG_HIGH("[%s] waiting for ack frame %d refcnt %d",
              p_mod->name, p_client->internal_buf.buf[i].p_frame ?
              (int32_t)p_client->internal_buf.buf[i].p_frame->frame_id : -1,
              p_client->internal_buf.buf[i].ref_cnt);
          }

          rc = img_buffer_release(&p_client->internal_buf.buf[i].mem_handle);
          if (IMG_ERROR(rc)) {
            IDBG_ERROR("[%s] Error, buf release fail", p_mod->name);
            pthread_mutex_unlock(&p_client->buf_mutex);
            pthread_mutex_unlock(&p_client->mutex);
            return rc;
          }
          p_client->internal_buf.buf[i].ref_cnt = 0;
          p_client->internal_buf.buf[i].exists_in_cache = FALSE;
          p_client->internal_buf.buf[i].is_init = FALSE;
        }
      }
    }

    p_client->cur_outbuf_cnt = 0;
    IDBG_LOW(" cur_outbuf_cnt %d", p_client->cur_outbuf_cnt);
    while ((p_out_frame = img_q_dequeue(&p_client->output_hold_q)) != NULL) {
      if (p_out_frame->private_data) {
      free(p_out_frame->private_data);
      }
      //free(p_out_frame);
    }
    img_q_flush_and_destroy(&p_client->output_hold_q);

    pthread_mutex_unlock(&p_client->buf_mutex);

    mct_list_free_all(p_client->meta_data_list, module_imglib_free_func);
    p_client->meta_data_list = NULL;

    if(p_client->native_buffer_list) {
      mct_list_free_list(p_client->native_buffer_list);
    }
    p_client->native_buffer_list =  NULL;

    mct_list_free_list(p_client->hal_meta_data_list);
    p_client->hal_meta_data_list = NULL;

    p_client->state = IMGLIB_STATE_INIT;
    img_q_flush_and_destroy(&p_mod->msg_thread.msg_q);
    img_q_flush_and_destroy(&p_client->stream_parm_q);
    img_q_flush_and_destroy(&p_client->meta_hold_q);

    p_client->is_update_valid = FALSE;
    p_client->stream_crop_valid = FALSE;
  } else {
    p_client->state = IMGLIB_STATE_STARTED;
  }

  if (p_mod->modparams.imgbase_client_streamoff) {
    p_mod->modparams.imgbase_client_streamoff(p_client);
  }

  pthread_mutex_unlock(&p_client->mutex);
  IDBG_MED("[%s] rc %d X", p_mod->name, rc);
  return rc;
}

/**
 * Function: module_imgbase_client_destroy
 *
 * Description: This function is used to destroy the imgbase client
 *
 * Arguments:
 *   @p_client: imgbase client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
void module_imgbase_client_destroy(imgbase_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = NULL;
  module_imgbase_t *p_mod;
  int i;

  if (NULL == p_client) {
    return;
  }

  p_mod = (module_imgbase_t *)p_client->p_mod;
  p_comp = &p_client->comp;
  IDBG_MED("%s_%s:%d] state %d", __func__, p_mod->name, __LINE__,
    p_client->state);

  if (IMGLIB_STATE_STARTED == p_client->state) {
    module_imgbase_client_stop(p_client);
  }

  if ((IMGLIB_STATE_INIT == p_client->state) && p_comp) {
    rc = IMG_COMP_DEINIT(p_comp);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] deinit failed %d", __func__, __LINE__, rc);
    }
    p_client->state = IMGLIB_STATE_IDLE;
  }

  if (p_mod->modparams.imgbase_client_destroy) {
    p_mod->modparams.imgbase_client_destroy(p_client);
  }

  if (IMGLIB_STATE_IDLE == p_client->state) {
    img_q_deinit(&p_client->stream_parm_q);
    img_q_deinit(&p_client->meta_hold_q);
    img_q_deinit(&p_client->output_hold_q);
    pthread_mutex_destroy(&p_client->mutex);
    pthread_mutex_destroy(&p_client->buf_mutex);
    pthread_cond_destroy(&p_client->cond);
    if (p_client->thread_ops.client_id) {
      img_thread_mgr_unreserve_threads(p_client->thread_ops.client_id);
      p_client->thread_ops.client_id = 0;
    }
    for (i = 0; i < MAX_IMGLIB_BATCH_SIZE; i++) {
      imgbase_buf_t *p_temp_buf = p_client->buf_list[i];
      if (p_temp_buf) {
        free(p_temp_buf);
        p_temp_buf = NULL;
      }
    }

    free(p_client);
    p_client = NULL;
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);

}

/**
  * Function: module_get_data_lib
  *
  * Description: This function is to get value from lib
  * Arguments:
  *   @p_appdata: imgbase client
  *   @img_get_data_lib: enum to get right data
  *   @val : value from lib
  * Return values:
  *     true on success, false on failure
  *
  * Notes: none
  **/
bool module_get_data_lib(void *p_appdata,
  uint32_t img_get_data_lib, void* val)
{

  IDBG_LOW("E");

  img_get_data_lib_type_t get_data_lib_type =  (img_get_data_lib_type_t)
    img_get_data_lib;

  mct_event_t event;
  int stream_idx;
  imgbase_stream_t *p_stream = NULL;
  bool rc = FALSE;
  int64_t ae_val = -1;

  imgbase_client_t *p_client = (void *)p_appdata;
  if (!p_client) {
    IDBG_ERROR("Error input p_client is NULL");
    return rc;
  }
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  if (!p_mod || !val) {
    IDBG_ERROR("Error input p_mod or val is NULL");
    return rc;
  }

  switch (get_data_lib_type) {
  case DEPTH_AE_VAL:
    ae_val = *(int64_t *)val;
    stream_idx = module_imgbase_find_stream_by_identity(p_client,
      p_client->divert_identity);
    if (stream_idx < 0) {
      IDBG_ERROR("%s_%s %d] Cannot find stream mapped to idx %x", __func__,
      p_mod->name, __LINE__, p_client->divert_identity);
      return rc;
    }

    p_stream = &p_client->stream[stream_idx];
    if (!p_stream) {
      IDBG_ERROR("Error input");
      return rc;
    }
    event.identity  = p_stream->identity;
    event.type      = MCT_EVENT_MODULE_EVENT;
    event.direction = MCT_EVENT_UPSTREAM;
    event.timestamp = 0;
    event.u.module_event.type = MCT_EVENT_MODULE_DEPTH_EXPOSURE_TIME;
    event.u.module_event.module_event_data = &ae_val;
    event.u.module_event.current_frame_id = p_stream->cur_frame_id;
    event.u.module_event.size = sizeof(ae_val);

    rc =  mct_port_send_event_to_peer(p_stream->p_sinkport, &event);
    if (rc == FALSE) {
      IDBG_ERROR("%s Fowarding event %d from src port failed",
      p_mod->name, event.type);
      return rc;
    }
    break;
  default:
      IDBG_ERROR("get_data_lib_type != DEPTH_AE_VAL");
    break;
  }

  IDBG_LOW("X");
  return rc;
}

/**
 * Function: module_imgbase_client_init
 *
 * Description: This function is used to initialize the imgbase
 * client
 *
 * Arguments:
 *   @p_appdata: imgbase client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int32_t module_imgbase_client_init(void *p_appdata)
{
  int32_t rc = IMG_SUCCESS;
  imgbase_client_t *p_client = (imgbase_client_t *)p_appdata;
  img_component_ops_t *p_comp = &p_client->comp;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  uint32_t deinit_rc;

  img_frame_ops_t l_frameops = {
    .get_frame = module_imgbase_client_get_frame,
    .release_frame = module_imgbase_client_release_frame,
    .dump_frame = img_dump_frame,
    .get_meta = img_get_meta,
    .set_meta = img_set_meta,
    .image_copy = img_image_copy,
    .image_scale = img_sw_downscale_2by2,
     .get_data_lib = module_get_data_lib
  };

  IDBG_MED("%s:%d] rc %d E", __func__, __LINE__, rc);
  rc = IMG_COMP_INIT(p_comp, p_client, &p_client->comp_init_params);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] init failed %d", __func__, __LINE__, rc);
    goto init_error;
  }

  rc = IMG_COMP_SET_CB(p_comp, module_imgbase_client_event_handler);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    goto error;
  }

  rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_CAPS, &p_mod->caps);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    goto error;
  }

  l_frameops.p_appdata = p_client;
  rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_FRAME_OPS, &l_frameops);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    goto error;
  }

  if ((p_mod->feature_mask == CAM_QCOM_FEATURE_LLVD) ||
    (p_mod->feature_mask == CAM_QTI_FEATURE_SW_TNR)) {
    img_lib_config_t lib_config;
    img_comp_execution_mode_t exec_mode = IMG_EXECUTION_UNKNOWN;
    int rc;

    lib_config.lib_param = IMG_ALGO_EXEC_MODE;
    lib_config.lib_data = &exec_mode;

    // Get Seemore exec mode
    rc = IMG_COMP_GET_PARAM(p_comp, QIMG_PARAM_LIB_CONFIG,
      &lib_config);
    if (IMG_SUCCEEDED(rc)) {
      p_mod->modparams.exec_mode = exec_mode;
      IDBG_HIGH("exec_mode=%d", exec_mode);
    }
  }

  QIMG_LOCK(&p_client->mutex);
  p_client->state = IMGLIB_STATE_INIT;
  QIMG_UNLOCK(&p_client->mutex);

  IDBG_HIGH("%s:%d] p_client->state %d X", __func__, __LINE__, p_client->state);
  return rc;

error:
  deinit_rc = IMG_COMP_DEINIT(p_comp);
  if (IMG_ERROR(deinit_rc)) {
    IDBG_ERROR("%s:%d] Deinit failed rc %d", __func__, __LINE__, deinit_rc);
  }
init_error:
  IDBG_ERROR("%s:%d] Transition to INIT failed", __func__, __LINE__);
  return rc;
}

/** Function: module_imgbase_client_create
 *
 * Description: This function is used to create the IMGLIB_BASE client
 *
 * Arguments:
 *   @p_mct_mod: mct module pointer
 *   @p_port: mct port pointer
 *   @identity: identity of the stream
 *   @session_id: session id
 *   @p_stream_info: pointer to the stream info
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_create(mct_module_t *p_mct_mod,
  mct_port_t *p_port,
  uint32_t session_id,
  mct_stream_info_t *p_stream_info)
{
  int rc = IMG_SUCCESS;
  imgbase_client_t *p_client = NULL;
  img_component_ops_t *p_comp = NULL;
  img_core_ops_t *p_core_ops = NULL;
  module_imgbase_t *p_mod =
    (module_imgbase_t *)p_mct_mod->module_private;
  mct_list_t *p_temp_list = NULL;
  img_init_params_t init_params;
  img_core_type_t thread_affinity[1] = {IMG_CORE_ARM};

  IDBG_MED("%s:%d]", __func__, __LINE__);
  p_client = (imgbase_client_t *)malloc(sizeof(imgbase_client_t));
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] IMGLIB_BASE client alloc failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  /* initialize the variables */
  memset(p_client, 0x0, sizeof(imgbase_client_t));
  memset(&init_params, 0x0, sizeof(img_init_params_t));

  pthread_mutex_init(&p_client->mutex, NULL);
  pthread_cond_init(&p_client->cond, NULL);
  pthread_mutex_init(&p_client->buf_mutex, NULL);
  p_client->state = IMGLIB_STATE_IDLE;
  img_q_init(&p_client->stream_parm_q, "stream_parm_q");
  img_q_init(&p_client->meta_hold_q, "meta_hold_q");
  img_q_init(&p_client->output_hold_q, "outBuf_to_comp_q");
  p_client->cur_outbuf_cnt = 0;
  IDBG_LOW(" cur_outbuf_cnt %d", p_client->cur_outbuf_cnt);

  p_core_ops = &p_mod->core_ops;
  p_comp = &p_client->comp;

  if ((p_stream_info->stream_type != CAM_STREAM_TYPE_PARM) ||
    (p_mod->caps.share_client_per_session)) {
    rc = IMG_COMP_CREATE(p_core_ops, p_comp);
    if (IMG_ERROR(rc)) {
     IDBG_ERROR("%s:%d] create failed %d", __func__, __LINE__, rc);
     goto error;
    }
  }
  /* add the client to the list */
  p_temp_list = mct_list_append(p_mod->imgbase_client, p_client,
    NULL, NULL);
  if (NULL == p_temp_list) {
    IDBG_ERROR("%s:%d] list append failed", __func__, __LINE__);
    rc = IMG_ERR_NO_MEMORY;
    goto error;
  }

  p_client->output_stream_mask = 0;
  p_mod->imgbase_client = p_temp_list;
  p_client->parent_mod = p_mod->parent_mod ? p_mod->parent_mod : p_mct_mod;
  p_client->p_mod = p_mod;
  p_port->port_private = p_client;
  p_client->num_ack = 1;
  p_client->internal_buf.type = IMG_BUFFER_ION_IOMMU;
  p_client->process_all_frames = FALSE;
  p_client->session_id = session_id;
  p_client->ion_fd = -1;
  p_client->session_client =
   (p_stream_info->stream_type == CAM_STREAM_TYPE_PARM);
  p_client->thread_job.client_id = p_mod->th_client_id;
  /* set default stream to process params */
  p_client->streams_to_process = p_mod->modparams.streams_to_process;

  IDBG_LOW("%s before_cpp %d streamstoprocess %llx offlineproc %llx client %0x",
    p_mod->name, p_client->before_cpp,p_mod->modparams.streams_to_process,
    IMG_2_MASK(CAM_STREAM_TYPE_OFFLINE_PROC), p_client );

  p_client->before_cpp = p_mod->modparams.streams_to_process &&
    ((p_mod->modparams.streams_to_process &
    (uint32_t)IMG_2_MASK(CAM_STREAM_TYPE_OFFLINE_PROC)) == 0);
  p_client->internal_buf.buf_cnt = p_mod->caps.internal_buf_cnt  ?
    MIN(p_mod->caps.internal_buf_cnt, MAX_IMGLIB_INTERNAL_BUFS) :
    DEFAULT_IMGLIB_INTERNAL_BUFS;
  p_client->bypass_enable = TRUE;

  /* Thread operations */
  p_client->thread_ops.schedule_job      = img_thread_mgr_schedule_job;
  p_client->thread_ops.get_time_of_job   = img_thread_mgr_get_time_of_job;
  p_client->thread_ops.get_time_of_joblist = img_thread_mgr_get_time_of_joblist;
  p_client->thread_ops.wait_for_completion_by_jobid =
    img_thread_mgr_wait_for_completion_by_jobid;
  p_client->thread_ops.wait_for_completion_by_joblist =
    img_thread_mgr_wait_for_completion_by_joblist;
  p_client->thread_ops.wait_for_completion_by_clientid =
    img_thread_mgr_wait_for_completion_by_clientid;

  // Add request for perflock based on stream type
  if (p_stream_info->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    p_mod->caps.is_offline_proc = true;
    IDBG_MED("Offline Processing being done");
  } else {
    p_mod->caps.is_offline_proc = FALSE;
  }

  if (p_mod->modparams.imgbase_client_created) {
    p_mod->modparams.imgbase_client_created(p_client);
  }

    /* set init params */
  p_client->comp_init_params.client_id = p_client->thread_job.client_id;
  if (p_mod->modparams.imgbase_client_init_params) {
    p_mod->modparams.imgbase_client_init_params(p_client, &init_params);
  }
  p_client->comp_init_params = init_params;

  if ((p_stream_info->stream_type != CAM_STREAM_TYPE_PARM) ||
    (p_mod->caps.share_client_per_session)) {
    if (!p_client->async_init) {
      rc = module_imgbase_client_init(p_client);
    } else {
      rc = mod_imgbase_schedule_ind_job(p_client,
        &p_client->thread_job,
        p_client->last_job_id,
        IMG_CORE_ARM,
        thread_affinity,
        TRUE,
        module_imgbase_client_init,
        p_client);
    }
    if (IMG_ERROR(rc)) {
     IDBG_ERROR("%s:%d] init %d failed %d", __func__, __LINE__,
       p_client->async_init, rc);
      goto error;
    }
  }

  IDBG_HIGH("[%s] bc %d stream_mask %x", p_mod->name, p_client->before_cpp,
    p_mod->modparams.streams_to_process);
  return rc;

error:
  if (p_client) {
    module_imgbase_client_destroy(p_client);
    p_client = NULL;
  }
  return rc;
}

/**
 * Function: module_imgbase_client_handle_buffer_ack
 *
 * Description: This function is used to handle the buffer ack
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @ack_event: ack event type
 *   @data: payload pertaining to ack_event
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_handle_buffer_ack(imgbase_client_t *p_client,
  img_ack_event_t ack_event, void* data)
{
  int rc = IMG_SUCCESS;
  uint32_t i, j;
  imgbase_buf_t *p_buf = NULL;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  int32_t idx = -1;

  switch (ack_event) {
  case IMG_EVT_ACK_HOLD: {
    isp_buf_divert_ack_t *p_ack = (isp_buf_divert_ack_t *)data;
    IDBG_MED("%s:%d] IMG_EVT_ACK_HOLD, frame %d", __func__, __LINE__,
      p_ack->frame_id);

    /* Do not hold buffers which are not consumed */
    rc = IMG_ERR_NOT_FOUND;
    pthread_mutex_lock(&p_client->buf_mutex);
    for (i = 0; i < MAX_IMGLIB_BATCH_SIZE; i++) {
      p_buf = p_client->buf_list[i];
      if (!p_buf || p_buf->frame.frame_id != p_ack->frame_id) {
        continue;
      }

      /* ACK is late, do not hold it */
      if ((p_buf->queued_cnt == p_mod->caps.num_input) &&
        (p_buf->dequeued_cnt == p_mod->caps.num_input)) {
        IDBG_HIGH("%s:%d] Forward ACK %d since it is no longer needed",
          __func__, __LINE__, p_ack->frame_id);
        // We will be doing divert for this buffer, set the flags accordingly
        // in buf_list index, so that these entries can be free.
        p_buf->divert_done = TRUE;
        p_buf->ack.frame_id = 0;
        pthread_mutex_unlock(&p_client->buf_mutex);
        goto error;
      }

      /* Ack was previously received */
      if (p_buf->ack.ack_received_cnt > 0) {
        p_buf->ack.divert_ack.buffer_access |= p_ack->buffer_access;

        IDBG_HIGH("CAMCACHE : ack buffer_access=%s(0x%x)",
          (p_ack->buffer_access == 0) ? "NONE" :
          (p_ack->buffer_access == CPU_HAS_READ) ? "READ" :
          (p_ack->buffer_access == CPU_HAS_WRITTEN) ? "WRITTEN" :
          "READ_WRITTEN", p_ack->buffer_access);

        p_buf->ack.ack_received_cnt++;
        rc = IMG_SUCCESS;
        break;
      }

      /* New ACK, hold it */
      for (j = 0; j < p_client->stream_cnt; j++) {
        if (p_ack->identity == p_client->stream[j].identity) {
          IDBG_MED("%s:%d] Hold ACK %d", __func__, __LINE__, p_ack->frame_id);
          p_buf->ack.frame_id = p_ack->frame_id;
          p_buf->ack.identity = p_ack->identity;
          p_buf->ack.divert_ack = *p_ack;
          p_buf->ack.ack_received_cnt++;
          rc = IMG_SUCCESS;
          break;
        }
      }
    }
    pthread_mutex_unlock(&p_client->buf_mutex);
    PRINT_BUF_LIST(p_client);
    break;
  }
  case IMG_EVT_ACK_TRY_RELEASE: {
    imgbase_buf_t *p_buf = (imgbase_buf_t *)data;
    IDBG_MED("%s:%d] IMG_EVT_ACK_TRY_RELEASE, frame %d, %d %d", __func__,
      __LINE__, p_buf->frame.frame_id, p_buf->queued_cnt, p_buf->dequeued_cnt);

    isp_buf_divert_ack_t *p_ack = &p_buf->ack.divert_ack;
    rc = module_imgbase_client_forward_ack(p_client, p_ack);
    if (IMG_SUCCEEDED(rc)) {
      IDBG_HIGH("%s:%d] ACK release successful for frame %d",
        __func__, __LINE__, p_ack->frame_id);
      p_buf->ack.frame_id = 0;
      p_buf->ack.ack_received_cnt = 0;
      break;
    }

    PRINT_BUF_LIST(p_client);
    break;
  }
  case IMG_EVT_ACK_FLUSH: {
    IDBG_MED("%s:%d] IMG_EVT_ACK_FLUSH", __func__, __LINE__);

    pthread_mutex_lock(&p_client->buf_mutex);
    for (i = 0; i < MAX_IMGLIB_BATCH_SIZE; i++) {
      p_buf = p_client->buf_list[i];
      if (p_buf && p_buf->ack.frame_id > 0) {
        isp_buf_divert_ack_t *p_ack = &p_buf->ack.divert_ack;
        rc = module_imgbase_client_forward_ack(p_client, p_ack);
        if (IMG_SUCCEEDED(rc)) {
          IDBG_HIGH("%s:%d] Flush outstanding ack for frame %d", __func__,
            __LINE__, p_buf->ack.frame_id);
          p_buf->ack.frame_id = 0;
          p_buf->ack.ack_received_cnt = 0;
        }
      }
    }
    pthread_mutex_unlock(&p_client->buf_mutex);
    PRINT_BUF_LIST(p_client);
    break;
  }
  case IMG_EVT_ACK_FORCE_RELEASE: {
    img_internal_bufnode_t *p_intbuf;
    isp_buf_divert_t *p_ack = (isp_buf_divert_t *)data;
    IDBG_LOW("[%s] IMG_EVT_ACK_FORCE_RELEASE, frameid %d buf id %x event id 0x%x",
      p_mod->name,  p_ack->buffer.sequence, p_ack->identity,
      p_client->divert_identity);

    isp_buf_divert_ack_t buff_divert_ack;
    memset(&buff_divert_ack, 0, sizeof(buff_divert_ack));
    buff_divert_ack.buf_idx = p_ack->buffer.index;
    buff_divert_ack.is_buf_dirty = 1;
    buff_divert_ack.identity = p_client->divert_identity;
    buff_divert_ack.buf_identity = p_ack->identity;
    buff_divert_ack.frame_id = p_ack->buffer.sequence;
    buff_divert_ack.channel_id = p_ack->channel_id;
    buff_divert_ack.meta_data = p_ack->meta_data;
    buff_divert_ack.buffer_access = p_ack->buffer_access;

    IDBG_HIGH("CAMCACHE : ACK_FORCE_RELEASE buffer_access=%s(0x%x)",
      (p_ack->buffer_access == 0) ? "NONE" :
      (p_ack->buffer_access == CPU_HAS_READ) ? "READ" :
      (p_ack->buffer_access == CPU_HAS_WRITTEN) ? "WRITTEN" :
      "READ_WRITTEN", p_ack->buffer_access);

    if(p_client->is_bayer) {
      buff_divert_ack.bayerdata = 1;
    }

    /* If overlap buffers are enabled, hold the ACK */
    if (p_mod->caps.num_overlap) {
      rc = module_imgbase_client_handle_buffer_ack(p_client,
        IMG_EVT_ACK_HOLD, &buff_divert_ack);
      if (IMG_SUCCEEDED(rc)) {
        break;
      }
    }

    /* Else forward the ack */
    rc = module_imgbase_client_forward_ack(p_client, &buff_divert_ack);
    if (IMG_SUCCEEDED(rc)) {
      IDBG_LOW("[%s] Send ACK successful frame %d", p_mod->name,
        p_ack->buffer.sequence);
    }

    /* If internal buffers are used, indicate ACK was forwarded */
    if (p_mod->caps.use_internal_bufs) {
      idx = -1;
      for (i = 0; i < p_client->internal_buf.buf_cnt; i++) {
        p_intbuf = &p_client->internal_buf.buf[i];
        if (p_intbuf->p_frame &&
          (p_ack->buffer.sequence == p_intbuf->p_frame->frame_id)) {
          idx = i;
          break;
        }
      }
      if (idx < 0) {
        IDBG_ERROR("%s:%d] Error, internal buf not found for frame %d",
          __func__, __LINE__, p_ack->buffer.sequence);
        goto error;
      }
      p_client->internal_buf.buf[idx].input_ack_held = FALSE;
      PRINT_INTERNAL_BUF_LIST(p_client);
    }

    break;
  }
  case IMG_EVT_ACK_FREE_INTERNAL_BUF: {
    isp_buf_divert_ack_t *p_ack = (isp_buf_divert_ack_t *)data;
    uint32_t ack_frame_id = p_ack->frame_id;
    img_internal_bufnode_t *p_intbuf;
    imgbase_buf_t *p_imgbase_buf;
    int stream_idx = 0;
    imgbase_stream_t *p_stream = NULL;

    stream_idx = module_imgbase_find_stream_by_identity(p_client, p_ack->identity);
    if (stream_idx < 0) {
      IDBG_ERROR("[%s] Cannot find stream mapped to idx %x",p_mod->name,
        p_client->divert_identity);
      goto error;
    }

    p_stream = &p_client->stream[stream_idx];

    if (!p_mod->caps.use_internal_bufs) {
      IDBG_ERROR("[%s] [IMG_BUG_DBG_OUT] Error, internal bufs not used",
        p_mod->name);
      goto error;
    }

    idx = -1;
    for (i = 0; i < p_client->internal_buf.buf_cnt; i++) {
      p_intbuf = &p_client->internal_buf.buf[i];
      if (p_intbuf->p_frame && (ack_frame_id == p_intbuf->p_frame->frame_id)) {
        idx = i;
        break;
      }
    }
    if (idx < 0) {
      if ( p_stream->eis_config.last_frameid != 0 &&
           p_stream->eis_config.last_frameid < p_ack->frame_id) {
        IDBG_LOW("[%s] [IMG_BUG_DBG_OUT] No Error, frameid %d is after last frame id %d ",
          p_mod->name, ack_frame_id, p_stream->eis_config.last_frameid);
        return IMG_ERR_NOT_FOUND;
      }

      IDBG_ERROR("[%s] [IMG_BUG_DBG_OUT] Error, intbuf not found for %d",
        p_mod->name, ack_frame_id);
      goto error;
    }

    p_imgbase_buf = (imgbase_buf_t *)p_intbuf->p_frame->private_data;
    IDBG_MED("[%s] [IMG_BUF_DBG_OUT_f%d_b%d_i%x] FREE_INTERNAL_BUF",
      p_mod->name,
      p_intbuf->p_frame->frame_id,
      p_intbuf->p_frame->idx,
      p_imgbase_buf->buf_divert.identity);
    if (p_imgbase_buf) {
      free(p_imgbase_buf);
      p_intbuf->p_frame = NULL;
    }

    IDBG_HIGH("CAMCACHE : ACK_FREE_INTERNAL_BUF buffer_access=%s(0x%x)",
      (p_ack->buffer_access == 0) ? "NONE" :
      (p_ack->buffer_access == CPU_HAS_READ) ? "READ" :
      (p_ack->buffer_access == CPU_HAS_WRITTEN) ? "WRITTEN" :
      "READ_WRITTEN", p_ack->buffer_access);

    if (p_ack->buffer_access) {
#if 0
      // Flush is not required.. but invalidate is required here.
      /* cache invalidate */
      rc = img_buffer_cacheops(&p_intbuf->mem_handle,
        IMG_CACHE_INV, IMG_INTERNAL);

      if (IMG_ERROR(rc)) {
        IDBG_ERROR("[%s] [IMG_BUG_DBG_OUT] Error, cache invalidate fail",
          p_mod->name);
      }
#else
      p_intbuf->exists_in_cache = TRUE;
#endif
    } else {
      p_intbuf->exists_in_cache = FALSE;
    }

    p_intbuf->ref_cnt--;
    PRINT_INTERNAL_BUF_LIST(p_client);

    break;
  }
  default:
    break;
  }


  return rc;

error:
  return IMG_ERR_GENERAL;
}
