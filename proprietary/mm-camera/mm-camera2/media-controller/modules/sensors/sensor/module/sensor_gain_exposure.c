/* sensor_gain_exposure.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "sensor_gain_exposure.h"
#include "sensor.h"
#include "modules.h"
#include "camera_dbg.h"

/** real_to_register_gain_smia:
 *
 *  @sensor_lib_t: sensor library
 *  @real_gain: real gain
 *
 *  Compute real to register gain
 *  Return: real gain  **/
static uint32_t real_to_register_gain_smia(sensor_lib_t *lib_ptr,
  float real_gain) {
   uint32_t reg_gain = 0;
   analaog_gain_map_coeff *gain_coeff = &(lib_ptr->aec_info.smia_type_gain_coeff);

  if (real_gain < lib_ptr->aec_info.min_analog_gain)
    real_gain = lib_ptr->aec_info.min_analog_gain;
  else if (real_gain > lib_ptr->aec_info.max_analog_gain)
    real_gain = lib_ptr->aec_info.max_analog_gain;

  reg_gain = (uint32_t)(((gain_coeff->c0 - (gain_coeff->c1 * real_gain)) /
    ((gain_coeff->m1 * real_gain) - gain_coeff->m0)));

  return reg_gain;
}

/** register_to_real_gain_smia:
 *
 *  @sensor_lib_t: sensor library
 *  @reg_gain: register gain
 *
 *  Compute register to real gain
 *  Return: real gain  **/
static float register_to_real_gain_smia(sensor_lib_t *lib_ptr,
  uint32_t reg_gain) {
  float gain;
  analaog_gain_map_coeff *gain_coeff = &(lib_ptr->aec_info.smia_type_gain_coeff);

  gain = ((gain_coeff->m0 * reg_gain) + gain_coeff->c0) /
    (float)((gain_coeff->m1 * reg_gain) + gain_coeff->c1);

  return gain;
}

/** digital_gain_calc_smia:
 *
 *  @sensor_lib_t: sensor library
 *  @real_gain: Overall Real gain
 *  @sensor_real_gain: sensor real gain
 *
 *  Calculate real, reg, digital gain for smia type
 *  Return: reg dig gain  **/
static uint32_t digital_gain_calc_smia(sensor_lib_t *lib_ptr,
 float real_gain, float sensor_real_gain)
{
  uint32_t reg_dig_gain;
  float real_dig_gain = lib_ptr->aec_info.min_digital_gain;

  if(real_gain > lib_ptr->aec_info.max_analog_gain)
    real_dig_gain = real_gain / sensor_real_gain;

  if(real_dig_gain > lib_ptr->aec_info.max_digital_gain)
    real_dig_gain = lib_ptr->aec_info.max_digital_gain;

  reg_dig_gain = (uint32_t)(real_dig_gain *
    lib_ptr->aec_info.dig_gain_decimator);

  return reg_dig_gain;
}

/** sensor_cmn_calc_exposure:
 *
 *  @sensor_lib_t: sensor library
 *  @real_gain: Real gain
 *  @line_count: line count
 *  @s_real_gain: Short real gain
 *  @hdr_param: hdr params
 *  @s_gain: short gain, used for dual exposure
 *
 *  Calculate real, reg, digital gain for smia type
 **/
void sensor_cmn_calc_exposure_smia(sensor_lib_t *lib_ptr, float real_gain,
  uint32_t line_count, sensor_exposure_info_t *exp_info, float s_real_gain)
{
  exp_info->reg_gain = real_to_register_gain_smia(lib_ptr, real_gain);
  exp_info->sensor_real_gain =
    register_to_real_gain_smia(lib_ptr, exp_info->reg_gain);
  exp_info->sensor_digital_gain =
    digital_gain_calc_smia(lib_ptr, real_gain, exp_info->sensor_real_gain);
  exp_info->sensor_real_dig_gain =
    (float)exp_info->sensor_digital_gain / lib_ptr->aec_info.dig_gain_decimator;
  exp_info->digital_gain =
    real_gain /(exp_info->sensor_real_gain * exp_info->sensor_real_dig_gain);
  exp_info->line_count = line_count;
  exp_info->s_reg_gain = real_to_register_gain_smia(lib_ptr, s_real_gain);
  return;
}

/** sensor_cmn_calc_exposure:
 *
 *  @sensor_lib_t: sensor library
 *  @real_gain: Real gain
 *  @line_count: line count
 *  @s_real_gain: Short real gain
 *  @hdr_param: hdr params
 *  @s_gain: short gain, which is needed for dual exposure
 *
 *  This function computes real, register, digital gain
 *  Return: -1 in case of failure, else return 0**/
