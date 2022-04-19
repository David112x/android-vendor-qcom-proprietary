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
  if (!isp_sub_module || !aec_ref) {
    return FALSE;
  }
  return TRUE;
}
