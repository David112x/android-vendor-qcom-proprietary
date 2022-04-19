/**
 * pmic_flash.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "pmic_flash.h"

void *flash_driver_open_lib(void)
{
  return &flash_lib_ptr;
}
