//======================================================================
// Copyright (c) 2015 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//======================================================================

#ifndef CHROMATIX_3A_H
#define CHROMATIX_3A_H

#include "chromatix.h"

typedef struct
{
    unsigned short chromatix_version;
    unsigned short AAA_header_version;
} AAA_version_type;

//=============================================================================
//                      CONSTANTS
//=============================================================================

// Generic package for 3 RGB float parameters
typedef struct
{
    float r;
    float g;
    float b;
} chromatix_float_rgb_type;

//=============================================================================
// AAA sync related data types; Used for dual sensor configurations.
//=============================================================================

typedef enum
{
    AWB_SYNC_MODE_DISABLE = 0,          // Not synchronized (Default)
    AWB_SYNC_MODE_SAME_SCALERS,         // Copy master's WBC to slave
    AWB_SYNC_MODE_STATIC_CORRECTION,    // Master's WBC multiplied by static correction
    AWB_SYNC_MODE_DYNAMIC_CORRECTION,   // Dynamic olcm
} chromatix_awb_sync_mode_type;

typedef struct
{
    chromatix_awb_sync_mode_type    mode;               // Default = AWB_SYNC_MODE_DISABLE
    chromatix_float_rgb_type        correction;         // rgb gains for fixed correction.
    float                           aggressiveness;     // Speed of correction changes
    float                           trans_ms;           // (msec) Transition time  on mode changes. Default = 1000ms
} chromatix_awb_sync_type;

typedef enum
{
    AEC_SYNC_MODE_DISABLE = 0,  // Not synchronized (Default)
    AEC_SYNC_MODE_ENABLE,       // Match master sensor's exposure
} chromatix_aec_sync_mode_type;

typedef struct
{
    chromatix_aec_sync_mode_type    mode;       // Default = AEC_SYNC_MODE_DISABLE
    float                           correction; // exposure multiplier, compensating for different sensetivity and/or aperture. Default 1.0f
    float                           trans_ms;   // (msec) Transition time  on mode changes. Default = 1000ms
} chromatix_aec_sync_type;

typedef struct
{
    chromatix_aec_sync_type aec;
    chromatix_awb_sync_type awb;
} chromatix_aaa_sync_type;

typedef enum
{
    FACE_PRIORITY_CENTER = 0,   // default
    FACE_PRIORITY_BIG,          // big face
    FACE_MAX_PRIORITY,
    FACE_INVALID_PRIORITY = FACE_MAX_PRIORITY
} chromatix_face_priority_type;

//=============================================================================
//  ASD basic struct
//=============================================================================
typedef struct
{
    //Snow/cloudy Scene Detection

    int             snow_scene_detection_enable;    // TRUE enables the feature from AEC perspective, FALSE disables the feature.
    unsigned long   y_cloudy_snow_threshold;        // AE region?s min luma to be considered a potential snow/cloudy region.
    unsigned long   awb_y_max_in_grey;              // Any AE region above this threshold is considered potential snow/cloudy region.  These regions will not have WB data because WB considered too bright and above YMAX WB configuration.
    unsigned long   min_snow_cloudy_sample_th;      // If count of snow/cloudy regions detected above this threshold, we consider the scene as snow/cloudy.
    unsigned long   extreme_snow_cloudy_sample_th;  // if count of detected snow regions above this threshold, scene is considered extreme snow/cloudy scene.  Luma offset is maxed.
    float           extreme_luma_target_offset;     // Maximum luma offset that can be applied when scnow scene is detected.  This happens when extreme_snow_cloudy_sample_th is reached or exceeded.  Luma offset is gradually reduced for darker scenes until eventually made 0 for indoor cases, this is based on exp_index. (cannot use lux_idx due to rapid change based on frame luma, will cause luma offset to be unstable).
    unsigned int    snow_scene_indoor_index;        // Use dedicated index for indoor/outdoor distinguish
    unsigned int    snow_scene_outdoor_index;       // Use dedicated index for indoor/outdoor distinguish
    unsigned long   severe_snow_scene_cap;          //? At what point will ?extreme_luma_target_offset? be applied, value is 0 to 255 for severity.  255 means snow scene detection must report 255 severity for extreme_luma_target_offset to be applied.
    float           snowscene_aggressiveness;
    unsigned char   ui_snow_cloudy_display_th;
} snow_scene_detect_type;

typedef struct
{
    int             backlight_detection_enable;         //TRUE enables the feature from AEC perspective, FALSE disables the feature.
    float           histogram_offset;
    unsigned long   low_luma_threshold;                 // Histogram samples which luma is below this threshold are added to low luma count.
    unsigned long   high_luma_threshold;                // Histogram samples which luma is above this threshold are added to high luma count.
    float           low_luma_count_percent_threshold;   // If count of low luma samples exceed this percentage of total samples, we consider potential backlight case.
    float           high_luma_count_percent_threshold;  // If count of high luma samples exceed this percentage of total samples, we consider potential backlight case.
    float           kept_pixel_ratio_th;
    int             backlight_max_la_luma_target_offset; // Maximum luma target adjustment when backlight is detected.   We expect to increase luma target.
    float           backlit_aggressiveness;
    float           max_percent_threshold;              //defines interpolation range of backlit severity in         //histogram detector.
    unsigned char   ui_backlit_display_th;
} backlit_scene_detect_type;

typedef struct
{
    float backlight_high_threshold;
    float backlight_low_threshold;
    float portrait_high_threshold;
    float portrait_low_threshold;
} auto_hdr_detect_type;

typedef struct
{
    int             landscape_detection_enable;                 //TRUE enables the feature ,FALSE disables the feature.
    float           landscape_red_boost_factor;
    float           landscape_green_boost_factor;
    float           landscape_blue_boost_factor;
    float           min_blue_green_content_detection_threshold; //min detection
    float           max_blue_green_content_detection_threshold; //stop interpolation detection range
    int             green_offset_rg;                            //for extreme green zone boundary config
    int             green_offset_bg;                            //for extreme green zone boundary config
    float           asd_ext_blue_th;                            //extreme blue region detection
    float           asd_ext_green_th_r;                         //extreme green region detection
    float           asd_ext_green_th_b;                         //extreme green region detection
    float           aggressiveness;                             //response of temporal filter to severity change
    long            lux_idx_indoor;                             //lux_idx above this threshold disabled landscape detection
    long            lux_idx_outdoor;                            //lux idx below this threshold has full application of landscape scene compensation.  Between indoor and outdoor, we interpolate severity.
    unsigned char   ui_landscape_display_th;
} landscape_scene_detect_type;

typedef struct
{
    int                         portrait_detection_enable;
    float                       skin_color_boost_factor;
    float                       min_face_content_threshold;
    float                       max_face_content_threshold;
    filter_sharpen_degree_type  soft_focus_degree_7_7;
    filter_sharpen_degree_type  soft_focus_degree_5_5;
    float                       aggressiveness;
    unsigned char               ui_portrait_display_th;
} portrait_scene_detect_type;

typedef struct
{
    int             hazy_detection_enable;
    float           hazy_running_frequency;
    float           hazy_detection_threshold_on;
    float           hazy_detection_threshold_off;
    unsigned char   ui_hazy_display_th;
    float           reserved[20];
} hazy_scene_detect_type;

//=============================================================================
//           AWB tuning structurse
//=============================================================================
#define AGW_NUMBER_GRID_POINT             241
#define MAX_LOW_LIGHT_AWB_LUT_SIZE        6
#define MAX_LED_MIX_LEVEL                 16

//=============================================================================
// AWB tuning parameters are divided into three groups:
// Level-0: must tune parameters
// Level-1: Performance improvement parameters
// Level-3: Least important parameters. Provided for AWB tuning flexibility
//=============================================================================

//=============================================================================
// Level-0
//=============================================================================

/**
* Reference light definitions
**/
typedef enum
{
    AGW_AWB_D65                 = 0,                // D65
    AGW_AWB_D75,                                    // D75
    AGW_AWB_A,                                      // A
    AGW_AWB_WARM_FLO,                               // TL84
    AGW_AWB_COLD_FLO,                               // CW
    AGW_AWB_HORIZON,                                // H
    AGW_AWB_D50,                                    // D50
    AGW_AWB_CUSTOM_FLO,                             // CustFlo
    AGW_AWB_NOON,                                   // Noon
    AGW_AWB_CUSTOM_DAYLIGHT,
    AGW_AWB_CUSTOM_A,
    AGW_AWB_U30,
    AGW_AWB_CUSTOM_DAYLIGHT1,
    AGW_AWB_CUSTOM_DAYLIGHT2,
    AGW_AWB_CUSTOM_FLO1,
    AGW_AWB_CUSTOM_FLO2,
    AGW_AWB_MAX_LIGHT,
    AGW_AWB_INVALID_LIGHT       = AGW_AWB_MAX_LIGHT,
    DAY_LINE_1                  = AGW_AWB_MAX_LIGHT,
    DAY_LINE_2,
    FLINE,
    A_LINE_1,
    A_LINE_2,
    AGW_AWB_HYBRID,                                 // Daylight, only used for algorithm, not data
    AGW_AWB_MAX_ALL_LIGHT       = AGW_AWB_HYBRID,   // Don't count the hybrid
    AGW_AWB_INVALID_ALL_LIGHT   = AGW_AWB_MAX_ALL_LIGHT
} chromatix_awb_all_light_type;

typedef enum
{
    AWB_LOWLIGHT_A = 0,
    AWB_LOWLIGHT_TL84,
    AWB_LOWLIGHT_D65,
    AWB_LOWLIGHT_LED,
    AWB_LOWLIGHT_STROBE,
    AWB_MAX_LOWLIGHT
} AWB_LOWLIGHT_type;

/* Manual white balance gains */
typedef struct
{
    /* 9-bit, Q7, unsigned */
    float r_gain;                           // RGain

    /* 9-bit, Q7, unsigned */
    float g_gain;                           // GGain

    /* 9-bit, Q7, unsigned */
    float b_gain;                           // BGain
} chromatix_manual_white_balance_type;

typedef struct
{
    float RG_ratio;                  // RedG
    float BG_ratio;                  // BlueG
} chromatix_awb_reference_type;

typedef struct
{
    float red_gain_adj;               // RedAdj
    float blue_gain_adj;              // BlueAdj
} chromatix_awb_gain_adj_type;

typedef struct
{
    int                     AWB_purple_prevent_enable;
    tuning_control_type     control_purple_prevent;
    trigger_point_type      purple_prevent_trigger;
    float                   purple_sky_prevention_bg_threshold;
} AWB_purple_prevent_type;

typedef struct
{
    int indoor_weight;                // Indoor
    int outdoor_weight;               // Outdoor
    int inoutdoor_weight;             // InOut
} chromatix_awb_weight_vector_type;

/* AWB sample influence */
typedef struct
{
    float outdoor_influence;          // Outdoor
    float indoor_influence;           // Indoor
} chromatix_awb_sample_influence_type;

typedef struct
{
    float gyro_trigger;
    float accelerometer_trigger;
    float magnetometer_trigger;
    float DIS_motion_vector_trigger;
} AWB_motion_sensor_type;

typedef struct
{
    int     lux_index;
    float   green_rg_offset_adj;
    float   green_bg_offset_adj;
    float   outlier_dist_adj;
} low_light_adj_type;

typedef struct
{
    unsigned char       enable;
    low_light_adj_type  lut_entry[MAX_LOW_LIGHT_AWB_LUT_SIZE];
} awb_lowlight_adj_lut_type;

typedef struct
{
    unsigned short  CCT;
    unsigned short  LED1_setting;
    unsigned short  LED2_setting;
    float           rg_ratio;
    float           bg_ratio;
    float           flux;
    float           preflash_flux;
    float           cont_burst_flux;
} LED_mix_type;

typedef struct
{
    int                         dual_led_enable;
    float                       preflash_current_ratio;
    float                       continuous_burst_current_ratio;
    chromatix_awb_gain_adj_type led1_gain_adj;
    chromatix_awb_gain_adj_type led2_gain_adj;
    unsigned short              table_size;

    LED_mix_type                CCT_control[MAX_LED_MIX_LEVEL];
} chromatix_match_LED_lighting_table_type;

typedef struct
{
    int                           skin_ctrl_enable;
    trigger_point_type            skin_ref_exp_index_trigger;
    chromatix_awb_reference_type  skin_reference[AGW_AWB_MAX_LIGHT];
} awb_skin_control_type;

typedef struct
{
    int                         aux_sensor_enable;
    AWB_motion_sensor_type      AWB_motion_sensor_data;
} awb_aux_sensor_type;

typedef struct
{
    unsigned char                skip_frames;
    int                          stat_saturation_threshold;
    int                          all_outlier_heuristic_flag;
    int                          valid_per_th;
} awb_flow_control_type;

typedef struct
{
    unsigned int    awb_day_cluster_left_outlier_distance;
    unsigned int    awb_day_cluster_top_outlier_distance;
    unsigned int    awb_a_h_cluster_left_outlier_distance;
} awb_cluster_distance_type;

typedef struct
{
    unsigned char                   awb_interpolate_gain_adj_enable;
    trigger_point_type              exp_index_bright_light_gain_adjust;
    trigger_point_type              exp_index_lowlight_gain_adjust;
    tuning_control_type             control_gain_adj_lowlight;

    chromatix_awb_gain_adj_type     gain_adj_brightlight[AGW_AWB_MAX_LIGHT];
    chromatix_awb_gain_adj_type     gain_adj_normallight[AGW_AWB_MAX_LIGHT];
    chromatix_awb_gain_adj_type     gain_adj_lowlight[AGW_AWB_MAX_LIGHT];
} awb_adjust_gain_table_type;

typedef struct
{
    int                         outlier_distance;
    int                         outlier_distance_day;
    int                         outlier_distance_f;
    int                         outlier_distance_a;
    int                         outlier_distance_h;
    int                         outlier_distance_customday;
    int                         outlier_distance_customf;

    awb_cluster_distance_type   special_cluster_distances;
} awb_outlier_distance_type;

typedef struct
{
    chromatix_awb_reference_type              reference[AGW_AWB_MAX_LIGHT];
    long                                      indoor_index;
    long                                      outdoor_index;

    chromatix_manual_white_balance_type       awb_min_gains;
    chromatix_manual_white_balance_type       awb_max_gains;

    int                                       awb_self_cal_enable;
    float                                     awb_self_cal_adj_ratio_high;
    float                                     awb_self_cal_adj_ratio_low;

    awb_lowlight_adj_lut_type                 AWB_lowlight_LUT;
    awb_adjust_gain_table_type                awb_adjust_gain_table;
    awb_outlier_distance_type                 outlier_distances;
} awb_basic_tuning_type;

typedef struct
{
    int lowlight_measure_delta;

    int dark_r_threshold;
    int dark_g_threshold;
    int dark_b_threshold;

    int dark_r_threshold_lowlight;
    int dark_g_threshold_lowlight;
    int dark_b_threshold_lowlight;
} awb_dark_threshold_type;

typedef struct
{
    int   white_stat_y_threshold_high;
    float white_outlier_valid_ymax_ratio;
    int   white_stat_cnt_threshold;
    int   white_peak_separation_distance;
    int   white_aec_stable_range_threshold;
    float white_history_weight;
    float white_current_weight;
} awb_white_world_struct_type;

typedef struct
{
    float                     ref_b_bg_tl84;
    float                     extreme_range_perc_b;
    float                     blue_sky_pec;
    float                     blue_sky_pec_buffer;
    float                     threshold_extreme_b_percent;

    AWB_purple_prevent_type   AWB_purple_prevent;
} awb_blue_sky_type;

typedef struct
{
    awb_blue_sky_type     blue_sky_param;
    float                 snow_blue_gain_adj_ratio;
    float                 beach_blue_gain_adj_ratio;
} awb_blue_tuning_type;

typedef struct
{
    int grey_weight_day;
    int grey_weight_f;
    int grey_weight_a;
    int grey_weight_h;

    int white_weight_day;
    int white_weight_f;
    int white_weight_a;
    int white_weight_h;

    int exposure_adjustment;
} awb_heuristic_control_type;

typedef struct
{
    int   awb_history_buffer_size;

    int   day_stability_enable;
    int   f_stability_enable;
    int   a_stability_enable;
    int   h_stability_enable;

    int   awb_lock_day_enable;
    int   awb_lock_f_enable;
    int   awb_lock_a_enable;
    int   lock_exp_threshold_day;
    int   lock_exp_threshold_f;
    int   lock_exp_threshold_a;

    int   save_historyaverage2history;

    float awb_convergence_factor_camera;
    float awb_convergence_factor_video;
} awb_temporal_tuning_type;

typedef struct
{
    int   dominant_cluster_enable;
    float cluster_high_pec;
    float cluster_mid_pec;
    float cluster_low_pec;
    int   threshold_compact_cluster_valid;
    int   threshold_compact_cluster;
    int   compact_to_grey_dis;
    int   dominant_cluster_threshold;
} awb_cluster_tuning_type;

typedef struct
{
    int   enable_AWB_module_cal;
    float AWB_golden_module_R_Gr_ratio[AGW_AWB_MAX_LIGHT];
    float AWB_golden_module_Gb_Gr_ratio[AGW_AWB_MAX_LIGHT];
    float AWB_golden_module_B_Gr_ratio[AGW_AWB_MAX_LIGHT];
} awb_golden_module_info_type;

typedef struct
{
    float                                     awb_led_strobe_adjustment_factor;

    chromatix_match_LED_lighting_table_type   mix_LED_table;
    chromatix_awb_gain_adj_type               LED_gain_adj;
    chromatix_awb_gain_adj_type               strobe_gain_adj;
} awb_led_tuning_type;

//=============================================================================
// Feature name :
// Manual white balance gains for both snapshot and viewfinder.
//=============================================================================
typedef struct
{
    chromatix_manual_white_balance_type    MWB_tl84;                    // Flourescent
    chromatix_manual_white_balance_type    MWB_d50;                     // Sunny
    chromatix_manual_white_balance_type    MWB_A;                       // Tungsten
    chromatix_manual_white_balance_type    MWB_d65;                     // Cloudy/Shade
    chromatix_manual_white_balance_type    strobe_flash_white_balance;  // Strobe
    chromatix_manual_white_balance_type    led_flash_white_balance;     // LED
} awb_MWB_type;

