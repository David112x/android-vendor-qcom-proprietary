/*************************************************************
* Copyright (c) 2016-2017 Qualcomm Technologies, Inc.             *
* All Rights Reserved.                                       *
* Confidential and Proprietary - Qualcomm Technologies, Inc. *
*************************************************************/

#include "jpegdma_util_intf.h"
#include "jpegdma_util.h"
#include <media/msm_jpeg_dma.h>

/** jpegdma_util_frame_to_find_t
 *   @frameid: frame_id.
 *   @buff_type: buffer type.
 *
 *   Jpeg dma working thread message.
 */
typedef struct {
  uint32_t frameid;
  jpegdma_buffer_type_t buff_type;
} jpegdma_util_frame_to_find_t;

/**
 * Function: jpegdma_util_get_dev_name
 *
 * Description: Go thru each "/dev/videoN" and search for v4l2
 *   jpeg dma device name.
 *
 * Input parameters:
 *   @p_drv_name: Char array where jpeg dma device name
 *   should be stored.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
int32_t jpegdma_util_get_dev_name(char **p_drv_name)
{
  char temp_name[JDMA_UTIL_MAX_NAME_SIZE];
  int temp_fd;
  int num_device;
  int cnt = JDMA_UTIL_MAX_DEVICES;
  int ret = 0;
  int status = IMG_ERR_NOT_FOUND;

  if (p_drv_name == NULL) {
    IDBG_ERROR("Invalid input");
    return IMG_ERR_INVALID_INPUT;
  }

  *p_drv_name = calloc(1, sizeof(char) * JDMA_UTIL_MAX_NAME_SIZE);
  if (*p_drv_name == NULL) {
    IDBG_ERROR("Can not alloc mem for device node name");
    return IMG_ERR_NO_MEMORY;
  }

  num_device = -1;
  while (cnt-- > 0) {
    num_device++;

    ret = snprintf(temp_name, sizeof(temp_name),
      JDMA_UTIL_V4L2_SYS_PATH"video%d/name", num_device);
    if (ret <= 0) {
      IDBG_ERROR("Snprintf fail %s", strerror(errno));
      status = IMG_ERR_GENERAL;
      goto error;
    }

    temp_fd = open(temp_name, O_RDONLY | O_NONBLOCK);
    if (temp_fd < 0) {
      if (errno == ENOENT) {
        IDBG_ERROR("Can not discover JPEG DMA, name %s\n", temp_name);
        goto error;
      } else {
        IDBG_ERROR("%s %s Skip\n", strerror(errno), temp_name);
        continue;
      }
    }

    ret = read(temp_fd, temp_name, sizeof(JDMA_UTIL_V4l2_DRV_NAME));
    close(temp_fd);
    if (ret <= 0) {
      temp_name[0] = 0;
      IDBG_ERROR("Can not read name for the device %s skip\n",
        strerror(errno));
      continue;
    }

    if ((size_t)ret < JDMA_UTIL_MAX_NAME_SIZE) {
      temp_name[ret - 1] = 0;
    } else {
      temp_name[JDMA_UTIL_MAX_NAME_SIZE - 1] = 0;
    }

    IDBG_HIGH("Check video device %s\n", temp_name);

    ret = strncmp(temp_name, JDMA_UTIL_V4l2_DRV_NAME,
      sizeof(JDMA_UTIL_V4l2_DRV_NAME));
    if (ret == 0) {
      snprintf(temp_name, sizeof(temp_name), "/dev/video%d", num_device);
      IDBG_HIGH("Found Jpegdma v4l2 device %s\n", temp_name);
      strlcpy(*p_drv_name, temp_name, JDMA_UTIL_MAX_NAME_SIZE);
      return IMG_SUCCESS;
    }
  }

  IDBG_ERROR("Exceed max dev number %d jpeg dma video device not found\n",
    JDMA_UTIL_MAX_DEVICES);

error:
  free(*p_drv_name);
  return status;

}

/**
 * Function: jpegdma_util_read_msg
 *
 * Description: Read message from message pipe.
 *
 * Input parameters:
 *   @p_comp: Pointer to jpeg dma component instance.
 *   @thread_msg: Pointer to message need to be sent.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
static int32_t jpegdma_util_read_msg(jpegdma_comp_t *p_comp,
  jpegdma_util_thread_msg_t *thread_msg)
{
  size_t read_size;

  read_size = (size_t)read(p_comp->msg_pipefd[0],
    thread_msg, sizeof(*thread_msg));
  if (read_size != sizeof(*thread_msg)) {
    IDBG_ERROR("Can not read message from pipe");
    return IMG_ERR_GENERAL;
  }

  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_write_msg
 *
 * Description: write message to message pipe.
 *
 * Input parameters:
 *   @p_comp: Pointer to jpeg dma component instance.
 *   @thread_msg: Pointer to message need to be sent.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
static int32_t jpegdma_util_write_msg(jpegdma_comp_t *p_comp,
  jpegdma_util_thread_msg_t *thread_msg)
{
  size_t write_size;

  write_size = (size_t)write(p_comp->msg_pipefd[1],
    thread_msg, sizeof(*thread_msg));
  if (write_size != sizeof(*thread_msg)) {
    IDBG_ERROR("Can not write message to pipe");
    return IMG_ERR_GENERAL;
  }

  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_send_ack_msg
 *
 * Description: Send ack message.
 *
 * Input parameters:
 *   @p_comp: Pointer to jpeg dma component instance.
 *   @thread_msg: Pointer to message need to be sent.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
static int32_t jpegdma_util_send_ack_msg(jpegdma_comp_t *p_comp,
  jpegdma_util_thread_msg_t *thread_msg)
{
  size_t write_size;

  write_size = (size_t)write(p_comp->ack_pipefd[1],
    thread_msg, sizeof(*thread_msg));
  if (write_size != sizeof(*thread_msg)) {
    IDBG_ERROR("Can not write ack message to pipe");
    return IMG_ERR_GENERAL;
  }

  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_wait_ack_msg
 *
 * Description: Wait for ack message.
 *
 * Input parameters:
 *   @p_comp: Pointer to jpeg dma component instance.
 *   @thread_msg: Pointer to message need to be sent.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
static int32_t jpegdma_util_wait_ack_msg(jpegdma_comp_t *p_comp,
  jpegdma_util_thread_msg_t *thread_msg)
{
  jpegdma_util_thread_msg_t tmp_thread_msg;
  size_t read_size;
  struct pollfd poll_fd;
  int ret;

  poll_fd.fd = p_comp->ack_pipefd[0];
  poll_fd.events = POLLIN | POLLPRI;
  ret = poll(&poll_fd, 1, JDMA_UTIL_MSG_ACK_TIMEOUT_MS);
  if (ret <= 0) {
    /* Do not return error on timeout we want stream off to finish correctly */
    IDBG_ERROR("Poll timeout return ack for msg type %d", thread_msg->type);
    return IMG_SUCCESS;
  }

  read_size = (size_t)read(p_comp->ack_pipefd[0],
    &tmp_thread_msg, sizeof(tmp_thread_msg));
  if (read_size != sizeof(tmp_thread_msg) ||
    tmp_thread_msg.mode != thread_msg->mode) {
    IDBG_ERROR("Can not read ack message to pipe");
    return IMG_ERR_GENERAL;
  }

  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_get_v4l2_yuv_fmt
 *
 * Description: Get v4l2 yuv format from cam_format_t
 *
 * Input parameters:
 *   @cam_fmt: cam yuv format.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
