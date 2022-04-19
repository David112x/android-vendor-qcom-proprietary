/**********************************************************************
* Copyright (c) 2013-2017,2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/


#include "frameproc_comp.h"
#include "img_dsp_dl_mgr.h"
#include "img_buffer.h"

#ifdef __ANDROID__
#ifdef USE_PERF_API
#include <dlfcn.h>
#include "mp-ctl.h"
#endif
#endif

/**
 *  Static functions
 **/
static int frameproc_comp_unload_lib(void * handle, void *p_userdata);
static int frameproc_comp_reload_lib(void * handle, const char *name,
  void *p_userdata);

/**
 * CONSTANTS and MACROS
 **/

 /**
 * Macro: FRAME_PROC_Q_MSG
 *
 * Description: Queues a message to the frameproc message queue
 *
 * Input parameters:
 *   @p_comp - The pointer to the component handle.
 *   @msg_type: message type
 *   @val_type: data type
 *   @data: data payload
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_NO_MEMORY
 *
 * Notes: none
 **/
#define FRAME_PROC_Q_MSG(p_comp, msg_type, val_type, data) ({ \
  int status = IMG_SUCCESS; \
  img_msg_t *p_msg; \
  p_msg = malloc(sizeof(img_msg_t)); \
  if (!p_msg) { \
    IDBG_ERROR("%s:%d] cannot alloc message", __func__, __LINE__); \
    status = IMG_ERR_NO_MEMORY; \
  } else { \
    memset(p_msg, 0x0, sizeof(img_msg_t)); \
    p_msg->type = msg_type; \
    p_msg->val_type = data; \
    status = img_q_enqueue(&p_comp->msgQ, p_msg); \
    if (IMG_ERROR(status)) { \
      IDBG_ERROR("%s:%d] Cannot enqueue bundle", __func__, __LINE__); \
      free(p_msg); \
    } else { \
      img_q_signal(&p_comp->msgQ); \
    } \
  } \
  status; \
})

/**
 * Function: frameproc_comp_init
 *
 * Description: Initializes the Qualcomm frameproc component
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
int frameproc_comp_init(void *handle, void* p_userdata, void *p_data)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;
  img_init_params_t *p_params = (img_init_params_t *)p_data;
  IDBG_MED("%s:%d] ", __func__, __LINE__);

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  status = p_comp->b.ops.init(&p_comp->b, p_userdata, p_data);
  if (status < 0) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, status);
    return status;
  }

  if (p_comp->p_lib->img_algo_init)
    status = p_comp->p_lib->img_algo_init(&p_comp->p_algocontext, p_params);
  return status;
}

/**
 * Function: frameproc_can_wait
 *
 * Description: Queue function to check if abort is issued
 *
 * Input parameters:
 *   p_userdata - The pointer to frameproc component
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
int frameproc_can_wait(void *p_userdata)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)p_userdata;
  img_component_t *p_base = &p_comp->b;
  return !((p_base->state == IMG_STATE_STOP_REQUESTED)
    || (p_base->state == IMG_STATE_STOPPED));
}

/**
 * Function: frameproc_send_meta_buffers
 *
 * Description: Function to send the meta buffers to client
 *
 * Input parameters:
 *   p_comp - The pointer to the component object
 *   p_bundle - frame bundle
 *   status - status of caller
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void frameproc_send_meta_buffers(frameproc_comp_t *p_comp,
  img_frame_bundle_t *p_bundle, int status)
{
  img_component_t *p_base = &p_comp->b;
  int i = 0;

  for (i = 0; i < p_base->caps.num_meta; i++) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_META_BUF_DONE,
      p_meta, p_bundle->p_meta[i]);
  }

  if (IMG_ERROR(status)) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_ERROR, status, status);
  }
}

/**
 * Function: frameproc_send_io_buffers
 *
 * Description: Function to send the input, output buffers to client
 *
 * Input parameters:
 *   p_comp - The pointer to the component object
 *   p_bundle - frame bundle
 *   status - status of caller
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void frameproc_send_io_buffers(frameproc_comp_t *p_comp,
  img_frame_bundle_t *p_bundle, int status)
{
  img_component_t *p_base = &p_comp->b;
  int i = 0;

  for (i = 0; i < p_base->caps.num_input; i++) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_BUF_DONE,
      p_frame, p_bundle->p_input[i]);
  }

  for (i = 0; i < p_base->caps.num_output; i++) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_OUT_BUF_DONE,
      p_frame, p_bundle->p_output[i]);
  }

  if (IMG_ERROR(status)) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_ERROR, status, status);
  }
}

/**
 * Function: frameproc_flush_buffers
 *
 * Description: Function to send the buffers to client
 *
 * Input parameters:
 *   p_comp - The pointer to the component object
 *   status - status of caller
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void frameproc_flush_buffers(frameproc_comp_t *p_comp, int status)
{
  img_component_t *p_base = &p_comp->b;
  img_frame_t *p_frame;
  img_meta_t *p_meta;
  img_msg_t *p_msg;

  IDBG_LOW("E");

  /* flush rest of the buffers */
  while ((p_msg = img_q_dequeue(&p_comp->msgQ)) != NULL) {
    switch (p_msg->type) {
    case IMG_MSG_BUNDLE:
      frameproc_send_meta_buffers(p_comp, &p_msg->bundle, status);
      frameproc_send_io_buffers(p_comp, &p_msg->bundle, status);
      break;
    default:;
    }
    free(p_msg);
    p_msg = NULL;
  }

  while ((p_frame = img_q_dequeue(&p_base->inputQ)) != NULL) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_BUF_DONE,
      p_frame, p_frame);
  }

  while ((p_frame = img_q_dequeue(&p_base->outBufQ)) != NULL) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_OUT_BUF_DONE,
      p_frame, p_frame);
  }

  while ((p_meta = img_q_dequeue(&p_base->metaQ)) != NULL) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_META_BUF_DONE,
      p_meta, p_meta);
  }
  if (IMG_ERROR(status)) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_ERROR, status, status);
  }

  IDBG_LOW("X");
}

