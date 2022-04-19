/* aec_biz.c
 *
 * Copyright (c) 2014-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


#include "aec_biz.h"

#define LED_CAL_FILE_PATH QCAMERA_DUMP_FRM_LOCATION"led_cal.cfg"

static char *aec_biz_get_param_strings[AEC_GET_PARAM_MAX+1] = {
  AEC_GET_PARAM_ENUM_LIST(AEC_PARAM_GENERATE_STRING)
};
static char *aec_biz_set_param_strings[AEC_SET_PARAM_MAX+1] = {
  AEC_SET_PARAM_ENUM_LIST(AEC_PARAM_GENERATE_STRING)
};

/** aec_biz_get_param_string
 *    @eventId: aec get param
 *
 *  Return char *
 **/

inline static char * aec_biz_get_param_string(int eventId)
{
  return aec_biz_get_param_strings[eventId < AEC_GET_PARAM_MAX ?
    eventId : AEC_GET_PARAM_MAX];
}

/** aec_biz_set_param_string
 *    @eventId: aec set param
 *
 *  Return char *
 **/
inline static char * aec_biz_set_param_string(int eventId)
{
  return aec_biz_set_param_strings[eventId < AEC_SET_PARAM_MAX ?
    eventId : AEC_SET_PARAM_MAX];
}

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION >= 0x0310)
static void aec_biz_copy_dual_led_setting(aec_led_combination_type *src, LED_setting_type *dst)
{
  dst->LED1_setting = src->LED1_setting;
  dst->LED2_setting = src->LED2_setting;
  dst->bg_ratio = src->bg_ratio;
  dst->rg_ratio = src->rg_ratio;
  dst->flux = src->flux;
  dst->first_entry_ratio = src->first_entry_ratio;
  dst->last_entry_ratio = src->last_entry_ratio;
}
#endif

static boolean aec_add_core_entry(int entry_type, uint32 entry_size,
  const void* entry_value, aec_core_input_info_type* input_info)
{
  if (entry_type >= AEC_INPUT_PARAM_MAX ||
    entry_size > sizeof(aec_input_payload_type)) {
    AEC_ERR("invalid input: entry_type: %d, entry_size: %d",
      entry_type, entry_size);
    return FALSE;
  }

  PARAM_POSITION_LINK_RESET(input_info, entry_type);
  STATS_MEMCPY(POINTER_OF(entry_type, input_info), sizeof(aec_input_payload_type),
    entry_value, entry_size);

  return TRUE;
}

boolean aec_biz_translate_coord_fov2camif(aec_biz_t *aec,
  uint16_t *x, uint16_t *y)
{
  int  roi_x, roi_y;
  if(aec->stream_crop_info.vfe_out_width == 0 ||
    aec->stream_crop_info.vfe_out_height == 0)
    return FALSE;
  if(aec->preview_width == 0 || aec->preview_height == 0)
  {
    aec->preview_width = aec->stream_crop_info.pp_crop_out_x + 1;
    aec->preview_height = aec->stream_crop_info.pp_crop_out_y + 1;
  }
  AEC_LOW("aec->preview_width=%d, aec->preview_height=%d,\
    aec->stream_crop_info.vfe_out_width=%d,aec->stream_crop_info.vfe_out_height=%d",
    aec->preview_width,aec->preview_height,
    aec->stream_crop_info.vfe_out_width, aec->stream_crop_info.vfe_out_height);
  if (! aec->stream_crop_info.pp_crop_out_x ||
    !aec->stream_crop_info.pp_crop_out_y) {
   aec->stream_crop_info.pp_x = 0;
   aec->stream_crop_info.pp_y = 0;
   aec->stream_crop_info.pp_crop_out_x = aec->preview_width;
   aec->stream_crop_info.pp_crop_out_y = aec->preview_height;
  }

  /* Reverse calculation to cpp output */
  roi_x = *x  * aec->stream_crop_info.pp_crop_out_x /aec->preview_width ;
  roi_y = *y *  aec->stream_crop_info.pp_crop_out_y/ aec->preview_height;
  roi_x += aec->stream_crop_info.pp_x;
  roi_y += aec->stream_crop_info.pp_y;
  /* Reverse calculation for vfe output */
  roi_x = roi_x * aec->stream_crop_info.vfe_map_width /  aec->stream_crop_info.vfe_out_width ;
  roi_y = roi_y *  aec->stream_crop_info.vfe_map_height /  aec->stream_crop_info.vfe_out_height;
  roi_x += aec->stream_crop_info.vfe_map_x;
  roi_y += aec->stream_crop_info.vfe_map_y;

  AEC_LOW("coor orig (%d,%d), vfe (%d, %d,%d %d ) pp(%d %d %d %d) after (%d, %d) prev(%d %d)",
    *x, *y, aec->stream_crop_info.vfe_map_x, aec->stream_crop_info.vfe_map_y,
    aec->stream_crop_info.vfe_map_width,aec->stream_crop_info.vfe_map_height,
    aec->stream_crop_info.pp_x, aec->stream_crop_info.pp_y,
    aec->stream_crop_info.pp_crop_out_x, aec->stream_crop_info.pp_crop_out_y,
    roi_x, roi_y, aec->preview_width, aec->preview_height);

  *x = roi_x;
  *y = roi_y;
  return TRUE;
}

boolean aec_biz_translate_coord_camif2fov(aec_biz_t *aec,
  uint16_t *roi_x, uint16_t *roi_y)
{
  boolean result = TRUE;
  int  x, y;

  x = *roi_x;
  y = *roi_y;

  if (aec->stream_crop_info.vfe_out_width == 0 ||
    aec->stream_crop_info.vfe_out_height == 0)
    return FALSE;

  if (aec->preview_width == 0 || aec->preview_height == 0) {
    aec->preview_width = aec->stream_crop_info.pp_crop_out_x + 1;
    aec->preview_height = aec->stream_crop_info.pp_crop_out_y + 1;
    AEC_ERR("preview size invalid. Set it to crop info. preview x,y: (%d, %d)",
      aec->preview_width, aec->preview_height);
  }
  AEC_LOW("aec->preview_width=%d, aec->preview_height=%d,\
    aec->stream_crop_info.vfe_out_width=%d,aec->stream_crop_info.vfe_out_height=%d",
    aec->preview_width,aec->preview_height,
    aec->stream_crop_info.vfe_out_width, aec->stream_crop_info.vfe_out_height);
  if (!aec->stream_crop_info.pp_crop_out_x ||
    !aec->stream_crop_info.pp_crop_out_y) {
    aec->stream_crop_info.pp_x = 0;
    aec->stream_crop_info.pp_y = 0;
    aec->stream_crop_info.pp_crop_out_x = aec->preview_width;
    aec->stream_crop_info.pp_crop_out_y = aec->preview_height;
    AEC_ERR("crop info invalid. Set it to preview size. crop_out x,y: (%d, %d)",
      aec->stream_crop_info.pp_crop_out_x, aec->stream_crop_info.pp_crop_out_y);
  }

  if(x >= (int)aec->stream_crop_info.vfe_map_x) {
    x -= aec->stream_crop_info.vfe_map_x;
  }
  if(y >= (int)aec->stream_crop_info.vfe_map_y) {
    y -= aec->stream_crop_info.vfe_map_y;
  }
  if (x < 0) {
    AEC_LOW("ROI x invalid %d", x);
    result = FALSE;
    x = 0;
  }
  if (y < 0) {
    AEC_LOW("ROI y invalid %d", y);
    result = FALSE;
    y = 0;
  }

  x = x * aec->stream_crop_info.vfe_out_width / aec->stream_crop_info.vfe_map_width;
  y = y * aec->stream_crop_info.vfe_out_height / aec->stream_crop_info.vfe_map_height;

  if(x >= (int)aec->stream_crop_info.pp_x) {
    x -= aec->stream_crop_info.pp_x;
  }
  if(y >= (int)aec->stream_crop_info.pp_y) {
    y -= aec->stream_crop_info.pp_y;
  }
  if (x < 0) {
    AEC_LOW("ROI x invalid %d", x);
    result = FALSE;
    x = 0;
  }
  if (y < 0) {
    AEC_LOW("ROI y invalid %d", y);
    result = FALSE;
    y = 0;
  }
  x = x * aec->preview_width / aec->stream_crop_info.pp_crop_out_x;
  y = y * aec->preview_height / aec->stream_crop_info.pp_crop_out_y;

  *roi_x = x;
  *roi_y = y;
  return result;
}

/** aec_process_translate_dim_fov2camif
 *
 **/
boolean aec_biz_translate_dim_fov2camif(aec_biz_t *aec,
  uint16_t *dx, uint16_t *dy)
{
  int  roi_dx, roi_dy;
  if(aec->stream_crop_info.vfe_out_width == 0 ||
    aec->stream_crop_info.vfe_out_height == 0)
    return FALSE;
  if (! aec->stream_crop_info.pp_crop_out_x ||
   !aec->stream_crop_info.pp_crop_out_y) {
   aec->stream_crop_info.pp_x = 0;
   aec->stream_crop_info.pp_y = 0;
   aec->stream_crop_info.pp_crop_out_x = aec->preview_width;
   aec->stream_crop_info.pp_crop_out_y = aec->preview_height;
  }

  /* Reverse calculation to cpp output */
  roi_dx = *dx  * aec->stream_crop_info.pp_crop_out_x /aec->preview_width ;
  roi_dy = *dy *  aec->stream_crop_info.pp_crop_out_y/ aec->preview_height;
  /* Reverse calculation for vfe output */
  roi_dx = roi_dx * aec->stream_crop_info.vfe_map_width /  aec->stream_crop_info.vfe_out_width ;
  roi_dy = roi_dy *  aec->stream_crop_info.vfe_map_height /  aec->stream_crop_info.vfe_out_height;
  AEC_LOW("orig (%d,%d), vfe (%d, %d,%d %d ) vfe out(%d %d) pp(%d %d %d %d) after (%d, %d)",
    *dx, *dy, aec->stream_crop_info.vfe_map_x, aec->stream_crop_info.vfe_map_y,
    aec->stream_crop_info.vfe_map_width,aec->stream_crop_info.vfe_map_height,
    aec->stream_crop_info.vfe_out_width, aec->stream_crop_info.vfe_out_height,
    aec->stream_crop_info.pp_x, aec->stream_crop_info.pp_y,
    aec->stream_crop_info.pp_crop_out_x, aec->stream_crop_info.pp_crop_out_y,
    roi_dx, roi_dy);
  *dx = roi_dx;
  *dy = roi_dy;
  return TRUE;
}

/** aec_process_translate_dim_camif2fov
 *
 **/
boolean aec_biz_translate_dim_camif2fov(aec_biz_t *aec,
  uint16_t *dx, uint16_t *dy)
{
  int  roi_dx, roi_dy;
  if (aec->stream_crop_info.vfe_out_width == 0 || aec->stream_crop_info.vfe_out_height == 0)
    return FALSE;

  if (! aec->stream_crop_info.pp_crop_out_x ||
   !aec->stream_crop_info.pp_crop_out_y) {
   aec->stream_crop_info.pp_x = 0;
   aec->stream_crop_info.pp_y = 0;
   aec->stream_crop_info.pp_crop_out_x = aec->preview_width;
   aec->stream_crop_info.pp_crop_out_y = aec->preview_height;
  }

  /* VFE to CPP */
  roi_dx = *dx * aec->stream_crop_info.vfe_out_width / aec->stream_crop_info.vfe_map_width;
  roi_dy = *dy * aec->stream_crop_info.vfe_out_height / aec->stream_crop_info.vfe_map_height;

  /* CPP to Preview */
  roi_dx = roi_dx * aec->preview_width / aec->stream_crop_info.pp_crop_out_x;
  roi_dy = roi_dy * aec->preview_height / aec->stream_crop_info.pp_crop_out_y;

  AEC_LOW("ddl orig (%d,%d), vfe (%d, %d,%d %d ) vfe out(%d %d) pp(%d %d %d %d) after (%d, %d)",
    *dx, *dy, aec->stream_crop_info.vfe_map_x, aec->stream_crop_info.vfe_map_y,
    aec->stream_crop_info.vfe_map_width,aec->stream_crop_info.vfe_map_height,
    aec->stream_crop_info.vfe_out_width, aec->stream_crop_info.vfe_out_height,
    aec->stream_crop_info.pp_x, aec->stream_crop_info.pp_y,
    aec->stream_crop_info.pp_crop_out_x, aec->stream_crop_info.pp_crop_out_y,
    roi_dx, roi_dy);

  *dx = roi_dx;
  *dy = roi_dy;
  return TRUE;
}

/** aec_process_pack_stats_config(output, aec);:
 *
 **/