int32_t jpegdma_util_get_v4l2_yuv_fmt(cam_format_t cam_fmt)
{
  switch (cam_fmt) {
  case CAM_FORMAT_Y_ONLY:
    return V4L2_PIX_FMT_GREY;

  case CAM_FORMAT_YUV_420_NV12:
    return V4L2_PIX_FMT_NV12;

  case CAM_FORMAT_YUV_420_NV21:
    return V4L2_PIX_FMT_NV21;

  case CAM_FORMAT_YUV_420_YV12:
    return V4L2_PIX_FMT_YVU420;

  default:
    return -1;
  }
}

/**
 * Function: jpegdma_util_check_dim
 *
 * Description: Check source and dest dimensions.
 *
 * Input parameters:
 *   @p_params: input params to validate.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: None
 **/
static int32_t jpegdma_util_check_dim(jpegdma_params_t *p_params)
{

  if (p_params == NULL) {
    IDBG_ERROR("Error, null pointer");
    return IMG_ERR_INVALID_INPUT;
  }

  img_dim_t src = p_params->src_dim;
  img_dim_t dest = p_params->dest_dim;
  img_rect_t crop = p_params->crop;
  float max_ds_factor = p_params->max_ds_factor;

  IDBG_HIGH("src wxh %dx%d stxsl %dx%d, dest wxh %dx%d stxsl %dx%d",
    src.width, src.height, src.stride, src.scanline,
    dest.width, dest.height, dest.stride, dest.scanline);

  if ((src.width == 0) || (src.height == 0) ||
    (dest.width == 0) || (dest.height == 0)) {
    IDBG_ERROR("Error, Input dim is zero, src %dx%d, dest %dx%d",
      src.width, src.height, dest.width, dest.height);
    return IMG_ERR_INVALID_INPUT;
  }

  if ((src.width & 1) || (src.height & 1) ||
    (dest.width & 1) || (dest.height & 1)) {
    IDBG_ERROR("Error, Input dim are not even, src %dx%d, dest %dx%d",
      src.width, src.height, dest.width, dest.height);
    return IMG_ERR_INVALID_INPUT;
  }

  if ((src.stride < src.width) || (src.scanline < src.height) ||
    (dest.stride < dest.width) || (dest.scanline < dest.height)) {
    IDBG_ERROR("Error, stride or scanline not valid");
    IDBG_ERROR("Error, src width x height %dx%d, stride x scan %dx%d",
      src.width, src.height, src.stride, src.scanline);
    IDBG_ERROR("Error, dest width x height %dx%d, stride x scan %dx%d",
      dest.width, dest.height, dest.stride, dest.scanline);
    return IMG_ERR_INVALID_INPUT;
  }

  if ((src.width < dest.width) ||
    (src.height < dest.height)) {
    IDBG_ERROR("Error, Upscale not supported, src %dx%d, dest %dx%d",
      src.width, src.height, dest.width, dest.height);
    return IMG_ERR_INVALID_INPUT;
  }

  if ((crop.size.width == 0) && (crop.size.height == 0)) {
    if ((((float)src.width / (float)dest.width) > max_ds_factor) ||
      (((float)src.height / (float)dest.height) > max_ds_factor)) {
      IDBG_ERROR("Error, max downscale ratio is %f, src %dx%d, dest %dx%d",
        max_ds_factor, src.width, src.height, dest.width,
        dest.height);
      return IMG_ERR_INVALID_INPUT;
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_check_crop
 *
 * Description: Check crop parameters against source and dest
 *   dimensions.
 *
 * Input parameters:
 *   @p_params: input params to validate.
 *   @crop: crop dimensions.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: None
 **/
static int32_t jpegdma_util_check_crop(jpegdma_params_t *p_params,
  img_rect_t crop)
{
  if (p_params == NULL) {
    IDBG_ERROR("Error, null pointer");
    return IMG_ERR_INVALID_INPUT;
  }

  img_dim_t src = p_params->src_dim;
  img_dim_t dest = p_params->dest_dim;
  float max_ds_factor = p_params->max_ds_factor;

  if ((crop.size.width == 0) || (crop.size.height == 0)) {
    return IMG_SUCCESS;
  }
  /* Upscale is not supported */
  if ((crop.size.width < (int)dest.width) ||
    (crop.size.height < (int)dest.height)) {
    IDBG_ERROR("Error, Upscale not supported crop wxh %dx%d, dest wxh %dx%d",
      crop.size.width, crop.size.height, dest.width, dest.height);
    return IMG_ERR_INVALID_INPUT;
  }

  if ((crop.size.width + crop.pos.x > (int)src.width) ||
    (crop.size.height + crop.pos.y > (int)src.height)) {
    IDBG_ERROR("Error, crop dim exceed src dim crop");
    IDBG_ERROR("crop lxt %dx%d wxh %dx%d, src wxh %dx%d",
      crop.pos.x,crop.pos.y,
      crop.size.width, crop.size.height,
      src.width, src.height);
    return IMG_ERR_INVALID_INPUT;
  }

  if ((crop.size.width % 2) || (crop.size.height % 2)) {
    IDBG_ERROR("Error, crop dimensions not even, %dx%d",
      crop.size.width, crop.size.height);
    return IMG_ERR_INVALID_INPUT;
  }

  if ((((float)crop.size.width / (float)dest.width) > max_ds_factor) ||
    (((float)crop.size.height / (float)dest.height) > max_ds_factor)) {
    IDBG_ERROR("Error, max downscale ratio is %f, src crop %dx%d, dest %dx%d",
      max_ds_factor, crop.size.width, crop.size.height, dest.width,
      dest.height);
    return IMG_ERR_INVALID_INPUT;
  }

  return IMG_SUCCESS;
}

/**
 * Function: jdma_util_hw_check_params
 *
 * Description: Check jpeg dma initialization parameters.
 *
 * Input parameters:
 *   @p_params: input params to validate.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
static int32_t jpegdma_util_check_params(jpegdma_params_t *p_params)
{
  int32_t status = IMG_SUCCESS;
  img_dim_t src_dim = p_params->src_dim;
  img_dim_t dest_dim = p_params->dest_dim;
  cam_format_t yuv_fmt = p_params->yuv_format;
  img_rect_t crop = p_params->crop;
  uint32_t framerate = p_params->framerate;

  IDBG_INFO("Src wxh %dx%d stxsl %dx%d, Dest wxh %dx%d stxsl %dx%d,"
    " Yuv fmt %d, Framerate %d, Crop lxt %dx%d wxh %dx%d",
    src_dim.width, src_dim.height, src_dim.stride, src_dim.scanline,
    dest_dim.width, dest_dim.height, dest_dim.stride, dest_dim.scanline,
    yuv_fmt, framerate, crop.pos.x, crop.pos.y, crop.size.width,
    crop.size.height);

  //framerate cannot be 0, will cause divide by 0 crash
  if (framerate == 0) {
    IDBG_ERROR("Error, framerate is 0");
    return IMG_ERR_INVALID_INPUT;
  }

  //check yuv format
  if (jpegdma_util_get_v4l2_yuv_fmt(yuv_fmt) < 0) {
    IDBG_ERROR("Error, Invalid yuv format %d", yuv_fmt);
    return IMG_ERR_INVALID_INPUT;
  }

  //check src and dest dimensions
  status = jpegdma_util_check_dim(p_params);
  if (IMG_ERROR(status)) {
    return IMG_ERR_INVALID_INPUT;
  }
  //check crop dimensions with respect to src and dest dimensions
  status = jpegdma_util_check_crop(p_params, crop);
  if (IMG_ERROR(status)) {
    return IMG_ERR_INVALID_INPUT;
  }

  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_set_format
 *
 * Description: Set the image format
 *
 * Input parameters:
 *   @fh: jpeg DMA V4L2 file handle.
 *   @v4l2_buftype: v4l2 buffer type
 *   @p_params: Pointer to input params.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
static int32_t jpegdma_util_set_format(int fh, enum v4l2_buf_type v4l2_buftype,
  jpegdma_params_t *p_params)
{
  struct v4l2_format fmt;
  int32_t ret;

  memset(&fmt, 0x00, sizeof(fmt));
  fmt.type = v4l2_buftype;
  fmt.fmt.pix.pixelformat =
    jpegdma_util_get_v4l2_yuv_fmt(p_params->yuv_format);

  switch (v4l2_buftype) {
  case V4L2_BUF_TYPE_VIDEO_OUTPUT:
    fmt.fmt.pix.width = p_params->src_dim.width;
    fmt.fmt.pix.height = p_params->src_dim.scanline;
    fmt.fmt.pix.bytesperline = p_params->src_dim.stride;
    break;
  case V4L2_BUF_TYPE_VIDEO_CAPTURE:
    fmt.fmt.pix.width = p_params->dest_dim.width;
    fmt.fmt.pix.height = p_params->dest_dim.scanline;
    fmt.fmt.pix.bytesperline = p_params->dest_dim.stride;
    break;
  default:
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_HIGH("VIDIOC_S_FMT, buff type %d",
    v4l2_buftype);
  ret = ioctl(fh, VIDIOC_S_FMT, &fmt);
  if (ret < 0) {
    IDBG_ERROR("Set format fail, buffer type %d %s",
      fmt.type, strerror(errno));
    return IMG_ERR_INVALID_INPUT;
  }

  switch (v4l2_buftype) {
  case V4L2_BUF_TYPE_VIDEO_OUTPUT:
    if ((p_params->src_dim.width != fmt.fmt.pix.width) ||
      (p_params->src_dim.scanline != fmt.fmt.pix.height)) {
      IDBG_ERROR("dim mismatch input %dx%d, drv compatable %dx%d",
        p_params->src_dim.width, p_params->src_dim.scanline,
        fmt.fmt.pix.width, fmt.fmt.pix.height);
      return IMG_ERR_INVALID_INPUT;
    }
    p_params->src_buf_sz = fmt.fmt.pix.sizeimage;
    IDBG_HIGH("VIDIOC_S_FMT, buff type output, size %d",
      p_params->src_buf_sz);
    break;
  case V4L2_BUF_TYPE_VIDEO_CAPTURE:
    if ((p_params->dest_dim.width != fmt.fmt.pix.width) ||
      (p_params->dest_dim.scanline != fmt.fmt.pix.height)) {
      IDBG_ERROR("dim mismatch input %dx%d, drv compatable %dx%d",
        p_params->dest_dim.width, p_params->dest_dim.scanline,
        fmt.fmt.pix.width, fmt.fmt.pix.height);
      return IMG_ERR_INVALID_INPUT;
    }
    p_params->dest_buf_sz = fmt.fmt.pix.sizeimage;
    IDBG_HIGH("VIDIOC_S_FMT, buff type capture, size %d",
      p_params->dest_buf_sz);
    break;
  default:
    return IMG_ERR_INVALID_INPUT;
  }

  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_req_buf
 *
 * Description: Request v4l2 buffers for specified buf type and
 *   count.
 *
 * Input parameters:
 *   @fh: JPEG DMA V4l2 driver file handle.
 *   @v4l2_buf_type: Buffer type for which buffer needs to be
 *     requested.
 *   @buf_cnt: Number of buffers to request.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
static int32_t jpegdma_util_req_buf(int fh,
  enum v4l2_buf_type v4l2_buf_type, uint32_t buf_cnt)
{
  struct v4l2_requestbuffers req_bufs;
  int8_t ret;

  memset(&req_bufs, 0x00, sizeof(req_bufs));
  req_bufs.type = v4l2_buf_type;
  req_bufs.memory = V4L2_MEMORY_USERPTR;
  req_bufs.count = buf_cnt;

  IDBG_HIGH("VIDIOC_REQBUFS for type %d, cnt %d",
    v4l2_buf_type, buf_cnt);
  ret = ioctl(fh, VIDIOC_REQBUFS, &req_bufs);
  if (ret < 0) {
    IDBG_ERROR("Buf request failed for buff %d, erro %s",
      v4l2_buf_type, strerror(errno));
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_set_framerate
 *
 * Description: Set frame rate to calculate clock speed.
 *
 * Input parameters:
 *   @fh: JPEG DMA V4l2 driver file handle.
 *   @framerate: set frame rate.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
static int32_t jpegdma_util_set_framerate(int fh, uint32_t framerate)
{
  struct v4l2_streamparm param;
  int8_t ret;

  memset(&param, 0x00, sizeof(param));
  param.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  param.parm.output.timeperframe.numerator = 1;
  param.parm.output.timeperframe.denominator = framerate;

  IDBG_HIGH("VIDIOC_S_PARM Set framerate %d", framerate);
  ret = ioctl(fh, VIDIOC_S_PARM, &param);
  if (ret < 0) {
    IDBG_ERROR("set framerate failed %s", strerror(errno));
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_set_crop
 *
 * Description: Apply crop.
 *
 * Input parameters:
 *   @fh: JPEG DMA V4L2 driver file handle.
 *   @p_crop: Crop dimensions.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
static int32_t jpegdma_util_set_crop(int fh, img_rect_t crop_info)
{
  struct v4l2_crop crop;
  int8_t ret;

  memset(&crop, 0x00, sizeof(crop));
  crop.c.left = crop_info.pos.x;
  crop.c.top = crop_info.pos.y;
  crop.c.width = crop_info.size.width;
  crop.c.height = crop_info.size.height;
  crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

  IDBG_HIGH("VIDIOC_S_CROP");
  ret = ioctl(fh, VIDIOC_S_CROP, &crop);
  if (ret < 0) {
    IDBG_ERROR("Set crop fail %s", strerror(errno));
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_get_max_scale
 *
 * Description: Get max downscale factor.
 *
 * Input parameters:
 *   @fh: JPEG DMA V4L2 driver file handle.
 *   @p_ds_factor: Max downscale factor.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
int32_t jpegdma_util_get_max_scale(int fh, float *p_ds_factor)
{
  struct v4l2_control control;
  int8_t ret;

  if (p_ds_factor == NULL) {
    IDBG_ERROR("Error, null pointer");
    return IMG_ERR_INVALID_INPUT;
  }

  memset(&control, 0x00, sizeof(control));
  control.id = V4L2_CID_JPEG_DMA_MAX_DOWN_SCALE;
  IDBG_HIGH("VIDIOC_S_CROP");
  ret = ioctl(fh, VIDIOC_G_CTRL, &control);
  if (ret < 0) {
    IDBG_ERROR("Get max dma downscal factor failed %s ", strerror(errno));
    return IMG_ERR_INVALID_INPUT;
  }

  *p_ds_factor = (float) control.value;
  IDBG_HIGH("Max ds factor got from driver is %d", *p_ds_factor);
  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_stream_on
 *
 * Description: Function calling v4l2 stream on.
 *
 * Input parameters:
 *   @fh: JPEG DMA V4l2 driver file handle.
 *   @v4l2_buftype: Buffer type for which buffer needs to be
 *     requested.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
static int8_t jpegdma_util_stream_on(int fh,
    enum v4l2_buf_type v4l2_buftype)
{
  int8_t ret;

  IDBG_HIGH("VIDIOC_STREAMON, buff type %d", v4l2_buftype);
  ret = ioctl(fh, VIDIOC_STREAMON, &v4l2_buftype);
  if (ret < 0) {
    IDBG_ERROR("Stream on failed for buff %d, erro %s", v4l2_buftype,
      strerror(errno));
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_stream_off
 *
 * Description: Function calling v4l2 stream off.
 *
 * Input parameters:
 *   @fh: JPEG DMA V4l2 driver file handle.
 *   @v4l2_buftype: Buffer type for which buffer needs to be
 *     requested.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
static int8_t jpegdma_util_stream_off(int fh,
    enum v4l2_buf_type v4l2_buftype)
{
  int8_t ret;

  IDBG_HIGH("VIDIOC_STREAMOFF, buff type %d", v4l2_buftype);
  ret = ioctl(fh, VIDIOC_STREAMOFF, &v4l2_buftype);
  if (ret < 0) {
    IDBG_ERROR("Stream off failed for buff %d, erro %s", v4l2_buftype,
      strerror(errno));
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_queue_drvbuff
 *
 * Description: Queue buffer into v4l2 using QBUF.
 *
 * Input parameters:
 *   @fh: JPEG DMA V4L2 driver file handle.
 *   @buf_idx: buffer index
 *   @buf_type: buffer type
 *   @buf_fd: buffer file descriptor
 *   @size: buffer size
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
static int8_t jpegdma_util_queue_drvbuff(int fh, uint32_t buf_idx,
  enum v4l2_buf_type v4l2_buftype, int32_t buf_fd, uint32_t buf_size,
  uint32_t buf_offset)
{
  struct v4l2_buffer buff;
  struct msm_jpeg_dma_buff* p_dma_buff;
  int ret;

  p_dma_buff = malloc (sizeof(struct msm_jpeg_dma_buff));
  if (p_dma_buff == NULL) {
    IDBG_ERROR("Error, No memory");
    return IMG_ERR_NO_MEMORY;
  }

  p_dma_buff->fd = buf_fd;
  p_dma_buff->offset = buf_offset;

  memset(&buff, 0x00, sizeof(buff));
  buff.index = buf_idx;
  buff.type = v4l2_buftype;
  buff.memory = V4L2_MEMORY_USERPTR;
  buff.m.userptr = (unsigned long)p_dma_buff;
  buff.length = buf_size;

  IDBG_HIGH("VIDIOC_QBUF, buff type %d, idx %d, len %d, fd %d, offset %d",
    v4l2_buftype, buf_idx, buf_size, buf_fd, buf_offset);
  ret = ioctl(fh, VIDIOC_QBUF, &buff);
  if (ret < 0) {
    IDBG_ERROR("QBUF drv fail for buf type %d, %s",
      v4l2_buftype, strerror(errno));
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_dequeue_drvbuff
 *
 * Description: Dequeue buffer into v4l2 using DQBUF.
 *
 * Input parameters:
 *   @fh: JPEG DMA V4L2 driver file handle
 *   @buf_type: buffer type
 *   @buf_idx: buffer index
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
static int8_t jpegdma_util_dequeue_drvbuff(int fh,
  enum v4l2_buf_type v4l2_buftype, uint32_t *p_buf_idx)
{
  struct v4l2_buffer buff;
  struct msm_jpeg_dma_buff* p_dma_buff;
  int ret;

  memset(&buff, 0x00, sizeof(buff));
  buff.type = v4l2_buftype;

  IDBG_HIGH("VIDIOC_DQBUF, buff type %d", v4l2_buftype);
  ret = ioctl(fh, VIDIOC_DQBUF, &buff);
  if (ret < 0) {
    IDBG_ERROR("DQBUF drv fail %s", strerror(errno));
    return IMG_ERR_INVALID_INPUT;
  }

  p_dma_buff = (struct msm_jpeg_dma_buff*) buff.m.userptr;
  *p_buf_idx = buff.index;
  IDBG_HIGH("VIDIOC_DQBUF, buff type %d, idx %d, len %d, fd %lu, offset %d",
    buff.type, buff.index, buff.length, p_dma_buff->fd, p_dma_buff->offset);

  free(p_dma_buff);
  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_flush_queues
 *
 * Description: function to flush queues
 *
 * Input parameters:
 *   @p_comp: Pointer to jpegdma component.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes:
 **/
static int32_t jpegdma_util_flush_queues(jpegdma_comp_t *p_comp)
{

  if(img_q_count(&p_comp->inProcessQ)) {
    img_q_flush_and_destroy(&p_comp->inProcessQ);
  }

  if(img_q_count(&p_comp->toProcessQ)) {
    img_q_flush_and_destroy(&p_comp->toProcessQ);
  }

  if (img_q_count(&p_comp->b.inputQ)) {
    img_q_flush(&p_comp->b.inputQ);
  }

  if (img_q_count(&p_comp->b.outBufQ)) {
    img_q_flush(&p_comp->b.outBufQ);
  }

  if (img_q_count(&p_comp->b.metaQ)) {
    img_q_flush(&p_comp->b.metaQ);
  }

  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_util_start_streaming
 *
 * Description: function to start util
 *
 * Input parameters:
 *   @p_comp: Pointer to jpegdma component.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes:
 **/
static int32_t jpegdma_util_start_streaming(jpegdma_comp_t *p_comp)
{
  int32_t status = IMG_SUCCESS;

  IDBG_HIGH("E");

  status = jpegdma_util_check_params(&p_comp->params);
  if (IMG_ERROR(status))
    return status;

  status = jpegdma_util_set_format(p_comp->fd_drv,
    V4L2_BUF_TYPE_VIDEO_OUTPUT, &p_comp->params);
  if (IMG_ERROR(status))
    return status;

  status = jpegdma_util_set_format(p_comp->fd_drv,
    V4L2_BUF_TYPE_VIDEO_CAPTURE, &p_comp->params);
  if (IMG_ERROR(status))
    return status;

  status = jpegdma_util_set_framerate(p_comp->fd_drv,
    p_comp->params.framerate);
  if (IMG_ERROR(status))
    return status;

  if ((p_comp->params.crop.size.width != 0) &&
    (p_comp->params.crop.size.height != 0)) {
    status = jpegdma_util_set_crop(p_comp->fd_drv,
      p_comp->params.crop);
    if (IMG_ERROR(status))
      return status;
  }

  status = jpegdma_util_req_buf(p_comp->fd_drv,
    V4L2_BUF_TYPE_VIDEO_OUTPUT, JDMA_UTIL_MAX_REQBUF);
  if (IMG_ERROR(status))
    return status;

  status = jpegdma_util_req_buf(p_comp->fd_drv,
    V4L2_BUF_TYPE_VIDEO_CAPTURE, JDMA_UTIL_MAX_REQBUF);
  if (IMG_ERROR(status))
    goto error_cap_reqbuf_failed;

  status = jpegdma_util_stream_on(p_comp->fd_drv,
    V4L2_BUF_TYPE_VIDEO_OUTPUT);
  if (IMG_ERROR(status))
    goto error_out_stream_on_failed;

  status = jpegdma_util_stream_on(p_comp->fd_drv,
    V4L2_BUF_TYPE_VIDEO_CAPTURE);
  if (IMG_ERROR(status))
    goto error_cap_stream_on_failed;

  IDBG_HIGH("X");
  return status;

error_cap_stream_on_failed:
  jpegdma_util_stream_off(p_comp->fd_drv,
    V4L2_BUF_TYPE_VIDEO_OUTPUT);

error_out_stream_on_failed:
  jpegdma_util_req_buf(p_comp->fd_drv,
    V4L2_BUF_TYPE_VIDEO_CAPTURE, 0);

error_cap_reqbuf_failed:
  jpegdma_util_req_buf(p_comp->fd_drv,
    V4L2_BUF_TYPE_VIDEO_OUTPUT, 0);

  return status;
}

/**
 * Function: jpegdma_util_stop_streaming
 *
 * Description: function to stop util
 *
 * Input parameters:
 *   @p_comp: Pointer to jpegdma component.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: This function is blocking
 **/
static int32_t jpegdma_util_stop_streaming(jpegdma_comp_t *p_comp)
{
  int32_t status = IMG_SUCCESS;

  IDBG_HIGH("E");

  status = jpegdma_util_stream_off(p_comp->fd_drv,
    V4L2_BUF_TYPE_VIDEO_OUTPUT);
  if (IMG_ERROR(status))
    return status;

  status = jpegdma_util_stream_off(p_comp->fd_drv,
    V4L2_BUF_TYPE_VIDEO_CAPTURE);
  if (IMG_ERROR(status))
    return status;

  status = jpegdma_util_req_buf(p_comp->fd_drv,
    V4L2_BUF_TYPE_VIDEO_OUTPUT, 0);
  if (IMG_ERROR(status))
    return status;

  status = jpegdma_util_req_buf(p_comp->fd_drv,
    V4L2_BUF_TYPE_VIDEO_CAPTURE, 0);
  if (IMG_ERROR(status))
    return status;

  IDBG_HIGH("X");
  return status;
}

/**
 * Function: jpegdma_util_make_bundle
 *
 * Description: This function is used bunble the input, output
 *   and meta buffers
 *
 * Input parameters:
 *   @p_comp: The pointer to the component handle.
 *   @p_bundle_handle: pointer to bundle
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes:
 **/
static int32_t jpegdma_util_make_bundle(jpegdma_comp_t *p_comp,
  img_frame_bundle_t **p_bundle_handle)
{
  int32_t status = IMG_SUCCESS;
  img_frame_bundle_t *p_bundle;

  p_bundle = (img_frame_bundle_t *)malloc(sizeof(img_frame_bundle_t));
  if (p_bundle == NULL) {
    IDBG_ERROR("Meta buffer enqueue failed");
    return IMG_ERR_NO_MEMORY;
  }

  p_bundle->p_input[0] = (img_frame_t *)img_q_dequeue(&p_comp->b.inputQ);
  if (!p_bundle->p_input[0]) {
    IDBG_ERROR("Cannot dequeue input frame");
    goto error;
  }

  p_bundle->p_output[0] = (img_frame_t *)img_q_dequeue(&p_comp->b.outBufQ);
  if (!p_bundle->p_output[0]) {
    IDBG_ERROR("Cannot dequeue output frame");
    goto error;
  }

  p_bundle->p_meta[0] = (img_meta_t *)img_q_dequeue(&p_comp->b.metaQ);
  if (!p_bundle->p_meta[0]) {
    IDBG_ERROR("Cannot dequeue meta frame");
    goto error;
  }

  *p_bundle_handle = p_bundle;
  return status;

error:
  free(p_bundle);
  return IMG_ERR_NO_MEMORY;
}

/**
 * Function: jpegdma_util_send_msg
 *
 * Description: Send message to jpeg dma worker thread.
 *
 * Input parameters:
 *   @p_comp: Pointer to jpeg dma component instance.
 *   @type: Message type need to be set.
 *   @mode: Message mode blocking/non-blocking.
 *   @p_data: Pointer to message data (It should match with message type).
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: None
 **/
static int32_t jpegdma_util_send_msg(jpegdma_comp_t *p_comp,
  jpegdma_util_msg_type_t type, jpegdma_util_msg_mode_t mode, void *p_data)
{
  jpegdma_util_thread_msg_t thread_msg;
  int status = IMG_SUCCESS;

  memset(&thread_msg, 0x0, sizeof(thread_msg));

  thread_msg.type = type;
  thread_msg.mode = mode;

  QIMG_LOCK(&p_comp->b.mutex);

  if (p_comp->fd_drv < 0) {
    IDBG_ERROR("Driver closed can not process the message");
    status = IMG_ERR_GENERAL;
    goto exit_unlock;
  }

  switch (thread_msg.type) {
  case JDMA_MSG_QUEUE_BUNDLE:
   if (p_data) {
     thread_msg.p_bundle = (img_frame_bundle_t *)p_data;
     IDBG_HIGH("JDMA_MSG_QUEUE_BUNDLE %p", p_data);
   } else {
     IDBG_ERROR("Error JDMA_MSG_QUEUE_BUNDLE %p", p_data);
     status = IMG_ERR_INVALID_INPUT;
   }
   break;
  case JDMA_MSG_STOP:
    IDBG_HIGH("JDMA_MSG_STOP");
    break;
  case JDMA_MSG_EXIT:
    IDBG_HIGH("JDMA_MSG_EXIT");
    break;
  default:
    IDBG_ERROR("Unknown message");
    status = IMG_ERR_INVALID_INPUT;
    break;
  }
  if (IMG_ERROR(status)) {
    IDBG_ERROR("Wrong message");
    goto exit_unlock;
  }

  status = jpegdma_util_write_msg(p_comp, &thread_msg);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("Can not write message to pipe");
    goto exit_unlock;
  }

  QIMG_UNLOCK(&p_comp->b.mutex);

  if (mode == JDMA_MSG_MODE_BLOCK) {
    IDBG_HIGH("Wait for ACK msg");
    status = jpegdma_util_wait_ack_msg(p_comp, &thread_msg);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("Error wait for ack message");
    }
    IDBG_HIGH("ACK msg received");
  }

  return status;

exit_unlock:
  QIMG_UNLOCK(&p_comp->b.mutex);
  return status;
}

/**
 * Function: jpegdma_util_find_buff_by_id
 *
 * Description: This function is used to find buff by id
 *
 * Input parameters:
 *   @p_comp: The pointer to the component handle.
 *   @p_buff: The buffer which needs to be processed
 *
 * Return values:
 *   1 if buffer found, 0 if not
 *
 * Notes:
 **/
int jpegdma_util_find_buff_by_id(void *p_nodedata, void *p_userdata)
{
  jpegdma_util_frame_to_find_t *p_frame_to_find;
  img_frame_t *p_frame = NULL;
  img_meta_t *p_meta = NULL;
  uint32_t frameid;
  jpegdma_buffer_type_t buff_type;

  if (!p_nodedata || !p_userdata) {
    IDBG_ERROR("NULL pointer %p %p", p_nodedata, p_userdata);
    return 0;
  }

  p_frame_to_find = (jpegdma_util_frame_to_find_t *)p_userdata;
  frameid = p_frame_to_find->frameid;
  buff_type = p_frame_to_find->buff_type;

  switch (buff_type) {
  case JDMA_IN_BUFFER:
  case JDMA_OUT_BUFFER:
    p_frame = (img_frame_t *)p_nodedata;
    if (frameid == p_frame->frame_id) {
      return 1;
    }
    return 0;
  case JDMA_META_BUFFER:
    p_meta = (img_meta_t *)p_nodedata;
    if (frameid == p_meta->frame_id) {
      return 1;
    }
    return 0;
  }

  return 0;
}

/**
 * Function: jpegdma_util_send_buffers
 *
 * Description: This function is used to send frame buffer for processing
 *
 * Input parameters:
 *   @p_comp: The pointer to the component handle.
 *   @p_buff: The buffer which needs to be processed
 *   @buff_type: buffer type
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: Does not support IMG_IN_OUT type. Is non blocking.
 **/
int32_t jpegdma_util_send_buffers(jpegdma_comp_t *p_comp, void *p_buff,
  jpegdma_buffer_type_t buff_type)
{
  int32_t status = IMG_SUCCESS;
  jpegdma_util_msg_mode_t mode = JDMA_MSG_MODE_NON_BLOCK;
  jpegdma_util_msg_type_t msg_type = JDMA_MSG_QUEUE_BUNDLE;
  img_frame_bundle_t *p_bundle = NULL;
  jpegdma_util_frame_to_find_t frame_to_find;

  switch (buff_type) {
  case JDMA_IN_BUFFER:
    status = img_q_enqueue(&p_comp->b.inputQ, p_buff);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("Input buffer enqueue failed");

      frame_to_find.frameid = ((img_frame_t *)p_buff)->frame_id;

      frame_to_find.buff_type = JDMA_OUT_BUFFER;
      img_q_traverse_dequeue(&p_comp->b.outBufQ,
        jpegdma_util_find_buff_by_id, &frame_to_find);

      frame_to_find.buff_type = JDMA_META_BUFFER;
      img_q_traverse_dequeue(&p_comp->b.metaQ,
        jpegdma_util_find_buff_by_id, &frame_to_find);

      return status;
    }
    break;
  case JDMA_OUT_BUFFER:
    status = img_q_enqueue(&p_comp->b.outBufQ, p_buff);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("Output buffer enqueue failed");

      frame_to_find.frameid = ((img_frame_t *)p_buff)->frame_id;

      frame_to_find.buff_type = JDMA_IN_BUFFER;
      img_q_traverse_dequeue(&p_comp->b.inputQ,
        jpegdma_util_find_buff_by_id, &frame_to_find);

      frame_to_find.buff_type = JDMA_META_BUFFER;
      img_q_traverse_dequeue(&p_comp->b.metaQ,
        jpegdma_util_find_buff_by_id, &frame_to_find);

      return status;
    }
    break;
  case JDMA_META_BUFFER:
    status = img_q_enqueue(&p_comp->b.metaQ, p_buff);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("Meta buffer enqueue failed");

      frame_to_find.frameid = ((img_meta_t *)p_buff)->frame_id;

      frame_to_find.buff_type = JDMA_IN_BUFFER;
      img_q_traverse_dequeue(&p_comp->b.inputQ,
        jpegdma_util_find_buff_by_id, &frame_to_find);

      frame_to_find.buff_type = JDMA_OUT_BUFFER;
      img_q_traverse_dequeue(&p_comp->b.outBufQ,
        jpegdma_util_find_buff_by_id, &frame_to_find);

      return status;
    }
    break;
  }

  if (img_q_count(&p_comp->b.inputQ) &&
    img_q_count(&p_comp->b.outBufQ) &&
    img_q_count(&p_comp->b.metaQ)) {

    IDBG_HIGH("Got all buffers , make bundle");
    status = jpegdma_util_make_bundle(p_comp, &p_bundle);
    if (IMG_ERROR(status))
      return status;

    status = jpegdma_util_send_msg(p_comp, msg_type, mode, p_bundle);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("send message failed");
      free(p_bundle);
      return status;
    }
  }

  return status;
}

