/* isp_sub_module_util.h
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ISP_SUBMODULE_UTIL_H__
#define __ISP_SUBMODULE_UTIL_H__

/* mctl headers*/
#include "mtype.h"
#include "media_controller.h"
#include "mct_port.h"
#include "mct_stream.h"

/* isp headers */
#include "isp_sub_module_common.h"

/* isp headers */
#include <media/msmb_isp.h>


boolean isp_sub_module_util_get_port_from_module(mct_module_t *module,
  mct_port_t **port, mct_port_direction_t direction, uint32_t identity);

boolean isp_sub_module_util_append_stream_info(
  isp_sub_module_t *session_param, mct_stream_info_t *stream_info);

boolean isp_sub_module_util_remove_stream_info(isp_sub_module_t *session_param,
  uint32_t identity);

boolean isp_sub_module_util_get_stream_info(isp_sub_module_t *session_param,
  uint32_t identity, mct_stream_info_t **stream_info);

void isp_sub_module_util_get_session_param(
  isp_sub_module_priv_t *isp_sub_module_priv, unsigned int session_id,
  isp_sub_module_t **session_param);

boolean isp_sub_module_util_create_session_param(
  isp_sub_module_priv_t *isp_sub_module_priv, unsigned int session_id,
  isp_sub_module_t **isp_sub_module);

boolean isp_sub_module_util_remove_session_param(
  isp_sub_module_priv_t *isp_sub_module_priv, unsigned int session_id);

boolean isp_sub_module_util_compare_identity(void *data1, void *data2);

boolean isp_sub_module_util_compare_identity_from_port(void *data1,
  void *data2);

boolean isp_sub_module_util_forward_event_from_port(mct_port_t *port,
  mct_event_t *event);

boolean isp_sub_module_util_forward_event_from_module(mct_module_t *module,
  mct_event_t *event);

boolean isp_sub_module_util_store_hw_update(
  isp_sub_module_t *isp_sub_module,
  struct msm_vfe_cfg_cmd_list *hw_update);

boolean isp_sub_module_util_append_hw_update_list(
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *output);

boolean isp_sub_module_util_update_streaming_mode(void *data1, void *data2);

boolean isp_sub_module_util_free_hw_update_list(
  isp_sub_module_t *isp_sub_module);

enum ISP_START_PIXEL_PATTERN isp_sub_module_fmt_to_pix_pattern(
  cam_format_t fmt);

boolean isp_sub_module_is_bayer_mono_fmt(cam_format_t fmt);

awb_cct_type isp_sub_module_util_get_awb_cct_type(cct_trigger_info* trigger,
  void *chromatix_ptr);

awb_cct_type isp_sub_module_util_get_awb_ext_h_cct_type(
  cct_trigger_info* trigger, void *chromatix_ptr);

awb_cct_type isp_sub_module_util_get_awb_ext_cct_type(
  cct_trigger_info* trigger, void *chromatix_ptr);

awb_cct_type isp_sub_module_util_get_awb_cct_with_H_type(
   cct_trigger_info* trigger);


boolean isp_sub_module_util_get_aec_ratio_bright_low(unsigned char tuning_type,
  void *outdoor_trigger_ptr, void *lowlight_trigger_ptr, aec_update_t* aec_out,
  trigger_ratio_t *rt, boolean use_sensor_gain);

float isp_sub_module_util_get_aec_ratio_lowlight(unsigned char tunning_type,
  void *trigger_ptr, aec_update_t* aec_out, boolean use_sensor_gain);

float isp_sub_module_util_calc_interpolation_weight(float value,
  float start, float end);

float isp_sub_module_util_get_aec_ratio_bright(unsigned char tunning_type,
  void *trigger_ptr, aec_update_t* aec_out);

uint32_t isp_sub_module_util_calculate_shift_bits(uint32_t pixels,
  uint32_t input_depth, uint32_t output_depth);
