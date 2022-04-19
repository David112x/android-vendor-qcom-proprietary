/* rccb42.h
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef __RCCB42_H__
#define __RCCB42_H__

/* mctl headers */
#include "chromatix.h"

/* isp headers */
#include "isp_sub_module_common.h"
#include "isp_defs.h"
#include "rccb_reg.h"

#define WB_GAIN(x) FLOAT_TO_Q(7, (x))

#define WB_GAIN_EQUAL(g1, g2) ( \
  F_EQUAL(g1->r_gain, g2.r_gain) && \
  F_EQUAL(g1->g_gain, g2.g_gain) && \
  F_EQUAL(g1->b_gain, g2.b_gain))

#define WB_GAIN_EQ_ZERO(g1) ( \
  F_EQUAL(g1->r_gain, 0.0) || \
  F_EQUAL(g1->g_gain, 0.0) || \
  F_EQUAL(g1->b_gain, 0.0))

/* rccb42_t
 *
 * r_gain: Red pixel gain
 * b_gain: blue pixel gain
 * g_gain: green pixel gain
 * r_offset: red pixel wb offset
 * b_offset: blue pixel wb offset
 * g_offset: green pixel wb offset
 * awb_gain: manual awb gain
 * dig_gain: residue digital gai needs
 *           to be applied by ISP
 * old_streaming_mode: streaming mode
 * awb_update: curren awb update
 */

typedef struct  {
  ISP_RccbWbGain                      r_gain;
  ISP_RccbWbGain                      b_gain;
  ISP_RccbWbGain                      g_gain;
  ISP_RccbWbOffset                    r_offset;
  ISP_RccbWbOffset                    b_offset;
  ISP_RccbWbOffset                    g_offset;
  chromatix_manual_white_balance_type awb_gain;
  float                               dig_gain;
  cam_streaming_mode_t                old_streaming_mode;
  awb_update_t                        awb_update;
  boolean                             is_rccb;
} rccb42_t;

boolean rccb42_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean rccb42_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean rccb42_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean rccb42_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean rccb42_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean rccb42_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void rccb42_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean rccb42_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);
#endif //__RCCB42_H__
