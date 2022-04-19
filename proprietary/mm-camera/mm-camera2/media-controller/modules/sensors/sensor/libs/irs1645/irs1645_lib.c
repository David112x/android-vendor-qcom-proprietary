/* irs1645_SR_lib.c
 *
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <string.h>
#include "debug_lib.h"
#include "irs1645_lib.h"
/**
 * FUNCTION: sensor_real_to_register_gain
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
static uint32_t sensor_real_to_register_gain(float gain)
{
    uint32_t reg_gain = 0;
    gain = gain*16.0;
    reg_gain = (uint16_t)gain;
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
 **/
static int sensor_calculate_exposure(float real_gain,
  __attribute__((unused)) unsigned int line_count,
  sensor_exposure_info_t *exp_info,
  __attribute__((unused)) float s_real_gain)
{
    if (!exp_info)
    {
      return -1;
    }

    exp_info->reg_gain = sensor_real_to_register_gain(real_gain);
    exp_info->sensor_real_gain =
      sensor_register_to_real_gain(exp_info->reg_gain);
    exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
    //exp_info->line_count = line_count;
    exp_info->line_count = IRS_REAL_HEIGHT * 9;
    exp_info->sensor_digital_gain = 0x1;

    return 0;
}

/**
 * FUNCTION: sensor_fill_exposure_array_from_value
 *
 * DESCRIPTION: Fill the sensor exposure array
 **/
static int sensor_fill_exp_array_from_value(int64_t exposure,
  struct camera_i2c_reg_setting* reg_setting,
  __attribute__((unused)) uint32_t* line_count,
  __attribute__((unused)) uint16_t* new_fl_lines)
{
    uint16_t reg_count = 0;

    SLOW("Set exposure %lld", exposure);
    /* Fill array for a write to an unused register address */
    reg_setting->reg_setting[reg_count].reg_addr = 0x9831;
    reg_setting->reg_setting[reg_count].reg_data = 0x0000;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->size = reg_count;
    reg_setting->addr_type = CAMERA_I2C_WORD_ADDR;
    reg_setting->data_type = CAMERA_I2C_WORD_DATA;
    reg_setting->delay = 0;

    return 0;
}

/**
 * FUNCTION: sensor_calculate_exposure_start_time
 *
 * DESCRIPTION: Calculate the start of exposure timestamp
 **/
static int64_t sensor_calculate_exposure_start_time(int64_t sof_timestamp,
  __attribute__((unused)) int64_t exposure_time,
  __attribute__((unused)) int cur_res)
{
    return sof_timestamp;
}

/**
 * FUNCTION: sensor_fill_exposure_array
 *
 * DESCRIPTION: Fill the sensor exposure array
 **/
static int sensor_fill_exposure_array(unsigned int gain,
  __attribute__((unused)) unsigned int digital_gain,
  unsigned int line, unsigned int fl_lines,
  __attribute__((unused)) int luma_avg,
  __attribute__((unused)) unsigned int hdr_param,
  struct camera_i2c_reg_setting* reg_setting,
  __attribute__((unused)) unsigned int s_gain,
  __attribute__((unused)) int s_linecount,
  __attribute__((unused)) int is_hdr_enabled)
{
    int32_t rc = 0;
    uint16_t reg_count = 0;
    uint16_t i = 0;

    reg_setting->size = 0;
    return(0); // ignore generic exposure

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
    reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr.frame_length_lines + 1;
    reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr - 1;
    reg_setting->reg_setting[reg_count].reg_data = line >> 12;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr;
    reg_setting->reg_setting[reg_count].reg_data = (line >> 4) & 0xff;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.coarse_int_time_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = (line << 4) & 0xff;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.global_gain_addr;
    reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info.global_gain_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    for (i = 0; i < sensor_lib_ptr.groupoff_settings.size; i++)
    {
        reg_setting->reg_setting[reg_count].reg_addr =
        sensor_lib_ptr.groupoff_settings.reg_setting_a[i].reg_addr;
        reg_setting->reg_setting[reg_count].reg_data =
        sensor_lib_ptr.groupoff_settings.reg_setting_a[i].reg_data;
        reg_setting->reg_setting[reg_count].delay = 0;
        reg_count = reg_count + 1;
    }

    reg_setting->size = reg_count;
    reg_setting->addr_type = CAMERA_I2C_WORD_ADDR;
    reg_setting->data_type = CAMERA_I2C_BYTE_DATA;
    reg_setting->delay = 0;
    return 0;
}

/**
 * FUNCTION: sensor_fill_fps_array
 *
 * DESCRIPTION: Fill the sensor fps array
 **/
static int sensor_fill_fps_array(float fps,
  struct camera_i2c_reg_setting* reg_setting)
{
    int frame_rate;
    uint16_t reg_count = 0;
    uint16_t reg_data = 0x0;
    int rc = 0;

    if (!reg_setting) {
        return -1;
    }

    /* Check for valid fps */
    frame_rate = (int)fps;
    switch (frame_rate) {
        case 3:
            reg_data = 0xf424;
            break;
        case 5:
            reg_data = 0x927c;
            break;
        default:
            rc = -1;
            break;
    }

    if (rc == 0) {
        reg_setting->reg_setting[reg_count].reg_addr = 0x982c;
        reg_setting->reg_setting[reg_count].reg_data = reg_data;
        reg_setting->reg_setting[reg_count].delay = 0;
        reg_count++;

        reg_setting->reg_setting[reg_count].reg_addr = 0x982f;
        reg_setting->reg_setting[reg_count].reg_data = 0x0001;
        reg_setting->reg_setting[reg_count].delay = 0;
        reg_count++;

        reg_setting->size = reg_count;
        reg_setting->addr_type = CAMERA_I2C_WORD_ADDR;
        reg_setting->data_type = CAMERA_I2C_WORD_DATA;
        reg_setting->delay = 0;
    }

    return rc;
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
