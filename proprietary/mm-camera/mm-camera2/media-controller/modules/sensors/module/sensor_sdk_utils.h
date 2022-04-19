/*==========================================================

  Copyright (c) 2015, 2018 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================*/

#ifndef __SENSOR_SDK_UTILS_H__
#define __SENSOR_SDK_UTILS_H__

#include <media/msm_cam_sensor.h>
#include "actuator_driver.h"
#include "sensor_lib.h"
#include "ois_driver.h"
#include "camera_config.h"
#include "eeprom_lib.h"

void translate_actuator_damping_param(
  struct damping_params_t *ringing_params_k,
  struct actuator_damping_params_t *ringing_params_u, uint32_t array_size);

void translate_actuator_reg_params(
  struct msm_actuator_reg_params_t *reg_params_k,
  struct actuator_reg_params_t *reg_params_u, uint32_t array_size);

void translate_actuator_reg_settings(
  struct reg_settings_t *reg_settings_k,
  struct actuator_reg_settings_t *reg_settings_u, uint32_t array_size);

void translate_actuator_region_params(
  struct region_params_t *region_params_k,
  struct actuator_region_params_t *region_params_u, uint32_t array_size);

void translate_sensor_csiphy_params(
  struct msm_camera_csiphy_params *csiphy_params_k,
  struct sensor_csi_params *csiphy_params_u,
  camera_csi_params_t *camera_csi_params,
  uint64_t data_rate);

void translate_sensor_csid_params(
  struct msm_camera_csid_params *csid_params_k,
  struct sensor_csi_params *csid_params_u,
  struct msm_camera_csid_vc_cfg *csid_vc_cfg,
  struct sensor_csid_lut_params *lut_params_u,
  camera_csi_params_t *camera_csi_params);

void translate_sensor_csid_test_params(
  struct msm_camera_csid_testmode_parms *csid_params_k,
  struct sensor_csid_testmode_parms *csid_params_u);

void translate_sensor_slave_info(
  struct msm_camera_sensor_slave_info *slave_info_k,
  struct camera_sensor_slave_info *slave_info_u,
  camera_module_config_t *camera_cfg,
  struct msm_sensor_power_setting *power_up_setting_k,
  struct msm_sensor_power_setting *power_down_setting_k);

void translate_sensor_reg_setting(
  struct msm_camera_i2c_reg_setting *setting_k,
  struct camera_i2c_reg_setting *setting_u);

void translate_sensor_seq_reg_setting(
  struct msm_camera_i2c_seq_reg_setting *setting_k,
  struct camera_i2c_seq_reg_setting *setting_u);

void translate_sensor_reg_setting_array(
  struct msm_camera_i2c_reg_setting_array *settings_k,
  struct camera_i2c_reg_setting_array *settings_u);

void translate_camera_power_setting(
  struct msm_sensor_power_setting_array *power_setting_array_k,
  struct camera_power_setting_array *power_setting_array_u);
enum i2c_freq_mode_t sensor_sdk_util_get_i2c_freq_mode(
  enum camera_i2c_freq_mode i2c_freq_mode);

void translate_ois_reg_settings(
  struct reg_settings_ois_t *reg_settings_k,
  ois_reg_settings_t *reg_settings_u, uint32_t array_size);

void translate_eeprom_memory_map(
  struct msm_eeprom_memory_map_array *eeprom_map_arr,
  struct eeprom_memory_map_array *memory_map_arr);

enum msm_camera_i2c_reg_addr_type sensor_sdk_util_get_kernel_i2c_addr_type(
  enum camera_i2c_reg_addr_type addr_type);

enum msm_camera_i2c_data_type sensor_sdk_util_get_kernel_i2c_data_type(
  enum camera_i2c_data_type data_type);

enum i2c_freq_mode_t sensor_sdk_util_get_i2c_freq_mode(
  enum camera_i2c_freq_mode i2c_freq_mode);

#endif