void aec_biz_pack_stats_config(aec_biz_t *aec,
  aec_output_data_t *output, aec_core_output_type *core_output)
{
  uint16_t top, left, width = 0, height = 0, config_width, config_height;
  uint32_t r_Max, gr_Max, gb_Max, b_Max;
  aec_config_t *config = NULL;
  aec_bg_config_t* bg_config = NULL;
  config = &aec->stats_config;
  top = left = 0;

  if (!aec_biz_translate_coord_fov2camif(aec, &left, &top)){
    AEC_LOW("invalid inputs to translate fov 2 camif ");
  }
  /* add check to to make widht as vfe width */
  if(!aec->stream_crop_info.pp_x && !aec->stream_crop_info.pp_y &&
    !aec->stream_crop_info.pp_crop_out_x && !aec->stream_crop_info.pp_crop_out_y){
    width = aec->stream_crop_info.vfe_out_width;
    height = aec->stream_crop_info.vfe_out_height;
  } else {
    width = aec->preview_width;
    height = aec->preview_height;
  }
  if(!aec_biz_translate_dim_fov2camif(aec,&width, &height)){
    AEC_HIGH("invalid inputs to translate dimensions for  fov 2 camif ");
    width = aec->sensor_info.sensor_res_width;
    height =  aec->sensor_info.sensor_res_height;
  }

  int32 bit_shift = 0;
  if (aec->stats_depth > AEC_BG_STATS_CONSUMP_BIT_WIDTH) {
    bit_shift = aec->stats_depth - AEC_BG_STATS_CONSUMP_BIT_WIDTH;
  }

  /* Select BG stats to be configure */
  if (aec->aec_enable_stats_mask & (1 << MSM_ISP_STATS_BG)) {
    bg_config = &config->bg_config;
  } else if (aec->aec_enable_stats_mask & (1 << MSM_ISP_STATS_AEC_BG)) {
    bg_config = &config->aec_bg_config;
  } else if (aec->aec_enable_stats_mask & (1 << MSM_ISP_STATS_HDR_BE) ||
             aec->aec_enable_stats_mask & (1 << MSM_ISP_STATS_BE)) {
    /* these two are of the same underlying type for now */
    bg_config = &config->be_config;
  } else {
    AEC_ERR("Error: invalid config mask: 0x%x", aec->aec_enable_stats_mask);
    return;
  }

  /* Configure selected BG stats */
  bg_config->is_valid = TRUE;
  float bg_thres_divider = MAX(1.0f, core_output->bg_thres_divider);
  if (aec->aec_enable_stats_mask & (1 << MSM_ISP_STATS_BG) ||
      aec->aec_enable_stats_mask & (1 << MSM_ISP_STATS_HDR_BE)) {
    b_Max  = (uint32_t)(((255 - 16) << bit_shift) / bg_thres_divider);
    gb_Max = (uint32_t)(((255 - 16) << bit_shift) / bg_thres_divider);
    gr_Max = (uint32_t)(((255 - 16) << bit_shift) / bg_thres_divider);
    r_Max  = (uint32_t)(((255 - 16) << bit_shift) / bg_thres_divider);
    bg_config->grid_info.h_num = 64;
    bg_config->grid_info.v_num = 48;
  } else if (aec->aec_enable_stats_mask & (1 << MSM_ISP_STATS_BE)) {
    /* Unlike HDR_BE, BE stats has 32x24 region limitation, AEC should set
     * correct region number accordingly. Note that for AEC, BE stats is a
     * rare usecase, only Mono sensor will use BE stats here */
    b_Max  = (uint32_t)((255 - 16) << bit_shift);
    gb_Max = (uint32_t)((255 - 16) << bit_shift);
    gr_Max = (uint32_t)((255 - 16) << bit_shift);
    r_Max  = (uint32_t)((255 - 16) << bit_shift);
    bg_config->grid_info.h_num = 32;
    bg_config->grid_info.v_num = 24;
  } else { /* For AEC_BG */
    b_Max  = (uint32_t)(((1 << aec->stats_depth) - 1) / bg_thres_divider);
    gb_Max = (uint32_t)(((1 << aec->stats_depth) - 1) / bg_thres_divider);
    gr_Max = (uint32_t)(((1 << aec->stats_depth) - 1) / bg_thres_divider);
    r_Max  = (uint32_t)(((1 << aec->stats_depth) - 1) / bg_thres_divider);
    bg_config->grid_info.h_num = 64;
    bg_config->grid_info.v_num = 48;
  }

  config_width =
    (left + width > aec->sensor_info.sensor_res_width) ?
    (aec->sensor_info.sensor_res_width - left) : width;
  config_height =
    (top + height > aec->sensor_info.sensor_res_height) ?
    (aec->sensor_info.sensor_res_height - top) : height;

  if (bg_config->roi.left   != left          ||
      bg_config->roi.top    != top           ||
      bg_config->roi.width  != config_width  ||
      bg_config->roi.height != config_height ||
      bg_config->r_Max      != r_Max         ||
      bg_config->gr_Max     != gr_Max        ||
      bg_config->gb_Max     != gb_Max        ||
      bg_config->b_Max      != b_Max) {

    /* Config BG */
    bg_config->roi.left   = left;
    bg_config->roi.top    = top;
    bg_config->roi.width  = config_width;
    bg_config->roi.height = config_height;
    bg_config->r_Max      = r_Max;
    bg_config->gr_Max     = gr_Max;
    bg_config->gb_Max     = gb_Max;
    bg_config->b_Max      = b_Max;

    /* Config Bhist*/
    config->bhist_config.roi.left   = left;
    config->bhist_config.roi.top    = top;
    config->bhist_config.roi.width  = config_width;
    config->bhist_config.roi.height = config_height;

    output->need_config = 1;
    output->config = (*config);
  } else {
    output->need_config = 0;
  }

  AEC_HIGH("BG STATS CONFIG: grid_info: h_num: %d, v_num: %d, "
      "ROI_info: left: %d, top: %d, width: %d, height: %d "
      "r_max: %d, gr_max: %d, b_max: %d, gb_max: %d, shift_bits: %d",
      bg_config->grid_info.h_num, bg_config->grid_info.v_num,
      bg_config->roi.left, bg_config->roi.top, bg_config->roi.width, bg_config->roi.height,
      bg_config->r_Max, bg_config->gr_Max, bg_config->b_Max, bg_config->gb_Max, bg_config->shift_bits);

}

/** aec_biz_map_stats_type_to_algo
 *    @dest_algo_stats: The stats mapped to algo are written here (output)
 *    @src_biz_stats: Stats mask from ISP (input)
 *
 * Map ISP stats to Core stats
 *
 * Return: TRUE in success
 **/
static boolean aec_biz_map_stats_type_to_algo(uint32 *dest_algo_stats,
  uint32 *src_biz_stats)
{
  uint32 algo_dest = 0;
  uint32 isp_stats = *src_biz_stats;

  /* Mapping only stats that AEC may require */
  if (isp_stats & (1 << MSM_ISP_STATS_AEC)) {
    algo_dest |= STATS_AEC; /* Map the stats corresponding to the bit */
  }
  if (isp_stats & (1 << MSM_ISP_STATS_IHIST)) {
    algo_dest |= STATS_IHISTO;
  }
  if (isp_stats & (1 << MSM_ISP_STATS_BG)) {
    algo_dest |= STATS_BG;
  }
  if (isp_stats & (1 << MSM_ISP_STATS_AEC_BG)) {
    algo_dest |= STATS_BG_AEC;
  }
  if (isp_stats & (1 << MSM_ISP_STATS_BE)) {
    algo_dest |= STATS_BE;
  }
  if (isp_stats & (1 << MSM_ISP_STATS_HDR_BHIST)) {
    algo_dest |= STATS_HBHISTO;
  }
  if (isp_stats & (1 << MSM_ISP_STATS_BHIST)) {
    algo_dest |= STATS_BHISTO;
  }
  if (isp_stats & (1 << MSM_ISP_STATS_HDR_BE)) {
    algo_dest |= STATS_HDR_BE;
  }

  *dest_algo_stats = algo_dest;

  AEC_LOW("map stats from 0x%x to 0x%x", *src_biz_stats, *dest_algo_stats);
  return TRUE;
}

/** aec_biz_map_stats_type_to_biz
 *    @dest_biz_stats: The stats mapped to biz logic ISP (output)
 *    @src_algo_stats: Stats mask from algo (input)
 *
 * Map Core to ISP stats
 *
 * Return: TRUE in success
 **/
static boolean aec_biz_map_stats_type_to_biz(uint32 *dest_biz_stats,
  uint32 *src_algo_stats)
{
  uint32 dest_stats = 0;
  uint32 algo_stats = *src_algo_stats;

  /* STATS_HDR_VID: This is provided by sensor no ISP, there is no need to map
   * Mapping only stats that AEC may require */
  if (algo_stats & STATS_BE) {
    dest_stats |= (1 << MSM_ISP_STATS_BE);
  }
  if (algo_stats & STATS_BG) {
    dest_stats |= (1 << MSM_ISP_STATS_BG);
  }
  if (algo_stats & STATS_BG_AEC) {
    dest_stats |= (1 << MSM_ISP_STATS_AEC_BG);
  }
  if (algo_stats & STATS_HBHISTO) {
    dest_stats |= (1 << MSM_ISP_STATS_HDR_BHIST);
  }
  if (algo_stats & STATS_BHISTO) {
    dest_stats |= (1 << MSM_ISP_STATS_BHIST);
  }
  if (algo_stats & STATS_HDR_BE) {
    dest_stats |= (1 << MSM_ISP_STATS_HDR_BE);
  }
  if (algo_stats & STATS_AEC) {
    dest_stats |= (1 << MSM_ISP_STATS_AEC);
  }
  if (algo_stats & STATS_IHISTO) {
    dest_stats |= (1 << MSM_ISP_STATS_IHIST);
  }

  *dest_biz_stats = dest_stats;

  AEC_LOW("map stats from 0x%x to 0x%x", *src_algo_stats, *dest_biz_stats);
  return TRUE;
}

/** aec_get_parameters:
 *
 **/
static boolean aec_biz_get_param(aec_get_parameter_t *param, void *aec_obj)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  aec_biz_t *aec = (aec_biz_t *)aec_obj;
  aec_core_get_info_type get_param_info;
  aec_core_get_output_type get_param_output;

  if (!param || !aec) {
   AEC_ERR("Invalid input: %p,%p",param, aec);
    return FALSE;
  }

  if (NULL == aec->aec_algo_ops.get_parameters) {
    AEC_ERR("ops.get_parameters is NULL, can't get: %p,%p", param, aec);
    return FALSE;
  }

  AEC_LOW("AEC_EVENT:cam:%u: %s", param->camera_id,
    aec_biz_get_param_string(param->type));

  switch (param->type) {
  case AEC_GET_PARAM_EXPOSURE_PARAMS: {
    get_param_info.param_type = AEC_GET_EXPOSURE_PARAMS;
    q3a_custom_data_t custom_data =
      param->u.exp_params.custom_param;
    STATS_MEMSET(&get_param_output.u.exp_params.custom_param, 0,
      sizeof(q3a_custom_data_t));
    if (custom_data.data != NULL &&
      custom_data.size > 0) {
      get_param_output.u.exp_params.custom_param =
        custom_data;
    }
    rc = aec->aec_algo_ops.get_parameters(
      aec->handle, &get_param_info, &get_param_output);
    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AEC_ERR("aec get parameter failed");
    return FALSE;
    }
    STATS_MEMCPY(&param->u.exp_params, sizeof(aec_exp_parms_t),
      &get_param_output.u.exp_params, sizeof(aec_exp_parms_t));
  } /* case AEC_GET_PARAM_EXPOSURE_PARAMS */
    break;

  case AEC_GET_PARAM_REQUIRED_STATS: {
    get_param_info.param_type = AEC_GET_STATS_REQUIRED;
    rc = aec_biz_map_stats_type_to_algo(&get_param_info.u.supported_stats_mask,
      &param->u.request_stats.supported_stats_mask);
    if (!rc) {
      AEC_ERR("Error to map stats to algo type");
      return FALSE;
    }
    rc = aec->aec_algo_ops.get_parameters(
      aec->handle, &get_param_info, &get_param_output);
    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AEC_ERR("aec get parameter failed");
      return FALSE;
    }

    rc = aec_biz_map_stats_type_to_biz(&param->u.request_stats.enable_stats_mask,
      &get_param_output.u.enable_stats_mask);
    if (!rc) {
      AEC_ERR("Error to map stats to biz type");
      return FALSE;
    }
    aec->aec_enable_stats_mask = param->u.request_stats.enable_stats_mask;

    AEC_LOW("AEC_GET_PARAM_REQUIRED_STATS: 0x%x",
      param->u.request_stats.enable_stats_mask);

    get_param_info.param_type = AEC_GET_RGN_SKIP_PATTERN;
    rc = aec_biz_map_stats_type_to_algo(&get_param_info.u.supported_rgn_skip_mask,
      &param->u.request_stats.supported_rgn_skip_mask);
    if (!rc) {
      AEC_ERR("Error to map stats to algo type");
      return FALSE;
    }
    rc = aec->aec_algo_ops.get_parameters(
      aec->handle, &get_param_info, &get_param_output);
    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AEC_ERR("aec get parameter failed");
      return FALSE;
    }
    param->u.request_stats.enable_rgn_skip_pattern =
      get_param_output.u.enable_rgn_skip_pattern;

  }
  break;

  case AEC_GET_PARAM_UNIFIED_FLASH: {
    get_param_info.param_type = AEC_GET_UNIFIED_FLASH;
    get_param_output.u.frame_info.num_batch = param->u.frame_info.num_batch;
    get_param_output.u.frame_info.frame_batch = param->u.frame_info.frame_batch;
    rc = aec->aec_algo_ops.get_parameters(
      aec->handle, &get_param_info, &get_param_output);
    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AEC_ERR("aec get parameter failed");
      return FALSE;
    }
    param->u.frame_info.luma_target = get_param_output.u.frame_info.luma_target;
    param->u.frame_info.current_luma = get_param_output.u.frame_info.current_luma;
    param->u.frame_info.led_off_real_gain = get_param_output.u.frame_info.led_off_real_gain;
    param->u.frame_info.led_off_sensor_gain = get_param_output.u.frame_info.led_off_sensor_gain;
    param->u.frame_info.led_off_linecount = get_param_output.u.frame_info.led_off_linecount;
    param->u.frame_info.valid_exp_entries = get_param_output.u.frame_info.valid_exp_entries;
    param->u.frame_info.use_led_estimation = get_param_output.u.frame_info.use_led_estimation;
    param->u.frame_info.metering_type = get_param_output.u.frame_info.metering_type;
    param->u.frame_info.led_off_drc_gains= get_param_output.u.frame_info.led_off_drc_gains;
    param->u.frame_info.led_off_hdr_gains= get_param_output.u.frame_info.led_off_hdr_gains;

    STATS_MEMCPY(&param->u.frame_info.flash_si, sizeof(param->u.frame_info.flash_si),
      &get_param_output.u.frame_info.flash_si, sizeof(get_param_output.u.frame_info.flash_si));

  } /* case AEC_GET_PARAM_EXPOSURE_PARAMS */
  break;

  case AEC_GET_PARAM_LED_CAL_CONFIG: {
    get_param_info.param_type = AEC_GET_LED_CAL_CONFIG;
    rc = aec->aec_algo_ops.get_parameters(
      aec->handle, &get_param_info, &get_param_output);
    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AEC_ERR("aec get parameter failed");
      return FALSE;
    }
    param->u.led_cal_config.led_on_interval =
      get_param_output.u.led_cal_config.led_on_interval;
    param->u.led_cal_config.led_off_interval =
      get_param_output.u.led_cal_config.led_off_interval;
    param->u.led_cal_config.led_toggle_num =
      get_param_output.u.led_cal_config.led_toggle_num;
    param->u.led_cal_config.led_cal_num =
      get_param_output.u.led_cal_config.led_cal_num;
  }
    break;

  default:
    AEC_ERR("invalid param type: %d",param->type);
    return FALSE;
  }

  return TRUE;
}

static boolean aec_biz_map_set_iso(aec_biz_t *aec, int32 iso)
{
  boolean rc = TRUE;
  aec_iso_mode_type iso_mode;
  uint32 iso_value;

  if (iso < AEC_ISO_AUTO)
    iso = AEC_ISO_AUTO;

  if (iso == AEC_ISO_AUTO || iso == AEC_ISO_DEBLUR) {
    iso_mode = (aec_iso_mode_type)iso;
    iso_value = 0;
  } else if (iso >= AEC_ISO_MAX){
    iso_mode = AEC_ISO_MODE_MANUAL;
    iso_value = (uint32)iso;
  } else {
    iso_mode = AEC_ISO_MODE_MANUAL;
    int32 temp_iso_multiplier = iso - AEC_ISO_100;
    temp_iso_multiplier = (1 << temp_iso_multiplier);
    iso_value = temp_iso_multiplier * 100;
  }

  aec->bestshot_off_data.iso_mode = iso_mode;
  aec->bestshot_off_data.iso_value = iso_value;
  rc = aec_add_core_entry(AEC_INPUT_PARAM_ISO_MODE,
    sizeof(aec_iso_mode_type), &iso_mode, &aec->core_input.input_info);
  rc = aec_add_core_entry(AEC_INPUT_PARAM_ISO_VALUE,
    sizeof(uint32), &iso_value, &aec->core_input.input_info);

  return rc;
}

