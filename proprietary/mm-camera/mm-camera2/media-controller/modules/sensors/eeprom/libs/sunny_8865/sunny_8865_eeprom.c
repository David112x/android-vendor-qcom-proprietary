/*============================================================================

Copyright (c) 2015 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include <stdio.h>
#include <string.h>
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "eeprom_util.h"
#include "debug_lib.h"
#include "sensor_lib.h"
#include <utils/Log.h>

#define BASE_ADDR 0x7010
#define INFO_GROUP_SIZE 5

#define WB_FLAG_ADDR 0x7020
#define WB_GROUP_SIZE 5
#define AWB_REG_SIZE 5

#define LENS_FLAG_ADDR 0x703A
#define LENS_GROUP_SIZE 62
#define LSC_REG_SIZE 62

#define RG_RATIO_TYPICAL_VALUE 0x12f
#define BG_RATIO_TYPICAL_VALUE 0x116

#define ABS(x)            (((x) < 0) ? -(x) : (x))

struct otp_struct {
  unsigned short flag;// bit[7]:info, bit[6]:wb, bit[5]:vcm, bit[4]:lenc
  unsigned short module_integrator_id;
  unsigned short lens_id;
  unsigned short production_year;
  unsigned short production_month;
  unsigned short production_day;
  unsigned short rg_ratio;
  unsigned short bg_ratio;
  unsigned short light_rg;
  unsigned short light_bg;
  unsigned short lenc[62];
} otp_data;

struct camera_i2c_reg_array g_reg_array[298];
struct camera_i2c_reg_setting g_reg_setting;

/** sunny_8865_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_8865_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);
  e_items->is_insensor = TRUE;
  e_items->is_afc = FALSE;
  e_items->is_wbc = FALSE;
  e_items->is_lsc = FALSE;
  e_items->is_dpc = FALSE;

  SHIGH("is_wbc: %d,is_afc: %d,is_lsc: %d,is_dpc: %d, is_insensor: %d,\
    is_ois: %d",e_items->is_wbc,e_items->is_afc,
    e_items->is_lsc,e_items->is_dpc,e_items->is_insensor,
    e_items->is_ois);
}

/** sunny_8865_get_group_index:
 *    @flag: group index register value
 *
 * Get which group is used
 *
 * This function executes in eeprom module context
 *
 * Return: int to indicate group index.
 **/
static int sunny_8865_get_group_index(uint8_t flag)
{
  int8_t group_index = -1;
  flag = flag & 0xFC;
  if((flag&0xC0) == 0x40)
    group_index = 0;
  else if((flag&0x30) == 0x10)
    group_index = 1;
  else if((flag&0x0C) == 0x04)
    group_index = 2;
  else
    group_index = -1;
  return group_index;
}

static int sunny_8865_read_otp(sensor_eeprom_data_t *e_ctrl)
{
  uint8_t flag;
  uint8_t temp;
  uint8_t reg_val;
  int8_t group_index = -1;
  int16_t start_address = 0;
  int i = 0;

  SDBG("Enter");

  flag = (uint8_t)(e_ctrl->eeprom_params.buffer[0]);
  if((group_index=sunny_8865_get_group_index(flag)) == -1)
  {
    SERR("eeprom %s:invalid or empty opt data",__func__);
    return -1;
  }
  start_address =  (INFO_GROUP_SIZE*group_index) + 1;

  //Read info
  otp_data.flag = 0x80;//valid info in OTP
  otp_data.module_integrator_id =
    (uint8_t)(e_ctrl->eeprom_params.buffer[start_address]);
  otp_data.lens_id =
    (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 1]);
  otp_data.production_year =
    (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 2]);
  otp_data.production_month =
    (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 3]);
  otp_data.production_day =
    (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 4]);

  SLOW("module id = %d lens id =%d",otp_data.module_integrator_id,
    otp_data.lens_id);

  //Read_wbdata
  flag = (uint8_t)(e_ctrl->eeprom_params.buffer[WB_FLAG_ADDR - BASE_ADDR]);

  if((group_index=sunny_8865_get_group_index(flag)) == -1)
    SHIGH("%s:invalid or empty awb data",__func__) ;
  else
  {
    start_address = ( WB_FLAG_ADDR - BASE_ADDR )+ (group_index*WB_GROUP_SIZE) + 1;
    otp_data.flag |= 0x40;
    temp = (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 4]);

    SLOW("temp=0x%x ",temp);

    reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[start_address]);
    SLOW("reg_val=0x%x",reg_val);
    otp_data.rg_ratio = (reg_val<<2) + ((temp>>6)&0x03);

    reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 1]);
    SLOW("reg_val=0x%x",reg_val);
    otp_data.bg_ratio = (reg_val<<2) + ((temp>>4)&0x03);

    reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 2]);
    SLOW("reg_val=0x%x",reg_val);
    otp_data.light_rg = (reg_val<<2) + ((temp>>2)&0x03);

    reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + 3]);
    SLOW("reg_val=0x%x",reg_val);
    otp_data.light_bg =  (reg_val<<2) + (temp&0x03);
  }

  //Read_LSCdata
  flag = (uint8_t)(e_ctrl->eeprom_params.buffer[LENS_FLAG_ADDR - BASE_ADDR]);

  if((group_index=sunny_8865_get_group_index(flag)) == -1)
    SHIGH("%s:invalid or empty LSC data",__func__);
  else
  {
    start_address = (LENS_FLAG_ADDR - BASE_ADDR)+(group_index*LENS_GROUP_SIZE) +1;
    otp_data.flag |= 0x10;

    for(i = 0; i < LSC_REG_SIZE; i++)
      otp_data.lenc[i] = (uint8_t)(e_ctrl->eeprom_params.buffer[start_address + i]);

    //Clear the OTP buffer
    for(i = 0x7010; i < 0x70f4; i++)
    {
      g_reg_array[g_reg_setting.size].reg_addr = i;
      g_reg_array[g_reg_setting.size].reg_data = 0x00;
      g_reg_setting.size++;
    }
  }
  return 0;
}

