/* aec.h
 *
 * Copyright (c) 2013-2016, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AEC_H__
#define __AEC_H__

#include "q3a_stats.h"
#include "mct_event_stats.h"
#include "q3a_common_types.h"
#include "aec_core.h"
#include "stats_util.h"

#define AEC_MANUAL_EXPOSURE_TIME_AUTO 0

/* To generate enum from macros */
#define AEC_PARAM_GENERATE_ENUM(ENUM) ENUM,
/* To generate string for enum from macros */
#define AEC_PARAM_GENERATE_STRING(STRING) #STRING,


/** _aec_interested_region:
 *    @enable:                 TODO
 *    @rgn_index:              TODO
 *    @rgn_index_window_start: TODO
 *    @rgn_index_window_end:   TODO
 *    @r:                      TODO
 *    @num_regions:            TODO
 *
 * TODO description
 **/
typedef struct _aec_interested_region {
  uint8_t     enable;
  stats_roi_t r[MAX_STATS_ROI_NUM];
  uint8_t     num_regions;
  int32_t     weight;
} aec_interested_region_t;

/** _aec_proc_roi_info:
 *    @roi:        TODO
 *    @hist:       TODO
 *    @type:       TODO
 *    @num_roi:    TODO
 *    @frm_width:  TODO
 *    @frm_height: TODO
 *
 * TODO description
 **/
typedef struct _aec_proc_roi_info {
  stats_roi_t    roi[MAX_STATS_ROI_NUM];
  aec_hist_t     hist[MAX_STATS_ROI_NUM];
  q3a_roi_type_t type;
  uint32_t       num_roi;
  uint32_t       frm_width;
  uint32_t       frm_height;
} aec_proc_roi_info_t;

/** _aec_proc_mtr_area:
 *    @num_area: TODO
 *    @weight:   TODO
 *    @mtr_area: TODO
 *
 * TODO description
 **/
typedef struct _aec_proc_mtr_area {
  int         num_area;
  int         weight[MAX_STATS_ROI_NUM];
  stats_roi_t mtr_area[MAX_STATS_ROI_NUM];
} aec_proc_mtr_area_t;

typedef enum {
  AEC_ISO_AUTO = 0,
  AEC_ISO_DEBLUR,
  AEC_ISO_100,
  AEC_ISO_200,
  AEC_ISO_400,
  AEC_ISO_800,
  AEC_ISO_1600,
  AEC_ISO_3200,
  AEC_ISO_MAX
} aec_iso_mode_t;

typedef enum {
  AEC_BESTSHOT_OFF,
  AEC_BESTSHOT_AUTO,
  AEC_BESTSHOT_LANDSCAPE,
  AEC_BESTSHOT_SNOW,
  AEC_BESTSHOT_BEACH,
  AEC_BESTSHOT_SUNSET,
  AEC_BESTSHOT_NIGHT,
  AEC_BESTSHOT_PORTRAIT,
  AEC_BESTSHOT_BACKLIGHT,
  AEC_BESTSHOT_SPORTS,
  AEC_BESTSHOT_ANTISHAKE, /* 10 */
  AEC_BESTSHOT_FLOWERS,
  AEC_BESTSHOT_CANDLELIGHT,
  AEC_BESTSHOT_FIREWORKS,
  AEC_BESTSHOT_PARTY,
  AEC_BESTSHOT_NIGHT_PORTRAIT,
  AEC_BESTSHOT_THEATRE,
  AEC_BESTSHOT_ACTION,
  AEC_BESTSHOT_AR,
  AEC_BESTSHOT_FACE_PRIORITY,
  AEC_BESTSHOT_BARCODE, /* 20 */
  AEC_BESTSHOT_HDR,
  AEC_BESTSHOT_MAX
} aec_bestshot_mode_type_t;


/** _aec_parms:
 *    @target_luma:        TODO
 *    @cur_luma:           TODO
 *    @exp_index:          TODO
 *    @exp_tbl_val:        TODO
 *    @lux_idx:            TODO
 *    @cur_real_gain:      TODO
 *    @snapshot_real_gain: TODO
 *
 * TODO description
 **/
