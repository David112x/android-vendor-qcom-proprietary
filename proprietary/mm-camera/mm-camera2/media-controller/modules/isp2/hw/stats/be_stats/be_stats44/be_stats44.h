/* be_stats44.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BE_STATS_44_H__
#define __BE_STATS_44_H__

/* mctl headers */
#include "modules.h"
#include "mct_event_stats.h"

/* isp headers */
#include "be_stats_reg44.h"
#include "isp_sub_module_common.h"

/** be_stats44_t:
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
  ISP_StatsBe_CfgCmdType     pcmd;
  ispif_out_info_t           ispif_out_info;
  isp_out_info_t             isp_out_info;
  aec_config_t               aec_config;
  sensor_out_info_t          sensor_out_info;
  uint32_t                   left_stipe_rgn_offset;
  uint32_t                   num_left_rgns;
  uint32_t                   num_right_rgns;
} be_stats44_t;

boolean be_stats44_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean be_stats44_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean be_stats44_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean be_stats44_init(isp_sub_module_t *isp_sub_module);

void be_stats44_destroy(isp_sub_module_t *isp_sub_module);

boolean be_stats44_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);

boolean be_stats44_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean be_stats44_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data);

#endif /* __BE_STATS_44_H__ */