//=============================================================================
// Feature name: awb_extended_outdoor_exp_trigger_type
// Outdoor exposure indeces for triggering special outdoor heuristics
//
// Variable name: directsun_exp_index
// exposure index for very bright sunlight (under direct sun light)
// default: 140
// range: [90-170]
// 3A version : supported only in 306 & 307
//
// Variable name: outdoor_heuristic_exposure_index_trigger
// exposure index based trigger points for running outdoor heurisitic
// default: outdoor_index
// range: [0, outdoor_index]
//
// Variable name: outdoor_exp_bright_shade_enable
// Enable/disable outdoor heuristic bright and shade exposure trigger points
// Default: 0 - disable
// range: 1 - enable, 0 - disable
//
// Variable name: outdoor_heuristic_exp_index_bright
// Exposure index based trigger point for bright condition
// Default: 170
// Range: [0, outdoor_index]
//
// Variable name: outdoor_heuristic_exp_index_shade
// Exposure index based trigger point for shade condition
// Default: 200
// Range: [0, (outdoor_index+indoor_index)/2]
//=============================================================================
typedef struct
{
    int directsun_exp_index;
    int outdoor_heuristic_exposure_index_trigger;
    int outdoor_exp_bright_shade_enable;
    int outdoor_heuristic_exp_index_bright;
    int outdoor_heuristic_exp_index_shade;
} awb_extended_outdoor_exp_trigger_type;

//=============================================================================
// Type definition of AWB subzone
//=============================================================================
typedef enum
{
    AWB_DAY_D75 = 0,                // Subzone above D75 reference point                                        (NOTE: D75 reference weight)
    AWB_DAY_D75_D65_1,              // 1st subzone between D75 and D65 reference points                         (NOTE: D75 reference weight)
    AWB_DAY_D75_D65_2,              // 2nd subzone between D75 and D65 reference points
    AWB_DAY_D75_D65_3,              // 3rd subzone between D75 and D65 reference points
    AWB_DAY_D65,                    // 4th subzone between D75 and D65 reference points                         (NOTE: d65 reference weight)
    AWB_DAY_D65_D50_1,              // 1st subzone between D65 and D50 reference points                         (NOTE: d65 reference weight)
    AWB_DAY_D65_D50_2,              // 2nd subzone between D65 and D50 reference points
    AWB_DAY_D65_D50_3,              // 3rd subzone between D65 and D50 reference points
    AWB_DAY_D50,                    // 4th subzone between D65 and D50 reference points                         (NOTE: d50 reference weight)
    AWB_DAY_NOON_LINE_0,            // Subzone above the shifted D65 reference point                            (NOTE: d65 reference weight)
    AWB_DAY_NOON_LINE_1,            // 1st subzone between the shifted D65 and the shifted D50 reference points (NOTE: d65 reference weight)
    AWB_DAY_NOON_LINE_2,            // 2nd subzone between the shifted D65 and the shifted D50 reference points
    AWB_DAY_NOON_LINE_3,            // 3rd subzone between the shifted D65 and the shifted D50 reference points (NOTE: Noon reference weight)
    AWB_DAY_NOON_LINE_4,            // 4th subzone between the shifted D65 and the shifted D50 reference points (NOTE: Noon reference weight)
    AWB_DAY_D50_FLO_1,              // 1st subzone between D50 and the center point of Flo line                 (NOTE: d50 reference weight)
    AWB_DAY_D50_FLO_2,              // 2nd subzone between D50 and the center point of Flo line
    AWB_DAY_D50_FLO_3,              // 3rd subzone between D50 and the center point of Flo line
    AWB_DAY_D50_FLO_4,              // 4th subzone between D50 and the center point of Flo line
    AWB_F_TL84,                     // 1sth subzone between CW and TL84 reference points                        (NOTE: TL84 reference weight)
    AWB_F_TL84_CW_2,                // 2nd subzone between CW and TL84 reference points
    AWB_F_TL84_CW_3,                // 3rd subzone between CW and TL84 reference points
    AWB_F_CW,                       // 4th subzone between CW and TL84 reference points                         (NOTE: CW referece weight)
    AWB_F_FLO_A_1,                  // 1st subzone between the center point of Flo line and A reference point
    AWB_F_FLO_A_2,                  // 2nd subzone between the center point of Flo line and A reference point
    AWB_F_FLO_A_3,                  // 3rd subzone between the center point of Flo line and A reference point
    AWB_A,                          // 4th subzone between the center point of Flo line and A reference point   (NOTE: A reference weight)
    AWB_AH_A_H_1,                   // 1st subzone between A and H reference points                             (NOTE: A reference weight)
    AWB_AH_A_H_2,                   // 2nd subzone between A and H reference points
    AWB_AH_A_H_3,                   // 3rd subzone between A and H reference points
    AWB_H,                          // 4th subzone between A and H reference points                             (NOTE: H reference weight)
    AWB_CUSTOM_DAYLIGHT1,           // Custom Day light 1                                                       (NOTE: Custom day reference weight)
    AWB_CUSTOM_DAYLIGHT2,           // Custom Day light 2                                                       (NOTE: Custom day1 reference weight)
    AWB_CUSTOM_DAYLIGHT3,           // Custom Day light 3                                                       (NOTE: Custom day2 reference weight)
    AWB_CUSTOM_FLO1,                // Custom F light 1                                                         (NOTE: Custom f reference weight)
    AWB_CUSTOM_FLO2,                // Custom F light 2                                                         (NOTE: Custom f1 reference weight)
    AWB_CUSTOM_FLO3,                // Custom F light 3                                                         (NOTE: Custom f2 reference weight)
    AWB_CUSTOM_A,                   // Custom A light                                                           (NOTE: Custom A reference weight)
    AWB_U30,                        // U30 light                                                                (NOTE: U30 reference weight)
    AWB_SUBZONE_DECISION_INVALID,
    AGW_AWB_SUBZONE_LIGHT           = AWB_SUBZONE_DECISION_INVALID
} awb_hybrid_decision_type;

//=============================================================================
// Type definition of Indoor CCM
//=============================================================================
typedef enum
{
    CCM_INDOOR_D65 = 0, // D65 CCM
    CCM_INDOOR_D50,     // D50 CCM
    CCM_INDOOR_CW,      // CW CCM
    CCM_INDOOR_TL84,    // TL84 CCM
    CCM_INDOOR_A,       // A CCM
    CCM_INDOOR_H,       // Horizon CCM
    CCM_INDOOR_NUM,
} ccm_indoor_entry;

//=============================================================================
// Type definition of Outdoor CCM
//=============================================================================
typedef enum
{
    CCM_OUTDOOR_SHADE = 0,  // Shade (around D75) CCM
    CCM_OUTDOOR_SUNNY,      // Suuny (daylight, D50) CCM
    CCM_OUTDOOR_GREEN,      // Green color CCM
    CCM_OUTDOOR_SKY,        // SKy color CCM
    CCM_OUTDOOR_SKIN,       // Skin color CCM
    CCM_OUTDOOR_LOWLIGHT,   // Low light CCM
    CCM_OUTDOOR_NUM,
} ccm_outdoor_entry;

//=============================================================================
// Type definition of Flash (LED) CCM
//=============================================================================
typedef enum
{
    CCM_FLASH_LED1 = 0,     //Flash CCM under full power for LED1
    CCM_FLASH_LED2,         //Flash CCM under full power for LED2
    CCM_FLASH_NUM,
} ccm_flash_entry;

#define AWB_FLASH_ADJUST_LUT 5

typedef struct
{
    int                            lux_index;
    float                          flash_sensitivity_ratio;
    chromatix_CCT_trigger_type     cct_threshold;
    chromatix_awb_gain_adj_type    adjust_ratio;
} awb_flash_lut_info;

//=============================================================================
// Feature name: awb_trigger_type_int
// trigger point structure with integer members
//=============================================================================
typedef struct
{
    int nstart;
    int nend;
} awb_trigger_type_int;

//=============================================================================
// Feature name: awb_trigger_type_float
// trigger point structure with float members
//=============================================================================
typedef struct
{
    float fstart;
    float fend;
} awb_trigger_type_float;

typedef struct
{
    float               rg_center;
    float               bg_center;
    int                 rg_radius_grid;
    int                 bg_radius_grid;
} awb_detect_zone_type;

#define DETECT_ZONE_NUM 8

typedef struct
{
    awb_detect_zone_type         detection_zone[DETECT_ZONE_NUM];
    awb_trigger_type_int         count_trigger;
    trigger_point_type           exp_index_trigger;
    float                        aux_cond[2];
    chromatix_awb_gain_adj_type  adjust_ratio;
} awb_outdoor_special_type;

#define WEIGHT_VECTOR_TABLE_SIZE    13

typedef struct
{
    int exposure_index;
    int weight_array[AGW_AWB_SUBZONE_LIGHT];
} awb_illuminant_weight_vector_type;

#define NUM_SPECIAL_COLOR_DETECT    8

typedef struct
{
    awb_detect_zone_type         detection_zone[DETECT_ZONE_NUM];
    trigger_point_type           exp_index;
    awb_trigger_type_int         count_offset;
    chromatix_awb_gain_adj_type  adjust_ratio;
} awb_single_color_type;

#define MISLEADING_COLOR_ZONE_NUM   30
#define GREEN_ZONE_ASSIST_NUM       5

typedef struct
{
    int                     zone_control;
    int                     trigger_type;
    int                     control_direction;
    awb_detect_zone_type    detect_zone;
    trigger_point_type      exp_index_trigger;
} awb_misleading_zone_type;

typedef struct
{
    int                                 enable;
    chromatix_color_correction_type     ccm;
} awb_extended_ccm_type;

typedef struct
{
    int                            awb_ccm_enable;
    trigger_point_type             awb_ccm_exp_index_trigger_outdoor;
    trigger_point_type             awb_ccm_exp_index_trigger_lowlight;
    trigger_point_type             awb_ccm_exp_index_trigger_LED_lowlight;
    awb_trigger_type_float         awb_ccm_LED_trigger;
    chromatix_CCT_trigger_type     awb_ccm_cct_trigger_outdoor;
    chromatix_CCT_trigger_type     awb_ccm_cct_trigger_indoor_day;
    chromatix_CCT_trigger_type     awb_ccm_cct_trigger_indoor_dayf;
    chromatix_CCT_trigger_type     awb_ccm_cct_trigger_indoor_fa;
    chromatix_CCT_trigger_type     awb_ccm_cct_trigger_indoor_ah;
    awb_extended_ccm_type          awb_ccm_indoor[CCM_INDOOR_NUM];
    awb_extended_ccm_type          awb_ccm_portrait[CCM_INDOOR_NUM];
    awb_extended_ccm_type          awb_ccm_lowlight[CCM_INDOOR_NUM];
    awb_extended_ccm_type          awb_ccm_outdoor[CCM_OUTDOOR_NUM];
    awb_extended_ccm_type          awb_ccm_flash[CCM_FLASH_NUM];
    awb_extended_ccm_type          awb_ccm_flash_lowlight[CCM_FLASH_NUM];
} awb_CCM_type;

typedef struct
{
    awb_illuminant_weight_vector_type      weight_vector[WEIGHT_VECTOR_TABLE_SIZE];
    int                                    distance_weight_table[AGW_NUMBER_GRID_POINT];
} awb_weight_vector_type;

typedef struct
{
    int                                     awb_exposure_outdoor_heuristic;
    awb_outdoor_special_type                awb_outdoor_green_adjust;
    awb_outdoor_special_type                awb_outdoor_bright_blue_sky_adjust;
    awb_outdoor_special_type                awb_outdoor_not_enough_stat_adjust;
    awb_outdoor_special_type                awb_outdoor_blue_ground_adjust;
    awb_outdoor_special_type                awb_outdoor_cloud_sky_adjust;
    awb_outdoor_special_type                awb_outdoor_blue_sky_adjust;
    awb_outdoor_special_type                awb_outdoor_fog_sky_adjust;
    awb_outdoor_special_type                awb_outdoor_green_detect_adjust;

    chromatix_awb_gain_adj_type             awb_outdoor_no_day_weighted_sample_adjust;
    chromatix_awb_gain_adj_type             awb_outdoor_all_out_of_zone_adjust;

    awb_extended_outdoor_exp_trigger_type   awb_extended_outdoor_exp_trigger;
} awb_extended_outdoor_heuristic_type;

typedef struct
{
    int                       awb_single_color_heuristic_enable;
    awb_single_color_type     awb_single_color_detect[NUM_SPECIAL_COLOR_DETECT];
} awb_single_color_tracking_type;

typedef struct
{
    awb_trigger_type_int      awb_day_sum_threshold_count;
    awb_trigger_type_int      awb_flo_sum_threshold_count;
    awb_misleading_zone_type  awb_misleading_color_zones[MISLEADING_COLOR_ZONE_NUM];
} awb_misleading_zone_process_type;

//=============================================================================
// Feature name: awb_green_struct
// Green tuning parameters structure type
//
// Variable name: green_proj
// Select green projection method
// Default: 0
// Range : 0 - projection on day lines, 1 - projection on one point
//
// Variable name: green_offset_bg.
// Green offset b/g for top part
// Default value: -100 (disable top part of the green zone before tuning).
// Data range: -100 to 100.
// Constraints:
// Effect:
//
// Variable name: green_rg_offset_top_H.
// Green offset r/g for top part of the green zone.
// Default value: -100 (disable top part of the green zone before tuning).
// Data range: -100 to 100.
// Constraints:
// Effect:
//
// Variable name: green_rg_offset_bottom_H.
// Green offset r/g for bottom part of the green zone.
// Default value: -100 (disable bottom part of the green zone before tuning).
// Data range: -100 to 100.
// Constraints:
// Effect:
//
// Variable name: slope_factor_m.
// The parameter to adjust the slope of the projection line from center of the green stats to the day light line.
// Default value: 1.5
// Data range:
// Constraints: None.
// Effect: The smaller the value, the smaller slope the projection line has. The resulted green scene images go towards the bluish green direction.
//         The larger the value, the larger slope the projection line has. The resulted green scene images go towards the yellowish green color direction.
//
//
// Variable name: slope_factor_m_shade.
// Slope factor used for green zone projection in shade condition.
// Default value: 1.5 (a must tune parameter for different sensors).
// Data range: 0 to 100.
// Constraints:
// Effect:
//
// Variable names: awb_cw_green_reject_threshold, awb_custom_fl_green_reject_threshold.
// The threshold used to screen out the possible green samples for indoor CW/custom flo conditions.
// Default value: 0 (disable)
// Data range: 0 to 255.
// Constraints:
// Effect: When a stat is close to CW/CF but its green value is less than this threshold,
//   the sample is counted as an outlier instead of a grey sample.
//
// Variable name: bright_green_percentage.
// Bright green percentage at outdoor index.
// Default value: 6.
// Data range: 0 to 100.
// Constraints: This is a pre-set threshold obtained by testing.
// Effect: Higher value extends the outdoor green range.
//
// Variable name: dark_green_percentage.
// Dark green percentage at indoor index.
// Default value: 12.
// Data range: 0 to 100.
// Constraints: This is a pre-set threshold obtained by testing.
// Effect: Indoor green range is extended or narrowed down.
//
// Variable name: green_enhance_low_pec
// low green percentage for running green enhancement
// TBD
//
// Variable name: green_enhance_high_pec
// high green percentage for running green enhancement
// TBD
//
// Variable name: green2f_outlier_dist2
// distance from CW reference point for rejecting green stats inside the grey zone
// Default: 0
// Range: [0, 16]
//
// Variable name: green_enhance_gain_adjust_high
// Adjust gain for green enhancement with high green percentage
//
// Variable name: green_zone_assist
// Additional green zones for handling other green colors (e.g., artificial green grass)
//
// Variable name: bright_green_exp_index
// green zone projection slope factor trigger point for normal scenes
// Range:[0, outdoor_index]
//
// Variable name: shade_green_exp_index
// green zone projection slope factor trigger point for shade scenes
// Range:[outdoor_index/2, inoutdoor_index]

