/* ltm_reg.h
 * Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#ifndef __LTM_REG_H__
#define __LTM_REG_H__

#define ISP_LTM_CFG_OFF     0x000006D8
#define ISP_LTM_STATUS_OFF  0x000006DC
#define ISP_LTM_H_SCALE_OFF 0x000006E0
#define ISP_LTM_V_SCALE_OFF 0x000006F4
#define ISP_LTM_DCIP_OFF    0x00000708
#define ISP_LTM_RGB2Y_OFF   0x00000720
#define ISP_LTM_H_PAD_OFF   0x000006F0
#define ISP_LTM_V_PAD_OFF   0x00000704
#define ISP_LTM_LEN         0x10
#define ISP_LTM_IP_EN     0x4
#define ISP_LTM_IP_INV_CELL_W_BIT 14

#define IP_INIT_CELLNUM(reg) reg->ltm_cfg.ip_initCellNumX
#define DC_INIT_CELLNUM(reg) reg->ltm_cfg.dc_initCellNumX

#define GAMMA_HW_PACK_BIT   12
#define GAMMA_NUM_ENTRIES   256
#define GAMMA_LUT_TYPE      uint16_t

#define SCALE_PHASE_Q 14
#define LUT_BASE_BITS 14
#define LUT_DELTA_BITS 14
#define LTM_HW_LUT_SIZE   128
#define BASE_MASK  0x3FFF
#define DELTA_MASK 0x3FFF
#define IP_INV_Q          18
#define LUT_PIX_BITS      12

#define MAX_SCALE_FACTOR_VAL 69
#define MIN_BASE_VAL -8192
#define MAX_BASE_VAL  8191
#define MIN_DIFF_VAL -8192
#define MAX_DIFF_VAL  8191

#define LTM_VERSION 46
#define CHROMATIX_LUT_ENTRY 129
#define LUT_SUB_SAMPLE CHROMATIX_LUT_ENTRY/LTM_HW_LUT_SIZE

#define LTM_CGC_OVERRIDE TRUE
#define LTM_CGC_OVERRIDE_REGISTER 0x34
#define LTM_CGC_OVERRIDE_BIT 4

/* LTM_CFG = 0x06D8 */
typedef struct ISP_LTMCfg {
  uint32_t hMnEnable               :1;
  uint32_t vMnEnable               :1;
  uint32_t ipEnable                :1;
  uint32_t dcEnable                :1;
  uint32_t autoSwitchOverride      :1;
  uint32_t dc3dAvgPongSel          :1;
  uint32_t dc3dSumClear            :1;
  uint32_t ip3dAvgPongSel          :1;
  uint32_t lutBankSelect           :1;
  uint32_t debugOutSelect          :2;
  uint32_t /* reserved */          :5;
  uint32_t dc_initCellNumX         :4;
  uint32_t ip_initCellNumX         :4;
  uint32_t /* reserved */          :8;
} __attribute__((packed, aligned(4))) ISP_LTMCfg;

/* LTM_STATUS = 0x06DC */
/* this register is read only */
typedef struct ISP_LTMStatus {
  uint32_t dc3dAvgPong             :1;
  uint32_t /* reserved */          :31;
} __attribute__((packed, aligned(4))) ISP_LTMStatus;

/* LTM_SCALE_H_IMAGE_SIZE_CFG = 0x06E0 */
typedef struct ISP_LTMScaleHImageSizeCfg {
  uint32_t hIn                     :14;
  uint32_t /* reserved */          :2;
  uint32_t hOut                    :9;
  uint32_t /* reserved */          :7;
} __attribute__((packed, aligned(4))) ISP_LTMScaleHImageSizeCfg;

/* LTM_SCALE_H_PHASE_CFG = 0x06E4 */
typedef struct ISP_LTMScaleHPhaseCfg {
  uint32_t hPhaseMult              :21;
  uint32_t /* reserved */          :7;
  uint32_t hInterpReso             :2;
  uint32_t /* reserved */          :2;
} __attribute__((packed, aligned(4))) ISP_LTMScaleHPhaseCfg;

