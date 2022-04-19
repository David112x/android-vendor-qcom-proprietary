/* hdr_reg.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __HDR_REG_H__
#define __HDR_REG_H__

/* HDR registers */
#define ISP_HDR_CFG        0x0000057C
#define ISP_HDR_CFG_LEN    5

#define ISP_HDR_CFG2       0x00000c84
#define ISP_HDR_CFG2_LEN   1

#define ISP_HDR_RECON_OFF  0x00000590
#define ISP_HDR_RECON_LEN  5

#define ISP_HDR_RECON2_OFF 0x00000C8C
#define ISP_HDR_RECON2_LEN 2

#define ISP_HDR_MAC_OFF    0x000005A4
#define ISP_HDR_MAC_LEN    8

#define HDR_CFG_0 0x0000057C
typedef struct ISP_HdrCfg0 {
  uint32_t recon_first_field   :1;
  uint32_t /* reserved bits */ :3;
  uint32_t exp_ratio           :15;
  uint32_t /* reserbed bits */ :13;
}__attribute__((packed, aligned(4))) ISP_HdrCfg0;

#define HDR_CFG_1 0x00000580
typedef struct ISP_HdrCfg1 {
  uint32_t rg_wb_gain_ratio   :17;
  uint32_t /*reserved bits */ :15;
}__attribute__((packed, aligned(4))) ISP_HdrCfg1;

#define HDR_CFG_2 0x00000584
typedef struct ISP_HdrCfg2 {
  uint32_t bg_wb_gain_ratio   :17;
  uint32_t /*reserved bits */ :15;
}__attribute__((packed, aligned(4))) ISP_HdrCfg2;

#define HDR_CFG_3 0x00000588
typedef struct ISP_HdrCfg3 {
  uint32_t gr_wb_gain_ratio   :17;
  uint32_t /*reserved bits */ :15;
}__attribute__((packed, aligned(4))) ISP_HdrCfg3;

#define HDR_CFG_4 0x0000058c
typedef struct ISP_HdrCfg4 {
  uint32_t gb_wb_gain_ratio   :17;
  uint32_t /*reserved bits */ :15;
}__attribute__((packed, aligned(4))) ISP_HdrCfg4;

#define HDR_CFG_5 0x00000c84
typedef struct ISP_HdrCfg5 {
  uint32_t blk_in             :8;
  uint32_t blk_out            :12;
  uint32_t /*reserved bits */ :12;
}__attribute__((packed, aligned(4))) ISP_HdrCfg5;

/* HDR reconstruct registers */

#define HDR_RECON_CFG_0 0x00000590
typedef struct ISP_HdrReconCfg0 {
  uint32_t recon_h_edge_th1      :10;
  uint32_t /* reserved bits */   :2;
  uint32_t recon_h_edge_dth_log2 :4;
  uint32_t recon_motion_th1      :10;
  uint32_t /* reserved bits */   :2;
  uint32_t recon_motion_dth_log2 :4;
}__attribute__((packed, aligned(4))) ISP_HdrReconCfg0;

#define HDR_RECON_CFG_1 0x00000594
typedef struct ISP_HdrReconCfg1 {
  uint32_t recon_dark_th1       :10;
  uint32_t /* reserved bits */  :2;
  uint32_t recon_dark_dth_log2  :3;
  uint32_t /* reserved bits */  :1;
  uint32_t recon_flat_region_th :10;
  uint32_t /* reserved bits */  :2;
  uint32_t recon_edge_lpf_tap0  :3;
  uint32_t /* reserved bits */  :1;
}__attribute__((packed, aligned(4))) ISP_HdrReconCfg1;

#define HDR_RECON_CFG_2 0x00000598
typedef struct ISP_HdrReconCfg2 {
  uint32_t r_recon_h_edge_th1      :10;
  uint32_t /* reserved bits */     :2;
  uint32_t r_recon_h_edge_dth_log2 :4;
  uint32_t r_recon_motion_th1      :10;
  uint32_t /* reserved bits */     :2;
  uint32_t r_recon_motion_dth_log2 :4;
}__attribute__((packed, aligned(4))) ISP_HdrReconCfg2;

#define HDR_RECON_CFG_3 0x0000059C
typedef struct ISP_HdrReconCfg3 {
  uint32_t r_recon_dark_th1      :10;
  uint32_t /* reserved bits */   :2;
  uint32_t r_recon_dark_dth_log2 :3;
  uint32_t /* reserved bits */   :1;
  uint32_t recon_min_factor      :5;
  uint32_t /* reserved bits */   :3;
  uint32_t r_recon_min_factor    :5;
  uint32_t /* reserved bits */   :1;
  uint32_t recon_linear_mode     :1;
  uint32_t /* reserved bits */   :1;
}__attribute__((packed, aligned(4))) ISP_HdrReconCfg3;

#define HDR_RECON_CFG_4 0x000005A0
typedef struct ISP_HdrReconCfg4 {
  uint32_t r_recon_flat_region_th :10;
  uint32_t /* reserved bits */    :22;
}__attribute__((packed, aligned(4))) ISP_HdrReconCfg4;

