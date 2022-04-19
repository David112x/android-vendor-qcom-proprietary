/* bcc40_ext.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


/* isp headers */
#include "isp_log.h"
#include "bcc40.h"
#include "isp_sub_module_util.h"

/** bcc40_get_min_max:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bcc40_get_min_max(void *data1,
  void *data2, void *data3)
{
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  uint8_t normal_light_idx = (MAX_LIGHT_TYPES_FOR_SPATIAL/2);
  bcc40_t              *bcc = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  bcc = (bcc40_t *)data1;
  isp_sub_module = (isp_sub_module_t *)data2;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_BPC = &chromatix_ptr->chromatix_VFE.chromatix_BPC;
  if (!chromatix_BPC) {
    ISP_ERR("failed: chromatix_BPC %p", chromatix_BPC);
    return FALSE;
  }

  if (((bcc->start_index >= 0) && (bcc->start_index < MAX_LIGHT_TYPES_FOR_SPATIAL)) &&
      ((bcc->end_index >= 0) && (bcc->end_index < MAX_LIGHT_TYPES_FOR_SPATIAL))) {
    bcc->Fmin = chromatix_BPC->chromatix_bpc_bcc_data[bcc->start_index].bcc_Fmin;
    bcc->Fmax = chromatix_BPC->chromatix_bpc_bcc_data[bcc->start_index].bcc_Fmax;
    bcc->Fmin_lowlight = chromatix_BPC->chromatix_bpc_bcc_data[bcc->end_index].bcc_Fmin;
    bcc->Fmax_lowlight = chromatix_BPC->chromatix_bpc_bcc_data[bcc->end_index].bcc_Fmax;
    bcc->bcc_normal_input_offset =
      chromatix_BPC->chromatix_bpc_bcc_data[bcc->start_index].bcc_4_offset;
    bcc->bcc_lowlight_input_offset =
      chromatix_BPC->chromatix_bpc_bcc_data[bcc->end_index].bcc_4_offset;
  }
  return TRUE;
}

/** bcc40_get_min_max:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bcc40_get_init_min_max(void *data1,
  void *data2, void *data3)
{
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  uint8_t normal_light_idx = (MAX_LIGHT_TYPES_FOR_SPATIAL/2);
  bcc40_t              *bcc = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  bcc = (bcc40_t *)data1;
  isp_sub_module = (isp_sub_module_t *)data2;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_BPC = &chromatix_ptr->chromatix_VFE.chromatix_BPC;
  if (!chromatix_BPC) {
    ISP_ERR("failed: chromatix_BPC %p", chromatix_BPC);
    return FALSE;
  }
  bcc->p_params.p_input_offset =
    &(chromatix_BPC->chromatix_bpc_bcc_data[normal_light_idx].bcc_4_offset);
  bcc->p_params.Fmin =
    chromatix_BPC->chromatix_bpc_bcc_data[normal_light_idx].bcc_Fmin;
  bcc->p_params.Fmax =
    chromatix_BPC->chromatix_bpc_bcc_data[normal_light_idx].bcc_Fmax;
  return TRUE;
}

/** bcc40_get_trigger_ratio:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bcc40_get_trigger_ratio(void *data1,
  void *data2, void *data3)
{
  trigger_ratio_t trigger_ratio;
  tuning_control_type tunning_control;
  int i =0;
  uint8_t startIdx, endIdx;
  float trigger_start;
  float trigger_end;
  float trigger;
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  float                 aec_ratio = 0.0;
  bcc40_t              *bcc = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  bcc = (bcc40_t *)data1;
  isp_sub_module = (isp_sub_module_t *)data2;


  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_BPC = &chromatix_ptr->chromatix_VFE.chromatix_BPC;
  if (!chromatix_BPC) {
    ISP_ERR("failed: chromatix_BPC %p", chromatix_BPC);
    return FALSE;
  }
  tunning_control = chromatix_BPC->control_bpc_bcc;
  trigger_ratio.ratio = 0;
  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
    if (tunning_control == 0) {
      trigger_start = chromatix_BPC->chromatix_bpc_bcc_data[i].bpc_bcc_trigger.lux_index_start;
      trigger_end = chromatix_BPC->chromatix_bpc_bcc_data[i].bpc_bcc_trigger.lux_index_end;
      trigger = bcc->aec_update.lux_idx;
      } else if (tunning_control == 1) {
      trigger_start = chromatix_BPC->chromatix_bpc_bcc_data[i].bpc_bcc_trigger.gain_start;
      trigger_end = chromatix_BPC->chromatix_bpc_bcc_data[i].bpc_bcc_trigger.gain_end;
      trigger = bcc->aec_update.sensor_gain;
      } else {
      ISP_ERR("%s: Unexpected tunning control: %d\n", __func__,
        tunning_control);
      return FALSE;
      }
      if (i == (MAX_LIGHT_TYPES_FOR_SPATIAL-1)) {
        startIdx = (MAX_LIGHT_TYPES_FOR_SPATIAL-1);
        endIdx = (MAX_LIGHT_TYPES_FOR_SPATIAL-1);
        trigger_ratio.lighting = (MAX_LIGHT_TYPES_FOR_SPATIAL-1);
        trigger_ratio.ratio = 1.0;
        break;
        } else if (trigger <= trigger_start) {
        startIdx = i;
        endIdx = i;
        trigger_ratio.lighting = i;
        trigger_ratio.ratio = 0.0;
        break;
        } else if (trigger < trigger_end) {
        startIdx = i;
        endIdx = i+1;
        trigger_ratio.lighting = i;
        trigger_ratio.ratio = 1.0 - (float)(trigger-trigger_start)/
        (float)(trigger_end-trigger_start);
        break;
        }
    }

  if (trigger_ratio.ratio > 1.0) {
    ISP_ERR("%s: Unexpected trigger ratio: %f\n", __func__,
      trigger_ratio.ratio);
    return FALSE;
  }

  if (!F_EQUAL(trigger_ratio.ratio, bcc->aec_ratio) ||
    ((bcc->start_index != startIdx) || (bcc->end_index != endIdx))) {
    isp_sub_module->trigger_update_pending = TRUE;
   }

  bcc->start_index = startIdx;
  bcc->end_index = endIdx;
  bcc->aec_ratio = trigger_ratio.ratio;
  return TRUE;
}

/** bcc40_check_enable_index:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bcc40_check_enable_index(void *data1,
  void *data2, void *data3)
{
  boolean                       ret = TRUE,
                                enable = TRUE;
  chromatix_parms_type         *chromatix_ptr = NULL;
  aec_update_t                 *aec_update = NULL;
  float                         trigger_value = 0.0f;
  float                         BCC_Enable_Index = 0.0f;
  isp_sub_module_output_t      *output;
  bcc40_t                      *bcc = NULL;
  isp_sub_module_t             *isp_sub_module = NULL;

  if (!data1 || !data2 || !data3) {
    ISP_ERR("failed: %p %p %p", data1, data2, data3);
    return FALSE;
  }
  bcc = (bcc40_t *)data1;
  isp_sub_module = (isp_sub_module_t *)data2;
  output = (isp_sub_module_output_t *)data3;

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }
  BCC_Enable_Index = chromatix_ptr->BCC_Enable_Index;
  aec_update = &bcc->aec_update;
  trigger_value = aec_update->lux_idx;
  ISP_DBG("trigger_value %f chromatix BCC_Enable_Index %f", trigger_value,
    BCC_Enable_Index);

  if (trigger_value > BCC_Enable_Index) {
    enable = TRUE;
  } else {
    enable = FALSE;
  }

  if (isp_sub_module->submod_enable != enable) {
    isp_sub_module->submod_enable = enable;
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    ret = isp_sub_module_util_update_module_bit(isp_sub_module, output);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_sub_module_util_update_module_bit");
    }
    PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  }
  return ret;
} /* bcc40_check_enable_index */

static ext_override_func bcc_override_func_ext = {
  .check_enable_idx  = bcc40_check_enable_index,
  .get_trigger_ratio = bcc40_get_trigger_ratio,
  .get_init_min_max  = bcc40_get_init_min_max,
  .get_min_max       = bcc40_get_min_max,
};

boolean bcc40_fill_func_table_ext(bcc40_t *bcc)
{
  bcc->ext_func_table = &bcc_override_func_ext;
  return TRUE;
} /* bcc40_fill_func_table */