/**
 * Function: frameproc_thread_loop
 *
 * Description: Main algorithm thread loop
 *
 * Input parameters:
 *   data - The pointer to the component object
 *
 * Return values:
 *     NULL
 *
 * Notes: none
 **/
void *frameproc_thread_loop(void *handle)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_msg_t *p_msg;
  frameproc_cb_data_t *p_cback;

#ifdef __ANDROID__
#ifdef USE_PERF_API
  void* perf_lock_handle = NULL;
  void* perf_lib_handle = NULL;

  int32_t perf_lock_params[] = {
    MPCTLV3_MIN_FREQ_CLUSTER_BIG_CORE_0,
    IMG_CPU_FREQ_MAX
  };

  // Acquire performance lock to turbo when requested
  if (p_base->caps.is_offline_proc == true) {
    perf_lib_handle = img_perf_handle_create();
    IDBG_MED("Perflock Start");

    if (perf_lib_handle) {
      perf_lock_handle = img_perf_lock_start(perf_lib_handle, perf_lock_params,
        IMGLIB_ARRAY_SIZE(perf_lock_params), 3000);
    }
  }

#endif
#endif

  /* TBD: unify set_frame_ops and start */
  if (p_comp->p_lib->img_algo_start) {
    p_comp->p_lib->img_algo_start(p_comp->p_algocontext);
  }
  if (p_comp->p_lib->img_algo_set_frame_ops) {
    p_comp->p_lib->img_algo_set_frame_ops(p_comp->p_algocontext,
      &p_base->frame_ops);
  }
  IDBG_MED("%s:%d] start sent", __func__, __LINE__);

  while (TRUE) {
    p_msg = img_q_wait(&p_comp->msgQ, frameproc_can_wait, p_comp);

    if (!frameproc_can_wait(p_comp))
      break;
    else if (!p_msg)
      continue;

    IMG_SET_PROCESSING_START(p_base);

    if (IMG_CHK_FLUSH_LOCKED(p_base, &p_base->mutex)) {
      frameproc_flush_buffers(p_comp, status);
      IMG_SET_PROCESSING_COMPLETE(p_base);
      continue;
    }

    switch (p_msg->type) {
    case IMG_MSG_BUNDLE:
      if (p_comp->is_async) {
        p_cback = (frameproc_cb_data_t *)malloc(sizeof(*p_cback));
        if (!p_cback) {
          IDBG_ERROR("failed");
          frameproc_send_meta_buffers(p_comp, &p_msg->bundle, IMG_ERR_NO_MEMORY);
          frameproc_send_io_buffers(p_comp, &p_msg->bundle, IMG_ERR_NO_MEMORY);
          free(p_msg);
          p_msg = NULL;
          break;
        }
        p_cback->p_comp = p_comp;
        p_cback->p_msg = p_msg;
        p_msg->bundle.p_meta[0]->userdata = p_cback;
        p_cback->p_comp->process_count++;
      }
      status = p_comp->p_lib->img_algo_process(p_comp->p_algocontext,
        p_msg->bundle.p_input,
        p_base->caps.num_input,
        (!p_base->caps.inplace_algo) ? p_msg->bundle.p_output :
          p_msg->bundle.p_input,
        (!p_base->caps.inplace_algo) ? p_base->caps.num_output :
          p_base->caps.num_input,
        p_msg->bundle.p_meta,
        p_base->caps.num_meta);
      if (IMG_ERROR(status)) {
        IDBG_ERROR("IMGLIB_BASE algo error %d", status);
        if (IMG_ERR_CONNECTION_FAILED == status) {
          status = img_dsp_dl_mgr_set_reload_needed(TRUE);
        }
      }
      if(!p_comp->is_async) {
        frameproc_send_meta_buffers(p_comp, &p_msg->bundle, status);
        if (p_comp->p_lib->img_algo_post_process) {
          p_comp->p_lib->img_algo_post_process(p_comp->p_algocontext);
        }
        frameproc_send_io_buffers(p_comp, &p_msg->bundle, status);
      }
      break;
    case IMG_MSG_FRAME:
      if (p_comp->p_lib->img_algo_frame_ind) {
        p_comp->p_lib->img_algo_frame_ind(p_comp->p_algocontext,
          p_msg->p_frame);
      }
      break;
    case IMG_MSG_META:
      if (p_comp->p_lib->img_algo_meta_ind) {
        p_comp->p_lib->img_algo_meta_ind(p_comp->p_algocontext,
          p_msg->p_meta);
      }
      break;
    default:;
    }
    if (p_msg) {
      if (!p_comp->is_async) {
        //if not async free p_msg of IMG_MSG_BUNDLE, IMG_MSG_FRAME, IMG_MSG_META
        free(p_msg);
        p_msg = NULL;
      } else {
        //if async free for IMG_MSG_META, IMG_MSG_FRAME
        //IMG_MSG_BUNDLE will be free in algo_callback
        if ((p_msg->type == IMG_MSG_META)|| (p_msg->type == IMG_MSG_FRAME)) {
          free(p_msg);
          p_msg = NULL;
        }
      }
    }

    if (IMG_CHK_FLUSH_LOCKED(p_base, &p_base->mutex)) {
      frameproc_flush_buffers(p_comp, status);
    }
    IMG_SET_PROCESSING_COMPLETE(p_base);
  }

  if (p_comp->p_lib->img_algo_stop) {
    p_comp->p_lib->img_algo_stop(p_comp->p_algocontext);
  }

