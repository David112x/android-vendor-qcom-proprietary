//=============================================================================
// Copyright (c) 2015 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//=============================================================================

#ifndef CHROMATIX_H
#define CHROMATIX_H

//=============================================================================
//                      CONSTANTS
//=============================================================================

#define CHROMATIX_VERSION 0x309

//0x303
#define MAX_LIGHT_TYPES_FOR_SPATIAL 6   //this is used for ABF3, WNR, 9x9ASF we use 6 sets of triggers
#define NUM_SCENE_MODES             0   // Scene modes Removed in 0x0305
#define MAX_SETS_FOR_TONE_NOISE_ADJ 6
#define MAX_SETS_FOR_ADRC_SCE_ADJ   3
#define MAX_SETS_FOR_ADRC_LUX_ADJ   3

//=============================================================================
// DATA TYPES
//=============================================================================

//=============================================================================
// Chromatix version info
//=============================================================================

typedef unsigned short chromatix_version_type;

typedef struct
{
    unsigned char major;
    unsigned char minor;
    unsigned char revision;
    unsigned char build;
} chromatix_app_version_type;

// Self identifying header type information
typedef enum
{
    HEADER_UNKNOWN = 0,
    HEADER_DEFAULT,
    HEADER_COMMON,
    HEADER_CPP,
    HEADER_POST_PROC,
    HEADER_3A,
} ChromatixHeaderType;

typedef enum
{
    MODE_UNKNOWN = 0,
    MODE_SNAPSHOT,
    MODE_PREVIEW,
    MODE_CUSTOM,
} ChromatixModeType;

typedef struct
{
    chromatix_version_type          chromatix_version;      // major header version (for example 0x0309)
    unsigned short                  revision_number;
    chromatix_app_version_type      chromatix_app_version;  // version of Chromatix that generated the file
    ChromatixHeaderType             chromatix_header_type;  // header type (default/cpp/common/pp/3a)
    unsigned char                   is_compressed;
    unsigned char                   is_mono;
    unsigned char                   is_video;
    unsigned char                   reserved_align;         // structure alignment
    ChromatixModeType               chromatix_mode;
    unsigned int                    target_id;              // identifies the target that was tuned with Chromatix
    unsigned int                    chromatix_id;           // unique group id, allows chromatix to find header files that were generated together
    unsigned int                    reserved[4];
} ChromatixVersionInfoType;

//=============================================================================

typedef unsigned short chromatix_number_elements_type;

// Tuning Control Flag. 0 is Lux Index based, 1 is Gain Based.
typedef enum
{
    CONTROL_LUX_IDX = 0,
    CONTROL_GAIN,
    CONTROL_AEC_EXP_SENSITIVITY_RATIO,
    CONTROL_EXP_TIME_RATIO,
    CONTROL_DRC_GAIN,
    CONTROL_MAX,
} ControlVariableType;

typedef unsigned char tuning_control_type;

// This type holds both luma index and gain based trigger points to be
// used in configuration of VFE blocks.
typedef struct
{
    float   gain_start;         // Gain based trigger point.
    float   gain_end;           // Gain based end point.
    long    lux_index_start;    // Lux index based trigger point.
    long    lux_index_end;      // Lux index based end point.
} trigger_point_type;

typedef struct
{
    float start;    // trigger point Start
    float end;      // trigger point End
} trigger_point2_type;

//=============================================================================
//0x207  trigger pt for CCT
//=============================================================================
typedef struct
{
    unsigned long CCT_start;
    unsigned long CCT_end;
} chromatix_CCT_trigger_type;

//=============================================================================
//  VFE basic struct
//=============================================================================

//=============================================================================
// Bad pixel correction data types
//=============================================================================
typedef enum
{
    LOW_LIGHT = 0,
    NORMAL_LIGHT,
    LOWLIGHT_MAX_LIGHT
} lowlight_type;

typedef enum
{
    BPC_NORMAL_LIGHT =  0,
    BPC_LOW_LIGHT,
    BPC_MAX_LIGHT
} bpc_light_type;

//=============================================================================
// 4 Channel Black Level data types
//=============================================================================
typedef struct
{
    unsigned short  black_even_row_even_col;    // BlackEvenRowEvenCol
    unsigned short  black_even_row_odd_col;     // BlackEvenRowOddCol
    unsigned short  black_odd_row_even_col;     // BlackOddRowEvenCol
    unsigned short  black_odd_row_odd_col;      // BlackOddRowOddCol
} chromatix_4_channel_black_level;

typedef enum
{
    AWB_STATS_LOW_LIGHT = 0,
    AWB_STATS_NORMAL_LIGHT,
    AWB_STATS_OUTDOOR,
    AWB_STATS_MAX_LIGHT
} chromatix_awb_stats_light_type;

typedef enum
{
    color_xform_format_601 = 0,
    color_xform_format_709,
    color_xform_format_691,
    color_xform_format_601sdtv,
} color_xform_format;

typedef struct
{
    color_xform_format  video_xform;
    int                 subtractive_offset[3];
    float               scaling_threshold;
} Chromatix_color_space_transform_type;

typedef struct
{
    /* 8-bit, Q0, unsigned */
    unsigned char y_min;                // LumaMin

    /* 8-bit, Q0, unsigned */
    unsigned char y_max;                // LumaMax

    /* Slope of neutral region line 1 */
    /* 8-bit, Q4, signed */
    char m1;                            // Slope1

    /* Slope of neutral region line 2 */
    /* 8-bit, Q4, signed */
    char m2;                            // Slope2

    /* Slope of neutral region line 3 */
    /* 8-bit, Q4, signed */
    char m3;                            // Slope3

    /* Slope of neutral region line 4 */
    /* 8-bit, Q4, signed */
    char m4;                            // Slope4

    /* Cb intercept of neutral region line 1 */
    /* 12-bit, Q0, signed */
    short c1;                           // CbOffset1

    /* Cb intercept of neutral region line 2 */
    /* 12-bit, Q0, signed */
    short c2;                           // CrOffset2

    /* Cb intercept of neutral region line 3 */
    /* 12-bit, Q0, signed */
    short c3;                           // CbOffset3

    /* Cb intercept of neutral region line 4 */
    /* 12-bit, Q0, signed */
    short c4;                           // CrOffset4
} chromatix_wb_exp_stats_type;

//=============================================================================
// AFD (8K) row sum / column sum stat collection
//=============================================================================
//205 mods
typedef struct
{
    int             row_sum_enable;             // 0=disable, 1=enable, default=1
    float           row_sum_hor_Loffset_ratio;  // default=0
    float           row_sum_ver_Toffset_ratio;  // default=0
    float           row_sum_hor_Roffset_ratio;  // default=0
    float           row_sum_ver_Boffset_ratio;  // default=0
    unsigned char   row_sum_V_subsample_ratio;  // 1 to 4, int

    int             col_sum_enable;             // 0=disable, 1=enable, default=0
    float           col_sum_hor_Loffset_ratio;  // default=0
    float           col_sum_ver_Toffset_ratio;  // default=0
    float           col_sum_hor_Roffset_ratio;  // default=0
    float           col_sum_ver_Boffset_ratio;  // default=0
    unsigned char   col_sum_H_subsample_ratio;  // 2 to 4, int
} chromatix_rscs_stat_config_type;

//this set of parameters is extracted from chromatix_rolloff_trigger_type in the common header ( 0x303 )
typedef struct
{
    int                 rolloff_enable;             //0x303
    int                 rolloff_control_enable;     //0x303
    tuning_control_type control_rolloff;
    trigger_point_type  rolloff_lowlight_trigger;   //changed to lowlight trigger in 0x300
} chromatix_rolloff_trigger_type;

/*
Color Correction

|R|   |C0 C1 C2||R|   |K0|
|G| = |C3 C4 C5||G| + |K1|
|B|   |C6 C7 C8||B|   |K2|
*/
typedef struct
{
    /* 12-bit, {Q7,Q8,Q9,Q10}, signed */
    /* R */
    float           c0;                 // RtoR
    float           c1;                 // GtoR
    float           c2;                 // BtoR

    /* G */
    float           c3;                 // RtoG
    float           c4;                 // GtoG
    float           c5;                 // BtoG

    /* B */
    float           c6;                 // RtoB
    float           c7;                 // GtoB
    float           c8;                 // BtoB

    /* 11-bit, Q0, signed */
    short           k0;                 // ROffset

    /* 11-bit, Q0, signed */
    short           k1;                 // GOffset

    /* 11-bit, Q0, signed */
    short           k2;                 // BOffset

    /* range 0x0-0x3 maps to Q7-Q10 */
    unsigned char   q_factor;           // QFactor
} chromatix_color_correction_type;

//=============================================================================
// Gamma
//=============================================================================

#define GAMMA_TABLE_SIZE           512 //0x303
//8994 has 512 entries with output 14bits
//8084 has 256 gamma entries with output 12bits, SW needs to downsample gamma by 2 and right shift 2 bits of the table values

typedef struct
{
    /* 14-bit, unsigned */
    unsigned short gamma_R[GAMMA_TABLE_SIZE];    // Gamma
    unsigned short gamma_G[GAMMA_TABLE_SIZE];
    unsigned short gamma_B[GAMMA_TABLE_SIZE];
} chromatix_gamma_table_type;

