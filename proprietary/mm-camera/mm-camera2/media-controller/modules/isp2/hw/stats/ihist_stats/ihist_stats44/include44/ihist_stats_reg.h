/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
#ifndef __IHIST_STATS_REG44_H__
#define __IHIST_STATS_REG44_H__

#define IHIST_TABLE_LENGTH 256

#define IHIST_STATS_OFF    0x000008F4
#define IHIST_STATS_LEN    2

/*memory size by bytes*/
#define ISP_STATS_IHIST_BUF_NUM  4
#define ISP_STATS_IHIST_BUF_SIZE 128 * 4

#define IHIST_CGC_OVERRIDE TRUE
#define IHIST_CGC_OVERRIDE_REGISTER 0x14
#define IHIST_CGC_OVERRIDE_BIT 15

/** ISP_StatsBf_CfgCmdType:
 *
 *  @rgnHOffset:  H offset
 *  @channelSelect: Input select for ihist: 0x0 = Y,
 *                  0x1 = Cb, 0x2 = Cr, 0x3 = Ch0,
 *                  0x4 = Ch1, 0x5 = Ch2
 *  @rgnVOffset: V offset
 *  @shiftBits:  Right shift bits for ihist counts
 *  @siteSelect: Specify the site to collect ihist
 *               0x0 = before LA; 0x1 = after LA
 *  @rgnHNum: H num
 *  @rgnVNum: V num

 **/
/* Stats ihist Config*/
typedef struct ISP_StatsIhist_CfgType {
  /*  VFE_STATS_IHIST_RGN_OFFSET_CFG  */
  uint32_t        rgnHOffset            :   13;
  uint32_t        channelSelect         :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t        shiftBits             :    3;
  uint32_t        siteSelect            :    1;

  /*  VFE_STATS_IHIST_RGN_SIZE_CFG  */
  uint32_t        rgnHNum               :   12;
  uint32_t        rgnVNum               :   11;
  uint32_t      /* reserved */          :    9;
}__attribute__((packed, aligned(4))) ISP_StatsIhist_CfgType;

#endif /* __IHIST_STATS_REG44_H__ */
