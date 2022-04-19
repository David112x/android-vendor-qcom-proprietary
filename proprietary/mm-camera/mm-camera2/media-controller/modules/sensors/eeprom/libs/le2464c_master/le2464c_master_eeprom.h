/*============================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include <stdio.h>
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "eeprom_util.h"
#include "debug_lib.h"

#define ID_OFFSET 0
#define WB_OFFSET 6
#define WB_GOLDEN_OFFSET 12
#define AF_OFFSET 18
#define LSC_OFFSET 512
#define QVALUE 1024.0
#define MESH_HWROLLOFF_SIZE 221
#define ROLLOFF_MAX_LIGHT 3
#define DC_OFFSET 0x800
#define DC_MAIN (DC_OFFSET+4)
#define DC_AUX (DC_OFFSET+20)
#define DC_ROTATION_MATRIX 9
#define DC_GEOMETRIC_MATRIX 32
#define DC_OTHERS (DC_OFFSET + 200)
#define DC_SIZE 470 /* Size is the total number of bytes in OTP */

#if !defined RETURN_ON_NULL
#define RETURN_ON_NULL(expr)         \
    if ((expr) == NULL) {            \
        SERR("failed NULL pointer detected " #expr); \
        return FALSE;                \
     }
#endif

#define FAR_MARGIN (0.00)
#define NEAR_MARGIN (-0.08)

void le2464c_master_get_calibration_items(void *e_ctrl);
void le2464c_master_format_calibration_data(void *e_ctrl);
static int le2464c_master_autofocus_calibration(void *e_ctrl);

static eeprom_lib_func_t le2464c_master_lib_func_ptr = {
  .get_calibration_items = le2464c_master_get_calibration_items,
  .format_calibration_data = le2464c_master_format_calibration_data,
  .do_af_calibration = le2464c_master_autofocus_calibration,
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
              2048, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x1A52, CAMERA_I2C_WORD_ADDR,
              470, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 2,
        },
      },
      .size_map_array = 1,
    },
  },
};
