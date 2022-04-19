/* bg_stats_reg.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BG_STATS_REG44_H__
#define __BG_STATS_REG44_H__

#define BG_STATS_OFF 0x00000BC8
#define BG_STATS_LEN 7

#define BG_QUAD_SYNC_EN_BIT 6
#define BG_QUAD_SYNC_EN 1

#define STATS_CFG_OFF_BG 0x00000AB0
#define STATS_CFG_MASK_BG (1 << BG_QUAD_SYNC_EN_BIT)
#define STATS_CFG_VAL_BG (BG_QUAD_SYNC_EN << BG_QUAD_SYNC_EN_BIT)

/** ISP_StatsBg_CfgCmdType:
 *
 *   @rgnHOffset: H offset
 *  @rgnVOffset: V offset
 *  @rgnWidth: region width
 *  @rgnHeight: region height
 *  @rgnHNum: H num
 *  @rgnVNum: V num
 *  @rMax: max R
 *  @grMax: max Gr
 *  @bMax: max R
 *  @gbMax: max Gb
 **/
typedef struct ISP_StatsBg_CfgCmdType {
  /*  STATS_AWB_BG_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   14;
  uint32_t       /*reserved */          :    2;
  /*  STATS_AWB_BG_RGN_NUM_CFG*/
  uint32_t        rgnHNum               :    8;
  uint32_t      /* reserved */          :    8;
  uint32_t        rgnVNum               :    7;
  uint32_t      /* reserved */          :    9;
  /*  STATS_AWB_BG_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :    9;
  uint32_t      /* reserved */          :    7;
  uint32_t        rgnHeight             :   10;
  uint32_t      /* reserved */          :    6;
  /* STATS_AWB_BG_HI_THRESHOLD_CFG_0 */
  uint32_t        rMax                  :   14;
  uint32_t       /*reserved */          :    2;
  uint32_t        grMax                 :   14;
  uint32_t       /*reserved */          :    2;
  /* STATS_AWB_BG_HI_THRESHOLD_CFG_1 */
  uint32_t        bMax                  :   14;
  uint32_t       /*reserved */          :    2;
  uint32_t        gbMax                 :   14;
  uint32_t       /*reserved */          :    2;
  /* STATS_AWB_BG_LO_THRESHOLD_CFG_0 */
  uint32_t        rMin                  :   14;
  uint32_t       /*reserved */          :    2;
  uint32_t        grMin                 :   14;
  uint32_t       /*reserved */          :    2;
  /* STATS_AWB_BG_LO_THRESHOLD_CFG_1 */
  uint32_t        bMin                  :   14;
  uint32_t       /*reserved */          :    2;
  uint32_t        gbMin                 :   14;
  uint32_t       /*reserved */          :    2;
} __attribute__((packed, aligned(4))) ISP_StatsBg_CfgCmdType;

#endif /* __BG_STATS_REG_H__ */
