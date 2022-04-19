/*============================================================================

  Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include <stdio.h>
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "eeprom_util.h"
#include "debug_lib.h"
#include "sensor_lib.h"
#include <utils/Log.h>
#include <string.h>

#define ID_OFFSET 0
#define WB_OFFSET 6
#define WB_GOLDEN_OFFSET 12
#define AF_OFFSET 18
#define LSC_OFFSET 512
#define QVALUE 1024.0
#define MESH_HWROLLOFF_SIZE 221
#define ROLLOFF_MAX_LIGHT 3

#define DC_OFFSET 0x1D33
#define DC_MAIN (DC_OFFSET+4)
#define DC_AUX (DC_OFFSET+20)
#define DC_SIZE 470 /* Size is the total number of bytes in OTP */

#define DC_ROTATION_MATRIX 9
#define DC_GEOMETRIC_MATRIX 32
#define DC_OTHERS (DC_OFFSET + 200)

#if !defined RETURN_ON_NULL
#define RETURN_ON_NULL(expr)         \
    if ((expr) == NULL) {            \
        SERR("failed NULL pointer detected " #expr); \
        return FALSE;                \
     }
#endif

#define SPC_OFFSET 0x1A4E
#define SPC_SETTING_SIZE 352
#define DCC_OFFSET 0x1BAE

#define FULL_SIZE_WIDTH 5344
#define FULL_SIZE_HEIGHT 4016

#define QVALUE 1024.0

#define PAGE_EMPTY 0
#define PAGE_NOT_EMPTY 1
#define MAX_EMPTY_BYTES 32

#define HALL_OFFSET 1792

#define SPC_ADDR 0x7c00
#define PD_KNOT_X 8
#define PD_KNOT_Y 6
#define PD_OFFSET_X 112
#define PD_OFFSET_Y 88
#define PD_AREA_X 640
#define PD_AREA_Y 640

#define FAR_MARGIN (0.00)
#define NEAR_MARGIN (-0.08)

void le2464c_get_calibration_items(void *e_ctrl);
void le2464c_format_calibration_data(void *e_ctrl);
static int le2464c_autofocus_calibration(void *e_ctrl);
static int le2464c_get_raw_data(void *e_ctrl, void *data);

static eeprom_lib_func_t le2464c_lib_func_ptr = {
  .get_calibration_items = le2464c_get_calibration_items,
  .format_calibration_data = le2464c_format_calibration_data,
  .do_af_calibration = le2464c_autofocus_calibration,
  .do_wbc_calibration = eeprom_whitebalance_calibration,
  .do_lsc_calibration = eeprom_lensshading_calibration,
  .get_raw_data = le2464c_get_raw_data,
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
            { 0x0, CAMERA_I2C_WORD_ADDR,
              8192, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 1,
        },
      },
      .size_map_array = 1,
    },
  },
};
