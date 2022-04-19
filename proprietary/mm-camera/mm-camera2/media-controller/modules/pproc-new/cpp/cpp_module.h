/*============================================================================

  Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#ifndef CPP_MODULE_H
#define CPP_MODULE_H

#include <pthread.h>
#include <math.h>
#include "modules.h"
#include "chromatix.h"
#include "chromatix_common.h"
#include "mct_queue.h"
#include "mct_list.h"
#include "media_controller.h"
#include "mct_port.h"
#include "mct_object.h"
#include "cam_types.h"
#include "cam_intf.h"
#include "mct_module.h"
#include "mct_pipeline.h"
#include "mct_stream.h"
#include "camera_dbg.h"
#include "cpp_port.h"
#include "cpp_thread.h"
#include "cpp_hardware.h"
#include "pproc_module.h"
#include "pp_buf_mgr.h"

#define READ_FD   0
#define WRITE_FD  1

#define CPP_MODULE_MAX_STREAMS      CPP_HARDWARE_MAX_STREAMS
#define CPP_MODULE_MAX_SESSIONS     12

#define CPP_OUTPUT_DUPLICATION_EN   1
#define CPP_MAX_OUTPUT_DUPLICATE   1
#define CPP_FPS_TABLE_COUNT   3
#define CPP_FPS_TABLE_SIZE   8

#define CPP_FRAMES_TURBO_THRESHOLD  1

#define MODULE_CPP_MIN_NUM_PP_BUFS  1
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
 #define FRAME_CTRL_SIZE 64
#else
 #define FRAME_CTRL_SIZE 32
#endif
#define MINIMUM_CPP_THROUGHPUT      1.5f
#define COMP_RATIO_NRT              1.26f
#define MAXIMUM_CPP_THROUGHPUT      1.5f

#define CAM_PERF_HIGH_PERFORMANCE_BIT 1


/* macros for unpacking identity */
#define CPP_GET_STREAM_ID(identity) ((identity) & 0xFFFF)
#define CPP_GET_SESSION_ID(identity) (((identity) & 0xFFFF0000) >> 16)

/* macros for per frame queue process operations */
#define CPP_FRAME_QUEUE_APPLY   (1 << 0)
#define CPP_FRAME_QUEUE_PREPARE (1 << 1)
#define CPP_FRAME_QUEUE_REPORT  (1 << 2)

/*CPP blocking wait timeout: ns, need to < 1s*/
#define CPP_WAIT_TIMEOUT 750000000
#define PTHREAD_COND_WAIT_TIME(cond_ptr, mutex_ptr, \
    timeout_ptr, ns, rc) \
  { \
    clock_gettime(CLOCK_REALTIME, (timeout_ptr)); \
    (timeout_ptr)->tv_nsec += ns;\
    if ((timeout_ptr)->tv_nsec > 999999999) { \
      (timeout_ptr)->tv_nsec -= 999999999; \
      (timeout_ptr)->tv_sec += 1; \
    } \
    rc = pthread_cond_timedwait(cond_ptr, \
      mutex_ptr, timeout_ptr);\
  }
#define IS_ULT_HW_VERSION(ctrl) \
(ctrl->cpphw->hwinfo.version == CPP_HW_VERSION_4_3_4) ? 1 : 0 \


#define IS_CROP_PARAM_VALID(crop_param, crop_max, crop_reset_value) \
{ \
  if (((int32_t)(crop_param) < 0) || \
    (crop_param > crop_max)) { \
    CPP_ERR("Crop param invalid %d, reset to %d", \
      crop_param, crop_reset_value); \
    crop_param = crop_reset_value; \
  } \
}


#define IS_CURVE_UPDATE_NEEDED(stream_params) \
  ((stream_params) && (stream_params->is_stream_on) && \
    (stream_params->hw_params.hdr_mode == CAM_SENSOR_HDR_ZIGZAG) && \
    ((stream_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) && \
    (stream_params->stream_type != CAM_STREAM_TYPE_PARM))) ? 1 : 0 \

#define IF_DIS_ENABLE(session_params, stream_params) \
  (((stream_params->stream_type == CAM_STREAM_TYPE_PREVIEW) || \
    (stream_params->stream_type == CAM_STREAM_TYPE_VIDEO)) && \
    (stream_params->stream_info->is_type == IS_TYPE_DIS) && \
    (session_params->dis_enable)) ? 1 : 0

/*
 * IF DIS is enabled, frame needs to be held and stream
 * is either preview or video
 */
#define IF_IS_HOLD_REQD(session_params, stream_params) \
  ((IF_DIS_ENABLE(session_params, stream_params)) && \
    (session_params->frame_hold.is_frame_hold == TRUE)) \
    ? 1 : 0

#define IS_CLOCK_BW_EQUAL(ctrl, clk, bw) \
  ((clk == ctrl->clk) && ((bw) == ctrl->bw)) ? 1 : 0

#define IS_CLOCK_BW_LESS(ctrl, clk, bw) \
  ((clk < ctrl->clk) || ((bw) < ctrl->bw)) ? 1 : 0

#define IS_SKIP_STREAM_LOAD(obj, load) \
  ((!(obj->process_delay || load->force || \
  (obj->stream_type == CAM_STREAM_TYPE_PREVIEW) || \
  (obj->stream_type == CAM_STREAM_TYPE_VIDEO) || \
  (obj->stream_type == CAM_STREAM_TYPE_CALLBACK))) || \
  (obj->slave_session)) ? 1 : 0

