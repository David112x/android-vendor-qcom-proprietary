/*============================================================================
   Copyright (c) 2012 - 2017 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.

   This file defines the media/module/master controller's interface with the
   DSPS modules. The functionalities of this module include:

   1. Control communication with the sensor module
   2. Process data received from the sensors

============================================================================*/
#include "dsps_hw.h"
#ifdef FEATURE_GYRO_DSPS
#include <sensor1.h>
#include <sns_smgr_api_v01.h>
#include <sns_time_api_v02.h>
#include <sns_reg_api_v02.h>
#include "sns_sam_gravity_vector_v01.h"
#include "sns_sam_orientation_v01.h"

#define Q16                   (1<<16)
#define UNIT_CONVERT_Q16      (1.0/Q16)
#define UNIT_CONVERT_LIGHT    UNIT_CONVERT_Q16 /* Android (lux), sensor1 (lux in q16) */

#define DSPS_TXN_ID_RESERVED  5
#define DSPS_TXN_ID_GYRO      DSPS_TXN_ID_RESERVED
#define DSPS_TXN_ID_GRAVITY   DSPS_TXN_ID_GYRO + DSPS_TXN_ID_RESERVED
#define DSPS_TXN_ID_LIGHT     DSPS_TXN_ID_GRAVITY + DSPS_TXN_ID_RESERVED
#define DSPS_TXN_ID_HEADER    DSPS_TXN_ID_LIGHT + DSPS_TXN_ID_RESERVED
#define DSPS_TXN_ID_GYRO_AF   DSPS_TXN_ID_HEADER + DSPS_TXN_ID_RESERVED


void dsps_sensor1_callback(intptr_t *data, sensor1_msg_header_s *msg_hdr,
  sensor1_msg_type_e msg_type, void *msg_ptr);

#endif

/*===========================================================================
 * FUNCTION      dsps_close
 *
 * DESCRIPTION   Close a connection with the sensor framework
 *==========================================================================*/
int dsps_close(sensor1_config_t *dsps_config)
{
  if (dsps_config == NULL)
    return -1;
#ifdef FEATURE_GYRO_DSPS
  if (sensor1_close(dsps_config->handle) != SENSOR1_SUCCESS)
    return -1;
#else
  return -1;
#endif
  return 0;
}


/*===========================================================================
 * FUNCTION      dsps_disconnect
 *
 * DESCRIPTION   Deregister an mctl client with the DSPS Thread
 *=========================================================================*/
int dsps_disconnect(void * sensor_config)
{
  int rc = 0;
  sensor1_config_t * dsps_config = (sensor1_config_t *)sensor_config;

  if (dsps_config == NULL) {
    IS_ERR("Configuration Parameter is NULL");
    return -1;
  }

  if (dsps_close(dsps_config) < 0) {
    IS_ERR("Error in closing sensor connection");
    rc = -1;
  }
  pthread_mutex_destroy(&(dsps_config->callback_mutex));
  pthread_condattr_destroy(&dsps_config->callback_condattr);
  pthread_cond_destroy(&(dsps_config->callback_condvar));
  pthread_mutex_destroy(&(dsps_config->thread_mutex));
  pthread_condattr_destroy(&dsps_config->thread_condattr);
  pthread_cond_destroy(&(dsps_config->thread_condvar));

  return rc;
}

#ifdef FEATURE_GYRO_DSPS
/*===========================================================================
 * FUNCTION      dsps_set_expiry_time
 *
 * DESCRIPTION   Set the expiry time for timed wait by adding timeout
 *               value to current time.
 *==========================================================================*/
int dsps_set_expiry_time(struct timespec *expiry_time)
{
  int rc;
  rc = clock_gettime(CLOCK_MONOTONIC, expiry_time);
  if (rc < 0) {
    IS_ERR("Error reading Monotonic clock");
    return rc;
  }
  expiry_time->tv_sec += SENSOR_TIME_OUT * MSEC_TO_SEC;
  expiry_time->tv_sec += (expiry_time->tv_nsec + (SENSOR_TIME_OUT % SEC_TO_MSEC)
    * MSEC_TO_NSEC) / NSEC_PER_SEC;
  expiry_time->tv_nsec += (SENSOR_TIME_OUT % SEC_TO_MSEC) * MSEC_TO_NSEC;
  expiry_time->tv_nsec %= NSEC_PER_SEC;

  return rc;
}


/*===========================================================================
 * FUNCTION      dsps_handle_wait_synchronization
 *
 * DESCRIPTION   common function to handle the wait events
 *==========================================================================*/
void dsps_handle_wait_synchronization(sensor1_config_t *dsps_obj,
  uint8_t cb_txn_id, dsps_wait_sync_t wait_resp)
{
  int retval = 0;
  switch(wait_resp) {
    case DSPS_WAIT_RESP_START:
      retval = pthread_mutex_lock(&(dsps_obj->callback_mutex));
      dsps_obj->callback_txn_id = cb_txn_id;
      dsps_obj->callback_arrived = 0;
      break;

    case DSPS_WAIT_RESP_RECVD:
      if(dsps_obj->callback_txn_id == cb_txn_id) {
        IS_LOW("Received callback with txn_id %d ", cb_txn_id);
        dsps_obj->callback_arrived = 1;
        pthread_cond_signal(&(dsps_obj->callback_condvar));
      }
      break;

    case DSPS_WAIT_RESP_END:
      dsps_obj->callback_txn_id = 0;
      dsps_obj->callback_arrived = 0;
      retval = pthread_mutex_unlock(&(dsps_obj->callback_mutex));
      break;

    default:
      break;
  }
}


/*===========================================================================
 * FUNCTION      dsps_send_req
 *
 * DESCRIPTION   Sending the request to DSPS with optional wait
 *==========================================================================*/
int32_t dsps_send_req( void *sensor_config, void *req_msg,
  sensor1_msg_header_s *msg_hdr, boolean wait)
{
  sensor1_config_t *dsps_config = (sensor1_config_t *)sensor_config;
  struct timespec expiry_time;
  int32_t ret_val = EINVAL;

  if (!dsps_config) {
    return ret_val;
  }

  if (wait) {
    dsps_handle_wait_synchronization(dsps_config,
      msg_hdr->txn_id, DSPS_WAIT_RESP_START);
  }

  ret_val = sensor1_write(dsps_config->handle, msg_hdr, req_msg);
  if (SENSOR1_SUCCESS != ret_val) {
    sensor1_free_msg_buf(dsps_config->handle, req_msg);
    IS_ERR("sensor1_write failed ret_val %d ", ret_val);
  } else if (wait) {
    ret_val = dsps_set_expiry_time(&expiry_time);
    if (ret_val != 0) {
      IS_ERR("DSPS Error setting expiry time ret_val %d", ret_val);
    } else {
      if (!dsps_config->callback_arrived) {
        /* Timed wait for callback */
        ret_val = pthread_cond_timedwait(&(dsps_config->callback_condvar),
          &(dsps_config->callback_mutex), &expiry_time);
        if(ret_val == ETIMEDOUT) {
          IS_ERR("DSPS Send Request Timeout!!");
        }
      }
    }
  }

  if (wait) {
    dsps_handle_wait_synchronization(dsps_config, 0, DSPS_WAIT_RESP_END);
  }
  return ret_val;
}

/*===========================================================================
 * FUNCTION      dsps_handle_smgr_all_sensor_info_resp
 *
 * DESCRIPTION   Handle SMGR all sensor info response
 *==========================================================================*/
void dsps_handle_smgr_all_sensor_info_resp( sensor1_config_t *dsps_config, void *msg_ptr )
{
  uint32_t i;
  sensor1_handle_s *handle = dsps_config->handle;
  dsps_data_type dsps_sensor_type = DSPS_DATA_TYPE_MAX;
  dsps_cb_data_t cb_data;
  cb_data.cb_type = DSPS_SENSOR_LIST;
  cb_data.u.sensor_list.num_sensors = 0;
  sns_smgr_all_sensor_info_resp_msg_v01 all_sensor_info =
    *((sns_smgr_all_sensor_info_resp_msg_v01*) msg_ptr);

  IS_LOW("Retrieved all sensor info\n");
  for( i = 0; i < all_sensor_info.SensorInfo_len; i ++ ) {
    switch(all_sensor_info.SensorInfo[i].SensorID) {
      case SNS_SMGR_ID_ACCEL_V01:
        dsps_sensor_type = DSPS_DATA_TYPE_ACCELEROMETER;
        break;
      case SNS_SMGR_ID_GYRO_V01:
        dsps_sensor_type = DSPS_DATA_TYPE_GYRO;
        break;
      case SNS_SMGR_ID_PROX_LIGHT_V01:
        dsps_sensor_type = DSPS_DATA_TYPE_LIGHT;
        break;
      case SNS_SMGR_ID_MAG_V01:
        dsps_sensor_type = DSPS_DATA_TYPE_MAGNETOMETER;
        break;
      default:
        continue;
    }

    if(dsps_sensor_type < DSPS_DATA_TYPE_MAX) {
      STATS_MEMCPY(dsps_config->sensor_info[dsps_sensor_type].sensor_name,
        sizeof(dsps_config->sensor_info[dsps_sensor_type].sensor_name),
        all_sensor_info.SensorInfo[i].SensorShortName,
        all_sensor_info.SensorInfo[i].SensorShortName_len );
      dsps_config->sensor_info[dsps_sensor_type].sensor_name[
        all_sensor_info.SensorInfo[i].SensorShortName_len] = 0;
      dsps_config->sensor_info[dsps_sensor_type].sensor_id =
        all_sensor_info.SensorInfo[i].SensorID;
      dsps_config->sensor_info[dsps_sensor_type].present = TRUE;
      cb_data.u.sensor_list.available_sensors[cb_data.u.sensor_list.num_sensors] = dsps_sensor_type;
      cb_data.u.sensor_list.num_sensors++;
      IS_HIGH("Detected Sensor: %s ID: %d",
        dsps_config->sensor_info[dsps_sensor_type].sensor_name,
        dsps_config->sensor_info[dsps_sensor_type].sensor_id);
    }
  }

  /* Check for the presence of gravity sensor. Gravity is not a physical sensor.
       Its data is derived from lin_accel and gyro or mag sensors*/
  if(dsps_config->sensor_info[DSPS_DATA_TYPE_ACCELEROMETER].present &&
      (dsps_config->sensor_info[DSPS_DATA_TYPE_GYRO].present ||
         dsps_config->sensor_info[DSPS_DATA_TYPE_MAGNETOMETER].present)) {
    dsps_config->sensor_info[DSPS_DATA_TYPE_GRAVITY_VECTOR].present = TRUE;
    cb_data.u.sensor_list.available_sensors[cb_data.u.sensor_list.num_sensors] = DSPS_DATA_TYPE_GRAVITY_VECTOR;
    cb_data.u.sensor_list.num_sensors++;
    IS_LOW("Gravity Sensor is present");
  }
  IS_LOW("Number of sensors: %d", cb_data.u.sensor_list.num_sensors);
  dsps_config->dsps_callback(dsps_config->port, &cb_data);
}