typedef struct _aec_parms {
  uint32_t target_luma;
  uint32_t cur_luma;
  int32_t  exp_index;
  int32_t  exp_tbl_val;
  float    lux_idx;
  float    cur_real_gain;
  float    snapshot_real_gain;
} aec_parms_t;

#define AEC_GET_PARAM_ENUM_LIST(ADD_ENTRY)                                    \
  ADD_ENTRY(AEC_GET_PARAM_EXPOSURE_PARAMS)                                    \
  ADD_ENTRY(AEC_GET_PARAM_REQUIRED_STATS)                                     \
  ADD_ENTRY(AEC_GET_PARAM_UNIFIED_FLASH)                                      \
  ADD_ENTRY(AEC_GET_PARAM_LED_CAL_CONFIG)                                     \
  ADD_ENTRY(AEC_GET_PARAM_MAX)                                                \

typedef enum {
  AEC_GET_PARAM_ENUM_LIST(AEC_PARAM_GENERATE_ENUM)
} aec_get_parameter_type;


typedef enum {
  AEC_SENSOR_HDR_OFF = 0,
  AEC_SENSOR_HDR_IN_SENSOR,
  AEC_SENSOR_HDR_DRC,
  AEC_SENSOR_HDR_STAGGERED,
  AEC_SENSOR_HDR_MAX,
} aec_snapshot_hdr_type;

/** _aec_frame_batch:
 *    @num_batch:
 *
 *  description
 **/
typedef struct _aec_frame_batch {
  int16_t  num_batch;
  int      luma_target;
  int      current_luma;
  float    led_off_real_gain;
  float    led_off_sensor_gain;
  uint32_t led_off_linecount;
  aec_adrc_gain_params_t led_off_drc_gains;
  aec_adrc_hdr_params_t led_off_hdr_gains;
  int      valid_exp_entries;
  int      use_led_estimation;
  aec_auto_exposure_mode_t metering_type;
  aec_capture_frame_info_t frame_batch[MAX_CAPTURE_BATCH_NUM];
  flash_sensitivity_t      flash_si;
} aec_frame_batch_t;

/**
 * This structure defines the parametes for the 'get' function call
 *
 **/

/** _aec_get_parameter:
 *    @type:                  Parameter data type
 *    @exp_params:      Exposure parameters
 *    @request_stats:     Lists of stats available and required
 *
 * API to get information from algorithm
 **/
typedef struct _aec_get_parameter {
  aec_get_parameter_type type;
  uint8_t                camera_id;

  union {
    aec_exp_parms_t     exp_params;
    aec_request_stats_t request_stats;
    aec_frame_batch_t frame_info;
    aec_led_cal_config_t led_cal_config;
  } u;
} aec_get_parameter_t;
/** _aec_update_af:
 *    @luma_settled_cnt: TODO
 *    @cur_af_luma:      TODO
 *
 * TODO description
 **/
typedef struct _aec_update_af {
  unsigned int luma_settled_cnt;
  unsigned int cur_af_luma;
} aec_update_af_t;

/** _aec_update_awb:
 *    @prev_exp_index: TODO
 *
 * TODO description
 **/
typedef struct _aec_update_awb {
  int prev_exp_index;
} aec_update_awb_t;

typedef enum {
  AEC_OUTPUT_UPDATE_PENDING = 0, /* Algo is not ready to provide output */
  AEC_UPDATE = 1,
  AEC_SEND_EVENT,
} aec_output_type_t;

/** _aec_update_asd:
 *    @roi_info: TODO
 *    @SY:       TODO
 *
 * TODO description
 **/
typedef struct _aec_update_asd {
  aec_proc_roi_info_t roi_info;
  uint32_t            *SY;
} aec_update_asd_t;