#define FORCE_LOAD_CALC(force, hal_version) \
  current_load.force = (force || (hal_version != CAM_HAL_V3)) ? 1 : 0

#define UPDATE_INPUT_LOAD(ctrl, session_params, stream_params, clk_rate_obj) \
{ \
  if (!(clk_rate_obj->input_load) && \
    (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC)) { \
    clk_rate_obj->input_load = (float)( \
    stream_params->hw_params.input_info.width * \
    stream_params->hw_params.input_info.height * \
    (float)(cpp_module_get_bytes_per_pixel( \
    stream_params->hw_params.input_info.plane_fmt)) * \
    (float)(cpp_module_set_input_stream_fps(ctrl, stream_params, session_params))); \
  } \
}

#define CLOCK_REFERENCE_THRESHOLD 15

#define DUP_FLAG 0x1
#define TNR_FLAG 0x2
#define DSDN_FLAG 0x4
#define UBWC_FLAG 0x8
#define FORCE_CHECK 0x10
#define SHARE_FLAG  0x32

#define DUP_FLAG_SHIFT 0
#define TNR_FLAG_SHIFT 1
#define DSDN_FLAG_SHIFT 2
#define UBWC_FLAG_SHIFT 3
#define FORCE_CHECK_SHIFT 4
#define SHARED_FLAG_SHIFT 5

#define MINIMUM_PROCESS_TIME 10.0f
#define MAX_SUPPORTED_FPS 30.0f

#define LIVE_SNAPSHOT_FPS    2.0f

#define CPP_UNUSED_VAR(__var) CAM_UNUSED_PARAM((__var))

typedef struct _cpp_module_ctrl_t cpp_module_ctrl_t;

typedef enum {
  CPP_DIVERT_UNPROCESSED,
  CPP_DIVERT_PROCESSED
} cpp_divert_type_t;

typedef enum {
  CPP_PRIORITY_REALTIME,
  CPP_PRIORITY_PARTIAL_FRAME,
  CPP_PRIORITY_OFFLINE
} cpp_priority_t;

/* aysnchronous events are the event processed by
  cpp_thread from the event_queue */
typedef enum {
  CPP_MODULE_EVENT_PROCESS_BUF,
  CPP_MODULE_EVENT_DIVERT_BUF,
  CPP_MODULE_EVENT_CLOCK,
  CPP_MODULE_EVENT_PARTIAL_FRAME,
  CPP_MODULE_EVENT_ISP_BUFFER_DROP
} cpp_module_event_type_t;

typedef struct _cpp_module_event_process_buf_data_t {
  isp_buf_divert_t       isp_buf_divert;
  uint32_t               proc_identity;
  boolean                proc_div_required;
  uint32_t               proc_div_identity;
  cpp_hardware_params_t  hw_params;
} cpp_module_event_process_buf_data_t;

typedef struct _cpp_module_event_divert_buf_data_t {
  isp_buf_divert_t  isp_buf_divert;
  uint32_t          div_identity;
} cpp_module_event_divert_buf_data_t;

typedef struct _cpp_module_event_clock_data_t {
  unsigned long clk_rate;
  uint64_t      bandwidth_avg;
  uint64_t      bandwidth_inst;
} cpp_module_event_clock_data_t;

typedef struct _cpp_module_event_partial_buf_data_t {
  cpp_hardware_partial_stripe_info_t partial_stripe_info;
  struct msm_cpp_frame_info_t     *frame;
} cpp_module_event_partial_frame_t;

/* key to find the ACK from list */
typedef struct _cpp_module_ack_key_t {
  uint32_t identity;
  uint32_t buf_idx;
  uint32_t channel_id;
  uint32_t frame_id;
  void *meta_datas;
  uint32_t buf_identity;
} cpp_module_ack_key_t;

typedef struct _cpp_module_hw_cookie_t {
  cpp_module_ack_key_t  key;
  boolean               proc_div_required;
  uint32_t              proc_div_identity;
  void                 *meta_datas;
#ifndef CACHE_PHASE2
  uint32_t              buffer_access;
#endif
  boolean               frame_dump_flag;
} cpp_module_hw_cookie_t;

/* data structure to store the ACK in list */
typedef struct _cpp_module_ack_t {
  isp_buf_divert_ack_t isp_buf_divert_ack;
  int32_t ref_count;
  int32_t frame_id;
  struct timeval in_time, out_time;
} cpp_module_ack_t;

/* list for storing ACKs */
typedef struct _cpp_module_ack_list_t {
  mct_list_t        *list;
  uint32_t           size;
  pthread_mutex_t    mutex;
} cpp_module_ack_list_t;

/* Information about frame skipping for HFR */
typedef struct _cpp_module_hfr_skip_info_t {
  boolean      skip_required;
  int32_t      skip_count;
  uint32_t     frame_offset;
  float        output_fps;
  float        input_fps;
} cpp_module_hfr_skip_info_t;

