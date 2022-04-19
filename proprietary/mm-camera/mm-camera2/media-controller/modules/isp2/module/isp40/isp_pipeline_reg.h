/* Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ISP_PIPELINE_REG40_H__
#define __ISP_PIPELINE_REG40_H__

#define ISP_PIPELINE_WIDTH      12
#define ISP_STATS_BIT_DEPTH     8
#define ISP_STATS_BITS_DIFF (ISP_STATS_BIT_DEPTH - 8)
#define MESH_ROLLOFF_TABLE_SIZE 13 * 10
#define ISP_STATS_CFG           0x00000888
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

#define STATS_BIT_DEPTH           8
#define ISP_RGB_LUT40_BANK_SEL_MASK      0x00000007

#endif /* __ISP_PIPELINE_REG40_H__ */
