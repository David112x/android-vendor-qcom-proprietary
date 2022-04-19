/*============================================================================

  Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/

#ifndef CPP_HW_MACRO_H
#define CPP_HW_MACRO_H

#ifndef sign
#define sign(x) (((x) < 0) ?(-1) : (1))
#endif

#ifndef Round
#define Round(x) (int)(x + sign(x)*0.5)
#endif

#ifndef clamp
#define clamp(x, t1, t2) (((x) < (t1))? (t1): ((x) > (t2))? (t2): (x))
#endif

#ifndef MIN
#define MIN(x,y) (((x)<(y)) ? (x) : (y))
#endif

extern struct Chromatix_HDR_ASF_adj_type chromatix_hdr_adj_asf_params;
extern struct Chromatix_HDR_WNR_adj_type chromatix_hdr_wnr_adj_params;
extern struct Chromatix_ADRC_WNR_adj_type chromatix_adrc_wnr_adj_params;


#define LINEAR_INTERPOLATE(factor, reference_i, reference_iplus1) \
  (((factor) * (reference_iplus1)) + ((1 - (factor)) * (reference_i)))

#if ((defined(CHROMATIX_307) || defined(CHROMATIX_306) || defined(CHROMATIX_308) || \
  defined (CHROMATIX_309) || defined(CHROMATIX_310)) && \
  defined(CAMERA_USE_CHROMATIX_HW_WNR_TYPE))
/* WNR Macro funcions */

#define WNR_LAYERS HW_WAVELET_LEVELS
#define CPP_DENOISE_NUM_PROFILES HW_WAVELET_LEVELS
#define BILITERAL_LAYERS HW_WNR_BILATERAL_FILTER_LEVELS
#define TNR_SCALE_LAYERS HW_TDN_LEVELS

#define GET_SCALE_COUNT()  \
  (sizeof(((hw_wavelet_scale_type *)0)->noise_profile_adj_y)/sizeof(float))

#define GET_UPSCALE_LEVELS(pointer, x) \
  ((Chromatix_wavelet_scaling_type *)pointer)->wnr_upscale_data[x].scaling_level

/* Downscale parameter common to 308 */
#if defined (CHROMATIX_308) || defined (CHROMATIX_309)|| defined(CHROMATIX_310)

#define GET_DOWNSCALE_LEVELS(pointer, x, ds_type) \
  ds_type ? \
  ((Chromatix_wavelet_scaling_type *)pointer)->cpp_wnr_downscale_data[x].scaling_level : \
  ((Chromatix_wavelet_scaling_type *)pointer)->wnr_downscale_data[x].scaling_level

#define DOWN_PROFILE_Y_ADJ(pointer, x, y, ds_type) \
  ds_type ? \
  ((Chromatix_wavelet_scaling_type *)pointer)->cpp_wnr_downscale_data[x].noise_profile_adj_y[y] : \
  ((Chromatix_wavelet_scaling_type *)pointer)->wnr_downscale_data[x].noise_profile_adj_y[y]

#define DOWN_PROFILE_CBCR_ADJ(pointer, x, y, ds_type) \
  ds_type ? \
  ((Chromatix_wavelet_scaling_type *)pointer)->cpp_wnr_downscale_data[x].noise_profile_adj_chroma[y] : \
  ((Chromatix_wavelet_scaling_type *)pointer)->wnr_downscale_data[x].noise_profile_adj_chroma[y]

#define DOWN_WEIGHT_Y_ADJ(pointer, x, y, ds_type) \
  ds_type ? \
  ((Chromatix_wavelet_scaling_type *)pointer)->cpp_wnr_downscale_data[x].denoise_weight_adj_y[y] : \
  ((Chromatix_wavelet_scaling_type *)pointer)->wnr_downscale_data[x].denoise_weight_adj_y[y]

#define DOWN_WEIGHT_CBCR_ADJ(pointer, x, y, ds_type) \
  ds_type ? \
  ((Chromatix_wavelet_scaling_type *)pointer)->cpp_wnr_downscale_data[x].denoise_weight_adj_chroma[y] : \
  ((Chromatix_wavelet_scaling_type *)pointer)->wnr_downscale_data[x].denoise_weight_adj_chroma[y]

#define GET_WNR_DEC_LUXIDX_TRIGGER_START(pointer) \
  ((Chromatix_hardware_wavelet_type *)pointer)->wnr_dec_filt_hysteresis_trigger.lux_index_start

#define GET_WNR_DEC_LUXIDX_TRIGGER_END(pointer) \
  ((Chromatix_hardware_wavelet_type *)pointer)->wnr_dec_filt_hysteresis_trigger.lux_index_end

#define GET_WNR_DEC_GAIN_TRIGGER_START(pointer) \
  ((Chromatix_hardware_wavelet_type *)pointer)->wnr_dec_filt_hysteresis_trigger.gain_start

#define GET_WNR_DEC_GAIN_TRIGGER_END(pointer) \
  ((Chromatix_hardware_wavelet_type *)pointer)->wnr_dec_filt_hysteresis_trigger.gain_end

#else /* Non 308 chromatix headers */
#define GET_DOWNSCALE_LEVELS(pointer, x, ds_type) \
  ((Chromatix_wavelet_scaling_type *)pointer)->wnr_downscale_data[x].scaling_level

#define DOWN_PROFILE_Y_ADJ(pointer, x, y, ds_type) \
  ((Chromatix_wavelet_scaling_type *)pointer)->wnr_downscale_data[x].noise_profile_adj_y[y]

#define DOWN_PROFILE_CBCR_ADJ(pointer, x, y, ds_type) \
  ((Chromatix_wavelet_scaling_type *)pointer)->wnr_downscale_data[x].noise_profile_adj_chroma[y]

#define DOWN_WEIGHT_Y_ADJ(pointer, x, y, ds_type) \
  ((Chromatix_wavelet_scaling_type *)pointer)->wnr_downscale_data[x].denoise_weight_adj_y[y]

#define DOWN_WEIGHT_CBCR_ADJ(pointer, x, y, ds_type) \
  ((Chromatix_wavelet_scaling_type *)pointer)->wnr_downscale_data[x].denoise_weight_adj_chroma[y]

#define GET_WNR_DEC_LUXIDX_TRIGGER_START(pointer) 0.0f

#define GET_WNR_DEC_LUXIDX_TRIGGER_END(pointer) 0.0f

#define GET_WNR_DEC_GAIN_TRIGGER_START(pointer) 0.0f

#define GET_WNR_DEC_GAIN_TRIGGER_END(pointer) 0.0f
#endif

#define UP_PROFILE_Y_ADJ(pointer, x, y) \
  ((Chromatix_wavelet_scaling_type *)pointer)->wnr_upscale_data[x].noise_profile_adj_y[y]

#define UP_PROFILE_CBCR_ADJ(pointer, x, y) \
  ((Chromatix_wavelet_scaling_type *)pointer)->wnr_upscale_data[x].noise_profile_adj_chroma[y]

