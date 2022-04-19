/* awb_core.h
 *                                                                   .
 * Copyright (c) 2014-2016, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __AWB_CORE_H__
#define __AWB_CORE_H__

/* ==========================================================================
                     INCLUDE FILES FOR MODULE
========================================================================== */
// 3A common include files.
#include "q3a_common_types.h"
#include "q3a_common_stats.h"

#include "stats_chromatix_wrapper.h"
/* ==========================================================================
                       Preprocessor Definitions and Constants
========================================================================== */
/**
 * Defines the size of the AWB decision map.
 */
#define AWB_DECISION_MAP_SIZE (64)

/**
 * Defines the number of rows in the color correction matrix (CCM).
 */
#define AWB_NUM_CCM_ROWS (3)

/**
 * Defines the number of columns in the color correction matrix (CCM).
 */
#define AWB_NUM_CCM_COLS (3)

/**
 * Defines the number of measurements to capture for each data point. The data point is then
 * calculated as the average of all measurements.
 */
 #define DUAL_LED_CALIB_MEASUREMENT_COUNT 3

//Q3A_AWB_AGW_MAX_LIGHT is defined in the chromatix.h
// match AWB_NUMBER_OF_REFERENCE_POINT with Q3A_AWB_AGW_MAX_LIGHT
#define AWB_NUMBER_OF_REFERENCE_POINT Q3A_AWB_AGW_MAX_LIGHT
#define AWB_MAX_NUMBER_OF_RG_BG_POINT_SIZE 30
#define AWB_ACCEPTABLE_ERROR 0.0078f


/* ==========================================================================
                       Static Declarations
========================================================================== */


/* ==========================================================================
                       Type Declarations
========================================================================== */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Defines an handle to the AWB core.
 */
typedef void* awb_handle_type;

typedef chromatix_manual_white_balance_adapter awb_rgb_gains_type;

typedef chromatix_awb_gain_adj_adapter awb_gain_adjust_type;

typedef chromatix_color_correction_adapter awb_ccm_test_output_type;

/**
 * Defines the awb sync modes for
 * of the dual camera.
 */
typedef enum {
  AWB_SYNC_MODE_FIXED_GAIN_MAPPING,
  AWB_SYNC_MODE_OVERLAP_COLOR_MAPPING,
  AWB_SYNC_MODE_FIXED_GAIN_MAPPING_WITH_OVERLAP_COLOR_INFO,
  AWB_SYNC_MODE_MAX,
} awb_sync_mode_t;

/**
 * Defines the awb overlap ROI information
 * of the dual camera.
 */
typedef struct _awb_overlap_roi_t {
  bool  enabled;
  int  x;
  int  y;
  int  dx;
  int  dy;
  int  frameWidth;
  int  frameHeight;
} awb_overlap_roi_t;

/**
 * Defines the awb overlap sample information
 */
typedef struct _awb_overlap_samples_t {
  int  xStart;
  int  yStart;
  int  xEnd;
  int  yEnd;
} awb_overlap_samples_t;

/**
 * Defines the awb overlap Color information
 */
typedef struct _awb_overlap_color_info_t {
  float  rg_ratio;
  float  bg_ratio;
  int    sample_count;
} awb_overlap_color_info_t;

/**
 * Defines the awb Reference Points
 */
typedef struct awb_reference_point_info{
  float    rg_table[AWB_NUMBER_OF_REFERENCE_POINT];
  float    bg_table[AWB_NUMBER_OF_REFERENCE_POINT];
}awb_reference_point_info;

/**
 * Defines the dual cam awb sync information
 */
typedef struct _awb_dualcam_sync_info_t {
  awb_sync_mode_t           sync_mode;          /**< AWB Sync modes */
  awb_reference_point_info  reference_points;   /**< Default Reference Points of Master */
  awb_custom_reference_point_info custom_reference_points;   /**<Customer  Reference Points of Master */
  awb_overlap_color_info_t  overlap_color_info; /**< Overlap Color Info from master*/
  float                     lux_level;  /**< Lux level in Master camera. */
  float                     zoom_level;  /**< Zoom level in Master camera. */
  float                     extreme_stat_cnt_ratio; /**< extream stat count ratio*/
  trigger_point_adapter     weight_trigger;
  float                     weight_fcm[4];
  float                     zoom_level_start;
  float                     zoom_level_end;
} awb_dualcam_sync_info_t;


/**
 * Defines the awb Dual Cam RGBG reference table information
 */
