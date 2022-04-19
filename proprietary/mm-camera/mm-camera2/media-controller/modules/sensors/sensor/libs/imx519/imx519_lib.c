/* imx519_lib.c
 *
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#include <stdio.h>
#include <string.h>
#include "debug_lib.h"
#include "imx519_lib.h"

#if !defined RETURN_ERROR_ON_NULL
#define RETURN_ERROR_ON_NULL(expr)\
    if ((expr) == NULL) {\
        SERR("failed evaluated expression is NULL");\
        return SENSOR_FAILURE;\
     }
#endif

static unsigned int imx519_real_to_register_gain(float real_gain)
{
    unsigned int reg_gain = 0;

    if (real_gain < IMX519_MIN_AGAIN)
    {
      real_gain = IMX519_MIN_AGAIN;
    }
    else if (real_gain > IMX519_MAX_AGAIN)
    {
      real_gain = IMX519_MAX_AGAIN;
    }

    reg_gain = (unsigned int)(1024.0 - (1024.0 / real_gain));

    return reg_gain;
}

/**
 * FUNCTION: imx519_register_to_real_gain
 *
 * DESCRIPTION: Calcuate sensor real gain value
 **/
static float imx519_register_to_real_gain(unsigned int reg_gain)
{
    float gain;

    if(reg_gain > IMX519_MAX_AGAIN_REG_VAL)
      reg_gain = IMX519_MAX_AGAIN_REG_VAL;

    gain = 1024.0 / (1024.0 - reg_gain);

    return gain;
}

/**
 * FUNCTION: imx519_digital_gain_calc
 *
 * DESCRIPTION: Calcuate the sensor digital gain
 **/
static unsigned int imx519_digital_gain_calc(
 float real_gain, float sensor_real_gain)
{
    unsigned int reg_dig_gain = IMX519_MIN_DGAIN_REG_VAL;
    float real_dig_gain = IMX519_MIN_DGAIN;

    if(real_gain > IMX519_MAX_AGAIN)
    {
      real_dig_gain = real_gain / sensor_real_gain;
    }
    else
    {
      real_dig_gain = IMX519_MIN_DGAIN;
    }

    if(real_dig_gain > IMX519_MAX_DGAIN)
    {
      real_dig_gain = IMX519_MAX_DGAIN;
    }

    reg_dig_gain =
      (unsigned int)(real_dig_gain * IMX519_MAX_DGAIN_DECIMATOR);

    return reg_dig_gain;
}

/**
 * FUNCTION: sensor_calculate_exposure
 *
 * DESCRIPTION: Calcuate the sensor exposure
 **/
int sensor_calculate_exposure(float real_gain,
  unsigned int line_count, sensor_exposure_info_t *exp_info,
  __attribute__((unused))float s_real_gain)
{
    if (!exp_info)
    {
      return -1;
    }

    exp_info->reg_gain = imx519_real_to_register_gain(real_gain);
    exp_info->sensor_real_gain =
      imx519_register_to_real_gain(exp_info->reg_gain);
    exp_info->sensor_digital_gain =
      imx519_digital_gain_calc(real_gain, exp_info->sensor_real_gain);
    exp_info->sensor_real_dig_gain =
      (float)exp_info->sensor_digital_gain / IMX519_MAX_DGAIN_DECIMATOR;
    exp_info->digital_gain =
      real_gain /(exp_info->sensor_real_gain * exp_info->sensor_real_dig_gain);
    exp_info->line_count = line_count;

    return 0;
}

/**
 * FUNCTION: imx519_NormalizeLineCount
 *
 * DESCRIPTION: Adjust the linecount for binning mode
 **/
static unsigned int imx519_NormalizeLineCount(
    unsigned int inputLineCount)
{
    unsigned int normalizedLineCount = inputLineCount;

    /*
    For resolutions with H:1/2 and V:1/2 binning w/o PDAF following restriction apply on linecount:
    If CIT <= 14, acceptable linecount follows 4N+2 rule. That implies N can only be {0, 1, 2, 3}.
    If CIT > 14 , acceptable linecount is only even numbered values.
    */

    if (inputLineCount > 14)
    {
        if (inputLineCount & 0x1)
        {
            normalizedLineCount++;
        }
    }
    else if (inputLineCount > 10)
    {
        normalizedLineCount = 14;
    }
    else if (inputLineCount > 6)
    {
        normalizedLineCount = 10;
    }
    else if (inputLineCount > 2)
    {
        normalizedLineCount = 6;
    }
    else
    {
        normalizedLineCount = 2;
    }

    return normalizedLineCount;
}

