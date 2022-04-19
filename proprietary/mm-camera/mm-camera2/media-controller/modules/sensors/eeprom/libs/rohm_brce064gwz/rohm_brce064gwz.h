/*============================================================================

  Copyright (c) 2014-2015 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "chromatix.h"
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "debug_lib.h"
#include "sensor_lib.h"

#define WB_OFFSET 272
#define AF_OFFSET 278
#define MESH_HWROLLOFF_SIZE (17*13)
#define LSC_OFFSET 512
#define LSC_R_OFFSET 512
#define LSC_GR_OFFSET (LSC_R_OFFSET+MESH_HWROLLOFF_SIZE*2)
#define LSC_GB_OFFSET (LSC_GR_OFFSET+MESH_HWROLLOFF_SIZE*2)
#define LSC_B_OFFSET (LSC_GB_OFFSET+MESH_HWROLLOFF_SIZE*2)

#define FULL_SIZE_WIDTH 4208
#define FULL_SIZE_HEIGHT 3120

#define QVALUE 1024.0

#define PAGE_EMPTY 0
#define PAGE_NOT_EMPTY 1
#define MAX_EMPTY_BYTES 8

#define FAR_MARGIN (-0.1)
#define NEAR_MARGIN (0.3)

#define DC_OFFSET 2304
#define DC_MAIN (DC_OFFSET + 4)
#define DC_AUX (DC_OFFSET + 20)
#define DC_ROTATION_MATRIX 9
#define DC_GEOMETRIC_MATRIX 32
#define DC_OTHERS (DC_OFFSET + 200)
#define DC_SIZE 512 /* Size is the total number of bytes in OTP */

static unsigned int datapresent = 0;

void brce064gwz_get_calibration_items(void *e_ctrl);
static void brce064gwz_format_calibration_data(void *e_ctrl);

static eeprom_lib_func_t brce064gwz_lib_func_ptr = {
  .get_calibration_items = brce064gwz_get_calibration_items,
  .format_calibration_data = brce064gwz_format_calibration_data,
  .do_af_calibration = eeprom_autofocus_calibration,
  .do_wbc_calibration = eeprom_whitebalance_calibration,
  .do_lsc_calibration = eeprom_lensshading_calibration,
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
          .slave_addr = 0xa8,
          .mem_settings =
          {
            { 0x0, CAMERA_I2C_WORD_ADDR,
              2900, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 1,
        },
      },
      .size_map_array = 1,
    },
  },
};
