
/* sensor_fast_aec.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __SENSOR_FAST_AEC_H__
#define __SENSOR_FAST_AEC_H__


boolean module_sensor_handle_fast_aec_ack(mct_module_t *module,
  uint32_t identity);


void sensor_fast_aec_timeout_ack(mct_module_t* module,
  mct_event_t* event, module_sensor_bundle_info_t* s_bundle);

boolean module_sensor_stream_on_fastaec(mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t *s_bundle);

boolean port_sensor_handle_stream_on_fastaec(mct_module_t *module,
  mct_event_t *event,
  sensor_bundle_info_t *bundle_info);

#endif /* __SENSOR_FAST_AEC_H__ */

