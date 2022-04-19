/* module_bf_scale_stats46.c
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BF_SCALE_46_REG_H__
#define __BF_SCALE_46_REG_H__

#define BF_SCALE_STATS_OFF 0x00000A14
#define BF_SCALE_STATS_LEN 21

#define ISP_STATS_BF_SCALE_BUF_NUM  4
#define ISP_STATS_BF_SCALE_BUF_SIZE  ((18 * 14 * 20) * 4)

#define BF_SCALE_FIELD_SEL_BIT 18
#define BF_SCALE_FIELD_SEL     0  /* 0: All lines */
                                  /* 1: F0 */
                                  /* 2: F1 */
#define STATS_CFG_OFF 0x000009B8
#define STATS_CFG_MASK ((3 << BF_SCALE_FIELD_SEL_BIT))
#define STATS_CFG_VAL ((BF_SCALE_FIELD_SEL << BF_SCALE_FIELD_SEL_BIT))

#define FV_MIN_BITWIDTH_HW 14

#define ISP_STATS_BF_SCALE_MAX_H_CFG 4055
#define ISP_STATS_BF_SCALE_MAX_V_CFG 8191

typedef struct ISP_StatsBf_Scale_CfgCmdType {
  /* VFE_STATS_BF_SCALE_SCALE_CFG */
  uint32_t        hEnable                :    1;
  uint32_t        vEnable                :    1;
  uint32_t      /* reserved */           :   30;
  /* VFE_STATS_BF_SCALE_SCALE_H_IMAGE_SIZE_CFG */
  uint32_t        hIn                    :   13;
  uint32_t      /* reserved */           :    3;
  uint32_t        hOut                   :   13;
  uint32_t      /* reserved */           :    3;
  /* VFE_STATS_BF_SCALE_SCALE_H_PHASE_CFG */
  uint32_t        horizPhaseMult         :   21;
  uint32_t      /* reserved */           :    7;
  uint32_t        horizInterResolution   :    2;
  uint32_t      /* reserved */           :    2;
  /* VFE_STATS_BF_SCALE_SCALE_H_STRIPE_CFG_0 */
  uint32_t        horizMNInit            :   13;
  uint32_t      /* reserved */           :   19;
  /* VFE_STATS_BF_SCALE_SCALE_H_STRIPE_CFG_1 */
  uint32_t        horizPhaseInit         :   21;
  uint32_t      /* reserved */           :   11;
  /* VFE_STATS_BF_SCALE_SCALE_H_PAD_CFG */
  uint32_t        scaleYInWidth          :   13;
  uint32_t      /* reserved */           :    3;
  uint32_t        hSkipCount             :   13;
  uint32_t      /* reserved */           :    3;
  /* VFE_STATS_BF_SCALE_SCALE_V_IMAGE_SIZE_CFG */
  uint32_t        vIn                    :   13;
  uint32_t      /* reserved */           :    3;
  uint32_t        vOut                   :   13;
  uint32_t      /* reserved */           :    3;
  /* VFE_STATS_BF_SCALE_SCALE_V_PHASE_CFG */
  uint32_t        vertPhaseMult          :   21;
  uint32_t      /* reserved */           :    7;
  uint32_t        vertInterResolution    :    2;
  uint32_t      /* reserved */           :    2;
  /* VFE_STATS_BF_SCALE_SCALE_V_STRIPE_CFG_0 */
  uint32_t        vertMNInit             :   13;
  uint32_t      /* reserved */           :   19;
  /* VFE_STATS_BF_SCALE_SCALE_V_STRIPE_CFG_1 */
  uint32_t        vertPhaseInit          :   21;
  uint32_t      /* reserved */           :   11;
  /* VFE_STATS_BF_SCALE_SCALE_V_PAD_CFG */
  uint32_t        scaleYInHeight         :   13;
  uint32_t      /* reserved */           :    3;
  uint32_t        vSkipCount             :   13;
  uint32_t      /* reserved */           :    3;
  /*  VFE_STATS_BF_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset             :   13;
  uint32_t      /* reserved */           :    3;
  uint32_t        rgnVOffset             :   14;
  uint32_t       /*reserved */           :    2;
  /*  VFE_STATS_BF_RGN_SIZE_CFG */
  uint32_t        rgnWidth               :   10;
  uint32_t      /* reserved */           :    6;
  uint32_t        rgnHeight              :   11;
  uint32_t      /* reserved */           :    5;
/* VFE_STATS_BF_FILTER_CFG_0 */
  uint32_t        r_fv_min               :   23;
  uint32_t      /* reserved  */          :    9;
/* VFE_STATS_BF_FILTER_CFG_1 */
  uint32_t        gr_fv_min              :   23;
  uint32_t      /* reserved  */          :    9;
  /* VFE_STATS_BF_FILTER_CFG_2 */
  uint32_t        b_fv_min               :   23;
  uint32_t      /* reserved  */          :    9;
  /* VFE_STATS_BF_FILTER_CFG_3 */
  uint32_t        gb_fv_min              :   23;
  uint32_t      /* reserved  */          :    9;
  /* VFE_STATS_BF_FILTER_COEFF_0 */
  int32_t         b00                   :    5;
  int32_t         b01                   :    5;
  int32_t         b02                   :    5;
  int32_t         b03                   :    5;
  int32_t         b04                   :    5;
  uint32_t       /* reserved  */        :    1;
  int32_t         b05                   :    5;
  uint32_t       /* reserved  */        :    1;
  /* VFE_STATS_BF_FILTER_COEFF_1 */
  int32_t         b06                   :    5;
  int32_t         b07                   :    5;
  int32_t         b08                   :    5;
  uint32_t       /* reserved  */        :    1;
  int32_t         b09                   :    5;
  uint32_t       /* reserved  */        :    3;
  int32_t         b10                   :    5;
  uint32_t       /* reserved  */        :    3;
  /* VFE_STATS_BF_FILTER_COEFF_2 */
  int32_t         b11                   :    5;
  int32_t         b12                   :    5;
  int32_t         b13                   :    5;
  uint32_t       /* reserved */         :    1;
  int32_t         b14                   :    5;
  int32_t         b15                   :    5;
  int32_t         b16                   :    5;
  uint32_t       /* reserved */         :    1;
  /* VFE_STATS_BF_FILTER_COEFF_3 */
  int32_t         b17                   :    5;
  int32_t         b18                   :    5;
  int32_t         b19                   :    5;
  uint32_t       /* reserved */         :    1;
  int32_t         b20                   :    5;
  int32_t         b21                   :    5;
  uint32_t       /* reserved */         :    6;

  /*
   * rgnHNum and rgnVNum are not present in the
   * HW registers of BF_SCALE, they are shared
   * with the rgnHNum and rgnVNum specified by BF.
   * They are still put here because they need to be
   * stored internally for computations.
   * They will not be written to the HW.
   */
  uint32_t        rgnHNum               :    5;
  uint32_t        rgnVNum               :    4;
  uint32_t       /*reserved*/           :   21;
}__attribute__((packed, aligned(4))) ISP_StatsBf_Scale_CfgCmdType;

#endif /*__BF_SCALE_46_REG_H__*/
