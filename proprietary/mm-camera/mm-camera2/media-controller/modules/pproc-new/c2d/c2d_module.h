/*============================================================================

  Copyright (c) 2013-2017, 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef C2D_MODULE_H
#define C2D_MODULE_H

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
#include "mct_module.h"
#include "mct_pipeline.h"
#include "mct_stream.h"
#include "camera_dbg.h"
#include "c2d_port.h"
#include "c2d_thread.h"
#include "c2d_hardware.h"
#include "c2d_interface.h"
#include "c2d2.h"
#include "c2dExt.h"
#include "pproc_module.h"
#include "pp_buf_mgr.h"

#define READ_FD   0
#define WRITE_FD  1

#define C2D_MODULE_MAX_STREAMS   16
#define C2D_MODULE_MAX_SESSIONS  12

#define MODULE_C2D_MIN_NUM_PP_BUFS 1

#define C2D_NUM_NATIVE_BUFFERS      5

#define C2D_NUM_FRAME_HOLD      2

/** CEIL_FL1
*    @x: image to be converted
*
*    Ceil the image to one decimal point.
*    For eq:- 1.12 will be converted to 1.2
**/
#define CEIL_FL1(x) (((float)((long long)((x * 10) + .5f)) / 10))

/** CALC_MARGIN
 *   @i: input
 *   @o: output
 *
 *   Returns margin between input and output
 **/
#define CALC_MARGIN(i, o) (CEIL_FL1(((float)(i - o) / (float) o)))

/* macros for unpacking identity */
#define C2D_GET_STREAM_ID(identity) ((identity) & 0xFFFF)
#define C2D_GET_SESSION_ID(identity) (((identity) & 0xFFFF0000) >> 16)
#define PER_FRAME_QUEUE_SIZE 16

#define IF_EIS_ENABLE(session_params, stream_params) \
  (((stream_params->stream_type == CAM_STREAM_TYPE_PREVIEW) || \
    (stream_params->stream_type == CAM_STREAM_TYPE_VIDEO)) && \
    (stream_params->stream_info->is_type == IS_TYPE_EIS_2_0) && \
    (stream_params->dis_enable)) ? 1 : 0


#define IS_LINKED_DIMENSION_BIGGER_OR_EQUAL(stream_params) \
  ((stream_params->linked_stream_params->stream_info->dim.width >= \
    stream_params->stream_info->dim.width) || \
    (stream_params->linked_stream_params->stream_info->dim.height >= \
    stream_params->stream_info->dim.height) || \
    (stream_params->linked_stream_params->stream_info-> \
    buf_planes.plane_info.mp[0].stride >= \
    stream_params->stream_info->buf_planes.plane_info.mp[0].stride) || \
    (stream_params->linked_stream_params->stream_info-> \
    buf_planes.plane_info.mp[0].scanline >= \
    stream_params->stream_info->buf_planes.plane_info.mp[0].scanline)) \
  ? 1 : 0

#define WIDTH_4K 3840
#define HEIGHT_4K 2160
#define IS_4k_VIDEO(stream, width, height, mask) \
  (((stream == CAM_STREAM_TYPE_VIDEO) && \
    (((width >= WIDTH_4K) && (height >= HEIGHT_4K)) || \
    (((width >= HEIGHT_4K) && (height >= WIDTH_4K)) && \
    (mask & CAM_QCOM_FEATURE_ROTATION)))) \
    ? 1 : 0 \
    )

#define MOVE_TO_ORIGIN_AT_CENTER(point, dim) \
  { \
    (point)->x -= (dim)->width / 2; \
    (point)->y -= (dim)->height / 2; \
  }
#define MOVE_TO_ORIGIN_AT_TOPLEFT(point, dim) \
  { \
    (point)->x += (dim)->width / 2; \
    (point)->y += (dim)->height / 2; \
  }

#define MULTIPLY_MATRIX(in_point, transform, epsilon, out_point) \
{ \
  (out_point)->x = \
    (transform[0] * (in_point)->x) + \
    (transform[1] * (in_point)->y) + \
    (transform[2] * 1); \
  (out_point)->y = \
    (transform[3] * (in_point)->x) + \
    (transform[4] * (in_point)->y) + \
    (transform[5] * 1); \
  float xy = \
    (transform[6] * (in_point)->x) + \
    (transform[7] * (in_point)->y) + \
    (transform[8] * 1); \
  if (fabs(xy) > epsilon) { \
    (out_point)->x /= xy; \
    (out_point)->y /= xy; \
  } \
}

