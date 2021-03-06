/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include "eeprom_lib.h"
#include "debug_lib.h"
#include "imx376_lib.h"
#include <cutils/log.h>

/**
 * FUNCTION: imx376_real_to_register_gain
 *
 * DESCRIPTION: Calcuate sensor analog gain register value
 **/
static unsigned int imx376_real_to_register_gain(float real_gain)
{
    unsigned int reg_gain = 0;

    if (real_gain < IMX376_MIN_AGAIN)
    {
        real_gain = IMX376_MIN_AGAIN;
        SHIGH("IMX376 Requested gain(%f) is lower than the sensor minimum(%d)",
            real_gain, IMX376_MIN_AGAIN);
    }
    else if (real_gain > IMX376_MAX_AGAIN)
    {
        real_gain = IMX376_MAX_AGAIN;
    }

    reg_gain = (unsigned int)(1024.0 - (1024.0 / real_gain));

    return reg_gain;
}

/**
 * FUNCTION: imx376_register_to_real_gain
 *
 * DESCRIPTION: Calcuate sensor real gain value
 **/
static float imx376_register_to_real_gain(unsigned int reg_gain)
{
    float gain;

    if(reg_gain > IMX376_MAX_AGAIN_REG_VAL)
        reg_gain = IMX376_MAX_AGAIN_REG_VAL;

    gain = 1024.0 / (1024.0 - reg_gain);

    return gain;
}

/**
 * FUNCTION: imx376_digital_gain_calc
 *
 * DESCRIPTION: Calcuate the sensor digital gain
 **/
static unsigned int imx376_digital_gain_calc(
 float real_gain, float sensor_real_gain)
{
    unsigned int reg_dig_gain = IMX376_MIN_DGAIN_REG_VAL;
    float real_dig_gain = IMX376_MIN_DGAIN;

    if(real_gain > IMX376_MAX_AGAIN)
    {
        real_dig_gain = real_gain / sensor_real_gain;
    }
    else
    {
        real_dig_gain = IMX376_MIN_DGAIN;
    }

    if(real_dig_gain > IMX376_MAX_DGAIN)
    {
        real_dig_gain = IMX376_MAX_DGAIN;
    }

    reg_dig_gain = (unsigned int)(real_dig_gain * 256);

    return reg_dig_gain;
}

/**
 * FUNCTION: sensor_calculate_exposure
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
int sensor_calculate_exposure(float real_gain,
  unsigned int line_count, sensor_exposure_info_t *exp_info, float s_real_gain)
{
    if (!exp_info)
    {
      return -1;
    }

    exp_info->reg_gain = imx376_real_to_register_gain(real_gain);
    exp_info->sensor_real_gain =
      imx376_register_to_real_gain(exp_info->reg_gain);
    exp_info->sensor_digital_gain =
      imx376_digital_gain_calc(real_gain, exp_info->sensor_real_gain);
    exp_info->sensor_real_dig_gain =
      (float)exp_info->sensor_digital_gain / IMX376_MAX_DGAIN_DECIMATOR;
    exp_info->digital_gain =
      real_gain /(exp_info->sensor_real_gain * exp_info->sensor_real_dig_gain);
    exp_info->line_count = line_count;
    exp_info->s_reg_gain = imx376_real_to_register_gain(s_real_gain);

    return 0;
}

/**
 * FUNCTION: imx376_fill_exposure_array
 *
 * DESCRIPTION: Fill the sensor exposure array
 **/
int sensor_fill_exposure_array(unsigned int gain,
  unsigned int digital_gain, unsigned int line, unsigned int fl_lines,
  __attribute__((unused)) int luma_avg, unsigned int hdr_param,
  struct camera_i2c_reg_setting* reg_setting,
  __attribute__((unused)) unsigned int s_gain,
  __attribute__((unused)) int s_linecount,
  __attribute__((unused)) int is_hdr_enabled)
{
    unsigned short i = 0;
    unsigned short reg_count = 0;
    unsigned int luma_delta = hdr_param & 0x0000FFFF;

    if (!reg_setting)
    {
      return -1;
    }
    SERR("sensor_fill_exposure_array, cur_res:%d",1);

    for (i = 0; i < sensor_lib_ptr.groupon_settings.size; i++) {
      reg_setting->reg_setting[reg_count].reg_addr =
        sensor_lib_ptr.groupon_settings.reg_setting_a[i].reg_addr;
      reg_setting->reg_setting[reg_count].reg_data =
        sensor_lib_ptr.groupon_settings.reg_setting_a[i].reg_data;
      reg_count = reg_count + 1;
    }

    if(luma_delta != 0){
      unsigned int hdr_indoor_detected = (hdr_param >> 16) & 0x1;
      unsigned int ratio = 8;

      if(hdr_indoor_detected)
        ratio = 2;
      else
        ratio = 8;

      reg_setting->reg_setting[reg_count].reg_addr = IMX376_EXP_RATIO_ADDR;
      reg_setting->reg_setting[reg_count].reg_data = ratio;

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

    reg_setting->reg_setting[reg_count].reg_addr = IMX376_DIG_GAIN_GR_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX376_DIG_GAIN_GR_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX376_DIG_GAIN_R_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX376_DIG_GAIN_R_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX376_DIG_GAIN_B_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX376_DIG_GAIN_B_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX376_DIG_GAIN_GB_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX376_DIG_GAIN_GB_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
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
 * FUNCTION: sensor_fill_awb_array
 *
 * DESCRIPTION: Fill the AWB HDR array
 **/
static int sensor_fill_awb_array(unsigned short awb_gain_r,
  unsigned short awb_gain_b, struct camera_i2c_seq_reg_setting* reg_setting)
{
    unsigned short reg_count = 0;

    reg_setting->reg_setting[reg_count].reg_addr = IMX376_ABS_GAIN_R_WORD_ADDR;
    reg_setting->reg_setting[reg_count].reg_data[0] = (awb_gain_r & 0xFF00) >> 8;
    reg_setting->reg_setting[reg_count].reg_data[1] = (awb_gain_r & 0xFF);
    reg_setting->reg_setting[reg_count].reg_data_size = 2;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX376_ABS_GAIN_B_WORD_ADDR;
    reg_setting->reg_setting[reg_count].reg_data[0] = (awb_gain_b & 0xFF00) >> 8;
    reg_setting->reg_setting[reg_count].reg_data[1] = (awb_gain_b & 0xFF);
    reg_setting->reg_setting[reg_count].reg_data_size = 2;
    reg_count = reg_count + 1;

    reg_setting->size = reg_count;
    reg_setting->addr_type = CAMERA_I2C_WORD_ADDR;
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
  ALOGE("Enter imx376_lib");
  return &sensor_lib_ptr;
}
