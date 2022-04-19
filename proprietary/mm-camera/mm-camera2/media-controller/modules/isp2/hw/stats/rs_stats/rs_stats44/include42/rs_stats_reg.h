/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#ifndef __RS_STATS_REG44_H__
#define __RS_STATS_REG44_H__

#define RS_STATS_OFF 0x000008E4
#define RS_STATS_LEN 2

#define ISP_STATS_RS_BUF_NUM  4
/* 1024x4 total number of 16-bit outputs */
#define ISP_STATS_RS_BUF_SIZE (1024 * 4) * 2

#define RS_MAX_V_REGIONS 1024
#define RS_MAX_H_REGIONS 4

/** ISP_StatsRs_CfgType:
 *
 *  @rgnHOffset: H offset
 *  @rgnVOffset: V offset
 *  @shiftBits: shift bits
 *  @rgnWidth: region width
 *  @rgnHeight: region height
 *  @rgnHNum: H num
 *  @rgnVNum: V num
 **/
typedef struct ISP_StatsRs_CfgType {
  /*  VFE_STATS_RS_RGN_OFFSET_CFG  */
  uint32_t        rgnHOffset          :   13;
  uint32_t      /* reserved */        :    3;
  uint32_t        rgnVOffset          :   12;
  uint32_t        shiftBits           :    3;
  uint32_t      /* reserved */        :    1;

  /*  VFE_STATS_RS_RGN_SIZE_CFG  */
  uint32_t        rgnWidth            :   13;
  uint32_t        rgnHeight           :    3;
  uint32_t        rgnHNum             :    2;
  uint32_t       /* reserved */       :    2;
  uint32_t        rgnVNum             :   10;
  uint32_t      /* reserved */        :    2;
}__attribute__((packed, aligned(4))) ISP_StatsRs_CfgType;

#endif /* __RS_STATS_REG44_H__ */
