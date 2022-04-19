/* af.h
 *
 * Copyright (c) 2016-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AF_H__
#define __AF_H__

#include "modules.h"
#include "q3a_stats.h"
#include "chromatix_common.h"
#include "mct_event_stats.h"
#include "depth_service.h"
#include "q3a_common_types.h"
#include "af_core.h"
#include "stats_util.h"
#include "math.h"

/* TBD: Number of grids currently supported. For bayer max is 14x18.
   YUV it's 9x9.*/
#define USE_MULTI_WINDOW                  1
#define NUM_AUTOFOCUS_HORIZONTAL_GRID     5
#define NUM_AUTOFOCUS_VERTICAL_GRID       5
#define NUM_AUTOFOCUS_MULTI_WINDOW_GRIDS  20

#define STATS_GYRO_MAX_SAMPLE_BUFFER_SIZE (32)
/* Additional Tuning Param */

// some customer wants to enable it.
#define AF_CAF_TRIGGER_AFTER_TAF 0
#define AF_SENSITIVITY .52
#define AF_ACTUATOR_SHIFT_COMP 0
#define AF_HYSTERESIS_COMP 0
#define AF_LASER_SAD_ENABLE 1
#define AF_FRAMES_TO_WAIT 3
#define AF_LASER_SAD_GAIN_MAX 30.0
#define AF_LASER_SAD_GAIN_MIN 2.0
#define AF_LASER_SAD_REF_GAIN_MAX 30.0f
#define AF_LASER_SAD_REF_GAIN_MIN 2.0f
#define AF_LASER_SAD_THRESHOLD_MAX 50
#define AF_LASER_SAD_THRESHOLD_MIN 40
#define AF_LASER_SAD_REF_THRESHOLD_MAX 25
#define AF_LASER_SAD_REF_THRESHOLD_MIN 20
#define AF_FAR_CONVERGE_POINT 148
#define AF_FAR_DISTANCE_FINE_STEP_SIZE 4
#define AF_FAR_DISTANCE_UNSTABLE_CNT_FACTOR 2
#define AF_JUMP_TO_START_LIMIT_LOW_LIGHT 50
#define AF_TOF_STEP_SIZE_LOW_LIGHT 8
#define AF_SCENE_CHANGE_DISTANCE_STD_DEV_THRES 30
#define AF_NUM_NEAR_STEPS_NEAR_DIRECTION 3
#define AF_NUM_FAR_STEPS_NEAR_DIRECTION 1
#define AF_NUM_NEAR_STEPS_FAR_DIRECTION 0
#define AF_NUM_FAR_STEPS_FAR_DIRECTION 3
#define AF_START_POS_FOR_TOF 80

#define USEC_PER_SEC 1000000

int                   num_near_steps_near_direction;
int                   num_far_steps_near_direction;
int                   num_near_steps_far_direction;
int                   num_far_steps_far_direction;
int                   start_pos_for_tof;


/** stats_disable_mask_type: Mask that indicates which HAF module is enabled or disabled.
**/
typedef enum {
  DISABLE_HAF_STATS_NONE = 0,
  DISABLE_HAF_STATS_PDAF_MASK = 1 << 0,
  DISABLE_HAF_STATS_TOF_MASK = 1 << 1,
  DISABLE_HAF_STATS_DCIAF_MASK = 1 << 2,

}stats_disable_mask_type;


typedef enum{
  AF_SCENE_CHANGE_DISTANCE_THRESHOLD_ZERO  = 20,
  AF_SCENE_CHANGE_DISTANCE_THRESHOLD_ONE   = 40,
  AF_SCENE_CHANGE_DISTANCE_THRESHOLD_TWO   = 60,
  AF_SCENE_CHANGE_DISTANCE_THRESHOLD_THREE = 80,
  AF_SCENE_CHANGE_DISTANCE_THRESHOLD_FOUR = 100,
}af_scene_change_distance_threshold_enum;

typedef enum{
  AF_DISTANCE_REGION_ZERO  = 70,
  AF_DISTANCE_REGION_ONE   = 200,
  AF_DISTANCE_REGION_TWO   = 350,
  AF_DISTANCE_REGION_THREE = 500,
  AF_DISTANCE_REGION_FOUR = 765,
}af_distance_region_enum;

typedef struct _distance_entry {
  uint32_t distance;
  single_index_t index_enum;
} distance_entry;

extern distance_entry distanceMap[SINGLE_MAX_IDX];


/** af_move_direction_type:
 *
 * Direction to move
 *
 **/
typedef enum {
  AF_MOVE_NEAR, /* Move towards MACRO position */
  AF_MOVE_FAR,  /* Move towards INFY position */
} af_move_direction_type;

/** af_metering_type:
 *
 * List of AF metering types supported.
 *
 **/
typedef enum {
  AF_METER_AUTO = 0x0,
  AF_METER_SPOT,
  AF_METER_CTR_WEIGHTED,
  AF_METER_AVERAGE,
} af_metering_type;

/** focus_distance_index_type:
 *
 * Index of near/optimal/far focus distance in the focus_distance array.
 *
 **/
