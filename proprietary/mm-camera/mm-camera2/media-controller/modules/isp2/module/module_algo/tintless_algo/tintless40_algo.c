/* tintless40_algo.c
 *
 * Copyright (c) 2014-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <dlfcn.h>
#include <stdio.h>

/* mctl headers */
#include "mct_event_stats.h"
#include "media_controller.h"
#include "mct_list.h"

/* isp headers */
#include "isp_log.h"
#include "tintless40_algo.h"
#include "isp_pipeline_reg.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#define BUFF_SIZE_255 255


#define TINTLESS_ALGO_NAME "libmmcamera_tintless_algo.so"

static boolean tintless40_algo_execute(mct_module_t *module,
  mct_event_stats_isp_t *stats_data,
  isp_algo_params_t *algo_parm,
  isp_saved_algo_params_t *saved_algo_parm,
  void **output,
  uint32_t curr_frame_id, boolean svhdr_enb);

static boolean tintless40_algo_stop_session(mct_module_t *module,
  isp_saved_algo_params_t *algo_session_params);

/* algorithm structure to be extern-ed */
isp_algo_t algo_tintless40 = {
  "tintless",                                /* name */
  &tintless40_algo_execute,                  /* algo func pointer */
  &tintless40_algo_stop_session,             /* stop session func pointer */
  MCT_EVENT_MODULE_ISP_TINTLESS_ALGO_UPDATE, /* output type */
};

/** tintless40_algo_open:
 *
 *  @tintless_params: tintless params
 *
 *  Open tintless library and link symbols
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean tintless40_algo_open(tintless40_algo_params_t *tintless_params)
{
  char lib_name[BUFF_SIZE_255] = {0};

  if (!tintless_params) {
    ISP_ERR("failed: params %p", tintless_params);
    return FALSE;
  }

  if (tintless_params->lib_handle) {
    ISP_ERR("tintless library already opened");
    return TRUE;
  }

  /* Copy library name in char array */
  strlcpy(lib_name, TINTLESS_ALGO_NAME, BUFF_SIZE_255);

  /* Call dlerror() once to flush out previous dl error */
  dlerror();

  /* Open tintless library */
  tintless_params->lib_handle = dlopen(lib_name, RTLD_NOW);
  if (!tintless_params->lib_handle) {
    ISP_ERR("failed: tintless40_algo_params.lib_handle %p error %s",
      tintless_params->lib_handle, dlerror());
    return FALSE;
  }

  tintless_params->init_func = dlsym(tintless_params->lib_handle,
    "dmlroc_init");
  if (!tintless_params->init_func) {
    ISP_ERR("failed: dmlroc_init for %s %s", lib_name, dlerror());
    goto ERROR;
  }

  tintless_params->update_func = dlsym(tintless_params->lib_handle,
    "dmlroc_entry");
  if (!tintless_params->update_func) {
    ISP_ERR("failed: dmlroc_entry for %s %s", lib_name, dlerror());
    goto ERROR;
  }

  tintless_params->get_version_func = dlsym(tintless_params->lib_handle,
    "dmlroc_get_version");
  if (!tintless_params->get_version_func) {
    ISP_ERR("failed: dmlroc_get_version for %s %s", lib_name, dlerror());
    goto ERROR;
  }

  tintless_params->deinit_func = dlsym(tintless_params->lib_handle,
    "dmlroc_deinit");
  if (!tintless_params->deinit_func) {
    ISP_ERR("failed: dmlroc_deinit for %s %s", lib_name, dlerror());
    goto ERROR;
  }

  return TRUE;

ERROR:
  dlclose(tintless_params->lib_handle);
  tintless_params->lib_handle = NULL;
  memset(&tintless_params->algo_cfg, 0, sizeof(dmlroc_config_t));
  return FALSE;
}

