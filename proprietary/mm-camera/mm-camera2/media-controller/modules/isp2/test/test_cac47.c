/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "cac/cac47/cac47.h"
#include "cac47_reg.h"
#include "isp_pipeline_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - vfe_mce_test_vector_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
int vfe_cac_tv_validate(void *input, void *output)
{
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)output;
  ISP_CacCfgCmdType *in, *out;
  ISP_CAC_CFG *module_enable_reg_in = NULL;
  ISP_CAC_CFG *module_enable_reg_out = NULL;

  CDBG("%s:\n", __func__);
  in = (ISP_CacCfgCmdType *)(mod_in->reg_dump + (ISP_CAC47_OFF/4));
  out = (ISP_CacCfgCmdType *)(mod_op->reg_dump + (ISP_CAC47_OFF/4));

  module_enable_reg_in = (ISP_CAC_CFG *)(mod_in->reg_dump + (ISP_CAC_SNR_OFFSET/4));
  module_enable_reg_out = (ISP_CAC_CFG *)(mod_op->reg_dump + (ISP_CAC_SNR_OFFSET/4));

  if ((module_enable_reg_out->fields.cac_enable == 0) &&
      (module_enable_reg_in->fields.cac_enable == 0)) {
    printf("\t(CAC disabled in MODULE_COLOR_EN)\n");
    return 0;
  }

  VALIDATE_TST_VEC(module_enable_reg_in->fields.cac_enable,
    module_enable_reg_out->fields.cac_enable, 0, "CAC_ENABLE");

  // Red Color
  VALIDATE_TST_VEC(in->ySpotThr, out->ySpotThr, 0, "ySpotThr");
  VALIDATE_TST_VEC(in->ResMode, out->ResMode, 0, "ResMode");
  VALIDATE_TST_VEC(in->ySaturationThr, out->ySaturationThr, 0, "ySaturationThr");
  VALIDATE_TST_VEC(in->cSaturationThr, out->cSaturationThr, 0, "cSaturationThr");
  VALIDATE_TST_VEC(in->cSpotThr, out->cSpotThr, 0, "cSpotThr");

  return 0;
}
