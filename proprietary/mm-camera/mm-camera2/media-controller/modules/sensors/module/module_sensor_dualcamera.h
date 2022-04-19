/*===========================================================================
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*==========================================================================*/

#ifndef __MODULE_SENSOR_DUALCAMERA_H__
#define __MODULE_SENSOR_DUALCAMERA_H__

#include "sensor_util.h"
#include "sensor_common.h"
#include "port_sensor.h"

// Time difference between same pair frames in ms
#define MAX_DUALC_PHASE_DIFF_THRESHOLD 6
#define MIN_DUALC_PHASE_DIFF_THRESHOLD -6

// Minimum number of frames to wait for before next phase correction
#define DUALC_PHASE_CORRECTION_FRAME_SET 6

int32_t module_sensor_dualcamera_phase_sync(mct_module_t* module,
  mct_event_t* event, module_sensor_bundle_info_t* s_bundle);

boolean module_sensor_dualcamera_streamon(mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t *s_bundle);

int32_t module_sensor_dualcamera_start_sensor(mct_module_t *module,
  module_sensor_bundle_info_t *s_bundle);

void module_sensor_dualcamera_reset_pc_params(
  module_sensor_bundle_info_t* s_bundle);

void module_sensor_dualcamera_apply_phase_correction( mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t* s_bundle,
  int32_t phase_diff);

boolean module_sensor_dualcamera_handle_aec_update(mct_module_t *module,
  mct_port_t *port, mct_event_t *event, sensor_bundle_info_t *bundle_info);

boolean module_sensor_dualcamera_handle_yuv_aec_update(mct_module_t *module,
  mct_port_t *port, mct_event_t *event, sensor_bundle_info_t *bundle_info);

boolean module_sensor_dualcamera_handle_bayer_aec_update(mct_module_t *module,
  mct_port_t *port, mct_event_t *event, sensor_bundle_info_t *bundle_info);

boolean module_sensor_dualcamera_handle_peer_aec_update(mct_module_t *module,
  mct_event_t *event, sensor_bundle_info_t *bundle_info);

void module_sensor_dualcamera_calculate_phase_diff(mct_module_t* module,
  mct_event_t* event, module_sensor_bundle_info_t* s_bundle,
  peer_info_t peer_info);

boolean module_sensor_dualcamera_link_sessions(mct_module_t *module,
  mct_event_t *event, sensor_bundle_info_t *bundle_info);

boolean module_sensor_dualcamera_hw_wakeup(mct_module_t *module,
  mct_event_t *event, sensor_bundle_info_t *bundle_info);

boolean module_sensor_dualcamera_hw_sleep(mct_module_t *module,
  mct_event_t *event, sensor_bundle_info_t *bundle_info);

boolean module_sensor_dualcamera_handle_master_info(mct_module_t *module,
  mct_event_t *event, sensor_bundle_info_t *bundle_info);

#endif /* __MODULE_SENSOR_DUALCAMERA_H__ */