typedef struct _cpp_module_frame_hold_t {
  /* frame hold flag, if flag is TRUE, then isp_buf holds frame */
  boolean          is_frame_hold;
  /* Store frame until DIS event for this frame is received */
  uint32_t     identity;
  isp_buf_divert_t isp_buf;
} cpp_module_frame_hold_t;

typedef struct _cpp_module_dis_hold_t {
  /* DIS hold flag, if is_valid is TRUE, then dis_frame_id has valid frame id */
  boolean      is_valid;
  /* Store frame until DIS event for this frame is received */
  uint32_t dis_frame_id;
} cpp_module_dis_hold_t;

typedef enum {
  CPP_FRM_CTRL_Q_APPLY = 1,
  CPP_FRM_CTRL_Q_REPORT,
} cpp_frm_ctrl_qtype_t;

typedef struct {
  uint32_t                   frame_id;
  cpp_frm_ctrl_qtype_t       q_entry_type;
  mct_event_type             mct_type;
  uint32_t                   identity;
  union {
    mct_event_control_parm_t ctrl_param;
    mct_event_module_t       module_event;
  } u;
} cpp_frame_ctrl_data_t;

typedef struct {
  pthread_mutex_t  mutex;
  mct_queue_t     *frame_ctrl_q[FRAME_CTRL_SIZE]; /* cpp_frame_ctrl_data_t */
  pthread_mutex_t  frame_ctrl_mutex[FRAME_CTRL_SIZE];
  uint32_t         max_apply_delay;
  uint32_t         max_report_delay;
  uint32_t         cpp_delay;
  uint32_t         cur_frame_id;
  int32_t          real_time_stream_cnt;
  int32_t          offline_stream_cnt;
} cpp_per_frame_params_t;

typedef struct _cpp_module_buffer_info {
  int fd;
  uint32_t index;
  uint32_t offset;
  uint8_t native_buff;
  uint8_t processed_divert;
} cpp_module_buffer_info_t;

typedef struct _cpp_module_max_snapshot_fps {
  uint32_t width;
  uint32_t height;
  float max_fps;
}cpp_module_max_snapshot_fps;

typedef struct _cpp_module_stream_params_t cpp_module_stream_params_t;

/* stream specific parameters */
struct _cpp_module_stream_params_t {
  void                         *parent;
  cpp_hardware_params_t        hw_params;
  uint32_t                     identity;
  cpp_priority_t               priority;
  cam_stream_type_t            stream_type;
  boolean                      is_stream_on;
  pthread_mutex_t              mutex;
  cpp_module_hfr_skip_info_t   hfr_skip_info;
  /* linked stream is the stream which shares same ISP input buffer.
     This is usually mapped on same port */
  uint32_t                     num_linked_streams;
  cpp_module_stream_params_t   *linked_streams[CPP_MODULE_MAX_STREAMS];
  uint32_t                     cur_frame_id;
  mct_stream_info_t            *stream_info;
  uint8_t                      req_frame_divert;
  int32_t                      current_burst_cnt;
  uint8_t                      req_frame_output;
  boolean                      track_frame_done;
  uint32_t                     turbo_frameproc_count;
  /* Load corresponding to one frame */
  int64_t                      load_per_frame;
  /* variable idicating how many buffers are active in cpp for a stream */
  /* Process dealy greater than 1 indicates performance bottleneck */
  int32_t                      process_delay;
  uint32_t                     native_buff;
  uint32_t                     share_output;
  uint32_t                     enable_post_cpp;
};

typedef struct {
  cpp_module_stream_params_t *process_stream;
  uint32_t dup_identity;
} cpp_module_hw_proc_info_t;

typedef struct {
  cpp_module_stream_params_t *linked_stream_params[CPP_MODULE_MAX_STREAMS];
  uint32_t num_linked_streams;
} cpp_module_link_info_t;

typedef struct {
  cpp_module_hw_proc_info_t hw_proc_info[CPP_MODULE_MAX_STREAMS];
  uint32_t num_hw_process;
} cpp_module_process_list_t;

typedef struct {
  uint32_t is_valid;
  uint32_t apply_frame_id;
  int32_t value;
  uint32_t  identity;
} cpp_module_session_set_parm_effect;

typedef struct {
  uint32_t is_valid;
  uint32_t apply_frame_id;
  cam_edge_application_t value;
  uint32_t  identity;
} cpp_module_session_set_parm_edge_mode;

typedef struct {
  uint32_t is_valid;
  uint32_t apply_frame_id;
  int32_t value;
  uint32_t  identity;
} cpp_module_session_set_parm_reduction_mode;

typedef struct {
  uint32_t is_valid;
  uint32_t apply_frame_id;
  int32_t value;
  uint32_t  identity;
} cpp_module_session_set_parm_cds_mode;

typedef struct {
  uint32_t  is_valid;
  uint32_t  apply_frame_id;
  cam_dyn_img_data_t value;
  uint32_t  identity;
} cpp_module_session_set_parm_dyn_feast_mode;

typedef struct {
  cpp_module_session_set_parm_effect spl_effect;
  cpp_module_session_set_parm_edge_mode edge_mode;
  cpp_module_session_set_parm_reduction_mode reduction_mode;
  cpp_module_session_set_parm_cds_mode cds_mode;
  cpp_module_session_set_parm_dyn_feast_mode dyn_feast;
} cpp_module_session_set_parm;

