/*==============================================================================

  Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

==============================================================================*/
#ifndef __CHROMATIX_SUB_MODULE_H__
#define __CHROMATIX_SUB_MODULE_H__

#include "chromatix_params.h"
#include "chromatix_manager.h"

typedef struct {
  chromatix_manager_type *cm;

  void *chromatix_ptr[SENSOR_CHROMATIX_MAX];
  char *chromatix_name[SENSOR_CHROMATIX_MAX];
} chromatix_data_t;

#endif
