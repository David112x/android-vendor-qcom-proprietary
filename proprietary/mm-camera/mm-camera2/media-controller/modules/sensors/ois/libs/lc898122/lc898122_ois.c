/*==========================================================
 Copyright (c) 2014-2015 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ===========================================================*/

#include "lc898122_ois_13P1BSD05.h"
#include "ois_driver.h"

static ois_driver_ctrl_t ois_lib_ptr = {
#include "lc898122_ois.h"
};

void *ois_driver_open_lib(void)
{
  return &ois_lib_ptr;
}