boolean isp_sub_module_util_write_dmi(void *tbl,
  uint32_t tbl_len, uint32_t channel,
  enum msm_vfe_reg_cfg_type dmi_write_type,
  uint32_t dmi_cfg_reg_offset, uint32_t dmi_addr_reg_offset,
  isp_sub_module_t *isp_sub_module);

double isp_sub_module_util_power(double base, double exponent);

void isp_sub_module_util_pack_cfg_cmd(struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd,
  uint32_t cmd_offset, uint32_t cmd_len, uint32_t cmd_type,
  uint32_t hw_reg_offset);

boolean isp_sub_module_util_reset_hist_dmi(isp_sub_module_t *isp_sub_module,
  uint32_t dmi_channel, uint32_t dmi_cfg_default, uint32_t dmi_cfg_off, uint32_t dmi_addr);

boolean isp_sub_module_util_hw_reg_set(uint32_t *data, uint32_t len);

boolean isp_sub_module_util_update_module_bit(isp_sub_module_t *isp_sub_module,
  void *data);

isp_hw_module_id_t isp_sub_module_util_convert_ez_to_isp_id(uint8_t ez_isp_id);

boolean isp_sub_module_traverse_ports(void *data, void *user_data);

boolean isp_sub_module_util_configure_from_chromatix_bit(
  isp_sub_module_t *isp_sub_module);

boolean isp_sub_mod_util_send_event(mct_module_t *module,
  mct_event_t *event);

boolean isp_sub_mod_get_port_from_module(mct_module_t *module,
  mct_port_t **port, mct_port_direction_t direction, uint32_t identity);

boolean isp_sub_mod_util_find_port_based_on_identity(void *data1,
  void *data2);

boolean isp_sub_mod_util_compare_identity(void *data1, void *data2);

boolean isp_sub_module_util_update_cgc_mask(isp_sub_module_t *isp_sub_module,
  uint32_t reg_offset, uint32_t cgc_bit, boolean enable);

boolean isp_sub_module_util_setloglevel(const char *name,
  isp_log_sub_modules_t mod_id);

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x306)
awb_cct_type isp_sub_module_util_get_awb_cct_with_H_type(
   cct_trigger_info* trigger);
#endif

void isp_sub_module_util_temporal_lux_calc(isp_temporal_luxfilter_params_t
  *lux_filter, float *lux_idx);

void isp_sub_module_getscalefactor(cam_format_t fmt, float *scale_w, float *scale_h);

boolean isp_sub_module_util_get_cfg_mask(isp_sub_module_t *isp_sub_module,
  mct_event_t *event);

void isp_sub_module_util_fill_sensor_roll_off_cfg(
  sensor_out_info_t  *sensor_out_info,
  sensor_rolloff_config_t   *sensor_rolloff_config);

boolean isp_sub_module_util_decide_hysterisis(
  isp_sub_module_t *isp_sub_module, float aec_ref, int mod_enable,
  int snapshot_hyst_en_flag, tuning_control_type control_type,
  trigger_point_type *hyster_trigger);
boolean isp_sub_module_util_is_adrc_mod_enable(float ratio,
  float total_drc_gain);

boolean isp_sub_module_util_is_adrc_gain_set(float col_drc_gain , float total_adrc_gain);

uint32_t isp_sub_module_util_get_lut_index(float drc_gain);

float isp_sub_module_util_linear_interpolate(float x,
  unsigned int x_left, unsigned int x_right, float val_left,
  float val_right);

float isp_sub_module_util_linear_interpolate_float(float x,
  float x_left, float x_right, float val_left,
  float val_right);

void isp_sub_module_util_gain_lut_with_drc_gain_weight( float *knee_gain_in,
  float *knee_index,unsigned short num_knee,float rate,float *knee_lut_out);

int isp_sub_module_util_find_region_index_spatial(float trigger,
  float *trigger_start,float *trigger_end,float *Interp_ratio,
  int *RegionIdxStrt,int *RegionIdxEnd,int total_regions);

uint32_t isp_sub_module_util_get_rgn_sample_pattern(
  isp_rgn_skip_pattern skip_pattern);

#endif
