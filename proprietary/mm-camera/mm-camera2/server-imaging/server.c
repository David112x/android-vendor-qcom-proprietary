/* server.c
 *
 * This file contains the server implementation. All commands coming
 * from the HAL arrive here first.
 *
 * Copyright (c) 2012-2015, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "server_process.h"

#include "camera_dbg.h"
#include "mct_profiler.h"
#include <sys/sysinfo.h>
#include <media/msm_cam_sensor.h>
#include "mct_util.h"
#include <sys/resource.h>
#include "mct_debug_fdleak.h"

#define LOGSYNC_FILENAME_SIZE 254
static pthread_mutex_t logsync_mutex;
/** read_fd_type:
 *    defines server selected FD types
 **/
typedef enum _read_fd_type {
  RD_FD_HAL,
  RD_DS_FD_HAL,
  RD_PIPE_FD_MCT,
  RD_FD_NONE
} read_fd_type;

/** read_fd_info_t:
 *    @type    -- either domain socket fd or mct fd
 *    @session -- session index
 *    @fd      --
 *      in case of domain socket fd: fd[0]
 *      in case of mct pipe fd: fd[0] - server read fd
 *                              fd[1] - mct write fd
 **/
typedef struct _read_fd_info {
  read_fd_type type;
  unsigned int session;
  int fd[2];
} read_fd_info_t;

typedef struct _server_select_fds {
  fd_set fds;
  int select_fd;
} server_select_fds_t;

/** server_find_listen_fd:
 *
 *  Return: TRUE if fd is set
 **/
static boolean server_check_listen_fd(void *data1, void *data2)
{
  return FD_ISSET(((read_fd_info_t *)data1)->fd[0], (fd_set *)data2);
}

/** server_find_listen_fd:
 *    @data1:
 *    @data2:
 *
 * return TRUE if the two FDs match.
 **/
static boolean server_find_listen_fd(void *data1, void *data2)
{
  read_fd_info_t *fd_info_match = (read_fd_info_t *)data2;
  read_fd_info_t *fd_info       = (read_fd_info_t *)data1;

  return ((fd_info_match->type == fd_info->type) &&
          ((fd_info_match->session == fd_info->session)));
}

/** server_reset_select_fd:
 *    @data:
 *    @user_data:
 *
 **/
static boolean server_reset_select_fd(void *data, void *user_data)
{
  read_fd_info_t      *fd_info  = (read_fd_info_t *)data;
  server_select_fds_t *selected = (server_select_fds_t *)user_data;

  FD_SET(fd_info->fd[0], &(selected->fds));
  selected->select_fd = MTYPE_MAX(selected->select_fd, fd_info->fd[0]);

  return TRUE;
}

/** get_server_node_name:
 *    @node_name
 *
 *
 **/
static boolean get_server_node_name(char *node_name)
{

  int num_media_devices = 0;
  char dev_name[MAX_DEV_NAME_SIZE];
  int dev_fd = 0;
  int rc = 0;
  struct media_device_info mdev_info;
  int last_entity = 0;

  while (1) {
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices++);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);

    if (dev_fd < 0) {
      /* Done enumerating media devices */
      break;
    }

    memset(&mdev_info, 0, sizeof(struct media_device_info));
    rc = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (rc < 0) {
      close(dev_fd);
      break;
    }

    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model)) != 0) {
      close(dev_fd);
      continue;
    }

    last_entity = 0;
    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = last_entity | MEDIA_ENT_ID_FLAG_NEXT;
      rc = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (rc < 0) {
        rc = 0;
        break;
      }
      last_entity = entity.id;
      CLOGD(CAM_MCT_MODULE, "entity name %s type %x group id %d",
        entity.name, entity.type, entity.group_id);

      if (mct_util_match_subdev_type(entity, QCAMERA_VNODE_GROUP_ID,
        MEDIA_ENT_T_DEVNODE_V4L)) {
        /* found the video device */
        strlcpy(node_name, entity.name, MAX_DEV_NAME_SIZE);
        CLOGD(CAM_MCT_MODULE, "node_name = %s", node_name);
        close(dev_fd);
        return TRUE;
      }
    } /* enumerate entites */
    close(dev_fd);
  } /* enumerate media devices */
  CLOGE(CAM_MCT_MODULE, "config node not found.");
  return FALSE;
}

