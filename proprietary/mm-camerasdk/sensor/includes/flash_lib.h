/*==========================================================

  Copyright (c) 2014-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================*/
#ifndef __FLASH_LIB_H__
#define __FLASH_LIB_H__

#include "sensor_sdk_common.h"

#define MAX_LED_TRIGGERS   3

/*
 * Flash driver version is given by:
 * <Major version>.<Minor version>.<Patch version>
 */
#define FLASH_DRIVER_VERSION "1.0.0"
#define FLASH_SDK_CAPABILITIES "PMIC, I2C, GPIO"

typedef enum {
  FLASH_TYPE_LED,
  FLASH_TYPE_STROBE
} flash_type_t;

enum flash_driver_type {
  FLASH_DRIVER_TYPE_PMIC,
  FLASH_DRIVER_TYPE_I2C,
  FLASH_DRIVER_TYPE_GPIO,
  FLASH_DRIVER_TYPE_DEFAULT
};

/**
 * flash_i2c_params_t:
 * @slave_addr: 8 bit write I2C address of the slave
 * @i2c_data_type: Width of the data
 * @i2c_addr_type: Width of the address
 * @flash_off_settings: I2C setting to off flash
 * @flash_low_settings: I2C setting with low intensity
 * @flash_high_settings: I2C setting with high intensity
 **/
typedef struct {
  unsigned int slave_addr;
  enum camera_i2c_freq_mode i2c_freq_mode;
  enum camera_i2c_reg_addr_type i2c_addr_type;
  struct camera_i2c_reg_setting_array flash_init_settings;
  struct camera_i2c_reg_setting_array flash_off_settings;
  struct camera_i2c_reg_setting_array flash_low_settings;
  struct camera_i2c_reg_setting_array flash_high_settings;
} flash_i2c_params_t;

/**
 * flash_lib_t:
 * @flash_name: Vendor name of the flash
 * @flash_type: Type of the flash. LED / STROBE
 * @flash_driver_type: Driver type: I2C / PMIC / GPIO
 * @i2c_flash_info: I2C flash info
 * @num_of_flash: No of available flashes
 * @max_flash_current: Max flash current supported
 * @max_torch_current: Max torch current supported
 * @max_flash_duration: Max duration of flash it can be on.
 * @main_flash_on_frame_skip: number of frames to be skipped from main led on
 * @main_flash_off_frame_skip: number of frames to be skipped from main led off
 * @torch_on_frame_skip: number of frames to be skipped from estimation start
 * @torch_off_frame_skip: number of frames to be skipped from estimation done
 **/
typedef struct {
  char flash_name[NAME_SIZE_MAX];
  flash_type_t flash_type;
  enum flash_driver_type flash_driver_type;
  struct camera_power_setting_array power_setting_array;
  flash_i2c_params_t i2c_flash_info;
  unsigned char num_of_flash;
  unsigned int max_flash_current[MAX_LED_TRIGGERS];
  unsigned int max_torch_current[MAX_LED_TRIGGERS];
  unsigned int max_flash_duration[MAX_LED_TRIGGERS];
  unsigned int main_flash_on_frame_skip;
  unsigned int main_flash_off_frame_skip;
  unsigned int torch_on_frame_skip;
  unsigned int torch_off_frame_skip;
} flash_lib_t;

#endif /* __FLASH_DRIVER_H__ */