#define UP_WEIGHT_Y_ADJ(pointer, x, y) \
  ((Chromatix_wavelet_scaling_type *)pointer)->wnr_upscale_data[x].denoise_weight_adj_y[y]

#define UP_WEIGHT_CBCR_ADJ(pointer, x, y) \
  ((Chromatix_wavelet_scaling_type *)pointer)->wnr_upscale_data[x].denoise_weight_adj_chroma[y]

#define WNR_NOISE_PROFILE_Y_DATA(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->noise_profile_y[x]

#define WNR_NOISE_PROFILE_CB_DATA(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->noise_profile_cb[x]

#define WNR_NOISE_PROFILE_CR_DATA(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->noise_profile_cr[x]

#define WNR_SCALE_Y_DATA(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->denoise_scale_y[x]

#define WNR_SCALE_CHROMA_DATA(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->denoise_scale_chroma[x]

#define WNR_EDGE_SOFTNESS_Y_DATA(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->denoise_edge_softness_y[x]

#define WNR_EDGE_SOFTNESS_CHROMA_DATA(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->denoise_edge_softness_chroma[x]

#define WNR_WEIGHT_Y_DATA(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->denoise_weight_y[x]

#define WNR_WEIGHT_CHROMA_DATA(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->denoise_weight_chroma[x]

#if defined(CHROMATIX_308) || defined (CHROMATIX_309) || defined (CHROMATIX_310)

#if defined(CHROMATIX_308)
#define GET_HDR_WNR_ADJ_TYPE_PTR(ptr, params, dst_ptr) \
  dst_ptr = (Chromatix_HDR_WNR_adj_type *)&chromatix_hdr_wnr_adj_params;
#define GET_HDR_ASF_ADJ_TYPE_PTR(ptr, params, dst_ptr) \
  dst_ptr = (Chromatix_HDR_ASF_adj_type *)&chromatix_hdr_adj_asf_params;
#elif defined (CHROMATIX_309) || defined (CHROMATIX_310)
#define GET_HDR_WNR_ADJ_TYPE_PTR(ptr, params, dst_ptr) \
  dst_ptr = &((chromatix_cpp_type *)ptr)->chromatix_hdr_wnr_adj_data
#define GET_HDR_ASF_ADJ_TYPE_PTR(ptr, params, dst_ptr) \
  dst_ptr = &((chromatix_cpp_type *)ptr)->chromatix_hdr_asf_adj_data
#endif
#define GET_HDR_WNR_ADJ_TYPE_ENABLE(ptr) \
  ((Chromatix_HDR_WNR_adj_type *)ptr)->enable

#define GET_HDR_WNR_ADJ_CONTROL_TYPE(ptr) \
  ((Chromatix_HDR_WNR_adj_type *)ptr)->control_hdr_wnr

#define GET_HDR_WNR_ADJ_DATA(ptr, x, dst_ptr) \
  dst_ptr = \
  &((Chromatix_HDR_WNR_adj_type *)ptr)->hdr_wavelet_adj_data[x].wavelet_adj_data;

#define GET_HDR_WNR_ADJ_DATA_AEC_SENSITIVITY_RATIO_START(ptr, x) \
  ((Chromatix_HDR_WNR_adj_type *)ptr)->hdr_wavelet_adj_data[x].aec_sensitivity_ratio.start;

#define GET_HDR_WNR_ADJ_DATA_AEC_SENSITIVITY_RATIO_END(ptr, x) \
  ((Chromatix_HDR_WNR_adj_type *)ptr)->hdr_wavelet_adj_data[x].aec_sensitivity_ratio.end;

#define GET_HDR_WNR_ADJ_DATA_EXP_TIME_RATIO_START(ptr, x) \
  ((Chromatix_HDR_WNR_adj_type *)ptr)->hdr_wavelet_adj_data[x].exp_time_trigger.start;

#define GET_HDR_WNR_ADJ_DATA_EXP_TIME_RATIO_END(ptr, x) \
  ((Chromatix_HDR_WNR_adj_type *)ptr)->hdr_wavelet_adj_data[x].exp_time_trigger.end;

#define GET_HDR_ASF_ADJ_TYPE_ENABLE(ptr) \
  ((Chromatix_HDR_ASF_adj_type *)ptr)->enable

#define GET_HDR_ASF_ADJ_CONTROL_TYPE(ptr) \
  ((Chromatix_HDR_ASF_adj_type *)ptr)->control_hdr_asf

#define GET_HDR_ASF_ADJ_DATA(ptr, x) \
  ((Chromatix_HDR_ASF_adj_type *)ptr)->hdr_asf_adj_data[x];

#define GET_HDR_ASF_ADJ_DATA_AEC_SENSITIVITY_RATIO_START(ptr, x) \
  ((Chromatix_HDR_ASF_adj_type *)ptr)->hdr_asf_adj_data[x].aec_sensitivity_ratio.start;

#define GET_HDR_ASF_ADJ_DATA_AEC_SENSITIVITY_RATIO_END(ptr, x) \
  ((Chromatix_HDR_ASF_adj_type *)ptr)->hdr_asf_adj_data[x].aec_sensitivity_ratio.end;

#define GET_HDR_ASF_ADJ_DATA_EXP_TIME_RATIO_START(ptr, x) \
  ((Chromatix_HDR_ASF_adj_type *)ptr)->hdr_asf_adj_data[x].exp_time_trigger.start;

#define GET_HDR_ASF_ADJ_DATA_EXP_TIME_RATIO_END(ptr, x) \
  ((Chromatix_HDR_ASF_adj_type *)ptr)->hdr_asf_adj_data[x].exp_time_trigger.end;

#define GET_HDR_ASF_ADJ_DATA_ADJ_FACTOR(ptr, x) \
  ((Chromatix_HDR_ASF_adj_type *)ptr)->hdr_asf_adj_data[x].asf_adj_factor;

#define GET_WAVELET_DENOISE_POINTER(ptr, params, dst_ptr) \
  if(params->lpm_enabled) \
    dst_ptr = \
      &((chromatix_cpp_type *)ptr)->chromatix_hw_denoise_data_lpm; \
  else \
    dst_ptr = \
      &((chromatix_cpp_type *)ptr)->chromatix_hw_denoise_data;

#define GET_WNR_DOWNSCALE_DATA(ptr, params, dst_ptr) \
  if(params->lpm_enabled) \
    dst_ptr = \
      &((chromatix_cpp_type *)ptr)->chromatix_hw_denoise_data_lpm.hw_wavelet_scale_adj; \
  else \
    dst_ptr = \
      &((chromatix_cpp_type *)ptr)->chromatix_hw_denoise_data.hw_wavelet_scale_adj;

#define GET_WNR_UPSCALE_DATA(ptr, params, dst_ptr) \
  if(params->lpm_enabled) \
    dst_ptr = \
      &((chromatix_cpp_type *)ptr)->chromatix_hw_denoise_data_lpm.hw_wavelet_scale_adj; \
  else \
    dst_ptr = \
      &((chromatix_cpp_type *)ptr)->chromatix_hw_denoise_data.hw_wavelet_scale_adj;

#else

