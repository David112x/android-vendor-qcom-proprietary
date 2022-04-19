//======================================================================
// Copyright (c) 2016 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//======================================================================

#ifndef CHROMATIX_CPP_H
#define CHROMATIX_CPP_H

#include "chromatix.h"

//=============================================================================
//                      CONSTANTS
//=============================================================================

#define CHROMATIX_REVISION_CPP          1   // Must match chromatix_version_info.revision_number in header data file

#define HW_WAVELET_LEVELS               6
#define HW_WNR_BILATERAL_FILTER_LEVELS  4

#define UP_SCALING_LEVELS               4
#define DOWN_SCALING_LEVELS             4 // zoom

#define EXT_CPP_ENTRIES                 1380

//=============================================================================
// DATA TYPES
//=============================================================================

typedef enum
{
    ASF_7x7_LOW_LIGHT = 0,
    ASF_7x7_NORMAL_LIGHT,
    ASF_7x7_BRIGHT_LIGHT,
    ASF_7x7_MAX_LIGHT
} ASF_7x7_light_type;

typedef struct
{
    // ASF on/off flag moved this to upper struct 0x303
    // unsigned char asf_en;

    //0x303
    trigger_point_type  asf_7x7_trigger;

    //*** VFE4 ***
    int                 checksum_flag;  // F5 | F4 | F3 | F2 | F1 ( 2 bit for each filter , Lower 10 bits valid )
                                        // 00 for Check Sum Disable ,
                                        // 01 for Check Sum Enable, Cheksum Value =0
                                        // 11 for Check Sum Enable, CheckSum Value =1
                                        //  1 for enable check sum , 0 for disable )
    // 3x3 cross-type median filter
    int                 sp;             // 3x3 smooth percentage, [0, 16], Q4

    // Special effects removed in 0x303
    //   unsigned char en_sp_eff;
    //   unsigned char  neg_abs_y1;
    int                 nz[3];          //[0] for F1, [1] for F2, [2] for F3 and F4.       // Order: nz[7]nz[6]nz[5]nz[4]nz[3]nz[2]nz[1]nz[0]

    // 7x7 filters
    // The last line is not median filtered to save 1 line buffer.
    // Alternated kernel symmatry by (negation & zero)'s according to zones:
    // nz[0]          |nz[1]|          nz[2]
    //        0  1  2 |   3 |  2  1  0
    //        4  5  6 |   7 |  6  5  4
    //        8  9 10 |  11 | 10  9  8
    // ---------------+-----+---------------
    // nz[7] 12 13 14 |  15 | 14 13 12 nz[3]
    // ---------------+-----+---------------
    //        8  9 10 |  11 | 10  9  8
    //        4  5  6 |   7 |  6  5  4
    //        0  1  2 |   3 |  2  1  0
    // nz[6]          |nz[5]|          nz[4]
    //
    float               f1[25];         // 9x9 H Sobel filter, 10sQ10 in quadrants, 11sQ10 in axes, 12sQ10 at center
    float               f2[25];         // 9x9 V Sobel filter, 10sQ10 in quadrants, 11sQ10 in axes, 12sQ10 at center
    float               f3[25];         // 9x9 H Wiener HPF, 10sQ10 in quadrants, 11sQ10 in axes, 12sQ10 at center
    float               f4[25];         // 9x9 V Wiener HPF, 10sQ10 in quadrants, 11sQ10 in axes, 12sQ10 at center
    float               f5[25];         // 9x9 Gaussian LPF, 10sQ10 in quadrants, 11sQ10 in axes, 12sQ10 at center

    // H & V sharpening LUTs, 24 entries
    float               lut1[24];       // Linear interpolated 16s Q6/Q5 entries
    float               lut2[24];       // Linear interpolated 16s Q6/Q5 entries

    // Dynamic clamping
    int                 en_dyna_clamp;
    float               smax;           // Dynamic clamping positive level scaling, 7uQ4
    int                 omax;           // Dynamic clamping positive offset level, 7u
    float               smin;           // Dynamic clamping negative level scaling, 7uQ4
    int                 omin;           // Dynamic clamping negative offset level, 7u
    int                 reg_hh;         // Manual fixed H positive clamping level, 9s
    int                 reg_hl;         // Manual fixed H negative clamping level, 9s
    int                 reg_vh;         // Manual fixed V positive clamping level, 9s
    int                 reg_vl;         // Manual fixed V negative clamping level, 9s
    int                 reg_th;         // Manual fixed Total positive clamping level, 9s
    int                 reg_tl;         // Manual fixed Total negative clamping level, 9s

    // LP & HP output combination LUT, 12 entries
    float               lut3[12];       // Linear interpolated 15s Q8/Q6 entries
} chromatix_asf_7_7_core_type;

