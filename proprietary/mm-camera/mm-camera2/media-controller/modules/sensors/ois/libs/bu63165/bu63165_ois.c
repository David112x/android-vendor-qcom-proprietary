/*============================================================================
  Copyright (c) 2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/

#include "ois_driver.h"

static ois_driver_ctrl_t ois_lib_ptr = {
#include "bu63165_ois.h"
};

void *ois_driver_open_lib(void)
{
  return &ois_lib_ptr;
}
