/*
* Copyright (c) 2014, 2016-2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef __GTM46_H__
#define __GTM46_H__

/* mctl headers */
#include "chromatix.h"

/* isp headers */
#include "isp_sub_module_common.h"
#include "isp_defs.h"
#include "gtm_reg.h"

//#define ZZHDR_CHROMATIX_EXTN
#ifdef ZZHDR_CHROMATIX_EXTN
#include "zzHDR_chromatix.h"
#endif

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_GTM, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_GTM, fmt, ##args)

typedef boolean (*gtm_trigger_params)(void *,
  void *, void *);

typedef struct {
  gtm_trigger_params  gtm_trigger_params;
} ext_override_gtm_hdr_func;

typedef boolean (*gtm_process_adj_regions)(void *gtm_v2,
  void *gtm46, uint32_t *, uint32_t *, float *);
typedef boolean (*gtm_apply_hdr_effects)(void *,
  void *, void *);
typedef boolean (*gtm_apply_hdr_dmi)(void *,
  void *);

typedef struct {
  gtm_process_adj_regions  gtm_process_adj_regions;
  gtm_apply_hdr_effects    gtm_apply_hdr_effects;
  gtm_apply_hdr_dmi        gtm_apply_hdr_dmi;

} ext_override_gtm_func;

typedef struct {
  float    GTM_key_max_th;
  float    GTM_max_percentile;
  float    GTM_min_percentile;
  float    GTM_key_hist_bin_weight;
} gtm_data_t;

/** gtm46_t:
 *
 *  @ISP_GTMCfgCmd: GTM register setting
 *  @is_aec_update_valid: indicate whether aec_update is valid
 *  @aec_update: stored aec update from stats update
 *  @algo_ran_once: flag indicating whether algo
 *                  has sent event to module at least once
 *                  indicating that the first LUT has been
 *                  calculated (key
 **/
typedef struct  {
  ISP_GTM_Cfg            reg_cfg;
  uint8_t                is_aec_update_valid;
  aec_update_t           aec_update;
  float                  hdr_sensitivity_ratio;
  isp_saved_gtm_params_t algo_output;
  uint64_t               dmi_tbl[GTM_LUT_NUM_BIN-1];
  uint32_t               base[GTM_LUT_NUM_BIN-1];
  int32_t                slope[GTM_LUT_NUM_BIN-1];
  boolean                enable_adrc;
  boolean                algo_update_valid;
  boolean                use_gtm_fix_curve;
  cam_sensor_hdr_type_t  sensor_hdr_mode;
  ext_override_gtm_hdr_func  *ext_func_hdr_table;
  ext_override_gtm_func  *ext_func_table;
  boolean                mod_enable;
  gtm_data_t             gtm_data;
  int                    gtm_manual_curve_enable;
  float                  yratio_base_manual[GTM_LUT_NUM_BIN];
#ifdef ZZHDR_CHROMATIX_EXTN
  chromatix_hdr_ext_type  *zzhdr_chromatix_extn;
#endif
} gtm46_t;

typedef struct {
  chromatix_GTM_core      core_params;
  chromatix_GTM_reserve   reserve_params;
} gtm46_algo_params_t;

boolean gtm46_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gtm46_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gtm46_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gtm46_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gtm46_algo_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gtm46_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  void *data);

boolean gtm46_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void gtm46_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean gtm46_v2_process_adj_regions(void *gtm_v2,
  void *gtm, uint32_t *trigger_idx_start, uint32_t *trigger_idx_end,
  float *ratio);

boolean gtm46_apply_hdr_effects(void *data1,
 void *data2, void *data3);

boolean gtm46_2d_apply_dmi(void *data1,
 void *data2);

boolean gtm46_fetch_gtm_data(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gtm46_calculate_default_cuve(gtm46_t *gtm,
      isp_gtm_algo_params_t   *algo_param);

boolean gtm46_set_hdr_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gtm46_configure_from_chromatix_bit(
  isp_sub_module_t *isp_sub_module);

boolean gtm46_fill_dmi_table(
  mct_module_t *module,
  gtm46_t               *gtm,
  isp_gtm_algo_params_t *algo_param,
  isp_sub_module_t      *isp_sub_module,
  chromatix_parms_type  *chromatix_ptr,
  mct_event_t *event
  );

void gtm46_pack_dmi_fix_curve(uint64_t *dmi_tbl,
  const uint32_t *base, const int32_t *slope);

void gtm46_pack_dmi(double *yout_tbl, uint32_t *xin_tbl,
  uint64_t *dmi_tbl, uint32_t *base, int32_t *slope);

void gtm46_derive_and_pack_lut_adrc_based(gtm46_t *gtm);

#ifdef ZZHDR_CHROMATIX_EXTN
boolean gtm46_fill_func_table_hdr_ext(gtm46_t *gtm);
#define FILL_FUNC_TABLE__HDR_GTM(field) gtm46_fill_func_table_hdr_ext(field)
#else
boolean gtm46_fill_func_table_hdr(gtm46_t *gtm);
#define FILL_FUNC_TABLE__HDR_GTM(field) gtm46_fill_func_table_hdr(field)
#endif

#ifdef OVERRIDE_FUNC
boolean gtm46_fill_func_table_ext(gtm46_t *gtm);
#define FILL_FUNC_TABLE_GTM(field) gtm46_fill_func_table_ext(field)
#else
boolean gtm46_fill_func_table(gtm46_t *gtm);
#define FILL_FUNC_TABLE_GTM(field) gtm46_fill_func_table(field)
#endif

#endif //__GTM46_H__
