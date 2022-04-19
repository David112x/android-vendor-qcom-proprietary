/* mct_event_stats.c
 *
 * Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __MCT_EVENT_STATS_H__
#define __MCT_EVENT_STATS_H__


#include "cam_ker_headers.h"
#include "mtype.h"
#include "cam_types.h"

#define MCT_MESH_ROLL_OFF_V4_TABLE_SIZE (17 * 13)
#define MAX_HPF_BUFF_SIZE          22
#define MAX_Y_CONFIG               3
#define MAX_BF_FIR_FILTER_SIZE     13
#define MAX_BF_FILTER_CORING_INDEX 17
#define BF_FW_MAX_ROI_REGIONS  281
#define MAX_BF_GAMMA_ENTRIES       32
#define MAX_PDAF_WINDOW 200
#define MAX_PDAF_PERIPHERAL_WINDOWS 8
#define MAX_PDAF_SINGLE_WINDOWS     1
#define MAX_BF_CAPS                3
#define MAX_IS_MESH_Y 10
#define NUM_MATRIX_ELEMENTS 9
#define MAX_VIDEO_HDR_FRAMES 3
#define AWB_MAX_ROI_COUNT 10

/* Size base in recomended size by the hardware API */
#define STATS_GYRO_MAX_SAMPLE_BUFFER_SIZE (32)

typedef struct _mct_event_stats_isp_data {
  enum msm_isp_stats_type stats_type;
  void *stats_buf;    /* 3A buffer pointer */
  uint32_t buf_size;      /* buffer size */
  uint32_t used_size; /* used size */
  uint32_t buf_idx;
} mct_event_stats_isp_data_t;

typedef struct _mct_event_stats_isp_rolloff {
  /* Table for red.  */
  uint16_t TableR[MCT_MESH_ROLL_OFF_V4_TABLE_SIZE];
  /* Table for Gr.   */
  uint16_t TableGr[MCT_MESH_ROLL_OFF_V4_TABLE_SIZE];
  /* Table for blue. */
  uint16_t TableB[MCT_MESH_ROLL_OFF_V4_TABLE_SIZE];
  /* Table for Gb.   */
  uint16_t TableGb[MCT_MESH_ROLL_OFF_V4_TABLE_SIZE];
} mct_event_stats_isp_rolloff_t;

typedef struct {
  uint32_t stats_mask;
}stats_cfg_t;

typedef struct {
  boolean start;      /* enable or disable */
  uint32_t stats_mask; /* which stats need to be enabled */
}stats_start_t;

typedef struct {
  unsigned int id; /* session id + stream id */
  unsigned int frame_id;
  int use_3d;
  /* IS divides a frame into horizontal meshes.  The top and bottom lines of
   * the frame and the lines separating the frames into sub-frames require
   * a matrix. So the number of matrices required is number of meshes + 1.
   * There are 9 numbers in 3x3 matrix.
   */
  int num_matrices;
  float transform_matrix[(MAX_IS_MESH_Y + 1) * NUM_MATRIX_ELEMENTS];
  unsigned int transform_type;
  int x;
  int y;
  int width;
  int height;
  int af_zoom_enable;
} is_update_t;

/** _mct_event_accel_vector_update
 *  @accel: linear acceleration (x/y/z) in m/s/s
 *  @accuracy: sensor accuracy (unreliable/low/medium/high)
 *  timestamp: timestamp of accel data
 *
 *  This structure is used to store and trasnmit accel
 *  vector received from the motion sensor.
 *
 **/
typedef struct _mct_event_accel_vector_update {
  float accel[3];
  uint8_t accuracy;
  uint64_t timestamp;
} mct_event_accel_vector_update_t;

/** _mct_event_gravity_vector_update
 *  @gravity:  gravity vector (x/y/z) in m/s/s
 *  @lin_accel: linear acceleration (x/y/z) in m/s/s
 *  @accuracy: sensor accuracy (unreliable/low/medium/high)
 *
 *  This structure is used to store and trasnmit gravity
 *  vector received from the motion sensor.
 *
 **/
typedef struct _mct_event_gravity_vector_update {
  float gravity[3];
  float lin_accel[3];
  uint8_t accuracy;
} mct_event_gravity_vector_update_t;

typedef struct {
  /* stats to isp */
  uint32_t landscape_severity;
  uint32_t backlight_detected;
  uint32_t backlight_scene_severity;
  boolean snow_or_cloudy_scene_detected;
  uint32_t snow_or_cloudy_luma_target_offset;
  boolean histo_backlight_detected;
  float saturation_adjust;
  uint32_t backlight_luma_target_offset;
  uint32_t portrait_severity;
  float asd_soft_focus_dgr;
  boolean mixed_light;
  boolean asd_enable;

  cam_auto_scene_t scene;
  uint32_t severity[S_MAX];
  uint32_t is_hdr_scene;
  float    hdr_confidence;
} asd_update_t;

typedef enum {
  AFD_TBL_OFF,
  AFD_TBL_60HZ,
  AFD_TBL_50HZ,
  AFD_TBL_MAX,
} afd_tbl_t;

typedef struct {
  /* afd update */
  boolean     afd_enable;
  afd_tbl_t   afd_atb;
  boolean     afd_monitor;
  boolean     afd_exec_once;
}afd_update_t;

typedef struct
{
  unsigned short LED1_setting;
  unsigned short LED2_setting;
  float          rg_ratio;
  float          bg_ratio;
  float          flux;
  float          first_entry_ratio;
  float          last_entry_ratio;
} LED_setting_type;

typedef struct {
  boolean is_valid;           /* indicates whether these settings have valid values */
  int32_t led1_low_setting;   /* led1 current value for pre-flash */
  int32_t led2_low_setting;   /* led2 current value for pre-flash */
  int32_t led1_high_setting;  /* led1 current value for main-flash */
  int32_t led2_high_setting;  /* led2 current value for main-flash */

  LED_setting_type low_setting;
  LED_setting_type high_setting;
  LED_setting_type full_setting;
} aec_dual_led_settings_t;

typedef struct {
  int32_t led1_low_setting;   /* led1 current value for pre-flash */
  int32_t led2_low_setting;   /* led2 current value for pre-flash */
  int32_t led1_high_setting;  /* led1 current value for main-flash */
  int32_t led2_high_setting;  /* led2 current value for main-flash */
} stats_dual_led_settings_t;

