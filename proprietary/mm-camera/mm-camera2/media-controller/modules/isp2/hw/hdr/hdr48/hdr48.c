/* hdr48.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */

/* isp headers */
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "isp_pipeline_reg.h"
#include "hdr48.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_HDR, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_HDR, fmt, ##args)

static void hdr48_print_config(hdr_reg_cmd_t *reg_cmd)
{
  ISP_DBG("exp_ratio %x",reg_cmd->cfg_cmd.cfg0.exp_ratio);
  ISP_DBG("recon_first_field %x",
    reg_cmd->cfg_cmd.cfg0.recon_first_field);
  ISP_DBG("rg_wb_gain_ratio %x",
    reg_cmd->cfg_cmd.cfg1.rg_wb_gain_ratio);
  ISP_DBG("bg_wb_gain_ratio %x",
    reg_cmd->cfg_cmd.cfg2.bg_wb_gain_ratio);
  ISP_DBG("gr_wb_gain_ratio %x",
    reg_cmd->cfg_cmd.cfg3.gr_wb_gain_ratio);
  ISP_DBG("gb_wb_gain_ratio %x",
    reg_cmd->cfg_cmd.cfg4.gb_wb_gain_ratio);
  ISP_DBG("blk_in %x",
    reg_cmd->cfg_cmd2.cfg5.blk_in);
  ISP_DBG("blk_out %x",
    reg_cmd->cfg_cmd2.cfg5.blk_out);

  ISP_DBG("recon_h_edge_dth_log2 %x",
    reg_cmd->recon_cmd.recon_cfg0.recon_h_edge_dth_log2);
  ISP_DBG("recon_h_edge_th1 %x",
    reg_cmd->recon_cmd.recon_cfg0.recon_h_edge_th1);
  ISP_DBG("recon_motion_dth_log2 %x",
    reg_cmd->recon_cmd.recon_cfg0.recon_motion_dth_log2);
  ISP_DBG("recon_motion_th1 %x",
    reg_cmd->recon_cmd.recon_cfg0.recon_motion_th1);

  ISP_DBG("recon_dark_dth_log2 %x",
    reg_cmd->recon_cmd.recon_cfg1.recon_dark_dth_log2);
  ISP_DBG("recon_dark_th1 %x",
    reg_cmd->recon_cmd.recon_cfg1.recon_dark_th1);
  ISP_DBG("recon_edge_lpf_tap0 %x",
    reg_cmd->recon_cmd.recon_cfg1.recon_edge_lpf_tap0);
  ISP_DBG("recon_flat_region_th %x",
    reg_cmd->recon_cmd.recon_cfg1.recon_flat_region_th);

  ISP_DBG("r_recon_h_edge_dth_log2 %x",
    reg_cmd->recon_cmd.recon_cfg2.r_recon_h_edge_dth_log2);
  ISP_DBG("r_recon_h_edge_th1 %x",
    reg_cmd->recon_cmd.recon_cfg2.r_recon_h_edge_th1);
  ISP_DBG("r_recon_motion_dth_log2 %x",
    reg_cmd->recon_cmd.recon_cfg2.r_recon_motion_dth_log2);
  ISP_DBG("r_recon_motion_th1 %x",
    reg_cmd->recon_cmd.recon_cfg2.r_recon_motion_th1);

  ISP_DBG("recon_linear_mode %x",
    reg_cmd->recon_cmd.recon_cfg3.recon_linear_mode);
  ISP_DBG("recon_min_factor %x",
    reg_cmd->recon_cmd.recon_cfg3.recon_min_factor);
  ISP_DBG("r_recon_dark_dth_log2 %x",
    reg_cmd->recon_cmd.recon_cfg3.r_recon_dark_dth_log2);
  ISP_DBG("r_recon_dark_th1 %x",
    reg_cmd->recon_cmd.recon_cfg3.r_recon_dark_th1);
  ISP_DBG("r_recon_min_factor %x",
    reg_cmd->recon_cmd.recon_cfg3.r_recon_min_factor);

  ISP_DBG("r_recon_flat_region_th %x",
    reg_cmd->recon_cmd.recon_cfg4.r_recon_flat_region_th);

  ISP_DBG("zrec_enable %x",
    reg_cmd->recon_cmd2.recon_cfg5.zrec_enable);
  ISP_DBG("zrec_first_rb_exp %x",
    reg_cmd->recon_cmd2.recon_cfg5.zrec_first_rb_exp);
  ISP_DBG("zrec_pattern %x",
    reg_cmd->recon_cmd2.recon_cfg5.zrec_pattern);
  ISP_DBG("zrec_prefilt_tap0 %x",
    reg_cmd->recon_cmd2.recon_cfg5.zrec_prefilt_tap0);

  ISP_DBG("zrec_g_dth_log2 %x",
    reg_cmd->recon_cmd2.recon_cfg6.zrec_g_dth_log2);
  ISP_DBG("zrec_g_grad_th1 %x",
    reg_cmd->recon_cmd2.recon_cfg6.zrec_g_grad_th1);
  ISP_DBG("zrec_rb_dth_log2 %x",
    reg_cmd->recon_cmd2.recon_cfg6.zrec_rb_dth_log2);
  ISP_DBG("zrec_rb_grad_th1 %x",
    reg_cmd->recon_cmd2.recon_cfg6.zrec_rb_grad_th1);

  ISP_DBG("mac_motion_0_th1 %x",
    reg_cmd->mac_cmd.mac_cfg0.mac_motion_0_th1);
  ISP_DBG("mac_motion_0_th2 %x",
    reg_cmd->mac_cmd.mac_cfg0.mac_motion_0_th2);
  ISP_DBG("r_mac_motion_0_th1 %x",
    reg_cmd->mac_cmd.mac_cfg0.r_mac_motion_0_th1);

  ISP_DBG("mac_dilation %x",
    reg_cmd->mac_cmd.mac_cfg1.mac_dilation);
  ISP_DBG("mac_sqrt_analog_gain %x",
    reg_cmd->mac_cmd.mac_cfg1.mac_sqrt_analog_gain);
  ISP_DBG("r_mac_motion_0_th2 %x",
    reg_cmd->mac_cmd.mac_cfg1.r_mac_motion_0_th2);
  ISP_DBG("r_mac_sqrt_analog_gain %x",
    reg_cmd->mac_cmd.mac_cfg1.r_mac_sqrt_analog_gain);

  ISP_DBG("mac_motion_0_dt0 %x",
    reg_cmd->mac_cmd.mac_cfg2.mac_motion_0_dt0);
  ISP_DBG("mac_motion_strength %x",
    reg_cmd->mac_cmd.mac_cfg2.mac_motion_strength);
  ISP_DBG("r_mac_motion_0_dt0 %x",
    reg_cmd->mac_cmd.mac_cfg2.r_mac_motion_0_dt0);
  ISP_DBG("r_mac_motion_strength %x",
    reg_cmd->mac_cmd.mac_cfg2.r_mac_motion_strength);

  ISP_DBG("mac_low_light_th1 %x",
    reg_cmd->mac_cmd.mac_cfg3.mac_low_light_th1);
  ISP_DBG("r_mac_low_light_th1 %x",
    reg_cmd->mac_cmd.mac_cfg3.r_mac_low_light_th1);

  ISP_DBG("mac_high_light_dth_log2 %x",
    reg_cmd->mac_cmd.mac_cfg4.mac_high_light_dth_log2);
  ISP_DBG("mac_low_light_dth_log2 %x",
    reg_cmd->mac_cmd.mac_cfg4.mac_low_light_dth_log2);
  ISP_DBG("mac_low_light_strength %x",
    reg_cmd->mac_cmd.mac_cfg4.mac_low_light_strength);
  ISP_DBG("r_mac_high_light_dth_log2 %x",
    reg_cmd->mac_cmd.mac_cfg4.r_mac_high_light_dth_log2);
  ISP_DBG("r_mac_low_light_dth_log2 %x",
    reg_cmd->mac_cmd.mac_cfg4.r_mac_low_light_dth_log2);
  ISP_DBG("r_mac_low_light_strength %x",
    reg_cmd->mac_cmd.mac_cfg4.r_mac_low_light_strength);

  ISP_DBG("mac_high_light_th1 %x",
    reg_cmd->mac_cmd.mac_cfg5.mac_high_light_th1);
  ISP_DBG("r_mac_high_light_th1 %x",
    reg_cmd->mac_cmd.mac_cfg5.r_mac_high_light_th1);

  ISP_DBG("mac_smooth_dth_log2 %x",
    reg_cmd->mac_cmd.mac_cfg6.mac_smooth_dth_log2);
  ISP_DBG("mac_smooth_enable %x",
    reg_cmd->mac_cmd.mac_cfg6.mac_smooth_enable);
  ISP_DBG("mac_smooth_th1 %x",
    reg_cmd->mac_cmd.mac_cfg6.mac_smooth_th1);
  ISP_DBG("r_mac_smooth_dth_log2 %x",
    reg_cmd->mac_cmd.mac_cfg6.r_mac_smooth_dth_log2);
  ISP_DBG("r_mac_smooth_th1 %x",
    reg_cmd->mac_cmd.mac_cfg6.r_mac_smooth_th1);

  ISP_DBG("exp_ratio_recip %x",
    reg_cmd->mac_cmd.mac_cfg7.exp_ratio_recip);
  ISP_DBG("mac_linear_mode %x",
    reg_cmd->mac_cmd.mac_cfg7.mac_linear_mode);
  ISP_DBG("mac_smooth_tap0 %x",
    reg_cmd->mac_cmd.mac_cfg7.mac_smooth_tap0);
  ISP_DBG("msb_aligned %x",
    reg_cmd->mac_cmd.mac_cfg7.msb_aligned);
  ISP_DBG("r_mac_smooth_tap0 %x",
    reg_cmd->mac_cmd.mac_cfg7.r_mac_smooth_tap0);
}

