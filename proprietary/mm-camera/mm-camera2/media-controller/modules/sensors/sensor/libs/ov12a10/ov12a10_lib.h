/* OV12a10_lib.h
 *
 *Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __OV12a10_LIB_H__
#define __OV12a10_LIB_H__

#include "sensor_lib.h"

#define SENSOR_MODEL "ov12a10"

#define OV12a10_MAX_INTEGRATION_MARGIN  8

#define OV12a10_DATA_PEDESTAL            0x40 /* 10bit value */

#define OV12a10_DIG_GAIN_GLOBAL_ADDR     0x350a

#define OV12a10_MIN_AGAIN_REG_VAL        0x80 /* 1.0x */

/* OV12a10_MAX_AGAIN_REG_VAL is 0x7FF, 2047, 15.99x
 * use max again 15.0x here
 */
#define OV12a10_MAX_AGAIN_REG_VAL       0x780 /* 15.0x */

#define OV12a10_MIN_DGAIN_REG_VAL        0x400 /* 1.0x */
#define OV12a10_MAX_DGAIN_REG_VAL        0x800 /* 2.0x */

#define OV12a10_MAX_DGAIN_DECIMATOR      1024

/* OV12a10 FORMULAS */
#define OV12a10_MIN_AGAIN    1.0
#define OV12a10_MAX_AGAIN    15.0

#define OV12a10_MIN_DGAIN    (OV12a10_MIN_DGAIN_REG_VAL / 1024)
#define OV12a10_MAX_DGAIN    (OV12a10_MAX_DGAIN_REG_VAL / 1024)

#define OV12a10_MIN_GAIN (OV12a10_MIN_AGAIN * OV12a10_MIN_DGAIN)
#define OV12a10_MAX_GAIN (OV12a10_MAX_AGAIN * OV12a10_MAX_DGAIN)

#define OV12A10_MAX_LINECOUNT (32767-8)

//#define OV12A10_ENABLE_MIRROR_FLIP

int OV12a10_calculate_exposure(float real_gain,
  unsigned int line_count, sensor_exposure_info_t *exp_info, float s_real_gain);

int OV12a10_fill_exposure_array(unsigned int gain,
  __attribute__((unused)) unsigned int digital_gain, unsigned int line,
  unsigned int fl_lines,  __attribute__((unused)) int luma_avg,
  __attribute__((unused)) unsigned int hdr_param,
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
    {0x3208, 0x00, 0x00}, \
}

#define GROUPOFF_REG_ARRAY \
{ \
    {0x3208, 0x10, 0x00}, \
    {0x3208, 0xa0, 0x00}, \
}

#ifdef OV12A10_ENABLE_MIRROR_FLIP
  #define RES0R3820 (0xb0)
  #define RES0R3811 (0x08)
  #define RES0R3813 (0x0a)

  #define RES1R3820 (0xb0)
  #define RES1R3811 (0x08)
  #define RES1R3813 (0x12)

  #define RES2R3820 (0xb0)
  #define RES2R3811 (0x88)
  #define RES2R3813 (0x5a)

  #define RES3R3820 (0xb3)
  #define RES3R3811 (0x44)
  #define RES3R3813 (0x04)

  #define RES4R3820 (0xb3)
  #define RES4R3811 (0x84)
  #define RES4R3813 (0x08)

  #define RES5R3820 (0xb3)
  #define RES5R3811 (0x84)
  #define RES5R3813 (0x08)
#else
  #define RES0R3820 (0xa8)
  #define RES0R3811 (0x0a)
  #define RES0R3813 (0x08)

  #define RES1R3820 (0xa8)
  #define RES1R3811 (0x0a)
  #define RES1R3813 (0x10)

  #define RES2R3820 (0xab)
  #define RES2R3811 (0x04)
  #define RES2R3813 (0x04)

  #define RES3R3820 (0xab)
  #define RES3R3811 (0x44)
  #define RES3R3813 (0x04)

  #define RES4R3820 (0xab)
  #define RES4R3811 (0x84)
  #define RES4R3813 (0x08)

  #define RES5R3820 (0xab)
  #define RES5R3811 (0x84)
  #define RES5R3813 (0x08)
#endif
#define DUALCAM_SLAVE_REG_ARRAY \
{ \
  /*ov12a10 FSIN_SLAVE */ \
    {0x3002, 0x21, 0x00}, \
    {0x3832, 0x08, 0x00}, \
    {0x3833, 0x30, 0x00}, \
    {0x3818, 0x00, 0x00}, \
    {0x3819, 0x00, 0x00}, \
    {0x381a, 0x00, 0x00}, \
    {0x381b, 0x01, 0x00}, \
    {0x3643, 0x22, 0x00}, \
    {0x3823, 0x60, 0x00}, \
    {0x3824, 0x02, 0x00}, \
    {0x3825, 0x14, 0x00}, \
    {0x3826, 0x1a, 0x00}, \
    {0x3827, 0x54, 0x00}, \
}