//=============================================================================
// Chroma enhancement
//=============================================================================

/* Chroma enhancement

|Cb|   |a  0| |1  b||B - G|   | KCb |
|  | = |    | |    ||     | + |     |
|Cr|   |0  c| |d  1||R - G|   | KCr |

*/
typedef struct
{
    /* 11-bit, Q8, signed*/
    float a_m;                  // CbScalingNegative_am

    /* 11-bit, Q8, signed*/
    float a_p;                  // CbScalingPositive_ap

    /* 11-bit, Q8, signed*/
    float b_m;                  // DiffRGtoCbShearingNegative_bm

    /* 11-bit, Q8, signed*/
    float b_p;                  // DiffRGtoCbShearingPositive_bp

    /* 11-bit, Q8, signed*/
    float c_m;                  // CrScalingNegative_cm

    /* 11-bit, Q8, signed*/
    float c_p;                  // CrScalingPositive_cp

    /* 11-bit, Q8, signed*/
    float d_m;                  // DiffBGtoCrShearingNegative_dm

    /* 11-bit, Q8, signed*/
    float d_p;                  // DiffBGtoCrShearingPositive_dp

    /* 11-bit, Q0, signed */
    short k_cb;                 // CBOffset_k_cb

    /* 11-bit, Q0, signed */
    short k_cr;                 // CROffset_k_cr
} chroma_enhancement_type;

/* RGB-to-Y conversion.

|R|
Y = |VO V1 V2| |G| + K
|B|

*/
typedef struct
{
    /* 12-bit, Q8, signed*/
    float v0;

    /* 12-bit, Q8, signed*/
    float v1;

    /* 12-bit, Q8, signed*/
    float v2;

    /* 9s, Q0, */
    short k;
} luma_calculation_type;

/* Color Conversion */
typedef struct
{
    chroma_enhancement_type chroma;           // Chroma
    luma_calculation_type   luma;             // Luma
} chromatix_color_conversion_type;

typedef struct
{
    //0x303
    trigger_point_type  abf2_trigger;
    // red channel parameters
    unsigned short      threshold_red[3];   // 12-bit pixels
    float               scale_factor_red[2];

    // green channel parameters
    unsigned short      threshold_green[3]; //12-bit  pixels
    float               scale_factor_green[2];
    float               a[2];

    // blue channel parameters
    unsigned short      threshold_blue[3];  //12-bit  pixels
    float               scale_factor_blue[2];

    // tables for red/green/blue channels
    float               table_pos[16];
    float               table_neg[8];
    // note: mult_factor is derived from threshold
} chromatix_adaptive_bayer_filter_data_type2;  // new 8K

typedef struct
{
    unsigned short  threshold[3];   // 12-bit pixels
    float           scale_factor[2];

    float           table_pos[16];
    float           table_neg[8];
} chromatix_luma_filter_type;

//0x303: for ABF3
typedef enum
{
    ABF3_LEVEL0 = 0,
    ABF3_LEVEL1,
    ABF3_LEVEL_MAX,
    ABF3_LEVEL_INVALID = ABF3_LEVEL_MAX
} abf3_level_type;

typedef enum
{
    ABF3_CHANNEL_R = 0,
    ABF3_CHANNEL_GR,
    ABF3_CHANNEL_GB,
    ABF3_CHANNEL_B,
    ABF3_CHANNEL_MAX,
    ABF3_CHANNEL_INVALID = ABF3_CHANNEL_MAX
} abf3_channel_type;

typedef struct
{
    unsigned short  filter_options;                                         // enableing cross-plance processing, FIR/IIR filtering, etc...
    unsigned int    signal2_lut_level0[33];                                 // level 0 signal squares for fixed pattern noise LUT
    unsigned int    signal2_lut_level1[33];                                 // level 1 signal squares for fixed pattern noise LUT

    unsigned short  shot_noise_scale[ABF3_LEVEL_MAX][ABF3_CHANNEL_MAX];     // scale factor for shot noise // floating
    unsigned short  fpn_noise_scale[ABF3_LEVEL_MAX][ABF3_CHANNEL_MAX];      // scale factor for fixed pattern noise
    unsigned short  noise_offset[ABF3_LEVEL_MAX][ABF3_CHANNEL_MAX];         // offset of dark noise

    unsigned short  wtable_adj[ABF3_LEVEL_MAX][ABF3_CHANNEL_MAX];           // bit precision adjustment for bilateral filter
    unsigned short  softthld_noise_shift[ABF3_LEVEL_MAX][ABF3_CHANNEL_MAX]; // adjusting bit precision in calculating soft threshold

    unsigned short  distance_ker[2][5];                                     // level 1 pixel distance to calculate bilateral filter coefficients
                                                                            // (distance_ker[0][] - Gr/Gb, distance_ker[1][] - R/B)
    unsigned short  minmax_sel[2][2];                                       // select two pixel groups in level 1 pixel mataching stage for min and max clamps
                                                                            // minmax_sel[0][] - Gr/Gb, minmax_sel[1][] - R/B
} chromatix_ABF3_reserve;

typedef struct
{
    trigger_point_type  ABF3_trigger;
    unsigned int        noise_std2_lut_level0[129];                             // level 0 noise variance LUT
    unsigned int        noise_std2_lut_level1[129];                             // level 1 noise variance LUT

    float               edge_softness[ABF3_LEVEL_MAX][ABF3_CHANNEL_MAX];        // bilateral filter response, 12uQ8
    float               softthld_noise_scale[ABF3_LEVEL_MAX][ABF3_CHANNEL_MAX]; // scale factor used to calculate soft threshold, 12uQ8
    float               filter_strength[ABF3_LEVEL_MAX][ABF3_CHANNEL_MAX];      // denoise weight, 8uQ8
    // 304->305 updates
    unsigned short      curve_offset[ABF3_LEVEL_MAX][ABF3_CHANNEL_MAX];         // offset adjustment for bilateral filter
    unsigned char       min_max_filter_en;
    // 0x0305 Changes
    float               spatial_scale[ABF3_LEVEL_MAX][ABF3_CHANNEL_MAX];        // 9uQ8
    float               distance_percentage;                                    // ABF3:: RNR mesh2d table
    float               gain_percentage;                                        // ABF3:: RNR mesh2d table
} chromatix_abf3_core_type;

typedef struct
{
    trigger_point2_type     aec_sensitivity_ratio;      // For CONTROL_AEC_EXP_SENSITIVITY_RATIO
    trigger_point2_type     exp_time_trigger;           // For CONTROL_EXP_TIME_RATIO
    float                   noise_profile_adj[2];
    float                   edge_softness_adj[2][2];    // [][0] for R/B [][1] for GR/GB
    float                   filter_strength_adj[2][2];  // [][0] for R/B [][1] for GR/GB
} hdr_abf3_adj_core_type;

typedef struct
{
    int                     enable;
    tuning_control_type     control_hdr_abf3;                                       // CONTROL_AEC_EXP_SENSITIVITY_RATIO or CONTROL_EXP_TIME_RATIO
    hdr_abf3_adj_core_type  hdr_abf3_adj_core_data[MAX_SETS_FOR_TONE_NOISE_ADJ];
} Chromatix_HDR_ABF3_adj_type;

/******************************************************************************
5x5 ASF data types
******************************************************************************/
/* 5 x 5 Adaptive Spatial Filter.
* There are two components in this filter
* High Pass Filter (HPF): 5 x 5
* Low Pass Filter (LPF): 3 x 3
* HPF can use upto two 5 x 5 filters. The sensor driver chooses to use
* both or single filter.
* LPF can be implemented in H/W Luma Filter module .
*/
typedef struct
{
    short  a11;
    short  a12;
    short  a13;
    short  a14;
    short  a15;

    short  a21;
    short  a22;
    short  a23;
    short  a24;
    short  a25;

    short  a31;
    short  a32;
    short  a33;
    short  a34;
    short  a35;

    short  a41;
    short  a42;
    short  a43;
    short  a44;
    short  a45;

    short  a51;
    short  a52;
    short  a53;
    short  a54;
    short  a55;
} matrix_5_5_type;

typedef enum
{
    ASF_LOW_LIGHT = 0,
    ASF_NORMAL_LIGHT,
    ASF_BRIGHT_LIGHT,
    ASF_MAX_LIGHT,
    ASF_INVALID_LIGHT = ASF_MAX_LIGHT
} chromatix_asf_brightness_level_type;

typedef enum
{
    ASF_MODE_DISABLE = 0,
    ASF_MODE_SINGLE,
    ASF_MODE_DUAL,
    ASF_MODE_SMART,
    ASF_MODE_MAX,
    ASF_MODE_INVALID = ASF_MODE_MAX
} chromatix_asf_mode_type;

typedef float           normalize_factor_type;
typedef char            filter_threshold_type;
typedef unsigned char   filter_smoothing_degree_type;
typedef float           filter_sharpen_degree_type;

