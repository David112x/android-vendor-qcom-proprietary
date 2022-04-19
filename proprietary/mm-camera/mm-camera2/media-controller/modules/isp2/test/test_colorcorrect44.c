/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "color_correct/color_correct46/color_correct46.h"
#include "color_correct_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - color_correct_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void color_correct_populate_data(uint32_t *reg, ISP_ColorCorrectionCfgCmdType
  *pcmd)
{
  CDBG("%s: size %d", __func__, sizeof(ISP_ColorCorrectionCfgCmdType));
  reg += (ISP_COLOR_COR46_OFF/4);
  memcpy((void *)pcmd, reg, sizeof(ISP_ColorCorrectionCfgCmdType));
}/*color_correct_populate_data*/

/*===========================================================================
 * FUNCTION    - vfe_color_correct_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_color_correct_tv_validate(void *test_ip, void *test_op)
{
  vfe_test_module_input_t *input = (vfe_test_module_input_t *)test_ip;
  vfe_test_module_output_t *output = (vfe_test_module_output_t *)test_op;
  ISP_ColorCorrectionCfgCmdType in_cc, out_cc;
  uint32_t *data;
  int i;

  color_correct_populate_data(input->reg_dump, &in_cc);
    data = (uint32_t*)input->reg_dump;
#if 0//def DUMP_CC
  for (i = 0; i < ISP_COLOR_COR46_LEN; i++) {
    CDBG_ERROR("%s color correct input data[%i] = %x\n",__func__, i, data[i]);
  }
#endif
  color_correct_populate_data(output->reg_dump, &out_cc);
    data = (uint32_t*)output->reg_dump;
#if 0//def DUMP_CC
  for (i = 0; i < ISP_COLOR_COR46_LEN; i++) {
	  CDBG_ERROR("%s color correct output data[%i] = %x\n",__func__, i, data[i]);
  }
#endif

  VALIDATE_TST_VEC(in_cc.C0, out_cc.C0, 2, "C0");
  VALIDATE_TST_VEC(in_cc.C1, out_cc.C1, 2, "C1");
  VALIDATE_TST_VEC(in_cc.C2, out_cc.C2, 2, "C2");
  VALIDATE_TST_VEC(in_cc.C3, out_cc.C3, 2, "C3");
  VALIDATE_TST_VEC(in_cc.C4, out_cc.C4, 2, "C4");
  VALIDATE_TST_VEC(in_cc.C5, out_cc.C5, 2, "C5");
  VALIDATE_TST_VEC(in_cc.C6, out_cc.C6, 2, "C6");
  VALIDATE_TST_VEC(in_cc.C7, out_cc.C7, 2, "C7");
  VALIDATE_TST_VEC(in_cc.C8, out_cc.C8, 2, "C8");
  VALIDATE_TST_VEC(in_cc.K0, out_cc.K0, 0, "K0");
  VALIDATE_TST_VEC(in_cc.K1, out_cc.K1, 0, "K1");
  VALIDATE_TST_VEC(in_cc.K2, out_cc.K2, 0, "K2");

  VALIDATE_TST_VEC(in_cc.coefQFactor, out_cc.coefQFactor, 0, "coefQFactor");

  return 0;
}/*vfe_color_correct_tv_validate*/
