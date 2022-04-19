/*============================================================================

Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
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

#define MODULE_INFO_OFFSET          0

#define AF_OFFSET                   0x24

#define AWB_OFFSET                  0x000C

#define LSC_OFFSET                  0x0200
#define MESH_HWROLLOFF_SIZE         (17*13)

#define PDAF_VERSION_NUMBER_OFFSET  0x0E0F
#define PDAF_MAP_WIDTH_OFFSET       0x0E11
#define PDAF_MAP_HEIGHT_OFFSET      0x0E13
#define PDAF_LEFT_GAIN_OFFSET       0x0E15
#define PDAF_RIGHT_GAIN_OFFSET      0x0FCF
#define DCC_Q_FACTORY_OFFSET        0x118B
#define DCC_MAP_WIDTH_OFFSET        0x118D
#define DCC_MAP_HEIGHT_OFFSET       0x118F
#define DCC_CONV_COEFF_OFFSET       0x1191

#define PDAF_GAIN_LENGTH            221 /* 17*13 */
#define DCC_LENGTH                  48 /* 8*6 */
#define VALID_FLAG                  0x01
#define QVALUE                      1024.0
#define INVALID_DATA                0xFFFF


/* margin is defined corresponding to the margin of golden */
#define INF_MARGIN         (-0.1)
#define MACRO_MARGIN       (0.1)

typedef struct {
  unsigned char vendor_id; /*sunny*/
  unsigned char year;
  unsigned char month;
  unsigned char day;
} module_info_t;

typedef struct {
  unsigned char r_avg_l;
  unsigned char r_avg_h;
  unsigned char gr_avg_l;
  unsigned char gr_avg_h;
  unsigned char gb_avg_l;
  unsigned char gb_avg_h;
  unsigned char b_avg_l;
  unsigned char b_avg_h;
  unsigned char r_avg_l_golden;
  unsigned char r_avg_h_golden;
  unsigned char gr_avg_l_golden;
  unsigned char gr_avg_h_golden;
  unsigned char gb_avg_l_golden;
  unsigned char gb_avg_h_golden;
  unsigned char b_avg_l_golden;
  unsigned char b_avg_h_golden;
} awb_data_t;

static unsigned char awb_present = 0;
static unsigned char af_present  = 0;
static unsigned char lsc_present = 0;

void sunny_gt24p64b_imx519_eeprom_get_calibration_items(void *e_ctrl);
void sunny_gt24p64b_imx519_eeprom_format_calibration_data(void *e_ctrl);

static eeprom_lib_func_t sunny_gt24p64b_imx519_eeprom_lib_func_ptr = {
  .get_calibration_items = sunny_gt24p64b_imx519_eeprom_get_calibration_items,
  .format_calibration_data = sunny_gt24p64b_imx519_eeprom_format_calibration_data,
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
              0x1200, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 }
          },
          .memory_map_size = 1,
        },
      },
      .size_map_array = 1,
    },
  },
};
