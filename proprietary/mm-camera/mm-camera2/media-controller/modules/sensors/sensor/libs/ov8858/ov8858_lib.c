/* ov8858_lib.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include "ov8858_lib.h"
#include "debug_lib.h"

/**
 * FUNCTION: sensor_real_to_register_gain
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
static unsigned int sensor_real_to_register_gain(float gain)
{
  uint16_t reg_analog_gain;

  if (gain < MIN_GAIN) {
      gain = MIN_GAIN;
  } else if (gain > MAX_ANALOG_GAIN) {
      gain = MAX_ANALOG_GAIN;
  }
  gain = (gain) * 128.0;
  reg_analog_gain = (uint16_t) gain;

  return reg_analog_gain;
}

/**
 * FUNCTION: sensor_register_to_real_gain
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
static float sensor_register_to_real_gain(unsigned int reg_gain)
{
  float real_gain = 0;

  if (reg_gain < 0x80) {
    reg_gain = 0x80;
  } else if (reg_gain > 0x7C0) {
    reg_gain = 0x7C0;
  }
  real_gain = (float) reg_gain / 128.0;

  return real_gain;
}

/**
 * FUNCTION: sensor_digital_gain_calc
 *
 * DESCRIPTION: Calcuate the sensor digital gain
 **/
static unsigned int sensor_digital_gain_calc(float real_gain, float sensor_real_gain)
{
  unsigned int reg_dig_gain = 0;
  float real_dig_gain = 0;

  if(real_gain > MAX_ANALOG_GAIN){
    real_dig_gain = real_gain / sensor_real_gain;
  }else {
    real_dig_gain = MIN_GAIN;
  }

  if(real_dig_gain > MAX_DIGITAL_GAIN){
    real_dig_gain = MAX_DIGITAL_GAIN;
  }

  reg_dig_gain = (unsigned int)(real_dig_gain * 2048);

  return reg_dig_gain;
}


/**
 * FUNCTION: sensor_calculate_exposure
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
static int sensor_calculate_exposure(float real_gain,
  unsigned int line_count, sensor_exposure_info_t *exp_info,
  __attribute__((unused)) float s_real_gain)
{
  if (!exp_info){
    return -1;
  }

  exp_info->reg_gain = sensor_real_to_register_gain(real_gain);
  exp_info->sensor_real_gain =
    sensor_register_to_real_gain(exp_info->reg_gain);
  exp_info->sensor_digital_gain =
    sensor_digital_gain_calc(real_gain, exp_info->sensor_real_gain);
  exp_info->sensor_real_dig_gain =
    (float)exp_info->sensor_digital_gain / OV8858_MAX_DGAIN_DECIMATOR;
  exp_info->digital_gain =
    real_gain /(exp_info->sensor_real_gain * exp_info->sensor_real_dig_gain);
  exp_info->line_count = line_count;

  return 0;
}

/**
 * FUNCTION: sensor_fill_exposure_array
 *
 * DESCRIPTION: Fill the sensor exposure array
 **/
static int sensor_fill_exposure_array(unsigned int gain,
  __attribute__((unused))unsigned int digital_gain, unsigned int line,
  unsigned int fl_lines, __attribute__((unused)) int luma_avg,
  __attribute__((unused)) unsigned int fgain,
  struct camera_i2c_reg_setting* reg_setting,
  __attribute__((unused)) unsigned int s_gain,
  __attribute__((unused)) int s_linecount,
  __attribute__((unused)) int is_hdr_enabled)
{
  int rc = 0;
  unsigned short reg_count = 0;
  unsigned short i = 0;
  uint8_t gain_factor = 0;

  if (!reg_setting){
    return -1;
  }

  for (i = 0; i < sensor_lib_ptr.groupon_settings.size; i++){
    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.groupon_settings.reg_setting_a[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      sensor_lib_ptr.groupon_settings.reg_setting_a[i].reg_data;
    reg_count = reg_count + 1;
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
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr;
  reg_setting->reg_setting[reg_count].reg_data = (line & 0xffff) >> 12;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (line & 0x0fff) >> 4;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr + 2;
  reg_setting->reg_setting[reg_count].reg_data = (line & 0x0f) << 4;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.global_gain_addr;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0x1FFF) >> 8;
  reg_count++;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.global_gain_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF);
  reg_count++;

  for (i = 0; i < sensor_lib_ptr.groupoff_settings.size; i++){
    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.groupoff_settings.reg_setting_a[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      sensor_lib_ptr.groupoff_settings.reg_setting_a[i].reg_data;
    reg_count = reg_count + 1;
  }

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
