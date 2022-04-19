/* ov13850_lib.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include "debug_lib.h"
#include "ov13850_lib.h"

static SensorGainMap gain_conversion[64] =
{{256,16},{272,17},{288,18},{304,19},{320,20},{336,21},{352,22},{368,23},
{384,24},{400,25},{416,26},{432,27},{448,28},{464,29},{480,30},{496,31},
{512,32},{544,33},{576,35},{608,37},{640,39},{672,41},{704,43},{736,45},
{768,47},{800,49},{832,51},{864,53},{896,55},{928,57},{960,59},{992,61},
{1024,63},{1088,65},{1152,69},{1216,73},{1280,77},{1344,81},{1408,85},
{1472,89},{1536,93},{1600,97},{1664,101},{1728,105},{1792,109},{1856,113},
{1920,117},{1984,121},{2048,125},{2176,129},{2304,137},{2432,145},{2560,153},
{2688,161},{2816,169},{2944,177},{3072,185},{3200,193},{3328,201},{3456,209},
{3584,217},{3712,225},{3840,233},{3968,241}};

/**
 * FUNCTION: ov13850_real_to_register_gain
 *
 * DESCRIPTION: Calcuate sensor analog gain register value
 **/
static uint32_t ov13850_real_to_register_gain(float gain)
{
   uint32_t RealGainQ8;
   uint32_t left_gain_index = 0,right_gain_index = 63,middle_gain_index = 0;

   /* Input:
   RealGainQ8 - unsigned 32 bit integer in Q8 format, for example Real Gain of
   1.0 is 1.0 x 256
   Return:
   RegGain  - unsigned 16 bit integer representing Register value equavalent of
   Real Gain.
   Logic:  The  real to register gain map is non linear. Converting the real
   gain and register gain map into a SensorGainMap structure
   which has real gain in Q8 format and correct register gain corresponding to that.
   use binary search to read from the map in Real to Register gain function.

   Adding Boundary check*/

   RealGainQ8 = (uint32_t)(gain*256.0f);

   if (RealGainQ8 < 256) {
     RealGainQ8 = 256;
   }
   else if (RealGainQ8 > 3968) {
     RealGainQ8 = 3968;
   }
   middle_gain_index = (left_gain_index + right_gain_index) / 2;

   while(left_gain_index <= right_gain_index)
   {
      if (RealGainQ8 > gain_conversion[middle_gain_index].real_gain) {
            if (RealGainQ8 < gain_conversion[middle_gain_index + 1].real_gain)
                 break;
            left_gain_index = middle_gain_index + 1;
        }
        else if (RealGainQ8 == gain_conversion[middle_gain_index].real_gain ) {
          break;
        }
        else {
          right_gain_index = middle_gain_index - 1;
        }
        middle_gain_index = (left_gain_index + right_gain_index) / 2;
    }
    SLOW("gain = %f Real_Gain = %d, Reg_gain = %d",
              gain,RealGainQ8,gain_conversion[middle_gain_index].reg_gain);
    return gain_conversion[middle_gain_index].reg_gain;
}

/**
 * FUNCTION: ov13850_register_to_real_gain
 *
 * DESCRIPTION: Calcuate sensor real gain value
 **/
static float ov13850_register_to_real_gain(uint32_t RegGain)
{
  uint32_t left_gain_index = 0,right_gain_index = 63,middle_gain_index = 0;
  float real_gain;
  /* Input:
       RegGain - unsigned 16 bit integer in Q8 format.
       Return:
       RealGainQ8 - Unsigned 32 bit integer representing real gain
       value equavalent of register gain
   */

  if (RegGain < OV13850_MIN_AGAIN_REG_VAL) {
    RegGain = OV13850_MIN_AGAIN_REG_VAL;
  }
  else if (RegGain > OV13850_MAX_AGAIN_REG_VAL) {
    RegGain = OV13850_MAX_AGAIN_REG_VAL;
  }
  middle_gain_index = (left_gain_index + right_gain_index) / 2;

  while (left_gain_index <= right_gain_index) {
    if (gain_conversion[middle_gain_index].reg_gain < RegGain) {
        if (RegGain < gain_conversion[middle_gain_index + 1].reg_gain)
           break;
        left_gain_index = middle_gain_index + 1;
    }
    else if (gain_conversion[middle_gain_index].reg_gain == RegGain) {
            break;
    }
    else {
      right_gain_index = middle_gain_index - 1;
    }
    middle_gain_index = (left_gain_index + right_gain_index) / 2;
  }
  real_gain = (float)((gain_conversion[middle_gain_index].real_gain) / 256.0f);
  return real_gain;
}

/**
 * FUNCTION: ov13850_digital_gain_calc
 *
 * DESCRIPTION: Calcuate the sensor digital gain
 **/
static uint32_t ov13850_digital_gain_calc(float real_gain, float sensor_real_gain)
{
    uint32_t reg_dig_gain = OV13850_MIN_DGAIN_REG_VAL;
    float real_dig_gain = OV13850_MIN_DGAIN;

    if(real_gain > OV13850_MAX_AGAIN)
    {
        real_dig_gain = real_gain / sensor_real_gain;
    }
    else
    {
        real_dig_gain = OV13850_MIN_DGAIN;
    }

    if(real_dig_gain > OV13850_MAX_DGAIN)
    {
        real_dig_gain = OV13850_MAX_DGAIN;
    }

    reg_dig_gain = (uint32_t)(real_dig_gain * 256);

    return reg_dig_gain;
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

    exp_info->reg_gain = ov13850_real_to_register_gain(real_gain);
    exp_info->sensor_real_gain =
      ov13850_register_to_real_gain(exp_info->reg_gain);
    exp_info->sensor_digital_gain =
      ov13850_digital_gain_calc(real_gain, exp_info->sensor_real_gain);
    exp_info->sensor_real_dig_gain =
      (float)exp_info->sensor_digital_gain / OV13850_MAX_DGAIN_DECIMATOR;
    exp_info->digital_gain =
      real_gain /(exp_info->sensor_real_gain * exp_info->sensor_real_dig_gain);
    exp_info->line_count = line_count;

    return 0;
}

/**
 * FUNCTION: ov13850_fill_exposure_array
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
    reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.output_reg_addr.frame_length_lines + 1;
    reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info.coarse_int_time_addr;
    reg_setting->reg_setting[reg_count].reg_data = (line & 0x000FF000) >> 12;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info.coarse_int_time_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = (line & 0x00000FF0) >> 4;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info.coarse_int_time_addr + 2;
    reg_setting->reg_setting[reg_count].reg_data = (line & 0x0000000F) << 4;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info.global_gain_addr;
    reg_setting->reg_setting[reg_count].reg_data = (gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info.global_gain_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = (gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count++;

    // reg_setting->reg_setting[reg_count].reg_addr = OV13850_DIG_GAIN;
    // reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    // reg_setting->reg_setting[reg_count].delay = 0;
    // reg_count++;

    // reg_setting->reg_setting[reg_count].reg_addr = OV13850_DIG_GAIN + 1;
    // reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    // reg_setting->reg_setting[reg_count].delay = 0;
    // reg_count++;

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
