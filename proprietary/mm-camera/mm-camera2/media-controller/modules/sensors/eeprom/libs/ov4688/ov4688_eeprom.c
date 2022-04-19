/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include <math.h>
#include "eeprom_lib.h"
#include "../eeprom_util/eeprom.h"
#include "debug_lib.h"
#include "sensor_lib.h"
#include "actuator_driver.h"

#define TABLE_NUM 3
#define BYTES_PER_TABLE 30
#define TOTAL_BYTES TABLE_NUM*BYTES_PER_TABLE

#define TABLE(n) BYTES_PER_TABLE*n

#define AF_START_OFFSET 17
#define WB_START_OFFSET 25

#define TABLE_L2 TABLE(2)
#define TABLE_L1 TABLE(1)
#define TABLE_L0 TABLE(0)


#define QVALUE 1024.0

#define TABLE_EMPTY 0
#define TABLE_NOT_EMPTY 1
#define MAX_EMPTY_BYTES 30

static unsigned char awb_present = 0;
static unsigned char af_present = 0;

/** eeprom_autofocus_calibration_ov4688:
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

int eeprom_autofocus_calibration_ov4688(void *e_ctrl) {
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *) e_ctrl;
  float                   adjust_ratio = 1;
  int                 i = 0, j = 0;
  step_size_table_t       *table;
  actuator_tuned_params_t *af_driver_tune = NULL;
  af_tuning_algo_t        *af_algo_tune = NULL;
  actuator_params_t       *act_params = NULL;
  unsigned int                total_steps = 0;

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

  /* Calculation adjust ratio */
  adjust_ratio = (float)(ectrl->eeprom_data.afc.macro_dac -
    ectrl->eeprom_data.afc.starting_dac) / (float)total_steps;

  for (i = 0; i < af_driver_tune->region_size; i++) {
    af_driver_tune->region_params[i].step_bound[0] =
      (unsigned short)round(
      af_driver_tune->region_params[i].step_bound[0] * adjust_ratio);
    af_driver_tune->region_params[i].step_bound[1] =
      (unsigned short)round(
      af_driver_tune->region_params[i].step_bound[1] * adjust_ratio);
  }

  SLOW("adjust_ratio = %f", adjust_ratio);
  /* Get the calibrated steps */
  total_steps = af_driver_tune->region_params[af_driver_tune->region_size - 1].
      step_bound[0] - af_driver_tune->region_params[0].step_bound[1];

  /* adjust af_driver_ptr */
  af_driver_tune->initial_code = ectrl->eeprom_data.afc.starting_dac;
  act_params->init_settings[3].reg_addr = 0x06;
  act_params->init_settings[3].addr_type = CAMERA_I2C_BYTE_ADDR;
  act_params->init_settings[3].reg_data =
    ectrl->eeprom_data.afc.starting_dac + 20;
  act_params->init_settings[3].data_type = CAMERA_I2C_WORD_DATA;
  act_params->init_settings[3].i2c_operation = ACTUATOR_I2C_OP_WRITE;

  return TRUE;
}

/** ov4688_eeprom_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void ov4688_eeprom_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);

  e_items->is_wbc = awb_present ? TRUE : FALSE;
  e_items->is_afc = af_present ? TRUE : FALSE;
  e_items->is_lsc = FALSE;
  e_items->is_dpc = FALSE;
  e_items->is_insensor = FALSE;
  e_items->is_ois = FALSE;

  SLOW("is_wbc:%d,is_afc:%d,is_lsc:%d,is_dpc:%d,is_insensor:%d,\
  is_ois:%d",e_items->is_wbc,e_items->is_afc,
    e_items->is_lsc,e_items->is_dpc,e_items->is_insensor,
    e_items->is_ois);

}

/** ov4688_check_empty_page:
 *    @buff: address of page buffer
 *
 * Checks if the page has non zero data
 *
 * Return:
 * unsigned char :  TABLE_EMPTY / TABLE_NOT_EMPTY
 **/
static unsigned char ov4688_check_empty_page( unsigned char *buff )
{
  unsigned char retval = TABLE_EMPTY;
  int i=0;

  for(i=0; i < MAX_EMPTY_BYTES; i++){
    if( buff[i] != 0 )
    {
      retval = TABLE_NOT_EMPTY;
      break;
    }
  }
  return retval;
}

/** ov4688_eeprom_format_wbdata_table:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration in a page
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void ov4688_eeprom_format_wbdata_table(sensor_eeprom_data_t *e_ctrl,
  unsigned int wb_offset, unsigned int light_type) {
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  wbcalib_data_t *wbc = &(e_ctrl->eeprom_data.wbc);
  unsigned short i;
  float awb_r_over_gr = 0.0, awb_b_over_gr = 0.0, awb_gr_over_gb = 0.0;

  awb_r_over_gr = (buffer[wb_offset] << 8 | buffer[wb_offset+1]) / (float)QVALUE;
  awb_b_over_gr = (buffer[wb_offset+2] << 8 | buffer[wb_offset+3]) / (float)QVALUE;
  awb_gr_over_gb = (buffer[wb_offset+4] << 8 | buffer[wb_offset+5]) / (float)QVALUE;

  SLOW("OTP:LightType[%d]:awb_r_over_gr: %f,awb_b_over_gr: %f,awb_gr_over_gb: %f",
    light_type, awb_r_over_gr, awb_b_over_gr, awb_gr_over_gb);

  wbc->r_over_g[light_type] = awb_r_over_gr;
  wbc->b_over_g[light_type] = awb_b_over_gr;
  wbc->gr_over_gb = awb_gr_over_gb;

  /* populate other light types */
  for ( i = 0; i < AGW_AWB_MAX_LIGHT; i++ ) {
    wbc->r_over_g[i] = wbc->r_over_g[light_type];
    wbc->b_over_g[i] = wbc->b_over_g[light_type];
  }

}

