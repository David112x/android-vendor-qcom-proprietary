/*Copyright (c) 2016, 2018 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * mct_shim_layer.c
 *
 * This file contains the mct_shimlayer_process helper function implementation.
 * After receiving a command from HAL, these helper functions pass on the commands
 * to modules for further processing.
 */
#include <sys/un.h>
#include "mct_shim_layer.h"
#include "mct_module.h"
#include "cam_intf.h"
#include "camera_dbg.h"
#include <sys/sysinfo.h>
#include "memleak.h"

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
int config_fd = -1;
pthread_mutex_t session_mutex;

/** mct_shimlayer_module_sensor_init:
 *
 * This is a local function which is called for initing only
 * the sensor module.
 **/

static boolean mct_shimlayer_module_sensor_init(void)
{
  mct_module_t *temp = NULL;
  CLOGH(CAM_SHIM_LAYER, "Sensor module init");
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
  CLOGH(CAM_SHIM_LAYER, "Sensor module init done");
  return TRUE;
}

static boolean get_config_node_name(char *node_name)
{

  int num_media_devices = 0;
  char dev_name[MAX_DEV_NAME_SIZE];
  int dev_fd = 0;
  int rc = 0;
  struct media_device_info mdev_info;
  int num_entities = 0;

  while (1) {
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices++);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd < 0) {
      /* Done enumerating media devices */
      break;
    } else {
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

    num_entities = 0;
    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities | MEDIA_ENT_ID_FLAG_NEXT;
      rc = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (rc < 0) {
        rc = 0;
        break;
      }
      num_entities = entity.id;
      if (mct_util_match_subdev_type(entity, QCAMERA_VNODE_GROUP_ID,
        MEDIA_ENT_T_DEVNODE_V4L)) {
        /* found the video device */
        strlcpy(node_name, entity.name, MAX_DEV_NAME_SIZE);
      } else {
        close(dev_fd);
        return TRUE;
      }
    } /* enumerate entites */
    close(dev_fd);
  } /* enumerate media devices */

  return FALSE;
}

/** mct_shimlayer_module_init:
 *
 * During boot up upon the receiving command from HAL this function
 * inits all the modules barring sensor which is inited in a different function
 **/
boolean mct_shimlayer_module_init(void)
{
  mct_module_t *temp = NULL;
  int          i;

  CLOGH(CAM_SHIM_LAYER, "Module Init");
  for (i = 1;
       i < (int)(sizeof(modules_list)/sizeof(mct_module_init_name_t)); i++) {
    if( NULL == modules_list[i].init_mod)
      continue;
    CLOGD(CAM_SHIM_LAYER, "Module name : %s: E", modules_list[i].name);
    temp = modules_list[i].init_mod(modules_list[i].name);
    CLOGD(CAM_SHIM_LAYER, "Module name : %s: X", modules_list[i].name);
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
        mct_list_append(modules_all, temp, NULL, NULL);
      }
    }
  } /* for */
  CLOGH(CAM_SHIM_LAYER, "Init mods done");

  return TRUE;
}


/** mct_shimlayer_process_module_deinit:
 *
 * This function is called only incase of chipsets where RAM
 * is less than 512 MB to save memory when camera is not
 * operational
 **/
boolean mct_shimlayer_module_deinit(void)
{
  int i;
  mct_module_t *temp = NULL;

  for (i = 1;
       i < (int)(sizeof(modules_list)/sizeof(mct_module_init_name_t)); i++) {
    if( NULL == modules_list[i].deinit_mod)
      continue;
    modules_list[i].deinit_mod(modules_list[i].module);
  } /* for */

  mct_list_free_all(modules_all, NULL);
  modules_all = NULL;


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
  pthread_mutex_destroy(&session_mutex);
  return TRUE;
}

/** mct_shimlayer_process_module_init:
 *
 * This function is the direct call from HAL to init all the modules present
 * in backend. Based on the chipset i.e. where RAM is less than 512 MB, a decision
 * is made to init all modules or just the sensor module to save memory.
 **/
