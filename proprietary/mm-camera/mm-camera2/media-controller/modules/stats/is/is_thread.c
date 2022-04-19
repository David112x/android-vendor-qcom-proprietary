/* is_thread.c
 *
 * Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "is_thread.h"
#include "is_port.h"
#include "camera_dbg.h"

/** is_thread_handler
 *    @data: IS thread data
 *
 *  This is the IS thread's main function.
 *
 *  Returns NULL
 **/
static void* is_thread_handler(void *data)
{
  is_thread_msg_t *msg = NULL;
  is_thread_data_t *thread_data = (is_thread_data_t *)data;
  int exit_flag = 0;
  is_port_private_t *private = thread_data->is_port->port_private;
  boolean rc = FALSE;

  if (!private) {
    return NULL;
  }

  thread_data->active = 1;
  sem_post(&thread_data->sem_launch);
  IS_HIGH("Starting IS thread handler");

  do {
    pthread_mutex_lock(&thread_data->thread_mutex);
    while (thread_data->msg_q->length == 0) {
      pthread_cond_wait(&thread_data->thread_cond, &thread_data->thread_mutex);
    }
    pthread_mutex_unlock(&thread_data->thread_mutex);

    /* Get the message */
    pthread_mutex_lock(&thread_data->msg_q_lock);
    msg = (is_thread_msg_t *)
      mct_queue_pop_head(thread_data->msg_q);
    pthread_mutex_unlock(&thread_data->msg_q_lock);

    if (!msg) {
      IS_ERR("msg NULL");
      continue;
    }

    /* Flush the queue if it is stopping. Free the enqueued messages */
    if (thread_data->active == 0) {
      if (msg->type != MSG_IS_STOP_THREAD) {
        if (msg->sync_flag == TRUE) {
          sem_post(&msg->sync_obj->msg_sem);
          /* Don't free msg, the sender will do */
          msg = NULL;
        } else {
          if ((msg->type == MSG_IS_PROCESS &&
            msg->u.is_process_parm.type == IS_PROCESS_RS_CS_STATS) &&
            msg->u.is_process_parm.u.stats_data.ack_data != NULL) {
            private->is_stats_cb(thread_data->is_port, &msg->u.is_process_parm.u.stats_data);
          }

          free(msg);
          msg = NULL;
        }
        continue;
      }
    }

    IS_LOW("Got event type %d", msg->type);
    switch (msg->type) {
    case MSG_IS_PROCESS:
      if (!thread_data->no_stats_mode || msg->u.is_process_parm.type != IS_PROCESS_RS_CS_STATS) {
        ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_IS);
        rc = private->process(&msg->u.is_process_parm, &private->is_process_output);
        ATRACE_CAMSCOPE_END(CAMSCOPE_IS);
      } else {
        IS_LOW("no_stats_mode");
      }
      if (rc) {
        private->process_callback(thread_data->is_port, &private->is_process_output);
      }
      if (msg->u.is_process_parm.type == IS_PROCESS_RS_CS_STATS &&
        msg->u.is_process_parm.u.stats_data.ack_data != NULL) {
        private->is_stats_cb(thread_data->is_port, &msg->u.is_process_parm.u.stats_data);
      }
      break;

    case MSG_IS_SET:
      rc = private->set_parameters(&msg->u.is_set_parm, &private->is_info, &private->is_set_output);
      if (rc) {
        private->set_callback(thread_data->is_port, &private->is_set_output);
      }
      break;

    case MSG_IS_STOP_THREAD:
      exit_flag = 1;
      break;

    default:
      break;
    }
    if (msg->sync_flag == TRUE) {
      sem_post(&msg->sync_obj->msg_sem);
      /*don't free msg, the sender will do*/
      msg = NULL;
    }
    if (msg) {
      free(msg);
    }
    msg = NULL;
  } while (!exit_flag);

   IS_HIGH("Exiting IS thread handler");
  return NULL;
}


/** is_thread_en_q_msg
 *    @thread_data: IS thread data
 *    @msg: message to enqueue
 *
 *  Enqueues message to the IS thread's queue.
 *
 *  Returns TRUE on success, FALSE on failure.
 **/