/* LTM_SCALE_H_STRIPE_CFG = 0x06E8 & 0x6EC */
typedef struct ISP_LTMScaleHStripeCfg {
  uint32_t hMnInit                 :14;
  uint32_t /* reserved */          :18;

  uint32_t hPhaseInit              :21;
  uint32_t /* reserved */          :11;
} __attribute__((packed, aligned(4))) ISP_LTMScaleHStripeCfg;

/* LTM_SCALE_V_IMAGE_SIZE_CFG = 0x06F4 */
typedef struct ISP_LTMScaleVImageSizeCfg {
  uint32_t vIn                     :14;
  uint32_t /* reserved */          :2;
  uint32_t vOut                    :8;
  uint32_t /* reserved */          :8;
} __attribute__((packed, aligned(4))) ISP_LTMScaleVImageSizeCfg;

/* LTM_SCALE_V_PHASE_CFG = 0x06F8 */
typedef struct ISP_LTMScaleVPhaseCfg {
  uint32_t vPhaseMult              :21;
  uint32_t /* reserved */          :7;
  uint32_t vInterpReso             :2;
  uint32_t /* reserved */          :2;
} __attribute__((packed, aligned(4))) ISP_LTMScaleVPhaseCfg;

/* LTM_SCALE_V_STRIPE_CFG = 0x06FC & 0x0700 */
typedef struct ISP_LTMScaleVStripeCfg {
  uint32_t vMnInit                 :14;
  uint32_t /* reserved */          :18;

  uint32_t vPhaseInit              :21;
  uint32_t /* reserved */          :11;
} __attribute__((packed, aligned(4))) ISP_LTMScaleVStripeCfg;

/* LTM_DC_CFG = 0x0708 */
typedef struct ISP_LTMDcCfg {
  uint32_t /* reserved */          :4;
  uint32_t initDX                  :5;
  uint32_t /* reserved */          :3;
  uint32_t binInitCnt              :4;
  uint32_t /* reserved */          :16;
} __attribute__((packed, aligned(4))) ISP_LTMDcCfg;

/* LTM_IP_INIT_CFG = 0x070C */
typedef struct ISP_LTMIpInitCfg {
  uint32_t initDX                  :11;
  uint32_t /* reserved */          :1;
  uint32_t initPX                  :18;
  uint32_t /* reserved */          :2;
} __attribute__((packed, aligned(4))) ISP_LTMIpInitCfg;

/* LTM_IP_INV_CELLWIDTH_CFG = 0x0710 */
typedef struct ISP_LTMIpInvCellwidthCfg {
  uint32_t invCellwidthL           :14;
  uint32_t /* reserved */          :2;
  uint32_t invCellwidthR           :14;
  uint32_t /* reserved */          :2;
} __attribute__((packed, aligned(4))) ISP_LTMIpInvCellwidthCfg;

/* LTM_IP_INV_CELLHEIGHT_CFG = 0x0714 */
typedef struct ISP_LTMIpInvCellheightCfg {
  uint32_t invCellheightL          :14;
  uint32_t /* reserved */          :2;
  uint32_t invCellheightR          :14;
  uint32_t /* reserved */          :2;
} __attribute__((packed, aligned(4))) ISP_LTMIpInvCellheightCfg;

/* LTM_IP_CELLWIDTH_CFG = 0x0718 */
typedef struct ISP_LTMIpCellwidthCfg {
  uint32_t cellwidthL              :11;
  uint32_t /* reserved */          :5;
  uint32_t cellwidthR              :11;
  uint32_t /* reserved */          :5;
} __attribute__((packed, aligned(4))) ISP_LTMIpCellwidthCfg;

/* LTM_IP_INV_CELLHEIGHT_CFG = 0x071C */
typedef struct ISP_LTMIpCellheightCfg {
  uint32_t cellheightL             :11;
  uint32_t /* reserved */          :5;
  uint32_t cellheightR             :11;
  uint32_t /* reserved */          :5;
} __attribute__((packed, aligned(4))) ISP_LTMIpCellheightCfg;

