/* Copyright (c) 2015, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc. */

#include "eeprom_dump.h"

/* Defines the name of the file in the dump directory */
const char *dump[EEPROM_DUMP_MAX] = {
  "wbc",
  "lsc",
  "autofocus",
  "pdaf",
  "defectpixel",
  "dualcali",
  "kbuffer",
};

/* Defines the type of light to be printed in LSC roll off table */
const char *rolloff_light_type[ROLLOFF_MAX_LIGHT] = {
  "TL84_LIGHT",
  "A_LIGHT",
  "D65_LIGHT",
  "H_LIGHT"
};

const char dumpDir[] = QCAMERA_DUMP_FRM_LOCATION;

/** eeprom_dbg_data_dump:
 *    @eeprom_dump: address of pointer to dump data
 *
 * This function provides appropriate dump based on the calibration
 * type requested example LSC, DPC, AF etc
 *
 * Return:
 * void
 **/
void eeprom_dbg_data_dump(void* eeprom_dump, void* data,
  eeprom_dump_type dump_type)
{
  boolean access_dump = FALSE;
  char value[PROPERTY_VALUE_MAX];
  sensor_eeprom_data_t* e_ctrl = NULL;
  eeprom_params_t* e_params = NULL;

  if (property_get("persist.vendor.camera.cal.dump", value, "0")) {
    access_dump = (atoi(value) == 1)? TRUE : FALSE;
  }

  if (access_dump == FALSE) {
    return;
  }

  if (!eeprom_dump) {
    SERR("No data to dump!");
    return;
  }

  switch (dump_type) {
  case EEPROM_DUMP_OTP:
    e_ctrl = (sensor_eeprom_data_t*) eeprom_dump;
    dump_otp_data(e_ctrl);
    break;
  case EEPROM_DUMP_KBUF:
    e_params = (eeprom_params_t*)eeprom_dump;
    dump_kbuffer_data(e_params, dump_type);
    break;
  case EEPROM_DUMP_CALIB:
    e_ctrl = (sensor_eeprom_data_t*) eeprom_dump;
    dump_lsc_calib_data(e_ctrl, data, EEPROM_DUMP_LSC);
    dump_wbc_calib_data(e_ctrl, data, EEPROM_DUMP_WB);
    break;
  default:
    SHIGH("Invalid dump_type: %d",dump_type);
  }

  return;
}

/** eeprom_dump_bin:
 *    @fp:          file pointer
 *    @dump_type:   eeprom dump type
 *    @eeprom_name: name of eeprom for which bin file is being dumped
 *    @size:        size of the data to be written
 *    @buff:        actual OTP data
 *
 * This function dumps OTP data in a binary file.
 *
 * Return: void
 **/
static void eeprom_dump_bin(FILE *fp, eeprom_dump_type dump_type,
    char* eeprom_name, uint32_t size, void* buff)
{

   RETURN_VOID_ON_NULL(eeprom_name);

  char bin_name[DUMP_NAME_SIZE] = { 0 };

  snprintf(bin_name, sizeof(bin_name), "%s%s_%s_%s", dumpDir,
      eeprom_name, dump[dump_type], "dump.bin");

  fp = fopen(bin_name, "wb");
  if (fp != NULL) {
    fwrite(buff, 1, size, fp);
    SHIGH(" Writing to %s", bin_name);
    fclose(fp);
  } else {
    SERR("Cannot open file name: %s", bin_name);
  }
}

/** dump_otp_data:
 *    @e_ctrl: address of pointer to dump data
 *
 * This function dumps OTP data in a text file in readable format
 * based on the features present. Supported dumps:
 * WBC
 * DPC
 * LSC
 * AFC
 * DUAL CAM
 *
 * Return: void
 **/
void dump_otp_data(sensor_eeprom_data_t* e_ctrl)
{

  RETURN_VOID_ON_NULL(e_ctrl);

  if (e_ctrl->eeprom_data.items_for_cal.is_wbc
      && e_ctrl->eeprom_data.items.is_wbc) {
    dump_wbc_data(e_ctrl, EEPROM_DUMP_WB);
  }

  if (e_ctrl->eeprom_data.items_for_cal.is_lsc
      && e_ctrl->eeprom_data.items.is_lsc) {
    dump_lsc_data(e_ctrl, EEPROM_DUMP_LSC);
  }

  if (e_ctrl->eeprom_data.items_for_cal.is_afc
      && e_ctrl->eeprom_data.items.is_afc) {
    dump_af_data(e_ctrl, EEPROM_DUMP_AF);
    dump_pdaf_data(e_ctrl, EEPROM_DUMP_PDAF);
  }

  if (e_ctrl->eeprom_data.items_for_cal.is_dpc
      && e_ctrl->eeprom_data.items.is_dpc) {
    dump_dpc_data(e_ctrl, EEPROM_DUMP_DPC);
  }

  if (e_ctrl->eeprom_data.items_for_cal.is_dual
      && e_ctrl->eeprom_data.items.is_dual) {
    dump_dualc_data(e_ctrl, EEPROM_DUMP_DUALC);
  }

}

