/***************************************************************************
Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
***************************************************************************/

#include "faceproc_dsp_comp.h"
#include "faceproc_common.h"
#ifdef FACEPROC_USE_NEW_WRAPPER
#include "faceproc_sw_wrapper.h"
#else
#include "facial_parts_wrapper.h"
#endif
#include <math.h>
#include <dlfcn.h>
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#include <cutils/trace.h>
#include "AEEStdErr.h"

#include "img_thread.h"
#include "img_dsp_dl_mgr.h"

#ifdef FD_WITH_DSP_SW_FALLBACK_TEST
static int init_return_count = 0;
#endif
/**
 * CONSTANTS and MACROS
 **/
/*#define FD_FPS*/
// max wait for scheduled job on DSP dedicated thread
#define MAX_FDDSP_API_WAIT 2000
#define MAX_FDDSP_TEST_API_WAIT 200
/* sometimes it might take longer time during load because so many others
 * threads also trying to allocate/initialize  */
#define MAX_FDDSP_LOAD_API_WAIT 5000

/*internal intermediate fddsp Q size */
#define MAX_INTER_Q_SIZE MAX_NUM_FD_FRAMES

#ifdef FD_FPS
static int start_fd_ms = 0;
static int end_fd_ms = 0;
static int total_elapsed_ms;
static struct timeval fd_time;
#endif

#ifdef FD_PROFILE
#define FACEPROC_START_MEASURE \
  struct timeval start_time, mid_time, end_time;\
  gettimeofday(&start_time, NULL); \
  mid_time = start_time \

#define FACEPROC_MIDDLE_TIME \
do { \
  gettimeofday(&end_time, NULL); \
  IDBG_INFO("%s]%d Middle mtime  %lu ms",  __func__, __LINE__, \
  ((end_time.tv_sec * 1000) + (end_time.tv_usec / 1000)) - \
  ((mid_time.tv_sec * 1000) + (mid_time.tv_usec / 1000))); \
  mid_time = end_time; \
} while (0)\

#define FACEPROC_END_MEASURE \
do { \
  gettimeofday(&end_time, NULL); \
  IDBG_HIGH("End of measure Total %lu ms", \
  ((end_time.tv_sec * 1000) + (end_time.tv_usec / 1000)) - \
  ((start_time.tv_sec * 1000) + (start_time.tv_usec / 1000))); \
} while (0) \

#else
#define FACEPROC_START_MEASURE \
  do{}while(0) \

#define FACEPROC_MIDDLE_TIME \
  do{}while(0) \

#define FACEPROC_END_MEASURE \
  do{}while(0) \

#endif

#define FD_MAX_DUMP_CNT 10

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

#undef FACEPROC_NORMAL
#define FACEPROC_NORMAL AEE_SUCCESS

faceproc_dsp_lib_t g_faceproc_dsp_lib;

static int faceproc_dsp_comp_abort(void *handle, void *p_data);
static int faceproc_dsp_comp_test_dsp_connection_common();

/**
 * Function: faceproc_dsp_error_to_img_error
 *
 * Description: Converts DSP error to Img error
 *
 * Input parameters:
 *   dsp_error - Error returned from DSP function
 *
 * Return values:
 *     IMG_xx error corresponds to DSP error
 *
 * Notes: none
 **/
int faceproc_dsp_error_to_img_error(int dsp_error)
{
  int img_error = IMG_SUCCESS;

  switch (dsp_error) {
    case AEE_SUCCESS :
      img_error = IMG_SUCCESS;
      break;

    case AEE_EFAILED :
      img_error = IMG_ERR_GENERAL;
      break;

    case AEE_ENOMEMORY :
      img_error = IMG_ERR_NO_MEMORY;
      break;

    case AEE_ECLASSNOTSUPPORT :
    case AEE_EVERSIONNOTSUPPORT :
    case AEE_ESCHEMENOTSUPPORTED :
      img_error = IMG_ERR_NOT_SUPPORTED;
      break;

    case AEE_EBADPARM :
      img_error = IMG_ERR_INVALID_INPUT;
      break;

    case AEE_ENOTALLOWED :
      img_error = IMG_ERR_INVALID_OPERATION;
      break;

    case AEE_ERESOURCENOTFOUND :
      img_error = IMG_ERR_NOT_FOUND;
      break;

    case AEE_EITEMBUSY :
      img_error = IMG_ERR_BUSY;
      break;

    case ECONNRESET :
      IDBG_ERROR("%s:%d] Connection reset error : %d ",
        __func__, __LINE__, dsp_error);
      img_error = IMG_ERR_CONNECTION_FAILED;
      break;

    default:
      img_error = IMG_ERR_GENERAL;
      break;
  }

  return img_error;
}


/**
 * Function: faceproc_dsp_comp_eng_handle_error
 *
 * Description: Handle dsp context error cases
 *
 * Input parameters:
 *   p_comp - The pointer to comp structure
 *   img_error - error
 *
 * Return values:
 *     None
 *
 * Notes: noneg
 **/
void faceproc_dsp_comp_eng_handle_error(faceproc_dsp_comp_t *p_comp,
  int32_t img_error)
{
  switch(img_error) {
    case IMG_ERR_CONNECTION_FAILED : {
      g_faceproc_dsp_lib.restore_needed_flag = TRUE;
      img_dsp_dl_mgr_set_reload_needed(TRUE);

      // If the error is Connection Lost, send an event to Module.
      if (p_comp != NULL) {
        IMG_SEND_EVENT(&(p_comp->b), QIMG_EVT_COMP_CONNECTION_FAILED);
      }
    }

    default :
      // Todo : Do we need to send ERROR event for all other errors.
      break;
  }
}