typedef enum {
  FOCUS_DISTANCE_NEAR_INDEX,
  FOCUS_DISTANCE_OPTIMAL_INDEX,
  FOCUS_DISTANCE_FAR_INDEX,
  FOCUS_DISTANCE_MAX_INDEX
} af_fd_index_type;

/** _focus_distances:
 *    @focus_distance: array of focus distances
 *
 * near/optimal/far focus distances
 **/
typedef struct _focus_distances {
  float focus_distance[FOCUS_DISTANCE_MAX_INDEX];
} af_focus_distances_t;


/** af_done_type:
 *
 * Type to indicate AF done event to send.
 *
 **/
typedef enum {
  AF_FAILED  = 0, /* AF is failed or rejected */
  AF_SUCCESS = 1, /* AF is sucessful */
  AF_ABORT   = 2, /* AF is aborted */
} af_done_type;

/** _af_done:
 *    @focus_done: boolean indicating if focusing is complete
 *    @status:     result of auto-focus - success/failure/aborted
 *
 * Result of focus done.
 **/
typedef struct _af_done {
  boolean      focus_done;
  af_done_type status;
} af_done_t;

/** af_status_type:
 *
 * Autofocus Status
 *
 **/
typedef enum {
  AF_STATUS_INVALID = -1,
  AF_STATUS_INIT = 0,
  AF_STATUS_FOCUSED,
  AF_STATUS_UNKNOWN,
  AF_STATUS_FOCUSING,
  AF_STATUS_CUSTOM,
} af_status_type;

/** _af_status:
 *    @focus_done: boolean indicating if focusing is complete
 *    @status:     result of auto-focus - focused/unknown/focusing
 *    @f_distance: calculated focus distance
 *
 * AF status
 **/
typedef struct _af_status {
  boolean              focus_done;
  int                  status;
  af_focus_distances_t f_distance;
} af_status_t;

/** _af_move_lens:
 *    @move_lens:    flag to check if we should move lens
 *    @direction:    which direction to move
 *    @num_of_steps: number of steps to move
 *    @cur_pos:      current lens position
 *    @use_dac_value: if true lens move request is in terms of
 *                    DAC
 *    @pos:          lens position in DAC(can be divided into
 *                   multiple steps)
 *    @delay:        delay between each steps
 *    num_of_interval: number of steps divided
 *    fovc_enabled:  if true fov compensation is enabled
 *    mag_factor:    magnificaiton factor based on lens position
 * Media Controller process return type
 **/
typedef struct _af_move_lens {
  boolean move_lens;
  boolean reset_lens;
  int     direction;
  int     num_of_steps;
  int32_t cur_pos;
  boolean use_dac_value;
  unsigned short pos[MAX_NUMBER_OF_STEPS];
  unsigned short delay[MAX_NUMBER_OF_STEPS];
  unsigned short num_of_interval;
  boolean fovc_enabled;
  float mag_factor;
} af_move_lens_t;

/** _af_preview_size:
 *    @width:  width of the frame
 *    @height: height of the frame
 *
 * Preview size received from upper layer.
 **/
typedef struct _af_preview_size {
  int width;
  int height;
} af_preview_size_t;

/** af_roi_type:
 *
 * Enum to indicate what type of ROI information we have received.
 *
 **/
typedef enum {
  AF_ROI_TYPE_GENERAL = 0x0, /* Default */
  AF_ROI_TYPE_FACE,          /* Face priority AF */
  AF_ROI_TYPE_TOUCH,         /* Touch-AF */
} af_roi_type;

/** _af_roi:
 *    @x:   horizon tal offset of the region
 *    @y:   vertical offset of the region
 *    @dx:  width of the RoI region
 *    @dy:  height of the region
 *    @roi: structure holding dimensions of each ROI
 *
 * RoI information in terms of frame
 **/
typedef struct _af_roi {
  uint16_t x;
  uint16_t y;
  uint16_t dx;
  uint16_t dy;
} af_roi_t;


/** _af_roi_info:
 *    @roi_updated: check if ROI information has been updated
 *    @type:        ROI type - General/Touch/Face
 *    @frm_id:      frame ID
 *    @num_roi:     Number of ROIs detected
 *    @roi:         structure holding dimensions of each ROI
 *    @weight:      array holding the weights
 *    @center_x: x center of touch AF
 *    @center_y: y center of touch AF
 *
 * ROI selected for AF stats.
 **/
typedef struct _af_roi_info {
  boolean     roi_updated;
  af_roi_type type;
  uint32_t    frm_id;
  uint32_t    num_roi;
  af_roi_t    roi[MAX_STATS_ROI_NUM];
  uint32_t    weight[MAX_STATS_ROI_NUM];
  uint16_t    center_x;
  uint16_t    center_y;
  float       roi_moving_rate_x;
  float       roi_moving_rate_y;
} af_roi_info_t;

/** _af_crop_info: RoI information about scalar crop
*
*    @x: horizontal offset of the region
*
*    @y: vertical offset of the region
*
*    @dx: width of the crop region
*
*    @dy: height of the region
*
**/
typedef struct _af_crop_info{
  uint16_t x;
  uint16_t y;
  uint16_t dx;
  uint16_t dy;
  float zoom_ratio;
} af_crop_info_t;

