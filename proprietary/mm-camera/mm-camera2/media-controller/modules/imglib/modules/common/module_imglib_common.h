/**********************************************************************
*  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __MODULE_IMGLIB_COMMON_H__
#define __MODULE_IMGLIB_COMMON_H__

#include <linux/media.h>
#include <pthread.h>
#include <media/msm_media_info.h>
#include "img_common.h"
#include "img_comp.h"
#include "img_dbg.h"
#include "img_queue.h"
#include "mct_port.h"
#include "mct_stream.h"
#include "mct_pipeline.h"
#include "modules.h"
#include "mgr/scene_detect_mgr.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

/** MOD_IMGLIB_ZOOM_DENUMINATOR
 *
 * Macro to set zoom denuminator
 **/
#define MOD_IMGLIB_ZOOM_DENUMINATOR 100

/** ABS:
 *    @x: value
 *
 * Macro to get absolute value of x
 **/
#if !defined ABS
  #define  ABS(x) ((x) > 0 ? (x) : -(x))
#endif

/** GET_STATUS
 *
 * Macro to convert img_status_t to boolean
 **/
#define GET_STATUS(v) (((v) == IMG_SUCCESS) ? TRUE : FALSE)

/** IMGLIB_SESSIONID
 *
 * Macro to get the session id
 **/
#define IMGLIB_SESSIONID(id) (((id) & 0xFFFF0000) >> 16)

/** IMGLIB_STREAMID:
 *
 * Macro to get the stream id
 **/
#define IMGLIB_STREAMID(id) ((id) & 0x0000FFFF)

/** IMGLIB_OVERFLOW:
 *
 * Macro which returns FALSE if a > b
 **/
#define IMGLIB_OVERFLOW(a , b) ((a) > (b) ? FALSE : TRUE)

/** IMGLIB_PACK_IDENTITY:
 *
 * Macro to pack the stream and session id
 **/
#define IMGLIB_PACK_IDENTITY(session_id, stream_id) \
  ((((session_id) & 0x0000FFFF) << 16) + ((stream_id) & 0x0000FFFF))

/** IMGLIB_GUARDED_CALL:
 *    @condition: condition
 *    @func: function to be called
 *
 * Calls specified function in case the condition is TRUE
 *
 * Returns FALSE in case condition is FALSE or
 *   function return value in case the condition is TRUE
 **/
#define IMGLIB_GUARDED_CALL(condition, function) \
  ((condition) ? (function) : FALSE)

/** PORT_NAME_LEN
 *
 * Max length of port name
 **/
#define PORT_NAME_LEN 32

/** MODULE_FACEPROC_DEBUG
 *
 * Enable logging for faceproc module
 **/
#define MODULE_FACEPROC_DEBUG 1

/** imglib_state_t
 *   @IMGLIB_STATE_IDLE: idle state
 *   @IMGLIB_STATE_INIT: init state
 *   @IMGLIB_STATE_STARTED: started state
 *   @IMGLIB_STATE_PROCESSING: processing state
 *   @IMGLIB_STATE_START_PENDING: pending for start
 *   @IMGLIB_STATE_STOP_PENDING: pending to stop
 *   @IMGLIB_STATE_ERROR: error state
 *
 *   States for IMGLIB state machine
 **/
typedef enum _imglib_state_t {
  IMGLIB_STATE_IDLE,
  IMGLIB_STATE_INIT,
  IMGLIB_STATE_STARTED,
  IMGLIB_STATE_PROCESSING,
  IMGLIB_STATE_START_PENDING,
  IMGLIB_STATE_STOP_PENDING,
  IMGLIB_STATE_ERROR,
} imglib_state_t;

/** mod_img_msg_type_t
 *   @MOD_IMG_MSG_BUF_ACK: buffer acknowledge message
 *   @MOD_IMG_MSG_DIVERT_BUF: divert message
 *   @MOD_IMG_MSG_EXIT: exit message
 *
 *   buffer ack message data
 **/
