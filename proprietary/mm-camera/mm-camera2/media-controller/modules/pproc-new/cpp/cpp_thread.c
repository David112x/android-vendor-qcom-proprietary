/*============================================================================

  Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "eztune_diagnostics.h"
#include "cpp_thread.h"
#include "cpp_module.h"
#include "cpp_log.h"
#include <poll.h>
#include <unistd.h>

#define PIPE_FD_IDX   0
#define SUBDEV_FD_IDX 1

#define CPP_FRAME_DUMP_MASK 0xF0

static int32_t cpp_thread_split_partial_frame(cpp_module_ctrl_t* ctrl,
  cpp_module_event_t* cpp_event, struct msm_cpp_frame_info_t *partial_frame);
static int32_t cpp_thread_handle_partial_frame_event(cpp_module_ctrl_t* ctrl,
  cpp_module_event_t* cpp_event);
static int32_t cpp_thread_handle_isp_drop_buffer_event(cpp_module_ctrl_t* ctrl,
  cpp_module_event_t* cpp_event);
static int32_t cpp_thread_process_pipe_message(cpp_module_ctrl_t *ctrl,
  cpp_thread_msg_t msg);
static void cpp_thread_fatal_exit(cpp_module_ctrl_t *ctrl, boolean post_to_bus);
static int32_t cpp_thread_process_hardware_event(cpp_module_ctrl_t *ctrl);


/** cpp_thread_func:
 *
 * Description:
 *   Entry point for cpp_thread. Polls over pipe read fd and cpp
 *   hw subdev fd. If there is any new pipe message or hardware
 *   event, it is processed.
 **/
void* cpp_thread_func(void* data)
{
  int32_t rc;
  mct_module_t *module = (mct_module_t *) data;
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
  ctrl->cpp_thread_started = TRUE;
  pthread_cond_signal(&(ctrl->th_start_cond));
  PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));

  /* open the cpp hardware */
  rc = cpp_hardware_open_subdev(ctrl->cpphw);
  if(rc < 0) {
    CPP_ERR("cpp_hardware_open_subdev() failed");
    cpp_thread_fatal_exit(ctrl, TRUE);
  }

  if(ctrl->cpphw->subdev_opened == FALSE) {
    CPP_ERR("failed, cpp subdev not open");
    cpp_thread_fatal_exit(ctrl, TRUE);
  }
  /* subscribe for event on subdev fd */
  cpp_hardware_cmd_t cmd;
  cmd.type = CPP_HW_CMD_SUBSCRIBE_EVENT;
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if(rc < 0) {
    CPP_ERR("failed, cannot subscribe to cpp hardware event");
    cpp_thread_fatal_exit(ctrl, TRUE);
  }

  /* poll on the pipe readfd and subdev fd */
  struct pollfd pollfds[2];
  uint32_t num_fds = 2;
  int ready = 0;
  uint32_t i = 0;
  pollfds[PIPE_FD_IDX].fd = ctrl->pfd[READ_FD];
  pollfds[PIPE_FD_IDX].events = POLLIN|POLLPRI;
  pollfds[SUBDEV_FD_IDX].fd = ctrl->cpphw->subdev_fd;
  pollfds[SUBDEV_FD_IDX].events = POLLIN|POLLPRI;
  CPP_HIGH("cpp_thread entering the polling loop...");
  while(1) {
    /* poll on the fds with no timeout */
    ready = poll(pollfds, (nfds_t)num_fds, -1);
    if(ready > 0) {
      /* loop through the fds to see if any event has occured */
      for(i=0; i<num_fds; i++) {
        if(pollfds[i].revents & (POLLIN|POLLPRI)) {
          switch(i) {
          case PIPE_FD_IDX: {
            int num_read=0;
            cpp_thread_msg_t pipe_msg;
            num_read = read(pollfds[i].fd, &(pipe_msg),
                         sizeof(cpp_thread_msg_t));
            if(num_read < 0) {
              CPP_ERR("read() failed, rc=%d", num_read);
              cpp_thread_fatal_exit(ctrl, TRUE);
            } else if(num_read != sizeof(cpp_thread_msg_t)) {
              CPP_ERR("failed, in read(), num_read=%d, msg_size=%zu",
                num_read, sizeof(cpp_thread_msg_t));
              cpp_thread_fatal_exit(ctrl, TRUE);
            }
            rc = cpp_thread_process_pipe_message(ctrl, pipe_msg);
            if (rc < 0) {
              CPP_ERR("failed %d", rc);
              cpp_thread_fatal_exit(ctrl, TRUE);
            }
            break;
          }
          case SUBDEV_FD_IDX: {
            rc = cpp_thread_process_hardware_event(ctrl);
            if(rc < 0) {
              CPP_ERR("failed %d", rc);
              cpp_thread_fatal_exit(ctrl, TRUE);
            }
            break;
          }
          default:
            CPP_ERR("error, bad fd index");
            cpp_thread_fatal_exit(ctrl, TRUE);
          } /* switch(i) */
        } /* if */
      } /* for */
    } else if(ready == 0){
      CPP_ERR("error: poll() timed out error =%s", strerror(errno));
      cpp_thread_fatal_exit(ctrl, TRUE);
    } else if (ready < 0 && errno == EINTR) {
      CPP_WARN("poll() failed - %s %d, continue...",
        strerror(errno), errno);
      continue;
    } else {
      CPP_ERR("error: poll() failed error =%s %d", strerror(errno), errno);
      cpp_thread_fatal_exit(ctrl, TRUE);
    }
  } /* while(1) */
  return NULL;
}

/* cpp_thread_handle_divert_buf_event:
 *
 *   send a buf divert event to downstream module, if the piggy-backed ACK
 *   is received, we can update the ACK from ack_list, otherwise, the ACK will
 *   be updated when buf_divert_ack event comes from downstream module.
 *
 **/
static int32_t cpp_thread_handle_divert_buf_event(cpp_module_ctrl_t* ctrl,
  cpp_module_event_t* cpp_event)
{
  int32_t rc;
  mct_event_t event;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.identity = cpp_event->u.divert_buf_data.div_identity;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
  event.u.module_event.module_event_data =
    &(cpp_event->u.divert_buf_data.isp_buf_divert);

  cpp_event->u.divert_buf_data.isp_buf_divert.ack_flag = TRUE;

  CPP_BUF_DBG("sending unproc_div, identity=0x%x", event.identity);
  rc = cpp_module_send_event_downstream(ctrl->p_module, &event);
  if (rc < 0) {
    CPP_BUF_ERR("failed %d", rc);
    return -EFAULT;
  }
  CPP_BUF_DBG("unprocessed divert ack = %d",
    cpp_event->u.divert_buf_data.isp_buf_divert.ack_flag);

  /* if ack is piggy backed, we can safely send ack to upstream */
  if (cpp_event->u.divert_buf_data.isp_buf_divert.ack_flag == TRUE) {
    CPP_BUF_DBG("doing ack for divert event");
    cpp_module_do_ack(ctrl, cpp_event->ack_key,
      cpp_event->u.divert_buf_data.isp_buf_divert.buffer_access);
  }
  return 0;
}

