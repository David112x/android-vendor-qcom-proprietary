/* Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

/* std headers */
#include <unistd.h>
#include <math.h>

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "isp_pipeline_reg.h"
#include "isp_log.h"
#include "mesh_rolloff44.h"

#ifndef ROLLOFF_CHROMATIX_TABLE_LED2
#define ROLLOFF_CHROMATIX_TABLE_LED2(pchromatix) (pchromatix->chromatix_mesh_rolloff_table_LED2)
#endif

/** mesh_rolloff44_calc_flash_trigger:
 *
 *  @tblNormalLight: Input table with normal light
 *  @tblOut: output table after interpolation
 *  @mesh_tbls: Pointer to set of different light tables
 *
 *  If Flash is ON, use flash_sensitivity values from aec update as ratio
 *  to interpolate between normal light table and flash table
 *
 *  Return void
 **/
void mesh_rolloff44_calc_flash_trigger(isp_sub_module_t *isp_sub_module,
  mesh_rolloff44_t *mod, MESH_RollOffTable_V4 *tblNormalLight,
  MESH_RollOffTable_V4 *tblOut, isp_rolloff_info_t *mesh_tbls)
{
  float                      ratio = 0;
  float                      flash_start = 0, flash_end = 0;
  MESH_RollOffTable_V4      *tblFlash = NULL;
  chromatix_VFE_common_type *chrComPtr = NULL;
  chromatix_rolloff_type    *chromatix_rolloff = NULL;
  flash_sensitivity_t       *flash_sensitivity = NULL;
  float                     led1_value_mainflash = 0.0;
  float                     led2_value_mainflash = 0.0;
  float                     led_mainflash_ratio = 0.0;
  MESH_RollOffTable_V4      *tblLED2Flash = NULL;
  MESH_RollOffTable_V4      *tblLedFlashStored = NULL;
  MESH_RollOffTable_V4      tblDualFlash;

  if (!isp_sub_module || !tblNormalLight || !tblOut || !mesh_tbls) {
    ISP_ERR("failed: %p %p %p %p", isp_sub_module, tblNormalLight, tblOut,
     mesh_tbls);
    return;
  }

  chrComPtr = isp_sub_module->chromatix_ptrs.chromatixComPtr;
  chromatix_rolloff = &chrComPtr->chromatix_rolloff;

  led1_value_mainflash =
    mod->dual_led_setting.led1_high_setting;
  led2_value_mainflash =
    mod->dual_led_setting.led2_high_setting;
  if (led1_value_mainflash > 0 && led2_value_mainflash > 0 ) {
    led_mainflash_ratio =
        (float)led1_value_mainflash/(led1_value_mainflash +
    led2_value_mainflash);
    tblLED2Flash = &(mod->rolloff_tbls.rolloff_tableset[0]->
        left[ISP_ROLLOFF_STROBE_FLASH]);
    tblLedFlashStored = &(mod->rolloff_tbls.rolloff_tableset[0]->
        left[ISP_ROLLOFF_LED_FLASH]);
    mesh_rolloff44_table_interpolate( tblLedFlashStored, tblLED2Flash,
        &tblDualFlash, led_mainflash_ratio);
    tblFlash = &tblDualFlash;
  } else if (led1_value_mainflash == 0) {
      tblFlash = &(mesh_tbls->left[ISP_ROLLOFF_STROBE_FLASH]);
  } else if (led2_value_mainflash == 0) {
      tblFlash = &(mesh_tbls->left[ISP_ROLLOFF_LED_FLASH]);
  } else {
   /* default as LED table */
      tblFlash = &(mesh_tbls->left[ISP_ROLLOFF_LED_FLASH]);
  }
  flash_start = chromatix_rolloff->rolloff_LED_start;
  flash_end = chromatix_rolloff->rolloff_LED_end;

  flash_sensitivity = &mod->aec_update.flash_sensitivity;
  if ((mod->cur_flash_mode == CAM_FLASH_MODE_ON) ||
    (mod->cur_flash_mode == CAM_FLASH_MODE_SINGLE)) {
     if (flash_sensitivity->high != 0) {
       ratio = flash_sensitivity->off / flash_sensitivity->high;
     } else {
       ISP_ERR("invalid sensitivity high %f", flash_sensitivity->high);
       ratio = flash_end;
     }
  } else {
    /* assume flash off. To be changed when AUTO mode is added */
    ratio = flash_start;
  }

  ISP_DBG("<flash_dbg> flash_ratio %f, flash_start %5.2f flash_end %5.2f ",
    ratio, flash_start, flash_end);
  ISP_DBG("off %5.2f high %5.2f ", flash_sensitivity->off,
    flash_sensitivity->high);
  ISP_DBG("flash_start %5.2f flash_end %5.2f ", flash_start, flash_end);

  if (isp_sub_module->dual_led_calibration_enabled == TRUE) {
    *tblOut = *tblFlash;
  } else {
    if (ratio >= flash_end) {
      *tblOut = *tblFlash;
    } else if (ratio <= flash_start) {
      *tblOut = *tblNormalLight;
    } else {
      ratio =    (ratio  - flash_start) / (flash_end - flash_start);
      mesh_rolloff44_table_interpolate( tblFlash, tblNormalLight, tblOut,
          ratio);
    }
  }
} /* mesh_rolloff44_calc_flash_trigger */

