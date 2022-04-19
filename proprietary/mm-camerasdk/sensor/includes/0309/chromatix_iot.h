//======================================================================
// Copyright (c) 2016 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//======================================================================

#ifndef CHROMATIX_IOT_H
#define CHROMATIX_IOT_H

#include "chromatix.h"

#define SHDR_EXP_LUT_ENTRIES    20
#define SHDR_EXP_RATIO_LOWLIGHT_LUT_SIZE 10


typedef struct
{
    unsigned short chromatix_version;
    unsigned short IOT_header_version;
} IOT_version_type;

//=============================================================================
// Feature name: Exposure LUT  Parameters
//
// Variable name: exp_ratio
// exposure ratio
// default: 16.0f
//
// Variable name: gtm_gamma
// GPU-GTM gamma
// default: 0.65f
//
// Variable name: exp_change_up_th
// up threshold for changing exposure ratio
// default: 1.5f
//
// Variable name: exp_change_low_th
// low threshold for changing exposure ratio
// default: 1.0f
//
// Variable name: non_linear_low_th
// low threshold for non_linear of bhist
// default: 4
//
// Variable name: non_linear_hi_th
// high threshold for non_linear of bhist
// default: 220
//
// Variable name: reserved[5]
// reserved for potential use
// default: 0.0f
//=============================================================================
typedef struct
{
    float   exp_ratio;
    float   gtm_gamma;
    float   exp_change_up_th;
    float   exp_change_low_th;
    int     non_linear_low_th;
    int     non_linear_hi_th;
    float   reserved[5];
} aec_shdr_exp_lut_type;

//=============================================================================
// Feature name: Low light LUT
//
// Variable name: lux_index
//lux index entry
//
// Variable name: k
//sensitivity for exposure ratio calcualted as bright and dark luma
//
// Variable name: exp_ratio_up_limit
// high exposure ratio limit for lux level entry
//
// Variable name: exp_ratio_low_limit
//Low expsoure ratio limit for lux level entry
//
// Variable name: reserved[5]
// reserved for potential use
// default: 0.0f
//=============================================================================


typedef struct aec_svhdr_lowlight_s {
  int lux_index;
  float k;
  float exp_ratio_up_limit;
  float exp_ratio_low_limit;
  float reserved[5];
} aec_svhdr_lowlight_t;


//=============================================================================
// Feature name: shdr_aec_tuning_type
// AEC control params for SHDR
//
// Variable name: shdr_enable
//Enable/disable feature
// default: 1
//
// Variable name: num_lut_entries
// number of entries into exposure LUT
// default: 8
//
// Variable name: num_lowlight_lut_entries
// default: 6
//
// Variable name: pre_Bhist_inverse_map
// param to control inverse mapping of Bhist during exp ratio calculation
// default: 1
//
// Variable name: temporal_filter1
// default: 0.02f
//
// Variable name: temporal_filter2
// default: 1.0f
//
// Variable name: exp_lut
// default: 0.02f
//
// Variable name: aec_svhdr_lowlight
// default: 1.0f
//
// Variable name: reserved[100]
// reserved for potential use
// default: 0
//=============================================================================
typedef struct
{
    int          shdr_enable;
    int          num_lut_entries;
    int          num_lowlight_lut_entries;
    int          pre_Bhist_inverse_map;
    float        temporal_filter1;
    float        temporal_filter2;
    aec_shdr_exp_lut_type exp_lut[SHDR_EXP_LUT_ENTRIES];
    aec_svhdr_lowlight_t aec_svhdr_lowlight[SHDR_EXP_RATIO_LOWLIGHT_LUT_SIZE];
    float        reserved[100];
} shdr_aec_tuning_type;