static boolean aec_biz_map_set_roi(aec_biz_t *aec,
  const aec_interested_region_t *roi, boolean sensor_roi)
{
  uint32 i = 0;
  boolean rc = TRUE;
  q3a_core_roi_configuration_type  aec_roi;

  if (!aec || !roi) {
    AEC_ERR("Invalid input: %p,%p",aec, roi);
    return FALSE;
  }

  if (!roi->enable) {
    aec_roi.roi_count = 0;
  } else {
    aec_roi.roi_count = roi->num_regions;
    for (i = 0; i < roi->num_regions && i < Q3A_CORE_MAX_ROI_COUNT; i++) {
      uint16 x  = roi->r[i].x;
      uint16 y  = roi->r[i].y;
      uint16 dx = roi->r[i].dx;
      uint16 dy = roi->r[i].dy;

      if (sensor_roi) {
        rc = aec_biz_translate_coord_camif2fov(aec, &x, &y);
        if (rc) {
          rc = aec_biz_translate_dim_camif2fov(aec, &dx, &dy);
        }
        if (!rc) {
          AEC_ERR("failed to convert from camif to fov");
          return FALSE;
        }
      }

      aec_roi.roi[i].x = (float)x;
      aec_roi.roi[i].y = (float)y;
      aec_roi.roi[i].dx = (float)dx;
      aec_roi.roi[i].dy = (float)dy;
      aec_roi.roi[i].weight = roi->weight;
    }
  }
  rc = aec_add_core_entry(AEC_INPUT_PARAM_ROI,
    sizeof(q3a_core_roi_configuration_type), &aec_roi, &aec->core_input.input_info);

  return rc;
}

static boolean aec_biz_map_set_fd_roi(aec_biz_t *aec,const aec_proc_roi_info_t* roi)
{
  uint32 i = 0, face_idx = 0;
  boolean rc = TRUE;
  aec_proc_fd_roi_info_type fd_roi;

  if (!aec || !roi) {
    AEC_ERR("Invalid input: %p,%p",aec, roi);
    return FALSE;
  }

  if (!aec->preview_width || !aec->preview_height) {
    AEC_ERR("invalid preview width and height (%d, %d)!", aec->preview_width, aec->preview_height);
    fd_roi.roi_count = 0;
  } else {
    fd_roi.frm_width  = (uint32)(aec->preview_width);
    fd_roi.frm_height = (uint32)(aec->preview_height);

    for (i = 0; i < roi->num_roi; i++) {
      uint16 x  = (uint16)roi->roi[i].x;
      uint16 y  = (uint16)roi->roi[i].y;
      uint16 dx = (uint16)roi->roi[i].dx;
      uint16 dy = (uint16)roi->roi[i].dy;
      /* Only copy the ROI if it's within the preview window */
      if (aec_biz_translate_coord_camif2fov(aec, &x, &y) &&
          aec_biz_translate_dim_camif2fov(aec, &dx, &dy)) {
        if(x + dx > aec->preview_width) {
          dx = aec->preview_width - x;
        }
        if(y + dy > aec->preview_height) {
          dy = aec->preview_height - y;
        }
        fd_roi.roi[face_idx].x  = x;
        fd_roi.roi[face_idx].y  = y;
        fd_roi.roi[face_idx].dx = dx;
        fd_roi.roi[face_idx].dy = dy;
        /* This field is currently not consumed by the algorithm */
        fd_roi.roi[face_idx].weight = 1;

        face_idx++;
      }
    }
    fd_roi.roi_count = (uint8)(face_idx);
  }

  rc = aec_add_core_entry(AEC_INPUT_PARAM_FD_ROI,
    sizeof(aec_proc_fd_roi_info_type), &fd_roi, &aec->core_input.input_info);

  return rc;
}

static boolean aec_biz_map_set_led_fd_roi(aec_biz_t *aec,const aec_proc_roi_info_t* src_roi,
  aec_proc_fd_roi_info_type *dest_roi)
{
  uint32 i = 0, face_idx = 0;
  boolean rc = TRUE;

  if (!aec || !src_roi) {
    AEC_ERR("Invalid input: %p,%p",aec, src_roi);
    return FALSE;
  }

  if (!aec->preview_width || !aec->preview_height) {
    AEC_ERR("invalid preview width and height (%d, %d)!", aec->preview_width, aec->preview_height);
    dest_roi->roi_count = 0;
  } else {
    dest_roi->frm_width  = (uint32)(aec->preview_width);
    dest_roi->frm_height = (uint32)(aec->preview_height);
    AEC_LOW("LED FD: preview(w=%d, h=%d)", aec->preview_width, aec->preview_height);

    for (i = 0; i < src_roi->num_roi; i++) {
      uint16 x  = (uint16)src_roi->roi[i].x;
      uint16 y  = (uint16)src_roi->roi[i].y;
      uint16 dx = (uint16)src_roi->roi[i].dx;
      uint16 dy = (uint16)src_roi->roi[i].dy;
      /* Only copy the ROI if it's within the preview window */
      if (aec_biz_translate_coord_camif2fov(aec, &x, &y) &&
          aec_biz_translate_dim_camif2fov(aec, &dx, &dy) &&
          x + dx <= aec->preview_width &&
          y + dy <= aec->preview_height) {
        dest_roi->roi[face_idx].x  = x;
        dest_roi->roi[face_idx].y  = y;
        dest_roi->roi[face_idx].dx = dx;
        dest_roi->roi[face_idx].dy = dy;
        /* This field is currently not consumed by the algorithm */
        dest_roi->roi[face_idx].weight = 1;
        face_idx++;
        AEC_LOW("LED FD: fd ROI(face idx=%d, x=%d, y=%d, dx=%d, dy=%d)", face_idx, x, y, dx, dy);
      }
    }
    dest_roi->roi_count = (uint8)(face_idx);
  }
  return rc;
}

static boolean aec_biz_map_set_mtr_area(aec_biz_t *aec, const aec_proc_mtr_area_t* mtr_area)
{
  int i = 0;
  boolean rc = TRUE;
  q3a_core_roi_configuration_type aec_mtr_area;

  if (!aec || !mtr_area) {
    AEC_ERR("Invalid input: %p,%p",aec, mtr_area);
    return FALSE;
  }

  aec_mtr_area.roi_count = mtr_area->num_area;
  for (i = 0; i <mtr_area->num_area; i++) {
    aec_mtr_area.roi[i].x = mtr_area->mtr_area[i].x;
    aec_mtr_area.roi[i].y = mtr_area->mtr_area[i].y;
    aec_mtr_area.roi[i].dx = mtr_area->mtr_area[i].dx;
    aec_mtr_area.roi[i].dy = mtr_area->mtr_area[i].dy;
    aec_mtr_area.roi[i].weight = (float)mtr_area->weight[i];
  }

  rc = aec_add_core_entry(AEC_INPUT_PARAM_MTR_AREA,
    sizeof(q3a_core_roi_configuration_type), &aec_mtr_area, &aec->core_input.input_info);

  return rc;
}

static boolean aec_biz_set_bestshot_mode(aec_biz_t *aec,
  aec_bestshot_mode_type_t new_mode)
{
  int rc = 0;
  int32 exp_comp_val = 0;
  aec_iso_mode_type iso_mode = AEC_ISO_MODE_AUTO;
  int32 iso_value = 0;
  aec_auto_exposure_mode_t metering_type = AEC_METERING_FRAME_AVERAGE;


  AEC_LOW("new mode: %d", new_mode);
  if (new_mode >= AEC_BESTSHOT_MAX) {
    new_mode = AEC_BESTSHOT_OFF;
    AEC_ERR("%s: Invalid bestshot mode, setting it to default", __func__);
  }

  if (new_mode != AEC_BESTSHOT_OFF) {
    /* CONFIG AEC for BESTHOT mode */
    switch (new_mode) {
    case AEC_BESTSHOT_SPORTS:
    case AEC_BESTSHOT_ANTISHAKE:
    case AEC_BESTSHOT_ACTION:
      iso_mode = AEC_ISO_MODE_MANUAL;
      iso_value = 400;
      rc = aec_add_core_entry(AEC_INPUT_PARAM_ISO_MODE,
        sizeof(aec_iso_mode_t), &iso_mode, &aec->core_input.input_info);
      rc = aec_add_core_entry(AEC_INPUT_PARAM_ISO_VALUE,
        sizeof(int32), &iso_value, &aec->core_input.input_info);
      break;

    case AEC_BESTSHOT_OFF:
    case AEC_BESTSHOT_LANDSCAPE:
    case AEC_BESTSHOT_SNOW:
    case AEC_BESTSHOT_BEACH:
    case AEC_BESTSHOT_SUNSET:
    case AEC_BESTSHOT_NIGHT:
    case AEC_BESTSHOT_PORTRAIT:
    case AEC_BESTSHOT_BACKLIGHT:
    case AEC_BESTSHOT_FLOWERS:
    case AEC_BESTSHOT_CANDLELIGHT:
    case AEC_BESTSHOT_FIREWORKS:
    case AEC_BESTSHOT_PARTY:
    case AEC_BESTSHOT_NIGHT_PORTRAIT:
    case AEC_BESTSHOT_THEATRE:
    case AEC_BESTSHOT_AR:
    default:
      iso_mode = AEC_ISO_MODE_AUTO;
      rc = aec_add_core_entry(AEC_INPUT_PARAM_ISO_MODE,
        sizeof(aec_iso_mode_t), &iso_mode, &aec->core_input.input_info);
      break;
    }

    switch (new_mode) {
    case AEC_BESTSHOT_LANDSCAPE:
    case AEC_BESTSHOT_SNOW:
    case AEC_BESTSHOT_BEACH:
    case AEC_BESTSHOT_SUNSET:
      metering_type = AEC_METERING_FRAME_AVERAGE;
      rc = aec_add_core_entry(AEC_INPUT_PARAM_METERING_MODE,
        sizeof(aec_auto_exposure_mode_t), &metering_type, &aec->core_input.input_info);
      break;

    case AEC_BESTSHOT_OFF:
    case AEC_BESTSHOT_NIGHT:
    case AEC_BESTSHOT_PORTRAIT:
    case AEC_BESTSHOT_BACKLIGHT:
    case AEC_BESTSHOT_SPORTS:
    case AEC_BESTSHOT_ANTISHAKE:
    case AEC_BESTSHOT_FLOWERS:
    case AEC_BESTSHOT_CANDLELIGHT:
    case AEC_BESTSHOT_FIREWORKS:
    case AEC_BESTSHOT_PARTY:
    case AEC_BESTSHOT_NIGHT_PORTRAIT:
    case AEC_BESTSHOT_THEATRE:
    case AEC_BESTSHOT_ACTION:
    case AEC_BESTSHOT_AR:
    default:
      metering_type = AEC_METERING_CENTER_WEIGHTED;
      rc = aec_add_core_entry(AEC_INPUT_PARAM_METERING_MODE,
        sizeof(aec_auto_exposure_mode_t), &metering_type, &aec->core_input.input_info);
    }

    switch (new_mode) {
    case AEC_BESTSHOT_SNOW:
    case AEC_BESTSHOT_BEACH:
      /* set EV of 6 */
      exp_comp_val = 6;
      rc = aec_add_core_entry(AEC_INPUT_PARAM_EXP_COMPENSATION,
        sizeof(int32), &exp_comp_val, &aec->core_input.input_info);
      break;

    case AEC_BESTSHOT_SUNSET:
    case AEC_BESTSHOT_CANDLELIGHT:
      /* set EV of -6 */
      exp_comp_val = -6;
      rc = aec_add_core_entry(AEC_INPUT_PARAM_EXP_COMPENSATION,
        sizeof(int32), &exp_comp_val, &aec->core_input.input_info);
      break;

    case AEC_BESTSHOT_FACE_PRIORITY:
      /* for face priority mode, we don't touch EV, otherwise it will
       * nullify the EV setting from app */
      break;
    default:
      exp_comp_val = 0;
      rc = aec_add_core_entry(AEC_INPUT_PARAM_EXP_COMPENSATION,
        sizeof(int32), &exp_comp_val, &aec->core_input.input_info);
      break;
    }
  } else if (AEC_BESTSHOT_OFF != aec->prev_bestshot_mode) {
    /* Restore AEC vals */
    rc = aec_add_core_entry(AEC_INPUT_PARAM_ISO_MODE, sizeof(aec_iso_mode_type),
      &aec->bestshot_off_data.iso_mode, &aec->core_input.input_info);
    rc = aec_add_core_entry(AEC_INPUT_PARAM_ISO_VALUE, sizeof(int32),
      &aec->bestshot_off_data.iso_value, &aec->core_input.input_info);
    rc = aec_add_core_entry(AEC_INPUT_PARAM_METERING_MODE,
      sizeof(aec_auto_exposure_mode_t), &aec->bestshot_off_data.metering_type,
      &aec->core_input.input_info);
    rc = aec_add_core_entry(AEC_INPUT_PARAM_EXP_COMPENSATION, sizeof(int32),
      &aec->bestshot_off_data.exp_comp_val, &aec->core_input.input_info);
  }

  aec->prev_bestshot_mode = new_mode;
  return rc;
}

static void aec_biz_map_exp_bracket(char *str, aec_bracket_t* aec_bracket)
{
  int val_i, table_size;
  char *val, *prev_value;

  if (str == NULL || aec_bracket == NULL) {
    AEC_ERR("Invalid input, %p, %p",str, aec_bracket);
    return;
  }

  STATS_MEMSET(aec_bracket, 0, sizeof(aec_bracket_t));
  val = strtok_r(str,",", &prev_value);
  while (val != NULL) {
    val_i = atoi(val);
    aec_bracket->ev_val[aec_bracket->valid_entries] = val_i;
    aec_bracket->valid_entries++;
    val = strtok_r(NULL, ",", &prev_value);
  }
}

/** aec_biz_load_led_cal_data
 *    @aec: pointer to aec_biz structure
 *    @cal_data: pointer to calibration data structrure
 *
 *  Load calibration data from calibration file if present.
 */
static void aec_biz_load_led_cal_data(aec_biz_t *aec,
  aec_led_cal_data_t *cal_data)
{
  int i = 0;
  FILE* file = NULL;

  if (!aec || !cal_data){
    AEC_ERR("invalid input: %p,%p", aec, cal_data);
    return;
  }

  STATS_MEMSET(cal_data, 0, sizeof(aec_led_cal_data_t));
  /* Try to load dual led calibration data from file */
  file = fopen(LED_CAL_FILE_PATH, "rb");
  if (file) {
    /* Try to read the calibration number*/
    if (!fread(&cal_data->cal_num, sizeof(int), 1, file)) {
      AEC_ERR("Failed to read from LED calibration file");
      cal_data->cal_num = 0;
      fclose(file);
      return;
    }
    for(i = 0; i < cal_data->cal_num; i++) {
      if (!fread(&cal_data->rg_ratio[i], sizeof(float), 1, file) ||
          !fread(&cal_data->bg_ratio[i], sizeof(float), 1, file)) {
        AEC_ERR("Failed to read to LED calibration file");
        cal_data->cal_num = 0;
        fclose(file);
        return;
      }
    }
    fclose(file);
  } else {
    AEC_LOW("Cound not find LED calibration file");
    return;
  }

  AEC_HIGH("Success to load %d led cal data", cal_data->cal_num);
}

