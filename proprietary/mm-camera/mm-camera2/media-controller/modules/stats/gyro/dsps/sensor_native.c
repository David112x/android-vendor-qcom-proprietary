/* sensor_native.c
 *
 * Copyright (c) 2014-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "sensor_native.h"


#ifdef SENSOR_NATIVE_LAYER
#include <pthread.h>
#include <utils/Timers.h>
#include "sensor_service_util.h"

/*keep in sync with hardware/sensor.h */
#define ASENSOR_TYPE_GRAVITY 9
#define ASENSOR_TYPE_LINEAR_ACCELERATION 10
#define ASENSOR_TYPE_TIME_OF_FLIGHT 40

sensor_native_type g_snobj;


/*===========================================================================
 * FUNCTION   timestamp_correction
 * timestamp correction b/w gettimeofday() and systemTime().
 * Correction is required because gyro port requests samples
 * using gettimeofday() timestamps but android sensor API uses systemTime()
 *=========================================================================*/
static void timestamp_correction(int64_t *tc)
{
  struct timeval tv;
  *tc = 0;
  gettimeofday(&tv, NULL);
  int64_t sys_time = systemTime(SYSTEM_TIME_BOOTTIME)/1000;
  int64_t get_time = ((int64_t)tv.tv_sec * 1000000) + tv.tv_usec;
  *tc = sys_time - get_time;
  IS_LOW("sys_time : %lld - gettime: %lld = time_correction: %lld",
    sys_time, get_time, *tc);
}

static int wait_loop(void *data)
{
  sensor_native_type *sn_obj = (sensor_native_type *)data;
  do {
    /*sleep it for long time to avoid unneccessary wake up */
    sleep(2000);
  } while (sn_obj->first_threadflag == TRUE);
  return 0;
}


/*===========================================================================
 * FUNCTION      sensor_native_gyro_start_search_forward
 *    @handle:Handler
 *    @start_pos:Start position
 *
 * DESCRIPTION : Search forward in the buffer for starting sample
 *=========================================================================*/
static sensor_return_type sensor_native_gyro_start_search_forward(
  void *handle,
  int start_pos)
{
  sensor_native_type *sn_obj = (sensor_native_type *)handle;
  if (start_pos > GYRO_BUFFER_SIZE - 1) {
    start_pos = 0;
  }
  IS_LOW("search forward for start pos");
  do {
    if (sn_obj->sof_info.t_start > sn_obj->cirq_buffer[start_pos].timestamp) {
      start_pos++;
    } else {
      sn_obj->start_idx = start_pos;
      return SENSOR_RET_SUCCESS;
    }
    if (start_pos > GYRO_BUFFER_SIZE - 1) {
      start_pos = 0;
    }
  } while (start_pos != sn_obj->end_idx);
  return SENSOR_RET_FAILURE;
}

/*===========================================================================
 * FUNCTION      sensor_native_gyro_start_search_backward
 *    @handle:Handler
 *    @start_pos: start position
 *
 * DESCRIPTION: Search backward in the buffer for starting sample
 *=========================================================================*/
static sensor_return_type sensor_native_gyro_start_search_backward(
  void *handle,
  int start_pos)
{
  sensor_native_type *sn_obj = (sensor_native_type *)handle;
  if (start_pos < 0) {
    start_pos = GYRO_BUFFER_SIZE - 1;
  }
  IS_LOW("search backward for start pos");
  do{
    if (sn_obj->sof_info.t_start < sn_obj->cirq_buffer[start_pos].timestamp) {
      start_pos--;
    } else {
      sn_obj->start_idx = start_pos;
      return SENSOR_RET_SUCCESS;
    }
    if (start_pos < 0) {
      start_pos = GYRO_BUFFER_SIZE - 1;
    }
  } while (start_pos != sn_obj->end_idx);
  return SENSOR_RET_FAILURE;
}

/*===========================================================================
 * FUNCTION      sensor_native_get_thread_status
 *    @p: Handler
 *
 * DESCRIPTION: Returns the current status of the sensor thread
 *=========================================================================*/
static boolean sensor_native_get_thread_status(void *p)
{
  sensor_native_type *sn_obj = (sensor_native_type *)p;
  sensor_native_thread_status_type status = sn_obj->thread_status;
  boolean ret_val = TRUE;

  if (status & SENSOR_NATIVE_THREAD_ERROR) {
    IS_LOW("Thread is in error state, ret false");
    ret_val = FALSE;
  } else if (status & SENSOR_NATIVE_THREAD_OFF) {
    IS_LOW("Thread state is OFF, return false")
    ret_val = FALSE;
  } else if (status & SENSOR_NATIVE_THREAD_ON) {
    IS_LOW("Thread state is active, ret true")
    ret_val = TRUE;
  } else if (status & SENSOR_NATIVE_THREAD_SUSPEND) {
    IS_LOW("Thread is in suspend state, ret false");
    ret_val = FALSE;
  } else {
    IS_ERR("Invalid_thread status");
    ret_val = FALSE;
  }

  IS_LOW("Sensor Thread status: %d ret_val: %d", status, ret_val);
  return ret_val;
}

/*===========================================================================
 * FUNCTION      sensor_native_gyro_port_notify
 *    @handle:Handler
 *
 * DESCRIPTION :  Copy the requested gyro samples to the callback data struct and notify
 *                          Gyro port
 *=========================================================================*/
static sensor_return_type sensor_native_gyro_port_notify(void *handle)
{
  sensor_native_type *sn_obj = (sensor_native_type *)handle;
  sensor_cb_struct_type cb_event_data;
  sensor_gyro_buffer_data_type *gyro_data = NULL;
  int start_pos = sn_obj->start_idx;
  int end_pos = sn_obj->end_idx;
  int sample_len = 0;
  int idx = 0;
  /* check for number of samples available for frame
     maximum allowed samples are in range */
  sample_len = (start_pos <= end_pos) ? (end_pos-start_pos + 1) :
    (GYRO_BUFFER_SIZE - start_pos + end_pos + 1);
  if (sample_len > STATS_GYRO_MAX_SAMPLE_BUFFER_SIZE) {
    IS_ERR("Sample length crossed the maximum allowed");
    return SENSOR_RET_FAILURE;
  }

  /* Prepare data for CB */
  cb_event_data.type = SENSOR_DATA;
  cb_event_data.u.sensor_data.type = SENSOR_GYRO;
  cb_event_data.u.sensor_data.timestamp = 0; /* not used */
  /* Getting data from cirq_buf */
  gyro_data = &cb_event_data.u.sensor_data.u.gyro;
  gyro_data->seqnum = sn_obj->sof_info.id;
  gyro_data->sample_len = sample_len;
  idx = 0;
  while (idx < sample_len) {
    gyro_data->sample[idx].timestamp =
      sn_obj->cirq_buffer[start_pos].timestamp;
    gyro_data->sample[idx].value[0]=
      sn_obj->cirq_buffer[start_pos].u.gyro.x;
    gyro_data->sample[idx].value[1]=
      sn_obj->cirq_buffer[start_pos].u.gyro.y;
    gyro_data->sample[idx].value[2]=
      sn_obj->cirq_buffer[start_pos].u.gyro.z ;
    start_pos++;
    idx++;
    if (start_pos > GYRO_BUFFER_SIZE - 1) {
      start_pos = start_pos % GYRO_BUFFER_SIZE;
    }
  }

  pthread_mutex_lock(&(sn_obj->cb_mutex));
  if (sn_obj->gyro.enabled) {
    sn_obj->data_cb(sn_obj->cb_hndl, cb_event_data);
  }
  pthread_mutex_unlock(&(sn_obj->cb_mutex));
  return SENSOR_RET_SUCCESS;
}