// Variable name: exp_weight
// exposure based offset that triggers slope_factor_m_shade
// Default: 0
// Range: [0, 300]
//
// Variable name: green_proj_interp_enable
// Enable/disable green projection interpolation.
// If it is enabled, the green interpolation ratio will be computed and do interpolating weighted sample average
// and green target point with the computed interpolation ratio
// Default: 0 - disable
// Range: 1 - enable 0 - disable
//
// Variable name: green_proj_per_low;
// The offset percentage for determining the lowest green percentage threshold.
// No effect on interpolation ratio between weighed sample average and green projected target point
// Default: 2.0
// Range: [0,50]
// Constraints: green_proj_per_low <= green_proj_per_high
//
// Variable name: green_proj_per_high;
// The offset percentage for determining the highest green percentage threshold.
// The interpolation ratio will be computed
// the lowest green percentage threshold and the highest green percentage threshold
// Default: 20.0
// Range: [0,80]
// Constraints: green_proj_per_high >= green_proj_per_low
//              green_proj_per_high <= green_proj_per_max
//
// Variable name: green_proj_per_max;
// The offset percentage for determining the maximum green percentage threshold.
// The interpolation ratio will be computed  with the highest green percentage threshold,
// the maximum green percentage threshold, and the maximum interpolation ratio
// Default: 80.0
// Range: [50,100]
// Constraints: green_proj_per_max >= green_proj_per_high
//
// Variable name: green_proj_per_max_ratio;
// The maximum interpolation ratio.
// If the green percentage is larger than the maximum green percentage threshold,
// this ratio should be the interpolation ratio
// Default: 1.2
// Range: [1.0,2.0]
// Constraints:
//
// Variable name: green_proj_index_start;
// The minimum exposure index for determining the interpolation ratio.
// If the current exposure index is lower than this index, no changes on the interpolation ratio.
// Note that we change the interpolation ratio that has been already computed by green percentage thresholds
// Default: 210.0
// Range: [150,300]
// Constraints: green_proj_index_start >= outdoor_heuristic_exposure_index_trigger
//              green_proj_index_start <= green_proj_index_end
//
// Variable name: green_proj_index_end;
// The maximum exposure index for determining the interpolation ratio.
// If the current exposure index is between index_start and index_end, the interpolation ratio is modified.
// If the current exposure index is higher than index_end, the final interpolation ratio will be set to 0
// Default: 310.0
// Range: [200,350]
// Constraints: green_proj_index_end <= indoor_index
//=============================================================================
typedef struct
{
    int                         green_proj;
    int                         green_offset_bg;
    int                         green_rg_offset_top_H;
    int                         green_rg_offset_bottom_H;
    float                       slope_factor_m;
    float                       slope_factor_m_shade;
    int                         awb_cw_green_reject_threshold;
    int                         awb_custom_fl_green_reject_threshold;
    int                         bright_green_percentage;
    int                         dark_green_percentage;
    int                         green_enhance_low_pec;
    int                         green_enhance_high_pec;
    int                         green2f_outlier_dist2;

    chromatix_awb_gain_adj_type green_enhance_gain_adjust_high;
    awb_misleading_zone_type    green_zone_assist[GREEN_ZONE_ASSIST_NUM];

    trigger_point_type          bright_green_exp_index;
    trigger_point_type          shade_green_exp_index;

    int                         exp_weight;
    int                         green_proj_interp_enable;
    float                       green_proj_per_low;
    float                       green_proj_per_high;
    float                       green_proj_per_max;
    float                       green_proj_per_max_ratio;
    float                       green_proj_index_start;
    float                       green_proj_index_end;
} awb_green_struct_type;

//=============================================================================
// Variable name: warmup_ah_interp_eanble
// Enable/Disable interpolation in warm color appearance
// Default: 0
// Range: 1 - Enable (0 - Disable)
//=============================================================================
typedef struct
{
    int   warmup_enable_aa;
    int   warmup_enable_hh;
    float weight_reference_aa;
    float weight_reference_ah;
    float weight_reference_hh;
    int   warmup_ah_interp_enable;
} awb_weight_lowlight_warm_type;

typedef struct
{
    float a_h_line_boundary;
    float day_f_line_boundary;
    float d75_d65_line_boundary;
    float d65_d50_line_boundary;
    float d65_d50_shifted_line_boundary;
    float f_a_line_boundary;
    float cw_tl84_line_boundary;
    float d50_weighted_sample_boundary;
    float d65_weighted_sample_boundary;
} awb_decision_boundary_type;

#define HEURISTIC_WEIGHT_NUM 20

typedef struct
{
    float exp_weight_low[2];
    float exp_weight_high[2];
    float cct_weight_low[2];
    float cct_weight_high[2];
} awb_heuristic_weight_type;

typedef struct
{
    int                           awb_pre_flash_enable;
    int                           awb_flash_stat_sat_threshold;
    int                           awb_preflash_outlier_distance;
    int                           awb_preflash_outlier_distance_no_flash;

    chromatix_awb_reference_type  awb_mainflash_typ_ref;
    chromatix_awb_reference_type  awb_preFlash_min_ref;
    chromatix_awb_reference_type  awb_preFlash_max_ref;
    chromatix_awb_reference_type  awb_mainflash_min_ref;
    chromatix_awb_reference_type  awb_mainflash_max_ref;

    int                           awb_flash_valid_sample_high_threshold;
    int                           awb_flash_valid_sample_low_threshold;

    trigger_point_type            awb_flash_typ_decision_rg_offset;
    trigger_point_type            awb_flash_typ_decision_bg_offset;

    chromatix_awb_gain_adj_type   awb_flash_corner_sample_ref_adjust;
    awb_flash_lut_info            awb_flash_lut[AWB_FLASH_ADJUST_LUT];
} awb_preflash_type;

typedef struct
{
    int awb_front_camera;
    int awb_cap_gain_enable;
    int awb_fd_awb_enable;
} awb_front_camera_ctrl_type;

//=============================================================================
// Feature name: awb_dynamic_convergence_type
// Control dynamic skip frames
//
// Variable name: awb_dynamic_frame_skip_control_enable
// Enable/Disable dynamic frame skip control
// Default: 0 - disable
// Range: 0 - disable, 1 - enable
//
// Variable name: awb_settle_time_threshold_for_temp_stability
// Define the awb settle time threshold for enabling awb_temproal_stability
// If awb_temproal_stability is enabled and awb settle time is longer than the threshold,
// the current average r/g and b/g ratio will be replaced by awb history�s average r/g and b/g ratio
// Default: 1.0
// Range: [0.3, 2]
//
// Variable name: awb_settle_time_threshold_for_awb_frame_skip
// If dynamic frame skip control is enabled and awb settle time is longer than the threshold,
// the AWB will start to kip frame, otherwise, AWB will not skip any frame
// Default: 1.0
// Range: [0.3, 2]
//
// Variable name: convergence_factor
// convergence speed of red gain (blue gain) for gain adjust
// Default: (0.075, 0.075)
// Range: [0.02, 1.0]
//
// Variable name: convergence_factor_camera_before_awb_settle
// convergence speed of still mode for awb output gain before AWB settle,
// the convergence speed should be slower before AWB settle
// Default: 0.03
// Range: [0.005, 0.2]
//
// Variable name: convergence_factor_camera_after_awb_settle
// convergence speed of still mode for awb output gain before AWB settle,
// the convergence speed should be faster after AWB settle
// Default: 0.15
// Range : [0.005, 0.2]
//
// Variable name: convergence_factor_video_before_awb_settle
// convergence speed of video mode for awb output gain before AWB settle,
// the convergence speed should be slower before AWB settle
// Default: 0.01
// Range: [0.005, 0.2]
//
// Variable name: convergence_factor_video_after_awb_settle
// convergence speed of video mode for awb output gain before AWB settle,
// the convergence speed should be faster after AWB settle
// Default: 0.05
// Range: [0.005, 0.2]
//=============================================================================
typedef struct
{
    int                             awb_dynamic_frame_skip_control_enable;
    float                           awb_settle_time_threshold_for_temp_stability;
    float                           awb_settle_time_threshold_for_awb_frame_skip;
    chromatix_awb_gain_adj_type     convergence_factor;
    float                           convergence_factor_camera_before_awb_settle;
    float                           convergence_factor_camera_after_awb_settle;
    float                           convergence_factor_video_before_awb_settle;
    float                           convergence_factor_video_after_awb_settle;
} awb_dynamic_convergence_type;

//=============================================================================
// Feature name: awb_gyro_assistance_type
// Convergence with gyro information
//
// Variable name: awb_gyro_assisted_awb_enable
// Enable/Disable gyro assistance awb convergence
// Default: 0 - disable
// Range:  0 - disable, 1 - enable
//
// Variable name: awb_gyro_threshold
// Define gyro min/max threshold for triggering awb_convergence_speed
// fstart: min threshold, fend: max threshold
// Default: (0.01, 0.3)
// Range: [0.005, 1.0]
//
// Variable name: awb_gyro_convergence_speed
// Define min/max AWB convergence speed for gyro information
// fstart: minimum AWB convergence speed
// fend: maximum AWB convergence speed
// Default: (0.03, 0.3)
// Range : [0.005, 0.2]
//
// Variable name: awb_gyro_stable_th
// define the gyro stable threshold
// Default: 0.01
// Range: [0.005, 0.03]
//
// Variable name: awb_gyro_luma_delta_th[AWB_GYRO_LUMA_DELTA_NUM]
// Define the min/max/stable luma difference values
// 0: minimum, 1: maximum, 2:stable
// Default: [5, 10, 2]
// Range: [1, 15]
//
// Variable name: awb_gyro_convergence_speed_for_luma
// Define min/max convergence speed for luma information
// fstart: min, fend:max
// Default: (0.15, 0.3)
// Range: [0.2 ,0.005]
//
// Variable name: awb_gyro_holdind_time_enable
// Enable/disable holding time control
// Default: 0 - disable
// Range: 0 -disable, 1 - enable
//
// Variable name: awb_gyro_holding_time_param[AWB_GYRO_HT_PARAM]
// Define several thresholds corresponding to the holding time properties
// 0: holding time threshold
// 1: luma delta threshold
// 2: maximum holding time threshold
// 3: lower boundary of gyro average values
// 4: the percentage of maximum holding time
// 5: higher boundary of gyro average value
// 6: the percentage of maximum holding time
// Default: [ 3.0, 10, 1.0, 0.01, 1.0, 0.3, 0.5 ]
// Range: 0: [1.0, 5.0]
//        1: [5,2 0]
//        2: [0.5, 5.0]
//        3: [0.005, 0.05]
//        4: [0.1, 1.0]
//        5: [0.1, 1.0]
//        6: [0.1, 1.0]
//
//=============================================================================
#define AWB_GYRO_LUMA_DELTA_NUM     3
#define AWB_GYRO_HT_PARAM           7

typedef struct
{
    int                     awb_gyro_assisted_awb_enable;
    awb_trigger_type_float  awb_gyro_threshold;
    awb_trigger_type_float  awb_gyro_convergence_speed;
    float                   awb_gyro_stable_th;
    float                   awb_gyro_luma_delta_th[AWB_GYRO_LUMA_DELTA_NUM];
    awb_trigger_type_float  awb_gyro_convergence_speed_for_luma;
    int                     awb_gyro_holdind_time_enable;
    float                   awb_gyro_holding_time_param[AWB_GYRO_HT_PARAM];
} awb_gyro_assistance_type;

#define AWB_RESERVED_PARAM 50

//Reserve values all set to 0
typedef struct
{
    int   reserved_int[AWB_RESERVED_PARAM];
    float reserved_float[AWB_RESERVED_PARAM];
} awb_reserved_data_type;

//=============================================================================
// Feature name:awb_algo_structure
// All tuning parameter definitions for AWB algorithm
//=============================================================================
typedef struct
{
    awb_basic_tuning_type               awb_basic_tuning;                           // basic tuning parameters
    awb_weight_vector_type              awb_weight_vector;                          // awb weight vectors including illuminant and distance weight vectors
    awb_green_struct_type               awb_green_zone_param;                       // green zones and mapping parameters
    awb_dark_threshold_type             awb_dark_threshold;                         // Thresholds for rejecting dark stats
    awb_temporal_tuning_type            awb_temporal_tuning;                        // temporal heuristic parameters
    awb_heuristic_control_type          awb_heuristic_control;                      // heuristic control parameters
    awb_heuristic_weight_type           awb_heuristic_weight[HEURISTIC_WEIGHT_NUM]; // Decision weights for heuristic functions
    awb_white_world_struct_type         awb_white_world;                            // White world decision parameters
    awb_decision_boundary_type          awb_decision_boundary;                      // awb decision boundaries
    awb_cluster_tuning_type             awb_clusters_tuning;                        // cluster control parameters
    awb_weight_lowlight_warm_type       awb_weight_lowlight_warm;                   // A and Horizon lights warming parameters
    awb_CCM_type                        awb_CCM_control;                            // AWB driven CCM parameter
    awb_preflash_type                   awb_preFlash_param;                         // AWB preflash tuning parameters
    awb_led_tuning_type                 awb_led_tuning;                             // LED tuning (excluding preFlash)
    awb_extended_outdoor_heuristic_type awb_extended_outdoor_heuristic;             // awb outdoor heuristics with extended tuning parameters
    awb_single_color_tracking_type      awb_single_color_tracking;                  // single color tracking parameters
    awb_misleading_zone_process_type    awb_misleading_color_zone;                  // misleading color zone
    awb_golden_module_info_type         awb_golden_module_info;                     // golden module information
    awb_flow_control_type               awb_flow_control;                           // awb flow control parameters
    awb_blue_tuning_type                awb_blue_tuning;                            // blue color tuning (sky, snow blue, and beach blue)
    awb_skin_control_type               awb_skin_control;                           // skin color control
    awb_aux_sensor_type                 awb_aux_sensor;                             // auxiliary sensor control
    awb_front_camera_ctrl_type          awb_front_camera_control;                   // Front camera control parameters
    awb_MWB_type                        awb_MWB;                                    // Manual White Balance parameters
    awb_dynamic_convergence_type        awb_dynamic_convergence;                    // Dynamic convergence with frame skip
    awb_gyro_assistance_type            awb_gyro_assisted_convergence;              // gyro assisted convergence
    awb_reserved_data_type              awb_reserved_data;                          // reserved data
} awb_algo_struct_type;

//=============================================================================
//  AEC tuning structures
//=============================================================================

#define NUM_AEC_STATS              16
#define MAX_EXPOSURE_TABLE_SIZE    700
#define MAX_SNAPSHOT_LUT_SIZE      10
#define MAX_AEC_TRIGGER_ZONE       6
#define MAX_EV_ARRAY_SIZE          49

typedef struct
{
    unsigned short gain;                        // Gain
    unsigned int   line_count;                  // LineCt
} exposure_entry_type;

typedef struct
{
    unsigned short          valid_entries;                              // Number of entries in the exposure table with digital EV feature disabled
    int                     aec_enable_digital_gain_for_EV_lowlight;    // Flag to enable\disable the digital EV feature
    unsigned short          total_entries_with_digital_gain_for_EV;     // Number of entries in the exposure table with digital EV feature enabled
    unsigned short          fix_fps_aec_table_index;                    // Index upto which the frame rate is fixed (linear AFR kicks in beyond this point)
    exposure_entry_type     exposure_entries[MAX_EXPOSURE_TABLE_SIZE];  // Exposure table with gain and line count entries
} aec_exposure_table_type;

/**
 * Lux trigger definition
 */
typedef struct
{
    int start;
    int end;
} aec_lux_trigger_type;

typedef enum
{
    BAYER_CHNL_R,
    BAYER_CHNL_G,
    BAYER_CHNL_B,
    BAYER_CHNL_MAX
} aec_bayer_channel_type;

/**
 * Luma target tuning
 */
typedef struct
{
    unsigned int           luma_target;           // Luma target
} aec_luma_target_triggered_type;

typedef struct
{
    int                             num_zones;                                  // Number of zones actually used
    aec_lux_trigger_type            triggers[MAX_AEC_TRIGGER_ZONE-1];           // Trigger point to enter the next zone
    aec_luma_target_triggered_type  triggered_params[MAX_AEC_TRIGGER_ZONE];     // Parameters that can be tuned per-zone
} aec_luma_target_type;

/**
 * This structure defines the metering tables
 */
typedef struct
{
    float    AEC_weight_center_weighted[NUM_AEC_STATS][NUM_AEC_STATS];          // Center-weighted metering table (0x304)
    float    AEC_weight_spot_metering[NUM_AEC_STATS][NUM_AEC_STATS];            // Spot-metering table (0x304)
} aec_metering_table_type;

//=============================================================================
// Feature name: AEC index compensation
// Applicable operation mode:  Snapshot, Preview and Camcorder
// When this feature is enabled, AEC output normalized AWB exposure index and
// lux index, regardless of the sensor resolution mode.
//
// Variable name: awb_exp_idx_comp_enable
// Flag to enable the AWB exposure index compensation feature
// Default value: 0
// Range: 0 and 1
//
// Variable name: lux_idx_comp_enable
// Flag to enable the lux index compensation feature
// Default value: 0
// Range: 0 and 1
//
// Variable name: ref_sensitivity
// Reference sensitivity. A must tune value if any of the above enable flags are set.
// The value is the sensitivity of the first entry of the preview mode exposure table.
// The sensitivity is calculated as exposure time (in second) multipled by gain, and
// further multipled by binning factor (most of the time, binning factor is 1).
// Default value: 0.00001
// Range : [0 1]
//=============================================================================
typedef struct
{
    int      awb_exp_idx_comp_enable;           // Flag to enable\disable the AWB exposure index compensation feature
    int      lux_idx_comp_enable;               // Flag to enable\disable the lux index compensation feature
    float    ref_sensitivity;                   // reference sensitivity value
} aec_idx_comp_type;

//=============================================================================
// Feature name: AEC EV compensation
// Applicable operation mode:  Snapshot, Preview and Camcorder
// This structure enables tuning of the EV tables for both regular EV compensation
// and bracketing use cases.
//
// Variable name: steps_per_ev
// This variable defines the number of steps for each EV in the EV tables
// Due to s/w limitation, this variable needs to be changed simultaneously with
// the hard-coded values inside stats module code (aec_module.c) on LA.
// Default value: 6
// Range: 1 to 12 (integer)
//
// Variable name: min_ev_val
// The minimal EV value in the ev table. It is recommended to NOT change normally.
// Also this variable needs to be changed simultaneously with the hard-coded
// values inside stats module code (aec_module.c) on LA.
// Default value: -2
// Range: -2 to 0
//
// Variable name: max_ev_val
// The maximal EV value in the ev table. It is recommended to NOT change normally.
// Also this variable needs to be changed simultaneously with the hard-coded
// values inside stats module code (aec_module.c) on LA.
// Default value: 2
// Range: 0 to 2
//
// Variable name: ev_comp_table
// EV table that can be tuned for EV compensation in AEC auto mode. The EV table
// normally starts from -2, and ends at +2. With steps_per_ev set at 6, the table
// will need 25 entries. The maximum steps_per_ev is 12, which requires 49 entries
// in the EV table.
// Default value: {0.2500f, 0.2806f, 0.3150f, 0.3536f, 0.3969f, 0.4454f, 0.5000f,
//   0.5612f, 0.6299f, 0.7071f, 0.7937f, 0.8909f, 1.0000f, 1.1225f, 1.2599f, 1.4142f,
//   1.5874f, 1.7818f, 2.0000f, 2.2449f, 2.5198f, 2.8284f, 3.1748f, 3.5636f, 4.0000f}
// Range : [0.0f, 10.0f] for each entry
//
// Variable name: bracketing_table
// It is also an EV table, except that it is used in bracketing exposure calculation
// which is the multi-frame HDR use case. It can be tuned independently of regular
// AEC EV compensation.
// Default value: {0.2500f, 0.2806f, 0.3150f, 0.3536f, 0.3969f, 0.4454f, 0.5000f,
//   0.5612f, 0.6299f, 0.7071f, 0.7937f, 0.8909f, 1.0000f, 1.1225f, 1.2599f, 1.4142f,
//   1.5874f, 1.7818f, 2.0000f, 2.2449f, 2.5198f, 2.8284f, 3.1748f, 3.5636f, 4.0000f}
// Range : [0.0f, 10.0f] for each entry
//=============================================================================
typedef struct
{
    unsigned short steps_per_ev;
    short          min_ev_val;
    short          max_ev_val;
    float          ev_comp_table[MAX_EV_ARRAY_SIZE];
    float          bracketing_table[MAX_EV_ARRAY_SIZE];
} aec_ev_table_type;

