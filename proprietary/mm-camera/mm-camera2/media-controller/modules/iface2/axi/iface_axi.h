/*============================================================================
Copyright (c) 2013-2014, 2016 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/

#ifndef __IFACE_AXI_H__
#define __IFACE_AXI_H__

#include <fcntl.h>
#include <poll.h>
#include <semaphore.h>

#include <media/msmb_camera.h>
#include "iface_def.h"
#include "iface_ops.h"
#include "iface_buf_mgr.h"

#define IFACE_AXI_STREAM_MAX      8
#define IFACE_AXI_MAX_THREAD_POIING_FDS 2
#define IFACE_STATS_MAX_BUFS         8

#define SRC_TO_INTF(src) \
  ((src < RDI_INTF_0 || src == VFE_AXI_SRC_MAX) ? VFE_PIX_0 : \
  (VFE_RAW_0 + src - RDI_INTF_0))

typedef enum {
  IFACE_AXI_SET_PARAM_INVALID,
  IFACE_AXI_SET_STREAM_CFG,
  IFACE_AXI_SET_STREAM_UNCFG,
  IFACE_AXI_SET_STATS_STREAM_CFG,
  IFACE_AXI_SET_STATS_STREAM_UNCFG,
  IFACE_AXI_SET_PARAM_FRAME_SKIP,  /* isp_param_frame_skip_pattern_t */
  IFACE_AXI_SET_PARAM_FRAME_SKIP_ALL_STREAM,
  IFACE_AXI_SET_PARAM_STATS_SKIP,
  IFACE_AXI_SET_STREAM_UPDATE,     /* isp_hwif_output_cfg_t */
  IFACE_AXI_SET_REQUEST_FRAME,
  IFACE_AXI_SET_OFFLINE_STREAM_UPDATE,
  IFACE_AXI_SET_MAX_NUM,
} iface_axi_set_param_id_t;

typedef enum {
  IFACE_AXI_ACTION_CODE_INVALID,
  IFACE_AXI_ACTION_CODE_STREAM_START,
  IFACE_AXI_ACTION_CODE_STREAM_START_ACK,
  IFACE_AXI_ACTION_CODE_STREAM_STOP,
  IFACE_AXI_ACTION_CODE_STREAM_STOP_ACK,
  IFACE_AXI_ACTION_CODE_STREAM_DIVERT_ACK,
  IFACE_AXI_ACTION_CODE_STATS_NOTIFY_ACK,
  IFACE_AXI_ACTION_CODE_HW_UPDATE,
  IFACE_AXI_ACTION_CODE_HALT_HW,
  IFACE_AXI_ACTION_CODE_RESET_HW,
  IFACE_AXI_ACTION_CODE_RESTART_HW,
  IFACE_AXI_ACTION_CODE_REG_BUF,
  IFACE_AXI_ACTION_CODE_UNREG_BUF,
  IFACE_AXI_ACTION_CODE_ADD_BUFQ,
  IFACE_AXI_ACTION_CODE_REMOVE_BUFQ,
  IFACE_AXI_ACTION_CODE_SET_MASTER_SLAVE,
  IFACE_AXI_ACTION_CODE_FETCH_START,
  IFACE_AXI_ACTION_CODE_UNMAP_BUF,
  IFACE_AXI_ACTION_CODE_REG_INPUTBUF,
  IFACE_AXI_ACTION_CODE_UNREG_INPUTBUF,
  IFACE_AXI_ACTION_CODE_MAX_NUM,
} iface_axi_action_code_t;

/* IFACE HW structures defined here */
typedef enum {
  IFACE_AXI_HW_STATE_INVALID,
  IFACE_AXI_HW_STATE_DEV_OPEN,
  IFACE_AXI_HW_STATE_IDLE,
  IFACE_AXI_HW_STATE_ACTIVE,
  IFACE_AXI_HW_STATE_STREAM_STOPPING,
  IFACE_HW_STATE_MAX
} iface_axi_hw_state_t;

