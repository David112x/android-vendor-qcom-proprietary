/**********************************************************************
*  Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __MODULE_IMGLIB_BASE_H__
#define __MODULE_IMGLIB_BASE_H__

#include "img_common.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "module_imgbase.h"
#include "module_imglib_common.h"
#include "camera_dbg.h"
#include "modules.h"
#include "mct_pipeline.h"
#include "img_thread.h"
#include "img_mem_ops.h"
#include "img_buffer.h"
#include "img_dsp_dl_mgr.h"

#define MAX_IMGLIB_BASE_STATIC_PORTS 6
#define MAX_IMGLIB_BASE_MAX_STREAM 4
#define MAX_IMGLIB_BASE_RELEASED_BUFS 10
#define MAX_IMGLIB_BATCH_SIZE 20
#define MAX_BAYER_PROC_NATIVE_BUF_NUM 10

#define MAX_BAYER_PROC_BASE_STATIC_PORTS 4
#define MAX_BAYER_PROC_BASE_MAX_STREAM 5
#define MAX_BAYER_PROC_BASE_RELEASED_BUFS 10
#define MAX_BAYER_PROC_BATCH_SIZE 20
#define MAX_BAYER_PROC_MULTI_FRAME_SIZE 5
#define MAX_BAYER_PROC_NATIVE_BUF_NUM 10
#define BAYER_PROC_DEF_SENSOR_BLACK_LEVEL 65

/**
 * Maximun count of internal buffers
 */
#define DEFAULT_IMGLIB_INTERNAL_BUFS 4
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
#define MAX_IMGLIB_INTERNAL_BUFS 80
#else
#define MAX_IMGLIB_INTERNAL_BUFS 20
#endif
#define MAX_IMGLIB_SESSIONS 8

/** MODULE_MASK:
 *
 * Mask to enable dynamic logging
 **/
#undef MODULE_MASK
#define MODULE_MASK IMGLIB_BASE_SHIFT

/**
 *Is 4k macro is only applicable for modules before CPP
 */
#define WIDTH_4K 3840
#define HEIGHT_4K 2160
#define IS_4k_VIDEO(stream, width, height, mask) \
  (((stream == CAM_STREAM_TYPE_VIDEO) && \
    (((width >= WIDTH_4K) && (height >= HEIGHT_4K)) || \
    (((width >= HEIGHT_4K) && (height >= WIDTH_4K)) && \
    (mask & CAM_QCOM_FEATURE_ROTATION)))) \
    ? 1 : 0 \
    )

/*Queue size to hold meta_stream_id requests*/
#define PER_FRAME_QUEUE_SIZE 64

/** imgbase_frame_config_t
 *   @frame_id: frame id
 *   @identity: identity
 *   @config_data: config data
 *
 *   imglib frame config
 **/
typedef struct {
  uint32_t frame_id;
  uint32_t identity;
  mct_event_control_parm_t config_data;
} imgbase_frame_config_t;

/** imgbase_preload_t:
 *
 * @param: params that need to be passed to preload op
 * @preload_done: Flag to indicate whether the preload is
 *              completed.
 *
 *  Structure to hold preload info
 */
typedef struct {
  img_preload_param_t param;
  bool preload_done;
} imgbase_preload_t;


/** imgbase_multi_camera_info_t:
 *
 * @peer_session_ids: Identities of the peer session's session
 *                  stream
 * @num_peer_sessions: Number of peer sessions
 * @cam_type: Session type -
 *                           Main/Aux/Standalone
 * @cam_mode: Driving camera of the related camera sub-system
 *      (Primary/Secondary)
 * @low_power_mode: Is low power mode enabled for the session
 * @is_binded: Indicates if the peer sessions are linked
 * @cam_role: Camera role (wide/tele,etc)
 * @perf_mode: LPM mode (sync/async,etc)
 * @cam_fallback_mode: cam fallback mode
 *
 *  Structure to hold related camera info in a multi camera
 *  system
 */
typedef struct {
  uint32_t peer_session_ids[MAX_IMGLIB_SESSIONS -1];
  int num_peer_sessions;
  cam_sync_type_t cam_type;
  cam_sync_mode_t cam_mode;
  bool low_power_mode;
  boolean is_linked;
  cam_dual_camera_role_t cam_role;
  cam_dual_camera_perf_mode_t perf_mode;
  cam_fallback_mode_t cam_fallback_mode;
} imgbase_multi_camera_info_t;

/**imgbase_framedrop_t
 *
 *@frameid : frame drop id
 *
 */
typedef struct {
  uint32_t frameid;
} imgbase_framedrop_t;

/** imgbase_session_data_t
 *   @sessionid: MCT session identity
 *   @max_apply_delay: max apply delay
 *   @max_reporting_delay: max reporting delay
 *   @zoom_ratio_tbl_cnt: zoom table entries count
 *   @zoom_ratio_tbl: zoom ratio table from isp cap
 *   @private_data: private sessions data
 *   @stream_on_cnt: streamon counter
 *   @frame_config_q: frame config queue
 *   @q_mutex: lock
 *   @preload_params: parameters needed for preload
 *   @p_mod: Ptr to the base module
 *   @multi_cam_info: Multi Camera info
 *   @sensor_max_dim: sensor dimension info
 *   @sensor_crop: sensor crop info
 *   @session_meta: common meta data for the current session
 *   @stream_id_requested: stream id requested
 *
 *   IMGLIB_BASE session structure
 **/

