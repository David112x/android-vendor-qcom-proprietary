/* cac47.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __CAC47_H__
#define __CAC47_H__

/* isp headers */
#include "module_cac47.h"
#include "isp_sub_module_common.h"

boolean cac47_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

void cac47_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

boolean cac47_query_cap(mct_module_t *module,
  void *query_buf);

boolean cac47_handle_isp_private_event(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

void cac47_update_streaming_mode_mask(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, uint32_t streaming_mode_mask);

boolean cac47_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean cac47_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean cac47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean cac47_save_aec_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean cac47_decide_scaling_threshold(
  isp_sub_module_t *isp_sub_module, float scale_factor);

boolean cac47_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean cac47_set_scaler_scale_factor(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean cac47_module_cfg(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean cac47_query_cap(mct_module_t *module,
  void *query_buf);

#endif /* __CAC47_H__ */