#ifdef _ANDROID_
int cpp_thread_dump_frame(cpp_hardware_params_t *hw_params,
  int fd,void *user,bool ip_op)
{
  void *buf = MAP_FAILED;
  size_t buf_size = 0;
  char value[PROPERTY_VALUE_MAX];
  float bytes_per_pixel = 1.5;
  int32_t file_fd = -1;
  char name[100];
  char io[15];
  cpp_params_dim_info_t dim_info;
  int32_t enabled = 0;
  int plane_count = 0;
  cpp_hardware_cmd_t *cmd = (cpp_hardware_cmd_t *)user;
  cpp_module_ctrl_t *ctrl = cmd->ctrl;
  cpp_module_hw_cookie_t *cookie = NULL;
  int32_t rc = 0;
  int count = 0;
  uint32_t video_type_flag = 0;
  int32_t width[MAX_PLANES];
  int32_t height[MAX_PLANES];
  int32_t stride[MAX_PLANES];
  int32_t scanline[MAX_PLANES];
  char timeBuf[128];
  time_t current_time;
  struct tm * timeinfo = NULL;
  static int CPPFrameCnt = 0;

  if(!hw_params || !ctrl) {
    CDBG_ERROR("Error- NULL pointers hw_params = %p,ctrl = %p",
      hw_params,ctrl);
    return 0;
  }
  property_get("persist.vendor.camera.pproc.framedump", value, "0");
  enabled = atoi(value);
  if (!enabled) {
    CPPFrameCnt = 0;
    return 0;
  }
  if (!((1<<(int)hw_params->stream_type) & enabled)) {
    return 0;
  }
  property_get("persist.vendor.camera.pproc.dump_cnt", value, "20");
  count = atoi(value);

  memset(&dim_info, 0x00, sizeof(cpp_params_dim_info_t));
  if(ip_op) { /* Input */
    cookie = (cpp_module_hw_cookie_t *)hw_params->cookie;
    memcpy(&dim_info,&hw_params->input_info,sizeof(cpp_params_dim_info_t));
    strlcpy(io,"Input",sizeof(io));
  } else { /* Output */
    cookie = cmd->u.event_data->cookie;
    memcpy(&dim_info,&hw_params->output_info,sizeof(cpp_params_dim_info_t));
    strlcpy(io,"Output",sizeof(io));
    hw_params->frame_id = cmd->u.event_data->frame_id;
  }

  if (CPPFrameCnt > count) {
    goto buf_done;
  }
  //Swap dimensions of output buffer if rotation 90/270
  if(!ip_op &&
    ((hw_params->rotation == 1) || (hw_params->rotation == 3))) {
    int temp = 0;
    if(!video_type_flag && (hw_params->stream_type != CAM_STREAM_TYPE_VIDEO)) {
      temp = dim_info.stride;
      dim_info.stride = dim_info.scanline;
      dim_info.scanline = temp;

      temp = dim_info.width;
      dim_info.width = dim_info.height;
      dim_info.height = temp;
    }
  }

  //TODO: Handle CDS format also
  switch (dim_info.plane_fmt) {
    case CPP_PARAM_PLANE_CBCR:
    case CPP_PARAM_PLANE_CRCB: {
        width[0] = dim_info.width;
        height[0] = dim_info.height;
        stride[0] = dim_info.stride;
        width[1] = dim_info.width;
        stride[1] = dim_info.stride;
        height[1] = dim_info.height/2;
        bytes_per_pixel = 1.5f;
        plane_count = 2;
      }
      break;
   case CPP_PARAM_PLANE_CBCR_UBWC:
   case CPP_PARAM_PLANE_CRCB_UBWC:{
        width[0] = dim_info.width;
        height[0] = dim_info.height;
        stride[0] = dim_info.stride;
        width[1] = dim_info.width;
        stride[1] = dim_info.stride;
        height[1] = dim_info.height/2;
        bytes_per_pixel = 1.5f;
        plane_count = 2;
      }
      break;
    case CPP_PARAM_PLANE_CRCB420: {
        width[0] = dim_info.width;
        height[0] = dim_info.height;
        stride[0] = dim_info.stride;
        width[1] = dim_info.width/2;
        stride[1] = ((dim_info.stride) + 31)&~31;
        stride[1] /= 2;
        height[1] = dim_info.height/2;
        width[2] = dim_info.width/2;
        stride[2] = ((dim_info.stride) + 31)&~31;
        stride[2] /= 2;
        height[2] = dim_info.height/2;
        plane_count = 3;
        bytes_per_pixel = 1.5f;
      }
      break;
    case CPP_PARAM_PLANE_CBCR422:
    case CPP_PARAM_PLANE_CRCB422: {
      {
        width[0] = dim_info.width;
        height[0] = dim_info.height;
        stride[0] = dim_info.stride;
        width[1] = dim_info.width;
        height[1] = dim_info.height;
        stride[1] = dim_info.stride;
        plane_count = 2;
        bytes_per_pixel = 2;
      }
      break;
    }
    case CPP_PARAM_PLANE_CBCR444:
    case CPP_PARAM_PLANE_CRCB444: {
      {
        width[0] = dim_info.width;
        height[0] = dim_info.height;
        stride[0] = dim_info.stride;
        width[1] = dim_info.width*2;
        height[1] = dim_info.height*2;
        stride[1] = dim_info.stride*2;
        plane_count = 2;
        bytes_per_pixel = 3;
      }
      break;
    }
    default: {
      CDBG_ERROR("Unsupported format for %s buffer iden:0x%x",
        io,hw_params->identity);
      return 0;
    }
  }
  memset(timeBuf, 0, sizeof(timeBuf));
  time (&current_time);
  timeinfo = localtime (&current_time);
  if (timeinfo)
      strftime(timeBuf, sizeof(timeBuf),"%Y%m%d%H%M%S", timeinfo);

  switch(hw_params->stream_type) {
    case CAM_STREAM_TYPE_PREVIEW: {
      snprintf(name, sizeof(name), QCAMERA_DUMP_FRM_LOCATION
      "%s_CPP_%s_Preview_%dx%d_%d.yuv",timeBuf,io,width[0],height[0],
      hw_params->frame_id);
      break;
    }
    case CAM_STREAM_TYPE_VIDEO: {
      snprintf(name, sizeof(name), QCAMERA_DUMP_FRM_LOCATION
      "%s_CPP_%s_Video_%dx%d_%d.yuv",timeBuf,io,width[0],height[0],
      hw_params->frame_id);
      break;
    }
    case CAM_STREAM_TYPE_OFFLINE_PROC: {
      snprintf(name, sizeof(name), QCAMERA_DUMP_FRM_LOCATION
      "%s_CPP_%s_OfflineProc_%dx%d_%d.yuv",timeBuf,io,width[0],height[0],
      hw_params->frame_id);
      break;
    }
    case CAM_STREAM_TYPE_SNAPSHOT: {
      snprintf(name, sizeof(name), QCAMERA_DUMP_FRM_LOCATION
      "%s_CPP_%s_Snapshot_%dx%d_%d.yuv",timeBuf,io,width[0],height[0],
      hw_params->frame_id);
      break;
    }
    case CAM_STREAM_TYPE_POSTVIEW: {
      snprintf(name, sizeof(name), QCAMERA_DUMP_FRM_LOCATION
      "%s_CPP_%s_Postview_%dx%d_%d.yuv",timeBuf,io,width[0],height[0],
      hw_params->frame_id);
      break;
    }
    default:
      CDBG_ERROR("Not dumping for streamtype %d",
        hw_params->stream_type);
      return 0;
  }

  if (ip_op) {
    hw_params->processed_divert |= CPP_FRAME_DUMP_MASK;
    cookie->frame_dump_flag = hw_params->processed_divert;
  }
  buf_size = (size_t)(dim_info.plane_info[0].plane_len +
    dim_info.plane_info[1].plane_len);
  buf = mmap(NULL, buf_size, PROT_READ  | PROT_WRITE,
   MAP_SHARED, fd, 0);
  if(buf == MAP_FAILED) {
    CDBG_ERROR("Error: mmap failed for frameid:%d,iden:0x%x and stream type %d",
      hw_params->frame_id,hw_params->identity,hw_params->stream_type);
    goto buf_done;
  } else {
    file_fd = open(name, O_RDWR | O_CREAT, 0777);
    if (file_fd >= 0) {
      void *data = NULL;
      ssize_t written_len = 0;
      int i = 0;
      int j = 0;
      for (i = 0; i < plane_count; i++) {
        uint32_t index = dim_info.plane_info[i].plane_offsets;
        if (i > 0) {
          index += dim_info.plane_info[i-1].plane_len;
        }
        /* Write plane's meta */
        data = (void *)((uint8_t *)buf + index);
        written_len += write(file_fd, data,
          (size_t)(dim_info.plane_info[i].plane_len));
      }
      CDBG_LOW("Dumped %d bytes of %s frame for stream type =%d and frameID:%d",
        written_len,io,hw_params->stream_type,hw_params->frame_id);
      CPPFrameCnt++;
    } else {
      CDBG_ERROR("Error,File open failed");
      goto buf_done;
    }
  }

 buf_done:
  if (buf != MAP_FAILED)
    munmap(buf, buf_size);
  if (file_fd >= 0)
    close(file_fd);
  /* If output dump and no processed divert, do buf done */
  #if 0
  if (!ip_op ) {
    if (cookie->frame_dump_flag == CPP_FRAME_DUMP_MASK) {
      cmd->u.event_data->is_buf_dirty = 0;
      cmd->type = CPP_HW_CMD_QUEUE_BUF;
      rc = cpp_hardware_process_command(
        ((cpp_module_ctrl_t *)(cmd->ctrl))->cpphw,*cmd);
      if(rc < 0) {
        CDBG_ERROR("failed buf done for frameID:%d,iden:0x%x\n",
          hw_params->frame_id,hw_params->identity);
        free(hw_params->cookie);
        cpp_thread_fatal_exit((cpp_module_ctrl_t *)(cmd->ctrl), FALSE);
      }
      CDBG_HIGH("Buf done on frameid:%d,iden:0x%x",
        hw_params->frame_id,hw_params->identity);
    }
    cookie->frame_dump_flag &= ~CPP_FRAME_DUMP_MASK;
  }
  #endif
  return 0;
}
#endif