/** _af_input_from_awb:
 *    @custom_param_af: OEM custom parameters
 *
 * Data needed from AWB module for AF operation.
 **/
typedef struct _af_input_from_awb {
  /* custom opaque parameters */
  q3a_custom_data_t     custom_param_af;
} af_input_from_awb_t;

/** _af_input_from_aec:
 *    @aec_settled:       flag to check if aec is settled now.
 *    @exp_index:         exposure index
 *    @pixels_per_region: TODO
 *    @comp_luma:         TODO
 *    @cur_luma:          current luma value
 *    @cur_real_gain:     TODO
 *    @lux_idx:           TODO
 *    @num_regions:       TODO
 *    @exp_tbl_val:       TODO
 *    @luma_settled_cnt:  TODO
 *    @SY:                TODO
 *
 * Data needed from AEC module for AF operation.
 **/
typedef struct _af_input_from_aec {
  int           aec_settled;
  int           exp_index;
  int           pixels_per_region;
  float         comp_luma;
  float         cur_luma;
  float         cur_real_gain;
  float         lux_idx;
  float         exp_time;
  int           preview_fps;
  int           preview_linesPerFrame;
  int           linecnt;
  float         target_luma;
  unsigned int  num_regions;
  unsigned int  exp_tbl_val;
  unsigned int  luma_settled_cnt;
  q3a_SY_data_t SY_data;
  float         Av_af;
  float         Tv_af;
  float         Sv_af;
  float         Bv_af;
  float         Ev_af;
  float         total_drc_gain;
  /* custom opaque parameters */
  q3a_custom_data_t custom_param_af;
} af_input_from_aec_t;

/** _af_input_from_isp:
 *    @width:  TODO
 *    @height: TODO
 *
 * Data needed from isp module for AF operation.
 **/
typedef struct _af_input_from_isp {
  unsigned int width;
  unsigned int height;
} af_input_from_isp_t;

typedef enum {
  AF_MANUAL_FOCUS_MODE_DIOPTER = 1,
  AF_MANUAL_FOCUS_MODE_POS_RATIO,
  AF_MANUAL_FOCUS_MODE_MAX
} af_manual_focus_mode_type;

typedef struct _af_input_manual_focus {
  af_manual_focus_mode_type flag;
  boolean manual_af_state;
  union {
  float af_manual_diopter;
  int af_manual_lens_position_ratio;
  } u;
} af_input_manual_focus_t;

/** _af_actuator_info:
 *    @focal_length:    focal length of lens (mm)
 *    @af_f_num:        numeric aperture of lens (mm)
 *    @af_f_pix:        pixel size (microns)
 *    @af_total_f_dist: Lens distance (microns)
 *    @hor_view_angle:  Sensor's horizontal view angle
 *    @ver_view_angle:  Sensor's vertical view angle
 *
 * information from actuator required for AF.
 **/
typedef struct _af_actuator_info{
  float focal_length;
  float af_f_num;
  float af_f_pix;
  float af_total_f_dist;
  float hor_view_angle;
  float ver_view_angle;
  float min_focus_distance;
} af_actuator_info_t;

/** _af_input_from_sensor:
 *    @af_not_supported: Flag indicating if the sensor supports focus or not
 *    @preview_fps:      current preview fps
 *    @max_preview_fps:  maximum preview fps
 *    @actuator_info:    AF specific data from actuator
 *    @actuator_sensitivity : Current actuator sensitivity
 *    @sensor_type: Sensor type. Usually default is Bayer
 *    @lens_type: Lens type. Usually default wide sensor
 *
 * Data needed from sensor module for AF operation.
 **/
typedef struct _af_input_from_sensor {
  boolean            af_not_supported;
  int                preview_fps;
  int                max_preview_fps;
  uint32_t           frame_time;
  af_actuator_info_t actuator_info;
  uint32_t           sensor_res_height;
  uint32_t           sensor_res_width;
  float              actuator_sensitivity;
  q3a_sensor_type_t  sensor_type;
  q3a_lens_type_t    lens_type;
} af_input_from_sensor_t;

/** _af_input_from_gravity_vector:
 *    @is_ready: boolean to indicate data is available
 *    @g_vector: gravity vector (x/y/z axes)
 *
 * Gravity estimate vector from motion sensor module.
**/
typedef struct _af_input_from_gravity_vector {
  boolean is_ready;
  float g_vector[3];
  float lin_accel[3];
  uint8_t accuracy;
} af_input_from_gravity_vector_t;

/** _af_input_from_gyro:
 *    @float_ready: float data is available
 *    @flt:         gyro metrics in float
 *    @q16_ready:   gyro metrics in q16 ready to use
 *    @q16:         gyro metrics in q16
 *
 * Gyro data required to assist AF.
**/
typedef struct _af_input_from_gyro {
  int   float_ready;
  float flt[3];
  int   q16_ready;
  long  q16[3];
  int sample_len;
  gyro_samples_t sample[STATS_GYRO_MAX_SAMPLE_BUFFER_SIZE];
} af_input_from_gyro_t;


/** _af_stats_caps: Enum to describe current AF capabilities
 *  in terms of stats supported.
 **/
