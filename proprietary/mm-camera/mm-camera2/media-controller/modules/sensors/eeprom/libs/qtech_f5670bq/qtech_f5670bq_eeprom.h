/* qtech_f5670bq_eeprom.h
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef  __QTECH_F5670BQ_EEPROM_H__
#define __QTECH_F5670BQ_EEPROM_H__

#include <stdio.h>
#include <string.h>
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "eeprom_util.h"
#include "debug_lib.h"
#include "sensor_lib.h"

#undef DEBUG_INFO
/* #define QTECH_F5670BQ_DEBUG */
#ifdef QTECH_F5670BQ_DEBUG
#define DEBUG_INFO(fmt, args...) SERR(fmt, ##args)
#else
#define DEBUG_INFO(fmt, args...) do { } while (0)
#endif

#define BASE_ADDR 0x7010
#define WB_FLAG_ADDR 0x7023

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

void qtech_f5670bq_get_calibration_items(void *e_ctrl);
void qtech_f5670bq_format_calibration_data(void *e_ctrl);
int qtech_f5670bq_get_raw_data(void *e_ctrl, void *data);

static eeprom_lib_func_t qtech_f5670bq_lib_func_ptr = {
  .get_calibration_items = qtech_f5670bq_get_calibration_items,
  .format_calibration_data = qtech_f5670bq_format_calibration_data,
  .do_af_calibration = NULL,
  .do_wbc_calibration = NULL,
  .do_lsc_calibration = NULL,
  .get_raw_data = qtech_f5670bq_get_raw_data,
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
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x5002, CAMERA_I2C_WORD_ADDR,
              0xa8, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d84, CAMERA_I2C_WORD_ADDR,
              0xc0, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d88, CAMERA_I2C_WORD_ADDR,
              0x7010, CAMERA_I2C_WORD_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d8a, CAMERA_I2C_WORD_ADDR,
              0x702f, CAMERA_I2C_WORD_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x3d81, CAMERA_I2C_WORD_ADDR,
              0x01, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 10 },
            { 0x7010, CAMERA_I2C_WORD_ADDR,
              32, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 1 },
            { 0x5002, CAMERA_I2C_WORD_ADDR,
              0xaa, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
            { 0x5002, CAMERA_I2C_WORD_ADDR,
              0x00, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_WRITE, 1 },
          },
          .memory_map_size = 9,
        },
      },
      .size_map_array = 1,
    },
  },
};

#endif