/*===========================================================================
 * FUNCTION      sensor_native_process_request
 *    @handle:Sensor handler object
 *
 * DESCRIPTION : Process the Gyro port request
 *=========================================================================*/
static sensor_return_type sensor_native_process_request(void *handle)
{
  sensor_native_type *sn_obj = (sensor_native_type *)handle;
  int end_pos = sn_obj->buf_pos - 1;
  int start_pos = sn_obj->pend_idx;
  unsigned long long start_ts = sn_obj->pend_ts;
  if (end_pos < 0) {
    end_pos = GYRO_BUFFER_SIZE - 1;
  }
  /* search for the end timestamp of gyro request in buffer */
  do {
    if (sn_obj->cirq_buffer[end_pos].timestamp < sn_obj->sof_info.t_end) {
      sn_obj->end_idx = end_pos;
      /* search for start position */
      if (sn_obj->sof_info.t_start == start_ts) {
        sn_obj->start_idx = start_pos;
      } else if (sn_obj->sof_info.t_start >
        sn_obj->cirq_buffer[start_pos].timestamp) {
        /* search forward */
        sensor_native_gyro_start_search_forward(handle,start_pos);
      } else {
        /* search backward */
        sensor_native_gyro_start_search_backward(handle,start_pos);
      }
        /* Requested data ready; Copy into circular queue */
      IS_LOW("copying data to circular queue: start:%d end:%d",
        sn_obj->start_idx,sn_obj->end_idx);
      return sensor_native_gyro_port_notify(handle);
    }
    else{
      end_pos--;
    }
    if (end_pos < 0) {
      end_pos = GYRO_BUFFER_SIZE - 1;
    }
  }  while (end_pos != sn_obj->pend_idx);
  return SENSOR_RET_FAILURE;
}

/*===========================================================================
 * FUNCTION      sensor_native_enqueue
 *    @handle: data pointer of sensor object
 *    @sensor_data_type: sensor data
 *
 * DESCRIPTION   Store the received samples in a circular buffer
 *=========================================================================*/
static sensor_return_type sensor_native_enqueue(
  void *handle,
  sensor_data_type sensor_data)
{
  sensor_native_type *sn_obj = (sensor_native_type *)handle;
  if (sn_obj->buf_pos > GYRO_BUFFER_SIZE - 1) {
    sn_obj->buf_pos = 0;
  }
  int buf_pos =sn_obj->buf_pos;
  IS_LOW("Storing sample in circular buffer:pos:%d", buf_pos);
  sn_obj->cirq_buffer[buf_pos].timestamp = sensor_data.timestamp;
  /*orienation changes in android frmaework. To make
     it in complaince with dsps; X <--> Y; Z --> -Z */
  sn_obj->cirq_buffer[buf_pos].u.gyro.x = (int)(sensor_data.u.gyro.y * (1 << 16));
  sn_obj->cirq_buffer[buf_pos].u.gyro.y = (int)(sensor_data.u.gyro.x * (1 << 16));
  sn_obj->cirq_buffer[buf_pos].u.gyro.z = -(int)(sensor_data.u.gyro.z * (1 << 16));

  /*  check for a request */
  if (sn_obj->sof_info.t_end != 0) {
    /* check wheteher the requested samples are available */
    if (sensor_data.timestamp >= sn_obj->sof_info.t_end) {
      /*Lock the update of SoF request from gyro_port*/
      pthread_mutex_lock(&(sn_obj->sof_req_mutex));
      /* The requested samples are available in buffer ;
         Copy them in to a circular queue and notify gyro port*/
      IS_LOW("Requested samples are available in buffer");
      if (sensor_native_process_request(handle) != SENSOR_RET_SUCCESS) {
        IS_ERR("Gyro request was not processed");
      }
      sn_obj->pend_ts = sn_obj->sof_info.t_end;
      sn_obj->pend_idx = sn_obj->end_idx;
      sn_obj->pstart_idx = sn_obj->start_idx;
      /* set previous frame request processed */
      sn_obj->prev_request_proc = TRUE;
      /* Reset untill a request comes*/
      sn_obj->sof_info.t_end = 0;
      /* check if there is any new request and update the timestamps */
      if (sn_obj->sof_request) {
        sn_obj->sof_info.id = sn_obj->sof_info_next.id;
        sn_obj->sof_info.t_start = sn_obj->sof_info_next.t_start;
        sn_obj->sof_info.t_end = sn_obj->sof_info_next.t_end;
        sn_obj->sof_request = FALSE;
        sn_obj->prev_request_proc = FALSE;
      }
      pthread_mutex_unlock(&(sn_obj->sof_req_mutex));
     }
   }
     ++sn_obj->buf_pos;
  return SENSOR_RET_SUCCESS;
}

/*===========================================================================
 * FUNCTION      sensor_native_event_handler
 *    @fd: file descriptor
 *    @events: Bitmask of the poll events
 *    @data: data pointer for handler
 *
 * DESCRIPTION   looper callback function called by looper thread
 *                        if there are any events available from the sensor
 *=========================================================================*/
