/*============================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "le2464c_master_eeprom.h"

static uint32_t datapresent = 0;
static uint8_t hall_offset = 0, hall_bias = 0;
static uint16_t lc_size = 0;

void le2464c_master_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);
  e_items->is_wbc  = datapresent ? TRUE : FALSE;
  e_items->is_lsc  = datapresent ? TRUE : FALSE;
  e_items->is_afc  = datapresent ? TRUE : FALSE;
  e_items->is_dpc  = FALSE;
  e_items->is_ois  = FALSE;
  e_items->is_dual = datapresent ? TRUE : FALSE;

  SLOW("is_wbc:%d,is_afc:%d,is_lsc:%d,is_dpc:%d,is_insensor:%d,\
       is_ois:%d",e_items->is_wbc,e_items->is_afc,
       e_items->is_lsc,e_items->is_dpc,e_items->is_insensor,
       e_items->is_ois);
}

/** le2464c_master_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of AF calibration in a page
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void le2464c_master_format_afdata(sensor_eeprom_data_t *e_ctrl) {
  uint8_t *buffer = e_ctrl->eeprom_params.buffer;
  int16_t af_infi = 0;
  int16_t af_macro = 0;
  int32_t dac_range = 0;
  int16_t af_top = 0, af_bottom = 0;

  af_macro = buffer[AF_OFFSET + 1] << 8
    | buffer[AF_OFFSET];
  af_infi = buffer[AF_OFFSET + 3] << 8
    | buffer[AF_OFFSET + 2];


  /*convert signed 16 bit to signed 12 bits*/
  e_ctrl->eeprom_data.afc.infinity_dac = af_infi / 16;
  e_ctrl->eeprom_data.afc.macro_dac = af_macro / 16;

  lc_size = buffer[AF_OFFSET + 5] << 8
    | buffer[AF_OFFSET + 4];

  hall_bias = buffer[AF_OFFSET + 6];
  hall_offset = buffer[AF_OFFSET + 7];

  af_top = buffer[AF_OFFSET + 9] << 8
    | buffer[AF_OFFSET + 8];
  af_bottom = buffer[AF_OFFSET + 11] << 8
    | buffer[AF_OFFSET + 10];

  dac_range = (af_infi - af_macro) / 16;

  e_ctrl->eeprom_data.afc.macro_dac += NEAR_MARGIN * (float)dac_range;
  e_ctrl->eeprom_data.afc.infinity_dac += FAR_MARGIN * (float)dac_range;

  SLOW("OTP:AF raw infi 0x%X, macro 0x%X",af_infi,af_macro);
  SLOW("OTP:near margin %.2f, FAR_MARGIN %.2f",NEAR_MARGIN,FAR_MARGIN);
  SLOW("OTP: VCM TOP: raw:0x%X, 12bit:%d, BOTTOM: raw:0x%X, 12bit:%d",
    af_top, af_top>>4, af_bottom, af_bottom>>4);

  SHIGH("OTP:AF Macro DAC 12bit= %d", (int16_t)e_ctrl->eeprom_data.afc.macro_dac);
  SHIGH("OTP:AF Infinity DAC 12bit= %d", (int16_t)e_ctrl->eeprom_data.afc.infinity_dac);
  SLOW("OTP:AF offset: 0x%x, bias: 0x%x, size: 0x%x, top: 0x%x, bottom: 0x%x",
    hall_offset, hall_bias, lc_size, af_top, af_bottom);

  e_ctrl->eeprom_data.afc.starting_dac = e_ctrl->eeprom_data.afc.infinity_dac;

}

