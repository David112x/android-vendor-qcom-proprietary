/*Copyright (c) 2012-2016, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* server_process.c
 *
 * This file contains the server_process helper function implementation.
 * After receiving a command from HAL, the server sends over the command here
 * for further processing.
 **/

#include <sys/un.h>

#include "server_process.h"
#include "mct_controller.h"
#include "mct_module.h"
#include "cam_intf.h"
#include "camera_dbg.h"
#include "memleak.h"
#include "mct_debug_fdleak.h"
#include <sys/sysinfo.h>

static mct_module_init_name_t modules_list[] = {
  {"sensor", module_sensor_init,   module_sensor_deinit, NULL},
  {"iface",  module_iface_init,   module_iface_deinit, NULL},
  {"isp",    module_isp_init,      module_isp_deinit, NULL},
  {"stats",  stats_module_init,    stats_module_deinit, NULL},
  {"pproc",  pproc_module_init,    pproc_module_deinit, NULL},
  {"imglib", module_imglib_init, module_imglib_deinit, NULL},
};

static mct_list_t *modules = NULL;
static mct_list_t *modules_all = NULL;

/** server_process_module_deinit:
 *    @data: MctModule_t from link list
 *
 * This function should be triggered ONLY when
 * server_process_module_init() fails. We shouldn't
 * hit here for all other scenarios.
 **/
static boolean server_process_module_deinit()
{
  int i;
  mct_module_t *temp = NULL;

  CLOGD(CAM_MCT_MODULE, "CAMERA_DAEMON: deinit mods");
  for (i = 1;
       i < (int)(sizeof(modules_list)/sizeof(mct_module_init_name_t)); i++) {
    if( NULL == modules_list[i].deinit_mod)
      continue;
    CLOGD(CAM_MCT_MODULE, "CAMERA_DAEMON: module name : %s: E",
      modules_list[i].name);
    modules_list[i].deinit_mod(modules_list[i].module);
    CLOGD(CAM_MCT_MODULE, "CAMERA_DAEMON: module name : %s: X",
      modules_list[i].name);
  } /* for */

  mct_list_free_all(modules_all, NULL);
  modules_all = NULL;

  CLOGD(CAM_MCT_MODULE, "CAMERA_DAEMON: deinit mods done");

  mct_list_free_all(modules, NULL);
  modules = NULL;

  temp = modules_list[0].module;

  if (temp) {
    if ((modules = mct_list_append(modules, temp, NULL, NULL)) == NULL) {
      if (modules) {
        modules_list[0].deinit_mod(temp);
        modules_list[0].module = NULL;
        return FALSE;
      }
    }
  }
  return TRUE;
}

/** server_process_module_init:
 *
 *  Very first thing Imaging Server performs after it starts
 *  to build up module list. One specific module initilization
 *  may not success because the module may not exist on this
 *  platform.
 **/
boolean server_process_module_init(void)
{
  mct_module_t *temp = NULL;
  int          i;
  pthread_t thread;
  pthread_attr_t attr;
  char prop[PROPERTY_VALUE_MAX];
  int enable_memleak = 0;

  CLOGI(CAM_MCT_MODULE, "Begin init mods");
  for (i = 1;
       i < (int)(sizeof(modules_list)/sizeof(mct_module_init_name_t)); i++) {
    if( NULL == modules_list[i].init_mod)
      continue;
    CLOGD(CAM_MCT_MODULE, "CAMERA_DAEMON: module name : %s: E",
      modules_list[i].name);
    temp = modules_list[i].init_mod(modules_list[i].name);
    CLOGD(CAM_MCT_MODULE, "CAMERA_DAEMON: module name : %s: X",
      modules_list[i].name);
    if (temp) {
      modules_list[i].module = temp;
      if ((modules = mct_list_append(modules, temp, NULL, NULL)) == NULL) {
        if (modules) {
          for (i--; i >= 0; i--) {
            modules_list[i].deinit_mod(temp);
            modules_list[i].module = NULL;
          }

          mct_list_free_all(modules, NULL);
          return FALSE;
        }
       modules_all = mct_list_append(modules_all, temp, NULL, NULL);
      }
    }
  } /* for */

  #ifdef MEMLEAK_FLAG
  property_get("persist.vendor.camera.memleak.enable", prop, "0");
  enable_memleak = atoi(prop);
  if(enable_memleak) {
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread, &attr, server_memleak_thread, NULL);
    pthread_setname_np(thread, "CAM_memleak");
  }
  #endif
  CLOGI(CAM_MCT_MODULE, "CAMERA_DAEMON: init mods done");

  return TRUE;
}

