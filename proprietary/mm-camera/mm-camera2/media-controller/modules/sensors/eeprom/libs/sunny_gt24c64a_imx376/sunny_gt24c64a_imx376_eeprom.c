/*============================================================================

Copyright (c) 2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "sunny_gt24c64a_imx376_eeprom.h"

/** sunny_gt24c64a_imx376_eeprom_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_gt24c64a_imx376_eeprom_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);

  e_items->is_wbc = awb_present ? TRUE : FALSE;
  e_items->is_afc = af_present ? TRUE : FALSE;
  e_items->is_lsc = lsc_present ? TRUE : FALSE;
  e_items->is_dpc = FALSE;
  e_items->is_insensor = FALSE;
  e_items->is_ois = FALSE;

  SLOW("is_wbc:%d,is_afc:%d,is_lsc:%d,is_dpc:%d,is_insensor:%d,\
  is_ois:%d",e_items->is_wbc,e_items->is_afc,
    e_items->is_lsc,e_items->is_dpc,e_items->is_insensor,
    e_items->is_ois);

}
/** sunny_gt24c64a_imx376_eeprom_get_module_info:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 *   Get Module detailes
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_gt24c64a_imx376_eeprom_get_module_info(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char flag;
  module_info_t *module_info;

  SDBG("Enter");

  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[MODULE_INFO_OFFSET];
  if (flag != VALID_FLAG) {
    SERR("empty module info!");
    return;
  }

  /* Print module info */
  module_info = (module_info_t *)
    (e_ctrl->eeprom_params.buffer + MODULE_INFO_OFFSET + 0x0C);
  SLOW("Module ID : 0x%x", module_info->vendor_id);
  SLOW("Y/M/D : %d/%d/%d",
    module_info->year, module_info->month, module_info->day);

  SDBG("Exit");
}


/** sunny_gt24c64a_imx376_eeprom_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_gt24c64a_imx376_eeprom_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  awb_data_t    *wb;
  float         r_over_gr, b_over_gb, gr_over_gb;
  float         r_avg, gr_avg, gb_avg, b_avg;
  int           i;

  SDBG("Enter");
  /* Check validity */
  awb_present = e_ctrl->eeprom_params.buffer[AWB_OFFSET];
  if (awb_present != VALID_FLAG) {
    SERR("empty module info!");
    return;
  }

  /* Get AWB data */
  wb = (awb_data_t *)(e_ctrl->eeprom_params.buffer + AWB_OFFSET + 1);

  r_avg = ((float)((wb->r_avg_h << 8) | wb->r_avg_l));
  gr_avg = ((float)((wb->gr_avg_h << 8) | wb->gr_avg_l));
  gb_avg = ((float)((wb->gb_avg_h << 8) | wb->gb_avg_l));
  b_avg = ((float)((wb->b_avg_h << 8) | wb->b_avg_l));
  r_over_gr = r_avg / gr_avg;
  b_over_gb = b_avg / gb_avg;
  gr_over_gb = gr_avg / gb_avg;

  SLOW("AWB : r/gr = %f", r_over_gr);
  SLOW("AWB : b/gb = %f", b_over_gb);
  SLOW("AWB : gr/gb = %f", gr_over_gb);

  for (i = 0; i < AGW_AWB_MAX_LIGHT; i++) {
    e_ctrl->eeprom_data.wbc.r_over_g[i] = r_over_gr;
    e_ctrl->eeprom_data.wbc.b_over_g[i] = b_over_gb;
  }
  e_ctrl->eeprom_data.wbc.gr_over_gb = gr_over_gb;
  SDBG("Exit");
}

