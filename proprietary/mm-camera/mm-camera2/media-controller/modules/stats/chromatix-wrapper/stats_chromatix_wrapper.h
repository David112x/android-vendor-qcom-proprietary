/* stats_chromatix_wrapper .h
 *
 * Copyright (c)2016-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __STATS_CHROMATIX_WRAPPER_H__
#define __STATS_CHROMATIX_WRAPPER_H__


/*TODO: read chromatix version from the make file and avoid including this file*/
#include "chromatix.h"
#include "chromatix_3a.h"
#include "chromatix_iot.h"
#include "q3a_platform.h"

/******************************************************************************
*******************************************************************************
**  AF related params start
*******************************************************************************
******************************************************************************/
#define MAX_AF_KERNEL_NUM               2
#define MAX_HPF_2x5_BUFF_SIZE           10
#define MAX_HPF_2x11_BUFF_SIZE          22
#define FILTER_SW_LENGTH_FIR            11
#define FILTER_SW_LENGTH_IIR            6
#define MAX_ACT_MOD_NAME_SIZE           32
#define MAX_ACT_NAME_SIZE               32

#define AF_PD_MAX_NUM_ROI               10
#define AF_PD_MAX_TABLE_ENTRIES         20
#define AF_PD_MAX_DEFOCUS_RANGE         4
#define AF_TOF_MAX_DISTANCE_ENTRIES     8
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
#define MAX_HPF_BUFF_SIZE               22
#define AF_BV_COUNT                     8
#define DEFAULT_MAX_SCAN_CNT_TH         60
#define AF_GRID_SIZE_MAX                281
#define AF_LUMA_TABLE_SIZE              20


/******************************************************************************
*******************************************************************************
**  Face AWB related params for 309 header hardcode
*******************************************************************************
******************************************************************************/
/*FaceAWB gain adjust ref.point number*/
#define FACE_AWB_GAIN_ADJUST_MAX_NUMBER 7
/*FaceAWB ref.point number*/
#define FACE_AWB_REF_POINT_NUMBER 15

#define AWB_MAX_NUMBER_OF_CUSTOMER_REFERENCE_POINT 20


//FaceAWB ROI related parameters:rarely tune
#define X_ROI_DIVIDE 0.33f
#define Y_ROI_DIVIDE 0.375f
//FaceAWB stats screening related parameters:rarely tune
#define DARK_SKIN_CCT_HIGH_THD 4500.0f
#define DARK_SKIN_CCT_LOW_THD 2800.0f
#define LIGHT_SKIN_CCT_HIGH_THD 4500.0f
#define LIGHT_SKIN_CCT_LOW_THD 2800.0f

#define LIGHT_SKIN_CCT_CONFIDENCE_RATIO_OF_HIGH_CCT_HIGH_LUX_INDEX 0.8f
#define LIGHT_SKIN_CCT_CONFIDENCE_RATIO_OF_HIGH_CCT_LOW_LUX_INDEX 0.95f
#define LIGHT_SKIN_CCT_CONFIDENCE_RATIO_OF_LOW_CCT_HIGH_LUX_INDEX 0.95f
#define LIGHT_SKIN_CCT_CONFIDENCE_RATIO_OF_LOW_CCT_LOW_LUX_INDEX 0.5f

#define DARK_SKIN_CCT_CONFIDENCE_RATIO_OF_HIGH_CCT_HIGH_LUX_INDEX 0.3f
#define DARK_SKIN_CCT_CONFIDENCE_RATIO_OF_HIGH_CCT_LOW_LUX_INDEX 0.95f
#define DARK_SKIN_CCT_CONFIDENCE_RATIO_OF_LOW_CCT_HIGH_LUX_INDEX 0.95f
#define DARK_SKIN_CCT_CONFIDENCE_RATIO_OF_LOW_CCT_LOW_LUX_INDEX 0.3f

#define MAX_WEIGHT 1.0f

#define GAIN_ADJ_BRIGHT_LIGHT_START 190.0f;
#define GAIN_ADJ_BRIGHT_LIGHT_END   230.0f;
#define GAIN_ADJ_LOWLIGHT_START     330.0f;
#define GAIN_ADJ_LOWLIGHT_END       360.0f;

typedef enum _af_kernel_adapter_type
{
    Q3A_AF_KERNEL_PRIMARY = 0,
    Q3A_AF_KERNEL_SCALE,
} af_kernel_adapter_type;

/******************************************************************************
** af_algo_adapter_type: Type of algorithm currently supported
******************************************************************************/
typedef enum _af_algo_adapter_t
{
    Q3A_AF_PROCESS_DEFAULT = -2,
    Q3A_AF_PROCESS_UNCHANGED = -1,
    Q3A_AF_EXHAUSTIVE_SEARCH = 0,
    Q3A_AF_EXHAUSTIVE_FAST,
    Q3A_AF_HILL_CLIMBING_CONSERVATIVE,
    Q3A_AF_HILL_CLIMBING_DEFAULT,
    Q3A_AF_HILL_CLIMBING_AGGRESSIVE,
    Q3A_AF_FULL_SWEEP,
    Q3A_AF_SLOPE_PREDICTIVE_SEARCH,
    Q3A_AF_CONTINUOUS_SEARCH,
    Q3A_AF_PROCESS_MAX
} af_algo_adapter_t;

/******************************************************************************
** af_single_index_adapter_t: Enum for indexing mapping for distance to
** lens position
******************************************************************************/
typedef enum _af_single_index_adapter_t
{
    Q3A_AF_SINGLE_NEAR_LIMIT_IDX = 0,
    Q3A_AF_SINGLE_7CM_IDX = 1,
    Q3A_AF_SINGLE_10CM_IDX = 2,
    Q3A_AF_SINGLE_14CM_IDX = 3,
    Q3A_AF_SINGLE_20CM_IDX = 4,
    Q3A_AF_SINGLE_30CM_IDX = 5,
    Q3A_AF_SINGLE_40CM_IDX = 6,
    Q3A_AF_SINGLE_50CM_IDX = 7,
    Q3A_AF_SINGLE_60CM_IDX = 8,
    Q3A_AF_SINGLE_120CM_IDX = 9,
    Q3A_AF_SINGLE_HYP_F_IDX = 10,
    Q3A_AF_SINGLE_INF_LIMIT_IDX = 11,
    Q3A_AF_SINGLE_MAX_IDX = 12,
} af_single_index_adapter_t;

/**
* af_cam_name_adapter: Enum for camera profile
**/
typedef enum _af_cam_name_adapter
{
    Q3A_AF_ACT_MAIN_CAM_0,
    Q3A_AF_ACT_MAIN_CAM_1,
    Q3A_AF_ACT_MAIN_CAM_2,
    Q3A_AF_ACT_MAIN_CAM_3,
    Q3A_AF_ACT_MAIN_CAM_4,
    Q3A_AF_ACT_MAIN_CAM_5,
    Q3A_AF_ACT_WEB_CAM_0,
    Q3A_AF_ACT_WEB_CAM_1,
    Q3A_AF_ACT_WEB_CAM_2,
} af_cam_name_adapter;

/**
* af_baf_tuning_preset_adapter_t : enum for preset_id
*
**/
typedef enum _af_baf_tuning_preset_adapter_t
{
    Q3A_BAF_TUNING_CUSTOM = -1,
    Q3A_BAF_TUNING_PRESET_0,
    Q3A_BAF_TUNING_PRESET_1,
    Q3A_BAF_TUNING_PRESET_2,
    Q3A_BAF_TUNING_PRESET_3,
    Q3A_BAF_TUNING_PRESET_4,
    Q3A_BAF_TUNING_PRESET_5,
    Q3A_BAF_TUNING_PRESET_MAX
} af_baf_tuning_preset_adapter_t;

/**
* af_baf_roi_pattern_adapter_t : enum for ROI pattern type
*
**/
typedef enum _af_baf_roi_pattern_adapter_t
{
    Q3A_BAF_ROI_PATTERN_CUSTOM = -1,
    Q3A_BAF_ROI_PATTERN_RECT = 0,
    Q3A_BAF_ROI_PATTERN_CROSS,
    Q3A_BAF_ROI_PATTERN_DIAMOND,
    Q3A_BAF_ROI_PATTERN_SPARSE,
    Q3A_BAF_ROI_PATTERN_MAX
} af_baf_roi_pattern_adapter_t;

/**
* _af_scene_type_enum_t : enum for AF scene type
*
**/
typedef enum _af_scene_type_adapter_t
{
    Q3A_AF_SCENE_TYPE_NORMAL = 0,
    Q3A_AF_SCENE_TYPE_LOWLIGHT = 1,
    Q3A_AF_SCENE_TYPE_FACE = 2,
    Q3A_AF_SCENE_TYPE_MAX,
} af_scene_type_adapter_t;

/**
* af_tuning_fdac_adapter_t: Enum for indexing mapping for collected DAC for
*   face forward/up/down
**/
typedef enum _af_tuning_fdac_adapter_t
{
    Q3A_AF_TUNING_DAC_FACE_FORWARD = 0,
    Q3A_AF_TUNING_DAC_FACE_UP,
    Q3A_AF_TUNING_DAC_FACE_DOWN,
    Q3A_AF_TUNING_DAC_MAX
} af_tuning_fdac_adapter_t;

/**
* af_actuator_adapter_t: Enum for Type of actuator, which impacts core algo behaviors
**/
typedef enum _af_actuator_adapter_t
{
    Q3A_AF_ACT_TYPE_CLOSELOOP = 0,
    Q3A_AF_ACT_TYPE_OPENLOOP,
    Q3A_AF_ACT_TYPE_MAX,
} af_actuator_adapter_t;


/**
 * af_fv_stats_adapter_t: Enum for Type of Stats
 **/
typedef enum _af_fv_stats_adapter_t
{
    Q3A_AF_FV_STATS_TYPE_HW_1 = 0,
    Q3A_AF_FV_STATS_TYPE_HW_2 = 1,
    Q3A_AF_FV_STATS_TYPE_SW   = 2,
    Q3A_AF_FV_STATS_TYPE_MAX  = 3,
} af_fv_stats_adapter_t;

/**
 * af_haf_algo_adapter_t: Enum for HAF algorithm types
 *
 *    @Q3A_AF_HAF_ALGO_TOF: TOF algo
 *    @Q3A_AF_HAF_ALGO_PDAF: PDAF algo
 *    @Q3A_AF_HAF_ALGO_DCIAF: DCIAF algo
 *    @Q3A_AF_HAF_ALGO_DBG: Debug Algo
 **/
typedef enum
{
    Q3A_AF_HAF_ALGO_TOF = 0,
    Q3A_AF_HAF_ALGO_PDAF,
    Q3A_AF_HAF_ALGO_DCIAF,
    Q3A_AF_HAF_ALGO_DBG,
    Q3A_AF_HAF_ALGO_MAX,
} af_haf_algo_adapter_t;

/**
* af_cam_orientation_adapter_t: Direction of Aux wrt Main Camera
*
*    @Q3A_AF_CAM_ORIENT_LEFT: Towards left
*    @Q3A_AF_CAM_ORIENT_RIGHT: Towards right
*    @Q3A_AF_CAM_ORIENT_UP: Towards up
*    @Q3A_AF_CAM_ORIENT_DOWN: Towards down
**/
typedef enum _af_cam_orientation_adapter_t
{
    Q3A_AF_CAM_ORIENT_LEFT = 0,
    Q3A_AF_CAM_ORIENT_RIGHT,
    Q3A_AF_CAM_ORIENT_UP,
    Q3A_AF_CAM_ORIENT_DOWN,
    Q3A_AF_CAM_ORIENT_MAX,
} af_cam_orientation_adapter_t;

typedef struct _af_step_size_adapter_t
{
    unsigned short rgn_0; /* reserved */
    unsigned short rgn_1;
    unsigned short rgn_2;
    unsigned short rgn_3;
    unsigned short rgn_4; /* reserved */
} af_step_size_adapter_t;

typedef struct _af_step_size_table_adapter_t
{
    af_step_size_adapter_t  Prescan_normal_light;
    af_step_size_adapter_t  Prescan_low_light;
    af_step_size_adapter_t  Finescan_normal_light;
    af_step_size_adapter_t  Finescan_low_light;
} af_step_size_table_adapter_t;

typedef enum {
  AF_FOLLOWER_COARSE_FOLLOW =0,
  AF_FOLLOWER_COMPLETE_FOLLOW,
  AF_FOLLOWER_SEQUENTIAL_FOLLOW,
} af_follower_search_strategy;

typedef enum {
  AF_DISTANCE_MACRO_RANGE =0,
  AF_DISTANCE_MID_RANGE,
  AF_DISTANCE_FAR_RANGE,
  AF_DISTANCE_MAX_RANGE
} af_distance_range;

/*
* af_fine_search_step_info:
* Fine Search Step info for Tuning
*
* @stepSize : Step Size
* @far_steps : No of Steps in Far Direction
* @near_steps : No of Steps in Near Direction
*/
typedef struct _af_fine_search_step_info {
  int                                       stepSize;
  int                                       far_steps;
  int                                       near_steps;
} af_fine_search_step_info;

/*
* af_hjr_info:
* Hand jitter resistant AF params for Tuning
* @HJ_Algorithm_enable         : Algo enabled or not
* @maxfv_in_beginning          : max fv in the beginning
* @coarse_min_sample_size      : Minimum stats number for LSP curve fitting calculation during coarse search
* @fine_min_sample_size        : Minimum stats number for LSP curve fitting calculation during fine search
* @cur_gyro_HJ_thrs            : Minimum HJ threshold of average gyro square value during coarse search
* @cur_gyro_HJ_thrs_fine       : Minimum HJ threshold of average gyro square value during fine search
* @cur_gyro_STD_HJ_thrs        : Minimum HJ threshold of the STD of gyro square value during coarse search
* @cur_gyro_STD_HJ_thrs_fine   : Minimum HJ threshold of the STD of gyro square value during fine search
* @gyro_thrs_for_diff_ratio    : Minimum HJ threshold of gyro square value need to calculate gyro ratio during coarse search
* @gyro_thrs_for_diff_ratio_fine   : Minimum HJ threshold of gyro square value need to calculate gyro ratio during fine search
* @gyro_diff_ratio_HJ_thrs         : Minimum HJ threshold of gyro difference ratio during coarse search
* @gyro_diff_ratio_HJ_thrs_fine	   : Minimum HJ threshold of gyro difference ratio during fine search
* @HJ_ReDo_rate_thrs               : Percentage ratio to retrigger AF search (HJ stats number/ All stats number)
* @HJ_handling_need_cnt            : Minimum HJ stats count threshold to trigger HJ algorithm
* @HJ_reset_Data_Before_fine       : Turn on Stats reset function before enter fine search
* @HJ_high_gyro_diff_thrs          : Highest threshold of gyro difference of HJ filter process
* @HJ_low_gyro_diff_thrs           : Lowest threshold of gyro difference of HJ filter process
* @HJ_high_gyro_Fv_weight          : Focus value weight when gyro difference is above AF_LSP_HJ_High_Gyro_Diff_Thrs
* @HJ_normalized_Fv_enable         : Turn on HJ filter process during fine search
* @HJ_algortihm_working_exp_idx    : Minimum exposure index requirement to enter HJ algorithm
* @sampling_rate:                  : Gyro sensor sampling rate needed for the algo
* @reporting_rate:                 : Gyro sensor reporting rate needed for the algo
*/
typedef struct _af_hjr_info {
  boolean   HJ_Algorithm_enable;
  float     maxfv_in_beginning;
  int       coarse_min_sample_size;
  int       fine_min_sample_size;
  float     cur_gyro_HJ_thrs;
  float     cur_gyro_HJ_thrs_fine;
  float     cur_gyro_STD_HJ_Thrs;
  float     cur_gyro_STD_HJ_thrs_fine;
  float     gyro_thrs_for_diff_ratio;
  float     gyro_thrs_for_diff_ratio_fine;
  float     gyro_diff_ratio_HJ_thrs;
  float     gyro_diff_ratio_HJ_thrs_fine;
  float     HJ_ReDo_rate_thrs;
  int       HJ_handling_need_cnt;
  int       HJ_reset_Data_Before_fine;
  float     HJ_high_gyro_diff_thrs;
  float     HJ_low_gyro_diff_thrs;
  float     HJ_high_gyro_Fv_weight;
  int       HJ_normalized_Fv_enable;
  int       HJ_normalized_weight_type;
  int       HJ_algortihm_working_exp_idx;
  int       sampling_rate;
  int       reporting_rate;
} af_hjr_info;

//TODO: Merge these 2 structures. have af_hj_info and create 2 objects of this structure
/*
* af_caf_hjr_info:
* hand jitter resistant AF tuning params fine seach
* @caf_HJ_Algorithm_enable      : Algo enabled or not
* @caf_min_sample_size          : Minimum stats number for LSP curve fitting calculation during HAF fine search
* @caf_cur_gyro_HJ_thrs         : Minimum HJ threshold of average gyro square value during HAF fine search
* @caf_cur_gyro_STD_HJ_Thrs     : Minimum HJ threshold of the STD of gyro square value during HAF fine search
* @caf_gyro_thrs_for_diff_ratio : Minimum HJ threshold of gyro square value need to calculate gyro ratio during HAF search
* @caf_gyro_diff_ratio_HJ_thrs  : Minimum HJ threshold of gyro difference ratio during HAF fine search
* @caf_HJ_handling_need_cnt     : Minimum HJ stats count threshold to trigger HJ algorithm
* @caf_HJ_high_gyro_diff_thrs   : Highest threshold of gyro difference of HJ filter process
* @caf_HJ_low_gyro_diff_thrs    : Lowest threshold of gyro difference of HJ filter process
* @caf_HJ_high_gyro_Fv_weight   : Focus value weight when gyro difference is above HAF_LSP_HJ_High_Gyro_Diff_Thrs
* @caf_HJ_normalized_Fv_enable  : Turn on HJ filter process during fine search
* @caf_HJ_algortihm_working_exp_idx:Minimum exposure index requirement to enter HJ algorithm

*/
typedef struct _af_caf_hjr_info {
  boolean   caf_HJ_Algorithm_enable;
  int       caf_min_sample_size;
  float     caf_cur_gyro_HJ_thrs;
  float     caf_cur_gyro_STD_HJ_Thrs;
  float     caf_gyro_thrs_for_diff_ratio;
  float     caf_gyro_diff_ratio_HJ_thrs;
  int       caf_HJ_handling_need_cnt;
  float     caf_HJ_high_gyro_diff_thrs;
  float     caf_HJ_low_gyro_diff_thrs;
  float     caf_HJ_high_gyro_Fv_weight;
  int       caf_HJ_normalized_Fv_enable;
  int       caf_HJ_normalized_weight_type;
  int       caf_HJ_algortihm_working_exp_idx;
} af_caf_hjr_info;

/*
* af_fine_search_info:
* Fine Search  info for Tuning
*
* @normalLightFineSearchInfo : Normal Light fine search info
* @lowLightFineSearchInfo : Low Light Fine search info
*/
typedef struct _af_fine_search_info {
  af_fine_search_step_info           normalLightFineSearchInfo[AF_DISTANCE_MAX_RANGE];
  af_fine_search_step_info           lowLightFineSearchInfo[AF_DISTANCE_MAX_RANGE];
} af_fine_search_info;


typedef enum{
  AF_ROI_SHIFT_DISABLED = 0,
  AF_ROI_SHIFT_BY_DISTANCE_DATA,
  AF_ROI_SHIFT_BY_DISTANCE_CALIB_DATA,
  AF_ROI_SHIFT_MAX,
}af_roi_shift_type;

typedef enum{
  AF_DISTANCE_ENTRY_ZERO = 0,
  AF_DISTANCE_ENTRY_ONE,
  AF_DISTANCE_ENTRY_TWO,
  AF_DISTANCE_ENTRY_THREE,
  AF_DISTANCE_ENTRY_FOUR,
  AF_DISTANCE_ENTRY_MAX,
} af_distance_entry_enum;

/**
* _af_stats_use_type: type of stats to use in algorithm.
**/
typedef enum _af_stats_use_type {
  AF_STATS_HV = 0,
  AF_STATS_HV_NORM,
  AF_STATS_H1,
  AF_STATS_H1_NORM,
  AF_STATS_H2,
  AF_STATS_H2_NORM,
  AF_STATS_V,
  AF_STATS_V_NORM,
  AF_STATS_SCALE,
  AF_STATS_SCALE_NORM,
  AF_STATS_MAX,
} af_stats_use_type;

typedef struct _af_tuning_tof_extended_adapter_t
{
  /*
   * reserve[0]->resv_enabled
   * Default Value: 1/true
   * Range: [0 – 1]
   * Description: A flag that indicate some tof reserve variables have proper values and can be used.*/
  float     resv_enabled;

  /*
   * reserve[6]  ->frames_to_wait_stable
   * Default Value: 3
   * Range: [1 – 10]
   * Description: Number of frames SAD need to be stable. If the number of stable frames are less
   * than this threshold, panning will continue.*/
  float     frames_to_wait_stable;

  /*
   * reserve[42]  ->tof_delay_flag_enable
   * Default Value: 1/true
   * Range: [0 – 1]
   * Description: A flag that indicate if some new tof reserve parameter are valid.*/
  float     tof_delay_flag_enable;
  /*
   * reserve[1]   ->tof_min_distance_range
   * Default Value: 40
   * Description: Value to be used for calculating sensitivity*/
  float     tof_min_distance_range;

} af_tuning_tof_extended_adapter_t;

typedef struct _af_tuning_pd_conf_tbl_entry_adapter_t
{
    float                                 noise_gain;
    int                                   min_conf;
} af_tuning_pd_conf_tbl_entry_adapter_t;

typedef struct _af_tuning_pd_conf_tbl_adapter_t
{
    int                                   num_entries;
    af_tuning_pd_conf_tbl_entry_adapter_t entries[AF_PD_MAX_TABLE_ENTRIES];
} af_tuning_pd_conf_tbl_adapter_t;

typedef struct _af_tuning_pd_dof_tbl_entry_adapter_t
{
    float                                 norm_lens_pos;
    int                                   dof;
} af_tuning_pd_dof_tbl_entry_adapter_t;

typedef struct _af_tuning_pd_dof_tbl_adapter_t
{
    int                                   num_entries;
    af_tuning_pd_dof_tbl_entry_adapter_t entries[AF_PD_MAX_TABLE_ENTRIES];
} af_tuning_pd_dof_tbl_adapter_t;

typedef struct _af_tuning_pdaf_extended_adapter_t
{
  /*
   * reserve[0] -> defocus_stable_filter_len
   * Default Value: 3
   * Range: [0, 10]
   * Description: Number of successive PD samples in which max/min is calculated
   * for assessing stability condition
   * Constraints: The larger, longer it takes to meet stability criteria and for
   * PDAF to trigger new search.*/
  float           defocus_stable_filter_len;

  /*
   * reserve[1] -> enable_trigger_suppr_by_scene_change
   * Default Value: 0
   * Range: [0, 1]
   * Description: When enabled,  PDAF focus search is triggered only when a large
   * defocus is also accompanied by scene change (i.e. gyro, focus value, luma change)
   * Constraints: Generally disabled for PDAF Type 1 and enabled for PDAF Type 2 and 3.*/
  boolean         enable_trigger_suppr_by_scene_change;

  /*
   * reserve[2] -> enable_fine_scan_extension
   * Default Value: 0
   * Range: [0, 1]
   * Description: When enabled, fine scan searches for focus peak beyond the
   * estimated range based on PD noise model
   * Constraints: Generally disabled for PDAF Type 1 and enabled for PDAF Type
   * 2 and 3.*/
  boolean         enable_fine_scan_extension;

  /*
   * reserve[3] -> enable_do_not_use_defocus_sample_by_pd
   * Default Value: 0
   * Range: [0, 1]
   * Description: When enabled, defocus sample is discarded if the reported
   * defocus exceeds pd_max_limit_do_not_use_defocus_sample
   * Constraints: Generally disabled for PDAF Type 1 and enabled for PDAF Type
   * 2 and 3.*/
  boolean         enable_do_not_use_defocus_sample_by_pd;

  /*
   * reserve[4] -> enable_inc_min_conf_thres_by_pd
   * Default Value: 0
   * Range: [0, 1]
   * Description: When enabled, minimum confidence level is increased by a linear
   * scaling factor, inc_min_conf_thres_by_pd_scaler, if defocus exceeds
   * pd_max_limit_inc_min_conf_thres
   * Constraints: Generally disabled for PDAF Type 1 and enabled for PDAF Type
   * 2 and 3.*/
  boolean         enable_inc_min_conf_thres_by_pd;

  /*
   * reserve[5] -> pd_max_limit_do_not_use_defocus_sample
   * Default Value: 616
   * Range: [0, 1000]
   * Description: Represents out-of-bound limit of absolute value of  PD when
   * enable_do_not_use_defcous_sample_by_pd is enabled
   * Constraints: Specifies the maximum limit of PD above which PD sample is
   * discarded by PDAF.*/
  float           pd_max_limit_do_not_use_defocus_sample;

  /*
   * reserve[6] -> pd_max_limit_inc_min_conf_thres
   * Default Value: 422
   * Range: [0, 1000]
   * Description: Represents out-of-bound limit of absolute value of  PD which
   * triggers increase in minimum confidence level when
   * enable_inc_min_conf_thres_by_pd is enabled
   * Constraints: Specifies the maximum limit of PD above which  a higher
   * confidence level requirement is imposed.*/
  float           pd_max_limit_inc_min_conf_thres;


  /*
   * reserve[7] -> inc_min_conf_thres_by_pd_scaler
   * Default Value: 1.5
   * Range: [0, 3]
   * Description: Represents the scaler by which the minimum confidence level
   * is increased
   * Constraints: Specifies the scaler used in increasing the minimum confidence level.*/
  float           inc_min_conf_thres_by_pd_scaler;


  /*
   * reserve[8] -> defocus_dof_multiplier
   * Default Value: 1.2
   * Range: [1, 2]
   * Description: Represents the scaler by which the increase in lens
   * depth-of-focus change from the far to the near focus limit is estimated
   * Constraints: Specifies a linear scaler used in modeling the change in lens
   * depth-of-focus from the far to the near focus limit.*/
  float           defocus_dof_multiplier;

  /*
   * reserve[9] -> first_run_not_conf_cnt_add
   * Default Value: 0
   * Range: [0, 5]
   * Description: Represents additional number of frame count PDAF is allowed
   * to wait for confident sample of PD at camera start-up
   * Constraints: Increasing this number allows PDAF to wait longer for a
   * confident PD sample at camera start-up (e.g. due to a delay in PD lib
   * initialization). Setting this number too high can cause a lengthy delay
   * in the transition between PDAF to  contrast-based AF at camera start-up.*/
  float           first_run_not_conf_cnt_add;

  /*
   * reserve[10] -> inc_defocus_thres_prev_error
   * Default Value: 1.5
   * Range: [1, 3]
   * Description: Represents the scaler by which defocus threshold is
   * de-sensitized if previous PDAF focus converge resulted in a failure (e.g. focus hunting)
   * Constraints: Set this value to ensure that PDAF does not oscillate between
   * focus convengence and monitor by requiring a larger minimum defocus requirement
   * for the transition out of monitor state. Generally disabled for PDAF Type 1
   * and enabled for Type 2 and 3.*/
  float           inc_defocus_thres_prev_error;

  /*
   * reserve[11] -> soft_rgn_thres_multiplier
   * Default Value: 0.6
   * Range: [0, 1]
   * Description: If defocus value is less than defocus threshold and
   * above soft defocus threshold, then scene change determines if PDAF focus
   * convergence is to be triggered
   * Constraints: Soft threshold defocus is the calculated by defocus_threshold(tuning)
   * multiplied by soft_rgn_thres_multiplier. Decreasing this number may cause PDAF
   * to be more sensitive to defocus condition that is accompanied by a scene change.*/
  float           soft_rgn_thres_multiplier;

  /*
   * reserve[12] -> fine_scan_range_enhance
   * Default Value: 0
   * Range: [0, 1]
   * Description: When enabled this feature decreases the estimated fine scan
   * range by 25%
   * Constraints: Enable this feature to tighten PDAF fine scan range by 25%
   * for PDAF sensors with lower PD noise.*/
  boolean         fine_scan_range_enhance;

  /*
   * reserve[13] -> enable_soft_conf_thresh
   * Default Value: 0
   * Range: [0, 1]
   * Description: Enables feature which allow the minimum confidence level to
   * slightly decrease during focus convergence due to increased aliasing
   * Constraints: Enable this feature if PDAF frequently transitions to
   * contrast-based AF in the midst of focus convergence. Generally disabled
   * for Type 1 and enabled for Type 2 and 3.*/
  boolean         enable_soft_conf_thresh;


  /*
   * reserve[14] -> soft_conf_thresh_delta
   * Default Value: 100
   * Range: [0, 200]
   * Description: Represents the decrease in minimum confidence level allowed
   * during focus convergence if enable_soft_conf_thresh is enabled
   * Constraints: Increasing this number allows PDAF to employ lower
   * minimum confidence level during focus converge. Setting this number
   * higher than the minimum confidence level causes minimum confidence level
   * requirement to be omitted during focus converge; hence, a care needs to
   * be taken in revising this parameter.*/
  int             soft_conf_thresh_delta;

  /*
   * reserve[15] -> soft_conf_focus_pnt
   * Default Value: 0.2
   * Range: [0, 1]
   * Description: Represents the scaler by which PDAF focus convergence slows
   * down for PD samples whose confidence level resides between minimum confidence
   * level and (minimum confidence level - soft_conf_thresh_delta)
   * Constraints: Decreasing this number causes PDAF to perform focus convergence
   * more slowly for PD samples whose confidence level lies between minimum
   * confidence level and  (minimum confidence level - soft_conf_thresh_delta).*/
  float           soft_conf_focus_pnt;


  /*
   * reserve[16] -> enable_soft_rgn_thres_high
   * Default Value: 1
   * Range: [0, 1]
   * Description: It is a boolean flag which indicate if soft_defocus_thres_high
   * will be used as a parameter to trigger focus in soft region when soft region
   * threshold hig(enable_soft_rgn_thres_high) is enabled.*/
  boolean         enable_soft_rgn_thres_high;

  /*
   * reserve[17] -> soft_rgn_thres_high_multiplier
   * Default Value: 4
   * Range: [2 - 6]
   * Description: Defocus multiplier to make defocus threshold higher.
   * Please refer to enable_soft_rgn_thres_high image.*/
  float           soft_rgn_thres_high_multiplier;

  /*
   * reserve[18] -> soft_scene_change_cnt
   * Default Value: 1
   * Range: [1 - 6]
   * Description: number of scene changed reported by AF continues.*/
  float           soft_scene_change_cnt;

  /*
   * reserve[21] -> PD_early_interrupt_enabled
   * Default Value: 0
   * Range: [0 - 1]
   * Description: if early interrupt is supported/enabled and
   * PDLib is in sensor side, this flag will be TRUE*/
  float           PD_early_interrupt_enabled;
   /*
   * reserve[22] -> sprs_pd_tf_conf_thr
   * Default Value: 400
   * Range: [0 - 1024]
   * Description: Confidence threshold required to determine if to
     fallback or run PDAF while touch/face.*/
  float           sprs_pd_tf_conf_thr;

  /*
   * reserve[23] -> sprs_pd_tf_low_defocus_conf_thr
   * Default Value: 500
   * Range: [0 - 1024]
   * Description: Confidence threshold required to determine if to
     fallback or fine scan while touch/face.*/
  float           sprs_pd_tf_low_defocus_conf_thr;

 /* Reserve[24] -> skip_search_if_scene_in_focus:
   Default value: 0
   Range: [0 or 1] 0 - Disable
                        1 - Enable
   Description: flag to enable PDAF skip refocus if scene is already in focus..
   Constraints: */
  boolean                                     skip_search_if_scene_in_focus;

 /*
  * reserve[30] -> is_2PD_enable
  * Default Value: 0
  * Range: [0 - 1]
  * Description: Whether the sensor is a 2PD sensor and 2PD is enabled.*/
 float           is_2PD_enable;

 /*
  * reserve[31] -> face_2pd_min_move_thresh
  * Default Value: 0
  * Range: [0 - 7]
  * Description: While running face tracking 2PD, if defocus is less than this threshold
         face 2PD will not move.*/
 float           face_2pd_min_move_thresh;

 /*
  * reserve[32] -> face_2pd_roi_move_converge_suppress_enable
  * Default Value: 0
  * Range: [0 - 1]
  * Description: Enable flag for face 2pd ROI move converge suppress .
  Example: If face roi is moving over threshold, suppress pd from converging to avoid
      jittery user experience. */
 float           face_2pd_roi_move_converge_suppress_enable;

 /*
  * reserve[33] -> face_2pd_roi_stable_counter_thresh
  * Default Value: 3
  * Range: [1 - 100]
  * Description: Stable counter threshold for face 2PD roi move converge suppress.
    Once face ROI is less than moving threshold for continuous frames over the counter
    PDAF will resume converging */
 float           face_2pd_roi_stable_counter_thresh;

 /*
  * reserve[34] -> face_2pd_roi_move_thresh_x
  * Default Value: 0.5f
  * Range: [0.1 - 1.0f]
  * Description: Face tracking 2PD, if ROI is moving over defined X threshold, then
      converge suppress will occur.*/
 float           face_2pd_roi_move_thresh_x;

 /*
  * reserve[35] -> face_2pd_roi_move_thresh_y
  * Default Value: 0.5f
  * Range: [0.1 - 1.0f]
  * Description: Face tracking 2PD, if ROI is moving over defined Y threshold, then
     converge suppress will occur.*/
 float           face_2pd_roi_move_thresh_y;

 /*
  * reserve[36] -> dpd_fallback_conf
  * Default Value: 10
  * Range: [1 - 1000f]
  * Description: The threshold value to decide do we use PD information when fallback.
  *              Two PD performance is good enough. When fallback condition is match,
  *              we still can trust PD information when conf is higher then this threshold.
  * ex: fallback conf threshold is 60. we still can trust PD near/far range if
  * conf is higher than 10, then we can set this value to 10. When fallback happened,
  * we pass this range to CAF and do contrast-base AF in PD near/far range.
  */
 float           dpd_fallback_conf;

 /*
  * reserve[37] -> touch_pd_off_center_thresh
  * Default Value: 0.33f
  * Range: [0 - 1.0f]
  * Description: Radius distance from touch ROI to center ROI in terms of dist divided
  *              by CAMIF radius. This value is a percentage of camif radius.
  *  Image center PD perforamnce is better than corner. When defocus is large
  *  enough, corner image PD may not accurate enough and hit over-shoot situation.
  *  So we use this threshold to divide image to two zone, center/corner zone.
  *  If corner zone defocus is too bigger, we move lens with half defocus value.
  *  And do residual PDAF process after half defocus move.
  */
 float           touch_pd_off_center_thresh;

 /*
  * reserve[38] -> conv_done_temporal_enable
  * Default Value: 1
  * Range: [0,1]
  * Description: flag to enable converge done temproal filter feature.
                 2PD didn't use fine search, so we need restrict condition for converge done.
                 We use temporal filter to recorder history target_pos and use averge target
                 position to check is AF focused.
  */
 float           conv_done_temporal_enable;

 /*
  * reserve[39] -> conv_done_temporal_thresh
  * Default Value: 5
  * Range: [1 - 900]
  * Description: Threshold to judge does focus converge done when enable converge done temporal
  * filter feature. We calculate average target position and check does difference
  *              between average target position and next position. If difference is small than
  *              this threshold, focus converage is done.*/
 float           conv_done_temporal_thresh;

 /*
  * reserve[40] -> conv_done_temporal_filter_len
  * Default Value: 10
  * Range: [1 - 5]
  * Description: how much history target position we are recorded for converge done temporal
  * filter feature.
 */
 float           conv_done_temporal_filter_len;

 /*
  * reserve[41] -> face_pd_temporal_enable
  * Default Value: 1
  * Range: [0, 1]
  * Description: 2 PD face always keep in converge status for tracking face AF.
  *              It has hunting issue if PD value has variation, usually happend in small face.
  *              This feature use a temporal filter to calculate average target position and
  *              use target positon to calculate average defocus and next position.
  */
 float          face_pd_temporal_enable;

 /*
 * reserve[42] -> face_pd_temporal_len
 * Default Value: 3
 * Range: [1 - 10]
 * Description: how much history data used for face pd temporal filter
 */
 float           face_pd_temporal_len;

 /*
 * reserve[43] -> face_pd_temporal_conf_thres_high
 * Default Value: 700
 * Range: [1 - 1024]
 * Description: Face treaking AF is always in converge status. We move less lens position
 *               if confidence is low. Thise threshold use to specifiy move lens with 100%
 *   defocus when PD confidence large this threshold. If confidence is lower
 *   than min_conf, we don't move lens. Confidence between high and min_conf,
 *   we move lens with interpaltion percentage.
 */
 float           face_pd_temporal_conf_thres_high;

 /*
  * reserve[44] ->
  * Default Value:
  * Range:
  * Description: */

 /*
 * reserve[45] -> face_pd_fallback_enable
 * Default Value: 1
 * Range: [0, 1]
 * Description: Face PDAF is always in converge status and no fallback by default.
 *              This flag is used to enable fallback in some specification condition.
 *              If face fallback condition is met and this flag is enable, we go to
 *              fallback flow. Currently, face enter fallback when roi size is too small.
 */
 float           face_pd_fallback_enable;

 /*
 * reserve[46] -> face_pd_fallback_roi_size
 * Default Value: 200
 * Range: [0 - 4032]
 * Description: Face PDAF enter fallback status if one of ROI dimension is small than this
 *              threshold.
 */
 float           face_pd_fallback_roi_size;

 /*
 * reserve[47] -> fast_enter_lsp_in_low_conf_and_hunting_enable
 * Default Value: 1
 * Range: [0, 1]
 * Description: PD variation is large in some test scene and cause hunting issue.
 *              Force this test scene enter to LSP when sign of PD is reverted in some times.
 *              LSP will use contrast information to find final position.
 */
 float           fast_enter_lsp_in_low_conf_and_hunting_enable;

 /*
 * reserve[48] -> fast_enter_lsp_in_low_conf_and_hunting_threshold
 * Default Value: 2
 * Range: [0 - 10]
 * Description: PDAF stats is forced to enter focus converge done when sign of PD reverted counter
 *              is over this threshold.
 */
 float           fast_enter_lsp_in_low_conf_and_hunting_threshold;

 /*
 * reserve[50 -60] -> conf_table_high
 * Default Value:
    conf_tbl_h.num_entries = 5;
   conf_tbl_h.entries[0].noise_gain = 0;
   conf_tbl_h.entries[0].min_conf = 180;
   conf_tbl_h.entries[1].noise_gain = 1;
   conf_tbl_h.entries[1].min_conf = 180;
   conf_tbl_h.entries[2].noise_gain = 12;
   conf_tbl_h.entries[2].min_conf = 200;
   conf_tbl_h.entries[3].noise_gain = 24;
   conf_tbl_h.entries[3].min_conf = 200;
   conf_tbl_h.entries[4].noise_gain = 36;
   conf_tbl_h.entries[4].min_conf = 250;
 * num_entries
 * Range: [0 - 5]
 * noise_gain
 * Range: [0 - 40]
 * high_conf
 * Range: [0 - 1000]
 * Description: When the conf is over high_conf,  PDAF algorithm will skip fine search and all rely on PD's info to reach AF target.
                Otherwise, as the conf is btw high_conf and min_conf,  PDAF algorithm will use fine search to get AF target.
 */
 af_tuning_pd_conf_tbl_adapter_t        conf_table_high;

 /*
 * reserve[61] -> face_change_refocus_trigger_enable
 * Default Value: 0
 * Range: [0 - 1]
 * Description: Enable face PDAF face change refocus trigger. When face change criteria is satisfied
 *  then scene change flag will be raised and PDAF refocus will be triggered.
 */
 boolean           face_change_refocus_trigger_enable;

 /*
 * reserve[62] -> face_roi_change_thresh_percentage
 * Default Value: 0.1f
 * Range: [0.01 - 1.0f]
 * Description: When face ROI vs. reference ROI has moved more than thresh percentage, face
 *   refocus will trigger. Percentage is calculated by face ROI move delta divide by CAMIF width or height.
 */
 float           face_roi_change_thresh_percentage;

 /*
 * reserve[63] -> face_roi_size_stable_thresh
 * Default Value: 10
 * Range: [1 - 100]
 * Description: When current face ROI vs. last frame face ROI moves less than this pixel threshold, we
 *   report face regains stability. This is not a percentage.
 */
 float           face_roi_size_stable_thresh;

/*
 * reserve[76] -> led_pdaf_max_frame_allow_enable
 * Default Value: 0
 * Range: [0 or 1]
 * Description: LED PDAF convergence max frame control enable flag. When this flag is enabled,
     it will read LED_PDAF_max_frame_cnt and force LED PDAF to complete converge when converge
     frame is more than the max allowed. This prevents power drain and app timeout in case of constant
     scene panning.
 */
 boolean       led_pdaf_max_frame_allow_enable;

 /*
 * reserve[77] -> led_pdaf_max_frame_cnt
 * Default Value: 50
 * Range: [0 - 200]
 * Description: PDAF stats is forced to enter focus converge done when LED pd converge
 *              is over this threshold.
 */
 int           led_pdaf_max_frame_cnt;

 /*
 * reserve[78] -> mw_active_zoom_threshold
 * Default Value: 4.0f
 * Range: [0 ~ 8]
 * Description: Multi-window PDAF will only enable if zoom level is less than this threshold.
  If zoom level level higher than this threshold, then PDAF will be configured to 1x1 to ensure
  enough pixels is located inside.
 */
 float       mw_active_zoom_threshold;

 /*
 * reserve[79] -> mw_active_zoom_control_enable
 * Default Value: 0
 * Range: [0 or 1]
 * Description: Multi-window PDAF will only enable if zoom level is less than mw_active_zoom_threshold.
  If zoom level is higher than this threshold, then PDAF will be configured to 1x1 ROI instead to ensure
  enough pixels located inside. This is flag to enable/disable the mechanism. If this is disabled,
  then all zoom level will run multi-window PDAF, which may have hunting effect at high zoom level.
 */
 boolean       mw_active_zoom_control_enable;

 /*
 * reserve[80] -> max_caf_stable_wait_cnt
 * Default Value: 50
 * Range: [0 ~ 100]
 * Description:when pd converge done and wait for caf stable for this max count, will force primary search done.
 */
 int        max_caf_stable_wait_cnt;

 /*
 * reserve[81] -> max_caf_stable_wait_cnt_enable
 * Default Value: 0
 * Range: [0/1]
 * Description: to enable caf stable wait time out mechanism, when pd converge done and wait for caf stable for
                    a max count, will force primary search done.
*/
 boolean    max_caf_stable_wait_cnt_enable;

 /*
  * reserve[83-97] -> dof_table
  * Default Value: 0
  * Range: [0/1]
  * Description: dof num_entries, dof_table.entries[0] - dof_table.entries[6]
 */

 af_tuning_pd_dof_tbl_adapter_t        dof_table;

 } af_tuning_pd_extended_adapter_t;


typedef struct _BV_threshold_adapter_t
{
    float thres[AF_BV_COUNT]; /* CUR_BV_INFO */ /* 0, 20, 50, 100, 400, 700, OUTDOOR_, Sky */
} BV_threshold_adapter_t;

typedef struct _single_threshold_adapter_t
{
    float                       change_ratio_inc;
    float                       change_ratio_dec;
    float                       flat_inc_thres;
    float                       flat_dec_thres;
    float                       macro_thres;
    float                       drop_thres;
    unsigned long int           hist_dec_dec_thres;
    unsigned long int           hist_inc_dec_thres;
    BV_threshold_adapter_t      dec_dec_3frame;
    BV_threshold_adapter_t      inc_dec_3frame;
    BV_threshold_adapter_t      dec_dec;
    BV_threshold_adapter_t      dec_dec_noise;
    BV_threshold_adapter_t      inc_dec;
    BV_threshold_adapter_t      inc_dec_noise;
    BV_threshold_adapter_t      flat_threshold;
} single_threshold_adapter_t;

typedef struct _FVscore_threshold_adapter_t
{
    int                     default_stats;
    BV_threshold_adapter_t  score_ratio_showdif;
    float                   strict_noise;
    float                   normal_noise;
    float                   light_noise;
    int                     strong_peak_thres;
    int                     strong_inc_thres;
    int                     strong_dec_thres;
} FVscore_threshold_adapter_t;

typedef struct _single_optic_adapter_t
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
} single_optic_adapter_t;

typedef struct _af_single_frame_delay_adapter_t
{
    unsigned short       base_frame_delay;
    unsigned short       coarse_frame_delay;
    unsigned short       fine_frame_delay;
} af_single_frame_delay_adapter_t;

typedef struct _af_tuning_single_adapter_t
{
    unsigned short                    index[Q3A_AF_SINGLE_MAX_IDX]; /* af_single_index_adapter_t */
    unsigned short                    actuator_type;                /* af_actuator_adapter_t */
    unsigned short                    is_hys_comp_needed;
    unsigned short                    step_index_per_um;
    af_step_size_table_adapter_t      TAF_step_table;
    af_step_size_table_adapter_t      CAF_step_table;
    unsigned short                    PAAF_enable;
    single_threshold_adapter_t        sw;
    single_threshold_adapter_t        hw;
    float                             BV_gain[AF_BV_COUNT];         /* CUR_BV_INFO */
    single_optic_adapter_t            optics;
    FVscore_threshold_adapter_t       score_thresh;
    af_single_frame_delay_adapter_t   frame_delay;
} af_tuning_single_adapter_t;

typedef struct _af_tuning_sp_adapter
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
} af_tuning_sp_adapter_t;

typedef struct _af_tuning_gyro_adapter
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
    unsigned short        num_of_samples;
} af_tuning_gyro_adapter_t;

typedef struct _af_tuning_lens_sag_comp_adapter
{
    unsigned char                         enable;
    unsigned int                          f_dac[Q3A_AF_TUNING_DAC_MAX];
    float                                 f_dist;
    float                                 trigger_threshold;
} af_tuning_lens_sag_comp_adapter_t;

typedef struct _af_tuning_pd_roi_config_adapter_t
{
    int                                   roi_loc_y;
    int                                   roi_loc_x;
    int                                   roi_num_rows;
    int                                   roi_num_cols;
} af_tuning_pd_roi_config_adapter_t;

typedef struct _af_tuning_pd_roi_adapter_t
{
    int                                   num_entries;
    af_tuning_pd_roi_config_adapter_t     config[AF_PD_MAX_NUM_ROI];
} af_tuning_pd_roi_adapter_t;

typedef struct _af_tuning_pd_noise_tbl_entry_adapter_t
{
    float                                 noise_gain;
    float                                 multiplier;
} af_tuning_pd_noise_tbl_entry_adapter_t;

typedef struct _af_tuning_pd_stable_tbl_entry_adapter_t
{
    int                                   fps;
    int                                   min_stable_cnt;
} af_tuning_pd_stable_tbl_entry_adapter_t;

typedef struct _af_tuning_pd_focus_tbl_entry_adapter_t
{
    int                                   defocus;
    float                                 move_pcnt;
} af_tuning_pd_focus_tbl_entry_adapter_t;

typedef struct _af_tuning_pd_noise_tbl_adapter_t
{
    int                                     num_entries;
    af_tuning_pd_noise_tbl_entry_adapter_t  entries[AF_PD_MAX_TABLE_ENTRIES];
} af_tuning_pd_noise_tbl_adapter_t;

typedef struct _af_tuning_pd_focus_tbl_adapter_t
{
    int                                     num_entries;
    af_tuning_pd_focus_tbl_entry_adapter_t  entries[AF_PD_MAX_TABLE_ENTRIES];
} af_tuning_pd_focus_tbl_adapter_t;

typedef struct _af_tuning_pd_stable_tbl_adapter_t
{
    int                                       num_entries;
    af_tuning_pd_stable_tbl_entry_adapter_t   entries[AF_PD_MAX_TABLE_ENTRIES];
} af_tuning_pd_stable_tbl_adapter_t;

typedef struct _af_tuning_pd_focus_scan_adapter_t
{
    int                                   focus_conv_frame_skip;
    int                                   enable_fine_scan;
    int                                   min_fine_scan_range;
    int                                   fine_scan_step_size;
    int                                   focus_done_threshold;
    int                                   enable_fine_scan_extension;
    int                                   first_run_not_conf_cnt_add;
} af_tuning_pd_focus_scan_adapter_t;

typedef struct _af_tuning_pd_monitor_adapter_t
{
    int                                   wait_after_focus_cnt;
    int                                   wait_conf_recover_cnt;
    float                                 defocused_threshold;
    float                                 depth_stable_threshold;
    int                                   defocus_stable_filter_len;
    int                                   enable_trigger_suppr_by_scene_change;
    float                                 defocus_dof_multiplier;
    float                                 inc_defocus_thres_scaler;
} af_tuning_pd_monitor_adapter_t;

typedef struct _af_tuning_pd_pdlib_adapter_t
{
    int                               enable;
    int                               max_shift;
    int                               resize_ratio;
    int                               confidence_scale;
    int                               smooth_filter[7];
    int                               sobel_filter[9];
    int                               defocus_confidence_th;
} af_tuning_pd_pdlib_adapter_t;

typedef struct _af_tuning_pd_preproc_adapter_t
{
    int                                   enable;
    boolean                               enable_do_not_use_defocus_sample_by_pd;
    boolean                               enable_inc_min_conf_thres_by_pd;
    float                                 pd_max_limit_do_not_use_defocus_sample;
    float                                 pd_max_limit_inc_min_conf_thres;
    float                                 inc_min_conf_thres_by_pd_scaler;
    float                                 pd_std_Threshold;
    int                                   pd_defocushi_err_sc_check;
    int                                   pd_avgdefocushi_err_sc_check;
    int                                   pd_lowlight_gain_thr_sc_check;
    int                                   pd_defocushi_sc_check[AF_PD_MAX_DEFOCUS_RANGE];
    int                                   pd_avgdefocushi_sc_check[AF_PD_MAX_DEFOCUS_RANGE];
    int                                   pd_lowlight_defocushi_sc_check[AF_PD_MAX_DEFOCUS_RANGE];
    int                                   pd_lowlight_avgdefocushi_sc_check[AF_PD_MAX_DEFOCUS_RANGE];
} af_tuning_pd_preproc_adapter_t;

typedef struct _af_tuning_pdaf_internal_adapter_t
{
    unsigned int                          type;
    unsigned int                          init_pos;
    unsigned int                          pdaf_enable_face_caf_priority;
    float                                 inc_defocus_thres_prev_error;
    boolean                               fine_scan_range_enhance;
} af_tuning_pd_internal_adapter_t;


typedef struct _af_tuning_pdaf_adapter_t
{
    af_tuning_pd_roi_adapter_t                      roi;
    af_tuning_pd_focus_tbl_adapter_t                focus_tbl;
    af_tuning_pd_noise_tbl_adapter_t                noise_tbl;
    af_tuning_pd_conf_tbl_adapter_t                 conf_tbl;
    af_tuning_pd_stable_tbl_adapter_t               stable_tbl;
    af_tuning_pd_focus_scan_adapter_t               focus_scan;
    af_tuning_pd_monitor_adapter_t                  scene_monitor;
    af_tuning_pd_pdlib_adapter_t                    pd_lib;
    af_tuning_pd_preproc_adapter_t                  pd_preproc;
    af_tuning_pd_internal_adapter_t                 pd_int;
    af_tuning_pd_extended_adapter_t                 pd_extended;
} af_tuning_pdaf_adapter_t;

typedef enum
{
    Q3A_AF_DCIAF_MACRO_RANGE = 0,
    Q3A_AF_DCIAF_MID_RANGE,
    Q3A_AF_DCIAF_FAR_RANGE,
    Q3A_AF_DCIAF_MAX_RANGE
} af_dciaf_distance_range_adapter;

typedef struct _af_dciaf_fine_search_step_adapter
{
    int                                   stepSize;
    int                                   far_steps;
    int                                   near_steps;
} af_dciaf_fine_search_step_adapter;

typedef struct _af_dciaf_extended
{

  /* reserve[0]  -> enable
   * Default Value: 1/true
   * Range: [0 – 1]
   * Description: if the following reserve parameter have valid values.*/
  boolean    enable;

  /* reserve[1]  ->maxLowLightWaitMs
   * Default Value: 250
   * Range: [0 – 2000]
   * Description: Convergence time out in Low light*/
  float      maxLowLightWaitMs;

  /* reserve[2]  ->maxNormalLightWaitMs
   * Default Value: 250
   * Range: [0 – 2000]
   * Description: Convergence time out in Normal light*/
  float      maxNormalLightWaitMs;


  /* reserve[3]  ->jumpWithNoSkip
   * Default Value: 15
   * Range: [10 - 500]
   * Description: If Skip frame is enabled, then if we have moved than the
   * threshold of lens steps specified by this, then we need to skip a frame*/
  float      jumpWithNoSkip;


  /* reserve[4]  ->SkipframesBeforeFineSearch
   * Default Value: 0
   * Range: [0 - 5]
   * Description: This will enable skip frame before fine search if we move beyond the threshold
   * specified by [jumpWithNoSkip]*/
  float      SkipframesBeforeFineSearch;


  /* reserve[5]  ->dist_thresh_macro_region_in_mm
   * Default Value: 200
   * Range: [0 - 10000]
   * Description: Distance threshold for Macro region in mm*/
  float      dist_thresh_macro_region_in_mm;


  /* reserve[6]  ->dist_thresh_non_macro_factor
   * Default Value: 2
   * Range: [0 - 10000]
   * Description: Distance threshold for Non Macro region in mm*/
  float      dist_thresh_non_macro_factor;


  /* reserve[7]  ->lowLightThreshold
   * Default Value: 3
   * Range: [0 - 8]
   * Description: Threshold used to compare against Scene BV to check
   * if we are in Low Light*/
  float      lowLightThreshold;


  /* reserve[8]  ->useFineSearchExtension
   * Default Value: 0
   * Range: [0 - 1]
   * Description: If Fine search extension needs to be enabled in HAF*/
  boolean    useFineSearchExtension;


  /* reserve[9]  ->macroRegionThreshold_in_mm
   * Default Value: 200
   * Range: [0 - 10000]
   * Description: Macro region threshold in mm*/
  float      macroRegionThreshold_in_mm;


  /* reserve[10]  ->farRegionThresholdin_mm
   * Default Value: 5000
   * Range: [0 - 10000]
   * Description: Far region threshold in mm*/
  float      farRegionThresholdin_mm;


  /* Fine Search Step info for Tuning
   * stepSize : Step Size
   * far_steps : No of Steps in Far Direction
   * near_steps : No of Steps in Near Direction*/
  af_dciaf_fine_search_step_adapter  normalLightFineSearchInfo[Q3A_AF_DCIAF_MAX_RANGE];


  /* Fine Search Step info for Tuning
   * stepSize : Step Size
   * far_steps : No of Steps in Far Direction
   * near_steps : No of Steps in Near Direction*/
  af_dciaf_fine_search_step_adapter  lowLightFineSearchInfo[Q3A_AF_DCIAF_MAX_RANGE];

} af_dciaf_extended;

typedef struct _af_tuning_dciaf_adapter_t
{
    unsigned char                         monitor_enable;
    unsigned int                          monitor_freq;
    unsigned int                          search_freq;
    float                                 baseline_mm;
    af_cam_orientation_adapter_t          aux_direction;
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
    int                                   maxLowLightWaitMs;
    int                                   maxNormalLightWaitMs;
    int                                   jumpWithNoSkip;
    int                                   SkipframesBeforeFineSearch;
    int                                   dist_thresh_macro_region_in_mm;
    int                                   dist_thresh_non_macro_factor;
    int                                   lowLightThreshold;
    unsigned char                         useFineSearchExtension;
    int                                   macroRegionThreshold_in_mm;
    int                                   farRegionThresholdin_mm;
    af_dciaf_fine_search_step_adapter     normalLightFineSearchInfo[Q3A_AF_DCIAF_MAX_RANGE];
    af_dciaf_fine_search_step_adapter     lowLightFineSearchInfo[Q3A_AF_DCIAF_MAX_RANGE];
    af_dciaf_extended                     dciaf_extended;
} af_tuning_dciaf_adapter_t;

/*
* _af_tuning_follower_adapter_t:
*  Tuning information which is not present in regular tuning data
*
* @enable : If follower data is present
* @gravity_comp_on : If Lens Sag Compension needs to be enabled
* @jump_to_start_limit : How much of a Jump Can we do in one shot before final convergence before we skip a frame
* @lowLightThreshold : Lux Index to indcate the low light level
* @useFineSearchExtension : if Fine search extension needs to be enabled
* @activeConvergenceFollow: Should Follower be actively Converging when peer is converging
* @lensPosThresholdForActiveConvergenceFollow : How much lens move should dictate a lens move on Peer
* @macroRegionThreshold_in_mm : Threshold to determine if its macro region
* @farRegionThresholdin_mm : Threshold to determine if its far region
* @normalLightFineSearchInfo : Fine Search Info for Normal Light
* @lowLightFineSearchInfo : Fine Search Info for Low Light
* @searchStrategy: Follower Search Strategy
*/
typedef struct _af_tuning_follower_adapter_t
{
  boolean                                     enable;
  boolean                                     gravity_comp_on;
  int                                         jump_to_start_limit;
  int                                         lowLightThreshold;
  boolean                                     useFineSearchExtension;
  boolean                                     activeConvergenceFollow;
  int                                         lensPosThresholdForActiveConvergenceFollow;
  int                                         macroRegionThreshold_in_mm;
  int                                         farRegionThresholdin_mm;
  af_fine_search_step_info                    normalLightFineSearchInfo[AF_DISTANCE_MAX_RANGE];
  af_fine_search_step_info                    lowLightFineSearchInfo[AF_DISTANCE_MAX_RANGE];
  af_follower_search_strategy                 searchStrategy;
} af_tuning_follower_adapter_t;

typedef struct _af_tuning_sad_adapter
{
    unsigned char            enable;
    float                    gain_min;
    float                    gain_max;
    float                    ref_gain_min;
    float                    ref_gain_max;
    float                    threshold_min;
    float                    threshold_max;
    float                    ref_threshold_min;
    float                    ref_threshold_max;
    unsigned short           unstable_count_th;
    unsigned short           stable_count_th;
    unsigned short           frames_to_wait;
    unsigned short           grid_region_start;
    unsigned short           grid_region_end;
    float                    sadr_threshold_scaler;
    float                    aec_gain_input[AF_SCENE_CHANGE_MAX_ENTRIES];
    float                    sadr_threshold_scaler_multiplier[AF_SCENE_CHANGE_MAX_ENTRIES];
} af_tuning_sad_adapter_t;

/******************************************************************************
** af_tuning_chroma_t: Chroma based scene change detection
**
** Variable name: enable
** Description: enable flag for chroma based scene change detection feature
** 3A version:
** Default value: 0
** Data range: 0 or 1
** Constraints:
**
** Variable name: g_over_b_thres
** Description: when G/B ratio is greater than this threshold
** scene change is detected and AF is triggered
** 3A version:
** Default value: 0.5
** Data range: [0 5]
** Constraints: Higher the threshold difficult to trigger AF
**              Smaller the threshold eady to trigger AF
**
** Variable name: g_over_r_thres
** Description: when G/R ratio is greater than this threshold
** scene change is detected and AF is triggered
** 3A version:
** Default value: 0.4
** Data range: [0 5]
** Constraints: Higher the threshold difficult to trigger AF
**              Smaller the threshold eady to trigger AF
**
** Variable name: aec_gain_input
** Description: Setting different gain levels to have different thresholds
** 3A version:
** Default value: {5, 10, 18, 22, 30}
** Data range: [0 30]
** Constraints:
**
** Variable name: g_over_b_threshold_scaler
** Description: Scaler threshold to multiple with g_over_b_thres at
** different gain level
** 3A version:
** Default value: {1, 1.5, 2, 2.2, 2.5}
** Data range: [1 5]
** Constraints: Higher the threshold difficult to trigger AF
**              Smaller the threshold eady to trigger AF
**
** Variable name: g_over_r_threshold_scaler
** Description: Scaler threshold to multiple with g_over_r_thres at
** different gain level
** 3A version:
** Default value: {1, 1.5, 2, 2.2, 2.5}
** Data range: [1 5]
** Constraints: Higher the threshold difficult to trigger AF
**              Smaller the threshold eady to trigger AF
**
** Variable name: unstable_count_th
** Description: number of frames to detect unstable scene change
** 3A version:
** Default value: 5
** Data range: [1 20]
** Constraints: Higher the threshold difficult to trigger AF
**              Smaller the threshold eady to trigger AF
**
** Variable name: num_of_samples
** Description: number of frames to monitor for scene change detection
** 3A version:
** Default value: 5
** Data range: [1 20]
******************************************************************************/
typedef struct _af_tuning_chroma_adapter_t
{
    unsigned char     enable;
    float             g_over_b_thres;
    float             g_over_r_thres;
    float             aec_gain_input[AF_SCENE_CHANGE_MAX_ENTRIES];
    float             g_over_b_threshold_scaler[AF_SCENE_CHANGE_MAX_ENTRIES];
    float             g_over_r_threshold_scaler[AF_SCENE_CHANGE_MAX_ENTRIES];
    unsigned short    unstable_count_th;
    unsigned short    num_of_samples;
} af_tuning_chroma_adapter_t;

typedef struct _af_tof_fine_scan_adapter_t
{
  int                                   num_near_steps_near_direction;
  int                                   num_far_steps_near_direction;
  int                                   num_near_steps_far_direction;
  int                                   num_far_steps_far_direction;
  int                                   num_near_steps_far_converge_point;
  int                                   num_far_steps_far_converge_point;
  int                                   near_steps_far_direction_tof_conf;
  int                                   tof_step_size;
  int                                   tof_step_size_low_light;
  int                                   far_distance_fine_step_size;
} af_tof_fine_scan_adapter_t;

typedef struct _af_tof_frame_delay_adapter_t
{
    int                                   frame_delay_focus_converge_bright_light;
    int                                   frame_delay_focus_converge_low_light;
    int                                   frame_delay_fine_search_bright_light;
    int                                   frame_delay_fine_search_low_light;
    int                                   frame_delay_outdoor_light;
} af_tof_frame_delay_adapter_t;

typedef struct _af_tof_scene_change_adapter_t
{
    af_tuning_sad_adapter_t               af_sad;
    int                                   scene_change_distance_thres[AF_TOF_MAX_DISTANCE_ENTRIES];
    int                                   scene_change_distance_std_dev_thres;
    int                                   distance_region[AF_TOF_MAX_DISTANCE_ENTRIES];
    int                                   far_distance_unstable_cnt_factor;
    int                                   panning_stable_cnt;
    int                                   panning_stable_thres;
    int                                   normal_light_cnt;
    int                                   lowlight_cnt;
    int                                   num_monitor_samples;
} af_tof_scene_change_adapter_t;

typedef struct _af_tof_flag_adapter_t
{
    int                                   use_custom_sensitivity;
    int                                   enable_sad_when_tof_is_conf;
    int                                   run_tof_in_outdoor_light_flag;
    int                                   aec_wait_check_flag_enable;
    int                                   far_scene_coarse_srch_enable;
} af_tof_flag_adapter_t;

typedef struct _af_tof_filter_adapter_t
{
    int                                   median_filter_enable;
    int                                   median_filter_num_of_samples;
    int                                   average_filter_enable;
    int                                   average_filter_num_of_samples;
} af_tof_filter_adapter_t;

typedef struct _af_tof_focus_converge_adapter_t
{
    int                                   jump_to_start_limit;
    int                                   jump_to_start_limit_low_light;
    int                                   start_pos_for_tof;
    int                                   far_converge_point;
} af_tof_focus_converge_adapter_t;

typedef struct _af_tuning_tof_adapter_t
{
    float                                 sensitivity;
    int                                   outdoor_lux_idx;
    int                                   jump_to_start_limit;
    int                                   jump_to_start_limit_low_light;
    int                                   start_pos_for_tof;
    int                                   far_converge_point;
    int                                   normal_light_cnt;
    int                                   lowlight_cnt;
    int                                   num_monitor_samples;
    int                                   tof_step_size;
    af_tof_filter_adapter_t               tof_filter;
    af_tof_flag_adapter_t                 tof_flag;
    af_tof_frame_delay_adapter_t          frame_delay;
    af_tof_scene_change_adapter_t         scene_change;
    af_tof_focus_converge_adapter_t       focus_converge;
    af_tof_fine_scan_adapter_t            fine_scan;
    af_tuning_tof_extended_adapter_t      tof_extended;
} af_tuning_tof_adapter_t;

typedef struct _af_fine_srch_extension_adapter_t
{
    unsigned char                        enable;
    int                                  max_fine_srch_extension_cnt;
    int                                  num_near_steps;
    int                                  num_far_steps;
    int                                  step_size;
    float                                decrease_drop_ratio;
    float                                fine_srch_reserve[10];
} af_fine_srch_extension_adapter_t;


typedef struct _af_tuning_haf_extended_adapter_t
{
  /* reserve[0] -> max_fine_srch_extension_cnt
   * Default Value: 3
   * Range: [0 , 5]
   * Description: This parameter keeps extending the range until max_fine_srch_extension_cnt
   * is met. For example: if the range provided by PDAF/TOF/DciAF is [200, 230]
   * and current lens pos is 200, it does search from 200 to 230 and if we keep
   * getting increasing FV trend, then it means that the peak is beyond 230, so
   * now it extends the range to [230, 260] and performs the search, again if it
   * doesn’t find a peak then does a extension again to [260, 290] now the
   * extension has been done 2 times, say if the max_fine_srch_extension_cnt
   * is set to 3 then it would do extension three times.
   * Constraints: Larger the number, more the search time, If we set small number
   * such 0 then it would not perform the extension at all.*/
  float     max_fine_srch_extension_cnt;

  /* reserve[1] -> num_near_steps
   * Default Value: 2
   * Range: [0, 10]
   * Description: In order to find the near_limit, num_near_steps is multiplied
   * with the step_size and is added to the current position to determine the near_limit
   * Constraints: Larger the number, larger the search range hence search time will increase*/
  float     num_near_steps;

  /* reserve[2] -> num_far_steps
   * Default Value: 2
   * Range: [0, 10]
   * Description: In order to find the far_limit, num_far_steps is multiplied
   * with the step_size and is added to the current position to determine the far_limit
   * Constraints: Larger the number, larger the search range hence search time will increase*/
  float     num_far_steps;

  /* reserve[3] -> step_size
   * Default Value: 6
   * Range: [1, 100]
   * Description: The step size used to find the search range if the extension is required.
   * Constraints: Larger the number, larger the search range hence search time will increase*/
  float     step_size;

  /*Reseve[4]->decrease_drop_ratio
   * Default Value: 0.985
   * Range: [0, 1]
   * Description: The decrease drop ratio is used to determine the stop criteria
   * for the search. If the ratio of current frame fv by previous frame fv is
   * less than decrease_drop_ratio then the search stops.
   * Constraints: higher the number easily to find the peak but more susceptible to the noise*/
  float     decrease_drop_ratio;

  /* reserve[5] -> enable_init_pos:
   * Default Value: 0/false
   * Range: [0 – 1]
   * Description: If this filed is enable, the initial/default lens position
   * will be between [SINGLE_NEAR_LIMIT_IDX - SINGLE_INF_LIMIT_IDX], otherwise
   * default lens positing will be infinity.*/
  boolean   enable_init_pos;

  /* reserve[6] -> idx
   * Default Value: 0
   * Range: [0 – 11]
   * Description: index of initial lens position. This filed will be used if
   * enable_init_pos is enable*/
  float     idx;

  /* reserve[7] -> depth_conf_cnt_th
   * Default Value: 2
   * Range: [1 – 10]
   * Description: The number of frames which algo should return confident in
   * order to overwrite caf decision.*/
  float     depth_conf_cnt_th;

  /* reserve[10] -> spotlight_enabled
   * Default Value: 0
   * Range: 0 or 1
   * Description: Enable flag for spotlight AF process.*/
  boolean   spotlight_enabled;

  /* reserve[11] -> AF_SPOT_LIGHT_INF_FLAT_THRS
   * Default Value: 0.55
   * Range: float [0.0 – 1.0]
   * Description: Flat threshold for far scene when detecting a spotlight condition.
   * The lower the spotlight infinity scene easier to be recognized as flat scene*/
  float     AF_SPOT_LIGHT_INF_FLAT_THRS; /*11*/

  /* reserve[12] -> AF_SPOT_LIGHT_MARCO_FLAT_THRS
   * Default Value: 0.85
   * Range: float [0.0 – 1.0]
   * Description: Flat_threshold for macro scene when detecting spotlight condition.
   * The lower the spotlight macro focus scene easier to be recognized as flat scene*/
  float     AF_SPOT_LIGHT_MARCO_FLAT_THRS; /*12*/

  /* reserve[13] -> AF_SPOT_LIGHT_INC_FLAT_THRS
   * Default Value: 1.035
   * Range: float [1-3]
   * Description: Threshold to judge if current AF position is in increase or flat condition.
   * General Scene = 1.005 Spotlight Scene = 1.035
   *The larger the value, the harder to be recognized as FV increment.
   * Increasing inc_flat_th and decreasing dec_flat_th helps to prevent ending
   * in a false peak position and to recognize it as a flat curve.*/
  float     AF_SPOT_LIGHT_INC_FLAT_THRS; /*13*/

  /* reserve[14] -> AF_SPOT_LIGHT_DEC_FLAT_THRS
   * Default Value: 0.978
   * Range: float [0 – 1]
   * Description: Threshold to judge if current AF position is in decrease or
   * flat condition. General Scene = 0.995, Spotlight Scene = 0.978. The lower
   * the value the harder to be recognized as FV decrement. Increasing inc_flat_th
   * and decreasing dec_flat_th helps to prevent ending in a false peak position
   * and to recognize it as a flat curve.*/
  float     AF_SPOT_LIGHT_DEC_FLAT_THRS; /*14*/

  /* reserve[15] -> exp_index_th
   * Default Value: 300
   * Range: int [0 – 500]
   * Description: Exposure index threshold, if exposure index is smaller than
   * this threshold, the spotlight will not be detected. The smaller the threshold,
   * the less chances that spotlight can be detected
   * Constraints: data value need to be bigger than aec_indoor_index*/
  float     exp_index_th;

  /* reserve[16] -> af_bright_region_zone_start_h
   * Default Value: 4
   * Range: uint32_t [0 – 15]
   * Description: Table that defines the spotlight zone size. This value defines
   * the horizontal start location in terms of grid of bright region zone
   * Constraints: The resulting grid should be equal to or smaller than the
   * background zone size*/
  float     af_bright_region_zone_start_h;

  /* reserve[17] -> af_bright_region_zone_end_h
   * Default Value: 12
   * Range: uint32_t [0 – 15]
   * Description: Table that defines the spotlight zone size. This value defines
   * the horizontal end location in terms of grid of bright region zone
   * Constraints: The resulting grid should be equal to or smaller than the
   * background zone size*/
  float     af_bright_region_zone_end_h;

  /* reserve[18] -> af_bright_region_zone_start_v
   * Default Value: 4
   * Range: uint32_t [0 – 15]
   * Description: Table that defines the spotlight zone size.
   * This value defines the vertical start location in terms of grid of bright
   * region zone
   * Constraints: The resulting grid should be equal to or smaller than the
   * background zone size*/
  float     af_bright_region_zone_start_v;

  /* reserve[19] -> af_bright_region_zone_end_v
   * Default Value: 12
   * Range: uint32_t [0 – 15]
   * Description: Table that defines the spotlight zone size. This value defines
   * the vertical end location in terms of grid of bright region zone
   * Constraints: The resulting grid should be equal to or smaller than the
   * background zone size*/
  float     af_bright_region_zone_end_v;

  /* reserve[20] -> af_background_zone_start_h
   * Default Value: 2
   * Range: uint32_t [0 – 15]
   * Description:  Table that defines the background zone location and size.
   * This variable defines the starting grid horizontal location of background zone.
   * Constraints: The resulting grid should be equal to or larger than the
   * spotlight detection zone size*/
  float     af_background_zone_start_h;

  /* reserve[21] -> af_background_zone_end_h
   * Default Value: 14
   * Range: uint32_t [0 – 15]
   * Description: Table that defines the background zone location and size.
   * This variable defines the end grid horizontal location of background zone.
   * Constraints: The resulting grid should be equal to or larger than the
   * spotlight detection zone size*/
  float     af_background_zone_end_h;

  /* reserve[22] -> af_background_zone_start_v
   * Default Value: 2
   * Range: uint32_t [0 – 15]
   * Description: Table that defines the background zone location and size.
   * This variable defines the starting grid vertical location of background zone.
   * Constraints: The resulting grid should be equal to or larger than the
   * spotlight detection zone size*/
  float     af_background_zone_start_v;

  /* reserve[23] -> af_background_zone_end_v
   * Default Value: 14
   * Range: uint32_t [0 – 15]
   * Description: Table that defines the background zone location and size.
   * This variable defines the en grid vertical location of background zone.
   * Constraints: The resulting grid should be equal to or larger than the
   * spotlight detection zone size*/
  float     af_background_zone_end_v;

  /* reserve[24] -> bright_region_luma_delta_background
   * Default Value: 2.0  
   * Range: float [1.0 – 8.0]
   * Description: if region’s luma is higher than average luma of background zone
   * multiply this parameter, this region will be recognize as a bright region  
   * Constraints: Give higher value will be harder to recognize region as
   * spotlight*/
  float     bright_region_luma_delta_background;

  /* reserve[25] -> bright_region_luma_delta_spotlight_zone_low_grey
   * Default Value: 2.5
   * Range: float [0.0 – 15.0]
   * Description: if region’s luma of cluster zone is higher than average luma
   * of background zone,  multiply it by this parameter. The region will be
   * recognized as a bright region. The bigger the threshold, the less likely
   * that region is detected as a bright region.
   * Constraints: Suggest to keep default*/
  float     bright_region_luma_delta_spotlight_zone_low_grey;

  /* reserve[26] -> bright_region_luma_delta_spotlight_zone_dark_grey
   * Default Value: 3.0
   * Range: float [0.0 – 15.0]
   * Description: if region’s luma of cluster zone is higher than average luma
   * of background zone,  multiply it by this parameter. The region will be
   * recognized as a bright region. The bigger the threshold, the less likely
   * that region is detected as a bright region.
   * Constraints: Suggest to keep default*/
  float     bright_region_luma_delta_spotlight_zone_dark_grey;

  /* reserve[27] -> region_low_grey_th
   * Default Value: 30
   * Range: uint32 [0 – 255]
   * Description: The threshold of the low grey region. If the region’s luma of
   * the background zone is smaller than this threshold, this region is recognized
   * as a low grey region. The smaller the threshold, the less likely that region
   * can be recognized as a low grey region.
   * Constraints: Should be larger than region_dark _grey_th*/
  float     region_low_grey_th;

  /* reserve[28] -> region_dark_grey_th
   * Default Value: 7
   * Range: uint32 [0 – 255]
   * Description: The threshold of the dark grey region, if the region’s luma of
   * the background zone is smaller than this threshold, this region will be
   * recognized as a dark grey region. The smaller the threshold, the less likely
   * that region can be recognized as a dark grey region.
   * Constraints: Should be smaller than region_low _grey_th*/
  float     region_dark_grey_th;

  /* reserve[29] -> background_low_grey_pct_th
   * Default Value: 0.85
   * Range: float [0.6 – 1.0]
   * Description: The threshold percentage of low in the background zone.
   * If low grey percentage of the background zone is larger than this threshold,
   * the system considers the background is dark enough. The bigger the threshold,
   * the less likely the system will consider the background is dark enough.
   * Constraints: Should be bigger than BACKGROUND_DARK_GREY_PCT_TH. Suggest to
   * keep default. A lower value is not recommended.*/
  float     background_low_grey_pct_th;

  /* reserve[30] -> background_dark_grey_pct_th
   * Default Value: 0.3
   * Range: float [0.2 – 1.0]
   * Description: The threshold percentage of dark grey in the background zone.
   * If dark grey percentage of the background zone is larger than this threshold,
   * the system considers the background is dark enough. The bigger the threshold,
   * the less likely the system will consider the background is dark enough.
   * Constraints: Should be smaller than BACKGROUND_LOW_GREY_PCT_TH.
   * Recommend to adjust by user preference*/
  float     background_dark_grey_pct_th;

  /* reserve[31] -> background_luma_ave_th
   * Default Value: 30
   * Range: uint32_t [0 – 255]
   * Description: The threshold for average luma of the background zone, if for
   * average luma of the background zone is bigger than this threshold, the
   * spotlight will not be detected. The larger the threshold, the more chances
   * that spotlight can be detected.
   * Constraints: None*/
  float     background_luma_ave_th;

  /* reserve[32] -> square_cluster_increase_size
   * Default Value: 1
   * Range: uint32_t [0 – 3]
   * Description: The extended size of the bright region square cluster,
   * both vertically and horizontally. Set to 0 to disable a cluster
   * shape (square, vertical, horizontal) extension. Just one bright region
   * can be recognized as spotlight.
   * Constraints: The extended size should not be over the background zone size.
   * Suggest to keep default.*/
  float     square_cluster_increase_size;

  /* reserve[33] -> square_cluster_pct_th
   * Default Value:
   * Range:
   * Description: The threshold percentage of bright region in the cluster zone.
   * If bright region percentage of the cluster zone is larger than this threshold
   * and background is dark enough, the flag of bright region cluster will be raised.
   * The bigger the threshold, the less likely that cluster is detected.
   * Constraints: If set to 1.0, the cluster is disabled. Suggest to keep default*/
  float     square_cluster_pct_th;

  /* reserve[34] -> vertical_cluster_increase_size
   * Default Value: 2
   * Range: uint32_t [0 – 3]
   * Description: The extended size of the bright region vertical cluster.
   * Set to 0 to disable a cluster shape (square, vertical, horizontal) extension.
   * Just one bright region can be recognized as spotlight.
   * Constraints: The extended size should not be over the background zone size.
   * Suggest to keep default.*/
  float     vertical_cluster_increase_size;

  /* reserve[35] -> vertical_cluster_pct_th
   * Default Value:
   * Range:
   * Description: The threshold percentage of bright region in the cluster zone.
   * If bright region percentage of the cluster zone is larger than this threshold
   * and background is dark enough, the flag of bright region cluster will be raised.
   * The bigger the threshold, the less likely that cluster is detected.
   * Constraints: If set to 1.0, the cluster is disabled. Suggest to keep default*/
  float     vertical_cluster_pct_th;

  /* reserve[36] -> horizontal_cluster_increase_size
   * Default Value: 2
   * Range: uint32_t [0 – 3]
   * Description: The extended size of the bright region horizontal cluster.
   * Set to 0 to disable a cluster shape (square, vertical, horizontal). Just one
   * bright region can be recognized as spotlight.
   * Constraints: The extended size should not be over the background zone size.
   * Suggest to keep default.*/
  float     horizontal_cluster_increase_size;

  /* reserve[37] -> horizontal_cluster_pct_th
   * Default Value:
   * Range:
   * Description: The threshold percentage of bright region in the cluster zone.
   * If bright region percentage of the cluster zone is larger than this threshold
   * and background is dark enough, the flag of bright region cluster will be raised.
   * The bigger the threshold, the less likely that cluster is detected.
   * Constraints: If set to 1.0, the cluster is disabled. Suggest to keep default*/
  float     horizontal_cluster_pct_th;

  /* reserve[38] -> spotlight_detect_cnt_th
   * Default Value: 3
   * Range: uint32_t [0 – max]
   * Description: Threshold to determine how many countinuous spotlight detected
   * flags need to be raised before declaring final spotlight detected to be true.
   * Usually the threshold means how many continuous frames of spotlight detected.
   * Constraints: Setting the value to too large will make final spotlight
   * detected to be infeasible.*/
  float     spotlight_detect_cnt_th;

  /* reserve[40] -> haf_roi_enable
   * Default Value: 0
   * Range: [0 – 7]
   * Description: control enableing/disableing touch and face ROI handling in HAF.*/
  int       haf_roi_enable;

  /* reserve[41] -> haf_roi_h_num
   * Default Value: 1
   * Range: 1-6
   * Description: Number of Horizontal PDAF window
   * Constraints: PDAF Type 1,2 and 3 currently supports only 1 window*/
  float     haf_roi_h_num;

  /* reserve[42] -> haf_roi_v_num
   * Default Value: 1
   * Range: 1-8
   * Description: Number of Vertical PDAF window
   * Constraints: PDAF Type 1,2 and 3 currently supports only 1 window*/
  float     haf_roi_v_num;

  /* reserve[43] -> haf_center_roi_size
   * Default Value: 0.2
   * Range: 0.2-0.3
   * Description: Default size of PDAF window at the center
   * Constraints: Value  has to be smaller or equal to haf_corner_roi_size*/
  float     haf_center_roi_size;

  /* reserve[44] -> haf_corner_roi_size
   * Default Value: 0.25
   * Range: 0.2-0.3
   * Description: Default size of PDAF window at the corner. ROI size will be interpolated between center and corner roi size.
   * Constraints: Value has to be larger or equal to haf_center_roi_size*/
  float     haf_corner_roi_size;
  float     haf_roi_delay;

  boolean            enableROIShifting;
  af_roi_shift_type  roi_shift_type;
  int       fine_start_move_buffer;
  float     dof_multiplier;

  boolean   enable_force_scan_end;
  int       enable_passive_scan_timeout;
  int       max_scan_cnt_th;

  boolean   enable_fine_search_at_roleSwitch;
  af_fine_search_step_info roleswitch_fine_search_info;
  /*Dynamic Anchor Points*/
  boolean                                     enable_dynamic_anchor_points;
  af_single_index_adapter_t                   anchor_pos1;
  af_single_index_adapter_t                   anchor_pos2;
  af_single_index_adapter_t                   minimum_focus_index;
  af_hjr_info                                 hjr_info;
  af_caf_hjr_info                             caf_hjr_info;

  /* Reserve[80] -> fine_search_peakdown_thres:
  Default value: 0.03f
  Range: [0.0f - 1.0f]
  Description: fine search will end if peakdown reach this thres
  Constraints: */
  int                                         fine_search_peakdown_thres;

  /* Reserve[81] -> face_far_end_enabled:
  Default value: 0
  Range: [0.0f or 1.0f]
  Description:when enable , new far end will be used for face fine search
  Constraints: */
  boolean                                     face_far_end_enabled;

  /* Reserve[82] -> face_far_end:
  Default value: 0
  Range: [0 - 1000]
  Description: this value will be updated as the new lens far end when fine search happened for face ROI
  Constraints: */
  int                                         face_far_end;

  /* Reserve[95] -> fine_search_range_ratio:
  Default value: 0.0f
  Range: [0.0f, 0.5f]
  Description: This flag determines the fine search range ratio threshold.
  if the primary algo outpu search range / AF full range < fine_search_range_ratio,
  the secondary algo will goto fine search. otherwise, it will goto af single.
  if given the default value(0.0f), the caf algo will use the internal default value 0.2f.
  Constraints: */
  float                                       fine_search_range_ratio;

  /* Reserve[97] -> spd_stats_type:
  Default value: 0 - Use 16 x 16 YSum
  Range: [0 or 1] 0 - 16x16 YSum stats
                             1 - 64x48 BG stats
  Description: This flag determines spotlight detection algorithm stats type. When value is 0, it runs with default 16x16 YSum stats.
                   If the value is 1, then SPD will run with 64x48 BG stats which provides finer granularity.
  Constraints: */
  int                                     spd_stats_type;

  /* Reserve[98] -> spd_bg_stats_thresh_auto_convert_enable:
  Default value: 1
  Range: [0 or 1] 0 - Disable
                             1 - Enable
  Description: When enable, all the YSum spotlight tuning thresholds will be auto converted to BG stats compatible thresholds.
  For example, if spotlight tuning thresholds are start_h = 4 and end_h = 12, then with BG auto convert, it will directly runtime
  change thresholds to start_h = 16 and en_h = 48(By multiplying the BG/YSum grid ratio). If it is disabled, then the grids will be
  use as it is, and no auto-convert will happen.

  Constraints: */
  boolean                                     spd_bg_stats_thresh_auto_convert_enable;

  /* Reserve[99] -> spd_bg_stats_bright_region_sat_ratio_thresh:
  Default value: 0.03f
  Range: [0.0f - 1.0f]

  Description: While running BG stats spotlight detection, if any bright region detection zone grid has saturation ratio higher
  than this threshold, then the grid will be count as a bright region.

  Constraints: */
  float                                     spd_bg_stats_bright_region_sat_ratio_thresh;

}af_tuning_haf_extended_adapter_t;

typedef struct _af_tuning_haf_adapter_t
{
    unsigned char                     enable;
    unsigned char                     algo_enable[Q3A_AF_HAF_ALGO_MAX];
    unsigned char                     stats_consume_enable[Q3A_AF_FV_STATS_TYPE_MAX];
    char                              lens_sag_comp_enable;
    char                              hysteresis_comp_enable;
    int                               actuator_shift_comp;
    int                               actuator_hysteresis_comp;
    float                             actuator_sensitivity;
    af_fv_stats_adapter_t             stats_select;
    float                             fine_srch_drop_thres;
    unsigned int                      fine_step_size;
    unsigned int                      max_move_step;
    unsigned int                      max_move_step_buffer;
    unsigned int                      base_frame_delay;
    af_tuning_pdaf_adapter_t          pdaf;
    af_tuning_tof_adapter_t           tof;
    af_tuning_dciaf_adapter_t         dciaf;
    af_tuning_follower_adapter_t      follower;
    af_fine_srch_extension_adapter_t  fine_srch_extension;
    af_tuning_sad_adapter_t           af_sad;
    af_tuning_gyro_adapter_t          af_gyro;
    unsigned int                      scene_change_during_FS;
    int                               face_caf_high_priority_enable;
    af_tuning_haf_extended_adapter_t  haf_extended;
} af_tuning_haf_adapter_t;

typedef struct _af_tuning_vbt_adapter_t
{
    unsigned char       enable;
    float               comp_time;
    float               contamination_pcnt_allow;
} af_tuning_vbt_adapter_t;

/******************************************************************************
** af_tuning_smooth_lens_convergence: Smooth lens convergence feature parameters
**
** Variable name: enable
** Description: enable flag for smooth lens convergence
** 3A version:
** Default value: 0
** Data range: 0 or 1
** Constraints: None
** Effect:
**
** Variable name: fudge_factor
** Description: Compensation factor inorder to get the
**              right crop factor
** 3A version:
** Default value: 1
** Data range: [0 1]
** Constraints: None
**
** Variable name: anchor_lens_pos1
** Description: Lens position corresponding to the anchor_distance_mm_1
**              Need to specify the lens position, can refer to Contrast AF
**              tuning settings from single_index_t
** 3A version:
** Default value: lens position specified at SINGLE_10CM_IDX in single_index_t
** Data range: [near_end far_end]
**
** Variable name: anchor_lens_pos2
** Description: Lens position corresponding to the anchor_distance_mm_2
**              Need to specify the lens position, can refer to Contrast AF
**              tuning settings from single_index_t
** 3A version:
** Default value: lens position specified at SINGLE_HYP_F_IDX in single_index_t
** Data range: [near_end far_end]
**
** Variable name: anchor_distance_mm_1
** Description: Physical distance in millimeters at which anchor_lens_pos1
**              corresponds to.
** 3A version:
** Default value: 100
** Data range: [0 3000]
**
** Variable name: anchor_distance_mm_2
** Description: Physical distance in millimeters at which anchor_lens_pos2
**              corresponds to.
** 3A version:
** Default value: 2000
** Data range: [0 3000]
**
** Variable name: focus_converge_step_size_bright_light
** Description: Step size for focus converge in bright light
** 3A version:
** Default value: 8
** Data range: [0 50]
**
** Variable name: fine_search_step_size_bright_light
** Description: Step size for fine search in bright light
** 3A version:
** Default value: 6
** Data range: [0 50]
**
** Variable name: focus_converge_step_size_low_light
** Description: Step size for focus converge in low light
** 3A version:
** Default value: 9
** Data range: [0 50]
**
** Variable name: fine_search_step_size_low_light
** Description: Step size for fine search in low light
** 3A version:
** Default value: 7
** Data range: [0 50]
******************************************************************************/
typedef struct _af_tuning_fovc_adapter_t
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
} af_tuning_fovc_adapter_t;

/******************************************************************************
** af_tuning_spotlight_af: Spotlight detection/af process feature parameters
**
** Variable name: enable
** Description: enable flag for spotlight af process
** 3A version:
** Default value: 0
** Data range: 0 or 1
** Constraints: None
**
** Variable name: far_flat_th
** Description: flat_threshold for far scene when detecting spotlight condition
** 3A version:
** Default value: 0.55
** Data range: [0 1]
** Constraints: None
** Effect: The lower the easier to be recognized as flat scene
**
** Variable name: marco_flat_th
** Description: flat_threshold for marco scene when detecting spotlight condition
** 3A version:
** Default value: 0.85
** Data range: [0 1]
** Constraints: None
** Effect: The lower the easier to be recognized as flat scene
**
** Variable name: inc_flat_th
** Description: inc_flat_threshold to judge current af position is in increase or flat condition
** 3A version:
** Default value: 1.035
** Data range: [1 3]
** Constraints: None
** Effect: The larger the harder to be recognized as FV increment
**
** Variable name: dec_flat_th
** Description: dec_flat_threshold to judge current af position is in decrease or flat condition
** 3A version:
** Default value: 0.978
** Data range: [0 1]
** Constraints: None
** Effect: The lower the harder to be recognized as FV decrement
**
** Variable name: spotlight_zone_start_h
** Description: Defines the horizontal start region of spotlight zone in 16x16 total regions
** 3A version:
** Default value:4
** Data range: [0 15]
** Constraints: need to be smaller than spotlight_zone_end_h and also smaller than background_zone_start_h
**
** Variable name: spotlight_zone_end_h
** Description: Defines the horizontal end region of spotlight zone in 16x16 total regions,
**              the spotlight zone horizontal end region = spotlight_zone_end_h - 1
** 3A version:
** Default value:12
** Data range: [1 16]
** Constraints: need to be bigger than spotlight_zone_start_h;
**
** Variable name: spotlight_zone_start_v
** Description: Defines the vertical start region of spotlight zone in 16x16 total regions
** 3A version:
** Default value:4
** Data range: [0 15]
** Constraints: need to be smaller than spotlight_zone_end_v
**
** Variable name: spotlight_zone_end_v
** Description: Defines the vertical end region of spotlight zone in 16x16 total regions,
**             the spotlight zone vertical end region = spotlight_zone_end_v - 1
** 3A version:
** Default value:12
** Data range: [1 16]
** Constraints: need to be bigger than spotlight_zone_start_v
**
** Variable name: background_zone_start_h
** Description: Defines the horizontal start region of background zone in 16x16 total regions
** 3A version:
** Default value:2
** Data range: [0 15]
** Constraints: need to be smaller than background_zone_end_h
**
** Variable name: background_zone_end_h
** Description: Defines the horizontal end region of background zone in 16x16 total regions,
**              the background zone horizontal end region = background_zone_end_h - 1
** 3A version:
** Default value:14
** Data range: [1 16]
** Constraints: need to be bigger than background_zone_start_h
**
** Variable name: background_zone_start_v
** Description: Defines the vertical start region of background zone in 16x16 total regions
** 3A version:
** Default value:2
** Data range: [0 15]
** Constraints: need to be smaller than background_zone_end_v
**
** Variable name: background_zone_end_v
** Description: Defines the vertical end region of background zone in 16x16 total regions,
**              the background zone vertical end region = background_zone_end_v - 1
** 3A version:
** Default value:14
** Data range: [1 16]
** Constraints: need to be bigger than background_zone_start_v
**
** Variable name: exposure_index_trigger_th
** Description: if exposure index is smaller than this threshold, the spotlight will not be detected
** 3A version:
** Default value:350
** Data range: [200 400]
** Constraints: None
**
** Variable name: background_luma_avg_th
** Description: If the average luma of background zone is higher than this threshold,
**              the spotlight will not be detected
** 3A version:
** Default value:30
** Data range: [1 255]
** Constraints:
**
** Variable name: region_low_grey_th
** Description: Defines the low grey luma threshold. If luma is smaller than this threshold,
**               it will be recognized as a low grey region, suggest to disable it by default value 255
** 3A version:
** Default value:255
** Data range: [1 255]
** Constraints:
**
** Variable name: region_dark_grey_th
** Description: Defines the dark grey luma threshold. If luma is smaller than this threshold,
**              it will be recognized as a dark grey region, good to keep default value
** 3A version:
** Default value:7
** Data range: [1 255]
** Constraints:
**
** Variable name: background_low_grey_pct_th
** Description: if the low grey percentage is higher than this threshold, the low grey flag will be raised.
**              Suggest to disable it by default
** 3A version:
** Default value:1.0
** Data range: [0 1.0]
** Constraints: If dark grey has been detected first, the low grey will not be used anymore
**
** Variable name: background_dark_grey_pct_th
** Description: if the dark grey percentage is higher than this threshold, the dark grey flag will be raised
** 3A version:
** Default value:0.3
** Data range: [0 1.0]
** Constraints: If dark grey has been detected first, the low grey will not be used anymore
**
** Variable name: bright_region_luma_delta_background_zone
** Description: if regionâ€™s luma is higher than average luma of background zone multiply this parameter, this region will be recognize as a bright region
** 3A version:
** Default value:2.0
** Data range: [0 8.0]
** Constraints:
**
** Variable name: bright_region_luma_delta_spotlight_zone_low_grey
** Description: In low grey condition and cluster is enabled,if regionâ€™s luma is higher than average luma of background zone multiply
**                this parameter, this region will be recognize as a bright region
** 3A version:
** Default value:2.5
** Data range: [0 8.0]
** Constraints:
**
** Variable name: bright_region_luma_delta_spotlight_zone_dark_grey
** Description: In dark grey condition and cluster is enabled,if regionâ€™s luma is higher than average luma of background zone multiply
**                this parameter, this region will be recognize as a bright region
** 3A version:
** Default value:3.0
** Data range: [0 8.0]
** Constraints:
**
** Variable name: square_cluster_inc_size
** Description: The maximum bright region will be the center region and the square cluster size will depends
**              on the parameter, set to 0 will disable this cluster effect
** 3A version:
** Default value:1
** Data range: [0 2]
** Constraints:
**
** Variable name: vertical_cluster_inc_size
** Description: The maximum bright region will be the center region and the vertical cluster size will depends
**              on the parameter, set to 0 will disable this cluster effect
** 3A version:
** Default value:2
** Data range: [0 2]
** Constraints:
**
** Variable name: horizontal_cluster_inc_size
** Description: The maximum bright region will be the center region and the vertical cluster size will depends
**              on the parameter, set to 0 will disable this cluster effect
** 3A version:
** Default value:2
** Data range: [0 2]
** Constraints:
**
** Variable name: square_cluster_pct_th
** Description: If the bright region percentage of square cluster zones is higher this threshold,
**              the flag of square_bright_region_cluster will be raised
** 3A version:
** Default value:0.8
** Data range: [0 1.0]
** Constraints:
**
** Variable name: vertical_cluster_pct_th
** Description: If the bright region percentage of vertical cluster zones is higher this threshold, the flag of
**              vertical_bright_region_cluster will be raised
** 3A version:
** Default value:0.2
** Data range: [0 1.0]
** Constraints:
**
** Variable name: horizontal_cluster_pct_th
** Description: If the bright region percentage of horizontal cluster zones is higher this threshold, the flag of
**              horizontal_bright_region_cluster will be raised
** 3A version:
** Default value:0.2
** Data range: [0 1.0]
** Constraints:
**
** Variable name: spotlight_detect_cnt_th
** Description: Spotlight detect flag threshold. Only if consecutive success spotlight detect over the threshold
**              count will result in the final spotlight detected flag be updated.
** 3A version:
** Default value:3
** Data range: [1 infinity]
** Constraints:
******************************************************************************/
typedef struct _af_tuning_spot_light_adapter
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
} af_tuning_spot_light_adapter_t;

typedef struct _af_tuning_continuous_adapter
{
    unsigned char                   enable;
    unsigned char                   scene_change_detection_ratio;
    float                           panning_stable_fv_change_trigger;
    float                           panning_stable_fvavg_to_fv_change_trigger;
    unsigned short                  panning_unstable_trigger_cnt;
    unsigned short                  panning_stable_trigger_cnt;
    unsigned long                   downhill_allowance;
    unsigned short                  uphill_allowance;
    unsigned short                  base_frame_delay;
    unsigned short                  scene_change_luma_threshold;
    unsigned short                  luma_settled_threshold;
    float                           noise_level_th;
    unsigned short                  search_step_size;
    af_algo_adapter_t               init_search_type;
    af_algo_adapter_t               search_type;
    unsigned short                  low_light_wait;
    unsigned short                  max_indecision_cnt;
    float                           flat_fv_confidence_level;
    af_tuning_sad_adapter_t         af_sad;
    af_tuning_gyro_adapter_t        af_gyro;
    af_tuning_chroma_adapter_t      af_chroma;
    unsigned int                    af_caf_trigger_after_taf;
    af_tuning_spot_light_adapter_t  af_spot_light;
} af_tuning_continuous_adapter_t;

typedef struct _af_tuning_exhaustive_adapter
{
    unsigned short    num_gross_steps_between_stat_points;
    unsigned short    num_fine_steps_between_stat_points;
    unsigned short    num_fine_search_points;
    unsigned short    downhill_allowance;
    unsigned short    uphill_allowance;
    unsigned short    base_frame_delay;
    unsigned short    coarse_frame_delay;
    unsigned short    fine_frame_delay;
    unsigned short    coarse_to_fine_frame_delay;
    float             noise_level_th;
    float             flat_fv_confidence_level;
    float             climb_ratio_th;
    int               low_light_luma_th;
    int               enable_multiwindow;
    float             gain_thresh;
} af_tuning_exhaustive_adapter_t;

typedef struct _af_tuning_fullsweep_adapter_t
{
    unsigned short    num_steps_between_stat_points;
    unsigned short    frame_delay_inf;
    unsigned short    frame_delay_norm;
    unsigned short    frame_delay_final;
    unsigned short    PAAF_enable;
} af_tuning_fullsweep_adapter_t;

typedef struct _af_shake_resistant_adapter
{
    unsigned char   enable;
    float           max_gain;
    unsigned char   min_frame_luma;
    float           tradeoff_ratio;
    unsigned char   toggle_frame_skip;
} af_shake_resistant_adapter_t;

typedef struct _af_motion_sensor_adapter
{
    float af_gyro_trigger;
    float af_accelerometer_trigger;
    float af_magnetometer_trigger;
    float af_dis_motion_vector_trigger;
} af_motion_sensor_adapter_t;

typedef struct _af_fd_priority_caf_adapter
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
} af_fd_priority_caf_adapter_t;

/******************************************************************************
** af_tuning_multiwindow_t: AF tuning parameters specific to
**  Multiwindow feature
**
** Variable name: enable
** Description: enable flag for multiwindow feature
** 3A version:
** Default value: 0
** Data range: 0 or 1
** Constraints: None
** Effect:
**
** Variable name: h_clip_ratio_normal_light
** AF window horizontal size in terms of ratio to the whole image. For the
** same example above, Horizontal Clip Ratio is 500/1000=0.5.
** 3A version:
** Default value: 0.5
** Data range: 0 - 1
** Constraints:
** Effect:
**
** Variable name: v_clip_ratio_normal_light
** AF window vertical size in terms of ratio to the whole image. For the
** same example above, Vertical Clip Ratio is 500/1000=0.5.
** 3A version:
** Default value: 0.5
** Data range: 0 -1
** Constraints:
** Effect:
**
** Variable name: h_clip_ratio_low_light
** AF window horizontal size in terms of ratio to the whole image. For the
** same example above, Horizontal Clip Ratio is 500/1000=0.5.
** 3A version:
** Default value: 0.5
** Data range: 0 - 1
** Constraints:
** Effect:
**
** Variable name: v_clip_ratio_low_light
** AF window vertical size in terms of ratio to the whole image. For the
** same example above, Vertical Clip Ratio is 500/1000=0.5.
** 3A version:
** Default value: 0.5
** Data range: 0 - 1
** Constraints:
** Effect:
**
** Variable name: h_num_default
** Default horizontal number of grids in an ROI.
** 3A version:
** Default value: 6
** Data range: [1 18]
** Constraints:
** Effect:
**
** Variable name: v_num_default
** Default vertical number of grids in an ROI.
** 3A version:
** Default value: 6
** Data range: [1 14]
** Constraints:
** Effect:
**
** Variable name: h_num_windows
** Default horizontal number of windows in an ROI.
** 3A version:
** Default value: 5
** Data range: [1 h_num_default]
** Constraints:
** Effect:
**
** Variable name: v_num_windows
** Default vertical number of windows in an ROI.
** 3A version:
** Default value: 5
** Data range: [1 v_num_default]
** Constraints:
** Effect:
******************************************************************************/
typedef struct _af_tuning_multiwindow_adapter_t
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

    /* reserve[0]  -> pd_percentile
         * Description: Nth Percentile of the valid windows to select from
         * 3A version:
         * Default value: 0.2
         * Data range: [0 1]
         */
    float pd_percentile;

    /* reserve[1] -> defocus_th
          * Description: Windows within this threshold of the best window will be selected
          * for contrast AF to calcuate FV.
         * 3A version:
         * Default value: 10
         * Data range: [0 50]
         */
    int defocus_th;

    /* reserve[3] -> mw_depth_percentile
          * Description: after sorting , will use this percentile to get the detect size for multi-depth detection
         * 3A version:
         * Default value: 0.08
         * Data range: [0 1]
         */
    float mw_depth_percentile;

    /* reserve[5] -> mw_confidence_th_multiple
          * Description: the confidence threshold for mw grids will be the min-conf * mw_confidence_th_multiple
         * 3A version:
         * Default value:1.5
         * Data range: [0 10]
         */
    float mw_confidence_th_multiple;

    /* reserve[6] -> mw_low_light_gain_th_low
          * Description:aec gain lower than this threshold when in lowlight mode , transition to normal light MW condition
         * 3A version:
         * Default value: 10
         * Data range: [0 60]
         */
    float mw_low_light_gain_th_low;

    /* reserve[7] -> mw_low_light_gain_th_high
          * Description: aec gain higher than this threshold when in normal light mode , transition to low light MW condition
         * 3A version:
         * Default value: 12
         * Data range: [0 60]
         */
    float mw_low_light_gain_th_high;

    /* reserve[8] -> mw_depth_detect_th_low
          * Description: depth difference from multi-depth detection lower than this threshold when in muti-depth mode ,
                              transition to not muti-depth condition.
         * 3A version:
         * Default value:100
         * Data range: [0 1000]
         */
    float mw_depth_detect_th_low;

    /* reserve[9] -> mw_depth_detect_th_high
          * Description: depth difference from multi-depth detection higher than this threshold when in not multi-depth mode ,
                              transition to multi-depth condition.
         * 3A version:
         * Default value:130
         * Data range: [0 1000]
         */
    float mw_depth_detect_th_high;


    /* reserve[10] -> lock_to_peripheral_during_search
         * Description: when trigger search by peripheral grids, this flag determin whether lock roi to
         *that peripheral grid to end of search.
         * 3A version:
         * Default value: 0
         * Data range: [0/1]
         */
    boolean lock_to_peripheral_during_search;
} af_tuning_multiwindow_adapter_t;

/******************************************************************************
** af_focus_mode_adapter_t: Enum for List of focus modes supported
**
**  Q3A_AF_FOCUS_MODE_DEFAULT: Default focus mode
**  Q3A_AF_FOCUS_MODE_MACRO: Macro  focus mode
**  Q3A_AF_FOCUS_MODE_NORMAL: Normal focus mode
**  Q3A_AF_FOCUS_MODE_CAF: Continuous focus mode
**  Q3A_AF_FOCUS_MODE_MANUAL: Manual focus mode
******************************************************************************/
typedef enum
{
    Q3A_AF_FOCUS_MODE_DEFAULT,
    Q3A_AF_FOCUS_MODE_MACRO,
    Q3A_AF_FOCUS_MODE_NORMAL,
    Q3A_AF_FOCUS_MODE_CAF,
    Q3A_AF_FOCUS_MODE_MANUAL,
    Q3A_AF_FOCUS_MODE_MAX
} af_focus_mode_adapter_t;

/**
* Structure defining search limits applicable to
* each focus mode.
* Note: This structure is to be moved to chromatix in the next
* chromatix revision.

 ******************************************************************************
** af_tuning_focus_mode_search_limit_t: AF tuning parameters defining search limits applicable to each focus mode
**
** Variable name: enable
** Description: enable/disable flag for the feature
** Default values: 0
** Range: [0,1]
**
**  Variable name: near_end
**  Description: Near end position for the mode
**  Default value: position_near_end.
**  Data range: [position_near_end position_far_end]
**
**  Variable name: far_end
**  Description: Near end position for the mode
**  Default value: position_far_end.
**  Data range: [position_near_end position_far_end]
**
**  Variable name: default_pos
**  Description: default position for the mode
**  Default value: position_default_in_normal.
**  Data range: [position_near_end position_far_end]
******************************************************************************/
typedef struct af_tuning_focus_mode_search_limit_adapter_t
{
    int          enable;
    unsigned int near_end;
    unsigned int far_end;
    unsigned int default_pos;
} af_tuning_focus_mode_search_limit_adapter_t;

typedef struct af_tuning_baf_exp_comp_adapter_t
{
  int enable;
  int kickin_progress;
} af_tuning_baf_exp_comp_adapter_t;

/**
* Structure defining advance fd tuning
* inculding robust fd-af and lost face enable
* Note: This structure is to be moved to chromatix in the next
* chromatix revision.
*/
typedef struct af_adv_fd_tuning {
  unsigned char     robust_fd_enable;
  unsigned int      noface_cnt_thr;
  unsigned int      stable_cnt_thr;
  float             fd_roi_pos_change_th;
  unsigned char     face_lost_trigger_enable;
  unsigned int      face_lost_stable_th;
}af_adv_fd_tuning;

/**
* Structure defining spot light tuning
* Note: This structure is to be moved to chromatix in the next
* chromatix revision.
*/
typedef struct af_spot_light_filter_tuning {
  int                   enable;
  float                 iir_2tap_filter[10];
  float                 iir_4tap_filter[10];
}af_spot_light_filter_tuning;

/**
* Structure defining spot light tuning
* Note: This structure is to be moved to chromatix in the next
* chromatix revision.
*/
typedef struct af_face_extension_tuning {
  int                   af_roi_face_extension;
  float                 af_roi_face_scale_x;
  float                 af_roi_face_scale_y;
  float                 af_roi_face_symmetric_x;
  float                 af_roi_face_symmetric_y;
}af_face_extension_tuning;


/**
* Structure defining search limits applicable to
* each focus mode.
* Note: This structure is to be moved to chromatix in the next
* chromatix revision.
*/
typedef struct _af_chromatix_search_limit {
  unsigned int near_end;
  unsigned int far_end;
  unsigned int default_pos;
} af_chromatix_search_limit_wrapper_t;

/******************************************************************************
* af_chromatix_step_back_wrapper_t: AF tuning parameters specific to step back
******************************************************************************/
typedef struct _af_chromatix_step_back {
  boolean enable;
  int     start_inf;
  int     start_mac;
  int     reverse_inf;
  int     reverse_mac;
} af_chromatix_step_back_wrapper_t;

/**
* Struct which defines mapping
* between luma and weight of a
* region
*/
typedef struct _luma_weight_entry {
  float luma;
  float weight;
} luma_weight_entry;

/**
* Table to contain mapping entries
* of luma and corresponding weight
*/
typedef struct _luma_weight_table {
  int  number_of_entries;
  luma_weight_entry entry[AF_LUMA_TABLE_SIZE];
} luma_weight_table;

/**
* Floating af weight table
*/
typedef struct _region_weight_table {
  int  window_size;
  float weight[AF_GRID_SIZE_MAX];
} region_weight_table;

typedef struct _fv_sort_weight_entry {
  float fv_sort;
  float weight;
} fv_sort_weight_entry;

typedef struct _fv_histogram_weight_table {
  int  number_of_entry;
  fv_sort_weight_entry entry[20];
} fv_histogram_weight_table;

typedef struct _fv_histogram_sort{
  uint64_t fv_after_sort;
  uint8    sort_result;
  uint8    update_flag;
} fv_histogram_sort;


/**
* Af weight tuning params
*/
typedef struct _af_weight_tuning{
  boolean is_af_region_weight_enable;
  boolean is_af_Luma_weight_enable;
  boolean is_af_face_weight_enable;
  region_weight_table region_weight_table;
  luma_weight_table luma_weight_table;
  fv_histogram_weight_table fv_histogram_weight_table;
} af_weight_tuning;

/**
* Structure defining parameters which are
* not a part of chromatix but needed by algo
* Note: This structure is to be moved to chromatix in the next
* chromatix revision.
*/
typedef struct _af_algo_params
{
  // weight params used by multi-window algorithm
  // to compensate brightness or position's influence
  af_weight_tuning  af_weight_tuning;
  int               firstrun_spotlight_detect_cnt_th;
} af_algo_params;

/******************************************************************************
* af_chromatix_scan_range_wrapper_t: AF tuning parameters specific to scan range
******************************************************************************/
typedef struct _af_chromatix_scan_range {
  float fratio_min;
  float fratio_max;
  float first_prev_y1;
  float first_next_y1;
  float first_prev_y2;
  float first_next_y2;
  float second_prev_y1;
  float second_next_y1;
  float second_prev_y2;
  float second_next_y2;
} af_chromatix_scan_range_wrapper_t;

/******************************************************************************
* af_chromatix_flat_detection_wrapper_t: AF tuning parameters about flat detection
******************************************************************************/
typedef struct _af_chromatix_flat_detection {
  boolean enable;
  float   threshold_bv;
} af_chromatix_flat_detection_wrapper_t;

/******************************************************************************
* af_chromatix_flat_threshold_wrapper_t: AF tuning parameters about flat threshold
******************************************************************************/
typedef struct _af_chromatix_flat_threshold {
  float increase;
  float decrease;
} af_chromatix_flat_threshold_wrapper_t;

/******************************************************************************
* af_chromatix_hys_offset_wrapper_t: AF tuning parameters about hysteresis offset
******************************************************************************/
typedef struct _af_chromatix_hys_offset {
  boolean enable;
  int     offset;
} af_chromatix_hys_offset_wrapper_t;

/******************************************************************************
* af_chromatix_fd_roi_wrapper_t: AF tuning parameters about FD ROI
******************************************************************************/
typedef struct _af_chromatix_fd_roi {
  boolean enable;
  float   ratio;
} af_chromatix_fd_roi_wrapper_t;

/******************************************************************************
* af_chromatix_outdoor_roi_wrapper_t: AF tuning parameters about Outdoor ROI
******************************************************************************/
typedef struct _af_chromatix_outdoor_roi {
  boolean enable;
  float   ratio_x;
  float   ratio_y;
} af_chromatix_outdoor_roi_wrapper_t;

/******************************************************************************
* af_chromatix_scene_detection_bv_wrapper_t: AF tuning parameters about scene change detection by BV
******************************************************************************/
typedef struct _af_chromatix_scene_detection_bv {
  boolean enable;
  float   outdoor_unstable_th;
  float   outdoor_stable_th;
  int     outdoor_trigger_cnt_th;
  float   low_unstable_th;
  float   low_stable_th;
  int     low_trigger_cnt_th;
  float   normal_unstable_th;
  float   normal_stable_th;
  int     normal_trigger_cnt_th;
} af_chromatix_scene_detection_bv_wrapper_t;

typedef struct _af_tuning_algo_extended_adapter_t
{
  af_chromatix_search_limit_wrapper_t         search_limit[AF_FOCUS_MODE_MAX];
  af_chromatix_step_back_wrapper_t            step_back;
  af_chromatix_scan_range_wrapper_t           scan_range;
  af_chromatix_flat_detection_wrapper_t       flat_detection;
  af_chromatix_flat_threshold_wrapper_t       flat_threshold;
  af_chromatix_hys_offset_wrapper_t           hys_offset;
  af_chromatix_fd_roi_wrapper_t               fd_roi;
  af_chromatix_outdoor_roi_wrapper_t          outdoor_roi;
  af_chromatix_scene_detection_bv_wrapper_t   scene_detect_bv;
  boolean                             en_ignore_first_invalid_frame;
  boolean                             en_detect_peak_in_decrease;
  boolean                             en_frame_skip_goto_peak;
  boolean                             en_fine_search_forcely;
  boolean                             en_simple_prescan;
  boolean                             en_optimize_peak;

  /* please refer to af_tuning_focus_mode_search_limit_wrapper_t.*/
  af_adv_fd_tuning      fd_adv_tuning;
  af_stats_use_type                   stats_comp_type;
  boolean                             hv_stats_ratio_enable;
  float                               gravity_x_land_thres;
  float                               gravity_y_land_thres;
  float                               gravity_x_port_thres;
  float                               gravity_y_port_thres;
  float                               hv_weight;
  af_spot_light_filter_tuning         spot_light_filter;
  af_face_extension_tuning            face_extension_tuning;
  boolean                             face_trig_priority_enable;
  boolean                             exp_comp_enable;
  af_tuning_baf_exp_comp_adapter_t    baf_exp_comp;
  af_algo_params        *af_algo_params;
  boolean                             face_fv_weight_enable;
  float                               face_fv_h_weight;
} af_tuning_algo_extended_adapter_t;

typedef struct _af_tuning_algo_adapter
{
    af_algo_adapter_t                           af_process_type;
    unsigned short                              position_near_end;
    unsigned short                              position_default_in_macro;
    unsigned short                              position_boundary;
    unsigned short                              position_default_in_normal;
    unsigned short                              position_far_end;
    unsigned short                              position_normal_hyperfocal;
    unsigned short                              position_macro_rgn;
    unsigned short                              undershoot_protect;
    unsigned short                              undershoot_adjust;
    float                                       min_max_ratio_th;
    int                                         led_af_assist_enable;
    long                                        led_af_assist_trigger_idx;
    int                                         lens_reset_frame_skip_cnt;
    float                                       low_light_gain_th;
    float                                       base_delay_adj_th;
    af_tuning_continuous_adapter_t              af_cont;
    af_tuning_exhaustive_adapter_t              af_exh;
    af_tuning_fullsweep_adapter_t               af_full;
    af_tuning_sp_adapter_t                      af_sp;
    af_tuning_single_adapter_t                  af_single;
    af_shake_resistant_adapter_t                af_shake_resistant;
    af_motion_sensor_adapter_t                  af_motion_sensor;
    af_fd_priority_caf_adapter_t                fd_prio;
    af_tuning_lens_sag_comp_adapter_t           lens_sag_comp;
    af_tuning_haf_adapter_t                     af_haf;
    af_tuning_vbt_adapter_t                     vbt;
    af_tuning_fovc_adapter_t                    fovc;
    af_tuning_multiwindow_adapter_t             af_multiwindow;
    af_tuning_focus_mode_search_limit_adapter_t search_limit[Q3A_AF_FOCUS_MODE_MAX];
    af_tuning_algo_extended_adapter_t           algo_extended;
} af_tuning_algo_adapter_t;

typedef struct _af_vfe_config_adapter
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
} af_vfe_config_adapter_t;

typedef struct _af_vfe_legacy_hpf_adapter
{
    char      a00;
    char      a02;
    char      a04;
    char      a20;
    char      a21;
    char      a22;
    char      a23;
    char      a24;
} af_vfe_legacy_hpf_adapter_t;

typedef struct _af_vfe_hpf_adapter
{
    af_vfe_legacy_hpf_adapter_t   af_hpf;
    int                           bf_hpf_2x5[MAX_HPF_2x5_BUFF_SIZE];
    int                           bf_hpf_2x11[MAX_HPF_2x11_BUFF_SIZE];
} af_vfe_hpf_adapter_t;

typedef struct _af_vfe_sw_fir_hpf_adapter_t
{
    int                    a[FILTER_SW_LENGTH_FIR];
    double                 fv_min_hi;
    double                 fv_min_lo;
    unsigned long int      coeff_length;
} af_vfe_sw_fir_hpf_adapter_t;

typedef struct _af_vfe_sw_iir_hpf_adapter_t
{
    double                 a[FILTER_SW_LENGTH_IIR];
    double                 b[FILTER_SW_LENGTH_IIR];
    double                 fv_min_hi;
    double                 fv_min_lo;
    unsigned long int      coeff_length;
} af_vfe_sw_iir_hpf_adapter_t;

typedef struct af_vfe_sw_hpf_adapter_t
{
    unsigned short                filter_type;    /* af_sw_filter_type */
    af_vfe_sw_fir_hpf_adapter_t   fir;
    af_vfe_sw_iir_hpf_adapter_t   iir;
} af_vfe_sw_hpf_adapter_t;

typedef struct _baf_tuning_gamma_adapter_t
{
    unsigned char                         ch_sel;
    float                                 Y_coeffs[MAX_BAF_GAMMA_Y_ENTRY];
    unsigned char                         g_sel;
    unsigned char                         LUT_enable;
    int                                   gamma_LUT[MAX_BAF_GAMMA_LUT_ENTRY];
} baf_tuning_gamma_adapter_t;

typedef struct _baf_tuning_gamma_param_adapter_t
{
    af_baf_tuning_preset_adapter_t      preset_id;
    baf_tuning_gamma_adapter_t          gamma_custom;
} baf_tuning_gamma_param_adapter_t;

typedef struct _baf_tuning_coring_adapter_t
{
    unsigned int                          x_index[MAX_BAF_CORING_ENTRY];
    int                                   threshold;
    int                                   gain;
} baf_tuning_coring_adapter_t;

typedef struct _baf_tuning_HV_filter_adapter_t
{
    unsigned char                         fir_en;
    unsigned char                         iir_en;
    int                                   fir[MAX_BAF_FIR_ENTRY];
    float                                 iir[MAX_BAF_IIR_ENTRY];
    int                                   shift_bits;
    unsigned char                         scaler_sel;
} baf_tuning_HV_filter_adapter_t;

typedef struct _baf_HV_filter_adapter_t
{
    af_baf_tuning_preset_adapter_t                preset_id;
    baf_tuning_coring_adapter_t                   coring;
    baf_tuning_HV_filter_adapter_t                filter_custom;
} baf_HV_filter_adapter_t;

typedef struct _baf_tuning_scaler_adapter_t
{
    int                                   M;
    int                                   N;
} baf_tuning_scaler_adapter_t;

typedef struct _baf_tuning_scaler_coefficient_adapter_t
{
    int                                   a;
    int                                   b;
} baf_tuning_scaler_coefficient_adapter_t;

typedef struct _baf_tuning_filter_adapter_t
{
    baf_HV_filter_adapter_t                       HV_filter[MAX_BAF_FILTER];
    baf_HV_filter_adapter_t                       HV_filter_2nd_tier[MAX_BAF_FILTER_2ND_TIER];
    baf_tuning_scaler_adapter_t                   scaler;
    baf_tuning_scaler_coefficient_adapter_t       coefficient;
} baf_tuning_filter_adapter_t;

typedef struct _baf_tuning_roi_adapter_t
{
    unsigned char                         flag;
    unsigned int                          x;
    unsigned int                          y;
    unsigned int                          width;
    unsigned int                          height;
} baf_tuning_roi_adapter_t;

typedef struct _baf_tuning_roi_param_adapter_t
{
    af_baf_roi_pattern_adapter_t          preset_id;
    float                                 grid_size_h;
    float                                 grid_size_v;
    float                                 grid_gap_h;
    float                                 grid_gap_v;
    int                                   num_ROI;
    baf_tuning_roi_adapter_t              ROI_custom[MAX_BAF_ROI_NUM];
} baf_tuning_roi_param_adapter_t;

typedef struct _af_tuning_baf_adapter_t
{
    baf_tuning_roi_param_adapter_t                roi_config[Q3A_AF_SCENE_TYPE_MAX];
    baf_tuning_gamma_param_adapter_t              gamma[Q3A_AF_SCENE_TYPE_MAX];
    baf_tuning_filter_adapter_t                   filter[Q3A_AF_SCENE_TYPE_MAX];
} af_tuning_baf_adapter_t;

typedef struct _af_tuning_vfe_adapter
{
    unsigned short   fv_metric;
    af_vfe_config_adapter_t  config;
    af_vfe_hpf_adapter_t     hpf_default;
    af_vfe_hpf_adapter_t     hpf_face;
    af_vfe_hpf_adapter_t     hpf_low_light;
} af_tuning_vfe_adapter_t;

typedef struct _af_tuning_sw_stats_adapter
{
    af_vfe_sw_hpf_adapter_t   sw_hpf_default;
    af_vfe_sw_hpf_adapter_t   sw_hpf_face;
    af_vfe_sw_hpf_adapter_t   sw_hpf_lowlight;
    float                     sw_fv_min_lux_trig_hi;
    float                     sw_fv_min_lux_trig_lo;
} af_tuning_sw_stats_adapter_t;

typedef struct _af_header_info_adapter
{
    unsigned short      header_version;
    char                module_name[MAX_ACT_MOD_NAME_SIZE];
    char                actuator_name[MAX_ACT_NAME_SIZE];
    af_cam_name_adapter cam_name;
} af_header_info_adapter_t;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)

/*========================================================================
 * Structure for AF Monitor Tuning:
 *   Support: four monitor algorithms: {CAF, PDAF, TOF, DCIAF}
 *   Each algo has its own topology.
 *   For each topology, specific value monitors (vm) are selected
 *   af_vm_sensitivity_level_t is a scalar for logical default sensitivity
 ========================================================================*/
/****** Macros *****/
typedef unsigned char af_vm_sensitivity_level_adapter_t;

/******  Enum  *****/
typedef enum
{
    Q3A_VM_SET_TYPE_SADR                   = 0,
    Q3A_VM_SET_TYPE_SAD_PANNING,
    Q3A_VM_SET_TYPE_GYRO,
    Q3A_VM_SET_TYPE_GYRO_PANNING,
    Q3A_VM_SET_TYPE_DEFOCUS_LOW,
    Q3A_VM_SET_TYPE_DEFOCUS_HIGH,
    Q3A_VM_SET_TYPE_TOF_CONF_HIGH,
    Q3A_VM_SET_TYPE_CONFIDENCE_PANNING,
    Q3A_VM_SET_TYPE_DISTANCE,
    Q3A_VM_SET_TYPE_FV,
    Q3A_VM_SET_TYPE_TOF_CONF_MEDIUM,
    Q3A_VM_SET_TYPE_SET11,
    Q3A_VM_SET_TYPE_CAF_SC,
    Q3A_VM_SET_TYPE_SET13,
    Q3A_VM_SET_TYPE_SET14,
    Q3A_VM_SET_TYPE_SET15,
    Q3A_VM_SET_TYPE_SET16,
    Q3A_VM_SET_TYPE_SET17,
    Q3A_VM_SET_TYPE_SET18,
    Q3A_VM_SET_TYPE_SET19,
    Q3A_VM_SET_TYPE_SET20,
    Q3A_VM_SET_TYPE_SET21,
    Q3A_VM_SET_TYPE_SET22,
    Q3A_VM_SET_TYPE_SET23,
    Q3A_VM_SET_TYPE_SET24,
    Q3A_VM_SET_TYPE_SET25,
    Q3A_VM_SET_TYPE_SET26,
    Q3A_VM_SET_TYPE_SET27,
    Q3A_VM_SET_TYPE_SET28,
    Q3A_VM_SET_TYPE_SET29,
    Q3A_VM_SET_TYPE_SET30,
    Q3A_VM_SET_TYPE_SET31,
    Q3A_VM_SET_TYPE_SET32,
    Q3A_VM_SET_TYPE_SET33,
    Q3A_VM_SET_TYPE_SET34,
    Q3A_VM_SET_TYPE_SET35,
    Q3A_VM_SET_TYPE_MAX                    = 36,
} af_vm_set_type_adapter_enum;

typedef enum
{
    Q3A_CAF_TOPO_PROFILE_SIMPLE           = 0,   /* Enum for current CAF included algo */
    Q3A_CAF_TOPO_PROFILE_ADVANCED,               /* Enum for enabling VM-based monitor algo */
    Q3A_CAF_TOPO_PROFILE_MAX,
} af_vm_topo_profile_caf_adapter_enum;

typedef enum
{
    Q3A_PDAF_TOPO_PROFILE_SPARSE           = 0,  /* Enum for current CAF included algo */
    Q3A_PDAF_TOPO_PROFILE_2PD,
    Q3A_PDAF_TOPO_PROFILE_ADVANCED,              /* Enum for enabling VM-based monitor algo */
    Q3A_PDAF_TOPO_PROFILE_MAX,
} af_vm_topo_profile_pdaf_adapter_enum;

typedef enum
{
    Q3A_TOF_TOPO_PROFILE_SIMPLE            = 0,  /* Enum for current CAF included algo */
    Q3A_TOF_TOPO_PROFILE_ADVANCED,               /* Enum for enabling VM-based monitor algo */
    Q3A_TOF_TOPO_PROFILE_MAX,
} af_vm_topo_profile_tof_adapter_enum;

typedef enum
{
    Q3A_DCIAF_TOPO_PROFILE_SIMPLE          = 0,  /* Enum for current CAF included algo */
    Q3A_DCIAF_TOPO_PROFILE_ADVANCED,             /* Enum for enabling VM-based monitor algo */
    Q3A_DCIAF_TOPO_PROFILE_MAX,
} af_vm_topo_profile_dciaf_adapter_enum;

typedef enum
{
    Q3A_VM_DETECTOR_STABLE = 0,
    Q3A_VM_DETECTOR_UNSTABLE,
    Q3A_VM_DETECTOR_MAX,
} af_vm_detector_type_adapter_enum;

/****** Struct *****/
typedef struct _af_bemo_params_adapter_t
{
   int                             recover_time;
   int                             lp_tolerance;
   int                             frame_skip_thres;
   int                             hammering_time;
   int                             sens_dec_delta;
   int                             sens_inc_delta;

} af_bemo_params_adapter_t;

typedef struct _af_monitor_topo_caf_config_adapter_t
{
    af_vm_topo_profile_caf_adapter_enum        profile_index;
    af_vm_sensitivity_level_adapter_t          sadr;
    af_vm_sensitivity_level_adapter_t          gyro;
    af_vm_sensitivity_level_adapter_t          gyro_panning;
    af_vm_sensitivity_level_adapter_t          sad_panning;
    af_vm_sensitivity_level_adapter_t          fv;
    af_vm_sensitivity_level_adapter_t          face_sc;
    af_vm_sensitivity_level_adapter_t          caf_sc;  /* reserved_sens[1] */
} af_monitor_topo_caf_config_adapter_t;

typedef struct _af_monitor_topo_pdaf_config_adapter_t
{
    af_vm_topo_profile_pdaf_adapter_enum       profile_index;
    af_vm_sensitivity_level_adapter_t          defocus_low;
    af_vm_sensitivity_level_adapter_t          defocus_high;
    af_vm_sensitivity_level_adapter_t          confidence_panning;
} af_monitor_topo_pdaf_config_adapter_t;

typedef struct _af_monitor_topo_tof_config_adapter_t
{
    af_vm_topo_profile_tof_adapter_enum        profile_index;
    af_vm_sensitivity_level_adapter_t          distance;
    af_vm_sensitivity_level_adapter_t          confidence_high;                  /* confidence */
    af_vm_sensitivity_level_adapter_t          confidence_medium;                /* reserved_sens[0] */
} af_monitor_topo_tof_config_adapter_t;

typedef struct _af_monitor_topo_dciaf_config_adapter_t
{
    af_vm_topo_profile_dciaf_adapter_enum      profile_index;
    af_vm_sensitivity_level_adapter_t          distance;
    af_vm_sensitivity_level_adapter_t          confidence;
} af_monitor_topo_dciaf_config_adapter_t;

typedef struct _af_monitor_algo_config_adapter_t
{
    af_monitor_topo_caf_config_adapter_t       topo_caf;
    af_monitor_topo_pdaf_config_adapter_t      topo_pdaf;
    af_monitor_topo_tof_config_adapter_t       topo_tof;
    af_monitor_topo_dciaf_config_adapter_t     topo_dciaf;
    int                                        medium_confidence_logic;           /* mixer_reserved[0] */
    int                                        secondary_scene_change_cnt_thres;  /* mixer_reserved[1] */
    int                                        secondary_unstable_cnt_thres;      /* mixer_reserved[2] */
    int                                        caf_topology_profile_advanced;     /* mixer_reserved[3] */
    int                                        pdaf_topology_profile_advanced;    /* mixer_reserved[4] */
    int                                        tof_topology_profile_advanced;     /* mixer_reserved[5] */
    int                                        dciaf_topology_profile_advanced;   /* mixer_reserved[6] */
    int                                        enable_bemo_for_primary_algo;      /* mixer_reserved[7] */
    int                                        enable_bemo_for_secondary_algo;    /* mixer_reserved[8] */
    af_bemo_params_adapter_t                   bemo_primary_params;                /* mixer_reserved[9 -14] */
    af_bemo_params_adapter_t                   bemo_secondary_params;              /* mixer_reserved[15-20] */
} af_monitor_algo_config_adapter_t;

typedef struct _af_vm_fltr_median_config_adapter_t
{
    int                                enable;
    int                                num_of_samples;
} af_vm_fltr_median_config_adapter_t;

typedef struct _af_vm_fltr_mvavg_config_adapter_t
{
    int                                enable;
    int                                num_of_samples;
} af_vm_fltr_mvavg_config_adapter_t;

typedef struct _af_vm_fltr_iir_config_adapter_t
{
    int                                enable;
    int                                num_of_samples;
    float                              coeff_numerator;
    float                              coeff_denominator;
} af_vm_fltr_iir_config_adapter_t ;

typedef struct _af_vm_sens_profile_adapter_t
{
    float                                      trigger_threshold;
    int                                        consecutive_count_threshold;
    int                                        frame2armed;
    af_vm_fltr_median_config_adapter_t         median_fltr_cfg;
    af_vm_fltr_mvavg_config_adapter_t          mvavg_fltr_cfg;
    af_vm_fltr_iir_config_adapter_t            iir_fltr_cfg;
} af_vm_sens_profile_adapter_t;

typedef struct _af_abs_mode_config_adapter_t
{
    int                                enable;
    float                              baseline;
} af_abs_mode_config_adapter_t;

typedef struct _af_vm_config_adapter_t
{
    af_vm_detector_type_adapter_enum    detection_type;
    af_abs_mode_config_adapter_t        absolute_mode_cfg;
    af_vm_sens_profile_adapter_t        sens_profile_low;
    af_vm_sens_profile_adapter_t        sens_profile_med;
    af_vm_sens_profile_adapter_t        sens_profile_high;
} af_vm_config_adapter_t;

typedef struct _af_value_monitor_bank_config_adapter_t
{
    af_vm_config_adapter_t             value_monitor[ Q3A_VM_SET_TYPE_MAX];
} af_value_monitor_bank_config_adapter_t;

typedef struct _af_tuning_monitor_adapter_t
{
    af_monitor_algo_config_adapter_t           monitor_algo_config;
    af_value_monitor_bank_config_adapter_t     value_monitor_bank;
} af_tuning_monitor_adapter_t;

typedef struct _af_tuning_extension_adapter_t
{
  boolean end; /*Added dummy member to avoid compile time warning*/
} af_tuning_extension_adapter_t;

#endif

typedef struct _af_algo_tune_parms_adapter
{
    af_header_info_adapter_t      af_header_info;
    af_tuning_algo_adapter_t      af_algo;
    af_tuning_sw_stats_adapter_t  af_swaf_config;
    af_tuning_vfe_adapter_t       af_vfe[MAX_AF_KERNEL_NUM];
    af_tuning_baf_adapter_t       af_baf;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    af_tuning_monitor_adapter_t   af_monitor;
    af_tuning_extension_adapter_t   af_reserve;
#endif

} af_algo_tune_parms_adapter_t;

/******************************************************************************
*******************************************************************************
**  AF related params end
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
**  AEC related params start
*******************************************************************************
******************************************************************************/
#define NUM_AEC_STATS              16
#define MAX_EXPOSURE_TABLE_SIZE    700
#define MAX_SNAPSHOT_LUT_SIZE      10
#define MAX_AEC_TRIGGER_ZONE       6
#define MAX_LED_MIX_LEVEL          16
#define MAX_LED_POWER_LEVEL        6
#define MAX_CALIB_NUM              12
#define CALIB_RANGE_NUM            2
#define MAX_EV_ARRAY_SIZE          49
/* Lux index temporal filter factor */
#define LUX_INDEX_TEMPORAL_FILTER_FACTOR  0.50f
/* Number of table entries for dual camera calibration compensationratios */
#define AEC_DUAL_CAM_COMP_TABLE_SIZE      6

/* IR mode Center weight */
#define IRMODE_WEIGHT_ROW_SIZE                         16
#define IRMODE_WEIGHT_COL_SIZE                         16
#define IRMODE_HIGHLIGHT_SUPRESS_WIEGHT_SIZE           64

typedef struct
{
    unsigned short gain;                        /* Gain*/
    unsigned int   line_count;                  /* Line Count*/
} exposure_entry_adapter;

typedef struct
{
    unsigned short          valid_entries;                              /* Number of entries in the exposure table with digital EV feature disabled*/
    int                     aec_enable_digital_gain_for_EV_lowlight;    /* Flag to enable\disable the digital EV feature*/
    unsigned short          total_entries_with_digital_gain_for_EV;     /* Number of entries in the exposure table with digital EV feature enabled*/
    unsigned short          fix_fps_aec_table_index;                    /* Index upto which the frame rate is fixed (linear AFR kicks in beyond this point)*/
    exposure_entry_adapter  exposure_entries[MAX_EXPOSURE_TABLE_SIZE];  /* Exposure table with gain and line count entries*/
} aec_exposure_table_adapter;

/**
* Lux trigger definition
*/
typedef struct
{
    int start;
    int end;
} aec_lux_trigger_adapter;

typedef enum
{
    Q3A_AEC_BAYER_CHNL_R,
    Q3A_AEC_BAYER_CHNL_G,
    Q3A_AEC_BAYER_CHNL_B,
    Q3A_AEC_BAYER_CHNL_MAX
} aec_bayer_channel_adapter_t;

/**
* Luma target tuning
*/
typedef struct
{
    unsigned int           luma_target;           /* Luma target*/
} aec_luma_target_triggered_adapter;

typedef struct
{
    int                                num_zones;                                  /* Number of zones actually used*/
    aec_lux_trigger_adapter            triggers[MAX_AEC_TRIGGER_ZONE - 1];         /* Trigger point to enter the next zone*/
    aec_luma_target_triggered_adapter  triggered_params[MAX_AEC_TRIGGER_ZONE];     /* Parameters that can be tuned per-zone*/
} aec_luma_target_adapter;

/**
* This structure defines the metering tables
*/
typedef struct
{
    float    AEC_weight_center_weighted[NUM_AEC_STATS][NUM_AEC_STATS];          /* Center-weighted metering table (0x304)*/
    float    AEC_weight_spot_metering[NUM_AEC_STATS][NUM_AEC_STATS];            /* Spot-metering table (0x304)*/
} aec_metering_table_adapter;

/******************************************************************************
** Feature name: AEC index compensation
** Applicable operation mode:  Snapshot, Preview and Camcorder
** When this feature is enabled, AEC output normalized AWB exposure index and
** lux index, regardless of the sensor resolution mode.
**
** Variable name: awb_exp_idx_comp_enable
** Flag to enable the AWB exposure index compensation feature
** Default value: 0
** Range: 0 and 1
**
** Variable name: lux_idx_comp_enable
** Flag to enable the lux index compensation feature
** Default value: 0
** Range: 0 and 1
**
** Variable name: ref_sensitivity
** Reference sensitivity. A must tune value if any of the above enable flags are set.
** The value is the sensitivity of the first entry of the preview mode exposure table.
** The sensitivity is calculated as exposure time (in second) multipled by gain, and
** further multipled by binning factor (most of the time, binning factor is 1).
** Default value: 0.00001
** Range : [0 1]
******************************************************************************/
typedef struct
{
    int      awb_exp_idx_comp_enable;           /* Flag to enable\disable the AWB exposure index compensation feature*/
    int      lux_idx_comp_enable;               /* Flag to enable\disable the lux index compensation feature*/
    float    ref_sensitivity;                   /* reference sensitivity value*/
} aec_idx_comp_adapter;

/******************************************************************************
** Feature name: AEC EV compensation
** Applicable operation mode:  Snapshot, Preview and Camcorder
** This structure enables tuning of the EV tables for both regular EV compensation
** and bracketing use cases.
**
** Variable name: steps_per_ev
** This variable defines the number of steps for each EV in the EV tables
** Due to s/w limitation, this variable needs to be changed simultaneously with
** the hard-coded values inside stats module code (aec_module.c) on LA.
** Default value: 6
** Range: 1 to 12 (integer)
**
** Variable name: min_ev_val
** The minimal EV value in the ev table. It is recommended to NOT change normally.
** Also this variable needs to be changed simultaneously with the hard-coded
** values inside stats module code (aec_module.c) on LA.
** Default value: -2
** Range: -2 to 0
**
** Variable name: max_ev_val
** The maximal EV value in the ev table. It is recommended to NOT change normally.
** Also this variable needs to be changed simultaneously with the hard-coded
** values inside stats module code (aec_module.c) on LA.
** Default value: 2
** Range: 0 to 2
**
** Variable name: ev_comp_table
** EV table that can be tuned for EV compensation in AEC auto mode. The EV table
** normally starts from -2, and ends at +2. With steps_per_ev set at 6, the table
** will need 25 entries. The maximum steps_per_ev is 12, which requires 49 entries
** in the EV table.
** Default value: {0.2500f, 0.2806f, 0.3150f, 0.3536f, 0.3969f, 0.4454f, 0.5000f,
**   0.5612f, 0.6299f, 0.7071f, 0.7937f, 0.8909f, 1.0000f, 1.1225f, 1.2599f, 1.4142f,
**   1.5874f, 1.7818f, 2.0000f, 2.2449f, 2.5198f, 2.8284f, 3.1748f, 3.5636f, 4.0000f}
** Range : [0.0f, 10.0f] for each entry
**
** Variable name: bracketing_table
** It is also an EV table, except that it is used in bracketing exposure calculation
** which is the multi-frame HDR use case. It can be tuned independently of regular
** AEC EV compensation.
** Default value: {0.2500f, 0.2806f, 0.3150f, 0.3536f, 0.3969f, 0.4454f, 0.5000f,
**   0.5612f, 0.6299f, 0.7071f, 0.7937f, 0.8909f, 1.0000f, 1.1225f, 1.2599f, 1.4142f,
**   1.5874f, 1.7818f, 2.0000f, 2.2449f, 2.5198f, 2.8284f, 3.1748f, 3.5636f, 4.0000f}
** Range : [0.0f, 10.0f] for each entry
******************************************************************************/
typedef struct
{
    unsigned short steps_per_ev;
    short          min_ev_val;
    short          max_ev_val;
    float          ev_comp_table[MAX_EV_ARRAY_SIZE];
    float          bracketing_table[MAX_EV_ARRAY_SIZE];
} aec_ev_table_adapter;

/******************************************************************************
** Feature name: AEC Luma Weight
** Applicable operation mode:  Snapshot, Preview and Camcorder
** Luma weight for calculating the luma is exposed. Also a compensation feature
** is introduced to allow AEC to compensate the r/g/b weights for luma
** calculation based on the current white balance decision. In addition, since
** WB gains are OTP corrected, the additional benefit of this feature is to also
** compensate luma variation caused by module-to-module variation as well.
**
** Variable name: r_weight
** The default weight used to multiply red channel to calculate luma. If color and
** sensor compensation is not enabled, this is the final red weight used by AEC.
** Default value: 0.2988
** Range: [0, 1]
**
** Variable name: g_weight
** The default weight used to multiply green channel to calculate luma. If color and
** sensor compensation is not enabled, this is the final green weight used by AEC.
** Default value: 0.5869
** Range: [0, 1]
**
** Variable name: b_weight
** The default weight used to multiply blue channel to calculate luma. If color and
** sensor compensation is not enabled, this is the final blue weight used by AEC.
** Default value: 0.1137
** Range: [0, 1]
**
** Variable name: color_and_sensor_comp_enable
** Enabling compensation for luma calculation under different lighting as well as
** different sensor modules. If this flag is set, "r_weight", "g_weight" and "b_weight"
** are modified dynamically based on the difference between current white balance gains
** and reference white balance gains as defined below.
** Default value: 0
** Range: 0 and 1
**
** Variable name: ref_rGain
** The reference white balance gain used to calculate the adjustment factor on
** r_weight. It is recommended to set this value to D50 red gain of the golden module.
** If feature is enabled, this value is must tune.
** Default value: 1.8
** Range: [1, 3]
**
** Variable name: ref_gGain
** The reference white balance gain used to calculate the adjustment factor on
** g_weight. It is recommended to set this value to D50 green gain of the golden module.
** If feature is enabled, this value is must tune.
** Default value: 1.0
** Range: [1, 3]
**
** Variable name: ref_bGain
** The reference white balance gain used to calculate the adjustment factor on
** b_weight. It is recommended to set this value to D50 blue gain of the golden module.
** If feature is enabled, this value is must tune.
** Default value: 1.5
** Range: [1, 3]
******************************************************************************/
typedef struct
{
    float   r_weight;                          /* Red weight for luma calculation*/
    float   g_weight;                          /* Green weight for luma calculation*/
    float   b_weight;                          /* Blue weight for luma calculation*/
    int     color_and_sensor_comp_enable;      /* Enable color and sensor module compensation for luma calculation*/
    float   ref_rGain;                         /* Reference red gain for golden module (recommend D50)*/
    float   ref_gGain;                         /* Reference green gain for golden module (recommend D50)*/
    float   ref_bGain;                         /* Reference blue gain for golden module (recommend D50)*/
} aec_luma_weight_adapter;

/** aec_aggressive_tuning:
*    @startup_frame_skip: Startup frame skip
*    @frame_skip: Frame skip
*    @luma_tol_percentage: Luma tolerance percentage
*    @max_frame_converge: Max frame convergence threshold for aec settled in instant capture
*    @min_settled_frame: Min settled frames needed to declare AEC is settled in instant capture
*
* Aggressive AEC tuning parameters
**/
typedef struct
{
    uint32 startup_frame_skip;
    uint32 frame_skip;
    float luma_tol_percentage;
    uint32 max_frame_converge;
    uint32 min_settled_frame;
    uint32 min_awb_settle_count;
} aec_aggressive_tuning;

/* Fast AEC tuning type */
typedef struct
{
    /* Fast AEC start index*/
    int     start_index;                /* Number of frames to skip after first AEC update during fast AEC*/
    int     startup_frame_skip;         /* Number of frames to skip between subsequent exposure updates*/
    int     frame_skip;                 /* Number of successive frames for which AEC has to be settled in order to declare AEC as 'settled'*/
    int     luma_settled_count;         /* Flag that determines if digital gain is to be used during fast AEC*/
    boolean use_digital_gain;
} aec_fast_aec_tuning;

/** aec_lls_tuning:
*  converted tuning parameters for Low light shot
*
*  enable:
*  luxindex_thr_*lux: Lux index threshold for * lux setting
*  req_exp_time_*lux: Required exp time for * lux setting
*/
typedef struct {
    uint32                               enable;
    float                                luxindex_thr_500lux;
    float                                luxindex_thr_200lux;
    float                                luxindex_thr_100lux;
    float                                luxindex_thr_050lux;
    float                                luxindex_thr_010lux;
    float                                req_exp_time_500lux;
    float                                req_exp_time_200lux;
    float                                req_exp_time_100lux;
    float                                req_exp_time_050lux;
    float                                req_exp_time_010lux;
    float                                req_extend_max_gain;
} aec_lls_tuning;

/******************************************************************************
** Feature name: AEC Dynamic Frame Skip
** Skip processing of frame if scene is stable
**
** Variable name: enable_frame_skip
** To enable or disable AEC dynamic frame skip.
** Default value: TRUE
**
** Variable name: min_fps_threshold
** Below this threshold, frame skip will be configured as 0.
** Default value: 15
** Range: [5,30]
**
** Variable name: max_frame_skip
** Max number of frames to be skipped when scene is stable.
** Default value: 1
** Range: {1, 2}

** Variable name: exp_index_tolerance
** EXposure index tolerance for dynamic aec frame skip
** Default value: 1
** Range: {1, 2}
******************************************************************************/
typedef struct {
  boolean   enable_frame_skip;
  int       min_fps_threshold;
  uint32    max_frame_skip;
  int       exp_index_tolerance;
} aec_dynamic_frame_skip;

typedef struct
{
    float dark_percentage_threshold;
    float sat_percentage_threshold;
    float dark_sat_percentage_threshold;
    float dark_bin_weight_adjust;
    float sat_bin_weight_adjust;
} aec_luma_adjust_adapter;

typedef struct
{
    int                      num_zones;
    aec_lux_trigger_adapter  triggers[MAX_AEC_TRIGGER_ZONE-1];
    aec_luma_adjust_adapter  data[MAX_AEC_TRIGGER_ZONE];
} aec_luma_adjust_lux_triggered_adapter;

#if 0
typedef struct {
  boolean   AE_hist_bin_weight_adjust_enable;
  float     AE_hist_bin_weight_adjust[128];
  float     dark_percentage_threshold;
  float     sat_percentage_threshold;
  float     lux_adjusted_target_offset;
  aec_luma_adjust_lux_triggered_adapter aec_luma_adjust;
}HDR_3exp_tuning_adapter;
#else
typedef struct {
  float   face_roi_factor;
  int     target_adjust_zone_num;
  float   gain[5];
  float   tolerance[5];
  boolean enable_target_adjust;
  float   es_ratio;
  boolean enable_HDR3;
}HDR_3exp_tuning_adapter;

#endif

/******************************************************************************
** Variable name: pct_based_luma_tolerance_enable
** Set this flag to enable AEC algorithm to use percentage based luma tolerance.
** It provides more consistency in AEC for lowlight and EV-minus use cases. This
** is required if ADRC is enabled.
** Default value: 0
** Range: 0 and 1
**
** Variable name: luma_tolerance_pct
** Luma tolerance to be used in all preview/camcorder AEC convergence.
** The smaller the luma tolerance, the more consistent AEC will be. However,
** chances of oscillation could also be higher.
** Suggest NOT tune unless absolutely necessary.
** Default value: 0.03
** Data range: (0, 1]
**
** Variable name: iso_quantization_enable
** Set this flag to enable standardized ISO value reporting from AEC.
** Gains sent to sensor are not changed.
** Default value: 1
** Range: 0 and 1
**
** Variable name: lux_index_ref
** Reference luma target for calculating lux index as well as awb exposure index.
** No need to tune.
** Default value: 50
** Range : [0 255]
**
** Variable name: aec_subsampling_factor
** Subsampling factor for AEC to skip bayer stats for the purpose of power saving
** Default value: 4
** Range: 1 to 8
******************************************************************************/
typedef struct
{
    unsigned int             aec_start_index;                     /* Exposure index to be used upon camera startup*/
    unsigned short           luma_tolerance;                      /* Tolerance range to deem AEC as settled*/
    int                      pct_based_luma_tolerance_enable;     /* Enable to switch algorithm to use percentage based luma tolerance*/
    float                    luma_tolerance_pct;                  /* Tolerance to deem AEC as settled, in percentage*/
    int                      frame_skip_startup;                  /* Frame skip value to be used upon camera startup*/
    float                    aggressiveness_startup;              /* Aggressiveness value to use upon camera startup*/
    float                    exposure_index_adj_step;             /* Exposure step size (0x304)*/
    float                    ISO100_gain;                         /* ISO 100 gain value (0x304)*/
    int                      antibanding_vs_ISO_priority;         /* In manual ISO, whether anti-banding or ISO takes priority*/
    int                      iso_quantization_enable;             /* Set this flag to enable standardized ISO*/
    float                    max_snapshot_exposure_time_allowed;  /* Maximum snapshot exposure time (in seconds) (0x304)*/
    unsigned int             lux_index_ref;                       /* A reference value to be used for lux index computation*/
    unsigned short           aec_subsampling_factor;              /* AEC stats subsampling for power optimization*/
    aec_idx_comp_adapter     idx_comp;                            /* AEC index compensation*/
    aec_ev_table_adapter     aec_ev_table;                        /* AEC EV compensation table*/
    aec_luma_weight_adapter  aec_luma_weight;                     /* Color and sensor compensation for luma calculation*/

    /* Extended Parameter */
    aec_fast_aec_tuning      fast_aec;                            /* Fast AEC tuning*/
    aec_lls_tuning           low_light_shot;                      /* Low Light Shot*/
    float                    flat_non_grey_th_low;                /* Flat scene parameters*/
    float                    flat_non_grey_th_high;               /* Flat scene parameters*/
    int                      enable_LV_test;                      /* Customer LV tests*/
    boolean                  enable_stored_params;                /* Flag to enable\disable warm start store/restore settings*/
    float                    warmstart_exp_range;                 /* Warm-Start exposure range*/
    aec_aggressive_tuning    aggressive;                          /* Aggressive AEC*/
    uint32                   ignore_hal_lock_before_preflash;     /* Sync sequence between Preflash and HAL lock*/
    float                    lux_index_filter_factor;             /* Temporal filtering of the lux index value*/
    int                      requested_rgn_skip_pattern;
    uint16                   fast_aec_forced_cnt;                 /* Force AWB to run with this number during fast AEC*/
    float                    cap_exp_index_ratio_for_flash_estimation;
    aec_dynamic_frame_skip   dynamic_frame_skip_config;           /* Dynamic frame skip */
    /* Extended Paramters for Dual Camera tuning */
    float                    dual_cam_exposure_mult;              // Reserved[3] - Dual Camera exposure multiplier
    float                    dual_cam_comp_ratio[AEC_DUAL_CAM_COMP_TABLE_SIZE]; // Reserved[4->9] - Compensation ratios for different AWB color temperatures
    int                      multi_frame_LLS_trigger_idx;         /* Auto LLS feature */
    HDR_3exp_tuning_adapter  HDR_3exp_tuning;
} aec_generic_tuning_adapter;

/**
* This structure defines the motion ISO tuning parameters
*/
typedef struct
{
    int       motion_iso_enable;           /* Flag to enable\disable the motion ISO feature*/
    float     motion_iso_aggressiveness;   /* Motion ISO aggressiveness*/
    float     motion_iso_threshold;        /* Threshold value for the motion ISO compensation to kick in*/
    float     motion_iso_max_gain;         /* Maximum gain that can be used for the motion ISO feature*/
} aec_motion_iso_adapter;

/**
* This structure defines the gain tradeoff and maximum exposure time entries for the snapshot look-up table (LUT)
*/
typedef struct
{
    unsigned short   lux_index;        /* Lux index corresponding to each entry*/
    float            gain_trade_off;   /* Gain tradeoff value*/
    float            max_exp_time;     /* Maximum exposure time value (in seconds)*/
} snapshot_trade_off_table_adapter;

/**
* This structure defines the the snapshot look-up table (LUT)
*/
typedef struct
{
    int                               enable;                                     /* Flag to enable\disable the snapshot LUT feature*/
    int                               exposure_stretch_enable;                    /* Flag to enable\disable the exposure stretch feature*/
    unsigned char                     valid_entries;                              /* Number of valid entries in the snapshot LUT*/
    snapshot_trade_off_table_adapter  snapshot_ae_table[MAX_SNAPSHOT_LUT_SIZE];   /* Snapshot look-up table (LUT)*/
} aec_snapshot_exposure_adapter;

/**
* LED mix entry contains: current combinations, reference point information, and flux value
* Variable name: LED1_setting.
* The LED1 current configuration for LED controller IC.
* Default value: 0.
*
* Variable name: LED2_setting.
* The LED2 current configuration for LED controller IC.
* Default value: 0
*
* Variable name: rg_ratio.
* rg_ratio measured for LUT entry
*   LED current settings by the sensor, use sgw logs in AWB.
* Default value:  1.0
* Data range: 0.1 to 7.0
* Constraints: None.
* Effect: Allows WB based on this rg-ratio to be applied under LED influence
*   only. Interpolation between ambient and this value when LED partially
*   influences the scene.
*
* Variable name: bg_ratio.
* bg_ratio measured for LUT entry
*   LED current settings by the sensor, use sgw logs in AWB.
* Default value:  1.0
* Data range: 0.1 to 7.0
* Constraints: None.
* Effect: Allows WB based on this bg_ratio to be applied under LED influence
*   only. Interpolation between ambient and this value when LED partially
*   influences the scene.
*
* Variable name: flux.
* Flux value for LUT entry settings.
* Default value:  1.0
* Data range: 0 to 1000.
* Constraints: None.
* Effect: Higher flux values expects more influence from LED flash.
*
* Variable name: CCT.
* Color temperature.
* Default value:  5000.
* Data range: 1000 to 10000.
* Constraints: None.
* Effect:
*
* Variable name: preflash_flux.
* Flux value for preflash mode.
* Default value:  1.0
* Data range: 0 to 1000.
* Constraints: None.
*
* Variable name: cont_burst_flux.
* Flux value for torch mode.
* Default value:  1.0
* Data range: 0 to 1000.
* Constraints: None.
*/
typedef struct
{
  unsigned short  LED1_setting;
  unsigned short  LED2_setting;
  float           rg_ratio;
  float           bg_ratio;
  float           flux;

  /* Extended Parameters */ /*309*/
  unsigned short  CCT;
  float           preflash_flux;
  float           cont_burst_flux;
} LED_mix_adapter;

/**
* This structure specifies a particular entry in led_table that should be calibrated
*/
typedef struct
{
    unsigned short   led_power_index;
    unsigned short   led_mix_index;
} LED_mix_calibration_entry_adapter;

/**
* This structure defines single/dual led calibration
*/
typedef struct
{
    unsigned short                     calib_num;                                          /* Number of combinations to calibrate*/
    LED_mix_calibration_entry_adapter  calib_entries[MAX_CALIB_NUM];                       /* Specific entries to calibrate*/
    int                                corner_sample_screen_enable;                        /* Enable screening of corner samples in calibration*/
    float                              rg_ratio_ok_range[MAX_CALIB_NUM][CALIB_RANGE_NUM];  /* RG ratio range to be flagged as OK*/
    float                              bg_ratio_ok_range[MAX_CALIB_NUM][CALIB_RANGE_NUM];  /* BG ratio range to be flagged as OK*/
    unsigned short                     fixed_CCTs[MAX_CALIB_NUM];                          /* CCTs to be fed to ISP during calibration*/
    unsigned int                       fixed_exp_index[MAX_CALIB_NUM];                     /* Fixed exposure indices for dual LED calibration*/
    float                              frame_err_allowance;                                /* Frame-to-frame allowance of error during calibration*/
    unsigned short                     frame_num;                                          /* Number of frames to average for each calibration*/
    unsigned short                     frame_num_max;                                      /* Max number of frames to be used for each calibration*/
} LED_mix_calibration_adapter;

/**
* Unified LED tuning
*/
typedef struct
{
    int                          dual_led_enable;            /* 1 to enable dual LED, 0 to enable single LED*/
    float                        dual_led_intersect_slope;   /* Slope is used to do intersection for dual LED estimation*/
    unsigned short               led_power_size;             /* Number of power levels for LED tuning (de-rating)*/
    unsigned short               led_mix_size;               /* Number of dual LED mixing entries*/
    LED_mix_adapter              cct_table[MAX_LED_POWER_LEVEL][MAX_LED_MIX_LEVEL];
    LED_mix_calibration_adapter  cct_table_calibration;      /* Dual LED calibration feature*/
} aec_led_match_table_adapter;

typedef struct
{
    float                        pre_flux;                    /* Pre-flash/torch flux value*/
    float                        flux_hi;                     /* High power LED flux value*/
    float                        flux_med;                    /* Medium power LED flux value*/
    float                        flux_low;                    /* Low power LED flux value*/
} aec_led_flux_adapter;

typedef struct
{
    /* For general flash and LED tuning*/
    aec_lux_trigger_adapter     wled_trigger;                  /* Lux index above which flash is fired in the AUTO mode*/
    int                         force_led_af_in_led_on_mode;   /* Set the flag to force AF always run when LED is set to ON*/
    boolean                     led_flux_used;                 /* Indicates whether led_flux field is used. If false, use the led_table_tuning field instead*/
    aec_led_flux_adapter        led_flux;                      /* Stores LED flux values.*/
    aec_led_match_table_adapter led_table_tuning;              /* For both dual and single LED tuning, with de-rating support*/

    int                         smart_flash_est_enable;        /* Enable smart LED estimation for AEC*/
    float                       smart_flash_est_strength;      /* Amount of flexibility/strength allowed for smart LED (0-1)*/
    int                         smart_flash_est_awb_enable;    /* Enable smart LED estimation also for manual AWB*/
    float                       smart_flash_est_awb_strength;  /* Amount of flexibility/strength allowed for AWB smart LED (0-1)*/

    int                         flash_target_enable;           /* Enable a dedicated set of flash luma target*/
    aec_luma_target_adapter     flash_target;                  /* Luma target for flash (similar as regular luma target)*/
    float                       target_interp_k_start;         /* Below this k value, current luma target is used for flash*/
    float                       target_interp_k_end;           /* Above this k value, flash target will be used*/

    int                         smart_flash_target_enable;     /* Enable smart decision of luma target (Target may increase)*/
    int                         max_target_offset;             /* Maximum allowed for the luma target to increase*/

    float                       lcd_flash_target;              /* LCD flash target */

    /* Extended Parameters */
    int                         enable_led_sat_prevent;        /* GenericTuning - Reserve[0] - Saturation prevention*/
    int                         enable_speckle_handling;       /* GenericTuning - Reserve[1] - speckle reflection handling*/
    float                       k_floor_pct;                   /* GenericTuning - Reserve[2] - LED Bias calculation related*/

    int                         bright_level_diff_thres;       /* reserve[0]*/
    int                         bright_level_thres;            /* reserve[1]*/
    float                       bright_sat_diff_thres;         /* reserve[2]*/
    float                       bright_sat_thres;              /* reserve[3]*/
    int                         conv_min_luma;                 /* reserve[4]*/
    int                         conv_min_adjust;               /* reserve[5]*/
    int                         off_stats_min;                 /* reserve[6]*/
    float                       sat_pct_thres;                 /* reserve[7]*/
    float                       k_stats_pct;                   /* reserve[8]*/
    float                       k_stats_min_diff;              /* reserve[9]*/
    int                         high_stat_target;              /* reserve[10]*/
    float                       led_conv_smooth_rate;          /* reserve[11]*/
    float                       sat_prevent_strength;          /* reserve[12]*/
    float                       sat_pct_start;                 /* reserve[13]*/
    float                       sat_pct_end;                   /* reserve[14]*/
} aec_flash_tuning_adapter;

/**
* This structure defines the touch AEC tuning parameters (0x304)
*/
typedef struct
{
    int     touch_roi_enable;       /* Flag to enable\disable the touch AEC feature*/
    float   touch_roi_weight;       /* Determines the influence of the touched region frame luma value on the overall frame luma calculation*/

    /* Extended Parameters - Advanced Tuning */
    float   temporal_filter_weight;
    float   speed_adjust_ratio;
    float   center_weight_strength;
    boolean touch_LED_metering_enable;

    /* Extended Parameters - Touch EV tuning */
    boolean touch_EV_enable;
    uint16  luma_delta_thres;
    float   luma_avg_pct_thres;
    float   luma_avg_min;
} aec_touch_adapter;

/* Max & Min Face weight */
typedef struct
{
    float face_wt_min;
    float face_wt_max;
} face_wt_zone_data_t;

/* Face weight adjustment according diffrent trigger zones */
typedef struct
{
    int                      num_zones;
    aec_lux_trigger_adapter  triggers[MAX_AEC_TRIGGER_ZONE-1];
    face_wt_zone_data_t      data[MAX_AEC_TRIGGER_ZONE];
} face_wt_zone_t;

/**
* This structure defines the face detection AEC tuning parameters (0x304: AEC_face_priority_adapter)
*/
typedef struct
{
    int     aec_face_enable;         /* Flag to enable\disable the face detection AEC feature*/
    float   aec_face_weight;         /* Determines the influence of the face frame luma value on the overall frame luma calculation*/

    /* Extended parameters - Advanced Tuning */
    float   status_update_delay_time_enter;
    float   status_update_delay_time_exit;
    float   speed_adjust_delay_time_enter;
    float   speed_adjust_delay_time_exit;
    float   speed_adjust_ratio;
    float   temporal_filter_weight;
    float   center_weight_strength;

    /* Extended Paramters - Backlit Compensation */
    boolean backlit_comp_enable;
    float   backlit_weight_adjust_hi;
    float   backlit_weight_adjust_low;
    float   backlit_luma_ratio_thres_hi;
    float   backlit_luma_ratio_thres_low;

    /* Extended Paramters - Frontlit Compensation */
    boolean frontlit_comp_enable;
    float   frontlit_weight_adjust_hi;
    float   frontlit_weight_adjust_low;
    float   frontlit_luma_ratio_thres_hi;
    float   frontlit_luma_ratio_thres_low;

    /* Face luma weight adjustment feature */
    float   face_wt_max;                  // face weight for maximum face roi size : map from aec_face_weight
    float   face_wt_min;                  // face weight for minimum face roi size : Reserve[12]
    float   face_sz_max;                  // maximum face roi size : Reserve[13] - Set 0 to disable ****
    float   face_sz_min;                  // minimum face roi size : Reserve[14]

    /* Configurable face luma target */
    float   face_luma_target;
    face_wt_zone_t face_wt_zone; // Face weight adjustment according diffrent trigger zones

} aec_face_priority_adapter;

/**
* This structure defines the face detection during flash tuning parameters
*/
typedef struct
{
    int     enable_led_fd_aec;                             /* AEC HDR Reserve[15] - Flag to enable\disable face detection during preflash */
    aec_lux_trigger_adapter lux_index_off_req_delay;       /* AEC HDR Reserve[16~17] - lux index to justify ENTERING request to run FD longer during pre-flash */
    float   led_face_wt_max;                               /* AEC HDR Reserve[18] - Luma calculation will weight face ROI and 1 - led_face_weight_max for frame ROI. Max: when face is large */
    float   led_face_wt_min;                               /* AEC HDR Reserve[19] - Luma calculation will weight face ROI and 1 - led_face_weight_max for frame ROI. Min: when face is small */
    float   led_face_sz_max;                               /* AEC HDR Reserve[20] - max face size as a % of smaller image dimension (height) */
    float   led_face_sz_min;                               /* AEC HDR Reserve[21] - max face size as a % of smaller image dimension (width) */
    uint8   led_fd_tuned_frame_count;                      /* AEC HDR Reserve[22] - Maximum acceptable additional delay after convergence */
    int     led_fd_luma_target;                            /* AEC HDR Reserve[23] - Luma target for LED+FD feature */
    float   led_fd_effect_face_sz_max;                     /* AEC HDR Reserve[24] - Face size larger or equal to threshold will cause LED+FD feature to take full effect */
    float   led_fd_effect_face_sz_min;                     /* AEC HDR Reserve[25] - Face size smaller or equal to threshold will cause LED+FD feature to have no effect */
} aec_led_fd_adapter;

typedef struct
{
    int     frame_skip;           /* Number of frames skipped between each exposure update*/
    float   aggressiveness;       /* Aggressivenes value which moving from a dark scene to a bright scene*/

    /* Extended Parameters */
    float   bright_ratio_adjust;
    float   dark_ratio_adjust;
    float   low_luma_cap_ratio;
    boolean enable_speed_adjust;
} aec_fast_conv_adapter;

/******************************************************************************
**
** Variable name: ht_tolerance_adjust_factor
** In order for holding time feature to activate, AEC needs to be stable for a
**   period of time. A different threshold/tolerance can be used to define whether
**   AEC is stable for holding time feature. This factor is used to adjust/relax
**   the luma tolerance used in normal mode for the purpose of holding time trigger.
** The higher this value, the easier holding time can be triggered.
** Default value: 2.0
** Range: [1.0, 4.0]
**
******************************************************************************/
typedef struct
{
    int     slow_convergence_enable;   /* Flag to enable slow convergence for video*/
    int     frame_skip;                /* Number of frames skipped between each exposure update*/
    float   conv_speed;                /* Convergence speed (0-1)*/
    int     ht_enable;                 /* Flag to enable\disable the holding time logic*/
    float   ht_tolerance_adjust_factor;/* This factor is used to relax luma tolerance for triggering holding time*/
    float   ht_thres;                  /* Holding time activation threshold for luma settle (in seconds)*/
    float   ht_max;                    /* Maximum amount of holding time allowed (in seconds)*/
    int     ht_gyro_enable;            /* Enable to use gyro for shortening holding time*/

    /* Extended parameters - Speed related advanced tuning */
    float   ref_frame_rate;
    float   step_dark;
    float   step_bright;
    float   step_regular;
    float   luma_tol_ratio_dark;
    float   luma_tol_ratio_bright;
    float   raw_step_adjust_cap;
    int     adjust_skip_luma_tolerance;
    float   dark_region_num_thres;
    float   bright_region_num_thres;

    /* Extended Parameters holding time advanced tuning */
    float   ht_luma_thres_low;
    float   ht_luma_thres_high;
    float   ht_luma_val_low;
    float   ht_luma_val_high;
    float   ht_gyro_thres_low;
    float   ht_gyro_thres_high;
    float   ht_gyro_val_low;
    float   ht_gyro_val_high;
} aec_slow_conv_adapter;

/**
* Bright region tuning parameters
*/
typedef struct
{
    unsigned int          bright_region_thres;  /* Threshold for a region to be considered a bright region (Q8)*/
    float                 bright_weight;        /* Weight to increase or decrease the influence of bright regions*/
} aec_bright_region_triggered_adapter;

typedef struct
{
    int                                  bright_region_enable;                    /* Flag to enable\disable the bright region feature*/
    int                                  num_zones;                               /* Number of zones actually used*/
    aec_lux_trigger_adapter              triggers[MAX_AEC_TRIGGER_ZONE - 1];        /* Trigger point to enter the next zone*/
    aec_bright_region_triggered_adapter  triggered_params[MAX_AEC_TRIGGER_ZONE];  /* Parameters that can be tuned per-zone*/
    float                                min_tweight;                             /* Minimal bias table weight for a stats to be included*/
    float                                max_bright_pct;                          /* Maximum number of bright regions to be counted*/
} aec_bright_region_adapter;

/**
* Dark region tuning parameters
*/
typedef struct
{
    unsigned int          dark_region_low_thres;  /* Below this threshold means it is dark (Q8)*/
    unsigned int          dark_region_high_thres; /* Above this threshold means it is not dark (Q8)*/
    float                 dark_weight;            /* Weight to increase or decrease the influence of dark region*/
} aec_dark_region_triggered_adapter;

typedef struct
{
    int                                dark_region_enable;                        /* Flag to enable\disable the dark region feature*/
    int                                num_zones;                                 /* Number of zones actually used*/
    aec_lux_trigger_adapter            triggers[MAX_AEC_TRIGGER_ZONE - 1];          /* Trigger point to enter the next zone*/
    aec_dark_region_triggered_adapter  triggered_params[MAX_AEC_TRIGGER_ZONE];    /* Parameters that can be tuned per-zone*/
    float                              min_tweight;                               /* Minimal bias table weight for a stats to be include*/
    float                              max_dark_pct;                              /* Maximum number of dark regions to be counted*/
} aec_dark_region_adapter;

/**
* This structure defines the AEC extreme color tuning parameters
*/
typedef struct
{
    float                             adj_ratio[Q3A_AEC_BAYER_CHNL_MAX];         /* adjust ratios for all channels*/
} aec_extreme_color_triggered_adapter;

typedef struct
{
    int                                  extreme_color_enable;                   /* Flag to enable\disable the extreme colour feature*/
    int                                  num_zones;                              /* Number of zones actually used*/
    aec_lux_trigger_adapter              triggers[MAX_AEC_TRIGGER_ZONE - 1];       /* Trigger point to enter the next zone*/
    aec_extreme_color_triggered_adapter  triggered_params[MAX_AEC_TRIGGER_ZONE]; /* Parameters that can be tuned per-zone*/

    float                                red_th;                                 /* Extreme red threshold of r/g ratio*/
    float                                red_interp_pct;                         /* Extreme red thresh interpolation range*/
    float                                green_rg_th;                            /* Extreme green threshold of r/g ratio*/
    float                                green_rg_interp_pct;                    /* Extreme green r/g thresh interpolation range*/
    float                                green_bg_th;                            /* Extreme green threshold of b/g ratio*/
    float                                green_bg_interp_pct;                    /* Extreme green b/g thresh interpolation range*/
    float                                green_cntr_th;                          /* Extreme green thresh for the distance to (1,1) on (r/g)&(b/g) domain*/
    float                                green_cntr_interp_pct;                  /* Extreme green center thresh interpolation range*/
    float                                blue_th;                                /* Threshold for extreme blue detection*/
    float                                blue_interp_pct;                        /* Extreme blue thresh interpolation range*/

    float                                luma_high_th;                           /* High threshold for region luma*/
    float                                luma_low_th;                            /* Low threshold for region luma*/
    float                                color_stats_pct_high_th;                /* High threshold for percentage of extreme color stats*/
    float                                color_stats_pct_low_th;                 /* Low threshold for percentage of extreme color stats*/

    /* Extended Parameters */
    float                                dbg_print_period;                       /* Debug message print period*/
} aec_extreme_color_adapter;

typedef struct
{
    /* Flat scene detection tuning */
    int     hist_flat_detector_enable;          /* Flag to enable\disable the histogram based flat scene detection feature*/
    int     start_level;                        /* Starting point for the histogram peak detection*/
    int     end_level;                          /* Ending point for the histogram peak detection*/
    int     range;                              /* Range for the histogram peak detection*/
    float   delta_th_low;                       /* Low threshold value for peak detection*/
    float   delta_th_high;                      /* High threshold value for peak detection*/
    float   bright_flat_det_th;                 /* Threshold value for bright flat scene detection*/
    float   dark_flat_det_th;                   /* Threshold value for dark flat scene detection*/
    float   bright_flat_tail_det;               /* Threshold value for tail detection for bright flat scenes*/
    float   dark_flat_tail_det;                 /* Threshold value for tail detection for dark flat scenes
     * Flat scene compensation tuning */
    int     bright_flat_compensation_enable;   /* Flag to enable\disable the bright flat scene compensation*/
    int     dark_flat_compensation_enable;     /* Flag to enable\disable the dark flat scene compensation*/
    float   flat_white_grey_vs_nongrey_th;     /* Threshold to determine if flat area is brighter than colored regions*/
    float   flat_dark_grey_vs_nongrey_th;      /* Threshold to determine if flat area is darker than colored regions*/
    float   near_grey_tolerance;               /* Regions with R/G and B/G within the tolerance percentage are considered grey regions*/
    float   bright_flat_compensation_ratio;    /* Ratio for the bright flat scene compensation*/
    float   dark_flat_compensation_ratio;      /* Ratio for the dark flat scene compensation*/
    int     flat_indoor_disable_start_idx;     /* Above this index, flat compensation start to get disabled*/
    int     flat_indoor_disable_end_idx;       /* Above this index, flat compensation fully disabled*/
} aec_flat_scene_adapter;

/******************************************************************************
** Variable name: start
** Start percentage for the range of pixels used to calculate
** either saturation position, or darkness position
** Default Value: 0.01
** Data range: [0 1]
**
** Variable name: end
** End percentage for the range of pixels used to calculate
** either saturation position, or darkness position
** Default Value: 0.04
** Data range: [0 1]
******************************************************************************/
typedef struct
{
    float start;
    float end;
} aec_hist_pct_range_adapter;

/******************************************************************************
** Variable name: hist_sat_pushup_range
** The saturated percentage range for push-up compensation (flat/low dynamic range scenes).
** It is recommended to use a bigger range for push-up for stability reasons.
** Default Value: {{0.02, 0.08}, {0.02, 0.08}, {0.02, 0.08}}
**
** Variable name: hist_sat_pushdown_range
** The saturated percentage range for push-down compensation (high dynamic range/overexposed scenes).
** It is recommended to use a smaller range for push-down, so that it can more effectively
** prevent smaller area of saturation.
** Default Value: {{0.01, 0.04}, {0.01, 0.04}, {0.01, 0.04}}
**
** Variable name: hist_dark range
** The dark area percentage range used to calculate darkness position and affect luma target.
** It is generally not recommended to use this particular part due to stability concerns.
** Default Value: {{0, 0.15}, {0, 0.15}, {0, 0.15}}
**
** Variable name: max_drc_gain
** Maximal DRC gain allowed for the particular lux zone. This variable is only valid when
** adrc mode is not set to disable.
** Default Value: {4.0, 4.0, 4.0}
** Range: [1.0, 8.0]
**
** Variable name: adrc_conv_speed
** Defines the convergence speed for ADRC tradeoff. The bigger, ADRC converges faster.
** If it is small, then the highlight may take more time to show up, while if it is too big,
** then there could be stability issues.
** No need to tune normally.
** Default Value: {0.15, 0.15, 0.15}
** Range: (0, 0.5]
**
** Variable name: gtm_percentage
** Percentage of current DRC gain to be applied to GTM. This is only applicable if chipset has
** GTM module support.
** Default Value: {0.5, 0.5, 0.5}
** Range: [0, 1]
**
** Variable name: ltm_percentage
** Percentage of current DRC gain to be applied to LTM. This is only applicable if chipset has
** LTM module support.
** Default Value: {0.5, 0.5, 0.5}
** Range: [0, 1]
**
** Variable name: la_percentage
** Percentage of current DRC gain to be applied to GTM. This is only applicable if chipset has
** LA module support.
** Default Value: {0,0,0}
** Range: [0, 1]
*****************************************************************************
 **
* This structure defines the histogram based AEC metering parameters
* default & range values described in aec_hist_metering_adapter.
*/
typedef struct
{
    float                       max_target_adjust_ratio;                         /* Maximum target adjust allowed*/
    float                       min_target_adjust_ratio;                         /* Minimum target adjust allowed*/
    float                       sat_low_ref;                                     /* Low reference/control point for saturation*/
    float                       sat_high_ref;                                    /* High reference/control point for saturation*/
    float                       dark_low_ref;                                    /* Low reference/control point for darkness*/
    float                       dark_high_ref;                                   /* High reference/control point for darkness*/
    aec_hist_pct_range_adapter  hist_sat_pushup_range[Q3A_AEC_BAYER_CHNL_MAX];   /* Percentage of bright pixels used in determine saturation*/
    aec_hist_pct_range_adapter  hist_sat_pushdown_range[Q3A_AEC_BAYER_CHNL_MAX]; /* Percentage of bright pixels used in determine saturation*/
    aec_hist_pct_range_adapter  hist_dark_range[Q3A_AEC_BAYER_CHNL_MAX];         /* Percentage of dark pixels used to determine darkness*/
    float                       max_drc_gain;                                    /* Maximal DRC gain allowed*/
    float                       adrc_conv_speed;                                 /* ADRC tradeoff convergence speed*/
    float                       gtm_percentage;                                  /* Percentage of total current DRC gain to be allocated to GTM*/
    float                       ltm_percentage;                                  /* Percentage of total current DRC gain to be allocated to LTM*/
    float                       la_percentage;                                   /* Percentage of total current DRC gain to be allocated to LA*/
    float                       gamma_percentage;                                /* Percentage of total current DRC gain to be allocated to Gamma*/
} aec_hist_metering_triggered_adapter;

typedef struct
{
    boolean                         adrc_antibanding_priority_enable;
    int                             atb_cutoff_idx_50hz;
    int                             atb_cutoff_idx_60hz;
    aec_lux_trigger_adapter         atb_trigger;
} adrc_atb_ctrl_tuning_adapter;

/******************************************************************************
** Variable name: adrc_enable
** ADRC is combined with histogram AEC for tuning interface structuring. Set this flag to
** either disable or enable ADRC. This replaced the "mode" parameter in previous header
** revision. GTM/LTM/LA are independently controlled via percentage variables, and hence
** there is no need to distinguish local or global modes.
** Default Value: 0
** Range: 0 and 1
**
** Variable name: dark_end_control_enable
** Dark end part of the histogram AEC is sometimes unstable (hist_dark_range etc.), and hence
** it can be disabled with this flag. It'll save computation if it is not used and disabled.
** Default Value: 0
** Range: 0 and 1
**
******************************************************************************/
typedef struct
{
    int                                   hist_metering_enable;                       /* Flag to enable histogram based AEC metering*/
    int                                   adrc_enable;                                /* Enable or disable ADRC*/
    int                                   dark_end_control_enable;                    /* This parameter will be used to disable dark end control*/
    int                                   num_zones;                                  /* Number of zones actually used*/
    aec_lux_trigger_adapter               triggers[MAX_AEC_TRIGGER_ZONE - 1];           /* Trigger point to enter the next zone*/
    aec_hist_metering_triggered_adapter   triggered_params[MAX_AEC_TRIGGER_ZONE];     /* Parameters that can be tuned per-zone*/
    float                                 target_filter_factor;                       /* Temporal filter for stability*/

    /* Extended Paramters */
    adrc_atb_ctrl_tuning_adapter          adrc_atb_ctrl;
    boolean                               gtm_enable;
    boolean                               ltm_enable;
    boolean                               la_enable;
    boolean                               gamma_enable;
    boolean                               dark_pushup_enable;
    boolean                               clamp_sat_pos_enable;

    /* Touch ROI or Dynamic ROI Metering for AEC
       Using Reserved params of AEC HDR */
    boolean touch_roi_hist_adjust_enable; /* reserve[0] of AEC HDR*/
    boolean face_roi_hist_adjust_enable;  /* reserve[1] of AEC HDR*/

    float   dark_roi_max_target_adjust_ratio[MAX_AEC_TRIGGER_ZONE]; /* reserve[2] of AEC HDR*/
    float   dark_roi_min_target_adjust_ratio[MAX_AEC_TRIGGER_ZONE]; /* reserve[3] of AEC HDR*/
    float   dark_roi_dark_low_ref[MAX_AEC_TRIGGER_ZONE];            /* reserve[4] of AEC HDR*/
    float   dark_roi_sat_high_ref[MAX_AEC_TRIGGER_ZONE];            /* reserve[5] of AEC HDR*/

    float   bright_roi_max_target_adjust_ratio[MAX_AEC_TRIGGER_ZONE]; /* reserve[6] of AEC HDR*/
    float   bright_roi_min_target_adjust_ratio[MAX_AEC_TRIGGER_ZONE]; /* reserve[7] of AEC HDR*/
    float   bright_roi_dark_low_ref[MAX_AEC_TRIGGER_ZONE];            /* reserve[8] of AEC HDR*/
    float   bright_roi_sat_high_ref[MAX_AEC_TRIGGER_ZONE];            /* reserve[9] of AEC HDR*/

    float   dark_roi_luma_ratio_start[MAX_AEC_TRIGGER_ZONE];          /* reserve[10] of AEC HDR*/
    float   dark_roi_luma_ratio_end[MAX_AEC_TRIGGER_ZONE];            /* reserve[11] of AEC HDR*/
    float   bright_roi_luma_ratio_start[MAX_AEC_TRIGGER_ZONE];        /* reserve[12] of AEC HDR*/
    float   bright_roi_luma_ratio_end[MAX_AEC_TRIGGER_ZONE];          /* reserve[13] of AEC HDR*/
} aec_hist_metering_adapter;

/******************************************************************************
** Feature name: Histogram Stretch feature.
** Applicable operation mode:  Snapshot, Preview and Camcorder.
** When this feature is enabled, 3A will send dark clamp to be subtracted to
** VFE ACE (Color Conversion) module per frame, ACE driver will compute the new
** RGB2Y coefficients based on strech gain computed.
**
** Variable name: dark_percentage
** Histogram counts will be accumulated from Pixel Value 0 and increasing,when
** the occurrence percentage is greater than or equal to dark_percentage, the
** corresponding pixel K0 is used as offset. K0 is subtracted in VFE Color conversion
** ( RGB to Y ) process after clamping with max_dark_clamp
** Default value: {0.005, 0.005, 0.005}
** Range: [0.001, 0.05]
**
** Variable name: max_dark_clamp
** RGB to Y offset programmed with minimum of (max_dark_clamp,K0)
** Default value: {3, 3, 3}
** Range: 0 to 6
**
******************************************************************************/
typedef struct
{
    float       dark_percentage;
    int         max_dark_clamp;
} aec_Y_hist_stretch_triggered_adapter;

/******************************************************************************
** Variable name: enable
** Enable histogram stretch feature to be applied in ACE.
** Default value: 0
** Range: 0 and 1
**
** Variable name: num_zones
** Number of lux trigger zones used for bright region tuning.
** Default value: 3
** Range: 1 to 6
**
** Variable name: triggers
** Defines each lux trigger to be used for separating the zones.
**   Number of triggers should be 1 less than num_zones, and it is arranged in
**   sequence to align with the zones. For example, if we have 3 zones (zone_1,
**   zone_2 & zone_3), then triggers should contain in sequence, trigger for
**   zone_1 to zone_2, and trigger for zone_2 to zone_3.
** Default value: Same as luma target triggers
******************************************************************************/
typedef struct
{
    int                                    enable;                                 /* Enable histogram stretch feature in ACE*/
    int                                    num_zones;                              /* Number of zones actually used*/
    aec_lux_trigger_adapter                triggers[MAX_AEC_TRIGGER_ZONE - 1];       /* Trigger point to enter the next zone*/
    aec_Y_hist_stretch_triggered_adapter   triggered_params[MAX_AEC_TRIGGER_ZONE]; /* Parameters that can be tuned per-zone*/
} aec_Y_hist_stretch_adapter;

typedef struct
{
    aec_lux_trigger_adapter  hdr_indoor_trigger;            /* For toggling exposure ratio*/
    float maxIdealHDRgain;                                  /* Making max HDR gain as configurable*/
} aec_hdr_tuning_adapter;

/******************************************************************************
** Feature name: Fast AEC convergence logic
** Description: This structure includes the tuning parameters for the fast AEC convergence logic
** Applicable operation modes: Camera start, camera switch (RFC <-> FFC), switch
** between preview and camcorder mode and vice versa
**
** Variable name: start_index
** The initial exposure index when sensor starts streaming in fast AEC mode
** Default value: 240.
** Data range: 0 to 500.
**
** Variable name: frame_skip
** This parameter specifies the number of frames to skip between each exposure update
** in the fast AEC mode. A higher value indicates that more frames are skipped between
** updates and a higher value is used when it takes a long time for the previous exposure
** update to be reflected in the stats
** Default: 3
** Range : 0 to 10
**
** Variable name: luma_settled_count
** This parameter specifies the number of successive frames where AEC is settled before the 'settled'
** flag  can be set to TRUE in the fast AEC mode. A higher value ensures accuracy at the expense
** of latency. A smaller value could result in some discrepancy observed in back-to-back runs
** Default value: 2
** Range: 0 to 3
******************************************************************************/
typedef struct
{
    unsigned int    start_index;           /* Exposure index to be used upon start of fast AEC*/
    int             frame_skip;            /* Number of frames skipped between each exposure update during fast AEC*/
    unsigned short  luma_settled_count;    /* Number of successive frames for AEC to be flag as 'settled'*/
} aec_fast_aec_adapter;

//=============================================================================
// Feature name : IR LED Adjust
// Description: This structure includes the tuning parameters for the IR LED
// control in IR mode
//
// Variable name: ir_led_enable
// This parameter specifies the IR LED is enabled for IR mode
// Default value: 1.
// Data range: 0 to 1.
//
// Variable name: ir_led_intensity_unit
// IR LED intensity unit
// Default value: 1.
// Data range: 0 to 5.
//
// Variable name: ir_led_intensity_idx_max
// Maximum IR LED Index
// Default value: 100.
// Data range: 0 to 100.
//
// Variable name: ir_led_intensity_idx_min
// Minimum IR LED Index
// Default value: 0.
// Data range: 0 to 100.
//=============================================================================
typedef struct
{
    int     ir_led_enable;
    float   ir_led_intensity_unit;
    int     ir_led_intensity_idx_max;
    int     ir_led_intensity_idx_min;
} ir_led_adjust_adapter;

//=============================================================================
// Feature name : IR ALS(Ambient Light Sensor) Adjust
// Description: This structure includes the tuning parameters for the IR ALS
// control in IR mode
//
// Variable name: ir_lightsensor_enable
// This parameter specifies ALS is enabled for IR mode
// Default value: 1.
// Data range: 0 to 1.
//
// Variable name: day2ir_switch_pt
// IR LED intensity unit
// Default value: TBD.
// Data range: TBD.
//
// Variable name: ir2day_switch_pt
// Maximum IR LED Index
// Default value: TBD.
// Data range: TBD.
//=============================================================================
typedef struct
{
    int     ir_lightsensor_enable;
    float   day2ir_switch_pt;
    float   ir2day_switch_pt;
} ir_als_adjust_adapter;

//=============================================================================
// Feature name : IR Daylight distribution adjust tuning
// Description: This structure includes the tuning parameters for the
// Day light(Normal light) distribution for IR mode.
//
// Variable name: normal_light_cnt_thres
// Count threshold of detected normal light
// Default value: TBD.
// Data range: TBD.
//
// Variable name: normal_light_target_thres
// Target_G threshold of detected normal light
// Default value: TBD.
// Data range: TBD.
//
// Variable name: normal_light_GB_ratio_max
// Maximum GB ratio to detect normal light
// Default value: TBD.
// Data range: TBD.
//
// Variable name: normal_light_GB_ratio_min
// Minimum GB ratio to detect normal light
// Default value: TBD.
// Data range: TBD.
//
// Variable name: normal_light_GR_ratio_max
// Maximum GR ratio to detect normal light
// Default value: TBD.
// Data range: TBD.
//
// Variable name: normal_light_GR_ratio_min
// Minimum GR ratio to detect normal light
// Default value: TBD.
// Data range: TBD.
//
// Variable name: normal_light_RB_ratio_max
// Maximum RB ratio to detect normal light
// Default value: TBD.
// Data range: TBD.
//
// Variable name: normal_light_RB_ratio_min
// Minimum RB ratio to detect normal light
// Default value: TBD.
// Data range: TBD.
//
// Variable name: ir_distribution_weight[16][16]
// Weight for calculating normal mode distribution (16x16)
// Default value: TBD.
// Data range: TBD.
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
    float   ir_distribution_weight[IRMODE_WEIGHT_ROW_SIZE][IRMODE_WEIGHT_COL_SIZE];
} ir_daylight_distribution_adjust_adapter;

//=============================================================================
// Feature name : IR mode control
// Description: This structure includes the tuning parameters for the
// to control the ir mode feature.
//
// Variable name: ir_mode_enable
// Enable or Disable IR Mode feature
// Default value: 0.
// Data range: 0 to 1.
//
// Variable name: ir_center_weight[16][16]
// Weight for calculating luma of IR mode (16x16)
// Default value: TBD.
// Data range: TBD.
//
// Variable name: ir_highlight_suppress_weight[64]
// Weight for avoid object over-exposure (64)
// Default value: TBD.
// Data range: TBD.
//
// Variable name: ir_mode_conv_speed
// AEC converge speed for IR mode
// Default value: TBD.
// Data range: 0 to 1.
//
// Variable name: day2ir_exp_index
// Day to IR exp_index to trigger IR LED
// Default value: TBD.
// Data range: TBD.
//
// Variable name: ir2day_exp_index
// IR to Day exp_index to turn off IR LED
// Default value: TBD.
// Data range: TBD.
//
// Variable name: ir_mode_day2ir_check_cnt
// Stable count of Day to IR mode switching
// Default value: TBD.
// Data range: TBD.
//
// Variable name: ir_mode_ir2day_check_cnt
// Stable count of IR to Day mode switching
// Default value: TBD.
// Data range: TBD.
//
// Variable name: ir_mode_skip_check_cnt
// Next check period after IR/Day switching
// Default value: TBD.
// Data range: TBD.
//
// Variable name: ir_mode_aec_skip_cnt
// Skip frames of AEC after IR cut switched
// Default value: TBD.
// Data range: TBD.
//=============================================================================
typedef struct
{
  int                       ir_mode_enable;
  ir_led_adjust_adapter     ir_led_adj;
  ir_als_adjust_adapter     ir_als_adj;
  float                     ir_center_weight[IRMODE_WEIGHT_ROW_SIZE][IRMODE_WEIGHT_COL_SIZE];
  float                     ir_highlight_suppress_weight[IRMODE_HIGHLIGHT_SUPRESS_WIEGHT_SIZE];
  float                     ir_mode_conv_speed;
  int                       day2ir_exp_index;
  int                       ir2day_exp_index;
  int                       ir_mode_day2ir_check_cnt;
  int                       ir_mode_ir2day_check_cnt;
  int                       ir_mode_skip_check_cnt;
  int                       ir_mode_aec_skip_cnt;
  ir_daylight_distribution_adjust_adapter ir_daylight_dist_adj;
} aec_ir_mode_adapter;


//shdr

typedef struct
{
  float   exp_ratio;
  float   gtm_gamma;
  float   exp_change_up_th;
  float   exp_change_low_th;
  int     non_linear_low_th;
  int     non_linear_hi_th;
} aec_shdr_exp_lut_type_adapter;

typedef struct
{
  int lux_index;
  float k;
  float exp_ratio_up_limit;
  float exp_ratio_low_limit;
}aec_svhdr_lowlight_t_adapter;

typedef struct
{
  int          shdr_enable;
  int          num_lut_entries;
  int          num_lowlight_lut_entries;
  boolean      pre_Bhist_inverse_map;
  aec_shdr_exp_lut_type_adapter exp_lut[SHDR_EXP_LUT_ENTRIES];
  aec_svhdr_lowlight_t_adapter aec_svhdr_lowlight[SHDR_EXP_RATIO_LOWLIGHT_LUT_SIZE];
  float        temporal_filter1;
  float        temporal_filter2;
} aec_shdr_mode_adapter;

typedef struct
{
    aec_generic_tuning_adapter     aec_generic;                                     /* Generic AEC tuning parameters*/
    aec_exposure_table_adapter     aec_exposure_table;                              /* Exposure table*/
    aec_snapshot_exposure_adapter  aec_snapshot_lut;                                /* Snapshot look-up table (LUT) tuning parameters*/
    aec_luma_target_adapter        aec_luma_target;                                 /* Defines the various luma targets*/
    aec_metering_table_adapter     aec_metering_tables;                             /* Metering tables (center-weighted\spot-metering)*/
    aec_fast_conv_adapter          aec_fast_convergence;                            /* Fast convergence tuning parameters*/
    aec_slow_conv_adapter          aec_slow_convergence;                            /* Slow convergence tuning parameters*/
    aec_flash_tuning_adapter       aec_led_flash;                                   /* AEC LED flash tuning parameters*/
    aec_touch_adapter              aec_touch;                                       /* Touch AEC tuning parameters*/
    aec_face_priority_adapter      aec_face_detection;                              /* Face detection AEC tuning parameters*/
    aec_led_fd_adapter             aec_led_fd;                                      /* Face detection during preflash */
    aec_bright_region_adapter      aec_bright_region;                               /* Bright region tuning parameters*/
    aec_dark_region_adapter        aec_dark_region;                                 /* Dark region tuning parameters*/
    aec_extreme_color_adapter      aec_extreme_color;                               /* Extreme colour tuning parameters*/
    aec_flat_scene_adapter         aec_flat_scene;                                  /* Flat scene tuning parameters*/
    aec_hist_metering_adapter      aec_hist_metering;                               /* Histogram based metering tuning parameters*/
    aec_Y_hist_stretch_adapter     aec_Y_hist_stretch;                              /* Y histogram stretch feature in ACE*/
    aec_hdr_tuning_adapter         aec_hdr;                                         /* HDR AEC tuning parameters*/
    aec_motion_iso_adapter         aec_preview_motion_iso;                          /* Preview motion ISO tuning parameters*/
    aec_motion_iso_adapter         aec_snapshot_motion_iso;                         /* Snapshot motion ISO tuning parameters*/
    aec_fast_aec_adapter           aec_fast_aec;                                    /* Fast AEC convergence tuning parameters*/
    aec_ir_mode_adapter            aec_ir_mode;                                     /* IR mode tuning parameters */
    aec_shdr_mode_adapter          aec_shdr_mode;                                   /* SHDR mode tuning parameters */
} aec_algo_tuning_adapter;

/******************************************************************************
*******************************************************************************
**  AEC related params end
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
**  AWB related params start
*******************************************************************************
******************************************************************************/
#define AGW_NUMBER_GRID_POINT             241
#define MAX_LOW_LIGHT_AWB_LUT_SIZE        6
#define RESERVED_DEV_NUM                  30

/**
* Reference light definitions
**/
typedef enum
{
  Q3A_AWB_AGW_D65 = 0,                                /* D65*/
  Q3A_AWB_AGW_D75,                                    /* D75*/
  Q3A_AWB_AGW_A,                                      /* A*/
  Q3A_AWB_AGW_WARM_FLO,                               /* TL84*/
  Q3A_AWB_AGW_COLD_FLO,                               /* CW*/
  Q3A_AWB_AGW_HORIZON,                                /* H*/
  Q3A_AWB_AGW_D50,                                    /* D50*/
  Q3A_AWB_AGW_CUSTOM_FLO,                             /* CustFlo*/
  Q3A_AWB_AGW_NOON,                                   /* Noon*/
  Q3A_AWB_AGW_CUSTOM_DAYLIGHT,
  Q3A_AWB_AGW_CUSTOM_A,
  Q3A_AWB_AGW_U30,
  Q3A_AWB_AGW_CUSTOM_DAYLIGHT1,
  Q3A_AWB_AGW_CUSTOM_DAYLIGHT2,
  Q3A_AWB_AGW_CUSTOM_FLO1,
  Q3A_AWB_AGW_CUSTOM_FLO2,
  Q3A_AWB_AGW_MAX_LIGHT,
  Q3A_AWB_AGW_INVALID_LIGHT = Q3A_AWB_AGW_MAX_LIGHT,
  Q3A_AWB_DAY_LINE_1 = Q3A_AWB_AGW_MAX_LIGHT,
  Q3A_AWB_DAY_LINE_2,
  Q3A_AWB_FLINE,
  Q3A_AWB_A_LINE_1,
  Q3A_AWB_A_LINE_2,
  Q3A_AWB_AGW_HYBRID,                                 /* Daylight, only used for algorithm, not data*/
  Q3A_AWB_AGW_MAX_ALL_LIGHT = Q3A_AWB_AGW_HYBRID,   /* Don't count the hybrid*/
  Q3A_AWB_AGW_INVALID_ALL_LIGHT = Q3A_AWB_AGW_MAX_ALL_LIGHT
} chromatix_awb_all_light_adapter;

typedef enum
{
  Q3A_AWB_LOWLIGHT_A = 0,
  Q3A_AWB_LOWLIGHT_TL84,
  Q3A_AWB_LOWLIGHT_D65,
  Q3A_AWB_LOWLIGHT_LED,
  Q3A_AWB_LOWLIGHT_STROBE,
  Q3A_AWB_MAX_LOWLIGHT
} AWB_LOWLIGHT_adapter;


/******************************************************************************
** Type definition of AWB subzone
******************************************************************************/
typedef enum
{
  Q3A_AWB_DAY_D75 = 0,                /* Subzone above D75 reference point                                        (NOTE: D75 reference weight)*/
  Q3A_AWB_DAY_D75_D65_1,              /* 1st subzone between D75 and D65 reference points                         (NOTE: D75 reference weight)*/
  Q3A_AWB_DAY_D75_D65_2,              /* 2nd subzone between D75 and D65 reference points*/
  Q3A_AWB_DAY_D75_D65_3,              /* 3rd subzone between D75 and D65 reference points*/
  Q3A_AWB_DAY_D65,                    /* 4th subzone between D75 and D65 reference points                         (NOTE: d65 reference weight)*/
  Q3A_AWB_DAY_D65_D50_1,              /* 1st subzone between D65 and D50 reference points                         (NOTE: d65 reference weight)*/
  Q3A_AWB_DAY_D65_D50_2,              /* 2nd subzone between D65 and D50 reference points*/
  Q3A_AWB_DAY_D65_D50_3,              /* 3rd subzone between D65 and D50 reference points*/
  Q3A_AWB_DAY_D50,                    /* 4th subzone between D65 and D50 reference points                         (NOTE: d50 reference weight)*/
  Q3A_AWB_DAY_NOON_LINE_0,            /* Subzone above the shifted D65 reference point                            (NOTE: d65 reference weight)*/
  Q3A_AWB_DAY_NOON_LINE_1,            /* 1st subzone between the shifted D65 and the shifted D50 reference points (NOTE: d65 reference weight)*/
  Q3A_AWB_DAY_NOON_LINE_2,            /* 2nd subzone between the shifted D65 and the shifted D50 reference points*/
  Q3A_AWB_DAY_NOON_LINE_3,            /* 3rd subzone between the shifted D65 and the shifted D50 reference points (NOTE: Noon reference weight)*/
  Q3A_AWB_DAY_NOON_LINE_4,            /* 4th subzone between the shifted D65 and the shifted D50 reference points (NOTE: Noon reference weight)*/
  Q3A_AWB_DAY_D50_FLO_1,              /* 1st subzone between D50 and the center point of Flo line                 (NOTE: d50 reference weight)*/
  Q3A_AWB_DAY_D50_FLO_2,              /* 2nd subzone between D50 and the center point of Flo line*/
  Q3A_AWB_DAY_D50_FLO_3,              /* 3rd subzone between D50 and the center point of Flo line*/
  Q3A_AWB_DAY_D50_FLO_4,              /* 4th subzone between D50 and the center point of Flo line*/
  Q3A_AWB_F_TL84,                     /* 1sth subzone between CW and TL84 reference points                        (NOTE: TL84 reference weight)*/
  Q3A_AWB_F_TL84_CW_2,                /* 2nd subzone between CW and TL84 reference points*/
  Q3A_AWB_F_TL84_CW_3,                /* 3rd subzone between CW and TL84 reference points*/
  Q3A_AWB_F_CW,                       /* 4th subzone between CW and TL84 reference points                         (NOTE: CW referece weight)*/
  Q3A_AWB_F_FLO_A_1,                  /* 1st subzone between the center point of Flo line and A reference point*/
  Q3A_AWB_F_FLO_A_2,                  /* 2nd subzone between the center point of Flo line and A reference point*/
  Q3A_AWB_F_FLO_A_3,                  /* 3rd subzone between the center point of Flo line and A reference point*/
  Q3A_AWB_A,                          /* 4th subzone between the center point of Flo line and A reference point   (NOTE: A reference weight)*/
  Q3A_AWB_AH_A_H_1,                   /* 1st subzone between A and H reference points                             (NOTE: A reference weight)*/
  Q3A_AWB_AH_A_H_2,                   /* 2nd subzone between A and H reference points*/
  Q3A_AWB_AH_A_H_3,                   /* 3rd subzone between A and H reference points*/
  Q3A_AWB_H,                          /* 4th subzone between A and H reference points                             (NOTE: H reference weight)*/
  Q3A_AWB_CUSTOM_DAYLIGHT1,           /* Custom Day light 1                                                       (NOTE: Custom day reference weight)*/
  Q3A_AWB_CUSTOM_DAYLIGHT2,           /* Custom Day light 2                                                       (NOTE: Custom day1 reference weight)*/
  Q3A_AWB_CUSTOM_DAYLIGHT3,           /* Custom Day light 3                                                       (NOTE: Custom day2 reference weight)*/
  Q3A_AWB_CUSTOM_FLO1,                /* Custom F light 1                                                         (NOTE: Custom f reference weight)*/
  Q3A_AWB_CUSTOM_FLO2,                /* Custom F light 2                                                         (NOTE: Custom f1 reference weight)*/
  Q3A_AWB_CUSTOM_FLO3,                /* Custom F light 3                                                         (NOTE: Custom f2 reference weight)*/
  Q3A_AWB_CUSTOM_A,                   /* Custom A light                                                           (NOTE: Custom A reference weight)*/
  Q3A_AWB_U30,                        /* U30 light                                                                (NOTE: U30 reference weight)*/
  Q3A_AWB_SUBZONE_DECISION_INVALID,
  Q3A_AWB_AGW_SUBZONE_LIGHT = Q3A_AWB_SUBZONE_DECISION_INVALID
} awb_hybrid_decision_adapter;

/******************************************************************************
** Type definition of Indoor CCM
******************************************************************************/
typedef enum
{
  Q3A_AWB_CCM_INDOOR_D65 = 0, /* D65 CCM*/
  Q3A_AWB_CCM_INDOOR_D50,     /* D50 CCM*/
  Q3A_AWB_CCM_INDOOR_CW,      /* CW CCM*/
  Q3A_AWB_CCM_INDOOR_TL84,    /* TL84 CCM*/
  Q3A_AWB_CCM_INDOOR_A,       /* A CCM*/
  Q3A_AWB_CCM_INDOOR_H,       /* Horizon CCM*/
  Q3A_AWB_CCM_INDOOR_NUM,
} ccm_indoor_adapter;

/******************************************************************************
** Type definition of Outdoor CCM
******************************************************************************/
typedef enum
{
  Q3A_AWB_CCM_OUTDOOR_SHADE = 0,  /* Shade (around D75) CCM*/
  Q3A_AWB_CCM_OUTDOOR_SUNNY,      /* Suuny (daylight, D50) CCM*/
  Q3A_AWB_CCM_OUTDOOR_GREEN,      /* Green color CCM*/
  Q3A_AWB_CCM_OUTDOOR_SKY,        /* SKy color CCM*/
  Q3A_AWB_CCM_OUTDOOR_SKIN,       /* Skin color CCM*/
  Q3A_AWB_CCM_OUTDOOR_LOWLIGHT,   /* Low light CCM*/
  Q3A_AWB_CCM_OUTDOOR_NUM,
} ccm_outdoor_adapter;

/******************************************************************************
** Type definition of Flash (LED) CCM
******************************************************************************/
typedef enum
{
  Q3A_AWB_CCM_FLASH_LED1 = 0,     /*Flash CCM under full power for LED1*/
  Q3A_AWB_CCM_FLASH_LED2,         /*Flash CCM under full power for LED2*/
  Q3A_AWB_CCM_FLASH_NUM,
} ccm_flash_adapter;

/**
 * Defines the awb Customer Reference Points
 */
typedef struct {
  int      num_customer_refer_points;
  int      calibration_index[AWB_MAX_NUMBER_OF_CUSTOMER_REFERENCE_POINT];
  float    rg_table[AWB_MAX_NUMBER_OF_CUSTOMER_REFERENCE_POINT];
  float    bg_table[AWB_MAX_NUMBER_OF_CUSTOMER_REFERENCE_POINT];
}awb_custom_reference_point_info;

/******************************************************************************
** Feature name: AWB reference points.
**
** Variable name: RG_ratio.
** R/G ratio.
** Variable name: BG_ratio.
** B/G ratio.
******************************************************************************/
typedef struct
{
  float RG_ratio;                  /* RedG*/
  float BG_ratio;                  /* BlueG*/
} chromatix_awb_reference_adapter;


/* Manual white balance gains */
typedef struct
{
  /* 9-bit, Q7, unsigned */
  float r_gain;                           /* RGain*/

  /* 9-bit, Q7, unsigned */
  float g_gain;                           /* GGain*/

  /* 9-bit, Q7, unsigned */
  float b_gain;                           /* BGain*/
} chromatix_manual_white_balance_adapter;

/******************************************************************************
** Feature name: AWB Lowlight Adj LUT entry.
** Lowlight look up table entry
**
** Variable name: lux_index.
** Trigger exposure index for interpolation.
** Default value: 0.
** Data range: 0 to 1000.
**
** Variable name: green_rg_offset_adj.
** Offset value for moving right side of green zone.
** Default value: -100.
** Data range: -100 to 100.
**
** Variable name: green_bg_offset_adj.
** Offset value for moving top side of green zone.
** Default value: -100.
** Data range: -100 to 100.
**
** Variable name: outlier_dist_adj.
** Outlier distance adjustment.
** Default value: 0.
** Data range: 0 to 16.
******************************************************************************/
typedef struct
{
  int     lux_index;
  float   green_rg_offset_adj;
  float   green_bg_offset_adj;
  float   outlier_dist_adj;
} low_light_adj_adapter;

/******************************************************************************
** This type holds both luma index and gain based trigger points to be
** used in configuration of VFE blocks.
**
** Variable name: gain_start.
** Gain based trigger point.
**
** Variable name: gain_end.
** Gain based end point.
**
** Variable name: lux_index_start.
** Lux index based trigger point.
**
** Variable name: lux_index_end.
** Lux index based end point.
******************************************************************************/
typedef struct
{
  float   gain_start;
  float   gain_end;
  long    lux_index_start;
  long    lux_index_end;
} trigger_point_adapter;


typedef struct {
  boolean                enabled;
  int                    sync_mode;
  trigger_point_adapter  weight_trigger;
  float                  weight_fcm[4];
  float                  stats_dark_threshold;
  float                  stats_sat_threshold;
  float                  zoom_level_start;
  float                  zoom_level_end;
  float                  sync_convergence_speed;
  awb_custom_reference_point_info custom_reference_points;
  int                    switch_ref_mode;
} awb_dualcam_sync_adapter;


typedef unsigned char tuning_control_adapter;

/******************************************************************************
** Feature name: AWB Lowlight Adj LUT.
** Lowlight look up table
**
** Variable name: Enable
** Enable/disable to use low light LUT.
** Default value: 1 (enable).
** Data range: 0 to 1 (0: disable, 1: enable).
**
** Variable name: lut_entry
** array of Lowlight look up table entries.
******************************************************************************/
typedef struct
{
  unsigned char         enable;
  low_light_adj_adapter lut_entry[MAX_LOW_LIGHT_AWB_LUT_SIZE];
} awb_lowlight_adj_lut_adapter;

typedef struct
{
  float red_gain_adj;               /* RedAdj*/
  float blue_gain_adj;              /* BlueAdj*/
} chromatix_awb_gain_adj_adapter;

#define ADJ_GAIN_EXP_TRIGGER_NUM 5
/******************************************************************************
** Feature name: AWB Adjust Gain Tables
**
** Variable name: awb_interpolate_gain_adj_enable.
**
** Variable name: exp_index_bright_light_gain_adjust.
** Gain adjust table trigger point between bright and normal.
**
** Variable name: exp_index_lowlight_gain_adjust.
** Gain adjust table trigger point betewen normal and lowlight.
**
** Variable name: control_gain_adj_lowlight.
** Default value: 0 for lux_index -- remove later
**
** Variable name: gain_adj_brightlight.
** Bright light adjust gain table.
**
** Variable name: gain_adj_normallight.
** Normal light adjust gain table.
**
** Variable name: gain_adj_lowlight.
** Lowlight adjust gain table.
**
** Variable name: ext_adj_gain_enable
** Specify if extended adjust gain tables are enabled
** Default value:  0 - Disable, (1 - Enable )
**
** Variable name: exp_index_trigger
** Exposure trigger points for interpolating adjust gain
** Default value:  [outdoor_index -50, outdoor_index - 20]
**                 [outdoor_index, outdoor_index + 30]
**                 [indoor_index - 30, indoor_index]
**                 [indoor_index + 50, indoor_index + 80]
**                 [linealization lowlight gain start, linealization lowlight gain end]
** Data range: [0, max_exp_index]
**
** Variable name: cct_trigger
** CCT trigger point for interpolating adjust gain
** Default value: (7500, 7100, 6700, 6300, 5900, 5500, 5100, 4700, 4300, 4100, 3900, 3500, 3100, 2900, 2600, 2300)
** Data range: [2300, 7500]
**
** Variable name: gain_adj_brightlight_shade
** Adjust gains for Shade region in the bright light
** Default value: (1.0f, 1.0f) -- for all cases
** Data range: [0.0f, 3.0f]
**
** Variable name: gain_adj_normallight_low
** Adjust gains for lowlight in normal light condition
** Default value: (1.0f, 1.0f) -- for all cases
** Data range: [0.0f, 3.0f]
**
** Variable name: gain_adj_extreme_lowlight
** Adjust gains for very lowlight condition
** Default value: (1.0f, 1.0f) -- for all cases
** Data range: [0.0f, 3.0f]
**
** Variable name: resv_dev_float1 and resv_dev_float2
** Reserved parameters for future developments
**
** Variable name: wb_adj_gain_compute_type
** 0: 3 adjust gain table, 1: 6 adjust gain tabel, 2: triangle gain table
******************************************************************************/
typedef struct
{
  unsigned char                   awb_interpolate_gain_adj_enable;
  trigger_point_adapter           exp_index_bright_light_gain_adjust;
  trigger_point_adapter           exp_index_lowlight_gain_adjust;
  tuning_control_adapter          control_gain_adj_lowlight;

  chromatix_awb_gain_adj_adapter  gain_adj_brightlight[Q3A_AWB_AGW_MAX_LIGHT];
  chromatix_awb_gain_adj_adapter  gain_adj_normallight[Q3A_AWB_AGW_MAX_LIGHT];
  chromatix_awb_gain_adj_adapter  gain_adj_lowlight[Q3A_AWB_AGW_MAX_LIGHT];

  /* Extended Paramters */
  int                             ext_adj_gain_enable;
  trigger_point_adapter           exp_index_trigger[ADJ_GAIN_EXP_TRIGGER_NUM];
  int                             cct_trigger[Q3A_AWB_AGW_MAX_LIGHT][ADJ_GAIN_EXP_TRIGGER_NUM];
  chromatix_awb_gain_adj_adapter  gain_adj_brightlight_shade[Q3A_AWB_AGW_MAX_LIGHT];
  chromatix_awb_gain_adj_adapter  gain_adj_normallight_low[Q3A_AWB_AGW_MAX_LIGHT];
  chromatix_awb_gain_adj_adapter  gain_adj_extreme_lowlight[Q3A_AWB_AGW_MAX_LIGHT];
  float                           resv_dev_float1[RESERVED_DEV_NUM];
  float                           resv_dev_float2[RESERVED_DEV_NUM];

  /*Additional parameters to remove reserved_data from older chromatix*/
  int                             wb_adj_gain_compute_type; /**/
} awb_adjust_gain_table_adapter;

/******************************************************************************
** Variable name: awb_day_cluster_left_outlier_distance.
** The outlier distance for left side of the day light line.
** Default value: 16.
** Data range: 8 to 100.
** Constraints: It can vary from 8 to 100 for the grid system of 241 grids.
** Effect: Smaller value reduces the outlier distance for the left side of the day light line.
**
** Variable name: awb_day_cluster_top_outlier_distance.
** The outlier distance for top of day light zone.
** Default value: 16.
** Data range: 8 to 100.
** Constraints: It can vary from 8 to 100 for the grid system of 241 grids.
** Effect: Smaller value reduces the outlier distance for the top of the day light zone.
**
** Variable name: awb_a_h_cluster_top_outlier_distance.
** The outlier distance for left side of A and H light zones.
** Default value: 16.
** Data range: 8 to 100.
** Constraints: It can vary from 8 to 100 for the grid system of 241 grids.
** Effect: Smaller value reduces the outlier distance for the left side of A and H light zones
**
** Variable name: ignore_day_cluster_left_top_outlier_distance
** Unconditionally set day cluster left and top outlier distance as
** day cluster distance
** If it is activated(enabled), awb_day_cluster_left_outlier_distance and
** awb_day_cluster_top_outlier_distance are ignored.
** Default value: 0
** Data range: 0 (disable), 1(enable)
******************************************************************************/
typedef struct
{
  unsigned int    awb_day_cluster_left_outlier_distance;
  unsigned int    awb_day_cluster_top_outlier_distance;
  unsigned int    awb_a_h_cluster_left_outlier_distance;

  /* Extended Paramters */
  int             ignore_day_cluster_left_top_outlier_distance;
} awb_cluster_distance_adapter;

/******************************************************************************
** Feature name: Outlier Distance
**
** Variable name: outlier_distance.
** Outlier distance for all clusters, globally applied.
** Default value: 16.
** Data Range: 4 to 20.
**
** Variable name: outlier_distance_day.
** Outlier distance for the day cluseter.
** Default value: 16.
** Data Range: 4 to 20.
**
** Variable name: outlier_distance_f.
** Outlier distance for the F cluster.
** Default value: 16.
** Data Range: 4 to 20.
**
** Variable name: outlier_distance_a.
** Outlier distance for the A cluster.
** Default value: 16.
** Data Range: 4 to 20.
**
** Variable name: outlier_distance_h.
** Outlier distnace for the H cluster.
** Default value: 16.
** Data Range: 4 to 20.
**
** Variable name: outlier_distance_customday.
** Outlier distance for the custom day cluster.
** Default value: 16.
** Data Range: 4 to 20.
**
** Variable name: outlier_distance_customf.
** Outlier distance for the custom F cluster.
** Default value: 16.
** Data Range: 4 to 20.
******************************************************************************/
typedef struct
{
  int                           outlier_distance;
  int                           outlier_distance_day;
  int                           outlier_distance_f;
  int                           outlier_distance_a;
  int                           outlier_distance_h;
  int                           outlier_distance_customday;
  int                           outlier_distance_customf;
  awb_cluster_distance_adapter  special_cluster_distances;
} awb_outlier_distance_adapter;

/******************************************************************************
** Outlier distance for some clusters and one global outlier distance
** Default value: 16(all cases)
** Data range: 4-20
**
** Variable name: ext_outlier_distance_enable
** enable/disable extended outlier distance
** If this value is disable, only outlier_distance_normal is valid.
** Default: 0 - disable, (1 - enable)
** Data range: {0, 1}
**
** Variable name: outlier_distance_bright_exp_index
** Trigger exposure indexes between bright and normal distances
** Default value: [outdoor_index, outdoor_index+30]
** Data range: [0, max_exp_index]
**
** Variable name: outlier_distance_lowlight_exp_index
** Trigger exposure indexes between normal and lowlight distances
** Default value: [indoor_index + 70, indoor_index + 100]
** Data range: [0, 500]
**
** Variable name: outlier_distance_bright
** Outlier distances for bright condition, usually outdoor
** Default value: 16
** Data range: [8, 100]
**
** Variable name: outlier_distance_normal
** Outlier distances for normal condition, usually between outdoor and indoor
** Default value: 16
** Data range: [8, 100]
**
** Variable name: outlier_distance_lowlight
** Outlier distance for low light condition, usually indoor
** Default value: 16
** Data range: [8, 100]
**
******************************************************************************/
typedef struct{
  int                           ext_outlier_distance_enable;
  trigger_point_adapter         outlier_distance_bright_exp_index;
  trigger_point_adapter         outlier_distance_lowlight_exp_index;
  awb_outlier_distance_adapter  outlier_distance_bright;
  awb_outlier_distance_adapter  outlier_distance_normal;
  awb_outlier_distance_adapter  outlier_distance_lowlight;
} awb_outlier_distance_ext_adapter;

#define MAX_CCT_EST 3
/******************************************************************************
** Feature name: AWB Basic Tuning.
**
** Variable name: reference.
** AWB reference points used for auto WB calculation.
**
** Variable name: indoor_index.
** The exposure index indicates the indoor condition.
**
** Variable name: outdoor_index.
** The exposure index indicates the outdoor condition.
**
** Variable name: awb_min_gains.
** Minimum AWB gains.
**
** Variable name: awb_max_gains.
** Maximum AWB gains.
**
** Variable name: awb_self_cal_enable.
** Enable/Disable of self calibration.
**
** Variable name: awb_self_cal_adj_ratio_high.
** Variable name: awb_self_cal_adj_ratio_low.
**
** Variable name: AWB_lowlight_LUT.
** AWB lowlight look up table.
**
** Variable name: awb_adjust_gain_table.
** AWB adjust gain tables for bright/normal/low light conditions.
**
** Variable name: outlier_distances.
** The outlier distances for all(or individual) clusters.
**
** Variable name: outlier_distances_ext.
** The outlier Extended Outlier Distance.
**
** Variable name: enable_awb_basic_init_cct.
** Enable/disable initial_cct_est and initial_weight_for_cct_est array
** Default: 0
** Range: 0 - disable, 1 - enable
**
** Variable name: initial_cct_est.
** Specify initial cct estimation values
** Default: {4100, 4100, 4100}
** Range: {4100, 5000}
** Note: cct_ct1, cct_ct2, and awb_bayer_cct
**
** Variable name: initial_weight_for_cct_est.
** Specify initial weight for the initial cct estimation value
** Default: {1, 1, 1}
** Range: [0,10]
** Note: the last entry is not valid in the current version (x308)
******************************************************************************/
typedef struct
{
  chromatix_awb_reference_adapter           reference[Q3A_AWB_AGW_MAX_LIGHT];
  long                                      indoor_index;
  long                                      outdoor_index;

  chromatix_manual_white_balance_adapter    awb_min_gains;
  chromatix_manual_white_balance_adapter    awb_max_gains;

  int                                       awb_self_cal_enable;
  float                                     awb_self_cal_adj_ratio_high;
  float                                     awb_self_cal_adj_ratio_low;

  awb_lowlight_adj_lut_adapter              AWB_lowlight_LUT;
  awb_adjust_gain_table_adapter             awb_adjust_gain_table;
  awb_outlier_distance_adapter              outlier_distances;

  /* Extended Paramters */
  awb_outlier_distance_ext_adapter          outlier_distances_ext;
  int                                       enable_awb_basic_init_cct;
  int                                       initial_cct_est[MAX_CCT_EST];
  int                                       initial_weight_for_cct_est[MAX_CCT_EST];
  float                                     tuning_version;
  int                                       awb_simulator_en;
  int                                       requested_rgn_skip_pattern;
  int                                       sim_region_pix_cnt;
  int                                       fast_aec_forced_cnt;
} awb_basic_tuning_adapter;

#define WEIGHT_VECTOR_TABLE_SIZE    13

/******************************************************************************
** Feature name: AWB Weight Vector entry.
**
** Variable name: exposure_index.
** expousre value
** Default exposure values (calculated):
** [0, outdoor_index/4, outdoor_index/2, outdoor_index*3/4,
**  outdoor_index, (outdoor_index+inoutdoor_midpoint)/2,
** inoutdoor_midpoint, (inoutdoor_midpoint+indoor_index)/2,
** indoor_index, ((3*indoor_index+max_exp_index)/4),
** (indoor_index+max_exp_index)/2,  ((3*max_exp_index+indoor_index)/4),
** max_exp_index].
**
** Variable name: weight_array.
** 38 columns in each entry correspond to color temparature associated with subzones defined in awb_hybrid_decision_adapter.
** Data range: 0 to 10.
** Constraints: None.
** Effect: Higher values give higher weight to the stat at the exposure index in the corresponding range.
******************************************************************************/
typedef struct
{
  int exposure_index;
  int weight_array[Q3A_AWB_AGW_SUBZONE_LIGHT];
} awb_illuminant_weight_vector_adapter;

/******************************************************************************
** Feature name: AWB Weight Vector.
** Weight vector tables in terms of illuminant and distance.
**
** Variable name: weight_vector.
** Weight vector corresponding to different range of exposure index and
** different color temparature (subzones)
**   The actual table size is 38x13 (up to now).
**   38: total number of subzones.
**   13: total number of different exposure values.
**
** Variable name: distance_weight_table[241].
** Distance weight table.
** Default value: 1.
** Data range: 0 to 3888.
** Constraints: None
** Effect: This array is used to control the distance weighting.  The index to the LUT is the grid distance.
**         If the outlier distance is N, only the first N entries of this weight table is non-zero, and all other entries are 0’s.
**         With a Gaussian kernel, the stat closer to reference points are given higher weight than stat farther away.
**         With equal weight, all samples in the grey zone are treated equally.
******************************************************************************/
typedef struct
{
  awb_illuminant_weight_vector_adapter   weight_vector[WEIGHT_VECTOR_TABLE_SIZE];
  int                                    distance_weight_table[AGW_NUMBER_GRID_POINT];
} awb_weight_vector_adapter;

/******************************************************************************
** awb rectangle detect zone
**
** Variable name: rg_center.
** The center point of zone in horizontal axis.
** Default value: 0.0
** Data range: 0.0 to 2.0
**
** Variable name: bg_center.
** The center point of zone in vertical axis.
** Default value: 0.0
** Data range: 0.0 to 2.0
**
** Variable name: rg_radius_grid.
** The horizontal offset from the center point.
** Default value: 0.
** Data range: 0 to 241.
**
** Variable name: bg_radius_grid.
** The vertical offset from the center point.
** Default value: 0.
** Data range: 0 to 241.
******************************************************************************/
typedef struct
{
  float               rg_center;
  float               bg_center;
  int                 rg_radius_grid;
  int                 bg_radius_grid;

  /* Extended Parameter */
  int                 calibration_index;
} awb_detect_zone_adapter;

/******************************************************************************
** Additional green zones for handling other green colors (e.g., artificial green grass)
**
** Variable name: zone_control.
** Specify the control method of the corresponding misleading zone.
** Default Value: 0.
** Data range: 0 to 10
**
** Variable name: trigger_type.
** Specify trigger point for enabling the corresponding misleading zone.
** Default Value: 0.
** Data range: >= 0
**
** Variable name: control_direction.
** Specify the direction of zone changes.
** Default value: 0.
** Data range : 0 to 10.
**
** Variable name: detect_zone.
** Rectangle zone misleading color zone as follows:
**
** Variable name: calibrated_index
** Specify which calibration factor will apply for the corresponding misleading color zone
** Default value: 1
** Data range: 0 -- D65 (D50), 1 -- TL84, 2 -- A
**
** Variable name:  exp_index_trigger.
** This exposure index indicates if the current misleading color zone is used
**   or not under the current brightness value.
** Data range: >= 0.
**
** Variable name: enable_mlc_zone
** Enable/Disable misleading color zone
** Default value: 0
** Data range: 0 - Disable, 1 - Enable
******************************************************************************/
typedef struct
{
  int                     zone_control;
  int                     trigger_type;
  int                     control_direction;
  awb_detect_zone_adapter detect_zone;
  trigger_point_adapter   exp_index_trigger;

  /* Extended Parameter */
  int                     enable_mlc_zone;
} awb_misleading_zone_adapter;

/******************************************************************************
** Feature name: awb_green_struct
** Green tuning parameters structure type
**
** Variable name: green_proj
** Select green projection method
** Default: 0
** Range : 0 - projection on day lines, 1 - projection on one point
**
** Variable name: green_offset_bg.
** Green offset b/g for top part
** Default value: -100 (disable top part of the green zone before tuning).
** Data range: -100 to 100.
** Constraints:
** Effect:
**
** Variable name: green_rg_offset_top_H.
** Green offset r/g for top part of the green zone.
** Default value: -100 (disable top part of the green zone before tuning).
** Data range: -100 to 100.
** Constraints:
** Effect:
**
** Variable name: green_rg_offset_bottom_H.
** Green offset r/g for bottom part of the green zone.
** Default value: -100 (disable bottom part of the green zone before tuning).
** Data range: -100 to 100.
** Constraints:
** Effect:
**
** Variable name: slope_factor_m.
** The parameter to adjust the slope of the projection line from center of the green stats to the day light line.
** Default value: 1.5
** Data range:
** Constraints: None.
** Effect: The smaller the value, the smaller slope the projection line has. The resulted green scene images go towards the bluish green direction.
**         The larger the value, the larger slope the projection line has. The resulted green scene images go towards the yellowish green color direction.
**
** Variable name: slope_factor_m_shade.
** Slope factor used for green zone projection in shade condition.
** Default value: 1.5 (a must tune parameter for different sensors).
** Data range: 0 to 100.
** Constraints:
** Effect:
**
** Variable names: awb_cw_green_reject_threshold, awb_custom_fl_green_reject_threshold.
** The threshold used to screen out the possible green samples for indoor CW/custom flo conditions.
** Default value: 0 (disable)
** Data range: 0 to 255.
** Constraints:
** Effect: When a stat is close to CW/CF but its green value is less than this threshold,
**   the sample is counted as an outlier instead of a grey sample.
**
** Variable name: bright_green_percentage.
** Bright green percentage at outdoor index.
** Default value: 6.
** Data range: 0 to 100.
** Constraints: This is a pre-set threshold obtained by testing.
** Effect: Higher value extends the outdoor green range.
**
** Variable name: dark_green_percentage.
** Dark green percentage at indoor index.
** Default value: 12.
** Data range: 0 to 100.
** Constraints: This is a pre-set threshold obtained by testing.
** Effect: Indoor green range is extended or narrowed down.
**
** Variable name: green_enhance_low_pec
** low green percentage for running green enhancement
** TBD
**
** Variable name: green_enhance_high_pec
** high green percentage for running green enhancement
** TBD
**
** Variable name: green2f_outlier_dist2
** distance from CW reference point for rejecting green stats inside the grey zone
** Default: 0
** Range: [0, 16]
**
** Variable name: green_enhance_gain_adjust_high
** Adjust gain for green enhancement with high green percentage
**
** Variable name: green_zone_assist
** Additional green zones for handling other green colors (e.g., artificial green grass)
**
** Variable name: bright_green_exp_index
** green zone projection slope factor trigger point for normal scenes
** Range:[0, outdoor_index]
**
** Variable name: shade_green_exp_index
** green zone projection slope factor trigger point for shade scenes
** Range:[outdoor_index/2, inoutdoor_index]
**
** Variable name: exp_weight
** exposure based offset that triggers slope_factor_m_shade
** Default: 0
** Range: [0, 300]
**
** Variable name: green_proj_interp_enable
** Enable/disable green projection interpolation.
** If it is enabled, the green interpolation ratio will be computed and do interpolating weighted sample average
** and green target point with the computed interpolation ratio
** Default: 0 - disable
** Range: 1 - enable 0 - disable
**
** Variable name: green_proj_per_low;
** The offset percentage for determining the lowest green percentage threshold.
** No effect on interpolation ratio between weighed sample average and green projected target point
** Default: 2.0
** Range: [0,50]
** Constraints: green_proj_per_low <= green_proj_per_high
**
** Variable name: green_proj_per_high;
** The offset percentage for determining the highest green percentage threshold.
** The interpolation ratio will be computed
** the lowest green percentage threshold and the highest green percentage threshold
** Default: 20.0
** Range: [0,80]
** Constraints: green_proj_per_high >= green_proj_per_low
**              green_proj_per_high <= green_proj_per_max
**
** Variable name: green_proj_per_max;
** The offset percentage for determining the maximum green percentage threshold.
** The interpolation ratio will be computed  with the highest green percentage threshold,
** the maximum green percentage threshold, and the maximum interpolation ratio
** Default: 80.0
** Range: [50,100]
** Constraints: green_proj_per_max >= green_proj_per_high
**
** Variable name: green_proj_per_max_ratio;
** The maximum interpolation ratio.
** If the green percentage is larger than the maximum green percentage threshold,
** this ratio should be the interpolation ratio
** Default: 1.2
** Range: [1.0,2.0]
** Constraints:
**
** Variable name: green_proj_index_start;
** The minimum exposure index for determining the interpolation ratio.
** If the current exposure index is lower than this index, no changes on the interpolation ratio.
** Note that we change the interpolation ratio that has been already computed by green percentage thresholds
** Default: 210.0
** Range: [150,300]
** Constraints: green_proj_index_start >= outdoor_heuristic_exposure_index_trigger
**              green_proj_index_start <= green_proj_index_end
**
** Variable name: green_proj_index_end;
** The maximum exposure index for determining the interpolation ratio.
** If the current exposure index is between index_start and index_end, the interpolation ratio is modified.
** If the current exposure index is higher than index_end, the final interpolation ratio will be set to 0
** Default: 310.0
** Range: [200,350]
** Constraints: green_proj_index_end <= indoor_index
******************************************************************************/
typedef struct
{
  int                             green_proj;
  int                             green_offset_bg;
  int                             green_rg_offset_top_H;
  int                             green_rg_offset_bottom_H;
  float                           slope_factor_m;
  float                           slope_factor_m_shade;
  int                             awb_cw_green_reject_threshold;
  int                             awb_custom_fl_green_reject_threshold;
  int                             bright_green_percentage;
  int                             dark_green_percentage;
  int                             green_enhance_low_pec;
  int                             green_enhance_high_pec;
  int                             green2f_outlier_dist2;
  int                             outdoor_heu_green_D50_enable;
  int                             green_zone_A_bottom_enable;
  int                             green_zone_bottom_grid;
  int                             green_zone_left_offset_enable;
  int                             green_zone_left_offset;

  chromatix_awb_gain_adj_adapter  green_enhance_gain_adjust_high;
  awb_misleading_zone_adapter     green_zone_assist[GREEN_ZONE_ASSIST_NUM];

  trigger_point_adapter           bright_green_exp_index;
  trigger_point_adapter           shade_green_exp_index;

  int                             exp_weight;
  int                             green_proj_interp_enable;
  float                           green_proj_per_low;
  float                           green_proj_per_high;
  float                           green_proj_per_max;
  float                           green_proj_per_max_ratio;
  float                           green_proj_index_start;
  float                           green_proj_index_end;
} awb_green_struct_adapter;

/******************************************************************************
** Feature name: AWB Dark Threshold.
**
** Variable name: lowlight_measure_delta.
** Trigger point to use the low light threshold values (dark_(r/g/b)_lowlight_threshold).
** Default value: 0.
** Data range: 0 to  max_exp_index-indoor_exp_index.
**
** Variable name: dark_r_threshold.
** R stat ave below this value is rejected (8-bit domain).
** Default value: 1.
** Data range: 0 to 255.
** Constraints: This is a pre-set threshold for collecting certain level of the red stat.
** Effect: Lower values allow stats with lower r values to be collected.
**
** Variable name: dark_g_threshold.
** G stat ave below this value is rejected (8-bit domain).
** Default value: 1.
** Data range: 0 to 255.
** Constraints: This is a pre-set threshold for collecting certain level of the green stat.
** Effect: Lower values allow stats with lower g values to be collected.
**
** Variable name: dark_b_threshold.
** B stat ave below this value is rejected (8-bit domain).
** Default value: 1.
** Data range: 0 to 255.
** Constraints: This is a pre-set threshold for collecting certain level of the blue stat.
** Effect: Lower values allow stats with lower b values to be collected.
**
** Variable name: dark_r_threshold_lowlight.
** R stat ave below this value is rejected (8-bit domain).
** Default value: 1.
** Data range: 0 to 10.
** Constraints: This is a pre-set threshold for collecting certain level of the red stat.
** Effect: Lower values allow stats with lower r values to be collected.
**
** Variable name: dark_g_threshold_lowlight.
** G stat ave below this value is rejected (8-bit domain).
** Default value: 1.
** Data range: 0 to 10.
** Constraints: This is a pre-set threshold for collecting certain level of the green stat.
** Effect: Lower values allow stats with lower g values to be collected.
**
** Variable name: dark_b_threshold_lowlight.
** B stat ave below this value is rejected (8-bit domain).
** Default value: 1.
** Data range: 0 to 10.
** Constraints: This is a pre-set threshold for collecting certain level of the blue stat.
** Effect: Lower values allow stats with lower b values to be collected.
**
** Variable name: dark_threshold_exp_trigger
** Exposure trigger points for interpolating normal/lowlight dark threshold values in parsing stats
** Default value: (lux_index_start, lux_index_end)=(310, 350)
** Data range: [indoor exposure index, maximum exposure index]
******************************************************************************/
typedef struct
{
  int                   lowlight_measure_delta;

  int                   dark_r_threshold;
  int                   dark_g_threshold;
  int                   dark_b_threshold;

  int                   dark_r_threshold_lowlight;
  int                   dark_g_threshold_lowlight;
  int                   dark_b_threshold_lowlight;

  /* Extended Parameter */
  trigger_point_adapter dark_threshold_exp_trigger;
} awb_dark_threshold_adapter;

/******************************************************************************
** Feature name: AWB Temporal Tuning.
** Tuning parameters for Temporal Heuristic Stage.
**
** Variable name: awb_history_buffer_size.
** AWB history size.
** Default value: 30.
** Data range: 15 to 1000.
** Constraints:
** Effect: Used for temporal heuristic.
**
** Variable names: day_stability_enable, f_stability_enable, a_stability_enable, h_stability_enable.
** Flag to enable day/f/a/h stability by history.
** Default value: 1.
** Data range: 0 (disable) or 1 (enable).
** Constraints:
** Effect:
**
** Variable names: awb_lock_day_enable, awb_lock_f_enable, awb_lock_a_enable.
** Flags used to enable/disable lock/unlock for Day, F, and A light.
** Default value: 0 (disable).
** Data range: 0 or 1.
** Constraints:
** Effect:
**
** Variable names: lock_exp_threshold_day, lock_exp_threshold_f, lock_exp_threshold_a.
** Lock/unlock threshold for Day, F and A. Used in temporal heuristics.
** Default value: 25.
** Data range: 0 to 50.
** Constraints:
** Effect: The higher the value, the higher possibility to get locked to that light.
**
** Variable name: save_historyaverage2history.
** Flag to enable save history average as current frame's final AWB decision to history.
** Default value: 1.
** Data range: 0 (disable) or 1 (enable).
** Constraints:
** Effect: Enable the feature to have more smooth/stable AWB performance.
**         Disable the feature to have a faster AWB convergence process.
**
** Variable name: awb_convergence_factor_camera, awb_convergence_factor_video.
** AWB convergence speed is changed to floating point scaler instead of
**   the level 0, 1, and 2 for more accurate convergence speed control.
**   Camera and video need different convergence speed.
** Default value: 0.05
** Data range: 0 to 1.
** Constraints:
** Effect: Too fast convergence speed could cause color jitter.
**         Old convergence speed slow=0.05, medium=0.1, fast=0.3
******************************************************************************/
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
} awb_temporal_tuning_adapter;

/******************************************************************************
** Feature name: AWB Heuristic Control.
** Definition of grey and white weights for different conditions.
**
** Variable names: grey_weight_day, grey_weight_f, grey_weight_a, grey_weight_h.
** Weight for grey decision when grey decision is the same as white decision.
** Default value: 1.
** Data range: 0 to 10.
** Constraints: None.
** Effect: Increase the value to increase the weight of grey decision for the final AWB decision.
**
** Variable names: white_weight_day, white_weight_f, white_weight_a, white_weight_h.
** Weight for white decision when white decision is the same as grey decision.
** Default value: 1.
** Data range: 0 to 10.
** Constraints: None.
** Effect: Increase the value to increase the weight of white decision for the final AWB decision.
**
** Variable name: exposure_adjustment.
** Interpolation range adjustment on the boundary of two different AWB decisions.
** Default value: 5.
** Data range: 0 to 140.
** Constraints: None.
** Effect: This parameter adjusts the interpolation range on the boundary of two different AWB decisions based on exposure.
**         Higher values give a larger interpolation range which results in a smoother AWB decision change.
******************************************************************************/
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
} awb_heuristic_control_adapter;

#define HEURISTIC_WEIGHT_NUM 20

/******************************************************************************
** Feature name: AWB Heuristic Weight.
** Exposure and CCT weights for all heuristic functions.
**
** Variable name:exp_weight_low[2].
** Default value: {0.5, 0.5}.
** Data range: 0 to 1.
**
** Variable name:exp_weight_high[2].
** Default value: {0.5, 0.5}.
** Data range: 0 to 1.
**
** Variable name:cct_weight_low[2].
** Default value: {0.5, 0.5}.
** Data range: 0 to 1.
**
** Variable name:cct_weight_high[2].
** Default value: {0.5, 0.5}.
** Data range: 0 to 1.
******************************************************************************/
typedef struct
{
  float exp_weight_low[2];
  float exp_weight_high[2];
  float cct_weight_low[2];
  float cct_weight_high[2];
} awb_heuristic_weight_adapter;

/******************************************************************************
** Feature name: AWB White World.
**
** Variable name: white_world_decision_enable
** Enable/Disable white world decision
** Default value: 0 (disable)
** Data range: 0 - disable, 1- enable
**
** Variable name: white_world_lowlight_exp_offset
** Exposure index based trigger point to disable white world decision
** under very low light condition
** if current_exposure index > maximum_exposure_index- white_world_lowlight_exp_offset,
** White world decision is deactivated
** Default value: 30
** Data range: [0, max_exp_index]
**
** Variable name: white_stat_y_threshold_high.
** The Y threshold for a region to be a white stat.
** Default value: white_stat_y_threshold_high=245.
** Data range: 0 to 255.
** Constraints: None.
** Effect: The upper limit for collecting white stat.
**         Too high will involve saturated stat,
**         and too low may reduce white stat collection accuracy.
**
** Variable name: white_outlier_valid_ymax_ratio.
** Check if Y max in outliers is outlier_valid_ymax_ratio
**   times higher than the Y max in valid samples.
** Default value: 3.
** Data range: 1 to 10.
** Constraints: None.
** Effect: If outlier Y max is outlier_valid_ymax times more than the Y max in valid samples,
**         set white world decision to be -1. Higher value requires a higher threshold for this decision.
**
** Variable name: white_stat_cnt_threshold.
** The minimum white stat count required for making a white decision.
** Default value: 10.
** Data range: 0 to 10000.
** Constraints:
** Effect:
**
** Variable name: white_peak_separation_distance.
** The minimum distance required between the first Y peak and the second Y peak among day, f, a, h clusters.
** Default value: 5.
** Data range: 0  to 255.
** Constraints:
** Effect:
**
** Variable name: white_aec_stable_range_threshold.
** Lock unlock threshold for temporal history heuristic of white world.
** Default value: 5.
** Data range: 0  to 100.
** Constraints:
** Effect: The lower the value, the higher restriction for applying lock to white world history.
**
** Variable name: white_history_weight.
** Weight for history white decision.
** Default value: 0.5
** Data range: 0 to 1.
** Constraints:
** Effect: The higher the value, the higher the weight for history white decision.
**
** Variable name: white_current_weight.
** Weight for current white decision.
** Default: 0.5
** Range : 0 to 1.
** Effect: The same as white_history_weight.
******************************************************************************/
typedef struct
{
  int   white_stat_y_threshold_high;
  float white_outlier_valid_ymax_ratio;
  int   white_stat_cnt_threshold;
  int   white_peak_separation_distance;
  int   white_aec_stable_range_threshold;
  float white_history_weight;
  float white_current_weight;

  /* Extended Parameter */
  int   white_world_decision_enable;
  int   white_world_lowlight_exp_offset;
} awb_white_world_struct_adapter;

/******************************************************************************
** Feature name: AWB Decision Boundary.
**
** Variable name: a_h_line_boundary.
** Boundary of A and H decision zones.
** Default value: 0.5
** Data range: 0 to 1.
**
** Variable name: day_f_line_boundary.
** Boundary of Day and F deicsion zones.
** Default value: 0.5
** Data range: 0 to 1.
**
** Variable name: d75_d65_line_boundary.
** Boundary of D75 and D65 decision zones.
** Default value: 0.5
** Data range: 0 to 1.
**
** Variable name: d65_d50_line_boundary.
** Boundary of D65 and D50 decision zones.
** Default value: 0.5
** Data range: 0 to 1.
**
** Variable name: d65_d50_shifted_line_boundary.
** Boundary of shifted D65 and shifted D50 decision zones.
** Default value: 0.5
** Data range: 0 to 1.
**
** Variable name: f_a_line_boundary.
** Boudnary of F and A decision zones.
** Default value: 0.5
** Data range: 0 to 1.
**
** Variable name: cw_tl84_line_boundary.
** Boundary of CW and TL84 decision zones.
** Default value: 0.5
** Data range: 0 to 1.
**
** Variable name: d50_weighted_sample_boundary.
** Boundary for stat screening for D65 and D50 clusters.
** Default value: 1.0
** Data range: 0 to 1.
**
** Variable name: d65_weighted_sample_boundary.
** Boundary for stat screending for D75 and D65 clusters.
** Default value: 1.0
** Data range: 0 to 1.
**
** Variable name: h_cluster_cutoff_boundary
** Specify cutoff offset boundary for h cluster in all outlier case
** Default value: (0.1, 0.1)
** Data range : [0, 0.5]
******************************************************************************/
typedef struct
{
  float                           a_h_line_boundary;
  float                           day_f_line_boundary;
  float                           d75_d65_line_boundary;
  float                           d65_d50_line_boundary;
  float                           d65_d50_shifted_line_boundary;
  float                           f_a_line_boundary;
  float                           cw_tl84_line_boundary;
  float                           d50_weighted_sample_boundary;
  float                           d65_weighted_sample_boundary;

  /* Extended Parameter */
  chromatix_awb_reference_adapter h_cluster_cutoff_boundary;
} awb_decision_boundary_adapter;

/******************************************************************************
** Feature name: AWB Cluster Tuning.
**
** Variable name: dominant_cluster_enable.
** Turn on/off Dominant cluster functionality.
** Default: 1 (enable).
** Range : 0 (disable) or 1 (enalbe).
**
** Variable names: cluster_high_pec, cluster_mid_pec, cluster_low_pec.
** Cluster percentage to determine dominant light in heuristics.
** Default values: 0.40, 0.25, 0.15
** Data range: 0 to 1.
** Constraints: None.
** Effect: These three parameters set three boundaries to recognize the dominant cluster in heuristics.
**         Higher values require a larger number of cluster counts. For example, cluster_high_pec=0.25 requires
**         the corresponding cluster to have a 25 percentage of the total-cluster-counts to proceed to the
**         corresponding AWB decision making. While cluster_low_pec=0.1 only requires 10 percentage of the
**         total-cluster counts to proceed to the corresponding AWB decision making.
**
** Variable name: threshold_compact_cluster_valid.
** Radius of the compact cluster within grey zone.
** Default value: 4.
** Data range: 0 to 100.
** Constraints: None.
** Effect: Lower value requires that the recognized compact cluster have a smaller radius.
**
** Variable name: threshold_compact_cluster.
** Compact cluster threshold.
** Default value: 60.
** Data range: 0 to 1000.
** Constraints: None.
** Effect: This is the threshold of measured distance from outlier sample grid to the average.
**         If the measured maximum distance is lower than this threshold, a compact cluster is recognized.
**         Therefore, a too high threshold results in false-alarm compact cluster recognition,
**         and too low threshold may result in missed detection on the compact clusters.
**
** Variable name: compact_to_grey_dis.
** Outlier compact cluster distance to grey zone.
** Default value: 4.
** Data range: 0 to 100.
** Constraints: None
** Effect: Lower value requires the recognized compact cluster is
**         closer to the grey zone.
**
** Variable name: dominant_cluster_threshold.
** Dominant cluster threshold.
** Default value: 4.
** Data range: 0 to 1000.
** Constraints: None
** Effect: This threshold decides on if there is a dominant cluster or not.
**         Higher threshold gives higher requirement on the cluster counts
**         to be recognized as a dominant cluster.
******************************************************************************/
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
} awb_cluster_tuning_adapter;

/******************************************************************************
** Feature name: AWB Low Light Weight Warm Color Appearance.
**
** Variable name: warmup_enable_aa.
** Enable/disable weight_referece_aa.
** Default Value: 1.
** Data range: 0 (disable) or 1 (enable).
**
** Variable name: warmup_enable_hh.
** Enable/disable weight_referece_hh.
** Default Value: 1.
** Data range: 0 (disable) or 1 (enable).
**
** Variable name: weight_reference_aa.
** Weights for lowlight condition for warm color appearance (A light decision).
** Default value: 0.5
** Data range: 0 to 1.
**
** Variable name: weight_reference_ah.
** Weights for lowlight condition for warm color appearance (between A and H ).
** Default value: 0.5
** Data range: 0 to 1.
**
** Variable name: weight_reference_hh.
** Weights for lowlight condition for warm color appearance (H light decision).
** Default value: 0.5
** Data range: 0 to 1.
**
** Variable name: warmup_ah_interp_eanble
** Enable/Disable interpolation in warm color appearance
** Default value: 0
** Data range: 1 - Enable (0 - Disable)
******************************************************************************/
typedef struct
{
  int   warmup_enable_aa;
  int   warmup_enable_hh;
  float weight_reference_aa;
  float weight_reference_ah;
  float weight_reference_hh;
  int   warmup_ah_interp_enable;
} awb_weight_lowlight_warm_adapter;

/******************************************************************************
** Feature name: awb_trigger_type_float
** trigger point structure with float members
******************************************************************************/
typedef struct
{
  float fstart;
  float fend;
} awb_trigger_adapter_float;

/******************************************************************************
** Feature name: awb_trigger_type_int
** trigger point structure with integer members
******************************************************************************/
typedef struct
{
  int nstart;
  int nend;
} awb_trigger_adapter_int;

/*****************************************************************************
** 0x207  trigger pt for CCT
******************************************************************************/
typedef struct
{
  unsigned long CCT_start;
  unsigned long CCT_end;
} chromatix_CCT_trigger_adapter;

/*
Color Correction

|R|   |C0 C1 C2||R|   |K0|
|G| = |C3 C4 C5||G| + |K1|
|B|   |C6 C7 C8||B|   |K2|
*/
typedef struct
{
  /* 12-bit, {Q7,Q8,Q9,Q10}, signed
   * R */
  float           c0;                 /* RtoR*/
  float           c1;                 /* GtoR*/
  float           c2;                 /* BtoR*/

  /* G */
  float           c3;                 /* RtoG*/
  float           c4;                 /* GtoG*/
  float           c5;                 /* BtoG*/

  /* B */
  float           c6;                 /* RtoB*/
  float           c7;                 /* GtoB*/
  float           c8;                 /* BtoB*/

  /* 11-bit, Q0, signed */
  short           k0;                 /* ROffset*/

  /* 11-bit, Q0, signed */
  short           k1;                 /* GOffset*/

  /* 11-bit, Q0, signed */
  short           k2;                 /* BOffset*/

  /* range 0x0-0x3 maps to Q7-Q10 */
  unsigned char   q_factor;           /* QFactor*/
} chromatix_color_correction_adapter;

typedef struct
{
  int                                 enable;
  chromatix_color_correction_adapter  ccm;
} awb_extended_ccm_adapter;

/******************************************************************************
** Feature name: AWB driven CCM
**
** Variable name: awb_ccm_enable.
** Enable/Disable AWB driven CCMs. If it is set to be 0, the conventional ISP ccm is used.
** Defaut value: 1 (enable).
** Data range: 0 (disable) or 1 (enable).
**
** Variable name: awb_ccm_exp_index_trigger_outdoor.
** Trigger points for selecting either indoor or outdoor CCMs
** Default values: {0, 0, 180, 220}.
** Data range (calculated): outdoor_index/2 to inoutdoor_index.
**
** Variable name: awb_ccm_exp_index_trigger_lowlight.
** Trigger points for selecting either normal or lowlight CCMs.
** Default values: {0, 0, 350, 370}.
** Data range (calculated): >= (indoor_index+inoutdoor_index)/2.
**
** Variable name: awb_ccm_exp_index_trigger_LED_lowlight.
** Trigger points for selecting either normal or lowlight CCMs for LED.
** Default values: {0, 0, 350, 370}.
** Data range: >= (indoor_index+inoutdoor_index)/2.
**
** Variable name: awb_ccm_LED_trigger.
** Trigger points for selecting either ambient or LED CCMs (K sensitivity value).
** Default values: {10.0, 15.0}.
** Data range: 1.0 to 100.0
**
** Variable name:  awb_ccm_cct_trigger_outdoor
** Trigger points for selecting either shade or sunny CCMs.
** Default values: {6500, 7000}.
** Data range: 5000 to 7500.
**
** Variable name: awb_ccm_cct_trigger_indoor_day.
** Trigger points for selecting either indoor D65 or indoor D50 CCMs.
** Default values: {6100,6200}
** Range: [5000, 6500]
**
** Variable name:  awb_ccm_cct_trigger_indoor_dayf
** Trigger points for selecting either indoor D50 or indoor CW CCMS.
** Default values: {4300, 4600}.
** Data range: 4100 to 5000.
**
** Variable name: awb_ccm_cct_trigger_indoor_fa.
** Trigger points for selecting either indoor CW  or indoor A CCMS.
** Default values: {2700, 2850}.
** Data range: 2500, 4850.
**
** Variable name:  awb_ccm_cct_trigger_indoor_ah.
** Trigger points for selecting either indoor A or indoor H CCMs.
** Default values: {2300, 2400}.
** Data range: 2000 to 2850.
**
** AWB Extended CCMs as follows:
**
**   Variable name: enable.
**   Enable/Disable the corresponding CCM.
**   Default value: 0.
**   Data range: 0 (disable) or 1 (enable).
**
**   Variable name: CCM
**   CCM data
**   Default and range of each CCM element:
**   Default values: c0=1, c1=0, c2=0, c3=0, c4=1, c5=0, c6=0, c7=0, c8=1, k0=k1=k2=0.
**   Data ranges: 0.5 to 3 for each c, -2^10 to 2^10-1 for each k.
**
** Variable name: awb_ccm_indoor.
** Indoor CCMs.
**
** Variable name: awb_ccm_portrait.
** Portrait CCMS (triggering by face dections).
**
** Variable name: awb_ccm_lowlight.
** Lowlight CCMs (each lowlight CCM corresponds to each one in Indoor CCMs).
**
** Variable name: awb_ccm_outdoor.
** Outdoor CCM.
** Note: green CCM, sky CCM, and skin CCM are not used (reserved for future expansion).
**
** Variable name:  awb_ccm_flash.
** Flash CCMs (both single, pre-flash, and dual leds).
**
** Variable name:  awb_ccm_flash_lowlight.
** Flash lowlight CCMs.
******************************************************************************/
typedef struct
{
  int                            awb_ccm_enable;
  trigger_point_adapter          awb_ccm_exp_index_trigger_outdoor;
  trigger_point_adapter          awb_ccm_exp_index_trigger_lowlight;
  trigger_point_adapter          awb_ccm_exp_index_trigger_LED_lowlight;
  awb_trigger_adapter_float      awb_ccm_LED_trigger;
  chromatix_CCT_trigger_adapter  awb_ccm_cct_trigger_outdoor;
  chromatix_CCT_trigger_adapter  awb_ccm_cct_trigger_indoor_day;
  chromatix_CCT_trigger_adapter  awb_ccm_cct_trigger_indoor_dayf;
  chromatix_CCT_trigger_adapter  awb_ccm_cct_trigger_indoor_fa;
  chromatix_CCT_trigger_adapter  awb_ccm_cct_trigger_indoor_ah;
  awb_extended_ccm_adapter       awb_ccm_indoor[Q3A_AWB_CCM_INDOOR_NUM];
  awb_extended_ccm_adapter       awb_ccm_portrait[Q3A_AWB_CCM_INDOOR_NUM];
  awb_extended_ccm_adapter       awb_ccm_lowlight[Q3A_AWB_CCM_INDOOR_NUM];
  awb_extended_ccm_adapter       awb_ccm_outdoor[Q3A_AWB_CCM_OUTDOOR_NUM];
  awb_extended_ccm_adapter       awb_ccm_flash[Q3A_AWB_CCM_FLASH_NUM];
  awb_extended_ccm_adapter       awb_ccm_flash_lowlight[Q3A_AWB_CCM_FLASH_NUM];
} awb_CCM_adapter;

#define AWB_FLASH_ADJUST_LUT 5
/******************************************************************************
** Feature name: Pre-flash AWB Lookup table.
**
** Variable name: lux_index.
** Measured lux value.
** Data range: 0 to 1500.
**
** Variable name: flash_sensitivity_ratio.
** Flash sensitivity value correspondent to the above lux_index.
**
** Variable name: cct_threshold.
** Color temperature trigger points for interpolating adjust ratios.
**
** Variable name: adjust_ratio.
** Adjust rg/bg ratio for each condition specified by flash_sensitivity_ratio
** Default value: 0.015
** Data range : -0.02 to 0.02
******************************************************************************/
typedef struct
{
  int                            lux_index;
  float                          flash_sensitivity_ratio;
  chromatix_CCT_trigger_adapter  cct_threshold;
  chromatix_awb_gain_adj_adapter adjust_ratio;
} awb_flash_lut_info_adapter;

/******************************************************************************
** Feature name: AWB Pre-flash.
**
** Variable name: awb_pre_flash_enable.
** Enable/Disable the pre-flash process.
** Default value: 0.
** Data range: 0 (disable) or 1 (enable).
**
** Variable name: awb_flash_stat_sat_threshold.
** Saturation threshold for flash AWB.
** Default value: 75.
** Data range: 0 to 100.
**
** Variable name: awb_preflash_outlier_distance.
** Outlier distance for Pre-flash AWB.
** Default value: 4.
** Data range: 2 to 20.
**
** Variable name: awb_preflash_outlier_distance_no_flash.
** Outlier distance for AWB without flash light in led_WB_update.
** Default value: 8.
** Data range: 8 to 16.
**
** Variable name: awb_mainflash_typ_ref.
** Main Flash typical module reference points.
**
** Variable name: awb_preFlash_min_ref.
** Pre-flash min reference points.
**
** Variable name: awb_preFlash_max_ref.
** Pre-flash max reference points.
**
** Variable name: awb_mainflash_min_ref.
** Main flash min reference points.
**
** Variable name: awb_mainflash_max_ref.
** Main flash max reference points.
**
** Variable name: awb_flash_valid_sample_high_threshold.
** Upper threshold of valid sample in a flash scene.
** Data range: 0 to 3072.
**
** Variable name: awb_flash_valid_sample_low_threshold.
** Lower threshold of valid sample in a flash scene.
** Data range: 0 to 3072.
**
** Variable name: awb_flash_typ_decision_rg_offset.
** Valid rg ratio range for AWB decision with regard to TYP module.
** Default value: 0.015
** Data range: -0.02 to 0.02
**
** Variable name: awb_flash_typ_decision_bg_offset.
** Valid bg ratio range for AWB decision with regard to TYP module.
** Default value: 0.015
** Data range: -0.02 to 0.02
**
** Variable name: awb_flash_corner_sample_ref_adjust.
** Flash AWB corner module reference point adjust.
** Default value: 1.0
** Data range: 0.5 to 1.5
**
** Variable name: awb_flash_lut.
** Flash AWB adjust Look up table.
**
** Variable name: awb_preflash_enhance_enable
** Enable/Disable preflash enhancement process
** Default value: 0 - disable (1 - enable)
**
** Variable name: awb_preflash_luma_difference
** Threshold value for using initial rg/bg ratio and cct in dark condition
** Default value: 15
** Data range: [0, 30]
**
** Variable name: awb_preflash_init_decision
** Initial awb decision for preflash in dark condition
** Default value: D50 reference point
**
** Variable name: awb_preflash_init_cct
** Initial cct for preflash in dark condition
** Default value: 5000 (D50)
** Data range: [4100 5000]
******************************************************************************/
typedef struct
{
  int                             awb_pre_flash_enable;
  int                             awb_flash_stat_sat_threshold;
  int                             awb_preflash_outlier_distance;
  int                             awb_preflash_outlier_distance_no_flash;

  chromatix_awb_reference_adapter awb_mainflash_typ_ref;
  chromatix_awb_reference_adapter awb_preFlash_min_ref;
  chromatix_awb_reference_adapter awb_preFlash_max_ref;
  chromatix_awb_reference_adapter awb_mainflash_min_ref;
  chromatix_awb_reference_adapter awb_mainflash_max_ref;

  int                             awb_flash_valid_sample_high_threshold;
  int                             awb_flash_valid_sample_low_threshold;

  trigger_point_adapter           awb_flash_typ_decision_rg_offset;
  trigger_point_adapter           awb_flash_typ_decision_bg_offset;

  chromatix_awb_gain_adj_adapter  awb_flash_corner_sample_ref_adjust;
  awb_flash_lut_info_adapter      awb_flash_lut[AWB_FLASH_ADJUST_LUT];

  /* Extended Parameter */
  int                             awb_preflash_enhance_enable;
  int                             awb_preflash_luma_difference;
  chromatix_awb_reference_adapter awb_preflash_init_decision;
  float                           awb_preflash_init_cct;
} awb_preflash_adapter;

/******************************************************************************
** Feature name: Dual LED CCT Matching Flash.
** Applicable chipset(s): All VFE with 3A 4.1 and newer.
** Applicable operation mode:  Preview, snapshot, camcorder modes and ZSL.
**
** Variable name: dual_led_enable.
** Flag to enable/disable AWB Dual led feature. Only supported on 309.
** 310 onwards we are using AEC based dual led feature
** Default value: 0.
** Data range: 0 to 1.
** Constraints: None.
** Effect: 0 disables Dual LED CCT Matching, 1 enables Dual LED CCT Matching.
**
** Variable name: preflash_current_ratio.
** TBD .
**
** Variable name: continuous_burst_current_ratio.
** TBD.
**
** Variable name: led1_gain_adj.
** gain adjust for LED1 of dual LED.
**
** Variable name: led2_gain_adj.
** gain adjust for LED2 of dual LED.
**
** Variable name: table_size.
** CCT_control table size, which depends on the dual LED controller IC.
**
** Variable name: dual_led_intersect_slope.
** Slope of the dual LED line.
**
** Variable name: range_low.
** the valid low range of golden data.
**
** Variable name: range_high.
** the valid high range of golden data.
******************************************************************************/
typedef struct
{
  int                             dual_led_enable;
  float                           preflash_current_ratio;
  float                           continuous_burst_current_ratio;
  chromatix_awb_gain_adj_adapter  led1_gain_adj;
  chromatix_awb_gain_adj_adapter  led2_gain_adj;
  unsigned short                  table_size;
  float                           dual_led_intersect_slope;

  LED_mix_adapter                 CCT_control[MAX_LED_MIX_LEVEL];

  /*Additional parameters to remove reserved_data*/
  float                           range_low;
  float                           range_high;
} chromatix_match_LED_lighting_table_adapter;

/******************************************************************************
** Feature name:LED Tuning
** Variable name: dual_led_enable.
** Flag to enable/disable feature.
** Default value: 0.
** Data range: 0 to 1.
** Constraints: None.
** Effect: 0 disables Dual LED CCT Matching, 1 enables Dual LED CCT Matching.
******************************************************************************/
typedef struct
{
  int                                         dual_led_enable;
  float                                       awb_led_strobe_adjustment_factor;

  chromatix_awb_gain_adj_adapter              LED_gain_adj;
  chromatix_awb_gain_adj_adapter              strobe_gain_adj;

  /* Extended Parameter */
  chromatix_awb_gain_adj_adapter              dual_led_gain_adj[2];
  chromatix_match_LED_lighting_table_adapter  mix_LED_table; /*309*/
} awb_led_tuning_adapter;

#define DETECT_ZONE_NUM 8
#define OUTDOOR_AUX_COND_NUM 4

/******************************************************************************
** Variable name: detection_zone
** Rectangle zone for gathering stat information as follows:
**
** Variable name: count_trigger.
** Count based trigger points for setting up interpolation zone.
** Data range : 0 to 3072.
**
** Variable name:  exp_index_trigger.
** Exposure index based trigger points for setting up interploation zone.
** Data range: 0 to 500.
**
** Variable name: aux_cond[OUTDOOR_AUX_COND_NUM]
** Auxiliary conditions for applying adjust ratio
** Default value: -1.0f
** Data range: [0.0f, 3072.0f]
**
** Variable name: adjust_ratio.
** Adjust rg/bg gains.
** Data range: 0.5 to 1.5
**
** Variable name: exp_index_trigger_shade
** exposure index based trigger point for applying adjust gain for shade condition
** Default value: (outdoor_index, inout_midpoint_index)
** Data range: [outdoor_index, indoor_index]
**
** Variable name: adjust_ratio_shade
** Adjust rg/bg gains for shade condition
** Default value: 1.0
** Data range: [0.5, 1.5]
******************************************************************************/
typedef struct
{
  awb_detect_zone_adapter         detection_zone[DETECT_ZONE_NUM];
  awb_trigger_adapter_int         count_trigger;
  trigger_point_adapter           exp_index_trigger;
  float                           aux_cond[OUTDOOR_AUX_COND_NUM];
  chromatix_awb_gain_adj_adapter  adjust_ratio;

  /* Extended Parameter */
  trigger_point_adapter           exp_index_trigger_shade;
  chromatix_awb_gain_adj_adapter  adjust_ratio_shade;
} awb_outdoor_special_adapter;

/******************************************************************************
** Feature name: awb_extended_outdoor_exp_trigger_type
** Outdoor exposure indeces for triggering special outdoor heuristics
**
** Variable name: directsun_exp_index
** exposure index for very bright sunlight (under direct sun light)
** default: 140
** range: [90-170]
** 3A version : supported only in 306 & 307
**
** Variable name: outdoor_heuristic_exposure_index_trigger
** exposure index based trigger points for running outdoor heurisitic
** default: outdoor_index
** range: [0, outdoor_index]
**
** Variable name: outdoor_exp_bright_shade_enable
** Enable/disable outdoor heuristic bright and shade exposure trigger points
** Default: 0 - disable
** range: 1 - enable, 0 - disable
**
** Variable name: outdoor_heuristic_exp_index_bright
** Exposure index based trigger point for bright condition
** Default: 170
** Range: [0, outdoor_index]
**
** Variable name: outdoor_heuristic_exp_index_shade
** Exposure index based trigger point for shade condition
** Default: 200
** Range: [0, (outdoor_index+indoor_index)/2]
******************************************************************************/
typedef struct
{
  int directsun_exp_index;
  int outdoor_heuristic_exposure_index_trigger;
  int outdoor_exp_bright_shade_enable;
  int outdoor_heuristic_exp_index_bright;
  int outdoor_heuristic_exp_index_shade;
} awb_extended_outdoor_exp_trigger_adapter;

#define EXT_OUTDOOR_NORMAL_ZONE_SELECT_NUM 3
/*****************************************************************************
** Feature name: AWB Extended Outdoor Heuristics
**
** Variable name: awb_exposure_outdoor_heuristic
** Select the different outdoor heuristics.
** Default value: 0.
** Data range: 0 (basic) or 1 (advanced).
**
** Special Outdoor Heuristics.
** NOTE: This is a superset of parameters for all special outdoor heuristics.
**       Not all parameters below are used for every special outdoor heuristics.
**
** Variable name: awb_outdoor_green_adjust.
** Adjusts the special outdoor green heuristic parameters.
** Default values: {-1, -1, -1.0f, -1.0f, -1, -1, -1.0f, -1.0f, 1.0, 1.0}.
**
** Variable name: awb_outdoor_bright_blue_sky_adjust.
** Adjusts the special blue sky heuristic parameters in very bright scene.
** Default values: {-1, -1, -1.0f, -1.0f, -1, -1, -1.0f, -1.0f, 1.0, 1.0}.
**
** Variable name: awb_outdoor_not_enough_stat_adjust.
** Adjusts the special outdoor heuristic parameters with only very small stats.
** Default values: {-1, -1, -1.0f, -1.0f, -1, -1, -1.0f, -1.0f, 1.0, 1.0}.
**
** Variable name: awb_outdoor_blue_ground_adjust.
** Adjusts the special outdoor heuristic parameters for blue ground.
** Default values: {-1, -1, -1.0f, -1.0f, -1, -1, -1.0f, -1.0f, 1.0, 1.0}.
**
** Variable name: awb_outdoor_cloud_sky_adjust.
** Adjusts the special outdoor heuristic parameters for cloud sky.
** Default values: {-1, -1, -1.0f, -1.0f, -1, -1, -1.0f, -1.0f, 1.0, 1.0}.
**
** Variable name: awb_outdoor_blue_sky_adjust.
** Adjusts the special outdoor heuristic parameters for blue sky.
** Default values: {-1, -1, -1.0f, -1.0f, -1, -1, -1.0f, -1.0f, 1.0, 1.0}.
**
** Variable name: awb_outdoor_fog_sky_adjust.
** Adjusts the special outdoor heuristic parameters for smog sky.
** Default values: {-1, -1, -1.0f, -1.0f, -1, -1, -1.0f, -1.0f, 1.0, 1.0}.
**
** Variable name: awb_outdoor_green_detect_adjust.
** Adjusts the special outdoor heuristic parameters for green detect.
** Default values: {-1, -1, -1.0f, -1.0f, -1, -1, -1.0f, -1.0f, 1.0, 1.0}.
**
** Variable name: awb_outdoor_no_day_weighted_sample_adjust.
** Adjusts rg/bg gains when no valid day samples and no weighted samples.
** Default values: {-1, -1, -1.0f, -1.0f, -1, -1, -1.0f, -1.0f, 1.0, 1.0}.
**
** Variable name: awb_outdoor_all_out_of_zone_adjust.
** Adjusts rg/bg gains when stats are located out of grey zone.
** Default values: {-1, -1, -1.0f, -1.0f, -1, -1, -1.0f, -1.0f, 1.0, 1.0}.
**
** ---
**
** Variable name: awb_extended_outdoor_exp_trigger.
** Exposure indexes for various outdoor cases
** See the comments where the real values are specified
**
** Variable name: enable_zone_select
** Enable/Disable zone select array
** Default value: 0
** Data range: 0 - Disable, 1 - Enable
**
** Variable name: enable_artf_grass_adjust
** Enable/Disable artificial grass adjustment
** Default value: 0
** Data range: 0 - Disable, 1- Enable
**
** Variable name: awb_outdoor_normal_zone_select
** Specify subzones whose stat counts can contribute on stat sums for normal day condition
** Default value: { 63, 4080, 0}
** Data range: [0, 4095]
**
** Variable name: awb_outdoor_shade_zone_select
** Specify subzones whose stat counts can contribute on stat sums for shade condition
** Default value: { 3968,0,0}
** Data range: [0, 4095]
**
** Variable name: awb_outdoor_artificial_grass_adjust
** Default value:{-1.0f, -1.0f, -1, -1, 0,
**            0, 0,
**            -1.0f, -1.0f, 0, 0,
**            -1.0f, -1.0f,-1.0f, -1.0f,
**            1.0f, 1.0f,
**            -1.0f, -1.0f, 0, 0,
**            1.0f, 1.0f}
** Set the special outdoor heuristic parameters for artificial grass
******************************************************************************/
typedef struct
{
  int                                       awb_exposure_outdoor_heuristic;
  awb_outdoor_special_adapter               awb_outdoor_green_adjust;
  awb_outdoor_special_adapter               awb_outdoor_bright_blue_sky_adjust;
  awb_outdoor_special_adapter               awb_outdoor_not_enough_stat_adjust;
  awb_outdoor_special_adapter               awb_outdoor_blue_ground_adjust;
  awb_outdoor_special_adapter               awb_outdoor_cloud_sky_adjust;
  awb_outdoor_special_adapter               awb_outdoor_blue_sky_adjust;
  awb_outdoor_special_adapter               awb_outdoor_fog_sky_adjust;
  awb_outdoor_special_adapter               awb_outdoor_green_detect_adjust;

  chromatix_awb_gain_adj_adapter            awb_outdoor_no_day_weighted_sample_adjust;
  chromatix_awb_gain_adj_adapter            awb_outdoor_all_out_of_zone_adjust;

  awb_extended_outdoor_exp_trigger_adapter  awb_extended_outdoor_exp_trigger;

  /* Extended Parameter */
  int                                       enable_zone_select;
  int                                       enable_artf_grass_adjust;
  int                                       awb_outdoor_normal_zone_select[EXT_OUTDOOR_NORMAL_ZONE_SELECT_NUM];
  int                                       awb_outdoor_shade_zone_select[EXT_OUTDOOR_NORMAL_ZONE_SELECT_NUM];
  awb_outdoor_special_adapter               awb_outdoor_artificial_grass_adjust;
  float                                     resv_dev_float1[RESERVED_DEV_NUM];
  float                                     resv_dev_float2[RESERVED_DEV_NUM];
} awb_extended_outdoor_heuristic_adapter;

/******************************************************************************
** Variable name: detection_zone
** Rectangle zone for gathering stat information:
**
** Variable name: exp_index.
** Exposure index based trigger points for applying special color heuristic.
** Default values: {0.0, 0.0, 0, 0}.
**
** Variable name: count_offset.
** Number of stats that are classified as a corresponding special color.
** Default values: {0, 0}.
**
** Variable name: adjust_ratio.
** Adjust rg/bg ratios.
** Default values: {1.0, 1.0}.
**
** Variable name: enalbe_single_color_zone[DETECT_ZONE_NUM]
** Enable/disable single color zone
** Default value: 0 -- disable
** Range : 0 -- disable, 1 - eable
**
** Variable name: all_outlier_selection
** Select single color heuristic operation type between all outlier and not all outlier
** Default value: 0
** Data range:[0: not all outlier, 1: all outlier]
**
** Variable name: trigger_type
** Select single color heuristic method
** Default value: 0
** Data range:[0: bright_hysteresis, 1: dark_hysteresis, 2: bright_adjust_ratio, 3: dark_adjust_ratio]
**
** Variable name: Decision_for_ccm
** Select decision for getting valid CCM
** Default value: 0
** Data range: [0, Q3A_AWB_AGW_MAX_LIGHT]
******************************************************************************/
typedef struct
{
  awb_detect_zone_adapter         detection_zone[DETECT_ZONE_NUM];
  trigger_point_adapter           exp_index;
  awb_trigger_adapter_int         count_offset;
  chromatix_awb_gain_adj_adapter  adjust_ratio;

  /* Extended Parameter */
  int                             enable_single_color_zone[DETECT_ZONE_NUM];
  int                             all_outlier_selection;
  int                             trigger_type;
  int                             decision_for_ccm;
} awb_single_color_adapter;

#define AWB_SINGLE_DETECT_ZONE_NUM 6
#define NUM_SPECIAL_COLOR_DETECT   8
/******************************************************************************
** Feature name: AWB Single Color Tracking, Speical Heuristics
**
** Variable name: awb_single_color_heuristic_enable.
** Enable flag for running special heuristic for single color tracking.
** Default value: 0.
** Data range: 0 (disable) or 1 (enable).
**
** Variable name: awb_single_color_detect.
** A table for containing single color decision parameters as follows:
**
**
** Variable name: awb_single_color_valid_zones
** Number of valid single color zones
** Default value: 0
** Range : [0, NUM_SPECIAL_COLOR_DETECT]
**
** Variable name: awb_single_color_transition_frame_count
** The number of frames to be needed for hysteresis transition
** Default value: 30
** Data range: [0,30]
**
** Variable name: resv_dev_float1 and resv_dev_float2
** Reserved parameters for future developments
******************************************************************************/
typedef struct
{
  int                       awb_single_color_heuristic_enable;
  awb_single_color_adapter  awb_single_color_detect[NUM_SPECIAL_COLOR_DETECT];

  /* Extended Parameter */
  int                       awb_single_color_valid_zones;
  int                       awb_single_color_transition_frame_count;
  float                     resv_dev_float1[RESERVED_DEV_NUM];
  float                     resv_dev_float2[RESERVED_DEV_NUM];

  /*Additional parameters to remove reserved_data*/
  float                     bright_light_lux_idx_trigger;
  float                     low_light_lux_idx_trigger;
  int                       single_color_escape_counter_initial_value;
  int                       bright_light_escape_counter_decrease_number;
  int                       normal_light_escape_counter_decrease_number;
  int                       low_light_escape_counter_decrease_number;
  boolean                   fast_single_color_heuristic_converge_onoff;
} awb_single_color_tracking_adapter;

#define MISLEADING_COLOR_ZONE_NUM   30
#define GREEN_ZONE_ASSIST_NUM       5
#define ML_STATS_CNT_ZONE_NUM 3
#define ML_SUM_AUXZONE_NUM 3
typedef struct{
  int                       awb_mlc_stats_sum_control_enable;
  int                       awb_misleading_stats_count_zones[ML_STATS_CNT_ZONE_NUM];
  awb_trigger_adapter_float awb_misleading_sum_threshold;
  awb_detect_zone_adapter   awb_misleading_sum_aux_zones[ML_SUM_AUXZONE_NUM];
} awb_misleading_stats_sum_control_adapter;

#define ML_SUM_CTRL_NUM 10
/******************************************************************************
** Feature name: AWB Misleading Zone Process.
** Advanced outlier stat screening with misleading color zones.
**
** Variable name: awb_day_sum_threshold_count.
** Count based trigger points for varying size of misleading color zone with Day stats.
** Default value: 0.
** Data range: 0 to 3072.
**
** Variable name: awb_flo_sum_threshold_count.
** Count based trigger points for varying size of misleading color zone with Flou stats.
** Default value: 0.
** Data range: 0 to 3072.
**
** Variable name: awb_misleading_color_zones.
** A table of misleading color zones as follows:
******************************************************************************/
typedef struct
{
  awb_trigger_adapter_int                   awb_day_sum_threshold_count;
  awb_trigger_adapter_int                   awb_flo_sum_threshold_count;
  awb_misleading_zone_adapter               awb_misleading_color_zones[MISLEADING_COLOR_ZONE_NUM];

  /* Extended Parameter */
  int                                       awb_misleading_color_valid_zones;
  awb_misleading_stats_sum_control_adapter  awb_misleading_sum_control_zones[ML_SUM_CTRL_NUM];
  float                                     resv_dev_float1[RESERVED_DEV_NUM];
  float                                     resv_dev_float2[RESERVED_DEV_NUM];
} awb_misleading_zone_process_adapter;

/******************************************************************************
** Feature name : AWB golden module calibration data
**                for module-by-module variation.
**
** Variable name: enable_AWB_module_cal.
** Enable/Disable for reading golden module calibration data.
**
** Variable name: AWB_golden_module_R_Gr_ratio.
** Not tunable. Read it out from module’s EEPROM and convert into floating point.
** Default value: None. The value is read out from module’s EEPROM.
** Data range: 0 to 4.
** Constraints: None
** Effect: Not tunable.
**
** Variable name: AWB_golden_module_Gb_Gr_ratio.
** Not tunable. Read it out from module’s EEPROM and convert into floating point.
** Default value: None. The value is read out from module’s EEPROM.
** Data range: 0.8 and 1.2
** Constraints: none
** Effect: Not tunable.
**
** Variable name: AWB_golden_module_B_Gr_ratio.
** Not tunable. Read it out from module’s EEPROM and convert into floating point.
** Default value: None. The value is read out from module’s EEPROM.
** Data range: 0 to 4.
** Constraints: None
** Effect: Not tunable.
******************************************************************************/
typedef struct
{
  int   enable_AWB_module_cal;
  float AWB_golden_module_R_Gr_ratio[Q3A_AWB_AGW_MAX_LIGHT];
  float AWB_golden_module_Gb_Gr_ratio[Q3A_AWB_AGW_MAX_LIGHT];
  float AWB_golden_module_B_Gr_ratio[Q3A_AWB_AGW_MAX_LIGHT];
} awb_golden_module_info_adapter;

/******************************************************************************
** Feature name: AWB Flow Control.
**
** Variable name: skip_frames.
** Default value: 0.
** Data range: 0 to 100.
** Number of frames for skipping AWB process,
**
** Variable name: stat_saturation_threshold.
** Bayer AWB stat saturation threshold; only stats with saturation percentage less than this threshold
**   will be used for AWB process.
** Default value: 80.
** Data range: 0 to 100.
** Constraints:
** Effect: Higher value allows using stats with higher saturation percentage.
**
** Variable name: all_outlier_heuristic_flag.
** Enable all outlier heuristic function. If disabled
**   the frame is skipped when all stats are outliers.
** Data range: 0 (disable) or 1 (enable).
** Default value: 1.
**
** Variable name: valid_per_th.
** Required percentage of valid sample for AWB.
** Default value: 1.
** Data range: 0 to 100.
**
** Variable name: enable_flow_control_grp1
** Enable/disable the new parameters introduced in 310 header
** It affects on
** trigger_exp_lowlight_skip_frame, lowlight_skip_frame, subsampling_factor
** trigger_exp_to_lux_index, initial_cct, initial_prev_cct
** Default: 0
** Data range: 0 - disable, 1 - enable
**
** Variable name: trigger_exp_lowlight_skip_frame
** Exposure index for enabling skip frames for low light condition
** Default value: 500
** Data range: [0, 500]
**
** Variable name: lowlight_skip_frames
** Number of skipped framed in low light condition
** Default value: 1
** Data range: [0, 100]
**
** Variable name: subsampling_factor
** Specify subsampling factor (how many sample we need to skip)
** Default value: 0
** Data range: {0,1,2,4,8}
**
** Variable name: trigger_exp_to_lux_index;
** Specify trigger exposure index to use lux index
** Default value: maximum_exposure index
** Data range: [indoor exposure index, maximum exposure index]
**
** Variable name: initial_cct
** Specify initial cct
** Default value: 4100
** Data range: {4100, 5000, 6500}
**
** Variable name: initial_prev_cct
** Specify initial previous cct value
** Default value: 4100
** Data range: {4100, 5000, 6500}
**
** Variable name: awb_full_camif_enable
** Enable AWB config ROI to full camif size
** Default value: [0 ]
** Data range: [0, 1]
** Effect: When set to 1, AWB will config its ROI to camif, otherwise, AWB will config ROI based on FOV and crop.
**
** Variable name: initial_gain
** Specify initial r/g/b gain
** Default value: (r_gain(TL84), 1.0f, b_gain(TL84))
** Data range: [1.0f, 3.0f]
******************************************************************************/
typedef struct
{
  unsigned char                           skip_frames;
  int                                     stat_saturation_threshold;
  int                                     all_outlier_heuristic_flag;
  int                                     valid_per_th;

  /* Extended Parameter */
  int                                     enable_flow_control_grp1;
  int                                     trigger_exp_lowlight_skip_frame;
  int                                     lowlight_skip_frame;
  int                                     subsampling_factor;
  int                                     trigger_exp_to_lux_index;
  int                                     initial_cct;
  int                                     initial_prev_cct;
  int                                     awb_full_camif_enable;
  chromatix_manual_white_balance_adapter  initial_gain;
} awb_flow_control_adapter;

/******************************************************************************
** Feature name: AWB Purple Prevent.
**
** Variable name: AWB_purple_prevent_enable.
** Enables the feature.
** Default value: 1.
** Data range: 0 to 1.
** Constraints: None.
**
** Variable name: control_purple_prevent.
** Control with gain or lux_index.
** Default value: 0.
** Data Range: 0 or 1.
** Constraints: None.
** Effect: Use gain or lux_index to control purple sky prevention.
**
** Variable name: purple_prevent_trigger.
** Control the purple sky prevention adjustment trigger.
** Default value (caluculated): Between 1/2 outdoor index and outdoor index.
** Data Range: Integers 0 to a few hundreds.
** Constraints: Only for bright outdoor.
** Effect: Smaller index values make the purple sky prevention harder to take
**         effect (has to be very bright).
**
** Variable name: purple_sky_prevention_bg_threshold.
** Controls the purple sky prevention adjustment.
** Default value (calculated): BG ratio of D50.
** Data Range: 0 to 2.
** Constraints: Around D50.
** Effect: Smaller index values make the adjustment larger.
******************************************************************************/
typedef struct
{
  int                     AWB_purple_prevent_enable;
  tuning_control_adapter  control_purple_prevent;
  trigger_point_adapter   purple_prevent_trigger;
  float                   purple_sky_prevention_bg_threshold;
} AWB_purple_prevent_adapter;

/******************************************************************************
** Feature name: AWB Blue Sky.
** Structure definition of blue sky parameters.
**
** Variable name: ref_b_bg_tl84.
** This reference R, B's r/g and b/g are for 8k FFA; it needs to be tuned for other sensors.
**   Corresponding rolloff table should be applied when doing the measurement.
** Default value: These two values are specifically estimated for the
**                corresponding sensor. Default value for 8k Foxcon sensor are:
**                ref_b_bg_tl84 = 1.4049; ref_r_rg_a = 3.1547.
** Data range: 0 to 10.
** Constraints: None.
** Effect: Accurately estimated values decide the stat collected for extreme blue and red.
**         For calibration of these two values, take full-view MCC chart under TL84 and A lighting.
**         Apply correct rolloff table and then measure the blue’s b/g from the MCC taken under TL84.
**         Measure the red’s r/g from the MCC taken under A.
**
** Variable name: extreme_range_perc_b.
** Extreme blue and red threshold.
** Default value: 1.
** Data range: 0 to 10.
** Constraints: None.
** Effect: Higher values increase the boundary of collecting extreme red and blue stat.
**         Lower values reduce the boundary of collecting the extreme stat.
**
** Variable name: blue_sky_pec.
** Percentage of the extreme blue stats.
** Default value: 0.20
** Data range: 0 to 1.
** Constraints: None.
** Effect: If the extreme blue stats exceed this percentage of all the collected stats, the blue sky prevention is to be conducted
**         to prevent purple sky. The smaller the percentage, the easier the purple sky prevention will be performed when AWB decision
**         is day light.
**
** Variable name: blue_sky_pec_buffer.
** Percentage of the extreme blue stats.
** Default value: 0.15
** Data range: 0 to awb2_blue_sky_pec.
** Constraints: None.
** Effect: This percentage is to set a buffer zone between doing the purple sky prevention and not doing it. The closer to awb2_blue_sky_pec,
**         the smaller the buffer zone is. The smaller it is than awb2_blue_sky_pec, the larger the buffer zone is.
**
** Variable name: threshold_extreme_b_percent.
** Threshold of the blue or red extreme stat counts.
** Default value: 0.1
** Data range: 0 to 1.
** Constraints: None.
** Effect: Higher value requires a higher number of the collected
**         extreme stats to be recognized as an extreme color case.
******************************************************************************/
typedef struct
{
  float                       ref_b_bg_tl84;
  float                       extreme_range_perc_b;
  float                       blue_sky_pec;
  float                       blue_sky_pec_buffer;
  float                       threshold_extreme_b_percent;

  AWB_purple_prevent_adapter  AWB_purple_prevent;
} awb_blue_sky_adapter;

/******************************************************************************
** Feature name:awb_blue_tuning_adapter
**
** Variable name: snow_blue_gain_adj_ratio.
** Snow blue gain adjustment.
** Default: 1.0
** Range: 0.0f to 4.0f.
**
** Variable name: beach_blue_gain_adj_ratio.
** Beach blue gain adjustment
** Default: 1.0
** Range: 0.0f, 4.0f.
******************************************************************************/
typedef struct
{
  awb_blue_sky_adapter  blue_sky_param;
  float                 snow_blue_gain_adj_ratio;
  float                 beach_blue_gain_adj_ratio;
} awb_blue_tuning_adapter;


/******************************************************************************
** Feature name: AWB Skin Control.
**
** Variable name: face_awb_enable.
** Enable/Disable face dominant AWB feature.
** Default: 0 (disable).
** Range: 0 - disable, 1-enable.
**
** Variable name: skin_ref_exp_index_trigger.
** Skin reference point trigger start and end points.
** Default: 0, 0.
** Range  : 0 to 1000.
**
** Variable name: skin_reference[Q3A_AWB_AGW_MAX_LIGHT];
** Skin reference points.
******************************************************************************/
typedef struct
{
  int face_awb_enable;
  trigger_point_adapter           skin_ref_exp_index_trigger;
  chromatix_awb_reference_adapter skin_reference[Q3A_AWB_AGW_MAX_LIGHT];

  /* Face AWB dominated AWB related parameters, put in awb skin control*/
  int exclude_face_stats_enable;
  int min_face_stats_count;
  int min_frame_count;
  int face_awb_stable_cnt_thd;
  int face_awb_off_cnt_thd;
  int face_stats_outlier_distance;
  int face_awb_ext_stable_cnt_thd;

  int faceawb_gain_stability_improvement;
  int faceawb_roi_stability_improvement;
  int faceawb_roi_stability_improvement_cnt_thd;
  int faceawb_stability_improvement_cnt_thd;
  int faceawb_stability_improvement_x_dev_thd;
  int faceawb_stability_improvement_y_dev_thd;
  int faceawb_stability_improvement_cnt_thd_for_gain;
  int faceawb_stability_improvement_lock_cnt_thd;
  float faceawb_stability_improvement_r_gain_dev_thd;
  float faceawb_stability_improvement_b_gain_dev_thd;

  float dark_skin_point_transfer_ratio_r;
  float dark_skin_point_transfer_ratio_b;
  float light_wb_target_rg;
  float light_wb_target_bg;
  float dark_wb_target_rg;
  float dark_wb_target_bg;
  float roi_start_zone_0;
  float roi_end_zone_0;
  float roi_start_zone_1;
  float roi_end_zone_1;
  float min_weight_zone_0;
  float min_weight_zone_1;
  float max_weight;
  float skin_dark_threshold_r;
  float skin_dark_threshold_g;
  float skin_dark_threshold_b;
  float light_skin_weight_of_dist_confidence_ratio;
  float light_skin_weight_of_cct_confidence_ratio;
  float dark_skin_cct_high_thd;
  float dark_skin_cct_low_thd;
  float light_skin_cct_high_thd;
  float light_skin_cct_low_thd;
  float light_skin_cct_cr_high_cct_high_lux_index;
  float light_skin_cct_cr_high_cct_low_lux_index;
  float light_skin_cct_cr_low_cct_high_lux_index;
  float light_skin_cct_cr_low_cct_low_lux_index;
  float dark_skin_cct_cr_high_cct_high_lux_index;
  float dark_skin_cct_cr_high_cct_low_lux_index;
  float dark_skin_cct_cr_low_cct_high_lux_index;
  float dark_skin_cct_cr_low_cct_low_lux_index;
  float faceawb_convergence_speed;
  float x_roi_divide;
  float y_roi_divide;
  float face_awb_exp_index_bright_light_gain_adjust_start;
  float face_awb_exp_index_bright_light_gain_adjust_end;
  float face_awb_exp_index_lowlight_gain_adjust_start;
  float face_awb_exp_index_lowlight_gain_adjust_end;
  float skin_gain_adjust_r_gain_bright_light[FACE_AWB_GAIN_ADJUST_MAX_NUMBER];
  float skin_gain_adjust_b_gain_bright_light[FACE_AWB_GAIN_ADJUST_MAX_NUMBER];
  float skin_gain_adjust_r_gain_normal_light[FACE_AWB_GAIN_ADJUST_MAX_NUMBER];
  float skin_gain_adjust_b_gain_normal_light[FACE_AWB_GAIN_ADJUST_MAX_NUMBER];
  float skin_gain_adjust_r_gain_low_light[FACE_AWB_GAIN_ADJUST_MAX_NUMBER];
  float skin_gain_adjust_b_gain_low_light[FACE_AWB_GAIN_ADJUST_MAX_NUMBER];
} awb_skin_control_adapter;


/******************************************************************************
** Variable name: awb_gyro_trigger.
** Used to control how scene change should be detected for AEC.
** Default value: 0.0.
** Data range: -16000.0 to +16000.0.
** Constraints: None.
** Effect: The bigger the value is, the less sensitive AEC response to
**         gyro output value.
**
** Variable name: awb_accelerometer_trigger.
** Used to control how scene change should be detected for AEC.
** Default value: 0.0.
** Data range: -16000.0 to +16000.0.
** Constraints: None.
** Effect: The bigger the value is, the less sensitive AEC response to
**         accelerometer change.
**
** Variable name: awb_magnetometer_trigger.
** Used to control how scene change should be detected for AEC.
** Default value: 0.0.
** Data range: 0.0 to 360.0.
** Constraints: None.
** Effect: The bigger the value is, the less sensitive AEC response to
**         magnetic field change.
**
** Variable name: awb_DIS_motion_vector_trigger.
** Used to control how scene change should be detected for AEC.
** Default value: 0.0.
** Data range: -16000.0 to +16000.0.
** Constraints: None.
** Effect: The bigger the value is, the less sensitive AEC response to
**         digital image stabilization movement/compensation value.
******************************************************************************/
typedef struct
{
  float gyro_trigger;
  float accelerometer_trigger;
  float magnetometer_trigger;
  float DIS_motion_vector_trigger;
} AWB_motion_sensor_adapter;

/******************************************************************************
** Feature name : AWB auxiliary sensors.
** NOTE: currently have motion sensor only. In future, more members will be added.
**
** Variable name: aux_sensor_enable.
** Enable/Disable auxiliary AWB sensor info.
** Default: 0 (disable).
** Range: 0 - disable, 1-enable.
**
** Variable name: AWB_motion_sensor_data.
** AWB sensor info.
******************************************************************************/
typedef struct
{
  int                         aux_sensor_enable;
  AWB_motion_sensor_adapter   AWB_motion_sensor_data;
} awb_aux_sensor_adapter;

/******************************************************************************
** Feature name: AWB Front Camera Control.
** A set of flags for enabling/disabling front camera features.
**
** Variable name: awb_front_camera.
** Enable/disable front camera speific features.
** Default value: 0.
** Data range: 0 (disable) or 1 (enable).
**
** Variable name: awb_cap_gain_enable.
** AWB cap gain adjust.
** Default value: 0.
** Data range: 0 (disable) or 1 (enable).
**
** Variable name: awb_fd_awb_enable.
** Enable/disable face info delivery and face-aware AWB.
** Default value: 0.
** Data range: 0 (disable) or 1 (enable).
******************************************************************************/
typedef struct
{
  int awb_front_camera;
  int awb_cap_gain_enable;
  int awb_fd_awb_enable;
} awb_front_camera_ctrl_adapter;

/******************************************************************************
** Feature name :
** Manual white balance gains for both snapshot and viewfinder.
******************************************************************************/
typedef struct
{
  chromatix_manual_white_balance_adapter  MWB_tl84;                    /* Flourescent*/
  chromatix_manual_white_balance_adapter  MWB_d50;                     /* Sunny*/
  chromatix_manual_white_balance_adapter  MWB_A;                       /* Tungsten*/
  chromatix_manual_white_balance_adapter  MWB_d65;                     /* Cloudy/Shade*/
  chromatix_manual_white_balance_adapter  strobe_flash_white_balance;  /* Strobe*/
  chromatix_manual_white_balance_adapter  led_flash_white_balance;     /* LED*/
} awb_MWB_adapter;

/******************************************************************************
** Feature name: awb_dynamic_convergence_type
** Control dynamic skip frames
**
** Variable name: awb_dynamic_frame_skip_control_enable
** Enable/Disable dynamic frame skip control
** Default: 0 - disable
** Range: 0 - disable, 1 - enable
**
** Variable name: awb_settle_time_threshold_for_temp_stability
** Define the awb settle time threshold for enabling awb_temproal_stability
** If awb_temproal_stability is enabled and awb settle time is longer than the threshold,
** the current average r/g and b/g ratio will be replaced by awb history’s average r/g and b/g ratio
** Default: 1.0
** Range: [0.3, 2]
**
** Variable name: awb_settle_time_threshold_for_awb_frame_skip
** If dynamic frame skip control is enabled and awb settle time is longer than the threshold,
** the AWB will start to kip frame, otherwise, AWB will not skip any frame
** Default: 1.0
** Range: [0.3, 2]
**
** Variable name: convergence_factor
** convergence speed of red gain (blue gain) for gain adjust
** Default: (0.075, 0.075)
** Range: [0.02, 1.0]
**
** Variable name: convergence_factor_camera_before_awb_settle
** convergence speed of still mode for awb output gain before AWB settle,
** the convergence speed should be slower before AWB settle
** Default: 0.03
** Range: [0.005, 0.2]
**
** Variable name: convergence_factor_camera_after_awb_settle
** convergence speed of still mode for awb output gain before AWB settle,
** the convergence speed should be faster after AWB settle
** Default: 0.15
** Range : [0.005, 0.2]
**
** Variable name: convergence_factor_video_before_awb_settle
** convergence speed of video mode for awb output gain before AWB settle,
** the convergence speed should be slower before AWB settle
** Default: 0.01
** Range: [0.005, 0.2]
**
** Variable name: convergence_factor_video_after_awb_settle
** convergence speed of video mode for awb output gain before AWB settle,
** the convergence speed should be faster after AWB settle
** Default: 0.05
** Range: [0.005, 0.2]
******************************************************************************/
typedef struct
{
  int                             awb_dynamic_frame_skip_control_enable;
  float                           awb_settle_time_threshold_for_temp_stability;
  float                           awb_settle_time_threshold_for_awb_frame_skip;
  chromatix_awb_gain_adj_adapter  convergence_factor;
  float                           convergence_factor_camera_before_awb_settle;
  float                           convergence_factor_camera_after_awb_settle;
  float                           convergence_factor_video_before_awb_settle;
  float                           convergence_factor_video_after_awb_settle;
} awb_dynamic_convergence_adapter;

/******************************************************************************
** Feature name: awb_gyro_assistance_type
** Convergence with gyro information
**
** Variable name: awb_gyro_assisted_awb_enable
** Enable/Disable gyro assistance awb convergence
** Default: 0 - disable
** Range:  0 - disable, 1 - enable
**
** Variable name: awb_gyro_threshold
** Define gyro min/max threshold for triggering awb_convergence_speed
** fstart: min threshold, fend: max threshold
** Default: (0.01, 0.3)
** Range: [0.005, 1.0]
**
** Variable name: awb_gyro_convergence_speed
** Define min/max AWB convergence speed for gyro information
** fstart: minimum AWB convergence speed
** fend: maximum AWB convergence speed
** Default: (0.03, 0.3)
** Range : [0.005, 0.2]
**
** Variable name: awb_gyro_stable_th
** define the gyro stable threshold
** Default: 0.01
** Range: [0.005, 0.03]
**
** Variable name: awb_gyro_luma_delta_th[AWB_GYRO_LUMA_DELTA_NUM]
** Define the min/max/stable luma difference values
** 0: minimum, 1: maximum, 2:stable
** Default: [5, 10, 2]
** Range: [1, 15]
**
** Variable name: awb_gyro_convergence_speed_for_luma
** Define min/max convergence speed for luma information
** fstart: min, fend:max
** Default: (0.15, 0.3)
** Range: [0.2 ,0.005]
**
** Variable name: awb_gyro_holdind_time_enable
** Enable/disable holding time control
** Default: 0 - disable
** Range: 0 -disable, 1 - enable
**
** Variable name: awb_gyro_holding_time_param[AWB_GYRO_HT_PARAM]
** Define several thresholds corresponding to the holding time properties
** 0: holding time threshold
** 1: luma delta threshold
** 2: maximum holding time threshold
** 3: lower boundary of gyro average values
** 4: the percentage of maximum holding time
** 5: higher boundary of gyro average value
** 6: the percentage of maximum holding time
** Default: [ 3.0, 10, 1.0, 0.01, 1.0, 0.3, 0.5 ]
** Range: 0: [1.0, 5.0]
**        1: [5,2 0]
**        2: [0.5, 5.0]
**        3: [0.005, 0.05]
**        4: [0.1, 1.0]
**        5: [0.1, 1.0]
**        6: [0.1, 1.0]
**
******************************************************************************/
#define AWB_GYRO_LUMA_DELTA_NUM     3
#define AWB_GYRO_HT_PARAM           7

typedef struct
{
  int                       awb_gyro_assisted_awb_enable;
  awb_trigger_adapter_float awb_gyro_threshold;
  awb_trigger_adapter_float awb_gyro_convergence_speed;
  float                     awb_gyro_stable_th;
  float                     awb_gyro_luma_delta_th[AWB_GYRO_LUMA_DELTA_NUM];
  awb_trigger_adapter_float awb_gyro_convergence_speed_for_luma;
  int                       awb_gyro_holdind_time_enable;
  float                     awb_gyro_holding_time_param[AWB_GYRO_HT_PARAM];
} awb_gyro_assistance_adapter;

/******************************************************************************
** Feature name: awb_special_weights_type
** Specify weights of corresponding cluster for lab detection
** NOTE: Not much useful in most cases, recommend not to use
**
** Variable name: awb_sp_weights_enable
** Enable/Disable special weights
** Default value: 0 - disable, (1 - Enable)
**
** Variable name: weight_day
** Weight for day cluster
** Default value: 1
** Data range: [1,10]
**
** Variable name: weight_f
** Weight for f cluster
** Default value: 1
** Data range: [1,10]
**
** Variable name: weight_a
** Weight for A cluster
** Default value: 1
** Data range: [1,10]
**
** Variable name: weight_h
** Weight for H cluster
** Default value: 1
** Data range: [1,10]
******************************************************************************/
typedef struct
{
  int awb_sp_weights_enable;
  int weight_day;
  int weight_f;
  int weight_a;
  int weight_h;
} awb_special_weights_adapter;

#define EXTR_CCT_SUBZONE_CNT_NUM        3
#define EXTR_CCT_ZONE_NUM               4
#define EXTR_CCT_LOWCCT_LOWLIGHT_COND   3
#define EXTR_CCT_HIGH_CCT_COND          3
#define EXTR_CCT_HIGH_CCT_CAT           6
#define EXTR_CCT_EXTRA_ZONE             3
#define EXTR_CCT_LOWLIGHT_RUN_COND      5
#define EXTR_CCT_LOWLIGHT_EXIT_COND     6
#define EXTR_CCT_LOWLIGHT_EXIT_CONTROL  5

/******************************************************************************
** Feature name: awb_extreme_cct_type
** Control extreme high CCT (>7500K) and low cct (<2000K) condition
**
** Variable name: awb_extr_cct_enable
** Enable/disable extreme cct operations
** Default value: 0 -disable,
** Data range: 1 - enable (0 - disable)
** Effect: The following flags are valid only if this flag is set to 1 (enable)
**         if not, always disable the following flags.
** (1) awb_extr_low_cct_enable
** (2) awb_extr_lowlight_enable
** (3) awb_extr_high_cct_enable
**
** Variable name: awb_extr_cct_zone_select[EXTR_CCT_SUBZONE_CNT_NUM][EXTR_CCT_ZONE_NUM]
** Specify the subzones that contribute on each CCT categories
** Each row is corresponding to
** [0]: extreme_high_cct, [1]: high_cct, [2]: mid_cct, [3]: low_cct
** Each column is corresponding to subzones
** [0]: 0~11 subzones, [1]: 12~23 subzones, [2]: 24~35 subzones
** Default value: { 2048,     0,     0,
**                  1984,     0,     0,
**                    63,  4064,     0,
**                     0,    31,  4095}
** Data range: [0, 4095]
**
** Variable name:awb_extr_low_cct_enable
** Enable/Disable extreme low cct case control
** Default value: 0 - disable
** Data range: 1 - enable (0 - disable)
**
** Effect: if this is valid, the following 3 parameters are working
**         (1) valid_low_cct_sample_num;
**         (2) awb_extr_horizon_detect_boundary;
**         (3) awb_extr_low_cct_adjust;
**
** Variable name:valid_low_cct_sample_num
** valid sample threshold for entering the logic
** Default value: 200
** Data range: [50, 300]
**
** Variable name: awb_extr_horizon_detect_boundary
** Specify the offset rg/bg ratio values for horizon boundary
** Default value: (0.1, 0.1)
** Data range: [0.0, 0.5]
**
** Variable name: awb_extr_low_cct_adjust
** adjust ratio for extreme_low_cct_heuristic target
** Default value: (1.0, 1.0)
** Data range: [0.5, 1.5]
**
** Variable name:awb_extr_lowlight_enable
** Enable/disable extreme low light conditino control
** Default value: 0 - disable,
** Data range: 1 - enable , (0 - disable)
** Effect: If it is enabled, the following 6 parameterse are valid
**        (1)awb_extr_lowlight_run_condition
**        (2)awb_extr_lowlight_exit_conditions
**        (3)awb_extr_lowlight_exit_frame_cnt_control
**        (4)awb_extr_lowlight_adjust
**        (5)awb_extr_lowlight_target_decision
**        (6)awb_extr_lowlight_exit_boundary
**
** Variable name: awb_extr_lowlight_run_condition
** Define the trigger values for running extreme low light heuristics
** [0]: low light luma values
** [1]: valid sample num for low light condition
** [2]: rg ratio values for region boundary (currently horizon region boundary)
** [3]: bg ratio values for region boundary (currently horizon region boundary)
** [4]; holding frame number before exiting from low light heuristic operation
** Default value: [3.0f, 100.0f, 0.0f, 0.0f, 30.0f]
** Data range: [0.0f, 10.0f], [50.0f, 300.0f], [0.0f, 1.0f],[0.0f, 1.0f], [25.0f,35.0f]
**
** Variable name: awb_extr_lowlight_exit_condition
** Exit conditions for extreme lowlight heuristic
** [0]: the first extreme lowlight heuristic trigger luma value
** [1]: the second extreme lowlight heuristic trigger luma value
** [2]: the first valid sample threshold for extreme lowlight heuristic
** [3]: the second valid sample threshold for extreme lowlight heuristic
** [4]: the third valid sample threshold for extreme lowlight heuristic
** [5]: the fourth valid sample threshold for extreme lowlight heuristic
**
** Default value: { 5, 10, 750, 500, 400, 250 }
** Data range: [0, 15] -- for [0~1] elements
**             [50, 1200] -- for [2~5] elements
**
** Variable name:	awb_extr_lowlight_exit_frame_cnt_control[EXTR_CCT_LOWLIGHT_EXTI_COND]
** Define speed of decreasing frame number for exiting extreme lowlight heuristic
** Default value: { 5, 5, 10, 7, 5 }
** Data range: [1, 15]
**
** Variable name: awb_extr_lowlight_exit_boudnary
** horizon stats detection boundary for exiting lowlight heuristic
** Default value: {0.1f, 0.1f }
** Data range: [0.0f, 1.0f]
**
** Variable name: awb_extr_lowlight_adjust
** Specify the adjust gain for lowlight condition
** Default value: { 1.0f, 1.0f }
** Data range: [0.5f, 1.5f]
**
** Variable name: awb_extr_lowlight_target_decision
** Specify the awb decision for valid CCM selection under lowlight condition
** Default value: 3
** Data range: [0, Q3A_AWB_AGW_MAX_LIGHT]
**
** Variable name: awb_extr_high_cct_enable
** Enable/Disable high cct condition control
** Default value: 0 - disable,
** Data range: 1 - enable, (0 - disable)
** Effect: if it is enabled, the following 4 parameters are valid
**         (1)awb_extr_high_cct_wb_zone
**         (2)awb_extr_high_cct_aux_zone_detect
**         (3)awb_extr_cct_hightcct_conditions
**         (4)awb_extr_cct_hightcct_adjust
**
** Variable name: awb_extr_high_cct_wb_zone
** high cct samples' rg, bg ratio calculation area selection
** Specify subzones in which high cct samples are.
** rg/bg ratios of high cct samples will be computed the stats in these subzones.
** Default value:  { 4095, 3584, 0 }
** Data range: [0, 4095]
**
** Variable name: awb_extr_high_cct_aux_zone_detect[3]
** collect outlier blue sky stats for extreme_high_cct_heuristic
** Default value: { 0.0f, 0.0f, 0, 0 }
** Data range: [0.0f ~ 2.0f], [0 ~ 241]
**
** Variable name: awb_extr_cct_hightcct_conditions
** Define threshold values for each condition
** each column represents
** [0]: extreme_high_cct, [1]: high_cct, [2]: mid_cct
** [3]: low_cct, [4]: extreme_high_cct + high_cct, [5]: mid_cct + low_cct
** each row represents
** [0]: extreme high cct condition
** [1]: the 1st high cct condition
** [2]: the 2nd high cct condition
** Default value: { 2000, 100, 100, 100, -1, -1,
**                  -1  , 500, 400, 100, 2000, 400,
**                  -1  ,2000, 100, 100, 2000, 2500 }
** Data range: [0, 3072]
**
** Variable name: awb_extr_cct_hightcct_adjust
** Define adjust ration for each condition
** [0]: extreme high cct condition
** [1]: the 1st high cct condition
** [2]: the 2nd high cct condition
** Default value: (1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f)
** Data range: [0.0f, 3.0f]
**
** Variable name: resv_dev_float and resv_dev_int
** Reserved parameters for future developments
******************************************************************************/
typedef struct{
  int                             awb_extr_cct_enable;
  int                             awb_extr_cct_zone_select[EXTR_CCT_ZONE_NUM][EXTR_CCT_SUBZONE_CNT_NUM];
  int                             awb_extr_low_cct_enable;
  int                             awb_extr_low_cct_sample_num;
  chromatix_awb_reference_adapter awb_extr_low_cct_detect_boundary;
  chromatix_awb_gain_adj_adapter  awb_extr_low_cct_adjust;
  int                             awb_extr_lowlight_enable;
  float                           awb_extr_lowlight_run_condition[EXTR_CCT_LOWLIGHT_RUN_COND];
  int                             awb_extr_lowlight_exit_condition[EXTR_CCT_LOWLIGHT_EXIT_COND];
  int                             awb_extr_lowlight_exit_frame_cnt_control[EXTR_CCT_LOWLIGHT_EXIT_CONTROL];
  chromatix_awb_reference_adapter awb_extr_lowlight_exit_boundary;
  chromatix_awb_gain_adj_adapter  awb_extr_lowlight_adjust;
  int                             awb_extr_lowlight_target_decision;
  int                             awb_extr_high_cct_enable;
  int                             awb_extr_high_cct_wb_zone[EXTR_CCT_EXTRA_ZONE];
  awb_detect_zone_adapter         awb_extr_high_cct_aux_zone_detect[EXTR_CCT_EXTRA_ZONE];
  float                           awb_extr_cct_hightcct_conditions[EXTR_CCT_HIGH_CCT_COND][EXTR_CCT_HIGH_CCT_CAT];
  chromatix_awb_gain_adj_adapter  awb_extr_cct_hightcct_adjust[EXTR_CCT_HIGH_CCT_COND];
  float                           resv_dev_float[RESERVED_DEV_NUM];
  int                             resv_dev_int[RESERVED_DEV_NUM];
} awb_extreme_cct_adapter;

#define PREFLASH_DETECT_NUM 3
typedef struct
{
  awb_detect_zone_adapter         detection_zone[PREFLASH_DETECT_NUM];
  chromatix_CCT_trigger_adapter   cct_boundary;
  awb_trigger_adapter_float       k1_boundary;
  chromatix_awb_reference_adapter target_ratio;
} awb_preflash_heuristic_adapter;

/******************************************************************************
** Feature name: awb_preflash_enhance_type
** Alternatives of preflash operation and corresponding parameters
**
** Variable name: awb_preflash_enhancement_enable
** Enable/Disable alternative preflash operation
** Default value: 0 - Disable, (1 - Enable)
**
** Variable name: outdoor_detection_exp_index
** Exposure trigger points for outdoor detection
** Default value: [outdoor_index + 30, outdoor_index + 50]
** Data range: [0, 1000]
**
** Variable name: awb_flash_stat_sat_threshold;
** The same as that in awb_preflash_type
**
** Variable name: awb_preflash_outlier_distance;
** The same as that in awb_preflash_type
**
** Variable name: awb_preFlash_min_ref;
** The same as that in awb_preflash_type
**
** Variable name: awb_preFlash_max_ref;
** The same as that in awb_preflash_type
**
** Variable name: valid_spl_trigger_for_index_selection;
** valid flash stats number for flash_index decision
** Default value:  [500, 1000]
** Data range: [0, 3072]
**
** Variable name: flash_index_trigger;
** Specify led line index trigger for interpolating pre flash rg/bg ratios
** Default value: {2, 8}
**
** Variable name: flash_adjust_gain_cct_trigger[3];
** Trigger CCT for gain adjust
** Default value: { 6500, 6200, 4200, 4000, 3500, 2600}

** Variable name: cct1_adjust_gain_table[6][5];
** Adjust gain table with k1 ratio, minimum rg/bg ratios, maximum rg/bg ratios in sequential order
** Default value: {1.0f, 1.0f, 1.0f, 1.0f, 1.0f}
** Data range: k1 ratio: [0.0f, 50.0f], rg/bg ratio: [0.0f, 3.0f]
**
** Variable name: cct2_adjust_gain_table[6][5];
** Adjust gain table with k1 ratio, minimum rg/bg ratios, maximum rg/bg ratios in sequential order
** Default value: {1.0f, 1.0f, 1.0f, 1.0f, 1.0f}
** Data range: k1 ratio: [0.0f, 50.0f], rg/bg ratio: [0.0f, 3.0f]
**
** Variable name: cct3_adjust_gain_table[6][5];
** Adjust gain table with k1 ratio, minimum rg/bg ratios, maximum rg/bg ratios in sequential order
** Default value: {1.0f, 1.0f, 1.0f, 1.0f, 1.0f}
** Data range: k1 ratio: [0.0f, 50.0f], rg/bg ratio: [0.0f, 3.0f]
**
** Variable name: cct4_adjust_gain_table[6][5];
** Adjust gain table with k1 ratio, minimum rg/bg ratios, maximum rg/bg ratios in sequential order
** Default value: {1.0f, 1.0f, 1.0f, 1.0f, 1.0f}
** Data range: k1 ratio: [0.0f, 50.0f], rg/bg ratio: [0.0f, 3.0f]
**
** Variable name: valid_spl_trigger_for_gain_calculation;
** Specify the valid sample trigger point for gain computation
** Default value: { 1250, 500}
** Data range: [0, 3072]
**
** Variable name: preflash_heuristic[10];
** Specify information for additional heuristics
** Default value: See awb_preflash_heuristic_type
**
** Variable name: flash_adjust_ccm_cct_trigger[3];
** adjust gain cct trigger for interpolation among cct1 ~ cct4 adjust gain table
** Default value: { {6500, 6100}, {4200, 3800}, {3300, 2600} }
** Data range: [2300, 7500]
**
** Variable name: cct1_adjust_ccm_table[6][3];
** CCM mix ratio between normal and flash CCMs according to k1 and cct.
** Default value: [k1, saturation_decrease_ratio, mix_ratio]
**  {0.96f, 1.0f, 1.0f},
**  {0.87f, 1.0f, 1.0f},
**  {0.79f, 1.0f, 1.0f},
**  {0.65f, 1.0f, 1.0f},
**  {0.25f, 1.0f, 1.0f},
**  {0.10f, 1.0f, 1.0f},
** Data range: k1: [0.0f 1.0f], saturation_decrease_ratio [0.0f, 1.0f], mix_ratio[0.0f, 1.0f]
**
** Variable name: cct2_adjust_ccm_table[6][3];
** CCM mix ratio between normal and flash CCMs according to k1 and cct.
** Default value: [k1, saturation_decrease_ratio, mix_ratio]
**  {0.96f, 1.0f, 1.0f},
**  {0.87f, 1.0f, 1.0f},
**  {0.79f, 1.0f, 1.0f},
**  {0.65f, 1.0f, 1.0f},
**  {0.25f, 1.0f, 1.0f},
**  {0.10f, 1.0f, 1.0f},
** Data range: k1: [0.0f 1.0f], saturation_decrease_ratio [0.0f, 1.0f], mix_ratio[0.0f, 1.0f]
**
** Variable name: cct3_adjust_ccm_table[6][3];
** CCM mix ratio between normal and flash CCMs according to k1 and cct.
** Default value: [k1, saturation_decrease_ratio, mix_ratio]
**  {0.96f, 1.0f, 1.0f},
**  {0.87f, 1.0f, 1.0f},
**  {0.79f, 1.0f, 1.0f},
**  {0.65f, 1.0f, 1.0f},
**  {0.25f, 1.0f, 1.0f},
**  {0.10f, 1.0f, 1.0f},
** Data range: k1: [0.0f 1.0f], saturation_decrease_ratio [0.0f, 1.0f], mix_ratio[0.0f, 1.0f]
**
** Variable name: cct4_adjust_ccm_table[6][3];
** CCM mix ratio between normal and flash CCMs according to k1 and cct.
** Default value: [k1, saturation_decrease_ratio, mix_ratio]
**  {0.96f, 1.0f, 1.0f},
**  {0.87f, 1.0f, 1.0f},
**  {0.79f, 1.0f, 1.0f},
**  {0.65f, 1.0f, 1.0f},
**  {0.25f, 1.0f, 1.0f},
**  {0.10f, 1.0f, 1.0f},
** Data range: k1: [0.0f 1.0f], saturation_decrease_ratio [0.0f, 1.0f], mix_ratio[0.0f, 1.0f]
**
** Variable name: resv_dev_float;
** Variable name: resv_dev_int;
******************************************************************************/
#define PREFLASH_ENHANCE_HEURISTIC_NUM 10
#define PREFLASH_ADJ_GAIN_CCT_H 6
#define PREFLASH_ADJ_GAIN_CCT_V 5
#define PREFLASH_CCT_TRIGGER 3
#define PREFLASH_ADJ_CCM_V 3
typedef struct
{
  int                             awb_preflash_enhancement_enable;
  trigger_point_adapter           outdoor_detection_exp_index;
  int                             awb_flash_stat_sat_threshold;
  int                             awb_preflash_outlier_distance;
  chromatix_awb_reference_adapter awb_preFlash_min_ref;
  chromatix_awb_reference_adapter awb_preFlash_max_ref;
  awb_trigger_adapter_int         valid_spl_trigger_for_index_selection;
  awb_trigger_adapter_int         flash_index_trigger;
  chromatix_CCT_trigger_adapter   flash_adjust_gain_cct_trigger[PREFLASH_CCT_TRIGGER];
  float                           cct1_adjust_gain_table[PREFLASH_ADJ_GAIN_CCT_H][PREFLASH_ADJ_GAIN_CCT_V];
  float                           cct2_adjust_gain_table[PREFLASH_ADJ_GAIN_CCT_H][PREFLASH_ADJ_GAIN_CCT_V];
  float                           cct3_adjust_gain_table[PREFLASH_ADJ_GAIN_CCT_H][PREFLASH_ADJ_GAIN_CCT_V];
  float                           cct4_adjust_gain_table[PREFLASH_ADJ_GAIN_CCT_H][PREFLASH_ADJ_GAIN_CCT_V];
  awb_trigger_adapter_int         valid_spl_trigger_for_gain_calculation;
  awb_preflash_heuristic_adapter  preflash_heuristic[PREFLASH_ENHANCE_HEURISTIC_NUM];
  chromatix_CCT_trigger_adapter   flash_adjust_ccm_cct_trigger[PREFLASH_CCT_TRIGGER];
    float                         cct1_adjust_ccm_table[PREFLASH_ADJ_GAIN_CCT_H][PREFLASH_ADJ_CCM_V];
  float                           cct2_adjust_ccm_table[PREFLASH_ADJ_GAIN_CCT_H][PREFLASH_ADJ_CCM_V];
  float                           cct3_adjust_ccm_table[PREFLASH_ADJ_GAIN_CCT_H][PREFLASH_ADJ_CCM_V];
  float                           cct4_adjust_ccm_table[PREFLASH_ADJ_GAIN_CCT_H][PREFLASH_ADJ_CCM_V];
  float                           resv_dev_float[RESERVED_DEV_NUM];
  int                             resv_dev_int[RESERVED_DEV_NUM];
} awb_preflash_enhance_adapter;

/******************************************************************************
** Feature name: awb_dynamic_frame_skip_type
** Dynamic frame skip control
** Variable name: enable
** Configure dyanmic frame skip values from tuning
** Default value: 0
** Data range: 0 - Use default value, 1 - Use tuning values
**
** Variable name: high_skip_value
** High frame skip value
** Default value: 2
** Data range: [0, 30]
**
** Variable name: low_frame_skip
** Low frame skip value
** Default value: 1
** Data range: [0, 30]
**
** Variable name: min_frame_skip
** Minimum frame skip value
** Default value: 0
** Data range: [0, 30]
**
** Variable name: scene_change_high_threshold
** Scene change high threshold
** Default value: 3.00000f
** Data range: [0.0f, 10.0f]
**
** Variable name: scene_change_low_threshold
** Scene change low threshold
** Default value: 1.00000f
** Data range: [0.0f, 10.0f]
******************************************************************************/
typedef struct
{
  int     enable;
  int     high_skip_value;
  int     low_frame_skip;
  int     min_frame_skip;
  float   scene_change_high_threshold;
  float   scene_change_low_threshold;
} awb_dynamic_frame_skip_adapter;

/******************************************************************************
** Feature name: awb_correlation_frame_skip_type
** Correlation based frame skip control
** Variable name: enable
** Enable correlation based frame skip
** Default value: 0
** Data range: 0 - disable, 1 - enable
**
** Variable name: correlation_threshold
** Correlation stable threshold. If correlation factor is more than threshold,
** scene is considered as stable and core algorithm processing will be skipped.
** Higher the value more strict criteria for stable scene and vice versa.
** Default value: 0.95
** Data range: [0, 1]
**
** Variable name: skip_threshold
** Skip threshold. This threshold is used in conjuction with DFS "high_skip_value".
** Total frame skip will be DFS (high_skip_value + 1) * skip_threshold.
** Total frame skip indicates how many max frames of core algo processing will
** be skipped for stable scene.
** Default value: 5
** Data range: [1, 10]
******************************************************************************/
typedef struct
{
  boolean enable;
  float   correlation_threshold;
  int     skip_threshold;
} awb_correlation_frame_skip_type;



#define MAX_WB_REF_PNT_LINE_COUNT           100
#define MAX_WB_REF_PNT_COL_COUNT            14
#define MAX_WB_REF_PNT_TRIAD_LINE_COUNT     200
#define MAX_WB_REF_PNT_TRIAD_COL_COUNT      6
#define MAX_WB_REF_PNT_RG_TRIIDX_NUM        4

/******************************************************************************
** Variable name: wb_pnt_length
** Description: Reserved parameter. Not editable
** Default value: 14
** Data range:
**
** Variable name: wb_triad_length
** Description: Reserved parameter. Not editable
** Default value: 6
** Data range:
**
** Variable name: wb_pnt_idx_rg
** Description: Reserved parameter. Not editable
** Default value: 0
** Data range:
**
** Variable name: wb_pnt_idx_bg
** Description: Reserved parameter. Not editable
** Default value: 1
** Data range:
**
** Variable name: wb_pnt_idx_wb
** Description: Reserved parameter. Not editable
** Default value: 2
** Data range:
**
** Variable name: wb_pnt_num_wb
** Description: Reserved parameter. Not editable
** Default value: 6
** Data range:
**
** Variable name: wb_ref_pnt
** Description: Reserved parameter. Not editable
** Default value: all -1
** Data range: [-1, 15.9f]
**
** Variable name: wb_ref_pnt_cnt
** Description: Reserved parameter. Not editable
** Default value: 100
** Data range:
**
** Variable name: wb_ref_pnt_rg_min
** Description: Reserved parameter. Not editable
** Default value: 0.0f
** Data range: [0, 1]
**
** Variable name: wb_ref_pnt_rg_max
** Description: Reserved parameter. Not editable
** Default value: 1.0f
** Data range: [0, 1]
**
** Variable name: wb_ref_pnt_rg_triidxnum
** Description: Reserved parameter. Not editable
** Default value: 4
** Data range:
**
** Variable name: wb_ref_pnt_rg_triidx
** Description: Reserved parameter. Not editable
** Default value: all 0
** Data range: [0, 197]
**
** Variable name: wb_ref_pnt_triad
** Description: Reserved parameter. Not editable
** Default value: all 255
** Data range: [0,255]
**
** Variable name: wb_ref_pnt_triad_cnt
** Description: Reserved parameter. Not editable
** Default value: 200
** Data range:
******************************************************************************/
typedef struct
{
  /* structure length*/
  int             wb_pnt_length;
  int             wb_triad_length;

  /* structure index for triangle method*/
  int             wb_pnt_idx_rg;
  int             wb_pnt_idx_bg;
  int             wb_pnt_idx_wb;
  int             wb_pnt_num_wb;

  /* reference points*/
  float           wb_ref_pnt[MAX_WB_REF_PNT_LINE_COUNT*MAX_WB_REF_PNT_COL_COUNT];
  int             wb_ref_pnt_cnt;
  float           wb_ref_pnt_rg_min;
  float           wb_ref_pnt_rg_max;

  int             wb_ref_pnt_rg_triidxnum;
  int             wb_ref_pnt_rg_triidx[MAX_WB_REF_PNT_RG_TRIIDX_NUM];

  /* reference triangles*/
  unsigned char   wb_ref_pnt_triad[MAX_WB_REF_PNT_TRIAD_LINE_COUNT*MAX_WB_REF_PNT_TRIAD_COL_COUNT];
  int             wb_ref_pnt_triad_cnt;

  /* rg(bg) module calibration factor.
   * 16 different rg and bg calibration factors that are corresponding to reference points*/
  float           wb_cal_factor[Q3A_AWB_AGW_MAX_LIGHT * 2];
} WBReferencePointCaliData_adapter;

/******************************************************************************
** Feature name:awb_algo_structure
** All tuning parameter definitions for AWB algorithm
******************************************************************************/
typedef struct
{
  awb_basic_tuning_adapter                awb_basic_tuning;                           /* basic tuning parameters*/
  awb_weight_vector_adapter               awb_weight_vector;                          /* awb weight vectors including illuminant and distance weight vectors*/
  awb_green_struct_adapter                awb_green_zone_param;                       /* green zones and mapping parameters*/
  awb_dark_threshold_adapter              awb_dark_threshold;                         /* Thresholds for rejecting dark stats*/
  awb_temporal_tuning_adapter             awb_temporal_tuning;                        /* temporal heuristic parameters*/
  awb_heuristic_control_adapter           awb_heuristic_control;                      /* heuristic control parameters*/
  awb_heuristic_weight_adapter            awb_heuristic_weight[HEURISTIC_WEIGHT_NUM]; /* Decision weights for heuristic functions*/
  awb_white_world_struct_adapter          awb_white_world;                            /* White world decision parameters*/
  awb_decision_boundary_adapter           awb_decision_boundary;                      /* awb decision boundaries*/
  awb_cluster_tuning_adapter              awb_clusters_tuning;                        /* cluster control parameters*/
  awb_weight_lowlight_warm_adapter        awb_weight_lowlight_warm;                   /* A and Horizon lights warming parameters*/
  awb_CCM_adapter                         awb_CCM_control;                            /* AWB driven CCM parameter*/
  awb_preflash_adapter                    awb_preFlash_param;                         /* AWB preflash tuning parameters*/
  awb_led_tuning_adapter                  awb_led_tuning;                             /* LED tuning (excluding preFlash)*/
  awb_extended_outdoor_heuristic_adapter  awb_extended_outdoor_heuristic;             /* awb outdoor heuristics with extended tuning parameters*/
  awb_single_color_tracking_adapter       awb_single_color_tracking;                  /* single color tracking parameters*/
  awb_misleading_zone_process_adapter     awb_misleading_color_zone;                  /* misleading color zone*/
  awb_golden_module_info_adapter          awb_golden_module_info;                     /* golden module information*/
  awb_flow_control_adapter                awb_flow_control;                           /* awb flow control parameters*/
  awb_blue_tuning_adapter                 awb_blue_tuning;                            /* blue color tuning (sky, snow blue, and beach blue)*/
  awb_skin_control_adapter                awb_skin_control;                           /* skin color control*/
  awb_aux_sensor_adapter                  awb_aux_sensor;                             /* auxiliary sensor control*/
  awb_front_camera_ctrl_adapter           awb_front_camera_control;                   /* Front camera control parameters*/
  awb_MWB_adapter                         awb_MWB;                                    /* Manual White Balance parameters*/
  awb_dynamic_convergence_adapter         awb_dynamic_convergence;                    /* Dynamic convergence with frame skip*/
  awb_gyro_assistance_adapter             awb_gyro_assisted_convergence;              /* gyro assisted convergence*/

  /* Extended Parameter */
  awb_special_weights_adapter             awb_special_weights_for_lab;                /* Special weights for lab detection*/
  awb_extreme_cct_adapter                 awb_extreme_cct_control;                    /* Extreme high/low cct condition*/
  awb_preflash_enhance_adapter            awb_preflash_enhance_control;               /* Enhanced preflash*/
  awb_dynamic_frame_skip_adapter          awb_dynamic_frame_skip;                     /* Dynamic frame skip control*/
  awb_correlation_frame_skip_type         awb_correlation_frame_skip;                 /* Correlation Based frame skip control*/
  WBReferencePointCaliData_adapter        wb_refpnt_cali_data;                        /* WBReferencePointCaliData*/
  boolean                                 awb_algo_improvement_enable;                /* set to true if newer and improved algorithm is being used*/
  awb_dualcam_sync_adapter                awb_dualcam_sync;
  awb_reserved_data_type                  awb_reserved_data;                          /* reserved data for AWB 6.x */
} awb_algo_tuning_adapter;

/******************************************************************************
*******************************************************************************
**  AWB related params end
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
**  ASD related params start
*******************************************************************************
******************************************************************************/

typedef struct
{

    unsigned short bits_depth;
    unsigned short entries_num;
    /* 14-bit, unsigned */
    unsigned short gamma_G[GAMMA_TABLE_SIZE];
} asd_gamma_table_adapter;

typedef struct
{
    snow_scene_detect_type      snow_scene_detect;
    backlit_scene_detect_type   backlit_scene_detect;
    landscape_scene_detect_type landscape_scene_detect;
    portrait_scene_detect_type  portrait_scene_detect;
    auto_hdr_detect_type        auto_hdr_detect;
    hazy_scene_detect_type      hazy_scene_detect;
    asd_gamma_table_adapter     gamma_table;
} asd_algo_tuning_adapter;

/******************************************************************************
*******************************************************************************
**  ASD related params end
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
**  AFD related params starts
*******************************************************************************
******************************************************************************/

typedef struct
{
    int             AFD_continuous_enable;
    float           std_threshold;
    unsigned char   percent_threshold;
    unsigned long   diff_threshold;
    unsigned long   frame_ct_threshold;
    unsigned char   num_frames;
    unsigned char   frame_skip;
    unsigned long   num_rows;
    unsigned char   num_frames_settle;
    unsigned char   num_peaks_threshold;
    float           INTL_adj_factor;
    int             start_antbanding_table;
    float           max_exp_time_ms;

    /* static band detection */
    int             thldLevel2Ratio;        /* energy ratio threshold of level 2 processing */
    int             thldLevel3Ratio;        /* energy ratio threshold of level 3 processing */
    int             thldEkL;                /* lower energy threshold of k_th level processing */
    int             thldEkU;                /* energy upper threshold of k_th level processing */
    int             thldCounterL3;          /* positive frame counter threshold for level 3 detection */
    int             thldCounterL2;          /* positive frame counter threshold for level 2 detection */
    int             LevelDepth;             /* processing level2 */
} afd_algo_tuning_adapter;

/******************************************************************************
*******************************************************************************
**  AFD related params end
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
**  Image Stabilization related params start
*******************************************************************************
******************************************************************************/
typedef struct
{
  boolean      eis_enable;
  unsigned int focal_length;
  unsigned int gyro_pixel_scale;
  float gyro_noise_floor;
  uint32_t gyro_frequency;
  float virtual_margin;
  int32_t rs_offset_1;
  int32_t rs_offset_2;
  int32_t rs_offset_3;
  int32_t rs_offset_4;
  int32_t s3d_offset_1;
  int32_t s3d_offset_2;
  int32_t s3d_offset_3;
  int32_t s3d_offset_4;
  float rs_threshold_1;
  float rs_threshold_2;
  float rs_threshold_3;
  float s3d_threshold_1;
  float s3d_threshold_2;
  float s3d_threshold_3;
  uint32_t rs_time_interval_1;
  uint32_t rs_time_interval_2;
  uint32_t rs_time_interval_3;
  uint32_t rs_time_interval_4;
  uint64_t reserve_1;
  uint64_t reserve_2;
  uint64_t reserve_3;
  uint64_t reserve_4;
  int rs_interval_offset;
  int s3d_interval_offset;
} is_algo_tuning_adapter;

/******************************************************************************
*******************************************************************************
**  Image Stabilization related params end
*******************************************************************************
******************************************************************************/

typedef struct
{
    /* TODO: temporarily use the old types until the wrapper has been extended for these types as well*/
    ChromatixVersionInfoType                    chromatix_version_info;
    AAA_version_type                            aaa_version;
    awb_algo_tuning_adapter                     AWB;
    aec_algo_tuning_adapter                     AEC;
    afd_algo_tuning_adapter                     AFD;
    asd_algo_tuning_adapter                     ASD;
    af_algo_tune_parms_adapter_t                AF;
    chromatix_aaa_sync_type                     aaa_sync_data;
    is_algo_tuning_adapter                      IS;
#ifdef _ANDROID_
    pthread_rwlock_t                            lock;
#endif
} chromatix_3a_parms_wrapper;

/******************************************************************************
*******************************************************************************
**  Function Declarations
*******************************************************************************
******************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif
/*void af_tuning_pdaf_reserve_parameters_read(void* p_params, af_tuning_algo_t* af_tuning_algo);*/
    boolean parse_AF_chromatix(void* af_tuning_info, af_algo_tune_parms_adapter_t* tuning_adapter);

    boolean parse_AEC_chromatix(void* aec_tuning_info, aec_algo_tuning_adapter* tuning_adapter);

    boolean parse_AWB_chromatix(void* awb_tuning_info, awb_algo_tuning_adapter* tuning_adapter);

    boolean parse_AFD_chromatix(void* afd_tuning_info, afd_algo_tuning_adapter* tuning_adapter);

    boolean parse_ASD_chromatix(void* asd_tuning_info, asd_algo_tuning_adapter* tuning_adapter);

    boolean parse_EIS_chromatix(void* is_tuning_info, is_algo_tuning_adapter* tuning_adapter);

    boolean parse_chromatix(void* chromatix_ptr, chromatix_3a_parms_wrapper* chromatix_adapter);

    boolean parse_chromatix_iot(void* chromatix_ptr, chromatix_3a_parms_wrapper* chromatix_adapter);

    boolean parse_chromatix_vfe(void* chromatix_ptr, chromatix_3a_parms_wrapper* chromatix_adapter);

    void* af_get_value_monitor_bank(void);

#ifdef __cplusplus
}
#endif

#endif /*__STATS_CHROMATIX_WRAPPER_H__*/