static boolean hdr48_fetch_blkLvl_offset(mct_module_t *module,
  uint32_t identity, uint32_t *BlkLvl_offset)
{
  boolean                               ret = TRUE;
  mct_event_t                           mct_event;
  isp_private_event_t                   private_event;
  uint32_t                              blklvl;

  if (!module || !BlkLvl_offset) {
    ISP_ERR("failed: %p %p", module, BlkLvl_offset);
    return FALSE;
  }

  /*Get black level offset info */
  memset(&private_event, 0, sizeof(isp_private_event_t));
  private_event.type = ISP_PRIVATE_FETCH_BLKLVL_OFFSET;
  private_event.data = (void *)&blklvl;
  private_event.data_size = sizeof(uint32_t);

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.identity = identity;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.direction = MCT_EVENT_UPSTREAM;
  mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  mct_event.u.module_event.module_event_data = (void *)&private_event;
  ret = isp_sub_mod_util_send_event(module, &mct_event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_mod_util_send_event");
    return ret;
  }
  *BlkLvl_offset = blklvl;
  ISP_DBG("Black Level offset %d", *BlkLvl_offset);
  return ret;
}

static void hdr48_find_interpolate_rgn(isp_sub_module_t *isp_sub_module,
  hdr48_t *hdr,
  chromatix_videoHDR_type *chromatix_HDR_core,
  chromatix_videoHDR_float_core *interp_data)
{
  float start = 0.0f, end = 0.0f, ratio = 0.0f, gain = 0.0f;
  uint32_t i = 0;
  hdr->multfactor = 1.0f;

  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG) {
    //LSB alligned data
      hdr->multfactor = 1.0f /(1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH));
  }

  for (i = 0; i < MAX_SETS_FOR_TONE_NOISE_ADJ - 1; i++) {

    switch (chromatix_HDR_core->control_hdr) {
      case CONTROL_AEC_EXP_SENSITIVITY_RATIO:
        start = chromatix_HDR_core->videoHDR_core_data[i].
            aec_sensitivity_ratio.start;
        end = chromatix_HDR_core->videoHDR_core_data[i].
            aec_sensitivity_ratio.end;
        gain = hdr->hdr_sensitivity_ratio;
        break;
        case CONTROL_EXP_TIME_RATIO:
        start = chromatix_HDR_core->videoHDR_core_data[i].
            exp_time_trigger.start;
        end = chromatix_HDR_core->videoHDR_core_data[i].
            exp_time_trigger.end;
        gain = hdr->hdr_exp_time_ratio;
        break;
      default:
        ISP_ERR("Invalid control type");
        break;
      }

    if (gain < start) {
      break;
    } else if (gain < end) {
      ratio = (end - gain)/(end - start);
      break;
    }
  }

  ISP_DBG("control type %d start %f aec gain %f, end %f ratio",
    start, gain, end, ratio);

  if (F_EQUAL(ratio, 0.0) || ((i + 1) == MAX_SETS_FOR_TONE_NOISE_ADJ)) {
    interp_data->recon_min_factor =
      chromatix_HDR_core->videoHDR_core_data[i].recon_min_factor;
    interp_data->recon_flat_region_th =
      chromatix_HDR_core->videoHDR_core_data[i].recon_flat_region_th;
    interp_data->recon_h_edge_th1 =
      chromatix_HDR_core->videoHDR_core_data[i].recon_h_edge_th1;
    interp_data->recon_motion_th1 =
      chromatix_HDR_core->videoHDR_core_data[i].recon_motion_th1;
    interp_data->recon_dark_th1 =
      chromatix_HDR_core->videoHDR_core_data[i].recon_dark_th1;
    interp_data->hdr_zrec_prefilt_tap0 =
      chromatix_HDR_core->videoHDR_core_data[i].hdr_zrec_prefilt_tap0;
    interp_data->hdr_zrec_g_grad_th1 =
      chromatix_HDR_core->videoHDR_core_data[i].hdr_zrec_g_grad_th1;
    interp_data->hdr_zrec_rb_grad_th1 =
      chromatix_HDR_core->videoHDR_core_data[i].hdr_zrec_rb_grad_th1;
    interp_data->mac_motion0_th1 =
      chromatix_HDR_core->videoHDR_core_data[i].mac_motion0_th1;
    interp_data->mac_motion0_th2 =
      chromatix_HDR_core->videoHDR_core_data[i].mac_motion0_th2;
    interp_data->mac_motion_strength =
      chromatix_HDR_core->videoHDR_core_data[i].mac_motion_strength;
    interp_data->mac_low_light_th1 =
      chromatix_HDR_core->videoHDR_core_data[i].mac_low_light_th1;
    interp_data->mac_low_light_strength =
      chromatix_HDR_core->videoHDR_core_data[i].mac_low_light_strength;
    interp_data->mac_high_light_th1 =
      chromatix_HDR_core->videoHDR_core_data[i].mac_high_light_th1;
    interp_data->mac_high_light_dth_log2 =
      chromatix_HDR_core->videoHDR_core_data[i].mac_high_light_dth_log2;
  }else {
    interp_data->recon_min_factor = (LINEAR_INTERPOLATION(
      chromatix_HDR_core->videoHDR_core_data[i].recon_min_factor,
      chromatix_HDR_core->videoHDR_core_data[i + 1].recon_min_factor,
      ratio));

    interp_data->recon_flat_region_th = (LINEAR_INTERPOLATION(
      chromatix_HDR_core->videoHDR_core_data[i].recon_flat_region_th,
      chromatix_HDR_core->videoHDR_core_data[i + 1].recon_flat_region_th,
      ratio));
    interp_data->recon_h_edge_th1 = (LINEAR_INTERPOLATION(
      chromatix_HDR_core->videoHDR_core_data[i].recon_h_edge_th1,
      chromatix_HDR_core->videoHDR_core_data[i + 1].recon_h_edge_th1,
      ratio));
    interp_data->recon_motion_th1 = (LINEAR_INTERPOLATION(
      chromatix_HDR_core->videoHDR_core_data[i].recon_motion_th1,
      chromatix_HDR_core->videoHDR_core_data[i + 1].recon_motion_th1,
      ratio));
    interp_data->recon_dark_th1 = (LINEAR_INTERPOLATION(
      chromatix_HDR_core->videoHDR_core_data[i].recon_dark_th1,
      chromatix_HDR_core->videoHDR_core_data[i + 1].recon_dark_th1,
      ratio));
    interp_data->hdr_zrec_prefilt_tap0 = (LINEAR_INTERPOLATION(
      chromatix_HDR_core->videoHDR_core_data[i].hdr_zrec_prefilt_tap0,
      chromatix_HDR_core->videoHDR_core_data[i + 1].hdr_zrec_prefilt_tap0,
      ratio));
    interp_data->hdr_zrec_g_grad_th1 = (LINEAR_INTERPOLATION(
      chromatix_HDR_core->videoHDR_core_data[i].hdr_zrec_g_grad_th1,
      chromatix_HDR_core->videoHDR_core_data[i + 1].hdr_zrec_g_grad_th1,
      ratio));
    interp_data->hdr_zrec_rb_grad_th1 = (LINEAR_INTERPOLATION(
      chromatix_HDR_core->videoHDR_core_data[i].hdr_zrec_rb_grad_th1,
      chromatix_HDR_core->videoHDR_core_data[i + 1].hdr_zrec_rb_grad_th1,
      ratio));
    interp_data->mac_motion0_th1 = (LINEAR_INTERPOLATION(
      chromatix_HDR_core->videoHDR_core_data[i].mac_motion0_th1,
      chromatix_HDR_core->videoHDR_core_data[i + 1].mac_motion0_th1,
      ratio));
    interp_data->mac_motion0_th2 = (LINEAR_INTERPOLATION(
      chromatix_HDR_core->videoHDR_core_data[i].mac_motion0_th2,
      chromatix_HDR_core->videoHDR_core_data[i + 1].mac_motion0_th2,
      ratio));
    interp_data->mac_motion_strength = (LINEAR_INTERPOLATION(
      chromatix_HDR_core->videoHDR_core_data[i].mac_motion_strength,
      chromatix_HDR_core->videoHDR_core_data[i + 1].mac_motion_strength,
      ratio));
    interp_data->mac_low_light_th1 = (LINEAR_INTERPOLATION(
      chromatix_HDR_core->videoHDR_core_data[i].mac_low_light_th1,
      chromatix_HDR_core->videoHDR_core_data[i + 1].mac_low_light_th1,
      ratio));
    interp_data->mac_low_light_strength = (LINEAR_INTERPOLATION(
      chromatix_HDR_core->videoHDR_core_data[i].mac_low_light_strength,
      chromatix_HDR_core->videoHDR_core_data[i + 1].mac_low_light_strength,
      ratio));
    interp_data->mac_high_light_th1 = (LINEAR_INTERPOLATION(
      chromatix_HDR_core->videoHDR_core_data[i].mac_high_light_th1,
      chromatix_HDR_core->videoHDR_core_data[i + 1].mac_high_light_th1,
      ratio));
    interp_data->mac_high_light_dth_log2 = (LINEAR_INTERPOLATION(
      chromatix_HDR_core->videoHDR_core_data[i].mac_high_light_dth_log2,
      chromatix_HDR_core->videoHDR_core_data[i + 1].mac_high_light_dth_log2,
      ratio));
  }

     if ((hdr->ext_func_table && hdr->ext_func_table->apply_2d_interpolation)
      && (hdr->apply_hdr_effects)) {
      hdr->ext_func_table->apply_2d_interpolation(hdr);
    }
}

