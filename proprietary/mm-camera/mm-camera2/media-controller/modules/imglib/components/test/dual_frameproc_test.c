/**********************************************************************
*  Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "img_test.h"
#include "img_thread_ops.h"

/**
 * CONSTANTS and MACROS
 **/
#define NUM_THREAD 4

/**
 * Function: dual_frameproc_test_handle_dcrf_done
 *
 * Description: Function to handle dcrf done event
 *
 * Arguments:
 *   @p_test - dual frame test object
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static void dual_frameproc_test_handle_dcrf_done(dual_frameproc_test_t *p_test)
{
  int rc = IMG_SUCCESS;
  img_dcrf_output_result_t result;
  img_dcrf_output_result_t *p_result = &result;

  if (!p_test) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return;
  }

  img_component_ops_t *p_comp;
  p_comp = &p_test->main_base->comp;

  rc = IMG_COMP_GET_PARAM(p_comp, QIMG_DCRF_RESULT, (void *)p_result);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return;
  }

  if (p_result) {

    IDBG_MED("%s:%d] DCRF result frameID:%d tmestamp %lld dis %d confidence %d"
      " status %d roi %dx%d (%d,%d) focus(%d,%d) ",
      __func__, __LINE__,
      p_result->frame_id,
      p_result->timestamp,
      p_result->distance_in_mm,
      p_result->confidence,
      p_result->status,
      p_result->focused_roi.size.width,
      p_result->focused_roi.size.height,
      p_result->focused_roi.pos.x,
      p_result->focused_roi.pos.y,
      p_result->focused_x ,
      p_result->focused_y);
  }

  pthread_cond_signal(&p_test->main_base->cond);
}

/**
 * Function: dual_frameproc_test_event_handler
 *
 * Description: event handler for dual frameproc test case
 *
 * Input parameters:
 *   p_appdata - test object
 *   p_event - pointer to the event
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int dual_frameproc_test_event_handler(void* p_appdata, img_event_t *p_event)
{
  dual_frameproc_test_t *p_test = (dual_frameproc_test_t *)p_appdata;

  if ((NULL == p_event) || (NULL == p_appdata)) {
    IDBG_ERROR("%s:%d] invalid event", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  IDBG_HIGH("%s:%d] type %d", __func__, __LINE__, p_event->type);
  switch (p_event->type) {
  case QIMG_EVT_DCRF_DONE: {
    dual_frameproc_test_handle_dcrf_done(p_test);
    break;
  }
  case QIMG_EVT_BUF_DONE:
    break;
  default:;
  }
  return IMG_SUCCESS;
}

/**
 * Function: dual_frameproc_test_init
 *
 * Description: init dual frameproc test case
 *
 * Input parameters:
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int dual_frameproc_test_init(dual_frameproc_test_t *p_test)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_main_comp = &p_test->main_base->comp;
  img_component_ops_t *p_aux_comp = &p_test->aux_base->comp;
  img_core_ops_t *p_core_ops = &p_test->main_base->core_ops;
  img_caps_t caps;
  char *libname;
  img_frame_ops_t l_frameops = {
    .dump_frame = img_dump_frame,
    .get_meta = img_get_meta,
    .set_meta = img_set_meta,
    .image_copy = img_image_copy,
  };

  memset(&caps, 0x0, sizeof(caps));

  IDBG_HIGH("%s:%d] ", __func__, __LINE__);
  switch (p_test->main_base->algo_index) {
  case 5:  /* dcrf */
    caps.num_input = 2;
    caps.num_meta = 1;
    caps.num_output = 0;
    libname = "libmmcamera_dcrf_lib.so";
    break;
  default:
    IDBG_ERROR("%s:%d] Algo not supported %d", __func__, __LINE__,
      p_test->main_base->algo_index);
    break;
  }

  rc = img_core_get_comp(IMG_COMP_DUAL_FRAME_PROC, "qti.dual_frameproc", p_core_ops);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_LOAD(p_core_ops, libname);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }
  p_test->aux_base->core_ops = *p_core_ops;

  rc = IMG_COMP_CREATE(p_core_ops, p_main_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_INIT(p_main_comp, (void *)p_test, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_CB(p_main_comp, dual_frameproc_test_event_handler);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_main_comp, QIMG_PARAM_CAPS,
      (void *)&caps);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  l_frameops.p_appdata = p_test;
  rc = IMG_COMP_SET_PARAM(p_main_comp, QIMG_PARAM_FRAME_OPS, &l_frameops);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_CREATE(p_core_ops, p_aux_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_INIT(p_aux_comp, (void *)p_test, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_aux_comp, QIMG_PARAM_CAPS,
      (void *)&caps);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  img_dual_cam_init_params_t dcrf_init_params;
  /*hack OTA calib data*/
  dcrf_init_params.dcrf_calib_data.calibration_format_version = 1;
  dcrf_init_params.dcrf_calib_data.relative_principle_point_x_offset = 0.0f;
  dcrf_init_params.dcrf_calib_data.relative_principle_point_y_offset = 0.0f;
  dcrf_init_params.dcrf_calib_data.relative_baseline_distance = 20.0f;
  dcrf_init_params.dcrf_calib_data.relative_position_flag = 0;

  dcrf_init_params.main.width = p_test->main_base->width;
  dcrf_init_params.main.height = p_test->main_base->height;
  dcrf_init_params.main.stride = p_test->main_base->stride;
  dcrf_init_params.main.format = DCRF_FORMAT_YUV;

  dcrf_init_params.aux.width = p_test->aux_base->width;
  dcrf_init_params.aux.height = p_test->aux_base->height;
  dcrf_init_params.aux.stride = p_test->aux_base->stride;
  dcrf_init_params.aux.format = DCRF_FORMAT_YUV;

  dcrf_init_params.alignment_check_enable = 0;
  dcrf_init_params.aux_direction = IMG_AF_CAM_ORIENT_LEFT;
  dcrf_init_params.baseline_mm = 20;
  dcrf_init_params.focus_preference = 0x110;
  dcrf_init_params.macro_est_limit_cm = 10;

  rc = IMG_COMP_SET_PARAM(p_main_comp, QIMG_PARAM_DCRF, &dcrf_init_params);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] Set dcrf init params failed %d",
      __func__, __LINE__, rc);
  }

  rc = img_thread_mgr_create_pool();
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d]Error create thread pool %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_BIND(p_core_ops, p_main_comp, p_aux_comp);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] Bind failed %d", __func__, __LINE__, rc);
    return rc;
  }

  return rc;
}

