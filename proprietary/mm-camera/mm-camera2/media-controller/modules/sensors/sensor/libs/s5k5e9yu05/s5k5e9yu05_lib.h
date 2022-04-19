/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __s5k5e9yu05_LIB_H__
#define __s5k5e9yu05_LIB_H__

#include "sensor_lib.h"

#define SENSOR_MODEL "s5k5e9yu05"

/* s5k5e9yu05 Regs */
#define s5k5e9yu05_DIG_GAIN_GR_ADDR       0x020E /* 0x4000020E */
#define s5k5e9yu05_DIG_GAIN_R_ADDR        0x0210 /* 0x40000210 */
#define s5k5e9yu05_DIG_GAIN_B_ADDR        0x0212 /* 0x40000212 */
#define s5k5e9yu05_DIG_GAIN_GB_ADDR       0x0214 /* 0x40000214 */

/* STATS DATA TYPE */
#define s5k5e9yu05_DATA_PEDESTAL            0x40 /* 10bit value */

#define s5k5e9yu05_MIN_AGAIN_REG_VAL        32 /* 1.0x */
#define s5k5e9yu05_MAX_AGAIN_REG_VAL        512 /* 16.0x */

#define s5k5e9yu05_MIN_DGAIN_REG_VAL        256 /* 1.0x */
#define s5k5e9yu05_MAX_DGAIN_REG_VAL        256 /* 1.0x */

#define s5k5e9yu05_MAX_DGAIN_DECIMATOR      256

/* s5k5e9yu05 FORMULAS */
#define s5k5e9yu05_MIN_AGAIN     1.0
#define s5k5e9yu05_MAX_AGAIN     16.0

#define s5k5e9yu05_MIN_DGAIN    (s5k5e9yu05_MIN_DGAIN_REG_VAL / s5k5e9yu05_MAX_DGAIN_DECIMATOR)
#define s5k5e9yu05_MAX_DGAIN    (s5k5e9yu05_MAX_DGAIN_REG_VAL / s5k5e9yu05_MAX_DGAIN_DECIMATOR)

#define s5k5e9yu05_MIN_GAIN     s5k5e9yu05_MIN_AGAIN * s5k5e9yu05_MIN_DGAIN
#define s5k5e9yu05_MAX_GAIN     s5k5e9yu05_MAX_AGAIN * s5k5e9yu05_MAX_DGAIN

int32_t s5k5e9yu05_calculate_exposure(float real_gain,
  unsigned int line_count, sensor_exposure_info_t *exp_info, float s_real_gain);

int32_t s5k5e9yu05_fill_exposure_array(unsigned int gain,
  unsigned int digital_gain, unsigned int line, unsigned int fl_lines,
  __attribute__((unused)) int luma_avg, unsigned int hdr_param,
  struct camera_i2c_reg_setting* reg_setting,
  __attribute__((unused)) unsigned int s_gain,
  __attribute__((unused)) int s_linecount,
  __attribute__((unused)) int is_hdr_enabled);

#define START_REG_ARRAY \
{ \
  {0x0100, 0x01, 0x00}, \
}

#define STOP_REG_ARRAY \
{ \
  {0x0100, 0x00, 0x00}, \
}

#define GROUPON_REG_ARRAY \
{ \
  {0x0104, 0x01, 0x00}, \
}

#define GROUPOFF_REG_ARRAY \
{ \
  {0x0104, 0x00, 0x00}, \
}