/** tintless40_algo_update_be_stats_config:
 *
 *  @stats_config: stats config handle
 *  @tintless_params: tintless params handle
 *  @is_be_stats_changed: flag to return whether be stats
 *                      changed
 *
 *  Update BE stats config
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean tintless40_algo_update_be_stats_config(
  isp_tintless_stats_config_t *stats_config,
  tintless40_algo_params_t *tintless_params, boolean *is_be_stats_changed)
{
  boolean          ret = TRUE;
  dmlroc_config_t *algo_cfg = NULL;
  uint32_t         stats_config_width = 0, stats_config_height = 0;

  if (!stats_config || !tintless_params || !is_be_stats_changed) {
    ISP_ERR("failed: %p %p %p", stats_config, tintless_params,
      is_be_stats_changed);
    return FALSE;
  }

  if (stats_config->is_valid == FALSE) {
    return TRUE;
  }

  algo_cfg = &tintless_params->algo_cfg;
  stats_config_width = stats_config->num_stat_elem_cols *
    stats_config->stat_elem_w;
  stats_config_height = stats_config->num_stat_elem_rows *
    stats_config->stat_elem_h;
  if (!stats_config->camif_win_w || !stats_config->camif_win_h ||
    !stats_config->stat_elem_w || !stats_config->stat_elem_h ||
    !stats_config->num_stat_elem_rows || !stats_config->num_stat_elem_cols ||
    (stats_config_width > stats_config->camif_win_w) ||
    (stats_config_height > stats_config->camif_win_h)) {
    ISP_ERR("failed: %d %d %d %d %d %d %d %d %d %d", stats_config->camif_win_w,
      stats_config->camif_win_h, stats_config->stat_elem_w,
      stats_config->stat_elem_h, stats_config->num_stat_elem_rows,
      stats_config->num_stat_elem_cols, stats_config_width, stats_config_height,
      stats_config->camif_win_w, stats_config->camif_win_h);
    return FALSE;
  }

  if ((stats_config->camif_win_w != algo_cfg->camif_win_w) ||
    (stats_config->camif_win_h != algo_cfg->camif_win_h) ||
    (stats_config->stat_elem_w != algo_cfg->stat_elem_w) ||
    (stats_config->stat_elem_h != algo_cfg->stat_elem_h)) {
    algo_cfg->camif_win_w = stats_config->camif_win_w;
    algo_cfg->camif_win_h = stats_config->camif_win_h;
    algo_cfg->stat_elem_w = stats_config->stat_elem_w;
    algo_cfg->stat_elem_h = stats_config->stat_elem_h;
    algo_cfg->num_stat_elem_rows = stats_config->num_stat_elem_rows;
    algo_cfg->num_stat_elem_cols = stats_config->num_stat_elem_cols;
    algo_cfg->b_post_stats = 0;
    algo_cfg->stats_saturation_limit = stats_config->saturation_limit;
    algo_cfg->stats_bit_depth = STATS_BIT_DEPTH;
    *is_be_stats_changed = TRUE;
    tintless_params->is_be_stats_config_done = TRUE;
  }

  return TRUE;
}

/** tintless40_algo_update_rolloff_config:
 *
 *  @rolloff_config: rolloff config handle
 *  @tintless_params: tintless params handle
 *  @is_rolloff_config_changed: flag to return whether rolloff
 *                      config changed
 *
 *  Update rolloff config
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean tintless40_algo_update_rolloff_config(
  isp_tintless_mesh_config_t *rolloff_config,
  tintless40_algo_params_t *tintless_params,
  boolean *is_rolloff_config_changed)
{
  boolean          ret = TRUE;
  dmlroc_config_t *algo_cfg = NULL;
  uint32_t         rolloff_config_width = 0, rolloff_config_height = 0;

  if (!rolloff_config || !tintless_params || !is_rolloff_config_changed) {
    ISP_ERR("failed: %p %p %p", rolloff_config, tintless_params,
      is_rolloff_config_changed);
    return FALSE;
  }

  if (rolloff_config->is_valid == FALSE) {
    return TRUE;
  }

  algo_cfg = &tintless_params->algo_cfg;
  rolloff_config_width = rolloff_config->subgrid_width *
    rolloff_config->subgrids * (rolloff_config->num_mesh_elem_cols - 1) + 1;
  rolloff_config_height = rolloff_config->subgrid_height *
    rolloff_config->subgrids * (rolloff_config->num_mesh_elem_rows - 1) + 1;
  if (!rolloff_config->num_mesh_elem_rows ||
    !rolloff_config->num_mesh_elem_cols ||
    !rolloff_config->subgrid_width ||
    !rolloff_config->subgrid_height ||
    !rolloff_config->subgrids ||
    (rolloff_config_width < algo_cfg->camif_win_w) ||
    (rolloff_config_height < algo_cfg->camif_win_h)) {
    ISP_ERR("failed: %d %d %d %d %d %d %d %d %d",
      rolloff_config->num_mesh_elem_rows, rolloff_config->num_mesh_elem_cols,
      rolloff_config->subgrid_width, rolloff_config->subgrid_height,
      rolloff_config->subgrids, rolloff_config_width, rolloff_config_height,
      algo_cfg->camif_win_w, algo_cfg->camif_win_h);
    return FALSE;
  }

  if ((rolloff_config->num_mesh_elem_rows != algo_cfg->num_mesh_elem_rows) ||
    (rolloff_config->num_mesh_elem_cols != algo_cfg->num_mesh_elem_cols) ||
    (rolloff_config->offset_horizontal != algo_cfg->mesh_offset_horizontal) ||
    (rolloff_config->offset_vertical != algo_cfg->mesh_offset_vertical) ||
    (rolloff_config->subgrid_width != algo_cfg->mesh_subgrid_width) ||
    (rolloff_config->subgrid_height != algo_cfg->mesh_subgrid_height) ||
    (rolloff_config->subgrids != algo_cfg->num_mesh_subgrids) ||
    (rolloff_config->update_algo)) {
    algo_cfg->num_mesh_elem_rows = rolloff_config->num_mesh_elem_rows;
    algo_cfg->num_mesh_elem_cols = rolloff_config->num_mesh_elem_cols;
    algo_cfg->mesh_offset_horizontal = rolloff_config->offset_horizontal;
    algo_cfg->mesh_offset_vertical = rolloff_config->offset_vertical;
    algo_cfg->mesh_subgrid_width = rolloff_config->subgrid_width;
    algo_cfg->mesh_subgrid_height = rolloff_config->subgrid_height;
    algo_cfg->num_mesh_subgrids = rolloff_config->subgrids;
    *is_rolloff_config_changed = TRUE;
    tintless_params->is_rolloff_config_done = TRUE;
    rolloff_config->update_algo = 0; //reset, this will be set in next triggerupdate
  }

  return TRUE;
}

/** tintless40_algo_update_tintless_strength_config:
 *
 *  @tintless_strength_params: tintless strength params handle
 *  @tintless_params: tintless params handle
 *  @is_tint_strength_changed: flag to return whether tintless
 *                      config changed
 *
 *  Update tintless strength config
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean tintless40_algo_update_tintless_strength_config(
  isp_tintless_strength_params_t *tintless_strength_params,
  tintless40_algo_params_t *tintless_params, boolean *is_tint_strength_changed)
{
  chromatix_color_tint_correction_type *tint_strength = NULL;
  dmlroc_config_t                      *algo_cfg = NULL;

  if (!tintless_strength_params || !tintless_params ||
    !is_tint_strength_changed) {
    ISP_ERR("failed: %p %p %p", tintless_strength_params, tintless_params,
      is_tint_strength_changed);
    return FALSE;
  }

  tint_strength = &tintless_strength_params->tintless_strength;

  if (tintless_strength_params->is_tintless_strength_valid == FALSE) {
    return TRUE;
  }

  algo_cfg = &tintless_params->algo_cfg;
  if (algo_cfg->tint_correction_strength != tint_strength->tintless_threshold ||
    algo_cfg->tint_accuracy != tint_strength->tintless_high_accuracy_mode) {
    algo_cfg->tint_correction_strength = tint_strength->tintless_threshold;
    ISP_DBG("tint_correction_strength %d", algo_cfg->tint_correction_strength);
    algo_cfg->tint_accuracy = tint_strength->tintless_high_accuracy_mode;
    *is_tint_strength_changed = TRUE;
    tintless_params->is_tintless_strength_config_done = TRUE;
    ISP_DBG("<tintless_dbg> strength %d accuracy %d",
      algo_cfg->tint_correction_strength, algo_cfg->tint_accuracy);
  }
  algo_cfg->tintless_update_delay = tint_strength->tintless_update_delay;
  algo_cfg->tintless_threshold = tint_strength->tintless_threshold;
  return TRUE;
}

/** tintless40_algo_process_be:
 *
 *  @tintless_params: tintless params handle
 *  @be_stats: be stats config
 *  @output_tintless_array: output tintless array handle
 *
 *  Trigger tintless algorithm
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean tintless40_algo_process_be(
  tintless40_algo_params_t *tintless_params,
  q3a_be_stats_t *be_stats, mesh_rolloff_array_type *output_tintless_array)
{
  boolean                      ret = TRUE;
  int32_t                      rc = 0;
  mesh_rolloff_array_type     *input_tintless_array = NULL;

  if (!tintless_params || !be_stats || !output_tintless_array) {
    ISP_ERR("failed: %p %p %p", tintless_params, be_stats,
      output_tintless_array);
    return FALSE;
  }

  input_tintless_array = &tintless_params->tintless_array;

  /* Fill input tintless array */
  dmlroc_mesh_rolloff_array_t  const p_tbl_3a = {
    input_tintless_array->mesh_rolloff_table_size,
    (float *)input_tintless_array->r_gain,
    (float *)input_tintless_array->gr_gain,
    (float *)input_tintless_array->gb_gain,
    (float *)input_tintless_array->b_gain,
  };

  /* Fill output tintless array */
  dmlroc_mesh_rolloff_array_t p_tbl_correction = {
    output_tintless_array->mesh_rolloff_table_size,
    (float *)output_tintless_array->r_gain,
    (float *)output_tintless_array->gr_gain,
    (float *)output_tintless_array->gb_gain,
    (float *)output_tintless_array->b_gain,
  };

  /* Fill stats configuration */
  dmlroc_bayer_stats_info_t pbayer_r = {
    (uint32_t *)be_stats->r_count,
    (uint32_t *)be_stats->r_sum,
    be_stats->nx * be_stats->ny,
  };
  dmlroc_bayer_stats_info_t pbayer_gr = {
    (uint32_t *)be_stats->gr_count,
    (uint32_t *)be_stats->gr_sum,
    be_stats->nx * be_stats->ny,
  };

  dmlroc_bayer_stats_info_t pbayer_gb = {
    (uint32_t *)be_stats->gb_count,
    (uint32_t *)be_stats->gb_sum,
    be_stats->nx * be_stats->ny,
  };

  dmlroc_bayer_stats_info_t pbayer_b = {
    (uint32_t *)be_stats->b_count,
    (uint32_t *)be_stats->b_sum,
    be_stats->nx * be_stats->ny,
  };

  /* Call tintless algorithm */
  rc = tintless_params->update_func(tintless_params->dmlroc_res,
    &pbayer_r, &pbayer_gr, &pbayer_gb, &pbayer_b, NULL, &p_tbl_3a,
    &p_tbl_correction);
  if (rc < 0) {
    ISP_ERR("failed: update_func rc %d", rc);
    ret = FALSE;
  }

  return ret;
}

