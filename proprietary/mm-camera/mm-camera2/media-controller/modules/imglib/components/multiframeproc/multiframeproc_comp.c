/**********************************************************************
* Copyright (c) 2016-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "multiframeproc_comp.h"
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
static int multi_frameproc_mgr_thread_loop(void *handle);
static int multi_frameproc_comp_deinit(void *handle);
static void multi_frameproc_comp_send_buffers(multi_frameproc_comp_t *p_comp,
  img_frame_bundle_t *p_bundle,
  int status,
  bool send_status);
static int multi_frameproc_core_reload_lib(void *handle,
  const char *name,  void *p_userdata);
void multi_frameproc_core_unload(void* handle);
int multi_frameproc_comp_check_create_bundle(multi_frameproc_comp_t *p_comp);
static void multi_frameproc_comp_flush_buffers(multi_frameproc_comp_t *p_comp,
  int status, bool send_status);
static void multi_frameproc_comp_remove_buffer(multi_frameproc_mgr_t *p_comp,
  int status, bool send_status, uint32_t frame_id);
int multi_frameproc_comp_frame_drop(void *handle, void *p_data);

/**
 * CONSTANTS and MACROS
 **/

/** MULTI_FRAMEPROC_GET_UNUSED_MGR:
 *
 *  @p_session: ptr to SAT session data
 *
 *   Returns index of the output stream
 */
#define MULTI_FRAMEPROC_GET_UNUSED_MGR(p_lib, p_mgr) ({ \
  for (i = 0; i < IMGLIB_ARRAY_SIZE(p_lib->mgr); i++) { \
    p_mgr = &p_lib->mgr[i]; \
    if (!p_mgr->is_used) { \
      break; \
    } \
  } \
}) \

 /**
 * Function: multi_frameproc_mgr_send_message
 *
 * Description: Queues a message to the multi_frameproc message
 * queue and signals main thread
 *
 * Input parameters:
 *   @p_comp: The pointer to the component handle
 *   @p_mgr: Pointer to the manager handle
 *   @msg_type: message type
 *   @p_frame: pointer to the frame
 *   @p_meta: pointer to the meta
 *   @p_data: pointer to data
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_NO_MEMORY
 *
 * Notes: none
 **/
static inline int32_t multi_frameproc_mgr_send_message(
  multi_frameproc_comp_t *p_comp,
  multi_frameproc_mgr_t *p_mgr,
  img_msg_type_t msg_type,
  img_frame_t *p_frame,
  img_meta_t *p_meta,
  void *p_data)
{
  int status = IMG_SUCCESS;
  img_msg_t *p_msg = NULL;

  if (p_comp && !p_comp->p_mgr) {
    IDBG_ERROR("comp manager NULL");
    return IMG_ERR_INVALID_OPERATION;
  } else if (!p_mgr) {
    IDBG_ERROR("comp manager NULL");
    return IMG_ERR_INVALID_OPERATION;
  }

  p_msg = calloc(1, sizeof(img_msg_t));
  if (!p_msg) {
    IDBG_ERROR("cannot alloc message");
    status = IMG_ERR_NO_MEMORY;
  } else {
    p_msg->type = msg_type;
    switch (msg_type) {
    case IMG_MSG_FRAME:
      p_msg->p_frame = p_frame;
      break;
    case IMG_MSG_META:
      p_msg->p_meta = p_meta;
      break;
    case IMG_MSG_UPDATE_MASTER:
      p_msg->p_data = p_data;
    default:;
    }
    p_msg->p_sender = p_comp;
    status = img_q_enqueue(&p_mgr->msgQ, p_msg);
    IDBG_MED("Enqueued message %d", p_msg->type);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("Cannot enqueue bundle");
      free(p_msg);
      p_msg = NULL;
    } else {
      img_q_signal(&p_mgr->msgQ);
    }
  }
  return status;
}

/**
 * Function: multi_frameproc_mgr_update_active_comp_count
 *
 * Description: This function is used to update the active
 * component count for the manager
 *
 * Input parameters:
 *   @p_mgr: Pointer to the manager handle
 *
 * Return values:
 *     None
 *
 * Notes: none
 **/
void multi_frameproc_mgr_update_active_comp_count(
   multi_frameproc_mgr_t *p_mgr)
{
  uint32_t i;

  p_mgr->active_comp_cnt = 0;
  for (i = 0; i < p_mgr->comp_cnt; i++) {
    if (p_mgr->p_comp[i] &&
      ((p_mgr->p_comp[i]->b.state == IMG_STATE_STARTED) ||
      (p_mgr->p_comp[i]->b.state == IMG_STATE_START_PENDING))) {
      p_mgr->active_comp_cnt++;
    }
  }
}

/**
 * Function: multi_frameproc_mgr_update_active_io_count
 *
 * Description: This function is used to update the active
 * input/output/meta count for the manager
 *
 * Input parameters:
 *   @p_mgr: Pointer to the manager handle
 *
 * Return values:
 *     None
 *
 * Notes: none
 **/
void multi_frameproc_mgr_update_active_io_count(
  multi_frameproc_mgr_t *p_mgr)
{
  uint32_t i;
  p_mgr->caps.num_input = 0;
  p_mgr->caps.num_meta = 0;

  for (i = 0; i < p_mgr->active_comp_cnt; i++) {
    p_mgr->caps.num_input += p_mgr->p_comp[i]->b.caps.num_input;
    p_mgr->caps.num_meta  += p_mgr->p_comp[i]->b.caps.num_meta;
  }
  IDBG_INFO("Updated input count %d, meta count %d", p_mgr->caps.num_input,
    p_mgr->caps.num_meta);
  return;
}

/**
 * Function: multi_frameproc_mgr_bind
 *
 * Description: This function is used to bind two camera
 *              instances
 *
 * Input parameters:
 *   @p_handle1: handle of the first component
 *   @p_handle2: handle of the second component
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int multi_frameproc_mgr_bind(void *p_handle1, void *p_handle2)
{
  uint32_t i;
  int32_t rc = IMG_SUCCESS;
  multi_frameproc_comp_t *p_comp1 = (multi_frameproc_comp_t *)p_handle1;
  multi_frameproc_comp_t *p_comp2 = (multi_frameproc_comp_t *)p_handle2;
  multi_frameproc_lib_info_t *p_lib;
  multi_frameproc_mgr_t *p_mgr;
  img_multicam_init_params_t multicam_init_params;
  bool single_comp_bind = FALSE;

  IDBG_MED(" ");
  /* validate input */
  if (!p_comp1) {
    IDBG_ERROR("Error invalid component %p", p_comp1);
    return IMG_ERR_INVALID_INPUT;
  }

  if (!p_comp2) {
    IDBG_MED("component2 %p", p_comp2);
    single_comp_bind = TRUE;
  }

  if (!single_comp_bind) {
    if (!p_comp1->p_lib || !p_comp2->p_lib) {
      IDBG_ERROR("Error invalid component lib %p %p", p_comp1->p_lib,
        p_comp2->p_lib);
      return IMG_ERR_INVALID_INPUT;
    }

    /* if both are already bound, return */
    if (p_comp1->p_mgr && p_comp2->p_mgr) {
      IDBG_ERROR("Already bound %p %p", p_comp1->p_mgr, p_comp2->p_mgr);
      return IMG_ERR_INVALID_OPERATION;
    }
  } else {
    if (!p_comp1->p_lib) {
      IDBG_ERROR("Error invalid component lib %p", p_comp1->p_lib);
      return IMG_ERR_INVALID_INPUT;
    }
    /* if already bound, return */
    if (p_comp1->p_mgr) {
      IDBG_ERROR("Already bound %p", p_comp1->p_mgr);
      return IMG_ERR_INVALID_OPERATION;
    }
  }

  p_lib = p_comp1->p_lib;

  /* Find the manager */

  pthread_mutex_lock(&p_lib->mutex);

  if (!single_comp_bind) {
    /* Both are unbound? */
    if (!p_comp1->p_mgr && !p_comp2->p_mgr) {
      /* get unused manager */
      MULTI_FRAMEPROC_GET_UNUSED_MGR(p_lib, p_mgr);
    } else { /* one of them is used */
      p_mgr = p_comp1->p_mgr ? p_comp1->p_mgr : p_comp2->p_mgr;
    }
  } else {
    if (!p_comp1->p_mgr) {
      /* get unused manager */
      MULTI_FRAMEPROC_GET_UNUSED_MGR(p_lib, p_mgr);
    } else {
      p_mgr = p_comp1->p_mgr;
    }
  }

  if (!p_mgr) {
    IDBG_ERROR("Cannot find manager");
    rc = IMG_ERR_GENERAL;
    goto end;
  }

  /* set the manager */
  for (i = 0; i < IMGLIB_ARRAY_SIZE(p_mgr->p_comp); i++) {
    if (!p_mgr->p_comp[i]) {
      IDBG_MED(" Mgr %p %p", p_comp1, p_comp1->p_mgr);
      if (!p_comp1->p_mgr) {
        p_mgr->p_comp[i] = p_comp1;
        p_comp1->p_mgr = p_mgr;
        p_mgr->comp_cnt++;
      } else if (p_comp2 && !p_comp2->p_mgr) {
        IDBG_MED(" Mgr %p %p", p_comp2, p_comp2->p_mgr);
        p_mgr->p_comp[i] = p_comp2;
        p_comp2->p_mgr = p_mgr;
        p_mgr->comp_cnt++;
      }
    }
  }

  p_mgr->abort = false;

  multi_frameproc_mgr_update_active_comp_count(p_mgr);

  /* set super capability */
  p_mgr->caps.num_input = p_comp1->b.caps.num_input * p_mgr->active_comp_cnt;
  p_mgr->caps.num_output = p_comp1->b.caps.num_output;
  p_mgr->caps.num_meta = p_comp1->b.caps.num_meta * p_mgr->active_comp_cnt;



  if (!p_mgr->is_used) {
    IDBG_MED(" Cnt %d ", p_mgr->comp_cnt);
    /* reserve thread */
    img_core_type_t thread_affinity[1] = { IMG_CORE_ARM };
    img_thread_job_params_t thread_job;

    p_mgr->th_client_id =
      img_thread_mgr_reserve_threads(1, thread_affinity);

    thread_job.client_id = p_mgr->th_client_id;
    thread_job.core_affinity = IMG_CORE_ARM;
    thread_job.delete_on_completion = TRUE;
    thread_job.execute = multi_frameproc_mgr_thread_loop;
    thread_job.dep_job_count = 0;
    thread_job.args = p_mgr;
    thread_job.dep_job_ids = NULL;
    p_mgr->job_id = img_thread_mgr_schedule_job(&thread_job);

    img_q_flush(&p_mgr->msgQ);
    img_q_flush(&p_mgr->dropQ);

    multicam_init_params.num_of_sessions = p_mgr->comp_cnt;
    for (i = 0; i < p_mgr->comp_cnt; i++) {
      if (p_mgr->p_comp[i]) {
        multicam_init_params.client_id[i] =
          p_mgr->p_comp[i]->init_params.client_id;
        multicam_init_params.multicam_dim_info[i] =
          p_mgr->p_comp[i]->init_params.multicam_dim_info;
      }
    }

    /* initialize algo context */
    if (p_mgr->p_lib->img_algo_init) {
      /*ToDo: to combine init parameters */
      rc = p_mgr->p_lib->img_algo_init(&p_mgr->p_algocontext,
        &multicam_init_params);
    }

    if (p_mgr->p_lib->img_algo_set_frame_ops) {
      p_mgr->p_lib->img_algo_set_frame_ops(p_mgr->p_algocontext,
        &p_comp1->b.frame_ops);
    }
  }

  p_mgr->is_used = true;


