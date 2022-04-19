/* rs_stats_reg.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __RS_STATS_REG_H__
#define __RS_STATS_REG_H__

#define RS_STATS_OFF 0x00000A8C
#define RS_STATS_LEN 3

#define ISP_STATS_RS_BUF_NUM  4
/* RS stats46 4096(V) X 4(H) total number of 16-bit outputs */
#define ISP_STATS_RS_BUF_SIZE (4096 * 4) * 2

#define STATS_CFG_OFF 0x000009B8
#define STATS_LEN 1

#define RS_MAX_V_REGIONS 4096
#define RS_MAX_H_REGIONS 4

#define RS_MAX_RGN_HEIGHT 4
#define RS_MAX_SHIFT_BIT  10

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
    uint32_t       rsShiftBits                 :   4;
    uint32_t       /*reserved */               :   1;
    uint32_t       /* csShiftBits  reserved*/  :   4;
    uint32_t       /* reserved */              :   2;
    uint32_t       /* iHistChanSel reserved*/  :   2;
    uint32_t       /*reserved */               :   1;
    uint32_t       /* iHistSiteSel reserved*/  :   2;
    uint32_t       /*reserved */               :   1;
    uint32_t       /* iHistShiftBits reserved*/:   4;
    uint32_t       /*reserved */               :   2;
  }__attribute__((packed, aligned(4)));
  uint32_t mask;
}__attribute__((packed, aligned(4))) ISP_Stats_CfgType;

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
  uint32_t        rgnVOffset          :   14;
  uint32_t      /* reserved */        :    2;

  /*  VFE_STATS_RS_RGN_NUM_CFG  */
  uint32_t        rgnHNum             :    2;
  uint32_t      /* reserved */        :   14;
  uint32_t        rgnVNum             :   12;
  uint32_t       /* reserved */       :    4;

  /*  VFE_STATS_RS_RGN_SIZE_CFG  */
  uint32_t        rgnWidth            :   13;
  uint32_t      /* reserved */        :    3;
  uint32_t        rgnHeight           :    2;
  uint32_t      /* reserved */        :   14;
}__attribute__((packed, aligned(4))) ISP_StatsRs_CfgType;

#endif /* __RS_STATS_REG_H__ */
