/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include <string.h>
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "eeprom_util.h"
#include "debug_lib.h"
#include "sensor_lib.h"

#define PAGE_NUM 14
#define BYTES_PER_PAGE 64
#define TOTAL_BYTES PAGE_NUM*BYTES_PER_PAGE

#define PAGE(n) BYTES_PER_PAGE*n

#define WB_PAGE_L2 PAGE(10)
#define WB_PAGE_L1 PAGE(9)
#define WB_PAGE_L0 PAGE(8)

#define AF_PAGE_L2 PAGE(13)
#define AF_PAGE_L1 PAGE(12)
#define AF_PAGE_L0 PAGE(11)

#define WB_START_OFFSET 16
#define LSC_FLAG_OFFSET 32
#define OIS_START_OFFSET 16
#define OIS_SETTING_SIZE 17

#define OTP_GRID_X 9
#define OTP_GRID_Y 7
#define OTP_MESH_HWROLLOFF_SIZE (OTP_GRID_X*OTP_GRID_Y)
#define MESH_HWROLLOFF_SIZE (13*17)

#define LSC_R_OFFSET 0
#define LSC_GR_OFFSET (LSC_R_OFFSET+OTP_MESH_HWROLLOFF_SIZE*2)
#define LSC_B_OFFSET (LSC_GR_OFFSET+OTP_MESH_HWROLLOFF_SIZE*2)
#define LSC_GB_OFFSET (LSC_B_OFFSET+OTP_MESH_HWROLLOFF_SIZE*2)

#define FULL_SIZE_WIDTH 4208
#define FULL_SIZE_HEIGHT 3120

#define QVALUE 1024.0

#define PAGE_EMPTY 0
#define PAGE_NOT_EMPTY 1
#define MAX_EMPTY_BYTES 18

#define FAR_MARGIN (-0.2)
#define NEAR_MARGIN (0.4)

static unsigned char awb_present = 0;
static unsigned char af_present = 0;
static unsigned char lsc_present = 0;
/* Temp data */
static float otp_r[OTP_MESH_HWROLLOFF_SIZE], otp_gr[OTP_MESH_HWROLLOFF_SIZE];
static float otp_gb[OTP_MESH_HWROLLOFF_SIZE], otp_b[OTP_MESH_HWROLLOFF_SIZE];
static float mesh_r[MESH_HWROLLOFF_SIZE], mesh_gr[MESH_HWROLLOFF_SIZE];
static float mesh_gb[MESH_HWROLLOFF_SIZE], mesh_b[MESH_HWROLLOFF_SIZE];

/* ois data */
struct camera_i2c_seq_reg_setting g_reg_setting;
struct camera_i2c_seq_reg_array g_reg_array[OIS_SETTING_SIZE];

/** sony_imx214_eeprom_get_calibration_items:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Get calibration capabilities and mode items.
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
void sony_imx214_eeprom_get_calibration_items(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  eeprom_calib_items_t *e_items = &(ectrl->eeprom_data.items);

  e_items->is_wbc = awb_present ? TRUE : FALSE;
  e_items->is_afc = af_present ? TRUE : FALSE;
  e_items->is_lsc = lsc_present ? TRUE : FALSE;
  e_items->is_dpc = FALSE;
  e_items->is_insensor = FALSE;
  e_items->is_ois = af_present ? TRUE : FALSE;

  SLOW("is_wbc:%d,is_afc:%d,is_lsc:%d,is_dpc:%d,is_insensor:%d,\
       is_ois:%d",e_items->is_wbc,e_items->is_afc,
       e_items->is_lsc,e_items->is_dpc,e_items->is_insensor,
       e_items->is_ois);
}

/** sony_imx214_check_empty_page:
 *    @buff: address of page buffer
 *
 * Checks if the page has non zero data
 *
 * Return:
 * unsigned char :  PAGE_EMPTY / PAGE_NOT_EMPTY
 **/