#define DUALCAM_MASTER_REG_ARRAY \
{ \
  /* VSYNC_Master */ \
    {0x3643, 0x22, 0x00}, \
    {0x3823, 0x18, 0x00}, \
    {0x3824, 0x00, 0x00}, \
    {0x3825, 0x20, 0x00}, \
    {0x3826, 0x00, 0x00}, \
    {0x3827, 0x00, 0x00}, \
    {0x3002, 0x61, 0x00}, \
    {0x3832, 0x18, 0x00}, \
    {0x3833, 0x10, 0x00}, \
    {0x3818, 0x02, 0x00}, \
    {0x3819, 0x10, 0x00}, \
    {0x381a, 0x1a, 0x00}, \
    {0x381b, 0x55, 0x00}, \
}

#define INIT0_REG_ARRAY \
{ \
    {0x0103, 0x01, 0x00}, \
    {0x0302, 0x49, 0x00}, \
    {0x0303, 0x00, 0x00}, \
    {0x0304, 0x00, 0x00}, \
    {0x0305, 0x01, 0x00}, \
    {0x030e, 0x02, 0x00}, \
    {0x0313, 0x05, 0x00}, \
    {0x3002, 0x21, 0x00}, \
    {0x3012, 0x40, 0x00}, \
    {0x3013, 0x72, 0x00}, \
    {0x3016, 0x72, 0x00}, \
    {0x301b, 0xd0, 0x00}, \
    {0x301d, 0xf0, 0x00}, \
    {0x301f, 0xd0, 0x00}, \
    {0x3021, 0x23, 0x00}, \
    {0x3022, 0x01, 0x00}, \
    {0x3106, 0x15, 0x00}, \
    {0x3107, 0x23, 0x00}, \
    {0x3500, 0x00, 0x00}, \
    {0x3501, 0xd0, 0x00}, \
    {0x3502, 0x00, 0x00}, \
    {0x3505, 0x83, 0x00}, \
    {0x3508, 0x02, 0x00}, \
    {0x3509, 0x00, 0x00}, \
    {0x3600, 0x43, 0x00}, \
    {0x3611, 0x8a, 0x00}, \
    {0x3613, 0x97, 0x00}, \
    {0x3620, 0x80, 0x00}, \
    {0x3624, 0x2c, 0x00}, \
    {0x3625, 0xa0, 0x00}, \
    {0x3626, 0x00, 0x00}, \
    {0x3631, 0x00, 0x00}, \
    {0x3632, 0x01, 0x00}, \
    {0x3641, 0x80, 0x00}, \
    {0x3642, 0x12, 0x00}, \
    {0x3644, 0x78, 0x00}, \
    {0x3645, 0xa7, 0x00}, \
    {0x364e, 0x44, 0x00}, \
    {0x364f, 0x44, 0x00}, \
    {0x3650, 0x11, 0x00}, \
    {0x3654, 0x00, 0x00}, \
    {0x3657, 0x31, 0x00}, \
    {0x3659, 0x0c, 0x00}, \
    {0x365f, 0x07, 0x00}, \
    {0x3661, 0x17, 0x00}, \
    {0x3662, 0x17, 0x00}, \
    {0x3663, 0x17, 0x00}, \
    {0x3664, 0x17, 0x00}, \
    {0x3666, 0x08, 0x00}, \
    {0x366b, 0x20, 0x00}, \
    {0x366c, 0xa4, 0x00}, \
    {0x366d, 0x20, 0x00}, \
    {0x366e, 0xa4, 0x00}, \
    {0x3680, 0x00, 0x00}, \
    {0x3714, 0x24, 0x00}, \
    {0x371a, 0x3e, 0x00}, \
    {0x3737, 0x04, 0x00}, \
    {0x3739, 0x12, 0x00}, \
    {0x3765, 0x20, 0x00}, \
    {0x3767, 0x00, 0x00}, \
    {0x37a1, 0x3e, 0x00}, \
    {0x37a8, 0x4d, 0x00}, \
    {0x37ab, 0x2c, 0x00}, \
    {0x37c2, 0x04, 0x00}, \
    {0x37d8, 0x03, 0x00}, \
    {0x37d9, 0x0c, 0x00}, \
    {0x37e0, 0x00, 0x00}, \
    {0x37e1, 0x0a, 0x00}, \
    {0x37e2, 0x14, 0x00}, \
    {0x37e3, 0x04, 0x00}, \
    {0x37e4, 0x2a, 0x00}, \
    {0x37e5, 0x03, 0x00}, \
    {0x37e6, 0x04, 0x00}, \
    {0x3800, 0x00, 0x00}, \
    {0x3801, 0x00, 0x00}, \
    {0x3802, 0x00, 0x00}, \
    {0x3803, 0x00, 0x00}, \
    {0x3804, 0x10, 0x00}, \
    {0x3805, 0x0f, 0x00}, \
    {0x3806, 0x0c, 0x00}, \
    {0x3807, 0x0f, 0x00}, \
    {0x3808, 0x10, 0x00}, \
    {0x3809, 0x00, 0x00}, \
    {0x380a, 0x0c, 0x00}, \
    {0x380b, 0x00, 0x00}, \
    {0x380c, 0x04, 0x00}, \
    {0x380d, 0x28, 0x00}, \
    {0x380e, 0x0d, 0x00}, \
    {0x380f, 0x12, 0x00}, \
    {0x3811, 0x0a, 0x00}, \
    {0x3813, 0x08, 0x00}, \
    {0x3814, 0x01, 0x00}, \
    {0x3815, 0x01, 0x00}, \
    {0x3816, 0x01, 0x00}, \
    {0x3817, 0x01, 0x00}, \
    {0x3820, 0xa8, 0x00}, \
    {0x3821, 0x00, 0x00}, \
    {0x3822, 0x91, 0x00}, \
    {0x3823, 0x18, 0x00}, \
    {0x3826, 0x00, 0x00}, \
    {0x3827, 0x00, 0x00}, \
    {0x3829, 0x03, 0x00}, \
    {0x3832, 0x08, 0x00}, \
    {0x3833, 0x30, 0x00}, \
    {0x3c80, 0x00, 0x00}, \
    {0x3c87, 0x01, 0x00}, \
    {0x3c8c, 0x1a, 0x00}, \
    {0x3c8d, 0x68, 0x00}, \
    {0x3c97, 0x02, 0x00}, \
    {0x3cc0, 0x40, 0x00}, \
    {0x3cc1, 0x54, 0x00}, \
    {0x3cc2, 0x34, 0x00}, \
    {0x3cc3, 0x04, 0x00}, \
    {0x3cc4, 0x00, 0x00}, \
    {0x3cc5, 0x00, 0x00}, \
    {0x3cc6, 0x00, 0x00}, \
    {0x3cc7, 0x00, 0x00}, \
    {0x3cc8, 0x00, 0x00}, \
    {0x3cc9, 0x00, 0x00}, \
    {0x3d8c, 0x73, 0x00}, \
    {0x3d8d, 0xc0, 0x00}, \
    {0x4001, 0x2b, 0x00}, \
    {0x4008, 0x02, 0x00}, \
    {0x4009, 0x0f, 0x00}, \
    {0x4011, 0xff, 0x00}, \
    {0x4013, 0x08, 0x00}, \
    {0x4014, 0x08, 0x00}, \
    {0x4015, 0x08, 0x00}, \
    {0x4017, 0x08, 0x00}, \
    {0x401a, 0x58, 0x00}, \
    {0x4050, 0x04, 0x00}, \
    {0x4051, 0x0b, 0x00}, \
    {0x405e, 0x20, 0x00}, \
    {0x4501, 0x00, 0x00}, \
    {0x4503, 0x00, 0x00}, \
    {0x450a, 0x04, 0x00}, \
    {0x4601, 0x30, 0x00}, \
    {0x4800, 0x00, 0x00}, \
    {0x481f, 0x30, 0x00}, \
    {0x4837, 0x0d, 0x00}, \
    {0x483c, 0x0f, 0x00}, \
    {0x484b, 0x01, 0x00}, \
    {0x4d00, 0x05, 0x00}, \
    {0x4d01, 0x19, 0x00}, \
    {0x4d02, 0xfd, 0x00}, \
    {0x4d03, 0xd1, 0x00}, \
    {0x4d04, 0xff, 0x00}, \
    {0x4d05, 0xff, 0x00}, \
    {0x5000, 0x09, 0x00}, \
    {0x5001, 0x42, 0x00}, \
    {0x5002, 0x45, 0x00}, \
    {0x5005, 0x00, 0x00}, \
    {0x5081, 0x04, 0x00}, \
    {0x5180, 0x00, 0x00}, \
    {0x5181, 0x10, 0x00}, \
    {0x5182, 0x02, 0x00}, \
    {0x5183, 0x0f, 0x00}, \
    {0x5185, 0x6c, 0x00}, \
    {0x5200, 0x03, 0x00}, \
    {0x520b, 0x07, 0x00}, \
    {0x520c, 0x0f, 0x00}, \
    {0x3642, 0x12, 0x00}, \
    {0x3666, 0x08, 0x00}, \
    {0x366b, 0x20, 0x00}, \
    {0x366c, 0xa4, 0x00}, \
    {0x366d, 0x20, 0x00}, \
    {0x366e, 0xa4, 0x00}, \
    {0x3714, 0x24, 0x00}, \
    {0x371a, 0x3e, 0x00}, \
    {0x3737, 0x04, 0x00}, \
    {0x3739, 0x12, 0x00}, \
    {0x37c2, 0x04, 0x00}, \
    {0x37d9, 0x0c, 0x00}, \
    {0x37e3, 0x04, 0x00}, \
    {0x37e4, 0x2a, 0x00}, \
    {0x37e6, 0x04, 0x00}, \
    {0x3800, 0x00, 0x00}, \
    {0x3801, 0x00, 0x00}, \
    {0x3802, 0x00, 0x00}, \
    {0x3803, 0x00, 0x00}, \
    {0x3804, 0x10, 0x00}, \
    {0x3805, 0x0f, 0x00}, \
    {0x3806, 0x0c, 0x00}, \
    {0x3807, 0x0f, 0x00}, \
    {0x3808, 0x10, 0x00}, \
    {0x3809, 0x00, 0x00}, \
    {0x380a, 0x0c, 0x00}, \
    {0x380b, 0x00, 0x00}, \
    {0x380e, 0x0d, 0x00}, \
    {0x380f, 0x36, 0x00}, \
    {0x3810, 0x00, 0x00}, \
    {0x3811, 0x0a, 0x00}, \
    {0x3813, 0x08, 0x00}, \
    {0x3814, 0x01, 0x00}, \
    {0x3816, 0x01, 0x00}, \
    {0x3820, 0xa8, 0x00}, \
    {0x4009, 0x0f, 0x00}, \
    {0x4050, 0x04, 0x00}, \
    {0x4051, 0x0b, 0x00}, \
    {0x4501, 0x00, 0x00}, \
    {0x5002, 0x45, 0x00}, \
    {0x3501, 0xd0, 0x00}, \
    {0x3502, 0x00, 0x00}, \
}

