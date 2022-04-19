/*************************************************************
* Copyright (c) 2016 Qualcomm Technologies, Inc.             *
* All Rights Reserved.                                       *
* Confidential and Proprietary - Qualcomm Technologies, Inc. *
*************************************************************/

#include "jpegdma_util_intf.h"

/** MODULE_MASK:
 *
 * Mask to enable dynamic logging
 **/
#undef MODULE_MASK
#define MODULE_MASK IMGLIB_JPEGDMA_SHIFT

/**
 * Function: jpegdma_thread_loop
 *
 * Description: Main algorithm thread loop
 *
 * Input parameters:
 *   @data: The pointer to the component object
 *
 * Return values:
 *     NULL
 *
 * Notes: none
 **/
void *jpegdma_thread_loop(void *handle)
{
  jpegdma_comp_t *p_comp = (jpegdma_comp_t *)handle;

  IDBG_HIGH("Start thread loop");
  jpegdma_util_start_thread_loop(p_comp);
  return NULL;
}

/**
 * Function: jpegdma_comp_queue_buffer
 *
 * Description: This function is used to send buffers to the component
 *
 * Input parameters:
 *   @handle - The pointer to the component handle.
 *   @p_frame - The frame buffer which needs to be processed by
 *     the imaging library
 *   @type: image type
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: Does not support IMG_IN_OUT type.
 **/
int jpegdma_comp_queue_buffer(void *handle, img_frame_t *p_frame,
  img_type_t type)
{
  jpegdma_comp_t *p_comp = (jpegdma_comp_t *)handle;
  int status = IMG_SUCCESS;
  jpegdma_buffer_type_t buff_type;

  if ((!p_comp) || (!p_frame) || (type == IMG_IN_OUT)) {
    IDBG_ERROR("invalid input, p_comp %p, p_frame %p, type %d",
      p_comp, p_frame, type);
    return IMG_ERR_INVALID_INPUT;
  }

  if (!img_comp_is_curr_state(&p_comp->b.mutex, &p_comp->b.state,
    IMG_STATE_STARTED)) {
    IDBG_ERROR("invalid operation");
    return IMG_ERR_INVALID_OPERATION;
  }

  switch (type) {
  case IMG_IN:
    buff_type = JDMA_IN_BUFFER;
    break;
  case IMG_OUT:
    buff_type = JDMA_OUT_BUFFER;
    break;
  default:
    IDBG_ERROR("Invalid image type");
    return IMG_ERR_INVALID_INPUT;
  }

  status = jpegdma_util_send_buffers(p_comp, p_frame, buff_type);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("queue buffer failed for frame idx %d, type %d",
      p_frame->frame_id, type);
    return status;
  }

  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_comp_queue_meta
 *
 * Description: This function is used to send meta buffers to
 *   the component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   @p_metabuffer - The meta buffer which needs to be
 *     processed by the imaging library
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
int jpegdma_comp_queue_meta(void *handle, img_meta_t *p_metabuffer)
{
  jpegdma_comp_t *p_comp = (jpegdma_comp_t *)handle;
  int status = IMG_SUCCESS;

  if (!p_comp || !p_metabuffer) {
    IDBG_ERROR("invalid input, p_comp %p, p_metabuffer %p",
      p_comp, p_metabuffer);
    return IMG_ERR_INVALID_INPUT;
  }

  if (!img_comp_is_curr_state(&p_comp->b.mutex, &p_comp->b.state,
    IMG_STATE_STARTED)) {
    IDBG_ERROR("invalid operation");
    return IMG_ERR_INVALID_OPERATION;
  }

  status = jpegdma_util_send_buffers(p_comp, p_metabuffer,
    JDMA_META_BUFFER);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("queue meta buffer failed for frame %d",
      p_metabuffer->frame_id);
    return status;
  }

  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_comp_abort
 *
 * Description: Aborts the execution of jpegdma
 *
 * Input parameters:
 *   @handle - The pointer to the component handle.
 *   @p_data - The pointer to the command structure.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
