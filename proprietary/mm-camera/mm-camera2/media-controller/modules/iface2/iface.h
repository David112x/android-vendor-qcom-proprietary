/*============================================================================
Copyright (c) 2013-2016,2019-2020 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef __IFACE_H__
#define __IFACE_H__

#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "iface_def.h"
#include "iface_buf_mgr.h"
#include "iface_ops.h"
#include "axi/iface_axi.h"
#include "pdaf_lib.h"
#include "iface_hvx.h"

#ifdef _ANDROID_

  #include <cutils/properties.h>

  #define iface_read_setprop_int(OUTPUT, STRING, DEFAULT) do {  \
    char set_prop_value[PROPERTY_VALUE_MAX];                    \
    property_get((STRING), set_prop_value, (DEFAULT));          \
    (OUTPUT) = atoi(set_prop_value);                            \
  } while(0);

  #define iface_read_setprop_string(STR, VALUE, DEFAULT)        \
    property_get((STR), (VALUE), (DEFAULT))

#else

  #define PROPERTY_VALUE_MAX 32

  #define iface_read_setprop_int(OUTPUT, STR, DEFAULT)         \
    OUTPUT = (atoi(DEFAULT))

  #define iface_read_setprop_string(STR, VALUE, DEFAULT)       \
    strlcpy((VALUE), "\0", sizeof(VALUE))

#endif /* _ANDROID_ */

#define UNPACK_STREAM_ID(identity) (identity & 0x0000FFFF)
#define UNPACK_SESSION_ID(identity) ((identity & 0xFFFF0000) >> 16)

#define IFACE_ISP40_NUM_REG_DUMP 576 /* 0x900(hex) = 2304 / 4(4 byte per register) */
#define IFACE_ISP32_NUM_REG_DUMP 576

#define IFACE_ISP_PIX_CLK_320MHZ 320000000
#define IFACE_SINK_PORTS_NUM 8
#define IFACE_SRC_PORTS_NUM  10

#define IFACE_MAX_NATIVE_BUF_NUM 4
#define IFACE_MAX_NATIVE_HFR_BUF_NUM 6
#define IFACE_MIN_NATIVE_BUF_NUM 2
#define NUM_AXI_WM 7
#define IFACE_UTIL_INPUT_Q_SIZE 1

#define IFACE_FRAME_CTRL_SIZE 6
#define IFACE_APPLY_DELAY 1
#define SKIP_CDS_FRAMES_THRESHOLD 6

#define IFACE_MAX_OFFLINE_PASS 2

#define IS_DUAL_VFE(resource) (((resource) > 1) ? TRUE : FALSE)

#define WHICH_VFE(resource) (((resource) & (1 << VFE0)) ? VFE0 : \
  (((resource) & (1 << VFE1)) ? VFE1 : VFE_MAX))

extern volatile iface_loglevel_t g_ifaceloglevel;

#ifdef _ANDROID_
  #undef CDBG_ERROR
  #undef CDBG_HIGH
  #undef CDBG
  #undef CDBG_INFO
  #undef CDBG_WARN
  #define CDBG_INFO(fmt, args...) MDBG_INFO(CAM_IFACE_MODULE, fmt, ##args)
  #define CDBG_WARN(fmt, args...) MDBG_WARN(CAM_IFACE_MODULE, fmt, ##args)
  #define CDBG_ERROR(fmt, args...) MDBG_ERROR(CAM_IFACE_MODULE, fmt, ##args)
  #define CDBG_HIGH(fmt, args...) MDBG_HIGH(CAM_IFACE_MODULE, fmt, ##args)
  #define CDBG(fmt, args...) MDBG(CAM_IFACE_MODULE, fmt, ##args)