#define RES0_REG_ARRAY \
{ \
    {0x3642, 0x12, 0x00}, \
    {0x3666, 0x08, 0x00}, \
    {0x366b, 0x20, 0x00}, \
    {0x366c, 0xa4, 0x00}, \
    {0x366d, 0x20, 0x00}, \
    {0x366e, 0xa4, 0x00}, \
    {0x3714, 0x24, 0x00}, \
    {0x371a, 0x3e, 0x00}, \
    {0x3737, 0x04, 0x00}, \
    {0x3739, 0x12, 0x00}, \
    {0x37c2, 0x04, 0x00}, \
    {0x37d9, 0x0c, 0x00}, \
    {0x37e3, 0x04, 0x00}, \
    {0x37e4, 0x2a, 0x00}, \
    {0x37e6, 0x04, 0x00}, \
    {0x3800, 0x00, 0x00}, \
    {0x3801, 0x00, 0x00}, \
    {0x3802, 0x00, 0x00}, \
    {0x3803, 0x00, 0x00}, \
    {0x3804, 0x10, 0x00}, \
    {0x3805, 0x0f, 0x00}, \
    {0x3806, 0x0c, 0x00}, \
    {0x3807, 0x0f, 0x00}, \
    {0x3808, 0x10, 0x00}, \
    {0x3809, 0x00, 0x00}, \
    {0x380a, 0x0c, 0x00}, \
    {0x380b, 0x00, 0x00}, \
    {0x380e, 0x0d, 0x00}, \
    {0x380f, 0x36, 0x00}, \
    {0x3810, 0x00, 0x00}, \
    {0x3811, 0x0a, 0x00}, \
    {0x3813, 0x08, 0x00}, \
    {0x3814, 0x01, 0x00}, \
    {0x3816, 0x01, 0x00}, \
    {0x3820, 0xa8, 0x00}, \
    {0x4009, 0x0f, 0x00}, \
    {0x4050, 0x04, 0x00}, \
    {0x4051, 0x0b, 0x00}, \
    {0x4501, 0x00, 0x00}, \
    {0x5002, 0x45, 0x00}, \
    {0x3501, 0xd0, 0x00}, \
    {0x3502, 0x00, 0x00}, \
    {0x3820, RES0R3820, 0x00}, \
    {0x3811, RES0R3811, 0x00}, \
    {0x3813, RES0R3813, 0x00}, \
}

