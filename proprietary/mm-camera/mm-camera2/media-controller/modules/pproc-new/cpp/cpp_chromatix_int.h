/* cpp_chromatix_int.h
*
* Copyright (c) 2014-2015 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*/

#include "chromatix_cpp.h"
#if 0

// Not CPP
#define MAX_SETS_FOR_TONE_NOISE_ADJ 6

typedef struct
{
    float start;    // trigger point Start
    float end;      // trigger point End
} trigger_point2_type;

//Not CPP

typedef struct
{
    float   noise_profile_adj_y[HW_WAVELET_LEVELS];
    float   noise_profile_adj_chroma[HW_WAVELET_LEVELS];
    float   denoise_weight_adj_y[HW_WAVELET_LEVELS];
    float   denoise_weight_adj_chroma[HW_WAVELET_LEVELS];
} wavelet_adj_type;

typedef struct
{
    trigger_point2_type         aec_sensitivity_ratio;                              // For CONTROL_AEC_EXP_SENSITIVITY_RATIO
    trigger_point2_type         exp_time_trigger;                                   // For CONTROL_EXP_TIME_RATIO
    wavelet_adj_type            wavelet_adj_data;
} hdr_wavelet_adj_core_type;

typedef struct
{
    int                         enable;
    tuning_control_type         control_hdr_wnr;                                    // CONTROL_AEC_EXP_SENSITIVITY_RATIO or CONTROL_EXP_TIME_RATIO
    hdr_wavelet_adj_core_type   hdr_wavelet_adj_data[MAX_SETS_FOR_TONE_NOISE_ADJ];
} Chromatix_HDR_WNR_adj_type;

typedef struct
{
    trigger_point2_type         aec_sensitivity_ratio;   // For CONTROL_AEC_EXP_SENSITIVITY_RATIO
    trigger_point2_type         exp_time_trigger;        // For CONTROL_EXP_TIME_RATIO
    float                       asf_adj_factor;
} hdr_asf_adj_core_type;

typedef struct
{
    int                         enable;
    tuning_control_type         control_hdr_asf;   // CONTROL_AEC_EXP_SENSITIVITY_RATIO or CONTROL_EXP_TIME_RATIO
    hdr_asf_adj_core_type       hdr_asf_adj_data[MAX_SETS_FOR_TONE_NOISE_ADJ];
} Chromatix_HDR_ASF_adj_type;

typedef struct
{
    trigger_point2_type         aec_sensitivity_ratio;    // For CONTROL_AEC_EXP_SENSITIVITY_RATIO
    trigger_point2_type         drc_gain_trigger;
    wavelet_adj_type            wavelet_adj_data;
} adrc_wavelet_adj_core_type;

typedef struct
{
    int                         enable;
    tuning_control_type         control_adrc_wnr;                                   // CONTROL_AEC_EXP_SENSITIVITY_RATIO or CONTROL_DRC_GAIN
    adrc_wavelet_adj_core_type  adrc_wavelet_adj_data[MAX_SETS_FOR_TONE_NOISE_ADJ];
} Chromatix_ADRC_WNR_adj_type;

#endif