#ifdef __ANDROID__
#ifdef USE_PERF_API
  // Unlock only if valid lock is acquired
  if (perf_lib_handle) {
    /* release perflock */
    if (perf_lock_handle) {
      img_perf_lock_end(perf_lib_handle, perf_lock_handle);
    }
    img_perf_handle_destroy(perf_lib_handle);
  }
  IDBG_MED("Perf lock end");
#endif
#endif

  frameproc_flush_buffers(p_comp, status);
  return NULL;
}

/**
 * Function: frameproc_comp_get_param
 *
 * Description: Gets frameproc parameters
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
int frameproc_comp_get_param(void *handle, img_param_type param, void *p_data)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, status);
    return IMG_ERR_INVALID_INPUT;
  }
  switch(param) {
    case QIMG_PARAM_LIB_CONFIG:
      if (p_comp && p_comp->p_lib->img_algo_get_lib_config) {
        if (p_data) {
          status = p_comp->p_lib->img_algo_get_lib_config(
            p_comp->p_algocontext, p_data);
        } else {
          status = IMG_ERR_INVALID_INPUT;
          IDBG_ERROR("Invalid input");
        }
      } else {
        status = IMG_ERR_INVALID_INPUT;
        IDBG_ERROR("Invalid input");
      }
      break;
    default:
      status = p_comp->b.ops.get_parm(&p_comp->b, param, p_data);
      break;
    }

  return status;
}

/**
 * Function: frameproc_comp_algo_callback
 *
 * Description: Handle algo callback
 *
 * Input parameters:
 *   user_data - The pointer to data pass to process function
 *
 * Return values:
 *     BAYERPROC_SUCCESS
 *     BAYERPROC_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int frameproc_comp_algo_callback(void *user_data)
{
  frameproc_cb_data_t *p_cback = (frameproc_cb_data_t *)user_data;
  if (!p_cback || !p_cback->p_comp || !p_cback->p_msg) {
    return IMG_ERR_INVALID_INPUT;
  }
  pthread_mutex_lock(&p_cback->p_comp->mutex);

  /* Sending back buffers with IMG_SUCCESS status as algo process succeded */
  frameproc_send_meta_buffers(p_cback->p_comp, &p_cback->p_msg->bundle, IMG_SUCCESS);
  frameproc_send_io_buffers(p_cback->p_comp, &p_cback->p_msg->bundle, IMG_SUCCESS);
  p_cback->p_comp->process_count--;

  pthread_cond_signal(&p_cback->p_comp->cond);
  pthread_mutex_unlock(&p_cback->p_comp->mutex);
  free(p_cback->p_msg);
  p_cback->p_msg = NULL;
  free(p_cback);
  p_cback = NULL;

  return IMG_SUCCESS;
}

/**
 * Function: frameproc_comp_set_param
 *
 * Description: Set frameproc parameters
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in frameproc.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int frameproc_comp_set_param(void *handle, img_param_type param, void *p_data)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  if (!p_comp) {
    IDBG_ERROR("Error");
    return IMG_ERR_INVALID_INPUT;
  }

  switch (param) {
  case QIMG_PARAM_SET_LIB_CALLBACK:
    if (p_comp && p_comp->p_lib->img_algo_set_callback) {
      if(p_data == NULL) {
        status = p_comp->p_lib->img_algo_set_callback(
          p_comp->p_algocontext, frameproc_comp_algo_callback);
        /*Indicate if component has call back function set*/
        if (IMG_SUCCEEDED(status)) {
          p_comp->is_async = TRUE;
        }
      } else {
        status = p_comp->p_lib->img_algo_set_callback(
          p_comp->p_algocontext, p_data);
        /*Indicate if component has call back function set*/
        if (IMG_SUCCEEDED(status)) {
          p_comp->is_async = TRUE;
        }
      }
    }
    break;
  case QIMG_PARAM_LIB_CONFIG:
    if (p_comp && p_comp->p_lib->img_algo_set_lib_config) {
      if (p_data) {
        status = p_comp->p_lib->img_algo_set_lib_config(
          p_comp->p_algocontext, p_data);
      } else {
        status = IMG_ERR_INVALID_INPUT;
        IDBG_ERROR("Invalid input");
      }
    }
    break;
  default:
    status = p_comp->b.ops.set_parm(&p_comp->b, param, p_data);
    break;
  }
  IDBG_LOW("param 0x%x", param);
  return status;
}

