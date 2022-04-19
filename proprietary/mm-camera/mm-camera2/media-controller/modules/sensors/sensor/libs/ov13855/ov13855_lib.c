/*============================================================================

    Copyright (c) 2017 Qualcomm Technologies, Inc.
    All Rights Reserved.
    Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include <stdio.h>
#include "ov13855_lib.h"
#include "debug_lib.h"
#include <utils/Log.h>

/**
 * FUNCTION: ov13855_real_to_register_gain
 *
 * DESCRIPTION: Calcuate sensor analog gain register value
 **/
static unsigned int ov13855_real_to_register_gain(float real_gain)
{
    unsigned int reg_gain = 0;

    if (real_gain < (float)OV13855_MIN_AGAIN)
    {
        real_gain = (float)OV13855_MIN_AGAIN;
        SHIGH("OV13855 Requested gain(%f) is lower than the sensor minimum(%d)",
            real_gain, OV13855_MIN_AGAIN_REG_VAL);
    }
    else if (real_gain > (float)OV13855_MAX_AGAIN)
    {
        real_gain = (float)OV13855_MAX_AGAIN;
    }

    reg_gain = (uint32_t)(real_gain * 128);
    if (reg_gain < 0x200)
        reg_gain = (reg_gain >> 4 ) << 4 ;
    else if (reg_gain < 0x400)
        reg_gain = (reg_gain >> 5 ) << 5 ;
    else if (reg_gain < 0x800)
        reg_gain = (reg_gain >> 6 ) << 6 ;
    return reg_gain;
}

/**
 * FUNCTION: ov13855_register_to_real_gain
 *
 * DESCRIPTION: Calcuate sensor real gain value
 **/
static float ov13855_register_to_real_gain(unsigned int reg_gain)
{
    float real_gain;

    if (reg_gain < OV13855_MIN_AGAIN_REG_VAL)
    {
        reg_gain = OV13855_MIN_AGAIN_REG_VAL;
        SHIGH("OV13855 Requested gain(%d) is lower than the sensor minimum(%d)",
            reg_gain, OV13855_MIN_AGAIN_REG_VAL);
    }
    else if(reg_gain > OV13855_MAX_AGAIN_REG_VAL)
    {
        reg_gain = OV13855_MAX_AGAIN_REG_VAL;
    }

    real_gain = (float)reg_gain / 128;

    return real_gain;
}

/**
 * FUNCTION: ov13855_digital_gain_calc
 *
 * DESCRIPTION: Calcuate the sensor digital gain
 **/
static unsigned int ov13855_digital_gain_calc(float real_gain, float sensor_real_gain)
{
    unsigned int reg_dig_gain = OV13855_MIN_DGAIN_REG_VAL;
    float real_dig_gain = OV13855_MIN_DGAIN;

    if(real_gain > OV13855_MAX_AGAIN)
    {
        real_dig_gain = real_gain / sensor_real_gain;
    }
    else
    {
        real_dig_gain = OV13855_MIN_DGAIN;
    }

    if(real_dig_gain > OV13855_MAX_DGAIN_REG_VAL)
    {
        real_dig_gain = OV13855_MAX_DGAIN_REG_VAL;
    }
    reg_dig_gain = (unsigned int)(real_dig_gain * 1024);
    return reg_dig_gain;
}

/**
 * FUNCTION: sensor_calculate_exposure
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
int ov13855_calculate_exposure(float real_gain,
  unsigned int line_count, sensor_exposure_info_t *exp_info,
  __attribute__((unused)) float s_real_gain)
{
    if (!exp_info) {
      return -1;
    }

    exp_info->reg_gain = ov13855_real_to_register_gain(real_gain);
    exp_info->sensor_real_gain =
      ov13855_register_to_real_gain(exp_info->reg_gain);
    exp_info->sensor_digital_gain =
      ov13855_digital_gain_calc(real_gain, exp_info->sensor_real_gain);
    exp_info->sensor_real_dig_gain =
      (float)exp_info->sensor_digital_gain / OV13855_MAX_DGAIN_DECIMATOR;
    exp_info->digital_gain =
      real_gain /(exp_info->sensor_real_gain * exp_info->sensor_real_dig_gain);
    exp_info->line_count = line_count;

    return 0;
}

/**
 * FUNCTION: ov13855_fill_exposure_array
 *
 * DESCRIPTION: Fill the sensor exposure array
 **/
int ov13855_fill_exposure_array(unsigned int gain,
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
    reg_setting->reg_setting[reg_count].reg_data = (line & 0x000FF000) >> 12;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = (line & 0x00000FF0) >> 4;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info.coarse_int_time_addr + 2;
    reg_setting->reg_setting[reg_count].reg_data = (line & 0x0000000F) << 4;
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
