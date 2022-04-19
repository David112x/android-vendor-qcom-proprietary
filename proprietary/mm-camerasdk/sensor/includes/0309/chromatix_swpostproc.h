//=============================================================================
// Copyright (c) 2015 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//=============================================================================

#ifndef CHROMATIX_SW_POSTPROC_H
#define CHROMATIX_SW_POSTPROC_H

#include "chromatix.h"

//=============================================================================
//=============================================================================
//=============================================================================
//  SW Algorithms ( CAC2 + RNR1  )
//=============================================================================
//=============================================================================
//=============================================================================

#define MAX_HYSTERESIS_LIGHT_REGIONS    2   // Bright to Normal Light and Normal Light to Low Light Transitions
#define RNR_LUT_SIZE                    164 // (Radius)/16-> for 4208x3120 , size is 164 = sqrt ( pow(2104,2)+pow(1560,2))
#define MAX_SIGMA_TABLES                3

typedef struct
{
    trigger_point_type  rnr_trigger;
    unsigned char       sampling_factor;        // Def 2,2,4,4,8,8
    float               center_noise_sigma;     // center noise
    float               center_noise_weight;    // default to 1.0
    float               weight_order;           // Def 2,2,1.5,1.5,1.0,1.0
} chromatix_radial_noise_reduction1_type;

typedef struct
{
    int                                     rnr_enable;
    int                                     rnr_control_enable;
    tuning_control_type                     control_rnr;            //DEFAULT 0 lux idx
    int                                     lut_size;
    trigger_point_type                      hysteresis_point[MAX_HYSTERESIS_LIGHT_REGIONS];
    trigger_point_type                      sigma_lut_trigger[MAX_SIGMA_TABLES - 1];
    float                                   sigma_lut[MAX_SIGMA_TABLES][RNR_LUT_SIZE];
    chromatix_radial_noise_reduction1_type  rnr_data[MAX_LIGHT_TYPES_FOR_SPATIAL];
} chromatix_RNR1_type;

typedef struct
{
    int                 cac2_enable;
    trigger_point_type  cac2_trigger;
    int                 y_spot_thr_low;     //NEW Detection_TH1;        //
    int                 y_spot_thr_up;      //NEW Detection_TH2;        //
    int                 y_saturation_thr;   //NEW Detection_TH3;        //
    int                 c_saturation_thr;   //NEW Verification_TH1;     //
    int                 y_spot_thr_weight;  //NEW Correction_Strength;  //u8Q6
    int                 c_spot_thr;         //NEW
} chromatix_chroma_aliasing_correction2;

typedef struct
{
    int                                     cac2_control_enable;
    tuning_control_type                     control_cac2;           //DEFAULT 0 lux idx
    chromatix_chroma_aliasing_correction2   cac2_data[MAX_LIGHT_TYPES_FOR_SPATIAL];
} chromatix_CAC2_type;

//=============================================================================
//  Variable name : float still_brIntensity;
//  Controls amount of brightening. Range: 0 ~ 1.
//  Larger value means brighter output.
//  Feature : Parameters for StillMore feature
//
//  Variable name : float still_brColor;
//  Controls amount of color after brightening. Range: 0 ~ 1.
//  Larger value means more colorful output.
//  Feature : Parameters for StillMore feature
//=============================================================================
typedef struct
{
    float still_brIntensity;
    float still_brColor;
} chromatix_Stillmore_type;

//=============================================================================
//  Variable name : float see_brIntensity
//   Controls amount of brightening. Range: 0 ~ 1.
//   Larger value means brighter output.
//   NOTE: only takes effect when enableLTM == true.
//  Feature : Parameters for SeeMore feature
//
//  Variable name : float see_brColor
//   Controls amount of color after brightening. Range: 0 ~ 1.
//   Larger value means more colorful output.
//   NOTE: only takes effect when enableLTM == true.
//  Feature : Parameters for SeeMore feature
//=============================================================================
typedef struct
{
    float see_brIntensity;
    float see_brColor;
} chromatix_Seemore_type;

typedef struct
{
    trigger_point_type          mod_trigger;
    chromatix_Stillmore_type    chromatix_stillmore_data;
    chromatix_Seemore_type      chromatix_seemore_data;
    float                       ext_data[20];
} chromatix_adv_sw_mod_core_type;

//=============================================================================
//  Variable name : int  stillmore_enable
//   True: enable StillMore feature
//   False: otherwise
//  Feature : Parameters for StillMore feature
//
//  Variable name : int  seemore_LTM_enable
//   True: enable LTM brightening
//   False: otherwise
//  Feature : Parameters for SeeMore feature
//
//  Variable name : int seemore_TNR_enable
//   True: enable temporal noise reduction
//   False: otherwise
//  Feature : Parameters for SeeMore feature
//
//  Variable name : stillmore_hyst_trigger/seemore_hyst_trigger
//   Hysteresis trigger to enable StillMore/SeeMore feature. trigger >= trigger_end,
//   will enable the feature, else will be disabled and once enabled, feature will be enabled
//  until trigger < trigger_start.
//  stillmore_hyst_trigger for stillmore feature & seemore_hyst_trigger for seemore features
//=============================================================================
typedef struct
{
    int                                     stillmore_enable;
    int                                     seemore_LTM_enable;
    int                                     seemore_TNR_enable;
    tuning_control_type                     control_mod;
    trigger_point_type                      stillmore_hyst_trigger;
    trigger_point_type                      seemore_hyst_trigger;
    chromatix_adv_sw_mod_core_type          mod_data[MAX_LIGHT_TYPES_FOR_SPATIAL];
} chromatix_adv_sw_feature_type;

//=============================================================================
//=============================================================================
//  CHROMATIX HEADER definition
//=============================================================================
//=============================================================================

typedef struct
{
    //=========================================================================
    // Chromatix header version info (MUST BE THE FIRST PARAMETER)
    //=========================================================================
    ChromatixVersionInfoType        chromatix_version_info;

    //=========================================================================
    //          Radial noise1 reduction
    //=========================================================================
    chromatix_RNR1_type             chromatix_radial_noise1_reduction;

    //=========================================================================
    //          CAC2 data
    //=========================================================================
    chromatix_CAC2_type             chromatix_CAC2_data;

    //=========================================================================
    //          Advanced SW features
    //=========================================================================
    chromatix_adv_sw_feature_type   chromatix_adv_sw_data;
} chromatix_sw_postproc_type;

#endif