#define AEC_SET_PARAM_ENUM_LIST(ADD_ENTRY)                                    \
  ADD_ENTRY(AEC_SET_PARAM_INVALID)                                            \
  ADD_ENTRY(AEC_SET_PARAM_INIT_CHROMATIX_SENSOR)                              \
  ADD_ENTRY(AEC_SET_PARAM_EXP_COMPENSATION)                                   \
  ADD_ENTRY(AEC_SET_PARAM_BRIGHTNESS_LVL)                                     \
  ADD_ENTRY(AEC_SET_PARAM_HJR_AF)                                             \
  ADD_ENTRY(AEC_SET_PARAM_METERING_MODE)                                      \
  ADD_ENTRY(AEC_SET_PARAM_ISO_MODE)                                           \
  ADD_ENTRY(AEC_SET_PARAM_ANTIBANDING)                                        \
  ADD_ENTRY(AEC_SET_PARAM_LED_RESET)                                          \
  ADD_ENTRY(AEC_SET_PARAM_PREPARE_FOR_SNAPSHOT)                               \
  ADD_ENTRY(AEC_SET_PARAM_FPS)                                       /* 10 */ \
  ADD_ENTRY(AEC_SET_PARAM_ROI)                                                \
  ADD_ENTRY(AEC_SET_PARAM_MTR_AREA)                                           \
  ADD_ENTRY(AEC_SET_PARAM_BESTSHOT)                                           \
  ADD_ENTRY(AEC_SET_PARAM_FD_ROI)                                             \
  ADD_ENTRY(AEC_SET_PARAM_EZ_DISABLE)                                         \
  ADD_ENTRY(AEC_SET_PARAM_EZ_LOCK_OUTPUT)                                     \
  ADD_ENTRY(AEC_SET_PARAM_EZ_FORCE_EXP)                                       \
  ADD_ENTRY(AEC_SET_PARAM_EZ_FORCE_LINECOUNT)                                 \
  ADD_ENTRY(AEC_SET_PARAM_EZ_FORCE_GAIN)                                      \
  ADD_ENTRY(AEC_SET_PARAM_EZ_TEST_ENABLE)                            /* 20 */ \
  ADD_ENTRY(AEC_SET_PARAM_EZ_TEST_ROI)                                        \
  ADD_ENTRY(AEC_SET_PARAM_EZ_TEST_MOTION)                                     \
  ADD_ENTRY(AEC_SET_PARAM_EZ_FORCE_SNAP_EXP)                                  \
  ADD_ENTRY(AEC_SET_PARAM_EZ_FORCE_SNAP_LINECOUNT)                            \
  ADD_ENTRY(AEC_SET_PARAM_EZ_FORCE_SNAP_GAIN)                                 \
  ADD_ENTRY(AEC_SET_PARAM_EZ_FORCE_DUAL_LED_IDX)                              \
  ADD_ENTRY(AEC_SET_PARAM_EZ_TUNE_RUNNING)                                    \
  ADD_ENTRY(AEC_SET_PARAM_LOCK)                                               \
  ADD_ENTRY(AEC_SET_PARAM_LED_EST)                                            \
  ADD_ENTRY(AEC_SET_PARAM_AFD_PARM)                                           \
  ADD_ENTRY(AEC_SET_PARAM_ASD_PARM)                                  /* 30 */ \
  ADD_ENTRY(AEC_SET_PARAM_INIT_SENSOR_INFO)                                   \
  ADD_ENTRY(AEC_SET_PARAM_ENABLE)                                             \
  ADD_ENTRY(AEC_SET_PARAM_AWB_PARM)                                           \
  ADD_ENTRY(AEC_SET_PARAM_GYRO_INFO)                                          \
  ADD_ENTRY(AEC_SET_PARAM_LED_MODE)                                           \
  ADD_ENTRY(AEC_SET_PARAM_BRACKET)                                            \
  ADD_ENTRY(AEC_SET_PARAM_UI_FRAME_DIM)                                       \
  ADD_ENTRY(AEC_SET_PARAM_CROP_INFO)                                          \
  ADD_ENTRY(AEC_SET_PARAM_ZSL_OP)                                             \
  ADD_ENTRY(AEC_SET_PARAM_VIDEO_HDR)                                 /* 40 */ \
  ADD_ENTRY(AEC_SET_PARAM_SNAPSHOT_HDR)                                       \
  ADD_ENTRY(AEC_SET_PARAM_ON_OFF)                                             \
  ADD_ENTRY(AEC_SET_PARAM_CTRL_MODE)                                          \
  ADD_ENTRY(AEC_SET_PARAM_MANUAL_EXP_TIME)                                    \
  ADD_ENTRY(AEC_SET_PARAM_MANUAL_GAIN)                                        \
  ADD_ENTRY(AEC_SET_PARAM_PACK_OUTPUT)                                        \
  ADD_ENTRY(AEC_SET_PARAM_SENSOR_ROI)                                         \
  ADD_ENTRY(AEC_SET_PARAM_DO_LED_EST_FOR_AF)                                  \
  ADD_ENTRY(AEC_SET_PARAM_PREP_FOR_SNAPSHOT_NOTIFY)                           \
  ADD_ENTRY(AEC_SET_PARAM_PREP_FOR_SNAPSHOT_LEGACY)                  /* 50 */ \
  ADD_ENTRY(AEC_SET_PARAM_RESET_LED_EST)                                      \
  ADD_ENTRY(AEC_SET_PARAM_LONGSHOT_MODE)                                      \
  ADD_ENTRY(AEC_SET_MANUAL_AUTO_SKIP)                                         \
  ADD_ENTRY(AEC_SET_PARAM_EXP_TIME)                                           \
  ADD_ENTRY(AEC_SET_PARAM_STATS_DEPTH)                                        \
  ADD_ENTRY(AEC_SET_PARM_FAST_AEC_DATA)                                       \
  ADD_ENTRY(AEC_SET_PARAM_PRECAPTURE_START)                                   \
  ADD_ENTRY(AEC_SET_PARAM_EFFECT)                                             \
  ADD_ENTRY(AEC_SET_PARAM_LED_FD_COMPLETED)                                   \
  /* Disable/Enable ADRC */                                                   \
  ADD_ENTRY(AEC_SET_PARAM_ADRC_ENABLE)                               /* 60 */ \
  /* Disable ADRC Using Ext: Events*/                                         \
  ADD_ENTRY(AEC_SET_PARAM_ADRC_FEATURE_DISABLE_FROM_APP)                      \
  /* Initialize Exposure Index from UI */                                     \
  ADD_ENTRY(AEC_SET_PARAM_INIT_EXPOSURE_INDEX)                                \
  ADD_ENTRY(AEC_SET_PARM_INSTANT_AEC_DATA)                                    \
  ADD_ENTRY(AEC_SET_PARM_DUAL_LED_CALIB_MODE)                                 \
  ADD_ENTRY(AEC_SET_PARAM_MAX_FLASH_CURRENT)                                  \
  ADD_ENTRY(AEC_SET_PARAM_CONFIG_AE_SCAN_TEST)                                \
  ADD_ENTRY(AEC_SET_PARAM_LIGHT_SENSOR_INFO)                                  \
  ADD_ENTRY(AEC_SET_PARAM_IR_MODE)                                            \
  ADD_ENTRY(AEC_SET_PARAM_CONV_SPEED)                                /* 70 */ \
  ADD_ENTRY(AEC_SET_PARAM_MULTI_CAM_SYNC_TYPE)                                \
  ADD_ENTRY(AEC_SET_PARAM_LOW_POWER_MODE)                                     \
  ADD_ENTRY(AEC_SET_PARAM_LED_CAL)                                            \
  ADD_ENTRY(AEC_SET_PARAM_SENSOR_FLASH_MODE)                                  \
  ADD_ENTRY(AEC_SET_PARAM_LCD_FLASH_STATE)                                    \
                                                                              \
  /*AEC custom data should be at the end always*/                             \
  /* Custom event coming from Modules */                                      \
  ADD_ENTRY(AEC_SET_PARM_CUSTOM_EVT_MOD)                                      \
  /* Custom event control */                                                  \
  ADD_ENTRY(AEC_SET_PARM_CUSTOM_EVT_CTRL)                                     \
  /* Custom event coming from HAL */                                          \
  ADD_ENTRY(AEC_SET_PARM_CUSTOM_EVT_HAL)                                      \
  ADD_ENTRY(AEC_SET_PARAM_ROLE_SWITCH)                                        \
  ADD_ENTRY(AEC_SET_PARAM_HDR_RATIO)                                          \
  ADD_ENTRY(AEC_SET_PARAM_MAX)                                                \

