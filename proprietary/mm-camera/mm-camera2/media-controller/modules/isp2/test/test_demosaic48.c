/*============================================================================

  Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "vfe_test_vector.h"
#include "demosaic/demosaic48/demosaic48.h"
#include "demosaic/demosaic48/include48/demosaic_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - vfe_demosaic_test_vector_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
int vfe_demosaic_tv_validate(void *input, void *output)
{
  int i = 0;

  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)output;
  ISP_DemosaicConfigType *in_cfg, *out_cfg;
  ISP_DemosaicConfigCmdType *in_cfgCmd, *out_cfgCmd;

  CDBG("%s:\n", __func__);

  /* Demo_cfg*/
  in_cfg = (ISP_DemosaicConfigType *)
    (mod_in->reg_dump + (ISP_DEMOSAIC_CFG_OFF/4));
  out_cfg = (ISP_DemosaicConfigType *)
    (mod_op->reg_dump + (ISP_DEMOSAIC_CFG_OFF/4));

  /*same start address, just differnt cast struct */
  in_cfgCmd = (ISP_DemosaicConfigCmdType *)
    (mod_in->reg_dump + (ISP_DEMOSAIC_CFG_OFF/4));
  out_cfgCmd = (ISP_DemosaicConfigCmdType *)
    (mod_op->reg_dump + (ISP_DEMOSAIC_CFG_OFF/4));

  /* DEMO_CFG*/
  VALIDATE_TST_VEC(in_cfg->dirGInterpDisable, out_cfg->dirGInterpDisable,
    0, "dirGInterpDisable");
  VALIDATE_TST_VEC(in_cfg->dirRBInterpDisable, out_cfg->dirRBInterpDisable,
    0, "dirRBInterpDisable");
  VALIDATE_TST_VEC(in_cfg->cositedRgbEnable, out_cfg->cositedRgbEnable,
    0, "cositedRgbEnable");
  VALIDATE_TST_VEC(in_cfg->dynGClampEnable, out_cfg->dynGClampEnable,
    0, "dynGClampEnable");
  VALIDATE_TST_VEC(in_cfg->dynRBClampEnable, out_cfg->dynRBClampEnable,
    0, "dynRBClampEnable");

  /*LEFT WB CFG 0*/
  VALIDATE_TST_VEC(in_cfgCmd->leftWBCfg0.g_gain,
    out_cfgCmd->leftWBCfg0.g_gain, 0, "g_gain");

  VALIDATE_TST_VEC(in_cfgCmd->leftWBCfg0.b_gain,
    out_cfgCmd->leftWBCfg0.b_gain, 0, "b_gain");
  /*LEFT WB CFG 1*/
  VALIDATE_TST_VEC(in_cfgCmd->leftWBCfg1.r_gain,
    out_cfgCmd->leftWBCfg1.r_gain, 0, "r_gain");
  /*LEFT WB OFFSET CFG 0*/
  VALIDATE_TST_VEC(in_cfgCmd->leftWBOffsetCfg0.g_offset,
    out_cfgCmd->leftWBOffsetCfg0.g_offset, 0, "g_offset");

  VALIDATE_TST_VEC(in_cfgCmd->leftWBOffsetCfg0.b_offset
    , out_cfgCmd->leftWBOffsetCfg0.b_offset, 0, "b_offset");
  /*LEFT WB OFFSET CFG 1*/
  VALIDATE_TST_VEC(in_cfgCmd->leftWBOffsetCfg1.r_offset,
    out_cfgCmd->leftWBOffsetCfg1.r_offset, 0, "r_offset");

  /*RIGHT WB CFG 0*/
  VALIDATE_TST_VEC(in_cfgCmd->rightWBCfg0.g_gain,
    out_cfgCmd->rightWBCfg0.g_gain, 0, "g_gain");
  VALIDATE_TST_VEC(in_cfgCmd->rightWBCfg0.b_gain,
    out_cfgCmd->rightWBCfg0.b_gain, 0, "b_gain");
  /*RIGHT WB CFG 1*/
  VALIDATE_TST_VEC(in_cfgCmd->rightWBCfg1.r_gain,
    out_cfgCmd->rightWBCfg1.r_gain, 0, "r_gain");

  /*RIGHTT WB OFFSET CFG 0*/
  VALIDATE_TST_VEC(in_cfgCmd->rightWBOffsetCfg0.g_offset,
    out_cfgCmd->rightWBOffsetCfg0.g_offset, 0, "g_offset");
  VALIDATE_TST_VEC(in_cfgCmd->rightWBOffsetCfg0.b_offset,
    out_cfgCmd->rightWBOffsetCfg0.b_offset, 0, "b_offset");
  /*RIGHTT WB OFFSET CFG 1*/
  VALIDATE_TST_VEC(in_cfgCmd->rightWBOffsetCfg1.r_offset,
    out_cfgCmd->rightWBOffsetCfg1.r_offset, 0, "r_offset");

  /* Interp Coeff*/
  VALIDATE_TST_VEC(in_cfgCmd->interpCoeffCfg.lambda_g,
    out_cfgCmd->interpCoeffCfg.lambda_g, 0, "lambda_g");
  VALIDATE_TST_VEC(in_cfgCmd->interpCoeffCfg.lambda_rb,
    out_cfgCmd->interpCoeffCfg.lambda_rb , 0, "lambda_rb");

  /*Interp Classfier*/
  VALIDATE_TST_VEC(in_cfgCmd->interpClassifier.w_n,
    out_cfgCmd->interpClassifier.w_n, 0, "classifier.w_n");
  VALIDATE_TST_VEC(in_cfgCmd->interpClassifier.a_n,
    out_cfgCmd->interpClassifier.a_n, 0, "classifier.a_n");

  return 0;
}
