/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __SNR47_H__
#define __SNR47_H__

#include "snr47_reg.h"
#include "isp_sub_module_common.h"

/* TODO pass from Android.mk */
#define SNR47_VERSION "47"

#define SNR_NAME(n) \
  "snr"n

typedef struct {
  uint32_t                  streaming_mode_mask;
  ISP_SnrCfg_CmdType        reg_cfgCmd;
  ISP_Snr_CmdType           reg_cmd;
  trigger_ratio_t           aec_ratio;
  uint8_t                   trigger_index;
  boolean                   cac_enable;
} snr47_t;

boolean snr47_init(isp_sub_module_t *isp_sub_module);

void snr47_destroy(isp_sub_module_t *isp_sub_module);

boolean snr47_aec_update(isp_sub_module_t *isp_sub_module,
  stats_update_t *stats_update);

boolean snr47_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  modulesChromatix_t *chromatix_ptrs);

boolean snr47_trigger_update(isp_sub_module_t *isp_sub_module,
  isp_private_event_t *private_event);

boolean snr47_streamon(isp_sub_module_t *isp_sub_module);

boolean snr47_streamoff(isp_sub_module_t *isp_sub_module);

void snr47_update_streaming_mode_mask(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, uint32_t streaming_mode_mask);

boolean snr47_update_submod_enable(isp_sub_module_t *isp_sub_module,
  isp_private_event_t *private_event);

boolean snr47_decide_hysterisis(isp_sub_module_t *isp_sub_module,
   float aec_ref);
#endif// __SNR47_H__
