/* sensor_sdk_utils.c
 *
 * Copyright (c) 2015-2019 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SENSOR_SDK_UTILS_H__
#define __SENSOR_SDK_UTILS_H__

#include <string.h>
#include "sensor_sdk_utils.h"
#include "sensor_sdk_common.h"
#include "sensor_common.h"

enum msm_camera_i2c_reg_addr_type sensor_sdk_util_get_kernel_i2c_addr_type(
  enum camera_i2c_reg_addr_type addr_type)
{
  switch (addr_type) {
  case CAMERA_I2C_BYTE_ADDR:
    return MSM_CAMERA_I2C_BYTE_ADDR;
  case CAMERA_I2C_WORD_ADDR:
    return MSM_CAMERA_I2C_WORD_ADDR;
  case CAMERA_I2C_3B_ADDR:
    return MSM_CAMERA_I2C_3B_ADDR;
  case CAMERA_I2C_DWORD_ADDR:
    return MSM_CAMERA_I2C_DWORD_ADDR;
  default:
    /*Sometimes it can enter default if the settings does not need to mention
          addr_type: ex: actuator reg_params and reg_write_type as
          ACTUATOR_WRITE_DAC*/
    SHIGH("Invalid addr_type = %d", addr_type);
    return MSM_CAMERA_I2C_ADDR_TYPE_MAX;
  }
}

enum msm_camera_i2c_data_type sensor_sdk_util_get_kernel_i2c_data_type(
  enum camera_i2c_data_type data_type)
{
  switch (data_type) {
  case CAMERA_I2C_BYTE_DATA:
    return MSM_CAMERA_I2C_BYTE_DATA;
  case CAMERA_I2C_WORD_DATA:
    return MSM_CAMERA_I2C_WORD_DATA;
  case CAMERA_I2C_DWORD_DATA:
    return MSM_CAMERA_I2C_DWORD_DATA;
  default:
    /*Sometimes it can enter default if the settings does not need to mention
          addr_type: ex: actuator reg_params and reg_write_type as
          ACTUATOR_WRITE_DAC*/
    SHIGH("Invalid data_type = %d", data_type);
    return MSM_CAMERA_I2C_DATA_TYPE_MAX;
  }
}

enum msm_sensor_power_seq_type_t sensor_sdk_util_get_kernel_power_seq_type(
  enum camera_power_seq_type seq_type)
{

  switch (seq_type) {
  case CAMERA_POW_SEQ_CLK:
    return SENSOR_CLK;
    break;
  case CAMERA_POW_SEQ_GPIO:
    return SENSOR_GPIO;
    break;
  case CAMERA_POW_SEQ_VREG:
    return SENSOR_VREG;
    break;
  case CAMERA_POW_SEQ_I2C:
    return SENSOR_I2C;
    break;
  default:
    SERR("Invalid seq_type = %d", seq_type);
    return -1;
  }
}

void translate_eeprom_memory_map(
  struct msm_eeprom_memory_map_array *eeprom_map_arr,
  struct eeprom_memory_map_array *memory_map_arr)
{
  uint32_t i = 0,j;

  eeprom_map_arr->msm_size_of_max_mappings =
    memory_map_arr->size_map_array;
  for (j = 0; j < memory_map_arr->size_map_array; j++)
  {
    eeprom_map_arr->memory_map[j].slave_addr =
      memory_map_arr->memory_map[j].slave_addr;
    eeprom_map_arr->memory_map[j].memory_map_size =
      memory_map_arr->memory_map[j].memory_map_size;

    for(i = 0; i < memory_map_arr->memory_map[j].memory_map_size; i++) {
      switch (memory_map_arr->memory_map[j].mem_settings[i].i2c_operation){
      case CAMERA_I2C_OP_WRITE:
        eeprom_map_arr->memory_map[j].mem_settings[i].i2c_operation =
          MSM_CAM_WRITE;
        break;
      case CAMERA_I2C_OP_POLL:
        eeprom_map_arr->memory_map[j].mem_settings[i].i2c_operation =
          MSM_CAM_POLL;
        break;
      case CAMERA_I2C_OP_READ:
        eeprom_map_arr->memory_map[j].mem_settings[i].i2c_operation =
          MSM_CAM_READ;
        break;
      default:
        SERR("Invalid i2c_operation = %d",
          memory_map_arr->memory_map[j].mem_settings[i].i2c_operation);
        return;
      }

      eeprom_map_arr->memory_map[j].mem_settings[i].data_type =
        sensor_sdk_util_get_kernel_i2c_data_type(memory_map_arr->memory_map[j].
        mem_settings[i].data_type);
      eeprom_map_arr->memory_map[j].mem_settings[i].addr_type =
        sensor_sdk_util_get_kernel_i2c_addr_type(memory_map_arr->memory_map[j].
        mem_settings[i].addr_type);
      eeprom_map_arr->memory_map[j].mem_settings[i].reg_addr =
        memory_map_arr->memory_map[j].mem_settings[i].reg_addr;
      eeprom_map_arr->memory_map[j].mem_settings[i].reg_data =
        memory_map_arr->memory_map[j].mem_settings[i].reg_data;
      eeprom_map_arr->memory_map[j].mem_settings[i].delay =
        memory_map_arr->memory_map[j].mem_settings[i].delay;
    }
  }
}