int mct_shimlayer_process_module_init(mm_camera_shim_ops_t
  *shim_ops_tbl)
{
  mct_module_t *temp = NULL;
  int32_t enabled_savemem = 0;
  char savemem[128];
  char config_node_name[MAX_DEV_NAME_SIZE];
  char dev_name[MAX_DEV_NAME_SIZE];
  int rc = 0;
  struct msm_v4l2_event_data event_data;
  pthread_t thread;
  pthread_attr_t attr;
  char prop[PROPERTY_VALUE_MAX];
  int enable_memleak = 0;

  if (!shim_ops_tbl) {
    CLOGE(CAM_SHIM_LAYER, "Ops table NULL");
    return FALSE;
  }

  #if defined(LOG_DEBUG)
    cam_debug_open();
  #endif

  pthread_mutex_init(&session_mutex, NULL);
  if (get_config_node_name(config_node_name) == FALSE) {
    CLOGE(CAM_SHIM_LAYER, "Failed to get config node name");
  }

  snprintf(dev_name, sizeof(dev_name), "/dev/%s", config_node_name);
  config_fd = open(dev_name, O_RDWR | O_NONBLOCK);
  if (config_fd < 0) {
    CLOGE(CAM_SHIM_LAYER, "Failed to open config node. errno %d (%s)", errno, strerror(errno));
    sleep(1);
    exit(1);
  }

  property_get("vendor.camera.cameradaemon.SaveMemAtBoot", savemem, "0");
  enabled_savemem = atoi(savemem);

  if (mct_shimlayer_module_sensor_init() == FALSE) {
    CLOGE(CAM_SHIM_LAYER, "sensor module init failed");
    return FALSE;
  }
  if (enabled_savemem != 1) {
    if (mct_shimlayer_module_init() == FALSE) {
      CLOGE(CAM_SHIM_LAYER, "Module init failed");
      return FALSE;
    }
  }
  /*Sending IOCTL to inform kernel that daemon is not present */
  rc = ioctl(config_fd, MSM_CAM_V4L2_IOCTL_DAEMON_DISABLED, &event_data);
  if (rc < 0) {
    CLOGE(CAM_SHIM_LAYER, "Failed to send Daemon disabled IOCTL to kernel");
  }

  shim_ops_tbl->mm_camera_shim_open_session = mct_shimlayer_start_session;
  shim_ops_tbl->mm_camera_shim_close_session = mct_shimlayer_stop_session;
  shim_ops_tbl->mm_camera_shim_send_cmd = mct_shimlayer_process_event;

  /*Initializing memleak tool if setprop is enabled */
#ifdef MEMLEAK_FLAG
  property_get("persist.vendor.camera.memleak.enable", prop, "0");
  enable_memleak = atoi(prop);
  if (enable_memleak) {
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&thread, &attr, server_memleak_thread, NULL);
    pthread_setname_np(thread, "CAM_memleak");
  }
#endif

  return TRUE;
}

/** mct_shimlayer_handle_parm
 * @parm_event: It contains the commands to be performed on backend
 * Return: TRUE/FALSE
 *.This function passes the payload/command from HAL to MCT.
 **/
static boolean mct_shimlayer_handle_parm(cam_shim_cmd_type cmd_type,
  uint32_t session_id, cam_shim_cmd_data *parm_event)
{
  mct_serv_msg_t serv_msg;
  struct v4l2_event msg;
  struct msm_v4l2_event_data *data =
    (struct msm_v4l2_event_data*)&msg.u.data[0];

  data->command = parm_event->command;
  data->stream_id = parm_event->stream_id;

  if (parm_event->value) {
    data->arg_value = *(unsigned int *)parm_event->value;
  }
  data->session_id = session_id;

  if (cmd_type == CAM_SHIM_GET_PARM) {
    cmd_type = MSM_CAMERA_GET_PARM;
  } else {
    cmd_type = MSM_CAMERA_SET_PARM;
  }

  serv_msg.msg_type  = SERV_MSG_HAL;
  serv_msg.u.hal_msg = msg;
  serv_msg.u.hal_msg.id = cmd_type;

  if (mct_controller_proc_serv_msg(&serv_msg) == FALSE) {
    CLOGE(CAM_SHIM_LAYER, "HAL event processing failed");
    return FALSE;
  }

  return TRUE;
}

