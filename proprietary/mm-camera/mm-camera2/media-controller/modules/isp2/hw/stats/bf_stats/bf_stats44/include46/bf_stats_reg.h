/* bf_stats_reg.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BF_STATS_REG_H__
#define __BF_STATS_REG_H__


#define BF_STATS_OFF_0 0x000009E8
#define BF_STATS_LEN_0 6

#define BF_STATS_OFF_1 0x00000A00
#define BF_STATS_LEN_1 5

#define ISP_STATS_BF_BUF_NUM  4
#define ISP_STATS_BF_BUF_SIZE  ((18 * 14 * 20) * 4)

#define BF_SITE_SEL_BIT 20
#define BF_FIELD_SEL_BIT 3
#define BF_SITE_SEL  2 /* 0: Before BPC */
                       /* 1: Before HDR fusion */
                       /* 2: Before Demosaic */
                       /* 3: After  Demosaic */
#define BF_FIELD_SEL 0 /* 0: All lines */
                       /* 1: F0 */
                       /* 2: F1 */

#define STATS_CFG_OFF 0x000009B8
#define STATS_CFG_MASK ((3 << BF_SITE_SEL_BIT) |\
                        (3 << BF_FIELD_SEL_BIT))
#define STATS_CFG_VAL ((BF_SITE_SEL << BF_SITE_SEL_BIT) |\
                       (BF_FIELD_SEL << BF_FIELD_SEL_BIT))

#define FV_MIN_BITWIDTH_HW 14

/** ISP_StatsBf_CfgCmdType:
 *
 *  @rgnHOffset: H offset
 *  @rgnVOffset: V offset
 *  @rgnWidth: region width
 *  @rgnHeight: region height
 *  @rgnHNum: H num
 *  @rgnVNum: V num
 *  @r_fv_min: min focus value for r
 *  @gr_fv_min: min focus value for gr
 *  @b_fv_min: min focus value for b
 *  @gb_fv_min: min focus value for gb
 *  @a00: a00 filter coefficient
 *  @a01: a01 filter coefficient
 *  @a02: a02 filter coefficient
 *  @a03: a03 filter coefficient
 *  @a04: a04 filter coefficient
 *  @a05: a05 filter coefficient
 *  @a06: a06 filter coefficient
 *  @a07: a07 filter coefficient
 *  @a08: a08 filter coefficient
 *  @a09: a09 filter coefficient
 *  @a10: a10 filter coefficient
 *  @a11: a11 filter coefficient
 *  @a12: a12 filter coefficient
 *  @a13: a13 filter coefficient
 *  @a14: a14 filter coefficient
 *  @a15: a15 filter coefficient
 *  @a16: a16 filter coefficient
 *  @a17: a17 filter coefficient
 *  @a18: a18 filter coefficient
 *  @a19: a19 filter coefficient
 *  @a20: a20 filter coefficient
 *  @a21: a21 filter coefficient
 **/
typedef struct ISP_StatsBf_CfgCmdType {
  /*  VFE_STATS_BF_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnVOffset            :   14;
  uint32_t       /*reserved */          :    2;
  /*  VFE_STATS_BF_RGN_NUM_CFG */
  uint32_t        rgnHNum               :    5;
  uint32_t      /* reserved */          :   11;
  uint32_t        rgnVNum               :    4;
  uint32_t      /* reserved */          :   12;
  /*  VFE_STATS_BF_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :   10;
  uint32_t      /* reserved */          :    6;
  uint32_t        rgnHeight             :   11;
  uint32_t      /* reserved */          :    5;
  /* VFE_STATS_BF_FILTER_CFG_0 */
  uint32_t        r_fv_min              :   23;
  uint32_t      /* reserved 23:31 */    :    9;
  /* VFE_STATS_BF_FILTER_CFG_1 */
  uint32_t        gr_fv_min             :   23;
  uint32_t      /* reserved 23:31 */    :    9;
  /* VFE_STATS_BF_FILTER_CFG_2 */
  uint32_t        b_fv_min              :   23;
  uint32_t      /* reserved 23:31 */    :    9;
  /* VFE_STATS_BF_FILTER_CFG_3 */
  uint32_t        gb_fv_min             :   23;
  uint32_t      /* reserved 23:31 */    :    9;
  /* VFE_STATS_BF_FILTER_COEFF_0 */
  int32_t         a00                   :    5;
  int32_t         a01                   :    5;
  int32_t         a02                   :    5;
  int32_t         a03                   :    5;
  int32_t         a04                   :    5;
  uint32_t       /* reserved 25 */      :    1;
  int32_t         a05                   :    5;
  uint32_t       /* reserved 31 */      :    1;
  /* VFE_STATS_BF_FILTER_COEFF_1 */
  int32_t         a06                   :    5;
  int32_t         a07                   :    5;
  int32_t         a08                   :    5;
  uint32_t       /* reserved */         :    1;
  int32_t         a09                   :    5;
  uint32_t       /* reserved 21:23 */   :    3;
  int32_t         a10                   :    5;
  uint32_t       /* reserved 29:31 */   :    3;
  /* VFE_STATS_BF_FILTER_COEFF_2 */
  int32_t         a11                   :    5;
  int32_t         a12                   :    5;
  int32_t         a13                   :    5;
  uint32_t       /* reserved */         :    1;
  int32_t         a14                   :    5;
  int32_t         a15                   :    5;
  int32_t         a16                   :    5;
  uint32_t       /* reserved */         :    1;
  /* VFE_STATS_BF_FILTER_COEFF_3 */
  int32_t         a17                   :    5;
  int32_t         a18                   :    5;
  int32_t         a19                   :    5;
  uint32_t       /* reserved */         :    1;
  int32_t         a20                   :    5;
  int32_t         a21                   :    5;
  uint32_t       /* reserved 26:31 */   :    6;
}__attribute__((packed, aligned(4))) ISP_StatsBf_CfgCmdType;

#endif /* __BF_STATS_REG_H__ */
