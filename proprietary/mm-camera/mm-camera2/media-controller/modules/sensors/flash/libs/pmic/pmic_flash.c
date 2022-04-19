/*============================================================================

  Copyright (c) 2014-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "pmic_flash.h"

void *flash_driver_open_lib(void)
{
  return &flash_lib_ptr;
}