sig_atomic_t signal_received;

void server_sigaction(int signum)
{
  FILE *fp;
  static uint64_t seq_num;
  char value[PROPERTY_VALUE_MAX] = {0};
  char debugfs_root[PROPERTY_VALUE_MAX] = {0};
  char logsync_file[LOGSYNC_FILENAME_SIZE + 1] = {0};
  int32_t logsync_en = 0;

  if (signum != SIGTERM && signum != SIGUSR1) {
    CLOGE(CAM_MCT_MODULE, "RECEIVED unexpected signal[%d] instead of \
      [%d] or [%d]", signum, SIGTERM, SIGUSR1);
      return;
  }

  if (signum == SIGTERM) {
    CLOGE(CAM_MCT_MODULE, "SIGTERM signal received");
    signal_received = 1;
  }

  property_get("persist.vendor.camera.logsync", value, "0");
  logsync_en = atoi(value);

  if (signum == SIGUSR1 && logsync_en) {
    property_get("persist.vendor.camera.debugfs_root", debugfs_root,
                                                "/sys/kernel/debug");
    snprintf(logsync_file, sizeof(logsync_file), "%s/%s/%s", debugfs_root,
             MSM_CAM_LOGSYNC_FILE_BASEDIR,
             MSM_CAM_LOGSYNC_FILE_NAME);
    fp = fopen(logsync_file, "w");
    if (!fp) {
      CLOGE(CAM_MCT_MODULE, "Failed to open file\n");
      return;
    }
    pthread_mutex_lock(&logsync_mutex);
    CLOGE(CAM_MCT_MODULE, "LOGSYNC (User Space): seq_num = %llu", seq_num);
    fprintf(fp, "%llu", seq_num);
    seq_num++;
    pthread_mutex_unlock(&logsync_mutex);
    fclose(fp);
  }
}

boolean server_register_for_kill_signal(struct sigaction *default_sa)
{
  struct sigaction sig_act;
  signal_received = 0;
  char value[PROPERTY_VALUE_MAX];
  int32_t logsync_en = 0;

  pthread_mutex_init(&logsync_mutex, NULL);
  property_get("persist.vendor.camera.logsync", value, "0");
  logsync_en = atoi(value);

  sig_act.sa_handler = server_sigaction;
  sigemptyset(&sig_act.sa_mask);
  sigaddset(&sig_act.sa_mask, SIGTERM);
  if (logsync_en)
    sigaddset(&sig_act.sa_mask, SIGUSR1);
  sig_act.sa_flags = SA_SIGINFO;
  sig_act.sa_restorer = NULL;
  if(sigaction(SIGTERM, &sig_act, default_sa) < 0) {
    CLOGE(CAM_MCT_MODULE, "Failed to register signal handler \
      w/ errno %s", strerror(errno));
    return FALSE;
  }

  if (logsync_en) {
    if(sigaction(SIGUSR1, &sig_act, default_sa) < 0) {
      CLOGE(CAM_MCT_MODULE, "Failed to register SIGUSR1 signal handler \
        w/ errno %s", strerror(errno));
      return FALSE;
    }
  }
  return TRUE;
}

