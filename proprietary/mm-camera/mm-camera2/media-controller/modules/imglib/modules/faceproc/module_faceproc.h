/**********************************************************************
*  Copyright (c) 2014-2017, 2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __MODULE_FACEPROC_H__
#define __MODULE_FACEPROC_H__

#include "img_common.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "module_imglib_common.h"
#include "faceproc.h"
#include "camera_dbg.h"
#include "fd_chromatix.h"
#include "mct_pipeline.h"
#include "img_mem_ops.h"
#include "img_rb.h"
#include "img_fcv.h"
#include "module_faceproc_newstab.h"

#define IMGLIB_MOD_NAME "faceproc"
#define MAX_FD_STATIC_PORTS 4
#define MAX_NUM_FRAMES 20
#define MOD_FACEPROC_SYNC TRUE

/** MODULE_MASK:
 *
 * Mask to enable dynamic logging
 **/
#undef MODULE_MASK
#define MODULE_MASK IMGLIB_FACEPROC_SHIFT

/**
 * Maximum number of face stats
 */
#define MAX_FACE_STATS 5

#define MAX_FD_WIDTH 8192
#define MAX_FD_HEIGHT 8192

/**
 * Recommended analysis dimensions for hw FD while camera preview
 */
#define RECOMMENDED_ANALYSIS_WIDTH 640
#define RECOMMENDED_ANALYSIS_HEIGHT 480

/**
 * Recommended analysis dimensions for hw FD while video mode
 */
#define RECOMMENDED_ANALYSIS_VIDEO_WIDTH 640
#define RECOMMENDED_ANALYSIS_VIDEO_HEIGHT 480

#define FACEPROC_STAB_HISTORY 10

#define MAX_FD_STREAM 5


/** combined_cam_face_detection_data_t
 *   @fd_data: face info meta for face only
 *   @fp_data: info from facial parts/landmarks detection
 *   @cont_data: info from contour detection
 *   @blink_data: info from blink detection
 *   @smile_data: info from smile detection
 *   @gaze_data: info from  gaze detection
 *   @fr_data: info from face recognition
 *
 *   faceproc meta information together
 **/
typedef struct {
  cam_face_detection_data_t fd_data;
  cam_face_landmarks_data_t fp_data;
  cam_face_contour_data_t   cont_data;
  cam_face_blink_data_t     blink_data;
  cam_face_smile_data_t     smile_data;
  cam_face_gaze_data_t      gaze_data;
  cam_face_recog_data_t     fr_data;
} combined_cam_face_detection_data_t;

/** faceproc_history_entry_t
 *   @x: X coordinate
 *   @y: Y coordinate
 *
 *   Structure which holds face coordinates
 **/
typedef struct {
  uint32_t x;
  uint32_t y;
} faceproc_history_entry_t;

/** faceproc_history_state_t
 *   @FD_STAB_STATE_STABLE: State is stable old coordinates will be used
 *   @FD_STAB_STATE_UNSTABLE: State is unstable also old coordinates will be
 *     used
 *   @FD_STAB_STATE_STABILIZE: Stabilizing move to new position when is done
 *     it will switch tos table state
 *
 *   Enum for face stabilization states
 **/
typedef enum {
  FD_STAB_STATE_STABLE,
  FD_STAB_STATE_UNSTABLE,
  FD_STAB_STATE_STABILIZE,
} faceproc_history_state_t;

/** faceproc_history_holder_t
 *   @index: Current active face index in the history
 *   @history_size: History size
 *   @faces_inside: Faces inside the history
 *   @state_count: Count frames per current state
 *   @max_state_count: Max state can be active (current valid only for unstable
 *     state)
 *   @state: Current state
 *   @stable_entry: Current stable entry
 *   @stable_refer: Stable reference if stabilization is used with reference
 *   @entry: Stabilization history entries
 *
 *   Structure which holds face entry stabilization parameters
 **/