/** dump_wbc_data:
 *    @e_ctrl: address of pointer to dump data
 *    @dump_type: dump type
 *
 * This function dumps WBC data in a text file in readable format.
 *
 * Return:
 * void
 **/
void dump_wbc_data(sensor_eeprom_data_t* e_ctrl, eeprom_dump_type dump_type)
{
  FILE *fp_wbc;
  FILE *fp_wbc_bin;
  char name[DUMP_NAME_SIZE] = { 0 };
  uint32_t i = 0;

  RETURN_VOID_ON_NULL(e_ctrl);

  /* name of the dump file */
  snprintf(name, sizeof(name), "%s%s_%s_%s", dumpDir,
      e_ctrl->eeprom_params.eeprom_name, dump[dump_type], "OTP.txt");

  /* open the file */
  fp_wbc = fopen(name, "w");
  if (fp_wbc != NULL ) {
    SHIGH(" Writing to %s", name);

    /* Format data for printing */
    fprintf(fp_wbc, "Whitebalance data dump for EEPROM %s \n",
        e_ctrl->eeprom_params.eeprom_name);
    fprintf(fp_wbc, "gr_over_gb = %f \n", e_ctrl->eeprom_data.wbc.gr_over_gb);

    fprintf(fp_wbc, "\nr_over_g:\n");
    for (i = 0; i < AGW_AWB_MAX_LIGHT; i++) {
      fprintf(fp_wbc, "r_over_g[%d] = %f \n", i,
          e_ctrl->eeprom_data.wbc.r_over_g[i]);
    }

    fprintf(fp_wbc, "\nb_over_g:\n");
    for (i = 0; i < AGW_AWB_MAX_LIGHT; i++) {
      fprintf(fp_wbc, "b_over_g[%d] = %f \n", i,
          e_ctrl->eeprom_data.wbc.b_over_g[i]);
    }

    /* End format, close file */
    fclose(fp_wbc);

  } else {
    SERR("Cannot open file name: %s", name);
  }
}

/** dump_lsc_data:
 *    @e_ctrl: address of pointer to dump data
 *    @dump_type: dump type
 *
 * This function dumps LSC data in a text file in readable format.
 *
 * Return:
 * void
 **/
void dump_lsc_data(sensor_eeprom_data_t* e_ctrl, eeprom_dump_type dump_type)
{
  FILE *fp_lsc;
  FILE *fp_lsc_bin;
  char name[DUMP_NAME_SIZE] = { 0 };
  uint32_t light_type, row, col;

  RETURN_VOID_ON_NULL(e_ctrl);

  /* name of the dump file */
  snprintf(name, sizeof(name), "%s%s_%s_%s", dumpDir,
      e_ctrl->eeprom_params.eeprom_name, dump[dump_type], "OTP.txt");

  /* open the file */
  fp_lsc = fopen(name, "w");
  if (fp_lsc != NULL) {
    SHIGH(" Writing to %s", name);

    /* Format data for printing */
    fprintf(fp_lsc,"Lens shading correction data dump for EEPROM %s \n",
        e_ctrl->eeprom_params.eeprom_name);
    for(light_type = 0; light_type < ROLLOFF_MAX_LIGHT; light_type++)
    {
      fprintf(fp_lsc,"\n************Light type: %s ************\n",
          rolloff_light_type[light_type]);

      fprintf(fp_lsc,"mesh_rolloff_table_size = %d\n",
         e_ctrl->eeprom_data.lsc.lsc_calib[light_type].mesh_rolloff_table_size);

      fprintf(fp_lsc,"\nr_gain\n");
      for(row = 0; row < MESH_ROLLOFF_ROW_SIZE; row ++)
      {
        for(col = 0; col < MESH_ROLLOFF_COL_SIZE; col ++)
        {
          fprintf(fp_lsc,"%f  ",e_ctrl->eeprom_data.lsc.lsc_calib[light_type].
              r_gain[(row*MESH_ROLLOFF_COL_SIZE)+col]);
        }
        fprintf(fp_lsc,"\n");
      }

      fprintf(fp_lsc,"\ngr_gain\n");
      for(row = 0; row < MESH_ROLLOFF_ROW_SIZE; row ++)
      {
        for(col = 0; col < MESH_ROLLOFF_COL_SIZE; col ++)
        {
          fprintf(fp_lsc,"%f  ",e_ctrl->eeprom_data.lsc.lsc_calib[light_type].
              gr_gain[(row*MESH_ROLLOFF_COL_SIZE)+col]);
        }
        fprintf(fp_lsc,"\n");
      }

      fprintf(fp_lsc,"\ngb_gain\n");
      for(row = 0; row < MESH_ROLLOFF_ROW_SIZE; row ++)
      {
        for(col = 0; col < MESH_ROLLOFF_COL_SIZE; col ++)
        {
          fprintf(fp_lsc,"%f  ",e_ctrl->eeprom_data.lsc.lsc_calib[light_type].
              gb_gain[(row*MESH_ROLLOFF_COL_SIZE)+col]);
        }
        fprintf(fp_lsc,"\n");
      }

      fprintf(fp_lsc,"\nb_gain\n");
      for(row = 0; row < MESH_ROLLOFF_ROW_SIZE; row ++)
      {
        for(col = 0; col < MESH_ROLLOFF_COL_SIZE; col ++)
        {
          fprintf(fp_lsc,"%f  ",e_ctrl->eeprom_data.lsc.lsc_calib[light_type].
              b_gain[(row*MESH_ROLLOFF_COL_SIZE)+col]);
        }
        fprintf(fp_lsc,"\n");
      }
    }

    /* End format, close file */
    fclose(fp_lsc);

  } else {
    SERR("Cannot open file name: %s", name);
  }
}

