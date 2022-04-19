/* mesh_rolloff40_309.c
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
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
#include "mesh_rolloff40.h"

/** mesh_rolloff40_calc_flash_trigger:
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
void mesh_rolloff40_calc_flash_trigger(isp_sub_module_t *isp_sub_module,
  mesh_rolloff40_t *mod, MESH_RollOffTable_V4 *tblNormalLight,
  MESH_RollOffTable_V4 *tblOut, isp_rolloff_info_t *mesh_tbls)
{
  float                      ratio = 0;
  float                      p_ratio = 0;
  float                      flash_start = 0, flash_end = 0;
  MESH_RollOffTable_V4      *tblFlash = NULL;
  chromatix_VFE_common_type *chrComPtr = NULL;
  chromatix_rolloff_type    *chromatix_rolloff = NULL;
  flash_sensitivity_t       *flash_sensitivity = NULL;
  float                     led1_value_mainflash = 0.0;
  float                     led2_value_mainflash = 0.0;
  float                     led_mainflash_ratio = 0.0;
  MESH_RollOffTable_V4      *tblStrobeFlash = NULL;
  MESH_RollOffTable_V4      *tblLedFlashStored = NULL;
  MESH_RollOffTable_V4      tblDualFlash;

  if (!isp_sub_module || !tblNormalLight || !tblOut || !mesh_tbls) {
    ISP_ERR("failed: %p %p %p %p", isp_sub_module, tblNormalLight, tblOut,
     mesh_tbls);
    return;
  }

  chrComPtr = isp_sub_module->chromatix_ptrs.chromatixComPtr;
  chromatix_rolloff = &chrComPtr->chromatix_rolloff;

  if (mod->flash_type == CAMERA_FLASH_STROBE) {
    tblFlash = &(mesh_tbls->left[ISP_ROLLOFF_STROBE_FLASH]);
    flash_start = chromatix_rolloff->rolloff_Strobe_start;
    flash_end = chromatix_rolloff->rolloff_Strobe_end;
  } else {
    led1_value_mainflash =
      mod->dual_led_setting.led1_high_setting;
    led2_value_mainflash =
      mod->dual_led_setting.led2_high_setting;
    if (led1_value_mainflash > 0 && led2_value_mainflash > 0 ) {
      led_mainflash_ratio =
          (float)led1_value_mainflash/(led1_value_mainflash +
      led2_value_mainflash);
      tblStrobeFlash = &(mod->rolloff_tbls.rolloff_tableset[0]->
          left[ISP_ROLLOFF_STROBE_FLASH]);
      tblLedFlashStored = &(mod->rolloff_tbls.rolloff_tableset[0]->
          left[ISP_ROLLOFF_LED_FLASH]);
      mesh_rolloff40_table_interpolate( tblLedFlashStored, tblStrobeFlash,
          &tblDualFlash, led_mainflash_ratio);
      tblFlash = &tblDualFlash;
    } else {
      if (led2_value_mainflash == 0)
        tblFlash = &(mesh_tbls->left[ISP_ROLLOFF_LED_FLASH]);
      if (led1_value_mainflash == 0)
        tblFlash = &(mesh_tbls->left[ISP_ROLLOFF_STROBE_FLASH]);
    }
    flash_start = chromatix_rolloff->rolloff_LED_start;
    flash_end = chromatix_rolloff->rolloff_LED_end;
  }

  flash_sensitivity = &mod->aec_update.flash_sensitivity;
  if (mod->cur_flash_mode == CAM_FLASH_MODE_ON) {
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

  if (ratio >= flash_end) {
    *tblOut = *tblFlash;
  } else if (ratio <= flash_start) {
    *tblOut = *tblNormalLight;
  } else {
    p_ratio = GET_INTERPOLATION_RATIO(ratio, flash_start, flash_end);
    mesh_rolloff40_table_interpolate( tblFlash, tblNormalLight, tblOut,
        p_ratio);
  }
} /* mesh_rolloff40_calc_flash_trigger */