#define RES1_REG_ARRAY \
{ \
    {0x3642, 0x12, 0x00}, \
    {0x3666, 0x08, 0x00}, \
    {0x366b, 0x20, 0x00}, \
    {0x366c, 0xa4, 0x00}, \
    {0x366d, 0x20, 0x00}, \
    {0x366e, 0xa4, 0x00}, \
    {0x3714, 0x24, 0x00}, \
    {0x371a, 0x3e, 0x00}, \
    {0x3737, 0x04, 0x00}, \
    {0x3739, 0x12, 0x00}, \
    {0x37c2, 0x04, 0x00}, \
    {0x37d9, 0x0c, 0x00}, \
    {0x37e3, 0x04, 0x00}, \
    {0x37e4, 0x2a, 0x00}, \
    {0x37e6, 0x04, 0x00}, \
    {0x3800, 0x00, 0x00}, \
    {0x3801, 0x00, 0x00}, \
    {0x3802, 0x01, 0x00}, \
    {0x3803, 0x78, 0x00}, \
    {0x3804, 0x10, 0x00}, \
    {0x3805, 0x0f, 0x00}, \
    {0x3806, 0x0a, 0x00}, \
    {0x3807, 0x97, 0x00}, \
    {0x3808, 0x10, 0x00}, \
    {0x3809, 0x00, 0x00}, \
    {0x380a, 0x09, 0x00}, \
    {0x380b, 0x00, 0x00}, \
    {0x380e, 0x0d, 0x00}, \
    {0x380f, 0x36, 0x00}, \
    {0x3810, 0x00, 0x00}, \
    {0x3811, 0x0a, 0x00}, \
    {0x3813, 0x10, 0x00}, \
    {0x3814, 0x01, 0x00}, \
    {0x3816, 0x01, 0x00}, \
    {0x3820, 0xa8, 0x00}, \
    {0x4009, 0x0f, 0x00}, \
    {0x4050, 0x04, 0x00}, \
    {0x4051, 0x0b, 0x00}, \
    {0x4501, 0x00, 0x00}, \
    {0x5002, 0x45, 0x00}, \
    {0x3501, 0xd0, 0x00}, \
    {0x3502, 0x00, 0x00}, \
    {0x3820, RES1R3820, 0x00}, \
    {0x3811, RES1R3811, 0x00}, \
    {0x3813, RES1R3813, 0x00}, \
}