typedef struct awb_dual_cam_refer_table_rgbg_info{
  int  refer_point_size;
  float master_refer_table_rg[AWB_MAX_NUMBER_OF_RG_BG_POINT_SIZE];
  float master_refer_table_bg[AWB_MAX_NUMBER_OF_RG_BG_POINT_SIZE];
  float slave_refer_table_rg[AWB_MAX_NUMBER_OF_RG_BG_POINT_SIZE];
  float slave_refer_table_bg[AWB_MAX_NUMBER_OF_RG_BG_POINT_SIZE];
}awb_dual_cam_refer_table_rgbg_info;

/**
 * Defines the format of the flash information.
 */
typedef struct
{
  boolean                           flash_needed; /**< Flag indicates, compute the gains for flash snapshot. */
  q3a_flash_sensitivity_type        sensitivity;  /**< Flash sensitivity information. */
} awb_snap_est_info_type;

/**
 * Defines the dual LED calibration states. Indicates the LED states for LED 1 and
 * LED 2 respectively when calibration is enabled.
 */
typedef enum{
  AWB_DUAL_LED_CALIB_ON_HI_LO = 0,
  AWB_DUAL_LED_CALIB_ON_MED_MED,
  AWB_DUAL_LED_CALIB_ON_LO_HI,
  AWB_DUAL_LED_CALIB_OFF,

  // Number of ON states for which calibration data will be collected
  AWB_DUAL_LED_CALIB_ON_STATE_COUNT = AWB_DUAL_LED_CALIB_OFF,
} awb_dual_led_calibration_state;

/**
 * Container for a range of data points for a single LED state
 */
typedef struct {
  /* Keeps track of the number of times the current data point has been measured during
   * calibration. This is used to compute the average value for that data point, and its max
   * value will be deteremined by DUAL_LED_CALIB_MEASUREMENT_COUNT */
  int measured_count;

  /* Index into the current data point */
  int index;

  float rg_ratios[DUAL_LED_CALIB_MEASUREMENT_COUNT]; /**< measured rg ratios */
  float bg_ratios[DUAL_LED_CALIB_MEASUREMENT_COUNT]; /**< measured bg ratios */
}awb_dual_led_calib_measurement;

/**
 * Encapsulates an averaged data point measurements for a single Dual LED calibration state
 */
typedef struct{
  boolean is_valid;
  float rg_ratio;     /**< averaged rg ratio */
  float bg_ratio;     /**< averaged bg ratio */
} awb_dual_led_calibration_point;

/**
 * Dual LED calibration parameters
 */
typedef struct{
  /* Indicates whether calibration mode is enabled */
  boolean is_calibration_mode;

  /* Indicates the current LED state during calibration */
  awb_dual_led_calibration_state state;

  /* Calibration data points before linearization. These are either loaded from file or
   * captured during calibration */
  awb_dual_led_calibration_point data[AWB_DUAL_LED_CALIB_ON_STATE_COUNT];
} awb_dual_led_calibaration;

/**
 * Defines the exposure params that are input to the AWB algorithm.
 */
typedef struct
{
  uint16                      exp_tbl_val;            /**< The maximum exposure index. */
  boolean                     flash_on;               /**< Flag that indicates that flash estimation has to be used. */
  float                       brightness_value;       /**< The brighness value(BV) computed by AEC. */
  float                       time_value;             /**< The time value (TV) computed by AEC. */
  float                       aperture_value;         /**< The aperture value (AV) computed by AEC. */
  float                       speed_value;            /**< The speed value (SV) computed by AEC. */
  float                       exposure_value;         /**< The exposure value (EV) computed by AEC. */
  uint32                      frame_luma;             /**< The frame luma computed by AEC. */
  uint32                      target_luma;            /**< The target luma computed by AEC. */
  float                       lux_index;              /**< The lux index computed by AEC. */
  float                       total_drc_gain;         /**< The drc gain computed by AEC. */
  uint16                      exp_index;              /**< The current exposure index. */
  float                       cur_real_gain;          /**< The current real gain computed by AEC. */
  uint32                      led_state;              /**< The current real gain computed by AEC. */
  //John0312:face_awb_sim
   float*                      face_x;
   float*                      face_y;
   float*                      face_width;
   float*                      face_height;
   uint16                      face_frm_width;
   uint16                      face_frm_height;
} awb_aec_input_params_type;

/**
 * Defines the list of illuminants that the core algorithm supports.
 */