/* session specific parameters */
typedef struct _cpp_module_session_params_t {
  cpp_module_stream_params_t   *stream_params[CPP_MODULE_MAX_STREAMS];
  int32_t                       stream_count;
  cpp_hardware_params_t         hw_params;
  uint32_t                      session_id;
  cam_hfr_mode_t                hfr_mode;
  cpp_params_aec_trigger_info_t aec_trigger;
  /* DIS enable flag to be used for frame hold */
  int32_t                       dis_enable;
  /* Latest frame id received from DIS crop event */
  cpp_module_dis_hold_t         dis_hold;
  /* Hold frame until DIS crop is received for this frame */
  cpp_module_frame_hold_t       frame_hold;
  ez_pp_params_t                diag_params;
  cam_hal_version_t             hal_version;
  cpp_per_frame_params_t        per_frame_params;
  boolean                       is_stream_on;
  uint32_t                      stream_on_count;
  cam_fps_range_t               fps_range;
  cam_stream_ID_t               valid_stream_ids[FRAME_CTRL_SIZE];
  cam_stream_ID_t               verify_proc_stream_ids[FRAME_CTRL_SIZE];
  pthread_mutex_t               dis_mutex;
  modulesChromatix_t            module_chromatix;
  chromatix_cpp_stripped_type   *def_chromatix_stripped;
  boolean                       runtime_clk_update;
  cam_dimension_t               camif_dim;
  uint32_t                      turbo_frame_count;
  int32_t                       clk_ref_threshold_idx;
  /* for Dual Cam: Primary or Secondary */
  bool                          is_slave;
  /* for Dual Cam: Main or Aux */
  cam_sync_type_t               cam_type;
  int32_t                       link_session_id;
  int32_t                       native_buf_ref;
  cam_dual_camera_perf_control_t dualcam_perf;
  cpp_module_session_set_parm   sticky_set_parm;
  cam_dual_camera_role_t        cam_role;
  bool                          force_slave_process;
  boolean                       cpp_debug_enable;
} cpp_module_session_params_t;

typedef struct _cpp_submodule_func_tbl_t {
  int32_t (*update)(modulesChromatix_t *module_chromatix,
    cpp_hardware_params_t *hw_params,cpp_params_aec_trigger_info_t *aec_trigger);
  int32_t (*set)(cpp_module_ctrl_t *ctrl, uint32_t identity, uint32_t enable);
  int32_t (*prepare)(cpp_module_ctrl_t *ctrl, uint32_t identity);
  int32_t (*unprepare)(cpp_module_ctrl_t *ctrl, uint32_t identity);
  int32_t (*private)(cpp_module_ctrl_t *ctrl, uint32_t identity);
} cpp_submodule_func_tbl_t;

typedef struct _cpp_module_stream_buff_info {
  uint32_t identity;
  uint32_t num_buffs;
  boolean delete_buf;
  mct_list_t *buff_list;
} cpp_module_stream_buff_info_t;

/* list for storing clock_rates of different streams */
typedef struct _cpp_module_clk_rate_list_t {
  mct_list_t        *list;
  uint32_t           size;
  pthread_mutex_t    mutex;
} cpp_module_clk_rate_list_t;

typedef enum {
  /* steady clock state */
  CPP_CLOCK_STEADY,
  /* clock bump up pending */
  CPP_CLOCK_BUMP_UP,
  /* clock bumped up */
  CPP_CLOCK_BUMPED_UP,
  /* clock bump down  pending */
  CPP_CLOCK_BUMP_DOWN,
} cpp_clock_state;

typedef struct _cpp_module_stream_clk_rate_t {
  uint32_t identity;
  uint32_t stream_type;
  uint64_t total_load;
  uint64_t input_load;
  uint64_t output_load;
  // Bit 1 duplication
  // Bit 2 TNR
  // Bit 3 DSDN
  // Bit 4 UBWC
  // Bit 5 Force check
  // Bit 6 Output count
  int32_t config_flags;
  float   system_overhead;
  cam_perf_mode_t perf_mode;
  int32_t process_delay;
  /* excess load for stream after a perf bottleneck */
  int64_t excess_load_per_frame;
  bool    slave_session;
  int32_t shared_output_num;
} cpp_module_stream_clk_rate_t;

typedef struct _cpp_module_total_load_t {
  uint64_t clk;
  uint64_t input_bw;
  uint64_t output_bw;
  uint64_t duplication_load;
 uint64_t input_ref_load;
  uint64_t output_ref_load;
  cam_perf_mode_t perf_mode;
  int32_t process_delay;
  /* force clock calculation */
  boolean force;
} cpp_module_total_load_t;

