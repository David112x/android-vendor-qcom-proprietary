/* sensor_hidl_intf.c
 *
 * Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#include "sensor_hidl_intf.h"

#ifdef SENSOR_HIDL_INTF
#define MS2US (1000L)

/*===========================================================================
 * FUNCTION   timestamp_correction
 * @tc: Timestamp difference
 *
 * @Descrption: timestamp correction b/w gettimeofday() and elapsedRealtimeNano().
 *                    Correction is required because gyro port requests samples
 *                    using gettimeofday() timestamps but android sensor API uses systemTime
 *=========================================================================*/
static void timestamp_correction(int64_t *tc){
  struct timeval tv;
  *tc = 0;
  gettimeofday(&tv, NULL);
  int64_t sys_time = android::elapsedRealtimeNano()/1000;
  int64_t get_time = ((int64_t)tv.tv_sec * 1000000) + tv.tv_usec;
  *tc = sys_time - get_time;
  IS_LOW("sys_time : %lld - gettime: %lld = time_correction: %lld",
    sys_time, get_time, *tc);
}


/*===========================================================================
 * FUNCTION      sensor_hidl_callback::sensor_hidl_callback
 *
 * DESCRIPTION : constructor for sensor_hidl_callback class
 *=========================================================================*/
sensor_hidl_callback::sensor_hidl_callback():state(STATE_INACTIVE){
}


/*===========================================================================
 * FUNCTION      sensor_hidl_callback::~sensor_hidl_callback
 *
 * DESCRIPTION : Destructor for sensor_hidl_intf class
 *=========================================================================*/
sensor_hidl_callback::~sensor_hidl_callback(){}


/*===========================================================================
 * FUNCTION     sensor_hidl_callback::onEvent
 *    @e: callback event
 *
 * DESCRIPTION : callback function registered on eventqueue.
 *                       receives events and calls the processEvent.
 *=========================================================================*/