/*===========================================================================
 * FUNCTION      dsps_prepare_smgr_all_sensor_info_req
 *
 * DESCRIPTION   Prepare Header to get SMGR All Sensor Info.
 *==========================================================================*/
void
dsps_prepare_smgr_all_sensor_info_req( sensor1_handle_s *handle, sensor1_msg_header_s *msg_hdr )
{
  (void)handle;
  msg_hdr->service_number = SNS_SMGR_SVC_ID_V01;
  msg_hdr->msg_id = SNS_SMGR_ALL_SENSOR_INFO_REQ_V01;
  msg_hdr->msg_size = 0;
  msg_hdr->txn_id = DSPS_TXN_ID_HEADER;
}


/*===========================================================================
 * FUNCTION      dsps_handle_smgr_single_sensor_info_resp
 *
 * DESCRIPTION   Handle SMGR single sensor info response
 *==========================================================================*/
void
dsps_handle_smgr_single_sensor_info_resp( sensor1_config_t *dsps_config, void *msg_ptr )
{
  uint32_t i;
  dsps_data_type dsps_sensor_type = DSPS_DATA_TYPE_MAX;
  uint8_t req_supported = 0;
  sns_smgr_single_sensor_info_resp_msg_v01 *info =
    (sns_smgr_single_sensor_info_resp_msg_v01*) msg_ptr;

  for( i = 0; i < info->SensorInfo.data_type_info_len; i++ ) {
    switch(info->SensorInfo.data_type_info[i].DataType) {
      case SNS_SMGR_DATA_TYPE_PRIMARY_V01:
        switch(info->SensorInfo.data_type_info[i].SensorID) {
          case SNS_SMGR_ID_ACCEL_V01:
            dsps_sensor_type = DSPS_DATA_TYPE_ACCELEROMETER;
            req_supported = DSPS_ENABLE_REQ |
              DSPS_DISABLE_REQ |
              DSPS_GET_REPORT;
            break;

          case SNS_SMGR_ID_GYRO_V01:
            dsps_sensor_type = DSPS_DATA_TYPE_GYRO;
            req_supported = DSPS_ENABLE_REQ |
              DSPS_DISABLE_REQ |
              DSPS_GET_REPORT |
              DSPS_TIMESTAMP_REQ;
            break;

          default:
            /*Sensor Not Supported*/
            continue;
        }
        break;

      case SNS_SMGR_DATA_TYPE_SECONDARY_V01:
        switch(info->SensorInfo.data_type_info[i].SensorID) {
          case SNS_SMGR_ID_PROX_LIGHT_V01:
            dsps_sensor_type = DSPS_DATA_TYPE_LIGHT;
            req_supported = DSPS_ENABLE_REQ |
              DSPS_DISABLE_REQ;
            break;

          default:
            /*Sensor Not Supported*/
            continue;
        }
        break;

      default:
        /*Data Type Not Supported*/
        continue;
    }

    if(dsps_sensor_type < DSPS_DATA_TYPE_MAX) {
      char sensor_name[SNS_SMGR_MAX_SENSOR_NAME_SIZE_V01+1];
      char vendor_name[SNS_SMGR_MAX_VENDOR_NAME_SIZE_V01+1];

      dsps_config->sensor_info[dsps_sensor_type].sensor_id =
        info->SensorInfo.data_type_info[i].SensorID;
      dsps_config->sensor_info[dsps_sensor_type].data_type =
        info->SensorInfo.data_type_info[i].DataType;
      dsps_config->sensor_info[dsps_sensor_type].sample_rate =
        info->SensorInfo.data_type_info[i].MaxSampleRate;
      dsps_config->sensor_info[dsps_sensor_type].max_range =
        info->SensorInfo.data_type_info[i].MaxRange;
      dsps_config->sensor_info[dsps_sensor_type].resolution =
        info->SensorInfo.data_type_info[i].Resolution;
      dsps_config->sensor_info[dsps_sensor_type].req_supported =
        req_supported;
      STATS_MEMCPY(sensor_name, sizeof(sensor_name),
        info->SensorInfo.data_type_info[i].SensorName,
        info->SensorInfo.data_type_info[i].SensorName_len);
      sensor_name[info->SensorInfo.data_type_info[i].SensorName_len] = 0;

      STATS_MEMCPY(vendor_name, sizeof(vendor_name),
        info->SensorInfo.data_type_info[i].VendorName,
        info->SensorInfo.data_type_info[i].VendorName_len);
      vendor_name[info->SensorInfo.data_type_info[i].VendorName_len] = 0;

      IS_LOW("Sensor Name %s Vendor Name %s Sensor id %d DataType %d Version %u "
        "MaxSampleRate %u IdlePower %u Max Power %u Max Range %u Resolution %u ",
        sensor_name, vendor_name,
        info->SensorInfo.data_type_info[i].SensorID,
        info->SensorInfo.data_type_info[i].DataType,
        info->SensorInfo.data_type_info[i].Version,
        info->SensorInfo.data_type_info[i].MaxSampleRate,
        info->SensorInfo.data_type_info[i].IdlePower,
        info->SensorInfo.data_type_info[i].MaxPower,
        info->SensorInfo.data_type_info[i].MaxRange,
        info->SensorInfo.data_type_info[i].Resolution );
    }
  }
}


/*===========================================================================
 * FUNCTION      dsps_prepare_smgr_single_sensor_info_req
 *
 * DESCRIPTION   Prepare Message and Header to get SMGR Single Sensor Info.
 *==========================================================================*/
 sns_smgr_single_sensor_info_req_msg_v01*
dsps_prepare_smgr_single_sensor_info_req( sensor1_handle_s *hndl, sensor1_msg_header_s *msg_hdr, uint8_t SensorID)
{
  sensor1_error_e error;
  sns_smgr_single_sensor_info_req_msg_v01 *smgr_req;

  msg_hdr->service_number = SNS_SMGR_SVC_ID_V01;
  msg_hdr->msg_id = SNS_SMGR_SINGLE_SENSOR_INFO_REQ_V01;
  msg_hdr->msg_size = sizeof(sns_smgr_single_sensor_info_req_msg_v01);
  msg_hdr->txn_id = DSPS_TXN_ID_HEADER + SensorID;

  error = sensor1_alloc_msg_buf( hndl,
                                 sizeof(sns_smgr_single_sensor_info_req_msg_v01),
                                 (void**)&smgr_req );
  if( SENSOR1_SUCCESS != error ) {
    IS_ERR("sensor1_alloc_msg_buf returned %d\n", error);
    return NULL;
  }
  smgr_req->SensorID = SensorID;
  return smgr_req;
}


/*===========================================================================
 * FUNCTION      dsps_get_sensor_list
 *
 * DESCRIPTION   Get Available Sensor List from DSPS.
 *==========================================================================*/
 void dsps_get_sensor_list(void *sensor_config)
{
  int               opt;
  uint32_t          i,j;
  int32_t ret_val = 0;
  sensor1_config_t *dsps_config = (sensor1_config_t *)sensor_config;
  sensor1_handle_s *handle = dsps_config->handle;
  sensor1_msg_header_s smgr_all_info_req_hdr;
  sns_smgr_single_sensor_info_req_msg_v01 *smgr_single_info_req_msg;

  /* Get the sensor list */
  dsps_prepare_smgr_all_sensor_info_req(handle, &smgr_all_info_req_hdr);

#ifdef DSPS_DISABLE_WAIT_FOR_CALLBACK
  ret_val = dsps_send_req(dsps_config, NULL, &smgr_all_info_req_hdr, FALSE);
  if(ret_val != 0) {
    return;
  }

#else
  ret_val = dsps_send_req(dsps_config, NULL, &smgr_all_info_req_hdr, TRUE);
  if(ret_val != 0) {
    return;
  }
  /* Get info about each sensor */
  for( i = 0; i < DSPS_DATA_TYPE_MAX; i ++ ) {
    sensor1_msg_header_s smgr_single_info_req_hdr;
    if(dsps_config->sensor_info[i].present) {
      smgr_single_info_req_msg = dsps_prepare_smgr_single_sensor_info_req(handle,
        &smgr_single_info_req_hdr, dsps_config->sensor_info[i].sensor_id);
      ret_val = dsps_send_req(dsps_config, smgr_single_info_req_msg,
        &smgr_single_info_req_hdr, TRUE);
      if(ret_val != 0) {
        return;
      }
    }
  }

  /* List info for all available sensor */
  for( i = 0; i < DSPS_DATA_TYPE_MAX; i ++ ) {
    if(dsps_config->sensor_info[i].present) {
      IS_HIGH("Available Sensors Name: %s ID %d DT %d "
        "SampleRate %d MaxRange %d Resolution %d MinDelay %d",
        dsps_config->sensor_info[i].sensor_name,
        dsps_config->sensor_info[i].sensor_id,
        dsps_config->sensor_info[i].data_type,
        dsps_config->sensor_info[i].sample_rate,
        dsps_config->sensor_info[i].max_range,
        dsps_config->sensor_info[i].resolution,
        dsps_config->sensor_info[i].min_delay);
    }
  }

#endif
}
#endif