typedef enum {
  IFACE_AXI_NOTIFY_ERR_HALT_AXI,
  IFACE_AXI_NOTIFY_CAMIF_SOF,
  IFACE_AXI_NOTIFY_STREAM_UPDATE_DONE,
  IFACE_AXI_NOTIFY_CAMIF_REG_UPDATE,
  IFACE_AXI_NOTIFY_BUF_DIVERT,
  IFACE_AXI_NOTIFY_STATS,
  IFACE_AXI_NOTIFY_HW_UPDATING,
  IFACE_AXI_NOTIFY_STATS_AWB_INFO,
  IFACE_AXI_NOTIFY_METADATA_INFO,
  IFACE_AXI_NOTIFY_META_VALID,
  IFACE_AXI_NOTIFY_BUS_OVERFLOW,
  IFACE_AXI_NOTIFY_PAGE_FAULT,
  IFACE_AXI_NOTIFY_PING_PONG_MISMATCH,
  IFACE_AXI_NOTIFY_REG_UPDATE_MISSING,
  IFACE_AXI_NOTIFY_OUTPUT_ERROR,
  IFACE_AXI_NOTIFY_FE_READ_DONE,
  IFACE_AXI_NOTIFY_UNMAP_BUF,
  IFACE_AXI_NOTIFY_BUF_FATAL_ERROR,
  IFACE_AXI_NOTIFY_MAX
} iface_axi_notify_params_id_t;

typedef enum {
  IFACE_AXI_RECOVERY_STATE_FORCE_HALT          = -2,
  IFACE_AXI_RECOVERY_STATE_RECOVERY_FAILED     = -1,
  IFACE_AXI_RECOVERY_STATE_NO_OVERFLOW         = 0,
  IFACE_AXI_RECOVERY_STATE_OVERFLOW_DETECTED   = 1,
  IFACE_AXI_RECOVERY_STATE_RECOVERY_DONE       = 2,
} iface_axi_recovery_state_t;

typedef enum {
  /* zero is not used enum */
  IFACE_THREAD_CMD_NOT_USED,
  IFACE_THREAD_CMD_NOTIFY_OPS_INIT, /* iface_pipe_notify_ops_init_t */
  IFACE_THREAD_CMD_SET_PARAMS,      /* iface_pipe_set_params_t */
  IFACE_THREAD_CMD_GET_PARAMS,      /* iface_pipe_get_params_t */
  IFACE_THREAD_CMD_ACTION,          /* iface_pipe_action_t */
  IFACE_THREAD_CMD_TIMER,
  IFACE_THREAD_CMD_DESTROY,         /* no payload, equal to ops's destroy */
 //MM_IFACE_CMD_HW_UPDATE,  // no need, hw update in isp      /* no payload */
  /* max count */
  IFACE_THREAD_CMD_MAX
} iface_thread_pipe_cmd_t;

typedef void (*iface_thread_timeout_func) (void *user_data, uint32_t timeout);

typedef struct {
  uint32_t isp_version; /* pass in init */
  int dev_idx;       /* device index, i.e. VFE0 or VFE1, pass in init */
  iface_isp_hw_cap_t cap;  /* hw capabilities */
  uint32_t session_id;
  iface_resource_t *session_resource;
  void *buf_mgr;
} iface_axi_init_params_t;

typedef struct {
  uint32_t params_id;
  void *in_params;
  uint32_t in_params_size;
} iface_pipe_set_params_t;

typedef struct {
  uint32_t params_id;
  void *in_params;
  uint32_t in_params_size;
  void *out_params;
  uint32_t out_params_size;
} iface_pipe_get_params_t;

typedef struct {
  uint32_t action_code;
  void *data;
  uint32_t data_size;
} iface_pipe_action_t;

typedef struct {
  void *init_params;
  iface_notify_ops_t *notify_ops;
} iface_axi_notify_ops_init_t;

typedef struct {
  cam_stream_ID_t framedrop;
  uint32_t frame_id;
  boolean controllable_output;
}iface_axi_framedrop;

