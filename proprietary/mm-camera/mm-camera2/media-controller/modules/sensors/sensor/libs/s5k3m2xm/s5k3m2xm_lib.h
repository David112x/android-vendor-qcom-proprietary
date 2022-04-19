/* s5k3m2xm_lib.h
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc
 */

#ifndef __S5K3M2XM_LIB_H__
#define __S5K3M2XM_LIB_H__

#include "sensor_lib.h"
#include "eeprom_lib.h"
#include "pdaf_api.h"
#include "pdaf_camif_api.h"
#define SENSOR_MODEL "s5k3m2xm"

/* S5K3M2XM Regs */
#define S5K3M2XM_DIG_GAIN_GR_ADDR       0x020E /* 0x4000020E */
#define S5K3M2XM_DIG_GAIN_R_ADDR        0x0210 /* 0x40000210 */
#define S5K3M2XM_DIG_GAIN_B_ADDR        0x0212 /* 0x40000212 */
#define S5K3M2XM_DIG_GAIN_GB_ADDR       0x0214 /* 0x40000214 */

/* S5K3M2XM CONSTANTS */
#define S5K3M2XM_MAX_INTEGRATION_MARGIN   8  /* Unknown */

#define S5K3M2XM_DATA_PEDESTAL            0x40 /* 10bit */

#define S5K3M2XM_MIN_AGAIN_REG_VAL        32 /* 1.0x */
#define S5K3M2XM_MAX_AGAIN_REG_VAL        512 /* 16.0x */

#define S5K3M2XM_MIN_DGAIN_REG_VAL        256 /* 1.0x */
#define S5K3M2XM_MAX_DGAIN_REG_VAL        3072 /* 12.0x */

/* S5K3M2XM FORMULAS */
#define S5K3M2XM_MIN_AGAIN    (1.0)
#define S5K3M2XM_MAX_AGAIN    (S5K3M2XM_MAX_AGAIN_REG_VAL / 32)

#define S5K3M2XM_MIN_DGAIN    (S5K3M2XM_MIN_DGAIN_REG_VAL / 256)
#define S5K3M2XM_MAX_DGAIN    (S5K3M2XM_MAX_DGAIN_REG_VAL / 256)

#define S5K3M2XM_MIN_GAIN     S5K3M2XM_MIN_AGAIN * S5K3M2XM_MIN_DGAIN
#define S5K3M2XM_MAX_GAIN     S5K3M2XM_MAX_AGAIN * S5K3M2XM_MAX_DGAIN

/* uncomment FLIP_MIRROR macro to
   enable flip and mirror in sensor readout
   change bayer pattern
   load pdaf flip and mirror header
*/
//#define FLIP_MIRROR

int32_t s5k3m2xm_calculate_exposure(float real_gain,
  uint32_t line_count, sensor_exposure_info_t *exp_info, float s_real_gain);

int32_t s5k3m2xm_fill_exposure_array(uint32_t gain,
  __attribute__((unused)) uint32_t digital_gain, uint32_t line,
  uint32_t fl_lines,  __attribute__((unused)) int32_t luma_avg,
  __attribute__((unused)) uint32_t hdr_param,
  struct camera_i2c_reg_setting* reg_setting,
  __attribute__((unused)) unsigned int s_gain,
  __attribute__((unused)) int s_linecount,
  __attribute__((unused)) int is_hdr_enabled);

#ifndef FLIP_MIRROR
#define START_REG_ARRAY \
{ \
  {0x6028, 0x4000, 0x00}, \
  {0x602A, 0x0100, 0x00}, \
  {0x6F12, 0x0100, 0x00}, \
}
#else
#define START_REG_ARRAY \
{ \
  {0x6028, 0x4000, 0x00}, \
  {0x602A, 0x0100, 0x00}, \
  {0x6F12, 0x0103, 0X00}, \
}
#endif