void translate_actuator_damping_param(
  struct damping_params_t *ringing_params_k,
  struct actuator_damping_params_t *ringing_params_u,
  uint32_t array_size)
{
  uint32_t i = 0;

  RETURN_VOID_ON_NULL(ringing_params_k);
  RETURN_VOID_ON_NULL(ringing_params_u);

  for (i = 0;i < array_size; i++) {
    ringing_params_k[i].damping_step = ringing_params_u[i].damping_step;
    ringing_params_k[i].damping_delay = ringing_params_u[i].damping_delay;
    ringing_params_k[i].hw_params = ringing_params_u[i].hw_params;
  }
}

void translate_actuator_reg_params(
  struct msm_actuator_reg_params_t *reg_params_k,
  struct actuator_reg_params_t *reg_params_u,
  uint32_t array_size)
{
  uint32_t i = 0;

  for (i = 0;i < array_size; i++) {
    switch(reg_params_u[i].reg_write_type) {
    case ACTUATOR_WRITE_HW_DAMP:
      reg_params_k[i].reg_write_type = MSM_ACTUATOR_WRITE_HW_DAMP;
      break;
    case ACTUATOR_WRITE_DAC:
      reg_params_k[i].reg_write_type = MSM_ACTUATOR_WRITE_DAC;
      break;
    case ACTUATOR_WRITE:
      reg_params_k[i].reg_write_type = MSM_ACTUATOR_WRITE;
      break;
    case ACTUATOR_WRITE_DIR_REG:
      reg_params_k[i].reg_write_type = MSM_ACTUATOR_WRITE_DIR_REG;
      break;
    case ACTUATOR_POLL:
      reg_params_k[i].reg_write_type = MSM_ACTUATOR_POLL;
      break;
    case ACTUATOR_READ_WRITE:
      reg_params_k[i].reg_write_type = MSM_ACTUATOR_READ_WRITE;
      break;
    default:
      SERR("Invalid reg_write_type = %d", reg_params_u[i].reg_write_type);
      return;
    }
    reg_params_k[i].hw_mask = reg_params_u[i].hw_mask;
    reg_params_k[i].reg_addr = reg_params_u[i].reg_addr;
    reg_params_k[i].hw_shift = reg_params_u[i].hw_shift;
    reg_params_k[i].data_shift = reg_params_u[i].data_shift;
    reg_params_k[i].data_type =
      sensor_sdk_util_get_kernel_i2c_data_type(reg_params_u[i].data_type);
    reg_params_k[i].addr_type =
      sensor_sdk_util_get_kernel_i2c_addr_type(reg_params_u[i].addr_type);
    reg_params_k[i].reg_data = reg_params_u[i].reg_data;
    reg_params_k[i].delay = reg_params_u[i].delay;
  }
}