static void hdr48_fill_cfg_cmd(
  isp_sub_module_t *isp_sub_module,
  hdr48_t *hdr,
  chromatix_videoHDR_reserve *chromatix_HDR_reserve
  )
{

  hdr->reg_cmd.cfg_cmd.cfg0.recon_first_field =
           CLAMP_LIMIT(chromatix_HDR_reserve->recon_first_field, 0, 1);
  hdr->reg_cmd.cfg_cmd.cfg0.exp_ratio =
    CLAMP_LIMIT(FLOAT_TO_Q(10, hdr->hdr_sensitivity_ratio), 1024, 16384);

  hdr->reg_cmd.cfg_cmd.cfg1.rg_wb_gain_ratio =
           CLAMP_LIMIT(FLOAT_TO_Q(12,
           hdr->awb_gain.r_gain/hdr->awb_gain.g_gain), 1024, 16383);;

  hdr->reg_cmd.cfg_cmd.cfg2.bg_wb_gain_ratio =
           CLAMP_LIMIT(FLOAT_TO_Q(12,
           hdr->awb_gain.b_gain/hdr->awb_gain.g_gain), 1024, 16383);;

  hdr->reg_cmd.cfg_cmd.cfg3.gr_wb_gain_ratio =
         CLAMP_LIMIT(FLOAT_TO_Q(12,
       hdr->awb_gain.g_gain/hdr->awb_gain.r_gain), 1024, 16383);;

  hdr->reg_cmd.cfg_cmd.cfg4.gb_wb_gain_ratio =
       CLAMP_LIMIT(FLOAT_TO_Q(12,
       hdr->awb_gain.g_gain/hdr->awb_gain.b_gain), 1024, 16383);;

  hdr->reg_cmd.cfg_cmd2.cfg5.blk_out = CLAMP_LIMIT(Round(((float)hdr->BLSS_out)*hdr->multfactor), 0, 0xFFF);

  /* format is 8 bit data should be in 10 bit format*/
  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG)
    hdr->reg_cmd.cfg_cmd2.cfg5.blk_in = CLAMP_LIMIT(
      (hdr->BLSS_offset >> 2), 0, 255);
  else
    hdr->reg_cmd.cfg_cmd2.cfg5.blk_in = CLAMP_LIMIT(
      (hdr->BLSS_offset << 2), 0, 16683);
}

