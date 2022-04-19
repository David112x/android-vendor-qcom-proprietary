/* aec_bg_stats47.h
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AEC_BG_STATS_47_H__
#define __AEC_BG_STATS_47_H__

/* mctl headers */
#include "modules.h"
#include "mct_event_stats.h"

/* isp headers */
#include "aec_bg_stats_reg.h"
#include "isp_sub_module_common.h"

#define PIX_VALUE_OFFSET    1

/** aec_bg_stats47_t:
 *
 *  @pcmd: hardware configuration
 *  @ispif_out_info: ispif out info
 *  @isp_out_info: isp out info
 *  @aec_config: aec config params
 *  @sensor_out_info: sensor out info
 *  @num_left_rgns: number of left regions
 *  @num_right_rgns: number of right regions
 **/
typedef struct {
  ISP_StatsAecBg_CfgCmdType  pcmd;
  ispif_out_info_t           ispif_out_info;
  isp_out_info_t             isp_out_info;
  aec_bg_config_t            aec_bg_config;
  sensor_out_info_t          sensor_out_info;
  uint32_t                   num_left_rgns;
  uint32_t                   num_right_rgns;
} aec_bg_stats47_t;

boolean aec_bg_stats47_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean aec_bg_stats47_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean aec_bg_stats47_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean aec_bg_stats47_init(isp_sub_module_t *isp_sub_module);

void aec_bg_stats47_destroy(isp_sub_module_t *isp_sub_module);

boolean aec_bg_stats47_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);

boolean aec_bg_stats47_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean aec_bg_stats47_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean aec_bg_stats_ext47_hw_update(isp_sub_module_t *isp_sub_module,
  aec_bg_stats47_t*be_stats);

#endif /* __AEC_BG_STATS_47_H__ */
