/*============================================================================
 * Copyright (c) 2014, 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/
#include "vfe_test_vector.h"
#include "bpc/bpc48/bpc48.h"
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

  ISP_BPCRegConfig  *in, *out;

  CDBG("%s:\n", __func__);

  in = (ISP_BPCRegConfig *)(mod_in->reg_dump + (ISP_BPC_CFG0_OFF/4));
  out = (ISP_BPCRegConfig *)(mod_op->reg_dump + (ISP_BPC_CFG0_OFF/4));

  module_enable_reg_in  = (ISP_ModuleEn *)(mod_in->reg_dump + (ISP_MODULE_LENS_EN_OFFSET/4));
  module_enable_reg_out = (ISP_ModuleEn *)(mod_op->reg_dump + (ISP_MODULE_LENS_EN_OFFSET/4));

  if ((module_enable_reg_out->fields.bpc == 0) &&
      (module_enable_reg_in->fields.bpc  == 0)) {
    printf("\t(BPC disabled in MODULE_LENS_EN)\n");
    return 0;
  }

  /* BPC_CFG_0*/
  VALIDATE_TST_VEC(in->bpc_cfg0.HotPixCor_Disable ,
    out->bpc_cfg0.HotPixCor_Disable , 0, "HotPixCor_Disable");
  VALIDATE_TST_VEC(in->bpc_cfg0.ColdPixCor_Disable ,
    out->bpc_cfg0.ColdPixCor_Disable , 0, "ColdPixCor_Disable");
  VALIDATE_TST_VEC(in->bpc_cfg0.SameChRecover ,
    out->bpc_cfg0.SameChRecover , 0, "SameChRecover");
  VALIDATE_TST_VEC(in->bpc_cfg0.BlkLevel , out->bpc_cfg0.BlkLevel,
     0, "BlkLevel");

  /* BPC_CFG_1*/
  VALIDATE_TST_VEC(in->bpc_cfg1.RG_WB_GainRatio , out->bpc_cfg1.RG_WB_GainRatio
     , 0, "RG_WB_GainRatio");

  /* BPC_CFG_2*/
  VALIDATE_TST_VEC(in->bpc_cfg2.BG_WB_GainRatio , out->bpc_cfg2.BG_WB_GainRatio
     , 0, "BG_WB_GainRatio");

  /* BPC_CFG_3*/
  VALIDATE_TST_VEC(in->bpc_cfg3.GR_WB_GainRatio , out->bpc_cfg3.GR_WB_GainRatio
     , 0, "GR_WB_GainRatio");

 /* BPC_CFG_4*/
  VALIDATE_TST_VEC(in->bpc_cfg4.GB_WB_GainRatio , out->bpc_cfg4.GB_WB_GainRatio
     , 0, "GB_WB_GainRatio");

  /* BPC_CFG_5*/
  VALIDATE_TST_VEC(in->bpc_cfg5.BPC_Offset , out->bpc_cfg5.BPC_Offset , 0,
    "BPC_Offset");
  VALIDATE_TST_VEC(in->bpc_cfg5.BCC_Offset , out->bpc_cfg5.BCC_Offset , 0,
    "BCC_Offset");

  /* BPC_CFG_6*/
  VALIDATE_TST_VEC(in->bpc_cfg6.Fmax , out->bpc_cfg6.Fmax , 0, "Fmax");
  VALIDATE_TST_VEC(in->bpc_cfg6.Fmin , out->bpc_cfg6.Fmin , 0, "Fmin");
  VALIDATE_TST_VEC(in->bpc_cfg6.CorrectThreshold ,
    out->bpc_cfg6.CorrectThreshold , 0, "CorrectThreshold");

  /* BPC_CFG_7 DPC_HOT_CNT and DPC_COLD_CNT are read only registers.
      No need to validate*/

  return 0;
}
