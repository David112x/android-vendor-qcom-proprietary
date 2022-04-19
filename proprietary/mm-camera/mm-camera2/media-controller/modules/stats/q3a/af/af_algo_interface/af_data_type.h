/***************************************************************************
Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
***************************************************************************/

/* =======================================================================

DEFINITIONS AND DECLARATIONS

This section contains definitions for constants, macros, types, variables
and other items needed by this interface.

========================================================================== */
#ifndef __AF_DATA_TYPE_H__
#define __AF_DATA_TYPE_H__

#include "q3a_platform.h"

// Tuning data includes.
#include "chromatix_common.h"
#include "af_core.h"


#define MAX_DIST_MAP_H_NUM     16
#define MAX_DIST_MAP_V_NUM     12
#define MAX_DIST_MAP_NUM       (MAX_DIST_MAP_H_NUM * MAX_DIST_MAP_V_NUM)
#define DS_1E4                 10000
#define DS_PDAF_CONF_THRESH    200
#define MAX_PDAF_WINDOW        200


/** time_stamp_t
*    @time_stamp_sec: The seconds component of the timestamp
*    @time_stamp_us:  The microseconds component of the timestamp
*
*  This structure represents a standard OS timestamp, including two
*  components - time in seconds and time in microseconds.
**/
typedef struct {
  uint32_t time_stamp_sec; /*time stamp second*/
  uint32_t time_stamp_us;  /*time stamp microsecond*/
} time_stamp_t;


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
    uint16 x;
    uint16 y;
    uint16 dx;
    uint16 dy;
} af_roi_t;


/** _af_roi_info:
*    @roi_updated: check if ROI information has been updated
*    @type:        ROI type - General/Touch/Face
*    @frm_id:      frame ID
*    @num_roi:     Number of ROIs detected
*    @roi:         structure holding dimensions of each ROI
*    @weight:      array holding the weights
*    @roi_moving_rate_x:      how fast ROI moves in X axis compare to last frame
*    @roi_moving_rate_y:      how fast ROI moves in Y axis compare to last frame
*
* ROI selected for AF stats.
**/
typedef struct _af_roi_info {
    boolean     roi_updated;
    af_roi_type type;
    uint32    frm_id;
    uint32    num_roi;
    af_roi_t  roi[MAX_STATS_ROI_NUM];
    uint32    weight[MAX_STATS_ROI_NUM];
    float     roi_moving_rate_x;
    float     roi_moving_rate_y;
} af_roi_info_t;


/** _af_input_from_aec:
*    @aec_settled:              Flag to check if aec is settled now.
*    @exp_index:               Exposure index
*    @pixels_per_region:    Number of pixels per Region
*    @comp_luma:             Compensated luma
*    @cur_luma:                Current luma value
*    @cur_real_gain:          Current Real Gain
*    @lux_idx:                    Lux Index
*    @num_regions:           Number of regions of the Luma Sum
*    @exp_tbl_val:             Exposure table Size
*    @luma_settled_cnt:     Number of settled AEC Frames
*    @SY:                          Array of Luma Sum
*
* Data needed from AEC module for AF operation.
**/
typedef struct _af_input_from_aec {
    int           aec_settled;
    int           exp_index;
    uint32        pixels_per_region;
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
    uint32      SY[MAX_YUV_STATS_NUM];
    /*TDB : Apex information*/
    float Av_af;
    float Tv_af;
    float Sv_af;
    float Bv_af;
    float Ev_af;
} af_input_from_aec_t;


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
    uint8 accuracy;
} af_input_from_gravity_vector_t;

/** _af_input_from_img_sensor:
 *    @focal_length: focal length of the sensor
 *    @af_f_num: f-number or aperture of the lens
 *    @af_f_pix: pixel size of the sensor
 *    @af_total_f_dist : Current actuator sensitivity
 *    @sensor_res_height: camif height
 *    @sensor_res_width: camif width
 *    @actuator_sensitivity: sensitivity of the current actuator
 *
 * Data needed from sensor module for AF operation.
 **/
typedef struct _af_input_from_img_sensor {
  float focal_length;
  float af_f_num;
  float af_f_pix;
  float af_total_f_dist;
  uint32 sensor_res_height;
  uint32 sensor_res_width;
  float actuator_sensitivity;
} af_input_from_img_sensor_t;

