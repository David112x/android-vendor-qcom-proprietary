/* isp_pipeline_reg.h
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ISP_PIPELINE_REG48_H__
#define __ISP_PIPELINE_REG48_H__

#define ISP_PIPELINE_WIDTH 14
#define ISP_STATS_BIT_DEPTH  14
#define ISP_STATS_BITS_DIFF (ISP_STATS_BIT_DEPTH - 8)
#define MESH_ROLLOFF_TABLE_SIZE 17 * 13
#define ISP_DMI_CFG_DEFAULT 0x00000100
#define ISP_DMI_CFG_OFF 0x00000C24
#define ISP_DMI_ADDR 0x00000C28
#define ISP_DMI_DATA_HI 0x00000C2C
#define ISP_DMI_DATA_LO 0x00000C30
#define ISP_MODULE_LENS_EN_OFFSET   0x00000040
#define ISP_MODULE_STATS_EN_OFFSET  0x00000044
#define ISP_MODULE_COLOR_EN_OFFSET  0x00000048
#define ISP_MODULE_ZOOM_EN_OFFSET   0x0000004c
#define PEDESTAL_F0_RAM_L_GR_R_BANK0 0x1
#define PEDESTAL_F0_RAM_L_GB_B_BANK0 0x2
#define PEDESTAL_F1_RAM_L_GR_R_BANK0 0x3
#define PEDESTAL_F1_RAM_L_GB_B_BANK0 0x4
#define ISP_SWAP_AEC_BG_HDR_BE TRUE

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
      uint32_t  /* reserved */      : 1;
      uint32_t  pdaf                : 1;
      uint32_t  /* reserved */      :18;

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
      uint32_t  /* dont care */ : 8;
      uint32_t  color_conv     : 1;
      uint32_t  /* dont care */ :23;
    } fields;

    uint32_t value;
  };
} __attribute__((packed, aligned(4))) ISP_STATS_CFG;

typedef struct ISP_ModuleCfg {
  ISP_ModuleEn    mod_en;
  ISP_CS_MCE_CFG  cs_mce_cfg;
  ISP_STATS_CFG   stats_cfg;
  ISP_CAC_CFG     cac_cfg;
} ISP_ModuleCfg;
#define ISP_DMI_NO_MEM_SELECTED 0x0
#define PEDESTAL_RAM_L_GR_R_BANK0 0x1
#define PEDESTAL_RAM_L_GB_B_BANK0 0x2
#define PEDESTAL_RAM_R_GR_R_BANK0 0x3
#define PEDESTAL_RAM_R_GB_B_BANK0 0x4
#define PEDESTAL_RAM_L_GR_R_BANK1 0x5
#define PEDESTAL_RAM_L_GB_B_BANK1 0x6
#define PEDESTAL_RAM_R_GR_R_BANK1 0x7
#define PEDESTAL_RAM_R_GB_B_BANK1 0x8
#define BLACK_LUT_RAM_BANK0 0x9
#define BLACK_LUT_RAM_BANK1 0xA
#define PDAF_LUT_BANK 0xB
#define ABF_STD2_L0_BANK0 0xC
#define ABF_STD2_L0_BANK1 0xD
#define ROLLOFF_RAM_L_GR_R_BANK0 0xE
#define ROLLOFF_RAM_L_GB_B_BANK0 0xF
#define ROLLOFF_RAM_R_GR_R_BANK0 0x10
#define ROLLOFF_RAM_R_GB_B_BANK0 0x11
#define ROLLOFF_RAM_L_GR_R_BANK1 0x12
#define ROLLOFF_RAM_L_GB_B_BANK1 0x13
#define ROLLOFF_RAM_R_GR_R_BANK1 0x14
#define ROLLOFF_RAM_R_GB_B_BANK1 0x15
#define GIC_NOISE_STD2_LUT_BANK0 0x16
#define GIC_NOISE_STD2_LUT_BANK1 0x17
#define GTM_LUT_RAM_BANK0 0x18
#define GTM_LUT_RAM_BANK1 0x19
#define RGBLUT_RAM_CH0_BANK0 0x1A
#define RGBLUT_RAM_CH0_BANK1 0x1B
#define RGBLUT_RAM_CH1_BANK0 0x1C
#define RGBLUT_RAM_CH1_BANK1 0x1D
#define RGBLUT_RAM_CH2_BANK0 0x1E
#define RGBLUT_RAM_CH2_BANK1 0x1F
#define RGBLUT_CHX_BANK0 0x20
#define RGBLUT_CHX_BANK1 0x21
#define LTM_MC_LUT_BANK0 0x22
#define LTM_MC_LUT_BANK1 0x23
#define LTM_MS_LUT_BANK0 0x24
#define LTM_MS_LUT_BANK1 0x25
#define LTM_SC_LUT_BANK0 0x26
#define LTM_SC_LUT_BANK1 0x27
#define LTM_SS_LUT_BANK0 0x28
#define LTM_SS_LUT_BANK1 0x29
#define LTM_MASK_LUT_BANK0 0x2A
#define LTM_MASK_LUT_BANK1 0x2B
#define LTM_WEIGHT_LUT_BANK0 0x2C
#define LTM_WEIGHT_LUT_BANK1 0x2D
#define LTM_PING_RAM0 0x2E
#define LTM_PING_RAM1 0x2F
#define LTM_PING_RAM2 0x30
#define LTM_PONG_RAM0 0x31
#define LTM_PONG_RAM1 0x32
#define LTM_PONG_RAM2 0x33
#define LTM_SAT_LUT_BANK0 0x34
#define LTM_SAT_LUT_BANK1 0x35
#define STATS_HDR_BHIST_RAM0 0x36
#define STATS_HDR_BHIST_RAM1 0x37
#define STATS_HDR_BHIST_RAM2 0x38
#define STATS_HDR_BHIST_RAMX 0x39
#define STATS_BHIST_RAM 0x3A
#define STATS_IHIST_Y_RAM 0x3B
#define STATS_IHIST_R_RAM 0x3C
#define STATS_IHIST_G_RAM 0x3D
#define STATS_IHIST_B_RAM 0x3E
#define STATS_IHIST_X_RAM 0x3F
#define STATS_BAF_RGN_IND_LUT_BANK0 0x40
#define STATS_BAF_RGN_IND_LUT_BANK1 0x41
#define STATS_BAF_GAMMA_LUT_BANK0 0x42
#define STATS_BAF_GAMMA_LUT_BANK1 0x43

#define STATS_BIT_DEPTH 10

#endif /* __ISP_PIPELINE_REG48_H__ */
