/**********************************************************************
*  Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "module_edge_smooth.h"
#include "chromatix_metadata.h"

/**
 * default width/height to be used
 */
#define MAX_DIM_W 5400
#define MAX_DIM_H 4100

/**
 *  Static functions
 **/
static void module_edge_smooth_send_prev_stage_done(imgbase_client_t *p_client);

/** g_caps:
 *
 *  Set the capabilities for edge_smooth module
 **/
static img_caps_t g_caps = {
  .num_input = 1,
  .num_output = 0,
  .num_meta = 1,
  .inplace_algo = 1,
  .num_release_buf = 0,
  .num_hold_outbuf = 1,
};

/**
 * Function: edge_smooth_dump_pproc_meta
 *
 * Description: function called to dump EA pproc metadata
 *
 * Arguments:
 *   @p_es_client: pointer to the edge client
 *
 * Return values:
 *   none
 *
 * Notes: If OEM chromatix is not present, this function will be no ops
 **/
static void edge_smooth_dump_pproc_meta(img_edge_client_t *p_es_client)
{
#ifdef _USE_OEM_CHROMATIX_
  pproc_meta_data_dump_t meta_dump;
  edge_smooth_info_t *p_ea_dump;
  edge_smooth_cfg_t *p_config;
  imgbase_client_t *p_client = p_es_client->p_client;
  pproc_meta_data_t *p_meta;
  isp_buf_divert_t *p_buf_div;
  imgbase_stream_t *p_stream;
  int32_t str_idx;
  meta_data_container *p_md_container;
  pproc_meta_entry_t *p_entry;

  /* dump only if the meta is valid */
  p_buf_div = p_client->p_current_buf_div;
  p_md_container = (meta_data_container *)p_buf_div->meta_data;
  IDBG_INFO("%s:%d] dump meta %p", __func__, __LINE__,
    p_md_container);

  if ((p_md_container) && (p_md_container->pproc_meta_data)) {
    /* validate stream */
    str_idx = module_imgbase_find_stream_by_identity(p_client,
      p_client->divert_identity);
    if (str_idx < 0) {
      IDBG_ERROR("%s:%d] Error Stream index", __func__, __LINE__);
      return;
    }
    p_stream = &p_client->stream[str_idx];

    /* dump is deleted in pproc. So the memory needs to be allocated in heap
       need to remove this once pproc is fixed */
    p_ea_dump = calloc(1, sizeof(edge_smooth_cfg_t));
    if (!p_ea_dump) {
      IDBG_ERROR("%s:%d] cannot allocate meta", __func__, __LINE__);
      return;
    }
    p_meta = p_md_container->pproc_meta_data;
    p_entry = &p_meta->entry[PPROC_META_DATA_EA_IDX];
    p_meta->header.tuning_size[PPROC_META_DATA_EA_IDX] = sizeof(*p_ea_dump);
    p_entry->dump_type = PPROC_META_DATA_EA;
    p_entry->pproc_meta_dump = p_ea_dump;
    p_entry->gain = p_es_client->cur_gain;
    p_entry->lux_idx = p_es_client->cur_lux_idx;
    p_entry->len = sizeof(*p_ea_dump);
    p_entry->component_revision_no = 0;
    p_entry->start_addr = 0;
    p_config = &p_es_client->p_session_data->es_config.config;

    /* need field by field copy */
    p_ea_dump->enable             = TRUE; /* Todo: update when hysteresis is added */
    p_ea_dump->flat_thr           = p_config->flat_thr;
    p_ea_dump->texture_thr        = p_config->texture_thr;
    p_ea_dump->similarity_thr     = p_config->similarity_thr;
    p_ea_dump->vStrength          = p_config->vStrength;
    p_ea_dump->zStrength          = p_config->zStrength;
    p_ea_dump->nStrength          = p_config->nStrength;
    p_ea_dump->hStrength          = p_config->hStrength;
    p_ea_dump->layer_2_flatThr    = p_config->layer_2_flatThr;
    p_ea_dump->layer_2_textureThr = p_config->layer_2_textureThr;

    /* update meta dump */
    meta_dump.frame_id  = p_buf_div->buffer.sequence;
    meta_dump.meta_data = p_buf_div->meta_data;

    /* send event */
    mod_imgbase_send_event(p_buf_div->identity, TRUE,
      MCT_EVENT_MODULE_PPROC_DUMP_METADATA, meta_dump);
  }
#endif
}

