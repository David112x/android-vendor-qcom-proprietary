/* aec_core.h
 *
 * Copyright (c) 2014-2016, 2018, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef __AEC_CORE_H__
#define __AEC_CORE_H__

/*===========================================================================

         A E C  C o r e  D a t a  S t r u c t u r e  D e c l a r a t i o n

*//** @file aec_core.h
      @brief Function prototypes that are part of the interface to the core
      AEC algorithm.

      This contains the prototypes that are part of the interface to the
      core AEC algorithm and any macros, constants, and global variables
      that you will need. The methods exposed by the interface are NOT
      thread safe. Thread safety is the responsiblity of the user of the
      interface.

===========================================================================*/

/* ==========================================================================
                     INCLUDE FILES FOR MODULE
========================================================================== */
/* 3A common include files. */
#include "q3a_common_types.h"
#include "q3a_common_stats.h"
#include "q3a_version.h"
#include "stats_chromatix_wrapper.h"
/* ==========================================================================
                       Preprocessor Definitions and Constants
========================================================================== */
/*Staggered HDR capabilities:*/
#define AEC_CORE_NUM_OF_VHDR_FRM_SUPPORTED 2
#define AEC_CORE_MIN_EXP_RATIO 1
#define AEC_CORE_MAX_EXP_RATIO 64


/* ==========================================================================
                       Type Declarations
========================================================================== */
#ifdef __cplusplus
extern "C"
{
#endif

/** aec_handle_type
 * Defines an handle to the AEC core.
 */
typedef void* aec_handle_type;

/* To generate enum from macros */
#define AEC_CORE_GENERATE_ENUM(ENUM) ENUM,
/* To generate string for enum from macros */
#define AEC_CORE_GENERATE_STRING(STRING) #STRING,

/** aec_operation_mode_t
 *  Defines the current mode of operation
 *
 *  @AEC_OPERATION_MODE_NONE: Mask to indicate that operation mode is not set.
 *  @AEC_OPERATION_MODE_INIT: Mask to indicate that operation mode is initialized.
 *  @AEC_OPERATION_MODE_PREVIEW: Mask to indicate that operation is in preview mode.
 *  @AEC_OPERATION_MODE_SNAPSHOT: Mask to indicate that operation is in snapshot mode.
 *  @AEC_OPERATION_MODE_CAMCORDER: Mask to indicate that operation is in camcorder mode.
 *  @AEC_OPERATION_MODE_ZSL: Mask to indicate that operation is in zsl mode.
 */
typedef enum {
  AEC_OPERATION_MODE_NONE = 0,
  AEC_OPERATION_MODE_INIT,
  AEC_OPERATION_MODE_PREVIEW,
  AEC_OPERATION_MODE_SNAPSHOT,
  AEC_OPERATION_MODE_CAMCORDER,
  AEC_OPERATION_MODE_ZSL,
  AEC_OPERATION_MODE_INVALID
} aec_operation_mode_t;

/** aec_fps_mode_t
 * Defines the current fps mode
 *
 * @AEC_FPS_MODE_AUTO: Mask to indicate that fps is in auto mode.
 * @AEC_FPS_MODE_FIXED: Mask to indicate that fps is in fixed mode.
 */
typedef enum
{
  AEC_FPS_MODE_AUTO = 0, /**< Mask to indicate that fps is in auto mode. */
  AEC_FPS_MODE_FIXED     /**< Mask to indicate that fps is in fixed mode */
} aec_fps_mode_t;

typedef enum
{
  TOUCH_EV_AEC_SETTLED = 0,
  TOUCH_EV_AEC_SCENE_CHANGE,
  TOUCH_EV_AEC_STABLE
} aec_touch_ev_status_type;

typedef enum
{
  AEC_CORE_HDR_OFF = 0,
  AEC_CORE_HDR_IN_SENSOR,
  AEC_CORE_HDR_DRC,
  AEC_CORE_HDR_STAGGERED,
  AEC_CORE_HDR_MAX,
} aec_core_hdr_type;

typedef enum {
  AEC_VHDR_SHORT_FRAME,
  AEC_VHDR_LONG_FRAME,
  AEC_VHDR_NORMAL_FRAME,
  AEC_VHDR_FRAMES_MAX,
}aec_core_vhdr_frames_t;

typedef enum {
  AEC_IR_MODE_FORCE_OFF,
  AEC_IR_MODE_FORCE_ON,
  AEC_IR_MODE_AUTO,
  AEC_IR_MODE_TOTAL,
} aec_ir_mode_t;

/** aec_hist_t
 * This structure defines the AEC histogram structure
 *
 * @roi_pixels: Number of pixels per Roi.
 * @bin: Pointer to Histogram bin.
 **/
typedef struct
{
  uint32   roi_pixels;  /**<  Number of pixels per Roi */
  uint32   *bin;        /**<  Histogram bin */
} aec_hist_t;

/** aec_algo_gyro_info_t
 * This structure defines the gyro information used by the AEC algorithm
 *
 * @float_ready: Gyro data in float ready.
 * @flt: Gyro data.
 * @q16_ready: Gyro data in q16 ready.
 * @q16: Gyro data.
 **/
typedef struct
{
  int     float_ready;
  float   flt[3];
  int     q16_ready;
  int32   q16[3];
} aec_algo_gyro_info_t;

/** aec_set_asd_param_t
 * This structure defines the Auto Scene Detection (ASD) feature inputs to the AEC algorithm
 *
 * @backlight_detected:Flag that denotes that backlight is detectect
 * @backlight_scene_severity: backlight scene severity from 1 to 255
 * @backlight_luma_target_offset: Luma target offset for backlight scene
 * @snow_or_cloudy_scene_detected: Flag that denotes that snowscene is detectect
 * @snow_or_cloudy_luma_target_offset: Luma target offset for snow  scene
 * @landscape_severity: landscape severity from 1 to 255
 * @portrait_severity: portrait severity from 1 to 255
 * @soft_focus_dgr: Amount of blurring to be applied
 * @enable: Flag to indicate whether to apply the settings or not
 **/
typedef struct
{
  uint32   backlight_detected;
  uint32   backlight_scene_severity;
  uint32   backlight_luma_target_offset;
  uint32   snow_or_cloudy_scene_detected;
  uint32   snow_or_cloudy_luma_target_offset;
  uint32   landscape_severity;
  uint32   portrait_severity;
  float    soft_focus_dgr;
  boolean  enable;
} aec_set_asd_param_t;

/** aec_aspect_ratio_t
 *  Enumerates the aec aspect ratio
 */
typedef enum {
  AR_UNDEF   = -1,
  AR_4_TO_3  =  0, /* used as index into the weight tables */
  AR_16_TO_9 =  1
} aec_aspect_ratio_t;

/**
 * Enumerates the anti-banding modes
 */
typedef enum {
  STATS_PROC_ANTIBANDING_OFF = 0,
  STATS_PROC_ANTIBANDING_60HZ,
  STATS_PROC_ANTIBANDING_50HZ,
  STATS_PROC_ANTIBANDING_AUTO,
  STATS_PROC_MAX_ANTIBANDING
} aec_antibanding_type_t;

/* Defines the modes for controlling how to address slight
 * frame rate drops when E.T. == 1/fps caused by readout delay
 */
typedef enum
{
  /* Default. This is the lagacy behavior.
   * Frame rate may drop very slightly when
   * 1/fps == ET (e.g. preview mode) */
  AEC_ANTIBANDING_FPS_DEFAULT = 0,

  /* If "ET == N / banding_period == 1/fps" then use N-1
   * banding period if N > 1. Will not work for 120fps 60Hz
   * and 100fps 50Hz */
  AEC_ANTIBANDING_FPS_STRICTLY_ENFORCE,

  /* If "ET == N / banding_period == 1/fps" then subtract
   * blanking from E.T. to account for readout time */
  AEC_ANTIBANDING_FPS_SUBTRACT_BLANKING,

  AEC_ANTIBANDING_FPS_MAX
} aec_antibanding_fps_adjust_type;

/**
 * Defines the antibanding configuration
 */
typedef struct
{
  aec_antibanding_type_t          mode;
  aec_antibanding_fps_adjust_type fps_adjust_mode;
} aec_antibanding_config_type_t;

/**
 * This structure defines the Auto Flicker Detection (AFD) inputs to the AEC algorithm
 *
 **/
typedef struct
{
  boolean                afd_enable;   /**<  Enable/Disable */
  aec_antibanding_type_t afd_atb;      /**< Antibanding table type to apply */
  aec_antibanding_fps_adjust_type afd_fps_mode;  /**< Antibanding mode adjustment for low fps */
} aec_set_afd_parm_t;

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
} aec_led_combination_type;

