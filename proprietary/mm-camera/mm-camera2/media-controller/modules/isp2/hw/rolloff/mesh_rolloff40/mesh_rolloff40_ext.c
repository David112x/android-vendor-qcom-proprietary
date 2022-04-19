/*
* Copyright (c) 2015 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/* isp headers */
#include "isp_log.h"
#include "mesh_rolloff40.h"
#include "isp_sub_module_util.h"

/** mesh_rolloff40_ext_normalize_table:
 *
 *  @isp_sub_module: isp sub module handle
 *  @mesh_mod: Pointer to rolloff module struct
 *
 *  Get the tables from the chromatix pointer and normalize the values to ensure
 *  all values are > 1
 *
 *  Return void
 **/
static void mesh_rolloff40_ext_normalize_table(isp_sub_module_t *isp_sub_module,
  void *mesh_mod_in)
{
  int                        i = 0, j = 0, k = 0;
  float                      min_value = 1.0, scaling_val = 0;
  chromatix_VFE_common_type *chrComPtr = NULL;
  mesh_rolloff_array_type    inTbl;
  mesh_rolloff_array_type    sensorCalcTable;
  MESH_RollOffTable_V4       downscaledTable;
  MESH_RollOffTable_V4      *outTbl = NULL;
  chromatix_rolloff_type    *chromatix_rolloff = NULL;
  AF_ROLLOFF_CHROMATIX_TYPE *inf_tables = NULL;
  sensor_rolloff_config_t   *sensor_rolloff_config = NULL;
  sensor_out_info_t         *sensor_out_info = NULL;
  sensor_request_crop_t     *request_crop = NULL;
  sensor_dim_output_t       *dim_output = NULL;
  mesh_rolloff40_t          *mesh_mod = NULL;

  if (!isp_sub_module || !mesh_mod_in) {
    ISP_ERR("failed: %p %p", isp_sub_module, mesh_mod);
    return;
  }

  mesh_mod = (mesh_rolloff40_t*)mesh_mod_in;
  chrComPtr = isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!chrComPtr) {
    ISP_ERR("failed: chrComPtr %p", chrComPtr);
    return;
  }

  chromatix_rolloff = &chrComPtr->chromatix_rolloff;
  sensor_out_info = &mesh_mod->sensor_out_info;
  sensor_rolloff_config = &sensor_out_info->sensor_rolloff_config;
  dim_output = &sensor_out_info->dim_output;
  request_crop = &sensor_out_info->request_crop;
  ISP_DBG("sensor_rolloff_config->enable %d", sensor_rolloff_config->enable);
  if (sensor_rolloff_config->enable == 0) {
    sensor_rolloff_config->enable = TRUE;
    sensor_rolloff_config->full_width = dim_output->width;
    sensor_rolloff_config->full_height = dim_output->height;
    sensor_rolloff_config->output_width = request_crop->last_pixel -
      request_crop->first_pixel + 1;
    sensor_rolloff_config->output_height = request_crop->last_line -
      request_crop->first_line + 1;
    sensor_rolloff_config->offset_x = request_crop->first_pixel;
    sensor_rolloff_config->offset_y = request_crop->first_line;
    sensor_rolloff_config->scale_factor = 1;
    ISP_DBG("full %d %d out %d %d offset %d %d scale %d",
      sensor_rolloff_config->full_width, sensor_rolloff_config->full_height,
      sensor_rolloff_config->output_width,
      sensor_rolloff_config->output_height,
      sensor_rolloff_config->offset_x,
      sensor_rolloff_config->offset_y,
      sensor_rolloff_config->scale_factor);
  }

  if (mesh_mod->af_rolloff_info_valid == TRUE) {
    inf_tables = &mesh_mod->af_rolloff_chromatix;
  }

  for (k = 0, min_value = 1.0; k < ISP_ROLLOFF_LENS_POSITION_MAX; k++) {
    for (i = ISP_ROLLOFF_TL84_LIGHT; i < ISP_ROLLOFF_MAX_LIGHT; i++) {
      outTbl = &(mesh_mod->rolloff_tbls.rolloff_tableset[k]->left[i]);

      /* input table from chromatix */
      if (k == ISP_ROLLOFF_LENS_POSITION_INF) {
        if (i == ISP_ROLLOFF_LED_FLASH)
          inTbl = (chromatix_rolloff->chromatix_mesh_rolloff_table_LED);
        else if (i == ISP_ROLLOFF_STROBE_FLASH)
          inTbl = (chromatix_rolloff->chromatix_mesh_rolloff_table_Strobe);
        else if (i == ISP_ROLLOFF_TL84_OUTDOOR_LIGHT)
          inTbl = (chromatix_rolloff->chromatix_mesh_rolloff_table_outdoor);
        else if (i < ROLLOFF_MAX_LIGHT)
          inTbl = (chromatix_rolloff->chromatix_mesh_rolloff_table[i]);
        else
          inTbl = (chromatix_rolloff->
            chromatix_mesh_rolloff_table_lowlight[i % ROLLOFF_MAX_LIGHT]);
      } else if (k == ISP_ROLLOFF_LENS_POSITION_MACRO) {
        if (!inf_tables) /* if not supported skip this normalization */
          break;
        /* get the table from sensor. Currently copied the above code */
        if (i == ISP_ROLLOFF_LED_FLASH)
          inTbl = (inf_tables->chromatix_mesh_rolloff_table_LED);
        else if (i == ISP_ROLLOFF_STROBE_FLASH)
          inTbl = ROLLOFF_CHROMATIX_TABLE_STROBE(inf_tables);
        else if (i == ISP_ROLLOFF_TL84_OUTDOOR_LIGHT)
          inTbl = (chromatix_rolloff->chromatix_mesh_rolloff_table_outdoor);
        else if (i < ROLLOFF_MAX_LIGHT)
          inTbl = (inf_tables->chromatix_mesh_rolloff_table[i]);
        else
          inTbl = (inf_tables->
            chromatix_mesh_rolloff_table_lowlight[i % ROLLOFF_MAX_LIGHT]);
      }

      /* normalize initial mesh rolloff table */
      for (j = 0; j < inTbl.mesh_rolloff_table_size; j++) {
        /* RED Channel */
        if (inTbl.r_gain[j] < min_value)
          min_value = inTbl.gb_gain[j];
        /* GR Channel */
        if (inTbl.gr_gain[j] < min_value)
          min_value = inTbl.gb_gain[j];
        /* BLUE Channel */
        if (inTbl.b_gain[j] < min_value)
          inTbl.b_gain[j] = min_value;
        /* GB Channel */
        if (inTbl.gb_gain[j] < min_value)
          min_value = inTbl.gb_gain[j];
      }
      if (min_value < 1.0) {
        /* Table values in chromatix header should NOT be zero */
        if (min_value == 0.0)
          ISP_ERR("%s: Invalid chromatix value", __func__);

        scaling_val = 1.0 / min_value;

        for (j = 0; j < inTbl.mesh_rolloff_table_size; j++) {
          /* RED Channel */
          inTbl.r_gain[j]  *= scaling_val;
          /* GR Channel */
          inTbl.gr_gain[j] *= scaling_val;
          /* BLUE Channel */
          inTbl.b_gain[j]  *= scaling_val;
          /* GB Channel */
          inTbl.gb_gain[j] *= scaling_val;
        }
      }

      /* calibrate rolloff table by the sensor info
         according to differnt camif setting and offset info from sensor */
      sensorCalcTable = inTbl;
      /* exec algo if sensor config are valid*/
      mesh_rolloff40_sensor_calc_tbl(&inTbl,
        &sensorCalcTable, sensor_rolloff_config);
      for (j = 0; j < MESH_ROLLOFF_SIZE; j++) {
        outTbl->TableB[j] = sensorCalcTable.b_gain[j];
        outTbl->TableR[j] = sensorCalcTable.r_gain[j];
        outTbl->TableGr[j] = sensorCalcTable.gr_gain[j];
        outTbl->TableGb[j] = sensorCalcTable.gb_gain[j];
      }
    }
  }
} /* mesh_rolloff40_ext_normalize_table */