/**
 * Function: frameproc_comp_deinit
 *
 * Description: Un-initializes the frameproc component
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
int frameproc_comp_deinit(void *handle)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, status);
    return IMG_ERR_INVALID_INPUT;
  }

  status = p_comp->b.ops.abort(handle, NULL);
  if (status < 0)
    return status;

  if (p_comp->p_lib->img_algo_deinit)
    status = p_comp->p_lib->img_algo_deinit(p_comp->p_algocontext);

  if (IMG_ERROR(status)) {
    IDBG_MED("%s:%d] Error status %d", __func__, __LINE__, status);
  }

  status = p_comp->b.ops.deinit(&p_comp->b);
  if (status < 0)
    return status;

  img_q_deinit(&p_comp->msgQ);
  free(p_comp);

  return status;
}

/**
 * Function: frameproc_comp_check_create_bundle
 *
 * Description: This function is used to check and create the
 *            bundle if needed
 *
 * Input parameters:
 *   p_comp - The pointer to the component handle.
 *
 * Return values:
 *     Imaging error values
 *
 * Notes: none
 **/
int frameproc_comp_check_create_bundle(frameproc_comp_t *p_comp)
{
  int8_t create_bundle = FALSE;
  int status = TRUE;
  img_component_t *p_base = &p_comp->b;
  int inputQcnt = img_q_count(&p_base->inputQ);
  int outputQcnt = img_q_count(&p_base->outBufQ);
  int metaQcount = img_q_count(&p_base->metaQ);
  img_msg_t *p_msg = NULL;
  int i = 0;

  if ((inputQcnt >= p_base->caps.num_input) &&
    (outputQcnt >= p_base->caps.num_output) &&
    (metaQcount >= p_base->caps.num_meta)) {
    p_msg = malloc(sizeof(img_msg_t));
    if (!p_msg) {
      IDBG_ERROR("%s:%d] Cannot create bundle", __func__, __LINE__);
      return IMG_ERR_NO_MEMORY;
    }
    memset(p_msg, 0x0, sizeof(img_msg_t));

    /* input */
    for (i = 0; i < p_base->caps.num_input; i++) {
      p_msg->bundle.p_input[i] = img_q_dequeue(&p_base->inputQ);
      if (!p_msg->bundle.p_input[i]) {
        IDBG_ERROR("%s:%d] Cannot dequeue in frame", __func__, __LINE__);
        goto error;
      }
    }
    /* output */
    for (i = 0; i < p_base->caps.num_output; i++) {
      p_msg->bundle.p_output[i] = img_q_dequeue(&p_base->outBufQ);
      if (!p_msg->bundle.p_output[i]) {
        IDBG_ERROR("%s:%d] Cannot dequeue out frame", __func__, __LINE__);
        goto error;
      }
    }
    /* meta */
    for (i = 0; i < p_base->caps.num_meta; i++) {
      p_msg->bundle.p_meta[i] = img_q_dequeue(&p_base->metaQ);
      if (!p_msg->bundle.p_meta[i]) {
        IDBG_ERROR("%s:%d] Cannot dequeue meta frame", __func__, __LINE__);
        goto error;
      }
    }

    status = img_q_enqueue(&p_comp->msgQ, p_msg);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("%s:%d] Cannot enqueue bundle", __func__, __LINE__);
      return status;
    }
    create_bundle = TRUE;
  }
  IDBG_MED("%s:%d] (%d %d) (%d %d) (%d %d) flag %d",
    __func__, __LINE__,
    inputQcnt,
    p_base->caps.num_input,
    outputQcnt,
    p_base->caps.num_output,
    metaQcount,
    p_base->caps.num_meta,
    create_bundle);

  /* signal the component */
  if (create_bundle) {
    img_q_signal(&p_comp->msgQ);
  }
  return IMG_SUCCESS;

error:
  if (p_msg) {
    /* input */
    for (i = 0; i < p_base->caps.num_input; i++) {
      if (p_msg->bundle.p_input[i])
        free(p_msg->bundle.p_input[i]);
    }
    /* output */
    for (i = 0; i < p_base->caps.num_output; i++) {
      if (p_msg->bundle.p_output[i])
        free(p_msg->bundle.p_output[i]);
    }
    /* meta */
    for (i = 0; i < p_base->caps.num_meta; i++) {
      if (p_msg->bundle.p_meta[i])
        free(p_msg->bundle.p_meta[i]);
    }
    free(p_msg);
  }
  return IMG_ERR_GENERAL;
}

