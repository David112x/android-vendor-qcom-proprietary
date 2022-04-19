/* hdr_be_stats46.h
 *
 * Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __HDR_BE_STATS_46_H__
#define __HDR_BE_STATS_46_H__

/* mctl headers */
#include "modules.h"
#include "mct_event_stats.h"

/* isp headers */
#include "hdr_be_stats_reg.h"
#include "isp_sub_module_common.h"

/** hdr_be_stats46_t:
 *
 *  @pcmd: hardware configuration
 *  @ispif_out_info: ispif out info
 *  @isp_out_info: isp out info
 *  @aec_config: aec config params
 *  @sensor_out_info: sensor out info
 *  @num_left_rgns: number of left regions
 *  @num_right_rgns: number of right regions
 **/

#define PIX_VALUE_OFFSET    1

typedef struct {
  ISP_StatsHdrBe_CfgCmdType  pcmd;
  ispif_out_info_t           ispif_out_info;
  isp_out_info_t             isp_out_info;
  aec_config_t               aec_config;
  sensor_out_info_t          sensor_out_info;
  uint32_t                   left_stipe_rgn_offset;
  uint32_t                   num_left_rgns;
  uint32_t                   num_right_rgns;
} hdr_be_stats46_t;

boolean hdr_be_stats46_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean hdr_be_stats46_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean hdr_be_stats46_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean hdr_be_stats46_init(isp_sub_module_t *isp_sub_module);

void hdr_be_stats46_destroy(isp_sub_module_t *isp_sub_module);

boolean hdr_be_stats46_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);

boolean hdr_be_stats46_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean hdr_be_stats46_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean hdr_be_stats_ext46_hw_update(isp_sub_module_t *isp_sub_module,
  hdr_be_stats46_t *be_stats);

#endif /* __HDR_BE_STATS_46_H__ */
