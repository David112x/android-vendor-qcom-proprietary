/* sce_algo.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_CHROMA_ENHANCE, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_CHROMA_ENHANCE, fmt, ##args)

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "sce40.h"


/** sce40_adjust_dest_triangle_by_adrc
 *
 *  @sce: adrc configuration from Chromatix Header
 *
 * updates the sce triangles based on ADRC configuration.
 *
 **/
boolean sce_adjust_dest_triangle_by_adrc(sce40_t *sce,
  Chromatix_ADRC_SCE_type *adrc_sce_data, aec_update_t aec_update)
{
  boolean            ret = TRUE;
  ADRC_SCE_CCT_type  sce_cct1, sce_cct2;
  chromatix_SCE_type chromatix_SCE_data_1;
  float              trigger, Interp_ratio,
                     trigger_start[MAX_SETS_FOR_ADRC_SCE_ADJ],
                     trigger_end[MAX_SETS_FOR_ADRC_SCE_ADJ];
  int                RegionIdxStrt, RegionIdxEnd, i;
  cct_trigger_info   *trigger_info = NULL;

  for (i = 0; i < MAX_SETS_FOR_ADRC_SCE_ADJ; i++) {
    trigger = aec_update.color_drc_gain;
    trigger_start[i] =
      (float)adrc_sce_data->adrc_sce_core_data[i].drc_gain_trigger.start;
    trigger_end[i] =
      (float)adrc_sce_data->adrc_sce_core_data[i].drc_gain_trigger.end;
  }

  trigger_info = &sce->trigger_info;

  CALC_CCT_TRIGGER_MIRED(trigger_info->trigger_A,
    adrc_sce_data->SCE_A_trigger);

  CALC_CCT_TRIGGER_MIRED(trigger_info->trigger_d65,
    adrc_sce_data->SCE_D65_trigger);

  CALC_CCT_TRIGGER_MIRED(trigger_info->trigger_H,
    adrc_sce_data->SCE_H_trigger);

   sce->cur_cct_type =
     isp_sub_module_util_get_awb_cct_with_H_type(trigger_info);

  isp_sub_module_util_find_region_index_spatial(trigger, trigger_start,
    trigger_end, &Interp_ratio,&RegionIdxStrt, &RegionIdxEnd,
    MAX_SETS_FOR_ADRC_SCE_ADJ);
  sce_cct1 =
    adrc_sce_data->adrc_sce_core_data[RegionIdxStrt].adrc_sce_cct_data;
  sce_cct2 =
    adrc_sce_data->adrc_sce_core_data[RegionIdxEnd].adrc_sce_cct_data;

  switch (sce->cur_cct_type) {
    case AWB_CCT_TYPE_A:{
      if (RegionIdxStrt != RegionIdxEnd){
        trigger_interpolate_sce_triangles_int(&sce_cct2.origin_triangles_A,
          &sce_cct1.origin_triangles_A,
          &sce->origin_triangles_A, Interp_ratio);

        trigger_interpolate_sce_triangles_int(&sce_cct2.destination_triangles_A,
          &sce_cct1.destination_triangles_A,
          &sce->destination_triangles_A, Interp_ratio);

        trigger_interpolate_sce_vectors(&sce_cct2.shift_vector_A,
          &sce_cct1.shift_vector_A,
          &chromatix_SCE_data_1.shift_vector_A, Interp_ratio);
      } else {
        sce_copy_triangle_float(&sce_cct1.origin_triangles_A,
          &sce->origin_triangles_A);
        sce_copy_triangle_float(&sce_cct1.destination_triangles_A,
          &sce->destination_triangles_A);
        chromatix_SCE_data_1.shift_vector_A = sce_cct1.shift_vector_A;
      }
    }
    break;

    case AWB_CCT_TYPE_D65:{
      if (RegionIdxStrt != RegionIdxEnd){
        trigger_interpolate_sce_triangles_int(&sce_cct2.origin_triangles_D65,
          &sce_cct1.origin_triangles_D65,
          &sce->origin_triangles_D65, Interp_ratio);

        trigger_interpolate_sce_triangles_int(
          &sce_cct2.destination_triangles_D65,
          &sce_cct1.destination_triangles_D65,
          &sce->destination_triangles_D65, Interp_ratio);

        trigger_interpolate_sce_vectors(&sce_cct2.shift_vector_D65,
          &sce_cct1.shift_vector_D65,
          &chromatix_SCE_data_1.shift_vector_D65,Interp_ratio);
      } else {
        sce_copy_triangle_float(&sce_cct1.origin_triangles_D65,
          &sce->origin_triangles_D65);
        sce_copy_triangle_float(&sce_cct1.destination_triangles_D65,
          &sce->destination_triangles_D65);
        chromatix_SCE_data_1.shift_vector_D65 = sce_cct1.shift_vector_D65;
      }
    }
    break;

    case AWB_CCT_TYPE_TL84_A:{
      if (RegionIdxStrt != RegionIdxEnd) {
        trigger_interpolate_sce_triangles_int(&sce_cct2.origin_triangles_A,
          &sce_cct1.origin_triangles_A,
          &sce->origin_triangles_A, Interp_ratio);

        trigger_interpolate_sce_triangles_int(&sce_cct2.destination_triangles_A,
          &sce_cct1.destination_triangles_A,
          &sce->destination_triangles_A, Interp_ratio);

        trigger_interpolate_sce_triangles_int(&sce_cct2.origin_triangles_TL84,
          &sce_cct1.origin_triangles_TL84,
          &sce->origin_triangles_TL84, Interp_ratio);

        trigger_interpolate_sce_triangles_int(
          &sce_cct2.destination_triangles_TL84,
          &sce_cct1.destination_triangles_TL84,
          &sce->destination_triangles_TL84, Interp_ratio);

        trigger_interpolate_sce_vectors(&sce_cct2.shift_vector_TL84,
          &sce_cct1.shift_vector_TL84,
          &chromatix_SCE_data_1.shift_vector_TL84, Interp_ratio);

        trigger_interpolate_sce_vectors(&sce_cct2.shift_vector_A,
          &sce_cct1.shift_vector_A,
          &chromatix_SCE_data_1.shift_vector_A, Interp_ratio);
      } else {

        sce_copy_triangle_float(&sce_cct1.origin_triangles_TL84,
          &sce->origin_triangles_TL84);
        sce_copy_triangle_float(&sce_cct1.destination_triangles_TL84,
          &sce->destination_triangles_TL84);

        sce_copy_triangle_float(&sce_cct1.origin_triangles_A,
          &sce->origin_triangles_A);
        sce_copy_triangle_float(&sce_cct1.destination_triangles_A,
          &sce->destination_triangles_A);

        chromatix_SCE_data_1.shift_vector_TL84 = sce_cct1.shift_vector_TL84;
        chromatix_SCE_data_1.shift_vector_A = sce_cct1.shift_vector_A;
      }
    }
    break;

    case AWB_CCT_TYPE_D65_TL84:{
      if (RegionIdxStrt != RegionIdxEnd) {
        trigger_interpolate_sce_triangles_int(&sce_cct2.origin_triangles_D65,
          &sce_cct1.origin_triangles_D65,
          &sce->origin_triangles_D65, Interp_ratio);

        trigger_interpolate_sce_triangles_int(
          &sce_cct2.destination_triangles_D65,
          &sce_cct1.destination_triangles_D65,
          &sce->destination_triangles_D65, Interp_ratio);

        trigger_interpolate_sce_triangles_int(&sce_cct2.origin_triangles_TL84,
          &sce_cct1.origin_triangles_TL84,
          &sce->origin_triangles_TL84, Interp_ratio);

        trigger_interpolate_sce_triangles_int(
          &sce_cct2.destination_triangles_TL84,
          &sce_cct1.destination_triangles_TL84,
          &sce->destination_triangles_TL84, Interp_ratio);

        trigger_interpolate_sce_vectors(&sce_cct2.shift_vector_TL84,
          &sce_cct1.shift_vector_TL84,
          &chromatix_SCE_data_1.shift_vector_TL84, Interp_ratio);

        trigger_interpolate_sce_vectors(&sce_cct2.shift_vector_D65,
          &sce_cct1.shift_vector_D65,
          &chromatix_SCE_data_1.shift_vector_D65,Interp_ratio);
      } else {
        sce_copy_triangle_float(&sce_cct1.origin_triangles_TL84,
          &sce->origin_triangles_TL84);
        sce_copy_triangle_float(&sce_cct1.destination_triangles_TL84,
          &sce->destination_triangles_TL84);

        sce_copy_triangle_float(&sce_cct1.origin_triangles_D65,
          &sce->origin_triangles_D65);
        sce_copy_triangle_float(&sce_cct1.destination_triangles_D65,
          &sce->destination_triangles_D65);

        chromatix_SCE_data_1.shift_vector_TL84 = sce_cct1.shift_vector_TL84;
        chromatix_SCE_data_1.shift_vector_D65 = sce_cct1.shift_vector_D65;
      }
    }
    break;

    case AWB_CCT_TYPE_A_H:{
      if (RegionIdxStrt != RegionIdxEnd){
        trigger_interpolate_sce_triangles_int(&sce_cct2.origin_triangles_A,
          &sce_cct1.origin_triangles_A,
          &sce->origin_triangles_A, Interp_ratio);

        trigger_interpolate_sce_triangles_int(&sce_cct2.destination_triangles_A,
          &sce_cct1.destination_triangles_A,
          &sce->destination_triangles_A, Interp_ratio);

        trigger_interpolate_sce_triangles_int(&sce_cct2.origin_triangles_H,
          &sce_cct1.origin_triangles_H,
          &sce->origin_triangles_H, Interp_ratio);

        trigger_interpolate_sce_triangles_int(&sce_cct2.destination_triangles_H,
          &sce_cct1.destination_triangles_H,
          &sce->destination_triangles_H, Interp_ratio);

        trigger_interpolate_sce_vectors(&sce_cct2.shift_vector_A,
          &sce_cct1.shift_vector_A,
          &chromatix_SCE_data_1.shift_vector_A, Interp_ratio);

        trigger_interpolate_sce_vectors(&sce_cct2.shift_vector_H,
          &sce_cct1.shift_vector_H,
          &chromatix_SCE_data_1.shift_vector_H, Interp_ratio);
      } else {
        sce_copy_triangle_float(&sce_cct1.origin_triangles_A,
          &sce->origin_triangles_A);
        sce_copy_triangle_float(&sce_cct1.destination_triangles_A,
          &sce->destination_triangles_A);

        sce_copy_triangle_float(&sce_cct1.origin_triangles_H,
          &sce->origin_triangles_H);
        sce_copy_triangle_float(&sce_cct1.destination_triangles_H,
          &sce->destination_triangles_H);

        chromatix_SCE_data_1.shift_vector_A = sce_cct1.shift_vector_A;
        chromatix_SCE_data_1.shift_vector_H = sce_cct1.shift_vector_H;
      }
    }
    break;

    case AWB_CCT_TYPE_H:{
      if (RegionIdxStrt != RegionIdxEnd) {
        trigger_interpolate_sce_triangles_int(&sce_cct2.origin_triangles_H,
          &sce_cct1.origin_triangles_H,
          &sce->origin_triangles_H, Interp_ratio);

        trigger_interpolate_sce_triangles_int(&sce_cct2.destination_triangles_H,
          &sce_cct1.destination_triangles_H,
          &sce->destination_triangles_H, Interp_ratio);

        trigger_interpolate_sce_vectors(&sce_cct2.shift_vector_H,
          &sce_cct1.shift_vector_H,
          &chromatix_SCE_data_1.shift_vector_H, Interp_ratio);
      } else {
        sce_copy_triangle_float(&sce_cct1.origin_triangles_H,
          &sce->origin_triangles_H);
        sce_copy_triangle_float(&sce_cct1.destination_triangles_H,
          &sce->destination_triangles_H);

        chromatix_SCE_data_1.shift_vector_H = sce_cct1.shift_vector_H;
      }
    }
    break;

    case AWB_CCT_TYPE_TL84:
    default:{
      if (RegionIdxStrt != RegionIdxEnd) {
        trigger_interpolate_sce_triangles_int(&sce_cct2.origin_triangles_TL84,
          &sce_cct1.origin_triangles_TL84,
          &sce->origin_triangles_TL84, Interp_ratio);

        trigger_interpolate_sce_triangles_int(
          &sce_cct2.destination_triangles_TL84,
          &sce_cct1.destination_triangles_TL84,
          &sce->destination_triangles_TL84, Interp_ratio);

        trigger_interpolate_sce_vectors(&sce_cct2.shift_vector_TL84,
          &sce_cct1.shift_vector_TL84,
          &chromatix_SCE_data_1.shift_vector_TL84, Interp_ratio);
      } else {
        sce_copy_triangle_float(&sce_cct1.origin_triangles_TL84,
          &sce->origin_triangles_TL84);
        sce_copy_triangle_float(&sce_cct1.destination_triangles_TL84,
          &sce->destination_triangles_TL84);

        chromatix_SCE_data_1.shift_vector_TL84 = sce_cct1.shift_vector_TL84;
      }
    }
    break;
  }

  /*CCT interpolate*/
  ret =
    trigger_sce_get_triangles(sce, &chromatix_SCE_data_1, sce->cur_cct_type);
  if (ret == FALSE) {
    ISP_ERR("failed, trigger_sce_get_triangles");
    return FALSE;
  }
  return ret;
}

