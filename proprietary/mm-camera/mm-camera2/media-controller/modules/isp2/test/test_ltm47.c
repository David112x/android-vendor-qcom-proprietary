/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "vfe_test_vector.h"
#include "ltm/adrc/ltm_algo.h"
#include "ltm/ltm47/ltm47.h"
#include "ltm_reg.h"
#include "camera_dbg.h"
#include <unistd.h>

void ltm_validate_cfg(ISP_LTMCfg *in, ISP_LTMCfg
  *out)
{
  /*  auto switch and bank select doesnt need to verify, flase alarm*/
  /* VALIDATE_TST_VEC(in->autoSwitchOverride, out->autoSwitchOverride, 0, "autoSwitchOverride"); */
  /* VALIDATE_TST_VEC(in->lutBankSelect, out->lutBankSelect, 0, "lutBankSelect"); */
  VALIDATE_TST_VEC(in->hMnEnable, out->hMnEnable, 0, "hMnEnable");
  VALIDATE_TST_VEC(in->vMnEnable, out->vMnEnable, 0, "vMnEnable");
  /* LTM is disabled in first frames so ipRnable will mismatch. Ignore it */
// VALIDATE_TST_VEC(in->ipEnable, out->ipEnable, 0, "ipEnable");
  VALIDATE_TST_VEC(in->dcEnable, out->dcEnable, 0, "dcEnable");
  VALIDATE_TST_VEC(in->dc3dAvgPongSel, out->dc3dAvgPongSel, 0, "dc3dAvgPongSel");
  VALIDATE_TST_VEC(in->dc3dSumClear, out->dc3dSumClear, 0, "dc3dSumClear");
  VALIDATE_TST_VEC(in->ip3dAvgPongSel, out->ip3dAvgPongSel, 0, "ip3dAvgPongSel");
  VALIDATE_TST_VEC(in->debugOutSelect, out->debugOutSelect, 0, "debugOutSelect");
  VALIDATE_TST_VEC(in->dc_initCellNumX, out->dc_initCellNumX, 0, "dc_initCellNumX");
  VALIDATE_TST_VEC(in->ip_initCellNumX, out->ip_initCellNumX, 0, "ip_initCellNumX");
}/*ltm_validate_nz_flag*/

void ltm_validate_cfg_status( ISP_LTMStatus *in, ISP_LTMStatus
  *out)
{
  VALIDATE_TST_VEC(in->dc3dAvgPong, out->dc3dAvgPong, 0, "dc3dAvgPong");
}


void ltm_validate_himage_cfg( ISP_LTMScaleHImageSizeCfg *in, ISP_LTMScaleHImageSizeCfg
  *out)
{
  VALIDATE_TST_VEC(in->hIn, out->hIn, 0, "hIn");
  VALIDATE_TST_VEC(in->hOut, out->hOut, 0, "hOut");
}

void ltm_validate_vimage_cfg( ISP_LTMScaleVImageSizeCfg *in, ISP_LTMScaleVImageSizeCfg
  *out)
{
  VALIDATE_TST_VEC(in->vIn, out->vIn, 0, "vIn");
  VALIDATE_TST_VEC(in->vOut, out->vOut, 0, "vOut");
}

void ltm_validate_hphase_cfg( ISP_LTMScaleHPhaseCfg *in, ISP_LTMScaleHPhaseCfg
  *out)
{
  VALIDATE_TST_VEC(in->hInterpReso, out->hInterpReso, 0, "hInterpReso");
  VALIDATE_TST_VEC(in->hPhaseMult, out->hPhaseMult, 0, "hPhaseMult");
}

void ltm_validate_vphase_cfg( ISP_LTMScaleVPhaseCfg *in, ISP_LTMScaleVPhaseCfg
  *out)
{
  VALIDATE_TST_VEC(in->vInterpReso, out->vInterpReso, 0, "vInterpReso");
  VALIDATE_TST_VEC(in->vPhaseMult, out->vPhaseMult, 0, "vPhaseMult");
}