typedef enum {
  MOD_IMG_MSG_BUF_ACK, /* mod_img_msg_buf_ack_t */
  MOD_IMG_MSG_DIVERT_BUF, /* mod_img_msg_buf_divert_t */
  MOD_IMG_MSG_EXEC_INFO, /* mod_img_msg_exec_t */
  MOD_IMG_MSG_EXIT, /* int */
} mod_img_msg_type_t;

/** mod_img_msg_buf_ack_t
 *   @frame_id: frame id
 *   @identity: MCT identity
 *
 *   buffer ack message data
 **/
typedef struct {
  int frame_id;
  uint32_t identity;
} mod_img_msg_buf_ack_t;

/** divert_buf_exec
 *
 *   divert buf function
 **/
typedef void (*divert_buf_exec)(void *user_data, void *data);

/** mod_img_msg_buf_divert_t
 *   @frame_id: frame id
 *   @buf_idx: buffer index
 *   @identity: MCT identity
 *   @p_exec: thread exec function
 *   @userdata: user data pointer
 *   @port: MCT port
 *
 *   buffer divert message data
 **/
typedef struct {
  isp_buf_divert_t buf_divert;
  uint32_t identity;
  divert_buf_exec p_exec;
  void *userdata;
  mct_port_t *port;
} mod_img_msg_buf_divert_t;

/** mod_img_msg_exec_t
 *   @p_exec: thread exec function
 *   @p_userdata: user data pointer
 *   @data: function parameter
 *
 *   exec message data
 **/
typedef struct {
  divert_buf_exec p_exec;
  void *p_userdata;
  void *data;
} mod_img_msg_exec_t;

/** mod_img_msg_t
 *   @type: message type
 *   @port: pointer to the port
 *   @buf_ack: buffer ack data
 *   @buf_divert: buffer divert data
 *
 *   Imaging message structure
 **/
typedef struct {
  mod_img_msg_type_t type;
  mct_port_t *port;

  union {
    mod_img_msg_buf_ack_t buf_ack;
    mod_img_msg_buf_divert_t buf_divert;
    mod_img_msg_exec_t exec_info;
  } data;
} mod_img_msg_t;

/** mod_img_buffer_t
 *   @frame: Frame structure used by imglib components
 *   @map_buf: buffer map provided by MCT
 *
 *   Imaging buffer structure
 **/
typedef struct {
  img_frame_t frame;
  mct_stream_map_buf_t map_buf;
} mod_img_buffer_t;

/** mod_img_buffer_info_t
 *   @p_buffer: array of buffers
 *   @buf_count: buffer count
 *   @total_count: total number of buffers allocates
 *   @fd_buf_count: number of fd buffers
 *
 *   This structure represents the buffer information for
 *   imaging module
 **/
typedef struct {
  mod_img_buffer_t *p_buffer;
  uint32_t buf_count;
  uint32_t total_count;
  uint32_t fd_buf_count;
} mod_img_buffer_info_t;

/** mod_imglib_msg_th_t
 *   @threadid: thread id
 *   @msg_q: message queue
 *   @is_ready: flag to indicate when thread is created
 *   @abort_flag: flag to indicate if abort is issued
 *
 *   This structure represents the generic message thread for
 *   imaging module
 **/
typedef struct {
  pthread_t threadid;
  img_queue_t msg_q;
  int is_ready;
  int abort_flag;
} mod_imglib_msg_th_t;

/** imglib_pfc_ctrl_type_t
 *  @IMGLIB_APPLY: Apply the ctrl
 *  @IMGLIB_POST: Post the output to metadata
 *
 *   This enumeration is for per frame control message type
 **/
typedef enum {
  IMGLIB_APPLY,
  IMGLIB_POST,
} imglib_pfc_ctrl_type_t;

/** module_imglib_destroy_msg_thread
 *
 * Enable logging for faceproc module
 **/
int module_imglib_destroy_msg_thread(mod_imglib_msg_th_t *p_msg_th);

int module_imglib_send_msg(mod_imglib_msg_th_t *p_msg_th,
  mod_img_msg_t *p_msg);

int module_imglib_create_msg_thread(mod_imglib_msg_th_t *p_msg_th);
void module_imglib_check_create_msg_thread(mod_imglib_msg_th_t *p_msg_th);

