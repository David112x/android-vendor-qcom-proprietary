/* ltm_reg.h
 * Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.*
 */

#ifndef __LTM_REG_H__
#define __LTM_REG_H__

#define ISP_LTM_CFG_OFF     0x00000978
#define ISP_LTM_STATUS_OFF  0x0000097C
#define ISP_LTM_H_SCALE_OFF 0x00000980
#define ISP_LTM_V_SCALE_OFF 0x0000098C
#define ISP_LTM_DCIP_OFF    0x00000998
#define ISP_LTM_RGB2Y_OFF   0x000009B0
#define ISP_LTM_H_PAD_OFF   0x00000AB0
#define ISP_LTM_V_PAD_OFF   0x00000AB4
#define ISP_LTM_LEN         0x10
#define ISP_LTM_IP_EN     0x4
#define ISP_LTM_IP_INV_CELL_W_BIT 11

#define IP_INIT_CELLNUM(reg) reg->ltm_dcip_cfg.ip_init.initCellNumX
#define DC_INIT_CELLNUM(reg) reg->ltm_dcip_cfg.dc_cfg.initCellNumX

#define SCALE_PHASE_Q 13
#define LUT_BASE_BITS 12
#define LUT_DELTA_BITS 11
#define LTM_HW_LUT_SIZE   64
#define BASE_MASK  0xFFF
#define DELTA_MASK 0x7FF
#define IP_INV_Q          15
#define LUT_PIX_BITS      12

#define MAX_SCALE_FACTOR_VAL 32
#define MIN_BASE_VAL -2048
#define MAX_BASE_VAL  2047
#define MIN_DIFF_VAL -1024
#define MAX_DIFF_VAL  1023

#define LTM_VERSION 44
#define CHROMATIX_LUT_ENTRY 129
#define LUT_SUB_SAMPLE CHROMATIX_LUT_ENTRY/LTM_HW_LUT_SIZE

#define LTM_CGC_OVERRIDE FALSE
#define LTM_CGC_OVERRIDE_REGISTER 0x0
#define LTM_CGC_OVERRIDE_BIT 0

#define GAMMA_HW_PACK_BIT   12
#define GAMMA_NUM_ENTRIES   256
#define GAMMA_LUT_TYPE      uint16_t

/* LTM_CFG = 0x0978 */
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
  uint32_t /* reserved */          :21;
} __attribute__((packed, aligned(4))) ISP_LTMCfg;

/* LTM_STATUS = 0x097C */
/* this register is read only */
typedef struct ISP_LTMStatus {
  uint32_t dc3dAvgPong             :1;
  uint32_t /* reserved */          :31;
} __attribute__((packed, aligned(4))) ISP_LTMStatus;

/* LTM_SCALE_H_IMAGE_SIZE_CFG = 0x0980 */
typedef struct ISP_LTMScaleHImageSizeCfg {
  uint32_t hIn                     :13;
  uint32_t /* reserved */          :3;
  uint32_t hOut                    :9;
  uint32_t /* reserved */          :7;
} __attribute__((packed, aligned(4))) ISP_LTMScaleHImageSizeCfg;

/* LTM_SCALE_H_PHASE_CFG = 0x0984 */
typedef struct ISP_LTMScaleHPhaseCfg {
  uint32_t hPhaseMult              :19;
  uint32_t /* reserved */          :1;
  uint32_t hInterpReso             :2;
  uint32_t /* reserved */          :10;
} __attribute__((packed, aligned(4))) ISP_LTMScaleHPhaseCfg;

/* LTM_SCALE_H_STRIPE_CFG = 0x0988 */
typedef struct ISP_LTMScaleHStripeCfg {
  uint32_t hMnInit                 :13;
  uint32_t /* reserved */          :3;
  uint32_t hPhaseInit              :16;
} __attribute__((packed, aligned(4))) ISP_LTMScaleHStripeCfg;

/* LTM_SCALE_V_IMAGE_SIZE_CFG = 0x098C */
typedef struct ISP_LTMScaleVImageSizeCfg {
  uint32_t vIn                     :14;
  uint32_t /* reserved */          :2;
  uint32_t vOut                    :8;
  uint32_t /* reserved */          :8;
} __attribute__((packed, aligned(4))) ISP_LTMScaleVImageSizeCfg;

