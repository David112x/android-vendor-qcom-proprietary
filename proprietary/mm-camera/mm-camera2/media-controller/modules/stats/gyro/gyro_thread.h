/* stats/gyro/gyro_thread.h
 *
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef __GYRO_THREAD_H__
#define __GYRO_THREAD_H__

#include <pthread.h>
#include <semaphore.h>
#include "mct_queue.h"
#include "mct_port.h"
#include "is.h"
#include "dsps_hw_interface.h"
#include "sensor_intf.h"
/** gyro_thread_data_t
 *    @msg_q_lock: mutex lock for protecting concurrent access to the message
 *       queue
 *    @msg_q: Gyro thread's message queue
 *    @active: indicates whether or not Gyro thread is active
 *    @thread_cond: conditional variable to wake the Gyro thread
 *    @thread_mutex: mutex to lock the conditional variable - needed for the
 *       signaling mechanism
 *    @thread_id: ID of the Gyro thread
 *    @sem_launch: semaphore used for thread synchronization
 *    @gyro_port: Gyro port
 **/
typedef struct {
  pthread_mutex_t msg_q_lock;
  mct_queue_t     *msg_q;
  boolean         active;
  pthread_cond_t  thread_cond;
  pthread_mutex_t thread_mutex;
  pthread_t       thread_id;
  sem_t           sem_launch;
  mct_port_t      *gyro_port;
} gyro_thread_data_t;

/** gyro_msg_sync_t
 *    @msg_sem: semaphore to synchronize the "Sync" messages
 *
 *  This structure is used for creating synchronized messages in the queue.
 *  When such message arrives, the enqueue function will block until the
 *  message is processed by the thread handler and the library returns with
 *  the result.
 **/
typedef struct {
  sem_t  msg_sem;
} gyro_msg_sync_t;

/** gyro_thread_msg_type_t
 **/
typedef enum {
  MSG_GYRO_SET,
  MSG_GYRO_PROCESS,
  MSG_GYRO_STOP,
} gyro_thread_msg_type_t;



typedef enum {
  MSG_GYRO_SET_IS_PARAMS,
  MSG_GYRO_SET_COMMON_PARAMS,
} gyro_set_params_type;

typedef struct {
  int32_t is_enable;
} gyro_thread_is_params;

typedef struct {
  boolean stream_on;
} gyro_thread_common_params;

typedef struct {
  gyro_set_params_type set_params_type;
  union {
    gyro_thread_is_params is_params;
    gyro_thread_common_params common_params;
  } u;
} gyro_thread_set_params;


typedef enum {
  MSG_GYRO_REQUEST_DATA,
  MSG_GYRO_PROCESS_CALLBACK,
} gyro_process_type;

typedef enum {
  CB_TYPE_DSPS,
  CB_TYPE_ANDROID,
} gyro_cb_data_type;

typedef struct {
  gyro_cb_data_type cb_type;
  union {
    dsps_cb_data_t  *dsps_cb_data;
    sensor_cb_struct_type *andr_cb_data;
  } u;
} gyro_callback_data;

typedef struct {
  uint32_t event_identity;
  mct_bus_msg_isp_sof_t sof_event;
} gyro_request_t;

typedef struct {
  gyro_process_type process_params_type;
  union {
    gyro_callback_data cb_data;
    gyro_request_t data_req;
  } u;
} gyro_thread_process_t;

/** gyro_thread_msg_t
 *    @type: message type
 *
 *  This structure is used for creating messages for the Gyro thread.
 **/
typedef struct {
  gyro_thread_msg_type_t type;
  boolean         sync_flag;
  gyro_msg_sync_t   *sync_obj;
  union {
    gyro_thread_set_params set_params;
    gyro_thread_process_t process_params;
  }u;
} gyro_thread_msg_t;

boolean gyro_thread_en_q_msg(gyro_thread_data_t *thread_data, gyro_thread_msg_t *msg);
boolean gyro_thread_start(gyro_thread_data_t *thread_data);
boolean gyro_thread_stop(gyro_thread_data_t *thread_data);
gyro_thread_data_t* gyro_thread_init(void);
void gyro_thread_deinit(gyro_thread_data_t *thread_data);
#endif /* __GYRO_THREAD_H__ */