/**
 * Function: frameproc_comp_queue_buffer
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
int frameproc_comp_queue_buffer(void *handle, img_frame_t *p_frame,
  img_type_t type)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  img_component_t *p_base = &p_comp->b;
  img_queue_t *queue = (type == IMG_OUT) ? &p_base->outBufQ : &p_base->inputQ;

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT)
    && (p_base->state != IMG_STATE_STARTED)) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__,
      p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  status = img_q_enqueue(queue, p_frame);
  if (status < 0) {
    IDBG_ERROR("%s:%d] Error enqueue", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return status;
  }

  if (type & IMG_IN) {
    FRAME_PROC_Q_MSG(p_comp, IMG_MSG_FRAME, p_frame, p_frame);
  }

  IDBG_MED("%s:%d] q_count %d", __func__, __LINE__, img_q_count(queue));
  frameproc_comp_check_create_bundle(p_comp);

  pthread_mutex_unlock(&p_base->mutex);
  return IMG_SUCCESS;
}

/**
 * Function: frameproc_comp_queue_buffer
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
int frameproc_comp_queue_metabuffer(void *handle, img_meta_t *p_metabuffer)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, status);
    return IMG_ERR_INVALID_INPUT;
  }

  img_component_t *p_base = &p_comp->b;
  img_queue_t *queue = &p_base->metaQ;

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT)
    && (p_base->state != IMG_STATE_STARTED)) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__,
      p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  status = img_q_enqueue(queue, p_metabuffer);
  if (status < 0) {
    IDBG_ERROR("%s:%d] Error enqueue", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return status;
  }

  FRAME_PROC_Q_MSG(p_comp, IMG_MSG_META, p_meta, p_metabuffer);

  IDBG_MED("%s:%d] q_count %d", __func__, __LINE__, img_q_count(queue));
  frameproc_comp_check_create_bundle(p_comp);

  pthread_mutex_unlock(&p_base->mutex);
  return IMG_SUCCESS;
}


/**
 * Function: frameproc_comp_flush
 *
 * Description: Function to return the buffers to client
 *
 * Input parameters:
 *   handle - The pointer to the component object
 *   type - buffer type
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int frameproc_comp_flush(void *handle, img_type_t type)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *) handle;

  IDBG_LOW("E");

  if (!p_comp) {
    IDBG_ERROR("Invalid Input");
    return IMG_ERR_INVALID_INPUT;
  }

  img_component_t *p_base = &p_comp->b;

  pthread_mutex_lock(&p_base->mutex);

  while ((p_base->processing == TRUE) || img_q_count(&p_comp->msgQ)) {
    pthread_cond_wait(&p_base->cond, &p_base->mutex);
  }
  frameproc_flush_buffers(p_comp, 0);

  p_base->flush_in_progress = FALSE;

  pthread_mutex_unlock(&p_base->mutex);

  IDBG_LOW("X");

  return IMG_SUCCESS;
}

/**
 * Function: frameproc_comp_notify_event
 *
 * Description: Function to notify event to wrapper
 *
 * Input parameters:
 *   handle - The pointer to the component object
 *   type - buffer type
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int frameproc_comp_notify_event(void *handle, comp_event_notification_t type)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *) handle;

  IDBG_LOW("E");

  if (!p_comp) {
    IDBG_ERROR("Invalid Input");
    return IMG_ERR_INVALID_INPUT;
  }
  img_component_t *p_base = &p_comp->b;

  pthread_mutex_lock(&p_base->mutex);
  IDBG_LOW("type %s %d",__func__,type);
  if (type == IMG_COMP_EVENT_FLUSH) {
    if (p_comp->p_lib->img_algo_flush) {
      IDBG_LOW("%s calling img_algo_flush",__func__);
      p_comp->p_lib->img_algo_flush(p_comp->p_algocontext);
    }
  }

  pthread_mutex_unlock(&p_base->mutex);

  IDBG_LOW("X");

  return IMG_SUCCESS;
}

/**
 * Function: frameproc_comp_abort
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
int frameproc_comp_abort(void *handle, void *p_data)
{
  IMG_UNUSED(p_data);

  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  img_component_t *p_base = &p_comp->b;

  IDBG_HIGH("%s:%d] state %d", __func__, __LINE__, p_base->state);
  pthread_mutex_lock(&p_base->mutex);
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
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: frameproc_comp_start
 *
 * Description: Start the execution of frameproc
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
int frameproc_comp_start(void *handle, void *p_data)
{
  frameproc_comp_t *p_comp = (frameproc_comp_t *)handle;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT) ||
    (NULL == p_base->thread_loop)) {
    IDBG_ERROR("%s:%d] Error state %d", __func__, __LINE__,
      p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_NOT_SUPPORTED;
  }

  if ((p_base->caps.num_input <= 0) ||
    (p_base->caps.num_output < 0) ||
    (p_base->caps.num_meta < 0) ||
    (p_base->caps.num_input > IMG_MAX_INPUT_FRAME) ||
    (p_base->caps.num_output > IMG_MAX_OUTPUT_FRAME) ||
    (p_base->caps.num_meta > IMG_MAX_META_FRAME)) {
    IDBG_ERROR("%s:%d] Error caps not set", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (p_data != NULL) {
    img_caps_t* update_caps = p_data;
    p_base->caps.num_input = update_caps->num_input;
    p_base->caps.num_output = update_caps->num_output;
    IDBG_HIGH("%s: updated caps input %d, output = %d",
      __func__, p_base->caps.num_input, p_base->caps.num_output);
  }

  /* flush the queues */
  img_q_flush(&p_base->inputQ);
  img_q_flush(&p_base->outBufQ);
  img_q_flush(&p_base->metaQ);
  img_q_flush(&p_comp->msgQ);

  pthread_mutex_unlock(&p_base->mutex);

  status = p_comp->b.ops.start(&p_comp->b, p_data);

  return status;
}

