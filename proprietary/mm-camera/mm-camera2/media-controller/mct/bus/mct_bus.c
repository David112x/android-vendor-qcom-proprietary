/* mct_bus.c
 *
 * This file contains the bus implementation.
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mct_event_stats.h"
#include "mct_bus.h"
#include "camera_dbg.h"

#define MCT_BUS_NANOSECOND_SCALER 1000000000
#define MAX_MCT_BUS_QUEUE_LENGTH 1000

static boolean mct_bus_queue_free(void *data, void *user_data __unused)
{
  mct_bus_msg_t *pdata = data;

  if (pdata) {
    if (pdata->msg) {
      free(pdata->msg);
      pdata->msg = NULL;
    }
    free(pdata);
    pdata = NULL;
  }

  return TRUE;
}

/*
 * mct_bus_timeout_wait:
 *  cond:    POSIX conditional variable;
 *  mutex:   POSIX mutex;
 *  timeout: type of signed long long,  specified
 *           timeout measured in nanoseconds;
 *           timeout = -1 means no timeout, it becomes
 *           to regular conditional timewait.
 *
 *  Commonly used for timeout waiting.
 * */
static int mct_bus_timeout_wait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                         signed long long timeout) {
  signed long long end_time;
  struct timeval r;
  struct timespec ts;
  int ret;
  pthread_mutex_lock(mutex);
  if (timeout != -1) {
    gettimeofday(&r, NULL);
    end_time = (((((signed long long)r.tv_sec) * 1000000) + r.tv_usec) +
      (timeout / 1000));
    ts.tv_sec  = (end_time / 1000000);
    ts.tv_nsec = ((end_time % 1000000) * 1000);
    ret = pthread_cond_timedwait(cond, mutex, &ts);
  } else {
    ret = pthread_cond_wait(cond, mutex);
  }
  pthread_mutex_unlock(mutex);
  return ret;
}

static void* mct_bus_sof_thread_run(void *data)
{
  mct_bus_t *bus = (mct_bus_t *)data;
  signed long long timeout =
    (((signed long long)(bus->thread_wait_time)) * MCT_BUS_NANOSECOND_SCALER);
  int ret;
#ifdef DBG_SOF_FREEZE
  int debug_data = 0;
  int enable = 0;
#endif
  pthread_mutex_lock(&bus->bus_sof_init_lock);
  bus->thread_run = 1;
  pthread_cond_signal(&bus->bus_sof_init_cond);
  pthread_mutex_unlock(&bus->bus_sof_init_lock);

#ifdef DBG_SOF_FREEZE
  while(bus->thread_run) {
    ret = mct_bus_timeout_wait(&bus->bus_sof_msg_cond,
                         &bus->bus_sof_msg_lock, timeout/2);
    if(bus->thread_run) {
      if ((ret == ETIMEDOUT) && (debug_data == 0)) {
        enable = 1;
        debug_data = 1;
        mct_bus_msg_t bus_msg;
        bus_msg.type = MCT_BUS_MSG_NOTIFY_KERNEL;
        bus_msg.size = sizeof(int);
        bus_msg.msg = (void *)&enable;
        bus_msg.sessionid = bus->session_id;
        bus->post_msg_to_bus(bus, &bus_msg);
        CLOGE(CAM_MCT_MODULE, "Session %d: Hinting SOF freeze to happen.\
          Sending event to dump info", bus->session_id);
      } else if ((ret == ETIMEDOUT) && (debug_data == 1)) {
          CLOGE(CAM_MCT_MODULE,
                "FATAL Session %d: SOF Freeze! Sending error message",
                bus->session_id);
          break;
      } else if (debug_data == 1) {
          enable = 0;
          debug_data = 0;
          CLOGE(CAM_MCT_MODULE, "Session %d: Hinting SOF freeze is recover.",
            bus->session_id);
          mct_bus_msg_t bus_msg;
          bus_msg.type = MCT_BUS_MSG_NOTIFY_KERNEL;
          bus_msg.size = sizeof(int);
          bus_msg.msg = (void *)&enable;
          bus_msg.sessionid = bus->session_id;
          bus->post_msg_to_bus(bus, &bus_msg);
      }
    } else {
      CLOGI(CAM_MCT_MODULE, "Closing SOF tracker thread");
      break;
    }
  }
#else
  while(bus->thread_run) {
    ret = mct_bus_timeout_wait(&bus->bus_sof_msg_cond,
                         &bus->bus_sof_msg_lock, timeout);
    if (ret == ETIMEDOUT) {
      CLOGE(CAM_MCT_MODULE,
            "FATAL Session %d: SOF Freeze! Sending error message",
            bus->session_id);
      break;
    }
  }
#endif

  if (bus->thread_run == 1) {
    /*Things went wrong*/
    CLOGI(CAM_MCT_MODULE, "Sending HW_ERROR from MCT on session =%d",
      bus->session_id);
    mct_bus_msg_t bus_msg;
    bus_msg.type = MCT_BUS_MSG_SEND_HW_ERROR;
    bus_msg.size = 0;
    bus_msg.sessionid = bus->session_id;
    bus->post_msg_to_bus(bus, &bus_msg);
  }
  return NULL;
}

