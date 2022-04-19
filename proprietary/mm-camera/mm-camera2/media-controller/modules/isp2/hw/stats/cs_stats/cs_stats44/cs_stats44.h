/* cs_stats44.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __CS_STATS_44_H__
#define __CS_STATS_44_H__

/* mctl headers */
#include "modules.h"
#include "mct_event_stats.h"

/* isp headers */
#include "cs_stats_reg44.h"
#include "isp_sub_module_common.h"

/** cs_stats44_t:
 *
 *  @pcmd: hardware configuration
 *  @ispif_out_info: ispif out info
 *  @isp_out_info: isp out info
 *  @sensor_out_info: sensor out info
 *  @num_left_rgns: number of left regions
 *  @num_right_rgns: number of right regions
 **/
typedef struct {
  ISP_StatsCs_CfgType pcmd;
  ispif_out_info_t    ispif_out_info;
  isp_out_info_t      isp_out_info;
  sensor_out_info_t   sensor_out_info;
  uint32_t            num_left_rgns;
  uint32_t            num_right_rgns;
} cs_stats44_t;

boolean cs_stats44_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean cs_stats44_fetch_cs_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean cs_stats44_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean cs_stats44_init(isp_sub_module_t *isp_sub_module);

void cs_stats44_destroy(isp_sub_module_t *isp_sub_module);

boolean cs_stats44_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);

boolean cs_stats44_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean cs_stats44_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data);
#endif /* __CS_STATS_44_H__ */
