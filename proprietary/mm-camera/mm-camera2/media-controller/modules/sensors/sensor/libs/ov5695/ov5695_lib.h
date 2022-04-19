/* ov5695_lib.h
 *
 *Copyright (c) 2016 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ov5695_LIB_H__
#define __ov5695_LIB_H__

#include "sensor_lib.h"
#define SENSOR_MODEL "ov5695"

/* ov5695 Regs */
#define ov5695_DIG_GAIN               0x5502

/* ov5695 CONSTANTS */
#define ov5695_MAX_INTEGRATION_MARGIN  10

#define ov5695_DATA_PEDESTAL           0x10 /* 10bit value */

#define ov5695_MIN_AGAIN_REG_VAL       16 /* 1.0x */
#define ov5695_MAX_AGAIN_REG_VAL       248 /* 15.5x */

#define ov5695_MIN_DGAIN_REG_VAL       256 /* 1.0x */
#define ov5695_MAX_DGAIN_REG_VAL       256 /* 1.0x */

#define ov5695_MAX_DGAIN_DECIMATOR     256

/* ov5695 FORMULAS */
#define ov5695_MIN_AGAIN    1.0
#define ov5695_MAX_AGAIN    15.5

#define ov5695_MIN_DGAIN    (ov5695_MIN_DGAIN_REG_VAL / 256)
#define ov5695_MAX_DGAIN    (ov5695_MAX_DGAIN_REG_VAL / 256)

#define ov5695_MIN_GAIN     \
    (ov5695_MIN_AGAIN*ov5695_MIN_DGAIN)
#define ov5695_MAX_GAIN    \
    (ov5695_MAX_AGAIN*ov5695_MAX_DGAIN)


#define START_REG_ARRAY \
{ \
  {0x0100, 0x01, 0xff}, \
}

