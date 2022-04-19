/*============================================================================

  Copyright (c) 2015 Qualcomm Technologies, Inc.
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
#include "sensor_lib.h"
#include <utils/Log.h>

#undef SLOW
#undef SDBG
/* #define QTECH_F3L8YAM_DEBUG */
#ifdef QTECH_F3L8YAM_DEBUG
#define SLOW(fmt, args...) ALOGE("%s:%d," fmt "\n",__func__,__LINE__, ##args)
#define SDBG(fmt, args...) SERR(fmt, ##args)
#else
#define SLOW(fmt, args...) do { } while (0)
#define SDBG(fmt, args...) do { } while (0)

#endif

#define MODULE_INFO_FLAG_OFFSET 0
#define MODULE_INFO_OFFSET 9

#define AF_FLAG_OFFSET     MODULE_INFO_FLAG_OFFSET+MODULE_INFO_OFFSET
#define AF_OFFSET          8


#define AWB_FLAG_OFFSET    AF_FLAG_OFFSET+AF_OFFSET
#define AWB_OFFSET         14

#define LSC_FLAG_OFFSET    AWB_FLAG_OFFSET+AWB_OFFSET
#define LSC_OFFSET         1770
#define LSC_GRID_SIZE      221 // 17 x 13


#define GAIN_MAP_FLAG_OFFSET    LSC_FLAG_OFFSET+LSC_OFFSET
#define GAIN_MAP_OFFSET   900

#define CC_FLAG_OFFSET    GAIN_MAP_FLAG_OFFSET+GAIN_MAP_OFFSET
#define CC_OFFSET         4


#define VALID_FLAG         0x01
#define QVALUE             1024.0
#define INVALID_DATA       0xFFFF

#define PDGAIN_WITDH        17
#define PDGAIN_HEIGHT       13
#define PDGAIN_LENGTH2D     (PDGAIN_HEIGHT * PDGAIN_WITDH)


/* margin is defined corresponding to the margin of golden */
#define INFINITY_MARGIN    0.25
#define MACRO_MARGIN       0.3

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
  unsigned char r_over_gr_h_golden;
  unsigned char r_over_gr_l_golden;
  unsigned char b_over_gb_h_golden;
  unsigned char b_over_gb_l_golden;
  unsigned char gr_over_gb_h_golden;
  unsigned char gr_over_gb_l_golden;
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

void qtech_f3l8yam_eeprom_get_calibration_items(void *e_ctrl);
void qtech_f3l8yam_eeprom_format_calibration_data(void *e_ctrl);
static int qtech_f3l8yam_autofocus_calibration(void *e_ctrl);

static eeprom_lib_func_t qtech_f3l8yam_eeprom_lib_func_ptr = {
  .get_calibration_items = qtech_f3l8yam_eeprom_get_calibration_items,
  .format_calibration_data = qtech_f3l8yam_eeprom_format_calibration_data,
  .do_af_calibration = qtech_f3l8yam_autofocus_calibration,
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
    .i2c_freq_mode = SENSOR_I2C_MODE_STANDARD,
    .mem_map_array =
    {
      .memory_map =
      {
        {
          .slave_addr = 0xB0,
          .mem_settings =
          {
            { 0x0000, CAMERA_I2C_WORD_ADDR, /* addr: 0x0000~0x0008;size:9 */
              9, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x0ce6, CAMERA_I2C_WORD_ADDR, /* addr: 0x0ce6~0x0ced;size:8 */
              8, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x0cf3, CAMERA_I2C_WORD_ADDR, /* addr: 0x0cf3~0x176e;size:2684 */
              2684, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x1848, CAMERA_I2C_WORD_ADDR, /* addr: 0x1848~0x184b;size;4 */
              4, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
          },
          .memory_map_size = 4,
        },
      },
      .size_map_array = 1,
    },
  },
};