typedef struct
{
    /* noise level to filter out */
    filter_threshold_type        lower_threshold;       // LowThresh
    /* max sharpening limit */
    filter_threshold_type        upper_threshold;       // UpThresh
    /* negative limit */
    filter_threshold_type        negative_threshold;    // NegThresh
    /* max sharpening limit f2 */
    filter_threshold_type        upper_threshold_f2;    // UpThreshF2
    /* negative limit f2 */
    filter_threshold_type        negative_threshold_f2; // NegThreshF2
    /* filter1 sharpening degree */
    filter_sharpen_degree_type   sharpen_degree_f1;     // SharpAmtF1
    /* filter2 sharpening degree */
    filter_sharpen_degree_type   sharpen_degree_f2;     // SharpAmtF2
    /* smoothing degree */
    filter_smoothing_degree_type smoothing_percent;     // SmoothPct (3x3)
    filter_smoothing_degree_type smoothing_percent_5x5; // [205], range= 0-100, default=90
} asf_setting_type;

typedef struct
{
    /* Filter mode, 0 or 1 */
    unsigned long           filter_mode;            // FilterMode
    /* normalize factor to filter 1 */
    normalize_factor_type   normalize_factor1;      // Normal1
    /* normalize factor to filter 2 */
    normalize_factor_type   normalize_factor2;      // Normal2
    /* HPF: 5x5 filter 1 coefficients */
    matrix_5_5_type         filter1;                // Filter1
    /* HPF: 5x5 filter 2 coefficients */
    matrix_5_5_type         filter2;                // Filter2
    /* extraction factor */
    unsigned char           extraction_factor;      // ExtrFactor
    /* asf settings based on lighting condition */
    asf_setting_type        setting[ASF_MAX_LIGHT]; // Setting
} chromatix_asf_5_5_type;

typedef struct
{
    tuning_control_type         control_asf_5X5;                //DEFAULT 1 GAIN default value for VF and snapshot is same

    trigger_point_type          asf_5x5_lowlight_trigger;
    trigger_point_type          asf_5x5_outdoor_trigger;

    chromatix_asf_5_5_type      asf_5_5;                        // SnapshotFilter
    /* soft_focus_degree */
    filter_sharpen_degree_type  soft_focus_degree_5_5;          // SoftFocusDegree

    float                       asf_5_5_sharp_min_ds_factor;    //default 0.5
    float                       asf_5_5_sharp_max_ds_factor;    //default 4.0
    float                       asf_5_5_sharp_max_factor;       //default 1 for 6k, 8k, old 7k (before sROC). default 2.0 for sROC.
} chromatix_ASF_5x5_type;

//=============================================================================
// Chroma Suppression data types
//=============================================================================
typedef struct
{
    unsigned char   cs_luma_threshold1;     // LumaThresh1
    unsigned char   cs_luma_threshold2;     // LumaThresh2
    unsigned char   cs_luma_threshold3;     // LumaThresh3
    unsigned char   cs_luma_threshold4;     // LumaThresh4
    unsigned char   cs_chroma_threshold1;   // ChromaThresh1
    unsigned char   cs_chroma_threshold2;   // ChromaThresh2
} cs_luma_threshold_type;

//=============================================================================
//  Bayer histogram config
//=============================================================================
typedef struct
{
    float   horizontalOffsetRatio;
    float   verticalOffsetRatio;
    float   horizontalWindowRatio;
    float   verticalWindowRatio;
} bayer_histogram_config_type;

//=============================================================================
//  Bayer grid config
//=============================================================================
typedef struct
{
    unsigned char   min_thresh_R;
    unsigned char   min_thresh_Gr;
    unsigned char   min_thresh_Gb;
    unsigned char   min_thresh_B;
    unsigned char   saturation_thresh_R;
    unsigned char   saturation_thresh_Gr;
    unsigned char   saturation_thresh_Gb;
    unsigned char   saturation_thresh_B;
    float           horizontalOffsetRatio;
    float           verticalOffsetRatio;
    float           horizontalWindowRatio;
    float           verticalWindowRatio;
    unsigned short  num_H_regions;
    unsigned short  num_V_regions;
} bayer_grid_config_type;

//=============================================================================
//  LA Struct definition
//=============================================================================
typedef struct
{
    unsigned char shadow_range;
    unsigned char shadow_boost_allowance;
    unsigned char LA_reduction_fine_tune;
    unsigned char highlight_suppress_fine_tune;
    unsigned char shadow_boost_fine_tune;
} LA_args_type;

typedef struct
{
    trigger_point_type  skintone_nr_trigger;
    // for skin-tone soft-thresholding
    unsigned char       skin_soft_threshold;
    float               skin_soft_strength;
    // for filterings
    int                 skin_nr_filter[4];
    int                 skin_detection_y_filter[4];
    int                 skin_detection_chroma_filter[4];
    int                 skin_probability_filter[8];
    // parameters to define skin-tone regions
    float               H_min;
    float               H_max;
    float               Y_min;
    float               Y_max;
    float               S_hY_min;
    float               S_hY_max;
    float               S_lY_min;
    float               S_lY_max;
    // parameters for quantizations of skin-tone probabilities, (4bit)
    unsigned char       boundary_probability;
    int                 skin_percent;
    float               non_skin_to_skin_q_ratio;
} Chromatix_SkinNR_core_type;

typedef struct
{
    int                         skintone_nr_en;
    int                         skintone_nr_control_enable;
    tuning_control_type         control_skintone_nr;
    Chromatix_SkinNR_core_type  SkinNR_data[MAX_LIGHT_TYPES_FOR_SPATIAL];
} Chromatix_Skintone_NR_type;

//=============================================================================
// HW CAC2
//=============================================================================
typedef struct
{
    trigger_point_type  CAC2_trigger;
    unsigned short      y_saturation_thr;
    unsigned short      y_spot_thr;
    unsigned short      c_saturation_thr;
    unsigned short      c_spot_thr;
} Chromatix_HW_CAC2_core_type;

typedef struct
{
    int                             CAC2_en;
    int                             CAC2_control_enable;
    tuning_control_type             control_cac2;
    trigger_point_type              CAC2_hysteresis_point;
    float                           CAC2_scaling_threshold;
    Chromatix_HW_CAC2_core_type     CAC2_data[MAX_LIGHT_TYPES_FOR_SPATIAL];
} Chromatix_HW_CAC2_type;

//=============================================================================
// MCE (Memory Color Enhancement) data types
//=============================================================================
typedef struct
{
    unsigned char   green_y[4];
    float           green_boost_factor;
    char            green_cr_boundary;
    char            green_cb_boundary;
    unsigned char   green_cr_transition_width;
    unsigned char   green_cb_transition_width;
    unsigned short  green_bright_index;
    unsigned short  green_dark_index;

    unsigned char   blue_y[4];
    float           blue_boost_factor;
    char            blue_cr_boundary;
    char            blue_cb_boundary;
    unsigned char   blue_cr_transition_width;
    unsigned char   blue_cb_transition_width;
    unsigned short  blue_bright_index;
    unsigned short  blue_dark_index;

    unsigned char   red_y[4];
    float           red_boost_factor;
    char            red_cr_boundary;
    char            red_cb_boundary;
    unsigned char   red_cr_transition_width;
    unsigned char   red_cb_transition_width;
    unsigned short  red_bright_index;
    unsigned short  red_dark_index;
} mce_type;

//=============================================================================
// for skin color enhancement
//=============================================================================
typedef struct
{
    int cr;
    int cb;
} cr_cb_point;

typedef struct
{
    cr_cb_point point1;
    cr_cb_point point2;
    cr_cb_point point3;
} cr_cb_triangle;

typedef struct
{
    cr_cb_triangle triangle1;
    cr_cb_triangle triangle2;
    cr_cb_triangle triangle3;
    cr_cb_triangle triangle4;
    cr_cb_triangle triangle5;
} sce_cr_cb_triangle_set;

typedef struct
{
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;
    //  cr' = [ a   b    c] cr
    //  cb' = [ d   e    f] cb
    //   1  = [ 0   0    1] 1
} sce_affine_transform_2d;

typedef struct
{
    int             hot_pixel_correction_disable;   //1u
    int             cold_pixel_correction_disable;  //1u
    unsigned char   highlight_detection_param1;     //4u
    char            highlight_detection_param2;     //4s
    char            highlight_detection_param3;     //4s
} BPC2_reserved_type;

typedef struct
{
    trigger_point_type  BPC2_trigger;
    int                 same_channel_detection;         //1u
    int                 same_channel_recovery;          //1u
    unsigned char       fmax;                           //7uQ6
    unsigned char       fmin;                           //7uQ6
    unsigned short      BPC_offset;                     //14u
    unsigned short      BCC_offset;                     //14u
    unsigned short      correction_threshold;           //13u
    unsigned char       noise_model_gain;               //8uQ6
    unsigned int        highlight_detection_threshold;  //20u
    unsigned short      highlight_value_threshold;      //14u
} BPC2_core_type;

#define NUM_AEC_STATS 16

