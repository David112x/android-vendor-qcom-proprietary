/*============================================================================
Copyright (c) 2016 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/
#include "m24c64s_eeprom.h"

static uint16_t lc_size = 0;

void m24c64s_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t   *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t   *e_items = &(ectrl->eeprom_data.items);

  e_items->is_wbc = wb_present ? TRUE : FALSE;
  e_items->is_lsc = lsc_present ? TRUE : FALSE;
  e_items->is_afc = af_present ? TRUE : FALSE;
  e_items->is_insensor = FALSE;
  e_items->is_dpc = FALSE;
  e_items->is_ois = FALSE;
  e_items->is_dual = FALSE;

  SLOW("is_wbc:%d,is_afc:%d,is_lsc:%d,is_dpc:%d,is_insensor:%d,\
  is_ois:%d",e_items->is_wbc,e_items->is_afc,
    e_items->is_lsc,e_items->is_dpc,e_items->is_insensor,
    e_items->is_ois);
}

/** m24c64s_get_module_info:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 *   Get Module detailes
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void m24c64s_get_module_info(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char flag;
  module_info_t *module_info;

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
  SLOW("Y/M/D : %d/%d/%d",module_info->year, module_info->month,
                           module_info->day);
}

/** m24c64s_format_wbdata
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of WB calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void m24c64s_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char flag;
  wb_data_t     *wb;
  float         r_over_gr, b_over_gb, gb_over_gr;
  int           i;

  /* Check validity: AWB and module info flag are same */
  flag = e_ctrl->eeprom_params.buffer[MODULE_INFO_OFFSET];
  if (flag != VALID_FLAG) {
    wb_present = FALSE;
    SERR("empty module info!");
    return;
  }
  wb_present = TRUE;

  /* Get WB data */
  wb = (wb_data_t *)(e_ctrl->eeprom_params.buffer + WB_OFFSET);

  r_over_gr = ((float)((wb->r_over_gr_h << 8) | wb->r_over_gr_l)) / QVALUE;
  b_over_gb = ((float)((wb->b_over_gb_h << 8) | wb->b_over_gb_l)) / QVALUE;
  gb_over_gr = ((float)((wb->gb_over_gr_h << 8) | wb->gb_over_gr_l)) / QVALUE;

  SLOW("WB : r/gr = %f", r_over_gr);
  SLOW("WB : b/gb = %f", b_over_gb);
  SLOW("WB : gr/gb = %f", 1.0 / gb_over_gr);

  for (i = 0; i < AGW_AWB_MAX_LIGHT; i++) {
    e_ctrl->eeprom_data.wbc.r_over_g[i] = r_over_gr;
    e_ctrl->eeprom_data.wbc.b_over_g[i] = b_over_gb;
  }
  e_ctrl->eeprom_data.wbc.gr_over_gb = (float)(1.0 / gb_over_gr);
}

/* m24c64s_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void m24c64s_format_lensshading(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char  flag;
  unsigned short i, light;
  unsigned char  *lsc_buf;
  float          gain_r, gain_gr, gain_gb, gain_b;

  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[LSC_OFFSET];
  if (flag != VALID_FLAG) {
   lsc_present = FALSE;
   SERR("LSC : empty or invalid data");
   return;
  }
  lsc_present = TRUE;

  lsc_buf = e_ctrl->eeprom_params.buffer;

  for (light = 0; light < ROLLOFF_MAX_LIGHT; light++) {
    e_ctrl->eeprom_data.lsc.lsc_calib[light].mesh_rolloff_table_size =
        LSC_GRID_SIZE;
  }

  /* read gain */
  for (i = 0; i < LSC_GRID_SIZE * 2; i += 2) {
    gain_r  = lsc_buf[i + LSC_R_OFFSET]  << 8 | lsc_buf[i + 1 + LSC_R_OFFSET];
    gain_gr = lsc_buf[i + LSC_GR_OFFSET] << 8 | lsc_buf[i + 1 + LSC_GR_OFFSET];
    gain_gb = lsc_buf[i + LSC_GB_OFFSET] << 8 | lsc_buf[i + 1 + LSC_GB_OFFSET];
    gain_b  = lsc_buf[i + LSC_B_OFFSET]  << 8 | lsc_buf[i + 1 + LSC_B_OFFSET];

    for (light = 0; light < ROLLOFF_MAX_LIGHT; light++) {
      e_ctrl->eeprom_data.lsc.lsc_calib[light].r_gain[i/2]  = gain_r;
      e_ctrl->eeprom_data.lsc.lsc_calib[light].gr_gain[i/2] = gain_gr;
      e_ctrl->eeprom_data.lsc.lsc_calib[light].gb_gain[i/2] = gain_gb;
      e_ctrl->eeprom_data.lsc.lsc_calib[light].b_gain[i/2]  = gain_b;
    }
  }

  SDBG("Exit");
}