#define HDR_RECON_CFG_5 0x00000C8C
typedef struct ISP_HdrReconCfg5 {
  uint32_t zrec_enable         :1;
  uint32_t zrec_first_rb_exp   :1;
  uint32_t /* reserved bits */ :2;
  uint32_t zrec_pattern        :2;
  uint32_t /* reserved bits */ :2;
  uint32_t zrec_prefilt_tap0   :7;
  uint32_t /* reserved bits */ :17;
}__attribute__((packed, aligned(4))) ISP_HdrReconCfg5;

#define HDR_RECON_CFG_6 0x00000C90
typedef struct ISP_HdrReconCfg6 {
  uint32_t zrec_g_grad_th1  :12;
  uint32_t zrec_g_dth_log2  :4;
  uint32_t zrec_rb_grad_th1 :12;
  uint32_t zrec_rb_dth_log2 :4;
}__attribute__((packed, aligned(4))) ISP_HdrReconCfg6;

/* HDR mac registers */
#define HDR_MAC_CFG_0 0x000005A4
typedef struct ISP_HdrMacCfg0 {
  uint32_t mac_motion_0_th1    :10;
  uint32_t /* reserved bits */ :2;
  uint32_t r_mac_motion_0_th1  :10;
  uint32_t /* reserved bits */ :2;
  uint32_t mac_motion_0_th2    :8;
}__attribute__((packed, aligned(4))) ISP_HdrMacCfg0;

#define HDR_MAC_CFG_1 0x000005A8
typedef struct ISP_HdrMacCfg1 {
  uint32_t r_mac_motion_0_th2     :8;
  uint32_t mac_sqrt_analog_gain   :7;
  uint32_t /* reserved bits */    :1;
  uint32_t r_mac_sqrt_analog_gain :7;
  uint32_t /* reserved bits */    :1;
  uint32_t mac_dilation           :3;
  uint32_t /* reserved bits */    :5;
}__attribute__((packed, aligned(4))) ISP_HdrMacCfg1;

#define HDR_MAC_CFG_2 0x000005AC
typedef struct ISP_HdrMacCfg2 {
  uint32_t mac_motion_0_dt0      :6;
  uint32_t /* reserved bits */   :2;
  uint32_t r_mac_motion_0_dt0    :6;
  uint32_t /* reserved bits */   :2;
  uint32_t mac_motion_strength   :5;
  uint32_t /* reserved bits */   :3;
  uint32_t r_mac_motion_strength :5;
  uint32_t /* reserved bits */   :3;
}__attribute__((packed, aligned(4))) ISP_HdrMacCfg2;

#define HDR_MAC_CFG_3 0x000005B0
typedef struct ISP_HdrMacCfg3 {
  uint32_t mac_low_light_th1   :14;
  uint32_t /* reserved bits */ :2;
  uint32_t r_mac_low_light_th1 :14;
  uint32_t /* reserved bits */ :2;
}__attribute__((packed, aligned(4))) ISP_HdrMacCfg3;

#define HDR_MAC_CFG_4 0x000005B4
typedef struct ISP_HdrMacCfg4 {
  uint32_t mac_low_light_dth_log2    :4;
  uint32_t r_mac_low_light_dth_log2  :4;
  uint32_t mac_low_light_strength    :5;
  uint32_t /* reserved bits */       :3;
  uint32_t r_mac_low_light_strength  :5;
  uint32_t /* reserved bits */       :3;
  uint32_t mac_high_light_dth_log2   :4;
  uint32_t r_mac_high_light_dth_log2 :4;
}__attribute__((packed, aligned(4))) ISP_HdrMacCfg4;

#define HDR_MAC_CFG_5 0x000005B8
typedef struct ISP_HdrMacCfg5 {
  uint32_t mac_high_light_th1   :14;
  uint32_t /* reserved bits */  :2;
  uint32_t r_mac_high_light_th1 :14;
  uint32_t /* reserved bits */  :2;
}__attribute__((packed, aligned(4))) ISP_HdrMacCfg5;

#define HDR_MAC_CFG_6 0x000005BC
typedef struct ISP_HdrMacCfg6 {
  uint32_t mac_smooth_th1        :9;
  uint32_t /* reserved bits */   :3;
  uint32_t r_mac_smooth_th1      :9;
  uint32_t /* reserved bits */   :1;
  uint32_t mac_smooth_enable     :1;
  uint32_t /* reserved bits */   :1;
  uint32_t mac_smooth_dth_log2   :4;
  uint32_t r_mac_smooth_dth_log2 :4;
}__attribute__((packed, aligned(4))) ISP_HdrMacCfg6;

#define HDR_MAC_CFG_7 0x000005C0
typedef struct ISP_HdrMacCfg7 {
  uint32_t mac_smooth_tap0     :3;
  uint32_t /* reserved bits */ :1;
  uint32_t r_mac_smooth_tap0   :3;
  uint32_t /* reserved bits */ :1;
  uint32_t exp_ratio_recip     :9;
  uint32_t msb_aligned         :1;
  uint32_t mac_linear_mode     :1;
  uint32_t /* reserved bits */ :13;
}__attribute__((packed, aligned(4))) ISP_HdrMacCfg7;

#endif /* __HDR_REG_H__ */