typedef struct
{
    unsigned short  bpc_4_offset_r_hi;
    unsigned short  bpc_4_offset_r_lo;

    unsigned short  bpc_4_offset_gr_hi;
    unsigned short  bpc_4_offset_gr_lo;

    unsigned short  bpc_4_offset_gb_hi;
    unsigned short  bpc_4_offset_gb_lo;

    unsigned short  bpc_4_offset_b_hi;
    unsigned short  bpc_4_offset_b_lo;
} bpc_4_offset_type;

typedef struct
{
    int  non_dir_weight_1;      // HW: 3uQ0
    int  non_dir_weight_2;      // HW: 3uQ0
    int  en_dyna_clamp_G;       // HW: 1uQ0
    int  en_dyna_clamp_RB;      // HW: 1uQ0
    int  dis_dclamp_cntr_G;     // HW: 1uQ0
    int  dis_dclamp_cntr_RB;    // HW: 1uQ0
} Demosaic4_reserve;

typedef struct
{
    int     tk[14];             // HW: 7uQ4
    int     ak[14];             // HW: 12uQ0
    float   wk[14];             // HW: 10uQ10
    int     dis_directional_G;  // HW: 1uQ0
    int     dis_directional_RB; // HW: 1uQ0
    int     low_freq_weight_G;  // HW: 8uQ8
    int     low_freq_weight_RB; // HW: 8uQ8
    int     noise_level_G;      // HW: 14uQ0
    int     noise_level_RB;     // HW: 14uQ0
} Demosaic4_core;

typedef struct
{
    float   wk[18];
    int     bk[18];
    int     lk[18];
    int     Tk[18];

    //add to 0x303
    //Applicale chipset(s): 8084, 8994 and later chipsets
    //Applicable operation mode: Viewfinder, snapshot processing, and video modes
    int     dis_directional_G;
    int     dis_directional_RB;
    int     en_dyna_clamp_G;
    int     en_dyna_clamp_RB;
    int     dis_dclamp_cntr_G;
    int     dis_dclamp_cntr_RB;
} demosaic3_Reserve;

typedef struct
{
    float   bL;
    float   aG;                 //need this for older versio VFE
    float   low_freq_weight_G;  //lambda_G;
    float   low_freq_weight_RB; //lambda_RB;
    int     noise_level_G;
    int     noise_level_RB;
    int     non_dir_weight_1;   //Wgr_1;
    int     non_dir_weight_2;   //Wgr_2;
} demosaic3_tune;

typedef struct
{
    demosaic3_Reserve   reserve;

    //2 sets for lowlight and normal light
    demosaic3_tune      data[LOWLIGHT_MAX_LIGHT];
} demosaic3_type;

//207 chroma filter
typedef struct
{
    float h[4];
    float v[2];
} Chroma_filter_type;

//=============================================================================
//=============================================================================
typedef struct
{
    // parameter for general use
    float           low_expIdx_delta;   // parameter for attenuation
    float           high_expIdx_delta;  // parameter for attenuation
    float           low_aec_gain;       // parameter for attenuation
    float           high_aec_gain;      // parameter for attenuation
    int             dehaze_en;

    float           fCV_R;              // HW: 10uQ10
    float           fCV_G;              // HW: 10uQ10
    float           fCV_B;              // HW: 10uQ10
    float           fCV_Kmax;           // HW: 10uQ10 //0x304
    float           fCV_Kmin;           // HW: 10uQ10 //0x304
    float           fCV_Kc;             // HW: 10uQ9
    float           fCV_Cthr;           // HW: 10uQ2 (8uQ2 for 12u pixels)
    int             nInitCntFactor;     // HW: 4uQ0
    float           fSigmaXY;
    float           fSigmaZ;
    int             wt_3x3;

    // parameters for calculated curve set
    int             calculation_algo_sel;
    int             nNormalLoCut;
    int             nNormalHiCut;
    int             nStrongLoCut;
    int             nStrongHiCut;
    float           fBusyThr1;
    float           fBusyThr2;
    unsigned long   nLowTrig0;
    unsigned long   nLowTrig1;
    unsigned long   nLowTrig2;
    unsigned long   nLowTrig3;
    unsigned long   nHighTrig0;
    unsigned long   nHighTrig1;
    unsigned long   nHighTrig2;
    unsigned long   nHighTrig3;
    float           fScaleConstraintLB;
    float           fScaleSigmaMult;
    float           fScaleWidthLB;
    float           fMasterScaleAdj;
    int             nSoftThr0;
    int             nSoftThr1;
    int             nSoftThrSlope;
    float           fShiftScaleAdj;

    float           dehaze_w1;
    float           dehaze_alpha;
    float           dehaze_delta;

    // fixed curve set
    int             mask_rect[129];             // 14s
    int             master_curve[129];          // 14s
    int             master_scale[129];          // 14s
    int             shift_curve[129];           // 14s
    int             shift_scale[129];           // 14s
    int             master_scale_dehaze[129];   // 14s
    int             shift_scale_dehaze[129];    // 14s

    // Denoise adjustment region
    int             idxStartRegion;
} chromatix_LTM_reserved;

typedef struct
{
    int     rgn_enable;
    int     sat_curve[129]; // 305
    float   Y_ratio_max;    // 305
    float   curve_blend;

    // parameters for fixed curve
    float   LTM_fixed_curve_strength;
    float   fixed_content_low;
    float   fixed_content_high;

    // parameters for calculated curve
    int     nGlobalToneStrength;
    int     nGlobalToneContrast;
    int     nGlobalToneWeight;
    int     nLocalToneStrength;
    int     nLocalToneContrast;
    // Parameters for ABF3 Adjustment
    float   fLTMgainRange[2];
    float   fStrengthScale;
} chromatix_LTM_core;

typedef enum
{
    LTM_BRIGHT_LIGHT = 0,
    LTM_NORMAL_LIGHT,
    LTM_LOW_LIGHT,
    LTM_MAX_LIGHT
} LTM_light_type;

// Parameters Bit width : 14
// Notes: For targets 8992, Software will adjust the curves dynamically before programming to HW
typedef struct
{
    int                     enable;
    int                     LTM_control_enable;
    tuning_control_type     control_LTM;
    trigger_point_type      LTM_low_light_trigger;
    trigger_point_type      LTM_bright_light_trigger;

    chromatix_LTM_core      chromatix_ltm_core_data[LTM_MAX_LIGHT];
    chromatix_LTM_reserved  reservedData;
} chromatix_LTM_type;

typedef struct
{
    // Backlit Scene Detection
    // 306 Change -> other parameters moved to 3A Header
    LA_args_type        backlight_la_8k_config; // Luma Adaptation configuration when backlight scene is detected, compensation portion.
    chromatix_LTM_type  backlit_LTM;            // 0x303
} backlit_scene_detect_vfe_type;

//added 207
//=============================================================================
// red eye reduction
//=============================================================================
typedef struct
{
    int red_eye_reduction_xenon_strobe_enable;  // 0x208
    int red_eye_reduction_led_flash_enable;     // 0x208
    int number_of_preflash_cycles;              // default =3
    int preflash_interval_between_pulese;       // default = 45, unit = msec
    int preflash_Xenon_pulese_duration;         // default = 20 , unit  = usec
    int preflash_LED_pulse_duration;            // default = 30, unit = msec
    int preflash_LED_current;                   // default = 600, unit = mA.
} red_eye_reduction_type;

//0x208, mod in 0x301
typedef struct
{
    /* converted to Q7 in the code */
    float green_even;                // GreenEvenRow
    float green_odd;                 // GreenOddRow
    float red;                       // Red
    float blue;                      // Blue
} chromatix_channel_balance_gains_type;

//=============================================================================
//  Bayer Stat AEC
//=============================================================================
typedef struct
{
    float   exposure_time;
    float   no_shake_threshold;
    float   moderate_shake_threshold;
} shake_table_type;

#define MAX_SHAKE_LUT_SIZE 10

typedef struct
{
    int                 enable;
    unsigned char       valid_entries;
    shake_table_type    shake_table[MAX_SHAKE_LUT_SIZE];
} shake_detection_type;

//=============================================================================
//  ASD struct
//=============================================================================
typedef struct
{
    int                                 ASD_Software_Type;              // 0 means hybrid, 1 means using bayer stats, for UA code only
    backlit_scene_detect_vfe_type       backlit_scene_detect;           // added aggressiveness in 0x207, added max_percent_threshold in 0x208
    // moved to ASD from VFE section
    chromatix_color_conversion_type     sunset_color_conversion;        // Sunset
    chromatix_color_conversion_type     skintone_color_conversion;      // SkinTL84
    chromatix_color_conversion_type     skintone_color_conversion_d65;  // SkinD65
    chromatix_color_conversion_type     skintone_color_conversion_a;    // SkinA
} ASD_VFE_struct_type;

//=============================================================================
//  Exposure struct
//=============================================================================
typedef struct
{
    float                  color_correction_global_gain;    // CCGlobalGain
    red_eye_reduction_type red_eye_reduction;               // added in 0x0207, added more enable params in 0x208
} exposure_struct_type;

//=============================================================================
//=============================================================================
//=============================================================================
//  VFE structs
//=============================================================================
//=============================================================================
//=============================================================================
typedef struct
{
    tuning_control_type                 control_blk;            // default 0 for lux_index
    trigger_point_type                  blk_lowlight_trigger;
    unsigned short                      max_blk_increase;       // default 15*16 in 12bit
    chromatix_4_channel_black_level     normal_light_4_channel; // NormalLight4Channel
} chromatix_black_level_type;