/** dump_af_data:
 *    @e_ctrl: address of pointer to dump data
 *    @dump_type: dump type
 *
 * This function dumps AF data in a text file in readable format.
 *
 * Return:
 * void
 **/
void dump_af_data(sensor_eeprom_data_t* e_ctrl, eeprom_dump_type dump_type)
{
  FILE *fp_af;
  FILE *fp_af_bin;
  char name[DUMP_NAME_SIZE] = { 0 };
  uint32_t i;

  RETURN_VOID_ON_NULL(e_ctrl);

  /* name of the dump file */
  snprintf(name, sizeof(name), "%s%s_%s_%s", dumpDir,
      e_ctrl->eeprom_params.eeprom_name, dump[dump_type], "OTP.txt");

  /* open the file */
  fp_af = fopen(name, "w");
  if (fp_af != NULL) {
    SHIGH(" Writing to %s", name);

    /* Format data for printing */
    fprintf(fp_af,"Autofocus data dump for EEPROM %s \n",
        e_ctrl->eeprom_params.eeprom_name);
    fprintf(fp_af,"macro_dac = %d\n", e_ctrl->eeprom_data.afc.macro_dac);
    fprintf(fp_af,"infinity_dac = %d\n", e_ctrl->eeprom_data.afc.infinity_dac);
    fprintf(fp_af,"starting_dac = %d\n", e_ctrl->eeprom_data.afc.starting_dac);

    /* End format, close file */
    fclose(fp_af);

  } else {
    SERR("Cannot open file name: %s", name);
  }
}

/** dump_pdaf_data:
 *    @e_ctrl: address of pointer to dump data
 *    @dump_type: dump type
 *
 * This function dumps AF data in a text file in readable format.
 *
 * Return:
 * void
 **/
