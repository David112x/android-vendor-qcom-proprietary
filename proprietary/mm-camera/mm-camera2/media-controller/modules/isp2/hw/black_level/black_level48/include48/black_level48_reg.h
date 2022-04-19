/* black_level.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __BLACK_LEVEL_REG_H__
#define __BLACK_LEVEL_REG_H__

#define ISP_BLACK_LEVEL_CFG_OFF        0x000006B8
#define ISP_STATS_BLACK_LEVEL_CFG_OFF  0x00000AB4
#define ISP_BLACK_LEVEL_NUM_REG    2

/* BLACK_LEVEL_CFG : 0x000006B8*/
typedef struct ISP_Black_LevelCfg {
  uint32_t     offset                           : 12;
  uint32_t     /* reserved */                   : 4;
  uint32_t     scale                            : 12;
  uint32_t     /* reserved */                   : 4;
}__attribute__((packed, aligned(4))) ISP_Black_LevelCfg;

/* STATS_BLACK_LEVEL_CFG : 0x00000AB4 */
typedef struct ISP_Black_Level_StatsCfg {
  uint32_t     offset                           : 12;
  uint32_t     /* reserved */                   : 4;
  uint32_t     scale                            : 12;
  uint32_t     /* reserved */                   : 4;
}__attribute__((packed, aligned(4))) ISP_Black_Level_StatsCfg;

/* structure to contain all registers */
typedef struct ISP_black_level_reg_t {
  ISP_Black_LevelCfg           black_level_cfg;
  ISP_Black_Level_StatsCfg     black_level_stats_cfg;
}__attribute__((packed, aligned(4))) ISP_black_level_reg_t;

#endif /* __BLACK_LEVEL_REG_H__ */
