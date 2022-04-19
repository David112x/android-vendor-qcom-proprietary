/*============================================================================

 Copyright (c) 2014-2018 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "vpu_module.h"
#include "vpu_log.h"
#include "vpu_client.h"
#include "pp_buf_mgr.h"
#include <cutils/properties.h>

int32_t vpu_module_get_native_output_buf(vpu_module_ctrl_t *ctrl,
  vpu_module_stream_params_t *stream_params,
  struct v4l2_buffer *buffer)
{
  mct_list_t *buf_list=NULL;
  pp_frame_buffer_t *img_buf;
  int32_t recv_count;

  VPU_DBG("E: identity=%x", stream_params->native_identity);

  recv_count = pp_native_manager_get_bufs(&ctrl->native_buf_mgr, 1,
    VPU_GET_SESSION_ID(stream_params->native_identity),
    VPU_GET_STREAM_ID(stream_params->native_identity), &buf_list);
  if (recv_count != 1) {
    VPU_ERR("failed, recv_count=%d", recv_count);
    return -1;
  }

  if (!buf_list) {
    VPU_ERR("failed");
    return -1;
  } else if(!buf_list->data) {
    VPU_ERR("failed");
    return -1;
  }
  img_buf = (pp_frame_buffer_t*) buf_list->data;
  memcpy(buffer, &img_buf->buffer, sizeof(struct v4l2_buffer));
  int32_t offset=0;
  uint32_t i;

  VPU_DBG("len=%d, idx=%d", buffer->length, buffer->index);

  for (i=0; i < buffer->length && i < VIDEO_MAX_PLANES; i++) {
    buffer->m.planes[i].bytesused = buffer->m.planes[i].length;
    buffer->m.planes[i].reserved[0] = offset;
    offset += buffer->m.planes[i].length;
  }

  return 0;
}

int32_t vpu_module_get_output_buf(vpu_module_ctrl_t *ctrl,
  vpu_module_stream_params_t *stream_params,
  struct v4l2_buffer *buffer)
{
  uint32_t i;
  mct_stream_map_buf_t *map_buf;
  void * vaddr;

  map_buf = pp_buf_mgr_get_buf(ctrl->buf_mgr, &stream_params->stream_info);
  if (!map_buf) {
    VPU_ERR("pp_buf_mgr_get_buf() failed");
    return -1;
  }

  pp_buf_mgr_get_vaddr(ctrl->buf_mgr, map_buf->buf_index,
    &stream_params->stream_info, &vaddr);

  buffer->index = map_buf->buf_index;
  buffer->length = map_buf->num_planes;
  buffer->memory = V4L2_MEMORY_USERPTR;
  buffer->reserved = (uint32_t) vaddr; /* using reserved field for virtual addr */
  buffer->reserved2 = map_buf->buf_size;
  int32_t offset=0;
  for (i = 0; i < buffer->length && i < VIDEO_MAX_PLANES; i++) {
    buffer->m.planes[i].bytesused = map_buf->buf_planes[i].stride
        * map_buf->buf_planes[i].scanline;
    buffer->m.planes[i].length = map_buf->buf_planes[i].size;
    buffer->m.planes[i].m.fd = map_buf->buf_planes[i].fd;
    buffer->m.planes[i].reserved[0] = offset;
    offset += buffer->m.planes[i].length;
  }
  return 0;
}

int32_t vpu_module_get_input_buf(vpu_module_ctrl_t *ctrl,
  vpu_module_stream_params_t *stream_params,
  struct v4l2_buffer *buffer, isp_buf_divert_t *buf_div)
{
  uint32_t i;
  void* vaddr;

  buffer->index = buf_div->buffer.index;
  buffer->length = buf_div->buffer.length;
  buffer->memory = V4L2_MEMORY_USERPTR;
  buffer->timestamp = buf_div->buffer.timestamp;
  buffer->sequence = buf_div->buffer.sequence;
  int32_t offset = 0;

  if (buf_div->native_buf) {
    vaddr = buf_div->vaddr;
  } else {
    pp_buf_mgr_get_vaddr(ctrl->buf_mgr, buf_div->buffer.index,
      &stream_params->stream_info,  &vaddr);
  }

