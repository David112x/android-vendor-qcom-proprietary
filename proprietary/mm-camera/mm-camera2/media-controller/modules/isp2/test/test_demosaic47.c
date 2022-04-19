/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "demosaic/demosaic47/demosaic47.h"
#include "demosaic_reg.h"
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
  VALIDATE_TST_VEC(in_cfg->dynGClampCDisable, out_cfg->dynGClampCDisable,
    0, "dynGClampCDisable");
  VALIDATE_TST_VEC(in_cfg->dynRBClampCDisable, out_cfg->dynRBClampCDisable,
    0, "dynRBClampCDisable");

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
  VALIDATE_TST_VEC(in_cfgCmd->interpCoeffCfg.wgr1,
    out_cfgCmd->interpCoeffCfg.wgr1 , 0, "wgr1");
  VALIDATE_TST_VEC(in_cfgCmd->interpCoeffCfg.wgr2,
    out_cfgCmd->interpCoeffCfg.wgr2, 0, "wgr2");
  VALIDATE_TST_VEC(in_cfgCmd->interpCoeffCfg.lambda_g,
    out_cfgCmd->interpCoeffCfg.lambda_g, 0, "lambda_g");
  VALIDATE_TST_VEC(in_cfgCmd->interpCoeffCfg.lambda_rb,
    out_cfgCmd->interpCoeffCfg.lambda_rb , 0, "lambda_rb");

  /*Interp Classfier*/
  for (i = 0; i < ISP_DEMOSAIC_CLASSIFIER_CNT; i++) {
    VALIDATE_TST_LUT(in_cfgCmd->interpClassifier[i].w_n ,
      out_cfgCmd->interpClassifier[i].w_n, 0, "classifier.w_n", i);
    VALIDATE_TST_LUT(in_cfgCmd->interpClassifier[i].t_n ,
      out_cfgCmd->interpClassifier[i].t_n, 0, "classifier.t_n", i);
    VALIDATE_TST_LUT(in_cfgCmd->interpClassifier[i].a_n ,
      out_cfgCmd->interpClassifier[i].a_n, 0, "classifier.a_n", i);
  }

  /*Interp Noise*/
  VALIDATE_TST_VEC(in_cfgCmd->interpNoiseCfg.noise_level_g , out_cfgCmd->interpNoiseCfg.noise_level_g , 0, "noise_level_g");
  VALIDATE_TST_VEC(in_cfgCmd->interpNoiseCfg.noise_level_rb , out_cfgCmd->interpNoiseCfg.noise_level_rb , 0, "noise_level_rb");

  return 0;
}
