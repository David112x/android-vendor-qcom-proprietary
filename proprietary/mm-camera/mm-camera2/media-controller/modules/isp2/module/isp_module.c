/* isp_module.c
 *
 * Copyright (c) 2012-2014, 2018 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <stdio.h>

/* mctl headers */
#include "media_controller.h"
#include "mct_list.h"
#include "mct_pipeline.h"
#include "mct_profiler.h"
#include "camera_dbg.h"

/* isp headers */
#include "isp_port.h"
#include "isp_module.h"
#include "isp_util.h"
#include "isp_trigger_thread.h"
#include "isp_hw_update_thread.h"
#include "isp_parser_thread.h"
#include "isp_log.h"
#include "isp_algo.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

uint32_t isp_modules_loglevel[ISP_LOG_MAX] = {0};

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COMMON, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COMMON, fmt, ##args)

/** isp_module_set_mod:
 *
 *  @module: module handle
 *  @module_type: module type
 *  @identity: identity of stream
 *
 *  Handle set mod
 *
 *  Returns void
 **/
static void isp_module_set_mod(mct_module_t *module, unsigned int module_type,
  unsigned int identity)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }
  mct_module_add_type(module, module_type, identity);
  return;
}

/** isp_module_query_mod:
 *
 *  @module: module handle
 *  @query_buf: query caps buffer handle
 *  @sessionid: session id
 *
 *  Return ISP capabilities
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_module_query_mod(mct_module_t *module, void *query_buf,
  unsigned int sessionid)
{
  boolean                 ret = TRUE;
  mct_pipeline_isp_cap_t *isp_cap = NULL;
  mct_pipeline_cap_t     *cap_buf = (mct_pipeline_cap_t *)query_buf;
  isp_t                  *isp = NULL;
  uint32_t                index = 0;
  isp_hw_id_t             isp_id = ISP_HW_0;
  isp_pipeline_t         *isp_pipeline = NULL;

  if (!query_buf || !module) {
    ISP_ERR("failed query_buf %p s_module %p", query_buf, module);
    return FALSE;
  }

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  isp_cap = &cap_buf->isp_cap;
  isp_cap->zoom_ratio_tbl_cnt = 0;

  isp_cap->supported_effects_cnt = 9;
  isp_cap->supported_effects[0] = CAM_EFFECT_MODE_OFF;
  isp_cap->supported_effects[1] = CAM_EFFECT_MODE_MONO;
  isp_cap->supported_effects[2] = CAM_EFFECT_MODE_NEGATIVE;
  isp_cap->supported_effects[3] = CAM_EFFECT_MODE_SOLARIZE;
  isp_cap->supported_effects[4] = CAM_EFFECT_MODE_SEPIA;
  isp_cap->supported_effects[5] = CAM_EFFECT_MODE_POSTERIZE;
  isp_cap->supported_effects[6] = CAM_EFFECT_MODE_WHITEBOARD;
  isp_cap->supported_effects[7] = CAM_EFFECT_MODE_BLACKBOARD;
  isp_cap->supported_effects[8] = CAM_EFFECT_MODE_AQUA;

  isp_cap->supported_scene_modes_cnt = 19;
  isp_cap->supported_scene_modes[0] = CAM_SCENE_MODE_OFF;
  isp_cap->supported_scene_modes[1] = CAM_SCENE_MODE_AUTO;
  isp_cap->supported_scene_modes[2] = CAM_SCENE_MODE_LANDSCAPE;
  isp_cap->supported_scene_modes[3] = CAM_SCENE_MODE_SNOW;
  isp_cap->supported_scene_modes[4] = CAM_SCENE_MODE_BEACH;
  isp_cap->supported_scene_modes[5] = CAM_SCENE_MODE_SUNSET;
  isp_cap->supported_scene_modes[6] = CAM_SCENE_MODE_NIGHT;
  isp_cap->supported_scene_modes[7] = CAM_SCENE_MODE_PORTRAIT;
  isp_cap->supported_scene_modes[8] = CAM_SCENE_MODE_BACKLIGHT;
  isp_cap->supported_scene_modes[9] = CAM_SCENE_MODE_SPORTS;
  isp_cap->supported_scene_modes[10] = CAM_SCENE_MODE_ANTISHAKE;
  isp_cap->supported_scene_modes[11] = CAM_SCENE_MODE_FLOWERS;
  isp_cap->supported_scene_modes[12] = CAM_SCENE_MODE_CANDLELIGHT;
  isp_cap->supported_scene_modes[13] = CAM_SCENE_MODE_FIREWORKS;
  isp_cap->supported_scene_modes[14] = CAM_SCENE_MODE_PARTY;
  isp_cap->supported_scene_modes[15] = CAM_SCENE_MODE_NIGHT_PORTRAIT;
  isp_cap->supported_scene_modes[16] = CAM_SCENE_MODE_THEATRE;
  isp_cap->supported_scene_modes[17] = CAM_SCENE_MODE_ACTION;
  isp_cap->supported_scene_modes[18] = CAM_SCENE_MODE_AR;

  isp_cap->max_meta_reporting_delay = 0;

  isp_cap->histogram_size = CAM_HISTOGRAM_STATS_SIZE;
  isp_cap->max_histogram_count = 256;

  ret = isp_resource_query_mod(&isp->isp_resource, query_buf, sessionid);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_resource_query_mod");
  }

  ISP_DBG("zoom tbl size %d", isp_cap->zoom_ratio_tbl_cnt);
  for (index = 0; index < isp_cap->zoom_ratio_tbl_cnt; index++) {
    ISP_DBG("zoom[%d] %d", index, isp_cap->zoom_ratio_tbl[index]);
  }

  /* These variables are common for both VFE's if one vfe is present, break the loop*/
  for (isp_id = ISP_HW_0; isp_id < ISP_HW_MAX; isp_id++) {
    isp_pipeline = isp->isp_resource.isp_resource_info[isp_id].isp_pipeline;
    if (isp_pipeline) {
      if (isp_pipeline->is_camif_raw_op_fmt_supported) {
        isp_cap->camif_supported_formats.format_count = 4;
        isp_cap->camif_supported_formats.format[0] = CAMIF_QCOM_FORMAT;
        isp_cap->camif_supported_formats.format[1] = CAMIF_PLAIN16_FORMAT;
        isp_cap->camif_supported_formats.format[2] = CAMIF_MIPI_FORMAT;
        isp_cap->camif_supported_formats.format[3] = CAMIF_PLAIN8_FORMAT;
      } else {
        isp_cap->camif_supported_formats.format_count = 1;
        isp_cap->camif_supported_formats.format[0] = CAMIF_QCOM_FORMAT;
      }
      isp_cap->max_viewfinder_size.width = isp_pipeline->hw_stream_info
        [ISP_HW_STREAM_VIEWFINDER].max_width;
      isp_cap->max_viewfinder_size.height = isp_pipeline->hw_stream_info
        [ISP_HW_STREAM_VIEWFINDER].max_height;
      isp_cap->max_encoder_size.width = isp_pipeline->hw_stream_info
        [ISP_HW_STREAM_ENCODER].max_width;
      isp_cap->max_encoder_size.height = isp_pipeline->hw_stream_info
        [ISP_HW_STREAM_ENCODER].max_height;
      isp_cap->max_frame_applying_delay = isp_pipeline->pipeline_applying_delay;
      if (isp_pipeline->max_scale_ratio){
        isp_cap->max_downscale_factor = isp_pipeline->max_scale_ratio;
      } else {
        isp_cap->max_downscale_factor = DEFAULT_MIN_SCALE_RATIO;
      }
      isp_cap->is_mono_stats_support = isp_pipeline->is_mono_stats_supported;
      break;
    }
  }

  /* save capability in isp */
  isp->isp_cap = *isp_cap;

  return ret;
}