struct _cpp_module_ctrl_t {
  mct_module_t                *p_module;
  mct_module_t                *parent_module;
  mct_queue_t                 *realtime_queue;
  mct_queue_t                 *partial_frame_queue;
  mct_queue_t                 *offline_queue;
  cpp_module_ack_list_t       ack_list;
  pthread_t                   cpp_thread;
  pthread_cond_t              th_start_cond;
  boolean                     cpp_thread_started;
  pthread_mutex_t             cpp_mutex;
  int                         pfd[2];
  int32_t                     session_count;
  cpp_hardware_t              *cpphw;
  cpp_module_clk_rate_list_t  clk_rate_list;
  unsigned long               clk_rate;
  boolean                     runtime_clk_update;
  pp_native_buf_mgr_t         pp_buf_mgr;
  pp_native_buf_mgr_t         pp_tnr_buf_mgr;
  cpp_module_session_params_t *session_params[CPP_MODULE_MAX_SESSIONS];
  cpp_submodule_func_tbl_t    tnr_module_func_tbl;
  cpp_submodule_func_tbl_t    pbf_module_func_tbl;
  mct_port_t                  *port_map[CPP_MODULE_MAX_SESSIONS][CPP_MODULE_MAX_STREAMS][2];
  /* last updated clock and bandwidth */
  int64_t                     clk;
  int64_t                     bw;
  /* current state for a clock update */
  cpp_clock_state             clk_state;
  /* threshold count to trigger a clock bump down */
  int32_t                     clock_threshold;
  int32_t                     clock_dcvs;
  int32_t                     turbo_caps;
  volatile bool               is_hw_error;
  uint32_t                    rtb_status;
  uint32_t                    sat_status;
  uint32_t                    soc_id;
};

typedef struct _cpp_module_event_drop_buffer_t {
  cpp_module_stream_params_t* stream_params;
  uint32_t frame_id;
}cpp_module_event_drop_buffer_t;

/* data structure for events stored in cpp queue */
typedef struct _cpp_module_async_event_t {
  cpp_module_event_type_t type;
  /* invalid bit used for stream-off handling */
  boolean invalid;
  /* indicates if cpp hw processing is needed for this event */
  boolean hw_process_flag;
  /* key to find corresponding ack */
  cpp_module_ack_key_t ack_key;

  /* event specific data based on type */
  union {
    cpp_module_event_process_buf_data_t     process_buf_data;
    cpp_module_event_divert_buf_data_t      divert_buf_data;
    cpp_module_event_clock_data_t           clock_data;
    cpp_module_event_partial_frame_t        partial_frame;
    cpp_module_event_drop_buffer_t          drop_buffer;
  } u;
} cpp_module_event_t;

mct_module_t* cpp_module_init(const char *name);
void module_cpp_set_parent(mct_module_t *module, mct_module_t *parent_mod);
void cpp_module_deinit(mct_module_t *mod);
static void cpp_module_set_mod (mct_module_t *module,  mct_module_type_t module_type,
  uint32_t identity);
static boolean cpp_module_query_mod(mct_module_t *module, void *query_buf,
  uint32_t sessionid);
static boolean cpp_module_start_session(mct_module_t *module,
  uint32_t sessionid);
static boolean cpp_module_stop_session(mct_module_t *module,
  uint32_t sessionid);
static cpp_module_ctrl_t* cpp_module_create_cpp_ctrl(void);
static int32_t cpp_module_destroy_cpp_ctrl(cpp_module_ctrl_t *ctrl);

static int32_t cpp_module_send_ack_event_upstream(cpp_module_ctrl_t *ctrl,
  isp_buf_divert_ack_t isp_ack);

int32_t cpp_module_post_msg_to_thread(cpp_module_ctrl_t *ctrl,
  cpp_thread_msg_t msg);

int32_t cpp_module_process_downstream_event(mct_module_t* module,
  mct_event_t *event);
int32_t cpp_module_process_upstream_event(mct_module_t* module,
  mct_event_t *event);
int32_t cpp_module_send_event_downstream(mct_module_t* module,
   mct_event_t* event);
int32_t cpp_module_notify_frame_drop(mct_module_t* module,
  uint32_t identity, uint32_t frame_id);
int32_t cpp_module_send_event_upstream(mct_module_t* module,
   mct_event_t* event);
int32_t cpp_module_notify_add_stream(mct_module_t* module, mct_port_t* port,
  mct_stream_info_t* stream_info);
int32_t cpp_module_notify_remove_stream(mct_module_t* module,
  uint32_t identity);
int32_t cpp_module_do_ack(cpp_module_ctrl_t *ctrl,
  cpp_module_ack_key_t key, uint32_t buffer_access);
int32_t cpp_module_invalidate_queue(cpp_module_ctrl_t* ctrl,
  uint32_t identity);
int32_t cpp_module_put_new_ack_in_list(cpp_module_ctrl_t *ctrl,
  cpp_module_ack_key_t key, int32_t buf_dirty, int32_t ref_count,
  isp_buf_divert_t *isp_buf);
int32_t cpp_module_enq_event(cpp_module_ctrl_t *ctrl,
  cpp_module_event_t* event, cpp_priority_t prio);
int32_t cpp_module_flush_queue_events(cpp_module_ctrl_t *ctrl,
  int32_t frame_id, uint32_t identity, uint8_t flush_frame);
int32_t cpp_module_set_clock_freq(cpp_module_ctrl_t *ctrl, mct_module_t* module,
  cpp_module_stream_params_t *stream_params, uint32_t stream_event,
  cpp_module_session_params_t* session_params);
float cpp_module_set_input_stream_fps(cpp_module_ctrl_t *ctrl,
  cpp_module_stream_params_t *stream_params,
  cpp_module_session_params_t* session_params);
int32_t cpp_module_set_clock_freq_for_HAL3(cpp_module_ctrl_t *ctrl,
  mct_module_t* module, cpp_module_stream_params_t *stream_params,
  uint32_t stream_event);

