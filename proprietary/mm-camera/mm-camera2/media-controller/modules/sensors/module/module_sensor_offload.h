/* Copyright (c) 2015,2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef __MODULE_SENSOR_OFFLOAD_H__
#define __MODULE_SENSOR_OFFLOAD_H__

#include "sensor_util.h"

void module_sensor_offload_proc_led(void* param1, void* param2,
  void* param3, void* param4);

void module_sensor_offload_ois_init_calibrate(
  void* param1,
  void* param2 __attribute__((unused)),
  void* param3 __attribute__((unused)),
  void* param4 __attribute__((unused)));

void module_sensor_offload_lens_move(void* param1, void* param2,
  void* param3, void* param4);

void module_sensor_offload_lens_reset(
  void* param1,
  void* param2 __attribute__((unused)),
  void* param3 __attribute__((unused)),
  void* param4 __attribute__((unused)));

void module_sensor_offload_lens_opt_stab_mode(void* param1,
  void* param2 __attribute__((unused)), void* param3 __attribute__((unused)),
  void* param4);

void module_sensor_offload_init_config(
  void* param1, void* param2, void* param3 __attribute__((unused)),
  void* param4 __attribute__((unused)));

void module_sensor_offload_open(
  void* param1, void* param2, void* param3 __attribute__((unused)),
  void* param4 __attribute__((unused)));

void module_sensor_offload_close(
  void* param1, void* param2 __attribute__((unused)),
  void* param3 __attribute__((unused)),
  void* param4 __attribute__((unused)));

void module_sensor_offload_cfg_resolution(void* param1,
  void* param2,
  void* param3 __attribute__((unused)),
  void* param4 __attribute__((unused)));

void module_sensor_offload_actuator(
  void* param1, void* param2 __attribute__((unused)),
  void* param3 __attribute__((unused)), void* param4 __attribute__((unused)));

#endif /* __MODULE_SENSOR_OFFLOAD_H__ */
