/* isp_pipeline_reg.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ISP_PIPELINE_REG44_H__
#define __ISP_PIPELINE_REG44_H__

#define ISP_PIPELINE_WIDTH      12
#define ISP_STATS_BIT_DEPTH     8
#define ISP_STATS_BITS_DIFF (ISP_STATS_BIT_DEPTH - 8)
#define MESH_ROLLOFF_TABLE_SIZE 17 * 13
#define ISP_STATS_CFG_LEN       4
#define ISP_DMI_CFG_DEFAULT     0x00000100
#define ISP_DMI_CFG_OFF         0x00000910
#define ISP_DMI_ADDR            0x00000914
#define ISP_DMI_DATA_HI         0x00000918
#define ISP_DMI_DATA_LO         0x0000091c
#define ISP_DMI_NO_MEM_SELECTED 0
#define ISP_SWAP_AEC_BG_HDR_BE  FALSE

#define BLACK_LUT_RAM_BANK0       0x1
#define BLACK_LUT_RAM_BANK1       0x2
#define ROLLOFF_RAM_L_GR_R_BANK0  0x3
#define ROLLOFF_RAM_L_GB_B_BANK0  0x4
#define BPC_LUT_RAM_BANK0         0x5
#define BPC_LUT_RAM_BANK1         0x6
#define STATS_BHIST_RAM0          0x7
#define STATS_BHIST_RAM1          0x8
#define RGBLUT_RAM_CH0_BANK0      0x9
#define RGBLUT_RAM_CH0_BANK1      0xa
#define RGBLUT_RAM_CH1_BANK0      0xb
#define RGBLUT_RAM_CH1_BANK1      0xc
#define RGBLUT_RAM_CH2_BANK0      0xd
#define RGBLUT_RAM_CH2_BANK1      0xe
#define RGBLUT_CHX_BANK0          0xf
#define RGBLUT_CHX_BANK1          0x10
#define STATS_IHIST_RAM           0x11
#define LA_LUT_RAM_BANK0          0x12
#define LA_LUT_RAM_BANK1          0x13
#define ROLLOFF_RAM_R_GR_R_BANK0  0x14
#define ROLLOFF_RAM_R_GB_B_BANK0  0x15
#define ROLLOFF_RAM_L_GR_R_BANK1  0x16
#define ROLLOFF_RAM_L_GB_B_BANK1  0x17
#define ROLLOFF_RAM_R_GR_R_BANK1  0x18
#define ROLLOFF_RAM_R_GB_B_BANK1  0x19
#define LTM_MC_LUT_BANK0          0x20
#define LTM_MC_LUT_BANK1          0x21
#define LTM_MS_LUT_BANK0          0x22
#define LTM_MS_LUT_BANK1          0x23
#define LTM_SC_LUT_BANK0          0x24
#define LTM_SC_LUT_BANK1          0x25
#define LTM_SS_LUT_BANK0          0x26
#define LTM_SS_LUT_BANK1          0x27
#define LTM_MASK_LUT_BANK0        0x28
#define LTM_MASK_LUT_BANK1        0x29
#define LTM_WEIGHT_LUT_BANK0      0x2a
#define LTM_WEIGHT_LUT_BANK1      0x2b
#define LTM_PING_RAM0             0x2c
#define LTM_PING_RAM1             0x2d
#define LTM_PING_RAM2             0x2e
#define LTM_PONG_RAM0             0x2f
#define LTM_PONG_RAM1             0x30
#define LTM_PONG_RAM2             0x31
#define ABF_STD2_L0_BANK0         0x32
#define ABF_STD2_L0_BANK1         0x33
#define ABF_STD2_L1_BANK0         0x34
#define ABF_STD2_L1_BANK1         0x35
#define ABF_SIG2_L0_BANK0         0x36
#define ABF_SIG2_L0_BANK1         0x37
#define ABF_SIG2_L1_BANK0         0x38
#define ABF_SIG2_L1_BANK1         0x39

#define STATS_BIT_DEPTH 8 // value range 0 - 255 (even though it might be a 10-bit sensor)

#define ISP_MODULE_CFG_OFFSET 0x18
typedef struct ISP_ModuleEn {

  union {
    struct __attribute__((packed, aligned(4))) {
      /* MODULE_LENS_EN */
      uint32_t  black               : 1; /* 0 */
      uint32_t  rolloff             : 1; /* 1 */
      uint32_t  demux               : 1; /* 2 */
      uint32_t  chroma_upsample     : 1; /* 3 */
      uint32_t  demosaic            : 1; /* 4 */
      uint32_t  be                  : 1; /* 5 */
      uint32_t  bg                  : 1; /* 6 */
      uint32_t  bf                  : 1; /* 7 */
      uint32_t  awb                 : 1; /* 8 */
      uint32_t  rs                  : 1; /* 9 */
      uint32_t  cs                  : 1; /* 10 */
      uint32_t  /* wb - not used */ : 1; /* 11 */
      uint32_t  clf                 : 1; /* 12 */
      uint32_t  color_correct       : 1; /* 13 */
      uint32_t  gamma               : 1; /* 14 */
      uint32_t  ihist               : 1; /* 15 */
      uint32_t  la                  : 1; /* 16 */
      uint32_t  chroma_enhan        : 1; /* 17 */
      uint32_t  skin_bhist          : 1; /* 18 */
      uint32_t  chroma_suppress     : 1; /* 19 */
      uint32_t  skin_enhan          : 1; /* 20 */
      uint32_t  color_xform_enc     : 1; /* 21 */
      uint32_t  color_xform_view    : 1; /* 22 */
      uint32_t  scaler_enc          : 1; /* 23 */
      uint32_t  scaler_view         : 1; /* 24 */
      uint32_t  bpc                 : 1; /* 25 */
      uint32_t  /* reserved */      : 1; /* 26 */
      uint32_t  crop_enc            : 1; /* 27 */
      uint32_t  crop_view           : 1; /* 28 */
      uint32_t  /* realign */       : 1; /* 29 */
      uint32_t  ltm                 : 1; /* 30 */
      uint32_t  abf                 : 1; /* 31 */
    } fields;

    uint32_t value;
  };
} __attribute__((packed, aligned(4))) ISP_ModuleEn;

