/* gamma_0310.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "module_gamma.h"
#include "isp_sub_module_util.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_pipeline_reg.h"
#include "gamma44.h"


/** gamma44_set_same_channel_data_flag:
 *
 *  This function sets same channel data flag
 *
 **/

boolean gamma44_set_same_channel_data_flag(isp_sub_module_t *isp_sub_module)
{
  chromatix_parms_type    *chromatix_ptr    = NULL;
  chromatix_gamma_type    *pchromatix_gamma = NULL;
  gamma44_t               *gamma            = NULL;
  if (!isp_sub_module) {
    ISP_ERR("failed: %p", isp_sub_module);
    return FALSE;
  }
  gamma = (gamma44_t *)isp_sub_module->private_data;
  if (!gamma) {
    ISP_ERR("failed: mod %p", gamma);
     return FALSE;
  }
  chromatix_ptr =
    isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  pchromatix_gamma =
    &(chromatix_ptr->chromatix_VFE.chromatix_gamma);
  if (!pchromatix_gamma) {
    ISP_ERR("failed: pchromatix_gamma %p", pchromatix_gamma);
    return FALSE;
  }

  if (pchromatix_gamma->same_channel_data_flag) {
    gamma->enable_same_data_channel = TRUE;
  } else {
    gamma->enable_same_data_channel = FALSE;
  }

  return TRUE;
}
