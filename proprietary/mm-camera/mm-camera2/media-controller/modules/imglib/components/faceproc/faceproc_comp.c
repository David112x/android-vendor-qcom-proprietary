/**********************************************************************
*  Copyright (c) 2013-2014, 2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "faceproc_comp.h"
#include <math.h>

#include <cutils/properties.h>
/**
 * CONSTANTS and MACROS
 **/

/** FD_SET_PROCESS_BUF_CNT_LOCKED
 *   @p: pointer to the image component
 *   @b: buffer cnt need to be set
 *   @m: pointer to the mutex
 *
 *   Set buffer count value with component locked.
 **/
#define FD_SET_PROCESS_BUF_CNT_LOCKED(p, b, m) ({ \
  pthread_mutex_lock(m); \
  (p)->processing_buff_cnt = (b); \
  pthread_mutex_unlock(m); \
})

#ifdef FACEPROC_USE_NEW_WRAPPER
static bool g_engine_loaded = false;
#else
static faceproc_lib_t g_faceproc_lib;
#endif

int faceproc_comp_abort(void *handle, void *p_data);

/**
 * Function: faceproc_comp_init
 *
 * Description: Initializes the Qualcomm faceproc component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_userdata - the handle which is passed by the client
 *   p_data - The pointer to the parameter which is required during the
 *            init phase
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int faceproc_comp_init(void *handle, void* p_userdata, void *p_data)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  status = p_comp->b.ops.init(&p_comp->b, p_userdata, p_data);
  if (status < 0)
    return status;

  p_comp->mode = FACE_DETECT;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_deinit
 *
 * Description: Un-initializes the face processing component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int faceproc_comp_deinit(void *handle)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  status = faceproc_comp_abort(handle, NULL);
  if (status < 0)
    return status;

  status = p_comp->b.ops.deinit(&p_comp->b);
  if (status < 0)
    return status;

  status = faceproc_comp_eng_destroy(p_comp);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] failed, status=%d", __func__, __LINE__, status);
    return status;
  }
  free(p_comp);
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_flush_buffers
 *
 * Description: Flush buffers from component queue.
 *
 * Arguments:
 *   @p_comp: Pointer to Faceproc component instance.
 *
 * Return values:
 *   none.
 **/
void faceproc_comp_flush_buffers(faceproc_comp_t *p_comp)
{
  int i;
  int count;
  int status;
  img_frame_t *p_frame;

  count = img_q_count(&p_comp->b.inputQ);

  for (i = 0; i < count; i++) {
    p_frame = img_q_dequeue(&p_comp->b.inputQ);
    if (NULL == p_frame) {
      IDBG_ERROR("Fail to dequeue input buffer");
      continue;
    }
    status = img_q_enqueue(&p_comp->b.outputQ, p_frame);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("Fail to enqueue input buffer status=%d", status);
      continue;
    }
    IMG_SEND_EVENT(&p_comp->b, QIMG_EVT_BUF_DONE);
  }
}

