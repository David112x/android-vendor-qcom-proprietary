/* bhist_stats44.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BHIST_STATS_44_H__
#define __BHIST_STATS_44_H__

/* mctl headers */
#include "modules.h"
#include "mct_event_stats.h"

/* isp headers */
#include "bhist_stats_reg.h"
#include "isp_sub_module_common.h"

/** isp_stats_grid_t:
 *
 *  @h_num: h num
 *  @v_num: v num
 **/
typedef struct {
  uint32_t h_num;
  uint32_t v_num;
} isp_stats_grid_t;

/** bhist_stats44_t:
 *
 *  @pcmd: hardware configuration
 *  @ispif_out_info: ispif out info
 *  @isp_out_info: isp out info
 *  @bhist_config: bhist config
 *  @num_left_rgns: number of left regions
 *  @num_right_rgns: number of right regions
 *  @grid_info: grid info
 *  @roi: roi
 *  @meta_enable: enable/disable bhist stats in meta info
 **/
typedef struct {
  ISP_StatsBhist_CfgCmdType pcmd;
  ispif_out_info_t          ispif_out_info;
  isp_out_info_t            isp_out_info;
  aec_bhist_config_t        bhist_config;
  uint32_t                  num_left_rgns;
  uint32_t                  num_right_rgns;
  isp_stats_grid_t          grid_info;
  cam_rect_t                roi;
  int32_t                  camif_window_width;
  int32_t                  camif_window_height;
  int32_t                  meta_enable;
} bhist_stats44_t;

boolean bhist_stats44_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bhist_stats44_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bhist_stats44_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean bhist_stats44_init(isp_sub_module_t *isp_sub_module);

void bhist_stats44_destroy(isp_sub_module_t *isp_sub_module);

boolean bhist_stats44_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bhist_stats44_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bhist_stats44_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bhist_stats_ext44_hw_update(isp_sub_module_t *isp_sub_module,
  bhist_stats44_t *bhist_stats);

boolean bhist_stats44_set_meta_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

#endif /* __BHIST_STATS_44_H__ */