/**
 * Function: module_edge_smooth_handle_frame_skip
 *
 * Description: function to handle frameskip
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_edge_smooth_handle_frame_skip(imgbase_client_t *p_client)
{
  img_edge_client_t *p_es_client = (img_edge_client_t *)p_client->p_private_data;
  imgbase_session_data_t *p_session_data =  NULL;
  module_imgbase_t *p_mod = NULL;
  p_mod = p_client->p_mod;
  p_session_data = IMGBASE_SSP(p_mod, p_client->session_client);
  if (p_session_data == NULL) {
    return IMG_ERR_GENERAL;
  }
  edge_smooth_dump_pproc_meta(p_es_client);
  IDBG_ERROR("%s:%d] EA skipped %f", __func__, __LINE__,
    p_session_data->session_meta.aec_info.lux_idx);
  module_edge_smooth_send_prev_stage_done(p_client);
  return IMG_SUCCESS;
}

/**
 * Function: module_edge_smooth_prealloc_bufs
 *
 * Description: function called to pre-allocate buffers
 *
 * Arguments:
 *   @p_appdata - pointer to base module
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int32_t module_edge_smooth_prealloc_bufs(void *p_appdata)
{
  int32_t rc = IMG_SUCCESS;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_appdata;
  img_edge_module_t *p_edgemod = (img_edge_module_t *)p_mod->mod_private;
  img_preload_param_t preload_param;
  IDBG_MED("%s:%d] E ", __func__, __LINE__);
  preload_param.max_dim = p_edgemod->max_dim;

  rc = IMG_CORE_PRELOAD(&p_mod->core_ops, &preload_param);
  if (IMG_ERROR(rc)) {
    p_edgemod->last_error = rc;
    IDBG_ERROR("%s:%d] IMG_COMP_MEM_ALLOC failed", __func__, __LINE__);
  } else {
    p_edgemod->prealloc_bufs = true;
  }

  IDBG_MED("%s:%d] X rc %d", __func__, __LINE__, rc);
  return rc;
}

/**
 * Function: module_edge_smooth_preload
 *
 * Description: function called for edge smooth preload
 *
 * Arguments:
 *   @p_appdata - pointer to base module
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
__unused int32_t module_edge_smooth_preload(module_imgbase_t *p_mod)
{
  img_edge_module_t *p_edgemod = (img_edge_module_t *)p_mod->mod_private;
  img_core_type_t thread_affinity[1] = {IMG_CORE_ARM};
  img_thread_job_params_t thread_job;
  uint32_t job_id;
  int32_t rc = IMG_SUCCESS;

  IDBG_HIGH("%s:%d] dim %dx%d", __func__, __LINE__,
    p_edgemod->max_dim.width,
    p_edgemod->max_dim.height);

  if (!p_edgemod->max_dim.width || !p_edgemod->max_dim.height) {
    /* use default dimensions */
    IDBG_ERROR("%s:%d] invalid dim %dx%d in query caps, use default",
      __func__, __LINE__,
      p_edgemod->max_dim.width,
      p_edgemod->max_dim.height);
    p_edgemod->max_dim.width = MAX_DIM_W;
    p_edgemod->max_dim.height = MAX_DIM_H;
    p_edgemod->max_dim.stride = IMG_PAD_TO_X(MAX_DIM_W, 128);
    p_edgemod->max_dim.scanline = IMG_PAD_TO_X(MAX_DIM_H, 128);
  }

  /* reserve thread */
  p_edgemod->client_id =
    img_thread_mgr_reserve_threads(1, thread_affinity);

  if (!p_edgemod->client_id) {
    IDBG_ERROR("%s:%d] Error reserve thread ", __func__, __LINE__);
    rc = IMG_ERR_GENERAL;
    goto end;
  }
  thread_job.client_id = p_edgemod->client_id;
  thread_job.core_affinity = IMG_CORE_ARM;
  thread_job.delete_on_completion = TRUE;
  thread_job.execute = module_edge_smooth_prealloc_bufs;
  thread_job.dep_job_count = 0;
  thread_job.args = p_mod;
  thread_job.dep_job_ids = NULL;
  job_id = img_thread_mgr_schedule_job(&thread_job);
  if (job_id) {
    IDBG_MED("%s:%d] scheduled job id %x client %x", __func__, __LINE__,
      job_id, p_edgemod->client_id);
  } else {
    IDBG_ERROR("%s:%d] Error cannot schedule job ", __func__, __LINE__);
    rc = IMG_ERR_GENERAL;
  }