/** tintless40_algo_process_hdr_be:
 *
 *  @tintless_params: tintless params handle
 *  @be_stats: be stats config
 *  @output_tintless_array: output tintless array handle
 *
 *  Trigger tintless algorithm
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean tintless40_algo_process_hdr_be(
  tintless40_algo_params_t *tintless_params,
  q3a_hdr_be_stats_t *hbe_stats, mesh_rolloff_array_type *output_tintless_array)
{
  boolean                      ret = TRUE;
  int32_t                      rc = 0;
  mesh_rolloff_array_type     *input_tintless_array = NULL;
  uint32_t                     num_rgns;

  if (!tintless_params || !hbe_stats || !output_tintless_array) {
    ISP_ERR("failed: %p %p %p", tintless_params, hbe_stats,
      output_tintless_array);
    return FALSE;
  }

  input_tintless_array = &tintless_params->tintless_array;
  num_rgns = hbe_stats->be_region_h_num * hbe_stats->be_region_v_num;

  /* Fill input tintless array */
  dmlroc_mesh_rolloff_array_t  const p_tbl_3a = {
    input_tintless_array->mesh_rolloff_table_size,
    (float *)input_tintless_array->r_gain,
    (float *)input_tintless_array->gr_gain,
    (float *)input_tintless_array->gb_gain,
    (float *)input_tintless_array->b_gain,
  };

  /* Fill output tintless array */
  dmlroc_mesh_rolloff_array_t p_tbl_correction = {
    output_tintless_array->mesh_rolloff_table_size,
    (float *)output_tintless_array->r_gain,
    (float *)output_tintless_array->gr_gain,
    (float *)output_tintless_array->gb_gain,
    (float *)output_tintless_array->b_gain,
  };

  /* Fill stats configuration */
  dmlroc_bayer_stats_info_t pbayer_r = {
    (uint32_t *)hbe_stats->be_r_num,
    (uint32_t *)hbe_stats->be_r_sum,
    num_rgns,
  };
  dmlroc_bayer_stats_info_t pbayer_gr = {
    (uint32_t *)hbe_stats->be_gr_num,
    (uint32_t *)hbe_stats->be_gr_sum,
    num_rgns,
  };

  dmlroc_bayer_stats_info_t pbayer_gb = {
    (uint32_t *)hbe_stats->be_gb_num,
    (uint32_t *)hbe_stats->be_gb_sum,
    num_rgns,
  };

  dmlroc_bayer_stats_info_t pbayer_b = {
    (uint32_t *)hbe_stats->be_b_num,
    (uint32_t *)hbe_stats->be_b_sum,
    num_rgns,
  };

  ISP_DBG("tintless_dbg input %d output %d", p_tbl_3a.table_size,
    p_tbl_correction.table_size);
  /* Call tintless algorithm */
  rc = tintless_params->update_func(tintless_params->dmlroc_res,
    &pbayer_r, &pbayer_gr, &pbayer_gb, &pbayer_b, NULL, &p_tbl_3a,
    &p_tbl_correction);
  if (rc < 0) {
    ISP_ERR("failed: update_func rc %d", rc);
    ret = FALSE;
  }

  return ret;
}