int32_t sensor_cmn_calc_exposure(sensor_lib_t *lib_ptr, float real_gain,
  uint32_t line_count, sensor_exposure_info_t *exp_info, float s_real_gain)
{
  int32_t rc = 0;
  switch(lib_ptr->exposure_func_table.calc_exp_array_type) {
    case CALC_SMIA_TYPE_METHOD:
      sensor_cmn_calc_exposure_smia(lib_ptr, real_gain, line_count,
        exp_info, s_real_gain);
      break;
    case CALC_CUSTOM_IN_CMN_LIB:
      break;
    default:
      rc =  -1;
      break;
  }
  return rc;
}

/** sensor_fill_exposure_array_3b_int_time:
 *
 *  @sensor_lib_t: sensor library
 *  @gain: Real gain
 *  @digital_gain: Digital gain
 *  @line: line count
 *  @luma_avg: Average luma
 *  @hdr_param: hdr params
 *  @reg_setting: structure in which gain and expo
 *                 related params are updated
 *  @s_gain: short gain, which is needed for dual exposure
 *  @s_linecount: short line count, for dual exposure
 *  @is_hdr_enabled: HDR enable flag
 *
 *  This function fills the gain and exposure details to
 *  the reg_setting structure, where gain, exposure sizes
 *  are of 16bits each.
 *  Return: -1 in case of failure, else return 0**/
static int32_t sensor_fill_exposure_array_3b_int_time( sensor_lib_t *lib_ptr,
  uint32_t gain, uint32_t digital_gain, uint32_t line,
  uint32_t fl_lines,
  __attribute__((unused)) uint32_t hdr_param,
  struct camera_i2c_reg_setting* reg_setting,
  __attribute__((unused)) uint32_t s_gain,
  __attribute__((unused)) int32_t s_linecount,
  __attribute__((unused)) int32_t is_hdr_enabled)
{
  int16_t                          i = 0;
  uint16_t                 reg_count = 0;
  uint32_t          digital_gain_msb = (digital_gain & 0xFF00) >> 8;
  uint32_t          digital_gain_lsb = digital_gain & 0xFF;

  if (!reg_setting)
  {
    return -1;
  }

  for (i = 0; i < lib_ptr->groupon_settings.size; i++) {
    reg_setting->reg_setting[reg_count].reg_addr =
      lib_ptr->groupon_settings.reg_setting_a[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      lib_ptr->groupon_settings.reg_setting_a[i].reg_data;
    reg_count = reg_count + 1;
  }

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->output_reg_addr.frame_length_lines;
  reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF00) >> 8;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->output_reg_addr.frame_length_lines + 1;
  reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF);
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.coarse_int_time_addr - 1;
  reg_setting->reg_setting[reg_count].reg_data = line >> 8;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.coarse_int_time_addr;
  reg_setting->reg_setting[reg_count].reg_data = line >> 4;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.coarse_int_time_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = line << 4;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.global_gain_addr;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF00) >> 8;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.global_gain_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF);
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_gr_addr;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_msb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_gr_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_lsb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_r_addr;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_msb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_r_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_lsb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_b_addr;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_msb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_b_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_lsb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_gb_addr;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_msb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_gb_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_lsb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  for (i = 0; i < lib_ptr->groupoff_settings.size; i++) {
    reg_setting->reg_setting[reg_count].reg_addr =
      lib_ptr->groupoff_settings.reg_setting_a[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      lib_ptr->groupoff_settings.reg_setting_a[i].reg_data;
    reg_count = reg_count + 1;
  }

  for (i = 0; i <= reg_count; i++ ) {
    SLOW("Addr: 0x%X, Data: 0x%X", reg_setting->reg_setting[i].reg_addr,
      reg_setting->reg_setting[i].reg_data);
  }

  reg_setting->size = reg_count;
  reg_setting->addr_type = CAMERA_I2C_WORD_ADDR;
  reg_setting->data_type = CAMERA_I2C_BYTE_DATA;
  reg_setting->delay = 0;

    return 0;
}

/** sensor_fill_exposure_array_2b_int_time:
 *
 *  @sensor_lib_t: sensor library
 *  @gain: Real gain
 *  @digital_gain: Digital gain
 *  @line: line count
 *  @luma_avg: Average luma
 *  @hdr_param: hdr params
 *  @reg_setting: structure in which gain and expo
 *                 related params are updated
 *  @s_gain: short gain, which is needed for dual exposure
 *  @s_linecount: short line count, for dual exposure
 *  @is_hdr_enabled: HDR enable flag
 *
 *  This function fills the gain and exposure details to
 *  the reg_setting structure, where gain, exposure sizes
 *  are of 16bits each.
 *  Return: -1 in case of failure, else return 0**/