typedef enum _af_stats_caps {
  AF_STATS_CAPS_DEFAULT = (1 << 0),
  AF_STATS_CAPS_SCALE = (1 << 1),
  AF_STATS_CAPS_SW = (1 << 2),
  AF_STATS_CAPS_FW = (1 << 3)
} af_stats_caps_type;

/** _af_stats_kernel_size: Enum to indicate AF kernel
 *  coefficient sizes.
 **/
typedef enum _af_stats_kernel_size {
  AF_STATS_HPF_LEGACY,
  AF_STATS_HPF_2x5,
  AF_STATS_HPF_2x11,
} af_stats_kernel_size_type;

/** af_config_kernel_primary:
 *    @region: roi dimensions
 *    @hpf: high pass filter coefficients
 *
 *  Primary AF kernel configuration info.
 **/
typedef struct _af_config_kernel_primary {
  af_roi_t region;
  int hpf[MAX_HPF_BUFF_SIZE];
  unsigned int r_min;
  unsigned int b_min;
  unsigned int gr_min;
  unsigned int gb_min;
} af_config_kernel_primary_t;

/** _af_config_kernel_scale:
 *   @region: roi dimensions
 *   @hpf: high pass filter coefficients
 *   @scale: down scaling factor
 *
 *  BF Scale AF kernel configuration info.
 *  */
typedef struct _af_config_kernel_scale {
  af_roi_t region;
  int hpf[MAX_HPF_BUFF_SIZE];
  int scale;
  unsigned int r_min;
  unsigned int b_min;
  unsigned int gr_min;
  unsigned int gb_min;
} af_config_kernel_scale_t;

/** _af_config_kernel_type: Enum to indicate which AF kernel
 *  to configure*/
typedef enum _af_config_kernel {
  AF_CONFIG_KERNEL_PRIMARY = (1 << 0),
  AF_CONFIG_KERNEL_SCALE = (1 << 1)
} af_config_kernel_type;

/** swaf_config_t
 *   @enable:    Flag to enable/disable stats collection
 *   @frame_id:  current frame id
 *   @roi:       ROI of the stats collection
 *   @coeffa:    filterA coefficients
 *   @coeffb:    filterB coefficients
 *   @coeff_len: length of coefficient table
 *   @sw_filter_type: filter type
 *
 *   Imglib preview assisted AF coefficients
 **/
typedef struct {
  int8_t     enable;
  int        frame_id;
  af_roi_t   roi;
  double     coeffa[MAX_SWAF_COEFFA_NUM];
  double     coeffb[MAX_SWAF_COEFFB_NUM];
  uint32_t   coeff_len;
  int        coeff_fir[MAX_SWAF_COEFFFIR_NUM];
  double     fv_min;
  af_sw_filter_type sw_filter_type;
} swaf_config_data_t;

/** af_imglib_output_t
 *   @frame_id: Frame ID out the AF output
 *   @fV: Focus value
 *   @pending: Flag to indicate the fv value has been query but
 *           missing.
 *
 *   Imglib preview assisted AF output
 **/
typedef struct {
  int     frame_id;
  double  fV;
  uint8_t pending;
} af_sw_stats_t;

/** _af_debug_info_t
 *   @enable: Flag to mark debug info is valid.
 *   @af_debug_size: AF debug data size.
 *
 *   AF debug data
 **/
typedef struct _af_debug_info_t {
  boolean enable;
  char debug_data[AF_DEBUG_DATA_SIZE];
} af_debug_info_t;


/** awb_bestshot_mode_type_t:
 *  List of scene modes supported.
 **/
typedef enum {
  AF_BESTSHOT_OFF,
  AF_BESTSHOT_AUTO,
  AF_BESTSHOT_LANDSCAPE,
  AF_BESTSHOT_SNOW,
  AF_BESTSHOT_BEACH,
  AF_BESTSHOT_SUNSET,
  AF_BESTSHOT_NIGHT,
  AF_BESTSHOT_PORTRAIT,
  AF_BESTSHOT_BACKLIGHT,
  AF_BESTSHOT_SPORTS,
  AF_BESTSHOT_ANTISHAKE, /* 10 */
  AF_BESTSHOT_FLOWERS,
  AF_BESTSHOT_CANDLELIGHT,
  AF_BESTSHOT_FIREWORKS,
  AF_BESTSHOT_PARTY,
  AF_BESTSHOT_NIGHT_PORTRAIT,
  AF_BESTSHOT_THEATRE,
  AF_BESTSHOT_ACTION,
  AF_BESTSHOT_AR,
  AF_BESTSHOT_FACE_PRIORITY,
  AF_BESTSHOT_BARCODE, /* 20 */
  AF_BESTSHOT_HDR,
  AF_BESTSHOT_MAX
} af_bestshot_mode_type_t;

/** _af_bestshot_data:
 *    @enable:    TODO
 *    @srch_mode: focus mode to be used
 *    @curr_mode: bestshot mode currently selected
 *
 * AF related informated for bestshot mode selected.
**/
typedef struct _af_bestshot_data {
  boolean               enable;
  uint32_t              srch_mode;
  uint32_t              curr_mode;
} af_bestshot_data_t;

