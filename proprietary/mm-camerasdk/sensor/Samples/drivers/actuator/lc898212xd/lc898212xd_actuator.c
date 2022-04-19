/**
 * lc898212xd_actuator.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "actuator_driver.h"

static actuator_driver_ctrl_t actuator_lib_ptr = {
#include "lc898212xd_actuator.h"
};

void *actuator_driver_open_lib(void)
{
  return &actuator_lib_ptr;
}
