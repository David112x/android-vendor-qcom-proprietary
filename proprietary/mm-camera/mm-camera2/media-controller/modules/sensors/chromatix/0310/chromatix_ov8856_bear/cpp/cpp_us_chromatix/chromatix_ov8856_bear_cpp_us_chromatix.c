/*============================================================================
Copyright (c) 2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "chromatix_cpp.h"


static chromatix_cpp_type chromatix_ov8856_bear_parms = {
#include "chromatix_ov8856_bear_cpp_us_chromatix.h"
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