static unsigned char sony_imx214_check_empty_page( unsigned char *buff )
{
  unsigned char retval = PAGE_EMPTY;
  int i=0;

  for(i=0; i < MAX_EMPTY_BYTES; i++){
    if( buff[i] != 0 )
    {
      retval = PAGE_NOT_EMPTY;
      break;
    }
  }
  return retval;
}

/** sony_imx214_eeprom_format_wbdata_page:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration in a page
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sony_imx214_eeprom_format_wbdata_page(sensor_eeprom_data_t *e_ctrl,
  unsigned int wb_offset, unsigned int light_type) {
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  wbcalib_data_t *wbc = &(e_ctrl->eeprom_data.wbc);
  unsigned short i;
  float awb_r_over_gr = 0.0, awb_b_over_gr = 0.0, awb_gr_over_gb = 0.0;

  awb_r_over_gr = (buffer[wb_offset+1] << 8 | buffer[wb_offset]) / (float)QVALUE;
  awb_b_over_gr = (buffer[wb_offset+3] << 8 | buffer[wb_offset+2]) / (float)QVALUE;
  awb_gr_over_gb = (float)QVALUE / (buffer[wb_offset+5] << 8 | buffer[wb_offset+4]);

  SLOW("OTP:LightType[%d]:awb_r_over_gr: %f,awb_b_over_gr: %f,awb_gr_over_gb: %f",
    light_type, awb_r_over_gr, awb_b_over_gr, awb_gr_over_gb);

  wbc->r_over_g[light_type] = awb_r_over_gr;
  wbc->b_over_g[light_type] = awb_b_over_gr;
  /* Need to set wbc->gr_over_gb according to Bayer pattern and calibration data
    for "BGGR" and "GBRG" of bayer pattern, Gb/Gr of stored calibration data
      wbc->gr_over_gb = awb_gr_over_gb;
    for "RGGB" and "GRBG" bayer pattern, Gb/Gr of stored calibration data
      wbc->gr_over_gb = 1.0f/awb_gr_over_gb; */
  wbc->gr_over_gb = awb_gr_over_gb;

  /* populate other light types */
  for ( i = 0; i < AGW_AWB_MAX_LIGHT; i++ ) {
    wbc->r_over_g[i] = wbc->r_over_g[light_type];
    wbc->b_over_g[i] = wbc->b_over_g[light_type];
  }

}

