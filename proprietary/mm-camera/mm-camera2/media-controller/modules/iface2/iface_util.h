/*============================================================================
Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/
#ifndef __IFACE_UTIL_H__
#define __IFACE_UTIL_H__

#include "iface.h"

typedef struct {
    /* hw streamon/off streams */
    uint32_t    hw_stream_ids[IFACE_MAX_STREAMS];
    uint32_t    num_hw_streams;

    /* mct streamon/off streams */
    uint32_t    user_stream_ids[MAX_STREAM_NUM_IN_BUNDLE];
    uint32_t    num_user_streams;

    /* mct streamon/off streams using default bufq */
    uint32_t    mct_ids_for_default_bufq[MAX_STREAM_NUM_IN_BUNDLE];
    uint32_t    num_mct_ids_for_default_bufq;

    /* hw streamon/off streams using shared bufq */
    uint32_t    mct_ids_for_shared_bufq[MAX_STREAM_NUM_IN_BUNDLE];
    uint32_t    num_mct_ids_for_shared_bufq;
} iface_util_process_bundle_info_t;

typedef struct {
    uint32_t  hw_version;
    uint32_t  max_fps;
}iface_util_maxfps_info_t;

static iface_util_maxfps_info_t iface_maxfps_hw_version[]=
{
  {ISP_MSM8917, 15},
  {ISP_MSM8937, 15},
  {ISP_MSM8952, 15},
  {ISP_MSM8953, 15},
  {ISP_MSM8956, 15},
  {ISP_SDM660,  15},
  {0, 0}
};

void iface_util_dump_sensor_cfg(iface_sink_port_t *sink_port);
uint32_t iface_util_find_primary_cid(sensor_out_info_t *sensor_cfg,
  sensor_src_port_cap_t *sensor_cap);
iface_port_t *iface_util_find_sink_port_by_session_id(iface_t *iface,
  uint32_t *session_id);
boolean iface_util_has_isp_pix_interface(iface_t *iface);
uint32_t iface_util_get_frame_skip_period(
 enum msm_vfe_frame_skip_pattern hfr_skip_pattern);
boolean iface_util_has_pix_resource(iface_t *iface, iface_sink_port_t *sink_port,
  mct_stream_info_t *stream_info);
int iface_util_del_stream(iface_t *iface, iface_stream_t *deleting_iface_stream);
iface_stream_t *iface_util_find_stream_in_sink_port(iface_port_t *iface_port,
  uint32_t session_id, uint32_t stream_id);
iface_hw_stream_t *iface_util_find_hw_stream_in_session(iface_session_t *session,
  uint32_t hw_stream_id);
iface_stream_t *iface_util_find_stream_in_src_port( iface_t *iface,
  iface_port_t *iface_src_port, uint32_t session_id, uint32_t stream_id);
iface_stream_t *iface_util_add_stream_to_sink_port(iface_t *iface,
  iface_port_t *iface_port, iface_session_t *session,
  uint32_t stream_id, mct_stream_info_t *stream_info);
int iface_util_del_stream_from_sink_port( iface_t *iface,
  iface_port_t *iface_sink_port, iface_stream_t *stream);
int iface_util_add_stream_to_src_port(iface_t *iface,
  iface_port_t *iface_src_port, iface_stream_t *stream);
int iface_util_del_stream_from_src_port(iface_t *iface,
  iface_port_t *iface_src_port, unsigned int session_id,
  unsigned int stream_id);
int iface_utill_fill_stats_stream_info(iface_session_t *session,
  iface_resource_request_t *isp_resource_request, iface_port_t *iface_sink_port,
  boolean is_offline);
static boolean iface_util_find_matched_src_port_by_caps(void *data1, void *data2);
iface_port_t *iface_util_get_match_src_port(iface_t *iface,
  iface_port_t *iface_sink_port, iface_stream_t *stream);
static boolean iface_util_compare_sink_port_caps(void *data1, void *data2);
static boolean iface_util_compare_identity_sink_port(void *data1, void *data2);
iface_port_t *iface_util_find_sink_port_by_cap(iface_t *iface,
  sensor_src_port_cap_t *sensor_cap);