/** isp_module_start_session:
 *
 *  @module: module handle
 *  @sessionid: session id
 *
 *  Handle start session
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_module_start_session(mct_module_t *module,
  unsigned int session_id)
{
  boolean                      ret = TRUE;
  isp_t                       *isp = NULL;
  mct_list_t                  *session_params = NULL;
  isp_session_param_t         *session_param = NULL;
  isp_hw_id_t                  hw_index = 0;
  isp_trigger_update_params_t *trigger_params = NULL;
  isp_hw_update_params_t      *hw_update_params = NULL;
  isp_parser_params_t         *parser_params = NULL;
  uint32_t                     setprop_enable = 0;
  char                         value[PROPERTY_VALUE_MAX];
  isp_resource_t              *isp_resource = NULL;
  uint32_t                     max_clk = 0;
  uint32_t                     i = 0;
  uint32_t                     dual_vfe_forced = 0;

  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  ISP_INFO("session id %d ", session_id);
  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  MCT_PROF_LOG_BEG(PROF_ISP_START_SESSION);

  isp_resource = &isp->isp_resource;
  PTHREAD_MUTEX_LOCK(&isp_resource->mutex);

  isp_resource->num_session_opened++;

  if (!isp_resource->num_isp_preallocate) {
#ifdef _ANDROID_
    /*enforce dual vfe enable by set property*/
    property_get("persist.vendor.camera.isp.dualisp", value, "0");
    dual_vfe_forced = atoi(value);