boolean mod_imglib_map_fd_buffer(void *data, void *user_data);
boolean mod_imglib_map_fr_buffer(void *data, void *user_data);

/**
 * Function: module_imglib_post_bus_msg
 *
 * Description: post a particular message to media bus
 *
 * Arguments:
 *   @p_mct_mod - media controller module
 *   @sessionid - session id
 *   @msg_id - bus message id
 *   @msg_data - bus message data
 *   @msg_size - bus message size
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_imglib_post_bus_msg(mct_module_t *p_mct_mod,
  unsigned int sessionid, mct_bus_msg_type_t msg_id,
  void *msg_data, uint32_t msg_size);

/**
 * Function: module_imglib_post_dynamic_scene_data
 *
 * Description: post dynamic scene detect data via bus message
 *
 * Arguments:
 *   @p_dyn_data - ptr to dynamic data
 *   @p_port - mct port
 *   @event_idx - event identity
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_imglib_post_dynamic_scene_data(cam_dyn_img_data_t *p_dyn_data,
  mct_port_t *p_port, uint32_t event_idx);


/** mod_imglib_dump_stream_info
 *    @info: stream info configuration
 *
 * Prints stream info configuration
 *
 * Returns TRUE in case of success
 **/
void mod_imglib_dump_stream_info(mct_stream_info_t* info);

/** mod_imglib_find_module_parent
 *    @identity: required identity
 *    @module: module, whichs parents will be serached
 *
 * Finds module parent (stream) with specified identity
 *
 * Returns Pointer to stream handler in case of cucess
 *   or NULL in case of failure
 **/
mct_stream_t* mod_imglib_find_module_parent(uint32_t identity,
  mct_module_t* module);

/** module_imglib_common_get_buffer:
 *  @subdev_fd: buffer mgr fd
 *  @identity: stream/session id
 *  @p_buffer_access: fill current buffer_access flag, ideally should be 0
 *  @requested_buf: In HAL3 fill the requested buffer index,
 *    for HAL1 0xFFFFFFFF
 *
 * Function to get buffer for denoise port
 *
 * Returns buffer index
 **/
int module_imglib_common_get_buffer(int subdev_fd, uint32_t identity,
  uint32_t *p_buffer_access, uint32_t requested_buf);

/** module_imglib_common_release_buffer_idx:
 *  @subdev_fd: buffer mgr fd
 *  @identity: stream/session id
 *  @idx: buffer index
 *  @frame_id: frame id
 *  @buff_done: buff done/put buff
 *  @p_frame: current frame
 *  @ion_fd: ion fd
 *  @buffer_access: buffer access flags
 *
 * Function to release buffer back to kernel
 *
 * Returns imaging error values
 **/
int module_imglib_common_release_buffer(int subdev_fd, uint32_t identity,
  uint32_t idx, uint32_t frame_id, boolean buff_done,
  img_frame_t *p_frame, int32_t ion_fd, uint32_t buffer_access);

/** module_imglib_common_get_bfr_mngr_subdev:
 *  @buf_mgr_fd: buffer manager file descriptor
 *
 * Function to get buffer manager file descriptor
 *
 * Returns TRUE in case of success
 **/
int module_imglib_common_get_bfr_mngr_subdev(int *buf_mgr_fd);

/** mod_imglib_find_module_parent
 *    @identity: required identity
 *    @module: module, whichs parents will be serached
 *
 * Finds module parent (stream) with specified identity
 *
 * Returns Pointer to stream handler in case of cucess
 *   or NULL in case of failure
 **/
mct_stream_t* mod_imglib_find_module_parent(uint32_t identity,
  mct_module_t* module);
/** module_imglib_get_next_from_list
 *    @data1: not used
 *    @data2: not used
 *
 *  Gets next element from the list
 *
 *  Return TRUE always
 **/
boolean module_imglib_get_next_from_list(void *data1, void *data2);

/** module_imglib_free_func
 *    @data: node data
 *    @user_data: not used. Null pointer
 *
 * Memory release traverse function for flushing queue
 *
 * Returns TRUE
 **/
