/* asd_thread.c
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <pthread.h>
#include "mct_queue.h"
#include "asd_thread.h"
#include "modules.h"
#include <math.h>
#include "camera_dbg.h"


/** asd_thread_free_msg
 *    @msg_pp: Address to the pointer of the msg to free
 *
 *  Free the msg and set it to NULL.
 *  This function free all structures related the the msg, including non-flat
 *  structures that have been allocated by the caller.
 *
 *  Return void
 **/
void asd_thread_free_msg(asd_thread_msg_t **msg_pp)
{
  asd_thread_msg_t *msg = *msg_pp;
  if (NULL == msg) {
    return;
  }

  /* Stats messages are not synced, so we have to free the payload here */
  switch (msg->type) {
    case MSG_ASD_STATS: {
      /* enq-msg has struct of pointers to stats buffer. Free up the memory
       * allocated for struct of pointers.
       * enq-msg carrying this pointer's struct will be deallocated just before
       * exiting this function
       * No need to free the actual stats buffers allocated by ISP. That will be
       * handled in circular_stats_data_ack call */
      free(msg->u.stats);
      msg->u.stats = NULL;
    }
      break;
    case MSG_ASD_SET: {
    /* setparam msg memory is freed up before exiting this function. Following is
     * placeholder to free any additional memory allocated by set-param */
      switch (msg->u.asd_set_parm.type) {
        case ASD_SET_PARAM_INIT_CHROMATIX:
        case ASD_SET_ENABLE:
        case ASD_SET_BESTSHOT:
        case ASD_SET_UI_FRAME_DIM:
        case ASD_SET_STATS_DEBUG_MASK:
        case ASD_SET_SOF:
        case ASD_SET_EZTUNE_RUNNING:
        case ASD_SET_PARAM_OP_MODE:
        default:
          break;
      }
    }
      break;
    default:
      break;
  }
  free(msg);
  *msg_pp = NULL;
}

/** asd_thread_init
 *
 **/
asd_thread_data_t* asd_thread_init(void)
{
  asd_thread_data_t *thread_data;

  thread_data = malloc(sizeof(asd_thread_data_t));
  if (thread_data == NULL)
    return NULL;

  STATS_MEMSET(thread_data, 0, sizeof(asd_thread_data_t));
  thread_data->msg_q = (mct_queue_t *)mct_queue_new;
  if (!thread_data->msg_q) {
    free(thread_data);
    return NULL;
  }

  pthread_mutex_init(&thread_data->msg_q_lock, NULL);
  mct_queue_init(thread_data->msg_q);
  pthread_condattr_init(&thread_data->thread_condattr);
  pthread_condattr_setclock(&thread_data->thread_condattr, CLOCK_MONOTONIC);
  pthread_cond_init(&(thread_data->thread_cond), &thread_data->thread_condattr);
  pthread_mutex_init(&(thread_data->thread_mutex), NULL);
  sem_init(&thread_data->sem_launch, 0, 0);

  return thread_data;
}

/** asd_thread_deinit
 *    @p:
 *
 *  deinitialize ASD thread related resources
 *
 *  Return: No
 **/
void asd_thread_deinit(asd_thread_data_t *thread_data)
{
  int ret;
  if (!thread_data) {
    ASD_ERR("thread_data is NULL");
    return;
  }
  ASD_LOW("thread_data: %p", thread_data);
  ret = pthread_mutex_destroy(&thread_data->thread_mutex);
  if (ret) {
    if (ret == EBUSY) {
      ASD_ERR("Unable to destroy thread_mutex, lock is acquired");
    } else if (ret == EINVAL) {
      ASD_ERR("Unable to destroy thread_mutex, lock is invalid");
    }
  }
  pthread_cond_destroy(&thread_data->thread_cond);
  pthread_condattr_destroy(&thread_data->thread_condattr);
  pthread_mutex_lock(&thread_data->msg_q_lock);
  mct_queue_free(thread_data->msg_q);
  pthread_mutex_unlock(&thread_data->msg_q_lock);
  ret = pthread_mutex_destroy(&thread_data->msg_q_lock);
  if (ret) {
    if (ret == EBUSY) {
      ASD_ERR("Unable to destroy msg_q_lock, lock is acquired");
    } else if (ret == EINVAL) {
      ASD_ERR("Unable to destroy msg_q_lock, lock is invalid");
    }
  }
  sem_destroy(&thread_data->sem_launch);
  free(thread_data);
}

/** asd_thread_en_q_msg:
 *
 **/