#if !OVERRIDE_FUNC
/** mesh_rolloff44_normalize_table:
 *
 *  @isp_sub_module: isp sub module handle
 *  @mesh_mod: Pointer to rolloff module struct
 *
 *  Get the tables from the chromatix pointer and normalize the values to ensure
 *  all values are > 1
 *
 *  Return void
 **/
void mesh_rolloff44_normalize_table(isp_sub_module_t *isp_sub_module,
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
  mesh_rolloff44_t          *mesh_mod = NULL;

  if (!isp_sub_module || !mesh_mod_in) {
    ISP_ERR("failed: %p %p", isp_sub_module, mesh_mod);
    return;
  }

  mesh_mod = (mesh_rolloff44_t*)mesh_mod_in;
  chrComPtr = isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!chrComPtr) {
    ISP_ERR("failed: chrComPtr %p", chrComPtr);
    return;
  }

  chromatix_rolloff = &chrComPtr->chromatix_rolloff;
  sensor_out_info = &mesh_mod->sensor_out_info;
  sensor_rolloff_config = &sensor_out_info->sensor_rolloff_config;
  ISP_DBG("sensor_rolloff_config->enable %d", sensor_rolloff_config->enable);

  isp_sub_module_util_fill_sensor_roll_off_cfg(sensor_out_info,
    sensor_rolloff_config);

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
          inTbl = (chromatix_rolloff->chromatix_mesh_rolloff_table_LED2);
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
          inTbl = ROLLOFF_CHROMATIX_TABLE_LED2(inf_tables);
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
          inTbl.r_gain[j] = min_value;
        /* GR Channel */
        if (inTbl.gr_gain[j] < min_value)
          inTbl.gr_gain[j] = min_value;
        /* BLUE Channel */
        if (inTbl.b_gain[j] < min_value)
          inTbl.b_gain[j] = min_value;
        /* GB Channel */
        if (inTbl.gb_gain[j] < min_value)
          inTbl.gb_gain[j] = min_value;
      }

      /* calibrate rolloff table by the sensor info
         according to differnt camif setting and offset info from sensor */
      sensorCalcTable = inTbl;
      /* exec algo if sensor config are valid*/
      mesh_rolloff44_sensor_calc_tbl(&inTbl,
        &sensorCalcTable, sensor_rolloff_config);
      for (j = 0; j < MESH_ROLLOFF_SIZE; j++) {
        outTbl->TableB[j] = sensorCalcTable.b_gain[j];
        outTbl->TableR[j] = sensorCalcTable.r_gain[j];
        outTbl->TableGr[j] = sensorCalcTable.gr_gain[j];
        outTbl->TableGb[j] = sensorCalcTable.gb_gain[j];
      }
    }
  }
} /* mesh_rolloff44_normalize_table */
#endif