/** cpp_thread_update_filters:
 * @ctrl - cpp control structure
 * @hw_params - structure holding the data for CPP hardware
 *
 * Description:
 *      The function calls interpolation of all modules before sent the buffer
 *   for process.
 **/
int32_t cpp_thread_update_filters(cpp_module_ctrl_t *ctrl,
  cpp_hardware_params_t* hw_params)
{
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *dup_stream_params = NULL;
  cpp_module_session_params_t *dup_session_params = NULL;
  modulesChromatix_t          *module_chromatix;
  int32_t ret = 0;

  if(!ctrl) {
    CPP_ERR("invalid cpp control, failed\n");
    return -EFAULT;
  }

  /* get stream parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, hw_params->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CPP_ERR("invalid stream params, failed\n");
    return -EFAULT;
  }

  module_chromatix = &hw_params->module_chromatix;

  /* get the the duplicate stream params if duplicate output is enabled */
  if (hw_params->duplicate_output)
    cpp_module_get_params_for_identity(ctrl, hw_params->duplicate_identity,
      &dup_session_params, &dup_stream_params);

  // update chromatix pointer based on scale ratio
  ret = cpp_module_util_update_chromatix_pointer(stream_params, dup_stream_params,
     module_chromatix,
    (hw_params->cpp_scale_ratio * hw_params->isp_scale_ratio));

  if (ret < 0) {
    CPP_ERR("update chromatix ptr failed\n");
    goto end;
  }

  CPP_HIGH("stream %d, frame %d, aec lux %f, gain %f",
    stream_params->stream_type, hw_params->frame_id,
    hw_params->aec_trigger.lux_idx, hw_params->aec_trigger.gain);

  /* Set the radial center */
  hw_params->input_info.plane_info[0].horizontal_center =
    floor(hw_params->input_info.width * 0.5f);
  hw_params->input_info.plane_info[0].vertical_center =
    floor(hw_params->input_info.height * 0.5f);

  hw_params->input_info.plane_info[1].horizontal_center =
    floor(hw_params->input_info.width * 0.25f);
  hw_params->input_info.plane_info[1].vertical_center =
    floor(hw_params->input_info.height * 0.25f);

  if (hw_params->interpolate_mask & CPP_INTERPOLATE_WNR) {
    if (stream_params->hw_params.denoise_mask == TRUE) {
      cpp_hw_params_update_wnr_params(module_chromatix,
        hw_params, &hw_params->aec_trigger);
      memcpy(stream_params->hw_params.denoise_info,
        hw_params->denoise_info, sizeof(hw_params->denoise_info));
      memcpy(&stream_params->hw_params.wnr_frame_info,
        &hw_params->wnr_frame_info, sizeof(hw_params->wnr_frame_info));
    }
  }

  if (hw_params->interpolate_mask & CPP_INTERPOLATE_TNR) {
    if (ctrl->tnr_module_func_tbl.update) {
      ctrl->tnr_module_func_tbl.update(module_chromatix,
      hw_params, &hw_params->aec_trigger);
      memcpy(&(stream_params->hw_params.tnr_info_Y), &(hw_params->tnr_info_Y),
        sizeof(cpp_tnr_info_t));
      memcpy(&(stream_params->hw_params.tnr_info_Cb), &(hw_params->tnr_info_Cb),
        sizeof(cpp_tnr_info_t));
      memcpy(&(stream_params->hw_params.tnr_info_Cr), &(hw_params->tnr_info_Cr),
        sizeof(cpp_tnr_info_t));
    }
  }

  if (hw_params->interpolate_mask & CPP_INTERPOLATE_ASF) {
    CPP_ASF_DBG("CALL ASF update for identity %x", hw_params->identity);
    if (stream_params->hw_params.asf_mask) {
      cpp_hw_params_asf_interpolate(ctrl->cpphw, hw_params, module_chromatix,
        &hw_params->aec_trigger);
      memcpy(&(stream_params->hw_params.asf_info), &(hw_params->asf_info),
        sizeof(cpp_params_asf_info_t));
    }
  }

  if (hw_params->interpolate_mask & CPP_INTERPOLATE_PBF) {
    if (ctrl->pbf_module_func_tbl.update) {
      ctrl->pbf_module_func_tbl.update(module_chromatix,
          hw_params, &hw_params->aec_trigger);
      memcpy(stream_params->hw_params.pbf_denoise_info,
        hw_params->pbf_denoise_info, sizeof(hw_params->pbf_denoise_info));
     }
  }

  cpp_hw_params_update_out_clamp(module_chromatix, hw_params);

end:
  return ret;
}

/* cpp_thread_handle_process_buf_event:
 *
 * Description:
 *
 *
 **/