#endif

    /* Get max_op_pixel_clk and sensor format for this session */
    for (i = 0; i < ISP_MAX_SESSION; i++) {
      /* Check whether this session id exist */
      if ((isp_resource->session_info[i].is_valid == TRUE) &&
        (isp_resource->session_info[i].session_id == session_id) &&
        (isp_resource->session_info[i].format != FORMAT_YCBCR)) {

        /* Get max_nominal_clk */
        if (isp_resource->isp_resource_info[ISP_HW_0].isp_pipeline) {
          max_clk =
            isp_resource->isp_resource_info[ISP_HW_0].isp_pipeline->
            max_pix_clk_rate[ISP_CX_LEVEL_NOMINAL];
        } else {
          max_clk = ISP_PIX_CLK_600MHZ;
        }

        /* If this is pip case, reserve single vfe */
        if (dual_vfe_forced ||
          (isp_resource->session_info[i].max_op_pixel_clk > max_clk)) {
          /* If max op pixel clk for this session >= max nominal clk, reserve 2 vfe */
           isp_resource->num_isp_preallocate =
           isp_resource->session_info[i].num_isp_preallocate = 2;
        } else {
          isp_resource->num_isp_preallocate =
          isp_resource->session_info[i].num_isp_preallocate = 1;
        }
        break;
      }
    }
  }

  PTHREAD_MUTEX_UNLOCK(&isp_resource->mutex);

  //PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));
  if (isp->l_session_params) {
    /* Check whether this session is already opened */
    session_params = mct_list_find_custom(isp->l_session_params, &session_id,
      isp_util_compare_sessionid_from_session_param);
    if (session_params) {
      /* Start session called twice without calling stop session for same
       * session
       */
      //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
      MCT_PROF_LOG_END();
      return FALSE;
    }
  }
  //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  /* Call start_session on sub modules */
  ret = isp_resource_start_session(&isp->isp_resource, session_id);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_resource_start_session");
    MCT_PROF_LOG_END();
    return ret;
  }

  /* Create ctrl struct for this session */
  session_param = (isp_session_param_t *)CAM_CALLOC(1, sizeof(*session_param));
  if (!session_param) {
    ISP_ERR("failed: isp_session_param %p", session_param);
    goto ERROR_MALLOC;
  }

  trigger_params = &session_param->trigger_update_params;
  hw_update_params = &session_param->hw_update_params;
  parser_params = &session_param->parser_params;
  pthread_cond_init(&trigger_params->cond, NULL);
  pthread_mutex_init(&trigger_params->mutex, NULL);
  pthread_cond_init(&parser_params->cond, NULL);
  pthread_mutex_init(&parser_params->mutex, NULL);
  pthread_mutex_init(&session_param->mutex, NULL);
  pthread_mutex_init(&session_param->metadata_mutex, NULL);
  pthread_mutex_init(&session_param->state_mutex, NULL);
  pthread_mutex_init(&session_param->thread_busy_mutex, NULL);
  pthread_mutex_init(&parser_params->buf_mgr[ISP_STREAMING_ONLINE].mutex, NULL);
  pthread_mutex_init(&parser_params->buf_mgr[ISP_STREAMING_OFFLINE].mutex, NULL);
  session_param->session_id = session_id;
  session_param->state = ISP_STATE_IDLE;
  trigger_params->is_thread_alive = FALSE;
  hw_update_params->is_thread_alive = FALSE;
  parser_params->is_thread_alive = FALSE;
  session_param->lpm_info.isupdated = FALSE;