/** sony_imx214_eeprom_format_wbdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sony_imx214_eeprom_format_wbdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  unsigned int wb_start_offset = 0;
  unsigned int light_type = AGW_AWB_D50;

  if(sony_imx214_check_empty_page(&buffer[WB_PAGE_L2]) != PAGE_EMPTY) {
    SLOW("DEBUGOTP: Read AWB data from layer 2");
    awb_present = 1;
    wb_start_offset = WB_PAGE_L2 + WB_START_OFFSET;
  } else if(sony_imx214_check_empty_page(&buffer[WB_PAGE_L1]) != PAGE_EMPTY) {
    SLOW("DEBUGOTP: Read AWB data from layer 1");
    awb_present = 1;
    wb_start_offset = WB_PAGE_L1 + WB_START_OFFSET;
  } else if(sony_imx214_check_empty_page(&buffer[WB_PAGE_L0]) != PAGE_EMPTY) {
    SLOW("DEBUGOTP: Read AWB data from layer 0");
    awb_present = 1;
    wb_start_offset = WB_PAGE_L0 + WB_START_OFFSET;
  } else {
    SHIGH("OTP:AWB data not available for LightType[%d].", light_type);
    awb_present = 0;
    return;
  }

  sony_imx214_eeprom_format_wbdata_page(e_ctrl, wb_start_offset, light_type);
}

/** sony_imx214_eeprom_format_afdata_page:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of AF calibration in a page
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sony_imx214_eeprom_format_afdata_page(sensor_eeprom_data_t *e_ctrl,
  unsigned int af_offset) {
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  unsigned int dac_range;

  e_ctrl->eeprom_data.afc.starting_dac = buffer[af_offset] << 8
    | buffer[af_offset+1];

  e_ctrl->eeprom_data.afc.infinity_dac = buffer[af_offset+2] << 8
    | buffer[af_offset+3];

  e_ctrl->eeprom_data.afc.macro_dac = buffer[af_offset+4] << 8
    | buffer[af_offset+5];


  SHIGH("OTP:AF Starting DAC = %d", e_ctrl->eeprom_data.afc.starting_dac);
  SHIGH("OTP:AF Macro DAC = %d", e_ctrl->eeprom_data.afc.macro_dac);
  SHIGH("OTP:AF Infinity DAC = %d", e_ctrl->eeprom_data.afc.infinity_dac);

  dac_range = e_ctrl->eeprom_data.afc.macro_dac
    - e_ctrl->eeprom_data.afc.infinity_dac;

  e_ctrl->eeprom_data.afc.macro_dac += NEAR_MARGIN * (float)dac_range;
  e_ctrl->eeprom_data.afc.infinity_dac += FAR_MARGIN * (float)dac_range;

  if(e_ctrl->eeprom_data.afc.infinity_dac <
    e_ctrl->eeprom_data.afc.starting_dac)
    e_ctrl->eeprom_data.afc.infinity_dac = e_ctrl->eeprom_data.afc.starting_dac;

  SHIGH("OTP:after adjustment,Starting DAC %d,Macro DAC %d,Infinity DAC %d",
   e_ctrl->eeprom_data.afc.starting_dac,
   e_ctrl->eeprom_data.afc.macro_dac,
   e_ctrl->eeprom_data.afc.infinity_dac);
}


static void sony_imx214_eeprom_format_oisdata(sensor_eeprom_data_t *e_ctrl,
  unsigned int ois_offset) {
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;

  g_reg_setting.addr_type = CAMERA_I2C_WORD_ADDR;
  g_reg_setting.reg_setting = &g_reg_array[0];
  g_reg_setting.size = OIS_SETTING_SIZE;
  g_reg_setting.delay = 0;

  /* Turn on 16 bit fix mode */
  g_reg_array[0].reg_addr = 0x018C;
  g_reg_array[0].reg_data[0] = 0x31;
  g_reg_array[0].reg_data_size = 1;

  /* Hall offset X */
  g_reg_array[1].reg_addr = 0x1479;
  g_reg_array[1].reg_data[0] = buffer[ois_offset];
  g_reg_array[1].reg_data[1] = buffer[ois_offset + 1];
  g_reg_array[1].reg_data_size = 2;

  /* Hall offset Y */
  g_reg_array[2].reg_addr = 0x14F9;
  g_reg_array[2].reg_data[0] = buffer[ois_offset + 2];
  g_reg_array[2].reg_data[1] = buffer[ois_offset + 3];
  g_reg_array[2].reg_data_size = 2;

  /* Hall bias X */
  g_reg_array[3].reg_addr = 0x147A;
  g_reg_array[3].reg_data[0] = buffer[ois_offset + 4];
  g_reg_array[3].reg_data[1] = buffer[ois_offset + 5];
  g_reg_array[3].reg_data_size = 2;

  /* Hall bias Y */
  g_reg_array[4].reg_addr = 0x14FA;
  g_reg_array[4].reg_data[0] = buffer[ois_offset + 6];
  g_reg_array[4].reg_data[1] = buffer[ois_offset + 7];
  g_reg_array[4].reg_data_size = 2;

  /* Hall A/D offset X */
  g_reg_array[5].reg_addr = 0x1450;
  g_reg_array[5].reg_data[0] = buffer[ois_offset + 8];
  g_reg_array[5].reg_data[1] = buffer[ois_offset + 9];
  g_reg_array[5].reg_data_size = 2;

  /* Hall A/D offset Y */
  g_reg_array[6].reg_addr = 0x14D0;
  g_reg_array[6].reg_data[0] = buffer[ois_offset + 10];
  g_reg_array[6].reg_data[1] = buffer[ois_offset + 11];
  g_reg_array[6].reg_data_size = 2;

  /* Loop gain X */
  g_reg_array[7].reg_addr = 0x10D3;
  g_reg_array[7].reg_data[0] = buffer[ois_offset + 12];
  g_reg_array[7].reg_data[1] = buffer[ois_offset + 13];
  g_reg_array[7].reg_data_size = 2;

  /* Loop gain Y */
  g_reg_array[8].reg_addr = 0x11D3;
  g_reg_array[8].reg_data[0] = buffer[ois_offset + 14];
  g_reg_array[8].reg_data[1] = buffer[ois_offset + 15];
  g_reg_array[8].reg_data_size = 2;

  /* Turn off 16 bit fix mode: 32 bit float mode */
  g_reg_array[9].reg_addr = 0x018C;
  g_reg_array[9].reg_data[0] = 0x00;
  g_reg_array[9].reg_data_size = 1;

  /* Gyro offset */
  g_reg_array[10].reg_addr = 0x02A0;
  g_reg_array[10].reg_data[0] = buffer[ois_offset + 20];
  g_reg_array[10].reg_data_size = 1;

  g_reg_array[11].reg_addr = 0x02A1;
  g_reg_array[11].reg_data[0] = buffer[ois_offset + 21];
  g_reg_array[11].reg_data_size = 1;

  g_reg_array[12].reg_addr = 0x02A2;
  g_reg_array[12].reg_data[0] = buffer[ois_offset + 22];
  g_reg_array[12].reg_data_size = 1;

  g_reg_array[13].reg_addr = 0x02A3;
  g_reg_array[13].reg_data[0] = buffer[ois_offset + 23];
  g_reg_array[13].reg_data_size = 1;

  /* Gyro gain X */
  g_reg_array[14].reg_addr = 0x1020;
  g_reg_array[14].reg_data[0] = buffer[ois_offset + 32];
  g_reg_array[14].reg_data[1] = buffer[ois_offset + 33];
  g_reg_array[14].reg_data[2] = buffer[ois_offset + 34];
  g_reg_array[14].reg_data[3] = buffer[ois_offset + 35];
  g_reg_array[14].reg_data_size = 4;

  /* Gyro gain Y */
  g_reg_array[15].reg_addr = 0x1120;
  g_reg_array[15].reg_data[0] = buffer[ois_offset + 36];
  g_reg_array[15].reg_data[1] = buffer[ois_offset + 37];
  g_reg_array[15].reg_data[2] = buffer[ois_offset + 38];
  g_reg_array[15].reg_data[3] = buffer[ois_offset + 39];
  g_reg_array[15].reg_data_size = 4;

  /* OSC value */
  g_reg_array[16].reg_addr = 0x0257;
  g_reg_array[16].reg_data[0] = buffer[ois_offset + 24];
  g_reg_array[16].reg_data_size = 1;
}