int jpegdma_comp_abort(void *handle, void *p_data)
{
  jpegdma_comp_t *p_comp = (jpegdma_comp_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_HIGH("E");

  if (!p_comp) {
    IDBG_ERROR("invalid input");
    return IMG_ERR_INVALID_INPUT;
  }

  if (img_comp_check_state(&p_comp->b.mutex, &p_comp->b.state,
    IMG_STATE_STOPPED)) {
    IDBG_ERROR("invalid operation");
    return IMG_ERR_INVALID_OPERATION;
  }

  status = jpegdma_util_abort(p_comp);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("jpegdma util start failed");
    return status;
  }

  status = img_comp_set_state(&p_comp->b.mutex, &p_comp->b.state,
    IMG_STATE_STOPPED);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("invalid state change operation");
    return status;
  }

  IDBG_HIGH("X");
  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_comp_start
 *
 * Description: Start the execution of jpegdma
 *
 * Input parameters:
 *   @handle: The pointer to the component handle.
 *   @p_data: The pointer to the command structure.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
int jpegdma_comp_start(void *handle, void *p_data)
{
  jpegdma_comp_t *p_comp = (jpegdma_comp_t *) handle;
  int status = IMG_SUCCESS;

  IDBG_HIGH("E");

  if (!p_comp) {
    IDBG_ERROR("invalid input");
    return IMG_ERR_INVALID_INPUT;
  }

  if (img_comp_check_state(&p_comp->b.mutex, &p_comp->b.state,
    IMG_STATE_STARTED)) {
    IDBG_ERROR("invalid operation");
    return IMG_ERR_INVALID_OPERATION;
  }

  status = jpegdma_util_start(p_comp);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("jpegdma util start failed");
    return status;
  }

  status = img_comp_set_state(&p_comp->b.mutex, &p_comp->b.state,
    IMG_STATE_STARTED);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("invalid state change operation");
    goto error_set_state;
  }

  IDBG_HIGH("X");
  return IMG_SUCCESS;

error_set_state:
  jpegdma_util_abort(p_comp);
  return status;
}