typedef struct {
  uint32_t sessionid;
  uint32_t max_apply_delay;
  uint32_t max_reporting_delay;
  size_t zoom_ratio_tbl_cnt;
  uint32_t zoom_ratio_tbl[MAX_ZOOMS_CNT];
  void *private_data;
  uint32_t stream_on_cnt;
  mct_queue_t frame_config_q;
  pthread_mutex_t q_mutex;
  imgbase_preload_t preload_params;
  void *p_mod;
  imgbase_multi_camera_info_t multi_cam_info;
  float pix_size;
  float focal_length;
  img_dim_t sensor_max_dim;
  sensor_request_crop_t sensor_crop;
  img_queue_t framedrop_hold_q;
  img_meta_t session_meta;
  cam_stream_ID_t stream_id_requested[PER_FRAME_QUEUE_SIZE];
  uint32_t master_info[PER_FRAME_QUEUE_SIZE];
} imgbase_session_data_t;

/** img_ack_event_t
 *  IMG_EVT_ACK_HOLD: event called when a new ACK is received by
 *    imglib and needs to be held (not forwarded)
 *  IMG_EVT_ACK_TRY_RELEASE: event called when a frame is no
 *    longer needed for batch processing and the ACK can be
 *    released
 *  IMG_EVT_ACK_FLUSH: event called to flush any outstanding
 *    acks from the ack list
 *  IMG_EVT_ACK_FORCE_RELEASE: event called to generate an ack
 *    and forward it upstream
 *  IMG_EVT_ACK_FREE_INTERNAL_BUF: event called upon receiving
 *    ack to free any internally allocated buffers
 *
 *  ACK event types
 **/
typedef enum {
  IMG_EVT_ACK_HOLD,
  IMG_EVT_ACK_TRY_RELEASE,
  IMG_EVT_ACK_FLUSH,
  IMG_EVT_ACK_FORCE_RELEASE,
  IMG_EVT_ACK_FREE_INTERNAL_BUF,
} img_ack_event_t;

/** imgbasebuf_ack_t
 *   @frame_id: ack frameid
 *   @identity: ack identity
 *   @divert_ack: divert ack struct
 *   @ack_received_cnt: number of acks received. acks can only
 *     be released once it equals num_ack in caps
 *
 *   IMGLIB_BASE ack structure
 **/
typedef struct {
  uint32_t frame_id;
  uint32_t identity;
  isp_buf_divert_ack_t divert_ack;
  int32_t ack_received_cnt;
} imgbasebuf_ack_t;

/** imgbase_buf_t
 *   @frame: frame info
 *   @buf_divert: ISP buffer divert info
 *   @queued_cnt: number of times frame was queued to
 *     component for algo processing
 *   @dequeued_cnt: number of times frame was dequeued from
 *     component after algo processing
 *   @divert_done: flag to indicate if buffer was forwarded
 *     downstream
 *   @ack: ack info associated with this buf frame
 *
 *   IMGLIB_BASE buffer structure
 **/
typedef struct {
  img_frame_t frame;
  isp_buf_divert_t buf_divert;
  int32_t queued_cnt;
  int32_t dequeued_cnt;
  uint32_t divert_done;
  imgbasebuf_ack_t ack;
  uint32_t event_identity;
  uint32_t put_buf;
} imgbase_buf_t;

/** imgbase_stream_t
 *   @identity: MCT session/stream identity
 *   @p_sinkport: sink port associated with the client
 *   @p_srcport: source port associated with the client
 *   @orig_stream_info: orignal stream info, should not be
 *     updated
 *   @stream_info: stream information
 *   @isp_stream_info: ISP stream info
 *   @cur_frame_id: current frameid
 *   @frames_pending_for_ack: no.of frames that are
 *     being processed currently and pending ack
 *   @streamon : stream on/off status
 *
 *   IMGLIB_BASE stream structure
 **/
typedef struct {
  uint32_t identity;
  mct_port_t *p_sinkport;
  mct_port_t *p_srcport;
  mct_stream_info_t *orig_stream_info;
  mct_stream_info_t *stream_info;
  mct_stream_info_t isp_stream_info;
  img_eis_config_t eis_config;
  uint32_t cur_frame_id;
  uint32_t frames_pending_for_ack;
  bool streamon;
} imgbase_stream_t;

/** img_internal_bufnode_t
 *  @mem_handle: memory handle
 *  @is_init: indicates if memory handle is valid
 *  @ref_cnt: reference count, 0 = internal buf is free
 *  @input_ack_held: indicates if input buf was acked
 *    1 = yet to be acked, 0 = ack done
 *  @p_frame: pointer to the image frame
 *  @exists_in_cache: whether this memory exists in cache
 *
 *  IMGLIB_BASE internal buffer structure
 */
typedef struct {
  img_mem_handle_t mem_handle;
  int8_t is_init;
  uint8_t ref_cnt;
  uint8_t input_ack_held;
  img_frame_t *p_frame;
  int8_t exists_in_cache;
} img_internal_bufnode_t;

