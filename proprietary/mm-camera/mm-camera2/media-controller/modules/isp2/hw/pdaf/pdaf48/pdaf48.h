/*
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef __PDAF48_H__
#define __PDAF48_H__

/* std headers */
#include <stdio.h>
#include <unistd.h>
#include <math.h>

/* mctl headers */
#include "chromatix.h"
#include "chromatix_common.h"

/* isp headers */
#include "isp_defs.h"
#include "isp_common.h"
#include "isp_log.h"
#include "isp_sub_module_log.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module_util.h"
#include "isp_pipeline_reg.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

/* sensor headers */
#include "pdaf_lib.h"

/* pdaf headers */
#include "pdaf_reg.h"

#define PDAF48_VERSION "48"
#define PDAF48_MODULE_NAME(n) \
  "pdaf"n

/*#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_PDAF, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_PDAF, fmt, ##args)*/

#define DEFAULT_CONFIG_INDEX            0
#define PDAF_MIN_FMAX                   0
#define PDAF_MAX_FMAX                   255 // ((1<<8)-1)
#define PDAF_MIN_FMIN                   0
#define PDAF_MAX_FMIN                   255 // ((1<<8)-1)
#define PDAF_MIN_OFFSET                 0
#define PDAF_MAX_OFFSET                 32767 // ((1<<15)-1)
#define PDAF_MIN_AWB_GAIN_RATIO         0
#define PDAF_MAX_AWB_GAIN_RATIO         16384 // ((1<<14))
#define PDAF_MIN_HDR_EXP_RATIO          1024
#define PDAF_MAX_HDR_EXP_RATIO          16384 // ((1<<14))
#define PDAF_MIN_HDR_EXP_RATIO_RECIP    16
#define PDAF_MAX_HDR_EXP_RATIO_RECIP    256 // ((1<<8))
#define PDAF_MIN_LOC_OFFSET             0
#define PDAF_MAX_LOC_OFFSET             16383 // ((1<<14)-1)

/* This needs to be defined in isp_pipeline_48 header */
#define PDAF_LUT_RAM_BANK               0xB

#define AWB_GAIN_EQUAL(g1, g2) ( \
  F_EQUAL(g1->r_gain, g2->r_gain) && \
  F_EQUAL(g1->g_gain, g2->g_gain) && \
  F_EQUAL(g1->b_gain, g2->b_gain))

#define AWB_GAIN_EQ_ZERO(g1) ( \
  F_EQUAL(g1->r_gain, 0.0) || \
  F_EQUAL(g1->g_gain, 0.0) || \
  F_EQUAL(g1->b_gain, 0.0))

#define CLAMP(a,b,c) (((a) <= (b))? (b) : (((a) >= (c))? (c) : (a)))


typedef boolean (*get_2d_interpolation)(void *, void *);
typedef void (*apply_2d_interpolation)(void *);

typedef struct {
  get_2d_interpolation get_2d_interpolation;
  apply_2d_interpolation apply_2d_interpolation;
} ext_override_func;

typedef struct {
  int            lut[PDAF_LUT_COUNT];
}__attribute__((packed, aligned(4)))pdaf_packed_lut_t;

typedef struct {
  /* Module Params*/
    float                                   pdpc_bp_offset_G_adj;
    float                                   pdpc_bp_offset_rb_adj;
    float                                   pdpc_Fmax_adj;
} pdaf48_ext_t;

/** pdaf48_t:
 *
 *  @aec_update: aec update info
 *  @curr_real_gain: current real gain
 *  @curr_awb_gain: gain from awb update
 *  @sensor_out_info: sensor information
 **/
typedef struct {
  ISP_PDAF_Reg_t                   pdaf_reg;
  float                            curr_real_gain;
  awb_gain_t                       curr_awb_gain;
  sensor_out_info_t                sensor_out_info;
  uint8_t                          trigger_index;
  float                            aec_ratio;
  float                            hdr_exp_ratio;
  float                            hdr_sensitivity_ratio;
  float                            aec_lux_reference;
  float                            aec_gain_reference;
  pdaf_block_pattern_t             pdaf_pattern;
  unsigned int                     block_count_horizontal;
  unsigned int                     block_count_vertical;
  pdaf_packed_lut_t                packed_lut;
  int                              BLSS_offset;
  ext_override_func        *ext_func_table;
  uint8_t                          one_time_config_done;
  float      fmax;
  float      offset_g_pixel;
  float      offset_rb_pixel;
  float      offset_g_pixel_t2;
  float      offset_rb_pixel_t2;
  pdaf48_ext_t              pdaf48_ext_params;
  uint8_t                  apply_hdr_effects;
  isp_out_info_t             isp_out_info;
} pdaf48_t;

boolean pdaf48_init(isp_sub_module_t *isp_sub_module);

void pdaf48_destroy(isp_sub_module_t *isp_sub_module);

boolean pdaf48_streamon(isp_sub_module_t *isp_sub_module, void *data);

boolean pdaf48_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean pdaf48_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  void *data);

boolean pdaf48_stats_aec_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean pdaf48_stats_awb_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean pdaf48_set_pdaf_pixel(isp_sub_module_t *isp_sub_module,
  void *data);

boolean pdaf48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean pdaf48_update_offset(isp_sub_module_t *isp_sub_module,
  void *data);

boolean pdaf48_set_split_info(isp_sub_module_t *isp_sub_module,
  void *data);

#if OVERRIDE_FUNC
boolean pdaf48_fill_func_table_ext(pdaf48_t *bpc);
#define FILL_FUNC_TABLE(field) pdaf48_fill_func_table_ext(field)
#else
boolean pdaf48_fill_func_table(pdaf48_t *bpc);
#define FILL_FUNC_TABLE(field) pdaf48_fill_func_table(field)
#endif

#endif /* __PDAF48_H__ */
