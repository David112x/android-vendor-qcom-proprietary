/* sensor_gain_exposure.h
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __SENSOR_GAIN_EXPOSURE_H__
#define __SENSOR_GAIN_EXPOSURE_H__

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "sensor.h"
#include "sensor_thread.h"
#include "sensor_pick_res.h"

int32_t sensor_cmn_calc_exposure(sensor_lib_t *lib_ptr, float real_gain,
  uint32_t line_count, sensor_exposure_info_t *exp_info, float s_real_gain);

int32_t sensor_cmn_lib_fill_exposure( sensor_lib_t *lib_ptr,
  uint32_t gain,
  uint32_t digital_gain, uint32_t line, uint32_t fl_lines,
  __attribute__((unused)) int luma_avg, uint32_t hdr_param,
  struct camera_i2c_reg_setting* reg_setting, uint32_t s_gain,
  int32_t s_linecount, int32_t is_hdr_enabled);

#endif /* __SENSOR_H__ */