/** img_internal_buflist_t
 *  @buf: imgbase internal buffers
 *  @buf_cnt: internal buffer count
 *  @type: internal buffer type
 *  @frame_id: Curent frame ID of the internal buffer
 *
 *  IMGLIB_BASE internal buffer list
 */
typedef struct {
  img_internal_bufnode_t buf[MAX_IMGLIB_INTERNAL_BUFS];
  uint32_t buf_cnt;
  img_buf_type_t type;
  uint32_t frame_id;
} img_internal_buflist_t;

/** imgbase_client_t
 *   @mutex: client lock
 *   @cond: conditional variable for the client
 *   @comp: component ops structure
 *   @frame: frame info from the module
 *   @state: state of face detection client
 *   @frame: array of image frames
 *   @parent_mod: pointer to the parent module
 *   @stream_on: Flag to indicate whether streamon is called
 *   @p_mod: pointer to the module
 *   @mode: IMBLIB mode
 *   @cur_buf_cnt: current buffer count
 *   @caps: imglib capabilities
 *   @current_meta: current meta data
 *   @stream_parm_q: stream param queue
 *   @frame_id: frame id
 *   @p_current_meta: pointer to current meta
 *   @num_meta_queued: count of the num meta queued and dequed
 *   @p_current_misc_data: pointer to current miscellaneous buffer
 *   @cur_index: current stream index
 *   @rate_control: control the rate of frames
 *   @start_time: start time for profiling
 *   @end_time: end time for profiling
 *   @first_frame: flag to indicate whether first frame is
 *               received
 *   @exp_frame_delay: expected frame delay
 *   @ion_fd: ION file descriptor
 *   @dis_enable: digital image stabilization enable flag set from HAL
 *   @is_update_valid: image stabilization valid data flag
 *   @is_update: image stabilization data
 *   @stream_crop_valid: isp_output_dim_stream_info_valid
 *   @stream_crop: stream crop data
 *   @isp_output_dim_stream_info_valid: isp output dim stream info valid flag
 *   @isp_output_dim_stream_info: isp output dim stream info
 *   @isp_extra_native_buf: num of extra native bufs needed
 *   @isp_native_buf_alignment: specifies native buffer alignemnts in bytes
 *   @before_cpp: indicates if base module is before cpp
 *   @is_dim_change: indicates if module changes dimensions
 *   @bypass_enable: bypass frames.
 *   @output_identity: event idx that buf divert is to sent on
 *   @divert_identity: event idx that buf divert is received on
 *   @divert_mask: curr divert mask if base module is after cpp
 *   @p_current_misc_data: pointer to current misc data
 *   @stream_mask: mask of all streams associated with client
 *   @streams_to_process: stream type that module should be
 *     enabled on. If 0, module should process all streams
 *   @processing_mask: mask of all the stream identity that
 *     module should be enabled on
 *   @output_stream_mask: Mask of all streams it can output
 *   @meta_data_list: list of frame metadata
 *   @hal_meta_data_list: list of hal metadata
 *   @feature_mask: feature mask for module
 *   @set_preferred_mapping: flag to indicate if special
 *     stream-to-port mapping is required from ISP
 *   @preferred_mapping_single: if set_preferred_mapping is enabled, this struct
 *     determines how ISP maps streams to a port for single processing stream
 *   @preferred_mapping_multi: if set_preferred_mapping is enabled, this struct
 *     determines how ISP maps streams to a port for multi processing stream
 *   @input_frame_id: frameIds of all the input bufs
 *   @release_buf_cnt: count of the buffers released to HAL
 *   @p_peer_comp: pointer to peer component ops
 *   @is_binded: flag to indicate if the peer compoents are binded
 *   @p_intra_port: pointer to intra port
 *   @plane_type: plane type
 *   @p_private_data: pointer to private data
 *   @thread_ops: thread operations table
 *   @thread_job: params for schedule thread manager job
 *   @buf_mutex: overlap buf lock
 *   @dummy_frame: dummy frame with null values which can be
 *     queued to component
 *   @buf_list: list of buf diverts which come into imglib
 *   @buf_list_idx: index within buf_list indicating the
 *     next available position to save a new buf
 *   @num_ack: number of ACKs to be received for each frame,
 *     determined by the number of src ports per sink port
 *   @native_buffs_map: array of stream mapped native buffers
 *   @native_buffer_list: lits of native buffer pointers
 *   @internal_buf: imgbase internal buffer list
 *   @process_all_frames: true if all bufs should be processed,
 *     false if only buffers with request should be processed
 *   @p_current_frame: pointer to hold the current frame. The
 *                   scope of the frame will be hold until the
 *                   next frame is obtained
 *   @session_id: session id of the client
 *   @effect: camera effect mode
 *   @event_identity: event idx that buf divert is received on
 *   @session_client: flag to indicate whether the client is
 *                  session based client
 *   @p_current_buf_div: holds current buf divert pointer
 *   @comp_init_params: component init parameters
 *   @async_init: asynchronous initialization
 *   @last_job_id: last job identity
 *   @processing_disabled: Flag to indicate if the processing is
 *     disbaled. Each module can have its own condition to
 *     enable/disable. By default processing_disabled = 0
 *   @lock_dyn_update: flag to indicate whether to lock the
 *                     dynamic updates
 *   @crop_required: If a module is before cpp, but needs
 *     updated dimensions from stream_crop, this flag needs to
 *     be set.
 *   @p_peer_client: peer client handle,if exists
 *   @meta_hold_q: Queue to hold the meta. This is applicable only
 *                 when hold_meta flag is set in img_caps_t. If the
 *                 module extn set this flag, the extended module
 *                 should do the meta buffer queuing to component.
 *   @out_stream_info: Output stream dim info.
 *   @is_bayer: Is this a bayer processing module.
 *   @m_outbuf2comp_q: queue for unfilled output bufs to be given
 *      to component on next putbuf done.
 *   @curr_outputbuf_count: number of output bufs with allocated
 *      buffer.
 *   @defer_intbuff_alloc: flag to disable allocation of internal
 *     buffers.
 *
 *   IMGLIB_BASE client structure
 **/
