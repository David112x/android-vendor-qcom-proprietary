/* wb46.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __WB46_H__
#define __WB46_H__

/* mctl headers */
#include "chromatix.h"

/* isp headers */
#include "isp_sub_module_common.h"
#include "isp_defs.h"
#include "wb46_reg.h"

#define WB_GAIN(x) FLOAT_TO_Q(7, (x))

#define WB_GAIN_EQUAL(g1, g2) ( \
  F_EQUAL(g1->r_gain, g2.r_gain) && \
  F_EQUAL(g1->g_gain, g2.g_gain) && \
  F_EQUAL(g1->b_gain, g2.b_gain))

#define WB_GAIN_EQ_ZERO(g1) ( \
  F_EQUAL(g1->r_gain, 0.0) || \
  F_EQUAL(g1->g_gain, 0.0) || \
  F_EQUAL(g1->b_gain, 0.0))

/** wb46_t:
 *
 *  @ISP_WhiteBalanceCfgCmd: WB register setting
 *  @ISP_WhiteBalanceRightCfgCmd: WB register setting for
 *                              right
 *  @awb_gain: handle to store AWB gain
 *  @dig_gain: handle to store digital gain
 *  @old_streaming_mode: old streaming mode
 **/
typedef struct  {
  ISP_WhiteBalanceConfig0_CmdType       ISP_WhiteBalanceCfg0_Cmd;
  ISP_WhiteBalanceConfig1_CmdType       ISP_WhiteBalanceCfg1_Cmd;
  ISP_WhiteBalanceOffsetCmdType       ISP_WhiteBalanceOffsetCmd;
  ISP_WhiteBalanceRightConfig0_CmdType  ISP_WhiteBalanceRightCfg0_Cmd;
  ISP_WhiteBalanceRightConfig1_CmdType  ISP_WhiteBalanceRightCfg1_Cmd;
  ISP_WhiteBalanceOffsetCmdType  ISP_WhiteBalanceRightOffsetCmd;
  chromatix_manual_white_balance_type awb_gain;
  awb_update_t                        awb_update;
  float                               dig_gain;
  cam_streaming_mode_t                old_streaming_mode;
} wb46_t;

boolean wb46_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean wb46_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean wb46_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean wb46_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean wb46_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean wb46_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void wb46_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

#endif //__WB46_H__