//=============================================================================
// Feature name: Bayer proc fusion algo params
//  Normal light distribution params for IR mode
//
// Variable name: adc_bit_depth
//
//
// Variable name: tm_out_bit_depth
// Target_G threshold of detected normal light
//
// Variable name:bayer_gtm_gamma
// Maximum GB ratio to detect normal light
//
// Variable name: hdr_dark_n1
// Minimum GB ratio to detect normal light
//
// Variable name: hdr_dark_n2_minus_n1_normalization_factor
// Maximum GR ratio to detect normal light
//
// Variable name: hdr_max_weight
// Minimum GR ratio to detect normal light
//
// Variable name: tm_gain
// Maximum RB ratio to detect normal light
//
// Variable name: hdr_ratio
// Minimum RB ratio to detect normal light
//
// Variable name: perf_hint
// Minimum RB ratio to detect normal light
//
// Variable name: num_gpu_passes
// Minimum RB ratio to detect normal light
//
// Variable name: ir_distribution_weight[16][16]
// Weight for calculating normal mode distribution (16x16)
//=============================================================================


typedef struct {
  int adc_bit_depth;
  int tm_out_bit_depth;
  float bayer_gtm_gamma;
  int hdr_dark_n1;
  float hdr_dark_n2_minus_n1_normalization_factor;
  int hdr_max_weight;
  float tm_gain;
  float hdr_ratio;
  int perf_hint;
  int num_gpu_passes;
  float reserved[10];
} shdr_bayer_proc_tuning_type;

//=============================================================================
// Feature name: auto LTM feature
//
//
// Variable name: normal_light_cnt_thres
//
//
// Variable name: normal_light_target_thres
//
//
// Variable name: normal_light_GB_ratio_max
//
//
// Variable name: normal_light_GB_ratio_min
//
//
// Variable name: normal_light_GR_ratio_max
//
//
// Variable name: normal_light_GR_ratio_min
//
//
// Variable name: normal_light_RB_ratio_max
//
//
// Variable name: normal_light_RB_ratio_min
//
//
// Variable name: reserved[10]
// reserved for potential use
// default: 0
//=============================================================================


typedef struct {
  int auto_ltm_enable;
  int halo_score_target;
  int brightness_target;
  float contrast_target;
  float ltm_gain_limit;
  int low_code_tone_end;
  int mid_tone_start;
  int mid_tone_end;
  float smear_prev_low_limit;
  float smear_prev_high_limit;
  float reserved[10];
} shdr_ltm_tuning_type;


typedef struct {
  shdr_aec_tuning_type              shdr_aec_params;
  shdr_bayer_proc_tuning_type       shdr_bayer_proc_params;
  shdr_ltm_tuning_type              shdr_ltm_params;
  float                             reserved[200];
} chromatix_shdr_type;

//=============================================================================
// Feature name : chromatix_tnr_type
// Params to control TNR
//
// Variable name: tnr_intensity
// TBD
//
// Variable name:mot_det_sensitivity
// TBD
//=============================================================================


typedef struct {
  float tnr_intensity;
  float mot_det_sensitivity;
  float reserved[10];
} chromatix_tnr_type;


//=============================================================================
// Feature name : ir_led_adjust_type
// IR LED params for IR mode
//
// Variable name: ir_led_enable
// Enable IR LED params
//
// Variable name: ir_led_intensity_unit
// IR LED intensity unit
//
// Variable name: ir_led_intensity_idx_max
// Maximum IR LED Index
//
// Variable name: ir_led_intensity_idx_min
// Minimum IR LED Index
//=============================================================================
typedef struct
{
    int     ir_led_enable;
    float   ir_led_intensity_unit;
    int     ir_led_intensity_idx_max;
    int     ir_led_intensity_idx_min;
} ir_led_adjust_type;

//=============================================================================
// Feature name: ir_als_adjust_type
// Ambient light sensor params for IR mode
//
// Variable name: ir_lightsensor_enable
// Enable Ambient Light Sensor
//
// Variable name: day2ir_switch_pt
// Day to IR mode switch point
//
// Variable name: ir2day_switch_pt
// IR to Day mode switch point
//=============================================================================
typedef struct
{
    int     ir_lightsensor_enable;
    float   day2ir_switch_pt;
    float   ir2day_switch_pt;
} ir_als_adjust_type;

