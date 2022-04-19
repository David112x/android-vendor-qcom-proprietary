/*============================================================================

 Copyright (c) 2017 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "chromatix_common.h"

static chromatix_3a_parms_type chromatix_ov12a10_bear_parms = {
#include "chromatix_ov12a10_bear_4k_preview_3a.h"
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