//=============================================================================
// Feature name: AEC Luma Weight
// Applicable operation mode:  Snapshot, Preview and Camcorder
// Luma weight for calculating the luma is exposed. Also a compensation feature
// is introduced to allow AEC to compensate the r/g/b weights for luma
// calculation based on the current white balance decision. In addition, since
// WB gains are OTP corrected, the additional benefit of this feature is to also
// compensate luma variation caused by module-to-module variation as well.
//
// Variable name: r_weight
// The default weight used to multiply red channel to calculate luma. If color and
// sensor compensation is not enabled, this is the final red weight used by AEC.
// Default value: 0.2988
// Range: [0, 1]
//
// Variable name: g_weight
// The default weight used to multiply green channel to calculate luma. If color and
// sensor compensation is not enabled, this is the final green weight used by AEC.
// Default value: 0.5869
// Range: [0, 1]
//
// Variable name: b_weight
// The default weight used to multiply blue channel to calculate luma. If color and
// sensor compensation is not enabled, this is the final blue weight used by AEC.
// Default value: 0.1137
// Range: [0, 1]
//
// Variable name: color_and_sensor_comp_enable
// Enabling compensation for luma calculation under different lighting as well as
// different sensor modules. If this flag is set, "r_weight", "g_weight" and "b_weight"
// are modified dynamically based on the difference between current white balance gains
// and reference white balance gains as defined below.
// Default value: 0
// Range: 0 and 1
//
// Variable name: ref_rGain
// The reference white balance gain used to calculate the adjustment factor on
// r_weight. It is recommended to set this value to D50 red gain of the golden module.
// If feature is enabled, this value is must tune.
// Default value: 1.8
// Range: [1, 3]
//
// Variable name: ref_gGain
// The reference white balance gain used to calculate the adjustment factor on
// g_weight. It is recommended to set this value to D50 green gain of the golden module.
// If feature is enabled, this value is must tune.
// Default value: 1.0
// Range: [1, 3]
//
// Variable name: ref_bGain
// The reference white balance gain used to calculate the adjustment factor on
// b_weight. It is recommended to set this value to D50 blue gain of the golden module.
// If feature is enabled, this value is must tune.
// Default value: 1.5
// Range: [1, 3]
//=============================================================================
typedef struct
{
    float   r_weight;                          // Red weight for luma calculation
    float   g_weight;                          // Green weight for luma calculation
    float   b_weight;                          // Blue weight for luma calculation
    int     color_and_sensor_comp_enable;      // Enable color and sensor module compensation for luma calculation
    float   ref_rGain;                         // Reference red gain for golden module (recommend D50)
    float   ref_gGain;                         // Reference green gain for golden module (recommend D50)
    float   ref_bGain;                         // Reference blue gain for golden module (recommend D50)
} aec_luma_weight_type;

//=============================================================================
// Variable name: pct_based_luma_tolerance_enable
// Set this flag to enable AEC algorithm to use percentage based luma tolerance.
// It provides more consistency in AEC for lowlight and EV-minus use cases. This
// is required if ADRC is enabled.
// Default value: 0
// Range: 0 and 1
//
// Variable name: luma_tolerance_pct
// Luma tolerance to be used in all preview/camcorder AEC convergence.
// The smaller the luma tolerance, the more consistent AEC will be. However,
// chances of oscillation could also be higher.
// Suggest NOT tune unless absolutely necessary.
// Default value: 0.03
// Data range: (0, 1]
//
// Variable name: iso_quantization_enable
// Set this flag to enable standardized ISO value reporting from AEC.
// Gains sent to sensor are not changed.
// Default value: 1
// Range: 0 and 1
//
// Variable name: lux_index_ref
// Reference luma target for calculating lux index as well as awb exposure index.
// No need to tune.
// Default value: 50
// Range : [0 255]
//
// Variable name: aec_subsampling_factor
// Subsampling factor for AEC to skip bayer stats for the purpose of power saving
// Default value: 4
// Range: 1 to 8
//=============================================================================
typedef struct
{
    unsigned int          aec_start_index;                     // Exposure index to be used upon camera startup
    unsigned short        luma_tolerance;                      // Tolerance range to deem AEC as settled
    int                   pct_based_luma_tolerance_enable;     // Enable to switch algorithm to use percentage based luma tolerance
    float                 luma_tolerance_pct;                  // Tolerance to deem AEC as settled, in percentage
    int                   frame_skip_startup;                  // Frame skip value to be used upon camera startup
    float                 aggressiveness_startup;              // Aggressiveness value to use upon camera startup
    float                 exposure_index_adj_step;             // Exposure step size (0x304)
    float                 ISO100_gain;                         // ISO 100 gain value (0x304)
    int                   antibanding_vs_ISO_priority;         // In manual ISO, whether anti-banding or ISO takes priority
    int                   iso_quantization_enable;             // Set this flag to enable standardized ISO
    float                 max_snapshot_exposure_time_allowed;  // Maximum snapshot exposure time (in seconds) (0x304)
    unsigned int          lux_index_ref;                       // A reference value to be used for lux index computation
    unsigned short        aec_subsampling_factor;              // AEC stats subsampling for power optimization
    aec_idx_comp_type     idx_comp;                            // AEC index compensation
    aec_ev_table_type     aec_ev_table;                        // AEC EV compensation table
    aec_luma_weight_type  aec_luma_weight;                     // Color and sensor compensation for luma calculation
    float                 reserved[50];                        // Reserved for temporary use
} aec_generic_tuning_type;

/**
 * This structure defines the motion ISO tuning parameters
 */
typedef struct
{
    int       motion_iso_enable;           // Flag to enable\disable the motion ISO feature
    float     motion_iso_aggressiveness;   // Motion ISO aggressiveness
    float     motion_iso_threshold;        // Threshold value for the motion ISO compensation to kick in
    float     motion_iso_max_gain;         // Maximum gain that can be used for the motion ISO feature
} aec_motion_iso_type;

/**
 * This structure defines the gain tradeoff and maximum exposure time entries for the snapshot look-up table (LUT)
 */
typedef struct
{
    unsigned short   lux_index;        // Lux index corresponding to each entry
    float            gain_trade_off;   // Gain tradeoff value
    float            max_exp_time;     // Maximum exposure time value (in seconds)
} snapshot_trade_off_table_type;

/**
 * This structure defines the the snapshot look-up table (LUT)
 */
typedef struct
{
    int                             enable;                                     // Flag to enable\disable the snapshot LUT feature
    int                             exposure_stretch_enable;                    // Flag to enable\disable the exposure stretch feature
    unsigned char                   valid_entries;                              // Number of valid entries in the snapshot LUT
    snapshot_trade_off_table_type   snapshot_ae_table[MAX_SNAPSHOT_LUT_SIZE];   // Snapshot look-up table (LUT)
} aec_snapshot_exposure_type;

//=============================================================================
// Variable name: wled_trigger
// The trigger defines when to trigger LED in auto mode. It replaces "wled_trigger_idx" in
// previous header version. The trigger has a pair of lux indices, which add hysteresis
// to avoid inconsistent triggering around lux threshold.
// Value should be tuned based on lux level. It is advised to trigger under 100lux.
// Default value: {410, 420}
// Range: 0 to 1000
//
// Variable name: force_led_af_in_led_on_mode
// Set this flag to 1 to always run LED AF in LED force ON mode.
// Default value: 0
// Range: 0 and 1
//
// Variable name: flash_target_enable
// Enabling use of dedicated flash luma target for the flash images
// Default value: 0
// Range: 0 and 1
//
// Variable name: flash_target
// This includes a full set of luma targets for flash use case. Basically, if flash target
//   is enabled, the flash luma target will be interpolated first based on estimated lux index
//   for main flash. Then it will be interpolated with regular luma target based on flash
//   influence (k).
// Only when "k" is big enough will AEC use the flash target. If "k" is small, AEC uses
//   regular luma target which is also based on estimated lux index for main flash. If it
//   is in between, AEC will use interpolation to determine the target.
// Default value: Same as aec_luma_target for regular case.
//=============================================================================
typedef struct
{
    // For general flash and LED tuning
    aec_lux_trigger_type  wled_trigger;                  // Lux index above which flash is fired in the AUTO mode
    int                   force_led_af_in_led_on_mode;   // Set the flag to force AF always run when LED is set to ON
    float                 aec_led_pre_flux;              // Pre-flash/torch flux value
    float                 aec_led_flux_hi;               // High power LED flux value
    float                 aec_led_flux_med;              // Medium power LED flux value
    float                 aec_led_flux_low;              // Low power LED flux value

    int                   smart_flash_est_enable;        // Enable smart LED estimation for AEC
    float                 smart_flash_est_strength;      // Amount of flexibility/strength allowed for smart LED (0-1)
    int                   smart_flash_est_awb_enable;    // Enable smart LED estimation also for manual AWB
    float                 smart_flash_est_awb_strength;  // Amount of flexibility/strength allowed for AWB smart LED (0-1)

    int                   flash_target_enable;           // Enable a dedicated set of flash luma target
    aec_luma_target_type  flash_target;                  // Luma target for flash (similar as regular luma target)
    float                 target_interp_k_start;         // Below this k value, current luma target is used for flash
    float                 target_interp_k_end;           // Above this k value, flash target will be used

    int                   smart_flash_target_enable;     // Enable smart decision of luma target (Target may increase)
    int                   max_target_offset;             // Maximum allowed for the luma target to increase

    float                 reserved[15];                  // Reserved for internal algorithm
} aec_flash_tuning_type;

/**
 * This structure defines the touch AEC tuning parameters (0x304)
 */
typedef struct
{
    int     touch_roi_enable;       // Flag to enable\disable the touch AEC feature
    float   touch_roi_weight;       // Determines the influence of the touched region frame luma value on the overall frame luma calculation
    float   reserved[5];            // Reserved for algo internally
} aec_touch_type;

/**
 * This structure defines the face detection AEC tuning parameters (0x304: AEC_face_priority_type)
 */
typedef struct
{
    int     aec_face_enable;         // Flag to enable\disable the face detection AEC feature
    float   aec_face_weight;         // Determines the influence of the face frame luma value on the overall frame luma calculation
    float   reserved[15];            // Reserved for algo internally
} aec_face_priority_type;

typedef struct
{
    int     frame_skip;           // Number of frames skipped between each exposure update
    float   aggressiveness;       // Aggressivenes value which moving from a dark scene to a bright scene
    float   reserved[10];         // Reserved for algo internally
} aec_fast_conv_type;

//=============================================================================
//
// Variable name: ht_tolerance_adjust_factor
// In order for holding time feature to activate, AEC needs to be stable for a
//   period of time. A different threshold/tolerance can be used to define whether
//   AEC is stable for holding time feature. This factor is used to adjust/relax
//   the luma tolerance used in normal mode for the purpose of holding time trigger.
// The higher this value, the easier holding time can be triggered.
// Default value: 2.0
// Range: [1.0, 4.0]
//
//=============================================================================
typedef struct
{
    int     slow_convergence_enable;   // Flag to enable slow convergence for video
    int     frame_skip;                // Number of frames skipped between each exposure update
    float   conv_speed;                // Convergence speed (0-1)
    int     ht_enable;                 // Flag to enable\disable the holding time logic
    float   ht_tolerance_adjust_factor;// This factor is used to relax luma tolerance for triggering holding time
    float   ht_thres;                  // Holding time activation threshold for luma settle (in seconds)
    float   ht_max;                    // Maximum amount of holding time allowed (in seconds)
    int     ht_gyro_enable;            // Enable to use gyro for shortening holding time
    float   reserved[25];              // Reserved for algo internally
} aec_slow_conv_type;

/**
 * Bright region tuning parameters
 */
typedef struct
{
    unsigned int          bright_region_thres;  // Threshold for a region to be considered a bright region (Q8)
    float                 bright_weight;        // Weight to increase or decrease the influence of bright regions
} aec_bright_region_triggered_type;

typedef struct
{
    int                                bright_region_enable;                    // Flag to enable\disable the bright region feature
    int                                num_zones;                               // Number of zones actually used
    aec_lux_trigger_type               triggers[MAX_AEC_TRIGGER_ZONE-1];        // Trigger point to enter the next zone
    aec_bright_region_triggered_type   triggered_params[MAX_AEC_TRIGGER_ZONE];  // Parameters that can be tuned per-zone
    float                              min_tweight;                             // Minimal bias table weight for a stats to be included
    float                              max_bright_pct;                          // Maximum number of bright regions to be counted
} aec_bright_region_type;

/**
 * Dark region tuning parameters
 */
typedef struct
{
    unsigned int          dark_region_low_thres;  // Below this threshold means it is dark (Q8)
    unsigned int          dark_region_high_thres; // Above this threshold means it is not dark (Q8)
    float                 dark_weight;            // Weight to increase or decrease the influence of dark region
} aec_dark_region_triggered_type;

typedef struct
{
    int                              dark_region_enable;                        // Flag to enable\disable the dark region feature
    int                              num_zones;                                 // Number of zones actually used
    aec_lux_trigger_type             triggers[MAX_AEC_TRIGGER_ZONE-1];          // Trigger point to enter the next zone
    aec_dark_region_triggered_type   triggered_params[MAX_AEC_TRIGGER_ZONE];    // Parameters that can be tuned per-zone
    float                            min_tweight;                               // Minimal bias table weight for a stats to be include
    float                            max_dark_pct;                              // Maximum number of dark regions to be counted
} aec_dark_region_type;

/**
 * This structure defines the AEC extreme color tuning parameters
 */
typedef struct
{
    float                             adj_ratio[BAYER_CHNL_MAX];              // adjust ratios for all channels
} aec_extreme_color_triggered_type;

typedef struct
{
    int                               extreme_color_enable;                   // Flag to enable\disable the extreme colour feature
    int                               num_zones;                              // Number of zones actually used
    aec_lux_trigger_type              triggers[MAX_AEC_TRIGGER_ZONE-1];       // Trigger point to enter the next zone
    aec_extreme_color_triggered_type  triggered_params[MAX_AEC_TRIGGER_ZONE]; // Parameters that can be tuned per-zone

    float                             red_th;                                 // Extreme red threshold of r/g ratio
    float                             red_interp_pct;                         // Extreme red thresh interpolation range
    float                             green_rg_th;                            // Extreme green threshold of r/g ratio
    float                             green_rg_interp_pct;                    // Extreme green r/g thresh interpolation range
    float                             green_bg_th;                            // Extreme green threshold of b/g ratio
    float                             green_bg_interp_pct;                    // Extreme green b/g thresh interpolation range
    float                             green_cntr_th;                          // Extreme green thresh for the distance to (1,1) on (r/g)&(b/g) domain
    float                             green_cntr_interp_pct;                  // Extreme green center thresh interpolation range
    float                             blue_th;                                // Threshold for extreme blue detection
    float                             blue_interp_pct;                        // Extreme blue thresh interpolation range

    float                             luma_high_th;                           // High threshold for region luma
    float                             luma_low_th;                            // Low threshold for region luma
    float                             color_stats_pct_high_th;                // High threshold for percentage of extreme color stats
    float                             color_stats_pct_low_th;                 // Low threshold for percentage of extreme color stats
    float                             reserved[10];                           // Reserved for internal algorithm use
} aec_extreme_color_type;

typedef struct
{
    /* Flat scene detection tuning */
    int     hist_flat_detector_enable;          // Flag to enable\disable the histogram based flat scene detection feature
    int     start_level;                        // Starting point for the histogram peak detection
    int     end_level;                          // Ending point for the histogram peak detection
    int     range;                              // Range for the histogram peak detection
    float   delta_th_low;                       // Low threshold value for peak detection
    float   delta_th_high;                      // High threshold value for peak detection
    float   bright_flat_det_th;                 // Threshold value for bright flat scene detection
    float   dark_flat_det_th;                   // Threshold value for dark flat scene detection
    float   bright_flat_tail_det;               // Threshold value for tail detection for bright flat scenes
    float   dark_flat_tail_det;                 // Threshold value for tail detection for dark flat scenes
    /* Flat scene compensation tuning */
    int      bright_flat_compensation_enable;   // Flag to enable\disable the bright flat scene compensation
    int      dark_flat_compensation_enable;     // Flag to enable\disable the dark flat scene compensation
    float    flat_white_grey_vs_nongrey_th;     // Threshold to determine if flat area is brighter than colored regions
    float    flat_dark_grey_vs_nongrey_th;      // Threshold to determine if flat area is darker than colored regions
    float    near_grey_tolerance;               // Regions with R/G and B/G within the tolerance percentage are considered grey regions
    float    bright_flat_compensation_ratio;    // Ratio for the bright flat scene compensation
    float    dark_flat_compensation_ratio;      // Ratio for the dark flat scene compensation
    int      flat_indoor_disable_start_idx;     // Above this index, flat compensation start to get disabled
    int      flat_indoor_disable_end_idx;       // Above this index, flat compensation fully disabled
} aec_flat_scene_type;