void translate_actuator_reg_settings(
  struct reg_settings_t *reg_settings_k,
  struct actuator_reg_settings_t *reg_settings_u,
  uint32_t array_size)
{
  uint32_t i = 0;

  for (i = 0;i < array_size; i++) {

    switch (reg_settings_u[i].i2c_operation){
    case ACTUATOR_I2C_OP_WRITE:
      reg_settings_k[i].i2c_operation = MSM_ACT_WRITE;
      break;
    case ACTUATOR_I2C_OP_POLL:
      reg_settings_k[i].i2c_operation = MSM_ACT_POLL;
      break;
    default:
      SERR("Invalid i2c_operation = %d", reg_settings_u[i].i2c_operation);
      return;
    }

    reg_settings_k[i].data_type = sensor_sdk_util_get_kernel_i2c_data_type(
      reg_settings_u[i].data_type);
    reg_settings_k[i].addr_type = sensor_sdk_util_get_kernel_i2c_addr_type(
      reg_settings_u[i].addr_type);
    reg_settings_k[i].reg_addr = reg_settings_u[i].reg_addr;
    reg_settings_k[i].reg_data = reg_settings_u[i].reg_data;
    reg_settings_k[i].delay = reg_settings_u[i].delay;
  }
}

void translate_actuator_region_params(
  struct region_params_t *region_params_k,
  struct actuator_region_params_t *region_params_u,
  uint32_t array_size)
{
  uint32_t i = 0;

  for (i = 0;i < array_size; i++) {
    region_params_k[i].code_per_step = region_params_u[i].code_per_step;
    region_params_k[i].qvalue = region_params_u[i].qvalue;
    region_params_k[i].step_bound[0] = region_params_u[i].step_bound[0];
    region_params_k[i].step_bound[1] = region_params_u[i].step_bound[1];
  }
}

void translate_sensor_csiphy_params(
  struct msm_camera_csiphy_params *csiphy_params_k,
  struct sensor_csi_params *csiphy_params_u,
  camera_csi_params_t *camera_csi_params,
  uint64_t data_rate)
{
  /* Copy form camera config struct */
  csiphy_params_k->csid_core = camera_csi_params->csid_core;
  csiphy_params_k->lane_mask = camera_csi_params->lane_mask;
  csiphy_params_k->combo_mode = camera_csi_params->combo_mode;

  csiphy_params_k->lane_cnt = csiphy_params_u->lane_cnt;
  csiphy_params_k->settle_cnt = csiphy_params_u->settle_cnt;
  csiphy_params_k->csi_3phase = csiphy_params_u->is_csi_3phase;

  csiphy_params_k->data_rate = data_rate;
}

void translate_sensor_csid_params(
  struct msm_camera_csid_params *csid_params_k,
  struct sensor_csi_params *csid_params_u,
  struct msm_camera_csid_vc_cfg *csid_vc_cfg,
  struct sensor_csid_lut_params *lut_params_u,
  camera_csi_params_t *camera_csi_params)
{
  uint32_t i, j;


  /* Copy form camera config struct */
  csid_params_k->lane_assign = camera_csi_params->lane_assign;

  csid_params_k->lane_cnt = csid_params_u->lane_cnt;
  csid_params_k->lut_params.num_cid = 0;
  csid_params_k->csi_3p_sel = csid_params_u->is_csi_3phase;
#ifdef SECURE_CAMERA
  csid_params_k->is_secure = csid_params_u->is_secure;
#endif

  for (i = 0, j = 0; i < lut_params_u->num_cid; i++) {
    if (lut_params_u->vc_cfg_a[i].dt != 0)
    {
     csid_vc_cfg[j].cid = lut_params_u->vc_cfg_a[i].cid;
     csid_vc_cfg[j].dt = lut_params_u->vc_cfg_a[i].dt;
     csid_vc_cfg[j].decode_format = lut_params_u->vc_cfg_a[i].decode_format;
     csid_params_k->lut_params.vc_cfg[j] = csid_vc_cfg + i;
     csid_params_k->lut_params.num_cid ++;
     j++;
    }
  }
}

void translate_sensor_csid_test_params(
  struct msm_camera_csid_testmode_parms *csid_params_k,
  struct sensor_csid_testmode_parms *csid_params_u)
{
  csid_params_k->num_bytes_per_line = csid_params_u->num_bytes_per_line;
  csid_params_k->num_lines = csid_params_u->num_lines;
  csid_params_k->h_blanking_count = csid_params_u->h_blanking_count;
  csid_params_k->v_blanking_count = csid_params_u->v_blanking_count;
  csid_params_k->payload_mode = csid_params_u->payload_mode;
}

