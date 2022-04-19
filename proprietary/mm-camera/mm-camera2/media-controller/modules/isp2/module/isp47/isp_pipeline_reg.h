/* isp_pipeline_reg.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#ifndef __ISP_PIPELINE_REG47_H__
#define __ISP_PIPELINE_REG47_H__

#define ISP_PIPELINE_WIDTH 14
#define ISP_STATS_BIT_DEPTH  14
#define ISP_STATS_BITS_DIFF (ISP_STATS_BIT_DEPTH - 8)
#define MESH_ROLLOFF_TABLE_SIZE 17 * 13
#define ISP_DMI_CFG_DEFAULT 0x00000100
#define ISP_DMI_CFG_OFF 0x00000C24
#define ISP_DMI_ADDR 0x00000C28
#define ISP_DMI_DATA_HI 0x00000C2C
#define ISP_DMI_DATA_LO 0x00000C30
#define ISP_SWAP_AEC_BG_HDR_BE  FALSE

#define ISP_MODULE_LENS_EN_OFFSET   0x00000040
#define ISP_MODULE_STATS_EN_OFFSET  0x00000044
#define ISP_MODULE_COLOR_EN_OFFSET  0x00000048
#define ISP_MODULE_ZOOM_EN_OFFSET   0x0000004c
typedef struct ISP_ModuleEn {
  union {
    struct __attribute__((packed, aligned(4))) {
      /* MODULE_LENS_EN */
      uint32_t  pedestal            : 1;
      uint32_t  black               : 1;
      uint32_t  demux               : 1;
      uint32_t  chroma_upsample     : 1;
      uint32_t  hdr_recon           : 1;
      uint32_t  hdr_mac             : 1;
      uint32_t  bpc                 : 1;
      uint32_t  abf                 : 1;
      uint32_t  rolloff             : 1;
      uint32_t  gic                 : 1;
      uint32_t  demosaic            : 1;
      uint32_t  bls                 : 1;
      uint32_t  /* reserved */      :20;

      /* MODULE_STATS_EN */
      uint32_t  hdr_be              : 1;
      uint32_t  hdr_bhist           : 1;
      uint32_t  bf                  : 1;
      uint32_t  bg                  : 1;
      uint32_t  skin_bhist          : 1;
      uint32_t  rs                  : 1;
      uint32_t  cs                  : 1;
      uint32_t  ihist               : 1;
      uint32_t  aec_bg              : 1;
      uint32_t  /* reserved */      :23;

      /* MODULE_COLOR_EN */
      uint32_t  cac_snr             : 1;
      uint32_t  color_correct       : 1;
      uint32_t  gtm                 : 1;
      uint32_t  gamma               : 1;
      uint32_t  ltm                 : 1;
      uint32_t  chroma_enhan        : 1;
      uint32_t  chroma_suppress     : 1;
      uint32_t  skin_enhan          : 1;
      uint32_t  /* reserved */      :24;

      /* MODULE_ZOOM_EN */
      uint32_t  color_xform_enc     : 1;
      uint32_t  scaler_enc          : 1;
      uint32_t  crop_enc            : 1;
      uint32_t  color_xform_view    : 1;
      uint32_t  scaler_view         : 1;
      uint32_t  crop_view           : 1;
      uint32_t  color_xform_vid     : 1;
      uint32_t  scaler_vid          : 1;
      uint32_t  crop_vid            : 1;
      uint32_t  realign_buf         : 1;
      uint32_t  /* reserved */      :22;
    } fields;

    struct {
      uint32_t lens_en;
      uint32_t stats_en;
      uint32_t color_en;
      uint32_t zoom_en;
    } values;
  };
} __attribute__((packed, aligned(4))) ISP_ModuleEn;

#define ISP_CAC_SNR_OFFSET        0x818
typedef struct ISP_CAC_CFG {
  union {
    struct  __attribute__((packed, aligned(4))) {
      uint32_t  cac_enable          : 1;
      uint32_t  /* reserved */      : 7;
      uint32_t  snr_enable          : 1;
      uint32_t /* reserved */       : 23;
     } fields;

     uint32_t value;
   };
} __attribute__((packed, aligned(4))) ISP_CAC_CFG;

