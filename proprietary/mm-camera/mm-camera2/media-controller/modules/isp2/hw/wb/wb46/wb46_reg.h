/* wb46_reg.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __WB46_REG_H__
#define __WB46_REG_H__

#define ISP_CLF_WB46_CFG_0            0x0000063C
#define ISP_CLF_WB46_CFG_1            0x00000640
#define ISP_CLF_WB46_RIGHT_CFG_0      0x0000064C
#define ISP_CLF_WB46_RIGHT_CFG_1      0x00000650
#define ISP_CLF_WB46_CFG_OFFSET       0x00000644
#define ISP_CLF_WB46_RIGHT_CFG_OFFSET 0x00000654
#define ISP_CLF_WB46_LEN 2

#define ISP_WB46_INIT_G_GAIN 1
#define ISP_WB46_INIT_B_GAIN 1.8
#define ISP_WB46_INIT_R_GAIN 1

#define ISP46_CLF_WB_MASK 0xFFF

typedef struct ISP_CLF_WB_Cfg_0 {
  /* CLF_WB_CFG_0 */
  uint32_t          ch0Gain             : 12;
  uint32_t         /* reserved */       :  4;
  uint32_t          ch1Gain             : 12;
  uint32_t         /* reserved */       :  4;
}__attribute__((packed, aligned(4))) ISP_WhiteBalanceConfig0_CmdType;

typedef struct ISP_CLF_WB_Cfg_1{
  /* CLF_WB_CFG_1 */
  uint32_t          ch2Gain             : 12;
  uint32_t         /* reserved */       : 19;
  uint32_t          wb_only             :  1;
}__attribute__((packed, aligned(4))) ISP_WhiteBalanceConfig1_CmdType;

typedef struct ISP_CLF_WB_RIGHT_Cfg_0 {
  /* CLF_WB_CFG_0 */
  uint32_t          ch0Gain             : 12;
  uint32_t         /* reserved */       :  4;
  uint32_t          ch1Gain             : 12;
  uint32_t         /* reserved */       :  4;
}__attribute__((packed, aligned(4))) ISP_WhiteBalanceRightConfig0_CmdType;

typedef struct ISP_CLF_WB_RIGHT_Cfg_1 {
  /* CLF_WB_CFG_1 */
  uint32_t          ch2Gain             : 12;
  uint32_t         /* reserved */       : 20;
}__attribute__((packed, aligned(4))) ISP_WhiteBalanceRightConfig1_CmdType;

typedef struct ISP_CLF_WB_OFFSET_Cfg {
  /* CLF_WB_CFG_0 */
  uint32_t          ch0Offset           : 15;
  uint32_t         /* reserved */       :  1;
  uint32_t          ch1Offset           : 15;
  uint32_t         /* reserved */       :  1;
  /* CLF_WB_CFG_1 */
  uint32_t          ch2Offset           : 15;
  uint32_t         /* reserved */       : 17;
}__attribute__((packed, aligned(4))) ISP_WhiteBalanceOffsetCmdType;

#endif //__WB46_REG_H__