/**
 * Enumerates the video hdr mode parameter types
 */
typedef enum
{
  AEC_VIDEO_HDR_MODE_OFF,
  AEC_VIDEO_HDR_MODE_SENSOR,
  AEC_VIDEO_HDR_MODE_ZZHDR,
  AEC_VIDEO_HDR_MODE_STAGGERED,
  AEC_VIDEO_HDR_MODE_MAX,
} aec_video_hdr_mode_t;



/**
 * This structure defines the Automatic Dynamic Range Compression (ADRC) outputs
 * from the AEC algorithm
 *
 **/
typedef struct
{
  float gtm_ratio;
  float ltm_ratio;
  float la_ratio;
  float gamma_ratio;
  float color_drc_gain;
  float total_drc_gain;
} aec_adrc_gain_params_t;

/**
 * This structure defines the Automatic Dynamic Range
 * Compression (ADRC) for HDR outputs from the AEC algorithm
 *
 **/
typedef struct
{
  float    l_gain;      /* HDR long gain */
  uint32   l_linecount; /* HDR long line count */
  float    m_gain;      /* HDR middle gain */
  uint32   m_linecount; /* HDR middle line count */
  float    s_gain;      /* HDR short gain */
  uint32   s_linecount; /* HDR short line count */
  float    hdr_sensitivity_ratio;  /* HDR sensitivity ratio (gain*lc ratio) */
  float    hdr_exp_time_ratio;     /* HDR exposure time ratio (lc ratio) */
} aec_adrc_hdr_params_t;

/**
 * This structure defines the Auto White Balance (AWB) inputs to the AEC algorithm
 **/
typedef struct _aec_set_awb_parm {
  float r_gain;
  float g_gain;
  float b_gain;
  float unadjusted_r_gain;
  float unadjusted_g_gain;
  float unadjusted_b_gain;
  int   colortemp;
  float dual_led_flux_gain;
  boolean is_wb_mode_incandescent;
  q3a_custom_data_t awb_custom_param_update;
} aec_set_awb_parm_t;

/**
 * Enumerates the ISO modes
 */
typedef enum {
  AEC_ISO_MODE_AUTO = 0,
  AEC_ISO_MODE_DEBLUR,
  AEC_ISO_MODE_MANUAL
} aec_iso_mode_type;

/**
 * Enumerates the manual exposure time mode type
 */
typedef enum
{
    AEC_MANUAL_EXPOSURE_TIME_OFF = 0,   /**< Mask to indicate that manual exposure time mode is off */
    AEC_MANUAL_EXPOSURE_TIME_ON,        /**< Mask to indicate that manual exposure time mode is on */
} aec_manual_exp_time_mode_type;

/**
 * Enumerates the metering modes
 */
typedef enum {
  AEC_METERING_FRAME_AVERAGE,
  AEC_METERING_CENTER_WEIGHTED,
  AEC_METERING_SPOT_METERING,
  AEC_METERING_SMART_METERING,
  AEC_METERING_USER_METERING,
  AEC_METERING_SPOT_METERING_ADV,
  AEC_METERING_CENTER_WEIGHTED_ADV,
  AEC_METERING_MAX_MODES
} aec_auto_exposure_mode_t;

typedef struct {
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  int32_t led1_low_setting;   // led1 current value for pre-flash
  int32_t led2_low_setting;   // led2 current value for pre-flash
  int32_t led1_high_setting;  // led1 current value for main-flash
  int32_t led2_high_setting;  // led2 current value for main-flash
#endif
  aec_led_combination_type low_setting;
  aec_led_combination_type high_setting;
  aec_led_combination_type full_setting;
} aec_core_dual_led_settings_t;

/**
 * This structure defines the exposure parameter types
 *
 * @luma_target:        TODO
 * @current_luma:       TODO
 * @gain:               TODO
 * @linecount:          TODO
 * @led_off_gain:       TODO
 * @led_off_linecount:  TODO
 * @valid_exp_entries:  TODO
 * @use_led_estimation: TODO
 * @lux_idx:            TODO
 * @exp_time:           TODO
 * @metering_type:      TODO
 *
 **/
