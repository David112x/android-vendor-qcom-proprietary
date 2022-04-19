/*============================================================================

  Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __CSID_H__
#define __CSID_H__

#include <media/msm_cam_sensor.h>
#include "sensor_common.h"

/** sensor_csid_data_t:
*
*  @fd: CSID v4l subdev fd
*  @csid_version: version of CSID hw
*  @cur_csid_params: store current CSID params applied to hw
*  @csi_lane_params: store CSI lane params common for all resolutions
*  @test_mode_params: store CSID test pattern mode info
*
*  This struct stores CSID sub module information per session
**/
typedef struct {
  int32_t                           fd;
  uint32_t                          csid_version;
  sensor_csi_config                 cur_csi_cfg;
  struct sensor_csid_testmode_parms *test_mode_params;
} sensor_csid_data_t;

#endif
