/* abcc_reg.h
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ABCC44_REG_H__
#define __ABCC44_REG_H__

#define ISP_ABCC_CFG_OFF      0x000007EC
#define ABCC_LUT_COUNT        512
#define ABCC_ENABLE           1

#define ABCC_CGC_OVERRIDE_REGISTER  0x14
#define ABCC_CGC_OVERRIDE_BIT       25

typedef union  ISP_ABCC_Cfg_CmdType {
  struct  {
    /* ABCC Config */
    /* Reserved for dbpc enable  */
    uint32_t     /* reserved */                   : 1;
    /* Reserved for dbcc enable  */
    uint32_t     /* reserved */                   : 1;
    /* Reserved for abcc enable */
    uint32_t     enable                           : 1;
    /* Reserved bits 7_3  */
    uint32_t     /* reserved */                   : 5;
    /* Reserved for ABCC LUT Bank Select */
    uint32_t     abcc_lut_bank_sel                : 1;
	/* reserved bits 15_9*/
    uint32_t     /* reserved */                   : 7;
    /* reserved for pipe flush count*/
    uint32_t      /* reserved */                  : 13;
    uint32_t     /* reserved bit 29 */            : 1;
    /* reserved for pipe flush ovd */
    uint32_t     /* reserved */                   : 1;
    /* reserved for flush halt ovd */
    uint32_t     /* reserved */                   : 1;
  }__attribute__((packed, aligned(4)));
  uint32_t cfg_reg;
}__attribute__((packed, aligned(4))) ISP_ABCC_Cfg_CmdType;

#endif /* __ABCC44_REG_H__ */