/**
 * FUNCTION: imx519_GetLineCount
 *
 * DESCRIPTION: Adjust the linecount according to the resolution
 **/
static unsigned int imx519_GetLineCount(unsigned int inputLineCount,
  int32_t resolutionIndex)
{
    unsigned int lineCount = inputLineCount;

    switch (resolutionIndex)
    {
        /* For full res modes IMX519 only accepts odd line count
           when the CIT is less than or equal to 16. Please refer
           Software Reference Manual IMX519 page - 94 Table 5-24 for
           more details
        */
        case 0:
        case 5:
        case 6:
        {
            if ((inputLineCount <= 16) && !(inputLineCount & 0x1))
            {
                lineCount++;
            }
        }
        break;
        /* For modes without PDAF and bin 2, linecount needs to be normalized based on
           certain rules described in NormalizeLineCount. Please refer
           Software Reference Manual IMX519 page - 94 Table 5-24 for
           more details
        */
        case 2:
        case 3:
        case 4:
        {
            lineCount = imx519_NormalizeLineCount(inputLineCount);
        }
        break;
        default:
            /*
            Remaining resolution modes do not have any restrictions.
            */
            break;
    }

    return lineCount;
}

/**
 * FUNCTION: imx519_fill_exposure_array
 *
 * DESCRIPTION: Fill the sensor exposure array
 **/
 int sensor_fill_exposure_array_with_res(unsigned int gain,
  unsigned int digital_gain, unsigned int line, unsigned int fl_lines,
  __attribute__((unused)) int luma_avg,
  __attribute__((unused)) unsigned int hdr_param,
  struct camera_i2c_reg_setting* reg_setting,
  __attribute__((unused)) unsigned int s_gain,
  __attribute__((unused)) signed int s_linecount,
  __attribute__((unused)) int is_hdr_enabled, int cur_res)
{
    unsigned short i = 0;
    unsigned short reg_count = 0;

    if (!reg_setting)
    {
      return -1;
    }
    for (i = 0; i < sensor_lib_ptr.groupon_settings.size; i++) {
      reg_setting->reg_setting[reg_count].reg_addr =
        sensor_lib_ptr.groupon_settings.reg_setting_a[i].reg_addr;
      reg_setting->reg_setting[reg_count].reg_data =
        sensor_lib_ptr.groupon_settings.reg_setting_a[i].reg_data;
      reg_count = reg_count + 1;
    }
#ifdef ENABLE_SHORT_EXPOSURE
    /* HDR control */
    if(is_hdr_enabled){
      reg_setting->reg_setting[reg_count].reg_addr =
        IMX519_SHORT_COARSE_INT_TIME_ADDR;
      reg_setting->reg_setting[reg_count].reg_data=(s_linecount & 0xFF00)>>8;
      reg_setting->reg_setting[reg_count].delay = 0;
      reg_count = reg_count + 1;

      reg_setting->reg_setting[reg_count].reg_addr =
        IMX519_SHORT_COARSE_INT_TIME_ADDR + 1;
      reg_setting->reg_setting[reg_count].reg_data = (s_linecount & 0xFF);
      reg_setting->reg_setting[reg_count].delay = 0;
      reg_count = reg_count + 1;

      reg_setting->reg_setting[reg_count].reg_addr =
        IMX519_SHORT_GAIN_ADDR;
      reg_setting->reg_setting[reg_count].reg_data = (s_gain & 0xFF00) >> 8;
      reg_setting->reg_setting[reg_count].delay = 0;
      reg_count = reg_count + 1;

      reg_setting->reg_setting[reg_count].reg_addr =
        IMX519_SHORT_GAIN_ADDR +  1;
      reg_setting->reg_setting[reg_count].reg_data = (s_gain & 0xFF);
      reg_setting->reg_setting[reg_count].delay = 0;
      reg_count = reg_count + 1;
    }
#endif
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

    line = imx519_GetLineCount(line, cur_res);

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

    reg_setting->reg_setting[reg_count].reg_addr = IMX519_DIG_GAIN_GR_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX519_DIG_GAIN_GR_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX519_DIG_GAIN_R_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX519_DIG_GAIN_R_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX519_DIG_GAIN_B_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX519_DIG_GAIN_B_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX519_DIG_GAIN_GB_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX519_DIG_GAIN_GB_ADDR + 1;
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
 * FUNCTION: sensor_open_lib
 *
 * DESCRIPTION: Open sensor library and returns data pointer
 **/
void *sensor_open_lib(void)
{
  return &sensor_lib_ptr;
}