//=============================================================================
// Variable name: start
// Start percentage for the range of pixels used to calculate
// either saturation position, or darkness position
// Default Value: 0.01
// Data range: [0 1]
//
// Variable name: end
// End percentage for the range of pixels used to calculate
// either saturation position, or darkness position
// Default Value: 0.04
// Data range: [0 1]
//=============================================================================
typedef struct
{
    float start;
    float end;
} aec_hist_pct_range_type;

//=============================================================================
// Variable name: hist_sat_pushup_range
// The saturated percentage range for push-up compensation (flat/low dynamic range scenes).
// It is recommended to use a bigger range for push-up for stability reasons.
// Default Value: {{0.02, 0.08}, {0.02, 0.08}, {0.02, 0.08}}
//
// Variable name: hist_sat_pushdown_range
// The saturated percentage range for push-down compensation (high dynamic range/overexposed scenes).
// It is recommended to use a smaller range for push-down, so that it can more effectively
// prevent smaller area of saturation.
// Default Value: {{0.01, 0.04}, {0.01, 0.04}, {0.01, 0.04}}
//
// Variable name: hist_dark range
// The dark area percentage range used to calculate darkness position and affect luma target.
// It is generally not recommended to use this particular part due to stability concerns.
// Default Value: {{0, 0.15}, {0, 0.15}, {0, 0.15}}
//
// Variable name: max_drc_gain
// Maximal DRC gain allowed for the particular lux zone. This variable is only valid when
// adrc mode is not set to disable.
// Default Value: {4.0, 4.0, 4.0}
// Range: [1.0, 8.0]
//
// Variable name: adrc_conv_speed
// Defines the convergence speed for ADRC tradeoff. The bigger, ADRC converges faster.
// If it is small, then the highlight may take more time to show up, while if it is too big,
// then there could be stability issues.
// No need to tune normally.
// Default Value: {0.15, 0.15, 0.15}
// Range: (0, 0.5]
//
// Variable name: gtm_percentage
// Percentage of current DRC gain to be applied to GTM. This is only applicable if chipset has
// GTM module support.
// Default Value: {0.5, 0.5, 0.5}
// Range: [0, 1]
//
// Variable name: ltm_percentage
// Percentage of current DRC gain to be applied to LTM. This is only applicable if chipset has
// LTM module support.
// Default Value: {0.5, 0.5, 0.5}
// Range: [0, 1]
//
// Variable name: la_percentage
// Percentage of current DRC gain to be applied to GTM. This is only applicable if chipset has
// LA module support.
// Default Value: {0,0,0}
// Range: [0, 1]
//=============================================================================
/**
 * This structure defines the histogram based AEC metering parameters
 * default & range values described in aec_hist_metering_type.
 */
typedef struct
{
    float                    max_target_adjust_ratio;                 // Maximum target adjust allowed
    float                    min_target_adjust_ratio;                 // Minimum target adjust allowed
    unsigned short           sat_low_ref;                             // Low reference/control point for saturation
    unsigned short           sat_high_ref;                            // High reference/control point for saturation
    unsigned short           dark_low_ref;                            // Low reference/control point for darkness
    unsigned short           dark_high_ref;                           // High reference/control point for darkness
    aec_hist_pct_range_type  hist_sat_pushup_range[BAYER_CHNL_MAX];   // Percentage of bright pixels used in determine saturation
    aec_hist_pct_range_type  hist_sat_pushdown_range[BAYER_CHNL_MAX]; // Percentage of bright pixels used in determine saturation
    aec_hist_pct_range_type  hist_dark_range[BAYER_CHNL_MAX];         // Percentage of dark pixels used to determine darkness
    float                    max_drc_gain;                            // Maximal DRC gain allowed
    float                    adrc_conv_speed;                         // ADRC tradeoff convergence speed
    float                    gtm_percentage;                          // Percentage of total current DRC gain to be allocated to GTM
    float                    ltm_percentage;                          // Percentage of total current DRC gain to be allocated to LTM
    float                    la_percentage;                           // Percentage of total current DRC gain to be allocated to LA
} aec_hist_metering_triggered_type;

//=============================================================================
// Variable name: adrc_enable
// ADRC is combined with histogram AEC for tuning interface structuring. Set this flag to
// either disable or enable ADRC. This replaced the "mode" parameter in previous header
// revision. GTM/LTM/LA are independently controlled via percentage variables, and hence
// there is no need to distinguish local or global modes.
// Default Value: 0
// Range: 0 and 1
//
// Variable name: dark_end_control_enable
// Dark end part of the histogram AEC is sometimes unstable (hist_dark_range etc.), and hence
// it can be disabled with this flag. It'll save computation if it is not used and disabled.
// Default Value: 0
// Range: 0 and 1
//
// reserved[25]: increased reserve size, set to all 0s for now
//=============================================================================
typedef struct
{
    int                                hist_metering_enable;                       // Flag to enable histogram based AEC metering
    int                                adrc_enable;                                // Enable or disable ADRC
    int                                dark_end_control_enable;                    // This parameter will be used to disable dark end control
    int                                num_zones;                                  // Number of zones actually used
    aec_lux_trigger_type               triggers[MAX_AEC_TRIGGER_ZONE-1];           // Trigger point to enter the next zone
    aec_hist_metering_triggered_type   triggered_params[MAX_AEC_TRIGGER_ZONE];     // Parameters that can be tuned per-zone
    float                              target_filter_factor;                       // Temporal filter for stability
    float                              reserved[25];                               // Reserved for internal algorithm use
} aec_hist_metering_type;

//=============================================================================
// Feature name: Histogram Stretch feature.
// Applicable operation mode:  Snapshot, Preview and Camcorder.
// When this feature is enabled, 3A will send dark clamp to be subtracted to
// VFE ACE (Color Conversion) module per frame, ACE driver will compute the new
// RGB2Y coefficients based on strech gain computed.
//
// Variable name: dark_percentage
// Histogram counts will be accumulated from Pixel Value 0 and increasing,when
// the occurrence percentage is greater than or equal to dark_percentage, the
// corresponding pixel K0 is used as offset. K0 is subtracted in VFE Color conversion
// ( RGB to Y ) process after clamping with max_dark_clamp
// Default value: {0.005, 0.005, 0.005}
// Range: [0.001, 0.05]
//
// Variable name: max_dark_clamp
// RGB to Y offset programmed with minimum of (max_dark_clamp,K0)
// Default value: {3, 3, 3}
// Range: 0 to 6
//
//=============================================================================
typedef struct
{
    float       dark_percentage;
    int         max_dark_clamp;
} aec_Y_hist_stretch_triggered_type;

//=============================================================================
// Variable name: enable
// Enable histogram stretch feature to be applied in ACE.
// Default value: 0
// Range: 0 and 1
//
// Variable name: num_zones
// Number of lux trigger zones used for bright region tuning.
// Default value: 3
// Range: 1 to 6
//
// Variable name: triggers
// Defines each lux trigger to be used for separating the zones.
//   Number of triggers should be 1 less than num_zones, and it is arranged in
//   sequence to align with the zones. For example, if we have 3 zones (zone_1,
//   zone_2 & zone_3), then triggers should contain in sequence, trigger for
//   zone_1 to zone_2, and trigger for zone_2 to zone_3.
// Default value: Same as luma target triggers
//=============================================================================
typedef struct
{
    int                                 enable;                                 // Enable histogram stretch feature in ACE
    int                                 num_zones;                              // Number of zones actually used
    aec_lux_trigger_type                triggers[MAX_AEC_TRIGGER_ZONE-1];       // Trigger point to enter the next zone
    aec_Y_hist_stretch_triggered_type   triggered_params[MAX_AEC_TRIGGER_ZONE]; // Parameters that can be tuned per-zone
} aec_Y_hist_stretch_type;

typedef struct
{
    aec_lux_trigger_type  hdr_indoor_trigger;            // For toggling exposure ratio
    float                 reserved[50];                  // Reserved for algo expansion
} aec_hdr_tuning_type;

//=============================================================================
// Feature name: Fast AEC convergence logic
// Description: This structure includes the tuning parameters for the fast AEC convergence logic
// Applicable operation modes: Camera start, camera switch (RFC <-> FFC), switch
// between preview and camcorder mode and vice versa
//
// Variable name: start_index
// The initial exposure index when sensor starts streaming in fast AEC mode
// Default value: 240.
// Data range: 0 to 500.
//
// Variable name: frame_skip
// This parameter specifies the number of frames to skip between each exposure update
// in the fast AEC mode. A higher value indicates that more frames are skipped between
// updates and a higher value is used when it takes a long time for the previous exposure
// update to be reflected in the stats
// Default: 3
// Range : 0 to 10
//
// Variable name: luma_settled_count
// This parameter specifies the number of successive frames where AEC is settled before the 'settled'
// flag  can be set to TRUE in the fast AEC mode. A higher value ensures accuracy at the expense
// of latency. A smaller value could result in some discrepancy observed in back-to-back runs
// Default value: 2
// Range: 0 to 3
//=============================================================================
typedef struct
{
    unsigned int    start_index;           // Exposure index to be used upon start of fast AEC
    int             frame_skip;            // Number of frames skipped between each exposure update during fast AEC
    unsigned short  luma_settled_count;    // Number of successive frames for AEC to be flag as 'settled'
} aec_fast_aec_type;

//
// This structure defines the tuning parameters input to the AEC algorithm.
//
typedef struct
{
    aec_generic_tuning_type     aec_generic;                                     // Generic AEC tuning parameters
    aec_exposure_table_type     aec_exposure_table;                              // Exposure table
    aec_snapshot_exposure_type  aec_snapshot_lut;                                // Snapshot look-up table (LUT) tuning parameters
    aec_luma_target_type        aec_luma_target;                                 // Defines the various luma targets
    aec_metering_table_type     aec_metering_tables;                             // Metering tables (center-weighted\spot-metering)
    aec_fast_conv_type          aec_fast_convergence;                            // Fast convergence tuning parameters
    aec_slow_conv_type          aec_slow_convergence;                            // Slow convergence tuning parameters
    aec_flash_tuning_type       aec_led_flash;                                   // AEC LED flash tuning parameters
    aec_touch_type              aec_touch;                                       // Touch AEC tuning parameters
    aec_face_priority_type      aec_face_detection;                              // Face detection AEC tuning parameters
    aec_bright_region_type      aec_bright_region;                               // Bright region tuning parameters
    aec_dark_region_type        aec_dark_region;                                 // Dark region tuning parameters
    aec_extreme_color_type      aec_extreme_color;                               // Extreme colour tuning parameters
    aec_flat_scene_type         aec_flat_scene;                                  // Flat scene tuning parameters
    aec_hist_metering_type      aec_hist_metering;                               // Histogram based metering tuning parameters
    aec_Y_hist_stretch_type     aec_Y_hist_stretch;                              // Y histogram stretch feature in ACE
    aec_hdr_tuning_type         aec_hdr;                                         // HDR AEC tuning parameters

    aec_motion_iso_type         aec_preview_motion_iso;                          // Preview motion ISO tuning parameters
    aec_motion_iso_type         aec_snapshot_motion_iso;                         // Snapshot motion ISO tuning parameters
    aec_fast_aec_type           aec_fast_aec;                                    // Fast AEC convergence tuning parameters
} AEC_algo_struct_type;

//=============================================================================
//  ASD struct
//=============================================================================
typedef struct
{
    int                         ASD_Software_Type;      // 0 means hybrid, 1 means using bayer stats, for UA code only
    snow_scene_detect_type      snow_scene_detect;      // added aggressiveness
    backlit_scene_detect_type   backlit_scene_detect;   // added aggressiveness , added max_percent_threshold

    landscape_scene_detect_type landscape_scene_detect;
    portrait_scene_detect_type  portrait_scene_detect;

    auto_hdr_detect_type        auto_hdr_detect;

    hazy_scene_detect_type      hazy_scene_detect;
} AAA_ASD_struct_type;

//=============================================================================
// Auto Flicker Detection data types
//=============================================================================
typedef struct
{
    int             AFD_continuous_enable;
    float           std_threshold;          // StdThresh
    unsigned char   percent_threshold;      // PctThresh
    unsigned long   diff_threshold;         // DiffThresh
    unsigned long   frame_ct_threshold;     // FrameCtThresh
    unsigned char   num_frames;             // 204,  default 6
    unsigned char   frame_skip;             // 204 , default 1
    unsigned long   num_rows;               // 204 , default 480
    unsigned char   num_frames_settle;      // 205, default 3
    unsigned char   num_peaks_threshold;    // default 6
    float           INTL_adj_factor;        // 0 to 0.45, default 0.25,how much INTL has to be away from band gap.
    int             start_antbanding_table; // default 60, 50 or 60
    float           max_exp_time_ms;        // max exp time we still do AFD, in ms, default 66

    // static band detection
    int             thldLevel2Ratio;        // energy ratio threshold of level 2 processing, default 0.97f*Q8
    int             thldLevel3Ratio;        // energy ratio threshold of level 3 processing, default 0.95f*Q8
    int             thldEkL;                // lower energy threshold of k_th level processing, default 6.4f*Q10
    int             thldEkU;                // energy upper threshold of k_th level processing, default 128.0f*Q10
    int             thldCounterL3;          // positive frame counter threshold for level 3 detection
    int             thldCounterL2;          // positive frame counter threshold for level 2 detection
    int             LevelDepth;             // processing level2
} chromatix_auto_flicker_detection_data_type;

//=============================================================================
//=============================================================================
//  AF related params start
//=============================================================================
//=============================================================================
#define MAX_AF_KERNEL_NUM               2
#define MAX_HPF_2x5_BUFF_SIZE           10
#define MAX_HPF_2x11_BUFF_SIZE          22
#define FILTER_SW_LENGTH_FIR            11
#define FILTER_SW_LENGTH_IIR            6
#define MAX_ACT_MOD_NAME_SIZE           32
#define MAX_ACT_NAME_SIZE               32

#define AF_PD_MAX_NUM_ROI               10
#define AF_PD_MAX_TABLE_ENTRIES         20
#define AF_SCENE_CHANGE_MAX_ENTRIES     5
/* BAF related macro */
#define MAX_BAF_GAMMA_Y_ENTRY           3
#define MAX_BAF_GAMMA_LUT_ENTRY         32
#define MAX_BAF_FIR_ENTRY               13
#define MAX_BAF_IIR_ENTRY               10
#define MAX_BAF_CORING_ENTRY            17
#define MAX_BAF_FILTER                  3
#define MAX_BAF_FILTER_2ND_TIER         2
#define MAX_BAF_ROI_NUM                 282

#ifndef MAX_HPF_BUFF_SIZE
#define MAX_HPF_BUFF_SIZE 22
#endif

typedef enum _af_kernel_type {
  AF_KERNEL_PRIMARY = 0,
  AF_KERNEL_SCALE,
} af_kernel_type;

//=============================================================================
// af_algo_type: Type of algorithm currently supported
//=============================================================================
typedef enum _af_algo_type
{
    AF_PROCESS_DEFAULT              = -2,
    AF_PROCESS_UNCHANGED            = -1,
    AF_EXHAUSTIVE_SEARCH            = 0,
    AF_EXHAUSTIVE_FAST,
    AF_HILL_CLIMBING_CONSERVATIVE,
    AF_HILL_CLIMBING_DEFAULT,
    AF_HILL_CLIMBING_AGGRESSIVE,
    AF_FULL_SWEEP,
    AF_SLOPE_PREDICTIVE_SEARCH,
    AF_CONTINUOUS_SEARCH,
    AF_PROCESS_MAX
} af_algo_type;

//=============================================================================
// single_index_t: Enum for indexing mapping for distance to
// lens position
//=============================================================================
typedef enum _single_index_t
{
    SINGLE_NEAR_LIMIT_IDX    = 0,
    SINGLE_7CM_IDX           = 1,
    SINGLE_10CM_IDX          = 2,
    SINGLE_14CM_IDX          = 3,
    SINGLE_20CM_IDX          = 4,
    SINGLE_30CM_IDX          = 5,
    SINGLE_40CM_IDX          = 6,
    SINGLE_50CM_IDX          = 7,
    SINGLE_60CM_IDX          = 8,
    SINGLE_120CM_IDX         = 9,
    SINGLE_HYP_F_IDX         = 10,
    SINGLE_INF_LIMIT_IDX     = 11,
    SINGLE_MAX_IDX           = 12,
} single_index_t; //TODO : Chage to Enum Capital

/**
 * af_cam_name: Enum for camera profile
 **/
typedef enum _af_cam_name
{
    ACT_MAIN_CAM_0,
    ACT_MAIN_CAM_1,
    ACT_MAIN_CAM_2,
    ACT_MAIN_CAM_3,
    ACT_MAIN_CAM_4,
    ACT_MAIN_CAM_5,
    ACT_WEB_CAM_0,
    ACT_WEB_CAM_1,
    ACT_WEB_CAM_2,
} af_cam_name;

/**
 * _baf_tuning_preset_enum_t : enum for preset_id
 *
 **/
typedef enum _baf_tuning_preset_enum_t
{
    AF_TUNING_CUSTOM                      = -1,
    AF_TUNING_PRESET_0,
    AF_TUNING_PRESET_1,
    AF_TUNING_PRESET_2,
    AF_TUNING_PRESET_3,
    AF_TUNING_PRESET_4,
    AF_TUNING_PRESET_5,
    AF_TUNING_PRESET_MAX
} baf_tuning_preset_enum_t;

/**
 * _baf_roi_pattern_enum_t : enum for ROI pattern type
 *
 **/
typedef enum _baf_roi_pattern_enum_t
{
    BAF_ROI_PATTERN_CUSTOM                = -1,
    BAF_ROI_PATTERN_RECT                  = 0,
    BAF_ROI_PATTERN_CROSS,
    BAF_ROI_PATTERN_DIAMOND,
    BAF_ROI_PATTERN_SPARSE,
    BAF_ROI_PATTERN_MAX
} baf_roi_pattern_enum_t;

/**
 * _af_scene_type_enum_t : enum for AF scene type
 *
 **/
