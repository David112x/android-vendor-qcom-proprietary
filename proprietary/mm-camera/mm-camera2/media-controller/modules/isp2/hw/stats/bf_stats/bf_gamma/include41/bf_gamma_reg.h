/* bf_gamma_reg.h
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __BF_GAMMA_REG_H__
#define __BF_GAMMA_REG_H__

#include <stdio.h>

#define BF_GAMMA_ENTRIES 32

#define BAF_GAMMA_DMI_DELTA_SHIFT 14
#define BAF_GAMMA_STATS_CFG_ADDR  0xAE4
#define BF_GAMMA_DOWNSCALE_FACTOR 2
#define BF_GAMMA_USE_Y_CHANNEL TRUE
#define BAF_GAMMA_MAX_VAL 16383 // (1 << 14) - 1
#define BAF_GAMMA_DELTA_MIN_VAL -8192
#define BAF_GAMMA_DELTA_MAX_VAL 8191

typedef struct _bf_gamma_stats_enable_t {

  /* STATS_BAF_CFG */
  uint32_t /* reserved */       : 1;
  uint32_t gamma_lut_en         : 1;
  uint32_t gamma_lut_bank_sel   : 1;
  uint32_t /* reserved */       : 29;

} __attribute__((packed, aligned(4))) bf_gamma_stats_enable_t;

#endif /* __BF_GAMMA_REG_H__ */
