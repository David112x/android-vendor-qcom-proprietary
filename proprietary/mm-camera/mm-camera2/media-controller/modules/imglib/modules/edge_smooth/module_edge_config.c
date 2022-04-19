/**********************************************************************
*  Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "module_edge_smooth.h"
#include "mgr/scene_detect_mgr.h"

/**
 * Function: module_edge_config_set_default_params
 *
 * Description: function called to set the default parameters
 *
 * Arguments:
 *   @p_config: image edge smooth meta
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_edge_config_set_default_params(img_edge_smooth_meta_t *p_config)
{
  IDBG_MED("%s:%d] ", __func__, __LINE__);

  p_config->config.flat_thr               =    8;
  p_config->config.texture_thr            =    8;
  p_config->config.similarity_thr         = 1382;
  p_config->config.hStrength              = 1024;
  p_config->config.vStrength              = 1024;
  p_config->config.nStrength              = 1024;
  p_config->config.zStrength              = 1024;
  p_config->config.layer_2_flatThr        =    4;
  p_config->config.layer_2_textureThr     =    8;
} /*module_edge_config_set_default_params*/

#ifdef _USE_OEM_CHROMATIX_
/**
 * Function: edge_config_interpolate
 *
 * Description: function to execute interpolation
 *
 * Arguments:
 *   @p_in: input tables for interpolation
 *   @int_ratio: interpolation ratio
 *   @p_out_config: output configuration
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void edge_config_interpolate(chromatix_EdgeAlignment_Core_type *p_in[],
  float int_ratio,
  edge_smooth_cfg_t *p_out_config)
{
  if (!p_in[1]) {
    int_ratio =  0.0f;
    p_in[1] = p_in[0];
  }
  p_out_config->flat_thr =
    round(IMG_LINEAR_INTERPOLATE((int32_t)p_in[0]->flatThr,
      (int32_t)p_in[1]->flatThr, int_ratio));
  p_out_config->texture_thr =
    round(IMG_LINEAR_INTERPOLATE((int32_t)p_in[0]->textureThr,
      (int32_t)p_in[1]->textureThr, int_ratio));
  p_out_config->similarity_thr =
    round(IMG_CONVERT_TO_Q(IMG_LINEAR_INTERPOLATE(p_in[0]->similarityThr,
      p_in[1]->similarityThr, int_ratio), 10));
  p_out_config->hStrength =
    round(IMG_CONVERT_TO_Q(IMG_LINEAR_INTERPOLATE(p_in[0]->hStrength,
      p_in[1]->hStrength, int_ratio), 10));
  p_out_config->vStrength =
    round(IMG_CONVERT_TO_Q(IMG_LINEAR_INTERPOLATE(p_in[0]->vStrength,
      p_in[1]->vStrength, int_ratio), 10));
  p_out_config->nStrength =
    round(IMG_CONVERT_TO_Q(IMG_LINEAR_INTERPOLATE(p_in[0]->nStrength,
      p_in[1]->nStrength, int_ratio), 10));
  p_out_config->zStrength =
    round(IMG_CONVERT_TO_Q(IMG_LINEAR_INTERPOLATE(p_in[0]->zStrength,
      p_in[1]->zStrength, int_ratio), 10));
  p_out_config->layer_2_flatThr =
    IMG_LINEAR_INTERPOLATE((int32_t)p_in[0]->layer_2_flatThr,
      (int32_t)p_in[1]->layer_2_flatThr, int_ratio);
  p_out_config->layer_2_textureThr =
    IMG_LINEAR_INTERPOLATE((int32_t)p_in[0]->layer_2_textureThr,
      (int32_t)p_in[1]->layer_2_textureThr, int_ratio);
} /*edge_config_interpolate*/

/**
 * Function: edge_config_get_region
 *
 * Description: function to find the region of interpolation
 *
 * Arguments:
 *   @p_edge_chromatix: edge alignment chromatix
 *   @in_trigger: input trigger value
 *   @p_ea_core: edge alignment core
 *   @p_ratio: interpolation ratio
 *   @nr_flag: noise reduction flag for OIS capture mode
 *
 * Return values:
 *   imaging errors
 *
 * Notes: none
 **/