/** tintless40_algo_dump_config:
 *
 *  @fptr: file pointer
 *  @tintless_params: tintless params
 *
 *  Dump configuration to file
 *
 *  Return void
 **/
static void tintless40_algo_dump_config(FILE *fptr,
  tintless40_algo_params_t *tintless_params)
{
  dmlroc_config_t *algo_cfg = NULL;

  if (!fptr || !tintless_params) {
    ISP_ERR("failed: fptr %p tintless_params %p", fptr, tintless_params);
    return;
  }

  algo_cfg = &tintless_params->algo_cfg;

  /* Print BE stats config */
  fprintf(fptr, "BE stats valid %d\n",
    tintless_params->is_be_stats_config_done);
  fprintf(fptr, "camif_win_w %d\n", algo_cfg->camif_win_w);
  fprintf(fptr, "camif_win_h %d\n", algo_cfg->camif_win_h);
  fprintf(fptr, "stat_elem_w %d\n", algo_cfg->stat_elem_w);
  fprintf(fptr, "stat_elem_h %d\n", algo_cfg->stat_elem_h);
  fprintf(fptr, "num_stat_elem_rows %d\n", algo_cfg->num_stat_elem_rows);
  fprintf(fptr, "num_stat_elem_cols %d\n", algo_cfg->num_stat_elem_cols);
  fprintf(fptr, "b_post_stats %d\n", algo_cfg->b_post_stats);
  fprintf(fptr, "stats_bit_depth %d\n", algo_cfg->stats_bit_depth);
  fprintf(fptr, "stats_saturation_limit %d\n",
    algo_cfg->stats_saturation_limit);

  /* Print rolloff config */
  fprintf(fptr, "rolloff config valid %d\n",
    tintless_params->is_rolloff_config_done);
  fprintf(fptr, "num_mesh_elem_rows %d\n", algo_cfg->num_mesh_elem_rows);
  fprintf(fptr, "num_mesh_elem_cols %d\n", algo_cfg->num_mesh_elem_cols);
  fprintf(fptr, "mesh_offset_horizontal %d\n", algo_cfg->mesh_offset_horizontal);
  fprintf(fptr, "mesh_offset_vertical %d\n", algo_cfg->mesh_offset_vertical);
  fprintf(fptr, "mesh_subgrid_width %d\n", algo_cfg->mesh_subgrid_width);
  fprintf(fptr, "mesh_subgrid_height %d\n", algo_cfg->mesh_subgrid_height);
  fprintf(fptr, "num_mesh_subgrids %d\n", algo_cfg->num_mesh_subgrids);

  /* Print chromatix strength */
  fprintf(fptr, "chromatix params valid %d strength %d\n",
    tintless_params->is_tintless_strength_config_done,
    algo_cfg->tint_correction_strength);
}