#define RES2_REG_ARRAY \
{ \
    {0x3642, 0x10, 0x00}, \
    {0x3666, 0x08, 0x00}, \
    {0x366b, 0x20, 0x00}, \
    {0x366c, 0xa4, 0x00}, \
    {0x366d, 0x20, 0x00}, \
    {0x366e, 0xa4, 0x00}, \
    {0x3714, 0x28, 0x00}, \
    {0x371a, 0x3e, 0x00}, \
    {0x3737, 0x08, 0x00}, \
    {0x3739, 0x20, 0x00}, \
    {0x37c2, 0x14, 0x00}, \
    {0x37d9, 0x0c, 0x00}, \
    {0x37e3, 0x08, 0x00}, \
    {0x37e4, 0x36, 0x00}, \
    {0x37e6, 0x08, 0x00}, \
    {0x3800, 0x00, 0x00}, \
    {0x3801, 0x00, 0x00}, \
    {0x3802, 0x00, 0x00}, \
    {0x3803, 0x00, 0x00}, \
    {0x3804, 0x10, 0x00}, \
    {0x3805, 0x1f, 0x00}, \
    {0x3806, 0x0c, 0x00}, \
    {0x3807, 0x0f, 0x00}, \
    {0x3808, 0x08, 0x00}, \
    {0x3809, 0x00, 0x00}, \
    {0x380a, 0x06, 0x00}, \
    {0x380b, 0x00, 0x00}, \
    {0x380e, 0x0d, 0x00}, \
    {0x380f, 0x36, 0x00}, \
    {0x3810, 0x00, 0x00}, \
    {0x3811, 0x04, 0x00}, \
    {0x3813, 0x04, 0x00}, \
    {0x3814, 0x03, 0x00}, \
    {0x3816, 0x03, 0x00}, \
    {0x3820, 0xab, 0x00}, \
    {0x4009, 0x0d, 0x00}, \
    {0x4050, 0x04, 0x00}, \
    {0x4051, 0x0b, 0x00}, \
    {0x4501, 0x00, 0x00}, \
    {0x5002, 0x45, 0x00}, \
    {0x3501, 0xd0, 0x00}, \
    {0x3502, 0x00, 0x00}, \
    {0x3820, RES2R3820, 0x00}, \
    {0x3811, RES2R3811, 0x00}, \
    {0x3813, RES2R3813, 0x00}, \
}