/** af_move_direction_type:
*
* Direction to move
*
**/
typedef enum {
    AF_MOVE_NEAR, /* Move towards MACRO position */
    AF_MOVE_FAR,  /* Move towards INFY position */
} af_move_direction_type;


/**
 * af_cam_rect_t
 **/
typedef struct  {
    int32_t left;
    int32_t top;
    int32_t width;
    int32_t height;
} af_cam_rect_t;


/** af_dcrf_result_t
 *   @id: Frame ID
 *   @timestamp: Time stamp
 *   @distance_in_mm: Distance of object in ROI's in mili meters
 *   @confidence: Confidence on distance from 0(No confidence)
 *              to 1024(Max confidence)
 *   @status: Status of DCRF library execution call
 *   @focused_roi: ROI's for which distance is estimated
 *   @focused_x: Focus location X inside ROI with distance
 *             estimation
 *   @focused_y: Focus location Y inside ROI with distance
 *             estimation
 **/
typedef struct
{
    uint32_t id;
    uint64_t timestamp;
    uint32_t distance_in_mm;
    uint32_t confidence;
    uint32_t status;
    af_cam_rect_t focused_roi;
    uint32_t focused_x;
    uint32_t focused_y;
} af_dcrf_result_t;


/** depth_service_type_t:
 *
 * Enum to distinguish differnt AF technology Input
 *
 **/
typedef enum {
    DEPTH_SERVICE_UNKNOWN = 0,
    DEPTH_SERVICE_PDAF,
    DEPTH_SERVICE_TOF,
    DEPTH_SERVICE_DUAL_CAM_AF,
    DEPTH_SERVICE_MAX,
} depth_service_type_t;


/* Struct definition */
/** depth_service_roi_t:
 *    @x:   horizon tal offset of the region
 *    @y:   vertical offset of the region
 *    @dx:  width of the RoI region
 *    @dy:  height of the region
 *
 * RoI information in terms of "CAMIF"
 **/
typedef struct _depth_service_roi_t {
    uint16_t x;
    uint16_t y;
    uint16_t dx;
    uint16_t dy;
} depth_service_roi_t;


/** depth_service_af_info_t:
 *    @af_focus_pos: Current AF position based on logical idx
 *
 **/
typedef struct _depth_service_af_info_t {
    int32_t  af_focus_pos_dac;
} depth_service_af_info_t;


/** depth_service_aec_info_t:
 *    @lux_idx: Current lux index
 *    @real_gain: Current gain
 *    @line_count: Current line count
 *
 **/
typedef struct _depth_service_aec_info_t {
    float lux_idx;
    float real_gain;
    int32_t line_count;
} depth_service_aec_info_t;


/** awb_gain_t: AWB RGB gains.
*
**/
typedef struct {
    float r_gain;
    float g_gain;
    float b_gain;
} awb_gain_t;



/** depth_service_awb_info_t:
 *    @gain: R,G and B gains
 *    @color_temp: Current color temperature
 &    @decision: Current decision
 *
 **/
typedef struct _depth_service_awb_info_t {
    awb_gain_t gain;
    uint32_t color_temp;
    int decision;
} depth_service_awb_info_t;


/** depth_service_gyro_t:
 *    @float_ready: float data is available
 *    @flt:         gyro metrics in float
 *    @q16_ready:   gyro metrics in q16 ready to use
 *    @q16:         gyro metrics in q16
 *
 * Gyro data required to assist AF.
**/
typedef struct _depth_service_gyro_t {
    int   float_ready;
    float flt[3];
    int   q16_ready;
    long  q16[3];
} depth_service_gyro_t;


/** depth_service_gravity_t
 *  @gravity:  gravity vector (x/y/z) in m/s/s
 *  @lin_accel: linear acceleration (x/y/z) in m/s/s
 *  @accuracy: sensor accuracy (unreliable/low/medium/high)
 *
 *  This structure is used to store and trasnmit gravity
 *  vector received from the motion sensor.
 *
 **/
typedef struct _depth_service_gravity_t {
    float gravity[3];
    float lin_accel[3];
    uint8_t accuracy;
} depth_service_gravity_t;



