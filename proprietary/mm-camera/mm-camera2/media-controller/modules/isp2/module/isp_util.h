/* isp_util.h
 *
 * Copyright (c) 2012-2014, 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ISP_UTIL_H__
#define __ISP_UTIL_H__

#include "isp_defs.h"
#include "isp_module.h"

#define ISP_SWAP_STATS_PARAM(dst, src) \
  if (src->is_valid) {                 \
      isp_saved_stats_params_t *tmp;   \
      tmp = dst;                       \
      dst = src;                       \
      src = tmp;                       \
      src->is_valid = FALSE;           \
      dst->is_valid = TRUE;            \
  }

/** isp_util_forward_event_info_t:
 *
 *  @session_param: session param
 *  @event: event handle
 *  @hw_id: ISP hw id
 **/
typedef struct {
  isp_session_param_t *session_param;
  mct_event_t         *event;
  isp_hw_id_t          hw_id;
} isp_util_forward_event_info_t;

/** isp_util_identity_list_t:
 *
 *  @identity: array of identities
 *  @num_identity: number of valid identities
 **/
typedef struct {
  uint32_t identity[MAX_IDENTITES];
  uint32_t num_identity;
} isp_util_identity_list_t;

/** isp_util_stream_desc:
 *
 *  @dim: original dimension
 *  @norm_dim: normalize dimension to sensor FOV
 *  @type: stream type
 *  @pp_mask: postprocess mask
 **/
typedef struct {
  cam_dimension_t dim;
  cam_dimension_t norm_dim;
  cam_dimension_t orig_dim;
  cam_dimension_t changed_dim;
  cam_stream_type_t type;
  uint32_t pp_mask;
} isp_util_stream_desc;

extern uint32_t isp_modules_loglevel[ISP_LOG_MAX];

void isp_util_update_stream_info_dims_for_rotation(
    mct_stream_info_t *stream_info);

boolean isp_util_compare_sessionid_from_session_param(void *data1, void *data2);

boolean isp_util_compare_identity_from_stream_param(void *data1,
  void *data2);

boolean isp_util_compare_identity(void *data1, void *data2);

boolean isp_util_find_port_based_on_identity(void *data1, void *data2);

boolean isp_util_get_port_from_module(mct_module_t *module,
  mct_port_t **port, mct_port_direction_t  direction, uint32_t identity);

boolean isp_util_dump_register(mct_module_t *isp_module,
  isp_session_param_t *session_param, uint32_t frame_id, uint32_t num_reg);

boolean isp_util_forward_event(mct_port_t *port, mct_event_t *event);

boolean isp_util_forward_event_from_module(mct_module_t *module,
  mct_event_t *event);

boolean isp_util_forward_event_downstream_to_type(mct_module_t *module,
  mct_event_t *event, mct_port_caps_type_t port_type);

boolean isp_util_forward_event_downstream_to_all_types(mct_module_t *module,
  mct_event_t *event);

boolean isp_util_forward_event_to_all_internal_pipelines(
  isp_session_param_t *session_param, mct_event_t *event);

boolean isp_util_get_session_stream_params(mct_module_t *module,
  uint32_t identity, isp_session_param_t **session_param,
  isp_stream_param_t **stream_param);

boolean isp_util_get_session_params(mct_module_t *module, uint32_t session_id,
  isp_session_param_t **session_param);

boolean isp_util_get_stream_params(isp_session_param_t *session_param,
  uint32_t identity, isp_stream_param_t **stream_param);

boolean isp_util_forward_event_to_internal_pipeline(
  isp_session_param_t *session_param, mct_event_t *event, isp_hw_id_t hw_id);

boolean isp_util_trigger_internal_hw_update_event(
 isp_session_param_t *session_param, uint32_t identity, uint32_t frame_id,
 isp_hw_id_t *hw_ids, uint32_t num_isp, isp_saved_events_t *saved_events);

boolean isp_util_get_vfe_id(isp_hw_id_t hw_id);

boolean isp_util_get_stream_src(isp_hw_streamid_t hw_stream);

boolean isp_util_forward_event_to_all_streams_all_internal_pipelines(
  isp_session_param_t *session_param, mct_event_t *event);

enum msm_vfe_axi_stream_src isp_util_get_axi_src_type(cam_format_t fmt,
  uint8_t stream_id);