typedef enum {
  /* AEC IDLE state */
  AEC_EST_OFF,
  /* AEC Estimation start during preflash */
  AEC_EST_START,
  /* AEC Estimation is done in preflash */
  AEC_EST_DONE,
  /* AEC Estimation done without Flash LED */
  AEC_EST_NO_LED_DONE,
  /* AEC Estimation done for LED FD */
  AEC_EST_DONE_FOR_FD,
  /* AEC Estimation done for AF */
  AEC_EST_DONE_FOR_AF,
  /* Preflash is done as part of LED AF and Current presnapshot call will be skipped */
  AEC_EST_DONE_SKIP,
}aec_led_est_state_t;

typedef struct {
  float off;
  float low;
  float high;
} flash_sensitivity_t;

/** faceproc_capbility_aec_t
 *    @avg_frames:Average frames to detect face
 *    @max_frames:Max frames to detect face
**/
typedef struct {
    uint8_t avg_frames;
    uint8_t max_frames;
} faceproc_capbility_aec_t;


/* Base structure to support opaque data communication */
typedef struct {
  uint32_t size;
  void *data;
} stats_custom_update_t;

typedef enum {
  STATS_UPDATE_DST_ALL,
  STATS_UPDATE_DST_SENSOR = 1 << 0,
  STATS_UPDATE_DST_ISP = 1 << 1,
  STATS_UPDATE_DST_ISPIF = 1 << 2,
  STATS_UPDATE_DST_CPP = 1 << 3,
  STATS_UPDATE_DST_CAC = 1 << 4,
  STATS_UPDATE_DST_COMPANION = 1 << 5,
} stats_update_dst;

typedef struct {
  boolean is_valid;
  unsigned int *SY;
} SY_data_t;

/** aec_led_off_gains_t
**/
typedef struct {
  /* Sensor gain /line count */
  float sensor_gain;
  unsigned int linecnt;
  float s_gain;
  unsigned int s_linecnt;
  float l_gain;
  unsigned int l_linecnt;

  float lux_index;

  /* ISP gains for adrc */
  float real_gain;
  float total_drc_gain;
  float color_drc_gain;
  float gtm_ratio;
  float ltm_ratio;
  float la_ratio;
  float gamma_ratio;
} aec_sensitivity_data_t;

/* ir_config_data_t: Defines parameter for IR Camera Mode
 *  @ir_needed:               Determines IR LED is nedeed or not
 *  @ir_led_brightness:       IR LED brightness in percentage
 */
typedef struct {
   boolean ir_needed;
   float ir_led_brightness;
} ir_config_data_t;

typedef enum {
  VIDEO_HDR_SHORT_FRAME,
  VIDEO_HDR_LONG_FRAME,
  VIDEO_HDR_NORMAL_FRAME,
  VIDEO_HDR_FRAMES_MAX,
}aec_vhdr_frames_t;

typedef struct {
  uint32_t number_of_hdr_frames;
  float real_gain[VIDEO_HDR_FRAMES_MAX];
  uint32_t linecount[VIDEO_HDR_FRAMES_MAX];
  float hdr_gtm_gamma;
  float hdr_sensitivity_ratio;
  float hdr_exp_time_ratio;
} aec_vhdr_update_t;

/* Option to control frame rate from AEC algo in dual cam
 *
 * @is_enable: Enable frame rate control from AEC and use fll given
 * @frame_length_lines: Total fll = active + blanking
 */
typedef struct {
  boolean is_enable;
  uint32_t frame_length_lines;
} aec_frame_rate_control_t;

typedef struct {
  /* to isp */
  uint32_t stats_frm_id;
  float real_gain;          /* Consumed by ISP for gain based trigger */
  float sensor_gain;        /* Consumed by Sensor */
  float total_drc_gain;     /* Consumed by Misc    (Negative to disable) */
  float color_drc_gain;     /* Consumed by SCE/ACE (Negative to disable) */
  float gtm_ratio;          /* Consumed by GTM     (Negative to disable) */
  float ltm_ratio;          /* Consumed by LTM     (Negative to disable) */
  float la_ratio;           /* Consumed by LA      (Negative to disable) */
  float gamma_ratio;        /* Consumed by Gamma   (Negative to disable) */
  uint32_t linecount;
  float dig_gain;
  unsigned int numRegions;
  int pixelsPerRegion;
  int comp_luma;
  uint32_t exp_tbl_val;
  int asd_extreme_green_cnt;
  int asd_extreme_blue_cnt;
  int asd_extreme_tot_regions;
  uint32_t target_luma; /*current luma target*/
  uint32_t cur_luma; /*current vfe luma*/
  int32_t exp_index; /**/
  uint32_t avg_luma;
  float lux_idx;
  int settled;
  float band_50hz_gap;
  int cur_atb;
  int max_line_cnt;
  float stored_digital_gain;
  int32_t led_mode;
  int led_state;
  boolean prep_snap_no_led;
  int use_led_estimation;
  int aec_flash_settled;
  unsigned int luma_settled_cnt;
  SY_data_t SY_data;
  aec_led_est_state_t est_state;
  float exp_time;
  flash_sensitivity_t flash_sensitivity;
  unsigned int frame_id;
  uint32_t flash_needed;
  uint32_t luma_delta;
  int preview_fps;
  int preview_linesPerFrame;
  unsigned int sof_id;
  boolean frame_ctrl_en;
  int op_mode;
  boolean led_needed;

  aec_sensitivity_data_t led_off_params;

  boolean nightshot_detected;
  boolean hdr_indoor_detected;
  float hdr_exp_ratio;
  boolean strobe_enable;
  /*TDB : Apex information*/
  float Tv;
  float Sv;
  float Av;
  float Bv;
  float Ev;
  uint32_t exif_iso;
  stats_update_dst aec_dst;
  cam_flash_mode_t flash_hal;
  float l_real_gain;    /* HDR long gain */
  int32_t l_linecount;  /* HDR long linecount */
  float s_real_gain;    /* HDR short gain */
  int32_t s_linecount;  /* HDR short linecount */
  float m_real_gain;    /* HDR short gain */
  int32_t m_linecount;  /* HDR short linecount */
  float hdr_sensitivity_ratio;  /* HDR sensitivity ratio (gain*lc ratio) */
  float hdr_exp_time_ratio;     /* HDR exposure time ratio (lc ratio) */
  aec_vhdr_update_t vhdr_update;
  /* low-light capture feature
     True if: This is a low-light capture update */
  boolean low_light_capture_update_flag;
  int32_t gamma_flag; /* Set for gamma related features */
  int32_t nr_flag; /* Set for noise reduction features */
  /* low-light scene type detected, provides data about
     the scene lighting conditions */
  int32_t low_light_shutter_flag;
  int32_t lls_flag;
  int32_t lds_Flag;

  int32_t touch_ev_status; /* Touch exp compensation status */

  ir_config_data_t ir_config; /*IR camera mode ouput*/

  aec_frame_rate_control_t frame_rate_ctrl; /* Option to fully control FPS from AEC */

  /* custom opaque parameters */
  stats_custom_update_t aec_custom_param_update;

  aec_dual_led_settings_t dual_led_setting;
  float Dual_LED1_influence_ratio;
  boolean af_exposure_compensate;

  /*Face AWB*/
  uint8_t  roi_count;
  float awb_roi_x[AWB_MAX_ROI_COUNT];
  float awb_roi_y[AWB_MAX_ROI_COUNT];
  float awb_roi_dx[AWB_MAX_ROI_COUNT];
  float awb_roi_dy[AWB_MAX_ROI_COUNT];
  float awb_roi_weight[AWB_MAX_ROI_COUNT];
  float frm_width;
  float frm_height;
  int faceawb;

  int led_cal_state; /* Led calibration state */
  /* Exposure multiplier, only used in the dual-camera usecase. Peer's exposure
     can be calculated by multiplying the overall exposure from this AEC update by
     this multiplier. */
  float dual_exp_multiplier;
}aec_update_t;

