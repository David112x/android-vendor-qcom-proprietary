/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include <string.h>
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "eeprom_util.h"
#include "debug_lib.h"
#include "sensor_lib.h"

#define AF_OFFSET_L3 0x0
#define AF_OFFSET_L2 (AF_OFFSET_L3+8)
#define AF_OFFSET_L1 (AF_OFFSET_L2+8)
#define AF_OFFSET_L0 (AF_OFFSET_L1+8)

#define WB_OFFSET_L3 (AF_OFFSET_L0+8)
#define WB_OFFSET_L2 (WB_OFFSET_L3+7)
#define WB_OFFSET_L1 (WB_OFFSET_L2+7)
#define WB_OFFSET_L0 (WB_OFFSET_L1+7)

#define LSC_OFFSET (WB_OFFSET_L0+7)
#define AF_OFFSET AF_OFFSET_L3
#define WB_OFFSET WB_OFFSET_L3

#define PAGE_EMPTY 0
#define PAGE_NOT_EMPTY 1
#define MAX_EMPTY_BYTES 7
#define AWBLSC_VALID_BIT 6

#define QVALUE 1024.0

#define LSC_REG_SIZE 504
/*9x7 = 63; 63*4channels *2 H/L + 3 for update*/
struct camera_i2c_reg_array g_reg_array[LSC_REG_SIZE + 3];
struct camera_i2c_reg_setting g_reg_setting;

static unsigned char bLscAwbValid;

/** sonyimx135_get_calibration_items:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *
 * Loads data structure for enabling / disabling parameters that can be
 * calibrated
 *
 * Return:
 * void
 **/
void sonyimx135_get_calibration_items( void *e_ctrl )
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);
  e_items->is_afc = TRUE;

  if(TRUE == bLscAwbValid){
    e_items->is_wbc = TRUE;
    e_items->is_insensor = TRUE;
    SHIGH("WBC and LSC Available and loaded");
  } else {
    e_items->is_wbc = FALSE;
    e_items->is_insensor = FALSE;
    SHIGH("WBC and LSC UNavailable and not loaded");
  }
  e_items->is_lsc = FALSE;
  e_items->is_dpc = FALSE;

  SLOW("is_wbc: %d,is_afc: %d,is_lsc: %d,is_dpc: %d, is_insensor: %d,\
    is_ois: %d",e_items->is_wbc,e_items->is_afc,
    e_items->is_lsc,e_items->is_dpc,e_items->is_insensor,
    e_items->is_ois);
}

/** sonyimx135_check_empty_page:
 *    @buff: address of page buffer
 *
 * Checks if the page has non zero data
 *
 * Return:
 * unsigned char :  PAGE_EMPTY / PAGE_NOT_EMPTY
 **/
unsigned char sonyimx135_check_empty_page( unsigned char *buff )
{
  unsigned char retval = PAGE_EMPTY;
  unsigned char i=0;

  for(i=0; i < MAX_EMPTY_BYTES; i++){
    if( buff[i] != 0 )
    {
      retval = PAGE_NOT_EMPTY;
      break;
    }
  }
  return retval;
}

/** sonyimx135_check_awblsc_valid:
 *    @buff: address of page buffer
 **
 * Checks if the page has non zero data and also validates if the AWB & LSC
 * data can be used for sensor calibration
 *
 * Return:
 * unsigned char :  PAGE_EMPTY / PAGE_NOT_EMPTY
 **/
unsigned char sonyimx135_check_awblsc_valid( unsigned char *buff )
{
  unsigned char retval = PAGE_EMPTY;
  unsigned char i=0;

  retval = sonyimx135_check_empty_page(buff );
  if(PAGE_EMPTY != retval){
    retval = PAGE_EMPTY;
    if (buff[AWBLSC_VALID_BIT] == 0xFF){
      retval = PAGE_NOT_EMPTY;
    }
  }
  return retval;
}

/** sonyimx135_format_afdata_internal:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *    @AF_START_OFFSET: start offset of page in eeprom memory
 *
 * Format Auto Focus calibration data for AF calibration
 *
 * Return:
 * void
 **/
void  sonyimx135_format_afdata_internal( sensor_eeprom_data_t *e_ctrl,
        unsigned int AF_START_OFFSET )
{
  e_ctrl->eeprom_data.afc.starting_dac =
    (unsigned short) (((e_ctrl->eeprom_params.buffer[AF_START_OFFSET]) << 8) |
    (e_ctrl->eeprom_params.buffer[AF_START_OFFSET + 1]));

  e_ctrl->eeprom_data.afc.infinity_dac =
    (unsigned short)(((e_ctrl->eeprom_params.buffer[AF_START_OFFSET + 2]) << 8) |
    (e_ctrl->eeprom_params.buffer[AF_START_OFFSET + 3]));

  e_ctrl->eeprom_data.afc.macro_dac =
    (unsigned short)(((e_ctrl->eeprom_params.buffer[AF_START_OFFSET + 4]) << 8) |
    (e_ctrl->eeprom_params.buffer[AF_START_OFFSET + 5]));

  SHIGH("AF Starting DAC = %d", e_ctrl->eeprom_data.afc.starting_dac);
  SHIGH("AF Macro DAC = %d", e_ctrl->eeprom_data.afc.macro_dac);
  SHIGH("AF Infinity DAC = %d", e_ctrl->eeprom_data.afc.infinity_dac);
}