/**
 * Function: dual_frameproc_test_fill_runtime_params
 *
 * Description: helper to fill runtime params for DCRF
 *
 * Arguments:
 *   @p_testbase - pointer base test obect
 *   @p_meta - pointer to meta data
 *
 * Return values:
 * None
 *
 * Notes: none
 **/
void dual_frameproc_test_fill_runtime_params(imglib_test_t *p_testbase,
  img_meta_t *p_meta)
{
  if (NULL == p_meta || NULL == p_testbase) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return;
  }

  img_dcrf_input_runtime_param_t dcrf_runtime_param;

  dcrf_runtime_param.lens_zoom_ratio = 1.0f;
  dcrf_runtime_param.roi_of_main.size.width = p_testbase->width / 3;
  dcrf_runtime_param.roi_of_main.size.height= p_testbase->height/ 3;
  dcrf_runtime_param.roi_of_main.pos.x = p_testbase->width / 2;
  dcrf_runtime_param.roi_of_main.pos.y = p_testbase->height / 2;
  dcrf_runtime_param.af_fps = 30;
  /*hack lens zoom ratio */
  if (dcrf_runtime_param.lens_zoom_ratio == 0.0f) {
    dcrf_runtime_param.lens_zoom_ratio = 1.0f;
  }

  /* Fill FOVs */
  dcrf_runtime_param.fov_params_main[0].module = SENSOR_FOV;
  dcrf_runtime_param.fov_params_main[0].input_width = p_testbase->width;
  dcrf_runtime_param.fov_params_main[0].input_height = p_testbase->height;
  dcrf_runtime_param.fov_params_main[0].offset_x = 0;
  dcrf_runtime_param.fov_params_main[0].offset_y = 0;
  dcrf_runtime_param.fov_params_main[0].fetch_window_width = p_testbase->width;
  dcrf_runtime_param.fov_params_main[0].fetch_window_height = p_testbase->height;
  dcrf_runtime_param.fov_params_main[0].output_window_width = p_testbase->width;
  dcrf_runtime_param.fov_params_main[0].output_window_height = p_testbase->height;

  dcrf_runtime_param.fov_params_main[1].module = ISPIF_FOV;
  dcrf_runtime_param.fov_params_main[1].input_width =
    dcrf_runtime_param.fov_params_main[0].output_window_width;
  dcrf_runtime_param.fov_params_main[1].input_height =
    dcrf_runtime_param.fov_params_main[0].output_window_height;
  dcrf_runtime_param.fov_params_main[1].offset_x = 0;
  dcrf_runtime_param.fov_params_main[1].offset_y = 0;
  dcrf_runtime_param.fov_params_main[1].fetch_window_width =
    dcrf_runtime_param.fov_params_main[1].input_width;
  dcrf_runtime_param.fov_params_main[1].fetch_window_height =
    dcrf_runtime_param.fov_params_main[1].input_height;
  dcrf_runtime_param.fov_params_main[1].output_window_width =
    dcrf_runtime_param.fov_params_main[1].input_width;
  dcrf_runtime_param.fov_params_main[1].output_window_height =
    dcrf_runtime_param.fov_params_main[1].input_height;

  dcrf_runtime_param.fov_params_main[2].module = CAMIF_FOV;
  dcrf_runtime_param.fov_params_main[2].input_width =
    dcrf_runtime_param.fov_params_main[1].output_window_width;
  dcrf_runtime_param.fov_params_main[2].input_height =
    dcrf_runtime_param.fov_params_main[1].output_window_height;
  dcrf_runtime_param.fov_params_main[2].offset_x = 0;
  dcrf_runtime_param.fov_params_main[2].offset_y = 0;
  dcrf_runtime_param.fov_params_main[2].fetch_window_width = p_testbase->width;
  dcrf_runtime_param.fov_params_main[2].fetch_window_height = p_testbase->height;
  dcrf_runtime_param.fov_params_main[2].output_window_width =
    dcrf_runtime_param.fov_params_main[2].fetch_window_width;
  dcrf_runtime_param.fov_params_main[2].output_window_height =
    dcrf_runtime_param.fov_params_main[2].fetch_window_height;

  dcrf_runtime_param.fov_params_main[3].module = ISP_OUT_FOV;
  dcrf_runtime_param.fov_params_main[3].input_width =
    dcrf_runtime_param.fov_params_main[2].output_window_width;
  dcrf_runtime_param.fov_params_main[3].input_height =
    dcrf_runtime_param.fov_params_main[2].output_window_height;
  dcrf_runtime_param.fov_params_main[3].offset_x = 0;
  dcrf_runtime_param.fov_params_main[3].offset_y = 0;
  dcrf_runtime_param.fov_params_main[3].fetch_window_width = p_testbase->width;
  dcrf_runtime_param.fov_params_main[3].fetch_window_height = p_testbase->height;
  dcrf_runtime_param.fov_params_main[3].output_window_width = p_testbase->width;
  dcrf_runtime_param.fov_params_main[3].output_window_height = p_testbase->height;

  /* Add DCRF runtime cfg to meta data */
  IDBG_MED("%s:%d] Set meta for DCRF", __func__, __LINE__);
  int32_t rc = img_set_meta(p_meta, IMG_META_DCRF_RUNTIME_PARAM,
    &dcrf_runtime_param);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
  }

}