#define STOP_REG_ARRAY \
{ \
  {0x0100, 0x00, 0xff}, \
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
  {0x0103, 0x01, 0xff}, \
  {0x0300, 0x04, 0x00}, \
  {0x0301, 0x00, 0x00}, \
  {0x0302, 0x69, 0x00}, \
  {0x0303, 0x00, 0x00}, \
  {0x0304, 0x00, 0x00}, \
  {0x0305, 0x01, 0x00}, \
  {0x0307, 0x00, 0x00}, \
  {0x030b, 0x00, 0x00}, \
  {0x030c, 0x00, 0x00}, \
  {0x030d, 0x1e, 0x00}, \
  {0x030e, 0x04, 0x00}, \
  {0x030f, 0x03, 0x00}, \
  {0x0312, 0x01, 0x00}, \
  {0x3000, 0x00, 0x00}, \
  {0x3002, 0x21, 0x00}, \
  {0x3022, 0x51, 0x00}, \
  {0x3106, 0x15, 0x00}, \
  {0x3107, 0x01, 0x00}, \
  {0x3108, 0x05, 0x00}, \
  {0x3500, 0x00, 0x00}, \
  {0x3501, 0x45, 0x00}, \
  {0x3502, 0x00, 0x00}, \
  {0x3503, 0x08, 0x00}, \
  {0x3504, 0x03, 0x00}, \
  {0x3505, 0x8c, 0x00}, \
  {0x3507, 0x03, 0x00}, \
  {0x3508, 0x00, 0x00}, \
  {0x3509, 0x10, 0x00}, \
  {0x350c, 0x00, 0x00}, \
  {0x350d, 0x80, 0x00}, \
  {0x3510, 0x00, 0x00}, \
  {0x3511, 0x02, 0x00}, \
  {0x3512, 0x00, 0x00}, \
  {0x3601, 0x55, 0x00}, \
  {0x3602, 0x58, 0x00}, \
  {0x3614, 0x30, 0x00}, \
  {0x3615, 0x77, 0x00}, \
  {0x3621, 0x08, 0x00}, \
  {0x3624, 0x40, 0x00}, \
  {0x3633, 0x0c, 0x00}, \
  {0x3634, 0x0c, 0x00}, \
  {0x3635, 0x0c, 0x00}, \
  {0x3636, 0x0c, 0x00}, \
  {0x3638, 0x00, 0x00}, \
  {0x3639, 0x00, 0x00}, \
  {0x363a, 0x00, 0x00}, \
  {0x363b, 0x00, 0x00}, \
  {0x363c, 0xff, 0x00}, \
  {0x363d, 0xfa, 0x00}, \
  {0x3650, 0x44, 0x00}, \
  {0x3651, 0x44, 0x00}, \
  {0x3652, 0x44, 0x00}, \
  {0x3653, 0x44, 0x00}, \
  {0x3654, 0x44, 0x00}, \
  {0x3655, 0x44, 0x00}, \
  {0x3656, 0x44, 0x00}, \
  {0x3657, 0x44, 0x00}, \
  {0x3660, 0x00, 0x00}, \
  {0x3661, 0x00, 0x00}, \
  {0x3662, 0x00, 0x00}, \
  {0x366a, 0x00, 0x00}, \
  {0x366e, 0x0c, 0x00}, \
  {0x3673, 0x04, 0x00}, \
  {0x3700, 0x14, 0x00}, \
  {0x3703, 0x0c, 0x00}, \
  {0x3715, 0x01, 0x00}, \
  {0x3733, 0x10, 0x00}, \
  {0x3734, 0x40, 0x00}, \
  {0x373f, 0xa0, 0x00}, \
  {0x3765, 0x20, 0x00}, \
  {0x37a1, 0x1d, 0x00}, \
  {0x37a8, 0x26, 0x00}, \
  {0x37ab, 0x14, 0x00}, \
  {0x37c2, 0x04, 0x00}, \
  {0x37cb, 0x09, 0x00}, \
  {0x37cc, 0x13, 0x00}, \
  {0x37cd, 0x1f, 0x00}, \
  {0x37ce, 0x1f, 0x00}, \
  {0x3800, 0x00, 0x00}, \
  {0x3801, 0x00, 0x00}, \
  {0x3802, 0x00, 0x00}, \
  {0x3803, 0x00, 0x00}, \
  {0x3804, 0x0a, 0x00}, \
  {0x3805, 0x3f, 0x00}, \
  {0x3806, 0x07, 0x00}, \
  {0x3807, 0xaf, 0x00}, \
  {0x3808, 0x05, 0x00}, \
  {0x3809, 0x10, 0x00}, \
  {0x380a, 0x03, 0x00}, \
  {0x380b, 0xcc, 0x00}, \
  {0x380c, 0x02, 0x00}, \
  {0x380d, 0xa0, 0x00}, \
  {0x380e, 0x08, 0x00}, \
  {0x380f, 0xb8, 0x00}, \
  {0x3810, 0x00, 0x00}, \
  {0x3811, 0x06, 0x00}, \
  {0x3812, 0x00, 0x00}, \
  {0x3813, 0x06, 0x00}, \
  {0x3814, 0x03, 0x00}, \
  {0x3815, 0x01, 0x00}, \
  {0x3816, 0x03, 0x00}, \
  {0x3817, 0x01, 0x00}, \
  {0x3818, 0x00, 0x00}, \
  {0x3819, 0x00, 0x00}, \
  {0x381a, 0x00, 0x00}, \
  {0x381b, 0x01, 0x00}, \
  {0x3820, 0x8b, 0x00}, \
  {0x3821, 0x01, 0x00}, \
  {0x3c80, 0x08, 0x00}, \
  {0x3c82, 0x00, 0x00}, \
  {0x3c83, 0x00, 0x00}, \
  {0x3c88, 0x00, 0x00}, \
  {0x3d85, 0x14, 0x00}, \
  {0x3f02, 0x08, 0x00}, \
  {0x3f03, 0x10, 0x00}, \
  {0x4008, 0x02, 0x00}, \
  {0x4009, 0x09, 0x00}, \
  {0x404e, 0x20, 0x00}, \
  {0x4501, 0x00, 0x00}, \
  {0x4502, 0x10, 0x00}, \
  {0x4800, 0x00, 0x00}, \
  {0x481f, 0x2a, 0x00}, \
  {0x4837, 0x13, 0x00}, \
  {0x5000, 0x17, 0x00}, \
  {0x5780, 0x3e, 0x00}, \
  {0x5781, 0x0f, 0x00}, \
  {0x5782, 0x44, 0x00}, \
  {0x5783, 0x02, 0x00}, \
  {0x5784, 0x01, 0x00}, \
  {0x5785, 0x01, 0x00}, \
  {0x5786, 0x00, 0x00}, \
  {0x5787, 0x04, 0x00}, \
  {0x5788, 0x02, 0x00}, \
  {0x5789, 0x0f, 0x00}, \
  {0x578a, 0xfd, 0x00}, \
  {0x578b, 0xf5, 0x00}, \
  {0x578c, 0xf5, 0x00}, \
  {0x578d, 0x03, 0x00}, \
  {0x578e, 0x08, 0x00}, \
  {0x578f, 0x0c, 0x00}, \
  {0x5790, 0x08, 0x00}, \
  {0x5791, 0x06, 0x00}, \
  {0x5792, 0x00, 0x00}, \
  {0x5793, 0x52, 0x00}, \
  {0x5794, 0xa3, 0x00}, \
  {0x5b00, 0x00, 0x00}, \
  {0x5b01, 0x1c, 0x00}, \
  {0x5b02, 0x00, 0x00}, \
  {0x5b03, 0x7f, 0x00}, \
  {0x5b05, 0x6c, 0x00}, \
  {0x5e10, 0xfc, 0x00}, \
  {0x4010, 0xf1, 0x00}, \
  {0x3503, 0x08, 0x00}, \
  {0x3505, 0x8c, 0x00}, \
  {0x3507, 0x03, 0x00}, \
  {0x3508, 0x00, 0x00}, \
  {0x3509, 0xf8, 0x00}, \
}

