/*============================================================================

Copyright (c) 2015 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "imx258_gt24c16_eeprom.h"

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

/** imx258_gt24c16_eeprom_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void imx258_gt24c16_eeprom_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);

  e_items->is_wbc         = awb_present ? TRUE: FALSE;
  e_items->is_afc         = af_present ? TRUE: FALSE;
  e_items->is_lsc         = lsc_present ? TRUE: FALSE;
  e_items->is_dpc         = FALSE;
  e_items->is_insensor    = insensor_present? TRUE: FALSE;
  e_items->is_ois         = FALSE;

  SLOW("is_wbc:%d,is_afc:%d,is_lsc:%d,is_dpc:%d,is_insensor:%d,\
is_ois:%d",e_items->is_wbc,e_items->is_afc,
    e_items->is_lsc,e_items->is_dpc,e_items->is_insensor,
    e_items->is_ois);
}

/** imx258_gt24c16_checksum:
 *    @buff: address of page buffer
 *
 *
 **/
static unsigned char imx258_gt24c16_checksum(unsigned char *buf,
unsigned int first, unsigned int last, unsigned int position)
{
  unsigned char retval = 0;
  unsigned int i = 0, sum = 0;

  SDBG("Enter");
  for(i = first; i <= last; i++){
    sum += buf[i];
  }
  if(sum%255 == buf[position]){
    SLOW("Checksum good, pos:%d, sum:%d buf: %d" , position,
      sum,buf[position]);
    retval = 1;
  }
  SDBG("Exit");
  return retval;
}

/** imx258_gt24c16_eeprom_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void imx258_gt24c16_eeprom_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char  flag, rc;
  awb_data_t     *wb;
  float          r_over_gr, b_over_gb, gr_over_gb;
  unsigned int   i;

  SDBG("Enter");
  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[FLAG_OFFSET];
  if ((flag & 0x01) != VALID_FLAG) {
    SERR("AWB : empty or invalid data");
    return;
  }

  awb_present = TRUE;

  /* Get AWB data */
  wb = (awb_data_t *)(e_ctrl->eeprom_params.buffer + AWB_OFFSET);

  r_over_gr = ((float)(wb->r_value) / wb->gb_value);
  b_over_gb = ((float)(wb->b_value) / wb->gb_value);
  gr_over_gb = ((float)(wb->gr_value) / wb->gb_value);

  SDBG("AWB : wb->r_value = %d", wb->r_value);
  SDBG("AWB : wb->b_value = %d", wb->b_value);
  SDBG("AWB : wb->gr_value = %d", wb->gr_value);
  SDBG("AWB : wb->gb_value = %d", wb->gb_value);

  SDBG("AWB : wb->r_value_golden = %d", wb->r_value_golden);
  SDBG("AWB : wb->b_value_golden = %d", wb->b_value_golden);
  SDBG("AWB : wb->gr_value_golden = %d", wb->gr_value_golden);
  SDBG("AWB : wb->gb_value_golden = %d", wb->gb_value_golden);

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