typedef enum {
  AEC_SET_PARAM_ENUM_LIST(AEC_PARAM_GENERATE_ENUM)
} aec_set_parameter_type;


/** _aec_set_parameter:
 *    @type:                    TODO
 *    @init_param:              TODO
 *    @aec_metering:            TODO
 *    @iso:                     TODO
 *    @antibanding:             TODO
 *    @antibanding_status:      TODO
 *    @brightness:              TODO
 *    @exp_comp:                TODO
 *    @fps_mode:                TODO
 *    @fps:                     TODO
 *    @aec_af_hjr:              TODO
 *    @aec_roi:                 TODO
 *    @fd_roi:                  TODO
 *    @mtr_area:                TODO
 *    @strobe_mode:             TODO
 *    @redeye_mode:             TODO
 *    @sensor_update:           TODO
 *    @bestshot_mode:           TODO
 *    @aec_bracket:             TODO
 *    @strobe_cfg_st:           TODO
 *    @ez_disable:              TODO
 *    @ez_lock_output:          TODO
 *    @ez_force_exp:            TODO
 *    @ez_force_linecount:      TODO
 *    @ez_force_gain:           TODO
 *    @ez_test_enable:          TODO
 *    @ez_test_roi;             TODO
 *    @ez_test_motion:          TODO
 *    @ez_force_snapshot_exp:   TODO
 *    @ez_force_snap_linecount: TODO
 *    @ez_force_snap_gain:      TODO
 *    @aec_lock:                TODO
 *    @sensitivity_ratio:       TODO
 *    @led_est_state:           TODO
 *    @asd_param:               TODO
 *    @afd_param:               TODO
 *    @awb_param:               TODO
 *    @led_mode:                TODO
 *    @stream_crop:             TODO
 *    @zsl_op:                  TODO
 *    @video_hdr:               TODO
 *    @snapshot_hdr:            TODO
 *    @multi_cam_sync_type: In dual cam, if algo doing sync. It set the role of the algo
 *    @low_power_mode: In dual cam, algo is request to work in low-power mode
 *    @instant_aec_type:        Instant aec type fast/aggressive
 *    @led_fd_enable:           Led fd feature enable/disable status to core
 * TODO description
 **/
