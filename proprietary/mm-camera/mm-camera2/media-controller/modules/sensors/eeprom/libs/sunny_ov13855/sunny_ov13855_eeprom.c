/*============================================================================

Copyright (c) 2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "sunny_ov13855_eeprom.h"

static void print_matrix(__attribute__((unused))float* paramlist)
{
  int j =0;
  for (j=0; j < 221; j = j + 17) {
   SLOW("%.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f,%.6f, %.6f, %.6f,"
     "%.6f, %.6f, %.6f, %.6f, %.6f, %.6f",
     paramlist[j], paramlist[j+1], paramlist[j+2], paramlist[j+3],
     paramlist[j+4], paramlist[j+5], paramlist[j+6], paramlist[j+7],
     paramlist[j+8], paramlist[j+9], paramlist[j+10], paramlist[j+11],
     paramlist[j+12], paramlist[j+13], paramlist[j+14], paramlist[j+15],
     paramlist[j+16]);
  }
}

/** sunny_ov13855_eeprom_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_ov13855_eeprom_get_calibration_items(void *e_ctrl)
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
/** sunny_ov13855_eeprom_get_module_info:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 *   Get Module detailes
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_ov13855_eeprom_get_module_info(sensor_eeprom_data_t *e_ctrl)
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
    (e_ctrl->eeprom_params.buffer + MODULE_INFO_OFFSET + 1);
  SLOW("Module ID : 0x%x", module_info->vendor_id);
  SLOW("Y/M/D : %d/%d/%d",
    module_info->year, module_info->month, module_info->day);

  SDBG("Exit");
}


/** sunny_ov13855_eeprom_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_ov13855_eeprom_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char flag;
  module_info_t *module_info;
  awb_data_t    *wb;
  float         r_over_gr, b_over_gb, gr_over_gb;
  float         golden_r_over_gr, golden_b_over_gb, golden_gr_over_gb;
  int           i;

  SDBG("Enter");
  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[AWB_OFFSET];
  if (flag != VALID_FLAG) {
    awb_present = FALSE;
    SERR("AWB : empty or invalid data");
    return;
  }
  awb_present = TRUE;

  /* Get AWB data */
  wb = (awb_data_t *)(e_ctrl->eeprom_params.buffer + AWB_OFFSET + 1);

  r_over_gr = ((float)((wb->r_over_gr_h << 8) | wb->r_over_gr_l)) / QVALUE;
  b_over_gb = ((float)((wb->b_over_gb_h << 8) | wb->b_over_gb_l)) / QVALUE;
  gr_over_gb = (float)QVALUE / (((wb->gr_over_gb_h << 8) | wb->gr_over_gb_l));
  golden_r_over_gr = ((float)((wb->golden_r_over_gr_h << 8) | wb->golden_r_over_gr_l)) / QVALUE;
  golden_b_over_gb = ((float)((wb->golden_b_over_gb_h << 8) | wb->golden_b_over_gb_l)) / QVALUE;
  golden_gr_over_gb = (float)QVALUE / (((wb->golden_gr_over_gb_h << 8) | wb->golden_gr_over_gb_l));

  SLOW("AWB : r/gr = %f", r_over_gr);
  SLOW("AWB : b/gb = %f", b_over_gb);
  SLOW("AWB : gr/gb = %f", gr_over_gb);
  SLOW("AWB : golden_r_over_gr = %f", golden_r_over_gr);
  SLOW("AWB : golden_b_over_gb = %f", golden_b_over_gb);
  SLOW("AWB : golden_gr_over_gb = %f", golden_gr_over_gb);

  for (i = 0; i < AGW_AWB_MAX_LIGHT; i++) {
    e_ctrl->eeprom_data.wbc.r_over_g[i] = r_over_gr;
    e_ctrl->eeprom_data.wbc.b_over_g[i] = b_over_gb;
  }
  e_ctrl->eeprom_data.wbc.gr_over_gb = gr_over_gb;
  SDBG("Exit");
}