// Variable name: hpf_h_coeff[25].
// Array to store horizontal sharpening filter coefficients.
// Default value:
// Applicable Chipset versions: 8084,8994,8992 and 8996
// Data range: Center coeff([24]) -2047 to 2047, axes coeff([4], [9], [14], [19], [20], [21],
//        [22], [23]) -1023 to 1023, quadrant coeff([0], [1], [2], [3], [5], [6], [7], [8],
//        [10], [11], [12], [13], [15], [16], [17], [18]) -511 to 511.  All are Q10 numbers.
//
// Variable name: hpf_se_diagonal_coeff[25].
// Array to store downward sharpening filter coefficients.
// Applicable Chipset versions: 8084,8994,8992 and 8996
// Default value:
// Data range: Center coeff([24]) -2047 to 2047, axes coeff([0], [8], [9], [15], [16], [20],
//        [21], [23]) -1023 to 1023, quadrant coeff([1], [2], [3], [4], [5], [6], [7], [10],
//        [11], [12], [13], [14], [17], [18], [19], [22]) -511 to 511.  All are Q10 numbers.
//
// Variable name: gain_negative_lut[64].
// Sharpening gain negative lookup table.
// Default value:
// Applicable Chipset versions: 8998
// Data range: 0 to 7.96
// Effect:
//
// Variable name: radial_activity_correction_factor[4].
// correction factor for activity
// Applicable Chipset versions: 8998
// Default value: 1.0f(11uQ8)
// Data range: 0.0f - 7.99f ( Monotonically increasing )
//
// Variable name: radial_gain_correction_factor[4].
// gain correction factor
// Applicable Chipset versions: 8998
// Default value: 1.0f (11uQ8)
// Data range: 0.0f - 7.99f ( Monotonically increasing )
#define ASF_WNR_RADIAL_POINTS 4

typedef struct
{
    trigger_point_type  asf_9x9_trigger;

    // 3x3 cross-type median filter
    int                 sp;                             // 3x3 smooth percentage

    // 9x9 filters
    int                 horizontal_nz[8];               // nz flag for both horizontal and SE filters (Sobel and hpf)
    int                 vertical_nz[8];                 // nz flag for both vertical and NE diagonal filters (Sobel and hpf)

    //0x303 coeff use fixed pt with Q number to avoid rounding issues from SW
    short               sobel_H_coeff[25];              // 9x9 Sobel horizontal filter (one-fourth coefficients), vertical filter is mapped from this array
    short               sobel_se_diagonal_coeff[25];    // 9x9 Sobel SE filter (one-fourth coefficients), NE filter is mapped from this array
    short               hpf_h_coeff[25];                // 9x9 horizontal high-pass filter (one-fourth coefficients), vertical high-pass filter is mapped from this array
    short               hpf_se_diagonal_coeff[25];      // 9x9 SE high-pass filter (one-fourth coefficients), NE high-pass filter is mapped from this array
    short               hpf_symmetric_coeff[15];        // 9x9 symmtrical high-pass filter (one-eighth coefficients)
    short               lpf_coeff[15];                  // 9x9 low-pass filter (one-eighth coefficients)
    short               activity_lpf_coeff[6];          // 5x5 activity low-pass filter (one-eighth coefficients)
    short               activity_band_pass_coeff[6];    // 5x5 activity band-pass filter (one-eighth coefficients)

    // LUTs, 64 entries
    float               activity_normalization_lut[64]; // level-based normalization LUT to caculate normalized activity

    float               weight_modulation_lut[64];      // normalized activity-based LUT to control the gain of sharpened value
    unsigned int        soft_threshold_lut[64];         // level-based soft-thresholding LUT to determine the final sharpened value

    float               gain_lut[64];                   // level-based sharpening gain LUT
    float               gain_weight_lut[256];           //[64]; // normalized activity-based sharpening gain LUT

    float               gain_cap;                       // upper cap of sharpening gain
    unsigned int        gamma_corrected_luma_target;    // luma target after gamma to control level-based sharpening

    // Dynamic clamping
    int                 en_dyna_clamp;                  // Dynamic clamping enable
    float               smax;                           // Dynamic clamping positive level scaling
    int                 omax;                           // Dynamic clamping positive offset level
    float               smin;                           // Dynamic clamping negative level scaling
    int                 omin;                           // Dynamic clamping negative offset level

    // Static clamping
    int                 clamp_UL;                       // Manual fixed positive clamping level
    int                 clamp_LL;                       // Manual fixed negative clamping level

    // processing control parameters
    float               perpendicular_scale_factor;     // threshold for the ratio between two orthogonal directions
    unsigned int        max_value_threshold;            // maximum absolute value threshold for pixel gridients in four directions

    float               norm_scale;                     // scale factor of 5x5 band-pass filter output (activity)
    unsigned int        activity_clamp_threshold;       // static clamp of 5x5 band-pass filter output (activity)
    unsigned char       L2_norm_en;                     // enable L2 norm for 5x5 band-pass filter output (activity)
                                                        // 0: L1 norm (absolute value), 1: L2 norm (squared value)

    float               median_blend_upper_offset;      // upper offset of median blend
    float               median_blend_lower_offset;      // lower offset of median blend
    // 0x0310 Updates
    float               gain_negative_lut[64];
    float               radial_activity_adj_factor[ASF_WNR_RADIAL_POINTS];
    float               radial_gain_adj_factor[ASF_WNR_RADIAL_POINTS];
} chromatix_asf_9_9_core_type;

