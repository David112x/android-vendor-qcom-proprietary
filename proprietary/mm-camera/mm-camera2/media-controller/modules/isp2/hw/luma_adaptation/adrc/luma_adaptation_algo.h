/* luma_adaptation_algo.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __LUMA_ADAPTATION40_ALGO__
#define __LUMA_ADAPTATION40_ALGO__

#include "modules.h"
#include "isp_sub_module_common.h"

boolean la_update_adrc_la_curve(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
#endif /* __LUMA_ADAPTATION40_ALGO__ */
