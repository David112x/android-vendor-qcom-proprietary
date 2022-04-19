/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "mce/mce40/mce40.h"
#include "mce_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - vfe_mce_test_vector_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
int vfe_mce_tv_validate(void *input, void *output)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)output;
  ISP_MCE_ConfigCmdType *in, *out;

  CDBG("%s:\n", __func__);
  in = (ISP_MCE_ConfigCmdType *)(mod_in->reg_dump + (ISP_MCE40_OFF/4));
  out = (ISP_MCE_ConfigCmdType *)(mod_op->reg_dump + (ISP_MCE40_OFF/4));

  // Red Color
  VALIDATE_TST_VEC(in->redCfg.y1, out->redCfg.y1, 0, "redCfg.y1");
  VALIDATE_TST_VEC(in->redCfg.y2, out->redCfg.y2, 0, "redCfg.y2");
  VALIDATE_TST_VEC(in->redCfg.y3, out->redCfg.y3, 0, "redCfg.y3");
  VALIDATE_TST_VEC(in->redCfg.y4, out->redCfg.y4, 0, "redCfg.y4");
  VALIDATE_TST_VEC(in->redCfg.yM1, out->redCfg.yM1, 0, "redCfg.yM1");
  VALIDATE_TST_VEC(in->redCfg.yM3, out->redCfg.yM3, 0, "redCfg.yM3");
  VALIDATE_TST_VEC(in->redCfg.yS1, out->redCfg.yS1, 0, "redCfg.yS1");
  VALIDATE_TST_VEC(in->redCfg.yS3, out->redCfg.yS3, 0, "redCfg.yS3");
  VALIDATE_TST_VEC(in->redCfg.transWidth, out->redCfg.transWidth, 0, "redCfg.transWidth");
  VALIDATE_TST_VEC(in->redCfg.transTrunc, out->redCfg.transTrunc, 0, "redCfg.transTrunc");
  VALIDATE_TST_VEC(in->redCfg.CRZone, out->redCfg.CRZone, 0, "redCfg.CRZone");
  VALIDATE_TST_VEC(in->redCfg.CBZone, out->redCfg.CBZone, 0, "redCfg.CBZone");
  VALIDATE_TST_VEC(in->redCfg.transSlope, out->redCfg.transSlope, 0, "redCfg.transSlope");
  VALIDATE_TST_VEC(in->redCfg.K, out->redCfg.K, 0, "redCfg.K");

  // Green Color
  VALIDATE_TST_VEC(in->greenCfg.y1, out->greenCfg.y1, 0, "greenCfg.y1");
  VALIDATE_TST_VEC(in->greenCfg.y2, out->greenCfg.y2, 0, "greenCfg.y2");
  VALIDATE_TST_VEC(in->greenCfg.y3, out->greenCfg.y3, 0, "greenCfg.y3");
  VALIDATE_TST_VEC(in->greenCfg.y4, out->greenCfg.y4, 0, "greenCfg.y4");
  VALIDATE_TST_VEC(in->greenCfg.yM1, out->greenCfg.yM1, 0, "greenCfg.yM1");
  VALIDATE_TST_VEC(in->greenCfg.yM3, out->greenCfg.yM3, 0, "greenCfg.yM3");
  VALIDATE_TST_VEC(in->greenCfg.yS1, out->greenCfg.yS1, 0, "greenCfg.yS1");
  VALIDATE_TST_VEC(in->greenCfg.yS3, out->greenCfg.yS3, 0, "greenCfg.yS3");
  VALIDATE_TST_VEC(in->greenCfg.transWidth, out->greenCfg.transWidth, 0, "greenCfg.transWidth");
  VALIDATE_TST_VEC(in->greenCfg.transTrunc, out->greenCfg.transTrunc, 0, "greenCfg.transTrunc");
  VALIDATE_TST_VEC(in->greenCfg.CRZone, out->greenCfg.CRZone, 0, "greenCfg.CRZone");
  VALIDATE_TST_VEC(in->greenCfg.CBZone, out->greenCfg.CBZone, 0, "greenCfg.CBZone");
  VALIDATE_TST_VEC(in->greenCfg.transSlope, out->greenCfg.transSlope, 0, "greenCfg.transSlope");

  // Blue Color
  VALIDATE_TST_VEC(in->blueCfg.y1, out->blueCfg.y1, 0, "blueCfg.y1");
  VALIDATE_TST_VEC(in->blueCfg.y2, out->blueCfg.y2, 0, "blueCfg.y2");
  VALIDATE_TST_VEC(in->blueCfg.y3, out->blueCfg.y3, 0, "blueCfg.y3");
  VALIDATE_TST_VEC(in->blueCfg.y4, out->blueCfg.y4, 0, "blueCfg.y4");
  VALIDATE_TST_VEC(in->blueCfg.yM1, out->blueCfg.yM1, 0, "blueCfg.yM1");
  VALIDATE_TST_VEC(in->blueCfg.yM3, out->blueCfg.yM3, 0, "blueCfg.yM3");
  VALIDATE_TST_VEC(in->blueCfg.yS1, out->blueCfg.yS1, 0, "blueCfg.yS1");
  VALIDATE_TST_VEC(in->blueCfg.yS3, out->blueCfg.yS3, 0, "blueCfg.yS3");
  VALIDATE_TST_VEC(in->blueCfg.transWidth, out->blueCfg.transWidth, 0, "blueCfg.transWidth");
  VALIDATE_TST_VEC(in->blueCfg.transTrunc, out->blueCfg.transTrunc, 0, "blueCfg.transTrunc");
  VALIDATE_TST_VEC(in->blueCfg.CRZone, out->blueCfg.CRZone, 0, "blueCfg.CRZone");
  VALIDATE_TST_VEC(in->blueCfg.CBZone, out->blueCfg.CBZone, 0, "blueCfg.CBZone");
  VALIDATE_TST_VEC(in->blueCfg.transSlope, out->blueCfg.transSlope, 0, "blueCfg.transSlope");

  return 0;
}
