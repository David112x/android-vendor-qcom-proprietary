/*===========================================================================
 * Copyright (c) 2012-2013,2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*==========================================================================*/

#ifndef __PORT_SENSOR_H__
#define __PORT_SENSOR_H__

#include "sensor_common.h"
#include "module_sensor_dualcamera.h"

#define SENSOR_MEASURE_FPS 0

boolean port_sensor_create(void *data, void *user_data);
boolean port_sensor_handle_stream_on(mct_module_t *module, mct_event_t *event,
  sensor_bundle_info_t *bundle_info);
boolean port_sensor_handle_aec_update(
  mct_module_t *module, mct_port_t *port, mct_event_t *event,
  sensor_bundle_info_t *bundle_info, mct_event_module_t *event_module);

#endif /* __PORT_SENSOR_H__ */