end:
  pthread_mutex_unlock(&p_lib->mutex);
  return rc;
}


/**
 * Function: multi_frameproc_mgr_unbind
 *
 * Description: This function is used to unbind multiple camera
 *              instances
 *
 * Input parameters:
 *   @p_handle1: Handle of the main instance
 *   @p_handle2: Handle of the aux instance
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int multi_frameproc_mgr_unbind(void *p_handle1, void *p_handle2)
{
  uint32_t i;
  int32_t rc = IMG_SUCCESS;
  multi_frameproc_comp_t *p_comp1 = (multi_frameproc_comp_t *)p_handle1;
  multi_frameproc_comp_t *p_comp2 = (multi_frameproc_comp_t *)p_handle2;
  multi_frameproc_mgr_t *p_mgr;

  /* validate input */
  if (!p_comp1 || !p_comp2) {
    IDBG_ERROR("Error invalid component %p %p", p_comp1, p_comp2);
    return IMG_ERR_INVALID_INPUT;
  }

  if (!p_comp1->p_lib || !p_comp2->p_lib) {
    IDBG_ERROR("Error invalid component lib %p %p", p_comp1->p_lib,
      p_comp2->p_lib);
    return IMG_ERR_INVALID_INPUT;
  }

  /* if both are already unbound, return */
  if (!p_comp1->p_mgr || !p_comp2->p_mgr) {
    IDBG_ERROR("Error invalid component mgr %p %p", p_comp1->p_mgr,
      p_comp2->p_mgr);
    return IMG_ERR_INVALID_INPUT;
  }

  p_mgr = p_comp1->p_mgr;
  if (!p_mgr->th_client_id || !p_mgr->job_id) {
    IDBG_ERROR("Error invalid client %d or job %d",
      p_mgr->th_client_id, p_mgr->job_id);
    return IMG_ERR_INVALID_INPUT;
  }

  pthread_mutex_lock(&p_mgr->p_lib->mutex);

  /* remove the component instances */
  for (i = 0; i < IMGLIB_ARRAY_SIZE(p_mgr->p_comp); i++) {
    if ((p_mgr->p_comp[i] == p_comp1) ||
      (p_mgr->p_comp[i] == p_comp2)) {
      p_mgr->comp_cnt--;
    }
  }
  IDBG_MED("Component count %d", p_mgr->comp_cnt);

  /* deinit library */
  if (!p_mgr->comp_cnt) {
    if (p_mgr->p_lib->img_algo_deinit) {
      rc = p_mgr->p_lib->img_algo_deinit(p_mgr->p_algocontext);
      p_mgr->p_algocontext = NULL;
    }
    p_mgr->is_used = false;

    /* abort the job */
    p_mgr->abort = true;
    rc = multi_frameproc_mgr_send_message(NULL, p_mgr, IMG_MSG_KILL,
      NULL, NULL, NULL);
    if (IMG_ERROR(rc)) {
       IDBG_ERROR("Send message failed %d", rc);
       goto end;
    }

    rc = img_thread_mgr_wait_for_completion_by_clientid(p_mgr->th_client_id,
      1000);
    if (IMG_ERROR(rc)) {
      IDBG_HIGH("Error wait for completion %d", rc);
    }
    /* unreserve thread */
    rc = img_thread_mgr_unreserve_threads(p_mgr->th_client_id);
    if (IMG_ERROR(rc)) {
      IDBG_HIGH("Cannot unreserve thread %d", rc);
    }
  }

  /* remove the component instances now */
  for (i = 0; i < IMGLIB_ARRAY_SIZE(p_mgr->p_comp); i++) {
    if ((p_mgr->p_comp[i] == p_comp1) ||
      (p_mgr->p_comp[i] == p_comp2)) {
      p_mgr->p_comp[i]->p_mgr = NULL;
      p_mgr->p_comp[i] = NULL;
    }
  }

end:
  IDBG_HIGH("rc %d", rc);
  pthread_mutex_unlock(&p_mgr->p_lib->mutex);
  return rc;
}

/**
 * Function: multi_frameproc_mgr_create
 *
 * Description: This function is used to create multi frameproc
 *   manager
 *
 * Input parameters:
 *   @p_mfp_lib: Multiframeproc library pointer
 *
 * Return values:
 *     None
 *
 * Notes: none
 **/
void multi_frameproc_mgr_create(multi_frameproc_lib_info_t *p_mfp_lib)
{
  uint32_t i;
  multi_frameproc_mgr_t *p_mgr;
  IDBG_MED("");
  for (i = 0; i < IMGLIB_ARRAY_SIZE(p_mfp_lib->mgr); i++) {
    p_mgr = &p_mfp_lib->mgr[i];
    img_q_init(&p_mgr->msgQ, "msgQ");
    img_q_init(&p_mgr->dropQ, "DropQ");
    p_mgr->comp_cnt = 0;
    memset(p_mgr->p_comp, 0x0,
      sizeof(multi_frameproc_comp_t *) * MFC_MAX_CAMERA_SESSIONS);
    p_mgr->p_lib = p_mfp_lib;
    p_mgr->p_algocontext = NULL;
    p_mgr->is_used = false;
  }
}

/**
 * Function: multi_frameproc_mgr_destroy
 *
 * Description: This function is used to destroy
 *   multi frameproc managers
 *
 * Input parameters:
 *   @p_mfp_lib: Multiframeproc library pointer
 *
 * Return values:
 *     None
 *
 * Notes: none
 **/
void multi_frameproc_mgr_destroy(multi_frameproc_lib_info_t *p_mfp_lib)
{
  uint32_t i, j;
  multi_frameproc_mgr_t *p_mgr;
  IDBG_MED("");
  for (i = 0; i < IMGLIB_ARRAY_SIZE(p_mfp_lib->mgr); i++) {
    p_mgr = &p_mfp_lib->mgr[i];
    for (j = 0; j < p_mgr->comp_cnt; j++) {
      if (p_mgr->p_comp[j]) {
        multi_frameproc_comp_deinit(p_mgr->p_comp[j]);
      }
    }
    img_q_deinit(&p_mgr->msgQ);
    img_q_deinit(&p_mgr->dropQ);
  }
} /* multi_frameproc_mgr_destroy */

/**
 * Function: multi_frameproc_mgr_can_wait
 *
 * Description: Queue function to check if abort is issued
 *
 * Input parameters:
 *   @p_userdata: The pointer to multi_frameproc component
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
int multi_frameproc_mgr_can_wait(void *p_userdata)
{
  multi_frameproc_mgr_t *p_mgr = (multi_frameproc_mgr_t *)p_userdata;
  return !p_mgr->abort;
}

/**
 * Function: multi_frameproc_clear_msg_queue
 *
 * Description: Queue clear until abort
 *
 * Input parameters:
 *   @p_userdata: The pointer to multi_frameproc component
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
int multi_frameproc_clear_comp_buf_queue(  multi_frameproc_comp_t *p_comp)
{
    img_component_t *p_base = &p_comp->b;
    int inputQcnt = img_q_count(&p_base->inputQ);
    int outputQcnt = img_q_count(&p_base->outBufQ);
    int metaQcount = img_q_count(&p_base->metaQ);
    img_frame_t *p_frame = NULL;
    img_meta_t  *p_meta  = NULL;
    int i = 0;

  if ((inputQcnt >= p_base->caps.num_input) &&
    (outputQcnt >= p_base->caps.num_output) &&
    (metaQcount >= p_base->caps.num_meta)) {
    /* input */
    for (i = 0; i < inputQcnt; i++) {
      p_frame = img_q_dequeue(&p_base->inputQ);
      if (!p_frame) {
        IDBG_ERROR("Cannot dequeue in frame");
      }
      IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_BUF_DONE,
        p_frame, p_frame);
    }
    p_frame = NULL;

    /* output */
    for (i = 0; i < outputQcnt; i++) {
      p_frame = img_q_dequeue(&p_base->outputQ);
      if (!p_frame) {
        IDBG_ERROR("Cannot dequeue in frame");
      }
      IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_OUT_BUF_DONE,
        p_frame, p_frame);
    }
    p_frame = NULL;

    /* meta */
    for (i = 0; i < metaQcount; i++) {
      p_meta = img_q_dequeue(&p_base->metaQ);
      if (!p_meta) {
        IDBG_ERROR("Cannot dequeue in frame");
      }
      IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_META_BUF_DONE,
        p_meta, p_meta);
    }
    p_meta = NULL;
  }
  return IMG_SUCCESS;
}