typedef enum
{
  AWB_ILLUMINANT_D75 = 0,         /**< Daylight, 7500K CCT. */
  AWB_ILLUMINANT_D65,             /**< Daylight, 6500K CCT. */
  AWB_ILLUMINANT_NOON,            /**< Daylight, approximately 5500K CCT. */
  AWB_ILLUMINANT_D50,             /**< Daylight, 5000K CCT. */

  AWB_ILLUMINANT_CW,              /**< Cold fluorescent, .*/
  AWB_ILLUMINANT_TL84,            /**< Warm fluorescent. */

  AWB_ILLUMINANT_A,               /**< Incandescent light. */
  AWB_ILLUMINANT_H,               /**< Horizon light. */

  AWB_ILLUMINANT_CUST_FLO,        /**< Custom fluorescent. */
  AWB_ILLUMINANT_CUST_DAY,        /**< Custom daylight. */
  AWB_ILLUMINANT_CUST_A,          /**< Custom incandescent. */
  AWB_ILLUMINANT_U30,             /**< U30. */

  AWB_ILLUMINANT_CUST_DAY1,       /**< Custom daylight 1. */
  AWB_ILLUMINANT_CUST_DAY2,       /**< Custom daylight 2. */

  AWB_ILLUMINANT_CUST_FLO1,       /**< Custom fluorescent 1. */
  AWB_ILLUMINANT_CUST_FLO2,       /**< Custom fluorescent 2. */

  AWB_ILLUMINANT_LAST,

  AWB_ILLUMINANT_MAX = 0xFF,
  AWB_ILLUMINANT_INVALID = AWB_ILLUMINANT_MAX
} awb_illuminant_type;

/** awb_interpolation_gain_params_t:
 **/
typedef struct _awb_interpolation_gain_params_t {
  awb_sync_mode_t           sync_mode;          /**< AWB Sync modes */
  awb_rgb_gains_type        gains;              /**< Red, Green, and Blue gains output by the AWB core. */
  awb_illuminant_type       decision;           /**< Decision output by the AWB core. */
  uint32                    color_temperature;  /**< The CCT output by the AWB core. */
  awb_reference_point_info  reference_points;   /**< Default Reference Points of Master */
  awb_custom_reference_point_info custom_reference_points;   /**<Customer  Reference Points of Master */

  awb_overlap_color_info_t  master_overlap_color_info; /**< Overlap Color Info from master*/
  awb_overlap_color_info_t  current_overlap_color_info; /**< Overlap Color Info from slave*/

  float                     zoom_level;  /**< Zoom level in Master camera. */
  float                     lux_level;  /**< Lux level in Master camera. */
  float                     extreme_stat_cnt_ratio; /**< extream stat count ration*/
  trigger_point_adapter     weight_trigger;
  float                     zoom_level_start;
  float                     zoom_level_end;
  float                     weight_fcm[4];

  float                     decision_point[2];
  float                     unadjusted_decision_point[2];
  awb_rgb_gains_type        unadjusted_gains;   /**< Unadjusted gain*/
  chromatix_color_correction_adapter curr_ccm;  /**< Current CCM*/
} awb_interpolation_gain_params_t;

/** awb_role_switch_params_t:
 **/
typedef struct _awb_role_switch_params_t {
  boolean                            master;           /**< If Algorithm is master or slave*/
  awb_interpolation_gain_params_t    gain_params;      /**< Interpolated gain params */
}awb_role_switch_params_t;

/**
 * Defines the format of the AWB decision map.
 */
typedef struct
{
  awb_illuminant_type    map[AWB_DECISION_MAP_SIZE];  /**< Pointer to the AWB decision map. */
} awb_decision_map_type;

/**
 * Defines the format of the color correction matrix.
 */
typedef struct
{
  boolean     override_ccm;                             /**< Flag indicates CCM output is to be used. */
  float       ccm[AWB_NUM_CCM_ROWS][AWB_NUM_CCM_COLS];  /**< The color correction matrix */
  float       ccm_offset[AWB_NUM_CCM_ROWS];             /**< The offsets for color correction matrix */
} awb_ccm_type;

/**
 * Defines the format of the gyro input into the algorithm.
 */
typedef struct
{
  boolean         enabled;    /**< Flag that indicates whether gyro is enabled. */
  float           x_axis;     /**< X axis data. */
  float           y_axis;     /**< Y axis data. */
  float           z_axis;     /**< Z axis data. */
} awb_gyro_info_type;

/**
 * Defines the format of the UV indicator.
 */
typedef struct
{
    uint32  uv_indicator;   /**< UV sensor data. */
} awb_uv_sensor_info_type;

typedef struct
{
  int32               led1_low_setting;       /**< LED1 current value for pre-flash */
  int32               led2_low_setting;       /**< LED2 current value for pre-flash */
  int32               led1_high_setting;      /**< LED1 current value for main-flash */
  int32               led2_high_setting;      /**< LED2 current value for main-flash */
} awb_dual_led_setting_type;

