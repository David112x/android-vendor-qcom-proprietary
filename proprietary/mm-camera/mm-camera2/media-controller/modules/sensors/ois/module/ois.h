/*==========================================================
Copyright (c) 2014-2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================*/
#ifndef __OIS_H__
#define __OIS_H__

#include <pthread.h>
#include "ois_driver.h"

typedef struct
{
  ois_driver_params_t *driver_ctrl;
} ois_ctrl_t;

typedef struct {
  int32_t fd;
  ois_ctrl_t *ctrl;
  uint8_t is_ois_supported;
  char *name;
  void *driver_lib_handle;
  ois_read_data_t ois_data;

  pthread_mutex_t control_mutex;
} ois_data_t;

#endif