/** imx258_gt24c16_eeprom_format_lensshading:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of lens shading correction calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void imx258_gt24c16_eeprom_format_lensshading(sensor_eeprom_data_t *e_ctrl)
{
  unsigned short i, light, grid_size;
  unsigned char  *a_r_gain, *a_gr_gain, *a_gb_gain, *a_b_gain, *higher_bits;
  unsigned char  *temp;
  unsigned char  flag;
  float          gain;

  SDBG("Enter");
  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[FLAG_OFFSET];
  if (((flag & 0x02) >> 1) != VALID_FLAG) {
    SERR("LSC : empty or invalid data");
    return;
  }

  if (!(imx258_gt24c16_checksum(e_ctrl->eeprom_params.buffer,
        LSC_OFFSET, LSC_LAST_BYTE, LSC_CRC_OFFSET))){
    SERR("failed: LSC : empty or invalid data");
    return;
  }

  lsc_present = TRUE;
  a_r_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_OFFSET]);
  a_gr_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_OFFSET + 1]);
  a_gb_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_OFFSET + 2]);
  a_b_gain = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_OFFSET + 3]);
  higher_bits = (unsigned char *) &(e_ctrl->eeprom_params.buffer[LSC_OFFSET + 4]);

/* missing information about order in eeprom data sheet */
  for (i = 0; i < MESH_HWROLLOFF_SIZE; i++) {
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].r_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].r_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].r_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].r_gain[i] =
      (float) (*a_r_gain | (((*higher_bits >> 6) & 0x3)) <<8);
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gr_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].gr_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].gr_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].gr_gain[i] =
    (float) (*a_gr_gain | (((*higher_bits >> 4) & 0x3))<<8);
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gb_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].gb_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].gb_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].gb_gain[i] =
    (float) (*a_gb_gain | (((*higher_bits >> 2) & 0x3))<<8);
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].b_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].b_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].b_gain[i] =
      e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].b_gain[i] =
    (float) (*a_b_gain | (((*higher_bits ) & 0x3))<<8);
    a_r_gain += 5;
    a_gr_gain += 5;
    a_gb_gain += 5;
    a_b_gain += 5;
    higher_bits += 5;
  }

  e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].mesh_rolloff_table_size
    = MESH_HWROLLOFF_SIZE;
  e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].mesh_rolloff_table_size
    = MESH_HWROLLOFF_SIZE;
  e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].mesh_rolloff_table_size
    = MESH_HWROLLOFF_SIZE;
    e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].mesh_rolloff_table_size
    = MESH_HWROLLOFF_SIZE;

  SLOW("OTP: LSC MESH R MATRIX");
  print_matrix(e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].r_gain);
  SLOW("OTP: LSC MESH GR MATRIX");
  print_matrix(e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gr_gain);
  SLOW("OTP: LSC MESH GB MATRIX");
  print_matrix(e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gb_gain);
  SLOW("OTP: LSC MESH B MATRIX");
  print_matrix(e_ctrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].b_gain);

  SDBG("Exit");
}

/** imx258_gt24c16_eeprom_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void imx258_gt24c16_eeprom_format_afdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char     flag;
  af_data_t         *af;
  unsigned short    hori_macro_dac;
  unsigned short    hori_infinity_dac;
  unsigned short    vert_macro_dac;
  unsigned short    vert_infinity_dac;

  SDBG("Enter");
  e_ctrl->eeprom_data.afc.macro_dac = INVALID_DATA;
  e_ctrl->eeprom_data.afc.starting_dac = INVALID_DATA;
  e_ctrl->eeprom_data.afc.infinity_dac = INVALID_DATA;

  /* Check validity */
  flag = e_ctrl->eeprom_params.buffer[FLAG_OFFSET];
  if ((((flag & 0x04)>> 2) != VALID_FLAG)) {
    SERR("AF : empty or invalid data");
    return;
  }

  af_present = TRUE;
  /* Get AF data */
  af = (af_data_t *)(e_ctrl->eeprom_params.buffer + AF_OFFSET);
  /* Use horizontal values */
  hori_macro_dac = ((af->macro_h << 8) | af->macro_l);
  hori_infinity_dac = ((af->infinity_h << 8) | af->infinity_l);
  vert_macro_dac = ((af->macro_vert_h << 8) | af->macro_vert_l);
  vert_infinity_dac =  ((af->infinity_vert_h << 8) | af->infinity_vert_l);
  if( hori_macro_dac > vert_macro_dac )
     e_ctrl->eeprom_data.afc.macro_dac = hori_macro_dac;
  else
     e_ctrl->eeprom_data.afc.macro_dac = vert_macro_dac;
  if( hori_infinity_dac < vert_infinity_dac )
    e_ctrl->eeprom_data.afc.infinity_dac = hori_infinity_dac;
  else
    e_ctrl->eeprom_data.afc.infinity_dac =  vert_infinity_dac;

  e_ctrl->eeprom_data.afc.infinity_margin = INF_MARGIN;
  e_ctrl->eeprom_data.afc.macro_margin = MACRO_MARGIN;

  SHIGH("OTP:AF: Macro DAC: %d, Infinity DAC: %d",
    e_ctrl->eeprom_data.afc.macro_dac, e_ctrl->eeprom_data.afc.infinity_dac);
  SLOW("OTP:AF horizontal reference : Macro: %d, Infinity %d",
        hori_macro_dac,
        hori_infinity_dac);
  SLOW("OTP:AF vertical reference : Macro %d Infinity %d",
        vert_macro_dac,
        vert_infinity_dac);
  SDBG("Exit");
}