iface_stream_t *iface_util_find_stream_in_sink_port_list(iface_t *iface,
  uint32_t session_id, uint32_t stream_id);
iface_session_t *iface_util_get_session_by_id(iface_t *iface,
  uint32_t session_id);
int iface_util_update_hw_buf_info(iface_t *iface, iface_port_t *iface_sink_port,
  iface_session_t *session, uint32_t *user_stream_ids, uint32_t num_user_streams);
void iface_util_choose_isp_interface(iface_t *ispif,
  iface_port_t *iface_port, iface_stream_t *stream);
static void iface_util_fill_hw_stream_plane_info(iface_hw_stream_t *hw_stream,
  cam_frame_len_offset_t *mct_plane_info);
static int32_t iface_util_fill_hw_stream_info_pix(
  iface_hw_stream_t *reserving_pix_stream, iface_session_t *session,
  iface_port_t *iface_sink_port, isp_pix_out_info_t *isp_pix_output,
  uint32_t isp_id_mask, uint8_t initial_frame_skip,
  boolean is_burst);
static int iface_util_reserve_pix_resource(iface_t *iface, uint32_t session_id,
  iface_resource_request_t *isp_resource);
int iface_util_set_hw_stream_config_pix(iface_t *iface,
  iface_port_t *iface_sink_port, uint32_t session_id, uint32_t stream_id);
iface_hw_stream_t *iface_util_fill_hw_stream_info_rdi(
  iface_session_t *iface_session, iface_sink_port_t *iface_sink_port,
  iface_stream_t *iface_stream, iface_intf_type_t reserved_rdi_intf,
  uint32_t isp_id);
int32_t iface_util_set_hw_stream_cfg_rdi(iface_t *iface,
  iface_port_t *iface_sink_port, iface_stream_t *user_stream,
  iface_session_t *iface_session, boolean is_meta, sensor_meta_t *meta_ch_info);
int32_t iface_utill_set_hw_stream_cfg_raw(iface_t *iface,
  iface_sink_port_t *iface_sink_port, iface_stream_t *user_stream,
  iface_session_t *iface_session, boolean is_meta, sensor_meta_t *meta_ch_info);
iface_hw_stream_t *iface_util_find_hw_stream_by_mct_id(iface_t *iface,
  iface_session_t *session, uint32_t mct_stream_id);
void iface_util_map_cam_sync_type_to_iface(
   iface_session_t *session,
  cam_sync_type_t sync_type);
void iface_util_map_cam_sync_mode_to_iface(
   iface_session_t *session,
   cam_sync_mode_t sync_mode);
int iface_util_set_vt(iface_t *iface, uint32_t session_id, int32_t *vt_enable);
int iface_util_handle_meta_stream_info(iface_t *iface, uint32_t session_id,
  uint32_t stream_id, cam_stream_size_info_t *stream_desc);
int iface_util_set_hfr(iface_t *iface, iface_session_t *session, uint32_t stream_id,
  int32_t *hfr_mode);
int iface_util_handle_bestshot(iface_t *iface, uint32_t session_id,
  cam_scene_mode_type *bestshot_mode);
int iface_util_handle_adv_capturemode(iface_t *iface, uint32_t session_id,
  uint32_t *enable);
int iface_util_request_frame_by_stream_ids(iface_t *iface,
  uint32_t session_id, void *parm_data, uint32_t frame_id);
int iface_util_set_hal_version(iface_t *iface, uint32_t session_id,
  cam_hal_version_t *hal_version);
int iface_util_set_frame_skip(iface_t *iface, uint32_t session_id,
  uint32_t hw_stream_id, enum msm_vfe_frame_skip_pattern *skip_pattern);
int iface_util_set_stats_frame_skip(iface_t *iface,
  iface_session_t *session, int32_t *skip_pattern);
int iface_util_set_bracketing_frame_skip(iface_t *iface,
  iface_session_t *session, int32_t *skip_pattern);
int iface_util_set_hal_frame_skip(iface_t *iface,
  iface_session_t *session, int32_t *skip_pattern);