boolean module_imglib_free_func(void *data, void *user_data);

/** module_imglib_common_get_miscdata:
 *  @info: Stream info
 *  @meta_index: Miscellaneous buffer index
 *
 * Function to get miscellaneous buffer pointer
 *
 * Returns Pointer to miscellaneous buffer / NULL on fail
 **/
void *module_imglib_common_get_miscdata(mct_stream_info_t *info,
  uint32_t misc_index);

/** module_imglib_common_get_metadata_buffer:
 *  @info: Stream info
 *  @meta_index: Metadata buffer index
 *
 * Function to get metadata buffer pointer
 *
 * Returns Pointer to metadata buffer / NULL on fail
 **/
metadata_buffer_t *module_imglib_common_get_metadata(mct_stream_info_t *info,
  uint32_t meta_index);

/**
 * Function: module_imglib_common_get_zoom_ratio
 *
 * Description: This function is returning current zoom ratio
 *
 * Arguments:
 *   @p_mct_mod: mct_module
 *   @zoom_level: hal zoom level
 *
 * Return values:
 *     zoom ratio, 0 if error
 *
 * Notes: none
 **/
float module_imglib_common_get_zoom_ratio(mct_module_t *p_mct_mod,
  uint8_t zoom_level, size_t zoom_ratio_tbl_cnt, uint32_t *zoom_ratio_tbl);

/**
 * Function: module_imglib_common_get_zoom_level
 *
 * Description: This function is returning current zoom ratio
 *
 * Arguments:
 *   p_mct_cap - capababilities
 *   @zoom_ratio: zoom ratio
 *
 * Return values:
 *     zoom level, -1 if error
 *
 * Notes: none
 **/
int module_imglib_common_get_zoom_level(mct_pipeline_cap_t *p_mct_cap,
  float zoom_ratio);

/**
 * Function: module_imglib_common_meta_find_by_frameid
 *
 * Description: This function is finds metadata pointer for current frame id
 *
 * Arguments:
 *   @current_frame_id: current frame id
 *   @p_meta_list - list of frame metadata
 *   @p_meta: output metadata pointer for current frame id
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_find_by_frameid(uint32_t current_frame_id,
  mct_list_t *p_meta_list, img_meta_t** p_meta);

/**
 * Function: module_imglib_common_meta_get_by_frameid
 *
 * Description: This function is gets metadata pointer for current frame id
 *
 * Arguments:
 *   @current_frame_id: current frame id
 *   @p_meta_list - list of frame metadata
 *   @p_meta: output metadata pointer for current frame id
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_get_by_frameid(uint32_t current_frame_id,
  mct_list_t **p_meta_list, img_meta_t** p_meta);

/**
 * Function: module_imglib_common_meta_set_aec
 *
 * Description: This function sets aec in frame metadata
 *
 * Arguments:
 *   p_meta_list - list of frame metadata
 *   @stats_update: stats update event payload
 *   @current_frame_id: current frame id
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_aec(mct_list_t **p_meta_list,
  stats_update_t *stats_update, uint32_t current_frame_id);

/**
 * Function: module_imglib_common_meta_set_awb
 *
 * Description: This function sets awb in frame metadata
 *
 * Arguments:
 *   p_meta_list - list of frame metadata
 *   @stats_update: stats update event payload
 *   @current_frame_id: current frame id
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_awb(mct_list_t **p_meta_list,
  stats_update_t *stats_update, uint32_t current_frame_id);

/**
 * Function: module_imglib_common_meta_set_af
 *
 * Description: This function sets af in frame metadata
 *
 * Arguments:
 *   p_meta_list - list of frame metadata
 *   @stats_update: stats update event payload
 *   @current_frame_id: current frame id
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_af(mct_list_t **p_meta_list,
  stats_update_t *stats_update, uint32_t current_frame_id);

/**
 * Function: module_imglib_common_meta_set_gamma
 *
 * Description: This function sets gamma in frame metadata
 *
 * Arguments:
 *   p_meta_list - list of frame metadata
 *   @p_gamma: stats update event payload
 *   @current_frame_id: current frame id
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_gamma(mct_list_t **p_meta_list,
  void *p_gamma, uint32_t current_frame_id);

/**
 * Function: module_imglib_common_meta_set_fd_info
 *
 * Description: This function sets fd info in frame metadata
 *
 * Arguments:
 *   @comb_faces_data: combined fd info
 *   @p_meta: metadata
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_fd_info(
  cam_faces_data_t *comb_faces_data, img_meta_t *p_meta);

/**
 * Function: module_imglib_common_meta_set_rotation
 *
 * Description: This function sets rotation in frame metadata
 *
 * Arguments:
 *   @rotation: rotation
 *   @p_meta: metadata
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_rotation(cam_rotation_info_t *rotation,
  img_meta_t *p_meta);

/**
 * Function: module_imglib_common_meta_set_flip
 *
 * Description: This function sets flip in frame metadata
 *
 * Arguments:
 *   @flip_mask: snapshot flip value
 *   @p_meta: metadata
 *
 * Return values:
 *   standard image lib return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_flip(int32_t flip_mask,
  img_meta_t *p_meta);

/**
 * Function: module_imglib_common_meta_fd_info_apply_rotation
 *
 * Description: This function applies rotation on fd info in frame metadata
 *
 * Arguments:
 *   @face_detect_tilt_cut_off - maximum angle for face tilt filter
 *   @p_meta: metadata
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_fd_info_apply_rotation(
  int32_t face_detect_tilt_cut_off, img_meta_t *p_meta);

/**
 * Function: module_imglib_common_add_imglib_debug_meta
 *
 * Description: This function fills adds imglib debug metato in hal meta
 *
 * Arguments:
 *   @p_hal_meta_list: list of hal metadata
 *   @p_meta_list: list of frame metadata
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_add_imglib_debug_meta(mct_list_t *p_hal_meta_list,
  mct_list_t *p_meta_list);

/**
 * Function: module_imglib_common_get_prop
 *
 * Description: This function returns property value in 32-bit
 * integer
 *
 * Arguments:
 *   @prop_name: name of the property
 *   @def_val: default value of the property
 *
 * Return values:
 *    value of the property in 32-bit integer
 *
 * Notes: API will return 0 in case of error
 **/
