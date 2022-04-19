/* gic48_reg.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __gic48_REG_H__
#define __gic48_REG_H__

#define ISP_GIC_OFF 0x000006E8
#define ISP_GIC_LEN 1

#define GIC_CGC_OVERRIDE TRUE
#define GIC_CGC_OVERRIDE_REGISTER 0x2C
#define GIC_CGC_OVERRIDE_BIT 9

#define GIC_NOISE_STD2_LUT_SIZE             64 //GIC2_NOISESTD_LENGTH
/* Green Imbalance Correction Config */
typedef struct ISP_GIC_CfgCmdType {
  /* GIC_CFG */
  uint32_t      lutBankSel                  :1;
  uint32_t      noiseOffset                 :14;
  uint32_t     /* reserved */               :5;
  uint32_t      filterStrength              :9;
  uint32_t     /* reserved */               :3;
} __attribute__((packed, aligned(4))) ISP_GIC_CfgCmdType;

#endif //__gic48_REG_H__
