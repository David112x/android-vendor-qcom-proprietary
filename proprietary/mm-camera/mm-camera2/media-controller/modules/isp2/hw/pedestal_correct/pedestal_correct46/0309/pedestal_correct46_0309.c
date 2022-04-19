/* pedestal_correct46_0309.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "pedestal_correct46.h"
#include "isp_pipeline_reg.h"
void pedestal46_scale_all_tables(chromatix_pedestalcorrection_type *inTbl,
  chromatix_pedestalcorrection_type *outTbl, sensor_rolloff_config_t* sensor_info,
  float hdr_factor)
{
  int i;
  for (i = 0; i < 2; i++) {

#ifdef PEDESTAL_TBL_DEBUG
  ISP_DBG("===== input R table======");
  pedestal46_mesh_sensor_calc_table_debug(&inTbl->pctable[i].channel_black_level_r[0]);
#endif

    pedestal46_scale_table(
          inTbl->pctable[i].channel_black_level_r,
          outTbl->pctable[i].channel_black_level_r,
          sensor_info->full_width,
          sensor_info->full_height,
          sensor_info->output_width,
          sensor_info->output_height,
          sensor_info->offset_x,
          sensor_info->offset_y,
          sensor_info->scale_factor,
          hdr_factor);

#ifdef PEDESTAL_TBL_DEBUG
  ISP_DBG("===== output R table======");
  pedestal46_mesh_sensor_calc_table_debug(&outTbl->pctable[i].channel_black_level_r[0]);

  ISP_DBG("===== input B table======");
  pedestal46_mesh_sensor_calc_table_debug(&inTbl->pctable[i].channel_black_level_b[0]);
#endif

    pedestal46_scale_table(
          inTbl->pctable[i].channel_black_level_b,
          outTbl->pctable[i].channel_black_level_b,
          sensor_info->full_width,
          sensor_info->full_height,
          sensor_info->output_width,
          sensor_info->output_height,
          sensor_info->offset_x,
          sensor_info->offset_y,
          sensor_info->scale_factor,
          hdr_factor);

#ifdef PEDESTAL_TBL_DEBUG
  ISP_DBG("===== output B table======");
  pedestal46_mesh_sensor_calc_table_debug(&outTbl->pctable[i].channel_black_level_b[0]);
  ISP_DBG("===== input GB table======");
  pedestal46_mesh_sensor_calc_table_debug(&inTbl->pctable[i].channel_black_level_gb[0]);
#endif

    pedestal46_scale_table(
          inTbl->pctable[i].channel_black_level_gb,
          outTbl->pctable[i].channel_black_level_gb,
          sensor_info->full_width,
          sensor_info->full_height,
          sensor_info->output_width,
          sensor_info->output_height,
          sensor_info->offset_x,
          sensor_info->offset_y,
          sensor_info->scale_factor,
          hdr_factor);

#ifdef PEDESTAL_TBL_DEBUG
  ISP_DBG("===== output GB table======");
  pedestal46_mesh_sensor_calc_table_debug(&outTbl->pctable[i].channel_black_level_gb[0]);
  ISP_DBG("===== input GR table======");
  pedestal46_mesh_sensor_calc_table_debug(&inTbl->pctable[i].channel_black_level_gr[0]);
#endif

    pedestal46_scale_table(
          inTbl->pctable[i].channel_black_level_gr,
          outTbl->pctable[i].channel_black_level_gr,
          sensor_info->full_width,
          sensor_info->full_height,
          sensor_info->output_width,
          sensor_info->output_height,
          sensor_info->offset_x,
          sensor_info->offset_y,
          sensor_info->scale_factor,
          hdr_factor);

#ifdef PEDESTAL_TBL_DEBUG
  ISP_DBG("===== output GR table======");
  pedestal46_mesh_sensor_calc_table_debug(&outTbl->pctable[i].channel_black_level_gr[0]);
#endif
  }
}

float pedestal_correct46_get_aec_ratio(pedestal46_mod_t *mod, stats_update_t *stats_update) {
  return 1.0;
}

void pedestal_correct46_get_table (pedestal46_mod_t *mod, pedestalcorrection_table *tblOut, int idx) {
    return;
}