/** sony_imx214_eeprom_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sony_imx214_eeprom_format_afdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  unsigned int af_start_offset = 0;

  if(sony_imx214_check_empty_page(&buffer[AF_PAGE_L2]) != PAGE_EMPTY) {
    SLOW("DEBUGOTP: Read AF data from layer 2");
    af_present = 1;
    af_start_offset = AF_PAGE_L2;
  } else if(sony_imx214_check_empty_page(&buffer[AF_PAGE_L1]) != PAGE_EMPTY) {
    SLOW("DEBUGOTP: Read AF data from layer 1");
    af_present = 1;
    af_start_offset = AF_PAGE_L1;
  } else if(sony_imx214_check_empty_page(&buffer[AF_PAGE_L0]) != PAGE_EMPTY) {
    SLOW("DEBUGOTP: Read AF data from layer 0");
    af_present = 1;
    af_start_offset = AF_PAGE_L0;
  } else {
    SHIGH("OTP:AF data not available");
    af_present = 0;
    return;
  }

  sony_imx214_eeprom_format_afdata_page(e_ctrl, af_start_offset);

  /* ois data on the same pages as AF */
  sony_imx214_eeprom_format_oisdata(e_ctrl, af_start_offset+OIS_START_OFFSET);
}

/** sony_imx214_eeprom_format_afdata:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format the data structure of white balance calibration
 *
 * This function executes in eeprom module context
 *
 * Return: void.
 **/