/** m24c64s_format_afdata
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of AF calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void m24c64s_format_afdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char    flag;
  af_data_t        *af;

  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[AF_OFFSET];
  if (flag != VALID_FLAG) {
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

  SHIGH("AF : macro %d infinity %d",
    e_ctrl->eeprom_data.afc.macro_dac, e_ctrl->eeprom_data.afc.infinity_dac);

  e_ctrl->eeprom_data.afc.macro_margin = NEAR_MARGIN;
  e_ctrl->eeprom_data.afc.infinity_margin= FAR_MARGIN;
  SHIGH("near margin %.2f, FAR_MARGIN %.2f",NEAR_MARGIN,FAR_MARGIN);
}

/** m24c64s_format_pdafgain:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of pdaf gain
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void m24c64s_format_pdafgain(sensor_eeprom_data_t *e_ctrl)
{

  pdaf_2D_cal_data_t    *pdafc_2d = &e_ctrl->eeprom_data.pdafc_2d;;
  unsigned char         *buffer = e_ctrl->eeprom_params.buffer;;
  unsigned char         flag;
  unsigned int          i = 0;
  int                   pd_offset = PDAF_OFFSET + 1;

  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[PDAF_OFFSET];
  if (flag == VALID_FLAG) {
    pdafc_2d->VersionNum = buffer[pd_offset] << 8 | buffer[pd_offset+1];
    SHIGH("pdaf version : %d",pdafc_2d->VersionNum);
    pd_offset += 2;
    pdafc_2d->MapWidth = buffer[pd_offset] << 8 | buffer[pd_offset+1];
    pd_offset += 2;
    pdafc_2d->MapHeight = buffer[pd_offset] << 8 | buffer[pd_offset+1];
    pd_offset += 2;
    for (i = 0; i < PDGAIN_LENGTH2D; i++) {
      pdafc_2d->Left_GainMap[i] = buffer[pd_offset] << 8 | buffer[pd_offset+1];
      pd_offset += 2;
    }
    for (i = 0; i < PDGAIN_LENGTH2D; i++) {
      pdafc_2d->Right_GainMap[i] = buffer[pd_offset] << 8 | buffer[pd_offset+1];
      pd_offset += 2;
    }
    pdafc_2d->VersionNum_DCC =  buffer[pd_offset] << 8 | buffer[pd_offset+1];
    pd_offset += 2;
    pdafc_2d->MapWidth_DCC=  buffer[pd_offset] << 8 | buffer[pd_offset+1];
    pd_offset += 2;
    pdafc_2d->MapHeight_DCC=  buffer[pd_offset] << 8 | buffer[pd_offset+1];
    pd_offset += 2;
    pdafc_2d->Q_factor_DCC =  buffer[pd_offset] << 8 | buffer[pd_offset+1];
    pd_offset += 2;
    for (i = 0; i < DCC_LENGTH; i++) {
      pdafc_2d->PD_conversion_coeff[i] = buffer[pd_offset] << 8 | buffer[pd_offset+1];
      pd_offset += 2;
    }
  } else {
    SERR("PD: no valid PDAF data in otp");
    return;
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
  for (i = 0; i < PDGAIN_LENGTH2D; i += PDGAIN_WITDH) {
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
  for (i = 0; i < PDGAIN_LENGTH2D; i += PDGAIN_WITDH) {
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
  for (i = 0; i < DCC_LENGTH; i += DCC_WITDH) {
   SLOW("%d, %d, %d, %d, %d, %d, %d, %d,",
     pdafc_2d->PD_conversion_coeff[i],    pdafc_2d->PD_conversion_coeff[i+1],
     pdafc_2d->PD_conversion_coeff[i+2],  pdafc_2d->PD_conversion_coeff[i+3],
     pdafc_2d->PD_conversion_coeff[i+4],  pdafc_2d->PD_conversion_coeff[i+5],
     pdafc_2d->PD_conversion_coeff[i+6],  pdafc_2d->PD_conversion_coeff[i+7]);
  }
}

/** m24c64s_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration setting of the sensor.
 *
 * Return: void.
 **/
void m24c64s_format_calibration_data(void *e_ctrl) {
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;

  RETURN_VOID_ON_NULL(ectrl);

  SLOW("Total bytes in OTP buffer: %d", ectrl->eeprom_params.num_bytes);

  if (!ectrl->eeprom_params.buffer || !ectrl->eeprom_params.num_bytes) {
    SERR("failed: Buff pointer %p buffer size %d", ectrl->eeprom_params.buffer,
      ectrl->eeprom_params.num_bytes);
    return;
  }

  m24c64s_get_module_info(ectrl);
  m24c64s_format_wbdata(ectrl);
  m24c64s_format_lensshading(ectrl);
  m24c64s_format_afdata(ectrl);
  m24c64s_format_pdafgain(ectrl);
}

/** m24c64s_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* m24c64s_eeprom_open_lib(void) {
  SERR("m24c64s_eeprom_open_lib Enter");
  return &m24c64s_lib_func_ptr;
}
