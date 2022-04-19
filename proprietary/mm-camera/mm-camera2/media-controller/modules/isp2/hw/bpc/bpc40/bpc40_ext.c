/* bpc40_ext.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


/* isp headers */
#include "isp_log.h"
#include "bpc40.h"
#include "isp_sub_module_util.h"

/** bpc40_get_min_max:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bpc40_get_min_max(void *data1,
  void *data2, void *data3)
{
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  uint8_t normal_light_idx = (MAX_LIGHT_TYPES_FOR_SPATIAL/2);
  bpc40_t              *bpc = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  bpc = (bpc40_t *)data1;
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
  bpc->Fmin = chromatix_BPC->chromatix_bpc_bcc_data[bpc->start_index].bpc_Fmin;
  bpc->Fmax = chromatix_BPC->chromatix_bpc_bcc_data[bpc->start_index].bpc_Fmax;
  bpc->Fmin_lowlight = chromatix_BPC->chromatix_bpc_bcc_data[bpc->end_index].bpc_Fmin;
  bpc->Fmax_lowlight = chromatix_BPC->chromatix_bpc_bcc_data[bpc->end_index].bpc_Fmax;
  bpc->bpc_normal_input_offset =
    chromatix_BPC->chromatix_bpc_bcc_data[bpc->start_index].bpc_4_offset;
  bpc->bpc_lowlight_input_offset =
    chromatix_BPC->chromatix_bpc_bcc_data[bpc->end_index].bpc_4_offset;
  ISP_DBG("bpc->bpc_lowlight_input_offset %p", bpc->bpc_lowlight_input_offset);
  return TRUE;
}

/** bpc4_get_init_min_max:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bpc40_get_init_min_max(void *data1,
  void *data2, void *data3)
{
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  uint8_t normal_light_idx = (MAX_LIGHT_TYPES_FOR_SPATIAL/2);
  bpc40_t              *bpc = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  bpc = (bpc40_t *)data1;
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
  bpc->p_params.p_input_offset =
    &(chromatix_BPC->chromatix_bpc_bcc_data[normal_light_idx].bpc_4_offset);
  bpc->p_params.p_Fmin =
    &(chromatix_BPC->chromatix_bpc_bcc_data[normal_light_idx].bpc_Fmin);
  bpc->p_params.p_Fmax =
    &(chromatix_BPC->chromatix_bpc_bcc_data[normal_light_idx].bpc_Fmax);
  return TRUE;
}

/** bpc40_get_trigger_ratio:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bpc40_get_trigger_ratio(void *data1,
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
  bpc40_t              *bpc = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  bpc = (bpc40_t *)data1;
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
      trigger = bpc->aec_update.lux_idx;
      } else if (tunning_control == 1) {
      trigger_start = chromatix_BPC->chromatix_bpc_bcc_data[i].bpc_bcc_trigger.gain_start;
      trigger_end = chromatix_BPC->chromatix_bpc_bcc_data[i].bpc_bcc_trigger.gain_end;
      trigger = bpc->aec_update.real_gain;
      } else {
      ISP_ERR("%s: Unexpected tunning control: %d\n", __func__,
        tunning_control);
      return -1;
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

  if (!F_EQUAL(trigger_ratio.ratio, bpc->aec_ratio) ||
    ((bpc->start_index != startIdx) || (bpc->end_index != endIdx))) {
    isp_sub_module->trigger_update_pending = TRUE;
   }

  bpc->start_index = startIdx;
  bpc->end_index = endIdx;
  bpc->aec_ratio = trigger_ratio.ratio;
  return TRUE;
}

/** bpc40_check_enable_index:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bpc40_check_enable_index(void *data1,
  void *data2, void *data3)
{
  boolean                       ret = TRUE,
                                enable = TRUE;
  uint32_t                      i = 0;
  chromatix_parms_type         *chromatix_ptr = NULL;
  aec_update_t                 *aec_update = NULL;
  float                         trigger_value[SENSOR_BPC_REGIONS];
  Sensor_BPC_type              *sensor_bpc_data = NULL;
  isp_sub_module_output_t      *output;
  bpc40_t                      *bpc = NULL;
  isp_sub_module_t             *isp_sub_module = NULL;

  if (!data1 || !data2 || !data3) {
    ISP_ERR("failed: %p %p %p", data1, data2, data3);
    return FALSE;
  }
  bpc = (bpc40_t *)data1;
  isp_sub_module = (isp_sub_module_t *)data2;
  output = (isp_sub_module_output_t *)data3;

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  sensor_bpc_data = &chromatix_ptr->sensor_bpc_data;
  aec_update = &bpc->aec_update;

  /* Decision to enable the bpc */
  for (i = 0; i < (SENSOR_BPC_REGIONS - 1); i++) {
    if (sensor_bpc_data->bpc_control[i]) {
      /*Gain trigger == 1*/
      trigger_value[i] = aec_update->real_gain;
      ISP_DBG("trigger_value[%d] %f", i, trigger_value[i]);
    } else {
      /*Lux index trigger*/
      trigger_value[i] = aec_update->lux_idx;
      ISP_DBG("trigger_value[%d] %f", i, trigger_value[i]);
    }
  }

  if (trigger_value[0] < sensor_bpc_data->Threshold[0]) {
    if (sensor_bpc_data->Q_bpc[0] == 0) {
      enable = FALSE;
    } else {
      enable = TRUE;
    }
  } else if (trigger_value[1] < sensor_bpc_data->Threshold[1]) {
    if (sensor_bpc_data->Q_bpc[1] == 0) {
      enable = FALSE;
    } else {
      enable = TRUE;
    }
  } else if (trigger_value[2] < sensor_bpc_data->Threshold[2]) {
    if (sensor_bpc_data->Q_bpc[2] == 0) {
      enable = FALSE;
    } else {
      enable = TRUE;
    }
  } else if (trigger_value[3] < sensor_bpc_data->Threshold[3]) {
    if (sensor_bpc_data->Q_bpc[3] == 0) {
      enable = FALSE;
    } else {
      enable = TRUE;
    }
  } else if (trigger_value[4] < sensor_bpc_data->Threshold[4]) {
    if (sensor_bpc_data->Q_bpc[4] == 0) {
      enable = FALSE;
    } else {
      enable = TRUE;
    }
  } else {
    if (sensor_bpc_data->Q_bpc[5] == 0) {
      enable = FALSE;
    } else {
      enable = TRUE;
    }
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
} /* bpc40_check_enable_index */

static ext_override_func bpc_override_func_ext = {
  .check_enable_idx  = bpc40_check_enable_index,
  .get_trigger_ratio = bpc40_get_trigger_ratio,
  .get_init_min_max  = bpc40_get_init_min_max,
  .get_min_max       = bpc40_get_min_max,
};

boolean bpc40_fill_func_table_ext(bpc40_t *bpc)
{
  bpc->ext_func_table = &bpc_override_func_ext;
  return TRUE;
} /* bpc40_fill_func_table */