/**
 * Function: multi_frameproc_mgr_update_active_bundle
 *
 * Description: Function to update active bundle
 *
 * Input parameters:
 *   @p_mgr: The pointer to the manager object
 *   @p_comp: Pointer to the component
 *
 * Return values:
 *     None
 *
 * Notes: none
 **/
void multi_frameproc_mgr_update_active_bundle(multi_frameproc_mgr_t *p_mgr,
  multi_frameproc_comp_t *p_comp,
  img_frame_bundle_t *p_bundle)
{
  img_active_bundle_t *p_ab;
  uint32_t abl_index;

  IDBG_MED("E");
  if (p_comp->abl.count >= MFC_ACTIVE_BUNDLE_CNT) {
    abl_index = p_comp->abl.min_fidx;
    /* replace oldest frame id */
    IDBG_MED("drop oldest and abl cnt is %d", p_comp->abl.count);
    multi_frameproc_comp_send_buffers(p_comp,
      &p_comp->abl.active_bundle[abl_index].bundle,
      IMG_ERR_FRAME_DROP, true);
    p_comp->abl.active_bundle[abl_index].frame_id = MFC_INVALID_FRAMEID;
    p_comp->abl.active_bundle[abl_index].valid = false;
    p_comp->abl.count--;
    IDBG_MED("abl cnt dec to %d", p_comp->abl.count);
    p_comp->abl.min_fidx = (p_comp->abl.min_fidx + 1) % MFC_ACTIVE_BUNDLE_CNT;
  } else if (!p_comp->abl.count) {
    p_comp->abl.min_fidx = 0;
    abl_index = 0;
  } else {
    abl_index = (p_comp->abl.min_fidx + p_comp->abl.count) %
      MFC_ACTIVE_BUNDLE_CNT;
  }
  p_ab = &p_comp->abl.active_bundle[abl_index];
  p_ab->bundle = *p_bundle;
  p_ab->frame_id = p_bundle->p_input[0]->frame_id;
  p_ab->valid = true;
  p_comp->abl.count++;
  IDBG_MED("abl cnt inc to %d", p_comp->abl.count);
}


/**
 * Function: multi_frameproc_comp_drop_queue_traverse
 *
 * Description: Drop queue traverse function
 *
 * Input parameters:
 *   @p_nodedata: Each node data from queue
 *   @p_userdata: User data
 *
 * Return values:
 *     true/false
 *
 **/
int multi_frameproc_comp_drop_queue_traverse(void *p_nodedata, void *p_userdata)
{
  multiframeproc_drop_t* slave_drop = p_nodedata;
  uint32_t *current_frame_id = p_userdata;

  if (!p_nodedata || !p_userdata) {
    IDBG_ERROR("NULL pointer %p %p", p_nodedata, p_userdata);
    return 0;
  }

  if (slave_drop->frame_id <= *current_frame_id) {
    IDBG_MED("frame drop for frame_id %d : %d ",slave_drop->frame_id,*current_frame_id);
    return 1;
  }

  return 0;
}

/**
 * Function: multi_frameproc_mgr_check_pcr_mono
 *
 * Description: Function to check PCR for mono/wide
 *
 * Input parameters:
 *   @p_mgr: The pointer to the manager object
 *
 * Return values:
 *     None
 *
 * Notes: The function will drop the mono/wide frame
 *   if the bayer/tele PCR is greater than mono/wide.
 *   Because Mono/Wide processing is not required.
 *
 **/

void multi_frameproc_mgr_check_pcr_mono(multi_frameproc_mgr_t *p_mgr) {
  multi_frameproc_comp_t *p_master_comp = NULL;
  multi_frameproc_comp_t *p_slave_comp = NULL;
  img_active_bundle_t *p_master_ab = NULL;
  img_active_bundle_t *p_slave_ab = NULL;
  multiframeproc_drop_t *p_drop_data = NULL;
  uint32_t abl_index;
  uint32_t i, count = 0;
  uint32_t master_frame_id = 0;


  /* send event */
  for (i = 0; i < MFC_MAX_CAMERA_SESSIONS; i++) {
    if (NULL == p_mgr->p_comp[i])
      continue;

    if (p_mgr->p_comp[i]->is_master)
      p_master_comp = p_mgr->p_comp[i];
    else
      p_slave_comp = p_mgr->p_comp[i];
  }

  if (p_master_comp == NULL || p_slave_comp == NULL)
    return;

  p_master_ab = &p_master_comp->abl.active_bundle[p_master_comp->abl.min_fidx];
  p_slave_ab = &p_slave_comp->abl.active_bundle[p_slave_comp->abl.min_fidx];

  master_frame_id = p_master_ab->frame_id;

  if (p_master_ab->valid == 0 || p_slave_ab->valid == 0 ||
    p_master_ab->frame_id ==  MFC_INVALID_FRAMEID)
    return;

  IDBG_MED("master frame id %d:%d valid %d slave frame_id %d valid %d",
    p_master_ab->frame_id,master_frame_id,p_master_ab->valid,
    p_slave_ab->frame_id,p_slave_ab->valid);

  if(p_master_ab->frame_id > p_slave_ab->frame_id) {
    abl_index = p_slave_comp->abl.min_fidx;
    multi_frameproc_comp_send_buffers(p_slave_comp,
      &p_slave_comp->abl.active_bundle[abl_index].bundle,
      IMG_ERR_FRAME_DROP, true);
    p_slave_comp->abl.active_bundle[abl_index].frame_id = MFC_INVALID_FRAMEID;
    p_slave_comp->abl.active_bundle[abl_index].valid = false;
    p_slave_comp->abl.count--;
    p_slave_comp->abl.min_fidx =
      (p_slave_comp->abl.min_fidx + 1) % MFC_ACTIVE_BUNDLE_CNT;
  }

  count = img_q_count(&p_mgr->dropQ);
  while(count) {
    p_drop_data  = (multiframeproc_drop_t *)img_q_traverse_dequeue(&p_mgr->dropQ,
      multi_frameproc_comp_drop_queue_traverse,
      &master_frame_id);
    if (p_drop_data != NULL) {
      if (p_drop_data->frame_id == master_frame_id) {
        multi_frameproc_comp_remove_buffer(p_mgr, IMG_ERR_FRAME_DROP, true, p_drop_data->frame_id);
      }
      free(p_drop_data);
      p_drop_data = NULL;
    }
    count--;
  }
  return;
}


/**
 * Function: multi_frameproc_mgr_create_super_bundle
 *
 * Description: Function to create super bundle
 *
 * Input parameters:
 *   @p_mgr: The pointer to the manager object
 *   @pp_act_bun_array: output parameter for bundle array list
 *   @pp_comp_array: output parameter for component array list
 *   @p_super_bundle: output parameter for super bundle pointer
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
int multi_frameproc_mgr_create_super_bundle(multi_frameproc_mgr_t *p_mgr,
  img_active_bundle_t **pp_act_bun_array,
  multi_frameproc_comp_t **pp_comp_array,
  img_frame_bundle_t *p_super_bundle)
{
  uint32_t i;
  multi_frameproc_comp_t *p_comp;
  multi_frameproc_comp_t *p_comp_fir = NULL;
  uint32_t b_cnt = 0;
  bool can_create_sb = false;
  img_active_bundle_t *p_ab;
  img_active_bundle_t *p_ab_fir;
  img_active_bundle_t *p_ab_master = NULL;

  multi_frameproc_mgr_check_pcr_mono(p_mgr);

  IDBG_MED("E");
  /* find min frame ID */
  for (i = 0; i < MFC_MAX_CAMERA_SESSIONS; i++) {
    p_comp = p_mgr->p_comp[i];

    if ((NULL == p_comp) || (!p_comp->abl.count)) {
      if(NULL != p_comp && !p_comp->abl.count) {
        IDBG_MED("abl count is 0 %d", i);
      }
      continue;
    }

    p_ab = &p_comp->abl.active_bundle[p_comp->abl.min_fidx];
    IDBG_HIGH("is_master %d abl.min_fidx %d frameid %d",p_comp->is_master,
      p_comp->abl.min_fidx,p_ab->frame_id);

    if (p_comp->is_master) {
      p_ab_master = p_ab;
    }
    if (!p_comp_fir) {
      p_comp_fir = p_comp;
      p_ab_fir = &p_comp_fir->abl.active_bundle[p_comp->abl.min_fidx];
    } else {
      if (p_ab_fir->frame_id != p_ab->frame_id) {
        IDBG_MED("Frameid mismatch %d %d", p_ab_fir->frame_id, p_ab->frame_id);
        break; /* Todo: Control frame id matching based on is HW sync enabled*/
      }
    }
    pp_act_bun_array[b_cnt] = p_ab;
    pp_comp_array[b_cnt] = p_comp;
    b_cnt++;
  }

  IDBG_HIGH("computed cnt %d actual %d active cmp %d", b_cnt,
    p_mgr->comp_cnt, p_mgr->active_comp_cnt);
  can_create_sb = (b_cnt == p_mgr->active_comp_cnt);

  if (can_create_sb) {

    if (NULL == p_ab_master) {
      IDBG_ERROR("Cannot find master");
      return false;
    }

    IDBG_HIGH("process_frame_id %d",p_ab_master->frame_id);

    for (i = 0; i < b_cnt; i++) {
      memcpy(&p_super_bundle->p_input[i * pp_comp_array[0]->b.caps.num_input],
        &pp_act_bun_array[i]->bundle.p_input[0],
        sizeof(img_frame_t *) * pp_comp_array[0]->b.caps.num_input);
      memcpy(&p_super_bundle->p_meta[i * pp_comp_array[0]->b.caps.num_meta],
        &pp_act_bun_array[i]->bundle.p_meta[0],
        sizeof(img_meta_t *) * pp_comp_array[0]->b.caps.num_meta);
    }
    if (pp_comp_array[0]->b.caps.num_output) {
      memcpy(p_super_bundle->p_output, p_ab_master->bundle.p_output,
        sizeof(img_frame_t *) * pp_comp_array[0]->b.caps.num_output);
    }
  }
  IDBG_MED("X");
  return can_create_sb;
}

