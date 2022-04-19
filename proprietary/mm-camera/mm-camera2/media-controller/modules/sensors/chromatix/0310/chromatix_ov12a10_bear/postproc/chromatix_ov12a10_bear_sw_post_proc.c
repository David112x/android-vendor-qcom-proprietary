
/*============================================================================

 Copyright (c) 2017 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "chromatix_swpostproc.h"

static chromatix_sw_postproc_type chromatix_ov12a10_bear_parms = {
#include "chromatix_ov12a10_bear_sw_post_proc.h"
};

/*============================================================================
 * FUNCTION    - load_chromatix -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *load_chromatix(void)
{
  return &chromatix_ov12a10_bear_parms;
}
