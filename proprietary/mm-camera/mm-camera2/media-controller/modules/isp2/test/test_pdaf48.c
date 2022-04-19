/*============================================================================
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/
#include "vfe_test_vector.h"
#include "pdaf/pdaf48/pdaf48.h"
#include "pdaf_reg.h"
#include "camera_dbg.h"
#include "isp_pipeline_reg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    - vfe_pdaf_test_vector_validate -
 *
 * DESCRIPTION: this function compares the test vector output with hw output
 *==========================================================================*/
int vfe_pdaf_tv_validate(void *input, void *output)
{
  vfe_test_module_input_t *mod_in  = (vfe_test_module_input_t *)input;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)output;

  ISP_ModuleEn *module_enable_reg_in  = NULL;
  ISP_ModuleEn *module_enable_reg_out = NULL;

  ISP_PDAF_Reg_t  *in, *out;

  CDBG("%s:\n", __func__);

  in = (ISP_PDAF_Reg_t *)(mod_in->reg_dump + (PDAF_OFFSET/4));
  out = (ISP_PDAF_Reg_t *)(mod_op->reg_dump + (PDAF_OFFSET/4));

  module_enable_reg_in  = (ISP_ModuleEn *)(mod_in->reg_dump + (ISP_MODULE_LENS_EN_OFFSET/4));
  module_enable_reg_out = (ISP_ModuleEn *)(mod_op->reg_dump + (ISP_MODULE_LENS_EN_OFFSET/4));
/*
  if ((module_enable_reg_out->fields.pdaf == 0) &&
      (module_enable_reg_in->fields.pdaf  == 0)) {
    printf("\t(PDAF disabled in MODULE_LENS_EN)\n");
    return 0;
  }
*/
  /* PDAF_CFG*/

  VALIDATE_TST_VEC(in->pdaf_config.pdaf_pdpc_en ,
    out->pdaf_config.pdaf_pdpc_en , 0, "pdaf_pdpc_en");
  VALIDATE_TST_VEC(in->pdaf_config.pdaf_dsbpc_en ,
    out->pdaf_config.pdaf_dsbpc_en , 0, "pdaf_dsbpc_en");
/*
  VALIDATE_TST_VEC(in->pdaf_config.blk_lvl ,
    out->pdaf_config.blk_lvl , 0, "blk_lvl");
*/
  /* PDAF_HDR_EXP_RATIO*/
  VALIDATE_TST_VEC(in->hdr_config.hdr_exp_ratio.exp_ratio , out->hdr_config.hdr_exp_ratio.exp_ratio
     , 0, "hdr_exp_ratio");
  VALIDATE_TST_VEC(in->hdr_config.hdr_exp_ratio.exp_ratio_recip , out->hdr_config.hdr_exp_ratio.exp_ratio_recip
     , 0, "hdr_exp_ratio_recip");

  /* PDAF_BP_TH*/
  VALIDATE_TST_VEC(in->bp_config.bp_th.fmax , out->bp_config.bp_th.fmax
     , 0, "FMax");
  VALIDATE_TST_VEC(in->bp_config.bp_th.fmin , out->bp_config.bp_th.fmin
     , 0, "Fmin");

  /* PDAF_BP_OFFSET*/
  VALIDATE_TST_VEC(in->bp_config.bp_offset.offset_g_pixel , out->bp_config.bp_offset.offset_g_pixel
     , 0, "Offset_G_Pixel");
  VALIDATE_TST_VEC(in->bp_config.bp_offset.offset_rb_pixel , out->bp_config.bp_offset.offset_rb_pixel
     , 0, "Offset_RB_Pixel");

  /* PDAF_T2_BP_OFFSET*/
  VALIDATE_TST_VEC(in->bp_config.bp_offset_t2.offset_g_pixel , out->bp_config.bp_offset_t2.offset_g_pixel
     , 0, "T2_Offset_G_Pixel");
  VALIDATE_TST_VEC(in->bp_config.bp_offset_t2.offset_rb_pixel , out->bp_config.bp_offset_t2.offset_rb_pixel
     , 0, "T2_Offset_RB_Pixel");

  /* PDAF_RG_WB_GAIN*/
  VALIDATE_TST_VEC(in->wb_gain_config.rg_wb_gain.rg_wb_gain , out->wb_gain_config.rg_wb_gain.rg_wb_gain
     , 0, "RG_WB_GainRatio");

  /* PDAF_BG_WB_GAIN*/
  VALIDATE_TST_VEC(in->wb_gain_config.bg_wb_gain.bg_wb_gain , out->wb_gain_config.bg_wb_gain.bg_wb_gain
     , 0, "BG_WB_GainRatio");

  /* PDAF_GR_WB_GAIN*/
  VALIDATE_TST_VEC(in->wb_gain_config.gr_wb_gain.gr_wb_gain , out->wb_gain_config.gr_wb_gain.gr_wb_gain
     , 0, "GR_WB_GainRatio");

  /* PDAF_GB_WB_GAIN*/
  VALIDATE_TST_VEC(in->wb_gain_config.gb_wb_gain.gb_wb_gain , out->wb_gain_config.gb_wb_gain.gb_wb_gain
     , 0, "GB_WB_GainRatio");

  /* PDAF_LOC_OFFSET_CFG*/
  VALIDATE_TST_VEC(in->loc_config.loc_offset_cfg.x_offset , out->loc_config.loc_offset_cfg.x_offset, 0, "X_Offset");
  VALIDATE_TST_VEC(in->loc_config.loc_offset_cfg.y_offset , out->loc_config.loc_offset_cfg.y_offset, 0, "Y_Offset");

  /* PDAF_LOC_END_CFG*/
  VALIDATE_TST_VEC(in->loc_config.loc_end_cfg.x_end , out->loc_config.loc_end_cfg.x_end, 0, "X_End");
  VALIDATE_TST_VEC(in->loc_config.loc_end_cfg.x_end , out->loc_config.loc_end_cfg.x_end, 0, "Y_End");

  return 0;
}
