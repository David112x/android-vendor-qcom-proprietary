/*============================================================================
   Copyright (c) 2012 - 2016 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

   This file defines the media/module/master controller's interface with the
   DSPS modules.

============================================================================*/

#ifndef _DSPS_HW_INTERFACE_H_
#define _DSPS_HW_INTERFACE_H_

#include "time.h"
#include "stats_event.h"


typedef enum {
  DSPS_ENABLE_REQ     = 1 << 0,
  DSPS_DISABLE_REQ    = 1 << 1,
  DSPS_GET_REPORT     = 1 << 2,
  DSPS_TIMESTAMP_REQ  = 1 << 3
} dsps_req_msg_type_t;

typedef enum _dsps_data_type {
  DSPS_DATA_TYPE_GYRO,
  DSPS_DATA_TYPE_GYRO_AF,
  DSPS_DATA_TYPE_GRAVITY_VECTOR,
  DSPS_DATA_TYPE_ACCELEROMETER,
  DSPS_DATA_TYPE_ROTATION_VECTOR,
  DSPS_DATA_TYPE_LIGHT,
  DSPS_DATA_TYPE_MAGNETOMETER,
  DSPS_DATA_TYPE_MAX,
} dsps_data_type;

typedef struct {
  uint8_t id;
  uint64_t t_start; /* Start Time (microsec) */
  uint64_t t_end;   /* End Time (microsec) */
} dsps_data_t;

typedef struct {
  uint32_t sample_rate;
  dsps_data_t data;
  uint32_t report_rate;
} dsps_set_data_gyro_t;

typedef struct {
  uint32_t report_period;
  uint8_t sample_rate_valid;
  uint32_t sample_rate;
} dsps_set_data_gravity_t;

typedef struct {
  uint32_t sample_rate;
} dsps_set_data_light_t;

typedef struct {
  dsps_data_type sensor_type;
  dsps_req_msg_type_t msg_type;
  union {
    dsps_set_data_gravity_t gravity;
    dsps_set_data_gyro_t gyro;
    dsps_set_data_light_t light;
  } u;
} dsps_set_data_t;

typedef struct {
  float gravity[3];
  float lin_accel[3];
  uint8_t accuracy;
}dsps_cb_data_gravity_t;

typedef struct {
  uint8_t seq_no;
  uint32_t sample_len;  /**< Must be set to # of elements in sample */
  gyro_samples_t sample[STATS_GYRO_MAX_SAMPLE_BUFFER_SIZE];
} dsps_cb_data_gyro_t;

typedef struct {
  float lux;
}dsps_cb_data_light_t;

typedef enum {
  DSPS_SENSOR_LIST,
  DSPS_SENSOR_DATA,
} dsps_cb_type;

typedef struct {
  int num_sensors;
  dsps_data_type available_sensors[DSPS_DATA_TYPE_MAX];
} dsps_sensor_list;

typedef struct {
  dsps_cb_type cb_type;
  dsps_data_type type;
  union {
    dsps_cb_data_gyro_t gyro;
    dsps_cb_data_gravity_t gravity;
    dsps_cb_data_light_t light;
    dsps_sensor_list sensor_list;
  } u;
} dsps_cb_data_t;

typedef void (*dsps_callback_func)(void *port, dsps_cb_data_t *cb_data);

/* Function Prototypes */
void* dsps_proc_init(void *port, dsps_callback_func dsps_callback);
void dsps_proc_deinit(void *dsps_client_handle);
int dsps_proc_set_params(void *dsps_client_handle, dsps_set_data_t *data);

#endif /* _DSPS_HW_INTERFACE_H_ */