/**
 * Function: faceproc_comp_set_param
 *
 * Description: Set faceproc parameters
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int faceproc_comp_set_param(void *handle, img_param_type param, void *p_data)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;

  status = p_comp->b.ops.set_parm(&p_comp->b, param, p_data);
  if (status < 0)
    return status;

  IDBG_MED("param 0x%x", param);
  switch (param) {
  case QWD_FACEPROC_CFG: {
    faceproc_config_t *p_config = (faceproc_config_t *)p_data;

    if (NULL == p_config) {
      IDBG_ERROR("invalid faceproc config");
      return IMG_ERR_INVALID_INPUT;
    }

    uint32_t current_fd_face_info_mask = p_comp->config.fd_face_info_mask;

    p_comp->config = *p_config;
    p_comp->config_set = TRUE;

    if (current_fd_face_info_mask != p_config->fd_face_info_mask) {
      IDBG_HIGH("Update Facial parts feature config. current=0x%x, new=0x%x",
        current_fd_face_info_mask, p_config->fd_face_info_mask);
      p_comp->need_config_update = TRUE;
    }
  }
  break;
  case QWD_FACEPROC_TRY_SIZE: {
    faceproc_frame_cfg_t *p_config = (faceproc_frame_cfg_t *)p_data;

    if (NULL == p_config) {
      IDBG_ERROR("invalid faceproc config");
      return IMG_ERR_INVALID_INPUT;
    }
  }
    break;
  case QWD_FACEPROC_MODE: {
    faceproc_mode_t *p_mode = (faceproc_mode_t *)p_data;

    if (NULL == p_mode) {
      IDBG_ERROR("invalid faceproc mode");
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->mode = *p_mode;
    IDBG_MED("mode %d", p_comp->mode);
  }
    break;
  case QWD_FACEPROC_CHROMATIX: {
    fd_chromatix_t *p_chromatix = (fd_chromatix_t *)p_data;

    if (NULL == p_chromatix) {
      IDBG_ERROR("invalid faceproc chromatix");
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->fd_chromatix = *p_chromatix;
    p_comp->is_chromatix_changed = TRUE;
    IDBG_MED("Set chromatix in state %d", p_base->state);
  }
    break;
  case QWD_FACEPROC_DYNAMIC_UPDATE_CONFIG: {
    faceproc_dynamic_cfg_params_t *dyn_cfg =
      (faceproc_dynamic_cfg_params_t *)p_data;
    p_comp->dynamic_cfg_params = *dyn_cfg;
    if (p_comp->dynamic_cfg_params.lux_index != -1) {
      p_comp->need_config_update = TRUE;
    }
    IDBG_MED("Updated lux index %d", p_comp->dynamic_cfg_params.lux_index);
  }
    break;
  case QWD_FACEPROC_DUMP_DATA: {
    faceproc_dump_mode_t *p_dump_mode = (faceproc_dump_mode_t *)p_data;
    if (NULL == p_dump_mode) {
      IDBG_ERROR("invalid faceproc dump mode");
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->dump_mode = *p_dump_mode;
    IDBG_MED("Set dump mode %d", p_comp->dump_mode);
  }
    break;
  case QWD_FACEPROC_DEBUG_SETTINGS: {
    faceproc_debug_settings_t *p_debug_settings =
      (faceproc_debug_settings_t *)p_data;
    if (NULL == p_debug_settings) {
      IDBG_ERROR("invalid faceproc debug settings");
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->debug_settings = *p_debug_settings;
#ifdef FACEPROC_USE_NEW_WRAPPER
    faceproc_sw_wrapper_set_debug(p_comp->p_wrapper, p_debug_settings);
#endif
  }
    break;
  case QWD_FACEPROC_ROTATION_HINT: {
    int32_t *p_device_rotation = (int32_t *)p_data;
    if (NULL == p_device_rotation) {
      IDBG_ERROR("invalid faceproc rotation hint");
      return IMG_ERR_INVALID_INPUT;
    }
    IDBG_LOW("Device rotation is %d", *p_device_rotation);
    if (p_comp->device_rotation != *p_device_rotation) {
      p_comp->device_rotation = *p_device_rotation;
      if (p_comp->fd_chromatix.enable_upfront) {
      p_comp->need_config_update = TRUE;
      }
    }
  }
    break;
  case QWD_FACEPROC_BATCH_MODE_DATA:
    break;
  default:
    IDBG_ERROR("Error param=%d", param);
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_get_param
 *
 * Description: Gets faceproc parameters
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int faceproc_comp_get_param(void *handle, img_param_type param, void *p_data)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  int status = IMG_SUCCESS;

  status = p_comp->b.ops.get_parm(&p_comp->b, param, p_data);
  if (status < 0)
    return status;

  switch (param) {
  case QWD_FACEPROC_RESULT: {
    faceproc_result_t *p_result = (faceproc_result_t *)p_data;

    if (NULL == p_result) {
      IDBG_ERROR("invalid faceproc result");
      return IMG_ERR_INVALID_INPUT;
    }
    if (!p_comp->width || !p_comp->height) {
      IDBG_ERROR("Invalid param, w=%d, h=%d",p_comp->width, p_comp->height);
      return IMG_ERR_INVALID_INPUT;
    }

    status = faceproc_comp_eng_get_output(p_comp, p_result);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("invalid faceproc result, status=%d", status);
      return status;
    }

    p_result->client_id = p_comp->client_id;
    break;
  }
  case QWD_FACEPROC_BUFF_TYPE: {
    img_buf_type_t *p_type = (img_buf_type_t *)p_data;

    if (NULL == p_type) {
      IDBG_ERROR("invalid input");
      return IMG_ERR_INVALID_INPUT;
    }
    *p_type = IMG_BUFFER_HEAP;

#ifdef FD_WITH_DSP
   char value[PROPERTY_VALUE_MAX];
   property_get("persist.vendor.camera.imglib.fddsp", value, "1");
   if (atoi(value)){
     IDBG_MED("FD in DSP mode, alloc IMG_BUFFER_ION_ADSP");
     *p_type = IMG_BUFFER_ION_ADSP;
   }
#endif
    break;
  }
  case QWD_FACEPROC_USE_INT_BUFF: {
    uint32_t *p_use_int_buff = (uint32_t *)p_data;

    if (NULL == p_use_int_buff) {
      IDBG_ERROR("invalid input");
      return IMG_ERR_INVALID_INPUT;
    }
    *p_use_int_buff = FALSE;
    break;
  }
  default:
    IDBG_ERROR("Error type %d", param);
    return IMG_ERR_INVALID_INPUT;
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_cfg_debug
 *
 * Description: Debug params for faceproc configuration
 *
 * Input parameters:
 *   p_denoiselib - library instance pointer
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static void faceproc_comp_cfg_debug(faceproc_config_t *p_config)
{
  IDBG_MED("%s:%d] FaceProc cfg hist enable %d", __func__, __LINE__,
    p_config->histogram_enable);
  IDBG_MED("%s:%d] FaceProc cfg max_height %d",
    __func__, __LINE__,
    p_config->frame_cfg.max_height);
  IDBG_MED("%s:%d] FaceProc cfg max_width %d",
    __func__, __LINE__,
    p_config->frame_cfg.max_width);
  IDBG_MED("%s:%d] FaceProc cfg face_orientation_hint %d",
    __func__, __LINE__,
    p_config->face_cfg.face_orientation_hint);
  IDBG_MED("%s:%d] FaceProc cfg max_face_size %d",
    __func__, __LINE__,
    p_config->face_cfg.max_face_size);
  IDBG_MED("%s:%d] FaceProc cfg max_num_face_to_detect %d",
    __func__, __LINE__,
    p_config->face_cfg.max_num_face_to_detect);
  IDBG_MED("%s:%d] FaceProc cfg min_face_size %d",
    __func__, __LINE__,
    p_config->face_cfg.min_face_size);
  IDBG_MED("%s:%d] FaceProc cfg rotation_range %d",
    __func__, __LINE__,
    p_config->face_cfg.rotation_range);
}

/**
 * Function: face_proc_can_wait
 *
 * Description: Queue function to check if abort is issued
 *
 * Input parameters:
 *   p_userdata - The pointer to faceproc component
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
int face_proc_can_wait(void *p_userdata)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)p_userdata;
  img_component_t *p_base = &p_comp->b;
  return !((p_base->state == IMG_STATE_STOP_REQUESTED)
    || (p_base->state == IMG_STATE_STOPPED));
}

#ifdef FD_DROP_FRAME
/**
 * Function: face_proc_can_wait
 *
 * Description: Queue function to check if abort is issued
 *
 * Input parameters:
 *   p_userdata - The pointer to faceproc component
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static int face_proc_release_frame(void *data, void *p_userdata)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)p_userdata;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_frame_t *p_frame = (img_frame_t *)data;

  status = img_q_enqueue(&p_base->outputQ, p_frame);
  if (status < 0) {
    IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
  } else {
    IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
  }
  p_comp->facedrop_cnt++;
  return status;
}
#endif
/**
 * Function: face_proc_thread_loop
 *
 * Description: Main algorithm thread loop
 *
 * Input parameters:
 *   data - The pointer to the component object
 *
 * Return values:
 *     NULL
 *
 * Notes: none
 **/
