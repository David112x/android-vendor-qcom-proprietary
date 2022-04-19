/*============================================================================

Copyright (c) 2016 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "imx258_gt24c32_eeprom.h"

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
static void print_matrix(__attribute__((unused))float* paramlist)
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

/** imx258_gt24c32_eeprom_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void imx258_gt24c32_eeprom_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);

  e_items->is_wbc         = awb_present ? TRUE: FALSE;
  e_items->is_afc         = af_present ? TRUE: FALSE;
  e_items->is_lsc         = lsc_present ? TRUE: FALSE;
  e_items->is_dpc         = FALSE;
  e_items->is_insensor    = insensor_present? TRUE: FALSE;
  e_items->is_ois         = FALSE;
  e_items->is_dual        = dualc_present ? TRUE : FALSE;

  SLOW("is_wbc:%d,is_afc:%d,is_lsc:%d,is_dpc:%d,is_insensor:%d,\
    is_ois:%d, is_dual:%d",e_items->is_wbc,e_items->is_afc,
    e_items->is_lsc,e_items->is_dpc,e_items->is_insensor,
    e_items->is_ois,e_items->is_dual);
}

/** imx258_gt24c32_eeprom_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/

static void imx258_gt24c32_eeprom_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  wbcalib_data_t *wbc = &(e_ctrl->eeprom_data.wbc);
  unsigned int i;
  unsigned char flip = 0, mirror = 0;
  float awb_r_over_gr = 0.0, awb_b_over_gr = 0.0, awb_gb_over_gr = 0.0;
  unsigned int wb_offset = AWB_OFFSET+2;

  mirror = buffer[AWB_OFFSET];
  flip = buffer[AWB_OFFSET+1];
  awb_present = TRUE;

  awb_r_over_gr = (buffer[wb_offset] << 8 | buffer[wb_offset + 1]) /
   (float)QVALUE;
  awb_b_over_gr = (buffer[wb_offset + 2] << 8 | buffer[wb_offset + 3]) /
   (float)QVALUE;
  awb_gb_over_gr = (buffer[wb_offset + 4] << 8 | buffer[wb_offset + 5]) /
   (float)QVALUE;

  SLOW("OTP:AWB:D65 awb_r_over_gr: %f,awb_b_over_gr: %f,awb_gb_over_gr: %f",
    awb_r_over_gr, awb_b_over_gr, awb_gb_over_gr);

  /* Need to set wbc->gr_over_gb according to Bayer pattern and calibration data
    for "BGGR" and "GBRG" of bayer pattern, Gb/Gr of stored calibration data
      wbc->gr_over_gb = awb_gr_over_gb;
    for "RGGB" and "GRBG" bayer pattern, Gb/Gr of stored calibration data
      wbc->gr_over_gb = 1.0f/awb_gr_over_gb; */
  if (!mirror)
    wbc->gr_over_gb = 1.0f/awb_gb_over_gr;
  else
    wbc->gr_over_gb = awb_gb_over_gr;
  /* populate all light types from light source 1 */
  for (i = 0; i < AGW_AWB_MAX_LIGHT; i++) {
    wbc->r_over_g[i] = awb_r_over_gr;
    wbc->b_over_g[i] = awb_b_over_gr;
  }
}
/** imx258_gt24c32_eeprom_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void imx258_gt24c32_eeprom_format_lensshading(sensor_eeprom_data_t *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  uint16_t i,j;
  unsigned char flip, mirror;
  uint8_t *buffer = &ectrl->eeprom_params.buffer[LSC_OFFSET+2];
  lsc_present = FALSE;
  mirror = ectrl->eeprom_params.buffer[LSC_OFFSET];
  flip = ectrl->eeprom_params.buffer[LSC_OFFSET+1];

  for (j = 0; j < MESH_HWROLLOFF_SIZE; j++) {
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].r_gain[j]
      = ( buffer[4] & 0xC0) << 2 | buffer[0];
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gr_gain[j]
      = ( buffer[4] & 0x30) << 4 | buffer[1];
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gb_gain[j]
      = ( buffer[4] & 0x0C) << 6 | (buffer[2]);
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].b_gain[j]
      = ( buffer[4] & 0x03)<< 8 | (buffer[3]);

    buffer += 5;

    /* populate other light types */
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].r_gain[j]
      = ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].r_gain[j];
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].gr_gain[j]
      = ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gr_gain[j];
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].gb_gain[j]
      = ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gb_gain[j];
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].b_gain[j]
      = ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].b_gain[j];

    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].r_gain[j]
      = ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].r_gain[j];
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].gr_gain[j]
      = ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gr_gain[j];
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].gb_gain[j]
      =  ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gb_gain[j];
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].b_gain[j]
      = ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].b_gain[j];
  }

  ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].mesh_rolloff_table_size
  = ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].mesh_rolloff_table_size
  = ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].mesh_rolloff_table_size
  = MESH_HWROLLOFF_SIZE;

  for (i = 0; i < ROLLOFF_MAX_LIGHT; i++) {
    SLOW("MESH R MATRIX %d", i);
    print_matrix(ectrl->eeprom_data.lsc.lsc_calib[i].r_gain);
    SLOW("MESH GR MATRIX %d", i);
    print_matrix(ectrl->eeprom_data.lsc.lsc_calib[i].gr_gain);
    SLOW("MESH GB MATRIX %d", i);
    print_matrix(ectrl->eeprom_data.lsc.lsc_calib[i].gb_gain);
    SLOW("MESH B MATRIX %d", i);
    print_matrix(ectrl->eeprom_data.lsc.lsc_calib[i].b_gain);
  }

}

