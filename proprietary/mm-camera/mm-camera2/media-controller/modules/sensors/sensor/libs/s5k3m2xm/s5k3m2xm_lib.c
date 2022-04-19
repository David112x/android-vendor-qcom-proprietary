/* s5k3m2xm_lib.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include <string.h>
#include "s5k3m2xm_lib.h"
#include "debug_lib.h"

/**
 * FUNCTION: s5k3m2xm_real_to_register_gain
 *
 * DESCRIPTION: Calcuate sensor analog gain register value
 **/
static unsigned int s5k3m2xm_real_to_register_gain(float real_gain)
{
    unsigned int reg_gain = 0;

    if (real_gain < S5K3M2XM_MIN_AGAIN)
    {
        real_gain = S5K3M2XM_MIN_AGAIN;
        SLOW("S5K3M2XM Requested gain(%f) is lower than the sensor minimum(%f)",
            real_gain, S5K3M2XM_MIN_AGAIN);
    }
    else if (real_gain > S5K3M2XM_MAX_AGAIN)
    {
        real_gain = S5K3M2XM_MAX_AGAIN;
    }

    reg_gain = (unsigned int)(real_gain * 32);

    return reg_gain;
}

/**
 * FUNCTION: s5k3m2xm_register_to_real_gain
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
static float s5k3m2xm_register_to_real_gain(unsigned int reg_gain)
{
    float real_gain;

    if (reg_gain <= S5K3M2XM_MIN_AGAIN_REG_VAL)
    {
        reg_gain = S5K3M2XM_MIN_AGAIN_REG_VAL;
        SHIGH("S5K3M2XM Requested gain(%d) is lower than the sensor minimum(%f)",
            reg_gain, S5K3M2XM_MIN_GAIN);
    }
    else if (reg_gain > S5K3M2XM_MAX_AGAIN_REG_VAL)
    {
        reg_gain = S5K3M2XM_MAX_AGAIN_REG_VAL;
    }

    real_gain = (float)reg_gain / 32;

    return real_gain;
}

/**
 * FUNCTION: s5k3m2xm_digital_gain_calc
 *
 * DESCRIPTION: Calcuate the sensor digital gain
 **/
static unsigned int s5k3m2xm_digital_gain_calc(float real_gain, float sensor_real_gain)
{
    unsigned int reg_dig_gain = S5K3M2XM_MIN_DGAIN_REG_VAL;
    float real_dig_gain = S5K3M2XM_MIN_DGAIN;

    if(real_gain > S5K3M2XM_MAX_AGAIN)
    {
        real_dig_gain = real_gain / sensor_real_gain;
    }
    else
    {
        real_dig_gain = S5K3M2XM_MIN_DGAIN;
    }

    if(real_dig_gain > S5K3M2XM_MAX_DGAIN)
    {
        real_dig_gain = S5K3M2XM_MAX_DGAIN;
    }

    reg_dig_gain = (unsigned int)(real_dig_gain * 256);

    return reg_dig_gain;
}


/**
 * FUNCTION: s5k3m2xm_calculate_exposure
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
int32_t s5k3m2xm_calculate_exposure(float real_gain,
  uint32_t line_count, sensor_exposure_info_t *exp_info,
  __attribute__((unused)) float s_real_gain)
{
    if (!exp_info)
    {
      return -1;
    }

    exp_info->reg_gain = s5k3m2xm_real_to_register_gain(real_gain);
    exp_info->sensor_real_gain = s5k3m2xm_register_to_real_gain(exp_info->reg_gain);
    exp_info->sensor_digital_gain =
      s5k3m2xm_digital_gain_calc(real_gain, exp_info->sensor_real_gain);
    exp_info->sensor_real_dig_gain = exp_info->sensor_digital_gain / 256;
    exp_info->digital_gain = real_gain/
      (exp_info->sensor_real_gain * exp_info->sensor_real_dig_gain);
    exp_info->line_count = line_count;

    return 0;
}

/**
 * FUNCTION: s5k3m2xm_fill_exposure_array
 *
 * DESCRIPTION: Fill the sensor exposure array
 **/
int32_t s5k3m2xm_fill_exposure_array(uint32_t gain,
    uint32_t digital_gain, uint32_t line,
    uint32_t fl_lines, __attribute__((unused)) int32_t luma_avg,
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

    for (i = 0; i < sensor_lib_ptr.groupon_settings.size; i++)
    {
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

    reg_setting->reg_setting[reg_count].reg_addr = S5K3M2XM_DIG_GAIN_GR_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = S5K3M2XM_DIG_GAIN_GR_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = S5K3M2XM_DIG_GAIN_R_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = S5K3M2XM_DIG_GAIN_R_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = S5K3M2XM_DIG_GAIN_B_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = S5K3M2XM_DIG_GAIN_B_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = S5K3M2XM_DIG_GAIN_GB_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = S5K3M2XM_DIG_GAIN_GB_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    for (i = 0; i < sensor_lib_ptr.groupoff_settings.size; i++)
    {
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
