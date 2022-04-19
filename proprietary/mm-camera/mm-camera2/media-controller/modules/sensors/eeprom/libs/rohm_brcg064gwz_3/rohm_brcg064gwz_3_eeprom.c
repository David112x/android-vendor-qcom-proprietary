/*============================================================================

Copyright (c) 2016 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "rohm_brcg064gwz_3_eeprom.h"

/** brcg064gwz_3_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void brcg064gwz_3_get_calibration_items(void *e_ctrl)
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

/** brcg064gwz_3_check_empty_page:
 *    @buff: address of page buffer
 *
 * Checks if the page has non zero data
 *
 * Return:
 * unsigned char :  PAGE_EMPTY / PAGE_NOT_EMPTY
 **/
static unsigned char brcg064gwz_3_check_empty_page( unsigned char *buff )
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

/** brcg064gwz_3_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void brcg064gwz_3_format_wbdata(sensor_eeprom_data_t *e_ctrl)
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

/** brcg064gwz_3_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of auto focus
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void brcg064gwz_3_format_afdata(sensor_eeprom_data_t *e_ctrl)
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
   SLOW("%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f,"
     "%.2f, %.2f, %.2f, %.2f, %.2f, %.2f",
     paramlist[j], paramlist[j+1], paramlist[j+2], paramlist[j+3],
     paramlist[j+4], paramlist[j+5], paramlist[j+6], paramlist[j+7],
     paramlist[j+8], paramlist[j+9], paramlist[j+10], paramlist[j+11],
     paramlist[j+12], paramlist[j+13], paramlist[j+14], paramlist[j+15],
     paramlist[j+16]);
  }
}

/** brcg064gwz_3_format_lscdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void brcg064gwz_3_format_lscdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  unsigned char *a_r_gain, *a_gr_gain, *a_gb_gain, *a_b_gain;
  unsigned short i;

  a_r_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_R_OFFSET]);
  a_gr_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_GR_OFFSET]);
  a_gb_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_GB_OFFSET]);
  a_b_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_B_OFFSET]);

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

/** brcg064gwz_3_format_pdafdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * populate the data structure of pdaf calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void brcg064gwz_3_format_pdafdata(sensor_eeprom_data_t *e_ctrl)
{
  pdaf_2D_cal_data_t          *pdafc_2d = &e_ctrl->eeprom_data.pdafc_2d;
  unsigned char               *buffer = e_ctrl->eeprom_params.buffer;
  unsigned short               i = 0;
  int                          gain_offset = PDGAIN_OFFSET;
  int                          dcc_offset = DCC_OFFSET;

  pdafc_2d->VersionNum = buffer[gain_offset] << 8 | buffer[gain_offset+1];
  gain_offset += 2;
  pdafc_2d->MapWidth = buffer[gain_offset] << 8 | buffer[gain_offset+1];
  gain_offset += 2;
  pdafc_2d->MapHeight = buffer[gain_offset] << 8 | buffer[gain_offset+1];
  gain_offset += 2;
  for (i = 0; i < 221; i++) {
    pdafc_2d->Left_GainMap[i] = buffer[gain_offset] << 8 | buffer[gain_offset+1];
    gain_offset += 2;
  }
  for (i = 0; i < 221; i++) {
    pdafc_2d->Right_GainMap[i] = buffer[gain_offset] << 8 | buffer[gain_offset+1];
    gain_offset += 2;
  }

  pdafc_2d->Q_factor_DCC =  buffer[dcc_offset] << 8 | buffer[dcc_offset+1];
  dcc_offset += 2;
  pdafc_2d->MapWidth_DCC=  buffer[dcc_offset] << 8 | buffer[dcc_offset+1];
  dcc_offset += 2;
  pdafc_2d->MapHeight_DCC=  buffer[dcc_offset] << 8 | buffer[dcc_offset+1];
  dcc_offset += 2;

  for (i = 0; i < 48; i++) {
    pdafc_2d->PD_conversion_coeff[i] =
      buffer[dcc_offset] << 8 | buffer[dcc_offset+1];
    dcc_offset += 2;
  }

  SLOW("-------------PDAF GAIN DUMP------------");
  SLOW("VersionNum: %d, OffsetX %d, OffsetY %d, RatioX %d, RatioY %d,"
       "MapWidth %d, MapHeight %d",
       pdafc_2d->VersionNum,
       pdafc_2d->OffsetX,
       pdafc_2d->OffsetY,
       pdafc_2d->RatioX,
       pdafc_2d->RatioY,
       pdafc_2d->MapWidth,
       pdafc_2d->MapHeight);
  SLOW("-----------Left_GainMap----------------");
  for (i = 0; i < 221; i = i + 17) {
   SLOW("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
     "%d, %d, %d, %d, %d",
     pdafc_2d->Left_GainMap[i],    pdafc_2d->Left_GainMap[i+1],
     pdafc_2d->Left_GainMap[i+2],  pdafc_2d->Left_GainMap[i+3],
     pdafc_2d->Left_GainMap[i+4],  pdafc_2d->Left_GainMap[i+5],
     pdafc_2d->Left_GainMap[i+6],  pdafc_2d->Left_GainMap[i+7],
     pdafc_2d->Left_GainMap[i+8],  pdafc_2d->Left_GainMap[i+9],
     pdafc_2d->Left_GainMap[i+10], pdafc_2d->Left_GainMap[i+11],
     pdafc_2d->Left_GainMap[i+12], pdafc_2d->Left_GainMap[i+13],
     pdafc_2d->Left_GainMap[i+14], pdafc_2d->Left_GainMap[i+15],
     pdafc_2d->Left_GainMap[i+16]);
  }
  SLOW("-----------Right_GainMap----------------");
  for (i = 0; i < 221; i = i + 17) {
   SLOW("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
     "%d, %d, %d, %d, %d",
     pdafc_2d->Right_GainMap[i],    pdafc_2d->Right_GainMap[i+1],
     pdafc_2d->Right_GainMap[i+2],  pdafc_2d->Right_GainMap[i+3],
     pdafc_2d->Right_GainMap[i+4],  pdafc_2d->Right_GainMap[i+5],
     pdafc_2d->Right_GainMap[i+6],  pdafc_2d->Right_GainMap[i+7],
     pdafc_2d->Right_GainMap[i+8],  pdafc_2d->Right_GainMap[i+9],
     pdafc_2d->Right_GainMap[i+10], pdafc_2d->Right_GainMap[i+11],
     pdafc_2d->Right_GainMap[i+12], pdafc_2d->Right_GainMap[i+13],
     pdafc_2d->Right_GainMap[i+14], pdafc_2d->Right_GainMap[i+15],
     pdafc_2d->Right_GainMap[i+16]);
  }
  SLOW("-----------DCC----------------");
  SLOW("VersionNum: %d, MapWidth %d, MapHeight %d, Q factor %d",
   pdafc_2d->VersionNum_DCC,
   pdafc_2d->MapWidth_DCC,
   pdafc_2d->MapHeight_DCC,
   pdafc_2d->Q_factor_DCC);
  for (i = 0; i < 48; i = i + 8) {
   SLOW("%d, %d, %d, %d, %d, %d, %d, %d,",
     pdafc_2d->PD_conversion_coeff[i],    pdafc_2d->PD_conversion_coeff[i+1],
     pdafc_2d->PD_conversion_coeff[i+2],  pdafc_2d->PD_conversion_coeff[i+3],
     pdafc_2d->PD_conversion_coeff[i+4],  pdafc_2d->PD_conversion_coeff[i+5],
     pdafc_2d->PD_conversion_coeff[i+6],  pdafc_2d->PD_conversion_coeff[i+7]);
  }
}

/** brcg064gwz_3_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
static void brcg064gwz_3_format_calibration_data(void *e_ctrl) {
  sensor_eeprom_data_t * ctrl = (sensor_eeprom_data_t *)e_ctrl;
  unsigned char *buffer = ctrl->eeprom_params.buffer;
  unsigned short crc = 0;
  datapresent = 0;

  SHIGH("OTP: total bytes: %d",ctrl->eeprom_params.num_bytes);
  if(ctrl->eeprom_params.num_bytes == 0 ||
    brcg064gwz_3_check_empty_page(&buffer[AF_OFFSET]) == PAGE_EMPTY) {
    SERR("failed. OTP/EEPROM empty.");
    return;
  }

  datapresent = 1;

  brcg064gwz_3_format_pdafdata(ctrl);
  brcg064gwz_3_format_wbdata(ctrl);
  brcg064gwz_3_format_afdata(ctrl);
  brcg064gwz_3_format_lscdata(ctrl);
}

/** rohm_brcg064gwz_3_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* rohm_brcg064gwz_3_eeprom_open_lib(void) {
  return &brcg064gwz_3_lib_func_ptr;
}
