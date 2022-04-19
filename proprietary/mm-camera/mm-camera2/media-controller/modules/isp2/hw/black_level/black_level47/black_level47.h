/* black_level47.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BLACK_LEVEL47_H__
#define __BLACK_LEVEL47_H__

/* mctl headers */
#include "chromatix.h"

/* isp headers */
#include "black_level47_reg.h"
#include "isp_sub_module_common.h"

/** black_level47_t:
 *
 *  @ISP_black_level_reg_t: Black Level registers
 *  @trigger_index: trigger index
 *  @aec_ratio: aec ratio
 *  @chromatix_channel_balance_gains_type: demux green gain to
 *                                       program black level
 *                                       scale
 **/
typedef struct {
  ISP_black_level_reg_t                 Reg;
  float                                 aec_ratio;
  uint8_t                               trigger_index;
  chromatix_channel_balance_gains_type  gain;
  float                                 dig_gain;
  aec_update_t                          aec_update;
} black_level47_t;

boolean black_level47_init(mct_module_t *module,
                           isp_sub_module_t *isp_sub_module);

void black_level47_destroy(mct_module_t *module,
                           isp_sub_module_t *isp_sub_module);

boolean black_level47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean black_level47_stats_aec_update(isp_sub_module_t *isp_sub_module, void *data);

boolean black_level47_manual_aec_update(isp_sub_module_t *isp_sub_module, void *data);

boolean black_level47_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data);

boolean black_level47_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean black_level47_fetch_blklvl_offset(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean black_level47_set_digital_gain(isp_sub_module_t *isp_sub_module,
  float *new_dig_gain);

boolean black_level47_set_hdr_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

#endif /* __BPC44_H__ */
