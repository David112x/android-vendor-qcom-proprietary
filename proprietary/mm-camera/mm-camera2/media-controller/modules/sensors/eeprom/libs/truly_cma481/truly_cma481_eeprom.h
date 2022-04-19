/* truly_cma481_eeprom.h
 *
 * Copyright (c) 2015, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "eeprom_lib.h"
#include "../eeprom_util/eeprom.h"
#include "sensor_lib.h"
#include "actuator_driver.h"
#include <utils/Log.h>
#include "debug_lib.h"

#undef DEBUG_INFO
#ifdef TRULY_CMA481_DEBUG
#define DEBUG_INFO(fmt, args...) ALOGE("%s:" fmt "\n",__func__, ##args)
#else
#define DEBUG_INFO(fmt, args...) do { } while (0)
#endif


#define BASE_ADDR 0x7010
#define WB_FLAG_ADDR_R2A 0x7016
#define VCM_FLAG_ADDR 0x7030
#define LENS_FLAG_ADDR_R2A 0x7028

#define WB_OFFSET_R2A (WB_FLAG_ADDR_R2A-BASE_ADDR)
#define WB_GROUP_SIZE_R2A 8
#define INFO_GROUP_SIZE_R2A 8

#define VCM_OFFSET (VCM_FLAG_ADDR-BASE_ADDR)
#define VCM_GROUP_SIZE 3

#define LENS_OFFSET_R2A (LENS_FLAG_ADDR_R2A-BASE_ADDR)
#define LENS_GROUP_SIZE_R2A 241

#define LSC_REG_SIZE_R2A 240
#define AWB_REG_SIZE_R2A 3
#define REG_SIZE 4

/*
Checked golden sample at 20140701
RG: 0x140
BG: 0x125
*/
#define RG_RATIO_TYPICAL_VALUE 0x140
#define BG_RATIO_TYPICAL_VALUE 0x125

#define ABS(x)            (((x) < 0) ? -(x) : (x))

struct otp_struct {
  uint16_t module_integrator_id;
  uint16_t lens_id;
  uint16_t production_year;
  uint16_t production_month;
  uint16_t production_day;
  uint16_t rg_ratio;
  uint16_t bg_ratio;
  uint16_t light_rg;
  uint16_t light_bg;
  uint16_t user_data[5];
  uint16_t lenc[62];
  uint16_t VCM_start;
  uint16_t VCM_end;
  uint16_t VCM_dir;
} otp_data;

void truly_cma481_get_calibration_items(void *e_ctrl);
void truly_cma481_format_calibration_data(void *e_ctrl);
int truly_cma481_get_raw_data(void *e_ctrl, void *data);

static eeprom_lib_func_t truly_cma481_lib_func_ptr = {
  .get_calibration_items = truly_cma481_get_calibration_items,
  .format_calibration_data = truly_cma481_format_calibration_data,
  .do_af_calibration = NULL,
  .do_wbc_calibration = NULL,
  .do_lsc_calibration = NULL,
  .get_raw_data = truly_cma481_get_raw_data,
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
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VANA,
          .config_val = 0,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VDIG,
          .config_val = 0,
          .delay = 1,
        },
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
          .delay = 5,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_STANDBY,
          .config_val = GPIO_OUT_HIGH,
          .delay = 5,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_HIGH,
          .delay = 10,
        },
        {
          .seq_type = CAMERA_POW_SEQ_CLK,
          .seq_val = CAMERA_MCLK,
          .config_val = 24000000,
          .delay = 10,
        },
      },
      .size = 8,
      .power_down_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_CLK,
          .seq_val = CAMERA_MCLK,
          .config_val = 0,
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
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 0,
        },
      },
      .size_down = 8,
    },
    .i2c_freq_mode = SENSOR_I2C_MODE_STANDARD,
    .mem_map_array =
    {
      .memory_map =
      {
        {
          .slave_addr = 0x6c,
          .mem_settings =
          {
            { 0x0100, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xc0, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d88, CAMERA_I2C_WORD_ADDR,
              0x7010, CAMERA_I2C_WORD_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d8a, CAMERA_I2C_WORD_ADDR,
              0x720a, CAMERA_I2C_WORD_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 10 },
            { 0x7010, CAMERA_I2C_WORD_ADDR,
              507, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
          },
          .memory_map_size = 6,
        },
      },
      .size_map_array = 1,
    },
  },
};