/**
 * Function: faceproc_dsp_comp_init
 *
 * Description: Initializes the Qualcomm Technologies, Inc. faceproc component
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
int faceproc_dsp_comp_init(void *handle, void* p_userdata, void *p_data)
{
  faceproc_dsp_comp_t *p_comp = (faceproc_dsp_comp_t *)handle;
  int status = IMG_SUCCESS;
  int32_t i = 0;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  status = p_comp->b.ops.init(&p_comp->b, p_userdata, p_data);
  if (IMG_ERROR(status))
    return status;

  p_comp->mode = FACE_DETECT;

  img_q_init(&p_comp->intermediate_in_use_Q, "intermediate_in_use_Q");
  img_q_init(&p_comp->intermediate_free_Q, "intermediate_free_Q");

  for (i = 0; i < MAX_INTER_Q_SIZE; i++) {
    faceproc_internal_queue_struct * p_node =
      (faceproc_internal_queue_struct *)malloc(
        sizeof(faceproc_internal_queue_struct));
    if (!p_node) {
      IDBG_ERROR("%s:%d] error initializing inter Q", __func__, __LINE__);
      return IMG_ERR_NO_MEMORY;
    }
    status = img_q_enqueue(&p_comp->intermediate_free_Q, (void*)p_node);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("%s:%d] can't add to free Q", __func__, __LINE__);
      free(p_node);
      return IMG_ERR_NO_MEMORY;
    }
  }

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_dsp_comp_common_destroy
 *
 * Description: schedules the destroy job  to the DSP thread
 *
 * Input parameters:
 *   p_comp - The pointer to the component handle.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int faceproc_dsp_comp_common_destroy(faceproc_dsp_comp_t * p_comp)
{
  int status = IMG_SUCCESS;
  IDBG_MED("%s:%d] Enter ", __func__, __LINE__);

  //Start:add img_thread API to schedule job.
  faceproc_dsp_comp_struct job_args;
  memset(&job_args, 0, sizeof(faceproc_dsp_comp_struct));
  job_args.p_comp = p_comp;
  uint32_t current_job_id = 0;
  img_thread_job_params_t fddspc_eng_destroy_job;
  fddspc_eng_destroy_job.args = (void*)&job_args;
  fddspc_eng_destroy_job.client_id = g_faceproc_dsp_lib.client_id;
  fddspc_eng_destroy_job.core_affinity = IMG_CORE_DSP;
  fddspc_eng_destroy_job.dep_job_ids = 0;
  fddspc_eng_destroy_job.dep_job_count = 0;
  fddspc_eng_destroy_job.delete_on_completion = TRUE;
  fddspc_eng_destroy_job.execute = faceproc_dsp_comp_eng_destroy_task_exec;
  current_job_id = img_thread_mgr_schedule_job(&fddspc_eng_destroy_job);
  //now wait for job to complete or let it run.
  if (0 < current_job_id) {
    status = img_thread_mgr_wait_for_completion_by_jobid(current_job_id,
      MAX_FDDSP_API_WAIT);
    status = job_args.return_value;
  }
  //End:add img_thread API to schedule job.

  IDBG_HIGH("%s:%d] Exit returned %d ", __func__, __LINE__ , status);

  return status;

}
/**
 * Function: faceproc_dsp_comp_deinit
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
int faceproc_dsp_comp_deinit(void *handle)
{
  faceproc_dsp_comp_t *p_comp = (faceproc_dsp_comp_t *)handle;
  int status = IMG_SUCCESS;
  int32_t i = 0;

  IDBG_HIGH("%s:%d] ", __func__, __LINE__);
  status = faceproc_dsp_comp_abort(handle, NULL);
  if (IMG_ERROR(status))
    return status;

  status = p_comp->b.ops.deinit(&p_comp->b);
  if (IMG_ERROR(status))
    return status;

  status = faceproc_dsp_comp_common_destroy(p_comp);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] failed status =%d ", __func__, __LINE__ , status);
    return status;
  }

  //free memory allocated for intermediate results
  faceproc_internal_queue_struct *p_node;
  for (i = 0; i < MAX_INTER_Q_SIZE; i++) {
    p_node = img_q_dequeue(&p_comp->intermediate_free_Q);
    if (NULL == p_node) {
      IDBG_ERROR("%s %d] Fail to dequeue free buffer", __func__, __LINE__);
      break;
    }
    free(p_node);
  }

  img_q_deinit(&p_comp->intermediate_in_use_Q);
  img_q_deinit(&p_comp->intermediate_free_Q);

  pthread_mutex_destroy(&p_comp->result_mutex);

  free(p_comp);
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_dsp_comp_flush_buffers
 *
 * Description: Flush buffers from component queue.
 *
 * Arguments:
 *   @p_comp: Pointer to Faceproc component instance.
 *
 * Return values:
 *   none.
 **/
void faceproc_dsp_comp_flush_buffers(faceproc_dsp_comp_t *p_comp)
{
  int i;
  int count;
  int status;
  img_frame_t *p_frame;

  count = img_q_count(&p_comp->b.inputQ);
  for (i = 0; i < count; i++) {
    p_frame = img_q_dequeue(&p_comp->b.inputQ);
    if (NULL == p_frame) {
      IDBG_ERROR("%s %d]Fail to dequeue input buffer", __func__, __LINE__);
      continue;
    }
    status = img_q_enqueue(&p_comp->b.outputQ, p_frame);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("%s %d]Fail to enqueue input buffer", __func__, __LINE__);
      continue;
    }
    IMG_SEND_EVENT(&p_comp->b, QIMG_EVT_BUF_DONE);
  }

  //free from intermediate Queue as well.
  faceproc_internal_queue_struct *p_node;
  count = img_q_count(&p_comp->intermediate_in_use_Q);
  for (i = 0; i < count; i++) {
    p_node = img_q_dequeue(&p_comp->intermediate_in_use_Q);
    if (NULL == p_node || NULL == p_node->p_frame) {
      IDBG_ERROR("Fail to dequeue input buffer");
      continue;
    }
    status = img_q_enqueue(&p_comp->b.outputQ, p_node->p_frame);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("Fail to enqueue input buffer");
      continue;
    }
    IMG_SEND_EVENT(&p_comp->b, QIMG_EVT_BUF_DONE);
  }
}