typedef struct {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  img_component_ops_t comp;
  int state;
  mct_module_t *parent_mod;
  uint8_t stream_on;
  void *p_mod;
  img_comp_mode_t mode;
  int cur_buf_cnt;
  img_caps_t caps;
  img_meta_t current_meta;
  img_queue_t stream_parm_q;
  int frame_id;
  img_meta_t *p_current_meta;
  uint32_t num_meta_queued;
  imgbase_stream_t stream[MAX_IMGLIB_BASE_MAX_STREAM];
  uint32_t stream_cnt;
  int32_t cur_index;
  int32_t rate_control;
  struct timeval start_time;
  struct timeval end_time;
  int32_t first_frame;
  uint64_t exp_frame_delay;
  int32_t ion_fd;
  boolean dis_enable;
  boolean is_update_valid;
  is_update_t is_update;
  boolean stream_crop_valid;
  mct_bus_msg_stream_crop_t stream_crop;
  boolean isp_output_dim_stream_info_valid;
  mct_stream_info_t isp_output_dim_stream_info;
  uint32_t isp_extra_native_buf;
  uint32_t isp_native_buf_alignment;
  uint32_t imglib_native_buf_alignment;
  boolean before_cpp;
  boolean is_dim_change;
  boolean bypass_enable;
  uint32_t output_identity;
  uint32_t divert_identity;
  uint32_t divert_mask;
  cam_misc_buf_t *p_current_misc_data;
  uint32_t stream_mask;
  uint32_t streams_to_process;
  uint32_t processing_mask;
  cam_stream_type_t output_stream_mask;
  mct_list_t* meta_data_list;
  mct_list_t* hal_meta_data_list;
  cam_feature_mask_t feature_mask;
  boolean set_preferred_mapping;
  isp_preferred_streams preferred_mapping_single;
  isp_preferred_streams preferred_mapping_multi;
  uint32_t input_frame_id[MAX_IMGLIB_BASE_RELEASED_BUFS];
  int release_buf_cnt;
  img_component_ops_t *p_peer_comp;
  boolean is_binded;
  mct_port_t *p_intra_port;
  img_plane_type_t plane_type[MAX_PLANE_CNT];
  void *p_private_data;
  img_thread_ops_t thread_ops;
  img_thread_job_params_t thread_job;
  pthread_mutex_t buf_mutex;
  img_frame_t dummy_frame;
  imgbase_buf_t *buf_list[MAX_IMGLIB_BATCH_SIZE];
  int buf_list_idx;
  int32_t num_ack;
  mct_stream_map_buf_t native_buffs_map[MAX_BAYER_PROC_NATIVE_BUF_NUM];
  mct_list_t *native_buffer_list;
  img_internal_buflist_t internal_buf;
  boolean process_all_frames;
  img_frame_t *p_current_frame;
  uint32_t session_id;
  uint32_t effect;
  uint32_t event_identity;
  bool session_client;
  isp_buf_divert_t *p_current_buf_div;
  img_init_params_t comp_init_params;
  bool async_init;
  uint32_t last_job_id;
  boolean processing_disabled;
  boolean lock_dyn_update;
  boolean crop_required;
  void *p_peer_client;
  img_queue_t meta_hold_q;
  mct_stream_info_t out_stream_info;
  uint32_t is_bayer;
  img_queue_t output_hold_q;
  uint32_t output_hold_q_buf_pending_count;
  int32_t cur_outbuf_cnt;
  bool defer_intbuff_alloc;
  bool piggy_back_ack;
  uint32_t valid_pcr[PER_FRAME_QUEUE_SIZE];
} imgbase_client_t;

/** imgbase_client_divert_wrapper_params_t
 *   @p_frame: pointer to img frame
 *   @p_client: pointer to base client
 *
 *   IMGLIB_BASE client handle src divert wrapper params structure
 **/
typedef struct {
  img_frame_t *p_frame;
  imgbase_client_t *p_client;
} imgbase_client_divert_wrapper_params_t;