typedef struct
{
    float               radial_pt_table[ASF_WNR_RADIAL_POINTS]; // 0 for center and 1 for diagonal
    unsigned int        horizontal_center;
    unsigned int        vertical_center;
} chromatix_asf_9_9_reserve_type;

typedef struct
{
    int                             asf_7x7_en;                             //0x303 moved from chromatix_asf_7_7_type
    int                             asf_7x7_control_enable;                 //0x303
    tuning_control_type             control_asf_7x7;                        //DEFAULT 1 GAIN default value for VF and snapshot is same

    chromatix_asf_7_7_core_type     asf_7_7[MAX_LIGHT_TYPES_FOR_SPATIAL];   // SnapshotFilter
    /* soft_focus_degree */
    filter_sharpen_degree_type      soft_focus_degree_7_7;                  // SoftFocusDegree

    float                           asf_7_7_sharp_min_ds_factor;            //default 0.5
    float                           asf_7_7_sharp_max_ds_factor;            //default 4.0
    float                           asf_7_7_sharp_max_factor;               //
} chromatix_ASF_7x7_type;

// Variable name: use_only_symmetric_filter
// symmetric filter output only for low power mode
// Default value: 1
// Chipset version: 8998 and later
//0x303
typedef struct
{
    // ASF on/off flag
    int                             asf_9x9_enable;                 // enable flag
    int                             asf_9x9_control_enable;
    int                             use_only_symmetric_filter;
    int                             radial_enable;
    tuning_control_type             control_asf_9x9;                // DEFAULT 1 GAIN default value for VF and snapshot is same
    trigger_point_type              asf_radial_hysteresis_trigger;

    chromatix_asf_9_9_core_type     asf_9_9[MAX_LIGHT_TYPES_FOR_SPATIAL];
    chromatix_asf_9_9_reserve_type  asf_9_9_reserve;

    /* soft_focus_degree */
    filter_sharpen_degree_type      soft_focus_degree_9_9;          // SoftFocusDegree

    float                           asf_9_9_sharp_min_ds_factor;    //default 0.5
    float                           asf_9_9_sharp_max_ds_factor;    //default 4.0
    float                           asf_9_9_sharp_max_factor;       //
} chromatix_ASF_9x9_type;

#define WNR_LEVEL_TABLE_ENTRIES (32+1)