typedef struct _aec_set_parameter {
  aec_set_parameter_type      type;
  uint8_t                     camera_id;

  union {
    aec_set_parameter_init_t      init_param;
    aec_auto_exposure_mode_t      aec_metering;
    cam_intf_parm_manual_3a_t     iso; /* HAL1 manual ISO/real gain */
    cam_intf_parm_manual_3a_t     manual_exposure_time; /* HAL1 manual exposure time */
    aec_antibanding_config_type_t antibanding;
    int                           brightness;
    int32_t                       exp_comp;
    aec_fps_mode_t                fps_mode;
    aec_fps_range_t               fps;
    uint32_t                      aec_af_hjr;
    aec_interested_region_t       aec_roi;
    aec_proc_roi_info_t           fd_roi;
    aec_proc_mtr_area_t           mtr_area;
    boolean                       redeye_mode;
    boolean                       sensor_update;
    aec_bestshot_mode_type_t      bestshot_mode;
    char                          aec_bracket[MAX_EXP_CHAR];
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
    float                         sensitivity_ratio;
    q3a_led_flash_state_t         led_est_state;
    aec_set_asd_param_t           asd_param;
    aec_set_afd_parm_t            afd_param;
    aec_set_awb_parm_t            awb_param;
    q3a_led_flash_mode_t          led_mode;
    q3a_stream_crop_t             stream_crop;
    int32_t                       zsl_op;
    int32_t                       video_hdr;
    aec_snapshot_hdr_type         snapshot_hdr;
    uint32_t                      stats_debug_mask;
    boolean                       enable_aec;
    aec_precapture_trigger_t      aec_trigger;
    int64_t                       manual_expTime;
    int32_t                       manual_gain;
    uint8_t                       aec_ctrl_mode;
    uint32_t                      current_sof_id;
    uint32_t                      capture_type;
    boolean                       est_for_af;
    aec_algo_gyro_info_t          gyro_info;
    boolean                       longshot_mode;
    uint32_t                      stats_depth;
    q3a_fast_aec_data_t           fast_aec_data;
    cam_effect_mode_type          effect_mode;
    boolean                       adrc_enable;
    uint32_t                      init_exposure_index;
    boolean                       led_cal_enable;
    aec_convergence_type          instant_aec_type;
    float                         light_sensor_info;
    uint32_t                      max_flash_current;
    q3a_ae_scan_test_config_t     ae_scan_test_config;
    aec_ir_mode_t                 ir_mode_type;
    float                         conv_speed;
    q3a_sync_mode_t               multi_cam_sync_type;
    q3a_low_power_mode_t          low_power_mode;
    flash_mode_t                  sensor_flash_mode;
    aec_role_switch_params_t      role_switch_params;
    aec_lcd_flash_state_t         lcd_flash_state;
    float                         hdr_ratio;

    /*AEC custom data should be at the end always*/
    q3a_custom_data_t             aec_custom_data;
  } u;
} aec_set_parameter_t;

