/*============================================================================

  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "gpio_flash.h"

void *flash_driver_open_lib(void)
{
  return &flash_lib_ptr;
}