typedef struct _aec_exp_parms {
  int      luma_target;
  int      current_luma;
  float    real_gain[MAX_EXP_ENTRIES];
  float    sensor_gain[MAX_EXP_ENTRIES];
  uint32   linecount[MAX_EXP_ENTRIES];
  aec_adrc_hdr_params_t hdr_gains;
  aec_adrc_gain_params_t drc_gains;
  float    led_off_real_gain;
  float    led_off_sensor_gain;
  uint32   led_off_linecount;
  aec_adrc_hdr_params_t led_off_hdr_gain;
  aec_adrc_gain_params_t led_off_drc_gains;
  uint32   flash_needed;
  q3a_flash_sensitivity_type flash_sensitivity;
  int      valid_exp_entries;
  int      use_led_estimation;
  float    lux_idx;
  float    exp_time[MAX_EXP_ENTRIES];
  uint32   iso[MAX_EXP_ENTRIES];
  aec_auto_exposure_mode_t metering_type;
  q3a_custom_data_t custom_param;
  aec_core_dual_led_settings_t dual_led_setting;
  uint32   min_gain;
  uint16   min_line_count;
} aec_exp_parms_t;

/**
 * This structure defines the interface to comunicate HW supported stats and
 * the stats that aec algo will request the HW to enable.
 *
 *  @supported_stats_mask:  HW supported stats mask (input)
 *  @enable_stats_mask: Stats that AEC requires HW to enable (output)
 *  @supported_rgn_skip_mask: HW supported skip pattern mask (input)
 *  @enable_rgn_skip_pattern: Skip pattern that AEC askes HW to enable (output)
 *
 **/
typedef struct {
  uint32               supported_stats_mask;
  uint32               enable_stats_mask;
  uint32               supported_rgn_skip_mask;
  q3a_rgn_skip_pattern enable_rgn_skip_pattern;
} aec_request_stats_t;


/**
 *  This structure define the aec meta infomation.
 *
 */
typedef struct
{
  void*                 metadata;               /**< AWB metadata buffer pointer. */
  uint32                metadata_size;          /**< AWB metadata size. */
} aec_meta_info_type;

/**
 * Enumerates the 'get' parameter types
 */
#define AEC_CORE_GET_ENUM_LIST(ADD_ENTRY)                                    \
  ADD_ENTRY(AEC_GET_EXPOSURE_PARAMS)            /* Mask to get aec info.  */ \
  ADD_ENTRY(AEC_GET_META_INFO)                  /* Mask to get meta info. */ \
  ADD_ENTRY(AEC_GET_REAL_GAIN_FROM_ISO)                                      \
  ADD_ENTRY(AEC_GET_STATS_REQUIRED)    /* Mask to get stats type required */ \
  ADD_ENTRY(AEC_GET_UNIFIED_FLASH)                                           \
  ADD_ENTRY(AEC_GET_RGN_SKIP_PATTERN)                                        \
  ADD_ENTRY(AEC_GET_RELOAD_EXPOSURE_PARAMS)                                  \
  ADD_ENTRY(AEC_GET_LED_CAL_CONFIG)                                          \
  ADD_ENTRY(AEC_GET_MAX)                                                     \

typedef enum
{
  AEC_CORE_GET_ENUM_LIST(AEC_CORE_GENERATE_ENUM)
} aec_core_get_enum_type;

/**
 *  This structure define the infomation for aec core get.
 */
typedef struct
{
  aec_core_get_enum_type   param_type;  /**<  Get parameter type  */
  union
  {
    uint32            metadata_size;           /**< Metadata size. */
    uint32            iso_value;               /**< ISO value. */
    uint32            supported_stats_mask;    /**< HW supported stats mask */
    uint32            supported_rgn_skip_mask; /**< HW supported region skip mask */
  }u;
} aec_core_get_info_type;

typedef enum {
    FLASH_MODE_OFF,
    FLASH_MODE_AUTO,
    FLASH_MODE_ON,
    FLASH_MODE_TORCH,
    FLASH_MODE_SINGLE,
    FLASH_MODE_MAX
} flash_mode_t;


typedef struct {
  q3a_capture_type_t capture_type;
  flash_mode_t flash_hal;
  boolean  flash_mode;
  int32    hdr_exp;
  float    real_gain;
  float    sensor_gain;
  uint32   line_count;
  float    lux_idx;
  uint32   gamma_flag;    /* Set for gamma related features */
  uint32   nr_flag;       /* Set for noise reduction features */
  float    exp_time;
  aec_adrc_gain_params_t drc_gains;
  aec_adrc_hdr_params_t  hdr_gains;
  uint32_t iso;
  aec_core_dual_led_settings_t dual_led_setting;
} aec_capture_frame_info_t;

/** _aec_frame_batch:
 *    @num_batch:
 *
 *  description
 **/
typedef struct _aec_frame_batch_core {
  int16  num_batch;
  int    luma_target;
  int    current_luma;
  float  led_off_real_gain;
  float  led_off_sensor_gain;
  uint32 led_off_linecount;
  aec_adrc_gain_params_t led_off_drc_gains;
  aec_adrc_hdr_params_t led_off_hdr_gains;
  int    valid_exp_entries;
  int    use_led_estimation;
  aec_auto_exposure_mode_t metering_type;
  aec_capture_frame_info_t *frame_batch;
  q3a_flash_sensitivity_type flash_si;
} aec_frame_batch_core_t;

/** _aec_proc_snapshot:
 *    @luma_target:             TODO
 *    @line_count:              TODO
 *    @redeye_led_on_skip_frm:  TODO
 *    @redeye_led_off_skip_frm: TODO
 *    @auto_mode:               TODO
 *    @exp_mode:                TODO
 *    @exp_program:             TODO
 *    @real_gain:               TODO
 *    @exp_time:                TODO
 *
 * TODO description
 **/
typedef struct _aec_proc_snapshot {
  boolean  is_flash_snapshot;
  uint32 luma_target;
  uint32 line_count;
  uint32 redeye_led_on_skip_frm;
  uint32 redeye_led_off_skip_frm;
  boolean  auto_mode;
  uint32 exp_mode;
  uint32 exp_program;
  float    real_gain;
  float    sensor_gain;
  aec_adrc_gain_params_t drc_gains;
  aec_adrc_hdr_params_t hdr_gains;
  uint32 exif_iso;
  float    exp_time;
  float  lux_index;
} aec_proc_snapshot_t;

/** aec_precapture_mode_t:
 *
 * The HAL3 AEC precapture trigger type
 **/
typedef enum {
  AEC_PRECAPTURE_TRIGGER_IDLE,
  AEC_PRECAPTURE_TRIGGER_START,
  AEC_PRECAPTURE_TRIGGER_CANCEL
} aec_precapture_mode_t;