boolean is_thread_en_q_msg(is_thread_data_t *thread_data, is_thread_msg_t *msg)
{
  boolean rc = FALSE;
  boolean sync_flag_set = FALSE;
  is_msg_sync_t msg_sync;

  pthread_mutex_lock(&thread_data->msg_q_lock);
  sync_flag_set = msg->sync_flag;
  if (thread_data->active &&
      !((msg->type == MSG_IS_PROCESS && msg->u.is_process_parm.type == IS_PROCESS_RS_CS_STATS)
        && thread_data->no_stats_mode)) {
    rc = TRUE;
    if (TRUE == sync_flag_set) {
      msg->sync_obj = &msg_sync;
      sem_init(&msg_sync.msg_sem, 0, 0);
      sync_flag_set = TRUE;
    }
    mct_queue_push_tail(thread_data->msg_q, msg);

    if (msg->type == MSG_IS_STOP_THREAD) {
      thread_data->active = 0;
    }
  }
  pthread_mutex_unlock(&thread_data->msg_q_lock);

  if (rc) {
    pthread_mutex_lock(&thread_data->thread_mutex);
    pthread_cond_signal(&thread_data->thread_cond);
    pthread_mutex_unlock(&thread_data->thread_mutex);

    if (TRUE == sync_flag_set) {
      sem_wait(&msg_sync.msg_sem);
      sem_destroy(&msg_sync.msg_sem);
    }
  } else {
    IS_HIGH("IS thread_data not active: %d", thread_data->active);
    /* Only free if sync flag is not set, caller must free */
    if (FALSE == sync_flag_set) {
      free(msg);
    }
  }

  return rc;
}


/** is_thread_start
 *    @thread_data: IS thread data
 *
 *  This function creates the IS thread.
 *
 *  Returns TRUE on success
 **/
boolean is_thread_start(is_thread_data_t *thread_data)
{
  boolean rc = TRUE;
   IS_LOW("is thread start! ");

  if (!pthread_create(&thread_data->thread_id, NULL, is_thread_handler,
    (void *)thread_data)) {
  } else {
    rc = FALSE;
  }
  pthread_setname_np(thread_data->thread_id, "CAM_IS");
  return rc;
}


/** is_thread_stop
 *    @thread_data: IS thread data
 *
 *  This function puts the MSG_STOP_THREAD message to the IS thread's queue so
 *  that the thread will stop.  After the message has been successfuly queued,
 *  it waits for the IS thread to join.
 *
 *  Returns TRUE on success, FALSE on failure.
 **/
boolean is_thread_stop(is_thread_data_t *thread_data)
{
  boolean rc;
  is_thread_msg_t *msg;

  sem_wait(&thread_data->sem_launch);

  msg = malloc(sizeof(is_thread_msg_t));
   IS_LOW("is thread stop! ");

  if (msg) {
    STATS_MEMSET(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_STOP_THREAD;
    rc = is_thread_en_q_msg(thread_data, msg);

    if (rc) {
      pthread_join(thread_data->thread_id, NULL);
    }
  } else {
    rc = FALSE;
  }
  return rc;
} /* is_thread_stop */


/** is_thread_init
 *
 *  Initializes the IS thread data and creates the queue.
 *
 *  Returns the thread data object, on failure returnw NULL.
 **/
is_thread_data_t* is_thread_init(void)
{
  is_thread_data_t *is_thread_data;

  is_thread_data = malloc(sizeof(is_thread_data_t));
  if (is_thread_data == NULL) {
    return NULL;
  }
  STATS_MEMSET(is_thread_data, 0, sizeof(is_thread_data_t));

   IS_LOW("Create IS queue ");
  is_thread_data->msg_q = (mct_queue_t *)mct_queue_new;

  if (!is_thread_data->msg_q) {
    free(is_thread_data);
    return NULL;
  }

   IS_LOW("Initialize the IS queue! ");
  pthread_mutex_init(&is_thread_data->msg_q_lock, NULL);
  mct_queue_init(is_thread_data->msg_q);

  pthread_condattr_init(&is_thread_data->thread_condattr);
  pthread_condattr_setclock(&is_thread_data->thread_condattr, CLOCK_MONOTONIC);
  pthread_cond_init(&(is_thread_data->thread_cond), &is_thread_data->thread_condattr);

  pthread_mutex_init(&is_thread_data->thread_mutex, NULL);
  sem_init(&is_thread_data->sem_launch, 0, 0);

  return is_thread_data;
} /* is_thread_init */


/** is_thread_deinit
 *    @thread_data: IS thread data
 *
 *  This function frees resources associated with the IS thread.
 *
 *  Returns void.
 **/
void is_thread_deinit(is_thread_data_t *thread_data)
{
  int ret;
  IS_LOW("called");
  ret = pthread_mutex_destroy(&thread_data->thread_mutex);
  if (ret) {
    if (ret == EBUSY) {
      IS_ERR("Unable to destroy thread_mutex, lock is acquired");
    } else if (ret == EINVAL) {
      IS_ERR("Unable to destroy thread_mutex, lock is invalid");
    }
  }
  pthread_condattr_destroy(&thread_data->thread_condattr);
  pthread_cond_destroy(&thread_data->thread_cond);
  mct_queue_free(thread_data->msg_q);
  ret = pthread_mutex_destroy(&thread_data->msg_q_lock);
  if (ret) {
    if (ret == EBUSY) {
      IS_ERR("Unable to destroy msg_q_lock, lock is acquired");
    } else if (ret == EINVAL) {
      IS_ERR("Unable to destroy msg_q_lock, lock is invalid");
    }
  }
  sem_destroy(&thread_data->sem_launch);
  free(thread_data);
} /* is_thread_deinit */
