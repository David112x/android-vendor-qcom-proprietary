/*============================================================================

  Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "eztune_diagnostics.h"
#include "cpp_hardware.h"
#include "cpp_module.h"
#include "cpp_log.h"
#include <linux/media.h>
#include <media/msmb_generic_buf_mgr.h>
#include <fcntl.h>
#include <errno.h>
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif
#if 0
#define CPPDUMP(fmt, args...) \
  ALOGE("CPPMETA "fmt"\n", ##args)
#else
#define CPPDUMP(fmt, args...) \
  do {} while (0);
#endif

#define MAX_SUPPORTED_RIM_PADDING 2

static int32_t cpp_thread_check_and_split_frame(cpp_module_ctrl_t* ctrl,
  struct msm_cpp_frame_info_t *partial_frame, cpp_hardware_t *cpphw);
static int32_t cpp_hardware_process_partial_frame(cpp_hardware_t *cpphw,
  struct msm_cpp_frame_info_t *partial_frame);
static inline bool cpp_hardware_streamoff_waitcheck(
  cpp_hardware_stream_status_t *stream_status);

int32_t cpp_hardware_notify_buf_done(cpp_hardware_t *cpphw, uint32_t identity);


/* Map CPP hardware version to firmware version */
int32_t cpp_hardware_select_fw_version(cpp_hardware_version_t hw_version,
  cpp_firmware_version_t *fw_version, char *fw_filename)
{
  if (!fw_filename || !fw_version) {
    CPP_ERR("fw_filename=%p, fw_version=%p", fw_filename, fw_version);
    return -EINVAL;
  }

  switch (hw_version) {
  case CPP_HW_VERSION_1_1_0:
    *fw_version = CPP_FW_VERSION_1_2_0;
    break;
  case CPP_HW_VERSION_1_1_1:
    *fw_version = CPP_FW_VERSION_1_2_0;
    break;
  case CPP_HW_VERSION_2_0_0:
    *fw_version = CPP_FW_VERSION_1_2_0;
    break;
  case CPP_HW_VERSION_4_0_0:
  case CPP_HW_VERSION_4_2_0:
    *fw_version = CPP_FW_VERSION_1_4_0;
    break;
  case CPP_HW_VERSION_4_3_0:
    *fw_version = CPP_FW_VERSION_1_5_1;
    break;
  case CPP_HW_VERSION_4_3_3:
  case CPP_HW_VERSION_4_3_4:
  case CPP_HW_VERSION_4_3_2:
    *fw_version = CPP_FW_VERSION_1_5_0;
    break;
  case CPP_HW_VERSION_4_1_0:
    *fw_version = CPP_FW_VERSION_1_6_0;
    break;
  case CPP_HW_VERSION_5_0_0:
  case CPP_HW_VERSION_5_1_0:
    *fw_version = CPP_FW_VERSION_1_8_0;
    break;
  case CPP_HW_VERSION_6_0_0:
    *fw_version = CPP_FW_VERSION_1_10_0;
    break;
  case CPP_HW_VERSION_6_1_0:
  case CPP_HW_VERSION_6_1_2:
    *fw_version = CPP_FW_VERSION_1_12_0;
    break;
  case CPP_HW_VERSION_6_1_4:
    *fw_version = CPP_FW_VERSION_1_12_1;
    break;
  default:
    CPP_ERR("invalid hw version=0x%08x\n", hw_version);
    *fw_version = CPP_FW_VERSION_MAX;
    return -EINVAL;
  }
  snprintf(fw_filename, CPP_FW_FILENAME_MAX,
    "cpp_firmware_v%d_%d_%d.fw",
    CPP_GET_FW_MAJOR_VERSION(*fw_version),
    CPP_GET_FW_MINOR_VERSION(*fw_version),
    CPP_GET_FW_STEP_VERSION(*fw_version));
  return 0;
}

/* cpp_hardware_create:
 *
 *  creates new cpp_hardware instance. Finds the cpp subdev in kernel,
 *  allocates memory and initializes the structure.
 *
 **/
cpp_hardware_t* cpp_hardware_create()
{
  cpp_hardware_t *cpphw;
  char  value[PROPERTY_VALUE_MAX];
  int32_t rc;
  cpphw = (cpp_hardware_t *) CAM_MALLOC(sizeof(cpp_hardware_t));
  if(!cpphw) {
    CPP_ERR("CAM_MALLOC() failed");
    return NULL;
  }
  memset(cpphw, 0x00, sizeof(cpp_hardware_t));
  rc = cpp_hardware_find_subdev(cpphw);
  if (rc < 0) {
    CPP_ERR("cannot find cpp subdev");
    free(cpphw);
    return NULL;
  }
  cpphw->subdev_opened = FALSE;
  cpphw->status = CPP_HW_STATUS_INVALID;
  cpphw->num_iommu_cnt = 0;

  /* initialize the stream_status */
  uint32_t i;
  for (i=0; i<CPP_HARDWARE_MAX_STREAMS; i++) {
    cpphw->stream_status[i].valid = FALSE;
    cpphw->stream_status[i].identity = 0x00;
    cpphw->stream_status[i].pending_buf = 0;
    cpphw->stream_status[i].pending_partial_frame = 0;
    cpphw->stream_status[i].stream_off_pending = FALSE;
  }
  pthread_cond_init(&(cpphw->subdev_cond), NULL);
  pthread_mutex_init(&(cpphw->mutex), NULL);
  property_get("persist.vendor.enable.max.pending.buf", value, "1");
  if (atoi(value)) {
    cpphw->max_pending_buffer = CPP_HARDWARE_MAX_PENDING_BUF;
  } else {
    cpphw->max_pending_buffer = CPP_HARDWARE_DEFAULT_PENDING_BUF;

  }
  return cpphw;
}

/* cpp_hardware_open_subdev:
 *
 *  opens the cpp subdev and updates instance info.
 *  updates the hardware status.
 *  currently only one cpp subdevice is supported.
 **/
int32_t cpp_hardware_open_subdev(cpp_hardware_t *cpphw)
{
  int fd;
  int32_t rc=0;
  char dev_name[SUBDEV_NAME_SIZE_MAX];
  if(!cpphw) {
    CPP_ERR("invalid cpp hw, failed");
    return -EINVAL;
  }
  /* make sure all code-paths unlock this mutex */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  if (cpphw->subdev_opened == TRUE) {
    CPP_HIGH("subdev already open");
    rc = -EFAULT;
    goto error_mutex;
  }
  snprintf(dev_name, sizeof(dev_name), "/dev/v4l-subdev%d",
    cpphw->subdev_ids[0]);
  fd = open(dev_name, O_RDWR | O_NONBLOCK);
  if (fd < 0) {
    CPP_ERR("cannot open cpp subdev: %s", dev_name);
    rc = -EIO;
    goto error_mutex;
  }
  cpphw->subdev_fd = fd;
  cpphw->subdev_opened = TRUE;
  pthread_cond_broadcast(&(cpphw->subdev_cond));
  /* get the instance info */
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct msm_cpp_frame_info_t inst_info;
  memset(&inst_info, 0x00, sizeof(struct msm_cpp_frame_info_t));
  v4l2_ioctl.ioctl_ptr = (void *)&inst_info;
  v4l2_ioctl.len = sizeof(inst_info);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_GET_INST_INFO, &v4l2_ioctl);
  if (rc < 0) {
    CPP_ERR("v4l2 ioctl() failed, rc=%d %d", rc, errno);
    rc = -EIO;
    goto error_open;
  }
  cpphw->inst_id = inst_info.inst_id;
  /* update hw state */
  if (cpphw->status == CPP_HW_STATUS_FW_LOADED) {
    cpphw->status = CPP_HW_STATUS_READY;
  } else {
    cpphw->status = CPP_HW_STATUS_INVALID;
  }
  cpphw->num_iommu_cnt = 0;
  cpphw->dump_preview_cnt = cpphw->dump_snapshot_cnt =
    cpphw->dump_video_cnt = 0;
  CPP_DBG("cpp subdev opened, subdev_fd=%d, inst_id=0x%x, status=%d",
    cpphw->subdev_fd, cpphw->inst_id, cpphw->status);

  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;

error_open:
  cpphw->subdev_fd = -1;
  cpphw->subdev_opened = FALSE;
  close(fd);
error_mutex:
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return rc;
}

/* cpp_hardware_close_subdev:
 *
 *  close the cpp subdev and update hardware status accordingly
 *
 **/