void le2464c_master_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  int index, offset, wb_offset_page;
  float r_over_g, b_over_g, gr_over_gb;
  float r_over_g_golden, b_over_g_golden, gr_over_gb_golden;

  r_over_g_golden = (float)
    (((uint16_t)(e_ctrl->eeprom_params.buffer[WB_GOLDEN_OFFSET + 1] << 8) |
    (e_ctrl->eeprom_params.buffer[WB_GOLDEN_OFFSET]))) / QVALUE;

  b_over_g_golden = (float)
    (((uint16_t)(e_ctrl->eeprom_params.buffer[WB_GOLDEN_OFFSET + 3] << 8) |
    (e_ctrl->eeprom_params.buffer[WB_GOLDEN_OFFSET + 2]))) / QVALUE;

  gr_over_gb_golden = (float)
    (((uint16_t)(e_ctrl->eeprom_params.buffer[WB_GOLDEN_OFFSET + 5] << 8) |
    (e_ctrl->eeprom_params.buffer[WB_GOLDEN_OFFSET + 4]))) / QVALUE;

  r_over_g = (float)
    (((uint16_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET + 1] << 8) |
    (e_ctrl->eeprom_params.buffer[WB_OFFSET]))) / QVALUE;

  b_over_g = (float)
    (((uint16_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET + 3] << 8) |
    (e_ctrl->eeprom_params.buffer[WB_OFFSET + 2]))) / QVALUE;

  gr_over_gb = (float)
    (((uint16_t)(e_ctrl->eeprom_params.buffer[WB_OFFSET + 5] << 8) |
    (e_ctrl->eeprom_params.buffer[WB_OFFSET +4]))) / QVALUE;

  e_ctrl->eeprom_data.wbc.r_over_g[AGW_AWB_D65] = r_over_g;
  e_ctrl->eeprom_data.wbc.b_over_g[AGW_AWB_D65] = b_over_g;
  e_ctrl->eeprom_data.wbc.gr_over_gb = gr_over_gb;

  SLOW("OTP:AWB: r_over_gr: %f,b_over_gr: %f,gr_over_gb: %f",
    r_over_g, b_over_g, gr_over_gb);

  /*
     only have awb for D50 light
     assign same value to other lighting condition
  */
  for (index = 1; index < AGW_AWB_MAX_LIGHT; index++ ) {
     e_ctrl->eeprom_data.wbc.r_over_g[index] =
       e_ctrl->eeprom_data.wbc.r_over_g[AGW_AWB_D65];
     e_ctrl->eeprom_data.wbc.b_over_g[index] =
       e_ctrl->eeprom_data.wbc.b_over_g[AGW_AWB_D65];
  }

}
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
void print_matrix(__attribute__((unused))float* paramlist)
{
  int j =0;
  for (j=0; j < MESH_HWROLLOFF_SIZE; j = j+17 ) {
     SLOW("%.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f,%.2f, %.2f, %.2f,"
          "%.2f, %.2f, %.2f, %.2f, %.2f, %.2f",
     paramlist[j], paramlist[j+1], paramlist[j+2], paramlist[j+3],
     paramlist[j+4], paramlist[j+5], paramlist[j+6], paramlist[j+7],
     paramlist[j+8], paramlist[j+9], paramlist[j+10], paramlist[j+11],
     paramlist[j+12], paramlist[j+13], paramlist[j+14], paramlist[j+15],
     paramlist[j+16]);
  }
}


void le2464c_master_format_lensshading(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  uint16_t i,j;
  uint8_t *buffer = &ectrl->eeprom_params.buffer[LSC_OFFSET];

  for (j = 0; j < MESH_HWROLLOFF_SIZE; j++) {
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].r_gain[j]
      = ( buffer[4] & 0b11000000) << 2 | buffer[0];
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gr_gain[j]
      = ( buffer[4] & 0b00110000) << 4 | buffer[1];
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gb_gain[j]
      = ( buffer[4] & 0b00001100) << 6 | (buffer[2]);
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].b_gain[j]
      = ( buffer[4] & 0b00000011)<< 8 | (buffer[3]);

    buffer += 5;

    /* skip empty bytes according to memory map */
    if((j + 1) % 51 == 0) {
      buffer += 1;
    }

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

    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].r_gain[j]
      = ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].r_gain[j];
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].gr_gain[j]
      = ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gr_gain[j];
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].gb_gain[j]
      =  ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gb_gain[j];
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].b_gain[j]
      = ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].b_gain[j];
  }

  ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].mesh_rolloff_table_size
  = ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].mesh_rolloff_table_size
  = ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].mesh_rolloff_table_size
  = ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].mesh_rolloff_table_size
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

/** le2464c_master_format_dcdata:
 *    @e_ctrl: this is the eeprom data buffer
 *    for dual camera calibration parameters
 *
 * This function formats the dual camera calib parameters.
 *
 * Return: Void
 */