typedef struct {
  uint32_t h_num;
  uint32_t v_num;
}stats_grid_t;

typedef struct {
  /* ---- To Sensor -----*/
  /* Move Lens*/
  uint32_t stats_frm_id;
  boolean move_lens;
  int32_t direction;
  int32_t num_of_steps;
  uint16_t cur_logical_lens_pos;
  boolean use_dac_value;
  uint32_t num_of_interval;
  uint16_t pos[MAX_NUMBER_OF_STEPS];
  uint16_t delay[MAX_NUMBER_OF_STEPS];

  /*Reset Lens*/
  boolean reset_lens;
  int reset_pos;

  /* Stop AF */
  boolean stop_af;

  /* To modules */
  boolean af_active;
  int af_status;
  cam_af_state_t af_state;
  boolean cont_af_enabled;

  /*----- To AEC ----*/
  boolean check_led;
  /* sensor output */
  int16_t dac_value;
  uint32_t scale;
  float diopter;

  /* custom opaque parameters */
  stats_custom_update_t af_custom_param_update;
}af_update_t;

typedef struct {
  int32_t defocus;
  int8_t df_confidence; /* 0-good, (-1)-not good*/
  uint32_t df_conf_level; /* = 1024*ConfidentLevel/Threshold */
  float phase_diff;
} pdaf_defocus_t;

typedef enum _pdaf_lib_processing_result_t {
  PDAF_PDLIB_RESULT_PARTIAL,
  PDAF_PDLIB_RESULT_DONE,
}pdaf_lib_processing_result_t;

typedef struct {
  uint8_t * pd_stats;
 /* sensor output */
  float x_offset;
  float y_offset;
  float width;
  float height;
  uint32_t x_win_num;
  uint32_t y_win_num;
  uint32_t num_of_valid_data_wrt_camif;
  pdaf_defocus_t defocus[MAX_PDAF_WINDOW];
  boolean is_peripheral_valid;
  pdaf_defocus_t peripheral[MAX_PDAF_PERIPHERAL_WINDOWS];
  boolean is_single_valid;
  pdaf_defocus_t single[MAX_PDAF_SINGLE_WINDOWS];
  boolean status;
  pdaf_lib_processing_result_t stat_process_result;
} pdaf_update_t;

typedef struct {
  int current_step;
} lens_position_update_isp_t;

typedef struct {
  uint32_t t1;
  uint32_t t2;
  uint32_t t3;
  uint32_t t6;
  uint32_t t4;
  uint32_t mg;
  uint32_t t5;
}awb_exterme_color_param_t;

typedef struct {
  float r_gain;
  float g_gain;
  float b_gain;
} awb_gain_t;

typedef struct
{
  unsigned char y_min;                      // LumaMin
  unsigned char y_max;                      // LumaMax

  /* Slope of neutral region line 1 */
  char m1;                          // Slope1
  /* Slope of neutral region line 2 */
  char m2;                          // Slope2

  /* Slope of neutral region line 3 */
  char m3;                          // Slope3

  /* Slope of neutral region line 4 */
  char m4;                          // Slope4

  /* Cb intercept of neutral region line 1 */
  short c1;                         // CbOffset1

  /* Cb intercept of neutral region line 2 */
  short c2;                         // CrOffset2

  /* Cb intercept of neutral region line 3 */
  short c3;                         // CbOffset3

  /* Cb intercept of neutral region line 4 */
  short c4;                         // CrOffset4
} awb_bounding_box_t;

typedef struct {
  boolean is_valid;           /* indicates whether these settings have valid values */
  int32_t led1_low_setting;   /* led1 current value for pre-flash */
  int32_t led2_low_setting;   /* led2 current value for pre-flash */
  int32_t led1_high_setting;  /* led1 current value for main-flash */
  int32_t led2_high_setting;  /* led2 current value for main-flash */
} awb_dual_led_settings_t;

/**
 * Defines the number of rows in the color correction matrix (CCM).
 */
#define AWB_NUM_CCM_ROWS (3)

/**
 * Defines the number of columns in the color correction matrix (CCM).
 */
#define AWB_NUM_CCM_COLS (3)

/**
 * Defines the format of the color correction matrix.
 *
 * @awb_ccm_enable: Flag indicates CCM enabled from 3A chromatix
 * @hard_awb_ccm_flag: when set, no extra modify allowed, EX: satuation effect
 * @ccm_update_flag: Flag indicates CCM output is to be used
 * @ccm: The color correction matrix
 * @ccm_offset: The offsets for color correction matrix
 *
 */
typedef struct
{
  boolean     awb_ccm_enable;
  boolean     hard_awb_ccm_flag;
  boolean     ccm_update_flag;
  float       ccm[AWB_NUM_CCM_ROWS][AWB_NUM_CCM_COLS];
  float       ccm_offset[AWB_NUM_CCM_ROWS];
} awb_ccm_update_t;

