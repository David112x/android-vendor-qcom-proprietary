/*============================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/

#include "actuator_driver.h"

static actuator_driver_ctrl_t actuator_lib_ptr = {
#include "lc898212xd_qc2002_actuator.h"
};

void *actuator_driver_open_lib(void)
{
  return &actuator_lib_ptr;
}
