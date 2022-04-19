/* bpc47.h
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __BPC47_H__
#define __BPC47_H__

/* mctl headers */
#include "chromatix.h"

/* isp headers */
#include "bpc_reg.h"
#include "abcc_algo.h"
#include "isp_sub_module_common.h"

#define DEFAULT_CONFIG_INDEX           0
#define ABCC_FORCED_CORRECTION_ENABLE  0
#define ABCC_ENABLE                    0
#define BPC_MIN_FMAX                   64
#define BPC_MAX_FMAX                   127
#define BPC_MIN_FMIN                   0
#define BPC_MAX_FMIN                   64
#define BPC_MIN_OFFSET                 0
#define BPC_MAX_OFFSET                 16383
#define BPC_MIN_COR_THR                0
#define BPC_MAX_COR_THR                8191
#define BPC_MIN_HDT                    0
#define BPC_MAX_HDT                    1048575
#define BPC_MIN_HVT                    0
#define BPC_MAX_HVT                    16383
#define BPC_MIN_NMG                    0
#define BPC_MAX_NMG                    255
#define BPC_MIN_HDP1                    0
#define BPC_MAX_HDP1                    15
#define BPC_MIN_HDP                    -7
#define BPC_MAX_HDP                    7
#define BPC_MIN_HDP_SUM                0
#define BPC_MAX_HDP_SUM                29
#define BPC_MIN_BLKLVL                 0
#define BPC_MAX_BLKLVL                 1023
#define BPC_MIN_AWB_GAIN_RATIO         1025
#define BPC_MAX_AWB_GAIN_RATIO         16383
#define BPC_MIN_SENSOR_GAIN            0
#define BPC_MAX_SENSOR_GAIN            127

#define AWB_GAIN_EQUAL(g1, g2) ( \
  F_EQUAL(g1->r_gain, g2->r_gain) && \
  F_EQUAL(g1->g_gain, g2->g_gain) && \
  F_EQUAL(g1->b_gain, g2->b_gain))

#define AWB_GAIN_EQ_ZERO(g1) ( \
  F_EQUAL(g1->r_gain, 0.0) || \
  F_EQUAL(g1->g_gain, 0.0) || \
  F_EQUAL(g1->b_gain, 0.0))


typedef struct {
  uint64_t            lut[ABCC_LUT_COUNT];
}__attribute__((packed, aligned(8)))abcc_packed_lut_t;

typedef struct {
  abcc_packed_lut_t              packed_lut;
  uint8_t                        abcc_enable;
  uint8_t                        one_time_config_done;
  abcc_algo_t                    abcc_info;
  uint32_t                       input_width;
  uint32_t                       input_height;
  uint8_t                        hw_update_pending;
  sensor_dim_output_t            sensor_dim;
} abcc_t;

typedef struct {
  uint32_t           Fmax;
  uint32_t           Fmin;
  uint32_t           noise_model_gain;
  float              highlight_value_threshold;
  float              highlight_detection_threshold;
  float              correction_threshold;
  float              BCC_offset;
  float              BPC_offset;
  trigger_ratio_t    aec_ratio_bpc;
  uint32_t           BlkLvl_offset;
  int                SameChannelDetect;
  int                SameChannelRecovery;
} bpc_params_t;

typedef boolean (*check_enable_idx)(void *, void *, void *);

typedef struct {
  check_enable_idx           check_enable_idx;
} ext_override_func;

/** bpc47_t:
 *
 **/
typedef struct {
  ISP_BPC_Reg_t            Reg;
  trigger_ratio_t          aec_ratio_bpc;
  awb_gain_t               curr_awb_gain;
  float                    curr_sensor_gain;
  uint8_t                  trigger_index;
  bpc_params_t             bpc_params;
  abcc_t                   abcc_subblock;
  aec_update_t             aec_update;
  ext_override_func       *ext_func_table;
  float                    aec_reference;
} bpc47_t;

boolean bpc47_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void bpc47_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean bpc47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean bpc47_stats_aec_update(isp_sub_module_t *isp_sub_module, void *data);

boolean bpc47_stats_awb_update(isp_sub_module_t *isp_sub_module, void *data);

boolean bpc47_decide_hysterisis(isp_sub_module_t *isp_sub_module, float aec_ref);

boolean bpc47_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean bpc47_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data);

boolean bpc47_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean bpc47_set_defective_pixel_for_abcc(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean bpc47_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

#if OVERRIDE_FUNC
boolean bpc47_fill_func_table_ext(bpc47_t *bpc);
#define FILL_FUNC_TABLE(field) bpc47_fill_func_table_ext(field)
#else
boolean bpc47_fill_func_table(bpc47_t *bpc);
#define FILL_FUNC_TABLE(field) bpc47_fill_func_table(field)
#endif

#endif /* __BPC47_H__ */