typedef enum _aec_operation_type {
  AEC_OPERATION_TYPE_2D_NORM,
  AEC_OPERATION_TYPE_2D_ZSL,
  AEC_OPERATION_TYPE_3D_NORM,
  AEC_OPERATION_TYPE_3D_ZSL,
} aec_operation_type_t;

/* Option to control frame rate from AEC algo in dual cam
 *
 * @is_enable: Enable frame rate control from AEC and use fll given
 * @frame_length_lines: Total fll = active + blanking
 */
typedef struct {
  boolean is_enable;
  uint32_t frame_length_lines;
} aec_out_frame_rate_ctrl_t;


/** _aec_output_data
 *    @stats_update:            TODO
 *    @result:                  TODO
 *    @type:                    TODO
 *    @aec_af:                  AF related update
 *    @aec_awb:                 AWB related update
 *    @aec_asd:                 ASD related update
 *    @exp_index:               TODO
 *    @indoor_index:            TODO
 *    @outdoor_index:           TODO
 *    @lux_idx:                 TODO
 *    @pixelsPerRegion:         TODO
 *    @numRegions:              TODO
 *    @SY:                      TODO
 *    @preview_fps:             TODO
 *    @afr_enable:              TODO
 *    @metering_type:           TODO
 *    @iso:                     TODO
 *    @preview_exp_time:        TODO
 *    @aec_settled:             TODO
 *    @stored_digital_gain:     TODO
 *    @target_luma:             TODO
 *    @cur_luma:                TODO
 *    @high_luma_region_count:  TODO
 *    @cur_line_cnt:            TODO
 *    @cur_real_gain:           TODO
 *    @prev_sensitivity:        TODO
 *    @exp_tbl_val:             TODO
 *    @max_line_cnt:            TODO
 *    @aec_out_frame_rate_ctrl_t: Option to fully control FPS from AEC
 *    @sof_update:              TODO
 *    @comp_luma:               TODO
 *    @led_state:               TODO
 *    @use_led_estimation:      TODO
 *    @led_frame_skip_cnt:      TODO
 *    @max_led_frame_skip:      TODO
 *    @aec_flash_settled:       TODO
 *    @use_strobe:              TODO
 *    @strobe_len:              TODO
 *    @flash_si:                TODO
 *    @strobe_cfg_st:           TODO
 *    @prep_snap_no_led:        TODO
 *    @band_50hz_gap:           TODO
 *    @cur_atb:                 TODO
 *    @hjr_snap_frame_cnt:      TODO
 *    @asd_extreme_green_cnt:   TODO
 *    @asd_extreme_blue_cnt:    TODO
 *    @asd_extreme_tot_regions: total bayer stat regions 64x48
 *                              used to get ratio of extreme stats.
 *    @hjr_dig_gain:            TODO
 *    @snap:                    TODO
 *    @eztune:                  TODO
 *    @iso_Exif:                TODO
 *    @config:                  TODO
 *
 * TODO description
 **/