void ltm_validate_hstripe_cfg( ISP_LTMScaleHStripeCfg *in, ISP_LTMScaleHStripeCfg
  *out)
{
  VALIDATE_TST_VEC(in->hMnInit, out->hMnInit, 0, "hMnInit");
    VALIDATE_TST_VEC(in->hPhaseInit, out->hPhaseInit, 0, "hPhaseInit");
}

void ltm_validate_vstripe_cfg( ISP_LTMScaleVStripeCfg *in, ISP_LTMScaleVStripeCfg
  *out)
{
  VALIDATE_TST_VEC(in->vMnInit, out->vMnInit, 0, "vMnInit");
  VALIDATE_TST_VEC(in->vPhaseInit, out->vPhaseInit, 0, "vPhaseInit");

}


/*===========================================================================
 * FUNCTION    - ltm_validate_coeff_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void ltm_validate_dcip_cfg(isp_ltm_dcip_cfg_t *in, isp_ltm_dcip_cfg_t
  *out)
{
  VALIDATE_TST_VEC(in->dc_cfg.initDX, out->dc_cfg.initDX, 0, "initDX");
  VALIDATE_TST_VEC(in->dc_cfg.binInitCnt, out->dc_cfg.binInitCnt, 0, "binInitCnt");
//  VALIDATE_TST_VEC(in->ip_cfg.yRatioMax, out->ip_cfg.yRatioMax, 0, "y_ratio_max");
  VALIDATE_TST_VEC(in->ip_init.initDX, out->ip_init.initDX, 0, "IPinitDX");
  VALIDATE_TST_VEC(in->ip_init.initPX, out->ip_init.initPX, 0, "IPinitPX");
  VALIDATE_TST_VEC(in->ip_inv_cellwidth.invCellwidthL, out->ip_inv_cellwidth.invCellwidthL, 0, "invCellwidthL");
  VALIDATE_TST_VEC(in->ip_inv_cellwidth.invCellwidthR, out->ip_inv_cellwidth.invCellwidthR, 0, "invCellwidthR");
  VALIDATE_TST_VEC(in->ip_inv_cellheight.invCellheightL, out->ip_inv_cellheight.invCellheightL, 0, "invCellwidthL");
  VALIDATE_TST_VEC(in->ip_inv_cellheight.invCellheightR, out->ip_inv_cellheight.invCellheightR, 0, "invCellheightR");
  VALIDATE_TST_VEC(in->ip_cellwidth.cellwidthL, out->ip_cellwidth.cellwidthL, 0, "CellwidthL");
  VALIDATE_TST_VEC(in->ip_cellwidth.cellwidthR, out->ip_cellwidth.cellwidthR, 0, "cellwidthR");
  VALIDATE_TST_VEC(in->ip_cellheight.cellheightL, out->ip_cellheight.cellheightL, 0, "cellheightL");
  VALIDATE_TST_VEC(in->ip_cellheight.cellheightR, out->ip_cellheight.cellheightR, 0, "cellheightR");
}/*ltm_validate_coeff_cfg*/

/*===========================================================================
 * FUNCTION    - ltm_validate_threshold_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void ltm_validate_rgb2y_cfg(isp_ltm_rgb2y_cfg_t *in, isp_ltm_rgb2y_cfg_t
  *out)
{
  VALIDATE_TST_VEC(in->rgb2y_cfg0.c1,   out->rgb2y_cfg0.c1,   0, "C1");
  VALIDATE_TST_VEC(in->rgb2y_cfg0.c2,   out->rgb2y_cfg0.c2,   0, "C2");
  VALIDATE_TST_VEC(in->rgb2y_cfg0.c3,   out->rgb2y_cfg0.c3,   0, "C3");
  VALIDATE_TST_VEC(in->rgb2y_cfg1.c4,   out->rgb2y_cfg1.c4,   0, "C4");
  VALIDATE_TST_VEC(in->rgb2y_cfg1.k,   out->rgb2y_cfg1.k,   0, "K");
  VALIDATE_TST_VEC(in->rgb2y_cfg1.thr,   out->rgb2y_cfg1.thr,   0, "thr");
//  VALIDATE_TST_VEC(in->rgb2y_cfg2.c5,   out->rgb2y_cfg2.c5,   0, "C5");
}/*ltm_validate_threshold_cfg*/