/** imx258_gt24c32_eeprom_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of auto focus
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void imx258_gt24c32_eeprom_format_afdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  unsigned int af_offset = AF_OFFSET;
  short af_infi_up, af_macro_up;
  int dac_range;

  af_present = TRUE;

  e_ctrl->eeprom_data.afc.infinity_dac =
    buffer[af_offset] << 8 | buffer[af_offset + 1];
  e_ctrl->eeprom_data.afc.macro_dac =
    buffer[af_offset + 2] << 8 | buffer[af_offset + 3];

  e_ctrl->eeprom_data.afc.starting_dac = e_ctrl->eeprom_data.afc.infinity_dac;
  SHIGH("Macro DAC %d,Infinity DAC %d, Starting DAC %d",
    (short)e_ctrl->eeprom_data.afc.macro_dac,
    (short)e_ctrl->eeprom_data.afc.infinity_dac,
    (short)e_ctrl->eeprom_data.afc.starting_dac);

  dac_range = (short)e_ctrl->eeprom_data.afc.macro_dac
    - (short)e_ctrl->eeprom_data.afc.infinity_dac;
  e_ctrl->eeprom_data.afc.macro_dac += MACRO_MARGIN * (float)dac_range;
  e_ctrl->eeprom_data.afc.infinity_dac += INF_MARGIN * (float)dac_range;
  e_ctrl->eeprom_data.afc.starting_dac = e_ctrl->eeprom_data.afc.infinity_dac;
  SHIGH("with margin: Macro DAC %d,Infinity DAC %d, Starting DAC %d",
    (short)e_ctrl->eeprom_data.afc.macro_dac,
    (short)e_ctrl->eeprom_data.afc.infinity_dac,
    (short)e_ctrl->eeprom_data.afc.starting_dac);
}

/** imx258_gt24c32_eeprom_format_pdafgain:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of pdaf gain
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void imx258_gt24c32_eeprom_format_pdafgain(sensor_eeprom_data_t *e_ctrl)
{

  pdaf_2D_cal_data_t     *pdafc_2d = &e_ctrl->eeprom_data.pdafc_2d;
  unsigned char                   *pdaf_ptr, *buffer;
  int                             i;

  SDBG("Enter");

  buffer   = e_ctrl->eeprom_params.buffer;
  pdaf_ptr = e_ctrl->eeprom_params.buffer + PDAF_GAIN_OFFSET;

  pdafc_2d->VersionNum = (unsigned short)(pdaf_ptr[0] << 8 | pdaf_ptr[1]);
  pdaf_ptr += 2;
  pdafc_2d->OffsetX = (unsigned short)(pdaf_ptr[0] << 8 | pdaf_ptr[1]);
  pdaf_ptr += 2;
  pdafc_2d->OffsetY = (unsigned short)(pdaf_ptr[0] << 8 | pdaf_ptr[1]);
  pdaf_ptr += 2;
  pdafc_2d->RatioX  = (unsigned short)(pdaf_ptr[0] << 8 | pdaf_ptr[1]);
  pdaf_ptr+=2;
  pdafc_2d->RatioY  = (unsigned short)(pdaf_ptr[0] << 8 | pdaf_ptr[1]);
  pdaf_ptr+=2;
  pdafc_2d->MapWidth= (unsigned short)(pdaf_ptr[0] << 8 | pdaf_ptr[1]);
  pdaf_ptr += 2;
  pdafc_2d->MapHeight=(unsigned short)(pdaf_ptr[0] << 8 | pdaf_ptr[1]);
  pdaf_ptr+=2;

  /* Left and Right Gain Map */
  for(i = 0; i < PDGAIN_LENGTH2D; i++){
    pdafc_2d->Left_GainMap[i] =
     (unsigned short)( pdaf_ptr[i * 2]  << 8 |
                       pdaf_ptr[i * 2 + 1]);
    pdafc_2d->Right_GainMap[i] =
     (unsigned short)( pdaf_ptr[i * 2 + PDGAIN_LENGTH2D * 2]  << 8 |
                       pdaf_ptr[i * 2 + 1 + PDGAIN_LENGTH2D * 2]);
   }

  /* PD Conversion coefficient 0 corresponds to the Best Cal Coefficient
   * defined at location 0x04B1. Values from 1 through 4 correspond to
   * other PD Conversion coefficients values specified from 0x04A9 through
   * 0x04B0.
  */
  pdafc_2d->PD_conversion_coeff[0] =
   (buffer[PDAF_KCOEF_OFFSET + 8] << 8 |
   buffer[PDAF_KCOEF_OFFSET + 9]);

  for(i = 1; i <= PD_KCALCOFF_MAX ; i++){
    pdafc_2d->PD_conversion_coeff[i] =
      (buffer[PDAF_KCOEF_OFFSET + (i-1)*2] << 8 |
      buffer[PDAF_KCOEF_OFFSET + ((i-1)*2) + 1]);
  }

  SLOW("-------------PDAF GAIN DUMP------------");
  SLOW("VersionNum %d, OffsetX %d, OffsetY %d, RatioX %d, RatioY %d,"
       "MapWidth %d, MapHeight %d, BEST PD_conversion_coeff %d",
       pdafc_2d->VersionNum,
       pdafc_2d->OffsetX,
       pdafc_2d->OffsetY,
       pdafc_2d->RatioX,
       pdafc_2d->RatioY,
       pdafc_2d->MapWidth,
       pdafc_2d->MapHeight,
       pdafc_2d->PD_conversion_coeff[0]
  );
  SLOW("-----------PD CONVERSION COEFF DUMP----------------");
  for(i = 1; i <= PD_KCALCOFF_MAX ; i++){
    SLOW("PDAF KCAL. Coeff.[%d] %d",
      i, pdafc_2d->PD_conversion_coeff[i]);
  }
  SLOW("-----------Left_GainMap----------------");
  for (i = 0; i < PDGAIN_LENGTH2D; i = i + 17) {
   SLOW("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,"
     "%d, %d, %d, %d, %d",
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
  SLOW("-----------Right_GainMap----------------");
  for (i = 0; i < PDGAIN_LENGTH2D; i = i + 17) {
   SLOW("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,"
     "%d, %d, %d, %d, %d",
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

  SDBG("Exit");
}

/* imx258_gt24c32_format_dcdata
*
* Format dual calibration data.
*
* This function executes in eeprom module context.
*
*/
static void imx258_gt24c32_format_dcdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  int i = 0, j = 0;

  e_ctrl->eeprom_data.dualc.dc_offset = DC_OFFSET;
  e_ctrl->eeprom_data.dualc.dc_size   = DC_SIZE;
  dualc_present = TRUE;

  /* Pointer to uncalibrated dual camera OTP data */
  e_ctrl->eeprom_data.dualc.system_calib_params.dc_otp_params =
    (void *)&buffer[DC_OFFSET];

  /* Size of dual camera OTP data in bytes */
  e_ctrl->eeprom_data.dualc.system_calib_params.dc_otp_size =
    e_ctrl->eeprom_data.dualc.dc_size;

  /* assign formatted data to dual camera system */
  e_ctrl->eeprom_data.dualc.system_calib_params.calibration_format_version
    = *(unsigned int *)&buffer[DC_OFFSET];

  /* assign formatted data to main camera */
  e_ctrl->eeprom_data.dualc.master_calib_params.focal_length = *(float *)&buffer[DC_MAIN];
  e_ctrl->eeprom_data.dualc.master_calib_params.native_sensor_resolution_width =
    *(unsigned short *)&buffer[DC_MAIN+4];
  e_ctrl->eeprom_data.dualc.master_calib_params.native_sensor_resolution_height=
    *(unsigned short *)&buffer[DC_MAIN+6];
  e_ctrl->eeprom_data.dualc.master_calib_params.calibration_resolution_width=
    *(unsigned short *)&buffer[DC_MAIN+8];
  e_ctrl->eeprom_data.dualc.master_calib_params.calibration_resolution_height=
    *(unsigned short *)&buffer[DC_MAIN+10];
  e_ctrl->eeprom_data.dualc.master_calib_params.focal_length_ratio=
    *(float *)&buffer[DC_MAIN+12];

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
    e_ctrl->eeprom_data.dualc.system_calib_params.relative_geometric_surface_parameters[j]
      = *(float *)&buffer[DC_OFFSET + 72 + j*4];

  e_ctrl->eeprom_data.dualc.system_calib_params.relative_principle_point_x_offset
    = *(float *)&buffer[DC_OTHERS];
  e_ctrl->eeprom_data.dualc.system_calib_params.relative_principle_point_y_offset
    = *(float *)&buffer[DC_OTHERS + 4];
  e_ctrl->eeprom_data.dualc.system_calib_params.relative_position_flag
    = *(unsigned short *)&buffer[DC_OTHERS + 8];
  e_ctrl->eeprom_data.dualc.system_calib_params.relative_baseline_distance =
    *(float *)&buffer[DC_OTHERS + 10];
  e_ctrl->eeprom_data.dualc.system_calib_params.master_sensor_mirror_flip_setting
    = *(unsigned short *)&buffer[DC_OTHERS + 14];
  e_ctrl->eeprom_data.dualc.system_calib_params.aux_sensor_mirror_flip_setting
    = *(unsigned short *)&buffer[DC_OTHERS + 16];
  e_ctrl->eeprom_data.dualc.system_calib_params.module_orientation_flag
    = *(unsigned short *)&buffer[DC_OTHERS + 18];
  e_ctrl->eeprom_data.dualc.system_calib_params.rotation_flag
    = *(unsigned short *)&buffer[DC_OTHERS + 20];

  /* AEC Calibration data */
  e_ctrl->eeprom_data.dualc.system_calib_params.brightness_ratio
    = (unsigned short)(buffer[AE_SYNC] | (buffer[AE_SYNC+1] << 8));
  e_ctrl->eeprom_data.dualc.system_calib_params.ref_aux_gain
    = (unsigned short)(buffer[AE_SYNC+2] | (buffer[AE_SYNC+3] << 8));
  e_ctrl->eeprom_data.dualc.system_calib_params.ref_aux_linecount
    = (unsigned short)(buffer[AE_SYNC+4] | (buffer[AE_SYNC+5] << 8));
  e_ctrl->eeprom_data.dualc.system_calib_params.ref_master_gain
    = (unsigned short)(buffer[AE_SYNC+6] | (buffer[AE_SYNC+7] << 8));
  e_ctrl->eeprom_data.dualc.system_calib_params.ref_master_linecount
    = (unsigned short)(buffer[AE_SYNC+8] | (buffer[AE_SYNC+9] << 8));
  e_ctrl->eeprom_data.dualc.system_calib_params.ref_master_color_temperature
    = (unsigned short)(buffer[AE_SYNC+10] | (buffer[AE_SYNC+11] << 8));
}

/** imx258_gt24c32_eeprom_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void imx258_gt24c32_eeprom_format_calibration_data(void *e_ctrl) {
  sensor_eeprom_data_t * ctrl = (sensor_eeprom_data_t *)e_ctrl;
  module_info_t        *module_info;
  unsigned short       rc     = SENSOR_FAILURE;

  RETURN_VOID_ON_NULL(ctrl);

  SLOW("Total bytes in OTP buffer: %d", ctrl->eeprom_params.num_bytes);

  if (!ctrl->eeprom_params.buffer || !ctrl->eeprom_params.num_bytes) {
    SERR("Buff pointer %p buffer size %d", ctrl->eeprom_params.buffer,
      ctrl->eeprom_params.num_bytes);
    return;
  }

  /* Print module info */
  module_info = (module_info_t *)
    (ctrl->eeprom_params.buffer + MODULE_INFO_OFFSET);
  SLOW("Vendor ID : 0x%x", (module_info->vendor_id_h << 8) | module_info->vendor_id_l);
  SLOW("Y/M/D : %d/%d/%d",
    module_info->year, module_info->month, module_info->day);

  /* bypass the eeprom page crc check for allowing manual calibration */
  imx258_gt24c32_eeprom_format_pdafgain(ctrl);

  imx258_gt24c32_eeprom_format_wbdata(ctrl);
  imx258_gt24c32_eeprom_format_lensshading(ctrl);
  imx258_gt24c32_eeprom_format_afdata(ctrl);
  imx258_gt24c32_format_dcdata(ctrl);
}

/** imx258_gt24c32_eeprom_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* imx258_gt24c32_eeprom_open_lib(void) {
  return &imx258_gt24c32_eeprom_lib_func_ptr;
}