//=============================================================================
// Feature name: ir_daylight_distribution_adjust_type
//  Normal light distribution params for IR mode
//
// Variable name: normal_light_cnt_thres
// Count threshold of detected normal light
//
// Variable name: normal_light_target_thres
// Target_G threshold of detected normal light
//
// Variable name: normal_light_GB_ratio_max
// Maximum GB ratio to detect normal light
//
// Variable name: normal_light_GB_ratio_min
// Minimum GB ratio to detect normal light
//
// Variable name: normal_light_GR_ratio_max
// Maximum GR ratio to detect normal light
//
// Variable name: normal_light_GR_ratio_min
// Minimum GR ratio to detect normal light
//
// Variable name: normal_light_RB_ratio_max
// Maximum RB ratio to detect normal light
//
// Variable name: normal_light_RB_ratio_min
// Minimum RB ratio to detect normal light
//
// Variable name: ir_distribution_weight[16][16]
// Weight for calculating normal mode distribution (16x16)
//=============================================================================
typedef struct
{
    float   normal_light_cnt_thres;
    float   normal_light_target_thres;
    int     normal_light_GB_ratio_max;
    int     normal_light_GB_ratio_min;
    int     normal_light_GR_ratio_max;
    int     normal_light_GR_ratio_min;
    int     normal_light_RB_ratio_max;
    int     normal_light_RB_ratio_min;
    float   ir_distribution_weight[16][16];
} ir_daylight_distribution_adjust_type;

//=============================================================================
// Feature name: chromatix_ir_mode_type
// IR Mode enable
//
// Variable name: ir_mode_enable
// Enable or Disable IR Mode feature
//
// Variable name: ir_center_weight[16][16]
// Weight for calculating luma of IR mode  (16x16)
//
// Variable name: ir_highlight_suppress_weight[64]
// Weight for avoid object over-exposure (64)
//
// Variable name: ir_mode_conv_speed
// AEC converge speed for IR mode
//
// Variable name: day2ir_exp_index
//  Day to IR exp_index to trigger IR LED
//
// Variable name: ir2day_exp_index
//  IR to Day exp_index to turn off IR LED
//
// Variable name: ir_mode_day2ir_check_cnt
//  Stable count of Day to IR mode switching
//
// Variable name: ir_mode_ir2day_check_cnt
//  Stable count of IR to Day mode switching
//
// Variable name: ir_mode_skip_check_cnt
//  Next check period after IR/Day switching
//
// Variable name: ir_mode_aec_skip_cnt
//  Skip frames of AEC after IR cut switched
//=============================================================================
typedef struct
{
    int                 ir_mode_enable;
    ir_led_adjust_type  ir_led_adj;
    ir_als_adjust_type  ir_als_adj;
    float               ir_center_weight[16][16];
    float               ir_highlight_suppress_weight[64];
    float               ir_mode_conv_speed;
    int                 day2ir_exp_index;
    int                 ir2day_exp_index;
    int                 ir_mode_day2ir_check_cnt;
    int                 ir_mode_ir2day_check_cnt;
    int                 ir_mode_skip_check_cnt;
    int                 ir_mode_aec_skip_cnt;
    ir_daylight_distribution_adjust_type ir_daylight_dist_adj;
    float               reserved[200];
} chromatix_ir_mode_type;

//=============================================================================
// Feature name: chromatix_iot_reserved
// Reserved Params
//
// Variable name: stats_reserved
//  Reserved Params for Stats related tuning parameters
//
// Variable name: generic_reserved
//  Reserved Params for generic tuning parameters
//=============================================================================
typedef struct
{
    float               stats_reserved[2048];
    float               generic_reserved[1024];
} chromatix_iot_reserved;


typedef struct
{
    //=========================================================================
    // Chromatix header version info (MUST BE THE FIRST PARAMETER)
    //=========================================================================
    ChromatixVersionInfoType        chromatix_version_info;

    IOT_version_type                iot_version;

    //=========================================================================
    // SHDR
    //=========================================================================
    chromatix_shdr_type             shdr_mode_data;

    //=========================================================================
    //Tnr mode
    //=========================================================================
    chromatix_tnr_type              tnr_data;

    //=========================================================================
    // AEC
    //=========================================================================
    chromatix_ir_mode_type          irmode_data;

    //=========================================================================
    // IOT Reserve
    //=========================================================================
    chromatix_iot_reserved          iot_reserved_params;
} chromatix_iot_parms_type;

#endif