#define STOP_REG_ARRAY \
{ \
  {0x6028, 0x4000, 0x00}, \
  {0x602A, 0x0100, 0x00}, \
  {0x6F12, 0x0000, 0x00}, \
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
  {0x6028, 0x4000, 0x00}, \
  {0x6214, 0x7971, 0x00}, \
  {0x6218, 0x0100, 0x00}, \
  {0x6028, 0x2000, 0x00}, \
  {0x602A, 0x448C, 0x00}, \
  {0x6F12, 0x0000, 0x00}, \
  {0x6F12, 0x0000, 0x00}, \
  {0x6F12, 0x0448, 0x00}, \
  {0x6F12, 0x0349, 0x00}, \
  {0x6F12, 0x0160, 0x00}, \
  {0x6F12, 0xC26A, 0x00}, \
  {0x6F12, 0x511A, 0x00}, \
  {0x6F12, 0x8180, 0x00}, \
  {0x6F12, 0x00F0, 0x00}, \
  {0x6F12, 0x2CB8, 0x00}, \
  {0x6F12, 0x2000, 0x00}, \
  {0x6F12, 0x4538, 0x00}, \
  {0x6F12, 0x2000, 0x00}, \
  {0x6F12, 0x1FA0, 0x00}, \
  {0x6F12, 0x0000, 0x00}, \
  {0x6F12, 0x0000, 0x00}, \
  {0x6F12, 0x0000, 0x00}, \
  {0x6F12, 0x0000, 0x00}, \
  {0x6F12, 0x2DE9, 0x00}, \
  {0x6F12, 0xF041, 0x00}, \
  {0x6F12, 0x0546, 0x00}, \
  {0x6F12, 0x1348, 0x00}, \
  {0x6F12, 0x134E, 0x00}, \
  {0x6F12, 0x018A, 0x00}, \
  {0x6F12, 0x4069, 0x00}, \
  {0x6F12, 0x06F1, 0x00}, \
  {0x6F12, 0x2007, 0x00}, \
  {0x6F12, 0x4143, 0x00}, \
  {0x6F12, 0x4FEA, 0x00}, \
  {0x6F12, 0x1138, 0x00}, \
  {0x6F12, 0x0024, 0x00}, \
  {0x6F12, 0x06EB, 0x00}, \
  {0x6F12, 0xC402, 0x00}, \
  {0x6F12, 0x0423, 0x00}, \
  {0x6F12, 0x3946, 0x00}, \
  {0x6F12, 0x4046, 0x00}, \
  {0x6F12, 0x00F0, 0x00}, \
  {0x6F12, 0x1EF8, 0x00}, \
  {0x6F12, 0x25F8, 0x00}, \
  {0x6F12, 0x1400, 0x00}, \
  {0x6F12, 0x641C, 0x00}, \
  {0x6F12, 0x042C, 0x00}, \
  {0x6F12, 0xF3DB, 0x00}, \
  {0x6F12, 0x0A48, 0x00}, \
  {0x6F12, 0x2988, 0x00}, \
  {0x6F12, 0x0180, 0x00}, \
  {0x6F12, 0x6988, 0x00}, \
  {0x6F12, 0x4180, 0x00}, \
  {0x6F12, 0xA988, 0x00}, \
  {0x6F12, 0x8180, 0x00}, \
  {0x6F12, 0xE988, 0x00}, \
  {0x6F12, 0xC180, 0x00}, \
  {0x6F12, 0xBDE8, 0x00}, \
  {0x6F12, 0xF081, 0x00}, \
  {0x6F12, 0x0022, 0x00}, \
  {0x6F12, 0xAFF2, 0x00}, \
  {0x6F12, 0x4B01, 0x00}, \
  {0x6F12, 0x0448, 0x00}, \
  {0x6F12, 0x00F0, 0x00}, \
  {0x6F12, 0x0DB8, 0x00}, \
  {0x6F12, 0x2000, 0x00}, \
  {0x6F12, 0x34D0, 0x00}, \
  {0x6F12, 0x2000, 0x00}, \
  {0x6F12, 0x7900, 0x00}, \
  {0x6F12, 0x4000, 0x00}, \
  {0x6F12, 0xD22E, 0x00}, \
  {0x6F12, 0x0000, 0x00}, \
  {0x6F12, 0x2941, 0x00}, \
  {0x6F12, 0x40F2, 0x00}, \
  {0x6F12, 0xFD7C, 0x00}, \
  {0x6F12, 0xC0F2, 0x00}, \
  {0x6F12, 0x000C, 0x00}, \
  {0x6F12, 0x6047, 0x00}, \
  {0x6F12, 0x4DF2, 0x00}, \
  {0x6F12, 0x474C, 0x00}, \
  {0x6F12, 0xC0F2, 0x00}, \
  {0x6F12, 0x000C, 0x00}, \
  {0x6F12, 0x6047, 0x00}, \
  {0x6F12, 0x0000, 0x00}, \
  {0x6F12, 0x0000, 0x00}, \
  {0x6F12, 0x0000, 0x00}, \
  {0x6F12, 0x0000, 0x00}, \
  {0x6F12, 0x30D2, 0x00}, \
  {0x6F12, 0x029C, 0x00}, \
  {0x6F12, 0x0000, 0x00}, \
  {0x6F12, 0x0001, 0x00}, \
  {0x602A, 0x7900, 0x00}, \
  {0x6F12, 0x4000, 0x00}, \
  {0x6F12, 0x3000, 0x00}, \
  {0x6F12, 0x2000, 0x00}, \
  {0x6F12, 0x1000, 0x00}, \
  {0x6F12, 0x4000, 0x00}, \
  {0x6F12, 0x3000, 0x00}, \
  {0x6F12, 0x2000, 0x00}, \
  {0x6F12, 0x1000, 0x00}, \
  {0x6F12, 0x4000, 0x00}, \
  {0x6F12, 0x3000, 0x00}, \
  {0x6F12, 0x2000, 0x00}, \
  {0x6F12, 0x1000, 0x00}, \
  {0x6F12, 0x4000, 0x00}, \
  {0x6F12, 0x3000, 0x00}, \
  {0x6F12, 0x2000, 0x00}, \
  {0x6F12, 0x1000, 0x00}, \
  {0x6F12, 0x0100, 0x00}, \
  {0x6F12, 0x0200, 0x00}, \
  {0x6F12, 0x0400, 0x00}, \
  {0x6F12, 0x0800, 0x00}, \
  {0x602A, 0x43F0, 0x00}, \
  {0x6F12, 0x0128, 0x00}, \
  {0x6F12, 0x00DC, 0x00}, \
  {0x6F12, 0x5590, 0x00}, \
  {0x6F12, 0x3644, 0x00}, \
  {0x602A, 0x1B50, 0x00}, \
  {0x6F12, 0x0000, 0x00}, \
  {0x602A, 0x1B54, 0x00}, \
  {0x6F12, 0x0000, 0x00}, \
  {0x602A, 0x1B64, 0x00}, \
  {0x6F12, 0x0800, 0x00}, \
  {0x602A, 0x1926, 0x00}, \
  {0x6F12, 0x0011, 0x00}, \
  {0x602A, 0x14FA, 0x00}, \
  {0x6F12, 0x0F00, 0x00}, \
  {0x602A, 0x4472, 0x00}, \
  {0x6F12, 0x0102, 0x00}, \
  {0x6028, 0x4000, 0x00}, \
  {0x0B04, 0x0000, 0x00}, \
  {0x3B22, 0x1110, 0x00}, \
  {0xF42E, 0x200C, 0x00}, \
  {0xF49E, 0x004C, 0x00}, \
  {0xF4A6, 0x00F0, 0x00}, \
  {0x3AFA, 0xFBB8, 0x00}, \
  {0xF49C, 0x0000, 0x00}, \
  {0xF496, 0x0000, 0x00}, \
  {0xF476, 0x0040, 0x00}, \
  {0x3AAA, 0x0205, 0x00}, \
  {0x3AFE, 0x07DF, 0x00}, \
  {0xF47A, 0x001B, 0x00}, \
  {0xF462, 0x0003, 0x00}, \
  {0xF460, 0x0020, 0x00}, \
  {0x3B06, 0x000E, 0x00}, \
  {0x3AD0, 0x0080, 0x00}, \
  {0x3B02, 0x0020, 0x00}, \
  {0xF468, 0x0001, 0x00}, \
  {0xF494, 0x000E, 0x00}, \
  {0xF40C, 0x2180, 0x00}, \
  {0x3870, 0x004C, 0x00}, \
  {0x3876, 0x0011, 0x00}, \
  {0x3366, 0x0128, 0x00}, \
  {0x3852, 0x00EA, 0x00}, \
  {0x623E, 0x0004, 0x00}, \
  {0x3B5C, 0x0006, 0x00}, \
}

