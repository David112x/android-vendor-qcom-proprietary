/**********************************************************************
*  Copyright (c) 2013-2015 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __IMG_COMPONENT_PRIV_H__
#define __IMG_COMPONENT_PRIV_H__

#include "img_comp.h"
#include "img_queue.h"
#include "img_mem_ops.h"
#include "img_thread.h"

/** IMG_CHK_ABORT
 *   @p: pointer to the image component
 *
 *   Check if the component can be aborted
 **/
#define IMG_CHK_ABORT(p) ((p)->state == IMG_STATE_STOP_REQUESTED)

/** IMG_CHK_ABORT_RET
 *   @p: pointer to the image component
 *
 *   Check if the component can be aborted and return the
 *   function
 **/
#define IMG_CHK_ABORT_RET(p) ({ \
  if ((p)->state == IMG_STATE_STOP_REQUESTED) \
    return 0; \
})

/** IMG_CHK_ABORT_RET_LOCKED
 *   @p: pointer to the image component
 *   @m: pointer to the mutex
 *
 *   Check if the component can be aborted with lock. If true
 *   unlock the mutex and return the function execution
 **/
#define IMG_CHK_ABORT_RET_LOCKED(p, m) ({ \
  pthread_mutex_lock(m); \
  if ((p)->state == IMG_STATE_STOP_REQUESTED) { \
    pthread_mutex_unlock(m); \
    return 0; \
  } \
  pthread_mutex_unlock(m); \
})

/** IMG_CHK_ABORT_LOCKED
 *   @p: pointer to the image component
 *   @m: pointer to the mutex
 *
 *   Check if the component can be aborted with lock.
 **/
#define IMG_CHK_ABORT_LOCKED(p, m) ({ \
  int abort_flag; \
  pthread_mutex_lock(m); \
  abort_flag = ((p)->state == IMG_STATE_STOP_REQUESTED); \
  pthread_mutex_unlock(m); \
  abort_flag; \
})

/** IMG_CHK_FLUSH_LOCKED
 *   @p: pointer to the image component
 *   @m: pointer to the mutex
 *
 *   Check if the component can be aborted with lock.
 **/
#define IMG_CHK_FLUSH_LOCKED(p, m) ({ \
  int flush_flag; \
  pthread_mutex_lock(m); \
  flush_flag = ((p)->flush_in_progress == TRUE); \
  pthread_mutex_unlock(m); \
  flush_flag; \
})

/** IMG_CHK_ABORT_UNLK_RET
 *   @p: pointer to the image component
 *   @m: pointer to the mutex
 *
 *   Check if the component can be aborted. If true unlock the
 *   mutex and return the function execution
 **/
#define IMG_CHK_ABORT_UNLK_RET(p, m) ({ \
  if ((p)->state == IMG_STATE_STOP_REQUESTED) { \
    pthread_mutex_unlock(m); \
    return 0; \
  } \
})

/** IMG_SEND_EVENT
 *   @p_base: pointer to the image component
 *   @evt_type: event type
 *
 *   If the callback is registered, fill the event type and
 *   issue the callback
 **/
#define IMG_SEND_EVENT(p_base, evt_type) ({ \
  img_event_t event; \
  event.type = evt_type; \
  if ((p_base)->p_cb) \
    (p_base)->p_cb((p_base)->p_userdata, &event); \
  IDBG_MED("%s:%d] send event", __func__, __LINE__); \
})

/** IMG_SEND_EVENT_PYL
 *   @p_base: pointer to the image component
 *   @evt_type: event type
 *   @d_type : data type
 *   @data: pointer to the data
 *
 *   If the callback is registered, fill the event type, add the
 *   payload and issue the callback
 **/
#define IMG_SEND_EVENT_PYL(p_base, evt_type, d_type, data) ({ \
  img_event_t event; \
  event.type = evt_type; \
  event.d.d_type = data; \
  if ((p_base)->p_cb) \
    (p_base)->p_cb(p_base->p_userdata, &event); \
})

