/* truly_cmb433_eeprom.c
 *
 * Copyright (c) 2016, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "truly_cmb433_eeprom.h"

struct camera_i2c_reg_array g_reg_array[AWB_REG_SIZE_R2A + LSC_REG_SIZE_R2A + REG_SIZE];
struct camera_i2c_reg_setting g_reg_setting;

/** truly_cmb433_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void truly_cmb433_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);
  e_items->is_insensor = TRUE;
  e_items->is_afc = FALSE;
  e_items->is_wbc = FALSE;
  e_items->is_lsc = FALSE;
  e_items->is_dpc = FALSE;
}

/** truly_cmb433_update_awb:
 *
 * Calculate and apply white balance calibration data
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void truly_cmb433_update_awb()
{
  uint16_t R_gain, G_gain, B_gain;
  uint16_t G_gain_R, G_gain_B;
  uint16_t nR_G_gain, nB_G_gain, nG_G_gain;
  uint16_t nBase_gain;

  DEBUG_INFO(": rg_ratio=0x%x,bg_ratio=0x%x", otp_data.rg_ratio,otp_data.bg_ratio);

  nR_G_gain = (RG_RATIO_TYPICAL_VALUE*1000) / otp_data.rg_ratio;
  nB_G_gain = (BG_RATIO_TYPICAL_VALUE*1000) / otp_data.bg_ratio;
  nG_G_gain = 1000;
  if (nR_G_gain < 1000 || nB_G_gain < 1000)
  {
    if (nR_G_gain < nB_G_gain)
      nBase_gain = nR_G_gain;
    else
      nBase_gain = nB_G_gain;
  }
  else
  {
    nBase_gain = nG_G_gain;
  }
  R_gain = 0x400 * nR_G_gain / (nBase_gain);
  B_gain = 0x400 * nB_G_gain / (nBase_gain);
  G_gain = 0x400 * nG_G_gain / (nBase_gain);

  DEBUG_INFO("R_gain=0x%x,G_gain=0x%x,B_gain=0x%x",R_gain,G_gain,B_gain);

  if (R_gain >= 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x5019;
    g_reg_array[g_reg_setting.size].reg_data = R_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x501A;
    g_reg_array[g_reg_setting.size].reg_data = R_gain & 0x00ff;
    g_reg_setting.size++;
  }
  if (G_gain >= 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x501B;
    g_reg_array[g_reg_setting.size].reg_data = G_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x501C;
    g_reg_array[g_reg_setting.size].reg_data = G_gain & 0x00ff;
    g_reg_setting.size++;
  }
  if (B_gain >= 0x400) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x501D;
    g_reg_array[g_reg_setting.size].reg_data = B_gain >> 8;
    g_reg_setting.size++;
    g_reg_array[g_reg_setting.size].reg_addr = 0x501E;
    g_reg_array[g_reg_setting.size].reg_data = B_gain & 0x00ff;
    g_reg_setting.size++;
  }
}

/** truly_cmb433_get_group_index:
 *    @flag: group index register value
 *
 * Get which group is used
 *
 * This function executes in eeprom module context
 *
 * Return: int to indicate group index.
 **/
static int truly_cmb433_get_group_index(uint8_t flag)
{
  int8_t group_index = -1;
  flag = flag & 0xFC;
  if((flag&0xC0) == 0x40){
    group_index = 0;
  }else if((flag&0x30) == 0x10){
    group_index = 1;
  }else{
    group_index = -1;
  }

  return group_index;
}

/** truly_cmb433_read_info:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Read the data structure of product information like product date
 *
 * This function executes in eeprom module context
 *
 * Return: int to indicate read information success or not.
 **/
static int truly_cmb433_read_info(sensor_eeprom_data_t *e_ctrl)
{
  uint8_t flag,reg_val;
  int i = 0, group_index,addr_offset = 0,group_offset=0;
  int r2a_version = -1;

  DEBUG_INFO("Enter");

  group_offset = INFO_GROUP_SIZE_R2A;

  flag = (uint8_t)(e_ctrl->eeprom_params.buffer[0]);
  group_index=truly_cmb433_get_group_index(flag);
  if(group_index==-1){
    ALOGE("%s:invalid or empty opt data",__func__);
    return -1;
  }
  addr_offset = group_offset*group_index;

  otp_data.module_integrator_id =
    (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + 1]);
  otp_data.lens_id =
    (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + 2]);
  otp_data.production_year =
    (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + 3]);
  otp_data.production_month =
    (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + 4]);
  otp_data.production_day =
    (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + 5]);

  return 0;
}