#define RES0_REG_ARRAY \
{ \
  {0x3501, 0x7e, 0x00}, \
  {0x366e, 0x18, 0x00}, \
  {0x3800, 0x00, 0x00}, \
  {0x3801, 0x00, 0x00}, \
  {0x3802, 0x00, 0x00}, \
  {0x3803, 0x04, 0x00}, \
  {0x3804, 0x0a, 0x00}, \
  {0x3805, 0x3f, 0x00}, \
  {0x3806, 0x07, 0x00}, \
  {0x3807, 0xab, 0x00}, \
  {0x3808, 0x0a, 0x00}, \
  {0x3809, 0x20, 0x00}, \
  {0x380a, 0x07, 0x00}, \
  {0x380b, 0x98, 0x00}, \
  {0x380c, 0x02, 0x00}, \
  {0x380d, 0xe4, 0x00}, \
  {0x380e, 0x07, 0x00}, \
  {0x380f, 0xe8, 0x00}, \
  {0x3811, 0x06, 0x00}, \
  {0x3813, 0x08, 0x00}, \
  {0x3814, 0x01, 0x00}, \
  {0x3816, 0x01, 0x00}, \
  {0x3817, 0x01, 0x00}, \
  {0x3820, 0x88, 0x00}, \
  {0x3821, 0x00, 0x00}, \
  {0x4501, 0x00, 0x00}, \
  {0x4008, 0x04, 0x00}, \
  {0x4009, 0x13, 0x00}, \
}

#define RES1_REG_ARRAY \
{ \
  {0x3501, 0x45, 0x00}, \
  {0x366e, 0x0c, 0x00}, \
  {0x3800, 0x00, 0x00}, \
  {0x3801, 0x00, 0x00}, \
  {0x3802, 0x00, 0x00}, \
  {0x3803, 0x00, 0x00}, \
  {0x3804, 0x0a, 0x00}, \
  {0x3805, 0x3f, 0x00}, \
  {0x3806, 0x07, 0x00}, \
  {0x3807, 0xaf, 0x00}, \
  {0x3808, 0x05, 0x00}, \
  {0x3809, 0x10, 0x00}, \
  {0x380a, 0x03, 0x00}, \
  {0x380b, 0xcc, 0x00}, \
  {0x380c, 0x02, 0x00}, \
  {0x380d, 0xa0, 0x00}, \
  {0x380e, 0x08, 0x00}, \
  {0x380f, 0xb8, 0x00}, \
  {0x3811, 0x06, 0x00}, \
  {0x3813, 0x06, 0x00}, \
  {0x3814, 0x03, 0x00}, \
  {0x3816, 0x03, 0x00}, \
  {0x3817, 0x01, 0x00}, \
  {0x3820, 0x8b, 0x00}, \
  {0x3821, 0x01, 0x00}, \
  {0x4501, 0x00, 0x00}, \
  {0x4008, 0x02, 0x00}, \
  {0x4009, 0x09, 0x00}, \
}