static int32_t cpp_thread_handle_process_buf_event(cpp_module_ctrl_t* ctrl,
  cpp_module_event_t* cpp_event)
{
  int32_t rc;
  unsigned long in_frame_fd;
  mct_event_t event;
  cpp_hardware_cmd_t cmd;
  cpp_module_hw_cookie_t *cookie = NULL;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *dup_stream_params = NULL;
  cpp_module_session_params_t *dup_session_params = NULL;
  uint32_t video_type_flag;
  cpp_hardware_params_t* hw_params = NULL;
  uint32_t buffer_access = 0;
  cpp_hardware_stream_status_t *stream_status = NULL;
  cpp_hardware_stream_status_t *duplicate_stream_status = NULL;

  if(!ctrl || !cpp_event) {
    CPP_ERR("failed, ctrl=%p, cpp_event=%p", ctrl, cpp_event);
    return -EINVAL;
  }

  /* cookie is used to attach data to kernel frame, which is to be retrieved
     once processing is done */
  cookie = (cpp_module_hw_cookie_t *) CAM_MALLOC(sizeof(cpp_module_hw_cookie_t));
  if(!cookie) {
    CPP_ERR("CAM_MALLOC failed");
    return -ENOMEM;
  }
  cookie->key = cpp_event->ack_key;
  cookie->proc_div_required = cpp_event->u.process_buf_data.proc_div_required;
  cookie->proc_div_identity = cpp_event->u.process_buf_data.proc_div_identity;
  cookie->meta_datas = cpp_event->u.process_buf_data.isp_buf_divert.meta_data;

  buffer_access = cpp_event->u.process_buf_data.isp_buf_divert.buffer_access;

  hw_params = &(cpp_event->u.process_buf_data.hw_params);
  hw_params->cookie = cookie;
  if (cpp_event->u.process_buf_data.isp_buf_divert.native_buf) {
    in_frame_fd = (unsigned long)cpp_event->u.process_buf_data.isp_buf_divert.fd;
  } else {
    in_frame_fd =
      cpp_event->u.process_buf_data.isp_buf_divert.buffer.m.planes[0].m.userptr;
  }
  hw_params->frame_id =
    cpp_event->u.process_buf_data.isp_buf_divert.buffer.sequence;
  hw_params->vaddr = (void *)cpp_event->u.process_buf_data.isp_buf_divert.plane_vaddr[0];
  CPP_BUF_LOW("plane_vaddr %p",
    (void *)cpp_event->u.process_buf_data.isp_buf_divert.plane_vaddr[0]);
  hw_params->timestamp =
    cpp_event->u.process_buf_data.isp_buf_divert.buffer.timestamp;
  hw_params->identity = cpp_event->u.process_buf_data.proc_identity;
  hw_params->buffer_info.fd = in_frame_fd;
  hw_params->buffer_info.index =
    cpp_event->u.process_buf_data.isp_buf_divert.buffer.index;
  hw_params->buffer_info.native_buff =
    cpp_event->u.process_buf_data.isp_buf_divert.native_buf;
  if (hw_params->buffer_info.native_buff) {
    hw_params->buffer_info.identity = hw_params->identity;
  } else {
    hw_params->buffer_info.identity =
        cpp_event->u.process_buf_data.isp_buf_divert.identity;
  }

  hw_params->uv_upsample_enable =
    cpp_event->u.process_buf_data.isp_buf_divert.is_uv_subsampled;
  if (cpp_event->u.process_buf_data.proc_div_identity == hw_params->identity)
    hw_params->output_buffer_info.processed_divert =
      cpp_event->u.process_buf_data.proc_div_required;
  if (cpp_event->u.process_buf_data.proc_div_identity ==
    hw_params->duplicate_identity)
    hw_params->dup_buffer_info.processed_divert =
      cpp_event->u.process_buf_data.proc_div_required;
#ifdef LDS_ENABLE
  hw_params->lds_enable =
    cpp_event->u.process_buf_data.isp_buf_divert.lds_enabled;

  CPP_DBG("[CDS_LDS] identity %x,streamtype %d,uv upsmple %d, lds %d",
    hw_params->identity, hw_params->stream_type,
    hw_params->uv_upsample_enable, hw_params->lds_enable);
#endif
  CPP_BUF_DBG("identity %x,streamtype %d,uv upsmple %d",
    hw_params->identity, hw_params->stream_type,
    hw_params->uv_upsample_enable);

  stream_status = cpp_hardware_get_stream_status(ctrl->cpphw,
     hw_params->identity);
  if (stream_status) {
    stream_status->pending_buf_divert++;
    CPP_HIGH("identity %d pending_buf_divert %d", hw_params->identity,
      stream_status->pending_buf_divert);
  } else {
    CPP_ERR("invalid stream status, failed\n");
    return -EFAULT;
  }

  /* get stream parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, hw_params->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CPP_BUF_ERR("invalid stream params, failed\n");
    stream_status->pending_buf_divert--;
    return -EFAULT;
  }

  /* get the the duplicate stream params if duplicate output is enabled */
  if (hw_params->duplicate_output) {
    duplicate_stream_status = cpp_hardware_get_stream_status(ctrl->cpphw,
      hw_params->duplicate_identity);
    if (duplicate_stream_status) {
      duplicate_stream_status->pending_buf_divert++;
      CPP_ERR("VDEBUG: identity %d pending_buf_divert %d",hw_params->duplicate_identity,
        stream_status->pending_buf_divert);
    } else {
      CPP_BUF_ERR("invalid stream status, failed\n");
      return -EFAULT;
    }
    cpp_module_get_params_for_identity(ctrl, hw_params->duplicate_identity,
      &dup_session_params, &dup_stream_params);
  }

  if (stream_params->stream_type ==  CAM_STREAM_TYPE_VIDEO) {
    video_type_flag = 1;
  } else {
    video_type_flag = 0;
  }
  /*Before validation, swap dimensions if 90 or 270 degrees rotation*/
  cpp_hardware_rotation_swap(hw_params,video_type_flag);

  /* before giving the frame to hw, make sure the parameters are good */
  if(FALSE == cpp_hardware_validate_params(hw_params))
  {
    free(cookie);
    CPP_BUF_ERR("hw_params invalid, dropping frame.");
    stream_status->pending_buf_divert--;
    if (hw_params->duplicate_output) {
     duplicate_stream_status->pending_buf_divert--;
    }
    return cpp_module_do_ack(ctrl, cpp_event->ack_key, buffer_access);
  }

  rc = cpp_params_calculate_crop(hw_params);
  if (rc < 0) {
    free(cookie);
    CPP_BUF_ERR("Crop info invalid, dropping frame\n");
    stream_status->pending_buf_divert--;
    if (hw_params->duplicate_output) {
     duplicate_stream_status->pending_buf_divert--;
    }
    return cpp_module_do_ack(ctrl, cpp_event->ack_key, buffer_access);
  }

  if (stream_params->stream_type ==  CAM_STREAM_TYPE_OFFLINE_PROC) {
    hw_params->camif_dim.width = session_params->camif_dim.width;
    hw_params->camif_dim.height = session_params->camif_dim.height;
  }

  rc = cpp_thread_update_filters(ctrl, hw_params);
  cpp_module_util_post_crop_info(ctrl->p_module, hw_params, stream_params);

  #ifdef ASF_OSD
  /*Update session param value since it will be consumed in sof_notify which will be in session
   stream  for OSD feature*/
  cpp_module_util_update_asf_region(session_params, stream_params, hw_params);
  #endif

  if (stream_params->stream_info->streaming_mode == CAM_STREAMING_MODE_BATCH ||
    (dup_stream_params &&
    dup_stream_params->stream_info->streaming_mode ==
    CAM_STREAMING_MODE_BATCH)) {
    hw_params->batch_info.batch_mode =
      cpp_event->u.process_buf_data.isp_buf_divert.batch_info.batch_mode;
    if (cpp_event->u.process_buf_data.isp_buf_divert.batch_info.batch_mode) {
      uint32_t i;
      hw_params->batch_info.batch_size =
        cpp_event->u.process_buf_data.isp_buf_divert.batch_info.batch_size;
      for (i = 0; i < MAX_PLANES; i++) {
        hw_params->batch_info.intra_plane_offset[i] =
          cpp_event->u.process_buf_data.
            isp_buf_divert.batch_info.intra_plane_offset[i];
      }
      hw_params->batch_info.pick_preview_idx =
        cpp_event->u.process_buf_data.
          isp_buf_divert.batch_info.pick_preview_idx;
    }
  }

  buffer_access = cpp_event->u.process_buf_data.isp_buf_divert.buffer_access;
  // Update the latest buffer_access flags
  hw_params->buffer_info.buffer_access = buffer_access;

  if (cookie->proc_div_required && stream_params->native_buff > 0) {
    mct_list_t *out_buf_list = NULL;
    pp_frame_buffer_t *img_buf;
    int32_t rc2 = 0;

    rc2 = pp_native_manager_get_bufs(&ctrl->pp_buf_mgr, 1,
      CPP_GET_SESSION_ID(hw_params->output_buffer_info.identity),
      CPP_GET_STREAM_ID(hw_params->output_buffer_info.identity), &out_buf_list);
    if ((rc2 < 0) || (out_buf_list == NULL) || out_buf_list->data == NULL) {
      CPP_ERR("failed to get native buffers for 0x%x",
        hw_params->output_buffer_info.identity);
      free(cookie);
      cpp_module_notify_frame_drop(ctrl->p_module,
        hw_params->output_buffer_info.identity, hw_params->frame_id);
      stream_status->pending_buf_divert--;
      if (hw_params->duplicate_output) {
        duplicate_stream_status->pending_buf_divert--;
      }
      return cpp_module_do_ack(ctrl, cpp_event->ack_key, buffer_access);
    }

    img_buf = (pp_frame_buffer_t*) out_buf_list->data;
    hw_params->output_buffer_info.fd = (unsigned long)img_buf->fd;
    hw_params->output_buffer_info.index = img_buf->buffer.index;
    hw_params->output_buffer_info.native_buff = TRUE;
    hw_params->output_buffer_info.offset = 0;
    hw_params->output_buffer_info.processed_divert = TRUE;
    hw_params->output_buffer_info.vaddr = img_buf->vaddr;
    hw_params->output_buffer_info.alloc_len = img_buf->ion_alloc[0].len;

    mct_list_free_list(out_buf_list);
  }
  CPP_LOW("frameid %d hw_params fd %d, index 0x%x, offset %d, native_buff %d,"
    "alloc_len %d,vaddr %p, identity 0x%x, processed_divert %d",
    hw_params->frame_id,
    hw_params->output_buffer_info.fd,
    hw_params->output_buffer_info.index,
    hw_params->output_buffer_info.offset,
    hw_params->output_buffer_info.native_buff,
    hw_params->output_buffer_info.alloc_len,
    hw_params->output_buffer_info.vaddr,
    hw_params->output_buffer_info.identity,
    hw_params->output_buffer_info.processed_divert);


  cmd.type = CPP_HW_CMD_PROCESS_FRAME;
  cmd.ctrl = ctrl;
  cmd.u.hw_params = hw_params;
#ifdef _ANDROID_
  if (session_params->cpp_debug_enable)
    cpp_thread_dump_frame(hw_params,hw_params->buffer_info.fd,&cmd,1);
#endif
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if (rc < 0) {
    free(cookie);
    if (stream_params->native_buff > 0) {
      int32_t rc1;
      CPP_ERR("process frame failed,frame %d identity %x",
        hw_params->frame_id, hw_params->output_buffer_info.identity);
      rc1 = pp_native_manager_put_buf((pp_native_buf_mgr_t *)&(ctrl->pp_buf_mgr),
        CPP_GET_SESSION_ID(stream_params->hw_params.output_buffer_info.identity),
        CPP_GET_STREAM_ID(stream_params->hw_params.output_buffer_info.identity),
        hw_params->output_buffer_info.index);
      if(rc1 < 0)
        CPP_ERR("put native buf failed");
    }
    switch (rc) {
    case -EAGAIN:
      if (!stream_params->hw_params.drop_count) {
        CPP_BUF_ERR("dropped frame id=%d identity=0x%x, HAL buffer not enqueued",
          hw_params->frame_id, hw_params->identity);
      }
      stream_params->hw_params.drop_count++;
      stream_status->pending_buf_divert--;
      if (hw_params->duplicate_output) {
        duplicate_stream_status->pending_buf_divert--;
      }
      return cpp_module_do_ack(ctrl, cpp_event->ack_key, buffer_access);
    case -CPP_ERR_PROCESS_FRAME_ERROR:
      CPP_ERR("process frame error id=%d identity=0x%x,",
        hw_params->frame_id, hw_params->identity);
      stream_status->pending_buf_divert--;
      if (hw_params->duplicate_output) {
        duplicate_stream_status->pending_buf_divert--;
      }
      return cpp_module_do_ack(ctrl, cpp_event->ack_key, buffer_access);
    case -EFAULT:
      CPP_ERR("drop frame, page fault occurred error id=%d identity=0x%x,",
          hw_params->frame_id, hw_params->identity);
      if (!ctrl->is_hw_error) {
        ctrl->is_hw_error = true;
        CPP_ERR("is_hw_error %d rc %d", ctrl->is_hw_error, rc);
      }
      stream_status->pending_buf_divert--;
      if (hw_params->duplicate_output) {
        duplicate_stream_status->pending_buf_divert--;
      }
      return cpp_module_do_ack(ctrl, cpp_event->ack_key, buffer_access);
    default:
      cpp_module_do_ack(ctrl, cpp_event->ack_key, buffer_access);
      if (!ctrl->is_hw_error) {
        ctrl->is_hw_error = true;
        CPP_ERR("is_hw_error %d rc %d", ctrl->is_hw_error, rc);
      }
      cpp_module_util_post_error_to_bus(ctrl->p_module, hw_params->identity);
      rc = 0;
    }
  }
  if (stream_params->hw_params.drop_count > 1) {
    CPP_BUF_ERR("Dropped %d frames for identity %x, HAL buffer not enqueued",
      stream_params->hw_params.drop_count, hw_params->identity);
    stream_params->hw_params.drop_count = 0;
  }

  stream_status->pending_buf_divert--;
  if (hw_params->duplicate_output) {
    duplicate_stream_status->pending_buf_divert--;
  }
  /* Update and post the current session's diag parameters */
  cpp_module_util_update_session_diag_params(ctrl->p_module, hw_params);
  return rc;
}

