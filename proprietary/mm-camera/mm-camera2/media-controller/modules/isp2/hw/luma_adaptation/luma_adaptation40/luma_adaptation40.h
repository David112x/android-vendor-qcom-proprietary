/* luma_adaptation40.h
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __LUMA_ADAPTATION40_H__
#define __LUMA_ADAPTATION40_H__

/* isp headers */
#include "module_luma_adaptation40.h"
#include "isp_sub_module_common.h"

boolean la40_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void la40_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean la40_handle_isp_private_event(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

void la40_update_streaming_mode_mask(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, uint32_t streaming_mode_mask);

boolean la40_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean la40_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean la40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean la40_save_aec_param(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean la40_save_asd_param(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean la40_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean la40_update_la_curve(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean la40_set_bestshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean la40_set_spl_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean la40_apply_bestshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, cam_scene_mode_type bestshot_mode);

boolean la40_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

#endif /* __LUMA_ADAPTATION40_H__ */