static void hdr48_fill_recon_cmd(
  isp_sub_module_t *isp_sub_module,
  hdr48_t *hdr,
  chromatix_videoHDR_reserve *chromatix_HDR_reserve,
  chromatix_videoHDR_float_core *interp_data)
{
  hdr->reg_cmd.recon_cmd.recon_cfg0.recon_motion_th1 =
    CLAMP_LIMIT(Round(interp_data->recon_motion_th1), 0, 1023);
  hdr->reg_cmd.recon_cmd.recon_cfg0.recon_motion_dth_log2 =
    CLAMP_LIMIT(chromatix_HDR_reserve->recon_motion_dth_log2,4, 8);
  hdr->reg_cmd.recon_cmd.recon_cfg0.recon_h_edge_th1 =
    CLAMP_LIMIT(Round(interp_data->recon_h_edge_th1), 0, 1023);
  hdr->reg_cmd.recon_cmd.recon_cfg0.recon_h_edge_dth_log2 =
    CLAMP_LIMIT(chromatix_HDR_reserve->recon_h_edge_dth_log2, 4,8);

  hdr->reg_cmd.recon_cmd.recon_cfg1.recon_dark_dth_log2 =
    CLAMP_LIMIT(chromatix_HDR_reserve->recon_dark_dth_log2,0, 4);
  hdr->reg_cmd.recon_cmd.recon_cfg1.recon_dark_th1 =
    CLAMP_LIMIT(Round(interp_data->recon_dark_th1), 0, 1023);
  hdr->reg_cmd.recon_cmd.recon_cfg1.recon_edge_lpf_tap0 =
    CLAMP_LIMIT(chromatix_HDR_reserve->recon_edge_lpf_tap0, 0, 5);
  hdr->reg_cmd.recon_cmd.recon_cfg1.recon_flat_region_th =
    CLAMP_LIMIT(Round(interp_data->recon_flat_region_th), 0, 1023);

  hdr->reg_cmd.recon_cmd.recon_cfg3.recon_linear_mode =
    CLAMP_LIMIT(chromatix_HDR_reserve->recon_linear_mode, 0, 1);
  hdr->reg_cmd.recon_cmd.recon_cfg3.recon_min_factor =
    CLAMP_LIMIT(Round(interp_data->recon_min_factor), 0, 31);

  if (hdr->is_3d_mode_on) {
    hdr->reg_cmd.recon_cmd.recon_cfg2.r_recon_motion_th1 =
        CLAMP_LIMIT(Round(interp_data->recon_motion_th1), 0, 1023);
    hdr->reg_cmd.recon_cmd.recon_cfg2.r_recon_motion_dth_log2 =
        CLAMP_LIMIT(chromatix_HDR_reserve->recon_motion_dth_log2, 4, 8);
    hdr->reg_cmd.recon_cmd.recon_cfg2.r_recon_h_edge_th1 =
        CLAMP_LIMIT(Round(interp_data->recon_h_edge_th1), 0, 1023);
    hdr->reg_cmd.recon_cmd.recon_cfg2.r_recon_h_edge_dth_log2 =
        CLAMP_LIMIT(chromatix_HDR_reserve->recon_h_edge_dth_log2, 4, 8);
    hdr->reg_cmd.recon_cmd.recon_cfg3.r_recon_dark_dth_log2 =
        CLAMP_LIMIT(chromatix_HDR_reserve->recon_dark_dth_log2, 0, 4);
    hdr->reg_cmd.recon_cmd.recon_cfg3.r_recon_dark_th1 =
        CLAMP_LIMIT(Round(interp_data->recon_dark_th1), 0, 1023);
    hdr->reg_cmd.recon_cmd.recon_cfg3.r_recon_min_factor =
        CLAMP_LIMIT(Round(interp_data->recon_min_factor), 0, 16);
    hdr->reg_cmd.recon_cmd.recon_cfg4.r_recon_flat_region_th =
        CLAMP_LIMIT(Round(interp_data->recon_flat_region_th), 0, 1023);
  }

  if ((isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG) &&
      (chromatix_HDR_reserve->hdr_zrec_sel)) {
    hdr->reg_cmd.recon_cmd2.recon_cfg5.zrec_enable = TRUE;

    hdr->reg_cmd.recon_cmd2.recon_cfg5.zrec_first_rb_exp =
      CLAMP_LIMIT(chromatix_HDR_reserve->hdr_zrec_first_rb_exp, 0, 1);
    hdr->reg_cmd.recon_cmd2.recon_cfg5.zrec_pattern =
      CLAMP_LIMIT(chromatix_HDR_reserve->hdr_zrec_pattern, 0, 3);
    hdr->reg_cmd.recon_cmd2.recon_cfg5.zrec_prefilt_tap0 =
      CLAMP_LIMIT(Round(interp_data->hdr_zrec_prefilt_tap0), 0, 64);

    hdr->reg_cmd.recon_cmd2.recon_cfg6.zrec_g_dth_log2 =
      CLAMP_LIMIT(chromatix_HDR_reserve->hdr_zrec_g_grad_dth_log2, 0, 12);
    hdr->reg_cmd.recon_cmd2.recon_cfg6.zrec_g_grad_th1 =
      Round(interp_data->hdr_zrec_g_grad_th1);
    hdr->reg_cmd.recon_cmd2.recon_cfg6.zrec_rb_dth_log2 =
      CLAMP_LIMIT(chromatix_HDR_reserve->hdr_zrec_rb_grad_dth_log2, 4, 8);
    hdr->reg_cmd.recon_cmd2.recon_cfg6.zrec_rb_grad_th1 =
      CLAMP_LIMIT(Round(interp_data->hdr_zrec_rb_grad_th1), 0, 4095);
  }
}

