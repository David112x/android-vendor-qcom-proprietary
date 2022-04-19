/*============================================================================

Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "sunny_gt24c64_s5k2l7sx_eeprom.h"

/** sunny_gt24c64_s5k2l7sx_eeprom_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_gt24c64_s5k2l7sx_eeprom_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);

  e_items->is_wbc = awb_present ? TRUE : FALSE;
  e_items->is_afc = af_present ? TRUE : FALSE;
  e_items->is_lsc = lsc_present ? TRUE : FALSE;
  e_items->is_dpc = FALSE;
  e_items->is_insensor = FALSE;
  e_items->is_ois = ois_present ? TRUE : FALSE;
  e_items->is_dual = dualc_present ? TRUE : FALSE;

  SLOW("is_wbc:%d,is_afc:%d,is_lsc:%d,is_dpc:%d,is_insensor:%d,\
  is_ois:%d, is_dual:%d",e_items->is_wbc,e_items->is_afc,
    e_items->is_lsc,e_items->is_dpc,e_items->is_insensor,
    e_items->is_ois,e_items->is_dual);

}
/** sunny_gt24c64_s5k2l7sx_eeprom_get_module_info:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 *   Get Module detailes
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_gt24c64_s5k2l7sx_eeprom_get_module_info(sensor_eeprom_data_t *e_ctrl)
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

/** sunny_gt24c64_s5k2l7sx_eeprom_get_ois_raw_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *    @data: point to the destination camera_i2c_seq_reg_setting
 *
 * Get the all the calibration registers setting for ois
 *
 * This function executes in eeprom module context.
 *
 * No return value.
**/
int sunny_gt24c64_s5k2l7sx_eeprom_get_ois_raw_data(void *e_ctrl, void *data)
{
  RETURN_ERR_ON_NULL(e_ctrl, SENSOR_FAILURE);
  RETURN_ERR_ON_NULL(data, SENSOR_FAILURE);

  SLOW("Returning OIS calibrated data, size %d", ois_reg_setting.size);
  SLOW("CURDAT - 0x%x%x",
  ois_reg_array[0].reg_data[0], ois_reg_array[0].reg_data[1]);
  SLOW("HALOFS_X - 0x%x%x",
  ois_reg_array[1].reg_data[0], ois_reg_array[1].reg_data[1]);
  SLOW("HALOFS_Y - 0x%x%x",
  ois_reg_array[2].reg_data[0], ois_reg_array[2].reg_data[1]);
  SLOW("HX_OFS - 0x%x%x",
  ois_reg_array[3].reg_data[0], ois_reg_array[3].reg_data[1]);
  SLOW("HY_OFS - 0x%x%x",
  ois_reg_array[4].reg_data[0], ois_reg_array[4].reg_data[1]);
  SLOW("PSTXOF - 0x%x%x",
  ois_reg_array[5].reg_data[0], ois_reg_array[5].reg_data[1]);
  SLOW("PSTYOF - 0x%x%x",
  ois_reg_array[6].reg_data[0], ois_reg_array[6].reg_data[1]);
  SLOW("GX_OFS - 0x%x%x",
  ois_reg_array[7].reg_data[0], ois_reg_array[7].reg_data[1]);
  SLOW("GY_OFS - 0x%x%x",
  ois_reg_array[8].reg_data[0], ois_reg_array[8].reg_data[1]);
  SLOW("KgxHG - 0x%x%x",
  ois_reg_array[9].reg_data[0], ois_reg_array[9].reg_data[1]);
  SLOW("KgyHG - 0x%x%x",
  ois_reg_array[10].reg_data[0], ois_reg_array[10].reg_data[1]);
  SLOW("KGXG - 0x%x%x",
  ois_reg_array[11].reg_data[0], ois_reg_array[11].reg_data[1]);
  SLOW("KGYG - 0x%x%x",
  ois_reg_array[12].reg_data[0], ois_reg_array[12].reg_data[1]);
  SLOW("TMP_X_ - 0x%x%x",
  ois_reg_array[13].reg_data[0], ois_reg_array[13].reg_data[1]);
  SLOW("TMP_Y_ - 0x%x%x",
  ois_reg_array[14].reg_data[0], ois_reg_array[14].reg_data[1]);
  SLOW("KgxH0 - 0x%x%x",
  ois_reg_array[15].reg_data[0], ois_reg_array[15].reg_data[1]);
  SLOW("KgyH0 - 0x%x%x",
  ois_reg_array[16].reg_data[0], ois_reg_array[16].reg_data[1]);
  memcpy(data, &ois_reg_setting, sizeof(ois_reg_setting));

  return SENSOR_SUCCESS;
}