/**
 * Function: jpegdma_comp_get_param
 *
 * Description: Gets jpegdma parameters
 *
 * Input parameters:
 *   @handle: The pointer to the component handle.
 *   @param: The type of the parameter
 *   @p_data: The pointer to the paramter structure. The
 *     structure for each paramter type will be defined in
 *     denoise.h
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
int jpegdma_comp_get_param(void *handle, img_param_type param, void *p_data)
{
  jpegdma_comp_t *p_comp = (jpegdma_comp_t *)handle;
  int status = IMG_SUCCESS;

  if (!p_comp) {
    IDBG_ERROR("invalid input");
    return IMG_ERR_INVALID_INPUT;
  }

  status = p_comp->b.ops.get_parm(&p_comp->b, param, p_data);
  if (status < 0) {
    return status;
  }

  switch (param) {
    case QIMG_PARAM_SRC_DIM : {
      img_dim_t *p_src_dim = (img_dim_t *) p_data;
      if (NULL == p_src_dim) {
        IDBG_ERROR("invalid src dimension ptr");
          return IMG_ERR_INVALID_INPUT;
      }
      *p_src_dim = p_comp->params.src_dim;
      break;
    }
    case QIMG_PARAM_DST_DIM : {
      img_dim_t *p_dest_dim = (img_dim_t *) p_data;
      if (NULL == p_dest_dim) {
        IDBG_ERROR("invalid dest dimension ptr");
        return IMG_ERR_INVALID_INPUT;
      }
      *p_dest_dim = p_comp->params.dest_dim;
      break;
    }
    case QIMG_PARAM_YUV_FMT : {
      cam_format_t *p_yuv_fmt = (cam_format_t *) p_data;
      if (NULL == p_yuv_fmt) {
        IDBG_ERROR("invalid yuv format ptr");
        return IMG_ERR_INVALID_INPUT;
      }
      *p_yuv_fmt = p_comp->params.yuv_format;
      break;
    }
    case QIMG_PARAM_CROP_DIM : {
      img_rect_t *p_crop = (img_rect_t *) p_data;
      if (NULL == p_crop) {
        IDBG_ERROR("invalid crop ptr");
        return IMG_ERR_INVALID_INPUT;
      }
      *p_crop = p_comp->params.crop;
      break;
    }
    case QIMG_PARAM_FRAME_RATE : {
      uint32_t *p_framerate = (uint32_t *) p_data;
      if (NULL == p_framerate) {
        IDBG_ERROR("invalid framerate ptr");
        return IMG_ERR_INVALID_INPUT;
      }
      *p_framerate = p_comp->params.framerate;
      break;
    }
    case QIMG_PARAM_SRC_BUF_SZ : {
      uint32_t *p_src_buf_sz = (uint32_t *) p_data;
      if (NULL == p_src_buf_sz) {
        IDBG_ERROR("invalid src buf size ptr");
        return IMG_ERR_INVALID_INPUT;
      }
      *p_src_buf_sz = p_comp->params.src_buf_sz;
      break;
    }
    case QIMG_PARAM_DST_BUF_SZ : {
      uint32_t *p_dest_buf_sz = (uint32_t *) p_data;
      if (NULL == p_dest_buf_sz) {
        IDBG_ERROR("invalid dest buf size ptr");
        return IMG_ERR_INVALID_INPUT;
      }
      *p_dest_buf_sz = p_comp->params.dest_buf_sz;
      break;
    }
    case QIMG_PARAM_MAX_DS : {
      float *p_max_ds = (float *) p_data;
      if (NULL == p_max_ds) {
        IDBG_ERROR("invalid max downscale factor ptr");
        return IMG_ERR_INVALID_INPUT;
      }
      *p_max_ds = p_comp->params.max_ds_factor;
      break;
    }
    default: {
      IDBG_ERROR("invalid parameter %d", param);
      return IMG_ERR_INVALID_INPUT;
    }
  }
  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_comp_set_param
 *
 * Description: Set jpegdma parameters
 *
 * Input parameters:
 *   @handle: The pointer to the component handle.
 *   @param: The type of the parameter
 *   @p_data: The pointer to the paramter structure.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
int jpegdma_comp_set_param(void *handle, img_param_type param, void *p_data)
{
  jpegdma_comp_t *p_comp = (jpegdma_comp_t *)handle;
  int status = IMG_SUCCESS;

  if (!p_comp) {
    IDBG_ERROR("invalid input");
    return IMG_ERR_INVALID_INPUT;
  }

  if (img_comp_check_state(&p_comp->b.mutex, &p_comp->b.state,
    IMG_STATE_STARTED)) {
    IDBG_ERROR("invalid operation");
    return IMG_ERR_INVALID_OPERATION;
  }

  status = p_comp->b.ops.set_parm(&p_comp->b, param, p_data);
  if (status < 0) {
    return status;
  }

  switch (param) {
    case QIMG_PARAM_SRC_DIM : {
      img_dim_t *p_src_dim = (img_dim_t *) p_data;
      if (NULL == p_src_dim) {
        IDBG_ERROR("invalid src dimension");
        return IMG_ERR_INVALID_INPUT;
      }
      p_comp->params.src_dim = *p_src_dim;
      IDBG_HIGH("src, width x height %d x %d, stride x scan %d x %d",
        p_src_dim->width, p_src_dim->height,
        p_src_dim->stride, p_src_dim->scanline);
      break;
    }
    case QIMG_PARAM_DST_DIM : {
      img_dim_t *p_dest_dim = (img_dim_t *) p_data;
      if (NULL == p_dest_dim) {
        IDBG_ERROR("invalid dest dimension");
        return IMG_ERR_INVALID_INPUT;
      }
      p_comp->params.dest_dim = *p_dest_dim;
      IDBG_HIGH("dest, width x height %d x %d, stride x scan %d x %d",
        p_dest_dim->width, p_dest_dim->height,
        p_dest_dim->stride, p_dest_dim->scanline);
      break;
    }
    case QIMG_PARAM_YUV_FMT : {
      cam_format_t *p_yuv_fmt = (cam_format_t *) p_data;
      if (NULL == p_yuv_fmt) {
        IDBG_ERROR("invalid yuv format");
        return IMG_ERR_INVALID_INPUT;
      }
      p_comp->params.yuv_format = *p_yuv_fmt;
      IDBG_HIGH("yuv format %d", *p_yuv_fmt);
      break;
    }
    case QIMG_PARAM_CROP_DIM : {
      img_rect_t *p_crop = (img_rect_t *) p_data;
      if (NULL == p_crop) {
        IDBG_ERROR("invalid crop");
        return IMG_ERR_INVALID_INPUT;
      }
      p_comp->params.crop = *p_crop;
      IDBG_HIGH("crop, left x top %d x %d, width x height %d x %d",
        p_crop->pos.x, p_crop->pos.y,
        p_crop->size.width, p_crop->size.height);
      break;
    }
    case QIMG_PARAM_FRAME_RATE : {
      uint32_t *p_framerate = (uint32_t *) p_data;
      if (NULL == p_framerate) {
        IDBG_ERROR("invalid framerate");
        return IMG_ERR_INVALID_INPUT;
      }
      p_comp->params.framerate = *p_framerate;
      IDBG_HIGH("framerate %d", *p_framerate);
      break;
    }
    case QIMG_PARAM_SRC_BUF_SZ :
    case QIMG_PARAM_DST_BUF_SZ :
    default : {
      IDBG_ERROR("invalid parameter type %d", param);
      return IMG_ERR_INVALID_INPUT;
    }
  }
  return status;
}

/**
 * Function: jpegdma_comp_init
 *
 * Description: Initializes the jpegdma component
 *
 * Input parameters:
 *   @handle: The pointer to the component handle.
 *   @p_userdata: the handle which is passed by the client
 *   @p_data: The pointer to the parameter which is required
 *     during the init phase, in this case it is the buff done
 *     call back function.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
int jpegdma_comp_init(void *handle, void* p_userdata, void *p_data)
{
  jpegdma_comp_t *p_comp = (jpegdma_comp_t *)handle;
  int status = IMG_SUCCESS;
  int drv_fd;

  IDBG_HIGH("E");

  if (!p_comp || !p_userdata || !p_data) {
    IDBG_ERROR("invalid input, p_comp %p, p_userdata %p, p_data %p",
      p_comp, p_userdata, p_data);
    return IMG_ERR_INVALID_INPUT;
  }

  status = img_comp_check_state(&p_comp->b.mutex, &p_comp->b.state,
    IMG_STATE_INIT);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("invalid operation");
    return IMG_ERR_INVALID_OPERATION;
  }

  drv_fd = open(p_comp->p_drv_name, O_RDWR | O_NONBLOCK);
  if (drv_fd < 0) {
    IDBG_ERROR("Can not open hw fd");
    return IMG_ERR_GENERAL;
  }
  p_comp->fd_drv = drv_fd;

  IDBG_HIGH("open driver %s for %p, fd %d",
    p_comp->p_drv_name, p_userdata, p_comp->fd_drv);

  status = jpegdma_util_get_max_scale(p_comp->fd_drv,
    &p_comp->params.max_ds_factor);
  if (IMG_ERROR(status))
    goto error_get_max_scale;

  status = pipe(p_comp->msg_pipefd);
  if (status < 0) {
    IDBG_ERROR("Can not create msg pipe");
    status = IMG_ERR_GENERAL;
    goto error_msg_pipe;
  }

  status = pipe(p_comp->ack_pipefd);
  if (status < 0) {
    IDBG_ERROR("Can not create msg ack pipe");
    status = IMG_ERR_GENERAL;
    goto error_ack_pipe;
  }

  status = p_comp->b.ops.init(&p_comp->b, p_userdata, NULL);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("p_comp->b.ops.init returned %d", status);
    goto error_init;
  }

  status = p_comp->b.ops.start(&p_comp->b, NULL);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("Can not start thread");
    goto error_start_thread;
  }

  p_comp->userdata = p_userdata;
  p_comp->jpegdma_cb = p_data;
  img_q_init(&p_comp->inProcessQ, "inProcessQ");
  img_q_init(&p_comp->toProcessQ, "toProcessQ");

  /* Put component back to init state */
  QIMG_LOCK(&p_comp->b.mutex);
  p_comp->b.state = IMG_STATE_INIT;
  QIMG_UNLOCK(&p_comp->b.mutex);

  IDBG_HIGH("X");
  return status;