/** _af_stream_crop:
 *    @vfe_map_x:      left
 *    @vfe_map_y:      top
 *    @vfe_map_width:  width
 *    @vfe_map_height: height
 *    @pp_x:           left
 *    @pp_y:           top
 *    @pp_crop_out_x:  width
 *    @pp_crop_out_y:  height
 *
 * TODO
 **/
typedef struct _af_stream_crop {
  uint32_t vfe_map_x;
  uint32_t vfe_map_y;
  uint32_t vfe_map_width;
  uint32_t vfe_map_height;
  uint32_t pp_x;
  uint32_t pp_y;
  uint32_t pp_crop_out_x;
  uint32_t pp_crop_out_y;
  uint32_t vfe_out_width;
  uint32_t vfe_out_height;
} af_stream_crop_t;

/** _af_move_lens_cb:
 *    @object_id:    pointer to the AF port
 *    @move_lens_cb: function pointer to the callback function in AF port
 *
 * This structure is used to pass the move lens callback function information
 * to the library.
 **/
typedef struct _af_move_lens_cb {
  void    *object_id;
  boolean (*move_lens_cb)(void *output, void *object_id);
} af_move_lens_cb_t;

/** _af_move_lens_cb:
 *    @af_trigger:    Trigger type to report back to HAL
 *    @af_trigger_id: Trigger ID to report back to HAL
 *
 * This structure is used to pass trigger information to af port
 **/
typedef struct _af_trigger_t {
 uint8_t                   af_trigger;
 int                       af_trigger_id;
} af_trigger_t;

typedef q3a_operation_mode_t af_operation_mode_t;

/** _af_set_parameter_init:
 *    @chromatix:      pointer to chromatix
 *    @comm_chromatix: pointer to common chromatix parameters
 *    @tuning_info:    af tuning header pointer
 *    @op_mode:        camera operation mode - video/camera/snapshot
 *                     it can be derived from stream info
 *    @preview_size:   current preview size
 *    @af_tuning_params: additional tuning param
 * Parameters that need to be set during initialization.
 **/
typedef struct _af_set_parameter_init{
  void                        *tuning_info;
  af_run_mode_type             op_mode;
  af_preview_size_t            preview_size;
  af_core_tuning_params_t      core_tuning_params;
} af_set_parameter_init_t;


/** af_pdaf_buf_type:
 * Indicates what kind of Pd buffer type will be sent to PDLib
**/
typedef enum {
  AF_PD_EARLY_BAF_BUF = 0,        // eraly interrupt buffer
  AF_PD_PREV_IDIVERT_BUF,   // previous frame buffer
  AF_PD_IDIVERT_BUF               // end of frame buffer
} af_pdaf_buf_type;

/** _af_pdaf_early_interrupt_data_t:
*    @buf_type: It indicate which buffer type will be sent to PDLib.
*    @frame_id: pcurrent frame id
* Parameters that need to be set during initialization.
**/
typedef struct _af_pdaf_early_interrupt_data_t {
  af_pdaf_buf_type buf_type;
  uint32_t frame_id;
} af_pdaf_early_interrupt_data_t;

typedef struct _af_pdaf_data_t {
  void    *object_id;
  void    *stats_buff;
  boolean (*pdaf_proc_cb)(void *pdaf_data, void *output);
  af_pdaf_early_interrupt_data_t pdaf_early_interrupt_data;
} af_pdaf_data_t;

typedef struct _af_bg_stats_data_t {
  mct_port_t *af_port;
  stats_af_t dummy_af_stats;
  q3a_bg_stats_t* p_q3a_bg_stats;
  stats_buffer_done_callback_func af_stats_cb;
} af_bg_stats_data_t;

/** af_set_parameter_type:
 * List of AF parameters that can be used by other componenets to set.
 **/
