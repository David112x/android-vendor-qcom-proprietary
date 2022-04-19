/* color_correct40.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __COLOR_CORRECT40_H__
#define __COLOR_CORRECT40_H__

/* isp headers */
#include "module_color_correct40.h"
#include "isp_sub_module_common.h"

boolean color_correct40_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

void color_correct40_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

boolean color_correct40_handle_isp_private_event(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

void color_correct40_update_streaming_mode_mask(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, uint32_t streaming_mode_mask);

static void color_correct40_interpolate(chromatix_color_correction_type *in1,
  chromatix_color_correction_type  *in2, chromatix_color_correction_type *out, float ratio,float gain);

boolean color_correct40_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct40_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct40_trigger_update_aec(color_correct40_t *mod,
  chromatix_color_correction_type *tblIn, chromatix_color_correction_type *tblOut,
  chromatix_parms_type *chromatix_ptr);

boolean color_correct40_trigger_update_awb(color_correct40_t *mod,
  chromatix_color_correction_type *tbl_out, chromatix_parms_type *chromatix_ptr);

boolean color_correct40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct40_save_awb_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct40_save_aec_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct40_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct40_set_bestshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct40_set_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean color_correct_set_spl_effect(isp_sub_module_t *isp_sub_module,
  color_correct40_t *mod, cam_effect_mode_type *spl_effects);

boolean color_correct40_set_flash_mode(isp_sub_module_t *isp_sub_module,
  void *data);

#endif /* __COLOR_CORRECT40_H__ */
