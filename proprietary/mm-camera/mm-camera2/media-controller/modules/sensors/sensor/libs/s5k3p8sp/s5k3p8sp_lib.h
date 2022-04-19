/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __s5k3p8sp_LIB_H__
#define __s5k3p8sp_LIB_H__

#include "sensor_lib.h"

#define SENSOR_MODEL "s5k3p8sp"

/* s5k3p8sp Regs */
#define s5k3p8sp_DIG_GAIN_GR_ADDR       0x020E /* 0x4000020E */
#define s5k3p8sp_DIG_GAIN_R_ADDR        0x0210 /* 0x40000210 */
#define s5k3p8sp_DIG_GAIN_B_ADDR        0x0212 /* 0x40000212 */
#define s5k3p8sp_DIG_GAIN_GB_ADDR       0x0214 /* 0x40000214 */

#define s5k3p8sp_DATA_PEDESTAL            0x40 /* 10bit */

#define s5k3p8sp_MIN_DGAIN_REG_VAL        256 /* 1.0x */
#define s5k3p8sp_MAX_DGAIN_REG_VAL        256 /* 1.0x */

/* s5k3p8sp FORMULAS */
#define s5k3p8sp_MIN_DGAIN    (s5k3p8sp_MIN_DGAIN_REG_VAL / 256)
#define s5k3p8sp_MAX_DGAIN    (s5k3p8sp_MAX_DGAIN_REG_VAL / 256)

int32_t s5k3p8sp_calculate_exposure(float real_gain,
  unsigned int line_count, sensor_exposure_info_t *exp_info, float s_real_gain);

int32_t s5k3p8sp_fill_exposure_array(unsigned int gain,
  unsigned int digital_gain, unsigned int line, unsigned int fl_lines,
  __attribute__((unused)) int luma_avg, unsigned int hdr_param,
  struct camera_i2c_reg_setting* reg_setting,
  __attribute__((unused)) unsigned int s_gain,
  __attribute__((unused)) int s_linecount,
  __attribute__((unused)) int is_hdr_enabled);

#define START_REG_ARRAY \
{ \
  {0x31A4, 0x0102, 0x0000}, \
  {0x0100, 0x0100, 0x00}, \
}

#define STOP_REG_ARRAY \
{ \
  {0x31A4, 0x0102, 0x0000}, \
  {0x0100, 0x0000, 0x00}, \
}

#define GROUPON_REG_ARRAY \
{ \
  {0x0104, 0x01, 0x00}, \
}

#define GROUPOFF_REG_ARRAY \
{ \
  {0x0104, 0x00, 0x00}, \
}