error_start_thread:
  p_comp->b.ops.deinit(&p_comp->b);

error_init:
  close(p_comp->ack_pipefd[0]);
  close(p_comp->ack_pipefd[1]);

error_ack_pipe:
  close(p_comp->msg_pipefd[0]);
  close(p_comp->msg_pipefd[1]);

error_get_max_scale:
error_msg_pipe:
  close(p_comp->fd_drv);
  p_comp->fd_drv = -1;

  IDBG_ERROR("failed %d", status);
  return status;
}

/**
 * Function: jpegdma_comp_deinit
 *
 * Description: Deinitializes the jpegdma component
 *
 * Input parameters:
 *   @handle: The pointer to the component handle.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
int jpegdma_comp_deinit(void *handle)
{
  jpegdma_comp_t *p_comp = (jpegdma_comp_t *)handle;

  IDBG_HIGH("E");
  if (!p_comp) {
    IDBG_ERROR("invalid input");
    return IMG_ERR_INVALID_INPUT;
  }

  if (img_comp_check_state(&p_comp->b.mutex, &p_comp->b.state,
    IMG_STATE_IDLE)) {
    IDBG_ERROR("invalid operation");
    return IMG_ERR_INVALID_OPERATION;
  }

  jpegdma_util_stop_thread_loop(p_comp);
  if (!pthread_equal(pthread_self(), p_comp->b.threadid)) {
    IDBG_HIGH("thread id 0x%x 0x%x",(uint32_t)pthread_self(),
      (uint32_t)p_comp->b.threadid);
    pthread_join(p_comp->b.threadid, NULL);
  }

  free(p_comp);

  IDBG_HIGH("X");
  return IMG_SUCCESS;
}

/**
 * Function: jpegdma_comp_create
 *
 * Description: This function is used to create jpegdma
 *   component
 *
 * Input parameters:
 *   @handle: library handle
 *   @p_ops: The pointer to img_component_t object.
 *     This object contains the handle and the function
 *     pointers for communicating with the imaging component.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
int jpegdma_comp_create(void* handle, img_component_ops_t *p_ops)
{
  jpegdma_comp_t *p_comp = NULL;
  int status = IMG_SUCCESS;

  IDBG_HIGH("E");

  if (NULL == handle) {
    IDBG_ERROR("util not loaded");
    return IMG_ERR_INVALID_OPERATION;
  }

  if (NULL == p_ops) {
    IDBG_ERROR("failed p_ops null");
    return IMG_ERR_INVALID_INPUT;
  }

  p_comp = (jpegdma_comp_t *)malloc(sizeof(jpegdma_comp_t));
  if (NULL == p_comp) {
    IDBG_ERROR("failed, no mem");
    return IMG_ERR_NO_MEMORY;
  }

  memset(p_comp, 0x0, sizeof(jpegdma_comp_t));
  status = img_comp_create(&p_comp->b);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("Error, Can not create component");
    free(p_comp);
    return status;
  }

  /*set the main thread */
  p_comp->b.thread_loop = jpegdma_thread_loop;
  p_comp->b.p_core = p_comp;

  /* Store pointer of jpeg dma node name */
  p_comp->p_drv_name = (char *)handle;

  /* copy the ops table from the base component */
  *p_ops = p_comp->b.ops;
  p_ops->init             = jpegdma_comp_init;
  p_ops->deinit           = jpegdma_comp_deinit;
  p_ops->set_parm         = jpegdma_comp_set_param;
  p_ops->get_parm         = jpegdma_comp_get_param;
  p_ops->start            = jpegdma_comp_start;
  p_ops->abort            = jpegdma_comp_abort;
  p_ops->queue_buffer     = jpegdma_comp_queue_buffer;
  p_ops->queue_metabuffer = jpegdma_comp_queue_meta;

  p_ops->handle = (void *)p_comp;

  /* Init component default values */
  p_comp->fd_drv = -1;
  IDBG_HIGH("X");
  return status;
}

