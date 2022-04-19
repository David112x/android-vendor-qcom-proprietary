/**
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef __BD7710_H__
#define __BD7710_H__

#include "flash_lib.h"

/*
 * if flash_driver_type is set to FLASH_DRIVER_DEFAULT
 * flash specifications are taken from kernel and the
 * flash is driven by kernel.
 * For other value FLASH_DRIVER_PMIC / FLASH_DRIVER_GPIO
 * FLASH_DRIVER_I2C all the information like max current
 * max duration and num of flashes must be provided in
 * this driver.
 */
static flash_lib_t flash_lib_ptr =
{
  .flash_name = "bd7710",
  .flash_type = FLASH_TYPE_LED,
  .flash_driver_type = FLASH_DRIVER_TYPE_I2C,
  .power_setting_array =
  {
    .power_setting_a =
    {
      {
        .seq_type = CAMERA_POW_SEQ_GPIO,
        .seq_val = CAMERA_GPIO_RESET,
        .config_val = GPIO_OUT_LOW,
        .delay = 1,
      },
      {
        .seq_type = CAMERA_POW_SEQ_GPIO,
        .seq_val = CAMERA_GPIO_RESET,
        .config_val = GPIO_OUT_HIGH,
        .delay = 1,
      },
    },
    .size = 2,
    .power_down_setting_a =
    {
      {
        .seq_type = CAMERA_POW_SEQ_GPIO,
        .seq_val = CAMERA_GPIO_RESET,
        .config_val = GPIO_OUT_HIGH,
        .delay = 1,
      },
      {
        .seq_type = CAMERA_POW_SEQ_GPIO,
        .seq_val = CAMERA_GPIO_RESET,
        .config_val = GPIO_OUT_LOW,
        .delay = 1,
      },
    },
    .size_down = 2,
  },
  .num_of_flash = 1,
  .max_flash_current = {1000, 0, 0},
  .max_torch_current = {200, 0, 0},
  .max_flash_duration = {1200, 0, 0},
  .i2c_flash_info =
  {
    .slave_addr = 0x66,
    .i2c_addr_type = CAMERA_I2C_BYTE_ADDR,
    .flash_init_settings =
    {
      .reg_setting_a =
      {
        {0x00, 0x10, 0x00},
      },
      .size = 1,
      .addr_type = CAMERA_I2C_BYTE_ADDR,
      .data_type = CAMERA_I2C_BYTE_DATA,
      .delay = 0,
    },
    .flash_off_settings =
    {
      .reg_setting_a =
      {
        {0x05, 0x00, 0x00},
        {0x02, 0x00, 0x00},
      },
      .size = 2,
      .addr_type = CAMERA_I2C_BYTE_ADDR,
      .data_type = CAMERA_I2C_BYTE_DATA,
      .delay = 0,
    },
    .flash_low_settings =
    {
      .reg_setting_a =
      {
        {0x05, 0x25, 0x00},
        {0x00, 0x38, 0x00},
        {0x02, 0x40, 0x00}
      },
      .size = 3,
      .addr_type = CAMERA_I2C_BYTE_ADDR,
      .data_type = CAMERA_I2C_BYTE_DATA,
      .delay = 0,
    },
    .flash_high_settings =
    {
      .reg_setting_a =
      {
        {0x05, 0x25, 0x00},
        {0x02, 0x7F, 0x00}
      },
      .size = 2,
      .addr_type = CAMERA_I2C_BYTE_ADDR,
      .data_type = CAMERA_I2C_BYTE_DATA,
      .delay = 0,
    },
  },
};

#endif
