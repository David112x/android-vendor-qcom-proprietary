/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "wb/wb46/wb46.h"
#include "wb/wb46/wb46_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - wb_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void wb_populate_data_cfg0(uint32_t *reg, ISP_WhiteBalanceConfig0_CmdType *pcmd)
{
  reg += (ISP_CLF_WB46_CFG_0/4);
  memcpy((void *)pcmd, (void *)reg, sizeof(ISP_WhiteBalanceConfig0_CmdType));
} /*wb_populate_data*/

void wb_populate_data_cfg1(uint32_t *reg, ISP_WhiteBalanceConfig1_CmdType *pcmd)
{
  reg += (ISP_CLF_WB46_CFG_1/4);
  memcpy((void *)pcmd, (void *)reg, sizeof(ISP_WhiteBalanceConfig1_CmdType));
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
  ISP_WhiteBalanceConfig0_CmdType in_0, out_0;
  ISP_WhiteBalanceConfig1_CmdType in_1,out_1;
  vfe_test_module_input_t* input = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t* output = (vfe_test_module_output_t *)test_output;

  wb_populate_data_cfg0(input->reg_dump, &in_0);
  wb_populate_data_cfg0(output->reg_dump, &out_0);
  wb_populate_data_cfg1(input->reg_dump, &in_1);
  wb_populate_data_cfg1(output->reg_dump, &out_1);

  VALIDATE_TST_VEC(in_0.ch0Gain, out_0.ch0Gain, 0, "ch0Gain");
  VALIDATE_TST_VEC(in_0.ch1Gain, out_0.ch1Gain, 0, "ch1Gain");
  VALIDATE_TST_VEC(in_1.ch2Gain, out_1.ch2Gain, 0, "ch2Gain");

  return rc;
} /*vfe_wb_tv_validate*/