/** sonyimx135_format_afdata_internal:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *
 *  Checks for non empty page to calibrate sensor for Auto Focus
 *  This function is called by sonyimx135_format_afdata
 *
 * Return:
 * void
 **/
static void sonyimx135_format_afdata( sensor_eeprom_data_t *e_ctrl )
{
  SDBG("Enter");

  if ( PAGE_NOT_EMPTY ==  sonyimx135_check_empty_page(
    &e_ctrl->eeprom_params.buffer[AF_OFFSET_L3]) ){

      SLOW( "Loading AF_OFFSET_L3" );
      sonyimx135_format_afdata_internal( e_ctrl, AF_OFFSET_L3 );

  } else if ( PAGE_NOT_EMPTY == sonyimx135_check_empty_page(
    &e_ctrl->eeprom_params.buffer[AF_OFFSET_L2]) ){

      SLOW( "Loading AF_OFFSET_L2" );
      sonyimx135_format_afdata_internal(e_ctrl, AF_OFFSET_L2);

  } else if ( PAGE_NOT_EMPTY == sonyimx135_check_empty_page(
    &e_ctrl->eeprom_params.buffer[AF_OFFSET_L1]) ){

      SLOW("Loading AF_OFFSET_L1");
      sonyimx135_format_afdata_internal(e_ctrl, AF_OFFSET_L1);

  } else if ( PAGE_NOT_EMPTY == sonyimx135_check_empty_page(
    &e_ctrl->eeprom_params.buffer[AF_OFFSET_L0])){

      SLOW("Loading AF_OFFSET_L0");
      sonyimx135_format_afdata_internal(e_ctrl, AF_OFFSET_L0);

  }
  SDBG("Exit");
}

/** sonyimx135_format_wbdata_internal:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *    @WB_START_OFFSET: start offset of page in eeprom memory
 *
 * Reads out White Balance calibration data from eeprom and calibrates sensor
 *  This function is called by sonyimx135_format_wbdata
 *
 * Return:
 * void
 **/
void sonyimx135_format_wbdata_internal( sensor_eeprom_data_t *e_ctrl,
        unsigned int WB_START_OFFSET )
{
  unsigned short index;
  float awb_r_over_gr, awb_b_over_gr, awb_gb_over_gr;
  bLscAwbValid = TRUE;

  awb_r_over_gr =(float)((float)
    ((unsigned short)((e_ctrl->eeprom_params.buffer[WB_START_OFFSET] << 8) |
    (e_ctrl->eeprom_params.buffer[WB_START_OFFSET+ 1])) ) / ((float) QVALUE));

  awb_b_over_gr = (float)((float)
    ((unsigned short)((e_ctrl->eeprom_params.buffer[WB_START_OFFSET + 2] << 8) |
    (e_ctrl->eeprom_params.buffer[WB_START_OFFSET+ 3])) ) / ((float) QVALUE));

  awb_gb_over_gr = (float)((float)
    ((unsigned short)((e_ctrl->eeprom_params.buffer[WB_START_OFFSET + 4] << 8) |
    (e_ctrl->eeprom_params.buffer[WB_START_OFFSET+ 5])) ));


  if ( awb_gb_over_gr != 0 ) {

    e_ctrl->eeprom_data.wbc.gr_over_gb = ((float) ((float) (QVALUE) /
      (float)awb_gb_over_gr));

  } else {

    e_ctrl->eeprom_data.wbc.gr_over_gb = 1.0f;

  }

  for ( index = 0; index < AGW_AWB_MAX_LIGHT; index++ ) {

    e_ctrl->eeprom_data.wbc.r_over_g[index] =
      (float) (awb_r_over_gr);

    e_ctrl->eeprom_data.wbc.b_over_g[index] =
      (float) (awb_b_over_gr);

  }

  SLOW("r_over_g \tb_over_g\tgr_over_gb");

  for ( index = 0; index < AGW_AWB_MAX_LIGHT; index++ ) {
    SHIGH("R_G: %f\t\t B_G: %f\t\t GR_GB: %f",
      e_ctrl->eeprom_data.wbc.r_over_g[index],
      e_ctrl->eeprom_data.wbc.b_over_g[index],
      e_ctrl->eeprom_data.wbc.gr_over_gb);
  }
}

/** sonyimx135_format_wbdata:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *
 *  Checks for non empty page to calibrate sensor for Auto Focus
 *
 * Return:
 * void
 **/