boolean asd_thread_en_q_msg(void *asd_data,
  asd_thread_msg_t  *msg)
{
  asd_thread_data_t *thread_data = (asd_thread_data_t *)asd_data;
  boolean rc = FALSE;
  boolean sync_flag_set = FALSE;
  asd_msg_sync_t msg_sync;

  if (!msg || !asd_data) {
    ASD_ERR("Invalid parameters!");
    return FALSE;
  }

  ASD_LOW("Enqueue ASD message");

  pthread_mutex_lock(&thread_data->msg_q_lock);
  sync_flag_set = msg->sync_flag;
  if (thread_data->active &&
      !(msg->type == MSG_ASD_STATS && thread_data->no_stats_mode)) {
    if (TRUE == sync_flag_set) {
      msg->sync_obj = &msg_sync;
      sem_init(&msg_sync.msg_sem, 0, 0);
      sync_flag_set = TRUE;
    }

    mct_queue_push_tail(thread_data->msg_q, msg);
    if (msg->type == MSG_STOP_THREAD) {
      thread_data->active = 0;
      ASD_LOW("Message Stop Thread");
    }
    rc = TRUE;
    if (msg->type == MSG_ASD_STATS) {
      ASD_LOW("Stats msg of stats_mask: %d",
      msg->u.stats->stats_type_mask);
    }
  }
  pthread_mutex_unlock(&thread_data->msg_q_lock);

  if (rc) {
    pthread_mutex_lock(&thread_data->thread_mutex);
    pthread_cond_signal(&thread_data->thread_cond);
    pthread_mutex_unlock(&thread_data->thread_mutex);
    ASD_LOW("Singalled ASD thread handler!");

    if (TRUE == sync_flag_set) {
      sem_wait(&msg_sync.msg_sem);
      sem_destroy(&msg_sync.msg_sem);
    }
  } else {
    ASD_LOW("ASD thread_data is not active: %d",
      thread_data->active);
    /* Only free if sync flag is not set, caller must free */
    if (FALSE == sync_flag_set) {
      asd_thread_free_msg(&msg);
    }
  }

  return rc;
}

/** asd_thread_handler:
 *
 **/
