/* sce40_0309.c
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
static boolean sce40_0309_adjust_dest_triangle_by_aec(sce40_t *sce, float aec_ratio)
{
  sce_cr_cb_triangle_set_float final_dest;
  memset(&final_dest, 0 , sizeof(sce_cr_cb_triangle_set_float));

  trigger_interpolate_sce_triangles(
    &sce->dest, &sce->orig, &final_dest, aec_ratio);
  sce->dest = final_dest;

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
  cct_trigger_info *trigger_info = NULL;
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

  trigger_info = &sce->trigger_info;

  if (trigger_info->mired_color_temp == 0) {
    ISP_ERR("SCE zero color temperature");
    return FALSE;
  }

 aec_ratio = isp_sub_module_util_get_aec_ratio_lowlight(
    chromatix_SCE->control_SCE, &chromatix_SCE->SCE_trigger, &sce->aec_update, 0);

  sce->enable_adrc = isp_sub_module_util_is_adrc_gain_set(
     sce->aec_update.color_drc_gain,
     sce->aec_update.total_drc_gain);

  /* check if update is needed */
  update_sce = ((sce->prev_sce_adj != sce->sce_adjust_factor) ||
    !F_EQUAL(sce->prev_aec_ratio, aec_ratio) ||
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
    ret = sce40_0309_adjust_dest_triangle_by_aec(sce, aec_ratio);
    if (ret == FALSE) {
      ISP_ERR("failed, trigger_sce_get_triangles");
      return FALSE;
    }
  }

  sce->prev_aec_ratio = aec_ratio;
  sce->prev_cct_type = sce->cur_cct_type;
  sce->prev_sce_adj = sce->sce_adjust_factor;
  sce->prev_color_drc_gain = sce->aec_update.color_drc_gain;

  ISP_DBG("new aec ratio %f", aec_ratio);
  ISP_DBG("new cct type %d", sce->cur_cct_type);
  ISP_DBG("new SCE Adj factor: %lf", sce->sce_adjust_factor);

  return TRUE;
} /* sce40_select_table */


void sce40_copy_triangles_from_chromatix(void *data __unused,
  chromatix_SCE_type *p_sce __unused) {
  return;
}
