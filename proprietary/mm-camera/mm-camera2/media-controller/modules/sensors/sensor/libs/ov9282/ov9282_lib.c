/* ov9282_lib.c
 *
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include "ov9282_lib.h"

/**
 * FUNCTION: sensor_real_to_register_gain
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
static uint32_t sensor_real_to_register_gain(float gain)
{
    uint32_t reg_gain = 0;
    gain = gain*16.0;
    reg_gain = (uint32_t)gain;
    return reg_gain;
}

/**
 * FUNCTION: sensor_register_to_real_gain
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
static float sensor_register_to_real_gain(uint32_t reg_gain)
{
    float real_gain;
    real_gain = (float) (((float)(reg_gain))/16.0);
    return real_gain;
}

/**
 * FUNCTION: sensor_calculate_exposure
 *
 * DESCRIPTION: Calcuate the sensor exposure
 * INPUTS PARAMS : real_gain, Line Count,
 * OUTPUT PARAMS : exp_info
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
 * DESCRIPTION: Fill the sensor exposure array.
 *  input param 1 : register gain
 *  input param 2 : digital gain
 *  input param 3 : line
 *  input param 4 : frame length lines
 *  input param 7 : struct reg_settings
 **/
 static int32_t sensor_fill_exposure_array(uint32_t gain,
  uint32_t digital_gain, uint32_t line,
  uint32_t fl_lines, __attribute__((unused)) int32_t luma_avg,
  __attribute__((unused)) uint32_t fgain,
  struct camera_i2c_reg_setting* reg_setting,
  __attribute__((unused)) unsigned int s_gain,
  __attribute__((unused)) int s_linecount,
  __attribute__((unused)) int is_hdr_enabled)
{
    int32_t rc = 0;
    uint16_t reg_count = 0;
    uint16_t i = 0;

    if (!reg_setting)
    {
      return -1;
    }

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr.frame_length_lines;
    reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF00) >> 8;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr.frame_length_lines + 1;
    reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF);
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr - 1;
    reg_setting->reg_setting[reg_count].reg_data = line >> 12;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr;
    reg_setting->reg_setting[reg_count].reg_data = (line >> 4) & 0xff;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = (line << 4) & 0xff;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.global_gain_addr;
    reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF);
    reg_count++;

    reg_setting->size = reg_count;
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