#else
  #include <stdio.h>
  #define CDBG_INFO(fmt, args...) fprintf(stderr, fmt, ##args)
  #define CDBG_WARN(fmt, args...) fprintf(stderr, fmt, ##args)
  #define CDBG_ERROR(fmt, args...) fprintf(stderr, fmt, ##args)
  #define CDBG(fmt, args...) fprintf(stderr, fmt, ##args)
  #define CDBG_HIGH(fmt, args...) fprintf(stderr, fmt, ##args)
#endif

typedef enum {
  IFACE_STREAM_NOTUSED,
  IFACE_STREAM_CREATED,
  IFACE_STREAM_ASSOCIATED_WITH_SINK_PORT,
  IFACE_STREAM_ASSOCIATED_WITH_SRC_PORT,
  IFACE_STREAM_MAX
} iface_stream_state_t;

typedef enum {
  IFACE_ASYNC_COMMAND_INVALID,
  IFACE_ASYNC_COMMAND_UV_SUBSAMPLE, /* uv subsampling - uint32_t */
  IFACE_ASYNC_COMMAND_STREAMON,     /* isp_session_cmd_streamon_t */
  IFACE_ASYNC_COMMAND_OFFLINE_CONFIG,     /* iface_session_cmd_offline_cfg_t */
  IFACE_ASYNC_COMMAND_STREAMOFF,    /* */
  IFACE_ASYNC_COMMAND_RECOVERY,    /* */
  IFACE_ASYNC_COMMAND_RESOURCE_ALLOC,    /* */
  IFACE_ASYNC_COMMAND_EXIT,         /* exit the thraed loop */
  IFACE_ASYNC_COMMAND_MAX           /* not used */
} iface_async_cmd_id_t;

typedef enum {
   IFACE_SENSOR_META_RDI,
   IFACE_SENSOR_META_RAW
} iface_sensor_meta_cfg_cmd_t;

/** iface_port_state_t:
  *
  *  @IFACE_PORT_STATE_CREATED: port is created
  *  @IFACE_PORT_STATE_RESERVED: port is reserved
  *  @IFACE_PORT_STATE_START_PENDING: port is ready to start
  *  @IFACE_PORT_STATE_ACTIVE: port is active
  *  @IFACE_PORT_STATE_MAX:
  *
  * iface port states enumeration
  **/
typedef enum {
  IFACE_PORT_STATE_CREATED,
  IFACE_PORT_STATE_RESERVED,
  IFACE_PORT_STATE_HW_CONFIGED,
  IFACE_PORT_STATE_ACTIVE,
  IFACE_PORT_STATE_MAX
} iface_port_state_t;

typedef struct {
  uint32_t isp_version;
  iface_isp_hw_cap_t hw_cap;
  char subdev_name[IFACE_SUBDEV_NAME_LEN];
} iface_isp_subdevs_t;

typedef struct {
  uint32_t active_count[INTF_MAX];
} iface_ispif_intf_state_t;

typedef struct {
  pthread_mutex_t mutex;
  int open_count;
  int fd;
  int num_active_streams;
  sensor_meta_data_t meta_info;
  uint32_t meta_identity;
  struct ispif_cfg_data cfg_cmd;
  struct msm_ispif_param_data_ext params_ext;
  boolean meta_pending;
  iface_ispif_intf_state_t interface_state[VFE_MAX];
} iface_ispif_t;

typedef struct {
  uint32_t num_subdev;
  char subdev_name[32];
  iface_ispif_t ispif_hw;
} iface_ispif_data_t;

typedef struct {
  iface_ops_t *axi_hw_ops;
  iface_notify_ops_t notify_ops;
  int ref_cnt;
  int fd;
  uint32_t interface_clk_request[IFACE_INTF_MAX];;
  pthread_mutex_t mutex;
} iface_axi_t;

typedef struct {
  uint32_t num_subdev;
  iface_isp_subdevs_t isp_subdev[IFACE_ISP_SD_NODE_ID_MAX_NUM];
  iface_axi_t axi_data[VFE_MAX];
} iface_isp_axi_data_t;