int32_t cpp_hardware_close_subdev(cpp_hardware_t *cpphw)
{
  if(!cpphw) {
    CPP_ERR("invalid cpp hw, failed");
    return -EINVAL;
  }
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  close(cpphw->subdev_fd);
  cpphw->subdev_opened = FALSE;
  /* if status is ready or busy, fw would be loaded */
  if (cpphw->status != CPP_HW_STATUS_INVALID) {
    cpphw->status = CPP_HW_STATUS_FW_LOADED;
  } else {
    cpphw->status = CPP_HW_STATUS_INVALID;
  }
  cpphw->num_iommu_cnt = 0;
  cpphw->dump_preview_cnt = cpphw->dump_snapshot_cnt =
    cpphw->dump_video_cnt = 0;
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

/* cpp_hardware_destroy:
 *
 *  destroy the hardware data structure and free memory
 *
 **/
int32_t cpp_hardware_destroy(cpp_hardware_t *cpphw)
{
  if(!cpphw) {
    CPP_ERR("invalid cpp hw, failed");
    return -EINVAL;
  }
  if (cpphw->subdev_opened == TRUE) {
    cpp_hardware_close_subdev(cpphw);
  }
  pthread_cond_destroy(&(cpphw->subdev_cond));
  pthread_mutex_destroy(&(cpphw->mutex));
  free(cpphw);
  return 0;
}

/* cpp_hardware_get_status:
 *
 *  get current status of the hardware. Hardware structure access is
 *  protected by the mutex
 *
 **/
cpp_hardware_status_t cpp_hardware_get_status(cpp_hardware_t *cpphw)
{
  cpp_hardware_status_t status;
  if (!cpphw) {
    CPP_ERR("invalid cpp hw, failed");
    return FALSE;
  }
  int32_t num_pending = 0;
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  uint32_t i;
  for (i=0; i<CPP_HARDWARE_MAX_STREAMS; i++) {
    if (cpphw->stream_status[i].valid) {
      num_pending += cpphw->stream_status[i].pending_buf;
    }
  }
  if (num_pending == CPP_HARDWARE_NO_PENDING_BUF) {
    status = CPP_HW_STATUS_IDLE;
  } else if (num_pending < cpphw->max_pending_buffer) {
    status = CPP_HW_STATUS_READY;
  } else {
    status = CPP_HW_STATUS_BUSY;
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return status;
}

/* cpp_hardware_send_buf_done:
 *
 *  Send buffer done to the processed divert buffer. All accesses to the
 *  shared hardware data structure are protected by mutex.
 *
 **/
static int32_t cpp_hardware_send_buf_done(cpp_hardware_t *cpphw,
  cpp_hardware_event_data_t *event_data)
{
  int32_t rc;
  struct msm_pproc_queue_buf_info queue_buf_info;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;

  memset(&queue_buf_info, 0, sizeof(struct msm_pproc_queue_buf_info));
  queue_buf_info.buff_mgr_info.session_id =
    ((event_data->identity >> 16) & 0xFFFF);
  queue_buf_info.buff_mgr_info.stream_id = (event_data->identity & 0xFFFF);
  queue_buf_info.buff_mgr_info.frame_id = event_data->frame_id;
  queue_buf_info.buff_mgr_info.timestamp = event_data->timestamp;
  queue_buf_info.buff_mgr_info.index = event_data->out_buf_idx;
#ifdef CACHE_PHASE2
  queue_buf_info.buff_mgr_info.flags = event_data->output_buffer_access;
#else
  // make sure to do flush, invalidate based on flags
#endif
  queue_buf_info.is_buf_dirty = event_data->is_buf_dirty;

  CPP_BUF_DBG("identity %x,frameid %d", event_data->identity, event_data->frame_id);

  v4l2_ioctl.ioctl_ptr = &queue_buf_info;
  v4l2_ioctl.len = sizeof(struct msm_pproc_queue_buf_info);
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_QUEUE_BUF, &v4l2_ioctl);
  if (rc < 0) {
    CPP_BUF_ERR("v4l2 ioctl() failed, rc=%d", rc);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EIO;
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

/* cpp_hardware_set_clock:
 *
 **/
static int32_t cpp_hardware_set_clock(cpp_hardware_t *cpphw,
  cpp_hardware_clock_settings_t *clock_settings)
{
  int32_t rc=0;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct msm_cpp_frame_info_t inst_info;
  char dev_name[SUBDEV_NAME_SIZE_MAX];
  struct msm_cpp_clock_settings_t hw_clock_settings;
  if(!cpphw) {
    CPP_ERR("failed invalid cpphw\n");
    return -EINVAL;
  }

  ATRACE_BEGIN_SNPRINTF(40, "cpp_hardware_set_clock:%ld",
    clock_settings->clock_rate);
  /* make sure all code-paths unlock this mutex */
  hw_clock_settings.clock_rate = clock_settings->clock_rate;
  hw_clock_settings.avg = clock_settings->avg;
  hw_clock_settings.inst = clock_settings->inst;

  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  v4l2_ioctl.ioctl_ptr = (void *)&hw_clock_settings;
  v4l2_ioctl.len = sizeof(struct msm_cpp_clock_settings_t);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_SET_CLOCK, &v4l2_ioctl);
  if (rc < 0) {
    CPP_ERR("v4l2 ioctl() failed, rc=%d\n", rc);
    rc = -EIO;
  }
  ATRACE_END();

  CPP_INFO("Set clock %ld BW avg %lld BW inst %lld",
    clock_settings->clock_rate, clock_settings->avg,
    clock_settings->inst);

error_mutex:
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return rc;
}


/* cpp_hardware_process_command:
 *
 *  processes the command given to the hardware. Hardware state is
 *  updated during the process. All accesses to the shared hardware
 *  data structure are protected by mutex.
 *
 **/
int32_t cpp_hardware_process_command(cpp_hardware_t *cpphw,
  cpp_hardware_cmd_t cmd)
{
  int32_t rc = 0;
  struct timespec timeout;

  if (!cpphw) {
    CPP_ERR("invalid cpp hw, failed");
    return -EINVAL;
  }
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  while(cpphw->subdev_opened == FALSE) {
    memset(&timeout, 0, sizeof(timeout));
    PTHREAD_COND_WAIT_TIME(&(cpphw->subdev_cond), &(cpphw->mutex),
      &timeout, CPP_WAIT_TIMEOUT, rc);
    if (rc == ETIMEDOUT || rc == EINVAL) {
      CPP_ERR("failed, subdev not opened!!!  errno= %d \
        (ETIMEOUT %d, INVALID %d)\n", rc, ETIMEDOUT, EINVAL);
      PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
      return -EFAULT;
    }
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));

  switch (cmd.type) {
  case CPP_HW_CMD_GET_CAPABILITIES:
    rc = cpp_hardware_get_capabilities(cpphw);
    break;
  case CPP_HW_CMD_SUBSCRIBE_EVENT:
    rc = cpp_hardware_subcribe_v4l2_event(cpphw);
    break;
  case CPP_HW_CMD_UNSUBSCRIBE_EVENT:
    rc = cpp_hardware_unsubcribe_v4l2_event(cpphw);
    break;
  case CPP_HW_CMD_NOTIFY_EVENT:
    rc = cpp_hardware_notify_event_get_data(cpphw, cmd.u.event_data);
    break;
  case CPP_HW_CMD_STREAMON:
    rc = cpp_hardware_process_streamon(cpphw, cmd.u.buff_update);
    break;
  case CPP_HW_CMD_STREAMOFF:
    rc = cpp_hardware_process_streamoff(cpphw, cmd.u.streamoff_data);
    break;
  case CPP_HW_CMD_LOAD_FIRMWARE:
    rc = cpp_hardware_load_firmware(cpphw);
    break;
  case CPP_HW_CMD_PROCESS_FRAME:
    rc = cpp_hardware_process_frame(cpphw, &cmd);
    break;
  case CPP_HW_CMD_PROCESS_PARTIAL_FRAME:
    rc = cpp_hardware_process_partial_frame(cpphw, cmd.u.partial_frame);
    break;
  case CPP_HW_CMD_QUEUE_BUF:
    rc = cpp_hardware_send_buf_done(cpphw, cmd.u.event_data);
    break;
  case CPP_HW_CMD_SET_CLK:
    rc = cpp_hardware_set_clock(cpphw, &cmd.u.clock_settings);
    break;
  case CPP_HW_CMD_BUF_UPDATE:
    rc = cpp_hardware_update_buffer_list(cpphw, cmd.u.buff_update);
    break;
  case CPP_HW_CMD_POP_STREAM_BUFFER:
    rc = cpp_hardware_pop_stream_buffer(cpphw, cmd.u.event_data);
    break;
  case CPP_HW_CMD_NOTIFY_BUF_DONE:
    rc = cpp_hardware_notify_buf_done(cpphw, cmd.u.buf_done_identity);
    break;
  case CPP_HW_CMD_UPDATE_PENDING_BUF:
    rc = cpp_hardware_update_pending_buffer(cpphw, cmd.u.status);
    break;
  default:
    CPP_ERR("bad command type=%d", cmd.type);
    rc = -EINVAL;
  }
  return rc;
}

/* cpp_hardware_get_capabilities:
 *
 * Description:
 *  Get hardware capabilities from kernel
 *
 **/
int32_t cpp_hardware_get_capabilities(cpp_hardware_t *cpphw)
{
  /* note: make sure to unlock this on each return path */
  char cpp_prop[PROPERTY_VALUE_MAX];
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  /* TODO: read from hw reg */
  cpphw->caps.caps_mask = CPP_CAPS_DENOISE | CPP_CAPS_SCALE |
                          CPP_CAPS_SHARPENING | CPP_CAPS_CROP |
                          CPP_CAPS_ROTATION | CPP_CAPS_FLIP;

  cpphw->caps.scaling_caps.max_scale_factor = 8.0;
  cpphw->caps.scaling_caps.min_scale_factor = 1.0/16.0;
  cpphw->caps.rotation_caps = ROTATION_90 | ROTATION_180 | ROTATION_270;
  cpphw->caps.filp_caps.h_flip = TRUE;
  cpphw->caps.filp_caps.v_flip = TRUE;
  cpphw->caps.sharpness_caps.max_value = CPP_MAX_SHARPNESS;
  cpphw->caps.sharpness_caps.min_value = CPP_MIN_SHARPNESS;
  cpphw->caps.sharpness_caps.def_value = CPP_DEFAULT_SHARPNESS;
  cpphw->caps.sharpness_caps.step = (CPP_MAX_SHARPNESS - CPP_MIN_SHARPNESS) /
                                      CPP_TOTAL_SHARPNESS_LEVELS;

  memset(cpp_prop, 0, sizeof(cpp_prop));
  property_get("persist.vendor.camera.llnoise", cpp_prop, "0");
  if (atoi(cpp_prop))
    cpphw->max_supported_padding = MAX_SUPPORTED_RIM_PADDING;
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

double cpp_get_sharpness_ratio(int32_t sharpness)
{
  return (double)(sharpness) / CPP_DEFAULT_SHARPNESS;
}

/* cpp_hardware_load_firmware:
 * Description:
 *   Calls kernel ioctl to load the firmware for CPP micro controller. Updates
 *   the states
 *
 **/
static int32_t cpp_hardware_load_firmware(cpp_hardware_t *cpphw)
{
  int32_t rc;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct cpp_hw_info cpp_hw_info;
  CPP_DBG("loading firmware");

  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));

  /* firmware should be loaded only once when state is invalid */
  if (cpphw->status != CPP_HW_STATUS_INVALID) {
    CPP_ERR("bad hw status=%d", cpphw->status);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EINVAL;
  }

  v4l2_ioctl.ioctl_ptr = (void *)&(cpp_hw_info);
  v4l2_ioctl.len = sizeof(struct cpp_hw_info);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_GET_HW_INFO,
    &v4l2_ioctl);
  if (rc < 0) {
    CPP_ERR("v4l2 ioctl() failed, rc=%d", rc);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EIO;
  }

  cpphw->hwinfo.version = cpp_hw_info.cpp_hw_version;
  cpphw->hwinfo.caps =  cpp_hw_info.cpp_hw_caps;
  cpphw->hwinfo.freq_tbl_count = cpp_hw_info.freq_tbl_count;
  memcpy(&cpphw->hwinfo.freq_tbl, cpp_hw_info.freq_tbl,
    MAX_FREQ_TBL * sizeof(long));

  /* Get corresponding fw version based on hw version */
  char fw_filename[CPP_FW_FILENAME_MAX];
  rc = cpp_hardware_select_fw_version(cpphw->hwinfo.version,
    &cpphw->fw_version, fw_filename);
  CPP_HIGH("hw_version=0x%08x, fw_version=0x%08x",
    cpphw->hwinfo.version, cpphw->fw_version);
  if (rc < 0) {
    CPP_ERR("select fw version, failed rc %d", rc);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EFAULT;
  }
  v4l2_ioctl.ioctl_ptr = (void *)fw_filename;
  v4l2_ioctl.len = strlen(fw_filename);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_LOAD_FIRMWARE,
    &v4l2_ioctl);
  if (rc < 0) {
    CPP_ERR("v4l2 ioctl() failed, rc=%d", rc);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EIO;
  }
  cpphw->status = CPP_HW_STATUS_FW_LOADED;
  CPP_HIGH("cpp firmware loaded: %s", fw_filename);
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}