/** module_imgbase_params_t
 *   @imgbase_query_mod: function pointer for module query
 *   @imgbase_client_init_params: function ptr for init params
 *   @imgbase_client_share_stream: called during caps reserve to
 *                            ensure that client supports the
 *                            particular stream
 *   @imgbase_client_created: function called when client is
 *                          created
 *   @imgbase_client_destroy: function called before client is
 *                          destroyed
 *
 *   @imgbase_client_streamon: function called when streamon is invoked.
 *   @imgbase_client_pre_streamon: function called prior to streamon
 *   @imgbase_client_process_done: function to indicate after
 *                               the frame is processed
 *   @imgbase_client_flush_done: function to indicate that all the buffers sent to the downstream
 *                              module are obtained
 *   @imgbase_client_update_meta: function called to the
 *                              registered modules before the
 *                              metadata is sent to the
 *                              component
 *   @imgbase_client_handle_frame_skip: function called to the
 *                                    registered modules to
 *                                    handle frame skip
 *   @imgbase_handle_module_event: function called to the
 *                              registered modules before the
 *                              base module event is handled
 *   @imgbase_client_event_handler: function called to the
 *                              registered modules before the
 *                              base client event handler is called
 *   @imgbase_session_start: Function pointer to trigger
 *                         start/stop session
 *   @imgbase_handle_ctrl_parm: function called to the
 *                            registered moduled before ctrl
 *                            parm is handled in base module
 *   @streams_to_process: Mask of streams to be processed by the
 *                      module
 *   @exec_mode: execution mode
 *   @access_mode: access mode
 *   @force_cache_op: force cache clean + invalidation
 *
 *   module parameters for imglib base
 **/
typedef struct {
  boolean (*imgbase_query_mod)(mct_pipeline_cap_t *, void *,unsigned int);
  boolean (*imgbase_client_init_params)(imgbase_client_t *,
    img_init_params_t *p_params);
  boolean (*imgbase_client_stream_supported)(imgbase_client_t *p_client,
    mct_stream_info_t *stream_info);
  int32_t (*imgbase_client_created)(imgbase_client_t *);
  int32_t (*imgbase_client_destroy)(imgbase_client_t *);
  int32_t (*imgbase_client_streamon)(imgbase_client_t *);
  int32_t (*imgbase_client_pre_streamon)(imgbase_client_t *,
    uint32_t identity);
  int32_t (*imgbase_client_flush_frame)(imgbase_client_t *,
    uint32_t event_id);
  int32_t (*imgbase_client_pre_streamoff)(imgbase_client_t *,
    uint32_t identity);
  int32_t (*imgbase_client_streamoff)(imgbase_client_t *);
  int32_t (*imgbase_client_process_done)(imgbase_client_t *, img_frame_t *);
  int32_t (*imgbase_client_flush_done)(imgbase_client_t *, imgbase_stream_t *);
  int32_t (*imgbase_client_update_meta)(imgbase_client_t *, img_meta_t *);
  int32_t (*imgbase_client_handle_frame_skip)(imgbase_client_t *);
  boolean (*imgbase_handle_module_event[MCT_EVENT_MODULE_MAX])(
    uint32_t identity, mct_event_module_t *, imgbase_client_t *,
    img_core_ops_t *, boolean *is_evt_handled);
  int32_t (*imgbase_client_event_handler[QIMG_EVT_MAX])(
    imgbase_client_t *, img_event_t *);
  int32_t (*imgbase_session_start)(void *p_imgbasemod, uint32_t sessionid);
  int32_t (*imgbase_session_stop)(void *p_imgbasemod, uint32_t sessionid);
  int32_t (*imgbase_client_handle_ssr)(imgbase_client_t *);
  int32_t (*imgbase_handle_ctrl_parm[CAM_INTF_PARM_MAX])(
    mct_event_control_parm_t *, imgbase_client_t *, img_core_ops_t *);
  int32_t (*imgbase_client_link_peer)(imgbase_client_t *, uint32_t identity);
  boolean (*imgbase_handle_ctrl_event[MCT_EVENT_CONTROL_MAX])(
    mct_event_control_t *p_ctrl_event, img_core_ops_t *, imgbase_client_t *,
    boolean *is_evt_handled);
  int32_t (*imgbase_client_sensor_query_out_size)(imgbase_client_t *,
    mct_event_t *, sensor_isp_stream_sizes_t *, boolean *p_fwd_event);
  boolean (*imgbase_handle_master_switch)(mct_port_t *port,
    imgbase_client_t *, uint32_t frame_id);
  bool ion_client_needed;
  uint32_t streams_to_process;
  img_comp_execution_mode_t exec_mode;
  img_comp_access_mode_t access_mode;
  bool force_cache_op;
} module_imgbase_params_t;

/** module_imgbase_t
 *   @imgbase_client_cnt: Variable to hold the number of IMGLIB_BASE
 *              clients
 *   @mutex: client lock
 *   @cond: conditional variable for the client
 *   @comp: core operation structure
 *   @lib_ref_count: reference count for imgbase library access
 *   @imgbase_client: List of IMGLIB_BASE clients
 *   @msg_thread: message thread
 *   @parent: pointer to the parent module
 *   @p_mct_mod: pointer to the MCT module
 *   @caps: Capabilities for imaging component
 *   @subdevfd: Buffer manager subdev FD
 *   @modparams: module parameters
 *   @name: name of the module
 *   @session_data_l: list of session data parameters
 *   @hal_version: hal version
 *   @ion_fd: ion client ID
 *   @session_cnt: number of active sessions
 *   @ssid_delta: delta of session id w.r.t base
 *   @module_type: Hold last updated module type
 *   @th_client_id: Thread client ID
 *   @last_error: flag to hold the last error
 *   @preload_done: flag indicating if preload is done
 *   @force_cache_op: whether to force cache operation
 *
 *   IMGLIB_BASE module structure
 **/