typedef struct
{
    int                                         ABF2_enable;            //0x303
    int                                         ABF2_control_enable;    //0x303
    tuning_control_type                         control_abf2;           // default 1
    unsigned short                              abf2_enable_index;

    chromatix_adaptive_bayer_filter_data_type2  abf2_config[MAX_LIGHT_TYPES_FOR_SPATIAL];
} chromatix_ABF2_type;

//0x303: 3 sets ABF3: lowlight, normal light, bright light
typedef struct
{
    int                         abf3_enable;
    int                         abf3_control_enable;
    tuning_control_type         control_abf3;

    trigger_point_type          abf3_hysteresis_trigger;
    int                         abf3_snapshot_hyst_en_flag;

    //triger is in each of the MAX_LIGHTS_TYPES_FOR_SPATIAL
    chromatix_abf3_core_type    abf3_config[MAX_LIGHT_TYPES_FOR_SPATIAL];
    chromatix_ABF3_reserve abf3_reserved_data;
} chromatix_ABF3_type;

//=============================================================================
//Green Imbalance Correction type 0x304
//To correct visual noise effect due to Gr/Gb imbalance;
//GIC works on Gr/Gb and the 2 channels share the same parameters.
//=============================================================================
typedef struct
{
    unsigned int    signal2_lut_level0[33]; // level 0 signal squares for fixed pattern noise LUT
    unsigned short  shot_noise_scale;       // scale factor for shot noise
    unsigned short  fpn_noise_scale;        // scale factor for fixed pattern noise
    unsigned short  noise_offset;           // offset of dark noise
} chromatix_GIC_reserve;

typedef struct
{
       trigger_point_type   GIC_trigger;
       unsigned int         noise_std2_lut_level0[129]; // level 0 noise variance LUT
       float                softthld_noise_scale;       // scale factor used to calculate soft threshold, 12uQ8
       unsigned short       softthld_noise_shift;       // adjusting bit precision in calculating soft threshold
       float                filter_strength;            // denoise weight, 8uQ8
} chromatix_gic_core_type;

typedef struct
{
    int                     gic_enable;
    int                     gic_control_enable;
    tuning_control_type     control_gic;
    chromatix_gic_core_type gic_config[MAX_LIGHT_TYPES_FOR_SPATIAL];
    chromatix_GIC_reserve   gic_reserved_data;
} chromatix_GIC_type;

typedef struct
{
    int                 bpc_enable;
    int                 bpc_control_enable;
    tuning_control_type control_bpc;            //default 0
    trigger_point_type  bpc_lowlight_trigger;

    //204
    unsigned char       bpc_Fmin;
    unsigned char       bpc_Fmax;
    unsigned char       bpc_Fmin_lowlight;
    unsigned char       bpc_Fmax_lowlight;

    bpc_4_offset_type   bpc_4_offset[BPC_MAX_LIGHT];

    // 0x207 bad pixel cluster params
    int                 bcc_enable;             //0x303
    int                 bcc_control_enable;     //0x303
    tuning_control_type control_bcc;            //default 0
    trigger_point_type  bcc_lowlight_trigger;

    unsigned char       bcc_Fmin;
    unsigned char       bcc_Fmax;
    unsigned char       bcc_Fmin_lowlight;
    unsigned char       bcc_Fmax_lowlight;

    bpc_4_offset_type   bcc_4_offset[BPC_MAX_LIGHT];
} chromatix_BPC_type;

typedef struct
{
    int                 BPC2_en;
    int                 BPC2_control_enable;
    tuning_control_type control_BPC2;
    trigger_point_type  bpc2_hysteresis_trigger;
    int                 bpc2_snapshot_hyst_en_flag;
    BPC2_core_type      BPC2_data[MAX_LIGHT_TYPES_FOR_SPATIAL];
    BPC2_reserved_type  BPC2_reserved_data;
} Chromatix_BPC2_type;

typedef struct
{
    //0x303 updated
    int                 demosaic_enable;
    int                 demosaic_control_enable;
    tuning_control_type control_demosaic3;
    trigger_point_type  demosaic3_trigger_lowlight;
    demosaic3_type      demosaic3_data;
} chromatix_demosaic_type;

typedef struct
{
    int                 demosaic4_enable;
    int                 demosaic4_control_enable;
    tuning_control_type control_demosaic4;
    trigger_point_type  demosaic4_trigger_lowlight;
    //2 sets for lowlight and normal light
    Demosaic4_core      demosaic4_data[LOWLIGHT_MAX_LIGHT];
    Demosaic4_reserve   demosaic4_reserve;
} Chromatix_demosaic4_type;

typedef struct
{
     //0x207 chroma filter
    tuning_control_type         control_chroma_filter;
    trigger_point_type          chroma_filter_trigger_lowlight;
    int                         chroma_filter_enable_index;
    Chroma_filter_type          chroma_filter[LOWLIGHT_MAX_LIGHT]; //2 light condition

    //0x207 luma filter , 0x300 new type
    tuning_control_type         control_LF;
    trigger_point_type          LF_low_light_trigger;
    trigger_point_type          LF_bright_light_trigger;
    int LF_enable_index;
    chromatix_luma_filter_type  LF_config_low_light;
    chromatix_luma_filter_type  LF_config_normal_light;
    chromatix_luma_filter_type  LF_config_bright_light;
} chromatix_CL_filter_type;

typedef struct
{
    int                                 cc_enable;                  // 0x303
    int                                 cc_control_enable;          // 0x303
    tuning_control_type                 control_cc;                 // default 0 for lux_index
    trigger_point_type                  cc_trigger;
    chromatix_CCT_trigger_type          CC_H_trigger;               // 306
    chromatix_CCT_trigger_type          CC_A_trigger;
    chromatix_CCT_trigger_type          CC_D50_trigger;             // 0x304
    chromatix_CCT_trigger_type          CC_Daylight_trigger;
    // use led influence params from AEC, sensitivity_low, senstivity_high, sensitivity_off
    float                               CC_LED_start;
    float                               CC_LED_end;
    float                               CC_strobe_start;
    float                               CC_strobe_end;

    //existing set is for TL84
    chromatix_color_correction_type     TL84_color_correction;      // default
    chromatix_color_correction_type     lowlight_color_correction;  // LowLight
    chromatix_color_correction_type     D50_color_correction;       // 0x304
    chromatix_color_correction_type     D65_color_correction;       // default
    chromatix_color_correction_type     A_color_correction;         // A
    chromatix_color_correction_type     H_color_correction;         // H //306
    chromatix_color_correction_type     outdoor_color_correction;   // NEW, use the same trigger as gamma outdoor trigger
    chromatix_color_correction_type     LED_color_correction;
    chromatix_color_correction_type     STROBE_color_correction;
} chromatix_CC_type;

typedef struct
{
    int                         gamma_enable;           //0x303
    int                         gamma_control_enable;   //0x303
    tuning_control_type         control_gamma;          //default 0 for lux_index

    trigger_point_type          gamma_lowlight_trigger;
    trigger_point_type          gamma_outdoor_trigger;

    chromatix_gamma_table_type  default_gamma_table;    // Default
    chromatix_gamma_table_type  lowlight_gamma_table;   // LowLight
    chromatix_gamma_table_type  outdoor_gamma_table;    // Outdoor
} chromatix_gamma_type;

typedef struct
{
    int                                 cv_enable;                          // 0x303 // whole module enable/disable
    int                                 cv_control_enable;                  // 0x303
    tuning_control_type                 control_cv;                         // default 0 for lux_index
    trigger_point_type                  cv_trigger;
    chromatix_CCT_trigger_type          CV_H_trigger;                       // 306
    chromatix_CCT_trigger_type          CV_A_trigger;
    chromatix_CCT_trigger_type          CV_D50_trigger;                     // 0x304
    chromatix_CCT_trigger_type          CV_Daylight_trigger;

    chromatix_color_conversion_type     TL84_color_conversion;              // TL84
    chromatix_color_conversion_type     A_color_conversion;                 // A
    chromatix_color_conversion_type     D50_color_conversion;               // D50
    chromatix_color_conversion_type     daylight_color_conversion;          // D65
    chromatix_color_conversion_type     lowlight_color_conversion;          // LowLight
    chromatix_color_conversion_type     H_color_conversion;                 // H // 306
    chromatix_color_conversion_type     outdoor_color_conversion;           // Outdoor  (205)

    chromatix_color_conversion_type     mono_color_conversion;              // Mono
    chromatix_color_conversion_type     sepia_color_conversion;             // Sepia
    chromatix_color_conversion_type     negative_color_conversion;          // Negative
    chromatix_color_conversion_type     aqua_color_conversion;              // Aqua

 //we can remove BSM params because BSM can have separate header

    float                               saturated_color_conversion_factor;  // SatFactor
} chromatix_CV_type;

