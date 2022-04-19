/*============================================================================

//  Copyright (c) 2016 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "chromatix_common.h"
#include "chromatix_3a.h"

static chromatix_3a_parms_type chromatix_imx230_parms = {
#include "chromatix_imx230_raw_hdr_lc898212xd.h"
};

/*============================================================================
 * FUNCTION    - load_chromatix -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *load_chromatix(void)
{
  return &chromatix_imx230_parms;
}