typedef struct {
  uint32_t index;
  uint32_t history_size;
  uint32_t faces_inside;
  uint32_t state_count;
  uint32_t max_state_count;
  faceproc_history_state_t state;
  faceproc_history_entry_t stable_entry;
  faceproc_history_entry_t stable_refer;
  faceproc_history_entry_t entry[FACEPROC_STAB_HISTORY];
} faceproc_history_holder_t;

/** faceproc_faces_history_t
 *   @id: FAce unique ID
 *   @face_size: Face size history holder
 *   @face_position: Face position history holder
 *   @mouth_position: Mouth position history holder
 *   @smile_degree: Smile degree history holder
 *
 *   Structure which holds face detection stabilization history parameters
 **/
typedef struct {
    uint32_t id;
    faceproc_history_holder_t face_size;
    faceproc_history_holder_t face_position;
    faceproc_history_holder_t mouth_position;
    faceproc_history_holder_t smile_degree;
} faceproc_faces_history_t;

/** faceproc_stabilization_t
 *   @faces_detected: Faces detected in the history
 *   @faces: Array of faces history for stabilization
 *
 *   Structure which holds face detection stabilization parameters
 **/
typedef struct {
  uint32_t detected_faces;
  faceproc_faces_history_t faces[MAX_FACES_TO_DETECT * 2];
  // We should have double history to handle the sorting
} faceproc_stabilization_t;

/** faceproc_engine_comp_priority_t
 *    @FACEPROC_ENGINE_COMP_PRIMARY: highest pri comp to use if
 *      available
 *    @FACEPROC_ENGINE_COMP_SECONDARY: second highest pri comp to
 *      use if available
 *    @FACEPROC_ENGINE_COMP_TERTIARY: tertiary component
 *    @FACEPROC_ENGINE_COMP_MAX: max number of components
 *      present
 *   faceproc_engine_comp_priority_t
 **/
typedef enum {
  FACEPROC_ENGINE_COMP_PRIMARY,
  FACEPROC_ENGINE_COMP_SECONDARY,
  FACEPROC_ENGINE_COMP_TERTIARY,
  FACEPROC_ENGINE_COMP_MAX
} faceproc_engine_comp_priority_t;

/** faceproc_input_dim_t
 *   @width: Faceproc input width.
 *   @height: Faceproc input height.
 *   @stride: Faceproc input stride.
 *   @scanline: Faceproc input scanline.
 *
 *   Faceproc input dimension.
 **/
typedef struct {
  uint32_t width;
  uint32_t height;
  uint32_t stride;
  uint32_t scanline;
} faceproc_input_dim_t;

/** faceproc_stream_t
 *   @stream_info: pointer to hold the stream info
 *   @p_sink_port: Mct sink port attached to this stream
 *   @isp_output_dim: dimension of the input buffer at FD module
 *   @input_dim: final stream dimensions
 *   @input_fmt: Input buffer format
 *   @input_trans_info: Input buffer translation info
 *   @camif_out_trans_info: Translation info to camif coordinates
 *   @identity: identity of the stream
 *   @buffer_info: buffer info from the stream
 *   @mapped: flag to indicate whether the buffers are mapped
 *   @streamon: flag to indicated whether the streamon is active
 *   @out_trans_info: translation w.r.t the stream crop
 *   @crop_info: crop info
 *   @processing_stream: This is processing stream.
 *   @reporting_stream: This is reporting stream.
 *   @use_int_bufs: Set if internal buffers need to be used.
 *   @buff_configured: flag indicating if buffers were
 *                   configured.
 *   @divert_buff_requested: Buffer divert requested flag.
 *
 *   Structure which holds face detection stream based info
 **/
typedef struct {
  mct_stream_info_t *stream_info;
  mct_port_t *p_sink_port;
  faceproc_input_dim_t isp_output_dim;
  faceproc_input_dim_t input_dim;
  cam_format_t input_fmt;
  img_trans_info_t input_trans_info;
  img_trans_info_t camif_out_trans_info;
  uint32_t identity;
  mod_img_buffer_info_t buffer_info;
  int8_t mapped;
  int8_t streamon;
  img_trans_info_t out_trans_info;
  img_rect_t crop_info;
  boolean processing_stream;
  boolean reporting_stream;
  boolean use_int_bufs;
  int8_t buff_configured;
  float width_margin;
  float height_margin;
  uint32_t buff_divert_requested;
} faceproc_stream_t;

