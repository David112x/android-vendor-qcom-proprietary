/*===========================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#ifndef __ZZHDR_REG_H__
#define __ZZHDR_REG_H__

typedef struct
{
  uint32_t frame_id; // current frame id
  // control, from VFE top level

  uint16_t zzhdr_linear_mode; // when 1 and exp_ratio==1,
                              //output = T1->wb->clip->inv_wb
  uint16_t hdr_exp_ratio; //15uQ10, 1024~16384 representing 1.0~16.0

  uint16_t hdr_exp_ratio_recip; //9uQ8, 16~256 representing 1/16~1.0,
                                //exp_ratio reciprocal

  // black level
  uint16_t hdr_black_in; //12u, black level on input
                         //(MSB-aligned)

  // WB parameters
  uint32_t hdr_rg_wb_gain_ratio;// 17uQ12, R gain vs G

  uint32_t hdr_bg_wb_gain_ratio;// 17uQ12, B gain vs G

  uint32_t hdr_gr_wb_gain_ratio; // 17uQ12, G gain vs R

  uint32_t hdr_gb_wb_gain_ratio; // 17uQ12, G gain vs B

  // HDR Reconstruction parameters
  uint16_t hdr_rec_h_edge_th1; // 10u, horz edge threshold_1
  uint16_t hdr_rec_h_edge_dth_log2; // 4u (4~8), log2(th2-th1)

  uint16_t hdr_rec_motion_th1; // 10u, motion threshold_1
  uint16_t hdr_rec_motion_dth_log2; // 4u (4~8), log2(th2-th1)

  uint16_t hdr_rec_dark_th1; // 10u, dark level threshold_1
  uint16_t hdr_rec_dark_dth_log2; // 3u (0~4), log2(th2 - th1),

  // HDR Motion Adaptive Combine parameters
  uint16_t hdr_mac_lowlight_strength; // 5uQ4 (0~16),
                                          //lowlight adaptation strength
  // was called motion_strength1
  uint16_t hdr_mac_lowlight_th1; // 14u, lowlight threshold_1

  uint16_t hdr_mac_lowlight_dth_log2; //4u (2~14),
                                          //log2 of lowlight th2 - th1
  uint16_t hdr_mac_hilight_th1; // 14u, highlight threshold_1

  uint16_t hdr_mac_hilight_dth_log2; //4u (2~14),
                                          //log2 of highlight th2 - th1
  // zigzag HDR recon additional parameters
  uint16_t hdr_zrec_pattern; // 0~3, Bayer starting R1:
                                          //0=R1G1G2B1, 1=R1G1G2B2,
                                          // 2=R1G2G1B1, 3=R1G2G1B2

  uint16_t hdr_zrec_first_rb_exp; // 0~1,first R/B exposure 0=T1, 1=T2

  uint16_t hdr_zrec_prefilt_tap0; // 7u,lowpass prefilter side-taps,
                                          //0~64, 0 to turn off

  uint16_t hdr_zrec_g_grad_th1; //12u, threshold_1 for G gradient
  uint16_t hdr_zrec_g_grad_dth_log2; //4u, log2 of th2 - th1 for
                                          //G gradient, 0~12

  uint16_t hdr_zrec_rb_grad_th1; //12u, threshold_1 for R/B gradient
  uint16_t hdr_zrec_rb_grad_dth_log2; //4u, log2 of th2 - th1
                                      //for R/B gradient, 0~12

} zzHDR_cfg_cmd;


/* High resolution LUT with floats*/
typedef struct {
  float r_lut_p;
  float gr_lut_p;
  float gb_lut_p;
  float b_lut_p;
} zzHDR_L_black_level_t;


typedef struct {
  float                          wb_rg_ratio;
  float                          wb_bg_ratio;
  float                          wb_gr_ratio;
  float                          wb_gb_ratio;
  float                          hdr_sensitivity_ratio;
  float                          hdr_exp_time_ratio;
  float                          analog_gain;
  float                          aec_ratio;
  zzHDR_cfg_cmd                  zzHDR_cfg;
  CHROMATIX_PARAMS_TYPE          *chromatix_ptr;
  chromatix_VFE_common_type      *chromatix_comm_ptr;
  zzHDR_L_black_level_t          lowlight;
  zzHDR_L_black_level_t          normal_light;
} zzHDR_mod_t;
#endif /* __ZZHDR_REG_H__ */