/** cpp_thread_handle_clock_event:
 *
 *  @ctrl - CPP control structure.
 *  @scpp_event - CPP event.
 *
 *  This function is called when clock event comes into thread. It sends
 *  clock command to kernel to set the clock according to the new load.
 *
 *  Return: It returns 0 on success.
 **/
static int32_t cpp_thread_handle_clock_event(cpp_module_ctrl_t* ctrl,
  cpp_module_event_t* cpp_event)
{
  int32_t rc = 0;
  cpp_hardware_cmd_t cmd;

  cmd.u.clock_settings.clock_rate = cpp_event->u.clock_data.clk_rate;
  cmd.u.clock_settings.avg = cpp_event->u.clock_data.bandwidth_avg;
  cmd.u.clock_settings.inst = cpp_event->u.clock_data.bandwidth_inst;
  cmd.type = CPP_HW_CMD_SET_CLK;
  /* The new stream needs higher clock */
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);

  if(rc < 0) {
    CPP_ERR("failed %d\n", rc);
    return rc;
  }

  ctrl->clk_rate = cpp_event->u.clock_data.clk_rate;

  return rc;
}


/* cpp_thread_get_event_from_queue
 *
 * Description:
 * - dq event from the queue based on priority. if there is any event in
 *   realtime queue, return it. Only when there is nothing in realtime queue,
 *   get event from offline queue.
 * - Get hardware related event only if the hardware is ready to process.
 **/
static cpp_module_event_t* cpp_thread_get_event_from_queue(
  cpp_module_ctrl_t *ctrl)
{
  if(!ctrl) {
    CPP_ERR("invalid cpp control, failed");
    return NULL;
  }
  cpp_module_event_t *cpp_event;
  /* TODO: see if this hardware related logic is suitable in this function
     or need to put it somewhere else */
  if (cpp_hardware_get_status(ctrl->cpphw) == CPP_HW_STATUS_IDLE) {
    PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->realtime_queue) == FALSE) {
      cpp_event = (cpp_module_event_t *)
                  mct_queue_pop_head(ctrl->realtime_queue);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
      return cpp_event;
    }
    if(MCT_QUEUE_IS_EMPTY(ctrl->partial_frame_queue) == FALSE) {
      cpp_event = (cpp_module_event_t *)
                  mct_queue_pop_head(ctrl->partial_frame_queue);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
      return cpp_event;
    }
    if(MCT_QUEUE_IS_EMPTY(ctrl->offline_queue) == FALSE) {
      cpp_event = (cpp_module_event_t *)
                  mct_queue_pop_head(ctrl->offline_queue);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
      return cpp_event;
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));

  } else if(cpp_hardware_get_status(ctrl->cpphw) == CPP_HW_STATUS_READY) {
    PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->realtime_queue) == FALSE) {
       cpp_event = (cpp_module_event_t *)
                     mct_queue_look_at_head(ctrl->realtime_queue);
       if(!cpp_event) {
           CPP_ERR("invalid cpp event, failed");
           PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
           return NULL;
       }
       if ((cpp_event->type == CPP_MODULE_EVENT_CLOCK) &&
         (cpp_event->invalid != TRUE)) {
         CPP_DBG("Ignore CLOCK event with pending buf with hardware");
         PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
         return NULL;
       }
       cpp_event = (cpp_module_event_t *)
                   mct_queue_pop_head(ctrl->realtime_queue);
       PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
       return cpp_event;
    }
    if(MCT_QUEUE_IS_EMPTY(ctrl->partial_frame_queue) == FALSE) {
      cpp_event = (cpp_module_event_t *)
                  mct_queue_pop_head(ctrl->partial_frame_queue);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
      return cpp_event;
    }
    if(MCT_QUEUE_IS_EMPTY(ctrl->offline_queue) == FALSE) {
       cpp_event = (cpp_module_event_t *)
                   mct_queue_pop_head(ctrl->offline_queue);
       PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
       return cpp_event;
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));

  }  else {
    PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->realtime_queue) == FALSE) {
      cpp_event = (cpp_module_event_t *)
                    mct_queue_look_at_head(ctrl->realtime_queue);
      if(!cpp_event) {
          CPP_ERR("invalid cpp event, failed");
          PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
          return NULL;
      }
      if (cpp_event->hw_process_flag == FALSE) {
        cpp_event = (cpp_module_event_t *)
                      mct_queue_pop_head(ctrl->realtime_queue);
        PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
        return cpp_event;
      }
    }
    if(MCT_QUEUE_IS_EMPTY(ctrl->partial_frame_queue) == FALSE) {
      cpp_event = (cpp_module_event_t *)
                    mct_queue_look_at_head(ctrl->partial_frame_queue);
      if(!cpp_event) {
          CPP_ERR("invalid cpp event, failed");
          PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
          return NULL;
      }
      if (cpp_event->hw_process_flag == FALSE) {
        cpp_event = (cpp_module_event_t *)
                      mct_queue_pop_head(ctrl->partial_frame_queue);
        PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
        return cpp_event;
      }
    }
    if(MCT_QUEUE_IS_EMPTY(ctrl->offline_queue) == FALSE) {
      cpp_event = (cpp_module_event_t *)
                    mct_queue_look_at_head(ctrl->offline_queue);
      if(!cpp_event) {
          CPP_ERR("invalid cpp event, failed");
          PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
          return NULL;
      }
      if (cpp_event->hw_process_flag == FALSE) {
        cpp_event = (cpp_module_event_t *)
                      mct_queue_pop_head(ctrl->offline_queue);
        PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
        return cpp_event;
      }
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
  }
  return NULL;
}