static void hdr48_fill_mac_cmd(
  hdr48_t *hdr,
  chromatix_videoHDR_reserve *chromatix_HDR_reserve,
  chromatix_videoHDR_float_core *interp_data)
{
  hdr->reg_cmd.mac_cmd.mac_cfg0.mac_motion_0_th1 =
    CLAMP_LIMIT(Round(interp_data->mac_motion0_th1), 0, 1023);
  hdr->reg_cmd.mac_cmd.mac_cfg0.mac_motion_0_th2 =
    CLAMP_LIMIT(Round(interp_data->mac_motion0_th2), 0, 255);

  hdr->reg_cmd.mac_cmd.mac_cfg1.mac_dilation =
    CLAMP_LIMIT(chromatix_HDR_reserve->mac_motion_dilation, 0, 5);

  hdr->reg_cmd.mac_cmd.mac_cfg1.mac_sqrt_analog_gain = CLAMP_LIMIT(
    FLOAT_TO_Q(4, sqrt(hdr->real_gain)), 16, 90);

  hdr->reg_cmd.mac_cmd.mac_cfg2.mac_motion_0_dt0 =
    CLAMP_LIMIT(chromatix_HDR_reserve->mac_motion0_dt0, 1, 63);
  hdr->reg_cmd.mac_cmd.mac_cfg2.mac_motion_strength =
    CLAMP_LIMIT(Round(interp_data->mac_motion_strength), 0, 16);

  hdr->reg_cmd.mac_cmd.mac_cfg3.mac_low_light_th1 =
    CLAMP_LIMIT( Round(interp_data->mac_low_light_th1),0, 16383);
  hdr->reg_cmd.mac_cmd.mac_cfg4.mac_high_light_dth_log2 =
    CLAMP_LIMIT(Round(interp_data->mac_high_light_dth_log2),2, 14);

  hdr->reg_cmd.mac_cmd.mac_cfg4.mac_low_light_dth_log2 =
    CLAMP_LIMIT(chromatix_HDR_reserve->mac_low_light_dth_log2,2, 14);
  hdr->reg_cmd.mac_cmd.mac_cfg4.mac_low_light_strength =
    CLAMP_LIMIT(Round(interp_data->mac_low_light_strength), 0, 16);

  hdr->reg_cmd.mac_cmd.mac_cfg5.mac_high_light_th1 =
    CLAMP_LIMIT(Round(interp_data->mac_high_light_th1), 0, 16383);

  hdr->reg_cmd.mac_cmd.mac_cfg6.mac_smooth_dth_log2 =
    CLAMP_LIMIT(chromatix_HDR_reserve->mac_smooth_dth_log2, 2, 8);
  hdr->reg_cmd.mac_cmd.mac_cfg6.mac_smooth_enable =
    CLAMP_LIMIT(chromatix_HDR_reserve->mac_smooth_enable, 0, 1);
  hdr->reg_cmd.mac_cmd.mac_cfg6.mac_smooth_th1 =
    CLAMP_LIMIT(chromatix_HDR_reserve->mac_smooth_th1, 0, 256);

  hdr->reg_cmd.mac_cmd.mac_cfg7.exp_ratio_recip = CLAMP_LIMIT(
       Round((float)256.0/hdr->hdr_sensitivity_ratio),
          16, 256);
  hdr->reg_cmd.mac_cmd.mac_cfg7.mac_linear_mode =
    CLAMP_LIMIT(chromatix_HDR_reserve->mac_linear_mode, 0, 1);
  hdr->reg_cmd.mac_cmd.mac_cfg7.msb_aligned =
      chromatix_HDR_reserve->hdr_MSB_align;
  hdr->reg_cmd.mac_cmd.mac_cfg7.mac_smooth_tap0 =
    CLAMP_LIMIT(chromatix_HDR_reserve->mac_smooth_tap0, 0, 5);

 if (hdr->is_3d_mode_on) {
    hdr->reg_cmd.mac_cmd.mac_cfg0.r_mac_motion_0_th1 =
        CLAMP_LIMIT(Round(interp_data->mac_motion0_th1), 0, 1023);
    hdr->reg_cmd.mac_cmd.mac_cfg1.r_mac_sqrt_analog_gain =
      CLAMP_LIMIT(FLOAT_TO_Q(7, sqrt(hdr->real_gain)), 16, 63);
    hdr->reg_cmd.mac_cmd.mac_cfg1.r_mac_motion_0_th2 =
        CLAMP_LIMIT(Round(interp_data->mac_motion0_th2), 0, 255);
    hdr->reg_cmd.mac_cmd.mac_cfg2.r_mac_motion_strength =
        CLAMP_LIMIT(Round(interp_data->mac_motion_strength), 0, 16);
    hdr->reg_cmd.mac_cmd.mac_cfg3.r_mac_low_light_th1 =
        CLAMP_LIMIT(Round(interp_data->mac_low_light_th1), 0, 16383);
    hdr->reg_cmd.mac_cmd.mac_cfg2.r_mac_motion_0_dt0 =
        CLAMP_LIMIT(chromatix_HDR_reserve->mac_motion0_dt0, 1, 63);
    hdr->reg_cmd.mac_cmd.mac_cfg4.r_mac_high_light_dth_log2 =
        CLAMP_LIMIT(Round(interp_data->mac_high_light_dth_log2), 2, 14);
    hdr->reg_cmd.mac_cmd.mac_cfg4.r_mac_low_light_dth_log2 =
        CLAMP_LIMIT(chromatix_HDR_reserve->mac_low_light_dth_log2, 2, 14);
    hdr->reg_cmd.mac_cmd.mac_cfg4.r_mac_low_light_strength =
        CLAMP_LIMIT(Round(interp_data->mac_low_light_strength), 0, 16);
    hdr->reg_cmd.mac_cmd.mac_cfg5.r_mac_high_light_th1 =
        CLAMP_LIMIT(Round(interp_data->mac_high_light_th1), 0, 16383);
    hdr->reg_cmd.mac_cmd.mac_cfg6.r_mac_smooth_dth_log2 =
        CLAMP_LIMIT(chromatix_HDR_reserve->mac_smooth_dth_log2, 2, 8);
    hdr->reg_cmd.mac_cmd.mac_cfg6.r_mac_smooth_th1 =
        CLAMP_LIMIT(chromatix_HDR_reserve->mac_smooth_th1, 0, 256);
    hdr->reg_cmd.mac_cmd.mac_cfg7.r_mac_smooth_tap0 =
        CLAMP_LIMIT(chromatix_HDR_reserve->mac_smooth_tap0, 0, 5);
 }
}


