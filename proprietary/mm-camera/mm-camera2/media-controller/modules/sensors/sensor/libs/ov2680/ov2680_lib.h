/* ov2680_lib.h
 *
 *Copyright (c) 2016 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __OV2680_LIB_H__
#define __OV2680_LIB_H__

#include "sensor_lib.h"
#define SENSOR_MODEL "ov2680"

#define MIN_GAIN 1
#define MAX_ANALOG_GAIN (16)
#define MAX_DIGITAL_GAIN (1)
#define MAX_GAIN (MAX_ANALOG_GAIN * MAX_DIGITAL_GAIN)

#define OV2680_MAX_DGAIN_DECIMATOR  2048

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
    {0x3208, 0x00, 0x00}, \
}

#define GROUPOFF_REG_ARRAY \
{ \
    {0x3208, 0x10, 0x00}, \
    {0x3208, 0xa0, 0x00}, \
}

#define INIT0_REG_ARRAY \
{ \
    {0x0103, 0x01, 200},\
    {0x3002, 0x00, 0x00},\
    {0x3016, 0x1c, 0x00},\
    {0x3018, 0x44, 0x00},\
    {0x3020, 0x00, 0x00},\
    {0x3080, 0x02, 0x00},\
    {0x3082, 0x37, 0x00},\
    {0x3084, 0x09, 0x00},\
    {0x3085, 0x04, 0x00},\
    {0x3086, 0x00, 0x00},\
    {0x3501, 0x26, 0x00},\
    {0x3502, 0x40, 0x00},\
    {0x3503, 0x03, 0x00},\
    {0x350b, 0x36, 0x00},\
    {0x3600, 0xb4, 0x00},\
    {0x3603, 0x35, 0x00},\
    {0x3604, 0x24, 0x00},\
    {0x3605, 0x00, 0x00},\
    {0x3620, 0x26, 0x00},\
    {0x3621, 0x37, 0x00},\
    {0x3622, 0x04, 0x00},\
    {0x3628, 0x00, 0x00},\
    {0x3701, 0x64, 0x00},\
    {0x3705, 0x3c, 0x00},\
    {0x370c, 0x50, 0x00},\
    {0x370d, 0xc0, 0x00},\
    {0x3718, 0x88, 0x00},\
    {0x3720, 0x00, 0x00},\
    {0x3721, 0x00, 0x00},\
    {0x3722, 0x00, 0x00},\
    {0x3723, 0x00, 0x00},\
    {0x3738, 0x00, 0x00},\
    {0x370a, 0x23, 0x00},\
    {0x3717, 0x58, 0x00},\
    {0x3781, 0x80, 0x00},\
    {0x3784, 0x0c, 0x00},\
    {0x3789, 0x60, 0x00},\
    {0x3800, 0x00, 0x00},\
    {0x3801, 0x00, 0x00},\
    {0x3802, 0x00, 0x00},\
    {0x3803, 0x00, 0x00},\
    {0x3804, 0x06, 0x00},\
    {0x3805, 0x4f, 0x00},\
    {0x3806, 0x04, 0x00},\
    {0x3807, 0xbf, 0x00},\
    {0x3808, 0x03, 0x00},\
    {0x3809, 0x20, 0x00},\
    {0x380a, 0x02, 0x00},\
    {0x380b, 0x58, 0x00},\
    {0x380c, 0x06, 0x00},\
    {0x380d, 0xac, 0x00},\
    {0x380e, 0x02, 0x00},\
    {0x380f, 0x84, 0x00},\
    {0x3810, 0x00, 0x00},\
    {0x3811, 0x04, 0x00},\
    {0x3812, 0x00, 0x00},\
    {0x3813, 0x04, 0x00},\
    {0x3814, 0x31, 0x00},\
    {0x3815, 0x31, 0x00},\
    {0x3819, 0x04, 0x00},\
    {0x3820, 0xc2, 0x00},\
    {0x3821, 0x01, 0x00},\
    {0x4000, 0x81, 0x00},\
    {0x4001, 0x40, 0x00},\
    {0x4008, 0x00, 0x00},\
    {0x4009, 0x03, 0x00},\
    {0x4602, 0x02, 0x00},\
    {0x481f, 0x36, 0x00},\
    {0x4825, 0x36, 0x00},\
    {0x4837, 0x18, 0x00},\
    {0x5002, 0x30, 0x00},\
    {0x5080, 0x00, 0x00},\
    {0x5081, 0x41, 0x00},\
    {0x5780, 0x3e, 0x00},\
    {0x5781, 0x0f, 0x00},\
    {0x5782, 0x04, 0x00},\
    {0x5783, 0x02, 0x00},\
    {0x5784, 0x01, 0x00},\
    {0x5785, 0x01, 0x00},\
    {0x5786, 0x00, 0x00},\
    {0x5787, 0x04, 0x00},\
    {0x5788, 0x02, 0x00},\
    {0x5789, 0x00, 0x00},\
    {0x578a, 0x01, 0x00},\
    {0x578b, 0x02, 0x00},\
    {0x578c, 0x03, 0x00},\
    {0x578d, 0x03, 0x00},\
    {0x578e, 0x08, 0x00},\
    {0x578f, 0x0c, 0x00},\
    {0x5790, 0x08, 0x00},\
    {0x5791, 0x04, 0x00},\
    {0x5792, 0x00, 0x00},\
    {0x5793, 0x00, 0x00},\
    {0x5794, 0x03, 0x00},\
}

#define RES0_REG_ARRAY \
{ \
    {0x3501, 0x4e, 0x00},\
    {0x3502, 0xe0, 0x00},\
    {0x3620, 0x24, 0x00},\
    {0x3622, 0x03, 0x00},\
    {0x3718, 0x80, 0x00},\
    {0x3721, 0x09, 0x00},\
    {0x3722, 0x06, 0x00},\
    {0x3723, 0x59, 0x00},\
    {0x3738, 0x99, 0x00},\
    {0x370a, 0x21, 0x00},\
    {0x3808, 0x06, 0x00},\
    {0x3809, 0x40, 0x00},\
    {0x380a, 0x04, 0x00},\
    {0x380b, 0xb0, 0x00},\
    {0x380c, 0x06, 0x00},\
    {0x380d, 0xa4, 0x00},\
    {0x380e, 0x05, 0x00},\
    {0x380f, 0x0e, 0x00},\
    {0x3811, 0x08, 0x00},\
    {0x3813, 0x08, 0x00},\
    {0x3814, 0x11, 0x00},\
    {0x3815, 0x11, 0x00},\
    {0x3820, 0xc0, 0x00},\
    {0x3821, 0x00, 0x00},\
    {0x4008, 0x02, 0x00},\
    {0x4009, 0x09, 0x00},\
}

#define RES1_REG_ARRAY \
{ \
    {0x3501, 0x26, 0x00},\
    {0x3502, 0x40, 0x00},\
    {0x3620, 0x26, 0x00},\
    {0x3622, 0x04, 0x00},\
    {0x3718, 0x88, 0x00},\
    {0x3721, 0x00, 0x00},\
    {0x3722, 0x00, 0x00},\
    {0x3723, 0x00, 0x00},\
    {0x3738, 0x00, 0x00},\
    {0x370a, 0x23, 0x00},\
    {0x3808, 0x03, 0x00},\
    {0x3809, 0x20, 0x00},\
    {0x380a, 0x02, 0x00},\
    {0x380b, 0x58, 0x00},\
    {0x380c, 0x06, 0x00},\
    {0x380d, 0xac, 0x00},\
    {0x380e, 0x05, 0x00},\
    {0x380f, 0x08, 0x00},\
    {0x3811, 0x04, 0x00},\
    {0x3813, 0x04, 0x00},\
    {0x3814, 0x31, 0x00},\
    {0x3815, 0x31, 0x00},\
    {0x3820, 0xc2, 0x00},\
    {0x3821, 0x01, 0x00},\
    {0x4008, 0x00, 0x00},\
    {0x4009, 0x03, 0x00},\
}
#define RES2_REG_ARRAY \
{ \
    {0x3501, 0x26, 0x00},\
    {0x3502, 0x40, 0x00},\
    {0x3620, 0x26, 0x00},\
    {0x3622, 0x04, 0x00},\
    {0x3718, 0x88, 0x00},\
    {0x3721, 0x00, 0x00},\
    {0x3722, 0x00, 0x00},\
    {0x3723, 0x00, 0x00},\
    {0x3738, 0x00, 0x00},\
    {0x370a, 0x23, 0x00},\
    {0x3808, 0x03, 0x00},\
    {0x3809, 0x20, 0x00},\
    {0x380a, 0x02, 0x00},\
    {0x380b, 0x58, 0x00},\
    {0x380c, 0x06, 0x00},\
    {0x380d, 0xac, 0x00},\
    {0x380e, 0x02, 0x00},\
    {0x380f, 0x84, 0x00},\
    {0x3811, 0x04, 0x00},\
    {0x3813, 0x04, 0x00},\
    {0x3814, 0x31, 0x00},\
    {0x3815, 0x31, 0x00},\
    {0x3820, 0xc2, 0x00},\
    {0x3821, 0x01, 0x00},\
    {0x4008, 0x00, 0x00},\
    {0x4009, 0x03, 0x00},\
}

static sensor_lib_t sensor_lib_ptr =
{
  .sensor_slave_info =
  {
    .sensor_name = SENSOR_MODEL,
    .slave_addr = 0x6C,
    .i2c_freq_mode = SENSOR_I2C_MODE_FAST,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .sensor_id_info =
    {
      .sensor_id_reg_addr = 0x300a,
      .sensor_id = 0x2680,
    },
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
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VANA,
          .config_val = GPIO_OUT_HIGH,
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
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VANA,
          .config_val = GPIO_OUT_LOW,
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
  },
  .sensor_output =
  {
    .output_format = SENSOR_BAYER,
    .connection_mode = SENSOR_MIPI_CSI,
    .raw_output = SENSOR_10_BIT_DIRECT,
    .filter_arrangement = SENSOR_BGGR,
  },
  .output_reg_addr =
  {
    .x_output = 0x3808,
    .y_output = 0x380a,
    .line_length_pclk = 0x380c,
    .frame_length_lines = 0x380e,
  },
  .exp_gain_info =
  {
    .coarse_int_time_addr = 0x3500,
    .global_gain_addr = 0x350a,
    .vert_offset = 4,
  },
  .aec_info =
  {
    .min_gain = 1.0,
    .max_gain = MAX_GAIN,
    .max_analog_gain = MAX_ANALOG_GAIN,
    .max_linecount = 26880,
  },
  .sensor_num_frame_skip = 2,
  .sensor_num_HDR_frame_skip = 2,
  .sensor_max_pipeline_frame_delay = 2,
  .sensor_property =
  {
    .pix_size = 1.75,
    .sensing_method = SENSOR_SMETHOD_ONE_CHIP_COLOR_AREA_SENSOR,
    .crop_factor = 1.0,
  },
  .pixel_array_size_info =
  {
    .active_array_size =
    {
      .width = 1616,
      .height = 1216,
    },
    .left_dummy = 0,
    .right_dummy = 0,
    .top_dummy = 0,
    .bottom_dummy = 0,
  },
  .color_level_info =
  {
    .white_level = 1023,
    .r_pedestal = 64,
    .gr_pedestal = 64,
    .gb_pedestal = 64,
    .b_pedestal = 64,
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
             {0x5080, 0x00, 0x00},
           },
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_BYTE_DATA,
          .delay = 0,
        },
      },
      {
        .mode = SENSOR_TEST_PATTERN_COLOR_BARS_FADE_TO_GRAY,
        .settings =
        {
          .reg_setting_a =
           {
              {0x5080, 0x88, 0x00},
           },
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_BYTE_DATA,
          .delay = 0,
        },
      },
      {
        .mode = SENSOR_TEST_PATTERN_COLOR_BARS,
        .settings =
        {
          .reg_setting_a =
           {
             {0x5080, 0x80, 0x00},
           },
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_BYTE_DATA,
          .delay = 0,
        },
      },
    },
    .size = 3,
    .solid_mode_addr =
    {
      .r_addr = 0x4327,
      .gr_addr = 0x4329,
      .gb_addr = 0x4327,
      .b_addr = 0x4323,
    },
  },
  .init_settings_array =
  {
    .reg_settings =
    {
      {
        .reg_setting_a = INIT0_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 60,
      },
    },
    .size = 1,
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
    },
    .size = 3,
  },
  .out_info_array =
  {
    .out_info =
    {
      /* Res 0 */
      {
        .x_output = 1600,
        .y_output = 1200,
        .line_length_pclk = 1700,
        .frame_length_lines = 1294,
        .op_pixel_clk = 66000000,
        .binning_factor = 1,
        .min_fps = 7.5,
        .max_fps = 30.0,
        .mode = SENSOR_DEFAULT_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0,
      },
      /* Res 1 */
      {
        .x_output = 800,
        .y_output = 600,
        .line_length_pclk = 1700,
        .frame_length_lines = 1294,
        .op_pixel_clk = 66000000,
        .binning_factor = 1,
        .min_fps = 7.5,
        .max_fps = 30.0,
        .mode = SENSOR_DEFAULT_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0,
      },
      /* Res 2 */
      {
        .x_output = 800,
        .y_output = 600,
        .line_length_pclk = 1700,
        .frame_length_lines = 644,
        .op_pixel_clk = 66000000,
        .binning_factor = 1,
        .min_fps = 60.0,
        .max_fps = 60.0,
        .mode = SENSOR_HFR_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0,
      },

    },
    .size = 3,
  },
  .csi_params =
  {
    .lane_cnt = 1,
    .settle_cnt = 0x12,
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

    },
    .size = 3,
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

    },
    .size = 3,
  },
  .exposure_func_table =
  {
    .sensor_calculate_exposure = sensor_calculate_exposure,
    .sensor_fill_exposure_array = sensor_fill_exposure_array,
  },
  .meta_data_out_info_array =
  {
    .meta_data_out_info =
    {
      {
        .width = 0,
        .height = 0,
        .stats_type = 0,
      },
    },
    .size = 0,
  },
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
  .noise_coeff = {
    .gradient_S = 3.738032e-06,
    .offset_S = 3.651935e-04,
    .gradient_O = 4.499952e-07,
    .offset_O = -2.968624e-04,
  },
};

#endif