int sensor_native_event_handler(int fd, int events, void* data)
{
  STATS_UNUSED(fd);
  STATS_UNUSED(events);
  ASensorEvent event;
  sensor_native_type *sn_obj = (sensor_native_type *)data;
  sensor_cb_struct_type cb_event_data;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t recv_time = ((int64_t)tv.tv_sec * 1000000) + tv.tv_usec;
  /* get the time stamp correction factor for every ten events */
  if (sn_obj->tc_counter % 10 == 0) {
    timestamp_correction(&sn_obj->time_correction);
  }
  sn_obj->tc_counter++;
  IS_LOW("Sensor Event handler!");
  if (events == ALOOPER_EVENT_INPUT) {
    /* Get events from the queue one by one */
    while (ASensorEventQueue_getEvents(sn_obj->event_queue, &event, 1) > 0) {
      switch (event.type) {
        case ASENSOR_TYPE_ACCELEROMETER: {
          event.timestamp = event.timestamp/1000 - sn_obj->time_correction;
          IS_LOW("Event type - Accelerometer!");
          IS_LOW("Accel: evt_timestamp:%lld ns recv_timestamp: %lld ns"
            "x:%f y:%f z:%f",
            event.timestamp, recv_time,
            event.acceleration.x, event.acceleration.y,
            event.acceleration.z);
          /* callback the gyro port notifying about the accelerometer */
          cb_event_data.type = SENSOR_DATA;
          cb_event_data.u.sensor_data.type = SENSOR_ACCELEROMETER;
          cb_event_data.u.sensor_data.timestamp = event.timestamp;
          cb_event_data.u.sensor_data.u.accel.x = event.acceleration.x;
          cb_event_data.u.sensor_data.u.accel.y = event.acceleration.y;
          cb_event_data.u.sensor_data.u.accel.z = event.acceleration.z;
          pthread_mutex_lock(&(sn_obj->cb_mutex));
          if (sn_obj->accel.enabled) {
            sn_obj->data_cb(sn_obj->cb_hndl, cb_event_data);
          }
          pthread_mutex_unlock(&(sn_obj->cb_mutex));
        }
        break;
        case ASENSOR_TYPE_GYROSCOPE: {
          sensor_data_type sensor_data;
          /* Time correction to make it in complaince with dsps */
          event.timestamp = event.timestamp / 1000 - sn_obj->time_correction;
          IS_LOW("Event type - Gyroscope!");
          IS_LOW("Gyro: evt_timestamp:%lld us recv_timestamp: %lld us diff:"
            "%lld x:%f y:%f z:%f",
            event.timestamp, recv_time,
            (recv_time - event.timestamp),
            event.uncalibrated_gyro.x_uncalib,
            event.uncalibrated_gyro.y_uncalib,
            event.uncalibrated_gyro.z_uncalib);
          sensor_data.type = SENSOR_GYRO;
          sensor_data.timestamp = event.timestamp;
          sensor_data.u.gyro.x = event.uncalibrated_gyro.x_uncalib;
          sensor_data.u.gyro.y = event.uncalibrated_gyro.y_uncalib;
          sensor_data.u.gyro.z = event.uncalibrated_gyro.z_uncalib;
          IS_LOW("start:%llu end:%llu",
            sn_obj->sof_info.t_start,
            sn_obj->sof_info.t_end);
          /* Return all data once per frame */
          sensor_native_enqueue(sn_obj, sensor_data);
        }
        break;

        case ASENSOR_TYPE_TIME_OF_FLIGHT: {
          event.timestamp = (event.timestamp / 1000) - sn_obj->time_correction;
          IS_LOW("Event type - Time of Flight!");
          IS_LOW("Laser:\n evt_timestamp:%lld ns\nrecv_timestamp: %lld ns"
            " version: %f mm\n type: %f\n distance: %fmm\n"
            "confidence: %f\n near_limit: %f\n far_limit: %f\n",
            event.timestamp, recv_time,
            event.data[0], event.data[1], event.data[2], event.data[3],
            event.data[4], event.data[5]);

          cb_event_data.type = SENSOR_DATA;
          cb_event_data.u.sensor_data.type = SENSOR_TIME_OF_FLIGHT;
          cb_event_data.u.sensor_data.timestamp = event.timestamp;
          cb_event_data.u.sensor_data.u.laser.version = event.data[0];
          cb_event_data.u.sensor_data.u.laser.type = event.data[1];
          cb_event_data.u.sensor_data.u.laser.distance = event.data[2];
          cb_event_data.u.sensor_data.u.laser.confidence = event.data[3];
          cb_event_data.u.sensor_data.u.laser.near_limit = event.data[4];
          cb_event_data.u.sensor_data.u.laser.far_limit = event.data[5];
          cb_event_data.u.sensor_data.u.laser.max_range = sn_obj->laser.max_range;
          /* callback the gyro port notifying about the tof*/
          pthread_mutex_lock(&(sn_obj->cb_mutex));
          if (sn_obj->laser.enabled) {
            sn_obj->data_cb(sn_obj->cb_hndl, cb_event_data);
          }
          pthread_mutex_unlock(&(sn_obj->cb_mutex));
        }
        break;

        case ASENSOR_TYPE_GRAVITY: {
          event.timestamp = (event.timestamp / 1000) - sn_obj->time_correction;
          IS_LOW("Event type - Gravity!");
          IS_LOW("Gravity: evt_timestamp:%lld ns recv_timestamp: %lld ns"
            "x:%f y:%f z:%f",
             event.timestamp, recv_time,
             event.acceleration.x, event.acceleration.y,
             event.acceleration.z);
          cb_event_data.type = SENSOR_DATA;
          cb_event_data.u.sensor_data.type = SENSOR_GRAVITY;
          cb_event_data.u.sensor_data.timestamp = event.timestamp;
          cb_event_data.u.sensor_data.u.gravity.x = event.acceleration.x;
          cb_event_data.u.sensor_data.u.gravity.y = event.acceleration.y;
          cb_event_data.u.sensor_data.u.gravity.z = event.acceleration.z;
          /* callback the gyro port notifying about the gravity*/
          pthread_mutex_lock(&(sn_obj->cb_mutex));
          if (sn_obj->gravity.enabled) {
            sn_obj->data_cb(sn_obj->cb_hndl, cb_event_data);
          }
          pthread_mutex_unlock(&(sn_obj->cb_mutex));
        }
        break;
        case ASENSOR_TYPE_LINEAR_ACCELERATION: {
          event.timestamp = (event.timestamp / 1000) - sn_obj->time_correction;
          IS_LOW("Event type - Linear accel!");
          IS_LOW("Linear accel: evt_timestamp:%lld ns recv_timestamp: %lld ns"
            "x:%f y:%f z:%f",
             event.timestamp, recv_time,
             event.acceleration.x, event.acceleration.y,
             event.acceleration.z);
          cb_event_data.type = SENSOR_DATA;
          cb_event_data.u.sensor_data.type = SENSOR_LINEAR_ACCELERATION;
          cb_event_data.u.sensor_data.timestamp = event.timestamp;
          cb_event_data.u.sensor_data.u.lin_accel.x = event.acceleration.x;
          cb_event_data.u.sensor_data.u.lin_accel.y = event.acceleration.y;
          cb_event_data.u.sensor_data.u.lin_accel.z = event.acceleration.z;
          /* callback the gyro port notifying about the gravity*/
          pthread_mutex_lock(&(sn_obj->cb_mutex));
          if (sn_obj->lin_accel.enabled) {
            sn_obj->data_cb(sn_obj->cb_hndl, cb_event_data);
          }
          pthread_mutex_unlock(&(sn_obj->cb_mutex));
        }
          break;
        case ASENSOR_TYPE_LIGHT: {
          event.timestamp = (event.timestamp / 1000) - sn_obj->time_correction;
          IS_LOW("Event type - Light!");
          IS_LOW("Light: evt_timestamp:%lld ns recv_timestamp: %lld ns"
            "lux:%f",
             event.timestamp, recv_time,
             event.light);
          cb_event_data.type = SENSOR_DATA;
          cb_event_data.u.sensor_data.type = SENSOR_LIGHT;
          cb_event_data.u.sensor_data.timestamp = event.timestamp;
          cb_event_data.u.sensor_data.u.light.lux = event.light;
          /* callback the gyro port notifying about the gravity*/
          pthread_mutex_lock(&(sn_obj->cb_mutex));
          if (sn_obj->light.enabled) {
            sn_obj->data_cb(sn_obj->cb_hndl, cb_event_data);
          }
          pthread_mutex_unlock(&(sn_obj->cb_mutex));
        }
          break;
      default:
        IS_LOW("Event for sensor(type: %d) not interested.", event.type);
        break;
      }
    }
  } else {
    IS_ERR("Error in sensor manager service, kill the thread" );
    sn_obj->thread_status |= SENSOR_NATIVE_THREAD_ERROR;
    sn_obj->first_threadflag = FALSE;
    if (sn_obj->looper) {
       IS_LOW("Waking up looper thread!");
      ALooper_wake(sn_obj->looper);
    }
     IS_LOW("Waiting for thread to join!");
    pthread_join(sn_obj->thread_id, NULL);
    IS_LOW("thread joined successfully");
  }

  /* return 1 to continue receiving callbacks or
     0 to unregister callback from the looper */
  return 1;
}

