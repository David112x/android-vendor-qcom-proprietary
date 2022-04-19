/*============================================================================

   Copyright (c) 2012 - 2015,2019 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

   This file defines the media/module/master controller's interface with the
   DSPS modules. The functionalities od this module include:

   1. Control communication with the sensor module
   2. Process data received from the sensors

============================================================================*/
#ifndef _DSPS_HW_H_
#define _DSPS_HW_H_

#ifdef FEATURE_GYRO_DSPS
#include <sensor1.h>
#endif
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include "camera_dbg.h"
#include "stats_debug.h"
#include "stats_util.h"
#include "dsps_hw_interface.h"


#define SENSOR_TIME_OUT     100  /* Timeout for response message */
#define MSEC_TO_SEC         1/1000
#define NSEC_PER_USEC       1000
#define SEC_TO_MSEC         1000
#define MSEC_TO_NSEC        1000000
#define USEC_PER_SEC        1000000
#define NSEC_PER_SEC        1000000000
#define INVALID_INSTANCE_ID 0xFF
#define DSPS_HZ             32768U
#define SNS_SAM_ALGO_REPORT_IND 0x05
#define TIMESYNC_FREQ       16

#define DSPS_PROC_DEBUG     0

#if(DSPS_PROC_DEBUG)
  #include <utils/Log.h>
  #undef LOG_NIDEBUG
  #undef LOG_TAG
  #define LOG_NIDEBUG 0
  #define LOG_TAG "mm-camera-DSPS"
#else
#endif

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

/*SNS_SMGR_SHORT_SENSOR_NAME_SIZE_V01 +1*/
#define DSPS_SENSOR_SHORT_NAME_SIZE 17

typedef enum {
  DSPS_WAIT_RESP_START,
  DSPS_WAIT_RESP_RECVD,
  DSPS_WAIT_RESP_END,
} dsps_wait_sync_t;

typedef struct {
  boolean present;
  boolean enabled;
  uint8_t sensor_id;
  uint8_t data_type;
  char sensor_name[DSPS_SENSOR_SHORT_NAME_SIZE];
  int32_t sample_rate;
  int32_t min_delay;
  int32_t max_range;
  int32_t resolution;
  uint8_t req_supported;
} dsps_sensor_info_t;

typedef struct {
  uint64_t t_start;
  uint64_t t_end;
  uint8_t seqnum;
  uint32_t sample_rate;
  uint32_t report_rate;
} sensor1_req_data_gyro_t;

typedef struct {
  uint32_t report_period;
  uint8_t sample_rate_valid;
  uint32_t sample_rate;
} sensor1_req_data_gravity_t;

typedef struct {
  uint32_t sample_rate;
} sensor1_req_data_light_t;

typedef struct {
  dsps_data_type sensor_type;
  dsps_req_msg_type_t msg_type;
  union {
    sensor1_req_data_gyro_t gyro;
    sensor1_req_data_gravity_t gravity;
    sensor1_req_data_light_t light;
  } u;
} sensor1_req_data_t;

typedef enum {
  DSPS_RUNNING = 1,
  DSPS_STOPPED = 0,
  DSPS_BROKEN_PIPE = -1,
  DSPS_FAILED_CREATE = -2,
} dsps_thread_status_t;


/** dsps_time_state_t:
 *    @ts_offset: offset between AP and DSPS clock in usec
 *    @ts_cntr: counter to keep track when to resync the AP and DSPS clocks
 *    @ts_offset_valid: AP/DSPS clock offset valid indicator
 *    @ts_dsps_prev: previous DSPS time
 *    @ts_dsps_ro_cnt: DSPS clock rollover counter
 */
typedef struct {
  uint64_t ts_offset;
  uint64_t ts_cnt;
  int ts_offset_valid;
  uint64_t ts_dsps_prev;
  uint32_t ts_dsps_ro_cnt;
} dsps_time_state_t;


/** sensor1_config_t:
 *    @thread_id: thread id used to join
 *    @handle: sensor1 handle
 *    @callback_mutex: mutex for use with sensor1 callback
 *    @callback_condvar: cvar for callback from sensor
 *    @callback_arrived: flag to indicate callback arrived
 *    @dsps_callback: callback into upper gyro layer
 *    @thread_mutex: mutex for DSPS thread
 *    @thread_condvar: cvar for DSPS thread
 *    @error: error indication
 *    @instance_id_gravity: garvity service instance id
 *    @status: current status of DSPS thread
 *    @queue: gyro data circular queue
 *    @seqnum: DSPS layer - sensor1 transaction id
 **/
typedef struct _sensor1_config {
  pthread_t thread_id;
#ifdef FEATURE_GYRO_DSPS
  sensor1_handle_s *handle;
#endif
  pthread_mutex_t callback_mutex;
  pthread_condattr_t callback_condattr;
  pthread_cond_t callback_condvar;
  dsps_sensor_info_t sensor_info[DSPS_DATA_TYPE_MAX];
  int callback_arrived;
  uint8_t callback_txn_id;
  dsps_callback_func dsps_callback;
  pthread_mutex_t thread_mutex;
  pthread_condattr_t thread_condattr;
  pthread_cond_t thread_condvar;
  int error;
  int instance_id_gravity;
  dsps_thread_status_t status;
  void *port;
  dsps_time_state_t dsps_time_state;
  uint8_t seqnum;
} sensor1_config_t;


/* Function Prototypes */
int dsps_send_request(void *sensor_config, void *req_data, int wait);
int dsps_disconnect(void *sensor_config);
int dsps_open(void *sensor_config);

#endif /* _DSPS_HW_H_ */