typedef struct {
  cam_awb_state_t awb_state;
  /* to isp */
  uint32_t stats_frm_id;
  boolean manul_update;
  awb_gain_t gain; /* cast to chromatix_manual_white_balance_type */
  awb_gain_t unadjusted_gain; /* cast to chromatix_manual_white_balance_type */
  uint32_t color_temp;
  awb_bounding_box_t bounding_box; /* cast to chromatix_wb_exp_stats_type */
  awb_exterme_color_param_t exterme_color_param;
  int wb_mode;
  int best_mode;
  int sample_decision[64];
  awb_ccm_update_t ccm_update;
  awb_gain_t predicted_gain;
  uint32_t   predicted_cct;
  awb_dual_led_settings_t dual_led_setting;
  float dual_led_flux_gain;
  int decision;
  stats_update_dst awb_dst;
  /* True if: This is a low-light capture update */
  boolean low_light_capture_update_flag;
  stats_custom_update_t awb_custom_param_update; /* custom opaque parameters */
}awb_update_t;

/* Manual Stats settings */

typedef struct {
  float sensor_gain;
  uint32_t linecount;
  float lux_idx;
  uint32_t exif_iso;
  int aec_settled;
}aec_manual_update_t;

typedef struct {
  awb_gain_t manual_gain; /* cast to chromatix_manual_white_balance_type */
  float cct;
}awb_manual_update_t;
typedef enum {
  STATS_UPDATE_AEC   = (1 << 0),
  STATS_UPDATE_AWB   = (1 << 1),
  STATS_UPDATE_AF    = (1 << 2),
  STATS_UPDATE_ASD   = (1 << 3),
  STATS_UPDATE_AFD   = (1 << 4),
}stats_update_mask_t;

typedef struct {
  union {
    aec_update_t   aec_update;
    awb_update_t   awb_update;
    af_update_t    af_update;
    pdaf_update_t  pdaf_update;
    asd_update_t   asd_update;
    afd_update_t   afd_update;
  };
  stats_update_mask_t flag;
}stats_update_t;

typedef struct {
  float real_gain[5];       /* Consumed by ISP for gain based trigger */
  float sensor_gain[5];     /* Consumed by Sensor */
  float total_drc_gain;     /* Consumed by Misc    (Negative to disable) */
  float color_drc_gain;     /* Consumed by SCE/ACE (Negative to disable) */
  float gtm_ratio;          /* Consumed by GTM     (Negative to disable) */
  float ltm_ratio;          /* Consumed by LTM     (Negative to disable) */
  float la_ratio;           /* Consumed by LA      (Negative to disable) */
  float gamma_ratio;        /* Consumed by Gamma   (Negative to disable) */
  uint32_t linecount[5];
  float led_off_real_gain;
  float led_off_sensor_gain;
  float led_off_total_drc_gain; /* Consumed by Misc    (Negative to disable) */
  float led_off_color_drc_gain; /* Consumed by SCE/ACE (Negative to disable) */
  float led_off_gtm_ratio;      /* Consumed by GTM     (Negative to disable) */
  float led_off_ltm_ratio;      /* Consumed by LTM     (Negative to disable) */
  float led_off_la_ratio;       /* Consumed by LA      (Negative to disable) */
  float led_off_gamma_ratio;    /* Consumed by Gamma   (Negative to disable) */
  uint32_t led_off_linecount;
  uint32_t valid_entries;
  uint32_t trigger_led;
  float exp_time;
  float lux_idx;
  boolean flash_needed;
  uint32_t exif_iso;
  float l_real_gain;    /* HDR long gain */
  int32_t l_linecount;  /* HDR long linecount */
  float m_real_gain;    /* HDR middle gain */
  int32_t m_linecount;  /* HDR middle linecount */
  float s_real_gain;    /* HDR short gain */
  int32_t s_linecount;  /* HDR short linecount */
  float hdr_sensitivity_ratio;  /* HDR sensitivity ratio (gain*lc ratio) */
  float hdr_exp_time_ratio;     /* HDR exposure time ratio (lc ratio) */
  stats_custom_update_t aec_get_custom_data;
  void* p_userdata;
  aec_dual_led_settings_t dual_led_setting;
  flash_sensitivity_t flash_sensitivity;
}aec_get_t;

typedef struct {
  float r_gain;
  float g_gain;
  float b_gain;
}_awb_get_t;

typedef struct {
  aec_get_t aec_get;
  _awb_get_t awb_get;
  stats_update_mask_t flag;
} __attribute__((__may_alias__))stats_get_data_t;

typedef struct {
  boolean is_valid;
  stats_grid_t grid_info;
  cam_rect_t   roi;
  uint32_t r_Max;
  uint32_t gr_Max;
  uint32_t b_Max;
  uint32_t gb_Max;
  uint8_t shift_bits;
} aec_bg_config_t;

typedef enum {
  STATS_CHANNEL_Y,
  STATS_CHANNEL_GR,
  STATS_CHANNEL_GB,
  STATS_CHANNEL_R,
  STATS_CHANNEL_B,
  STATS_CHANNEL_MAX
} stats_channel_type_t;

typedef struct {
  stats_grid_t grid_info;
  cam_rect_t roi;
  boolean is_bin_uniform;
  stats_channel_type_t channel_type;
} aec_bhist_config_t;

/* BE and BG config have the same output */
typedef aec_bg_config_t aec_be_config_t;

typedef struct {
  aec_bg_config_t bg_config;
  aec_bg_config_t aec_bg_config;
  aec_bhist_config_t bhist_config;
  aec_update_t aec_init_val;
  aec_be_config_t be_config;      /* BE and HDR-BE stats */
}aec_config_t;

typedef struct {
  aec_bg_config_t bg_config;
  stats_grid_t grid_info;
  cam_rect_t   roi;
  awb_update_t awb_init_val;
}awb_config_t;

/* mct_event_stats_config_type:
 * ROI configuration type
 */
typedef enum {
  STATS_CONFIG_MODE_SINGLE,
  STATS_CONFIG_MODE_MULTIPLE,
} mct_event_stats_config_type;

/** mct_stats_hpf_size:
 *  */
typedef enum _mct_stats_hpf_size {
  MCT_EVENT_STATS_HPF_LEGACY,
  MCT_EVENT_STATS_HPF_2X5,
  MCT_EVENT_STATS_HPF_2X11
} mct_stats_hpf_size_type;

typedef enum _mct_event_adrc_isp_module_mask{
  ISP_ADRC_MOD_NONE            = 0,
  ISP_ADRC_MOD_LA              = 1,
  ISP_ADRC_MOD_LTM             = 2,
  ISP_ADRC_MOD_GTM             = 3
} mct_event_adrc_isp_module_mask_type;

typedef struct _mct_event_stats_isp_adrc {
  uint32_t isp_adrc_hw_module_id_mask;
} mct_event_stats_isp_adrc_hw_module_t;

