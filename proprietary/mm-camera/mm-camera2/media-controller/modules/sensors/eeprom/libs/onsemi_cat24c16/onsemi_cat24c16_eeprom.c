/*============================================================================

  Copyright (c) 2014-2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "onsemi_cat24c16_eeprom.h"

static int wbcpresent = 0;
static int afpresent = 0;
static int lscpresent = 0;

/** eeprom_autofocus_calibration_s5k3m2xx:
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

int eeprom_autofocus_calibration_s5k3m2xx(void *e_ctrl) {
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *) e_ctrl;
  int                 i = 0;
  actuator_tuned_params_t *af_driver_tune = NULL;
  actuator_params_t       *act_params = NULL;
  unsigned int                total_steps = 0;
  unsigned int                qvalue = 0;

  /* Validate params */
  RETURN_ON_NULL(e_ctrl);
  RETURN_ON_NULL(ectrl->eeprom_afchroma.af_driver_ptr);

  af_driver_tune =
    &(ectrl->eeprom_afchroma.af_driver_ptr->actuator_tuned_params);
  act_params =
    &(ectrl->eeprom_afchroma.af_driver_ptr->actuator_params);
  /* Get the total steps */
  total_steps = af_driver_tune->region_params[af_driver_tune->region_size - 1].
    step_bound[0] - af_driver_tune->region_params[0].step_bound[1];

  if (!total_steps) {
    SERR("Invalid total_steps count = 0");
    return FALSE;
  }

  /* ad5816g: set VCM_THRESHOLD register with calibration data */
  for(i = 0;i < act_params->init_setting_size;i++) {
    if(act_params->init_settings[i].reg_addr == 0x08) {
      act_params->init_settings[i].reg_data =
        (unsigned short)round(ectrl->eeprom_data.afc.starting_dac / 2);
      break;
    }
  }

  qvalue = af_driver_tune->region_params[0].qvalue;
  if(qvalue >= 1 && qvalue <= 4096)
    af_driver_tune->region_params[0].code_per_step =
     (ectrl->eeprom_data.afc.macro_dac - ectrl->eeprom_data.afc.infinity_dac) /
     (float)total_steps * qvalue;

  /* adjust af_driver_ptr */
  af_driver_tune->initial_code = ectrl->eeprom_data.afc.infinity_dac;

  SHIGH("With Margin:AF : macro %d infinity %d macro_margin %f inf_margin %f",
      ectrl->eeprom_data.afc.macro_dac, ectrl->eeprom_data.afc.infinity_dac,
      ectrl->eeprom_data.afc.macro_margin,
      ectrl->eeprom_data.afc.infinity_margin);

  return TRUE;
}

/** cat24c16_eeprom_check_crc:
 *    @data: input byte stream
 *    @size: number of bytes
 *    @ref_crc: crc to be checked
 *
 * Check the CRC stored in eeprom
 *
 * Return:
 *  1 - crc match
 *  0 - crc not match
 **/
static int cat24c16_eeprom_check_crc(unsigned char *data,
  unsigned short size, unsigned short ref_crc)
{
  unsigned int sum = 0;
  unsigned short crc = 0;
  unsigned short i;

  /* CRC = Sum of Index (0~1787) % 0xFFFF + 1 */
  for(i = 0; i < size; i++) {
    sum += data[i];
  }
  crc = (sum % 0xFFFF) + 1;

  if (crc == ref_crc) {
    SLOW("CRC check OK ref: 0x%x / calc: 0x%x \n", ref_crc, crc);
    return 1;
  } else {
    SERR("failed: CRC check ref: 0x%x / calc: 0x%x \n", ref_crc, crc);
    return 0;
  }
}

/** cat24c16_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void cat24c16_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);

  e_items->is_wbc = wbcpresent ? TRUE : FALSE;
  e_items->is_afc = afpresent ? TRUE : FALSE;
  e_items->is_lsc = lscpresent ? TRUE : FALSE;
  e_items->is_dpc = FALSE;
  e_items->is_insensor = FALSE;
  e_items->is_ois = FALSE;

  SLOW("In OTP:is_wbc:%d,is_afc:%d,is_lsc:%d,is_dpc:%d,is_insensor:%d,\
is_ois:%d",e_items->is_wbc,e_items->is_afc,
    e_items->is_lsc,e_items->is_dpc,e_items->is_insensor,
    e_items->is_ois);

}

/** cat24c16_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static int cat24c16_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
   unsigned char *buffer = e_ctrl->eeprom_params.buffer;
   wbcalib_data_t *wbc = &(e_ctrl->eeprom_data.wbc);

   unsigned int light_type = AGW_AWB_D50;
   unsigned short i;
   float awb_r_over_gr, awb_b_over_gr, awb_gr_over_gb;

   awb_r_over_gr  = (buffer[AWB_OFFSET + 0] << 8 | buffer[AWB_OFFSET + 1])/ QVALUE;
   awb_b_over_gr  = (buffer[AWB_OFFSET + 2] << 8 | buffer[AWB_OFFSET + 3])/ QVALUE;
   awb_gr_over_gb =  QVALUE/(buffer[AWB_OFFSET + 4] << 8 | buffer[AWB_OFFSET + 5]);

   SLOW("CAT24C16: LightType[%d]:\n  awb_r_over_gr: %f\n  \
       awb_b_over_gr: %f\n  awb_gr_over_gb: %f",
     light_type, awb_r_over_gr, awb_b_over_gr, awb_gr_over_gb);

   wbc->r_over_g[light_type] = awb_r_over_gr;
   wbc->b_over_g[light_type] = awb_b_over_gr;
   wbc->gr_over_gb = awb_gr_over_gb;

   /* Populate other light types */
   for ( i = 0; i < AGW_AWB_MAX_LIGHT; i++ ) {
     wbc->r_over_g[i] = awb_r_over_gr;
     wbc->b_over_g[i] = awb_b_over_gr;
   }

   return 1;
}