typedef struct _aec_output_data {
  aec_output_type_t        type;
  stats_update_t           stats_update;
  boolean                  result;
  aec_update_af_t          aec_af;
  aec_update_awb_t         aec_awb;
  aec_update_asd_t         aec_asd;
  int                      exp_index;
  int                      indoor_index;
  int                      outdoor_index;
  float                    lux_idx;
  int                      pixelsPerRegion;
  unsigned int             numRegions;
  unsigned int             SY[256];
  int                      preview_fps;
  boolean                  afr_enable;
  aec_auto_exposure_mode_t metering_type;
  uint32_t                 iso;
  float                    preview_exp_time;
  int                      aec_settled;
  float                    stored_digital_gain;
  uint32_t                 target_luma;
  uint32_t                 cur_luma;
  uint32_t                 cur_line_cnt;
  float                    cur_real_gain;
  float                    prev_sensitivity;
  uint32_t                 exp_tbl_val;
  int                      max_line_cnt;
  aec_out_frame_rate_ctrl_t aec_frame_rate_ctrl;
  int                      comp_luma;
  int                      avg_luma;
  flash_sensitivity_t      flash_si;
  boolean                  force_prep_snap_done;
  float                    band_50hz_gap;
  aec_antibanding_type_t   cur_atb;
  int                      hjr_snap_frame_cnt;
  int                      asd_extreme_green_cnt;
  int                      asd_extreme_blue_cnt;
  int                      asd_extreme_tot_regions;
  uint32_t                 hjr_dig_gain;
  aec_proc_snapshot_t      snap;
  aec_ez_tune_t            eztune;
  int16_t                  iso_Exif;
  float                    Bv_Exif;
  float                    Av_Exif;
  float                    Sv_Exif;
  float                    Tv_Exif;
  aec_config_t             config;
  int                      need_config;
  boolean                  locked_from_algo;
  int32_t                  trigger_id;
  q3a_custom_data_t        aec_custom_param;
  uint32_t                 aec_debug_data_size;
  boolean                  aec_locked;
  aec_vhdr_update_t        vhdr_update;
  float                    conv_speed;
  /* Face AEC: This variable set by aec core in low light.
   * TRUE: Wait for tuned frame count before sending FD ROI to core
   * FALSE: No need to wait. Send FD ROI immediately if led fd feature is enabled.
   */
  boolean                  led_fd_use_delay;
  uint32_t                 lcd_flash_settled;
  /*IMP please keep aec_debug_data_array at end only do not move this*/
  char                     aec_debug_data_array[AEC_DEBUG_DATA_SIZE];
} aec_output_data_t;

typedef boolean (* aec_set_parameters_func)(aec_set_parameter_t *param,
  aec_output_data_t *output, uint8_t num_of_outputs, void *aec_obj);
typedef boolean (* aec_get_parameters_func)(aec_get_parameter_t *param,
  void *aec_obj);
typedef boolean (* aec_process_func)(stats_t *stats, void *aec_obj,
  aec_output_data_t *output, uint8_t num_of_ouputs);
typedef void (* aec_callback_func)(aec_output_data_t *output, void *port);
typedef void *(* aec_init_func)(void *aec_lib);
typedef void (* aec_deinit_func)(void *aec);
typedef float (* aec_iso_to_real_gain)(void *aec_obj, uint32_t iso,
  uint8_t camera_id);
typedef boolean (* aec_get_version)(void *aec_obj, Q3a_version_t *version,
  uint8_t camera_id);


/** _aec_object:
 *    @obj_lock:       TODO
 *    @aec:            typecase to aec_internal_control_t
 *    @stats:          TODO
 *    @set_parameters: TODO
 *    @get_parameters: TODO
 *    @process:        TODO
 *    @init:           TODO
 *    @deinit:         TODO
 *    @cb:             TODO
 *    @output:         TODO
 *    @port:           TODO
 *    @thread_data:    typecase to q3a_thread_data_t
 *
 * TODO description
 **/
typedef struct _aec_object {
  pthread_mutex_t         obj_lock;
  void                    *aec;
  aec_set_parameters_func set_parameters;
  aec_get_parameters_func get_parameters;
  aec_process_func        process;
  aec_init_func           init;
  aec_deinit_func         deinit;
  aec_output_data_t       output;
  aec_iso_to_real_gain    iso_to_real_gain;
  q3a_custom_data_t       aec_custom_param;
  aec_get_version         get_version;
} aec_object_t;

#endif /* __AEC_H__ */
