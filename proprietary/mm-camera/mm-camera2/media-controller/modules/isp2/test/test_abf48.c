/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/

#include "vfe_test_vector.h"
#include "abf/abf48/abf48.h"
//#include "abf48_reg.h"
#include "camera_dbg.h"
#include "isp_pipeline_reg.h"
#include <unistd.h>


/*===========================================================================
 * FUNCTION    - abf_validate_tables -
 *
 * DESCRIPTION:
 *==========================================================================*/
void abf_validate_tables(vfe_test_table_t *in,
  vfe_test_table_t *out, char *tbl_name)
{
  uint32_t i;
  for (i = 0; i < in->size; i++)
    VALIDATE_TST_LUT(in->table[i], out->table[i], 0, tbl_name, i);

} /* abf_validate_tables*/

/*===========================================================================
 * FUNCTION    - vfe_abf_test_channel_cfg -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
static void vfe_abf_test_channel_cfg(ISP_ABF34_ChannelCfg *in_chCfg,
  ISP_ABF34_ChannelCfg *out_chCfg, char *ch_name)
{

  VALIDATE_TST_VEC2(in_chCfg->curveOffset,
    out_chCfg->curveOffset , 0, "curveOffset", ch_name);

  return;
}

/*===========================================================================
 * FUNCTION    - vfe_abf_test_vector_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
int vfe_abf_tv_validate(void *input, void *output)
{
  if (!input || !output) {
    CDBG_ERROR("failed: %s Null pointer input %p output %p\n",
      __func__, input, output);
    return -1;
  }

  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)output;

  ISP_ModuleEn *module_enable_reg_in = NULL;
  ISP_ModuleEn *module_enable_reg_out = NULL;
  ISP_ABF34_cfg_cmd_t *cfg0_in, *cfg0_out;
  ISP_ABF34_cfg_2_cmd_t *in, *out;

  module_enable_reg_in  = (ISP_ModuleEn *)(mod_in->reg_dump + (ISP_MODULE_LENS_EN_OFFSET/4));
  module_enable_reg_out = (ISP_ModuleEn *)(mod_op->reg_dump + (ISP_MODULE_LENS_EN_OFFSET/4));

  if ((module_enable_reg_out->fields.abf == 0) &&
      (module_enable_reg_in->fields.abf  == 0)) {
    printf("\t(ABF disabled in MODULE_LENS_EN)\n");
    return 0;
  }

  CDBG("%s:\n", __func__);

  cfg0_in = (ISP_ABF34_cfg_cmd_t *)(mod_in->reg_dump + (ISP_ABF34_CFG/4));
  cfg0_out = (ISP_ABF34_cfg_cmd_t *)(mod_op->reg_dump + (ISP_ABF34_CFG/4));

  in = (ISP_ABF34_cfg_2_cmd_t *)(mod_in->reg_dump + (ISP_ABF34_CFG2/4));
  out = (ISP_ABF34_cfg_2_cmd_t *)(mod_op->reg_dump + (ISP_ABF34_CFG2/4));
  /*ABF_CFG*/
  VALIDATE_TST_VEC(cfg0_in->cfg.filterEn, cfg0_out->cfg.filterEn, 0, "filterEn");
  VALIDATE_TST_VEC(cfg0_in->cfg.singleBpcEn, cfg0_out->cfg.singleBpcEn, 0, "singleBpcEn");
  VALIDATE_TST_VEC(cfg0_in->cfg.crossPlEn, cfg0_out->cfg.crossPlEn, 0, "crossPlEn");
  VALIDATE_TST_VEC(cfg0_in->cfg.pixelMatchLvGrGb, cfg0_out->cfg.pixelMatchLvGrGb, 0, "pixelMatchLvGrGb");
  VALIDATE_TST_VEC(cfg0_in->cfg.pixelMatchLvRb, cfg0_out->cfg.pixelMatchLvRb, 0, "pixelMatchLvRb");
  VALIDATE_TST_VEC(cfg0_in->cfg.distGrGb_0, cfg0_out->cfg.distGrGb_0, 0, "distGrGb_0");
  VALIDATE_TST_VEC(cfg0_in->cfg.distGrGb_1, cfg0_out->cfg.distGrGb_1, 0, "distGrGb_1");
  VALIDATE_TST_VEC(cfg0_in->cfg.distGrGb_2, cfg0_out->cfg.distGrGb_2, 0, "distGrGb_2");
  VALIDATE_TST_VEC(cfg0_in->cfg.distRb_0, cfg0_out->cfg.distRb_0, 0, "distRb_0");
  VALIDATE_TST_VEC(cfg0_in->cfg.distRb_1, cfg0_out->cfg.distRb_1, 0, "distRb_1");
  VALIDATE_TST_VEC(cfg0_in->cfg.distRb_2, cfg0_out->cfg.distRb_2, 0, "distRb_2");

  /*Channel CFG, GR/GB/R/B */
  vfe_abf_test_channel_cfg(&in->grCfg, &out->grCfg, "CH_GR");
  vfe_abf_test_channel_cfg(&in->gbCfg, &out->gbCfg, "CH_GB");
  vfe_abf_test_channel_cfg(&in->rCfg, &out->rCfg, "CH_R");
  vfe_abf_test_channel_cfg(&in->bCfg, &out->bCfg, "CH_B");

  /*RNR CFG*/
  /*CFG_0*/
  VALIDATE_TST_VEC(in->rnrCfg.bx , out->rnrCfg.bx , 0, "bx");
  VALIDATE_TST_VEC(in->rnrCfg.by , out->rnrCfg.by , 0, "by");
  /*CFG_1*/
  VALIDATE_TST_VEC(in->rnrCfg.initSquare , out->rnrCfg.initSquare , 0, "initSquare");
  /*CFG_2*/
  VALIDATE_TST_VEC(in->rnrCfg.anchor_0 , out->rnrCfg.anchor_0 , 0, "anchor_0");
  VALIDATE_TST_VEC(in->rnrCfg.anchor_1 , out->rnrCfg.anchor_1 , 0, "anchor_1");
  /*CFG_3*/
  VALIDATE_TST_VEC(in->rnrCfg.anchor_2 , out->rnrCfg.anchor_2 , 0, "anchor_2");
  VALIDATE_TST_VEC(in->rnrCfg.anchor_3 , out->rnrCfg.anchor_3 , 0, "anchor_3");
  /*CFG_4*/
  VALIDATE_TST_VEC(in->rnrCfg.coeffBase_0 , out->rnrCfg.coeffBase_0 , 0, "coeffBase_0");
  VALIDATE_TST_VEC(in->rnrCfg.coeffSlope_0 , out->rnrCfg.coeffSlope_0 , 0, "coeffSlope_0");
  VALIDATE_TST_VEC(in->rnrCfg.coeffShift_0 , out->rnrCfg.coeffShift_0 , 0, "coeffShift_0");
  /*CFG_5*/
  VALIDATE_TST_VEC(in->rnrCfg.coeffBase_1 , out->rnrCfg.coeffBase_1 , 0, "coeffBase_1");
  VALIDATE_TST_VEC(in->rnrCfg.coeffSlope_1 , out->rnrCfg.coeffSlope_1 , 0, "coeffSlope_1");
  VALIDATE_TST_VEC(in->rnrCfg.coeffShift_1 , out->rnrCfg.coeffShift_1 , 0, "coeffShift_1");
  /*CFG_6*/
  VALIDATE_TST_VEC(in->rnrCfg.coeffBase_2 , out->rnrCfg.coeffBase_2 , 0, "coeffBase_2");
  VALIDATE_TST_VEC(in->rnrCfg.coeffSlope_2 , out->rnrCfg.coeffSlope_2 , 0, "coeffSlope_2");
  VALIDATE_TST_VEC(in->rnrCfg.coeffShift_2 , out->rnrCfg.coeffShift_2 , 0, "coeffShift_2");
  /*CFG_7*/
  VALIDATE_TST_VEC(in->rnrCfg.coeffBase_3 , out->rnrCfg.coeffBase_3 , 0, "coeffBase_3");
  VALIDATE_TST_VEC(in->rnrCfg.coeffSlope_3 , out->rnrCfg.coeffSlope_3 , 0, "coeffSlope_3");
  VALIDATE_TST_VEC(in->rnrCfg.coeffShift_3 , out->rnrCfg.coeffShift_3 , 0, "coeffShift_3");
  /*CFG_8*/
  VALIDATE_TST_VEC(in->rnrCfg.threshBase_0 , out->rnrCfg.threshBase_0 , 0, "threshBase_0");
  VALIDATE_TST_VEC(in->rnrCfg.threshSlope_0 , out->rnrCfg.threshSlope_0 , 0, "threshSlope_0");
  VALIDATE_TST_VEC(in->rnrCfg.threshShift_0 , out->rnrCfg.threshShift_0 , 0, "threshShift_0");
  /*CFG_9*/
  VALIDATE_TST_VEC(in->rnrCfg.threshBase_1 , out->rnrCfg.threshBase_1 , 0, "threshBase_1");
  VALIDATE_TST_VEC(in->rnrCfg.threshSlope_1 , out->rnrCfg.threshSlope_1 , 0, "threshSlope_1");
  VALIDATE_TST_VEC(in->rnrCfg.threshShift_1 , out->rnrCfg.threshShift_1 , 0, "threshShift_1");
  /*CFG_10*/
  VALIDATE_TST_VEC(in->rnrCfg.threshBase_2 , out->rnrCfg.threshBase_2 , 0, "threshBase_2");
  VALIDATE_TST_VEC(in->rnrCfg.threshSlope_2 , out->rnrCfg.threshSlope_2 , 0, "threshSlope_2");
  VALIDATE_TST_VEC(in->rnrCfg.threshShift_2 , out->rnrCfg.threshShift_2 , 0, "threshShift_2");
  /*CFG_11*/
  VALIDATE_TST_VEC(in->rnrCfg.threshBase_3 , out->rnrCfg.threshBase_3 , 0, "threshBase_3");
  VALIDATE_TST_VEC(in->rnrCfg.threshSlope_3 , out->rnrCfg.threshSlope_3 , 0, "threshSlope_3");
  VALIDATE_TST_VEC(in->rnrCfg.threshShift_3 , out->rnrCfg.threshShift_3 , 0, "threshShift_3");
  /*CFG_12*/
  VALIDATE_TST_VEC(in->rnrCfg.rsquareShift , out->rnrCfg.rsquareShift , 0, "rsquareShift");

  /*BPC CFG*/
  /*CFG_0*/
  VALIDATE_TST_VEC(in->bpcCfg.fmax , out->bpcCfg.fmax , 0, "fmax");
  VALIDATE_TST_VEC(in->bpcCfg.fmin , out->bpcCfg.fmin , 0, "fmin");
  VALIDATE_TST_VEC(in->bpcCfg.offset , out->bpcCfg.offset , 0, "offset");
  /*CFG_1*/
  VALIDATE_TST_VEC(in->bpcCfg.minShift , out->bpcCfg.minShift , 0, "minShift");
  VALIDATE_TST_VEC(in->bpcCfg.maxShift , out->bpcCfg.maxShift , 0, "maxShift");
  VALIDATE_TST_VEC(in->bpcCfg.bls , out->bpcCfg.bls , 0, "bls");

  /*NOSIE_PREV CFG*/
  /*CFG_0*/
  VALIDATE_TST_VEC(in->noisePrsvCfg.anchor_lo , out->noisePrsvCfg.anchor_lo , 0, "anchor_lo");
  VALIDATE_TST_VEC(in->noisePrsvCfg.anchor_gap , out->noisePrsvCfg.anchor_gap , 0, "anchor_gap");
  /*CFG_1*/
  VALIDATE_TST_VEC(in->noisePrsvCfg.lo_grgb , out->noisePrsvCfg.lo_grgb , 0, "lo_grgb");
  VALIDATE_TST_VEC(in->noisePrsvCfg.slope_grgb , out->noisePrsvCfg.slope_grgb , 0, "slope_grgb");
  VALIDATE_TST_VEC(in->noisePrsvCfg.shift_grgb , out->noisePrsvCfg.shift_grgb , 0, "shift_grgb");
  /*CFG_2*/
  VALIDATE_TST_VEC(in->noisePrsvCfg.lo_rb , out->noisePrsvCfg.lo_rb , 0, "lo_rb");
  VALIDATE_TST_VEC(in->noisePrsvCfg.slope_rb , out->noisePrsvCfg.slope_rb , 0, "slope_rb");
  VALIDATE_TST_VEC(in->noisePrsvCfg.shift_rb , out->noisePrsvCfg.shift_rb , 0, "shift_rb");

  abf_validate_tables(&mod_in->abf_std_l0, &mod_op->abf_std_l0, "abf_std_l0");

  return 0;
}

