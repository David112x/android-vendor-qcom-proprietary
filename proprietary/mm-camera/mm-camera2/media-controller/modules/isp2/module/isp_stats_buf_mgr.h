/* isp_stats_buf_mgr.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ISP_STATS_BUF_MGR_H__
#define __ISP_STATS_BUF_MGR_H__

boolean isp_stats_buf_mgr_init(isp_stats_buf_mgr_t *buf_mgr,
  iface_resource_request_t *resource_request,
  boolean aec_bg_hdr_be_swapped);

boolean isp_stats_buf_mgr_deinit(isp_stats_buf_mgr_t *buf_mgr);

mct_event_stats_isp_t *isp_stats_buf_mgr_get_buf(isp_stats_buf_mgr_t *buf_mgr,
  uint32_t stats_mask);

boolean isp_stats_buf_mgr_put_buf(isp_stats_buf_mgr_t *buf_mgr,
  mct_event_stats_isp_t *stats_event_data);

boolean isp_bundled_stats_buf_init(isp_bundled_stats_buf_mgr_t *buf_mgr);

boolean isp_bundled_stats_buf_deinit(isp_bundled_stats_buf_mgr_t *buf_mgr);

isp_bundled_stats_buf_info_params_t *isp_bundled_stats_buf_mgr_get_buf(
  isp_bundled_stats_buf_mgr_t *buf_mgr);

boolean isp_bundled_stats_buf_mgr_put_buf(isp_bundled_stats_buf_mgr_t *buf_mgr,
  isp_bundled_stats_buf_info_params_t *bundled_stats_info);

#endif