int32_t cpp_module_process_module_event(mct_module_t* module,
  mct_event_t* event);
int32_t cpp_module_handle_module_event(mct_module_t* module,
  mct_event_t *event);
int32_t cpp_module_handle_request_divert(mct_module_t* module,
  mct_event_t* event);
int32_t cpp_module_set_parm_dsdn(cpp_module_ctrl_t *ctrl,
  uint32_t identity, int32_t value);
int32_t cpp_module_set_parm_img_dyn_feat(cpp_module_ctrl_t *ctrl,
  uint32_t identity, uint64_t value);
boolean cpp_module_check_frame_skip(cpp_module_stream_params_t *stream_params,
  cpp_module_session_params_t *session_params, uint32_t frame_id);
int32_t cpp_module_process_set_param_event(cpp_module_ctrl_t *ctrl,
   uint32_t identity, cam_intf_parm_type_t type, void *parm_data,
   boolean *param_processed, uint32_t current_frame_id);
int32_t cpp_module_handle_set_parm_event(mct_module_t* module,
  mct_event_t* event);
int32_t cpp_module_update_clock_freq(mct_module_t* module __unused,
  cpp_module_ctrl_t *ctrl,
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params,
  bool force);
int32_t cpp_module_get_and_update_buf_index(
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params, uint32_t frame_id);

/* ----------------------- utility functions -------------------------------*/

mct_port_t* cpp_module_find_port_with_identity(mct_module_t *module,
  mct_port_direction_t dir, uint32_t identity);
cpp_module_ack_t* cpp_module_find_ack_from_list(cpp_module_ctrl_t *ctrl,
  cpp_module_ack_key_t key);
cam_streaming_mode_t cpp_module_get_streaming_mode(mct_module_t *module,
  uint32_t identity);
int32_t cpp_module_get_params_for_identity(cpp_module_ctrl_t* ctrl,
  uint32_t identity, cpp_module_session_params_t** session_params,
  cpp_module_stream_params_t** stream_params);
int32_t cpp_module_get_params_for_streamtype(
  cam_stream_type_t type,
  cpp_module_stream_params_t** stream_params);
int32_t cpp_module_util_update_clock_rate(cpp_module_ctrl_t* ctrl,
  cpp_module_session_params_t* session_params,
  cam_stream_type_t type,
  cpp_module_stream_params_t** stream_params);
int32_t cpp_module_get_params_for_session_id(cpp_module_ctrl_t* ctrl,
  uint32_t session_id, cpp_module_session_params_t** session_params);
void cpp_module_dump_stream_params(cpp_module_stream_params_t* stream_params,
  const char* func, uint32_t line);
boolean cpp_module_util_map_buffer_info(void *d1, void *d2);
boolean cpp_module_util_free_buffer_info(void *d1, void *d2);
boolean cpp_module_util_create_hw_stream_buff(void *d1, void *d2);
boolean cpp_module_invalidate_q_traverse_func(void* qdata, void* input);
boolean cpp_module_release_ack_traverse_func(void* data, void* userdata);
boolean cpp_module_key_list_free_traverse_func(void* data, void* userdata);
int32_t cpp_module_update_hfr_skip(cpp_module_stream_params_t *stream_params);
int32_t cpp_module_set_output_duplication_flag(
  cpp_module_stream_params_t *stream_params);
int32_t cpp_port_get_linked_info(cpp_module_ctrl_t *ctrl,
  mct_port_t *port, uint32_t identity, cpp_module_link_info_t *linked_info);
boolean cpp_port_decide_src(mct_module_t *module,
  mct_port_t *port, uint32_t identity);
pproc_divert_info_t *cpp_module_get_divert_info(uint32_t *identity_list,
  uint32_t identity_list_size, cpp_divert_info_t *cpp_divert_info);
int32_t cpp_module_util_post_diag_to_bus(mct_module_t *module,
  ez_pp_params_t *cpp_params, uint32_t identity);
int32_t cpp_module_util_update_session_diag_params(mct_module_t *module,
  cpp_hardware_params_t* hw_params);
int32_t cpp_module_util_post_crop_info(mct_module_t *module,
  cpp_hardware_params_t *hw_params,
  cpp_module_stream_params_t *stream_params);
uint32_t cpp_module_get_frame_valid(mct_module_t* module,
  uint32_t identity, uint32_t frame_id, unsigned int stream_id,
  cam_stream_type_t stream_type, boolean reset);
int32_t cpp_module_util_handle_frame_drop(mct_module_t *module,
  cpp_module_stream_params_t* stream_params, uint32_t frame_id,
  cam_hal_version_t hal_version);
boolean cpp_module_utill_free_queue_data(void *data, void *user_data);
int32_t cpp_module_util_post_to_bus(mct_module_t *module,
  mct_bus_msg_t *bus_msg, uint32_t identity);
int32_t cpp_module_util_post_metadata_to_bus(mct_module_t *module,
  cam_intf_parm_type_t type, void *parm_data, uint32_t identity);
int32_t cpp_module_get_total_load_by_value(cpp_module_ctrl_t *ctrl,
  cpp_module_total_load_t *current_load);
cpp_module_stream_clk_rate_t *
  cpp_module_find_clk_rate_by_identity(cpp_module_ctrl_t *ctrl,
    uint32_t identity);