#define ISP_WB_CLF_CFG_OFFSET     0x588
#define ISP_WB_CFG0_OFFSET        0x58C
#define ISP_WB_CFG1_OFFSET        0x590
typedef struct ISP_WB_CFG {
  union {
    struct  __attribute__((packed, aligned(4))) {
      /* CLF_CFG */
      uint32_t wb_only              : 1; /* 0 */
      uint32_t  /* reserved */      :31;

      /* WB_CFG_0 */
      uint32_t  ch0_gain            :12;
      uint32_t  /* reserved */      : 4;
      uint32_t  ch1_gain            :12;
      uint32_t /* reserved */       : 4;

      /* WB_CFG_1 */
      uint32_t  ch2_gain            :12;
      uint32_t  /* reserved */      :20;
    } fields;

    struct {
      uint32_t wb_clf_cfg;
      uint32_t wb_cfg_0;
      uint32_t wb_cfg_1;
    } values;
  };
} __attribute__((packed, aligned(4))) ISP_WB_CFG;

#define ISP_BPC_CFG_OFFSET       0x7EC
typedef struct ISP_BPC_CFG {
  union {
    struct  __attribute__((packed, aligned(4))) {
      uint32_t  dbpc_en         : 1; /* 0 */
      uint32_t  dbcc_en         : 1; /* 1 */
      uint32_t  abcc_en         : 1; /* 2 */
      uint32_t  /* dont care */ :29;
    } fields;

    uint32_t value;
  };
} __attribute__((packed, aligned(4))) ISP_BPC_CFG;

#define ISP_CS_MCE_CFG_OFFSET     0x668
typedef struct ISP_CS_MCE_CFG {
  union {
    struct  __attribute__((packed, aligned(4))) {
      uint32_t  /* dont care */ :24;
      uint32_t  cs_en           : 1; /* 24 */
      uint32_t  /* dont care */ : 3;
      uint32_t  mce_en          : 1; /* 28 */
      uint32_t  /* dont care */ : 3;
    } fields;

    uint32_t value;
  };
} __attribute__((packed, aligned(4))) ISP_CS_MCE_CFG;

#define ISP_STATS_CFG_OFFSET     0x888
typedef struct ISP_STATS_CFG {
  union {
    struct  __attribute__((packed, aligned(4))) {
      uint32_t  color_conv_en   : 1; /* 0 */
      uint32_t  bhist_sel       : 1; /* 1 */
      uint32_t  bf_scale_en     : 1; /* 2 */
      uint32_t  /* dont care */ : 29;
    } fields;

    uint32_t value;
  };
} __attribute__((packed, aligned(4))) ISP_STATS_CFG;

typedef struct ISP_ModuleCfg {
  ISP_ModuleEn    mod_en;
  ISP_WB_CFG      wb_cfg;
  ISP_BPC_CFG     bpc_cfg;
  ISP_CS_MCE_CFG  cs_mce_cfg;
  ISP_STATS_CFG   stats_cfg;
} ISP_ModuleCfg;

#endif /* __ISP_PIPELINE_REG44_H__ */