typedef struct
{
  float                       dual_led_flux_gain;     /**< LED flux gain value for AEC */
  awb_dual_led_setting_type   dual_led_settings;      /**< LED current setting for sensor. */
  awb_dual_led_calibaration   dual_led_calib;         /**< Dual LED Calibration data and settings. */
} awb_dual_led_data_type;

/**
 * This structure defines the interface to comunicate HW supported stats and
 * the stats that aec algo will request the HW to enable.
 *
 **/
typedef struct {
  uint32_t              supported_stats_mask;    /**< HW supported stats mask (input)*/
  q3a_stats_stream_type stats_stream_type;       /**< ISP streaming type - offline or online (input) */
  uint32_t              enable_stats_mask;       /**< stats that AWB requires HW to enable (output) */
  uint32                supported_rgn_skip_mask; /**< HW supported region skip pattern (input) */
  q3a_rgn_skip_pattern  enable_rgn_skip_pattern; /**< skip pattern that AWB requires HW to enable (output) */
} awb_request_stats_t;

/**
 * This structure contains the inputs to the AWB algorithm.
 */
typedef struct
{
  awb_aec_input_params_type       exposure_params;    /**< Exposure parameters. */
  awb_gyro_info_type              gyro_info;          /**< Input from the gyro. */
  awb_uv_sensor_info_type         uv_info;            /**< Input from the UV sensor. */
  q3a_core_roi_configuration_type roi_info;           /**< ROI info from face/touch/fixed ROI. */
  awb_snap_est_info_type          snap_est_info;      /**< flash info for snapshot estimation. */
  q3a_core_bg_stats_type          bg_stats;           /**< Bayer grid stats. */
} awb_input_params_type;

/**
 * Defines the AWB output paramters.
 */
typedef struct
{
  boolean                 is_converged;       /**< AWB converge status */
  awb_rgb_gains_type      gains;              /**< Red, Green, and Blue gains output by the AWB core. */
  uint32                  color_temperature;  /**< The CCT output by the AWB core. */
  awb_illuminant_type     decision;           /**< Decision output by the AWB core. */
  awb_decision_map_type   decision_map;       /**< Map of AWB decisions output by the core. */
  awb_ccm_type            ccm;                /**< Color correction matrix to be used. */
  awb_ccm_type            snapshot_ccm;       /**< Color correction matrix to be used for snapshot. */
  awb_dual_led_data_type  dual_led_data;      /**< Dual led data */
  awb_rgb_gains_type      snap_gains;         /**< Gains for snapshot */
  uint32                  snap_color_temp;    /**< The CCT output for snapshot */
  awb_rgb_gains_type      unadjust_gains;     /**< Unjusted Red, Green, and Blue gains output by the AWB core. */
  float                   conv_speed;         /**< Convergence speed */
  awb_dualcam_sync_info_t  dualcam_sync;      /**< Dual Cam Sync Info */
  boolean                 led_calc_done;
} awb_output_params_type;

/*
 * Defines the reserved tuning parameters that are input into AWB.
 */
typedef struct
{
  float     white_current_decision_weight;  /** White decision weight. */
  float     a_h_line_boundary;              /**< Boundary of the line joining A and H. */
  float     day_f_line_boundary;            /**< Boundary of the line joining D65 and F. */
  float     d75_d65_line_boundary;          /**< Boundary of the line joining D75 and D65. */
  float     d65_d50_line_boundary;          /**< Boundary of the line joining D65 and D50. */
  float     d65_d50_shifted_line_boundary;  /**< Boundary of the line joining shifted D65 and shifted D50. */
  float     f_a_line_boundary;              /** Boundary of the line joining F and A. */
  float     cw_tl84_line_boundary;          /**< Boundary of the line joining CW and TL84. */
  boolean   d2cw_enable;                    /**< Flag when set to TRUE enables the squared distance to CW check. */
  boolean   A_H_warmup_enable;              /**< Flag when set to TRUE enables the warmup TODO. */
  float     dual_led_intersect_slope;       /**< Slope of the dual LED line. */
  float     led1_r_adj;                     /**< Red gain adjust of LED1. */
  float     led1_b_adj;                     /**< Blue gain adjust of LED1. */
  float     led2_r_adj;                     /**< Red gain adjust of LED2. */
  float     led2_b_adj;                     /**< Blue gain adjust of LED2. */
  boolean   face_enable;                    /**< Flag to enable and disable the face feature. */
  awb_overlap_roi_t overlap_roi_info;       /**< Dual Camera Overlap ROI information*/
} awb_reserved_tuning_params_type;

 /*
  * Defines the parameters which will be used in next session as a starting point to improve the convergence.
  */