/**
 * Function: faceproc_dsp_comp_set_param
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
int faceproc_dsp_comp_set_param(void *handle, img_param_type param,
  void *p_data)
{
  faceproc_dsp_comp_t *p_comp = (faceproc_dsp_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;

  status = p_comp->b.ops.set_parm(&p_comp->b, param, p_data);
  if (IMG_ERROR(status))
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
    IDBG_HIGH("Set chromatix in state %d", p_base->state);
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
    IDBG_HIGH("Set dump mode %d", p_comp->dump_mode);
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
    faceproc_sw_wrapper_set_debug(p_comp->p_sw_wrapper, p_debug_settings);
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
    IDBG_ERROR("Error, param=%d", param);
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_dsp_comp_get_param
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
int faceproc_dsp_comp_get_param(void *handle, img_param_type param,
  void *p_data)
{
  faceproc_dsp_comp_t *p_comp = (faceproc_dsp_comp_t *)handle;
  int status = IMG_SUCCESS;

  status = p_comp->b.ops.get_parm(&p_comp->b, param, p_data);
  if (IMG_ERROR(status))
    return status;

  switch (param) {
  case QWD_FACEPROC_RESULT: {
    faceproc_result_t *p_result = (faceproc_result_t *)p_data;

    if (NULL == p_result) {
      IDBG_ERROR("%s:%d] invalid faceproc result", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    if (!p_comp->width || !p_comp->height) {
      IDBG_ERROR("%s:%d] Invalid param w=%d, h=%d",
        __func__, __LINE__, p_comp->width, p_comp->height);
      return IMG_ERR_INVALID_INPUT;
    }
    pthread_mutex_lock(&p_comp->result_mutex);
    *p_result = p_comp->inter_result;
    pthread_mutex_unlock(&p_comp->result_mutex);

    p_result->client_id = p_comp->client_id;
    break;
  }
  case QWD_FACEPROC_USE_INT_BUFF: {
    uint32_t *p_use_int_buff = (uint32_t *)p_data;

    if (NULL == p_use_int_buff) {
      IDBG_ERROR("%s:%d] invalid input", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    *p_use_int_buff = FALSE;
    break;
  }
  case QWD_FACEPROC_BUFF_TYPE: {
    img_buf_type_t *p_type = (img_buf_type_t *)p_data;

    if (NULL == p_type) {
      IDBG_ERROR("%s:%d] invalid input", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    *p_type = IMG_BUFFER_HEAP;

   char value[PROPERTY_VALUE_MAX];
   property_get("persist.vendor.camera.imglib.fddsp", value, "1");
   if (atoi(value)){
#ifdef USE_SMMU_BUFFERS_FOR_FDDSP
     IDBG_MED("%s:%d] FD in DSP mode, use ION_IOMMU",
       __func__, __LINE__);
     *p_type = IMG_BUFFER_ION_IOMMU;
#else
     IDBG_MED("%s:%d] FD in DSP mode, use ION_ADSP",
       __func__, __LINE__);
     *p_type = IMG_BUFFER_ION_ADSP;
#endif
   } else {
     IDBG_MED("%s:%d] FD NOT in DSP mode, use BUFFER_HEAP",__func__, __LINE__);
   }

    break;
  }
  default:
    IDBG_ERROR("%s:%d] Error, param=%d", __func__, __LINE__, param);
    return IMG_ERR_INVALID_INPUT;
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_dsp_comp_cfg_debug
 *
 * Description: Debug params for faceproc configuration
 *
 * Input parameters:
 * @p_config: pointer to facepeoc config
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static void faceproc_dsp_comp_cfg_debug(faceproc_config_t *p_config)
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
  IDBG_HIGH("%s:%d] FaceProc cfg max_face_size %d",
    __func__, __LINE__,
    p_config->face_cfg.max_face_size);
  IDBG_HIGH("%s:%d] FaceProc cfg max_num_face_to_detect %d",
    __func__, __LINE__,
    p_config->face_cfg.max_num_face_to_detect);
  IDBG_HIGH("%s:%d] FaceProc cfg min_face_size %d",
    __func__, __LINE__,
    p_config->face_cfg.min_face_size);
  IDBG_MED("%s:%d] FaceProc cfg rotation_range %d",
    __func__, __LINE__,
    p_config->face_cfg.rotation_range);
}

/**
 * Function: face_proc_dsp_can_wait
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
int face_proc_dsp_can_wait(void *p_userdata)
{
  faceproc_dsp_comp_t *p_comp = (faceproc_dsp_comp_t *)p_userdata;
  img_component_t *p_base = &p_comp->b;
  return !((p_base->state == IMG_STATE_STOP_REQUESTED)
    || (p_base->state == IMG_STATE_STOPPED));
}

/**
 * Function: face_proc_dsp_can_wait
 *
 * Description: Queue function to check if abort is issued
 *
 * Input parameters:
 *   p_userdata - The pointer to faceproc component
 *   data - pointer to frame data
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_OPERATION
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
static int face_proc_release_frame(void *data, void *p_userdata)
{
  faceproc_dsp_comp_t *p_comp = (faceproc_dsp_comp_t *)p_userdata;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_frame_t *p_frame = (img_frame_t *)data;

  status = img_q_enqueue(&p_base->outputQ, p_frame);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
  } else {
    IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
  }
  p_comp->facedrop_cnt++;
  return status;
}

/**
 * Function: faceproc_dsp_comp_start
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
int faceproc_dsp_comp_start(void *handle, void *p_data)
{
  faceproc_dsp_comp_t *p_comp = (faceproc_dsp_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;

  p_comp->is_chromatix_changed = FALSE;
  p_comp->need_config_update = FALSE;
  pthread_mutex_lock(&p_base->mutex);
  if (p_base->state != IMG_STATE_INIT) {
    IDBG_ERROR("%s:%d] Error state %d", __func__, __LINE__,
      p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_NOT_SUPPORTED;
  }
  if (NULL == p_base->thread_loop) {
    IDBG_MED("%s:%d] comp loop warning %d", __func__, __LINE__,
      p_base->state);
  }

  if (!p_comp->config_set) {
    IDBG_ERROR("%s:%d] error config not set", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return status;
  }
  faceproc_dsp_comp_cfg_debug(&p_comp->config);

  IDBG_LOW("%s:%d] Enter ", __func__, __LINE__);
  //Start:add img_thread API to schedule job.
  faceproc_dsp_comp_struct struct_faceproc_comp ;
  memset(&struct_faceproc_comp, 0, sizeof(faceproc_dsp_comp_struct));
  struct_faceproc_comp.p_comp = p_comp;
  uint32_t current_job_id = 0;
  img_thread_job_params_t cfddspc_eng_config_job;
  cfddspc_eng_config_job.args = &struct_faceproc_comp;
  cfddspc_eng_config_job.client_id =
    g_faceproc_dsp_lib.client_id;
  cfddspc_eng_config_job.core_affinity = IMG_CORE_DSP;
  cfddspc_eng_config_job.dep_job_ids = 0;
  cfddspc_eng_config_job.dep_job_count = 0;
  cfddspc_eng_config_job.delete_on_completion = TRUE;
  cfddspc_eng_config_job.execute =
    faceproc_dsp_comp_eng_config_task_exec;
  current_job_id = img_thread_mgr_schedule_job(
     &cfddspc_eng_config_job);
  IDBG_LOW("%s:%d] Current Job Id =%d ", __func__, __LINE__ , current_job_id);

  if (0 < current_job_id) {
    status = img_thread_mgr_wait_for_completion_by_jobid(current_job_id,
      MAX_FDDSP_API_WAIT );
    status = struct_faceproc_comp.return_value;
  }
  //End:add img_thread API to schedule job.
    IDBG_LOW("%s:%d] Exit ", __func__, __LINE__);

  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] failed, status=%d", __func__, __LINE__, status);
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

  return 0;
}

/**
 * Function: faceproc_dsp_comp_abort
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
int faceproc_dsp_comp_abort(void *handle, void *p_data)
{
  IMG_UNUSED(p_data);

  faceproc_dsp_comp_t *p_comp = (faceproc_dsp_comp_t *)handle;
  img_component_t *p_base = (img_component_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_HIGH("%s:%d] state %d", __func__, __LINE__, p_base->state);
  pthread_mutex_lock(&p_base->mutex);
  if (IMG_STATE_STARTED != p_base->state) {
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_SUCCESS;
  }
  p_base->state = IMG_STATE_STOP_REQUESTED;
  pthread_mutex_unlock(&p_base->mutex);
  /*signal the thread*/
  img_q_signal(&p_base->inputQ);

  IDBG_MED("waiting for all jobs to finish for client id %d",
    p_comp->p_lib->client_id);
  status = img_thread_mgr_wait_for_completion_by_clientid(
    p_comp->p_lib->client_id, MAX_FDDSP_LOAD_API_WAIT );
  if (status == IMG_ERR_TIMEOUT) {
    IDBG_ERROR("%s:%d] wait for threadmgr TIMED OUT ", __func__, __LINE__);
  } else if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] wait for threadmgr FAILED status=%d",
      __func__, __LINE__, status);
  }
  IDBG_MED("%s:%d] done wait for jobs  ", __func__, __LINE__);

  /* flush rest of the buffers */
  faceproc_dsp_comp_flush_buffers(p_comp);

  /* destroy the handle */
  status = faceproc_dsp_comp_common_destroy(p_comp);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] failed, status=%d", __func__, __LINE__, status);
    return status;
  }

  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_INIT;
  pthread_mutex_unlock(&p_base->mutex);
  IDBG_HIGH("%s:%d] X", __func__, __LINE__);
  return status;
}

