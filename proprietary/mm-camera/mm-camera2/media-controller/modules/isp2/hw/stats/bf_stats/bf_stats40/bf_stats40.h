/* bf_stats40.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BF_STATS_40_H__
#define __BF_STATS_40_H__

/* mctl headers */
#include "modules.h"
#include "mct_event_stats.h"

/* isp headers */
#include "bf_stats_reg.h"
#include "isp_sub_module_common.h"

/** bf_stats40_t:
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
  ISP_StatsBf_CfgCmdType pcmd;
  ispif_out_info_t       ispif_out_info;
  isp_out_info_t         isp_out_info;
  af_config_t            af_config;
  sensor_out_info_t      sensor_out_info;
  uint32_t               num_left_rgns;
  uint32_t               num_right_rgns;
  cam_dimension_t        camif;
} bf_stats40_t;

boolean bf_stats40_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_stats40_stats_config_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_stats40_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean bf_stats40_init(isp_sub_module_t *isp_sub_module);

void bf_stats40_destroy(isp_sub_module_t *isp_sub_module);

boolean bf_stats40_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_stats40_update_min_stripe_overlap(
  isp_sub_module_t *isp_sub_module, void *data);

boolean bf_stats40_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_stats40_set_stripe_info(isp_sub_module_t *isp_sub_module,
  void *data);

boolean bf_stats40_get_stats_capabilities(isp_sub_module_t *isp_sub_module,
  void *data);

#endif /* __BF_STATS_40_H__ */
