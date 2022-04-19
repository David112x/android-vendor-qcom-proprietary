/*============================================================================

Copyright (c) 2015 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "rohm_brce064gwz.h"

/** brce064gwz_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void brce064gwz_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);

  e_items->is_wbc = datapresent ? TRUE : FALSE;
  e_items->is_afc = datapresent ? TRUE : FALSE;
  e_items->is_lsc = datapresent ? TRUE : FALSE;
  e_items->is_dpc = FALSE;
  e_items->is_insensor = FALSE;
  e_items->is_ois = FALSE;
}

/** brce064gwz_check_empty_page:
 *    @buff: address of page buffer
 *
 * Checks if the page has non zero data
 *
 * Return:
 * unsigned char :  PAGE_EMPTY / PAGE_NOT_EMPTY
 **/
static unsigned char brce064gwz_check_empty_page( unsigned char *buff )
{
  unsigned char retval = PAGE_EMPTY;
  int i = 0;

  for(i = 0; i < MAX_EMPTY_BYTES; i++){
    if( buff[i] != 0xFF && buff[i] != 0)
    {
      retval = PAGE_NOT_EMPTY;
      break;
    }
  }
  return retval;
}


/** brce064gwz_format_dcdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void brce064gwz_format_dcdata(sensor_eeprom_data_t *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  int i,j;

  if (*(unsigned int *)&buffer[DC_OFFSET] == 0xFFFFFFFF) {
    ectrl->eeprom_data.items.is_dual = FALSE;
    SERR("this eeprom doesnt have the dual calib data");
    return;
  } else {
    ectrl->eeprom_data.items.is_dual = TRUE;
  }

  e_ctrl->eeprom_data.dualc.dc_offset = DC_OFFSET;
  e_ctrl->eeprom_data.dualc.dc_size   = DC_SIZE;

  /* assign formatted data to dual camera system */
  e_ctrl->eeprom_data.dualc.system_calib_params.calibration_format_version
    = *(unsigned int *)&buffer[DC_OFFSET];

  /* assign formatted data to main camera */
  e_ctrl->eeprom_data.dualc.master_calib_params.focal_length
  = *(float *)&buffer[DC_MAIN];
  e_ctrl->eeprom_data.dualc.master_calib_params.
  native_sensor_resolution_width = *(unsigned short *)&buffer[DC_MAIN + 4];
  e_ctrl->eeprom_data.dualc.master_calib_params.
  native_sensor_resolution_height = *(unsigned short *)&buffer[DC_MAIN + 6];
  e_ctrl->eeprom_data.dualc.master_calib_params.calibration_resolution_width=
    *(unsigned short *)&buffer[DC_MAIN + 8];
  e_ctrl->eeprom_data.dualc.master_calib_params.calibration_resolution_height=
    *(unsigned short *)&buffer[DC_MAIN + 10];
  e_ctrl->eeprom_data.dualc.master_calib_params.focal_length_ratio=
    *(float *)&buffer[DC_MAIN + 12];

  /* assign formatted data to aux camera */
  e_ctrl->eeprom_data.dualc.aux_calib_params.focal_length =
    *(float *)&buffer[DC_AUX];
  e_ctrl->eeprom_data.dualc.aux_calib_params.native_sensor_resolution_width =
    *(unsigned short *)&buffer[DC_AUX + 4];
  e_ctrl->eeprom_data.dualc.aux_calib_params.native_sensor_resolution_height =
    *(unsigned short *)&buffer[DC_AUX + 6];
  e_ctrl->eeprom_data.dualc.aux_calib_params.calibration_resolution_width =
    *(unsigned short *)&buffer[DC_AUX + 8];
  e_ctrl->eeprom_data.dualc.aux_calib_params.calibration_resolution_height =
    *(unsigned short *)&buffer[DC_AUX + 10];
  e_ctrl->eeprom_data.dualc.aux_calib_params.focal_length_ratio =
    *(float *)&buffer[DC_AUX + 12];

  for(i = 0;i < DC_ROTATION_MATRIX;i++)
    e_ctrl->eeprom_data.dualc.system_calib_params.relative_rotation_matrix[i]=
      *(float *)&buffer[DC_OFFSET + 36 + i*4];

  for(j = 0;j < DC_GEOMETRIC_MATRIX;j++)
    e_ctrl->eeprom_data.dualc.system_calib_params.
    relative_geometric_surface_parameters[j] =
    *(float *)&buffer[DC_OFFSET + 72 + j*4];

  e_ctrl->eeprom_data.dualc.system_calib_params
    .relative_principle_point_x_offset = *(float *)&buffer[DC_OTHERS];
  e_ctrl->eeprom_data.dualc.system_calib_params
    .relative_principle_point_y_offset = *(float *)&buffer[DC_OTHERS + 4];
  e_ctrl->eeprom_data.dualc.system_calib_params.relative_position_flag
    = *(unsigned short *)&buffer[DC_OTHERS + 8];
  e_ctrl->eeprom_data.dualc.system_calib_params.relative_baseline_distance =
    *(float *)&buffer[DC_OTHERS + 10];
  e_ctrl->eeprom_data.dualc.system_calib_params.
  master_sensor_mirror_flip_setting =
    *(unsigned short *)&buffer[DC_OTHERS + 14];
  e_ctrl->eeprom_data.dualc.system_calib_params.aux_sensor_mirror_flip_setting
    = *(unsigned short *)&buffer[DC_OTHERS + 16];
  e_ctrl->eeprom_data.dualc.system_calib_params.module_orientation_flag
    = *(unsigned short *)&buffer[DC_OTHERS + 18];
  e_ctrl->eeprom_data.dualc.system_calib_params.rotation_flag
    = *(unsigned short *)&buffer[DC_OTHERS + 20];
}