typedef enum _af_scene_type_enum_t
{
    AF_SCENE_TYPE_NORMAL                  = 0,
    AF_SCENE_TYPE_LOWLIGHT                = 1,
    AF_SCENE_TYPE_FACE                    = 2,
    AF_SCENE_TYPE_MAX,
} af_scene_type_enum_t;

/**
 * af_tuning_fdac_type: Enum for indexing mapping for collected DAC for
 *   face forward/up/down
 **/
typedef enum _af_tuning_fdac_type
{
    AF_TUNING_DAC_FACE_FORWARD            = 0,
    AF_TUNING_DAC_FACE_UP,
    AF_TUNING_DAC_FACE_DOWN,
    AF_TUNING_DAC_MAX
} af_tuning_fdac_type;

/**
 * ACTUATOR_TYPE: Enum for Type of actuator, which impacts core algo behaviors
 **/
typedef enum _ACTUATOR_TYPE
{
    ACT_TYPE_CLOSELOOP                    = 0,
    ACT_TYPE_OPENLOOP,
    ACT_TYPE_MAX,
} ACTUATOR_TYPE;

/**
 * _e_AF_FV_STATS_TYPE: Enum for Type of Stats
 **/
typedef enum _e_AF_FV_STATS_TYPE
{
    AF_FV_STATS_TYPE_HW_1                 = 0,
    AF_FV_STATS_TYPE_HW_2                 = 1,
    AF_FV_STATS_TYPE_SW                   = 2,
    AF_FV_STATS_TYPE_MAX                  = 3,
} AF_FV_STATS_TYPE;

/**
 * _af_haf_algo_enum_type: Enum for HAF algorithm types
 *
 *    @AF_HAF_ALGO_TOF: TOF algo
 *    @AF_HAF_ALGO_PDAF: PDAF algo
 *    @AF_HAF_ALGO_DCIAF: DCIAF algo
 *    @AF_HAF_ALGO_DBG: Debug Algo
 **/
typedef enum
{
    AF_HAF_ALGO_TOF                       = 0,
    AF_HAF_ALGO_PDAF,
    AF_HAF_ALGO_DCIAF,
    AF_HAF_ALGO_DBG,
    AF_HAF_ALGO_MAX,
} af_haf_algo_enum_type;

/**
 * _AF_CAM_ORIENTATION_TYPE: Direction of Aux wrt Main Camera
 *
 *    @AF_CAM_ORIENT_LEFT: Towards left
 *    @AF_CAM_ORIENT_RIGHT: Towards right
 *    @AF_CAM_ORIENT_UP: Towards up
 *    @AF_CAM_ORIENT_DOWN: Towards down
 **/
typedef enum _AF_CAM_ORIENTATION_TYPE
{
    AF_CAM_ORIENT_LEFT                  = 0,
    AF_CAM_ORIENT_RIGHT,
    AF_CAM_ORIENT_UP,
    AF_CAM_ORIENT_DOWN,
    AF_CAM_ORIENT_MAX,
} AF_CAM_ORIENTATION_TYPE;

typedef struct _step_size_t
{
    unsigned short rgn_0; /* reserved */
    unsigned short rgn_1;
    unsigned short rgn_2;
    unsigned short rgn_3;
    unsigned short rgn_4; /* reserved */
} step_size_t;

typedef struct _step_size_table_t
{
    step_size_t    Prescan_normal_light;
    step_size_t    Prescan_low_light;
    step_size_t    Finescan_normal_light;
    step_size_t    Finescan_low_light;
} step_size_table_t;

typedef struct _BV_threshold_t
{
    float thres[8]; /* CUR_BV_INFO */ /* 0, 20, 50, 100, 400, 700, OUTDOOR_, Sky */
} BV_threshold_t;

typedef struct _single_threshold_t
{
    float               flat_inc_thres;
    float               flat_dec_thres;
    float               macro_thres;
    float               drop_thres;
    unsigned long int   hist_dec_dec_thres;
    unsigned long int   hist_inc_dec_thres;
    BV_threshold_t      dec_dec_3frame;
    BV_threshold_t      inc_dec_3frame;
    BV_threshold_t      dec_dec;
    BV_threshold_t      dec_dec_noise;
    BV_threshold_t      inc_dec;
    BV_threshold_t      inc_dec_noise;
    BV_threshold_t      flat_threshold;
} single_threshold_t;

typedef struct _FVscore_threshold_t
{
    int                 default_stats;
    BV_threshold_t      score_ratio_showdif;
    float               strict_noise;
    float               normal_noise;
    float               light_noise;
    int                 strong_peak_thres;
    int                 strong_inc_thres;
    int                 strong_dec_thres;
} FVscore_threshold_t;

typedef struct _single_optic_t
{
    unsigned short CAF_far_end;
    unsigned short CAF_near_end;
    unsigned short TAF_far_end;
    unsigned short TAF_near_end;
    unsigned short srch_rgn_1;
    unsigned short srch_rgn_2;
    unsigned short srch_rgn_3;
    unsigned short fine_srch_rgn;
    unsigned short far_zone;
    unsigned short near_zone;
    unsigned short mid_zone;
    unsigned short far_start_pos;
    unsigned short near_start_pos;
    unsigned short init_pos;
} single_optic_t;

typedef struct _af_tuning_single_t
{
    unsigned short       index[SINGLE_MAX_IDX]; /* single_index_t */
    unsigned short       actuator_type;         /* ACTUATOR_TYPE */
    unsigned short       is_hys_comp_needed;
    unsigned short       step_index_per_um;
    step_size_table_t    TAF_step_table;
    step_size_table_t    CAF_step_table;
    unsigned short       PAAF_enable;
    single_threshold_t   sw;
    single_threshold_t   hw;
    float                BV_gain[8];            /* CUR_BV_INFO */
    single_optic_t       optics;
    FVscore_threshold_t  score_thresh;
} af_tuning_single_t;

typedef struct _af_tuning_sp
{
    float           fv_curve_flat_threshold;
    float           slope_threshold1;
    float           slope_threshold2;
    float           slope_threshold3;
    float           slope_threshold4;
    unsigned int    lens_pos_0;
    unsigned int    lens_pos_1;
    unsigned int    lens_pos_2;
    unsigned int    lens_pos_3;
    unsigned int    lens_pos_4;
    unsigned int    lens_pos_5;
    unsigned int    base_frame_delay;
    int             downhill_allowance;
    int             downhill_allowance_1;
} af_tuning_sp_t;

typedef struct _af_tuning_gyro
{
    unsigned char         enable;
    float                 min_movement_threshold;
    float                 stable_detected_threshold;
    unsigned short        unstable_count_th;
    unsigned short        stable_count_th;
    float                 fast_pan_threshold;
    float                 slow_pan_threshold;
    unsigned short        fast_pan_count_threshold;
    unsigned short        sum_return_to_orig_pos_threshold;
    unsigned short        stable_count_delay;
} af_tuning_gyro_t;

typedef struct _af_tuning_lens_sag_comp
{
    unsigned char                         enable;
    unsigned int                          f_dac[AF_TUNING_DAC_MAX];
    float                                 f_dist;
    float                                 trigger_threshold;
} af_tuning_lens_sag_comp_t;

typedef struct _af_tuning_pd_roi_config_t
{
    int                                   roi_loc_y;
    int                                   roi_loc_x;
    int                                   roi_num_rows;
    int                                   roi_num_cols;
} af_tuning_pd_roi_config_t;

typedef struct _af_tuning_pd_roi_t
{
    int                                   num_entries;
    af_tuning_pd_roi_config_t             config[AF_PD_MAX_NUM_ROI];
} af_tuning_pd_roi_t;

typedef struct _af_tuning_pd_noise_tbl_entry_t
{
    float                                 noise_gain;
    float                                 multiplier;
} af_tuning_pd_noise_tbl_entry_t;

typedef struct _af_tuning_pd_conf_tbl_entry_t
{
    float                                 noise_gain;
    int                                   min_conf;
} af_tuning_pd_conf_tbl_entry_t;

typedef struct _af_tuning_pd_stable_tbl_entry_t
{
    int                                   fps;
    int                                   min_stable_cnt;
} af_tuning_pd_stable_tbl_entry_t;

typedef struct _af_tuning_pd_focus_tbl_entry_t
{
    int                                   defocus;
    float                                 move_pcnt;
} af_tuning_pd_focus_tbl_entry_t;

typedef struct _af_tuning_pd_noise_tbl_t
{
    int                                   num_entries;
    af_tuning_pd_noise_tbl_entry_t        entries[AF_PD_MAX_TABLE_ENTRIES];
} af_tuning_pd_noise_tbl_t;

typedef struct _af_tuning_pd_conf_tbl_t
{
    int                                   num_entries;
    af_tuning_pd_conf_tbl_entry_t         entries[AF_PD_MAX_TABLE_ENTRIES];
} af_tuning_pd_conf_tbl_t;

typedef struct _af_tuning_pd_focus_tbl_t
{
    int                                   num_entries;
    af_tuning_pd_focus_tbl_entry_t        entries[AF_PD_MAX_TABLE_ENTRIES];
} af_tuning_pd_focus_tbl_t;

typedef struct _af_tuning_pd_stable_tbl_t
{
    int                                   num_entries;
    af_tuning_pd_stable_tbl_entry_t       entries[AF_PD_MAX_TABLE_ENTRIES];
} af_tuning_pd_stable_tbl_t;

typedef struct _af_tuning_pd_focus_scan_t
{
    int                                   focus_conv_frame_skip;
    int                                   enable_fine_scan;
    int                                   min_fine_scan_range;
    int                                   fine_scan_step_size;
    int                                   focus_done_threshold;
} af_tuning_pd_focus_scan_t;

typedef struct _af_tuning_pd_monitor_t
{
    int                                   wait_after_focus_cnt;
    int                                   wait_conf_recover_cnt;
    float                                 defocused_threshold;
    float                                 depth_stable_threshold;
} af_tuning_pd_monitor_t;

typedef struct _af_tuning_pdaf_t
{
    af_tuning_pd_roi_t                    roi;
    af_tuning_pd_focus_tbl_t              focus_tbl;
    af_tuning_pd_noise_tbl_t              noise_tbl;
    af_tuning_pd_conf_tbl_t               conf_tbl;
    af_tuning_pd_stable_tbl_t             stable_tbl;
    af_tuning_pd_focus_scan_t             focus_scan;
    af_tuning_pd_monitor_t                scene_monitor;
    float                                 reserve[200];
} af_tuning_pdaf_t;

typedef struct _af_tuning_dciaf_t
{
    unsigned char                         monitor_enable;
    unsigned int                          monitor_freq;
    unsigned int                          search_freq;
    float                                 baseline_mm;
    AF_CAM_ORIENTATION_TYPE               aux_direction;
    float                                 macro_est_limit_cm;
    unsigned char                         alignment_check_enable;
    int                                   jump_to_start_limit;
    int                                   num_near_steps;
    int                                   num_far_steps;
    int                                   dciaf_step_size;
    unsigned char                         motion_data_use_enable;
    int                                   scene_change_lens_pos_th;
    int                                   panning_stable_lens_pos_th;
    int                                   panning_stable_duration_ms_bright;
    int                                   panning_stable_duration_ms_low;
    float                                 reserve[100];
} af_tuning_dciaf_t;

//=============================================================================
// af_tuning_chroma_t: Chroma based scene change detection
//
// Variable name: enable
// Description: enable flag for chroma based scene change detection feature
// 3A version:
// Default value: 0
// Data range: 0 or 1
// Constraints:
//
// Variable name: g_over_b_thres
// Description: when G/B ratio is greater than this threshold
// scene change is detected and AF is triggered
// 3A version:
// Default value: 0.5
// Data range: [0 5]
// Constraints: Higher the threshold difficult to trigger AF
//              Smaller the threshold eady to trigger AF
//
// Variable name: g_over_r_thres
// Description: when G/R ratio is greater than this threshold
// scene change is detected and AF is triggered
// 3A version:
// Default value: 0.4
// Data range: [0 5]
// Constraints: Higher the threshold difficult to trigger AF
//              Smaller the threshold eady to trigger AF
//
// Variable name: aec_gain_input
// Description: Setting different gain levels to have different thresholds
// 3A version:
// Default value: {5, 10, 18, 22, 30}
// Data range: [0 30]
// Constraints:
//
// Variable name: g_over_b_threshold_scaler
// Description: Scaler threshold to multiple with g_over_b_thres at
// different gain level
// 3A version:
// Default value: {1, 1.5, 2, 2.2, 2.5}
// Data range: [1 5]
// Constraints: Higher the threshold difficult to trigger AF
//              Smaller the threshold eady to trigger AF
//
// Variable name: g_over_r_threshold_scaler
// Description: Scaler threshold to multiple with g_over_r_thres at
// different gain level
// 3A version:
// Default value: {1, 1.5, 2, 2.2, 2.5}
// Data range: [1 5]
// Constraints: Higher the threshold difficult to trigger AF
//              Smaller the threshold eady to trigger AF
//
// Variable name: unstable_count_th
// Description: number of frames to detect unstable scene change
// 3A version:
// Default value: 5
// Data range: [1 20]
// Constraints: Higher the threshold difficult to trigger AF
//              Smaller the threshold eady to trigger AF
//
// Variable name: num_of_samples
// Description: number of frames to monitor for scene change detection
// 3A version:
// Default value: 5
// Data range: [1 20]
//=============================================================================
typedef struct _af_tuning_chroma_t
{
  unsigned char     enable;
  float             g_over_b_thres;
  float             g_over_r_thres;
  float             aec_gain_input[AF_SCENE_CHANGE_MAX_ENTRIES];
  float             g_over_b_threshold_scaler[AF_SCENE_CHANGE_MAX_ENTRIES];
  float             g_over_r_threshold_scaler[AF_SCENE_CHANGE_MAX_ENTRIES];
  unsigned short    unstable_count_th;
  unsigned short    num_of_samples;
  float             reserve[20];
} af_tuning_chroma_t;

typedef struct _af_tuning_tof_t
{
    int             lens_laser_dist_comp_mm;
    int             wait_frame_limt;
    int             max_tof_srch_cnt;
    int             jump_to_start_limit;
    int             num_near_steps;
    int             num_far_steps;
    int             tof_step_size;
    int             normal_light_cnt;
    int             lowlight_cnt;
    int             num_monitor_samples;
    int             scene_change_distance_thres;
    int             scene_change_lens_pos_thres;
    int             far_scene_coarse_srch_enable;
    unsigned char   enable;
    int             outdoor_lux_idx;
    float           reserve[100];
} af_tuning_tof_t;

typedef struct _af_fine_srch_extension_t
{
    int             max_fine_srch_extension_cnt;
    int             num_near_steps;
    int             num_far_steps;
    int             step_size;
    float           decrease_drop_ratio;
} af_fine_srch_extension_t;

typedef struct _af_tuning_haf_t
{
    unsigned char       enable;
    unsigned char       algo_enable[AF_HAF_ALGO_MAX];
    unsigned char       stats_consume_enable[AF_FV_STATS_TYPE_MAX];
    char                lens_sag_comp_enable;
    char                hysteresis_comp_enable;
    int                 actuator_shift_comp;
    int                 actuator_hysteresis_comp;
    float               actuator_sensitivity;
    AF_FV_STATS_TYPE    stats_select;
    float               fine_srch_drop_thres;
    unsigned int        fine_step_size;
    unsigned int        max_move_step;
    unsigned int        max_move_step_buffer;
    unsigned int        base_frame_delay;
    af_tuning_pdaf_t    pdaf;
    af_tuning_tof_t     tof;
    af_tuning_dciaf_t   dciaf;
    float               reserve[100];
} af_tuning_haf_t;

typedef struct _af_tuning_vbt_t
{
    unsigned char       enable;
    float               comp_time;
    float               contamination_pcnt_allow;
} af_tuning_vbt_t;

//=============================================================================
// af_tuning_smooth_lens_convergence: Smooth lens convergence feature parameters
//
// Variable name: enable
// Description: enable flag for smooth lens convergence
// 3A version:
// Default value: 0
// Data range: 0 or 1
// Constraints: None
// Effect:
//
// Variable name: fudge_factor
// Description: Compensation factor inorder to get the
//              right crop factor
// 3A version:
// Default value: 1
// Data range: [0 1]
// Constraints: None
//
// Variable name: anchor_lens_pos1
// Description: Lens position corresponding to the anchor_distance_mm_1
//              Need to specify the lens position, can refer to Contrast AF
//              tuning settings from single_index_t
// 3A version:
// Default value: lens position specified at SINGLE_10CM_IDX in single_index_t
// Data range: [near_end far_end]
//
// Variable name: anchor_lens_pos2
// Description: Lens position corresponding to the anchor_distance_mm_2
//              Need to specify the lens position, can refer to Contrast AF
//              tuning settings from single_index_t
// 3A version:
// Default value: lens position specified at SINGLE_HYP_F_IDX in single_index_t
// Data range: [near_end far_end]
//
// Variable name: anchor_distance_mm_1
// Description: Physical distance in millimeters at which anchor_lens_pos1
//              corresponds to.
// 3A version:
// Default value: 100
// Data range: [0 3000]
//
// Variable name: anchor_distance_mm_2
// Description: Physical distance in millimeters at which anchor_lens_pos2
//              corresponds to.
// 3A version:
// Default value: 2000
// Data range: [0 3000]
//
// Variable name: focus_converge_step_size_bright_light
// Description: Step size for focus converge in bright light
// 3A version:
// Default value: 8
// Data range: [0 50]
//
// Variable name: fine_search_step_size_bright_light
// Description: Step size for fine search in bright light
// 3A version:
// Default value: 6
// Data range: [0 50]
//
// Variable name: focus_converge_step_size_low_light
// Description: Step size for focus converge in low light
// 3A version:
// Default value: 9
// Data range: [0 50]
//
// Variable name: fine_search_step_size_low_light
// Description: Step size for fine search in low light
// 3A version:
// Default value: 7
// Data range: [0 50]
//=============================================================================
typedef struct _af_tuning_fovc
{
    unsigned char   enable;
    float           fudge_factor;
    int             anchor_lens_pos1;
    int             anchor_lens_pos2;
    int             anchor_distance_mm_1;
    int             anchor_distance_mm_2;
    int             focus_converge_step_size_bright_light;
    int             fine_search_step_size_bright_light;
    int             focus_converge_step_size_low_light;
    int             fine_search_step_size_low_light;
    float           reserve[20];
} af_tuning_fovc;