typedef struct {
  int imgbase_client_cnt;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  img_core_ops_t core_ops;
  int lib_ref_count;
  mct_list_t *imgbase_client;
  mod_imglib_msg_th_t msg_thread;
  mct_module_t *parent_mod;
  mct_module_t *p_mct_mod;
  uint32_t extra_buf;
  cam_feature_mask_t feature_mask;
  void *mod_private;
  img_caps_t caps;
  int subdevfd;
  module_imgbase_params_t modparams;
  const char *name;
  imgbase_session_data_t session_data_l[MAX_IMGLIB_SESSIONS];
  int32_t hal_version;
  int32_t ion_fd;
  uint32_t session_cnt;
  uint32_t ssid_delta;
  mct_module_type_t module_type;
  uint32_t th_client_id;
  uint32_t last_error;
  bool preload_done;
  bool force_cache_op;
  img_component_ops_t comp_ops;
  img_algo_caps_t algo_caps;
  uint32_t previous_master_sessionid;
} module_imgbase_t;

/**
 * Macro: mod_imgbase_send_event
 *
 * Description: This macro is used for sending an event between
 *            the modules
 *
 * Arguments:
 *   @id: identity
 *   @is_upstream: flag to indicate whether its upstream event
 *   @evt_type: event type
 *   @evt_data: event payload
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
#define mod_imgbase_send_event(id, is_upstream, evt_type, evt_data) ({ \
  boolean rc = TRUE; \
  mct_port_t *p_port; \
  mct_event_t event; \
  memset(&event, 0x0, sizeof(mct_event_t)); \
  event.type = MCT_EVENT_MODULE_EVENT; \
  event.identity = id; \
  if (is_upstream) { \
    event.direction = MCT_EVENT_UPSTREAM; \
    p_port = p_stream->p_sinkport; \
  } else { \
    event.direction = MCT_EVENT_DOWNSTREAM; \
    p_port = p_stream->p_srcport; \
  } \
  event.u.module_event.type = evt_type; \
  event.u.module_event.module_event_data = &evt_data; \
  rc =  mct_port_send_event_to_peer(p_port, &event); \
  rc; \
})

/**
 * Macro: mod_imgbase_schedule_ind_job
 *
 * Description: This macro is used for reserved the thread and
 *            schedule an independent job
 *
 * Arguments:
 *   @p_bclient: pointer to the imgbase client
 *   @p_thread_job: pointer to the thread job parameters
 *   @job_id: job id
 *   @core_type: type of the core in which job needs to be
 *             executed
 *   @thread_affinity: affinity of thread to be created
 *   @del: flag to indicate whether the delete on completion
 *   @p_func: function to be executed
 *   @arg: arguments for the function
 *
 * Return values:
 *     valid job id or 0
 *
 * Notes: none
 **/
#define mod_imgbase_schedule_ind_job(p_bclient, p_thread_job, job_id, \
  core_type, thread_affinity, del, p_func, arg) ({ \
  int32_t rc = IMG_SUCCESS; \
  if (!p_bclient->thread_ops.client_id) { \
    p_bclient->thread_ops.client_id = \
      img_thread_mgr_reserve_threads(1, thread_affinity); \
  } \
  if (!p_bclient->thread_ops.client_id) { \
    IDBG_ERROR("%s:%d] Error reserve thread ", __func__, __LINE__); \
    rc = IMG_ERR_GENERAL; \
  } else { \
    (p_thread_job)->client_id = p_bclient->thread_ops.client_id; \
    (p_thread_job)->core_affinity = core_type; \
    (p_thread_job)->delete_on_completion = del; \
    (p_thread_job)->execute = p_func; \
    (p_thread_job)->dep_job_count = 0; \
    (p_thread_job)->args = arg; \
    (p_thread_job)->dep_job_ids = NULL; \
    job_id = p_bclient->thread_ops.schedule_job(p_thread_job); \
    if (job_id) { \
      IDBG_MED("%s:%d] scheduled job id %x client %x", __func__, __LINE__, \
        job_id, p_bclient->thread_ops.client_id); \
    } else { \
      IDBG_ERROR("%s:%d] Error cannot schedule job ", __func__, __LINE__); \
      rc = IMG_ERR_GENERAL; \
    } \
  } \
  rc; \
})

/** IMGBASE_SSP:
 *
 *  @p_mod: pointer to the module
 *  @ssid: session id
 *
 *   Returns session pointer. NULL if error
 */
#define IMGBASE_SSP(p_mod, ssid) ({ \
  imgbase_session_data_t *p_session = NULL; \
  int32_t idx = (int32_t)ssid - (int32_t)p_mod->ssid_delta; \
  if ((idx < 0) || (idx >= MAX_IMGLIB_SESSIONS)) { \
    IDBG_ERROR("Invalid idx %d", idx); \
  } else { \
    p_session = &(p_mod->session_data_l[idx]); \
  } \
  p_session; \
})