/**
 * Function: dual_frameproc_test_start
 *
 * Description: start dual frameproc test case
 *
 * Input parameters:
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int dual_frameproc_test_process_frame(dual_frameproc_test_t *p_test)
{
  uint32_t i = 0;
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_main_comp = &p_test->main_base->comp;
  img_component_ops_t *p_aux_comp = &p_test->aux_base->comp;

  for (i = 0; i < p_test->main_base->in_count; i++) {
    IDBG_HIGH("%s:%d] dim %dx%d frame %p", __func__, __LINE__,
      p_test->main_base->frame[i].info.width,
      p_test->main_base->frame[i].info.height,
      &p_test->main_base->frame[i]);
    /*hack time stamp*/
    p_test->main_base->frame[i].timestamp = (1000000/30) * (i + 1);
    p_test->main_base->frame[i].frame_id = i;
    p_test->aux_base->frame[i].timestamp = (1000000/30) * (i + 1);
    p_test->aux_base->frame[i].frame_id = i;

    rc = IMG_COMP_Q_BUF(p_main_comp, &p_test->main_base->frame[i], IMG_IN);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      return rc;
    }
    dual_frameproc_test_fill_runtime_params(p_test->main_base,
      &p_test->main_base->meta_frame[i]);
    p_test->main_base->meta_frame[i].frame_id = i;
    /* queue the meta buffer */
    rc = IMG_COMP_Q_META_BUF(p_main_comp, &p_test->main_base->meta_frame[i]);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      return rc;
    }

    rc = IMG_COMP_Q_BUF(p_aux_comp, &p_test->aux_base->frame[i], IMG_IN);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      return rc;
    }
    dual_frameproc_test_fill_runtime_params(p_test->aux_base,
      &p_test->aux_base->meta_frame[i]);
    p_test->aux_base->meta_frame[i].frame_id = i;
    /* queue the meta buffer */
    rc = IMG_COMP_Q_META_BUF(p_aux_comp, &p_test->aux_base->meta_frame[i]);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      return rc;
    }
  }
  return rc;
}

