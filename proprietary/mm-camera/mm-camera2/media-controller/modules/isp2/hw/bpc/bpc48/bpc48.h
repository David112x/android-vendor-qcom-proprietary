/* bpc48.h
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __BPC48_H__
#define __BPC48_H__

/* mctl headers */
#include "chromatix.h"

/* isp headers */
#include "bpc_reg.h"
#include "isp_sub_module_common.h"

#define DEFAULT_CONFIG_INDEX           0
#define BPC_MIN_FMAX                   64
#define BPC_MAX_FMAX                   127 // ((1<<7)-1) for 7uQ6
#define BPC_MIN_FMIN                   0
#define BPC_MAX_FMIN                   64 // ((1<<7)-1) for 7uQ6
#define BPC_MIN_OFFSET                 0
#define BPC_MAX_OFFSET                 32767 // ((1<<15)-1)
#define BPC_MIN_COR_THR                0
#define BPC_MAX_COR_THR                8191 // ((1<<13)-1)
#define BPC_MIN_BLKLVL                 0
#define BPC_MAX_BLKLVL                 4095 // ((1<<12)-1)
#define BPC_MIN_AWB_GAIN_RATIO         0
#define BPC_MAX_AWB_GAIN_RATIO         131071 // ((1<<17)-1) for 17uQ12


#define AWB_GAIN_EQUAL(g1, g2) ( \
  F_EQUAL(g1->r_gain, g2->r_gain) && \
  F_EQUAL(g1->g_gain, g2->g_gain) && \
  F_EQUAL(g1->b_gain, g2->b_gain))

#define AWB_GAIN_EQ_ZERO(g1) ( \
  F_EQUAL(g1->r_gain, 0.0) || \
  F_EQUAL(g1->g_gain, 0.0) || \
  F_EQUAL(g1->b_gain, 0.0))

typedef struct {
  float              Fmax;
  uint32_t           Fmin;
  float              correction_threshold;
  float              BCC_offset;
  float              BPC_offset;
  trigger_ratio_t    aec_ratio_bpc;
  uint32_t           BlkLvl_offset;
  int                SameChannelRecovery;
} bpc_params_t;

typedef boolean (*check_enable_idx)(void *, void *, void *);
typedef boolean (*get_2d_interpolation)(void *, void *);
typedef void (*apply_2d_interpolation)(void *);

typedef struct {
  check_enable_idx           check_enable_idx;
  get_2d_interpolation get_2d_interpolation;
  apply_2d_interpolation apply_2d_interpolation;
} ext_override_func;

typedef struct {
  /* Module Params*/
    float                  bpc_offset_adj;
    float                  bpc_Fmax_adj;
} bpc48_ext_t;

/** bpc48_t:
 *
 **/
typedef struct {
  ISP_BPC_Reg_t            Reg;
  trigger_ratio_t          aec_ratio_bpc;
  awb_gain_t               curr_awb_gain;
  float                    curr_sensor_gain;
  uint8_t                  trigger_index;
  bpc_params_t             bpc_params;
  aec_update_t             aec_update;
  float                    aec_lux_reference;
  float                    aec_gain_reference;
  float                    hdr_sensitivity_ratio;
  float                    hdr_exp_time_ratio;
  ext_override_func        *ext_func_table;
  float                    aec_reference;
  bpc48_ext_t              bpc48_ext_params;
  uint8_t                  apply_hdr_effects;
} bpc48_t;

boolean bpc48_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void bpc48_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean bpc48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean bpc48_stats_aec_update(isp_sub_module_t *isp_sub_module, void *data);

boolean bpc48_stats_awb_update(isp_sub_module_t *isp_sub_module, void *data);

boolean bpc48_decide_hysterisis(isp_sub_module_t *isp_sub_module, float aec_ref);

boolean bpc48_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data);

boolean bpc48_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean bpc48_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

#if OVERRIDE_FUNC
boolean bpc48_fill_func_table_ext(bpc48_t *bpc);
#define FILL_FUNC_TABLE(field) bpc48_fill_func_table_ext(field)
#else
boolean bpc48_fill_func_table(bpc48_t *bpc);
#define FILL_FUNC_TABLE(field) bpc48_fill_func_table(field)
#endif

#endif /* __BPC48_H__ */