/** mct_shimlayer_reg_buffer
 * @session: session for which the buffer needs to be mapped.
 * @pkt: This is the payload which contains information about the
 * operation to be carried out like mapping/unmapping
 * Return: TRUE/FALSE
 *.This function handles the mapping and unmapping of buffer requests coming from
 * HAL.
 **/
 boolean mct_shimlayer_reg_buffer(const int session,
   cam_reg_buf_t *packet)
{
  mct_serv_msg_t     serv_msg;
  uint32_t i;
  serv_msg.msg_type = SERV_MSG_DS;
  serv_msg.u.ds_msg.session  = session;
  serv_msg.u.ds_msg.operation = (unsigned int)packet->msg_type;

  switch (packet->msg_type) {

    case CAM_MAPPING_TYPE_FD_BUNDLED_MAPPING: {
      serv_msg.u.ds_msg.num_bufs = packet->payload.buf_map_list.length;
      for (i= 0; i < serv_msg.u.ds_msg.num_bufs; i++) {
        serv_msg.u.ds_msg.ds_buf[i].buf_type  =
          (unsigned int)packet->payload.buf_map_list.buf_maps[i].type;
        serv_msg.u.ds_msg.ds_buf[i].fd =
          packet->payload.buf_map_list.buf_maps[i].fd;
        serv_msg.u.ds_msg.ds_buf[i].stream =
         packet->payload.buf_map_list.buf_maps[i].stream_id;
        serv_msg.u.ds_msg.ds_buf[i].size =
          packet->payload.buf_map_list.buf_maps[i].size;
        serv_msg.u.ds_msg.ds_buf[i].index =
          packet->payload.buf_map_list.buf_maps[i].frame_idx;
        serv_msg.u.ds_msg.ds_buf[i].plane_idx =
          packet->payload.buf_map_list.buf_maps[i].plane_idx;
#ifndef DAEMON_PRESENT
        serv_msg.u.ds_msg.ds_buf[i].buffer =
          packet->payload.buf_map_list.buf_maps[i].buffer;
#endif
      }
    }
     break;

    case CAM_MAPPING_TYPE_FD_BUNDLED_UNMAPPING: {
      serv_msg.u.ds_msg.num_bufs = packet->payload.buf_map_list.length;
      for (i= 0; i <= serv_msg.u.ds_msg.num_bufs; i++) {
        serv_msg.u.ds_msg.ds_buf[i].buf_type =
          (int)packet->payload.buf_unmap_list.buf_unmaps[i].type;
        serv_msg.u.ds_msg.ds_buf[i].stream =
          packet->payload.buf_unmap_list.buf_unmaps[i].stream_id;
        serv_msg.u.ds_msg.ds_buf[i].index  =
          packet->payload.buf_unmap_list.buf_unmaps[i].frame_idx;
      }
    }
      break;

    case CAM_MAPPING_TYPE_FD_MAPPING: {
      serv_msg.u.ds_msg.num_bufs = 1;
      serv_msg.u.ds_msg.ds_buf[0].buf_type =
        (unsigned int)packet->payload.buf_map.type;
      serv_msg.u.ds_msg.ds_buf[0].fd        = packet->payload.buf_map.fd;
      serv_msg.u.ds_msg.ds_buf[0].stream    = packet->payload.buf_map.stream_id;
      serv_msg.u.ds_msg.ds_buf[0].size      = packet->payload.buf_map.size;
      serv_msg.u.ds_msg.ds_buf[0].index     = packet->payload.buf_map.frame_idx;
      serv_msg.u.ds_msg.ds_buf[0].plane_idx = packet->payload.buf_map.plane_idx;
#ifndef DAEMON_PRESENT
      serv_msg.u.ds_msg.ds_buf[0].buffer =    packet->payload.buf_map.buffer;
#endif
    }
      break;

    case CAM_MAPPING_TYPE_FD_UNMAPPING: {
      serv_msg.u.ds_msg.num_bufs = 1;
      serv_msg.u.ds_msg.ds_buf[0].buf_type =
        (int)packet->payload.buf_unmap.type;
      serv_msg.u.ds_msg.ds_buf[0].stream =
        packet->payload.buf_unmap.stream_id;
      serv_msg.u.ds_msg.ds_buf[0].index  =
        packet->payload.buf_unmap.frame_idx;
    }
      break;

    default:
      break;
  }

  if (mct_controller_proc_serv_msg(&serv_msg) == FALSE) {
    CLOGE(CAM_SHIM_LAYER, "Buffer reg/unreg request failed")
    return FALSE;
  }

  return TRUE;
}