static void start_sof_check_thread(mct_bus_t *bus)
{
  int rc = 0;
  char sof_thread_name[16];
  if (!bus) {
    CLOGE(CAM_MCT_MODULE, "NULL bus ptr");
    return;
  }
  if (bus->thread_run == 1)
    return;

  CLOGI(CAM_MCT_MODULE,
        "Starting SOF timeout thread session id =%d",
        bus->session_id);

  pthread_mutex_init(&bus->bus_sof_msg_lock, NULL);
  pthread_cond_init(&bus->bus_sof_msg_cond, NULL);
  pthread_mutex_lock(&bus->bus_sof_init_lock);
  rc = pthread_create(&bus->bus_sof_tid, NULL, mct_bus_sof_thread_run, bus);
  if(!rc) {
    snprintf(sof_thread_name, sizeof(sof_thread_name),
      "CAM_softimer_%d", bus->session_id);
    pthread_setname_np(bus->bus_sof_tid, sof_thread_name);
    pthread_cond_wait(&bus->bus_sof_init_cond, &bus->bus_sof_init_lock);
  }
  pthread_mutex_unlock(&bus->bus_sof_init_lock);
}

static void stop_sof_check_thread(mct_bus_t *bus)
{
  if (!bus) {
    CLOGE(CAM_MCT_MODULE, "NULL bus ptr");
    return;
  }
  pthread_mutex_lock(&bus->bus_sof_init_lock);
  if (bus->thread_run == 0) {
    CLOGE(CAM_MCT_MODULE, "Returning as SOF timer thread not yet initialized");
    pthread_mutex_unlock(&bus->bus_sof_init_lock);
    return;
  }
  bus->thread_run = 0;
  pthread_mutex_unlock(&bus->bus_sof_init_lock);


  CLOGI(CAM_MCT_MODULE,
        "Stopping SOF timeout thread session =%d", bus->session_id);

  pthread_mutex_lock(&bus->bus_sof_msg_lock);
  pthread_cond_signal(&bus->bus_sof_msg_cond);
  pthread_mutex_unlock(&bus->bus_sof_msg_lock);
  pthread_join(bus->bus_sof_tid, NULL);
  pthread_cond_destroy(&bus->bus_sof_msg_cond);
  pthread_mutex_destroy(&bus->bus_sof_msg_lock);
}

/** mct_bus_sof_tracker:
 *    @bus: Bus pointer
 *    @cur_sof_id: Current SOF frame ID
 *
 *  Description:
 *    Tracker for incoming SOF frame IDs from ISP.
 *    SOF frame IDs are expected to be monotonically increasing
 *    tokens generated by ISP. In normal operations, every SOF ID
 *    is exactly 1 more than the previous SOF ID.
 *
 *  Return: TRUE if uniform monotonic increase in SOF ID is detected.
 *             FALSE if not.
 **/
boolean mct_bus_sof_tracker(mct_bus_t *bus, uint32_t cur_sof_id)
{
  int32_t sof_diff = 0;
  boolean ret = TRUE;
  if (!bus) {
    CLOGE(CAM_MCT_MODULE, "Invalid bus ptr");
    return FALSE;
  }

  sof_diff = (int32_t)(cur_sof_id - bus->prev_sof_id);
  if ( sof_diff < 1 ) {
    CLOGE(CAM_MCT_MODULE, "Invalid ISP SOF ID %u received, prev ID is %u",
      cur_sof_id, bus->prev_sof_id);
    ret = FALSE;
  } else if ( sof_diff != 1 ) {
    CLOGE(CAM_MCT_MODULE, "Current SOF: %u. SOF missed for %d frame(s) session %d",
      cur_sof_id, sof_diff-1, bus->session_id);
    ret = TRUE;
  }
  bus->prev_sof_id = cur_sof_id;

  return ret;
}

