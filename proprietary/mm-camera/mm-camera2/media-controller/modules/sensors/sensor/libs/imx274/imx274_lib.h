/* imx274_lib.h
 *
 * DESCRIPTION
 *
 * Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __IMX274_LIB_H__
#define __IMX274_LIB_H__

#include "sensor_lib.h"
#define SENSOR_MODEL "imx274"

#define RHS1_DOL_MODE   (618)
#define SHR_DOL1_MIN    (6)
#define SHR_DOL2_MIN    (RHS1_DOL_MODE + 6)
#define VMAX_DOL_MODE   (2852)

#define REAL_A_GAIN_MAX      22.5
#define REAL_D_GAIN_MAX      7.94
#define REAL_GAIN_SATURATION 1.00
#define REAL_GAIN_PROTECTION 1.00
#define REGISTER_A_GAIN_MAX  0x7A5
#define REGISTER_A_GAIN_MIN  0
#define REG_DIGITAL_GAIN  0x3012

#define IMX317_MAX_DGAIN_DECIMATOR  2048


#define START_REG_ARRAY \
{ \
  {0x3000, 0x00, 0x00}, \
  {0x303E, 0x02, 0x0e}, \
  {0x30F4, 0x00, 0x00}, \
  {0x3018, 0xA2, 0x00}, \
}


#define STOP_REG_ARRAY \
{ \
  {0x3000, 0x01, 0x00}, \
  {0x30F4, 0x01, 0x00}, \
  {0x3018, 0xA3, 0x00}, \
}

#define GROUPON_REG_ARRAY \
{ \
   {0x302d, 0x01, 0x00}, \
}

#define GROUPOFF_REG_ARRAY \
{ \
  {0x302d, 0x00, 0x00},  \
}

#define INIT0_REG_ARRAY \
{ \
  {0x3000, 0x12, 0x00}, \
  {0x3120, 0xF0, 0x00}, \
  {0x3121, 0x00, 0x00}, \
  {0x3122, 0x02, 0x00}, \
  {0x3129, 0x9c, 0x00}, \
  {0x312A, 0x02, 0x00}, \
  {0x312D, 0x02, 0x00}, \
  {0x310B, 0x00, 0x00}, \
  {0x3045, 0x40, 0x00}, \
  {0x304C, 0x00, 0x00}, \
  {0x304D, 0x03, 0x00}, \
  {0x30EE, 0x01, 0x00}, \
  {0x30F6, 0xED, 0x00}, \
  {0x30F7, 0x01, 0x00}, \
  {0x30F8, 0x08, 0x00}, \
  {0x30F9, 0x13, 0x00}, \
  {0x30FA, 0x00, 0x00}, \
  {0x3130, 0xAA, 0x00}, \
  {0x3131, 0x08, 0x00}, \
  {0x3132, 0x9A, 0x00}, \
  {0x3133, 0x08, 0x00}, \
  {0x3304, 0x0A, 0x00}, \
  {0x3305, 0x00, 0x00}, \
  {0x3306, 0x0A, 0x00}, \
  {0x3307, 0x00, 0x00}, \
  {0x331C, 0x1A, 0x00}, \
  {0x331D, 0x00, 0x00}, \
  {0x3502, 0x02, 0x00}, \
  {0x3529, 0x0E, 0x00}, \
  {0x352A, 0x0E, 0x00}, \
  {0x352B, 0x0E, 0x00}, \
  {0x3538, 0x0E, 0x00}, \
  {0x3539, 0x0E, 0x00}, \
  {0x3553, 0x00, 0x00}, \
  {0x357D, 0x05, 0x00}, \
  {0x357F, 0x05, 0x00}, \
  {0x3581, 0x04, 0x00}, \
  {0x3583, 0x76, 0x00}, \
  {0x3587, 0x01, 0x00}, \
  {0x3590, 0x0A, 0x00}, \
  {0x3591, 0x00, 0x00}, \
  {0x35BB, 0x0E, 0x00}, \
  {0x35BC, 0x0E, 0x00}, \
  {0x35BD, 0x0E, 0x00}, \
  {0x35BE, 0x0E, 0x00}, \
  {0x35BF, 0x0E, 0x00}, \
  {0x366E, 0x00, 0x00}, \
  {0x366F, 0x00, 0x00}, \
  {0x3670, 0x00, 0x00}, \
  {0x3671, 0x00, 0x00}, \
  {0x3686, 0x0A, 0x00}, \
  {0x3687, 0x00, 0x00}, \
  {0x3A41, 0x10, 0x00}, \
}

#define RES0_REG_ARRAY \
{ \
  {0x3004, 0x00, 0x00}, \
  /* mode ralated*/ \
  {0x3005, 0x07, 0x00}, \
  {0x3006, 0x00, 0x00}, \
  {0x3007, 0x02, 0x00}, \
  {0x300C, 0x0C, 0x00}, \
  {0x300D, 0x00, 0x00}, \
  {0x301A, 0x00, 0x00}, \
  {0x3045, 0x40, 0x00}, \
  {0x30E2, 0x00, 0x00}, \
  {0x30F6, 0xED, 0x00}, \
  {0x30F7, 0x01, 0x00}, \
  {0x30F8, 0x08, 0x00}, \
  {0x30F9, 0x13, 0x00}, \
  {0x30FA, 0x00, 0x00}, \
  {0x30EE, 0x01, 0x00}, \
  {0x3130, 0xAA, 0x00}, \
  {0x3131, 0x08, 0x00}, \
  {0x3132, 0x9A, 0x00}, \
  {0x3133, 0x08, 0x00}, \
  {0x3342, 0xFF, 0x00}, \
  {0x3343, 0x01, 0x00}, \
  {0x3344, 0xFF, 0x00}, \
  {0x3345, 0x01, 0x00}, \
  {0x3528, 0x0F, 0x00}, \
  {0x3554, 0x00, 0x00}, \
  {0x3555, 0x00, 0x00}, \
  {0x3556, 0x00, 0x00}, \
  {0x3557, 0x00, 0x00}, \
  {0x3558, 0x00, 0x00}, \
  {0x3559, 0x1F, 0x00}, \
  {0x355A, 0x1F, 0x00}, \
  {0x35BA, 0x0F, 0x00}, \
  {0x366A, 0x00, 0x00}, \
  {0x366B, 0x00, 0x00}, \
  {0x366C, 0x00, 0x00}, \
  {0x366D, 0x00, 0x00}, \
  /* DOL settings */ \
  {0x3019, 0x00, 0x00}, \
  {0x302E, 0x04, 0x00}, \
  {0x302F, 0x00, 0x00}, \
  {0x3030, 0x43, 0x00}, \
  {0x3031, 0x05, 0x00}, \
  {0x3032, 0x08, 0x00}, \
  {0x3033, 0x00, 0x00}, \
  {0x3041, 0x30, 0x00}, \
  {0x3042, 0x04, 0x00}, \
  {0x3043, 0x01, 0x00}, \
  {0x30E9, 0x00, 0x00}, \
  {0x3A41, 0x10, 0x14}, \
}

