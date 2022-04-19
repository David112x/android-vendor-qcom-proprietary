/*============================================================================

Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "ofilm_ohs0443_ov12a10_eeprom.h"

/** ofilm_ohs0443_ov12a10_eeprom_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void ofilm_ohs0443_ov12a10_eeprom_get_calibration_items(void *e_ctrl)
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
/** ofilm_ohs0443_ov12a10_eeprom_get_module_info:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 *   Get Module detailes
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void ofilm_ohs0443_ov12a10_eeprom_get_module_info(sensor_eeprom_data_t *e_ctrl)
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
  SLOW("Y/M/D : 20%d/%d/%d",
    module_info->year, module_info->month, module_info->day);

  SDBG("Exit");
}

/** ofilm_ohs0443_ov12a10_eeprom_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void ofilm_ohs0443_ov12a10_eeprom_format_wbdata(sensor_eeprom_data_t *e_ctrl)
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
  gr_over_gb = ((float)((wb->gb_over_gr_h << 8) | wb->gb_over_gr_l)) / QVALUE;

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

/** ofilm_ohs0443_ov12a10_eeprom_format_lensshading:
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
void ofilm_ohs0443_ov12a10_eeprom_format_lensshading(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char  flag;
  unsigned short i, light;
  unsigned char  *lsc_buf;
  float          gain_r, gain_gr, gain_gb, gain_b;
  SDBG("Enter");
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
      e_ctrl->eeprom_data.lsc.lsc_calib[light].r_gain[i/5]  = gain_r;
      e_ctrl->eeprom_data.lsc.lsc_calib[light].gr_gain[i/5] = gain_gr;
      e_ctrl->eeprom_data.lsc.lsc_calib[light].gb_gain[i/5] = gain_gb;
      e_ctrl->eeprom_data.lsc.lsc_calib[light].b_gain[i/5]  = gain_b;
    }
  }

  SDBG("Exit");
}

/** ofilm_ohs0443_ov12a10_eeprom_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void ofilm_ohs0443_ov12a10_eeprom_format_afdata(
  sensor_eeprom_data_t *e_ctrl)
{
  unsigned char    flag;
  af_data_t        *af;
  int              dac_range = 0;
  unsigned short   pos_nums  = 0;
  SDBG("Enter");

  if (af_present == 0) {
    return ;
  }
  /* Get AF data from dual camera calibration*/
  pos_nums =
    e_ctrl->eeprom_data.dualc.system_calib_params.nums_of_af_sync_point;
  e_ctrl->eeprom_data.afc.macro_dac =
    e_ctrl->eeprom_data.dualc.system_calib_params.dac_of_af_sync_point[pos_nums - 1];
  e_ctrl->eeprom_data.afc.infinity_dac =
    e_ctrl->eeprom_data.dualc.system_calib_params.dac_of_af_sync_point[0];

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

