/* hdr46.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __HDR46_H__
#define __HDR46_H__

/* isp headers */
#include "module_hdr46.h"
#include "isp_sub_module_common.h"

boolean hdr46_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

void hdr46_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

boolean hdr46_handle_isp_private_event(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

void hdr46_update_streaming_mode_mask(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, uint32_t streaming_mode_mask);

boolean hdr46_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean hdr46_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean hdr46_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean hdr46_save_aec_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean hdr46_save_awb_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

#endif /* __HDR46_H__ */