end:
  return rc;
}

/**
 * Function: module_edge_smooth_session_start
 *
 * Description: function called after session start
 *
 * Arguments:
 *   @p_imgbasemod - IMGLIB_BASE module
 *   @sessionid: session ID
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_edge_smooth_session_start(void *p_imgbasemod, uint32_t sessionid)
{
  img_sd_client_data_t client_data;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_imgbasemod;
  img_edge_module_t *p_edgemod = (img_edge_module_t *)p_mod->mod_private;
  img_edge_session_data_t *p_session_data;
  uint32_t i;

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  bool edge_enabled = module_imglib_common_get_prop(
    "persist.vendor.camera.imglib.oemfeat1", "0");
  if (!edge_enabled) {
    IDBG_HIGH("OEMFEAT1 Disabled ");
    return IMG_SUCCESS;
  }

  /* register the client */
  p_session_data = &p_edgemod->session_data[sessionid - 1];
  client_data.p_appdata = p_session_data;
  client_data.p_detect = module_edge_config_handle_hysteresis;
  p_session_data->sd_client_id =
    img_scene_mgr_register(get_scene_mgr(), &client_data);
  if (!p_session_data->sd_client_id) {
    /* non fatal */
    IDBG_ERROR("%s:%d] register error %d", __func__, __LINE__,
      p_session_data->sd_client_id);
  }
  p_session_data->ea_state = IMG_ON;

  p_edgemod->force_enable = module_imglib_common_get_prop(
    "persist.vendor.camera.imglib.force_ea", "0");

  p_edgemod->bypass = module_imglib_common_get_prop(
    "persist.vendor.camera.imglib.ea_bypass", "0");

  /* fill preload params incase of daemon restart */
  if (!p_edgemod->max_dim.width || !p_edgemod->max_dim.height) {
    /* use default dimensions */
    IDBG_ERROR("%s:%d] invalid dim %dx%d in query caps, use default",
      __func__, __LINE__,
      p_edgemod->max_dim.width,
      p_edgemod->max_dim.height);
    p_edgemod->max_dim.width = MAX_DIM_W;
    p_edgemod->max_dim.height = MAX_DIM_H;
    p_edgemod->max_dim.stride = IMG_PAD_TO_X(MAX_DIM_W, 128);
    p_edgemod->max_dim.scanline = IMG_PAD_TO_X(MAX_DIM_H, 128);
  }

  for (i = 0; i < IMGLIB_ARRAY_SIZE(p_mod->session_data_l); i++) {
    p_mod->session_data_l[i].preload_params.param.max_dim = p_edgemod->max_dim;
  }

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: module_edge_smooth_session_stop
 *
 * Description: function called after session start
 *
 * Arguments:
 *   @p_imgbasemod - IMGLIB_BASE module
 *   @sessionid: session ID
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_edge_smooth_session_stop(void *p_imgbasemod, uint32_t sessionid)
{
  int32_t rc;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_imgbasemod;
  img_edge_module_t *p_edgemod = p_mod->mod_private;
  img_edge_session_data_t *p_session_data;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  /*unregister the client*/
  p_session_data = &p_edgemod->session_data[sessionid - 1];
  rc = img_scene_mgr_unregister(get_scene_mgr(), p_session_data->sd_client_id);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] img_scene_mgr_unregister failed %d",
      __func__, __LINE__, rc);
    /* non fatal - pass through */
  }

  IDBG_MED("%s:%d] rc %d", __func__, __LINE__, rc);
  return rc;
}

