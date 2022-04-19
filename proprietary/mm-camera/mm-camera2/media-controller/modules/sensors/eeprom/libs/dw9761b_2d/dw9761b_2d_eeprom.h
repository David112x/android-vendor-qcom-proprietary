/*============================================================================

  Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
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

#define AWB_FLAG_OFFSET    0
#define MODULE_INFO_OFFSET 1
#define AWB_OFFSET         9

#define LSC_FLAG_OFFSET    22
#define LSC_OFFSET         LSC_FLAG_OFFSET + 1
#define LSC_GRID_SIZE      221 // 17 x 13

//#define AF_FLAG_OFFSET     LSC_OFFSET + LSC_GRID_SIZE * 2 * 4 + 1
#define AF_FLAG_OFFSET     1792

#define AF_OFFSET          AF_FLAG_OFFSET + 1

#define VALID_FLAG         0x01
#define QVALUE             1024.0
#define INVALID_DATA       0xFFFF

#define PDGAIN_FLAG        1800
#define PDGAIN             PDGAIN_FLAG + 1

#define PDGAIN_WITDH         17
#define PDGAIN_HEIGHT        13
#define PDGAIN_LENGTH2D      (PDGAIN_HEIGHT * PDGAIN_WITDH)

//#define DCC_OFFSET         2699
#define DCC_OFFSET         2691



//#define LEFT_GAIN_OFFSET   PDGAIN + 7
//#define PDGAIN_MAP_SIZE    131
//#define RIGHT_GAIN_OFFSET  LEFT_GAIN_OFFSET + PDGAIN_MAP_SIZE * 2

/* margin is defined corresponding to the margin of golden */
#define INFINITY_MARGIN    0.1
#define MACRO_MARGIN       0.1

typedef struct {
  unsigned char id;
  unsigned char year;
  unsigned char month;
  unsigned char day;
} module_info_t;

typedef struct {
  unsigned char r_over_gr_h;
  unsigned char r_over_gr_l;
  unsigned char b_over_gb_h;
  unsigned char b_over_gb_l;
  unsigned char gr_over_gb_h;
  unsigned char gr_over_gb_l;
} awb_data_t;

typedef struct {
  unsigned char macro_h;
  unsigned char macro_l;
  unsigned char infinity_h;
  unsigned char infinity_l;
} af_data_t;

static unsigned char awb_present = 0;
static unsigned char af_present  = 0;
static unsigned char lsc_present = 0;


void dw9761b_2d_eeprom_get_calibration_items(void *e_ctrl);
void dw9761b_2d_eeprom_format_calibration_data(void *e_ctrl);
static int dw9761b_2d_autofocus_calibration(void *e_ctrl);

static eeprom_lib_func_t dw9761b_2d_eeprom_lib_func_ptr = {
  .get_calibration_items = dw9761b_2d_eeprom_get_calibration_items,
  .format_calibration_data = dw9761b_2d_eeprom_format_calibration_data,
  .do_af_calibration = dw9761b_2d_autofocus_calibration,
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
          .seq_val = CAMERA_VDIG,
          .config_val = 0,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VDIG,
          .config_val = GPIO_OUT_HIGH,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VAF,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VANA,
          .config_val = GPIO_OUT_HIGH,
          .delay = 1,
        },
      },
      .size = 5,
      .power_down_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VDIG,
          .config_val = 0,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VDIG,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VAF,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VANA,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
      },
      .size_down = 5,
    },
    .i2c_freq_mode = SENSOR_I2C_MODE_STANDARD,
    .mem_map_array =
    {
      .memory_map =
      {
        {
          .slave_addr = 0xb0,
          .mem_settings =
          {
            { 0x400, CAMERA_I2C_WORD_ADDR,
              (0x0EE9 - 0x400), CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
#if 0
            { 0x0F11, CAMERA_I2C_WORD_ADDR,
              2, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0F10, CAMERA_I2C_WORD_ADDR,
              1, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
#endif
          },
          .memory_map_size = 1,
        },
#if 0
        {
          .slave_addr = 0x5a,
          .mem_settings =
          {
            { 0x6028, CAMERA_I2C_WORD_ADDR,
              0x4000, CAMERA_I2C_WORD_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0136, CAMERA_I2C_WORD_ADDR,
              0x1800, CAMERA_I2C_WORD_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0304, CAMERA_I2C_WORD_ADDR,
              0x0006, CAMERA_I2C_WORD_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0306, CAMERA_I2C_WORD_ADDR,
              0x0073, CAMERA_I2C_WORD_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x030C, CAMERA_I2C_WORD_ADDR,
              0x0004, CAMERA_I2C_WORD_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x030E, CAMERA_I2C_WORD_ADDR,
              0x0064, CAMERA_I2C_WORD_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0302, CAMERA_I2C_WORD_ADDR,
              0x0001, CAMERA_I2C_WORD_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0300, CAMERA_I2C_WORD_ADDR,
              0x0004, CAMERA_I2C_WORD_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x030A, CAMERA_I2C_WORD_ADDR,
              0x0001, CAMERA_I2C_WORD_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0308, CAMERA_I2C_WORD_ADDR,
              0x0008, CAMERA_I2C_WORD_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0100, CAMERA_I2C_WORD_ADDR,
              0x0100, CAMERA_I2C_WORD_DATA, CAMERA_I2C_OP_WRITE, 10 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x0D, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              64, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x0E, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              64, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x0F, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              64, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x10, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              64, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x11, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              64, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x12, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              64, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x13, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              64, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x14, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              64, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x15, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              64, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x16, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              64, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x17, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              64, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x18, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              64, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x19, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              64, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x1A, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              64, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x1B, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              64, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x0A02, CAMERA_I2C_WORD_ADDR,
              0x1C, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0A00, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0xA04, CAMERA_I2C_WORD_ADDR,
              60, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
          },
          .memory_map_size = 59,
        },
#endif
      },
      .size_map_array = 1,
    },
  },
};