/* cpp_thread_process_queue_event:
 *
 * Description:
 *
 **/
static int32_t cpp_thread_process_queue_event(cpp_module_ctrl_t *ctrl,
  cpp_module_event_t* cpp_event)
{
  int32_t rc = 0;
  if(!ctrl || !cpp_event) {
    CPP_ERR("failed, ctrl=%p, cpp_event=%p", ctrl, cpp_event);
    if(cpp_event) free(cpp_event);
    return -EINVAL;
  }
  /* if the event is invalid, no need to process, just free the memory */
  if ((cpp_event->invalid == TRUE) &&
    ((cpp_event->type != CPP_MODULE_EVENT_PARTIAL_FRAME) ||
    (!cpp_event->u.partial_frame.partial_stripe_info.last_payload))) {
    CPP_DBG("invalidated event received. %d", cpp_event->type);
    free(cpp_event);
    return 0;
  }
  switch(cpp_event->type) {
  case CPP_MODULE_EVENT_DIVERT_BUF:
    CPP_BUF_LOW("CPP_MODULE_EVENT_DIVERT_BUF");
    rc = cpp_thread_handle_divert_buf_event(ctrl, cpp_event);
    break;
  case CPP_MODULE_EVENT_PROCESS_BUF:
    CPP_BUF_LOW("CPP_MODULE_EVENT_PROCESS_BUF");
    rc = cpp_thread_handle_process_buf_event(ctrl, cpp_event);
    break;
  case CPP_MODULE_EVENT_CLOCK:
    CPP_LOW("CPP_MODULE_EVENT_CLOCK");
    rc = cpp_thread_handle_clock_event(ctrl, cpp_event);
    break;
  case CPP_MODULE_EVENT_PARTIAL_FRAME:
    CPP_BUF_LOW("CPP_MODULE_EVENT_PARTIAL_FRAME");
    rc = cpp_thread_handle_partial_frame_event(ctrl, cpp_event);
    break;
  case CPP_MODULE_EVENT_ISP_BUFFER_DROP:
    CPP_BUF_LOW("CPP_MODULE_EVENT_ISP_BUFFER_DROP");
    cpp_thread_handle_isp_drop_buffer_event(ctrl, cpp_event);
    break;
  default:
    CPP_ERR("failed, bad event type=%d", cpp_event->type);
    free(cpp_event);
    return -EINVAL;
  }
  /* free the event memory */
  free(cpp_event);
  if (rc < 0) {
    CPP_ERR("failed, rc=%d", rc);
  }
  return rc;
}

/* cpp_thread_process_pipe_message:
 *
 * Description:
 *
 **/
static int32_t cpp_thread_process_pipe_message(cpp_module_ctrl_t *ctrl,
  cpp_thread_msg_t msg)
{
  int32_t rc = 0;
  cpp_hardware_cmd_t cmd;
  switch(msg.type) {
  case CPP_THREAD_MSG_ABORT: {
    cpp_module_event_t* cpp_event;
    CPP_HIGH("CPP_THREAD_MSG_ABORT: cpp_thread exiting..");
    /* Check if there is pending clock event */
    while(1) {
      cpp_event = cpp_thread_get_event_from_queue(ctrl);
      if(!cpp_event) {
        break;
      }
      rc = cpp_thread_process_queue_event(ctrl, cpp_event);
      if(rc < 0) {
        CPP_ERR("cpp_thread_process_queue_event() failed");
      }
    }
    ctrl->cpp_thread_started = FALSE;
    cmd.type = CPP_HW_CMD_UNSUBSCRIBE_EVENT;
    cpp_hardware_process_command(ctrl->cpphw, cmd);
    pthread_exit(NULL);
  }
  case CPP_THREAD_MSG_NEW_EVENT_IN_Q: {
    CPP_LOW("CPP_THREAD_MSG_NEW_EVENT_IN_Q:");
    cpp_module_event_t* cpp_event;
    /* while there is some valid event in queue process it */
    while(1) {
      cpp_event = cpp_thread_get_event_from_queue(ctrl);
      if(!cpp_event) {
        break;
      }
      rc = cpp_thread_process_queue_event(ctrl, cpp_event);
      if(rc < 0) {
        CPP_ERR("cpp_thread_process_queue_event() failed");
      }
    }
    break;
  }
  default:
    CPP_ERR("error: bad msg type=%d", msg.type);
    return -EINVAL;
  }
  return rc;
}

/* cpp_thread_send_processed_divert:
 *
 * Description:
 *
 **/
static int32_t cpp_thread_send_processed_divert(cpp_module_ctrl_t *ctrl,
  isp_buf_divert_t *buf_divert, uint32_t event_identity)
{
  if(!ctrl || !buf_divert) {
    CPP_ERR("failed ctrl:%p, buf_divert:%p\n", ctrl, buf_divert);
    return -EINVAL;
  }
  mct_event_t event;
  int32_t rc;
  memset(&event, 0x00, sizeof(mct_event_t));
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = event_identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
  event.u.module_event.module_event_data = (void *)buf_divert;

  rc = cpp_module_send_event_downstream(ctrl->p_module, &event);
  if(rc < 0) {
    CPP_ERR("failed %d", rc);
    return -EFAULT;
  }
  return 0;
}


/* cpp_thread_process_hardware_event:
 *
 * Description:
 *
 **/