cpp_module_stream_clk_rate_t *
cpp_module_find_clk_rate_by_value(cpp_module_ctrl_t *ctrl);
boolean cpp_module_utils_fetch_native_bufs(void *data, void *user_data);
int32_t cpp_module_handle_frame_skip_event(mct_module_t *module,
  mct_event_t *event);
int32_t cpp_module_add_report_entry(
  cpp_per_frame_params_t *per_frame_params,
  cpp_frame_ctrl_data_t *frame_ctrl_data, uint32_t cur_frame_id,
  uint32_t identity);
int32_t cpp_module_post_sticky_meta_entry(cpp_module_ctrl_t *ctrl,
  uint32_t identity, cpp_per_frame_params_t *per_frame_params,
  uint32_t cur_frame_id, cam_intf_parm_type_t type);
boolean cpp_module_check_queue_compatibility(cpp_module_ctrl_t *ctrl,
  cam_stream_type_t process_stream_type, uint32_t process_identity,
  uint32_t queue_identity);
boolean cpp_module_pop_per_frame_entry(cpp_module_ctrl_t *ctrl,
  cpp_per_frame_params_t *per_frame_params, uint32_t q_idx,
  uint32_t cur_frame_id, cpp_frame_ctrl_data_t **frame_ctrl_data,
  cpp_module_stream_params_t *stream_params);
void cpp_module_free_stream_based_entry(cpp_module_ctrl_t *ctrl,
  cam_stream_type_t stream_type, cpp_per_frame_params_t *per_frame_params);
void cpp_module_util_update_asf_params(cpp_hardware_params_t *hw_params,
  bool asf_mask);
void cpp_module_util_calculate_scale_ratio(cpp_hardware_params_t *hw_params,
  float *isp_scale_ratio, float *cpp_scale_ratio);
int32_t cpp_module_util_update_chromatix_pointer(
 cpp_module_stream_params_t  *stream_params,
 cpp_module_stream_params_t  *dup_stream_params,
 modulesChromatix_t *chromatix_ptr, float cpp_scale_ratio);
bool cpp_module_util_is_two_pass_reprocess(
  cpp_module_stream_params_t *stream_params);
int32_t cpp_module_util_update_plane_info(cpp_hardware_params_t *hw_params,
  cpp_params_dim_info_t *dim_info, cam_frame_len_offset_t *plane_info);
int32_t cpp_module_util_configure_clock_rate(cpp_module_ctrl_t *ctrl,
  uint32_t perf_mode, uint32_t *index, long unsigned int *clk_rate);
int32_t cpp_module_util_post_error_to_bus(mct_module_t *module,
  uint32_t identity);
boolean cpp_module_util_check_duplicate(
  cpp_module_stream_params_t *stream_params1,
  cpp_module_stream_params_t *stream_params2,
  uint32_t version);
boolean cpp_module_util_decide_proc_frame_per_stream(mct_module_t *module,
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params, uint32_t frame_id);
void cpp_module_util_update_stream_frame_id(
  cpp_module_stream_params_t *stream_params, uint32_t frame_id);
uint32_t cpp_module_util_decide_divert_id(mct_module_t *module,
  cpp_module_stream_params_t *stream_params);
boolean cpp_module_util_check_link_streamon(
  cpp_module_stream_params_t *stream_params);
int32_t cpp_module_util_pop_buffer(cpp_module_ctrl_t *ctrl,
  cpp_module_stream_params_t* stream_params, uint32_t frame_id);
int32_t cpp_module_port_mapping(mct_module_t *module, mct_port_direction_t,
  mct_port_t *port, uint32_t identity);
bool cpp_module_utils_get_hysteresis_trigger(float trigger_input,
  float trigger_start, float trigger_end, bool prev_state);
int32_t cpp_module_util_get_hystersis_info(mct_module_t *module,
  mct_event_t *event);
int32_t cpp_module_util_check_per_frame_limits(cpp_module_ctrl_t *ctrl,
  uint32_t identity, uint32_t cur_frame_id, cam_stream_ID_t *valid_stream_ids);
void cpp_module_util_update_asf_region(cpp_module_session_params_t
  *session_params, cpp_module_stream_params_t *stream_params,
  cpp_hardware_params_t *hw_params);
void cpp_module_util_dumps_feature_info(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params,
  struct cpp_frame_info_t* frame_info,
  struct msm_cpp_frame_info_t *msm_cpp_frame_info);
boolean cpp_module_util_is_cpp_downscale_enabled(uint32_t width,
  uint32_t height, cam_stream_type_t stream_type,
  cam_feature_mask_t pp_mask);
void cpp_module_util_get_max_downscale_dim(uint32_t width,
  uint32_t height, uint32_t *max_width, uint32_t *max_height,
  cam_feature_mask_t pp_mask, cam_format_t format);
int32_t cpp_module_util_check_event_params(mct_module_t *module,
  mct_event_t *event,
  cpp_module_session_params_t **session_params,
  cpp_module_stream_params_t **stream_params);

int32_t cpp_module_util_inc_clock_counter(
  cpp_module_session_params_t  *session_params,
  cpp_module_stream_params_t  *stream_params);