/**
 * Function: frameproc_comp_create
 *
 * Description: This function is used to create Qualcomm frameproc
 *   component
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
int frameproc_comp_create(void* handle, img_component_ops_t *p_ops)
{
  frameproc_comp_t *p_comp = NULL;
  int status;
  frameproc_lib_info_t *p_frameproc_lib = (frameproc_lib_info_t *)handle;

  if (!handle) {
    IDBG_ERROR("%s:%d] Error invalid handle", __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (NULL == p_frameproc_lib->ptr) {
    IDBG_ERROR("%s:%d] library not loaded", __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (NULL == p_ops) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }


  p_comp = (frameproc_comp_t *)malloc(sizeof(frameproc_comp_t));
  if (NULL == p_comp) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  memset(p_comp, 0x0, sizeof(frameproc_comp_t));
  status = img_comp_create(&p_comp->b);
  if (status < 0) {
    free(p_comp);
    return status;
  }

  /*set the main thread*/
  p_comp->b.thread_loop = frameproc_thread_loop;
  p_comp->b.p_core = p_comp;
  p_comp->p_lib = p_frameproc_lib;
  pthread_mutex_init(&p_comp->mutex, NULL);
  pthread_cond_init(&p_comp->cond, NULL);
  img_q_init(&p_comp->msgQ, "msgQ");

  /* copy the ops table from the base component */
  *p_ops = p_comp->b.ops;
  p_ops->init            = frameproc_comp_init;
  p_ops->deinit          = frameproc_comp_deinit;
  p_ops->set_parm        = frameproc_comp_set_param;
  p_ops->get_parm        = frameproc_comp_get_param;
  p_ops->start           = frameproc_comp_start;
  p_ops->queue_buffer    = frameproc_comp_queue_buffer;
  p_ops->queue_metabuffer = frameproc_comp_queue_metabuffer;
  p_ops->abort           = frameproc_comp_abort;
  p_ops->flush           = frameproc_comp_flush;
  p_ops->notify_event    = frameproc_comp_notify_event;

  p_ops->handle = (void *)p_comp;
  return IMG_SUCCESS;
}

/**
 * Function: frameproc_comp_load_fn_ptrs
 *
 * Description: This function is used to load Qualcomm frameproc
 * library
 *
 * Input parameters:
 *   @p_frameproc_lib: library handle
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_NOT_FOUND
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
static int frameproc_comp_load_fn_ptrs(frameproc_lib_info_t *p_frameproc_lib)
{
  int rc = IMG_SUCCESS;

  if (!p_frameproc_lib) {
    IDBG_ERROR("invalid input ");
    return IMG_ERR_INVALID_INPUT;
  }

  IMG_DLSYM_ERROR_RET(p_frameproc_lib, ptr, img_algo_init,
    "img_algo_init", 1);
  IMG_DLSYM_ERROR_RET(p_frameproc_lib, ptr, img_algo_deinit,
    "img_algo_deinit", 1);
  IMG_DLSYM_ERROR_RET(p_frameproc_lib, ptr, img_algo_process,
    "img_algo_process", 1);
  IMG_DLSYM_ERROR_RET(p_frameproc_lib, ptr, img_algo_frame_ind,
    "img_algo_frame_ind", 0);
  IMG_DLSYM_ERROR_RET(p_frameproc_lib, ptr, img_algo_meta_ind,
    "img_algo_meta_ind", 0);
  IMG_DLSYM_ERROR_RET(p_frameproc_lib, ptr, img_algo_set_frame_ops,
    "img_algo_set_frame_ops", 0);
  IMG_DLSYM_ERROR_RET(p_frameproc_lib, ptr, img_algo_preload,
    "img_algo_preload", 0);
  IMG_DLSYM_ERROR_RET(p_frameproc_lib, ptr, img_algo_shutdown,
    "img_algo_shutdown", 0);
  IMG_DLSYM_ERROR_RET(p_frameproc_lib, ptr, img_algo_start,
    "img_algo_start", 0);
  IMG_DLSYM_ERROR_RET(p_frameproc_lib, ptr, img_algo_stop,
    "img_algo_stop", 0);
  IMG_DLSYM_ERROR_RET(p_frameproc_lib, ptr, img_algo_set_callback,
    "img_algo_set_callback", 0);
  IMG_DLSYM_ERROR_RET(p_frameproc_lib, ptr, img_algo_set_lib_config,
    "img_algo_set_lib_config", 0);
  IMG_DLSYM_ERROR_RET(p_frameproc_lib, ptr, img_algo_get_lib_config,
    "img_algo_get_lib_config", 0);
  IMG_DLSYM_ERROR_RET(p_frameproc_lib, ptr, img_algo_post_process,
    "img_algo_post_process", 0);
  IMG_DLSYM_ERROR_RET(p_frameproc_lib, ptr, img_algo_flush,
    "img_algo_flush", 0);
  return rc;
}

/**
 * Function: frame_proc_set_base_ops
 *
 * Description: Set the base ops
 *
 * Input parameters:
 *   @p_frameproc_lib: library handle
 *
 * Return values:
 *   none
  *
 * Notes: none
 **/