/*===========================================================================
 * FUNCTION      sensor_native_thread_handler
 *    @arg: Arguments
 *
 * DESCRIPTION :  Sensor thread handler function
 *                          - Create sensor manager instance
 *                          - Look for available sensors to verify presence
 *                          - Create ALooper and ASensor event queue
 *                          - Disable sensors at ALooper exit
 *=========================================================================*/
static void* sensor_native_thread_handler(void *arg)
{
  sensor_native_type *sn_obj = (sensor_native_type *)arg;
  sensor_cb_struct_type cb_event_data;
  sensor_list_type sensor_list;

  memset(&sensor_list, 0, sizeof(sensor_list_type));
  sensor_list.num_of_sensors = 0;

  pthread_setname_np(sn_obj->thread_id, "CAM_gyro_sens");
  IS_LOW("Started sensor thread handler!");
  IS_LOW("Acquiring instance of sensor manager");
  /* Create instance of sensor manager */
  sn_obj->smgr = sensor_service_get_object();

  /* Get default Gyro sensor */
  IS_LOW("Get Default Gyro sensor!");
  sn_obj->gyro.sensor =
    ASensorManager_getDefaultSensor(sn_obj->smgr, ASENSOR_TYPE_GYROSCOPE);
  if (!sn_obj->gyro.sensor) {
    IS_LOW("Gyro not found!");
    sn_obj->gyro.present = FALSE;
  } else {
    IS_LOW("Gyro Sensor: %s found. Vendor: %s!",
      ASensor_getName(sn_obj->gyro.sensor),
      ASensor_getVendor(sn_obj->gyro.sensor));
    /* record gyro available */
    sn_obj->gyro.present = TRUE;
    sensor_list.available_sensors[sensor_list.num_of_sensors] =
      SENSOR_GYRO;
    sensor_list.num_of_sensors++;

    /* get minimum delay allowed between events in us. A value of zero
       means event is reported only when available */
    sn_obj->gyro.min_delay = ASensor_getMinDelay(sn_obj->gyro.sensor);
    sn_obj->gyro.sample_rate =
      (sn_obj->gyro.min_delay < DEFAULT_RATE_IN_US) ?
      DEFAULT_RATE_IN_US : sn_obj->gyro.min_delay;
    IS_LOW("Gyro - min_delay: %d sample_rate: %d!",
      sn_obj->gyro.min_delay, sn_obj->gyro.sample_rate);
  }

  /* Get default accelerometer sensor */
  sn_obj->accel.sensor =
    ASensorManager_getDefaultSensor(sn_obj->smgr, ASENSOR_TYPE_ACCELEROMETER);
  if (!sn_obj->accel.sensor) {
    IS_ERR("Accelerometer not found!");
    sn_obj->accel.present = FALSE;
  } else {
    IS_LOW("Accelerometer Sensor: %s found. Vendor: %s!",
      ASensor_getName(sn_obj->accel.sensor),
      ASensor_getVendor(sn_obj->accel.sensor));
    /* Enable it if accelerometer data is needed */
    sn_obj->accel.present = TRUE;
    sensor_list.available_sensors[sensor_list.num_of_sensors] =
      SENSOR_ACCELEROMETER;
    sensor_list.num_of_sensors++;

    sn_obj->accel.min_delay =
      ASensor_getMinDelay(sn_obj->accel.sensor);
    sn_obj->accel.sample_rate =
      (sn_obj->accel.min_delay < DEFAULT_RATE_IN_US_ACCEL) ?
      DEFAULT_RATE_IN_US_ACCEL : sn_obj->accel.min_delay;
    IS_LOW("Accelerometer - min_delay: %d sample_rate: %d!",
      sn_obj->accel.min_delay, sn_obj->accel.sample_rate);
  }

  /* Get default gravity sensor */
  sn_obj->gravity.sensor =
    ASensorManager_getDefaultSensor(sn_obj->smgr, ASENSOR_TYPE_GRAVITY);
  if (!sn_obj->gravity.sensor) {
    IS_ERR("Gravity sensor not found!");
    sn_obj->gravity.present = FALSE;
  } else {
    IS_LOW("Gravity Sensor: %s found. Vendor: %s!",
      ASensor_getName(sn_obj->gravity.sensor),
      ASensor_getVendor(sn_obj->gravity.sensor));
    /* Enable it if gravity data is needed */
    sn_obj->gravity.present = TRUE;
    sensor_list.available_sensors[sensor_list.num_of_sensors] =
      SENSOR_GRAVITY;
    sensor_list.num_of_sensors++;

    sn_obj->gravity.min_delay =
      ASensor_getMinDelay(sn_obj->gravity.sensor);
    sn_obj->gravity.sample_rate =
      (sn_obj->gravity.min_delay < DEFAULT_RATE_IN_US_ACCEL) ?
      DEFAULT_RATE_IN_US_ACCEL : sn_obj->gravity.min_delay;
    IS_LOW("Gravity - min_delay: %d sample_rate: %d!",
      sn_obj->gravity.min_delay, sn_obj->gravity.sample_rate);
  }
  /* Get default linear accelerometer sensor */
  sn_obj->lin_accel.sensor = ASensorManager_getDefaultSensor(sn_obj->smgr,
      ASENSOR_TYPE_LINEAR_ACCELERATION);
  if (!sn_obj->lin_accel.sensor) {
    IS_ERR("Linear Acceleration not found!");
    sn_obj->lin_accel.present = FALSE;
  } else {
    IS_LOW("Linear Accelerometer Sensor: %s found. Vendor: %s!",
      ASensor_getName(sn_obj->lin_accel.sensor),
      ASensor_getVendor(sn_obj->lin_accel.sensor));
    /* Enable it if accelerometer data is needed */
    sn_obj->lin_accel.present = TRUE;
    sensor_list.available_sensors[sensor_list.num_of_sensors] =
      SENSOR_LINEAR_ACCELERATION;
    sensor_list.num_of_sensors++;

    sn_obj->lin_accel.min_delay =
      ASensor_getMinDelay(sn_obj->lin_accel.sensor);
    sn_obj->lin_accel.sample_rate =
      (sn_obj->lin_accel.min_delay < DEFAULT_RATE_IN_US_ACCEL) ?
      DEFAULT_RATE_IN_US_ACCEL : sn_obj->lin_accel.min_delay;
    IS_LOW("Linear Accelerometer - min_delay: %d sample_rate: %d!",
      sn_obj->lin_accel.min_delay, sn_obj->lin_accel.sample_rate);
  }

  /* Get default laser sensor */
  sn_obj->laser.sensor = ASensorManager_getDefaultSensor(sn_obj->smgr,
      ASENSOR_TYPE_TIME_OF_FLIGHT);
  if (!sn_obj->laser.sensor) {
    IS_ERR("Laser Sensor not found!");
    sn_obj->laser.present = FALSE;
  } else {
    IS_LOW("Laser Sensor: %s found. Vendor: %s!",
      ASensor_getName(sn_obj->laser.sensor),
      ASensor_getVendor(sn_obj->laser.sensor));
    /* Enable it if accelerometer data is needed */
    sn_obj->laser.present = TRUE;
    sensor_list.available_sensors[sensor_list.num_of_sensors] =
      SENSOR_TIME_OF_FLIGHT;
    sensor_list.num_of_sensors++;

    sn_obj->laser.min_delay =
      ASensor_getMinDelay(sn_obj->laser.sensor);
    /* Get maximum range of the sensor */
    sn_obj->laser.max_range =
      ASensor_getMaxRange(sn_obj->laser.sensor);
    IS_LOW("Laser Sensor - min_delay: %d max_range: %d!",
      sn_obj->laser.min_delay, sn_obj->laser.max_range);
  }

  /* Get default Light sensor */
  sn_obj->light.sensor = ASensorManager_getDefaultSensor(sn_obj->smgr,
      ASENSOR_TYPE_LIGHT);
  if (!sn_obj->light.sensor) {
    IS_ERR("Light Sensor not found!");
    sn_obj->light.present = FALSE;
  } else {
    IS_LOW("Light Sensor: %s found. Vendor: %s!",
      ASensor_getName(sn_obj->light.sensor),
      ASensor_getVendor(sn_obj->light.sensor));
    /* Enable it if light data is needed */
    sn_obj->light.present = TRUE;
    sensor_list.available_sensors[sensor_list.num_of_sensors] =
      SENSOR_LIGHT;
    sensor_list.num_of_sensors++;

    sn_obj->light.min_delay =
      ASensor_getMinDelay(sn_obj->light.sensor);
    /* Get maximum range of the sensor */
    sn_obj->light.max_range =
      ASensor_getMaxRange(sn_obj->light.sensor);
    sn_obj->light.sample_rate =
      (sn_obj->light.sample_rate < DEFAULT_RATE_IN_US_LIGHT) ?
      DEFAULT_RATE_IN_US_LIGHT : sn_obj->light.sample_rate;
    IS_LOW("%s:Light - min_delay: %d sample_rate: %d!",__func__,
      sn_obj->light.min_delay, sn_obj->light.sample_rate);
  }

  /* Send list of available sensors */
  cb_event_data.type = SENSOR_LIST;
  cb_event_data.u.sensor_list = sensor_list;
  pthread_mutex_lock(&(sn_obj->cb_mutex));
  sn_obj->data_cb(sn_obj->cb_hndl, cb_event_data);

  /* Create looper thread */
  sn_obj->looper = ALooper_forThread();
  if(!sn_obj->looper) {
    IS_LOW("Creating new looper thread!");
    sn_obj->looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
  }

  IS_LOW("Create new event queue!");
  sn_obj->event_queue = ASensorManager_createEventQueue(
    sn_obj->smgr,
    sn_obj->looper,
    0/*LOOPER_POLL_CALLBACK*/,
    sensor_native_event_handler,
    (void *)sn_obj);
  pthread_mutex_unlock(&(sn_obj->cb_mutex));
  /* sensors will be enabled from gyro port only after event queue*/

  do {
    /* poll the looper.
       If the timeout is zero, returns immediately without blocking.
       If negative, waits indefinitely until an event appears.
        Time out : 100 ms */

    IS_LOW("Polling the looper!");
    int ident = ALooper_pollOnce(100, NULL, NULL, NULL);
    IS_LOW("Looper thread returned: %d", ident);
    boolean exit_loop = FALSE;
    switch (ident) {
      case ALOOPER_POLL_WAKE:
        IS_LOW("Poll was awoken using wake before timeout.");
        exit_loop = FALSE;
        break;
      case ALOOPER_POLL_CALLBACK:
        IS_LOW("One or more callbacks executed");
        exit_loop = FALSE;
        break;
      case ALOOPER_POLL_TIMEOUT:
        IS_LOW("Timeout expired!");
        exit_loop = FALSE;
        break;
      case ALOOPER_POLL_ERROR:
        IS_ERR("Error during poll.");
        exit_loop = FALSE;
        sn_obj->thread_status |= SENSOR_NATIVE_THREAD_ERROR;
        break;
      default:
        IS_ERR("Ident retured by poll: %d", ident);
        exit_loop = FALSE;
        break;
    }

    if (exit_loop) {
      IS_LOW("Breaking out of do-while loop.");
      break;
    }
  } while (sensor_native_get_thread_status(sn_obj));

  /* DO not kill the first thread becasue of sensormanager issue */
  /* Issue link : https://code.google.com/p/android/issues/detail?id=80212 */
  if (sn_obj->first_threadflag == TRUE) {
    wait_loop(sn_obj);
  }

  /* Exiting the thread - do necessary stuff */
  /* Sensors will be disabled in deinit*/
  /*no need to call alooper_wake in deinit when crtl is here*/
  pthread_mutex_lock(&(sn_obj->cb_mutex));

  /* Destroy event queue */
  IS_LOW("Destroy event queue!");
  ASensorManager_destroyEventQueue(sn_obj->smgr, sn_obj->event_queue);

#ifndef UPGRADE_SENSOR_MANAGER
  /* Delete service manager object */
  sensor_service_destroy(sn_obj->smgr);
#endif
  sn_obj->looper = NULL;
  sn_obj->smgr = NULL;
  sn_obj->event_queue = NULL;
  pthread_mutex_unlock(&(sn_obj->cb_mutex));
  IS_LOW("Destroyed sensor service");

  /* Free the object if signaled by deinit*/
  if(sn_obj->thread_status & SENSOR_NATIVE_THREAD_OFF) {
    /* Free the object as camera session closed*/
    IS_INFO("camera session close, exit the thread; free obj");
    pthread_mutex_destroy(&sn_obj->cb_mutex);
    free(sn_obj);
  } else {
    /* This is error case, deinit will free the object */
    IS_ERR("Error in sensor service manager, free in deinit");
  }

  return NULL;
}