/** cat24c16_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static int cat24c16_format_afdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char   *buffer = e_ctrl->eeprom_params.buffer;
  unsigned short   af_start_curr, af_infinity, af_marco;

  af_start_curr = buffer[AF_OFFSET + 0] << 8 | buffer[AF_OFFSET + 1];
  af_infinity   = buffer[AF_OFFSET + 2] << 8 | buffer[AF_OFFSET + 3];
  af_marco      = buffer[AF_OFFSET + 4] << 8 | buffer[AF_OFFSET + 5];

  e_ctrl->eeprom_data.afc.starting_dac = af_start_curr;
  e_ctrl->eeprom_data.afc.infinity_dac = af_infinity;
  e_ctrl->eeprom_data.afc.macro_dac    = af_marco;

  e_ctrl->eeprom_data.afc.infinity_margin = INF_MARGIN;
  e_ctrl->eeprom_data.afc.macro_margin = MACRO_MARGIN;

  SLOW("CAT24C16:AF Starting DAC = 0x%04x (%d)",
    e_ctrl->eeprom_data.afc.starting_dac, af_start_curr);
  SLOW("AF : macro %d infinity %d",
      e_ctrl->eeprom_data.afc.macro_dac, e_ctrl->eeprom_data.afc.infinity_dac);
  return 1;
}

/** cat24c16_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static int cat24c16_format_lscdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  unsigned char *a_r_gain, *a_gr_gain, *a_gb_gain, *a_b_gain;
  unsigned short i;

  SDBG("Enter");
  a_r_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_R_OFFSET]);
  a_gr_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_GR_OFFSET]);
  a_b_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_B_OFFSET]);
  a_gb_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_GB_OFFSET]);

  for (i = 0; i < MESH_HWROLLOFF_SIZE; i++) {
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].r_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].r_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].r_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].r_gain[i] =
      (float)((a_r_gain[2 * i + 1] << 8 | a_r_gain[2 * i]) - BL_16)/QVALUE;

    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gr_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].gr_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].gr_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].gr_gain[i] =
      (float)((a_gr_gain[2 * i + 1] << 8|a_gr_gain[2 * i]) - BL_16)/QVALUE;

    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gb_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].gb_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].gb_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].gb_gain[i] =
      (float)((a_gb_gain[2 * i + 1] << 8|a_gb_gain[2 * i]) - BL_16)/QVALUE;

    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].b_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].b_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].b_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].b_gain[i] =
      (float)((a_b_gain[2 * i + 1] << 8|a_b_gain[2 * i]) - BL_16)/QVALUE;
  }

  e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].mesh_rolloff_table_size =
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].mesh_rolloff_table_size =
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].mesh_rolloff_table_size =
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].mesh_rolloff_table_size =
    MESH_HWROLLOFF_SIZE;

  SDBG("Exit");
  return 1;
}

/** cat24c16_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
static void cat24c16_format_calibration_data(void *e_ctrl)
{
  sensor_eeprom_data_t * ctrl = (sensor_eeprom_data_t *)e_ctrl;
  unsigned char *buffer = ctrl->eeprom_params.buffer;
  unsigned short num_bytes = ctrl->eeprom_params.num_bytes;

  unsigned short crc = 0;
  unsigned short i = 0;

  afpresent = 0;
  lscpresent = 0;
  wbcpresent = 0;

#ifdef CAT24C16_DEBUG
  SDBG("Enter");
  unsigned char year, month, day;
  unsigned char module_id, vendor_id;
  unsigned int serial;

  /* Get Year/Month/Day VendorID/ModuleID and Serial Number */
  year = buffer[0];
  month = buffer[1];
  day = buffer[2];
  module_id = buffer[3];
  vendor_id = buffer[4];
  serial = (buffer[5] << 16) | (buffer[6] << 8) | buffer[7];

  /* Print data */
  SLOW("============ CAT24C16 EEPROM ============");
  SLOW("         YYYY/MM/DD - 20%02d/%02d/%02d ",
    year, month, day);
  SLOW("  Module ID:0x%02x\n  Vendor ID:0x%02x\n  Serial Nr:0x%06x",
    module_id, vendor_id, serial);
  SLOW("=========================================");


  /* Print EEPROM data */
  SLOW("================== DUMP =================");
  SLOW("=========================================");
  for(i = 0; i < num_bytes; i = i + 8) {
    SLOW("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
      buffer[i + 0], buffer[i + 1], buffer[i + 2], buffer[i + 3],
      buffer[i + 4], buffer[i + 5], buffer[i + 6], buffer[i + 7]);
  }
  SLOW("=========================================");
#endif

  /* The last two bytes stores CRC */
  crc = buffer[CRC_OFFSET] << 8 | buffer[CRC_OFFSET+1];
  if(cat24c16_eeprom_check_crc(buffer, CRC_OFFSET, crc)) {
    SLOW("CRC match!");
  } else {
    SERR("failed: CRC does NOT match. Calibration disabled.");
    return;
  }
  wbcpresent = cat24c16_format_wbdata(ctrl);
  afpresent  = cat24c16_format_afdata(ctrl);
  lscpresent = cat24c16_format_lscdata(ctrl);
  SDBG("Exit");
}

/** onsemi_cat24c16_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* onsemi_cat24c16_eeprom_open_lib(void) {
  SLOW("Enter");
  return &cat24c16_lib_func_ptr;
}