typedef struct
{
    int                     LA_enable;              // 0x207 default false
    int                     LA_control_enable;      // 0x303
    tuning_control_type     control_la;             // 205 default 0 for lux_index  //new205 mods
    trigger_point_type      la_brightlight_trigger;

    LA_args_type            LA_config;              //this is for indoor
    LA_args_type            LA_config_outdoor;      //new205 mods

    int                     la_luma_target_adj;     //0x300 mods default -7. we reduce luma target when LA is on. This parameter is for 7k,6k,8k.
} chromatix_LA_type;

typedef struct
{
    int                     CS_enable;          //0x303
    int                     CS_control_enable;
    int                     MCE_enable;
    int                     MCE_control_enable;
    tuning_control_type     control_cs;         //default 0 for lux_index
    trigger_point_type      cs_lowlight_trigger;
    cs_luma_threshold_type  cs_lowlight;
    cs_luma_threshold_type  cs_normal;
    unsigned char           chroma_thresh_BW;
    unsigned char           chroma_thresh_color;
    mce_type                mce_config;         //205
} chromatix_CS_MCE_type;

typedef struct
{
    float cr;
    float cb;
} sce_shift_vector;

typedef struct
{
    int                         sce_enable;
    int                         sce_control_enable;         // 0x303
    // 0x207
    tuning_control_type         control_SCE;
    trigger_point_type          SCE_trigger;                // under lowlight reduce SCE mapping to none
    chromatix_CCT_trigger_type  SCE_H_trigger;              // H // 306
    chromatix_CCT_trigger_type  SCE_A_trigger;
    chromatix_CCT_trigger_type  SCE_D65_trigger;

    sce_cr_cb_triangle_set      origin_triangles_TL84;      // existing set is for TL84
    sce_cr_cb_triangle_set      destination_triangles_TL84;
    sce_shift_vector            shift_vector_TL84;
    sce_affine_transform_2d     outside_region_mapping;
     // for other illuminants
    sce_cr_cb_triangle_set      origin_triangles_A;
    sce_cr_cb_triangle_set      destination_triangles_A;
    sce_shift_vector            shift_vector_A;
    sce_cr_cb_triangle_set      origin_triangles_H;         // H 306
    sce_cr_cb_triangle_set      destination_triangles_H;    // H 306
    sce_shift_vector            shift_vector_H;             // H 306
    sce_cr_cb_triangle_set      origin_triangles_D65;
    sce_cr_cb_triangle_set      destination_triangles_D65;
    sce_shift_vector            shift_vector_D65;
} chromatix_SCE_type;

typedef struct
{
    /* Stats bounding box with hw rolloff */
    chromatix_wb_exp_stats_type         wb_exp_stats[AWB_STATS_MAX_LIGHT];  // StatConfig
    chromatix_rscs_stat_config_type     rscs_cnfig;
    bayer_histogram_config_type         bayer_histogram_config;
    bayer_grid_config_type              AWB_AEC_stats_window;
    bayer_grid_config_type              bayer_exp_window;                   // 0x301
} chromatix_grid_stats_type;

typedef struct
{
    float           skintone_Hmin;          // 1.0 to 3.0, default 1.5
    float           skintone_Hmax;          // 0.0 to 1.0, default 0.5
    float           skintone_Ymin;          // 0.0 to 0.3, default 0.1
    float           skintone_Ymax;          // 0.7 to 1.0, default 0.9
    float           skintone_S_HY_min;      // 0.0 to 0.4, default 0.05
    float           skintone_S_HY_max;      // 0.1 to 0.5, default 0.25
    float           skintone_S_LY_min;      // 0.0 to 0.5, default 0.25
    float           skintone_S_LY_max;      // 0.2 to 1.0, default 0.6
    unsigned char   skintone_percentage;    // threshold percentage (0 to 100), output=1 when skin pixels exceed this percentage
} chromatix_skin_detection_type;

typedef struct
{
    unsigned short ZSL_default_shutter_lag;
    unsigned short ZSL_max_Q_factor_reduction;
} chromatix_ZSL_type;

// 0x301
typedef struct
{
    float tolerance;
    float contrast_meter;
    float normal_lowlight_boost_meter;
    float brightlight_boost_meter;
} chromatix_HDR_type;

//0x304
//=============================================================================
//----------------8994 Video HDR-------------------
// Feature name : videoHDR.
// Applicale chipset: MSM8994 and later.
// New feature starting VFE4.6. Support interlaced HDR sensors, which have interlaced even/odd row pairs to capture long (T1) and short (T2) exposures.
// Implemented as 2 blocks: HDR_RECON, HDR_MAC.
// Reconstruction (RECON) block: generate two full resolution T1 and T2 frames from half resolution T1 and T2 fields, using interploation and field merging.
// Motion Adaptive Combine (MAC) block: combine full resolution T1 and T2 images into a linearized HDR image.
//
// Variable name: hdr_zrec_sel // 1u.
// Description: This flag selects zzHDR recon(1) or iHDR recon(0)
// Constraints:  This Flag Should be enabled only for VIDEO MODE and HDR Sensors.
// Default value: 0 (iHDR).
// Applicable chipset: 8998 and later
//
// Variable name: hdr_MSB_align // 1u.
// Description: MSB Align for HDR module
// Default value: 0
//
// Variable name: hdr_zrec_pattern // 2u.
// This value represents zzHDR different patterns.
// Default value: 0.
// Range: 0 to 3
// Effect: Select different zzHDR patterns.
//
// Variable name: hdr_zrec_first_rb_exp // 1u.
// This value represents the t1/t2 field for the first red/blue in zzHDR pattern.
// Default value: 0.
// Range: 0 to 1
// Effect: Identify the t1/t2 field for the first red/blue in different zzHDR patterns.
// Applicable chipset: 8998 and later
//
// Variable name: hdr_zrec_g_grad_dth_log2// 4u.
// This value represents threshold 2 for the green pixel gradient delta for directional interpolation.
// Default value: 5.
// Range: 0 to 12
// Effect: Controls the threshold of green pixel gradient delta for directional interpolation.
// Applicable chipset: 8998 and later
//
// Variable name: hdr_zrec_rb_grad_dth_log2// 4u.
// This value represents threshold 2 for the red/blue pixel gradient delta for directional interpolation.
// Default value: 5.
// Range: 0 to 12
// Effect: Controls the threshold of red/blue pixel gradient delta for directional interpolation.
// Applicable chipset: 8998 and later
//
// Variable name: hdr_zrec_prefilt_tap0// 6u.
// This value represents the strength of the Zigzag HDR prefilter
// Default value: 16.
// Range: 0 to 63
// Effect: Controls the strength of the zzHDR Red/Blue color difference prefileter.
// Applicable chipset: 8998 and later
//
// Variable name: hdr_zrec_g_grad_th1 // 12u.
// This value represents threshold 1 for the green pixel gradient for directional interpolation.
// Default value: 32.
// Range: 0 to 4095
// Effect: Controls the threshold of green pixel gradient for directional interpolation.
// Applicable chipset: 8998 and later
//
// Variable name: hdr_zrec_rb_grad_th1// 12u.
// This value represents threshold 1 for the red/blue pixel gradient for directional interpolation.
// Default value: 32.
// Range: 0 to 4095
// Effect: Controls the threshold of red/blue pixel gradient for directional interpolation.
// Applicable chipset: 8998 and later
//=============================================================================
typedef struct
{
    // config parameters: these parameters should be fixed for a specific sensor
    unsigned char   recon_first_field;          // 1u. Image starting field, 0=T1 start, 1=T2 start, determined by sensor property
    /*Set the 2 params below to 1 for power saving. RECON and MAC blocks are by-passed when exp_ratio=1. Performance will be tested later on real chips. */
    unsigned char   recon_linear_mode;          // 1u. Enable RECON linear mode (1=linear, 0=normal). When in linear mode and exp_ratio==1, RECON is bypassed.
    unsigned char   mac_linear_mode;            // 1u. Enable MAC linear mode (1=linear, 0=normal). When in linear mode and exp_ratio==1, MAC is bypassed except for applying/removing WB gain and clipping to 10bit.
    /*The 2 params below should always be set to 1 for 8994. For future chips with staggered HDR sensor, set module_cfg.hdr_recon_en=0, module_cfg.hdr_mac_en=1.*/
    unsigned char   hdr_recon_en;               // 1u. Enable RECON block of HDR.
    unsigned char   hdr_zrec_sel;               // 1u. RECON block selection . Value 1 for zzHDR and 0 for iHDR   - default: 0
    unsigned char   hdr_mac_en;                 // 1u. Enable MAC block of HDR.
    unsigned char   hdr_MSB_align;              // 1u. HDR MSB align, when 1 align to MSB   - default: 0

    // zzHDR parameters
    unsigned short  hdr_zrec_pattern;           // 2u, Zigzag HDR sensor pattern (0~3)   - default: 0
    unsigned short  hdr_zrec_first_rb_exp;      // 1u, Zigzag HDR first R/B exposure (0~1)   - default: 0
    unsigned short  hdr_zrec_g_grad_dth_log2;   // 4u. Zigzag HDR G gradient delta threshold log2 0~12   - default: 5
    unsigned short  hdr_zrec_rb_grad_dth_log2;  // 4u. Zigzag HDR R/B gradient delta threshold log2 0~12   - default: 5

    // RECON parameters
    unsigned short  recon_edge_lpf_tap0;        // 3uQ4, range 0-5, default 3. Tap0 value for low-pass filter applied before edge detection.
    unsigned short  recon_h_edge_dth_log2;      // 4u, range 4-8. log2(th2-th1) for horizontal edge switching function.
    unsigned short  recon_motion_dth_log2;      // 4u, range 4-8. log2(th2-th1) for motion switching function.
    unsigned short  recon_dark_dth_log2;        // 3u, range 0-4. log2(th2-th1) for dark noise switching function.

    // MAC parameters
    unsigned short  mac_motion_dilation;        // 3u, range 0-5. Size of motion dilation max filter, 5 means -5 to 5, i.e. 11-tap filter.
    unsigned short  mac_motion0_dt0;            // 6u, range 1-63. An additive term to noise estimate, to avoid dividing by zero.
    unsigned short  mac_low_light_dth_log2;     // 4u, range 2-14. log2(th2-th1) for low light switching.
    unsigned char   mac_smooth_enable;          // 1u. Enalbe motion adaptive smoothing, an optional step in MAC.
    unsigned short  mac_smooth_th1;             // 9uQ8, range 0-256. Th1 for motion adaptive smoothing.
    unsigned short  mac_smooth_dth_log2;        // 4u, range 2-8. log2(th2-th1) for motion adaptive smoothing.
    unsigned short  mac_smooth_tap0;            // 3uQ4, range 0-5. Tap0 value for the low-pass filter in motion adaptive smoothing.
} chromatix_videoHDR_reserve;