/* LTM_RGB2Y_CFG0 = 0x0720 */
typedef struct ISP_LTMRGB2YCfg0 {
  uint32_t c1                      :10;
  uint32_t c2                      :10;
  uint32_t c3                      :10;
  uint32_t /* reserved */          :2;
} __attribute__((packed, aligned(4))) ISP_LTMRGB2YCfg0;

/* LTM_RGB2Y_CFG1 = 0x0724 */
typedef struct ISP_LTMRGB2YCfg1 {
  uint32_t thr                     :10;
  uint32_t k                       :10;
  uint32_t c4                      :10;
  uint32_t /* reserved */          :2;
} __attribute__((packed, aligned(4))) ISP_LTMRGB2YCfg1;

/* ######################### PADDING REGISTERS ###################### */

/* LTM_SCALE_H_PAD_CFG = 0x06F0 */
typedef struct ISP_LTMScaleHPadCfg {
  uint32_t scaleYInWidth           :14;
  uint32_t /* reserved */          :2;
  uint32_t hSkipCnt                :14;
  uint32_t /* reserved */          :2;
} __attribute__((packed, aligned(4))) ISP_LTMScaleHPadCfg;

/* LTM_SCALE_V_PAD_CFG = 0x0704 */
typedef struct ISP_LTMScaleVPadCfg {
  uint32_t scaleYInHeight          :14;
  uint32_t /* reserved */          :2;
  uint32_t vSkipCnt                :14;
  uint32_t /* reserved */          :2;
} __attribute__((packed, aligned(4))) ISP_LTMScaleVPadCfg;

/* structures for ltm registers*/
/* bundle adjacent registers that configure the same things for less writes */
typedef struct {
  ISP_LTMScaleHImageSizeCfg h_image_size;
  ISP_LTMScaleHPhaseCfg     h_phase;
  ISP_LTMScaleHStripeCfg    h_stripe;
}__attribute__((packed, aligned(4))) isp_ltm_h_scale_cfg_t;

typedef struct {
  ISP_LTMScaleVImageSizeCfg v_image_size;
  ISP_LTMScaleVPhaseCfg     v_phase;
  ISP_LTMScaleVStripeCfg    v_stripe;
}__attribute__((packed, aligned(4))) isp_ltm_v_scale_cfg_t;

typedef struct {
  ISP_LTMDcCfg              dc_cfg;
  ISP_LTMIpInitCfg          ip_init;
  ISP_LTMIpInvCellwidthCfg  ip_inv_cellwidth;
  ISP_LTMIpInvCellheightCfg ip_inv_cellheight;
  ISP_LTMIpCellwidthCfg     ip_cellwidth;
  ISP_LTMIpCellheightCfg    ip_cellheight;
}__attribute__((packed, aligned(4))) isp_ltm_dcip_cfg_t;

typedef struct {
  ISP_LTMRGB2YCfg0          rgb2y_cfg0;
  ISP_LTMRGB2YCfg1          rgb2y_cfg1;
}__attribute__((packed, aligned(4))) isp_ltm_rgb2y_cfg_t;

/* structure to contain all registers */
typedef struct {
  ISP_LTMCfg                ltm_cfg;
  ISP_LTMStatus             ltm_status;
  isp_ltm_h_scale_cfg_t     ltm_h_scale_cfg;
  isp_ltm_v_scale_cfg_t     ltm_v_scale_cfg;
  isp_ltm_dcip_cfg_t        ltm_dcip_cfg;
  isp_ltm_rgb2y_cfg_t       ltm_rgb2y_cfg;
  ISP_LTMScaleHPadCfg       ltm_scale_h_pad_cfg;
  ISP_LTMScaleVPadCfg       ltm_scale_v_pad_cfg;
}__attribute__((packed, aligned(4))) isp_ltm_reg_t;

#endif //__LTM_REG_H__