/** IMG_DLSYM_ERROR_RET:
 *
 * @p_lib: ponter to the faceproc lib
 * @ptr: pointer name of the lib handle
 * @fptr_name: name of the function pointer
 * @str_name: name of the symbol to be loaded
 * @error: indicate if its fatal error
 *
 *   Helper macro to set the symbol and assign the pointer
 */
#define IMG_DLSYM_ERROR_RET(p_lib, ptr, fptr_name, sym_name, error) ({\
  *(void **)&(p_lib->fptr_name) = dlsym(p_lib->ptr, sym_name); \
  if (p_lib->fptr_name == NULL) { \
    if (error) { \
      IDBG_ERROR("Loading %s error %s", sym_name, dlerror()); \
      dlclose(p_lib->ptr); \
      p_lib->ptr = NULL; \
      return IMG_ERR_NOT_FOUND; \
    } else { \
      IDBG_HIGH("Loading %s warning %s", sym_name, dlerror()); \
    } \
  } \
})

/** IMG_SET_START_PROCESSING
 *   @p_base: pointer to the image component
 *
 *   Sets the variable that processing has started.
 **/
#define IMG_SET_PROCESSING_START(p_base) ({ \
  pthread_mutex_lock(&p_base->mutex); \
  p_base->processing = TRUE; \
  pthread_mutex_unlock(&p_base->mutex); \
})

/** IMG_SET_END_PROCESSING
 *   @p_base: pointer to the image component
 *
 *   Sets the variable that processing has ended.
 **/
#define IMG_SET_PROCESSING_COMPLETE(p_base) ({ \
  pthread_mutex_lock(&p_base->mutex); \
  p_base->processing = FALSE; \
  pthread_cond_signal(&p_base->cond); \
  pthread_mutex_unlock(&p_base->mutex); \
})

/** thread_func_t
 *   @p_data: pointer to the data
 *
 *   Thread function
 **/
typedef void *(*thread_func_t) (void *p_data);

/** img_component_t
 *   @inputQ: queue for storing the input buffers
 *   @outBufQ: queue for storing the output buffers
 *   @outputQ: queue for storing the buffers after processed
 *   @metaQ: metadata queue
 *   @mutex : component lock
 *   @cond: conditional variable for the component
 *   @threadid: Execution thread id
 *   @p_userdata: Pointer to the userdata
 *   @thread_loop: Function pointer for the execution main
 *               function
 *   @thread_exit: Flag indicates whether the thread can be
 *               exited
 *   @p_cb: Pointer to the callback function
 *   @error: Variable for storing the last error
 *   @state: State of the component
 *   @is_ready: Flag to indicate whether the flag is ready
 *   @ops: Function table to store the component operations
 *   @frame_info: Frame information for the buffers. Used for
 *              components which supports only one dimension per
 *              session.
 *   @p_core: Pointer to the derived class component
 *   @mode: Executed in sync or Async mode
 *   @img_debug_info: Info to enable additional debug options
 *   @caps: capability of the component
 *   @mem_ops: memory operations table
 *   @thread_ops: thread operations table
 *   @frame_ops: frame operations
 *   @processing: Whether the component is processing any buffer
 *   @flush_in_progress: Whether Flush is in progress
 *
 *   If the callback is registered, fill the event type, add the
 *   payload and issue the callback
 **/
typedef struct {
  img_queue_t inputQ;
  img_queue_t outBufQ;
  img_queue_t outputQ;
  img_queue_t metaQ;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_t threadid;
  void *p_userdata;
  thread_func_t thread_loop;
  int thread_exit;
  notify_cb p_cb;
  int error;
  comp_state_t state;
  int is_ready;
  img_component_ops_t ops;
  img_frame_info_t frame_info;
  void *p_core;
  img_comp_mode_t mode;
  img_debug_info_t debug_info;
  img_caps_t caps;
  img_mem_ops_t mem_ops;
  img_thread_ops_t thread_ops;
  img_frame_ops_t frame_ops;
  uint8_t processing;
  uint8_t flush_in_progress;
} img_component_t;

/** img_comp_create
 *   @p_comp: Pointer to the component
 *
 *   Creates the base component
 **/
int img_comp_create(img_component_t *p_comp);

#endif //__IMG_COMPONENT_PRIV_H__
