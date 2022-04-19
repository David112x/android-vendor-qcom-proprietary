
/*============================================================================

Copyright (c) 2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "chromatix_swpostproc.h"

static chromatix_sw_postproc_type chromatix_imx214_parms = {
#include "chromatix_imx214_sw_post_proc.h"
};

/*============================================================================
 * FUNCTION    - load_chromatix -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *load_chromatix(void)
{
  return &chromatix_imx214_parms;
}