void *face_proc_thread_loop(void *handle)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_frame_t *p_frame = NULL;

  IDBG_MED("%s:%d] state %d abort %d", __func__, __LINE__,
    p_base->state, p_comp->abort_flag);

#ifdef FD_DROP_FRAME
  while (1) {

    /* wait for frame */
    img_q_wait_for_signal(&p_base->inputQ, face_proc_can_wait, p_comp);

    if (!face_proc_can_wait(p_comp)) {
      IDBG_HIGH("%s:%d] Exit the thread", __func__, __LINE__);
      break;
    }
    p_comp->facedrop_cnt = 0;
    p_frame = (img_frame_t *)img_q_get_last_entry(&p_base->inputQ,
      face_proc_release_frame, p_comp);
    if (NULL == p_frame) {
      IDBG_ERROR("%s:%d] No more elements.", __func__, __LINE__);
      continue;
    }
    IDBG_MED("%s:%d] frame drop cnt %d q_cnt %d buf_idx %d",
      __func__, __LINE__,
      p_comp->facedrop_cnt, img_q_count(&p_base->inputQ), p_frame->idx);
#else
  while ((p_frame = img_q_wait(&p_base->inputQ,
    face_proc_can_wait, p_comp)) != NULL) {
#endif

    if ((p_comp->width != FD_WIDTH(p_frame)) ||
      (p_comp->height != FD_HEIGHT(p_frame))) {
      IDBG_HIGH("%s:%d] [FD_HAL3] Update dimensions to %dx%d", __func__,
        __LINE__, FD_WIDTH(p_frame), FD_HEIGHT(p_frame));
      p_comp->width = FD_WIDTH(p_frame);
      p_comp->height = FD_HEIGHT(p_frame);
      p_comp->need_config_update = TRUE;
    }

    FD_SET_PROCESS_BUF_CNT_LOCKED(p_comp, 1, &p_base->mutex);
    status = faceproc_comp_eng_exec(p_comp, p_frame);
    FD_SET_PROCESS_BUF_CNT_LOCKED(p_comp, 0, &p_base->mutex);
    if (status != 0) {
      IDBG_ERROR("%s:%d] frameproc exec error %d", __func__, __LINE__,
        status);
      status = img_q_enqueue(&p_base->outputQ, p_frame);
      if (status < 0) {
        IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
      } else {
        IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
      }
      IMG_SEND_EVENT(p_base, QIMG_EVT_ERROR);
      continue;
    }
    IDBG_MED("%s:%d] state %d abort %d", __func__, __LINE__,
      p_base->state, p_comp->abort_flag);

    if (IMG_CHK_ABORT_LOCKED(p_base, &p_base->mutex)) {
      /* On abort release current frame and flush all pending buffers */
      status = img_q_enqueue(&p_base->outputQ, p_frame);
      if (IMG_SUCCEEDED(status)) {
        IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
      }
      faceproc_comp_flush_buffers(p_comp);
      return IMG_SUCCESS;
    }

    p_comp->client_id = p_frame->info.client_id;
    IMG_SEND_EVENT(p_base, QIMG_EVT_FACE_PROC);

    status = img_q_enqueue(&p_base->outputQ, p_frame);
    if (status != 0) {
      IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
    } else {
      IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
    }
  }

  IDBG_MED("%s:%d] state %d abort %d", __func__, __LINE__,
    p_base->state, p_comp->abort_flag);
  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_STOPPED;
  pthread_mutex_unlock(&p_base->mutex);
  IMG_SEND_EVENT(p_base, QIMG_EVT_DONE);
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_start
 *
 * Description: Start the execution of faceproc
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type will be defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_start(void *handle, void *p_data)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;

  p_comp->is_chromatix_changed = FALSE;
  p_comp->need_config_update = FALSE;
  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT) ||
    (NULL == p_base->thread_loop)) {
    IDBG_ERROR("%s:%d] Error state %d", __func__, __LINE__,
      p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_NOT_SUPPORTED;
  }

  if (!p_comp->config_set) {
    IDBG_ERROR("%s:%d] error config not set", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return status;
  }
  faceproc_comp_cfg_debug(&p_comp->config);

  status = faceproc_comp_eng_config(p_comp);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    /*In DSP case if eng config fails it will block the preview */
    return status;
  }

  IMG_CHK_ABORT_UNLK_RET(p_base, &p_base->mutex);

  /* flush the queues */
  img_q_flush(&p_base->inputQ);
  img_q_flush(&p_base->outputQ);

  pthread_mutex_unlock(&p_base->mutex);

  status = p_comp->b.ops.start(&p_comp->b, p_data);
  if (status < 0)
    return status;

  return status;
}

