/* mce40.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MCE40_H__
#define __MCE40_H__

/* mctl headers */
#include "eztune_vfe_diagnostics.h"

/* isp headers */
#include "module_mce40.h"
#include "isp_sub_module_common.h"

boolean mce40_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

void mce40_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

boolean mce40_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean mce40_stats_asd_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean mce40_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean mce40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean mce40_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
#endif /*__MCE40_H_*/