/** _aec_sensor_info:
 *    @current_fps:           TODO
 *    @preview_fps:           TODO
 *    @snapshot_fps:          TODO
 *    @video_fps:             hardcode it to 30
 *    @max_preview_fps:       TODO
 *    @preview_linesPerFrame: TODO
 *    @snap_linesPerFrame:    TODO
 *    @snap_max_line_cnt:     TODO
 *    @max_gain:              TODO
 *    @pixel_clock:           TODO
 *    @pixel_clock_per_line:  TODO
 *    @af_is_supported:       TODO
 *    @sensor_res_width:      TODO
 *    @sensor_res_height:     TODO
 *    @pixel_sum_factor:      the binning factor
 *    @sensor_type: Sensor type. Usually default is Bayer
 *    @lens_type: Lens type. Usually default wide sensor
 *
 * TODO description
 **/
typedef struct _aec_sensor_info {
  uint32 current_fps;
  uint32 preview_fps;
  uint32 snapshot_fps;
  uint32 video_fps;
  uint32 max_preview_fps;
  uint32 preview_linesPerFrame;
  uint32 snap_linesPerFrame;
  uint32 snap_max_line_cnt;
  uint32 prev_max_line_cnt;
  float    max_gain;
  uint32 pixel_clock;
  uint32 pixel_clock_per_line;
  uint32 af_is_supported;
  uint32 sensor_res_width;
  uint32 sensor_res_height;
  boolean  ois_supported;
  uint16 pixel_sum_factor;
  uint16 pixel_sum_factor_snap;
  float f_number;
  boolean svhdr_use_separate_gain;
  boolean svhdr_use_separate_limits;
  uint32_t min_line_cnt[AEC_VHDR_FRAMES_MAX];
  uint32_t max_line_cnt[AEC_VHDR_FRAMES_MAX];
  q3a_sensor_type_t  sensor_type;
  q3a_lens_type_t    lens_type;
} aec_sensor_info_t;

/**
 *
 * This structure defines the FD ROI inputs to AEC algorithm
 **/
typedef struct
{
  q3a_core_roi_dimension_type   roi[Q3A_CORE_MAX_ROI_COUNT];  /**< Array of Fd roi */
  uint8                         roi_count;              /**< Number of Fd roi entries */
  aec_hist_t                    hist[Q3A_CORE_MAX_ROI_COUNT];/**< Array of Fd histogram */
  uint32                        frm_width;              /**< Coordinate reference width */
  uint32                        frm_height;             /**< Coordinate reference height */
} aec_proc_fd_roi_info_type;

/**
 * This structure defines the parameters for manual eztune linecount
 */
typedef struct {
  uint32 forced;
  uint32 force_linecount_value;
} aec_ez_force_linecount_t;

/**
 * This structure defines the parameters for manual eztune gain
 */
typedef struct {
  uint32 forced;
  float force_gain_value;
} aec_ez_force_gain_t;

/**
 * This structure defines the parameters for manual eztune exposure time
 */
typedef struct {
  uint32 forced;
  float force_exp_value;
} aec_ez_force_exp_t;

/**
 * This structure defines the parameters for manual eztune snapshot linecount
 */
typedef struct {
  uint32 forced;
  uint32 force_snap_linecount_value;
} aec_ez_force_snap_linecount_t;

/**
 * This structure defines the parameters for manual eztune snapshot gain
 */
typedef struct {
  uint32 forced;
  float force_snap_gain_value;
} aec_ez_force_snap_gain_t;

/**
 * This structure defines the parameters for manual eztune snapshot exposure time
 */
typedef struct {
  uint32 forced;
  float force_snap_exp_value;
} aec_ez_force_snap_exp_t;


/**
 * This structure defines the parameters for the AEC ramp test
 *
 **/
typedef struct
{
  boolean   enable;        /**< Flag to enable\disable the ramp test */
  uint8     frame_skip;    /**< Number of frames to skip between each exposure update */
  uint8     step_size;     /**< Exposure adjustment step size between each update */
  uint32    lower_limit;   /**< Lower limit in the exposure table for the ramp test */
  uint32    upper_limit;   /**< Upper limit in the exposure table for the ramp test */
} aec_algo_ramp_test_type;

/** _aec_ui_frame_dim:
 *    @width:  TODO
 *    @height: TODO
 *
 * dimension of the ui (i.e preview) frame
 **/
typedef struct _aec_ui_frame_dim {
  uint32       width;
  uint32       height;
} aec_ui_frame_dim_t;

/** _aec_stored_parm_t:
 *    @enable: True indicates, start AEC using previous session exp index
 *    @session_id: Session ID
 *    @first_init: First camera init
 *    @exp_index: Previous exposure index
 *
 * Stores the current exposure index and which will be used in next camera
 * session as a starting point to improve the convergence.
 **/
typedef struct _aec_stored_params_t {
  boolean   enable;
  uint32    session_id;
  boolean   first_init;
  uint32    exp_index;
  float     sensitivity;
}aec_stored_params_t;

/** aec_role_switch_params_t:
 *    @master: If AEC is master/slave
 *    @exp_index: Interpolated exposure index
 *
 * Stores the current exposure index and which will be used in next camera
 * session as a starting point to improve the convergence.
 **/
typedef struct _aec_role_switch_params_t {
  boolean   master;
  int       exp_index;
  float     lux_idx;
  float     exp_time;
  float     real_gain;
  uint32    linecount;
  float     total_drc_gain;
} aec_role_switch_params_t;

/** _aec_iot_config:
 *    @reserved: Currently this interface is reserved
 *
 **/
typedef struct _aec_iot_config_t {
  void *reserved;
}aec_iot_config_t;

/** _aec_warm_start_t:
 *    @stored_params: Previous session parameter
 *    @start_range_perc: Percentage value used to compute the range for start exp index.
 *      If set to 1, min and max cap will be 0 and max exp index. For < 1, middle range of exp table
 *      will be used for capping the start exp index.
 *
 * Stores previous session parameters and warm start(store restore) tuning parameter.
 * Warm starts refers to starting the AEC using previous session exposure settings.
 **/
typedef struct _aec_warm_start_t {
  aec_stored_params_t *stored_params;
  float start_range_perc;
}aec_warm_start_t;

typedef enum {
  AEC_LED_CAL_INACTIVE,
  AEC_LED_CAL_RUNNING,
  AEC_LED_CAL_SUCCESS,
  AEC_LED_CAL_FAILED,
  AEC_LED_CAL_MAX
} aec_led_cal_state_t;

/** _aec_led_cal
 *
 **/
typedef struct _aec_led_cal_t {
  uint32_t led_cal_num;
  uint32_t led_toggle_num;
  uint32_t led_on_interval;
  uint32_t led_off_interval;
} aec_led_cal_config_t;