#define RES0_REG_ARRAY \
{ \
  {0x6028, 0x2000, 0x00}, \
  {0x602A, 0x14F0, 0x00}, \
  {0x6F12, 0x0040, 0x00}, \
  {0x6F12, 0x0040, 0x00}, \
  {0x602A, 0x192C, 0x00}, \
  {0x6F12, 0x0001, 0x00}, \
  {0x6028, 0x4000, 0x00}, \
  {0x0344, 0x0008, 0x00}, \
  {0x0346, 0x0008, 0x00}, \
  {0x0348, 0x1077, 0x00}, \
  {0x034A, 0x0C37, 0x00}, \
  {0x034C, 0x1070, 0x00}, \
  {0x034E, 0x0C30, 0x00}, \
  {0x0900, 0x0111, 0x00}, \
  {0x0380, 0x0001, 0x00}, \
  {0x0382, 0x0001, 0x00}, \
  {0x0384, 0x0001, 0x00}, \
  {0x0386, 0x0001, 0x00}, \
  {0x0400, 0x0002, 0x00}, \
  {0x0404, 0x0010, 0x00}, \
  {0x0114, 0x0300, 0x00}, \
  {0x0110, 0x0002, 0x00}, \
  {0x112C, 0x0000, 0x00}, \
  {0x112E, 0x0000, 0x00}, \
  {0x0136, 0x1800, 0x00}, \
  {0x0304, 0x0006, 0x00}, \
  {0x0306, 0x006E, 0x00}, \
  {0x0302, 0x0001, 0x00}, \
  {0x0300, 0x0004, 0x00}, \
  {0x030C, 0x0004, 0x00}, \
  {0x030E, 0x005A, 0x00}, \
  {0x030A, 0x0001, 0x00}, \
  {0x0308, 0x0008, 0x00}, \
  {0x0342, 0x11F0, 0x00}, \
  {0x0340, 0x0C74, 0x00}, \
  {0x0202, 0x0200, 0x00}, \
  {0x0200, 0x0400, 0x00}, \
  {0x0B04, 0x0000, 0x00}, \
  {0x0B08, 0x0000, 0x00}, \
  {0x0B00, 0x0080, 0x00}, \
  {0x3B3C, 0x0107, 0x00}, \
  {0x3B34, 0x3030, 0x00}, \
  {0x3B36, 0x3030, 0x00}, \
  {0x3B38, 0x3030, 0x00}, \
  {0x3B3A, 0x3030, 0x00}, \
  {0x306A, 0x0068, 0x00}, \
}