boolean server_process_module_sensor_init(void)
{
  mct_module_t *temp = NULL;

  CLOGD(CAM_MCT_MODULE, "CAMERA_DAEMON: Begin sensor init mods");
    if( NULL == modules_list[0].init_mod)
      return FALSE;

    temp = modules_list[0].init_mod(modules_list[0].name);
    if (temp) {
      modules_list[0].module = temp;
      if ((modules = mct_list_append(modules, temp, NULL, NULL)) == NULL) {
        if (modules) {
            modules_list[0].deinit_mod(temp);
            modules_list[0].module = NULL;
            return FALSE;
        }
      }
    }
  CLOGD(CAM_MCT_MODULE, "CAMERA_DAEMON: sensor init mods done");

  return TRUE;
}
/** server_process_bind_hal_dsocket
 *    @session: new session index
 *    @ds_fd:   domain socket file descriptor with HAL
 *
 *  Return - TRUE if it everything is fine
 **/
static boolean server_process_bind_hal_ds(int session,
  int *ds_fd)
{
  struct sockaddr_un addr;

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;

  snprintf(addr.sun_path, UNIX_PATH_MAX,
           QCAMERA_DUMP_FRM_LOCATION"cam_socket%d", session);

  /* remove the socket path if it already exists, otherwise bind might fail */
  unlink(addr.sun_path);

  *ds_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if (*ds_fd == -1)
    return FALSE;

  if (bind(*ds_fd, (struct sockaddr *)&addr,
      sizeof(struct sockaddr_un)) == -1) {
    CLOGE(CAM_MCT_MODULE, "Error in bind socket_fd=%d %s ",
      *ds_fd, strerror(errno));
    close(*ds_fd);
    return FALSE;
  }
  return TRUE;
}

/** server_process_hal_event:
 *    @event: v4l2_event from kernel
 *
 * Process any command recevied from HAL through kernel.
 *
 * Return: process result, action server should take.
 **/
