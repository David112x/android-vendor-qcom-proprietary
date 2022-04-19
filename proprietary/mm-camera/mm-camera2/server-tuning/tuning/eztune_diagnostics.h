/***************************************************************************
 * Copyright (c) 2011-2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ***************************************************************************/
#ifndef __EZTUNE_DIAGNOSTICS_H__
#define __EZTUNE_DIAGNOSTICS_H__
#include <inttypes.h>
#include "eztune_vfe_diagnostics.h"
#include "chromatix_cpp.h"

#if ((defined(CHROMATIX_307) || defined(CHROMATIX_306) || defined(CHROMATIX_308) || \
  defined(CHROMATIX_308E) || defined(CHROMATIX_309) || defined(CHROMATIX_310)) && \
  defined(CAMERA_USE_CHROMATIX_HW_WNR_TYPE))
  #define DENOISE_NUM_PROFILES HW_WAVELET_LEVELS
  #define BILATERAL_LAYERS HW_WNR_BILATERAL_FILTER_LEVELS
#else
  #define DENOISE_NUM_PROFILES WAVELET_LEVEL + 1
  #define BILATERAL_LAYERS WAVELET_LEVEL
#endif

typedef struct {
  uint8_t linearenable;
  uint8_t linearstepsize;
  uint8_t ringenable;
  uint8_t ringstepsize;
  uint8_t deffocenable;
  uint8_t movfocenable;
  uint8_t movfocdirection;
  uint32_t movfocsteps;
} ez_af_tuning_params_t;

typedef struct {
  int32_t bilateral_scalecore0[BILATERAL_LAYERS];
  int32_t bilateral_scalecore1[BILATERAL_LAYERS];
  int32_t bilateral_scalecore2[BILATERAL_LAYERS];
  int32_t noise_thresholdcore0[DENOISE_NUM_PROFILES];
  int32_t noise_thresholdcore1[DENOISE_NUM_PROFILES];
  int32_t noise_thresholdcore2[DENOISE_NUM_PROFILES];
  int32_t weightcore0[DENOISE_NUM_PROFILES];
  int32_t weightcore1[DENOISE_NUM_PROFILES];
  int32_t weightcore2[DENOISE_NUM_PROFILES];

  int32_t y_noise_thresh[DENOISE_NUM_PROFILES];
  int32_t y_weight1[DENOISE_NUM_PROFILES];
  int32_t y_bilat_scale[DENOISE_NUM_PROFILES];
  int32_t cb_noise_thresh[DENOISE_NUM_PROFILES];
  int32_t cb_weight1[DENOISE_NUM_PROFILES];
  int32_t cb_bilat_scale[DENOISE_NUM_PROFILES];
  int32_t cr_noise_thresh[DENOISE_NUM_PROFILES];
  int32_t cr_weight1[DENOISE_NUM_PROFILES];
  int32_t cr_bilat_scale[DENOISE_NUM_PROFILES];
} wavelet_t;

typedef struct {
  int32_t smoothpercent;
  uint8_t neg_abs_y1;
  uint8_t dyna_clamp_en;
  uint8_t sp_eff_en;
  int16_t clamp_hh;
  int16_t clamp_hl;
  int16_t clamp_vh;
  int16_t clamp_vl;
  int32_t clamp_scale_max;
  int32_t clamp_scale_min;
  uint16_t clamp_offset_max;
  uint16_t clamp_offset_min;
  uint32_t nz_flag;
  int32_t sobel_h_coeff[16]; // f1
  int32_t sobel_v_coeff[16]; // f2
  int32_t hpf_h_coeff[16];   // f3
  int32_t hpf_v_coeff[16];   // f4
  int32_t lpf_coeff[16];
  int32_t lut1[24];
  int32_t lut2[24];
  int32_t lut3[12];
  bool  valid;
} asfsharpness7x7_t;

typedef struct {
  int32_t  trigger_gainStart;
  int32_t  trigger_gainEnd;
  int32_t  trigger_lutIndexStart;
  int32_t  trigger_lutIndexEnd;
  int32_t  sp; //smoothPct???
  int32_t  horizontal_nz[8];
  int32_t  vertical_nz[8];
  int16_t  sobel_H_coeff[25];
  int16_t  sobel_se_diagonal_coeff[25];
  int16_t  hpf_h_coeff[25];
  int16_t  hpf_se_diagonal_coeff[25];
  int16_t  hpf_symmetric_coeff[15];
  int16_t  lpf_coeff[15];
  int16_t  activity_lpf_coeff[6];
  int16_t  activity_band_pass_coeff[6];
  float    activity_normalization_lut[256];
  float    weight_modulation_lut[256];
  uint32_t soft_threshold_lut[256];
  float    gain_lut[256];
  float    gain_weight_lut[256];
  float    gain_cap;
  uint32_t gamma_corrected_luma_target;
  int32_t  en_dyna_clamp;
  float    smax;
  int32_t  omax;
  float    smin;
  int32_t  omin;
  int32_t  clamp_UL;
  int32_t  clamp_LL;
  float    perpendicular_scale_factor;
  uint32_t max_value_threshold;
  float    norm_scale;
  uint32_t activity_clamp_threshold;
  uint8_t  L2_norm_en;
  float    median_blend_upper_offset;
  float    median_blend_lower_offset;
  bool     valid;
} asfsharpness9x9_t;