#define INIT0_REG_ARRAY \
{ \
  {0x0100, 0x00, 0x00}, \
  {0x3400, 0x00, 0x00}, \
  {0x0B00, 0x01, 0x00}, \
  {0x3B45, 0x01, 0x00}, \
  {0x0B05, 0x01, 0x00}, \
  {0x392F, 0x01, 0x00}, \
  {0x3930, 0x00, 0x00}, \
  {0x3924, 0x7F, 0x00}, \
  {0x3925, 0xFD, 0x00}, \
  {0x3C08, 0xFF, 0x00}, \
  {0x3C09, 0xFF, 0x00}, \
  {0x3C31, 0xFF, 0x00}, \
  {0x3C32, 0xFF, 0x00}, \
  {0x3290, 0x10, 0x00}, \
  {0x3200, 0x01, 0x00}, \
  {0x3074, 0x06, 0x00}, \
  {0x3075, 0x2F, 0x00}, \
  {0x308A, 0x20, 0x00}, \
  {0x308B, 0x08, 0x00}, \
  {0x308C, 0x0B, 0x00}, \
  {0x3081, 0x07, 0x00}, \
  {0x307B, 0x85, 0x00}, \
  {0x307A, 0x0A, 0x00}, \
  {0x3079, 0x0A, 0x00}, \
  {0x306E, 0x71, 0x00}, \
  {0x306F, 0x28, 0x00}, \
  {0x301F, 0x20, 0x00}, \
  {0x3012, 0x4E, 0x00}, \
  {0x306B, 0x9A, 0x00}, \
  {0x3091, 0x16, 0x00}, \
  {0x30C4, 0x06, 0x00}, \
  {0x306A, 0x79, 0x00}, \
  {0x30B0, 0xFF, 0x00}, \
  {0x306D, 0x08, 0x00}, \
  {0x3084, 0x16, 0x00}, \
  {0x3070, 0x0F, 0x00}, \
  {0x30C2, 0x05, 0x00}, \
  {0x3069, 0x87, 0x00}, \
  {0x3C0F, 0x00, 0x00}, \
  {0x0A02, 0x3F, 0x00}, \
  {0x3083, 0x14, 0x00}, \
  {0x3080, 0x08, 0x00}, \
  {0x3C34, 0xEA, 0x00}, \
  {0x3C35, 0x5C, 0x00}, \
}

#define RES0_REG_ARRAY \
{ \
  {0x0100, 0x00, 0x00}, \
  {0x0136, 0x18, 0x00}, \
  {0x0137, 0x00, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0x5F, 0x00}, \
  {0x030D, 0x04, 0x00}, \
  {0x030E, 0x00, 0x00}, \
  {0x030F, 0x92, 0x00}, \
  {0x3C1F, 0x00, 0x00}, \
  {0x3C17, 0x00, 0x00}, \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x01, 0x00}, \
  {0x0820, 0x03, 0x00}, \
  {0x0821, 0x6C, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3929, 0x0F, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x00, 0x00}, \
  {0x0347, 0x00, 0x00}, \
  {0x0348, 0x0A, 0x00}, \
  {0x0349, 0x2F, 0x00}, \
  {0x034A, 0x07, 0x00}, \
  {0x034B, 0xA7, 0x00}, \
  {0x034C, 0x0A, 0x00}, \
  {0x034D, 0x30, 0x00}, \
  {0x034E, 0x07, 0x00}, \
  {0x034F, 0xA8, 0x00}, \
  {0x0900, 0x00, 0x00}, \
  {0x0901, 0x00, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0101, 0x00, 0x00}, \
  {0x0340, 0x07, 0x00}, \
  {0x0341, 0xEE, 0x00}, \
  {0x0342, 0x0C, 0x00}, \
  {0x0343, 0x28, 0x00}, \
  {0x0200, 0x0B, 0x00}, \
  {0x0201, 0x9C, 0x00}, \
  {0x0202, 0x00, 0x00}, \
  {0x0203, 0x02, 0x00}, \
  {0x30B8, 0x2E, 0x00}, \
  {0x30BA, 0x36, 0x00}, \
}

#define RES1_REG_ARRAY \
{ \
  {0x0100, 0x00, 0x00}, \
  {0x0136, 0x18, 0x00}, \
  {0x0137, 0x00, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0x5F, 0x00}, \
  {0x030D, 0x04, 0x00}, \
  {0x030E, 0x00, 0x00}, \
  {0x030F, 0x92, 0x00}, \
  {0x3C1F, 0x00, 0x00}, \
  {0x3C17, 0x00, 0x00}, \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x01, 0x00}, \
  {0x0820, 0x03, 0x00}, \
  {0x0821, 0x6C, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3929, 0x0F, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x08, 0x00}, \
  {0x0346, 0x00, 0x00}, \
  {0x0347, 0x08, 0x00}, \
  {0x0348, 0x0A, 0x00}, \
  {0x0349, 0x27, 0x00}, \
  {0x034A, 0x07, 0x00}, \
  {0x034B, 0x9F, 0x00}, \
  {0x034C, 0x05, 0x00}, \
  {0x034D, 0x10, 0x00}, \
  {0x034E, 0x03, 0x00}, \
  {0x034F, 0xCC, 0x00}, \
  {0x0900, 0x01, 0x00}, \
  {0x0901, 0x22, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x03, 0x00}, \
  {0x0340, 0x07, 0x00}, \
  {0x0341, 0xEE, 0x00}, \
  {0x0342, 0x0C, 0x00}, \
  {0x0343, 0x28, 0x00}, \
  {0x0200, 0x0B, 0x00}, \
  {0x0201, 0x9C, 0x00}, \
  {0x0202, 0x00, 0x00}, \
  {0x0203, 0x02, 0x00}, \
  {0x30B8, 0x2A, 0x00}, \
  {0x30BA, 0x2E, 0x00}, \
}