/**
 * Function: faceproc_dsp_comp_process
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
int faceproc_dsp_comp_process (void *handle, img_cmd_type cmd, void *p_data)
{
  faceproc_dsp_comp_t *p_comp = (faceproc_dsp_comp_t *)handle;
  int status;

  status = p_comp->b.ops.process(&p_comp->b, cmd, p_data);
  if (IMG_ERROR(status))
    return status;

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_dsp_comp_queue_buffer
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
int faceproc_dsp_comp_queue_buffer(void *handle, img_frame_t *p_frame,
  img_type_t type)
{
  if (!handle || !p_frame || type != IMG_IN) {
    IDBG_ERROR("invalid input : handle=%p, p_frame=%p, type=%d",
      handle, p_frame, type);
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_dsp_comp_t *p_comp = (faceproc_dsp_comp_t *)handle;
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

  /* drop the frame if DSP is currently processing the frame */
  if ((count > p_comp->fd_chromatix.input_pending_buf) || p_comp->processing) {
    IDBG_MED("%s:%d] Drop the frame %d processing %d",
      __func__, __LINE__, count, p_comp->processing);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_BUSY;
  }

  status = img_q_enqueue(queue, p_frame);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] Error enqueue, status=%d", __func__, __LINE__, status);
    pthread_mutex_unlock(&p_base->mutex);
    return status;
  }
  IDBG_MED("%s:%d] q_count %d", __func__, __LINE__, img_q_count(queue));
  IDBG_MED("%s:%d] p_frame before q_remove p_comp %d p_base %d "
    "  &p_base->inputQ %d",__func__, __LINE__ ,
    (int) p_comp, (int) p_base, (int) queue);

  //// start FROM THREAD_LOOP
  //Start:add img_thread API to schedule job.
  IDBG_MED("%s:%d] Enter scheduled job creation ", __func__, __LINE__);
  faceproc_dsp_comp_exec_struct *p_job_args = (faceproc_dsp_comp_exec_struct*)
    calloc(1, sizeof(faceproc_dsp_comp_exec_struct));
  if (NULL == p_job_args ) {
    IDBG_ERROR("%s:%d] No memory, continue", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_NO_MEMORY;
  }
  p_job_args->p_comp = p_comp;
  uint32_t current_job_id = 0;
  img_thread_job_params_t fddspc_eng_exec_job;
  fddspc_eng_exec_job.args = p_job_args;
  fddspc_eng_exec_job.client_id = g_faceproc_dsp_lib.client_id;
  fddspc_eng_exec_job.core_affinity = IMG_CORE_DSP;
  fddspc_eng_exec_job.dep_job_ids = 0;
  fddspc_eng_exec_job.dep_job_count = 0;
  fddspc_eng_exec_job.delete_on_completion = TRUE;
  fddspc_eng_exec_job.execute = faceproc_dsp_comp_eng_exec_task_exec;
  current_job_id = img_thread_mgr_schedule_job(&fddspc_eng_exec_job);
  //now wait for job to complete or let it run.
  if (0 < current_job_id) {
    IDBG_MED("%s:%d] not Waiting for scheduled faceproc_dsp"
      "_comp_eng_exec_job completion ", __func__, __LINE__);
    status = p_job_args->return_value;
  }
  //End:add img_thread API to schedule job.
  IDBG_MED("%s:%d] Exit ", __func__, __LINE__);
  //// end FROM THREAD_LOOP

  pthread_mutex_unlock(&p_base->mutex);
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_dsp_comp_create
 *
 * Description: This function is used to create Qualcomm Technologies, Inc.
 * faceproc component
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
int faceproc_dsp_comp_create(void* handle, img_component_ops_t *p_ops)
{
  IMG_UNUSED(handle);

  faceproc_dsp_comp_t *p_comp = NULL;
  int status;

  if (g_faceproc_dsp_lib.restore_needed_flag == TRUE) {
    IDBG_MED("restore needed");
    img_dsp_dl_requestall_to_close_and_reopen();
  }

#ifdef FDDSP_USE_SW_WRAPPER_FOR_ALL
    if (!g_faceproc_dsp_lib.lib_loaded) {
#else
  if (NULL == g_faceproc_dsp_lib.ptr_stub) {
#endif
    IDBG_WARN("fddsp stub library not loaded");
    return IMG_ERR_INVALID_OPERATION;
  }

  if(IMG_SUCCESS != faceproc_dsp_comp_test_dsp_connection_common()){
    IDBG_ERROR("test DSP connection failed ");
    img_dsp_dl_mgr_set_reload_needed(TRUE);
    g_faceproc_dsp_lib.restore_needed_flag = TRUE;
    return IMG_ERR_GENERAL;
  }
  g_faceproc_dsp_lib.restore_needed_flag = FALSE;

  p_comp = (faceproc_dsp_comp_t *)malloc(sizeof(faceproc_dsp_comp_t));
  if (NULL == p_comp) {
    IDBG_ERROR("malloc failed");
    return IMG_ERR_NO_MEMORY;
  }

  if (NULL == p_ops) {
    IDBG_ERROR("p_ops NULL");
    status = IMG_ERR_INVALID_INPUT;
    goto error;
  }

  memset(p_comp, 0x0, sizeof(faceproc_dsp_comp_t));
  status = img_comp_create(&p_comp->b);
  if (IMG_ERROR(status)) {
    goto error;
  }

  p_comp->clip_face_data = 1;
  p_comp->device_rotation = -1;
  p_comp->dynamic_cfg_params.lux_index = -1;
  p_comp->p_lib = &g_faceproc_dsp_lib;
  //NOTE : FD-DSP comp does not need thread loop
  p_comp->b.thread_loop = NULL;

  p_comp->b.p_core = p_comp;

  /* copy the ops table from the base component */
  *p_ops = p_comp->b.ops;
  p_ops->init            = faceproc_dsp_comp_init;
  p_ops->deinit          = faceproc_dsp_comp_deinit;
  p_ops->set_parm        = faceproc_dsp_comp_set_param;
  p_ops->get_parm        = faceproc_dsp_comp_get_param;
  p_ops->start           = faceproc_dsp_comp_start;
  p_ops->abort           = faceproc_dsp_comp_abort;
  p_ops->process         = faceproc_dsp_comp_process;
  p_ops->queue_buffer    = faceproc_dsp_comp_queue_buffer;

  p_ops->handle = (void *)p_comp;

  pthread_mutex_init(&p_comp->result_mutex, NULL);

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
 * Function: faceproc_dsp_comp_load
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
int faceproc_dsp_comp_load(const char* name, void** handle)
{
  IDBG_HIGH("%s:%d] E \n", __func__, __LINE__);
  IMG_UNUSED(name);
  IMG_UNUSED(handle);
  int ret = IMG_SUCCESS;

#ifdef FDDSP_USE_SW_WRAPPER_FOR_ALL
  if (g_faceproc_dsp_lib.lib_loaded) {
#else
  if (g_faceproc_dsp_lib.ptr_stub) {
#endif
    IDBG_ERROR("%s:%d] library already loaded", __func__, __LINE__);
    return IMG_ERR_NOT_FOUND;
  }

#ifdef FD_WITH_DSP_NEW
  uint32_t client_id = 0;
  char value[PROPERTY_VALUE_MAX];
  #define MODULE_FDDSP_CONTROL_FLAG     ("persist.vendor.camera.imglib.fddsp")
  #define MODULE_FDDSP_CONTROL_ENABLED  ("1")
  #define MODULE_FDDSP_CONTROL_DISABLED ("0")
  #define MODULE_FDDSP_CONTROL_DEFAULT  (MODULE_FDDSP_CONTROL_DISABLED)

  property_get(MODULE_FDDSP_CONTROL_FLAG, value, MODULE_FDDSP_CONTROL_DEFAULT);
  if (!strncmp(MODULE_FDDSP_CONTROL_ENABLED,
    value, PROPERTY_VALUE_MAX - 1)) {
    g_faceproc_dsp_lib.load_dsp_lib = 1;

    IDBG_HIGH("%s:%d] Enter ", __func__, __LINE__);
    img_core_type_t fddsp_img_core_type[2] = { IMG_CORE_DSP,
      IMG_CORE_ARM
    };

    if (IMG_SUCCESS != img_thread_mgr_create_pool()) {
      IDBG_ERROR("%s:%d] FDDSP img_thread_mgr_create_pool failed ",
                 __func__, __LINE__);
      return IMG_ERR_NOT_FOUND;
    }
    client_id = img_thread_mgr_reserve_threads(2,
      (img_core_type_t *)(&fddsp_img_core_type));
    if (client_id > 0x7FFFFFFF) {
      IDBG_ERROR("%s:%d] FDDSP reserve threads failed client_id %d \n",
        __func__, __LINE__,  client_id);
    }
    if (client_id == 0) {
      IDBG_ERROR("%s:%d] FDDSP reserve threads failed client_id %d \n",
                 __func__, __LINE__, client_id);
      return IMG_ERR_GENERAL;
    }
    g_faceproc_dsp_lib.client_id = client_id;
    IDBG_HIGH("%s:%d] FDDSP reserve threads client id %d \n",
      __func__, __LINE__, g_faceproc_dsp_lib.client_id);

    //Start:add img_thread API to schedule job.
    faceproc_dsp_comp_eng_load_struct job_args;
    job_args.p_lib = &g_faceproc_dsp_lib;
    uint32_t current_job_id = 0;
    img_thread_job_params_t fddspc_eng_load_job;
    fddspc_eng_load_job.args = &job_args;
    fddspc_eng_load_job.client_id = g_faceproc_dsp_lib.client_id;
    fddspc_eng_load_job.core_affinity = IMG_CORE_DSP;
    fddspc_eng_load_job.dep_job_ids = 0;
    fddspc_eng_load_job.dep_job_count = 0;
    fddspc_eng_load_job.delete_on_completion = TRUE;
    fddspc_eng_load_job.execute = faceproc_dsp_comp_eng_load_task_exec;
    current_job_id = img_thread_mgr_schedule_job(&fddspc_eng_load_job);
    //now wait for job to complete or let it run.
    if (0 < current_job_id) {
      ret = img_thread_mgr_wait_for_completion_by_jobid(current_job_id,
        MAX_FDDSP_LOAD_API_WAIT);
      if (IMG_ERR_TIMEOUT == ret) {
        IDBG_ERROR("%s:%d] TIME OUT - something went wrong during loading",
          __func__, __LINE__);
        ret = img_thread_mgr_unreserve_threads(g_faceproc_dsp_lib.client_id);
        if(IMG_ERROR(ret)){
          IDBG_MED("%s:%d] error unreserving threads",__func__, __LINE__);
        }
        g_faceproc_dsp_lib.client_id = 0;
        img_thread_mgr_destroy_pool();
        ret = IMG_ERR_GENERAL;
      } else {
        ret = job_args.return_value;
      }
    }
    //End:add img_thread API to schedule job.
    IDBG_HIGH("%s:%d] Exit ", __func__, __LINE__);
    if (IMG_SUCCEEDED(ret)) {
      IDBG_HIGH("face detection running in DSP mode");
    } else {
      IDBG_ERROR("face detection DSP Mode Failed");
      ret = IMG_ERR_GENERAL;
    }
  } else {
    g_faceproc_dsp_lib.load_dsp_lib = 0;
    IDBG_WARN("face detection dsp disabled , enable by set property");
    ret = IMG_ERR_GENERAL;
  }
#else
  g_faceproc_dsp_lib.load_dsp_lib = 0;
  IDBG_HIGH("face detection running in ARM mode,"
    "DSP is disabled in make file");
  ret = IMG_ERR_GENERAL;
#endif

  return ret;
}

/**
 * Function: faceproc_dsp_comp_unload
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
void faceproc_dsp_comp_unload(void* handle)
{
  int32 ret = IMG_ERR_GENERAL;
  IDBG_HIGH("%s:%d] Enter ", __func__, __LINE__);
  IMG_UNUSED(handle);

  //Start:add img_thread API to schedule job.
  faceproc_dsp_comp_eng_load_struct job_args ;
  memset(&job_args, 0, sizeof(faceproc_dsp_comp_eng_load_struct));
  job_args.p_lib = &g_faceproc_dsp_lib;
  uint32_t current_job_id = 0;
  img_thread_job_params_t fddspc_eng_unload_job;
  fddspc_eng_unload_job.args = &job_args;
  fddspc_eng_unload_job.client_id = g_faceproc_dsp_lib.client_id;
  fddspc_eng_unload_job.core_affinity = IMG_CORE_DSP;
  fddspc_eng_unload_job.dep_job_ids = 0;
  fddspc_eng_unload_job.dep_job_count = 0;
  fddspc_eng_unload_job.delete_on_completion = TRUE;
  fddspc_eng_unload_job.execute =
    faceproc_dsp_comp_eng_unload_task_exec;
  current_job_id = img_thread_mgr_schedule_job(
    &fddspc_eng_unload_job);
  if (0 < current_job_id) {
    ret = img_thread_mgr_wait_for_completion_by_jobid(current_job_id,
      MAX_FDDSP_API_WAIT );
    ret = job_args.return_value;
  }
  //End:add img_thread API to schedule job.
  IDBG_MED("%s:%d] Exit ret %d ", __func__, __LINE__ ,(int) ret);

  ret = img_thread_mgr_unreserve_threads(g_faceproc_dsp_lib.client_id);
  if(IMG_ERROR(ret)){
    IDBG_MED("%s:%d] error unreserving threads",__func__, __LINE__);
  }
  g_faceproc_dsp_lib.client_id = 0;
  img_thread_mgr_destroy_pool();
  IDBG_HIGH("%s:%d] Exit after Thread Destroy ", __func__, __LINE__);

}

/**
 * Function: get_faceproc_dsp_lib
 *
 * Description: returns pointer to golbal faceproc dsp lib
 * struct
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *     faceproc_dsp_lib_t* pointer to lib
 *
 * Notes: none
 **/
static faceproc_dsp_lib_t * get_faceproc_dsp_lib()
{
  return &g_faceproc_dsp_lib;
}

/**
 * Function: faceproc_dsp_comp_test_dsp_connection_common
 *
 * Description: schedules the destroy job  to the DSP thread
 *
 * Input parameters:
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int faceproc_dsp_comp_test_dsp_connection_common() {
  IDBG_HIGH("%s:%d] E \n", __func__, __LINE__);
  int ret = IMG_SUCCESS;

  //Start:add img_thread API to schedule job.
  faceproc_dsp_comp_eng_load_struct job_args ;
  memset(&job_args , 0, sizeof(faceproc_dsp_comp_eng_load_struct));
  job_args.p_lib = &g_faceproc_dsp_lib;
  uint32_t current_job_id = 0;
  img_thread_job_params_t fddspc_eng_test_job;
  fddspc_eng_test_job.args = &job_args;
  fddspc_eng_test_job.client_id = g_faceproc_dsp_lib.client_id;
  fddspc_eng_test_job.core_affinity = IMG_CORE_DSP;
  fddspc_eng_test_job.dep_job_ids = 0;
  fddspc_eng_test_job.dep_job_count = 0;
  fddspc_eng_test_job.delete_on_completion = TRUE;
  fddspc_eng_test_job.execute =
    faceproc_dsp_comp_eng_test_dsp_connection_task_exec;
  current_job_id = img_thread_mgr_schedule_job(
    &fddspc_eng_test_job);
  //now wait for job to complete or let it run.
  if (0 < current_job_id) {
    ret = img_thread_mgr_wait_for_completion_by_jobid(current_job_id,
      MAX_FDDSP_TEST_API_WAIT);
    if (IMG_ERR_TIMEOUT == ret) {
      IDBG_ERROR("%s:%d] TIME OUT - something went wrong during fddsptest API",
        __func__, __LINE__);
    } else {
      ret = job_args.return_value;
      if (ret != IMG_SUCCESS) {
        IDBG_ERROR("%s:%d] pstruct->return_value %d", __func__, __LINE__ ,
          job_args.return_value);
      }
    }
  }

  IDBG_HIGH("%s:%d] Exit returned %d ", __func__, __LINE__, ret);

  return ret;

}

extern void faceproc_dsp_comp_flush_buffers(faceproc_dsp_comp_t *p_comp);
extern faceproc_dsp_lib_t * get_faceproc_dsp_lib();

/**
 * Function: faceproc_dsp_comp_eng_exec
 *
 * Description: main algorithm execution function for face processing
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   p_frame - Input frame
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_dsp_comp_eng_exec(faceproc_dsp_comp_t *p_comp,
  img_frame_t *p_frame)
{
  INT32 num_faces;
  img_component_t *p_base;
  faceproc_result_t fd_result;
  int status = IMG_SUCCESS;

  if (NULL == p_comp) {
    IDBG_ERROR("%s:%d] null p_comp ", __func__, __LINE__ );
    return IMG_SUCCESS;
  }

  p_base = &p_comp->b;

  if (!face_proc_dsp_can_wait(p_comp)) {
    IDBG_HIGH("%s:%d] Exit the thread", __func__, __LINE__);
    return IMG_SUCCESS;
  }

  IDBG_MED("%s:%d] p_frame before q_remove p_comp %d p_base %d &p_base->inputQ"
    " %d",__func__, __LINE__ , (int)p_comp, (int)p_base, (int)&p_base->inputQ);
  p_frame = img_q_dequeue(&p_base->inputQ);
  IDBG_MED("%s:%d] p_frame AFTER q_remove %d ", __func__, __LINE__ ,
    (int) p_frame);

  if (NULL == p_frame) {
    IDBG_ERROR("%s:%d] NO Frame in in input Queue ", __func__, __LINE__ );
    return IMG_ERR_INVALID_INPUT;
  }
  if ((p_comp->width != IMG_FD_WIDTH(p_frame)) ||
    (p_comp->height != IMG_HEIGHT(p_frame))) {
    IDBG_HIGH("%s:%d] [FD_HAL3] Update dimensions to %dx%d", __func__,
      __LINE__, IMG_FD_WIDTH(p_frame), IMG_HEIGHT(p_frame));
    p_comp->width = IMG_FD_WIDTH(p_frame);
    p_comp->height = IMG_HEIGHT(p_frame);
    p_comp->need_config_update = TRUE;
  }
  IDBG_MED("%s:%d] Enter ", __func__, __LINE__);
  //////   END FROM THREAD_LOOP

  switch (p_comp->mode) {
    case FACE_DETECT_BSGC:
    case FACE_DETECT:
    case FACE_DETECT_LITE:
      if (p_comp->is_chromatix_changed == TRUE) {
        p_comp->is_chromatix_changed = FALSE;

        status = faceproc_dsp_comp_eng_reconfig_core(p_comp, TRUE);
      }

      if (p_comp->need_config_update == TRUE) {
        faceproc_dsp_comp_eng_update_cfg(p_comp);
      }

      status = faceproc_fd_execute(p_comp, p_frame, &num_faces);
      break;
    default :
      IDBG_ERROR("%s MODE not selected/recognized", __func__);
  }

  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] error %d", __func__, __LINE__, status);
    goto exec_error;
  }

  IDBG_MED("%s:%d] state %d abort %d", __func__, __LINE__,
    p_base->state, p_comp->abort_flag);
  if (IMG_CHK_ABORT_LOCKED(p_base, &p_base->mutex)) {
    IDBG_ERROR("%s:%d] Abort requested %d", __func__, __LINE__, status);
    status = face_proc_release_frame(p_frame, p_comp);
    return IMG_SUCCESS;
  }

  memset(&fd_result,0,sizeof(fd_result));
  status = faceproc_dsp_comp_eng_get_output(p_comp, p_frame, &fd_result);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] invalid faceproc result", __func__, __LINE__);
    goto exec_error;
  }

  faceproc_internal_queue_struct *p_node;
  p_node = img_q_dequeue(&p_comp->intermediate_free_Q);
  if (!p_node) {
    IDBG_ERROR("%s:%d] dequeue error %d", __func__, __LINE__, status);
    goto exec_error;
  }

  p_node->inter_result = fd_result;
  p_node->p_frame = p_frame;
  status = img_q_enqueue(&p_comp->intermediate_in_use_Q, p_node);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
    goto exec_error;
  }

  //S:add img_thread API to schedule job.
  faceproc_dsp_comp_eng_get_output_struct *p_job_args =
    (faceproc_dsp_comp_eng_get_output_struct*)calloc(1,
       sizeof(faceproc_dsp_comp_eng_get_output_struct));
  if (NULL == p_job_args ) {
    IDBG_ERROR("%s:%d] No memory", __func__, __LINE__);
    status = IMG_ERR_NO_MEMORY;
    goto exec_error;
  }

  p_job_args->p_comp = p_comp;
  uint32_t current_job_id = 0;
  img_thread_job_params_t fddspc_eng_get_output_job;
  fddspc_eng_get_output_job.args = p_job_args;
  fddspc_eng_get_output_job.client_id =
    get_faceproc_dsp_lib()->client_id;
  fddspc_eng_get_output_job.core_affinity = IMG_CORE_ARM;
  fddspc_eng_get_output_job.dep_job_ids = 0;
  fddspc_eng_get_output_job.dep_job_count = 0;
  fddspc_eng_get_output_job.delete_on_completion = TRUE;
  fddspc_eng_get_output_job.execute =
    faceproc_dsp_comp_eng_get_output_task_exec;
  current_job_id = img_thread_mgr_schedule_job(
     &fddspc_eng_get_output_job);
  if (0 < current_job_id) {
    IDBG_MED("%s:%d] not waiting for scheduled faceproc_dsp_comp"
      "_eng_get_output_task_exec completion ", __func__, __LINE__);
    status = p_job_args->return_value;
  }
  //End:add img_thread API to schedule job.
  IDBG_MED("%s:%d] Exit QWD_FACEPROC_RESULT", __func__, __LINE__);

  return IMG_SUCCESS;

exec_error:
  face_proc_release_frame(p_frame, p_comp);
  return status;
}

/**
 * Function: faceproc_dsp_comp_eng_get_output
 *
 * Description: Get the output from the frameproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   fd_data - Input frame
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_dsp_comp_eng_get_output(faceproc_dsp_comp_t *p_comp,
  img_frame_t *p_frame, faceproc_result_t *fd_data)
{
  INT32 num_faces;
  int status = IMG_SUCCESS;
  IDBG_LOW("%s %d, p_comp:%p, p_res:%p",  __func__, __LINE__, p_comp, fd_data);
  switch (p_comp->mode) {
  case FACE_DETECT_BSGC:
  case FACE_DETECT:
  case FACE_DETECT_LITE:
    status = faceproc_fd_output(p_comp, p_frame, fd_data, &num_faces);
    break;

  default:
    IDBG_ERROR("%s %d: Unsupported mode selected", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  IDBG_LOW("%s, After rc: %d, p_comp:%p, p_res:%p",  __func__,
    status, p_comp, fd_data);
  return status;
}

/**
 * Function: faceproc_dsp_comp_eng_destroy
 *
 * Description: Destroy the faceproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_dsp_comp_eng_destroy(faceproc_dsp_comp_t *p_comp)
{
  int rc;

  if (!p_comp)
    return IMG_ERR_GENERAL;

  IDBG_MED("%s:%d] faceproc engine clean", __func__, __LINE__);
  rc = faceproc_dsp_comp_eng_reset(p_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s: faceproc_dsp_comp_eng_reset failed %d", __func__, rc);
  }
  return IMG_SUCCESS;
}

//// Below are the Job Execute functions.    //////

/**
 * Function: faceproc_dsp_comp_eng_config_task_exec
 *
 * Description: executes the config function as a job in
 * loadbalancer/threadmgr. eng_config has actual implementation.
 *
 * Input parameters:
 *   param - The pointer to function/usecase specific params
 *   structure
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_dsp_comp_eng_config_task_exec(void *param)
{
  if (NULL == param) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  faceproc_dsp_comp_struct * pstruct = (faceproc_dsp_comp_struct *)param;
  IDBG_LOW("FD_DSP_JOB : Start");
  pstruct->return_value = faceproc_dsp_comp_eng_config(pstruct->p_comp);
  IDBG_LOW("FD_DSP_JOB : End");
  return IMG_SUCCESS;

}

/**
 * Function: faceproc_dsp_comp_eng_exec_task_exec
 *
 * Description: executes the eng_execute function as a job in
 * loadbalancer/threadmgr. eng_execute has actual
 * implementation.
 *
 * Input parameters:
 *   param - The pointer to function/usecase specific params
 *   structure
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_dsp_comp_eng_exec_task_exec(void *param)
{
  if (NULL == param) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  faceproc_dsp_comp_exec_struct * pstruct =
    (faceproc_dsp_comp_exec_struct *)param;
  IDBG_LOW("FD_DSP_JOB : Start");

  pstruct->p_comp->processing = TRUE;

  pstruct->return_value = faceproc_dsp_comp_eng_exec(
    pstruct->p_comp , pstruct->p_frame);
  pstruct->p_comp->processing = FALSE;

  // If the error is Connection Lost, send an event to Module.
  if (pstruct->return_value == IMG_ERR_CONNECTION_FAILED) {
    g_faceproc_dsp_lib.restore_needed_flag = TRUE;
    img_dsp_dl_mgr_set_reload_needed(TRUE);
    IMG_SEND_EVENT(&(pstruct->p_comp->b), QIMG_EVT_COMP_CONNECTION_FAILED);
  }

  free(pstruct);
  IDBG_LOW("FD_DSP_JOB : end");

  return IMG_SUCCESS;

}

/**
 * Function: faceproc_dsp_comp_get_result_and_send
 *
 * Description: executes the facial parts detection,
 * sending fd_result to faceproc module
 * sending of BUF_DONE event upstream
 * as a job in loadbalancer/threadmgr on ARM thread so that DSP
 * thread is not blocked.
 *
 * Input parameters:
 *   p_comp - The pointer to fddsp component structure
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_dsp_comp_get_result_and_send(faceproc_dsp_comp_t *p_comp)
{
  int32_t status = IMG_SUCCESS;
  faceproc_internal_queue_struct *p_node;
  img_frame_t *p_frame;
  faceproc_result_t *p_fd_result;
  img_component_t *p_base;

  if (NULL == p_comp) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  p_base = &(p_comp->b);

  p_node = img_q_dequeue(&p_comp->intermediate_in_use_Q);
  if (!p_node) {
    IDBG_ERROR("%s:%d] dequeue error %d", __func__, __LINE__, status);
    return IMG_ERR_GENERAL;
  }

  p_fd_result = &(p_node->inter_result);
  p_frame = p_node->p_frame;

  pthread_mutex_lock(&p_comp->result_mutex);
  p_comp->inter_result = *p_fd_result;
  pthread_mutex_unlock(&p_comp->result_mutex);

  memset(p_node, 0x0, sizeof(*p_node));
  status = img_q_enqueue(&p_comp->intermediate_free_Q, p_node);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
    face_proc_release_frame(p_frame, p_comp);
    return IMG_ERR_GENERAL;
  }

  status = img_q_enqueue(&p_base->outputQ, p_frame);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
    IMG_SEND_EVENT(p_base, QIMG_EVT_ERROR);
    return IMG_ERR_GENERAL;
  }

  if ((IMG_SUCCEEDED(status)) &&
    !(IMG_CHK_ABORT_LOCKED(p_base, &p_base->mutex))) {
    p_comp->client_id = p_frame->info.client_id;
    IDBG_MED("%s:%d] Sending QIMG_EVT_FACE_PROC ", __func__, __LINE__);
    IMG_SEND_EVENT(p_base, QIMG_EVT_FACE_PROC);
    IDBG_MED("%s:%d] after  QIMG_EVT_FACE_PROC ", __func__, __LINE__);
  }

  IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_dsp_comp_eng_get_output_task_exec
 *
 * Description: executes the sending of BUF_DONE event upstream
 * as a job in loadbalancer/threadmgr on ARM thread so that DSP
 * thread is not blocked.
 *
 * Input parameters:
 *   param - The pointer to function/usecase specific params
 *   structure
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_dsp_comp_eng_get_output_task_exec(void *param)
{
  if (NULL == param) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  faceproc_dsp_comp_eng_get_output_struct * pstruct =
    (faceproc_dsp_comp_eng_get_output_struct *)param;
  IDBG_LOW("FD_DSP_JOB : Start");

  faceproc_dsp_comp_get_result_and_send(pstruct->p_comp);
  pstruct->return_value = IMG_SUCCESS;
  free(pstruct);
  IDBG_LOW("FD_DSP_JOB : End");
  return IMG_SUCCESS;

}

/**
 * Function: faceproc_dsp_comp_eng_destroy_task_exec
 *
 * Description: executes the eng_destroy function as a job in
 * loadbalancer/threadmgr. eng_destroy has actual
 * implementation.
 *
 * Input parameters:
 *   param - The pointer to function/usecase specific params
 *   structure
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_dsp_comp_eng_destroy_task_exec(void *param)
{
  if (NULL == param) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  faceproc_dsp_comp_struct * pstruct = (faceproc_dsp_comp_struct *)param;
  IDBG_LOW("FD_DSP_JOB : Start");
  pstruct->return_value = faceproc_dsp_comp_eng_destroy(pstruct->p_comp);
  IDBG_LOW("FD_DSP_JOB : end");
  return IMG_SUCCESS;

}

/**
 * Function: faceproc_dsp_comp_eng_load_task_exec
 *
 * Description: executes the eng_load function as a job in
 * loadbalancer/threadmgr. eng_load has actual
 * implementation.
 *
 * Input parameters:
 *   param - The pointer to function/usecase specific params
 *   structure
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_dsp_comp_eng_load_task_exec(void *param)
{
  if (NULL == param) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  faceproc_dsp_comp_eng_load_struct * pstruct =
    (faceproc_dsp_comp_eng_load_struct *)param;
  IDBG_LOW("FD_DSP_JOB : Start");
  pstruct->return_value = faceproc_dsp_comp_eng_load(pstruct->p_lib);
  IDBG_LOW("FD_DSP_JOB : End");
  return IMG_SUCCESS;

}

/**
 * Function: faceproc_dsp_comp_eng_unload_task_exec
 *
 * Description: executes the eng_unload function as a job in
 * loadbalancer/threadmgr. eng_unload has actual implementation.
 *
 * Input parameters:
 *   param - The pointer to function/usecase specific params
 *   structure
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_dsp_comp_eng_unload_task_exec(void *param)
{
  if (NULL == param) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  faceproc_dsp_comp_eng_load_struct * pstruct =
    (faceproc_dsp_comp_eng_load_struct *)param;
  IDBG_LOW("FD_DSP_JOB : Start");
  pstruct->return_value = IMG_SUCCESS;
  faceproc_dsp_comp_eng_unload(pstruct->p_lib);
  IDBG_LOW("FD_DSP_JOB : End");
  return IMG_SUCCESS;

}

/**
 * Function: faceproc_dsp_comp_eng_load_task_exec
 *
 * Description: executes the eng_test_dsp_connection function as
 * a job in loadbalancer/threadmgr. eng_test_dsp_connection has
 * actual implementation for calling DSP rpc call.
 *
 * Input parameters:
 *   param - The pointer to function/usecase specific params
 *   structure
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: noneg
 **/
int faceproc_dsp_comp_eng_test_dsp_connection_task_exec(void *param)
{
  if (NULL == param) {
    IDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  faceproc_dsp_comp_eng_load_struct * pstruct =
    (faceproc_dsp_comp_eng_load_struct *)param;
  IDBG_LOW("FD_DSP_JOB : Start");
  pstruct->return_value = faceproc_dsp_comp_eng_test_dsp_connection(
     pstruct->p_lib);
  IDBG_MED("FD_DSP_JOB : End input pstruct->return_value %d",
    pstruct->return_value);
  return IMG_SUCCESS;

}