static int32_t cpp_thread_process_hardware_event(cpp_module_ctrl_t *ctrl)
{
  int32_t rc = 0;
  cpp_hardware_cmd_t cmd;
  cpp_hardware_event_data_t event_data;
  cpp_module_event_t *cpp_event = NULL;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  bool buf_done_flag = false;
  isp_buf_divert_t buf_divert;
  struct v4l2_plane plane;
  cpp_module_ack_key_t key;
  cpp_module_hw_cookie_t *cookie = NULL;
  uint32_t frame_done_iden = 0;
  uint32_t buffer_access;

  memset(&buf_divert, 0, sizeof(buf_divert));
  memset(&plane, 0, sizeof(plane));
  memset(&event_data, 0, sizeof(event_data));

  /* get the event data from hardware */
  cmd.type = CPP_HW_CMD_NOTIFY_EVENT;
  cmd.u.event_data = &event_data;
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if(rc < 0) {
    CPP_ERR("failed %d", rc);
    cpp_thread_fatal_exit(ctrl, TRUE);
  }

  /* get stream parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, event_data.identity,
    &session_params, &stream_params);

  ATRACE_INT_SNPRINTF(0, 40, "Camera:CPP type %d", stream_params->stream_type);

  CPP_BUF_DBG("cpp frame done, frame_id=%d, buf_idx=%d, identity=0x%x, out_idx:0x%x",
    event_data.frame_id, event_data.buf_idx,
    event_data.identity, event_data.out_buf_idx);

  if (event_data.do_ack == FALSE)
    goto CPP_THREAD_GET_NEXT_EVENT_FROM_QUEUE;

  /* Use cookie which has buffer identity now.*/
  if(!event_data.cookie) {
    CPP_ERR("failed. cookie=NULL\n");
    return -EFAULT;
  }

  /* update the pending ack for this buffer */
  cookie = (cpp_module_hw_cookie_t *)event_data.cookie;
  CPP_BUF_LOW("proc_div_req=%d, proc_div_identity=0x%x",
    cookie->proc_div_required, cookie->proc_div_identity);

  buffer_access = event_data.input_buffer_access;
  rc = cpp_module_do_ack(ctrl, cookie->key, buffer_access);
  if(rc < 0) {
    CPP_BUF_ERR("failed, buf_idx=%d, identity=0x%x",
      event_data.buf_idx, event_data.identity);
    return rc;
  }

  if(!stream_params) {
    CPP_BUF_ERR("invalid stream params, failed\n");
    return -EFAULT;
  }
  frame_done_iden = event_data.identity;

  cmd.ctrl = ctrl;
#ifdef _ANDROID_
  if (session_params->cpp_debug_enable)
    cpp_thread_dump_frame(&stream_params->hw_params,event_data.out_fd,&cmd,0);
#endif

  /* If processed divert is enabled send the processed buffer downstream */
  if (!cookie->proc_div_required) {
    CPP_BUF_LOW("done processing buffer, no divert needed");
    buf_done_flag = true;
  } else {
    /* Fill the paramters for buffer divert structure for downstream module */
    if (cookie->proc_div_identity == event_data.dup_identity) {
     event_data.out_buf_idx = event_data.dup_out_buf_idx;
     event_data.out_fd = (unsigned long) event_data.dup_out_fd;
     event_data.identity = event_data.dup_identity;
    }
    if (stream_params->is_stream_on) {
      buf_divert.buffer.sequence = event_data.frame_id;
      buf_divert.buffer.index = event_data.out_buf_idx;
      buf_divert.buffer.timestamp = event_data.timestamp;
      buf_divert.buffer.m.planes = &plane;
      buf_divert.buffer.m.planes[0].m.userptr =
        (unsigned long)event_data.out_fd;
      buf_divert.native_buf = event_data.out_native_buff;
      buf_divert.identity = event_data.identity;
      buf_divert.is_buf_dirty = 0;
      buf_divert.ack_flag = 1;
      buf_divert.meta_data = cookie->meta_datas;
      buf_divert.buffer_access = event_data.output_buffer_access;

      if (event_data.out_native_buff) {
        pp_frame_buffer_t* pp_buf = pp_native_buf_manage_get_buf_info(
          &ctrl->pp_buf_mgr,
          CPP_GET_SESSION_ID(stream_params->hw_params.output_buffer_info.identity),
          CPP_GET_STREAM_ID(stream_params->hw_params.output_buffer_info.identity),
          event_data.out_buf_idx);
        if (!pp_buf) {
          CPP_BUF_ERR("failed, idx=%d, native_identity=%x %p",
            event_data.out_buf_idx,
            stream_params->hw_params.output_buffer_info.identity,
            &(ctrl->pp_buf_mgr));
          return -1;
        }

        buf_divert.vaddr = &pp_buf->vaddr;
        buf_divert.plane_vaddr[0] = (uint32_t)pp_buf->vaddr;
        buf_divert.buffer.length = pp_buf->ion_alloc[0].len;
        buf_divert.fd = pp_buf->fd;
        buf_divert.buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        buf_divert.buffer.memory = V4L2_MEMORY_USERPTR;
        buf_divert.is_uv_subsampled = stream_params->hw_params.uv_upsample_enable;

        buf_divert.identity = stream_params->hw_params.output_buffer_info.identity;
        CPP_LOW("proc_div_identity:0x%x, dup_identity:0x%x,"
                 "out_fd:%d, fd:%d plane_vaddr[0]:0x%x vaddr[0]:0x%x",
                 cookie->proc_div_identity, event_data.dup_identity,
                 event_data.out_fd,pp_buf->fd,(uint32_t)buf_divert.vaddr,
                 &pp_buf->vaddr);
      }

      CPP_BUF_LOW("identity %x, frame id %d",
        buf_divert.identity, event_data.frame_id);

      rc = cpp_thread_send_processed_divert(ctrl, &buf_divert,
        stream_params->identity);
      if(rc < 0) {
        CPP_BUF_ERR("failed processed divert %d\n", rc);
        rc = 0;
      }

      /* Release this processed divert buffer in kernel if downstream module is
         giving piggy-back ack */
      if (buf_divert.ack_flag == 1) {
        // piggy back ack.
        // we need to pass the updated buffer_access flag from downstream
        // modules to kernel while Q_BUF.
        // Update the latest buffer_access flag
        event_data.output_buffer_access = buf_divert.buffer_access;
        event_data.is_buf_dirty = buf_divert.is_buf_dirty;

        CPP_LOW("stream_params->native_buff is %d",stream_params->native_buff);
        if (stream_params->native_buff > 0) {
          rc = pp_native_manager_put_buf((pp_native_buf_mgr_t *)&(ctrl->pp_buf_mgr),
            CPP_GET_SESSION_ID(stream_params->hw_params.output_buffer_info.identity),
            CPP_GET_STREAM_ID(stream_params->hw_params.output_buffer_info.identity),
            buf_divert.buffer.index);
          if(rc < 0)
            CPP_ERR("put native buf failed");
          } else {
            cmd.type = CPP_HW_CMD_QUEUE_BUF;
            cmd.u.event_data = &event_data;
            rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
            if(rc < 0) {
              CPP_BUF_ERR("failed %d\n", rc);
              free(cookie);
              cpp_thread_fatal_exit(ctrl, TRUE);
           }
        }
        buf_done_flag = true;
        if (cookie->proc_div_identity == event_data.dup_identity) {
          cmd.type = CPP_HW_CMD_NOTIFY_BUF_DONE;
          cmd.u.buf_done_identity = event_data.identity;
          rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
          if (rc < 0) {
            CPP_BUF_ERR("failed %d", rc);
            free(cookie);
            cpp_thread_fatal_exit(ctrl, TRUE);
          }
        }
      }
    } else if (!event_data.out_native_buff) {
      CPP_BUF_DBG("stream is already off, dirty put buffer\n");
      /*if no divert, need to mark as dirty and do put buf directly*/
      event_data.is_buf_dirty = 1;
      cmd.type = CPP_HW_CMD_QUEUE_BUF;
      cmd.u.event_data = &event_data;
      // event_data is already having the latest output buffer_access,
      // no need to do anything here.
      rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
      if(rc < 0) {
        CPP_BUF_ERR("failed %d\n", rc);
        free(cookie);
        cpp_thread_fatal_exit(ctrl, TRUE);
      }
      buf_done_flag = true;
    } else {
      /* Native buffer and stream_status OFF */
      buf_done_flag = true;
    }
  }

  if (ctrl->tnr_module_func_tbl.private) {
    if (event_data.batch_size % 2) {
      ctrl->tnr_module_func_tbl.private(ctrl, event_data.identity);
    }
  }

  if (buf_done_flag) {
    cmd.type = CPP_HW_CMD_NOTIFY_BUF_DONE;
    cmd.u.buf_done_identity = frame_done_iden;
    rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
    if (rc < 0) {
      CPP_BUF_ERR("failed %d", rc);
      free(cookie);
      cpp_thread_fatal_exit(ctrl, TRUE);
    }
  }
  free(cookie);

CPP_THREAD_GET_NEXT_EVENT_FROM_QUEUE:
  /* if there is any pending valid event in queue, process it */
  while(1) {
    cpp_event = cpp_thread_get_event_from_queue(ctrl);
    if(!cpp_event) {
      break;
    }
    rc = cpp_thread_process_queue_event(ctrl, cpp_event);
    if(rc < 0) {
      CPP_ERR("cpp_thread_process_queue_event() failed");
      break;
    }
  }

  return rc;
}

/* cpp_thread_fatal_exit:
 *
 * Description:
 *
 **/
void cpp_thread_fatal_exit(cpp_module_ctrl_t *ctrl, boolean post_to_bus)
{
  cpp_hardware_cmd_t cmd;
  uint32_t i;
  mct_module_t *parent_mod;
  mct_bus_msg_t bus_msg;
  uint32_t session_id = 0;

  CPP_ERR(" fatal error: killing cpp_thread....!");
  cmd.type = CPP_HW_CMD_UNSUBSCRIBE_EVENT;
  cpp_hardware_process_command(ctrl->cpphw, cmd);

  PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
  ctrl->cpp_thread_started = FALSE;
  PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));

  /*
   * Pick the first valid session and post bus message.
   * If session is not created, error will be returned
   * on incoming commands as thread is not started
   */
  for(i = 0; i < CPP_MODULE_MAX_SESSIONS; i++) {
    if (ctrl->session_params[i])
      session_id = ctrl->session_params[i]->session_id;
  }
  parent_mod = ctrl->parent_module;
  if (post_to_bus && (parent_mod != NULL) &&
    (session_id != 0)) {
    CPP_ERR("posting error to MCT BUS!");
    if (!ctrl->is_hw_error) {
      ctrl->is_hw_error = true;
      CPP_ERR("is_hw_err %d", ctrl->is_hw_error);
    }
    memset(&bus_msg, 0x0, sizeof(mct_bus_msg_t));
    bus_msg.type = MCT_BUS_MSG_SEND_HW_ERROR;
    bus_msg.sessionid =  session_id;
    mct_module_post_bus_msg(parent_mod, &bus_msg);
  }
  pthread_exit(NULL);
}

