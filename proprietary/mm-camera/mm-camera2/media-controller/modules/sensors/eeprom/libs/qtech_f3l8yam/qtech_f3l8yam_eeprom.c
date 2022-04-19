/*============================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "qtech_f3l8yam_eeprom.h"

/* default PD Gain Map */
static unsigned short left_gain[PDGAIN_LENGTH2D] = {
 499,490,493,500,500,501,500,501,513,520,526,534,533,534,531,540,535,
 508,500,499,500,497,496,497,498,509,518,529,533,532,532,535,536,537,
 522,514,511,510,501,496,496,500,511,520,530,530,535,537,530,528,527,
 536,521,517,507,500,498,499,504,509,515,525,530,534,534,529,523,523,
 539,523,517,507,501,497,500,503,507,514,521,525,533,533,527,518,516,
 551,527,513,505,500,501,498,499,506,511,517,520,528,532,523,508,511,
 548,530,518,507,502,500,498,498,505,511,516,521,526,523,517,506,508,
 552,533,521,509,505,499,499,500,505,509,515,519,524,522,511,503,502,
 547,534,519,510,501,498,499,500,505,508,513,518,521,518,510,503,499,
 545,533,519,514,504,497,496,500,506,508,512,516,519,515,508,500,498,
 533,522,518,511,500,495,494,497,503,507,512,515,515,509,502,499,506,
 525,516,514,512,504,498,495,496,502,504,510,513,512,506,502,502,500,
 522,512,517,512,516,509,507,500,501,506,506,511,506,502,500,502,508
};

static unsigned short right_gain[PDGAIN_LENGTH2D] = {
 560,561,545,538,526,522,521,507,501,488,483,478,478,484,480,476,478,
 549,553,543,540,533,528,522,513,495,487,478,477,477,477,478,478,478,
 539,542,534,533,529,528,518,511,497,485,478,476,475,478,485,485,483,
 534,537,534,534,533,527,521,509,497,489,483,479,478,480,486,493,492,
 532,533,536,536,533,526,519,508,500,490,485,482,478,479,489,497,502,
 525,534,539,534,528,523,517,510,498,490,486,482,480,482,492,505,510,
 521,532,536,531,527,521,515,509,498,491,487,483,481,486,497,511,516,
 523,523,531,529,526,522,513,507,496,488,486,484,482,488,500,509,515,
 520,524,528,529,530,523,513,504,497,491,488,484,485,492,501,511,517,
 524,522,526,525,526,524,516,506,497,492,489,487,486,490,501,508,510,
 528,527,525,523,526,522,517,507,495,491,488,485,489,497,505,509,508,
 532,533,524,522,521,517,514,507,498,491,488,485,491,498,503,500,502,
 529,531,518,516,509,507,506,500,500,496,493,489,493,499,505,501,502
};

/** qtech_f3l8yam_eeprom_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void qtech_f3l8yam_eeprom_get_calibration_items(void *e_ctrl)
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
/** qtech_f3l8yam_eeprom_get_module_info:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 *   Get Module detailes
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void qtech_f3l8yam_eeprom_get_module_info(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char flag;
  module_info_t *module_info;

  SDBG("Enter");

  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[MODULE_INFO_FLAG_OFFSET];
  if (flag != VALID_FLAG) {
    SERR("empty module info!");
    return;
  }

  /* Print module info */
  module_info = (module_info_t *)
    (e_ctrl->eeprom_params.buffer + MODULE_INFO_FLAG_OFFSET + 1);
  SLOW("Module ID : 0x%x", module_info->id);
  SLOW("Y/M/D : %d/%d/%d",
    module_info->year, module_info->month, module_info->day);

  SDBG("Exit");
}


