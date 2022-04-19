/* live_connect.h
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __LIVE_CONNECT_H__
#define __LIVE_CONNECT_H__

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "sensor_common.h"
#include "actuator_driver.h"

typedef struct
{
  int size;
  void *pValue;
}live_connect_items_map;

typedef enum
{
  LIVE_CONNECT_CMD_PING = 0x4000,
  LIVE_CONNECT_CMD_MOVE_FOCUS_NEAR,
  LIVE_CONNECT_CMD_MOVE_FOCUS_FAR,
  LIVE_CONNECT_CMD_GET_CUR_STEP_POSITION,
  LIVE_CONNECT_CMD_GET_CUR_LENS_POSITION,
  LIVE_CONNECT_CMD_RUN_LINEAR_TEST,
  LIVE_CONNECT_CMD_RUN_RINGING_TEST,
  LIVE_CONNECT_CMD_SET_DEFAULT_FOCUS,
  LIVE_CONNECT_CMD_START_PLOT,
  LIVE_CONNECT_CMD_STOP_PLOT,
}live_connect_control_cmd;

typedef struct
{
  uint32_t live_connect_cmd_start;
  uint32_t live_connect_cmd_end;
}live_connect_info;

int32_t sensor_live_connect_thread_create(
  module_sensor_bundle_info_t *module);
void sensor_live_connect_thread_destroy(
  module_sensor_bundle_info_t *s_bundle);
uint32_t live_connect_get_cmd_num();
uint32_t live_connect_get_start_cmd();
uint32_t live_connect_get_end_cmd();
boolean live_connect_fill_map(live_connect_items_map *item_map,
  actuator_tuned_params_t *actuator_tuned_params_ptr);
#endif //__LIVE_CONNECT_H__

