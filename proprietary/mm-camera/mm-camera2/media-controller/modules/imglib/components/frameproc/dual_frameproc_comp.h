/***************************************************************************
* Copyright (c) 2015-2016 Qualcomm Technologies, Inc.                      *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

#ifndef __DUAL_FRAMEPROC_COMP_H__
#define __DUAL_FRAMEPROC_COMP_H__

#include <pthread.h>
#include "img_comp_priv.h"
#include "img_thread_ops.h"
#include "img_list.h"

#define MAX_COMP_INST 2

/** MODULE_MASK:
 *
 * Mask to enable dynamic logging
 **/
#undef MODULE_MASK
#define MODULE_MASK IMGLIB_DCRF_SHIFT

/** img_af_cam_orientation_type
*
*    AF_CAM_ORIENT_LEFT: aus sensor at left
*    AF_CAM_ORIENT_RIGHT: aus sensor at right
*    AF_CAM_ORIENT_UP: aus sensor at up
*    AF_CAM_ORIENT_DOWN: aus sensor at down
*    AF_CAM_ORIENT_MAX: max orient value
*
**/
typedef enum {
  IMG_AF_CAM_ORIENT_LEFT = 0,
  IMG_AF_CAM_ORIENT_RIGHT,
  IMG_AF_CAM_ORIENT_UP,
  IMG_AF_CAM_ORIENT_DOWN,
  IMG_AF_CAM_ORIENT_MAX,
} img_af_cam_orientation_type;

/** dcrf_frame_format_type
*
*    DCRF_FORMAT_YUV: YUV semi planar
*    DCRF_FORMAT_YUYV_YVYU: YUV interleaved YUYV and YVYU
*    DCRF_FORMAT_UYVY_VYUY: YUV interleaved UYVY and VYUY
*    DCRF_FORMAT_MAX: max format value
*
**/
typedef enum {
  DCRF_FORMAT_YUV = 0,
  DCRF_FORMAT_YUYV_YVYU,
  DCRF_FORMAT_UYVY_VYUY,
  DCRF_FORMAT_MAX,
} dcrf_frame_format_type;

typedef struct dual_frameproc_comp_inst_t dual_frameproc_comp_inst_t;

/** img_dcrf_intrinsics_t
 *    @normalized_focal_length:  Focal length in pixels @
 *      calibration resolution. Intrinsic param output by Calib
 *    @native_sensor_resolution_width:
 *      Native sensor resolution W that was used to capture calibration image
 *    @native_sensor_resolution_height:
 *      Native sensor resolution H that was used to capture calibration image
 *    @calibration_sensor_resolution_width:
 *      Image size W used internally by calibration tool
 *    @calibration_sensor_resolution_height:
 *      Image size H used internally by calibration tool
 *    @focal_length_ratio: Focal length ratio @ Calibration
 *
 *    DCRF intrinsics
**/
typedef struct {
  float       normalized_focal_length;
  uint16_t    native_sensor_resolution_width;
  uint16_t    native_sensor_resolution_height;
  uint16_t    calibration_sensor_resolution_width;
  uint16_t    calibration_sensor_resolution_height;
  float       focal_length_ratio;
} img_dcrf_intrinsics_t;