typedef enum {
  C2D_DIVERT_UNPROCESSED,
  C2D_DIVERT_PROCESSED
} c2d_divert_type_t;

typedef enum {
  C2D_PRIORITY_REALTIME,
  C2D_PRIORITY_OFFLINE
} c2d_priority_t;

/* aysnchronous events are the event processed by
  c2d_thread from the event_queue */
typedef enum {
  C2D_MODULE_EVENT_PROCESS_BUF,
  C2D_MODULE_EVENT_DIVERT_BUF,
  C2D_MODULE_EVENT_STREAM_OFF,
} c2d_module_event_type_t;

typedef struct _c2d_module_streamoff_data_t {
  mct_stream_info_t     *stream_info;
  uint32_t               identity;
} c2d_module_streamoff_data_t;

typedef struct _c2d_module_event_process_buf_data_t {
  isp_buf_divert_t       isp_buf_divert;
  uint32_t               proc_identity;
  mct_stream_info_t     *input_stream_info;
  mct_stream_info_t     *stream_info;
  boolean                proc_div_required;
  uint32_t               proc_div_identity;
  c2d_hardware_params_t  hw_params;
} c2d_module_event_process_buf_data_t;

typedef struct _c2d_module_event_divert_buf_data_t {
  isp_buf_divert_t  isp_buf_divert;
  uint32_t          div_identity;
} c2d_module_event_divert_buf_data_t;

/* key to find the ACK from list */
typedef struct _c2d_module_ack_key_t {
  uint32_t identity;
  uint32_t buf_idx;
  int32_t channel_id;
  uint32_t buf_identity;
} c2d_module_ack_key_t;

typedef struct _c2d_module_hw_cookie_t {
  c2d_module_ack_key_t  key;
  boolean               proc_div_required;
  uint32_t              proc_div_identity;
} c2d_module_hw_cookie_t;

/* data structure for events stored in c2d queue */
typedef struct _c2d_module_async_event_t {
  c2d_module_event_type_t type;
  /* invalid bit used for stream-off handling */
  boolean invalid;
  /* indicates if c2d hw processing is needed for this event */
  boolean hw_process_flag;
  /* key to find corresponding ack */
  c2d_module_ack_key_t ack_key;

  /* event specific data based on type */
  union {
    c2d_module_event_process_buf_data_t     process_buf_data;
    c2d_module_event_divert_buf_data_t      divert_buf_data;
    c2d_module_streamoff_data_t             streamoff_data;
  } u;
} c2d_module_event_t;

/* data structure to store the ACK in list */
typedef struct _c2d_module_ack_t {
  isp_buf_divert_ack_t isp_buf_divert_ack;
  int32_t ref_count;
  int32_t frame_id;
  struct timeval in_time, out_time;
} c2d_module_ack_t;

/* queue for handling async events */
typedef struct _c2d_module_event_queue_t {
  mct_queue_t       *q;
  pthread_mutex_t   mutex;
} c2d_module_event_queue_t;

/* list for storing ACKs */
typedef struct _c2d_module_ack_list_t {
  mct_list_t        *list;
  uint32_t           size;
  pthread_mutex_t    mutex;
} c2d_module_ack_list_t;

/* Information about frame skipping for HFR */
typedef struct _c2d_module_hfr_skip_info_t {
  boolean      skip_required;
  int32_t      skip_count;
  uint32_t     frame_offset;
  float        output_fps;
  float        input_fps;
} c2d_module_hfr_skip_info_t;

typedef struct _c2d_module_stream_params_t c2d_module_stream_params_t;
#if 0
typedef struct _c2d_gpu_buf_t{
  uint16_t num_planes;
  union {
    uint32_t sp_gAddr;
    uint32_t mp_gAddr[MAX_PLANES];
  };
} c2d_gpu_buf_t;

typedef struct _c2d_module_libparams {
  uint32_t id;
  C2D_YUV_SURFACE_DEF surface_def;
  C2D_SURFACE_BITS surface_type;
  c2d_gpu_buf_t gpu_buf;
  C2D_YUV_FORMAT format;
} c2d_module_libparams;
#endif

typedef struct _c2d_module_frame_hold_t {
  /* frame hold flag, if flag is TRUE, then isp_buf holds frame */
  boolean          is_frame_hold;
  /* Store frame until DIS event for this frame is received */
  uint32_t     identity;
  isp_buf_divert_t isp_buf;
} c2d_module_frame_hold_t;


typedef struct _c2d_module_dis_hold_t {
  /* DIS hold flag, if is_valid is TRUE, then dis_frame_id has valid frame id */
  boolean      is_valid;
  /* Store frame until DIS event for this frame is received */
  uint32_t     dis_frame_id;
} c2d_module_dis_hold_t;