/**
 * Function: jpegdma_util_start
 *
 * Description: function to start util with input params
 *
 * Input parameters:
 *   @p_comp: Pointer to jpegdma component.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: This function is blocking
 **/
int32_t jpegdma_util_start(jpegdma_comp_t *p_comp)
{
  return jpegdma_util_start_streaming(p_comp);
}

/**
 * Function: jpegdma_util_abort
 *
 * Description: function to start util with input params
 *
 * Input parameters:
 *   @p_comp: Pointer to jpegdma component.
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: This function is blocking
 **/
int32_t jpegdma_util_abort(jpegdma_comp_t *p_comp)
{
  int32_t status = IMG_SUCCESS;
  jpegdma_util_msg_mode_t mode = JDMA_MSG_MODE_BLOCK;
  jpegdma_util_msg_type_t msg_type = JDMA_MSG_STOP;

  status = jpegdma_util_send_msg(p_comp, msg_type, mode, NULL);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("send message failed");
  }
  return status;
}

/**
 * Function: jpegdma_util_proc_bundle
 *
 * Description: This function is used to process bundle
 *
 * Input parameters:
 *   @handle: The pointer to the component handle.
 *   @p_bundle: pointer to bundel
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes:
 **/
static int32_t jpegdma_util_proc_bundle(jpegdma_comp_t *p_comp,
  img_frame_bundle_t *p_bundle)
{
  int32_t status = IMG_SUCCESS;
  img_rect_t new_crop;
  img_rect_t curr_crop = p_comp->params.crop;
  uint32_t buf_idx, buf_size, buf_offset;
  int32_t buf_fd;

  if (p_bundle == NULL) {
    IDBG_ERROR("Error bundle pointer null");
    return IMG_ERR_NO_MEMORY;
  }

  new_crop = p_bundle->p_meta[0]->output_crop;

  if (IMG_CHECK_CROP_NEEDED(new_crop, curr_crop)) {
    status = jpegdma_util_check_crop(&p_comp->params, new_crop);
    if (IMG_ERROR(status))
      return status;

    IDBG_INFO("New crop, left x top %d x %d, width x height %d x %d",
      new_crop.pos.x, new_crop.pos.y, new_crop.size.width,
      new_crop.size.height);
    status = jpegdma_util_set_crop(p_comp->fd_drv, new_crop);
    if (IMG_ERROR(status))
      return status;

    p_comp->params.crop = new_crop;
  }

  buf_idx = JDMA_UTIL_FRAME_TO_BUF_IDX(p_bundle->p_input[0]->frame_id);
  buf_fd = p_bundle->p_input[0]->frame[0].plane[0].fd;
  buf_offset = p_bundle->p_input[0]->frame[0].plane[0].offset;
  buf_size = IMG_FRAME_LEN(p_bundle->p_input[0]);

  if (p_comp->params.src_buf_sz > buf_size) {
    IDBG_ERROR("In sufficient buffer size %d, config size %d,",
      buf_size, p_comp->params.src_buf_sz);
    return IMG_ERR_OUT_OF_BOUNDS;
  }

  img_dump_frame(p_bundle->p_input[0], "JPEGDMA_input",
    p_bundle->p_input[0]->frame_id, NULL);

  status = jpegdma_util_queue_drvbuff(p_comp->fd_drv, buf_idx,
    V4L2_BUF_TYPE_VIDEO_OUTPUT, buf_fd, buf_size, buf_offset);
    if (IMG_ERROR(status))
      return status;

  buf_fd = p_bundle->p_output[0]->frame[0].plane[0].fd;
  buf_offset = p_bundle->p_output[0]->frame[0].plane[0].offset;
  buf_size = IMG_FRAME_LEN(p_bundle->p_output[0]);

  if (p_comp->params.dest_buf_sz < buf_size) {
    IDBG_ERROR("dest frame size exceeds configured dest size, config size %d,"
      "input size %d", p_comp->params.dest_buf_sz, buf_size);
    return IMG_ERR_OUT_OF_BOUNDS;
  }

  status = jpegdma_util_queue_drvbuff(p_comp->fd_drv, buf_idx,
    V4L2_BUF_TYPE_VIDEO_CAPTURE, buf_fd, buf_size, buf_offset);
    if (IMG_ERROR(status))
      return status;

  status = img_q_enqueue(&p_comp->inProcessQ, p_bundle);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("frame bundle enque failed");
    return status;
  }

  return status;
}