static void sony_imx214_eeprom_format_lscdata(sensor_eeprom_data_t *e_ctrl)
{
  unsigned char *buffer = e_ctrl->eeprom_params.buffer;
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  unsigned char *a_r_gain, *a_gr_gain, *a_gb_gain, *a_b_gain;
  unsigned short i, j;
  unsigned short x,y;

  if(buffer[WB_PAGE_L2+LSC_FLAG_OFFSET] == 1
    || buffer[WB_PAGE_L1+LSC_FLAG_OFFSET] == 1
    || buffer[WB_PAGE_L0+LSC_FLAG_OFFSET] == 1) {
    lsc_present = 1;
  } else {
    SHIGH("OTP:lsc data not available");
    lsc_present = 0;
    return;
  }
  SLOW("OTP:formating lsc calibration data");
  a_r_gain = (unsigned char *) &(ectrl->eeprom_params.buffer[LSC_R_OFFSET]);
  a_gr_gain = (unsigned char *) &(ectrl->eeprom_params.buffer[LSC_GR_OFFSET]);
  a_gb_gain = (unsigned char *) &(ectrl->eeprom_params.buffer[LSC_GB_OFFSET]);
  a_b_gain = (unsigned char *) &(ectrl->eeprom_params.buffer[LSC_B_OFFSET]);

  /* Load data from eeprom */
  for (j = 0; j < OTP_MESH_HWROLLOFF_SIZE; j++) {

    otp_r[j] = (float)(*(a_r_gain+1) << 8 | *a_r_gain);
    otp_gr[j] = (float)(*(a_gr_gain+1) << 8 | *a_gr_gain);
    otp_gb[j] = (float)(*(a_gb_gain+1) << 8 | *a_gb_gain);
    otp_b[j] = (float)(*(a_b_gain+1) << 8 | *a_b_gain);

    a_r_gain += 2;
    a_gr_gain += 2;
    a_gb_gain += 2;
    a_b_gain += 2;
  }

  memset( mesh_r, 0, sizeof(mesh_r) );
  memset( mesh_gr, 0, sizeof(mesh_gr) );
  memset( mesh_gb, 0, sizeof(mesh_gb) );
  memset( mesh_b, 0, sizeof(mesh_b) );
  /* Convert 7x9 Grid to 13x17 grid for all channels */
  float block_size_x = FULL_SIZE_WIDTH/2/(float)(OTP_GRID_X-1);
  float block_size_y = FULL_SIZE_HEIGHT/2/(float)(OTP_GRID_Y-1);

  mesh_rolloff_V4_ScaleOTPMesh_to_13x17(otp_r, mesh_r, FULL_SIZE_WIDTH,
    FULL_SIZE_HEIGHT, OTP_GRID_X, OTP_GRID_Y, 0,0, block_size_x, block_size_y);
  mesh_rolloff_V4_ScaleOTPMesh_to_13x17(otp_gr, mesh_gr, FULL_SIZE_WIDTH,
    FULL_SIZE_HEIGHT, OTP_GRID_X, OTP_GRID_Y, 0,0, block_size_x, block_size_y);
  mesh_rolloff_V4_ScaleOTPMesh_to_13x17(otp_gb, mesh_gb, FULL_SIZE_WIDTH,
    FULL_SIZE_HEIGHT, OTP_GRID_X, OTP_GRID_Y, 0,0, block_size_x, block_size_y);
  mesh_rolloff_V4_ScaleOTPMesh_to_13x17(otp_b, mesh_b, FULL_SIZE_WIDTH,
    FULL_SIZE_HEIGHT, OTP_GRID_X, OTP_GRID_Y, 0,0, block_size_x, block_size_y);

  for (j = 0; j < MESH_HWROLLOFF_SIZE; j++) {
    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].r_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].r_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].r_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].r_gain[j] =
      mesh_r[j];

    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gr_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].gr_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].gr_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].gr_gain[j] =
      mesh_gr[j];

    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].gb_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].gb_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].gb_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].gb_gain[j] =
      mesh_gb[j];

    ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].b_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].b_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].b_gain[j] =
      ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].b_gain[j] =
      mesh_b[j];
  }

  ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_TL84_LIGHT].mesh_rolloff_table_size=
    MESH_HWROLLOFF_SIZE;
  ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_A_LIGHT].mesh_rolloff_table_size=
    MESH_HWROLLOFF_SIZE;
  ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_D65_LIGHT].mesh_rolloff_table_size=
    MESH_HWROLLOFF_SIZE;
  ectrl->eeprom_data.lsc.lsc_calib[ROLLOFF_H_LIGHT].mesh_rolloff_table_size=
    MESH_HWROLLOFF_SIZE;
}