#ifdef _ANDROID_
  property_get("persist.vendor.camera.dumpmetadata", value, "0");
  setprop_enable = atoi(value);
  hw_update_params->metadata_dump_enable = setprop_enable;

  property_get("persist.vendor.camera.saturationext", value, "0");
  setprop_enable = atoi(value);
  if (setprop_enable) {
    parser_params->parser_session_params.saturated_ext = TRUE;
  } else {
    parser_params->parser_session_params.saturated_ext = FALSE;
  }

  property_get("persist.vendor.camera.tintless.skip", value,
   TINTLESS_FRAME_SKIP_STRING);
  parser_params->algo_saved_parm.tintless_saved_algo_parm.tintless_algo_skip =
    atoi(value);
#else
  hw_update_params->metadata_dump_enable = 0;
  parser_params->parser_session_params.saturated_ext = FALSE;
  parser_params->algo_saved_parm.tintless_saved_algo_parm.tintless_algo_skip =
    TINTLESS_FRAME_SKIP;
#endif
  parser_params->algo_saved_parm.tintless_saved_algo_parm.algo_skip_cnt = 1;

  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    isp_hw_update_list_params_t *hw_update_list_params =
      &session_param->hw_update_params.hw_update_list_params[hw_index];
    if (hw_update_params->metadata_dump_enable) {
      hw_update_list_params->meta_reg_dump = (uint32_t*)CAM_MALLOC(
         (isp->isp_resource.isp_resource_info[hw_index].
        isp_pipeline->num_register) * sizeof(uint32_t));
      hw_update_list_params->temp_meta_dump_parms = (isp_meta_t *)
       CAM_MALLOC(sizeof(isp_meta_t));
      if (hw_update_list_params->temp_meta_dump_parms)
        memset(hw_update_list_params->temp_meta_dump_parms, 0,
             sizeof(isp_meta_t));
    }
    session_param->hw_id[hw_index] = ISP_HW_MAX;
    pthread_mutex_init(&session_param->hw_update_params.mutex, NULL);
    pthread_cond_init(&session_param->hw_update_params.cond, NULL);

    hw_update_list_params->stats_params =
      (isp_saved_stats_params_t *)CAM_CALLOC(1, sizeof(isp_saved_stats_params_t));
    if (!hw_update_list_params->stats_params) {
      ISP_ERR("failed: memory allocation");
      goto ERROR_TRIGGER_THREAD_CREATE;
    }

    hw_update_list_params->applied_stats_params =
      (isp_saved_stats_params_t *)CAM_CALLOC(1, sizeof(isp_saved_stats_params_t));
    if (!hw_update_list_params->applied_stats_params) {
      ISP_ERR("failed: memory allocation");
      goto ERROR_TRIGGER_THREAD_CREATE;
    }
  }
  /* Create isp trigger update thread for this session */
  ret = isp_trigger_thread_create(module, session_param, &isp->isp_resource);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_thread_create");
    goto ERROR_TRIGGER_THREAD_CREATE;
  }

  /* Create isp hw update thread for this session */
  ret = isp_hw_update_thread_create(&isp->isp_resource, session_param);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_thread_create");
    goto ERROR_HW_UPDATE_THREAD_CREATE;
  }

  /* Create isp parser thread for this session */
  ret = isp_parser_thread_create(module, &isp->isp_resource, session_param);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_parser_thread_create");
    goto ERROR_PARSER_THREAD_CREATE;
  }

  session_param->offline_trigger_param_q =
      (mct_queue_t *)CAM_CALLOC(1, sizeof(mct_queue_t));
  if (!session_param->offline_trigger_param_q) {
      ISP_ERR("failed: to allocate offline_trigger_param_q");
      ret = FALSE;
      goto ERROR_SESSION_PARAMS_APPEND;
    }
    mct_queue_init(session_param->offline_trigger_param_q);

  //PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));
  /* Add session_param to list of sessions */
  session_params = mct_list_append(isp->l_session_params, session_param, NULL,
    NULL);
  if (!session_params) {
    ISP_ERR("failed: session_params %p", session_params);
    //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
    ret = FALSE;
    goto ERROR_SESSION_PARAMS_APPEND;
  }
  isp->l_session_params = session_params;
  //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  MCT_PROF_LOG_END();
  ISP_DBG("num_session_opened %d", isp_resource->num_session_opened);

  return ret;

ERROR_SESSION_PARAMS_APPEND:
  if (session_param->offline_trigger_param_q) {
    free(session_param->offline_trigger_param_q);
    session_param->offline_trigger_param_q = NULL;
  }
  isp_parser_thread_join(session_param);