typedef struct {
  cam_hfr_mode_t hfr_mode;  /* HFR mode */
  boolean pp_do_frame_skip;
} iface_hfr_param_t;

typedef struct {
  void *iface;
  void *session;
  uint32_t session_id;
  uint32_t isp_id;
  uint32_t frame_id;
} iface_session_cmd_recovery_cfg_t;

typedef struct {
  void *iface;
  void *iface_sink_port;
  void *session;
  uint32_t session_id;
  uint32_t stream_id;
  boolean  is_first_streamon;
  mct_event_t *event;
  mct_port_t * mct_port;
} iface_session_cmd_stream_cfg_t;

typedef struct {
  cam_dimension_t           input_dim;
  cam_format_t              input_fmt;
  uint32_t                  input_stream_id;
  mct_stream_info_t         input_stream_info;
  uint32_t                  offline_frame_id;
  uint32_t                  op_pixel_clk;
  cam_stream_parm_buffer_t *stream_param_buf;
  mct_list_t               *offline_img_buffer_list;
  boolean                   offline_mode;
  uint32_t                  fd;
#ifdef VIDIOC_MSM_ISP_MAP_BUF_START_MULTI_PASS_FE
  enum msm_vfe_fetch_eng_pass offline_pass;
#endif
  uint32_t output_buf_idx;
  uint32_t input_buf_offset;
  uint32_t output_stream_id;
} iface_offline_isp_info_t;

typedef struct {
  void *iface;
  void *iface_sink_port;
  uint32_t session_id;
  uint32_t stream_id;
  iface_offline_isp_info_t *offline_info;
} iface_session_cmd_offline_cfg_t;

typedef struct {
  void *iface;
  uint32_t session_id;
  uint32_t stream_id;
  iface_sw_frameskip *sw_skip_info;
} iface_session_cmd_sw_skip_t;

typedef struct {
  uint32_t enable;
} iface_session_cmd_uv_ss_t;

typedef struct {
  void *iface;
  uint32_t session_id;
  uint32_t stream_id;
  iface_buf_alloc_t buf_alloc_info;
} iface_session_cmd_native_alloc_t;

typedef struct {
  uint8_t batch_mode;
  uint32_t batch_size;
  uint32_t camif_period;
} iface_batch_info_t;

typedef struct {
  uint32_t timer_cnt;
  void *hw_ptr;
  pthread_t pid;
  //iface_tread_timer_t *timer; /* only support one timer */
  int return_code; /* return code of the pipe command */
  int async_ret;

  /*pipe.  remove????*/
  int32_t pipe_fds[2];
  int poll_fd; /* non zero if is asked to poll subdev event */
  int poll_timeoutms;
  struct pollfd poll_fds[IFACE_AXI_MAX_THREAD_POIING_FDS];
  uint8_t num_fds;

  /*data for session async thread*/
  iface_async_cmd_id_t async_cmd_id;
  /*cmd data to session thread*/
  iface_session_cmd_uv_ss_t uv_subsample_parm;
  iface_session_cmd_stream_cfg_t stream_on_off_parm;
  iface_session_cmd_recovery_cfg_t recovery_param;
  iface_session_cmd_offline_cfg_t offline_cfg_parm;
  iface_session_cmd_sw_skip_t sw_skip_cfg_parm;
  iface_session_cmd_native_alloc_t resource_alloc;
  /*to sync*/
  boolean thread_busy;
  boolean wait_for_sof;

  pthread_mutex_t  cmd_mutex;
  pthread_mutex_t  busy_mutex;
  pthread_mutex_t sof_mutex;
  sem_t sig_sem;
  sem_t thread_wait_sem; /* thread waits on this semphore */
  sem_t wait_sof_sem;
  sem_t busy_sem;
  sem_t ispif_start_sem;

  /*busy flag for sync*/
  uint32_t is_busy;
  uint32_t busy_cnt;
  uint32_t applied_frame_id; /* Frame no on which UV subsample was applied */
  boolean  is_busy_wait;
} iface_session_thread_t;