#define RES1_REG_ARRAY \
{\
  {0x30E2, 0x01, 0x00}, \
  {0x30F6, 0x1C, 0x00}, \
  {0x30F7, 0x04, 0x00}, \
  {0x30F8, 0x24, 0x00}, \
  {0x30F9, 0x0B, 0x00}, \
  {0x30FA, 0x00, 0x00}, \
  {0x3130, 0x86, 0x00}, \
  {0x3131, 0x08, 0x00}, \
  {0x3132, 0x8E, 0x00}, \
  {0x3133, 0x08, 0x00}, \
  {0x3004, 0x06, 0x00}, \
  {0x3005, 0x01, 0x00}, \
  {0x3006, 0x00, 0x00}, \
  {0x3007, 0x02, 0x00}, \
  {0x3A41, 0x08, 0x00}, \
  {0x3342, 0x0A, 0x00}, \
  {0x3343, 0x00, 0x00}, \
  {0x3344, 0x16, 0x00}, \
  {0x3345, 0x00, 0x00}, \
  {0x3528, 0x0E, 0x00}, \
  {0x3554, 0x1F, 0x00}, \
  {0x3555, 0x01, 0x00}, \
  {0x3556, 0x01, 0x00}, \
  {0x3557, 0x01, 0x00}, \
  {0x3558, 0x01, 0x00}, \
  {0x3559, 0x00, 0x00}, \
  {0x355A, 0x00, 0x00}, \
  {0x35BA, 0x0E, 0x00}, \
  {0x366A, 0x1B, 0x00}, \
  {0x366B, 0x1A, 0x00}, \
  {0x366C, 0x19, 0x00}, \
  {0x366D, 0x17, 0x00}, \
  {0x33A6, 0x01, 0x00}, \
  {0x306B, 0x05, 0x00}, \
  /* DOL settings */ \
  {0x3019, 0x01, 0x00}, \
  {0x302E, 0x06, 0x00}, \
  {0x302F, 0x00, 0x00}, \
  {0x3030, 0x10, 0x00}, \
  {0x3031, 0x00, 0x00}, \
  {0x3032, 0x6A, 0x00}, \
  {0x3033, 0x02, 0x00}, \
  {0x3041, 0x31, 0x00}, \
  {0x3042, 0x07, 0x00}, \
  {0x3043, 0x01, 0x00}, \
  {0x30E9, 0x01, 0x00}, \
}