ERROR_PARSER_THREAD_CREATE:
  isp_hw_update_thread_join(session_param);
ERROR_HW_UPDATE_THREAD_CREATE:
  isp_trigger_thread_join(session_param);
ERROR_TRIGGER_THREAD_CREATE:
  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    isp_hw_update_list_params_t *hw_update_list_params =
      &session_param->hw_update_params.hw_update_list_params[hw_index];

    if (hw_update_list_params->stats_params) {
      free(hw_update_list_params->stats_params);
      hw_update_list_params->stats_params = NULL;
    }

    if (hw_update_list_params->applied_stats_params) {
      free(hw_update_list_params->applied_stats_params);
      hw_update_list_params->applied_stats_params = NULL;
    }
    if (hw_update_list_params->meta_reg_dump) {
      free(hw_update_list_params->meta_reg_dump);
      hw_update_list_params->meta_reg_dump = NULL;
    }
    if (hw_update_list_params->temp_meta_dump_parms) {
      free(hw_update_list_params->temp_meta_dump_parms);
      hw_update_list_params->temp_meta_dump_parms = NULL;
    }

    pthread_mutex_destroy(&session_param->hw_update_params.mutex);
    pthread_cond_destroy(&session_param->hw_update_params.cond);
  }
  pthread_mutex_destroy(&parser_params->buf_mgr[ISP_STREAMING_ONLINE].mutex);
  pthread_mutex_destroy(&parser_params->buf_mgr[ISP_STREAMING_OFFLINE].mutex);
  pthread_cond_destroy(&trigger_params->cond);
  pthread_mutex_destroy(&trigger_params->mutex);
  pthread_cond_destroy(&parser_params->cond);
  pthread_mutex_destroy(&parser_params->mutex);
  pthread_mutex_destroy(&session_param->mutex);
  pthread_mutex_destroy(&session_param->metadata_mutex);
  pthread_mutex_destroy(&session_param->state_mutex);
  pthread_mutex_destroy(&session_param->thread_busy_mutex);
  free(session_param);
ERROR_MALLOC:
  isp_resource_stop_session(&isp->isp_resource, session_id);
  PTHREAD_MUTEX_LOCK(&isp_resource->mutex);
  if (isp_resource->num_session_opened > 0) {
    isp_resource->num_session_opened--;
  }
  PTHREAD_MUTEX_UNLOCK(&isp_resource->mutex);
  ISP_ERR("failed: isp_module_start_session");

  MCT_PROF_LOG_END();

  return ret;
}

