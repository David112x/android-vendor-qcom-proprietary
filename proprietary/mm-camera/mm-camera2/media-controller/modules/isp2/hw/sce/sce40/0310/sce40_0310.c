/* sce40_0310.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* isp headers */
#include "sce40.h"
#include "sce_algo.h"
#include "isp_sub_module_util.h"
#include "isp_sub_module_log.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_SCE, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_SCE, fmt, ##args)

static void sce40_0310_print_orig_triangle(int i,
  cr_cb_triangle_float *triangle)
{
  ISP_DBG("camera_trigger_SkinColorEnhancement::INFO: tNormOrig[%d].point1.cb = %f\n", i, triangle->point1.cb);
  ISP_DBG("camera_trigger_SkinColorEnhancement::INFO: tNormOrig[%d].point1.cr = %f\n", i, triangle->point1.cr);
  ISP_DBG("camera_trigger_SkinColorEnhancement::INFO: tNormOrig[%d].point2.cb = %f\n", i, triangle->point2.cb);
  ISP_DBG("camera_trigger_SkinColorEnhancement::INFO: tNormOrig[%d].point2.cr = %f\n", i, triangle->point2.cr);
  ISP_DBG("camera_trigger_SkinColorEnhancement::INFO: tNormOrig[%d].point3.cb = %f\n", i, triangle->point3.cb);
  ISP_DBG("camera_trigger_SkinColorEnhancement::INFO: tNormOrig[%d].point3.cr = %f\n", i, triangle->point3.cr);
}

static void sce40_0310_print_dest_triangle(int i,
  cr_cb_triangle_float *triangle)
{
  ISP_DBG("camera_trigger_SkinColorEnhancement::INFO: tNormDest[%d].point1.cb = %f\n", i, triangle->point1.cb);
  ISP_DBG("camera_trigger_SkinColorEnhancement::INFO: tNormDest[%d].point1.cr = %f\n", i, triangle->point1.cr);
  ISP_DBG("camera_trigger_SkinColorEnhancement::INFO: tNormDest[%d].point2.cb = %f\n", i, triangle->point2.cb);
  ISP_DBG("camera_trigger_SkinColorEnhancement::INFO: tNormDest[%d].point2.cr = %f\n", i, triangle->point2.cr);
  ISP_DBG("camera_trigger_SkinColorEnhancement::INFO: tNormDest[%d].point3.cb = %f\n", i, triangle->point3.cb);
  ISP_DBG("camera_trigger_SkinColorEnhancement::INFO: tNormDest[%d].point3.cr = %f\n", i, triangle->point3.cr);
}

static void sce_debug_table(sce_cr_cb_triangle_set_float *orig,
  sce_cr_cb_triangle_set_float *dest, sce_shift_vector *interp_vector __unused)
{
  if (!orig || !dest || !interp_vector) {
    return;
  }

  sce40_0310_print_orig_triangle(0, &orig->triangle1);
  sce40_0310_print_orig_triangle(1, &orig->triangle2);
  sce40_0310_print_orig_triangle(2, &orig->triangle3);
  sce40_0310_print_orig_triangle(3, &orig->triangle4);
  sce40_0310_print_orig_triangle(4, &orig->triangle5);

  sce40_0310_print_dest_triangle(0, &dest->triangle1);
  sce40_0310_print_dest_triangle(1, &dest->triangle2);
  sce40_0310_print_dest_triangle(2, &dest->triangle3);
  sce40_0310_print_dest_triangle(3, &dest->triangle4);
  sce40_0310_print_dest_triangle(4, &dest->triangle5);
}

