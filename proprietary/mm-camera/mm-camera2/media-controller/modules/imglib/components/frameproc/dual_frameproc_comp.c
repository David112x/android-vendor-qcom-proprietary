/**********************************************************************
*  Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "chromatix.h"
#include "dual_frameproc_comp.h"

/**
 * CONSTANTS and MACROS
 **/

/** DEFAULT_AF_FPS:
 *
 * Defines default FPS for AF
 *
 * Returns default FPS for AF
 **/
#define DEFAULT_AF_FPS 30

/** MAX_CACHED_META_FRAME:
 *
 * Defines maximum number of cached meta
 *
 * Returns maximum number of cached meta
 **/
#define MAX_CACHED_META_FRAME 4

/** DEFAULT_FRAME_INTERVAL_ADJUSTMENT
 *
 * Defines default frame interval adjustment
 *
 * Returns default frame interval adjustment
 **/
#define DEFAULT_FRAME_INTERVAL_ADJUSTMENT 3000LL

/** time_interval_t
*   @TIME_INTERVAL_WITHIN_RANGE: 2 compared time stamps within the
*      specified range
*   @TIMESTAMP_TOO_OLD: first time stamp is older than second by the
*     specified range
*   @TIMESTAMP_TOO_NEW: first time stamp is newer than second by the
*     specified range
*
**/
typedef enum {
   TIME_INTERVAL_WITHIN_RANGE,
   TIMESTAMP_TOO_OLD,
   TIMESTAMP_TOO_NEW,
} time_interval_t;

/**
 * Function: dual_frameproc_comp_find_inst
 *
 * Description: This method is used to find the comp inst
 *
 * Arguments:
 *   @p_data: data in the list
 *   @p_input: input data to be seeked
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static int dual_frameproc_comp_find_inst(void *p_data, void *p_input)
{

  dual_frameproc_comp_inst_t *stored_param =
    (dual_frameproc_comp_inst_t *)p_data;
  dual_frameproc_comp_inst_t *target = (dual_frameproc_comp_inst_t *)p_input;

  return (stored_param == target) ? TRUE : FALSE;
}

/**
 * Function: dual_frameproc_comp_check_timestamp_interval
 *
 * Description: This method is used to find if the timestamps are in range
 *
 * Arguments:
 *   @timestamp1: timestamp1
 *   @timestamp2: timestamp2
 *   @timeinterval: timeinterval
 *
 * Return values:
 *     TIME_INTERVAL_WITHIN_RANGE,
 *     TIMESTAMP_TOO_OLD,
 *     TIMESTAMP_TOO_NEW,
 *
 * Notes: none
 **/
static int dual_frameproc_comp_check_timestamp_interval(
  uint64_t timestamp1, uint64_t timestamp2, uint64_t timeinterval)
{
  time_interval_t rc;

  if (timestamp1 > timestamp2) {
    if ((timestamp1 - timestamp2) < timeinterval) {
      rc = TIME_INTERVAL_WITHIN_RANGE;
    } else {
      rc = TIMESTAMP_TOO_NEW;
    }
  } else {
    if ((timestamp2 - timestamp1) < timeinterval) {
      rc = TIME_INTERVAL_WITHIN_RANGE;
    } else {
      rc = TIMESTAMP_TOO_OLD;
    }
  }

  return rc;
}