#define GET_WAVELET_DENOISE_POINTER(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_hw_denoise_data

#define GET_WNR_DOWNSCALE_DATA(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_hw_denoise_data.hw_wavelet_scale_adj

#define GET_WNR_UPSCALE_DATA(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_hw_denoise_data.hw_wavelet_scale_adj
#endif
#define GET_WNR_NOISE_PROFILE(pointer, x, dst_ptr) \
  dst_ptr = \
    &((Chromatix_hardware_wavelet_type *)pointer)->noise_profile[x]

#define GET_WNR_LUXIDX_TRIGER_START(pointer, x) \
  ((Chromatix_hardware_wavelet_type *)pointer)->noise_profile[x].WNR_trigger.lux_index_start

#define GET_WNR_LUXIDX_TRIGER_END(pointer, x) \
  ((Chromatix_hardware_wavelet_type *)pointer)->noise_profile[x].WNR_trigger.lux_index_end

#define GET_WNR_GAIN_TRIGER_START(pointer, x) \
  ((Chromatix_hardware_wavelet_type *)pointer)->noise_profile[x].WNR_trigger.gain_start

#define GET_WNR_GAIN_TRIGER_END(pointer, x) \
  ((Chromatix_hardware_wavelet_type *)pointer)->noise_profile[x].WNR_trigger.gain_end
#else
#define WNR_LAYERS WAVELET_LEVEL
#define CPP_DENOISE_NUM_PROFILES WAVELET_LEVEL+1
#define BILITERAL_LAYERS WAVELET_LEVEL
#define TNR_SCALE_LAYERS WAVELET_LEVEL

#define GET_HDR_WNR_ADJ_TYPE_PTR(ptr, params, dst_ptr) \
  dst_ptr = NULL;

#define GET_HDR_ASF_ADJ_TYPE_PTR(ptr, params, dst_ptr) \
  dst_ptr = NULL;

#define GET_HDR_WNR_ADJ_TYPE_ENABLE(ptr) 0

#define GET_HDR_WNR_ADJ_CONTROL_TYPE(ptr) 0

#define GET_HDR_WNR_ADJ_DATA(ptr, x, dst_ptr) 0

#define GET_HDR_WNR_ADJ_DATA_AEC_SENSITIVITY_RATIO_START(ptr, x) 0

#define GET_HDR_WNR_ADJ_DATA_AEC_SENSITIVITY_RATIO_END(ptr, x) 0

#define GET_HDR_WNR_ADJ_DATA_EXP_TIME_RATIO_START(ptr, x) 0

#define GET_HDR_WNR_ADJ_DATA_EXP_TIME_RATIO_END(ptr, x) 0

#define GET_HDR_ASF_ADJ_TYPE_ENABLE(ptr) 0

#define GET_HDR_ASF_ADJ_CONTROL_TYPE(ptr) 0

#define GET_HDR_ASF_ADJ_DATA(ptr, x) 0

#define GET_HDR_ASF_ADJ_DATA_AEC_SENSITIVITY_RATIO_START(ptr, x) 0

#define GET_HDR_ASF_ADJ_DATA_AEC_SENSITIVITY_RATIO_END(ptr, x) 0

#define GET_HDR_ASF_ADJ_DATA_EXP_TIME_RATIO_START(ptr, x) 0

#define GET_HDR_ASF_ADJ_DATA_EXP_TIME_RATIO_END(ptr, x) 0

#define GET_HDR_ASF_ADJ_DATA_ADJ_FACTOR(ptr, x) 0

#define GET_SCALE_COUNT()  \
  (sizeof(((wavelet_downscale_type *)0)->Y_noise_profile_adj_downscale)/sizeof(float))

#define GET_UPSCALE_LEVELS(pointer, x) \
  ((NoiseProfile_Upscaling_type *)pointer)->wnr_upscale_data[x].upscaling_levels

#define GET_DOWNSCALE_LEVELS(pointer, x, ds_type) \
  ((NoiseProfile_Downscaling_type *)pointer)->wnr_downscale_data[x].downscaling_levels;

#define DOWN_PROFILE_Y_ADJ(pointer, x, y, ds_type) \
  ((NoiseProfile_Downscaling_type *)pointer)->wnr_downscale_data[x].Y_noise_profile_adj_downscale

#define DOWN_PROFILE_CBCR_ADJ(pointer, x, y, ds_type) \
  ((NoiseProfile_Downscaling_type *)pointer)->wnr_downscale_data[x].chroma_noise_profile_adj_downscale

#define DOWN_WEIGHT_Y_ADJ(pointer, x, y, ds_type) \
  ((NoiseProfile_Downscaling_type *)pointer)->wnr_downscale_data[x].Y_denoise_weight_adj_downscale

#define DOWN_WEIGHT_CBCR_ADJ(pointer, x, y, ds_type) \
  ((NoiseProfile_Downscaling_type *)pointer)->wnr_downscale_data[x].chroma_denoise_weight_adj_downscale

#define UP_PROFILE_Y_ADJ(pointer, x, y) \
  ((NoiseProfile_Upscaling_type *)pointer)->wnr_upscale_data[x].Y_noise_profile_adj_upscale

#define UP_PROFILE_CBCR_ADJ(pointer, x, y) \
  ((NoiseProfile_Upscaling_type *)pointer)->wnr_upscale_data[x].chroma_noise_profile_adj_upscale

#define UP_WEIGHT_Y_ADJ(pointer, x, y) \
  ((NoiseProfile_Upscaling_type *)pointer)->wnr_upscale_data[x].Y_denoise_weight_adj_upscale

#define UP_WEIGHT_CBCR_ADJ(pointer, x, y) \
  ((NoiseProfile_Upscaling_type *)pointer)->wnr_upscale_data[x].chroma_denoise_weight_adj_upscale

#define WNR_NOISE_PROFILE_DATA(pointer, x) \
  ((ReferenceNoiseProfile_type *)pointer)->referenceNoiseProfileData[x]

#define WNR_SCALE_Y_DATA(pointer, x) \
  ((ReferenceNoiseProfile_type *)pointer)->denoise_scale_y[x]

#define WNR_SCALE_CHROMA_DATA(pointer, x) \
  ((ReferenceNoiseProfile_type *)pointer)->denoise_scale_chroma[x]

#define WNR_EDGE_SOFTNESS_Y_DATA(pointer, x) \
  ((ReferenceNoiseProfile_type *)pointer)->denoise_edge_softness_y[x]

#define WNR_EDGE_SOFTNESS_CHROMA_DATA(pointer, x) \
  ((ReferenceNoiseProfile_type *)pointer)->denoise_edge_softness_chroma[x]

#define WNR_WEIGHT_Y_DATA(pointer, x) \
  ((ReferenceNoiseProfile_type *)pointer)->denoise_weight_y[x]

#define WNR_WEIGHT_CHROMA_DATA(pointer, x) \
  ((ReferenceNoiseProfile_type *)pointer)->denoise_weight_chroma[x]

#define WNR_SCALE_5TH_CHROMA_DATA(pointer) \
  ((ReferenceNoiseProfile_type *)pointer)->denoise_scale_chroma_5th