#define RES1_REG_ARRAY \
{ \
  {0x6028, 0x2000, 0x00}, \
  {0x602A, 0x14F0, 0x00}, \
  {0x6F12, 0x0040, 0x00}, \
  {0x6F12, 0x0040, 0x00}, \
  {0x602A, 0x192C, 0x00}, \
  {0x6F12, 0x0001, 0x00}, \
  {0x6028, 0x4000, 0x00}, \
  {0x0344, 0x0004, 0x00}, \
  {0x0346, 0x0000, 0x00}, \
  {0x0348, 0x107B, 0x00}, \
  {0x034A, 0x0C3F, 0x00}, \
  {0x034C, 0x0838, 0x00}, \
  {0x034E, 0x0618, 0x00}, \
  {0x0900, 0x0112, 0x00}, \
  {0x0380, 0x0001, 0x00}, \
  {0x0382, 0x0001, 0x00}, \
  {0x0384, 0x0001, 0x00}, \
  {0x0386, 0x0003, 0x00}, \
  {0x0400, 0x0001, 0x00}, \
  {0x0404, 0x0020, 0x00}, \
  {0x0114, 0x0300, 0x00}, \
  {0x0110, 0x0002, 0x00}, \
  {0x112C, 0x0000, 0x00}, \
  {0x112E, 0x0000, 0x00}, \
  {0x0136, 0x1800, 0x00}, \
  {0x0304, 0x0006, 0x00}, \
  {0x0306, 0x006E, 0x00}, \
  {0x0302, 0x0001, 0x00}, \
  {0x0300, 0x0004, 0x00}, \
  {0x030C, 0x0004, 0x00}, \
  {0x030E, 0x005A, 0x00}, \
  {0x030A, 0x0001, 0x00}, \
  {0x0308, 0x0008, 0x00}, \
  {0x0342, 0x11F0, 0x00}, \
  {0x0340, 0x0C74, 0x00}, \
  {0x0202, 0x0200, 0x00}, \
  {0x0200, 0x0400, 0x00}, \
  {0x0204, 0x0020, 0x00}, \
  {0x0B04, 0x0000, 0x00}, \
  {0x0B08, 0x0000, 0x00}, \
  {0x0B00, 0x0080, 0x00}, \
  {0x3B3C, 0x0107, 0x00}, \
  {0x3B34, 0x3030, 0x00}, \
  {0x3B36, 0x3030, 0x00}, \
  {0x3B38, 0x3030, 0x00}, \
  {0x3B3A, 0x3030, 0x00}, \
  {0x306A, 0x0068, 0x00}, \
}