boolean isp_util_broadcast_crop_info(mct_module_t *module,
  isp_session_param_t *session_param, unsigned int frame_id,
  struct timeval *timestamp, isp_zoom_params_t *zoom_params,
  cam_zoom_info_t *zoom, boolean is_online);

boolean isp_util_prepare_meta_dump(mct_module_t *isp_module,
  isp_session_param_t *session_param);

boolean isp_util_clear_all_stored_metadata(isp_session_param_t *session_param);

boolean isp_util_report_stored_metadata(mct_module_t *module,
  isp_session_param_t *session_param, uint32_t frame_id);

boolean isp_util_send_metadata_entry(mct_module_t *module,
  mct_bus_msg_t *bus_msg, uint32_t frame_id);

boolean isp_util_send_metadata(mct_module_t *isp_module,
  isp_session_param_t *session_param, boolean is_online);

boolean isp_util_send_rdi_only_metadata(mct_module_t *isp_module,
  isp_session_param_t *session_param);

boolean isp_util_read_reg_dump(int vfe_fd,
  void *dump_entry, uint32_t read_type,
  uint32_t read_len, uint8_t print_dump);

boolean isp_util_update_hw_param(mct_module_t *module,
  isp_session_param_t *session_param);

boolean isp_util_report_meta_dump(mct_module_t *module,
  isp_session_param_t *session_param);

boolean isp_util_post_cds_update(mct_module_t *module,
  isp_session_param_t *session_param);

boolean isp_util_free_all_meta_dump_entry(mct_module_t *module,
  isp_session_param_t *session_param);

void isp_util_free_saved_events(isp_saved_events_t *saved_events);

boolean isp_util_saved_event_reset_valid_flag(isp_saved_events_t *saved_events);

boolean isp_util_forward_saved_events(isp_session_param_t *session_param,
  isp_saved_events_t *saved_events, boolean reset_valid_flag);

boolean isp_util_convert_set_parm_event_type(cam_intf_parm_type_t hal_type,
  isp_set_param_type_t *isp_type, uint32_t *set_parm_size);

boolean isp_util_clear_invalid_saved_event(isp_saved_events_t *saved_events);

boolean isp_util_clear_saved_events(isp_saved_events_t *saved_events);

boolean isp_util_free_dmi_meta_tbl(mct_module_t *module,
  isp_session_param_t *session_param);

boolean isp_util_free_per_frame_meta(mct_module_t *module,
  isp_session_param_t *session_param);

boolean isp_util_set_split_output_info_per_stream(
  isp_session_param_t *session_param, isp_out_info_t *isp_split_out_info,
  uint32_t hw_id, uint32_t stream_identity);

boolean isp_util_set_isp_frame_skip(mct_module_t *module,
  isp_session_param_t *session_param, uint8_t *frame_skip,
  uint32_t hw_id, uint32_t stream_identity);

boolean isp_util_forward_event_to_stream(void *data, void *user_data);

boolean isp_util_forward_event_to_all_streams(
  isp_session_param_t *session_param, mct_event_t *event, isp_hw_id_t hw_id);

boolean isp_util_set_fast_aec_mode(mct_module_t *module, mct_port_t *port,
mct_event_t *event, mct_fast_aec_mode_t *fast_aec);

boolean isp_util_clear_isp_port_data(void *data1, void *data2);

boolean isp_util_decide_port_mapping(mct_module_t *module,
  isp_session_param_t *session_param);

boolean isp_util_set_hal_version(isp_session_param_t *session_param,
  cam_hal_version_t *hal_version);

boolean isp_util_set_chromatix_meta(
  isp_session_param_t *session_param,
  mct_bus_msg_sensor_metadata_t *chromatix_meta);

boolean isp_set_pipeline_delay(mct_module_t *module, uint32_t session_id,
  mct_pipeline_session_data_t *session_data);

boolean isp_fill_frame_format_param(cam_format_t fmt,
  uint32_t *output);

boolean isp_util_get_isp_version(mct_module_t *module,
  uint32_t identity, uint32_t *version);

boolean isp_util_compare_hal_stream_type(void *data, void *userdata);

boolean isp_util_compare_stream_type(void *data, void *user_data);

boolean isp_util_get_session_stream_param_from_type(mct_module_t *module,
  cam_stream_type_t stream_type, isp_session_param_t **session_param,
  isp_stream_param_t **stream_param, uint32_t session_id);

boolean isp_util_compare_non_identity(void *data1, void *data2);