#define WNR_WEIGHT_5TH_CHROMA_DATA(pointer) \
  ((ReferenceNoiseProfile_type *)pointer)->denoise_weight_chroma_5th

#define GET_WAVELET_DENOISE_POINTER(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_wavelet.wavelet_denoise_HW_420

#define GET_WNR_DOWNSCALE_DATA(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_wavelet.wavelet_downscale_adj

#define GET_WNR_UPSCALE_DATA(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_wavelet.wavelet_upscale_adj

#define GET_WNR_NOISE_PROFILE(pointer, x, dst_ptr) \
  dst_ptr = \
    &((wavelet_denoise_type *)pointer)->noise_profile[x]

#define GET_WNR_LUXIDX_TRIGER_START(pointer, x) \
  ((wavelet_denoise_type *)pointer)->noise_profile[x].WNR_trigger.lux_index_start

#define GET_WNR_LUXIDX_TRIGER_END(pointer, x) \
  ((wavelet_denoise_type *)pointer)->noise_profile[x].WNR_trigger.lux_index_end

#define GET_WNR_GAIN_TRIGER_START(pointer, x) \
  ((wavelet_denoise_type *)pointer)->noise_profile[x].WNR_trigger.gain_start

#define GET_WNR_GAIN_TRIGER_END(pointer, x) \
  ((wavelet_denoise_type *)pointer)->noise_profile[x].WNR_trigger.gain_end

#define GET_WNR_DEC_LUXIDX_TRIGGER_START(pointer) 0.0f

#define GET_WNR_DEC_LUXIDX_TRIGGER_END(pointer) 0.0f

#define GET_WNR_DEC_GAIN_TRIGGER_START(pointer) 0.0f

#define GET_WNR_DEC_GAIN_TRIGGER_END(pointer) 0.0f
#endif

#if defined(CHROMATIX_310)

#define WNR_RNR_LNR_VALID(pointer) (pointer != NULL)

#define WNR_LNR_POINTS WNR_LEVEL_TABLE_ENTRIES

#define WNR_RNR_POINTS ASF_WNR_RADIAL_POINTS

#define WNR_PADDING_MODE_Y(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->PaddingMode_y[x]

#define WNR_PADDING_MODE_CHROMA(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->PaddingMode_chroma[x]

#define WNR_RADIAL_AND_LEVEL_WEIGHT_Y_ENABLE(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->radial_and_level_weight_y_enable[x]

#define WNR_RADIAL_WEIGHT_CHROMA_ENABLE(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->radial_weight_chroma_enable[x]

#define WNR_LEVEL_EDGESOFTNESS_ADJ_FACTOR(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->level_edgesoftness_adj_factor[x]

#define WNR_LEVEL_WEIGHT_ADJ_FACTOR(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->level_weight_adj_factor[x]

#define WNR_RADIAL_EDGESOFTNESS_ADJ_FACTOR_Y(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->radial_edgesoftness_adj_factor_y[x]

#define WNR_RADIAL_EDGESOFTNESS_ADJ_FACTOR_CHROMA(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->radial_edgesoftness_adj_factor_chroma[x]

#define WNR_RADIAL_WEIGHT_ADJ_FACTOR_Y(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->radial_weight_adj_factor_y[x]

#define WNR_RADIAL_WEIGHT_ADJ_FACTOR_CHROMA(pointer, x) \
  ((hw_WNR_noiseprofile_type *)pointer)->radial_weight_adj_factor_chroma[x]

#define GET_WNR_RESERVED_DATA(pointer) \
  &(((Chromatix_hardware_wavelet_type *)pointer)->hw_wavelet_reserve_data)

#define WNR_RADIAL_AND_LEVEL_Y_ENABLE(pointer, x) \
  ((Chromatix_hardware_wavelet_type *)pointer)->radial_and_level_y_enable[x]

#define WNR_RADIAL_CHROMA_ENABLE(pointer, x) \
  ((Chromatix_hardware_wavelet_type *)pointer)->radial_chroma_enable[x]

#define WNR_RADIAL_PT_TABLE(pointer, x) \
  ((Chromatix_wavelet_reserve_type *)pointer)->radial_pt_table[x]

#define WNR_TRILATERAL_FILTER_Y_ENABLE(pointer, x) \
  ((Chromatix_wavelet_reserve_type *)pointer)->transition_enable_luma[x]

#define WNR_TRILATERAL_FILTER_CHROMA_ENABLE(pointer, x) \
  ((Chromatix_wavelet_reserve_type *)pointer)->transition_enable_chroma[x]

#define GET_WNR_RNR_LUXIDX_TRIGGER_START(pointer) \
  ((Chromatix_hardware_wavelet_type *)pointer)->wnr_radial_level_hysteresis_trigger.lux_index_start

#define GET_WNR_RNR_LUXIDX_TRIGGER_END(pointer) \
  ((Chromatix_hardware_wavelet_type *)pointer)->wnr_radial_level_hysteresis_trigger.lux_index_end

#define GET_WNR_RNR_GAIN_TRIGGER_START(pointer) \
  ((Chromatix_hardware_wavelet_type *)pointer)->wnr_radial_level_hysteresis_trigger.gain_start

#define GET_WNR_RNR_GAIN_TRIGGER_END(pointer) \
  ((Chromatix_hardware_wavelet_type *)pointer)->wnr_radial_level_hysteresis_trigger.gain_end

#define GET_ASF_RNR_HYSTERESIS_TRIGGER_TYPE(pointer) \
  ((chromatix_ASF_9x9_type *)pointer)->control_asf_9x9

#define GET_ASF_RNR_HYSTERESIS_LUXIDX_TRIGER_START(pointer) \
  ((chromatix_ASF_9x9_type *)pointer)->asf_radial_hysteresis_trigger.lux_index_start

#define GET_ASF_RNR_HYSTERESIS_LUXIDX_TRIGER_END(pointer) \
  ((chromatix_ASF_9x9_type *)pointer)->asf_radial_hysteresis_trigger.lux_index_end

#define GET_ASF_RNR_HYSTERESIS_GAIN_TRIGER_START(pointer) \
  ((chromatix_ASF_9x9_type *)pointer)->asf_radial_hysteresis_trigger.gain_start

#define GET_ASF_RNR_HYSTERESIS_GAIN_TRIGER_END(pointer) \
  ((chromatix_ASF_9x9_type *)pointer)->asf_radial_hysteresis_trigger.gain_end



#else // defined(CHROMATIX_310)

#define WNR_RNR_LNR_VALID(pointer) FALSE

#define WNR_LNR_POINTS 33

#define WNR_RNR_POINTS 4

#define WNR_PADDING_MODE_Y(pointer, x) 0

#define WNR_PADDING_MODE_CHROMA(pointer, x) 0

#define WNR_RADIAL_AND_LEVEL_WEIGHT_Y_ENABLE(pointer, x) ({ \
  (void)pointer; \
  0; \
})

#define WNR_RADIAL_WEIGHT_CHROMA_ENABLE(pointer, x) 0