static boolean mct_bus_post_msg(mct_bus_t *bus, mct_bus_msg_t *bus_msg)
{
  mct_bus_msg_t *local_msg;
  boolean post_msg = FALSE;
  mct_bus_msg_isp_sof_t *isp_sof_bus_msg = NULL;
  unsigned int payload_size;

  if (!bus || !bus_msg) {
    CLOGE(CAM_MCT_MODULE, "NULL ptr detected: bus = [%p], bus_msg = [%p]",
       bus, bus_msg);
    goto error_2;
  }

  if (bus->bus_queue->length > MAX_MCT_BUS_QUEUE_LENGTH) {
    pthread_mutex_lock(&bus->bus_msg_q_lock);
    mct_bus_queue_flush(bus);
    CLOGI(CAM_MCT_MODULE,
          "Discarded the bus msgs that got stagnated in the queue");
    pthread_mutex_unlock(&bus->bus_msg_q_lock);
    return TRUE;
  }
  if (bus_msg->type >= MCT_BUS_MSG_MAX) {
      CLOGI(CAM_MCT_MODULE, "bus_msg type %d is not valid", bus_msg->type);
      goto error_2;
  }
  payload_size = bus_msg->size;

  switch (bus_msg->type) {
    case MCT_BUS_MSG_ISP_SOF:
      post_msg = TRUE;
      if (bus->thread_run == 1) {
        pthread_mutex_lock(&bus->bus_sof_msg_lock);
        pthread_cond_signal(&bus->bus_sof_msg_cond);
        pthread_mutex_unlock(&bus->bus_sof_msg_lock);
      }
      isp_sof_bus_msg = bus_msg->msg;
      CLOGD(CAM_MCT_MODULE,
        "Posting SOF for frame ID %d session = %d",
        isp_sof_bus_msg->frame_id, bus_msg->sessionid);
      break;
    case MCT_BUS_MSG_SEND_HW_ERROR:
      post_msg = TRUE;
      pthread_mutex_lock(&bus->bus_msg_q_lock);
      mct_bus_queue_flush(bus);
      pthread_mutex_unlock(&bus->bus_msg_q_lock);
      break;
    case MCT_BUS_MSG_SENSOR_STARTING:
      bus->thread_wait_time = *((uint32_t *)bus_msg->msg);
      start_sof_check_thread(bus);
      return TRUE;
      break;
    case MCT_BUS_MSG_SENSOR_STOPPING:
      stop_sof_check_thread(bus);
      return TRUE;
      break;

    case MCT_BUS_MSG_ERROR_MESSAGE:
    case MCT_BUS_MSG_NOTIFY_KERNEL:
    case MCT_BUS_MSG_CONTROL_REQUEST_FRAME:
    case MCT_BUS_MSG_EZTUNE_JPEG:
    case MCT_BUS_MSG_EZTUNE_RAW:
    case MCT_BUS_MSG_DELAY_SUPER_PARAM:
    case MCT_BUS_MSG_FRAME_SKIP:
    case MCT_BUS_MSG_CAC_STAGE_DONE:
    case MCT_BUS_MSG_ISP_RD_DONE:
      post_msg = TRUE;
      break;

    default:
      break;
  }

  local_msg = malloc(sizeof(mct_bus_msg_t));
  if (!local_msg) {
    CLOGE(CAM_MCT_MODULE, "Can't allocate memory");
    goto error_2;
  }

  local_msg->sessionid = bus_msg->sessionid;
  local_msg->type = bus_msg->type;
  local_msg->size = bus_msg->size;

  if (payload_size) {
    local_msg->msg = malloc(payload_size);
    if (!local_msg->msg) {
      CLOGE(CAM_MCT_MODULE, "Can't allocate memory");
      goto error_1;
    }
    memcpy(local_msg->msg, bus_msg->msg, payload_size);
  } else {
    local_msg->msg = NULL;
  }

  /* Push critical messages to priority Queue
    * and post signal to Media Controller */
  if (post_msg) {
    pthread_mutex_lock(&bus->priority_q_lock);
    mct_queue_push_tail(bus->priority_queue, local_msg);
    pthread_mutex_unlock(&bus->priority_q_lock);

    pthread_mutex_lock(bus->mct_mutex);
    pthread_cond_signal(bus->mct_cond);
    pthread_mutex_unlock(bus->mct_mutex);
  }
  else {
   /*Store bus messages in bus_msg queue
        and dequeue during next SOF. */
    pthread_mutex_lock(&bus->bus_msg_q_lock);
    mct_queue_push_tail(bus->bus_queue, local_msg);
    pthread_mutex_unlock(&bus->bus_msg_q_lock);
  }
  return TRUE;

error_1:
  free(local_msg);
  local_msg = NULL;
error_2:
  return FALSE;
}