/** mesh_rolloff40_get_awb_cct_type
 *
 *    @trigger:
 *    @chromatix_ptr:
 *
 **/
awb_cct_type mesh_rolloff40_get_awb_cct_type(cct_trigger_info* trigger,
  void *chromatix_ptr)
{
  chromatix_parms_type *p_chromatix = chromatix_ptr;
  awb_cct_type cct_type = AWB_CCT_TYPE_TL84;

  ISP_DBG("%s: CCT %f D65 %f %f, A %f %f, H %f %f", __func__,
    trigger->mired_color_temp,
    trigger->trigger_d65.mired_end,
    trigger->trigger_d65.mired_start,
    trigger->trigger_A.mired_start,
    trigger->trigger_A.mired_end,
    trigger->trigger_H.mired_start,
    trigger->trigger_H.mired_end);

  if (trigger->mired_color_temp <= trigger->trigger_d65.mired_end) {
    cct_type = AWB_CCT_TYPE_D65;
  } else if ((trigger->mired_color_temp > trigger->trigger_d65.mired_end) &&
             (trigger->mired_color_temp <= trigger->trigger_d65.mired_start)) {
    cct_type = AWB_CCT_TYPE_D65_TL84;
  } else if ((trigger->mired_color_temp >= trigger->trigger_A.mired_start) &&
             (trigger->mired_color_temp < trigger->trigger_A.mired_end)) {
    cct_type = AWB_CCT_TYPE_TL84_A;
  } else if ((trigger->mired_color_temp >= trigger->trigger_A.mired_end) &&
             (trigger->mired_color_temp < trigger->trigger_H.mired_start)) {
    cct_type = AWB_CCT_TYPE_A;
  } else if ((trigger->mired_color_temp >= trigger->trigger_H.mired_start) &&
             (trigger->mired_color_temp < trigger->trigger_H.mired_end)) {
    cct_type = AWB_CCT_TYPE_A_H;
  } else if (trigger->mired_color_temp >= trigger->trigger_H.mired_end) {
    cct_type = AWB_CCT_TYPE_H;
  }
  /* else its TL84*/

  return cct_type;
}