/**
 * Function: dual_frameproc_comp_init
 *
 * Description: Initializes the dual_frameproc component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_userdata - the handle which is passed by the client
 *   p_data - The pointer to the parameter which is required during the
 *            init phase
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int dual_frameproc_comp_init(void *handle, void* p_userdata, void *p_data)
{
  dual_frameproc_comp_inst_t *p_inst = (dual_frameproc_comp_inst_t *)handle;
  dual_frameproc_comp_t *p_comp = NULL;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] p_ops->handle %p", __func__, __LINE__, handle);

  if (!p_inst) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  p_comp = p_inst->p_comp_body;
  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  if (p_comp->inst_cnt == 1) {
    status = p_comp->b.ops.init(&p_comp->b, p_userdata, p_data);
    if (status < 0) {
      IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, status);
      return status;
    }
  }

  img_q_init(&p_inst->inputQ, "dual_comp_inputQ");
  img_q_init(&p_inst->outputQ, "dual_comp_outputQ");
  img_q_init(&p_inst->outBufQ, "dual_comp_outBufQ");
  img_q_init(&p_inst->metaQ, "dual_comp_metaQ");
  p_inst->p_userdata = p_userdata;

  return status;
}

/**
 * Function: dual_frameproc_can_wait
 *
 * Description: Queue function to check if abort is issued
 *
 * Input parameters:
 *   p_userdata - The pointer to dual_frameproc component
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
int dual_frameproc_can_wait(void *p_userdata)
{
  dual_frameproc_comp_t *p_comp = (dual_frameproc_comp_t *)p_userdata;
  img_component_t *p_base = &p_comp->b;
  return !((p_base->state == IMG_STATE_STOP_REQUESTED) ||
    (p_base->state == IMG_STATE_STOPPED));
}

/**
 * Function: dual_frameproc_send_buffer_done
 *
 * Description: Function to send the buffer done to client
 *
 * Input parameters:
 *   p_inst - The pointer to the component instance
 *   p_frame - pointer to frame buffer
 *   event_type - img event type
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void dual_frameproc_send_buffer_done(dual_frameproc_comp_inst_t *p_inst,
  void *p_data, img_event_type event_type)
{
  img_frame_t *p_frame = NULL;
  img_meta_t *p_meta = NULL;

  if (p_inst && p_data) {
    IDBG_MED("%s:%d]p_inst %p p_data %p", __func__, __LINE__, p_inst, p_data);
    switch (event_type) {
    case QIMG_EVT_IMG_BUF_DONE:
    case QIMG_EVT_IMG_OUT_BUF_DONE:
      p_frame = (img_frame_t *)p_data;
      IMG_SEND_EVENT_PYL(p_inst, event_type, p_frame, p_frame);
      break;
    case QIMG_EVT_META_BUF_DONE:
      p_meta = (img_meta_t *)p_data;
      IMG_SEND_EVENT_PYL(p_inst, event_type, p_meta, p_meta);
      break;
    default:
      IDBG_ERROR("%s:%d]Wrong type %d", __func__, __LINE__, event_type);
    }
  }
}

/**
 * Function: dual_frameproc_flush_jobs
 *
 * Description: Function to flush yet to be executed
 *
 * Input parameters:
 *   p_main_inst - The pointer to the main inst
 *   p_aux_inst - The pointer to the aux inst
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void dual_frameproc_flush_jobs(dual_frameproc_comp_inst_t *p_main_inst,
  dual_frameproc_comp_inst_t *p_aux_inst)
{
  IMG_UNUSED(p_aux_inst);
  IDBG_MED("%s:%d] E ", __func__, __LINE__);
  if (p_main_inst) {
    if (p_main_inst->thread_job.dep_job_ids) {
      free(p_main_inst->thread_job.dep_job_ids);
      p_main_inst->thread_job.dep_job_ids = NULL;
    }
  }
}

/**
 * Function: dual_frameproc_flush_buffers
 *
 * Description: Function to flush the queue buffers
 *
 * Input parameters:
 *   p_userdata - The pointer to the instance object
 *   p_params - NULL
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static int dual_frameproc_flush_buffers(void *p_userdata, void *p_params)
{
  IMG_UNUSED(p_params);
  dual_frameproc_comp_t *p_comp = NULL;
  img_component_t *p_base = NULL;
  dual_frameproc_comp_inst_t *p_inst = (dual_frameproc_comp_inst_t *)p_userdata;
  dual_frameproc_comp_inst_t *p_main_inst = NULL;
  dual_frameproc_comp_inst_t *p_aux_inst = NULL;
  img_frame_t *p_frame = NULL;
  img_meta_t *p_meta = NULL;
  int rc, i = 0;

  IDBG_HIGH("%s:%d] p_inst %p", __func__, __LINE__, p_inst);
  if (!p_inst) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  if (p_inst->master_flag) {
    p_main_inst = p_inst;
    p_aux_inst = p_inst->p_peer_inst;
  } else {
    p_aux_inst = p_inst;
  }

  if (p_main_inst) {

    p_comp = p_inst->p_comp_body;
    if (!p_comp) {
      IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }

    p_base = &p_comp->b;
    if (p_main_inst->prev_job_id) {
      IDBG_MED("%s:%d] before wait", __func__, __LINE__);
      rc = p_base->thread_ops.wait_for_completion_by_jobid(
        p_main_inst->prev_job_id, 100);

      IDBG_MED("%s:%d] after wait rc %d, p_main_inst->unbind_flag %d",
        __func__, __LINE__, rc,
        p_main_inst->unbind_flag);
      p_main_inst->prev_job_id = 0;
    }
    //Flush ping pong buffer
    pthread_mutex_lock(&p_main_inst->buf_mutex);
    for (i = 0 ; i < 2; i++) {
      if (p_main_inst->dual_frame_buf[i].is_filled){
        dual_frameproc_send_buffer_done(p_main_inst,
          (void *)p_main_inst->dual_frame_buf[i].p_main_buf, QIMG_EVT_IMG_BUF_DONE);
        dual_frameproc_send_buffer_done(p_aux_inst,
          (void *)p_main_inst->dual_frame_buf[i].p_aux_buf, QIMG_EVT_IMG_BUF_DONE);
        p_main_inst->dual_frame_buf[i].is_busy = 0;
        p_main_inst->dual_frame_buf[i].is_filled = 0;
      }
    }
    if (p_main_inst->cached_flag) {
      dual_frameproc_send_buffer_done(p_main_inst,
          (void *)p_main_inst->p_cached_frame, QIMG_EVT_IMG_BUF_DONE);
      p_main_inst->cached_flag = 0;
      p_main_inst->p_cached_frame = NULL;
    }
    pthread_mutex_unlock(&p_main_inst->buf_mutex);

    while ((p_frame = img_q_dequeue(&p_main_inst->outBufQ)) != NULL) {
      IMG_SEND_EVENT_PYL(p_main_inst, QIMG_EVT_IMG_OUT_BUF_DONE,
        p_frame, p_frame);
    }

    while ((p_meta = img_q_dequeue(&p_main_inst->metaQ)) != NULL) {
      IMG_SEND_EVENT_PYL(p_main_inst, QIMG_EVT_META_BUF_DONE,
        p_meta, p_meta);
    }
  }
  if (p_aux_inst) {
    pthread_mutex_lock(&p_aux_inst->buf_mutex);
    if (p_aux_inst->cached_flag) {
      dual_frameproc_send_buffer_done(p_aux_inst,
          (void *)p_aux_inst->p_cached_frame, QIMG_EVT_IMG_BUF_DONE);
      p_aux_inst->cached_flag = 0;
      p_aux_inst->p_cached_frame = NULL;
    }
    pthread_mutex_unlock(&p_aux_inst->buf_mutex);
  }
  return IMG_SUCCESS;
}


/**
 * Function: dual_frameproc_copy_img_frame
 *
 * Description: Function to copy img frame
 *
 * Input parameters:
 *   p_dest - The pointer to the destination frame
 *   p_src - The pointer to the source frame
 *   format - Frame format
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void __unused dual_frameproc_copy_img_frame(img_frame_t *p_dest,
  img_frame_t *p_src, dcrf_frame_format_type format)
{
  uint8_t *tmp = p_dest->frame[0].plane[0].addr;
  if (tmp) {
    memcpy(p_dest, p_src, sizeof(img_frame_t));
    p_dest->frame[0].plane[0].addr = tmp;

    if(DCRF_FORMAT_YUYV_YVYU == format) {
      p_dest->frame_cnt = 1;
      memcpy(p_dest->frame[0].plane[0].addr,
        p_src->frame[0].plane[0].addr,
        (size_t)(p_src->frame[0].plane[0].length * 2));
    }
    else {
      memcpy(p_dest->frame[0].plane[0].addr,
        p_src->frame[0].plane[0].addr,
        p_src->frame[0].plane[0].length);
    }
  }
}

/**
 * Function: dual_frameproc_comp_process_job
 *
 * Description: DCRF process
 *
 * Input parameters:
 *   p_userdata - The pointer to the component handle.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int dual_frameproc_comp_process_job(void *p_userdata)
{
  dual_frameproc_comp_inst_t *p_inst =
    (dual_frameproc_comp_inst_t *)p_userdata;
  dual_frameproc_comp_t *p_comp = NULL;
  dual_frameproc_comp_inst_t *p_main_inst = NULL;
  dual_frameproc_comp_inst_t *p_aux_inst = NULL;
  img_component_t *p_base = NULL;
  img_frame_t *p_in_frame[2];
  img_meta_t *p_meta[1];
  uint32_t idx = 0xff;
  uint32_t rc = 0;
  int32_t ret = IMG_SUCCESS;

  if (!p_inst) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  p_main_inst = p_inst;
  p_aux_inst = p_main_inst->p_peer_inst;
  if (p_inst->unbind_flag) {
    IDBG_HIGH("%s:%d] Unbinded, no need to proceed", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  p_comp = p_inst->p_comp_body;
  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_MED("%s:%d] E p_comp %p ", __func__, __LINE__, p_comp);
  p_base = &p_comp->b;

  if (p_base->flush_in_progress) {
    IDBG_HIGH("%s:%d] Flush in progress", __func__, __LINE__);
    return IMG_SUCCESS;
  }
  pthread_mutex_lock(&p_inst->buf_mutex);
  if (p_inst->dual_frame_buf[0].is_busy) {
    idx = 0;
  } else if (p_inst->dual_frame_buf[1].is_busy){
    idx = 1;
  }

  IDBG_MED("%s:%d] Buf idx %d ", __func__, __LINE__, idx);
  /* only run the algorithm when there is buf set to busy */
  if (idx < 2) {
    p_in_frame[0] = p_inst->dual_frame_buf[idx].p_main_buf;
    p_in_frame[1] = p_inst->dual_frame_buf[idx].p_aux_buf;
    p_meta[0] = p_inst->dual_frame_buf[idx].p_meta;
    pthread_mutex_unlock(&p_inst->buf_mutex);

    if (!p_in_frame[0] || !p_in_frame[1] || !p_meta[0]) {
      IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }

    if (p_comp->p_lib->img_algo_process) {
      ret = p_comp->p_lib->img_algo_process(p_comp->p_algocontext,
        p_in_frame,
        2,
        NULL,
        0,
        p_meta,
        1);
    }

    pthread_mutex_lock(&p_inst->buf_mutex);

    p_inst->dual_frame_buf[idx].is_busy = 0;
    p_inst->dual_frame_buf[idx].is_filled = 0;
    p_inst->cur_buf_idx = (idx + 1) % 2;
    if (p_inst->dual_frame_buf[p_inst->cur_buf_idx].is_filled &&
      !p_inst->unbind_flag && !p_base->flush_in_progress) {
      /* schedule job when there is data filled in Ping-Pong buffer */
      rc = p_base->thread_ops.schedule_job(&p_inst->thread_job);
      if (!rc) {
        IDBG_ERROR("%s:%d] Error schedule job", __func__, __LINE__);
        p_inst->dual_frame_buf[p_inst->cur_buf_idx].is_busy = 0;
      } else {
        p_inst->processing_timestamp =
          p_inst->dual_frame_buf[p_inst->cur_buf_idx].p_main_buf->timestamp;
        p_inst->p_peer_inst->processing_timestamp =
          p_inst->dual_frame_buf[p_inst->cur_buf_idx].p_aux_buf->timestamp;
        p_inst->dual_frame_buf[p_inst->cur_buf_idx].is_busy = 1;
        p_inst->prev_job_id = rc;
        IDBG_HIGH("%s:%d] scheduled job id %d", __func__, __LINE__, rc);
      }
    } else {
      if (p_base->flush_in_progress) {
        IDBG_MED("%s %d: Flush in progress", __func__, __LINE__);
      }
    }

    /* Send DCRF results */
    memcpy(&p_inst->dcrf_result, &p_meta[0]->dcrf_result,
      sizeof(img_dcrf_output_result_t));

    if (IMG_ERROR(ret)) {
      p_inst->dcrf_result.status = IMG_ERR_GENERAL;
    }
    IDBG_MED("%s:%d] sending DCRF result id=%d, timestamp=%lld, distance=%d",
      __func__, __LINE__,
      p_inst->dcrf_result.frame_id,
      p_inst->dcrf_result.timestamp,
      p_inst->dcrf_result.distance_in_mm);

    IMG_SEND_EVENT(p_inst, QIMG_EVT_DCRF_DONE);

    dual_frameproc_send_buffer_done(p_main_inst,
      (void *)p_in_frame[0], QIMG_EVT_IMG_BUF_DONE);
    dual_frameproc_send_buffer_done(p_aux_inst,
      (void *)p_in_frame[1], QIMG_EVT_IMG_BUF_DONE);
    dual_frameproc_send_buffer_done(p_main_inst,
      (void *)p_meta[0], QIMG_EVT_META_BUF_DONE);

  }
  pthread_mutex_unlock(&p_inst->buf_mutex);
  IDBG_MED("%s:%d] X ", __func__, __LINE__);

  return IMG_SUCCESS;
}

