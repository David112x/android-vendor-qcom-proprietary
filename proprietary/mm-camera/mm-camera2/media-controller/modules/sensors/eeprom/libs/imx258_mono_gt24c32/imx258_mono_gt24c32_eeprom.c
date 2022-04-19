/*============================================================================

Copyright (c) 2016 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "imx258_mono_gt24c32_eeprom.h"

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

/** imx258_mono_gt24c32_eeprom_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void imx258_mono_gt24c32_eeprom_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);

  e_items->is_wbc         = awb_present ? TRUE: FALSE;
  e_items->is_afc         = af_present ? TRUE: FALSE;
  e_items->is_lsc         = lsc_present ? TRUE: FALSE;
  e_items->is_dpc         = FALSE;
  e_items->is_insensor    = insensor_present? TRUE: FALSE;
  e_items->is_ois         = FALSE;
  e_items->is_dual        = FALSE;

  SLOW("is_wbc:%d,is_afc:%d,is_lsc:%d,is_dpc:%d,is_insensor:%d,\
is_ois:%d, is_dual:%d",e_items->is_wbc,e_items->is_afc,
    e_items->is_lsc,e_items->is_dpc,e_items->is_insensor,
    e_items->is_ois, e_items->is_dual);
}

/** imx258_mono_gt24c32_eeprom_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/

static void imx258_mono_gt24c32_eeprom_format_lensshading(sensor_eeprom_data_t *e_ctrl)
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

/** imx258_mono_gt24c32_eeprom_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void imx258_mono_gt24c32_eeprom_format_afdata(sensor_eeprom_data_t *e_ctrl)
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

/** imx258_mono_gt24c32_eeprom_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void imx258_mono_gt24c32_eeprom_format_calibration_data(void *e_ctrl) {
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

  imx258_mono_gt24c32_eeprom_format_lensshading(ctrl);
  imx258_mono_gt24c32_eeprom_format_afdata(ctrl);
}

/** imx258_mono_gt24c32_eeprom_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* imx258_mono_gt24c32_eeprom_open_lib(void) {
  return &imx258_mono_gt24c32_eeprom_lib_func_ptr;
}