#define RES3_REG_ARRAY \
{ \
    {0x3642, 0x10, 0x00}, \
    {0x3666, 0x00, 0x00}, \
    {0x366b, 0xa4, 0x00}, \
    {0x366c, 0x20, 0x00}, \
    {0x366d, 0xa4, 0x00}, \
    {0x366e, 0x20, 0x00}, \
    {0x3714, 0x28, 0x00}, \
    {0x371a, 0x3e, 0x00}, \
    {0x3737, 0x08, 0x00}, \
    {0x3739, 0x20, 0x00}, \
    {0x37c2, 0x14, 0x00}, \
    {0x37d9, 0x0c, 0x00}, \
    {0x37e3, 0x08, 0x00}, \
    {0x37e4, 0x36, 0x00}, \
    {0x37e6, 0x08, 0x00}, \
    {0x3800, 0x00, 0x00}, \
    {0x3801, 0x00, 0x00}, \
    {0x3802, 0x01, 0x00}, \
    {0x3803, 0xc8, 0x00}, \
    {0x3804, 0x0f, 0x00}, \
    {0x3805, 0xff, 0x00}, \
    {0x3806, 0x0a, 0x00}, \
    {0x3807, 0x47, 0x00}, \
    {0x3808, 0x07, 0x00}, \
    {0x3809, 0x80, 0x00}, \
    {0x380a, 0x04, 0x00}, \
    {0x380b, 0x38, 0x00}, \
    {0x380e, 0x06, 0x00}, \
    {0x380f, 0x9a, 0x00}, \
    {0x3810, 0x00, 0x00}, \
    {0x3811, 0x44, 0x00}, \
    {0x3813, 0x04, 0x00}, \
    {0x3814, 0x03, 0x00}, \
    {0x3816, 0x03, 0x00}, \
    {0x3820, 0xab, 0x00}, \
    {0x4009, 0x0d, 0x00}, \
    {0x4050, 0x04, 0x00}, \
    {0x4051, 0x0b, 0x00}, \
    {0x4501, 0x00, 0x00}, \
    {0x5002, 0x45, 0x00}, \
    {0x3501, 0x69, 0x00}, \
    {0x3502, 0x00, 0x00}, \
    {0x3820, RES3R3820, 0x00}, \
    {0x3811, RES3R3811, 0x00}, \
    {0x3813, RES3R3813, 0x00}, \
}

#define RES4_REG_ARRAY \
{ \
    {0x3642, 0x10, 0x00}, \
    {0x3666, 0x00, 0x00}, \
    {0x366b, 0xa4, 0x00}, \
    {0x366c, 0x20, 0x00}, \
    {0x366d, 0xa4, 0x00}, \
    {0x366e, 0x20, 0x00}, \
    {0x3714, 0x28, 0x00}, \
    {0x371a, 0x3e, 0x00}, \
    {0x3737, 0x08, 0x00}, \
    {0x3739, 0x20, 0x00}, \
    {0x37c2, 0x14, 0x00}, \
    {0x37d9, 0x0c, 0x00}, \
    {0x37e3, 0x08, 0x00}, \
    {0x37e4, 0x36, 0x00}, \
    {0x37e6, 0x08, 0x00}, \
    {0x3800, 0x00, 0x00}, \
    {0x3801, 0x00, 0x00}, \
    {0x3802, 0x03, 0x00}, \
    {0x3803, 0x28, 0x00}, \
    {0x3804, 0x0f, 0x00}, \
    {0x3805, 0xff, 0x00}, \
    {0x3806, 0x08, 0x00}, \
    {0x3807, 0xe7, 0x00}, \
    {0x3808, 0x05, 0x00}, \
    {0x3809, 0x00, 0x00}, \
    {0x380a, 0x02, 0x00}, \
    {0x380b, 0xd0, 0x00}, \
    {0x380e, 0x03, 0x00}, \
    {0x380f, 0x4c, 0x00}, \
    {0x3810, 0x01, 0x00}, \
    {0x3811, 0x84, 0x00}, \
    {0x3813, 0x08, 0x00}, \
    {0x3814, 0x03, 0x00}, \
    {0x3816, 0x03, 0x00}, \
    {0x3820, 0xab, 0x00}, \
    {0x4009, 0x0d, 0x00}, \
    {0x4050, 0x04, 0x00}, \
    {0x4051, 0x0b, 0x00}, \
    {0x4501, 0x00, 0x00}, \
    {0x5002, 0x45, 0x00}, \
    {0x3501, 0x34, 0x00}, \
    {0x3502, 0x40, 0x00}, \
    {0x3820, RES4R3820, 0x00}, \
    {0x3811, RES4R3811, 0x00}, \
    {0x3813, RES4R3813, 0x00}, \
}

