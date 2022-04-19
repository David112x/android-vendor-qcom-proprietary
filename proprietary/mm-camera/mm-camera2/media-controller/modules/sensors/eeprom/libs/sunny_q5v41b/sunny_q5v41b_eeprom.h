/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <stdio.h>
#include <string.h>
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "eeprom_util.h"
#include "debug_lib.h"
#include "sensor_lib.h"
#include <utils/Log.h>

#undef DEBUG_INFO
#define SUNNY_Q5V41B_DEBUG
#ifdef SUNNY_Q5V41B_DEBUG
#define DEBUG_INFO(fmt, args...) ALOGE(fmt, ##args)
#else
#define DEBUG_INFO(fmt, args...) do { } while (0)
#endif


#define BASE_ADDR 0x7010
#define WB_FLAG_ADDR 0x7020

#define WB_OFFSET (WB_FLAG_ADDR-BASE_ADDR)
#define WB_GROUP_SIZE 5

#define AWB_REG_SIZE 6

#define RG_RATIO_TYPICAL_VALUE 0x12D
#define BG_RATIO_TYPICAL_VALUE 0x132

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
} otp_data;


struct camera_i2c_reg_setting g_reg_setting;
struct camera_i2c_reg_array g_reg_array[AWB_REG_SIZE];

void sunny_q5v41b_get_calibration_items(void *e_ctrl);
void sunny_q5v41b_format_calibration_data(void *e_ctrl);
int sunny_q5v41b_get_raw_data(void *e_ctrl, void *data);


static eeprom_lib_func_t sunny_q5v41b_lib_func_ptr = {
  .get_calibration_items    = sunny_q5v41b_get_calibration_items,
  .format_calibration_data  = sunny_q5v41b_format_calibration_data,
  .do_af_calibration        = NULL,
  .do_wbc_calibration       = NULL,
  .do_lsc_calibration       = NULL,
  .get_raw_data             = sunny_q5v41b_get_raw_data,
  .get_ois_raw_data         = NULL,
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
          .seq_type = CAMERA_POW_SEQ_CLK,
          .seq_val = CAMERA_MCLK,
          .config_val = 24000000,
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
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_CLK,
          .seq_val = CAMERA_MCLK,
          .config_val = 0,
          .delay = 1,
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
      .size_down = 6,
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
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x5002, CAMERA_I2C_WORD_ADDR,
              0x28, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xc0, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x3d88, CAMERA_I2C_WORD_ADDR,
              0x70, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x3d89, CAMERA_I2C_WORD_ADDR,
              0x10, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x3d8a, CAMERA_I2C_WORD_ADDR,
              0x70, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x3d8b, CAMERA_I2C_WORD_ADDR,
              0x29, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x7010, CAMERA_I2C_WORD_ADDR,
              26, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 },
            { 0x5002, CAMERA_I2C_WORD_ADDR,
              0x2a, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
            { 0x0100, CAMERA_I2C_WORD_ADDR,
              0x00, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 0 },
          },
          .memory_map_size = 11,
        },
      },
      .size_map_array = 1,
    },
  },
};