serv_proc_ret_t server_process_hal_event(struct v4l2_event *event)
{
  serv_proc_ret_t ret;
  mct_serv_msg_t  serv_msg;
  struct msm_v4l2_event_data *data =
    (struct msm_v4l2_event_data *)(event->u.data);
  struct msm_v4l2_event_data *ret_data =
    (struct msm_v4l2_event_data *)(ret.ret_to_hal.ret_event.u.data);
  cam_status_t mct_ret_type;
  int32_t enabled_savemem = 0;
  char savemem[92];
  char prop[PROPERTY_VALUE_MAX];
  int enable_fdleak = 0;

  memset(&ret, 0, sizeof(serv_proc_ret_t));
  /* by default don't return command ACK to HAL,
   * return ACK only for two cases:
   * 1. NEW SESSION
   * 2. DEL SESSION
   * 3. Failure
   *
   * other commands will return after they are processed
   * in MCT */
  ret.ret_to_hal.ret       = FALSE;
  ret.ret_to_hal.ret_type  = SERV_RET_TO_HAL_CMDACK;
  ret.ret_to_hal.ret_event = *event;
  ret_data->v4l2_event_type   = event->type;
  ret_data->v4l2_event_id     = event->id;
  ret.result               = RESULT_SUCCESS;

  property_get("vendor.camera.cameradaemon.SaveMemAtBoot", savemem, "0");
  enabled_savemem = atoi(savemem);

  switch (event->id) {
  case MSM_CAMERA_NEW_SESSION: {
    ret.ret_to_hal.ret = TRUE;
    ret.result = RESULT_NEW_SESSION;

#ifdef MEMLEAK_FLAG
    enable_memleak_trace(0);
#endif

#if defined(LOG_DEBUG)
    cam_debug_open();
#endif

#ifdef FDLEAK_FLAG
  property_get("persist.vendor.camera.fdleak.enable", prop, "0");
  enable_fdleak = atoi(prop);
  if (enable_fdleak) {
    CLOGI(CAM_MCT_MODULE,"fdleak tool is enabled for camera daemon");
    mct_degug_enable_fdleak_trace();
  }
#endif
    if(enabled_savemem == 1) {
      if (server_process_module_init() == FALSE)
        goto error_return;
    }

    /* new session starts, need to create a MCT:
     * open a pipe first.
     *
     * Note the 3 file descriptors:
     * one domain socket fd and two pipe fds are closed
     * at server side once session close information
     * is recevied by server.
     * */
    int pipe_fd[2];

    if (!pipe(pipe_fd)) {
      ret.new_session_info.mct_msg_rd_fd = pipe_fd[0];
      ret.new_session_info.mct_msg_wt_fd = pipe_fd[1];
    } else {
      goto error_return;
    }

    if (server_process_bind_hal_ds(data->session_id,
          &(ret.new_session_info.hal_ds_fd)) == FALSE) {
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      goto error_return;
    }

    mct_ret_type = mct_controller_new (modules, data->session_id, pipe_fd[1], 0);
    if (CAM_STATUS_SUCCESS == mct_ret_type) {
      ret.new_session    = TRUE;
      ret.new_session_info.session_idx = data->session_id;
      goto process_done;
    } else if (CAM_STATUS_BUSY== mct_ret_type) {
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      close(ret.new_session_info.hal_ds_fd);
      goto return_busy;
    } else {
      close(pipe_fd[0]);
      close(pipe_fd[1]);
      close(ret.new_session_info.hal_ds_fd);
      ret.ret_to_hal.ret_type       = SERV_RET_TO_HAL_NOTIFY_ERROR;
      ret_data->session_id          = data->session_id;
      goto error_return;
    }

  } /* case MSM_CAMERA_NEW_SESSION */
    break;

  case MSM_CAMERA_DEL_SESSION: {
    ret.result = RESULT_DEL_SESSION;
    ret.ret_to_hal.ret = TRUE;

#if defined(LOG_DEBUG)
    cam_debug_flush();
#endif

    if (mct_controller_destroy(data->session_id) == FALSE) {
      if (enabled_savemem == 1)
        server_process_module_deinit();
      goto error_return;
    } else {
      if (enabled_savemem == 1)
        server_process_module_deinit();
#ifdef MEMLEAK_FLAG
      if (pthread_mutex_trylock (&server_memleak_mut) == 0) {
        server_memleak_event = PRINT_LEAK_MEMORY;
        pthread_cond_signal(&server_memleak_con);
        pthread_mutex_unlock (&server_memleak_mut);
      }
#endif
      goto process_done;
    }
  } /* case MSM_CAMERA_DEL_SESSION */
    break;

  default:
    serv_msg.msg_type  = SERV_MSG_HAL;
    serv_msg.u.hal_msg = *event;
    break;
  } /* switch (event->type) */

  if (mct_controller_proc_serv_msg(&serv_msg) == FALSE) {
    ret.result = RESULT_FAILURE;
    goto error_return;
  }

process_done:
  ret_data->status = MSM_CAMERA_CMD_SUCCESS;
  return ret;

return_busy:
  ret_data->status = MSM_CAMERA_ERR_DEVICE_BUSY;
  return ret;

error_return:
  ret_data->status = MSM_CAMERA_ERR_CMD_FAIL;
  return ret;
}

/** server_process_read_dsocket_packet:
 *    @fd:
 *    @packet:
 **/