typedef struct {
  void *session;
  uint32_t session_id;
  uint32_t stream_id;
  mct_stream_info_t stream_info;
  mct_port_t *sink_port;
  mct_port_t *src_port;
  enum msm_vfe_axi_stream_src axi_path;
  iface_stream_state_t state;
  int link_cnt;
  uint8_t use_pix;
  uint32_t reserved_for_rdi;
  ispif_out_info_t split_info;
} iface_stream_t;

/** iface_sink_port_t:
  *
  *  @iface_port: MCTL base module of ispif port.
  *  @streams: streams linked to port
  *  @sensor_cfg: sensor configuration
  *  @sensor_cap: matching sensor source port capabilities
  *               this cap matches to Sensor's src cap
  *  @num_streams: number of streams
  *
  * ispif sink port data structure
  **/
typedef struct {
  mct_port_t *iface_port;
  iface_stream_t streams[IFACE_MAX_STREAMS];
  sensor_out_info_t sensor_out_info;
  sensor_src_port_cap_t sensor_cap;
  int num_streams;
} iface_sink_port_t;

typedef struct {
  mct_port_t *iface_port;
  iface_stream_t streams[IFACE_MAX_STREAMS];
  ispif_src_port_caps_t caps;   // define in module.h,  sensor cap input type and if use pix
  int num_streams;
} iface_src_port_t;

typedef struct {
  mct_port_t *port;
  void *iface;
  iface_port_state_t state;
  uint32_t session_id;
  int num_active_streams;
  union {
    iface_src_port_t src_port;
    iface_sink_port_t sink_port;
  } u;
} iface_port_t;

typedef struct {
  uint32_t user_stream_id;
  sensor_meta_data_t sensor_meta;
} iface_bundle_meta_info_t;

typedef struct {
  uint32_t user_stream_id;
  pdaf_sensor_native_info_t pdaf_config;
  cam_dimension_t block_dim;
  uint16_t left_pix_skip_pattern;
  uint16_t left_line_skip_pattern;
  uint16_t right_pix_skip_pattern;
  uint16_t right_line_skip_pattern;
  camif_fmt_t camif_output_format;
} iface_bundle_pdaf_info_t;


typedef enum {
  IFACE_CDS_STATUS_IDLE,
  IFACE_CDS_STATUS_UPDATE,
  IFACE_CDS_STATUS_REQUST_SEND,
} iface_cds_status_t;

/** iface_util_fe_input_buf_t: used to store fetch engine cfg in SVHDR
  *
  *  @hw_stream_id: hw_stream_id to get input buf
  *  @interface_mask: used for identifying isp id
  *  @buf_idx: buf_idx of buffer to be programmed on fe
  *  @frame_id: frame id of buffer being processed by fe
  *  @user_bufq_handle: user bufq handle of fe input bufq
  *
  **/
typedef struct {
  uint32_t hw_stream_id;
  uint32_t interface_mask;
  uint32_t buf_idx;
  uint32_t frame_id;
  uint32_t user_bufq_handle;
} iface_util_fe_input_buf_t;

/** iface_fe_data_t: fe input command queue for SVHDR
  *
  *  @mutex: to protect fe input command queue operations
  *  @num: current number of items in cmd queue
  *  @next_free_entry: index to next free entry in cmd queue
  *  @frame_q: mct queue
  *  @q_data: elements to be stored in queue
  *  @busy: indecates if fe is busy or not
  **/
typedef struct {
  pthread_mutex_t mutex;
  uint32_t num;
  uint32_t next_free_entry;
  mct_queue_t *frame_q;
  iface_util_fe_input_buf_t q_data[IFACE_UTIL_INPUT_Q_SIZE];
  boolean busy;
} iface_fe_data_t;

