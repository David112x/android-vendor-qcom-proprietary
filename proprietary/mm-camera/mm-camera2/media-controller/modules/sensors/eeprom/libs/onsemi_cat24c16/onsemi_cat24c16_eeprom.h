/*============================================================================

  Copyright (c) 2014-2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "eeprom_util.h"
#include "debug_lib.h"

#define BL_64 64
#define BL_16 0

#define AWB_OFFSET      8
#define AF_OFFSET       14
#define LSC_OFFSET      20

#define CRC_OFFSET      1788

#define MESH_HWROLLOFF_SIZE (17 * 13)

#define LSC_R_OFFSET    20
#define LSC_GR_OFFSET   (LSC_R_OFFSET + 221 * 2)
#define LSC_B_OFFSET    (LSC_GR_OFFSET + 221 * 2)
#define LSC_GB_OFFSET   (LSC_B_OFFSET + 221 * 2)

#define INF_MARGIN          (-0.2)
#define MACRO_MARGIN        (0.4)

#define QVALUE 1024.0

#if !defined RETURN_ON_NULL
#define RETURN_ON_NULL(expr)         \
    if ((expr) == NULL) {            \
        SERR("failed NULL pointer detected " #expr); \
        return FALSE;                \
     }
#endif

void cat24c16_get_calibration_items(void *e_ctrl);
static void cat24c16_format_calibration_data(void *e_ctrl);
int eeprom_autofocus_calibration_s5k3m2xx(void *e_ctrl);

static eeprom_lib_func_t cat24c16_lib_func_ptr = {
  .get_calibration_items = cat24c16_get_calibration_items,
  .format_calibration_data = cat24c16_format_calibration_data,
  .do_af_calibration = eeprom_autofocus_calibration_s5k3m2xx,
  .do_wbc_calibration = eeprom_whitebalance_calibration,
  .do_lsc_calibration = eeprom_lensshading_calibration,
  .get_raw_data = NULL,
  .get_ois_raw_data = NULL,
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
            { 0x0, CAMERA_I2C_BYTE_ADDR,
              256, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 1,
        },
        {
          .slave_addr = 0xa2,
          .mem_settings =
          {
            { 0x0, CAMERA_I2C_BYTE_ADDR,
              256, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 1,
        },
        {
          .slave_addr = 0xa4,
          .mem_settings =
          {
            { 0x0, CAMERA_I2C_BYTE_ADDR,
              256, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 1,
        },
        {
          .slave_addr = 0xa6,
          .mem_settings =
          {
            { 0x0, CAMERA_I2C_BYTE_ADDR,
              256, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 1,
        },
        {
          .slave_addr = 0xa8,
          .mem_settings =
          {
            { 0x0, CAMERA_I2C_BYTE_ADDR,
              256, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 1,
        },
        {
          .slave_addr = 0xaa,
          .mem_settings =
          {
            { 0x0, CAMERA_I2C_BYTE_ADDR,
              256, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 1,
        },
        {
          .slave_addr = 0xac,
          .mem_settings =
          {
            { 0x0, CAMERA_I2C_BYTE_ADDR,
              254, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 1,
        },
      },
      .size_map_array = 7,
    },
  },
};