int32_t module_imglib_common_get_prop(const char* prop_name,
  const char* def_val);

boolean mod_imglib_read_file(char* file_name, int file_count,
  img_mem_handle_t *p_buffer);

/**
 * Function: module_imglib_common_get_frame_fmt
 *
 * Description: Function used to convert camera type format to frame format.
 *
 * Arguments:
 *   @frame_fmt - Camera type format.
 *   @frame_fmt - Output frame foramt.
 *
 * Return values:
 * Bayerproc error code.
 *
 **/
int32_t module_imglib_common_get_frame_fmt(cam_format_t cam_fmt, img_format_t *frame_fmt);

/**
* Function: module_imglib_common_get_cam_fmt
*
* Description: Function used to convert frame format to camera format.
*
* Arguments:
*   @frame_fmt - Input bayerproc frame format.
*   @frame_fmt - Output converted camera format.
*
* Return values:
* Bayerproc error code.
*
**/
int32_t module_imglib_common_get_cam_fmt(img_format_t frame_fmt, cam_format_t *cam_fmt);

/**
 * Function: module_imglib_common_get_stride_in_pix
 *
 * Description: Function used to convert stride in pixels from bytes based
 *   on given format.
 *
 * Arguments:
 *   @s_bytes - Stride in bytes.
 *   @frame_fmt - Frame format.
 *
 * Return values:
 *   Stride in pixels. Zero if fail.
 *
 **/
uint32_t module_imglib_common_get_stride_in_pix(uint32_t s_bytes, img_format_t frame_fmt);

/**
  * Function: module_imglib_common_get_stride_in_bytes
  *
  * Description: Function used to convert stride from pixels to bytes based
  *   on given format.
  *
  * Arguments:
  *   @s_pix - Stride in pixels.
  *   @frame_fmt - Frame format.
  *
  * Return values:
  *   Stride in bytes. Zero if fail.
  *
  * Notes: This function will align and provide stride based on given pixels
**/
uint32_t module_imglib_common_get_stride_in_bytes(uint32_t s_pix,
  img_format_t frame_fmt);