boolean server_register_timer_cb(timer_t *ptimer)
{
  struct sigevent sev;
  struct itimerspec its;
  int rc = 0;
  char value[PROPERTY_VALUE_MAX];
  int32_t logsync_int = 0;
  int32_t logsync_en = 0;

  property_get("persist.vendor.camera.logsync.interval", value, "5");
  logsync_int = atoi(value);

  property_get("persist.vendor.camera.logsync", value, "0");
  logsync_en = atoi(value);

  if (!logsync_en)
    return FALSE;

  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_notify_function = NULL;
  sev.sigev_signo = SIGUSR1;
  sev.sigev_value.sival_ptr = ptimer;
  rc = timer_create(CLOCK_REALTIME, &sev, ptimer);
  if (rc) {
    CLOGE(CAM_MCT_MODULE, "Failed:  installing callback timer");
    return FALSE;
  }

  its.it_value.tv_sec = logsync_int <= 5 ? 5 : logsync_int;
  its.it_value.tv_nsec = 0;
  its.it_interval.tv_sec = its.it_value.tv_sec;
  its.it_interval.tv_nsec = its.it_value.tv_nsec;

  if(!timer_settime(*ptimer, 0, &its, NULL)) {
    return TRUE;
  } else {
    CLOGE(CAM_MCT_MODULE, "Failed: setting callback timer");
    timer_delete(*ptimer);
    return FALSE;
  }
  return TRUE;
}

/** main:
 *
 **/