/**
 * Function: faceproc_comp_abort
 *
 * Description: Aborts the execution of faceproc
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type is defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int faceproc_comp_abort(void *handle, void *p_data)
{
  IMG_UNUSED(p_data);

  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  img_component_t *p_base = (img_component_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] state %d", __func__, __LINE__, p_base->state);
  pthread_mutex_lock(&p_base->mutex);
  if (IMG_STATE_STARTED != p_base->state) {
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_SUCCESS;
  }
  p_base->state = IMG_STATE_STOP_REQUESTED;
  pthread_mutex_unlock(&p_base->mutex);
  /*signal the thread*/
  img_q_signal(&p_base->inputQ);

  if (!pthread_equal(pthread_self(), p_base->threadid)) {
    IDBG_MED("%s:%d] thread id %d %d", __func__, __LINE__,
      (uint32_t)pthread_self(), (uint32_t)p_base->threadid);
    pthread_join(p_base->threadid, NULL);
  }

  /* destroy the handle */
  status = faceproc_comp_eng_destroy(p_comp);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return status;
  }

  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_INIT;
  pthread_mutex_unlock(&p_base->mutex);
  IDBG_HIGH("%s:%d] X", __func__, __LINE__);
  return status;
}

/**
 * Function: faceproc_comp_process
 *
 * Description: This function is used to send any specific commands for the
 *              faceproc component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   cmd - The command type which needs to be processed
 *   p_data - The pointer to the command payload
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int faceproc_comp_process (void *handle, img_cmd_type cmd, void *p_data)
{
  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  int status;

  status = p_comp->b.ops.process(&p_comp->b, cmd, p_data);
  if (status < 0)
    return status;

  return 0;
}

/**
 * Function: faceproc_comp_queue_buffer
 *
 * Description: This function is used to send buffers to the component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_frame - The frame buffer which needs to be processed by the imaging
 *             library
 *   @type: image type (main image or thumbnail image)
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int faceproc_comp_queue_buffer(void *handle, img_frame_t *p_frame,
  img_type_t type)
{
  if (!handle || !p_frame || type != IMG_IN) {
    IDBG_ERROR("invalid input");
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_comp_t *p_comp = (faceproc_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_queue_t *queue = &p_base->inputQ;
  unsigned int count = img_q_count(queue);

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT)
    && (p_base->state != IMG_STATE_STARTED)) {
    IDBG_ERROR("%s:%d] Error %d", __func__, __LINE__, p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_INVALID_OPERATION;
  }

  count += p_comp->processing_buff_cnt;
  if (count  > p_comp->fd_chromatix.input_pending_buf) {
    IDBG_MED("%s:%d] Drop the frame %d", __func__, __LINE__, count);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_BUSY;
  }

  status = img_q_enqueue(queue, p_frame);
  if (status < 0) {
    IDBG_ERROR("%s:%d] Error enqueue", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return status;
  }
  IDBG_MED("%s:%d] q_count %d", __func__, __LINE__, img_q_count(queue));
  img_q_signal(queue);

  pthread_mutex_unlock(&p_base->mutex);
  return IMG_SUCCESS;
}


/**
 * Function: faceproc_comp_create
 *
 * Description: This function is used to create Qualcomm faceproc component
 *
 * Input parameters:
 *   @handle: library handle
 *   @p_ops - The pointer to img_component_t object. This object
 *            contains the handle and the function pointers for
 *            communicating with the imaging component.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_NO_MEMORY
 *     IMG_ERR_INVALID_INPUT
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int faceproc_comp_create(void* handle, img_component_ops_t *p_ops)
{
  IMG_UNUSED(handle);

  faceproc_comp_t *p_comp = NULL;
  int status;

#ifdef FACEPROC_USE_NEW_WRAPPER
  if (g_engine_loaded == false) {
#else
  if (NULL == g_faceproc_lib.ptr) {
#endif
    IDBG_ERROR("library not loaded");
    return IMG_ERR_INVALID_OPERATION;
  }

  p_comp = (faceproc_comp_t *)malloc(sizeof(faceproc_comp_t));
  if (NULL == p_comp) {
    IDBG_ERROR("failed");
    return IMG_ERR_NO_MEMORY;
  }

  if (NULL == p_ops) {
    IDBG_ERROR("failed");
    status = IMG_ERR_INVALID_INPUT;
    goto error;
  }

  memset(p_comp, 0x0, sizeof(faceproc_comp_t));
  status = img_comp_create(&p_comp->b);
  if (status < 0) {
    goto error;
  }

  p_comp->clip_face_data = 1;
  p_comp->device_rotation = -1;
  p_comp->dynamic_cfg_params.lux_index = -1;
#ifndef FACEPROC_USE_NEW_WRAPPER
  p_comp->p_lib = &g_faceproc_lib;
#endif

  /*set the main thread*/
  p_comp->b.thread_loop = face_proc_thread_loop;
  p_comp->b.p_core = p_comp;

  /* copy the ops table from the base component */
  *p_ops = p_comp->b.ops;
  p_ops->init            = faceproc_comp_init;
  p_ops->deinit          = faceproc_comp_deinit;
  p_ops->set_parm        = faceproc_comp_set_param;
  p_ops->get_parm        = faceproc_comp_get_param;
  p_ops->start           = faceproc_comp_start;
  p_ops->abort           = faceproc_comp_abort;
  p_ops->process         = faceproc_comp_process;
  p_ops->queue_buffer    = faceproc_comp_queue_buffer;

  p_ops->handle = (void *)p_comp;
  return IMG_SUCCESS;

