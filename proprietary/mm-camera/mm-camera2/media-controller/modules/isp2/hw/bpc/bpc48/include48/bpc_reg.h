/* bpc_reg.h
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __BPC_REG_H__
#define __BPC_REG_H__

//#define ABCC_LUT_COUNT 512

#define ISP_BPC_CFG0_OFF               0x000005C4
#define ISP_BPC_CFG1_OFF               0x000005C8
#define ISP_BPC_CFG2_OFF               0x000005CC
#define ISP_BPC_CFG3_OFF               0x000005D0
#define ISP_BPC_CFG4_OFF               0x000005D4
#define ISP_BPC_CFG5_OFF               0x000005D8
#define ISP_BPC_CFG6_OFF               0x000005DC
#define ISP_BPC_DPC_OFF                0x000005E0
#define ISP_BPC_IP_EN                  0x00000001
#define ISP_BPC_TOTAL_NUM_REG          8
#define NUM_BLKLEVEL_BITS              12
#define SQRT_SENSOR_GAIN_BITS          7

/* BPC_CFG_0 = 0x000005C4 */
typedef struct ISP_BPCCfg0 {
  uint32_t HotPixCor_Disable           :1;
  uint32_t ColdPixCor_Disable          :1;
  uint32_t SameChRecover               :1;
  uint32_t BlkLevel                    :12;
  uint32_t /*reserved*/                :17;
} __attribute__((packed, aligned(4))) ISP_BPCCfg0;

/* BPC_CFG_1 = 0x000005C8 */
typedef struct ISP_BPCCfg1 {
  uint32_t RG_WB_GainRatio             :17;
  uint32_t /*reserved*/                :15;
} __attribute__((packed, aligned(4))) ISP_BPCCfg1;

/* BPC_CFG_2 = 0x000005CC */
typedef struct ISP_BPCCfg2 {
  uint32_t BG_WB_GainRatio             :17;
  uint32_t /*reserved*/                :15;
} __attribute__((packed, aligned(4))) ISP_BPCCfg2;

/* BPC_CFG_3 = 0x000005D0 */
typedef struct ISP_BPCCfg3 {
  uint32_t GR_WB_GainRatio             :17;
  uint32_t /*reserved*/                :15;
} __attribute__((packed, aligned(4))) ISP_BPCCfg3;

/* BPC_CFG_4 = 0x000005D4 */
typedef struct ISP_BPCCfg4 {
  uint32_t GB_WB_GainRatio             :17;
  uint32_t /*reserved*/                :15;
} __attribute__((packed, aligned(4))) ISP_BPCCfg4;

/* BPC_CFG_5 = 0x000005D8 */
typedef struct ISP_BPCCfg5 {
  uint32_t BPC_Offset                  :15;
  uint32_t /*reserved*/                :1;
  uint32_t BCC_Offset                  :15;
  uint32_t /*reserved*/                :1;
} __attribute__((packed, aligned(4))) ISP_BPCCfg5;

/* BPC_CFG_6 = 0x000005DC */
typedef struct ISP_BPCCfg6 {
  uint32_t Fmax                        :7;
  uint32_t /*reserved*/                :1;
  uint32_t Fmin                        :7;
  uint32_t /*reserved*/                :1;
  uint32_t CorrectThreshold            :13;
  uint32_t /*reserved*/                :3;
} __attribute__((packed, aligned(4))) ISP_BPCCfg6;

/* BPC_DSP_STATS = 0x000005E0 */
typedef struct ISP_BPC_DPCstats {
  uint32_t NumOfHotPixels              :16;
  uint32_t NumOfColdPixels             :16;
} __attribute__((packed, aligned(4))) ISP_BPC_DPCstats;

typedef struct ISP_BPCRegConfig {
  ISP_BPCCfg0              bpc_cfg0;
  ISP_BPCCfg1              bpc_cfg1;
  ISP_BPCCfg2              bpc_cfg2;
  ISP_BPCCfg3              bpc_cfg3;
  ISP_BPCCfg4              bpc_cfg4;
  ISP_BPCCfg5              bpc_cfg5;
  ISP_BPCCfg6              bpc_cfg6;
} __attribute__((packed, aligned(4))) ISP_BPCRegConfig;

typedef struct {
  ISP_BPCRegConfig         bpc_regcfg;
  ISP_BPC_DPCstats         bpc_dpc_stats;
} __attribute__((packed, aligned(4))) ISP_BPC_Reg_t;

#endif /* __BPC_REG_H__ */