/** imx258_gt24c16_eeprom_format_pdafgain:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of pdaf gain
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void imx258_gt24c16_eeprom_format_pdafgain(sensor_eeprom_data_t *e_ctrl)
{

  pdaf_2D_cal_data_t              *pdafc_2d = &e_ctrl->eeprom_data.pdafc_2d;
  unsigned char                   *pdaf_ptr, *buffer;
  int                             i;

  SDBG("Enter");

  buffer   = e_ctrl->eeprom_params.buffer;
  pdaf_ptr = e_ctrl->eeprom_params.buffer + PDAF_GAIN_OFFSET;

  if((imx258_gt24c16_checksum(buffer, PDAF_GAIN_OFFSET,
                              PDAF_LAST_BYTE, PDAF_CRC_OFFSET))) {
   SLOW("Verified PDAF");
   pdafc_2d->VersionNum = 2;
   pdafc_2d->OffsetX = (unsigned short)(pdaf_ptr[0] | pdaf_ptr[1] << 8);
   pdaf_ptr += 2;
   pdafc_2d->OffsetY = (unsigned short)(pdaf_ptr[0]|pdaf_ptr[1] << 8);
   pdaf_ptr += 2;
   pdafc_2d->RatioX  = (unsigned short)(pdaf_ptr[0]|pdaf_ptr[1] << 8);
    pdaf_ptr+=2;
   pdafc_2d->RatioY  = (unsigned short)(pdaf_ptr[0]|pdaf_ptr[1] << 8);
    pdaf_ptr+=2;
   pdafc_2d->MapWidth=(unsigned short)(pdaf_ptr[0]|pdaf_ptr[1] << 8);
   pdaf_ptr += 2;
   pdafc_2d->MapHeight =(unsigned short)(pdaf_ptr[0]|pdaf_ptr[1] << 8);
    pdaf_ptr+=2;

   for(i = 0; i < PDGAIN_LENGTH2D; i++){
     pdafc_2d->Left_GainMap[i] =
      (unsigned short)( pdaf_ptr[i * 2] |
                        pdaf_ptr[i * 2 + 1] << 8);
     pdafc_2d->Right_GainMap[i] =
      (unsigned short)( pdaf_ptr[i * 2 + PDGAIN_LENGTH2D * 2] |
                        pdaf_ptr[i * 2 + 1 + PDGAIN_LENGTH2D * 2] << 8);
    }

   pdaf_ptr = e_ctrl->eeprom_params.buffer + PDAF_LAST_BYTE - 1;
   pdafc_2d->PD_conversion_coeff[0] =
    (unsigned short)(pdaf_ptr[0]|pdaf_ptr[1] << 8);
  }
  else
  {
   SLOW("PD: no valid gain map, use default table");
   unsigned short left_gain[PDGAIN_LENGTH2D] = {
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
   unsigned short right_gain[PDGAIN_LENGTH2D] = {
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
}

/** imx258_gt24c16_eeprom_format_spcdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of pdaf calibration:
 *   shield pixel correction (SPC)
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void imx258_gt24c16_eeprom_format_spcdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer    = e_ctrl->eeprom_params.buffer;
  unsigned int  spc_offset = SPC_OFFSET;
  unsigned int  i          = 0;

  SDBG("Enter");
  if(!(imx258_gt24c16_checksum(buffer, SPC_OFFSET,
      EEPROM_LAST_BYTE, SPC_CRC_OFFSET))){
   SERR("failed:invalid SPC data. insensor SPC is not turn on");
   return;
  }

  insensor_present          = TRUE;
  g_reg_setting.reg_setting = g_reg_array;
  g_reg_setting.size        = SPC_SETTING_SIZE;
  g_reg_setting.addr_type   = CAMERA_I2C_WORD_ADDR;
  g_reg_setting.data_type   = CAMERA_I2C_BYTE_DATA;
  g_reg_setting.delay       = 0;

  for(i = 0;i < SPC_SETTING_SIZE; i++) {
    g_reg_array[i].reg_addr = SPC_ADDR + i;
    g_reg_array[i].reg_data = buffer[spc_offset + i];
    g_reg_array[i].delay    = 0;
    SLOW("OTP: SPCData[%d]: addr: 0x%X, data: 0x%X",
      i, g_reg_array[i].reg_addr, g_reg_array[i].reg_data);
  }
  SDBG("Exit");
}

static int imx258_gt24c16_autofocus_calibration(void *e_ctrl) {
  sensor_eeprom_data_t    *ectrl = (sensor_eeprom_data_t *) e_ctrl;
  unsigned short          i = 0;
  actuator_tuned_params_t *af_driver_tune = NULL;
  actuator_params_t       *af_params = NULL;
  unsigned short          total_steps = 0;
  unsigned short          macro_dac, infinity_dac;
  unsigned short          qvalue = 0;

  SDBG("Enter");
  /* Validate params */
  RETURN_ON_NULL(e_ctrl);
  RETURN_ON_NULL(ectrl->eeprom_afchroma.af_driver_ptr);

  af_driver_tune =
    &(ectrl->eeprom_afchroma.af_driver_ptr->actuator_tuned_params);
  af_params = &(ectrl->eeprom_afchroma.af_driver_ptr->actuator_params);

  /* Get the total steps */
  total_steps = af_driver_tune->region_params[af_driver_tune->region_size - 1].
    step_bound[0] - af_driver_tune->region_params[0].step_bound[1];

  if (!total_steps) {
    SERR("Invalid total_steps count = 0");
    return FALSE;
  }

  qvalue = af_driver_tune->region_params[0].qvalue;
  if ((unsigned short)ectrl->eeprom_data.afc.macro_dac < INVALID_DATA)
  {
   macro_dac = ectrl->eeprom_data.afc.macro_dac;
   infinity_dac = ectrl->eeprom_data.afc.infinity_dac;
   if(qvalue >= 1 && qvalue <= 4096)
     af_driver_tune->region_params[0].code_per_step =
      (macro_dac - infinity_dac) / (float)total_steps * qvalue;

  /* adjust af_driver_ptr */
   af_driver_tune->initial_code = infinity_dac;
   /* protect point b */
   if ( af_params->init_settings[2].reg_data > infinity_dac - 20)
   {
    SLOW("original point b %d, new point b %d, starting dac %d",
          af_params->init_settings[2].reg_data,
          infinity_dac - 20,
          infinity_dac);
    af_params->init_settings[2].reg_data = infinity_dac - 20;
   }
  }
  else{
   /* if AF data is invalid, only boost code_per_step */
   if(qvalue >= 1 && qvalue <= 4096)
     af_driver_tune->region_params[0].code_per_step = qvalue;
  }

  SHIGH("With Margin:AF : macro %d infinity %d macro_margin %f inf_margin %f",
    ectrl->eeprom_data.afc.macro_dac, ectrl->eeprom_data.afc.infinity_dac,
    ectrl->eeprom_data.afc.macro_margin,
    ectrl->eeprom_data.afc.infinity_margin);

  SLOW("adjusted code_per_step: %d, qvalue: %d",
    af_driver_tune->region_params[0].code_per_step, qvalue);

  return TRUE;
}

