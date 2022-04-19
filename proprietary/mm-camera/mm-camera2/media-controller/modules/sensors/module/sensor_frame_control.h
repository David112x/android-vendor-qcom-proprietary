/* sensor_frame_control.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SENSOR_FRAME_CONTROL_H__
#define __SENSOR_FRAME_CONTROL_H__

#include <linux/media.h>
#include "mct_stream.h"
#include "module_sensor.h"

boolean sensor_fc_store(module_sensor_bundle_info_t* s_bundle,
    uint32_t future_frame_id, sensor_frame_ctrl_type_t type, void *data);
boolean sensor_fc_process(mct_module_t* module,
  module_sensor_bundle_info_t* s_bundle, mct_event_t* event);

boolean sensor_fc_process_now(mct_module_t* module,
  module_sensor_bundle_info_t* s_bundle, mct_event_t* event, uint32_t qidx);

boolean sensor_fc_report_meta(mct_module_t* module,
  module_sensor_bundle_info_t *s_bundle, mct_event_t* event);

boolean sensor_fc_post_static_meta(mct_module_t *module,
  module_sensor_bundle_info_t *s_bundle, mct_event_t *event);

#endif /* __SENSOR_FRAME_CONTROL_H__ */