/**
 * Function: multi_frameproc_mgr_thread_loop
 *
 * Description: Main algorithm thread loop
 *
 * Input parameters:
 *   @handle: The pointer to the manager object
 *
 * Return values:
 *     NULL
 *
 * Notes: none
 **/
int multi_frameproc_mgr_thread_loop(void *handle)
{
  multi_frameproc_mgr_t *p_mgr = (multi_frameproc_mgr_t *)handle;
  int status = IMG_SUCCESS;
  img_msg_t *p_msg = NULL;
  img_frame_bundle_t super_bundle;
  multi_frameproc_comp_t *p_comp = NULL;
  multiframeproc_drop_t *p_drop_data = NULL;
  img_active_bundle_t *p_act_bun_array[MFC_MAX_CAMERA_SESSIONS] = {NULL};
  multi_frameproc_comp_t *p_comp_array[MFC_MAX_CAMERA_SESSIONS] = {NULL};
  uint32_t i;

  while (TRUE) {
    p_msg = img_q_wait(&p_mgr->msgQ, multi_frameproc_mgr_can_wait, p_mgr);

    if (!multi_frameproc_mgr_can_wait(p_mgr)) {
      IDBG_INFO("multiframe proc thread exiting");
      break;
    } else if (!p_msg)
      continue;

    switch (p_msg->type) {
    case IMG_MSG_BUNDLE:
      IDBG_MED("IMG_MSG_BUNDLE");

      /* update active bundle */
      p_comp = (multi_frameproc_comp_t *)p_msg->p_sender;
      if (p_comp)
        multi_frameproc_mgr_update_active_bundle(p_mgr, p_comp, &p_msg->bundle);

      /* create superbundle */
      if (multi_frameproc_mgr_create_super_bundle(p_mgr,
        &p_act_bun_array[0],
        &p_comp_array[0],
        &super_bundle)) {
        status = p_mgr->p_lib->img_algo_process(p_mgr->p_algocontext,
          super_bundle.p_input,
          p_mgr->caps.num_input,
          super_bundle.p_output,
          p_mgr->caps.num_output,
          super_bundle.p_meta,
          p_mgr->caps.num_meta);
        if (IMG_ERROR(status)) {
          IDBG_ERROR("IMGLIB_BASE algo error %d", status);
          if (IMG_ERR_CONNECTION_FAILED == status) {
            status = img_dsp_dl_mgr_set_reload_needed(TRUE);
          }
        }

        /* Now release the bundles */
        for (i = 0; i < p_mgr->active_comp_cnt; i++) {
          /* send event */
          if (p_comp_array[i] == NULL || p_act_bun_array[i] == NULL) {
            IDBG_ERROR("p_com_array = %p  p_act_bun_array = %p \
              not initialized yet",
              p_comp_array[i], p_act_bun_array[i]);
            break;
          }
          multi_frameproc_comp_send_buffers(p_comp_array[i],
            &p_act_bun_array[i]->bundle, status, true);
          p_act_bun_array[i]->valid = false;
          p_act_bun_array[i]->frame_id = MFC_INVALID_FRAMEID;
          p_comp_array[i]->abl.count--;
          IDBG_MED("abl cnt dec to %d", p_comp_array[i]->abl.count);
          p_comp_array[i]->abl.min_fidx = (p_comp_array[i]->abl.min_fidx + 1) %
            MFC_ACTIVE_BUNDLE_CNT;
        }
      }
      break;
    case IMG_MSG_FRAME:
      IDBG_MED("IMG_MSG_FRAME");
      if (p_mgr->p_lib->img_algo_frame_ind) {
        p_mgr->p_lib->img_algo_frame_ind(p_mgr->p_algocontext,
          p_msg->p_frame);
      }
      break;
    case IMG_MSG_META:
      IDBG_MED("IMG_MSG_META");
      if (p_mgr->p_lib->img_algo_meta_ind)
        p_mgr->p_lib->img_algo_meta_ind(p_mgr->p_algocontext,
          p_msg->p_meta);
      break;
    case IMG_MSG_WAIT_FOR_START: {
      IDBG_MED("IMG_MSG_WAIT_FOR_START");
      multi_frameproc_comp_t *p_comp =
        (multi_frameproc_comp_t *)p_msg->p_sender;

      pthread_mutex_lock(&p_comp->b.mutex);
      multi_frameproc_mgr_update_active_comp_count(p_mgr);
      multi_frameproc_mgr_update_active_io_count(p_mgr);
      pthread_mutex_unlock(&p_comp->b.mutex);
      pthread_cond_broadcast(&p_comp->b.cond);
      break;
    }
    case IMG_MSG_WAIT_FOR_ABORT: {
      IDBG_MED("IMG_MSG_WAIT_FOR_ABORT");
      multi_frameproc_comp_t *p_comp =
        (multi_frameproc_comp_t *)p_msg->p_sender;

      pthread_mutex_lock(&p_comp->b.mutex);
      multi_frameproc_comp_flush_buffers(p_comp, IMG_ERR_FRAME_DROP, true);
      multi_frameproc_mgr_update_active_comp_count(p_mgr);
      multi_frameproc_mgr_update_active_io_count(p_mgr);
      if (p_mgr->p_lib->img_algo_abort) {
        status = p_mgr->p_lib->img_algo_abort(p_mgr->p_algocontext);
      }
      pthread_mutex_unlock(&p_comp->b.mutex);
      pthread_cond_broadcast(&p_comp->b.cond);
      break;
    }
    case IMG_MSG_KILL: {
      IDBG_MED("IMG_MSG_KILL");
      break;
    }
    case IMG_MSG_UPDATE_MASTER: {
      IDBG_MED("IMG_MSG_UPDATE_MASTER");
      multi_frameproc_comp_t *p_comp =
        (multi_frameproc_comp_t *)p_msg->p_sender;

      pthread_mutex_lock(&p_comp->b.mutex);
      bool *p_is_master_flag = (bool *)p_msg->p_data;
      p_comp->is_master = *p_is_master_flag;
      pthread_mutex_unlock(&p_comp->b.mutex);
      pthread_cond_broadcast(&p_comp->b.cond);
      break;
    }
    default:;
      IDBG_MED("Unknown message");
    }
    if (p_msg)
      free(p_msg);
      p_msg = NULL;
  }

  /* flush rest of the buffers */
  while ((p_msg = img_q_dequeue(&p_mgr->msgQ)) != NULL) {
    switch (p_msg->type) {
    case IMG_MSG_BUNDLE:
      IDBG_MED("Flushing out buffers");
      multi_frameproc_comp_send_buffers(p_msg->p_sender, &p_msg->bundle,
        IMG_ERR_FRAME_DROP, false);
      break;
    default:;
    }
    free(p_msg);
    p_msg = NULL;
  }

  IDBG_LOW("Flush the pending queue buffers");
  while ((p_drop_data = img_q_dequeue(&p_mgr->dropQ)) != NULL) {
    free(p_drop_data);
    p_drop_data = NULL;
  }
  return 0;
}