static void frame_proc_set_base_ops(frameproc_lib_info_t *p_frameproc_lib)
{
  /* memory operations */
  p_frameproc_lib->base_ops.mem_ops.open            = img_buffer_open;
  p_frameproc_lib->base_ops.mem_ops.get_buffer      = img_buffer_get;
  p_frameproc_lib->base_ops.mem_ops.release_buffer  = img_buffer_release;
  p_frameproc_lib->base_ops.mem_ops.cache_ops       = img_buffer_cacheops;
  p_frameproc_lib->base_ops.mem_ops.close           = img_buffer_close;

  /*Thread operations */
  p_frameproc_lib->base_ops.thread_ops.schedule_job =
    img_thread_mgr_schedule_job;
  p_frameproc_lib->base_ops.thread_ops.get_time_of_job =
    img_thread_mgr_get_time_of_job;
  p_frameproc_lib->base_ops.thread_ops.get_time_of_joblist =
    img_thread_mgr_get_time_of_joblist;
  p_frameproc_lib->base_ops.thread_ops.wait_for_completion_by_jobid =
    img_thread_mgr_wait_for_completion_by_jobid;
  p_frameproc_lib->base_ops.thread_ops.wait_for_completion_by_joblist =
    img_thread_mgr_wait_for_completion_by_joblist;
  p_frameproc_lib->base_ops.thread_ops.wait_for_completion_by_clientid =
    img_thread_mgr_wait_for_completion_by_clientid;
}

/**
 * Function: frameproc_comp_load
 *
 * Description: This function is used to load Qualcomm frameproc
 * library
 *
 * Input parameters:
 *   @name: library name
 *   @handle: library handle
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_NOT_FOUND
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int frameproc_comp_load(const char* name, void** handle)
{
  frameproc_lib_info_t *p_frameproc_lib;
  IDBG_LOW(" %s is Loading", name);

  if (!name || !handle) {
    IDBG_ERROR("%s:%d] invalid input %p %p",
      __func__, __LINE__, name, handle);
    return IMG_ERR_INVALID_INPUT;
  }

  p_frameproc_lib = malloc(sizeof(frameproc_lib_info_t));
  if (!p_frameproc_lib) {
    IDBG_ERROR("%s:%d] cannot alloc p_frameproc_lib %s",
      __func__, __LINE__, name);
    return IMG_ERR_NOT_FOUND;
  }
  if (IMG_SUCCESS != frameproc_comp_reload_lib(p_frameproc_lib, name, NULL)) {
    free(p_frameproc_lib);
    IDBG_ERROR("%s:%d] cannot load %s",
      __func__, __LINE__, name);
    return IMG_ERR_NOT_FOUND;
  }

  /* set baseops */
  frame_proc_set_base_ops(p_frameproc_lib);

  img_dsp_dlopen(name, p_frameproc_lib, frameproc_comp_unload_lib,
    frameproc_comp_reload_lib);
  *handle = p_frameproc_lib;
  IDBG_HIGH("%s:%d] %s loaded successfully", __func__, __LINE__, name);
  return IMG_SUCCESS;
}

/**
 * Function: frameproc_comp_unload
 *
 * Description: This function is used to unload Qualcomm frameproc
 * library
 *
 * Input parameters:
 *   @handle: library handle
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void frameproc_comp_unload(void* handle)
{
  frameproc_lib_info_t *p_frameproc_lib = (frameproc_lib_info_t *)handle;
  IDBG_HIGH("%s:%d] ", __func__, __LINE__);

  if (!p_frameproc_lib) {
    IDBG_ERROR("%s:%d] Error unloading library", __func__, __LINE__);
    return;
  }
  if (p_frameproc_lib->ptr) {
    frameproc_comp_unload_lib(p_frameproc_lib, NULL);
    img_dsp_dlclose(p_frameproc_lib);
  }
  free(p_frameproc_lib);
}

/**
 * Function: frameproc_comp_unload_lib
 *
 * Description: Callback to be called from dsp dl mgr to unload
 *   library
 *
 * Input parameters:
 *   @handle: library handle
 *   @p_userdata: pointer to userdata
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
  *
 * Notes: none
 **/