/** truly_cmb433_read_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Read the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: int to indicate read white balance success or not.
 **/
static int truly_cmb433_read_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  uint8_t flag;
  uint8_t temp;
  uint8_t reg_val;
  int addr_offset = -1;
  int group_index  = -1;
  int group_offset = WB_GROUP_SIZE_R2A;

  flag = (uint8_t)(e_ctrl->eeprom_params.buffer[0]);

  group_index=truly_cmb433_get_group_index(flag);
  if(group_index==-1){
    ALOGE("%s:invalid or empty awb data",__func__);
    return -1;
  }

  addr_offset = WB_OFFSET_R2A + group_index * group_offset;

  temp = (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + 2]);
  DEBUG_INFO("temp=0x%x ",temp);

  reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset]);
  DEBUG_INFO("reg_val=0x%x",reg_val);
  otp_data.rg_ratio = (reg_val<<2) + ((temp>>6)&0x03);

  reg_val = (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + 1]);
  DEBUG_INFO("reg_val=0x%x",reg_val);
  otp_data.bg_ratio = (reg_val<<2) + ((temp>>4)&0x03);

  return 0;
}

/** truly_cmb433_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void truly_cmb433_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  DEBUG_INFO("Enter");
  int rc = 0;
  uint8_t flag = 0;
  int r2a_version = 0;

  rc = truly_cmb433_read_wbdata(e_ctrl);

  if(rc < 0)
    ALOGE("read wbdata failed");
  truly_cmb433_update_awb();

  DEBUG_INFO("Exit");
}


/** truly_cmb433_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void truly_cmb433_format_lensshading(sensor_eeprom_data_t *e_ctrl)
{
  int addr_offset = -1;
  int group_index  = -1;
  int group_offset = 0;
  int j = 0;
  uint8_t flag;
  int lens_offset = 0;
  int lens_reg_size = 0;
  int r2a_version = 0;
  int temp;

  lens_offset = LENS_OFFSET_R2A;
  group_offset = LENS_GROUP_SIZE_R2A;
  lens_reg_size = LSC_REG_SIZE_R2A;

  flag = (uint8_t)(e_ctrl->eeprom_params.buffer[lens_offset]);

  group_index=truly_cmb433_get_group_index(flag);
  if(group_index==-1){
    ALOGE("%s:invalid or empty lensshading data",__func__);
    return ;
  }

  temp = (uint8_t)(e_ctrl->eeprom_params.buffer[507]);
  temp = 0x20 | temp;
  g_reg_array[g_reg_setting.size].reg_addr = 0x5000;
  g_reg_array[g_reg_setting.size].reg_data = temp;
  g_reg_setting.size++;

  addr_offset = lens_offset + group_index * group_offset;
  for (j = 0; j < lens_reg_size; j++) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x5900 + j;
    g_reg_array[g_reg_setting.size].reg_data =
      (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + j + 1]);
    g_reg_setting.size++;
    DEBUG_INFO("reg_val=0x%x",
      (uint8_t)(e_ctrl->eeprom_params.buffer[addr_offset + j + 1]));
  }
  DEBUG_INFO("Exit");
}

/** truly_cmb433_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void truly_cmb433_format_calibration_data(void *e_ctrl) {
  DEBUG_INFO("Enter");
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  uint8_t *data = ectrl->eeprom_params.buffer;

  g_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
  g_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;
  g_reg_setting.reg_setting = &g_reg_array[0];
  g_reg_setting.size = 0;
  g_reg_setting.delay = 0;

  truly_cmb433_read_info(ectrl);
  truly_cmb433_format_wbdata(ectrl);
  truly_cmb433_format_lensshading(ectrl);

  DEBUG_INFO("Exit");
}

/** truly_cmb433_get_raw_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *    @data: point to the destination msm_camera_i2c_reg_setting
 *
 * Get the all the calibration registers setting of the sensor
 *
 * This function executes in eeprom module context.
 *
 * Return: void.
 **/
int truly_cmb433_get_raw_data(void *e_ctrl, void *data) {
  if (e_ctrl && data){
    memcpy(data, &g_reg_setting, sizeof(g_reg_setting));
  return 0;
  }else{
    ALOGE("failed Null pointer");
  return -1;
  }
}

/** truly_cmb433_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* truly_cmb433_eeprom_open_lib(void) {
  return &truly_cmb433_lib_func_ptr;
}