void le2464c_master_format_dcdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  e_ctrl->eeprom_data.dualc.dc_offset = DC_OFFSET;
  e_ctrl->eeprom_data.dualc.dc_size   = DC_SIZE;
  int i = 0, j = 0;
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
}

void le2464c_master_format_calibration_data(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;

  SDBG("Enter");
  RETURN_VOID_ON_NULL(ectrl);

  SHIGH("OTP total bytes: %d",ectrl->eeprom_params.num_bytes);
  if(ectrl->eeprom_params.num_bytes == 0) {
    SERR("failed. OTP/EEPROM empty.");
    datapresent = 0;
    return;
  }
  datapresent = 1;
  le2464c_master_format_afdata(ectrl);
  le2464c_master_format_wbdata(ectrl);
  le2464c_master_format_lensshading(ectrl);
  le2464c_master_format_dcdata(ectrl);

  SDBG("Exit");
}


/** le2464c_master_autofocus_calibration:
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
static int le2464c_master_autofocus_calibration(void *e_ctrl) {
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *) e_ctrl;
  int32_t                 i = 0;
  actuator_tuned_params_t *af_driver_tune = NULL;
  actuator_params_t       *af_params = NULL;
  uint32_t                total_steps = 0;
  int16_t                 macro_dac, infinity_dac;
  uint32_t                qvalue = 0;
  int16_t                 code_per_step_signed = 0;

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

  /* LC898212AXB only: set HALL offset/bias calibration data and reg diff */
  for(i = 0;i < af_params->init_setting_size;i++) {
    if(af_params->init_settings[i].reg_addr == 0x28) {
      af_params->init_settings[i].reg_data = hall_offset<<8 | hall_bias;
      break;
    }
  }

  for(i = 0;i < af_params->init_setting_size;i++) {
    if(af_params->init_settings[i].reg_addr == 0x40) {
      af_params->init_settings[i].reg_data = 0x8010;
      break;
    }
  }
  for(i = 0;i < af_params->init_setting_size;i++) {
    if(af_params->init_settings[i].reg_addr == 0x48) {
      af_params->init_settings[i].reg_data = 0x65B0;
      break;
    }
  }
  for(i = 0;i < af_params->init_setting_size;i++) {
    if(af_params->init_settings[i].reg_addr == 0x4A) {
      af_params->init_settings[i].reg_data = 0x2870;
      break;
    }
  }
  for(i = 0;i < af_params->init_setting_size;i++) {
    if(af_params->init_settings[i].reg_addr == 0x54) {
      af_params->init_settings[i].reg_data = 0x16C0;
      break;
    }
  }
  for(i = 0;i < af_params->init_setting_size;i++) {
    if(af_params->init_settings[i].reg_addr == 0x5A) {
      af_params->init_settings[i].reg_data = 0x0680;
      break;
    }
  }

 /* LC898212: adjust code_per_step, assuming one region only */
  macro_dac = ectrl->eeprom_data.afc.macro_dac;
  infinity_dac = ectrl->eeprom_data.afc.infinity_dac;

  if (infinity_dac > macro_dac)
  {
   af_driver_tune->damping[NUM_ACTUATOR_DIR - 1]->ringing_params->hw_params
    = 0x00000180;
   af_driver_tune->damping[0]->ringing_params->hw_params
    = 0x0000FE80;
  }

  if(infinity_dac == 0 || infinity_dac == 0xFF) {
    SHIGH("failed. EEPROM may be empty macro:%d infinity:%d",
      macro_dac, infinity_dac);
    return TRUE;
  }

  /* adjust af_driver_ptr */
  af_driver_tune->initial_code = infinity_dac;

  qvalue = af_driver_tune->region_params[0].qvalue;

  if(qvalue >= 1 && qvalue <= 4096) {
  code_per_step_signed = (macro_dac - infinity_dac)/ (float)total_steps * qvalue;
  af_driver_tune->region_params[0].code_per_step = code_per_step_signed;
  }
  SLOW("adjusted code_per_step: %d, qvalue: %d",
    (int16_t)(af_driver_tune->region_params[0].code_per_step), qvalue);

  return TRUE;
}

void* le2464c_master_eeprom_open_lib(void) {
  SERR("le2464c_master_eeprom_open_lib Enter");
  return &le2464c_master_lib_func_ptr;
}