typedef enum {
  AF_SET_PARAM_INIT_CHROMATIX_PTR = 1, /* Initialize chromatix */
  AF_SET_PARAM_UPDATE_TUNING_HDR,      /* Update AF tuning header */
  AF_SET_PARAM_INIT,                   /* Init AF data */
  AF_SET_PARAM_RESET_LENS,             /* Reset lens */
  AF_SET_PARAM_METERING_MODE,          /* Metering mode */
                                       /* - Auto/Spot/Average/Center Weighted */
  AF_SET_PARAM_START,                  /* Start AF */
  AF_SET_PARAM_MOVE_LENS,              /* Move lens */
  AF_SET_PARAM_LENS_MOVE_DONE,         /* Lens move request completed */
  AF_SET_PARAM_FOCUS_MODE,             /* Change focus mode */
  AF_SET_PARAM_CANCEL_FOCUS,           /* Cancel auto focus */
  AF_SET_PARAM_IDLE_TRIGGER,           /*  Set idle trigger for HAL 3 */
  AF_SET_PARAM_STOP_FOCUS,             /* Stop auto-focus */
  AF_SET_PARAM_BESTSHOT,               /* Set required AF parameters */
                                       /* different for bestshot modes */
  AF_SET_PARAM_ROI,                    /* ROI information */
                                       /* - in case of Touch/Face */
  AF_SET_PARAM_SENSOR_ROI,             /* ROI information for HAL3 */
  AF_SET_PARAM_LOCK_CAF,               /* Request to lock CAF till unlocked */
  AF_SET_PARAM_EZ_ENABLE,              /* Enable/disable eztune */
  AF_SET_PARAM_UPDATE_AWB_INFO,        /* Update awb info */
  AF_SET_PARAM_UPDATE_AEC_INFO,        /* Update aec info */
  AF_SET_PARAM_UPDATE_SENSOR_INFO,     /* Update sensor info */
  AF_SET_PARAM_UPDATE_GYRO_INFO,       /* Update gyro data */
  AF_SET_PARAM_UPDATE_FACE_INFO,       /* Update face data */
  AF_SET_PARAM_UPDATE_ISP_INFO,        /* Update isp info */
  AF_SET_PARAM_STREAM_CROP_INFO,       /* Update stream crop info */
  AF_SET_PARAM_RUN_MODE,               /* Camera run mode
                                        * - Video/Camera/Snapshot */
  AF_SET_PARAM_MOVE_LENS_CB,           /* Set move lens callback function */
  AF_SET_PARAM_STATS_DEBUG_MASK,       /* Set the stats debug mask to algorithm lib*/
  AF_SET_PARAM_EZ_TUNE_RUNNING,        /* Set the state of the eztune */
  AF_SET_PARAM_SUPER_EVT,              /* Super evt id */
  AF_SET_PARAM_SOF,                    /* Set sof id */
  AF_SET_PARAM_META_MODE,
  AF_SET_PARAM_WAIT_FOR_AEC_EST,       /* Wait for AEC to complete estimation when LED is ON */
  AF_SET_PARAM_RESET_CAF,              /* Reset CAF to make it start from the beginning */
  AF_SET_PARAM_CROP_REGION,            /* Update Scalar crop info */
  AF_SET_PARAM_IMGLIB_OUTPUT,          /* SW stats output*/
  AF_SET_PARAM_HFR_MODE,               /* HFR mode */
  AF_SET_PARAM_SW_STATS_POINTER,       /* SW stats backup pointer*/
  AF_SET_PARAM_UPDATE_STATS_CAPS,      /* Update AF stats capabilities */
  AF_SET_PARAM_MANUAL_FOCUS_DISTANCE,  /* Set manual focus distance */
  AF_SET_PARAM_PAAF,                   /* Control PAAF from HAL */
  AF_SET_PARAM_UPDATE_GRAVITY_VECTOR,  /* update gravity vector */
  AF_SET_PARAM_LENS_POS_FOCUS,         /* Set Lens Position after comp */
  AF_SET_PARAM_RECONFIG_ISP,           /* Request AF algo to reconfigure ISP */
  AF_SET_PARAM_DEPTH_SERVICE,          /* Depth map input to AF algorithm*/
  AF_SET_PARAM_FOCUS_MANUAL_POSITION,
  AF_SET_PARAM_DEF_POS,               /* move lens to default position */
  AF_SET_PARAM_FD_ENABLED,            /* FD enabled or not */
  AF_SET_PARAM_FAST_AEC_ENABLED,      /* Inform AF that "Fast AEC" is enable */
  AF_SET_PARAM_MULTI_CAM_SYNC_TYPE,   /* In dual cam, if algo doing sync. It set the role of the algo */
  AF_SET_PARAM_LOW_POWER_MODE,        /* Algo is request to work in low-power mode (dual camera) */
  AF_SET_PARM_CUSTOM_EVT_MOD,         /* Custom event coming from Modules */
  AF_SET_PARM_CUSTOM_EVT_CTRL,        /* Custom event control */
  AF_SET_PARM_CUSTOM_EVT_HAL,         /* Custom event coming from HAL */
  AF_SET_PARAM_HAF_ENABLE,            /* Enable/Disable HAF technologies */
  AF_SET_PARAM_SET_ROLE_SWITCH,       /* Set AF Algorithm Role*/
  AF_SET_PARAM_SET_PEER_FOCUS_INFO,   /* Focus info of Peer AF Algorithm*/
  AF_SET_PARAM_SET_STEREO_CAL_DATA,   /* Stereo calibration data for dual-camera */
  AF_SET_PARAM_CONFIG_AE_SCAN_TEST,   /* Config for AE Scan test */
  AF_SET_PARAM_PDAF_DATA,             /* Pdaf stats data */
  AF_SET_PARAM_HAF_CUSTOM_PARAM,      /*Custom HAF Param */
  AF_SET_PARAM_FOV_COMP_ENABLE,       /* Enable/Disable FOV Compensation */
  AF_SET_PARAM_MW_ENABLE,              /* Enable Multiwindow feature */
  AF_SET_PARAM_BG_STATS,              /* AF BG stats */
  AF_SET_PARAM_AF_HJ_ENABLE,          /* Enable/Disable Hand Jitter AF */
  AF_SET_PARAM_SKIP_FINE_SCAN,        /* Param to skip fine scan at runtime */
  AF_SET_PARAM_AEC_MANUAL_UPDATE,     /* Update aec manual info */
  AF_SET_PARAM_MAX = 0xFF             /* TODO */
} af_set_parameter_type;