/* stream specific parameters */
struct _c2d_module_stream_params_t {
  void                         *parent;
  c2d_hardware_params_t        hw_params;
  uint32_t                     identity;
  c2d_priority_t               priority;
  cam_stream_type_t            stream_type;
  boolean                      is_stream_on;
  pthread_mutex_t              mutex;
  c2d_module_hfr_skip_info_t   hfr_skip_info;
  boolean                      out_dim_initialized;
  mct_stream_info_t            *stream_info;
  /* linked stream is the stream which shares same ISP input buffer.
     This is usually mapped on same port */
  c2d_module_stream_params_t  *linked_stream_params;
  c2d_libparams               c2d_input_lib_params;
  c2d_libparams               c2d_output_lib_params;
  boolean                      native_buff;
  uint32_t                     streaming_identity;
  int32_t                      interleaved;
  boolean                      single_module;
  int32_t                      dis_enable;
  /* Latest frame id received from DIS crop event */
  c2d_module_dis_hold_t        dis_hold;
  c2d_module_frame_hold_t      frame_hold[C2D_NUM_FRAME_HOLD];
  uint32_t                     frame_hold_cnt;
  uint32_t                     frame_hold_f_idx;
  uint32_t                     frame_hold_r_idx;
};

/* session specific parameters */
typedef struct _c2d_module_session_params_t {
  c2d_module_stream_params_t   *stream_params[C2D_MODULE_MAX_STREAMS];
  int32_t                       stream_count;
  c2d_hardware_params_t         hw_params;
  uint32_t                      session_id;
  cam_hfr_mode_t                hfr_mode;
  cam_hal_version_t             hal_version;
  /* Hold frame until DIS crop is received for this frame */
  int32_t                       ion_fd;
  cam_fps_range_t               fps_range;
  int32_t                       dis_enable;
  cam_stream_ID_t               valid_stream_ids[PER_FRAME_QUEUE_SIZE];
  uint32_t                      c2d_delay;
  uint32_t                      max_apply_delay;
  uint32_t                      max_report_delay;
  pthread_mutex_t               c2d_per_frame_mutex;
  uint32_t                      real_time_stream_cnt;
  boolean                       c2d_debug_enable;
} c2d_module_session_params_t;

typedef struct _c2d_module_buffer_info {
  int fd;
  uint32_t index;
  uint32_t offset;
  uint8_t native_buff;
  uint8_t processed_divert;
} c2d_module_buffer_info_t;

typedef struct _c2d_module_stream_buff_info {
  uint32_t identity;
  uint32_t num_buffs;
  mct_list_t *buff_list;
} c2d_module_stream_buff_info_t;

typedef struct _c2d_module_ctrl_t {
  mct_module_t                *p_module;
  c2d_module_event_queue_t    realtime_queue;
  c2d_module_event_queue_t    offline_queue;
  c2d_module_ack_list_t       ack_list;
  pthread_t                   c2d_thread;
  pthread_cond_t              th_start_cond;
  boolean                     c2d_thread_started;
  pthread_mutex_t             c2d_mutex;
  int                         pfd[2];
  int32_t                     session_count;
  void                        *buf_mgr;
  pproc_interface_lib_params_t *c2d;
  void                        *c2d_ctrl;
  c2d_hardware_t              *c2dhw;
  c2d_module_session_params_t *session_params[C2D_MODULE_MAX_SESSIONS];
  pp_native_buf_mgr_t          native_buf_mgr;
} c2d_module_ctrl_t;

typedef struct _c2d_fpoint {
  float x;
  float y;
} c2d_fpoint;

mct_module_t* c2d_module_init(const char *name);
void c2d_module_deinit(mct_module_t *mod);
static void c2d_module_set_mod (mct_module_t *module, mct_module_type_t module_type,
  uint32_t identity);
static boolean c2d_module_query_mod(mct_module_t *module, void *query_buf,
  uint32_t sessionid);
static boolean c2d_module_start_session(mct_module_t *module,
  uint32_t sessionid);
static boolean c2d_module_stop_session(mct_module_t *module,
  uint32_t sessionid);
static c2d_module_ctrl_t* c2d_module_create_c2d_ctrl(void);
static int32_t c2d_module_destroy_c2d_ctrl(c2d_module_ctrl_t *ctrl);



static int32_t c2d_module_send_ack_event_upstream(c2d_module_ctrl_t *ctrl,
  isp_buf_divert_ack_t isp_ack);

