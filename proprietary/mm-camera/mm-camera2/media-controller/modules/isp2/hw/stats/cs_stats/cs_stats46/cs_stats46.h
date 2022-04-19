/* cs_stats46.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __CS_STATS_46_H__
#define __CS_STATS_46_H__

/* mctl headers */
#include "modules.h"
#include "mct_event_stats.h"

/* isp headers */
#include "cs_stats_reg.h"
#include "isp_sub_module_common.h"

/** cs_stats46_t:
 *
 *  @pcmd: hardware configuration
 *  @ispif_out_info: ispif out info
 *  @isp_out_info: isp out info
 *  @sensor_out_info: sensor out info
 *  @num_left_rgns: number of left regions
 *  @num_right_rgns: number of right regions
 **/
typedef struct {
  ISP_Stats_CfgType   init_pcmd;
  ISP_StatsCs_CfgType cs_pcmd;
  ispif_out_info_t    ispif_out_info;
  isp_out_info_t      isp_out_info;
  sensor_out_info_t   sensor_out_info;
  uint32_t            num_left_rgns;
  uint32_t            num_right_rgns;
} cs_stats46_t;

boolean cs_stats46_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean cs_stats46_fetch_cs_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean cs_stats46_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean cs_stats46_init(isp_sub_module_t *isp_sub_module);

void cs_stats46_destroy(isp_sub_module_t *isp_sub_module);

boolean cs_stats46_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);

boolean cs_stats46_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean cs_stats46_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data);
#endif /* __CS_STATS_46_H__ */