/** faceproc_divert_holder_t
 *   @locked: Structure is locked buffer divert pending.
 *   @return_ack: True if buffer divert ack need to be sent.
 *   @msg: Buffer divert message.
 *   @mutex: Mutex protecting this structure.
 *
 *   Structure hold buffer divert information.
 **/
typedef struct {
  boolean locked;
  boolean return_ack;
  mod_img_msg_buf_divert_t msg;
  pthread_mutex_t mutex;
} faceproc_divert_holder_t;

/** faceproc_low_light_t
 *   @low_light_scene: True if scene is low light.
 *   @last_lux_index: Last lux index.
 *   Structure hold low light scene information.
 **/
typedef struct {
  boolean low_light_scene;
  float last_lux_index;
} faceproc_low_light_t;

/** fd_score_t
 *
 *   Enum to specify face score
 **/
typedef enum {
  FD_SCORE_LOW,
  FD_SCORE_SAME,
  FD_SCORE_HIGH,
} fd_score_t;

/** faceproc_low_light_t
 *  @mfi_last_updated: store the last updated face info
 *  @mfi_prev: store the previous face info
 *  @hold_cnt: filter hold count
 *  @locked: flag to indicate whether the face is locked
 *
 *   Structure hold low light scene information.
 **/
typedef struct {
  mct_face_info_t mfi_last_updated;
  mct_face_info_t mfi_prev;
  uint32_t hold_cnt;
  uint32_t ui_hold_cnt;
  bool locked;
} face_stats_filter_t;

