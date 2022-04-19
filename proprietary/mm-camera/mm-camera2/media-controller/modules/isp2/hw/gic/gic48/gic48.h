/* gic48.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __gic48_H__
#define __gic48_H__

/* mctl headers */
#include "chromatix.h"

/* isp headers */
#include "isp_sub_module_common.h"
#include "isp_defs.h"
#include "gic_reg.h"

//#define GIC_NOISE_STD2_LUT_SIZE             64 //GIC2_NOISESTD_LENGTH
#define GIC_NOISE_STD_LUT_SIZE_CHROMATIX   (GIC_NOISE_STD2_LUT_SIZE+1)

#define GIC2_FILTER_STRENGTH_MAX            256
#define GIC2_THINLINE_OFF_MAX               16383 //((1<<14)-1)
#define GIC2_NOISE_STD_LUT_LEVEL0_MIN       0
#define GIC2_NOISE_STD_LUT_LEVEL0_MAX       16383
#define GIC2_NOISE_OFFSET_MAX_VAL_CHROMATIX 65535 //((1<<16)-1)
#define GIC2_NOISESTDLUT_DELTA_MIN (-(1<<(14-1)))
#define GIC2_NOISESTDLUT_DELTA_MAX ((1<<13)-1)

typedef boolean (*get_2d_interpolation)(void *, void *);
typedef void (*apply_2d_interpolation)(void *);

typedef struct {
  get_2d_interpolation get_2d_interpolation;
  apply_2d_interpolation apply_2d_interpolation;
} ext_override_func;

typedef struct {
  /* Module Params*/
  float                    gic_ds_adj;
  float                    gic_fs_adj;
  float                    gic_thin_line_noise_offset;
} gic48_ext_t;

/** gic48_t:
 *
 *  @ISP_GICCfgCmd: GIC register setting
 *  @old_streaming_mode: old streaming mode
 **/
typedef struct  {
  uint32_t noiseStdTable[GIC_NOISE_STD_LUT_SIZE_CHROMATIX];
  uint32_t packed_noiseStdTable[GIC_NOISE_STD2_LUT_SIZE];
  ISP_GIC_CfgCmdType      ISP_GICCfgCmd;
  float                   aec_reference;
  float                   aec_lux_reference;
  float                   aec_gain_reference;
  float                   hdr_sen_trigger_ref;
  float                   hdr_time_trigger_ref;
  float                   aec_ratio; 
  uint8_t                 trigger_index;
  ext_override_func      *ext_func_table;
  float                   noise_offset;
  float                   filter_str;
  float                   denoise_scale;
  gic48_ext_t             gic48_ext_params;
  float                   multifactor;
  uint8_t                 apply_hdr_effects;
} gic48_t;

boolean gic48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gic48_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gic48_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gic48_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gic48_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void gic48_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

#if OVERRIDE_FUNC
boolean gic48_fill_func_table_ext(gic48_t *gic);
#define FILL_FUNC_TABLE(field) gic48_fill_func_table_ext(field)
#else
boolean gic48_fill_func_table(gic48_t *gic);
#define FILL_FUNC_TABLE(field) gic48_fill_func_table(field)
#endif

#endif //__gic48_H__