int iface_util_handle_bracketing_update(iface_t *iface, iface_session_t *session,
  mct_bracketing_update_t *bracketing_update);
int iface_util_set_frame_skip_all_stream(iface_t *iface,
  iface_session_t *session);
enum msm_vfe_frame_skip_pattern iface_util_get_hfr_skip_pattern(
  iface_session_t *session);
void iface_util_dump_frame(int ion_fd, hw_stream_info_t *stream_info,
  cam_stream_type_t stream_type, iface_frame_buffer_t *image_buf,
  uint32_t frame_idx, uint32_t session_id);
int iface_util_buf_divert_notify(iface_t *iface,
  iface_frame_divert_notify_t *divert_event);
void iface_util_broadcast_sof_msg_to_modules(iface_t *isp, uint32_t session_id,
  uint32_t user_stream_id, mct_bus_msg_isp_sof_t *sof_event);
int iface_util_axi_notify (void *parent,  uint32_t handle, uint32_t session_id,
  uint32_t type, void *notify_data, uint32_t notify_data_size);
int iface_util_config_axi(iface_t *iface, iface_session_t *session);
int iface_util_axi_streamon(iface_t *iface,iface_session_t *iface_session,
  uint32_t num_hw_streams, uint32_t *hw_stream_ids);
int iface_util_unconfig_axi(iface_t *iface, iface_session_t *session);
int iface_util_axi_streamoff(iface_t *iface,iface_session_t *session,
  uint32_t num_hw_streams, uint32_t *hw_stream_ids, enum msm_vfe_axi_stream_cmd stop_cmd);
int iface_util_set_hw_stream_cfg(iface_t *iface,
  iface_port_t *iface_sink_port, uint32_t session_id, uint32_t stream_id);
enum msm_ispif_intftype iface_util_find_isp_intf_type(
  iface_hw_stream_t *hw_stream, uint32_t isp_id);
static int iface_util_request_image_buf_hal(iface_t *iface,
  iface_session_t *session, iface_hw_stream_t *hw_stream);
int iface_util_request_image_buf_native(
  iface_t *iface, iface_session_t *session, iface_hw_stream_t *hw_stream);
int iface_util_request_image_buf(iface_t *iface, iface_session_t *session,
  int num_hw_streams, uint32_t *hw_stream_ids);
int iface_util_update_streamon_id(iface_hw_stream_t *hw_stream,
 uint32_t user_streamon_id, uint32_t is_streamon);
uint32_t iface_util_decide_hw_streams(iface_t *iface, iface_session_t *session,
  uint32_t num_user_streams, uint32_t *user_stream_ids,
  uint32_t *hw_stream_ids, uint32_t is_streamon);
uint32_t iface_util_get_hw_streams_ids_in_session(iface_t *iface,
  iface_session_t *session, uint32_t *hw_stream_ids);
int iface_util_get_user_streams_by_bundle(iface_session_t *session,
  uint32_t stream_id, uint32_t *user_stream_ids, uint32_t is_streamon);
int iface_util_set_bundle(iface_t *iface, iface_port_t *iface_sink_port,
  iface_session_t *session, uint32_t stream_id, cam_bundle_config_t *bundle_param);
int iface_util_release_image_buf(iface_t *iface, iface_session_t *session,
  iface_hw_stream_t *hw_stream);
static void iface_update_thread_parm_in_sof(iface_session_thread_t *session_thread);
int iface_util_uv_subsample(iface_session_t *session, uint32_t uv_subsample_enb);
int iface_util_divert_ack(iface_t *iface, iface_session_t *session, uint32_t user_stream_id,
  uint32_t buf_idx, uint32_t is_dirty, boolean bayerdata, uint32_t buffer_access);
void iface_util_send_hw_stream_output_dim_downstream(iface_t *iface,
  iface_session_t *session, int32_t num_hw_streams, uint32_t *hw_stream_ids);
int iface_util_hw_notify_meta_valid(iface_t *iface, mct_bus_msg_t *bus_msg);
int iface_util_update_buf_info(iface_t *iface, iface_port_t *iface_sink_port,
  iface_session_t *session, uint32_t stream_id, mct_event_t *event);