static boolean hdr48_prepare_hw_config(
  mct_module_t *module,
  isp_sub_module_t *isp_sub_module,
  hdr48_t *hdr)
{
  boolean                     ret = TRUE;
  chromatix_parms_type       *chromatix_ptr = NULL;
  chromatix_videoHDR_type    *chromatix_VHDR = NULL;
  chromatix_videoHDR_reserve *chromatix_HDR_reserve = NULL;
  //chromatix_videoHDR_core     interp_data;

  if (!module || !hdr || !isp_sub_module) {
    ISP_ERR("failed: %p %p", hdr, isp_sub_module);
    return FALSE;
  }

  chromatix_ptr = (chromatix_parms_type *)
    isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }
  chromatix_VHDR =
    &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;

  chromatix_HDR_reserve = &chromatix_VHDR->videoHDR_reserve_data;

  hdr48_find_interpolate_rgn(isp_sub_module, hdr, chromatix_VHDR,
    &hdr->interp_data);
  hdr48_fill_cfg_cmd(isp_sub_module, hdr, chromatix_HDR_reserve);
  hdr48_fill_recon_cmd(isp_sub_module, hdr,  chromatix_HDR_reserve,
      &hdr->interp_data);
  hdr48_fill_mac_cmd(hdr, chromatix_HDR_reserve, &hdr->interp_data);

  return ret;
}