void dump_pdaf_data(sensor_eeprom_data_t* e_ctrl, eeprom_dump_type dump_type)
{
  FILE *fp;
  char name[DUMP_NAME_SIZE] = { 0 };
  uint32_t i;
  pdafcalib_data_t    *pdaf_data = &e_ctrl->eeprom_data.pdafc;
  pdaf_2D_cal_data_t  *pdafc_2d = &e_ctrl->eeprom_data.pdafc_2d;

  RETURN_VOID_ON_NULL(e_ctrl);

  /* name of the dump file */
  snprintf(name, sizeof(name), "%s%s_%s_%s", dumpDir,
      e_ctrl->eeprom_params.eeprom_name, dump[dump_type], "OTP.txt");

  /* open the file */
  fp = fopen(name, "w");
  if (fp != NULL) {
    SHIGH(" Writing to %s", name);

    fprintf(fp,"-------------PDAF T1 data DUMP------------\n");
    for(i = 0;i < 48;i++) {
      fprintf(fp,"SlopeData[%d]: %f\n", i,
        pdaf_data->SlopeData[i]);
    }

    fprintf(fp,"-------------PDAF T2/3 data DUMP----------\n");
    fprintf(fp,"VersionNum: %d, OffsetX %d, OffsetY %d, RatioX %d, RatioY %d,"
      "MapWidth %d, MapHeight %d, PD_conversion_coeff[0] %d\n",
      pdafc_2d->VersionNum,
      pdafc_2d->OffsetX,
      pdafc_2d->OffsetY,
      pdafc_2d->RatioX,
      pdafc_2d->RatioY,
      pdafc_2d->MapWidth,
      pdafc_2d->MapHeight,
      pdafc_2d->PD_conversion_coeff[0]
    );
    fprintf(fp,"-----------Left_GainMap----------------\n");
    for (i = 0; i < 221; i = i + 17) {
      fprintf(fp,"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
        "%d, %d, %d, %d, %d\n",
        pdafc_2d->Left_GainMap[i],    pdafc_2d->Left_GainMap[i+1],
        pdafc_2d->Left_GainMap[i+2],  pdafc_2d->Left_GainMap[i+3],
        pdafc_2d->Left_GainMap[i+4],  pdafc_2d->Left_GainMap[i+5],
        pdafc_2d->Left_GainMap[i+6],  pdafc_2d->Left_GainMap[i+7],
        pdafc_2d->Left_GainMap[i+8],  pdafc_2d->Left_GainMap[i+9],
        pdafc_2d->Left_GainMap[i+10], pdafc_2d->Left_GainMap[i+11],
        pdafc_2d->Left_GainMap[i+12], pdafc_2d->Left_GainMap[i+13],
        pdafc_2d->Left_GainMap[i+14], pdafc_2d->Left_GainMap[i+15],
        pdafc_2d->Left_GainMap[i+16]
     );
    }
    fprintf(fp,"-----------Right_GainMap----------------\n");
    for (i = 0; i < 221; i = i + 17) {
      fprintf(fp,"%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
        "%d, %d, %d, %d, %d\n",
        pdafc_2d->Right_GainMap[i],    pdafc_2d->Right_GainMap[i+1],
        pdafc_2d->Right_GainMap[i+2],  pdafc_2d->Right_GainMap[i+3],
        pdafc_2d->Right_GainMap[i+4],  pdafc_2d->Right_GainMap[i+5],
        pdafc_2d->Right_GainMap[i+6],  pdafc_2d->Right_GainMap[i+7],
        pdafc_2d->Right_GainMap[i+8],  pdafc_2d->Right_GainMap[i+9],
        pdafc_2d->Right_GainMap[i+10], pdafc_2d->Right_GainMap[i+11],
        pdafc_2d->Right_GainMap[i+12], pdafc_2d->Right_GainMap[i+13],
        pdafc_2d->Right_GainMap[i+14], pdafc_2d->Right_GainMap[i+15],
        pdafc_2d->Right_GainMap[i+16]
     );
    }
    fprintf(fp,"-----------DCC----------------\n");
    fprintf(fp,"VersionNum: %d, MapWidth %d, MapHeight %d, Q factor %d\n",
      pdafc_2d->VersionNum_DCC,
      pdafc_2d->MapWidth_DCC,
      pdafc_2d->MapHeight_DCC,
      pdafc_2d->Q_factor_DCC);
    for (i = 0; i < 48; i = i + 8) {
      fprintf(fp,"%d, %d, %d, %d, %d, %d, %d, %d,\n",
        pdafc_2d->PD_conversion_coeff[i],    pdafc_2d->PD_conversion_coeff[i+1],
        pdafc_2d->PD_conversion_coeff[i+2],  pdafc_2d->PD_conversion_coeff[i+3],
        pdafc_2d->PD_conversion_coeff[i+4],  pdafc_2d->PD_conversion_coeff[i+5],
        pdafc_2d->PD_conversion_coeff[i+6],  pdafc_2d->PD_conversion_coeff[i+7]);
  }
    /* End format, close file */
    fclose(fp);
  } else {
    SERR("Cannot open file name: %s", name);
  }
}



/** dump_dpc_data:
 *    @e_ctrl: address of pointer to dump data
 *    @dump_type: dump type
 *
 * This function dumps DPC data in a text file in readable format.
 *
 * Return:
 * void
 **/
void dump_dpc_data(sensor_eeprom_data_t* e_ctrl, eeprom_dump_type dump_type)
{
  FILE *fp_dpc;
  FILE *fp_dpc_bin;
  char name[DUMP_NAME_SIZE] = { 0 };
  int32_t i , j;

  RETURN_VOID_ON_NULL(e_ctrl);

  /* name of the dump file */
  snprintf(name, sizeof(name), "%s%s_%s_%s", dumpDir,
      e_ctrl->eeprom_params.eeprom_name, dump[dump_type], "OTP.txt");

  /* open the file */
  fp_dpc = fopen(name, "w");
  if (fp_dpc != NULL ) {
    SHIGH(" Writing to %s", name);

    /* Format data for printing */
    fprintf(fp_dpc, "Defect pixel correction data dump for EEPROM %s \n",
        e_ctrl->eeprom_params.eeprom_name);

    for (i = 0; i < MAX_RESOLUTION_MODES ; i++) {
      fprintf(fp_dpc, "Resolution mode = %d\n", i);
      fprintf(fp_dpc, "defect pixel count = %d\n",
              e_ctrl->eeprom_data.dpc.dpc_calib[i].count);
      for (j =0; (j < e_ctrl->eeprom_data.dpc.dpc_calib[i].count )
                                      && (j < MAX_DPC_COORD); j++) {
        fprintf(fp_dpc, "dpc_calib_pix[%d](x,y) = (%d,%d) \n", j,
                e_ctrl->eeprom_data.dpc.dpc_calib[i].pix[j].x,
                e_ctrl->eeprom_data.dpc.dpc_calib[i].pix[j].y);
      }
    }
    /* End format, close file */
    fclose(fp_dpc);

  } else {
    SERR("Cannot open file name: %s", name);
  }
}