/**
 * struct iface_axi_sof_event
 *  This structure is used by axi layer to report sof information to the
 *  iface layer.
 * @sof_buf_ms:           sof event message
 * @get_buf_fail_drop:    framedrop data due to get buf failed
 * @reg_update_fail_drop: framedrop data due to reg update failed
 */
typedef struct {
  mct_bus_msg_t         sof_buf_msg;
  iface_axi_framedrop   get_buf_fail_drop;
  iface_axi_framedrop   reg_update_fail_drop;
  iface_axi_framedrop   axi_updating_drop;
  iface_axi_framedrop   dual_cam_sync_drop;
}iface_axi_sof_event;

#if 0
typedef struct {
  uint32_t timer_num;
  struct timeval start_time;
  int32_t timeoutms;
  void *userdata;
  iface_thread_timeout_func timeout_func;
  sem_t sleep_sem;
}iface_tread_timer_t;
#endif
typedef struct {
  uint32_t timer_cnt;
  pthread_t pid;
  //iface_tread_timer_t *timer; /* only support one timer */
  uint32_t cmd_id; /* commend sending to the thread */
  int return_code; /* return code of the pipe command */
  int async_ret;
  pthread_mutex_t  cmd_mutex;
  iface_pipe_set_params_t *set_param_cmd;
  iface_pipe_get_params_t *get_param_cmd;
  iface_pipe_action_t     *action_cmd;
  iface_axi_notify_ops_init_t *init_cmd;
  sem_t sig_sem;
  int32_t pipe_fds[2];
  int poll_fd; /* non zero if is asked to poll subdev event */
  int poll_timeoutms;
  struct pollfd poll_fds[IFACE_AXI_MAX_THREAD_POIING_FDS];
  uint8_t num_fds;
  void *hw_ptr;
  pthread_mutex_t  busy_mutex;
  boolean thread_busy;
  boolean wake_up_at_sof;
  sem_t thread_wait_sem; /* thread waits on this semphore */
} iface_thread_t;

typedef enum {
  IFACE_AXI_STREAM_STATE_INVALID,
  IFACE_AXI_STREAM_STATE_CFG,
  IFACE_AXI_STREAM_STATE_STARTING,
  IFACE_AXI_STREAM_STATE_ACTIVE,
  IFACE_AXI_STREAM_STATE_STOPPING,
  IFACE_AXI_STREAM_STATE_MAX
} iface_axi_stream_state_t;

typedef struct {
  void *vaddr;
  int fd;
  struct v4l2_buffer buffer;
  struct v4l2_plane planes[VIDEO_MAX_PLANES];
} isp_axi_buf_t;


typedef struct {
  uint32_t session_id;
  uint32_t stream_id;
  int buf_idx;
  boolean is_buf_dirty;
  uint32_t buffer_access;
} iface_axi_buf_divert_ack_t;

typedef struct {
  iface_axi_stream_state_t state;
  iface_hw_stream_t hw_stream_info;
  uint32_t axi_stream_handle;
  uint32_t divert_event_id;
} iface_axi_stream_t;

typedef struct {
  iface_axi_stream_state_t state;
  iface_stats_stream_t stats_stream_info;
  uint32_t stream_handle;
  uint32_t bufq_handle;
} iface_axi_stats_stream_t;

typedef struct {
  union {
    struct msm_vfe_axi_stream_request_cmd stream_request_cmd;
    struct msm_vfe_axi_stream_release_cmd stream_release_cmd;
    struct msm_isp_buf_request buf_request_cmd; /* request bufs */
    struct msm_isp_qbuf_info qbuf_cmd;     /* qbuf to kernel */
    struct msm_isp_event_data buf_event;
    struct msm_vfe_axi_stream_cfg_cmd stream_start_stop_cmd; /* start/stop */
    struct msm_vfe_input_cfg vfe_input_cfg;
    struct msm_vfe_axi_halt_cmd halt_cmd;
    struct msm_vfe_axi_reset_cmd reset_cmd;
    struct msm_vfe_axi_restart_cmd restart_cmd;
  } u;
} iface_axi_hw_work_struct_t;