#define RES2_REG_ARRAY \
{ \
  {0x6028, 0x2000, 0x00}, \
  {0x602A, 0x14F0, 0x00}, \
  {0x6F12, 0x0040, 0x00}, \
  {0x6F12, 0x0040, 0x00}, \
  {0x602A, 0x192C, 0x00}, \
  {0x6F12, 0x0001, 0x00}, \
  {0x6028, 0x4000, 0x00}, \
  {0x0344, 0x00BC, 0x00}, \
  {0x0346, 0x01E0, 0x00}, \
  {0x0348, 0x0FC3, 0x00}, \
  {0x034A, 0x0A5F, 0x00}, \
  {0x034C, 0x0780, 0x00}, \
  {0x034E, 0x0438, 0x00}, \
  {0x0900, 0x0112, 0x00}, \
  {0x0380, 0x0001, 0x00}, \
  {0x0382, 0x0001, 0x00}, \
  {0x0384, 0x0001, 0x00}, \
  {0x0386, 0x0003, 0x00}, \
  {0x0400, 0x0001, 0x00}, \
  {0x0404, 0x0020, 0x00}, \
  {0x0114, 0x0300, 0x00}, \
  {0x0110, 0x0002, 0x00}, \
  {0x112C, 0x0000, 0x00}, \
  {0x112E, 0x0000, 0x00}, \
  {0x0136, 0x1800, 0x00}, \
  {0x0304, 0x0006, 0x00}, \
  {0x0306, 0x006E, 0x00}, \
  {0x0302, 0x0001, 0x00}, \
  {0x0300, 0x0004, 0x00}, \
  {0x030C, 0x0004, 0x00}, \
  {0x030E, 0x005A, 0x00}, \
  {0x030A, 0x0001, 0x00}, \
  {0x0308, 0x0008, 0x00}, \
  {0x0342, 0x11F0, 0x00}, \
  {0x0340, 0x063c, 0x00}, \
  {0x0202, 0x0200, 0x00}, \
  {0x0200, 0x0400, 0x00}, \
  {0x0B04, 0x0000, 0x00}, \
  {0x0B08, 0x0000, 0x00}, \
  {0x0B00, 0x0080, 0x00}, \
  {0x3B3C, 0x0107, 0x00}, \
  {0x3B34, 0x3030, 0x00}, \
  {0x3B36, 0x3030, 0x00}, \
  {0x3B38, 0x3030, 0x00}, \
  {0x3B3A, 0x3030, 0x00}, \
  {0x306A, 0x0068, 0x00}, \
}