  for (i=0; i < buffer->length && i < VIDEO_MAX_PLANES; i++) {
    buffer->m.planes[i].bytesused = buffer->m.planes[i].length =
      stream_params->isp_output_info.buf_planes.plane_info.mp[i].stride *
        stream_params->isp_output_info.buf_planes.plane_info.mp[i].scanline;
    buffer->m.planes[i].m.fd = buf_div->buffer.m.planes[i].m.fd;
    buffer->m.planes[i].reserved[0] = offset;
    offset += buffer->m.planes[i].length;
  }
  return 0;
}

int32_t vpu_module_put_new_ack_in_list(vpu_module_ctrl_t *ctrl,
    isp_buf_divert_ack_t isp_ack)
{
  if (!ctrl) {
    VPU_ERR("failed");
    return -1;
  }
  vpu_module_ack_t *vpu_ack =
    (vpu_module_ack_t *) CAM_MALLOC (sizeof(vpu_module_ack_t));
  if (!vpu_ack) {
    VPU_ERR("CAM_MALLOC() failed");
    return -1;
  }
  memset(vpu_ack, 0x00, sizeof(vpu_module_ack_t));
  vpu_ack->isp_buf_divert_ack = isp_ack;

  PTHREAD_MUTEX_LOCK(&(ctrl->ack_list.mutex));
  ctrl->ack_list.list = mct_list_append(ctrl->ack_list.list, vpu_ack, NULL,
      NULL);
  ctrl->ack_list.size++;
  PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
  return 0;
}

int32_t vpu_module_dump_buffer_to_file(void *p, int32_t length, char *file_name)
{
  FILE *fp;
  int32_t w_count;
  VPU_HIGH("p=%p length=%d", p, length);
  fp = fopen(file_name, "w");
  if (!fp) {
    VPU_ERR("failed");
    return -1;
  }
  w_count = fwrite(p, 1, length, fp);
  if (w_count != length) {
    VPU_ERR("failed");
    fclose(fp);
    return -1;
  }
  VPU_HIGH("file written: %s", file_name);
  fclose(fp);
  return 0;
}

static bool has_video_stream(vpu_module_stream_params_t *stream_params)
{
  if (stream_params->stream_info.stream_type == CAM_STREAM_TYPE_VIDEO) {
    return true;
  }
  if (stream_params->linked_stream_params && stream_params->linked_stream_params
      ->stream_info.stream_type == CAM_STREAM_TYPE_VIDEO) {
    return true;
  }
  return false;
}

int32_t vpu_module_init_native_buffers(vpu_module_ctrl_t *ctrl,
  mct_stream_info_t *stream_info, uint32_t identity, int32_t buf_count)
{
  cam_frame_len_offset_t buf_info;
  int32_t rc;
  VPU_HIGH("E: identity=%x", identity);

  uint32_t i;
  memset(&buf_info, 0x00, sizeof(cam_frame_len_offset_t));
  buf_info.frame_len = 0;
  buf_info.num_planes = stream_info->buf_planes.plane_info.num_planes;
  for (i=0; i<stream_info->buf_planes.plane_info.num_planes; i++) {
    buf_info.mp[i].stride = stream_info->buf_planes.plane_info.mp[i].stride;
    buf_info.mp[i].scanline = stream_info->buf_planes.plane_info.mp[i].scanline;
    buf_info.mp[i].len = stream_info->buf_planes.plane_info.mp[i].stride *
      stream_info->buf_planes.plane_info.mp[i].scanline;
    buf_info.frame_len += buf_info.mp[i].len;
    VPU_DBG("plane %d: st=%d, sc=%d, len=%d", i, buf_info.mp[i].stride,
      buf_info.mp[i].scanline, buf_info.mp[i].len);
  }
  VPU_DBG("frame_len = %d", buf_info.frame_len);
  rc = pp_native_buf_mgr_allocate_buff(&ctrl->native_buf_mgr, buf_count,
    VPU_GET_SESSION_ID(identity), VPU_GET_STREAM_ID(identity),
    &buf_info, 0);
  if (rc < 0) {
    VPU_ERR("failed");
    return -1;
  }
  return 0;
}