/**
 * Function: multi_frameproc_comp_init
 *
 * Description: Initializes the multi_frameproc component
 *
 * Input parameters:
 *   @handle: The pointer to the component handle.
 *   @p_userdata: the handle which is passed by the client
 *   @p_data: The pointer to the parameter which is required
 *            during the init phase
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int multi_frameproc_comp_init(void *handle, void* p_userdata, void *p_data)
{
  multi_frameproc_comp_t *p_comp = (multi_frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;
  img_init_params_t *p_params = (img_init_params_t *)p_data;
  IDBG_MED(" ");

  if (!p_comp) {
    IDBG_ERROR("Error ");
    return IMG_ERR_INVALID_INPUT;
  }

  status = p_comp->b.ops.init(&p_comp->b, p_userdata, p_data);
  if (status < 0) {
    IDBG_ERROR("Error %d", status);
    return status;
  }

  if (p_params) {
    p_comp->init_params = *p_params;
  }
  return status;
}

#if 0
/**
 * Function: multi_frameproc_flush_buffers
 *
 * Description: Function to send the buffers to client
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
static void multi_frameproc_comp_flush_buffers(multi_frameproc_comp_t *p_comp,
  int status)
{
  img_component_t *p_base = &p_comp->b;
  img_frame_t *p_frame;
  img_meta_t *p_meta;

  while ((p_frame = img_q_dequeue(&p_base->inputQ)) != NULL) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_BUF_DONE,
      p_frame, p_frame);
  }

  while ((p_frame = img_q_dequeue(&p_base->outBufQ)) != NULL) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_OUT_BUF_DONE,
      p_frame, p_frame);
  }

  while ((p_meta = img_q_dequeue(&p_base->metaQ)) != NULL) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_BUF_DONE,
      p_meta, p_meta);
  }
  if (IMG_ERROR(status)) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_ERROR, status, status);
  }
}
#endif
/**
 * Function: multi_frameproc_send_buffers
 *
 * Description: Function to send the buffers to client
 *
 * Input parameters:
 *   p_comp - The pointer to the component object
 *   p_bundle - frame bundle
 *   status - status of caller
 *   @send_status: flag to indicate whether the status needs to
 *               be sent
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void multi_frameproc_comp_send_buffers(multi_frameproc_comp_t *p_comp,
  img_frame_bundle_t *p_bundle,
  int status,
  bool send_status)
{
  img_component_t *p_base = &p_comp->b;
  int i;
  int output_buf_count = 0;

  if(p_comp->is_master) {
    output_buf_count = p_base->caps.num_master_output;
  } else {
    output_buf_count = p_base->caps.num_slave_output;
  }

  IDBG_MED("E");
  if (send_status && IMG_ERROR(status)) {
    if (status == IMG_ERR_FRAME_DROP) {
      int frame_id = 0;
      if (p_base->caps.num_input)
        frame_id = p_bundle->p_input[0]->frame_id;
      IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_FRAME_DROP, frameid, frame_id);
    } else {
      IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_ERROR, status, status);
    }
  }

  for (i = 0; i < p_base->caps.num_input; i++) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_BUF_DONE,
      p_frame, p_bundle->p_input[i]);
  }

  for (i = 0; i < p_base->caps.num_meta; i++) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_META_BUF_DONE,
      p_meta, p_bundle->p_meta[i]);
  }

  for (i = 0; i < output_buf_count; i++) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_IMG_OUT_BUF_DONE,
      p_frame, p_bundle->p_output[i]);
  }


  if (send_status && IMG_SUCCEEDED(status)) {
    IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_DONE, status, status);
  }
  IDBG_MED("X");
}

/**
 * Function: multi_frameproc_flush_buffers
 *
 * Description: Function to flush buffers
 *
 * Input parameters:
 *   p_comp - The pointer to the component object
 *   status - status of caller
 *   send_status - flag to send status
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void multi_frameproc_comp_flush_buffers(multi_frameproc_comp_t *p_comp,
  int status, bool send_status)
{
  uint32_t i = p_comp->abl.min_fidx;
  while (p_comp->abl.count) {
    multi_frameproc_comp_send_buffers(p_comp,
      &p_comp->abl.active_bundle[i].bundle,
      status, send_status);
    p_comp->abl.active_bundle[i].frame_id = MFC_INVALID_FRAMEID;
    p_comp->abl.active_bundle[i].valid = false;
    p_comp->abl.count--;
    IDBG_MED("abl cnt dec to %d", p_comp->abl.count);
    p_comp->abl.min_fidx = (p_comp->abl.min_fidx + 1) % MFC_ACTIVE_BUNDLE_CNT;
    i = p_comp->abl.min_fidx;
  }
 }

/**
 * Function: multi_frameproc_comp_remove_buffer
 *
 * Description: Function to remove drop(unwanted) buffers
 *
 * Input parameters:
 *   p_mgr - The pointer to the manager object
 *   status - status of caller
 *   send_status - flag to send status
 *   frame_id - frame id
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void multi_frameproc_comp_remove_buffer(multi_frameproc_mgr_t *p_mgr,
  int status, bool send_status, uint32_t frame_id)
{
  uint32_t i;
  multi_frameproc_comp_t *p_comp;
  for (i =0; i < p_mgr->active_comp_cnt; i++) {
    p_comp = p_mgr->p_comp[i];
    if (p_comp->abl.active_bundle[p_comp->abl.min_fidx].frame_id == frame_id) {
      multi_frameproc_comp_send_buffers(p_comp,
        &p_comp->abl.active_bundle[p_comp->abl.min_fidx].bundle,
        status, send_status);
      p_comp->abl.active_bundle[p_comp->abl.min_fidx].frame_id = MFC_INVALID_FRAMEID;
      p_comp->abl.active_bundle[p_comp->abl.min_fidx].valid = false;
      p_comp->abl.count--;
      p_comp->abl.min_fidx = (p_comp->abl.min_fidx + p_comp->abl.count) % MFC_ACTIVE_BUNDLE_CNT;
      IDBG_LOW("abl cnt dec to %d min_fidx %d", p_comp->abl.count, p_comp->abl.min_fidx);
    }
  }
}

/**
 * Function: multi_frameproc_comp_update_is_master
 *
 * Description: Sends pending frames to process and updates the
 *   is_master flag.
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
int multi_frameproc_comp_update_is_master(void *handle, void *p_data)
{
  int32_t rc = IMG_SUCCESS;
  bool *p_master_flag;
  multi_frameproc_comp_t *p_comp = (multi_frameproc_comp_t *)handle;
  if (!p_comp) {
    IDBG_ERROR("Error NULL component");
    return IMG_ERR_INVALID_INPUT;
  }

  img_component_t *p_base = &p_comp->b;
  p_master_flag = (bool *)p_data;
  IDBG_MED("update is master to %d", *p_master_flag);

  if (p_comp->p_mgr == NULL) {
    p_comp->is_master = *p_master_flag;
    return IMG_SUCCESS;
  }

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT)
    && (p_base->state != IMG_STATE_STARTED)) {
    IDBG_ERROR("Error state %d", p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  /* if there are buffers queued, queue them to be processed */
  while((img_q_count(&p_base->inputQ) >= p_base->caps.num_input)
    && (img_q_count(&p_base->outBufQ) >= p_base->caps.num_output)
    && (img_q_count(&p_base->metaQ) >= p_base->caps.num_meta)) {
    multi_frameproc_comp_check_create_bundle(p_comp);
  }

  if (p_comp->p_mgr) {
    rc = multi_frameproc_mgr_send_message(p_comp, p_comp->p_mgr,
      IMG_MSG_UPDATE_MASTER, NULL, NULL, p_data);
    if (IMG_ERROR(rc)) {
       IDBG_ERROR("Send message failed %d", rc);
       goto end;
    }
  }
  /* wait for state change */
  rc = img_wait_for_completion(&p_base->cond, &p_base->mutex, 1000);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("update is master to %d failed %d", *p_master_flag, rc);
    goto end;
  }

end:
  pthread_mutex_unlock(&p_base->mutex);
  IDBG_MED("rc %d X", rc);
  return rc;
}

/**
 * Function: multi_frameproc_comp_get_param
 *
 * Description: Gets multi_frameproc parameters
 *
 * Input parameters:
 *   @handle: The pointer to the component handle.
 *   @param: The type of the parameter
 *   @p_data: The pointer to the paramter structure. The
 *   structure
 *            for each paramter type will be defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int multi_frameproc_comp_get_param(void *handle, img_param_type param, void *p_data)
{
  multi_frameproc_comp_t *p_comp = (multi_frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, status);
    return IMG_ERR_INVALID_INPUT;
  }
  switch (param) {
  case QIMG_PARAM_IS_MASTER: {
    bool *is_master = (bool *)p_data;
    if (!is_master) {
      IDBG_ERROR("Invalid Input to get param QIMG_PARAM_IS_MASTER");
      return IMG_ERR_INVALID_INPUT;
    }
    is_master = &p_comp->is_master;
  }
  break;
  case QIMG_PARAM_SRC_DIM: {
    multi_frameproc_lib_info_t *p_mfp_lib =
      (multi_frameproc_lib_info_t *)p_comp->p_lib;
    if (p_mfp_lib->img_algo_get_lib_config) {
      status = p_mfp_lib->img_algo_get_lib_config(NULL, p_data);
      if (IMG_ERROR(status)) {
        IDBG_ERROR("get algo config failed %p", p_data);
      }
    }
  }
  break;
  case QIMG_PARAM_ALGO_CAPS: {
    multi_frameproc_lib_info_t *p_mfp_lib =
      (multi_frameproc_lib_info_t *)p_comp->p_lib;
    if (p_mfp_lib->img_algo_get_caps) {
      status = p_mfp_lib->img_algo_get_caps(NULL, p_data);
      if (IMG_ERROR(status)) {
        IDBG_ERROR("get algo caps failed %p", p_data);
      }
    }
  }
  break;
  default:
   status = p_comp->b.ops.get_parm(&p_comp->b, param, p_data);
  }
  return status;
}

/**
 * Function: multi_frameproc_comp_set_param
 *
 * Description: Set multi_frameproc parameters
 *
 * Input parameters:
 *   @handle: The pointer to the component handle.
 *   @param - The type of the parameter
 *   @p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in
 *            multi_frameproc_comp.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int multi_frameproc_comp_set_param(void *handle, img_param_type param,
  void *p_data)
{
  multi_frameproc_comp_t *p_comp = (multi_frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  switch (param) {
  case QIMG_PARAM_IS_MASTER: {
    if (NULL == p_data) {
      IDBG_ERROR("Invaid Multiframeproc params");
      return IMG_ERR_INVALID_INPUT;
    }
    status = multi_frameproc_comp_update_is_master((void *)p_comp, p_data);
    if(IMG_ERROR(status)) {
      IMG_ERROR("Failed to set master");
    }
  }
  break;
  case QIMG_PARAM_FRAME_DROP: {
    if (NULL == p_data) {
      IDBG_ERROR("Invaid Multiframeproc params");
      return IMG_ERR_INVALID_INPUT;
    }
    status = multi_frameproc_comp_frame_drop((void *)p_comp, p_data);
    if(IMG_ERROR(status)) {
      IMG_ERROR("Failed to set master");
    }
  }
  break;
  case QIMG_PARAM_INIT_UPDATE: {
    if (NULL == p_data) {
      IDBG_ERROR("Invaid Multiframeproc init params");
      return IMG_ERR_INVALID_INPUT;
    }
    img_init_params_t *p_params = (img_init_params_t *)p_data;
    p_comp->init_params = *p_params;
  }
  break;
  default:
    status = p_comp->b.ops.set_parm(&p_comp->b, param, p_data);
  }
  IDBG_LOW("%s:%d] param 0x%x", __func__, __LINE__, param);
  return status;
}

/**
 * Function: multi_frameproc_comp_deinit
 *
 * Description: Un-initializes the multi_frameproc component
 *
 * Input parameters:
 *   @handle: The pointer to the component handle.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int multi_frameproc_comp_deinit(void *handle)
{
  multi_frameproc_comp_t *p_comp = (multi_frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;
  uint32_t i;
  multi_frameproc_mgr_t *p_mgr;

  IDBG_MED("");

  if (!p_comp) {
    IDBG_ERROR("Error %d", status);
    return IMG_ERR_INVALID_INPUT;
  }

  p_mgr = p_comp->p_mgr;
  if (p_mgr) {
    IDBG_ERROR("Error unbind missing %p cleanup", p_mgr);
    for (i = 0; i < IMGLIB_ARRAY_SIZE(p_mgr->p_comp); i++) {
      if (p_mgr->p_comp[i] == p_comp) {
        p_mgr->p_comp[i] = NULL;
        p_mgr->comp_cnt--;
      }
    }
  }

  status = p_comp->b.ops.abort(handle, NULL);
  if (IMG_ERROR(status)) {
    IDBG_MED("Error status %d", status);
    return status;
  }

  status = p_comp->b.ops.deinit(&p_comp->b);
  if (IMG_ERROR(status)) {
    IDBG_MED("Error status %d", status);
    return status;
  }

  free(p_comp);

  return status;
}

/**
 * Function: multi_frameproc_comp_query_outputbuf
 *
 * Description: This function is used to capture output
 *            buffer assigned to master and slave.
 *
 * Input parameters:
 *   type - image type.
 *   p_comp - The pointer to the component handle.
 *
 * Return values:
 *    true/false
 *
 * Notes: none
 **/