/** qtech_f3l8yam_eeprom_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void qtech_f3l8yam_eeprom_format_wbdata(sensor_eeprom_data_t *e_ctrl)
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
  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[AWB_FLAG_OFFSET];
  if (flag != VALID_FLAG) {
    awb_present = FALSE;
    SERR("AWB : empty or invalid data");
    return;
  }
  awb_present = TRUE;

  /* Get AWB data */
  wb = (awb_data_t *)(e_ctrl->eeprom_params.buffer + AWB_FLAG_OFFSET + 1);

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

/** qtech_f3l8yam_eeprom_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void qtech_f3l8yam_eeprom_format_lensshading(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char  flag;
  unsigned short i, light;
  unsigned char  *lsc_buf;
  float          gain_r, gain_gr, gain_gb, gain_b;

  SDBG("Enter");
  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[LSC_FLAG_OFFSET];
  if (flag != VALID_FLAG) {
   lsc_present = FALSE;
   SERR("LSC : empty or invalid data");
   return;
  }
  lsc_present = TRUE;

  lsc_buf = e_ctrl->eeprom_params.buffer + LSC_FLAG_OFFSET + 1;

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

  SDBG("Exit");
}

/** qtech_f3l8yam_eeprom_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void qtech_f3l8yam_eeprom_format_afdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char    flag;
  af_data_t        *af;

  SDBG("Enter");
  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[AF_FLAG_OFFSET];
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
  af = (af_data_t *)(e_ctrl->eeprom_params.buffer + AF_FLAG_OFFSET + 1);

  e_ctrl->eeprom_data.afc.macro_dac = ((af->macro_h << 8) | af->macro_l);
  e_ctrl->eeprom_data.afc.infinity_dac =
    ((af->infinity_h << 8) | af->infinity_l);
  e_ctrl->eeprom_data.afc.starting_dac = e_ctrl->eeprom_data.afc.infinity_dac;

  SLOW("AF : macro %d infinity %d (no starting DAC set to infinity)",
    e_ctrl->eeprom_data.afc.macro_dac, e_ctrl->eeprom_data.afc.infinity_dac);
  SDBG("Exit");
}

static void qtech_f3l8yam_eeprom_format_pdafgain(sensor_eeprom_data_t *e_ctrl)
{

  pdaf_2D_cal_data_t              *pdafc_2d = &e_ctrl->eeprom_data.pdafc_2d;;
  unsigned char                   *pdaf_ptr;
  unsigned char                   flag;
  unsigned short                  pdaf_version;
  unsigned int                    i;
  SDBG("Enter");
  /* Check validity */
  flag     = e_ctrl->eeprom_params.buffer[GAIN_MAP_FLAG_OFFSET];
  pdaf_ptr = e_ctrl->eeprom_params.buffer + GAIN_MAP_FLAG_OFFSET + 1;
  if ( flag == VALID_FLAG ) {
    pdafc_2d->VersionNum = 2;
    pdaf_version        = (unsigned short)(pdaf_ptr[1] | pdaf_ptr[0] << 8);
    pdaf_ptr += 2;
    pdafc_2d->OffsetX   = (unsigned short)(pdaf_ptr[1] | pdaf_ptr[0] << 8);
    pdaf_ptr += 2;
    pdafc_2d->OffsetY   = (unsigned short)(pdaf_ptr[1] | pdaf_ptr[0] << 8);
    pdaf_ptr += 2;
    pdafc_2d->RatioX    = (unsigned short)(pdaf_ptr[1] | pdaf_ptr[0] << 8);
    pdaf_ptr+=2;
    pdafc_2d->RatioY    = (unsigned short)(pdaf_ptr[1] | pdaf_ptr[0] << 8);
    pdaf_ptr+=2;
    pdafc_2d->MapWidth  = (unsigned short)(pdaf_ptr[1] | pdaf_ptr[0] << 8);
    pdaf_ptr += 2;
    pdafc_2d->MapHeight = (unsigned short)(pdaf_ptr[1] | pdaf_ptr[0] << 8);
    pdaf_ptr+=2;

    for ( i = 0; i < PDGAIN_LENGTH2D; i++)
    {
      pdafc_2d->Left_GainMap[i]  =
            (unsigned short)( pdaf_ptr[i * 2 + 1] |
                          pdaf_ptr[i * 2 + 0] << 8);
      pdafc_2d->Right_GainMap[i] =
            (unsigned short)( pdaf_ptr[i * 2 + PDGAIN_LENGTH2D * 2 + 1] |
                          pdaf_ptr[i * 2 + PDGAIN_LENGTH2D * 2] << 8);
    }
    pdaf_ptr = e_ctrl->eeprom_params.buffer + CC_FLAG_OFFSET + 1;
    pdafc_2d->PD_conversion_coeff[0] =
      (unsigned short)(pdaf_ptr[1]|pdaf_ptr[0] << 8);
  } else /* use default */
  {
  SLOW("PD: no valid gain map, use default table");
   e_ctrl->eeprom_data.pdafc_2d.OffsetX   = 24;
   e_ctrl->eeprom_data.pdafc_2d.OffsetY   = 24;
   e_ctrl->eeprom_data.pdafc_2d.RatioX    = 8;
   e_ctrl->eeprom_data.pdafc_2d.RatioY    = 8;
   e_ctrl->eeprom_data.pdafc_2d.MapWidth  = 17;
   e_ctrl->eeprom_data.pdafc_2d.MapHeight = 13;
   e_ctrl->eeprom_data.pdafc_2d.PD_conversion_coeff[0] = 17965;
   memcpy(e_ctrl->eeprom_data.pdafc_2d.Left_GainMap, &left_gain,
    sizeof(unsigned short) * PDGAIN_LENGTH2D);
   memcpy(e_ctrl->eeprom_data.pdafc_2d.Right_GainMap, &right_gain,
    sizeof(unsigned short) * PDGAIN_LENGTH2D);
  }
  SLOW("-------------PDAF GAIN DUMP------------");
  SLOW("OffsetX %d, OffsetY %d, RatioX %d, RatioY %d,"
       "MapWidth %d, MapHeight %d, PD_conversion_coeff[0] %d",
       pdafc_2d->OffsetX,
       pdafc_2d->OffsetY,
       pdafc_2d->RatioX,
       pdafc_2d->RatioY,
       pdafc_2d->MapWidth,
       pdafc_2d->MapHeight,
       pdafc_2d->PD_conversion_coeff[0]
  );
  SLOW("-----------Left_GainMap----------------");
  for (i = 0; i < PDGAIN_LENGTH2D; i = i + 17) {
   SLOW("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,"
     "%d, %d, %d, %d, %d",
     pdafc_2d->Left_GainMap[i],    pdafc_2d->Left_GainMap[i+1],
     pdafc_2d->Left_GainMap[i+2],  pdafc_2d->Left_GainMap[i+3],
     pdafc_2d->Left_GainMap[i+4],  pdafc_2d->Left_GainMap[i+5],
     pdafc_2d->Left_GainMap[i+6],  pdafc_2d->Left_GainMap[i+7],
     pdafc_2d->Left_GainMap[i+8],  pdafc_2d->Left_GainMap[i+9],
     pdafc_2d->Left_GainMap[i+10], pdafc_2d->Left_GainMap[i+11],
     pdafc_2d->Left_GainMap[i+12], pdafc_2d->Left_GainMap[i+13],
     pdafc_2d->Left_GainMap[i+14], pdafc_2d->Left_GainMap[i+15],
     pdafc_2d->Left_GainMap[i+16]
   );
  }
  SLOW("-----------Right_GainMap----------------");
  for (i = 0; i < PDGAIN_LENGTH2D; i = i + 17) {
   SLOW("%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,"
     "%d, %d, %d, %d, %d",
     pdafc_2d->Right_GainMap[i],    pdafc_2d->Right_GainMap[i+1],
     pdafc_2d->Right_GainMap[i+2],  pdafc_2d->Right_GainMap[i+3],
     pdafc_2d->Right_GainMap[i+4],  pdafc_2d->Right_GainMap[i+5],
     pdafc_2d->Right_GainMap[i+6],  pdafc_2d->Right_GainMap[i+7],
     pdafc_2d->Right_GainMap[i+8],  pdafc_2d->Right_GainMap[i+9],
     pdafc_2d->Right_GainMap[i+10], pdafc_2d->Right_GainMap[i+11],
     pdafc_2d->Right_GainMap[i+12], pdafc_2d->Right_GainMap[i+13],
     pdafc_2d->Right_GainMap[i+14], pdafc_2d->Right_GainMap[i+15],
     pdafc_2d->Right_GainMap[i+16]
   );
  }

  SLOW("Exit");
}