#define RES3_REG_ARRAY \
{ \
  {0x6028, 0x2000, 0x00}, \
  {0x602A, 0x14F0, 0x00}, \
  {0x6F12, 0x0040, 0x00}, \
  {0x6F12, 0x0040, 0x00}, \
  {0x602A, 0x192C, 0x00}, \
  {0x6F12, 0x0001, 0x00}, \
  {0x6028, 0x4000, 0x00}, \
  {0x0344, 0x00BC, 0x00}, \
  {0x0346, 0x01DC, 0x00}, \
  {0x0348, 0x0FC3, 0x00}, \
  {0x034A, 0x0A63, 0x00}, \
  {0x034C, 0x0500, 0x00}, \
  {0x034E, 0x02D0, 0x00}, \
  {0x0900, 0x0113, 0x00}, \
  {0x0380, 0x0001, 0x00}, \
  {0x0382, 0x0001, 0x00}, \
  {0x0384, 0x0001, 0x00}, \
  {0x0386, 0x0005, 0x00}, \
  {0x0400, 0x0001, 0x00}, \
  {0x0404, 0x0030, 0x00}, \
  {0x0114, 0x0300, 0x00}, \
  {0x0110, 0x0002, 0x00}, \
  {0x112C, 0x0000, 0x00}, \
  {0x112E, 0x0000, 0x00}, \
  {0x0136, 0x1800, 0x00}, \
  {0x0304, 0x0006, 0x00}, \
  {0x0306, 0x006E, 0x00}, \
  {0x0302, 0x0001, 0x00}, \
  {0x0300, 0x0004, 0x00}, \
  {0x030C, 0x0004, 0x00}, \
  {0x030E, 0x005A, 0x00}, \
  {0x030A, 0x0001, 0x00}, \
  {0x0308, 0x0008, 0x00}, \
  {0x0342, 0x11F0, 0x00}, \
  {0x0340, 0x0428, 0x00}, \
  {0x0202, 0x0200, 0x00}, \
  {0x0200, 0x0400, 0x00}, \
  {0x0B04, 0x0000, 0x00}, \
  {0x0B08, 0x0000, 0x00}, \
  {0x0B00, 0x0080, 0x00}, \
  {0x3B3C, 0x0107, 0x00}, \
  {0x3B34, 0x3030, 0x00}, \
  {0x3B36, 0x3030, 0x00}, \
  {0x3B38, 0x3030, 0x00}, \
  {0x3B3A, 0x3030, 0x00}, \
  {0x306A, 0x0068, 0x00}, \
}