/**
 * Function: jdma_util_attempt_proc_bundle
 *
 * Description: This function is used check if bundle can be
 *   processed and call process bundle function.
 *
 * Input parameters:
 *   @p_comp: The pointer to the component handle.
 *
 * Return values:
 *   none
 *
 * Notes:
 **/
static void jdma_util_attempt_proc_bundle(jpegdma_comp_t *p_comp)
{
  int32_t status = IMG_SUCCESS;
  img_frame_bundle_t *p_bundle = NULL;

  if (img_q_count(&p_comp->inProcessQ) < JDMA_UTIL_MAX_REQBUF) {

    if (img_q_count(&p_comp->toProcessQ)) {
      p_bundle = img_q_dequeue(&p_comp->toProcessQ);

      status = jpegdma_util_proc_bundle(p_comp, p_bundle);
      if (IMG_ERROR(status)) {
        IDBG_ERROR("Process bundle failed");
        p_comp->jpegdma_cb(p_comp->userdata, p_bundle, QIMG_EVT_ERROR);
        free(p_bundle);
      }
    }
  }
}

/**
 * Function: jpegdma_util_queue_bundle
 *
 * Description: This function is used to queue input, output and
 *   meta buffers. once a set of buffers are available it calls
 *   helper functions to enqueue buffers to driver.
 *
 * Input parameters:
 *   @p_comp: The pointer to the component handle.
 *   @p_data: buffer to enqueue.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes:
 **/
