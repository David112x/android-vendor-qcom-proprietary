/* abf48.h
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __DEMOSAIC_ABF48_H__
#define __DEMOSAIC_ABF48_H__

#include "chromatix.h"

#include "abf48_reg.h"
#include "isp_sub_module_common.h"

#define NORMAL_LIGHT_INDEX 4

#define CLAMP_ABF(x1,v1) ((x1 >= v1) ? v1 : x1 )
#ifndef Clamp
#define Clamp(x, t1, t2) (((x) < (t1))? (t1): ((x) > (t2))? (t2): (x))
#endif

#define ABF34_RADIAL_POINTS 5
#define ABF34_BPC_BX_MIN    -3120
#define ABF34_BPC_BX_MAX    3110
#define ABF34_BPC_BY_MIN    -8192
#define ABF34_BPC_BY_MAX    -5
#define ABF34_RSQRE_INIT_MIN    25
#define ABF34_RSQRE_INIT_MAX    76843264
#define ABF34_RSQRE_SHFT_MIN    0
#define ABF34_RSQRE_SHFT_MAX    15
#define ABF34_BASE_MIN  0
#define ABF34_BASE_MAX  255
#define ABF34_SHIFT_MIN 0
#define ABF34_SHIFT_MAX 15
#define ABF34_SLOPE_MIN 0
#define ABF34_SLOPE_MAX 255
#define ABF34_ANCHOR_MIN    0
#define ABF34_ANCHOR_MAX    4095
#define ABF34_FILTERNOISEPRESERVE_MAX 256
//#define ABFV34_NOISE_STD_LENGTH 65

#define ABF_SBPC_FMAX_MIN       0
#define ABF_SBPC_FMAX_MAX       63
#define ABF_SBPC_FMIN_MIN       0
#define ABF_SBPC_FMIN_MAX       63
#define ABF_SBPC_MAXSHFT_MIN    0
#define ABF_SBPC_MAXSHFT_MAX    14
#define ABF_SBPC_MINSHFT_MIN    0
#define ABF_SBPC_MINSHFT_MAX    14
#define ABF_SBPC_BPC_OFF_MIN  0
#define ABF_SBPC_BPC_OFF_MAX  4095
#define ABF34_CURVEOFFSET_MIN  0
#define ABF34_CURVEOFFSET_MAX  64
#define DISTANCE_KER_IDX_0_MIN  1
#define DISTANCE_KER_IDX_1_MIN  0
#define DISTANCE_KER_IDX_2_MIN  0
#define DISTANCE_KER_IDX_0_MAX  4
#define DISTANCE_KER_IDX_1_MAX  2
#define DISTANCE_KER_IDX_2_MAX  1
#define ABF3_BLKPIX_MIN 0
#define ABF3_BLKPIX_MAX 4
#define ABF34_BPC_BLS_MIN   0
#define ABF34_BPC_BLS_MAX   4095
#define ABF34_NOISESTDLUT_BASE_MIN 0
#define ABF34_NOISESTDLUT_BASE_MAX ((1<<9)-1)
#define ABF34_NOISESTDLUT_DELTA_MIN 0
#define ABF34_NOISESTDLUT_DELTA_MAX ((1<<9)-1)


//typedef uint32_t noise_std2_lut_t[ABFV34_NOISE_STD_LENGTH];

typedef struct {
  noise_std2_lut_t  noise_std_lut_level;
} abf34_parms_t;

typedef boolean (*get_trigger_index)(void *, void *);
typedef boolean (*get_2d_interpolation)(void *, void *);
typedef void (*apply_2d_interpolation)(void *);
typedef boolean (*denoise_adj)(isp_sub_module_t *isp_sub_module, void *);

typedef struct {
  get_trigger_index  get_trigger_index;
  get_2d_interpolation get_2d_interpolation;
  apply_2d_interpolation apply_2d_interpolation;
  denoise_adj denoise_adj;
} ext_override_func;

typedef struct {
  /* Module Params*/
  float                    abf_noise_profile_adjust_ratio;
  float                    abf_noise_offset;
  float                    abf_bpc_offset_adj;
  float                    abf_Fmax_adj;
  float                    abf_Fmin_adj;
  float                    bpc_maxshft;
  float                    bpc_minshft;
  float                    abf_HDR_thr_1;
  float                    abf_HDR_thr_2;
} abf48_ext_t;

typedef struct {
  /* Module Params*/
  ISP_ABF34_CmdType        v34RegCmd;
  trigger_ratio_t          aec_ratio;
  uint8_t                  trigger_index;
  abf34_parms_t            abf34_parms;
  sensor_out_info_t        sensor_out_info;
  float                    anchor_table[ABF34_RADIAL_POINTS];
  float                    filter_strength_lo[2];
  float                    filter_strength_hi[2];
  uint32_t                 filter_strength_anchor_pt_lo;
  uint32_t                 filter_strength_anchor_pt_hi;
  int32_t                  base_table[2][ABF34_RADIAL_POINTS];
  ispif_out_info_t         ispif_out_info;
  isp_out_info_t           isp_out_info;
  float                    aec_reference;
  float                    hdr_trigger_ref;
  float                    hdr_trigger_ratio;
  uint8_t                  hdr_trigger_index;
  float                    edge_softness;
  float                    noise_std_lut_level[ABFV34_NOISE_STD_LENGTH];
  aec_update_t             aec_update;
  aec_manual_update_t      aec_manual_update;
  float                    multfactor;
  uint32_t                 BLSS_offset;
  ext_override_func        *ext_func_table;
  int                      abf34_enable;
  int                      hdr_enable;
  uint32_t                 filterEn;
  uint32_t                 singleBpcEn;
  uint16_t                 init_h_offset;
  uint16_t                 init_v_offset;
  float                    aec_lux_reference;
  float                    aec_gain_reference;
  float                    hdr_sensitivity_ratio;
  float                    hdr_exp_ratio;
  float                    bpc_offset;
  float                    bpc_fmax;
  float                    bpc_fmin;
  float                    bpc_maxshft;
  float                    bpc_minshft;
  float                    blk_pix_matching_g;
  float                    blk_pix_matching_rb;
  abf48_ext_t              abf48_ext_params;
  uint8_t                  apply_hdr_effects;
} abf48_t;

boolean abf48_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void abf48_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean abf48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abf48_decide_hysterisis(  isp_sub_module_t *isp_sub_module);

boolean abf48_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abf48_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abf48_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abf48_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abf48_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);

boolean abf48_decide_hyster_point_enable(void* data1, void* data2);

boolean compute_abf34_denoise_adj(isp_sub_module_t
   *isp_sub_module, void *data);

boolean abf48_calcualte_hdr_ratio(isp_sub_module_t
   *isp_sub_module, abf48_t *abf);
#if OVERRIDE_FUNC
boolean abf48_fill_func_table_ext(abf48_t *abf);
#define FILL_FUNC_TABLE(field) abf48_fill_func_table_ext(field)
#else
boolean abf48_fill_func_table(abf48_t *abf);
#define FILL_FUNC_TABLE(field) abf48_fill_func_table(field)
#endif

#endif //__DEMOSAIC_ABF48_H__