/** mct_shimlayer_handle_bundle_event
 * @bundle_event: It contains the multiple command and payload for the action
 * to be performed by backend.
 * Return: TRUE/FALSE
 *.This function passes the bundled payload/command from HAL to MCT.
 **/
static boolean mct_shimlayer_handle_bundle_event(uint32_t session_id,
  cam_shim_stream_cmd_packet_t *bundle_cmd)
{
  uint32_t i,j, session;
  uint32_t num_stream = bundle_cmd->stream_count;
  cam_shim_cmd_packet_t *stream_event = NULL;;
  uint8_t event_count = 0;
  cam_shim_packet_t *event = NULL;
  boolean rc = FALSE;
  uint32_t cmd_type;

  session = session_id;
  for (i = 0; i < num_stream && i < MAX_NUM_STREAMS; i++) {
     event = bundle_cmd->stream_event[i].cmd;
     event_count = bundle_cmd->stream_event[i].cmd_count;
     if (event) {
       for (j = 0; j < event_count; j++) {
         if (event[j].cmd_type == CAM_SHIM_GET_PARM ||
           event[j].cmd_type == CAM_SHIM_SET_PARM) {
           rc = mct_shimlayer_handle_parm(event[j].cmd_type, session,
             &event[j].cmd_data);
           if (rc == FALSE) {
             CLOGE(CAM_SHIM_LAYER, "HAL parm processing failed in a bundled cmd");
             return FALSE;
           }
         } else if(event[j].cmd_type == CAM_SHIM_REG_BUF) {
           session = event[j].session_id;
           rc = mct_shimlayer_reg_buffer(session, &event[j].reg_buf);
           if (rc == FALSE) {
             CLOGE(CAM_SHIM_LAYER, "Failed HAL buffer processing in a bundled cmd");
             return FALSE;
           }
         } else if (event[j].cmd_type == CAM_SHIM_BUNDLE_CMD) {
           rc = mct_shimlayer_handle_bundle_event(session, &event[j].bundle_cmd);
           if (rc == FALSE) {
             CLOGE(CAM_SHIM_LAYER, "Failed processing HAL bundled event");
             return FALSE;
           }
         }
      }
    }
  }
  return TRUE;
}

/*
 * mct_shimlayer_process_event
 * @event: payload received from HAL which contain info about the operation
 *   to be processed in backend.
 * This function processes the command from HAL
**/
int mct_shimlayer_process_event(cam_shim_packet_t *packet)
{

  cam_shim_cmd_data *parm_event;
  cam_reg_buf_t *reg_buf;
  cam_shim_stream_cmd_packet_t *bundle_event;
  boolean rc = FALSE;
  uint32_t session_id;
  uint32_t cmd_type;

  CLOGH(CAM_SHIM_LAYER, "Received event from HAL type =%d", packet->cmd_type);

  switch (packet->cmd_type) {
    case CAM_SHIM_SET_PARM:
    case CAM_SHIM_GET_PARM: {
      session_id = packet->session_id;
      parm_event = &packet->cmd_data;
      rc = mct_shimlayer_handle_parm(packet->cmd_type, session_id, parm_event);
      if (rc == FALSE) {
        CLOGE(CAM_SHIM_LAYER, "Failed to process HAL parm command");
        return CAM_STATUS_FAILED;
      }
    }
      break;

    case CAM_SHIM_REG_BUF: {
      session_id = packet->session_id;
      reg_buf = &packet->reg_buf;
      rc = mct_shimlayer_reg_buffer(session_id, reg_buf);
      if (rc == FALSE) {
        CLOGE(CAM_SHIM_LAYER, "Failed to process HAL buf reg command");
        return CAM_STATUS_FAILED;
      }
    }
      break;

    case CAM_SHIM_BUNDLE_CMD: {
      session_id = packet->session_id;
      bundle_event = &packet->bundle_cmd;
      rc = mct_shimlayer_handle_bundle_event(session_id, bundle_event);
      if (rc == FALSE) {
        CLOGE(CAM_SHIM_LAYER, "Failed to process bundled HAL event");
        return CAM_STATUS_FAILED;
      }
    }
      break;

    default: {
      CLOGE(CAM_SHIM_LAYER, "Unsupported command from HAL");
      return CAM_STATUS_FAILED;
    }
      break;

  CLOGH(CAM_SHIM_LAYER, "Processed event from HAL type =%d", packet->cmd_type);
  }
  return CAM_STATUS_SUCCESS;
}