// Variable name: PaddingMode_y/Chroma
// Padding mode per wavelet level for Luma/Chroma
// Applicable Chipset versions: 8998
// Default value: 0 for Luma and 1 for Chroma
// Data range: 0 ( replication ) ,1(pad with gray )
//
// Variable name: transition_enable_luma/Chroma
// transition enable for Luma/Chroma
// Applicable Chipset versions: 8998
// Default value: 1 for Luma and 0 for Chroma
// Effect: To improve quality on smooth intensity transition areas
//
// Variable name: radial_and_level_weight_y_enable
// Radial and level correction weight correction enable flag for Luma
// Applicable Chipset versions: 8998
// Default value: 0 for all 4 levels
// Data range: 0,1
//
// Variable name: radial_weight_chroma_enable
// Radial weight correction enable flag for Chroma
// Applicable Chipset versions: 8998
// Default value: 0 for all 4 levels
// Data range: 0,1
//
// Variable name: level_edgesoftness_adj_factor[33]
// Level based bilateral scale adjustment factor
// Applicable Chipset versions: 8998
// Default value: 0
// Data range: 0 to 0.995
// Notes: (lut[i+1]-lut[i]) should be less than 0.5
//
// Variable name: level_weight_adj_factor[33]
// Level based weight adjustment factor
// Applicable Chipset versions: 8998
// Default value: 0
// Data range: 0 ( no level correction ) to 0.995 ( max correction )
// Notes: (lut[i+1]-lut[i]) should be less than 0.5
//
// Variable name: radial_edgesoftness_adj_factor_y/chroma
// Radial based bilateral scale adjustment factor  for luma/Chroma
// Applicable Chipset versions: 8998
// Default value: 0
// Data range: 0 ( no radial correction ) to 0.995 ( max correction )
//
// Variable name: radial_weight_adj_factor_y/chroma
// Radial based weight adjustment factor
// Applicable Chipset versions: 8998
// Default value: 0
// Data range: 0 ( no radial weight adjustment ) to 0.995 ( max correction )
//
// Variable name: radial_pt_table[ASF_WNR_RADIAL_POINTS]
// Anchor point radius percentage square table for radial processing, each entry holds square of radius percentage
// (i.e. radius/distance_from_center_to_corner ). For example: value of 0.25 means 1st control point is half
// way from image center
// Applicable Chipset versions: 8998
// Default value: 0.2500, 0.4444, 0.6944, 1.00
// Data range: 0  to 1.000
typedef struct
{
    trigger_point_type  WNR_trigger;
    float               noise_profile_y[HW_WAVELET_LEVELS];
    float               noise_profile_cb[HW_WAVELET_LEVELS];
    float               noise_profile_cr[HW_WAVELET_LEVELS];
    float               denoise_scale_y[HW_WAVELET_LEVELS];
    float               denoise_scale_chroma[HW_WAVELET_LEVELS];
    float               denoise_edge_softness_y[HW_WNR_BILATERAL_FILTER_LEVELS];
    float               denoise_edge_softness_chroma[HW_WNR_BILATERAL_FILTER_LEVELS];
    float               denoise_weight_y[HW_WAVELET_LEVELS];
    float               denoise_weight_chroma[HW_WAVELET_LEVELS];

    int                 PaddingMode_y[HW_WNR_BILATERAL_FILTER_LEVELS];
    int                 PaddingMode_chroma[HW_WNR_BILATERAL_FILTER_LEVELS];
    unsigned char       radial_and_level_weight_y_enable[HW_WNR_BILATERAL_FILTER_LEVELS];
    unsigned char       radial_weight_chroma_enable[HW_WNR_BILATERAL_FILTER_LEVELS];
    float               level_edgesoftness_adj_factor[WNR_LEVEL_TABLE_ENTRIES];
    float               level_weight_adj_factor[WNR_LEVEL_TABLE_ENTRIES];
    float               radial_edgesoftness_adj_factor_y[ASF_WNR_RADIAL_POINTS];
    float               radial_edgesoftness_adj_factor_chroma[ASF_WNR_RADIAL_POINTS];
    float               radial_weight_adj_factor_y[ASF_WNR_RADIAL_POINTS];
    float               radial_weight_adj_factor_chroma[ASF_WNR_RADIAL_POINTS];
} hw_WNR_noiseprofile_type;

typedef struct
{
    float noise_profile_adj_y[HW_WAVELET_LEVELS];
    float noise_profile_adj_chroma[HW_WAVELET_LEVELS];
    float denoise_weight_adj_y[HW_WAVELET_LEVELS];
    float denoise_weight_adj_chroma[HW_WAVELET_LEVELS];
    float scaling_level;
} hw_wavelet_scale_type;

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
    tuning_control_type         control_hdr_asf;   // CONTROL_EXPOSURE_RATIO or CONTROL_EXP_TIME_RATIO
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
    tuning_control_type         control_adrc_wnr;                                   // CONTROL_EXPOSURE_RATIO or CONTROL_DRC_GAIN
    adrc_wavelet_adj_core_type  adrc_wavelet_adj_data[MAX_SETS_FOR_TONE_NOISE_ADJ];
} Chromatix_ADRC_WNR_adj_type;