/*===========================================================================
 * FUNCTION    - ltm_validate_interp_slope_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void ltm_validate_hpad_cfg(ISP_LTMScaleHPadCfg *in, ISP_LTMScaleHPadCfg
  *out)
{
  VALIDATE_TST_VEC(in->hSkipCnt, out->hSkipCnt, 0, "hSkipCnt");
  VALIDATE_TST_VEC(in->scaleYInWidth, out->scaleYInWidth, 0, "scaleYInWidth");

}/*ltm_validate_interp_slope_cfg*/

/*===========================================================================
 * FUNCTION    - ltm_validate_interp_slope_cfg -
 *
 * DESCRIPTION:
 *==========================================================================*/
void ltm_validate_vpad_cfg(ISP_LTMScaleVPadCfg *in, ISP_LTMScaleVPadCfg
  *out)
{
  VALIDATE_TST_VEC(in->vSkipCnt, out->vSkipCnt, 0, "vSkipCnt");
  VALIDATE_TST_VEC(in->scaleYInHeight, out->scaleYInHeight, 0, "scaleYInHeight");

}/*ltm_validate_interp_slope_cfg*/

void ltm_populate_hpad_data(uint32_t *reg, ISP_LTMScaleHPadCfg *pcmd)
{
  void *dst = (void *)pcmd;
  uint32_t size = 1 * sizeof(uint32_t);
  memcpy(dst, (void *)(reg + ISP_LTM_H_PAD_OFF/4), size);

}

void ltm_populate_vpad_data(uint32_t *reg, ISP_LTMScaleVPadCfg *pcmd)
{
  void *dst = (void *)pcmd;
  uint32_t size = 1 * sizeof(uint32_t);
  memcpy(dst, (void *)(reg + (ISP_LTM_V_PAD_OFF)/4), size);

}

void ltm_populate_HScale_data(uint32_t *reg, isp_ltm_h_scale_cfg_t *pcmd)
{
  void *dst = (void *)pcmd;
  uint32_t size = 4 * sizeof(uint32_t);
  memcpy(dst, (void *)(reg + ISP_LTM_H_SCALE_OFF/4), size);

}

void ltm_populate_VScale_data(uint32_t *reg, isp_ltm_v_scale_cfg_t *pcmd)
{
  void *dst = (void *)pcmd;
  uint32_t size = 4 * sizeof(uint32_t);
  memcpy(dst, (void *)(reg + ISP_LTM_V_SCALE_OFF/4), size);
}

void ltm_populate_DCIP_data(uint32_t *reg, isp_ltm_dcip_cfg_t *pcmd)
{
  void *dst = (void *)pcmd;
  uint32_t size = 7 * sizeof(uint32_t);
  memcpy(dst, (void *)(reg + ISP_LTM_DCIP_OFF/4), size);
}

void ltm_populate_RGB2_data(uint32_t *reg, isp_ltm_rgb2y_cfg_t *pcmd)
{
  void *dst = (void *)pcmd;
  uint32_t size = 3 * sizeof(uint32_t);
  memcpy(dst, (void *)(reg + ISP_LTM_RGB2Y_OFF/4), size);

}

/*===========================================================================
 * FUNCTION    - ltm_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void ltm_populate_data(uint32_t *reg, isp_ltm_reg_t *pcmd)
{
  void *dst = (void *)pcmd;
  uint32_t size = 2* sizeof(uint32_t);
  memcpy(dst, (void *)(reg + ISP_LTM_CFG_OFF/4), size);
  ltm_populate_hpad_data(reg,&pcmd->ltm_scale_h_pad_cfg);
  ltm_populate_vpad_data(reg,&pcmd->ltm_scale_v_pad_cfg);
  ltm_populate_HScale_data(reg,&pcmd->ltm_h_scale_cfg);
  ltm_populate_VScale_data(reg,&pcmd->ltm_v_scale_cfg);
  ltm_populate_DCIP_data(reg,&pcmd->ltm_dcip_cfg);
  ltm_populate_RGB2_data(reg,&pcmd->ltm_rgb2y_cfg);
}

void ltm_validate_table(
  vfe_test_table_t *in, vfe_test_table_t *out, uint32_t size, char* channel)
{
  uint32_t i;
  for (i = 0; i < size; i++) {
    VALIDATE_TST_LUT(in->table[i], out->table[i], 0, channel, i);
  }
}/*mesh_rolloff_validate_rb_cfg*/