/** sunny_gt24c64_s5k2l7sx_eeprom_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_gt24c64_s5k2l7sx_eeprom_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char flag;
  module_info_t *module_info;
  awb_data_t    *wb;
  float         r_over_gr, b_over_gb, gr_over_gb;
  float         r_over_gr_golden;
  float         b_over_gb_golden;
  float         gr_over_gb_golden;
  int           i;

  SDBG("Enter");
  /* Check validity:
   *  AWB Valid flag is same as Module Valid flag for this EEPROM */
  flag = e_ctrl->eeprom_params.buffer[MODULE_INFO_OFFSET];
  if (flag != VALID_FLAG) {
    awb_present = FALSE;
    SERR("AWB : empty or invalid data");
    return;
  }
  awb_present = TRUE;

  /* Get AWB data */
  wb = (awb_data_t *)(e_ctrl->eeprom_params.buffer + AWB_OFFSET);

  r_over_gr = ((float)((wb->r_over_gr_h << 8) | wb->r_over_gr_l)) / QVALUE;
  b_over_gb = ((float)((wb->b_over_gb_h << 8) | wb->b_over_gb_l)) / QVALUE;
  gr_over_gb = ((float)((wb->gr_over_gb_h << 8) | wb->gr_over_gb_l)) / QVALUE;

  r_over_gr_golden =
    ((float)((wb->r_over_gr_h_golden << 8) | wb->r_over_gr_l_golden)) / QVALUE;
  b_over_gb_golden =
    ((float)((wb->b_over_gb_h_golden << 8) | wb->b_over_gb_l_golden)) / QVALUE;
  gr_over_gb_golden =
    ((float)((wb->gr_over_gb_h_golden << 8) | wb->gr_over_gb_l_golden)) / QVALUE;

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