typedef struct
{
    hw_wavelet_scale_type   wnr_upscale_data[UP_SCALING_LEVELS];
    hw_wavelet_scale_type   wnr_downscale_data[DOWN_SCALING_LEVELS];
    hw_wavelet_scale_type   cpp_wnr_downscale_data[DOWN_SCALING_LEVELS];
} Chromatix_wavelet_scaling_type;

typedef struct
{
    int             transition_enable_luma[HW_WNR_BILATERAL_FILTER_LEVELS];
    int             transition_enable_chroma[HW_WNR_BILATERAL_FILTER_LEVELS];
    float           radial_pt_table[ASF_WNR_RADIAL_POINTS];                     // 0 for center and 1 for diagonal
    unsigned int    horizontal_center;
    unsigned int    vertical_center;
} Chromatix_wavelet_reserve_type;

// Variable name: wavelet_dec_filt_enable
// Enable Flag for 5th and 6th layer of wavelet
// Applicable Chipset versions: 8996, 8998 and later
// Default value: 1
// Data range: 0,1
//
// Variable name: radial_and_level_y_enable
// Enable Radial and Level filter enable flag for Y channel 4 layers
// Applicable Chipset versions: 8998 and later
// Default value: 1,1,1,1
// Data range: 0,1
//
// Variable name: radial_chroma_enable
// Enable Radial filter enable flag for Chroma channel 4 layers
// Applicable Chipset versions: 8998 and later
// Default value: 1,1,1,1
// Data range: 0,1
//
// Variable name: wnr_radial_hysteresis_trigger
// Hysteresis trigger for radial and level feature enabling
// Applicable Chipset versions: 8998 and later
//
// Variable name: wnr_dec_filt_hysteresis_trigger
// Hysteresis trigger for decimated 5th and 6th layer enabling
// Applicable Chipset versions: 8996 and later
typedef struct
{
    int                             WNR_luma_enable;
    int                             WNR_chroma_enable;
    int                             WNR_control_enable;
    int                             wavelet_enable_index;
    tuning_control_type             control_denoise;
    int                             wavelet_dec_filt_enable;
    unsigned char                   radial_and_level_y_enable[HW_WNR_BILATERAL_FILTER_LEVELS];
    unsigned char                   radial_chroma_enable[HW_WNR_BILATERAL_FILTER_LEVELS];
    trigger_point_type              wnr_radial_level_hysteresis_trigger;
    trigger_point_type              wnr_dec_filt_hysteresis_trigger;

    hw_WNR_noiseprofile_type        noise_profile[MAX_LIGHT_TYPES_FOR_SPATIAL];
    Chromatix_wavelet_reserve_type  hw_wavelet_reserve_data;
    Chromatix_wavelet_scaling_type  hw_wavelet_scale_adj; // zoom

} Chromatix_hardware_wavelet_type;

typedef struct
{
    trigger_point_type  PBF_trigger;
    float               noise_profile_y;
    float               noise_profile_cb;
    float               noise_profile_cr;
    float               denoise_scale_y;
    float               denoise_scale_chroma;
    float               denoise_edge_softness_y;
    float               denoise_edge_softness_chroma;
    float               denoise_weight_y;
    float               denoise_weight_chroma;
} PBF_noiseprofile_type;

typedef struct
{
    float noise_profile_adj_y;
    float noise_profile_adj_chroma;
    float denoise_weight_adj_y;
    float denoise_weight_adj_chroma;
    float scaling_level;
} PBF_scale_type;

typedef struct
{
    PBF_scale_type  PBF_upscale_data[UP_SCALING_LEVELS];
    PBF_scale_type  PBF_downscale_data[DOWN_SCALING_LEVELS];
} Chromatix_PBF_scaling_type;

