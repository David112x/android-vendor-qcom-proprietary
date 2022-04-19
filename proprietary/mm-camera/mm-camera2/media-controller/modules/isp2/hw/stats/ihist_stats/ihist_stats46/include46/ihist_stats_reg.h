/* ihist_stats_reg46.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __IHIST_STATS_REG46_H__
#define __IHIST_STATS_REG46_H__

#define IHIST_TABLE_LENGTH 256

#define IHIST_STATS_OFF    0x00000AA4
#define IHIST_STATS_LEN    2

#define STATS_CFG_OFF 0x000009B8
#define STATS_LEN 1

#define IHIST_CGC_OVERRIDE TRUE
#define IHIST_CGC_OVERRIDE_REGISTER 0x30
#define IHIST_CGC_OVERRIDE_BIT 7

typedef union ISP_Stats_CfgCmdType {
struct
{
  uint32_t       /* hdrBEfieldSel reserved*/ :   2;
  uint32_t       /* BFsiteSel   reserved*/   :   1;
  uint32_t       /*BFfieldSel  reserved*/    :   2;
  uint32_t       /*dualBFen  reserved*/      :   1;
  uint32_t       /* BGquadSyncEn reserved*/  :   1;
  uint32_t       /* bayerHistSel reserved*/  :   1;
  uint32_t       /* colorConvEn  reserved*/  :   1;
  uint32_t       /*rsShiftBits reserved*/    :   4;
  uint32_t       /*reserved */               :   1;
  uint32_t       /* csShiftBits reserved*/   :   4;
  uint32_t       /* reserved */              :   2;
  uint32_t       iHistChanSel                :   2;
  uint32_t       /*reserved */               :   1;
  uint32_t       iHistSiteSel                :   2;
  uint32_t       /*reserved */               :   1;
  uint32_t       iHistShiftBits              :   4;
  uint32_t       /*reserved */               :   2;
}__attribute__((packed, aligned(4)));
uint32_t mask;
}__attribute__((packed, aligned(4))) ISP_Stats_CfgType;

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
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   14;
  uint32_t      /* reserved */          :    2;

  /*  VFE_STATS_IHIST_RGN_NUM_CFG  */
  uint32_t        rgnHNum               :   12;
  uint32_t      /* reserved */          :    4;
  uint32_t        rgnVNum               :   13;
  uint32_t      /* reserved */          :    3;


}__attribute__((packed, aligned(4))) ISP_StatsIhist_CfgType;

#endif /* __IHIST_STATS_REG46_H__ */
