
/*============================================================================

Copyright (c) 2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/

/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "chromatix_swpostproc.h"

static chromatix_sw_postproc_type chromatix_s5k3p8sp_parms = {
#include "chromatix_s5k3p8sp_sw_post_proc.h"
};

/*============================================================================
 * FUNCTION    - load_chromatix -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *load_chromatix(void)
{
  return &chromatix_s5k3p8sp_parms;
}