typedef struct
{
    int                         PBF_luma_enable;
    int                         PBF_chroma_enable;
    int                         PBF_control_enable;
    int                         PBF_enable_index;
    tuning_control_type         control_PBF;
    trigger_point_type          pbf_hysteresis_trigger;
    PBF_noiseprofile_type       PBF_noise_profile[MAX_LIGHT_TYPES_FOR_SPATIAL];

    Chromatix_PBF_scaling_type  pbf_scale_adj; //zoom
} Chromatix_prescaler_bf_type;

//wavelet denoise, updated in 0x301
#define NUM_NOISE_PROFILE   24
#define WAVELET_LEVEL       4

typedef struct
{
    trigger_point_type  WNR_trigger;
    float               referenceNoiseProfileData[NUM_NOISE_PROFILE]; // default0
    float               denoise_scale_y[WAVELET_LEVEL];
    float               denoise_scale_chroma[WAVELET_LEVEL];
    float               denoise_edge_softness_y[WAVELET_LEVEL];
    float               denoise_edge_softness_chroma[WAVELET_LEVEL];
    float               denoise_weight_y[WAVELET_LEVEL];
    float               denoise_weight_chroma[WAVELET_LEVEL];
    float               denoise_scale_chroma_5th;
    float               denoise_weight_chroma_5th;
    int                 sw_denoise_edge_threshold_y;
    int                 sw_denoise_edge_threshold_chroma;
} ReferenceNoiseProfile_type;

//zoom
typedef struct
{
    float Y_noise_profile_adj_upscale;
    float chroma_noise_profile_adj_upscale;
    float Y_denoise_weight_adj_upscale;
    float chroma_denoise_weight_adj_upscale;
    float upscaling_levels;
} wavelet_upscale_type;

typedef struct
{
    wavelet_upscale_type    wnr_upscale_data[UP_SCALING_LEVELS];
} NoiseProfile_Upscaling_type;

typedef struct
{
    float Y_noise_profile_adj_downscale;
    float chroma_noise_profile_adj_downscale;
    float Y_denoise_weight_adj_downscale;
    float chroma_denoise_weight_adj_downscale;
    float downscaling_levels;
} wavelet_downscale_type;

typedef struct
{
    wavelet_downscale_type  wnr_downscale_data[DOWN_SCALING_LEVELS];
} NoiseProfile_Downscaling_type;

typedef struct
{
    int                         WNR_luma_enable;        //0x303
    int                         WNR_chroma_enable;      //0x303
    int                         WNR_control_enable;     //0x303
    int                         wavelet_enable_index;

    tuning_control_type         control_denoise;
    ReferenceNoiseProfile_type  noise_profile[MAX_LIGHT_TYPES_FOR_SPATIAL];
} wavelet_denoise_type;

typedef struct
{
    wavelet_denoise_type            wavelet_denoise_SW_420;
    wavelet_denoise_type            wavelet_denoise_HW_420;

    NoiseProfile_Upscaling_type     wavelet_upscale_adj;    // zoom
    NoiseProfile_Downscaling_type   wavelet_downscale_adj;  // zoom
} chromatix_wavelet_type;

#define HW_TDN_LEVELS 5

typedef struct
{
    trigger_point_type  WNR_trigger;
    float               referenceNoiseProfileData[NUM_NOISE_PROFILE]; // default 0
    float               denoise_scale_y[WAVELET_LEVEL];
    float               denoise_scale_chroma[WAVELET_LEVEL];
    float               denoise_edge_softness_y[WAVELET_LEVEL];
    float               denoise_edge_softness_chroma[WAVELET_LEVEL];
    float               denoise_weight_y[WAVELET_LEVEL];
    float               denoise_weight_chroma[WAVELET_LEVEL];
    float               denoise_scale_y_5th;
    float               denoise_weight_y_5th;
    float               denoise_scale_chroma_5th;
    float               denoise_weight_chroma_5th;
} WNR_noise_profile_type;

typedef struct
{
    float noise_profile_adj_y[HW_TDN_LEVELS];
    float noise_profile_adj_chroma[HW_TDN_LEVELS];
    float denoise_weight_adj_y[HW_TDN_LEVELS];
    float denoise_weight_adj_chroma[HW_TDN_LEVELS];
    float scaling_level;
} hw_TDN_scale_type;

typedef struct
{
    hw_TDN_scale_type   TDN_upscale_data[UP_SCALING_LEVELS];
    hw_TDN_scale_type   TDN_downscale_data[DOWN_SCALING_LEVELS];
} Chromatix_TDN_scaling_type;

