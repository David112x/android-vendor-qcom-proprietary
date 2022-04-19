/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef VPU_CLIENT_PRIVATE_H
#define VPU_CLIENT_PRIVATE_H

#include <pthread.h>
#include <linux/videodev2.h>
#include "vpu_client.h"
#include "vpu_client_tuning.h"

#define VPU_CLIENT_NUM_BUF_MAX  16

#define READ_FD   0
#define WRITE_FD  1

typedef enum _vpu_client_pipe_msg_type_t {
  PIPE_MSG_EXIT = 100,
  PIPE_MSG_MAX
} vpu_client_pipe_msg_type_t;

typedef struct _vpu_client_pipe_msg_t {
  vpu_client_pipe_msg_type_t type;
  void* data;
} vpu_client_pipe_msg_t;

typedef struct _vpu_thread_data_t {
  int32_t exit_signal;
  pthread_mutex_t mutex;
} vpu_thread_data_t;

typedef struct _vpu_client_ctrl_parm_t {
  uint32_t tnr_enable;
  uint32_t tnr_auto_mode;
  uint32_t tnr_value;
} vpu_client_ctrl_parm_t;

struct _vpu_client_t {
  int                       fd;
  int32_t                   dev_id;
  pthread_t                 vpu_poll_thread;
  vpu_thread_data_t         th_data;
  pthread_mutex_t           mutex;
  pthread_cond_t            cond_no_pending_buf;
  int32_t                   sessions_supported;
  int32_t                   session_attached;
  int32_t                   is_streaming;
  int32_t                   is_session_open;
  struct v4l2_format        format;
  struct                    v4l2_buffer buffers[VPU_CLIENT_NUM_BUF_MAX];
  int32_t                   num_buffers;
  vpu_client_cb_t           event_cb;
  int32_t                   input_bufs_pending;
  int32_t                   output_bufs_pending;
  int32_t                   is_polling;
  int32_t                   pfd[2];
  void                      *userdata;
  void                      *chromatix_ptr;
  vpu_tuning_ctrl_t         t_ctrl;
  vpu_client_ctrl_parm_t    cur_ctrl_parm;
};

int32_t vpu_client_set_ctrl_parm(vpu_client_t client,
    vpu_client_ctrl_parm_t ctrl_parm);

#endif