typedef struct _aec_led_cal_output_t {
  aec_led_cal_state_t state;
  int cal_num;
  float rg_ratio[MAX_CALIB_NUM];
  float bg_ratio[MAX_CALIB_NUM];
} aec_led_cal_data_t;

/** _aec_set_parameter_init:
 *    @stats_type:     TODO
 *    @chromatix:      TODO
 *    @comm_chromatix: TODO
 *    @sensor_info:    TODO
 *    @numRegions:     information from VFE/ISP
 *    @op_mode:        op_mode can be derived from stream info
 *    @frame_dim:      TODO
 *    @enable_flash_for_aec_lock: Enable/Disable Flash when is locked
 *                                from flash is force ON
 *    @stored_param:  Previous session exposure params
 *    @iot_config:    Custom configuration for IOT features
 * TODO description
 **/
typedef struct _aec_set_parameter_init {
  void                     *chromatix;
  aec_sensor_info_t        sensor_info;
  unsigned int             numRegions;
  aec_operation_mode_t     op_mode;
  aec_ui_frame_dim_t       frame_dim;
  boolean                  longshot_flash_aec_lock;
  boolean                  enable_flash_for_aec_lock;
  aec_warm_start_t         warm_start;
  aec_iot_config_t         iot_config;
  aec_led_cal_data_t       *led_cal_param;
} aec_set_parameter_init_t;

/** aec_precapture_trigger_t:
 *    @trigger:    The HAL3 trigger
 *    @trigger_id: The ID of the trigger
 *
 * TODO description
 **/
typedef struct {
  uint8 trigger;
  int32 trigger_id;
} aec_precapture_trigger_t;

/** _aec_fps_range:
 *    @min_fps: min fpa for aec
 *    @max_fps: max fps for aec
 *
 * TODO description
 **/
typedef struct _aec_fps_range {
  int min_fps;
  int max_fps;
} aec_fps_range_t;

/** aec_bracket_t
 *  This structure is used to store exposure bracketing values
 *  sent from HAL
 **/
typedef struct
{
  int8   ev_val[10];
  uint8  valid_entries;
} aec_bracket_t;

typedef struct
{
  int8 enabled;
  int  prefered_exp_index;
} aec_dual_led_calib_data_t;

typedef struct
{
    int32_t test_config;
    int    test_config_freq;
} aec_ae_scan_test_config_data_t;

/* Display Flash Definition */
typedef enum aec_lcd_flash_state {
  AEC_LCD_FLASH_OFF,
  AEC_LCD_FLASH_START,
  AEC_LCD_FLASH_MAX_POW,
  AEC_LCD_FLASH_IDLE
} aec_lcd_flash_state_t;

/**
 * Enumerates the input info type
 */
#define AEC_INPUT_PARAM_ENUM_LIST(ADD_ENTRY)                                 \
  /* Mask to set exposure compensation mode */                               \
  ADD_ENTRY(AEC_INPUT_PARAM_EXP_COMPENSATION)                                \
  ADD_ENTRY(AEC_INPUT_PARAM_UI_FRAME_DIM)                                    \
  /* Mask to set brightness level */                                         \
  ADD_ENTRY(AEC_INPUT_PARAM_BRIGHTNESS_LVL)                                  \
  ADD_ENTRY(AEC_INPUT_PARAM_HJR_AF)                                          \
  /* Mask to set metering mode */                                            \
  ADD_ENTRY(AEC_INPUT_PARAM_METERING_MODE)                                   \
  /* Mask to set iso mode*/                                                  \
  ADD_ENTRY(AEC_INPUT_PARAM_ISO_MODE)                                        \
  /* Mask to set the ISO value */                                            \
  ADD_ENTRY(AEC_INPUT_PARAM_ISO_VALUE)                                       \
  /* Mask to set the manual exposure time mode */                            \
  ADD_ENTRY(AEC_INPUT_PARAM_MANUAL_EXP_TIME_MODE)                            \
  /* Mast to set the manual exposure time */                                 \
  ADD_ENTRY(AEC_INPUT_PARAM_MANUAL_EXP_TIME)                                 \
  /* Mask to set Antibanding mode*/                                          \
  ADD_ENTRY(AEC_INPUT_PARAM_ANTIBANDING)                                     \
  /* Mask to set ROI */                                                      \
  ADD_ENTRY(AEC_INPUT_PARAM_ROI)                                             \
  /* Mask to set Metering Area*/                                             \
  ADD_ENTRY(AEC_INPUT_PARAM_MTR_AREA)                                        \
  /* Mask to set Face detection ROI */                                       \
  ADD_ENTRY(AEC_INPUT_PARAM_FD_ROI)                                          \
  /* Mask to set asd parameters */                                           \
  ADD_ENTRY(AEC_INPUT_PARAM_ASD_PARM)                                        \
  /* Mask to set afd parameter */                                            \
  ADD_ENTRY(AEC_INPUT_PARAM_AFD_PARM)                                        \
  ADD_ENTRY(AEC_INPUT_PARAM_AWB_PARM)                                        \
  ADD_ENTRY(AEC_INPUT_PARAM_GYRO_INFO)                                       \
  ADD_ENTRY(AEC_INPUT_PARAM_LED_MODE)                                        \
  ADD_ENTRY(AEC_INPUT_PARAM_ZSL_OP)                                          \
  ADD_ENTRY(AEC_INPUT_MANUAL_SKIP)                                           \
  ADD_ENTRY(AEC_INPUT_PARAM_LCD_FLASH_STATE)                                 \
  ADD_ENTRY(AEC_INPUT_PARAM_MAX)                                             \

typedef enum
{
  AEC_INPUT_PARAM_ENUM_LIST(AEC_CORE_GENERATE_ENUM)
} aec_input_enum_type;

/**
 * This structure define the input info entry data structure
 */
typedef union
{
  int32                            exp_comp;
  aec_set_parameter_init_t         init_param;
  int                              brightness;
  uint32                           hjr_af;
  aec_auto_exposure_mode_t         aec_metering;
  aec_iso_mode_type                iso_mode;
  uint32                           iso_value;
  aec_antibanding_config_type_t    antibanding;
  aec_manual_exp_time_mode_type    manual_exp_time_mode;
  float                            manual_exp_time_in_sec;
  q3a_core_roi_configuration_type  aec_roi;
  q3a_core_roi_configuration_type  mtr_area;
  aec_proc_fd_roi_info_type        fd_roi;
  aec_set_asd_param_t              asd_param;
  aec_set_afd_parm_t               afd_param;
  aec_set_awb_parm_t               awb_param;
  aec_algo_gyro_info_t             gyro_info;
  q3a_led_flash_mode_t             led_mode;
  int32                            zsl_op;
  int64                            manual_expTime;
  int32                            manual_gain;
  boolean                          longshot_mode;
  uint32                           manual_to_auto_skip_cnt;
  aec_lcd_flash_state_t            lcd_flash_state;
} aec_input_payload_type;

