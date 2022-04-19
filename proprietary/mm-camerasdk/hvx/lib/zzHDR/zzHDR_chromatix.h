/*===========================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/

#ifndef CHROMATIX_HDR_EXTN_H
#define CHROMATIX_HDR_EXTN_H

#include "zzHDR_chromatix_common.h"

typedef struct
{
    int GTM_maxval_th;
    int GTM_key_min_th;
    int GTM_key_max_th;
    float GTM_key_hist_bin_weight;
    int GTM_Yout_maxval;
    int GTM_minval_th ;
    float reserved_1 ;
    float reserved_2 ;
} chromatix_GTM_hdr_reserve;

typedef struct
{
    float GTM_a_middletone;
    float GTM_temporal_w;
    float GTM_middletone_w;
    float GTM_max_percentile ;
    float GTM_min_percentile ;
} chromatix_GTM_hdr_core;

typedef struct
{
    trigger_point2_type     aec_sensitivity_ratio_v2; // For CONTROL_AEC_EXP_SENSITIVITY_RATIO
    chromatix_GTM_hdr_core      gtm_core_v2;
    chromatix_GTM_hdr_reserve   gtm_reserve_v2;
} chromatix_GTM_v2_hdr_exp_type;

typedef struct
{
    trigger_point_linear_type          gtm_v2_trigger;
    chromatix_GTM_v2_hdr_exp_type   gtm_exp_data[MAX_SETS_FOR_TONE_NOISE_ADJ];
} chromatix_GTM_v2_hdr_rgn_type;

typedef struct
{
    int                         gtm_v2_enable;
    int                         gtm_v2_control_enable;
    tuning_control_type         control_gtm_v2;
    chromatix_GTM_v2_hdr_rgn_type   gtm_v2[MAX_SETS_FOR_GTM_EXP_ADJ];  //6 sets
} chromatix_GTM_v2_hdr_type;

typedef struct
{
    // GTM 3 light region and 6 region sensitivity based data
    //=========================================================================
    chromatix_GTM_v2_hdr_type               chromatix_gtm_v2;
}chromatix_zzhdr_post_processing_type;

typedef struct
{
    chromatix_zzhdr_post_processing_type chromatix_post_processing;
}chromatix_hdr_ext_type;
#endif
