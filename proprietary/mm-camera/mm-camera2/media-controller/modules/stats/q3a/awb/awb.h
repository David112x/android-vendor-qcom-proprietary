/* awb.h
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AWB_H__
#define __AWB_H__

#include "chromatix_common.h"
#include "q3a_stats.h"
#include "mct_event_stats.h"
#include "awb_core.h"
#include "stats_util.h"

/* This flag decides whether to use awb init setting from last camera session. */
#define AWB_START_RESTORE FALSE

typedef enum {
  AWB_STATS_YUV,
  AWB_STATS_BAYER
} awb_stats_type_t;

typedef enum {
  CAMERA_WB_MIN_MINUS_1 = -1,
  CAMERA_WB_AUTO ,  /* This list must match aeecamera.h */
  CAMERA_WB_CUSTOM,
  CAMERA_WB_INCANDESCENT,
  CAMERA_WB_FLUORESCENT,
  CAMERA_WB_WARM_FLUORESCENT,
  CAMERA_WB_DAYLIGHT,
  CAMERA_WB_CLOUDY_DAYLIGHT,
  CAMERA_WB_TWILIGHT,
  CAMERA_WB_SHADE,
  CAMERA_WB_MANUAL, /* HAL 1 manual AWB */
  CAMERA_WB_OFF,
  CAMERA_WB_MAX_PLUS_1
} awb_config3a_wb_t;

typedef enum {
  AWB_OUTPUT_UPDATE_PENDING = 0,
  AWB_UPDATE = 1,
  AWB_SEND_OUTPUT_EVENT,
  AWB_SEND_ROI_EVENT,
  AWB_SEND_MODE_EVENT,
  AWB_SEND_LOCK_EVENT,
  AWB_UPDATE_OFFLINE,
} awb_output_type_t;

typedef enum {
  AWB_BESTSHOT_OFF,
  AWB_BESTSHOT_AUTO,
  AWB_BESTSHOT_LANDSCAPE,
  AWB_BESTSHOT_SNOW,
  AWB_BESTSHOT_BEACH,
  AWB_BESTSHOT_SUNSET,
  AWB_BESTSHOT_NIGHT,
  AWB_BESTSHOT_PORTRAIT,
  AWB_BESTSHOT_BACKLIGHT,
  AWB_BESTSHOT_SPORTS,
  AWB_BESTSHOT_ANTISHAKE, /* 10 */
  AWB_BESTSHOT_FLOWERS,
  AWB_BESTSHOT_CANDLELIGHT,
  AWB_BESTSHOT_FIREWORKS,
  AWB_BESTSHOT_PARTY,
  AWB_BESTSHOT_NIGHT_PORTRAIT,
  AWB_BESTSHOT_THEATRE,
  AWB_BESTSHOT_ACTION,
  AWB_BESTSHOT_AR,
  AWB_BESTSHOT_FACE_PRIORITY,
  AWB_BESTSHOT_BARCODE, /* 20 */
  AWB_BESTSHOT_HDR,
  AWB_BESTSHOT_MAX
} awb_bestshot_mode_type_t;