typedef struct{
  boolean                               enable;     /** True indicates, start AWB using CCT and gains of previous session. */
  uint32_t                              session_id; /** Session ID. */
  boolean                               first_init; /** First camera init. */
  chromatix_manual_white_balance_type   gains;      /** Previous session gains. */
  uint32_t                              color_temp; /** Previous session color temperature. */
  awb_ccm_type                          ccm;        /**< Color correction matrix to be used. */
}awb_stored_params_type;

/**
 * Defines the AWB config parameters.
 */
typedef struct
{
  boolean                                       reload_needed;              /**< Recalculate awb bayer setup. */
  const awb_algo_tuning_adapter*                tuning_params;              /**< Bayer AWB tuning parameters. */
  const awb_rgb_gains_type*                     initial_white_balance;      /**< AWB initial gains. */
  const awb_rgb_gains_type*                     led_flash_white_balance;    /**< AWB led white balance gains. */
  const awb_reserved_tuning_params_type*        reserved_tuning_param;      /**< Reserved tuning params. */
  const awb_dual_led_calibaration*              dual_led_calib;             /**< Dual LED calibration data. */
  awb_stored_params_type*                       stored_params;              /**< Stored parameters. */
} awb_tuning_params_type;

 /**
  * LED output parameters
  */
typedef struct
{
  unsigned short LED1_setting;
  unsigned short LED2_setting;
  float          rg_ratio;
  float          bg_ratio;
  float          flux;
  float          first_entry_ratio;
  float          last_entry_ratio;
} awb_led_combination_type;

typedef struct {
  awb_led_combination_type low_setting;
  awb_led_combination_type high_setting;
  awb_led_combination_type full_setting;
} awb_core_dual_led_settings_t;

typedef struct{
  int  aec_settled;
  int  use_led_estimation;
  int  led_state;
  bool flash_on;
  float cur_real_gain;

  awb_core_dual_led_settings_t dual_led;
  float Dual_LED1_influence_ratio;
  q3a_custom_data_t custom_param_awb;

  uint8_t roi_count;
  float awb_roi_x[Q3A_CORE_MAX_ROI_COUNT];
  float awb_roi_y[Q3A_CORE_MAX_ROI_COUNT];
  float awb_roi_dx[Q3A_CORE_MAX_ROI_COUNT];
  float awb_roi_dy[Q3A_CORE_MAX_ROI_COUNT];
  float awb_roi_weight[Q3A_CORE_MAX_ROI_COUNT];
  float frm_width;
  float frm_height;
  int faceawb;

} awb_set_aec_parms_core;

/**
 * Defines the parameters that can be set on the core AWB algorithm.
 */
typedef enum
{
  AWB_SET_PARAM_TUNING_DATA = 0,              /**< Tuning data information. */
  AWB_SET_PARAM_BYPASS_TEMPORAL_FILTER,       /**< Bypass temporal filter option. */
  AWB_SET_PARAM_DEBUG_DATA_ENABLE,            /**< Enable the debug data. */
  AWB_SET_PARAM_DEBUG_DATA_LEVEL,             /**< Sets the level for the debug data. */
  AWB_SET_PARAM_DUAL_LED_FORCE_IDX,           /**< Forced dual led table idx. */
  AWB_SET_PARAM_OPERATIONAL_MODE,             /**< Camera operational mode. */
  AWB_SET_PARAM_DUALLED_CALIB_MODE,           /**< Dual LED Calibration mode. */
  AWB_SET_PARAM_AEC_PARMS,                    /**< AEC parameters. */
  AWB_SET_PARAM_INSTANT_AEC_TYPE,             /**< Instant AEC Type */
  AWB_SET_PARAM_HDR_MODE,                     /**< HDR mode parameters. */
  AWB_SET_PARAM_CONV_SPEED,                   /**< Awb convergence speed. */
  AWB_SET_PARAM_ROLE_SWITCH,                  /**< Role Switch Information */
  AWB_SET_PARAM_CUSTOM_EVT_HAL,               /**< Custom HAL event for algo */
  AWB_SET_PARAM_LAST
} awb_set_param_enum_type;

/**
 * Defines the debug data levels supported by the AWB algorithm.
 */
typedef enum
{
  AWB_DEBUG_DATA_LEVEL_NONE = 0,              /**< Disables debug data. */
  AWB_DEBUG_DATA_LEVEL_CONCISE,               /**< Enables concise level of debug data. */
  AWB_DEBUG_DATA_LEVEL_VERBOSE,               /**< Enables verbose level of debug data. */
  AWB_DEBUG_DATA_LEVEL_INVALID = 0xFF         /**< Invalid/Max level. */
} awb_debug_data_level_type;