/** aec_biz_save_led_cal_data
 *    @cal_data: pointer to calibration data structrure
 *
 *  Save calibration data to file
 */
static boolean aec_biz_save_led_cal_data(aec_led_cal_data_t *cal_data)
{
  int i = 0;
  FILE *file = NULL;

  file = fopen(LED_CAL_FILE_PATH, "w");
  if (file) {
    AEC_HIGH("Saving LED calibration file...");
    if (!fwrite(&cal_data->cal_num, sizeof(int), 1, file)) {
      AEC_ERR("Failed to write to LED calibration file");
      fclose(file);
      return FALSE;
    }
    for(i = 0; i < cal_data->cal_num; i++) {
      if (!fwrite(&cal_data->rg_ratio[i], sizeof(float), 1, file) ||
          !fwrite(&cal_data->bg_ratio[i], sizeof(float), 1, file)) {
        AEC_ERR("Failed to write to LED calibration file");
        fclose(file);
        return FALSE;
      }
    }
    fclose(file);
  } else {
    AEC_ERR("Cound not open to LED calibration file to save the data");
    return FALSE;
  }

  AEC_HIGH("Success to write %d led cal data to file", cal_data->cal_num);
  return TRUE;
}

static void aec_biz_pack_output(aec_biz_t* aec,
  aec_output_data_t *output)
{
  int size = sizeof(aec_output_data_t);
  aec_core_output_type *core_output = &aec->core_output;

  if (!aec || !output) {
    AEC_ERR("invalid input: %p,%p", aec, output);
    return;
  }

  if (!aec->exif_dbg_enable) {
    size -= AEC_DEBUG_DATA_SIZE * sizeof(char);
  }

  /* Clear older output values */
  STATS_MEMSET(output, 0, size);
  size = 0;

  /* Fill requested output */
  output->type = AEC_UPDATE;
  output->aec_af.cur_af_luma = core_output->cur_af_luma;

  output->stats_update.aec_update.cur_luma = core_output->cur_luma;
  output->stats_update.aec_update.luma_delta = core_output->luma_delta;

  output->stats_update.aec_update.luma_settled_cnt = core_output->luma_settled_cnt;
  /* AWB related update */
  output->aec_awb.prev_exp_index  = core_output->prev_exp_index;
  output->stats_update.aec_update.exp_time = core_output->exp_time;
  output->stats_update.aec_update.sof_id = aec->sof_id;

  /* === Normal Updates === */
  output->stats_update.aec_update.exp_index     = core_output->exp_index;
  output->stats_update.aec_update.lux_idx       = core_output->lux_idx;
  output->pixelsPerRegion  = core_output->pixelsPerRegion;
  output->numRegions = core_output->numRegions;
  output->stats_update.aec_update.frame_id = core_output->frame_id;
  STATS_MEMCPY(output->SY, sizeof(output->SY), core_output->SY, sizeof(core_output->SY));
  output->stats_update.aec_update.SY_data.SY = &core_output->SY[0];
  output->stats_update.aec_update.SY_data.is_valid = TRUE;
  output->preview_fps = core_output->preview_fps;
  output->afr_enable = core_output->afr_enable;
  output->aec_locked = core_output->aec_locked;
  output->metering_type = core_output->metering_type;
  if (core_output->iso_mode == AEC_ISO_MODE_AUTO ||
    core_output->iso_mode == AEC_ISO_MODE_DEBLUR){
    output->iso = core_output->iso_mode;
  } else {
    output->iso = core_output->iso_value / 100 + 1;
  }
  output->stats_update.aec_update.settled = core_output->settled;
  output->stats_update.aec_update.pixelsPerRegion = core_output->pixelsPerRegion;
  output->stats_update.aec_update.numRegions = core_output->numRegions;
  output->stats_update.aec_update.low_light_shutter_flag =
    core_output->low_light_shutter_flag;
  output->stats_update.aec_update.touch_ev_status = core_output->touch_ev_status;
  /* Luma */
  output->stats_update.aec_update.target_luma = core_output->target_luma;
  output->stats_update.aec_update.cur_luma  = core_output->cur_luma;
  output->stats_update.aec_update.avg_luma = core_output->avg_luma;
  output->prev_sensitivity = core_output->prev_sensitivity;
  output->stats_update.aec_update.exp_tbl_val  = core_output->exp_tbl_val;
  output->stats_update.aec_update.max_line_cnt = core_output->max_line_cnt;
  output->stats_update.aec_update.comp_luma = core_output->comp_luma;
  output->stats_update.aec_update.preview_fps = (core_output->preview_fps);
  output->stats_update.aec_update.preview_linesPerFrame = core_output->preview_linesPerFrame;

  output->stats_update.aec_update.real_gain = core_output->real_gain;
  output->stats_update.aec_update.sensor_gain = core_output->sensor_gain;
  output->stats_update.aec_update.total_drc_gain = core_output->drc_gains.total_drc_gain;
  output->stats_update.aec_update.color_drc_gain = core_output->drc_gains.color_drc_gain;
  output->stats_update.aec_update.gtm_ratio = core_output->drc_gains.gtm_ratio;
  output->stats_update.aec_update.ltm_ratio = core_output->drc_gains.ltm_ratio;
  output->stats_update.aec_update.la_ratio = core_output->drc_gains.la_ratio;
  output->stats_update.aec_update.gamma_ratio = core_output->drc_gains.gamma_ratio;
  output->stats_update.aec_update.linecount = core_output->linecount;
  output->stats_update.aec_update.s_real_gain = core_output->s_real_gain;
  output->stats_update.aec_update.s_linecount = core_output->s_linecount;
  output->stats_update.aec_update.l_real_gain = core_output->l_real_gain;
  output->stats_update.aec_update.l_linecount = core_output->l_linecount;
  output->stats_update.aec_update.m_real_gain = core_output->m_real_gain;
  output->stats_update.aec_update.m_linecount = core_output->m_linecount;
  output->stats_update.aec_update.hdr_sensitivity_ratio = core_output->hdr_sensitivity_ratio;
  output->stats_update.aec_update.hdr_exp_time_ratio = core_output->hdr_exp_time_ratio;

  output->stats_update.aec_update.led_off_params.real_gain = core_output->led_off_real_gain;
  output->stats_update.aec_update.led_off_params.sensor_gain = core_output->led_off_sensor_gain;
  output->stats_update.aec_update.led_off_params.linecnt = core_output->led_off_linecnt;
  output->stats_update.aec_update.led_off_params.lux_index = core_output->led_off_lux_index;
  output->stats_update.aec_update.led_off_params.s_gain = core_output->led_off_s_gain;
  output->stats_update.aec_update.led_off_params.s_linecnt = core_output->led_off_s_linecnt;
  output->stats_update.aec_update.led_off_params.l_gain = core_output->led_off_l_gain;
  output->stats_update.aec_update.led_off_params.l_linecnt = core_output->led_off_l_linecnt;
  output->force_prep_snap_done = core_output->force_prep_snap_done;
  output->stats_update.aec_update.prep_snap_no_led = core_output->prep_snap_no_led;
  output->stats_update.aec_update.led_state = core_output->led_state;
  output->stats_update.aec_update.use_led_estimation = core_output->use_led_estimation;
  output->stats_update.aec_update.led_needed = core_output->led_needed;
  output->stats_update.aec_update.flash_sensitivity.high = core_output->flash_sensitivity.high;
  output->stats_update.aec_update.flash_sensitivity.low = core_output->flash_sensitivity.low;
  output->stats_update.aec_update.flash_sensitivity.off = core_output->flash_sensitivity.off;
  output->stats_update.aec_update.ir_config.ir_needed = core_output->ir_config.ir_needed;
  output->stats_update.aec_update.ir_config.ir_led_brightness =
    core_output->ir_config.ir_led_brightness;
  output->conv_speed = core_output->conv_speed;
  output->led_fd_use_delay = core_output->led_fd_use_delay;

  if (core_output->eztune.running) {
    STATS_MEMCPY(&output->eztune, sizeof(aec_ez_tune_t),
      &core_output->eztune, sizeof(aec_ez_tune_t));
  }
  /* AFD */
  output->stats_update.aec_update.band_50hz_gap = core_output->band_50hz_gap;
  output->stats_update.aec_update.cur_atb = core_output->cur_atb;

  /* HJR */
  output->hjr_snap_frame_cnt    = core_output->hjr_snap_frame_cnt;
  output->stats_update.aec_update.asd_extreme_green_cnt = core_output->asd_extreme_green_cnt;
  output->stats_update.aec_update.asd_extreme_blue_cnt  = core_output->asd_extreme_blue_cnt;

  /* total bayer stat regions 64x48  used to get ratio of extreme stats.*/
  output->stats_update.aec_update.asd_extreme_tot_regions = core_output->asd_extreme_tot_regions;
  output->hjr_dig_gain = core_output->hjr_dig_gain;
  output->snap = core_output->snap;
  /* EZtune related update */
  output->stats_update.aec_update.flash_needed = core_output->flash_needed;
  output->stats_update.aec_update.hdr_indoor_detected = core_output->hdr_indoor_detected;

  /*FaceAWB*/
  output->stats_update.aec_update.roi_count = core_output->roi_count;
  int k = 0;
  for (k = 0; k < Q3A_CORE_MAX_ROI_COUNT; k++) {
    output->stats_update.aec_update.awb_roi_x[k] = core_output->awb_roi_x[k];
    output->stats_update.aec_update.awb_roi_y[k] = core_output->awb_roi_y[k];
    output->stats_update.aec_update.awb_roi_dx[k] = core_output->awb_roi_dx[k];
    output->stats_update.aec_update.awb_roi_dy[k] = core_output->awb_roi_dy[k];
    output->stats_update.aec_update.awb_roi_weight[k] = core_output->awb_roi_weight[k];
  }
  output->stats_update.aec_update.frm_width = core_output->frm_width;
  output->stats_update.aec_update.frm_height = core_output->frm_height;
  output->stats_update.aec_update.faceawb = core_output->faceawb;

  output->stats_update.aec_update.Bv = core_output->Bv;
  output->stats_update.aec_update.Tv = core_output->Tv;
  output->stats_update.aec_update.Sv = core_output->Sv;
  output->stats_update.aec_update.Av = core_output->Av;
  output->stats_update.aec_update.exif_iso = core_output->iso_Exif;
  output->lcd_flash_settled = core_output->lcd_flash_settled;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION >= 0x0310)
  output->stats_update.aec_update.dual_led_setting.is_valid = true;
  output->stats_update.aec_update.dual_led_setting.led1_low_setting =
    core_output->dual_led_setting.low_setting.LED1_setting;
  output->stats_update.aec_update.dual_led_setting.led2_low_setting =
    core_output->dual_led_setting.low_setting.LED2_setting;
  output->stats_update.aec_update.dual_led_setting.led1_high_setting =
    core_output->dual_led_setting.high_setting.LED1_setting;
  output->stats_update.aec_update.dual_led_setting.led2_high_setting =
    core_output->dual_led_setting.high_setting.LED2_setting;

  output->stats_update.aec_update.Dual_LED1_influence_ratio = core_output->Dual_LED1_influence_ratio;

  aec_biz_copy_dual_led_setting(&core_output->dual_led_setting.low_setting,
    &output->stats_update.aec_update.dual_led_setting.low_setting);
  aec_biz_copy_dual_led_setting(&core_output->dual_led_setting.high_setting,
    &output->stats_update.aec_update.dual_led_setting.high_setting);
  aec_biz_copy_dual_led_setting(&core_output->dual_led_setting.full_setting,
    &output->stats_update.aec_update.dual_led_setting.full_setting);

  if (aec->led_cal_data.enable) {
    output->stats_update.aec_update.led_cal_state =
      core_output->led_cal_output.state;
    if(core_output->led_cal_output.state == AEC_LED_CAL_SUCCESS &&
      !aec->led_cal_data.save_done) {
      if(!aec_biz_save_led_cal_data(&core_output->led_cal_output)) {
        output->stats_update.aec_update.led_cal_state = AEC_LED_CAL_FAILED;
      }
      aec->led_cal_data.save_done = TRUE;
    }
  }
#endif

  /* Stats configuration */
  aec_biz_pack_stats_config(aec, output, core_output);

  if (aec->exif_dbg_enable) {
    q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
    aec_core_get_info_type get_param_info;
    aec_core_get_output_type get_param_output;
    get_param_info.param_type = AEC_GET_META_INFO;
    get_param_info.u.metadata_size = AEC_DEBUG_DATA_SIZE;
    get_param_output.u.meta_info.metadata = (void*)output->aec_debug_data_array;
    if (aec->aec_algo_ops.get_parameters) {
      rc = aec->aec_algo_ops.get_parameters(aec->handle, &get_param_info, &get_param_output);
    } else {
      rc = Q3A_CORE_RESULT_FAILED;
    }
    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AEC_ERR("aec get parameter failed");
    } else {
      output->aec_debug_data_size = get_param_output.u.meta_info.metadata_size;
      AEC_LOW("send AEC debug data of size: %d", output->aec_debug_data_size);
    }
  } else {
    output->aec_debug_data_size = 0;
    AEC_LOW("debug data not enabled");
  }

  /*Fill staggered HDR params*/
  if(aec->hdr_type == AEC_CORE_HDR_STAGGERED) {
    output->vhdr_update.number_of_hdr_frames =
      core_output->number_of_hdr_frames;
    output->vhdr_update.hdr_gtm_gamma = core_output->hdr_gtm_gamma;
    output->vhdr_update.hdr_exp_time_ratio = core_output->hdr_exposure_ratio;
    output->vhdr_update.linecount[VIDEO_HDR_SHORT_FRAME] =
      core_output->hdr_linecount[VIDEO_HDR_SHORT_FRAME];
    output->vhdr_update.linecount[VIDEO_HDR_LONG_FRAME] =
      core_output->hdr_linecount[VIDEO_HDR_LONG_FRAME];
    output->vhdr_update.real_gain[VIDEO_HDR_LONG_FRAME] =
      core_output->hdr_real_gain[VIDEO_HDR_LONG_FRAME];
    output->vhdr_update.real_gain[VIDEO_HDR_SHORT_FRAME] =
      core_output->hdr_real_gain[VIDEO_HDR_SHORT_FRAME];
  /*TODO: Need to remove this logic once the sensor driver fill
  exposure accepts API accepts array.of exposures*/
  output->stats_update.aec_update.s_real_gain = core_output->hdr_real_gain[VIDEO_HDR_SHORT_FRAME];
  output->stats_update.aec_update.s_linecount = core_output->hdr_linecount[VIDEO_HDR_SHORT_FRAME];
  output->stats_update.aec_update.l_real_gain = core_output->hdr_real_gain[VIDEO_HDR_LONG_FRAME];
  output->stats_update.aec_update.l_linecount = core_output->hdr_linecount[VIDEO_HDR_LONG_FRAME];
  AEC_MSG_LOW("SHDR: short :(%f,%d) long: (%f,%d)",
    output->stats_update.aec_update.s_real_gain,
    output->stats_update.aec_update.s_linecount,
    output->stats_update.aec_update.l_real_gain,
    output->stats_update.aec_update.l_linecount);
  }

  AEC_LOW("SOF ID = %d target_luma=%d cur_luma=%d stored_digital_gain=%f exp_index=%d, "
    "real_gain=%f, linecnt=%d, aec_settled=%d, iso %d, is_flash_snap=%d, "
    "snap_lux_idx=%f snap_gain=%f snap_lc=%d",
    output->stats_update.aec_update.sof_id,
    output->stats_update.aec_update.target_luma,
    output->stats_update.aec_update.cur_luma, output->stats_update.aec_update.stored_digital_gain,
    output->stats_update.aec_update.exp_index, output->stats_update.aec_update.real_gain,
    output->stats_update.aec_update.linecount, output->stats_update.aec_update.settled,
    output->stats_update.aec_update.exif_iso, output->snap.is_flash_snapshot, output->snap.lux_index,
    output->snap.real_gain, output->snap.line_count);
}