/** faceproc_client_t
 *   @mutex: client lock
 *   @cond: conditional variable for the client
 *   @threadid: pthread id if the client is running is async mode
 *   @comps: component ops structure
 *   @current_comp_idx: component index that is being used currently
 *   @curr_comp_connection_lost: whether the current component's connection
 *     is valid or not
 *   @engine: Faceproc engine used.
 *   @new_engine: variable to store the new engine type
 *   @mode: facial processing mode
 *   @config: structure to hold the facial processing configuration
 *   @result: structure to hold the result of face detection
 *   @state: state of face detection client
 *   @is_ready: flag for synchronizing the thread start
 *   @status: integer to hold the asynchronous results
 *   @sync: synchronous/asynchrnous mode
 *   @main_dim: variable to store the sensor dimension
 *   @zoom_val: zoom value provided by the UI
 *   @prev_face_count: face count for the previous iteration
 *   @buf_idx: current buffer index
 *   @active: flag to indicate whether the client is active
 *   @p_mod: module pointer
 *   @mutex: mutex to synchronize the result
 *   @num_frames_processed_in_no_skip_cnt : Number of frames that FD could
 *     process in first p_fd_chromatix->initial_frame_no_skip_cnt frames.
 *   @p_fd_chromatix: FD chromatix pointer
 *   @stabilization: Face detection stabilization function
 *   @newstabilization: New Face detection stabilization function
 *   @p_fpc_q: queue for per frame control
 *   @fpc_q_size: size of the frame control queue
 *   @ref_count: ref count for the number of streams
 *   @def_id: Client definition id.
 *   @fp_stream: faceproc stream
 *   @frame: array for frame buffers
 *   @fd_async: flag to indicate whether the buffer handling
 *            should be sync or async
 *   @fpc_mode: mode used for reporting the metadata
 *   @current_frame_cnt: frame count since the begining of FD enable
 *   @faceproc_mem_handle: Array holding internal frames memory allocations
 *   @divert_holder: Array holding buffer divert structures.
 *   @streamid_mask: mask for all stream ids associated with the client
 *   @set_mode: booloean flag indicating whether new mode is set
 *   @new_mode: new faceproc_mode_t mode
 *   @selected_size: Selected size based on stream config
 *   @fd_apply_mask: Mask indiacating if FD has to be applied
 *   @fd_post_mask: Mask indicating if fd results have to be posted
 *   @event_identity: Current Event identity
 *   @use_device_orientation: Set if device orientation will be used
 *   @use_sw_assisted_fd: Flag indicating to use sw assisted fd.
 *   @alloc_int_bufs: Flag set if internal buffers need to be allocated.
 *   @low_light: Structure containing low light scene information.
 *   @multi_client: Multi client mode is enabled.
 *   @dump_mode: Frame dump mode.
 *   @flip_mask: flip value
 *   @enable_bsgc: Enable blink, smile, gaze, contour detection
 *   @use_video_chromatix: whether to use video specific chromatix versions
 *   @use_fd_lite_for_front_camera: whether to use lite chromatix versions
       for front camera
 *   @last_stats: store the last face stats
 *   @result_rb: result ring buffer
 *   @face_hist_enable: flag to indicate whether the face
 *                    histogram needs to be enabled
 *   @histogram: face histogram data
 *   @force_enable_region_based_filtering: Whether to enable
 *       region based filtering based on setprop irrespective of
 *       the configuration in chromatix
 *   @fd_ui_mode: face detect UI mode ON/OFF ROI update to UI
 *      HAL
 *   @num_comp: number of active components for faceproc
 *   @video_mode: whether running in video
 *   @debug_settings: FD specific debug settings
 *   @fd_profile: FD profile information
 *   @timeval first_frame_time_stamp: time stamp of first FD frame processed
 *   @gravity_info : gravity sensor information
 *   @turbo_mode : indicates whether FD has to run in turbo mode
 *   @sensor_mount_angle : current sensor mount angle
 *   @device_orientation current device orientation
 *   @dynamic_cfg_params: Holds dynamically configurable parameters
 *   @new_stab: to use new or old stabilization filter
 *
 *   Faceproc client structure
 **/
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_t threadid;
  img_component_ops_t comps[FACEPROC_ENGINE_MAX];
  boolean curr_comp_connection_lost;
  faceproc_engine_t engine;
  faceproc_engine_t new_engine;
  faceproc_mode_t mode;
  faceproc_config_t config;
  faceproc_result_t result[MAX_FACE_STATS];
  imglib_state_t state;
  int is_ready;
  int status;
  int sync;
  img_size_t main_dim;
  cam_zoom_info_t zoom_val;
  int8_t prev_face_count;
  int buf_idx;
  int buf_idx_offline;
  int buf_idx_min_try;
  int trial_on;
  int8_t active;
  void *p_mod;
  pthread_mutex_t result_mutex;
  uint32_t num_frames_processed_in_no_skip_cnt;
  fd_chromatix_t *p_fd_chromatix;
  faceproc_stabilization_t stabilization;
  StabilizationHistory newstabilization;
  int32_t hal_version;
  img_queue_t *p_fpc_q;
  uint32_t fpc_q_size;
  uint32_t ref_count;
  int def_id;
  faceproc_stream_t fp_stream[MAX_FD_STREAM];
  img_frame_t frame[MAX_NUM_FD_FRAMES];
  int8_t fd_async;
  faceproc_mode_t fpc_mode;
  uint32_t current_frame_cnt;
  img_mem_handle_t faceproc_mem_handle[MAX_NUM_FD_FRAMES];
  faceproc_divert_holder_t divert_holder[MAX_NUM_FD_FRAMES];
  uint32_t streamid_mask;
  int8_t set_mode;
  faceproc_mode_t new_mode;
  faceproc_frame_cfg_t selected_size;
  uint16_t fd_apply_mask;
  uint16_t fd_post_mask;
  uint32_t event_identity;
  boolean use_device_orientation;
  boolean use_sw_assisted_fd;
  boolean multi_client;
  boolean alloc_int_bufs;
  faceproc_low_light_t low_light;
  faceproc_dump_mode_t dump_mode;
  faceproc_batch_mode_info_t frame_batch_info;
  uint32_t flip_mask;
  int8_t enable_bsgc;
  boolean use_video_chromatix;
  boolean use_fd_lite_for_front_camera;
  combined_cam_face_detection_data_t last_stats;
  img_rb_t result_rb;
  bool face_hist_enable;
  mct_face_hist_t histogram[MAX_FACE_STATS][MAX_ROI];
  int8_t force_enable_region_based_filtering;
  cam_face_detect_mode_t fd_ui_mode;
  face_stats_filter_t stats_filter;
  uint32_t num_comp;
  uint32_t offline_buf_count;
  img_mem_handle_t faceproc_offline_bufs[MAX_NUM_OFFLINE_FD_FRAMES];
  bool use_old_stats;
  boolean video_mode;
  faceproc_debug_settings_t debug_settings;
  img_profiling_t fd_profile;
  struct timeval first_frame_time_stamp;
  img_gravity_info_t gravity_info;
  boolean turbo_mode;
  uint32_t sensor_mount_angle;
  int32_t device_orientation;
  faceproc_dynamic_cfg_params_t dynamic_cfg_params;
  boolean new_stab;
} faceproc_client_t;