Return<void> sensor_hidl_callback::onEvent(const Event &e) {
  sensor_hidl_intf* s_obj = (sensor_hidl_intf*)this->sensor_hidl_intf_obj;
  if(s_obj && this->state == STATE_ACTIVE){
    s_obj->processEvent(e);
  } else {
    IS_LOW("Not interested in event as obj is deleted or state is inactive");
  }
  return Void();
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf::sensor_hidl_intf
 *
 * DESCRIPTION : constructor for sensor_hidl_intf class where initialization of private members are done
 *=========================================================================*/
sensor_hidl_intf::sensor_hidl_intf(){
  event_q_= nullptr;
  memset(&list_info,0,sizeof(sensor_hidl_list_info));
  /* For first frame no previous frame, hence true*/
  prev_request_proc = TRUE;
  /* Initialize thread and mutexes */
  pthread_mutex_init(&sof_req_mutex, NULL);
  pthread_mutex_init(&cb_mutex, NULL);
  buf_pos = 0;
  start_idx = 0;
  end_idx = 0;
  pstart_idx = 0;
  pend_idx = 0;
  pend_ts = 0;
  tc_counter = 0;
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  prev_request_proc_accel = TRUE;
  pthread_mutex_init(&sof_req_mutex_accel, NULL);
  buf_pos_accel = 0;
  start_idx_accel = 0;
  end_idx_accel = 0;
  pstart_idx_accel = 0;
  pend_idx_accel = 0;
  pend_ts_accel = 0;
#endif
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf::~sensor_hidl_intf
 *
 * DESCRIPTION : Destructor for sensor_hidl_intf class
 *=========================================================================*/
sensor_hidl_intf::~sensor_hidl_intf(){
  pthread_mutex_destroy(&sof_req_mutex);
  pthread_mutex_destroy(&cb_mutex);
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  pthread_mutex_destroy(&sof_req_mutex_accel);
#endif
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf::processEvent
 *    @e: callback event
 *
 * DESCRIPTION : callback function registered on eventqueue.
 *                       receives and processes the events
 *=========================================================================*/
Return<void> sensor_hidl_intf::processEvent(const Event &e) {
  sensor_hidl_list_info *list_info= &this->list_info;
  sensor_cb_struct_type cb_event_data;
  sensor_data_type sensor_data;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int64_t recv_time = ((int64_t)tv.tv_sec * 1000000) + tv.tv_usec;
  /* get the time stamp correction factor for every ten events */
  if (this->tc_counter % 10 == 0) {
    timestamp_correction(&this->time_correction);
  }
  this->tc_counter++;

  if(!e.sensorHandle) {
    IS_ERR("invalid sensor handle, ignore");
    return Void();
  } else {
    /* Time correction to make it in compliance with dsps */
    sensor_data.timestamp = e.timestamp / 1000 - this->time_correction;
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
    sensor_data.is_timestamp = e.timestamp;
#endif
    switch (e.sensorType) {
    case SensorType::GYROSCOPE: {
      sensor_data.type = SENSOR_GYRO;
      sensor_data.u.gyro.x = e.u.uncal.x;
      sensor_data.u.gyro.y = e.u.uncal.y;
      sensor_data.u.gyro.z = e.u.uncal.z;
      IS_LOW("Event type - gyro!");
      IS_LOW("Gyro: evt_ts:%lld us rec_ts:%lld us x:%f y:%f z:%f",
        sensor_data.timestamp,
        recv_time,
        e.u.uncal.x,
        e.u.uncal.y,
        e.u.uncal.z);
      /* Enqueue gyro data and return all data as per request made(t_start-t_end)*/
      sensor_hidl_enqueue(sensor_data);
    }
    break;

    case SensorType::ACCELEROMETER: {
      cb_event_data.type = SENSOR_DATA;
      cb_event_data.u.sensor_data.type = SENSOR_ACCELEROMETER;
      cb_event_data.u.sensor_data.timestamp = sensor_data.timestamp;
      cb_event_data.u.sensor_data.u.accel.x = e.u.uncal.x;
      cb_event_data.u.sensor_data.u.accel.y = e.u.uncal.y;
      cb_event_data.u.sensor_data.u.accel.z = e.u.uncal.z;
      IS_LOW("Event type - Accel!");
      IS_LOW("Accel: evt_timestamp:%lld us rec_ts:%lld us x:%f y:%f z:%f",
        sensor_data.timestamp,
        recv_time,
        e.u.uncal.x,
        e.u.uncal.y,
        e.u.uncal.z);
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
      sensor_data.type = SENSOR_ACCELEROMETER;
      sensor_data.u.accel.x = e.u.uncal.x;
      sensor_data.u.accel.y = e.u.uncal.y;
      sensor_data.u.accel.z = e.u.uncal.z;
      /* Enqueue accel data and return all data as per request made(t_start-t_end)*/
      sensor_hidl_enqueue(sensor_data);
#else
      pthread_mutex_lock(&(this->cb_mutex));
      /* callback the gyro port notifying about the accelerometer */
      if (list_info->s_info[SENSOR_ACCELEROMETER].enabled) {
        this->data_cb(this->cb_hndl, cb_event_data);
      }
      pthread_mutex_unlock(&(this->cb_mutex));
#endif
    }
    break;

    case SensorType::GRAVITY: {
      cb_event_data.type = SENSOR_DATA;
      cb_event_data.u.sensor_data.type = SENSOR_GRAVITY;
      cb_event_data.u.sensor_data.timestamp = sensor_data.timestamp;
      cb_event_data.u.sensor_data.u.gravity.x = e.u.uncal.x;
      cb_event_data.u.sensor_data.u.gravity.y = e.u.uncal.y;
      cb_event_data.u.sensor_data.u.gravity.z = e.u.uncal.z;
      IS_LOW("Event type - Gravity!");
      IS_LOW("Gravity: evt_timestamp:%lld us rec_ts:%lld us x:%f y:%f z:%f",
        sensor_data.timestamp,
        recv_time,
        e.u.uncal.x,
        e.u.uncal.y,
        e.u.uncal.z);
      /* callback the gyro port notifying about the gravity */
      pthread_mutex_lock(&(this->cb_mutex));
      if (list_info->s_info[SENSOR_GRAVITY].enabled) {
       this->data_cb(this->cb_hndl, cb_event_data);
      }
      pthread_mutex_unlock(&(this->cb_mutex));
    }
    break;

    case SensorType::LINEAR_ACCELERATION: {
      cb_event_data.type = SENSOR_DATA;
      cb_event_data.u.sensor_data.type = SENSOR_LINEAR_ACCELERATION;
      cb_event_data.u.sensor_data.timestamp = sensor_data.timestamp;
      cb_event_data.u.sensor_data.u.lin_accel.x = e.u.uncal.x;
      cb_event_data.u.sensor_data.u.lin_accel.y = e.u.uncal.y;
      cb_event_data.u.sensor_data.u.lin_accel.z = e.u.uncal.z;
      IS_LOW("Event type - lin_accel!");
      IS_LOW("lin_accel: evt_timestamp:%lld us rec_ts:%lld us x:%f y:%f z:%f",
        sensor_data.timestamp,
        recv_time,
        e.u.uncal.x,
        e.u.uncal.y,
        e.u.uncal.z);
      /* callback the gyro port notifying about the lin_Accel */
      pthread_mutex_lock(&(this->cb_mutex));
      if (list_info->s_info[SENSOR_LINEAR_ACCELERATION].enabled) {
       this->data_cb(this->cb_hndl, cb_event_data);
      }
      pthread_mutex_unlock(&(this->cb_mutex));
    }
    break;


    case SensorType::LIGHT: {
      cb_event_data.type = SENSOR_DATA;
      cb_event_data.u.sensor_data.type = SENSOR_LIGHT;
      cb_event_data.u.sensor_data.timestamp = sensor_data.timestamp;
      cb_event_data.u.sensor_data.u.light.lux = e.u.scalar;
      IS_LOW("Event type - Light!");
      IS_LOW("Light: evt_timestamp:%lld us rec_ts:%lld us lux:%f",
        sensor_data.timestamp,
        recv_time,
        e.u.scalar);
      /* callback the gyro port notifying about the gravity */
      pthread_mutex_lock(&(this->cb_mutex));
      if (list_info->s_info[SENSOR_LIGHT].enabled) {
       this->data_cb(this->cb_hndl, cb_event_data);
      }
      pthread_mutex_unlock(&(this->cb_mutex));
    }
    break;

    default:
      IS_LOW("Event for sensor(type: %d) not interested", e.sensorType);
    break;
    }
  }

  return Void();
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf::sensor_hidl_gyro_start_search_forward
 *  @start_pos:Start position
 *
 * DESCRIPTION : Search forward in the buffer for starting sample
 *=========================================================================*/
sensor_return_type sensor_hidl_intf::sensor_hidl_gyro_start_search_forward(
  int start_pos, sensor_type type){
  if (start_pos > GYRO_BUFFER_SIZE - 1) {
    start_pos = 0;
  }
  IS_LOW("search forward for start pos for sensor type %d", type);
  if(type == SENSOR_GYRO) {
    this->start_idx = start_pos;
    do {
      if (this->sof_info.t_start > this->cirq_buffer[start_pos].timestamp) {
        start_pos++;
      } else {
        this->start_idx = start_pos;
        return SENSOR_RET_SUCCESS;
      }
      if (start_pos > GYRO_BUFFER_SIZE - 1) {
        start_pos = 0;
      }
    } while (start_pos != this->end_idx);
  }

#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  if(type == SENSOR_ACCELEROMETER) {
    this->start_idx_accel = start_pos;
    do {
      if (this->sof_info_accel.t_start > this->cirq_buffer_accel[start_pos].timestamp) {
        start_pos++;
      } else {
        this->start_idx_accel = start_pos;
        return SENSOR_RET_SUCCESS;
      }
      if (start_pos > GYRO_BUFFER_SIZE - 1) {
        start_pos = 0;
      }
    } while (start_pos != this->end_idx_accel);
  }
#endif

  return SENSOR_RET_FAILURE;
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf::sensor_hidl_gyro_start_search_backward
 *  @start_pos: start position
 *
 * DESCRIPTION: Utility function to Search backward in the buffer for starting sample
 *=========================================================================*/
sensor_return_type sensor_hidl_intf::sensor_hidl_gyro_start_search_backward(
  int start_pos, sensor_type type){
  if (start_pos < 0) {
    start_pos = GYRO_BUFFER_SIZE - 1;
  }
  IS_LOW("search backward for start pos for sensor type %d", type);
  if(type == SENSOR_GYRO) {
    do{
      if (this->sof_info.t_start < this->cirq_buffer[start_pos].timestamp) {
        start_pos--;
      } else {
        this->start_idx = start_pos;
        return SENSOR_RET_SUCCESS;
      }
      if (start_pos < 0) {
        start_pos = GYRO_BUFFER_SIZE - 1;
      }
    } while (start_pos != this->end_idx);
  }

#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  if(type == SENSOR_ACCELEROMETER) {
    do{
      if (this->sof_info_accel.t_start < this->cirq_buffer_accel[start_pos].timestamp) {
        start_pos--;
      } else {
        this->start_idx_accel = start_pos;
        return SENSOR_RET_SUCCESS;
      }
      if (start_pos < 0) {
        start_pos = GYRO_BUFFER_SIZE - 1;
      }
    } while (start_pos != this->end_idx_accel);
  }
#endif

  return SENSOR_RET_FAILURE;
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf::sensor_hidl_gyro_port_notify
 *
 * DESCRIPTION :  Copy the requested gyro samples to the callback data struct and notify
 *                          Gyro port
 *=========================================================================*/
sensor_return_type sensor_hidl_intf::sensor_hidl_gyro_port_notify( sensor_type type ){
  sensor_cb_struct_type cb_event_data;
  sensor_gyro_buffer_data_type *gyro_data = NULL;
  int start_pos = this->start_idx;
  int end_pos = this->end_idx;
  int sample_len = 0;
  int idx = 0;

#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  sensor_accel_buffer_data_type *accel_data = NULL;
  if(type == SENSOR_ACCELEROMETER){
    start_pos = this->start_idx_accel;
    end_pos = this->end_idx_accel;
  }
#endif

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
  cb_event_data.u.sensor_data.timestamp = 0; /* not used */

  /* Getting data from cirq_buf */
  if( type == SENSOR_GYRO ) {
    if( this->cirq_buffer[buf_pos].type == SENSOR_GYRO ){
      IS_LOW("sensor type: %d, buf_pos %d type %d, sample_len %d",
        type, buf_pos, this->cirq_buffer[buf_pos].type, sample_len);
      cb_event_data.u.sensor_data.type = this->cirq_buffer[buf_pos].type;
      gyro_data = &cb_event_data.u.sensor_data.u.gyro;
      gyro_data->seqnum = this->sof_info.id;
      gyro_data->sample_len = sample_len;
    }
  }

#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  if( type == SENSOR_ACCELEROMETER ){
    if( this->cirq_buffer_accel[buf_pos_accel].type == SENSOR_ACCELEROMETER ){
      IS_LOW("sensor type: %d, buf_pos %d type %d, sample_len %d",
        type, buf_pos_accel, this->cirq_buffer_accel[buf_pos_accel].type, sample_len);
      cb_event_data.u.sensor_data.type = this->cirq_buffer_accel[buf_pos_accel].type;
      accel_data = &cb_event_data.u.sensor_data.u.accel_buf;
      accel_data->seqnum = this->sof_info_accel.id;
      accel_data->sample_len = sample_len;
    }
  }
#endif

  idx = 0;
  while (idx < sample_len) {
    if( type == SENSOR_GYRO ) {
      if( this->cirq_buffer[buf_pos].type == SENSOR_GYRO ) {
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
        gyro_data->sample[idx].timestamp_ns =
          this->cirq_buffer[start_pos].is_timestamp;
#endif
        gyro_data->sample[idx].timestamp =
          this->cirq_buffer[start_pos].timestamp;
        gyro_data->sample[idx].value[0]=
          this->cirq_buffer[start_pos].u.gyro.x;
        gyro_data->sample[idx].value[1]=
          this->cirq_buffer[start_pos].u.gyro.y;
        gyro_data->sample[idx].value[2]=
          this->cirq_buffer[start_pos].u.gyro.z ;
        }
      }
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
    if( type == SENSOR_ACCELEROMETER ) {
      if ( this->cirq_buffer_accel[buf_pos_accel].type == SENSOR_ACCELEROMETER ) {
        accel_data->sample[idx].timestamp_ns =
          this->cirq_buffer_accel[start_pos].is_timestamp;
        accel_data->sample[idx].timestamp =
          this->cirq_buffer_accel[start_pos].timestamp;
        accel_data->sample[idx].value[0]=
          this->cirq_buffer_accel[start_pos].u.accel.x;
        accel_data->sample[idx].value[1]=
          this->cirq_buffer_accel[start_pos].u.accel.y;
        accel_data->sample[idx].value[2]=
          this->cirq_buffer_accel[start_pos].u.accel.z;
        IS_LOW("accel[%d] ts: %lld ts_ns %lld (%lf, %lf, %lf)", start_pos,
          accel_data->sample[idx].timestamp,
          accel_data->sample[idx].timestamp_ns,
          accel_data->sample[idx].value[0],
          accel_data->sample[idx].value[1],
          accel_data->sample[idx].value[2]);
        }
      }
#endif
    start_pos++;
    idx++;
    if (start_pos > GYRO_BUFFER_SIZE - 1) {
      start_pos = start_pos % GYRO_BUFFER_SIZE;
    }
  }

  pthread_mutex_lock(&(this->cb_mutex));
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  if (this->list_info.s_info[SENSOR_GYRO].enabled ||
    this->list_info.s_info[SENSOR_ACCELEROMETER].enabled ) {
#else
  if (this->list_info.s_info[SENSOR_GYRO].enabled ) {
#endif
    this->data_cb(this->cb_hndl, cb_event_data);
  }
  pthread_mutex_unlock(&(this->cb_mutex));
  return SENSOR_RET_SUCCESS;
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf::sensor_hidl_process_request
 *
 * DESCRIPTION : Process the Gyro port request when required data is available
 *=========================================================================*/
sensor_return_type sensor_hidl_intf::sensor_hidl_process_request( sensor_type type ){
  int end_pos = 0;
  int start_pos = 0;
  unsigned long long start_ts = 0;

  if(type == SENSOR_GYRO) {
    start_pos = this->pend_idx;
    start_ts = this->pend_ts;
    end_pos = this->buf_pos - 1;
    if (end_pos < 0) {
      end_pos = GYRO_BUFFER_SIZE - 1;
    }

    /* search for the end timestamp of gyro request in buffer */
    do {
      if (this->cirq_buffer[end_pos].timestamp < this->sof_info.t_end) {
        this->end_idx = end_pos;
        /* search for start position */
        if (this->sof_info.t_start == start_ts) {
          this->start_idx = start_pos;
        } else if (this->sof_info.t_start >
          this->cirq_buffer[start_pos].timestamp) {
          /* search forward */
          sensor_hidl_gyro_start_search_forward(start_pos, type);
        } else {
          /* search backward */
          sensor_hidl_gyro_start_search_backward(start_pos, type);
        }

        /* Requested data ready; Copy into circular queue */
        IS_LOW("type %d copying data to circular queue: start:%d end:%d",
          type, this->start_idx,this->end_idx);
        return sensor_hidl_gyro_port_notify(type);
      }
      else{
        end_pos--;
      }
      if (end_pos < 0) {
        end_pos = GYRO_BUFFER_SIZE - 1;
      }
    }  while (end_pos != this->pend_idx);
  }

#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  if(type == SENSOR_ACCELEROMETER) {
    start_pos = this->pend_idx_accel;
    start_ts = this->pend_ts_accel;
    end_pos = this->buf_pos_accel - 1;
    if (end_pos < 0) {
      end_pos = GYRO_BUFFER_SIZE - 1;
    }

    /* search for the end timestamp of gyro request in buffer */
    do {
      if (this->cirq_buffer_accel[end_pos].timestamp < this->sof_info_accel.t_end) {
        IS_LOW("[hidl] sensor type:%d, sof_info> id:%d, t_end:%ld",
          type, this->sof_info_accel.id, this->sof_info_accel.t_end);
        this->end_idx_accel = end_pos;
        /* search for start position */
        if (this->sof_info_accel.t_start == start_ts) {
          this->start_idx_accel = start_pos;
        } else if (this->sof_info_accel.t_start >
          this->cirq_buffer_accel[start_pos].timestamp) {
          /* search forward */
          sensor_hidl_gyro_start_search_forward(start_pos, type);
        } else {
         /* search backward */
         sensor_hidl_gyro_start_search_backward(start_pos, type);
        }
        /* Requested data ready; Copy into circular queue */
        IS_LOW("type %d copying data to circular queue: start:%d end:%d",
          type, this->start_idx_accel,this->end_idx_accel);
        return sensor_hidl_gyro_port_notify(type);
      }
      else {
        end_pos--;
      }

      if (end_pos < 0) {
        end_pos = GYRO_BUFFER_SIZE - 1;
      }
    } while (end_pos != this->pend_idx_accel);
  }
#endif

  return SENSOR_RET_FAILURE;
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf::sensor_hidl_enqueue
 *    @sensor_data_type: sensor data
 *
 * DESCRIPTION   Store the received samples in a circular buffer until request from gyro port is met
 *=========================================================================*/
sensor_return_type sensor_hidl_intf::sensor_hidl_enqueue(
  sensor_data_type sensor_data){

  int buf_pos = 0;
  if ( sensor_data.type == SENSOR_GYRO ) {
    if (this->buf_pos > GYRO_BUFFER_SIZE - 1) {
      this->buf_pos = 0;
    }
    int buf_pos = this->buf_pos;
    IS_LOW("Storing sample in circular buffer:pos:%d", buf_pos);
    this->cirq_buffer[buf_pos].timestamp = sensor_data.timestamp;
    this->cirq_buffer[buf_pos].type = sensor_data.type;
    /*orienation changes in android frmaework. To make
       it in complaince with dsps; X <--> Y; Z --> -Z */
    this->cirq_buffer[buf_pos].u.gyro.x = (int)(sensor_data.u.gyro.y * (1 << 16));
    this->cirq_buffer[buf_pos].u.gyro.y = (int)(sensor_data.u.gyro.x * (1 << 16));
    this->cirq_buffer[buf_pos].u.gyro.z = -(int)(sensor_data.u.gyro.z * (1 << 16));
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
    this->cirq_buffer[buf_pos].is_timestamp = sensor_data.is_timestamp;
    this->cirq_buffer[buf_pos].u.gyro.x = sensor_data.u.gyro.y;
    this->cirq_buffer[buf_pos].u.gyro.y = sensor_data.u.gyro.x;
    this->cirq_buffer[buf_pos].u.gyro.z = -sensor_data.u.gyro.z;
#endif

  }
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  else if( sensor_data.type == SENSOR_ACCELEROMETER ) {
    if (this->buf_pos_accel > GYRO_BUFFER_SIZE - 1) {
      this->buf_pos_accel = 0;
    }
    buf_pos = this->buf_pos_accel;

    IS_LOW("Storing ACCEL sample in circular buffer:pos:%d", buf_pos);
    this->cirq_buffer_accel[buf_pos].timestamp = sensor_data.timestamp;
    this->cirq_buffer_accel[buf_pos].is_timestamp = sensor_data.is_timestamp;
    this->cirq_buffer_accel[buf_pos].type = sensor_data.type;
    this->cirq_buffer_accel[buf_pos].u.accel.x = sensor_data.u.accel.y;
    this->cirq_buffer_accel[buf_pos].u.accel.y = sensor_data.u.accel.x;
    this->cirq_buffer_accel[buf_pos].u.accel.z = -sensor_data.u.accel.z;
  }
#endif

  if(sensor_data.type == SENSOR_GYRO) {
    /*  check for a request */
    if (this->sof_info.t_end != 0) {
      /* check wheteher the requested samples are available */
      if (sensor_data.timestamp >= this->sof_info.t_end) {
        /*Lock the update of SoF request from gyro_port*/
        pthread_mutex_lock(&(this->sof_req_mutex));
        /* The requested samples are available in buffer ;
           Copy them in to a circular queue and notify gyro port*/
        IS_LOW("Requested samples are available in buffer");
        if (sensor_hidl_process_request(sensor_data.type) != SENSOR_RET_SUCCESS) {
          IS_ERR("Gyro request was not processed");
      }
      this->pend_ts = this->sof_info.t_end;
      this->pend_idx = this->end_idx;
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
      this->pend_idx = this->end_idx + 1;
      this->pend_idx = (this->pend_idx > GYRO_BUFFER_SIZE - 1) ? 0 : this->pend_idx;
#endif
      this->pstart_idx = this->start_idx;
      /* set previous frame request processed */
      this->prev_request_proc = TRUE;
      /* Reset untill a request comes*/
      this->sof_info.t_end = 0;
      /* check if there is any new request and update the timestamps */
      if (this->sof_request) {
        this->sof_info.id = this->sof_info_next.id;
        this->sof_info.t_start = this->sof_info_next.t_start;
        this->sof_info.t_end = this->sof_info_next.t_end;
        this->sof_request = FALSE;
        this->prev_request_proc = FALSE;
      }
      pthread_mutex_unlock(&(this->sof_req_mutex));
      }
    }
    ++this->buf_pos;
  }

#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  if(sensor_data.type == SENSOR_ACCELEROMETER) {
    /*  check for a request */
    if (this->sof_info_accel.t_end != 0) {
      /* check wheteher the requested samples are available */
      if (sensor_data.timestamp >= this->sof_info_accel.t_end) {
        /*Lock the update of SoF request from accel_port*/
        pthread_mutex_lock(&(this->sof_req_mutex_accel));

        /* The requested samples are available in buffer ;
           Copy them in to a circular queue and notify gyro port*/
        IS_LOW("Type %d Requested samples are available in buffer", sensor_data.type);
        if (sensor_hidl_process_request( sensor_data.type ) != SENSOR_RET_SUCCESS) {
          IS_ERR("Accel request was not processed");
        }

        this->pend_ts_accel = this->sof_info_accel.t_end;
        this->pend_idx_accel = this->end_idx_accel + 1;
        this->pend_idx_accel = (this->pend_idx_accel > GYRO_BUFFER_SIZE - 1) ? 0 : this->pend_idx_accel;
        this->pstart_idx_accel = this->start_idx_accel;
        /* set previous frame request processed */
        this->prev_request_proc_accel = TRUE;
        /* Reset untill a request comes*/
        this->sof_info_accel.t_end = 0;
        /* check if there is any new request and update the timestamps */
        if (this->sof_request_accel) {
          this->sof_info_accel.id = this->sof_info_next_accel.id;
          this->sof_info_accel.t_start = this->sof_info_next_accel.t_start;
          this->sof_info_accel.t_end = this->sof_info_next_accel.t_end;
          this->sof_request_accel = FALSE;
          this->prev_request_proc_accel = FALSE;
        }
        pthread_mutex_unlock(&(this->sof_req_mutex_accel));
      }
    }
    ++this->buf_pos_accel;
  }
#endif

  return SENSOR_RET_SUCCESS;
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf::sensor_hidl_init
 *    @cb_hndl: callback handle to notify gyro port of the requested data
 *    @data_cb : set/req data info
 *
 * DESCRIPTION : creates sensormanager,eventqueue and query for the required sensor info
 *=========================================================================*/
sensor_return_type sensor_hidl_intf::sensor_hidl_init(
  void* cb_hndl,
  sensor_data_cb data_cb){

  SensorInfo info;
  Result result;
  sensor_cb_struct_type cb_event_data;
  sensor_list_type *sensor_list = &cb_event_data.u.sensor_list;
  memset(&cb_event_data, 0, sizeof(cb_event_data));

  this->cb_hndl = cb_hndl;
  this->data_cb = data_cb;
  sensor_hidl_sensor_info_type *s_info =
  this->list_info.s_info;

  IS_LOW("Trying to get sensor service");
  sp<ISensorManager> smgr = ISensorManager::tryGetService();

  if (smgr == NULL) {
    IS_ERR("Failed to get ISensorManager interface");
    return SENSOR_RET_FAILURE;
  }

/*check for gyroscope presence and get details*/
  smgr->getDefaultSensor(SensorType::GYROSCOPE,
                          [&] (const auto& tmpInfo, const auto& tmpResult) {
                                 result = tmpResult;
                                 if (result != Result::OK) {
                                   return;
                                 }
                               info = tmpInfo;
                               });

  if (result == Result::OK) {
    IS_INFO("[%s]: name: %s, version: 0x%x",
      info.typeAsString.c_str(), info.name.c_str(), info.version);
    IS_INFO("[%s]: minDelay: %d, maxDelay: %d",
      info.typeAsString.c_str(), info.minDelay, info.maxDelay);
    IS_INFO("[%s]: handle = 0x%x, flags: 0x%x",
      info.typeAsString.c_str(), info.sensorHandle, info.flags);

    s_info[SENSOR_GYRO].present= TRUE;
    s_info[SENSOR_GYRO].sensor_handle = info.sensorHandle;
    s_info[SENSOR_GYRO].min_delay = info.minDelay;
    s_info[SENSOR_GYRO].max_delay = info.maxDelay;

    sensor_list->available_sensors[sensor_list->num_of_sensors] =
      SENSOR_GYRO;
    sensor_list->num_of_sensors++;
  } else {
    IS_INFO("No gyro presence");
  }


/*check for accelerometer presence and get details*/
  smgr->getDefaultSensor(SensorType::ACCELEROMETER,
                           [&] (const auto& tmpInfo, const auto& tmpResult) {
                                 result = tmpResult;
                                 if (result != Result::OK) {
                                   return;
                                 }
                                 info = tmpInfo;
                               });

  if (result == Result::OK) {
    IS_INFO("[%s]: name: %s, version: 0x%x",
      info.typeAsString.c_str(), info.name.c_str(), info.version);
    IS_INFO("[%s]: minDelay: %d, maxDelay: %d",
      info.typeAsString.c_str(), info.minDelay, info.maxDelay);
    IS_INFO("[%s]: handle = 0x%x, flags: 0x%x",
      info.typeAsString.c_str(), info.sensorHandle, info.flags);

    s_info[SENSOR_ACCELEROMETER].present= TRUE;
    s_info[SENSOR_ACCELEROMETER].sensor_handle = info.sensorHandle;
    s_info[SENSOR_ACCELEROMETER].min_delay = info.minDelay;
    s_info[SENSOR_ACCELEROMETER].max_delay = info.maxDelay;

    sensor_list->available_sensors[sensor_list->num_of_sensors] =
      SENSOR_ACCELEROMETER;
    sensor_list->num_of_sensors++;
  } else {
    IS_INFO("No accel presence");
  }

  /*check for gravity presence and get details*/
  smgr->getDefaultSensor(SensorType::GRAVITY,
                          [&] (const auto& tmpInfo, const auto& tmpResult) {
                                result = tmpResult;
                                if (result != Result::OK) {
                                  return;
                                }
                                info = tmpInfo;
                               });

  if (result == Result::OK) {
    IS_INFO("[%s]: name: %s, version: 0x%x",
      info.typeAsString.c_str(), info.name.c_str(), info.version);
    IS_INFO("[%s]: minDelay: %d, maxDelay: %d",
      info.typeAsString.c_str(), info.minDelay, info.maxDelay);
    IS_INFO("[%s]: handle = 0x%x, flags: 0x%x",
      info.typeAsString.c_str(), info.sensorHandle, info.flags);

    s_info[SENSOR_GRAVITY].present= TRUE;
    s_info[SENSOR_GRAVITY].sensor_handle = info.sensorHandle;
    s_info[SENSOR_GRAVITY].min_delay = info.minDelay;
    s_info[SENSOR_GRAVITY].max_delay = info.maxDelay;

    sensor_list->available_sensors[sensor_list->num_of_sensors] =
    SENSOR_GRAVITY;
    sensor_list->num_of_sensors++;
  } else {
    IS_INFO("No gravity presence");
  }

  /*check for linear accel presence and get details*/
  smgr->getDefaultSensor(SensorType::LINEAR_ACCELERATION,
                           [&] (const auto& tmpInfo, const auto& tmpResult) {
                                 result = tmpResult;
                                 if (result != Result::OK) {
                                   return;
                                 }
                                 info = tmpInfo;
                                });

  if (result == Result::OK) {
    IS_INFO("[%s]: name: %s, version: 0x%x",
      info.typeAsString.c_str(), info.name.c_str(), info.version);
    IS_INFO("[%s]: minDelay: %d, maxDelay: %d",
      info.typeAsString.c_str(), info.minDelay, info.maxDelay);
    IS_INFO("[%s]: handle = 0x%x, flags: 0x%x",
      info.typeAsString.c_str(), info.sensorHandle, info.flags);

    s_info[SENSOR_LINEAR_ACCELERATION].present= TRUE;
    s_info[SENSOR_LINEAR_ACCELERATION].sensor_handle = info.sensorHandle;
    s_info[SENSOR_LINEAR_ACCELERATION].min_delay = info.minDelay;
    s_info[SENSOR_LINEAR_ACCELERATION].max_delay = info.maxDelay;

    sensor_list->available_sensors[sensor_list->num_of_sensors] =
      SENSOR_LINEAR_ACCELERATION;
    sensor_list->num_of_sensors++;
  } else {
    IS_INFO("No lin accel presence");
  }


  /*check for light sensor presence and get details*/
  smgr->getDefaultSensor(SensorType::LIGHT,
                           [&] (const auto& tmpInfo, const auto& tmpResult) {
                                 result = tmpResult;
                                 if (result != Result::OK) {
                                   return;
                                 }
                                 info = tmpInfo;
                               });

  if (result == Result::OK) {
    IS_INFO("[%s]: name: %s, version: 0x%x",
      info.typeAsString.c_str(), info.name.c_str(), info.version);
    IS_INFO("[%s]: minDelay: %d, maxDelay: %d",
      info.typeAsString.c_str(), info.minDelay, info.maxDelay);
    IS_INFO("[%s]: handle = 0x%x, flags: 0x%x",
      info.typeAsString.c_str(), info.sensorHandle, info.flags);

    s_info[SENSOR_LIGHT].present= TRUE;
    s_info[SENSOR_LIGHT].sensor_handle = info.sensorHandle;
    s_info[SENSOR_LIGHT].min_delay = info.minDelay;
    s_info[SENSOR_LIGHT].max_delay = info.maxDelay;

    sensor_list->available_sensors[sensor_list->num_of_sensors] =
      SENSOR_LIGHT;
    sensor_list->num_of_sensors++;
  } else {
    IS_INFO("No light sensor presence");
  }



  /* TO DO : get Time of flight info */


  /*send sensor list info to client*/
  cb_event_data.type = SENSOR_LIST;
  this->data_cb(this->cb_hndl, cb_event_data);

  /*callback object(s_hidl) is freed by IEventqueue implemter, don't delete in deinit*/
  this->cbr = new sensor_hidl_callback();
  this->cbr->sensor_hidl_intf_obj = (void *)this;
  /* Create sensor event queue  */
  smgr->createEventQueue(this->cbr,
                          [&] (const auto &tmpQueue,
                               const auto &tmpResult) {
                                result = tmpResult;
                                if (result != Result::OK) {
                                  return;
                                }
                                event_q_ = tmpQueue;
                              });
  if (result != Result::OK) {
    IS_ERR("Failed to create sensor event queue");
    return SENSOR_RET_FAILURE;
  }

  /*mark state as active to process events received through onEvent callback*/
  if(this->cbr) {
    this->cbr->state = STATE_ACTIVE;
  }

  return SENSOR_RET_SUCCESS;
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf::sensor_hidl_set_params
 *    @set_data : set/req data info
 *
 * DESCRIPTION : sets/config the sensor data.
 *=========================================================================*/
sensor_return_type sensor_hidl_intf::sensor_hidl_set_data(sensor_set_data_type *set_data) {
  sensor_return_type rc = SENSOR_RET_SUCCESS;
  sensor_data_range_type data_range = set_data->u.data_range;

  IS_LOW("sensorType %d", set_data->sensor);
  if(set_data->sensor == SENSOR_GYRO) {
    pthread_mutex_lock(&(this->sof_req_mutex));


    if (this->prev_request_proc) {
      IS_LOW("sof_event :%d", data_range.id);
        this->sof_info.id = data_range.id;
        this->sof_info.t_start = data_range.t_start;
        this->sof_info.t_end = data_range.t_end;
        this->prev_request_proc = FALSE ;
    } else {
      IS_LOW("New sof_event :%d ",data_range.id);
        this->sof_request = TRUE;
        this->sof_info_next.id = data_range.id;
        this->sof_info_next.t_start = data_range.t_start;
        this->sof_info_next.t_end = data_range.t_end;
    }
  pthread_mutex_unlock(&(this->sof_req_mutex));
  }
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  if(set_data->sensor == SENSOR_ACCELEROMETER) {
    pthread_mutex_lock(&(this->sof_req_mutex_accel));
    if (this->prev_request_proc_accel) {
      IS_LOW("sof_event :%d ", data_range.id);
        this->sof_info_accel.id = data_range.id;
        this->sof_info_accel.t_start = data_range.t_start;
        this->sof_info_accel.t_end = data_range.t_end;
        this->prev_request_proc_accel = FALSE ;
    } else {
      IS_LOW("New sof_event :%d", data_range.id);
        this->sof_request_accel = TRUE;
        this->sof_info_next_accel.id = data_range.id;
        this->sof_info_next_accel.t_start = data_range.t_start;
        this->sof_info_next_accel.t_end = data_range.t_end;
    }
   pthread_mutex_unlock(&(this->sof_req_mutex_accel));
  }
#endif

  rc = SENSOR_RET_SUCCESS;
  return rc;
}

/*===========================================================================
 * FUNCTION      sensor_hidl_intf::sensor_hidl_enable
 *    @sensorType: type of sensor to configure
 *    @sample_rateUS : sample rate in micro seconds
 *
 * DESCRIPTION : enables and sets the sample rate of a particular sensor
 *=========================================================================*/

sensor_return_type sensor_hidl_intf::sensor_hidl_enable(sensor_type sensorType,
  int32_t sample_rateUS){
  Result result;
  if(!this->list_info.s_info[sensorType].enabled) {
    if(sample_rateUS == 0) {
      sample_rateUS = (sensorType == SENSOR_GYRO)?
        DEFAULT_RATE_IN_US:DEFAULT_RATE_IN_US_ACCEL;
    }

    if(sample_rateUS < this->list_info.s_info[sensorType].min_delay) {
      sample_rateUS = this->list_info.s_info[sensorType].min_delay;
      IS_HIGH("sample rate is not supported, setting it min possible");
    }
    /*Set batching period of each sensor to 15000 us*/
    result = event_q_->enableSensor(this->list_info.s_info[sensorType].sensor_handle,
                                        sample_rateUS,
                                        15000);
  } else {
    IS_INFO("sensor: %d already enabled",sensorType);
    return SENSOR_RET_BAD_PARAMS;
  }

  if (result != Result::OK) {
    IS_ERR("Failed to enable sensorType %d", sensorType);
    return SENSOR_RET_FAILURE;
  }

  if(sensorType == SENSOR_GYRO) {
    /*If gyro sensor is enabled, set prev req processed to true*/
    this->prev_request_proc = TRUE;
    memset(&sof_info,0,sizeof(sensor_data_range_type));
    memset(&sof_info_next,0,sizeof(sensor_data_range_type));
    sof_request = FALSE;
  }

#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  if(sensorType == SENSOR_ACCELEROMETER) {
    /*If accel sensor is enabled, set prev req processed to true*/
    this->prev_request_proc_accel = TRUE;
    memset(&sof_info_accel,0,sizeof(sensor_data_range_type));

    memset(&sof_info_next_accel,0,sizeof(sensor_data_range_type));
    sof_request_accel = FALSE;
  }
#endif

IS_LOW("sensor type %d, sample_rateUS %d", sensorType, sample_rateUS);
  this->list_info.s_info[sensorType].enabled = TRUE;

  return SENSOR_RET_SUCCESS;
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf::sensor_hidl_disable
 *    @sensorType: type of sensor to configure
 *
 * DESCRIPTION : Disables a particular sensor
 *=========================================================================*/
sensor_return_type sensor_hidl_intf::sensor_hidl_disable(sensor_type sensorType){
  Result result;
  if(this->list_info.s_info[sensorType].enabled) {
    result = event_q_->disableSensor(this->list_info.s_info[sensorType].sensor_handle);
  } else {
    IS_INFO("sensor: %d already disabled", sensorType);
    return SENSOR_RET_BAD_PARAMS;
  }
  if (result != Result::OK) {
    IS_ERR("Failed to disable sensortype %d ",sensorType);
    return SENSOR_RET_FAILURE;
  }
  this->list_info.s_info[sensorType].enabled = FALSE;

  return SENSOR_RET_SUCCESS;
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf::sensor_hidl_set_params
 *
 * DESCRIPTION : disables any enabled sensors and assigns event queue ptr to null
 *=========================================================================*/
sensor_return_type sensor_hidl_intf::sensor_hidl_deinit(){

  /*Disable sensors if not already*/
  for(int i = 0; i < SENSOR_TYPE_MAX_AVAILABLE; i++){
    if(this->list_info.s_info[i].enabled) {
      this->sensor_hidl_disable((sensor_type)i);
    }
  }

  /*mark state as Inactive to ignore events received through onEvent callback*/
  if(this->cbr) {
    this->cbr->state = STATE_INACTIVE;
  }
  /*Event queue is freed by Ieventqueue implementer once reference cnt is 0*/
  this->event_q_ = nullptr;
  return SENSOR_RET_SUCCESS;
}



extern "C" {


/*===========================================================================
 * FUNCTION      sensor_hidl_intf_enable_sensor
 *    @handle: Handler
 *    @set_data : set/req data info
 *
 * DESCRIPTION : Interface function for the user to call the enable sensor method of HIDL intf
 *=========================================================================*/
static sensor_return_type sensor_hidl_intf_enable_sensor(
  void *handle,
  sensor_set_data_type *set_data){
  sensor_hidl_intf* s_hidl = (sensor_hidl_intf*)(handle);
  s_hidl->sensor_hidl_enable(set_data->sensor, set_data->u.sample_rate*MS2US);
  IS_LOW("Enable sensor type %d", set_data->sensor);
  return SENSOR_RET_SUCCESS;
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf_disable_sensor
 *    @handle: Handler
 *    @set_data : set/req data info
 *
 * DESCRIPTION : Interface function for the user to call the disable sensor method of HIDL intf
 *=========================================================================*/
static sensor_return_type sensor_hidl_intf_disable_sensor(
  void *handle,
  sensor_type sensorType){
  sensor_hidl_intf* s_hidl = (sensor_hidl_intf*)(handle);
  s_hidl->sensor_hidl_disable(sensorType);
  return SENSOR_RET_SUCCESS;
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf_set_params
 *    @handle: Handler
 *    @set_data : set/req data info
 *
 * DESCRIPTION : sets/config the sensor data.
 *=========================================================================*/
sensor_return_type sensor_hidl_intf_set_params(
  void *handle, sensor_set_data_type set_data){
  sensor_return_type rc = SENSOR_RET_SUCCESS;
  sensor_hidl_intf *s_hidl = (sensor_hidl_intf *)handle;
  if (!s_hidl) {
    IS_HIGH("sensor object not initialized!");
    return SENSOR_RET_BAD_PARAMS;
  }
  switch (set_data.msg_type) {
  case SENSOR_SET_DATA_CONFIG:
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
    if ((set_data.sensor == SENSOR_GYRO)||(set_data.sensor == SENSOR_ACCELEROMETER)) {
#else
    if (set_data.sensor == SENSOR_GYRO) {
#endif
      rc = (sensor_return_type)s_hidl->sensor_hidl_set_data(&set_data);
    } else {
      rc = SENSOR_RET_BAD_PARAMS;
    }
  break;
  case SENSOR_ENABLE_REQ:
    rc = (sensor_return_type)sensor_hidl_intf_enable_sensor(handle, &set_data);
  break;
  case SENSOR_DISABLE_REQ:
    rc = (sensor_return_type)sensor_hidl_intf_disable_sensor(handle, set_data.sensor);
  break;
  default:
    IS_ERR("Error not handle msg_type: %d", set_data.msg_type);
    rc = SENSOR_RET_BAD_PARAMS;
  break;
  }
  return rc;
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf_init
 *    @handle: :Handler
 *    @cb_hndl:Pointer for callback
 *    @cb: data callback
 *
 * DESCRIPTION: Initialization of sensor Hidl Obj
 *=========================================================================*/
sensor_return_type sensor_hidl_intf_init(
  void **handle,
  void *cb_hndl,
  sensor_data_cb cb){
  sensor_hidl_intf* s_hidl = new sensor_hidl_intf();
  if(s_hidl->sensor_hidl_init(cb_hndl,cb)!= SENSOR_RET_SUCCESS){
    IS_ERR("Sensor HIDL intf failed");
    return SENSOR_RET_FAILURE;
  }
  *handle = (void *)s_hidl;
  return SENSOR_RET_SUCCESS;
}


/*===========================================================================
 * FUNCTION      sensor_hidl_intf_deinit
 *    @handle: :Handler
 *
 * DESCRIPTION: Deinit of sensor Hidl Obj
 *=========================================================================*/
sensor_return_type sensor_hidl_intf_deinit(
  void *handle){
  sensor_hidl_intf* s_hidl = (sensor_hidl_intf*)(handle);
  s_hidl->sensor_hidl_deinit();
  delete s_hidl;
  return SENSOR_RET_SUCCESS;
}
}
#endif /*SENSOR_HIDL_INTF */