#define RES2_REG_ARRAY \
{ \
  {0x0100, 0x00, 0x00}, \
  {0x0136, 0x18, 0x00}, \
  {0x0137, 0x00, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0x5F, 0x00}, \
  {0x030D, 0x04, 0x00}, \
  {0x030E, 0x00, 0x00}, \
  {0x030F, 0x92, 0x00}, \
  {0x3C1F, 0x00, 0x00}, \
  {0x3C17, 0x00, 0x00}, \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x01, 0x00}, \
  {0x0820, 0x03, 0x00}, \
  {0x0821, 0x6C, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3929, 0x0F, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x08, 0x00}, \
  {0x0346, 0x00, 0x00}, \
  {0x0347, 0x08, 0x00}, \
  {0x0348, 0x0A, 0x00}, \
  {0x0349, 0x27, 0x00}, \
  {0x034A, 0x07, 0x00}, \
  {0x034B, 0x9F, 0x00}, \
  {0x034C, 0x05, 0x00}, \
  {0x034D, 0x10, 0x00}, \
  {0x034E, 0x03, 0x00}, \
  {0x034F, 0xCC, 0x00}, \
  {0x0900, 0x01, 0x00}, \
  {0x0901, 0x22, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x03, 0x00}, \
  {0x0340, 0x03, 0x00}, \
  {0x0341, 0xF9, 0x00}, \
  {0x0342, 0x0C, 0x00}, \
  {0x0343, 0x28, 0x00}, \
  {0x0200, 0x0B, 0x00}, \
  {0x0201, 0x9C, 0x00}, \
  {0x0202, 0x00, 0x00}, \
  {0x0203, 0x02, 0x00}, \
  {0x30B8, 0x2A, 0x00}, \
  {0x30BA, 0x2E, 0x00}, \
}

#define RES3_REG_ARRAY \
{ \
  {0x0100, 0x00, 0x00}, \
  {0x0136, 0x18, 0x00}, \
  {0x0137, 0x00, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0x5F, 0x00}, \
  {0x030D, 0x04, 0x00}, \
  {0x030E, 0x00, 0x00}, \
  {0x030F, 0x92, 0x00}, \
  {0x3C1F, 0x00, 0x00}, \
  {0x3C17, 0x00, 0x00}, \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x01, 0x00}, \
  {0x0820, 0x03, 0x00}, \
  {0x0821, 0x6C, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3929, 0x0F, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x18, 0x00}, \
  {0x0346, 0x00, 0x00}, \
  {0x0347, 0x14, 0x00}, \
  {0x0348, 0x0A, 0x00}, \
  {0x0349, 0x17, 0x00}, \
  {0x034A, 0x07, 0x00}, \
  {0x034B, 0x93, 0x00}, \
  {0x034C, 0x02, 0x00}, \
  {0x034D, 0x80, 0x00}, \
  {0x034E, 0x01, 0x00}, \
  {0x034F, 0xE0, 0x00}, \
  {0x0900, 0x01, 0x00}, \
  {0x0901, 0x44, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x07, 0x00}, \
  {0x0340, 0x02, 0x00}, \
  {0x0341, 0x20, 0x00}, \
  {0x0342, 0x0C, 0x00}, \
  {0x0343, 0x28, 0x00}, \
  {0x0200, 0x0B, 0x00}, \
  {0x0201, 0x9C, 0x00}, \
  {0x0202, 0x00, 0x00}, \
  {0x0203, 0x02, 0x00}, \
  {0x30B8, 0x2E, 0x00}, \
  {0x30BA, 0x36, 0x00}, \
}