/**
 * Function: dual_frameproc_prepare_buffer
 *
 * Description: Prepare data for dcrf ping-pong buffer
 *
 * Input parameters:
 *   p_msg - The pointer to msg
 *
 * Return values:
 *     NULL
 *
 * Notes: none
 **/
int dual_frameproc_prepare_buffer(img_msg_t *p_msg)
{
  dual_frameproc_comp_t *p_comp = NULL;
  img_component_t *p_base = NULL;
  dual_frameproc_comp_inst_t *p_main_inst = NULL;
  dual_frameproc_comp_inst_t *p_aux_inst = NULL;
  img_frame_t *p_main_frame = NULL;
  img_frame_t *p_aux_frame = NULL;
  img_meta_t *p_meta = NULL;
  int status = IMG_SUCCESS;
  uint32_t rc = 0;
  int i = 0;
  uint32_t next_buf_idx;
  img_dual_cam_init_params_t init_params;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  switch (p_msg->type) {
  case IMG_MSG_BUNDLE:
    p_main_inst = (dual_frameproc_comp_inst_t *)p_msg->p_sender;
    p_aux_inst = p_main_inst->p_peer_inst;
    p_comp = p_main_inst->p_comp_body;
    p_base = &p_comp->b;
    p_main_frame = p_msg->bundle.p_input[0];
    p_aux_frame = p_msg->bundle.p_input[1];
    p_meta = p_msg->bundle.p_meta[0];
    status = IMG_SUCCESS;
    if (p_base->flush_in_progress) {
      IDBG_MED("%s:%d] Flush in progress", __func__, __LINE__);
      return IMG_ERR_BUSY;
    }

    pthread_mutex_lock(&p_main_inst->buf_mutex);
    p_main_inst->busy_count = p_main_inst->dual_frame_buf[0].is_busy +
      p_main_inst->dual_frame_buf[1].is_busy;
    if (p_main_inst->busy_count >=  2) {
      IDBG_ERROR("%s:%d] Process is busy", __func__, __LINE__);
      status = IMG_ERR_BUSY;
      pthread_mutex_unlock(&p_main_inst->buf_mutex);
      break;
    }
    next_buf_idx = (p_main_inst->cur_buf_idx + 1) % 2;
    if (p_main_inst->dual_frame_buf[next_buf_idx].is_busy == 0) {
      p_main_inst->cur_buf_idx = next_buf_idx;
    }

    if (!p_main_frame || !p_aux_frame || !p_meta) {
      IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
      status = IMG_ERR_INVALID_INPUT;
    } else if ((p_main_frame->timestamp != p_main_inst->current_timestamp)
      || (p_aux_frame->timestamp != p_aux_inst->current_timestamp)) {
      IDBG_HIGH("%s:%d] Frame overwritten", __func__, __LINE__);
      status = IMG_ERR_INVALID_INPUT;
    }

    if (IMG_SUCCEEDED(status)) {
      pthread_mutex_lock(&p_aux_inst->buf_mutex);

      init_params = p_main_inst->init_params;

      /* send buf done if overwrite the cached buffer */
      if (p_main_inst->dual_frame_buf[p_main_inst->cur_buf_idx].is_filled) {
        pthread_mutex_lock(&p_main_inst->mutex);
        img_frame_t *p_cached_frame =
          p_main_inst->dual_frame_buf[p_main_inst->cur_buf_idx].p_main_buf;
        if (p_cached_frame) {
          IDBG_LOW("%s: %d: Sending buf done for frame %d", __func__, __LINE__,
            p_cached_frame->frame_id);
          dual_frameproc_send_buffer_done(p_main_inst,
            (void *)p_cached_frame, QIMG_EVT_IMG_BUF_DONE);
        }
        pthread_mutex_unlock(&p_main_inst->mutex);
        pthread_mutex_lock(&p_aux_inst->mutex);
        p_cached_frame =
          p_main_inst->dual_frame_buf[p_main_inst->cur_buf_idx].p_aux_buf;
        if (p_cached_frame) {
          IDBG_LOW("%s: %d: Sending buf done for frame %d", __func__, __LINE__,
            p_cached_frame->frame_id);
          dual_frameproc_send_buffer_done(p_aux_inst,
            (void *)p_cached_frame, QIMG_EVT_IMG_BUF_DONE);
        }
        pthread_mutex_unlock(&p_aux_inst->mutex);
        dual_frameproc_send_buffer_done(p_main_inst,
          (void *)(&p_main_inst->dual_frame_buf[p_main_inst->cur_buf_idx]
          .p_meta), QIMG_EVT_META_BUF_DONE);

      }

      p_main_inst->dual_frame_buf[p_main_inst->cur_buf_idx].p_main_buf =
        p_main_frame;
      p_main_inst->dual_frame_buf[p_main_inst->cur_buf_idx].p_aux_buf =
        p_aux_frame;
      for (i = 0; i < IMG_MAX_FOV; i++) {
        memcpy(&p_meta->dcrf_runtime_params.fov_params_aux[i],
          &p_aux_inst->fov_params_aux[i],
          sizeof(img_fov_t));
      }
      p_main_inst->dual_frame_buf[p_main_inst->cur_buf_idx].p_meta = p_meta;
      p_main_inst->dual_frame_buf[p_main_inst->cur_buf_idx].is_filled = 1;
      p_main_inst->filled_frame_id = p_main_frame->frame_id;
      p_aux_inst->filled_frame_id = p_aux_frame->frame_id;
      p_main_inst->cached_flag = 0;
      p_aux_inst->cached_flag = 0;
      IDBG_MED("%s:%d] cur_buf_idx %d, main_timestamp %lld, "
        "aux_timestamp %lld", __func__, __LINE__,
        p_main_inst->cur_buf_idx, p_main_frame->timestamp,
        p_aux_frame->timestamp);

      if (p_main_inst->busy_count == 0 && !p_main_inst->unbind_flag &&
        !p_comp->b.flush_in_progress) {
        /* schedule job when there is no busy job, and instances are still binded */
        p_main_inst->dual_frame_buf[p_main_inst->cur_buf_idx].is_busy = 1;
        rc = p_base->thread_ops.schedule_job(&p_main_inst->thread_job);
        if (!rc) {
          IDBG_ERROR("%s:%d] Error schedule job", __func__, __LINE__);
          p_main_inst->dual_frame_buf[p_main_inst->cur_buf_idx].is_busy = 0;
        } else {
          p_main_inst->processing_timestamp =
            p_main_inst->dual_frame_buf[p_main_inst->cur_buf_idx]
            .p_main_buf->timestamp;
          p_aux_inst->processing_timestamp =
            p_main_inst->dual_frame_buf[p_main_inst->cur_buf_idx]
            .p_aux_buf->timestamp;
          p_main_inst->prev_job_id = rc;
          IDBG_HIGH("%s:%d] scheduled job id %d", __func__, __LINE__, rc);
        }
      }

      pthread_mutex_unlock(&p_aux_inst->buf_mutex);
    }
    pthread_mutex_unlock(&p_main_inst->buf_mutex);

    break;
  default:;
  }

  return status;
}