#define WNR_LEVEL_EDGESOFTNESS_ADJ_FACTOR(pointer, x) 1.0f

#define WNR_LEVEL_WEIGHT_ADJ_FACTOR(pointer, x) 1.0f

#define WNR_RADIAL_EDGESOFTNESS_ADJ_FACTOR_Y(pointer, x) 1.0f

#define WNR_RADIAL_EDGESOFTNESS_ADJ_FACTOR_CHROMA(pointer, x) 1.0f

#define WNR_RADIAL_WEIGHT_ADJ_FACTOR_Y(pointer, x) 1.0f

#define WNR_RADIAL_WEIGHT_ADJ_FACTOR_CHROMA(pointer, x) 1.0f

#define GET_WNR_RESERVED_DATA(pointer) 0

#define WNR_RADIAL_AND_LEVEL_Y_ENABLE(pointer, x) ({ \
  (void)pointer; \
  0; \
})
#define WNR_RADIAL_CHROMA_ENABLE(pointer, x) 0

#define WNR_RADIAL_PT_TABLE(pointer, x) 0.0f

#define WNR_TRILATERAL_FILTER_Y_ENABLE(pointer, x) 0

#define WNR_TRILATERAL_FILTER_CHROMA_ENABLE(pointer, x) 0

#define GET_WNR_RNR_LUXIDX_TRIGGER_START(pointer) 0

#define GET_WNR_RNR_LUXIDX_TRIGGER_END(pointer) 0

#define GET_WNR_RNR_GAIN_TRIGGER_START(pointer) 0

#define GET_WNR_RNR_GAIN_TRIGGER_END(pointer) 0

#define GET_ASF_RNR_HYSTERESIS_TRIGGER_TYPE(pointer) 0

#define GET_ASF_RNR_HYSTERESIS_LUXIDX_TRIGER_START(pointer) 0

#define GET_ASF_RNR_HYSTERESIS_LUXIDX_TRIGER_END(pointer) 0

#define GET_ASF_RNR_HYSTERESIS_GAIN_TRIGER_START(pointer) 0

#define GET_ASF_RNR_HYSTERESIS_GAIN_TRIGER_END(pointer) 0


#endif // defined(CHROMATIX_310)

#if defined(CHROMATIX_307) || defined(CHROMATIX_306) || defined(CHROMATIX_308) || \
  defined(CHROMATIX_309)|| defined(CHROMATIX_310)


#if defined(CHROMATIX_308)
#define GET_ADRC_WNR_ADJ_TYPE_PTR(ptr, params, dst_ptr) \
  dst_ptr = (Chromatix_ADRC_WNR_adj_type *)&chromatix_adrc_wnr_adj_params;
#elif defined (CHROMATIX_309) || defined (CHROMATIX_310)
#define GET_ADRC_WNR_ADJ_TYPE_PTR(ptr, params, dst_ptr) \
    dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_adrc_wnr_adj_data;
#endif

#define GET_WNR_ADJ_DATA_PROFILE_Y_ADJ(ptr, x) \
  ((wavelet_adj_type *)ptr)->noise_profile_adj_y[x]

#define GET_WNR_ADJ_DATA_PROFILE_CBCR_ADJ(ptr, x) \
  ((wavelet_adj_type *)ptr)->noise_profile_adj_chroma[x]

#define GET_WNR_ADJ_DATA_WEIGHT_Y_ADJ(ptr, x) \
  ((wavelet_adj_type *)ptr)->denoise_weight_adj_y[x]

#define GET_WNR_ADJ_DATA_WEIGHT_CBCR_ADJ(ptr, x) \
  ((wavelet_adj_type *)ptr)->denoise_weight_adj_chroma[x]

#define GET_WNR_ADJ_COUNT() \
  (sizeof(((wavelet_adj_type *)0)->noise_profile_adj_y)/sizeof(float))

#define GET_ADRC_ADJ_COUNT() \
  (sizeof(((wavelet_adj_type *)0)->noise_profile_adj_y)/sizeof(float))

#define GET_ADRC_WNR_ADJ_TYPE_ENABLE(ptr) \
  ((Chromatix_ADRC_WNR_adj_type *)ptr)->enable

#define GET_ADRC_WNR_ADJ_CONTROL_TYPE(ptr) \
  ((Chromatix_ADRC_WNR_adj_type *)ptr)->control_adrc_wnr

#define GET_ADRC_WNR_ADJ_DATA(ptr, x, dst_ptr) \
  dst_ptr = \
  &((Chromatix_ADRC_WNR_adj_type *)ptr)->adrc_wavelet_adj_data[x].wavelet_adj_data;

#define GET_ADRC_WNR_ADJ_DATA_AEC_SENSITIVITY_RATIO_START(ptr, x) \
  ((Chromatix_ADRC_WNR_adj_type *)ptr)->adrc_wavelet_adj_data[x].aec_sensitivity_ratio.start;

#define GET_ADRC_WNR_ADJ_DATA_AEC_SENSITIVITY_RATIO_END(ptr, x) \
  ((Chromatix_ADRC_WNR_adj_type *)ptr)->adrc_wavelet_adj_data[x].aec_sensitivity_ratio.end;

#define GET_ADRC_WNR_ADJ_DATA_DRC_GAIN_TRIGGER_START(ptr, x) \
  ((Chromatix_ADRC_WNR_adj_type *)ptr)->adrc_wavelet_adj_data[x].drc_gain_trigger.start;

#define GET_ADRC_WNR_ADJ_DATA_DRC_GAIN_TRIGGER_END(ptr, x) \
  ((Chromatix_ADRC_WNR_adj_type *)ptr)->adrc_wavelet_adj_data[x].drc_gain_trigger.end;


/* TNR Macro funcions */
#define GET_TNR_SCALE_COUNT()  \
  (sizeof(((hw_TDN_scale_type *)0)->noise_profile_adj_y)/sizeof(float))

#define GET_TNR_UPSCALE_LEVELS(pointer, x) \
  ((Chromatix_TDN_scaling_type *)pointer)->TDN_upscale_data[x].scaling_level

#define GET_TNR_DOWNSCALE_LEVELS(pointer, x) \
  ((Chromatix_TDN_scaling_type *)pointer)->TDN_downscale_data[x].scaling_level;

#define TNR_DOWN_PROFILE_Y_ADJ(pointer, x, y) \
  ((Chromatix_TDN_scaling_type *)pointer)->TDN_downscale_data[x].noise_profile_adj_y[y]

#define TNR_DOWN_PROFILE_CBCR_ADJ(pointer, x, y) \
  ((Chromatix_TDN_scaling_type *)pointer)->TDN_downscale_data[x].noise_profile_adj_chroma[y]

#define TNR_DOWN_WEIGHT_Y_ADJ(pointer, x, y) \
  ((Chromatix_TDN_scaling_type *)pointer)->TDN_downscale_data[x].denoise_weight_adj_y[y]

#define TNR_DOWN_WEIGHT_CBCR_ADJ(pointer, x, y) \
  ((Chromatix_TDN_scaling_type *)pointer)->TDN_downscale_data[x].denoise_weight_adj_chroma[y]