enum i2c_freq_mode_t sensor_sdk_util_get_i2c_freq_mode(
  enum camera_i2c_freq_mode i2c_freq_mode)
{
  switch (i2c_freq_mode){
  case SENSOR_I2C_MODE_STANDARD:
    return I2C_STANDARD_MODE;
  case SENSOR_I2C_MODE_FAST:
    return I2C_FAST_MODE;
  case SENSOR_I2C_MODE_CUSTOM:
    return I2C_CUSTOM_MODE;
  case SENSOR_I2C_MODE_FAST_PLUS:
    return I2C_FAST_PLUS_MODE;
  default:
    SINFO("Invalid i2c_freq_mode = %d", i2c_freq_mode);
    return I2C_MAX_MODES;
  }
}

void translate_sensor_slave_info(
  struct msm_camera_sensor_slave_info *slave_info_k,
  struct camera_sensor_slave_info *slave_info_u,
  camera_module_config_t *camera_cfg,
  struct msm_sensor_power_setting *power_up_setting_k,
  struct msm_sensor_power_setting *power_down_setting_k)
{
  uint16_t             i = 0;
  enum i2c_freq_mode_t mode;
  strlcpy(slave_info_k->sensor_name, camera_cfg->sensor_name,
    sizeof(slave_info_k->sensor_name));
  strlcpy(slave_info_k->eeprom_name, camera_cfg->eeprom_name,
    sizeof(slave_info_k->eeprom_name));
  strlcpy(slave_info_k->actuator_name, camera_cfg->actuator_name,
    sizeof(slave_info_k->actuator_name));
  strlcpy(slave_info_k->ois_name, camera_cfg->ois_name,
    sizeof(slave_info_k->ois_name));
  strlcpy(slave_info_k->flash_name, camera_cfg->flash_name,
    sizeof(slave_info_k->flash_name));

  switch (camera_cfg->camera_id) {
  case 0:
    slave_info_k->camera_id = CAMERA_0;
    break;
  case 1:
    slave_info_k->camera_id = CAMERA_1;
    break;
  case 2:
    slave_info_k->camera_id = CAMERA_2;
    break;
  case 3:
    slave_info_k->camera_id = CAMERA_3;
    break;
  default:
    SERR("Invalid camera_id = %d", camera_cfg->camera_id);
    return;
  }

  mode = sensor_sdk_util_get_i2c_freq_mode(camera_cfg->i2c_freq_mode);
  if (mode == I2C_MAX_MODES) {
    SLOW("No special I2C Frequency Mode, use default value");
    slave_info_k->i2c_freq_mode =
      sensor_sdk_util_get_i2c_freq_mode(slave_info_u->i2c_freq_mode);
  } else
    slave_info_k->i2c_freq_mode = mode;

  if (camera_cfg->sensor_slave_addr == 0x00) {
    SLOW("No special slave address configure, use default slave addr");
    slave_info_k->slave_addr = slave_info_u->slave_addr;
  }else
    slave_info_k->slave_addr = camera_cfg->sensor_slave_addr;

  slave_info_k->addr_type =
    sensor_sdk_util_get_kernel_i2c_addr_type(slave_info_u->addr_type);

  slave_info_k->sensor_id_info.sensor_id_reg_addr =
    slave_info_u->sensor_id_info.sensor_id_reg_addr;
  slave_info_k->sensor_id_info.sensor_id =
    slave_info_u->sensor_id_info.sensor_id;
  slave_info_k->sensor_id_info.sensor_id_mask =
    slave_info_u->sensor_id_info.sensor_id_mask;

#ifdef SENSOR_PROBE_WRITE
  slave_info_k->sensor_id_info.setting.addr_type =
    sensor_sdk_util_get_kernel_i2c_addr_type(
    slave_info_u->sensor_id_info.settings.addr_type);

  slave_info_k->sensor_id_info.setting.data_type =
    sensor_sdk_util_get_kernel_i2c_data_type(
    slave_info_u->sensor_id_info.settings.data_type);

  slave_info_k->sensor_id_info.setting.delay =
    slave_info_u->sensor_id_info.settings.delay;

  slave_info_k->sensor_id_info.setting.size =
    slave_info_u->sensor_id_info.settings.size;