static void ofilm_ohs0443_ov12a10_eeprom_format_pdafgain(sensor_eeprom_data_t *e_ctrl)
{

  pdaf_2D_cal_data_t    *pdafc_2d = &e_ctrl->eeprom_data.pdafc_2d;;
  unsigned char         *buffer = e_ctrl->eeprom_params.buffer;;
  unsigned char         flag;
  unsigned int          i = 0;
  int                   gain_offset    = GAIN_OFFSET;
  int                   pd_data_offset = PD_OFFSET + 2;
  SLOW("Enter");

  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[PD_OFFSET];
  if (flag == VALID_FLAG) {
    pdafc_2d->VersionNum = buffer[pd_data_offset] << 8 | buffer[pd_data_offset+1];
    pd_data_offset += 2;
    pdafc_2d->MapWidth   = buffer[pd_data_offset] << 8 | buffer[pd_data_offset+1];
    pd_data_offset += 2;
    pdafc_2d->MapHeight  = buffer[pd_data_offset] << 8 | buffer[pd_data_offset+1];

    for (i = 0; i < PDGAIN_LENGTH2D ; i++) {
      pdafc_2d->Left_GainMap[i] = buffer[gain_offset] << 8 | buffer[gain_offset+1];
      gain_offset += 2;
    }
    for (i = 0; i < PDGAIN_LENGTH2D ; i++) {
      pdafc_2d->Right_GainMap[i] = buffer[gain_offset] << 8 | buffer[gain_offset+1];
      gain_offset += 2;
    }
    pdafc_2d->Q_factor_DCC =  buffer[gain_offset] << 8 | buffer[gain_offset+1];
    gain_offset += 2;
    pdafc_2d->MapWidth_DCC =  buffer[gain_offset] << 8 | buffer[gain_offset+1];
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

/** ofilm_ohs0443_ov12a10_eeprom_format_dcdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format dual calibration data structure
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
static void ofilm_ohs0443_ov12a10_eeprom_format_dcdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  int i = 0, j = 0, flag = 0;
  unsigned short nums_of_af_calib_point = 0;

  e_ctrl->eeprom_data.dualc.dc_offset = DC_OFFSET;
  e_ctrl->eeprom_data.dualc.dc_size   = DC_SIZE;
  dualc_present = TRUE;

  e_ctrl->eeprom_data.dualc.system_calib_params.nums_of_af_sync_point =
    nums_of_af_calib_point = (buffer[DC_AF + 2] << 8) | (buffer[DC_AF + 3]);

  if( nums_of_af_calib_point == 0) {
    SERR("No calibration data for dual camera af");
    af_present = 0;
  } else {
    af_present = 1;
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

  for(i = 0 ; i < DC_ROTATION_MATRIX ; i++)
    e_ctrl->eeprom_data.dualc.system_calib_params.relative_rotation_matrix[i]=
      *(float *)&buffer[DC_OFFSET + 36 + i * 4];

  for(j = 0;j < DC_GEOMETRIC_MATRIX;j++)
    e_ctrl->eeprom_data.dualc.system_calib_params.relative_geometric_surface_parameters[j]
      = *(float *)&buffer[DC_OFFSET + 72 + j * 4];

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
    = (buffer[AE_SYNC + 0] << 8) | (buffer[AE_SYNC + 1]);
  e_ctrl->eeprom_data.dualc.system_calib_params.ref_master_gain
    = (buffer[AE_SYNC + 2] << 8) | (buffer[AE_SYNC + 3]);
  e_ctrl->eeprom_data.dualc.system_calib_params.ref_master_linecount
    = (buffer[AE_SYNC + 4] << 8) | (buffer[AE_SYNC + 5]);
  e_ctrl->eeprom_data.dualc.system_calib_params.ref_aux_gain
    = (buffer[AE_SYNC + 6] << 8) | (buffer[AE_SYNC + 7]);
  e_ctrl->eeprom_data.dualc.system_calib_params.ref_aux_linecount
    = (buffer[AE_SYNC + 8] << 8) | (buffer[AE_SYNC + 9]);

  /* af calibration data */
  e_ctrl->eeprom_data.dualc.system_calib_params.af_sync_calib_version
    = (buffer[DC_AF + 0] << 8) | (buffer[DC_AF + 1]);
  e_ctrl->eeprom_data.dualc.system_calib_params.nums_of_af_sync_point
    = nums_of_af_calib_point = (buffer[DC_AF + 2] << 8) | (buffer[DC_AF + 3]);

  /* Reference master color temperature not provided in the map */
  e_ctrl->eeprom_data.dualc.system_calib_params.ref_master_color_temperature
    = 0;
}

/** ofilm_ohs0443_ov12a10_eeprom_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void ofilm_ohs0443_ov12a10_eeprom_format_calibration_data(void *e_ctrl)
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
  ofilm_ohs0443_ov12a10_eeprom_get_module_info(ctrl);
  ofilm_ohs0443_ov12a10_eeprom_format_wbdata(ctrl);
  ofilm_ohs0443_ov12a10_eeprom_format_lensshading(ctrl);
  ofilm_ohs0443_ov12a10_eeprom_format_pdafgain(ctrl);
  ofilm_ohs0443_ov12a10_eeprom_format_dcdata(ctrl);
  ofilm_ohs0443_ov12a10_eeprom_format_afdata(ctrl);
}

/** ofilm_ohs0443_ov12a10_eeprom_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* ofilm_ohs0443_ov12a10_eeprom_open_lib(void) {
  return &ofilm_ohs0443_ov12a10_eeprom_lib_func_ptr;
}
