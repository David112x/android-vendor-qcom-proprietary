/*==========================================================

  Copyright (c) 2014-2016 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

===========================================================*/
#ifndef __CHROMATIX_PARAMS_H__
#define __CHROMATIX_PARAMS_H__

#include "chromatix.h"
#include "chromatix_common.h"
#if defined(CHROMATIX_VERSION) && CHROMATIX_VERSION >= 0x304
#include "chromatix_cpp.h"
#else
#define chromatix_cpp_type int
#endif
#if defined(CHROMATIX_VERSION) && CHROMATIX_VERSION >= 0x304
#include "chromatix_swpostproc.h"
#else
#define chromatix_sw_postproc_type int
#endif

typedef enum
{
  SENSOR_CHROMATIX_ISP,
  SENSOR_CHROMATIX_ISP_SNAPSHOT,
  SENSOR_CHROMATIX_ISP_COMMON,
  SENSOR_CHROMATIX_CPP_PREVIEW,
  SENSOR_CHROMATIX_CPP_SNAPSHOT,
  SENSOR_CHROMATIX_CPP_FLASH_SNAPSHOT,
  SENSOR_CHROMATIX_CPP_OIS_SNAPSHOT,
  SENSOR_CHROMATIX_CPP_OIS_US_SNAPSHOT,
  SENSOR_CHROMATIX_CPP_OIS_DS_SNAPSHOT,
  SENSOR_CHROMATIX_CPP_DS,
  SENSOR_CHROMATIX_CPP_US,
  SENSOR_CHROMATIX_CPP_VIDEO,
  SENSOR_CHROMATIX_SW_PPROC,
  SENSOR_CHROMATIX_3A,
  SENSOR_CHROMATIX_IOT,
  SENSOR_CHROMATIX_EXTERNAL,
  SENSOR_CHROMATIX_MAX,
} sensor_chromatix_type_t;

typedef struct {
  unsigned int  stream_mask;
  void *chromatix_ptr[SENSOR_CHROMATIX_MAX];
  char *chromatix_lib_name[SENSOR_CHROMATIX_MAX];
  unsigned char chromatix_reloaded[SENSOR_CHROMATIX_MAX];
} sensor_chromatix_params_t;

#endif /* __CHROMATIX_PARAMS_H__ */