typedef enum _awb_set_parameter_type {
  AWB_SET_PARAM_INIT_CHROMATIX_SENSOR   = 1,
  AWB_SET_PARAM_INIT_SENSOR_INFO,
  AWB_SET_PARAM_UI_FRAME_DIM,
  AWB_SET_PARAM_WHITE_BALANCE,
  AWB_SET_PARAM_RESTORE_LED_GAINS,
  AWB_SET_PARAM_LOCK,
  AWB_SET_PARAM_BESTSHOT,
  AWB_SET_PARAM_EZ_DISABLE,
  AWB_SET_PARAM_EZ_LOCK_OUTPUT,
  AWB_SET_PARAM_EZ_DUAL_LED_FORCE_IDX,
  AWB_SET_PARAM_LINEAR_GAIN_ADJ,
  AWB_SET_PARAM_AEC_PARM,
  AWB_SET_PARAM_OP_MODE,
  AWB_SET_PARAM_VIDEO_HDR,
  AWB_SET_PARAM_SNAPSHOT_HDR,
  AWB_SET_PARAM_STATS_DEBUG_MASK,
  AWB_SET_PARAM_ENABLE,
  AWB_SET_PARAM_EZ_TUNE_RUNNING,
  AWB_SET_PARAM_ROI,
  AWB_SET_PARAM_FD_ROI,
  AWB_SET_PARAM_PACK_OUTPUT,
  AWB_SET_PARAM_META_MODE,
  AWB_SET_PARAM_MANUAL_GAINS,
  AWB_SET_PARAM_CAPTURE_MODE,
  AWB_SET_MANUAL_AUTO_SKIP,
  AWB_SET_PARAM_CROP_INFO,
  AWB_SET_PARAM_MANUAL_WB,
  AWB_SET_PARAM_STATS_DEPTH,
  AWB_SET_PARAM_DO_LED_EST_FOR_AF,
  AWB_SET_PARAM_MULTI_CAM_SYNC_TYPE,   /* In dual cam, if algo doing sync. It set the role of the algo */
  AWB_SET_PARAM_LOW_POWER_MODE,        /* In dual cam, algo is request to work in low-power mode */
  AWB_SET_PARM_CUSTOM_EVT_MOD,         /* Custom event coming from Modules */
  AWB_SET_PARM_CUSTOM_EVT_CTRL,        /* Custom event control */
  AWB_SET_PARM_CUSTOM_EVT_HAL,         /* Custom event coming from HAL */
  AWB_SET_PARM_FAST_AEC_DATA,
  AWB_SET_PARAM_LONGSHOT_MODE,
  AWB_SET_PARAM_DUAL_LED_CALIB_MODE,   /* Sets the Dual LED calibration mode */
  AWB_SET_PARAM_LED_RESET,             /* Reset the LED parameters */
  AWB_SET_PARAM_ZSL_START,             /* Indicates ZSL start */
  AWB_SET_PARAM_ZSL_STOP,              /* Indicates ZSL stop */
  AWB_SET_PARM_INSTANT_AEC_TYPE,
  AWB_SET_PARM_CONV_SPEED,
  AWB_SET_PARM_ROLE_SWITCH,           /*Indicates the role of the algorithm*/
  AWB_SET_PARAM_MAX
} awb_set_parameter_type;

typedef struct _awb_sensor_info_t {
  uint32             sensor_res_width;
  uint32             sensor_res_height;
  uint32             sensor_top;
  uint32             sensor_left;
  q3a_sensor_type_t  sensor_type; /* Sensor type use. Usually default is Bayer sensor*/
  q3a_lens_type_t    lens_type; /* Type of lens the sensor use */
} awb_sensor_info_t;

/** awb_ui_frame_dim:
 *    @width:  TODO
 *    @height: TODO
 *
 * dimension of the ui (i.e preview) frame
 **/
typedef struct _awb_ui_frame_dim_t {
  uint32       width;
  uint32       height;
} awb_ui_frame_dim_t;

typedef q3a_operation_mode_t awb_operation_mode_t;

typedef struct _awb_set_parameter_init {
  awb_stats_type_t     stats_type;
  void                 *chromatix;
  awb_sensor_info_t    sensor_info;
  awb_operation_mode_t op_mode; /* op_mode can be derived from stream info */
  awb_ui_frame_dim_t   frame_dim;
  awb_stored_params_type *stored_params;
  awb_role_switch_params_t role_switch_param;
} awb_set_parameter_init_t;

typedef struct {
   int   exp_index;
   float lux_idx;
   int   aec_settled;

   /* Luma */
   uint32_t target_luma;
   uint32_t cur_luma;
   uint32_t average_luma;
   /* exposure */
   uint32_t cur_line_cnt;
   float cur_real_gain;
   float stored_digital_gain;
   float total_drc_gain;

   flash_sensitivity_t  flash_sensitivity;

   /*Led state*/
   int32_t led_mode;
   int led_state;
   int use_led_estimation;
   aec_led_est_state_t est_state;

   int exp_tbl_val;

   float Bv;
   float Tv;
   float Sv;
   float Av;
   boolean is_hdr_drc_enabled;
   aec_dual_led_settings_t dual_led_setting;
   float Dual_LED1_influence_ratio;
   /* custom opaque parameters */
   q3a_custom_data_t custom_param_awb;

   /*FaceAWB*/
   uint8_t  roi_count;
   float awb_roi_x[Q3A_CORE_MAX_ROI_COUNT];
   float awb_roi_y[Q3A_CORE_MAX_ROI_COUNT];
   float awb_roi_dx[Q3A_CORE_MAX_ROI_COUNT];
   float awb_roi_dy[Q3A_CORE_MAX_ROI_COUNT];
   float awb_roi_weight[Q3A_CORE_MAX_ROI_COUNT];
   float frm_width;
   float frm_height;
   int faceawb;
} awb_set_aec_parms;

typedef enum {
   MANUAL_WB_MODE_CCT,
   MANUAL_WB_MODE_GAIN,
   MANUAL_WB_MODE_MAX
} manual_wb_mode_type;