#define RES0_REG_ARRAY \
{ \
  {0x6028, 0x2000, 0x0000}, \
  {0x6214, 0x7971, 0x0000}, \
  {0x6218, 0x7150, 0x0000}, \
  {0x030E, 0x0071, 0x0000}, \
  {0x6028, 0x2000, 0x0000}, \
  {0x602A, 0x2F38, 0x0000}, \
  {0x6F12, 0x0088, 0x0000}, \
  {0x6F12, 0x0D70, 0x0000}, \
  {0x0344, 0x0018, 0x0000}, \
  {0x0348, 0x1217, 0x0000}, \
  {0x0346, 0x0018, 0x0000}, \
  {0x034A, 0x0D97, 0x0000}, \
  {0x034C, 0x1200, 0x0000}, \
  {0x034E, 0x0D80, 0x0000}, \
  {0x0342, 0x1400, 0x0000}, \
  {0x0340, 0x0E3B, 0x0000}, \
  {0x0202, 0x0200, 0x0000}, \
  {0x0200, 0x0618, 0x0000}, \
  {0x0900, 0x0011, 0x0000}, \
  {0x0380, 0x0001, 0x0000}, \
  {0x0382, 0x0001, 0x0000}, \
  {0x0384, 0x0001, 0x0000}, \
  {0x0386, 0x0001, 0x0000}, \
  {0x0400, 0x0000, 0x0000}, \
  {0x0404, 0x0010, 0x0000}, \
  {0x3604, 0x0002, 0x0000}, \
  {0x3606, 0x0103, 0x0000}, \
  {0xF496, 0x0048, 0x0000}, \
  {0xF470, 0x0020, 0x0000}, \
  {0xF43A, 0x0015, 0x0000}, \
  {0xF484, 0x0006, 0x0000}, \
  {0xF440, 0x00AF, 0x0000}, \
  {0xF442, 0x44C6, 0x0000}, \
  {0xF408, 0xFFF7, 0x0000}, \
  {0x3664, 0x0019, 0x0000}, \
  {0xF494, 0x1010, 0x0000}, \
  {0x367A, 0x0100, 0x0000}, \
  {0x362A, 0x0104, 0x0000}, \
  {0x362E, 0x0404, 0x0000}, \
  {0x32B2, 0x0008, 0x0000}, \
  {0x3286, 0x0003, 0x0000}, \
  {0x328A, 0x0005, 0x0000}, \
  {0xF47C, 0x001F, 0x0000}, \
  {0xF62E, 0x00C5, 0x0000}, \
  {0xF630, 0x00CD, 0x0000}, \
  {0xF632, 0x00DD, 0x0000}, \
  {0xF634, 0x00E5, 0x0000}, \
  {0xF636, 0x00F5, 0x0000}, \
  {0xF638, 0x00FD, 0x0000}, \
  {0xF63a, 0x010D, 0x0000}, \
  {0xF63C, 0x0115, 0x0000}, \
  {0xF63E, 0x0125, 0x0000}, \
  {0xF640, 0x012D, 0x0000}, \
  {0x6028, 0x2000, 0x0000}, \
  {0x602A, 0x1704, 0x0000}, \
  {0x6F12, 0x8010, 0x0000}, \
  {0x3070, 0x0000, 0x0000}, \
  {0x0B0E, 0x0000, 0x0000}, \
  {0x317A, 0x0130, 0x0000}, \
  {0x31C0, 0x00C8, 0x0000}, \
  {0x1006, 0x0004, 0x0000}, \
  {0x31A4, 0x0102, 0x0000}, \
}