/** tintless40_dump_be_stats:
 *
 *  @fptr: file pointer
 *  @be_stats: BE parsed stats
 *  @frame_id: frame id
 *
 *  Dump parsed BE stats in file
 *
 *  Return void
 **/
static void tintless40_dump_be_stats(FILE *fptr, q3a_be_stats_t *be_stats,
  uint32_t frame_id)
{
  int32_t  i = 0;
  uint16_t print_count = MAX_BE_STATS_NUM;

  if (!fptr || !be_stats) {
    ISP_ERR("failed: fptr %p be_stats = %p\n", fptr, be_stats);
    return;
  }

  fprintf(fptr, "BE stats: h_num %d v_num %d\n", be_stats->nx, be_stats->ny);

  if (be_stats != NULL) {
     fprintf(fptr, "==========BE STATS===frame id %d========\n", frame_id);
     for (i = 0;i < print_count; i++) {
       fprintf(fptr, "BE stats: [%d] num: r %5d, b %5d, gr %5d, gb %5d "
         "sum: r %5d, b %5d, gr %5d, gb %5d\n", i,
         be_stats->r_count[i], be_stats->b_count[i],
         be_stats->gr_count[i], be_stats->gb_count[i],
         be_stats->r_sum[i], be_stats->b_sum[i],
         be_stats->gr_sum[i], be_stats->gb_sum[i]);
    }
  }

 return;
} /* tintless40_dump_be_stats */

/** tintless40_dump_hdr_be_stats:
 *
 *  @fptr: file pointer
 *  @be_stats: BE parsed stats
 *  @frame_id: frame id
 *
 *  Dump parsed BE stats in file
 *
 *  Return void
 **/
static void tintless40_dump_hdr_be_stats(FILE *fptr, q3a_hdr_be_stats_t *be_stats,
  uint32_t frame_id)
{
  int32_t  i = 0;
  uint16_t print_count = MAX_BE_STATS_NUM;

  if (!fptr || !be_stats) {
    ISP_ERR("failed: fptr %p be_stats = %p\n", fptr, be_stats);
    return;
  }

  fprintf(fptr, "BE stats: h_num %d v_num %d\n", be_stats->be_region_h_num,
    be_stats->be_region_v_num);

  if (be_stats != NULL) {
     fprintf(fptr, "==========BE STATS===frame id %d========\n", frame_id);
     for (i = 0;i < print_count; i++) {
       fprintf(fptr, "BE stats: [%d] num: r %5d, b %5d, gr %5d, gb %5d "
         "sum: r %5d, b %5d, gr %5d, gb %5d\n", i,
         be_stats->be_r_num[i], be_stats->be_b_num[i],
         be_stats->be_gr_num[i], be_stats->be_gb_num[i],
         be_stats->be_r_sum[i], be_stats->be_b_sum[i],
         be_stats->be_gr_sum[i], be_stats->be_gb_sum[i]);
    }
  }

 return;
} /* tintless40_dump_be_stats */

/** tintless40_dump_tintless_array:
 *
 *  @fptr: file pointer
 *  @data: tintless array to be dumped
 *  @frame_id: frame id
 *
 *  Dump tintless array to file
 *
 *  Return void
 **/
