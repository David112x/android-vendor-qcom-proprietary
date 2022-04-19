/*==========================================================
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
==========================================================*/
#ifndef CHROMATIX_CPP_STRIPPED_H
#define CHROMATIX_CPP_STRIPPED_H

#include "chromatix_cpp.h"

typedef struct
{
  // ASF on/off flag
  int asf_9x9_enable;
  chromatix_asf_9_9_core_type  asf_9_9;
  /* soft_focus_degree */
  filter_sharpen_degree_type   soft_focus_degree_9_9;

  float asf_9_9_sharp_min_ds_factor; //default 0.5
  float asf_9_9_sharp_max_ds_factor; //default 4.0
  float asf_9_9_sharp_max_factor;   //
} chromatix_ASF_9x9_stripped_type;


typedef struct
{
  int WNR_luma_enable; //0x303
  int WNR_chroma_enable; //0x303
  int WNR_control_enable; //0x303
  int wavelet_enable_index;
  tuning_control_type control_denoise;
  ReferenceNoiseProfile_type noise_profile;
} wavelet_denoise_stripped_type;

typedef struct
{
  wavelet_denoise_stripped_type wavelet_denoise_HW_420;
  NoiseProfile_Upscaling_type wavelet_upscale_adj; //zoom
  NoiseProfile_Downscaling_type wavelet_downscale_adj; //zoom
} chromatix_wavelet_stripped_type;

typedef struct{
  int WNRenable;
  WNR_noise_profile_type  referenceNoiseProfileData;
 } chromatixWNRDenoise_stripped_type;


typedef struct {
    unsigned char is_compressed;
    unsigned short revision_number;
    chromatix_ASF_9x9_stripped_type chromatix_ASF_9x9;
    chromatix_wavelet_stripped_type chromatix_wavelet;
    chromatixWNRDenoise_stripped_type chromatix_temporal_denoise;
} chromatix_cpp_stripped_type;

#endif  // #ifndef CHROMATIX_CPP_STRIPPED_H
