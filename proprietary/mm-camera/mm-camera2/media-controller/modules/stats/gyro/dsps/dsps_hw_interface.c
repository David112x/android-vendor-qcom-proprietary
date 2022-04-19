/*============================================================================

   Copyright (c) 2012 - 2015, 2017, 2019 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.

   This file defines the media/module/master controller's interface with the
   DSPS modules.

============================================================================*/
#include "dsps_hw.h"
#include "camera_dbg.h"


#define AP_DSPS_TIME_SYNC_PERIOD     32


/** dsps_proc_init:
 *    @port: gyro port (opaque to dsps layer)
 *    @dsps_callback: callback to upper layer client
 *
 * This function creates an instance of DSPS (Discrete Sensors Processing
 * System) session and opens a channel to the DSPS.
 *
 * Returns DSPS client handle on success, NULL otherwise.
 **/
void *dsps_proc_init(void *port, dsps_callback_func dsps_callback)
{
  sensor1_config_t *dsps_config;

   IS_LOW("E\n");
  dsps_config = (sensor1_config_t *)malloc(sizeof(sensor1_config_t));
  if (dsps_config == NULL) {
    IS_ERR("malloc error");
    return NULL;
  }

  memset((void *)dsps_config, 0, sizeof(sensor1_config_t));
#ifdef FEATURE_GYRO_DSPS
  dsps_config->handle = NULL;
#endif
  dsps_config->callback_arrived = 0;
  dsps_config->status = DSPS_RUNNING;
  dsps_config->error = 0;
  dsps_config->port = port;
  dsps_config->dsps_callback = dsps_callback;
  pthread_mutex_init(&(dsps_config->callback_mutex), NULL);

  pthread_condattr_init(&dsps_config->callback_condattr);
  pthread_condattr_setclock(&dsps_config->callback_condattr, CLOCK_MONOTONIC);
  pthread_cond_init(&(dsps_config->callback_condvar), &dsps_config->callback_condattr);

  pthread_mutex_init(&(dsps_config->thread_mutex), NULL);

  pthread_condattr_init(&dsps_config->thread_condattr);
  pthread_condattr_setclock(&dsps_config->thread_condattr, CLOCK_MONOTONIC);
  pthread_cond_init(&(dsps_config->thread_condvar), &dsps_config->thread_condattr);
  if (dsps_open((void *)dsps_config) != 0) {
    goto dsps_open_error;
  }
   IS_LOW("X");
  return dsps_config;

dsps_open_error:
   IS_ERR("Failed to open sensor1 port");
  free(dsps_config);
  return NULL;
}


/** dsps_proc_deinit:
 *    @port: dsps_client_handle
 *
 * This function closes the DSPS channel and frees the session memory.
 **/
void dsps_proc_deinit(void *dsps_client_handle)
{
  sensor1_config_t *dsps_config = (sensor1_config_t *)dsps_client_handle;

  if (dsps_config == NULL) {
     IS_ERR("DSPS client handle is NULL");
    return;
  }

  dsps_disconnect((void *)dsps_config);
  free(dsps_config);
}

/*===========================================================================
 * FUNCTION    - dsps_proc_set_params_gyro -
 *
 * DESCRIPTION:
 *==========================================================================*/