static void tintless40_dump_tintless_array(FILE *fptr,
  mesh_rolloff_array_type *data, uint32_t frame_id)
{
  int32_t i = 0;

  if (!fptr || !data) {
    ISP_ERR("failed: fptr %p data %p", fptr, data);
    return;
  }
  fprintf(fptr, "Frame ID %d", frame_id);
  fprintf(fptr, "==========R===========\n");
  for (i = 0; i < 13 ; i ++) {
    fprintf(fptr, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
      data->r_gain[(i*17) + 0], data->r_gain[(i*17) + 1],
      data->r_gain[(i*17) + 2], data->r_gain[(i*17) + 3],
      data->r_gain[(i*17) + 4], data->r_gain[(i*17) + 5],
      data->r_gain[(i*17) + 6], data->r_gain[(i*17) + 7],
      data->r_gain[(i*17) + 8], data->r_gain[(i*17) + 9],
      data->r_gain[(i*17) + 10], data->r_gain[(i*17) + 11],
      data->r_gain[(i*17) + 12], data->r_gain[(i*17) + 13],
      data->r_gain[(i*17) + 14], data->r_gain[(i*17) + 15],
      data->r_gain[(i*17) + 16]);
  }
  fprintf(fptr, "==========GR=========== \n");
  for (i = 0; i < 13 ; i ++) {
    fprintf(fptr, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
      data->gr_gain[(i*17) + 0], data->gr_gain[(i*17) + 1],
      data->gr_gain[(i*17) + 2], data->gr_gain[(i*17) + 3],
      data->gr_gain[(i*17) + 4], data->gr_gain[(i*17) + 5],
      data->gr_gain[(i*17) + 6], data->gr_gain[(i*17) + 7],
      data->gr_gain[(i*17) + 8], data->gr_gain[(i*17) + 9],
      data->gr_gain[(i*17) + 10], data->gr_gain[(i*17) + 11],
      data->gr_gain[(i*17) + 12], data->gr_gain[(i*17) + 13],
      data->gr_gain[(i*17) + 14], data->gr_gain[(i*17) + 15],
      data->gr_gain[(i*17) + 16]);
   }
  fprintf(fptr, "==========GB=========== \n");
  for (i = 0; i < 13 ; i ++) {
    fprintf(fptr, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
      data->gb_gain[(i*17) + 0], data->gr_gain[(i*17) + 1],
      data->gb_gain[(i*17) + 2], data->gb_gain[(i*17) + 3],
      data->gb_gain[(i*17) + 4], data->gb_gain[(i*17) + 5],
      data->gb_gain[(i*17) + 6], data->gb_gain[(i*17) + 7],
      data->gb_gain[(i*17) + 8], data->gb_gain[(i*17) + 9],
      data->gb_gain[(i*17) + 10], data->gb_gain[(i*17) + 11],
      data->gb_gain[(i*17) + 12], data->gb_gain[(i*17) + 13],
      data->gb_gain[(i*17) + 14], data->gb_gain[(i*17) + 15],
      data->gb_gain[(i*17) + 16]);
   }
  fprintf(fptr, "==========B=========== \n");
   for (i = 0; i < 13 ; i ++) {
    fprintf(fptr, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
      data->b_gain[(i*17) + 0], data->b_gain[(i*17) + 1],
      data->b_gain[(i*17) + 2], data->b_gain[(i*17) + 3],
      data->b_gain[(i*17) + 4], data->b_gain[(i*17) + 5],
      data->b_gain[(i*17) + 6], data->b_gain[(i*17) + 7],
      data->b_gain[(i*17) + 8], data->b_gain[(i*17) + 9],
      data->b_gain[(i*17) + 10], data->b_gain[(i*17) + 11],
      data->b_gain[(i*17) + 12], data->b_gain[(i*17) + 13],
      data->b_gain[(i*17) + 14], data->b_gain[(i*17) + 15],
      data->b_gain[(i*17) + 16]);
  }
  return;
} /* tintless40_dump_tintless_array */