/** _af_config_mask: Mask to differentiate which AF kernel to
 *  configure */
typedef enum _af_config_mask {
  MCT_EVENT_STATS_BF_SINGLE = (1 << 0),
  MCT_EVENT_STATS_BF_SCALE  = (1 << 1),
  MCT_EVENT_STATS_BF_FW = (1 << 2),
} af_config_mask_type;

typedef enum _bf_filter_type_t {
  BF_FILTER_TYPE_H_1,
  BF_FILTER_TYPE_H_2,
  BF_FILTER_TYPE_V,
  BF_FILTER_TYPE_MAX,
} bf_filter_type_t;

/** mct_stats_filter_caps_t:
 *
 *  @filter_type: type of filter for which caps are posted
 *  @num_fir_a_kernel: number of 'a' coefficients in FIR
 *                   filter
 *  @num_iir_a_kernel: number of 'a' coefficients in IIR
 *                   filter
 *  @num_iir_b_kernel: number of 'b' coefficients in IIR
 *                   filter
 *  @fir_disable_supported: boolean flag to indicate whether
 *                        FIR disable supported
 *  @iir_disable_supported: boolean flag to indicate whether
 *                        IIR disable supported
 *  @downscale_supported: boolean flag to indicate whether
 *                      downscale supported
 *  @is_coring_variable: Is coring fixed / variable
 *
 **/
typedef struct _mct_stats_filter_caps_t {
  bf_filter_type_t filter_type;
  uint32_t         num_fir_a_kernel;
  uint32_t         num_iir_a_kernel;
  uint32_t         num_iir_b_kernel;
  boolean          fir_disable_supported;
  boolean          iir_disable_supported;
  boolean          downscale_supported;
  boolean          is_coring_variable;
} mct_stats_filter_caps_t;

/** mct_stats_bf_roi_caps_t:
 *
 *  @is_valid: boolean flag to indicate whether params are
 *           valid
 *  @min_grid_width: minimum per grid width
 *  @max_grid_width: maximum per grid width
 *  @min_grid_height: minimum per grid height
 *  @max_grid_height: maximum per grid height
 *  @min_hor_offset: minimum horizontal offset of any grid
 *  @max_hor_offset: maximum horizontal offset of any grid
 *  @min_ver_offset: minimum vertical offset of any grid
 *  @max_ver_offset: maximum vertical offset of any grid
 *  @min_hor_grids: minimum horizontal grids
 *  @max_hor_grids: maximum horizontal grids
 *  @min_ver_grids: minimum vertical grids
 *  @max_ver_grids: maximum vertical grids
 *  @max_total_grids: maximum total grids
 *
 **/
typedef struct _mct_stats_bf_roi_caps_t {
  boolean  is_valid;
  uint32_t min_grid_width;
  uint32_t max_grid_width;
  uint32_t min_grid_height;
  uint32_t max_grid_height;
  uint32_t min_hor_offset;
  uint32_t max_hor_offset;
  uint32_t min_ver_offset;
  uint32_t max_ver_offset;
  uint32_t min_hor_grids;
  uint32_t max_hor_grids;
  uint32_t min_ver_grids;
  uint32_t max_ver_grids;
  uint32_t max_total_grids;
} mct_stats_bf_roi_caps_t;

/** mct_stats_bf_fw_caps_t:
 *
 *  @is_valid: boolen flag to indicate whether params are
 *           valid
 *  @is_ch_sel_supported: boolean flag to indicate whether
 *                      channel select block is present
 *  @is_lut_supported: boolean flag to indicate whether RGB
 *                   LUT (gamma) block is present
 *  @lut_table_size: RGB LUT table size
 *  @min_independent_windows: minimum independent windows
 *  @max_independent_windows: maximum independent windows
 *  @min_hor_start_pixel_spacing: minimum horizontal spacing
 *                              between start pixels of any two
 *                              consecutive region
 *  @min_hor_end_pixel_spacing: minimum horizontal spacing
 *                              between end pixels of any two
 *                              consecutive region
 *  @max_overlapping_windows: maximum number of overlapping
 *                          regions - a single pixel
 *                          can contribute to N regions where
 *                          N is the value mentioned in this
 *                          field
 *  @max_hor_grids_each_line: Number of consecutive horizontal
 *                          grids in each line
 *
 **/
typedef struct _mct_stats_bf_fw_caps_t {
  boolean                 is_valid;
  boolean                 is_ch_sel_supported;
  boolean                 is_lut_supported;
  boolean                 lut_table_size;
  uint32_t                min_independent_windows;
  uint32_t                max_independent_windows;
  uint32_t                min_hor_start_pixel_spacing;
  uint32_t                min_hor_end_pixel_spacing;
  uint32_t                max_overlapping_windows;
  uint32_t                max_hor_grids_each_line;
  mct_stats_filter_caps_t filter_caps[BF_FILTER_TYPE_MAX];
  uint32_t                num_filter_caps;
} mct_stats_bf_fw_caps_t;

/** mct_stats_rs_caps_t:
 *
 *  @is_valid:    is the maxHnum/VNum valid
 *  @max_Hnum:    max Horizontal regions hardware supports
 *  @max_VNum:    max Vertical regions hardware supports
 *
 **/
typedef struct _mct_stats_rs_caps_t {
  boolean                 is_valid;
  uint32_t                max_Hnum;
  uint32_t                max_Vnum;
} mct_stats_rs_caps_t;

/** mct_stats_bf_caps_t:
 *
 *  @bf_mask: type of bf supported
 *  @bf_roi_caps: bf roi capabilities
 *  @bf_fw_caps: bf fw capabilities
 *
 **/
typedef struct _mct_stats_bf_caps_t {
  af_config_mask_type        bf_mask;
  mct_stats_bf_roi_caps_t    bf_roi_caps;
  mct_stats_bf_fw_caps_t     bf_fw_caps;
} mct_stats_bf_caps_t;

/**mct_stats_info_t: Used by ISP to report the supported stats.
 *
 * @stats_mask: the mask that tells which stats are supported by
 *   ISP
 *
 **/
typedef struct mct_stats_info_t {
  uint32_t stats_mask;
  uint32_t stats_depth;
  uint32_t pipeline_bit_width;
  mct_stats_hpf_size_type kernel_size;
  mct_stats_bf_caps_t     bf_caps;
  mct_stats_rs_caps_t     rs_caps;
} mct_stats_info_t;