#define RES1_REG_ARRAY \
{ \
  {0x6028, 0x2000, 0x0000}, \
  {0x6214, 0x7971, 0x0000}, \
  {0x6218, 0x7150, 0x0000}, \
  {0x030E, 0x003D, 0x0000}, \
  {0x6028, 0x2000, 0x0000}, \
  {0x602A, 0x2F38, 0x0000}, \
  {0x6F12, 0x0088, 0x0000}, \
  {0x6F12, 0x0D70, 0x0000}, \
  {0x0344, 0x0018, 0x0000}, \
  {0x0348, 0x1217, 0x0000}, \
  {0x0346, 0x0018, 0x0000}, \
  {0x034A, 0x0D97, 0x0000}, \
  {0x034C, 0x0900, 0x0000}, \
  {0x034E, 0x06C0, 0x0000}, \
  {0x0342, 0x1400, 0x0000}, \
  {0x0340, 0x0E3B, 0x0000}, \
  {0x0202, 0x0200, 0x0000}, \
  {0x0200, 0x0618, 0x0000}, \
  {0x0900, 0x0122, 0x0000}, \
  {0x0380, 0x0001, 0x0000}, \
  {0x0382, 0x0003, 0x0000}, \
  {0x0384, 0x0003, 0x0000}, \
  {0x0386, 0x0001, 0x0000}, \
  {0x0400, 0x0000, 0x0000}, \
  {0x0404, 0x0010, 0x0000}, \
  {0x3604, 0x0002, 0x0000}, \
  {0x3606, 0x0103, 0x0000}, \
  {0xF496, 0x0048, 0x0000}, \
  {0xF470, 0x0020, 0x0000}, \
  {0xF43A, 0x0015, 0x0000}, \
  {0xF484, 0x0006, 0x0000}, \
  {0xF440, 0x00AF, 0x0000}, \
  {0xF442, 0x44C6, 0x0000}, \
  {0xF408, 0xFFF7, 0x0000}, \
  {0x3664, 0x0019, 0x0000}, \
  {0xF494, 0x1010, 0x0000}, \
  {0x367A, 0x0100, 0x0000}, \
  {0x362A, 0x0104, 0x0000}, \
  {0x362E, 0x0404, 0x0000}, \
  {0x32B2, 0x0008, 0x0000}, \
  {0x3286, 0x0003, 0x0000}, \
  {0x328A, 0x0005, 0x0000}, \
  {0xF47C, 0x001F, 0x0000}, \
  {0xF62E, 0x00C5, 0x0000}, \
  {0xF630, 0x00CD, 0x0000}, \
  {0xF632, 0x00DD, 0x0000}, \
  {0xF634, 0x00E5, 0x0000}, \
  {0xF636, 0x00F5, 0x0000}, \
  {0xF638, 0x00FD, 0x0000}, \
  {0xF63a, 0x010D, 0x0000}, \
  {0xF63C, 0x0115, 0x0000}, \
  {0xF63E, 0x0125, 0x0000}, \
  {0xF640, 0x012D, 0x0000}, \
  {0x6028, 0x2000, 0x0000}, \
  {0x602A, 0x1704, 0x0000}, \
  {0x6F12, 0x8011, 0x0000}, \
  {0x3070, 0x0000, 0x0000}, \
  {0x0B0E, 0x0000, 0x0000}, \
  {0x317A, 0x0007, 0x0000}, \
  {0x31C0, 0x00C8, 0x0000}, \
  {0x1006, 0x0004, 0x0000}, \
  {0x31A4, 0x0102, 0x0000}, \
}