#define TNR_UP_PROFILE_Y_ADJ(pointer, x, y) \
  ((Chromatix_TDN_scaling_type *)pointer)->TDN_upscale_data[x].noise_profile_adj_y[y]

#define TNR_UP_PROFILE_CBCR_ADJ(pointer, x, y) \
  ((Chromatix_TDN_scaling_type *)pointer)->TDN_upscale_data[x].noise_profile_adj_chroma[y]

#define TNR_UP_WEIGHT_Y_ADJ(pointer, x, y) \
  ((Chromatix_TDN_scaling_type *)pointer)->TDN_upscale_data[x].denoise_weight_adj_y[y]

#define TNR_UP_WEIGHT_CBCR_ADJ(pointer, x, y) \
  ((Chromatix_TDN_scaling_type *)pointer)->TDN_upscale_data[x].denoise_weight_adj_chroma[y]

#define GET_TNR_DOWNSCALE_DATA(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_temporal_denoise.temporal_denoise_scale_adj

#define GET_TNR_UPSCALE_DATA(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_temporal_denoise.temporal_denoise_scale_adj

#define GET_TNR_HYSTERESIS_TRIGGER_TYPE(pointer) \
  ((chromatixWNRDenoise_type *)pointer)->control_WNR_denoise;

#define GET_TNR_LUXIDX_TRIGGER_START(pointer) \
  ((chromatixWNRDenoise_type *)pointer)->tdn_hysteresis_trigger.lux_index_start

#define GET_TNR_LUXIDX_TRIGGER_END(pointer) \
  ((chromatixWNRDenoise_type *)pointer)->tdn_hysteresis_trigger.lux_index_end

#define GET_TNR_GAIN_TRIGGER_START(pointer) \
  ((chromatixWNRDenoise_type *)pointer)->tdn_hysteresis_trigger.gain_start

#define GET_TNR_GAIN_TRIGGER_END(pointer) \
  ((chromatixWNRDenoise_type *)pointer)->tdn_hysteresis_trigger.gain_end

/* Prescale BF Macro functions */
#define PBF_NOISE_PROFILE_Y_DATA(pointer) \
  ((PBF_noiseprofile_type *)pointer)->noise_profile_y

#define PBF_NOISE_PROFILE_CB_DATA(pointer) \
  ((PBF_noiseprofile_type *)pointer)->noise_profile_cb

#define PBF_NOISE_PROFILE_CR_DATA(pointer) \
  ((PBF_noiseprofile_type *)pointer)->noise_profile_cr

#define PBF_SCALE_Y_DATA(pointer) \
  ((PBF_noiseprofile_type *)pointer)->denoise_scale_y

#define PBF_SCALE_CHROMA_DATA(pointer) \
  ((PBF_noiseprofile_type *)pointer)->denoise_scale_chroma

#define PBF_EDGE_SOFTNESS_Y_DATA(pointer) \
  ((PBF_noiseprofile_type *)pointer)->denoise_edge_softness_y

#define PBF_EDGE_SOFTNESS_CHROMA_DATA(pointer) \
  ((PBF_noiseprofile_type *)pointer)->denoise_edge_softness_chroma

#define PBF_WEIGHT_Y_DATA(pointer) \
  ((PBF_noiseprofile_type *)pointer)->denoise_weight_y

#define PBF_WEIGHT_CHROMA_DATA(pointer) \
  ((PBF_noiseprofile_type *)pointer)->denoise_weight_chroma

#define GET_PBF_UPSCALE_LEVELS(pointer, x) \
  ((Chromatix_PBF_scaling_type *)pointer)->PBF_upscale_data[x].scaling_level

#define GET_PBF_DOWNSCALE_LEVELS(pointer, x) \
  ((Chromatix_PBF_scaling_type *)pointer)->PBF_downscale_data[x].scaling_level;

#define PBF_DOWN_PROFILE_Y_ADJ(pointer, x) \
  ((Chromatix_PBF_scaling_type *)pointer)->PBF_downscale_data[x].noise_profile_adj_y

#define PBF_DOWN_PROFILE_CBCR_ADJ(pointer, x) \
  ((Chromatix_PBF_scaling_type *)pointer)->PBF_downscale_data[x].noise_profile_adj_chroma

#define PBF_DOWN_WEIGHT_Y_ADJ(pointer, x) \
  ((Chromatix_PBF_scaling_type *)pointer)->PBF_downscale_data[x].denoise_weight_adj_y

#define PBF_DOWN_WEIGHT_CBCR_ADJ(pointer, x) \
  ((Chromatix_PBF_scaling_type *)pointer)->PBF_downscale_data[x].denoise_weight_adj_chroma

#define PBF_UP_PROFILE_Y_ADJ(pointer, x) \
  ((Chromatix_PBF_scaling_type *)pointer)->PBF_upscale_data[x].noise_profile_adj_y

#define PBF_UP_PROFILE_CBCR_ADJ(pointer, x) \
  ((Chromatix_PBF_scaling_type *)pointer)->PBF_upscale_data[x].noise_profile_adj_chroma

#define PBF_UP_WEIGHT_Y_ADJ(pointer, x) \
  ((Chromatix_PBF_scaling_type *)pointer)->PBF_upscale_data[x].denoise_weight_adj_y

#define PBF_UP_WEIGHT_CBCR_ADJ(pointer, x) \
  ((Chromatix_PBF_scaling_type *)pointer)->PBF_upscale_data[x].denoise_weight_adj_chroma

#define GET_PBF_DENOISE_POINTER(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_pbf_data

#define GET_PBF_DOWNSCALE_DATA(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_pbf_data.pbf_scale_adj

#define GET_PBF_UPSCALE_DATA(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_pbf_data.pbf_scale_adj

#define GET_PBF_NOISE_PROFILE(pointer, x, dst_ptr) \
  dst_ptr = \
    &((Chromatix_prescaler_bf_type *)pointer)->PBF_noise_profile[x]

#define GET_PBF_LUXIDX_TRIGER_START(pointer, x) \
  ((Chromatix_prescaler_bf_type *)pointer)->PBF_noise_profile[x].PBF_trigger.lux_index_start

#define GET_PBF_LUXIDX_TRIGER_END(pointer, x) \
  ((Chromatix_prescaler_bf_type *)pointer)->PBF_noise_profile[x].PBF_trigger.lux_index_end

#define GET_PBF_GAIN_TRIGER_START(pointer, x) \
  ((Chromatix_prescaler_bf_type *)pointer)->PBF_noise_profile[x].PBF_trigger.gain_start

#define GET_PBF_GAIN_TRIGER_END(pointer, x) \
  ((Chromatix_prescaler_bf_type *)pointer)->PBF_noise_profile[x].PBF_trigger.gain_end

#define GET_PBF_TRIGGER_TYPE(pointer) \
  ((Chromatix_prescaler_bf_type *)pointer)->control_PBF

#define GET_PBF_HYSTERESIS_TRIGGER_TYPE(pointer) \
  ((Chromatix_prescaler_bf_type *)pointer)->control_PBF