#define ISP_BPC_CFG_OFFSET       0x5C4
typedef struct ISP_BPC_CFG {
  union {
    struct  __attribute__((packed, aligned(4))) {
      uint32_t  dpc_en         : 1;
      uint32_t  abcc_en         : 1;
      uint32_t  /* dont care */ :30;
    } fields;

    uint32_t value;
  };
} __attribute__((packed, aligned(4))) ISP_BPC_CFG;

#define ISP_CS_MCE_CFG_OFFSET     0x848
typedef struct ISP_CS_MCE_CFG {
  union {
    struct  __attribute__((packed, aligned(4))) {
      uint32_t  /* dont care */ :24;
      uint32_t  cs_en           : 1;
      uint32_t  /* dont care */ : 3;
      uint32_t  mce_en          : 1;
      uint32_t  /* dont care */ : 3;
    } fields;

    uint32_t value;
  };
} __attribute__((packed, aligned(4))) ISP_CS_MCE_CFG;

#define ISP_STATS_CFG_OFFSET     0xAB0
typedef struct ISP_STATS_CFG {
  union {
    struct  __attribute__((packed, aligned(4))) {
      uint32_t  /* dont care */ : 7;
      uint32_t  bhist_sel       : 1;
      uint32_t  color_conv     : 1;
      uint32_t  /* dont care */ :23;
    } fields;

    uint32_t value;
  };
} __attribute__((packed, aligned(4))) ISP_STATS_CFG;

typedef struct ISP_ModuleCfg {
  ISP_ModuleEn    mod_en;
  ISP_BPC_CFG     bpc_cfg;
  ISP_CS_MCE_CFG  cs_mce_cfg;
  ISP_STATS_CFG   stats_cfg;
  ISP_CAC_CFG     cac_cfg;
} ISP_ModuleCfg;