int dsps_proc_set_params_gyro(void *dsps_client_handle, dsps_set_data_t *data)
{
  sensor1_config_t *dsps_obj = (sensor1_config_t *)dsps_client_handle;

  if (dsps_obj == NULL || dsps_obj->status != DSPS_RUNNING) {
     IS_LOW("DSPS is not running, deinitialized or not connected");
    return -1;
  }

  int rc = 0;
  sensor1_req_data_t msg_data;
  msg_data.sensor_type = data->sensor_type;
  int wait = 0;

   IS_LOW("msg_type %d", data->msg_type);

#ifdef FEATURE_GYRO_DSPS
  switch (data->msg_type) {
  case DSPS_GET_REPORT:
    dsps_obj->dsps_time_state.ts_cnt++;
    if (dsps_obj->dsps_time_state.ts_cnt % AP_DSPS_TIME_SYNC_PERIOD == 0) {
      msg_data.msg_type = DSPS_TIMESTAMP_REQ;
      if (dsps_send_request((void *)dsps_obj, &msg_data, wait) < 0) {
         IS_ERR("Error sending timestamp request");
      }
    }

    if (dsps_obj->dsps_time_state.ts_offset_valid) {
      msg_data.msg_type = DSPS_GET_REPORT;
      msg_data.u.gyro.seqnum = data->u.gyro.data.id;
      msg_data.u.gyro.t_start = data->u.gyro.data.t_start;
      msg_data.u.gyro.t_end = data->u.gyro.data.t_end;
      if (dsps_send_request((void *)dsps_obj, &msg_data, wait) < 0) {
         IS_ERR("Error sending query request");
        rc = -1;
      }
    } else {
       IS_HIGH("Query request not sent");
      rc = -1;
    }
    break;

  case DSPS_ENABLE_REQ:
    wait = 0;
    msg_data.msg_type              = DSPS_ENABLE_REQ;
    msg_data.u.gyro.sample_rate    = data->u.gyro.sample_rate;
    msg_data.u.gyro.report_rate    = data->u.gyro.report_rate;
    if (dsps_send_request((void *)dsps_obj, &msg_data, wait) == 0) {
      msg_data.msg_type = DSPS_TIMESTAMP_REQ;
      if (dsps_send_request((void *)dsps_obj, &msg_data, wait) < 0) {
         IS_ERR("Error sending timestamp request");
      }
    } else {
       IS_ERR("Error sending enable request");
      rc = -1;
    }
    break;

  case DSPS_DISABLE_REQ:
    wait = 1;
    msg_data.msg_type = DSPS_DISABLE_REQ;
    if (dsps_send_request((void *)dsps_obj, &msg_data, wait) < 0) {
       IS_ERR("Error sending disable request");
      rc = -1;
    }
    break;

  default:
     IS_ERR("Invalid param");
    rc = -1;
  }
#endif

  return rc;
}  /* dsps_proc_set_params_gyro */



int dsps_proc_set_params_gyro_af(void *dsps_client_handle, dsps_set_data_t *data)
{
  sensor1_config_t *dsps_obj = (sensor1_config_t *)dsps_client_handle;

  if (dsps_obj == NULL || dsps_obj->status != DSPS_RUNNING) {
     IS_LOW("DSPS is not running, deinitialized or not connected");
    return -1;
  }

  int rc = 0;
  sensor1_req_data_t msg_data;
  msg_data.sensor_type = data->sensor_type;
  int wait = 0;

   IS_LOW("msg_type %d", data->msg_type);

#ifdef FEATURE_GYRO_DSPS
  switch (data->msg_type) {
  case DSPS_ENABLE_REQ:
    wait = 0;
    msg_data.msg_type              = DSPS_ENABLE_REQ;
    msg_data.u.gyro.sample_rate    = data->u.gyro.sample_rate;
    msg_data.u.gyro.report_rate    = data->u.gyro.report_rate;
    if (dsps_send_request((void *)dsps_obj, &msg_data, wait) == 0) {
      msg_data.msg_type = DSPS_TIMESTAMP_REQ;
      if (dsps_send_request((void *)dsps_obj, &msg_data, wait) < 0) {
         IS_ERR("Error sending timestamp request");
      }
    } else {
       IS_ERR("Error sending enable request");
      rc = -1;
    }
    break;

  case DSPS_DISABLE_REQ:
    wait = 1;
    msg_data.msg_type = DSPS_DISABLE_REQ;
    if (dsps_send_request((void *)dsps_obj, &msg_data, wait) < 0) {
       IS_ERR("Error sending disable request");
      rc = -1;
    }
    break;

  default:
     IS_ERR("Invalid param");
    rc = -1;
  }
#endif

  return rc;
}  /* dsps_proc_set_params_gyro */


/*===========================================================================
 * FUNCTION    - dsps_proc_set_params_gravity -
 *
 * DESCRIPTION:
 *==========================================================================*/