/**
 * Defines the debug data levels supported by the AWB algorithm.
 */
typedef enum _awb_snapshot_hdr_type {
  AWB_SENSOR_HDR_OFF = 0,                     /**< Disables debug data. */
  AWB_SENSOR_HDR_IN_SENSOR,                   /**< In sensor hdr mode */
  AWB_SENSOR_HDR_DRC,                         /**< zzhdr mode . */
  AWB_SENSOR_HDR_MAX,                         /**< Invalid/Max level */
} awb_snapshot_hdr_type;

/**
 * Defines the format of the set param input structure that is passed to the core algorithm.
 */
typedef struct
{
  awb_set_param_enum_type     param_type; /**< Type of parameter being set. */
  union
  {
    awb_tuning_params_type          tuning_params;
    boolean                         bypass_temporal_filter;
    boolean                         debug_data_enable;
    awb_debug_data_level_type       debug_data_level;
    uint8                           ez_force_dual_led_idx;
    q3a_core_operational_mode_type  op_mode;
    boolean                         dual_led_calib_enabled;
    awb_set_aec_parms_core          aec_parms;
    /* Instant AEC convergence type */
    aec_convergence_type instant_aec_type;
    uint8                           snapshot_hdr;
    float                           conv_speed;
    awb_role_switch_params_t        role_switch;
    q3a_custom_data_t               q3a_custom_data;
  } u;
} awb_set_param_info_type;

/**
 * Defines the parameters that are needed to get CCM from AWB.
 */
typedef struct
{
  uint32                          color_temperature;
  int                             exp_idx;
} awb_get_ccm_info_type;


/**
 * Defines the parameters that can be obtained from the core AWB algorithm.
 */
typedef enum
{
  AWB_GET_PARAM_CCT_FROM_RGB = 0,         /**< CCT value from RGB gains. */
  AWB_GET_PARAM_CCM_FROM_CCT,             /**< CCM value from CCT */
  AWB_GET_PARAM_RGB_FROM_CCT,             /**< RGB values from CCT. */
  AWB_GET_SNAPSHOT_INFO,                  /**< Gains, CCT, and other info for snapshot. */
  AWB_GET_METADATA_INFO,                  /**< Metadata infomation. */
  AWB_GET_EZTUNE_INFO,                    /**< Eztune infomation. */
  AWB_GET_UNIFIED_FLASH,                  /**< Led off gains infomation. */
  AWB_GET_MISLEADING_ZONE_INFO,           /**< Misleading zone co-ordinates. */
  AWB_GET_DUAL_LED_INFO,                  /**< Dual LED information. */
  AWB_GET_RGN_SKIP_PATTERN,               /**< Region skip pattern */
  AWB_GET_INTERPLOATED_GAIN,              /**< Interpolated gain from Master Camera*/
  AWB_GET_PARAM_LAST
} awb_get_param_enum_type;

/**
 * Defines the format of the get param output structures that is output from the core algorithm.
 */
typedef struct
{
  awb_get_param_enum_type     param_type; /**< Type of parameter being obtained. */
  union
  {
    awb_rgb_gains_type              gains;              /**< RGB gains for cct estimation. */
    awb_snap_est_info_type          snap_est_info;      /**< Flash information. */
    awb_get_ccm_info_type           ccm_input_info;
    uint32                          color_temperature;  /**< Color temperature input into the core. */
    uint32                          metadata_size;      /**< Metadata size. */
    uint32                          supported_rgn_skip_mask; /**< HW supported region skip mask */
    awb_interpolation_gain_params_t interploatedGains;
  } u;
} awb_get_param_info_type;

/**
 * Defines the parameters that are output by the core algorithm for snapshot.
 */
typedef struct
{
  awb_output_params_type          output_params;  /**< All generic AWB output params. */
} awb_snapshot_info_type;

/**
 * Defines the parameters that are output by the core algorithm for snapshot.
 */
typedef struct
{
  awb_output_params_type          output_params;  /**< All generic AWB output params. */
  awb_rgb_gains_type              led_off_final_gain;
  uint32                          led_off_color_temp;
} awb_unified_flash_info_type;

/**
 * Defines the metadata infomation data structure
 */
typedef struct
{
  void*                 metadata;               /**< AWB metadata buffer pointer. */
  uint32                metadata_size;          /**< AWB metadata size. */
} awb_metadata_info_type;

/**
 * Defines the format of each misleading zone.
 */
