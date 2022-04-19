/*============================================================================

  Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include <stdio.h>
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "eeprom_util.h"
#include "debug_lib.h"
#include "sensor_lib.h"
#include <string.h>

#define MODULE_INFO_OFFSET  0
#define WB_OFFSET           0x0009
#define AF_OFFSET           0x0706
#define FAR_MARGIN          (-0.10)
#define NEAR_MARGIN         (0.00)
#define LSC_OFFSET          0x001C
#define LSC_R_OFFSET        0x001D
#define LSC_GR_OFFSET       0x01D7
#define LSC_GB_OFFSET       0x0391
#define LSC_B_OFFSET        0x054B
#define LSC_GRID_SIZE       221
#define QVALUE              1024.0
#define ROLLOFF_MAX_LIGHT   4
#define PDAF_OFFSET         0x0710
#define PDGAIN_WITDH        17
#define PDGAIN_HEIGHT       13
#define PDGAIN_LENGTH2D     (PDGAIN_HEIGHT * PDGAIN_WITDH)
#define DCC_WITDH           8
#define DCC_HEIGHT          6
#define DCC_LENGTH          (DCC_WITDH * DCC_HEIGHT)

#define VALID_FLAG          0x01

static unsigned char        wb_present = 0;
static unsigned char        af_present  = 0;
static unsigned char        lsc_present = 0;

typedef struct {
  unsigned char vendor_id;
  unsigned char year;
  unsigned char month;
  unsigned char day;
} module_info_t;

typedef struct {
  unsigned char r_over_gr_h;
  unsigned char r_over_gr_l;
  unsigned char b_over_gb_h;
  unsigned char b_over_gb_l;
  unsigned char gb_over_gr_h;
  unsigned char gb_over_gr_l;
} wb_data_t;

typedef struct {
  unsigned char macro_h;
  unsigned char macro_l;
  unsigned char infinity_h;
  unsigned char infinity_l;
} af_data_t;

void m24c64s_get_calibration_items(void *e_ctrl);
void m24c64s_format_calibration_data(void *e_ctrl);
static int m24c64s_autofocus_calibration(void *e_ctrl);
static int m24c64s_get_raw_data(void *e_ctrl, void *data);

static eeprom_lib_func_t m24c64s_lib_func_ptr = {
  .get_calibration_items = m24c64s_get_calibration_items,
  .format_calibration_data = m24c64s_format_calibration_data,
  .do_af_calibration = eeprom_autofocus_calibration,
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
          .slave_addr = 0xa2,
          .mem_settings =
          {
            { 0x0, CAMERA_I2C_WORD_ADDR,
              0x0AF2, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 1,
        },
      },
      .size_map_array = 1,
    },
  },
};