/* LTM_SCALE_V_PHASE_CFG = 0x0990 */
typedef struct ISP_LTMScaleVPhaseCfg {
  uint32_t vPhaseMult              :19;
  uint32_t /* reserved */          :1;
  uint32_t vInterpReso             :2;
  uint32_t /* reserved */          :10;
} __attribute__((packed, aligned(4))) ISP_LTMScaleVPhaseCfg;

/* LTM_SCALE_V_STRIPE_CFG = 0x0994 */
typedef struct ISP_LTMScaleVStripeCfg {
  uint32_t vMnInit                 :14;
  uint32_t /* reserved */          :2;
  uint32_t vPhaseInit              :16;
} __attribute__((packed, aligned(4))) ISP_LTMScaleVStripeCfg;

/* LTM_DC_CFG = 0x0998 */
typedef struct ISP_LTMDcCfg {
  uint32_t initCellNumX            :4;
  uint32_t initDX                  :5;
  uint32_t /* reserved */          :3;
  uint32_t binInitCnt              :4;
  uint32_t /* reserved */          :16;
} __attribute__((packed, aligned(4))) ISP_LTMDcCfg;

/* LTM_IP_INIT_CFG = 0x099C */
typedef struct ISP_LTMIpInitCfg {
  uint32_t initCellNumX            :4;
  uint32_t initDX                  :10;
  uint32_t /* reserved */          :2;
  uint32_t initPX                  :15;
  uint32_t /* reserved */          :1;
} __attribute__((packed, aligned(4))) ISP_LTMIpInitCfg;

/* LTM_IP_INV_CELLWIDTH_CFG = 0x09A0 */
typedef struct ISP_LTMIpInvCellwidthCfg {
  uint32_t invCellwidthL           :11;
  uint32_t /* reserved */          :5;
  uint32_t invCellwidthR           :11;
  uint32_t /* reserved */          :5;
} __attribute__((packed, aligned(4))) ISP_LTMIpInvCellwidthCfg;

/* LTM_IP_INV_CELLHEIGHT_CFG = 0x09A4 */
typedef struct ISP_LTMIpInvCellheightCfg {
  uint32_t invCellheightL          :11;
  uint32_t /* reserved */          :5;
  uint32_t invCellheightR          :11;
  uint32_t /* reserved */          :5;
} __attribute__((packed, aligned(4))) ISP_LTMIpInvCellheightCfg;

/* LTM_IP_CELLWIDTH_CFG = 0x09A8 */
typedef struct ISP_LTMIpCellwidthCfg {
  uint32_t cellwidthL              :10;
  uint32_t /* reserved */          :6;
  uint32_t cellwidthR              :10;
  uint32_t /* reserved */          :6;
} __attribute__((packed, aligned(4))) ISP_LTMIpCellwidthCfg;

/* LTM_IP_INV_CELLHEIGHT_CFG = 0x09AC */
typedef struct ISP_LTMIpCellheightCfg {
  uint32_t cellheightL             :10;
  uint32_t /* reserved */          :6;
  uint32_t cellheightR             :10;
  uint32_t /* reserved */          :6;
} __attribute__((packed, aligned(4))) ISP_LTMIpCellheightCfg;

/* LTM_RGB2Y_CFG0 = 0x09B0 */
typedef struct ISP_LTMRGB2YCfg0 {
  uint32_t c1                      :10;
  uint32_t c2                      :10;
  uint32_t c3                      :10;
  uint32_t /* reserved */          :2;
} __attribute__((packed, aligned(4))) ISP_LTMRGB2YCfg0;

/* LTM_RGB2Y_CFG1 = 0x09B4 */
typedef struct ISP_LTMRGB2YCfg1 {
  uint32_t thr                     :10;
  uint32_t /* reserved */            :6;
  uint32_t k                       :10;
  uint32_t c4 /* reserved */         :6;  /* This variable is added to be parellel with 8994 */
} __attribute__((packed, aligned(4))) ISP_LTMRGB2YCfg1;

/* ######################### PADDING REGISTERS ###################### */

/* LTM_SCALE_H_PAD_CFG = 0x0AB0 */
typedef struct ISP_LTMScaleHPadCfg {
  uint32_t scaleYInWidth           :13;
  uint32_t /* reserved */          :3;
  uint32_t hSkipCnt                :13;
  uint32_t /* reserved */          :3;
} __attribute__((packed, aligned(4))) ISP_LTMScaleHPadCfg;

/* LTM_SCALE_V_PAD_CFG = 0x0AB4 */
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