int main(int argc __unused, char *argv[] __unused)
{
  struct v4l2_event_subscription subscribe;
  struct v4l2_event              event;

  mct_list_t                *listen_fd_list = NULL;
  read_fd_info_t            *hal_fd = NULL, *hal_ds_fd = NULL, *mct_fds = NULL;
  server_select_fds_t        select_fds;
  serv_proc_ret_t            proc_ret;
  int mct_t_ret = 0;
  struct timespec timeToWait;

  struct msm_v4l2_event_data *ret_data;
  char serv_hal_node_name[MAX_DEV_NAME_SIZE];
  char dev_name[MAX_DEV_NAME_SIZE];
  char probe_done_node_name[MAX_DEV_NAME_SIZE];
  char probe_done_dev_name[MAX_DEV_NAME_SIZE];
  int probe_done_fd;
  struct sensor_init_cfg_data cfg;
  int  ret, i, j;
  mode_t old_mode;
  struct sigaction default_sa;
  boolean is_signal_registered = FALSE;
  boolean is_timer_cb_registered = FALSE;
  timer_t timer_id = 0;
  uint32_t num_sessions = 0;
  int32_t enabled_savemem = 0;
  char savemem[92];
  int32_t ret_subdev;
  struct rlimit rl;
  char prop[PROPERTY_VALUE_MAX];
  int enable_fdleak = 0;
#ifdef GCOV_TOOL
  int is_exit = 0;
  char property_value[PROPERTY_VALUE_MAX] = {0};
#endif

  old_mode = umask(S_IRWXO);
   if (getrlimit(RLIMIT_CORE, &rl) == 0) {
     CLOGD(CAM_MCT_MODULE,"get rimit core %d",rl.rlim_cur);
     rl.rlim_cur = RLIM_INFINITY;
     if (setrlimit(RLIMIT_CORE, &rl) != 0) {
       CLOGE(CAM_MCT_MODULE,"can't get rlimit");
     }
     if (getrlimit(RLIMIT_CORE, &rl) != 0) {
       CLOGE(CAM_MCT_MODULE,"can't get rlimit");
     }
     CLOGD(CAM_MCT_MODULE," set rimit core %d",rl.rlim_cur);
   }

#if defined(LOG_DEBUG)
  cam_debug_open();
#endif

  CLOGD(CAM_MCT_MODULE, "CAMERA_DAEMON: start of camera Daemon.");
  /* 1. find server node name and open the node */

  if (get_server_node_name(serv_hal_node_name) == FALSE){
    CLOGE(CAM_MCT_MODULE, "Going to bad node");
    goto bad_node_fd;
  }

  CLOGD(CAM_MCT_MODULE, "after get_server_node_name");
  hal_fd = malloc(sizeof(read_fd_info_t));
  if (hal_fd == NULL)
    goto bad_node_fd;

  snprintf(dev_name, sizeof(dev_name), "/dev/%s", serv_hal_node_name);
  hal_fd->fd[0] = open(dev_name, O_RDWR | O_NONBLOCK);
  if (hal_fd->fd[0] < 0)
    goto open_hal_fail;

  hal_fd->type = RD_FD_HAL;

  listen_fd_list = mct_list_append(listen_fd_list, hal_fd, NULL, NULL);
  if (!listen_fd_list)
    goto list_append_fail;

  property_get("vendor.camera.cameradaemon.SaveMemAtBoot", savemem, "0");
  enabled_savemem = atoi(savemem);

  CLOGD(CAM_MCT_MODULE, "CAMERA_DAEMON: start all modules init");
  /* 2. after open node, initialize modules */
  if(server_process_module_sensor_init() == FALSE)
    goto module_init_fail;
  CLOGD(CAM_MCT_MODULE, "CAMERA_DAEMON:End of all modules init");

  if (enabled_savemem != 1) {
    if (server_process_module_init() == FALSE)
      goto module_init_fail;
  }

  /* Subcribe V4L2 event */
  memset(&subscribe, 0, sizeof(struct v4l2_event_subscription));
  subscribe.type = MSM_CAMERA_V4L2_EVENT_TYPE;
  for (i = MSM_CAMERA_EVENT_MIN + 1; i < MSM_CAMERA_EVENT_MAX; i++) {
    subscribe.id = i;
    if (ioctl(hal_fd->fd[0], VIDIOC_SUBSCRIBE_EVENT, &subscribe) < 0)
      goto subscribe_failed;
  }

  signal_received = 0;
  select_fds.select_fd = hal_fd->fd[0];
  /* create a timer */
  mct_t_ret = mct_util_create_timer();

  ret_subdev = mct_util_find_v4l2_subdev(probe_done_node_name);
  if (ret_subdev < 0) {
    CLOGE(CAM_MCT_MODULE, "can not find subdev");
    goto subscribe_failed;
  }

  snprintf(probe_done_dev_name, sizeof(probe_done_dev_name), "/dev/%s",
    probe_done_node_name);
  probe_done_fd = open(probe_done_dev_name, O_RDWR | O_NONBLOCK);

  if (probe_done_fd < 0)
    goto subscribe_failed;

  cfg.cfgtype = CFG_SINIT_PROBE_DONE;
  if (ioctl(probe_done_fd, VIDIOC_MSM_SENSOR_INIT_CFG, &cfg) < 0) {
    CLOGE(CAM_MCT_MODULE, "ioctl SENSOR_INIT_CFG failed");
    ret = FALSE;
  }
  close(probe_done_fd);
  CLOGD(CAM_MCT_MODULE, "CAMERA_DAEMON:waiting for camera to open");
  do {

    FD_ZERO(&(select_fds.fds));
    mct_list_traverse(listen_fd_list, server_reset_select_fd, &select_fds);

    /* no timeout */
    ret = select(select_fds.select_fd + 1, &(select_fds.fds), NULL, NULL, NULL);

    if (ret > 0) {

      mct_list_t     *find_list;
      read_fd_info_t *fd_info;

      find_list = mct_list_find_custom(listen_fd_list, &(select_fds.fds),
        server_check_listen_fd);
      if (!find_list)
        continue;

      fd_info = (read_fd_info_t *)find_list->data;


      switch (fd_info->type) {
      case RD_FD_HAL: {

        if (ioctl(fd_info->fd[0], VIDIOC_DQEVENT, &event) < 0) {
          continue;
        }
        /* server process HAL event:
         *
         *   1. if it returns success, it means the event message has been
         *      posted to MCT, don't need to send CMD ACK back to kernel
         *      immediately, because MCT will notify us after process;
         *
         *   2. if it returns failure, it means the event message was not
         *      posted to MCT successfully, hence we need to send CMD ACK back
         *      to kernel immediately so that HAL thread which sends this
         *      event can be blocked.
         */

      timeToWait.tv_sec  = MCT_THREAD_TIMEOUT;
      timeToWait.tv_nsec = 0;
      /*start timer*/
      if (!mct_t_ret)
        mct_util_set_timer(&timeToWait);

        proc_ret = server_process_hal_event(&event);

      /*stop timer */
      if (!mct_t_ret) {
        timeToWait.tv_sec  = 0;
        timeToWait.tv_nsec = 0;
        mct_util_set_timer(&timeToWait);
      }
      }
        break;

      case RD_DS_FD_HAL:
      timeToWait.tv_sec  = MCT_THREAD_TIMEOUT;
      timeToWait.tv_nsec = 0;
      /*start timer*/
      if (!mct_t_ret)
        mct_util_set_timer(&timeToWait);

        /* server process message sent by HAL through Domain Socket */
        proc_ret = server_process_hal_ds_packet(fd_info->fd[0],
          fd_info->session);

      /*stop timer */
      if (!mct_t_ret) {
        timeToWait.tv_sec  = 0;
        timeToWait.tv_nsec = 0;
        mct_util_set_timer(&timeToWait);
      }

        break;

      case RD_PIPE_FD_MCT:
        /* server process message sent by media controller
         * through pipe: */
        proc_ret = server_process_mct_msg(fd_info->fd[0],
          fd_info->session);
        break;

      default:
        continue;
      } /* switch (fd_info->type) */

      switch (proc_ret.result) {
      case RESULT_NEW_SESSION: {
        struct msm_v4l2_event_data *ret_data =
          (struct msm_v4l2_event_data *)proc_ret.ret_to_hal.ret_event.u.data;
        if( ret_data->status == MSM_CAMERA_CMD_SUCCESS) {
          num_sessions++;
          hal_ds_fd = malloc(sizeof(read_fd_info_t));
          if (!hal_ds_fd) {
            /* Shouldn't end directly, need to shutdown MCT thread */
            goto server_proc_new_session_error;
          } else {
            hal_ds_fd->session = proc_ret.new_session_info.session_idx;
            hal_ds_fd->fd[0]   = proc_ret.new_session_info.hal_ds_fd;
            hal_ds_fd->type    = RD_DS_FD_HAL;
          }

          mct_fds = malloc(sizeof(read_fd_info_t));
          if (!mct_fds) {
            free(hal_ds_fd);
            hal_ds_fd = NULL;
            goto server_proc_new_session_error;
          } else {
            mct_fds->session = proc_ret.new_session_info.session_idx;
            mct_fds->fd[0]   = proc_ret.new_session_info.mct_msg_rd_fd;
            mct_fds->fd[1]   = proc_ret.new_session_info.mct_msg_wt_fd;
            mct_fds->type    = RD_PIPE_FD_MCT;
          }

          listen_fd_list = mct_list_append(listen_fd_list,hal_ds_fd,NULL,NULL);
          if (!listen_fd_list) {
            free(hal_ds_fd);
            free(mct_fds);
            goto server_proc_new_session_error;
          }

          listen_fd_list = mct_list_append(listen_fd_list, mct_fds, NULL,NULL);
          if (!listen_fd_list) {
            free(hal_ds_fd);
            free(mct_fds);
            goto server_proc_new_session_error;
          }
        } else {
          CLOGE(CAM_MCT_MODULE, "New session [%d] creation failed with error",
            ret_data->session_id);
        }
        if (TRUE == server_register_for_kill_signal(&default_sa)) {
          is_signal_registered = TRUE;
        }

        if (num_sessions == 1) {
          if (TRUE == server_register_timer_cb(&timer_id)) {
            is_timer_cb_registered = TRUE;
          }
        }

        goto check_proc_ret;
      } /* RESULT_NEW_SESSION */
        break;

      case RESULT_DEL_SESSION: {
        mct_list_t     *find_list;
        read_fd_info_t fd_info_match;
        read_fd_info_t *ds_fd_info = NULL, *mct_fd_info = NULL;
        struct msm_v4l2_event_data *event_data = (struct msm_v4l2_event_data *)
          &proc_ret.ret_to_hal.ret_event.u.data[0];

        num_sessions--;
        /* this is for Domain Socket FD */
        fd_info_match.type    = RD_DS_FD_HAL;
        fd_info_match.session = event_data->session_id;

        find_list = mct_list_find_custom(listen_fd_list, &fd_info_match,
          server_find_listen_fd);
        if (find_list) {
          ds_fd_info = (read_fd_info_t *)find_list->data;
          listen_fd_list = mct_list_remove(listen_fd_list, ds_fd_info);
          FD_CLR(ds_fd_info->fd[0], &select_fds.fds);
          close(ds_fd_info->fd[0]);
          free(ds_fd_info);
        }

        /* this is for MCT FD */
        fd_info_match.type    = RD_PIPE_FD_MCT;

        find_list = mct_list_find_custom(listen_fd_list, &fd_info_match,
          server_find_listen_fd);
        if (find_list) {
          mct_fd_info = (read_fd_info_t *)find_list->data;
          listen_fd_list = mct_list_remove(listen_fd_list, mct_fd_info);
          FD_CLR(mct_fd_info->fd[0], &select_fds.fds);
          close(mct_fd_info->fd[0]);
          close(mct_fd_info->fd[1]);
          free(mct_fd_info);
        }
#ifdef FDLEAK_FLAG
  property_get("persist.vendor.camera.fdleak.enable", prop, "0");
  enable_fdleak = atoi(prop);
  if (enable_fdleak) {
    CLOGI(CAM_MCT_MODULE,"fdleak tool is disable for camera daemon");
    mct_degug_dump_fdleak_trace();
   }
#endif
        if (num_sessions == 0 && is_timer_cb_registered) {
          if (timer_delete(timer_id)) {
            CLOGE(CAM_MCT_MODULE, "Failed to delete callback timer \
              w/ errno %s", strerror(errno));
          }
          is_timer_cb_registered = FALSE;
        }

      } /* case RESULT_DEL_SESSION */
         goto check_proc_ret;
        break;

      case RESULT_FAILURE:
        goto server_proc_error;
        break;

      case RESULT_SUCCESS:
        goto check_proc_ret;
        break;

      default:
        break;
      } /* switch (proc_ret.result) */

server_proc_new_session_error:
      event.id = MSM_CAMERA_DEL_SESSION;
      server_process_hal_event(&event);

server_proc_error:
      proc_ret.ret_to_hal.ret = TRUE;

check_proc_ret:
      if (proc_ret.ret_to_hal.ret == TRUE) {
        switch (proc_ret.ret_to_hal.ret_type) {
        /* @MSM_CAM_V4L2_IOCTL_CMD_ACK is Acknowledge to HAL's
         *   control command, which has command processing status.
         */
        case SERV_RET_TO_HAL_CMDACK:
          ioctl(hal_fd->fd[0], MSM_CAM_V4L2_IOCTL_CMD_ACK,
            (struct msm_v4l2_event_data *)&(proc_ret.ret_to_hal.ret_event.u.data));
          break;

        /* @MSM_CAM_V4L2_IOCTL_NOTIFY is MCT originated event such
         *   as meta data, SOF etc. Normally it comes
         *  1. domain socket buffer mapping process;
         *  2. from MCT.
         */
        case SERV_RET_TO_HAL_NOTIFY:
          ioctl(hal_fd->fd[0], MSM_CAM_V4L2_IOCTL_NOTIFY,
            &(proc_ret.ret_to_hal.ret_event.u.data));
          break;

        /* @MMSM_CAM_V4L2_IOCTL_NOTIFY_META is Meta data notification
         *   sent back to HAL during streaming. It is generated by
         *   BUS message.
         */
        case SERV_RET_TO_KERNEL_NOTIFY:
          ioctl(hal_fd->fd[0], MSM_CAM_V4L2_IOCTL_NOTIFY_DEBUG,
            &(proc_ret.ret_to_hal.ret_event.u.data));
          break;
        case SERV_RET_TO_HAL_NOTIFY_ERROR:
          CLOGE(CAM_MCT_MODULE, "main: camera daemon notify error");
          ioctl(hal_fd->fd[0], MSM_CAM_V4L2_IOCTL_NOTIFY_ERROR,
            &(proc_ret.ret_to_hal.ret_event.u.data));
          break;

        default:
          break;
        }
      }

#ifdef GCOV_TOOL
      property_get("persist.vendor.camera.debug.exit", property_value, "0");
      is_exit = atoi(property_value);

      if(is_exit && (RESULT_DEL_SESSION == proc_ret.result)) {
        extern void __gcov_flush();
        __gcov_flush();
      }
#else
      if(signal_received) {
        goto server_down;
      }
#endif
    } else {
      /* select failed. it cannot time out.*/
      /* TO DO: HANDLE ERROR */
       int rc = 0;
      CLOGE(CAM_MCT_MODULE, "Exit from select w/ errno %s", strerror(errno));
      if(signal_received) {
        CLOGE(CAM_MCT_MODULE, "sending MSM_CAM_V4L2_IOCTL_NOTIFY_ERROR");
        rc = ioctl(hal_fd->fd[0], MSM_CAM_V4L2_IOCTL_NOTIFY_ERROR,
          &(proc_ret.ret_to_hal.ret_event.u.data));
        if (rc < 0) {
          CLOGE(CAM_MCT_MODULE, "ioctl MSM_CAM_V4L2_IOCTL_NOTIFY_ERROR \
            failed rc:%d", rc);
        }
      }
    }

  } while (1);

server_down:
  // restore defualt signal handle
  if(TRUE == is_signal_registered) {
    if (sigaction(SIGTERM, &default_sa, NULL)) {
      CLOGE(CAM_MCT_MODULE, "Failed to restore default SIGTERM signal handlers\
        w/ errno %s", strerror(errno));
    }

    if (sigaction(SIGUSR1, &default_sa, NULL)) {
      CLOGE(CAM_MCT_MODULE, "Failed to restore default SIGUSR1 signal handlers\
        w/ errno %s", strerror(errno));
    }
 }

  if(TRUE == is_timer_cb_registered) {
    if (timer_delete(timer_id)) {
      CLOGE(CAM_MCT_MODULE, "Failed to delete callback timer \
        w/ errno %s", strerror(errno));
    }
    is_timer_cb_registered = FALSE;
  }

  /* while exit, if we let system to close domain socket FDn
   * then there can be race condition where HAL gets DAEMON_DIED, calls unmap
   * that can stuck as camera daemon is exiting.
   */
  if (hal_ds_fd) {
    close(hal_ds_fd->fd[0]);
  }

subscribe_failed:
  memset(&subscribe, 0, sizeof(struct v4l2_event_subscription));
  subscribe.type = MSM_CAMERA_V4L2_EVENT_TYPE;
  for (j = MSM_CAMERA_EVENT_MIN + 1; j < i; j++)  {
    subscribe.id = j;
    ioctl(hal_fd->fd[0], VIDIOC_UNSUBSCRIBE_EVENT, &subscribe);
  }
module_init_fail:
  mct_list_remove(listen_fd_list, hal_fd);
list_append_fail:
  close(hal_fd->fd[0]);
open_hal_fail:
  free(hal_fd);
bad_node_fd:

#if defined(LOG_DEBUG)
  cam_debug_close();
#endif

  return 0;
}