static boolean server_process_read_ds_packet(const int fd,
  cam_sock_packet_t *packet)
{
  struct msghdr  msgh;
  struct iovec   iov[1];
  struct cmsghdr *cmsghp = NULL;
  int fd_recv[CAM_MAX_NUM_BUFS_PER_STREAM];
  char   control[CMSG_SPACE(sizeof(fd_recv))];
  int num_fds = 0;
  int *fds_ptr = NULL;
  int i = 0;

  memset(&msgh, 0, sizeof(msgh));
  msgh.msg_name       = NULL;
  msgh.msg_namelen    = 0;
  msgh.msg_control    = control;
  msgh.msg_controllen = sizeof(control);

  memset(packet, 0, sizeof(cam_sock_packet_t));
  iov[0].iov_base = packet;
  iov[0].iov_len  = sizeof(cam_sock_packet_t);
  msgh.msg_iov    = iov;
  msgh.msg_iovlen = 1;

  if ((recvmsg(fd, &(msgh), 0)) <= 0)
    return FALSE;

  if ((packet->msg_type == CAM_MAPPING_TYPE_FD_BUNDLED_UNMAPPING)
     || (packet->msg_type == CAM_MAPPING_TYPE_FD_UNMAPPING)) {
    return TRUE;
  }

  if ((packet->msg_type == CAM_MAPPING_TYPE_FD_MAPPING)
     || (packet->msg_type == CAM_MAPPING_TYPE_FD_UNMAPPING)) {
    num_fds = 1;
  }
  else if ((packet->msg_type == CAM_MAPPING_TYPE_FD_BUNDLED_MAPPING)
    || (packet->msg_type == CAM_MAPPING_TYPE_FD_BUNDLED_UNMAPPING)) {
    num_fds = packet->payload.buf_map_list.length;
  }
  if (((cmsghp = CMSG_FIRSTHDR(&msgh)) != NULL) &&
      (cmsghp->cmsg_len == CMSG_LEN(sizeof(int)* num_fds))) {

    if (cmsghp->cmsg_level == SOL_SOCKET &&
        cmsghp->cmsg_type  == SCM_RIGHTS) {
      fds_ptr = ((int *)CMSG_DATA(cmsghp));
      memcpy(fd_recv, fds_ptr, sizeof(int) * num_fds);
    } else {
      return FALSE;
    }
  } else {
    return FALSE;
  }

  if (packet->msg_type == CAM_MAPPING_TYPE_FD_MAPPING) {
    packet->payload.buf_map.fd = fd_recv[0];
  }
  else if (packet->msg_type == CAM_MAPPING_TYPE_FD_BUNDLED_MAPPING) {
    for (i = 0; i < num_fds; i++) {
      packet->payload.buf_map_list.buf_maps[i].fd = fd_recv[i];
    }
  }
  return TRUE;
}

/** server_process_hal_ds_packet:
 *    @fd
 *    @session
 *
 *  Return: serv_proc_ret_t
 *          FAILURE - will return to HAL immediately
 **/