/* cpp_hardware_update_buffer_list
 *
 *  @cpphw - structure holding hardware parameters
 *  @hw_strm_buff_info - structure holding information for the buufers that
 *             have to be appended to the buffer queue
 *
 *  Send all the information for the buffers that have to be added to the
 *  stream queue to the kernel.
 *
 *  Returns 0 on success
 *
 **/
int32_t cpp_hardware_update_buffer_list(cpp_hardware_t *cpphw,
  cpp_hardware_buff_update_t *buff_update)
{
  int32_t rc;
  uint32_t i;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct msm_cpp_stream_buff_info_t cpp_strm_buff_info;
  cpp_hardware_stream_buff_info_t *hw_strm_buff_info;

  if (NULL == buff_update) {
    CPP_BUF_ERR("error buff_update:%p\n", buff_update);
    return -EINVAL;
  }

  hw_strm_buff_info = buff_update->stream_buff_list;

  /* Translate to msm stream buff list */
   cpp_strm_buff_info.identity = hw_strm_buff_info->identity;
   cpp_strm_buff_info.num_buffs = hw_strm_buff_info->num_buffs;
   cpp_strm_buff_info.buffer_info =
     CAM_MALLOC(sizeof(struct msm_cpp_buffer_info_t) *
     hw_strm_buff_info->num_buffs);
   if (NULL == cpp_strm_buff_info.buffer_info) {
     CPP_BUF_ERR("error allocating buffer info\n");
     return -ENOMEM;
   }

   for (i = 0; i < cpp_strm_buff_info.num_buffs; i++) {
     cpp_strm_buff_info.buffer_info[i].fd =
       hw_strm_buff_info->buffer_info[i].fd;
     cpp_strm_buff_info.buffer_info[i].index =
       hw_strm_buff_info->buffer_info[i].index;
     cpp_strm_buff_info.buffer_info[i].native_buff =
       hw_strm_buff_info->buffer_info[i].native_buff;
     cpp_strm_buff_info.buffer_info[i].offset =
       hw_strm_buff_info->buffer_info[i].offset;
     cpp_strm_buff_info.buffer_info[i].processed_divert =
       hw_strm_buff_info->buffer_info[i].processed_divert;
     cpp_strm_buff_info.buffer_info[i].identity =
       hw_strm_buff_info->buffer_info[i].identity;
   }

   /* note: make sure to unlock this on each return path */
   PTHREAD_MUTEX_LOCK(&(cpphw->mutex));

   v4l2_ioctl.len = sizeof(cpp_strm_buff_info);
   v4l2_ioctl.ioctl_ptr = (void *)&cpp_strm_buff_info;

   if (buff_update->delete_buf == TRUE) {
     rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_DELETE_STREAM_BUFF,
       &v4l2_ioctl);
   } else {
     rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_APPEND_STREAM_BUFF_INFO,
       &v4l2_ioctl);
   }
   free(cpp_strm_buff_info.buffer_info);
   if (rc < 0) {
     CPP_BUF_ERR("v4l2 ioctl() failed, rc=%d\n", rc);
     PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
     return -EIO;
   }
   PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));

   return 0;
}


/* cpp_hardware_process_streamon:
 *
 **/
int32_t cpp_hardware_process_streamon(cpp_hardware_t *cpphw,
  cpp_hardware_buff_update_t *buff_update)
{
  int32_t rc;
  uint32_t i;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl, v4l2_ioctl_iommu;
  struct msm_cpp_stream_buff_info_t cpp_strm_buff_info;
  struct msm_camera_smmu_attach_type cpp_attach_info;
  cpp_hardware_stream_buff_info_t *hw_strm_buff_info;

  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  if (cpphw->num_iommu_cnt == 0) {
    if (buff_update->is_secure == SECURE) {
#ifdef SECURE_SLAVE_MODE
        if(buff_update->secure_mode == SECURE_SLAVE)
            cpp_attach_info.attach = SECURE_SLAVE_MODE;
        else
#endif
            cpp_attach_info.attach = SECURE_MODE;
    } else {
      cpp_attach_info.attach = NON_SECURE_MODE;
    }
    v4l2_ioctl_iommu.len = sizeof(cpp_attach_info);
    v4l2_ioctl_iommu.ioctl_ptr = (void*)&cpp_attach_info;
    rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_IOMMU_ATTACH,
               &v4l2_ioctl_iommu);
    if (rc < 0) {
      CPP_ERR("IOMMMU Attach v4l2 ioctl() failed, rc=%d\n", rc);
      PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
      return -EIO;
    }
    cpphw->num_iommu_cnt++;
  } else
    cpphw->num_iommu_cnt++;

  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));

  if (NULL == buff_update) {
    CPP_ERR("buff_update:%p\n", buff_update);
    return -EINVAL;
  }

  hw_strm_buff_info = buff_update->stream_buff_list;

  /* Translate to msm stream buff list */
  cpp_strm_buff_info.identity = hw_strm_buff_info->identity;
  cpp_strm_buff_info.num_buffs = hw_strm_buff_info->num_buffs;
  cpp_strm_buff_info.buffer_info =
    CAM_MALLOC(sizeof(struct msm_cpp_buffer_info_t) *
    hw_strm_buff_info->num_buffs);
  if (NULL == cpp_strm_buff_info.buffer_info) {
    CPP_ERR("CAM_MALLOC() failed");
    return -ENOMEM;
  }

  for (i = 0; i < cpp_strm_buff_info.num_buffs; i++) {
    cpp_strm_buff_info.buffer_info[i].fd =
      hw_strm_buff_info->buffer_info[i].fd;
    cpp_strm_buff_info.buffer_info[i].index =
      hw_strm_buff_info->buffer_info[i].index;
    cpp_strm_buff_info.buffer_info[i].native_buff =
      hw_strm_buff_info->buffer_info[i].native_buff;
    cpp_strm_buff_info.buffer_info[i].offset =
      hw_strm_buff_info->buffer_info[i].offset;
    cpp_strm_buff_info.buffer_info[i].processed_divert =
      hw_strm_buff_info->buffer_info[i].processed_divert;
    cpp_strm_buff_info.buffer_info[i].identity =
      hw_strm_buff_info->buffer_info[i].identity;
  }

  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));

  v4l2_ioctl.len = sizeof(cpp_strm_buff_info);
  v4l2_ioctl.ioctl_ptr = (void *)&cpp_strm_buff_info;
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_ENQUEUE_STREAM_BUFF_INFO,
    &v4l2_ioctl);
  free(cpp_strm_buff_info.buffer_info);
  if (rc < 0) {
    CPP_ERR("v4l2 ioctl() failed, rc=%d", rc);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EIO;
  }

  /* make stream_status valid for this stream */
  for (i=0; i<CPP_HARDWARE_MAX_STREAMS; i++) {
    if (cpphw->stream_status[i].valid == FALSE) {
      cpphw->stream_status[i].identity = hw_strm_buff_info->identity;
      cpphw->stream_status[i].pending_buf = 0;
      cpphw->stream_status[i].valid = TRUE;
      cpphw->stream_status[i].stream_off_pending = FALSE;
      cpphw->stream_status[i].pending_divert = 0;
      cpphw->stream_status[i].pending_partial_frame = 0;
      CPP_HIGH("streamon identity = %x", cpphw->stream_status[i].identity);
      break;
    }
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

/* cpp_hardware_streamoff_waitcheck:
 *
 **/
static inline bool cpp_hardware_streamoff_waitcheck(
  cpp_hardware_stream_status_t *stream_status)
{
  CPP_HIGH("waiting for pending buf, identity=0x%x, pending_buf=%d, "
    "pending_divert=%d, partial_frame=%d, stream_off_op=%d, "
    "pernding_buf_divert %d\n",
    stream_status->identity, stream_status->pending_buf,
    stream_status->pending_divert, stream_status->pending_partial_frame,
    stream_status->hold_stream_off_op, stream_status->pending_buf_divert);
  return ((stream_status->pending_buf != 0) ||
    (stream_status->pending_divert != 0) ||
    (stream_status->pending_partial_frame != 0) ||
    (stream_status->hold_stream_off_op) ||
    (stream_status->pending_buf_divert != 0));
}

/* cpp_hardware_process_streamoff:
 *
 **/
int32_t cpp_hardware_process_streamoff(cpp_hardware_t *cpphw,
  cpp_hardware_streamoff_event_t streamoff_data)
{
  int32_t rc = 0;
  int32_t err_status = 0;
  uint32_t i;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl, v4l2_ioctl_iommu;
  struct msm_camera_smmu_attach_type cpp_attach_info;
  cpp_hardware_stream_status_t *stream_status = NULL;
  cpp_hardware_stream_status_t *duplicate_stream_status = NULL;
  bool is_hw_err = false;
  struct timespec timeout;


  CPP_DBG("identity=0x%x", streamoff_data.streamoff_identity);

  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  is_hw_err = streamoff_data.is_hw_err;
  stream_status = cpp_hardware_get_stream_status(cpphw,
    streamoff_data.streamoff_identity);
  if (!stream_status) {
    CPP_ERR("failed, stream off identity %x",
      streamoff_data.streamoff_identity);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EFAULT;
  }

  CPP_DBG("is_hw_err %d", is_hw_err);

  stream_status->stream_off_pending = TRUE;
  /* wait for all pending buffers to be processed */
  while (!is_hw_err && cpp_hardware_streamoff_waitcheck(stream_status)) {
    CPP_BUF_DBG("waiting for pending buf, identity=0x%x, pending_buf=%d, "
      "pending_divert=%d, partial_frame=%d, stream_off_op=%d, "
      "pernding_buf_divert %d\n",
      streamoff_data.streamoff_identity, stream_status->pending_buf,
      stream_status->pending_divert, stream_status->pending_partial_frame,
      stream_status->hold_stream_off_op, stream_status->pending_buf_divert);
    memset(&timeout, 0, sizeof(timeout));
    PTHREAD_COND_WAIT_TIME(&(cpphw->subdev_cond),
      &(cpphw->mutex), &timeout, CPP_WAIT_TIMEOUT, rc);
    if (rc == ETIMEDOUT || rc == EINVAL) {
      CPP_INFO("failed! streamoff time out! errno= %d\n", rc);
      rc = -EFAULT;
      stream_status->stream_off_pending = FALSE;
      stream_status->valid = FALSE;
      goto timeout_err;
    }
  }

  stream_status->stream_off_pending = FALSE;
  stream_status->valid = FALSE;
  stream_status->pending_divert = 0;
  stream_status->pending_buf = 0;
  stream_status->pending_partial_frame = 0;

  for (i = 0; i < streamoff_data.num_linked_streams; i++) {
    duplicate_stream_status = cpp_hardware_get_stream_status(cpphw,
      streamoff_data.linked_identity[i]);

    CPP_HIGH("skip_iden:0x%x, duplicate_stream_status:%p",
      streamoff_data.linked_identity[i], duplicate_stream_status);

    if (duplicate_stream_status) {
      duplicate_stream_status->stream_off_pending = TRUE;
      /* wait for all pending buffers to be processed */
      while (!is_hw_err && cpp_hardware_streamoff_waitcheck(
        duplicate_stream_status)) {
        CPP_BUF_DBG("waiting for pending buf, identity=0x%x,"
          "pending_buf=%d, pending_divert:%d, partial:%d",
           "wait_identity:0x%x,stream_off_op=%d\n",
          streamoff_data.streamoff_identity,
          duplicate_stream_status->pending_buf,
          duplicate_stream_status->pending_divert,
          duplicate_stream_status->pending_partial_frame,
          duplicate_stream_status->identity,
          duplicate_stream_status->hold_stream_off_op);
        memset(&timeout, 0, sizeof(timeout));
        PTHREAD_COND_WAIT_TIME(&(cpphw->subdev_cond), &(cpphw->mutex),
          &timeout, CPP_WAIT_TIMEOUT, rc);
        if (rc == ETIMEDOUT || rc == EINVAL) {
          CPP_ERR("streamoff failed!!!  errno= %d \
            (ETIMEOUT %d, INVALID %d)\n", rc, ETIMEDOUT, EINVAL);
          rc = -EFAULT;
          goto timeout_err;
        }
      }
      duplicate_stream_status->stream_off_pending = FALSE;
    }
  }

timeout_err:
  if (rc < 0) {
    CPP_INFO("Streamoff timeout, dequeue buffers %d", rc);
    err_status = rc;
  } else {
   CPP_BUF_DBG("pending buffers done, hw streaming off. identity=0x%x",
    streamoff_data.streamoff_identity);
  }

  v4l2_ioctl.ioctl_ptr = (void *)(&streamoff_data.streamoff_identity);
  v4l2_ioctl.len = sizeof(uint32_t);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_DEQUEUE_STREAM_BUFF_INFO,
    &v4l2_ioctl);
  if (rc < 0) {
    CPP_ERR("v4l2 ioctl() failed, rc=%d\n", rc);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EIO;
  }
  CPP_DBG("hw stream-off done. identity=0x%x\n",
    streamoff_data.streamoff_identity);

  if (cpphw->num_iommu_cnt > 0) {
    cpphw->num_iommu_cnt--;
  } else {
    CPP_ERR("INVALID IOMMU cnt=%d\n", cpphw->num_iommu_cnt);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EIO;
  }

  if (cpphw->num_iommu_cnt == 0) {
    if (streamoff_data.is_secure == SECURE) {
      cpp_attach_info.attach = SECURE_MODE;
    } else {
      cpp_attach_info.attach = NON_SECURE_MODE;
    }
    v4l2_ioctl_iommu.ioctl_ptr = (void*)&cpp_attach_info;
    v4l2_ioctl_iommu.len = sizeof(cpp_attach_info);
    rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_IOMMU_DETACH,
               &v4l2_ioctl_iommu);
    if (rc < 0) {
      CPP_ERR("IOMMMU detach v4l2 ioctl() failed, rc=%d\n", rc);
      PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
      return -EIO;
    }
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));

  return err_status;
}