#define GET_PBF_HYSTERESIS_LUXIDX_TRIGER_START(pointer) \
  ((Chromatix_prescaler_bf_type *)pointer)->pbf_hysteresis_trigger.lux_index_start

#define GET_PBF_HYSTERESIS_LUXIDX_TRIGER_END(pointer) \
  ((Chromatix_prescaler_bf_type *)pointer)->pbf_hysteresis_trigger.lux_index_end

#define GET_PBF_HYSTERESIS_GAIN_TRIGER_START(pointer) \
  ((Chromatix_prescaler_bf_type *)pointer)->pbf_hysteresis_trigger.gain_start

#define GET_PBF_HYSTERESIS_GAIN_TRIGER_END(pointer) \
  ((Chromatix_prescaler_bf_type *)pointer)->pbf_hysteresis_trigger.gain_end

#else


/* TNR Macro Functions */
#define GET_TNR_SCALE_COUNT()  \
  (sizeof(((wavelet_downscale_type *)0)->Y_noise_profile_adj_downscale)/sizeof(float))

#define GET_TNR_UPSCALE_LEVELS(pointer, x) \
  ((NoiseProfile_Upscaling_type *)pointer)->wnr_upscale_data[x].upscaling_levels

#define GET_TNR_DOWNSCALE_LEVELS(pointer, x) \
  ((NoiseProfile_Downscaling_type *)pointer)->wnr_downscale_data[x].downscaling_levels;

#define TNR_DOWN_PROFILE_Y_ADJ(pointer, x, y) \
  ((NoiseProfile_Downscaling_type *)pointer)->wnr_downscale_data[x].Y_noise_profile_adj_downscale

#define TNR_DOWN_PROFILE_CBCR_ADJ(pointer, x, y) \
  ((NoiseProfile_Downscaling_type *)pointer)->wnr_downscale_data[x].chroma_noise_profile_adj_downscale

#define TNR_DOWN_WEIGHT_Y_ADJ(pointer, x, y) \
  ((NoiseProfile_Downscaling_type *)pointer)->wnr_downscale_data[x].Y_denoise_weight_adj_downscale

#define TNR_DOWN_WEIGHT_CBCR_ADJ(pointer, x, y) \
  ((NoiseProfile_Downscaling_type *)pointer)->wnr_downscale_data[x].chroma_denoise_weight_adj_downscale

#define TNR_UP_PROFILE_Y_ADJ(pointer, x, y) \
  ((NoiseProfile_Upscaling_type *)pointer)->wnr_upscale_data[x].Y_noise_profile_adj_upscale

#define TNR_UP_PROFILE_CBCR_ADJ(pointer, x, y) \
  ((NoiseProfile_Upscaling_type *)pointer)->wnr_upscale_data[x].chroma_noise_profile_adj_upscale

#define TNR_UP_WEIGHT_Y_ADJ(pointer, x, y) \
  ((NoiseProfile_Upscaling_type *)pointer)->wnr_upscale_data[x].Y_denoise_weight_adj_upscale

#define TNR_UP_WEIGHT_CBCR_ADJ(pointer, x, y) \
  ((NoiseProfile_Upscaling_type *)pointer)->wnr_upscale_data[x].chroma_denoise_weight_adj_upscale

#define GET_TNR_DOWNSCALE_DATA(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_wavelet.wavelet_downscale_adj

#define GET_TNR_UPSCALE_DATA(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_wavelet.wavelet_upscale_adj

/* Prescsale BF macro functions */

#define GET_PBF_UPSCALE_LEVELS(pointer, x) \
  ((NoiseProfile_Upscaling_type *)pointer)->wnr_upscale_data[x].upscaling_levels

#define GET_PBF_DOWNSCALE_LEVELS(pointer, x) \
  ((NoiseProfile_Downscaling_type *)pointer)->wnr_downscale_data[x].downscaling_levels;

#define PBF_DOWN_PROFILE_Y_ADJ(pointer, x) \
  ((NoiseProfile_Downscaling_type *)pointer)->wnr_downscale_data[x].Y_noise_profile_adj_downscale

#define PBF_DOWN_PROFILE_CBCR_ADJ(pointer, x) \
  ((NoiseProfile_Downscaling_type *)pointer)->wnr_downscale_data[x].chroma_noise_profile_adj_downscale

#define PBF_DOWN_WEIGHT_Y_ADJ(pointer, x) \
  ((NoiseProfile_Downscaling_type *)pointer)->wnr_downscale_data[x].Y_denoise_weight_adj_downscale

#define PBF_DOWN_WEIGHT_CBCR_ADJ(pointer, x) \
  ((NoiseProfile_Downscaling_type *)pointer)->wnr_downscale_data[x].chroma_denoise_weight_adj_downscale

#define PBF_UP_PROFILE_Y_ADJ(pointer, x) \
  ((NoiseProfile_Upscaling_type *)pointer)->wnr_upscale_data[x].Y_noise_profile_adj_upscale

#define PBF_UP_PROFILE_CBCR_ADJ(pointer, x) \
  ((NoiseProfile_Upscaling_type *)pointer)->wnr_upscale_data[x].chroma_noise_profile_adj_upscale

#define PBF_UP_WEIGHT_Y_ADJ(pointer, x) \
  ((NoiseProfile_Upscaling_type *)pointer)->wnr_upscale_data[x].Y_denoise_weight_adj_upscale

#define PBF_UP_WEIGHT_CBCR_ADJ(pointer, x) \
  ((NoiseProfile_Upscaling_type *)pointer)->wnr_upscale_data[x].chroma_denoise_weight_adj_upscale

#define PBF_NOISE_PROFILE_Y_DATA(pointer) \
  ((ReferenceNoiseProfile_type *)pointer)->referenceNoiseProfileData[4]

#define PBF_NOISE_PROFILE_CB_DATA(pointer) \
  ((ReferenceNoiseProfile_type *)pointer)->referenceNoiseProfileData[12]

#define PBF_NOISE_PROFILE_CR_DATA(pointer) \
  ((ReferenceNoiseProfile_type *)pointer)->referenceNoiseProfileData[20]

#define PBF_SCALE_Y_DATA(pointer) \
  ((ReferenceNoiseProfile_type *)pointer)->denoise_scale_y[0]

#define PBF_SCALE_CHROMA_DATA(pointer) \
  ((ReferenceNoiseProfile_type *)pointer)->denoise_scale_chroma[0]

#define PBF_EDGE_SOFTNESS_Y_DATA(pointer) \
  ((ReferenceNoiseProfile_type *)pointer)->denoise_edge_softness_y[0]

#define PBF_EDGE_SOFTNESS_CHROMA_DATA(pointer) \
  ((ReferenceNoiseProfile_type *)pointer)->denoise_edge_softness_chroma[0]

#define PBF_WEIGHT_Y_DATA(pointer) \
  ((ReferenceNoiseProfile_type *)pointer)->denoise_weight_y[0]

