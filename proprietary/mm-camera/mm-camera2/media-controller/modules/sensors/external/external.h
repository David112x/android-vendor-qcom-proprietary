/*============================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __EXTERNAL_H__
#define __EXTERNAL_H__

#include <media/msm_cam_sensor.h>
#include "sensor_common.h"

typedef struct{
  int32_t                         fd;
  uint32_t                        ext_version;
  uint32_t                        pipeline_delay;
} external_ctrl_t;


#endif
