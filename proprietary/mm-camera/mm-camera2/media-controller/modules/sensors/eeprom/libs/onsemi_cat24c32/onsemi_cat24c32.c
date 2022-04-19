/*============================================================================

  Copyright (c) 2014-2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "onsemi_cat24c32.h"

/** cat24c32_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void cat24c32_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);

  e_items->is_wbc = datapresent ? TRUE : FALSE;
  e_items->is_afc = datapresent ? TRUE : FALSE;
  e_items->is_lsc = datapresent ? TRUE : FALSE;
  e_items->is_dpc = FALSE;
  e_items->is_insensor = datapresent ? TRUE : FALSE;
  e_items->is_ois = FALSE;

  SLOW("In OTP:is_wbc:%d,is_afc:%d,is_lsc:%d,is_dpc:%d,is_insensor:%d,\
is_ois:%d",e_items->is_wbc,e_items->is_afc,
    e_items->is_lsc,e_items->is_dpc,e_items->is_insensor,
    e_items->is_ois);

}

/** cat24c32_check_empty_page:
 *    @buff: address of page buffer
 *
 * Checks if the page has non zero data
 *
 * Return:
 * unsigned char :  PAGE_EMPTY / PAGE_NOT_EMPTY
 **/
static unsigned char cat24c32_check_empty_page( unsigned char *buff )
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

/** sony_imx230_eeprom_format_spcdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of pdaf calibration:
 *   shield pixel correction (SPC)
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void cat24c32_format_spcdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  unsigned int spc_offset = SPC_OFFSET;
  unsigned int i = 0;

  g_reg_setting.reg_setting = g_reg_array;
  g_reg_setting.size = SPC_SETTING_SIZE;
  g_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
  g_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;
  g_reg_setting.delay = 0;

  for(i = 0;i < SPC_SETTING_SIZE;i++) {
    g_reg_array[i].reg_addr = SPC_ADDR + i;
    g_reg_array[i].reg_data = buffer[spc_offset + i];
    g_reg_array[i].delay = 0;
    SLOW("OTP: SPCData[%d]: addr: 0x%X, data: 0x%X",
      i, g_reg_array[i].reg_addr, g_reg_array[i].reg_data);
  }
}

/** sony_imx230_eeprom_format_dccdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of pdaf calibration:
 *   defocus conversion coefficient  (DCC)
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void cat24c32_format_dccdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  unsigned int pdaf_offset = DCC_OFFSET;
  unsigned int i = 0;
  pdafcalib_data_t *pdaf_data = &e_ctrl->eeprom_data.pdafc;

  pdaf_data->XKnotNumSlopeOffset = PD_KNOT_X;
  pdaf_data->YKnotNumSlopeOffset = PD_KNOT_Y;

  for(i = 0;i < PD_KNOT_X * PD_KNOT_Y;i++) {
    /*SlopeData format: u6.10 */
    pdaf_data->SlopeData[i] = (float)(buffer[pdaf_offset + i * 2]<<8 |
      buffer[pdaf_offset + i * 2 + 1]) / 1024.0;
    pdaf_data->OffsetData[i] = 0;
    SLOW("OTP SlopeData[%d]: u6.10: 0x%x, float: %f", i,
     (buffer[pdaf_offset + i * 2]<<8 | buffer[pdaf_offset + i * 2 + 1]),
     pdaf_data->SlopeData[i]);
  }

  for(i = 0;i < PD_KNOT_X;i++)
    pdaf_data->XAddressKnotSlopeOffset[i] = PD_OFFSET_X + PD_AREA_X * i;

  for(i = 0;i < PD_KNOT_Y;i++)
    pdaf_data->YAddressKnotSlopeOffset[i] = PD_OFFSET_Y + PD_AREA_Y * i;

}

/** cat24c32_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void cat24c32_format_wbdata(sensor_eeprom_data_t *e_ctrl)
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

  SLOW("OTP:AWB:D65 awb_r_over_gr: %f,awb_b_over_gr: %f,awb_gr_over_gb: %f",
    awb_r_over_gr, awb_b_over_gr, awb_gr_over_gb);

  /* Need to set wbc->gr_over_gb according to Bayer pattern and calibration data
    for "BGGR" and "GBRG" of bayer pattern, Gb/Gr of stored calibration data
      wbc->gr_over_gb = awb_gr_over_gb;
    for "RGGB" and "GRBG" bayer pattern, Gb/Gr of stored calibration data
      wbc->gr_over_gb = 1.0f/awb_gr_over_gb; */
  wbc->gr_over_gb = awb_gr_over_gb;
  /* populate all light types from light source 1 */
  for (i = 0; i < AGW_AWB_MAX_LIGHT; i++) {
    wbc->r_over_g[i] = awb_r_over_gr;
    wbc->b_over_g[i] = awb_b_over_gr;
  }
}

