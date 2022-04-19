/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __IFACE_AXI_UTIL_H__
#define __IFACE_AXI_UTIL_H__

iface_axi_stream_t *iface_axi_util_find_stream(iface_axi_hw_t *axi, uint32_t session_id,
  uint32_t stream_id);
iface_axi_stream_t *iface_axi_util_find_stream_handle(iface_axi_hw_t *axi_hw,
  uint32_t handle);
uint32_t iface_axi_util_cam_fmt_to_v4l2_fmt(
  cam_format_t fmt, uint32_t uv_subsample);
int iface_axi_stream_request_frame(iface_axi_hw_t *axi,
  iface_param_frame_request_t *frame_request, uint32_t in_params_size);
static uint32_t iface_axi_util_calculate_output_width(iface_axi_stream_t *stream);
int iface_axi_subscribe_v4l2_event_multi(iface_axi_hw_t *axi_hw, boolean subscribe);
int iface_axi_prepare_sw_frame_skip_info(iface_axi_hw_t *axi,
  iface_param_frame_skip_pattern_t *skip_pattern, struct msm_vfe_axi_stream_update_cmd *cmd);
int iface_stats_prepare_sw_frame_skip_info(iface_axi_hw_t *axi,
  iface_param_frame_skip_pattern_t *skip_pattern,
  struct msm_vfe_axi_stream_update_cmd *update_cmd);
int iface_axi_util_subscribe_v4l2_event(iface_axi_hw_t *axi_hw, uint32_t event_type,
  boolean subscribe);
int iface_axi_util_fill_plane_info(iface_axi_hw_t *axi_hw,
  struct msm_vfe_axi_plane_cfg *planes, iface_axi_stream_t *axi_stream);

#endif /* __IFACE_AXI_UTIL_H__ */