/** sunny_ov13855_eeprom_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_ov13855_eeprom_format_lensshading(sensor_eeprom_data_t *e_ctrl)
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

  /* (1) r gain */
  for (i = 0; i < LSC_GRID_SIZE * 8; i += 8) {
    gain_r  = lsc_buf[i + 0] << 8 | lsc_buf[i + 1];
    gain_gr = lsc_buf[i + 2] << 8 | lsc_buf[i + 3];
    gain_gb = lsc_buf[i + 4] << 8 | lsc_buf[i + 5];
    gain_b  = lsc_buf[i + 6] << 8 | lsc_buf[i + 7];

    for (light = 0; light < ROLLOFF_MAX_LIGHT; light++) {
      e_ctrl->eeprom_data.lsc.lsc_calib[light].r_gain[i/8]  = gain_r;
      e_ctrl->eeprom_data.lsc.lsc_calib[light].gr_gain[i/8] = gain_gr;
      e_ctrl->eeprom_data.lsc.lsc_calib[light].gb_gain[i/8] = gain_gb;
      e_ctrl->eeprom_data.lsc.lsc_calib[light].b_gain[i/8]  = gain_b;
    }
  }
  SLOW("EEPROM: LSC MESH R MATRIX");
  print_matrix(e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].r_gain);
  SLOW("EEPROM: LSC MESH GR MATRIX");
  print_matrix(e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gr_gain);
  SLOW("EEPROM: LSC MESH GB MATRIX");
  print_matrix(e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gb_gain);
  SLOW("EEPROM: LSC MESH B MATRIX");
  print_matrix(e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].b_gain);
  SDBG("Exit");
}

/** sunny_ov13855_eeprom_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_ov13855_eeprom_format_afdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char    flag;
  af_data_t        *af;
  int               dac_range = 0;
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

  e_ctrl->eeprom_data.afc.infinity_margin = INF_MARGIN;
  e_ctrl->eeprom_data.afc.macro_margin = MACRO_MARGIN;
  dac_range = e_ctrl->eeprom_data.afc.macro_dac -
    e_ctrl->eeprom_data.afc.infinity_dac;

  if((e_ctrl->eeprom_data.afc.infinity_dac +
    (e_ctrl->eeprom_data.afc.infinity_margin * (float)dac_range)) < 0) {
    e_ctrl->eeprom_data.afc.infinity_margin = 0 -
      e_ctrl->eeprom_data.afc.infinity_dac / (float)dac_range;
  }

  SLOW("AF : macro %d infinity %d (no starting DAC set to infinity)",
    e_ctrl->eeprom_data.afc.macro_dac, e_ctrl->eeprom_data.afc.infinity_dac);
  SDBG("Exit");
}

static void sunny_ov13855_eeprom_format_pdafgain(sensor_eeprom_data_t *e_ctrl)
{

  pdaf_2D_cal_data_t    *pdafc_2d = &e_ctrl->eeprom_data.pdafc_2d;;
  unsigned char         *buffer = e_ctrl->eeprom_params.buffer;;
  unsigned char         flag;
  unsigned int          i = 0;
  int                   gain_offset = GAIN_OFFSET;
  SERR("Enter");

  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[gain_offset++];
  if (flag == VALID_FLAG) {
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
    gain_offset = PDAF_DCC_OFFSET;
    pdafc_2d->Q_factor_DCC =  buffer[gain_offset] << 8 | buffer[gain_offset+1];
    gain_offset += 2;
    pdafc_2d->MapWidth_DCC=  buffer[gain_offset] << 8 | buffer[gain_offset+1];
    gain_offset += 2;
    pdafc_2d->MapHeight_DCC=  buffer[gain_offset] << 8 | buffer[gain_offset+1];
    gain_offset += 2;

    for (i = 0; i < 48; i++) {
      pdafc_2d->PD_conversion_coeff[i] =
          buffer[gain_offset] << 8 | buffer[gain_offset+1];
      gain_offset += 2;
    }

  } else {
    SERR("PD: no valid gain map in otp data");
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
   pdafc_2d->VersionNum,
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

  SLOW("Exit");
}

/** sunny_ov13855_eeprom_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void sunny_ov13855_eeprom_format_calibration_data(void *e_ctrl)
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
  sunny_ov13855_eeprom_get_module_info(ctrl);
  sunny_ov13855_eeprom_format_wbdata(ctrl);
  sunny_ov13855_eeprom_format_lensshading(ctrl);
  sunny_ov13855_eeprom_format_afdata(ctrl);
  sunny_ov13855_eeprom_format_pdafgain(ctrl);
}

/** sunny_ov13855_eeprom_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* sunny_ov13855_eeprom_open_lib(void) {
  return &sunny_ov13855_eeprom_lib_func_ptr;
}