/**
 * This structure define the input info entry.
 */
typedef struct
{
  uint8                   next_flagged_entry;
  aec_input_payload_type  data;
} aec_input_entry_type;

/**
 * This structure define the input info data structure
 */
typedef struct
{
  uint8                first_flagged_entry;
  aec_input_entry_type entry[AEC_INPUT_PARAM_MAX];
} aec_core_input_info_type;

/**
 * Enumerates the 'set' parameter types
 */
#define AEC_SET_ENUM_LIST(ADD_ENTRY)                                          \
  /* Mask to set chromatix information */                                     \
  ADD_ENTRY(AEC_SET_COPY_CHROMATIX_SENSOR)                                    \
  ADD_ENTRY(AEC_SET_INIT_CHROMATIX_SENSOR)                                    \
  /* Mask to reset led gains*/                                                \
  ADD_ENTRY(AEC_SET_LED_RESET)                                                \
  /* Mask to trigger led estimation*/                                         \
  ADD_ENTRY(AEC_SET_PREPARE_FOR_SNAPSHOT)                                     \
  /* Mask to force trigger led estimation*/                                   \
  ADD_ENTRY(AEC_SET_FORCE_LED_ESTIMATION)                                     \
  ADD_ENTRY(AEC_SET_EZ_DISABLE)                                               \
  ADD_ENTRY(AEC_SET_EZ_LOCK_OUTPUT)                                           \
  ADD_ENTRY(AEC_SET_EZ_FORCE_EXP)                                             \
  ADD_ENTRY(AEC_SET_EZ_FORCE_LINECOUNT)                                       \
  ADD_ENTRY(AEC_SET_EZ_FORCE_GAIN)                                            \
  ADD_ENTRY(AEC_SET_EZ_TEST_ENABLE)                                           \
  ADD_ENTRY(AEC_SET_EZ_TEST_ROI)                                              \
  ADD_ENTRY(AEC_SET_EZ_TEST_MOTION)                                           \
  ADD_ENTRY(AEC_SET_EZ_FORCE_SNAP_EXP)                                        \
  ADD_ENTRY(AEC_SET_EZ_FORCE_SNAP_LINECOUNT)                                  \
  ADD_ENTRY(AEC_SET_EZ_FORCE_SNAP_GAIN)                                       \
  ADD_ENTRY(AEC_SET_EZ_FORCE_DUAL_LED_IDX)                                    \
  ADD_ENTRY(AEC_SET_EZ_TUNE_RUNNING)                                          \
  ADD_ENTRY(AEC_SET_LOCK)                                                     \
  /* Mask to trigger Precapture sequence */                                   \
  ADD_ENTRY(AEC_SET_LED_EST)                                                  \
  /* Mask to set sensor configuration */                                      \
  ADD_ENTRY(AEC_SET_INIT_SENSOR_INFO)                                         \
  ADD_ENTRY(AEC_SET_ENABLE)                                                   \
  ADD_ENTRY(AEC_SET_VIDEO_HDR)                                                \
  ADD_ENTRY(AEC_SET_SNAPSHOT_HDR)                                             \
  ADD_ENTRY(AEC_SET_CAPTURE_MODE)                                             \
  ADD_ENTRY(AEC_SET_DO_LED_EST_FOR_AF)                                        \
  ADD_ENTRY(AEC_SET_PREPARE_FOR_LED_AF)                                       \
  ADD_ENTRY(AEC_SET_PREP_FOR_SNAPSHOT_NOTIFY)                                 \
  ADD_ENTRY(AEC_SET_PREP_FOR_SNAPSHOT_LEGACY)                                 \
  ADD_ENTRY(AEC_SET_RESET_LED_EST)                                            \
  ADD_ENTRY(AEC_SET_BRACKET)                                                  \
  ADD_ENTRY(AEC_SET_ENABLE_DEBUG_DATA)                                        \
  ADD_ENTRY(AEC_SET_CLAMP_SNAP_LC_FOR_MIN_FPS)                                \
  ADD_ENTRY(AEC_SET_DEBUG_DATA_LEVEL)                                         \
  ADD_ENTRY(AEC_SET_FPS_RANGE)                                                \
  ADD_ENTRY(AEC_SET_ADRC_ENABLE)                                              \
  ADD_ENTRY(AEC_SET_INIT_EXPOSURE_INDEX)                                      \
  ADD_ENTRY(AEC_SET_LONGSHOT_ENABLE)                                          \
  ADD_ENTRY(AEC_SET_INSTANT_AEC_TYPE)                                         \
  ADD_ENTRY(AEC_SET_DUAL_LED_CALIB_MODE)                                      \
  ADD_ENTRY(AEC_SET_MAX_FLASH_CURRENT)                                        \
  ADD_ENTRY(AEC_SET_CONFIG_AE_SCAN_TEST)                                      \
  ADD_ENTRY(AEC_SET_LIGHT_SENSOR_INFO)                                        \
  ADD_ENTRY(AEC_SET_IR_MODE)                                                  \
  ADD_ENTRY(AEC_SET_CONV_SPEED)                                               \
  ADD_ENTRY(AEC_SET_ROLE_SWITCH)                                              \
  ADD_ENTRY(AEC_SET_LED_CAL)                                                  \
  ADD_ENTRY(AEC_SET_SENSOR_FLASH_MODE)                                        \
  /* Result of LED face detection */                                          \
  ADD_ENTRY(AEC_SET_LED_FD_COMPLETED)                                         \
  ADD_ENTRY(AEC_SET_HDR_RATIO)                                                \
  ADD_ENTRY(AEC_SET_EXP_COMPENSATION)                                         \
  ADD_ENTRY(AEC_SET_MAX)                                                      \

typedef enum
{
  AEC_SET_ENUM_LIST(AEC_CORE_GENERATE_ENUM)
} aec_set_enum_type;

/**
 * Defines the debug data levels supported by the AEC algorithm.
 */
