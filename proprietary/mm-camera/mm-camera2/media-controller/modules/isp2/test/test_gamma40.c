/*============================================================================

 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "vfe_test_vector.h"
#include "gamma40.h"
#include "gamma40_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - vfe_gamma_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_gamma_tv_validate(void *test_input,
  void *test_output)
{
  int i = 0;
  uint32_t in, out;
  uint32_t in_delta, out_delta;
  uint32_t in_val, out_val;
  vfe_test_module_input_t* input = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t* output = (vfe_test_module_output_t *)test_output;
  uint32_t mask = input->gamma_r.mask;

#define TEST_TABLE(tab) \
  for (i = 0; i < ISP_GAMMA_NUM_ENTRIES; i++) { \
    in = input->tab.table[i] & mask; \
    out = output->tab.table[i] & mask; \
    in_delta = in >> GAMMA_HW_PACK_BIT; \
    out_delta = out >> GAMMA_HW_PACK_BIT; \
    in_val = in & GAMMA_HW_UNPACK_MASK; \
    out_val = out & GAMMA_HW_UNPACK_MASK; \
    VALIDATE_TST_LUT(in_delta, out_delta, 0, "gamma delta", i); \
    VALIDATE_TST_LUT(in_val, out_val, 0, "gamma value", i); \
  }
  TEST_TABLE(gamma_r);
  TEST_TABLE(gamma_b);
  TEST_TABLE(gamma_g);

  return 0;
}/*vfe_gamma_tv_validate*/
