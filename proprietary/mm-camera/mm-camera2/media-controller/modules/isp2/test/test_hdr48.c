/*============================================================================
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/
#include "vfe_test_vector.h"
#include "hdr_reg.h"
#include "hdr48.h"
#include "camera_dbg.h"
#include "isp_pipeline_reg.h"
#include <unistd.h>

/*===========================================================================
 * FUNCTION    --hdr_populate_data -
 *
 * DESCRIPTION:
 *==========================================================================*/
void hdr_populate_data(uint32_t *reg, hdr_reg_cmd_t *pcmd)
{
  int size = 0;
  uint8_t *ptr = NULL;
  uint32_t* reg_store = NULL;

  reg_store = reg;

  size = ISP_HDR_CFG_LEN * sizeof(uint32_t);
  ptr = (uint8_t *)&pcmd->cfg_cmd;
  reg = reg_store + (ISP_HDR_CFG/4);
  memcpy((void *)ptr, (void *)reg, size);

  size = ISP_HDR_CFG2_LEN * sizeof(uint32_t);
  ptr = (uint8_t *)&pcmd->cfg_cmd2;
  reg = reg_store + (ISP_HDR_CFG2/4);
  memcpy((void *)ptr, (void *)reg, size);

  size = ISP_HDR_RECON_LEN * sizeof(uint32_t);
  ptr = (uint8_t *)&pcmd->recon_cmd;
  reg = reg_store + (ISP_HDR_RECON_OFF/4);
  memcpy((void *)ptr, (void *)reg, size);

  size = ISP_HDR_RECON2_LEN * sizeof(uint32_t);
  ptr = (uint8_t *)&pcmd->recon_cmd2;
  reg = reg_store + (ISP_HDR_RECON2_OFF/4);
  memcpy((void *)ptr, (void *)reg, size);

  size = ISP_HDR_MAC_LEN * sizeof(uint32_t);
  ptr = (uint8_t *)&pcmd->mac_cmd;
  reg = reg_store + (ISP_HDR_MAC_OFF/4);
  memcpy((void *)ptr, (void *)reg, size);
}

/*===========================================================================
 * FUNCTION    - vfe_hdr_tv_validate -
 *
 * DESCRIPTION:
 *==========================================================================*/