static int32_t edge_config_get_region(
  chromatix_Edge_Alignment_type *p_edge_chromatix,
  float in_trigger,
  chromatix_EdgeAlignment_Core_type *p_ea_core[],
  float *p_ratio,
  uint32_t nr_flag)
{
  uint32_t i = 0;
  int32_t rc = IMG_SUCCESS;
  IDBG_MED("%s:%d] ", __func__, __LINE__);
  float s_trig, e_trig;
  tuning_control_type control = p_edge_chromatix->control_edgealign;
  uint32_t tbl_size = MAX_LIGHT_TYPES_FOR_SPATIAL;
  int32_t tbl_idx[2];
  uint32_t max_tbl_size = IMGLIB_ARRAY_SIZE(p_edge_chromatix->edgealign_data);

  if (tbl_size > max_tbl_size) {
    IDBG_ERROR("%s:%d] EA table size incorrect current %d max %d",
      __func__, __LINE__,
      tbl_size, max_tbl_size);
    return IMG_ERR_OUT_OF_BOUNDS;
  }

  if (nr_flag > (max_tbl_size - tbl_size)) {
    IDBG_ERROR("%s:%d] invalid NR mode %d tbl %u %u",
      __func__, __LINE__,
      nr_flag, max_tbl_size, tbl_size);
    return IMG_ERR_INVALID_INPUT;
  }

  /* set default interpolation ratio */
  *p_ratio = 1.0;

  /* check for OIS capture mode */
  if (nr_flag) {
    uint32_t ois_tbl_idx = tbl_size + nr_flag - 1;
    p_ea_core[0] = &p_edge_chromatix->edgealign_data[ois_tbl_idx];
    p_ea_core[1] = NULL;
    IDBG_HIGH("%s:%d] OIS Region start %d end -1", __func__, __LINE__,
      ois_tbl_idx);
    return IMG_SUCCESS;
  }

  /* set base values */
  p_ea_core[0] = p_ea_core[1] = NULL;

  for (i = 0; i < tbl_size - 1; i++) {
    chromatix_EdgeAlignment_Core_type *p_cur =
      &p_edge_chromatix->edgealign_data[i];
    chromatix_EdgeAlignment_Core_type *p_next =
      &p_edge_chromatix->edgealign_data[i+1];
    if (control) {
      s_trig = p_cur->edge_alginment_trigger.gain_start;
      e_trig = p_cur->edge_alginment_trigger.gain_end;
    } else {
      s_trig = p_cur->edge_alginment_trigger.lux_index_start;
      e_trig = p_cur->edge_alginment_trigger.lux_index_end;
    }
    if (IMG_F_EQUAL(s_trig, 0.0) || IMG_F_EQUAL(e_trig, 0.0) ||
      IMG_F_EQUAL(s_trig, e_trig) ) {
      IDBG_ERROR("%s:%d] Error invalid triggers %f %f", __func__, __LINE__,
        s_trig, e_trig);
      return IMG_ERR_GENERAL;
    }
    /* check the regions */
    if (in_trigger <= s_trig) {
      p_ea_core[0] = p_cur;
      p_ea_core[1] = NULL;
      tbl_idx[0] = i;
      tbl_idx[1] = -1;
      break;
    } else if ((in_trigger > s_trig) && (in_trigger < e_trig)) {
      p_ea_core[0] = p_cur;
      p_ea_core[1] = p_next;
      tbl_idx[0] = i;
      tbl_idx[1] = i + 1;
      *p_ratio = IMG_GET_INTERPOLATION_RATIO(in_trigger, s_trig, e_trig);
      break;
    }
  }

  if ((tbl_size - 1) == i) {
    /* use the last region */
    p_ea_core[0] = &p_edge_chromatix->edgealign_data[tbl_size - 1];
    p_ea_core[1] = NULL;
    tbl_idx[0] = tbl_size - 1;
    tbl_idx[1] = -1;
  }

  IDBG_HIGH("%s:%d] Region start %d end %d", __func__, __LINE__,
    tbl_idx[0], tbl_idx[1]);
  return rc;
} /*edge_config_get_region*/
#endif

/**
 * Function: module_edge_config_update_from_chromatix
 *
 * Description: function called to interpolate the parameters
 *
 * Arguments:
 *   @p_client: pointer to the edge client
 *   @p_config: image edge smooth meta
 *
 * Return values:
 *   none
 *
 * Notes: If OEM chromatix is not present, chromatix values are incorrect
 *        or invalid triggers are provided, the default configuration will
 *        be send with error log
 **/
void module_edge_config_update_from_chromatix(img_edge_client_t *p_client,
  img_edge_smooth_meta_t *p_config)
{
#ifdef _USE_OEM_CHROMATIX_
  chromatix_Edge_Alignment_type *p_edge_chromatix;
  float intp_ratio; /* interpolation ratio*/
  float in_trigger;
  chromatix_EdgeAlignment_Core_type *p_ea_core[2];
  imgbase_session_data_t *p_session_data =  NULL;
  imgbase_client_t p_base_client = p_client->p_client;
  p_session_data = IMGBASE_SSP(p_mod,
    p_base_client->session_id);
  if (!p_client->p_session_data->p_chromatix) {
    IDBG_ERROR("%s:%d] chromatix %p ", __func__, __LINE__,
      p_client->p_session_data->p_chromatix);
    goto error;
  }

  p_edge_chromatix =
    &p_client->p_session_data->p_chromatix->chromatix_Edge_Alignment_data;
  in_trigger = p_edge_chromatix->control_edgealign ?
    p_session_data->session_meta.aec_info.real_gain :p_session_data->session_meta.aec_info.lux_idx;

  if (IMG_F_EQUAL(0.0f, in_trigger)) {
    IDBG_ERROR("%s:%d] trigger %f", __func__, __LINE__, in_trigger);
    goto error;
  }

  IDBG_MED("%s:%d] control %d trigger %f", __func__, __LINE__,
    p_edge_chromatix->control_edgealign, in_trigger);

  if (IMG_ERROR(edge_config_get_region(p_edge_chromatix,
    in_trigger, p_ea_core, &intp_ratio, p_session_data->session_meta.aec_info.nr_flag))) {
    goto error;
  }

  /* interpolate the parameters */
  edge_config_interpolate(p_ea_core, intp_ratio, &p_config->config);
  /* success return */
  return;

#else
  IDBG_ERROR("%s:%d] Warning chromatix not enabled", __func__, __LINE__);
  IMG_UNUSED(p_client);
  goto error;
#endif

error:
  IDBG_ERROR("%s:%d] Error use default params", __func__, __LINE__);
  module_edge_config_set_default_params(p_config);
} /*module_edge_config_update_from_chromatix*/