typedef struct
{
    int                           WNRenable;
    int                           WNR_control_enable;
    tuning_control_type           control_WNR_denoise;
    trigger_point_type            tdn_hysteresis_trigger;
    WNR_noise_profile_type        referenceNoiseProfileData[MAX_LIGHT_TYPES_FOR_SPATIAL]; // 6 sets
    Chromatix_TDN_scaling_type    temporal_denoise_scale_adj;                             //zoom
} chromatixWNRDenoise_type;

//0x303
typedef struct
{
    int sknTnSigThrLo;
    int sknTnSigThrHi;
    int sknTnCrThrLo;
    int sknTnCrThrHi;
} chromatix_VDP_SkinRange;

typedef struct
{
    trigger_point_type      temporal_denoise_trigger;

    int                     nRNR_index;
    int                     nSRNR_strength;
    int                     nTNR_strength;

    int                     rnr_level;
    int                     kminY;
    int                     kminC;
    int                     rnrFfact;
    int                     txtThr3;
    int                     txtThr4;
    int                     calcUpperLimit;
    int                     rnrMinNoise;
    int                     rnrMaxNoise;
    int                     rnrChromaFfact;
    int                     rnrSkinFfact;

    chromatix_VDP_SkinRange skinR;
} chromatix_VDP_temporal_denoise;

typedef struct
{
    int                             enable;
    int                             control_enable;
    tuning_control_type             control_temporal_denoise;
    chromatix_VDP_temporal_denoise  temporal_denoise[MAX_LIGHT_TYPES_FOR_SPATIAL];
} chromatix_VDP_temporal_denoise_type;

//=============================================================================
// Feature name : Clamp
// Chipset Versions : 8994, 8992, 8996 and later
// applicable operation mode: View finder,Cam corder and snapshot.
//=============================================================================
typedef struct
{
    int lower_clamp_Y;
    int upper_clamp_Y;
    int lower_clamp_Cb;
    int upper_clamp_Cb;
    int lower_clamp_Cr;
    int upper_clamp_Cr;
} Chromatix_cpp_clamp_type;

typedef struct
{
    //=========================================================================
    // Chromatix header version info (MUST BE THE FIRST PARAMETER)
    //=========================================================================
    ChromatixVersionInfoType                chromatix_version_info;

    //=========================================================================
    // 7x7 ASF
    //=========================================================================
    chromatix_ASF_7x7_type                  chromatix_ASF_7x7;

    //=========================================================================
    // 9x9 ASF
    //=========================================================================
    chromatix_ASF_9x9_type                  chromatix_ASF_9x9;

    Chromatix_HDR_ASF_adj_type              chromatix_hdr_asf_adj_data;
    //=========================================================================
    // wavelet Denoise for 8974,8084,8994,8916,8936,8992
    //=========================================================================
    // 0x0304 Header
    chromatix_wavelet_type                  chromatix_wavelet;

    //=========================================================================
    // wavelet Denoise for 8996 & 8993
    //=========================================================================
    // 305 Header
    Chromatix_hardware_wavelet_type         chromatix_hw_denoise_data;

    Chromatix_HDR_WNR_adj_type              chromatix_hdr_wnr_adj_data;
    Chromatix_ADRC_WNR_adj_type             chromatix_adrc_wnr_adj_data;

    //=========================================================================
    // HW temporal denoise
    //=========================================================================
    chromatixWNRDenoise_type                chromatix_temporal_denoise; //0x304

    //=========================================================================
    // Pre Scaler Bilateral Filter ( PBF )
    //=========================================================================
    Chromatix_prescaler_bf_type             chromatix_pbf_data;

    //=========================================================================
    // CPP Output Clamp
    //=========================================================================
    Chromatix_cpp_clamp_type                chromatix_clamp_data; // Y,Cb,Cr

    //=========================================================================
    // VDP temporal denoise
    //=========================================================================
    chromatix_VDP_temporal_denoise_type     chromatix_VDP_temporal_denoise;  //0x303

    float                                   up_scale_threshold;
    float                                   down_scale_threshold;
    Chromatix_hardware_wavelet_type         chromatix_hw_denoise_data_lpm;

    // 0x310 Updates
    float                                   ext_data[EXT_CPP_ENTRIES];
} chromatix_cpp_type;

#endif  // #ifndef CHROMATIX_CPP_H