/* Sensor Handler */
static sensor_lib_t sensor_lib_ptr =
{
  .sensor_slave_info =
  {
    .sensor_name = "s5k3p8sp",
    .slave_addr = 0x20,
    .i2c_freq_mode = SENSOR_I2C_MODE_FAST,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .sensor_id_info =
    {
      .sensor_id_reg_addr = 0x0000,
      .sensor_id = 0x3108,
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
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_STANDBY,
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
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VDIG,
          .config_val = 0,
          .delay = 5,
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
          .delay = 5,
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
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VDIG,
          .config_val = 0,
          .delay = 5,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 5,
        },
       {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VANA,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
      },
      .size_down = 6,
    },
  },
  .sensor_output =
  {
    .output_format = SENSOR_BAYER,
    .connection_mode = SENSOR_MIPI_CSI,
    .raw_output = SENSOR_10_BIT_DIRECT,
    .filter_arrangement = SENSOR_GRBG,
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
    .vert_offset = 4,
  },
  .aec_info =
  {
    .min_gain = 1,
    .max_gain = 16.0,
    .max_analog_gain = 16.0,
    .max_linecount = 65535 - 4,
  },
  .sensor_num_frame_skip = 2,
  .sensor_num_HDR_frame_skip = 2,
  .sensor_max_pipeline_frame_delay = 2,
  .sensor_property =
  {
    .pix_size = 1.12,
    .sensing_method = SENSOR_SMETHOD_ONE_CHIP_COLOR_AREA_SENSOR,
    .crop_factor = 1.33,
  },
  .pixel_array_size_info =
  {
    .active_array_size =
    {
      .width = 4608,
      .height = 3456,
    },
    .left_dummy = 8,
    .right_dummy = 8,
    .top_dummy = 8,
    .bottom_dummy = 8,
  },
  .color_level_info =
  {
    .white_level = 1023,
    .r_pedestal = s5k3p8sp_DATA_PEDESTAL,
    .gr_pedestal = s5k3p8sp_DATA_PEDESTAL,
    .gb_pedestal = s5k3p8sp_DATA_PEDESTAL,
    .b_pedestal = s5k3p8sp_DATA_PEDESTAL,
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
    .data_type = CAMERA_I2C_WORD_DATA,
    .delay = 0,
  },
  .stop_settings =
  {
    .reg_setting_a = STOP_REG_ARRAY,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .data_type = CAMERA_I2C_WORD_DATA,
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
            {0x6028, 0x4000, 0x00},
            {0x602A, 0x0600, 0x00},
            {0x6F12, 0x0000, 0x00},
          },
          .size = 3,
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
            {0x6028, 0x4000, 0x0000},
            {0x602A, 0x0600, 0x0000},
            {0x6F12, 0x0001, 0x0000},
          },
          .size = 3,
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
            {0x6028, 0x4000, 0x0000},
            {0x602A, 0x0600, 0x0000},
            {0x6F12, 0x0002, 0x0000},
          },
          .size = 3,
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
            {0x6028, 0x4000, 0x0000},
            {0x602A, 0x0600, 0x0000},
            {0x6F12, 0x0003, 0x0000},
          },
          .size = 3,
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_WORD_DATA,
          .delay = 0,
        },
      },
      {
        .mode = SENSOR_TEST_PATTERN_PN9,
        .settings =
        {
          .reg_setting_a =
          {
            {0x6028, 0x4000, 0x0000},
            {0x602A, 0x0600, 0x0000},
            {0x6F12, 0x0004, 0x0000},
          },
          .size = 3,
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_WORD_DATA,
          .delay = 0,
        },
      },
      {
        .mode = SENSOR_TEST_PATTERN_CUSTOM1, /* Macbeth */
        .settings =
        {
          .reg_setting_a =
          {
            {0x6028, 0x4000, 0x0000},
            {0x602A, 0x0600, 0x0000},
            {0x6F12, 0x0100, 0x0000},
          },
          .size = 3,
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_WORD_DATA,
          .delay = 0,
        },
      },
    },
    .size = 6,
    .solid_mode_addr =
    {
      .r_addr = 0x0602,
      .gr_addr = 0x0604,
      .gb_addr = 0x0606,
      .b_addr = 0x0608,
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
        .data_type = CAMERA_I2C_WORD_DATA,
        .delay = 0,
      },
      /* Res 1 */
      {
        .reg_setting_a = RES1_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_WORD_DATA,
        .delay = 0,
      },
    },
    .size = 2,
  },
  .out_info_array =
  {
    .out_info =
    {
      /* Res 0 */
      {
        .x_output = 4608,
        .y_output = 3456,
        .line_length_pclk = 5120,
        .frame_length_lines = 3643,
        .vt_pixel_clk = 560000000,
        .op_pixel_clk = 542400000,
        .binning_factor = 1,
        .binning_method = 0,
        .min_fps = 7.5,
        .max_fps = 30.0,
        .mode = SENSOR_QUADRA_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0,
        .is_pdaf_supported = 0,
      },
      /* Res 1 */
      {
        .x_output = 2304,
        .y_output = 1728,
        .line_length_pclk = 5120,
        .frame_length_lines = 3643,
        .vt_pixel_clk = 560000000,
        .op_pixel_clk = 292800000,
        .binning_factor = 2,
        .binning_method = 1,
        .min_fps = 7.5,
        .max_fps = 30.0,
        .mode = SENSOR_DEFAULT_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0,
      },
    },
    .size = 2,
  },
  .csi_params =
  {
    .lane_cnt = 4,
    .settle_cnt = 0x18,
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
    },
    .size = 2,
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
    },
    .size = 2,
  },

  .exposure_func_table =
  {
    .sensor_calculate_exposure = s5k3p8sp_calculate_exposure,
    .sensor_fill_exposure_array = s5k3p8sp_fill_exposure_array,
  },
  .sensor_capability = SENSOR_FEATURE_QUADRA_CFA,
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

#endif /* __s5k3p8sp_LIB_H__ */