static int32_t vpu_module_convert_img_fmt(cam_format_t cam_fmt, uint32_t *pix_fmt)
{
  switch (cam_fmt) {
    case CAM_FORMAT_YUV_420_NV12:
      *pix_fmt = V4L2_PIX_FMT_NV12; break;
    case CAM_FORMAT_YUV_420_NV21:
      *pix_fmt = V4L2_PIX_FMT_NV21; break;
    default:
      VPU_ERR("unsupported format %d", cam_fmt);
      return -1;
  }
  return 0;
}

int32_t vpu_module_save_stream_fmt(vpu_module_stream_params_t *stream_params,
  mct_stream_info_t *stream_info)
{
  struct v4l2_format *format;
  uint32_t pix_fmt=0;
  vpu_module_convert_img_fmt(stream_info->fmt, &pix_fmt);

  format = &stream_params->format;
  memset(format, 0x00, sizeof(struct v4l2_format));

  VPU_DBG("stream format for 0x%x", stream_info->identity);

  format->fmt.pix_mp.width = stream_info->dim.width;
  format->fmt.pix_mp.height = stream_info->dim.height;
  format->fmt.pix_mp.pixelformat = pix_fmt;
  format->fmt.pix_mp.num_planes =
    stream_info->buf_planes.plane_info.num_planes;

  VPU_DBG("w=%u, h=%u, pix_fmt=0x%x, num_planes=%u",
    format->fmt.pix_mp.width, format->fmt.pix_mp.height,
    format->fmt.pix_mp.pixelformat, format->fmt.pix_mp.num_planes);
  uint32_t i;
  for (i=0; i < stream_info->buf_planes.plane_info.num_planes
         && i < VIDEO_MAX_PLANES; i++) {
    format->fmt.pix_mp.plane_fmt[i].bytesperline =
       stream_info->buf_planes.plane_info.mp[i].stride;
    VPU_DBG("plane[%d]: bytesperline=%u", i,
      format->fmt.pix_mp.plane_fmt[i].bytesperline);
  }

  return 0;
}

static int32_t vpu_module_update_isp_dim(
  vpu_module_stream_params_t *stream_params, mct_stream_info_t *isp_out_info)
{
  int32_t rc;

  PTHREAD_MUTEX_LOCK(&stream_params->mutex);
  vpu_module_save_stream_fmt(stream_params, isp_out_info);
  stream_params->img_fmt_ready = true;
  PTHREAD_MUTEX_UNLOCK(&stream_params->mutex);

  VPU_DBG("stxsc=%dx%d, wxh=%dx%d, fmt=%d",
    isp_out_info->buf_planes.plane_info.mp[0].stride,
    isp_out_info->buf_planes.plane_info.mp[0].scanline,
    isp_out_info->dim.width, isp_out_info->dim.height, isp_out_info->fmt);
  VPU_DBG("plane0_len=%d, plane1_len=%d",
    isp_out_info->buf_planes.plane_info.mp[0].len,
    isp_out_info->buf_planes.plane_info.mp[1].len);

  memcpy(&stream_params->isp_output_info, isp_out_info,
      sizeof(mct_stream_info_t));

  return 0;
}

int32_t vpu_module_handle_isp_out_dim_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  vpu_module_ctrl_t *ctrl;
  uint32_t identity = event->identity;

  vpu_module_session_params_t *session_params;
  vpu_module_stream_params_t *stream_params;
  vpu_module_stream_params_t *linked_stream_params;
  mct_module_type_t           module_type;

  ctrl = (vpu_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);

  mct_stream_info_t *stream_info =
    (mct_stream_info_t *)(event->u.module_event.module_event_data);
  if(!stream_info) {
    VPU_ERR("failed");
    return -1;
  }
  rc = vpu_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if (!stream_params) {
    VPU_ERR("failed");
    return -1;
  }

  /* if this identity is not related to video stream, ignore the event */
  if (false == has_video_stream(stream_params)) {
    rc = vpu_module_send_event_downstream(module, event);
    if (rc < 0) {
      VPU_ERR("failed");
      return -1;
    }
    return 0;
  }

  rc = vpu_module_update_isp_dim(stream_params, stream_info);
  if (rc < 0) {
    VPU_ERR("failed");
    return -1;
  }

  module_type = mct_module_find_type(module, identity);
  if ((module_type != MCT_MODULE_FLAG_SINK) &&
    (module_type != MCT_MODULE_FLAG_PEERLESS)){
    stream_params->is_native_buf = true;
  }
  rc = vpu_module_send_event_downstream(module, event);
  if (rc < 0) {
    VPU_ERR("failed");
    return -1;
  }

  return 0;
}