boolean isp_util_get_stream_param_from_type(mct_module_t *module,
  cam_stream_type_t stream_type, isp_session_param_t *session_param,
  isp_stream_param_t **stream_param);

boolean isp_util_forward_event_to_session_based_stream_all_int_pipelines(
  isp_session_param_t *session_param, mct_event_t *event);

boolean isp_util_send_submod_enable_to_pipeline(isp_resource_t *isp_resource,
  isp_session_param_t *session_param);

boolean isp_util_update_hw_param_offline(mct_module_t *module,
  isp_session_param_t *session_param, isp_hw_id_t hw_id);

metadata_buffer_t *isp_util_extract_metadata_buffer(mct_module_t *module,
  isp_session_param_t *session_param, isp_stream_param_t *stream_param,
  cam_stream_parm_buffer_t *stream_parm_buf);

boolean isp_util_forward_offline_event_modules(isp_stream_param_t *stream_param,
  metadata_buffer_t *metadata, isp_session_param_t *session_param,
  mct_event_t *event, isp_hw_id_t hw_id);

void isp_util_fill_hal_params(mct_event_control_type_t type,
  mct_event_t *internel_event, void  *hal_param,
  uint32_t event_identity);

boolean isp_util_set_offline_cfg_event(isp_t *isp,
  isp_session_param_t *session_parm,
  isp_fetch_eng_cfg_data_t *fetch_eng_cfg_data,
  isp_hw_id_t hw_id, uint32_t identity);

boolean isp_util_forward_stats_update_to_pipeline(
  isp_session_param_t *session_param,
  isp_hw_id_t hw_id, uint32_t identity);

boolean isp_util_prepare_offline_cfg_data(
  isp_session_param_t *session_parm,
  metadata_buffer_t *metadata,
  isp_fetch_eng_cfg_data_t *fetch_eng_cfg_data,
  iface_fetch_engine_cfg_t *iface_fetch_eng_cfg);

boolean isp_util_set_chromatix_meta(
  isp_session_param_t *session_param,
  mct_bus_msg_sensor_metadata_t *chromatix_meta);

boolean isp_util_set_meta_hw_update_list(
  isp_session_param_t *session_param,
  void *meta_hw_update_list);

boolean isp_util_set_meta_hw_data_overwrite(
  isp_session_param_t *session_param,
  void *hw_overwrite_data);

boolean isp_util_handle_stream_info(mct_module_t *module,
  isp_session_param_t *session_param, cam_stream_size_info_t *streams_desc);

boolean isp_util_update_internal_pipeline(isp_session_param_t *session_param,
  uint32_t identity, isp_hw_id_t *hw_id, uint32_t num_isp);

boolean isp_util_set_preferred_mapping(mct_module_t *module,
  mct_event_t *event);

boolean isp_util_unlink_session_stream(mct_module_t *module,
  isp_session_param_t *session_param);

boolean isp_util_forward_event_to_int_pipeline_to_fill_cfg(
  isp_session_param_t *session_param, isp_resource_t *isp_resource,
  uint32_t identity, cam_stream_type_t stream_type);
boolean isp_util_send_adrc_hw_module_mask(mct_module_t *module,
  isp_session_param_t *session_param);

boolean isp_util_set_offline_split_output_info_per_stream(
  isp_session_param_t *session_param, isp_out_info_t *isp_split_out_info,
  uint32_t hw_id, uint32_t stream_identity, boolean firstpass);

boolean isp_util_set_offline_cfg_overwrite(isp_t *isp,
  isp_session_param_t *session_parm,
  isp_fetch_eng_cfg_data_t *fetch_eng_cfg_data,
  isp_hw_id_t hw_id, uint32_t identity);
void isp_util_free_offline_shared_queue(mct_queue_t *offline_trigger_param_q);

boolean isp_util_handle_vhdr(mct_module_t *module,
  isp_session_param_t *session_param, cam_sensor_hdr_type_t *video_hdr_mode);

boolean isp_util_send_zoom_crop(mct_module_t *isp_module,
  isp_session_param_t *session_param, isp_zoom_params_t *zoom_params_arr,
  cam_zoom_info_t *zoom);

boolean isp_util_post_hdr_data(mct_port_t *port,
  isp_session_param_t *session_param, uint32_t frame_id);

boolean isp_util_setloglevel(const char *name,
  isp_log_sub_modules_t mod_id);
#endif
