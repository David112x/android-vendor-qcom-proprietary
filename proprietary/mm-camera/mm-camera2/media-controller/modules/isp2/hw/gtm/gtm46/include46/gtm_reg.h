/* gtm46_reg.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __GTM46_REG_H__
#define __GTM46_REG_H__

#define ISP_GTM_OFF 0x000006D4

#define GTM_CGC_OVERRIDE TRUE
#define GTM_CGC_OVERRIDE_REGISTER 0x34
#define GTM_CGC_OVERRIDE_BIT 2

#define USE_GTM46_FIXED_CURVE 0

typedef struct ISP_GTM_Cfg {
  union {
    struct __attribute__((packed, aligned(4))) {
      /* LUT Bank Select Config */
      uint32_t lutBankSel         : 1;
      uint32_t /* reserved */     :31;
    } fields;
    struct __attribute__((packed, aligned(4))) {
      uint32_t value;
    } bytes;
  };
} ISP_GTM_Cfg;

#endif //__GTM46_REG_H__
