/*============================================================================

Copyright (c) 2017 Qualcomm Technologies, Inc.
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
#include <utils/Log.h>

#define MODULE_INFO_OFFSET      0

#define AF_OFFSET               0x0C

#define AWB_OFFSET              0x19

#define LSC_OFFSET              0x0027
#define LSC_GRID_SIZE           221 // 17 x 13

#define GAIN_OFFSET             0x0711
#define PDAF_DCC_OFFSET         0x0D9A
#define DCC_LENGTH              48 /* 8*6 */


#define VALID_FLAG         0x01
#define QVALUE             1024.0
#define INVALID_DATA       0xFFFF

#define PDGAIN_WITDH        17
#define PDGAIN_HEIGHT       13
#define PDGAIN_LENGTH2D     (PDGAIN_HEIGHT * PDGAIN_WITDH)


/* margin is defined corresponding to the margin of golden */
#define INF_MARGIN         (-0.1)
#define MACRO_MARGIN       (0.25)

typedef struct {
  unsigned short vendor_id;
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
  unsigned char golden_r_over_gr_h;
  unsigned char golden_r_over_gr_l;
  unsigned char golden_b_over_gb_h;
  unsigned char golden_b_over_gb_l;
  unsigned char golden_gr_over_gb_h;
  unsigned char golden_gr_over_gb_l;
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

void sunny_ov13855_eeprom_get_calibration_items(void *e_ctrl);
void sunny_ov13855_eeprom_format_calibration_data(void *e_ctrl);

static eeprom_lib_func_t sunny_ov13855_eeprom_lib_func_ptr = {
  .get_calibration_items = sunny_ov13855_eeprom_get_calibration_items,
  .format_calibration_data = sunny_ov13855_eeprom_format_calibration_data,
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
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_STANDBY,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VANA,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VDIG,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_CLK,
          .seq_val = CAMERA_MCLK,
          .config_val = 24000000,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_HIGH,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_STANDBY,
          .config_val = GPIO_OUT_HIGH,
          .delay = 1,
        },
      },
      .size = 8,
      .power_down_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_STANDBY,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_CLK,
          .seq_val = CAMERA_MCLK,
          .config_val = 0,
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
          .seq_val = CAMERA_VDIG,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VANA,
          .config_val = 0,
          .delay = 0,
        },
      },
      .size_down = 6,
    },
    .i2c_freq_mode = SENSOR_I2C_MODE_FAST,
    .mem_map_array =
    {
      .memory_map =
      {
        {
          .slave_addr = 0xA0,
          .mem_settings =
          {
            { 0x0000, CAMERA_I2C_WORD_ADDR,
              0x0E37, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 }
          },
          .memory_map_size = 1,
        },
      },
      .size_map_array = 1,
    },
  },
};