int32_t cpp_module_util_inc_clock_counter_l(
  cpp_module_session_params_t  *session_params,
  cpp_module_stream_params_t  *stream_params);

int32_t cpp_module_util_dec_stream_clock_counter(
  cpp_module_session_params_t  *session_params,
  cpp_module_stream_params_t  *stream_params);

int32_t cpp_module_util_dec_stream_clock_counter_l(
  cpp_module_session_params_t  *session_params,
  cpp_module_stream_params_t  *stream_params);

int32_t cpp_module_decide_runtime_clock_update(
  cpp_module_ctrl_t *ctrl,
  cpp_module_session_params_t *session_params,
  int32_t perf_bottleneck, int32_t dup_status,
  int32_t bump_down, int32_t offline, int32_t crop_based);

int32_t cpp_module_init_clock_parameters(
  cpp_module_stream_params_t *stream_params, int64_t dim);

int32_t cpp_module_init_clock_parameters_l(
  cpp_module_stream_params_t *stream_params, int64_t dim);

int32_t cpp_module_update_process_delay_l(
  cpp_module_ctrl_t *ctrl,
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params,
  cpp_module_stream_clk_rate_t *clk_rate_obj,
  bool perf_bottleneck);

int32_t cpp_module_update_process_delay(
  cpp_module_ctrl_t *ctrl,
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params,
  cpp_module_stream_clk_rate_t *clk_rate_obj,
   bool perf_bottleneck);

int32_t cpp_module_is_clock_update_needed(
  cpp_module_ctrl_t *ctrl,
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params,
  int64_t clk, int64_t bw, int32_t force);

int32_t cpp_module_update_threshold_idx(
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params);

int32_t cpp_module_determine_perf_bottlneck(
  cpp_module_ctrl_t *ctrl,
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params);

int32_t cpp_module_handle_request_based_fallback(
  cpp_module_ctrl_t *ctrl,
  cpp_module_session_params_t *session_params);

int32_t cpp_module_util_dec_clock_counter_l(
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params);

int32_t cpp_module_util_dec_clock_counter(
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params);

cpp_module_stream_clk_rate_t * cpp_module_get_clk_obj (
  cpp_module_ctrl_t *ctrl,
  cpp_module_stream_params_t *stream_params);

int32_t cpp_module_update_clock_status(cpp_module_ctrl_t *ctrl,
  cpp_clock_state state);

int32_t cpp_module_get_round_clock_rate(cpp_module_ctrl_t *ctrl,
  unsigned long *clk_rate, unsigned long *round_clk_rate,
  uint32_t *clock_index);

int32_t cpp_module_clock_bump(cpp_module_ctrl_t *ctrl,
   cpp_module_total_load_t *load, unsigned long *clk_rate,
   uint32_t clock_indx);

boolean cpp_module_invalidate_clock_q_traverse(void* qdata, void* userdata);

int32_t cpp_invalidate_clock_event(cpp_module_ctrl_t *ctrl, mct_queue_t *queue);

int32_t cpp_module_set_clock_dcvs_params(cpp_module_ctrl_t *ctrl);

boolean cpp_module_util_is_cpp_proc_need(mct_module_t *module,
   cpp_module_stream_params_t *stream_param,
   cpp_module_stream_params_t *linked_stream_param);

void cpp_module_util_update_dim(mct_stream_info_t *stream_info,
  cpp_params_dim_info_t *dim_info);

int cpp_module_util_update_format(mct_stream_info_t *stream_info,
  cpp_params_dim_info_t *dim_info);

void cpp_module_util_update_feature_mask(cam_pp_feature_config_t *pp_config,
  cpp_hardware_params_t *hw_params);

float cpp_module_get_bytes_per_pixel(cpp_params_plane_fmt_t format);

int32_t cpp_module_handle_intra_link_session(mct_module_t* module,
   mct_event_t* event);

int32_t cpp_module_handle_master_info(mct_module_t* module,
  mct_event_t* event);

void cpp_module_util_apply_fixed_fov_margin(
  sensor_isp_stream_sizes_t *sensor_isp_info, uint32_t si);

int cpp_module_util_get_sac_margins(
  cam_frame_margins_t *margins,
  cpp_module_session_params_t *session_params);

void cpp_module_util_apply_sac_margin(
  sensor_isp_stream_sizes_t *sensor_isp_info,
  uint32_t si,
  const cam_frame_margins_t *sac_crop_margins);

void cpp_module_util_cache_sac_margin_to_stream(
  cpp_module_stream_params_t *stream_params,
  const cpp_module_session_params_t *session_params);

void cpp_module_util_apply_cpp_downscaler_margin(
  sensor_isp_stream_sizes_t *sensor_isp_info, uint32_t si);

/*
 *Declare TNR init function
 */
void cpp_module_tnr_init(uint32_t hw_caps,
  cpp_submodule_func_tbl_t *tnr_module_func_tbl);
/* -------------------------------------------------------------------------*/

/*----------------------------prescalar init function---------------------------*/
void cpp_module_pbf_init(uint32_t hw_caps,
  cpp_submodule_func_tbl_t *pbf_module_func_tbl);

uint32_t cpp_module_util_get_posteis(void);

uint32_t cpp_module_util_is_slave_proc_required(
  cpp_module_session_params_t *session_params,
  cpp_module_stream_params_t *stream_params);

#endif