#define RES2_REG_ARRAY \
{ \
  {0x3501, 0x45, 0x00}, \
  {0x366e, 0x0c, 0x00}, \
  {0x3800, 0x00, 0x00}, \
  {0x3801, 0x00, 0x00}, \
  {0x3802, 0x00, 0x00}, \
  {0x3803, 0x00, 0x00}, \
  {0x3804, 0x0a, 0x00}, \
  {0x3805, 0x3f, 0x00}, \
  {0x3806, 0x07, 0x00}, \
  {0x3807, 0xaf, 0x00}, \
  {0x3808, 0x05, 0x00}, \
  {0x3809, 0x10, 0x00}, \
  {0x380a, 0x03, 0x00}, \
  {0x380b, 0xcc, 0x00}, \
  {0x380c, 0x02, 0x00}, \
  {0x380d, 0xa0, 0x00}, \
  {0x380e, 0x04, 0x00}, \
  {0x380f, 0x5c, 0x00}, \
  {0x3811, 0x06, 0x00}, \
  {0x3813, 0x06, 0x00}, \
  {0x3814, 0x03, 0x00}, \
  {0x3816, 0x03, 0x00}, \
  {0x3817, 0x01, 0x00}, \
  {0x3820, 0x8b, 0x00}, \
  {0x3821, 0x01, 0x00}, \
  {0x4501, 0x00, 0x00}, \
  {0x4008, 0x02, 0x00}, \
  {0x4009, 0x09, 0x00}, \
}

#define RES3_REG_ARRAY \
{ \
  {0x3501, 0x22, 0x00}, \
  {0x366e, 0x0c, 0x00}, \
  {0x3800, 0x00, 0x00}, \
  {0x3801, 0x00, 0x00}, \
  {0x3802, 0x00, 0x00}, \
  {0x3803, 0x08, 0x00}, \
  {0x3804, 0x0a, 0x00}, \
  {0x3805, 0x3f, 0x00}, \
  {0x3806, 0x07, 0x00}, \
  {0x3807, 0xa7, 0x00}, \
  {0x3808, 0x02, 0x00}, \
  {0x3809, 0x80, 0x00}, \
  {0x380a, 0x01, 0x00}, \
  {0x380b, 0xe0, 0x00}, \
  {0x380c, 0x02, 0x00}, \
  {0x380d, 0xa0, 0x00}, \
  {0x380e, 0x02, 0x00}, \
  {0x380f, 0xe8, 0x00}, \
  {0x3811, 0x06, 0x00}, \
  {0x3813, 0x04, 0x00}, \
  {0x3814, 0x07, 0x00}, \
  {0x3816, 0x05, 0x00}, \
  {0x3817, 0x03, 0x00}, \
  {0x3820, 0x8d, 0x00}, \
  {0x3821, 0x01, 0x00}, \
  {0x4501, 0x00, 0x00}, \
  {0x4008, 0x02, 0x00}, \
  {0x4009, 0x09, 0x00}, \
}

#define RES4_REG_ARRAY \
{ \
  {0x3501, 0x22, 0x00}, \
  {0x366e, 0x0c, 0x00}, \
  {0x3800, 0x00, 0x00}, \
  {0x3801, 0x00, 0x00}, \
  {0x3802, 0x00, 0x00}, \
  {0x3803, 0x08, 0x00}, \
  {0x3804, 0x0a, 0x00}, \
  {0x3805, 0x3f, 0x00}, \
  {0x3806, 0x07, 0x00}, \
  {0x3807, 0xa7, 0x00}, \
  {0x3808, 0x02, 0x00}, \
  {0x3809, 0x80, 0x00}, \
  {0x380a, 0x01, 0x00}, \
  {0x380b, 0xe0, 0x00}, \
  {0x380c, 0x02, 0x00}, \
  {0x380d, 0xa0, 0x00}, \
  {0x380e, 0x02, 0x00}, \
  {0x380f, 0x2e, 0x00}, \
  {0x3811, 0x06, 0x00}, \
  {0x3813, 0x04, 0x00}, \
  {0x3814, 0x07, 0x00}, \
  {0x3816, 0x05, 0x00}, \
  {0x3817, 0x03, 0x00}, \
  {0x3820, 0x8d, 0x00}, \
  {0x3821, 0x01, 0x00}, \
  {0x4501, 0x00, 0x00}, \
  {0x4008, 0x02, 0x00}, \
  {0x4009, 0x09, 0x00}, \
}