static boolean aec_biz_map_input_info(aec_biz_t *aec, aec_set_parameter_t *param)
{
  boolean rc = TRUE;

  if (!aec || !param) {
    AEC_ERR("invalid input: %p, %p",aec, param)
    return FALSE;
  }
  AEC_LOW("param_type: %d",param->type);
  switch (param->type) {
  case AEC_SET_PARAM_METERING_MODE:
    aec->bestshot_off_data.metering_type = param->u.aec_metering;
    rc = aec_add_core_entry(AEC_INPUT_PARAM_METERING_MODE,
      sizeof(aec_auto_exposure_mode_t),&param->u.aec_metering, &aec->core_input.input_info);
    break;

  case AEC_SET_PARAM_ISO_MODE:
    rc = aec_biz_map_set_iso(aec, (int32)param->u.iso.value);
    break;

  case AEC_SET_PARAM_ANTIBANDING:
    rc = aec_add_core_entry(AEC_INPUT_PARAM_ANTIBANDING,
      sizeof(aec_antibanding_type_t), &param->u.antibanding, &aec->core_input.input_info);
    break;

  case AEC_SET_PARAM_BRIGHTNESS_LVL:
    rc = aec_add_core_entry(AEC_INPUT_PARAM_BRIGHTNESS_LVL,
      sizeof(int),&param->u.brightness,&aec->core_input.input_info);
    break;

  case AEC_SET_PARAM_HJR_AF:
    rc = aec_add_core_entry(AEC_INPUT_PARAM_HJR_AF,
      sizeof(uint32),&param->u.aec_af_hjr, &aec->core_input.input_info);
    break;

  case AEC_SET_PARAM_ROI:
    rc = aec_biz_map_set_roi(aec, &param->u.aec_roi, FALSE);
    break;

  case AEC_SET_PARAM_CROP_INFO:
    aec->stream_crop_info = param->u.stream_crop;
    break;

  case AEC_SET_PARAM_FD_ROI:
    rc = aec_biz_map_set_fd_roi(aec, &param->u.fd_roi);
    break;

  case AEC_SET_PARAM_MTR_AREA:
    rc = aec_biz_map_set_mtr_area(aec, &param->u.mtr_area);
    break;

  case AEC_SET_PARAM_ASD_PARM: {
    rc = aec_add_core_entry(AEC_INPUT_PARAM_ASD_PARM,
      sizeof(aec_set_asd_param_t),&param->u.asd_param, &aec->core_input.input_info);
  }
    break;

  case AEC_SET_PARAM_AFD_PARM:
    rc = aec_add_core_entry(AEC_INPUT_PARAM_AFD_PARM,
      sizeof(aec_set_afd_parm_t),&param->u.afd_param, &aec->core_input.input_info);
    break;

  case AEC_SET_PARAM_AWB_PARM:
    rc = aec_add_core_entry(AEC_INPUT_PARAM_AWB_PARM,
      sizeof(aec_set_awb_parm_t), &param->u.awb_param, &aec->core_input.input_info);
    break;

  case AEC_SET_PARAM_GYRO_INFO:
    rc = aec_add_core_entry(AEC_INPUT_PARAM_GYRO_INFO,
      sizeof(aec_algo_gyro_info_t), &param->u.gyro_info, &aec->core_input.input_info);
    break;

  case AEC_SET_PARAM_LED_MODE:
    rc = aec_add_core_entry(AEC_INPUT_PARAM_LED_MODE,
      sizeof(q3a_led_flash_mode_t), &param->u.led_mode, &aec->core_input.input_info);
    break;

  case AEC_SET_PARAM_UI_FRAME_DIM: {
    aec_set_parameter_init_t *init_param = &(param->u.init_param);
    aec->preview_width =  init_param->frame_dim.width;
    aec->preview_height = init_param->frame_dim.height;
    rc = aec_add_core_entry(AEC_INPUT_PARAM_UI_FRAME_DIM,
      sizeof(aec_set_parameter_init_t), init_param, &aec->core_input.input_info);
  }
    break;

  case AEC_SET_PARAM_ZSL_OP:
    rc = aec_add_core_entry(AEC_INPUT_PARAM_ZSL_OP,
      sizeof(int32), &param->u.zsl_op, &aec->core_input.input_info);
    break;

  case AEC_SET_PARAM_CTRL_MODE: {
    q3a_core_roi_configuration_type aec_roi;
    aec_roi.roi_count = 0;
    rc = aec_add_core_entry(AEC_INPUT_PARAM_ROI,
      sizeof(q3a_core_roi_configuration_type), &aec_roi, &aec->core_input.input_info);
  }
    break;

  case AEC_SET_PARAM_SENSOR_ROI:
    rc = aec_biz_map_set_roi(aec, &param->u.aec_roi, TRUE);
    break;

  case AEC_SET_MANUAL_AUTO_SKIP:{
    uint32 manual_to_auto_skip_cnt = MANUAL_TO_AUTO_SKIP_CNT;
    rc = aec_add_core_entry(AEC_INPUT_MANUAL_SKIP,
      sizeof(uint32), &manual_to_auto_skip_cnt, &aec->core_input.input_info);
  }
    break;
  case AEC_SET_PARAM_STATS_DEPTH: {
    aec->stats_depth = param->u.stats_depth;
    AEC_LOW("set stats bit depth: %d", aec->stats_depth);
  }
    break;
  case AEC_SET_PARAM_EXP_TIME: {
    float exp_time = (float)param->u.manual_exposure_time.value / 1000000000;
    aec_manual_exp_time_mode_type manual_exp_time_mode = AEC_MANUAL_EXPOSURE_TIME_OFF;

    if (param->u.manual_exposure_time.value != AEC_MANUAL_EXPOSURE_TIME_AUTO) {
      manual_exp_time_mode = AEC_MANUAL_EXPOSURE_TIME_ON;
    }

    rc = aec_add_core_entry(AEC_INPUT_PARAM_MANUAL_EXP_TIME_MODE,
      sizeof(aec_manual_exp_time_mode_type), &manual_exp_time_mode, &aec->core_input.input_info);
    rc = aec_add_core_entry(AEC_INPUT_PARAM_MANUAL_EXP_TIME,
      sizeof(float), &exp_time, &aec->core_input.input_info);
  }
    break;
  default:
    AEC_ERR("invalid param type: %d",param->type);
    rc = FALSE;
    break;
  }

  return rc;
}

static void aec_biz_initialize_custom_tuning_param(
  aec_set_parameter_init_t *param)
{
  if (param) {
   /*
   * Here we initialize future chromatix variables if chromatix header is fixed.
   * These variables will go in to next chromatix headers
   */
  }
}

static boolean aec_biz_map_init_chromatix_sensor(aec_biz_t *aec, aec_set_parameter_t *param)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  aec_core_set_param_type set_param;
  chromatix_3a_parms_wrapper *chromatix;

  if (!aec || !param) {
    AEC_ERR("invalid input: %p, %p",aec, param)
    return FALSE;
  }

  set_param.type = AEC_SET_CLAMP_SNAP_LC_FOR_MIN_FPS;
  /* if ISO is manual mode and LC calculated is greater than limit to
   * maintain min fps, then clamp the linecount. Captured images can
   * be dark in this case */
  set_param.u.clamp_snap_lc_for_min_fps = TRUE;
  if (aec->aec_algo_ops.set_parameters) {
    rc = aec->aec_algo_ops.set_parameters(aec->handle, &set_param);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }
  if (rc != Q3A_CORE_RESULT_SUCCESS) {
    AEC_ERR("aec set param failed");
    return FALSE;
  }

  set_param.type = AEC_SET_COPY_CHROMATIX_SENSOR;
  chromatix = (chromatix_3a_parms_wrapper*)param->u.init_param.chromatix;
  if (!chromatix) {
    AEC_ERR("Invalid chromatix: %p", chromatix)
    return FALSE;
  }
  pthread_rwlock_rdlock(&chromatix->lock);
  STATS_MEMCPY(&set_param.u.init_param, sizeof(aec_set_parameter_init_t),
    &param->u.init_param, sizeof(aec_set_parameter_init_t));
  if (aec->aec_algo_ops.set_parameters) {
    rc = aec->aec_algo_ops.set_parameters(aec->handle, &set_param);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }
  pthread_rwlock_unlock(&chromatix->lock);
  if (rc != Q3A_CORE_RESULT_SUCCESS) {
    AEC_ERR("aec set param failed");
    return FALSE;
  }

  set_param.type = AEC_SET_INIT_CHROMATIX_SENSOR;
  STATS_MEMCPY(&set_param.u.init_param, sizeof(aec_set_parameter_init_t),
    &param->u.init_param, sizeof(aec_set_parameter_init_t));

  /* Set the longshot parameter */
  set_param.u.init_param.longshot_flash_aec_lock = LONGSHOT_FLASH_AEC_LOCK;

  /* Set Flash ON/OFF parameter for AEC lock state and Flash ON*/
  set_param.u.init_param.enable_flash_for_aec_lock =
    ENABLE_FLASH_FOR_AEC_LOCKED_STATE;
  aec_biz_initialize_custom_tuning_param(&set_param.u.init_param);

  if (!aec->led_cal_data.load_done) {
    aec_biz_load_led_cal_data(aec, &aec->led_cal_data.cal_data);
    aec->led_cal_data.load_done = TRUE;
  }
  set_param.u.init_param.led_cal_param = &aec->led_cal_data.cal_data;

  if (aec->aec_algo_ops.set_parameters) {
    rc = aec->aec_algo_ops.set_parameters(aec->handle, &set_param);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }
  if (rc != Q3A_CORE_RESULT_SUCCESS) {
    AEC_ERR("aec set param failed");
    return FALSE;
  }

  /* Do not reload parameter for snapshot mode */
  if (param->u.init_param.op_mode != AEC_OPERATION_MODE_SNAPSHOT)
  {
    aec_core_get_info_type get_param_info;
    get_param_info.param_type = AEC_GET_RELOAD_EXPOSURE_PARAMS;
    aec_core_get_output_type get_param_output;
    get_param_output.u.core_output = &aec->core_output;
    if (aec->aec_algo_ops.get_parameters) {
      rc = aec->aec_algo_ops.get_parameters(aec->handle, &get_param_info, &get_param_output);
    } else {
      rc = Q3A_CORE_RESULT_FAILED;
    }
    if(rc != Q3A_CORE_RESULT_SUCCESS){
      AEC_ERR("aec get param failed");
      return FALSE;
    }
  }

  return TRUE;
}