mct_bus_t *mct_bus_create(unsigned int session)
{
  mct_bus_t *new_bus;
  new_bus = malloc(sizeof(mct_bus_t));
  if (!new_bus) {
    CLOGE(CAM_MCT_MODULE, "Failed to allocate memory to new bus");
    return NULL;
  }

  memset(new_bus, 0 , sizeof(mct_bus_t));
  pthread_mutex_init(&new_bus->bus_msg_q_lock, NULL);
  pthread_mutex_init(&new_bus->priority_q_lock, NULL);
  pthread_mutex_init(&new_bus->bus_sof_init_lock, NULL);
  pthread_cond_init(&new_bus->bus_sof_init_cond, NULL);

  new_bus->bus_queue = mct_queue_new;
  if (!new_bus->bus_queue) {
    goto busmsgq_error;
  }
  mct_queue_init(new_bus->bus_queue);

  new_bus->priority_queue = mct_queue_new;
  if (!new_bus->priority_queue) {
    goto busmsgpq_error;
  }
  mct_queue_init(new_bus->priority_queue);

  new_bus->post_msg_to_bus = mct_bus_post_msg;
  new_bus->session_id = session;
  return new_bus;

busmsgpq_error:
  free(new_bus->bus_queue);
busmsgq_error:
  pthread_cond_destroy(&new_bus->bus_sof_init_cond);
  pthread_mutex_destroy(&new_bus->bus_sof_init_lock);
  pthread_mutex_destroy(&new_bus->priority_q_lock);
  pthread_mutex_destroy(&new_bus->bus_msg_q_lock);
  free(new_bus);
  new_bus= NULL;
  return NULL;
}

void mct_bus_destroy(mct_bus_t *bus)
{
  if (!bus) {
    CLOGE(CAM_MCT_MODULE, "NULL bus ptr");
    return;
  }
  pthread_mutex_lock(&bus->bus_msg_q_lock);
  if (!MCT_QUEUE_IS_EMPTY(bus->bus_queue))
    mct_queue_free_all(bus->bus_queue, mct_bus_queue_free);
  else {
    free(bus->bus_queue);
    bus->bus_queue = NULL;
  }

  pthread_mutex_unlock(&bus->bus_msg_q_lock);

  pthread_mutex_lock(&bus->priority_q_lock);
  if (!MCT_QUEUE_IS_EMPTY(bus->priority_queue))
    mct_queue_free_all(bus->priority_queue, mct_bus_queue_free);
  else {
    free(bus->priority_queue);
    bus->priority_queue = NULL;
  }

  pthread_mutex_unlock(&bus->priority_q_lock);
  pthread_mutex_destroy(&bus->priority_q_lock);

  stop_sof_check_thread (bus);

  pthread_cond_destroy(&bus->bus_sof_init_cond);
  pthread_mutex_destroy(&bus->bus_sof_init_lock);
  pthread_mutex_destroy(&bus->bus_msg_q_lock);

  free(bus);
  bus = NULL;
  return;
}


void mct_bus_queue_flush(mct_bus_t *bus)
{
  if (!bus)
    return;

  if (!MCT_QUEUE_IS_EMPTY(bus->bus_queue))
    mct_queue_flush(bus->bus_queue, mct_bus_queue_free);

  return;
}

void mct_bus_priority_queue_flush(mct_bus_t *bus)
{
  if (!bus)
    return;

  if (!MCT_QUEUE_IS_EMPTY(bus->priority_queue))
    mct_queue_flush(bus->priority_queue, mct_bus_queue_free);

  return;
}
