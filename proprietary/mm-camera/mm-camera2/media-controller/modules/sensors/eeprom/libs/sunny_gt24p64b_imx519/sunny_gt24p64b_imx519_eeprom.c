/*============================================================================

Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "sunny_gt24p64b_imx519_eeprom.h"

/** sunny_gt24p64b_imx519_eeprom_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/

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

void sunny_gt24p64b_imx519_eeprom_get_calibration_items(void *e_ctrl)
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
/** sunny_gt24p64b_imx519_eeprom_get_module_info:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 *   Get Module detailes
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_gt24p64b_imx519_eeprom_get_module_info(sensor_eeprom_data_t *e_ctrl)
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
    (e_ctrl->eeprom_params.buffer + MODULE_INFO_OFFSET);
  SLOW("Module ID : 0x%x", module_info->vendor_id);
  SLOW("Y/M/D : %d/%d/%d",
    module_info->year, module_info->month, module_info->day);

  SDBG("Exit");
}


/** sunny_gt24p64b_imx519_eeprom_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_gt24p64b_imx519_eeprom_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char flag;
  module_info_t *module_info;
  awb_data_t    *wb;
  float         r_value, gr_value, gb_value, b_value;
  float         r_value_golden, gr_value_golden, gb_value_golden, b_value_golden;
  float         r_over_gr, b_over_gb, gr_over_gb;
  float         r_over_gr_golden;
  float         b_over_gb_golden;
  float         gr_over_gb_golden;
  int           i;

  SDBG("Enter");
  /* Check validity */
  awb_present = TRUE;

  /* Get AWB data */
  wb = (awb_data_t *)(e_ctrl->eeprom_params.buffer + AWB_OFFSET);

  r_value = (float)((wb->r_avg_h << 8) | wb->r_avg_l);
  gr_value = (float)((wb->gr_avg_h << 8) | wb->gr_avg_l);
  gb_value = (float)((wb->gb_avg_h << 8) | wb->gb_avg_l);
  b_value = (float)((wb->b_avg_h << 8) | wb->b_avg_l);

  r_over_gr = ((float)(r_value) / gb_value);
  b_over_gb = ((float)(b_value) / gb_value);
  gr_over_gb = ((float)(gr_value) / gb_value);

  r_value_golden = (float)((wb->r_avg_h_golden << 8) | wb->r_avg_l_golden);
  gr_value_golden = (float)((wb->gr_avg_h_golden << 8) | wb->gr_avg_l_golden);
  gb_value_golden = (float)((wb->gb_avg_h_golden << 8) | wb->gb_avg_l_golden);
  b_value_golden = (float)((wb->b_avg_h_golden << 8) | wb->b_avg_l_golden);

  r_over_gr_golden =
    ((float)(r_value_golden) / gb_value_golden);
  b_over_gb_golden =
    ((float)(b_value_golden) /gb_value_golden);
  gr_over_gb_golden =
    ((float)(gr_value_golden) / gb_value_golden);

  SLOW("AWB : r/gr = %f", r_over_gr);
  SLOW("AWB : b/gb = %f", b_over_gb);
  SLOW("AWB : gr/gb = %f", gr_over_gb);
  SLOW("AWB : golden r/gr = %f", r_over_gr_golden);
  SLOW("AWB : golden b/gb = %f", b_over_gb_golden);
  SLOW("AWB : golden gr/gb = %f", gr_over_gb_golden);

  for (i = 0; i < AGW_AWB_MAX_LIGHT; i++) {
    e_ctrl->eeprom_data.wbc.r_over_g[i] = r_over_gr;
    e_ctrl->eeprom_data.wbc.b_over_g[i] = b_over_gb;
  }
  e_ctrl->eeprom_data.wbc.gr_over_gb = gr_over_gb;
  SDBG("Exit");
}


/** sunny_gt24p64b_imx519_eeprom_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_gt24p64b_imx519_eeprom_format_lensshading(sensor_eeprom_data_t *e_ctrl)
{


  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  uint16_t i,j;
  /* Check validity */
  uint8_t *buffer = &ectrl->eeprom_params.buffer[LSC_OFFSET];
  lsc_present = TRUE;

  SDBG("Enter");
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