typedef struct {
  void *iface;
  uint32_t session_id;
  uint32_t peer_session_id;
  enum msm_vfe_dual_hw_ms_type ms_type;
  uint32_t num_hw_stream;
  uint32_t sof_frame_id;
  uint32_t active_count;
  sensor_out_info_t sensor_out_info;
  iface_hw_stream_t hw_streams[IFACE_MAX_STREAMS];
  uint32_t num_stats_stream;
  iface_stats_stream_t stats_streams[MSM_ISP_STATS_MAX];
  iface_stats_stream_t offline_stats_streams[MSM_ISP_STATS_MAX];
  iface_resource_t session_resource;
  int ion_fd;
  uint32_t num_bundled_streamon;
  uint32_t num_bundled_streamoff;
  cam_bundle_config_t hal_bundle_parm;
  enum msm_vfe_frame_skip_pattern hal_skip_pattern;
  enum msm_vfe_frame_skip_pattern stats_skip_pattern;
  enum msm_vfe_frame_skip_pattern bracketing_frame_skip_pattern; /* pattern sent by sensor during bracketing */
  iface_hfr_param_t hfr_param;
  boolean  cds_feature_enable;
  boolean  dynamic_stats_skip_feature_enb;
  uint32_t cds_curr_enb;
  uint32_t cds_saved_state;
  iface_cds_info_t cds_info;
  iface_cds_status_t  cds_status;
  cam_cds_mode_type_t cds_mode;
  mct_bracketing_state_t bracketing_state;
  uint32_t fast_aec_mode;
  uint32_t num_bundle_meta;
  uint32_t num_pdaf_meta;
  iface_bundle_meta_info_t bundle_meta_info[IFACE_MAX_STREAMS];
  iface_bundle_pdaf_info_t bundle_pdaf_info[IFACE_MAX_STREAMS];
  pdaf_data_buffer_info_t camif_info;
  enum smmu_attach_mode security_mode;
  modulesChromatix_t chromatix_param;
  iface_axi_recovery_state_t overflow_recovery_state;
  pthread_mutex_t mutex;
  /* thread in session*/
  iface_session_thread_t session_thread;
  uint32_t prev_sent_streamids[MAX_STREAMS_NUM];
  uint32_t prev_sent_streamids_cnt;
  cam_hal_version_t hal_version;
  uint32_t max_apply_delay;
  uint32_t max_reporting_delay;
  pthread_mutex_t parm_q_lock[IFACE_FRAME_CTRL_SIZE];
  mct_queue_t *parm_q[IFACE_FRAME_CTRL_SIZE];
  uint32_t session_streams_identity;
  uint32_t vt_enable;
  uint32_t hal_max_buffer;
  iface_offline_isp_info_t offline_info;
  boolean dual_vfe_notify[IFACE_AXI_NOTIFY_MAX][VFE_MAX];
  iface_sw_frameskip sw_skip_info;
  int skip_cds_timeout; /*ignore CDS switches for next x frames*/
  boolean cds_skip_disable;
  isp_preferred_streams preferred_mapping;
  iface_batch_info_t batch_info;
  uint8_t initial_frame_skip;
  int create_axi_count[VFE_MAX];

  iface_hvx_t hvx;
  uint32_t num_pix_streamon;
  boolean first_pix_stream_on_off;
  iface_buf_alloc_t nativbuf_alloc_info;
  iface_frame_buffer_t image_bufs[VFE_AXI_SRC_MAX][IFACE_MAX_IMG_BUF];
  uint8_t dis_enabled;
  uint32_t hw_version;
  cam_sensor_hdr_type_t hdr_mode;
  boolean  zzhdr_hw_available;
  isp_camif_caps_info camif_cap;
  boolean use_testgen; /* Indicates whether input is from CAMIF testgen */
  struct timeval time_stamp;
  struct timeval delta_time_stamp;
  uint8_t fe_done_cnt;
  iface_fe_data_t fe;  /*to store fe input command queue*/
  hw_stream_buffer_info_t reproc_buff_info; /*internal list of fe input buf*/
  boolean bayer_processing; /*indecates if bayer processing usecase or not*/
  iface_param_frame_request_t  frame_request; /*cache frame_request in svhdr mode*/
  boolean binncorr_mode;
  boolean lpm_enable;
  boolean prev_lpm_mode; /*previous lpm mode of session*/
  boolean cur_lpm_mode; /*current lpm mode of session*/
  uint32_t dual_isp_sync; /*dual isp sync enablement*/
  uint32_t prev_sof_frame_id;
  uint64_t nano_timestamp;
  uint32_t nanosec_ts_enable;
  uint32_t drop_reconfig;
  uint32_t dual_cam_drop; /*sw sync drop*/
} iface_session_t;