#define RES5_REG_ARRAY \
{ \
    {0x3642, 0x10, 0x00}, \
    {0x3666, 0x00, 0x00}, \
    {0x366b, 0xa4, 0x00}, \
    {0x366c, 0x20, 0x00}, \
    {0x366d, 0xa4, 0x00}, \
    {0x366e, 0x20, 0x00}, \
    {0x3714, 0x28, 0x00}, \
    {0x371a, 0x3e, 0x00}, \
    {0x3737, 0x08, 0x00}, \
    {0x3739, 0x20, 0x00}, \
    {0x37c2, 0x14, 0x00}, \
    {0x37d9, 0x0c, 0x00}, \
    {0x37e3, 0x08, 0x00}, \
    {0x37e4, 0x36, 0x00}, \
    {0x37e6, 0x08, 0x00}, \
    {0x3800, 0x00, 0x00}, \
    {0x3801, 0x00, 0x00}, \
    {0x3802, 0x03, 0x00}, \
    {0x3803, 0x28, 0x00}, \
    {0x3804, 0x0f, 0x00}, \
    {0x3805, 0xff, 0x00}, \
    {0x3806, 0x08, 0x00}, \
    {0x3807, 0xe7, 0x00}, \
    {0x3808, 0x05, 0x00}, \
    {0x3809, 0x00, 0x00}, \
    {0x380a, 0x02, 0x00}, \
    {0x380b, 0xd0, 0x00}, \
    {0x380e, 0x04, 0x00}, \
    {0x380f, 0x68, 0x00}, \
    {0x3810, 0x01, 0x00}, \
    {0x3811, 0x84, 0x00}, \
    {0x3813, 0x08, 0x00}, \
    {0x3814, 0x03, 0x00}, \
    {0x3816, 0x03, 0x00}, \
    {0x3820, 0xab, 0x00}, \
    {0x4009, 0x0d, 0x00}, \
    {0x4050, 0x04, 0x00}, \
    {0x4051, 0x0b, 0x00}, \
    {0x4501, 0x00, 0x00}, \
    {0x5002, 0x45, 0x00}, \
    {0x3501, 0x46, 0x00}, \
    {0x3502, 0x00, 0x00}, \
    {0x3820, RES5R3820, 0x00}, \
    {0x3811, RES5R3811, 0x00}, \
    {0x3813, RES5R3813, 0x00}, \
}

