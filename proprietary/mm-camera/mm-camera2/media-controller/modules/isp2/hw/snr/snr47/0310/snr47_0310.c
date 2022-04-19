/*============================================================================

  Copyright (c) 2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "snr47.h"

/** snr47_decide_hysterisis:
 *
 * @isp_sub_module: sub module handle
 * @aec_ref: AEC reference
 *
 * Handle hysterisis
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean snr47_decide_hysterisis(isp_sub_module_t *isp_sub_module,
   float aec_ref)
{
  chromatix_parms_type       *chromatix_ptr = NULL;
  Chromatix_Skintone_NR_type *chromatix_SNR = NULL;

  if (isp_sub_module == NULL) {
    ISP_ERR("failed: NULL pointer %p", isp_sub_module);
    return FALSE;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_SNR =
    &chromatix_ptr->chromatix_VFE.chromatix_skin_nr_data;

  return isp_sub_module_util_decide_hysterisis(isp_sub_module,
    aec_ref, chromatix_SNR->skintone_nr_en,
    FALSE, chromatix_SNR->control_skintone_nr,
    &chromatix_SNR->skinnr_hysteresis_trigger);
}