/*
 * mct_shimlayer_init_session
 * @session: session which needs to be started.
 * @callback: callback function sent from HAL which is invoked
 *    incase of any failure or to pass any notiifcation from MCT to HAL.
 * This function to start a session This can be called multiple times from
 * HAL with different session ids.
 * Return: cam_status_t: It can return CAM_STATUS_BUSY/CAM_STATUS_FAILED/
 *            CAM_STATUS_SUCCESS based on result.
**/
cam_status_t mct_shimlayer_start_session(int session,
  mm_camera_shim_event_handler_func event_cb)
{
  int32_t enabled_savemem;
  char savemem[128];
  cam_status_t ret = CAM_STATUS_FAILED;
  char prop[PROPERTY_VALUE_MAX];
  int enable_memleak = 0;

#ifdef MEMLEAK_FLAG
  property_get("persist.vendor.camera.memleak.enable", prop, "0");
  enable_memleak = atoi(prop);
  if (enable_memleak) {
    CLOGH(CAM_MCT_MODULE, "Memory leak tracking enabled.");
    enable_memleak_trace(0);
  }
#endif

  pthread_mutex_lock(&session_mutex);
  property_get("vendor.camera.cameradaemon.SaveMemAtBoot", savemem, "0");
  enabled_savemem = atoi(savemem);

  if (enabled_savemem == 1) {
    if (mct_shimlayer_module_init() == FALSE) {
      pthread_mutex_unlock(&session_mutex);
      return CAM_STATUS_FAILED;
    }
  }

  ret = mct_controller_new(modules, session, config_fd, event_cb);
  if (ret == CAM_STATUS_BUSY || ret == CAM_STATUS_FAILED) {
    pthread_mutex_unlock(&session_mutex);
    CLOGE(CAM_SHIM_LAYER,"Session creation for session =%d failed with err %d",
      session, ret);
  /*Signalling memleak thread to print if any memory leak present */
#ifdef MEMLEAK_FLAG
    if (enable_memleak) {
      CLOGH(CAM_MCT_MODULE, "Signal to print memory leak");
      if (pthread_mutex_trylock (&server_memleak_mut) == 0) {
        server_memleak_event = PRINT_LEAK_MEMORY;
        pthread_cond_signal(&server_memleak_con);
        pthread_mutex_unlock (&server_memleak_mut);
      }
    }
#endif
    return ret;
  }
  pthread_mutex_unlock(&session_mutex);
  return ret;
}

/*
 * mct_shimlayer_deinit_session
 * @session: session which needs to be started.
 * This function destroys the session associated with the session.
 * Return: TRUE/FALSE based on success or failure.
 **/
int mct_shimlayer_stop_session(int session)
{
  boolean rc = 0;
  int32_t enabled_savemem;
  char savemem[128];
  char prop[PROPERTY_VALUE_MAX];
  int enable_memleak = 0;

  pthread_mutex_lock(&session_mutex);
  rc = mct_controller_destroy(session);

  property_get("vendor.camera.cameradaemon.SaveMemAtBoot", savemem, "0");
  enabled_savemem = atoi(savemem);

  if (enabled_savemem == 1) {
    if (mct_shimlayer_module_deinit() == FALSE) {
      pthread_mutex_unlock(&session_mutex);
      return FALSE;
    }
  }
  /*Signalling memleak thread to print if any memory leak present */
#ifdef MEMLEAK_FLAG
  property_get("persist.vendor.camera.memleak.enable", prop, "0");
  enable_memleak = atoi(prop);
  if (enable_memleak) {
    CLOGH(CAM_MCT_MODULE, "Signal to print memory leak");
    if (pthread_mutex_trylock (&server_memleak_mut) == 0) {
      server_memleak_event = PRINT_LEAK_MEMORY;
      pthread_cond_signal(&server_memleak_con);
      pthread_mutex_unlock (&server_memleak_mut);
    }
  }
#endif
  if (!rc) {
    CLOGE(CAM_SHIM_LAYER, "Session destroy failed session =%d", session);
    pthread_mutex_unlock(&session_mutex);
    return FALSE;
  }
  pthread_mutex_unlock(&session_mutex);
  return TRUE;
}