typedef struct {
  /* TODO: update data types */
  int32_t current_reconenable;
  int32_t current_macenable;
  int32_t current_reconlinearmode;
  int32_t current_maclinearmode;
  int32_t current_reconedgelpftap0;
  int32_t current_macmotiondilation;
  int32_t current_macsmoothenable;
  int32_t current_reconflatregionthresh;
  int32_t current_reconminfactor;
  int32_t current_reconhedgethresh1;
  int32_t current_reconhedgelogthreshdiff;
  int32_t current_reconmotionthresh1;
  int32_t current_reconmotionlogthreshdiff;
  int32_t current_recondarkthresh1;
  int32_t current_recondarklogthreshdiff;
  int32_t current_macmotion0thresh1;
  int32_t current_macmotion0thresh2;
  int32_t current_motion0dt0;
  int32_t current_macmotionstrength;
  int32_t current_maclowlightstrength;
  int32_t current_maclowlightthresh1;
  int32_t current_maclowlightlogthreshdiff;
  int32_t current_macbrightthresh1;
  int32_t current_macbrightlogthreshdiff;
  int32_t current_macsmoothtap0;
  int32_t current_macsmooththresh1;
  int32_t current_macsmoothlogthreshdiff;
  int32_t pixelpattern;
  int32_t enable_3d;
  int32_t leftpanelwidth;
  int32_t firstfield;
  int32_t msbalign;
  int32_t expratio;
  int32_t expratiorecip;
  int32_t rgwbgainratio;
  int32_t bgwbgainratio;
  int32_t grwbgainratio;
  int32_t gbwbgainratio;
  int32_t macsqrtanaloggain;
} video_hdr_t;

typedef struct {
  config_cntrl_t control_asf7x7;
  asfsharpness7x7_t prev_asf7x7;
  asfsharpness7x7_t snap_asf7x7;

  config_cntrl_t control_asf9x9;
  asfsharpness9x9_t prev_asf9x9;
  asfsharpness9x9_t snap_asf9x9;

  config_cntrl_t control_wnr;
  wavelet_t prev_wnr;
  wavelet_t snap_wnr;

  video_hdr_t video_hdr;

} ez_pp_params_t;

typedef enum {
  EZ_AF_LOADPARAMS,
  EZ_AF_LINEARTEST_ENABLE,
  EZ_AF_RINGTEST_ENABLE,
  EZ_AF_DEFFOCUSTEST_ENABLE,
  EZ_AF_MOVFOCUSTEST_ENABLE,
} aftuning_optype_t;

typedef enum {
  EZ_STATUS,
  EZ_AEC_ENABLE,
  EZ_AEC_TESTENABLE,
  EZ_AEC_LOCK,
  EZ_AEC_FORCESNAPEXPOSURE,
  EZ_AEC_FORCESNAPLINECOUNT,
  EZ_AEC_FORCESNAPGAIN,
  EZ_AEC_FORCEPREVEXPOSURE,
  EZ_AEC_FORCEPREVLINECOUNT,
  EZ_AEC_FORCEPREVGAIN,
  EZ_AWB_MODE,
  EZ_AWB_ENABLE,
  EZ_AWB_LOCK,
  EZ_AWB_FORCE_DUAL_LED_IDX,
  EZ_AF_ENABLE,
} aaa_set_optype_t;

typedef enum {
  EZ_JPEG_SNAPSHOT,
  EZ_RAW_SNAPSHOT,
} action_set_optype_t;

typedef enum {
  PP_MODULE_ALL,
  PP_MODULE_ASF,
  PP_MODULE_WNR,
} pp_module_t;

typedef enum {
  EZ_MISC_PREVIEW_RESOLUTION,
  EZ_MISC_SNAPSHOT_RESOLUTION,
  EZ_MISC_CURRENT_RESOLUTION,
  EZ_MISC_SENSOR_FORMAT,
  EZ_MISC_SENSOR_TYPE,
  EZ_MISC_SENSOR_FULLWIDTH,
  EZ_MISC_SENSOR_FULLHEIGHT,
  EZ_MISC_SENSOR_QTRWIDTH,
  EZ_MISC_SENSOR_QTRHEIGHT,
  EZ_MISC_SENSOR_PIXELCLKFREQ,
  EZ_MISC_SENSOR_PIXELSPERLINE,
  EZ_MISC_SENSOR_LENSSPEC,
} miscoptype_t;

#endif /* __EZTUNE_DIAGNOSTICS_H__ */