typedef struct {
  uint32_t session_id;
} iface_axi_intf_param_t;

typedef struct {
  iface_axi_intf_param_t intf_param[IFACE_INTF_MAX];
  iface_axi_stream_t stream[IFACE_MAX_STREAMS];
  iface_axi_stats_stream_t stats_stream[MSM_ISP_STATS_MAX];
  int8_t axi_handle_to_stream_idx[VFE_AXI_SRC_MAX];
  uint32_t isp_version;
  uint32_t num_active_streams;
  uint32_t num_pix_streams;
  uint32_t hw_update_skip;
  iface_axi_init_params_t init_params;
  iface_axi_hw_work_struct_t work_struct;
  struct msm_vfe_axi_stream_update_cmd update_cmd;
  boolean hw_update_pending;
  int open_cnt;
  int dev_idx;
  int fd;                  /* isp file descriptor */
  iface_axi_hw_state_t hw_state; /* ISP HW state */
  void *parent;            /* isp_data_t pointer */
  iface_ops_t hw_ops;
  iface_notify_ops_t *notify_ops;
  /* thread in axi hw*/
  iface_thread_t thread_poll;
  iface_thread_t thread_stream;
  iface_thread_t thread_hw;
  //boolean use_hw_thread_for_ack; // no need anymore?
  /* from core to hw is a sync call. the sync_mutex
   * guarantees the head of blocking calling */
  pthread_mutex_t sync_mutex;
  iface_buf_mgr_t *buf_mgr;
  isp_kstate_ptr_t kstate_ptr;
} iface_axi_hw_t;

typedef enum {
  ISP_AXI_NOTIFY_INVALID,
  ISP_AXI_NOTIFY_PIX_SOF,
  ISP_AXI_NOTIFY_WM_BUS_OVERFLOW,
  ISP_AXI_NOTIFY_MAX
} isp_axi_notify_param_t;

iface_ops_t *iface_axi_create_hw(char *dev_name);
int iface_axi_init(void *ctrl, void *in_params,
      iface_notify_ops_t *notify_ops);
int iface_axi_get_params(void *ctrl, uint32_t params_id, void *in_params,
  uint32_t in_params_size, void *out_params, uint32_t out_params_size);
int iface_axi_set_params(void *ctrl, uint32_t params_id, void *in_params,
  uint32_t in_params_size);
int iface_axi_destroy(void *ctrl);
int iface_axi_action(void *ctrl, uint32_t action_code, void *action_data,
  uint32_t action_data_size, int previous_ret_code);
int iface_axi_divert_ack(iface_axi_hw_t *axi_hw, iface_axi_buf_divert_ack_t *ack,
  uint32_t data_size);
void iface_axi_proc_subdev_event(iface_axi_hw_t *axi_hw, iface_thread_t *thread_data);
void iface_dump_axi_plane_config(iface_axi_stream_t *stream,
  struct msm_vfe_axi_stream_request_cmd *axi_request_cfg,
  cam_format_t cam_format);
int iface_thread_start(iface_thread_t *thread_data, void *hw_ptr, int poll_fd);
int iface_sem_thread_stop(iface_thread_t *thread_data);
int iface_sem_thread_start(iface_thread_t *thread_data, void *hw_ptr);
static int iface_axi_do_hw_update(iface_axi_hw_t *axi_hw);
int iface_axi_set_master_slave_info(iface_axi_hw_t *axi_hw,
  struct msm_isp_set_dual_hw_ms_cmd *ms_cmd);
#ifdef VIDIOC_MSM_ISP_DUAL_HW_MASTER_SLAVE_SYNC
int iface_axi_set_master_slave_sync(iface_axi_hw_t *axi_hw,
  struct msm_isp_dual_hw_master_slave_sync *sync);
#endif
#ifdef VIDIOC_MSM_ISP_DUAL_HW_LPM_MODE
int iface_axi_lpm_session(iface_axi_hw_t *axi_hw,
  struct msm_vfe_dual_lpm_mode *ab_ib_vote);
#endif
#endif /* __IFACE_AXI_H__ */