/*===========================================================================
 * FUNCTION      sensor_native_enable_req
 *    @sn_obj: handler of native interface
 *    @sensor: sensor type
 *
 * DESCRIPTION : Request to enable native sensor
 *=========================================================================*/
static sensor_return_type sensor_native_enable_req(void *handle,
  sensor_type sensor)
{
  sensor_native_type *sn_obj = (sensor_native_type *)handle;
  int rc = SENSOR_RET_SUCCESS;
  ASensorRef android_sensor = NULL;
  int32_t sample_rate = 0;
  boolean *enabled_success = NULL;

  switch (sensor) {
  case SENSOR_GYRO: {
    if (sn_obj->gyro.present) {
      IS_LOW("Enable Gyro sensor and set default rate!");
      android_sensor = sn_obj->gyro.sensor;
      sample_rate = sn_obj->gyro.sample_rate;
      enabled_success = &sn_obj->gyro.enabled;
    } else {
      IS_ERR("Try to enable gyro but not present");
      rc = SENSOR_RET_UNKNOWN;
    }
  }
    break;
  case SENSOR_GRAVITY: {
    if (sn_obj->gravity.present) {
      IS_LOW("Enable gravity sensor and set default rate!");
      android_sensor = sn_obj->gravity.sensor;
      sample_rate = sn_obj->gravity.sample_rate;
      enabled_success = &sn_obj->gravity.enabled;
    } else {
      IS_ERR("Try to enable gravity but not present");
      rc = SENSOR_RET_UNKNOWN;
    }
  }
    break;
  case SENSOR_ACCELEROMETER: {
    if (sn_obj->accel.present) {
      IS_LOW("Enable Accel sensor and set default rate!");
      android_sensor = sn_obj->accel.sensor;
      sample_rate = sn_obj->accel.sample_rate;
      enabled_success = &sn_obj->accel.enabled;
    } else {
      IS_ERR("Try to enable accelerometer but not present");
      rc = SENSOR_RET_UNKNOWN;
    }
  }
    break;
  case SENSOR_LINEAR_ACCELERATION: {
    if (sn_obj->lin_accel.present) {
      IS_LOW("Enable linear acceleration sensor and set default rate!");
      android_sensor = sn_obj->lin_accel.sensor;
      sample_rate = sn_obj->lin_accel.sample_rate;
      enabled_success = &sn_obj->lin_accel.enabled;
    } else {
      IS_ERR("Try to enable linear acceleration but not present");
      rc = SENSOR_RET_UNKNOWN;
    }
  }
    break;
  case SENSOR_TIME_OF_FLIGHT: {
    if (sn_obj->laser.present) {
      IS_LOW("Enable laser sensor and set default rate!");
      android_sensor = sn_obj->laser.sensor;
      sample_rate = sn_obj->laser.sample_rate;
      enabled_success = &sn_obj->laser.enabled;
    } else {
      IS_ERR("Try to enabl laser sensor but not present");
      rc = SENSOR_RET_UNKNOWN;
    }
  }
    break;
  case SENSOR_LIGHT: {
    if (sn_obj->light.present) {
      IS_LOW("Enable light sensor and set default rate!");
      android_sensor = sn_obj->light.sensor;
      sample_rate = sn_obj->light.sample_rate;
      enabled_success = &sn_obj->light.enabled;
    } else {
      IS_ERR("Try to enable light but not present");
      rc = SENSOR_RET_UNKNOWN;
    }
  }
    break;
  default:
    IS_ERR("Unkown sensor type: %d", sensor);
    rc = SENSOR_RET_BAD_PARAMS;
    break;
  }

  /* Calls to android sensor interface to perform operations */
  if (rc == SENSOR_RET_SUCCESS) {
    if (enabled_success) {
      /* enable the sensor only after eventqueue is created*/
      pthread_mutex_lock(&(sn_obj->cb_mutex));
      if(sn_obj->event_queue) {
        rc = ASensorEventQueue_enableSensor(sn_obj->event_queue, android_sensor);
      } else {
        rc = -1;
      }
      pthread_mutex_unlock(&(sn_obj->cb_mutex));
      if (rc < 0) {
        IS_ERR("Fail to enable sensor");
      } else {
        pthread_mutex_lock(&(sn_obj->cb_mutex));
        *enabled_success = TRUE;
        pthread_mutex_unlock(&(sn_obj->cb_mutex));
        rc = ASensorEventQueue_setEventRate(sn_obj->event_queue, android_sensor,
          sample_rate);
        if (rc < 0) {
          IS_ERR("Fail to set event rate");
          ASensorEventQueue_disableSensor(sn_obj->event_queue,android_sensor);
        }
      }
    } else {
      IS_ERR("Fail to init param enabled_success");
      rc = SENSOR_RET_BAD_PARAMS;
    }
  }

  if (rc < 0)
    rc = SENSOR_RET_FAILURE;

  return rc;
}