typedef enum
{
  AEC_DEBUG_DATA_LEVEL_NONE = 0,              /**< Disables debug data. */
  AEC_DEBUG_DATA_LEVEL_CONCISE,               /**< Enables concise level of debug data. */
  AEC_DEBUG_DATA_LEVEL_VERBOSE,               /**< Enables verbose level of debug data. */
  AEC_DEBUG_DATA_LEVEL_INVALID = 0xFF         /**< Invalid/Max level. */
} aec_debug_data_level_type;

/**
 * Defines the config data for AE Scan test
 */
typedef struct {
  int test_config;
  int test_config_freq;
} aec_scan_test_config_t;

/**
 * This structure defines the parameters that are passed in as part of the 'set' call
 **/

typedef struct
{
  aec_set_enum_type type; /**<  */
  union
  {
    aec_set_parameter_init_t      init_param;
    boolean                       ez_disable;
    boolean                       ez_lock_output;
    aec_ez_force_exp_t            ez_force_exp;
    aec_ez_force_linecount_t      ez_force_linecount;
    aec_ez_force_gain_t           ez_force_gain;
    boolean                       ez_test_enable;
    boolean                       ez_test_roi;
    boolean                       ez_test_motion;
    aec_ez_force_snap_exp_t       ez_force_snap_exp;
    aec_ez_force_snap_linecount_t ez_force_snap_linecount;
    aec_ez_force_snap_gain_t      ez_force_snap_gain;
    uint8_t                       ez_force_dual_led_idx;
    boolean                       ez_running;
    boolean                       aec_lock;
    boolean                       aec_enable;
    aec_bracket_t                 aec_bracket;
    int32                         video_hdr;  /**<  */
    aec_core_hdr_type             snapshot_hdr;
    aec_precapture_trigger_t      aec_trigger;
    boolean                       est_for_af;
    aec_algo_ramp_test_type       ramp_test;
    boolean                       debug_data_enable;
    boolean                       clamp_snap_lc_for_min_fps;
    aec_debug_data_level_type     debug_data_level;
    aec_fps_range_t               fps_range;
    boolean                       adrc_enable;
    uint32                        exp_index;
    boolean                       longshot_enable;
    aec_convergence_type          instant_aec_type;
    aec_dual_led_calib_data_t     dual_led_calib_data;
    uint32                        max_flash_current;
    aec_scan_test_config_t        ae_scan_test_config;
    float                         light_sensor_info;
    aec_ir_mode_t                 ir_mode_type;
    float                         conv_speed;
    boolean                       led_cal_enable;
    flash_mode_t                  sensor_flash_mode;
    aec_proc_fd_roi_info_type     fd_roi;
    aec_role_switch_params_t      role_switch_params;
    float                         hdr_ratio;
    int32                         exp_comp;
  } u;
} aec_core_set_param_type;

/** _aec_ez_tune:
 *    @test_enable:          TODO
 *    @test_roi:             TODO
 *    @test_motion:          TODO
 *    @force_snapshot_exp:   TODO
 *    @disable:              TODO
 *    @lock_output:          TODO
 *    @force_exp:            TODO
 *    @force_linecount:      TODO
 *    @force_gain:           TODO
 *    @force_snap_linecount: TODO
 *    @force_snap_gain:      TODO
 *    @stored_gain:          TODO
 *    @stored_line_count:    TODO
 *    @touch_roi_luma:       TODO
 *    @preview_exp_time:     TODO
 *
 * TODO description
 **/
typedef struct __attribute__((__packed__)) {
  boolean                        test_enable;
  boolean                        test_roi;
  boolean                        test_motion;
  int                            disable;
  boolean                        lock_output; /* ??? */
  aec_ez_force_exp_t             force_exp;
  aec_ez_force_linecount_t       force_linecount;
  aec_ez_force_gain_t            force_gain;
  aec_ez_force_snap_linecount_t  force_snap_linecount;
  aec_ez_force_snap_gain_t       force_snap_gain;
  aec_ez_force_snap_exp_t        force_snapshot_exp;
  int                            ez_force_dual_led_idx;

  uint32                         touch_roi_luma; /* ??? */
  boolean                        running;
  boolean                        enable;
  int32                          luma;
  int32                          lock;
  int32                          exposure_index;
  int32                          lux_index;
  int32                          preview_linecount;
  float                          preview_realgain;
  float                          preview_exp_time;
  int32                          snap_linecount;
  float                          snap_realgain;
  float                          snap_exp_time;
  int32                          antibanding_enable;

  uint32                         hybrid_luma;
  uint32                         average_luma;
  uint32                         bright_regions;

  float                          red_severity;
  float                          green_severity;
  float                          blue_severity;
  float                          bimodal_severity;

  uint32                         compensated_target;
  uint32                         default_target;

  uint16                         current_bright_level;
  float                          current_bright_weight;

  int32                          entropy_luma_offset;

  int32                          flash_off_exp_index;
  int32                          flash_off_luma;
  int32                          flash_lo_exp_index;
  int32                          flash_lo_luma;
  float                          flash_sensitivity_off;
  float                          flash_sensitivity_lo;
  float                          flash_sensitivity_high;

  float                          flat_severity;
  float                          flat_near_severity;
  int32                          flat_bright_detected;
  int32                          flat_dark_detected;
  int32                          flat_luma_ratio;

  uint32                         ssd_luma_target_offset;
  uint32                         bsd_luma_target_offset;

  uint32                         touch_luma;
  uint32                         pixel_clk;
  uint32                         pixel_clk_per_line;
} aec_ez_tune_t;

/* aec_ir_config_data_t: Defines AEC core output parameters for IR Camera Mode
 *  @ir_needed:               Determines IR LED is nedeed or not
 *  @ir_led_brightness:       IR LED brightness in percentage
 */
typedef struct {
   boolean ir_needed;
   float ir_led_brightness;
} aec_ir_config_data_t;

/** aec_output_type
 *
 *
 **/