/*===========================================================================
 * FUNCTION      dsps_open
 *
 * DESCRIPTION   Open a new connection with the sensor framework
 *==========================================================================*/
int dsps_open(void *sensor_config)
{
  sensor1_config_t *dsps_config = (sensor1_config_t *)sensor_config;
#ifdef FEATURE_GYRO_DSPS
  /* Open sensor1 port */
  IS_LOW("try to sensor1_open()");
  if (sensor1_open(&dsps_config->handle,
      (sensor1_notify_data_cb_t)&dsps_sensor1_callback,
      (intptr_t)dsps_config) == SENSOR1_SUCCESS) {
      dsps_get_sensor_list(dsps_config);
    return 0;
  }
#endif
  return -1;
}


#ifdef FEATURE_GYRO_DSPS
/** dsps_get_gyro_samples:
 *    @data: gyro data from sensor
 *    @dsps_obj: dsps object (client handle)
 *    @cb_data: memory to store the gyro data
 *
 * This function transfers gyro data from sensors' memory to that of camera's.
 **/
void dsps_get_gyro_samples(void *data, sensor1_config_t *dsps_obj,
  dsps_cb_data_t *cb_data)
{
  uint8_t i, sample_len;
  uint32_t timestamp;
  sns_smgr_buffering_query_ind_msg_v01 *sensor_data = data;

  sample_len = (sensor_data->Samples_len > STATS_GYRO_MAX_SAMPLE_BUFFER_SIZE) ?
    STATS_GYRO_MAX_SAMPLE_BUFFER_SIZE : sensor_data->Samples_len;
  cb_data->u.gyro.sample_len = sample_len;

  timestamp = sensor_data->FirstSampleTimestamp;

  for (i = 0; i < sample_len; i++) {
    timestamp += sensor_data->Samples[i].TimeStampOffset;
    cb_data->u.gyro.sample[i].timestamp = (uint64_t)timestamp *
      USEC_PER_SEC / DSPS_HZ + dsps_obj->dsps_time_state.ts_offset;
    IS_LOW("timestamp (ticks) = %u, timestamp (us) = %llu, ts_offset = %llu",
       timestamp, cb_data->u.gyro.sample[i].timestamp,
      dsps_obj->dsps_time_state.ts_offset);
    cb_data->u.gyro.sample[i].value[0] = sensor_data->Samples[i].Data[0];
    cb_data->u.gyro.sample[i].value[1] = sensor_data->Samples[i].Data[1];
    cb_data->u.gyro.sample[i].value[2] = sensor_data->Samples[i].Data[2];
    IS_LOW("gyro sample buffered, %d, %d, %d", cb_data->u.gyro.sample[i].value[0], cb_data->u.gyro.sample[i].value[1], cb_data->u.gyro.sample[i].value[2]);
  }
}


/** dsps_get_gyro_samples_af:
 *    @data: gyro data from sensor
 *    @dsps_obj: dsps object (client handle)
 *    @cb_data: memory to store the gyro data
 *
 * This function transfers gyro data from sensors' memory to that of camera's.
 **/
boolean dsps_get_gyro_samples_af(sns_smgr_buffering_ind_msg_v01 *ind_msg,
  sensor1_config_t *dsps_obj,
  dsps_cb_data_t *cb_data)
{
  uint32_t timestamp = 0;
  uint8_t i, j;
  uint32_t index;
  uint8_t sample_len;

  if(NULL == dsps_obj || NULL == cb_data || NULL == ind_msg) {
  /* Invalid Inputs */
     IS_ERR("NULL dsps_obj %x, msg_hdr %x, msg_ptr %x",dsps_obj, cb_data, ind_msg);
     return FALSE;
  }

  IS_LOW("total indices %d, samples %d", ind_msg->Indices_len, ind_msg->Samples_len);
  for(i=0; i<ind_msg->Indices_len; i++) {
    timestamp = ind_msg->Indices[i].FirstSampleTimestamp;

    IS_LOW("Gyro: id %d, first %d, count %d", ind_msg->Indices[i].SensorId,
        ind_msg->Indices[i].FirstSampleIdx, ind_msg->Indices[i].SampleCount);
    if (ind_msg->Indices[i].SensorId == SNS_SMGR_ID_GYRO_V01) {
      sample_len = (ind_msg->Indices[i].SampleCount > STATS_GYRO_MAX_SAMPLE_BUFFER_SIZE) ?
        STATS_GYRO_MAX_SAMPLE_BUFFER_SIZE : ind_msg->Indices[i].SampleCount;
      cb_data->u.gyro.sample_len = sample_len;
      for(j=0; j<sample_len; j++) {
        index = ind_msg->Indices[i].FirstSampleIdx + j;
        timestamp += ind_msg->Samples[index].TimeStampOffset;

        cb_data->u.gyro.sample[j].value[0] = ind_msg->Samples[index].Data[0];
        cb_data->u.gyro.sample[j].value[1] = ind_msg->Samples[index].Data[1];
        cb_data->u.gyro.sample[j].value[2] = ind_msg->Samples[index].Data[2];
        cb_data->u.gyro.sample[j].timestamp = (uint64_t)timestamp *
            USEC_PER_SEC / DSPS_HZ + dsps_obj->dsps_time_state.ts_offset;

        IS_LOW("sample index %d, (x, y, z) = (%d, %d, %d) t %llu", index,
            cb_data->u.gyro.sample[j].value[0],
            cb_data->u.gyro.sample[j].value[1],
            cb_data->u.gyro.sample[j].value[2],
            cb_data->u.gyro.sample[j].timestamp);
        IS_LOW("timestamp (ticks) = %u, timestamp (us) = %llu, ts_offset = %llu",
            timestamp, cb_data->u.gyro.sample[i].timestamp, dsps_obj->dsps_time_state.ts_offset);
      }
    }
  }

  return TRUE;
}

/** dsps_prepare_smgr_req_msg:
 *    @dsps_obj: dsps object (client handle)
 *    @msg_data: request message data
 *
 * This function fills in the request message
 * This function assumes that sensor1_alloc_msg_buf returns 0-initialized
 * memory (calloc).
 **/
static void *dsps_prepare_smgr_req_msg(sensor1_config_t *dsps_config,
  sensor1_req_data_t *msg_data)
{
  sensor1_error_e error;
  sns_smgr_periodic_report_req_msg_v01 *smgr_req = NULL;

  boolean is_valid = FALSE;
  dsps_sensor_info_t *sensor_info;

  if(!dsps_config || !msg_data) {
    IS_ERR("Input Validation Failed");
    return NULL;
  }
  sensor_info = &dsps_config->sensor_info[msg_data->sensor_type];

  if(sensor_info->present == TRUE) {
    /*Sensor is Available, Check for Supported Type*/
    if(msg_data->msg_type | sensor_info->req_supported) {
      is_valid = TRUE;
    }
  }

  if(is_valid) {
    IS_LOW("Valid Sensor Request Message SensorType %d MsgType %",
      msg_data->sensor_type, msg_data->msg_type);

    error = sensor1_alloc_msg_buf(dsps_config->handle,
      sizeof(sns_smgr_periodic_report_req_msg_v01),
      (void**)&smgr_req );
    if(error != SENSOR1_SUCCESS) {
      IS_ERR("Allocation Failed Error %d", error);
      return NULL;
    }
    switch(msg_data->sensor_type) {
      case DSPS_DATA_TYPE_LIGHT:

       /*Use values from the Light Sensor Driver itself in the
        planned gyro thread. For now, these values are hardcoded*/
#ifdef DSPS_DISABLE_WAIT_FOR_CALLBACK
        smgr_req->ReportRate = msg_data->u.light.sample_rate;
        smgr_req->Item[0].SensorId = SNS_SMGR_ID_PROX_LIGHT_V01;
        smgr_req->Item[0].DataType = SNS_SMGR_DATA_TYPE_SECONDARY_V01;

#else
        smgr_req->ReportRate = sensor_info->sample_rate;
        smgr_req->Item[0].SensorId = sensor_info->sensor_id;
        smgr_req->Item[0].DataType = sensor_info->data_type;

#endif
        smgr_req->ReportId = 0;
        smgr_req->Action = (msg_data->msg_type == DSPS_ENABLE_REQ) ?
          SNS_SMGR_REPORT_ACTION_ADD_V01 : SNS_SMGR_REPORT_ACTION_DELETE_V01;
        smgr_req->BufferFactor = 1;
        smgr_req->Item_len = 1;
        smgr_req->Item[0].Sensitivity = 0; // Default
        smgr_req->Item[0].Decimation = SNS_SMGR_DECIMATION_RECENT_SAMPLE_V01;
        smgr_req->Item[0].MinSampleRate = 0;
        smgr_req->Item[0].StationaryOption = SNS_SMGR_REST_OPTION_NO_REPORT_V01;
        smgr_req->Item[0].DoThresholdTest = 0;
        smgr_req->Item[0].ThresholdOutsideMinMax = 0;
        smgr_req->Item[0].ThresholdDelta = 0;
        smgr_req->Item[0].ThresholdAllAxes = 0;
        smgr_req->Item[0].ThresholdMinMax[0] = 0;
        smgr_req->Item[0].ThresholdMinMax[1] = 0;
        IS_LOW("SAMPLE RATE LIGHT: %d",smgr_req->ReportRate);
        break;
      default:
        IS_ERR("Sensor Not Supported");
        break;
    }
  }

  return smgr_req;
}

/** dsps_prepare_smgr_req_header:
 *    @dsps_obj: dsps object (client handle)
 *    @msg_data: request message data
 *    @req_hdr: request header
 *
 * This function fills in the request header.
 **/
