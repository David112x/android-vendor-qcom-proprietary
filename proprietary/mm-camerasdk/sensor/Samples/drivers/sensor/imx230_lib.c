/* imx230_lib.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <string.h>
#include "eeprom_lib.h"
#include "debug_lib.h"
#include "pdaf_lib.h"
#include "imx230_lib.h"

/** The following mask defines how many MSBs are valid data.
 *  We have 14 bits for luma, and the mask is defining which 8 upper bits are
 *  valid data. 0xFF means all are valid, 0x7F means bit 13 is not valid data,
 *  0x3F means bits 13 and 12 are not valid data, and so on...
 *  Depending on the HDR exposure ratio we may have the following masks:
 *
 *  0xFF - HDR exposure ratio 1:16
 *  0x7F - HDR exposure ratio 1:8
 *  0x3F - HDR exposure ratio 1:4
 *  0x1F - HDR exposure ratio 1:2
 *  0x0F - HDR exposure ratio 1:1
 **/
#define VIDEO_HDR_LUMA_MASK_RATIO_16 0xFF
#define VIDEO_HDR_LUMA_MASK_RATIO_8  0x7F
#define VIDEO_HDR_LUMA_MASK_RATIO_4  0x3F
#define VIDEO_HDR_LUMA_MASK_RATIO_2  0x1F
#define VIDEO_HDR_LUMA_MASK_RATIO_1  0x0F

/** The following definition is used to correct the final luma data
 *  to be 8 bits. Valid values:
 *
 *  4 - HDR exposure ratio 1:16
 *  3 - HDR exposure ratio 1:8
 *  2 - HDR exposure ratio 1:4
 *  1 - HDR exposure ratio 1:2
 *  0 - HDR exposure ratio 1:1
 *
 **/
#define VIDEO_HDR_EXPOSURE_RATIO_CORRECTION_16 6
#define VIDEO_HDR_EXPOSURE_RATIO_CORRECTION_8  5
#define VIDEO_HDR_EXPOSURE_RATIO_CORRECTION_4  4
#define VIDEO_HDR_EXPOSURE_RATIO_CORRECTION_2  3
#define VIDEO_HDR_EXPOSURE_RATIO_CORRECTION_1  2

/**
 * FUNCTION: imx230_real_to_register_gain
 *
 * DESCRIPTION: Calcuate sensor analog gain register value
 **/
static unsigned int imx230_real_to_register_gain(float real_gain)
{
    unsigned int reg_gain = 0;

    if (real_gain < IMX230_MIN_AGAIN)
    {
        real_gain = IMX230_MIN_AGAIN;
        SLOW("IMX230 Requested gain(%f) is lower than the sensor minimum(%d)",
            real_gain, IMX230_MIN_AGAIN);
    }
    else if (real_gain > IMX230_MAX_AGAIN)
    {
        real_gain = IMX230_MAX_AGAIN;
    }

    reg_gain = (unsigned int)(512.0 - (512.0 / real_gain));

    return reg_gain;
}

/**
 * FUNCTION: imx230_register_to_real_gain
 *
 * DESCRIPTION: Calcuate sensor real gain value
 **/
static float imx230_register_to_real_gain(unsigned int reg_gain)
{
    float gain;

    if(reg_gain > IMX230_MAX_AGAIN_REG_VAL)
        reg_gain = IMX230_MAX_AGAIN_REG_VAL;

    gain = 512.0 / (512.0 - reg_gain);

    return gain;
}

/**
 * FUNCTION: imx230_digital_gain_calc
 *
 * DESCRIPTION: Calcuate the sensor digital gain
 **/