/** isp_module_stop_session:
 *
 *  @module: module handle
 *  @session_id: session id
 *
 *  Handle stop session
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_module_stop_session(mct_module_t *module,
  unsigned int session_id)
{
  boolean                      ret = TRUE;
  isp_t                       *isp = NULL;
  isp_session_param_t         *session_param = NULL;
  isp_hw_id_t                  hw_index = 0;
  isp_trigger_update_params_t *trigger_params = NULL;
  isp_parser_params_t         *parser_params = NULL;
  uint32_t                     i = 0;
  isp_resource_t              *isp_resource = NULL;

  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  ISP_INFO("session id %d", session_id);
  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  MCT_PROF_LOG_BEG(PROF_ISP_STOP_SESSION);

  isp_resource = &isp->isp_resource;
  PTHREAD_MUTEX_LOCK(&isp_resource->mutex);

  if (isp_resource->num_session_opened > 0) {
    isp_resource->num_session_opened--;
  }

  /* Get max_op_pixel_clk and sensor format for this session */
  for (i = 0; i < ISP_MAX_SESSION; i++) {
    /* Check whether this session id exist */
    if ((isp_resource->session_info[i].is_valid == TRUE) &&
      (isp_resource->session_info[i].session_id == session_id) &&
      (isp_resource->session_info[i].format != FORMAT_YCBCR)) {
       isp_resource->num_isp_preallocate -=
         isp_resource->session_info[i].num_isp_preallocate;
       isp_resource->session_info[i].num_isp_preallocate = 0;
      break;
    }
  }

  PTHREAD_MUTEX_UNLOCK(&isp_resource->mutex);

  ret = isp_util_get_session_params(module, session_id, &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    MCT_PROF_LOG_END();
    return FALSE;
  }

  /* Check if all ISP resources are freed and free it if not */
  if (session_param->num_isp) {
    ISP_INFO("warning: ISP resource is not freed on time!");
    ret = isp_resource_deallocate(module, &isp->isp_resource, session_id,
      &session_param->num_isp, session_param->hw_id);
    if (ret == FALSE) {
      ISP_ERR("failed: isp resource deallocate");
    }
  }
  if (session_param->offline_num_isp) {
    ISP_INFO("warning: ISP offline resource is not freed on time!");
    ret = isp_resource_deallocate(module, &isp->isp_resource, session_id,
      &session_param->offline_num_isp, session_param->offline_hw_id);
    if (ret == FALSE) {
      ISP_ERR("failed: isp resource deallocate");
    }
  }

  //PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));
  isp->l_session_params = mct_list_remove(isp->l_session_params,
    (void *)session_param);
  //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  /* Join ISP thread*/
  isp_parser_thread_join(session_param);

  /* Join ISP thread*/
  isp_trigger_thread_join(session_param);

  /* Join ISP thread*/
  isp_hw_update_thread_join(session_param);

  if (session_param->offline_trigger_param_q) {
    free(session_param->offline_trigger_param_q);
    session_param->offline_trigger_param_q = NULL;
  }

  /* Call stop session on algo */
  isp_algo_stop_session(module, session_param);

  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    isp_hw_update_list_params_t *hw_update_list_params =
      &session_param->hw_update_params.hw_update_list_params[hw_index];

    if (hw_update_list_params->stats_params) {
      free(hw_update_list_params->stats_params);
      hw_update_list_params->stats_params = NULL;
    }
    if (hw_update_list_params->applied_stats_params) {
      free(hw_update_list_params->applied_stats_params);
      hw_update_list_params->applied_stats_params = NULL;
    }
    if (hw_update_list_params->meta_reg_dump) {
      free(hw_update_list_params->meta_reg_dump);
      hw_update_list_params->meta_reg_dump = NULL;
    }
    if (hw_update_list_params->temp_meta_dump_parms) {
      free(hw_update_list_params->temp_meta_dump_parms);
      hw_update_list_params->temp_meta_dump_parms = NULL;
    }
  }
  pthread_mutex_destroy(&session_param->hw_update_params.mutex);
  pthread_cond_destroy(&session_param->hw_update_params.cond);
  trigger_params = &session_param->trigger_update_params;
  parser_params = &session_param->parser_params;
  pthread_mutex_destroy(&parser_params->buf_mgr[ISP_STREAMING_ONLINE].mutex);
  pthread_mutex_destroy(&parser_params->buf_mgr[ISP_STREAMING_OFFLINE].mutex);
  pthread_cond_destroy(&trigger_params->cond);
  pthread_mutex_destroy(&trigger_params->mutex);
  pthread_cond_destroy(&parser_params->cond);
  pthread_mutex_destroy(&parser_params->mutex);
  pthread_mutex_destroy(&session_param->state_mutex);
  pthread_mutex_destroy(&session_param->thread_busy_mutex);
  pthread_mutex_destroy(&session_param->mutex);
  pthread_mutex_destroy(&session_param->metadata_mutex);

  isp_util_free_saved_events(&session_param->saved_events);

  free(session_param);

  /* Call stop_session on sub modules */
  ret = isp_resource_stop_session(&isp->isp_resource, session_id);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_resource_stop_session");
    MCT_PROF_LOG_END();
    return ret;
  }

  MCT_PROF_LOG_END();

  return TRUE;
}

