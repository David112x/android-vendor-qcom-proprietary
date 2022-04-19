/* mct_controller.h
 *
 *Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
  * All Rights Reserved.
  * Confidential and Proprietary - Qualcomm Technologies, Inc.
  */

#ifndef __MCT_CONTROLLER_H__
#define __MCT_CONTROLLER_H__

#include "mct_object.h"
#include "mct_queue.h"
#include "mct_pipeline.h"

#include <pthread.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

typedef enum {
  MCT_THREAD_STATE_IDLE,
  MCT_THREAD_STATE_RUNNING,
  MCT_THREAD_STATE_MAX
}mct_thread_state_type;

struct _mct_controller {
 /* MCT's child is pipeline */
  mct_object_t  object;
  mct_queue_t   *serv_cmd_q;
  mct_queue_t   *bus_cmd_q;
  pthread_mutex_t  serv_msg_q_lock;
  pthread_mutex_t  server_lock;

  /* Server result Q */
  mct_queue_t   *serv_res_q;

  /* Result Q signal variable and its lock */
  pthread_cond_t serv_res_cond;
  pthread_condattr_t serv_res_condattr;
  pthread_mutex_t serv_res_cond_lock;

  pthread_cond_t mctl_thread_started_cond;
  pthread_mutex_t mctl_thread_started_mutex;

  /* 1. Server communicates with Media Controller through signal(servSignal);
   * 2. Media Controller communicate with Server through Pipe(servFd) */
  int         serv_fd;
  int         config_fd;

  pthread_t   mct_tid;             /* Thread ID of mct main thread */
  pthread_t   mct_bus_handle_tid;  /* Thread ID of mct bus_handler thread */

  pthread_cond_t mctl_cond;
  pthread_mutex_t mctl_mutex;
  pthread_cond_t mctl_bus_handle_cond;
  pthread_mutex_t mctl_bus_handle_mutex;

  mct_thread_state_type bus_thread_state;
  pthread_mutex_t mctl_state_mutex;
  pthread_cond_t mctl_state_cond;

  /* HAL message queue counter */
  unsigned int serv_cmd_q_counter;

  mct_pipeline_t *pipeline;
  /* HAL callback */
  void *event_cb;

  boolean timedout_status;
};

#define MCT_CAST(mct)         ((mct_controller_t *)(mct))
#define MCT_CHILDREN(mct)     (MCT_OBJECT_CHILDREN(mct))
#define MCT_NUM_CHILDREN(mct) (MCT_OBJECT_NUM_CHILDREN(mct))
#define MCT_PARENT(mct)       (MCT_OBJECT_PARENT(mct))
#define MCT_NUM_PARENTS(mct)  (MCT_OBJECT_NUM_PARENTS(mct))
#define MCT_LOCK(mct)         (MCT_OBJECT_LOCK(mct))
#define MCT_UNLOCK(mct)       (MCT_OBJECT_UNLOCK(mct))

typedef enum _mct_serv_msg_type {
  SERV_MSG_DS,
  SERV_MSG_HAL,
  SERV_MSG_MAX
} mct_serv_msg_type;

/** _mct_serv_ds_msg:
 *    @buf_type:
 *    @session: session index
 *    @stream:  stream index
 *    @size:    mapped buffer size
 *    @index:   mapped buffer index
 *    @fd:      buffer's file descriptor
 *              from domain socket
 *
 *  This structure defines the message received
 *  via domain socket
 **/
typedef struct _mct_serv_ds_msg {
  uint32_t buf_type;
  uint32_t session;
  uint32_t  stream;
  size_t size;
  uint32_t index;
  int32_t plane_idx;
  int fd;
  void *buffer;
} mct_serv_ds_msg_t;

typedef struct _mct_serv_ds_msg_list {
  uint32_t num_bufs;
  uint32_t operation;
  uint32_t session;
  mct_serv_ds_msg_t ds_buf[CAM_MAX_NUM_BUFS_PER_STREAM];
} mct_serv_ds_msg_bundle_t;

/** _mct_server_msg:
 *    @msg_type: defines whether it is HAL message or Domain
 *               Socket message
 *    @ds_msg:   content of message received from Domain Socket
 *    @hal_msg:  content of message received from HAL, it's
 *               definition is in msmb_camera.h
 *
 * Message sent to Media Controller from Imaging Server
 **/
typedef struct _mct_serv_msg {
  mct_serv_msg_type msg_type;

  union {
    mct_serv_ds_msg_bundle_t ds_msg;
    struct v4l2_event hal_msg;
  } u;
} mct_serv_msg_t;

/** _mct_process_ret_type:
 *
 *
 *
 **/
typedef enum _mct_process_ret_type {
  MCT_PROCESS_RET_SERVER_MSG,
  MCT_PROCESS_NOTIFY_SERVER_MSG,
  MCT_PROCESS_RET_BUS_MSG,
  MCT_PROCESS_DUMP_INFO,
  MCT_PROCESS_RET_ERROR_MSG,
} mct_process_ret_type;

typedef struct _mct_proc_serv_msg_ret {
  boolean error;
  mct_serv_msg_t msg;
} mct_proc_serv_msg_ret;

/** mct_proc_bus_msg_type
 *
 *
 *
 **/
typedef enum _mct_proc_bus_msg_type {
  MCT_PROC_BUS_METADATA,
  MCT_PROC_BUS_ISP_SOF,
  MCT_PROC_BUS_ISP_ERROR,
} mct_proc_bus_msg_type;

/** mct_proc_bus_msg_ret:
 *   @msg_type: Bus message type
 *   @metadata_buf_idx: meta data buffer
 *      index
 *   @session: session index
 *   @stream:  stream index
 *
 * Bus message return value sent to imaging server
 **/
typedef struct _mct_proc_bus_msg_ret {
  boolean error;
  mct_bus_msg_type_t msg_type;
  int metadata_buf_idx;
  int sof_debug;
  unsigned int session;
  unsigned int stream;
} mct_proc_bus_msg_ret;

typedef enum {
  MCT_SUPERPARAM_SINGLE_CAM,
  MCT_SUPERPARAM_DC_W_PARAM,
  MCT_SUPERPARAM_DC_WO_PARAM,
} mct_superparam_type_t;

/** _mct_process_ret: Media Controller process return type
 *    @type: server messagre or bus message
 *      - server HAL message
 *        use SERV_RET_TO_HAL_CMDACK for control command
 *        use SERV_RET_TO_HAL_NOTIFY for DS Buf  mapping
 *      - bus message use SERV_RET_TO_HAL_NOTIFY
 *
 *    @serv_msg_ret: return value after processsed server message
 *    @bus_msg_ret: return value after processed bus message
 **/
typedef struct _mct_process_ret {
  mct_process_ret_type type;

  union {
    mct_proc_serv_msg_ret serv_msg_ret;
    mct_proc_bus_msg_ret  bus_msg_ret;
  } u;

} mct_process_ret_t;

cam_status_t mct_controller_new(mct_list_t *mods,
  unsigned int session_idx, int servFd, void *event_cb);

boolean mct_controller_destroy(unsigned int session_idx);

boolean mct_controller_proc_serv_msg(mct_serv_msg_t *msg);

boolean mct_controller_get_thread_state (mct_controller_t *mct,
  mct_thread_state_type *state);

boolean mct_controller_set_thread_state (mct_controller_t *mct,
  mct_thread_state_type state);

boolean mct_controller_send_cb(mct_pipeline_t *pipeline,
  mct_bus_msg_type_t type);


#endif /* __MCT_CONTROLLER_H__ */