/*
0x304
tuning parameters for GTM and their default values:
    GTM_a_middletone        = 0.14; // the mapped middle tone of the scene
    GTM_temporal_w          = 0.92;  // damping strength for video
    GTM_middletone_w        = 3.6; // a weight to compute the key of image

    GTM_maxval_th           = 1024; // the max value of pixel do not mapping
    GTM_key_min_th          = 64; // the min of key
    GTM_key_max_th          = 4096; // the max of the key
    GTM_key_hist_bin_weight = 0.5; // histogram bin weight
    GTM_Yout_maxval         = 16383; // max value of Yout
*/
typedef enum
{
    GTM_BRIGHT_LIGHT = 0,
    GTM_NORMAL_LIGHT,
    GTM_LOW_LIGHT,
    GTM_MAX_LIGHT
} GTM_light_type;

// Variable name:     GTM_minval_th
// Brief description of the parameter:  The minimum luma in image is not larger than this threshold.
// This threshold always keep minimum luma smaller than a preset value.
// Data Range   0 ~ 1024
// Default value:  128
// Variable name:     reserved_1,reserved_2
// reserved parameters for future usage
typedef struct
{
    int     GTM_maxval_th;
    int     GTM_key_min_th;
    int     GTM_key_max_th;
    float   GTM_key_hist_bin_weight;
    int     GTM_Yout_maxval;
    int     GTM_minval_th;
    float   reserved_1;
    float   reserved_2;
} chromatix_GTM_reserve;

typedef struct
{
    float GTM_a_middletone;
    float GTM_temporal_w;
    float GTM_middletone_w;
    float GTM_max_percentile;
    float GTM_min_percentile;
} chromatix_GTM_core;

typedef struct
{
    trigger_point_type      gtm_trigger;
    trigger_point2_type     aec_sensitivity_ratio;  // For CONTROL_AEC_EXP_SENSITIVITY_RATIO
    chromatix_GTM_core      gtm_core;
    chromatix_GTM_reserve   gtm_reserve;
} chromatix_GTM;

typedef struct
{
    int                 gtm_enable;
    int                 gtm_control_enable;
    tuning_control_type control_gtm;
    chromatix_GTM       gtm[MAX_SETS_FOR_TONE_NOISE_ADJ];    // 6 sets
} chromatix_GTM_type;

typedef struct
{
    trigger_point2_type aec_sensitivity_ratio;      // For CONTROL_AEC_EXP_SENSITIVITY_RATIO
    trigger_point2_type exp_time_trigger;           // For CONTROL_EXP_TIME_RATIO
    // RECON parameters
    unsigned short      recon_min_factor;           // 5uQ4, range 0-31. Scaling factor to force vertical direction: if 16*d_min>min_factor*d_sec_min, force vertical edge direction
    unsigned short      recon_flat_region_th;       // 10u. Threshold for flat region. If d_max<th, consider as flat region and force vertical edge direction.
    unsigned short      recon_h_edge_th1;           // 10u. Th1 for horizontal edge switching function.
    unsigned short      recon_motion_th1;           // 10u. Th1 for motion switching function.
    unsigned short      recon_dark_th1;             // 10u. Th1 for dark noise switching function.

    // zzHDR parameters
    unsigned short      hdr_zrec_prefilt_tap0;      //6u. Zigzag HDR prefilter side-taps (0~64)  - default: 16
    unsigned short      hdr_zrec_g_grad_th1;        //12u. Zigzag HDR G gradient threshold_1   - default: 32
    unsigned short      hdr_zrec_rb_grad_th1;       //12u. Zigzag HDR R/B gradient threshold_1,   - default: 32
    // MAC parameters
    unsigned short      mac_motion0_th1;            // 10u. Noise floor value for motion detection. Noise model is: noise = th2*sqrt(gain*itensity)+th1.
    unsigned short      mac_motion0_th2;            // 8u. Noise-luma slope for motion detection.
    unsigned short      mac_motion_strength;        // 5uQ4, range 0-16. Scaling factor for motion adaptation strength in MAC.
    unsigned short      mac_low_light_th1;          // 14u. Th1 for low light switching.
    unsigned short      mac_low_light_strength;     // 5uQ4, range 0-16. Scaling factor for low light switching.
    unsigned short      mac_high_light_th1;         // 14u. Th1 for high light switching.
    unsigned short      mac_high_light_dth_log2;    // 4u, range 2-14. log2(th2-th1) for high light switching.
} chromatix_videoHDR_core;

typedef struct
{
    int                         enable;
    int                         control_enable;
    tuning_control_type         control_hdr;                                    // CONTROL_AEC_EXP_SENSITIVITY_RATIO or CONTROL_EXP_TIME_RATIO
    chromatix_videoHDR_core     videoHDR_core_data[MAX_SETS_FOR_TONE_NOISE_ADJ];
    chromatix_videoHDR_reserve  videoHDR_reserve_data;
} chromatix_videoHDR_type;

//face detection and recognition
//0x301

#define ANGLE_0     0x00001001  /* Top of the head facing from 345 degrees to 15  degrees */
#define ANGLE_1     0x00002002  /* Top of the head facing from 15  degrees to 45  degrees */
#define ANGLE_2     0x00004004  /* Top of the head facing from 45  degrees to 75  degrees */
#define ANGLE_3     0x00008008  /* Top of the head facing from 75  degrees to 105 degrees */
#define ANGLE_4     0x00010010  /* Top of the head facing from 105 degrees to 135 degrees */
#define ANGLE_5     0x00020020  /* Top of the head facing from 135 degrees to 165 degrees */
#define ANGLE_6     0x00040040  /* Top of the head facing from 165 degrees to 195 degrees */
#define ANGLE_7     0x00080080  /* Top of the head facing from 195 degrees to 225 degrees */
#define ANGLE_8     0x00100100  /* Top of the head facing from 225 degrees to 255 degrees */
#define ANGLE_9     0x00200200  /* Top of the head facing from 255 degrees to 285 degrees */
#define ANGLE_10    0x00400400  /* Top of the head facing from 285 degrees to 315 degrees */
#define ANGLE_11    0x00800800  /* Top of the head facing from 315 degrees to 345 degrees */

#define ANGLE_ALL   0x00ffffff  /* All angles are detected                                */
#define ANGLE_NONE  0x00000000  /* None of the angles will be detected                    */

typedef struct
{
    //  face detection tuning parameters
    int minimum_face_width;
    int maximum_face_width;
    int maximum_num_face_to_detect;
    int angles_to_detect_pose_front;
    int angles_to_detect_pose_half_profile;
    int angles_to_detect_pose_profile;
} chromatix_face_detection_type;

typedef struct
{
    // face recognition tuning parameters
    int max_num_users;     // maximum number of users in the database;
    int max_data_per_user; // maximum number of data per user in the                //database;

    // the above two perimeters govern the upper limit of how big the database can grow to
    // the database size is approximately:
    // 32 (header) + 152 x (# people registered) + 148 x (# data/images registered overall) bytes
} chromatix_face_recognition_type;

typedef struct
{
    int NZ_HV[8];
    int NZ_D[8];
    int Fx[9];
} chromatix_CAC_reserve;

typedef struct
{
    short                   ThSaturation;
    int                     ThEdge;

    float                   PossibleCAPixelRGHighThreshold;
    float                   PossibleCAPixelRGLowThreshold;
    float                   PossibleCAPixelBGHighThreshold;
    float                   PossibleCAPixelBGLowThreshold;

    float                   RGDifferenceHighThreshold;
    float                   RGDifferenceLowThreshold;
    float                   BGDifferenceHighThreshold;
    float                   BGDifferenceLowThreshold;

    chromatix_CAC_reserve   reserve;
} chromatix_chroma_aliasing_correction_type;