#define RES4_REG_ARRAY \
{ \
  {0x6028, 0x2000, 0x00}, \
  {0x602A, 0x14F0, 0x00}, \
  {0x6F12, 0x0040, 0x00}, \
  {0x6F12, 0x0040, 0x00}, \
  {0x602A, 0x192C, 0x00}, \
  {0x6F12, 0x0001, 0x00}, \
  {0x6028, 0x4000, 0x00}, \
  {0x0344, 0x00BC, 0x00}, \
  {0x0346, 0x01DC, 0x00}, \
  {0x0348, 0x0FC3, 0x00}, \
  {0x034A, 0x0A63, 0x00}, \
  {0x034C, 0x0500, 0x00}, \
  {0x034E, 0x02D0, 0x00}, \
  {0x0900, 0x0113, 0x00}, \
  {0x0380, 0x0001, 0x00}, \
  {0x0382, 0x0001, 0x00}, \
  {0x0384, 0x0001, 0x00}, \
  {0x0386, 0x0005, 0x00}, \
  {0x0400, 0x0001, 0x00}, \
  {0x0404, 0x0030, 0x00}, \
  {0x0114, 0x0300, 0x00}, \
  {0x0110, 0x0002, 0x00}, \
  {0x112C, 0x0000, 0x00}, \
  {0x112E, 0x0000, 0x00}, \
  {0x0136, 0x1800, 0x00}, \
  {0x0304, 0x0006, 0x00}, \
  {0x0306, 0x006E, 0x00}, \
  {0x0302, 0x0001, 0x00}, \
  {0x0300, 0x0004, 0x00}, \
  {0x030C, 0x0004, 0x00}, \
  {0x030E, 0x005A, 0x00}, \
  {0x030A, 0x0001, 0x00}, \
  {0x0308, 0x0008, 0x00}, \
  {0x0342, 0x11F0, 0x00}, \
  {0x0340, 0x031E, 0x00}, \
  {0x0202, 0x0200, 0x00}, \
  {0x0200, 0x0400, 0x00}, \
  {0x0B04, 0x0000, 0x00}, \
  {0x0B08, 0x0000, 0x00}, \
  {0x0B00, 0x0080, 0x00}, \
  {0x3B3C, 0x0107, 0x00}, \
  {0x3B34, 0x3030, 0x00}, \
  {0x3B36, 0x3030, 0x00}, \
  {0x3B38, 0x3030, 0x00}, \
  {0x3B3A, 0x3030, 0x00}, \
  {0x306A, 0x0068, 0x00}, \
}