static int32_t sensor_fill_exposure_array_2b_int_time( sensor_lib_t *lib_ptr,
  uint32_t gain, uint32_t digital_gain, uint32_t line,
  uint32_t fl_lines,
  __attribute__((unused)) uint32_t hdr_param,
  struct camera_i2c_reg_setting* reg_setting,
  __attribute__((unused)) uint32_t s_gain,
  __attribute__((unused)) int32_t s_linecount,
  __attribute__((unused)) int32_t is_hdr_enabled)
{
  int16_t                          i = 0;
  uint16_t                 reg_count = 0;
  uint32_t          digital_gain_msb = (digital_gain & 0xFF00) >> 8;
  uint32_t          digital_gain_lsb = digital_gain & 0xFF;

  if (!reg_setting)
  {
    return -1;
  }

  for (i = 0; i < lib_ptr->groupon_settings.size; i++) {
    reg_setting->reg_setting[reg_count].reg_addr =
      lib_ptr->groupon_settings.reg_setting_a[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      lib_ptr->groupon_settings.reg_setting_a[i].reg_data;
    reg_count = reg_count + 1;
  }

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->output_reg_addr.frame_length_lines;
  reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF00) >> 8;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->output_reg_addr.frame_length_lines + 1;
  reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0xFF);
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.coarse_int_time_addr;
  reg_setting->reg_setting[reg_count].reg_data = (line & 0xFF00) >> 8;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.coarse_int_time_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (line & 0xFF);
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.global_gain_addr;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF00) >> 8;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.global_gain_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = (gain & 0xFF);
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_gr_addr;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_msb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_gr_addr	+ 1;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_lsb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_r_addr;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_msb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_r_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_lsb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_b_addr;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_msb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_b_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_lsb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_gb_addr;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_msb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  reg_setting->reg_setting[reg_count].reg_addr =
    lib_ptr->exp_gain_info.dig_gain_gb_addr + 1;
  reg_setting->reg_setting[reg_count].reg_data = digital_gain_lsb;
  reg_setting->reg_setting[reg_count].delay = 0;
  reg_count = reg_count + 1;

  for (i = 0; i < lib_ptr->groupoff_settings.size; i++) {
    reg_setting->reg_setting[reg_count].reg_addr =
      lib_ptr->groupoff_settings.reg_setting_a[i].reg_addr;
    reg_setting->reg_setting[reg_count].reg_data =
      lib_ptr->groupoff_settings.reg_setting_a[i].reg_data;
    reg_count = reg_count + 1;
  }

  for (i = 0; i <= reg_count; i++ ) {
    SLOW("Addr: 0x%X, Data: 0x%X", reg_setting->reg_setting[i].reg_addr,
      reg_setting->reg_setting[i].reg_data);
  }

  reg_setting->size = reg_count;
  reg_setting->addr_type = CAMERA_I2C_WORD_ADDR;
  reg_setting->data_type = CAMERA_I2C_BYTE_DATA;
  reg_setting->delay = 0;

    return 0;
}

/** sensor_cmn_lib_fill_exposure:
 *
 *  @sensor_lib_t: sensor library
 *  @gain: Real gain
 *  @digital_gain: Digital gain
 *  @line: line count
 *  @luma_avg: Average luma
 *  @hdr_param: hdr params
 *  @reg_setting: structure in which gain and expo
 *                 related params are updated
 *  @s_gain: short gain, which is needed for dual exposure
 *  @s_linecount: short line count, for dual exposure
 *  @is_hdr_enabled: HDR enable flag
 *
 *  This function fills the gain and exposure details to
 *  the reg_setting structure
 *  Return: -1 in case of failure, else return 0**/
int32_t sensor_cmn_lib_fill_exposure( sensor_lib_t *lib_ptr,
  uint32_t gain,
  uint32_t digital_gain, uint32_t line, uint32_t fl_lines,
  __attribute__((unused)) int32_t luma_avg, uint32_t hdr_param,
  struct camera_i2c_reg_setting* reg_setting,
  uint32_t s_gain, int32_t s_linecount, int32_t is_hdr_enabled) {

  int32_t rc = 0;
  switch(lib_ptr->exposure_func_table.fill_exp_array_type) {
    case FILL_2B_GAIN_2B_IT_2B_FLL:
      rc = sensor_fill_exposure_array_2b_int_time(lib_ptr, gain,
        digital_gain, line, fl_lines, hdr_param,
        reg_setting, s_gain, s_linecount, is_hdr_enabled);
      break;
    case FILL_2B_GAIN_3B_IT_2B_FLL:
      rc = sensor_fill_exposure_array_3b_int_time(lib_ptr, gain,
        digital_gain, line, fl_lines, hdr_param,
        reg_setting, s_gain, s_linecount, is_hdr_enabled);
      break;
    case FILL_CUSTOM_IN_CMN_LIB:
      break;
    default:
      rc = -1;
      SERR("Not a valid argument");
  }
  return rc;
}