static int dsps_prepare_smgr_req_header(sensor1_config_t *dsps_config,
  sensor1_req_data_t *msg_data, sensor1_msg_header_s *req_hdr)
{
  int rc = 0;

  boolean is_valid = FALSE;
  dsps_sensor_info_t *sensor_info;

  if(!dsps_config || !msg_data || !req_hdr) {
    IS_ERR("Input Validation Failed");
    return -1;
  }
  sensor_info = &dsps_config->sensor_info[msg_data->sensor_type];

  if(sensor_info->present == TRUE) {
    /*Sensor is Available, Check for Supported Type*/
    if(msg_data->msg_type | sensor_info->req_supported) {
      is_valid = TRUE;
    }
  }

  if(is_valid) {
    switch(msg_data->sensor_type) {
      case DSPS_DATA_TYPE_LIGHT:
        req_hdr->service_number = SNS_SMGR_SVC_ID_V01;
        req_hdr->msg_id = SNS_SMGR_REPORT_REQ_V01;
        req_hdr->msg_size = sizeof(sns_smgr_periodic_report_req_msg_v01);
        req_hdr->txn_id = DSPS_TXN_ID_LIGHT;
        break;
      default:
        IS_ERR("Sensor Not Supported");
        rc = -1;
        break;
    }
  } else {
    IS_ERR("Sensor Not Available/Supported");
    rc = -1;
  }

  return rc;
}


/** dsps_prepare_req_header_gyro:
 *    @req_hdr: request header
 *    @msg_data: request message data
 *
 * This function prepares the header for the request message.
 **/
static int dsps_prepare_req_header_gyro(sensor1_msg_header_s *req_hdr,
    sensor1_req_data_t *msg_data)
{
  int rc = 0;
  /* Prepare Message Header */
  switch (msg_data->msg_type) {
  case DSPS_ENABLE_REQ:
  case DSPS_DISABLE_REQ:
    req_hdr->service_number = SNS_SMGR_SVC_ID_V01;
    req_hdr->msg_id = SNS_SMGR_BUFFERING_REQ_V01;
    req_hdr->msg_size = sizeof(sns_smgr_buffering_req_msg_v01);
    req_hdr->txn_id = DSPS_TXN_ID_GYRO;
    break;

  case DSPS_GET_REPORT:
    req_hdr->service_number = SNS_SMGR_SVC_ID_V01;
    req_hdr->msg_id = SNS_SMGR_BUFFERING_QUERY_REQ_V01;
    req_hdr->msg_size = sizeof(sns_smgr_buffering_query_req_msg_v01);
    req_hdr->txn_id = msg_data->u.gyro.seqnum;
    break;

  case DSPS_TIMESTAMP_REQ:
    req_hdr->service_number = SNS_TIME2_SVC_ID_V01;
    req_hdr->msg_id = SNS_TIME_TIMESTAMP_REQ_V02;
    req_hdr->msg_size = sizeof(sns_time_timestamp_req_msg_v02);
    req_hdr->txn_id = DSPS_TXN_ID_GYRO + 1;
    break;

  default:
     IS_ERR("Invalid type");
    rc = -1;
  }
  return rc;
}

/** dsps_prepare_req_header_gyro_af
 *    @req_hdr: request header
 *    @msg_data: request message data
 *
 * This function prepares the header for the request message.
 **/
static int dsps_prepare_req_header_gyro_af(sensor1_msg_header_s *req_hdr,
    sensor1_req_data_t *msg_data)
{
  int rc = 0;
  /* Prepare Message Header */
  switch (msg_data->msg_type) {
  case DSPS_ENABLE_REQ:
  case DSPS_DISABLE_REQ:
    req_hdr->service_number = SNS_SMGR_SVC_ID_V01;
    req_hdr->msg_id = SNS_SMGR_BUFFERING_REQ_V01;
    req_hdr->msg_size = sizeof(sns_smgr_buffering_req_msg_v01);
    req_hdr->txn_id = DSPS_TXN_ID_GYRO_AF;
    break;

  case DSPS_GET_REPORT:
    req_hdr->service_number = SNS_SMGR_SVC_ID_V01;
    req_hdr->msg_id = SNS_SMGR_BUFFERING_QUERY_REQ_V01;
    req_hdr->msg_size = sizeof(sns_smgr_buffering_query_req_msg_v01);
    req_hdr->txn_id = msg_data->u.gyro.seqnum;
    break;

  case DSPS_TIMESTAMP_REQ:
    req_hdr->service_number = SNS_TIME2_SVC_ID_V01;
    req_hdr->msg_id = SNS_TIME_TIMESTAMP_REQ_V02;
    req_hdr->msg_size = sizeof(sns_time_timestamp_req_msg_v02);
    req_hdr->txn_id = DSPS_TXN_ID_GYRO_AF + 1;
    break;

  default:
     IS_ERR("Invalid type");
    rc = -1;
  }
  return rc;
}

/*===========================================================================
 * FUNCTION      dsps_prepare_req_header_gravity
 *
 * DESCRIPTION   Prepare header for a request message
 *==========================================================================*/
static int dsps_prepare_req_header_gravity(sensor1_msg_header_s *req_hdr,
    sensor1_req_data_t *msg_data)
{
  int rc = 0;
  /* Prepare Message Header */
  switch (msg_data->msg_type) {
    case DSPS_ENABLE_REQ:
      req_hdr->service_number = SNS_SAM_GRAVITY_VECTOR_SVC_ID_V01;
      req_hdr->msg_id = SNS_SAM_GRAVITY_ENABLE_REQ_V01;
      req_hdr->msg_size = sizeof(sns_sam_gravity_enable_req_msg_v01);
      req_hdr->txn_id = DSPS_TXN_ID_GRAVITY;
      break;
    case DSPS_DISABLE_REQ:
      req_hdr->service_number = SNS_SAM_GRAVITY_VECTOR_SVC_ID_V01;
      req_hdr->msg_id = SNS_SAM_GRAVITY_DISABLE_REQ_V01;
      req_hdr->msg_size = sizeof(sns_sam_gravity_disable_req_msg_v01);
      req_hdr->txn_id = DSPS_TXN_ID_GRAVITY + 1;
      break;
    case DSPS_GET_REPORT:
      req_hdr->service_number = SNS_SAM_GRAVITY_VECTOR_SVC_ID_V01;
      req_hdr->msg_id = SNS_SAM_GRAVITY_GET_REPORT_REQ_V01;
      req_hdr->msg_size = sizeof(sns_sam_gravity_get_report_req_msg_v01);
      req_hdr->txn_id = DSPS_TXN_ID_GRAVITY + 2;
      break;
    default:
       IS_ERR("Invalid type");
      rc = -1;
  }
  return rc;
}

/*===========================================================================
 * FUNCTION      dsps_prepare_req_header_gravity_without_gyro
 *
 * DESCRIPTION   Prepare header for a request message when gyro sensor is not
                 present. Gravity data in this case is derived from orientation
 *==========================================================================*/
static int dsps_prepare_req_header_gravity_without_gyro(sensor1_msg_header_s *req_hdr,
    sensor1_req_data_t *msg_data)
{
  int rc = 0;
  /* Prepare Message Header */
  switch (msg_data->msg_type) {
    case DSPS_ENABLE_REQ:
      req_hdr->service_number = SNS_SAM_ORIENTATION_SVC_ID_V01;
      req_hdr->msg_id = SNS_SAM_ORIENTATION_ENABLE_REQ_V01;
      req_hdr->msg_size = sizeof(sns_sam_orientation_enable_req_msg_v01);
      req_hdr->txn_id = DSPS_TXN_ID_GRAVITY;
      break;
    case DSPS_DISABLE_REQ:
      req_hdr->service_number = SNS_SAM_ORIENTATION_SVC_ID_V01;
      req_hdr->msg_id = SNS_SAM_ORIENTATION_DISABLE_REQ_V01;
      req_hdr->msg_size = sizeof(sns_sam_orientation_disable_req_msg_v01);
      req_hdr->txn_id = DSPS_TXN_ID_GRAVITY + 1;
      break;
    case DSPS_GET_REPORT:
      req_hdr->service_number = SNS_SAM_ORIENTATION_SVC_ID_V01;
      req_hdr->msg_id = SNS_SAM_ORIENTATION_GET_REPORT_REQ_V01;
      req_hdr->msg_size = sizeof(sns_sam_orientation_get_report_req_msg_v01);
      req_hdr->txn_id = DSPS_TXN_ID_GRAVITY + 2;
      break;
    default:
       IS_ERR("Invalid type");
      rc = -1;
  }
  return rc;
}


/*===========================================================================
 * FUNCTION      dsps_prepare_req_header
 *
 * DESCRIPTION   Prepare header for a request message
 *==========================================================================*/
static int dsps_prepare_req_header(sensor1_config_t *dsps_config,
    sensor1_msg_header_s *req_hdr, sensor1_req_data_t *msg_data)
{
  int rc = 0;
  switch (msg_data->sensor_type) {
  case DSPS_DATA_TYPE_GYRO:
    rc = dsps_prepare_req_header_gyro(req_hdr, msg_data);
    break;
   case DSPS_DATA_TYPE_GYRO_AF:
    rc = dsps_prepare_req_header_gyro_af(req_hdr, msg_data);
    break;
  case DSPS_DATA_TYPE_GRAVITY_VECTOR:
    if(dsps_config->sensor_info[DSPS_DATA_TYPE_GYRO].present) {
      rc = dsps_prepare_req_header_gravity(req_hdr, msg_data);
    } else {
      rc = dsps_prepare_req_header_gravity_without_gyro(req_hdr, msg_data);
    }
    break;
  case DSPS_DATA_TYPE_LIGHT:
    rc = dsps_prepare_smgr_req_header(dsps_config, msg_data, req_hdr);
    break;
  default:
     IS_ERR("Sensor type %d not supported yet!",
      msg_data->sensor_type);
    rc = -1;
    break;
  }
  return rc;
}

/** dsps_prepare_req_msg_gyro:
 *    @dsps_obj: dsps object (client handle)
 *    @msg_data: request message data
 *
 * This function fills in the request message.
 * This function assumes that sensor1_alloc_msg_buf returns 0-initialized
 * memory (calloc).
 **/