#define ISP_DMI_NO_MEM_SELECTED    0
#define BLACK_LUT_RAM_BANK0          0x1
#define BLACK_LUT_RAM_BANK1          0x2
#define PEDESTAL_F0_RAM_L_GR_R_BANK0 0x3
#define PEDESTAL_F0_RAM_L_GB_B_BANK0 0x4
#define PEDESTAL_F0_RAM_R_GR_R_BANK0 0x5
#define PEDESTAL_F0_RAM_R_GB_B_BANK0 0x6
#define PEDESTAL_F1_RAM_L_GR_R_BANK0 0x7
#define PEDESTAL_F1_RAM_L_GB_B_BANK0 0x8
#define PEDESTAL_F1_RAM_R_GR_R_BANK0 0x9
#define PEDESTAL_F1_RAM_R_GB_B_BANK0 0xA
#define PEDESTAL_F0_RAM_L_GR_R_BANK1 0xB
#define PEDESTAL_F0_RAM_L_GB_B_BANK1 0xC
#define PEDESTAL_F0_RAM_R_GR_R_BANK1 0xD
#define PEDESTAL_F0_RAM_R_GB_B_BANK1 0xE
#define PEDESTAL_F1_RAM_L_GR_R_BANK1 0xF
#define PEDESTAL_F1_RAM_L_GB_B_BANK1 0x10
#define PEDESTAL_F1_RAM_R_GR_R_BANK1 0x11
#define PEDESTAL_F1_RAM_R_GB_B_BANK1 0x12
/*remove BPC bank 1 in ISP47 */
#define BPC_LUT_RAM_BANK             0x13
#define ABF_STD2_L0_BANK0            0x15
#define ABF_STD2_L0_BANK1            0x16
#define ABF_STD2_L1_BANK0            0x17
#define ABF_STD2_L1_BANK1            0x18
#define ABF_SIG2_L0_BANK0            0x19
#define ABF_SIG2_L0_BANK1            0x1A
#define ABF_SIG2_L1_BANK0            0x1B
#define ABF_SIG2_L1_BANK1            0x1C
#define ROLLOFF_RAM_L_GR_R_BANK0     0x1D
#define ROLLOFF_RAM_L_GB_B_BANK0     0x1E
#define ROLLOFF_RAM_R_GR_R_BANK0     0x1F
#define ROLLOFF_RAM_R_GB_B_BANK0     0x20
#define ROLLOFF_RAM_L_GR_R_BANK1     0x21
#define ROLLOFF_RAM_L_GB_B_BANK1     0x22
#define ROLLOFF_RAM_R_GR_R_BANK1     0x23
#define ROLLOFF_RAM_R_GB_B_BANK1     0x24
#define GIC_NOISE_STD2_LUT_BANK0     0x25
#define GIC_NOISE_STD2_LUT_BANK1     0x26
#define GIC_SIGNAL_SQR_LUT_BANK0     0x27
#define GIC_SIGNAL_SQR_LUT_BANK1     0x28
#define GIC_ABF_L0_NOISE_STD2_LUT_BANK0 0x29
#define GIC_ABF_L0_NOISE_STD2_LUT_BANK1 0x2A
#define GIC_ABF_L0_SIGNAL_SQR_LUT_BANK0 0x2B
#define GIC_ABF_L0_SIGNAL_SQR_LUT_BANK1 0x2C
#define GTM_LUT_RAM_BANK0               0x2D
#define GTM_LUT_RAM_BANK1               0x2E
#define RGBLUT_RAM_CH0_BANK0            0x2F
#define RGBLUT_RAM_CH0_BANK1            0x30
#define RGBLUT_RAM_CH1_BANK0            0x31
#define RGBLUT_RAM_CH1_BANK1            0x32
#define RGBLUT_RAM_CH2_BANK0            0x33
#define RGBLUT_RAM_CH2_BANK1            0x34
#define RGBLUT_CHX_BANK0                0x35
#define RGBLUT_CHX_BANK1                0x36
#define LTM_MC_LUT_BANK0                0x37
#define LTM_MC_LUT_BANK1                0x38
#define LTM_MS_LUT_BANK0                0x39
#define LTM_MS_LUT_BANK1                0x3A
#define LTM_SC_LUT_BANK0                0x3B
#define LTM_SC_LUT_BANK1                0x3C
#define LTM_SS_LUT_BANK0                0x3D
#define LTM_SS_LUT_BANK1                0x3E
#define LTM_MASK_LUT_BANK0              0x3F
#define LTM_MASK_LUT_BANK1              0x40
#define LTM_WEIGHT_LUT_BANK0            0x41
#define LTM_WEIGHT_LUT_BANK1            0x42
#define LTM_PING_RAM0                   0x43
#define LTM_PING_RAM1                   0x44
#define LTM_PING_RAM2                   0x45
#define LTM_PONG_RAM0                   0x46
#define LTM_PONG_RAM1                   0x47
#define LTM_PONG_RAM2                   0x48
#define STATS_HDR_BHIST_F0_RAM0         0x49
#define STATS_HDR_BHIST_F0_RAM1         0x4A
#define STATS_BHIST_RAM0                0x4E
#define STATS_BHIST_RAM1                0x4F
#define STATS_BHIST_RAMX                0x50
#define STATS_IHIST_Y_RAM               0x51
#define STATS_IHIST_R_RAM               0x52
#define STATS_IHIST_G_RAM               0x53
#define STATS_IHIST_B_RAM               0x54
#define STATS_IHIST_X_RAM               0x55
/*ISP47 new bank*/
#define ABF_MESH_LUT_BANK0              0x56
#define ABF_MESH_LUT_BANK1              0x57
#define LTM_SAT_LUT_BANK0               0x58
#define LTM_SAT_LUT_BANK1               0x59
#define STATS_BAF_RGN_IND_LUT_BANK0     0x5A
#define STATS_BAF_RGN_IND_LUT_BANK1     0x5B
#define STATS_BAF_GAMMA_LUT_BANK0       0x5C
#define STATS_BAF_GAMMA_LUT_BANK1       0x5D

#define STATS_BIT_DEPTH 10

#endif /* __ISP_PIPELINE_REG47_H__ */