typedef struct
{
    float rg_center;    /**< R/G ratio of center of zone. */
    float bg_center;    /**< B/G ratio of center of zone. */
    float width;        /**< Width of zone. */
    float height;       /**< Height of zone. */
} awb_misleading_zone_coord_type;

/**
 * Defines the format of the misleading zones.
 */
typedef struct
{
    awb_misleading_zone_coord_type  misleading_zone_list[MISLEADING_COLOR_ZONE_NUM]; /**< Co-orcinates of each zone. */
    uint8                           num_zones_in_use;                                /**< Number of zones in use. */
} awb_misleading_zones_info_type;

/**
 * Defines the eztune output data structure
 */
typedef struct {
  boolean  awb_enable;
  boolean  ez_running;
  int      awb_mode;
  int      prev_exp_index;
  int32    outlier_dist2_dayleft;
  int32    outlier_dist2_daytop;
  int      valid_sample_cnt;
  int      n_outlier;
  float    day_rg_ratio;
  float    day_bg_ratio;
  int      day_cluster;
  int      day_cluster_weight_distance;
  int      day_cluster_weight_illuminant;
  int      day_cluster_weight_dis_ill;
  float    f_rg_ratio;
  float    f_bg_ratio;
  int      f_cluster;
  int      f_cluster_weight_distance;
  int      f_cluster_weight_illuminant;
  int      f_cluster_weight_dis_ill;
  float    a_rg_ratio;
  float    a_bg_ratio;
  int      a_cluster;
  int      a_cluster_weight_distance;
  int      a_cluster_weight_illuminant;
  int      a_cluster_weight_dis_ill;
  float    h_rg_ratio;
  float    h_bg_ratio;
  int      h_cluster;
  int      h_cluster_weight_distance;
  int      h_cluster_weight_illuminant;
  int      h_cluster_weight_dis_ill;
  int      sgw_cnt;
  float    sgw_rg_ratio;
  float    sgw_bg_ratio;
  int      green_line_mx;
  int      green_line_bx;
  int      green_zone_top;
  int      green_zone_bottom;
  int      green_zone_left;
  int      green_zone_right;
  float    outdoor_green_rg_ratio;
  float    outdoor_green_bg_ratio;
  float    outdoor_green_grey_rg_ratio;
  float    outdoor_green_grey_bg_ratio;
  int      outdoor_green_cnt;
  int      green_percent;
  float    slope_factor_m;
  int      extreme_b_mag;
  int      nonextreme_b_mag;
  int      ave_rg_ratio_x;
  int      ave_bg_ratio_x;
  int      weighted_sample_rg_grid;
  int      weighted_sample_bg_grid;
  float    weighted_sample_day_rg_ratio;
  float    weighted_sample_day_bg_ratio;
  float    weighted_sample_day_shade_rg_ratio;
  float    weighted_sample_day_shade_bg_ratio;
  float    weighted_sample_day_d50_rg_ratio;
  float    weighted_sample_day_d50_bg_ratio;
  float    weighted_sample_fah_rg_ratio;
  float    weighted_sample_fah_bg_ratio;
  float    white_rg_ratio;
  float    white_bg_ratio;
  int      white_stat_y_threshold_low;
  int      unsat_y_min_threshold;
  int      unsat_y_max;
  int      unsat_y_mid;
  int      unsat_y_day_max;
  int      unsat_y_f_max;
  int      unsat_y_a_max;
  int      unsat_y_h_max;
  float    sat_day_rg_ratio;
  float    sat_day_bg_ratio;
  int      sat_day_cluster;
  float    sat_f_rg_ratio;
  float    sat_f_bg_ratio;
  int      sat_f_cluster;
  float    sat_a_rg_ratio;
  float    sat_a_bg_ratio;
  int      sat_a_cluster;
  float    sat_h_rg_ratio;
  float    sat_h_bg_ratio;
  int      sat_h_cluster;
  float    max_compact_cluster;
  int      count_extreme_b_mcc;
  int      green_zone_right2;
  int      green_line_bx2;
  int      green_zone_bottom2;
  int      output_is_confident;
  int      output_sample_decision;
  float    output_wb_gain_r;
  float    output_wb_gain_g;
  float    output_wb_gain_b;
  float    regular_ave_rg_ratio;
  float    regular_ave_bg_ratio;
  float    cct_awb_bayer;
  int      count_extreme_b;
  float    preview_r_gain;
  float    preview_g_gain;
  float    preview_b_gain;
  float    snapshot_r_gain;
  float    snapshot_g_gain;
  float    snapshot_b_gain;
  int      color_temp;
  int      decision;
  int      samp_decision[64];
  boolean  lock;
  int mix_led_table_index_override;
} awb_output_eztune_data_type;