#if !OVERRIDE_FUNC

/** mesh_rolloff40_normalize_table:
 *
 *  @isp_sub_module: isp sub module handle
 *  @mesh_mod: Pointer to rolloff module struct
 *
 *  Get the tables from the chromatix pointer and normalize the values to ensure
 *  all values are > 1
 *
 *  Return void
 **/
void mesh_rolloff40_normalize_table(isp_sub_module_t *isp_sub_module,
  void *mesh_in_mod)
{
  int                        i = 0, j = 0, k = 0, l = 0;
  float                      min_value = 1.0, scaling_val = 0;
  chromatix_VFE_common_type *chrComPtr = NULL;
  mesh_rolloff_array_type    inTbl;
  mesh_rolloff_array_type    sensorCalcTable;
  MESH_RollOffTable_V4       downscaledTable;
  MESH_RollOffTable_V4      *outTbl = NULL;
  MESH_RollOffTable_V4      *deltaTbl = NULL;
  chromatix_rolloff_type    *chromatix_rolloff = NULL;
  chromatix_rolloff_type    *inf_tables = NULL;
  chromatix_VFE_common_type *infTblPtr = NULL;
  sensor_rolloff_config_t   *sensor_rolloff_config = NULL;
  sensor_out_info_t         *sensor_out_info = NULL;
  sensor_request_crop_t     *request_crop = NULL;
  sensor_dim_output_t       *dim_output = NULL;
  mesh_rolloff40_t          *mesh_mod = NULL;


  if (!isp_sub_module || !mesh_in_mod) {
    ISP_ERR("failed: %p %p", isp_sub_module, mesh_in_mod);
    return;
  }

  mesh_mod = (mesh_rolloff40_t *)mesh_in_mod;
  chrComPtr = isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!chrComPtr) {
    ISP_ERR("Case failed: chrComPtr %p", chrComPtr);
    return;
  }

  chromatix_rolloff = &chrComPtr->chromatix_rolloff;
  sensor_out_info = &mesh_mod->sensor_out_info;
  sensor_rolloff_config = &sensor_out_info->sensor_rolloff_config;
  dim_output = &sensor_out_info->dim_output;
  request_crop = &sensor_out_info->request_crop;
  ISP_DBG("sensor_rolloff_config->enable %d", sensor_rolloff_config->enable);

  /* TODO: Sensor should provide this as currently this is a hack*/
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
    ISP_INFO("full %d %d out %d %d offset %d %d scale %d",
      sensor_rolloff_config->full_width, sensor_rolloff_config->full_height,
      sensor_rolloff_config->output_width,
      sensor_rolloff_config->output_height,
      sensor_rolloff_config->offset_x,
      sensor_rolloff_config->offset_y,
      sensor_rolloff_config->scale_factor);
  }

  if (mesh_mod->af_rolloff_info_valid == TRUE) {
    infTblPtr =
      (chromatix_VFE_common_type *)&mesh_mod->af_rolloff_chromatix;
    inf_tables = &infTblPtr->chromatix_rolloff;
  }

  for (k = 0, min_value = 1.0; k < ISP_ROLLOFF_LENS_POSITION_MAX; k++) {
    if (mesh_mod->rolloff_calibration_table.rolloff_tableset[k] != NULL) {
      for (i = ISP_ROLLOFF_TL84_LIGHT; i < ISP_ROLLOFF_MAX_LIGHT; i++) {
        outTbl = &(mesh_mod->rolloff_tbls.rolloff_tableset[k]->left[i]);
        deltaTbl =
          &(mesh_mod->rolloff_calibration_table.rolloff_tableset[k]->left[i]);

        /* input table from chromatix */
        if (k == ISP_ROLLOFF_LENS_POSITION_INF) {
          if (i == ISP_ROLLOFF_LED_FLASH)
            inTbl = (chromatix_rolloff->chromatix_mesh_rolloff_table_LED);
          else if (i == ISP_ROLLOFF_STROBE_FLASH)
            inTbl = (chromatix_rolloff->chromatix_mesh_rolloff_table_Strobe);
          else if (i < ROLLOFF_MAX_LIGHT){
            inTbl = (chromatix_rolloff->chromatix_mesh_rolloff_table[i]);
          }
          else {
            inTbl = (chromatix_rolloff->
              chromatix_mesh_rolloff_table_lowlight[i % ROLLOFF_MAX_LIGHT]);
          }
        } else if (k == ISP_ROLLOFF_LENS_POSITION_MACRO) {
          if (!inf_tables) /* if not supported skip this normalization */
            break;
          /* get the table from sensor. Currently copied the above code */
          if (i == ISP_ROLLOFF_LED_FLASH)
            inTbl = (inf_tables->chromatix_mesh_rolloff_table_LED);
          else if (i == ISP_ROLLOFF_STROBE_FLASH)
            inTbl = (inf_tables->chromatix_mesh_rolloff_table_Strobe);
          else if (i < ROLLOFF_MAX_LIGHT){
            inTbl = (inf_tables->chromatix_mesh_rolloff_table[i]);
          }
          else {
            inTbl = (inf_tables->
              chromatix_mesh_rolloff_table_lowlight[i % ROLLOFF_MAX_LIGHT]);
          }
        }

        /* normalize initial mesh rolloff table */
        for (j = 0; j < inTbl.mesh_rolloff_table_size; j++) {
          /* RED Channel */
          if (inTbl.r_gain[j] < min_value)
            min_value = inTbl.r_gain[j];
          /* GR Channel */
          if (inTbl.gr_gain[j] < min_value)
            min_value = inTbl.gr_gain[j];
          /* BLUE Channel */
          if (inTbl.b_gain[j] < min_value)
            min_value = inTbl.b_gain[j];
          /* GB Channel */
          if (inTbl.gb_gain[j] < min_value)
            min_value = inTbl.gb_gain[j];
        }

        if (min_value < 1.0) {
          /* Table values in chromatix header should NOT be zero */
          if (min_value == 0.0)
            ISP_ERR("%s: Invalid chromatix value",__func__);

          scaling_val = 1.0 / min_value;
          ISP_DBG("%s: Scaling value = %f",__func__,scaling_val);

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
        if (sensor_rolloff_config->enable == 1) {
           /* exec algo if sensor config are valid*/
           mesh_rolloff40_sensor_calc_tbl(&inTbl,
               &sensorCalcTable, sensor_rolloff_config);
        } else {
             ISP_ERR("%s: no exec sensor calibration algo,"
                "sensor_rolloff_info.enable = %d\n",
                  __func__, sensor_rolloff_config->enable);
         }

        for (j = 0; j < MESH_ROLL_OFF_V4_TABLE_SIZE; j++) {
          /* RED Channel */
          outTbl->TableR[j] = sensorCalcTable.r_gain[j] * deltaTbl->TableR[j];
          /* GR Channel */
          outTbl->TableGr[j] = sensorCalcTable.gr_gain[j] * deltaTbl->TableGr[j];
          /* BLUE Channel */
          outTbl->TableB[j] = sensorCalcTable.b_gain[j] * deltaTbl->TableB[j];
          /* GB Channel */
          outTbl->TableGb[j] = sensorCalcTable.gb_gain[j] * deltaTbl->TableGb[j];
        }
      }
    }
  }
  /* Added for dual LED rolloff, store original table to led_rolloff_stored */
  if (mesh_mod->rolloff_tbls.rolloff_tableset[0] != NULL) {
    memcpy(&mesh_mod->led_rolloff_stored,
             &(mesh_mod->rolloff_tbls.rolloff_tableset[0]->left[ISP_ROLLOFF_LED_FLASH]),
             sizeof(MESH_RollOffTable_V4));
  }
} /* mesh_rolloff40_normalize_table */

#endif
