/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <string.h>
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "eeprom_util.h"
#include "debug_lib.h"
#include "sensor_lib.h"



#define AF_OFFSET           0x10
#define AWB_OFFSET          0x20

#define VALID_FLAG          0x01
#define QVALUE              1024.0
#define INVALID_DATA        0xFFFF

#define SPC_OFFSET          0x3e
#define SPC_SETTING_SIZE    96

#define SPC_ADDR            0x7600
#define PD_KNOT_X           8
#define PD_KNOT_Y           6
#define PD_OFFSET_X         12
#define PD_OFFSET_Y         16
#define PD_AREA_X           504
#define PD_AREA_Y           502

#define DCC_OFFSET          0x9e

#define INF_MARGIN          (-0.2)
#define MACRO_MARGIN        (0.4)

typedef struct {
  unsigned char r_over_gr_h;
  unsigned char r_over_gr_l;
  unsigned char b_over_gb_h;
  unsigned char b_over_gb_l;
  unsigned char gr_over_gb_h;
  unsigned char gr_over_gb_l;
} awb_data_t;

typedef struct {
  unsigned char infinity_h;
  unsigned char infinity_l;
  unsigned char macro_h;
  unsigned char macro_l;
} af_data_t;

/* in-sensor SPC (shield pixel correction) */
static struct camera_i2c_reg_setting g_reg_setting;
static struct camera_i2c_reg_array g_reg_array[SPC_SETTING_SIZE];

void sony_imx378_eeprom_get_calibration_items(void *e_ctrl);
void sony_imx378_eeprom_format_calibration_data(void *e_ctrl);
static int sony_imx378_eeprom_get_raw_data(void *e_ctrl, void *data);

static eeprom_lib_func_t sony_imx378_eeprom_lib_func_ptr = {
  .get_calibration_items    = sony_imx378_eeprom_get_calibration_items,
  .format_calibration_data  = sony_imx378_eeprom_format_calibration_data,
  .do_af_calibration        = eeprom_autofocus_calibration,
  .do_wbc_calibration       = eeprom_whitebalance_calibration,
  .do_lsc_calibration       = NULL,
  .get_raw_data             = sony_imx378_eeprom_get_raw_data,
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
    .i2c_freq_mode = SENSOR_I2C_MODE_FAST,
    .mem_map_array =
    {
      .memory_map =
      {
        {
          .slave_addr = 0xa0,
          .mem_settings =
          {
            { 0x0, CAMERA_I2C_WORD_ADDR,
              256, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 1,
        },
      },
      .size_map_array = 1,
    },
  },
};