static int multi_frameproc_comp_query_outputbuf
  (img_type_t type, multi_frameproc_comp_t *p_comp) {

  int8_t num_output;

  if (type != IMG_OUT) {
    return TRUE;
  }

  num_output = p_comp->is_master ? p_comp->b.caps.num_master_output
   : p_comp->b.caps.num_slave_output;

  return num_output;
}


/**
 * Function: multi_frameproc_comp_check_create_bundle
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
int multi_frameproc_comp_check_create_bundle(multi_frameproc_comp_t *p_comp)
{
  int8_t create_bundle = FALSE;
  int status = TRUE;
  img_component_t *p_base = &p_comp->b;
  int inputQcnt = img_q_count(&p_base->inputQ);
  int outputQcnt = img_q_count(&p_base->outBufQ);
  int metaQcount = img_q_count(&p_base->metaQ);
  img_msg_t *p_msg = NULL;
  int i = 0;
  int output_buf_count = 0;

  if (!p_comp->p_mgr) {
    IDBG_ERROR("comp manager NULL");
    return IMG_ERR_INVALID_OPERATION;
  }

  if(p_comp->is_master) {
    output_buf_count = p_base->caps.num_master_output;
  } else {
    output_buf_count = p_base->caps.num_slave_output;
  }

  if ((inputQcnt >= p_base->caps.num_input) &&
    (outputQcnt >= output_buf_count) &&
    (metaQcount >= p_base->caps.num_meta)) {
    p_msg = calloc(1, sizeof(img_msg_t));
    if (!p_msg) {
      IDBG_ERROR("Cannot create bundle");
      return IMG_ERR_NO_MEMORY;
    }
    p_msg->type = IMG_MSG_BUNDLE;

    /* input */
    for (i = 0; i < p_base->caps.num_input; i++) {
      p_msg->bundle.p_input[i] = img_q_dequeue(&p_base->inputQ);
      if (!p_msg->bundle.p_input[i]) {
        IDBG_ERROR("Cannot dequeue in frame");
        goto error;
      }
    }
    /* output */
    for (i = 0; i < output_buf_count; i++) {
      p_msg->bundle.p_output[i] = img_q_dequeue(&p_base->outBufQ);
      if (!p_msg->bundle.p_output[i]) {
        IDBG_ERROR(" Cannot dequeue out frame");
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

    p_msg->p_sender = p_comp;
    status = img_q_enqueue(&p_comp->p_mgr->msgQ, p_msg);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("%s:%d] Cannot enqueue bundle", __func__, __LINE__);
      return status;
    }
    create_bundle = TRUE;
  }
  IDBG_MED("p_comp %p (%d %d) (%d %d) (%d %d) flag %d",
    p_comp,
    inputQcnt,
    p_base->caps.num_input,
    outputQcnt,
    p_base->caps.num_output,
    metaQcount,
    p_base->caps.num_meta,
    create_bundle);

  /* signal the component */
  if (create_bundle) {
    img_q_signal(&p_comp->p_mgr->msgQ);
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
    p_msg = NULL;
  }
  return IMG_ERR_GENERAL;
}

/**
 * Function: multi_frameproc_comp_queue_buffer
 *
 * Description: This function is used to handle buffers from the
 *             client
 *
 * Input parameters:
 *   @handle: The pointer to the component handle.
 *   @p_frame: The frame buffer which needs to be processed by
 *             the imaging library
 *   @type: image type (main image or thumbnail image)
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int multi_frameproc_comp_queue_buffer(void *handle, img_frame_t *p_frame,
  img_type_t type)
{
  multi_frameproc_comp_t *p_comp = (multi_frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;
  int is_enqueue = TRUE;

  if (!p_comp) {
    IDBG_ERROR("Error NULL component");
    return IMG_ERR_INVALID_INPUT;
  }

  if (!p_comp->p_mgr) {
    IDBG_ERROR("comp manager NULL");
    return IMG_ERR_INVALID_OPERATION;
  }

  img_component_t *p_base = &p_comp->b;
  img_queue_t *queue = (type == IMG_OUT) ? &p_base->outBufQ : &p_base->inputQ;

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT)
    && (p_base->state != IMG_STATE_STARTED)) {
    IDBG_ERROR("Error state %d", p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  is_enqueue = multi_frameproc_comp_query_outputbuf(type,p_comp);

  if(is_enqueue) {
    status = img_q_enqueue(queue, p_frame);
    if (status < 0) {
      IDBG_ERROR("Error enqueue %d", status);
      pthread_mutex_unlock(&p_base->mutex);
      return status;
    }
  }

  if (type & IMG_IN) {
    status = multi_frameproc_mgr_send_message(p_comp, p_comp->p_mgr,
      IMG_MSG_FRAME, p_frame, NULL, NULL);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("Error send message %d", status);
      pthread_mutex_unlock(&p_base->mutex);
      return status;
    }
  }

  IDBG_MED("q_count %d", img_q_count(queue));
  status = multi_frameproc_comp_check_create_bundle(p_comp);

  pthread_mutex_unlock(&p_base->mutex);
  return status;
}

/**
 * Function: multi_frameproc_comp_queue_buffer
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
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int multi_frameproc_comp_queue_metabuffer(void *handle, img_meta_t *p_metabuffer)
{
  multi_frameproc_comp_t *p_comp = (multi_frameproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, status);
    return IMG_ERR_INVALID_INPUT;
  }

  if (!p_comp->p_mgr) {
    IDBG_ERROR("comp manager NULL");
    return IMG_ERR_INVALID_OPERATION;
  }

  img_component_t *p_base = &p_comp->b;
  img_queue_t *queue = &p_base->metaQ;

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT)
    && (p_base->state != IMG_STATE_STARTED)) {
    IDBG_ERROR("Error %d", p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  status = img_q_enqueue(queue, p_metabuffer);
  if (status < 0) {
    IDBG_ERROR("%s:%d] Error enqueue", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return status;
  }

  status = multi_frameproc_mgr_send_message(p_comp, p_comp->p_mgr,
    IMG_MSG_META, NULL, p_metabuffer, NULL);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("Error send message %d", status);
    pthread_mutex_unlock(&p_base->mutex);
    return status;
  }

  IDBG_MED("q_count %d", img_q_count(queue));
  multi_frameproc_comp_check_create_bundle(p_comp);

  pthread_mutex_unlock(&p_base->mutex);
  return IMG_SUCCESS;
}

/**
 * Function: multi_frameproc_comp_deque_buf_by_fid
 *
 * Description: This function is used to dequeue buffers based
 *             on frame id
 *
 * Input parameters:
 *   @handle: The pointer to the component handle.
 *   @pp_buf: The buffer that is dequeued
 *   @fid: frame id
 *   @type: image type (main image or thumbnail image)
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int multi_frameproc_comp_deque_buf_by_fid(void *handle, void **pp_buf,
  uint32_t fid, img_type_t type) {
  multi_frameproc_comp_t *p_comp = (multi_frameproc_comp_t *)handle;
  img_queue_t *queue = NULL;
  img_component_t *p_base = NULL;
  img_buffer_to_find_t buffer_to_find;
  int status = IMG_SUCCESS;
  int is_dequeue = TRUE;

  if (!p_comp || !pp_buf) {
    IDBG_ERROR("Error NULL %d %d", p_comp, pp_buf);
    return IMG_ERR_INVALID_INPUT;
  }

  if (!p_comp->p_mgr) {
    IDBG_ERROR("comp manager NULL");
    return IMG_ERR_INVALID_OPERATION;
  }

  is_dequeue = multi_frameproc_comp_query_outputbuf(type,p_comp);

  if(!is_dequeue) {
    return IMG_SUCCESS;
  }

  p_base = &p_comp->b;
  buffer_to_find.frame_id = fid;
  buffer_to_find.buff_type = type;

  switch (type) {
  case IMG_IN:
    queue = &p_base->inputQ;
    break;
  case IMG_OUT:
    queue = &p_base->outBufQ;
    break;
  case IMG_META:
    queue = &p_base->metaQ;
    break;
  default:
    IDBG_ERROR("Invalid type");
    return IMG_ERR_INVALID_INPUT;
  }

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT)
    && (p_base->state != IMG_STATE_STARTED)) {
    IDBG_ERROR("Error state %d", p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  /* Travers through the queue and dequeue buffer */
  *pp_buf = img_q_traverse_dequeue(queue, img_comp_find_buffer_by_fid,
    &buffer_to_find);
  if (*pp_buf == NULL) {
    IDBG_ERROR("cannot find %d buffer by frame id %d", type, fid);
  }

  pthread_mutex_unlock(&p_base->mutex);
  return status;
}