/** IMG_CLIENT_HAS_STREAM:
 *
 *  @p_client: Imglib base client
 *  @str_type: Stream Type
 *
 *   Checks if the stream is connected on the client
 */
#define IMG_CLIENT_HAS_STREAM(p_client, str_type) ({ \
  boolean str_present = false; \
  uint32_t i; \
  for (i = 0; i < p_client->stream_cnt; i++) { \
    if (p_client->stream[i].stream_info->stream_type == str_type) { \
      str_present = true; \
      break; \
    } \
  } \
  str_present; \
})


/**
 * Function: module_paaf_get_paaf_streams
 *
 * Description: Helper function to find streams that have PAAF
 * enabled
 *
 * Arguments:
 * @meta_stream_info - pointer to meta stream info
 * @paaf_streams - Streams on which PAAF feature mask is set
 *
 *
 * Return values:
 *     Number of streams with PAAF enabled
 *
 * Notes: none
 **/
#define IMG_GET_FEATURE_ENABLD_STRS(p_meta_str_info, p_streams, ftr_mask)({ \
  uint32_t meta_idx, i = 0; \
  IDBG_LOW("Meta str info: num of streams %d", p_meta_str_info->num_streams); \
  for (meta_idx = 0; meta_idx < p_meta_str_info->num_streams; meta_idx++) { \
    if (p_meta_str_info->postprocess_mask[meta_idx] & ftr_mask) { \
      p_streams[i] = p_meta_str_info->type[meta_idx]; \
      i++; \
    } \
  } \
  p_streams[i] = CAM_STREAM_TYPE_MAX; \
  return i+1; \
})

/*IMGLIB_BASE client APIs*/

/**
 * Function: module_imgbase_deinit
 *
 * Description: This function is used to free the imgbase module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_imgbase_deinit(mct_module_t *p_mct_mod);

/** module_imgbase_init:
 *
 *  Arguments:
 *  @name - name of the module
 *  @comp_role: imaging component role
 *  @comp_name: imaging component name
 *  @mod_private: derived structure pointer
 *  @p_caps: imaging capability
 *  @lib_name: library name
 *  @feature_mask: feature mask of imaging algo
 *  @p_modparams: module parameters
 *
 * Description: This function is used to initialize the imgbase
 * module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_imgbase_init(const char *name,
  img_comp_role_t comp_role,
  char *comp_name,
  void *mod_private,
  img_caps_t *p_caps,
  char *lib_name,
  cam_feature_mask_t feature_mask,
  module_imgbase_params_t *p_modparams);

/** Function: module_imgbase_client_create
 *
 * Description: This function is used to create the IMGLIB_BASE client
 *
 * Arguments:
 *   @p_mct_mod: mct module pointer
 *   @p_port: mct port pointer
 *   @session_id: session id
 *   @p_stream_info: pointer to the stream info
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_create(mct_module_t *p_mct_mod,
  mct_port_t *p_port,
  uint32_t session_id,
  mct_stream_info_t *p_stream_info);

/**
 * Function: module_imgbase_client_destroy
 *
 * Description: This function is used to destroy the imgbase client
 *
 * Arguments:
 *   @p_client: imgbase client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
void module_imgbase_client_destroy(imgbase_client_t *p_client);


/**
 * Function: module_imgbase_client_flush
 *
 * Description: This function is used to flush the
 *   buffers with component and imgbase client
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_flush(imgbase_client_t *p_client);


/**
 * Function: module_imgbase_client_stop
 *
 * Description: This function is used to stop the IMGLIB_BASE
 *              client
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_stop(imgbase_client_t *p_client);

/**
 * Function: module_imgbase_client_start
 *
 * Description: This function is used to start the IMGLIB_BASE
 *              client
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @identity: identity of the stream
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_start(imgbase_client_t *p_client, uint32_t identity);

/**
 * Function: module_imgbase_client_handle_framedrop
 *
 * Description: This function is used to handle the framedrop
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: mct event
 *   @frame_id: frame id
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/


int module_imgbase_client_handle_framedrop(mct_port_t *port,
  mct_event_t *event, uint32_t frameid, uint32_t streamoff);

/**
 * Function: module_imgbase_client_handle_update_superparms_drop_queue
 *
 * Description: Super params handler function to drop queue
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: mct event
 *   @current_frame_id: current frameid
 *
 * Return values:
 *    standard image lib return codes
 *
 * Notes: none
 **/

int module_imgbase_client_handle_update_superparms_drop_queue
  (mct_port_t *port,
  mct_event_t *event, uint32_t current_frameid);


/**
 * Function: module_imgbase_process_super_params
 *
 * Description: Super params handler function
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: mct event
 *
 * Return values:
 *    standard image lib return codes
 *
 * Notes: none
 **/

int32_t module_imgbase_process_super_params(mct_port_t *port,
  mct_event_t *event, bool buf_divert, uint32_t current_frameid);