/**
  * Function: module_imglib_common_get_black_level
  *
  * Description: This function is used extract black level from chromatix.
  *
  * Arguments:
  *   @p_client: BAYER_PROC_BASE client.
  *   @p_info: Prepare info struct where black level need to be filled.
  *
  * Return values:
  *     imaging error values
  *
  * Notes: none
  **/
uint32_t module_imglib_common_get_black_level(
  int *shift_bits, cam_format_t fmt);

/**
 * Function: module_imglib_common_get_color_format
 *
 * Description: This function translates color format from stream format
 *  to Venus color format
 *
 * Arguments:
 *   cam_fmt - Stream format
 *
 *
 * Return values:
 *     int - color format
 *
 * Notes: none
 **/
int module_imglib_common_get_venus_color_format(cam_format_t cam_fmt);

/**
 * Function: module_imglib_common_get_img_camera_role
 *
 * Description: This function translates camera role from
 *   HAL/MCT camera role to Imglib camera role
 *
 * Arguments:
 *   cam_role - Camera role (HAL/MCT format)
 *
 *
 * Return values:
 *     int - img camera role
 *
 * Notes: none
 **/
int module_imglib_common_get_img_camera_role(cam_dual_camera_role_t cam_role);

/**
 * Function: module_imglib_common_get_cam_role
 *
 * Description: This function translates camera role from
 *  Imglib camera role to HAL/MCT cam role format
 *
 * Arguments:
 *   img_cam_role - Camera role (Imglib format)
 *
 *
 * Return values:
 *     int - HAL/MCT camera role
 *
 * Notes: none
 **/
int module_imglib_common_get_cam_role(img_camera_role_t img_cam_role);

/**
 * Function: module_imglib_common_meta_set_aec_info
 *
 * Description: This function sets aec info in current metadata
 *
 * Arguments:
 *   @stats_update: aec info
 *   @p_meta: metadata
 *
 * Return values:
 *   standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_aec_info(stats_update_t *stats_update,
  img_meta_t *p_meta);

/**
 * Function: module_imglib_common_meta_set_awb_info
 *
 * Description: This function sets awb info in current metadata
 *
 * Arguments:
 *   @stats_update: awb info
 *   @p_meta: metadata
 *
 * Return values:
 *   standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_awb_info(stats_update_t *stats_update,
  img_meta_t *p_meta);

/**
 * Function: module_imglib_common_meta_set_gamma_info
 *
 * Description: This function sets gamma in metadata
 *
 * Arguments:
 *   @p_gamma: stats update event payload
 *   @p_meta: pointer to metadata
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_gamma_info(
  void *p_gamma, img_meta_t *p_meta);

/**
 * Function: module_imglib_common_fill_fov_params
 *
 * Description: This function is used to fill in the
 *   ISP and CAMIF crop/scale params
 *
 * Arguments:
 *   @sensor_max_dim - sensor dimensions
 *   @sensor_crop: sensor crop info
 *   @stream_crop_valid: whether stream crop info is valid
 *   @stream_crop: stream crop info
 *   @isp_output_dim_stream_info_valid: whether isp output dim info valid
 *   @isp_output_dim_stream_info: isp output dim info
 *   @p_fov_cfg_arr: fov config array
 *   @arr_size: fov config array size
 *
 * Return values:
 *     None
 *
 * Notes: none
 **/
void module_imglib_common_fill_fov_params(
  img_dim_t *sensor_max_dim,
  sensor_request_crop_t *sensor_crop,
  boolean stream_crop_valid,
  mct_bus_msg_stream_crop_t *stream_crop,
  boolean isp_output_dim_stream_info_valid,
  mct_stream_info_t *isp_output_dim_stream_info,
  cam_dimension_t *final_stream_dim_info,
  img_fov_t *p_fov_cfg_arr,
  int32_t arr_size);

#endif //__MODULE_IMGLIB_COMMON_H__