/* cpp_hardware_subcribe_v4l2_event:
 *
 **/
static int32_t cpp_hardware_subcribe_v4l2_event(cpp_hardware_t *cpphw)
{
  int32_t rc;
  struct v4l2_event_subscription sub;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;

  sub.id = cpphw->inst_id;
  sub.type = V4L2_EVENT_CPP_FRAME_DONE;
  rc = ioctl(cpphw->subdev_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
  if (rc < 0) {
    CPP_ERR("v4l2 ioctl() failed, rc=%d", rc);
    return -EIO;
  }
  cpphw->event_subs_info.valid = TRUE;
  cpphw->event_subs_info.id = sub.id;
  cpphw->event_subs_info.type = sub.type;
  return 0;
}

/* cpp_hardware_get_stream_status:
 *
 **/
cpp_hardware_stream_status_t*
  cpp_hardware_get_stream_status(cpp_hardware_t* cpphw, uint32_t identity)
{
  uint32_t i;
  for (i=0; i<CPP_HARDWARE_MAX_STREAMS; i++) {
    if (cpphw->stream_status[i].valid == TRUE) {
      if (cpphw->stream_status[i].identity == identity) {
        return &(cpphw->stream_status[i]);
      }
    }
  }
  return NULL;
}

/* cpp_hardware_get_event_data:
 *
 **/
static int32_t cpp_hardware_notify_event_get_data(cpp_hardware_t *cpphw,
  cpp_hardware_event_data_t *event_data)
{
  int32_t rc;
  struct msm_cpp_frame_info_t frame;
  cpp_module_hw_cookie_t *cookie;

  if(!event_data) {
    CPP_ERR("invalid event data, failed");
    return -EINVAL;
  }
  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  memset(&frame, 0x0, sizeof(struct msm_cpp_frame_info_t));
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct v4l2_event event;
  rc = ioctl(cpphw->subdev_fd, VIDIOC_DQEVENT, &event);
  if (rc < 0) {
    CPP_ERR("v4l2 ioctl() failed, rc=%d", rc);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EIO;
  }
  /* TODO: use event type to distinguish */
  v4l2_ioctl.ioctl_ptr = (void *)&frame;
  v4l2_ioctl.len = sizeof(struct msm_cpp_frame_info_t);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_GET_EVENTPAYLOAD, &v4l2_ioctl);
  if (rc < 0) {
    CPP_ERR("v4l2 ioctl() failed, rc=%d", rc);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EIO;
  }
  event_data->frame_id = frame.frame_id;
  event_data->buf_idx = frame.input_buffer_info.index;
  event_data->out_fd = frame.output_buffer_info[0].fd;
  event_data->identity = frame.identity;
  event_data->timestamp = frame.timestamp;
  event_data->cookie = frame.cookie;
  event_data->batch_mode = frame.batch_info.batch_mode;
  if (BATCH_MODE_VIDEO == event_data->batch_mode) {
    event_data->out_buf_idx = frame.batch_info.cont_idx;
    event_data->batch_size = frame.batch_info.batch_size;
  } else {
    event_data->out_buf_idx = frame.output_buffer_info[0].index;
    event_data->batch_size = frame.batch_info.batch_size;
  }
  event_data->out_native_buff = frame.output_buffer_info[0].native_buff;
  event_data->dup_out_native_buff = frame.duplicate_buffer_info.native_buff;
  event_data->dup_out_buf_idx = frame.duplicate_buffer_info.index;
  event_data->dup_out_fd = frame.duplicate_buffer_info.fd;
  event_data->dup_identity = frame.duplicate_identity;

  cookie = event_data->cookie;

#ifdef CACHE_PHASE2
  event_data->input_buffer_access = frame.input_buffer_info.buffer_access;
  event_data->output_buffer_access = frame.output_buffer_info[0].buffer_access;
#else
  event_data->input_buffer_access = cookie ? cookie->buffer_access : 0;
  event_data->output_buffer_access = 0;
#endif

  event_data->identity = frame.identity;

  event_data->do_ack = TRUE;
  if (frame.partial_frame_indicator && !frame.last_payload) {
    event_data->do_ack = FALSE;
  }

  CPP_BUF_DBG("frame_id=%d, buf_idx=%d, identity=0x%x",
    event_data->frame_id, event_data->buf_idx,
    event_data->identity);

  CPP_LOW("in_time=%ld.%ldus out_time=%ld.%ldus",
    frame.in_time.tv_sec, frame.in_time.tv_usec,
    frame.out_time.tv_sec, frame.out_time.tv_usec);
  CPP_PROFILE("processing time = %6ld us",
    (frame.out_time.tv_sec - frame.in_time.tv_sec)*1000000L +
    (frame.out_time.tv_usec - frame.in_time.tv_usec));

  /* update hardware stream_status */
  cpp_hardware_stream_status_t *stream_status =
    cpp_hardware_get_stream_status(cpphw, event_data->identity);
  if (!stream_status) {
    CPP_ERR("stream status null, failed");
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EFAULT;
  }

  stream_status->pending_buf--;
  if (frame.partial_frame_indicator && frame.last_payload) {
    stream_status->pending_partial_frame--;
  }

  stream_status->hold_stream_off_op = true;
   CPP_BUF_DBG("stream_status->hold_stream_off_op is %d\n",stream_status->hold_stream_off_op);
  if (cookie && cookie->proc_div_required &&
    cookie->proc_div_identity != stream_status->identity) {
    stream_status =
        cpp_hardware_get_stream_status(cpphw, cookie->proc_div_identity);
      if (!stream_status) {
        CPP_ERR("stream status null, failed");
        PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
        return -EFAULT;
      }
  }
  if ((frame.frame_type != MSM_CPP_OFFLINE_FRAME) && cookie &&
    cookie->proc_div_required && ((!frame.partial_frame_indicator) ||
    frame.last_payload)) {
    stream_status->pending_divert++;
    CPP_DBG(" ++ pending_divert %d for iden 0x%x",
     stream_status->pending_divert, stream_status->identity);
  }

  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

int32_t cpp_hardware_notify_buf_done(cpp_hardware_t *cpphw, uint32_t identity)
{
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  /* update hardware stream_status */
  cpp_hardware_stream_status_t *stream_status =
    cpp_hardware_get_stream_status(cpphw, identity);

  if (stream_status) {
    if (stream_status->pending_divert)
      stream_status->pending_divert--;

    CPP_DBG(" -- pending_divert %d for iden 0x%x",
     stream_status->pending_divert, stream_status->identity);
    /* send signal to thread which is waiting on stream_off
       for pending buffer to be zero */
    if ((stream_status->stream_off_pending == TRUE) &&
      (stream_status->pending_buf == 0) &&
      (stream_status->pending_divert == 0) &&
      (stream_status->pending_partial_frame == 0) &&
      (stream_status->pending_buf_divert == 0)) {
      CPP_BUF_DBG("pp_dbg sending broadcast for pending stream-off");
      pthread_cond_broadcast(&(cpphw->subdev_cond));
    }
    stream_status->hold_stream_off_op = false;
  } else {
    CPP_WARN("stream_status is NULL, iden:0x%x", identity);
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

/* cpp_hardware_create_hw_frame:
 *
 **/
static struct msm_cpp_frame_info_t*
  cpp_hardware_create_hw_frame(cpp_hardware_t *cpphw,
  struct cpp_frame_info_t *cpp_frame_info)
{
  int32_t rc;
  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_CPP_CREATE_HW_FRAME);
  /* note: make sure to unlock this on each return path */
  struct msm_cpp_frame_info_t *msm_cpp_frame_info =
    CAM_MALLOC(sizeof(struct msm_cpp_frame_info_t));
  if (!msm_cpp_frame_info) {
    CPP_ERR("CAM_MALLOC failed");
    ATRACE_CAMSCOPE_END(CAMSCOPE_CPP_CREATE_HW_FRAME);
    return NULL;
  }
  memset(msm_cpp_frame_info, 0, sizeof(struct msm_cpp_frame_info_t));
  /* create frame msg */
  msm_cpp_frame_info->client_id = 0;
  msm_cpp_frame_info->inst_id = cpphw->inst_id;
  cpp_params_prepare_frame_info(cpphw, cpp_frame_info,
    msm_cpp_frame_info);
  ATRACE_CAMSCOPE_END(CAMSCOPE_CPP_CREATE_HW_FRAME);
  return msm_cpp_frame_info;
}

/* cpp_hardware_destroy_hw_frame:
 *
 **/
static void cpp_hardware_destroy_hw_frame(
  struct msm_cpp_frame_info_t* msm_cpp_frame_info)
{
  if (!msm_cpp_frame_info) {
    CPP_HIGH("msm_cpp_frame_info=NULL");
    return;
  }
  CPP_LOW("Enter");
  free(msm_cpp_frame_info->cpp_cmd_msg);
  free(msm_cpp_frame_info);
}

/* cpp_hardware_fill_bus_message_params:
 *
 **/
static int32_t cpp_hardware_dump_metadata(cpp_hardware_params_t *hw_params,
  struct msm_cpp_frame_info_t *msm_cpp_frame_info,
  struct cpp_frame_info_t *cpp_frame_info, cpp_firmware_version_t fw_version)
{
  uint32_t                i = 0;
  int32_t                 enabled = 0;
  char                    value[PROPERTY_VALUE_MAX];
  int32_t                 meta_frame_count = 0;
  pproc_meta_data_t      *meta_data;
  cpp_info_t             *cpp_meta_info;
  fe_config_t            *fe_config;
  cpp_module_hw_cookie_t *cookie;
  meta_data_container    *md_container;

  if (!hw_params || !msm_cpp_frame_info) {
    CPP_ERR("invalid params hw_params %p frame info %p\n",
      hw_params, msm_cpp_frame_info);
    return -EINVAL;
  }

  property_get("persist.vendor.camera.dumpmetadata", value, "0");
  enabled = atoi(value);
  if (enabled == 0) {
    return 0;
  }

  cookie = hw_params->cookie;
  if (!cookie || !cookie->meta_datas) {
    return 0;
  }

  md_container = (meta_data_container *)cookie->meta_datas;
  if (!md_container->pproc_meta_data) {
    return 0;
  }

  meta_data = (pproc_meta_data_t *)md_container->pproc_meta_data;
  meta_data->entry[PPROC_META_DATA_CPP_IDX].dump_type =
    PPROC_META_DATA_INVALID;
  meta_data->entry[PPROC_META_DATA_FE_IDX].dump_type =
    PPROC_META_DATA_INVALID;
  meta_data->entry[PPROC_META_DATA_CPP_IDX].pproc_meta_dump = NULL;
  meta_data->entry[PPROC_META_DATA_FE_IDX].pproc_meta_dump = NULL;

  cpp_meta_info = (cpp_info_t *)CAM_MALLOC(sizeof(cpp_info_t));
  if (!cpp_meta_info) {
    CPP_ERR("CAM_MALLOC failed\n");
    meta_data->entry[PPROC_META_DATA_CPP_IDX].pproc_meta_dump = NULL;
    return 0;
  }
  memset(cpp_meta_info, 0, sizeof(cpp_info_t));
  fe_config = (fe_config_t *)CAM_MALLOC(sizeof(fe_config_t));
  if (!fe_config) {
    CPP_ERR("CAM_MALLOC failed\n");
    free(cpp_meta_info);
    meta_data->entry[PPROC_META_DATA_CPP_IDX].pproc_meta_dump = NULL;
    return 0;
  }
  memset(fe_config, 0, sizeof(fe_config_t));
  fe_config->luma_x = cpp_frame_info->plane_info[0].h_scale_initial_phase;
  fe_config->luma_y = cpp_frame_info->plane_info[0].v_scale_initial_phase;
  fe_config->chroma_x = cpp_frame_info->plane_info[1].h_scale_initial_phase;
  fe_config->chroma_y = cpp_frame_info->plane_info[1].v_scale_initial_phase;
  if ((hw_params->rotation == 0) || (hw_params->rotation == 2)) {
    fe_config->luma_dx = cpp_frame_info->plane_info[0].h_scale_ratio *
      cpp_frame_info->plane_info[0].dst_width;
    fe_config->luma_dy = cpp_frame_info->plane_info[0].v_scale_ratio *
      cpp_frame_info->plane_info[0].dst_height;
    fe_config->chroma_dx = cpp_frame_info->plane_info[1].h_scale_ratio *
      cpp_frame_info->plane_info[1].dst_width;
    fe_config->chroma_dy = cpp_frame_info->plane_info[1].v_scale_ratio *
      cpp_frame_info->plane_info[1].dst_height;
  } else {
    fe_config->luma_dx = cpp_frame_info->plane_info[0].v_scale_ratio *
      cpp_frame_info->plane_info[0].dst_width;
    fe_config->luma_dy = cpp_frame_info->plane_info[0].h_scale_ratio *
      cpp_frame_info->plane_info[0].dst_height;
    fe_config->chroma_dx = cpp_frame_info->plane_info[1].v_scale_ratio *
      cpp_frame_info->plane_info[1].dst_width;
    fe_config->chroma_dy = cpp_frame_info->plane_info[1].h_scale_ratio *
      cpp_frame_info->plane_info[1].dst_height;
  }

  memcpy(&cpp_meta_info->cpp_frame_msg, msm_cpp_frame_info->cpp_cmd_msg,
    msm_cpp_frame_info->msg_len * sizeof(unsigned int));
  cpp_meta_info->size = msm_cpp_frame_info->msg_len;
  meta_data->header.tuning_size[PPROC_META_DATA_CPP_IDX] =
    (cpp_meta_info->size + 1) * sizeof(uint32_t);
  meta_data->entry[PPROC_META_DATA_CPP_IDX].dump_type =
    PPROC_META_DATA_CPP;
  meta_data->entry[PPROC_META_DATA_CPP_IDX].len =
    (cpp_meta_info->size + 1) * sizeof(uint32_t);
  meta_data->entry[PPROC_META_DATA_CPP_IDX].lux_idx =
    hw_params->aec_trigger.lux_idx;
  meta_data->entry[PPROC_META_DATA_CPP_IDX].gain =
    hw_params->aec_trigger.gain;
  meta_data->entry[PPROC_META_DATA_CPP_IDX].component_revision_no =
    fw_version;
  meta_data->entry[PPROC_META_DATA_CPP_IDX].pproc_meta_dump = cpp_meta_info;

  meta_data->header.tuning_size[PPROC_META_DATA_FE_IDX] = sizeof(fe_config_t);
  meta_data->entry[PPROC_META_DATA_FE_IDX].dump_type =
    PPROC_META_DATA_FE;
  meta_data->entry[PPROC_META_DATA_FE_IDX].len =
    sizeof(fe_config_t);
  meta_data->entry[PPROC_META_DATA_FE_IDX].lux_idx =
    hw_params->aec_trigger.lux_idx;
  meta_data->entry[PPROC_META_DATA_FE_IDX].gain =
    hw_params->aec_trigger.gain;
  meta_data->entry[PPROC_META_DATA_FE_IDX].component_revision_no =
    fw_version;
  meta_data->entry[PPROC_META_DATA_FE_IDX].pproc_meta_dump = fe_config;

  return 0;
}

int32_t cpp_hardware_flush_frame(cpp_hardware_t *cpphw,
  struct msm_cpp_frame_info_t *frame)
{
  int32_t rc = 0;
  cpp_hardware_stream_status_t *stream_status;
  struct msm_pproc_queue_buf_info queue_buf_info;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;

  memset(&queue_buf_info, 0, sizeof(struct msm_pproc_queue_buf_info));
  queue_buf_info.buff_mgr_info.session_id =
    ((frame->identity >> 16) & 0xFFFF);
  queue_buf_info.buff_mgr_info.stream_id = (frame->identity & 0xFFFF);
  queue_buf_info.buff_mgr_info.frame_id = frame->frame_id;
  queue_buf_info.buff_mgr_info.timestamp = frame->timestamp;
  queue_buf_info.buff_mgr_info.index = frame->output_buffer_info[0].index;
  queue_buf_info.is_buf_dirty = (frame->frame_type != MSM_CPP_OFFLINE_FRAME) ?
    TRUE : FALSE;

  v4l2_ioctl.ioctl_ptr = &queue_buf_info;
  v4l2_ioctl.len = sizeof(struct msm_pproc_queue_buf_info);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_QUEUE_BUF, &v4l2_ioctl);
  if (rc < 0) {
    CPP_ERR("v4l2 ioctl() failed, rc=%d", rc);
  }

  stream_status = cpp_hardware_get_stream_status(cpphw,
    frame->identity);
  if (stream_status) {
    stream_status->pending_partial_frame--;
    if (stream_status->pending_divert)
      stream_status->pending_divert--;
    /* send signal to thread which is waiting on stream_off
       for pending buffer to be zero */
    if ((stream_status->stream_off_pending == TRUE) &&
      (stream_status->pending_buf == 0) &&
      (stream_status->pending_divert == 0) &&
      (stream_status->pending_partial_frame == 0) &&
      (stream_status->pending_buf_divert == 0)) {
      CPP_DBG("info: sending broadcast for pending stream-off");
      pthread_cond_broadcast(&(cpphw->subdev_cond));
    }
  }

  return 0;
}

#ifdef _ANDROID_
void cpp_hardware_dump_payload(cpp_hardware_t *cpphw,
  struct msm_cpp_frame_info_t *msm_cpp_frame_info,
  cpp_hardware_params_t *hw_params)
{
  char cpp_prop[PROPERTY_VALUE_MAX];
  uint32_t temp;
  int32_t count = 0;
  char name[FILENAME_MAX] = "";
  char sufix_file_name[FILENAME_MAX] = "";
  int32_t out_file_fd;
  char string[255];
  /* Usage: To enable dumps
    Preview: adb shell setprop camera.cpp.dumppreviewpayload x
      will dump payload of x frames.
    offline: adb shell camera.cpp.dumpreprocesspayload 1
    will dump payload offline frames.
    To dump x frames again, just reset prop value to 0 and then set again */

  switch(hw_params->stream_type) {
    case CAM_STREAM_TYPE_PREVIEW:
      property_get("vendor.camera.cpp.dumppreviewpayload", cpp_prop, "0");
      temp = atoi(cpp_prop);
      if (temp == 0)
        cpphw->dump_preview_cnt = 0;
      snprintf(sufix_file_name, sizeof(sufix_file_name),
        "preview");
      if ((temp != 0) && (cpphw->dump_preview_cnt == 0)) {
        cpphw->dump_preview_cnt = temp + msm_cpp_frame_info->frame_id;
        CPP_LOW("dumping preview %d", cpphw->dump_preview_cnt);
      }
      count = cpphw->dump_preview_cnt;
      break;
      case CAM_STREAM_TYPE_VIDEO:
        property_get("vendor.camera.cpp.dumpvideopayload", cpp_prop, "0");
        temp = atoi(cpp_prop);
        if (temp == 0)
          cpphw->dump_video_cnt = 0;
        snprintf(sufix_file_name, sizeof(sufix_file_name),
          "video");
        if ((temp != 0) && (cpphw->dump_video_cnt == 0)) {
          cpphw->dump_video_cnt = temp + msm_cpp_frame_info->frame_id;
          CPP_LOW("dumping video %d", cpphw->dump_video_cnt);
        }
        count = cpphw->dump_video_cnt;
        break;
    case CAM_STREAM_TYPE_OFFLINE_PROC:
      property_get("vendor.camera.cpp.dumpreprocesspayload", cpp_prop, "0");
      temp = atoi(cpp_prop);
      cpphw->dump_snapshot_cnt = 0;
      snprintf(sufix_file_name, sizeof(sufix_file_name),
        "snapshot");
      if (temp != 0) {
        cpphw->dump_snapshot_cnt = temp + msm_cpp_frame_info->frame_id;
        CPP_LOW("dumping snapshot %d", cpphw->dump_snapshot_cnt);
      }
      count = cpphw->dump_snapshot_cnt;
      break;
      default:
        CPP_LOW("dumping not supported for stream type %d", hw_params->stream_type);
        break;
  }

  if (msm_cpp_frame_info->frame_id < count ) {
    char timeBuf[128];
    time_t current_time;
    struct tm * timeinfo;

    memset(timeBuf, 0, sizeof(timeBuf));
    time (&current_time);
    timeinfo = localtime (&current_time);
    timeBuf[0] = '\0';
    if (timeinfo)
      strftime(timeBuf, sizeof(timeBuf),
            "%Y%m%d%H%M%S", timeinfo);
    snprintf(name, sizeof(name), QCAMERA_DUMP_FRM_LOCATION
      "%scpp_payload_%s_%d_%d_%d_%d_%d.txt",
      timeBuf, sufix_file_name, hw_params->input_info.width,
      hw_params->input_info.height, hw_params->input_info.stride,
      hw_params->input_info.scanline,
      hw_params->frame_id);
    out_file_fd = open(name, O_RDWR | O_CREAT, 0777);
    if (out_file_fd >= 0) {
      uint32_t i;
      for (i = 0;i < msm_cpp_frame_info->msg_len; ++i) {
        snprintf(string, sizeof(string), "msg[%03d] = 0x%08x\n",
          i, msm_cpp_frame_info->cpp_cmd_msg[i]);
        write(out_file_fd, &string[0], strlen(&string[0]));
      }
      close(out_file_fd);
    }
    CPP_LOW("dumping payload for stream type %d frame_id %d",
      hw_params->stream_type, msm_cpp_frame_info->frame_id);
  }
}
#endif

/* cpp_hardware_process_frame:
 *
 **/
static int32_t cpp_hardware_process_frame(cpp_hardware_t *cpphw,
  cpp_hardware_cmd_t *cmd)
{
  int32_t rc = 0;
  struct cpp_frame_info_t cpp_frame_info;
  struct msm_cpp_frame_info_t *msm_cpp_frame_info;
  int retry = 0;
  cpp_hardware_params_t *hw_params = cmd->u.hw_params;
  cpp_hardware_stream_status_t *dup_stream_status = NULL;

  memset(&cpp_frame_info, 0, sizeof(struct cpp_frame_info_t));
  if (!cpphw) {
    CPP_ERR("cpp hw invalid, failed");
    return -EINVAL;
  }

  CPP_LOW("identity=0x%x", hw_params->identity);

  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  cpp_hardware_stream_status_t *stream_status =
    cpp_hardware_get_stream_status(cpphw, hw_params->identity);
  if (!stream_status) {
#if 0
    CPP_ERROR("failed");
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EFAULT;
#else
    /* Invalid stream at this layer means the stream is invalidated due to
       racing streamoff and this is considered as a racing process frame
       before queue invalidate */
     /* Need to trigger ack so return -EAGAIN*/
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    CPP_ERR("Invalid stream status, identity =0x%x", hw_params->identity);
    return -EAGAIN;
#endif
  }

  /* TODO: cant do this, thread wont be able to release ack for this frame */
#if 0
  /* if a stream of pending for this identity, drop this frame and return */
  if (stream_status->stream_off_pending == TRUE) {
    CPP_DBG("pending stream-off, frame dropped, identity=0x%x",
      hw_params->identity);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return 0;
  }
#endif

  CPP_BUF_HIGH("stream_status:%p, iden:0x%x, cur_frame_id:%d\n",
    stream_status, stream_status->identity, hw_params->frame_id);

  ATRACE_BEGIN_SNPRINTF(35, "cpp_hardware_process_frame id%d", hw_params->frame_id);
  ATRACE_END();

  rc = cpp_params_create_frame_info(cpphw, hw_params, &cpp_frame_info);

  if(rc == -CPP_ERR_PROCESS_FRAME_ERROR) {
    CPP_BUF_DBG("Error : id %x Acking\n",
        hw_params->identity);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return rc;
  }

  /* Translate the sysetm interface params to msm frame cfg params */
  cpp_frame_info.frame_id = hw_params->frame_id;
  cpp_frame_info.timestamp = hw_params->timestamp;
  cpp_frame_info.buff_index = hw_params->buffer_info.index;
  cpp_frame_info.native_buff = hw_params->buffer_info.native_buff;
  cpp_frame_info.in_buff_identity = hw_params->buffer_info.identity;
  cpp_frame_info.in_buff_buffer_access = hw_params->buffer_info.buffer_access;
#ifndef CACHE_PHASE2
  // Since we cannot pass in_buff_buffer_access flags to kernel,
  // workaround it with saving the flags in cookie
  cpp_module_hw_cookie_t *cookie = (cpp_module_hw_cookie_t *)hw_params->cookie;
  if (cookie) {
    cookie->buffer_access = cpp_frame_info.in_buff_buffer_access;
  }
#endif
  cpp_frame_info.cookie = hw_params->cookie;
  cpp_frame_info.identity = hw_params->identity;
  cpp_frame_info.processed_divert =
    (int32_t)hw_params->output_buffer_info.processed_divert;
  cpp_frame_info.frame_type = MSM_CPP_REALTIME_FRAME;
  if (hw_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    cpp_frame_info.frame_type = MSM_CPP_OFFLINE_FRAME;
  }
  cpp_frame_info.plane_info[0].src_fd = hw_params->buffer_info.fd;
  cpp_frame_info.plane_info[1].src_fd = hw_params->buffer_info.fd;
  cpp_frame_info.out_buff_info = hw_params->output_buffer_info;
  CPP_BUF_DBG("fd %d, index 0x%x, offset %d, native_buff %d, alloc_len %d,"
    "vaddr %p, identity %d, processed_divert %d",
    hw_params->output_buffer_info.fd, hw_params->output_buffer_info.index,
    hw_params->output_buffer_info.offset,
    hw_params->output_buffer_info.native_buff,
    hw_params->output_buffer_info.alloc_len,
    hw_params->output_buffer_info.vaddr,
    hw_params->output_buffer_info.identity,
    hw_params->output_buffer_info.processed_divert);
  cpp_frame_info.out_buff_info.processed_divert =
    (int32_t)hw_params->output_buffer_info.processed_divert;
  cpp_frame_info.tnr_input_scratch_buff_info = hw_params->
    tnr_scratch_bfr_array.buff_array[0];
  cpp_frame_info.tnr_output_scratch_buff_info = hw_params->
    tnr_scratch_bfr_array.buff_array[1];
  /* TODO: Removal of this member needs careful kernel/userspace dependency */
  cpp_frame_info.plane_info[0].dst_fd = -1;
  cpp_frame_info.plane_info[1].dst_fd = -1;

  cpp_frame_info.dup_buff_info.processed_divert = 0;
  cpp_frame_info.dup_buff_info.identity = 0x00000000;
  if (hw_params->duplicate_output == TRUE) {
    /* The stream status needs to be checked because the duplicate flag is
      tagged before streamoff call is received. The streamoff call will not be
      clearing the duplicate flag. */
    dup_stream_status =
      cpp_hardware_get_stream_status(cpphw, hw_params->duplicate_identity);
    if (dup_stream_status) {
      /* The stream is ON */
      cpp_frame_info.dup_output = hw_params->duplicate_output;
      cpp_frame_info.dup_buff_info.identity = hw_params->duplicate_identity;
      cpp_frame_info.dup_buff_info.processed_divert =
        hw_params->dup_buffer_info.processed_divert;
      cpp_frame_info.dup_buff_info.index = hw_params->dup_buffer_info.index;
      cpp_frame_info.dup_buff_info.native_buff =
        hw_params->dup_buffer_info.native_buff;
      CPP_BUF_DBG("index 0x%x, native_buff %d, identity %d, processed_divert %d",
        hw_params->dup_buffer_info.index,
        hw_params->dup_buffer_info.native_buff,
        hw_params->dup_buffer_info.identity,
        hw_params->dup_buffer_info.processed_divert);
    } else {
      CPP_ERR("Duplication stream is already off");
    }
  }

  uint32_t i;
  /* copy batch info from hw_params to msm_cpp_frame_info. */
  if (hw_params->batch_info.batch_mode) {

    if (CAM_STREAM_TYPE_VIDEO == hw_params->stream_type) {
      cpp_frame_info.batch_info.batch_mode = BATCH_MODE_VIDEO;
      cpp_frame_info.batch_info.batch_size = hw_params->batch_info.batch_size;
    } else if (hw_params->duplicate_output &&
      hw_params->stream_type == CAM_STREAM_TYPE_PREVIEW) {
      cpp_frame_info.batch_info.batch_mode = BATCH_MODE_PREVIEW;
      cpp_frame_info.batch_info.batch_size = hw_params->batch_info.batch_size;
    } else{
      cpp_frame_info.batch_info.batch_mode = BATCH_MODE_PREVIEW;
      cpp_frame_info.batch_info.batch_size = 1;
    }
    for (i = 0; i < MAX_PLANES; i++){
      cpp_frame_info.batch_info.intra_plane_offset[i] = hw_params->batch_info.intra_plane_offset[i];
    }
    cpp_frame_info.batch_info.pick_preview_idx = hw_params->batch_info.pick_preview_idx;
  } else{
    cpp_frame_info.batch_info.batch_mode = BATCH_MODE_NONE;
    cpp_frame_info.batch_info.batch_size = 1;
    for (i = 0; i < MAX_PLANES; i++) {
      cpp_frame_info.batch_info.intra_plane_offset[i] = 0;
    }
    cpp_frame_info.batch_info.pick_preview_idx = 0;
  }

  msm_cpp_frame_info = cpp_hardware_create_hw_frame(cpphw, &cpp_frame_info);
  if (!msm_cpp_frame_info) {
    CPP_ERR("frame info invalid, failed");
    cpp_hardware_destroy_hw_frame(msm_cpp_frame_info);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EFAULT;
  }

  cpp_module_util_dumps_feature_info(cpphw, hw_params, &cpp_frame_info, msm_cpp_frame_info);
  CPP_BUF_DBG("identity 0x%x buf_divert %d duplicate 0x%x dup_buf_divert %d",
    msm_cpp_frame_info->identity,
    msm_cpp_frame_info->output_buffer_info[0].processed_divert,
    msm_cpp_frame_info->duplicate_identity,
    msm_cpp_frame_info->duplicate_buffer_info.processed_divert);

  /* Partial_frame is for offline frames only and only. If total_stripes is
     large enough and need to split, then we split the frames into
     partial_frame check and if needed, prepare first_payload */
  if (msm_cpp_frame_info->frame_type == MSM_CPP_OFFLINE_FRAME) {
    rc = cpp_thread_check_and_split_frame(cmd->ctrl, msm_cpp_frame_info, cpphw);
    if (rc < 0) {
      CPP_ERR("failed: cpp_thread_check_and_split_frame failed rc %d\n", rc);
      PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
      return rc;

    }
  }
  /* Fill bus message params */
  rc = cpp_hardware_dump_metadata(hw_params, msm_cpp_frame_info,
    &cpp_frame_info, cpphw->fw_version);
  if (rc < 0) {
    CPP_ERR("failed: cpp_hardware_fill_bus_message_params rc %d\n", rc);
  }

  /* Copy the current diagnostics parameters */
  if(hw_params->diagnostic_enable)
  {
    hw_params->asf9x9_diag.valid = 0;
    hw_params->asf_diag.valid = 0;
    if ((cpphw->fw_version == CPP_FW_VERSION_1_6_0) ||
       (cpphw->fw_version == CPP_FW_VERSION_1_8_0) ||
       (cpphw->fw_version == CPP_FW_VERSION_1_10_0) ||
       (cpphw->fw_version == CPP_FW_VERSION_1_12_0) ||
       (cpphw->fw_version == CPP_FW_VERSION_1_12_1)) {
      hw_params->asf9x9_diag.valid = 1;
    } else {
      hw_params->asf_diag.valid = 1;
    }
    if(hw_params->ez_tune_asf_enable) {
      cpp_hw_params_copy_asf_diag_params(&cpp_frame_info, &hw_params->asf_diag);
      cpp_hw_params_copy_asf9x9_diag_params(&cpp_frame_info,
        &hw_params->asf9x9_diag);
    }
    if(hw_params->ez_tune_wnr_enable)
      cpp_hw_params_copy_wnr_diag_params(&cpp_frame_info, &hw_params->wnr_diag);
  }

  /* send kernel ioctl for processing */
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  int32_t status = 0;
  msm_cpp_frame_info->status = &status;
#ifdef _ANDROID_
  cpp_hardware_dump_payload(cpphw, msm_cpp_frame_info, hw_params);
#endif
  v4l2_ioctl.ioctl_ptr = (void *)msm_cpp_frame_info;
  v4l2_ioctl.len = sizeof(struct msm_cpp_frame_info_t);

  ATRACE_INT_SNPRINTF(1, 40, "Camera:CPP type %d", hw_params->stream_type);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_CFG, &v4l2_ioctl);
  if (rc < 0) {
    CPP_BUF_DBG("v4l2 ioctl() failed. rc:%d, trans_code:%d, stream_id=%d\n",
      rc, *msm_cpp_frame_info->status, (cpp_frame_info.identity & 0xFFFF));
    if (*msm_cpp_frame_info->status == -EAGAIN && hw_params->retry_disable == 0) {
      do {
        CPP_BUF_DBG("retry=%d\n", retry);
        usleep(1000);
        rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_CFG, &v4l2_ioctl);
        if (*msm_cpp_frame_info->status == -EAGAIN) {
          CPP_BUF_DBG("v4l2 ioctl() failed. retry=%d\n", retry);
          retry = retry + 1;
        } else {
          break;
        }
      } while(retry != 5);
    }
    if (*msm_cpp_frame_info->status == -EAGAIN) {
      CPP_BUF_DBG("drop this frame");
      rc = -EAGAIN;
    } else if (*msm_cpp_frame_info->status == -EFAULT) {
      CPP_ERR("page fault, drop this frame rc %d", rc);
      rc = -EFAULT;
    }
    if (rc < 0) {
      CPP_ERR("processing frame failed %d", *msm_cpp_frame_info->status);
      if (msm_cpp_frame_info->partial_frame_indicator) {
        cpp_module_flush_queue_events(cmd->ctrl,
          msm_cpp_frame_info->frame_id, msm_cpp_frame_info->identity, FALSE);
      }
      cpp_hardware_destroy_hw_frame(msm_cpp_frame_info);
      PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
      return rc;
    }
  }

  CPP_BUF_DBG("frame scheduled for processing, frame_id=%d, "
           "buf_idx=%d, identity=0x%x",
           msm_cpp_frame_info->frame_id,
           msm_cpp_frame_info->input_buffer_info.index,
           msm_cpp_frame_info->identity);

  /* For partial_frame, save the remainder of partial_frame for splitting
     in the thread */
  if (!msm_cpp_frame_info->partial_frame_indicator) {
    cpp_hardware_destroy_hw_frame(msm_cpp_frame_info);
  } else {
    stream_status->pending_partial_frame++;
  }

  /* update stream status */
  stream_status->pending_buf++;
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