/**
 * Function: dual_frameproc_comp_get_param
 *
 * Description: Gets dual_frameproc parameters
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int dual_frameproc_comp_get_param(void *handle, img_param_type param,
  void *p_data)
{
  dual_frameproc_comp_inst_t *p_inst = (dual_frameproc_comp_inst_t *)handle;
  dual_frameproc_comp_t *p_comp = NULL;
  int status = IMG_SUCCESS;

  if (!p_inst) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  p_comp = p_inst->p_comp_body;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error invalid p_comp", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  status = p_comp->b.ops.get_parm(&p_comp->b, param, p_data);

  switch (param) {
  case QIMG_DCRF_RESULT: {
    IDBG_MED("%s:%d] QIMG_DCRF_RESULT p_inst %p", __func__, __LINE__, p_inst);
    img_dcrf_output_result_t *l_result = (img_dcrf_output_result_t *)p_data;
    *l_result = p_inst->dcrf_result;
  }
  break;
  default:;
  }

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return status;
}

/**
 * Function: dual_frameproc_comp_set_param
 *
 * Description: Set dual_frameproc parameters
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in dual_frameproc.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int dual_frameproc_comp_set_param(void *handle, img_param_type param,
  void *p_data)
{
  dual_frameproc_comp_inst_t *p_inst = (dual_frameproc_comp_inst_t *)handle;
  dual_frameproc_comp_t *p_comp = NULL;
  img_dual_cam_init_params_t *p_init_params = NULL;
  dual_frameproc_lib_info_t *p_dual_frameproc_lib = NULL;
  int status = IMG_SUCCESS;

  if (!p_inst) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  p_comp = p_inst->p_comp_body;
  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  IDBG_MED("%s:%d] p_comp %p ", __func__, __LINE__, p_comp);

  switch (param) {
  case QIMG_PARAM_DCRF: {
    if (NULL == p_data) {
      IDBG_ERROR("%s:%d] invalid DCRF params", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_init_params = (img_dual_cam_init_params_t *)p_data;
    IDBG_MED("%s:%d] Receive DCRF init param %p p_inst %p p_comp %p",
      __func__, __LINE__, p_data, p_inst, p_comp);

    pthread_mutex_lock(&p_inst->mutex);
    memcpy(&p_inst->init_params, p_init_params,
      sizeof(img_dual_cam_init_params_t));

    p_dual_frameproc_lib = p_comp->p_lib;
    if (NULL == p_dual_frameproc_lib->ptr) {
      IDBG_ERROR("%s:%d] library not loaded", __func__, __LINE__);
      pthread_mutex_unlock(&p_inst->mutex);
      return IMG_ERR_INVALID_OPERATION;
    }

    IDBG_MED("%s:%d] state %d", __func__, __LINE__, p_comp->b.state);

    img_q_init(&p_comp->msgQ, "msgQ");

    if (p_comp->p_lib->img_algo_init) {
      status = p_comp->p_lib->img_algo_init(&p_comp->p_algocontext,
        &p_inst->init_params);
    }

    if (p_comp->p_lib->img_algo_set_frame_ops) {
      p_comp->p_lib->img_algo_set_frame_ops(p_comp->p_algocontext,
        &p_comp->b.frame_ops);
    }

    pthread_mutex_unlock(&p_inst->mutex);
  }
  break;
  default:;
  }

  status = p_comp->b.ops.set_parm(&p_comp->b, param, p_data);

  IDBG_MED("%s:%d] param 0x%x", __func__, __LINE__, param);
  return status;
}

/**
 * Function: dual_frameproc_comp_deinit
 *
 * Description: Un-initializes the dual_frameproc component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int dual_frameproc_comp_deinit(void *handle)
{
  dual_frameproc_comp_inst_t *p_inst = (dual_frameproc_comp_inst_t *)handle;
  dual_frameproc_comp_t *p_comp = NULL;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  if (!p_inst) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  p_comp = p_inst->p_comp_body;
  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error p_comp", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  pthread_mutex_lock(&p_inst->mutex);

  img_q_deinit(&p_inst->inputQ);
  img_q_deinit(&p_inst->outputQ);
  img_q_deinit(&p_inst->outBufQ);
  img_q_deinit(&p_inst->metaQ);
  pthread_mutex_unlock(&p_inst->mutex);
  pthread_mutex_destroy(&p_inst->buf_mutex);
  pthread_mutex_destroy(&p_inst->mutex);

  pthread_mutex_lock(&p_comp->mutex);
  img_list_remove(p_comp->inst_list, dual_frameproc_comp_find_inst, p_inst);
  if (p_comp->inst_cnt > 0) {
    p_comp->inst_cnt--;
  }
  pthread_mutex_unlock(&p_comp->mutex);

  /* Do comp deinit only when the last inst call deinit */
  if (p_comp->inst_cnt == 0) {
    if (p_comp->b.state == IMG_STATE_INIT ||
      p_comp->b.state == IMG_STATE_STARTED) {
      status = p_comp->b.ops.abort(&p_comp->b, NULL);
      if (IMG_ERROR(status)) {
        return status;
      }
    }
    if (p_comp->p_lib->img_algo_deinit) {
      status = p_comp->p_lib->img_algo_deinit(p_comp->p_algocontext);
      p_comp->p_algocontext = NULL;
    }

    if (IMG_ERROR(status)) {
      IDBG_MED("%s:%d] Error status %d", __func__, __LINE__, status);
    }

    if (p_comp->b.state == IMG_STATE_INIT ||
      p_comp->b.state == IMG_STATE_STARTED) {
      status = p_comp->b.ops.deinit(&p_comp->b);
      if (IMG_ERROR(status)) {
        return status;
      }
    }
    img_q_deinit(&p_comp->msgQ);
  }
  IDBG_MED("%s:%d] X %d", __func__, __LINE__, status);
  return status;
}

/**
 * Function: dual_frameproc_comp_check_create_bundle
 *
 * Description: This function is used to check and create the
 *            bundle if needed
 *
 * Input parameters:
 *   p_inst - The pointer to the component inst handle.
 *
 * Return values:
 *     Imaging error values
 *
 * Notes: none
 **/