/** ov4688_eeprom_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void ov4688_eeprom_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  unsigned int wb_start_offset = 0;
  unsigned int light_type = AGW_AWB_D50;


  if(ov4688_check_empty_page(&buffer[TABLE_L0]) != TABLE_EMPTY) {
    SLOW("DEBUGOTP: Read AWB data from layer 0");
    awb_present = 1;
    wb_start_offset = TABLE_L0 + WB_START_OFFSET;
  } else if(ov4688_check_empty_page(&buffer[TABLE_L1]) != TABLE_EMPTY) {
    SLOW("DEBUGOTP: Read AWB data from layer 1");
    awb_present = 1;
    wb_start_offset = TABLE_L1 + WB_START_OFFSET;
  } else if(ov4688_check_empty_page(&buffer[TABLE_L2]) != TABLE_EMPTY) {
    SLOW("DEBUGOTP: Read AWB data from layer 2");
    awb_present = 1;
    wb_start_offset = TABLE_L2 + WB_START_OFFSET;
  } else {
    SHIGH("OTP:AWB data not available for LightType[%d].", light_type);
    awb_present = 0;
    return;
  }

  ov4688_eeprom_format_wbdata_table(e_ctrl, wb_start_offset, light_type);
}

/** ov4688_eeprom_format_afdata_table:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of AF calibration in a page
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void ov4688_eeprom_format_afdata_table(sensor_eeprom_data_t *e_ctrl,
  unsigned int af_offset) {
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;

  e_ctrl->eeprom_data.afc.starting_dac = buffer[af_offset+4] << 8
    | buffer[af_offset+5];

  e_ctrl->eeprom_data.afc.infinity_dac = buffer[af_offset] << 8
    | buffer[af_offset+1];

  e_ctrl->eeprom_data.afc.macro_dac = buffer[af_offset+2] << 8
    | buffer[af_offset+3];

  SHIGH("OTP:AF Starting DAC = %d", e_ctrl->eeprom_data.afc.starting_dac);
  SHIGH("OTP:AF Macro DAC = %d", e_ctrl->eeprom_data.afc.macro_dac);
  SHIGH("OTP:AF Infinity DAC = %d", e_ctrl->eeprom_data.afc.infinity_dac);
}


/** ov4688_eeprom_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void ov4688_eeprom_format_afdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  unsigned int af_start_offset = 0;

  if(ov4688_check_empty_page(&buffer[TABLE_L0]) != TABLE_EMPTY) {
    SLOW("DEBUGOTP: Read AF data from layer 0");
    af_present = 1;
    af_start_offset =  TABLE_L0 + AF_START_OFFSET;
  } else if(ov4688_check_empty_page(&buffer[TABLE_L1]) != TABLE_EMPTY) {
    SLOW("DEBUGOTP: Read AF data from layer 1");
    af_present = 1;
    af_start_offset = TABLE_L1 + AF_START_OFFSET;
  } else if(ov4688_check_empty_page(&buffer[TABLE_L2]) != TABLE_EMPTY) {
    SLOW("DEBUGOTP: Read AF data from layer 2");
    af_present = 1;
    af_start_offset =  TABLE_L2 + AF_START_OFFSET;
  } else {
    SHIGH("OTP:AF data not available");
    af_present = 0;
    return;
  }

  ov4688_eeprom_format_afdata_table(e_ctrl, af_start_offset);

}

/** ov4688_eeprom_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void ov4688_eeprom_format_calibration_data(void *e_ctrl) {
   sensor_eeprom_data_t * ctrl = (sensor_eeprom_data_t *)e_ctrl;

  RETURN_VOID_ON_NULL(ctrl);

  SLOW("Total bytes in OTP buffer: %d", ctrl->eeprom_params.num_bytes);

  if (!ctrl->eeprom_params.buffer || !ctrl->eeprom_params.num_bytes) {
    SERR("Buff pointer %p buffer size %d", ctrl->eeprom_params.buffer,
      ctrl->eeprom_params.num_bytes);
    return;
  }

  ov4688_eeprom_format_wbdata(ctrl);
  ov4688_eeprom_format_afdata(ctrl);

}

static eeprom_lib_func_t ov4688_eeprom_lib_func_ptr = {
  .get_calibration_items = ov4688_eeprom_get_calibration_items,
  .format_calibration_data = ov4688_eeprom_format_calibration_data,
  .do_af_calibration = eeprom_autofocus_calibration_ov4688,
  .do_wbc_calibration = eeprom_whitebalance_calibration,
  .do_lsc_calibration = NULL,
  .get_raw_data = NULL,
  .get_ois_raw_data = NULL,
};

/** ov4688_eeprom_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* ov4688_eeprom_open_lib(void) {
  return &ov4688_eeprom_lib_func_ptr;
}