/** mesh_rolloff40_calc_awb_outdoor_light_trigger:
 *
 *  @isp_sub_module: isp sub module handle
 *  @mod: Pointer to rolloff module struct
 *  @tblOut: output table after interpolation
 *  @mesh_tbls: Pointer to set of different light tables
 *
 *  Since, there is only one light table, we will copy that.
 *  If more tables are added for outdoor, we can ehnance this
 *  API.
 *
 *  Return void
 **/
static void mesh_rolloff40_calc_awb_outdoor_light_trigger(isp_sub_module_t *isp_sub_module,
  void *data1, void *data2, isp_rolloff_info_t *mesh_tbls)
{

  mesh_rolloff40_t          *mod = NULL;
  MESH_RollOffTable_V4      *tblOut = NULL;

  if (!isp_sub_module || !data1 || !data2 || !mesh_tbls) {
    ISP_ERR("failed: %p %p %p %p", isp_sub_module, data1, data2, mesh_tbls);
    return;
  }

  mod = (mesh_rolloff40_t *)data1;
  tblOut = (MESH_RollOffTable_V4 *)data2;
  *tblOut = mesh_tbls->left[ISP_ROLLOFF_TL84_OUTDOOR_LIGHT];
} /* mesh_rolloff40_calc_awb_outdoor_light_trigger */