#define PBF_WEIGHT_CHROMA_DATA(pointer) \
  ((ReferenceNoiseProfile_type *)pointer)->denoise_weight_chroma[0]

#define GET_PBF_DENOISE_POINTER(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_wavelet.wavelet_denoise_HW_420

#define GET_PBF_DOWNSCALE_DATA(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_wavelet.wavelet_downscale_adj

#define GET_PBF_UPSCALE_DATA(ptr, params, dst_ptr) \
  dst_ptr = \
    &((chromatix_cpp_type *)ptr)->chromatix_wavelet.wavelet_upscale_adj

#define GET_PBF_NOISE_PROFILE(pointer, x, dst_ptr) \
  dst_ptr = \
    &((wavelet_denoise_type *)pointer)->noise_profile[x]

#define GET_PBF_LUXIDX_TRIGER_START(pointer, x) \
  ((wavelet_denoise_type *)pointer)->noise_profile[x].WNR_trigger.lux_index_start

#define GET_PBF_LUXIDX_TRIGER_END(pointer, x) \
  ((wavelet_denoise_type *)pointer)->noise_profile[x].WNR_trigger.lux_index_end

#define GET_PBF_GAIN_TRIGER_START(pointer, x) \
  ((wavelet_denoise_type *)pointer)->noise_profile[x].WNR_trigger.gain_start

#define GET_PBF_GAIN_TRIGER_END(pointer, x) \
  ((wavelet_denoise_type *)pointer)->noise_profile[x].WNR_trigger.gain_end

#define GET_PBF_TRIGGER_TYPE(pointer) \
  ((wavelet_denoise_type *)pointer)->control_denoise

#endif

#define GET_ASF_POINTER(ptr, params, dst_ptr) \
    dst_ptr = &((chromatix_cpp_type *)ptr)->chromatix_ASF_9x9;

#define GET_ASF_7x7_POINTER(ptr, params, dst_ptr) \
    dst_ptr = &((chromatix_cpp_type *)ptr)->chromatix_ASF_7x7;

#define GET_ASF_DOWNSCALE_DATA(ptr, params, dst_ptr) \
    dst_ptr = NULL

#define GET_ASF_UPSCALE_DATA(ptr, params, dst_ptr) \
    dst_ptr = NULL

/* Get asf lux trigger start*/
#define GET_ASF_LUXIDX_TRIGER_START(pointer, x) \
  ((chromatix_ASF_9x9_type *)pointer)->asf_9_9[x].asf_9x9_trigger.lux_index_start

/* Get asf lux trigger end*/
#define GET_ASF_LUXIDX_TRIGER_END(pointer, x) \
  ((chromatix_ASF_9x9_type *)pointer)->asf_9_9[x].asf_9x9_trigger.lux_index_end

/* Get asf gain trigger start*/
#define GET_ASF_GAIN_TRIGER_START(pointer, x) \
  ((chromatix_ASF_9x9_type *)pointer)->asf_9_9[x].asf_9x9_trigger.gain_start

/* Get asf gain trigger end*/
#define GET_ASF_GAIN_TRIGER_END(pointer, x) \
  ((chromatix_ASF_9x9_type *)pointer)->asf_9_9[x].asf_9x9_trigger.gain_end

#define GET_SCALORUP(pointer, index) 0

#define GET_SCALORDOWN(pointer, index) 0

#define GET_UP_FACTOR(pointer, index) 0

#define GET_DOWN_FACTOR(pointer, index) 0

#if (defined(CHROMATIX_307) || defined(CHROMATIX_308) || \
      defined (CHROMATIX_309)|| defined(CHROMATIX_310))
#define GET_ASF_CLAMP_TL_UL(pointer, x) pointer[x].reg_th

#define GET_ASF_CLAMP_TL_LL(pointer, x) pointer[x].reg_tl
#else
#define GET_ASF_CLAMP_TL_UL(pointer, x) 0

#define GET_ASF_CLAMP_TL_LL(pointer, x) 0

#define GET_TNR_HYSTERESIS_TRIGGER_TYPE(pointer) 0

#define GET_TNR_LUXIDX_TRIGGER_START(pointer) 0

#define GET_TNR_LUXIDX_TRIGGER_END(pointer) 0

#define GET_TNR_GAIN_TRIGGER_START(pointer) 0

#define GET_TNR_GAIN_TRIGGER_END(pointer) 0

#define GET_PBF_HYSTERESIS_TRIGGER_TYPE(pointer) 0

#define GET_PBF_HYSTERESIS_LUXIDX_TRIGER_START(pointer) 0

#define GET_PBF_HYSTERESIS_LUXIDX_TRIGER_END(pointer) 0

#define GET_PBF_HYSTERESIS_GAIN_TRIGER_START(pointer) 0

#define GET_PBF_HYSTERESIS_GAIN_TRIGER_END(pointer) 0

#endif


#if (defined(CHROMATIX_310))
  #define ASF_RNR 1
  #define ASF_RNR_IN_ENTRIES ASF_WNR_RADIAL_POINTS
  #define ASF_RADIAL_GAIN_ADJ(asfCore, IDX, ENTRY) \
    ((asfCore)[(IDX)].radial_gain_adj_factor[(ENTRY)])
  #define ASF_RADIAL_ACTIVITY_ADJ(asfCore, IDX, ENTRY) \
    ((asfCore)[(IDX)].radial_activity_adj_factor[(ENTRY)])
  #define ASF_NEG_LUT(asfCore, IDX, ENTRY) \
    ((asfCore)[(IDX)].gain_negative_lut[(ENTRY)])
  #define ASF_RNR_CENTER_H(asfPtr) \
    ((asfPtr)->asf_9_9_reserve.horizontal_center)
  #define ASF_RNR_CENTER_V(asfPtr) \
    ((asfPtr)->asf_9_9_reserve.vertical_center)
  #define ASF_RNR_POINT_LUT(asfPtr, ENTRY) \
    ((asfPtr)->asf_9_9_reserve.radial_pt_table[(ENTRY)])
  #define ASF_USE_SYMM_FILTER_ONLY(asfPtr) \
    ((asfPtr)->use_only_symmetric_filter)
#define ASF_RADIAL_ENABLE(asfPtr) \
      ((asfPtr)->radial_enable)

#else
  #define ASF_RNR 0
  #define ASF_RNR_IN_ENTRIES 4
  #define ASF_RADIAL_GAIN_ADJ(asfCore, IDX, ENTRY) 0
  #define ASF_RADIAL_ACTIVITY_ADJ(asfCore, IDX, ENTRY) 0
  #define ASF_NEG_LUT(asfCore, IDX, ENTRY) 0
  #define ASF_RNR_POINT_LUT(asfPtr, ENTRY) 0
  #define ASF_RNR_CENTER_H(asfPtr) 0
  #define ASF_RNR_CENTER_V(asfPtr) 0
  #define ASF_USE_SYMM_FILTER_ONLY(asfPtr) 0
  #define ASF_USE_SYMM_FILTER_ONLY(asfPtr) 0
  #define ASF_RADIAL_ENABLE(asfPtr) 0

#endif

#endif


