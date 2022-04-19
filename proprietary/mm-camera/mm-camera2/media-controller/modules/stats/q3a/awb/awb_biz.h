/* awb_biz.h
 *
 * Copyright (c) 2014-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __BIZ_AWB_H__
#define __BIZ_AWB_H__

#include "awb.h"
#include "awb_core.h"
#include <dlfcn.h>

#define AWB_MANUAL_TO_AUTO_SKIP_CNT     3
#define AWB_LED_TO_OFF_SKIP_CNT         4
#define MAX_AWB_ALGO_OUTPUTS            1

/**
 * Defines whether AWB should lock/unlock in longshot flash snapshot.
 */
#define LONGSHOT_FLASH_AWB_LOCK  (TRUE)

/* File path where Dual LED calibration data is saved to or loaded from.
 * OEMs should redefine this */
#define DUAL_LED_CALIB_FILE_PATH QCAMERA_DUMP_FRM_LOCATION"dual_led_calibration"

/* Property name to check to see if dual LED calibration feature is enabled.
 * OEMs could redefine this */
#define DUAL_LED_CALIB_FEATURE_ENABLED_PROP "persist.vendor.camera.ledcalib.enabled"

/**
 * Defines the future tuning parameter, pending move to chormatix.
 */
#define default_white_current_decision_weight     (1)
#define default_a_h_line_boundary                 (0.5)
#define default_day_f_line_boundary               (0.5)
#define default_d75_d65_line_boundary             (0.5)
#define default_d65_d50_line_boundary             (0.5)
#define default_d65_d50_shifted_line_boundary     (0.5)
#define default_f_a_line_boundary                 (0.5)
#define default_cw_tl84_line_boundary             (0.5)
#define default_d2cw_enable                       (0)
#define default_A_H_warmup_enable                 (1)
#define default_dual_led_intersect_slope          (2)
#define default_led1_r_adj                        (1.0)
#define default_led1_b_adj                        (1.0)
#define default_led2_r_adj                        (1.0)
#define default_led2_b_adj                        (1.0)
#define default_face_supported                    (FALSE)

/* Create function pointers matching awb_core.h function definitions */
typedef q3a_core_result_type (* awb_init_core_func)
  (
    awb_handle_type* p_handle
  );

typedef q3a_core_result_type (* awb_process_core_func)
  (
    awb_handle_type                 handle,
    const awb_input_params_type*    input,
    awb_output_params_type*         output
  );

typedef q3a_core_result_type (* awb_set_parameters_core_func)
  (
    awb_handle_type                 handle,
    const awb_set_param_info_type*  set_param_info
  );

typedef q3a_core_result_type (* awb_get_parameters_core_func)
  (
    awb_handle_type                 handle,
    const awb_get_param_info_type*  get_param_info,
    awb_get_param_output_type*      get_param_output
  );

typedef void (* awb_deinit_core_func)
  (
    awb_handle_type handle
  );
/* Function pointer to set log level */
typedef void (*awb_set_log_core_func)(void);


/**
 * awb_biz_algo_ops_t: Defines a structure with all the AWB algo interface operations
 *
 * @set_parameters: Set operation
 * @get_parameters: Get operation
 * @process: Process stats operation
 * @init: Initialization
 * @deinit: De-init
 * @set_log: Set logs in algo
 **/
typedef struct {
  awb_set_parameters_core_func    set_parameters;
  awb_get_parameters_core_func    get_parameters;
  awb_process_core_func           process;
  awb_init_core_func              init;
  awb_deinit_core_func            deinit;
  awb_set_log_core_func           set_log;
} awb_biz_algo_ops_t;

/**
 * awb_bestshot_data_t: Define the bestshot data structure
 *
 *  @stored_bst_blue_gain_adj: the blue channel gain adjust for
 *                           this mode.
 *  @stored_wb: the stored white balance type when set best
 *            mode.
 *  @curr_mode: current bestshot mode.
 */
typedef struct {
  float                       stored_bst_blue_gain_adj;
  int                         stored_wb;
  awb_bestshot_mode_type_t    curr_mode;
} awb_bestshot_data_t;