/**
 * Function: jpegdma_comp_load
 *
 * Description: Try to discover jpeg dma device node, if device
 *   node is not present load will fail.
 *
 * Input parameters:
 *   @name: library name
 *   @handle: library handle
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
int jpegdma_comp_load(const char* name, void** handle)
{
  IMG_UNUSED(name);
  int status = IMG_SUCCESS;
  char *p_drv_name;

  IMG_INIT_LOGGING();

  IDBG_HIGH("E");

  if (handle == NULL) {
    IDBG_ERROR("invalid input");
    return IMG_ERR_INVALID_INPUT;
  }

  *handle = NULL;
  status = jpegdma_util_get_dev_name(&p_drv_name);

  if (IMG_ERROR(status)) {
    IDBG_ERROR("Error cannot find driver name");
    return IMG_ERR_NOT_FOUND;
  }

  IDBG_HIGH("Component load successfully device name %s", p_drv_name);
  *(char **)handle = p_drv_name;

  IDBG_HIGH("X");
  return status;
}

/**
 * Function: jpegdma_comp_unload
 *
 * Description: This function is used to unload jpegdma comp
 *
 * Input parameters:
 *   @handle: library handle
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
int jpegdma_comp_unload(void* handle)
{
  IDBG_HIGH("E");

  if (!handle) {
    IDBG_ERROR("invalid handler");
    return IMG_ERR_INVALID_INPUT;
  }

  free(handle);
  IDBG_HIGH("X");
  return IMG_SUCCESS;
}