/*===========================================================================
 * FUNCTION      sensor_native_disable_req
 *    @sn_obj: handler of native interface
 *    @sensor: sensor type
 *
 * DESCRIPTION : Request to disable native sensor
 *=========================================================================*/
static sensor_return_type sensor_native_disable_req(void *handle,
  sensor_type sensor)
{
  int rc = SENSOR_RET_SUCCESS;
  sensor_native_type *sn_obj = (sensor_native_type *)handle;
  switch (sensor) {
  case SENSOR_GYRO: {
    if (sn_obj->gyro.enabled) {
      IS_LOW("Disable Gyro Sensor!");
      pthread_mutex_lock(&(sn_obj->cb_mutex));
      sn_obj->gyro.enabled = FALSE;
      pthread_mutex_unlock(&(sn_obj->cb_mutex));
      if(sn_obj->event_queue && sn_obj->gyro.sensor) {
        rc = ASensorEventQueue_disableSensor(sn_obj->event_queue,
          sn_obj->gyro.sensor);
      } else {
        IS_ERR("Stype:%d;Sensor event queue was already destroyed",sensor);
        rc = SENSOR_RET_BAD_PARAMS;
      }
    }
  }
  break;
  case SENSOR_GRAVITY: {
    if (sn_obj->gravity.enabled) {
      IS_LOW("Disable Gravity Sensor!");
      pthread_mutex_lock(&(sn_obj->cb_mutex));
      sn_obj->gravity.enabled = FALSE;
      pthread_mutex_unlock(&(sn_obj->cb_mutex));
      if(sn_obj->event_queue && sn_obj->gravity.sensor) {
        rc = ASensorEventQueue_disableSensor(sn_obj->event_queue,
          sn_obj->gravity.sensor);
      } else {
        IS_ERR("Stype:%d;Sensor event queue was already destroyed",sensor);
        rc = SENSOR_RET_BAD_PARAMS;
      }
    }
  }
  break;
  case SENSOR_ACCELEROMETER: {
    if (sn_obj->accel.enabled) {
      IS_LOW("Disable Acceleroemter Sensor!");
      pthread_mutex_lock(&(sn_obj->cb_mutex));
      sn_obj->accel.enabled = FALSE;
      pthread_mutex_unlock(&(sn_obj->cb_mutex));
      if(sn_obj->event_queue && sn_obj->accel.sensor) {
        rc = ASensorEventQueue_disableSensor(sn_obj->event_queue,
         sn_obj->accel.sensor);
      } else {
        IS_ERR("Stype:%d;Sensor event queue was already destroyed",sensor);
        rc = SENSOR_RET_BAD_PARAMS;
      }
    }
  }
  break;
  case SENSOR_LINEAR_ACCELERATION: {
    if (sn_obj->lin_accel.enabled) {
      IS_LOW("Disable Linear accel Sensor!");
      pthread_mutex_lock(&(sn_obj->cb_mutex));
      sn_obj->lin_accel.enabled = FALSE;
      pthread_mutex_unlock(&(sn_obj->cb_mutex));
      if(sn_obj->event_queue && sn_obj->lin_accel.sensor) {
        rc = ASensorEventQueue_disableSensor(sn_obj->event_queue,
          sn_obj->lin_accel.sensor);
      } else {
        IS_ERR("Stype:%d;Sensor event queue was already destroyed",sensor);
        rc = SENSOR_RET_BAD_PARAMS;
      }
    }
  }
  break;
  case SENSOR_TIME_OF_FLIGHT: {
    if (sn_obj->laser.enabled) {
      IS_LOW("Disable Laser Sensor!");
      pthread_mutex_lock(&(sn_obj->cb_mutex));
      sn_obj->laser.enabled = FALSE;
      pthread_mutex_unlock(&(sn_obj->cb_mutex));
      if(sn_obj->event_queue && sn_obj->laser.sensor) {
        rc = ASensorEventQueue_disableSensor(sn_obj->event_queue,
         sn_obj->laser.sensor);
      } else {
        IS_ERR("Stype:%d;Sensor event queue was already destroyed",sensor);
        rc = SENSOR_RET_BAD_PARAMS;
      }

    }
  }
  break;
  case SENSOR_LIGHT: {
    if (sn_obj->light.enabled) {
      IS_LOW("Disable Light Sensor!");
      pthread_mutex_lock(&(sn_obj->cb_mutex));
      sn_obj->light.enabled = FALSE;
      pthread_mutex_unlock(&(sn_obj->cb_mutex));
      if(sn_obj->event_queue && sn_obj->light.sensor) {
        rc = ASensorEventQueue_disableSensor(sn_obj->event_queue,
          sn_obj->light.sensor);}
      else {
        IS_ERR("Stype:%d;Sensor event queue was already destroyed",sensor);
        rc = SENSOR_RET_BAD_PARAMS;
      }
    }
  }
    break;
  default:
    IS_ERR("Unkown sensor type: %d", sensor);
    rc = SENSOR_RET_BAD_PARAMS;
  break;
  }

  if (rc < 0)
    rc = SENSOR_RET_FAILURE;

  return rc;
}