error:
  IDBG_ERROR("failed %d", status);
  if (p_comp) {
    free(p_comp);
    p_comp = NULL;
  }
  return status;
}

/**
 * Function: faceproc_comp_load
 *
 * Description: This function is used to load the faceproc library
 *
 * Input parameters:
 *   @name: library name
 *   @handle: library handle
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_NOT_FOUND
 *
 * Notes: none
 **/
int faceproc_comp_load(const char* name, void** handle)
{
  IMG_UNUSED(name);
  IMG_UNUSED(handle);

#ifdef FACEPROC_USE_NEW_WRAPPER
  int rc = faceproc_sw_wrapper_load(FALSE);
  if (IMG_SUCCEEDED(rc)) {
    g_engine_loaded = true;
  }
  return rc;
#else
  if (g_faceproc_lib.ptr) {
    IDBG_ERROR("library already loaded");
    return IMG_ERR_NOT_FOUND;
  }

#ifdef FD_WITH_DSP

  char value[PROPERTY_VALUE_MAX];
  #define MODULE_FDDSP_CONTROL_FLAG     ("persist.vendor.camera.imglib.fddsp")
  #define MODULE_FDDSP_CONTROL_ENABLED  ("1")
  #define MODULE_FDDSP_CONTROL_DISABLED ("0")
  #define MODULE_FDDSP_CONTROL_DEFAULT  (MODULE_FDDSP_CONTROL_ENABLED)

  property_get(MODULE_FDDSP_CONTROL_FLAG, value, MODULE_FDDSP_CONTROL_DEFAULT);
  if (!strncmp(MODULE_FDDSP_CONTROL_ENABLED,
    value,
    PROPERTY_VALUE_MAX-1)){
    g_faceproc_lib.load_dsp_lib = 1;
    IDBG_HIGH("face detection running in DSP mode");
  } else {
    g_faceproc_lib.load_dsp_lib = 0;
    IDBG_HIGH("face detection running in ARM mode");
  }
#else
    g_faceproc_lib.load_dsp_lib = 0;
    IDBG_HIGH("face detection running in ARM mode,"
      "DSP is disabled in make file");
#endif

  return faceproc_comp_eng_load(&g_faceproc_lib);
#endif
}

/**
 * Function: faceproc_comp_unload
 *
 * Description: This function is used to unload the faceproc library
 *
 * Input parameters:
 *   @handle: library handle
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_NOT_FOUND
 *
 * Notes: none
 **/
void faceproc_comp_unload(void* handle)
{
  IMG_UNUSED(handle);
#ifdef FACEPROC_USE_NEW_WRAPPER
  faceproc_sw_wrapper_unload(FALSE);
  g_engine_loaded = false;
#else
  faceproc_comp_eng_unload(&g_faceproc_lib);
#endif
}