/** faceproc_session_params_t
 *   @session_id: Session id for which parameters are stored
 *   @valid_params: Valid parameters Yes/No
 *   @fd_enable: Face detection enable
 *   @fr_enable: Face recognition enable
 *   @zoom_val: zoom value provided by the UI
 *
 *   Structure which holds session based parameters
 **/
typedef struct {
  uint32_t session_id;
  boolean valid_params;
  struct {
    boolean fd_enable;
    boolean fr_enable;
    cam_zoom_info_t zoom_val;
  } param;
} faceproc_session_params_t;

/** module_faceproc_t
 *   @client_cnt: Variable to hold the number of faceproc
 *              clients
 *   @module_type: Hold last updated module type
 *   @mutex: client lock
 *   @cond: conditional variable for the client
 *   @threadid: pthread id if the client is running is async
 *            mode
 *   @comp: core operation structure
 *   @lib_ref_count: reference count for faceproc library access
 *   @fp_client: list of faceproc clients
 *   @session_parms: List of session based parameters
 *   @msg_thread: message thread
 *   @active: flag to indicate whether the client is active
 *   @session_data: session data for per frame control
 *   @fd_preview_size: Analysis frame size required in camera preview mode
 *   @fd_video_size: Analysis frame size required in video mode
 *   @engine: Faceproc engine used.
 *   @core_ops: pointer to base img component ops used by module
 *   @session_cnt: Number of active sessions
 *   @p_fcv_hdl: Pointer to fastcv lib handle
 *   @ion_fd: ION fd
 *   @force_cache_op: Whether to do force cache operation
 *
 *   Faceproc module structure
 **/
typedef struct {
  int client_cnt;
  mct_module_type_t module_type;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  img_core_ops_t core_ops[FACEPROC_ENGINE_MAX];
  int lib_ref_count;
  mct_list_t *fp_client;
  mct_list_t *session_parms;
  mod_imglib_msg_th_t msg_thread;
  int8_t active;
  mct_pipeline_session_data_t session_data;
  faceproc_frame_cfg_t fd_preview_size;
  faceproc_frame_cfg_t fd_video_size;
  faceproc_engine_t engine;
  int32_t session_cnt;
  img_fcv_lib_handle_t *p_fcv_hdl;
  uint32_t ion_fd;
  bool force_cache_op;
} module_faceproc_t;

/** fd_pfc_data_type_t
 *  @MOD_FD_FPC_MODE: face detection mode
 *  @MOD_FD_FPC_SCENE_MODE: Face priority scene mode
 *  @MOD_FD_FPC_GEN: generic msg
 *
 *   This enumeration is for per frame control message type.
 *   Do not change the order of the enum
 **/
typedef enum {
  MOD_FD_FPC_MODE = 0,
  MOD_FD_FPC_SCENE_MODE,
  MOD_FD_FPC_GEN,
} fd_pfc_data_type_t;

/** fd_pfc_data_t
 *  @ctrl_type: per frame ctrl type
 *  @data_type: per frame ctrl data type
 *  @d: payload
 *
 *   This structure represents the message for per frame control
 **/