int dual_frameproc_comp_check_create_bundle(dual_frameproc_comp_inst_t *p_inst)
{
  int8_t create_bundle = FALSE;
  int rc;
  dual_frameproc_comp_t *p_comp = NULL;
  img_component_t *p_base = NULL;
  int outputQcnt = 0;
  int metaQcount = 0;
  dual_frameproc_comp_inst_t *p_main_inst = NULL;
  dual_frameproc_comp_inst_t *p_aux_inst = NULL;
  img_frame_t *p_aux_frame = NULL;
  img_frame_t *p_main_frame = NULL;
  img_frame_t *p_output = NULL;;
  img_meta_t *p_meta = NULL;
  uint32_t cur_main_frame_id = 0;
  uint64_t time_interval = 0;
  img_msg_t msg;

  IDBG_HIGH("%s:%d] p_inst %p", __func__, __LINE__, p_inst);
  if (!p_inst) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  if (p_inst->master_flag) {
    p_main_inst = p_inst;
    p_aux_inst = p_main_inst->p_peer_inst;
  } else {
    p_aux_inst = p_inst;
    p_main_inst = p_inst->p_peer_inst;
  }
  if (!p_main_inst || !p_aux_inst) {
    IDBG_HIGH("%s:%d] Wait for bind", __func__, __LINE__);
    return IMG_ERR_NOT_FOUND;
  }
  if (p_main_inst->unbind_flag) {
    IDBG_MED("%s:%d] Received unbind", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  p_comp = p_inst->p_comp_body;
  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error p_comp NULL", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  p_base = &p_comp->b;
  outputQcnt = img_q_count(&p_main_inst->outBufQ);
  metaQcount = img_q_count(&p_main_inst->metaQ);
  p_aux_frame = p_aux_inst->p_cached_frame;
  p_main_frame = p_main_inst->p_cached_frame;

  if ((outputQcnt >= p_base->caps.num_output) &&
    (metaQcount >= p_base->caps.num_meta) &&
    p_aux_inst->fov_params_aux[0].input_width) {
    if (p_main_frame && p_aux_frame) {
      if (p_main_inst->af_fps == 0) {
        p_main_inst->af_fps = DEFAULT_AF_FPS;
      }
      time_interval = (1000000LL / p_main_inst->af_fps) / 2 + 1;
      IDBG_MED("%s:%d] time_interval %lld, main %lld, aux %lld",
        __func__, __LINE__, time_interval,
        p_main_inst->current_timestamp, p_aux_inst->current_timestamp);
      rc = dual_frameproc_comp_check_timestamp_interval(
        p_main_inst->current_timestamp,
        p_aux_inst->current_timestamp,
        time_interval);
      switch (rc) {
      case TIME_INTERVAL_WITHIN_RANGE:
        create_bundle = TRUE;
        IDBG_LOW("%s:%d] p_main %p, p_aux %p", __func__, __LINE__,
          p_main_frame, p_aux_frame);
        break;
      case TIMESTAMP_TOO_OLD:
        pthread_mutex_lock(&p_main_inst->mutex);
        if (p_main_inst->cached_flag) {
          /* main frame is too old, send buf done for main frame */
          dual_frameproc_send_buffer_done(p_main_inst,
            (void *)p_main_frame, QIMG_EVT_IMG_BUF_DONE);
          p_main_inst->p_cached_frame = NULL;
          p_main_inst->cached_flag = 0;
        }
        pthread_mutex_unlock(&p_main_inst->mutex);
        IDBG_MED("%s:%d] Bundle fail, main frame too old", __func__, __LINE__);
        break;
      case TIMESTAMP_TOO_NEW:
        pthread_mutex_lock(&p_aux_inst->mutex);
        if (p_aux_inst->cached_flag) {
          /* aux frame is too old, send buf done for main frame */
          dual_frameproc_send_buffer_done(p_aux_inst,
            (void *)p_aux_frame, QIMG_EVT_IMG_BUF_DONE);
          p_aux_inst->p_cached_frame = NULL;
          p_aux_inst->cached_flag = 0;
        }
        pthread_mutex_unlock(&p_aux_inst->mutex);
        IDBG_MED("%s:%d] Bundle fail, aux frame too old", __func__, __LINE__);
        break;
      default:
        break;
      }

      if (p_main_frame) {
        cur_main_frame_id = p_main_frame->frame_id;
      }
      if (p_main_inst->filled_frame_id == p_main_frame->frame_id ||
        p_aux_inst->filled_frame_id == p_aux_frame->frame_id) {
        create_bundle = FALSE;
      }
    }
    if (create_bundle) {
      /* output */
      while (img_q_count(&p_main_inst->outBufQ) > 0) {
        p_output = img_q_dequeue(&p_main_inst->outBufQ);
        if (!p_output) {
          IDBG_ERROR("%s:%d] Cannot dequeue out frame", __func__, __LINE__);
          goto error;
        }
        if (p_output->frame_id == cur_main_frame_id) {
          break;
        }
        //frame id not match, buf done for this output buf
        dual_frameproc_send_buffer_done(p_main_inst, (void *)p_output,
          QIMG_EVT_IMG_OUT_BUF_DONE);
        p_output = NULL;
      }
      /* meta */
      while (img_q_count(&p_main_inst->metaQ) > 0) {
        p_meta = img_q_dequeue(&p_main_inst->metaQ);
        if (!p_meta) {
          IDBG_ERROR("%s:%d] Cannot dequeue meta frame", __func__, __LINE__);
          goto error;
        }
        IDBG_MED("%s:%d] meta id %d main id %d",
          __func__, __LINE__, p_meta->frame_id, cur_main_frame_id);
        if (p_meta->frame_id == cur_main_frame_id) {
          p_main_inst->af_fps = p_meta->dcrf_runtime_params.af_fps;
          break;
        }
        //frame id not match, buf done for this meta buf
        dual_frameproc_send_buffer_done(p_main_inst, (void *)p_meta,
          QIMG_EVT_META_BUF_DONE);
        p_meta = NULL;
      }
      if (!p_meta) {
        create_bundle = FALSE;
        IDBG_HIGH("%s:%d] Bundle fail, p_output %p p_meta %p",
          __func__, __LINE__, p_output, p_meta);
        goto error;
      }
    }

  }
  IDBG_MED("%s:%d] create_bundle flag %d", __func__, __LINE__, create_bundle);

  if (create_bundle) {
    memset(&msg, 0x0, sizeof(img_msg_t));

    msg.p_sender = (void *)p_main_inst;
    msg.bundle.p_input[0] = p_main_frame;
    msg.bundle.p_input[1] = p_aux_frame;
    msg.bundle.p_output[0] = p_output;
    msg.bundle.p_meta[0] = p_meta;
    msg.type = IMG_MSG_BUNDLE;
    IDBG_MED("%s:%d] main %lld, aux %lld", __func__, __LINE__,
      p_main_frame->timestamp, p_aux_frame->timestamp);
    rc = dual_frameproc_prepare_buffer(&msg);

    IDBG_MED("%s:%d] Bundle created, pushed to Ping Pong buffer rc %d",
      __func__, __LINE__, rc);
  }

  return IMG_SUCCESS;

error:
  return IMG_ERR_GENERAL;
}

/**
 * Function: dual_frameproc_comp_queue_buffer
 *
 * Description: This function is used to handle buffers from the
 *             client
 *
 * Input parameters:
 *   @handle - The pointer to the component handle.
 *   @p_frame - The frame buffer which needs to be processed by
 *             the imaging library
 *   @type: image type (main image or thumbnail image)
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int dual_frameproc_comp_queue_buffer(void *handle, img_frame_t *p_frame,
  img_type_t type)
{
  dual_frameproc_comp_inst_t *p_inst = (dual_frameproc_comp_inst_t *)handle;
  dual_frameproc_comp_t *p_comp = NULL;
  int status = IMG_SUCCESS;
  img_queue_t *queue;
  uint64_t time_interval;

  if (!p_inst) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  p_comp = p_inst->p_comp_body;
  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error p_comp", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  IDBG_LOW("%s:%d] p_comp %p ", __func__, __LINE__, p_comp);

  img_component_t *p_base = &p_comp->b;
  if (p_base->flush_in_progress) {
    IDBG_MED("%s %d: Flush in progres", __func__, __LINE__);
    return IMG_ERR_BUSY;
  }
  if (p_inst->af_fps ==0) {
    p_inst->af_fps = DEFAULT_AF_FPS;
  }
  time_interval = (1000000LL / p_inst->af_fps) -
    DEFAULT_FRAME_INTERVAL_ADJUSTMENT;
  IDBG_MED("%s:%d] FPS %d  time_interval %lld", __func__, __LINE__,
    p_inst->af_fps, time_interval);

  pthread_mutex_lock(&p_comp->mutex);
  if ((p_base->state != IMG_STATE_INIT) &&
    (p_base->state != IMG_STATE_STARTED)) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__,
      p_base->state);
    pthread_mutex_unlock(&p_comp->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (type == IMG_IN) {
    if (p_inst->master_flag) { //get main buf
      IDBG_MED("%s:%d] main frame timestamp %lld ", __func__, __LINE__,
        p_frame->timestamp);
    } else { //get aux buf
      IDBG_MED("%s:%d] aux frame timestamp %lld ", __func__, __LINE__,
        p_frame->timestamp);
    }

    /* FPS control */
    if (time_interval + p_inst->processing_timestamp > p_frame->timestamp) {
      /* drop frame based on AF FPS */
      dual_frameproc_send_buffer_done(p_inst, (void *)p_frame,
        QIMG_EVT_IMG_BUF_DONE);
      IDBG_MED("%s:%d] Drop frame expect timestamp %lld ", __func__, __LINE__,
        time_interval + p_inst->processing_timestamp);
      p_inst->p_cached_frame = NULL;

      pthread_mutex_unlock(&p_comp->mutex);
      return IMG_SUCCESS;
    }
    pthread_mutex_lock(&p_inst->buf_mutex);
    if (p_inst->cached_flag) {
      /* frame cached but not bundled, before overwritten send buf done */
      IDBG_LOW("%s: %d: Sending buf done for frame %d", __func__, __LINE__,
        p_inst->p_cached_frame->frame_id);
      dual_frameproc_send_buffer_done(p_inst, (void *)p_inst->p_cached_frame,
        QIMG_EVT_IMG_BUF_DONE);
    }
    p_inst->p_cached_frame = p_frame;
    p_inst->current_timestamp = p_frame->timestamp;
    p_inst->cached_flag = 1;
    pthread_mutex_unlock(&p_inst->buf_mutex);
    dual_frameproc_comp_check_create_bundle(p_inst);
  } else if (type == IMG_OUT) {

    /*only handle out frame for master camera */
    if (p_inst->master_flag) {
      queue = &p_inst->outBufQ;

      status = img_q_enqueue(queue, p_frame);
      if (IMG_ERROR(status)) {
        IDBG_ERROR("%s:%d] Error enqueue", __func__, __LINE__);
        pthread_mutex_unlock(&p_comp->mutex);
        return status;
      }

      IDBG_MED("%s:%d] q_count %d", __func__, __LINE__, img_q_count(queue));
        dual_frameproc_comp_check_create_bundle(p_inst);
    }
  }
  pthread_mutex_unlock(&p_comp->mutex);
  IDBG_MED("%s:%d] p_comp %p p_inst %p", __func__, __LINE__, p_comp, p_inst);

  return IMG_SUCCESS;
}