static int32_t jpegdma_util_queue_bundle(jpegdma_comp_t *p_comp, void *p_data)
{
  int32_t status = IMG_SUCCESS;
  img_frame_bundle_t *p_bundle = NULL;

  if (p_data != NULL) {
    status = img_q_enqueue(&p_comp->toProcessQ, p_data);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("bundle enqueue failed");
      p_comp->jpegdma_cb(p_comp->userdata, p_data, QIMG_EVT_ERROR);
      free(p_bundle);
    }
  }

  IDBG_HIGH("Attempt to process bundle if available");
  jdma_util_attempt_proc_bundle(p_comp);

  return status;
}

/**
 * Function: jpegdma_util_proc_msg
 *
 * Description: Helper function executed from worker thread context used to
 *   process message sent from jpeg dma util.
 *
 * Input parameters:
 *   @p_comp: Pointer to jpeg dma component instance.
 *   @fh: Jpeg dma driver file handle.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: None
 **/
static int32_t jpegdma_util_proc_msg(jpegdma_comp_t *p_comp)
{
  jpegdma_util_thread_msg_t rec_msg;
  int ack_ret;
  int32_t status = IMG_SUCCESS;

  status = jpegdma_util_read_msg(p_comp, &rec_msg);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("Can not read cmd message");
    return status;
  }

  switch (rec_msg.type) {
  case JDMA_MSG_QUEUE_BUNDLE:
    IDBG_HIGH("JDMA_MSG_QUEUE_BUNDLE");
    status = jpegdma_util_queue_bundle(p_comp, rec_msg.p_bundle);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("Fail to queue bundle");
    }
    break;
  case JDMA_MSG_STOP:
    /* Flush the buffers before stop call */
    IDBG_HIGH("JDMA_MSG_STOP received");
    jpegdma_util_flush_queues(p_comp);

    status = jpegdma_util_stop_streaming(p_comp);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("Stop streaming failed");
    }
    break;
  case JDMA_MSG_EXIT:
    IDBG_HIGH("JDMA_MSG_EXIT received");
    QIMG_LOCK(&p_comp->b.mutex);
    p_comp->b.thread_exit = true;
    QIMG_UNLOCK(&p_comp->b.mutex);
    break;
  default:
    IDBG_ERROR("Unknown message type %d",
      rec_msg.type);
    status = IMG_ERR_NOT_FOUND;
    break;
  }

  if (rec_msg.mode == JDMA_MSG_MODE_BLOCK) {
    IDBG_HIGH("send ack msg");
    ack_ret = jpegdma_util_send_ack_msg(p_comp, &rec_msg);
    if (IMG_ERROR(ack_ret)) {
      IDBG_ERROR("Can not send ack message");
      status = IMG_ERR_GENERAL;
    }
  }

  return status;
}