/*===========================================================================
 * FUNCTION      sensor_native_set_params
 *    @handle: Handler
 *    @set_data : set/req data info
 *
 * DESCRIPTION : sets/config the sensor data.
 *=========================================================================*/

sensor_return_type sensor_native_set_params(
  void *handle, sensor_set_data_type set_data)
{
  sensor_return_type rc = SENSOR_RET_SUCCESS;
  sensor_native_type *sn_obj = (sensor_native_type *)handle;
  if (!sn_obj->initialized) {
    IS_HIGH("sensor object not initialized!");
    return rc;
  }
  switch (set_data.msg_type) {
  case SENSOR_SET_DATA_CONFIG:
    if (set_data.sensor == SENSOR_GYRO) {
      sensor_data_range_type data_range = set_data.u.data_range;
      pthread_mutex_lock(&(sn_obj->sof_req_mutex));
      /*If previous request is processed by now, directly update the current request params*/
      /*else enqueue it to be next request by updating next request params*/
      if (sn_obj->prev_request_proc) {
        IS_LOW("sof_event :%d", data_range.id);
        sn_obj->sof_info.id = data_range.id;
        sn_obj->sof_info.t_start = data_range.t_start;
        sn_obj->sof_info.t_end = data_range.t_end;
        sn_obj->prev_request_proc = FALSE ;
      } else {
        IS_LOW("New sof_event :%d", data_range.id);
        sn_obj->sof_request = TRUE;
        sn_obj->sof_info_next.id = data_range.id;
        sn_obj->sof_info_next.t_start = data_range.t_start;
        sn_obj->sof_info_next.t_end = data_range.t_end;
      }
      pthread_mutex_unlock(&(sn_obj->sof_req_mutex));
      rc = SENSOR_RET_SUCCESS;
    }
    break;
  case SENSOR_ENABLE_REQ:
    rc = sensor_native_enable_req(handle, set_data.sensor);
    break;
  case SENSOR_DISABLE_REQ:
    rc = sensor_native_disable_req(handle, set_data.sensor);
    break;
  default:
    IS_ERR("Error not handle msg_type: %d", set_data.msg_type);
    rc = SENSOR_RET_BAD_PARAMS;
    break;
  }
  return rc;
}



/*===========================================================================
 * FUNCTION      sensor_native_init
 *    @handle: :Handler
 *    @cb_hndl:Pointer for callback
 *    @cb: data callback
 *
 * DESCRIPTION: Initialization of sensor thread
 *=========================================================================*/