static sensor_lib_t sensor_lib_ptr =
{
  .sensor_slave_info =
  {
    .sensor_name = SENSOR_MODEL,
    .slave_addr = 0x34,
    .i2c_freq_mode = SENSOR_I2C_MODE_FAST,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .sensor_id_info =
    {
      .sensor_id_reg_addr = 0x3120,
      .sensor_id = 0x8000,
    },
    .power_setting_array =
    {
      .power_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VDIG,
          .config_val = GPIO_OUT_HIGH,
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
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_V_CUSTOM1,
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
          .delay = 10,
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
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_V_CUSTOM1,
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
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VANA,
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
    .raw_output = SENSOR_12_BIT_DIRECT,
    .filter_arrangement = SENSOR_RGGB,
  },
  .output_reg_addr =
  {
    .frame_length_lines = 0x30F8,
  },
  .exp_gain_info =
  {
    .coarse_int_time_addr = 0x300C,
    .global_gain_addr = 0x300A,
    .vert_offset = 12,
  },
  .aec_info =
  {
    .min_gain = REAL_GAIN_PROTECTION,
    .max_gain = REAL_A_GAIN_MAX * REAL_D_GAIN_MAX,
    .max_analog_gain = REAL_A_GAIN_MAX,
    .max_linecount = 4860,
    .svhdr_use_separate_gain = 1,
    .svhdr_use_separate_limits = 1,
    .min_line_cnt = {2, 4},
    .max_line_cnt = {612, 2234},
  },
  .sensor_num_frame_skip = 2,
  .sensor_num_HDR_frame_skip = 0,
  .sensor_max_pipeline_frame_delay = 2,
  .pixel_array_size_info =
  {
    .active_array_size =
    {
      .width = 3840,
      .height = 2160,
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
        .vc_cfg_size = 4,
        .vc_cfg =
        {
          {
            .cid = 0,
            .dt = CSI_RAW12,
            .decode_format = CSI_DECODE_12BIT
          },
          {
            .cid = 1,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 2,
            .dt = CSI_EMBED_DATA,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 3,
            .dt = 0x37,
            .decode_format = CSI_DECODE_10BIT
          },
        },
        .pix_data_fmt =
        {
          SENSOR_BAYER,
          SENSOR_BAYER_SVHDR_SUBFRAME,
          SENSOR_META,
          SENSOR_META,
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
             {0x5040, 0x00, 0x00},
             {0x8008, 0x00, 0x00},
           },
          .size = 2,
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_BYTE_DATA,
          .delay = 0,
        },
      },
      {
        .mode = SENSOR_TEST_PATTERN_SOLID_COLOR,
        .settings =
        {
          .reg_setting_a = {{0x8008, 0x01, 0x00}},
          .size = 1,
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_BYTE_DATA,
          .delay = 0,
        },
      },
      {
        .mode = SENSOR_TEST_PATTERN_COLOR_BARS,
        .settings =
        {
          .reg_setting_a = {{0x5040, 0x80, 0x00}},
          .size = 1,
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_BYTE_DATA,
          .delay = 0,
        },
      },
      {
        .mode = SENSOR_TEST_PATTERN_PN9,
        .settings =
        {
          .reg_setting_a = {{0x8008, 0x02, 0x00}},
          .size = 1,
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_BYTE_DATA,
          .delay = 0,
        },
      },
    },
    .size = 0,
    .solid_mode_addr =
    {
      .r_addr = 0x8006,
      .gr_addr = 0x8004,
      .gb_addr = 0x8002,
      .b_addr = 0x8000,
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
        .delay = 15,
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
        .delay = 10,
      },
      /* Res 1 */
      {
        .reg_setting_a = RES1_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 10,
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
        .x_output = 3864,
        .y_output = 2202,
        .line_length_pclk = 493,
        .frame_length_lines = 4872,
        .vt_pixel_clk = 72056880,
        .op_pixel_clk = 460000000,
        .binning_factor = 1,
        .min_fps = 15.0,
        .max_fps = 30.0,
        .mode = SENSOR_DEFAULT_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0,
      },
      /* Res 1 */
      {
        .x_output = 3840,
        .y_output = 2160,
        .line_length_pclk = 1052,
        .frame_length_lines = VMAX_DOL_MODE,
        .vt_pixel_clk = 72007296,
        .op_pixel_clk = 460000000,
        .binning_factor = 1,
        .min_fps = 7.0,
        .max_fps = 24.0,
        .mode = SENSOR_RAW_HDR_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0,
        .custom_format =
        {
          .enable = 1,
          .subframes_cnt = 2,
          .start_x = 16,
          .start_y = 18,
          .width = 3872,
          .height = (2186 + RHS1_DOL_MODE),
        },
      },
    },
    .size = 2,
  },
  .csi_params =
  {
    .lane_cnt = 4,
    .settle_cnt = 0x10,
    .is_csi_3phase = 0,
  },
  .csid_lut_params_array =
  {
    .lut_params =
      {
        {
          .num_cid = 3,
            .vc_cfg_a =
            {
              {
                .cid = 0,
                .dt = CSI_RAW12,
                .decode_format = CSI_DECODE_12BIT
              },
              {
                .cid = 2,
                .dt = 0x12,
                .decode_format = CSI_DECODE_10BIT
              },
              {
                .cid = 3,
                .dt = 0x37,
                .decode_format = CSI_DECODE_10BIT
              },
            },
         },
         /* Res 1 */
         {
           .num_cid = 3,
           .vc_cfg_a =
             {
               {
                 .cid = 1,
                 .dt = CSI_RAW10,
                 .decode_format = CSI_DECODE_10BIT
               },
               {
                 .cid = 2,
                 .dt = 0x12,
                 .decode_format = CSI_DECODE_10BIT
               },
               {
                 .cid = 3,
                 .dt = 0x37,
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
        .top_crop = 24,
        .bottom_crop = 18,
        .left_crop = 12,
        .right_crop = 12,
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
    .sensor_calculate_exposure = sensor_calculate_exposure,
    .sensor_fill_exposure_array = sensor_fill_exposure_array,
  },
  .meta_data_out_info_array =
  {
    .meta_data_out_info =
    {
      {
        .width = 4208,
        .height = 0,
        .stats_type = CSI_DECODE_10BIT,
        .dt = CSI_EMBED_DATA,
      },
    },
    .size = 0,
  },
  .sensor_capability = (SENSOR_STAGGERED_VIDEO_HDR_FLAG),
  .awb_func_table =
  {
    .sensor_fill_awb_array = 0,
    .awb_table_size = 0,
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
