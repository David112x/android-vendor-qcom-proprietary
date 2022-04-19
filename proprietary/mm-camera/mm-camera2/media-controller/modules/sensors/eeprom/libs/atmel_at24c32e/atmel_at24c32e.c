/*============================================================================

  Copyright (c) 2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "atmel_at24c32e.h"

/** at24c32e_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void at24c32e_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);

  e_items->is_wbc = datapresent ? TRUE : FALSE;
  e_items->is_afc = datapresent ? TRUE : FALSE;
  e_items->is_lsc = datapresent ? TRUE : FALSE;
  e_items->is_dpc = FALSE;
  e_items->is_insensor = datapresent ? TRUE : FALSE;
  e_items->is_ois = FALSE;
}

/** at24c32e_check_empty_page:
 *    @buff: address of page buffer
 *
 * Checks if the page has non zero data
 *
 * Return:
 * unsigned char :  PAGE_EMPTY / PAGE_NOT_EMPTY
 **/
static unsigned char at24c32e_check_empty_page( unsigned char *buff )
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
static void at24c32e_format_spcdata(sensor_eeprom_data_t *e_ctrl)
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
static void at24c32e_format_dccdata(sensor_eeprom_data_t *e_ctrl)
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

/** at24c32e_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void at24c32e_format_wbdata(sensor_eeprom_data_t *e_ctrl)
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

/** at24c32e_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of auto focus
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void at24c32e_format_afdata(sensor_eeprom_data_t *e_ctrl)
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

  hall_bias = buffer[HALL_BIAS];
  hall_offset= buffer[HALL_BIAS + 1];

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

/** at24c32e_format_lscdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void at24c32e_format_lscdata(sensor_eeprom_data_t *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  uint16_t i,j;
  uint8_t *buffer = &ectrl->eeprom_params.buffer[LSC_OFFSET];

  for (j = 0; j < MESH_HWROLLOFF_SIZE; j++) {
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].r_gain[j]
      = ( buffer[4] & 0b11000000) << 2 | buffer[0];
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gr_gain[j]
      = ( buffer[4] & 0b00110000) << 4 | buffer[1];
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gb_gain[j]
      = ( buffer[4] & 0b00001100) << 6 | (buffer[2]);
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].b_gain[j]
      = ( buffer[4] & 0b00000011)<< 8 | (buffer[3]);

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

/** at24c32e_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
static void at24c32e_format_calibration_data(void *e_ctrl) {
  sensor_eeprom_data_t * ctrl = (sensor_eeprom_data_t *)e_ctrl;
  unsigned char *buffer = ctrl->eeprom_params.buffer;
  unsigned short crc = 0;
  datapresent = 0;

  SLOW("OTP: total bytes: %d",ctrl->eeprom_params.num_bytes);
  if(ctrl->eeprom_params.num_bytes == 0 ||
    at24c32e_check_empty_page(buffer) == PAGE_EMPTY) {
    SERR("failed. OTP/EEPROM empty.");
    return;
  }

  datapresent = 1;

  at24c32e_format_wbdata(ctrl);
  at24c32e_format_afdata(ctrl);
  at24c32e_format_lscdata(ctrl);
  at24c32e_format_spcdata(ctrl);
  at24c32e_format_dccdata(ctrl);

}

/** at24c32e_get_ois_raw_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *    @data: point to the destination camera_i2c_seq_reg_setting
 *
 * Get the all the calibration registers setting for ois
 *
 * This function executes in eeprom module context.
 *
 * No return value.
 **/
static int at24c32e_get_raw_data(void *e_ctrl, void *data)
{
  RETURN_ERR_ON_NULL(e_ctrl, SENSOR_FAILURE);
  RETURN_ERR_ON_NULL(data, SENSOR_FAILURE);

  memcpy(data, &g_reg_setting, sizeof(g_reg_setting));

  return SENSOR_SUCCESS;
}

/** at24c32e_autofocus_calibration:
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
static int at24c32e_autofocus_calibration(void *e_ctrl) {
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

/** atmel_at24c32e_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* atmel_at24c32e_eeprom_open_lib(void) {
  return &at24c32e_lib_func_ptr;
}