typedef struct {
  mct_module_t *module;
  pthread_mutex_t mutex;
  iface_buf_mgr_t buf_mgr;
  iface_ispif_data_t ispif_data;
  iface_isp_axi_data_t isp_axi_data;
  iface_session_t sessions[IFACE_MAX_SESSIONS];
  int fd;
  int num_active_streams;
  int frame_cnt;
  int skip_meta;
  iface_hvx_singleton_t hvx_singleton;
  uint32_t rdi_used_resource_mask;
  uint32_t used_wm[VFE_MAX];
  int    vfe1_reserved_rdi;
} iface_t;


/** iface_get_match_src_port_t
  *
  * Helper struct for finding matching source port
  **/
typedef struct {
  iface_port_t *iface_sink_port;
  iface_stream_t *stream;
  isp_preferred_streams *preferred_mapping;
} iface_get_match_src_port_t;

typedef struct {
  uint32_t  session_id;
  int       num_streams;
  uint32_t  mct_streamon_id[IFACE_MAX_STREAMS];
  uint32_t  stream_ids[IFACE_MAX_STREAMS];
  boolean   first_pix_start;
  uint8_t sync_frame_id_src;
  enum msm_vfe_axi_stream_cmd stop_cmd;
} start_stop_stream_t;

typedef struct {
  uint32_t session_id;
  uint32_t hw_stream_id;
  uint32_t buf_idx;
  boolean  offline_mode;
  uint32_t fd;
  uint32_t frame_id;
#ifdef VIDIOC_MSM_ISP_MAP_BUF_START_MULTI_PASS_FE
  enum msm_vfe_fetch_eng_pass offline_pass;
#endif
  uint32_t output_buf_idx;
  uint32_t input_buf_offset;
  uint32_t output_stream_id;
} triger_fetch_stream_t;

typedef struct {
  int num_hw_streams;
  uint32_t session_id;
  uint32_t hw_stream_ids[IFACE_MAX_STREAMS];
  uint32_t uv_subsample_enb;
  uint32_t stripe_id;
} iface_stream_update_t;

typedef enum {
  IFACE_PARAM_Q_HAL_PARAM,
  IFACE_PARAM_Q_REPORT_METADATA,
  IFACE_PARAM_Q_MAX
} iface_param_q_type_t;

/** iface_frame_ctrl_data_t;
 *  @session_id: session id
 *  @stream_id: stream id
 *  @iface_sink_port: iface sink port handle
 *  @ctrl_param: ctrl param
 *  @type: frame ctrl type
 *  @iface_metadata: iface metadata
 **/

typedef struct {
  uint32_t session_id;
  uint32_t stream_id;
  iface_port_t *iface_sink_port;
  mct_event_control_parm_t ctrl_param;
  iface_param_q_type_t         type;
  mct_bus_msg_iface_metadata_t iface_metadata;
} iface_frame_ctrl_data_t;

typedef struct {
  uint32_t session_id;
  uint32_t hw_stream_id;
  uint32_t user_stream_id;
} iface_reg_buf_t;


typedef struct{
  int32_t x;
  int32_t y;
  int32_t flag;   //0 - left, 1 - right
  int32_t b_x;    //block index
  int32_t b_y;
}iface_camif_pix_ext_t;