/** tintless40_algo_execute:
 *
 *  @module: mct module handle
 *  @parsed_stats: stats after parsing
 *  @stats_mask: stats mask
 *  @algo_parm: algorithm params
 *  @saved_algo_parm: container to hold the output
 *  @output: actual output payload to be sent
 *
 *  Execute tintless algo
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean tintless40_algo_execute(mct_module_t *module,
  mct_event_stats_isp_t *stats_data, isp_algo_params_t *algo_parm,
  isp_saved_algo_params_t *saved_algo_parm, void **output,
  uint32_t curr_frame_id, boolean svhdr_enb)
{
  boolean                      ret = TRUE, update_algo = FALSE;
  int32_t                      rc = 0;
  isp_tintless_mesh_config_t  *tintless_mesh_config = NULL;
  isp_saved_tintless_params_t *tintless_saved_algo_parm = NULL;
  isp_tintless_stats_config_t *tintless_stats_config = NULL;
  static uint32_t              frame_id = 0, s_counter = 0;
  char                         value[PROPERTY_VALUE_MAX];
  uint32_t                     metadump_enable = 0;
  char                         buf[32];
  FILE                        *fptr = NULL;
  tintless40_algo_params_t    *tintless40_algo_params = NULL;

  /* Validate input parameters */
  if (!module || !algo_parm || !saved_algo_parm || !output) {
    ISP_ERR("failed: %d %p %p %p %p", module, algo_parm,saved_algo_parm, output, svhdr_enb);
    return FALSE;
  }
  if (!curr_frame_id) {
    ISP_ERR("failed: curr_frame_id 0");
    return FALSE;
  }

  *output = NULL;
  tintless_mesh_config = &algo_parm->tintless_mesh_config;
  tintless_saved_algo_parm = &saved_algo_parm->tintless_saved_algo_parm;
  /* skip if no valid stats */
  if ((stats_data->stats_mask & (1 << MSM_ISP_STATS_BE)) ||
       (stats_data->stats_mask & (1 << MSM_ISP_STATS_HDR_BE)))
       tintless_saved_algo_parm->algo_skip_cnt++;//algo_skip_cnt default set 1,then 1st will run the algo ...
  else
    return TRUE;

  if (tintless_saved_algo_parm->tintless_algo_skip > (tintless_saved_algo_parm->algo_skip_cnt))
    return TRUE;

  tintless_saved_algo_parm->algo_skip_cnt = 0;//reset algo_skip_cnt when processing a frame

  /* Skip running Tintless Algo when HDR is enabled. This is because
     in HDR use case when snapshot is taken the frames will be of
     different exposure and the corresponding BE stats should not be
     used for tintless. Hence in HDR use case to avoid tint all the
     rolloff tables need to be tuned */
  if (((stats_data->stats_mask & (1 << MSM_ISP_STATS_BE)) &&
    algo_parm->ae_bracketing_enable) || (algo_parm->manual_control_enable)) {
    ISP_DBG("HDR %d, Manual control %d. Skip running Tintless Algo",
      algo_parm->ae_bracketing_enable, algo_parm->manual_control_enable);
    return TRUE;
  }

  /* Check whether flash mode is on */
  if (tintless_saved_algo_parm->is_flash_mode == TRUE) {
    return TRUE;
  }

  if (stats_data->stats_mask & (1 << MSM_ISP_STATS_BE))
    tintless_stats_config = &algo_parm->tintless_stats_config[MSM_ISP_STATS_BE];
  else if (stats_data->stats_mask & (1 << MSM_ISP_STATS_HDR_BE))
    tintless_stats_config =
      &algo_parm->tintless_stats_config[MSM_ISP_STATS_HDR_BE];

  /* Check whether BE or HDR_BE parsed stats is provided */
  if (!tintless_stats_config)
    return TRUE;

  if (!tintless_saved_algo_parm->tintless_params) {
    tintless_saved_algo_parm->tintless_params =
      CAM_MALLOC(sizeof(tintless40_algo_params_t ));
    if (!tintless_saved_algo_parm->tintless_params) {
      ISP_ERR("failed: tintless_saved_algo_parm->tintless_params %p",
        tintless_saved_algo_parm->tintless_params);
      return FALSE;
    }
    memset(tintless_saved_algo_parm->tintless_params, 0,
      sizeof(tintless40_algo_params_t));
  }

  tintless40_algo_params =
    (tintless40_algo_params_t *)tintless_saved_algo_parm->tintless_params;

  ISP_DBG("be stats valid %d rolloff config valid %d libhandle %p",
    tintless_stats_config->is_valid,
    tintless_mesh_config->is_valid,
    tintless40_algo_params->lib_handle);
  /* Check whether BE stats config or rolloff config is valid */
  if ((tintless_stats_config->is_valid == FALSE) &&
    (tintless_mesh_config->is_valid == FALSE)){
    /* Tintless not enabled */
    return TRUE;
  }

  /* Check whether library is opened already, else open it */
  if (!tintless40_algo_params->lib_handle) {
    ret = tintless40_algo_open(tintless40_algo_params);
    if ((ret == FALSE) || !tintless40_algo_params->lib_handle) {
      ISP_ERR("failed: ret %d tintless40_algo_params.lib_handle %p",
        ret, tintless40_algo_params->lib_handle);
      return FALSE;
    }
  }

  /* Check for change in BE stats config and update algo_cfg */
  ret = tintless40_algo_update_be_stats_config(
    tintless_stats_config,
    tintless40_algo_params, &update_algo);
  if (ret == FALSE) {
    ISP_ERR("failed: tintless40_algo_update_be_stats_config");
  }

  /* Check for change in tintless rolloff config and update algo_cfg */
  ret = tintless40_algo_update_rolloff_config(
    &algo_parm->tintless_mesh_config, tintless40_algo_params,
    &update_algo);
  if (ret == FALSE) {
    ISP_ERR("failed: tintless40_algo_update_rolloff_config");
  }

  /* Check for change in chromatix tintless strength and update algo_cfg */
  ret = tintless40_algo_update_tintless_strength_config(
    &algo_parm->tintless_strength_params, tintless40_algo_params,
    &update_algo);
  if (ret == FALSE) {
    ISP_ERR("failed: tintless40_algo_update_rolloff_config");
  }

  /* if BE stats config / rolloff config / tint strength changed, update algo */
  if (update_algo == TRUE) {
    rc = tintless40_algo_params->init_func(&tintless40_algo_params->dmlroc_res,
      &tintless40_algo_params->algo_cfg);
    if (rc < 0) {
      ISP_ERR("failed: init_func rc %d", rc);
      goto ERROR;
    }
  }

  /* Update tintless array from rolloff module */
  if (tintless_mesh_config->is_tintless_array_valid == TRUE) {
    memcpy(&tintless40_algo_params->tintless_array,
      &tintless_mesh_config->input_tintless_array,
      sizeof(mesh_rolloff_array_type));
    tintless40_algo_params->is_tintless_array_valid = TRUE;
  }

  /* Fill paramaters in structure provided by algo and call
   * update_func
   */
  if ((tintless40_algo_params->is_be_stats_config_done == TRUE) &&
    (tintless40_algo_params->is_rolloff_config_done == TRUE) &&
    (tintless40_algo_params->is_tintless_strength_config_done == TRUE) &&
    (tintless40_algo_params->is_tintless_array_valid == TRUE)) {
    memset(&tintless_saved_algo_parm->tintless_array_param, 0,
      sizeof(mesh_rolloff_array_type));
    tintless_saved_algo_parm->tintless_array_param.mesh_rolloff_table_size =
      MESH_ROLLOFF_TABLE_SIZE;
    if (algo_parm->tintless_stats_config[MSM_ISP_STATS_BE].is_valid)
      ret = tintless40_algo_process_be(tintless40_algo_params,
        (q3a_be_stats_t *)stats_data->stats_data[MSM_ISP_STATS_BE].stats_buf,
        &tintless_saved_algo_parm->tintless_array_param);
    else {
      ret = tintless40_algo_process_hdr_be(tintless40_algo_params,
        (q3a_hdr_be_stats_t *)stats_data->
        stats_data[tintless_stats_config->stats_type].stats_buf,
        &tintless_saved_algo_parm->tintless_array_param);
    }
    if (ret == FALSE) {
      ISP_ERR("failed: tintless40_trigger_algo");
      goto ERROR;
    }
    /* Fill output handle to pass tintless array to rolloff module */
    *output = (void *)tintless_saved_algo_parm;

    /* Check whether setprop is enabled to dump configuration */
    if (tintless40_algo_params->dump_enabled == FALSE) {
       property_get("persist.vendor.camera.dumpmetadata", value, "0");
       tintless40_algo_params->dump_frames = atoi(value);
       tintless40_algo_params->dump_enabled = TRUE;
       if(tintless40_algo_params->dump_frames > 1)
         tintless40_algo_params->dump_frames = 1;
    }
    if (tintless40_algo_params->dump_frames) {
      frame_id++;
      snprintf(buf, sizeof(buf), "/data/dump/tintless_%d.txt", frame_id);
      fptr = fopen(buf, "w+");
      if (!fptr) {
        ISP_ERR("error open files! fptr = %p\n", fptr);
      } else {
        fprintf(fptr, "========== Cfg Start ===========\n");
        tintless40_algo_dump_config(fptr, tintless40_algo_params);
        fprintf(fptr, "========== Cfg End ===========\n");
        fprintf(fptr, "========== BE stats Start ===========\n");
        if (algo_parm->tintless_stats_config[MSM_ISP_STATS_BE].is_valid) {
          tintless40_dump_be_stats(fptr, (q3a_be_stats_t *)stats_data->
            stats_data[MSM_ISP_STATS_BE].stats_buf, frame_id);
        } else {
          tintless40_dump_hdr_be_stats(fptr, (q3a_hdr_be_stats_t *)stats_data->
            stats_data[tintless_stats_config->stats_type].stats_buf, frame_id);
        }
        fprintf(fptr, "========== BE stats End ===========\n");
        fprintf(fptr, "========== input Start ===========\n");
        tintless40_dump_tintless_array(fptr,
          &tintless40_algo_params->tintless_array, frame_id);
        fprintf(fptr, "========== input End ===========\n");
        fprintf(fptr, "========== output Start ===========\n");
        tintless40_dump_tintless_array(fptr,
          &tintless_saved_algo_parm->tintless_array_param, frame_id);
        fprintf(fptr, "========== output End ===========\n");
        fclose(fptr);
        tintless40_algo_params->dump_frames--;
      }
    }
  }

  return TRUE;

ERROR:
  return FALSE;
}

/** tintless40_algo_stop_session:
 *
 *  @module: mct module handle
 *
 *  Call dlclose if tintless library is opened
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean tintless40_algo_stop_session(mct_module_t *module,
  isp_saved_algo_params_t *algo_session_params)
{
  tintless40_algo_params_t *tintless40_algo_params = NULL;

  if (!module || !algo_session_params) {
    ISP_ERR("failed: module %p algo_session_params %p", module,
      algo_session_params);
    return FALSE;
  }

  tintless40_algo_params = (tintless40_algo_params_t *)
    algo_session_params->tintless_saved_algo_parm.tintless_params;
  if (!tintless40_algo_params) {
    return TRUE;
  }

  /* Call close function and call deinit_func */
  if (tintless40_algo_params->lib_handle) {
    tintless40_algo_params->deinit_func(&tintless40_algo_params->dmlroc_res);
    dlclose(tintless40_algo_params->lib_handle);
    tintless40_algo_params->lib_handle = NULL;
  }
  memset(&tintless40_algo_params->algo_cfg, 0, sizeof(dmlroc_config_t));
  free(tintless40_algo_params);
  algo_session_params->tintless_saved_algo_parm.tintless_params = NULL;

  return TRUE;
}