serv_proc_ret_t server_process_hal_ds_packet(const int fd,
  const int session)
{
  cam_sock_packet_t  packet;
  mct_serv_msg_t     serv_msg;
  serv_proc_ret_t    ret;
  uint32_t i;
  struct msm_v4l2_event_data *ret_data = (struct msm_v4l2_event_data *)
    &(ret.ret_to_hal.ret_event.u.data[0]);

  memset(&ret, 0, sizeof(serv_proc_ret_t));
  ret.ret_to_hal.ret         = TRUE;
  ret.ret_to_hal.ret_type    = SERV_RET_TO_HAL_NOTIFY;
  ret.ret_to_hal.ret_event.type  = MSM_CAMERA_V4L2_EVENT_TYPE;
  ret_data->v4l2_event_type   = MSM_CAMERA_V4L2_EVENT_TYPE;
  ret.result = RESULT_SUCCESS;

  if (server_process_read_ds_packet(fd, &packet) == FALSE) {
    ret.result        = RESULT_FAILURE;
    goto error_return;
  }
  serv_msg.msg_type = SERV_MSG_DS;
  serv_msg.u.ds_msg.session  = session;
  serv_msg.u.ds_msg.operation = (unsigned int)packet.msg_type;

  switch (packet.msg_type) {

    case CAM_MAPPING_TYPE_FD_BUNDLED_MAPPING: {
      serv_msg.u.ds_msg.num_bufs = packet.payload.buf_map_list.length;
      for (i= 0; i < serv_msg.u.ds_msg.num_bufs; i++) {
        serv_msg.u.ds_msg.ds_buf[i].buf_type  =
          (unsigned int)packet.payload.buf_map_list.buf_maps[i].type;
        serv_msg.u.ds_msg.ds_buf[i].fd =
          packet.payload.buf_map_list.buf_maps[i].fd;
        serv_msg.u.ds_msg.ds_buf[i].stream =
         packet.payload.buf_map_list.buf_maps[i].stream_id;
        serv_msg.u.ds_msg.ds_buf[i].size =
          packet.payload.buf_map_list.buf_maps[i].size;
        serv_msg.u.ds_msg.ds_buf[i].index =
          packet.payload.buf_map_list.buf_maps[i].frame_idx;
        serv_msg.u.ds_msg.ds_buf[i].plane_idx =
          packet.payload.buf_map_list.buf_maps[i].plane_idx;
      }
    }
     break;

    case CAM_MAPPING_TYPE_FD_BUNDLED_UNMAPPING: {
      serv_msg.u.ds_msg.num_bufs = packet.payload.buf_map_list.length;
      for (i= 0; i <= serv_msg.u.ds_msg.num_bufs; i++) {
        serv_msg.u.ds_msg.ds_buf[i].buf_type =
          (int)packet.payload.buf_unmap_list.buf_unmaps[i].type;
        serv_msg.u.ds_msg.ds_buf[i].stream =
          packet.payload.buf_unmap_list.buf_unmaps[i].stream_id;
        serv_msg.u.ds_msg.ds_buf[i].index  =
          packet.payload.buf_unmap_list.buf_unmaps[i].frame_idx;
      }
    }
      break;

    case CAM_MAPPING_TYPE_FD_MAPPING: {
      serv_msg.u.ds_msg.num_bufs = 1;
      serv_msg.u.ds_msg.ds_buf[0].buf_type =
        (unsigned int)packet.payload.buf_map.type;
      serv_msg.u.ds_msg.ds_buf[0].fd        = packet.payload.buf_map.fd;
      serv_msg.u.ds_msg.ds_buf[0].stream    = packet.payload.buf_map.stream_id;
      serv_msg.u.ds_msg.ds_buf[0].size      = packet.payload.buf_map.size;
      serv_msg.u.ds_msg.ds_buf[0].index     = packet.payload.buf_map.frame_idx;
      serv_msg.u.ds_msg.ds_buf[0].plane_idx = packet.payload.buf_map.plane_idx;
    }
      break;

    case CAM_MAPPING_TYPE_FD_UNMAPPING: {
      serv_msg.u.ds_msg.num_bufs = 1;
      serv_msg.u.ds_msg.ds_buf[0].buf_type =
        (int)packet.payload.buf_unmap.type;
      serv_msg.u.ds_msg.ds_buf[0].stream =
        packet.payload.buf_unmap.stream_id;
      serv_msg.u.ds_msg.ds_buf[0].index  =
        packet.payload.buf_unmap.frame_idx;
    }
      break;

    default:
      break;
  }

  if (mct_controller_proc_serv_msg(&serv_msg) == FALSE) {
    ret.result = RESULT_FAILURE;
    goto error_return;
  }

  ret_data->status = MSM_CAMERA_BUF_MAP_SUCCESS;
  return ret;

error_return:
  if (ret.result == RESULT_FAILURE) {
    ret.ret_to_hal.ret         = TRUE;
    ret_data->status        = MSM_CAMERA_ERR_MAPPING;
    ret_data->session_id    = session;

    if (packet.msg_type == CAM_MAPPING_TYPE_FD_MAPPING) {
      CLOGE(CAM_MCT_MODULE, "Buffer map error");
      ret_data->stream_id = packet.payload.buf_map.stream_id;
      /*ret_data->command   = packet.payload.buf_map.type;*/
    } else if (packet.msg_type == CAM_MAPPING_TYPE_FD_UNMAPPING) {
      CLOGE(CAM_MCT_MODULE, "Buffer unmap error");
      ret_data->stream_id = packet.payload.buf_unmap.stream_id;
      /*ret_data->command = packet.payload.buf_unmap.type;*/
    }
  }
  return ret;
}