/**
 * Function: dual_frameproc_comp_queue_metabuffer
 *
 * Description: This function is used to handle input meta
 *            buffers from the client
 *
 * Input parameters:
 *   @handle - The pointer to the component handle.
 *   @p_metabuffer - The meta buffer which needs to be
 *             processed by the imaging library
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int dual_frameproc_comp_queue_metabuffer(void *handle, img_meta_t *p_metabuffer)
{
  dual_frameproc_comp_inst_t *p_inst = (dual_frameproc_comp_inst_t *)handle;
  dual_frameproc_comp_t *p_comp = NULL;
  int status = IMG_SUCCESS;
  uint32_t i;

  if (!p_inst) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  p_comp = p_inst->p_comp_body;
  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, status);
    return IMG_ERR_INVALID_INPUT;
  }
  IDBG_MED("%s:%d] p_inst %p p_meta %p", __func__, __LINE__,
    p_inst, p_metabuffer);

  p_inst->af_fps = p_metabuffer->dcrf_runtime_params.af_fps;

  if (p_inst->master_flag) {
    img_component_t *p_base = &p_comp->b;
    img_queue_t *queue = &p_inst->metaQ;

    pthread_mutex_lock(&p_comp->mutex);
    if ((p_base->state != IMG_STATE_INIT) &&
      (p_base->state != IMG_STATE_STARTED)) {
      IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__,
        p_base->state);
      pthread_mutex_unlock(&p_comp->mutex);
      return IMG_ERR_INVALID_OPERATION;
    }

    while (img_q_count(&p_inst->metaQ) >= MAX_CACHED_META_FRAME) {
      img_meta_t *p_meta = img_q_dequeue(&p_inst->metaQ);
      if (!p_meta) {
        IDBG_ERROR("%s:%d] Cannot dequeue meta frame", __func__, __LINE__);
        break;
      }
      dual_frameproc_send_buffer_done(p_inst, (void *)p_meta,
        QIMG_EVT_META_BUF_DONE);
    }

    status = img_q_enqueue(queue, p_metabuffer);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("%s:%d] Error enqueue", __func__, __LINE__);
      pthread_mutex_unlock(&p_comp->mutex);
      return status;
    }

    IDBG_MED("%s:%d] q_count %d", __func__, __LINE__, img_q_count(queue));
    dual_frameproc_comp_check_create_bundle(p_inst);

    pthread_mutex_unlock(&p_comp->mutex);
  } else {
    IDBG_MED("%s:%d] Save FOV for aux", __func__, __LINE__);
    pthread_mutex_lock(&p_comp->mutex);
    for(i = 0; i < IMG_MAX_FOV; i++) {
      memcpy(&p_inst->fov_params_aux[i],
        &p_metabuffer->dcrf_runtime_params.fov_params_main[i],
        sizeof(img_fov_t));
    }

    pthread_mutex_unlock(&p_comp->mutex);
    dual_frameproc_send_buffer_done(p_inst, (void *)p_metabuffer,
      QIMG_EVT_META_BUF_DONE);
  }

  return IMG_SUCCESS;
}

/**
 * Function: dual_frameproc_comp_flush
 *
 * Description: Aborts the execution of the base imaging component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   type - image type
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: This function will flush all the buffers from the ping
 * pong buffers, the o/p and meta queues. Will not change the
 * component state ot abort the current job
 **/
int dual_frameproc_comp_flush(void *handle, img_type_t type)
{
  dual_frameproc_comp_inst_t *p_inst = (dual_frameproc_comp_inst_t *)handle;
  dual_frameproc_comp_t *p_comp = NULL;
  p_comp = p_inst->p_comp_body;
  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  img_component_t *p_base = &p_comp->b;
  IDBG_MED("%s:%d] state %d", __func__, __LINE__, p_base->state);
  pthread_mutex_lock(&p_base->mutex);
  p_base->flush_in_progress = TRUE;
  pthread_mutex_unlock(&p_base->mutex);
  IDBG_MED("%s:%d] calling flush buffers", __func__, __LINE__);
  dual_frameproc_flush_buffers(p_inst, NULL);
  pthread_mutex_lock(&p_base->mutex);
  p_base->flush_in_progress = FALSE;
  pthread_mutex_unlock(&p_base->mutex);
  return IMG_SUCCESS;

}