int iface_util_add_buf_info(iface_t *iface, iface_session_t *session,
  int user_stream_id);
int iface_util_remove_buf_info(iface_t *iface, iface_port_t *iface_sink_port,
  iface_session_t *session, uint32_t stream_id, mct_event_t *event);
int iface_util_axi_unreg_shared_bufq(iface_session_t *session, iface_t *iface,
  uint32_t hw_stream_id, uint32_t user_stream_id);
int iface_util_set_master_slave_info(iface_session_t *session,
  iface_t *iface);
int iface_util_remove_bufq_from_stream(iface_session_t *session, iface_t *iface,
  uint32_t hw_stream_id, uint32_t user_stream_id);
int iface_util_io_cfg(iface_t *iface, iface_session_t *session,
  iface_port_t *iface_sink_port, uint32_t isp_id, uint32_t *vfe_clk);
int iface_util_request_image_bufq_input(iface_t *iface, mct_event_t *event);
int iface_util_handle_buffer_divert(iface_t *iface, mct_event_t *event);
int iface_util_send_buff_list_downstream(iface_t *iface,
  iface_session_t *session, int num_hw_streams, uint32_t *hw_stream_ids,
  uint32_t streamon_id);
int iface_util_stats_ack(iface_t *iface, iface_session_t *session,
  uint32_t user_stream_id, iface_raw_stats_buf_info_t *stats_buf_ack);
int iface_util_handle_frame_skip_event(iface_t *iface, iface_session_t *session,
  uint32_t user_stream_id, uint32_t skip_frame_id);
int iface_util_adjust_paramq(iface_t *iface, iface_session_t *session,
  uint32_t user_stream_id, uint32_t skip_frame_id, uint32_t frame_skip_num);
boolean iface_util_config_meta(iface_t *iface, iface_port_t *iface_sink_port,
  iface_session_t *session, uint32_t stream_id, sensor_meta_data_t *sensor_meta_cfg);
boolean iface_util_config_pdaf_meta(iface_t *iface,
  iface_port_t *iface_sink_port,iface_session_t *session, uint32_t stream_id,
  pdaf_sensor_native_info_t *pdaf_config);
int iface_util_decide_cds_update(iface_t *iface, iface_session_t *session,
  uint32_t user_stream_id, stats_update_t *stats_update);
int iface_util_update_cds(iface_t *iface, uint32_t session_id,
  uint32_t user_stream_id);
int iface_util_request_isp_cds_update(iface_t *iface,
  iface_session_t *session);
int iface_util_cds_request_done(iface_t *iface, iface_session_t *session,
  isp_cds_request_t *isp_cds_request);
void iface_util_send_hw_stream_output_dim_downstream_int(iface_t *iface,
  iface_session_t *session, iface_hw_stream_t *hw_stream);
int iface_util_request_frame(iface_session_t *session,
  iface_t *iface, iface_param_frame_request_t *frame_request);
iface_cds_status_t iface_util_calc_cds_trigger(iface_t *iface,
  iface_session_t *session, stats_update_t *stats_update, uint32_t *cds_enable);
int iface_util_set_chromatix(iface_t *iface, iface_session_t *session,
  uint32_t stream_id, modulesChromatix_t *chromatix_param);
int iface_util_set_cds_mode(iface_t *iface, uint32_t session_id,
  cam_cds_mode_type_t *cds_mode);
boolean iface_util_is_dual_vfe_used_per_stream(iface_session_t *session,
  iface_hw_stream_t *hw_stream);
int iface_util_start_isp_pipeline_cfg(iface_t *iface,
  iface_offline_isp_info_t *offline_info, iface_fetch_engine_cmd_t
  fetch_cfg_cmd, iface_session_t *session, uint32_t hw_stream_id);
int iface_offline_stream_config(iface_t *iface, iface_port_t *iface_sink_port,
  uint32_t stream_id, iface_session_t *session, iface_offline_isp_info_t
  *offline_info);
int iface_offline_stream_unconfig(iface_t *iface, uint32_t hw_stream_id,
    uint32_t session_id);