/**
 * Function: jpegdma_util_dequeue_buf
 *
 * Description: This function callse helper functions to DQBUF
 *   input, output buffers and send call back to user.
 *
 * Input parameters:
 *   @p_comp: The pointer to the component handle.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes:
 **/
static int32_t jpegdma_util_dequeue_bundle(jpegdma_comp_t *p_comp)
{
  int32_t status = IMG_SUCCESS;
  uint32_t src_buf_idx, dest_buf_idx;
  img_frame_bundle_t *p_bundle;

  IDBG_HIGH("HW completed process call DQBUF");

  p_bundle = img_q_dequeue(&p_comp->inProcessQ);
  if (p_bundle == NULL) {
    IDBG_ERROR("bundle dequeue failed");
    status IMG_ERR_NO_MEMORY;
    goto dequeue_fail;
  }

  status = jpegdma_util_dequeue_drvbuff(p_comp->fd_drv,
    V4L2_BUF_TYPE_VIDEO_OUTPUT, &src_buf_idx);
  if (IMG_ERROR(status))
    goto error;

  status = jpegdma_util_dequeue_drvbuff(p_comp->fd_drv,
    V4L2_BUF_TYPE_VIDEO_CAPTURE, &dest_buf_idx);
  if (IMG_ERROR(status))
    goto error;

  if (src_buf_idx != dest_buf_idx) {
    IDBG_ERROR("src and dest dq buf idx mismatch");
    status = IMG_ERR_GENERAL;
    goto error;
  }

  if (src_buf_idx !=
    JDMA_UTIL_FRAME_TO_BUF_IDX(p_bundle->p_input[0]->frame_id)) {
    IDBG_ERROR("idx and frame id mismatch");
    status = IMG_ERR_GENERAL;
    goto error;
  }

  img_dump_frame(p_bundle->p_output[0], "JPEGDMA_output",
    p_bundle->p_output[0]->frame_id, NULL);

  IDBG_HIGH("Send BUF DONE for frame id %d", p_bundle->p_input[0]->idx);
  p_comp->jpegdma_cb(p_comp->userdata, p_bundle, QIMG_EVT_BUF_DONE);
  if (p_bundle) {
    free(p_bundle);
  }
  p_bundle = NULL;

  IDBG_HIGH("Attempt to process bundle if available");
  jdma_util_attempt_proc_bundle(p_comp);

  return status;

error:
  p_comp->jpegdma_cb(p_comp->userdata, p_bundle, QIMG_EVT_ERROR);
  if (p_bundle) {
   free(p_bundle);
  }

dequeue_fail:
  IDBG_HIGH("Attempt to process bundle if available");
  jdma_util_attempt_proc_bundle(p_comp);
  return status;
}

