/* demosaic47.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __DEMOSAIC47_H__
#define __DEMOSAIC47_H__

/* isp headers */
#include "module_demosaic47.h"
#include "isp_sub_module_common.h"

boolean demosaic47_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void demosaic47_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean demosaic47_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic47_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic47_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic47_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic47_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic47_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic47_trigger_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic47_reset(mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);

boolean demosaic47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic47_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

#endif /* __DEMOSAIC47_H__ */
