/* sensor_intf.h
 *
 * Copyright (c) 2014-2017, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/*Only one of them should be defined at a timefrom makefile based on sdk version*/
//#define SENSOR_HIDL_INTF
//#define SENSOR_NATIVE_LAYER

#ifndef _ANDROID_
#ifdef SENSOR_NATIVE_LAYER
#undef SENSOR_NATIVE_LAYER
#endif
#ifdef SENSOR_HIDL_INTF
#undef SENSOR_HIDL_INTF
#endif
#endif

#ifndef __SENSOR_INTF_H__
#define __SENSOR_INTF_H__
#include "camera_dbg.h"
#include "mtype.h"
#include "stats_event.h"
#include "stats_debug.h"


#define GYRO_BUFFER_SIZE 100
#define DEFAULT_RATE_IN_US 5000
#define DEFAULT_RATE_IN_US_ACCEL 20000
#define DEFAULT_RATE_IN_US_LIGHT 100000
#define MAX_Q_SIZE          5


/* sensor_android_fmwk_mask:
  * Setprop mask to enable/disable specific sensor accessed
  * through android sensor framework */
typedef enum {
  SENSOR_ANDROID_FMWK_DISABLE = 0,
  SENSOR_ANDROID_FMWK_ENABLE_ALL = (1 << 0),
  SENSOR_ANDROID_FMWK_LASER = (1 << 1),
  SENSOR_ANDROID_FMWK_GYRO = (1 << 2),
  SENSOR_ANDROID_FMWK_ACCELEROMETER = (1 << 3),
  SENSOR_ANDROID_FMWK_GRAVITY_VECTOR = (1 << 4),
  SENSOR_ANDROID_FMWK_MAGNETOMETER = (1 << 5),
  SENSOR_ANDROID_FMWK_LINEAR_ACCEL = (1 << 6),
  SENSOR_ANDROID_FMWK_LIGHT = (1 << 7),
} sensor_android_fmwk_mask;

typedef enum _sensor_return {
  SENSOR_RET_SUCCESS,
  SENSOR_RET_FAILURE,
  SENSOR_RET_BAD_PARAMS,
  SENSOR_RET_NO_MEMMORY,
  SENSOR_RET_UNKNOWN,
} sensor_return_type;


typedef enum _sensor {
  SENSOR_ACCELEROMETER,
  SENSOR_GYRO,
  SENSOR_MAGNETOMETER,
  SENSOR_GRAVITY,
  SENSOR_LINEAR_ACCELERATION,
  SENSOR_TIME_OF_FLIGHT,
  SENSOR_LIGHT,
  SENSOR_TYPE_MAX_AVAILABLE,
} sensor_type;


typedef struct _sensor_data_gyro {
  float x;
  float y;
  float z;
} sensor_data_gyro_type;

typedef struct _sensor_data_accel {
  float x;
  float y;
  float z;
} sensor_data_accel_type;

typedef struct _sensor_data_light {
  float lux;
} sensor_data_light_type;

typedef struct _sensor_data_laser {
  float version;
  float type;
  float distance;
  float confidence;
  float near_limit;
  float far_limit;
  int64_t timestamp;
  int32_t max_range;
} sensor_data_laser_type;

typedef struct _sensor_data {
  sensor_type type;
  uint64_t timestamp;
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  uint64_t is_timestamp;
#endif
  union {
    sensor_data_gyro_type gyro;
    sensor_data_accel_type accel;
    sensor_data_accel_type gravity;
    sensor_data_accel_type lin_accel;
    sensor_data_light_type light;
  } u;
} sensor_data_type;

typedef struct {
  uint8_t seqnum;
  uint32_t sample_len;  /**< Must be set to # of elements in sample */
  gyro_samples_t sample[STATS_GYRO_MAX_SAMPLE_BUFFER_SIZE];
} sensor_gyro_buffer_data_type;

typedef struct {
  sensor_type type;
  uint64_t timestamp;
  union {
    sensor_gyro_buffer_data_type gyro;
    sensor_data_accel_type accel;
    sensor_data_accel_type gravity;
    sensor_data_accel_type lin_accel;
    sensor_data_laser_type laser;
    sensor_data_light_type light;
    sensor_accel_buffer_data_type accel_buf;
  } u;
} sensor_callback_data_type;

typedef enum {
  SENSOR_ENABLE_REQ,
  SENSOR_DISABLE_REQ,
  SENSOR_SET_DATA_CONFIG, /* Setup time interval of data per request */
} sensor_set_msg_type;

typedef enum {
  SENSOR_GET_FRAME_DATA,
  SENSOR_GET_LAST_FRAME_DATA,
} sensor_get_msg_type;

typedef struct {
  uint8_t id;
  uint64_t t_start; /* Start Time (microsec) */
  uint64_t t_end;   /* End Time (microsec) */
} sensor_data_range_type;

typedef struct {
  sensor_type sensor;
  sensor_set_msg_type msg_type;
  union {
    int32_t sample_rate; /* SENSOR_ENABLE_REQ */
    sensor_data_range_type data_range; /* SENSOR_SET_DATA_CONFIG */
  } u;
} sensor_set_data_type;

typedef struct {
  int num_of_sensors;
  sensor_type available_sensors[SENSOR_TYPE_MAX_AVAILABLE];
} sensor_list_type;

typedef enum {
  SENSOR_LIST,
  SENSOR_DATA,
} sensor_cb_type;

typedef struct {
  sensor_cb_type type;
  union {
    sensor_list_type sensor_list;
    sensor_callback_data_type sensor_data;
  } u;
} sensor_cb_struct_type;

typedef void (*sensor_data_cb)(
  void *p,
  sensor_cb_struct_type cb_event_data);

sensor_return_type sensor_intf_init(
  void **handle,
  void *cb_hndl,
  sensor_data_cb cb);

sensor_return_type sensor_intf_deinit(void *handle);

sensor_return_type sensor_set_params(
  void *handle, sensor_set_data_type sensor_set);

#endif /* __SENSOR_INTF_H__*/
