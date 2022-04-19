/* chroma_enhan40.h
 *
 * Copyright (c) 2012-2014, 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __CHROMA_ENHAN40_H__
#define __CHROMA_ENHAN40_H__

/* isp headers */
#include "module_chroma_enhan40.h"
#include "isp_sub_module_common.h"

boolean chroma_enhan40_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void chroma_enhan40_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean chroma_enhan40_handle_isp_private_event(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

void chroma_enhan40_update_streaming_mode_mask(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, uint32_t streaming_mode_mask);

boolean chroma_enhan40_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean chroma_enhan40_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean chroma_enhan40_trigger_update_aec(chroma_enhan40_t *mod,
  chromatix_parms_type *chromatix_ptr);

boolean chroma_enhan40_trigger_update_awb(chroma_enhan40_t *mod,
  chromatix_parms_type *chromatix_ptr);

boolean chroma_enhan40_trigger_update_flash(chroma_enhan40_t *mod,
  chromatix_parms_type *chromatix_ptr);

boolean chroma_enhan40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean chroma_enhan40_save_awb_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean chroma_enhan40_save_aec_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean chroma_enhan40_set_flash_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean chroma_enhan40_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean chroma_enhan_set_bestshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean chroma_enhan_set_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean chroma_enhan_set_spl_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean chroma_enhan_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean chroma_enhance_apply_default_settings(isp_sub_module_t
  *isp_sub_module);

boolean chroma_enhan_apply_spl_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, cam_effect_mode_type effects);

void chroma_enhan_interpolate(
  color_conversion_type_t* in1, color_conversion_type_t *in2,
  color_conversion_type_t* out, float ratio);

boolean chroma_enhan_apply_bestshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, cam_scene_mode_type bestshot_mode);

void chroma_enhan40_copy_cv_data(
   color_conversion_type_t         *dst_cv_data,
   chromatix_color_conversion_type *src_cv_data);

#endif /* __CHROMA_ENHAN40_H__ */
