/* hdr_be_stats_reg.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __HDR_BE_STATS_REG_H__
#define __HDR_BE_STATS_REG_H__

#define HDR_BE_STATS_OFF 0x00000AB8
#define HDR_BE_STATS_LEN 7

#define STATS_HDR_BE_CFG 0x0000C98
#define STATS_HDR_BE_CFG_LEN 1

#define ISP_STATS_HDR_BE_GRID_H  64 
#define ISP_STATS_HDR_BE_GRID_V  48

#define HDR_STATS_SITE_SEL_BIT 3
#define HDR_STATS_SITE_SEL_VAL 1

#define HDR_BE_FIELD_SEL_BIT 0
#define HDR_BE_FIELD_SEL_VAL 0

#define STATS_CFG_OFF 0x00000AB0
#define STATS_CFG_MASK ((3 << HDR_BE_FIELD_SEL_BIT) | \
                        (1 << HDR_STATS_SITE_SEL_BIT))
#define STATS_CFG_VAL ((HDR_BE_FIELD_SEL_VAL << HDR_BE_FIELD_SEL_BIT) | \
                       (HDR_STATS_SITE_SEL_VAL << HDR_STATS_SITE_SEL_BIT))

/* AEC BG points to HDR BE for pipeline 48 -> this is used for AEC */
#define HW_STATS_TYPE MSM_ISP_STATS_AEC_BG

typedef union ISP_Stats_CfgCmdType {
  /* Configure STATS SITE SEL from HDR BE stats */
  struct
  {
    uint32_t     hdr_be_field_sel          :   2;
    uint32_t     /* reserved*/             :   1;
    uint32_t     hdr_stats_site_sel        :   1;
    uint32_t     /* reserved*/             :   28;
  }__attribute__((packed, aligned(4)));

  uint32_t mask;
}__attribute__((packed, aligned(4))) ISP_Stats_CfgType;

typedef struct ISP_Stats_hdr_be_CfgCmdType {
  uint32_t sat_stats_en         :             1;
  uint32_t shift_bits           :             3;
  uint32_t /* reserved */       :             4;
  uint32_t rgn_sample_pattern   :            16;
  uint32_t /* reserved */       :             8;
} __attribute__((packed, aligned(4))) ISP_Stats_hdr_be_CfgCmdType;

typedef struct ISP_StatsHdrBe_CfgCmdType {
  /*  STATS_HDR_BE_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   14;
  uint32_t       /*reserved */          :    2;
  /*  STATS_HDR_BE_RGN_NUM_CFG */
  uint32_t        rgnHNum               :    8;
  uint32_t        /* reserved */        :    8;
  uint32_t        rgnVNum               :    7;
  uint32_t        /* reserved */        :    9;
  /*  STATS_HDR_BE_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :    9;
  uint32_t        /* reserved */        :    7;
  uint32_t        rgnHeight             :    9;
  uint32_t        /* reserved */        :    7;
  /* STATS_HDR_BE_HI_THRESHOLD_CFG_0 */
  uint32_t        rMax                  :    14;
  uint32_t        /* reserved */        :    2;
  uint32_t        grMax                 :    14;
  uint32_t        /* reserved */        :    2;
  /* STATS_HDR_BE_HI_THRESHOLD_CFG_1 */
  uint32_t        bMax                  :    14;
  uint32_t        /* reserved */        :    2;
  uint32_t        gbMax                 :    14;
  uint32_t        /* reserved */        :    2;
  /* STATS_HDR_BE_LO_THRESHOLD_CFG_0 */
  uint32_t        rMin                  :    14;
  uint32_t        /* reserved */        :    2;
  uint32_t        grMin                 :    14;
  uint32_t        /* reserved */        :    2;
  /* STATS_HDR_BE_LO_THRESHOLD_CFG_1 */
  uint32_t        bMin                  :    14;
  uint32_t        /* reserved */        :    2;
  uint32_t        gbMin                 :    14;
  uint32_t        /* reserved */        :    2;
}__attribute__((packed, aligned(4))) ISP_StatsHdrBe_CfgCmdType;

#endif /* __HDR_BE_STATS_REG_H__ */