/** img_dcrf_calib_t
 *    @calibration_format_version:  Version information
 *    @main_cam_specific_calibration:  Main Camera Calibration
 *    @aux_cam_specific_calibration:  Aux Camera calibration
 *    @relative_rotation_matrix:  Relative viewpoint matching matrix w.r.t Main
 *    @relative_geometric_surface_parameters:
 *      Relative geometric surface description parameters
 *    @relative_principle_point_x_offset:
 *      Relative offset of sensor center from optical axis along horizontal dimension
 *    @relative_principle_point_y_oofset:
 *      Relative offset of sensor center from optical axis along vertical dimension
 *    @relative_position_flag:  0=Main Camera is on the left of Aux;
 *       1=Main Camera is on the right of Aux
 *    @relative_baseline_distance:  Camera separation in mm
 *    @main_sensor_mirror_flip_setting: 0-none, 1-hor-mirror, 2-ver-flip,
 *       3-both
 *    @aux_sensor_mirror_flip_setting: 0-none, 1-hor-mirror, 2-ver-flip,
 *       3-both
 *    @module_orientation_during_calibration: 0-sensors in landscape,
 *       1-sensors in portrait
 *    @rotation_flag:  0-no, 1-90 degrees right, 2-90 degrees left
 *    @extra_padding:  Reserved for future use
 *
 *    DCRF calibration data
**/
typedef struct {
  uint32_t      calibration_format_version;
  img_dcrf_intrinsics_t main_cam_specific_calibration;
  img_dcrf_intrinsics_t aux_cam_specific_calibration;
  float      relative_rotation_matrix[9];
  float      relative_geometric_surface_parameters[32];
  float      relative_principle_point_x_offset;
  float      relative_principle_point_y_offset;
  uint16_t   relative_position_flag;
  float      relative_baseline_distance;
  uint16_t   main_sensor_mirror_flip_setting;
  uint16_t   aux_sensor_mirror_flip_setting;
  uint16_t   module_orientation_during_calibration;
  uint16_t   rotation_flag;
  float      extra_padding[62];
} img_dcrf_calib_t;

/** img_dcrf_params_t
 *    @width:  frame width
 *    @height:  frame height
 *    @stride:  frame stride
 *    @format:  frame format
 *    @sensor_w:  sensor width
 *    @sensor_h:  sensor height
 *    @sensor_x_offset:  sensor x offset
 *    @sensor_y_offset:  sensor y offset
 *    DCRF params
**/
typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t stride;
  uint32_t format;
  uint32_t sensor_w;
  uint32_t sensor_h;
  uint32_t sensor_x_offset;
  uint32_t sensor_y_offset;
} img_dcrf_params_t;

/** img_dual_cam_init_params_t
 *    @dcrf_calib_data: dcrf calibration data
 *    @mainh: init params for main camera
 *    @aux: init params for aux camera
 *    @focus_preference: dcrf focus preference
 *    @aux_direction: aux sensor position to the main sensor
 *    @baseline_mm: baseline distance in mm
 *    @alignment_check_enable: alignment check enable flag
 *    @macro_est_limit_cm: estimation limit in cm
 *
 *    DCRF init params
 **/
typedef struct {
  img_dcrf_calib_t dcrf_calib_data;
  img_dcrf_params_t main;
  img_dcrf_params_t aux;
  uint32_t focus_preference;
  uint8_t aux_direction;  //Todo: get the types in chromatix
  float baseline_mm;
  uint8_t alignment_check_enable;
  float macro_est_limit_cm;
} img_dual_cam_init_params_t;

/** dual_frameproc_comp_t
 *   @ptr: library handle
 *   @img_algo_process: Main algo process function pointer
 *   @img_algo_init: Function pointer to create and initialize
 *                 the algorithm wrapper
 *   @img_algo_deinit: Function pointer to deinitialize and
 *                   destroy the algorithm wrapper
 *   @img_algo_frame_ind: Function pointer to indicate when an
 *             input frame is available
 *   @img_algo_meta_ind: Function pointer to indicate when an
 *             meta frame is available
 *   @img_algo_set_frame_ops: Function pointer to set frame ops
 *
 *   dual_frameproc library function pointers
 **/
typedef struct {
  void *ptr;
  int (*img_algo_process)(void *p_context,
    img_frame_t *p_in_frame[],
    int in_frame_cnt,
    img_frame_t *p_out_frame[],
    int out_frame_cnt,
    img_meta_t *p_meta[],
    int meta_cnt);
  int (*img_algo_init)(void **pp_context, img_dual_cam_init_params_t *p_params);
  int (*img_algo_deinit)(void *p_context);
  int (*img_algo_frame_ind)(void *p_context, img_frame_t *p_frame);
  int (*img_algo_meta_ind)(void *p_context, img_meta_t *p_meta);
  int (*img_algo_set_frame_ops)(void *p_context, img_frame_ops_t *p_ops);
} dual_frameproc_lib_info_t;