/* Sensor Handler */
static sensor_lib_t sensor_lib_ptr =
{
  .sensor_slave_info =
  {
    .sensor_name = "s5k5e9yu05",
    .slave_addr = 0x20,
    .i2c_freq_mode = SENSOR_I2C_MODE_FAST,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .sensor_id_info =
    {
      .sensor_id_reg_addr = 0x0000,
      .sensor_id = 0x559B,
    },
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
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_CLK,
          .seq_val = CAMERA_MCLK,
          .config_val = 24000000,
          .delay = 10,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_HIGH,
          .delay = 18,
        },
      },
      .size = 6,
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
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
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
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VANA,
          .config_val = 0,
          .delay = 0,
        },
      },
      .size_down = 5,
    },
  },
  .sensor_output =
  {
    .output_format = SENSOR_BAYER,
    .connection_mode = SENSOR_MIPI_CSI,
    .raw_output = SENSOR_10_BIT_DIRECT,
    .filter_arrangement = SENSOR_Y,
  },
  .output_reg_addr =
  {
    .x_output = 0x034C,
    .y_output = 0x034E,
    .line_length_pclk = 0x0342,
    .frame_length_lines = 0x0340,
  },
  .exp_gain_info =
  {
    .coarse_int_time_addr = 0x0202,
    .global_gain_addr = 0x0204,
    .vert_offset = 8,
  },
  .aec_info =
  {
    .min_gain = 1,
    .max_gain = 16.0,
    .max_analog_gain = 16.0,
    .max_linecount = 65535 - 4,
  },
  .sensor_num_frame_skip = 1,
  .sensor_num_HDR_frame_skip = 2,
  .sensor_max_pipeline_frame_delay = 2,
  .sensor_property =
  {
    .pix_size = 1.4,
    .sensing_method = SENSOR_SMETHOD_ONE_CHIP_COLOR_AREA_SENSOR,
    .crop_factor = 5.78,
  },
  .pixel_array_size_info =
  {
    .active_array_size =
    {
      .width = 2608,
      .height = 1960,
    },
    .left_dummy = 8,
    .right_dummy = 8,
    .top_dummy = 8,
    .bottom_dummy = 8,
  },
  .color_level_info =
  {
    .white_level = 1023,
    .r_pedestal = s5k5e9yu05_DATA_PEDESTAL,
    .gr_pedestal = s5k5e9yu05_DATA_PEDESTAL,
    .gb_pedestal = s5k5e9yu05_DATA_PEDESTAL,
    .b_pedestal = s5k5e9yu05_DATA_PEDESTAL,
  },
  .sensor_stream_info_array =
  {
    .sensor_stream_info =
    {
      {
        .vc_cfg_size = 1,
        .vc_cfg =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
        },
        .pix_data_fmt =
        {
          SENSOR_BAYER,
        },
      },
    },
    .size = 1,
  },
  .start_settings =
  {
    .reg_setting_a = START_REG_ARRAY,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .data_type = CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
  .stop_settings =
  {
    .reg_setting_a = STOP_REG_ARRAY,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .data_type = CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
  .groupon_settings =
  {
    .reg_setting_a = GROUPON_REG_ARRAY,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .data_type = CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
  .groupoff_settings =
  {
    .reg_setting_a = GROUPOFF_REG_ARRAY,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .data_type = CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
  .test_pattern_info =
  {
    .test_pattern_settings =
    {
      {
        .mode = SENSOR_TEST_PATTERN_OFF,
        .settings =
        {
          .reg_setting_a =
          {
            {0x0600, 0x0000, 0x00},
          },
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_WORD_DATA,
          .delay = 0,
        }
      },
      {
        .mode = SENSOR_TEST_PATTERN_SOLID_COLOR,
        .settings =
        {
          .reg_setting_a =
          {
            {0x0601, 0x0001, 0x00},
          },
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_WORD_DATA,
          .delay = 0,
        },
      },
      {
        .mode = SENSOR_TEST_PATTERN_COLOR_BARS,
        .settings =
        {
          .reg_setting_a =
          {
            {0x0601, 0x0002, 0x00},
          },
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_WORD_DATA,
          .delay = 0,
        },
      },
      {
        .mode = SENSOR_TEST_PATTERN_COLOR_BARS_FADE_TO_GRAY,
        .settings =
        {
          .reg_setting_a =
          {
            {0x0601, 0x0003, 0x00},
          },
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_WORD_DATA,
          .delay = 0,
        },
      },
    },
    .size = 4,
    .solid_mode_addr =
    {
      .r_addr = 0x0602,
      .gr_addr = 0x0604,
      .gb_addr = 0x0608,
      .b_addr = 0x0606,
    },
  },
  .res_settings_array =
  {
    .reg_settings =
    {
      /* Res 0 */
      {
        .reg_setting_a = RES0_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 0,
      },
      /* Res 1 */
      {
        .reg_setting_a = RES1_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 0,
      },
      /* Res 2 */
      {
        .reg_setting_a = RES2_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 0,
      },
      /* Res 3 */
      {
        .reg_setting_a = RES3_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 0,
      },
    },
    .size = 4,
  },
  .out_info_array =
  {
    .out_info =
    {
      /* Res 0 */
      {
        .x_output = 2608,
        .y_output = 1960,
        .line_length_pclk = 3112,
        .frame_length_lines = 2030,
        .op_pixel_clk = 175200000,
        .binning_factor = 1,
        .min_fps = 7.5,
        .max_fps = 30.0,
        .mode = SENSOR_DEFAULT_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 1,
        .is_pdaf_supported = 0,
      },
      /* Res 1 */
      {
        .x_output = 1296,
        .y_output = 972,
        .line_length_pclk = 3112,
        .frame_length_lines = 2030,
        .op_pixel_clk = 175200000,
        .binning_factor = 2,
        .min_fps = 7.5,
        .max_fps = 30.0,
        .mode = SENSOR_DEFAULT_MODE,
        .offset_x = 8,
        .offset_y = 8,
        .scale_factor = 1,
      },
      /* Res 2 */
      {
        .x_output = 1296,
        .y_output = 972,
        .line_length_pclk = 3112,
        .frame_length_lines = 1017,
        .op_pixel_clk = 175200000,
        .binning_factor = 2,
        .min_fps = 7.5,
        .max_fps = 60.0,
        .mode = SENSOR_HFR_MODE,
        .offset_x = 8,
        .offset_y = 8,
        .scale_factor = 1,
      },
      /* Res 3 */
      {
        .x_output = 640,
        .y_output = 480,
        .line_length_pclk = 3112,
        .frame_length_lines = 544,
        .op_pixel_clk = 175200000,
        .binning_factor = 4,
        .min_fps = 7.5,
        .max_fps = 120.0,
        .mode = SENSOR_HFR_MODE,
        .offset_x = 24,
        .offset_y = 24,
        .scale_factor = 1,
      },
    },
    .size = 4,
  },
  .csi_params =
  {
    .lane_cnt = 2,
    .settle_cnt = 0xE,
    .is_csi_3phase = 0,
  },
  .csid_lut_params_array =
  {
    .lut_params =
    {
      /* Res 0 */
      {
        .num_cid = 1,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
      /* Res 1 */
      {
        .num_cid = 1,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
      /* Res 2 */
      {
        .num_cid = 1,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
      /* Res 3 */
      {
        .num_cid = 1,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
    },
    .size = 4,
  },
  .crop_params_array =
  {
    .crop_params =
    {
      /* Res 0 */
      {
        .top_crop = 0,
        .bottom_crop = 0,
        .left_crop = 0,
        .right_crop = 0,
      },
      /* Res 1 */
      {
        .top_crop = 0,
        .bottom_crop = 0,
        .left_crop = 0,
        .right_crop = 0,
      },
      /* Res 2 */
      {
        .top_crop = 0,
        .bottom_crop = 0,
        .left_crop = 0,
        .right_crop = 0,
      },
      /* Res 3 */
      {
        .top_crop = 0,
        .bottom_crop = 0,
        .left_crop = 0,
        .right_crop = 0,
      },
    },
    .size = 4,
  },

  .exposure_func_table =
  {
    .sensor_calculate_exposure = s5k5e9yu05_calculate_exposure,
    .sensor_fill_exposure_array = s5k5e9yu05_fill_exposure_array,
  },
  .sensor_capability = 0,
  .rolloff_config =
  {
    .enable = FALSE,
    .full_size_info =
    {
      .full_size_width = 0,
      .full_size_height = 0,
      .full_size_left_crop = 0,
      .full_size_top_crop = 0,
    },
  },
  .adc_readout_time = 0,
  .sensor_num_fast_aec_frame_skip = 0,
  .noise_coeff = {
    .gradient_S = 3.738032e-06,
    .offset_S = 3.651935e-04,
    .gradient_O = 4.499952e-07,
    .offset_O = -2.968624e-04,
  },
};

#endif /* __s5k5e9yu05_LIB_H__ */
