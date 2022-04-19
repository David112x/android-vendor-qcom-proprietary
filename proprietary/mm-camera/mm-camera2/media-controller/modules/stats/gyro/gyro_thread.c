/* stats/gyro/gyro_thread.c
 *
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "gyro_thread.h"
#include "gyro_port.h"
#include "camera_dbg.h"

/** gyro_thread_handler
 *    @data: Gyro thread data
 *
 *  This is the Gyro thread's main function.
 *
 *  Returns NULL
 **/
static void* gyro_thread_handler(void *data)
{
  gyro_thread_msg_t *msg = NULL;
  gyro_thread_data_t *thread_data = (gyro_thread_data_t *)data;
  int exit_flag = 0;
  gyro_port_private_t *private = thread_data->gyro_port->port_private;
  boolean rc = FALSE;

  if (!private) {
    return NULL;
  }

  thread_data->active = 1;
  sem_post(&thread_data->sem_launch);

  /*Initialize Sensors' frameworks
    TODO: Handle the case when gyro_port_init_sensors returns FALSE*/
  gyro_port_init_sensors(thread_data->gyro_port);
  IS_HIGH("Starting Gyro thread handler");

  do {
    pthread_mutex_lock(&thread_data->thread_mutex);
    while (thread_data->msg_q->length == 0) {
      pthread_cond_wait(&thread_data->thread_cond, &thread_data->thread_mutex);
    }
    pthread_mutex_unlock(&thread_data->thread_mutex);

    /* Get the message */
    pthread_mutex_lock(&thread_data->msg_q_lock);
    msg = (gyro_thread_msg_t *)
      mct_queue_pop_head(thread_data->msg_q);
    pthread_mutex_unlock(&thread_data->msg_q_lock);

    if (!msg) {
      IS_ERR("msg NULL");
      continue;
    }

    /* Flush the queue if it is stopping. Free the enqueued messages */
    if (thread_data->active == 0) {
      if (msg->type != MSG_GYRO_STOP) {
        if (msg->sync_flag == TRUE) {
          sem_post(&msg->sync_obj->msg_sem);
          /* Don't free msg, the sender will do */
          msg = NULL;
        } else {
          if (msg->type == MSG_GYRO_PROCESS && msg->u.process_params.process_params_type == MSG_GYRO_PROCESS_CALLBACK) {
            if (msg->u.process_params.u.cb_data.cb_type== CB_TYPE_DSPS) {
              gyro_port_process_callback(thread_data->gyro_port, msg->u.process_params.u.cb_data.u.dsps_cb_data);
              free(msg->u.process_params.u.cb_data.u.dsps_cb_data);
            } else if (msg->u.process_params.u.cb_data.cb_type == CB_TYPE_ANDROID) {
              sensor_event_cb_sensor_data(thread_data->gyro_port, &msg->u.process_params.u.cb_data.u.andr_cb_data->u.sensor_data);
              free(msg->u.process_params.u.cb_data.u.andr_cb_data);
            }
          }
          free(msg);
          msg = NULL;
        }
        continue;
      }
    }

    IS_LOW("Got event type %d", msg->type);
    switch (msg->type) {
      case MSG_GYRO_PROCESS:
        switch(msg->u.process_params.process_params_type) {
          case MSG_GYRO_PROCESS_CALLBACK:
            switch(msg->u.process_params.u.cb_data.cb_type) {
              case CB_TYPE_DSPS:
                gyro_port_process_callback(thread_data->gyro_port, msg->u.process_params.u.cb_data.u.dsps_cb_data);
                if (msg->u.process_params.u.cb_data.u.dsps_cb_data) {
                  free(msg->u.process_params.u.cb_data.u.dsps_cb_data);
                }
                break;
              case CB_TYPE_ANDROID:
                sensor_event_cb_sensor_data(thread_data->gyro_port, &msg->u.process_params.u.cb_data.u.andr_cb_data->u.sensor_data);
                if (msg->u.process_params.u.cb_data.u.andr_cb_data) {
                  free(msg->u.process_params.u.cb_data.u.andr_cb_data);
                }
                break;

              default:
                break;
            }
            break;

          case MSG_GYRO_REQUEST_DATA:
            IS_LOW("SOF event : gyro_thread");
            gyro_thread_process_sof_event(&msg->u.process_params.u.data_req,private);
            break;

          default:
            break;
        }
        break;

      case MSG_GYRO_SET:
        switch(msg->u.set_params.set_params_type) {
          case MSG_GYRO_SET_IS_PARAMS:
            dsps_port_handle_set_is_enable(private,msg->u.set_params.u.is_params.is_enable);
            break;

          case MSG_GYRO_SET_COMMON_PARAMS:
            dsps_port_handle_set_common_params(thread_data->gyro_port,msg->u.set_params.u.common_params.stream_on);
            break;

          default:
            break;
        }
      break;

      case MSG_GYRO_STOP:
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

  IS_HIGH("Exiting Gyro thread handler");
  return NULL;
}


/** gyro_thread_en_q_msg
 *    @thread_data: Gyro thread data
 *    @msg: message to enqueue
 *
 *  Enqueues message to the Gyro thread's queue.
 *
 *  Returns TRUE on success, FALSE on failure.
 **/
boolean gyro_thread_en_q_msg(gyro_thread_data_t *thread_data, gyro_thread_msg_t *msg)
{
  boolean rc = FALSE;
  boolean sync_flag_set = FALSE;
  gyro_msg_sync_t msg_sync;

  pthread_mutex_lock(&thread_data->msg_q_lock);
  sync_flag_set = msg->sync_flag;
  if (thread_data->active) {
    rc = TRUE;
    if (TRUE == sync_flag_set) {
      msg->sync_obj = &msg_sync;
      sem_init(&msg_sync.msg_sem, 0, 0);
      sync_flag_set = TRUE;
    }
    mct_queue_push_tail(thread_data->msg_q, msg);

    if (msg->type == MSG_GYRO_STOP) {
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
    IS_HIGH("Gyro thread_data not active: %d", thread_data->active);
    /* Only free if sync flag is not set, caller must free */
    if (FALSE == sync_flag_set) {
      if (msg->u.process_params.u.cb_data.u.andr_cb_data) {
        free(msg->u.process_params.u.cb_data.u.andr_cb_data);
      }
      if (msg->u.process_params.u.cb_data.u.dsps_cb_data) {
        free(msg->u.process_params.u.cb_data.u.dsps_cb_data);
      }
      free(msg);
    }
  }

  return rc;
}


/** gyro_thread_start
 *    @thread_data: Gyro thread data
 *
 *  This function starts the Gyro thread.
 *
 *  Returns TRUE on success
 **/
boolean gyro_thread_start(gyro_thread_data_t *thread_data)
{
  boolean rc = TRUE;
  IS_LOW("Gyro thread start! ");

  if (!pthread_create(&thread_data->thread_id, NULL, gyro_thread_handler,
    (void *)thread_data)) {
    pthread_setname_np(thread_data->thread_id, "CAM_GYRO");
  } else {
    rc = FALSE;
  }
  return rc;
}


/** gyro_thread_stop
 *    @thread_data: Gyro thread data
 *
 *  This function puts the MSG_STOP_THREAD message to the Gyro thread's queue so
 *  that the thread will stop.  After the message has been successfuly queued,
 *  it waits for the Gyro thread to join.
 *
 *  Returns TRUE on success, FALSE on failure.
 **/
boolean gyro_thread_stop(gyro_thread_data_t *thread_data)
{
  boolean rc;
  gyro_thread_msg_t *msg;

  sem_wait(&thread_data->sem_launch);

  msg = malloc(sizeof(gyro_thread_msg_t));
  IS_LOW("Gyro thread stop! ");

  if (msg) {
    memset(msg, 0, sizeof(gyro_thread_msg_t));
    msg->type = MSG_GYRO_STOP;
    rc = gyro_thread_en_q_msg(thread_data, msg);

    if (rc) {
      pthread_join(thread_data->thread_id, NULL);
    }
  } else {
    rc = FALSE;
  }
  return rc;
} /* gyro_thread_stop */


/** gyro_thread_init
 *
 *  Initializes the Gyro thread data and creates the queue.
 *
 *  Returns the thread data object, on failure returnw NULL.
 **/
gyro_thread_data_t* gyro_thread_init(void)
{
  gyro_thread_data_t *gyro_thread_data;

  gyro_thread_data = malloc(sizeof(gyro_thread_data_t));
  if (gyro_thread_data == NULL) {
    return NULL;
  }
  memset(gyro_thread_data, 0, sizeof(gyro_thread_data_t));

  IS_LOW("Create Gyro queue ");
  gyro_thread_data->msg_q = (mct_queue_t *)mct_queue_new;

  if (!gyro_thread_data->msg_q) {
    free(gyro_thread_data);
    return NULL;
  }

  IS_LOW("Initialize the gyro queue! ");
  pthread_mutex_init(&gyro_thread_data->msg_q_lock, NULL);
  mct_queue_init(gyro_thread_data->msg_q);
  pthread_cond_init(&gyro_thread_data->thread_cond, NULL);
  pthread_mutex_init(&gyro_thread_data->thread_mutex, NULL);
  sem_init(&gyro_thread_data->sem_launch, 0, 0);

  return gyro_thread_data;
} /* gyro_thread_init */


/** gyro_thread_deinit
 *    @thread_data: Gyro thread data
 *
 *  This function frees resources associated with the Gyro thread.
 *
 *  Returns void.
 **/
void gyro_thread_deinit(gyro_thread_data_t *thread_data)
{
  IS_LOW("De-initializing Gyro Thread");
  pthread_mutex_destroy(&thread_data->thread_mutex);
  pthread_cond_destroy(&thread_data->thread_cond);
  mct_queue_free(thread_data->msg_q);
  pthread_mutex_destroy(&thread_data->msg_q_lock);
  sem_destroy(&thread_data->sem_launch);
  free(thread_data);
} /* gyro_thread_deinit */