/**
 * Function: jpegdma_util_start_thread_loop
 *
 * Description: worker thread loop
 *
 * Input parameters:
 *   @p_comp: Pointer to jpegdma component.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
void jpegdma_util_start_thread_loop(jpegdma_comp_t *p_comp)
{
  struct pollfd poll_fd[2];
  nfds_t wait_on_fds;
  int drv_fd;
  int32_t status;
  int fail_count = 0;
  int process_succeeded;

  if (!p_comp) {
    IDBG_ERROR("Null component");
    return;
  }
  drv_fd = p_comp->fd_drv;

  poll_fd[0].fd = p_comp->msg_pipefd[0];
  poll_fd[0].events = POLLIN | POLLPRI;

  poll_fd[1].fd = drv_fd;
  poll_fd[1].events = POLLIN | POLLOUT | POLLPRI;

  while (fail_count < JDMA_UTIL_MAX_FAIL_CNT) {
    poll_fd[0].revents = 0;
    poll_fd[1].revents = 0;

    wait_on_fds = 1;
    if (img_q_count(&p_comp->inProcessQ)) {
      wait_on_fds = 2;
    }

    IDBG_HIGH("In worker thread, num fds waiting on %d", wait_on_fds);
    status = poll(poll_fd, wait_on_fds, -1);
    if (status < 0 && errno == EINTR) {
      IDBG_ERROR("Poll error exiting from worker thread, error %s continue...",
        strerror(errno));
      continue;
    } else if (status < 0) {
      IDBG_ERROR("Poll error exiting from worker thread, error %s",
        strerror(errno));
      status = IMG_ERR_GENERAL;
      goto thread_exit;
    }
    process_succeeded = 1;

    /* Process the message */
    if (poll_fd[0].revents & (POLLIN | POLLPRI)) {
      IDBG_HIGH("Reveived message on poll, stopped wait");
      status = jpegdma_util_proc_msg(p_comp);
      if (IMG_ERROR(status)) {
        IDBG_ERROR("Error process message");
        fail_count++;
        process_succeeded = 0;
      }

      QIMG_LOCK(&p_comp->b.mutex);
      if (p_comp->b.thread_exit) {
        QIMG_UNLOCK(&p_comp->b.mutex);
        goto thread_exit;
      }
      QIMG_UNLOCK(&p_comp->b.mutex);
    }

    /* Check id jpeg dma driver returned buffer after processing */
    if ((poll_fd[1].revents & POLLIN) && (poll_fd[1].revents & POLLOUT)) {

      IDBG_HIGH("Poll from HW, stopped wait");
      status = jpegdma_util_dequeue_bundle(p_comp);
      if (IMG_ERROR(status)) {
        IDBG_ERROR("Can not dequeue bundle");
        fail_count++;
        process_succeeded = 0;
      }
    }

    if (process_succeeded) {
      fail_count = 0;
    }
  }

