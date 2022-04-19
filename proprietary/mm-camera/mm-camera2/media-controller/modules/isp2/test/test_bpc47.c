/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "bpc/bpc47/bpc47.h"
#include "bpc_reg.h"
#include "camera_dbg.h"
#include "isp_pipeline_reg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - vfe_bpc_test_vector_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
int vfe_bpc_tv_validate(void *input, void *output)
{
  vfe_test_module_input_t *mod_in  = (vfe_test_module_input_t *)input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)output;

  ISP_ModuleEn *module_enable_reg_in  = NULL;
  ISP_ModuleEn *module_enable_reg_out = NULL;

  ISP_BPCCfg0 *cfg0_in, *cfg0_out;
  ISP_BPCRegConfig  *in, *out;

  CDBG("%s:\n", __func__);
  cfg0_in = (ISP_BPCCfg0 *)(mod_in->reg_dump + (ISP_BPC_CFG0_OFF/4));
  cfg0_out = (ISP_BPCCfg0 *)(mod_op->reg_dump + (ISP_BPC_CFG0_OFF/4));

  in = (ISP_BPCRegConfig *)(mod_in->reg_dump + (ISP_BPC_CFG1_OFF/4));
  out = (ISP_BPCRegConfig *)(mod_op->reg_dump + (ISP_BPC_CFG1_OFF/4));

  module_enable_reg_in  = (ISP_ModuleEn *)(mod_in->reg_dump + (ISP_MODULE_LENS_EN_OFFSET/4));
  module_enable_reg_out = (ISP_ModuleEn *)(mod_op->reg_dump + (ISP_MODULE_LENS_EN_OFFSET/4));

  if ((module_enable_reg_out->fields.bpc == 0) &&
      (module_enable_reg_in->fields.bpc  == 0)) {
    printf("\t(BPC disabled in MODULE_LENS_EN)\n");
    return 0;
  }

  /* BPC_CFG_0*/
  VALIDATE_TST_VEC(cfg0_in->HotPixCor_Disable ,
    cfg0_out->HotPixCor_Disable , 0, "HotPixCor_Disable");
  VALIDATE_TST_VEC(cfg0_in->ColdPixCor_Disable ,
    cfg0_out->ColdPixCor_Disable , 0, "ColdPixCor_Disable");
  VALIDATE_TST_VEC(cfg0_in->SameChDetect ,
    cfg0_out->SameChDetect , 0, "SameChDetect");
  VALIDATE_TST_VEC(cfg0_in->SameChRecover ,
    cfg0_out->SameChRecover , 0, "SameChRecover");
  VALIDATE_TST_VEC(cfg0_in->SqrtSensorGain ,
    cfg0_out->SqrtSensorGain , 0, "SqrtSensorGain");
  VALIDATE_TST_VEC(cfg0_in->BlkLevel , cfg0_out->BlkLevel,
     0, "BlkLevel");

  /* BPC_CFG_1*/
  VALIDATE_TST_VEC(in->bpc_cfg1.RG_WB_GainRatio , out->bpc_cfg1.RG_WB_GainRatio
     , 0, "RG_WB_GainRatio");
  VALIDATE_TST_VEC(in->bpc_cfg1.BG_WB_GainRatio , out->bpc_cfg1.BG_WB_GainRatio
     , 0, "BG_WB_GainRatio");

  /* BPC_CFG_2*/
  VALIDATE_TST_VEC(in->bpc_cfg2.GR_WB_GainRatio , out->bpc_cfg2.GR_WB_GainRatio
     , 0, "GR_WB_GainRatio");
  VALIDATE_TST_VEC(in->bpc_cfg2.GB_WB_GainRatio , out->bpc_cfg2.GB_WB_GainRatio
     , 0, "GB_WB_GainRatio");

  /* BPC_CFG_3*/
  VALIDATE_TST_VEC(in->bpc_cfg3.BPC_Offset , out->bpc_cfg3.BPC_Offset , 0,
    "BPC_Offset");
  VALIDATE_TST_VEC(in->bpc_cfg3.BCC_Offset , out->bpc_cfg3.BCC_Offset , 0,
    "BCC_Offset");

  /* BPC_CFG_4*/
  VALIDATE_TST_VEC(in->bpc_cfg4.Fmax , out->bpc_cfg4.Fmax , 0, "Fmax");
  VALIDATE_TST_VEC(in->bpc_cfg4.Fmin , out->bpc_cfg4.Fmin , 0, "Fmin");
  VALIDATE_TST_VEC(in->bpc_cfg4.CorrectThreshold ,
    out->bpc_cfg4.CorrectThreshold , 0, "CorrectThreshold");

  /* BPC_CFG_5*/
  VALIDATE_TST_VEC(in->bpc_cfg5.HiglightValueThreshold ,
    out->bpc_cfg5.HiglightValueThreshold , 0, "HiglightValueThreshold");
  VALIDATE_TST_VEC(in->bpc_cfg5.NoiseModelGain , out->bpc_cfg5.NoiseModelGain ,
     0, "NoiseModelGain");
  /* BPC_CFG_6*/
  VALIDATE_TST_VEC(in->bpc_cfg6.HighlightDetectThreshold ,
    out->bpc_cfg6.HighlightDetectThreshold , 0, "HighlightDetectThreshold");
  /* BPC_CFG_7*/
  VALIDATE_TST_VEC(in->bpc_cfg7.HighlightDetectParam1 ,
    out->bpc_cfg7.HighlightDetectParam1 , 0, "HighlightDetectParam1");
  VALIDATE_TST_VEC(in->bpc_cfg7.HighlightDetectParam2 ,
    out->bpc_cfg7.HighlightDetectParam2 , 0, "HighlightDetectParam2");
  VALIDATE_TST_VEC(in->bpc_cfg7.HighlightDetectParam3 ,
    out->bpc_cfg7.HighlightDetectParam3 , 0, "HighlightDetectParam3");
  VALIDATE_TST_VEC(in->bpc_cfg7.HighlightDetectParamSum ,
    out->bpc_cfg7.HighlightDetectParamSum , 0, "HighlightDetectParamSum");
  /* BPC_CFG_8 DPCHotCount and DPCCOld Cnt are read only registers.
      No need to validate*/
/* VALIDATE_TST_VEC(in->bpc_dpc_stats.DpcHotCnt ,
     out->bpc_dpc_stats.DpcHotCnt , 0, "DpcHotCnt");
  VALIDATE_TST_VEC(in->bpc_dpc_stats.DpcColdCnt ,
    out->bpc_dpc_stats.DpcColdCnt , 0, "DpcColdCnt"); */
  return 0;
}
