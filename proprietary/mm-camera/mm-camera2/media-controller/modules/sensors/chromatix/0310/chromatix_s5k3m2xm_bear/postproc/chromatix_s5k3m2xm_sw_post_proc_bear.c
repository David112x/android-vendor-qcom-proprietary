
/*============================================================================

Copyright (c) 2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "chromatix_swpostproc.h"

static chromatix_sw_postproc_type chromatix_s5k3m2xm_parms = {
#include "chromatix_s5k3m2xm_sw_post_proc_bear.h"
};

/*============================================================================
 * FUNCTION    - load_chromatix -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *load_chromatix(void)
{
  return &chromatix_s5k3m2xm_parms;
}