/** sunny_gt24c64a_imx376_eeprom_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_gt24c64a_imx376_eeprom_format_lensshading(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char  flag;
  unsigned short i, light;
  unsigned char  *lsc_buf;
  float          gain_r, gain_gr, gain_gb, gain_b;

  SDBG("Enter");
  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[LSC_OFFSET];
  if (flag != VALID_FLAG) {
   lsc_present = FALSE;
   SERR("LSC : empty or invalid data");
   return;
  }
  lsc_present = TRUE;

  lsc_buf = e_ctrl->eeprom_params.buffer + LSC_OFFSET + 1;

  for (light = 0; light < ROLLOFF_MAX_LIGHT; light++) {
    e_ctrl->eeprom_data.lsc.lsc_calib[light].mesh_rolloff_table_size =
        LSC_GRID_SIZE;
  }

  /* read gain */
  for (i = 0; i < LSC_GRID_SIZE * 2; i += 2) {
    gain_r  = lsc_buf[i + 0] << 8 | lsc_buf[i + 1];
    gain_gr = lsc_buf[i + (LSC_GRID_SIZE<<1)] << 8 | lsc_buf[i + (LSC_GRID_SIZE<<1) + 1];
    gain_gb = lsc_buf[i + (LSC_GRID_SIZE<<2)] << 8 | lsc_buf[i + (LSC_GRID_SIZE<<2) + 1];
    gain_b  = lsc_buf[i + (LSC_GRID_SIZE*6) ] << 8 | lsc_buf[i + (LSC_GRID_SIZE*6) + 1];

    for (light = 0; light < ROLLOFF_MAX_LIGHT; light++) {
      e_ctrl->eeprom_data.lsc.lsc_calib[light].r_gain[i>>1]  = gain_r;
      e_ctrl->eeprom_data.lsc.lsc_calib[light].gr_gain[i>>1] = gain_gr;
      e_ctrl->eeprom_data.lsc.lsc_calib[light].gb_gain[i>>1] = gain_gb;
      e_ctrl->eeprom_data.lsc.lsc_calib[light].b_gain[i>>1]  = gain_b;
    }
  }

  SDBG("Exit");
}

/** sunny_gt24c64a_imx376_eeprom_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_gt24c64a_imx376_eeprom_format_afdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char    flag;
  af_data_t        *af;

  SDBG("Enter");
  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[AF_OFFSET];
  if (flag != VALID_FLAG) {
   /* need to call autofocus caliberation to boost up code_per_step
      invalid AF EEPROM data will not be consumed by af parameters
   */
   SERR("AF : empty or invalid data");
   af_present = FALSE;
   return ;
  }
  af_present = TRUE;
  /* Get AF data */
  af = (af_data_t *)(e_ctrl->eeprom_params.buffer + AF_OFFSET + 1);

  e_ctrl->eeprom_data.afc.macro_dac = ((af->macro_h << 8) | af->macro_l);
  e_ctrl->eeprom_data.afc.infinity_dac =
    ((af->infinity_h << 8) | af->infinity_l);
  e_ctrl->eeprom_data.afc.starting_dac = e_ctrl->eeprom_data.afc.infinity_dac;

  SLOW("AF : macro %d infinity %d (no starting DAC set to infinity)",
    e_ctrl->eeprom_data.afc.macro_dac, e_ctrl->eeprom_data.afc.infinity_dac);
  SDBG("Exit");
}

/** sunny_gt24c64a_imx376_eeprom_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void sunny_gt24c64a_imx376_eeprom_format_calibration_data(void *e_ctrl)
{
  sensor_eeprom_data_t * ctrl = (sensor_eeprom_data_t *)e_ctrl;

  SDBG("Enter");
  RETURN_VOID_ON_NULL(ctrl);

  SLOW("Total bytes in OTP buffer: %d", ctrl->eeprom_params.num_bytes);

  if (!ctrl->eeprom_params.buffer || !ctrl->eeprom_params.num_bytes) {
    SERR("failed: Buff pointer %p buffer size %d", ctrl->eeprom_params.buffer,
      ctrl->eeprom_params.num_bytes);
    return;
  }
  sunny_gt24c64a_imx376_eeprom_get_module_info(ctrl);
  sunny_gt24c64a_imx376_eeprom_format_wbdata(ctrl);
  sunny_gt24c64a_imx376_eeprom_format_lensshading(ctrl);
  sunny_gt24c64a_imx376_eeprom_format_afdata(ctrl);
}

/** sunny_gt24c64a_imx376_eeprom_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* sunny_gt24c64a_imx376_eeprom_open_lib(void) {
  return &sunny_gt24c64a_imx376_eeprom_lib_func_ptr;
}