static boolean aec_biz_map_set_param(aec_biz_t *aec, aec_set_parameter_t *param)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  aec_core_set_param_type set_param;

  if (!aec || !param) {
    AEC_ERR("invalid input: %p, %p",aec, param)
    return FALSE;
  }

  AEC_LOW("param_type: %d",param->type);
  switch (param->type) {
  case AEC_SET_PARAM_LED_RESET:
    set_param.type = AEC_SET_LED_RESET;
    break;

  case AEC_SET_PARAM_RESET_LED_EST:
    set_param.type = AEC_SET_RESET_LED_EST;
    break;

  case AEC_SET_PARAM_PREPARE_FOR_SNAPSHOT:
    set_param.type = AEC_SET_PREPARE_FOR_SNAPSHOT;
    set_param.u.aec_trigger = param->u.aec_trigger;
    break;

  case AEC_SET_PARAM_PREP_FOR_SNAPSHOT_NOTIFY:
    set_param.type = AEC_SET_PREP_FOR_SNAPSHOT_NOTIFY;
    set_param.u.aec_trigger = param->u.aec_trigger;
    break;

  case AEC_SET_PARAM_DO_LED_EST_FOR_AF:
    set_param.type = AEC_SET_DO_LED_EST_FOR_AF;
    set_param.u.est_for_af = param->u.est_for_af;
    break;

  case AEC_SET_PARAM_EZ_DISABLE:
    set_param.type = AEC_SET_EZ_DISABLE;
    set_param.u.ez_disable = param->u.ez_disable;
    break;

  case AEC_SET_PARAM_EZ_LOCK_OUTPUT:
    set_param.type = AEC_SET_EZ_LOCK_OUTPUT;
    set_param.u.ez_lock_output = param->u.ez_lock_output;
    break;

  case AEC_SET_PARAM_EZ_FORCE_EXP:
    set_param.type = AEC_SET_EZ_FORCE_EXP;
    set_param.u.ez_force_exp = param->u.ez_force_exp;
    break;

  case AEC_SET_PARAM_EZ_FORCE_LINECOUNT:
    set_param.type = AEC_SET_EZ_FORCE_LINECOUNT;
    set_param.u.ez_force_linecount = param->u.ez_force_linecount;
    break;

  case AEC_SET_PARAM_EZ_FORCE_GAIN:
    set_param.type = AEC_SET_EZ_FORCE_GAIN;
    set_param.u.ez_force_gain = param->u.ez_force_gain;
    break;

  case AEC_SET_PARAM_EZ_TEST_ENABLE:
    set_param.type = AEC_SET_EZ_TEST_ENABLE;
    set_param.u.ez_test_enable = param->u.ez_test_enable;
    break;

  case AEC_SET_PARAM_EZ_TEST_ROI:
    set_param.type = AEC_SET_EZ_TEST_ROI;
    set_param.u.ez_test_roi = param->u.ez_test_roi;
    break;

  case AEC_SET_PARAM_EZ_TEST_MOTION:
    set_param.type = AEC_SET_EZ_TEST_MOTION;
    set_param.u.ez_test_motion = param->u.ez_test_motion;
    break;

  case AEC_SET_PARAM_EZ_FORCE_SNAP_EXP:
    set_param.type = AEC_SET_EZ_FORCE_SNAP_EXP;
    set_param.u.ez_force_snap_exp = param->u.ez_force_snap_exp;
    break;

  case AEC_SET_PARAM_EZ_FORCE_SNAP_LINECOUNT:
    set_param.type = AEC_SET_EZ_FORCE_SNAP_LINECOUNT;
    set_param.u.ez_force_snap_linecount = param->u.ez_force_snap_linecount;
    break;

  case AEC_SET_PARAM_EZ_FORCE_SNAP_GAIN:
    set_param.type = AEC_SET_EZ_FORCE_SNAP_GAIN;
    set_param.u.ez_force_snap_gain = param->u.ez_force_snap_gain;
    break;

  case AEC_SET_PARAM_EZ_FORCE_DUAL_LED_IDX:
    set_param.type = AEC_SET_EZ_FORCE_DUAL_LED_IDX;
    set_param.u.ez_force_dual_led_idx = param->u.ez_force_dual_led_idx;
    break;

  case AEC_SET_PARAM_EZ_TUNE_RUNNING:
    set_param.type = AEC_SET_EZ_TUNE_RUNNING;
    set_param.u.ez_running = param->u.ez_running;
    break;

  case AEC_SET_PARAM_LOCK:
    set_param.type = AEC_SET_LOCK;
    set_param.u.aec_lock = param->u.aec_lock;
    break;

  case AEC_SET_PARAM_ADRC_ENABLE:
    set_param.type = AEC_SET_ADRC_ENABLE;
    set_param.u.adrc_enable = param->u.adrc_enable;
    break;

  case AEC_SET_PARAM_INIT_SENSOR_INFO:
    set_param.type = AEC_SET_INIT_SENSOR_INFO;
    STATS_MEMCPY(&aec->sensor_info, sizeof(aec_sensor_info_t),
      &param->u.init_param.sensor_info, sizeof(aec_sensor_info_t));
    STATS_MEMCPY(&set_param.u.init_param, sizeof(aec_set_parameter_init_t),
      &param->u.init_param, sizeof(aec_set_parameter_init_t));
    STATS_MEMSET(&aec->stats_config, 0, sizeof(aec_config_t));
    break;

  case AEC_SET_PARAM_VIDEO_HDR:
    set_param.type = AEC_SET_VIDEO_HDR;
    set_param.u.video_hdr = param->u.video_hdr;
    aec->video_hdr = param->u.video_hdr;
    break;

  case AEC_SET_PARAM_SNAPSHOT_HDR:
    set_param.type = AEC_SET_SNAPSHOT_HDR;
    set_param.u.snapshot_hdr = (aec_core_hdr_type)param->u.snapshot_hdr;
    aec->hdr_type = set_param.u.snapshot_hdr;
    break;

  case AEC_SET_PARAM_HDR_RATIO:
    set_param.type = AEC_SET_HDR_RATIO;
    set_param.u.hdr_ratio = (float)param->u.hdr_ratio;
    AEC_LOW("HDR ratio: %f", set_param.u.hdr_ratio);
    break;

  case AEC_SET_PARAM_BRACKET:
    set_param.type = AEC_SET_BRACKET;
    aec_biz_map_exp_bracket(param->u.aec_bracket, &set_param.u.aec_bracket);
    break;

  case AEC_SET_PARAM_ENABLE:
    set_param.type = AEC_SET_ENABLE;
    set_param.u.aec_enable = param->u.aec_enable;
    break;

  case AEC_SET_PARM_FAST_AEC_DATA:
    if (aec->fast_aec_enable == param->u.fast_aec_data.enable)
      return TRUE;

    set_param.type = AEC_SET_INSTANT_AEC_TYPE;
    set_param.u.instant_aec_type = (param->u.fast_aec_data.enable) ? (AEC_CONVERGENCE_FAST) :
                                      AEC_CONVERGENCE_NORMAL;
    AEC_LOW("Instant AEC type: %d",set_param.u.instant_aec_type);
    aec->fast_aec_enable = param->u.fast_aec_data.enable;
    break;

  case AEC_SET_PARAM_FPS:
    set_param.type = AEC_SET_FPS_RANGE;
    set_param.u.fps_range = param->u.fps;
    break;

  case AEC_SET_PARAM_INIT_EXPOSURE_INDEX:
    set_param.type = AEC_SET_INIT_EXPOSURE_INDEX;
    set_param.u.exp_index = param->u.init_exposure_index;
    break;

  case AEC_SET_PARAM_LONGSHOT_MODE:
    set_param.type = AEC_SET_LONGSHOT_ENABLE;
    set_param.u.longshot_enable = param->u.longshot_mode;
    break;

  case AEC_SET_PARM_INSTANT_AEC_DATA:
    set_param.type = AEC_SET_INSTANT_AEC_TYPE;
    set_param.u.instant_aec_type = param->u.instant_aec_type;
    break;

  case AEC_SET_PARM_DUAL_LED_CALIB_MODE:
    set_param.type = AEC_SET_DUAL_LED_CALIB_MODE;
    set_param.u.dual_led_calib_data.enabled = param->u.led_cal_enable;
    set_param.u.dual_led_calib_data.prefered_exp_index =
      DUAL_LED_CALIB_AEC_PREFERED_EXP_INDEX;
    break;

  case AEC_SET_PARAM_MAX_FLASH_CURRENT:
    set_param.type = AEC_SET_MAX_FLASH_CURRENT;
    set_param.u.max_flash_current = param->u.max_flash_current;
    break;

  case AEC_SET_PARAM_CONFIG_AE_SCAN_TEST:
    set_param.type = AEC_SET_CONFIG_AE_SCAN_TEST;
    set_param.u.ae_scan_test_config.test_config = param->u.ae_scan_test_config.test_config;
    set_param.u.ae_scan_test_config.test_config_freq = param->u.ae_scan_test_config.test_config_freq;
    break;

  case AEC_SET_PARAM_LIGHT_SENSOR_INFO:
    set_param.type = AEC_SET_LIGHT_SENSOR_INFO;
    set_param.u.light_sensor_info = param->u.light_sensor_info;
    break;

  case AEC_SET_PARAM_IR_MODE:
    set_param.type = AEC_SET_IR_MODE;
    set_param.u.ir_mode_type = param->u.ir_mode_type;
    break;

  case AEC_SET_PARAM_CONV_SPEED:
    set_param.type = AEC_SET_CONV_SPEED;
    set_param.u.conv_speed = param->u.conv_speed;
    break;

  case AEC_SET_PARAM_ROLE_SWITCH:
    set_param.type = AEC_SET_ROLE_SWITCH;
    set_param.u.role_switch_params = param->u.role_switch_params;
    break;

  case AEC_SET_PARAM_LED_FD_COMPLETED:
    set_param.type = AEC_SET_LED_FD_COMPLETED;
    aec_biz_map_set_led_fd_roi(aec, &param->u.fd_roi, &set_param.u.fd_roi);
    break;

  case AEC_SET_PARAM_LED_CAL:
    set_param.type = AEC_SET_LED_CAL;
    set_param.u.led_cal_enable = param->u.led_cal_enable;
    if (param->u.led_cal_enable) {
      STATS_MEMSET(&aec->led_cal_data, 0, sizeof(aec->led_cal_data));
    }
    aec->led_cal_data.enable = param->u.led_cal_enable;
    break;

  case AEC_SET_PARAM_SENSOR_FLASH_MODE:
    set_param.type = AEC_SET_SENSOR_FLASH_MODE;
    set_param.u.sensor_flash_mode = param->u.sensor_flash_mode;
    break;

  case AEC_SET_PARAM_EXP_COMPENSATION:
    aec->bestshot_off_data.exp_comp_val = param->u.exp_comp;
    set_param.type = AEC_SET_EXP_COMPENSATION;
    set_param.u.exp_comp = param->u.exp_comp;
    break;

  default:
    AEC_ERR("invalid param type: %d",param->type);
    return FALSE;
  }

  if (aec->aec_algo_ops.set_parameters) {
    rc = aec->aec_algo_ops.set_parameters(aec->handle, &set_param);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }
  if (rc != Q3A_CORE_RESULT_SUCCESS) {
    AEC_ERR("aec set param failed for type=%d", param->type);
    return FALSE;
  }

  return TRUE;
}

/** aec_set_parameters:
 *
 **/
boolean aec_biz_set_param(aec_set_parameter_t *param,
  aec_output_data_t *output, uint8_t num_of_outputs, void *aec_obj)
{
  boolean rc            = TRUE;
  aec_biz_t *aec = (aec_biz_t *)aec_obj;
  (void)num_of_outputs;

  if (!param || !aec || num_of_outputs > MAX_AEC_ALGO_OUTPUTS) {
   rc = FALSE;
   goto done;
  }

  AEC_HIGH("AEC_EVENT: camera_id:%d: %s", param->camera_id,
    aec_biz_set_param_string(param->type));

  switch (param->type) {
  case AEC_SET_PARAM_METERING_MODE:
  case AEC_SET_PARAM_ISO_MODE:
  case AEC_SET_PARAM_ANTIBANDING:
  case AEC_SET_PARAM_BRIGHTNESS_LVL:
  case AEC_SET_PARAM_HJR_AF:
  case AEC_SET_PARAM_ROI:
  case AEC_SET_PARAM_CROP_INFO:
  case AEC_SET_PARAM_FD_ROI:
  case AEC_SET_PARAM_MTR_AREA:
  case AEC_SET_PARAM_ASD_PARM:
  case AEC_SET_PARAM_AFD_PARM:
  case AEC_SET_PARAM_AWB_PARM:
  case AEC_SET_PARAM_GYRO_INFO:
  case AEC_SET_PARAM_LED_MODE:
  case AEC_SET_PARAM_UI_FRAME_DIM:
  case AEC_SET_PARAM_ZSL_OP:
  case AEC_SET_PARAM_CTRL_MODE:
  case AEC_SET_PARAM_SENSOR_ROI:
  case AEC_SET_MANUAL_AUTO_SKIP:
  case AEC_SET_PARAM_STATS_DEPTH:
  case AEC_SET_PARAM_EXP_TIME:
    rc = aec_biz_map_input_info(aec,param);
    break;

  case AEC_SET_PARAM_FPS:
  case AEC_SET_PARAM_LED_RESET:
  case AEC_SET_PARAM_RESET_LED_EST:
  case AEC_SET_PARAM_PREPARE_FOR_SNAPSHOT:
  case AEC_SET_PARAM_PREP_FOR_SNAPSHOT_NOTIFY:
  case AEC_SET_PARAM_DO_LED_EST_FOR_AF:
  case AEC_SET_PARAM_EZ_DISABLE:
  case AEC_SET_PARAM_EZ_LOCK_OUTPUT:
  case AEC_SET_PARAM_EZ_FORCE_EXP:
  case AEC_SET_PARAM_EZ_FORCE_LINECOUNT:
  case AEC_SET_PARAM_EZ_FORCE_GAIN:
  case AEC_SET_PARAM_EZ_TEST_ENABLE:
  case AEC_SET_PARAM_EZ_TEST_ROI:
  case AEC_SET_PARAM_EZ_TEST_MOTION:
  case AEC_SET_PARAM_EZ_FORCE_SNAP_EXP:
  case AEC_SET_PARAM_EZ_FORCE_SNAP_LINECOUNT:
  case AEC_SET_PARAM_EZ_FORCE_SNAP_GAIN:
  case AEC_SET_PARAM_EZ_TUNE_RUNNING:
  case AEC_SET_PARAM_LOCK:
  case AEC_SET_PARAM_INIT_SENSOR_INFO:
  case AEC_SET_PARAM_VIDEO_HDR:
  case AEC_SET_PARAM_SNAPSHOT_HDR:
  case AEC_SET_PARAM_BRACKET:
  case AEC_SET_PARAM_ENABLE:
  case AEC_SET_PARAM_LED_EST:
  case AEC_SET_PARM_FAST_AEC_DATA:
  case AEC_SET_PARAM_ADRC_ENABLE:
  case AEC_SET_PARAM_INIT_EXPOSURE_INDEX:
  case AEC_SET_PARAM_LONGSHOT_MODE:
  case AEC_SET_PARM_INSTANT_AEC_DATA:
  case AEC_SET_PARM_DUAL_LED_CALIB_MODE:
  case AEC_SET_PARAM_MAX_FLASH_CURRENT:
  case AEC_SET_PARAM_CONFIG_AE_SCAN_TEST:
  case AEC_SET_PARAM_LIGHT_SENSOR_INFO:
  case AEC_SET_PARAM_IR_MODE:
  case AEC_SET_PARAM_CONV_SPEED:
  case AEC_SET_PARAM_EZ_FORCE_DUAL_LED_IDX:
  case AEC_SET_PARAM_ROLE_SWITCH:
  case AEC_SET_PARAM_LED_CAL:
  case AEC_SET_PARAM_SENSOR_FLASH_MODE:
  case AEC_SET_PARAM_LED_FD_COMPLETED:
  case AEC_SET_PARAM_HDR_RATIO:
  case AEC_SET_PARAM_EXP_COMPENSATION:
    rc = aec_biz_map_set_param(aec,param);
    break;

  case AEC_SET_PARAM_BESTSHOT:
    rc = aec_biz_set_bestshot_mode(aec, param->u.bestshot_mode);
    break;

  case AEC_SET_PARAM_PACK_OUTPUT:
    aec->sof_id = param->u.current_sof_id;
    aec_biz_pack_output(aec, output);
    break;

  case AEC_SET_PARAM_INIT_CHROMATIX_SENSOR:
    rc = aec_biz_map_init_chromatix_sensor(aec,param);
    break;
  default:
    rc = FALSE;
    break;
  }

done:
  return rc;
}