typedef struct
{
  int32                           exp_index;
  float                           lux_idx;
  uint32                          pixelsPerRegion;
  uint32                          numRegions;
  int                             preview_fps;
  boolean                         afr_enable;
  aec_auto_exposure_mode_t        metering_type;
  float                           exp_time;
  uint32                          settled;
  float                           target_luma;
  float                           cur_luma;
  float                           real_gain;
  float                           sensor_gain;
  aec_adrc_gain_params_t          drc_gains;
  float                           bg_thres_divider;
  uint32                          linecount;
  float                           s_real_gain;
  uint32                          s_linecount;
  float                           m_real_gain;
  uint32                          m_linecount;
  float                           l_real_gain;
  uint32                          l_linecount;
  float                           hdr_sensitivity_ratio;
  float                           hdr_exp_time_ratio;
  float                           prev_sensitivity;
  uint32                          exp_tbl_val;
  int                             max_line_cnt;
  float                           comp_luma;
  uint32                          avg_luma;
  q3a_flash_sensitivity_type      flash_sensitivity;
  float                           band_50hz_gap;
  int                             cur_atb;
  boolean                         force_prep_snap_done;
  uint32                          asd_extreme_green_cnt;
  uint32                          asd_extreme_blue_cnt;
  uint32                          asd_extreme_tot_regions;
  uint32                          hjr_dig_gain;
  aec_proc_snapshot_t             snap;
  aec_ez_tune_t                   eztune;
  int16                           iso_Exif;
  float                           Bv;
  float                           Av;
  float                           Sv;
  float                           Tv;
  float                           Ev;

  int                             need_config;
  boolean                         locked_from_algo;
  int32                           trigger_id;
  uint32                          luma_delta;
  uint32                          luma_settled_cnt;
  uint32                          sof_id;
  uint32                          frame_id;
  uint32                          SY[256];
  int                             preview_linesPerFrame;
  float                           led_off_lux_index;
  float                           led_off_real_gain;
  float                           led_off_sensor_gain;
  uint32                          led_off_linecnt;
  float                           led_off_s_gain;
  uint32                          led_off_s_linecnt;
  float                           led_off_l_gain;
  uint32                          led_off_l_linecnt;
  boolean                         is_hdr_drc_enabled;
  boolean                         prep_snap_no_led;
  int                             led_state;
  int                             use_led_estimation;
  int32                           led_exp_index_adjustment;
  boolean                         led_af_needed;
  q3a_core_roi_configuration_type roi;
  boolean                         hdr_indoor_detected;
  uint32                          flash_needed;
  int                             led_fd_use_delay;
  aec_ir_config_data_t            ir_config;

  uint8                           video_hdr_min_stats_average;
  uint8                           video_hdr_max_stats_average;

  int                             op_mode;
  boolean                         aec_maxed_out; /* [A] - Flag that indicates that AEC is maxed out */
  uint32                          convergence_percentage; /* [A] - AEC convergence expressed in terms of a percentage */

  int                             prev_exp_index;
  aec_iso_mode_type               iso_mode;
  uint32                          iso_value;
  boolean                         led_needed;
  int                             hjr_snap_frame_cnt;
  uint32                          cur_af_luma;
  aec_touch_ev_status_type        touch_ev_status;
  int                             low_light_shutter_flag;
  boolean                         aec_locked;
  aec_core_dual_led_settings_t    dual_led_setting;
  aec_led_cal_data_t              led_cal_output;
  float                           Dual_LED1_influence_ratio;

  uint32_t                        number_of_hdr_frames;
  float                           hdr_real_gain[AEC_VHDR_FRAMES_MAX];
  uint32_t                        hdr_linecount[AEC_VHDR_FRAMES_MAX];
  float                           hdr_gtm_gamma;
  float                           hdr_exposure_ratio;
  float                           conv_speed;
  uint8                           roi_count;
  float                           awb_roi_x[Q3A_CORE_MAX_ROI_COUNT];
  float                           awb_roi_y[Q3A_CORE_MAX_ROI_COUNT];
  float                           awb_roi_dx[Q3A_CORE_MAX_ROI_COUNT];
  float                           awb_roi_dy[Q3A_CORE_MAX_ROI_COUNT];
  float                           awb_roi_weight[Q3A_CORE_MAX_ROI_COUNT];
  float                           frm_width;
  float                           frm_height;
  int                             faceawb;
  uint32_t                        lcd_flash_settled;
} aec_core_output_type;

typedef struct
{
  union
  {
    aec_frame_batch_core_t  frame_info;           /**< Query Unfied Flash info **/
    aec_exp_parms_t      exp_params;              /**< Query exposure parameters */
    aec_meta_info_type   meta_info;               /**< Query meta info. */
    float                real_gain;               /**< Query real gain from iso value */
    uint32               enable_stats_mask;       /**< Query stats that AEC requires HW to enable */
    q3a_rgn_skip_pattern enable_rgn_skip_pattern; /**< Query skip pattern that AEC requires HW to enable */
    aec_core_output_type *core_output;            /**< Query core_output requires to reload aec output */
    aec_led_cal_config_t led_cal_config;          /**< Query led cal info */
  } u;
} aec_core_get_output_type;

/**
 * This structure defines the inputs to the AEC algorithm
 *
 **/
typedef struct
{
  aec_core_input_info_type    input_info;
  aec_stats_input_type        stats;
  boolean                     aec_debug;
} aec_core_input_type;

/* ==========================================================================
                       Function Declarations
========================================================================== */
/**
 * Initializes the AEC core module.
 *
 * @param[out]  Null
 *
 * @return Returns handle Pointer to the handle to the AEC core module.
 */
q3a_core_result_type aec_init
(
  aec_handle_type *p_handle
);

/**
 * DeInitializes the AEC core module.
 *
 * @param[in]  Pointer to the handle to the AEC core module.
 *
 * @return Returns 3A_RESULT_SUCCESS upon success, any other
 *  error upon failure.
 */
q3a_core_result_type aec_deinit(aec_handle_type p_handle);

/**
 * Configures the core AEC algorithm as a set parameter
 *
 * @param[in] p_handle    Handle to the AEC core module.
 * @param[in] param     Parameters that will be used for
 *                      re-configuring AEC
 *
 * @return Returns 3A_RESULT_SUCCESS upon success, any other error upon failure.
 */
q3a_core_result_type aec_set_param
(
  aec_handle_type p_handle,
  aec_core_set_param_type *param
);
/**
 * Runs the core AEC algorithm.
 *
 * @param[in] p_handle  Handle to the AEC core module.
 * @param[in] input   Parameters input into the AEC core module.
 * @param[out] output Parameters output by the AEC core module.
 *
 * @return Returns 3A_RESULT_SUCCESS upon success, any other error upon failure.
 */
q3a_core_result_type aec_process
(
  aec_handle_type p_handle,
  const aec_core_input_type *input,
  aec_core_output_type *output
);
/**
 * Query the core AEC algorithm as a get parameter
 *
 * @param[in] p_handle    Handle to the AEC core module.
 * @param[in] param     Parameters that will be used for querying  AEC algorithm
 *
 * @return Returns 3A_RESULT_SUCCESS upon success, any other error upon failure.
 */
q3a_core_result_type aec_get_param
(
  aec_handle_type p_handle,
  const aec_core_get_info_type *get_param_info,
  aec_core_get_output_type* get_param_output
);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* __AEC_CORE_H__ */