/**
 * Function: dual_frameproc_test_start
 *
 * Description: start dual frameproc test case
 *
 * Input parameters:
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int dual_frameproc_test_start(dual_frameproc_test_t *p_test)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_main_comp = &p_test->main_base->comp;
  img_component_ops_t *p_aux_comp = &p_test->aux_base->comp;

  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] Bind failed %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_START(p_main_comp, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_START(p_aux_comp, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  /* wait for the result */
  pthread_mutex_lock(&p_test->main_base->mutex);
  rc = dual_frameproc_test_process_frame(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_test->main_base->mutex);
    return rc;
  }
  IDBG_HIGH("%s:%d] before wait rc %d", __func__, __LINE__, rc);
  rc = img_wait_for_completion(&p_test->main_base->cond, &p_test->main_base->mutex,
    1000);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_test->main_base->mutex);
    return rc;
  }
  IDBG_HIGH("%s:%d] after wait rc %d", __func__, __LINE__, rc);
  pthread_mutex_unlock(&p_test->main_base->mutex);
  return rc;
}

/**
 * Function: dual_frameproc_test_deinit
 *
 * Description: deinit dual frameproc test case
 *
 * Input parameters:
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int dual_frameproc_test_deinit(dual_frameproc_test_t *p_test)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_main_comp = &p_test->main_base->comp;
  img_component_ops_t *p_aux_comp = &p_test->aux_base->comp;
  img_core_ops_t *p_core_ops = &p_test->main_base->core_ops;

  rc = IMG_COMP_UNBIND(p_core_ops, p_main_comp, p_aux_comp);

  rc = IMG_COMP_ABORT(p_aux_comp, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_DEINIT(p_aux_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_ABORT(p_main_comp, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_DEINIT(p_main_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  IMG_COMP_UNLOAD(p_core_ops);

  img_thread_mgr_destroy_pool();
  return IMG_SUCCESS;
}

/**
 * Function: dual_frameproc_test_finish
 *
 * Description: finish executing dual frameproc test case
 *
 * Input parameters:
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int dual_frameproc_test_finish(dual_frameproc_test_t *p_test)
{
  int rc = IMG_SUCCESS;

  IDBG_INFO("%s:%d] E", __func__, __LINE__);

  rc = dual_frameproc_test_deinit(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }
  return IMG_SUCCESS;
}

/**
 * Function: dual_frameproc_test_execute
 *
 * Description: execute dual frameproc test case
 *
 * Input parameters:
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int dual_frameproc_test_execute(dual_frameproc_test_t *p_test)
{
  int rc = IMG_SUCCESS;

  rc = dual_frameproc_test_init(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return rc;
  }

  rc = dual_frameproc_test_start(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return rc;
  }

  rc = dual_frameproc_test_finish(p_test);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return rc;
  }
  return rc;
}
