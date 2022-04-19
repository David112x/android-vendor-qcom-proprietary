/* gic46_reg.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __GIC46_REG_H__
#define __GIC46_REG_H__

#define ISP_GIC_OFF 0x000006E8
#define ISP_GIC_LEN 4

#define GIC_CGC_OVERRIDE TRUE
#define GIC_CGC_OVERRIDE_REGISTER 0x2C
#define GIC_CGC_OVERRIDE_BIT 9

/* Green Imbalance Correction Config */
typedef struct ISP_GIC_CfgCmdType {
  /* GIC_CFG */
  uint32_t      lutBankSel                  :1;
  uint32_t     /* reserved */               :19;
  uint32_t      filterStrength              :9;
  uint32_t     /* reserved */               :3;
  /* CGIC_NOISE_CFG_0 */
  uint32_t      noiseScale0                 :12;
  uint32_t     /* reserved */               :4;
  uint32_t      noiseScale1                 :16;
  /* GIC_NOISE_CFG_1 */
  uint32_t      noiseOffset                 :16;
  uint32_t     /* reserved */               :16;
  /* GIC_SOFT_THD_CFG */
  uint32_t      softThNoiseShift            :4;
  uint32_t      softThNoiseScale            :12;
  uint32_t     /* reserved */               :16;
} __attribute__((packed, aligned(4))) ISP_GIC_CfgCmdType;

#endif //__GIC46_REG_H__