typedef struct {
  manual_wb_mode_type type;
  union {
  int32_t cct;
  awb_gain_t gains;
  } u;
} manual_wb_parm_t;

/** _awb_roi: RoI information in terms of frame
*
*    @x: horizontal offset of the region
*
*    @y: vertical offset of the region
*
*    @dx: width of the RoI region
*
*    @dy: height of the region
*
*    @roi: structure holding dimensions of each ROI
**/
typedef struct _awb_roi{
  uint16_t x;
  uint16_t y;
  uint16_t dx;
  uint16_t dy;
} awb_roi_t;

/** _awb_roi_info: ROI selected for awb stats.
*
*    @type: ROI type - Touch/Face
*
*    @enable:  check if touch roi is enable or not
*
*    @frm_id: frame ID
*
*    @num_roi: Number of ROIs detected
*
*    @roi: structure holding dimensions of each ROI
*
*    @weight array holding the weights
*
*    @frm_width: preview frame width
*
*    @frm_height: preview frame height
**/
typedef struct _awb_roi_info {
  q3a_roi_type_t    type;
  boolean           enable;
  uint32_t          frm_id;
  uint32_t          num_roi;
  awb_roi_t         roi[MAX_STATS_ROI_NUM];
  uint32_t          weight[MAX_STATS_ROI_NUM];
  uint32_t          frm_width;
  uint32_t          frm_height;
} awb_roi_info_t;
/** _awb_set_parameter
*
*
**/

typedef struct _awb_set_parameter {
  awb_set_parameter_type type;
  uint8_t               camera_id;

  union {
    awb_set_parameter_init_t init_param;
    awb_config3a_wb_t        awb_current_wb;
    manual_wb_parm_t         manual_wb_params;
    awb_bestshot_mode_type_t awb_best_shot;
    int                      ez_disable;
    int                      ez_lock_output;
    int                      linear_gain_adj;
    int8_t                   ez_force_dual_led_idx;
    awb_set_aec_parms        aec_parms;
    boolean                  awb_lock;
    boolean                  awb_enable;
    int32_t                  video_hdr;
    uint8                    snapshot_hdr;
    uint32_t                 stats_debug_mask;
    boolean                  ez_running;
    awb_roi_info_t           awb_roi_info;
    unsigned int             current_sof_id; /* SOF id wrt to set param*/
    uint8_t                  awb_meta_mode;
    awb_gain_t               awb_m_gains;
    uint32_t                 capture_type;
    q3a_stream_crop_t        stream_crop;
    uint32_t                 stats_depth;
    boolean                  est_for_af;
    q3a_custom_data_t        awb_custom_data;
    q3a_fast_aec_data_t      fast_aec_data;
    boolean                  longshot_mode;
    boolean                  dual_led_calib_mode;
    aec_convergence_type     instant_aec_type;
    float                    conv_speed;
    q3a_sync_mode_t          multi_cam_sync_type;
    q3a_low_power_mode_t     low_power_mode;
  } u;
} awb_set_parameter_t;

typedef struct {
  uint32_t t1;
  uint32_t t2;
  uint32_t t3;
  uint32_t t6;
  uint32_t t4;
  uint32_t mg;
  uint32_t t5;
}awb_exterme_col_param_t;

typedef struct {
  uint32_t regionW;
  uint32_t regionH;
  uint32_t regionHNum;
  uint32_t regionVNum;
  uint32_t regionHOffset;
  uint32_t regionVOffset;
}awb_stats_region_info_t;


typedef struct {
  chromatix_manual_white_balance_adapter  gain;
  uint32_t                                color_temp;
  chromatix_wb_exp_stats_type             bounding_box;
  //awb_stats_region_info_t region_info;
  awb_exterme_col_param_t                 exterme_col_param;
}stats_proc_awb_params_t;

typedef struct {
  chromatix_manual_white_balance_adapter  curr_gains;
  chromatix_manual_white_balance_adapter  unadjusted_awb_gain;
  uint32_t                                color_temp;
  int32_t led1_low_setting;   // led1 current value for pre-flash
  int32_t led2_low_setting;   // led2 current value for pre-flash
  int32_t led1_high_setting;  // led1 current value for main-flash
  int32_t led2_high_setting;  // led2 current value for main-flash
  boolean            awb_ccm_enable;
  awb_ccm_type       ccm;
}stats_proc_awb_gains_t;

typedef struct {
  q3a_capture_type_t capture_type;
  boolean flash_mode;
  float    r_gain;
  float    b_gain;
  float    g_gain;
  uint32_t color_temp;
  awb_ccm_type ccm;
} awb_capture_frame_info_t;