/** brce064gwz_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void brce064gwz_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  wbcalib_data_t *wbc = &(e_ctrl->eeprom_data.wbc);
  unsigned int i;
  float awb_r_over_gr = 0.0, awb_b_over_gr = 0.0, awb_gr_over_gb = 0.0;
  unsigned int wb_offset = WB_OFFSET;

  awb_r_over_gr = (buffer[wb_offset + 1] << 8 | buffer[wb_offset]) /
   (float)QVALUE;
  awb_b_over_gr = (buffer[wb_offset + 3] << 8 | buffer[wb_offset + 2]) /
   (float)QVALUE;
  awb_gr_over_gb = (buffer[wb_offset + 5] << 8 | buffer[wb_offset + 4]) /
   (float)QVALUE;

  SHIGH("OTP:AWB:D65 awb_r_over_gr: %f,awb_b_over_gr: %f,awb_gr_over_gb: %f",
    awb_r_over_gr, awb_b_over_gr, awb_gr_over_gb);

  /* Need to set wbc->gr_over_gb according to Bayer pattern and cal data
    for "BGGR" and "GBRG" of bayer pattern, Gb/Gr of stored calibration data
      wbc->gr_over_gb = awb_gr_over_gb;
    for "RGGB" and "GRBG" bayer pattern, Gb/Gr of stored calibration data
      wbc->gr_over_gb = 1.0f/awb_gr_over_gb; */
  wbc->gr_over_gb = 1.0f/awb_gr_over_gb;
  /* populate all light types from light source 1 */
  for (i = 0; i < AGW_AWB_MAX_LIGHT; i++) {
    wbc->r_over_g[i] = awb_r_over_gr;
    wbc->b_over_g[i] = awb_b_over_gr;
  }
}

