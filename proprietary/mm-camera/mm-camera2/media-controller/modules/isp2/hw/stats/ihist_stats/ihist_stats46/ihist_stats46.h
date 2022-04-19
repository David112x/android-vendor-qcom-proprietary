/* ihist_stats46.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __IHIST_STATS_46_H__
#define __IHIST_STATS_46_H__

/* mctl headers */
#include "modules.h"
#include "mct_event_stats.h"

/* isp headers */
#include "ihist_stats_reg.h"
#include "isp_sub_module_common.h"

/** ihist_stats46_t:
 *
 *  @pcmd: hardware configuration
 *  @ispif_out_info: ispif out info
 *  @isp_out_info: isp out info
 *  @af_config: af config params
 *  @sensor_out_info: sensor out info
 *  @num_left_rgns: number of left regions
 *  @num_right_rgns: number of right regions
 **/
typedef struct {
  ISP_StatsIhist_CfgType iHist_pcmd;
  ISP_Stats_CfgType      init_pcmd;
  ispif_out_info_t       ispif_out_info;
  isp_out_info_t         isp_out_info;
  sensor_out_info_t      sensor_out_info;
  cam_rect_t             roi;
  uint32_t               num_left_rgns;
  uint32_t               num_right_rgns;
} ihist_stats46_t;

boolean ihist_stats46_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean ihist_stats46_streamoff(isp_sub_module_t *isp_sub_module,
  void *data);

boolean ihist_stats46_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean ihist_stats46_init(isp_sub_module_t *isp_sub_module);

void ihist_stats46_destroy(isp_sub_module_t *isp_sub_module);

boolean ihist_stats46_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);

boolean ihist_stats46_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean ihist_stats46_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean ihist_stats46_stats_config_validate(ihist_stats46_t *ihist_stats,
  af_config_t *af_config);
#endif /* __IHIST_STATS_46_H__ */
