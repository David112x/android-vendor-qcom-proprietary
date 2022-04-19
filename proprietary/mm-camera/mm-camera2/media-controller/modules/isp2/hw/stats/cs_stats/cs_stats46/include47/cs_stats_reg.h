/* cs_stats_reg.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __CS_STATS_REG_H__
#define __CS_STATS_REG_H__

/*CS_STATS 47*/
#define CS_STATS_OFF 0x00000BF8
#define CS_STATS_LEN 3

#define STATS_CFG_OFF 0x00000AB0
#define STATS_LEN 1

#define CS_MAX_V_REGIONS 1
#define CS_MAX_H_REGIONS 1344

#define CS_MIN_RGN_WIDTH 2
#define CS_MAX_RGN_WIDTH 4

#define CS_MAX_SHIFT_BITS 12

typedef union ISP_Stats_CfgCmdType {
struct
  {
    uint32_t     /* hdrBEfieldSel reserved*/     :   2;
    uint32_t     /* BAFSiteSel  reserved*/       :   1;
    uint32_t     /* BAFInpFieldSel reserved*/    :   2;
    uint32_t     /*AecBgQuadSyncEn reserved*/    :   1;
    uint32_t     /* AwbBgQuadSyncEn reserved*/   :   1;
    uint32_t     /* bayerHistSel reserved*/      :   1;
    uint32_t     /* colorConvEn  reserved*/      :   1;
    uint32_t     /* rsShiftBits reserved */      :   4;
    uint32_t     /*HdrBeQuadSyncEn reserved */   :   1;
    uint32_t     csShiftBits                     :   4;
    uint32_t     /* reserved */                  :   6;
    uint32_t     /* iHistChanSel reserved */     :   2;
    uint32_t     /*iHistSiteSel reserved */      :   2;
    uint32_t     /*iHistShiftBits reserved */    :   4;
  }__attribute__((packed, aligned(4)));
  uint32_t mask;
}__attribute__((packed, aligned(4))) ISP_Stats_CfgType;

/** ISP_StatsCs_CfgType:
 *
 *  @rgnHOffset: H offset
 *  @rgnVOffset: V offset
 *  @shiftBits: shift bits
 *  @rgnWidth: region width
 *  @rgnHeight: region height
 *  @rgnHNum: H num
 *  @rgnVNum: V num
 **/
typedef struct ISP_StatsCs_CfgType {
  /*  VFE_STATS_CS_RGN_OFFSET_CFG  */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   14;
  uint32_t      /* reserved */          :    2;

  /*  VFE_STATS_CS_RGN_NUM_CFG  */
  uint32_t        rgnHNum               :   11;
  uint32_t      /* reserved */          :    5;
  uint32_t        rgnVNum               :    2;
  uint32_t      /* reserved */          :   14 ;
  /*  VFE_STATS_CS_RGN_SIZE_CFG  */
  uint32_t        rgnWidth              :    2;
  uint32_t      /* reserved */          :   14;
  uint32_t        rgnHeight             :   14;
  uint32_t       /* reserved */         :    2;
}__attribute__((packed, aligned(4))) ISP_StatsCs_CfgType;

#endif /* __CS_STATS_REG_H__ */
