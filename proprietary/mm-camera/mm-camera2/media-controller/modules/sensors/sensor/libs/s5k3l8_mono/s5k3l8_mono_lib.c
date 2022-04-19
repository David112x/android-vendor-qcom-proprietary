/* s5k3l8_lib.c
 *
 *Copyright (c) 2015 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include "s5k3l8_mono_lib.h"
#include "debug_lib.h"

/**
 * FUNCTION: s5k3l8_real_to_register_gain
 *
 * DESCRIPTION: Calcuate sensor analog gain register value
 **/
static unsigned int s5k3l8_real_to_register_gain(float real_gain)
{
    unsigned int reg_gain = 0;

    if (real_gain < S5K3L8_MIN_AGAIN)
    {
        real_gain = S5K3L8_MIN_AGAIN;
        SHIGH("s5k3l8 Requested gain(%f) is lower than the sensor minimum(%d)",
            real_gain, S5K3L8_MIN_AGAIN);
    }
    else if (real_gain > S5K3L8_MAX_AGAIN)
    {
        real_gain = S5K3L8_MAX_AGAIN;
    }

    reg_gain = (unsigned int)(real_gain * 32);

    return reg_gain;
}

/**
 * FUNCTION: s5k3l8_register_to_real_gain
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
static float s5k3l8_register_to_real_gain(unsigned int reg_gain)
{
    float real_gain;

    if (reg_gain <= S5K3L8_MIN_AGAIN_REG_VAL)
    {
        reg_gain = S5K3L8_MIN_AGAIN_REG_VAL;
        SHIGH("S5K3M2XX Requested gain(%d) is lower than the sensor minimum(%d)",
            reg_gain, S5K3L8_MIN_GAIN);
    }
    else if (reg_gain > S5K3L8_MAX_AGAIN_REG_VAL)
    {
        reg_gain = S5K3L8_MAX_AGAIN_REG_VAL;
    }

    real_gain = (float)reg_gain / 32.0;

    return real_gain;
}

static unsigned int s5k3l8_digital_gain_calc(float real_gain, float sensor_real_gain)
{
    unsigned int reg_dig_gain = S5K3L8_MIN_DGAIN_REG_VAL;
    float real_dig_gain = S5K3L8_MIN_DGAIN;

    if(real_gain > S5K3L8_MAX_AGAIN)
    {
        real_dig_gain = real_gain / sensor_real_gain;
    }
    else
    {
        real_dig_gain = S5K3L8_MIN_DGAIN;
    }
    if(real_dig_gain > S5K3L8_MAX_DGAIN)
    {
        real_dig_gain = S5K3L8_MAX_DGAIN;
    }
    reg_dig_gain = (unsigned int)(real_dig_gain * 256);
    return reg_dig_gain;
}
/**
 * FUNCTION: s5k3l8_calculate_exposure
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
int s5k3l8_calculate_exposure(float real_gain,
  unsigned int line_count, sensor_exposure_info_t *exp_info,
  __attribute__((unused)) float s_real_gain)
{
    if (!exp_info)
    {
      return -1;
    }

    exp_info->reg_gain = s5k3l8_real_to_register_gain(real_gain);
    exp_info->sensor_real_gain = s5k3l8_register_to_real_gain(exp_info->reg_gain);
    exp_info->sensor_digital_gain =
      s5k3l8_digital_gain_calc(real_gain, exp_info->sensor_real_gain);
    exp_info->sensor_real_dig_gain = exp_info->sensor_digital_gain / 256;
    exp_info->digital_gain = real_gain/
      (exp_info->sensor_real_gain * exp_info->sensor_real_dig_gain);
    exp_info->line_count = line_count;

    return 0;
}

/**
 * FUNCTION: s5k3l8_fill_exposure_array
 *
 * DESCRIPTION: Fill the sensor exposure array
 **/
int s5k3l8_fill_exposure_array(unsigned int gain,
    __attribute__((unused)) unsigned int digital_gain, unsigned int line,
    unsigned int fl_lines, __attribute__((unused)) int luma_avg,
    __attribute__((unused)) unsigned int hdr_param,
    struct camera_i2c_reg_setting* reg_setting,
    __attribute__((unused)) unsigned int s_gain,
    __attribute__((unused)) int s_linecount,
    __attribute__((unused)) int is_hdr_enabled)
{
    unsigned short i = 0;
    unsigned short reg_count = 0;

  if (!reg_setting) {
    return -1;
  }

  for (i = 0; i < sensor_lib_ptr.groupon_settings.size; i++) {
    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.groupon_settings.reg_setting_a[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      sensor_lib_ptr.groupon_settings.reg_setting_a[i].reg_data;
    reg_count = reg_count + 1;
  }

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr.frame_length_lines;
    reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr.frame_length_lines + 1;
    reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr;
    reg_setting->reg_setting[reg_count].reg_data = (line & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = (line & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.global_gain_addr;
    reg_setting->reg_setting[reg_count].reg_data = (gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.global_gain_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = (gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

  for (i = 0; i < sensor_lib_ptr.groupoff_settings.size; i++) {
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