int32_t vpu_module_handle_iface_div_req_event(mct_module_t* module,
  mct_event_t* event)
{
  vpu_module_ctrl_t *ctrl;
  vpu_client_buf_req_t buf_req;
  pp_buf_divert_request_t *divert_request =
    (pp_buf_divert_request_t *)(event->u.module_event.module_event_data);
  divert_request->need_divert = TRUE;
  ctrl = (vpu_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  vpu_client_get_buf_requirements(ctrl->client, &buf_req);
  divert_request->buf_alignment = buf_req.stride_padding;
  return 0;
}

static int32_t vpu_module_set_tnr_enable_from_property(vpu_module_ctrl_t *ctrl)
{
  int32_t fe;
  char value[PROPERTY_VALUE_MAX];
  property_get("persist.vendor.camera.tnr.force_enable", value, "0");
  fe = atoi(value);
  if (!fe) {
    return 0;
    ctrl->bypass = false;
  }
  if (fe == 1) {
    vpu_client_set_tnr_enable(ctrl->client, true);
    VPU_HIGH("force enable");
    ctrl->bypass = false;
  } else if (fe == 2) {
    vpu_client_set_tnr_enable(ctrl->client, false);
    VPU_HIGH("force disable");
    ctrl->bypass = true;
  }
  return 0;
}

int32_t vpu_module_handle_buf_divert_event(mct_module_t* module,
    mct_event_t* event)
{
  int32_t rc;
  vpu_module_ctrl_t *ctrl;
  isp_buf_divert_t *buf_div;
  int32_t frame_id;
  uint32_t identity = event->identity;
  struct v4l2_buffer in_buf, out_buf;
  struct v4l2_plane in_planes[VIDEO_MAX_PLANES];
  struct v4l2_plane out_planes[VIDEO_MAX_PLANES];
  mct_stream_map_buf_t *map_buf;
  isp_buf_divert_ack_t isp_ack;
  vpu_module_session_params_t *session_params;
  vpu_module_stream_params_t *stream_params;
  vpu_module_stream_params_t *linked_stream_params;

  memset(&in_buf, 0x00, sizeof(struct v4l2_buffer));
  memset(&out_buf, 0x00, sizeof(struct v4l2_buffer));
  memset(in_planes, 0x00, VIDEO_MAX_PLANES * sizeof(struct v4l2_plane));
  memset(out_planes, 0x00, VIDEO_MAX_PLANES * sizeof(struct v4l2_plane));
  in_buf.m.planes = in_planes;
  out_buf.m.planes = out_planes;

  ctrl = (vpu_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);

  if (ctrl->bypass == true) {
    VPU_DBG("vpu module bypass");
    rc = vpu_module_send_event_downstream(module, event);
    if (rc < 0) {
      VPU_ERR("failed");
      return -1;
    }
    return 0;
  }
  rc = vpu_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if (!stream_params) {
    VPU_ERR("failed");
    return -1;
  }

  buf_div = (isp_buf_divert_t * )(event->u.module_event.module_event_data);
  if (!buf_div) {
    VPU_ERR("failed");
    return -1;
  }

  /* if this identity is not related to video stream, ignore the event */
  if (false == has_video_stream(stream_params)) {
    rc = vpu_module_send_event_downstream(module, event);
    if (rc < 0) {
      VPU_ERR("failed");
      return -1;
    }
    return 0;
  }
  linked_stream_params = stream_params->linked_stream_params;
  bool is_streaming = stream_params->is_stream_on;

  if (linked_stream_params && linked_stream_params->is_stream_on) {
      is_streaming = true;
  }
  if (!is_streaming) {
    VPU_ERR("failed, no stream is on");
    return -1;
  }
  bool stream_off_requested;
  PTHREAD_MUTEX_LOCK(&stream_params->mutex);
  stream_off_requested = stream_params->is_stream_off_requested;
  PTHREAD_MUTEX_UNLOCK(&stream_params->mutex);
  if (stream_off_requested) {
    VPU_DBG("stream off requested, doing piggy-back ACK");
    buf_div->ack_flag = true;
    buf_div->is_buf_dirty = 0;
    return 0;
  }

  ctrl->client_identity = identity;

  VPU_DBG("identity=%x, buf_div_identity=%x, frame_id=%d, native=%d",
    identity, buf_div->identity, buf_div->buffer.sequence, buf_div->native_buf);

  rc = vpu_module_get_input_buf(ctrl, stream_params, &in_buf, buf_div);
  if (rc < 0) {
    VPU_ERR("failed");
    return rc;
  }
  /* if isp output is native buf, we need to use native buf */
  if (stream_params->is_native_buf) {
    VPU_HIGH("getting native buf");
    rc = vpu_module_get_native_output_buf(ctrl, stream_params, &out_buf);
    if (rc < 0) {
      VPU_ERR("failed");
      return rc;
    }
    stream_params->is_native_buf = true;
  } else {
    rc = vpu_module_get_output_buf(ctrl, stream_params, &out_buf);
    if (rc < 0) {
      VPU_ERR("failed");
      return rc;
    }
  }

  memset(&isp_ack, 0x00, sizeof(isp_buf_divert_ack_t));

  /* before sending frame for processing, put corresponding ack in list */
  isp_ack.buf_idx = buf_div->buffer.index;
  isp_ack.frame_id = buf_div->buffer.sequence;
  isp_ack.channel_id = buf_div->channel_id;
  isp_ack.identity = identity;
  isp_ack.is_buf_dirty = 1;

  vpu_module_put_new_ack_in_list(ctrl, isp_ack);

  rc = vpu_client_sched_frame_for_processing(ctrl->client, &in_buf, &out_buf);
   if (rc < 0) {
    VPU_ERR("failed");
    return rc;
  }
  return 0;
}

int32_t vpu_module_handle_buf_divert_ack_event(mct_module_t* module,
    mct_event_t* event)
{
  int32_t rc;
  vpu_module_session_params_t *session_params;
  vpu_module_stream_params_t *stream_params;
  struct timeval tv;

  vpu_module_ctrl_t *ctrl = (vpu_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);

  if (ctrl->bypass == true) {
    VPU_DBG("vpu module bypass");
    rc = vpu_module_send_event_upstream(module, event);
    if (rc < 0) {
      VPU_ERR("failed");
      return -1;
    }
    return 0;
  }
  vpu_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if (!stream_params) {
    VPU_ERR("failed");
    return -1;
  }

  /* if this identity is not related to video stream, ignore the event */
  if (false == has_video_stream(stream_params)) {
    rc = vpu_module_send_event_upstream(module, event);
    if (rc < 0) {
      VPU_ERR("failed");
      return -1;
    }
    return 0;
  }
  isp_buf_divert_ack_t* buf_ack = (isp_buf_divert_ack_t*)
    (event->u.module_event.module_event_data);
  if (!buf_ack) {
    VPU_ERR("failed");
    return -1;
  }
  gettimeofday(&tv, NULL);

  if (stream_params->is_native_buf) {
    /* native-buf: always put it back */
    VPU_DBG("native_put_buf: identity=%x, buf_idx=%d",
       stream_params->native_identity, buf_ack->buf_idx);
    rc = pp_native_manager_put_buf(&ctrl->native_buf_mgr,
           session_params->session_id,
           VPU_GET_STREAM_ID(stream_params->native_identity),
           buf_ack->buf_idx);
    if (rc < 0) {
      VPU_ERR("failed");
      return -1;
    }
  } else {
    /* hal-buf: put_buf if dirty, buf_done if clean */
    if (buf_ack->is_buf_dirty) {
      rc = pp_buf_mgr_put_buf(ctrl->buf_mgr, event->identity, buf_ack->buf_idx,
            buf_ack->frame_id, tv);
      if (rc < 0) {
        VPU_ERR("failed");
        return -1;
      }
    } else {
      rc = pp_buf_mgr_buf_done(ctrl->buf_mgr, event->identity, buf_ack->buf_idx,
          buf_ack->frame_id, tv);
      if (rc < 0) {
        VPU_ERR("failed");
        return -1;
      }
    }
  }
  return 0;
}

int32_t vpu_module_handle_streamon_event(mct_module_t* module,
    mct_event_t* event)
{
  int32_t rc;
  uint32_t identity = event->identity;

  VPU_DBG("E : identity=0x%x", identity);
  mct_stream_info_t *streaminfo =
   (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
  vpu_module_ctrl_t *ctrl = (vpu_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);

  vpu_module_session_params_t *session_params;
  vpu_module_stream_params_t *stream_params;

  rc = vpu_module_get_params_for_identity(ctrl, identity,
      &session_params, &stream_params);
  if (rc < 0) {
    VPU_ERR("failed");
    return -1;
  }

  /* if this identity is not related to video stream, ignore the event */
  if (false == has_video_stream(stream_params)) {
    rc = vpu_module_send_event_downstream(module, event);
    if (rc < 0) {
      VPU_ERR("failed");
      return -1;
    }
    return 0;
  }

  PTHREAD_MUTEX_LOCK(&stream_params->mutex);
  stream_params->is_stream_off_requested = false;
  stream_params->stream_info.img_buffer_list = streaminfo->img_buffer_list;

  /* turn on the client only if linked stream is non-existent or off,
     otherwise client is already streaming buffers on linked stream */
  if (stream_params->linked_stream_params &&
       stream_params->linked_stream_params->is_stream_on) {
    VPU_DBG("linked stream %x is on",
        stream_params->linked_stream_params->identity);
    stream_params->native_identity =
      stream_params->linked_stream_params->identity;
  } else {
    if (stream_params->is_native_buf) {
      rc = vpu_module_init_native_buffers(ctrl, &stream_params->isp_output_info,
             identity, VPU_NUM_NATIVE_BUFFERS);
      if (rc < 0) {
        VPU_ERR("failed");
        PTHREAD_MUTEX_UNLOCK(&stream_params->mutex);
        return rc;
      }
      stream_params->native_identity = identity;
    }
    /* start streaming on client */
    if (!stream_params->img_fmt_ready) {
      VPU_ERR("failed, stream format not available");
      PTHREAD_MUTEX_UNLOCK(&stream_params->mutex);
      return -1;
    }
    vpu_client_set_format(ctrl->client, stream_params->format);
    vpu_client_init_streaming(ctrl->client,
      stream_params->stream_info.num_bufs);
    VPU_HIGH("num_bufs=%d", stream_params->stream_info.num_bufs);
    rc = vpu_client_stream_on(ctrl->client);
    if (rc < 0) {
      VPU_ERR("failed");
      PTHREAD_MUTEX_UNLOCK(&stream_params->mutex);
      return rc;
    }
    /* set tnr enable/disable using setprop overide */
    vpu_module_set_tnr_enable_from_property(ctrl);
  }
  stream_params->is_stream_on = true;
  PTHREAD_MUTEX_UNLOCK(&stream_params->mutex);
  rc = vpu_module_send_event_downstream(module, event);
  if (rc < 0) {
    VPU_ERR("failed");
    return rc;
  }
  VPU_DBG("done");
  return 0;
}

int32_t vpu_module_handle_streamoff_event(mct_module_t* module,
    mct_event_t* event)
{
  int32_t rc;
  uint32_t identity = event->identity;
  VPU_DBG("E : identity=0x%x", identity);

  vpu_module_ctrl_t *ctrl = (vpu_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);

  vpu_module_session_params_t *session_params;
  vpu_module_stream_params_t *stream_params;

  rc = vpu_module_get_params_for_identity(ctrl, identity,
      &session_params, &stream_params);
  if (rc < 0) {
    VPU_ERR("failed");
    return -1;
  }
  /* if this identity is not related to video stream, ignore the event */
  if (false == has_video_stream(stream_params)) {
    rc = vpu_module_send_event_downstream(module, event);
    if (rc < 0) {
      VPU_ERR("failed");
      return -1;
    }
    return 0;
  }

  /* turn of the client only if linked stream is non-existent or off,
     otherwise keep it running */
  if (stream_params->linked_stream_params &&
       stream_params->linked_stream_params->is_stream_on) {
    VPU_DBG("linked stream %x is on",
        stream_params->linked_stream_params->identity);
  } else {
    PTHREAD_MUTEX_LOCK(&stream_params->mutex);
    stream_params->is_stream_off_requested = true;
    PTHREAD_MUTEX_UNLOCK(&stream_params->mutex);
    rc = vpu_client_stream_off(ctrl->client);
    if (rc < 0) {
      VPU_ERR("failed");
      return rc;
    }
    if (stream_params->is_native_buf) {
      /* free native buffers */
      pp_native_buf_mgr_free_queue(&ctrl->native_buf_mgr,
        VPU_GET_SESSION_ID(stream_params->native_identity),
        VPU_GET_STREAM_ID(stream_params->native_identity));
    }
  }

  PTHREAD_MUTEX_LOCK(&stream_params->mutex);
  stream_params->is_stream_on = false;
  PTHREAD_MUTEX_UNLOCK(&stream_params->mutex);

  rc = vpu_module_send_event_downstream(module, event);
  if (rc < 0) {
    VPU_ERR("failed");
    return rc;
  }
  VPU_DBG("done");
  return 0;
}

/** vpu_module_handle_chromatix_ptr_event:
 *
 * Description:
 *   Handle the chromatix ptr event.
 **/
int32_t vpu_module_handle_chromatix_ptr_event(mct_module_t* module,
    mct_event_t* event)
{
  int32_t rc;
  modulesChromatix_t *modules_chromatix;

  vpu_module_ctrl_t *ctrl = (vpu_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if (!ctrl) {
    VPU_ERR("failed");
    return -1;
  }
  modules_chromatix = (modulesChromatix_t *) event->u.module_event
      .module_event_data;

  if (!modules_chromatix) {
    VPU_ERR("failed");
    return -1;
  }
  vpu_client_update_chromatix(ctrl->client,
      modules_chromatix->chromatixCppPtr);
  rc = vpu_module_send_event_downstream(module, event);
  if (rc < 0) {
    VPU_ERR("failed");
    return -1;
  }
  return 0;
}

/** vpu_module_handle_aec_update_event:
 *
 * Description:
 *
 */
int32_t vpu_module_handle_aec_update_event(mct_module_t* module,
    mct_event_t* event)
{
  int32_t rc;
  stats_update_t *stats_update;

  stats_update =
      (stats_update_t *)event->u.module_event.module_event_data;

  if (stats_update == NULL) {
    VPU_ERR("failed, invalid event");
    return -1;
  }

  vpu_module_ctrl_t *ctrl = (vpu_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if (!ctrl) {
    VPU_ERR("failed");
    return -1;
  }

  if (stats_update->flag & STATS_UPDATE_AEC) {
    rc = vpu_client_aec_update(ctrl->client, stats_update->aec_update.real_gain,
      stats_update->aec_update.lux_idx);
    if (rc < 0) {
      VPU_ERR("failed aec update");
    }
  }

  rc = vpu_module_send_event_downstream(module, event);
    if (rc < 0) {
      VPU_ERR("failed");
      return rc;
    }
  return 0;
}

/** vpu_module_handle_set_parm_event:
 *
 * Description:
 *   Handle the set_parm event.
 **/
int32_t vpu_module_handle_set_parm_event(mct_module_t* module,
    mct_event_t* event)
{
  int32_t rc;

  vpu_module_ctrl_t *ctrl = (vpu_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if (!ctrl) {
    VPU_ERR("failed");
    return -1;
  }

  mct_event_control_parm_t *ctrl_parm = (mct_event_control_parm_t *) event->u
      .ctrl_event.control_event_data;
  if (!ctrl_parm) {
    VPU_ERR("failed");
    return -1;
  }
  switch (ctrl_parm->type) {
/* todo: enable this after HAL paramter is implemented */
#if 0
    case CAM_INTF_PARM_VPU_TNR:
    vpu_client_ctrl_parm_t vpu_ctrl;
    vpu_client_set_ctrl_parm(ctrl->client, vpu_ctrl);
    break;
#endif
    default:
    break;
  }
  rc = vpu_module_send_event_downstream(module, event);
  if (rc < 0) {
    VPU_ERR("failed");
    return rc;
  }
  return 0;
}