/**
 * Function: module_edge_smooth_client_created
 *
 * Description: function called after client creation
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_edge_smooth_client_created(imgbase_client_t *p_client)
{
  int32_t rc = IMG_SUCCESS;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  img_edge_module_t *p_edgemod = (img_edge_module_t *)p_mod->mod_private;
  img_edge_client_t *p_es_client;

  p_es_client = calloc(1, sizeof(img_edge_client_t));
  IDBG_MED("%s:%d] E", __func__, __LINE__);
  if (!p_es_client) {
    IDBG_ERROR("%s:%d] Failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  p_es_client->p_session_data =
    &p_edgemod->session_data[p_client->session_id - 1];
  module_edge_config_set_default_params(
    &p_es_client->p_session_data->es_config);
  p_client->p_private_data = p_es_client;
  p_es_client->p_client = p_client;

  p_edgemod->force_enable = module_imglib_common_get_prop(
    "persist.vendor.camera.imglib.force_ea", "0");

  p_edgemod->bypass = module_imglib_common_get_prop(
    "persist.vendor.camera.imglib.ea_bypass", "0");

  if (p_edgemod->bypass) {
    p_es_client->p_session_data->ea_state = IMG_OFF;
    p_client->processing_disabled = true;
    p_client->lock_dyn_update = true;
  } else if (p_edgemod->force_enable) {
    p_es_client->p_session_data->ea_state = IMG_ON;
    p_client->processing_disabled = false;
    p_client->lock_dyn_update = true;
  } else { /* regular case */
    p_client->processing_disabled =
      (p_es_client->p_session_data->ea_state == IMG_OFF);
  }
  IDBG_HIGH("%s:%d] Edge processing state %d force %u bypass %u",
    __func__, __LINE__,
    p_es_client->p_session_data->ea_state,
    p_edgemod->force_enable,
    p_edgemod->bypass);

  /* client ID */
  p_client->comp_init_params.client_id = p_mod->th_client_id;
  return rc;
}

/**
 * Function: module_edge_smooth_client_destroy
 *
 * Description: function called before client is destroyed
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_edge_smooth_client_destroy(imgbase_client_t *p_client)
{
  IDBG_MED("%s %d: E", __func__, __LINE__);
  if (p_client->p_private_data) {
    free(p_client->p_private_data);
    p_client->p_private_data = NULL;
  }
  return IMG_SUCCESS;
}

/**
 * Function: module_edge_smooth_deinit
 *
 * Description: This function is used to deinit edge_smooth
 *               module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_edge_smooth_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}

/**
 * Function: module_edge_smooth_query_mod
 *
 * Description: This function is used to query edge_smooth
 *               caps
 *
 * Arguments:
 *   @p_mct_cap - capababilities
 *   @p_basemod - pointer to the base module
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_edge_smooth_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* p_basemod,unsigned int sessionid)
{
  mct_pipeline_imaging_cap_t *p_img_caps;
  mct_pipeline_sensor_cap_t  *p_sensor_caps;
  mct_pipeline_pp_cap_t *p_pproc_caps;
  mct_pipeline_common_cap_t *p_common_caps;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_basemod;
  img_edge_module_t *p_edgemod = (img_edge_module_t *)p_mod->mod_private;
  uint32_t width = 0, height = 0;
  uint32_t width_pad = 0, height_pad = 0, plane_pad = 0;
#ifdef _USE_SENSOR_TBL_
  uint32_t i
#endif

  IMG_UNUSED(sessionid);
  if (!p_mct_cap || !p_edgemod) {
    IDBG_ERROR("%s:%d] Error %p %p", __func__, __LINE__,
      p_mct_cap, p_edgemod);
    return FALSE;
  }
  p_img_caps = &p_mct_cap->imaging_cap;
  p_sensor_caps = &p_mct_cap->sensor_cap;
  p_pproc_caps = &p_mct_cap->pp_cap;
  p_common_caps = &p_mct_cap->common_cap;

#ifdef _USE_SENSOR_TBL_
  /* fill the max sensor dimension */
  for (i = 0; i < p_sensor_caps->dim_fps_table_count; i++) {
    if (p_sensor_caps->dim_fps_table[i].fps.max_fps >= 29.0 &&
      p_sensor_caps->dim_fps_table[i].fps.max_fps < 60.0) {
      if (width < (uint32_t)p_sensor_caps->dim_fps_table[i].dim.width) {
        width = p_sensor_caps->dim_fps_table[i].dim.width;
      }
      if (height < (uint32_t)p_sensor_caps->dim_fps_table[i].dim.height) {
        height = p_sensor_caps->dim_fps_table[i].dim.height;
      }
    }
  }