//=============================================================================
// af_tuning_spotlight_af: Spotlight detection/af process feature parameters
//
// Variable name: enable
// Description: enable flag for spotlight af process
// 3A version:
// Default value: 0
// Data range: 0 or 1
// Constraints: None
//
// Variable name: far_flat_th
// Description: flat_threshold for far scene when detecting spotlight condition
// 3A version:
// Default value: 0.55
// Data range: [0 1]
// Constraints: None
// Effect: The lower the easier to be recognized as flat scene
//
// Variable name: marco_flat_th
// Description: flat_threshold for marco scene when detecting spotlight condition
// 3A version:
// Default value: 0.85
// Data range: [0 1]
// Constraints: None
// Effect: The lower the easier to be recognized as flat scene
//
// Variable name: inc_flat_th
// Description: inc_flat_threshold to judge current af position is in increase or flat condition
// 3A version:
// Default value: 1.035
// Data range: [1 3]
// Constraints: None
// Effect: The larger the harder to be recognized as FV increment
//
// Variable name: dec_flat_th
// Description: dec_flat_threshold to judge current af position is in decrease or flat condition
// 3A version:
// Default value: 0.978
// Data range: [0 1]
// Constraints: None
// Effect: The lower the harder to be recognized as FV decrement
//
// Variable name: spotlight_zone_start_h
// Description: Defines the horizontal start region of spotlight zone in 16x16 total regions
// 3A version:
// Default value:4
// Data range: [0 15]
// Constraints: need to be smaller than spotlight_zone_end_h and also smaller than background_zone_start_h
//
// Variable name: spotlight_zone_end_h
// Description: Defines the horizontal end region of spotlight zone in 16x16 total regions,
//              the spotlight zone horizontal end region = spotlight_zone_end_h - 1
// 3A version:
// Default value:12
// Data range: [1 16]
// Constraints: need to be bigger than spotlight_zone_start_h;
//
// Variable name: spotlight_zone_start_v
// Description: Defines the vertical start region of spotlight zone in 16x16 total regions
// 3A version:
// Default value:4
// Data range: [0 15]
// Constraints: need to be smaller than spotlight_zone_end_v
//
// Variable name: spotlight_zone_end_v
// Description: Defines the vertical end region of spotlight zone in 16x16 total regions,
//             the spotlight zone vertical end region = spotlight_zone_end_v - 1
// 3A version:
// Default value:12
// Data range: [1 16]
// Constraints: need to be bigger than spotlight_zone_start_v
//
// Variable name: background_zone_start_h
// Description: Defines the horizontal start region of background zone in 16x16 total regions
// 3A version:
// Default value:2
// Data range: [0 15]
// Constraints: need to be smaller than background_zone_end_h
//
// Variable name: background_zone_end_h
// Description: Defines the horizontal end region of background zone in 16x16 total regions,
//              the background zone horizontal end region = background_zone_end_h - 1
// 3A version:
// Default value:14
// Data range: [1 16]
// Constraints: need to be bigger than background_zone_start_h
//
// Variable name: background_zone_start_v
// Description: Defines the vertical start region of background zone in 16x16 total regions
// 3A version:
// Default value:2
// Data range: [0 15]
// Constraints: need to be smaller than background_zone_end_v
//
// Variable name: background_zone_end_v
// Description: Defines the vertical end region of background zone in 16x16 total regions,
//              the background zone vertical end region = background_zone_end_v - 1
// 3A version:
// Default value:14
// Data range: [1 16]
// Constraints: need to be bigger than background_zone_start_v
//
// Variable name: exposure_index_trigger_th
// Description: if exposure index is smaller than this threshold, the spotlight will not be detected
// 3A version:
// Default value:350
// Data range: [200 400]
// Constraints: None
//
// Variable name: background_luma_avg_th
// Description: If the average luma of background zone is higher than this threshold,
//              the spotlight will not be detected
// 3A version:
// Default value:30
// Data range: [1 255]
// Constraints:
//
// Variable name: region_low_grey_th
// Description: Defines the low grey luma threshold. If luma is smaller than this threshold,
//               it will be recognized as a low grey region, suggest to disable it by default value 255
// 3A version:
// Default value:255
// Data range: [1 255]
// Constraints:
//
// Variable name: region_dark_grey_th
// Description: Defines the dark grey luma threshold. If luma is smaller than this threshold,
//              it will be recognized as a dark grey region, good to keep default value
// 3A version:
// Default value:7
// Data range: [1 255]
// Constraints:
//
// Variable name: background_low_grey_pct_th
// Description: if the low grey percentage is higher than this threshold, the low grey flag will be raised.
//              Suggest to disable it by default
// 3A version:
// Default value:1.0
// Data range: [0 1.0]
// Constraints: If dark grey has been detected first, the low grey will not be used anymore
//
// Variable name: background_dark_grey_pct_th
// Description: if the dark grey percentage is higher than this threshold, the dark grey flag will be raised
// 3A version:
// Default value:0.3
// Data range: [0 1.0]
// Constraints: If dark grey has been detected first, the low grey will not be used anymore
//
// Variable name: bright_region_luma_delta_background_zone
// Description: if region’s luma is higher than average luma of background zone multiply this parameter, this region will be recognize as a bright region
// 3A version:
// Default value:2.0
// Data range: [0 8.0]
// Constraints:
//
// Variable name: bright_region_luma_delta_spotlight_zone_low_grey
// Description: In low grey condition and cluster is enabled,if region’s luma is higher than average luma of background zone multiply
//                this parameter, this region will be recognize as a bright region
// 3A version:
// Default value:2.5
// Data range: [0 8.0]
// Constraints:
//
// Variable name: bright_region_luma_delta_spotlight_zone_dark_grey
// Description: In dark grey condition and cluster is enabled,if region’s luma is higher than average luma of background zone multiply
//                this parameter, this region will be recognize as a bright region
// 3A version:
// Default value:3.0
// Data range: [0 8.0]
// Constraints:
//
// Variable name: square_cluster_inc_size
// Description: The maximum bright region will be the center region and the square cluster size will depends
//              on the parameter, set to 0 will disable this cluster effect
// 3A version:
// Default value:1
// Data range: [0 2]
// Constraints:
//
// Variable name: vertical_cluster_inc_size
// Description: The maximum bright region will be the center region and the vertical cluster size will depends
//              on the parameter, set to 0 will disable this cluster effect
// 3A version:
// Default value:2
// Data range: [0 2]
// Constraints:
//
// Variable name: horizontal_cluster_inc_size
// Description: The maximum bright region will be the center region and the vertical cluster size will depends
//              on the parameter, set to 0 will disable this cluster effect
// 3A version:
// Default value:2
// Data range: [0 2]
// Constraints:
//
// Variable name: square_cluster_pct_th
// Description: If the bright region percentage of square cluster zones is higher this threshold,
//              the flag of square_bright_region_cluster will be raised
// 3A version:
// Default value:0.8
// Data range: [0 1.0]
// Constraints:
//
// Variable name: vertical_cluster_pct_th
// Description: If the bright region percentage of vertical cluster zones is higher this threshold, the flag of
//              vertical_bright_region_cluster will be raised
// 3A version:
// Default value:0.2
// Data range: [0 1.0]
// Constraints:
//
// Variable name: horizontal_cluster_pct_th
// Description: If the bright region percentage of horizontal cluster zones is higher this threshold, the flag of
//              horizontal_bright_region_cluster will be raised
// 3A version:
// Default value:0.2
// Data range: [0 1.0]
// Constraints:
//
// Variable name: spotlight_detect_cnt_th
// Description: Spotlight detect flag threshold. Only if consecutive success spotlight detect over the threshold
//              count will result in the final spotlight detected flag be updated.
// 3A version:
// Default value:3
// Data range: [1 infinity]
// Constraints:
//=============================================================================
typedef struct _af_tuning_spot_light
{
    unsigned char              enable;
    float                      far_flat_th;
    float                      marco_flat_th;
    float                      inc_flat_th;
    float                      dec_flat_th;
    unsigned int               spotlight_zone_start_h;
    unsigned int               spotlight_zone_end_h;
    unsigned int               spotlight_zone_start_v;
    unsigned int               spotlight_zone_end_v;
    unsigned int               background_zone_start_h;
    unsigned int               background_zone_end_h;
    unsigned int               background_zone_start_v;
    unsigned int               background_zone_end_v;
    int                        exposure_index_trigger_th;
    unsigned int               background_luma_avg_th;
    unsigned int               region_low_grey_th;
    unsigned int               region_low_dark_th;
    float                      background_low_grey_pct_th;
    float                      background_dark_grey_pct_th;
    float                      bright_region_luma_delta_background_zone;
    float                      bright_region_luma_delta_spotlight_zone_low_grey;
    float                      bright_region_luma_delta_spotlight_zone_dark_grey;
    unsigned int               square_cluster_inc_size;
    unsigned int               vertical_cluster_inc_size;
    unsigned int               horizontal_cluster_inc_size;
    float                      square_cluster_pct_th;
    float                      vertical_cluster_pct_th;
    float                      horizontal_cluster_pct_th;
    unsigned int               spotlight_detect_cnt_th;
    float                      reserved[20];
} af_tuning_spot_light_t;

typedef struct _af_tuning_sad
{
    unsigned char   enable;
    float           gain_min;
    float           gain_max;
    float           ref_gain_min;
    float           ref_gain_max;
    unsigned short  threshold_min;
    unsigned short  threshold_max;
    unsigned short  ref_threshold_min;
    unsigned short  ref_threshold_max;
    unsigned short  frames_to_wait;
} af_tuning_sad_t;

typedef struct _af_tuning_continuous
{
    unsigned char           enable;
    unsigned char           scene_change_detection_ratio;
    float                   panning_stable_fv_change_trigger;
    float                   panning_stable_fvavg_to_fv_change_trigger;
    unsigned short          panning_unstable_trigger_cnt;
    unsigned short          panning_stable_trigger_cnt;
    unsigned long           downhill_allowance;
    unsigned short          uphill_allowance;
    unsigned short          base_frame_delay;
    unsigned short          scene_change_luma_threshold;
    unsigned short          luma_settled_threshold;
    float                   noise_level_th;
    unsigned short          search_step_size;
    af_algo_type            init_search_type;
    af_algo_type            search_type;
    unsigned short          low_light_wait;
    unsigned short          max_indecision_cnt;
    float                   flat_fv_confidence_level;
    af_tuning_sad_t         af_sad;
    af_tuning_gyro_t        af_gyro;
    af_tuning_chroma_t      af_chroma;
    af_tuning_spot_light_t  af_spot_light;
} af_tuning_continuous_t;

typedef struct _af_tuning_exhaustive
{
    unsigned short num_gross_steps_between_stat_points;
    unsigned short num_fine_steps_between_stat_points;
    unsigned short num_fine_search_points;
    unsigned short downhill_allowance;
    unsigned short uphill_allowance;
    unsigned short base_frame_delay;
    unsigned short coarse_frame_delay;
    unsigned short fine_frame_delay;
    unsigned short coarse_to_fine_frame_delay;
    float          noise_level_th;
    float          flat_fv_confidence_level;
    float          climb_ratio_th;
    int            low_light_luma_th;
    int            enable_multiwindow;
    float          gain_thresh;
} af_tuning_exhaustive_t;

typedef struct _af_tuning_fullsweep_t
{
    unsigned short num_steps_between_stat_points;
    unsigned short frame_delay_inf;
    unsigned short frame_delay_norm;
    unsigned short frame_delay_final;
} af_tuning_fullsweep_t;

typedef struct _af_shake_resistant
{
    unsigned char   enable;
    float           max_gain;
    unsigned char   min_frame_luma;
    float           tradeoff_ratio;
    unsigned char   toggle_frame_skip;
} af_shake_resistant_t;

typedef struct _af_motion_sensor
{
    float af_gyro_trigger;
    float af_accelerometer_trigger;
    float af_magnetometer_trigger;
    float af_dis_motion_vector_trigger;
} af_motion_sensor_t;

typedef struct _af_fd_priority_caf
{
    float   pos_change_th;
    float   pos_stable_th_hi;
    float   pos_stable_th_low;
    float   size_change_th;
    float   old_new_size_diff_th;
    int     stable_count_size;
    int     stable_count_pos;
    int     no_face_wait_th;
    int     fps_adjustment_th;
} af_fd_priority_caf_t;

//=============================================================================
// af_tuning_multiwindow_t: AF tuning parameters specific to
//  Multiwindow feature
//
// Variable name: enable
// Description: enable flag for multiwindow feature
// 3A version:
// Default value: 0
// Data range: 0 or 1
// Constraints: None
// Effect:
//
// Variable name: h_clip_ratio_normal_light
// AF window horizontal size in terms of ratio to the whole image. For the
// same example above, Horizontal Clip Ratio is 500/1000=0.5.
// 3A version:
// Default value: 0.5
// Data range: 0 - 1
// Constraints:
// Effect:
//
// Variable name: v_clip_ratio_normal_light
// AF window vertical size in terms of ratio to the whole image. For the
// same example above, Vertical Clip Ratio is 500/1000=0.5.
// 3A version:
// Default value: 0.5
// Data range: 0 -1
// Constraints:
// Effect:
//
// Variable name: h_clip_ratio_low_light
// AF window horizontal size in terms of ratio to the whole image. For the
// same example above, Horizontal Clip Ratio is 500/1000=0.5.
// 3A version:
// Default value: 0.5
// Data range: 0 - 1
// Constraints:
// Effect:
//
// Variable name: v_clip_ratio_low_light
// AF window vertical size in terms of ratio to the whole image. For the
// same example above, Vertical Clip Ratio is 500/1000=0.5.
// 3A version:
// Default value: 0.5
// Data range: 0 - 1
// Constraints:
// Effect:
//
// Variable name: h_num_default
// Default horizontal number of grids in an ROI.
// 3A version:
// Default value: 6
// Data range: [1 18]
// Constraints:
// Effect:
//
// Variable name: v_num_default
// Default vertical number of grids in an ROI.
// 3A version:
// Default value: 6
// Data range: [1 14]
// Constraints:
// Effect:
//
// Variable name: h_num_windows
// Default horizontal number of windows in an ROI.
// 3A version:
// Default value: 5
// Data range: [1 h_num_default]
// Constraints:
// Effect:
//
// Variable name: v_num_windows
// Default vertical number of windows in an ROI.
// 3A version:
// Default value: 5
// Data range: [1 v_num_default]
// Constraints:
// Effect:
//=============================================================================
typedef struct _af_tuning_multiwindow_t
{
    int     enable;
    float   h_clip_ratio_normal_light;
    float   v_clip_ratio_normal_light;
    float   h_clip_ratio_low_light;
    float   v_clip_ratio_low_light;
    int     h_num_default;
    int     v_num_default;
    int     h_num_windows;
    int     v_num_windows;
    float   reserve[50];
} af_tuning_multiwindow_t;

//=============================================================================
// af_focus_mode_type: Enum for List of focus modes supported
//
//  AF_FOCUS_MODE_DEFAULT: Default focus mode
//  AF_FOCUS_MODE_MACRO: Macro  focus mode
//  AF_FOCUS_MODE_NORMAL: Normal focus mode
//  AF_FOCUS_MODE_CAF: Continuous focus mode
//  AF_FOCUS_MODE_MANUAL: Manual focus mode
//=============================================================================
typedef enum
{
    AF_FOCUS_MODE_DEFAULT,
    AF_FOCUS_MODE_MACRO,
    AF_FOCUS_MODE_NORMAL,
    AF_FOCUS_MODE_CAF,
    AF_FOCUS_MODE_MANUAL,
    AF_FOCUS_MODE_MAX
} af_focus_mode_type;

/**
* Structure defining search limits applicable to
* each focus mode.
* Note: This structure is to be moved to chromatix in the next
* chromatix revision.
*/
//=============================================================================
// af_tuning_focus_mode_search_limit_t: AF tuning parameters defining search limits applicable to each focus mode
//
// Variable name: enable
// Description: enable/disable flag for the feature
// Default values: 0
// Range: [0,1]
//
//  Variable name: near_end
//  Description: Near end position for the mode
//  Default value: position_near_end.
//  Data range: [position_near_end position_far_end]
//
//  Variable name: far_end
//  Description: Near end position for the mode
//  Default value: position_far_end.
//  Data range: [position_near_end position_far_end]
//
//  Variable name: default_pos
//  Description: default position for the mode
//  Default value: position_default_in_normal.
//  Data range: [position_near_end position_far_end]
//=============================================================================
typedef struct af_tuning_focus_mode_search_limit_t
{
    int          enable;
    unsigned int near_end;
    unsigned int far_end;
    unsigned int default_pos;
} af_tuning_focus_mode_search_limit_t;

typedef struct _af_tuning_algo
{
    af_algo_type                        af_process_type;
    unsigned short                      position_near_end;
    unsigned short                      position_default_in_macro;
    unsigned short                      position_boundary;
    unsigned short                      position_default_in_normal;
    unsigned short                      position_far_end;
    unsigned short                      position_normal_hyperfocal;
    unsigned short                      position_macro_rgn;
    unsigned short                      undershoot_protect;
    unsigned short                      undershoot_adjust;
    float                               min_max_ratio_th;
    int                                 led_af_assist_enable;
    long                                led_af_assist_trigger_idx;
    int                                 lens_reset_frame_skip_cnt;
    float                               low_light_gain_th;
    float                               base_delay_adj_th;
    af_tuning_continuous_t              af_cont;
    af_tuning_exhaustive_t              af_exh;
    af_tuning_fullsweep_t               af_full;
    af_tuning_sp_t                      af_sp;
    af_tuning_single_t                  af_single;
    af_shake_resistant_t                af_shake_resistant;
    af_motion_sensor_t                  af_motion_sensor;
    af_fd_priority_caf_t                fd_prio;
    af_tuning_lens_sag_comp_t           lens_sag_comp;
    af_tuning_haf_t                     af_haf;
    af_tuning_vbt_t                     vbt;
    af_tuning_fovc                      fovc;
    af_tuning_multiwindow_t             af_multiwindow;
    af_tuning_focus_mode_search_limit_t search_limit[AF_FOCUS_MODE_MAX];
    float                               reserve[100];
} af_tuning_algo_t;

