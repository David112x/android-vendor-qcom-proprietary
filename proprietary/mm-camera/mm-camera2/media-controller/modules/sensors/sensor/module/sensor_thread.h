
/* sensor.h
 *
 * Copyright (c) 2012-2015,2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SENSOR_THREAD_H__
#define __SENSOR_THREAD_H__

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "modules.h"

typedef void (*offload_func_t) (void* param1, void* param2, void* param3,
  void* param4);

typedef enum {
  SET_AUTOFOCUS,
  CANCEL_AUTOFOCUS,
  OFFLOAD_FUNC,
}sensor_thread_msg_type_t;

typedef struct {
  pthread_t td;
  pthread_mutex_t mutex;
  pthread_condattr_t condattr;
  pthread_cond_t cond;
  int32_t readfd;
  int32_t writefd;
  boolean is_thread_started;
}sensor_thread_t;

typedef struct {
  sensor_thread_msg_type_t  msgtype;
  int32_t                   fd;
  uint32_t                  sessionid;
  boolean                   stop_thread;
  void*                     module;
  offload_func_t            offload_func;
  void                     *param1;
  void                     *param2;
  void                     *param3;
  void                     *param4;
} sensor_thread_msg_t;

void sensor_cancel_autofocus_loop();
boolean sensor_thread_create(module_sensor_bundle_info_t *s_bundle);
#endif //__SENSOR_THREAD_H__