  slave_info_k->sensor_id_info.setting.reg_setting = (struct msm_camera_i2c_reg_array *)
    &(slave_info_u->sensor_id_info.settings.reg_setting_a[0]);
#endif

  slave_info_k->power_setting_array.power_setting = power_up_setting_k;
  slave_info_k->power_setting_array.size =
    slave_info_u->power_setting_array.size;
  slave_info_k->power_setting_array.power_down_setting = power_down_setting_k;
  slave_info_k->power_setting_array.size_down =
    slave_info_u->power_setting_array.size_down;

  for (i = 0; i < slave_info_k->power_setting_array.size; i++) {
    power_up_setting_k[i].seq_type =
      sensor_sdk_util_get_kernel_power_seq_type(
      slave_info_u->power_setting_array.power_setting_a[i].seq_type);
    power_up_setting_k[i].seq_val =
      slave_info_u->power_setting_array.power_setting_a[i].seq_val;
    power_up_setting_k[i].config_val =
      slave_info_u->power_setting_array.power_setting_a[i].config_val;
    power_up_setting_k[i].delay =
      slave_info_u->power_setting_array.power_setting_a[i].delay;
  }

  for (i = 0; i < slave_info_k->power_setting_array.size_down; i++) {
    power_down_setting_k[i].seq_type =
      sensor_sdk_util_get_kernel_power_seq_type(
      slave_info_u->power_setting_array.power_down_setting_a[i].seq_type);
    power_down_setting_k[i].seq_val =
      slave_info_u->power_setting_array.power_down_setting_a[i].seq_val;
    power_down_setting_k[i].config_val =
      slave_info_u->power_setting_array.power_down_setting_a[i].config_val;
    power_down_setting_k[i].delay =
      slave_info_u->power_setting_array.power_down_setting_a[i].delay;
  }

  slave_info_k->is_init_params_valid = slave_info_u->is_init_params_valid;

  switch (camera_cfg->position){
  case CAM_POSITION_BACK:
    slave_info_k->sensor_init_params.position = BACK_CAMERA_B;
    break;
  case CAM_POSITION_FRONT:
    slave_info_k->sensor_init_params.position = FRONT_CAMERA_B;
    break;
  case CAM_POSITION_BACK_AUX:
    slave_info_k->sensor_init_params.position = AUX_CAMERA_B;
    break;
  default:
    SERR("Invalid position = %d", camera_cfg->position);
    return;
  }

  switch (camera_cfg->modes_supported){
  case 1:
    slave_info_k->sensor_init_params.modes_supported = CAMSENSOR_MODE_2D;
    break;
  case 2:
    slave_info_k->sensor_init_params.modes_supported = CAMSENSOR_MODE_3D;
    break;
  case -1:
    slave_info_k->sensor_init_params.modes_supported = CAMSENSOR_MODE_INVALID;
    break;
  default:
    SERR("Invalid position = %d", camera_cfg->modes_supported);
    return;
  }

  slave_info_k->sensor_init_params.sensor_mount_angle =
    camera_cfg->sensor_mount_angle;
}

void translate_sensor_reg_setting(
  struct msm_camera_i2c_reg_setting *setting_k,
  struct camera_i2c_reg_setting *setting_u)
{
  setting_k->reg_setting =
    (struct msm_camera_i2c_reg_array *)setting_u->reg_setting;
  setting_k->size = setting_u->size;
  setting_k->addr_type =
    sensor_sdk_util_get_kernel_i2c_addr_type(setting_u->addr_type);
  setting_k->data_type =
    sensor_sdk_util_get_kernel_i2c_data_type(setting_u->data_type);
  setting_k->delay = setting_u->delay;
}

void translate_sensor_seq_reg_setting(
  struct msm_camera_i2c_seq_reg_setting *setting_k,
  struct camera_i2c_seq_reg_setting *setting_u)
{
  setting_k->reg_setting =
    (struct msm_camera_i2c_seq_reg_array *)setting_u->reg_setting;
  setting_k->size = setting_u->size;
  setting_k->addr_type =
    sensor_sdk_util_get_kernel_i2c_addr_type(setting_u->addr_type);
  setting_k->delay = setting_u->delay;
}