static int iface_discover_subdev_nodes(iface_t *iface);
int iface_start_session(iface_t *iface, uint32_t session_id);
int iface_reserve_sink_port(iface_t *iface, iface_port_t *reserving_sink_port,
  sensor_src_port_cap_t *sensor_port_cap, mct_stream_info_t *stream_info,
  unsigned int session_id, unsigned int stream_id);
int iface_ext_unlink_sink_port(iface_t *iface, iface_port_t *unlinking_sink_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id);
int iface_ext_unlink_src_port(iface_t *iface, iface_port_t *unlinking_src_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id);
int iface_reserve_src_port(iface_t *iface, iface_port_t *reserving_src_port,
  mct_stream_info_t *stream_info, unsigned int session_id,
  unsigned int stream_id);
int iface_unreserve_sink_port(iface_t *iface, iface_port_t *unreserving_sink_port,
  uint32_t session_id, uint32_t stream_id);
int iface_stop_session(iface_t *iface, uint32_t session_id);
int iface_set_session_data(iface_t *iface, uint32_t session_id,
  mct_pipeline_session_data_t *session_data);
int iface_handle_link_intra_session(iface_t *iface,
  iface_port_t *iface_sink_port, iface_session_t *iface_session,
  mct_event_t *event);
int iface_unreserve_src_port(iface_t *iface, iface_port_t *iface_src_port,
  unsigned int session_id, unsigned int stream_id);
int iface_ext_link_sink_port(iface_t *iface, iface_port_t *linking_sink_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id);
int iface_ext_link_src_port(iface_t *iface, iface_port_t *linking_src_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id);
int iface_sink_port_stream_config(iface_t *iface, iface_port_t *iface_sink_port,
  uint32_t stream_id, iface_session_t *session);
int iface_streamon(iface_t *iface, iface_port_t *iface_sink_port,
  uint32_t session_id, uint32_t user_stream_id, mct_event_t *event);
int iface_offline_stream_config(iface_t *iface, iface_port_t *iface_sink_port,
  uint32_t stream_id, iface_session_t *session, iface_offline_isp_info_t *offline_info);
int iface_sw_skip_config_to_thread(iface_t *iface,
  iface_sw_frameskip *skip_info);
int iface_offline_config_to_thread(iface_t *iface, iface_port_t *iface_sink_port,
  uint32_t user_stream_id, uint32_t session_id, iface_offline_isp_info_t  *offline_info);
int iface_streamon_to_thread(iface_t *iface, mct_port_t *mct_iface_port,
  iface_session_t *session, uint32_t user_stream_id, mct_event_t *event);
int iface_streamoff_to_thread(iface_t *iface, iface_port_t *iface_sink_port,
  iface_session_t *session, uint32_t user_stream_id, mct_event_t *event);
int iface_streamoff(iface_t *iface, iface_port_t *iface_sink_port,
  uint32_t session_id, uint32_t user_stream_id, mct_event_t *event);
int iface_set_hal_param(iface_t *iface, iface_port_t *iface_sink_port,
  iface_session_t *session, uint32_t stream_id, mct_event_control_parm_t *param,
  uint32_t frame_id);
int iface_set_super_param(iface_t *iface, iface_port_t *iface_sink_port,
  iface_session_t *session, uint32_t stream_id, mct_event_t *event);
int iface_handle_control_sof(iface_t *iface, iface_port_t *iface_sink_port,
  iface_session_t *session, uint32_t stream_id, mct_event_t *event);
int iface_handle_cds_request_to_thread(iface_t *iface,
  iface_session_t *session, uint32_t cds_enable);
int iface_set_hal_stream_param(iface_t *iface, iface_port_t *iface_sink_port,
  iface_session_t *session, uint32_t stream_id, mct_event_t *event);