/** sunny_gt24c64_s5k2l7sx_eeprom_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sunny_gt24c64_s5k2l7sx_eeprom_format_lensshading(sensor_eeprom_data_t *e_ctrl)
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

  lsc_buf = e_ctrl->eeprom_params.buffer;

  for (light = 0; light < ROLLOFF_MAX_LIGHT; light++) {
    e_ctrl->eeprom_data.lsc.lsc_calib[light].mesh_rolloff_table_size =
        LSC_GRID_SIZE;
  }

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

/** sunny_gt24c64_s5k2l7sx_eeprom_format_oisdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure for OIS initialization
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_gt24c64_s5k2l7sx_eeprom_format_oisdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  unsigned char  flag;

  SDBG("Enter");
  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[OIS_OFFSET];
  if (flag != VALID_FLAG) {
   ois_present = FALSE;
   SERR("OIS : empty or invalid data");
   return;
  }
  ois_present = TRUE;

  ois_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
  ois_reg_setting.reg_setting = &ois_reg_array[0];
  ois_reg_setting.size = OIS_SETTING_SIZE;
  ois_reg_setting.delay = 0;

  /* Hall Current DAC */
  ois_reg_array[0].reg_addr = CURDAT;
  ois_reg_array[0].reg_data[0] = buffer[OIS_OFFSET + 3];
  ois_reg_array[0].reg_data[1] = buffer[OIS_OFFSET + 4];
  ois_reg_array[0].reg_data_size = 2;

  /* Hall input differntial amp(pre-amp) Offset DAC X */
  ois_reg_array[1].reg_addr = HALOFS_X;
  ois_reg_array[1].reg_data[0] = buffer[OIS_OFFSET + 5];
  ois_reg_array[1].reg_data[1] = buffer[OIS_OFFSET + 6];
  ois_reg_array[1].reg_data_size = 2;

  /* Hall input differntial amp(pre-amp) Offset DAC Y */
  ois_reg_array[2].reg_addr = HALOFS_Y;
  ois_reg_array[2].reg_data[0] = buffer[OIS_OFFSET + 7];
  ois_reg_array[2].reg_data[1] = buffer[OIS_OFFSET + 8];
  ois_reg_array[2].reg_data_size = 2;

  /* Hall Residual offset X */
  ois_reg_array[3].reg_addr = HX_OFS;
  ois_reg_array[3].reg_data[0] = buffer[OIS_OFFSET + 9];
  ois_reg_array[3].reg_data[1] = buffer[OIS_OFFSET + 10];
  ois_reg_array[3].reg_data_size = 2;

  /* Hall Residual offset Y */
  ois_reg_array[4].reg_addr = HY_OFS;
  ois_reg_array[4].reg_data[0] = buffer[OIS_OFFSET + 11];
  ois_reg_array[4].reg_data[1] = buffer[OIS_OFFSET + 12];
  ois_reg_array[4].reg_data_size = 2;

  /* Hall post-amp offset X */
  ois_reg_array[5].reg_addr = PSTXOF;
  ois_reg_array[5].reg_data[0] = buffer[OIS_OFFSET + 13];
  ois_reg_array[5].reg_data[1] = buffer[OIS_OFFSET + 14];
  ois_reg_array[5].reg_data_size = 2;

  /* Hall post-amp offset Y */
  ois_reg_array[6].reg_addr = PSTYOF;
  ois_reg_array[6].reg_data[0] = buffer[OIS_OFFSET + 15];
  ois_reg_array[6].reg_data[1] = buffer[OIS_OFFSET + 16];
  ois_reg_array[6].reg_data_size = 2;

  /* Gyro Offset X */
  ois_reg_array[7].reg_addr = GX_OFS;
  ois_reg_array[7].reg_data[0] = buffer[OIS_OFFSET + 17];
  ois_reg_array[7].reg_data[1] = buffer[OIS_OFFSET + 18];
  ois_reg_array[7].reg_data_size = 2;

  /* Gyro Offset Y */
  ois_reg_array[8].reg_addr = GY_OFS;
  ois_reg_array[8].reg_data[0] = buffer[OIS_OFFSET + 19];
  ois_reg_array[8].reg_data[1] = buffer[OIS_OFFSET + 20];
  ois_reg_array[8].reg_data_size = 2;

  /* Hall Sensitivity parameter X */
  ois_reg_array[9].reg_addr = KgxHG;
  ois_reg_array[9].reg_data[0] = buffer[OIS_OFFSET + 21];
  ois_reg_array[9].reg_data[1] = buffer[OIS_OFFSET + 22];
  ois_reg_array[9].reg_data_size = 2;

  /* Hall Sensitivity parameter Y */
  ois_reg_array[10].reg_addr = KgyHG;
  ois_reg_array[10].reg_data[0] = buffer[OIS_OFFSET + 23];
  ois_reg_array[10].reg_data[1] = buffer[OIS_OFFSET + 24];
  ois_reg_array[10].reg_data_size = 2;

  /* Actuator gain parameter X */
  ois_reg_array[11].reg_addr = KGXG;
  ois_reg_array[11].reg_data[0] = buffer[OIS_OFFSET + 25];
  ois_reg_array[11].reg_data[1] = buffer[OIS_OFFSET + 26];
  ois_reg_array[11].reg_data_size = 2;

  /* Actuator gain parameter Y */
  ois_reg_array[12].reg_addr = KGYG;
  ois_reg_array[12].reg_data[0] = buffer[OIS_OFFSET + 27];
  ois_reg_array[12].reg_data[1] = buffer[OIS_OFFSET + 28];
  ois_reg_array[12].reg_data_size = 2;

  /* Gyro Temperature Compensation parameter X */
  ois_reg_array[13].reg_addr = TMP_X_;
  ois_reg_array[13].reg_data[0] = buffer[OIS_OFFSET + 33];
  ois_reg_array[13].reg_data[1] = buffer[OIS_OFFSET + 34];
  ois_reg_array[13].reg_data_size = 2;

  /* Gyro Temperature Compensation parameter Y */
  ois_reg_array[14].reg_addr = TMP_Y_;
  ois_reg_array[14].reg_data[0] = buffer[OIS_OFFSET + 35];
  ois_reg_array[14].reg_data[1] = buffer[OIS_OFFSET + 36];
  ois_reg_array[14].reg_data_size = 2;

  /* Actuator Cross-talk compensation X */
  ois_reg_array[15].reg_addr = KgxH0;
  ois_reg_array[15].reg_data[0] = buffer[OIS_OFFSET + 37];
  ois_reg_array[15].reg_data[1] = buffer[OIS_OFFSET + 38];
  ois_reg_array[15].reg_data_size = 2;

  /* Actuator Cross-talk compensation Y */
  ois_reg_array[16].reg_addr = KgyH0;
  ois_reg_array[16].reg_data[0] = buffer[OIS_OFFSET + 39];
  ois_reg_array[16].reg_data[1] = buffer[OIS_OFFSET + 40];
  ois_reg_array[16].reg_data_size = 2;

  SDBG("Exit");
}