int32_t c2d_module_post_msg_to_thread(mct_module_t *module,
  c2d_thread_msg_t msg);
int32_t c2d_module_process_downstream_event(mct_module_t* module,
  mct_event_t *event);
int32_t c2d_module_process_upstream_event(mct_module_t* module,
  mct_event_t *event);
int32_t c2d_module_send_event_downstream(mct_module_t* module,
   mct_event_t* event);
int32_t c2d_module_send_event_upstream(mct_module_t* module,
   mct_event_t* event);
int32_t c2d_module_notify_add_stream(mct_module_t* module, mct_port_t* port,
  mct_stream_info_t* stream_info, void *peer_caps);
int32_t c2d_module_notify_remove_stream(mct_module_t* module,
  uint32_t identity);
int32_t c2d_module_do_ack(c2d_module_ctrl_t *ctrl,
  c2d_module_ack_key_t key, uint32_t buffer_access);
int32_t c2d_module_invalidate_queue(c2d_module_ctrl_t* ctrl,
  uint32_t identity);
int32_t c2d_module_put_new_ack_in_list(c2d_module_ctrl_t *ctrl,
  c2d_module_ack_key_t key, int32_t buf_dirty, int32_t ref_count,
  isp_buf_divert_t *isp_buf);
int32_t c2d_module_enq_event(mct_module_t* module,
  c2d_module_event_t* event, c2d_priority_t prio);
int32_t c2d_module_reorder_q_event(mct_module_t* module,
  c2d_module_event_t* c2d_event, c2d_priority_t prio);
int32_t c2d_module_handle_sof_set_parm_event(mct_module_t* module,
  mct_event_t* event);
int32_t c2d_module_check_interleaved_stream(mct_module_t* module,
  uint32_t identity);
int32_t c2d_module_init_native_buffers(c2d_module_ctrl_t *ctrl,
  cam_frame_len_offset_t *plane_info, uint32_t identity,
  int32_t buf_count);
int32_t c2d_module_handle_dis_config_event(mct_module_t* module,
  mct_event_t* event);
int32_t c2d_module_set_output_stream_parameters(
  c2d_module_ctrl_t *ctrl,
  c2d_module_session_params_t *session_params,
  c2d_module_stream_params_t *stream_params);
int32_t c2d_module_update_output_dimension(
  c2d_module_ctrl_t *ctrl,
  c2d_hardware_params_t *hw_params,
  c2d_module_stream_params_t *stream_params,
  uint32_t width, uint32_t height, bool custom_param);
uint32_t c2d_module_get_frame_valid(mct_module_t* module,
  uint32_t identity, uint32_t frame_id, unsigned int stream_id,
  cam_stream_type_t stream_type);

/* ----------------------- utility functions -------------------------------*/

mct_port_t* c2d_module_find_port_with_identity(mct_module_t *module,
  mct_port_direction_t dir, uint32_t identity);
c2d_module_ack_t* c2d_module_find_ack_from_list(c2d_module_ctrl_t *ctrl,
  c2d_module_ack_key_t key);
cam_streaming_mode_t c2d_module_get_streaming_mode(mct_module_t *module,
  uint32_t identity);
int32_t c2d_module_get_params_for_identity(c2d_module_ctrl_t* ctrl,
  uint32_t identity, c2d_module_session_params_t** session_params,
  c2d_module_stream_params_t** stream_params);
void c2d_module_dump_stream_params(c2d_module_stream_params_t* stream_params,
  const char* func, int line);
boolean c2d_module_util_map_buffer_info(void *d1, void *d2);
boolean c2d_module_util_free_buffer_info(void *d1, void *d2);
boolean c2d_module_util_create_hw_stream_buff(void *d1, void *d2);
boolean c2d_module_invalidate_q_traverse_func(void* qdata, void* input);
int32_t c2d_port_get_linked_identity(mct_port_t *port, uint32_t identity,
  uint32_t *linked_identity);
int32_t c2d_module_update_hfr_skip(c2d_module_stream_params_t *stream_params);
pproc_divert_info_t *c2d_module_get_divert_info(uint32_t *identity_list,
  uint32_t identity_list_size, c2d_divert_info_t *c2d_divert_info);
boolean c2d_module_release_ack_traverse_func(void* data, void* userdata);
boolean c2d_module_key_list_free_traverse_func(void* data, void* userdata);
int32_t c2d_module_update_hfr_skip(c2d_module_stream_params_t *stream_params);
int32_t c2d_module_util_update_format(mct_stream_info_t* stream_info, int *fmt);
/* -------------------------------------------------------------------------*/

#endif