/**
 * Defines the format of the structure output by the AWB core algorithm
 */
typedef struct
{
  uint32                          param_length;       /**< Param length in bytes. */
  union
  {
    uint32                        color_temperature;  /**< Color temperature. */
    awb_ccm_type                  ccm;                /**< Color correction matrix. */
    awb_rgb_gains_type            gains;              /**< RGB gains. */
    awb_snapshot_info_type        snapshot_info;      /**< Snapshot info. */
    awb_metadata_info_type        metadata_info;      /**< AWB metadata buffer pointer. */
    void*                         eztune_info;        /**< AWB eztune output pointer. */
    awb_dual_led_data_type        dual_led_data;      /**< AWB dual led setting. */
    awb_unified_flash_info_type   unified_flash;  /**< Led Off Final gains*/
    awb_misleading_zones_info_type  misleading_zone_info; /**< Misleading zone information. */
    q3a_rgn_skip_pattern          enable_rgn_skip_pattern; /**< AWB region skip pattern */
    awb_interpolation_gain_params_t interploatedGains;
  } u;
} awb_get_param_output_type;

/**
 * Defines the format of input structure for CCM BET testing.
 */
typedef struct
{
    uint32                      color_temperature;  /**< Color temperature. */
    int                         exp_idx;            /**< Exposure index. */
    boolean                     flash_on;           /**< Flag to indicates flash ON. */
    q3a_flash_sensitivity_type  flash_sensitivity;  /**< Flash sensitivity. */
    awb_dual_led_setting_type   dual_led_settings;  /**< Flash dual LED settings. */
} awb_ccm_test_input_type;

/* ==========================================================================
                       Function Declarations
========================================================================== */
/**
 * Initializes the AWB core module.
 *
 * @param[out] handle Pointer to the handle to the AWB core module.
 *
 * @return Returns Q3A_CORE_RESULT_SUCCESS upon success, any other error upon failure.
 */
q3a_core_result_type awb_init
(
  awb_handle_type* p_handle
);

/**
 * Runs the core AWB algorithm.
 *
 * @param[in] handle    Handle to the AWB core module.
 * @param[in] input     Parameters input into the AWB core module.
 * @param[out] output   Parameters output by the AWB core module.
 *
 * @return Returns Q3A_CORE_RESULT_SUCCESS upon success, any other error upon failure.
 */
q3a_core_result_type awb_process
(
  awb_handle_type                 handle,
  const awb_input_params_type*    input,
  awb_output_params_type*         output
);

/**
 * Sets parameters pertaining to the core AWB algorithm.
 *
 * @param[in] handle                Handle to the AWB core module.
 * @param[in] set_param             Parameters set on the AWB core module.
 *
 * @return Returns Q3A_CORE_RESULT_SUCCESS upon success, any other error upon failure.
 */
q3a_core_result_type awb_set_param
(
  awb_handle_type                 handle,
  const awb_set_param_info_type*  set_param_info
);

/**
 * Gets parameters pertaining to the core AWB algorithm.
 *
 * @param[in] handle                Handle to the AWB core module.
 * @param[in] get_param             Parameters for which info is requested from the AWB core module.
 * @param[out] get_param_output     Parameters output by the AWB core module.
 *
 * @return Returns Q3A_CORE_RESULT_SUCCESS upon success, any other error upon failure.
 */
q3a_core_result_type awb_get_param
(
  awb_handle_type                 handle,
  const awb_get_param_info_type*  get_param_info,
  awb_get_param_output_type*      get_param_output
);

/**
 * @brief De-initializes the AWB core module.
 *
 * This method de-inits the AWB core and frees any memory that had been allocated.
 *
 * @param[in] handle Handle to the AWB core module.
 */
void awb_deinit
(
  awb_handle_type handle
);

/* ==========================================================================
                       Test Function Declarations
========================================================================== */

/**
 * @brief Method to retrieve the AWB CCM
 *
 * This method is used only to test the AWB CCM logic using BET test vectors.
 * This should never be called in a non-test setup and is only used for testing the CCM
 * logic of AWB.
 *
 * @param[in] handle Handle to the AWB core module.
 * @param[in] input Input to AWB CCM module.
 * @param[out] output Contains the CCM computed by AWB CCM logic.
 *
 * @return Returns Q3A_CORE_RESULT_SUCCESS upon success, any other error upon failure.
 */
q3a_core_result_type awb_test_get_ccm
(
  awb_handle_type                   handle,
  const awb_ccm_test_input_type*    input,
  awb_ccm_test_output_type*         output
);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // __AWB_CORE_H__

