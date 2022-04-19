/* color_correct46.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __COLOR_CORRECT46_H__
#define __COLOR_CORRECT46_H__

/* isp headers */
#include "module_color_correct46.h"
#include "isp_sub_module_common.h"

boolean color_correct46_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

void color_correct46_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

boolean color_correct46_handle_isp_private_event(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

void color_correct46_update_streaming_mode_mask(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, uint32_t streaming_mode_mask);

static void color_correct46_interpolate(color_correct46_param_t *in1,
  color_correct46_param_t  *in2, color_correct46_param_t *out, float ratio,float gain);

boolean color_correct46_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct46_set_chromatix(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct46_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct46_trigger_update_aec(color_correct46_t *mod,
  color_correct46_param_t *tblIn, color_correct46_param_t *tblOut,
  chromatix_parms_type *chromatix_ptr);

boolean color_correct46_trigger_update_awb(color_correct46_t *mod,
  color_correct46_param_t *tbl_out, chromatix_parms_type *chromatix_ptr);

boolean color_correct46_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct46_save_awb_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct46_save_aec_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct46_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct46_set_bestshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct46_set_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct_set_spl_effect(isp_sub_module_t *isp_sub_module,
  color_correct46_t *mod, cam_effect_mode_type *spl_effects);

boolean color_correct46_set_flash_mode(isp_sub_module_t *isp_sub_module,
  void *data);

#endif /* __COLOR_CORRECT46_H__ */
