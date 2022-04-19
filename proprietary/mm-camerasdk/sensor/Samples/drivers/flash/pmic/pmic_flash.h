/**
 * pmic_flash.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __PMIC_FLASH_H__
#define __PMIC_FLASH_H__

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
static flash_lib_t flash_lib_ptr = {
  .flash_name = "pmic",
  .flash_driver_type = FLASH_DRIVER_TYPE_DEFAULT,
  .main_flash_on_frame_skip = 3,
  .main_flash_off_frame_skip = 3,
  .torch_on_frame_skip = 2,
  .torch_off_frame_skip = 2,
};

#endif