/*===========================================================================
 * FUNCTION    - vfe_ltm_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_ltm_tv_validate(void* test_input,
  void* test_output)
{
  vfe_test_module_input_t *input = (vfe_test_module_input_t *)test_input;
  vfe_test_module_output_t *output = (vfe_test_module_output_t *)test_output;
  isp_ltm_reg_t in, out;

  ltm_populate_data(input->reg_dump, &in);
  ltm_populate_data(output->reg_dump, &out);

  ltm_validate_cfg(&in.ltm_cfg, &out.ltm_cfg);
  ltm_validate_himage_cfg(&in.ltm_h_scale_cfg.h_image_size, &out.ltm_h_scale_cfg.h_image_size);
  ltm_validate_vimage_cfg(&in.ltm_v_scale_cfg.v_image_size, &out.ltm_v_scale_cfg.v_image_size);
  ltm_validate_hphase_cfg(&in.ltm_h_scale_cfg.h_phase, &out.ltm_h_scale_cfg.h_phase);
  ltm_validate_hstripe_cfg(&in.ltm_h_scale_cfg.h_stripe, &out.ltm_h_scale_cfg.h_stripe);
  ltm_validate_vphase_cfg(&in.ltm_v_scale_cfg.v_phase, &out.ltm_v_scale_cfg.v_phase);
  ltm_validate_vstripe_cfg(&in.ltm_v_scale_cfg.v_stripe, &out.ltm_v_scale_cfg.v_stripe);
  ltm_validate_dcip_cfg(&in.ltm_dcip_cfg, &out.ltm_dcip_cfg);
  ltm_validate_rgb2y_cfg(&in.ltm_rgb2y_cfg, &out.ltm_rgb2y_cfg);
  ltm_validate_vpad_cfg(&in.ltm_scale_v_pad_cfg, &out.ltm_scale_v_pad_cfg);
  ltm_validate_hpad_cfg(&in.ltm_scale_h_pad_cfg, &out.ltm_scale_h_pad_cfg);

  printf("****Compare weight**********\n");
  ltm_validate_table(&input->ltm_weight, &output->ltm_weight,LTM_HW_W_LUT_SIZE, "weight");
  printf("****Compare ltm_mask_curve**********\n");
  ltm_validate_table(&input->ltm_mask_curve, &output->ltm_mask_curve, LTM_HW_LUT_SIZE,"ltm_mask_curve");
  printf("****Compare ltm_master_curve**********\n");
  ltm_validate_table(&input->ltm_master_curve, &output->ltm_master_curve, LTM_HW_LUT_SIZE, "ltm_master_curve");
  printf("****Compare ltm_master_scale**********\n");
  ltm_validate_table(&input->ltm_master_scale, &output->ltm_master_scale,LTM_HW_LUT_SIZE,  "ltm_master_scale");
  printf("****Compare ltm_shift_curve**********\n");
  ltm_validate_table(&input->ltm_shift_curve, &output->ltm_shift_curve, LTM_HW_LUT_SIZE, "ltm_shift_curve");
  printf("****Compare ltm_shift_scale**********\n");
  ltm_validate_table(&input->ltm_shift_scale, &output->ltm_shift_scale,LTM_HW_LUT_SIZE, "ltm_shift_scale");
  printf("****Compare ltm_sat_curve**********\n");
  ltm_validate_table(&input->ltm_sat_curve, &output->ltm_sat_curve,LTM_HW_LUT_SIZE, "ltm_sat_curve");
  return 0;
}/*vfe_ltm_tv_validate*/

