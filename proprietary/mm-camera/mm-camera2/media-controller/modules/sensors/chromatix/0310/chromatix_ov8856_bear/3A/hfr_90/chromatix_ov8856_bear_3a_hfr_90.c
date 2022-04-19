/*============================================================================

Copyright (c) 2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "chromatix_common.h"
#include "chromatix_3a.h"


static chromatix_3a_parms_type chromatix_ov8856_bear_parms = {
#include "chromatix_ov8856_bear_3a_hfr_90.h"
};

/*============================================================================
 * FUNCTION    - load_chromatix -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *load_chromatix(void)
{

  return &chromatix_ov8856_bear_parms;
}
