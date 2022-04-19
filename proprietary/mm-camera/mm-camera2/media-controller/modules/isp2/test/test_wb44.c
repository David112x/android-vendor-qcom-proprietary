/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "wb/wb40/wb40.h"
#include "wb/wb40/wb40_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - wb_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void wb_populate_data(uint32_t *reg, ISP_WhiteBalanceConfigCmdType *pcmd)
{
  reg += (ISP_WB40_OFF/4);
  memcpy((void *)pcmd, (void *)reg, sizeof(ISP_WhiteBalanceConfigCmdType));
} /*wb_populate_data*/

/*===========================================================================
 * FUNCTION    - vfe_wb_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_wb_tv_validate(void *test_input,
  void *test_output)
{
  int rc = 0;
  ISP_WhiteBalanceConfigCmdType in, out;
  vfe_test_module_input_t* input = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t* output = (vfe_test_module_output_t *)test_output;

  wb_populate_data(input->reg_dump, &in);
  wb_populate_data(output->reg_dump, &out);

  VALIDATE_TST_VEC(in.ch0Gain, out.ch0Gain, 0, "ch0Gain");
  VALIDATE_TST_VEC(in.ch1Gain, out.ch1Gain, 0, "ch1Gain");
  VALIDATE_TST_VEC(in.ch2Gain, out.ch2Gain, 0, "ch2Gain");

  return rc;
} /*vfe_wb_tv_validate*/