static void aec_biz_set_debug_data_enable(aec_biz_t *aec)
{
  boolean exif_dbg_enable = FALSE;
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  aec_debug_data_level_type debug_level = AEC_DEBUG_DATA_LEVEL_VERBOSE;

  /* Check whether debug data levels are set. */
  exif_dbg_enable = stats_exif_debug_mask & EXIF_DEBUG_MASK_AEC ? TRUE : FALSE;

  if ((stats_debug_data_log_level & Q3A_DEBUG_DATA_LEVEL_CONCISE) > 0) {
    debug_level = AEC_DEBUG_DATA_LEVEL_CONCISE;
  } else if ((stats_debug_data_log_level & Q3A_DEBUG_DATA_LEVEL_VERBOSE) > 0) {
    debug_level = AEC_DEBUG_DATA_LEVEL_VERBOSE;
  }

  if (aec->exif_dbg_enable != exif_dbg_enable) {
    aec_core_set_param_type set_info;
    set_info.type = AEC_SET_ENABLE_DEBUG_DATA;
    set_info.u.debug_data_enable = exif_dbg_enable;
    if (aec->aec_algo_ops.set_parameters) {
      rc = aec->aec_algo_ops.set_parameters(aec->handle, &set_info);
    } else {
      rc = Q3A_CORE_RESULT_FAILED;
    }
    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AEC_ERR("aec set parameter failed with result %d", rc);
      return;
    }
    aec->exif_dbg_enable = exif_dbg_enable;
  }

  /* Set the EXIF debug level if it has changed and EXIF debugging is enabled. */
  if (aec->exif_dbg_level != debug_level && TRUE == aec->exif_dbg_enable) {
    aec_core_set_param_type set_param_info;

    /* Use AEC_SET_DEBUG_DATA_LEVEL set param to set debug data level. */
    set_param_info.type = AEC_SET_DEBUG_DATA_LEVEL;
    set_param_info.u.debug_data_level = debug_level;

    if (aec->aec_algo_ops.set_parameters) {
      rc = aec->aec_algo_ops.set_parameters(aec->handle, &set_param_info);
    } else {
      rc = Q3A_CORE_RESULT_FAILED;
    }

    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AEC_ERR("aec set param AEC_SET_PARAM_DEBUG_DATA_LEVEL failed with result %d", rc);
      return;
    }

    aec->exif_dbg_level = debug_level;
  }

}


static boolean aec_biz_stats_map(aec_biz_t *aec, const stats_t* stats)
{
  if (!aec || !stats) {
    AEC_ERR("Invalid input: %p,%p",aec, stats);
    return FALSE;
  }

  if(stats->stats_type_mask == (STATS_3HDR | STATS_BG_AEC)) {
    aec->core_input.stats.stats_type_mask = Q3A_CORE_STATS_3HDR | Q3A_CORE_STATS_BAYER;
    aec->core_input.stats.frame_id = stats->frame_id;
    /* map hist stats  */
    q3a_core_3hdr_stats_type* core_hdr3_stats = &aec->core_input.stats.hdr3_stats;
    const stats_3hdr_stats_t* hdr3_stats = &stats->hdr3_stats;
    if(hdr3_stats == NULL){
      AEC_ERR("stats buffer error: hdr3_stats: %p",hdr3_stats);
      return FALSE;
    }

    STATS_MEMCPY(&core_hdr3_stats->stats_hdr3_hist_data, sizeof(stats_hdr3_hist_data_type),
      &hdr3_stats->stats_hdr3_hist_data, sizeof(stats_hdr3_hist_data_type));

    core_hdr3_stats->hdr3_sensor_type = hdr3_stats->hdr3_sensor_type;

    /* map bg stats */
    const q3a_bg_stats_t* q3a_bg_stats = stats->bayer_stats.p_q3a_bg_stats;
    if (q3a_bg_stats == NULL) {
      AEC_ERR("stats buffer error!");
      return FALSE;
    }
    q3a_core_bg_stats_type* core_bg_stats = &aec->core_input.stats.bg_stats;
    core_bg_stats->array_length = MAX_BG_STATS_NUM;
    core_bg_stats->num_horizontal_regions = q3a_bg_stats->bg_region_h_num;
    core_bg_stats->num_vertical_regions = q3a_bg_stats->bg_region_v_num;
    core_bg_stats->region_height = q3a_bg_stats->bg_region_height;
    core_bg_stats->region_width = q3a_bg_stats->bg_region_width;
    core_bg_stats->region_pixel_cnt = q3a_bg_stats->region_pixel_cnt;
    core_bg_stats->r_max = q3a_bg_stats->rMax;
    core_bg_stats->gr_max = q3a_bg_stats->grMax;
    core_bg_stats->gb_max = q3a_bg_stats->gbMax;
    core_bg_stats->b_max = q3a_bg_stats->bMax;
    core_bg_stats->bit_depth = aec->stats_depth;
    core_bg_stats->r_info.channel_sums = q3a_bg_stats->bg_r_sum;
    core_bg_stats->r_info.channel_counts = q3a_bg_stats->bg_r_num;
    core_bg_stats->r_info.channel_sat_sums = q3a_bg_stats->bg_r_sat_sum;
    core_bg_stats->r_info.channel_sat_counts = q3a_bg_stats->bg_r_sat_num;
    core_bg_stats->gr_info.channel_sums = q3a_bg_stats->bg_gr_sum;
    core_bg_stats->gr_info.channel_counts = q3a_bg_stats->bg_gr_num;
    core_bg_stats->gr_info.channel_sat_sums = q3a_bg_stats->bg_gr_sat_sum;
    core_bg_stats->gr_info.channel_sat_counts = q3a_bg_stats->bg_gr_sat_num;
    core_bg_stats->b_info.channel_sums = q3a_bg_stats->bg_b_sum;
    core_bg_stats->b_info.channel_counts = q3a_bg_stats->bg_b_num;
    core_bg_stats->b_info.channel_sat_sums = q3a_bg_stats->bg_b_sat_sum;
    core_bg_stats->b_info.channel_sat_counts = q3a_bg_stats->bg_b_sat_num;
    core_bg_stats->gb_info.channel_sums = q3a_bg_stats->bg_gb_sum;
    core_bg_stats->gb_info.channel_counts = q3a_bg_stats->bg_gb_num;
    core_bg_stats->gb_info.channel_sat_sums = q3a_bg_stats->bg_gb_sat_sum;
    core_bg_stats->gb_info.channel_sat_counts = q3a_bg_stats->bg_gb_sat_num;

  }else if (stats->stats_type_mask & STATS_BG ||
      stats->stats_type_mask & STATS_BG_AEC){
    aec->core_input.stats.stats_type_mask = Q3A_CORE_STATS_BAYER;
    aec->core_input.stats.frame_id = stats->frame_id;

    const q3a_bg_stats_t* q3a_bg_stats = stats->bayer_stats.p_q3a_bg_stats;
    const q3a_bhist_stats_t* q3a_bhist_stats = stats->bayer_stats.p_q3a_bhist_stats;

    if (q3a_bg_stats == NULL) {
      AEC_ERR("stats buffer error!");
      return FALSE;
    }

    /** map bg stats. */
    q3a_core_bg_stats_type* core_bg_stats = &aec->core_input.stats.bg_stats;
    core_bg_stats->array_length = MAX_BG_STATS_NUM;
    core_bg_stats->num_horizontal_regions = q3a_bg_stats->bg_region_h_num;
    core_bg_stats->num_vertical_regions = q3a_bg_stats->bg_region_v_num;
    core_bg_stats->region_height = q3a_bg_stats->bg_region_height;
    core_bg_stats->region_width = q3a_bg_stats->bg_region_width;
    core_bg_stats->region_pixel_cnt = q3a_bg_stats->region_pixel_cnt;
    core_bg_stats->r_max = q3a_bg_stats->rMax;
    core_bg_stats->gr_max = q3a_bg_stats->grMax;
    core_bg_stats->gb_max = q3a_bg_stats->gbMax;
    core_bg_stats->b_max = q3a_bg_stats->bMax;
    core_bg_stats->bit_depth = aec->stats_depth;
    core_bg_stats->r_info.channel_sums = q3a_bg_stats->bg_r_sum;
    core_bg_stats->r_info.channel_counts = q3a_bg_stats->bg_r_num;
    core_bg_stats->r_info.channel_sat_sums = q3a_bg_stats->bg_r_sat_sum;
    core_bg_stats->r_info.channel_sat_counts = q3a_bg_stats->bg_r_sat_num;
    core_bg_stats->gr_info.channel_sums = q3a_bg_stats->bg_gr_sum;
    core_bg_stats->gr_info.channel_counts = q3a_bg_stats->bg_gr_num;
    core_bg_stats->gr_info.channel_sat_sums = q3a_bg_stats->bg_gr_sat_sum;
    core_bg_stats->gr_info.channel_sat_counts = q3a_bg_stats->bg_gr_sat_num;
    core_bg_stats->b_info.channel_sums = q3a_bg_stats->bg_b_sum;
    core_bg_stats->b_info.channel_counts = q3a_bg_stats->bg_b_num;
    core_bg_stats->b_info.channel_sat_sums = q3a_bg_stats->bg_b_sat_sum;
    core_bg_stats->b_info.channel_sat_counts = q3a_bg_stats->bg_b_sat_num;
    core_bg_stats->gb_info.channel_sums = q3a_bg_stats->bg_gb_sum;
    core_bg_stats->gb_info.channel_counts = q3a_bg_stats->bg_gb_num;
    core_bg_stats->gb_info.channel_sat_sums = q3a_bg_stats->bg_gb_sat_sum;
    core_bg_stats->gb_info.channel_sat_counts = q3a_bg_stats->bg_gb_sat_num;
    /** map bhist stats.  */
    if (NULL != q3a_bhist_stats) {
      q3a_core_bhist_stats_type* core_bhist_stats = &aec->core_input.stats.bhist;
      core_bhist_stats->array_size = MAX_BHIST_STATS_NUM;
      core_bhist_stats->hdr_mode = (q3a_core_stats_hdr_type)q3a_bhist_stats->hdr_mode;
      core_bhist_stats->num_bins = q3a_bhist_stats->num_bins;
      core_bhist_stats->num_of_active_channels = 0;

      /* BHist stats are available */
      if (stats->stats_type_mask & STATS_BHISTO)
      {
        /* 8998 HDR MODE USE CASE: On 8998, when HDR mode is enabled, only
        the Y-channel information is present and it is sent as part of the
        R-channel from the ISP. Other channels are invalid */
        if ((TRUE == q3a_bhist_stats->bayer_r_hist_valid) &&
            (FALSE == q3a_bhist_stats->bayer_gr_hist_valid) &&
            (FALSE == q3a_bhist_stats->bayer_gb_hist_valid) &&
            (FALSE == q3a_bhist_stats->bayer_b_hist_valid))
        {
          core_bhist_stats->hist_data[0]      = q3a_bhist_stats->bayer_r_hist;
          core_bhist_stats->hist_data_type[0] = Q3A_CORE_BAYER_HIST_Y;
          core_bhist_stats->num_of_active_channels = 1;
        }

        /* GENERAL USE CASE: Other than the 8998 HDR use case, when BHist
        stats are available, all the four channels are active and contain R,
        Gr, Gb and B information */
        if ((TRUE == q3a_bhist_stats->bayer_r_hist_valid) &&
            (TRUE == q3a_bhist_stats->bayer_gr_hist_valid) &&
            (TRUE == q3a_bhist_stats->bayer_gb_hist_valid) &&
            (TRUE == q3a_bhist_stats->bayer_b_hist_valid))
        {
          core_bhist_stats->hist_data[0]      = q3a_bhist_stats->bayer_r_hist;
          core_bhist_stats->hist_data_type[0] = Q3A_CORE_BAYER_HIST_R;

          core_bhist_stats->hist_data[1]      = q3a_bhist_stats->bayer_gr_hist;
          core_bhist_stats->hist_data_type[1] = Q3A_CORE_BAYER_HIST_GR;

          core_bhist_stats->hist_data[2]      = q3a_bhist_stats->bayer_gb_hist;
          core_bhist_stats->hist_data_type[2] = Q3A_CORE_BAYER_HIST_GB;

          core_bhist_stats->hist_data[3]      = q3a_bhist_stats->bayer_b_hist;
          core_bhist_stats->hist_data_type[3] = Q3A_CORE_BAYER_HIST_B;

          core_bhist_stats->num_of_active_channels = 4;
        }
      }

      /* HDR BHist stats are available */
      else if (stats->stats_type_mask & STATS_HBHISTO)
      {
        /* 8998 USE CASE: On 8998, when HDR BHist stats are available, only
        3 channels are active and contain R, G (Gr) and B information */
        if ((TRUE == q3a_bhist_stats->bayer_r_hist_valid) &&
            (TRUE == q3a_bhist_stats->bayer_gr_hist_valid) &&
            (FALSE == q3a_bhist_stats->bayer_gb_hist_valid) &&
            (TRUE == q3a_bhist_stats->bayer_b_hist_valid))
        {
          core_bhist_stats->hist_data[0]      = q3a_bhist_stats->bayer_r_hist;
          core_bhist_stats->hist_data_type[0] = Q3A_CORE_BAYER_HIST_R;

          core_bhist_stats->hist_data[1]      = q3a_bhist_stats->bayer_gr_hist;
          core_bhist_stats->hist_data_type[1] = Q3A_CORE_BAYER_HIST_G;

          core_bhist_stats->hist_data[2]      = q3a_bhist_stats->bayer_b_hist;
          core_bhist_stats->hist_data_type[2] = Q3A_CORE_BAYER_HIST_B;

          core_bhist_stats->num_of_active_channels = 3;
        }

        /* GENERAL USE CASE: Other than the 8998 use case, when HDR BHist
        stats are available, all the four channels are active and contain R,
        Gr, Gb and B information */
        if ((TRUE == q3a_bhist_stats->bayer_r_hist_valid) &&
            (TRUE == q3a_bhist_stats->bayer_gr_hist_valid) &&
            (TRUE == q3a_bhist_stats->bayer_gb_hist_valid) &&
            (TRUE == q3a_bhist_stats->bayer_b_hist_valid))
        {
          core_bhist_stats->hist_data[0]      = q3a_bhist_stats->bayer_r_hist;
          core_bhist_stats->hist_data_type[0] = Q3A_CORE_BAYER_HIST_R;

          core_bhist_stats->hist_data[1]      = q3a_bhist_stats->bayer_gr_hist;
          core_bhist_stats->hist_data_type[1] = Q3A_CORE_BAYER_HIST_GR;

          core_bhist_stats->hist_data[2]      = q3a_bhist_stats->bayer_gb_hist;
          core_bhist_stats->hist_data_type[2] = Q3A_CORE_BAYER_HIST_GB;

          core_bhist_stats->hist_data[3]      = q3a_bhist_stats->bayer_b_hist;
          core_bhist_stats->hist_data_type[3] = Q3A_CORE_BAYER_HIST_B;

          core_bhist_stats->num_of_active_channels = 4;
        }
      }

      else
      {
        AEC_ERR("ERROR!!! Histogram stats enabled but neither BHist nor HDR BHist"
                " stats are present - mask = %d", stats->stats_type_mask);
        return FALSE;
      }
    }

  }else if(stats->stats_type_mask & STATS_HDR_VID){
    aec->core_input.stats.stats_type_mask = Q3A_CORE_STATS_HDR_VID;
    aec->core_input.stats.frame_id = stats->frame_id;
    /** map yuv stats.  */
    q3a_core_yuv_stats_type* core_yuv_stats = &aec->core_input.stats.yuv_stats;
    const q3a_aec_stats_t* q3a_aec_stats = stats->yuv_stats.p_q3a_aec_stats;
    if(q3a_aec_stats == NULL){
      AEC_ERR("stats buffer error: q3a_aec_stats: %p",q3a_aec_stats);
      return FALSE;
    }
    core_yuv_stats->SY = q3a_aec_stats->SY;
    core_yuv_stats->ae_region_h_num = q3a_aec_stats->ae_region_h_num;
    core_yuv_stats->ae_region_v_num = q3a_aec_stats->ae_region_v_num;
    core_yuv_stats->array_size = MAX_YUV_STATS_NUM;

  } else if (stats->stats_type_mask & STATS_BE) {
    aec->core_input.stats.stats_type_mask = Q3A_CORE_STATS_BE;
    aec->core_input.stats.frame_id = stats->frame_id;

    const q3a_be_stats_t* q3a_be_stats = stats->bayer_stats.p_q3a_be_stats;
    if(NULL == q3a_be_stats){
      AEC_ERR("stats buffer error: q3a_be_stats: %p", q3a_be_stats);
      return FALSE;
    }
    /** map be stats. */
    q3a_core_be_stats_type* core_be_stats = &aec->core_input.stats.be_stats;
    core_be_stats->array_length = MAX_BE_STATS_NUM;
    core_be_stats->num_horizontal_regions   = (uint32)q3a_be_stats->nx;
    core_be_stats->num_vertical_regions     = (uint32)q3a_be_stats->ny;
    core_be_stats->region_width             = q3a_be_stats->sx;
    core_be_stats->region_height            = q3a_be_stats->sy;
    core_be_stats->bit_depth                = aec->stats_depth;
    core_be_stats->r_max                    = q3a_be_stats->rMax;
    core_be_stats->gr_max                   = q3a_be_stats->grMax;
    core_be_stats->gb_max                   = q3a_be_stats->gbMax;
    core_be_stats->b_max                    = q3a_be_stats->bMax;
    core_be_stats->r_info.channel_sums      = q3a_be_stats->r_sum;
    core_be_stats->r_info.channel_counts    = q3a_be_stats->r_count;
    core_be_stats->gr_info.channel_sums     = q3a_be_stats->gr_sum;
    core_be_stats->gr_info.channel_counts   = q3a_be_stats->gr_count;
    core_be_stats->gb_info.channel_sums     = q3a_be_stats->gb_sum;
    core_be_stats->gb_info.channel_counts   = q3a_be_stats->gb_count;
    core_be_stats->b_info.channel_sums      = q3a_be_stats->b_sum;
    core_be_stats->b_info.channel_counts    = q3a_be_stats->b_count;
  } else if (stats->stats_type_mask & STATS_HDR_BE) {
    aec->core_input.stats.stats_type_mask = Q3A_CORE_STATS_BE;
    aec->core_input.stats.frame_id = stats->frame_id;

    const q3a_hdr_be_stats_t* q3a_hdr_be_stats = stats->bayer_stats.p_q3a_hdr_be_stats;
    if(NULL == q3a_hdr_be_stats){
      AEC_ERR("stats buffer error: q3a_hdr_be_stats: %p", q3a_hdr_be_stats);
      return FALSE;
    }
    /** map HDR be stats. */
    q3a_core_be_stats_type* core_be_stats = &aec->core_input.stats.be_stats;
    core_be_stats->array_length = MAX_BE_STATS_NUM;
    core_be_stats->num_horizontal_regions   = (uint32)q3a_hdr_be_stats->be_region_h_num;
    core_be_stats->num_vertical_regions     = (uint32)q3a_hdr_be_stats->be_region_v_num;
    core_be_stats->region_width             = q3a_hdr_be_stats->rgnWidth;
    core_be_stats->region_height            = q3a_hdr_be_stats->rgnHeight;
    core_be_stats->bit_depth                = aec->stats_depth;
    core_be_stats->r_max                    = (uint32)q3a_hdr_be_stats->rMax;
    core_be_stats->gr_max                   = (uint32)q3a_hdr_be_stats->grMax;
    core_be_stats->gb_max                   = (uint32)q3a_hdr_be_stats->gbMax;
    core_be_stats->b_max                    = (uint32)q3a_hdr_be_stats->bMax;
    core_be_stats->r_info.channel_sums      = (int*)q3a_hdr_be_stats->be_r_sum;
    core_be_stats->r_info.channel_counts    = (int*)q3a_hdr_be_stats->be_r_num;
    core_be_stats->gr_info.channel_sums     = (int*)q3a_hdr_be_stats->be_gr_sum;
    core_be_stats->gr_info.channel_counts   = (int*)q3a_hdr_be_stats->be_gr_num;
    core_be_stats->gb_info.channel_sums     = (int*)q3a_hdr_be_stats->be_gb_sum;
    core_be_stats->gb_info.channel_counts   = (int*)q3a_hdr_be_stats->be_gb_num;
    core_be_stats->b_info.channel_sums      = (int*)q3a_hdr_be_stats->be_b_sum;
    core_be_stats->b_info.channel_counts    = (int*)q3a_hdr_be_stats->be_b_num;
  } else {
    AEC_ERR("invalid stats type mask: 0x%x", stats->stats_type_mask);
    return FALSE;
  }

  return TRUE;
}