static int frameproc_comp_unload_lib(void *handle, void *p_userdata)
{
  int rc = 0;
  frameproc_lib_info_t *p_frameproc_lib = (frameproc_lib_info_t *)handle;
  uint8_t *p_ssr_flag = (uint8_t *)p_userdata;
  IDBG_HIGH("%s:%d] ", __func__, __LINE__);

  if (!p_frameproc_lib) {
    IDBG_ERROR("%s:%d] Error unloading library", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  if ((p_ssr_flag != NULL) && (*p_ssr_flag)) {
    if (p_frameproc_lib->img_algo_shutdown) {
      p_frameproc_lib->img_algo_shutdown(&p_frameproc_lib->base_ops);
    }
  }

  if (p_frameproc_lib->ptr) {
    rc = dlclose(p_frameproc_lib->ptr);
    if (rc < 0)
      IDBG_ERROR("error %s", dlerror());
  }

  p_frameproc_lib->ptr                    = NULL;
  p_frameproc_lib->img_algo_init          = NULL;
  p_frameproc_lib->img_algo_deinit        = NULL;
  p_frameproc_lib->img_algo_process       = NULL;
  p_frameproc_lib->img_algo_post_process  = NULL;
  p_frameproc_lib->img_algo_frame_ind     = NULL;
  p_frameproc_lib->img_algo_meta_ind      = NULL;
  p_frameproc_lib->img_algo_set_frame_ops = NULL;
  p_frameproc_lib->img_algo_preload       = NULL;
  p_frameproc_lib->img_algo_shutdown      = NULL;
  p_frameproc_lib->img_algo_set_callback  = NULL;
  p_frameproc_lib->img_algo_get_lib_config = NULL;
  p_frameproc_lib->img_algo_set_lib_config = NULL;

  return IMG_SUCCESS;

}

/**
 * Function: frameproc_comp_reload_lib
 *
 * Description: Callback to be called from dsp dl mgr to reload
 *   library
 *
 * Input parameters:
 *   @handle: library handle
 *   @name: library name
 *   @p_userdata: pointer to userdata
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_NOT_GENERAL
 *   IMG_ERR_NOT_FOUND
 *
 * Notes: none
 **/
static int frameproc_comp_reload_lib(void *handle,
  const char *name,  void *p_userdata)
{
  frameproc_lib_info_t *p_frameproc_lib = (frameproc_lib_info_t *)handle;
  uint8_t *p_ssr_flag = (uint8_t *)p_userdata;
  IDBG_HIGH("+");

  if (!p_frameproc_lib || !name) {
    IDBG_ERROR("Error loading library %p", p_frameproc_lib);
    return IMG_ERR_GENERAL;
  }
  p_frameproc_lib->ptr = dlopen(name, RTLD_NOW);
  if (!p_frameproc_lib->ptr) {
    IDBG_ERROR("Error opening frameproc library %s error %s\n",
      name, dlerror());
    return IMG_ERR_NOT_FOUND;
  }

  if (IMG_SUCCESS != frameproc_comp_load_fn_ptrs(p_frameproc_lib)) {
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_HIGH("%s loaded successfully", name);

  if ((p_ssr_flag != NULL) && (*p_ssr_flag)) {
    if (p_frameproc_lib->img_algo_preload) {
      p_frameproc_lib->img_algo_preload(&p_frameproc_lib->base_ops, NULL);
      //TODO : save and restore preload params for quadcfa  etc. instead of NULL
    }
  }

  return IMG_SUCCESS;

}

/**
 * Function: frameproc_comp_alloc
 *
 * Description: This function is used to Allocate buffers
 * library
 *
 * Input parameters:
 *   @handle: library handle
 *   @p_params: preload parameters
 *
 * Return values:
 *     Standard Imglb Error values
 *
 * Notes: none
 **/
int frameproc_comp_alloc(void *handle, void *p_params)
{
  int32_t status = IMG_SUCCESS;
  frameproc_lib_info_t *p_frameproc_lib = (frameproc_lib_info_t *)handle;
  img_preload_param_t *p_preload_param = (img_preload_param_t*)p_params;
  img_dim_t *p_dim = &p_preload_param->max_dim;
  IDBG_HIGH("%s:%d] ", __func__, __LINE__);

  if (!p_frameproc_lib) {
    IDBG_ERROR("%s:%d] Error alloc library", __func__, __LINE__);
    status = IMG_ERR_GENERAL;
    goto end;
  }

  if (p_frameproc_lib->img_algo_preload && p_dim) {
    p_frameproc_lib->base_ops.max_w = p_dim->stride;
    p_frameproc_lib->base_ops.max_h = p_dim->height;
    status = p_frameproc_lib->img_algo_preload(&p_frameproc_lib->base_ops,
      p_params);
  }

end:
  return status;
}

/**
 * Function: frameproc_comp_dealloc
 *
 * Description: This function is used to deallocate buffers in
 * the library
 *
 * Input parameters:
 *   @handle: library handle
 *
 * Return values:
 *     Standard Imglb Error values
 *
 * Notes: none
 **/
int frameproc_comp_dealloc(void *handle)
{
  int32_t status = IMG_SUCCESS;
  frameproc_lib_info_t *p_frameproc_lib = (frameproc_lib_info_t *)handle;
  IDBG_HIGH("%s:%d] ", __func__, __LINE__);

  if (!p_frameproc_lib) {
    IDBG_ERROR("%s:%d] Error dealloc library", __func__, __LINE__);
    status = IMG_ERR_GENERAL;
    goto end;
  }

  if (p_frameproc_lib->img_algo_shutdown)
    status = p_frameproc_lib->img_algo_shutdown(&p_frameproc_lib->base_ops);

end:
  return status;
}

/**
 * Function: frameproc_comp_preload_needed
 *
 * Description: This function is used to indicate whether
 *              preload is needed
 *
 * Input parameters:
 *   @handle: library handle
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
bool frameproc_comp_preload_needed(void *handle)
{
  bool preload_needed = false;
  frameproc_lib_info_t *p_frameproc_lib = (frameproc_lib_info_t *)handle;

  if (!p_frameproc_lib) {
    IDBG_ERROR("%s:%d] Error alloc library", __func__, __LINE__);
  } else {
    if (p_frameproc_lib->img_algo_preload) {
      preload_needed = true;
    }
    IDBG_MED("preload_needed %d", preload_needed);
  }

  return preload_needed;
}