/** sony_imx214_eeprom_get_ois_raw_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *    @data: point to the destination camera_i2c_seq_reg_setting
 *
 * Get the all the calibration registers setting for ois
 *
 * This function executes in eeprom module context.
 *
 * No return value.
 **/
int sony_imx214_eeprom_get_ois_raw_data(void *e_ctrl, void *data)
{
  RETURN_ERR_ON_NULL(e_ctrl, SENSOR_FAILURE);
  RETURN_ERR_ON_NULL(data, SENSOR_FAILURE);

  memcpy(data, &g_reg_setting, sizeof(g_reg_setting));

  return SENSOR_SUCCESS;
}

/** sony_imx214_eeprom_format_calibration_data:
 *    @e_ctrl: point to sensor_eeprom_data_t of the eeprom device
 *
 * Format all the data structure of calibration
 *
 * This function executes in eeprom module context and generate
 *   all the calibration registers setting of the sensor.
 *
 * Return: void.
 **/
void sony_imx214_eeprom_format_calibration_data(void *e_ctrl) {
   sensor_eeprom_data_t * ctrl = (sensor_eeprom_data_t *)e_ctrl;

  SDBG("Enter");
  RETURN_VOID_ON_NULL(ctrl);

  SLOW("Total bytes in OTP buffer: %d", ctrl->eeprom_params.num_bytes);

  if (!ctrl->eeprom_params.buffer || !ctrl->eeprom_params.num_bytes) {
    SERR("Buff pointer %p buffer size %d", ctrl->eeprom_params.buffer,
      ctrl->eeprom_params.num_bytes);
    return;
  }
  sony_imx214_eeprom_format_wbdata(ctrl);
  sony_imx214_eeprom_format_afdata(ctrl);
  sony_imx214_eeprom_format_lscdata(ctrl);
}

static eeprom_lib_func_t sony_imx214_eeprom_lib_func_ptr = {
  .get_calibration_items = sony_imx214_eeprom_get_calibration_items,
  .format_calibration_data = sony_imx214_eeprom_format_calibration_data,
  .do_af_calibration = eeprom_autofocus_calibration,
  .do_wbc_calibration = eeprom_whitebalance_calibration,
  .do_lsc_calibration = eeprom_lensshading_calibration,
  .get_raw_data = NULL,
  .get_ois_raw_data = sony_imx214_eeprom_get_ois_raw_data,
};

/** sony_imx214_eeprom_eeprom_open_lib:
 *
 * Get the funtion pointer of this lib.
 *
 * This function executes in eeprom module context.
 *
 * Return: eeprom_lib_func_t point to the function pointer.
 **/
void* sony_imx214_eeprom_open_lib(void) {
  return &sony_imx214_eeprom_lib_func_ptr;
}