void translate_sensor_reg_setting_array(
  struct msm_camera_i2c_reg_setting_array *settings_k,
  struct camera_i2c_reg_setting_array *settings_u)
{
  uint32_t i = 0;
  settings_k->size = settings_u->size;
  settings_k->addr_type =
    sensor_sdk_util_get_kernel_i2c_addr_type(settings_u->addr_type);
  settings_k->data_type =
    sensor_sdk_util_get_kernel_i2c_data_type(settings_u->data_type);
  settings_k->delay = settings_u->delay;

  for (i = 0; i < settings_k->size;i++) {
    settings_k->reg_setting_a[i].reg_addr =
      settings_u->reg_setting_a[i].reg_addr;
    settings_k->reg_setting_a[i].reg_data =
      settings_u->reg_setting_a[i].reg_data;
    settings_k->reg_setting_a[i].delay =
      settings_u->reg_setting_a[i].delay;
  }
}

void translate_camera_power_setting(
  struct msm_sensor_power_setting_array *power_setting_array_k,
  struct camera_power_setting_array *power_setting_array_u)
{
  uint32_t i =0;
  power_setting_array_k->size = power_setting_array_u->size;
  power_setting_array_k->size_down = power_setting_array_u->size_down;

  for (i = 0;i < power_setting_array_k->size; i++) {
    power_setting_array_k->power_setting_a[i].seq_type =
      sensor_sdk_util_get_kernel_power_seq_type(
      power_setting_array_u->power_setting_a[i].seq_type);
    power_setting_array_k->power_setting_a[i].config_val =
      power_setting_array_u->power_setting_a[i].config_val;
    power_setting_array_k->power_setting_a[i].delay =
      power_setting_array_u->power_setting_a[i].delay;
    power_setting_array_k->power_setting_a[i].seq_val =
      power_setting_array_u->power_setting_a[i].seq_val;
  }

  for (i = 0;i < power_setting_array_k->size_down; i++) {
    power_setting_array_k->power_down_setting_a[i].seq_type =
      sensor_sdk_util_get_kernel_power_seq_type(
      power_setting_array_u->power_down_setting_a[i].seq_type);
    power_setting_array_k->power_down_setting_a[i].config_val =
      power_setting_array_u->power_down_setting_a[i].config_val;
    power_setting_array_k->power_down_setting_a[i].delay =
      power_setting_array_u->power_down_setting_a[i].delay;
    power_setting_array_k->power_down_setting_a[i].seq_val =
      power_setting_array_u->power_down_setting_a[i].seq_val;
  }
}

/** translate_ois_reg_settings
 *
 * This is a utility function that converts ois
 * reg settings as defined in ois_driver.h to
 * kernel settings defined by msm_cam_sensor.h
 *
 *  @reg_settings_k: kernel format
 *  @reg_settings_u: userspace format
 *  @array_size: number of elements to be converted
 *
 *  Return: void
 **/
void translate_ois_reg_settings(
  struct reg_settings_ois_t *reg_settings_k,
  ois_reg_settings_t *reg_settings_u, uint32_t array_size)
{
  uint32_t i = 0;

  for (i = 0; i < array_size; i++) {

    reg_settings_k[i].reg_addr = reg_settings_u[i].reg_addr;
    reg_settings_k[i].addr_type = sensor_sdk_util_get_kernel_i2c_addr_type(
        reg_settings_u[i].addr_type);
    reg_settings_k[i].reg_data = reg_settings_u[i].reg_data;
    reg_settings_k[i].data_type = sensor_sdk_util_get_kernel_i2c_data_type(
        reg_settings_u[i].data_type);
    reg_settings_k[i].delay = reg_settings_u[i].delay;

    switch (reg_settings_u[i].i2c_operation) {
    case OIS_WRITE:
      reg_settings_k[i].i2c_operation = MSM_OIS_WRITE;
      break;
    case OIS_POLL:
      reg_settings_k[i].i2c_operation = MSM_OIS_POLL;
      break;
    case OIS_READ:
      reg_settings_k[i].i2c_operation = MSM_OIS_READ;
      break;
    default:
      SERR("Invalid i2c_operation = %d", reg_settings_u[i].i2c_operation);
      return;
    }

  }

}
#endif /* __SENSOR_SDK_UTILS_H__ */
