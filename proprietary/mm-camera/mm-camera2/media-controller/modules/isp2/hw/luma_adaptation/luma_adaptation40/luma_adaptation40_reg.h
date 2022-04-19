/* color_correct40_reg.h
 *
 * Copyright (c) 2013, 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __LUMA_ADAPTATION40_REG_H__
#define __LUMA_ADAPTATION40_REG_H__

#define ISP_LA40_OFF 0x0000063C
#define ISP_LA40_LEN 1
#define GAMMA_HW_PACK_BIT             8
#define GAMMA_NUM_ENTRIES             64

typedef struct ISP_LABankSelCfg {
  /* LA Config */
  uint32_t      lutBankSelect               : 1;
  uint32_t     /* reserved */               :31;
}__attribute__((packed, aligned(4))) ISP_LABankSelCfg;

#endif /* __COLOR_CORRECT40_REG_H__ */
