/* gamma44.h
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __GAMMA44_H__
#define __GAMMA44_H__

/* isp headers */
#include "module_gamma.h"
#include "isp_sub_module_common.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_GAMMA, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_GAMMA, fmt, ##args)
//#define PRINT_REG_VAL_SET

#define TBL_INTERPOLATE_GAMMA(in1, in2, out, ratio, size, i) ({\
  for (i=0; i<size; i++) \
    out[i] = ((float)LINEAR_INTERPOLATION(in1[i], in2[i], ratio)); })
/* Macro defines*/
#define PRINT_TABLE(table) ({ \
  int i; \
  ISP_ERR("gamma44 table from %s", #table); \
  for (i=0; i<ISP_GAMMA_NUM_ENTRIES/16; i++) \
    ISP_ERR("%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", \
      table[i], table[i+1], table[i+2], table[i+3], table[i+4], \
      table[i+5], table[i+6], table[i+7], table[i+8], \
      table[i+9], table[i+10], table[i+11], table[i+12], \
      table[i+13], table[i+14], table[i+15]); })

boolean gamma44_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

void gamma44_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);

boolean gamma44_query_cap(mct_module_t *module,
  void *query_buf);

boolean gamma44_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma44_stats_asd_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma44_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma44_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma44_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma44_set_contrast(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma44_set_bestshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma44_set_spl_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gamma44_set_table(isp_sub_module_t *isp_sub_module,
  gamma44_t* gamma, gamma_table_type gamma_table_type);

boolean gamma44_streamon(isp_sub_module_t *isp_sub_module, void *data);

boolean gamma44_get_interpolated_table(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
boolean gamma44_init_config(isp_sub_module_t *isp_sub_module,
  gamma44_t *gamma);

boolean gamma44_store_hw_update(isp_sub_module_t *isp_sub_module,
  gamma44_t *gamma);

boolean gamma44_compute_contrast_table(double sigmoid, int lut_size,
  gamma_rgb_ds_t  *input_table_rgb, gamma_rgb_ds_t  *output_table_rgb,
  boolean enable_same_data_channel);

void gamma44_update_vfe_table(gamma44_t *gamma,
  volatile ISP_GammaConfigCmdType *gamma_config_cmd,
  gamma_rgb_ds_t *table_rgb);
void gamma44_copy_gamma(gamma44_t *gamma, gamma_rgb_ds_t *dst,
  gamma_rgb_ds_t *src, size_t size);
void gamma44_chromatix_to_isp_gamma(gamma44_t *gamma, gamma_rgb_t *gamma_rgb,
  chromatix_gamma_table_type *chromatix_gamma_table);
void gamma44_interpolate(gamma44_t *gamma, gamma_rgb_ds_t  *tbl1,
    gamma_rgb_ds_t  *tbl2, gamma_rgb_ds_t  *gamma_rgb,
  float ratio);
boolean gamma44_dmi_hw_update(void *data1,
  ISP_GammaLutSelect bank_sel, void *data2);
void gamma44_get_downsampled_tables(gamma44_t *gamma, gamma_rgb_ds_t *rgb_tbl,
  uint16_t *tbl_in, gamma_rgb_t *rgb_in, int skip_ratio);
void gamma44_update_tone_map_curves(void * data1,
  void * data2, int val);

int16_t gamma44_apply_contrast_sigmoid(double sigmoid, double multiplier,
  double x, double mid_pt);

int32_t gamma44_get_last_gamma_value(float *table);

int32_t gamma44_get_hi_lo_gamma_bits(float *table, int i);

void gamma44_get_downsampled_table_uint16(float *ispTable,
  uint16_t *table, int skipRatio);

void gamma44_get_downsampled_table_float(float *ispTable,
  float *table, int skipRatio);

  void gamma44_fill_ch_func_table(gamma44_t *gamma, int use_single_G_ch,
  int8_t manual_gamma_update);

void gamma44_set_solarize_table(gamma44_t *gamma,
  chromatix_parms_type *chromatix_ptr);

boolean gamma44_set_same_channel_data_flag(
  isp_sub_module_t *isp_sub_module);

void gamma44_send_meta_data(
  isp_meta_entry_t *gamma_dmi_info,
  gamma44_t *gamma);

#endif /* __GAMMA44_H__ */