/** hdr48_store_hw_update
 *
 *  @isp_sub_module: isp sub module handle
 *  @lineariztion: hdr handle
 *
 *  Create hw update list and store in isp sub module
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean hdr48_store_hw_update(isp_sub_module_t *isp_sub_module,
  hdr48_t *hdr)
{
  boolean                      ret = TRUE;
  int                          i, rc = 0;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  hdr_reg_cmd_t               *reg_cmd = NULL;

  if (!hdr || !isp_sub_module) {
    ISP_ERR("failed: %p %p", hdr, isp_sub_module);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  cfg_cmd = &hw_update->cfg_cmd;
  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
  CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd) * 5);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_MALLOC;
  }
  memset(reg_cfg_cmd, 0, (sizeof(struct msm_vfe_reg_cfg_cmd)) * 5);

  reg_cmd = (hdr_reg_cmd_t *)CAM_MALLOC(sizeof(hdr_reg_cmd_t));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cmd);
    goto ERROR_REG_CMD_MALLOC;
  }
  memset(reg_cmd, 0, sizeof(hdr_reg_cmd_t));

  *reg_cmd = hdr->reg_cmd;

  cfg_cmd->cfg_data = (void *)reg_cmd;
  cfg_cmd->cmd_len = sizeof(*reg_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 5;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_HDR_CFG;
  reg_cfg_cmd[0].u.rw_info.len = ISP_HDR_CFG_LEN * sizeof(uint32_t);

  reg_cfg_cmd[1].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[0].u.rw_info.cmd_data_offset + sizeof(hdr_cfg_cmd_t);
  reg_cfg_cmd[1].cmd_type = VFE_WRITE;
  reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_HDR_CFG2;
  reg_cfg_cmd[1].u.rw_info.len = ISP_HDR_CFG2_LEN * sizeof(uint32_t);

  reg_cfg_cmd[2].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[1].u.rw_info.cmd_data_offset + sizeof(hdr_cfg_2_cmd_t);
  reg_cfg_cmd[2].cmd_type = VFE_WRITE;
  reg_cfg_cmd[2].u.rw_info.reg_offset = ISP_HDR_RECON_OFF;
  reg_cfg_cmd[2].u.rw_info.len = ISP_HDR_RECON_LEN * sizeof(uint32_t);

  reg_cfg_cmd[3].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[2].u.rw_info.cmd_data_offset + sizeof(hdr_recon_cmd_t);
  reg_cfg_cmd[3].cmd_type = VFE_WRITE;
  reg_cfg_cmd[3].u.rw_info.reg_offset = ISP_HDR_RECON2_OFF;
  reg_cfg_cmd[3].u.rw_info.len = ISP_HDR_RECON2_LEN * sizeof(uint32_t);

  reg_cfg_cmd[4].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[3].u.rw_info.cmd_data_offset + sizeof(hdr_recon2_cmd_t);
  reg_cfg_cmd[4].cmd_type = VFE_WRITE;
  reg_cfg_cmd[4].u.rw_info.reg_offset = ISP_HDR_MAC_OFF;
  reg_cfg_cmd[4].u.rw_info.len = ISP_HDR_MAC_LEN * sizeof(uint32_t);

  hdr48_print_config(&hdr->reg_cmd);

  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;
  }

  hdr->apply_hdr_effects = 0;
  return TRUE;

ERROR_APPEND:
  free(reg_cmd);
ERROR_REG_CMD_MALLOC:
  free(reg_cfg_cmd);
ERROR_REG_CFG_MALLOC:
  free(hw_update);

  return FALSE;
} /* hdr48_store_hw_update */

