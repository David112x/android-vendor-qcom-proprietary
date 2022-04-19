/* wb40.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __WB40_H__
#define __WB40_H__

/* mctl headers */
#include "chromatix.h"

/* isp headers */
#include "isp_sub_module_common.h"
#include "isp_defs.h"
#include "wb_reg.h"

#define WB_GAIN(x) FLOAT_TO_Q(7, (x))

#define WB_GAIN_EQUAL(g1, g2) ( \
  F_EQUAL(g1->r_gain, g2.r_gain) && \
  F_EQUAL(g1->g_gain, g2.g_gain) && \
  F_EQUAL(g1->b_gain, g2.b_gain))

#define WB_GAIN_EQ_ZERO(g1) ( \
  F_EQUAL(g1->r_gain, 0.0) || \
  F_EQUAL(g1->g_gain, 0.0) || \
  F_EQUAL(g1->b_gain, 0.0))

/** wb40_t:
 *
 *  @ISP_WhiteBalanceCfgCmd: WB register setting
 *  @ISP_WhiteBalanceRightCfgCmd: WB register setting for
 *                              right
 *  @awb_gain: handle to store AWB gain
 *  @dig_gain: handle to store digital gain
 *  @old_streaming_mode: old streaming mode
 **/
typedef struct  {
  ISP_WhiteBalanceConfigCmdType       ISP_WhiteBalanceCfgCmd;
  ISP_WhiteBalanceRightConfigCmdType  ISP_WhiteBalanceRightCfgCmd;
  chromatix_manual_white_balance_type awb_gain;
  float                               dig_gain;
  cam_streaming_mode_t                old_streaming_mode;
  awb_update_t                        awb_update;
  boolean                             is_rccb;
} wb40_t;

boolean wb40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean wb40_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean wb40_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean wb40_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean wb40_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean wb40_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void wb40_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean wb40_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);

#endif //__WB40_H__