/**
 * Function: module_imgbase_client_handle_buffer
 *
 * Description: This function is used to start the IMGLIB_BASE
 *              client
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @p_buf_divert: Buffer divert structure
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_handle_buffer(imgbase_client_t *p_client,
  isp_buf_divert_t *p_buf_divert);

/**
 * Function: module_imgbase_client_pack_buffer
 *
 * Description: This function is used to pack internal
 * buffers into stream mapped buffer structure for offline
 * isp usecases
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @stride_in_bytes: Stride in bytes
 *   @scanline: Scanline size
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_pack_buffer(imgbase_client_t *p_client,
  int32_t stride_in_bytes, int32_t scanline);

/**
 * Function: module_imgbase_client_handle_sof_notify
 *
 * Description: This function is used to update tardy
 * delay to frame_config_q queue
 *
 * Arguments:
 *   @p_port: mct port pointer
 *   @event :  mct event pointer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/

int module_imgbase_client_handle_sof_notify(mct_port_t *port,
  mct_event_t *event);

/**
 * Function: module_imgbase_client_alloc_int_buf
 *
 * Description: This function is used to allocate internal
 *                 buffers for IMGLIB_BASE client.
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @identity: input identity
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_alloc_int_buf(imgbase_client_t *p_client,
  uint32_t identity);

/**
 * Function: module_imgbase_find_stream_by_identity
 *
 * Description: This method is used to find the client based
 *              on stream identity
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @identity: input identity
 *
 * Return values:
 *     Index of the stream in the stream port mapping of the
 *     client
 *
 * Notes: none
 **/
int module_imgbase_find_stream_by_identity(imgbase_client_t *p_client,
  uint32_t identity);

/**
 * Function: module_imgbase_client_handle_buffer_ack
 *
 * Description: This function is used to handle the buffer ack
 *
 * Arguments:
 *   @p_client: IMGLIB_BASE client
 *   @ack_event: ack event type
 *   @data: payload pertaining to ack_event
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
int module_imgbase_client_handle_buffer_ack(imgbase_client_t *p_client,
  img_ack_event_t ack_event, void* data);

/**
 * Function: module_imgbase_find_stream_idx_by_type
 *
 * Description: This method is used to find the client based
 *              on stream type
 *
 * Arguments:
 *   @p_client: imgbase client
 *   @stream_type: input stream type
 *
 * Return values:
 *     Index of the stream in the stream port mapping of the
 *     client
 *
 * Notes: none
 **/
int module_imgbase_find_stream_idx_by_type(imgbase_client_t *p_client,
  uint32_t stream_type);

/** module_imgbase_set_parent:
 *
 *  Arguments:
 *  @p_parent - parent module pointer
 *
 * Description: This function is used to set the parent pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_imgbase_set_parent(mct_module_t *p_mct_mod, mct_module_t *p_parent);

/**
 * Function: module_imgbase_post_bus_msg
 *
 * Description: post a particular message to media bus
 *
 * Arguments:
 *   @p_mct_mod - media controller module
 *   @sessionid - session id
 *   @msg_id - bus message id
 *   @msg_data - bus message data
 *   @size - bus message size
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_imgbase_post_bus_msg(mct_module_t *p_mct_mod,
  unsigned int sessionid, mct_bus_msg_type_t msg_id, void *msg_data, uint32_t size);

/**
 * Function: module_imgbase_get_zoom_ratio
 *
 * Description: This method is used to find the session data
 *
 * Arguments:
 *   @p_module: mct module pointer
 *   @sessionid: session id
 *   @zoom_level: zoom level to get zoom ratio
 *
 * Return values:
 *     zoom ratio: float
 *
 * Notes: none
 **/
float module_imgbase_get_zoom_ratio(
  void* p_module, uint32_t sessionid,
  uint8_t zoom_level);

/**
 * Function: module_imgbase_client_preload
 *
 * Description: function called for base client preload
 *
 * Arguments:
 *   @p_session_data - Session data
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_imgbase_client_preload(imgbase_session_data_t *p_session_data);

/**
 * Function: module_imgbase_find_session_str_client
 *
 * Description: This method is used to find the client
 * associated with the session stream
 *
 * Arguments:
 *   @p_fp_data: imgbase client
 *   @p_input: input data
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_imgbase_find_session_str_client(void *p_imgbase_data,
  void *p_input);

void module_imgbase_client_handle_outbuf_done(
  imgbase_client_t *p_client,
  img_frame_t *p_frame,
  int8_t buf_done);

int module_imgbase_client_get_outputbuf(imgbase_client_t *p_client,
  img_frame_t *pframe,
  bool internal_buf,
  bool fetch_buf,
  uint32_t *p_buffer_access);

/**
 * Function: module_imgbase_client_get_and_update_buf_index
 *
 * Description: This function is used to get HAL3 requested
 * buffer index
 *
 * Arguments:
 *   @p_mod: Ptr to the module
 *   @p_client: IMGLIB_BASE client
 *   @frame_id: Frame id
 *   @reset : flag to reset queue index
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/

int32_t module_imgbase_client_get_and_update_buf_index(module_imgbase_t *p_mod,
  imgbase_client_t *p_client, uint32_t frame_id, int reset);

#endif //__MODULE_IMGLIB_BASE_H__