/** depth_service_af_info_t:
 *
 *    @frame_id : Frame id associated to each SOF
 *    @type: Determine which AF technology is used
 *    @grid_roi: roi of depth map based on CAMIF
 *    @x_win_num: Number of horizontal regions
 *    @y_win_num: Number of vertical regions
 *
 * Generic info which needs to be copes
 **/
typedef struct _depth_service_info_t{
    uint32_t frame_id;
    depth_service_type_t type;
    depth_service_roi_t grid_roi;
    uint8_t x_win_num;
    uint8_t y_win_num;
    depth_service_af_info_t af;
    depth_service_aec_info_t aec;
    depth_service_awb_info_t awb;
    depth_service_gravity_t gravity;
    depth_service_gyro_t gyro;
} depth_service_info_t;


/** _defocus_cal_data_t:
 *
 *    @actuator_travel_btw_cal_pts_um : 0 - invalid
 *    @af_cal_inf_ff: 0 - invalid
 *    @af_cal_macro_ff:  0 - invalid
 *    @af_cal_fd:
 *    @af_cal_ff:
 *    @actuator_tuning_lens_efl:
 *
 * Generic info which needs to be copes
 **/
typedef struct {
    boolean is_pdaf_supported;
    float af_cal_inf_ff;
    float af_cal_inf_fd;
    float af_cal_inf_fu;
    float eff_focal_length;
    float f_num;
    float pixel_size;
    float actuator_sensitivity;
} depth_cal_data_t;


/** _pdaf_sensor_info_t:
*
*    @actuator_sensitivity : Sensor actuator sensitivity
*    @sensor_gain : Sensor gain correspodning to PD sample
*    @scene_bv : Scene Bv from AEC correspodning to PD sample
*    @lens_pos : Lens position correspodning to PD sample
*    @fps      : Frame per second information to PD sample
*
* Generic info which needs to be copes
**/
typedef struct _pdaf_sensor_info_t{
    float actuator_sensitivity;
    float sensor_gain;
    int   lens_pos;
    float scene_bv;
    float fps;
} pdaf_sensor_info_t;


typedef struct {
    int32_t defocus;
    int8_t df_confidence; /* 0-good, (-1)-not good*/
    uint32_t df_conf_level; /* = 1024*ConfidentLevel/Threshold */
    float phase_diff;
} pdaf_defocus_t;

typedef struct {
    uint8_t * pd_stats;
   /* sensor output */
    uint32_t x_offset;
    uint32_t y_offset;
    uint32_t x_win_num;
    uint32_t y_win_num;
    pdaf_defocus_t defocus[MAX_PDAF_WINDOW];
    boolean status;
} pdaf_update_t;

/** tof_update_t:
 * This is the structure that would be passed from laser sensor to Android
 * sensor Framework.
 *
 *  @frame_id : current frame id, can be optional since it relies on timestamp
 *  @timestamp : timestamp of arrival of the laser data
 *  @distance: calculated object distance in mm
 *  @confidence: confidence of distance measured
 *  @near_limit: near end of distance measured in mm (when
 *             laser is semi confident)
 *  @far_limit: far end of distance measured in mm
 *  @max_distance: maximum distance that can be measured by this
 *               laser
 *
 **/
typedef struct {
  uint32_t frame_id;
  int64_t timestamp;
  int32_t distance;
  int32_t confidence;
  int32_t near_limit;
  int32_t far_limit;
  int32_t max_distance;
} tof_update_t;

/** _depth_service_input_t:
 *
 *    @depth_service_info_t : Information to be copied to output
 *    @pdaf_info: Sensor output from PDAF library
 *
 * Input to Depth service layer
 **/
typedef struct _depth_service_input_t{
    depth_service_info_t info;
    depth_cal_data_t cal_data;
    union {
      pdaf_update_t pdaf_info;
      af_dcrf_result_t dciaf_info;
      tof_update_t  tof_info;
    } u;
    union {
      pdaf_sensor_info_t pdaf_sensor_info;
      //tof_sensor_info_t  tof_sensor_info;
      //dual_cam_af_sensor_info_t dual_cam_af_sensor_info;
    } u_sensor_info;
} af_depth_service_input_t;


/** _af_depth_service_output_t
 *
 **/
typedef struct _af_depth_service_output_t {
    af_depth_service_input_t input;
} af_depth_service_output_t;


#endif
