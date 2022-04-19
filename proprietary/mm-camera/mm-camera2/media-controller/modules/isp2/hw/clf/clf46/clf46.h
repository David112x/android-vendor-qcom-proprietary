/* clf46.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __CLF46_H__
#define __CLF46_H__

/* isp headers */
#include "module_clf46.h"
#include "isp_sub_module_common.h"

boolean clf46_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void clf46_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean clf46_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf46_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf46_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf46_trigger_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf46_reset(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf46_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf46_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf46_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean clf46_query_cap(mct_module_t *module,
  void *query_buf);

#endif //__CLF46_H__
