/*============================================================================
Copyright (c) 2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/
#include "ofilm_ohs0443_ov13880_eeprom.h"

static uint16_t lc_size = 0;

void ofilm_ohs0443_ov13880_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t   *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t   *e_items = &(ectrl->eeprom_data.items);

  e_items->is_wbc = wb_present ? TRUE : FALSE;
  e_items->is_lsc = lsc_present ? TRUE : FALSE;
  e_items->is_afc = af_present ? TRUE : FALSE;
  e_items->is_insensor = FALSE;
  e_items->is_dpc = FALSE;
  e_items->is_ois = FALSE;
  e_items->is_dual = dc_af_sync_present ? TRUE : FALSE;

  SLOW("is_wbc:%d,is_afc:%d,is_lsc:%d,is_dpc:%d,is_insensor:%d,\
  is_ois:%d",e_items->is_wbc,e_items->is_afc,
    e_items->is_lsc,e_items->is_dpc,e_items->is_insensor,
    e_items->is_ois);
}

/** ofilm_ohs0443_ov13880_get_module_info:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 *   Get Module detailes
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void ofilm_ohs0443_ov13880_get_module_info(sensor_eeprom_data_t *e_ctrl)
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
  SLOW("Y/M/D : 20%d/%d/%d",module_info->year, module_info->month,
                           module_info->day);
}

/** ofilm_ohs0443_ov13880_format_wbdata
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of WB calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void ofilm_ohs0443_ov13880_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char flag;
  awb_data_t     *wb;
  float         r_over_gr, b_over_gb, gb_over_gr;
  int           i;

  /* Check validity: AWB and module info flag are same */
  flag = e_ctrl->eeprom_params.buffer[WB_OFFSET];
  if (flag != VALID_FLAG) {
    wb_present = FALSE;
    SERR("empty module info!");
    return;
  }
  wb_present = TRUE;

  /* Get WB data */
  wb = (awb_data_t *)(e_ctrl->eeprom_params.buffer + WB_OFFSET + 1);

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

/* ofilm_ohs0443_ov13880_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
#define GET_R_GAIN_H(value)  (((value) & (0x03 << 6)) >> 6)
#define GET_GR_GAIN_H(value) (((value) & (0x03 << 4)) >> 4)
#define GET_GB_GAIN_H(value) (((value) & (0x03 << 2)) >> 2)
#define GET_B_GAIN_H(value)  (((value) & (0x03 << 0)) >> 0)
void ofilm_ohs0443_ov13880_format_lensshading(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char  flag;
  unsigned short i, light;
  unsigned char  *lsc_buf;
  float          gain_r, gain_gr, gain_gb, gain_b;

  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[LSC_FLAG];
  if (flag != VALID_FLAG) {
   lsc_present = FALSE;
   SERR("LSC : empty or invalid data");
   return;
  }
  lsc_present = TRUE;

  lsc_buf = e_ctrl->eeprom_params.buffer + LSC_OFFSET;

  for (light = 0; light < ROLLOFF_MAX_LIGHT; light++) {
    e_ctrl->eeprom_data.lsc.lsc_calib[light].mesh_rolloff_table_size =
        LSC_GRID_SIZE;
  }

  for (i = 0; i < LSC_GRID_SIZE * 5 ; i += 5) {
    gain_r  = GET_R_GAIN_H (lsc_buf[i + 4]) << 8 | lsc_buf[i + 0];
    gain_gr = GET_GR_GAIN_H(lsc_buf[i + 4]) << 8 | lsc_buf[i + 1];
    gain_gb = GET_GB_GAIN_H(lsc_buf[i + 4]) << 8 | lsc_buf[i + 2];
    gain_b  = GET_B_GAIN_H (lsc_buf[i + 4]) << 8 | lsc_buf[i + 3];

    for (light = 0; light < ROLLOFF_MAX_LIGHT; light++) {
      e_ctrl->eeprom_data.lsc.lsc_calib[light].r_gain [i/5]  = gain_r;
      e_ctrl->eeprom_data.lsc.lsc_calib[light].gr_gain[i/5]  = gain_gr;
      e_ctrl->eeprom_data.lsc.lsc_calib[light].gb_gain[i/5]  = gain_gb;
      e_ctrl->eeprom_data.lsc.lsc_calib[light].b_gain [i/5]  = gain_b;
    }
  }

  SDBG("Exit");
}


/** ofilm_ohs0443_ov13880_format_afdata
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of AF calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void ofilm_ohs0443_ov13880_format_afdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char    flag;
  af_data_t        *af;
  int              dac_range = 0;
  unsigned short   pos_nums  = 0;
  int              i;
  unsigned short   nums_of_af_calib_point = 0;
  unsigned char    *buffer = e_ctrl->eeprom_params.buffer;
  /* af calibration data */
  e_ctrl->eeprom_data.dualc.system_calib_params.af_sync_calib_version
    = (buffer[DC_AF + 0] << 8) | (buffer[DC_AF + 1]);
  e_ctrl->eeprom_data.dualc.system_calib_params.nums_of_af_sync_point
    = nums_of_af_calib_point = (buffer[DC_AF + 2] << 8) | (buffer[DC_AF + 3]);
  if( nums_of_af_calib_point == 0) {
    SERR("No calibration data for dual camera af");
    af_present = 0;
    return ;
  } else {
    af_present = 1;
    dc_af_sync_present = 1;
    for ( i = 0 ; i < nums_of_af_calib_point ; i++) {
      e_ctrl->eeprom_data.dualc.system_calib_params.dist_of_af_sync_point[i]
        = (buffer[DC_AF + 4 + i * 4 + 0] << 8) | (buffer[DC_AF + 4 + i * 4 + 1]);
      e_ctrl->eeprom_data.dualc.system_calib_params.dac_of_af_sync_point[i]
        = (buffer[DC_AF + 4 + i * 4 + 2] << 8) | (buffer[DC_AF + 4 + i * 4 + 3]);
      SLOW("index:%d,distance:%d,dac value:%d", i,
        e_ctrl->eeprom_data.dualc.system_calib_params.dist_of_af_sync_point[i],
        e_ctrl->eeprom_data.dualc.system_calib_params.dac_of_af_sync_point[i]);
    }
    /* after get dc data, also need to fill infinity data and macro data*/
  }

  /* Get AF data */
  pos_nums =
    e_ctrl->eeprom_data.dualc.system_calib_params.nums_of_af_sync_point;
  e_ctrl->eeprom_data.afc.macro_dac =
    e_ctrl->eeprom_data.dualc.system_calib_params.dac_of_af_sync_point[pos_nums - 1];
  e_ctrl->eeprom_data.afc.infinity_dac =
    e_ctrl->eeprom_data.dualc.system_calib_params.dac_of_af_sync_point[0];

  e_ctrl->eeprom_data.afc.starting_dac = e_ctrl->eeprom_data.afc.infinity_dac;

  SHIGH("AF : macro %d infinity %d",
    e_ctrl->eeprom_data.afc.macro_dac, e_ctrl->eeprom_data.afc.infinity_dac);

  e_ctrl->eeprom_data.afc.macro_margin = NEAR_MARGIN;
  e_ctrl->eeprom_data.afc.infinity_margin= FAR_MARGIN;

  dac_range = e_ctrl->eeprom_data.afc.macro_dac -
    e_ctrl->eeprom_data.afc.infinity_dac;

  if((e_ctrl->eeprom_data.afc.infinity_dac +
    (e_ctrl->eeprom_data.afc.infinity_margin * (float)dac_range)) < 0) {
     e_ctrl->eeprom_data.afc.infinity_margin = 0 -
       e_ctrl->eeprom_data.afc.infinity_dac / (float)dac_range;
  }

  SHIGH("near margin %.2f, FAR_MARGIN %.2f" ,
    e_ctrl->eeprom_data.afc.macro_margin,
    e_ctrl->eeprom_data.afc.infinity_margin);
}