static sensor_lib_t sensor_lib_ptr =
{
  .sensor_slave_info =
  {
    .sensor_name = SENSOR_MODEL,
    .slave_addr = 0x20,
    .i2c_freq_mode = SENSOR_I2C_MODE_FAST,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .sensor_id_info =
    {
      .sensor_id_reg_addr = 0x300b,
      .sensor_id = 0x5695,
    },
    .power_setting_array =
    {
      .power_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VIO,
          .config_val = GPIO_OUT_HIGH,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VANA,
          .config_val = GPIO_OUT_HIGH,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VDIG,
          .config_val = GPIO_OUT_HIGH,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_STANDBY,
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
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_HIGH,
          .delay = 5,
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
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VDIG,
          .config_val = GPIO_OUT_LOW,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VANA,
          .config_val = GPIO_OUT_LOW,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VIO,
          .config_val = GPIO_OUT_LOW,
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
    .global_gain_addr = 0x3508,
    .vert_offset = 4,
  },
  .aec_info =
  {
    .min_gain = 1.0,
    .max_gain = 15.5,
    .max_analog_gain = ov5695_MAX_AGAIN,
    .max_linecount = 65530,
  },
  .sensor_num_frame_skip = 2,
  .sensor_num_HDR_frame_skip = 2,
  .sensor_max_pipeline_frame_delay = 2,
  .sensor_property =
  {
    .pix_size = 1.12,
    .sensing_method = SENSOR_SMETHOD_ONE_CHIP_COLOR_AREA_SENSOR,
    .crop_factor = 1.0,
  },
  .pixel_array_size_info =
  {
    .active_array_size =
    {
      .width = 2592,
      .height = 1944,
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
  .embedded_data_enable_settings =
  {
    .reg_setting_a = {},
    .addr_type = 0,
    .data_type = 0,
    .delay = 0,
  },
  .embedded_data_disable_settings =
  {
    .reg_setting_a = {},
    .addr_type = 0,
    .data_type = 0,
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
             {0x4303, 0x00, 0x00},
             {0x4320, 0x80, 0x00}
           },
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_BYTE_DATA,
          .delay = 0,
        },
      },
      {
        .mode = SENSOR_TEST_PATTERN_SOLID_COLOR,
        .settings =
        {
          .reg_setting_a =
           {
             {0x4303, 0x00, 0x00},
             {0x4320, 0x82, 0x00}
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
             {0x4320, 0x80, 0x00},
             {0x4303, 0x08, 0x00}
           },
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_BYTE_DATA,
          .delay = 0,
        },
      },
    },
    .size = 4,
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
      /* Res 3 */
      {
        .reg_setting_a = RES3_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 0,
      },
      /* Res 4 */
      {
        .reg_setting_a = RES4_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 0,
      },
    },
    .size = 5,
  },
  .out_info_array =
  {
    .out_info =
    {
      /* Res 0 */
      {
        .x_output = 2592,
        .y_output = 1944,
        .line_length_pclk = 740,
        .frame_length_lines = 2024,
        .op_pixel_clk = 168000000,
        .binning_factor = 1,
        .min_fps = 7.500,
        .max_fps = 30.00,
        .mode = SENSOR_DEFAULT_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0,
      },
      /* Res 1 */
      {
        .x_output = 1296,
        .y_output = 972,
        .line_length_pclk = 672,
        .frame_length_lines = 2232,
        .op_pixel_clk = 168000000,
        .binning_factor = 1,
        .min_fps = 7.500,
        .max_fps = 30.00,
        .mode = SENSOR_DEFAULT_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0,
      },
      /* Res 2 */
      {
        .x_output = 1296,
        .y_output = 972,
        .line_length_pclk = 672,
        .frame_length_lines = 1116,
        .op_pixel_clk = 168000000,
        .binning_factor = 1,
        .min_fps = 7.500,
        .max_fps = 60.00,
        .mode = SENSOR_HFR_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0,
      },
      /* Res 3 */
      {
        .x_output = 640,
        .y_output = 480,
        .line_length_pclk = 672,
        .frame_length_lines = 744,
        .op_pixel_clk = 168000000,
        .binning_factor = 1,
        .min_fps = 7.500,
        .max_fps = 90.00,
        .mode = SENSOR_HFR_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0,
      },
      /* Res 4 */
      {
        .x_output = 640,
        .y_output = 480,
        .line_length_pclk = 672,
        .frame_length_lines = 558,
        .op_pixel_clk = 168000000,
        .binning_factor = 1,
        .min_fps = 7.500,
        .max_fps = 120.00,
        .mode = SENSOR_HFR_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0,
      },
    },
    .size = 5,
  },
  .csi_params =
  {
    .lane_cnt = 2,
    .settle_cnt = 0x10,
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
      /* Res 4 */
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
    .size = 5,
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
      /* Res 4 */
      {
        .top_crop = 0,
        .bottom_crop = 0,
        .left_crop = 0,
        .right_crop = 0,
      },
    },
    .size = 5,
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