/** qtech_f3l8yam_eeprom_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void qtech_f3l8yam_eeprom_format_calibration_data(void *e_ctrl)
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
  qtech_f3l8yam_eeprom_get_module_info(ctrl);
  qtech_f3l8yam_eeprom_format_wbdata(ctrl);
  qtech_f3l8yam_eeprom_format_lensshading(ctrl);
  qtech_f3l8yam_eeprom_format_afdata(ctrl);
  qtech_f3l8yam_eeprom_format_pdafgain(ctrl);

}

static int qtech_f3l8yam_autofocus_calibration(void *e_ctrl) {
  sensor_eeprom_data_t    *ectrl = (sensor_eeprom_data_t *) e_ctrl;
  int                     i = 0;
  actuator_tuned_params_t *af_driver_tune = NULL;
  actuator_params_t       *af_params = NULL;
  unsigned int            total_steps = 0;
  unsigned short          macro_dac, infinity_dac;
  unsigned short          new_step_bound, otp_step_bound;
  unsigned int            qvalue = 0;

  SDBG("Enter");
  RETURN_ON_NULL(e_ctrl);
  RETURN_ON_NULL(ectrl->eeprom_afchroma.af_driver_ptr);

  af_driver_tune =
    &(ectrl->eeprom_afchroma.af_driver_ptr->actuator_tuned_params);
  af_params = &(ectrl->eeprom_afchroma.af_driver_ptr->actuator_params);

  /* Get the total steps */
  total_steps = af_driver_tune->region_params[af_driver_tune->region_size - 1].
    step_bound[0] - af_driver_tune->region_params[0].step_bound[1];

  if (!total_steps) {
    SERR("Invalid total_steps count: %d",total_steps);
    return FALSE;
  }

  qvalue = af_driver_tune->region_params[0].qvalue;
  if ( qvalue < 1 && qvalue > 4096){
    SERR("Invalid qvalue %d", qvalue);
    return FALSE;
  }
  if ((unsigned short)ectrl->eeprom_data.afc.macro_dac < INVALID_DATA)
  {
   macro_dac = ectrl->eeprom_data.afc.macro_dac;
   infinity_dac = ectrl->eeprom_data.afc.infinity_dac;
   otp_step_bound = macro_dac - infinity_dac;
   /* adjust af_driver_ptr */
   af_driver_tune->initial_code = infinity_dac - otp_step_bound * INFINITY_MARGIN;
   new_step_bound = otp_step_bound * (1 + INFINITY_MARGIN + MACRO_MARGIN);
   af_driver_tune->region_params[0].code_per_step =
    new_step_bound / (float)total_steps * qvalue;
  }
  else{
   /* if AF data is invalid, only boost code_per_step */
     af_driver_tune->region_params[0].code_per_step = qvalue;
  }

  SLOW("initial code %d, adjusted code_per_step: %d, qvalue: %d",
    af_driver_tune->initial_code,
    af_driver_tune->region_params[0].code_per_step,
    qvalue);

  SDBG("Exit");

  return TRUE;
}

/** qtech_f3l8yam_eeprom_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* qtech_f3l8yam_eeprom_open_lib(void) {
  return &qtech_f3l8yam_eeprom_lib_func_ptr;
}