/**
 * Function: dual_frameproc_comp_abort
 *
 * Description: Aborts the execution of the base imaging component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type will be defined in the corresponding
 *            include file.
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int dual_frameproc_comp_abort(void *handle, void *p_data)
{
  IMG_UNUSED(p_data);

  dual_frameproc_comp_inst_t *p_inst = (dual_frameproc_comp_inst_t *)handle;
  dual_frameproc_comp_t *p_comp = NULL;

  if (!p_inst) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  p_comp = p_inst->p_comp_body;
  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  img_component_t *p_base = &p_comp->b;
  IDBG_HIGH("%s:%d] state %d", __func__, __LINE__, p_base->state);

  dual_frameproc_comp_flush(handle, IMG_IN);

  /* only stop the comp thread when last inst aborts */
  if (p_comp->inst_cnt == 1) {
    pthread_mutex_lock(&p_base->mutex);
    /* Thread is not present in init state */
    if (IMG_STATE_INIT == p_base->state) {
      pthread_mutex_unlock(&p_base->mutex);
      return IMG_SUCCESS;
    }
    p_base->state = IMG_STATE_STOP_REQUESTED;
    pthread_mutex_unlock(&p_base->mutex);

    img_q_signal(&p_comp->msgQ);

    if (!pthread_equal(pthread_self(), p_base->threadid)) {
      IDBG_MED("%s:%d] thread id %d", __func__, __LINE__,
        (uint32_t)p_base->threadid);
      pthread_join(p_base->threadid, NULL);
    }
    pthread_mutex_lock(&p_base->mutex);
    p_base->state = IMG_STATE_INIT;
    pthread_mutex_unlock(&p_base->mutex);

  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: dual_frameproc_comp_start
 *
 * Description: Start the execution of dual_frameproc
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type will be defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int dual_frameproc_comp_start(void *handle, void *p_data)
{
 dual_frameproc_comp_inst_t *p_inst = (dual_frameproc_comp_inst_t *)handle;
 dual_frameproc_comp_t *p_comp = NULL;

  IDBG_HIGH("%s:%d] p_ops->handle %p", __func__, __LINE__, handle);
  if (!p_inst) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  p_comp = p_inst->p_comp_body;
  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  IDBG_MED("%s:%d] p_comp %p ", __func__, __LINE__, p_comp);

  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;

  pthread_mutex_lock(&p_comp->mutex);

  if ((p_base->caps.num_input <= 0) ||
    (p_base->caps.num_output < 0)) {
    IDBG_ERROR("%s:%d] Error caps not set", __func__, __LINE__);
    pthread_mutex_unlock(&p_comp->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  /*set the main thread*/
  if (p_base->state == IMG_STATE_INIT) {
    p_comp->b.thread_loop = NULL;
    p_comp->b.p_core = p_comp;

    status = p_comp->b.ops.start(&p_comp->b, p_data);
  }

  /* flush the queues */
  img_q_flush(&p_inst->inputQ);
  img_q_flush(&p_inst->outBufQ);
  img_q_flush(&p_inst->metaQ);
  img_q_flush(&p_comp->msgQ);

  pthread_mutex_unlock(&p_comp->mutex);

  return status;
}

/**
 * Function: dual_frameproc_comp_set_callback
 *
 * Description: Sets the callback for the dual frameproc component
 *
 * Input parameters:
 *   handle - The pointer to the component inst handle.
 *   notify - The function pointer for the event callback
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int dual_frameproc_comp_set_callback(void *handle, notify_cb notify)
{
  dual_frameproc_comp_inst_t *p_inst = (dual_frameproc_comp_inst_t *)handle;
  dual_frameproc_comp_t *p_comp = NULL;

  if (!p_inst || !notify) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  p_comp = p_inst->p_comp_body;
  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error p_comp NULL", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  pthread_mutex_lock(&p_comp->b.mutex);
  if ((p_comp->b.state != IMG_STATE_INIT) &&
    (p_comp->b.state != IMG_STATE_STARTED)) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    pthread_mutex_unlock(&p_comp->b.mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  p_inst->p_cb = notify;
  IDBG_MED("%s:%d] p_comp %p  cb %p", __func__, __LINE__,
    p_inst, p_inst->p_cb);
  pthread_mutex_unlock(&p_comp->b.mutex);
  return IMG_SUCCESS;
}

/**
 * Function: dual_frameproc_comp_create
 *
 * Description: This function is used to create dual_frameproc component
 *
 * Input parameters:
 *   @handle: library handle
 *   @p_ops - The pointer to img_component ops. This
 *            object contains the handle and the function
 *            pointers for communicating with the imaging
 *            component.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *     IMG_ERR_NO_MEMORY
 *
 * Notes: none
 **/
int dual_frameproc_comp_create(void* handle, img_component_ops_t *p_ops)
{
  dual_frameproc_comp_inst_t *p_inst = NULL;
  dual_frameproc_comp_t *p_comp = NULL;
  int status;

  IDBG_MED("%s:%d] E", __func__, __LINE__);
  if (!handle) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (NULL == p_ops) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  p_comp = (dual_frameproc_comp_t *)handle;

  /* create the base componet just once */
  if (p_comp->inst_cnt == 0) {
    pthread_mutex_lock(&p_comp->mutex);
    status = img_comp_create(&p_comp->b);
    if (IMG_ERROR(status)) {
      pthread_mutex_unlock(&p_comp->mutex);
      IDBG_ERROR("%s:%d] Error create base comp", __func__, __LINE__);
      return IMG_ERR_GENERAL;
    }
    pthread_mutex_unlock(&p_comp->mutex);
  }

  if (p_comp->inst_cnt >= MAX_COMP_INST) {
    IDBG_ERROR("%s:%d] Error - Dual frameproc reach MAX inst cnt",
      __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  p_inst =
    (dual_frameproc_comp_inst_t *)calloc(1, sizeof(dual_frameproc_comp_inst_t));
  if (NULL == p_inst) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  pthread_mutex_lock(&p_comp->mutex);
  status = img_list_append(p_comp->inst_list, p_inst);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] Append dual frameproc inst failed", __func__, __LINE__);
    free(p_inst);
    pthread_mutex_unlock(&p_comp->mutex);
    return IMG_ERR_NO_MEMORY;
  }
  p_comp->inst_cnt++;
  pthread_mutex_unlock(&p_comp->mutex);

  /* copy the ops table from the base component */
  *p_ops = p_comp->b.ops;
  p_ops->init            = dual_frameproc_comp_init;
  p_ops->deinit          = dual_frameproc_comp_deinit;
  p_ops->set_parm        = dual_frameproc_comp_set_param;
  p_ops->get_parm        = dual_frameproc_comp_get_param;
  p_ops->start           = dual_frameproc_comp_start;
  p_ops->queue_buffer    = dual_frameproc_comp_queue_buffer;
  p_ops->abort           = dual_frameproc_comp_abort;
  p_ops->set_callback    = dual_frameproc_comp_set_callback;
  p_ops->queue_metabuffer = dual_frameproc_comp_queue_metabuffer;
  p_ops->flush           = dual_frameproc_comp_flush;

  p_inst->p_comp_body = p_comp;
  p_inst->af_fps = DEFAULT_AF_FPS;
  p_inst->filled_frame_id = 0;
  p_inst->processing_timestamp = 0;
  p_inst->master_flag = 1;
  pthread_mutex_init(&p_inst->mutex, NULL);
  pthread_mutex_init(&p_inst->buf_mutex, NULL);
  p_ops->handle = (void *)p_inst;

  IDBG_MED("%s:%d] p_ops->handle %p", __func__, __LINE__, p_ops->handle);
  IDBG_MED("%s:%d] inst_cnt %d", __func__, __LINE__, p_comp->inst_cnt);
  IDBG_MED("%s:%d] p_comp %p lib %p", __func__, __LINE__, p_comp, p_comp->p_lib);
  IDBG_MED("%s:%d] p_comp %p ", __func__, __LINE__, p_inst->p_comp_body);
  return IMG_SUCCESS;
}

/**
 * Function: dual_frameproc_comp_load
 *
 * Description: This function is used to load dual_frameproc library
 *
 * Input parameters:
 *   @name: library name
 *   @handle: library handle
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_NOT_FOUND
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int dual_frameproc_comp_load(const char* name, void** handle)
{
  dual_frameproc_lib_info_t *p_dual_frameproc_lib;
  dual_frameproc_comp_t *p_comp = NULL;

  if (!name || !handle) {
    IDBG_ERROR("%s:%d] invalid input %p %p",
      __func__, __LINE__, name, handle);
    return IMG_ERR_INVALID_INPUT;
  }

  p_comp = (dual_frameproc_comp_t *)calloc(1, sizeof(dual_frameproc_comp_t));
  if (NULL == p_comp) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  p_comp->inst_list = img_list_create();
  if (NULL == p_comp->inst_list) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    free(p_comp);
    return IMG_ERR_NO_MEMORY;
  }

  p_dual_frameproc_lib = calloc(1, sizeof(dual_frameproc_lib_info_t));
  if (!p_dual_frameproc_lib) {
    IDBG_ERROR("%s:%d] cannot alloc p_dual_frameproc_lib %s",
      __func__, __LINE__, name);
    img_list_free(p_comp->inst_list);
    free(p_comp);
    return IMG_ERR_NO_MEMORY;
  }

  p_dual_frameproc_lib->ptr = dlopen(name, RTLD_NOW);
  if (!p_dual_frameproc_lib->ptr) {
    IDBG_ERROR("%s:%d] Error opening dual_frameproc library %s error %s\n",
      __func__, __LINE__, name, dlerror());
    free(p_dual_frameproc_lib);
    img_list_free(p_comp->inst_list);
    free(p_comp);
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(p_dual_frameproc_lib->img_algo_init) =
    dlsym(p_dual_frameproc_lib->ptr, "img_algo_init");
  if (!p_dual_frameproc_lib->img_algo_init) {
    IDBG_ERROR("%s:%d] Error linking camera img_algo_init",
      __func__, __LINE__);
    dlclose(p_dual_frameproc_lib->ptr);
    p_dual_frameproc_lib->ptr = NULL;
    free(p_dual_frameproc_lib);
    img_list_free(p_comp->inst_list);
    free(p_comp);
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(p_dual_frameproc_lib->img_algo_deinit) =
    dlsym(p_dual_frameproc_lib->ptr, "img_algo_deinit");
  if (!p_dual_frameproc_lib->img_algo_deinit) {
    IDBG_ERROR("%s:%d] Error linking img_algo_deinit",
    __func__, __LINE__);
    dlclose(p_dual_frameproc_lib->ptr);
    p_dual_frameproc_lib->ptr = NULL;
    free(p_dual_frameproc_lib);
    img_list_free(p_comp->inst_list);
    free(p_comp);
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(p_dual_frameproc_lib->img_algo_process) =
    dlsym(p_dual_frameproc_lib->ptr, "img_algo_process");
  if (!p_dual_frameproc_lib->img_algo_process) {
    IDBG_ERROR("%s:%d] Error linking img_algo_process",
    __func__, __LINE__);
    dlclose(p_dual_frameproc_lib->ptr);
    p_dual_frameproc_lib->ptr = NULL;
    free(p_dual_frameproc_lib);
    img_list_free(p_comp->inst_list);
    free(p_comp);
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(p_dual_frameproc_lib->img_algo_frame_ind) =
    dlsym(p_dual_frameproc_lib->ptr, "img_algo_frame_ind");
  if (!p_dual_frameproc_lib->img_algo_frame_ind) {
    IDBG_HIGH("%s:%d] Warning linking img_algo_frame_ind",
    __func__, __LINE__);
  }

  *(void **)&(p_dual_frameproc_lib->img_algo_meta_ind) =
    dlsym(p_dual_frameproc_lib->ptr, "img_algo_meta_ind");
  if (!p_dual_frameproc_lib->img_algo_meta_ind) {
    IDBG_HIGH("%s:%d] Warning linking img_algo_meta_ind",
    __func__, __LINE__);
  }

  *(void **)&(p_dual_frameproc_lib->img_algo_set_frame_ops) =
    dlsym(p_dual_frameproc_lib->ptr, "img_algo_set_frame_ops");
  if (!p_dual_frameproc_lib->img_algo_set_frame_ops) {
    IDBG_HIGH("%s:%d] Warning linking img_algo_set_frame_ops",
    __func__, __LINE__);
  }

  pthread_mutex_init(&p_comp->mutex, NULL);
  p_comp->p_lib = p_dual_frameproc_lib;
  *handle = p_comp;

  IDBG_MED("%s:%d] %s loaded successfully", __func__, __LINE__, name);
  return IMG_SUCCESS;
}

/**
 * Function: dual_frameproc_comp_unload
 *
 * Description: This function is used to unload dual_frameproc library
 *
 * Input parameters:
 *   @handle: library handle
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void dual_frameproc_comp_unload(void* handle)
{
  int rc = 0;
  dual_frameproc_comp_t *p_comp = NULL;
  dual_frameproc_lib_info_t *p_dual_frameproc_lib = NULL;

  IDBG_HIGH("%s:%d] ", __func__, __LINE__);

  if (!handle) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return;
  }

  p_comp = (dual_frameproc_comp_t *)handle;
  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error NULL p_comp", __func__, __LINE__);
    return;
  }

  p_dual_frameproc_lib = p_comp->p_lib;
  if (!p_dual_frameproc_lib) {
    IDBG_ERROR("%s:%d] Error unloading library", __func__, __LINE__);
    return;
  }
  if (p_dual_frameproc_lib->ptr) {
    rc = dlclose(p_dual_frameproc_lib->ptr);
    if (rc < 0) {
      IDBG_HIGH("%s:%d] error %s", __func__, __LINE__, dlerror());
    }
    p_dual_frameproc_lib->ptr = NULL;
  }
  free(p_dual_frameproc_lib);
  pthread_mutex_destroy(&p_comp->mutex);
  img_list_free(p_comp->inst_list);
  free(p_comp);
}

/**
 * Function: dual_frameproc_comp_bind
 *
 * Description: This function is used to bind 2 dual camera instances
 *
 * Input parameters:
 *   @p_handle_main: Handle of the main instance
 *   @p_handle_aux: Handle of the aux instance
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_NOT_FOUND
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int dual_frameproc_comp_bind(void *p_handle_main, void *p_handle_aux)
{
  img_core_type_t thread_affinity[1] = {
    IMG_CORE_ARM,
  };
  int rc = IMG_SUCCESS;
  dual_frameproc_comp_t *p_comp = NULL;
  img_component_t *p_base = NULL;
  dual_frameproc_lib_info_t *p_dual_frameproc_lib = NULL;
  dual_frameproc_comp_inst_t *p_main_inst =
    (dual_frameproc_comp_inst_t *)p_handle_main;
  dual_frameproc_comp_inst_t *p_aux_inst =
    (dual_frameproc_comp_inst_t *)p_handle_aux;

  IDBG_LOW("%s:%d] E", __func__, __LINE__);
  if (!p_main_inst || !p_aux_inst) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  IDBG_MED("%s:%d] main %p aux %p", __func__, __LINE__,
    p_main_inst, p_aux_inst);

  p_comp = p_main_inst->p_comp_body;
  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_NOT_FOUND;
  }
  p_base = &p_comp->b;

  pthread_mutex_lock(&p_comp->mutex);
  p_main_inst->p_peer_inst = p_aux_inst;
  p_aux_inst->p_peer_inst = p_main_inst;

  p_main_inst->master_flag = 1;
  p_aux_inst->master_flag = 0;
  p_main_inst->unbind_flag = 0;

  p_dual_frameproc_lib = p_comp->p_lib;

  if (NULL == p_dual_frameproc_lib->ptr) {
    IDBG_ERROR("%s:%d] library not loaded", __func__, __LINE__);
    pthread_mutex_unlock(&p_comp->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (p_base->thread_ops.client_id == 0) {
    rc = img_thread_mgr_reserve_threads(1, thread_affinity);
    if (rc == 0) {
      IDBG_ERROR("%s:%d] Error reserve thread ", __func__, __LINE__);
      pthread_mutex_unlock(&p_comp->mutex);
      return IMG_ERR_GENERAL;
    }

    p_base->thread_ops.client_id = rc;
    IDBG_MED("%s:%d] Reserved thread with client id %d ", __func__, __LINE__,
      p_base->thread_ops.client_id);

    uint32_t *dep_job_ids = calloc(MAX_DEPENDANT_JOBS, sizeof(uint32_t));
    if (!dep_job_ids) {
      IDBG_ERROR("%s:%d] Failed alloc dep ids", __func__, __LINE__);
      pthread_mutex_unlock(&p_comp->mutex);
      return IMG_ERR_NO_MEMORY;
    }

    p_main_inst->thread_job.client_id = p_base->thread_ops.client_id;
    p_main_inst->thread_job.core_affinity = IMG_CORE_ARM;
    p_main_inst->thread_job.execute = dual_frameproc_comp_process_job;
    p_main_inst->thread_job.dep_job_count = 0;
    p_main_inst->thread_job.args = (void *)p_main_inst;
    p_main_inst->thread_job.dep_job_ids = dep_job_ids;
    p_main_inst->thread_job.delete_on_completion = TRUE;
  }

  pthread_mutex_unlock(&p_comp->mutex);

  return IMG_SUCCESS;
}

/**
 * Function: dual_frameproc_comp_unbind
 *
 * Description: This function is used to unbind 2 dual camera instances
 *
 * Input parameters:
 *   @p_handle_main: Handle of the main instance
 *   @p_handle_aux: Handle of the aux instance
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int dual_frameproc_comp_unbind(void *p_handle_main, void *p_handle_aux)
{
  dual_frameproc_comp_inst_t *p_main_inst =
    (dual_frameproc_comp_inst_t *)p_handle_main;
  dual_frameproc_comp_inst_t *p_aux_inst =
    (dual_frameproc_comp_inst_t *)p_handle_aux;
  dual_frameproc_comp_t *p_comp;

  if (!p_main_inst || !p_aux_inst) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  p_comp = p_main_inst->p_comp_body;
  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  pthread_mutex_lock(&p_main_inst->buf_mutex);
  //Unreserve threads for the client
  if (p_comp->b.thread_ops.client_id) {
    IDBG_HIGH("%s:%d] Unreserve thread with client id %d ", __func__, __LINE__,
      p_comp->b.thread_ops.client_id);
    img_thread_mgr_unreserve_threads(p_comp->b.thread_ops.client_id);
    p_comp->b.thread_ops.client_id = 0;
  }
  dual_frameproc_flush_jobs(p_main_inst, p_aux_inst);
  p_main_inst->unbind_flag = 1;
  pthread_mutex_unlock(&p_main_inst->buf_mutex);

  return IMG_SUCCESS;
}