/** mesh_rolloff40_ext_calc_aec_ratio
 *
 *    @tunning_type:
 *    @trigger_ptr_low:
 *    @trigger_ptr_outdoor:
 *    @aec_out:
 *    @rgn
 *
 *   Get trigger ratio based on low light and outdoor trigger.
 *   Please note that ratio is the weight of the normal light.
 *    Low     Mix    Normal     Mix       Outdoor
 *    ----|-------|----------|---------|----------
 *   End(372) Start(334)  Start(220)  End(200)
 *
 **/
static float mesh_rolloff40_ext_calc_aec_ratio(unsigned char tunning_type,
  void *trigger_ptr_low, void *trigger_ptr_outdoor, aec_update_t* aec_out, int* rgn)
{
  float light_ratio = 0.0;
  float ratio = 0.0;
  tuning_control_type tunning = (tuning_control_type)tunning_type;
  trigger_point_type *trigger_low = (trigger_point_type *)trigger_ptr_low;
  trigger_point_type *trigger_outdoor = (trigger_point_type *)trigger_ptr_outdoor;
  int region = 0;

  if(!trigger_low || !trigger_outdoor) {
    ISP_ERR("failed: %p %p", trigger_low, trigger_outdoor);
    *rgn = 2;
    return light_ratio;
  }

  switch (tunning) {
  /* 0 is Lux Index based */
  case 0: {
    if(aec_out->lux_idx < trigger_outdoor->lux_index_start) {
      ratio = isp_sub_module_util_calc_interpolation_weight(
      aec_out->lux_idx, trigger_outdoor->lux_index_end, trigger_outdoor->lux_index_start);
      region = 0;
    }
    else if(aec_out->lux_idx > trigger_low->lux_index_start) {
      ratio = isp_sub_module_util_calc_interpolation_weight(
      aec_out->lux_idx, trigger_low->lux_index_start, trigger_low->lux_index_end);
      region = 1;
    }
    else {
      region = 2;
    }
  }
      break;
  /* 1 is Gain Based */
  case 1: {
    if(aec_out->real_gain < trigger_outdoor->gain_start) {
      ratio = isp_sub_module_util_calc_interpolation_weight(
      aec_out->real_gain, trigger_outdoor->gain_end, trigger_outdoor->gain_start);
      region = 0;
    }
    else if(aec_out->real_gain > trigger_low->gain_start) {
      ratio = isp_sub_module_util_calc_interpolation_weight(
      aec_out->real_gain, trigger_low->gain_start, trigger_low->gain_end);
      region = 1;
    }
    else {
      region = 2;
    }
  }
    break;

  default: {
    ISP_ERR("get_trigger_ratio: tunning type %d is not supported",
      tunning);
    region = 2;
    ratio = 0.0;
  }
    break;
  }

  /*ratio is the sitance to low start,
    the smaller distance to low start,
    the higher ratio applied normal light*/
  light_ratio = 1 - ratio;

  if (light_ratio < 0) {
    light_ratio = 0;
  } else if (light_ratio > 1.0) {
    light_ratio = 1.0;
  }

  *rgn = region;
  return light_ratio;
}


/** mesh_rolloff40_ext_calc_interpolation:
 *
 *  @isp_sub_module: isp sub module handle
 *  @mesh_rolloff40: Pointer to rolloff module struct
 *  @mesh_tbls: mesh table
 *  @tblOut: output table after interpolation
 *
 *  Use aec ratio from aec_update to interpolate between normal, low
 *  and outdoor light tables
 *
 *  Return void
 **/