/** isp_module_get_session_data: get session data
 *
 *  @module: MCT module handle
 *  @get_buf: get buffer handle where the data would be updated
 *  @sessionid: session id for which session data shall be
 *            applied
 *
 *  This function fetches the session realted data that has per frame
 *  contorl parameters
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean isp_module_get_session_data(mct_module_t *module,
  void *get_buf, unsigned int session_id)
{
  mct_pipeline_session_data_t *session_data;
  isp_t                       *isp = NULL;
  boolean                      rc = FALSE;
  isp_hw_id_t                  res_isp = 0;
  isp_resource_t              *isp_resource;
  isp_resource_info_t         *res_info = NULL;
  isp_pipeline_t              *isp_pipeline = NULL;
  uint8_t                      available_vfe = 0;
  boolean                      ret = TRUE;
  mct_pipeline_cap_t           cap_buf;
  uint8_t                      *submod_enable = NULL;
  isp_hw_params_t              *common_hw_params = NULL;
  isp_hw_module_id_t            isp_hw = ISP_MOD_MAX_NUM;
  isp_hw_info_t                *hw_info_table = NULL;
  uint32_t                      i = 0;

  if (!module || !get_buf) {
    ISP_ERR("failed %p %p\n", module, get_buf);
    return FALSE;
  }

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }
  isp_resource = &isp->isp_resource;

  session_data = (mct_pipeline_session_data_t *)get_buf;

  session_data->max_pipeline_frame_applying_delay =
   MTYPE_MAX(session_data->max_pipeline_frame_applying_delay,
     isp->isp_resource.isp_resource_info
     [ISP_HW_0].isp_pipeline->pipeline_applying_delay);

   session_data->max_pipeline_meta_reporting_delay =
   MTYPE_MAX(session_data->max_pipeline_meta_reporting_delay, 0);

   session_data->min_isp_sw_frame_skip_retry = 3;

  /* Checking for Dual VFE */
  for (res_isp = 0; res_isp < isp_resource->num_isp &&
  res_isp < ISP_HW_MAX; res_isp++) {
  res_info = &isp_resource->isp_resource_info[res_isp];

  if (res_info->resource_alloc.state == ISP_RESOURCE_FREE)
    available_vfe++;
  }
  if (!available_vfe || ((isp_resource->num_session_opened > 1) &&
    (isp_resource->num_isp_preallocate >= ISP_HW_MAX)))
    session_data->dual_vfe_enabled = TRUE;

  /* zoom table need to populate to image lib use
     when boot up the isp cap already have the copy.
     however, when daemon crash and restart, the data may be gone,
     ISP repopulate whole cap if the cap data is gone*/
  if (session_data->zoom_ratio_tbl_cnt == 0) {
    memset(&cap_buf, 0, sizeof (mct_pipeline_cap_t));

    ret = isp_resource_query_mod(isp_resource, &cap_buf, session_id);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_resource_query_mod");
    }
    /* save capability in isp */
    isp->isp_cap = cap_buf.isp_cap;
  }

  session_data->zoom_ratio_tbl_cnt = isp->isp_cap.zoom_ratio_tbl_cnt;
  memcpy(session_data->zoom_ratio_tbl, isp->isp_cap.zoom_ratio_tbl,
    sizeof(isp->isp_cap.zoom_ratio_tbl));

    /* Get hw_id used by left VFE */

  isp_pipeline = isp->isp_resource.isp_resource_info[ISP_HW_0].isp_pipeline;

  if (!isp_pipeline) {
    ISP_ERR("failed; pipeline %p", isp_pipeline);
    return rc;
  }

  submod_enable = isp_pipeline->submod_enable;
  common_hw_params = isp_pipeline->common_hw_params;
  if (!submod_enable || !common_hw_params) {
    ISP_ERR("failed; sumod_enable %p common_hw_params %p", submod_enable,
      common_hw_params);
    return rc;
  }

  hw_info_table = common_hw_params->hw_info_table;
  if (!hw_info_table) {
    ISP_ERR("failed: %p", hw_info_table);
    return rc;
  }

  session_data->zzhdr_hw_available = FALSE;

  /* Prepare  mask */
  for (i = 0; i < common_hw_params->num_hw; i++) {
    isp_hw = hw_info_table[i].hw_module_id;
    if (isp_hw == ISP_MOD_HDR) {
        session_data->zzhdr_hw_available = TRUE;
        break;
      }
  }

  return TRUE;
}