// Feature name: Dynamic Color Shading Correction 0x303 (Tintless) new version 0x304
typedef struct
{
    int                 tintless_enable;
    trigger_point_type  tintless_lowlight_trigger;
    unsigned char       tintless_threshold;
    unsigned char       tintless_high_accuracy_mode;
    unsigned char       tintless_update_delay;
} chromatix_color_tint_correction_type;

//=============================================================================
// CDS Hysteresis Trigger
//=============================================================================
typedef struct
{
    tuning_control_type     control_CDS;            //DEFAULT 0 lux idx
    trigger_point_type      cds_trigger;
} chromatix_CDS_type;

typedef struct
{
    trigger_point2_type     aec_sensitivity_ratio;      // For CONTROL_AEC_EXP_SENSITIVITY_RATIO
    trigger_point2_type     drc_gain_trigger;       // CONTROL_DRC_GAIN
    float                   drc_sat_adj_factor;
} ADRC_ACE_adj_type;

typedef struct
{
    trigger_point2_type     adrc_lux_idx_trigger;
    ADRC_ACE_adj_type       adrc_ace_adj_data[MAX_SETS_FOR_TONE_NOISE_ADJ];
}ADRC_ACE_core_adj_type;

typedef struct
{
    int                     adrc_ace_adj_enable;
    tuning_control_type     control_adrc_ace;       // CONTROL_AEC_EXP_SENSITIVITY_RATIO or CONTROL_DRC_GAIN

    ADRC_ACE_core_adj_type  adrc_core_adj_data[MAX_SETS_FOR_ADRC_LUX_ADJ];
} Chromatix_ADRC_ACE_adj_type;

typedef struct
{
    sce_cr_cb_triangle_set      origin_triangles_TL84;
    sce_cr_cb_triangle_set      destination_triangles_TL84;
    sce_shift_vector            shift_vector_TL84;
    sce_cr_cb_triangle_set      origin_triangles_A;
    sce_cr_cb_triangle_set      destination_triangles_A;
    sce_shift_vector            shift_vector_A;
    sce_cr_cb_triangle_set      origin_triangles_H;
    sce_cr_cb_triangle_set      destination_triangles_H;
    sce_shift_vector            shift_vector_H;
    sce_cr_cb_triangle_set      origin_triangles_D65;
    sce_cr_cb_triangle_set      destination_triangles_D65;
    sce_shift_vector            shift_vector_D65;
} ADRC_SCE_CCT_type;

typedef struct
{
    trigger_point2_type     aec_sensitivity_ratio;      // For CONTROL_AEC_EXP_SENSITIVITY_RATIO
    trigger_point2_type     drc_gain_trigger;
    ADRC_SCE_CCT_type       adrc_sce_cct_data;
 }ADRC_SCE_core_type;

typedef struct
{
    int                         adrc_sce_adj_enable;
    tuning_control_type         control_adrc_sce;       // CONTROL_AEC_EXP_SENSITIVITY_RATIO or CONTROL_DRC_GAIN
    chromatix_CCT_trigger_type  SCE_H_trigger;
    chromatix_CCT_trigger_type  SCE_A_trigger;
    chromatix_CCT_trigger_type  SCE_D65_trigger;

    sce_affine_transform_2d     outside_region_mapping;
    ADRC_SCE_core_type          adrc_sce_core_data[MAX_SETS_FOR_ADRC_SCE_ADJ];
} Chromatix_ADRC_SCE_type;

//=============================================================================
// post processing
//=============================================================================
typedef struct
{
    chromatix_HDR_type                          chromatix_HDR;
    chromatix_videoHDR_type                     chromatix_video_HDR;//0x304
    chromatix_face_detection_type               chromatix_face_detection;
    chromatix_face_recognition_type             chromatix_face_recognition;
    chromatix_chroma_aliasing_correction_type   chromatix_chroma_aliasing_correction;
    chromatix_color_tint_correction_type        chromatix_color_tint_correction;
    chromatix_LTM_type                          chromatix_LTM_data;
    chromatix_CDS_type                          chromatix_chroma_sub_sampling;
} chromatix_post_processing_type;

//=============================================================================
//=============================================================================
// VFE overall
//=============================================================================
//=============================================================================

typedef struct
{
    //=========================================================================
    // rolloff trigger. Rolloff tables are in common header
    //=========================================================================
    chromatix_rolloff_trigger_type          chromatix_rolloff_trigger;          //0x303

    //=========================================================================
    // channel balance
    //=========================================================================
    chromatix_channel_balance_gains_type    chromatix_channel_balance_gains;    // ChannelGain


    //=========================================================================
    // Adaptive Bayer Filter 2    (ABF2)  new 8K, 7x30, 8x55 and later chips
    // no change
    //=========================================================================
    chromatix_ABF2_type                     chromatix_ABF2;

    chromatix_ABF3_type                     chromatix_ABF3;                     //0x303
    Chromatix_HDR_ABF3_adj_type             chromatix_hdr_abf3_adj_data;

    chromatix_GIC_type                      chromatix_GIC;                      //0x304

    //=========================================================================
    // Bad Pixel Correction
    //=========================================================================
    chromatix_BPC_type                      chromatix_BPC;

    //=========================================================================
    // Bad Pixel Correction2
    //=========================================================================
    Chromatix_BPC2_type                     chromatix_BPC2_data;

    //=========================================================================
    // Demosaic3 updated in 0x303
    //=========================================================================
    chromatix_demosaic_type                 chromatix_demosaic;
    Chromatix_demosaic4_type                chromatix_demosaic4;                // 0x0305

    //=========================================================================
    // Color Correction     (ColorCorr)
    // added outdoor CC
    // changed name to remove chromatix_, changed yhi_ylo to lowlight
    // removed LED , strobe trigger in 0x300, use sensitivity trigger instead
    //=========================================================================
    chromatix_CC_type                       chromatix_CC;

    //=========================================================================
    // Gamma
    // change to 512 entries
    // changed name to remove chromatix_rgb_, changed yhi_ylo to lowlight, removed backlit gamma, needs to use LA
    //=========================================================================
    chromatix_gamma_type                    chromatix_gamma;

    /*
    GTM parameters for constructing GTM LUT
    */
    chromatix_GTM_type                      chromatix_gtm;                      //0x304

    //=========================================================================
    // Color Conversion   (ColorConv)
    // changed name to remove chromatix_, changed yhi_ylo to lowlight
    //=========================================================================
    chromatix_CV_type                       chromatix_CV;
    Chromatix_ADRC_ACE_adj_type             chromatix_adrc_ace_adj_data;

    //=========================================================================
    // Luma Adaptation
    // removed old (8K)struct, added new struct LA_type
    //=========================================================================
    chromatix_LA_type                       chromatix_LA;

    //=========================================================================
    // Skin Tone Noise Reduction
    //=========================================================================
    Chromatix_Skintone_NR_type              chromatix_skin_nr_data;

    //=========================================================================
    // HW CAC2
    //=========================================================================
    Chromatix_HW_CAC2_type                  chromatix_hw_CAC2_data;

    //=========================================================================
    // 5x5 ASF      (5X5ASF)
    //=========================================================================
    chromatix_ASF_5x5_type                  chromatix_ASF_5x5;

    //=========================================================================
    // Chroma Suppression  (ChromaSupp) & MCE (memory color enhancement)
    // no change
    //=========================================================================
    chromatix_CS_MCE_type                   chromatix_CS_MCE;

    //=========================================================================
    // skin color enhancement (SCE)
    // no change, move trigger to beginning
    //=========================================================================
    chromatix_SCE_type                      chromatix_SCE;
    Chromatix_ADRC_SCE_type                 chromatix_adrc_sce_data;

    //=========================================================================
    // STATS config
    //=========================================================================
    chromatix_grid_stats_type               chromatix_grid_stats;

    //=========================================================================
    // Skin Tone Detection (Skintone)
    //=========================================================================
    chromatix_skin_detection_type           chromatix_skin_detection;

    // color_xform_format video_xform; 0x304->0x305
    Chromatix_color_space_transform_type    chromatix_xform_data;
} chromatix_VFE_type;

//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//  CHROMATIX HEADER definition
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================

typedef struct
{
    //=========================================================================
    // Chromatix header version info (MUST BE THE FIRST PARAMETER)
    //=========================================================================
    ChromatixVersionInfoType        chromatix_version_info;

    //=========================================================================
    // VFE structs
    //=========================================================================
    chromatix_VFE_type              chromatix_VFE;

    // 306
    exposure_struct_type            exposure_data;

    //=========================================================================
    // ASD
    //=========================================================================
    ASD_VFE_struct_type             ASD_algo_data;

    //=========================================================================
    // ZSL
    //=========================================================================
    chromatix_ZSL_type              chromatix_ZSL;

    //=========================================================================
    // post processing modules
    //=========================================================================
    chromatix_post_processing_type  chromatix_post_processing;
} chromatix_parms_type;

#endif
