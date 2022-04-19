/* rccb_reg.h
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef __RCCB_REG_H__
#define __RCCB_REG_H__

#define ISP_RCCB_WB_B_GAIN 0x00000444
#define ISP_RCCB_WB_R_GAIN 0x00000454

#define ISP_RCCB_WB_B_OFFSET 0x000004A4
#define ISP_RCCB_WB_R_OFFSET 0x000004E0

#define ISP_RCCB_WB_G_GAIN 0x00000568
#define ISP_RCCB_WB_G_OFFSET 0x0000056C

#define ISP_RCCB_WB_GAIN_MASK 0x0FFF0000
#define ISP_RCCB_WB_OFFSET_MASK 0x7FFC0000

/* Important note:
   These structures represent demosaic HW module registers.
   Please do not arbitrary add/remove fields or modify
   their types or their order */

typedef union ISP_RccbWbGain {
  struct {
    uint32_t   /* reserved */                : 16;
    uint32_t   gain                          : 12;
    uint32_t   /* reserved */                : 4;
  }__attribute__((packed, aligned(4)));
  uint32_t reg;
}__attribute__((packed, aligned(4))) ISP_RccbWbGain;

typedef union ISP_RccbWbOffset {
  struct {
    uint32_t   /* reserved */                : 18;
    uint32_t   offset                        : 13;
    uint32_t   /* reserved */                : 1;
  }__attribute__((packed, aligned(4)));
  uint32_t reg;
}__attribute__((packed, aligned(4))) ISP_RccbWbOffset;


#endif //__RCCB_REG_H__