/** imx258_gt24c16_eeprom_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void imx258_gt24c16_eeprom_format_calibration_data(void *e_ctrl) {
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
  SLOW("Module ID : 0x%x", module_info->module_id);
  SLOW("Y/M/D : %d/%d/%d",
    module_info->year, module_info->month, module_info->day);

  /* bypass the eeprom page crc check for allowing manual calibration */
  imx258_gt24c16_eeprom_format_pdafgain(ctrl);

  /* checksum for whole eeprom page */
  rc = imx258_gt24c16_checksum(ctrl->eeprom_params.buffer,
  MODULE_INFO_OFFSET, EEPROM_LAST_BYTE, TOTAL_CHECKSUM);
  if (rc != 1)
  {
   SERR("failed: checksum");
   return;
  }

  imx258_gt24c16_eeprom_format_wbdata(ctrl);
  imx258_gt24c16_eeprom_format_lensshading(ctrl);
  imx258_gt24c16_eeprom_format_afdata(ctrl);
  imx258_gt24c16_eeprom_format_spcdata(ctrl);
}

/** imx258_get_raw_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *    @data: point to the destination camera_i2c_seq_reg_setting
 *
 * Get the all the calibration registers setting for insensor
 * sheild pixel correction
 *
 * This function executes in eeprom module context.
 *
 * No return value.
 **/
static int imx258_get_raw_data(void *e_ctrl, void *data)
{
  RETURN_ERR_ON_NULL(e_ctrl, SENSOR_FAILURE);
  RETURN_ERR_ON_NULL(data, SENSOR_FAILURE);

  memcpy(data, &g_reg_setting, sizeof(g_reg_setting));

  return SENSOR_SUCCESS;
}

/** imx258_gt24c16_eeprom_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* imx258_gt24c16_eeprom_open_lib(void) {
  return &imx258_gt24c16_eeprom_lib_func_ptr;
}
