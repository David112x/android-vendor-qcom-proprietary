/* gtm46_reg.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __GTM46_REG_H__
#define __GTM46_REG_H__

#define ISP_GTM_OFF 0x00000794

#define GTM_CGC_OVERRIDE TRUE
#define GTM_CGC_OVERRIDE_REGISTER 0x34
#define GTM_CGC_OVERRIDE_BIT 2

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

typedef struct ISP_GTM_CfgEn {
  /* Module Color Enable */
  uint32_t      reserved1      :2;
  uint32_t      module_en      :1;
  uint32_t      reserved2      :29;
} __attribute__((packed, aligned(4))) ISP_GTM_CfgEn;

#endif //__GTM46_REG_H__