/** ofilm_ohs0443_ov13880_format_pdafgain:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of pdaf gain
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void ofilm_ohs0443_ov13880_format_pdafgain(sensor_eeprom_data_t *e_ctrl)
{

  pdaf_2D_cal_data_t    *pdafc_2d = &e_ctrl->eeprom_data.pdafc_2d;;
  unsigned char         *buffer = e_ctrl->eeprom_params.buffer;;
  unsigned char         flag;
  unsigned int          i = 0;
  int                   pd_offset = PDAF_GAIN_OFFSET;

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
    pdafc_2d->Q_factor_DCC =  buffer[pd_offset] << 8 | buffer[pd_offset+1];
    pd_offset += 2;
    pdafc_2d->MapWidth_DCC=  buffer[pd_offset] << 8 | buffer[pd_offset+1];
    pd_offset += 2;
    pdafc_2d->MapHeight_DCC=  buffer[pd_offset] << 8 | buffer[pd_offset+1];
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

/** ofilm_ohs0443_ov13880_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration setting of the sensor.
 *
 * Return: void.
 **/
void ofilm_ohs0443_ov13880_format_calibration_data(void *e_ctrl) {
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;

  RETURN_VOID_ON_NULL(ectrl);

  SLOW("Total bytes in OTP buffer: %d", ectrl->eeprom_params.num_bytes);

  if (!ectrl->eeprom_params.buffer || !ectrl->eeprom_params.num_bytes) {
    SERR("failed: Buff pointer %p buffer size %d", ectrl->eeprom_params.buffer,
      ectrl->eeprom_params.num_bytes);
    return;
  }

  ofilm_ohs0443_ov13880_get_module_info(ectrl);
  ofilm_ohs0443_ov13880_format_wbdata(ectrl);
  ofilm_ohs0443_ov13880_format_lensshading(ectrl);
  ofilm_ohs0443_ov13880_format_afdata(ectrl);
  ofilm_ohs0443_ov13880_format_pdafgain(ectrl);
}

/** ofilm_ohs0443_ov13880_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* ofilm_ohs0443_ov13880_eeprom_open_lib(void) {
  SERR("ofilm_ohs0443_ov13880_eeprom_open_lib Enter");
  return &ofilm_ohs0443_ov13880_lib_func_ptr;
}