/** isp_module_set_session_data: set session data
 *
 *  @module: sensor module handle
 *  @set_buf: set buffer handle that has session data
 *  @sessionid: session id for which session data shall be
 *            applied
 *
 *  This function provides session data that has per frame
 *  contorl parameters
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean isp_module_set_session_data(mct_module_t *module,
  void *set_buf, unsigned int session_id)
{
  mct_pipeline_session_data_t *session_data;
  boolean                      rc = FALSE, is_cached = FALSE;
  isp_t                       *isp = NULL;
  isp_resource_t              *isp_resource = NULL;
  uint32_t                      i = 0;

  if (!module || !set_buf) {
    ISP_ERR("failed %p %p\n", module, set_buf);
    return FALSE;
  }

  session_data = (mct_pipeline_session_data_t *)set_buf;

  rc = isp_set_pipeline_delay(module, session_id, session_data);

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }

  isp_resource = &isp->isp_resource;
  PTHREAD_MUTEX_LOCK(&isp_resource->mutex);
  is_cached = FALSE;
  for (i = 0; i < ISP_MAX_SESSION; i++) {
    /* Check whether this session id already exist */
    if ((isp_resource->session_info[i].is_valid == TRUE) &&
      (isp_resource->session_info[i].session_id == session_id)) {
      is_cached = TRUE;
      break;
    }
  }
  if (is_cached == FALSE) {
    /* Cache session param */
    for (i = 0; i < ISP_MAX_SESSION; i++) {
      /* Check whether this session id already exist */
      if (isp_resource->session_info[i].is_valid == FALSE) {
        isp_resource->session_info[i].is_valid = TRUE;
        isp_resource->session_info[i].session_id = session_id;
        isp_resource->session_info[i].max_op_pixel_clk =
          session_data->sensor_pix_clk;
        isp_resource->session_info[i].num_isp_preallocate = 0;
        isp_resource->session_info[i].format = session_data->sensor_format;
        break;
      }
    }
  }
  PTHREAD_MUTEX_UNLOCK(&isp_resource->mutex);
  return rc;
}

/** isp_module_overwrite_funcs:
 *
 *  @module: mct module handle
 *
 *  Fill function pointers to ISP specific functions
 **/
static void isp_module_overwrite_funcs(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }
  mct_module_set_set_mod_func(module, isp_module_set_mod);
  mct_module_set_query_mod_func(module, isp_module_query_mod);
  mct_module_set_start_session_func(module, isp_module_start_session);
  mct_module_set_stop_session_func(module, isp_module_stop_session);
  mct_module_set_session_data_func(module, isp_module_set_session_data);
  mct_module_get_session_data_func(module, isp_module_get_session_data);
}

/** module_isp_init:
 *
 *    @name: name of ISP module - "isp"
 *
 * Initializes new instance of ISP module
 *
 * 1) Create mct module for ISP
 * 2) Find ISP subdev's
 * 3) Power ON and read ISP version
 * 4) Based on ISP version, Initialize individual ISP hw modules
 * and corresponding port
 * 5) Store topology of hw modules based on ISP version in ISP
 * module's private
 * 6) Initialize stats parser
 * 7) Initialize zoom table
 *
 * Return mct module handle on success or NULL on failure
 **/
mct_module_t *module_isp_init(const char *name)
{
  boolean       ret = TRUE;
  mct_module_t *module = NULL;
  isp_t        *isp = NULL;
  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %p", name);
    return NULL;
  }

  if (strncmp(name, "isp", strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name, "isp");
    return NULL;
  }

  /* Create MCT module for ISP */
  module = mct_module_create(name);
  if (!module) {
    ISP_ERR("failed: mct_module_create");
    return NULL;
  }

  isp = (isp_t *)CAM_MALLOC(sizeof(*isp));
  if (!isp) {
    ISP_ERR("failed: no mem");
    return FALSE;
  }
  memset(isp, 0, sizeof(*isp));

  /* Initialize isp pipeline */
  ret = isp_resource_init(&isp->isp_resource);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_gen_hws_caps");
    goto ERROR;
  }

  /* Set isp private data */
  MCT_OBJECT_PRIVATE(module) = (void *)isp;

  /* Create ports */
  ret = isp_port_create(module);
  if (ret == FALSE) {
    ISP_ERR("failed: port_isp_create");
    goto ERROR;
  }

  /* Set log level */
  ret = isp_util_setloglevel("common", ISP_LOG_COMMON);
  if (ret == FALSE) {
    ISP_INFO("failed: unable to set log level");
  }

  /* Overwrite module functions */
  isp_module_overwrite_funcs(module);

  ISP_INFO("module %p", module);
  return module;

ERROR:
  ISP_ERR("failed: module_isp_init");
  mct_module_destroy(module);
  return NULL;
}

/** module_isp_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
void module_isp_deinit(mct_module_t *module)
{
  isp_t *isp = NULL;

  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return;
  }

  /* Destroy ISP pipeline */
  isp_resource_destroy(&isp->isp_resource);

  /* Delete source and sink ports */
  isp_port_delete_ports(module);

  MCT_OBJECT_PRIVATE(module) = NULL;

  /* Destroy mct module */
  mct_module_destroy(module);
}