/**
 * awb_ez_tune_t: Define the eztune data structure.
 *
 * @disable:
 * @lock_output:
 * @stored_gains:
 */
typedef struct {
  int disable;
  int lock_output;
  awb_rgb_gains_type stored_gains;
} awb_ez_tune_t;

/**
 * awb_flash_off_settings_t: Structure to store the flash off color temperature,
 * gains and ccm. This is used to restore after flash snapshot.
 */
typedef struct
{
  uint32 color_temperature;
  awb_rgb_gains_type gains;
  awb_rgb_gains_type unadjusted_awb_gain;
  awb_ccm_type ccm;
} awb_flash_off_settings_t;

/**
 * awb_mode_type_t: Define the awb type in algorithm.
 */
typedef enum {
  AWB_MIN_MINUS_1,
  AWB_AUTO = 0,
  AWB_CUSTOM,
  AWB_INCANDESCENT,
  AWB_FLUORESCENT,
  AWB_WARM_FLUORESCENT,
  AWB_DAYLIGHT,
  AWB_CLOUDY_DAYLIGHT,
  AWB_TWILIGHT,
  AWB_SHADE,
  AWB_OFF,
  AWB_MAX_PLUS_1
} awb_mode_type_t;

/** awb_biz_t: Define the business logic data structure.
 *
 *  @awb_process_input: frame info for stats process.
 *  @handle: core library handler.
 */
typedef struct _awb_biz_t
{
  awb_biz_algo_ops_t awb_algo_ops;
  boolean first_init;
  awb_config_t stats_config;
  awb_sensor_info_t sensor_info;
  q3a_stream_crop_t stream_crop_info;
  uint32 preview_width;
  uint32 preview_height;
  awb_rgb_gains_type chromatix_tl84_white_balance;
  awb_rgb_gains_type chromatix_d50_white_balance;
  awb_rgb_gains_type chromatix_incandescent_white_balance;
  awb_rgb_gains_type chromatix_d65_white_balance;
  awb_gain_adjust_type* gain_adj;
  float snow_blue_gain_adj_ratio;
  float beach_blue_gain_adj_ratio;
  float bst_blue_gain_adj;
  awb_ez_tune_t eztune;
  boolean awb_locked;
  boolean awb_enable;
  boolean is_awb_converge;
  awb_bestshot_data_t bestshot_info;
  awb_mode_type_t current_wb_type;
  awb_rgb_gains_type curr_gains;
  awb_rgb_gains_type stored_awb_gain;
  awb_rgb_gains_type unadjusted_awb_gain;
  int awb_update;
  int decision;
  uint32 color_temp;
  awb_rgb_gains_type snap_gains;
  uint32 snap_color_temp;
  int led_frame_skip_cnt;
  int max_led_frame_skip;
  uint8 manual_to_auto_skip_cnt;
  awb_operation_mode_t previous_op_mode;
  awb_operation_mode_t op_mode;
  stats_t stats;
  int32 video_hdr;
  awb_snapshot_hdr_type snapshot_hdr;
  boolean ez_running;
  boolean exif_dbg_enable;
  uint8 exif_dbg_level;
  uint32 sof_id;
  int prev_color_temp;
  int sample_decision[64];
  boolean is_still_capture;
  aec_led_est_state_t est_state;
  int use_led_estimation;
  boolean flash_on;
  q3a_flash_sensitivity_type flash_si;
  int led_state;
  boolean dual_LED_enable;
  float dual_led_flux_gain;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  awb_dual_led_setting_type dual_led_settings;
#endif
  awb_input_params_type core_input;
  awb_output_params_type core_output;
  awb_handle_type handle;
  uint32  stats_depth;
  boolean awb_ccm_enable;
  awb_flash_off_settings_t awb_flash_off_settings;
  boolean dual_led_calib_mode_enabled;
  boolean bg_config_stats_required;
  boolean awb_full_camif_enable;
  boolean in_longshot_mode;
  awb_dualcam_sync_info_t  dualcam_sync;
} awb_biz_t;

void * awb_biz_load_function(awb_ops_t *awb_ops);
void awb_biz_unload_function(awb_ops_t *awb_ops, void *lib_handler);

#endif /* __BIZ_AWB_H__ */

