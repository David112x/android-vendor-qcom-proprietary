/* module_bf_scale_stats44.c
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BF_SCALE_44_REG_H__
#define __BF_SCALE_44_REG_H__

#define BF_SCALE_STATS_OFF_0 0x000009D8
#define BF_SCALE_STATS_LEN_0 13

#define BF_SCALE_STATS_OFF_1 0x00000AA0
#define BF_SCALE_STATS_LEN_1 4

#define ISP_STATS_BF_SCALE_BUF_NUM  4
#define ISP_STATS_BF_SCALE_BUF_SIZE  ((18 * 14 * 14) * 4)

#define ISP_STATS_BF_SCALE_MAX_H_CFG 4055
#define ISP_STATS_BF_SCALE_MAX_V_CFG 8191

typedef struct ISP_StatsBf_Scale_CfgCmdType {
  /* VFE_STATS_BF_SCALE_SCALE_CFG */
  uint32_t        hEnable               :    1;
  uint32_t        vEnable               :    1;
  uint32_t      /* reserved */           :   30;
  /* VFE_STATS_BF_SCALE_SCALE_H_IMAGE_SIZE_CFG */
  uint32_t        hIn                   :   12;
  uint32_t      /* reserved */           :    4;
  uint32_t        hOut                  :   12;
  uint32_t      /* reserved */           :    4;
  /* VFE_STATS_BF_SCALE_SCALE_H_PHASE_CFG */
  uint32_t        horizPhaseMult        :   19;
  uint32_t      /* reserved */           :    1;
  uint32_t        horizInterResolution  :    2;
  uint32_t      /* reserved */           :   10;
  /* VFE_STATS_BF_SCALE_SCALE_V_IMAGE_SIZE_CFG */
  uint32_t        vIn                   :   13;
  uint32_t      /* reserved */           :    3;
  uint32_t        vOut                  :   13;
  uint32_t      /* reserved */           :    3;
  /* VFE_STATS_BF_SCALE_SCALE_V_PHASE_CFG */
  uint32_t        vertPhaseMult         :   19;
  uint32_t      /* reserved */           :    1;
  uint32_t        vertInterResolution   :    2;
  uint32_t      /* reserved */           :   10;
  /*  VFE_STATS_BF_RGN_OFFSET_CFG   */
  uint32_t        rgnHOffset            :   13;
  uint32_t      /* reserved */           :    3;
  uint32_t        rgnVOffset            :   12;
  uint32_t       /*reserved */           :    4;
  /*  VFE_STATS_BF_RGN_SIZE_CFG */
  uint32_t        rgnWidth              :    9;
  uint32_t      /* reserved */           :    3;
  uint32_t        rgnHeight             :    9;
  uint32_t        rgnHNum               :    5;
  uint32_t      /* reserved */           :    2;
  uint32_t        rgnVNum               :    4;
  /* VFE_STATS_BF_FILTER_CFG_0 */
  uint32_t        r_fv_min              :   14;
  uint32_t      /* reserved 14:15 */      :    2;
  uint32_t        gr_fv_min             :   14;
  uint32_t      /* reserved 30:31 */      :    2;
  /* VFE_STATS_BF_FILTER_CFG_1 */
  uint32_t        b_fv_min              :   14;
  uint32_t      /* reserved 14:15 */      :    2;
  uint32_t        gb_fv_min             :   14;
  uint32_t      /* reserved 30:31 */      :    2;
  /* VFE_STATS_BF_FILTER_COEFF_0 */
  int32_t         b00                   :    5;
  int32_t         b01                   :    5;
  int32_t         b02                   :    5;
  int32_t         b03                   :    5;
  int32_t         b04                   :    5;
  uint32_t       /* reserved 25:31 */   :    7;
  /* VFE_STATS_BF_FILTER_COEFF_1 */
  int32_t         b10                   :    5;
  int32_t         b11                   :    5;
  int32_t         b12                   :    5;
  uint32_t       /* reserved */          :    1;
  int32_t         b13                   :    5;
  uint32_t       /* reserved */          :    3;
  int32_t         b14                   :    5;
  uint32_t       /* reserved 25:31 */   :    3;
  /* VFE_STATS_BF_FILTER_COEFF_2 */
  int32_t         b05                   :    5;
  int32_t         b06                   :    5;
  int32_t         b07                   :    5;
  uint32_t       /* reserved */          :    1;
  int32_t         b08                   :    5;
  int32_t         b09                   :    5;
  int32_t         b15                   :    5;
  uint32_t       /* reserved */          :    1;
  /* VFE_STATS_BF_FILTER_COEFF_3 */
  int32_t         b16                   :    5;
  int32_t         b17                   :    5;
  int32_t         b18                   :    5;
  uint32_t       /* reserved */          :    1;
  int32_t         b19                   :    5;
  int32_t         b20                   :    5;
  int32_t         b21                   :    5;
  uint32_t       /* reserved */         :    1;
}__attribute__((packed, aligned(4))) ISP_StatsBf_Scale_CfgCmdType;

typedef struct ISP_StatsBf_Scale_Scaler_CfgCmdType {
  /* VFE_STATS_BF_SCALE_SCALE_H_STRIPE_CFG */
  uint32_t        horizMNInit           :   12;
  uint32_t      /* reserved */           :    4;
  uint32_t        horizPhaseInit        :   16;
  /* VFE_STATS_BF_SCALE_SCALE_H_PAD_CFG */
  uint32_t        scaleYInWidth         :   12;
  uint32_t      /* reserved */           :    4;
  uint32_t        hSkipCount            :   12;
  uint32_t      /* reserved */          :    4;
  /* VFE_STATS_BF_SCALE_SCALE_V_STRIPE_CFG */
  uint32_t        vertMNInit            :   13;
  uint32_t      /* reserved */           :    3;
  uint32_t        vertPhaseInit         :   16;
  /* VFE_STATS_BF_SCALE_SCALE_V_PAD_CFG */
  uint32_t        scaleYInHeight        :   13;
  uint32_t      /* reserved */           :    3;
  uint32_t        vSkipCount            :   13;
  uint32_t      /* reserved */           :    3;
}__attribute__((packed, aligned(4))) ISP_StatsBf_Scale_Scaler_CfgCmdType;

typedef struct ISP_StatsBf_Scale_MixCfgCmdType {
  ISP_StatsBf_Scale_CfgCmdType          cfg1;
  ISP_StatsBf_Scale_Scaler_CfgCmdType   cfg2;
} ISP_StatsBf_Scale_MixCfgCmdType;

#endif /*__BF_SCALE_44_REG_H__*/
