/*============================================================================

  Copyright (c) 2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __PEDESTAL_CORRECT_REG_H__
#define __PEDESTAL_CORRECT_REG_H__

#define ISP_PEDESTAL_OFF 0x000004B0
#define ISP_PEDESTAL_LEN (11 * sizeof(uint32_t))

#define PEDESTAL_CGC_OVERRIDE TRUE
#define PEDESTAL_CGC_OVERRIDE_REGISTER 0x2C
#define PEDESTAL_CGC_OVERRIDE_BIT      0
#define PEDESTAL_MIN_STRIPE_OVERLAP    32

#define PEDESTAL_T1_T2  1

/* Pedestal Correction Config */
typedef struct ISP_Pedestal_CfgCmdType {
  /* PEDESTAL_CFG */
  uint32_t                     scaleBypass           :  1;
  uint32_t                     hdrEnable             :  1;
  uint32_t                     /* reserved */        :  6;
  uint32_t                     lutBankSel            :  1;
  uint32_t                     /* reserved */        : 23;
   /* PEDESTAL_GRID_CFG_0 */
  uint32_t                     blockWidth            : 11;
  uint32_t                     /* reserved */        :  5;
  uint32_t                     blockHeight           : 11;
  uint32_t                     /* reserved */        :  5;
  /* PEDESTAL_GRID_CFG_1 */
  uint32_t                     subGridHeight         : 10;
  uint32_t                     /* reserved */        :  2;
  uint32_t                     subGridYDelta         : 17;
  uint32_t                     /* reserved */        :  1;
  uint32_t                     interpFactor          :  2;
  /* PEDESTAL_GRID_CFG_2 */
  uint32_t                     subGridWidth          : 11;
  uint32_t                     /* reserved */        :  1;
  uint32_t                     subGridXDelta         : 17;
  uint32_t                     /* reserved */        :  3;
   /* PEDESTAL_RIGHT_GRID_CFG_0 */
  uint32_t                     blockWidth_r          : 11;
  uint32_t                     /* reserved */        :  5;
  uint32_t                     blockHeight_r         : 11;
  uint32_t                     /* reserved */        :  5;
  /* PEDESTAL_RIGHT_GRID_CFG_1 */
  uint32_t                     subGridHeight_r       : 10;
  uint32_t                     /* reserved */        :  2;
  uint32_t                     subGridYDelta_r       : 17;
  uint32_t                     /* reserved */        :  1;
  uint32_t                     interpFactor_r        :  2;
  /* PEDESTAL_RIGHT_GRID_CFG_2 */
  uint32_t                     subGridWidth_r        : 11;
  uint32_t                     /* reserved */        :  1;
  uint32_t                     subGridXDelta_r       : 17;
  uint32_t                     /* reserved */        :  3;
  /* PEDESTAL_STRIPE_CFG_0 */
  uint32_t                     blockXIndex           :  4;
  uint32_t                     blockYIndex           :  4;
  uint32_t                     yDeltaAccum           : 20;
  uint32_t                     /* reserved */        :  4;
  /* PEDESTAL_STRIPE_CFG_1 */
  uint32_t                     pixelXIndex           : 11;
  uint32_t                     /* reserved */        :  1;
  uint32_t                     pixelYIndex           : 10;
  uint32_t                     /* reserved */        :  2;
  uint32_t                     subGridXIndex         :  3;
  uint32_t                     /* reserved */        :  1;
  uint32_t                     subGridYIndex         :  3;
  uint32_t                     /* reserved */        :  1;
  /* PEDESTAL_STRIPE_CFG_0 */
  uint32_t                     blockXIndex_r         :  4;
  uint32_t                     blockYIndex_r         :  4;
  uint32_t                     yDeltaAccum_r         : 20;
  uint32_t                     /* reserved */        :  4;
  /* PEDESTAL_STRIPE_CFG_1 */
  uint32_t                     pixelXIndex_r         : 11;
  uint32_t                     /* reserved */        :  1;
  uint32_t                     pixelYIndex_r         : 10;
  uint32_t                     /* reserved */        :  2;
  uint32_t                     subGridXIndex_r       :  3;
  uint32_t                     /* reserved */        :  1;
  uint32_t                     subGridYIndex_r       :  3;
  uint32_t                     /* reserved */        :  1;

} __attribute__((packed, aligned(4))) ISP_Pedestal_CfgCmdType;

#endif //__PEDESTAL_CORRECT_REG_H__
