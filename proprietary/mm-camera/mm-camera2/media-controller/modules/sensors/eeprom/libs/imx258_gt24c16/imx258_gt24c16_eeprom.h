/*============================================================================

Copyright (c) 2015 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "eeprom_util.h"
#include "debug_lib.h"
#include "sensor_lib.h"

#define MODULE_INFO_OFFSET 0x0001
#define FLAG_OFFSET        0x000A
#define AWB_OFFSET         0x000B
#define AF_OFFSET           0x0013
#define LSC_CRC_OFFSET     0x001B
#define LSC_OFFSET         0x001C
#define LSC_LAST_BYTE      0x046C
#define PDAF_CRC_OFFSET    0x046D
#define PDAF_GAIN_OFFSET   0x046E
#define PDAF_LAST_BYTE      0x07EF
#define EEPROM_LAST_BYTE    0x086E
#define SPC_CRC_OFFSET      0x07F0
#define SPC_OFFSET          0x07F1
#define SPC_ADDR            0xD04C
#define SPC_SETTING_SIZE    (63 * 2)
#define TOTAL_CHECKSUM      0x086F
#define MESH_HWROLLOFF_SIZE (17*13)
#define PDGAIN_WITDH        17
#define PDGAIN_HEIGHT       13
#define PDGAIN_LENGTH2D     (PDGAIN_HEIGHT * PDGAIN_WITDH)
#define VALID_FLAG          0x01
#define INVALID_DATA        0xFFFF
#define QVALUE              1024.0
#define INF_MARGIN          (-0.2)
#define MACRO_MARGIN        (0.4)

typedef struct {
  unsigned char year;
  unsigned char month;
  unsigned char day;
  unsigned char hour;
  unsigned char minute;
  unsigned char second;
  unsigned char module_id;
  unsigned char lens_id;
} module_info_t;

typedef struct {
  unsigned char r_value;
  unsigned char gr_value;
  unsigned char gb_value;
  unsigned char b_value;
  unsigned char r_value_golden;
  unsigned char gr_value_golden;
  unsigned char gb_value_golden;
  unsigned char b_value_golden;
} awb_data_t;

typedef struct {
  unsigned char infinity_l;
  unsigned char infinity_h;
  unsigned char macro_l;
  unsigned char macro_h;
  unsigned char infinity_vert_l;
  unsigned char infinity_vert_h;
  unsigned char macro_vert_l;
  unsigned char macro_vert_h;
} af_data_t;

static unsigned char awb_present      = FALSE;
static unsigned char af_present       = FALSE;
static unsigned char lsc_present      = FALSE;
static unsigned char insensor_present = FALSE;
/* in-sensor SPC (shield pixel correction) */
static struct camera_i2c_reg_setting g_reg_setting;
static struct camera_i2c_reg_array   g_reg_array[SPC_SETTING_SIZE];

void imx258_gt24c16_eeprom_get_calibration_items(void *e_ctrl);
void imx258_gt24c16_eeprom_format_calibration_data(void *e_ctrl);
static int imx258_gt24c16_autofocus_calibration(void *e_ctrl);
static int imx258_get_raw_data(void *e_ctrl, void *data);

static eeprom_lib_func_t imx258_gt24c16_eeprom_lib_func_ptr = {
  .get_calibration_items    = imx258_gt24c16_eeprom_get_calibration_items,
  .format_calibration_data  = imx258_gt24c16_eeprom_format_calibration_data,
  .do_af_calibration        = imx258_gt24c16_autofocus_calibration,
  .do_wbc_calibration       = eeprom_whitebalance_calibration,
  .do_lsc_calibration       = eeprom_lensshading_calibration,
  .get_raw_data             = imx258_get_raw_data,
  .get_ois_raw_data         = NULL,
  .eeprom_info =
  {
    .power_setting_array =
    {
      .power_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 0,
        },
      },
      .size = 1,
      .power_down_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 0,
        },
      },
      .size_down = 1,
    },
    .i2c_freq_mode = SENSOR_I2C_MODE_STANDARD,
    .mem_map_array =
    {
      .memory_map =
      {
        {
          .slave_addr = 0xa0,
          .mem_settings =
          {
            { 0x0, CAMERA_I2C_WORD_ADDR,
              2160, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 1,
        },
      },
      .size_map_array = 1,
    },
  },
};