/** dump_dualc_data:
 *    @e_ctrl: address of pointer to dump data
 *    @dump_type: dump type
 *
 * This function dumps DUAL CALIB data in a text file in readable format.
 *
 * Return:
 * void
 **/
void dump_dualc_data(sensor_eeprom_data_t* e_ctrl, eeprom_dump_type dump_type)
{
  FILE *fp_dc = NULL;
  FILE *fp_dc_bin = NULL;
  char name[DUMP_NAME_SIZE] = { 0 };
  uint32_t i;

  RETURN_VOID_ON_NULL(e_ctrl);

  /* dump DUALC data in a binary file */
  eeprom_dump_bin(fp_dc_bin,dump_type,e_ctrl->eeprom_params.eeprom_name,
      e_ctrl->eeprom_data.dualc.dc_size,
      &e_ctrl->eeprom_params.buffer[e_ctrl->eeprom_data.dualc.dc_offset]);

  /* name of the dump file */
  snprintf(name, sizeof(name), "%s%s_%s_%s", dumpDir,
      e_ctrl->eeprom_params.eeprom_name, dump[dump_type], "OTP.txt");

  /* open the file */
  fp_dc = fopen(name, "w");
  if (fp_dc != NULL ) {
    SHIGH(" Writing to %s", name);

    /* Format data for printing */
    fprintf(fp_dc, "Dual Camera Calib data dump for EEPROM %s \n",
        e_ctrl->eeprom_params.eeprom_name);

    fprintf(fp_dc, "MASTER SENSOR CALIB: \n");
    fprintf(fp_dc, "Focal length = %f\n",
        e_ctrl->eeprom_data.dualc.master_calib_params.focal_length);
    fprintf(fp_dc, "Native sensor resolution width = %d\n",
        e_ctrl->eeprom_data.dualc.master_calib_params.
        native_sensor_resolution_width);
    fprintf(fp_dc, "Native sensor resolution height = %d\n",
        e_ctrl->eeprom_data.dualc.master_calib_params.
        native_sensor_resolution_height);
    fprintf(fp_dc, "Calibration resolution width = %d\n",
        e_ctrl->eeprom_data.dualc.master_calib_params.
        calibration_resolution_width);
    fprintf(fp_dc, "Calibration resolution height = %d\n",
        e_ctrl->eeprom_data.dualc.master_calib_params.
        calibration_resolution_height);
    fprintf(fp_dc, "Focal length ratio= %f\n",
        e_ctrl->eeprom_data.dualc.master_calib_params.focal_length_ratio);

    fprintf(fp_dc, "\nAUXILIARY SENSOR CALIB: \n");
    fprintf(fp_dc, "Focal length = %f\n",
        e_ctrl->eeprom_data.dualc.aux_calib_params.focal_length);
    fprintf(fp_dc, "Native sensor resolution width = %d\n",
        e_ctrl->eeprom_data.dualc.aux_calib_params.
        native_sensor_resolution_width);
    fprintf(fp_dc, "Native sensor resolution height = %d\n",
        e_ctrl->eeprom_data.dualc.aux_calib_params.
        native_sensor_resolution_height);
    fprintf(fp_dc, "Calibration resolution width = %d\n",
        e_ctrl->eeprom_data.dualc.aux_calib_params.
        calibration_resolution_width);
    fprintf(fp_dc, "Calibration resolution height = %d\n",
        e_ctrl->eeprom_data.dualc.aux_calib_params.
        calibration_resolution_height);
    fprintf(fp_dc, "Focal length ratio= %f\n",
        e_ctrl->eeprom_data.dualc.aux_calib_params.focal_length_ratio);

    fprintf(fp_dc, "\nSYSTEM CALIB: \n");
    fprintf(fp_dc, "Calibration version format = %d\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        calibration_format_version);
    fprintf(fp_dc, "Relative Principle Point X Offset = %f\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        relative_principle_point_x_offset);
    fprintf(fp_dc, "Relative Principle Point Y Offset = %f\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        relative_principle_point_y_offset);
    fprintf(fp_dc, "Relative Position Flag = %d\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        relative_position_flag);
    fprintf(fp_dc, "Relative Baseline Distance = %f\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        relative_baseline_distance);

    for (i = 0; i < RELATIVE_ROTATION_MATRIX_MAX; i++) {
      fprintf(fp_dc, "Relative_rotation_matrix[%d] = %f\n", i,
          e_ctrl->eeprom_data.dualc.system_calib_params.
          relative_rotation_matrix[i]);
    }
    for (i = 0; i < RELATIVE_GEOMETRIC_SURFACE_PARAMS_MAX; i++) {
      fprintf(fp_dc, "Relative_geometric_surface_parameters[%d] = %f\n", i,
          e_ctrl->eeprom_data.dualc.system_calib_params.
          relative_geometric_surface_parameters[i]);
    }

    fprintf(fp_dc, "Master sensor mirror and flip setting = %d\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        master_sensor_mirror_flip_setting);
    fprintf(fp_dc, "Auxiliary sensor mirror and flip setting = %d\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        aux_sensor_mirror_flip_setting);
    fprintf(fp_dc, "Module orientation flag = %d\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        module_orientation_flag);
    fprintf(fp_dc, "Rotation flag = %d\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        rotation_flag);
    fprintf(fp_dc, "AEC CALIB DATA: \n");
    fprintf(fp_dc, "Brightness ratio = %d\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        brightness_ratio);
    fprintf(fp_dc, "Ref aux gain = %d\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        ref_aux_gain);
    fprintf(fp_dc, "Ref aux linecount = %dms\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        ref_aux_linecount);
    fprintf(fp_dc, "Ref master gain = %d\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        ref_master_gain);
    fprintf(fp_dc, "Ref master linecount = %dms\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        ref_master_linecount);
    fprintf(fp_dc, "Af calibration version = %d\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        af_sync_calib_version);
    fprintf(fp_dc, "Af Sync Calib points = %d\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        nums_of_af_sync_point);
    for ( i = 0 ; i < e_ctrl->eeprom_data.dualc.system_calib_params.nums_of_af_sync_point ; i++) {
      fprintf(fp_dc, "Af sync point[%d] distance = %dcm, dac = %d\n", i,
        e_ctrl->eeprom_data.dualc.system_calib_params.dist_of_af_sync_point[i],
        e_ctrl->eeprom_data.dualc.system_calib_params.dac_of_af_sync_point[i]);
    }
    fprintf(fp_dc, "Ref master color temperature = %d\n",
        e_ctrl->eeprom_data.dualc.system_calib_params.
        ref_master_color_temperature);

    /* End format, close file */
    fclose(fp_dc);

  } else {
    SERR("Cannot open file name: %s", name);
  }
}

/** dump_kbuffer_data:
 *    @e_params: address of pointer to dump data
 *    @dump_type: dump type
 *
 * This function dumps buffer returned from kernel, byte by byte, into a text
 * file.
 *
 * Return:
 * void
 **/
void dump_kbuffer_data(eeprom_params_t* e_params, eeprom_dump_type dump_type)
{
  FILE *fp_kbuf;
  char name[DUMP_NAME_SIZE] = { 0 };
  uint32_t i;

  RETURN_VOID_ON_NULL(e_params);

  /* name of the dump file */
  snprintf(name, sizeof(name), "%s%s_%s_%s", dumpDir,
      e_params->eeprom_name, dump[dump_type], "dump.txt");

  /* open the file */
  fp_kbuf = fopen(name, "w");
  if (fp_kbuf != NULL) {
    SHIGH(" Writing to %s", name);

    /* Format data for printing */
    fprintf(fp_kbuf,"Kernel buffer data dump: \n");
    fprintf(fp_kbuf,"eeprom_name: %s\n",e_params->eeprom_name);
    fprintf(fp_kbuf,"Number of bytes: %d\n",e_params->num_bytes);
    for(i=0;i<e_params->num_bytes;i++)
    {
      fprintf(fp_kbuf,"0x%02X\n",e_params->buffer[i]);
    }

    /* End format, close file */
    fclose(fp_kbuf);

  } else {
    SERR("Cannot open file name: %s", name);
  }
}

/** dump_lsc_calib_data:
 *    @e_ctrl: address of pointer to dump data
 *    @dump_type: dump type
 *
 * This function dumps calibrated LSC data from chromatix headers
 * in a text file in readable format.
 *
 * Return:
 * void
 **/
void dump_lsc_calib_data(sensor_eeprom_data_t* e_ctrl, void* data,
  eeprom_dump_type dump_type)
{

  FILE*                      fp_lsc_calib;
  char                       name[DUMP_NAME_SIZE] = { 0 };
  uint32_t                   light_type, row, col;
  chromatix_VFE_common_type* common_chromatix = NULL;
  chromatix_rolloff_type*    chromatix = NULL;
  sensor_chromatix_params_t* chromatix_params;

  if(!e_ctrl || !data)
    return;

  chromatix_params = (sensor_chromatix_params_t*)data;
  common_chromatix = (chromatix_VFE_common_type *)
    chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_ISP_COMMON];
  if(!common_chromatix)
    return;

  chromatix = &(common_chromatix->chromatix_rolloff);
  if(!chromatix)
    return;

  /* name of the dump file */
  snprintf(name, sizeof(name), "%s%s_%s_%s", dumpDir,
      e_ctrl->eeprom_params.eeprom_name, dump[dump_type], "calibrated.txt");

  /* open the file */
  fp_lsc_calib = fopen(name, "w");
  if (fp_lsc_calib != NULL) {
    SERR(" Writing to %s", name);

    /* Format data for printing */
    fprintf(fp_lsc_calib,"Dumping data for %s \n",
            chromatix_params->chromatix_lib_name[SENSOR_CHROMATIX_ISP_COMMON]);

    for(light_type = 0; light_type < ROLLOFF_MAX_LIGHT; light_type++)
    {
      fprintf(fp_lsc_calib,"\n************Light type: %s ************\n",
          rolloff_light_type[light_type]);

      fprintf(fp_lsc_calib,"\nr_gain\n");
      for(row = 0; row < MESH_ROLLOFF_ROW_SIZE; row ++)
      {
        for(col = 0; col < MESH_ROLLOFF_COL_SIZE; col ++)
        {
          fprintf(fp_lsc_calib,"%f  ",chromatix->chromatix_mesh_rolloff_table[light_type].
              r_gain[(row*MESH_ROLLOFF_COL_SIZE)+col]);
        }
        fprintf(fp_lsc_calib,"\n");
      }

      fprintf(fp_lsc_calib,"\ngr_gain\n");
      for(row = 0; row < MESH_ROLLOFF_ROW_SIZE; row ++)
      {
        for(col = 0; col < MESH_ROLLOFF_COL_SIZE; col ++)
        {
          fprintf(fp_lsc_calib,"%f  ",chromatix->chromatix_mesh_rolloff_table[light_type].
              gr_gain[(row*MESH_ROLLOFF_COL_SIZE)+col]);
        }
        fprintf(fp_lsc_calib,"\n");
      }

      fprintf(fp_lsc_calib,"\ngb_gain\n");
      for(row = 0; row < MESH_ROLLOFF_ROW_SIZE; row ++)
      {
        for(col = 0; col < MESH_ROLLOFF_COL_SIZE; col ++)
        {
          fprintf(fp_lsc_calib,"%f  ",chromatix->chromatix_mesh_rolloff_table[light_type].
              gb_gain[(row*MESH_ROLLOFF_COL_SIZE)+col]);
        }
        fprintf(fp_lsc_calib,"\n");
      }

      fprintf(fp_lsc_calib,"\nb_gain\n");
      for(row = 0; row < MESH_ROLLOFF_ROW_SIZE; row ++)
      {
        for(col = 0; col < MESH_ROLLOFF_COL_SIZE; col ++)
        {
          fprintf(fp_lsc_calib,"%f  ",chromatix->chromatix_mesh_rolloff_table[light_type].
              b_gain[(row*MESH_ROLLOFF_COL_SIZE)+col]);
        }
        fprintf(fp_lsc_calib,"\n");
      }
    }

    /* End format, close file */
    fclose(fp_lsc_calib);

  } else {
    SERR("Cannot open file name: %s", name);
  }

}
/** dump_wbc_calib_data:
 *    @e_ctrl: address of pointer to dump data
 *    @dump_type: dump type
 *
 * This function dumps calibrated WBC data from chromatix headers
 * in a text file in readable format.
 *
 * Return:
 * void
 **/
void dump_wbc_calib_data(sensor_eeprom_data_t* e_ctrl, void* data,
  eeprom_dump_type dump_type)
{
  FILE*                      fp_wbc_calib;
  char                       name[DUMP_NAME_SIZE] = { 0 };
  uint32_t                   i = 0;
  chromatix_3a_parms_type*   chromatix = NULL;
  sensor_chromatix_params_t* chromatix_params;

  if(!e_ctrl || !data)
    return;

  chromatix_params = (sensor_chromatix_params_t*)data;
  chromatix = chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_3A];
  if(!chromatix)
    return;

  /* name of the dump file */
  snprintf(name, sizeof(name), "%s%s_%s_%s", dumpDir,
      e_ctrl->eeprom_params.eeprom_name, dump[dump_type], "calibrated.txt");

  /* open the file */
  fp_wbc_calib = fopen(name, "w");
  if (fp_wbc_calib!= NULL ) {
    SERR(" Writing to %s", name);

    /* Format data for printing */
    fprintf(fp_wbc_calib,"Dumping data for %s \n",
        chromatix_params->chromatix_lib_name[SENSOR_CHROMATIX_3A]);

    fprintf(fp_wbc_calib, "\nr_over_g:\n");
    for (i = 0; i < AGW_AWB_MAX_LIGHT; i++) {
      fprintf(fp_wbc_calib, "r_over_g[%d] = %f \n", i,
          chromatix->AWB_bayer_algo_data.awb_basic_tuning.reference[i].
          RG_ratio);
    }

    fprintf(fp_wbc_calib, "\nb_over_g:\n");
    for (i = 0; i < AGW_AWB_MAX_LIGHT; i++) {
      fprintf(fp_wbc_calib, "b_over_g[%d] = %f \n", i,
          chromatix->AWB_bayer_algo_data.awb_basic_tuning.reference[i].
          BG_ratio);
    }

    fprintf(fp_wbc_calib, "\nMWB TL84:\n");
    fprintf(fp_wbc_calib, "r_gain = %f \n",
      chromatix->AWB_bayer_algo_data.awb_MWB.MWB_tl84.r_gain);
    fprintf(fp_wbc_calib, "g_gain = %f \n",
      chromatix->AWB_bayer_algo_data.awb_MWB.MWB_tl84.g_gain);
    fprintf(fp_wbc_calib, "b_gain = %f \n",
      chromatix->AWB_bayer_algo_data.awb_MWB.MWB_tl84.b_gain);

    fprintf(fp_wbc_calib, "\nMWB D50:\n");
    fprintf(fp_wbc_calib, "r_gain = %f \n",
      chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d50.r_gain);
    fprintf(fp_wbc_calib, "g_gain = %f \n",
      chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d50.g_gain);
    fprintf(fp_wbc_calib, "b_gain = %f \n",
      chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d50.b_gain);

    fprintf(fp_wbc_calib, "\nMWB Incandescent:\n");
    fprintf(fp_wbc_calib, "r_gain = %f \n",
      chromatix->AWB_bayer_algo_data.awb_MWB.MWB_A.r_gain);
    fprintf(fp_wbc_calib, "g_gain = %f \n",
      chromatix->AWB_bayer_algo_data.awb_MWB.MWB_A.g_gain);
    fprintf(fp_wbc_calib, "b_gain = %f \n",
      chromatix->AWB_bayer_algo_data.awb_MWB.MWB_A.b_gain);

    fprintf(fp_wbc_calib, "\nMWB D65:\n");
    fprintf(fp_wbc_calib, "r_gain = %f \n",
      chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d65.r_gain);
    fprintf(fp_wbc_calib, "g_gain = %f \n",
      chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d65.g_gain);
    fprintf(fp_wbc_calib, "b_gain = %f \n",
      chromatix->AWB_bayer_algo_data.awb_MWB.MWB_d65.b_gain);

    fprintf(fp_wbc_calib, "\nMWB Strobe:\n");
    fprintf(fp_wbc_calib, "r_gain = %f \n",
     chromatix->AWB_bayer_algo_data.awb_MWB.strobe_flash_white_balance.r_gain);
    fprintf(fp_wbc_calib, "g_gain = %f \n",
     chromatix->AWB_bayer_algo_data.awb_MWB.strobe_flash_white_balance.g_gain);
    fprintf(fp_wbc_calib, "b_gain = %f \n",
     chromatix->AWB_bayer_algo_data.awb_MWB.strobe_flash_white_balance.b_gain);

    fprintf(fp_wbc_calib, "\nMWB LED flash:\n");
    fprintf(fp_wbc_calib, "r_gain = %f \n",
      chromatix->AWB_bayer_algo_data.awb_MWB.led_flash_white_balance.r_gain);
    fprintf(fp_wbc_calib, "g_gain = %f \n",
      chromatix->AWB_bayer_algo_data.awb_MWB.led_flash_white_balance.g_gain);
    fprintf(fp_wbc_calib, "b_gain = %f \n",
      chromatix->AWB_bayer_algo_data.awb_MWB.led_flash_white_balance.b_gain);

    fprintf(fp_wbc_calib, "\nAWB in Misleading color zone:\n");
    for (i = 0; i < MISLEADING_COLOR_ZONE_NUM; i++) {

        fprintf(fp_wbc_calib, "rg_center = %f \n",
        chromatix->AWB_bayer_algo_data.awb_misleading_color_zone.
        awb_misleading_color_zones[i].detect_zone.rg_center);

        fprintf(fp_wbc_calib, "bg_center = %f \n",
        chromatix->AWB_bayer_algo_data.awb_misleading_color_zone.
        awb_misleading_color_zones[i].detect_zone.bg_center);

    }

    /* End format, close file */
    fclose(fp_wbc_calib);

  } else {
    SERR("Cannot open file name: %s", name);
  }
}


