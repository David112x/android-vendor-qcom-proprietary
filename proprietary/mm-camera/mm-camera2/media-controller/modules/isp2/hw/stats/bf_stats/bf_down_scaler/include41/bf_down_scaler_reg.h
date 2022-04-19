/* bf_down_scaler_reg.h
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BF_DOWN_SCALER_REG_REG_H__
#define __BF_DOWN_SCALER_REG_REG_H__

#define BF_DOWN_SCALER_ENABLE_ADDR 0xAE4
#define BF_DOWN_SCALER_CONFIG_ADDR 0xB94

#define ISP_STATS_BF_SCALE_MAX_H_CFG 16383
#define ISP_STATS_BF_SCALE_MAX_V_CFG 16383
#define PHASE_ADDER                  14

typedef struct _bf_down_scaler_reg_cfg_t {

  /* STATS_BAF_SCALE_H_IMAGE_SIZE_CFG */
  uint32_t h_in                 : 13;
  uint32_t /* reserved */       : 3;
  uint32_t h_out                : 13;
  uint32_t /* reserved */       : 3;

  /* STATS_BAF_SCALE_H_PHASE_CFG */
  uint32_t h_phase_mult         : 21;
  uint32_t /* reserved */       : 7;
  uint32_t h_interp_reso        : 2;
  uint32_t /* reserved */       : 3;

  /* STATS_BAF_SCALE_H_STRIPE_CFG_0 */
  uint32_t h_mn_init            : 13;
  uint32_t /* reserved */       : 19;

  /* STATS_BAF_SCALE_H_STRIPE_CFG_1 */
  uint32_t h_phase_init         : 21;
  uint32_t /* reserved */       : 11;

  /* STATS_BAF_SCALE_H_PAD_CFG */
  uint32_t scale_y_in_width     : 13;
  uint32_t /* reserved */       : 3;
  uint32_t h_skip_cnt           : 13;
  uint32_t /* reserved */       : 3;

}__attribute__((packed, aligned(4))) bf_down_scaler_reg_cfg_t;

typedef struct bf_down_scaler_enable_t {

  /* STATS_BAF_CFG */
  uint32_t /* reserved */       : 4;
  uint32_t scale_en             : 1;
  uint32_t /* reserved */       : 27;

} __attribute__((packed, aligned(4))) bf_down_scaler_enable_t;

#endif /*__BF_DOWN_SCALER_REG_REG_H__*/