boolean aec_biz_process(stats_t *stats, void *aec_obj,
  aec_output_data_t *output, uint8_t num_of_outputs)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  aec_biz_t* aec = (aec_biz_t*)aec_obj;
  boolean ret = TRUE;

  if (!aec || !stats || !output || num_of_outputs != MAX_AEC_ALGO_OUTPUTS) {
    AEC_ERR("Invalid input: %p, %p, %p, num_outs: %u", aec, stats, output,
      num_of_outputs);
    return FALSE;
  }

  aec_biz_set_debug_data_enable(aec);
  ret = aec_biz_stats_map(aec, stats);
  if(ret != TRUE){
    AEC_ERR("aec stats mapping failed");
    return FALSE;
  }
  if (aec->aec_algo_ops.process) {
    rc = aec->aec_algo_ops.process(aec->handle, &aec->core_input, &aec->core_output);
    aec_biz_pack_output(aec, output);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }
  if (rc != Q3A_CORE_RESULT_SUCCESS) {
    AEC_ERR("aec core process failed");
    return FALSE;
  }
  aec->core_input.input_info.first_flagged_entry = AEC_INPUT_PARAM_MAX;

  return TRUE;
}

float aec_biz_map_iso_to_real_gain(void *aec_obj, uint32_t iso, uint8_t camera_id)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  uint32 iso_value = 0;
  (void)camera_id;
  aec_biz_t* aec = (aec_biz_t*)aec_obj;
  if(!aec){
    AEC_ERR("Invalid input: %p",aec);
    return 0;
  }

  if(iso < AEC_ISO_100)
    iso = AEC_ISO_100;

  if (iso >= AEC_ISO_MAX){
    iso_value = (uint32)iso;
  } else {
    uint32 temp_iso_multiplier = iso - AEC_ISO_100;
    temp_iso_multiplier = (1 << temp_iso_multiplier);
    iso_value = temp_iso_multiplier * 100;
  }

  aec_core_get_info_type get_param_info;
  get_param_info.param_type = AEC_GET_REAL_GAIN_FROM_ISO;
  get_param_info.u.iso_value = iso_value;
  aec_core_get_output_type get_param_output;
  if (aec->aec_algo_ops.get_parameters) {
    rc = aec->aec_algo_ops.get_parameters(aec->handle, &get_param_info, &get_param_output);
  } else {
    rc = Q3A_CORE_RESULT_FAILED;
  }
  if(rc != Q3A_CORE_RESULT_SUCCESS){
    AEC_ERR("aec get param failed");
    return 0;
  }

  return get_param_output.u.real_gain;
}

/**
 * aec_biz_clear_iface_ops
 *
 * @aec_object: structure with function pointers to be assign
 *
 * Clear interface by setting all pointers to NULL
 *
 * Return: void
 **/
void aec_biz_clear_iface_ops(aec_object_t *aec_object)
{
  if (aec_object) {
    aec_object->set_parameters = NULL;
    aec_object->get_parameters = NULL;
    aec_object->process = NULL;
    aec_object->init = NULL;
    aec_object->deinit = NULL;
    aec_object->iso_to_real_gain = NULL;
    aec_object->get_version = NULL;
  } else {
    AEC_ERR("Invalid input");
  }
  return;
}

/**
 * aec_biz_clear_algo_ops
 *
 * @aec_algo_ops: structure with function pointers to algo lib
 *
 * Clear interface by setting all pointers to NULL
 *
 * Return: void
 **/
void aec_biz_clear_algo_ops(aec_biz_algo_ops_t *aec_algo_ops)
{
  if (aec_algo_ops) {
    aec_algo_ops->set_parameters = NULL;
    aec_algo_ops->get_parameters = NULL;
    aec_algo_ops->process = NULL;
    aec_algo_ops->init = NULL;
    aec_algo_ops->deinit = NULL;
    aec_algo_ops->get_version = NULL;
    aec_algo_ops->print_change_id = NULL;
    aec_algo_ops->set_log = NULL;
  } else {
    AEC_ERR("Invalid input");
  }
  return;
}

/** aec_biz_dlsym
 *
 *    @lib_handler: Handler to library
 *    @fn_ptr: Function to initialize
 *    @fn_name: Name of symbol to find in library
 *
 * Return: TRUE on success
 **/
static boolean aec_biz_dlsym(void *lib_handler, void *fn_ptr,
  const char *fn_name)
{
  char *error = NULL;

  if (NULL == lib_handler || NULL == fn_ptr) {
    AEC_ERR("Error Loading %s", fn_name);
    return FALSE;
  }

  *(void **)(fn_ptr) = dlsym(lib_handler, fn_name);
  if (!fn_ptr) {
    error = (char *)dlerror();
    AEC_ERR("Error: %s", error);
    return FALSE;
  }

  AEC_LOW("Loaded %s %p", fn_name, fn_ptr);
  return TRUE;
}

static void aec_biz_init_data(aec_biz_t *aec)
{
  aec->bestshot_off_data.curr_mode = AEC_BESTSHOT_OFF;
  aec->bestshot_off_data.metering_type = AEC_METERING_CENTER_WEIGHTED;
  aec->bestshot_off_data.exp_comp_val = 0;
  aec->bestshot_off_data.iso_mode = AEC_ISO_MODE_AUTO;
  aec->bestshot_off_data.iso_value = 0;
  aec->prev_bestshot_mode = AEC_BESTSHOT_OFF;
}

void *aec_biz_init(void *libptr)
{
  q3a_core_result_type rc = Q3A_CORE_RESULT_SUCCESS;
  aec_biz_t  *aec_biz = NULL;

  do {
    aec_biz = (aec_biz_t *)calloc(1, sizeof(aec_biz_t));
    if (NULL == aec_biz) {
      AEC_ERR("malloc failed");
      break;
    }
    aec_biz_init_data(aec_biz);

    aec_biz->core_input.input_info.first_flagged_entry = AEC_INPUT_PARAM_MAX;

    dlerror(); /* Clear previous errors */
    if (!aec_biz_dlsym(libptr, &aec_biz->aec_algo_ops.set_parameters,
      "aec_set_param")) {
      break;
    }
    if (!aec_biz_dlsym(libptr, &aec_biz->aec_algo_ops.get_parameters,
      "aec_get_param")) {
      break;
    }
    if (!aec_biz_dlsym(libptr, &aec_biz->aec_algo_ops.process,
      "aec_process")) {
      break;
    }
    if (!aec_biz_dlsym(libptr, &aec_biz->aec_algo_ops.init,
      "aec_init")) {
      break;
    }
    if (!aec_biz_dlsym(libptr, &aec_biz->aec_algo_ops.deinit,
      "aec_deinit")) {
      break;
    }
    if (!aec_biz_dlsym(libptr, &aec_biz->aec_algo_ops.get_version,
      "get_3A_version")) {
      break;
    }
    if (!aec_biz_dlsym(libptr, &aec_biz->aec_algo_ops.print_change_id,
      "q3a_core_print_change_id")) {
      break;
    }

    if (!aec_biz_dlsym(libptr, &aec_biz->aec_algo_ops.set_log,
      "q3a_core_set_log_level")) {
      break;
    }
    /* set logging for core */
    aec_biz->aec_algo_ops.set_log();

   if (aec_biz->aec_algo_ops.print_change_id)
     aec_biz->aec_algo_ops.print_change_id();

    rc = aec_biz->aec_algo_ops.init(&aec_biz->handle);
    if (rc != Q3A_CORE_RESULT_SUCCESS) {
      AEC_ERR("aec core init failed");
      break;
    }

    return aec_biz;
  } while (0);

  /* Handling error */
  aec_biz_clear_algo_ops(&aec_biz->aec_algo_ops);
  if (aec_biz) {
    free(aec_biz);
    aec_biz = NULL;
  }
  return NULL;
}

void aec_biz_destroy(void *aec_obj)
{
  aec_biz_t *aec_biz = (aec_biz_t*)aec_obj;

  if (aec_biz) {
    if (aec_biz->handle && aec_biz->aec_algo_ops.deinit) {
      aec_biz->aec_algo_ops.deinit(aec_biz->handle);
    }
    aec_biz_clear_algo_ops(&aec_biz->aec_algo_ops);
    free(aec_biz);
  }
}

/**
 * aec_biz_get_version
 *
 * @version: Output structure were the version is written
 *
 * Query the AEC library for the version and write data in @version
 *
 * Return: void
 **/
boolean aec_biz_get_version(void *aec_obj, Q3a_version_t *version,
  uint8_t camera_id)
{
  aec_biz_t *aec_biz = (aec_biz_t*)aec_obj;
  (void)camera_id;
  if (!(aec_biz && aec_biz->aec_algo_ops.get_version)) {
    AEC_ERR("error: get version function not available");
    return FALSE;
  }
  /* Get 3A version number */
  aec_biz->aec_algo_ops.get_version((void *)version);

  return TRUE;
}

/**
 * aec_biz_load_function
 *
 * @aec_object: structure with function pointers to be assign
 *
 * Return: Handler to AEC interface library
 **/
void * aec_biz_load_function(aec_object_t *aec_object)
{
  void *q3a_handler = NULL;

  if (!aec_object) {
    return NULL;
  }

  q3a_handler = dlopen("libmmcamera2_q3a_core.so", RTLD_NOW);
  if (!q3a_handler) {
    AEC_ERR("dlerror: %s", dlerror());
    return NULL;
  }

  aec_object->set_parameters = aec_biz_set_param;
  aec_object->get_parameters = aec_biz_get_param;
  aec_object->process = aec_biz_process;
  aec_object->init = aec_biz_init;
  aec_object->deinit = aec_biz_destroy;
  aec_object->iso_to_real_gain = aec_biz_map_iso_to_real_gain;
  aec_object->get_version = aec_biz_get_version;

  return q3a_handler;
}

/**
 * aec_biz_unload_function
 *
 * @aec_object: structure with function pointers to be assign
 * @lib_handler: Handler to the algo library
 *
 * Return: void
 **/
void aec_biz_unload_function(aec_object_t *aec_object, void *lib_handler)
{
  if (lib_handler) {
    dlclose(lib_handler);
  }
  aec_biz_clear_iface_ops(aec_object);

  return;
}
