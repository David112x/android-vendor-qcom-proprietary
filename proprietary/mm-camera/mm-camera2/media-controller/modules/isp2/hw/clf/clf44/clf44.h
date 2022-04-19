/* clf44.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __CLF44_H__
#define __CLF44_H__

/* isp headers */
#include "module_clf44.h"
#include "isp_sub_module_common.h"

boolean clf44_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void clf44_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean clf44_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf44_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf44_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf44_trigger_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf44_reset(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf44_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf44_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf44_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf44_query_cap(mct_module_t *module,
  void *query_buf);

#endif //__CLF44_H__
