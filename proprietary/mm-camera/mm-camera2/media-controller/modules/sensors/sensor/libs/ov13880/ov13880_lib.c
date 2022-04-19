/* ov13880_lib.c
 *
 *Copyright (c) 2017 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include "debug_lib.h"
#include "ov13880_lib.h"

/**
 * FUNCTION: ov13880_real_to_register_gain
 *
 * DESCRIPTION: Calcuate sensor analog gain register value
 **/
static uint32_t ov13880_real_to_register_gain(float gain)
{
    unsigned int reg_gain = 0;
    if (gain > MAX_GAIN) {
      gain = MAX_GAIN;
    } else if (gain < MIN_GAIN) {
      gain = MIN_GAIN;
    }
    gain = gain * 16.0;
    reg_gain = (unsigned short)gain;
    return reg_gain;
}

/**
 * FUNCTION: ov13880_register_to_real_gain
 *
 * DESCRIPTION: Calcuate sensor real gain value
 **/
static float ov13880_register_to_real_gain(uint32_t RegGain)
{
    float real_gain;
    if(RegGain > 0xf8) {
      RegGain = 0xf8;
    } else if (RegGain < 0x10) {
      RegGain = 0x10;
    }
    real_gain = (float) (((float)(RegGain)) / 16.0);
    return real_gain;

}

/**
 * FUNCTION: sensor_calculate_exposure
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
int32_t sensor_calculate_exposure(float real_gain,
  uint32_t line_count, sensor_exposure_info_t *exp_info,
  __attribute__((unused)) float s_real_gain)
{
    if (!exp_info)
    {
      return -1;
    }

    exp_info->reg_gain = ov13880_real_to_register_gain(real_gain);
    exp_info->sensor_real_gain =
      ov13880_register_to_real_gain(exp_info->reg_gain);
    exp_info->sensor_digital_gain = 0x400;
    exp_info->sensor_real_dig_gain = 1.0f;
    exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
    exp_info->line_count = line_count;

    return 0;
}

/**
 * FUNCTION: ov13880_fill_exposure_array
 *
 * DESCRIPTION: Fill the sensor exposure array
 **/
static int32_t sensor_fill_exposure_array(uint32_t gain,
  __attribute__((unused)) uint32_t digital_gain,
  uint32_t line, uint32_t fl_lines,
  __attribute__((unused)) int32_t luma_avg,
  __attribute__((unused)) uint32_t hdr_param,
  struct camera_i2c_reg_setting* reg_setting,
  __attribute__((unused)) unsigned int s_gain,
  __attribute__((unused)) int s_linecount,
  __attribute__((unused)) int is_hdr_enabled)
{

    uint16_t i = 0;
    uint16_t reg_count = 0;

    if (!reg_setting)
    {
      return -1;
    }

    for (i = 0; i < sensor_lib_ptr.groupon_settings.size; i++) {
      reg_setting->reg_setting[reg_count].reg_addr =
        sensor_lib_ptr.groupon_settings.reg_setting_a[i].reg_addr;
      reg_setting->reg_setting[reg_count].reg_data =
        sensor_lib_ptr.groupon_settings.reg_setting_a[i].reg_data;
      reg_setting->reg_setting[reg_count].delay =
        sensor_lib_ptr.groupon_settings.reg_setting_a[i].delay;
      reg_count++;
    }
    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.output_reg_addr.frame_length_lines;
    reg_setting->reg_setting[reg_count].reg_data = (uint8_t)((fl_lines & 0xFF00) >> 8);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.output_reg_addr.frame_length_lines + 1;
    reg_setting->reg_setting[reg_count].reg_data = (uint8_t)(fl_lines & 0xFF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info.coarse_int_time_addr;
    reg_setting->reg_setting[reg_count].reg_data = (uint8_t)((line >> 16) & 0x0F);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info.coarse_int_time_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = (uint8_t)((line>>8) & 0xFF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info.coarse_int_time_addr + 2;
    reg_setting->reg_setting[reg_count].reg_data = (uint8_t)(line & 0xFF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info.global_gain_addr;
    reg_setting->reg_setting[reg_count].reg_data = (uint8_t)((gain & 0xFF00) >> 8);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info.global_gain_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = (uint8_t)(gain & 0xFF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    for (i = 0; i < sensor_lib_ptr.groupoff_settings.size; i++) {
      reg_setting->reg_setting[reg_count].reg_addr =
        sensor_lib_ptr.groupoff_settings.reg_setting_a[i].reg_addr;
      reg_setting->reg_setting[reg_count].reg_data =
        sensor_lib_ptr.groupoff_settings.reg_setting_a[i].reg_data;
      reg_setting->reg_setting[reg_count].delay =
        sensor_lib_ptr.groupoff_settings.reg_setting_a[i].delay;
      reg_count++;
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
