/* isp_sub_module_port.h
 *
 * Copyright (c) 2012-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ISP_SUB_MODULE_PORT_H__
#define __ISP_SUB_MODULE_PORT_H__

/* std headers */
#include "pthread.h"

/* isp headers */
#include "isp_sub_module_common.h"

/** isp_sub_module_port_data_t:
 *
 *  @mutex: mutex lock
 *  @is_reserved: is reserved flag
 *  @session_id: session id
 *  @num_streams: number of streams
 *  @int_peer_port: handle to store int peer port
 **/
typedef struct {
  pthread_mutex_t mutex;
  boolean         is_reserved;
  uint32_t        session_id;
  uint32_t        num_streams;
} isp_sub_module_port_data_t;

mct_port_t *isp_sub_module_port_request_new_port(mct_module_t *module,
  mct_port_direction_t direction);

boolean isp_sub_module_port_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_create(mct_module_t *module,
  uint32_t num_sink_ports, uint32_t num_source_ports);

void isp_sub_module_port_delete_ports(mct_module_t *module);

boolean isp_sub_module_port_isp_private_event(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_trigger_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_append_hw_update_list(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_set_param(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_set_super_param(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_set_vfe_command(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_set_dual_led_calibration(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_set_parm_tintless(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
boolean isp_sub_module_set_manual_controls(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_set_log_level(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_set_hdr_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_request_stats_type(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_disable_module(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_set_zsl_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

#endif
