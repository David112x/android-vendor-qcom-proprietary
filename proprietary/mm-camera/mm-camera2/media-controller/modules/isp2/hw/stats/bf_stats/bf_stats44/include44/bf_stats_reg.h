/* bf_stats_reg.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BF_STATS_REG_H__
#define __BF_STATS_REG_H__


#define BF_STATS_OFF_0 0x000008A4
#define BF_STATS_LEN_0 6

#define BF_STATS_OFF_1 0x000009D0
#define BF_STATS_LEN_1 2

#define ISP_STATS_BF_BUF_NUM  4
#define ISP_STATS_BF_BUF_SIZE  ((18 * 14 * 14) * 4)

#define STATS_CFG_OFF 0x00000888
#define STATS_CFG_MASK 0
#define STATS_CFG_VAL 0

#define FV_MIN_BITWIDTH_HW 8

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
  uint32_t        rgnVOffset            :   12;
  uint32_t       /*reserved */          :    4;
  /*  VFE_STATS_BF_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :    9;
  uint32_t      /* reserved */          :    3;
  uint32_t        rgnHeight             :    9;
  uint32_t        rgnHNum               :    5;
  uint32_t      /* reserved */          :    2;
  uint32_t        rgnVNum               :    4;
  /* VFE_STATS_BF_FILTER_CFG_0 */
  uint32_t        r_fv_min              :    14;
  uint32_t      /* reserved 14:15 */    :    2;
  uint32_t        gr_fv_min             :    14;
  uint32_t      /* reserved 30:31 */    :    2;
  /* VFE_STATS_BF_FILTER_CFG_1 */
  uint32_t        b_fv_min              :    14;
  uint32_t      /* reserved 14:15 */    :    2;
  uint32_t        gb_fv_min             :    14;
  uint32_t      /* reserved 30:31 */    :    2;

  /* Kernel coeff map: A05 A06 A07 A00 A01 A02 A03 A04 A08 A09 A15
   * A16 A17 A18 A10 A11 A12 A13 A14 A19 A20 A21
   */

  /* VFE_STATS_BF_FILTER_COEFF_0 */
  int32_t         a03                   :    5; /* a00 */ /* diff kernel map */
  int32_t         a04                   :    5; /* a01 */ /* diff kernel map */
  int32_t         a05                   :    5; /* a02 */ /* diff kernel map */
  int32_t         a06                   :    5; /* a03 */ /* diff kernel map */
  int32_t         a07                   :    5; /* a04 */ /* diff kernel map */
  uint32_t       /* reserved 25:31 */   :    7;
  /* VFE_STATS_BF_FILTER_COEFF_1 */
  int32_t         a14                   :    5; /* a10 */ /* diff kernel map */
  int32_t         a15                   :    5; /* a11 */ /* diff kernel map */
  int32_t         a16                   :    5; /* a12 */ /* diff kernel map */
  uint32_t       /* reserved */         :    1;
  int32_t         a17                   :    5; /* a13 */ /* diff kernel map */
  uint32_t       /* reserved */         :    3;
  int32_t         a18                   :    5; /* a14 */ /* diff kernel map */
  uint32_t       /* reserved 25:31 */   :    3;
  /* VFE_STATS_BF_FILTER_COEFF_2 */
  int32_t         a00                   :    5; /* a05 */ /* diff kernel map */
  int32_t         a01                   :    5; /* a06 */ /* diff kernel map */
  int32_t         a02                   :    5; /* a07 */ /* diff kernel map */
  uint32_t       /* reserved */         :    1;
  int32_t         a08                   :    5; /* a08 */ /* diff kernel map */
  int32_t         a09                   :    5; /* a09 */ /* diff kernel map */
  int32_t         a10                   :    5; /* a15 */ /* diff kernel map */
  uint32_t       /* reserved */         :    1;
  /* VFE_STATS_BF_FILTER_COEFF_3 */
  int32_t         a11                   :    5; /* a16 */ /* diff kernel map */
  int32_t         a12                   :    5; /* a17 */ /* diff kernel map */
  int32_t         a13                   :    5; /* a18 */ /* diff kernel map */
  uint32_t       /* reserved */         :    1;
  int32_t         a19                   :    5; /* a19 */ /* diff kernel map */
  int32_t         a20                   :    5; /* a20 */ /* diff kernel map */
  int32_t         a21                   :    5; /* a21 */ /* diff kernel map */
  uint32_t       /* reserved */         :    1;
}__attribute__((packed, aligned(4))) ISP_StatsBf_CfgCmdType;

#endif /* __BF_STATS_REG_H__ */