/** server_process_mct_msg:
 *    @fd: pipe fd media controller uses to send message to HAL
 *    @session: session index
 *
 *  Return: serv_proc_ret_t
 *          FAILURE - will return to HAL immediately
 **/
serv_proc_ret_t server_process_mct_msg(const int fd, const unsigned int session)
{
  int read_len;
  mct_process_ret_t mct_ret;
  serv_proc_ret_t ret;
  struct msm_v4l2_event_data *ret_data = (struct msm_v4l2_event_data *)
    ret.ret_to_hal.ret_event.u.data;

  memset(&ret, 0, sizeof(serv_proc_ret_t));
  read_len = read(fd, &mct_ret, sizeof(mct_process_ret_t));
  if (read_len <= 0) {
    CLOGE(CAM_MCT_MODULE, "ERROR:read len %d is less than expected", read_len);
    goto error;
  }

  ret.result           = RESULT_SUCCESS;
  ret.ret_to_hal.ret   = TRUE;

  switch (mct_ret.type) {
  case MCT_PROCESS_RET_SERVER_MSG: {

    if (mct_ret.u.serv_msg_ret.msg.msg_type == SERV_MSG_HAL) {
      /* We just processed a HAL command, need to ACK it */
      struct v4l2_event *msg = &(mct_ret.u.serv_msg_ret.msg.u.hal_msg);
      struct msm_v4l2_event_data *data =
        (struct msm_v4l2_event_data *)(msg->u.data);
      if (data->session_id != session) {
        CLOGE(CAM_MCT_MODULE, "ERROR: MSG:HAL: session ID %d doesn't match %d",
          data->session_id, session);
        goto error;
      }

      ret.ret_to_hal.ret_type  = SERV_RET_TO_HAL_CMDACK;
      ret.ret_to_hal.ret_event = *msg;
      ret_data->v4l2_event_type   = msg->type;
      ret_data->v4l2_event_id     = msg->id;

      ret_data->status         = (mct_ret.u.serv_msg_ret.error == TRUE) ?
        MSM_CAMERA_CMD_SUCCESS : MSM_CAMERA_ERR_CMD_FAIL;
    } else if (mct_ret.u.serv_msg_ret.msg.msg_type == SERV_MSG_DS) {
      /* Note we just processed a Domain Socket mapping,
       * need to send an event to HAL */
       mct_serv_ds_msg_bundle_t *msg = &(mct_ret.u.serv_msg_ret.msg.u.ds_msg);

      if (msg->session!= session) {
        CLOGE(CAM_MCT_MODULE, "ERROR: MSG:DS: session ID: %d doesn't match %d",
          msg->session, session);
        goto error;
      }

      ret.ret_to_hal.ret_type       = SERV_RET_TO_HAL_NOTIFY;
      ret.ret_to_hal.ret_event.type = MSM_CAMERA_V4L2_EVENT_TYPE;
      ret.ret_to_hal.ret_event.id = MSM_CAMERA_MSM_NOTIFY;
      ret_data->v4l2_event_type   = MSM_CAMERA_V4L2_EVENT_TYPE;
      ret_data->v4l2_event_id     = MSM_CAMERA_MSM_NOTIFY;
      ret_data->command           = CAM_EVENT_TYPE_MAP_UNMAP_DONE;
      ret_data->session_id        = msg->session;
      ret_data->stream_id         = msg->ds_buf[0].stream;
      ret_data->status            = (mct_ret.u.serv_msg_ret.error == TRUE) ?
        MSM_CAMERA_STATUS_SUCCESS : MSM_CAMERA_STATUS_FAIL;
    } else {
      CLOGE(CAM_MCT_MODULE, "ERROR: unexpected message type: %d",
        mct_ret.u.serv_msg_ret.msg.msg_type);
      goto error;
    }
  }
    break;

  case MCT_PROCESS_RET_BUS_MSG: {

    ret.ret_to_hal.ret_type       = SERV_RET_TO_HAL_NOTIFY;
    ret.ret_to_hal.ret_event.type = MSM_CAMERA_V4L2_EVENT_TYPE;
    ret.ret_to_hal.ret_event.id   = MSM_CAMERA_MSM_NOTIFY;

    ret_data->v4l2_event_type   = MSM_CAMERA_V4L2_EVENT_TYPE;
    ret_data->v4l2_event_id     = MSM_CAMERA_MSM_NOTIFY;

    if (mct_ret.u.bus_msg_ret.msg_type == MCT_BUS_MSG_EZTUNE_JPEG) {
      ret_data->command = CAM_EVENT_TYPE_INT_TAKE_JPEG;
    } else if (mct_ret.u.bus_msg_ret.msg_type == MCT_BUS_MSG_EZTUNE_RAW) {
      ret_data->command = CAM_EVENT_TYPE_INT_TAKE_RAW;
    } else if (mct_ret.u.bus_msg_ret.msg_type == MCT_BUS_MSG_CAC_STAGE_DONE) {
      ret_data->command = CAM_EVENT_TYPE_CAC_DONE;
    }

    ret_data->session_id          = mct_ret.u.bus_msg_ret.session;
    ret_data->status              = (mct_ret.u.bus_msg_ret.error == TRUE) ?
      MSM_CAMERA_STATUS_SUCCESS : MSM_CAMERA_STATUS_FAIL;
  }
    break;

  case MCT_PROCESS_NOTIFY_SERVER_MSG: {

    struct v4l2_event *msg = &(mct_ret.u.serv_msg_ret.msg.u.hal_msg);
    struct msm_v4l2_event_data *data =
      (struct msm_v4l2_event_data *)(msg->u.data);
    if (data) {
      ret.ret_to_hal.ret_type       = SERV_RET_TO_HAL_NOTIFY;
      ret.ret_to_hal.ret_event.type = MSM_CAMERA_V4L2_EVENT_TYPE;
      ret.ret_to_hal.ret_event.id   = MSM_CAMERA_MSM_NOTIFY;

      ret_data->v4l2_event_type   = MSM_CAMERA_V4L2_EVENT_TYPE;
      ret_data->v4l2_event_id     = MSM_CAMERA_MSM_NOTIFY;
      ret_data->command           = data->command;
      ret_data->session_id        = data->session_id;
      ret_data->status            = (mct_ret.u.serv_msg_ret.error == TRUE) ?
        MSM_CAMERA_CMD_SUCCESS : MSM_CAMERA_ERR_CMD_FAIL;
    } else {
      CLOGE(CAM_MCT_MODULE, "Invalid event data. Not notifying HAL");
    }
  }
    break;

  case MCT_PROCESS_DUMP_INFO: {
    ret.ret_to_hal.ret_type       = SERV_RET_TO_KERNEL_NOTIFY;
    ret_data->session_id          = mct_ret.u.bus_msg_ret.session;
    ret_data->status              = mct_ret.u.bus_msg_ret.sof_debug;
  }
    break;

  case MCT_PROCESS_RET_ERROR_MSG: {
    ret.ret_to_hal.ret_type       = SERV_RET_TO_HAL_NOTIFY_ERROR;
    ret_data->session_id          = mct_ret.u.bus_msg_ret.session;
  }
    break;
  default:
    break;
  }
  return ret;

error:
  ret.result         = RESULT_FAILURE;
  ret.ret_to_hal.ret = FALSE;
  return ret;
}