static int sunny_8865_apply_otp()
{
  uint16_t rg, bg, R_gain, G_gain, B_gain, Base_gain;
  uint16_t temp, i;

  if(otp_data.flag & 0x40)
  {
    if (otp_data.light_rg)
      rg = otp_data.rg_ratio * (otp_data.light_rg + 512) / 1024;
    else
      rg = otp_data.rg_ratio;

    if (otp_data.light_bg)
      bg = otp_data.bg_ratio * (otp_data.light_bg + 512) / 1024;
    else
      bg = otp_data.bg_ratio;

    SLOW("rg_ratio=0x%x,bg_ratio=0x%x,light_rg=0x%x,light_bg=0x%x",
    otp_data.rg_ratio,otp_data.bg_ratio,otp_data.light_rg,otp_data.light_bg);

    R_gain = (RG_RATIO_TYPICAL_VALUE*1000) / rg;
    B_gain = (BG_RATIO_TYPICAL_VALUE*1000) / bg;
    G_gain = 1000;

    if (R_gain < 1000 || B_gain < 1000)
    {
      if (R_gain < B_gain)
        Base_gain = R_gain;
      else
        Base_gain = B_gain;
    }
    else
    {
       Base_gain = G_gain;
    }
    R_gain = 0x400 * R_gain / (Base_gain);
    B_gain = 0x400 * B_gain / (Base_gain);
    G_gain = 0x400 * G_gain / (Base_gain);

    SLOW("R_gain=0x%x,G_gain=0x%x,B_gain=0x%x",R_gain,G_gain,B_gain);

    if (R_gain > 0x400)
    {
      g_reg_array[g_reg_setting.size].reg_addr = 0x5018;
      g_reg_array[g_reg_setting.size].reg_data = R_gain >> 6;
      g_reg_setting.size++;
      g_reg_array[g_reg_setting.size].reg_addr = 0x5019;
      g_reg_array[g_reg_setting.size].reg_data = R_gain & 0x003f;
      g_reg_setting.size++;
    }
    if (G_gain > 0x400)
    {
      g_reg_array[g_reg_setting.size].reg_addr = 0x503A;
      g_reg_array[g_reg_setting.size].reg_data = G_gain >> 6;
      g_reg_setting.size++;
      g_reg_array[g_reg_setting.size].reg_addr = 0x501B;
      g_reg_array[g_reg_setting.size].reg_data = G_gain & 0x003f;
      g_reg_setting.size++;
    }
    if (B_gain > 0x400)
    {
      g_reg_array[g_reg_setting.size].reg_addr = 0x501C;
      g_reg_array[g_reg_setting.size].reg_data = B_gain >> 6;
      g_reg_setting.size++;
      g_reg_array[g_reg_setting.size].reg_addr = 0x501D;
      g_reg_array[g_reg_setting.size].reg_data = B_gain & 0x003f;
      g_reg_setting.size++;
    }

    //apply OTP lens calib
    if (otp_data.flag & 0x10)
    {
      //read from init  (0x5000) which is equals to 0x96
      temp = 0x80 | 0x96;
      g_reg_array[g_reg_setting.size].reg_addr = 0x5000;
      g_reg_array[g_reg_setting.size].reg_data = temp;
      g_reg_setting.size++;

      for(i = 0; i < LSC_REG_SIZE; i++)
      {
        g_reg_array[g_reg_setting.size].reg_addr = 0x5800 + i;
        g_reg_array[g_reg_setting.size].reg_data = otp_data.lenc[i];
        g_reg_setting.size++;
      }
    }
  }
  return 0;
}


/** sunny_8865_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void sunny_8865_format_calibration_data(void *e_ctrl)
{
  uint8_t flag = 0;
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  uint8_t *data = ectrl->eeprom_params.buffer;

  g_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
  g_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;
  g_reg_setting.reg_setting = &g_reg_array[0];
  g_reg_setting.size = 0;
  g_reg_setting.delay = 0;
  sunny_8865_read_otp(ectrl);
  sunny_8865_apply_otp();
  SDBG("Exit");
}

/** sunny_8865_get_raw_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *    @data: point to the destination msm_camera_i2c_reg_setting
 *
 * Get the all the calibration registers setting of the sensor
 *
 * This function executes in eeprom module context.
 *
 * Return: void.
 **/
int sunny_8865_get_raw_data(void *e_ctrl, void *data)
{
  RETURN_ERR_ON_NULL(e_ctrl, SENSOR_FAILURE);
  RETURN_ERR_ON_NULL(data, SENSOR_FAILURE);

  memcpy(data, &g_reg_setting, sizeof(g_reg_setting));

  return 0;
}

static eeprom_lib_func_t sunny_8865_lib_func_ptr = {
  .get_calibration_items = sunny_8865_get_calibration_items,
  .format_calibration_data = sunny_8865_format_calibration_data,
  .do_af_calibration = NULL,
  .do_wbc_calibration = NULL,
  .do_lsc_calibration = NULL,
  .get_raw_data = sunny_8865_get_raw_data,
  .get_ois_raw_data = NULL,
};

/** sunny_8865_eeprom_open_lib:
 *
 * Get the function pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* sunny_8865_eeprom_open_lib(void)
{
  return &sunny_8865_lib_func_ptr;
}