static void *dsps_prepare_req_msg_gyro(sensor1_config_t *dsps_obj,
  sensor1_req_data_t *msg_data)
{
  int size = 0;
  sensor1_error_e err;
  void *req_msg = NULL;
  sns_smgr_buffering_req_msg_v01 *req;
  sns_smgr_buffering_query_req_msg_v01 *query_req;

  if (msg_data->msg_type == DSPS_GET_REPORT) {
    size = sizeof(sns_smgr_buffering_query_req_msg_v01);
    err = sensor1_alloc_msg_buf(dsps_obj->handle, size, (void *)&query_req);
    if (err == SENSOR1_SUCCESS) {
      query_req->QueryId = SNS_SMGR_ID_GYRO_V01;
      query_req->QueryId |= (uint16_t)(msg_data->u.gyro.seqnum << 8) & 0xFF00;
      query_req->SensorId = SNS_SMGR_ID_GYRO_V01;
      query_req->DataType = SNS_SMGR_DATA_TYPE_PRIMARY_V01;
      query_req->TimePeriod[0] =
        (msg_data->u.gyro.t_start - dsps_obj->dsps_time_state.ts_offset) *
        NSEC_PER_USEC * DSPS_HZ / NSEC_PER_SEC;
      query_req->TimePeriod[1] =
        (msg_data->u.gyro.t_end - dsps_obj->dsps_time_state.ts_offset) *
        NSEC_PER_USEC * DSPS_HZ / NSEC_PER_SEC;
       IS_LOW("QueryId = 0x%x, seqnum = 0x%x, t_start = %llu, t_end = %llu,"
        " ts_offset = %llu", query_req->QueryId,
        msg_data->u.gyro.seqnum, msg_data->u.gyro.t_start,
        msg_data->u.gyro.t_end, dsps_obj->dsps_time_state.ts_offset);
       IS_LOW("TimePeriod[0] = %u, TimePeriod[1] = %u",
        query_req->TimePeriod[0], query_req->TimePeriod[1]);
      req_msg = query_req;
    }
  } else if (msg_data->msg_type == DSPS_TIMESTAMP_REQ) {
    size = sizeof(sns_time_timestamp_req_msg_v02);
    err = sensor1_alloc_msg_buf(dsps_obj->handle, size, &req_msg);
    if (err != SENSOR1_SUCCESS) {
      req_msg = NULL;
    }
  } else if (msg_data->msg_type == DSPS_ENABLE_REQ ||
    msg_data->msg_type == DSPS_DISABLE_REQ) {
    size = sizeof(sns_smgr_buffering_req_msg_v01);
    err = sensor1_alloc_msg_buf(dsps_obj->handle, size, (void *)&req);
    if (err == SENSOR1_SUCCESS) {
      req->ReportId = SNS_SMGR_ID_GYRO_V01;
      if (msg_data->msg_type == DSPS_ENABLE_REQ) {
        req->Action                     = SNS_SMGR_BUFFERING_ACTION_ADD_V01;
        req->ReportRate                 = 0;
        req->Item_len                   = 1;
        req->Item[0].SensorId           = SNS_SMGR_ID_GYRO_V01;
        req->Item[0].DataType           = SNS_SMGR_DATA_TYPE_PRIMARY_V01;
        req->Item[0].Decimation         = SNS_SMGR_DECIMATION_FILTER_V01;
        req->Item[0].Calibration        = SNS_SMGR_CAL_SEL_FULL_CAL_V01;
        req->Item[0].SamplingRate       = msg_data->u.gyro.sample_rate;
        req->Item[0].SampleQuality      = SNS_SMGR_SAMPLE_QUALITY_ACCURATE_TIMESTAMP_V01;
        req->notify_suspend_valid       = FALSE;
        req->SrcModule_valid            = FALSE;
        IS_LOW("SR = %d, RR = %d", req->Item[0].SamplingRate, req->ReportRate);
      } else {
        req->Action = SNS_SMGR_BUFFERING_ACTION_DELETE_V01;
      }
      req_msg = req;
    }
  } else {
     IS_ERR("Invalid type");
  }

  return req_msg;
}


/** dsps_prepare_req_msg_gyro_af:
 *    @dsps_obj: dsps object (client handle)
 *    @msg_data: request message data
 *
 * This function fills in the request message.
 * This function assumes that sensor1_alloc_msg_buf returns 0-initialized
 * memory (calloc).
 **/
static void *dsps_prepare_req_msg_gyro_af(sensor1_config_t *dsps_obj,
  sensor1_req_data_t *msg_data)
{
  IS_LOW("E");

  int size = 0;
  sensor1_error_e err;
  void *req_msg = NULL;
  sns_smgr_buffering_req_msg_v01 *req;
  sns_smgr_buffering_query_req_msg_v01 *query_req;

  if (msg_data->msg_type == DSPS_ENABLE_REQ ||
    msg_data->msg_type == DSPS_DISABLE_REQ) {
    size = sizeof(sns_smgr_buffering_req_msg_v01);
    err = sensor1_alloc_msg_buf(dsps_obj->handle, size, (void *)&req);
    if (err == SENSOR1_SUCCESS) {
      req->ReportId = SNS_SMGR_ID_GYRO_V01 + 1;
      if (msg_data->msg_type == DSPS_ENABLE_REQ) {
        req->Action                     = SNS_SMGR_BUFFERING_ACTION_ADD_V01;
        req->ReportRate                 = msg_data->u.gyro.report_rate;
        req->Item_len                   = 1;
        req->Item[0].SensorId           = SNS_SMGR_ID_GYRO_V01;
        req->Item[0].DataType           = SNS_SMGR_DATA_TYPE_PRIMARY_V01;
        req->Item[0].Decimation         = SNS_SMGR_DECIMATION_FILTER_V01;
        req->Item[0].Calibration        = SNS_SMGR_CAL_SEL_FULL_CAL_V01;
        req->Item[0].SamplingRate       = msg_data->u.gyro.sample_rate;
        req->Item[0].SampleQuality      = SNS_SMGR_SAMPLE_QUALITY_ACCURATE_TIMESTAMP_V01;
        req->notify_suspend_valid       = FALSE;
        req->SrcModule_valid            = FALSE;
        IS_LOW("SR = %d, RR = %d", req->Item[0].SamplingRate, req->ReportRate);
      } else {
        req->Action = SNS_SMGR_BUFFERING_ACTION_DELETE_V01;
      }
      req_msg = req;
    }
  } else {
     IS_ERR("Invalid type");
  }

  return req_msg;
}

/*===========================================================================
 * FUNCTION      dsps_prepare_req_msg_gravity
 *
 * DESCRIPTION   Prepare body of a request message
 *==========================================================================*/
static void *dsps_prepare_req_msg_gravity(sensor1_config_t *dsps_config,
  sensor1_req_data_t *msg_data)
{
  int size = 0;
  int rc = 0;
  sensor1_error_e error;
  void *req_msg = NULL;

   IS_LOW("Prepare Request message of type : %d",
    msg_data->msg_type);

  switch (msg_data->msg_type) {
  case DSPS_ENABLE_REQ: {
    sns_sam_gravity_enable_req_msg_v01 *enable_req_msg;
    size = sizeof(sns_sam_gravity_enable_req_msg_v01);
    error = sensor1_alloc_msg_buf(dsps_config->handle, size, &req_msg);
    if (error != SENSOR1_SUCCESS) {
       IS_ERR("DSPS_ENABLE_REQ: Error allocating buffer %d\n",
        error);
      return NULL;
    }

    /* Gravity vector enable request message has following fields:
     * Mandatory -
     * report_period: ouput rate - units of seconds, Q16. 0 to report
     *   at sampling rate.
     * Optional -
     * sample_rate_valid: true if sample rate to be passed.
     * sample_rate: in Hz, Q16. If less than report rate, set to report rate
     * notify_suspend_valid: true if notify_suspend is being passed.
     * notify_suspend: send indicaiton for the request when the processor
     *   is in suspend state.
     **/
    enable_req_msg = (sns_sam_gravity_enable_req_msg_v01 *)req_msg;
    enable_req_msg->report_period = msg_data->u.gravity.report_period;
    if (msg_data->u.gravity.sample_rate_valid) {
      enable_req_msg->sample_rate_valid = TRUE;
      enable_req_msg->sample_rate = msg_data->u.gravity.sample_rate;
    }
  }
    break;

  case DSPS_DISABLE_REQ: {
    sns_sam_gravity_disable_req_msg_v01 *disable_req_msg;
    size = sizeof(sns_sam_gravity_disable_req_msg_v01);
    error = sensor1_alloc_msg_buf(dsps_config->handle, size, &req_msg);
    if (error != SENSOR1_SUCCESS) {
       IS_ERR("DSPS_DISABLE_REQ: Error allocating buffer %d\n",
         error);
      return NULL;
    }
    disable_req_msg = (sns_sam_gravity_disable_req_msg_v01 *)req_msg;
    disable_req_msg->instance_id = dsps_config->instance_id_gravity;
  }
    break;

  case DSPS_GET_REPORT: {
    sns_sam_gravity_get_report_req_msg_v01 *get_report_req_msg;
    size = sizeof(sns_sam_gravity_get_report_req_msg_v01);
    error = sensor1_alloc_msg_buf(dsps_config->handle, size, &req_msg);
    if (error != SENSOR1_SUCCESS) {
       IS_ERR("DSPS_GET_REPORT: Error allocating buffer %d\n",
        error);
      return NULL;
    }

    get_report_req_msg = (sns_sam_gravity_get_report_req_msg_v01 *)req_msg;
    get_report_req_msg->instance_id = dsps_config->instance_id_gravity;
  }
    break;

  default:
     IS_ERR("Invalid type");
    return NULL;
  }

  return req_msg;
} /* dsps_prepare_req_msg_gravity */


/*===========================================================================
 * FUNCTION      dsps_prepare_req_msg_gravity_without_gyro
 *
 * DESCRIPTION   Prepare body of a request message in the case when gyro is
                 not present. In this case, gravity data is derived from orientation
 *==========================================================================*/