/** sce40_adjust_dest_triangle_by_aec
 *
 *  @isp_sub_module: isp sub module handle
 *  @sce: sce handle
 *
 *  Select sce table based on lighting condition and AWB
 *  decision
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean sce40_0310_adjust_dest_triangle_by_aec(sce40_t *sce,
  chromatix_parms_type *pchromatix __unused, trigger_ratio_t trigger_ratio)
{

  switch (trigger_ratio.lighting) {
     case TRIGGER_OUTDOOR:
       trigger_interpolate_sce_triangles(
         &(sce->orig),
         &(sce->origin_triangles_outdoor),
         &(sce->orig),
         trigger_ratio.ratio);

       trigger_interpolate_sce_triangles(
         &(sce->dest),
         &(sce->destination_triangles_outdoor),
         &(sce->dest),
         trigger_ratio.ratio);

       break;
     case TRIGGER_LOWLIGHT:
     case TRIGGER_NORMAL:
       trigger_interpolate_sce_triangles(
         &sce->dest, &sce->orig, &sce->dest, trigger_ratio.ratio);
       break;
     default:
       ISP_DBG("Lighting condition not specified");
       break;
  }

  return TRUE;
}

/** sce40_select_table
 *
 *  @isp_sub_module: isp sub module handle
 *  @sce: sce handle
 *
 *  Select sce table based on lighting condition and AWB
 *  decision
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean sce40_select_table(isp_sub_module_t *isp_sub_module,
  void *data)
{
  uint8_t update_sce = FALSE;
  chromatix_parms_type *pchromatix;
  chromatix_SCE_type *chromatix_SCE;
  chromatix_black_level_type *pchromatix_black_level;
  chromatix_gamma_type *chromatix_gamma_ptr = NULL;
  cct_trigger_info *trigger_info = NULL;
  trigger_ratio_t trigger_ratio;
  trigger_point_type *outdoor_trigger_point = NULL;
  boolean ret = TRUE;
  float aec_ratio = 0.0;
  Chromatix_ADRC_SCE_type *adrc_sce_data;
  sce40_t *sce = NULL;

  if (!data || !isp_sub_module) {
    ISP_ERR("failed: %p %p", data, isp_sub_module);
    return FALSE;
  }

  sce = (sce40_t *)data;
  pchromatix = isp_sub_module->chromatix_ptrs.chromatixPtr;

  if (!pchromatix) {
    ISP_ERR("%s: Null Pointer: %p\n", __func__, pchromatix);
    return FALSE;
  }

  chromatix_SCE = &pchromatix->chromatix_VFE.chromatix_SCE;
  adrc_sce_data = &pchromatix->chromatix_VFE.chromatix_adrc_sce_data;
  chromatix_gamma_ptr = &pchromatix->chromatix_VFE.chromatix_gamma;
  outdoor_trigger_point =
    &(GAMMA_OUTDOOR_TRIGGER(chromatix_gamma_ptr));
  trigger_info = &sce->trigger_info;

  if (trigger_info->mired_color_temp == 0) {
    ISP_ERR("SCE zero color temperature");
    return FALSE;
  }

  /* Decide the trigger ratio for current lighting condition,
     fill in trigger ratio*/
  ret = isp_sub_module_util_get_aec_ratio_bright_low(
    chromatix_SCE->control_SCE,
    outdoor_trigger_point, &chromatix_SCE->SCE_trigger,
    &sce->aec_update, &trigger_ratio, 0);
  if (ret != TRUE) {
    ISP_DBG("get aec ratio error");
    return TRUE;
  }

  aec_ratio = trigger_ratio.ratio;
  ISP_ERR("aec_ratio %f", aec_ratio);

  sce->enable_adrc = isp_sub_module_util_is_adrc_gain_set(
     sce->aec_update.color_drc_gain,
     sce->aec_update.total_drc_gain);

  /* check if update is needed */
  update_sce = ((sce->prev_sce_adj != sce->sce_adjust_factor) ||
    !F_EQUAL(sce->prev_aec_ratio, aec_ratio) ||
    (sce->prev_lighting != trigger_ratio.lighting) ||
    (sce->prev_cct_type != sce->cur_cct_type) ||
    (sce->enable_adrc &&
     !F_EQUAL(sce->prev_color_drc_gain, sce->aec_update.color_drc_gain)));

  if (!update_sce) {
    ISP_DBG("no updates on SCE table");
    return TRUE;
  }

  if (chromatix_SCE->sce_enable &&
      sce->enable_adrc &&
      adrc_sce_data->adrc_sce_adj_enable) {
    ret = sce_adjust_dest_triangle_by_adrc(sce, adrc_sce_data, sce->aec_update);
    if (ret == FALSE) {
      ISP_ERR("failed, sce_adjust_dest_triangle_by_adrc");
      return FALSE;
    }
  } else {
    ret = trigger_sce_get_triangles(sce, chromatix_SCE, sce->cur_cct_type);
    if (ret == FALSE) {
      ISP_ERR("failed, trigger_sce_get_triangles");
      return FALSE;
    }
    ret = sce40_0310_adjust_dest_triangle_by_aec(sce, pchromatix,
      trigger_ratio);
    if (ret == FALSE) {
      ISP_ERR("failed, trigger_sce_get_triangles");
      return FALSE;
    }
    sce_debug_table(&sce->orig, &sce->dest, &sce->interp_vector);
  }

  sce->prev_aec_ratio = aec_ratio;
  sce->prev_lighting = trigger_ratio.lighting;
  sce->prev_cct_type = sce->cur_cct_type;
  sce->prev_sce_adj = sce->sce_adjust_factor;
  sce->prev_color_drc_gain = sce->aec_update.color_drc_gain;

  ISP_DBG("new aec ratio %f", aec_ratio);
  ISP_DBG("new cct type %d", sce->cur_cct_type);
  ISP_DBG("new SCE Adj factor: %lf", sce->sce_adjust_factor);

  return TRUE;
} /* sce40_select_table */

void sce40_copy_triangles_from_chromatix(void *data,
  chromatix_SCE_type *p_sce)
{
  sce40_t *sce = NULL;
  if (!data || !p_sce) {
    ISP_ERR("failed: %p %p", data, p_sce);
    return;
  }

  sce = (sce40_t *)data;
  sce_copy_triangle_float(&p_sce->origin_triangles_outdoor,
    &sce->origin_triangles_outdoor);
  sce_copy_triangle_float(&p_sce->destination_triangles_outdoor,
    &sce->destination_triangles_outdoor);
}