/**
 * Function: module_edge_config_handle_hysteresis
 *
 * Description: function called by scene detect manager to
 *       handle hysteresis
 *
 * Arguments:
 *  @p_userdata: pointer to the userdata
 *  @session_id: session id
 *  @p_scenelist: scene data list
 *  @p_dyn_data: output dynamic data set by the client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_edge_config_handle_hysteresis(void *p_userdata,
  uint32_t session_id __unused,
  img_scene_detect_list_t *p_scenelist,
  cam_dyn_img_data_t *p_dyn_data)
{
  int32_t rc = IMG_SUCCESS;

  if (!p_userdata || !p_scenelist) {
    IDBG_ERROR("%s:%d] Error input %p %p", __func__, __LINE__,
      p_userdata, p_scenelist);
    rc = IMG_ERR_INVALID_INPUT;
    goto end;
  }

#ifdef _USE_OEM_CHROMATIX_
  img_edge_session_data_t *p_session_data;
  float trigger[2];
  float cur_trigger;
  int32_t new_ea_state;

  p_session_data = (img_edge_session_data_t *)p_userdata;
  chromatix_Edge_Alignment_type *p_ea_data;
  /* fetch pp chromatix ptr */
  chromatix_sw_postproc_type *p_swpp_chromatix =
    (chromatix_sw_postproc_type *)p_scenelist->chromatix_param.chromatixPostProcPtr;

  if (!p_swpp_chromatix) {
    IDBG_MED("%s:%d] Error chromatix %p", __func__, __LINE__, p_swpp_chromatix);
    rc = IMG_ERR_BUSY;
    goto end;
  }

  p_ea_data = &p_swpp_chromatix->chromatix_Edge_Alignment_data;

  if (p_ea_data->control_edgealign) {
    trigger[0] = p_ea_data->edge_alignment_hysteresis_trigger.gain_start;
    trigger[1] = p_ea_data->edge_alignment_hysteresis_trigger.gain_end;
    cur_trigger = p_scenelist->sd_data[p_scenelist->cur_idx].gain;
  } else {
    trigger[0] = p_ea_data->edge_alignment_hysteresis_trigger.lux_index_start;
    trigger[1] = p_ea_data->edge_alignment_hysteresis_trigger.lux_index_end;
    cur_trigger = p_scenelist->sd_data[p_scenelist->cur_idx].lux_idx;
  }

  if (IMG_F_EQUAL(trigger[0], 0.0f) || IMG_F_EQUAL(trigger[1], 0.0f) ||
    IMG_F_EQUAL(cur_trigger, 0.0f)) {
    IDBG_ERROR("%s:%d] Error use default params %f %f %f",
      __func__, __LINE__, trigger[0], trigger[1], cur_trigger);
    rc = IMG_ERR_GENERAL;
    goto end;
  }
  /* do hysteresis*/
  new_ea_state =
    IMG_HYSTERESIS(cur_trigger, trigger[0], trigger[1],
      p_session_data->ea_state, IMG_ON, IMG_OFF);
  IDBG_LOW("%s:%d] trigger %f %f %f ea_state new %d old %d",
    __func__, __LINE__,
    cur_trigger, trigger[0], trigger[1], new_ea_state,
    p_session_data->ea_state);

  /* success update feature mask */
  if (IMG_ON == new_ea_state) {
    p_dyn_data->dyn_feature_mask |= CAM_OEM_FEATURE_1;
  }

  p_session_data->ea_state = new_ea_state;

#else
  IDBG_HIGH("%s:%d] Error cannot use hysteresis", __func__, __LINE__);

  if (module_imglib_common_get_prop("persist.vendor.camera.imglib.force_ea", "0")) {
    IDBG_HIGH("edgesmooth Force Enabled");
    p_dyn_data->dyn_feature_mask |= CAM_OEM_FEATURE_1;
  }
  rc = IMG_ERR_NOT_SUPPORTED;
#endif

end:
  return rc;
} /*module_edge_config_handle_hysteresis*/