void iface_destroy_hw(iface_t *iface, int hw_idx);
int32_t iface_set_sensor_output(iface_t *iface, mct_event_t *event);
int iface_create_axi(iface_t *iface, iface_session_t *session, int hw_idx);
int iface_init(iface_t *iface);
void iface_deinit(iface_t *iface);
void iface_util_release_resource(iface_t *iface,
  iface_session_t *session, iface_hw_stream_t *hw_stream);
static int iface_session_sem_thread_execute(void *ctrl,
  iface_session_thread_t *thread, iface_async_cmd_id_t async_cmd_id);
int iface_session_sem_thread_start(
  iface_session_thread_t *thread_data, void *hw_ptr);
int iface_session_sem_thread_stop(iface_session_thread_t *thread_data);
boolean iface_fast_aec_mode(iface_t *iface, uint32_t stream_id,
  iface_session_t *session, mct_fast_aec_mode_t *fast_aec);
int iface_halt_recovery_to_thread(iface_t *iface ,
  iface_halt_recovery_info_t *halt_recovery_info, uint32_t isp_id);
int iface_halt_recovery(iface_t *iface,
  uint32_t *session_id, uint32_t isp_id, uint32_t halt_frame_id);
int iface_sink_port_copy_stream_config(iface_t *iface,
  iface_port_t *iface_sink_port, uint32_t stream_id, iface_session_t *session,
  sensor_out_info_t *sensor_out_info);
int port_iface_forward_event_to_peer(iface_t *iface,
  mct_port_t *mct_port, mct_event_t *event);
boolean iface_store_per_frame_metadata(iface_session_t *session,
  mct_bus_msg_iface_metadata_t *iface_metadata, uint32_t q_idx);
int iface_update_sw_frame_skip(iface_t *iface,
  iface_sw_frameskip *sw_skip_info);

boolean iface_find_stream_by_sessionid(void *data1, void *data2);

int32_t iface_hvx_start_session(iface_hvx_singleton_t *hvx_singleton,
  iface_hvx_t *hvx);

int32_t iface_hvx_stop_session(iface_hvx_singleton_t *hvx_singleton,
  iface_hvx_t *hvx);

int32_t iface_hvx_handle_set_stream_config(iface_hvx_singleton_t *hvx_singleton,
  iface_hvx_t *hvx, sensor_out_info_t *sensor_out_info,
  iface_resource_t *session_resource, uint32_t *vfe_clk);

int32_t iface_hvx_streamon(iface_hvx_t *hvx);

int32_t iface_hvx_streamoff(iface_hvx_t *hvx);

int32_t iface_hvx_sof(iface_hvx_t *hvx, uint32_t frame_id);

int32_t iface_hvx_aec_update(iface_hvx_t *hvx, stats_update_t *stats_update);

int32_t iface_hvx_awb_update(iface_hvx_t *hvx, stats_update_t *stats_update);

int32_t iface_hvx_open(iface_hvx_singleton_t *hvx_singleton, iface_hvx_t *hvx,
  sensor_set_dim_t *set_dim);

int32_t iface_hvx_close(iface_hvx_singleton_t *hvx_singleton,
  iface_hvx_t *hvx);

int32_t iface_hvx_send_module_event_isp(iface_t *iface,
  iface_session_t *session, uint32_t stream_id);

int iface_resource_alloc_to_thread(iface_t *iface,
        uint32_t user_stream_id, uint32_t session_id,
        iface_buf_alloc_t *p_buf_alloc_info);

int iface_session_hw_sleep(
  iface_t         *iface,
  iface_port_t    *iface_sink_port,
  iface_session_t *iface_session,
  mct_event_t     *event);

int iface_session_hw_wake(
  iface_t         *iface,
  iface_port_t    *iface_sink_port,
  iface_session_t *iface_session,
  mct_event_t     *event);

int iface_streamoff_post_isp(
    iface_t         *iface,
    iface_port_t    *iface_sink_port,
    iface_session_t *session);
#endif /* __IFACE_H__ */