/** _af_set_parameter: Used for setting AF parameters
 *    @type:              parameter type as listed by af_set_parameter_type
 *    @current_frame_id:  SOF id wrt to set param
 *    @af_set_data:       TODO
 *    @af_init_param:     init AF
 *    @af_lens_move_done: TODO
 *    @af_run_mode:       TODO
 *    @af_metering_mode:  Metering mode -Auto/Spot/Average/CenterWeighted
 *    @af_steps_to_move:  number of steps to move lens
 *    @af_mode:           focus mode - Auto/Macro/Normal
 *    @af_cont_focus:     enable/disable continuous auto-focus
 *    @af_lock_af:       enable/disable AF lock. Till AF is locked
 *    @af_ez_disable:     enable/disable eztune logging for CAF
 *    @af_roi_info:       contents hold AF ROI information
 *    @aec_info:          TODO
 *    @sensor_info:       TODO
 *    @gyro_info:         TODO
 *    @isp_info:          TODO
 *    @af_bestshot_mode:  TODO
 *    @stream_crop:       TODO
 *    @move_lens_cb_info: TODO
 *    @paaf_mode:         used to turn ON/OFF PAAF from HAL (1 - on, 0 - off)
 *    @multi_cam_sync_type: In dual cam, if algo doing sync. It set the role of the algo
 *    @low_power_mode: In dual cam, algo is request to work in low-power mode
 *
 * Used for setting AF parameters
 **/
typedef struct _af_set_parameter {
  af_set_parameter_type type;
  uint8_t              camera_id;
  unsigned int          current_frame_id;

  union {
    boolean                   af_set_data;
    af_set_parameter_init_t   af_init_param;
    boolean                   af_lens_move_done;
    af_run_mode_type          af_run_mode;
    int                       af_metering_mode;
    int                       af_steps_to_move;
    int                       af_mode;
    int                       af_cont_focus;
    boolean                   af_lock_caf;
    int                       af_ez_enable;
    af_roi_info_t             af_roi_info;
    af_input_from_awb_t       awb_info;
    af_input_from_aec_t       aec_info;
    af_input_from_sensor_t    sensor_info;
    af_input_from_gyro_t      gyro_info;
    boolean                   face_detected;
    af_input_from_gravity_vector_t gravity_info;
    af_input_from_isp_t       isp_info;
    af_sw_stats_t             sw_stats;
    af_sw_stats_t             *p_sw_stats;
    af_bestshot_mode_type_t   af_bestshot_mode;
    af_stream_crop_t          stream_crop;
    af_move_lens_cb_t         move_lens_cb_info;
    uint32_t                  stats_debug_mask;
    boolean                   ez_running;
    af_trigger_t              trigger;
    uint32_t                  af_set_sof_id;
    uint32_t                  af_set_parm_id;
    uint8_t                   af_set_meta_mode;
    int                       af_wait_for_aec_est;
    af_crop_info_t            crop_info;
    int32_t                   hfr_mode;
    mct_stats_info_t          af_stats_caps_info;
    int                       kernel_size;
    float                     manual_focus_distance;
    int32_t                   paaf_mode;
    int                       cur_pos_comp;
    depth_service_output_t    depth_service;
    af_input_manual_focus_t   af_manual_focus_info;
    boolean                   fast_aec_mode;
    boolean                   fd_enabled;
    af_core_dciaf_update_t    af_dciaf_update;
    q3a_custom_data_t         af_custom_data;
    af_haf_enable_type        haf;
    af_core_focus_info        af_peer_focus_info;
    af_role_switch_params_t   role_switch;            /**< AF Algorithm in  Follow Mode or not*/
    cam_related_system_calibration_data_t *p_af_syscalib_data;
    int                       ae_scan_test_config;
    q3a_sync_mode_t           multi_cam_sync_type;
    af_pdaf_data_t            pdaf_data;
    q3a_low_power_mode_t      low_power_mode;
    boolean                   fov_comp_enable;
    boolean                   mw_enable;
    boolean                   af_hj_enable;
    boolean                   skip_fine_scan;
    af_bg_stats_data_t        bg_stats;
    boolean                   af_led_assist;
  } u;
} af_set_parameter_t;

/** af_get_parameter_type:
 * List of AF parameters that can be
 * requested by other components.
 **/
typedef enum {
  AF_GET_PARAM_FOCUS_DISTANCES,       /* focus distances - near/far/optimal */
  AF_GET_PARAM_CUR_LENS_POSITION,     /* current lens position */
  AF_GET_PARAM_DEFAULT_LENS_POSITION, /* default reset position */
  AF_GET_PARAM_STATUS,                /* AF status - Focused/Unknown/Focusing */
  AF_GET_PARAM_STATS_CONFIG_INFO,     /* Stats configuration data */
  AF_GET_PARAM_FOCUS_MODE,            /* Autofocus mode */
  AF_GET_PARAM_MOBICAT_INFO,          /* Mobicat info */
  AF_GET_PARAM_SW_STATS_FILTER_TYPE,  /* SW Stats filter type */
} af_get_parameter_type;

