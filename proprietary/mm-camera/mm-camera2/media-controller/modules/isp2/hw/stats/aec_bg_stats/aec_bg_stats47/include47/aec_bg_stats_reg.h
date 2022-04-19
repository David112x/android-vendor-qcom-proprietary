/* aec_bg_stats_reg.h
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AEC_BG_STATS_REG_H__
#define __AEC_BG_STATS_REG_H__

#define AEC_BG_STATS_OFF 0x00000BAC
#define AEC_BG_STATS_LEN 7

#define AEC_BG_QUAD_SYNC_ENABLE 1
#define AEC_BG_QUAD_SYNC_EN_BIT 5
#define AEC_BG_QUAD_SYNC_EN 1

#define STATS_CFG_OFF_AEC_BG 0x00000AB0
#define STATS_CFG_MASK_AEC_BG (1 << AEC_BG_QUAD_SYNC_EN_BIT)
#define STATS_CFG_VAL_AEC_BG (AEC_BG_QUAD_SYNC_EN << AEC_BG_QUAD_SYNC_EN_BIT)

#define HW_STATS_TYPE MSM_ISP_STATS_AEC_BG

#define ISP_STATS_AEC_BG_GRID_H   64
#define ISP_STATS_AEC_BG_GRID_V   48

/** ISP_StatsAecBg_CfgCmdType:
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
typedef struct ISP_StatsAecBg_CfgCmdType {
  /*  STATS_AEC_BG_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   14;
  uint32_t       /*reserved */          :    2;
  /*  STATS_AEC_BG_RGN_NUM_CFG*/
  uint32_t        rgnHNum               :    8;
  uint32_t      /* reserved */          :    8;
  uint32_t        rgnVNum               :    7;
  uint32_t      /* reserved */          :    9;
  /*  STATS_AEC_BG_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :    9;
  uint32_t      /* reserved */          :    7;
  uint32_t        rgnHeight             :   10;
  uint32_t      /* reserved */          :    6;
  /* STATS_AEC_BG_HI_THRESHOLD_CFG_0 */
  uint32_t        rMax                  :   14;
  uint32_t       /*reserved */          :    2;
  uint32_t        grMax                 :   14;
  uint32_t       /*reserved */          :    2;
  /* STATS_AEC_BG_HI_THRESHOLD_CFG_1 */
  uint32_t        bMax                  :   14;
  uint32_t       /*reserved */          :    2;
  uint32_t        gbMax                 :   14;
  uint32_t       /*reserved */          :    2;
  /* STATS_AEC_BG_LO_THRESHOLD_CFG_0 */
  uint32_t        rMin                  :   14;
  uint32_t       /*reserved */          :    2;
  uint32_t        grMin                 :   14;
  uint32_t       /*reserved */          :    2;
  /* STATS_AEC_BG_LO_THRESHOLD_CFG_1 */
  uint32_t        bMin                  :   14;
  uint32_t       /*reserved */          :    2;
  uint32_t        gbMin                 :   14;
  uint32_t       /*reserved */          :    2;
} __attribute__((packed, aligned(4))) ISP_StatsAecBg_CfgCmdType;

#endif /* __AEC_BG_STATS_REG_H__ */
