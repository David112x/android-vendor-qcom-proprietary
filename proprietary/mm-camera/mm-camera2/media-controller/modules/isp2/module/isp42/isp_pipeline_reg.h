/* isp_pipeline42_parser.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#ifndef __ISP_PIPELINE_REG42_H__
#define __ISP_PIPELINE_REG42_H__

#define ISP_PIPELINE_WIDTH      12
#define ISP_STATS_BIT_DEPTH     8
#define STATS_BIT_DEPTH         8
#define MESH_ROLLOFF_TABLE_SIZE 13 * 10
#define ISP_STATS_CFG           0x00000888
#define ISP_STATS_CFG_LEN       4
#define ISP_DMI_CFG_DEFAULT     0x00000100
#define ISP_DMI_CFG_OFF         0x00000910
#define ISP_DMI_ADDR            0x00000914
#define ISP_DMI_DATA_HI         0x00000918
#define ISP_DMI_DATA_LO         0x0000091c
#define ISP_DMI_NO_MEM_SELECTED 0
#define ISP_SWAP_AEC_BG_HDR_BE FALSE

#define BLACK_LUT_RAM_BANK0       0x1
#define BLACK_LUT_RAM_BANK1       0x2
#define LENS_ROLLOFF_RAM_BANK0    0x3
#define LENS_ROLLOFF_RAM_BANK1    0x4
//#define DEMOSAIC_LUT_RAM_BANK0    0x5
//#define DEMOSAIC_LUT_RAM_BANK1    0x6
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
#define STATS_BAF_GAMMA_LUT_BANK0 0x14
#define STATS_BAF_GAMMA_LUT_BANK1 0x15
#define STATS_BAF_RGN_IND_LUT_BANK0 0x16
#define STATS_BAF_RGN_IND_LUT_BANK1 0x17

#define ISP_RGB_LUT42_BANK_SEL_MASK      0x00000007

#endif /* __ISP_PIPELINE_REG42_H__ */