#else
  width = p_sensor_caps->pixel_array_size.width;
  height = p_sensor_caps->pixel_array_size.height;
#endif

  /* override with the largest dimension */
  if (p_edgemod->max_dim.width < width)
    p_edgemod->max_dim.width = p_edgemod->max_dim.stride =
      IMG_PAD_TO_X(width, 128);

  if (p_edgemod->max_dim.height < height)
    p_edgemod->max_dim.height = p_edgemod->max_dim.scanline =
      IMG_PAD_TO_X(height, 128);

  IDBG_HIGH("%s:%d] dim %dx%d", __func__, __LINE__,
    p_edgemod->max_dim.width,
    p_edgemod->max_dim.height);

  width_pad = module_imglib_common_get_prop(
    "persist.vendor.camera.imglib.ea.wp", "64");
  height_pad = module_imglib_common_get_prop(
    "persist.vendor.camera.imglib.ea.hp", "64");
  plane_pad = module_imglib_common_get_prop(
    "persist.vendor.camera.imglib.ea.pp", "64");

  p_pproc_caps->width_padding =
    mct_util_calculate_lcm(p_pproc_caps->width_padding, width_pad);
  p_pproc_caps->height_padding =
    mct_util_calculate_lcm(p_pproc_caps->height_padding, height_pad);
  p_common_caps->plane_padding =
    mct_util_calculate_lcm(p_common_caps->plane_padding, plane_pad);

  return TRUE;
}

/**
 * Function: module_edge_smooth_send_prev_stage_done
 *
 * Description: This function is to indicate that the previous
 *             stage is completed
 *
 * Arguments:
 *   @p_client: pointer to imgbase client
 *
 * Return values:
 *     None
 *
 * Notes: none
 **/
void module_edge_smooth_send_prev_stage_done(imgbase_client_t *p_client)
{
  cam_cac_info_t cac_info;
  mct_module_t *p_mct_mod =
    MCT_MODULE_CAST(MCT_PORT_PARENT(p_client->stream[0].p_sinkport)->data);
  cac_info.buf_idx = p_client->p_current_buf_div->buffer.index;
  cac_info.frame_id = p_client->p_current_buf_div->buffer.sequence;
  IDBG_HIGH("%s:%d] frame %d buf %d", __func__, __LINE__, cac_info.buf_idx,
    cac_info.frame_id);
  module_imgbase_post_bus_msg(p_mct_mod, p_client->session_id,
    MCT_BUS_MSG_CAC_STAGE_DONE, &cac_info, sizeof(cac_info));
}

/**
 * Function: module_edge_smooth_update_meta
 *
 * Description: This function is called when the base
 *   module updates the metadata
 *
 * Arguments:
 *   @p_client: pointer to imgbase client
 *   @p_meta: pointer to the image meta
 *
 * Return values:
 *     error values
 *
 * Notes: none
 **/
int32_t module_edge_smooth_update_meta(imgbase_client_t *p_client,
  img_meta_t *p_meta)
{
  int rc = IMG_SUCCESS;
  img_edge_client_t *p_es_client = (img_edge_client_t *)p_client->p_private_data;
  img_edge_smooth_meta_t *p_es_config =
    &p_es_client->p_session_data->es_config;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  img_edge_module_t *p_edgemod = (img_edge_module_t *)p_mod->mod_private;
  img_opaque_data_set_t opaque_data_set;

  if (p_edgemod->last_error) {
    IDBG_ERROR("%s:%d] Error last_error %d", __func__, __LINE__,
      p_edgemod->last_error);
    rc = p_edgemod->last_error;
    goto end;
  }

  /* send previous stage done indication */
  module_edge_smooth_send_prev_stage_done(p_client);

  /* interpolate and get data */
  module_edge_config_update_from_chromatix(p_es_client, p_es_config);

  /* set meta now */
  opaque_data_set.data_size = sizeof(*p_es_config);
  opaque_data_set.p_data = (uint8_t *)p_es_config;
  rc = img_set_meta(p_meta, IMG_META_OPAQUE_DATA, &opaque_data_set);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Error rc %d", rc);
    goto end;
  }

  /* dump pproc meta */
  edge_smooth_dump_pproc_meta(p_es_client);

  if (!p_edgemod->prealloc_bufs && p_client->thread_job.client_id) {
    rc = p_client->thread_ops.wait_for_completion_by_clientid(
      p_client->thread_job.client_id, 1000);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] Error rc %d client %u", __func__, __LINE__, rc,
        p_client->thread_job.client_id);
      goto end;
    }
  }

