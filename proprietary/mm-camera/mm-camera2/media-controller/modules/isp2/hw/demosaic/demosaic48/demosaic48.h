/* demosaic48.h
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __DEMOSAIC48_H__
#define __DEMOSAIC48_H__

/* isp headers */
#include "module_demosaic48.h"
#include "isp_sub_module_common.h"

boolean demosaic48_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void demosaic48_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean demosaic48_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic48_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic48_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic48_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic48_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic48_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic48_trigger_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic48_reset(mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);

boolean demosaic48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic48_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

#endif /* __DEMOSAIC48_H__ */