typedef struct {
  imglib_pfc_ctrl_type_t ctrl_type;
  fd_pfc_data_type_t data_type;
  union {
    faceproc_mode_t mode;
  }d;
} fd_pfc_data_t;

/**
 * FACEPROC client APIs
 **/
int module_faceproc_client_create(mct_module_t *p_mct_mod, mct_port_t *p_port,
  uint32_t identity, mct_stream_info_t *stream_info);

int module_faceproc_client_start(faceproc_client_t *p_client);

int module_faceproc_client_stop(faceproc_client_t *p_client, uint32_t identity);

void module_faceproc_client_destroy(faceproc_client_t *p_client, uint32_t identity);

int module_faceproc_client_set_mode(faceproc_client_t *p_client,
  faceproc_mode_t mode,
  int8_t lock);

int module_faceproc_client_configure_buffers(faceproc_client_t *,
  mct_stream_info_t *stream_info, int s_idx);

int module_faceproc_client_map_buffers(faceproc_client_t *,
  int s_idx);

boolean module_faceproc_client_handle_buf_divert(faceproc_client_t *p_client,
  mod_img_msg_buf_divert_t *p_msg_divert);

int module_faceproc_client_process_buffers(faceproc_client_t *p_client);

int module_faceproc_client_unmap_buffers(faceproc_client_t *p_client,
  uint32_t identity);

int module_faceproc_client_handle_ctrl_parm(faceproc_client_t *p_client,
  mct_event_control_parm_t *param,
  int frame_id);

int module_faceproc_client_set_offset_onlux(faceproc_client_t *p_client,
  float lux_value);

int module_faceproc_client_set_lux_idx(faceproc_client_t *p_client, float idx);

int module_faceproc_client_set_scale_ratio(faceproc_client_t *p_client,
  mct_bus_msg_stream_crop_t *stream_crop, int s_idx);

int module_faceproc_client_fill_fd_caps(faceproc_client_t *p_client,
  faceproc_capbility_aec_t *fd_capability);

int module_faceproc_client_set_default_scale_ratio(faceproc_client_t *p_client,
  mct_stream_info_t *p_stream_info, int s_idx);

int module_faceproc_handle_streamon(module_faceproc_t *p_mod,
  faceproc_client_t *p_client, uint32_t identity);

int module_faceproc_handle_streamoff(module_faceproc_t *p_mod,
  faceproc_client_t *p_client, uint32_t identity);

int module_faceproc_faces_stabilization(faceproc_stabilization_t *p_stab,
  fd_chromatix_t *p_fd_chromatix, img_size_t *p_frame_dim,
  faceproc_result_t *p_result);

int module_faceproc_client_handle_sof(faceproc_client_t *p_client,
  unsigned int frame_id);

void module_faceproc_post_bus_msg(mct_module_t *p_mct_mod,
  unsigned int sessionid, mct_bus_msg_type_t msg_id, void *msg_data,
  uint32_t msg_size);

int module_faceproc_client_add_stream(faceproc_client_t *p_client,
  mct_stream_info_t *p_stream_info, mct_port_t *p_port);

int module_faceproc_client_remove_stream(faceproc_client_t *p_client,
  unsigned int identity);

int faceproc_get_stream_by_id(faceproc_client_t *p_client,
  uint32_t identity);

int faceproc_get_stream_by_type(faceproc_client_t *p_client,
  cam_stream_type_t type);

int module_faceproc_client_flush(void *p_nodedata, void *p_userdata);

void module_faceproc_client_buf_divert_exec(void *userdata, void *data);

int module_faceproc_client_comp_create(faceproc_client_t *p_client);

int module_faceproc_client_comp_destroy(faceproc_client_t *p_client);

int module_faceproc_client_set_rotation(faceproc_client_t *p_client,
  int32_t rotation);

void module_faceproc_client_apply_flip_on_orientation(
  uint32_t flip_mask, int32_t *p_orientation_angle);

#endif //__MODULE_FACEPROC_H__