/* Sensor Handler */
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
      .sensor_id = 0x1241,
    },
     .power_setting_array =
    {
      .power_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VANA,
          .config_val = 0,
          .delay = 1,
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
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_V_CUSTOM1,
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
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VAF,
          .config_val = 0,
          .delay = 1,
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
      },
      .size = 9,
      .power_down_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_CLK,
          .seq_val = CAMERA_MCLK,
          .config_val = 0,
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VAF,
          .config_val = 0,
          .delay = 5,
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
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VDIG,
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
      },
      .size_down = 9,
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
    .y_output = 0x380A,
    .line_length_pclk = 0x380C,
    .frame_length_lines = 0x380E,
  },
  .exp_gain_info =
  {
    .coarse_int_time_addr = 0x3500,
    .global_gain_addr = 0x3508,
    .vert_offset = OV12a10_MAX_INTEGRATION_MARGIN,
  },
  .aec_info =
  {
    .min_gain = OV12a10_MIN_GAIN,
    .max_gain = OV12a10_MAX_GAIN,
    .max_analog_gain = OV12a10_MAX_AGAIN,
    .min_linecount = 4,
    .max_linecount = OV12A10_MAX_LINECOUNT,
  },
  .sensor_num_frame_skip = 2,
  .sensor_num_HDR_frame_skip = 2,
  .sensor_max_pipeline_frame_delay = 2,
  .sensor_property =
  {
    .pix_size = 1.25,
    .sensing_method = SENSOR_SMETHOD_ONE_CHIP_COLOR_AREA_SENSOR,
  },
  .pixel_array_size_info =
  {
    .active_array_size =
    {
      .width =  4096,
      .height = 3072,
    },
    .left_dummy = 8,
    .right_dummy = 8,
    .top_dummy = 8,
    .bottom_dummy = 8,
  },
  .color_level_info =
  {
    .white_level = 1023,
    .r_pedestal = OV12a10_DATA_PEDESTAL,
    .gr_pedestal = OV12a10_DATA_PEDESTAL,
    .gb_pedestal = OV12a10_DATA_PEDESTAL,
    .b_pedestal = OV12a10_DATA_PEDESTAL,
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
  .dualcam_master_settings =
  {
    .reg_setting_a = DUALCAM_MASTER_REG_ARRAY,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .data_type = CAMERA_I2C_BYTE_DATA,
    .delay = 0,
    .size = 13,
  },
  .test_pattern_info =
  {
    .test_pattern_settings =
    {
      {
        .mode = SENSOR_TEST_PATTERN_OFF,
        .settings =
        {
          .reg_setting_a = {{0x4503, 0x00, 0x00}},
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_BYTE_DATA,
          .delay = 0,
        }
      },
    },
    .size = 1,
    .solid_mode_addr = {
      .r_addr = 0,
      .gr_addr = 0,
      .gb_addr = 0,
      .b_addr = 0,
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
        .delay = 0,
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
      /* Res 5 */
      {
        .reg_setting_a = RES5_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 0,
      },
    },
    .size = 6,
  },
  .out_info_array =
  {
    .out_info =
    {
     /* Res 0 */
     {
      .x_output = 4096,
      .y_output = 3072,
      .line_length_pclk = 4256,
      .frame_length_lines = 3382,
      .op_pixel_clk = 468000000,
      .binning_factor = 1,
      .binning_method = 0,
      .min_fps = 7.5,
      .max_fps = 30,
      .mode = SENSOR_DEFAULT_MODE,
      .offset_x = 0,
      .offset_y = 0,
      .scale_factor = 1.000,
      .is_pdaf_supported = 1,
      .data_rate = 1120000000ULL * 4,
     },
     /* Res 1 */
     {
      .x_output = 4096,
      .y_output = 2304,
      .line_length_pclk = 4256,
      .frame_length_lines = 3382,
      .op_pixel_clk = 468000000,
      .binning_factor = 1,
      .binning_method = 0,
      .max_fps = 30,
      .min_fps = 7.5,
      .mode = SENSOR_DEFAULT_MODE,
      .offset_x = 0,
      .offset_y = 384,
      .scale_factor = 1.000,
      .is_pdaf_supported = 1,
      .data_rate = 1120000000ULL * 4,
     },
     /* Res 2 */
     {
      .x_output = 2048,
      .y_output = 1536,
      .line_length_pclk = 4256,
      .frame_length_lines = 3382,
      .op_pixel_clk = 468000000,
      .binning_factor = 2,
      .binning_method = 0,
      .max_fps = 30,
      .min_fps = 7.5,
      .mode = SENSOR_DEFAULT_MODE,
      .offset_x = 0,
      .offset_y = 0,
      .scale_factor = 1.000,
      .data_rate = 1120000000ULL * 4,
     },
     /* Res 3 */
     {
      .x_output = 1920,
      .y_output = 1080,
      .line_length_pclk = 4256,
      .frame_length_lines = 1690,
      .op_pixel_clk = 468000000,
      .binning_factor = 2,
      .binning_method = 0,
      .max_fps = 60.0,
      .min_fps = 7.5,
      .mode = SENSOR_HFR_MODE | SENSOR_DEFAULT_MODE,
      .offset_x = 64,
      .offset_y = 228,
      .scale_factor = 1.000,
      .data_rate = 1120000000ULL * 4,
     },
     /* Res 4 */
     {
      .x_output = 1280,
      .y_output = 720,
      .line_length_pclk = 4256,
      .frame_length_lines = 1128,
      .op_pixel_clk = 468000000,
      .binning_factor = 2,
      .binning_method = 0,
      .max_fps = 90.0,
      .min_fps = 7.5,
      .mode = SENSOR_DEFAULT_MODE | SENSOR_HFR_MODE,
      .offset_x = 384,
      .offset_y = 408,
      .scale_factor = 1.000,
      .data_rate = 1120000000ULL * 4,
     },
     /* Res 5 */
     {
      .x_output = 1280,
      .y_output = 720,
      .line_length_pclk = 4256,
      .frame_length_lines = 844,
      .op_pixel_clk = 468000000,
      .binning_factor = 2,
      .binning_method = 0,
      .max_fps = 120,
      .min_fps = 7.5,
      .mode = SENSOR_HFR_MODE,
      .offset_x = 384,
      .offset_y = 408,
      .scale_factor = 1.000,
      .data_rate = 1120000000ULL * 4,
     },
    },
    .size = 6,
  },
  .csi_params =
  {
    .lane_cnt = 4,
    .settle_cnt = 0x14,
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
      /* Res 5 */
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
    .size = 6,
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
      /* Res 5 */
      {
        .top_crop = 0,
        .bottom_crop = 0,
        .left_crop = 0,
        .right_crop = 0,
      },
    },
    .size = 6,
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
  .sensor_capability = 0,
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
   .app_delay = {
    [SENSOR_DELAY_ANALOG_SENSOR_GAIN] = 0,
    [SENSOR_DELAY_DIGITAL_SENSOR_GAIN] = 0,
  },
  .adc_readout_time = 6000000,
  .sensor_num_fast_aec_frame_skip = 0,
  .noise_coeff = {
    .gradient_S = 3.738032e-06,
    .offset_S = 3.651935e-04,
    .gradient_O = 6.396835e-11,
    .offset_O = -2.968624e-04,
  },
  .pdaf_config = {
#ifdef OV12A10_ENABLE_MIRROR_FLIP
    #include "ov12a10_pdaf_mirror_flip.h"
#else
    #include "ov12a10_pdaf.h"
#endif
   }
};

#endif /* __OV12a10_LIB_H__ */