static void *dsps_prepare_req_msg_gravity_without_gyro(sensor1_config_t *dsps_config,
  sensor1_req_data_t *msg_data)
{
  int size = 0;
  int rc = 0;
  sensor1_error_e error;
  void *req_msg = NULL;
  IS_LOW("Prepare Request message of type : %d",
    msg_data->msg_type);

  switch (msg_data->msg_type) {
  case DSPS_ENABLE_REQ: {
    sns_sam_orientation_enable_req_msg_v01 *enable_req_msg = NULL;
    size = sizeof(sns_sam_orientation_enable_req_msg_v01);
    error = sensor1_alloc_msg_buf(dsps_config->handle, size, &req_msg);
    if(error != SENSOR1_SUCCESS) {
       IS_ERR("DSPS_ENABLE_REQ: Error allocating buffer %d\n",
        error);
      return NULL;
    }

    /* Gravity vector enable request message has following fields:
     * Mandatory -
     * report_period: ouput rate - units of seconds, Q16. 0 to report
     *   at sampling rate.
     * Optional -
     * sample_rate_valid: true if sample rate to be passed.
     * sample_rate: in Hz, Q16. If less than report rate, set to report rate
     * notify_suspend_valid: true if notify_suspend is being passed.
     * notify_suspend: send indicaiton for the request when the processor
     *   is in suspend state.
     **/
    enable_req_msg = (sns_sam_orientation_enable_req_msg_v01 *)req_msg;
    enable_req_msg->report_period = msg_data->u.gravity.report_period;
    if(msg_data->u.gravity.sample_rate_valid) {
      enable_req_msg->sample_rate_valid = TRUE;
      enable_req_msg->sample_rate = msg_data->u.gravity.sample_rate;
    }
  }
    break;

  case DSPS_DISABLE_REQ: {
    sns_sam_orientation_disable_req_msg_v01 *disable_req_msg = NULL;
    size = sizeof(sns_sam_orientation_disable_req_msg_v01);
    error = sensor1_alloc_msg_buf(dsps_config->handle, size, &req_msg);
    if(error != SENSOR1_SUCCESS) {
       IS_ERR("DSPS_DISABLE_REQ: Error allocating buffer %d\n",
         error);
      return NULL;
    }
    disable_req_msg = (sns_sam_orientation_disable_req_msg_v01 *)req_msg;
    disable_req_msg->instance_id = dsps_config->instance_id_gravity;
  }
    break;

  case DSPS_GET_REPORT: {
    sns_sam_orientation_get_report_req_msg_v01 *get_report_req_msg = NULL;
    size = sizeof(sns_sam_orientation_get_report_req_msg_v01);
    error = sensor1_alloc_msg_buf(dsps_config->handle, size, &req_msg);
    if(error != SENSOR1_SUCCESS) {
       IS_ERR("DSPS_GET_REPORT: Error allocating buffer %d\n",
        error);
      return NULL;
    }
    get_report_req_msg = (sns_sam_orientation_get_report_req_msg_v01 *)req_msg;
    get_report_req_msg->instance_id = dsps_config->instance_id_gravity;
  }
    break;

  default:
     IS_ERR("Invalid type");
     return NULL;
  }

  return req_msg;
} /* dsps_prepare_req_msg_gravity_without_gyro */



/*===========================================================================
 * FUNCTION      dsps_prepare_req_msg
 *
 * DESCRIPTION   Prepare body of a request message
 *==========================================================================*/
void *dsps_prepare_req_msg(sensor1_config_t *dsps_config,
    sensor1_req_data_t *msg_data)
{
  int size = 0;
  void *req_msg = NULL;
  switch (msg_data->sensor_type) {
  case DSPS_DATA_TYPE_GYRO:
    req_msg = dsps_prepare_req_msg_gyro(dsps_config, msg_data);
    break;
  case DSPS_DATA_TYPE_GYRO_AF:
    req_msg = dsps_prepare_req_msg_gyro_af(dsps_config, msg_data);
    break;
  case DSPS_DATA_TYPE_GRAVITY_VECTOR:
    if(dsps_config->sensor_info[DSPS_DATA_TYPE_GYRO].present) {
      req_msg = dsps_prepare_req_msg_gravity(dsps_config, msg_data);
    } else {
      req_msg = dsps_prepare_req_msg_gravity_without_gyro(dsps_config, msg_data);
    }
    break;
  case DSPS_DATA_TYPE_LIGHT:
    req_msg = dsps_prepare_smgr_req_msg(dsps_config, msg_data);
    break;
  default:
     IS_ERR("Sensor type %d not supported yet!",
      msg_data->sensor_type);
    break;
  }

  return req_msg;
} /* dsps_prepare_req_msg_gravity */


/*===========================================================================
 * FUNCTION      dsps_send_request
 *
 * DESCRIPTION   Send a request message to the sensor framework.
 *               Typically used for adding and deleting reports
 *==========================================================================*/
int dsps_send_request(void *sensor_config,
  void *req_data, int wait)
{
  int32_t ret_val = 0;
  sensor1_error_e error;
  sensor1_msg_header_s req_hdr;
  void *req_msg = NULL;
  sensor1_config_t *dsps_config = (sensor1_config_t *)sensor_config;
  sensor1_req_data_t *msg_data = (sensor1_req_data_t *)req_data;

  req_msg = dsps_prepare_req_msg(dsps_config, msg_data);
  if (req_msg == NULL) {
     IS_ERR("Error preparing request message!");
    return -1;
  }
  ret_val = dsps_prepare_req_header(dsps_config, &req_hdr, msg_data);

  if (!ret_val) {
    ret_val = dsps_send_req(dsps_config, req_msg, &req_hdr, wait);
  }

  return ret_val;
}


/*===========================================================================
 * FUNCTION      dsps_handle_broken_pipe
 *
 * DESCRIPTION   Handle error condition of broken pipe with the sensor
 *               framework
 *==========================================================================*/
void dsps_handle_broken_pipe(sensor1_config_t *dsps_config)
{
   IS_ERR("Broken Pipe Exception\n");
  pthread_mutex_lock(&(dsps_config->thread_mutex));
  dsps_config->status = DSPS_BROKEN_PIPE;
  pthread_mutex_unlock(&(dsps_config->thread_mutex));
  pthread_cond_signal(&(dsps_config->thread_condvar));
}


/*===========================================================================
 * FUNCTION      dsps_process_repsonse_gyro
 *
 * DESCRIPTION   Process response received from sensor framework.
 *               A response message is in response to a message sent to the
 *               sensor framework. Signal waiting condition variable in
 *               dsps_wait_for_response()
 *=========================================================================*/
