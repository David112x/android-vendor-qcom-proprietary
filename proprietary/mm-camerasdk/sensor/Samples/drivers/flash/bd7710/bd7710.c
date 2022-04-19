/**
 *Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "bd7710.h"

void *flash_driver_open_lib(void)
{
  return &flash_lib_ptr;
}
