/* ov2685_scv3b4035_lib.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include "ov2685_scv3b4035_lib.h"
#include "debug_lib.h"

/**
 * FUNCTION: sensor_real_to_register_gain
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
static unsigned int sensor_real_to_register_gain(float gain)
{
    unsigned short reg_analog_gain;
    unsigned char  reg_gain_bit_7to0 = 0;
    unsigned int   addl_gain;

    /* Limiter - min / max */
    if (gain <= 1.0) {
      reg_gain_bit_7to0 = 0x10;
    } else if (gain > MAX_ANALOG_GAIN) {
      reg_gain_bit_7to0 = 0xf8;
    } else if (gain > 1.0 && gain < 2.0) {
      addl_gain = (gain - 1.000)/0.0625;
      reg_gain_bit_7to0 = 16 + addl_gain;
    } else if (gain >= 2.0 && gain <= 4.0) {
      addl_gain = (gain - 2.0000)/0.125;
      reg_gain_bit_7to0 = 32 + addl_gain * 2;
    } else if (gain >= 4.0 && gain <= 8.0) {
      addl_gain = (gain - 4.0000)/0.2500;
      reg_gain_bit_7to0 = 64 + addl_gain * 4;
    } else {
      addl_gain = (gain - 8.0)/0.5;
      reg_gain_bit_7to0 = 128 + addl_gain * 8;
    }

    reg_analog_gain = (reg_gain_bit_7to0 & 0xFF);

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
    unsigned char reg_gain_bit_7to0 = 0;

    reg_gain_bit_7to0 = reg_gain & 0x00FF;

    if (reg_gain_bit_7to0 > 0xF8) {
      real_gain = 15.5;
    } else if(reg_gain_bit_7to0 < 0x10) {
      real_gain = 1.0;
    } else if (reg_gain_bit_7to0 >= 0x10 && reg_gain_bit_7to0 <= 0x20) {
      real_gain = 1.000 + (reg_gain_bit_7to0 - 0x10) * 0.0625;
    } else if (reg_gain_bit_7to0 > 0x20 && reg_gain_bit_7to0 <= 0x40) {
      real_gain = 2.000 + (reg_gain_bit_7to0 + (reg_gain_bit_7to0 & 1) - 0x20)
        * 0.0625;
    } else if (reg_gain_bit_7to0 > 0x40 && reg_gain_bit_7to0 <= 0x80) {
      real_gain = 4.000 + (reg_gain_bit_7to0 + (4 - (reg_gain_bit_7to0 & 3))
        - 0x40) * 0.0625;
    } else
      real_gain = 8.000 + (reg_gain_bit_7to0 + (8 - (reg_gain_bit_7to0 & 7))
        - 0x80) * 0.0625;

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

    if(real_gain > MAX_ANALOG_GAIN)
    {
        real_dig_gain = real_gain / sensor_real_gain;
    }
    else
    {
        real_dig_gain = MIN_GAIN;
    }

    if(real_dig_gain > MAX_DIGITAL_GAIN)
    {
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
    if (!exp_info)
    {
      return -1;
    }

    exp_info->reg_gain = sensor_real_to_register_gain(real_gain);
    exp_info->sensor_real_gain =
      sensor_register_to_real_gain(exp_info->reg_gain);
    exp_info->sensor_digital_gain =
      sensor_digital_gain_calc(real_gain, exp_info->sensor_real_gain);
    exp_info->sensor_real_dig_gain =
      (float)exp_info->sensor_digital_gain / OV2685_SCV3B4035_MAX_DGAIN_DECIMATOR;
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

    if (!reg_setting)
    {
      return -1;
    }

    gain = (gain & 0xFFFF);

    for (i = 0; i < sensor_lib_ptr.groupon_settings.size; i++) {
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
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr - 1;
    reg_setting->reg_setting[reg_count].reg_data = line >> 12;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr;
    reg_setting->reg_setting[reg_count].reg_data = line >> 4;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = line << 4;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.global_gain_addr;
    reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF00) >> 8;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.global_gain_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF);
    reg_count++;
    /*
      reg_setting->reg_setting[reg_count].reg_addr = REG_DIGITAL_GAIN_FRAC;
      reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0xFF00) >> 8;
      reg_count++;

      reg_setting->reg_setting[reg_count].reg_addr = REG_DIGITAL_GAIN_FRAC + 1;
      reg_setting->reg_setting[reg_count].reg_data = digital_gain & 0xFF;
      reg_count++;
     */
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
