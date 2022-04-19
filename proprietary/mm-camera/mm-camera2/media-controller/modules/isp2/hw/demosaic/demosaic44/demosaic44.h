/* demosaic44.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __DEMOSAIC44_H__
#define __DEMOSAIC44_H__

/* isp headers */
#include "module_demosaic44.h"
#include "isp_sub_module_common.h"

boolean demosaic44_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void demosaic44_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean demosaic44_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic44_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic44_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic44_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic44_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic44_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic44_trigger_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demosaic44_reset(mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);

boolean demosaic44_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

#endif /* __DEMOSAIC44_H__ */