static void sonyimx135_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  SDBG("Enter sonyimx135_format_wbdata");
  bLscAwbValid = FALSE; /* Reset value before every read */
  if ( PAGE_NOT_EMPTY ==  sonyimx135_check_awblsc_valid(
    &e_ctrl->eeprom_params.buffer[WB_OFFSET_L3]) ) {

    SLOW("Loading WB_OFFSET_L3");
    sonyimx135_format_wbdata_internal(e_ctrl, WB_OFFSET_L3);

  } else if ( PAGE_NOT_EMPTY == sonyimx135_check_awblsc_valid(
    &e_ctrl->eeprom_params.buffer[WB_OFFSET_L2])){

    SLOW("Loading WB_OFFSET_L2");
    sonyimx135_format_wbdata_internal(e_ctrl, WB_OFFSET_L2);

  } else if ( PAGE_NOT_EMPTY == sonyimx135_check_awblsc_valid(
    &e_ctrl->eeprom_params.buffer[WB_OFFSET_L1])){

    SLOW("Loading WB_OFFSET_L1");
    sonyimx135_format_wbdata_internal(e_ctrl, WB_OFFSET_L1);

  } else if ( PAGE_NOT_EMPTY == sonyimx135_check_awblsc_valid(
    &e_ctrl->eeprom_params.buffer[WB_OFFSET_L0])){

    SLOW("Loading WB_OFFSET_L0");
    sonyimx135_format_wbdata_internal(e_ctrl, WB_OFFSET_L0);

  }

  SDBG("Exit");
}

/** sonyimx135_format_lensshading:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *
 *  Loads lens shading data from the eeprom into the chromatix data
 *
 * Return:
 * void
 **/
static void sonyimx135_format_lensshading (void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  unsigned char *lsc_gain;
  unsigned short j;

  SLOW("Enter");
  lsc_gain = ( unsigned char *) &(ectrl->eeprom_params.buffer[LSC_OFFSET]);
  for (j = 0; j < LSC_REG_SIZE; j++) {
    g_reg_array[g_reg_setting.size].reg_addr = 0x4800 + j;
    g_reg_array[g_reg_setting.size].reg_data = *lsc_gain++;
    g_reg_array[g_reg_setting.size].delay = 0;
    g_reg_setting.size++;
  }

  g_reg_array[g_reg_setting.size].reg_addr = 0x4500;
  g_reg_array[g_reg_setting.size].reg_data = 0x1F;
  g_reg_array[g_reg_setting.size].delay = 0;
  g_reg_setting.size++;

  g_reg_array[g_reg_setting.size].reg_addr = 0x0700;
  g_reg_array[g_reg_setting.size].reg_data = 0x01;
  g_reg_array[g_reg_setting.size].delay = 0;
  g_reg_setting.size++;

  g_reg_array[g_reg_setting.size].reg_addr = 0x3A63;
  g_reg_array[g_reg_setting.size].reg_data = 0x01;
  g_reg_array[g_reg_setting.size].delay = 0;
  g_reg_setting.size++;
  SLOW("Exit");
}


/** sonyimx135_get_raw_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *    @data: point to the destination camera_i2c_reg_setting
 *
 * Get the calibration registers setting of the sensor
 *
 * This function executes in eeprom module context.
 *
 * No return value.
 **/
int sonyimx135_get_raw_data(void *e_ctrl, void *data)
{
  RETURN_ERR_ON_NULL(e_ctrl, SENSOR_FAILURE);
  RETURN_ERR_ON_NULL(data, SENSOR_FAILURE);

  memcpy(data, &g_reg_setting, sizeof(g_reg_setting));

  return SENSOR_SUCCESS;
}

/** sonyimx135_format_calibration_data:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *
 *  This function call all the sub function to read chromatix data and calibrate
 *  the sensor
 *
 * Return:
 * void
 **/
void sonyimx135_format_calibration_data(void *e_ctrl) {
  SDBG("Enter");

  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  unsigned char *data = ectrl->eeprom_params.buffer;

  sonyimx135_format_afdata(ectrl);
  sonyimx135_format_wbdata(ectrl);
  g_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
  g_reg_setting.data_type = CAMERA_I2C_BYTE_DATA;
  g_reg_setting.reg_setting = &g_reg_array[0];
  g_reg_setting.size = 0;
  g_reg_setting.delay = 0;
  sonyimx135_format_lensshading(ectrl);

  SDBG("Exit");
}

/** sonyimx135_lib_func_ptr:
 *  This structure creates the function pointer for imx135 eeprom lib
 **/
static eeprom_lib_func_t sonyimx135_lib_func_ptr = {
    .get_calibration_items = sonyimx135_get_calibration_items,
    .format_calibration_data = sonyimx135_format_calibration_data,
    .do_af_calibration = eeprom_autofocus_calibration,
    .do_wbc_calibration = eeprom_whitebalance_calibration,
    .do_lsc_calibration = NULL,
    .get_raw_data = sonyimx135_get_raw_data,
};

/** sonyimx135_eeprom_open_lib:
 *    @e_ctrl: address of pointer to
 *                   chromatix struct
 *
 *  This function call all the sub function to read chromatix data and calibrate
 *  the sensor
 *
 * Return:
 * void* : Pinter to the sonyimx135 function table
 **/
void* sonyimx135_eeprom_open_lib(void) {
  return &sonyimx135_lib_func_ptr;
}