int iface_offline_start_fetch_engine(iface_t *iface,
  iface_session_t *session,uint32_t hw_stream_id,
  uint32_t buf_stream_id, iface_offline_isp_info_t *offline_info);
int iface_util_add_user_stream_to_hw_stream(iface_t *iface,
  iface_port_t *iface_sink_port, uint32_t session_id,
  uint32_t user_stream_id);
void iface_util_setloglevel(void);
boolean iface_util_setloglevel_frm_hal(uint32_t *halloglevel);
boolean iface_util_set_preferred_mapping(iface_t *iface, mct_event_t *event);
boolean iface_util_set_buf_allignment(iface_t *iface, mct_event_t *event);
int32_t iface_util_post_bus_msg(iface_session_t *session,
  mct_bus_msg_type_t type, void *data, uint32_t data_size);
int iface_post_control_sof_to_thread(iface_t *iface,
  iface_port_t *iface_sink_port, iface_session_t *session, uint32_t user_stream_id,
    mct_event_t *event);
uint32_t iface_util_find_number_skip(uint16_t skip_pattern);
uint16_t iface_util_circularShiftLeft(uint16_t value, int shift);
uint16_t iface_util_reverse_skip(uint16_t skip_pattern);

int iface_util_request_stream_mapping_info(iface_t *iface,
  iface_buf_alloc_t *buf_alloc_info,
  uint32_t session_id, uint32_t stream_id);
int iface_util_calculate_frame_length_for_native_buf(cam_dimension_t *dim,
  cam_format_t fmt, cam_stream_buf_plane_info_t *buf_planes,
    uint32_t buf_alignment, uint32_t min_stride, uint32_t min_scanline);
int iface_util_request_image_bufs(iface_t *iface,
  iface_session_t *session,
  cam_frame_len_offset_t *buf_info,
  enum msm_vfe_axi_stream_src axi_stream_src,
  boolean  need_adsp_heap, uint32_t num_additional_buffers);
static int iface_util_request_pp_divert(iface_t *iface,
  uint32_t session_id, uint32_t stream_id,
  pp_buf_divert_request_t *pp_divert_request);
int iface_util_set_dis_enable_flag(iface_t *iface, uint32_t session_id,
  int32_t *dis_enable_flag);
int iface_util_set_sensor_hdr(iface_t *iface, uint32_t session_id, uint32_t stream_id,
   cam_sensor_hdr_type_t *hdr_mode);
int iface_util_request_pp_meta_stream_info(iface_t *iface,
  pp_meta_stream_info_request_t *pp_stream_info,
  uint32_t session_id, uint32_t stream_id);

int iface_util_process_bundle_streamonoff_list(
    iface_session_t                     *session,
    iface_util_process_bundle_info_t    *bundle_info,
    boolean                              is_streamon);

int iface_util_process_mct_stream_for_shared_bufq(
    iface_t                             *iface,
    iface_port_t                        *iface_sink_port,
    iface_session_t                     *session,
    iface_util_process_bundle_info_t    *bundle_info,
    boolean                              is_streamon);
int iface_util_decide_frame_skip_pattern(iface_session_t *session,
   iface_port_t *iface_sink_port,
   enum msm_vfe_frame_skip_pattern *skip_pattern);
int iface_util_set_master_slave_info(iface_session_t *session,
  iface_t *iface);
int iface_util_save_input_dim(iface_t *iface, mct_event_t *event);
int iface_util_set_master_slave_sync(iface_t *iface, iface_session_t *session,
  uint32_t sync);
int iface_util_set_ispif_frame_drop(iface_t *iface,
  iface_session_t *session, iface_port_t *iface_sink_port, enum ispif_cfg_type_t cfg_type);
#ifdef VIDIOC_MSM_ISP_DUAL_HW_LPM_MODE
int iface_util_lpm_session(iface_t *iface, iface_session_t *session,
  bool lpm_mode);
#endif
int32_t iface_util_get_buffers_id(iface_hw_stream_t *hw_stream,
  uint32_t stream_id);

#endif /* __IFACE_UTIL_H__ */