/** dual_frame_buffer_t
 *   @p_main_buf: pointer to main frame
 *   @p_aux_buf: pointer to aux frame
 *   @p_meta: pointer to meta frame
 *   @is_busy: flag indicate the buf is being processed
 *   @is_filled: flag indicate pre-process data has been filled
 *
 *   dual_frameproc frame buffer
 **/
typedef struct {
  img_frame_t *p_main_buf;
  img_frame_t *p_aux_buf;
  img_meta_t *p_meta;
  uint32_t is_busy;
  uint32_t is_filled;
} dual_frame_buffer_t;

/** dual_frameproc_comp_t
 *   @b: base component
 *   @p_lib: library info
 *   @p_algocontext: algorithm context
 *   @abort: Flag to indicate whether the algo is aborted
 *   @msgQ: Message queue
 *   @inst_list: List to save all comp instances
 *   @inst_cnt: counter of active comp instances
 *   @mutex: component lock
 *
 *   dual_frameproc component/shared by all clients
 **/
typedef struct {
  /*base component*/
  img_component_t b;
  dual_frameproc_lib_info_t *p_lib;
  void *p_algocontext;
  int8_t abort;
  img_queue_t msgQ;
  img_list_t *inst_list;
  uint32_t inst_cnt;
  pthread_mutex_t mutex;
} dual_frameproc_comp_t;

/** img_component_t
 *   @inputQ: queue for storing the input buffers
 *   @outBufQ: queue for storing the output buffers
 *   @outputQ: queue for storing the buffers after processed
 *   @metaQ: metadata queue
 *   @p_comp_body: pointer to shared component
 *   @p_peer_inst: pointer to peer comp instance
 *   @p_cb: pointer to callback function
 *   @master_flag: flag to indicate main camera
 *   @p_userdata: pointer to the userdata
 *   @p_cached_frame: pointer to cached frame
 *   @cached_flag: flag indicate if the frame is cached
 *   @current_timestamp: latest buffered main frame time stamp
 *   @dual_frame_buf: Ping-Pong buffer to cache frames
 *   @cur_buf_idx: current index in Ping-Pong buffer
 *   @thread_job: params for schedule thread manager job
 *   @init_params: init params for DCRF lib
 *   @prev_job_id: job id for previous schedule job
 *   @dcrf_result: DCRF result
 *   @busy_cnt: DCRF busy job count
 *   @unbind_flag: flag to indicate if unbind is received
 *   @af_fps: FPS from AF
 *   @processing_timestamp: timestamp of the frame being processed
 *   @filled_frame_id: the most recent filled frame id
 *   @fov_params_aux: runtime FOV for AUX sensor
 *   @mutex: component lock
 *   @buf_mutex: buffer lock
 *
 *   dual frameproc comp instances/every client has one instance
**/
struct dual_frameproc_comp_inst_t{
  img_queue_t inputQ;
  img_queue_t outBufQ;
  img_queue_t outputQ;
  img_queue_t metaQ;
  dual_frameproc_comp_t *p_comp_body;
  dual_frameproc_comp_inst_t *p_peer_inst;
  notify_cb p_cb;
  uint32_t master_flag;
  void *p_userdata;
  img_frame_t *p_cached_frame;
  uint32_t cached_flag;
  uint64_t current_timestamp;
  dual_frame_buffer_t dual_frame_buf[2]; //Ping-Pong buffer
  uint32_t cur_buf_idx;
  img_thread_job_params_t thread_job;
  img_dual_cam_init_params_t init_params;
  uint32_t prev_job_id;
  img_dcrf_output_result_t dcrf_result;
  uint32_t busy_count;
  uint32_t unbind_flag;
  uint32_t af_fps;
  uint64_t processing_timestamp;
  uint32_t filled_frame_id;
  img_fov_t fov_params_aux[IMG_MAX_FOV];
  pthread_mutex_t mutex;
  pthread_mutex_t buf_mutex;
};

#endif //__DUAL_FRAMEPROC_COMP_H__
