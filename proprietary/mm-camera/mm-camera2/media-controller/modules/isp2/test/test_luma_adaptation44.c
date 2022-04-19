/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
/* to change to luma_adaptation44.h */
#include "luma_adaptation/luma_adaptation40/luma_adaptation40.h"
#include "luma_adaptation/luma_adaptation40/luma_adaptation40_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - vfe_la_tv_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
int vfe_la_tv_validate(void *test_input, void *test_output)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)test_output;

  uint32_t i, ip, op;
  CDBG_ERROR("%s, size of LA = %d \n",__func__, mod_in->la.size);
  for(i = 0; i < mod_in->la.size; i++) {
    ip = mod_in->la.table[i] & 0x0000ffff;
    op = mod_in->la.table[i] & 0x0000ffff;
    VALIDATE_TST_LUT(ip, op, 3, "LA", i);
    ip = mod_in->la.table[i] & 0xffff0000;
    op = mod_in->la.table[i] & 0xffff0000;
    VALIDATE_TST_LUT(ip, op, 3, "LA", i);
  }
  return 0;
} /* vfe_la_tv_validate*/