end:
  return rc;
}

/**
 * Function: module_edge_smooth_handle_chromatix
 *
 * Description: This function handles chromatix event
 *
 * Arguments:
 *   p_mod_event - pointer of module event
 *   p_client - pointer to imgbase client
 *   p_core_ops - pointer to imgbase module ops
 *
 * Return values:
 *     TRUE/FALSE
 *
 * Notes: none
 **/
boolean module_edge_smooth_handle_chromatix( uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled)
{
  img_edge_client_t *p_es_client =
    (img_edge_client_t *)p_client->p_private_data;
  modulesChromatix_t *p_chromatix_param =
    (modulesChromatix_t *)p_mod_event->module_event_data;

  if (!p_chromatix_param) {
    IDBG_ERROR("%s:%d] Error input %p", __func__, __LINE__, p_chromatix_param);
  } else {
    p_es_client->p_session_data->p_chromatix =
      p_chromatix_param->chromatixPostProcPtr;
  }
  IDBG_MED("%s:%d] chromatix %p", __func__, __LINE__,
    p_es_client->p_session_data->p_chromatix);

  return TRUE;
}

/**
 * Function: module_edge_smooth_client_init_params
 *
 * Description: This function updates any init_params if needed
 *
 * Arguments:
 *   p_client - pointer to imgbase client
 *   p_params -params that need update
 *
 * Return values:
 *     TRUE/FALSE
 *
 * Notes: none
 **/
boolean module_edge_smooth_client_init_params(
  imgbase_client_t *p_client,
  img_init_params_t *p_params)
{
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;

  if(p_params){
    p_params->client_id = p_mod->th_client_id;
    IDBG_MED("client_id %d", p_params->client_id);
  }

  return TRUE;
}

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_query_mod = module_edge_smooth_query_mod,
  .imgbase_client_init_params = module_edge_smooth_client_init_params,
  .imgbase_client_update_meta = module_edge_smooth_update_meta,
  .imgbase_session_stop = module_edge_smooth_session_stop,
  .imgbase_client_created = module_edge_smooth_client_created,
  .imgbase_client_destroy = module_edge_smooth_client_destroy,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_SET_CHROMATIX_PTR] =
    module_edge_smooth_handle_chromatix,
  .imgbase_session_start = module_edge_smooth_session_start,
  .imgbase_client_handle_frame_skip = module_edge_smooth_handle_frame_skip,
  .exec_mode = IMG_EXECUTION_SW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .force_cache_op = FALSE,
};

/** module_edge_smooth_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the edge_smooth
 * module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_edge_smooth_init(const char *name)
{
  int32_t is_edgesmooth_enabled = 0;
  is_edgesmooth_enabled =  module_imglib_common_get_prop
    ("persist.vendor.camera.imglib.oemfeat1", "0");

  IDBG_HIGH("Is edgesmooth enabled %d", is_edgesmooth_enabled);

  if (!is_edgesmooth_enabled) {
    IDBG_HIGH("edgesmooth disabled %d, init skipped",
      is_edgesmooth_enabled);
    return NULL;
  }

  img_edge_module_t *p_edgemod = calloc(1, sizeof(img_edge_module_t));
  if (!p_edgemod) {
    IDBG_ERROR("%s:%d] Failed", __func__, __LINE__);
    return NULL;
  }

  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    p_edgemod,
    &g_caps,
    "libmmcamera_edgesmooth_lib.so",
    CAM_OEM_FEATURE_1,
    &g_params);
}

#ifndef __IMGLIB_USE_OEM_FEAT1_STUB__
/** module_oem_feat1_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: Stub module init function for oem feat1
 *
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_oem_feat1_init(const char *name)
{
  return module_edge_smooth_init(name);
}

/**
 * Function: module_oem_feat1_deinit
 *
 * Description: Stub function to deinit oem feat1
 *
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_oem_feat1_deinit(mct_module_t *p_mct_mod)
{
  module_edge_smooth_deinit(p_mct_mod);
}
#endif //__IMGLIB_USE_OEM_FEAT1_STUB__