/** brce064gwz_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of auto focus
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void brce064gwz_format_afdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  unsigned int af_offset = AF_OFFSET;
  short af_infi_up, af_marco_up;
  int dac_range;

  af_infi_up = buffer[af_offset + 1] << 8 | buffer[af_offset];
  af_marco_up = buffer[af_offset + 3] << 8 | buffer[af_offset + 2];

  e_ctrl->eeprom_data.afc.infinity_dac = af_infi_up;
  e_ctrl->eeprom_data.afc.macro_dac = af_marco_up;

  SHIGH("OTP:AF Macro DAC %d", (short)e_ctrl->eeprom_data.afc.macro_dac);
  SHIGH("OTP:AF Infinity DAC %d", (short)e_ctrl->eeprom_data.afc.infinity_dac);

  dac_range = (short)e_ctrl->eeprom_data.afc.macro_dac
    - (short)e_ctrl->eeprom_data.afc.infinity_dac;
  e_ctrl->eeprom_data.afc.infinity_margin = FAR_MARGIN;
  e_ctrl->eeprom_data.afc.macro_margin = NEAR_MARGIN;

}


/** print_matrix:
 *    @paramlist: address of pointer to
 *                   chromatix struct
 *
 * Prints out debug logs
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 */
static void print_matrix(float* paramlist)
{
  int j =0;
  for (j=0; j < MESH_HWROLLOFF_SIZE; j = j + 17) {
   SERR("%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f,"
     "%.2f, %.2f, %.2f, %.2f, %.2f, %.2f",
     paramlist[j], paramlist[j+1], paramlist[j+2], paramlist[j+3],
     paramlist[j+4], paramlist[j+5], paramlist[j+6], paramlist[j+7],
     paramlist[j+8], paramlist[j+9], paramlist[j+10], paramlist[j+11],
     paramlist[j+12], paramlist[j+13], paramlist[j+14], paramlist[j+15],
     paramlist[j+16]);
  }
}

/** brce064gwz_format_lscdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void brce064gwz_format_lscdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  unsigned char *a_r_gain, *a_gr_gain, *a_gb_gain, *a_b_gain;
  unsigned short i;

  a_r_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_R_OFFSET]);
  a_gr_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_GR_OFFSET]);
  a_b_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_B_OFFSET]);
  a_gb_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_GB_OFFSET]);

  for (i = 0; i < MESH_HWROLLOFF_SIZE; i++) {
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].r_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].r_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].r_gain[i] =
      (float)(*(a_r_gain+1) << 8 | *a_r_gain);
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gr_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].gr_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].gr_gain[i] =
      (float)(*(a_gr_gain+1) << 8 | *a_gr_gain);
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gb_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].gb_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].gb_gain[i] =
      (float)(*(a_gb_gain+1) << 8 | *a_gb_gain);
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].b_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].b_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].b_gain[i] =
      (float)(*(a_b_gain+1) << 8 | *a_b_gain);

    a_r_gain += 2;
    a_gr_gain += 2;
    a_gb_gain += 2;
    a_b_gain += 2;
  }

  e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].mesh_rolloff_table_size
    = MESH_HWROLLOFF_SIZE;
  e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].mesh_rolloff_table_size
    = MESH_HWROLLOFF_SIZE;
  e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].mesh_rolloff_table_size
    = MESH_HWROLLOFF_SIZE;

  SLOW("OTP: LSC MESH R MATRIX");
  print_matrix(e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].r_gain);
  SLOW("OTP: LSC MESH GR MATRIX");
  print_matrix(e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gr_gain);
  SLOW("OTP: LSC MESH GB MATRIX");
  print_matrix(e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gb_gain);
  SLOW("OTP: LSC MESH B MATRIX");
  print_matrix(e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].b_gain);
}

/** brce064gwz_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
static void brce064gwz_format_calibration_data(void *e_ctrl) {
  sensor_eeprom_data_t * ctrl = (sensor_eeprom_data_t *)e_ctrl;
  unsigned char *buffer = ctrl->eeprom_params.buffer;
  unsigned short crc = 0;
  datapresent = 0;

  SHIGH("OTP: total bytes: %d",ctrl->eeprom_params.num_bytes);
  if(ctrl->eeprom_params.num_bytes == 0 ||
    brce064gwz_check_empty_page(&buffer[AF_OFFSET]) == PAGE_EMPTY) {
    SERR("failed. OTP/EEPROM empty.");
    return;
  }

  datapresent = 1;

  brce064gwz_format_wbdata(ctrl);
  brce064gwz_format_afdata(ctrl);
  brce064gwz_format_lscdata(ctrl);
  brce064gwz_format_dcdata(ctrl);

}

/** rohm_brce064gwz_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* rohm_brce064gwz_eeprom_open_lib(void) {
  return &brce064gwz_lib_func_ptr;
}
