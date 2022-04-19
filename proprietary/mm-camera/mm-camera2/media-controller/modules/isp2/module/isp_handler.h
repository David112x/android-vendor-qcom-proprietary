/* isp_handler.h
 *
 * Copyright (c) 2012-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ISP_HANDLER_H__
#define __ISP_HANDLER_H__

/** isp_mapped_stream_info_t:
 *
 *  @module: mct module handle
 *  @port: mct port handle
 *  @isp_pix_output: isp pix output handle
 *  @session_param: isp session param handle
 *  @hw_id: isp hw id
 **/
typedef struct {
  mct_module_t        *module;
  mct_port_t          *port;
  isp_pix_out_info_t  *isp_pix_output;
  isp_session_param_t *session_param;
  isp_hw_id_t          hw_id;
  uint32_t             num_pix_stream;
  iface_resource_request_t *resource_request;
} isp_mapped_stream_info_t;

boolean isp_handler_create_internal_link_for_stream(
  mct_module_t *module, isp_session_param_t *session_param,
  unsigned int identity, isp_hw_id_t hw_id);

boolean isp_handler_create_stream_param(mct_port_t *port, void *peer_caps,
 mct_stream_info_t *stream_info);

boolean isp_handler_get_hw_params(mct_port_t *port,
  mct_stream_info_t *mct_stream_info, isp_hw_streamid_t *hw_stream,
  isp_hw_id_t *hw_id);

void isp_handler_destroy_stream_param(mct_port_t *port, unsigned int identity);

boolean isp_handler_control_set_parm(mct_module_t *module, mct_port_t *port,
  mct_event_t *event);

boolean isp_handler_create_internal_link(mct_port_t *port,
  mct_stream_info_t *stream_info);

boolean isp_handler_control_parm_stream_buf(mct_module_t *module,
  mct_port_t *port, mct_event_t *event);

boolean isp_handler_control_streamon(mct_module_t *module, mct_port_t *port,
  mct_event_t *event);

boolean isp_handler_control_streamoff(mct_module_t *module, mct_port_t *port,
  mct_event_t *event);

boolean isp_handler_module_set_chromatix_ptr(mct_module_t *module,
  mct_port_t *port, mct_event_t *event);

boolean isp_handler_module_offline_pipeline_config(mct_module_t *module,
  mct_port_t *port, mct_event_t *event);

boolean isp_handler_module_start_offline_pipeline(mct_module_t *module,
  mct_port_t *port, mct_event_t *event);

boolean isp_handler_module_set_stream_config(mct_module_t *module,
  mct_port_t *port, mct_event_t *event);

boolean isp_handler_module_offline_metadata(mct_module_t *module,
  mct_port_t *port, mct_event_t *event);

boolean isp_handler_module_iface_request_output_resource(mct_module_t *module,
  mct_port_t *port, mct_event_t *event);

boolean isp_handler_module_iface_request_offline_output_resource(
  mct_module_t *module, mct_port_t *port, mct_event_t *event);

boolean isp_handler_sensor_output_dim(mct_module_t *module, mct_event_t *event);

boolean isp_handler_event(mct_module_t *module, mct_port_t *port,
  mct_event_t *event);

boolean isp_handler_handle_super_param(mct_module_t *module, mct_port_t *port,
  mct_event_t *event);

boolean isp_handler_link_intra_session(mct_module_t *module, mct_port_t *port,
  mct_event_t *event);

boolean isp_handler_module_handle_reg_update(mct_module_t *module,
  mct_port_t *port, mct_event_t *event);

boolean isp_handler_control_sof(mct_module_t *module, mct_port_t *port,
  mct_event_t *event);

boolean isp_handler_sof(mct_module_t *module, mct_port_t *port,
  mct_event_t *event);

boolean isp_handler_module_fe_sof(mct_module_t *module, mct_port_t *port,
  mct_event_t *event);

boolean isp_handler_raw_stats_divert(mct_module_t *module, mct_event_t *event);

boolean isp_handler_request_pp_divert(mct_module_t *module,
  mct_port_t *port, mct_event_t *event);

boolean isp_handler_handle_hal_param(mct_module_t *module, mct_port_t *port,
  mct_event_t *event);

boolean isp_handler_stats_data_ack(mct_module_t *module, mct_event_t *event);

boolean isp_handler_prepare_snapshot(mct_module_t *module, mct_port_t *port,
  mct_event_t *event);

boolean isp_handler_stop_zsl_snapshot(mct_module_t *module, mct_port_t *port,
  mct_event_t *event);

boolean isp_handler_module_iface_request_stream_mapping_info
        (mct_module_t *module, mct_port_t *port, mct_event_t *event);

boolean isp_handler_module_send_initial_crop_info(mct_module_t *module,
  mct_port_t *port, mct_event_t *event);

boolean isp_handler_hw_wake(mct_module_t *module, mct_port_t *port,
  mct_event_t *event);
#endif