/* Sensor Handler */
static sensor_lib_t sensor_lib_ptr =
{
  .sensor_slave_info =
  {
    .sensor_name = SENSOR_MODEL,
    .slave_addr = 0x5A,
    .i2c_freq_mode = SENSOR_I2C_MODE_FAST,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .sensor_id_info =
    {
      .sensor_id_reg_addr = 0x0000,
      .sensor_id = 0x30D2,
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
          .delay = 0,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VANA,
          .config_val = GPIO_OUT_HIGH,
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
          .seq_val = CAMERA_GPIO_VDIG,
          .config_val = GPIO_OUT_HIGH,
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
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_RESET,
          .config_val = GPIO_OUT_HIGH,
          .delay = 10,
        },
        {
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_STANDBY,
          .config_val = GPIO_OUT_HIGH,
          .delay = 100,
        },
      },
      .size = 10,
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
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VDIG,
          .config_val = GPIO_OUT_LOW,
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
          .delay = 1,
        },
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VANA,
          .config_val = 0,
          .delay = 0,
        },
      },
      .size_down = 8,
    },
  },
  .sensor_output =
  {
    .output_format = SENSOR_BAYER,
    .connection_mode = SENSOR_MIPI_CSI,
    .raw_output = SENSOR_10_BIT_DIRECT,
#ifndef FLIP_MIRROR
    .filter_arrangement = SENSOR_GRBG,
#else
    .filter_arrangement = SENSOR_GBRG,
#endif
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
    .vert_offset = S5K3M2XM_MAX_INTEGRATION_MARGIN,
  },
  .aec_info =
  {
    .min_gain = S5K3M2XM_MIN_GAIN,
    .max_gain = S5K3M2XM_MAX_GAIN,
    .max_analog_gain = S5K3M2XM_MAX_GAIN,
    .max_linecount = 65535 - S5K3M2XM_MAX_INTEGRATION_MARGIN,
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
      .width = 4208,
      .height = 3120,
    },
    .left_dummy = 8,
    .right_dummy = 8,
    .top_dummy = 8,
    .bottom_dummy = 8,
  },
  .color_level_info =
  {
    .white_level = 1023,
    .r_pedestal = S5K3M2XM_DATA_PEDESTAL,
    .gr_pedestal = S5K3M2XM_DATA_PEDESTAL,
    .gb_pedestal = S5K3M2XM_DATA_PEDESTAL,
    .b_pedestal = S5K3M2XM_DATA_PEDESTAL,
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
  .init_settings_array =
  {
    .reg_settings =
    {
      {
        .reg_setting_a = INIT0_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_WORD_DATA,
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
      /* Res 2 */
      {
        .reg_setting_a = RES2_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_WORD_DATA,
        .delay = 0,
      },
      /* Res 3 */
      {
        .reg_setting_a = RES3_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_WORD_DATA,
        .delay = 0,
      },
      /* Res 4 */
      {
        .reg_setting_a = RES4_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_WORD_DATA,
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
        .x_output = 4208,
        .y_output = 3120,
        .line_length_pclk = 4592,
        .frame_length_lines = 3188,
        .op_pixel_clk = 465000000,
        .binning_factor = 1,
        .min_fps = 7.500,
        .max_fps = 30.0,
        .mode = SENSOR_DEFAULT_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 1.000,
        .is_pdaf_supported = 1,
      },
      /* Res 1 */
      {
        .x_output = 2104,
        .y_output = 1560,
        .line_length_pclk = 4592,
        .frame_length_lines = 3188,
        .op_pixel_clk = 465000000,
        .binning_factor = 1,
        .min_fps = 7.500,
        .max_fps = 30.0,
        .mode = SENSOR_DEFAULT_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 1.000,
        .is_pdaf_supported = 0,
      },
      /* Res 2 */
      {
        .x_output = 1920,
        .y_output = 1080,
        .line_length_pclk = 4592,
        .frame_length_lines = 1596,
        .op_pixel_clk = 465000000,
        .binning_factor = 1,
        .min_fps = 7.500,
        .max_fps = 60.0,
        .mode = SENSOR_HFR_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 1.000,
        .is_pdaf_supported = 0,
      },
      /* Res 3 */
      {
        .x_output = 1280,
        .y_output = 720,
        .line_length_pclk = 4592,
        .frame_length_lines = 1064,
        .op_pixel_clk = 465000000,
        .binning_factor = 1,
        .min_fps = 7.500,
        .max_fps = 90.0,
        .mode = SENSOR_HFR_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 1.000,
        .is_pdaf_supported = 0,
      },
      /* Res 4 */
      {
        .x_output = 1280,
        .y_output = 720,
        .line_length_pclk = 4592,
        .frame_length_lines = 798,
        .op_pixel_clk = 465000000,
        .binning_factor = 1,
        .min_fps = 7.500,
        .max_fps = 120.0,
        .mode = SENSOR_HFR_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 1.000,
        .is_pdaf_supported = 0,
      },
    },
    .size = 5,
  },
  .csi_params =
  {
    .lane_cnt = 4,
    .settle_cnt = 0x1B,
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
    .sensor_calculate_exposure = s5k3m2xm_calculate_exposure,
    .sensor_fill_exposure_array = s5k3m2xm_fill_exposure_array,
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
  .adc_readout_time = 0,
  .sensor_num_fast_aec_frame_skip = 0,
  .noise_coeff = {
    .gradient_S = 3.738032e-06,
    .offset_S = 3.651935e-04,
    .gradient_O = 4.499952e-07,
    .offset_O = -2.968624e-04,
  },
  .pdaf_config = {
#ifndef FLIP_MIRROR
    #include "s5k3m2xm_pdaf.h"
#else
   #include "s5k3m2xm_pdaf_flip_mirror.h"
#endif
  },
};

#endif /* __S5K3M2XM_LIB_H__ */