/** cat24c32_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of auto focus
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void cat24c32_format_afdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  unsigned int af_offset = AF_OFFSET;
  short af_infi_up, af_marco_up;
  int dac_range;

  af_infi_up = buffer[af_offset + 1] << 8 | buffer[af_offset];
  af_marco_up = buffer[af_offset + 3] << 8 | buffer[af_offset + 2];

  /*convert signed 16 bit to signed 12 bits*/
  e_ctrl->eeprom_data.afc.infinity_dac = af_infi_up / 16;
  e_ctrl->eeprom_data.afc.macro_dac = af_marco_up / 16;

  hall_offset = buffer[HALL_OFFSET];
  hall_bias = buffer[HALL_OFFSET + 1];

  SHIGH("OTP:(12 bit)AF Macro DAC: %d, Infinity DAC: %d",
    (short)e_ctrl->eeprom_data.afc.macro_dac,
    (short)e_ctrl->eeprom_data.afc.infinity_dac);
  SLOW("OTP: hall_offset: 0x%x, hall_bias: 0x%x", hall_offset,hall_bias);

  dac_range = (short)e_ctrl->eeprom_data.afc.macro_dac
    - (short)e_ctrl->eeprom_data.afc.infinity_dac;
  e_ctrl->eeprom_data.afc.macro_dac += NEAR_MARGIN * (float)dac_range;
  e_ctrl->eeprom_data.afc.infinity_dac += FAR_MARGIN * (float)dac_range;
  e_ctrl->eeprom_data.afc.starting_dac = e_ctrl->eeprom_data.afc.infinity_dac;
  SHIGH("with margin: Macro DAC %d,Infinity DAC %d, Starting DAC %d",
    (short)e_ctrl->eeprom_data.afc.macro_dac,
    (short)e_ctrl->eeprom_data.afc.infinity_dac,
    (short)e_ctrl->eeprom_data.afc.starting_dac);

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

/** cat24c32_format_lscdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void cat24c32_format_lscdata(sensor_eeprom_data_t *e_ctrl)
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
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].r_gain[i] =
      (float)(*(a_r_gain+1) << 8 | *a_r_gain);
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gr_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].gr_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].gr_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].gr_gain[i] =
      (float)(*(a_gr_gain+1) << 8 | *a_gr_gain);
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gb_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].gb_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].gb_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].gb_gain[i] =
      (float)(*(a_gb_gain+1) << 8 | *a_gb_gain);
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].b_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].b_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].b_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].b_gain[i] =
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
  e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].mesh_rolloff_table_size
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

/** cat24c32_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
static void cat24c32_format_calibration_data(void *e_ctrl) {
  sensor_eeprom_data_t * ctrl = (sensor_eeprom_data_t *)e_ctrl;
  unsigned char *buffer = ctrl->eeprom_params.buffer;
  unsigned short crc = 0;
  datapresent = 0;

  SLOW("OTP: total bytes: %d",ctrl->eeprom_params.num_bytes);
  if(ctrl->eeprom_params.num_bytes == 0 ||
    cat24c32_check_empty_page(buffer) == PAGE_EMPTY) {
    SERR("failed. OTP/EEPROM empty.");
    return;
  }

  datapresent = 1;

  cat24c32_format_wbdata(ctrl);
  cat24c32_format_afdata(ctrl);
  cat24c32_format_lscdata(ctrl);
  cat24c32_format_spcdata(ctrl);
  cat24c32_format_dccdata(ctrl);

}

/** cat24c32_get_ois_raw_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *    @data: point to the destination camera_i2c_seq_reg_setting
 *
 * Get the all the calibration registers setting for ois
 *
 * This function executes in eeprom module context.
 *
 * No return value.
 **/
static int cat24c32_get_raw_data(void *e_ctrl, void *data)
{
  RETURN_ERR_ON_NULL(e_ctrl, SENSOR_FAILURE);
  RETURN_ERR_ON_NULL(data, SENSOR_FAILURE);

  memcpy(data, &g_reg_setting, sizeof(g_reg_setting));

  return SENSOR_SUCCESS;
}

/** cat24c32_autofocus_calibration:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * performs autofocus calibration assuming 2 regions
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/
static int cat24c32_autofocus_calibration(void *e_ctrl) {
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *) e_ctrl;
  int                 i = 0;
  actuator_tuned_params_t *af_driver_tune = NULL;
  actuator_params_t       *af_params = NULL;
  unsigned int                total_steps = 0;
  short                 macro_dac, infinity_dac;
  unsigned int                qvalue = 0;

  /* Validate params */
  RETURN_ON_NULL(e_ctrl);
  RETURN_ON_NULL(ectrl->eeprom_afchroma.af_driver_ptr);

  af_driver_tune =
    &(ectrl->eeprom_afchroma.af_driver_ptr->actuator_tuned_params);
  af_params = &(ectrl->eeprom_afchroma.af_driver_ptr->actuator_params);
  /* Get the total steps */
  total_steps = af_driver_tune->region_params[af_driver_tune->region_size - 1].
    step_bound[0] - af_driver_tune->region_params[0].step_bound[1];

  if (!total_steps) {
    SERR("Invalid total_steps count = 0");
    return FALSE;
  }

  /* LC898212: set HALL offset/bias calibration data */
  for(i = 0;i < af_params->init_setting_size;i++) {
    if(af_params->init_settings[i].reg_addr == 0x28) {
      af_params->init_settings[i].reg_data = hall_offset<<8 | hall_bias;
      break;
    }
  }

  /* Get the calibrated steps */
  total_steps = af_driver_tune->region_params[af_driver_tune->region_size - 1].
    step_bound[0] - af_driver_tune->region_params[0].step_bound[1];

  /* adjust af_driver_ptr */
  af_driver_tune->initial_code = ectrl->eeprom_data.afc.infinity_dac;

 /* LC898212: adjust code_per_step, assuming one region only */
  macro_dac = ectrl->eeprom_data.afc.macro_dac;
  infinity_dac = ectrl->eeprom_data.afc.infinity_dac;

  qvalue = af_driver_tune->region_params[0].qvalue;
  if(qvalue >= 1 && qvalue <= 4096)
    af_driver_tune->region_params[0].code_per_step =
     (macro_dac - infinity_dac) / (float)total_steps * qvalue;

  SLOW("adjusted code_per_step: %d, qvalue: %d",
    af_driver_tune->region_params[0].code_per_step, qvalue);

  return TRUE;
}

/** onsemi_cat24c32_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* onsemi_cat24c32_eeprom_open_lib(void) {
  return &cat24c32_lib_func_ptr;
}
