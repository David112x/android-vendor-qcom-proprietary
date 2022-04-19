/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vfe_test_vector.h"
#include "abf/abf47/abf47.h"
#include "abf47_reg.h"
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
static void vfe_abf_test_channel_cfg(ISP_ABF3_ChannelCfg *in_chCfg,
  ISP_ABF3_ChannelCfg *out_chCfg, char *ch_name)
{

  /*CH_CFG_0*/
  VALIDATE_TST_VEC2(in_chCfg->noiseScale0Lv0,
    out_chCfg->noiseScale0Lv0 , 0, "noiseScale0Lv0", ch_name);
  VALIDATE_TST_VEC2(in_chCfg->wTableAdjLv0,
    out_chCfg->wTableAdjLv0 , 0, "wTableAdjLv0", ch_name);
  VALIDATE_TST_VEC2(in_chCfg->noiseScale0Lv1,
    out_chCfg->noiseScale0Lv1 , 0, "noiseScale0Lv1", ch_name);
  VALIDATE_TST_VEC2(in_chCfg->wTableAdjLv1,
    out_chCfg->wTableAdjLv1 , 0, "wTableAdjLv1", ch_name);

  /*CH_CFG_1*/
  VALIDATE_TST_VEC2(in_chCfg->noiseScale1Lv0,
    out_chCfg->noiseScale1Lv0 , 0, "noiseScale1Lv0", ch_name);
  VALIDATE_TST_VEC2(in_chCfg->noiseScale1Lv1,
    out_chCfg->noiseScale1Lv1 , 0, "noiseScale1Lv1", ch_name);
  /*CH_CFG_2*/
  VALIDATE_TST_VEC2(in_chCfg->noiseScale2Lv0,
    out_chCfg->noiseScale2Lv0 , 0, "noiseScale2Lv0", ch_name);
  VALIDATE_TST_VEC2(in_chCfg->noiseScale2Lv1,
    out_chCfg->noiseScale2Lv1 , 0, "noiseScale2Lv1", ch_name);
  /*CH_CFG_3*/
  VALIDATE_TST_VEC2(in_chCfg->noiseOffLv0,
    out_chCfg->noiseOffLv0 , 0, "noiseOffLv0", ch_name);
  VALIDATE_TST_VEC2(in_chCfg->noiseOffLv1,
    out_chCfg->noiseOffLv1 , 0, "noiseOffLv1", ch_name);
  /*CH_CFG_4*/
  VALIDATE_TST_VEC2(in_chCfg->curvOffLv0,
    out_chCfg->curvOffLv0 , 0, "curvOffLv0", ch_name);
  VALIDATE_TST_VEC2(in_chCfg->curvOffLv1,
    out_chCfg->curvOffLv1 , 0, "curvOffLv1", ch_name);
  /*CH_CFG_5*/
  VALIDATE_TST_VEC2(in_chCfg->sftThrdNoiseScaleLv0,
    out_chCfg->sftThrdNoiseScaleLv0, 0, "sftThrdNoiseScaleLv0", ch_name);
  VALIDATE_TST_VEC2(in_chCfg->sftThrdNoiseShiftLv0,
    out_chCfg->sftThrdNoiseShiftLv0, 0, "sftThrdNoiseShiftLv0", ch_name);
  VALIDATE_TST_VEC2(in_chCfg->sftThrdNoiseScaleLv1,
    out_chCfg->sftThrdNoiseScaleLv1, 0, "sftThrdNoiseScaleLv1", ch_name);
  VALIDATE_TST_VEC2(in_chCfg->sftThrdNoiseShiftLv1,
    out_chCfg->sftThrdNoiseShiftLv1, 0, "sftThrdNoiseShiftLv1", ch_name);
  /*CH_CFG_6*/
  VALIDATE_TST_VEC2(in_chCfg->filterNoisePreserveL_Lv0 ,
    out_chCfg->filterNoisePreserveL_Lv0, 0,
    "filterNoisePreserveL_Lv0", ch_name);
  VALIDATE_TST_VEC2(in_chCfg->filterNoisePreserveR_Lv0,
    out_chCfg->filterNoisePreserveR_Lv0, 0,
    "filterNoisePreserveR_Lv0", ch_name);
  /*CH_CFG_7*/
  VALIDATE_TST_VEC2(in_chCfg->filterNoisePreserveL_Lv1,
    out_chCfg->filterNoisePreserveL_Lv1, 0,
    "filterNoisePreserveL_Lv1", ch_name);
  VALIDATE_TST_VEC2(in_chCfg->filterNoisePreserveR_Lv1,
    out_chCfg->filterNoisePreserveR_Lv1, 0,
    "filterNoisePreserveR_Lv1", ch_name);
  /*CH_CFG_8*/
  VALIDATE_TST_VEC2(in_chCfg->spatialScaleL_Lv0, out_chCfg->spatialScaleL_Lv0,
    0, "spatialScaleL_Lv0", ch_name);
  VALIDATE_TST_VEC2(in_chCfg->spatialScaleR_Lv0, out_chCfg->spatialScaleR_Lv0,
    0, "spatialScaleR_Lv0", ch_name);
  /*CH_CFG_9*/
  VALIDATE_TST_VEC2(in_chCfg->spatialScaleL_Lv1, out_chCfg->spatialScaleL_Lv1,
    0, "spatialScaleL_Lv1", ch_name);
  VALIDATE_TST_VEC2(in_chCfg->spatialScaleR_Lv1, out_chCfg->spatialScaleR_Lv1,
    0, "spatialScaleR_Lv1", ch_name);

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
  ISP_ABF3_CmdType *in, *out;

  module_enable_reg_in  = (ISP_ModuleEn *)(mod_in->reg_dump + (ISP_MODULE_LENS_EN_OFFSET/4));
  module_enable_reg_out = (ISP_ModuleEn *)(mod_op->reg_dump + (ISP_MODULE_LENS_EN_OFFSET/4));

  if ((module_enable_reg_out->fields.abf == 0) &&
      (module_enable_reg_in->fields.abf  == 0)) {
    printf("\t(ABF disabled in MODULE_LENS_EN)\n");
    return 0;
  }

  CDBG("%s:\n", __func__);
  in = (ISP_ABF3_CmdType *)(mod_in->reg_dump + (ISP_ABF3_OFF/4));
  out = (ISP_ABF3_CmdType *)(mod_op->reg_dump + (ISP_ABF3_OFF/4));
  /*ABF_CFG*/
  VALIDATE_TST_VEC(in->cfg.crossPlLevel0En, out->cfg.crossPlLevel0En, 0, "CrossPlevel0En");
  VALIDATE_TST_VEC(in->cfg.crossPlLevel1En, out->cfg.crossPlLevel1En, 0, "CrossPlevel1En");
  VALIDATE_TST_VEC(in->cfg.minMaxEn, out->cfg.minMaxEn, 0, "minMaxEn");

  /*Level 1 cfg*/
  /*CFG_0*/
  VALIDATE_TST_VEC(in->level1.distGrGb_0 , out->level1.distGrGb_0 , 0, "distGrGb_0");
  VALIDATE_TST_VEC(in->level1.distGrGb_1 , out->level1.distGrGb_1 , 0, "distGrGb_1");
  VALIDATE_TST_VEC(in->level1.distGrGb_2 , out->level1.distGrGb_2 , 0, "distGrGb_2");
  VALIDATE_TST_VEC(in->level1.distGrGb_3 , out->level1.distGrGb_3 , 0, "distGrGb_3");
  VALIDATE_TST_VEC(in->level1.distGrGb_4 , out->level1.distGrGb_4 , 0, "distGrGb_4");
  VALIDATE_TST_VEC(in->level1.minMaxSelGrGb0 , out->level1.minMaxSelGrGb0 , 0, "minMaxSelGrGb0");
  VALIDATE_TST_VEC(in->level1.minMaxSelGrGb1 , out->level1.minMaxSelGrGb1 , 0, "minMaxSelGrGb1");
  /*CFG_1*/
  VALIDATE_TST_VEC(in->level1.distRb0 , out->level1.distRb0 , 0, "distRb0");
  VALIDATE_TST_VEC(in->level1.distRb1 , out->level1.distRb1 , 0, "distRb1");
  VALIDATE_TST_VEC(in->level1.distRb2 , out->level1.distRb2 , 0, "distRb2");
  VALIDATE_TST_VEC(in->level1.distRb3 , out->level1.distRb3 , 0, "distRb3");
  VALIDATE_TST_VEC(in->level1.distRb4 , out->level1.distRb4 , 0, "distRb4");
  VALIDATE_TST_VEC(in->level1.minMaxSelRb0 , out->level1.minMaxSelRb0 , 0, "minMaxSelRb0");
  VALIDATE_TST_VEC(in->level1.minMaxSelRb1 , out->level1.minMaxSelRb1 , 0, "minMaxSelRb1");

  /*Channel CFG, GR/GB/R/B */
  vfe_abf_test_channel_cfg(&in->grCfg, &out->grCfg, "CH_GR");
  vfe_abf_test_channel_cfg(&in->gbCfg, &out->gbCfg, "CH_GB");
  vfe_abf_test_channel_cfg(&in->rCfg, &out->rCfg, "CH_R");
  vfe_abf_test_channel_cfg(&in->bCfg, &out->bCfg, "CH_B");

  /*SPATIAL_CFG_9*/
  /*CFG_0*/
  VALIDATE_TST_VEC(in->spCfg.init_CellNum_X_L , out->spCfg.init_CellNum_X_L , 0, "init_CellNum_X_L");
  VALIDATE_TST_VEC(in->spCfg.init_CellNum_X_R , out->spCfg.init_CellNum_X_R , 0, "init_CellNum_X_R");
  VALIDATE_TST_VEC(in->spCfg.init_CellNum_Y_L , out->spCfg.init_CellNum_Y_L , 0, "init_CellNum_Y_L");
  VALIDATE_TST_VEC(in->spCfg.init_CellNum_Y_R , out->spCfg.init_CellNum_Y_R , 0, "init_CellNum_Y_R");
  /*CFG_1*/
  VALIDATE_TST_VEC(in->spCfg.init_DX_L , out->spCfg.init_DX_L , 0, "init_DX_L");
  VALIDATE_TST_VEC(in->spCfg.init_DX_R , out->spCfg.init_DX_R , 0, "init_DX_R");
  /*CFG_2*/
  VALIDATE_TST_VEC(in->spCfg.init_DY_L , out->spCfg.init_DY_L , 0, "init_DY_L");
  VALIDATE_TST_VEC(in->spCfg.init_DY_R , out->spCfg.init_DY_R , 0, "init_DY_R");
  /*CFG_3*/
  VALIDATE_TST_VEC(in->spCfg.init_PX_L , out->spCfg.init_PX_L , 0, "init_PX_L");
  VALIDATE_TST_VEC(in->spCfg.init_PX_R , out->spCfg.init_PX_R , 0, "init_PX_R");
  /*CFG_4*/
  VALIDATE_TST_VEC(in->spCfg.init_PY_L , out->spCfg.init_PY_L , 0, "init_PY_L");
  VALIDATE_TST_VEC(in->spCfg.init_PY_R , out->spCfg.init_PY_R , 0, "init_PY_R");
  /*CFG_5*/
  VALIDATE_TST_VEC(in->spCfg.inv_CellWidth_L , out->spCfg.inv_CellWidth_L , 0, "inv_CellWidth_L");
  VALIDATE_TST_VEC(in->spCfg.inv_CellWidth_R , out->spCfg.inv_CellWidth_R , 0, "inv_CellWidth_R");
  /*CFG_6*/
  VALIDATE_TST_VEC(in->spCfg.inv_CellHeight_L , out->spCfg.inv_CellHeight_L , 0, "inv_CellHeight_L");
  VALIDATE_TST_VEC(in->spCfg.inv_CellHeight_R , out->spCfg.inv_CellHeight_R , 0, "inv_CellHeight_R");
  /*CFG_7*/
  VALIDATE_TST_VEC(in->spCfg.cellWidth_L , out->spCfg.cellWidth_L , 0, "cellWidth_L");
  VALIDATE_TST_VEC(in->spCfg.cellWidth_R , out->spCfg.cellWidth_R , 0, "cellWidth_R");
  /*CFG_8*/
  VALIDATE_TST_VEC(in->spCfg.cellHeight_L , out->spCfg.cellHeight_L , 0, "cellHeight_L");
  VALIDATE_TST_VEC(in->spCfg.cellHeight_R , out->spCfg.cellHeight_R , 0, "cellHeight_R");

  abf_validate_tables(&mod_in->abf_sig2_l0, &mod_op->abf_sig2_l0, "abf_sig2_l0");
  abf_validate_tables(&mod_in->abf_sig2_l1, &mod_op->abf_sig2_l1, "abf_sig2_l1");
  abf_validate_tables(&mod_in->abf_std2_l0, &mod_in->abf_std2_l0, "abf_std2_l0");
  abf_validate_tables(&mod_in->abf_std2_l1, &mod_op->abf_std2_l1, "abf_std2_l1");
  abf_validate_tables(&mod_in->abf_mesh_2D, &mod_op->abf_mesh_2D, "abf_mesh_2D");
  return 0;
}

