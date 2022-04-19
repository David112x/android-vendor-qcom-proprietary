/* csidtg_lib.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include "csidtg_lib.h"
#include "debug_lib.h"

/**
 * FUNCTION: sensor_real_to_register_gain
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
static uint32_t sensor_real_to_register_gain(float gain)
{
  uint32_t reg_gain;

  if (gain < 1.0) {
    gain = 1.0;
  } else if (gain > 8.0) {
    gain = 8.0;
  }

  reg_gain = (uint32_t)(256.0 - 256.0 / gain);

  return reg_gain;
}

/**
 * FUNCTION: sensor_register_to_real_gain
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
static float sensor_register_to_real_gain(uint32_t reg_gain)
{
  float gain;

  if (reg_gain > 224) {
    reg_gain = 224;
  }
  gain = 256.0 / (256.0 - reg_gain);
  return gain;
}

/**
 * FUNCTION: sensor_calculate_exposure
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
static int32_t sensor_calculate_exposure(float real_gain,
  uint32_t line_count, sensor_exposure_info_t *exp_info,
  __attribute__((unused)) float s_real_gain)
{
    if (!exp_info)
    {
      return -1;
    }

    exp_info->reg_gain = sensor_real_to_register_gain(real_gain);
    exp_info->sensor_real_gain = sensor_register_to_real_gain(exp_info->reg_gain);
    exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
    exp_info->line_count = line_count;
    exp_info->sensor_digital_gain = 0x1;

    return 0;
}

/**
 * FUNCTION: sensor_fill_exposure_array
 *
 * DESCRIPTION: Fill the sensor exposure array
 **/
static int sensor_fill_exposure_array(__attribute__((unused))unsigned int gain,
    __attribute__((unused)) unsigned int digital_gain,
    __attribute__((unused)) unsigned int line,
    __attribute__((unused)) unsigned int fl_lines,
    __attribute__((unused)) int luma_avg,
    __attribute__((unused)) unsigned int hdr_param,
    struct camera_i2c_reg_setting* reg_setting,
    __attribute__((unused)) unsigned int s_gain,
    __attribute__((unused)) int s_linecount,
    __attribute__((unused)) int is_hdr_enabled)
{

  if (!reg_setting) {
    return -1;
  }

  reg_setting->reg_setting[0].reg_addr = 0;
  reg_setting->reg_setting[0].reg_data = 0;

  reg_setting->size = 1;
  reg_setting->addr_type = CAMERA_I2C_WORD_ADDR;
  reg_setting->data_type = CAMERA_I2C_BYTE_DATA;
  reg_setting->delay = 0;
  return 0;
}


/**
 * FUNCTION: sensor_open_lib
 *
 * DESCRIPTION: Open sensor library and returns data pointer
 **/
void *sensor_open_lib(void)
{
  return &sensor_lib_ptr;
}