/**bf_config_t: Information required to configure AF stats for
 * primary AF kernel.
 *
 * @roi: roi dimensions
 *
 * hpf: filter coefficients
 *
 * @r_min/b_min/gr_min/gb_min: minimum sharpness value
 **/
typedef struct _bf_config {
  cam_rect_t   roi;
  int hpf[MAX_HPF_BUFF_SIZE];
  unsigned int r_min;
  unsigned int b_min;
  unsigned int gr_min;
  unsigned int gb_min;
} bf_config_t;

/**bf_scale_config_t: Information required to configure AF stats
 * for secondary AF kernel.
 *
 * @roi: roi dimensions.
 *
 * @hpf: filter coefficients
 *
 * @scale: downscaling factor
 *
 * @r_min/b_min/gr_min/gb_min: minimum sharpness value
 **/
typedef struct _bf_scale_config {
  cam_rect_t roi;
  int hpf[MAX_HPF_BUFF_SIZE];
  int scale;
  unsigned int r_min;
  unsigned int b_min;
  unsigned int gr_min;
  unsigned int gb_min;
} bf_scale_config_t;

/**bf_common_t: Parameters common to different kernels.
 *
 * @grid_info: number of horizontal and vertical regions.
 **/
typedef struct _bf_common {
  stats_grid_t grid_info;
} bf_common_t;

typedef enum _bf_channel_sel_t {
  BF_CHANNEL_SELECT_G,
  BF_CHANNEL_SELECT_Y,
  BF_CHANNEL_SELECT_MAX,
} bf_channel_sel_t;

typedef enum _bf_input_g_sel_t {
  BF_INPUT_SELECT_GR,
  BF_INPUT_SELECT_GB,
  BF_INPUT_SELECT_MAX,
} bf_input_g_sel_t;

/** bf_input_cfg_t:
 *
 *  @is_valid: flag to indicate whether bf_input_cfg_t params
 *           are valid
 *  @bf_input_sel: type of channel for which stats to be
 *               collected
 *  @y_a_cfg: filter coefficients for Y conversion
 **/
typedef struct _bf_input_cfg_t {
  boolean        is_valid;
  bf_channel_sel_t bf_channel_sel;
  bf_input_g_sel_t bf_input_g_sel;
  float          y_a_cfg[MAX_Y_CONFIG];
} bf_input_cfg_t;

/** bf_gamma_lut_cfg_t:
 *
 *  @is_valid: flag to indicate whether bf_gamma_lut_cfg_t
 *           params are valid
 *  @gamma_lut: lut table for gamma
 *  @num_gamma_lut: number of valid gamma lut entries
 **/
typedef struct bf_gamma_lut_cfg_t {
  boolean  is_valid;
  uint32_t gamma_lut[MAX_BF_GAMMA_ENTRIES];
  uint32_t num_gamm_lut;
} bf_gamma_lut_cfg_t;

/** bf_scale_cfg_t:
 *
 *  @is_valid: flag to indicate whether bf_scale_cfg_t params
 *           are valid
 *  @bf_scale_en: bf scale enable bit
 *  @roi: roi params of bf scale
 *  @scale_m: M scale value
 *  @scale_n: N scale value
 **/
typedef struct _bf_scale_cfg_t {
  boolean    is_valid;
  boolean    bf_scale_en;
  int32_t    scale_m;
  int32_t    scale_n;
} bf_scale_cfg_t;

/** bf_fir_filter_cfg_t:
 *
 *  @a: coefficients for bf stats H and V filter
 **/
typedef struct _bf_fir_filter_cfg_t {
  boolean enable;
  int32_t a[MAX_BF_FIR_FILTER_SIZE];
  uint32_t num_a;
} bf_fir_filter_cfg_t;

/** bf_iir_filter_cfg_t:
 *
 *  @a11: a11 coefficient in Q14
 *  @a12: a12 coefficient in Q14
 *  @a21: a21 coefficient in Q14
 *  @a22: a22 coefficient in Q14
 *  @b10: b10 coefficient in Q14
 *  @b11: b11 coefficient in Q14
 *  @b12: b12 coefficient in Q14
 *  @b20: b20 coefficient in Q14
 *  @b21: b21 coefficient in Q14
 *  @b22: b22 coefficient in Q14
 **/
typedef struct _bf_iir_filter_cfg_t {
  boolean enable;
  float b10;
  float b11;
  float b12;
  float a11;
  float a12;
  float b20;
  float b21;
  float b22;
  float a21;
  float a22;
} bf_iir_filter_cfg_t;

/** bf_filter_coring_cfg_t:
 *
 *  @threshold: threshold value
 *  @ind: coring threshold
 **/
typedef struct _bf_filter_coring_cfg_t {
  int32_t  threshold;
  uint32_t ind[MAX_BF_FILTER_CORING_INDEX];
  uint32_t gain;
} bf_filter_coring_cfg_t;

/** bf_filter_cfg_t:
 *
 *  @is_valid: flag to indicate whether bf_filter_cfg_t
 *           params are valid
 *  @h_scale_en: flag to indicate whether this filter gets
 *             scaled input
 *  @bf_fir_filter_cfg: FIR filter config
 *  @bf_iir_filter_cfg: IIR filter config
 *  @shift_bits; shift bits to configure shifter
 *  @bf_filter_coring_cfg: coring filter config
 *  @gain: filter gain
 *   */
typedef struct _bf_filter_cfg_t {
  boolean                is_valid;
  boolean                h_scale_en;
  bf_fir_filter_cfg_t    bf_fir_filter_cfg;
  bf_iir_filter_cfg_t    bf_iir_filter_cfg;
  int32_t                shift_bits;
  bf_filter_coring_cfg_t bf_filter_coring_cfg;
} bf_filter_cfg_t;

typedef enum _bf_fw_region_type_t {
  BF_FW_PRIMARY_REGION,
  BF_FW_SECONDARY_REGION,
  BF_FW_MAX_REGION,
} bf_fw_region_type_t;

/** bf_fw_roi_dim_t:
 *
 *  @region: primary / secondary
 *  @x: x offset
 *  @y: y offset
 *  @w: width
 *  @h: height
 *  @is_valid: flag to indicate whether this roi is valid
 **/
typedef struct _bf_fw_roi_dim_t {
  bf_fw_region_type_t     region;
  uint32_t                x;
  uint32_t                y;
  uint32_t                w;
  uint32_t                h;
  uint32_t                region_num;
  boolean                 is_valid;
} bf_fw_roi_dim_t;

/** bf_fw_roi_type_t:
 *
 * DEFAULT ROI can include any pattern for which the hardware
 * limitations are observed.
 * CUSTOM_ROI may not observe the hardware limitations
 *
 **/