/**
 * Function: multi_frameproc_comp_frame_drop
 *
 * Description: To handle the master frame drop due to frame
 *    drop of slave frame from ISP/CPP
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
int multi_frameproc_comp_frame_drop(void *handle, void *p_data)
{
  int32_t rc = IMG_SUCCESS;
  multi_frameproc_comp_t *p_comp = (multi_frameproc_comp_t *)handle;
  multiframeproc_drop_t *p_drop_data = NULL;
  uint32_t *frameid = p_data;
  if (!p_comp) {
    IDBG_ERROR("Error NULL component");
    return IMG_ERR_INVALID_INPUT;
  }

  img_component_t *p_base = &p_comp->b;

  pthread_mutex_lock(&p_base->mutex);
  if (IMG_STATE_INIT == p_base->state) {
    IDBG_ERROR("already stopped");
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_SUCCESS;
  }

  if (p_comp->p_mgr) {
    p_drop_data = calloc(1, sizeof(multiframeproc_drop_t));
    if (p_drop_data == NULL) {
      pthread_mutex_unlock(&p_base->mutex);
      return rc;
    }
    p_drop_data->frame_id = *frameid;
    IDBG_MED("frame drop %d",p_drop_data->frame_id)
    rc = img_q_enqueue(&p_comp->p_mgr->dropQ, p_drop_data);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] Cannot enqueue bundle", __func__, __LINE__);
    }
    rc = multi_frameproc_mgr_send_message(NULL, p_comp->p_mgr,
      IMG_MSG_BUNDLE,  NULL, NULL, NULL);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Send message failed %d", rc);
    }
  }

  pthread_mutex_unlock(&p_base->mutex);
  IDBG_MED("rc %d X", rc);
  return rc;
}


/**
 * Function: multi_frameproc_comp_abort
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
int multi_frameproc_comp_abort(void *handle, void *p_data)
{
  int32_t rc = IMG_SUCCESS;
  int status;
  IMG_UNUSED(p_data);

  multi_frameproc_comp_t *p_comp = (multi_frameproc_comp_t *)handle;

  if (!p_comp) {
    IDBG_ERROR("Error NULL component");
    return IMG_ERR_INVALID_INPUT;
  }

  img_component_t *p_base = &p_comp->b;

  pthread_mutex_lock(&p_base->mutex);
  if (IMG_STATE_INIT == p_base->state) {
    IDBG_ERROR("already stopped");
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_SUCCESS;
  }
  IDBG_INFO("State %d", p_base->state);
  p_base->state = IMG_STATE_STOP_REQUESTED;

  /* if there are buffers queued, queue them to be processed */
  status = multi_frameproc_clear_comp_buf_queue(p_comp);
  if (!IMG_SUCCEEDED(status)) {
    IDBG_ERROR("Tried clearing buffers, no luck !! status %d", status);
  }

  if (p_comp->p_mgr) {
    rc = multi_frameproc_mgr_send_message(p_comp, p_comp->p_mgr,
      IMG_MSG_WAIT_FOR_ABORT,
      NULL, NULL, NULL);
    if (IMG_ERROR(rc)) {
       IDBG_ERROR("Send message failed %d", rc);
       goto end;
    }

    /* wait for state change */
    rc = img_wait_for_completion(&p_base->cond, &p_base->mutex, 1000);
    if (IMG_ERROR(rc)) {
       IDBG_ERROR("State transition failed %d", rc);
       goto end;
    }

    if (p_comp->p_mgr->active_comp_cnt) {
      rc = multi_frameproc_mgr_send_message(NULL, p_comp->p_mgr,
        IMG_MSG_BUNDLE, NULL, NULL, NULL);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("Send message failed %d", rc);
        goto end;
      }
    }
  }

  p_base->state = IMG_STATE_INIT;
end:
  pthread_mutex_unlock(&p_base->mutex);
  IDBG_MED("rc %d X", rc);
  return rc;
}

/**
 * Function: multi_frameproc_comp_start
 *
 * Description: Start the execution of multi_frameproc
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
int multi_frameproc_comp_start(void *handle, void *p_data)
{
  multi_frameproc_comp_t *p_comp = (multi_frameproc_comp_t *)handle;

  if (!p_comp) {
    IDBG_ERROR("Error comp NULL");
    return IMG_ERR_INVALID_INPUT;
  }

  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;

  pthread_mutex_lock(&p_base->mutex);
  if (p_base->state != IMG_STATE_INIT) {
    IDBG_ERROR("Error state %d", p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_NOT_SUPPORTED;
  }

  if ((p_base->caps.num_input <= 0) ||
    (p_base->caps.num_output < 0) ||
    (p_base->caps.num_meta < 0) ||
    (p_base->caps.num_input > IMG_MAX_INPUT_FRAME) ||
    (p_base->caps.num_output > IMG_MAX_OUTPUT_FRAME) ||
    (p_base->caps.num_meta > IMG_MAX_META_FRAME)) {
    IDBG_ERROR(" Error caps not set");
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  /* dynamic caps not supported */
  if (NULL != p_data) {
    IDBG_ERROR("Error dynamic caps state %d", p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_NOT_SUPPORTED;
  }

  /* flush the queues */
  img_q_flush(&p_base->inputQ);
  img_q_flush(&p_base->outBufQ);
  img_q_flush(&p_base->metaQ);

  p_base->state = IMG_STATE_START_PENDING;
  if (p_comp->p_mgr) {
    status = multi_frameproc_mgr_send_message(NULL, p_comp->p_mgr,
      IMG_MSG_BUNDLE, NULL, NULL, NULL);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("Send message failed %d", status);
      p_base->state = IMG_STATE_INIT;
      goto end;
    }

    status = multi_frameproc_mgr_send_message(p_comp, p_comp->p_mgr,
      IMG_MSG_WAIT_FOR_START,
      NULL, NULL, NULL);
    if (IMG_ERROR(status)) {
       IDBG_ERROR("Send message failed %p %d", p_comp, status);
       p_base->state = IMG_STATE_INIT;
       goto end;
    }

    /* wait for state change */
    status = img_wait_for_completion(&p_base->cond, &p_base->mutex, 1000);
    if (IMG_ERROR(status)) {
       IDBG_ERROR("State transition failed %d", status);
       p_base->state = IMG_STATE_START_PENDING;
       goto end;
    }

    status = multi_frameproc_mgr_send_message(NULL, p_comp->p_mgr,
      IMG_MSG_BUNDLE, NULL, NULL, NULL);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("Send message failed %d", status);
      p_base->state = IMG_STATE_INIT;
      goto end;
    }
  }

  p_base->state = IMG_STATE_STARTED;
  p_comp->last_frame_id = MFC_INVALID_FRAMEID;
  memset(&p_comp->abl, 0x0, sizeof(p_comp->abl));

end:
  pthread_mutex_unlock(&p_base->mutex);


  return status;
}

/**
 * Function: multi_frameproc_comp_create
 *
 * Description: This function is used to create multi_frameproc
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
int multi_frameproc_comp_create(void* handle, img_component_ops_t *p_ops)
{
  multi_frameproc_comp_t *p_comp = NULL;
  int status;
  multi_frameproc_lib_info_t *p_mfp_lib =
    (multi_frameproc_lib_info_t *)handle;

  if (!handle) {
    IDBG_ERROR("Error invalid handle");
    return IMG_ERR_INVALID_OPERATION;
  }

  if (NULL == p_mfp_lib->ptr) {
    IDBG_ERROR("library not loaded");
    return IMG_ERR_INVALID_OPERATION;
  }

  if (NULL == p_ops) {
    IDBG_ERROR("failed");
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_MED("calling img_dsp_ reload");
  img_dsp_dl_requestall_to_close_and_reopen();

  p_comp = (multi_frameproc_comp_t *)calloc(1, sizeof(multi_frameproc_comp_t));
  if (NULL == p_comp) {
    IDBG_ERROR("failed");
    return IMG_ERR_NO_MEMORY;
  }

  status = img_comp_create(&p_comp->b);
  if (status < 0) {
    free(p_comp);
    return status;
  }

  /* set the main thread */
  p_comp->b.thread_loop = NULL;
  p_comp->b.p_core = p_comp;
  p_comp->p_lib = p_mfp_lib;

  /* copy the ops table from the base component */
  *p_ops = p_comp->b.ops;
  p_ops->init             = multi_frameproc_comp_init;
  p_ops->deinit           = multi_frameproc_comp_deinit;
  p_ops->set_parm         = multi_frameproc_comp_set_param;
  p_ops->get_parm         = multi_frameproc_comp_get_param;
  p_ops->start            = multi_frameproc_comp_start;
  p_ops->queue_buffer     = multi_frameproc_comp_queue_buffer;
  p_ops->queue_metabuffer = multi_frameproc_comp_queue_metabuffer;
  p_ops->deque_buffer_by_fid = multi_frameproc_comp_deque_buf_by_fid;
  p_ops->abort            = multi_frameproc_comp_abort;

  p_ops->handle = (void *)p_comp;
  return IMG_SUCCESS;
}