sensor_return_type sensor_native_init(
  void **handle,
  void *cb_hndl,
  sensor_data_cb cb)
{
  sensor_native_type  *sn_obj = NULL;
  IS_LOW("status mul_inst: %d", g_snobj.first_threadflag);
  /*if first thread already exists; go for dynamic objects */
  if (g_snobj.first_threadflag) {
    /* Local object (not first instance) */
    IS_LOW("Alloc for internal structure, mul_inst: %d",
      g_snobj.first_threadflag);
    sn_obj = calloc(1, sizeof(sensor_native_type));
    if (sn_obj == NULL) {
      IS_ERR("Failure allocating memory!");
      return SENSOR_RET_NO_MEMMORY;
    }
    IS_LOW("Alloc for internal structure: %p, mul_inst: %d",
      sn_obj, g_snobj.first_threadflag);
    sn_obj->data_cb = cb;
    sn_obj->cb_hndl = cb_hndl;
    sn_obj->first_threadflag = FALSE;
    /* Time correction between getTimeOfday and systemTime */
    timestamp_correction(&sn_obj->time_correction);
    if (sn_obj->initialized != TRUE) {
      sn_obj->thread_status |= SENSOR_NATIVE_THREAD_ON;
      /* For first frame no previous frame, hence true*/
      sn_obj->prev_request_proc = TRUE;
      /* Initialize thread and mutexes */
      pthread_mutex_init(&sn_obj->sof_req_mutex, NULL);
      pthread_mutex_init(&sn_obj->cb_mutex, NULL);
      pthread_create(&sn_obj->thread_id, NULL, sensor_native_thread_handler,
        (void *)sn_obj);
      IS_LOW("thread id :%lu", sn_obj->thread_id);
      pthread_setname_np(sn_obj->thread_id, "GA2");
      sn_obj->initialized = TRUE;
      pthread_detach(sn_obj->thread_id);
    }
  } else {
    /* First time: use global object g_snobj */
    /* TODO: Remove this else part if google resolves the issue with SM
     * NOTE : The below created thread will not be killed during the entire
     *           target UP time; Consumes memory as we are not clearing up
     *           Need to remove this else part if google resolves the issue.
   Issue link : https://code.google.com/p/android/issues/detail?id=80212
     */
    IS_LOW("Using g_snobj: %p, mul_inst: %d",
      &g_snobj, g_snobj.first_threadflag);
    sn_obj = &g_snobj;
    /*Only first thread enters into else condition; set the flag*/
    sn_obj->first_threadflag = TRUE;

    /* Init native circular buffer */
    memset(&sn_obj->sof_info, 0, sizeof(sensor_data_range_type));
    memset(&sn_obj->cirq_buffer, 0, GYRO_BUFFER_SIZE * sizeof(sensor_data_type));
    sn_obj->buf_pos = 0;
    sn_obj->start_idx = 0;
    sn_obj->end_idx = 0;
    sn_obj->pstart_idx = 0;
    sn_obj->pend_idx = 0;
    sn_obj->pend_ts = 0;

    /* For first frame no previous frame; hence true */
    sn_obj->prev_request_proc = TRUE;

    sn_obj->data_cb = cb;
    sn_obj->cb_hndl = cb_hndl;
    /* Time correction between getTimeOfday and systemTime */
    timestamp_correction(&sn_obj->time_correction);
    if (sn_obj->initialized != TRUE) {
      sn_obj->thread_status |= SENSOR_NATIVE_THREAD_ON;
      /* Initialize thread and mutexes */
      pthread_mutex_init(&sn_obj->sof_req_mutex, NULL);
      pthread_mutex_init(&sn_obj->cb_mutex, NULL);
      pthread_create(&sn_obj->thread_id, NULL, sensor_native_thread_handler,
        (void *)sn_obj);
      IS_LOW("thread id :%lu", sn_obj->thread_id);
      pthread_setname_np(sn_obj->thread_id, "GA1");
      sn_obj->initialized = TRUE;
      pthread_detach(sn_obj->thread_id);
    }
  }
  *handle = (void *)sn_obj;
  return SENSOR_RET_SUCCESS;
}

/*===========================================================================
 * FUNCTION      sensor_native_deinit
 *    @handle:Handler
 *
 * DESCRIPTION : Deinitilize sensor thread
 *=========================================================================*/

sensor_return_type sensor_native_deinit(void *handle)
{
  sensor_native_type *sn_obj = (sensor_native_type *)handle;
  IS_LOW("Deinitialize native sensor!");
  if (sn_obj->gyro.sensor && sn_obj->gyro.enabled) {
    IS_LOW("Disable Gyro Sensor!");
    sn_obj->gyro.enabled = FALSE;
    ASensorEventQueue_disableSensor(sn_obj->event_queue,
      sn_obj->gyro.sensor);
  }

  if (sn_obj->accel.sensor && sn_obj->accel.enabled) {
    IS_LOW("Disable Accelerometer Sensor!");
    sn_obj->accel.enabled = FALSE;
    ASensorEventQueue_disableSensor(sn_obj->event_queue,
      sn_obj->accel.sensor);
    sn_obj->accel.enabled = FALSE;
  }

  if (sn_obj->gravity.sensor && sn_obj->gravity.enabled) {
    IS_LOW("Disable Gravity Sensor!");
    sn_obj->accel.enabled = FALSE;
    ASensorEventQueue_disableSensor(sn_obj->event_queue,
      sn_obj->gravity.sensor);
    sn_obj->gravity.enabled = FALSE;
  }

  if (sn_obj->lin_accel.sensor && sn_obj->lin_accel.enabled) {
    IS_LOW("Disable Linear accel Sensor!");
    sn_obj->lin_accel.enabled = FALSE;
    ASensorEventQueue_disableSensor(sn_obj->event_queue,
      sn_obj->lin_accel.sensor);
    sn_obj->lin_accel.enabled = FALSE;
  }
  if (sn_obj->laser.sensor && sn_obj->laser.enabled) {
    IS_LOW("Disable Acceleroemter Sensor!");
    ASensorEventQueue_disableSensor(sn_obj->event_queue,
      sn_obj->laser.sensor);
    sn_obj->laser.enabled = FALSE;
  }

  if (sn_obj->light.sensor && sn_obj->light.enabled) {
    IS_LOW("Disable Light Sensor!");
    ASensorEventQueue_disableSensor(sn_obj->event_queue,
      sn_obj->light.sensor);
    sn_obj->light.enabled = FALSE;
  }

  /* Deinit mutexes and wait for thread to exit */

  /* wake up looper thread so it can exit */
  /* First time donot kill the thread */
  /* TODO:Remove this condition if google resolves the issue and make sure all *
   *               created threads are joined here after google resolves the issue*/
  /*   Issue link : https://code.google.com/p/android/issues/detail?id=80212 */
  sn_obj->initialized = FALSE;
  IS_LOW(" is it second or next thread ?: %d", !sn_obj->first_threadflag);
  pthread_mutex_destroy(&sn_obj->sof_req_mutex);

  /*Signal GA thread to exit and free the object */
  if(!(sn_obj->thread_status & SENSOR_NATIVE_THREAD_ERROR))
  {
     sn_obj->thread_status |= SENSOR_NATIVE_THREAD_OFF;
  } else {
    IS_ERR("Error in sensorservice manager, free the object");
    pthread_mutex_lock(&(sn_obj->cb_mutex));
    if (sn_obj->looper) {
      IS_LOW("Waking up looper thread!");
      ALooper_wake(sn_obj->looper);
    }
    pthread_mutex_unlock(&(sn_obj->cb_mutex));
    /* first thread uses a global object; donot clear first object */
    IS_LOW(" first thread ? : %d", sn_obj->first_threadflag);
    if (!sn_obj->first_threadflag) {
      IS_LOW("Waiting for thread to join!");
      pthread_join(sn_obj->thread_id, NULL);
      pthread_mutex_destroy(&sn_obj->cb_mutex);
      free(sn_obj);
      sn_obj = NULL;
    } else {
      IS_LOW("Keeping thread alive and exit");
    }
  }
  return SENSOR_RET_SUCCESS;
}
#endif /* SENSOR_NATIVE_LAYER*/
