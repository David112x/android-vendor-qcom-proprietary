/* cac47_reg.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __CAC47_REG_H__
#define __CAC47_REG_H__

#define  ISP_CAC47_OFF 0x0000081C
#define  ISP_CAC47_LEN 1

#define  ISP_CAC47_Y_SPOT_TH_LIMIT ((1 << 6) - 1)
#define  ISP_CAC47_Y_SATU_TH_LIMIT ((1 << 8) - 1)

#define  ISP_CAC47_C_SATU_TH_LIMIT ((1 << 7) - 1)
#define  ISP_CAC47_C_SPOT_TH_LIMIT ((1 << 8) - 1)

typedef struct ISP_CacCfgCmdType {
  /*CAC Config*/
  uint32_t      ySpotThr            :6;
  uint32_t     ResMode              :1;
  uint32_t     /* reserved */       :1;
  uint32_t      ySaturationThr      :8;
  uint32_t      cSaturationThr      :7;
  uint32_t     /* reserved */       :1;
  uint32_t      cSpotThr            :8;
}__attribute__((packed, aligned(4))) ISP_CacCfgCmdType;

#endif /* __CAC47_REG_H__ */