static void mesh_rolloff40_ext_calc_interpolation(isp_sub_module_t *isp_sub_module,
  void *mesh_rolloff40, isp_rolloff_info_t *mesh_tbls,
  MESH_RollOffTable_V4 *tblOut)
{
  float aec_ratio = 0.0;
  chromatix_VFE_common_type *chrComPtr = NULL;
  chromatix_rolloff_type *chromatix_rolloff = NULL;
  chromatix_rolloff_trigger_type *chromatix_rolloff_trigger = NULL;
  chromatix_parms_type *chromatix_ptr = NULL;
  MESH_RollOffTable_V4 meshRolloffTableNormalLight;
  MESH_RollOffTable_V4 meshRolloffTableLowLight;
  MESH_RollOffTable_V4 meshRolloffTableOutdoorLight;
  mesh_rolloff40_t *mod = (mesh_rolloff40_t*)mesh_rolloff40;
  int rgn = 0;

  if (!isp_sub_module || !mod || !mesh_tbls) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, mod, mesh_tbls);
    return;
  }

  if (!isp_sub_module->chromatix_ptrs.chromatixComPtr ||
    !isp_sub_module->chromatix_ptrs.chromatixPtr) {
    ISP_ERR("failed: chromatix %p %p",
      isp_sub_module->chromatix_ptrs.chromatixComPtr,
      isp_sub_module->chromatix_ptrs.chromatixPtr);
    return;
  }

  chrComPtr =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;
  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  chromatix_rolloff = &chrComPtr->chromatix_rolloff;
  chromatix_rolloff_trigger =
    &chromatix_ptr->chromatix_VFE.chromatix_rolloff_trigger;

  mesh_rolloff40_calc_awb_trigger(isp_sub_module, mod,
    &meshRolloffTableNormalLight, mesh_tbls);

  mesh_rolloff40_calc_awb_trigger_lowLight(isp_sub_module,
      mod, &meshRolloffTableLowLight, mesh_tbls);

  mesh_rolloff40_calc_awb_outdoor_light_trigger(isp_sub_module,
        mod, &meshRolloffTableOutdoorLight, mesh_tbls);

  aec_ratio = mesh_rolloff40_ext_calc_aec_ratio(
    chromatix_rolloff_trigger->control_rolloff,
    &(chromatix_rolloff_trigger->rolloff_lowlight_trigger),
    &(chromatix_rolloff_trigger->rolloff_outdoor_trigger),
    &mod->aec_update, &rgn);

  ISP_DBG("aec ratio %f", aec_ratio);

  switch(rgn) {
  case 0: {
    if (F_EQUAL(aec_ratio, 1.0)) {
      ISP_DBG("Outdoor Light ");
      *tblOut = meshRolloffTableOutdoorLight;
    }
    else if (F_EQUAL(aec_ratio, 0.0)) {
        ISP_ERR("We should never be here");
    }
    else {
      ISP_DBG("Interpolate between Outdoor and Low Light ");
      mesh_rolloff40_table_interpolate(&meshRolloffTableOutdoorLight,
       &meshRolloffTableNormalLight, tblOut, aec_ratio);
    }
  }
    break;
  case 1: {
    if (F_EQUAL(aec_ratio, 0.0)) {
      ISP_DBG(" Low Light ");
      *tblOut = meshRolloffTableLowLight;
    }
    else if (F_EQUAL(aec_ratio, 1.0)) {
        ISP_ERR("We should never be here");
    }
    else {
      ISP_DBG("Interpolate between Normal and Low Light ");
      mesh_rolloff40_table_interpolate(&meshRolloffTableNormalLight,
       &meshRolloffTableLowLight, tblOut, aec_ratio);
    }
  }
    break;
  case 2: {
    ISP_DBG("Normal light ");
    *tblOut = meshRolloffTableNormalLight;
  }
    break;
  default:
      ISP_ERR("We should never reach here ");
    break;
  }
} /* mesh_rolloff40_calc_aec_trigger */

static ext_override_func mesh_rolloff_override_func_ext = {
  .calc_interpolation        = mesh_rolloff40_ext_calc_interpolation,
  .normalize                 = mesh_rolloff40_ext_normalize_table
};

boolean mesh_rolloff40_fill_func_table_ext(mesh_rolloff40_t *mesh_rolloff)
{
  mesh_rolloff->ext_func_table = &mesh_rolloff_override_func_ext;
  return TRUE;
} /* mesh_rolloff40_ext_calc_interpolation */