/**
 * Function: multi_frameproc_comp_load_fn_ptrs
 *
 * Description: This function is used to load  multi_frameproc
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
static int multi_frameproc_comp_load_fn_ptrs(
   multi_frameproc_lib_info_t *p_mfp_lib)
{
  int rc = IMG_SUCCESS;

  if (!p_mfp_lib) {
    IDBG_ERROR("invalid input ");
    return IMG_ERR_INVALID_INPUT;
  }

  IMG_DLSYM_ERROR_RET(p_mfp_lib, ptr, img_algo_init,
    "img_algo_init", 1);
  IMG_DLSYM_ERROR_RET(p_mfp_lib, ptr, img_algo_deinit,
    "img_algo_deinit", 1);
  IMG_DLSYM_ERROR_RET(p_mfp_lib, ptr, img_algo_process,
    "img_algo_process", 1);
  IMG_DLSYM_ERROR_RET(p_mfp_lib, ptr, img_algo_frame_ind,
    "img_algo_frame_ind", 0);
  IMG_DLSYM_ERROR_RET(p_mfp_lib, ptr, img_algo_meta_ind,
    "img_algo_meta_ind", 0);
  IMG_DLSYM_ERROR_RET(p_mfp_lib, ptr, img_algo_set_frame_ops,
    "img_algo_set_frame_ops", 0);
  IMG_DLSYM_ERROR_RET(p_mfp_lib, ptr, img_algo_preload,
    "img_algo_preload", 0);
  IMG_DLSYM_ERROR_RET(p_mfp_lib, ptr, img_algo_shutdown,
    "img_algo_shutdown", 0);
  IMG_DLSYM_ERROR_RET(p_mfp_lib, ptr, img_algo_get_caps,
    "img_algo_get_caps", 0);
  IMG_DLSYM_ERROR_RET(p_mfp_lib, ptr, img_algo_set_lib_config,
    "img_algo_set_lib_config", 0);
  IMG_DLSYM_ERROR_RET(p_mfp_lib, ptr, img_algo_get_lib_config,
    "img_algo_get_lib_config", 0);
  IMG_DLSYM_ERROR_RET(p_mfp_lib, ptr, img_algo_abort,
    "img_algo_abort", 0);
  return rc;
}

/**
 * Function: multi_frameproc_core_unload_lib
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
static int multi_frameproc_core_unload_lib(void *handle, void *p_userdata)
{
  int rc = 0;
  multi_frameproc_lib_info_t *p_mfp_lib =
    (multi_frameproc_lib_info_t *)handle;
  uint8_t *p_ssr_flag = (uint8_t *)p_userdata;
  IDBG_HIGH("Unload lib %p SSR %p ", p_mfp_lib, p_ssr_flag);

  if (!p_mfp_lib) {
    IDBG_ERROR("rror unloading library");
    return IMG_ERR_GENERAL;
  }

  if ((p_ssr_flag != NULL) && (*p_ssr_flag)) {
    if (p_mfp_lib->img_algo_shutdown) {
      p_mfp_lib->img_algo_shutdown(&p_mfp_lib->base_ops);
    }
  }

  if (p_mfp_lib->ptr) {
    rc = dlclose(p_mfp_lib->ptr);
    if (rc < 0)
      IDBG_ERROR("error %s", dlerror());
  }

  p_mfp_lib->ptr                    = NULL;
  p_mfp_lib->img_algo_init          = NULL;
  p_mfp_lib->img_algo_deinit        = NULL;
  p_mfp_lib->img_algo_process       = NULL;
  p_mfp_lib->img_algo_frame_ind     = NULL;
  p_mfp_lib->img_algo_meta_ind      = NULL;
  p_mfp_lib->img_algo_set_frame_ops = NULL;
  p_mfp_lib->img_algo_preload       = NULL;
  p_mfp_lib->img_algo_shutdown      = NULL;
  p_mfp_lib->img_algo_abort         = NULL;

  return IMG_SUCCESS;

}

/**
 * Function: multi_frameproc_core_reload_lib
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
static int multi_frameproc_core_reload_lib(void *handle,
  const char *name,  void *p_userdata)
{
  multi_frameproc_lib_info_t *p_mfp_lib =
    (multi_frameproc_lib_info_t *)handle;
  uint8_t *p_ssr_flag = (uint8_t *)p_userdata;
  int32_t status;
  IDBG_HIGH("Reload lib %p SSR %p ", p_mfp_lib, p_ssr_flag);

  if (!p_mfp_lib) {
    IDBG_ERROR("Error unloading library");
    return IMG_ERR_GENERAL;
  }
  p_mfp_lib->ptr = dlopen(name, RTLD_NOW);
  if (!p_mfp_lib->ptr) {
    IDBG_ERROR("Error opening multi_frameproc library %s error %s\n",
      name, dlerror());
    return IMG_ERR_NOT_FOUND;
  }

  status = multi_frameproc_comp_load_fn_ptrs(p_mfp_lib);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("Error status %d", status);
    return status;
  }

  IDBG_HIGH("%s loaded successfully", name);

  if ((p_ssr_flag != NULL) && (*p_ssr_flag)) {
    if (p_mfp_lib->img_algo_preload) {
      p_mfp_lib->img_algo_preload(&p_mfp_lib->base_ops);
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: multi_frameproc_core_set_base_ops
 *
 * Description: Set the base ops
 *
 * Input parameters:
 *   @p_mfp_lib: library handle
 *
 * Return values:
 *   none
  *
 * Notes: none
 **/
static void multi_frameproc_core_set_base_ops(
  multi_frameproc_lib_info_t *p_mfp_lib)
{
  /* memory operations */
  p_mfp_lib->base_ops.mem_ops.open            = img_buffer_open;
  p_mfp_lib->base_ops.mem_ops.get_buffer      = img_buffer_get;
  p_mfp_lib->base_ops.mem_ops.release_buffer  = img_buffer_release;
  p_mfp_lib->base_ops.mem_ops.cache_ops       = img_buffer_cacheops;
  p_mfp_lib->base_ops.mem_ops.close           = img_buffer_close;

  /* Thread operations */
  p_mfp_lib->base_ops.thread_ops.schedule_job =
    img_thread_mgr_schedule_job;
  p_mfp_lib->base_ops.thread_ops.get_time_of_job =
    img_thread_mgr_get_time_of_job;
  p_mfp_lib->base_ops.thread_ops.get_time_of_joblist =
    img_thread_mgr_get_time_of_joblist;
  p_mfp_lib->base_ops.thread_ops.wait_for_completion_by_jobid =
    img_thread_mgr_wait_for_completion_by_jobid;
  p_mfp_lib->base_ops.thread_ops.wait_for_completion_by_joblist =
    img_thread_mgr_wait_for_completion_by_joblist;
  p_mfp_lib->base_ops.thread_ops.wait_for_completion_by_clientid =
    img_thread_mgr_wait_for_completion_by_clientid;
}

/**
 * Function: multi_frameproc_core_unload
 *
 * Description: This function is used to unload multi_frameproc
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
void multi_frameproc_core_unload(void* handle)
{
  multi_frameproc_lib_info_t *p_mfp_lib =
    (multi_frameproc_lib_info_t *)handle;
  IDBG_HIGH("Unload %p ", p_mfp_lib);

  if (!p_mfp_lib) {
    IDBG_ERROR("Error unloading library");
    return;
  }
  if (p_mfp_lib->ptr) {
    multi_frameproc_core_unload_lib(p_mfp_lib, NULL);
    img_dsp_dlclose(p_mfp_lib);
  }

  multi_frameproc_mgr_destroy(p_mfp_lib);

  pthread_mutex_destroy(&p_mfp_lib->mutex);
  free(p_mfp_lib);
}

/**
 * Function: multi_frameproc_core_load
 *
 * Description: This function is used to load multi_frameproc
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
int multi_frameproc_core_load(const char* name, void** handle)
{
  multi_frameproc_lib_info_t *p_mfp_lib;
  int status;

  if (!name || !handle) {
    IDBG_ERROR("invalid input %p %p", name, handle);
    return IMG_ERR_INVALID_INPUT;
  }

  p_mfp_lib = calloc(1, sizeof(multi_frameproc_lib_info_t));
  if (!p_mfp_lib) {
    IDBG_ERROR("Cannot alloc p_mfp_lib %s", name);
    return IMG_ERR_NOT_FOUND;
  }

  status = multi_frameproc_core_reload_lib(p_mfp_lib, name, NULL);
  if (IMG_ERROR(status)) {
    free(p_mfp_lib);
    IDBG_ERROR("cannot load %s status %d", name, status);
    return status;
  }

  /* set baseops */
  multi_frameproc_core_set_base_ops(p_mfp_lib);

  img_dsp_dlopen(name, p_mfp_lib,
    multi_frameproc_core_unload_lib,
    multi_frameproc_core_reload_lib);

  /* initialize managers */
  multi_frameproc_mgr_create(p_mfp_lib);

  pthread_mutex_init(&p_mfp_lib->mutex, NULL);

  *handle = p_mfp_lib;
  IDBG_HIGH("%s loaded successfully", name);
  return IMG_SUCCESS;
}

/**
 * Function: multi_frameproc_core_alloc
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
int multi_frameproc_core_alloc(void *handle, void *p_params)
{
  int32_t status = IMG_SUCCESS;
  multi_frameproc_lib_info_t *p_mfp_lib =
    (multi_frameproc_lib_info_t *)handle;
  img_dim_t *p_dim = (img_dim_t *)p_params;
  IDBG_HIGH(" ");

  if (!p_mfp_lib) {
    IDBG_ERROR("Error alloc library");
    status = IMG_ERR_GENERAL;
    goto end;
  }

  if (p_mfp_lib->img_algo_preload && p_dim) {
    p_mfp_lib->base_ops.max_w = p_dim->stride;
    p_mfp_lib->base_ops.max_h = p_dim->height;
    status = p_mfp_lib->img_algo_preload(&p_mfp_lib->base_ops);
  }

end:
  return status;
}

/**
 * Function: multi_frameproc_comp_dealloc
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
int multi_frameproc_core_dealloc(void *handle)
{
  int32_t status = IMG_SUCCESS;
  multi_frameproc_lib_info_t *p_mfp_lib =
    (multi_frameproc_lib_info_t *)handle;
  IDBG_HIGH("");

  if (!p_mfp_lib) {
    IDBG_ERROR("Error dealloc library");
    status = IMG_ERR_GENERAL;
    goto end;
  }

  if (p_mfp_lib->img_algo_shutdown) {
    status = p_mfp_lib->img_algo_shutdown(
      &p_mfp_lib->base_ops);
  }

end:
  return status;
}

/**
 * Function: multi_frameproc_comp_preload_needed
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
bool multi_frameproc_core_preload_needed(void *handle)
{
  bool preload_needed = false;
  multi_frameproc_lib_info_t *p_mfp_lib =
    (multi_frameproc_lib_info_t *)handle;

  if (!p_mfp_lib) {
    IDBG_ERROR("Error alloc library");
  } else {
    if (p_mfp_lib->img_algo_preload) {
      preload_needed = true;
    }
    IDBG_MED("preload_needed %d", preload_needed);
  }

  return preload_needed;
}