static void dsps_process_response_gyro(sensor1_config_t *dsps_obj,
    sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{

  switch (msg_hdr->msg_id) {
  case SNS_SMGR_BUFFERING_RESP_V01: {
    sns_smgr_buffering_resp_msg_v01 *resp = msg_ptr;
    if (resp->Resp.sns_result_t != SNS_RESULT_SUCCESS_V01) {
       IS_ERR("Request denied, error code = %d",
        resp->Resp.sns_err_t);
    }
  }
    break;

  case SNS_SMGR_BUFFERING_QUERY_RESP_V01: {
    sns_smgr_buffering_query_resp_msg_v01 *resp = msg_ptr;
    if (resp->Resp.sns_result_t != SNS_RESULT_SUCCESS_V01) {
       IS_ERR("Query request denied, error code = %d, QId_valid = %d, "
        "QueryId = 0x%x, AckNak_valid = %d, AckNak = %d",
        resp->Resp.sns_err_t, resp->QueryId_valid, resp->QueryId,
        resp->AckNak_valid, resp->AckNak);
    }
  }
    break;

  case SNS_TIME_TIMESTAMP_RESP_V02: {
    sns_time_timestamp_resp_msg_v02 *resp = msg_ptr;
    if (resp->resp.sns_result_t == SNS_RESULT_SUCCESS_V01) {
      if (resp->timestamp_dsps_valid && resp->timestamp_apps_valid) {
        uint64_t apps_us = resp->timestamp_apps / NSEC_PER_USEC;
        uint64_t dsps_us = (uint64_t)resp->timestamp_dsps * USEC_PER_SEC /
          DSPS_HZ;

        dsps_obj->dsps_time_state.ts_offset = apps_us - dsps_us;
        dsps_obj->dsps_time_state.ts_offset_valid = 1;
         IS_LOW("apps_us = %llu, dsps_us = %lld, ts_offset = %llu",
           apps_us, dsps_us, dsps_obj->dsps_time_state.ts_offset);

        if (dsps_us < dsps_obj->dsps_time_state.ts_dsps_prev) {
          dsps_obj->dsps_time_state.ts_dsps_ro_cnt++;
          dsps_obj->dsps_time_state.ts_offset +=
            (dsps_obj->dsps_time_state.ts_dsps_ro_cnt * UINT32_MAX);
        }
        dsps_obj->dsps_time_state.ts_dsps_prev = dsps_us;
      }
    }
  }
    break;

  default:
     IS_ERR("Response not valid");
    break;
  }
} /* dsps_process_response_gyro */


/*===========================================================================
 * FUNCTION      dsps_process_response_gravity
 *
 * DESCRIPTION   Process response received from sensor framework.
 *               A response message is in response to a message sent to the
 *               sensor framework. Signal waiting condition variable in
 *               dsps_wait_for_response()
 *=========================================================================*/
static void dsps_process_response_gravity(sensor1_config_t *dsps_config,
    sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
  sns_sam_gravity_enable_resp_msg_v01 *enable_resp_msg;
  sns_sam_gravity_disable_resp_msg_v01 *disable_resp_msg;
  sns_sam_gravity_get_report_resp_msg_v01 *get_report_resp_msg;

  switch (msg_hdr->msg_id) {
    case SNS_SAM_GRAVITY_ENABLE_RESP_V01:
      enable_resp_msg = (sns_sam_gravity_enable_resp_msg_v01*)msg_ptr;
      if (enable_resp_msg->resp.sns_result_t == SNS_RESULT_SUCCESS_V01) {
        if (enable_resp_msg->instance_id_valid) {
          dsps_config->instance_id_gravity = enable_resp_msg->instance_id;
           IS_LOW("Ensable Response Instance ID received: %d",
             dsps_config->instance_id_gravity);
        }
      } else {
         IS_ERR("Enable Request failed (err: %d)",
          enable_resp_msg->resp.sns_err_t);
      }
      break;
    case SNS_SAM_GRAVITY_DISABLE_RESP_V01:
      disable_resp_msg = (sns_sam_gravity_disable_resp_msg_v01*)msg_ptr;
      if (disable_resp_msg->resp.sns_result_t == SNS_RESULT_SUCCESS_V01) {
        if (disable_resp_msg->instance_id_valid)
          dsps_config->instance_id_gravity = INVALID_INSTANCE_ID;
         IS_LOW("Disable Response Instance ID received: %d",
          dsps_config->instance_id_gravity);
      } else {
         IS_ERR("Disable Request failed (err: %d)",
          disable_resp_msg->resp.sns_err_t);
      }
      break;
    case SNS_SAM_GRAVITY_GET_REPORT_RESP_V01:
      get_report_resp_msg = (sns_sam_gravity_get_report_resp_msg_v01 *)msg_ptr;
      if (get_report_resp_msg->resp.sns_result_t == SNS_RESULT_SUCCESS_V01) {
         IS_LOW("Report Request Accepted\n");
      } else {
         IS_ERR("Report Request Denied\n");
      }
      break;
    default:
       IS_ERR("Response not valid");
      break;
  }
} /* dsps_process_response_gravity */

/*===========================================================================
 * FUNCTION      dsps_process_response_gravity_without_gyro
 * DESCRIPTION   Process response received from sensor framework.
 *               A response message is in response to a message sent to the
 *               sensor framework. Signal waiting condition variable in
 *               dsps_wait_for_response().
 *=========================================================================*/
static void dsps_process_response_gravity_without_gyro(sensor1_config_t *dsps_config,
    sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
  sns_sam_orientation_enable_resp_msg_v01 *enable_resp_msg = NULL;
  sns_sam_orientation_disable_resp_msg_v01 *disable_resp_msg = NULL;
  sns_sam_orientation_get_report_resp_msg_v01 *get_report_resp_msg = NULL;

  switch (msg_hdr->msg_id) {
    case SNS_SAM_ORIENTATION_ENABLE_RESP_V01:
      enable_resp_msg = (sns_sam_orientation_enable_resp_msg_v01*)msg_ptr;
      if(enable_resp_msg->resp.sns_result_t == SNS_RESULT_SUCCESS_V01) {
        if(enable_resp_msg->instance_id_valid) {
          dsps_config->instance_id_gravity = enable_resp_msg->instance_id;
          IS_LOW("Ensable Response Instance ID received: %d",
             dsps_config->instance_id_gravity);
        }
      } else {
         IS_ERR("Enable Request failed (err: %d)",
          enable_resp_msg->resp.sns_err_t);
      }
      break;
    case SNS_SAM_ORIENTATION_DISABLE_RESP_V01:
      disable_resp_msg = (sns_sam_orientation_disable_resp_msg_v01*)msg_ptr;
      if(disable_resp_msg->resp.sns_result_t == SNS_RESULT_SUCCESS_V01) {
        if(disable_resp_msg->instance_id_valid)
          dsps_config->instance_id_gravity = INVALID_INSTANCE_ID;
          IS_LOW("Disable Response Instance ID received: %d",
          dsps_config->instance_id_gravity);
      } else {
         IS_ERR("Disable Request failed (err: %d)",
          disable_resp_msg->resp.sns_err_t);
      }
      break;
    case SNS_SAM_ORIENTATION_GET_REPORT_RESP_V01:
      get_report_resp_msg = (sns_sam_orientation_get_report_resp_msg_v01 *)msg_ptr;
      if(get_report_resp_msg->resp.sns_result_t == SNS_RESULT_SUCCESS_V01) {
         IS_LOW("Report Request Accepted\n");
      } else {
         IS_ERR("Report Request Denied\n");
      }
      break;

    default:
       IS_ERR("Response not valid");
      break;
  }
} /* dsps_process_response_gravity_without_gyro */




/*===========================================================================
 * FUNCTION      dsps_process_indication_gyro
 *
 * DESCRIPTION   Process indication received from sensor framework.
 *=========================================================================*/
static void dsps_process_indication_gyro(sensor1_config_t *dsps_obj,
    sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
  sns_smgr_buffering_query_ind_msg_v01 *indication = msg_ptr;

  switch (msg_hdr->msg_id) {
  case SNS_SMGR_BUFFERING_QUERY_IND_V01: {
    dsps_cb_data_t cb_data;
    dsps_get_gyro_samples(indication, dsps_obj, &cb_data);
    cb_data.cb_type = DSPS_SENSOR_DATA;
    cb_data.type = DSPS_DATA_TYPE_GYRO;
    cb_data.u.gyro.seq_no = (indication->QueryId & 0xFF00) >> 8;
    dsps_obj->dsps_callback(dsps_obj->port, &cb_data);
  }
    break;

  case SNS_SMGR_BUFFERING_IND_V01: {
    IS_LOW("Gyro AF indication SNS_SMGR_BUFFERING_IND_V01");
    dsps_cb_data_t cb_data;
    dsps_get_gyro_samples_af(msg_ptr, dsps_obj, &cb_data);
    cb_data.cb_type = DSPS_SENSOR_DATA;
    cb_data.type = DSPS_DATA_TYPE_GYRO_AF;
    dsps_obj->dsps_callback(dsps_obj->port, &cb_data);
  }
  break;
  default:
     IS_ERR("Invalid Indication ID\n");
    break;
  }
} /* dsps_process_response_gyro */


/*===========================================================================
 * FUNCTION      dsps_process_indication_gravity
 *
 * DESCRIPTION   Process indication received from sensor framework.
 *=========================================================================*/
static void dsps_process_indication_gravity(sensor1_config_t *dsps_config,
  sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
  switch(msg_hdr->msg_id) {
  case SNS_SAM_GRAVITY_REPORT_IND_V01: {
    sns_sam_gravity_report_ind_msg_v01 *indication =
        (sns_sam_gravity_report_ind_msg_v01 *) msg_ptr;
    sns_sam_gravity_result_s_v01 *gravity_data =
      &indication->result;
    dsps_cb_data_t cb_data;

    cb_data.type = DSPS_DATA_TYPE_GRAVITY_VECTOR;
    STATS_MEMCPY(&cb_data.u.gravity.gravity, sizeof(cb_data.u.gravity.gravity),
      gravity_data->gravity, sizeof(gravity_data->gravity));
    STATS_MEMCPY(&cb_data.u.gravity.lin_accel, sizeof(cb_data.u.gravity.lin_accel),
      gravity_data->lin_accel, sizeof(gravity_data->lin_accel));
    cb_data.cb_type = DSPS_SENSOR_DATA;
    cb_data.u.gravity.accuracy = gravity_data->accuracy;
    dsps_config->dsps_callback(dsps_config->port, &cb_data);
  }
    break;
  default:
     IS_ERR("Invalid Indication ID\n");
    break;
  }
} /* dsps_process_indication_gravity */

/*===========================================================================
 * FUNCTION      dsps_process_indication_gravity_without_gyro
 *
 * DESCRIPTION   Process indication received from sensor framework.
 *=========================================================================*/
static void dsps_process_indication_gravity_without_gyro(sensor1_config_t *dsps_config,
  sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
  switch(msg_hdr->msg_id) {
  case SNS_SAM_ORIENTATION_REPORT_IND_V01: {
    sns_sam_orientation_report_ind_msg_v01 *indication =
        (sns_sam_orientation_report_ind_msg_v01 *) msg_ptr;
    sns_sam_orientation_result_s_v01 *gravity_data =
        &indication->result;
    dsps_cb_data_t cb_data;

    cb_data.cb_type = DSPS_SENSOR_DATA;
    cb_data.type = DSPS_DATA_TYPE_GRAVITY_VECTOR;
    memcpy(&cb_data.u.gravity.gravity, gravity_data->gravity,
      sizeof(float) * 3);
    memcpy(&cb_data.u.gravity.lin_accel, gravity_data->lin_accel,
      sizeof(float) *3);
    IS_LOW("Gravity ( %f %f %f ) Lin_Accel ( %f %f %f )", cb_data.u.gravity.gravity[0],
      cb_data.u.gravity.gravity[1], cb_data.u.gravity.gravity[2], cb_data.u.gravity.lin_accel[0],
        cb_data.u.gravity.lin_accel[1], cb_data.u.gravity.lin_accel[2]);
    cb_data.u.gravity.accuracy = gravity_data->gravity_accuracy;
    dsps_config->dsps_callback(dsps_config->port, &cb_data);
  }
  break;

  default:
     IS_ERR("Invalid Indication ID\n");
    break;
  }
} /* dsps_process_indication_gravity_without_gyro */


/*===========================================================================
 * FUNCTION      dsps_process_rep_light
 *  @dsps_config:   dsps handle
 *  @msg_hdr:       Message header pointer from the response
 *  @msg_ptr:       Message pointer from the response
 * DESCRIPTION   Process indication/response received from sensor framework.
 *               A response message is in response to a message sent to the
 *               sensor framework. Signal waiting condition variable in
 *               dsps_wait_for_response()
 *=========================================================================*/
static int32_t dsps_process_rep_light(sensor1_config_t *dsps_config,
    sensor1_msg_header_s *msg_hdr, void *msg_ptr)
{
  int32_t retval = 0;
  if(!dsps_config->sensor_info[DSPS_DATA_TYPE_LIGHT].present) {
    /* Sensor Not Present*/
    return retval;
  }

  switch (msg_hdr->msg_id) {
    case SNS_SMGR_REPORT_RESP_V01:
      /*Nothing to do in Sensor Info Response*/
      /*may require to handle in future*/
      break;

    case SNS_SMGR_REPORT_IND_V01: {
      sns_smgr_data_item_s_v01 *item = msg_ptr;
      dsps_cb_data_t cb_data;
      IS_LOW("Indication SensorId:%02d DataType:%d "
        "ItemData[0]:%-10d ItemData[1]:%-10d ItemData[2]:%-10d",
        item->SensorId, item->DataType, item->ItemData[0],
        item->ItemData[1], item->ItemData[2]);

      cb_data.cb_type = DSPS_SENSOR_DATA;
      cb_data.type = DSPS_DATA_TYPE_LIGHT;
      /*TODO: Need to check the conversion of light value to lux*/
      cb_data.u.light.lux = ((float)item->ItemData[0] * UNIT_CONVERT_LIGHT);
      dsps_config->dsps_callback(dsps_config->port, &cb_data);
    }
      break;
  }
  return retval;
} /* dsps_process_response_gyro */


/*===========================================================================
 * FUNCTION      dsps_handle_smgr_rep_response
 *  @dsps_obj:      dsps handle
 *  @msg_hdr:       Message header pointer from the response
 *  @resp_msg:      Message pointer from the response
 * DESCRIPTION   Handle SMGR report messages(response).
 *==========================================================================*/
int32_t dsps_handle_smgr_rep_response(sensor1_config_t *dsps_obj,
  sensor1_msg_header_s *msg_hdr,
  sns_smgr_periodic_report_resp_msg_v01 *resp_msg)
{
  int32_t ret_val = 0;

  if(NULL == dsps_obj || NULL == msg_hdr || NULL == resp_msg) {
    /* Invalid Inputs */
    IS_ERR("Input Validation dsps_obj %x, msg_hdr %x, resp_msg %x",
       dsps_obj, msg_hdr, resp_msg);
    ret_val = -1;
    return ret_val;
  }
  /*TODO: Handle Report Response*/

  return ret_val;
}


/*===========================================================================
 * FUNCTION      dsps_handle_smgr_rep_indication
 *  @dsps_obj:      dsps handle
 *  @msg_hdr:       Message header pointer from the response
 *  @ind_msg:       Message pointer from the indication
 * DESCRIPTION   Handle SMGR report messages(indications).
 *==========================================================================*/
 int32_t dsps_handle_smgr_rep_indication(sensor1_config_t *dsps_obj,
  sensor1_msg_header_s *msg_hdr,
  sns_smgr_periodic_report_ind_msg_v01 *ind_msg)
{
  int32_t ret_val = 0;
  sensor1_handle_s *handle = NULL;
  uint32_t i = 0;
  uint8_t data_type;

  if(NULL == dsps_obj || NULL == msg_hdr || NULL == ind_msg) {
    /* Invalid Inputs */
    IS_ERR("Input Validation dsps_obj %x, msg_hdr %x, msg_ptr %x",
       dsps_obj, msg_hdr, ind_msg);
    ret_val = -1;
    return ret_val;
  }

  handle = dsps_obj->handle;

  for(i = 0; i < ind_msg->Item_len; i++)
  {
    IS_LOW("Indication (%02d of %02d) SensorId:%02d DataType:%d "
      "ItemData[0]:%-10d ItemData[1]:%-10d ItemData[2]:%-10d",
       i+1, ind_msg->Item_len, ind_msg->Item[i].SensorId,
       ind_msg->Item[i].DataType, ind_msg->Item[i].ItemData[0],
       ind_msg->Item[i].ItemData[1], ind_msg->Item[i].ItemData[2]);

    data_type = ind_msg->Item[i].DataType;
    switch(ind_msg->Item[i].SensorId){
      case SNS_SMGR_ID_PROX_LIGHT_V01:
        if(data_type == SNS_SMGR_DATA_TYPE_SECONDARY_V01) {
          /*ALS Indication Received*/
          IS_LOW("ALS Indication Received");
          dsps_process_rep_light(dsps_obj, msg_hdr, &ind_msg->Item[i]);
        }
        break;
      default:
        /*Nothing to Do*/
        break;
    }
  }

  return ret_val;
}

/*===========================================================================
 * FUNCTION      dsps_handle_smgr_response
 *  @dsps_obj:      dsps handle
 *  @msg_hdr:       Message header pointer from the response
 *  @msg_ptr:       Message pointer from the response
 * DESCRIPTION   Handle SMGR response messages.
 *==========================================================================*/
int32_t dsps_handle_smgr_response(sensor1_config_t *dsps_obj,
  sensor1_msg_header_s *msg_hdr, void *msg_ptr) {

  int32_t retval = 0;
  switch(msg_hdr->msg_id) {
    case SNS_SMGR_ALL_SENSOR_INFO_RESP_V01:
      IS_LOW("All Sensor Info Received");
      dsps_handle_smgr_all_sensor_info_resp(dsps_obj, msg_ptr);
      break;

    case SNS_SMGR_SINGLE_SENSOR_INFO_RESP_V01:
      IS_LOW("Single Sensor Info Received");
      dsps_handle_smgr_single_sensor_info_resp(dsps_obj, msg_ptr);
      break;

    case SNS_SMGR_BUFFERING_QUERY_RESP_V01:
    case SNS_SMGR_BUFFERING_RESP_V01:
      IS_LOW("Gyro Response Received");
      /*TODO: Handle this case with Sensor ID*/
      dsps_process_response_gyro(dsps_obj, msg_hdr, msg_ptr);
      break;

    case SNS_SMGR_REPORT_RESP_V01:
      IS_LOW("SNS_SMGR_REPORT_RESP_V01 Received");
      retval = dsps_handle_smgr_rep_response(dsps_obj, msg_hdr, msg_ptr);
      break;

    default:
      IS_LOW("Message Not Supported MsgID %d", msg_hdr->msg_id);
      break;
  }
  return retval;
}


/*===========================================================================
 * FUNCTION      dsps_handle_smgr_indication
 *  @dsps_obj:      dsps handle
 *  @msg_hdr:       Message header pointer from the response
 *  @msg_ptr:       Message pointer from the indication
 * DESCRIPTION   Handle SMGR indication messages.
 *==========================================================================*/
int32_t dsps_handle_smgr_indication(sensor1_config_t *dsps_obj,
  sensor1_msg_header_s *msg_hdr, void *msg_ptr) {

  int32_t retval = 0;
  switch(msg_hdr->msg_id) {
    case SNS_SMGR_BUFFERING_QUERY_IND_V01:
    case SNS_SMGR_BUFFERING_IND_V01:
      /*TODO: Handle this case with Sensor ID*/
      dsps_process_indication_gyro(dsps_obj, msg_hdr, msg_ptr);
      break;

    case SNS_SMGR_REPORT_IND_V01:
      retval = dsps_handle_smgr_rep_indication(dsps_obj, msg_hdr, msg_ptr);
      break;

    default:
      IS_LOW("Message Not Supported MsgID %d", msg_hdr->msg_id);
      break;
  }
  return retval;
}

/*===========================================================================
 * FUNCTION      dsps_sensor1_callback
 *
 * DESCRIPTION   Callback function to be registered with the sensor framework.
 *               This will be called in context of the sensor framework.
 *==========================================================================*/
void dsps_sensor1_callback(intptr_t *data,
  sensor1_msg_header_s *msg_hdr,
  sensor1_msg_type_e msg_type,
  void *msg_ptr)
{
  sensor1_config_t *dsps_obj =(sensor1_config_t *)data;
  sensor1_handle_s *handle = dsps_obj->handle;

  switch (msg_type) {
  case SENSOR1_MSG_TYPE_RESP:
    switch(msg_hdr->service_number) {
      case SNS_SMGR_SVC_ID_V01:
        /* Handle smgr responses based on the sensor ID
         * for gyro and light sensor response
         */
        dsps_handle_smgr_response(dsps_obj, msg_hdr, msg_ptr);
        break;

      case SNS_TIME2_SVC_ID_V01:
        /*TODO: Handle this case with Sensor ID*/
        dsps_process_response_gyro(dsps_obj, msg_hdr, msg_ptr);
        break;

      case SNS_SAM_GRAVITY_VECTOR_SVC_ID_V01:
        /*TODO: Handle this case with Sensor ID*/
        dsps_process_response_gravity(dsps_obj, msg_hdr, msg_ptr);
        break;

      case SNS_SAM_ORIENTATION_SVC_ID_V01:
        /*TODO: Handle this case with Sensor ID*/
        dsps_process_response_gravity_without_gyro(dsps_obj, msg_hdr, msg_ptr);
        break;


      default:
        IS_LOW("Unexpected Response Service Num %d",
          msg_hdr->service_number);
        break;
    }
    dsps_handle_wait_synchronization(dsps_obj,
      msg_hdr->txn_id, DSPS_WAIT_RESP_RECVD);
    break;
  case SENSOR1_MSG_TYPE_IND:
    switch(msg_hdr->service_number) {
      case SNS_SMGR_SVC_ID_V01:
        /* Handle smgr indication based on the sensor ID
         * for gyro and light sensor response
         */
        dsps_handle_smgr_indication(dsps_obj, msg_hdr, msg_ptr);
        break;

      case SNS_SAM_GRAVITY_VECTOR_SVC_ID_V01:
        /*TODO: Handle this case with Sensor ID*/
        dsps_process_indication_gravity(dsps_obj, msg_hdr, msg_ptr);
        break;


      case SNS_SAM_ORIENTATION_SVC_ID_V01:
        /*TODO: Handle this case with Sensor ID*/
        dsps_process_indication_gravity_without_gyro(dsps_obj, msg_hdr, msg_ptr);
        break;

      default:
       IS_LOW("Unexpected Indication Msg type received ");
       break;
    }
    break;

  case SENSOR1_MSG_TYPE_BROKEN_PIPE:
    dsps_handle_broken_pipe(dsps_obj);
    break;

  default:
     IS_ERR("Invalid Message Type\n");
    break;
  }

  if (msg_ptr != NULL) {
    sensor1_free_msg_buf(handle, msg_ptr);
  }
}

void dump_time(const char *id)
{
  struct timespec t_now;

#if !defined(LOG_DEBUG)
  CAM_UNUSED_PARAM(id);
#endif
  clock_gettime( CLOCK_REALTIME, &t_now );
  IS_HIGH("%s, %s, time, %llu, (ms)", __FUNCTION__,id,
       (((int64_t)t_now.tv_sec * 1000) + t_now.tv_nsec/1000000));
}
#endif /* FEATURE_GYRO_DSPS */