typedef enum _bf_fw_roi_type_t {
  BF_FW_DEFAULT_ROI,
  BF_FW_CUSTOM_ROI,
  BF_FW_MAX_ROI,
} bf_fw_roi_type_t;

/** bf_fw_roi_cfg_t:
 *
 *  @is_valid: flag to indicate whether bf_fw_roi_cfg_t params
 *           are valid
 *  @bf_fw_roi_dim: roi dim
 *  @num_bf_fw_roi_dim: number of roi dim configuration
 *  @bf_fw_roi_type: default or custom roi type
 *  @last_primary_region: index in the bf_fw_roi_dim[] where
 *  primary regions end.
 **/
typedef struct _bf_fw_roi_cfg_t {
  boolean                  is_valid;
  bf_fw_roi_dim_t          bf_fw_roi_dim[BF_FW_MAX_ROI_REGIONS];
  uint32_t                 num_bf_fw_roi_dim;
  bf_fw_roi_type_t         bf_fw_roi_type;
  uint32_t                 last_primary_region;
  uint32_t                 max_primary_abs_height;
  uint32_t                 max_secondary_abs_height;
} bf_fw_roi_cfg_t;

typedef struct _bf_fw_config_t {
  bf_input_cfg_t      bf_input_cfg;
  bf_gamma_lut_cfg_t  bf_gamma_lut_cfg;
  bf_scale_cfg_t      bf_scale_cfg;
  bf_filter_cfg_t     bf_filter_cfg[BF_FILTER_TYPE_MAX];
  bf_fw_roi_cfg_t     bf_fw_roi_cfg;
} bf_fw_config_t;

typedef struct _ihist_config_t {
  unsigned int is_valid;
  cam_rect_t   roi;
} ihist_config_t;

/**af_config_t: Information required to configure AF stats.
 *
 * @stream_id: stream id
 *
 * @config_id: this particular configuration request id.
 *
 * @mask: mask to indicate which kernel to configure (primary,
 *   secondary or both)
 *
 * @bf: primary kernel information
 *
 * @bf_scale: bf_scale configuration data
 *
 * @grid_info: vertical and horizontal grids (common to both)
 *
 * @bf_fw: bf config for isp ver 47 and later
 *
 **/
typedef struct {
  unsigned int stream_id;
  unsigned int config_id;
  af_config_mask_type mask;
  bf_config_t bf;
  bf_scale_config_t bf_scale;
  bf_common_t common;
  bf_fw_config_t bf_fw;
  ihist_config_t  ihist_config;
} af_config_t;

/** isp_streaming_type_t
 * ISP online/offline streaming info
 *
 *
 **/
typedef enum _isp_streaming_type_t {
  ISP_STREAMING_ONLINE,
  ISP_STREAMING_OFFLINE,
  ISP_STREAMING_MAX
} isp_streaming_type_t;

/* rs_config_t:
*
* @max_algo_support_h_rgn: Max h regions AFD algo can handle
*
**/
typedef struct {
  uint32_t max_algo_support_h_rgn;
  uint32_t max_algo_support_v_rgn;
}rs_config_t;

typedef struct _mct_event_stats_isp {
  struct timeval timestamp;
  uint32_t frame_id;
  uint32_t stats_mask;
  mct_event_stats_isp_data_t stats_data[MSM_ISP_STATS_MAX];
  isp_streaming_type_t isp_streaming_type;
  boolean ack_flag;
  struct timeval sof_timestamp;
} mct_event_stats_isp_t;

/** mct_event_stats_type
 *
 *
 *
 * 3rd level event type with type of structure mct_event_stats_t
 * Event type used module stats module and sub-modules
 **/
typedef enum _mct_event_stats_type {
  MCT_EVENT_STATS_ISP_STATS_CFG,     /* config stats */
  MCT_EVENT_STATS_ISP_STATS_ENABLE,  /* enable stats */
  MCT_EVENT_STATS_ISP_STATS,         /* isp parsed stats */
  MCT_EVENT_STATS_STATS_MAX
} mct_event_stats_type;

typedef struct _mct_event_stats {
  mct_event_stats_type type;
  struct {
    mct_event_stats_isp_t   isp_stats; /*from isp to 3a*/
    stats_start_t           start; /*3a to isp cmd*/
    stats_update_t          stats_update; /*3a to isp update*/
  } u;
} mct_event_stats_t;

/** isp_stats_tap_loc:
 *  Possible stats tap locations in VFE
 *
 *  @ISP_STATS_TAP_DEFAULT:
 *               default, used for fixed tap location
 *  @ISP_STATS_TAP_BEFORE_LENS_ROLLOFF:
 *               stats is from tap location before lens roll-off
 *  @ISP_STATS_TAP_AFTER_LENS_ROLLOFF:
 *               stats is from tap location after lens roll-off
 **/
typedef enum {
  ISP_STATS_TAP_DEFAULT             = (0),     /* default tap location*/
  ISP_STATS_TAP_BEFORE_LENS_ROLLOFF = (1<<0),  /* tap before lens roll-off */
  ISP_STATS_TAP_AFTER_LENS_ROLLOFF  = (1<<1)   /* tap after lens roll-off */
} isp_stats_tap_loc;

/** isp_rgn_skip_pattern:
 *  Region skip pattern index number
 *  Each number corresponding to a specific pattern
 **/
typedef enum {
  RGN_SKIP_PATTERN_0,   /* 0xFFFF */
  RGN_SKIP_PATTERN_1,   /* 0xA5A5 */
  RGN_SKIP_PATTERN_2,   /* 0xAAAA */
  RGN_SKIP_PATTERN_3,   /* 0xA5A1 */
  RGN_SKIP_PATTERN_4,   /* 0xA149 */
  RGN_SKIP_PATTERN_5,   /* 0x41A1 */
  RGN_SKIP_PATTERN_6,   /* 0x1A0A */
  RGN_SKIP_PATTERN_7,   /* 0x8421 */
  RGN_SKIP_PATTERN_8,   /* 0xA0A0 */
  RGN_SKIP_PATTERN_9,   /* 0x8204 */
  RGN_SKIP_PATTERN_10,  /* 0x1820 */
  RGN_SKIP_PATTERN_11,  /* 0x8020 */
  RGN_SKIP_PATTERN_12,  /* 0x4020 */
  RGN_SKIP_PATTERN_13,  /* 0x8000 */
  RGN_SKIP_PATTERN_MAX  /* invalid pattern, same as disable region skip */
} isp_rgn_skip_pattern;

