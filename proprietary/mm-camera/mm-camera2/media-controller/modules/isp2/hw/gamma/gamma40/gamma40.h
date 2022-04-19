/* gamma40.h
 *
 * Copyright (c) 2012-2014, 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __GAMMA40_H__
#define __GAMMA40_H__

/* isp headers */
#include "module_gamma.h"
#include "isp_sub_module_common.h"

boolean gamma40_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

void gamma40_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

boolean gamma40_query_cap(mct_module_t *module,
  void *query_buf);

boolean gamma40_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma40_stats_asd_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma40_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma40_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma40_set_contrast(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma40_set_bestshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma40_set_spl_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma40_get_interpolated_table(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
#endif /* __GAMMA40_H__ */
