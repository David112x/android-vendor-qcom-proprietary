/**
 * onsemi_cat24c32.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <string.h>
#include "chromatix.h"
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "debug_lib.h"
#include "sensor_lib.h"

#if !defined RETURN_ON_NULL
#define RETURN_ON_NULL(expr)\
    if ((expr) == NULL) {\
        SERR("failed NULL pointer detected " #expr);\
        return FALSE;\
     }
#endif

#define WB_OFFSET 14
#define AF_OFFSET 20
#define MESH_HWROLLOFF_SIZE (17*13)
#define LSC_OFFSET 24
#define LSC_R_OFFSET 24
#define LSC_GR_OFFSET (LSC_R_OFFSET+MESH_HWROLLOFF_SIZE*2)
#define LSC_B_OFFSET (LSC_GR_OFFSET+MESH_HWROLLOFF_SIZE*2)
#define LSC_GB_OFFSET (LSC_B_OFFSET+MESH_HWROLLOFF_SIZE*2)

#define SPC_OFFSET 1798
#define SPC_SETTING_SIZE 352
#define DCC_OFFSET 2150

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

#define FAR_MARGIN (-0.15)
#define NEAR_MARGIN (0.3)

static unsigned int datapresent = 0;

/* in-sensor SPC (shield pixel correction) */
static struct camera_i2c_reg_setting g_reg_setting;
static struct camera_i2c_reg_array g_reg_array[SPC_SETTING_SIZE];
static unsigned char hall_offset = 0, hall_bias = 0;

void cat24c32_get_calibration_items(void *e_ctrl);
static void cat24c32_format_calibration_data(void *e_ctrl);
static int cat24c32_autofocus_calibration(void *e_ctrl);
static int cat24c32_get_raw_data(void *e_ctrl, void *data);

static eeprom_lib_func_t cat24c32_lib_func_ptr = {
  .get_calibration_items = cat24c32_get_calibration_items,
  .format_calibration_data = cat24c32_format_calibration_data,
  .do_af_calibration = cat24c32_autofocus_calibration,
  .do_wbc_calibration = eeprom_whitebalance_calibration,
  .do_lsc_calibration = eeprom_lensshading_calibration,
  .get_raw_data = cat24c32_get_raw_data,
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
              2245, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 1,
        },
      },
      .size_map_array = 1,
    },
  },
};