int vfe_hdr_tv_validate(void *in, void *op)
{
  uint32_t i;
  hdr_reg_cmd_t in_hdr, out_hdr;
  vfe_test_module_input_t *mod_in = (vfe_test_module_input_t *)in;
  vfe_test_module_output_t *mod_op = (vfe_test_module_output_t *)op;

  ISP_ModuleEn *module_enable_reg_in  = NULL;
  ISP_ModuleEn *module_enable_reg_out = NULL;

  module_enable_reg_in  = (ISP_ModuleEn *)(mod_in->reg_dump + (ISP_MODULE_LENS_EN_OFFSET/4));
  module_enable_reg_out = (ISP_ModuleEn *)(mod_op->reg_dump + (ISP_MODULE_LENS_EN_OFFSET/4));

  if ((module_enable_reg_out->fields.hdr_recon == 0) &&
      (module_enable_reg_in->fields.hdr_recon  == 0)) {
    printf("\t(HDR disabled in MODULE_LENS_EN)\n");
    return 0;
  }

  ALOGE("%s @@@@ vfe_hdr_tv_validate", __func__);


  hdr_populate_data(mod_in->reg_dump, &in_hdr);
  hdr_populate_data(mod_op->reg_dump, &out_hdr);

  VALIDATE_TST_VEC(in_hdr.cfg_cmd.cfg0.recon_first_field,
                   out_hdr.cfg_cmd.cfg0.recon_first_field,
                   0, "recon_first_field");
  VALIDATE_TST_VEC(in_hdr.cfg_cmd.cfg0.exp_ratio,
                   out_hdr.cfg_cmd.cfg0.exp_ratio,
                   0, "exp_ratio");

  VALIDATE_TST_VEC(in_hdr.cfg_cmd.cfg1.rg_wb_gain_ratio,
                   out_hdr.cfg_cmd.cfg1.rg_wb_gain_ratio,
                   0, "rg_wb_gain_ratio");

  VALIDATE_TST_VEC(in_hdr.cfg_cmd.cfg2.bg_wb_gain_ratio,
                   out_hdr.cfg_cmd.cfg2.bg_wb_gain_ratio,
                   0, "bg_wb_gain_ratio");

  VALIDATE_TST_VEC(in_hdr.cfg_cmd.cfg3.gr_wb_gain_ratio,
                   out_hdr.cfg_cmd.cfg3.gr_wb_gain_ratio,
                   0, "gr_wb_gain_ratio");

  VALIDATE_TST_VEC(in_hdr.cfg_cmd.cfg4.gb_wb_gain_ratio,
                   out_hdr.cfg_cmd.cfg4.gb_wb_gain_ratio,
                   0, "gb_wb_gain_ratio");

  VALIDATE_TST_VEC(in_hdr.cfg_cmd2.cfg5.blk_in,
                   out_hdr.cfg_cmd2.cfg5.blk_in,
                   0, "blk_in");
  VALIDATE_TST_VEC(in_hdr.cfg_cmd2.cfg5.blk_out,
                   out_hdr.cfg_cmd2.cfg5.blk_out,
                   0, "blk_out");

  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg0.recon_h_edge_dth_log2,
                   out_hdr.recon_cmd.recon_cfg0.recon_h_edge_dth_log2,
                   0, "recon_h_edge_dth_log2");
  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg0.recon_h_edge_th1,
                   out_hdr.recon_cmd.recon_cfg0.recon_h_edge_th1,
                   0, "recon_h_edge_th1");
  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg0.recon_motion_dth_log2,
                   out_hdr.recon_cmd.recon_cfg0.recon_motion_dth_log2,
                   0, "recon_motion_dth_log2");
  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg0.recon_motion_th1,
                   out_hdr.recon_cmd.recon_cfg0.recon_motion_th1,
                   0, "recon_motion_th1");

  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg1.recon_dark_dth_log2,
                   out_hdr.recon_cmd.recon_cfg1.recon_dark_dth_log2,
                   0, "recon_dark_dth_log2");
  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg1.recon_dark_th1,
                   out_hdr.recon_cmd.recon_cfg1.recon_dark_th1,
                   0, "recon_dark_th1");
  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg1.recon_edge_lpf_tap0,
                   out_hdr.recon_cmd.recon_cfg1.recon_edge_lpf_tap0,
                   0, "recon_edge_lpf_tap0");
  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg1.recon_flat_region_th,
                   out_hdr.recon_cmd.recon_cfg1.recon_flat_region_th,
                   0, "recon_flat_region_th");

  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg2.r_recon_h_edge_dth_log2,
                   out_hdr.recon_cmd.recon_cfg2.r_recon_h_edge_dth_log2,
                   0, "r_recon_h_edge_dth_log2");
  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg2.r_recon_h_edge_th1,
                   out_hdr.recon_cmd.recon_cfg2.r_recon_h_edge_th1,
                   0, "r_recon_h_edge_th1");
  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg2.r_recon_motion_dth_log2,
                   out_hdr.recon_cmd.recon_cfg2.r_recon_motion_dth_log2,
                   0, "r_recon_motion_dth_log2");
  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg2.r_recon_motion_th1,
                   out_hdr.recon_cmd.recon_cfg2.r_recon_motion_th1,
                   0, "r_recon_motion_th1");

  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg3.recon_linear_mode,
                   out_hdr.recon_cmd.recon_cfg3.recon_linear_mode,
                   0, "recon_linear_mode");
  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg3.recon_min_factor,
                   out_hdr.recon_cmd.recon_cfg3.recon_min_factor,
                   0, "recon_min_factor");
  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg3.r_recon_dark_dth_log2,
                   out_hdr.recon_cmd.recon_cfg3.r_recon_dark_dth_log2,
                   0, "r_recon_dark_dth_log2");
  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg3.r_recon_dark_th1,
                   out_hdr.recon_cmd.recon_cfg3.r_recon_dark_th1,
                   0, "r_recon_dark_th1");
  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg3.r_recon_min_factor,
                   out_hdr.recon_cmd.recon_cfg3.r_recon_min_factor,
                   0, "r_recon_min_factor");

  VALIDATE_TST_VEC(in_hdr.recon_cmd.recon_cfg4.r_recon_flat_region_th,
                   out_hdr.recon_cmd.recon_cfg4.r_recon_flat_region_th,
                   0, "r_recon_flat_region_th");

  ALOGE("%s @@@@ vfe_hdr_tv_validate 1", __func__);

  VALIDATE_TST_VEC(in_hdr.recon_cmd2.recon_cfg5.zrec_enable,
                   out_hdr.recon_cmd2.recon_cfg5.zrec_enable,
                   0, "zrec_enable");
  VALIDATE_TST_VEC(in_hdr.recon_cmd2.recon_cfg5.zrec_first_rb_exp,
                   out_hdr.recon_cmd2.recon_cfg5.zrec_first_rb_exp,
                   0, "zrec_first_rb_exp");
  VALIDATE_TST_VEC(in_hdr.recon_cmd2.recon_cfg5.zrec_pattern,
                   out_hdr.recon_cmd2.recon_cfg5.zrec_pattern,
                   0, "zrec_pattern");
  VALIDATE_TST_VEC(in_hdr.recon_cmd2.recon_cfg5.zrec_prefilt_tap0,
                   out_hdr.recon_cmd2.recon_cfg5.zrec_prefilt_tap0,
                   0, "zrec_prefilt_tap0");

  VALIDATE_TST_VEC(in_hdr.recon_cmd2.recon_cfg6.zrec_g_dth_log2,
                   out_hdr.recon_cmd2.recon_cfg6.zrec_g_dth_log2,
                   0, "zrec_g_dth_log2");
  VALIDATE_TST_VEC(in_hdr.recon_cmd2.recon_cfg6.zrec_g_grad_th1,
                   out_hdr.recon_cmd2.recon_cfg6.zrec_g_grad_th1,
                   0, "zrec_g_grad_th1");
  VALIDATE_TST_VEC(in_hdr.recon_cmd2.recon_cfg6.zrec_rb_dth_log2,
                   out_hdr.recon_cmd2.recon_cfg6.zrec_rb_dth_log2,
                   0, "zrec_rb_dth_log2");
  VALIDATE_TST_VEC(in_hdr.recon_cmd2.recon_cfg6.zrec_rb_grad_th1,
                   out_hdr.recon_cmd2.recon_cfg6.zrec_rb_grad_th1,
                   0, "zrec_rb_grad_th1");

  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg0.mac_motion_0_th1,
                   out_hdr.mac_cmd.mac_cfg0.mac_motion_0_th1,
                   0, "mac_motion_0_th1");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg0.mac_motion_0_th2,
                   out_hdr.mac_cmd.mac_cfg0.mac_motion_0_th2,
                   0, "mac_motion_0_th2");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg0.r_mac_motion_0_th1,
                   out_hdr.mac_cmd.mac_cfg0.r_mac_motion_0_th1,
                   0, "r_mac_motion_0_th1");

  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg1.mac_dilation,
                   out_hdr.mac_cmd.mac_cfg1.mac_dilation,
                   0, "mac_dilation");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg1.mac_sqrt_analog_gain,
                   out_hdr.mac_cmd.mac_cfg1.mac_sqrt_analog_gain,
                   0, "mac_sqrt_analog_gain");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg1.r_mac_motion_0_th2,
                   out_hdr.mac_cmd.mac_cfg1.r_mac_motion_0_th2,
                   0, "r_mac_motion_0_th2");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg1.r_mac_sqrt_analog_gain,
                   out_hdr.mac_cmd.mac_cfg1.r_mac_sqrt_analog_gain,
                   0, "r_mac_sqrt_analog_gain");

  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg2.mac_motion_0_dt0,
                   out_hdr.mac_cmd.mac_cfg2.mac_motion_0_dt0,
                   0, "mac_motion_0_dt0");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg2.mac_motion_strength,
                   out_hdr.mac_cmd.mac_cfg2.mac_motion_strength,
                   0, "mac_motion_strength");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg2.r_mac_motion_0_dt0,
                   out_hdr.mac_cmd.mac_cfg2.r_mac_motion_0_dt0,
                   0, "r_mac_motion_0_dt0");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg2.r_mac_motion_strength,
                   out_hdr.mac_cmd.mac_cfg2.r_mac_motion_strength,
                   0, "r_mac_motion_strength");

  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg3.mac_low_light_th1,
                   out_hdr.mac_cmd.mac_cfg3.mac_low_light_th1,
                   0, "mac_low_light_th1");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg3.r_mac_low_light_th1,
                   out_hdr.mac_cmd.mac_cfg3.r_mac_low_light_th1,
                   0, "r_mac_low_light_th1");

  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg4.mac_high_light_dth_log2,
                   out_hdr.mac_cmd.mac_cfg4.mac_high_light_dth_log2,
                   0, "mac_high_light_dth_log2");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg4.mac_low_light_dth_log2,
                   out_hdr.mac_cmd.mac_cfg4.mac_low_light_dth_log2,
                   0, "mac_low_light_dth_log2");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg4.mac_low_light_strength,
                   out_hdr.mac_cmd.mac_cfg4.mac_low_light_strength,
                   0, "mac_low_light_strength");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg4.r_mac_high_light_dth_log2,
                   out_hdr.mac_cmd.mac_cfg4.r_mac_high_light_dth_log2,
                   0, "r_mac_high_light_dth_log2");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg4.r_mac_low_light_dth_log2,
                   out_hdr.mac_cmd.mac_cfg4.r_mac_low_light_dth_log2,
                   0, "r_mac_low_light_dth_log2");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg4.r_mac_low_light_strength,
                   out_hdr.mac_cmd.mac_cfg4.r_mac_low_light_strength,
                   0, "r_mac_low_light_strength");

  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg5.mac_high_light_th1,
                   out_hdr.mac_cmd.mac_cfg5.mac_high_light_th1,
                   0, "mac_high_light_th1");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg5.r_mac_high_light_th1,
                   out_hdr.mac_cmd.mac_cfg5.r_mac_high_light_th1,
                   0, "r_mac_high_light_th1");

  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg6.mac_smooth_dth_log2,
                   out_hdr.mac_cmd.mac_cfg6.mac_smooth_dth_log2,
                   0, "mac_smooth_dth_log2");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg6.mac_smooth_enable,
                   out_hdr.mac_cmd.mac_cfg6.mac_smooth_enable,
                   0, "mac_smooth_enable");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg6.mac_smooth_th1,
                   out_hdr.mac_cmd.mac_cfg6.mac_smooth_th1,
                   0, "mac_smooth_th1");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg6.r_mac_smooth_dth_log2,
                   out_hdr.mac_cmd.mac_cfg6.r_mac_smooth_dth_log2,
                   0, "r_mac_smooth_dth_log2");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg6.r_mac_smooth_th1,
                   out_hdr.mac_cmd.mac_cfg6.r_mac_smooth_th1,
                   0, "r_mac_smooth_th1");


  ALOGE("%s @@@@ vfe_hdr_tv_validate 2", __func__);

  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg7.exp_ratio_recip,
                   out_hdr.mac_cmd.mac_cfg7.exp_ratio_recip,
                   0, "exp_ratio_recip");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg7.mac_linear_mode,
                   out_hdr.mac_cmd.mac_cfg7.mac_linear_mode,
                   0, "mac_linear_mode");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg7.mac_smooth_tap0,
                   out_hdr.mac_cmd.mac_cfg7.mac_smooth_tap0,
                   0, "exp_ratio_recip");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg7.msb_aligned,
                   out_hdr.mac_cmd.mac_cfg7.msb_aligned,
                   0, "msb_aligned");
  VALIDATE_TST_VEC(in_hdr.mac_cmd.mac_cfg7.r_mac_smooth_tap0,
                   out_hdr.mac_cmd.mac_cfg7.r_mac_smooth_tap0,
                   0, "r_mac_smooth_tap0");


  ALOGE("%s @@@@ vfe_hdr_tv_validate 3", __func__);

  return 0;
} /*vfe_hdr_tv_validate*/