static void* asd_thread_handler(void *data)
{
  asd_thread_data_t *thread_data = (asd_thread_data_t *) data;
  asd_thread_msg_t *msg = NULL;
  asd_object_t *asd_obj = NULL;
  int exit_flag = 0;
  uint8_t face_info_confidence = 0;
  boolean face_info_detected = FALSE;
  const uint8_t face_detect_threshold = 10;
  boolean face_info_updated = FALSE;
  uint8_t face_update_wait_cnt = 0;
  const uint8_t face_update_threshold = 3;


  if (!thread_data)
    return NULL;

  sem_post(&thread_data->sem_launch);

  asd_obj = thread_data->asd_obj;

  ASD_LOW("Starting ASD thread handler");
  do {
    pthread_mutex_lock(&thread_data->thread_mutex);
    while (thread_data->msg_q->length == 0) {
       pthread_cond_wait(&thread_data->thread_cond,
         &thread_data->thread_mutex);
    }
    pthread_mutex_unlock(&thread_data->thread_mutex);

    ASD_LOW("Got signal - time to wake up!");
    /* Get the message */
    pthread_mutex_lock(&thread_data->msg_q_lock);
    msg = (asd_thread_msg_t *)
      mct_queue_pop_head(thread_data->msg_q);
    pthread_mutex_unlock(&thread_data->msg_q_lock);

    if (!msg) {
      continue;
    }

    if(thread_data->active == 0) {
      if(msg->type != MSG_STOP_THREAD) {
        if (msg->sync_flag == TRUE) {
          sem_post(&msg->sync_obj->msg_sem);
          /* Don't free msg, the sender will do */
          msg = NULL;
        } else {
          if (msg->type == MSG_ASD_STATS && msg->u.stats) {
            /* ACK the unused the STATS buffer from ISP */
            thread_data->asd_stats_cb(thread_data->asd_port, msg->u.stats);
          }
          asd_thread_free_msg(&msg);
        }
        continue;
      }
    }

    /* Process message accordingly */
    ASD_LOW("Got the message of type: %d", msg->type);
    switch (msg->type) {
    case MSG_ASD_SET:
      ASD_LOW("Set ASD parameters!");
      if (asd_obj->asd_ops.set_parameters) {
        asd_obj->asd_ops.set_parameters(&msg->u.asd_set_parm, asd_obj->asd);
      }

      /*This needs to be put in asd_set.c */
      if(msg->u.asd_set_parm.type == ASD_SET_SOF){
        ASD_LOW("SOF event");
        thread_data->process_data.frame_count++;

        //sticky logic for face count
        if(!face_info_updated &&
          (face_update_wait_cnt < face_update_threshold)){
          //no face info message updated, increase wait cnt, and break.
          face_update_wait_cnt++;
          break;
        }

        //if no face detected, reduce confidence.
        if (!face_info_detected) {
          if (face_info_confidence > 0)
            face_info_confidence--;
        }

        //update ASD process state if needed
        //note that this is based off the knowledge of the
        //last X frames. we don't know yet if a face is
        //detected for this frame.
        asd_data_face_info_t * face_data =
          &thread_data->process_data.face_data;
        if (face_info_confidence == 0) {
          face_data->use_roi = 0;
        } else if (face_info_confidence == face_detect_threshold) {
          face_data->use_roi = 1;
        }

        face_info_detected = FALSE;
        /* set update to false, and wait to process next face_info*/
        face_info_updated = FALSE;
      }

      break;

    case MSG_ASD_STATS:
      ASD_LOW("Received HISTO Stats for ASD!");

      stats_t* stats = msg->u.stats;
      thread_data->process_data.stats.stats_type_mask = stats->stats_type_mask;
      thread_data->process_data.stats.frame_id = stats->frame_id;
      thread_data->process_data.stats.time_stamp = stats->time_stamp;

      if (stats->stats_type_mask & STATS_IHISTO) {
        thread_data->process_data.stats.yuv_stats.p_histogram = stats->yuv_stats.p_histogram;
      }
      if ((stats->stats_type_mask & STATS_BHISTO) || (stats->stats_type_mask & STATS_HBHISTO)) {
        thread_data->process_data.stats.bayer_stats.p_q3a_bhist_stats = stats->bayer_stats.p_q3a_bhist_stats;
      }
      if (thread_data->process & ASD_ASD_PROCESS) {
        /* output is STATS_MEMSET at QC process, save custom data */
        asd_custom_data_t custom_output = asd_obj->output.asd_custom_output;

        if (!thread_data->no_stats_mode) {
          ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_ASD);
          if (asd_obj->asd_ops.process) {
            asd_obj->asd_ops.process(&(thread_data->process_data),
              asd_obj->asd, &(asd_obj->output));
          }
          /* Assign back ptr to custom memory */
          asd_obj->output.asd_custom_output = custom_output;

          ATRACE_CAMSCOPE_END(CAMSCOPE_ASD);
          thread_data->asd_cb(&(asd_obj->output), thread_data->asd_port);
        } else {
          ASD_LOW("no_stats_mode");
        }
      }
      thread_data->asd_stats_cb(thread_data->asd_port, stats);
      break;
    case MSG_AEC_DATA: {
      ASD_LOW("AEC data received!");
      /* don't process ASD till AWB update data is received */

      thread_data->process |= ASD_AEC_UPDATED;
      thread_data->process_data.aec_data = msg->u.aec_data;
    }
      break;

    case MSG_AWB_DATA: {
      ASD_LOW("AWB data received!");
      thread_data->process |= ASD_AWB_UPDATED;
      thread_data->process_data.awb_data = msg->u.awb_data;
    }
      break;
    case MSG_FACE_INFO: {
      size_t i;
      thread_data->process |= ASD_FACE_INFO_UPDATED;
      //need to call process if state is process? or wait for awb_update?

      /* face info is updated*/
      face_update_wait_cnt = 0;
      face_info_updated = TRUE;
      if (msg->u.face_data.face_count > 0) {
        face_info_detected = TRUE;
        if (face_info_confidence < face_detect_threshold)
          face_info_confidence++;
      }

      ASD_LOW("FACE info received w/ %d faces!", msg->u.face_data.face_count);

      //update to most recent roi
      asd_data_face_info_t * face_data =
        &thread_data->process_data.face_data;
      asd_data_face_info_t * new_face_data =
        &(msg->u.face_data);

      face_data->face_count = msg->u.face_data.face_count;

      for (i = 0; i < face_data->face_count; i++) {
        face_data->faces[i].roi = new_face_data->faces[i].roi;
        face_data->faces[i].score = new_face_data->faces[i].score;
      }
      break;
    }
    case MSG_STOP_THREAD:
      exit_flag = 1;
      break;
    case MSG_SOF:
      break;
    default:
      break;
    }

    if (msg->sync_flag == TRUE) {
      sem_post(&msg->sync_obj->msg_sem);
      /*don't free msg, the sender will do*/
      msg = NULL;
    } else {
      asd_thread_free_msg(&msg);
    }
  } while (!exit_flag);

  return NULL;
}

/**
 *
 **/
boolean asd_thread_start(asd_thread_data_t *thread_data)
{
  pthread_t id;
  int32_t ret;
  ret = pthread_create(&id, NULL, asd_thread_handler, thread_data);
  if (ret < 0) {
    ASD_ERR("Failed to create asd thread");
    return FALSE;
  }
  pthread_setname_np(id, "CAM_ASD");
  thread_data->thread_id = id;
  thread_data->active    = 1;

  return TRUE;
}

/** asd_thread_stop:
 *
 **/
boolean asd_thread_stop(asd_thread_data_t *asd_data)
{
  boolean rc ;
  asd_thread_msg_t *msg = malloc(sizeof(asd_thread_msg_t));

  sem_wait(&asd_data->sem_launch);

  if (msg) {
    ASD_LOW("MSG_STOP_ASD_THREAD");
    STATS_MEMSET(msg, 0, sizeof(asd_thread_msg_t));
    msg->type = MSG_STOP_THREAD;
    rc = asd_thread_en_q_msg(asd_data, msg);
    if (rc) {
      pthread_join(asd_data->thread_id, NULL);
      ASD_LOW("pthread_join");
    }
  } else {
    rc = FALSE;
  }

  return rc;
}