/** sunny_gt24c64_s5k2l7sx_eeprom_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sunny_gt24c64_s5k2l7sx_eeprom_format_afdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char    flag;
  af_data_t        *af;
  int              dac_range = 0;
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

static void sunny_gt24c64_s5k2l7sx_eeprom_format_pdafgain(sensor_eeprom_data_t *e_ctrl)
{

  pdaf_2D_cal_data_t    *pdafc_2d = &e_ctrl->eeprom_data.pdafc_2d;;
  unsigned char         *buffer = e_ctrl->eeprom_params.buffer;;
  unsigned char         flag;
  unsigned int          i = 0;
  int                   gain_offset = GAIN_OFFSET;
  int                   pd_data_offset = PD_OFFSET + 1;
  SLOW("Enter");

  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[PD_OFFSET];
  if (flag == VALID_FLAG) {
    pdafc_2d->VersionNum = buffer[pd_data_offset] << 8 | buffer[pd_data_offset+1];
    pd_data_offset += 2;
    pdafc_2d->MapWidth = buffer[pd_data_offset] << 8 | buffer[pd_data_offset+1];
    pd_data_offset += 2;
    pdafc_2d->MapHeight = buffer[pd_data_offset] << 8 | buffer[pd_data_offset+1];

    for (i = 0; i < 221; i++) {
      pdafc_2d->Left_GainMap[i] = buffer[gain_offset] << 8 | buffer[gain_offset+1];
      gain_offset += 2;
    }
    for (i = 0; i < 221; i++) {
      pdafc_2d->Right_GainMap[i] = buffer[gain_offset] << 8 | buffer[gain_offset+1];
      gain_offset += 2;
    }
    pdafc_2d->Q_factor_DCC =  buffer[gain_offset] << 8 | buffer[gain_offset+1];
    gain_offset += 2;
    pdafc_2d->MapWidth_DCC=  buffer[gain_offset] << 8 | buffer[gain_offset+1];
    gain_offset += 2;
    pdafc_2d->MapHeight_DCC=  buffer[gain_offset] << 8 | buffer[gain_offset+1];
    gain_offset += 2;

    for (i = 0; i < DCC_LENGTH; i++) {
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
  for (i = 0; i < DCC_LENGTH; i = i + 8) {
   SLOW("%d, %d, %d, %d, %d, %d, %d, %d,",
     pdafc_2d->PD_conversion_coeff[i],    pdafc_2d->PD_conversion_coeff[i+1],
     pdafc_2d->PD_conversion_coeff[i+2],  pdafc_2d->PD_conversion_coeff[i+3],
     pdafc_2d->PD_conversion_coeff[i+4],  pdafc_2d->PD_conversion_coeff[i+5],
     pdafc_2d->PD_conversion_coeff[i+6],  pdafc_2d->PD_conversion_coeff[i+7]);
  }

  SLOW("Exit");
}

/** sunny_gt24c64_s5k2l7sx_eeprom_format_dcdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format dual calibration data structure
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
static void sunny_gt24c64_s5k2l7sx_eeprom_format_dcdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  int i = 0, j = 0, flag = 0;

  e_ctrl->eeprom_data.dualc.dc_offset = DC_OFFSET;
  e_ctrl->eeprom_data.dualc.dc_size   = DC_SIZE;
  dualc_present = TRUE;

  flag = e_ctrl->eeprom_params.buffer[DC_OFFSET_VALID];
  if(flag != 0x01)
  {
    SERR("Dual camera calibration not present");
    dualc_present = FALSE;
    return;
  }

  /* Pointer to uncalibrated dual camera OTP data */
  e_ctrl->eeprom_data.dualc.system_calib_params.dc_otp_params =
    (void *)&buffer[DC_OFFSET];

  /* Size of dual camera OTP data */
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
    = (unsigned short)(buffer[AE_SYNC]);
  e_ctrl->eeprom_data.dualc.system_calib_params.ref_aux_gain
    = (unsigned short)(buffer[AE_SYNC + 2]);
  e_ctrl->eeprom_data.dualc.system_calib_params.ref_aux_linecount
    = (unsigned short)(buffer[AE_SYNC + 4]);
  e_ctrl->eeprom_data.dualc.system_calib_params.ref_master_gain
    = (unsigned short)(buffer[AE_SYNC + 6]);
  e_ctrl->eeprom_data.dualc.system_calib_params.ref_master_linecount
    = (unsigned short)(buffer[AE_SYNC + 8]);
  /* Reference master color temperature not provided in the map */
  e_ctrl->eeprom_data.dualc.system_calib_params.ref_master_color_temperature
    = 0;
}

/** sunny_gt24c64_s5k2l7sx_eeprom_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void sunny_gt24c64_s5k2l7sx_eeprom_format_calibration_data(void *e_ctrl)
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
  sunny_gt24c64_s5k2l7sx_eeprom_get_module_info(ctrl);
  sunny_gt24c64_s5k2l7sx_eeprom_format_wbdata(ctrl);
  sunny_gt24c64_s5k2l7sx_eeprom_format_lensshading(ctrl);
  sunny_gt24c64_s5k2l7sx_eeprom_format_afdata(ctrl);
  sunny_gt24c64_s5k2l7sx_eeprom_format_pdafgain(ctrl);
  sunny_gt24c64_s5k2l7sx_eeprom_format_dcdata(ctrl);
  sunny_gt24c64_s5k2l7sx_eeprom_format_oisdata(ctrl);
}

/** sunny_gt24c64_s5k2l7sx_eeprom_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* sunny_gt24c64_s5k2l7sx_eeprom_open_lib(void) {
  return &sunny_gt24c64_s5k2l7sx_eeprom_lib_func_ptr;
}