/** hdr48_trigger_update
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  trigger update based on 3A event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean hdr48_trigger_update(
  mct_module_t *module,
  isp_sub_module_t *isp_sub_module,
  void *data,
  uint32_t identity)
{
  boolean                        ret = TRUE;
  hdr48_t                     *hdr = NULL;
  isp_private_event_t           *private_event = NULL;
  isp_sub_module_output_t       *sub_module_output = NULL;
  chromatix_parms_type *chromatix_ptr = NULL;
  uint32_t BlkLvl_offset = 0;

  if (!module || !isp_sub_module || !data) {
    ISP_ERR("failed: %p %p %p",module, isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  sub_module_output = (isp_sub_module_output_t *)private_event->data;
  if (!sub_module_output) {
    ISP_ERR("failed: output %p", sub_module_output);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  chromatix_ptr =
  (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;

  hdr = (hdr48_t *)isp_sub_module->private_data;
  if (!hdr) {
    ISP_ERR("failed: hdr %p", hdr);
    goto ERROR;
  }

  /* TODO: Need to propagate black level lock to meta data 'every frame' */

  if ((isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG) &&
      (chromatix_ptr->chromatix_post_processing.
        chromatix_video_HDR.enable > 0))
    isp_sub_module->submod_enable = TRUE;
  else
    isp_sub_module->submod_enable = FALSE;

  if (!isp_sub_module->submod_enable ||
      !isp_sub_module->submod_trigger_enable) {
    ISP_DBG("no trigger update for hdr, enabled %d, trig_enable %d\n",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    goto FILL_METADATA;
  }
  hdr48_fetch_blkLvl_offset(module, identity,
    &BlkLvl_offset);
  hdr->BLSS_offset = BlkLvl_offset;
  hdr->BLSS_out = BlkLvl_offset;

  if ((isp_sub_module->trigger_update_pending == TRUE) ||
    (hdr->apply_hdr_effects))  {
    isp_sub_module->trigger_update_pending = FALSE;

    ret = hdr48_prepare_hw_config(module, isp_sub_module, hdr);
    if (ret == FALSE) {
      ISP_ERR("failed: hdr48_prepare_hw_config");
      goto ERROR;
    }

    ret = hdr48_store_hw_update(isp_sub_module, hdr);
    if (ret == FALSE) {
      ISP_ERR("failed: hdr48_store_hw_update");
      goto ERROR;
    }
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
    sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR;
  }

FILL_METADATA:

  if (sub_module_output->meta_dump_params &&
    sub_module_output->metadata_dump_enable == 1) {

#ifdef DYNAMIC_DMI_ALLOC
 /* Add meta data fill */
#endif
  }

  if (sub_module_output->frame_meta) {
  /* Fill in meta info */
  }

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* hdr48_trigger_update */


/** hdr48_stats_aec_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_update_t
 *
 *  Handle AEC update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean hdr48_stats_aec_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  stats_update_t            *stats_update = NULL;
  hdr48_t                 *hdr = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  hdr = (hdr48_t *)isp_sub_module->private_data;
  if (!hdr) {
    ISP_ERR("failed: hdr %p", hdr);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  if (!F_EQUAL(hdr->real_gain,
        stats_update->aec_update.real_gain)) {
    hdr->real_gain = stats_update->aec_update.real_gain;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  if (!F_EQUAL(hdr->hdr_sensitivity_ratio,
        stats_update->aec_update.hdr_sensitivity_ratio )) {
    hdr->hdr_sensitivity_ratio = stats_update->aec_update.hdr_sensitivity_ratio;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  if (!F_EQUAL(hdr->hdr_exp_time_ratio,
        stats_update->aec_update.hdr_exp_time_ratio)) {
    hdr->hdr_exp_time_ratio = stats_update->aec_update.hdr_exp_time_ratio;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  if (hdr->ext_func_table && hdr->ext_func_table->get_2d_interpolation) {
    hdr->ext_func_table->get_2d_interpolation(isp_sub_module, hdr);
  }

  hdr->lux_gain = stats_update->aec_update.lux_idx;
  ISP_DBG("real gain %f exp ratio %f exp time %f",
    hdr->real_gain, hdr->hdr_sensitivity_ratio, hdr->hdr_exp_time_ratio);

  return TRUE;
} /* hdr48_stats_aec_update */

/** hdr48_stats_awb_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_update_t
 *
 *  Handle AWB update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean hdr48_stats_awb_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  stats_update_t            *stats_update = NULL;
  hdr48_t                 *hdr = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  hdr = (hdr48_t *)isp_sub_module->private_data;
  if (!hdr) {
    ISP_ERR("failed: hdr %p", hdr);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  if (!F_EQUAL(hdr->awb_gain.r_gain,
         stats_update->awb_update.gain.r_gain) ||
    !F_EQUAL(hdr->awb_gain.r_gain,
         stats_update->awb_update.gain.r_gain) ||
    !F_EQUAL(hdr->awb_gain.r_gain,
         stats_update->awb_update.gain.r_gain)) {
    hdr->awb_gain = stats_update->awb_update.gain;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  ISP_DBG("r_gain %f g_gain %f b_gain %f",
    hdr->awb_gain.r_gain, hdr->awb_gain.g_gain, hdr->awb_gain.b_gain);

  return TRUE;
} /* hdr48_stats_awb_update */

/** hdr48_set_chromatix_ptr:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Store chromatix ptr and make initial configuration
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean hdr48_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean              ret = TRUE;
  hdr48_t              *hdr = NULL;
  modulesChromatix_t   *chromatix_ptrs = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  chromatix_ptrs = (modulesChromatix_t *)data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    return FALSE;
  }

  hdr = (hdr48_t *)isp_sub_module->private_data;
  if (!hdr) {
    ISP_ERR("failed: hdr %p", hdr);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* hdr48_set_chromatix_ptr */

/** hdr48_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean hdr48_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  hdr48_t *hdr = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  hdr = (hdr48_t *)isp_sub_module->private_data;
  if (!hdr) {
    ISP_ERR("failed: hdr %p", hdr);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(hdr, 0, sizeof(*hdr));

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  FILL_FUNC_TABLE(hdr);
  return TRUE;
} /* hdr48_streamoff */

/** hdr48_streamon:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean hdr48_streamon(isp_sub_module_t *isp_sub_module, void *data)
{
  boolean                    ret = TRUE;
  hdr48_t                 *hdr = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  hdr = (hdr48_t *)isp_sub_module->private_data;
  if (!hdr) {
    ISP_ERR("failed: hdr %p", hdr);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->config_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* hdr48_streamoff */

/** hdr_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the hdr module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean hdr48_init(isp_sub_module_t *isp_sub_module)
{
  hdr48_t *hdr = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  hdr = (hdr48_t *)CAM_MALLOC(sizeof(hdr48_t));
  if (!hdr) {
    ISP_ERR("failed: hdr %p", hdr);
    return FALSE;
  }

  memset(hdr, 0, sizeof(*hdr));

  isp_sub_module->private_data = (void *)hdr;

  FILL_FUNC_TABLE(hdr);

  return TRUE;
}/* hdr48_init */

/** hdr48_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void hdr48_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* hdr48_destroy */

#if !OVERRIDE_FUNC
static ext_override_func hdr_override_func = {
  .get_2d_interpolation = NULL,
  .apply_2d_interpolation = NULL,
};

boolean hdr48_fill_func_table(hdr48_t *hdr)
{
  hdr->ext_func_table = &hdr_override_func;
  return TRUE;
} /* hdr48_fill_func_table */
#endif