static unsigned int imx230_digital_gain_calc(
 float real_gain, float sensor_real_gain)
{
    unsigned int reg_dig_gain = IMX230_MIN_DGAIN_REG_VAL;
    float real_dig_gain = IMX230_MIN_DGAIN;

    if(real_gain > IMX230_MAX_AGAIN)
    {
        real_dig_gain = real_gain / sensor_real_gain;
    }
    else
    {
        real_dig_gain = IMX230_MIN_DGAIN;
    }

    if(real_dig_gain > IMX230_MAX_DGAIN)
    {
        real_dig_gain = IMX230_MAX_DGAIN;
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

    exp_info->reg_gain = imx230_real_to_register_gain(real_gain);
    exp_info->sensor_real_gain =
      imx230_register_to_real_gain(exp_info->reg_gain);
    exp_info->sensor_digital_gain =
      imx230_digital_gain_calc(real_gain, exp_info->sensor_real_gain);
    exp_info->sensor_real_dig_gain =
      (float)exp_info->sensor_digital_gain / IMX230_MAX_DGAIN_DECIMATOR;
    exp_info->digital_gain =
      real_gain /(exp_info->sensor_real_gain * exp_info->sensor_real_dig_gain);
    exp_info->line_count = line_count;
    exp_info->s_reg_gain = imx230_real_to_register_gain(s_real_gain);

    return 0;
}

/**
 * FUNCTION: imx230_fill_exposure_array
 *
 * DESCRIPTION: Fill the sensor exposure array
 **/
static int sensor_fill_exposure_array(unsigned int gain,
  unsigned int digital_gain, unsigned int line, unsigned int fl_lines,
  __attribute__((unused)) int luma_avg, unsigned int hdr_param,
  struct camera_i2c_reg_setting* reg_setting,
  __attribute__((unused)) unsigned int s_gain,
  __attribute__((unused)) signed int s_linecount,
  __attribute__((unused)) int hdr_mode)
{
    unsigned short i = 0;
    unsigned short reg_count = 0;
    unsigned int luma_delta = hdr_param & 0x0000FFFF;
    unsigned int hdr_indoor_detected;
    unsigned int ratio = 8;

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

    /* Enable writing direct short exposure for only for  Raw HDR mode*/
    if (hdr_mode == SENSOR_HDR_RAW) {
      reg_setting->reg_setting[reg_count].reg_addr =
          IMX230_SHORT_COARSE_INT_TIME_ADDR;
      reg_setting->reg_setting[reg_count].reg_data=(s_linecount & 0xFF00)>>8;
      reg_setting->reg_setting[reg_count].delay = 0;
      reg_count = reg_count + 1;

      reg_setting->reg_setting[reg_count].reg_addr =
          IMX230_SHORT_COARSE_INT_TIME_ADDR + 1;
      reg_setting->reg_setting[reg_count].reg_data = (s_linecount & 0xFF);
      reg_setting->reg_setting[reg_count].delay = 0;
      reg_count = reg_count + 1;

      reg_setting->reg_setting[reg_count].reg_addr =
          IMX230_SHORT_GAIN_ADDR;
      reg_setting->reg_setting[reg_count].reg_data = (s_gain & 0xFF00) >> 8;
      reg_setting->reg_setting[reg_count].delay = 0;
      reg_count = reg_count + 1;

      reg_setting->reg_setting[reg_count].reg_addr =
          IMX230_SHORT_GAIN_ADDR +  1;
      reg_setting->reg_setting[reg_count].reg_data = (s_gain & 0xFF);
      reg_setting->reg_setting[reg_count].delay = 0;
      reg_count = reg_count + 1;
    }else if (luma_delta != 0){
      hdr_indoor_detected = (hdr_param >> 16) & 0x1;
      ratio = 8;
      if(hdr_indoor_detected)
        ratio = 2;
      else
        ratio = 8;

      reg_setting->reg_setting[reg_count].reg_addr = IMX230_EXP_RATIO_ADDR;
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

    reg_setting->reg_setting[reg_count].reg_addr = IMX230_DIG_GAIN_GR_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX230_DIG_GAIN_GR_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX230_DIG_GAIN_R_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX230_DIG_GAIN_R_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX230_DIG_GAIN_B_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX230_DIG_GAIN_B_ADDR + 1;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX230_DIG_GAIN_GB_ADDR;
    reg_setting->reg_setting[reg_count].reg_data = (digital_gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX230_DIG_GAIN_GB_ADDR + 1;
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

    reg_setting->reg_setting[reg_count].reg_addr = IMX230_ABS_GAIN_R_WORD_ADDR;
    reg_setting->reg_setting[reg_count].reg_data[0] = (awb_gain_r & 0xFF00) >> 8;
    reg_setting->reg_setting[reg_count].reg_data[1] = (awb_gain_r & 0xFF);
    reg_setting->reg_setting[reg_count].reg_data_size = 2;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr = IMX230_ABS_GAIN_B_WORD_ADDR;
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
 * FUNCTION    - parse_VHDR_stats -
 *
 * DESCRIPTION: VHDR statistic parser
 **/
static int parse_VHDR_stats(unsigned int *destLumaBuff, void *rawBuff)
{
  unsigned char       *buf = NULL;
  unsigned int      temp_y = 0;
  int       i;
  unsigned int      count = 0;
  unsigned char       bit_mask = 0;
  unsigned char       bit_shift = 0;
  unsigned char       hdr_indoor = 0;

  if (destLumaBuff == NULL || rawBuff == NULL) {
    SERR("%s: Invalid HDR Stats buffers %p %p!",
      __func__, destLumaBuff, rawBuff);
    return -1;
  }

  buf = (unsigned char *)rawBuff;

  /* HDR stats is limited to 10 bits from sensor for all ratios */
  bit_mask = VIDEO_HDR_LUMA_MASK_RATIO_1;
  bit_shift = VIDEO_HDR_EXPOSURE_RATIO_CORRECTION_1;

  /* Parse the stats only if camera is started and AF is stopped */

  /* Stats data format:
   * A pair of pixel data (10 bit each) is used for transmission of
   * each sub-block 14 bit data.
   * |y(0,0) U |y(0,0) L | y(0,1)U | y(0,1)L| ... | y(15,15)U | y(15,15)L |
   * To get 14 bit y-avg we need to combine Upper and Lower pixel:
   * Upper: D13 - D12 - D11 - D10 - D09 - D08 - D07 - D06 - 0 - 1
   * Lower: D05 - D04 - D03 - D02 - D01 - D00 - 0   -   1 - 0 - 1
   * 14-bit final data:
   * D13 - D12 - D11 - D10 - D09 - D8 - D7 - D6 - D5 - D4 - D3 - D2 - D1 -D0
   **/

  /* We have 16x16 blocks
   * Each block is 20 bits, so 256 blocks * 20 bits is 5120 bits = 640 bytes.
   * We will process 5 bytes (2 luma values) on each iteration, so we need
   * 128 iterations
   **/
  for (i = 0; i < 128; i++) {
    /* Each pixel is 10 bits. And each stat data has pair of
     * pixel - 20 bits. So for each Y we have 2 and a half bytes */

    /* take only the bits specified by the mask */
    temp_y = buf[0] & bit_mask;
    temp_y = temp_y << 6;
    temp_y |= (buf[1] >> 2);
    SLOW("%s: Value obtained for block %d: %d", __func__, i*2, temp_y);
    /* Shift at least 2 bits to make 10bit data to 8bit,
     * as AEC algorithm only takes 8bit data */
    destLumaBuff[count++] = temp_y >> bit_shift;
    /* Now calculate the next pair */
    temp_y = 0;

    temp_y = buf[2] & bit_mask;
    temp_y = temp_y << 6;
    temp_y |= (buf[3] >> 2);
    SLOW("%s: Value obtained for block %d: %d", __func__, i*2+1, temp_y);
    /* Shift at least 2 bits to make 10bit data to 8bit,
     * as AEC algorithm only takes 8bit data */
    destLumaBuff[count++] = temp_y >> bit_shift;
    /* now advance the buf pointer */
    buf += 5;
  }
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