/* cpp_hardware_unsubcribe_v4l2_event:
 *
 **/
static int32_t cpp_hardware_unsubcribe_v4l2_event(cpp_hardware_t *cpphw)
{
  int32_t rc;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct v4l2_event_subscription sub;

  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  if (cpphw->event_subs_info.valid == TRUE) {
    cpphw->event_subs_info.valid = FALSE;
    sub.id = cpphw->event_subs_info.id;
    sub.type = cpphw->event_subs_info.type;
    rc = ioctl(cpphw->subdev_fd, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
    if (rc < 0) {
      CPP_ERR("v4l2 ioctl() failed, rc=%d", rc);
      PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
      return -EIO;
    }
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

/* cpp_hardware_find_subdev:
 *
 **/
static int32_t cpp_hardware_find_subdev(cpp_hardware_t *cpphw)
{
  uint32_t i = 0;
  int32_t rc = 0;
  int fd;
  struct media_device_info mdev_info;
  char name[SUBDEV_NAME_SIZE_MAX];
  if (!cpphw) {
    CPP_ERR("invalid cpp hw, failed");
    return -EINVAL;
  }
  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  cpphw->num_subdev = 0;
  while(1) {
    snprintf(name, sizeof(name), "/dev/media%d", i);
    CPP_DBG("media device: %s\n",name);
    fd = open(name, O_RDWR | O_NONBLOCK);
    if(fd < 0) {
      CPP_DBG("no more media devices");
      break;
    }
    i++;
    rc = ioctl(fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (rc < 0) {
      CPP_ERR("ioctl() failed: %s", strerror(errno));
      close(fd);
      /* check for next device */
      continue;
    }
    if ((strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model))) != 0) {
      /* not camera device */
      close(fd);
      continue;
    }
    uint32_t num_entities = 0;
    /* enumerate all entities in the media device */
    while(1) {
      if (cpphw->num_subdev >= MAX_CPP_DEVICES) {
        break;
      }
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities | MEDIA_ENT_ID_FLAG_NEXT;
      rc = ioctl(fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (rc < 0) {
        CPP_DBG("done enumerating media entities");
        rc = 0;
        break;
      }
      num_entities = entity.id;
      CPP_DBG("entity.name=%s entity.revision=%d, entity.type=%x id %x\n",
        entity.name, entity.revision, entity.type, MSM_CAMERA_SUBDEV_CPP);
      if(mct_util_match_subdev_type(entity, MSM_CAMERA_SUBDEV_CPP,
        MEDIA_ENT_T_V4L2_SUBDEV)) {
        int subdev_num;
        sscanf(entity.name, "v4l-subdev%d", &subdev_num);
        CPP_DBG("CPP entity found: name=%s", entity.name);
        cpphw->subdev_ids[cpphw->num_subdev] = subdev_num;
        cpphw->num_subdev++;
      }
    }
    close(fd);
  }
  if (cpphw->num_subdev == 0) {
    CPP_ERR("no cpp device found");
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -ENODEV;
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

cpp_firmware_version_t cpp_hardware_get_fw_version(cpp_hardware_t *cpphw)
{
  return cpphw->fw_version;
}


/* cpp_hardware_pop_stream_buffer:
 *
 **/
static int32_t cpp_hardware_pop_stream_buffer(cpp_hardware_t *cpphw,
  cpp_hardware_event_data_t *event_data)
{
  int32_t rc = 0;
  struct msm_cpp_frame_info_t frame;

  if(!event_data) {
    CPP_ERR("null event data, failed\n");
    return -EINVAL;
  }
  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  v4l2_ioctl.ioctl_ptr = (void *)&frame;
  v4l2_ioctl.len = sizeof(struct msm_cpp_frame_info_t);
  frame.frame_id = event_data->frame_id;
  frame.identity = event_data->identity;
  frame.output_buffer_info[0].index = event_data->buf_idx;
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_POP_STREAM_BUFFER, &v4l2_ioctl);
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return rc;
}

static int32_t cpp_hardware_process_partial_frame(cpp_hardware_t *cpphw,
  struct msm_cpp_frame_info_t *partial_frame)
{
  int32_t rc = 0;
  int retry = 0;

  if (!cpphw || !partial_frame) {
    CPP_ERR("failed, cpphw:%p, partial_frame:%p\n", cpphw, partial_frame);
    return -EINVAL;
  }

  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));

  cpp_hardware_stream_status_t *stream_status =
    cpp_hardware_get_stream_status(cpphw, partial_frame->identity);
  if (!stream_status) {
    /* Invalid stream at this layer means the stream is invalidated due to
       racing streamoff and this is considered as a racing process frame
       before queue invalidate */
     /* Need to trigger ack so return -EAGAIN*/
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EAGAIN;
  }

  CPP_BUF_DBG("identity=0x%x", partial_frame->identity);

  /* send kernel ioctl for processing */
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  int32_t status = 0;
  partial_frame->status = &status;
  v4l2_ioctl.ioctl_ptr = (void *)partial_frame;
  v4l2_ioctl.len = sizeof(struct msm_cpp_frame_info_t);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_CFG, &v4l2_ioctl);
  if (rc < 0) {
    CPP_BUF_DBG("v4l2 ioctl() failed. rc:%d, trans_code:%d, stream_id=%d\n",
      rc, *partial_frame->status, (partial_frame->identity) & 0xFFFF);
    if (*partial_frame->status == -EAGAIN) {
      CPP_BUF_DBG("drop this frame");
      rc = -EAGAIN;
    }
    if (rc < 0) {
      PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
      partial_frame->status = NULL;
      return rc;
    }
  }
  CPP_BUF_DBG("frame scheduled for processing, frame_id=%d, "
           "buf_idx=%d, identity=0x%x",
           partial_frame->frame_id,
           partial_frame->input_buffer_info.index,
           partial_frame->identity);

  if (partial_frame->last_payload) {
    cpp_hardware_destroy_hw_frame(partial_frame);
    partial_frame = NULL;
  } else
    partial_frame->status = NULL;

  /* update stream status */
  stream_status->pending_buf++;
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

uint32_t cpp_hardware_calculate_stripe_info(uint32_t stripe_count,
  int32_t plane_index)
{
  uint32_t stripe_info = stripe_count;
  int32_t i;

  if (plane_index) {
    for (i = 0; i < plane_index; i++) {
      stripe_info = stripe_info << 10;
    }
  }
  return stripe_info;
}

void cpp_hardware_update_partial_stripe_info(uint32_t *stripe_count,
  cpp_hardware_partial_stripe_info_t *partial_frame, uint32_t *total_stripes)
{
  int32_t i;
  for (i = 2; i >= 0; i--) {
    if (stripe_count[i]) {
      uint32_t remainder;
      remainder = stripe_count[i] % PARTIAL_FRAME_STRIPE_COUNT;
      if (!remainder) {
        remainder = PARTIAL_FRAME_STRIPE_COUNT;
      }
      partial_frame->first_stripe_index = *total_stripes - remainder;
      partial_frame->last_stripe_index = *total_stripes - 1;
      stripe_count[i] -= remainder;
      *total_stripes -= remainder;
      partial_frame->stripe_info =
        cpp_hardware_calculate_stripe_info(remainder, i);
      break;
    }
  }
}

static int32_t cpp_thread_check_and_split_frame(cpp_module_ctrl_t* ctrl,
  struct msm_cpp_frame_info_t *partial_frame, cpp_hardware_t *cpphw __unused)
{
  uint32_t                           stripe_count[3] = {0, 0, 0};
  uint32_t                           total_stripes = 0;
  uint32_t                          *cpp_frame_msg;
  int32_t                            i;
  cpp_hardware_partial_stripe_info_t stripe_info;
  cpp_module_event_t                *partial_frame_event = NULL;

  cpp_frame_msg = partial_frame->cpp_cmd_msg;

  stripe_count[0] =
    cpp_frame_msg[partial_frame->stripe_info_offset] & 0x3FF;
  stripe_count[1] =
    (cpp_frame_msg[partial_frame->stripe_info_offset] >> 10) & 0x3FF;
  stripe_count[2] =
    (cpp_frame_msg[partial_frame->stripe_info_offset] >> 20) & 0x3FF;
  total_stripes = stripe_count[0] + stripe_count[1] + stripe_count[2];

  if (total_stripes <= PARTIAL_FRAME_STRIPE_COUNT) {
    return 0;
  }

  /* Break into partial_frame, starting from the last stripe */
  partial_frame->partial_frame_indicator = 1;
  partial_frame->first_payload = 1;
  partial_frame->last_payload = 0;
  /* Update stripe_count & total_stripes after creating partial frame. This
     part of partial frame is going to be scheduled immediately. There is
     no need to enqueue to the partial frame queue */
  cpp_hardware_update_partial_stripe_info(&stripe_count[0],
    &stripe_info, &total_stripes);
  partial_frame->first_stripe_index = stripe_info.first_stripe_index;
  partial_frame->last_stripe_index = stripe_info.last_stripe_index;
  partial_frame->stripe_info = stripe_info.stripe_info;
  partial_frame->cpp_cmd_msg[partial_frame->stripe_info_offset] =
    partial_frame->stripe_info;

  while (total_stripes) {
    /* These partial_frame entities are going to be enqueued */
    partial_frame_event =
      (cpp_module_event_t*)CAM_MALLOC(sizeof(cpp_module_event_t));
    if(!partial_frame_event) {
      free(partial_frame->cpp_cmd_msg);
      free(partial_frame);
      CPP_ERR("CAM_MALLOC() failed\n");
      return -ENOMEM;
    }
    memset(partial_frame_event, 0x00, sizeof(cpp_module_event_t));
    partial_frame_event->type = CPP_MODULE_EVENT_PARTIAL_FRAME;
    partial_frame_event->invalid = FALSE;
    /* this is hw processing event */
    partial_frame_event->hw_process_flag = TRUE;
    partial_frame_event->ack_key =
      ((cpp_module_hw_cookie_t *)partial_frame->cookie)->key;
    partial_frame_event->u.partial_frame.frame = partial_frame;

    cpp_hardware_update_partial_stripe_info(&stripe_count[0],
      &partial_frame_event->u.partial_frame.partial_stripe_info,
      &total_stripes);

    PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
    cpp_module_enq_event(ctrl, partial_frame_event,
      CPP_PRIORITY_PARTIAL_FRAME);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
  }
  if (partial_frame_event) {
    /* this is the last payload. Indicate in the stored stripe info */
    partial_frame_event->u.partial_frame.partial_stripe_info.last_payload = 1;
  } else {
    /* The last payload is indicated in current stripe payload */
    partial_frame->last_payload = 1;
  }
  return 0;
}
void cpp_hardware_set_private_data(cpp_hardware_t *cpphw, void *data)
{
  if (cpphw != NULL) {
    PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
    cpphw->private_data = data;
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  }
}

int32_t cpp_hardware_update_pending_buffer(cpp_hardware_t *cpphw, int32_t value)
{
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  if (cpphw != NULL) {
    cpphw->max_pending_buffer = value ? CPP_HARDWARE_MAX_PENDING_BUF :
      CPP_HARDWARE_DEFAULT_PENDING_BUF;
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}