/** sunny_gt24p64b_imx519_eeprom_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_gt24p64b_imx519_eeprom_format_afdata(sensor_eeprom_data_t *e_ctrl)
{

  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  unsigned int af_offset = AF_OFFSET;
  short af_infi_up, af_macro_up;
  int dac_range;
  SDBG("Enter");

  af_present = TRUE;

  af_macro_up = buffer[af_offset + 1] << 8 | buffer[af_offset];
  af_infi_up = buffer[af_offset + 3] << 8 | buffer[af_offset + 2];

  e_ctrl->eeprom_data.afc.infinity_dac = af_infi_up;
  e_ctrl->eeprom_data.afc.macro_dac = af_macro_up;

  SERR("OTP:AF Macro DAC %d", (short)e_ctrl->eeprom_data.afc.macro_dac);
  SERR("OTP:AF Infinity DAC %d", (short)e_ctrl->eeprom_data.afc.infinity_dac);

  dac_range = (short)e_ctrl->eeprom_data.afc.macro_dac
    - (short)e_ctrl->eeprom_data.afc.infinity_dac;
  e_ctrl->eeprom_data.afc.infinity_margin = INF_MARGIN;
  e_ctrl->eeprom_data.afc.macro_margin = MACRO_MARGIN;
  SDBG("Exit");
}

static void sunny_gt24p64b_imx519_eeprom_format_pdafgain(sensor_eeprom_data_t *e_ctrl)
{

  pdaf_2D_cal_data_t          *pdafc_2d = &e_ctrl->eeprom_data.pdafc_2d;
  unsigned char               *buffer = e_ctrl->eeprom_params.buffer;
  unsigned short               i = 0;
  unsigned int                 offset;

  pdafc_2d->VersionNum = buffer[PDAF_VERSION_NUMBER_OFFSET] << 8 | buffer[PDAF_VERSION_NUMBER_OFFSET+1];
  pdafc_2d->MapWidth = buffer[PDAF_MAP_WIDTH_OFFSET] << 8 | buffer[PDAF_MAP_WIDTH_OFFSET+1];
  pdafc_2d->MapHeight = buffer[PDAF_MAP_HEIGHT_OFFSET] << 8 | buffer[PDAF_MAP_HEIGHT_OFFSET+1];

  offset = PDAF_LEFT_GAIN_OFFSET;
  for (i = 0; i < PDAF_GAIN_LENGTH; i++) {
    pdafc_2d->Left_GainMap[i] = buffer[offset] << 8 | buffer[offset+1];
    offset += 2;
  }

  offset = PDAF_RIGHT_GAIN_OFFSET;
  for (i = 0; i < PDAF_GAIN_LENGTH; i++) {
    pdafc_2d->Right_GainMap[i] = buffer[offset] << 8 | buffer[offset+1];
    offset += 2;
  }

  pdafc_2d->Q_factor_DCC =  buffer[DCC_Q_FACTORY_OFFSET] << 8 | buffer[DCC_Q_FACTORY_OFFSET+1];
  pdafc_2d->MapWidth_DCC=  buffer[DCC_MAP_WIDTH_OFFSET] << 8 | buffer[DCC_MAP_WIDTH_OFFSET+1];
  pdafc_2d->MapHeight_DCC=  buffer[DCC_MAP_HEIGHT_OFFSET] << 8 | buffer[DCC_MAP_HEIGHT_OFFSET+1];

  offset = DCC_CONV_COEFF_OFFSET;
  for (i = 0; i < DCC_LENGTH; i++) {
    pdafc_2d->PD_conversion_coeff[i] =
      buffer[offset] << 8 | buffer[offset+1];
    offset += 2;
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

/** sunny_gt24p64b_imx519_eeprom_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void sunny_gt24p64b_imx519_eeprom_format_calibration_data(void *e_ctrl)
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
  sunny_gt24p64b_imx519_eeprom_get_module_info(ctrl);
  sunny_gt24p64b_imx519_eeprom_format_wbdata(ctrl);
  sunny_gt24p64b_imx519_eeprom_format_lensshading(ctrl);
  sunny_gt24p64b_imx519_eeprom_format_afdata(ctrl);
  sunny_gt24p64b_imx519_eeprom_format_pdafgain(ctrl);
}

/** sunny_gt24p64b_imx519_eeprom_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* sunny_gt24p64b_imx519_eeprom_open_lib(void) {
  return &sunny_gt24p64b_imx519_eeprom_lib_func_ptr;
}