typedef struct _af_vfe_config
{
    unsigned short  fv_min;
    unsigned short  max_h_num;
    unsigned short  max_v_num;
    unsigned short  max_block_width;
    unsigned short  max_block_height;
    unsigned short  min_block_width;
    unsigned short  min_block_height;
    float           h_offset_ratio_normal_light;
    float           v_offset_ratio_normal_light;
    float           h_clip_ratio_normal_light;
    float           v_clip_ratio_normal_light;
    float           h_offset_ratio_low_light;
    float           v_offset_ratio_low_light;
    float           h_clip_ratio_low_light;
    float           v_clip_ratio_low_light;
    float           touch_scaling_factor_normal_light;
    float           touch_scaling_factor_low_light;
    int             bf_scale_factor;
    unsigned short  h_num_default;
    unsigned short  v_num_default;
} af_vfe_config_t;

typedef struct _af_vfe_legacy_hpf
{
    char      a00;
    char      a02;
    char      a04;
    char      a20;
    char      a21;
    char      a22;
    char      a23;
    char      a24;
} af_vfe_legacy_hpf_t;

typedef struct _af_vfe_hpf
{
    af_vfe_legacy_hpf_t     af_hpf;
    int                     bf_hpf_2x5[MAX_HPF_2x5_BUFF_SIZE];
    int                     bf_hpf_2x11[MAX_HPF_2x11_BUFF_SIZE];
} af_vfe_hpf_t;

typedef struct _af_vfe_sw_fir_hpf_t
{
    int                    a[FILTER_SW_LENGTH_FIR];
    double                 fv_min_hi;
    double                 fv_min_lo;
    unsigned long int      coeff_length;
} af_vfe_sw_fir_hpf_t;

typedef struct _af_vfe_sw_iir_hpf_t
{
    double                 a[FILTER_SW_LENGTH_IIR];
    double                 b[FILTER_SW_LENGTH_IIR];
    double                 fv_min_hi;
    double                 fv_min_lo;
    unsigned long int      coeff_length;
} af_vfe_sw_iir_hpf_t;

typedef struct af_vfe_sw_hpf_t
{
    unsigned short         filter_type;    /* af_sw_filter_type */
    af_vfe_sw_fir_hpf_t    fir;
    af_vfe_sw_iir_hpf_t    iir;
} af_vfe_sw_hpf_t;

typedef struct _baf_tuning_gamma_t
{
    unsigned char                         ch_sel;
    float                                 Y_coeffs[MAX_BAF_GAMMA_Y_ENTRY];
    unsigned char                         g_sel;
    unsigned char                         LUT_enable;
    int                                   gamma_LUT[MAX_BAF_GAMMA_LUT_ENTRY];
} baf_tuning_gamma_t;

typedef struct _baf_tuning_gamma_param_t
{
    baf_tuning_preset_enum_t              preset_id;
    baf_tuning_gamma_t                    gamma_custom;
} baf_tuning_gamma_param_t;

typedef struct _baf_tuning_coring_t
{
    unsigned int                          x_index[MAX_BAF_CORING_ENTRY];
    int                                   threshold;
    int                                   gain;
} baf_tuning_coring_t;

typedef struct _baf_tuning_HV_filter_t
{
    unsigned char                         fir_en;
    unsigned char                         iir_en;
    int                                   fir[MAX_BAF_FIR_ENTRY];
    float                                 iir[MAX_BAF_IIR_ENTRY];
    int                                   shift_bits;
    unsigned char                         scaler_sel;
} baf_tuning_HV_filter_t;

typedef struct _baf_HV_filter_t
{
    baf_tuning_preset_enum_t              preset_id;
    baf_tuning_coring_t                   coring;
    baf_tuning_HV_filter_t                filter_custom;
} baf_HV_filter_t;

typedef struct _baf_tuning_scaler_t
{
    int                                   M;
    int                                   N;
} baf_tuning_scaler_t;

typedef struct _baf_tuning_filter_t
{
    baf_HV_filter_t                       HV_filter[MAX_BAF_FILTER];
    baf_HV_filter_t                       HV_filter_2nd_tier[MAX_BAF_FILTER_2ND_TIER];
    baf_tuning_scaler_t                   scaler;
} baf_tuning_filter_t;

typedef struct _baf_tuning_roi_t
{
    unsigned char                         flag;
    unsigned int                          x;
    unsigned int                          y;
    unsigned int                          width;
    unsigned int                          height;
} baf_tuning_roi_t;

typedef struct _baf_tuning_roi_param_t
{
    baf_roi_pattern_enum_t                preset_id;
    float                                 grid_size_h;
    float                                 grid_size_v;
    float                                 grid_gap_h;
    float                                 grid_gap_v;
    int                                   num_ROI;
    baf_tuning_roi_t                      ROI_custom[MAX_BAF_ROI_NUM];
} baf_tuning_roi_param_t;

typedef struct _af_tuning_baf_t
{
    baf_tuning_roi_param_t                roi_config[AF_SCENE_TYPE_MAX];
    baf_tuning_gamma_param_t              gamma[AF_SCENE_TYPE_MAX];
    baf_tuning_filter_t                   filter[AF_SCENE_TYPE_MAX];
} af_tuning_baf_t;

typedef struct _af_tuning_vfe
{
    unsigned short   fv_metric;
    af_vfe_config_t  config;
    af_vfe_hpf_t     hpf_default;
    af_vfe_hpf_t     hpf_face;
    af_vfe_hpf_t     hpf_low_light;
} af_tuning_vfe_t;

typedef struct _af_tuning_sw_stats
{
    af_vfe_sw_hpf_t  sw_hpf_default;
    af_vfe_sw_hpf_t  sw_hpf_face;
    af_vfe_sw_hpf_t  sw_hpf_lowlight;
    float            sw_fv_min_lux_trig_hi;
    float            sw_fv_min_lux_trig_lo;
} af_tuning_sw_stats_t;

typedef struct _af_header_info
{
    unsigned short  header_version;
    char            module_name[MAX_ACT_MOD_NAME_SIZE];
    char            actuator_name[MAX_ACT_NAME_SIZE];
    af_cam_name     cam_name;
} af_header_info_t;

typedef struct _pdaf_loc
{
    int x;
    int y;
} pdaf3_loc_t;

typedef struct _af_pdaf3_params
{
    //Internal Tuning Parameters
    pdaf3_loc_t     RegionTopLeft;          // ROI Region for PDAF Phase Difference Calculation
    pdaf3_loc_t     RegionBotRight;         // ROI Region for PDAF Phase Difference Calculation
    unsigned char   SmoothFilter[7];        // 1D Smooth Filter
    short           SobelFilter[9];         // 1D Sobel Filter
    int             Confidence_k;           // Confidence Value Coefficient Q64: 64 mean 1.0
    int             DefocusConfidenceTh;    // Confidence Level Threshold

    //Customer Tuning Parameter
    unsigned char   HorGridNumber;          // Grid Number along x direction
    unsigned char   VerGridNumber;          // Grid Number along y direction
} af_pdaf3_params_t;

typedef struct _af_algo_tune_parms
{
    af_header_info_t      af_header_info;
    af_tuning_algo_t      af_algo;
    af_tuning_sw_stats_t  af_swaf_config;
    af_tuning_vfe_t       af_vfe[MAX_AF_KERNEL_NUM];
    af_tuning_baf_t       af_baf;
    af_pdaf3_params_t     af_pdaf3;
} af_algo_tune_parms_t;

//=============================================================================
//=============================================================================
//  AF related params end
//=============================================================================
//=============================================================================

//=============================================================================
// feature name: Electronic Image Stabilization 2.0
// variable name: rs_offset_1
// Brief description of the parameter: Offset to adjust the rolling shutter
//   gyro interval start/end times to better align with the frame.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 10000
// data range: -15000 to 15000 (usec)
// constraints: -15000 to 15000
//   For exposure times less than rs_threshold_1, the rolling shutter gyro
//   interval start/end times are adjusted by rs_offset_1.
// effect: Rolling shutter correction
//
// variable name: rs_offset_2
// Brief description of the parameter: Offset to adjust the rolling shutter
//   gyro interval start/end times to better align with the frame.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 5000
// data range: -15000 to 15000 (usec)
// constraints: -15000 to 15000
//   For exposure times less than rs_threshold_2, the rolling shutter gyro
//   interval start/end times are adjusted by rs_offset_2.
// effect: Rolling shutter correction
//
// variable name: rs_offset_3
// Brief description of the parameter: Offset to adjust the rolling shutter
//   gyro interval start/end times to better align with the frame.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 0
// data range: -15000 to 15000 (usec)
// constraints: -15000 to 15000
//   For exposure times less than rs_threshold_3, the rolling shutter gyro
//   interval start/end times are adjusted by rs_offset_3.
// effect: Rolling shutter correction
//
// variable name: rs_offset_4
// Brief description of the parameter: Offset to adjust the rolling shutter
//   gyro interval start/end times to better align with the frame.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: -5000
// data range: -15000 to 15000 (usec)
// constraints: -15000 to 15000
//   For exposure times greater than or equal to rs_threshold_3, the rolling
//   shutter gyro interval start/end times are adjusted by rs_offset_4.
// effect: Rolling shutter correction
//
// variable name: s3d_offset_1
// Brief description of the parameter: Offset to adjust the 3D shake gyro
//   interval start/end times to better align with the frame.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 10000
// data range: -15000 to 15000 (usec)
// constraints: -15000 to 15000
//   For exposure times less than s3d_threshold_1, the 3D shake gyro interval
//   start/end times are adjusted by s3d_offset_1.
// effect: 3D shake correction
//
// variable name: s3d_offset_2
// Brief description of the parameter: Offset to adjust the 3D shake gyro
//   interval start/end times to better align with the frame.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 5000
// data range: -15000 to 15000 (usec)
// constraints: -15000 to 15000
//   For exposure times less than s3d_threshold_2, the 3D shake gyro interval
//   start/end times are adjusted by s3d_offset_2.
// effect: 3D shake correction
//
// variable name: s3d_offset_3
// Brief description of the parameter: Offset to adjust the 3D shake gyro
//   interval start/end times to better align with the frame.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 0
// data range: -15000 to 15000 (usec)
// constraints: -15000 to 15000
//   For exposure times less than s3d_threshold_3, the 3D shake gyro interval
//   start/end times are adjusted by s3d_offset_3.
// effect: 3D shake correction
//
// variable name: s3d_offset_4
// Brief description of the parameter: Offset to adjust the 3D shake gyro
//   interval start/end times to better align with the frame.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: -10000
// data range: -15000 to 15000 (usec)
// constraints: -15000 to 15000
//   For exposure times greater than or equal to s3d_threshold_3, the 3D shake
//   gyro interval start/end times are adjusted by s3d_offset_4.
// effect: 3D shake correction
//
// variable name: rs_threshold_1
// Brief description of the parameter: The exposure threshold whereby exposure
//   times above and below this threshold results in a different offset getting
//   applied to the rolling shutter gyro time interval.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 0.010
// data range: 0.00 to 0.050 (sec)
// constraints: 0.00 to 0.050
//   For exposure times less than rs_threshold_1, the rolling shutter gyro
//   interval start/end times are adjusted by rs_offset_1.
// effect: Rolling shutter correction
//
// variable name: rs_threshold_2
// Brief description of the parameter: The exposure threshold whereby exposure
//   times above and below this threshold results in a different offset getting
//   applied to the rolling shutter gyro time interval.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 0.020
// data range: 0.00 to 0.050 (sec)
// constraints: 0.00 to 0.050
//   For exposure times less than rs_threshold_2, the rolling shutter gyro
//   interval start/end times are adjusted by rs_offset_2.
// effect: Rolling shutter correction
//
// variable name: rs_threshold_3
// Brief description of the parameter: The exposure threshold whereby exposure
//   times above and below this threshold results in a different offset getting
//   applied to the rolling shutter gyro time interval.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 0.028
// data range: 0.00 to 0.050 (sec)
// constraints: 0.00 to 0.050
//   For exposure times less than rs_threshold_3, the rolling shutter gyro
//   interval start/end times are adjusted by rs_offset_3.  For exposure times
//   greater than or equal to rs_threshold_3, the rolling shutter gyro interval
//   start/end times are adjusted by rs_offset_4.
// effect: Rolling shutter correction
//
// variable name: s3d_threshold_1
// Brief description of the parameter: The exposure threshold whereby exposure
//   times above and below this threshold results in a different offset getting
//   applied to the 3D shake gyro time interval.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 0.010
// data range: 0.00 to 0.050 (sec)
// constraints: 0.00 to 0.050
//   For exposure times less than s3d_threshold_1, the 3D shake gyro interval
//   start/end times are adjusted by s3d_offset_1.
// effect: 3D shake correction
//
// variable name: s3d_threshold_2
// Brief description of the parameter: The exposure threshold whereby exposure
//   times above and below this threshold results in a different offset getting
//   applied to the 3D shake gyro time interval.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 0.020
// data range: 0.00 to 0.050 (sec)
// constraints: 0.00 to 0.050
//   For exposure times less than s3d_threshold_2, the 3D shake gyro interval
//   start/end times are adjusted by s3d_offset_2.
// effect: 3D shake correction
//
// variable name: s3d_threshold_3
// Brief description of the parameter: The exposure threshold whereby exposure
//   times above and below this threshold results in a different offset getting
//   applied to the rolling shutter gyro time interval.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 0.028
// data range: 0.00 to 0.050 (sec)
// constraints: 0.00 to 0.050
//   For exposure times less than s3d_threshold_3, the 3D shake gyro interval
//   start/end times are adjusted by s3d_offset_3.  For exposure times greater
//   than or equal to s3d_threshold_3, the 3D shake gyro interval start/end
//   times are adjusted by s3d_offset_4.
// effect: 3D shake correction
//
// variable name: rs_time_interval_1
// Brief description of the parameter: Width of the rolling shutter gyro time
//   interval.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 33
// data range: 0 to 33 (msec)
// constraints: 0 to 33
//   For exposure times less than rs_threshold_1, the rolling shutter gyro
//   time interval is rs_time_interval_1.
// effect: Rolling shutter correction
//
// variable name: rs_time_interval_2
// Brief description of the parameter: Width of the rolling shutter gyro time
//   interval.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 33
// data range: 0 to 33 (msec)
// constraints: 0 to 33
//   For exposure times less than rs_threshold_2, the rolling shutter gyro
//   time interval is rs_time_interval_2.
// effect: Rolling shutter correction
//
// variable name: rs_time_interval_3
// Brief description of the parameter: Width of the rolling shutter gyro time
//   interval.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 33
// data range: 0 to 33 (msec)
// constraints: 0 to 33
//   For exposure times less than rs_threshold_3, the rolling shutter gyro
//   time interval is rs_time_interval_3.
// effect: Rolling shutter correction
//
// variable name: rs_time_interval_4
// Brief description of the parameter: Width of the rolling shutter gyro time
//   interval.
// applicable chipset(s): Targets with gyro sensor
// applicable operation mode:  Camcorder
// default value: 33
// data range: 0 to 33 (msec)
// constraints: 0 to 33
//   For exposure times greater than or equal to rs_threshold_3, the rolling
//   shutter gyro time interval is rs_time_interval_4.
// effect: Rolling shutter correction
//=============================================================================
typedef struct
{
    int             enable;
    unsigned int    focal_length;
    unsigned int    gyro_pixel_scale;
    float           virtual_margin;
    float           gyro_noise_floor;
    unsigned int    gyro_frequency;
    int             rs_offset_1;
    int             rs_offset_2;
    int             rs_offset_3;
    int             rs_offset_4;
    int             s3d_offset_1;
    int             s3d_offset_2;
    int             s3d_offset_3;
    int             s3d_offset_4;
    float           rs_threshold_1;
    float           rs_threshold_2;
    float           rs_threshold_3;
    float           s3d_threshold_1;
    float           s3d_threshold_2;
    float           s3d_threshold_3;
    unsigned int    rs_time_interval_1;
    unsigned int    rs_time_interval_2;
    unsigned int    rs_time_interval_3;
    unsigned int    rs_time_interval_4;
    unsigned long   reserve_1;
    unsigned long   reserve_2;
    unsigned long   reserve_3;
    unsigned long   reserve_4;
} Chromatix_EIS_algo_type;

typedef struct
{
    //=========================================================================
    // Chromatix header version info (MUST BE THE FIRST PARAMETER)
    //=========================================================================
    ChromatixVersionInfoType                    chromatix_version_info;

    AAA_version_type                            aaa_version;

    //=========================================================================
    // AWB
    //=========================================================================
    awb_algo_struct_type                        AWB_bayer_algo_data; //this contains Bayer algorithm related params

    //=========================================================================
    // AEC
    //=========================================================================
    AEC_algo_struct_type                        AEC_algo_data;

    //=========================================================================
    // AFD
    //=========================================================================
    chromatix_auto_flicker_detection_data_type  auto_flicker_detection_data;   // DetectionData

    //=========================================================================
    // ASD
    //=========================================================================
    AAA_ASD_struct_type                         ASD_3A_algo_data;

    //=========================================================================
    // AF
    //=========================================================================
    af_algo_tune_parms_t                        AF_algo_data;

    //=========================================================================
    // AAA Sync (Dual Sensor)
    //=========================================================================
    chromatix_aaa_sync_type                     aaa_sync_data;

    //=========================================================================
    // EIS
    //=========================================================================
    Chromatix_EIS_algo_type                     chromatix_EIS_data;
} chromatix_3a_parms_type;

#endif
