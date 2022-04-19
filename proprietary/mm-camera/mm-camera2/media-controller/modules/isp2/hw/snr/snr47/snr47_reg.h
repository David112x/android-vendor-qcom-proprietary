/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __SNR47_REG_H__
#define __SNR47_REG_H__

#define ISP_SNR47_CFG_OFF 0x00000818
#define ISP_SNR47_CFG_LEN 1

#define ISP_SNR47_OFF 0x00000820
#define ISP_SNR47_LEN 9

#define MAX_Q_SKIN 128

#define ISP_SNR47_CAC_ON_REGION 2
#define ISP_SNR47_C_COEFF0_DEFAULT 74
#define ISP_SNR47_C_COEFF1_DEFAULT 27
#define ISP_SNR47_C_COEFF2_DEFAULT 0
#define ISP_SNR47_C_COEFF3_DEFAULT 0

typedef union  ISP_SnrCfg_CmdType {
  struct  {
    uint32_t    /* reserved */                   : 8;
    uint32_t    enable                         : 1;
    uint32_t    /* reserved */                   : 23;
  }__attribute__((packed, aligned(4)));
  uint32_t cfg_reg;
}__attribute__((packed, aligned(4))) ISP_SnrCfg_CmdType;

typedef struct ISP_Snr_Filter_Type {
  uint32_t    coeff0                            : 8;
  uint32_t    coeff1                            : 7;
  uint32_t    /* reserved */                      : 1;
  uint32_t    coeff2                            : 7;
  uint32_t    /* reserved */                      : 1;
  uint32_t    coeff3                            : 7;
  uint32_t    /* reserved */                      : 1;
}__attribute__((packed, aligned(4))) ISP_Snr_Filter_Type;

typedef struct ISP_Snr_Filter1_Type {
  uint32_t    coeff0                            : 7;
  uint32_t    /* reserved */                      : 1;
  uint32_t    coeff1                            : 7;
  uint32_t    /* reserved */                      : 1;
  uint32_t    coeff2                            : 7;
  uint32_t    /* reserved */                      : 1;
  uint32_t    coeff3                            : 7;
  uint32_t    /* reserved */                      : 1;
}__attribute__((packed, aligned(4))) ISP_Snr_Filter1_Type;

typedef struct ISP_Snr_BorderCfg0_Type {
  uint32_t    ymin                              : 8;
  uint32_t    ymax                              : 8;
  uint32_t    shymin                            : 8;
  uint32_t    shymax                            : 8;
}__attribute__((packed, aligned(4))) ISP_Snr_BorderCfg0_Type;

typedef struct ISP_Snr_BorderCfg1_Type {
  uint32_t    smin_para                         : 8;
  uint32_t    smax_para                         : 8;
  uint32_t    boundary_prob                     : 4;
  uint32_t    hmin                              : 10;
  uint32_t    /* reserved */                      : 2;
}__attribute__((packed, aligned(4))) ISP_Snr_BorderCfg1_Type;

typedef struct ISP_Snr_BorderCfg2_Type {
  uint32_t    /* reserved */                      : 8;
  uint32_t    hmax                              : 8;
  uint32_t    q_skin                            : 8;
  uint32_t    q_nonskin                         : 8;
}__attribute__((packed, aligned(4))) ISP_Snr_BorderCfg2_Type;

typedef struct ISP_Snr_SkinToneCfg_Type {
  uint32_t    soft_thr                          : 8;
  uint32_t    soft_strength                     : 7;
  uint32_t    /* reserved */                      : 17;
}__attribute__((packed, aligned(4))) ISP_Snr_SkinToneCfg_Type;

typedef struct ISP_Snr_CmdType {
  /** These are in order of HW registers. Do not add variables or change order
    * That will affect hw write
   **/
  ISP_Snr_Filter_Type      nr_filter;
  ISP_Snr_Filter_Type      y_filter;
  ISP_Snr_Filter_Type      c_filter;
  ISP_Snr_BorderCfg0_Type  border_cfg0;
  ISP_Snr_BorderCfg1_Type  border_cfg1;
  ISP_Snr_BorderCfg2_Type  border_cfg2;
  ISP_Snr_Filter_Type      prob_filter;
  ISP_Snr_Filter1_Type     prob_filter1;
  ISP_Snr_SkinToneCfg_Type skinTone_cfg;
}__attribute__((packed, aligned(4))) ISP_Snr_CmdType;

#endif // __SNR47_REG_H__