int dsps_proc_set_params_gravity(void *dsps_client_handle, dsps_set_data_t *data)
{
  sensor1_config_t *dsps_config = (sensor1_config_t *)dsps_client_handle;

  if (dsps_config == NULL || dsps_config->status != DSPS_RUNNING) {
     IS_LOW("DSPS is not running, deinitialized or not connected");
    return -1;
  }

  int rc = 0;
  sensor1_req_data_t msg_data;
  msg_data.sensor_type = data->sensor_type;
  msg_data.msg_type = data->msg_type;
  int wait = 0;

   IS_LOW("msg_type %d", data->msg_type);
  switch (data->msg_type) {
  case DSPS_ENABLE_REQ: {
    msg_data.u.gravity.report_period = data->u.gravity.report_period;
    msg_data.u.gravity.sample_rate_valid = data->u.gravity.sample_rate_valid;
    msg_data.u.gravity.sample_rate = data->u.gravity.sample_rate;
     IS_LOW("DSPS_ENABLE_REQ: repor_period: %u sample_rate_valid: %d "
      "sample_rate: %u",
       msg_data.u.gravity.report_period,
      msg_data.u.gravity.sample_rate_valid,
      msg_data.u.gravity.sample_rate);
    wait = 0;
  }
    break;

  case DSPS_DISABLE_REQ:
    wait = 1;
    break;

  case DSPS_GET_REPORT:
    wait = 1;
    break;

  default:
    IS_ERR("Invalid param");
    return -1;
  }
#ifdef FEATURE_GYRO_DSPS
  if (dsps_send_request((void *)dsps_config, &msg_data, wait) < 0) {
    IS_ERR("Error sending request");
    return -1;
  }
#endif
  return rc;
}  /* dsps_proc_set_params_gravity */


/*===========================================================================
 * FUNCTION    - dsps_proc_set_params_light -
 *
 * DESCRIPTION:
 *==========================================================================*/
int dsps_proc_set_params_light(void *dsps_client_handle, dsps_set_data_t *data)
{
  sensor1_config_t *dsps_config = (sensor1_config_t *)dsps_client_handle;

  if (dsps_config == NULL || dsps_config->status != DSPS_RUNNING) {
     IS_ERR("DSPS is not running, deinitialized or not connected");
    return -1;
  }

  int rc = 0;
  sensor1_req_data_t msg_data;
  msg_data.sensor_type = data->sensor_type;
  msg_data.msg_type = data->msg_type;
  int wait = 0;

  IS_LOW("msg_type %d", data->msg_type);
  switch (data->msg_type) {
    case DSPS_ENABLE_REQ: {
      msg_data.u.light.sample_rate = data->u.light.sample_rate;
       IS_LOW("DSPS_ENABLE_REQ: sample_rate: %u",
        msg_data.u.light.sample_rate);
      wait = 0;
    }
    break;

    case DSPS_DISABLE_REQ:
    case DSPS_GET_REPORT:
      wait = 1;
      break;

    default:
      IS_ERR("Invalid param");
      return -1;
      break;
    }
#ifdef FEATURE_GYRO_DSPS
  if (dsps_send_request((void *)dsps_config, &msg_data, wait) < 0) {
    IS_LOW("Error sending request");
    return -1;
  }
#endif
  return rc;
}  /* dsps_proc_set_params_gravity */


/*===========================================================================
 * FUNCTION    - dsps_proc_set_params -
 *
 * DESCRIPTION:
 *==========================================================================*/
int dsps_proc_set_params(void *dsps_client_handle, dsps_set_data_t *data)
{
  sensor1_config_t *dsps_config = (sensor1_config_t *)dsps_client_handle;
  int rc = 0;

  if (dsps_config == NULL || dsps_config->status != DSPS_RUNNING) {
     IS_LOW("DSPS is not running, deinitialized or not connected");
    return -1;
  }

   IS_LOW("DSPS Set Parameter for %d", data->sensor_type);

  switch (data->sensor_type) {
  case DSPS_DATA_TYPE_GYRO:
    rc = dsps_proc_set_params_gyro(dsps_client_handle, data);
    break;
  case DSPS_DATA_TYPE_GYRO_AF:
    rc = dsps_proc_set_params_gyro_af(dsps_client_handle, data);
    break;
  case DSPS_DATA_TYPE_GRAVITY_VECTOR:
    rc = dsps_proc_set_params_gravity(dsps_client_handle, data);
    break;
  case DSPS_DATA_TYPE_LIGHT:
    rc = dsps_proc_set_params_light(dsps_client_handle, data);
    break;
  default:
     IS_ERR("Sensor type %d not supported yet!", data->sensor_type);
    rc = -1;
    break;
  }

  return rc;
}  /* dsps_proc_set_params */