/** _af_get_parameter:
 *    @type:              parameter type as listed by af_get_parameter_type
 *    @af_cur_lens_pos:   current lens position
 *    @af_def_lens_pos:   default lens reset position
 *    @af_status:         AF status - Focused/Unknown/Focusing
 *    @af_mode:           focus mode - Auto/Macro/Normal
 *    @af_focus_distance: focus distances
 *    @af_stats_config:   Stats configuration data
 *
 * Interface to get access to AF parameters from outside.
 **/
typedef struct _af_get_parameter {
  af_get_parameter_type  type;
  uint8_t               camera_id;
  union {
    int                  af_cur_lens_pos;
    int                  af_def_lens_pos;
    int                  af_status;
    int                  af_mode;
    af_focus_distances_t af_focus_distance;
    af_config_t          af_stats_config;
    af_sw_filter_type    af_sw_stats_filter_type;
  } u;
} af_get_parameter_t;

/** af_output_type
 * TODO
 **/
typedef enum {
  AF_OUTPUT_UPDATE_PENDING = 0,
  AF_OUTPUT_STOP_AF        = 1,
  AF_OUTPUT_CHECK_LED      = (1 << 1),
  AF_OUTPUT_STATUS         = (1 << 2),
  AF_OUTPUT_MOVE_LENS      = (1 << 4),
  AF_OUTPUT_FOCUS_MODE     = (1 << 5),
  AF_OUTPUT_ROI_INFO       = (1 << 6),
  AF_OUTPUT_EZTUNE         = (1 << 7),
  AF_OUTPUT_STATS_CONFIG   = (1 << 8),
  AF_OUTPUT_EZ_METADATA    = (1 << 9),
  AF_OUTPUT_SWAF_CONFIG    = (1 << 10),
  AF_OUTPUT_RESET_AEC      = (1 << 11),
  AF_OUTPUT_UPDATE_EVENT   = (1 << 12),
  AF_OUTPUT_SEND_EVENT     = (1 << 13),
  AF_OUTPUT_PDAF_CONFIG    = (1 << 14),
  AF_OUTPUT_TOF_CONFIG     = (1 << 15),
  AF_OUTPUT_DCIAF_CONFIG   = (1 << 16),
  AF_OUTPUT_CUSTOM         = (1 << 17),
  AF_OUTPUT_DEBUG_DATA     = (1 << 18),
  AF_OUTPUT_SPOT_LIGHT_DETECTION     = (1 << 19),
  AF_OUTPUT_FOCUS_VALUE        = (1 << 20),
  AF_OUTPUT_EXP_COMPENSATE = (1 << 21)
} af_output_type;

/** _af_output_data:
 *    @result:          TODO
 *    @type:            TODO
 *    @stop_af:         if true AF needs to be stopped.
 *    @check_led:       ask AEC to check if LED needs to be turned on
 *    @focus_mode_info: parameters specific to each focus mode
 *    @move_lens:       move the lens by number of steps if requested
 *    @focus_status:    AF status that needs to be sent
 *    @roi_info:        TODO
 *    @eztune:          eztune data
 *    @af_stats_config: Stats configuration data
 *    @asf_config:      configure software AF in the imglib
 *    @debug_info:      AF debug data that needs to be sent
 *
 * Output AF data for other components to consume once AF stat is processed.
 **/
typedef struct _af_output_data {
  af_output_type              type;
  uint32_t                    frame_id;
  boolean                     result;
  boolean                     stop_af;
  boolean                     check_led;
  af_mode_info_t              focus_mode_info;
  af_move_lens_t              move_lens;
  af_status_t                 focus_status;
  af_roi_info_t               roi_info;
  af_eztune_t                 eztune;
  af_mobicat_t                mobicat;
  af_config_t                 af_stats_config;
  uint32_t                    sof_id;
  swaf_config_data_t          swaf_config;
  af_core_pdaf_config_data_t  pdaf_config;
  af_core_dciaf_config_data_t dciaf_config;
  float                       grav_applied;
  q3a_custom_data_t           af_custom_param;
  af_debug_info_t             *debug_info;
  boolean                     af_depth_based_focus;
  af_core_focus_info          af_peer_focus_info;
  float                       focus_value;
  boolean                     spot_light_detected;
  boolean                     exp_comp_enable;
} af_output_data_t;

typedef boolean (* af_set_parameters_func)(af_set_parameter_t *param,
  af_output_data_t *output, uint8_t num_of_outputs, void *af_obj);
typedef boolean (* af_get_parameters_func)(af_get_parameter_t *param,
  void *af_obj);
typedef void (* af_process_func)(stats_af_t *stats, af_output_data_t *output,
  uint8_t num_of_outputs, void *af);
typedef void (* af_callback_func)(af_output_data_t *output, void *port);
typedef void * (* af_init_func)(void *af_lib);
typedef void (* af_deinit_func)(void *af_obj);

/** _af_ops:
 *    @set_parameters: function pointer to set parameters
 *    @get_parameters: function pointer to get parameters
 *    @process:        function pointer to process stats
 *
 * Operation table open to external component
 **/
typedef struct _af_ops {
  af_set_parameters_func  set_parameters;
  af_get_parameters_func  get_parameters;
  af_process_func         process;
  af_init_func            init;
  af_deinit_func          deinit;
}af_ops_t;

#endif /* __AF_H__ */