thread_exit:
  QIMG_LOCK(&p_comp->b.mutex);

  if (p_comp->b.state == IMG_STATE_STARTED) {
    jpegdma_util_flush_queues(p_comp);
    jpegdma_util_stop_streaming(p_comp);
  }
  p_comp->b.state = IMG_STATE_INIT;

  /* reset component to default and move to IDLE state */
  memset(&p_comp->params, 0x0, sizeof(jpegdma_params_t));
  close(p_comp->msg_pipefd[0]);
  close(p_comp->msg_pipefd[1]);
  close(p_comp->ack_pipefd[0]);
  close(p_comp->ack_pipefd[1]);
  close(p_comp->fd_drv);
  p_comp->fd_drv = -1;
  p_comp->userdata = NULL;
  p_comp->jpegdma_cb = NULL;
  QIMG_UNLOCK(&p_comp->b.mutex);

  img_q_deinit(&p_comp->inProcessQ);
  p_comp->b.ops.deinit(&p_comp->b);

  IDBG_HIGH("Thread exit");
  return;
}

/**
 * Function: jpegdma_util_stop_thread
 *
 * Description: function to stop worker thread
 *
 * Input parameters:
 *   @p_comp: Pointer to jpegdma component.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
int32_t jpegdma_util_stop_thread_loop(jpegdma_comp_t *p_comp)
{
  int32_t status = IMG_SUCCESS;
  jpegdma_util_msg_mode_t mode = JDMA_MSG_MODE_NON_BLOCK;
  jpegdma_util_msg_type_t msg_type = JDMA_MSG_EXIT;

  status = jpegdma_util_send_msg(p_comp, msg_type, mode, NULL);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("send message failed");
  }
  return status;
}