/* cpp_thread_create:
 *
 * Description:
 *
 **/
int32_t cpp_thread_create(mct_module_t *module)
{
  int32_t rc;
 struct timespec timeout;

  if(!module) {
    CPP_ERR("invalid module, failed");
    return -EINVAL;
  }
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(ctrl->cpp_thread_started == TRUE) {
    CPP_ERR("failed, thread already started, "
      "can't create the thread again!");
    return -EFAULT;
  }
  PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
  ctrl->cpp_thread_started = FALSE;
  rc = pthread_create(&(ctrl->cpp_thread), NULL, cpp_thread_func, module);
  pthread_setname_np(ctrl->cpp_thread, "CAM_cpp");
  if(rc < 0) {
    CPP_ERR("pthread_create() failed, rc= ");
    PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
    return rc;
  }
  /* wait to confirm if the thread is started */
  while(ctrl->cpp_thread_started == FALSE) {
    memset(&timeout, 0, sizeof(timeout));
    PTHREAD_COND_WAIT_TIME(&(ctrl->th_start_cond), &(ctrl->cpp_mutex),
      &timeout, CPP_WAIT_TIMEOUT, rc);
    if (rc == ETIMEDOUT || rc == EINVAL) {
      CPP_ERR("thread create failed!!!  errno= %d \
        (ETIMEOUT %d, INVALID %d)\n", rc, ETIMEDOUT, EINVAL);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
      return -EFAULT;
    }
  }

  PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
  return 0;
}

/* cpp_thread_handle_partial_frame_event:
 *
 * Description:
 *
 *
 **/
static int32_t cpp_thread_handle_partial_frame_event(cpp_module_ctrl_t* ctrl,
  cpp_module_event_t* cpp_event)
{
  int32_t rc;
  unsigned long in_frame_fd;
  mct_event_t event;
  cpp_hardware_cmd_t cmd;
  cpp_module_hw_cookie_t *cookie;
  cpp_hardware_event_data_t event_data;
  uint32_t buffer_access;

  if(!ctrl || !cpp_event) {
    CPP_ERR("failed, ctrl=%p, cpp_event=%p", ctrl, cpp_event);
    return -EINVAL;
  }

  if ((cpp_event->invalid == TRUE) &&
    (cpp_event->u.partial_frame.partial_stripe_info.last_payload == TRUE)) {
    cpp_hardware_stream_status_t *stream_status;
    struct msm_cpp_frame_info_t *partial_frame =
      cpp_event->u.partial_frame.frame;
    cmd.type = CPP_HW_CMD_QUEUE_BUF;
    cmd.u.event_data = &event_data;
    memset(&event_data, 0, sizeof(event_data));
    event_data.identity = partial_frame->identity;
    event_data.out_buf_idx = partial_frame->output_buffer_info[0].index;
    event_data.timestamp = partial_frame->timestamp;
    event_data.frame_id = partial_frame->frame_id;
    event_data.is_buf_dirty =
      (partial_frame->frame_type != MSM_CPP_OFFLINE_FRAME) ? TRUE : FALSE;
#ifdef CACHE_PHASE2
    event_data.output_buffer_access =
      partial_frame->output_buffer_info[0].buffer_access;
#else
    event_data.output_buffer_access = 0;
#endif
    rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
    if(rc < 0) {
      CPP_BUF_ERR("failed %d\n", rc);
    }

#ifdef CACHE_PHASE2
    buffer_access = cmd.u.partial_frame->input_buffer_info.buffer_access;
#else
    buffer_access = 0;
#endif

   rc = cpp_module_do_ack(ctrl, cpp_event->ack_key, buffer_access);
   if (rc < 0) {
     CPP_ERR("do_ack failed %d", rc);
     goto error;
   }
    stream_status = cpp_hardware_get_stream_status(ctrl->cpphw,
      partial_frame->identity);
    if (stream_status) {
      stream_status->pending_partial_frame--;
      stream_status->hold_stream_off_op = true;
      CPP_BUF_DBG("cpp_event->invalid %d pending_partial_frame %d",
        cpp_event->invalid, stream_status->pending_partial_frame);
    }
    cmd.type = CPP_HW_CMD_NOTIFY_BUF_DONE;
    cmd.u.buf_done_identity = event_data.identity;
    rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
    if (rc < 0) {
      CPP_BUF_ERR("failed %d", rc);
    }
  error:
    free(partial_frame->cpp_cmd_msg);
    free(partial_frame);
    return rc;
  }

  cmd.type = CPP_HW_CMD_PROCESS_PARTIAL_FRAME;
  cmd.u.partial_frame = cpp_event->u.partial_frame.frame;
  cmd.u.partial_frame->first_stripe_index =
    cpp_event->u.partial_frame.partial_stripe_info.first_stripe_index;
  cmd.u.partial_frame->last_stripe_index =
    cpp_event->u.partial_frame.partial_stripe_info.last_stripe_index;
  cmd.u.partial_frame->stripe_info =
    cpp_event->u.partial_frame.partial_stripe_info.stripe_info;
  cmd.u.partial_frame->first_payload =
    cpp_event->u.partial_frame.partial_stripe_info.first_payload;
  cmd.u.partial_frame->last_payload =
    cpp_event->u.partial_frame.partial_stripe_info.last_payload;
  cmd.u.partial_frame->partial_frame_indicator = 1;
  cmd.u.partial_frame->cpp_cmd_msg[cmd.u.partial_frame->stripe_info_offset] =
    cmd.u.partial_frame->stripe_info;
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if (rc < 0) {
    // flush partial_frame_queue
    if (rc == -EAGAIN) {
      if (!cmd.u.partial_frame->last_payload) {
        /* Actual HW frame is already destroyed */
        cpp_module_flush_queue_events(ctrl, cmd.u.partial_frame->frame_id,
          cmd.u.partial_frame->identity, TRUE);
      } else {
        cpp_hardware_flush_frame(ctrl->cpphw, cmd.u.partial_frame);
      }
      free(cmd.u.partial_frame->cpp_cmd_msg);
      free(cmd.u.partial_frame);

#ifdef CACHE_PHASE2
      buffer_access = cmd.u.partial_frame->input_buffer_info.buffer_access;
#else
      buffer_access = 0;
#endif
      return cpp_module_do_ack(ctrl, cpp_event->ack_key, buffer_access);
    } else {
      if (!ctrl->is_hw_error) {
        ctrl->is_hw_error = true;
        CPP_ERR("is_hw_error %d rc %d", ctrl->is_hw_error, rc);
      }
      cpp_module_util_post_error_to_bus(ctrl->p_module, cmd.u.partial_frame->identity);
      rc = 0;
    }
  }

  /* Update and post the current session's diag parameters */
  //cpp_module_util_update_session_diag_params(ctrl->p_module, hw_params);
  return rc;
}

/* cpp_thread_handle_isp_drop_buffer_event:
 *
 * Description:
 *
 *
 **/
static int32_t cpp_thread_handle_isp_drop_buffer_event(cpp_module_ctrl_t* ctrl,
  cpp_module_event_t* cpp_event)
{
  int32_t rc;

  if(!ctrl || !cpp_event) {
    CPP_BUF_ERR("failed, ctrl=%p, cpp_event=%p", ctrl, cpp_event);
    return -EINVAL;
  }
  CPP_BUF_DBG("Going to request pop frameid = %d",
    cpp_event->u.drop_buffer.frame_id);
  rc = cpp_module_util_pop_buffer(ctrl,
    cpp_event->u.drop_buffer.stream_params,
    cpp_event->u.drop_buffer.frame_id);

  return rc;

}


