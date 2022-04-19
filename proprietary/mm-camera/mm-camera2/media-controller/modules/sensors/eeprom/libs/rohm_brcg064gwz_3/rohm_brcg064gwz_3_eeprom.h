/*============================================================================

  Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include <stdio.h>
#include "chromatix.h"
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "debug_lib.h"
#include "sensor_lib.h"
#include <utils/Log.h>
#include <string.h>

#define WB_OFFSET            272
#define AF_OFFSET            278
#define MESH_HWROLLOFF_SIZE  (17*13)
#define LSC_R_OFFSET         512
#define LSC_GR_OFFSET        (LSC_R_OFFSET+MESH_HWROLLOFF_SIZE*2)
#define LSC_GB_OFFSET        (LSC_GR_OFFSET+MESH_HWROLLOFF_SIZE*2)
#define LSC_B_OFFSET         (LSC_GB_OFFSET+MESH_HWROLLOFF_SIZE*2)
#define PDGAIN_OFFSET        2816
#define DCC_OFFSET           3706
#define PDGAIN_WITDH         17
#define PDGAIN_HEIGHT        13
#define PDGAIN_LENGTH2D      (PDGAIN_HEIGHT * PDGAIN_WITDH)

#define QVALUE 1024.0

#define PAGE_EMPTY 0
#define PAGE_NOT_EMPTY 1
#define MAX_EMPTY_BYTES 8

#define FAR_MARGIN (-0.074)
#define NEAR_MARGIN (0.245)

static unsigned int datapresent = 0;

void brcg064gwz_3_get_calibration_items(void *e_ctrl);
static void brcg064gwz_3_format_calibration_data(void *e_ctrl);

static eeprom_lib_func_t brcg064gwz_3_lib_func_ptr = {
  .get_calibration_items = brcg064gwz_3_get_calibration_items,
  .format_calibration_data = brcg064gwz_3_format_calibration_data,
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
              3840, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 1,
        },
      },
      .size_map_array = 1,
    },
  },
};