/** _awb_frame_batch:
 *    @num_batch:
 *    @Arrat of Frame Info
 *  description
 **/
typedef struct _awb_frame_batch {
  int16_t  num_batch;
  awb_capture_frame_info_t frame_batch[MAX_CAPTURE_BATCH_NUM];
} awb_frame_batch_t;

/* AWB GET DATA */
typedef enum {
  AWB_PARMS,
  AWB_GAINS,
  AWB_UNIFIED_FLASH,
  AWB_REQUIRED_STATS,
  AWB_INTERPOLATED_GAINS,
} awb_get_t;

typedef struct {
  awb_get_t type;
  union {
    stats_proc_awb_params_t awb_params;
    stats_proc_awb_gains_t  awb_gains;
  } d;
} stats_proc_get_awb_data_t;

typedef struct _awb_get_parameter {
  awb_get_t    type;
  uint8_t     camera_id;
  unsigned int current_frame_id; /* SOF id wrt to set param*/

  union {
    stats_proc_awb_params_t awb_params;
    stats_proc_awb_gains_t  awb_gains;

    awb_frame_batch_t frame_info;
    awb_request_stats_t request_stats;
    awb_interpolation_gain_params_t interpolated_gains;
  } u;
} awb_get_parameter_t;

typedef struct _awb_output_eztune_data {
  boolean  awb_enable;
  boolean  ez_running;
  int      awb_mode;
  int      prev_exp_index;
  int32_t  outlier_dist2_dayleft;
  int32_t  outlier_dist2_daytop;
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
} awb_output_eztune_data_t;


/** _awb_output_data
 *
 **/
typedef struct _awb_output_data {
  awb_output_type_t        type;
  stats_update_t           stats_update;
  uint32_t                 frame_id;
  awb_config_t             config;
  boolean                  need_config;
  boolean                  is_awb_converge;
  float                    r_gain;
  float                    g_gain;
  float                    b_gain;
  float                    unadjusted_r_gain;
  float                    unadjusted_g_gain;
  float                    unadjusted_b_gain;
  float                    snap_r_gain;
  float                    snap_g_gain;
  float                    snap_b_gain;
  int                      snap_color_temp;
  int                      color_temp;
  int                      awb_update;
  int                      decision;
  int                      samp_decision[64];
  int                      wb_mode;
  int                      best_mode;
  uint32_t                 sof_id;
  awb_output_eztune_data_t eztune_data;
  awb_operation_mode_t     op_mode;
  boolean                  awb_lock;
  awb_roi_info_t           awb_roi_info;
  awb_dual_led_settings_t  dual_led_settings;
  float                    dual_led_flux_gain;
  q3a_custom_data_t        awb_custom_param;
  boolean                  awb_ccm_enable;
  awb_ccm_type             ccm;
  awb_ccm_type             snap_ccm;
  float                    conv_speed;
  awb_dualcam_sync_info_t  dualcam_sync;
  uint32_t                 awb_debug_data_size;
  /*IMP please keep aec_debug_data_array at end only do not move this*/
  char                     awb_debug_data_array[AWB_DEBUG_DATA_SIZE];
} awb_output_data_t;
/*Data structure for awb ends */

typedef void    (* awb_callback_func)(awb_output_data_t *output, void *port);

typedef boolean (* awb_set_parameters_func)(awb_set_parameter_t *param,
    awb_output_data_t *output, uint8_t number_of_outputs, void *awb_obj);

typedef boolean (* awb_get_parameters_func)(awb_get_parameter_t *param,
  void *awb_obj);

typedef void    (* awb_process_func)(stats_t *stats,
  void *awb_obj, awb_output_data_t *output, uint8_t number_of_outputs);

typedef boolean  (* awb_estimate_cct)(void *awb_obj,
  float r_gain, float g_gain, float b_gain, float *cct, uint8_t camera_id);

typedef boolean  (* awb_estimate_gains)(void *awb_obj,
  float* r, float* g, float* b, float cct, uint8_t camera_id);

typedef boolean  (* awb_estimate_ccm)(void *awb_obj,
  float cct, awb_ccm_type* ccm, uint8_t camera_id);

typedef void    *(* awb_init_func)(void *awb_lib);

typedef void    (* awb_deinit_func)(void *awb_obj);

typedef struct {
  awb_set_parameters_func set_parameters;
  awb_get_parameters_func get_parameters;
  awb_process_func        process;
  awb_init_func           init;
  awb_deinit_func         deinit;
  awb_estimate_cct        estimate_cct;
  awb_estimate_gains      estimate_gains;
  awb_estimate_ccm        estimate_ccm;
}awb_ops_t;

#endif /* __AWB_H__ */
