/*============================================================================

Copyright (c) 2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "chromatix_common.h"
#include "chromatix_3a.h"

static chromatix_VFE_common_type chromatix_s5k3m2xm_parms = {
#include "chromatix_s5k3m2xm_common_bear.h"
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