/** mct_event_request_stats_type:
 *
 *  @supported_stats_mask: stats supported by ISP, assume BG and
 *                       BE are enabled, this mask will contain
 *                       ((1 << MSM_ISP_STATS_BG) | (1 <<
 *                       MSM_ISP_STATS_BE))
 *  @enable_stats_mask: stats to be enabled by 3A
 *  @enable_stats_parse_mask: 3A does disable/enable stats parsing by ISP
 *                       assume BG & BE stats parsing needed, then mask
 *                       will be ((1 << MSM_ISP_STATS_BG) | (1 <<
 *                       MSM_ISP_STATS_BE))
 *  @num_isp_stat_comp_grp: No. of stats group allowed in ISP.
 *  @stats_comp_grp_mask: stats group selection for given stats bit
 *                        location by 3A, assume you need to ihist to be
 *                        part of STATS_COMPOSITE_GRP2 then we need set
 *                        as below:
 *                        stats_comp_grp_mask[1]= (1 << MSM_ISP_STATS_IHIST)
 *  @isp_streaming_type: isp online/offline streaming info
 *  @supported_tap_location: available tap locations for each stat from ISP
 *  @requested_tap_location: requested tap location from 3A for each stats
 *  @supported_rgn_skip_mask: stats which support region skip from ISP,
 *                       assume BG and AEC_BG stats support region skip,
 *                       this mask will contain
 *                       ((1 << MSM_ISP_STATS_BG) | (1 <<
  *                       MSM_ISP_STATS_AEC_BG))
 *  @enable_rgn_skip_pattern: region skip pattern enabled by 3A
 **/
typedef struct _mct_event_request_stats_type_t {
  isp_streaming_type_t isp_streaming_type;
  uint32_t             supported_stats_mask;
  uint32_t             enable_stats_mask;
  uint32_t             enable_stats_parse_mask;
  uint32_t             num_isp_stat_comp_grp;
  uint32_t             stats_comp_grp_mask[2]; /*ISP STATS COMPOSITE GROUPS*/
  isp_stats_tap_loc    supported_tap_location[MSM_ISP_STATS_MAX];
  isp_stats_tap_loc    requested_tap_location[MSM_ISP_STATS_MAX];
  uint32_t             supported_rgn_skip_mask;
  isp_rgn_skip_pattern enable_rgn_skip_pattern[MSM_ISP_STATS_MAX];
} mct_event_request_stats_type;

/** mct_aec_video_hdr_input_t
 *   @number_of_frames: number of frames
 *   @frame_exp_compensation: exposure compensation for each frame,
 *     where 1.0f is normal exposure
 *
 *  Required exposure input video HDR algorithm
 *
 **/
typedef struct {
  uint32_t number_of_frames;
  float frame_exp_compensation[MAX_VIDEO_HDR_FRAMES];
} mct_aec_video_hdr_input_t;

/** gyro_samples_t
 * @value: samples gyro sample (x,y,z), rad/sec, Q16
 * @timestamp: realtime [usec]
 * @timestamp_ns: realtime [nano sec]
 *
 * Base structure for gyro samples
 **/
typedef struct _gyro_samples{
  float value[3];
  uint64_t timestamp;
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  uint64_t timestamp_ns;
#endif
} gyro_samples_t;

/** accel_samples_t
 * @value: samples accel sample (x,y,z), meter/sec^2
 * @timestamp: realtime [usec]
 * @timestamp_ns: realtime [nano sec]
 *
 * Base structure for accel samples
 **/
typedef struct _accel_samples{
  float value[3];
  uint64_t timestamp;
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  uint64_t timestamp_ns;
#endif
} accel_samples_t;

typedef struct {
  uint8_t seqnum;
  uint32_t sample_len;  /**< Must be set to # of elements in sample */
  accel_samples_t sample[STATS_GYRO_MAX_SAMPLE_BUFFER_SIZE];
} sensor_accel_buffer_data_type;

/** mct_event_gyro_data_t
 *    @frame_id:      the id of frame
 *    @ready:         indicate whether the data is ready
 *    @sample_len:    indicate the number of samples in @sample
 *    @sof:           the timestamp of SOF
 *    @frame_time:    the duration of frame
 *    @exposure_time: the expoure value
 *    @sof_timestamp_ns: sof timestamp nanosec
 *    @sample:        the array of sample
 *
 *  This structure is used to store gyro data
 *
 **/
typedef struct _mct_event_gyro_data {
  uint32_t frame_id;
  int      ready;
  uint32_t sample_len;
  uint64_t sof_timestamp;
  uint64_t frame_time;
  float    exposure_time;
  uint64_t sof_timestamp_ns;
  gyro_samples_t sample[STATS_GYRO_MAX_SAMPLE_BUFFER_SIZE];
} mct_event_gyro_data_t;

/** mct_event_gyro_stats_t
 *    @q16_angle:    the angle for x, y, z
 *    @ts:           the timestamp
 *    @is_gyro_data  gyro data
 *
 *  This structure is used to store gyro data
 *
 **/
typedef struct _mct_event_gyro_stats {
  float q16_angle[3];
  uint64_t ts;
  mct_event_gyro_data_t is_gyro_data;
} mct_event_gyro_stats_t;

/** mct_event_accel_data_t
 *    @frame_id:      the id of frame
 *    @ready:         indicate whether the data is ready
 *    @sample_len:    indicate the number of samples in @sample
 *    @sof:           the timestamp of SOF
 *    @frame_time:    the duration of frame
 *    @exposure_time: the expoure value
 *    @sof_timestamp_ns: sof timestamp nanosec
 *    @sample:        the array of sample
 *
 *  This structure is used to store accel data
 *
 **/
typedef struct _mct_event_accel_data {
  uint32_t frame_id;
  int      ready;
  uint32_t sample_len;
  uint64_t sof_timestamp;
  uint64_t frame_time;
  float    exposure_time;
  uint64_t sof_timestamp_ns;
  accel_samples_t sample[STATS_GYRO_MAX_SAMPLE_BUFFER_SIZE];
} mct_event_accel_data_t;

/** mct_event_accel_stats_t
 *    @q16_angle:    the angle for x, y, z
 *    @ts:           the timestamp
 *    @is_accel_data accel data
 *
 *  This structure is used to store accel stats
 *
 **/
typedef struct _mct_event_accel_stats {
  mct_event_accel_data_t is_accel_data;
} mct_event_accel_stats_t;

#endif /* __MCT_EVENT_STATS_H___*/
