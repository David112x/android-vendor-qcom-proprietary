/* imx362_DPHY_lib.h
 *
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


#ifndef __IMX362_LIB_H__
#define __IMX362_LIB_H__

#include "sensor_lib.h"
#define SENSOR_MODEL "imx362"

/* IMX362 Regs */
#define IMX362_DIG_GAIN_GLOBAL_ADDR   0x020E
#define IMX362_DIG_GAIN_R_ADDR        0x0210
#define IMX362_DIG_GAIN_B_ADDR        0x0212
#define IMX362_DIG_GAIN_GB_ADDR       0x0214
#define IMX362_EXP_RATIO_ADDR         0x0222
#define IMX362_ABS_GAIN_R_WORD_ADDR   0x0B90
#define IMX362_ABS_GAIN_B_WORD_ADDR   0x0B92

#define ORIENTATION_ADDR 0x0101
#define HDR_EN_ADDR 0x0220
#define HDR_BINNING_ADDR 0x0221
#define X_ADD_STA_ADDR 0x0344
#define Y_ADD_STA_ADDR 0x0346
#define X_ADD_END_ADDR 0x0348
#define Y_ADD_END_ADDR 0x034A
#define X_OUT_SIZE_ADDR 0x034C
#define Y_OUT_SIZE_ADDR 0x034E
#define X_EVN_INC_ADDR 0x0381
#define X_ODD_INC_ADDR 0x0383
#define Y_EVN_INC_ADDR 0x0385
#define Y_ODD_INC_ADDR 0x0387
#define SCALE_MODE_ADDR 0x0401
#define SCALE_M_ADDR 0x0404
#define DIG_CROP_X_OFFSET_ADDR 0x0408
#define DIG_CROP_Y_OFFSET_ADDR 0x040A
#define DIG_CROP_IMAGE_WIDTH_ADDR 0x040C
#define DIG_CROP_IMAGE_HEIGHT_ADDR 0x040E
#define BINNING_MODE_ADDR 0x0900
#define BINNING_TYPE_ADDR 0x0901

/* IMX362 CONSTANTS */
#define IMX362_MAX_INTEGRATION_MARGIN   10

#define IMX362_DATA_PEDESTAL            0x40 /* 10bit value */

#define IMX362_MIN_AGAIN_REG_VAL        0 /* 1.0x */
#define IMX362_MAX_AGAIN_REG_VAL        448 /* 8.0x */

#define IMX362_MIN_DGAIN_REG_VAL        256 /* 1.0x */
#define IMX362_MAX_DGAIN_REG_VAL        512 /* 2.0x */

#define IMX362_MAX_DGAIN_DECIMATOR      256

/* IMX362 FORMULAS */
#define IMX362_MIN_AGAIN    (512 / (512 - IMX362_MIN_AGAIN_REG_VAL))
#define IMX362_MAX_AGAIN    (512 / (512 - IMX362_MAX_AGAIN_REG_VAL))

#define IMX362_MIN_DGAIN    (IMX362_MIN_DGAIN_REG_VAL / 256)
#define IMX362_MAX_DGAIN    (IMX362_MAX_DGAIN_REG_VAL / 256)

#define IMX362_MIN_GAIN     IMX362_MIN_AGAIN * IMX362_MIN_DGAIN
#define IMX362_MAX_GAIN     IMX362_MAX_AGAIN * IMX362_MAX_DGAIN

#define IMX362_SHORT_COARSE_INT_TIME_ADDR 0x0224
#define IMX362_SHORT_GAIN_ADDR 0x0216

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
  /* Power ON   */ \
  /* Input EXTCLK   */ \
  /* XCLR OFF   */ \
  /* External Clock Setting  */ \
  {0x0136, 0x18, 0x00}, \
  {0x0137, 0x00, 0x00}, \
  /* Global Setting   */ \
  {0x31A3, 0x00, 0x00}, \
  {0x5812, 0x04, 0x00}, \
  {0x5813, 0x04, 0x00}, \
  {0x58D0, 0x08, 0x00}, \
  {0x5F20, 0x01, 0x00}, \
  {0x5FF0, 0x00, 0x00}, \
  {0x5FF1, 0xFE, 0x00}, \
  {0x5FF2, 0x00, 0x00}, \
  {0x5FF3, 0x52, 0x00}, \
  {0x72E8, 0x96, 0x00}, \
  {0x72E9, 0x59, 0x00}, \
  {0x72EA, 0x65, 0x00}, \
  {0x72FB, 0x2C, 0x00}, \
  {0x737E, 0x02, 0x00}, \
  {0x737F, 0x30, 0x00}, \
  {0x7380, 0x28, 0x00}, \
  {0x7381, 0x00, 0x00}, \
  {0x7383, 0x02, 0x00}, \
  {0x7384, 0x00, 0x00}, \
  {0x7385, 0x00, 0x00}, \
  {0x74CC, 0x00, 0x00}, \
  {0x74CD, 0x55, 0x00}, \
  {0x74D2, 0x00, 0x00}, \
  {0x74D3, 0x52, 0x00}, \
  {0x74DA, 0x00, 0x00}, \
  {0x74DB, 0xFE, 0x00}, \
  {0x9333, 0x03, 0x00}, \
  {0x9334, 0x04, 0x00}, \
  {0x9335, 0x05, 0x00}, \
  {0x9346, 0x96, 0x00}, \
  {0x934A, 0x8C, 0x00}, \
  {0x9352, 0xAA, 0x00}, \
  {0xB0B6, 0x05, 0x00}, \
  {0xB0B7, 0x05, 0x00}, \
  {0xB0B9, 0x05, 0x00}, \
  {0xBC88, 0x06, 0x00}, \
  {0xBC89, 0xD8, 0x00}, \
  /* LRC_COR_BYPASS   */ \
  {0x30F2, 0x01, 0x00}, \
}

#define RES0_REG_ARRAY \
{ \
  /* reg_M4_1    */ \
  /* Mode4 Full resolution 30 FPS */ \
  /* H: 4032   */ \
  /* V: 3024   */ \
  /* Mode Setting   */ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0220, 0x00, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0340, 0x0C, 0x00}, \
  {0x0341, 0x40, 0x00}, \
  {0x0342, 0x22, 0x00}, \
  {0x0343, 0xE0, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x00, 0x00}, \
  {0x0901, 0x11, 0x00}, \
  {0x30F4, 0x02, 0x00}, \
  {0x30F5, 0x80, 0x00}, \
  {0x30F6, 0x00, 0x00}, \
  {0x30F7, 0x14, 0x00}, \
  {0x31A0, 0x00, 0x00}, \
  {0x31A5, 0x01, 0x00}, \
  {0x31A6, 0x00, 0x00}, \
  {0x560F, 0xBE, 0x00}, \
  /* Output Size Setting  */ \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x00, 0x00}, \
  {0x0347, 0x00, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0xBF, 0x00}, \
  {0x034A, 0x0B, 0x00}, \
  {0x034B, 0xCF, 0x00}, \
  {0x034C, 0x0F, 0x00}, \
  {0x034D, 0xC0, 0x00}, \
  {0x034E, 0x0B, 0x00}, \
  {0x034F, 0xD0, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x00, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x00, 0x00}, \
  {0x040C, 0x0F, 0x00}, \
  {0x040D, 0xC0, 0x00}, \
  {0x040E, 0x0B, 0x00}, \
  {0x040F, 0xD0, 0x00}, \
  /* Clock Setting   */ \
  {0x0301, 0x03, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0xD2, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x01, 0x00}, \
  {0x030D, 0x04, 0x00}, \
  {0x030E, 0x00, 0x00}, \
  {0x030F, 0xDB, 0x00}, \
  {0x0310, 0x01, 0x00}, \
}

#define RES1_REG_ARRAY \
{ \
  /* reg_M4_2    */ \
  /* Mode4 4k2k 30 FPS  */ \
  /* H: 4032   */ \
  /* V: 2016   */ \
  /* Mode Setting   */ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0220, 0x00, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0340, 0x09, 0x00}, \
  {0x0341, 0x56, 0x00}, \
  {0x0342, 0x16, 0x00}, \
  {0x0343, 0xE0, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x00, 0x00}, \
  {0x0901, 0x11, 0x00}, \
  {0x30F4, 0x01, 0x00}, \
  {0x30F5, 0xF4, 0x00}, \
  {0x30F6, 0x00, 0x00}, \
  {0x30F7, 0x14, 0x00}, \
  {0x31A0, 0x00, 0x00}, \
  {0x31A5, 0x01, 0x00}, \
  {0x31A6, 0x00, 0x00}, \
  {0x560F, 0xBE, 0x00}, \
  /* Output Size Setting  */ \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x01, 0x00}, \
  {0x0347, 0xF8, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0xBF, 0x00}, \
  {0x034A, 0x09, 0x00}, \
  {0x034B, 0xD7, 0x00}, \
  {0x034C, 0x0F, 0x00}, \
  {0x034D, 0xC0, 0x00}, \
  {0x034E, 0x07, 0x00}, \
  {0x034F, 0xE0, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x00, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x00, 0x00}, \
  {0x040C, 0x0F, 0x00}, \
  {0x040D, 0xC0, 0x00}, \
  {0x040E, 0x07, 0x00}, \
  {0x040F, 0xE0, 0x00}, \
  /* Clock Setting   */ \
  {0x0301, 0x03, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0x69, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x01, 0x00}, \
  {0x030D, 0x04, 0x00}, \
  {0x030E, 0x00, 0x00}, \
  {0x030F, 0xBF, 0x00}, \
  {0x0310, 0x01, 0x00}, \
}

#define RES2_REG_ARRAY \
{ \
  /* reg_M4_3_3    */ \
  /* Mode4 1080p 30 FPS  */ \
  /* H: 1920   */ \
  /* V: 1080   */ \
  /* Mode Setting   */ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0220, 0x00, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0340, 0x05, 0x00}, \
  {0x0341, 0x10, 0x00}, \
  {0x0342, 0x15, 0x00}, \
  {0x0343, 0x18, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x01, 0x00}, \
  {0x0901, 0x22, 0x00}, \
  {0x30F4, 0x02, 0x00}, \
  {0x30F5, 0x58, 0x00}, \
  {0x30F6, 0x00, 0x00}, \
  {0x30F7, 0x14, 0x00}, \
  {0x31A0, 0x00, 0x00}, \
  {0x31A5, 0x00, 0x00}, \
  {0x31A6, 0x00, 0x00}, \
  {0x560F, 0xFF, 0x00}, \
  /* Output Size Setting  */ \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x60, 0x00}, \
  {0x0346, 0x01, 0x00}, \
  {0x0347, 0xB0, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0x5F, 0x00}, \
  {0x034A, 0x0A, 0x00}, \
  {0x034B, 0x1F, 0x00}, \
  {0x034C, 0x07, 0x00}, \
  {0x034D, 0x80, 0x00}, \
  {0x034E, 0x04, 0x00}, \
  {0x034F, 0x38, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x00, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x00, 0x00}, \
  {0x040C, 0x07, 0x00}, \
  {0x040D, 0x80, 0x00}, \
  {0x040E, 0x04, 0x00}, \
  {0x040F, 0x38, 0x00}, \
  /* Clock Setting   */ \
  {0x0301, 0x05, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0x58, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x01, 0x00}, \
  {0x030D, 0x04, 0x00}, \
  {0x030E, 0x00, 0x00}, \
  {0x030F, 0xBF, 0x00}, \
  {0x0310, 0x01, 0x00}, \
}

#define RES3_REG_ARRAY \
{ \
  /* reg_M4_3_2    */ \
  /* Mode4 1080p@60 FPS  */ \
  /* H: 1920   */ \
  /* V: 1080   */ \
  /* Mode Setting   */ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0220, 0x00, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0340, 0x05, 0x00}, \
  {0x0341, 0x10, 0x00}, \
  {0x0342, 0x15, 0x00}, \
  {0x0343, 0x18, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x01, 0x00}, \
  {0x0901, 0x22, 0x00}, \
  {0x30F4, 0x02, 0x00}, \
  {0x30F5, 0x58, 0x00}, \
  {0x30F6, 0x00, 0x00}, \
  {0x30F7, 0x14, 0x00}, \
  {0x31A0, 0x00, 0x00}, \
  {0x31A5, 0x00, 0x00}, \
  {0x31A6, 0x00, 0x00}, \
  {0x560F, 0xFF, 0x00}, \
  /* Output Size Setting  */ \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x60, 0x00}, \
  {0x0346, 0x01, 0x00}, \
  {0x0347, 0xB0, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0x5F, 0x00}, \
  {0x034A, 0x0A, 0x00}, \
  {0x034B, 0x1F, 0x00}, \
  {0x034C, 0x07, 0x00}, \
  {0x034D, 0x80, 0x00}, \
  {0x034E, 0x04, 0x00}, \
  {0x034F, 0x38, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x00, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x00, 0x00}, \
  {0x040C, 0x07, 0x00}, \
  {0x040D, 0x80, 0x00}, \
  {0x040E, 0x04, 0x00}, \
  {0x040F, 0x38, 0x00}, \
  /* Clock Setting   */ \
  {0x0301, 0x03, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0x69, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x01, 0x00}, \
  {0x030D, 0x04, 0x00}, \
  {0x030E, 0x00, 0x00}, \
  {0x030F, 0xBF, 0x00}, \
  {0x0310, 0x01, 0x00}, \
}

#define RES4_REG_ARRAY \
{ \
  /* reg_M4_3_1    */ \
  /* Mode4 1080p 120 FPS  */ \
  /* H: 1920   */ \
  /* V: 1080   */ \
  /* Mode Setting   */ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0220, 0x00, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0340, 0x05, 0x00}, \
  {0x0341, 0x10, 0x00}, \
  {0x0342, 0x15, 0x00}, \
  {0x0343, 0x18, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x01, 0x00}, \
  {0x0901, 0x22, 0x00}, \
  {0x30F4, 0x02, 0x00}, \
  {0x30F5, 0x58, 0x00}, \
  {0x30F6, 0x00, 0x00}, \
  {0x30F7, 0x14, 0x00}, \
  {0x31A0, 0x00, 0x00}, \
  {0x31A5, 0x00, 0x00}, \
  {0x31A6, 0x00, 0x00}, \
  {0x560F, 0xFF, 0x00}, \
  /* Output Size Setting  */ \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x60, 0x00}, \
  {0x0346, 0x01, 0x00}, \
  {0x0347, 0xB0, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0x5F, 0x00}, \
  {0x034A, 0x0A, 0x00}, \
  {0x034B, 0x1F, 0x00}, \
  {0x034C, 0x07, 0x00}, \
  {0x034D, 0x80, 0x00}, \
  {0x034E, 0x04, 0x00}, \
  {0x034F, 0x38, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x00, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x00, 0x00}, \
  {0x040C, 0x07, 0x00}, \
  {0x040D, 0x80, 0x00}, \
  {0x040E, 0x04, 0x00}, \
  {0x040F, 0x38, 0x00}, \
  /* Clock Setting   */ \
  {0x0301, 0x03, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0xD2, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x01, 0x00}, \
  {0x030D, 0x04, 0x00}, \
  {0x030E, 0x00, 0x00}, \
  {0x030F, 0xBF, 0x00}, \
  {0x0310, 0x01, 0x00}, \
}

#define RES5_REG_ARRAY \
{ \
  /* reg_M4_5    */ \
  /* Mode4 HDR mode, full */ \
  /* H: 4032   */ \
  /* V: 3024   */ \
  /* Mode Setting   */ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0220, 0x61, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0340, 0x0C, 0x00}, \
  {0x0341, 0x40, 0x00}, \
  {0x0342, 0x22, 0x00}, \
  {0x0343, 0xE0, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x00, 0x00}, \
  {0x0901, 0x11, 0x00}, \
  {0x30F4, 0x02, 0x00}, \
  {0x30F5, 0x80, 0x00}, \
  {0x30F6, 0x00, 0x00}, \
  {0x30F7, 0x14, 0x00}, \
  {0x31A0, 0x00, 0x00}, \
  {0x31A5, 0x01, 0x00}, \
  {0x31A6, 0x00, 0x00}, \
  {0x560F, 0xBE, 0x00}, \
  /* Output Size Setting  */ \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x00, 0x00}, \
  {0x0347, 0x00, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0xBF, 0x00}, \
  {0x034A, 0x0B, 0x00}, \
  {0x034B, 0xCF, 0x00}, \
  {0x034C, 0x0F, 0x00}, \
  {0x034D, 0xC0, 0x00}, \
  {0x034E, 0x0B, 0x00}, \
  {0x034F, 0xD0, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x00, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x00, 0x00}, \
  {0x040C, 0x0F, 0x00}, \
  {0x040D, 0xC0, 0x00}, \
  {0x040E, 0x0B, 0x00}, \
  {0x040F, 0xD0, 0x00}, \
  /* Clock Setting   */ \
  {0x0301, 0x03, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0xD2, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x01, 0x00}, \
  {0x030D, 0x04, 0x00}, \
  {0x030E, 0x00, 0x00}, \
  {0x030F, 0xDB, 0x00}, \
  {0x0310, 0x01, 0x00}, \
}

#define RES6_REG_ARRAY \
{ \
  /* reg_M4_6    */ \
  /* Mode4 HDR mode, 4K */ \
  /* H: 4032   */ \
  /* V: 2016   */ \
  /* Mode Setting   */ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0220, 0x61, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0340, 0x09, 0x00}, \
  {0x0341, 0x56, 0x00}, \
  {0x0342, 0x22, 0x00}, \
  {0x0343, 0xE0, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x00, 0x00}, \
  {0x0901, 0x11, 0x00}, \
  {0x30F4, 0x02, 0x00}, \
  {0x30F5, 0x80, 0x00}, \
  {0x30F6, 0x00, 0x00}, \
  {0x30F7, 0x14, 0x00}, \
  {0x31A0, 0x00, 0x00}, \
  {0x31A5, 0x01, 0x00}, \
  {0x31A6, 0x00, 0x00}, \
  {0x560F, 0xBE, 0x00}, \
  /* Output Size Setting  */ \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x01, 0x00}, \
  {0x0347, 0xF8, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0xBF, 0x00}, \
  {0x034A, 0x09, 0x00}, \
  {0x034B, 0xD7, 0x00}, \
  {0x034C, 0x0F, 0x00}, \
  {0x034D, 0xC0, 0x00}, \
  {0x034E, 0x07, 0x00}, \
  {0x034F, 0xE0, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x00, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x00, 0x00}, \
  {0x040C, 0x0F, 0x00}, \
  {0x040D, 0xC0, 0x00}, \
  {0x040E, 0x07, 0x00}, \
  {0x040F, 0xE0, 0x00}, \
  /* Clock Setting   */ \
  {0x0301, 0x03, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0xA1, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x01, 0x00}, \
  {0x030D, 0x04, 0x00}, \
  {0x030E, 0x00, 0x00}, \
  {0x030F, 0xBF, 0x00}, \
  {0x0310, 0x01, 0x00}, \
}


/* Sensor Handler */
static sensor_lib_t sensor_lib_ptr =
{
  .sensor_slave_info =
  {
    .sensor_name = SENSOR_MODEL,
    .slave_addr = 0x20,
    .i2c_freq_mode = SENSOR_I2C_MODE_FAST_PLUS,
    .addr_type = CAMERA_I2C_WORD_ADDR,
    .sensor_id_info =
    {
      .sensor_id_reg_addr = 0x0016,
      .sensor_id = 0x0362,
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
          .seq_val = CAMERA_GPIO_VANA,
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
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VDIG,
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
          .delay = 18,
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
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
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
          .seq_type = CAMERA_POW_SEQ_GPIO,
          .seq_val = CAMERA_GPIO_VDIG,
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
          .config_val = GPIO_OUT_LOW,
          .delay = 1,
        },
      },
      .size_down = 7,
    },
  },
  .sensor_output =
  {
    .output_format = SENSOR_BAYER,
    .connection_mode = SENSOR_MIPI_CSI,
    .raw_output = SENSOR_10_BIT_DIRECT,
    .filter_arrangement = SENSOR_RGGB,
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
    .vert_offset = IMX362_MAX_INTEGRATION_MARGIN,
  },
  .aec_info =
  {
    .min_gain = IMX362_MIN_GAIN,
    .max_gain = IMX362_MAX_GAIN,
    .max_analog_gain = IMX362_MAX_AGAIN,
    .max_linecount = 65535 - IMX362_MAX_INTEGRATION_MARGIN,
  },
  .sensor_num_frame_skip = 1,
  .sensor_num_HDR_frame_skip = 2,
  .sensor_max_pipeline_frame_delay = 2,
  .sensor_property =
  {
    .pix_size = 1.4, /* um */
    .sensing_method = SENSOR_SMETHOD_ONE_CHIP_COLOR_AREA_SENSOR,
    .crop_factor = 5.78,
  },
  .pixel_array_size_info =
  {
    .active_array_size =
    {
      .width = 4032,
      .height = 3024,
    },
    .left_dummy = 8,
    .right_dummy = 8,
    .top_dummy = 8,
    .bottom_dummy = 8,
  },
  .color_level_info =
  {
    .white_level = 1023,
    .r_pedestal = IMX362_DATA_PEDESTAL,
    .gr_pedestal = IMX362_DATA_PEDESTAL,
    .gb_pedestal = IMX362_DATA_PEDESTAL,
    .b_pedestal = IMX362_DATA_PEDESTAL,
  },
  .sensor_stream_info_array =
  {
    .sensor_stream_info =
    {
      {
        .vc_cfg_size = 2,
        .vc_cfg =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT,
          },
          {
            .cid = 4,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT_PLAIN16_LSB,
          },
        },
        .pix_data_fmt =
        {
          SENSOR_BAYER,
          SENSOR_META,
        },
      },
      {
        .vc_cfg_size = 1,
        .vc_cfg =
        {
          {
            .cid = 3,
            .dt = CSI_EMBED_DATA,
            .decode_format = CSI_DECODE_8BIT,
          },
        },
        .pix_data_fmt =
        {
          SENSOR_META,
        },
      },
    },
    .size = 2,
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
            {0x0600, 0x0001, 0x00},
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
            {0x0600, 0x0002, 0x00},
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
            {0x0600, 0x0003, 0x00},
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
            {0x0600, 0x0004, 0x00},
          },
          .addr_type = CAMERA_I2C_WORD_ADDR,
          .data_type = CAMERA_I2C_WORD_DATA,
          .delay = 0,
        },
      },
    },
    .size = 5,
    .solid_mode_addr =
    {
      .r_addr = 0x0602,
      .gr_addr = 0x0604,
      .gb_addr = 0x0608,
      .b_addr = 0x0606,
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
      /* Res 6 */
      {
        .reg_setting_a = RES6_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 0,
      },
    },
    .size = 7,
  },
  .out_info_array =
  {
    .out_info =
    {
      /* Res 0 */
      {
        .x_output = 4032,
        .y_output = 3024,
        .line_length_pclk = 8928,
        .frame_length_lines = 3136,
        .op_pixel_clk = 525600000,
        .binning_factor = 1,
        .binning_method = 0,
        .min_fps = 7.500,
        .max_fps = 30.00,
        .mode = SENSOR_DEFAULT_MODE,
        .is_pdaf_supported = 1,
        .min_hbi = 437,
        .min_vbi = 106,
      },
      /* Res 1 */
      {
        .x_output = 4032,
        .y_output = 2016,
        .line_length_pclk = 5856,
        .frame_length_lines = 2390,
        .op_pixel_clk = 458400000,
        .binning_factor = 1,
        .binning_method = 0,
        .min_fps = 7.500,
        .max_fps = 30.00,
        .mode = SENSOR_DEFAULT_MODE,
        .is_pdaf_supported = 1,
        .min_hbi = 1081,
        .min_vbi = 368,
      },
      /* Res 2 */
      {
        .x_output = 1920,
        .y_output = 1080,
        .line_length_pclk = 5400,
        .frame_length_lines = 1296,
        .op_pixel_clk = 458400000,
        .binning_factor = 2,
        .binning_method = 0,
        .min_fps = 7.500,
        .max_fps = 30.17,
        .mode = SENSOR_DEFAULT_MODE,
        .is_pdaf_supported = 1,
        .min_hbi = 7456,
        .min_vbi = 210,
      },
      /* Res 3 */
      {
        .x_output = 1920,
        .y_output = 1080,
        .line_length_pclk = 5400,
        .frame_length_lines = 1296,
        .op_pixel_clk = 458400000,
        .binning_factor = 2,
        .binning_method = 0,
        .min_fps = 7.500,
        .max_fps = 60.01,
        .mode = SENSOR_HFR_MODE | SENSOR_DEFAULT_MODE,
        .is_pdaf_supported = 1,
        .min_hbi = 2795,
        .min_vbi = 210,
      },
      /* Res 4 */
      {
        .x_output = 1920,
        .y_output = 1080,
        .line_length_pclk = 5400,
        .frame_length_lines = 1296,
        .op_pixel_clk = 458400000,
        .binning_factor = 2,
        .binning_method = 0,
        .min_fps = 7.500,
        .max_fps = 120.02,
        .mode = SENSOR_HFR_MODE,
        .is_pdaf_supported = 1,
        .min_hbi = 437,
        .min_vbi = 210,
      },
      /* Res 5 */
      {
        .x_output = 4032,
        .y_output = 3024,
        .line_length_pclk = 8928,
        .frame_length_lines = 3136,
        .op_pixel_clk = 525600000,
        .binning_factor = 1,
        .binning_method = 0,
        .min_fps = 7.500,
        .max_fps = 30.00,
        .mode = SENSOR_RAW_HDR_MODE,
        .is_pdaf_supported = 1,
        .min_hbi = 437,
        .min_vbi = 106,
      },
      /* Res 6 */
      {
        .x_output = 4032,
        .y_output = 2016,
        .line_length_pclk = 8928,
        .frame_length_lines = 2390,
        .op_pixel_clk = 458400000,
        .binning_factor = 1,
        .binning_method = 0,
        .min_fps = 7.500,
        .max_fps = 30.18,
        .mode = SENSOR_RAW_HDR_MODE,
        .is_pdaf_supported = 1,
        .min_hbi = 1052,
        .min_vbi = 368,
      },
    },
    .size = 7,
  },
  .csi_params =
  {
    .lane_cnt = 4,
    .settle_cnt = 0xE,
    .is_csi_3phase = 0,
  },
  .csid_lut_params_array =
  {
    .lut_params =
    {
      /* Res 0 */
      {
        .num_cid = 3,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 4,
            .dt = CSI_RAW10,
#if defined(MSM8952_SENSORS)
            .decode_format = CSI_DECODE_10BIT
#else
            .decode_format = CSI_DECODE_10BIT_PLAIN16_LSB
#endif
          },
          {
            .cid = 1,
            .dt = CSI_EMBED_DATA,
            .decode_format = CSI_DECODE_8BIT
          },
        },
      },
      /* Res 1 */
      {
        .num_cid = 3,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 4,
            .dt = CSI_RAW10,
#if defined(MSM8952_SENSORS)
            .decode_format = CSI_DECODE_10BIT
#else
            .decode_format = CSI_DECODE_10BIT_PLAIN16_LSB
#endif
          },
          {
            .cid = 1,
            .dt = CSI_EMBED_DATA,
            .decode_format = CSI_DECODE_8BIT
          },
        },
      },
      /* Res 2 */
      {
        .num_cid = 3,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 4,
            .dt = CSI_RAW10,
#if defined(MSM8952_SENSORS)
            .decode_format = CSI_DECODE_10BIT
#else
            .decode_format = CSI_DECODE_10BIT_PLAIN16_LSB
#endif
          },
          {
            .cid = 1,
            .dt = CSI_EMBED_DATA,
            .decode_format = CSI_DECODE_8BIT
          },
        },
      },
      /* Res 3 */
      {
        .num_cid = 3,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 4,
            .dt = CSI_RAW10,
#if defined(MSM8952_SENSORS)
            .decode_format = CSI_DECODE_10BIT
#else
            .decode_format = CSI_DECODE_10BIT_PLAIN16_LSB
#endif
          },
          {
            .cid = 1,
            .dt = CSI_EMBED_DATA,
            .decode_format = CSI_DECODE_8BIT
          },
        },
      },
      /* Res 4 */
      {
        .num_cid = 3,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 4,
            .dt = CSI_RAW10,
#if defined(MSM8952_SENSORS)
            .decode_format = CSI_DECODE_10BIT
#else
            .decode_format = CSI_DECODE_10BIT_PLAIN16_LSB
#endif
          },
          {
            .cid = 1,
            .dt = CSI_EMBED_DATA,
            .decode_format = CSI_DECODE_8BIT
          },
        },
      },
      /* Res 5 */
      {
        .num_cid = 3,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 4,
            .dt = CSI_RAW10,
#if defined(MSM8952_SENSORS)
            .decode_format = CSI_DECODE_10BIT
#else
            .decode_format = CSI_DECODE_10BIT_PLAIN16_LSB
#endif
          },
          {
            .cid = 1,
            .dt = CSI_EMBED_DATA,
            .decode_format = CSI_DECODE_8BIT
          },
        },
      },
      /* Res 6 */
      {
        .num_cid = 3,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 4,
            .dt = CSI_RAW10,
#if defined(MSM8952_SENSORS)
            .decode_format = CSI_DECODE_10BIT
#else
            .decode_format = CSI_DECODE_10BIT_PLAIN16_LSB
#endif
          },
          {
            .cid = 1,
            .dt = CSI_EMBED_DATA,
            .decode_format = CSI_DECODE_8BIT
          },
        },
      },
    },
    .size = 7,
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
      /* Res 6 */
      {
        .top_crop = 0,
        .bottom_crop = 0,
        .left_crop = 0,
        .right_crop = 0,
      },
    },
    .size = 7,
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
        .width = 4032*2,
        .height = 756,
        .stats_type = PD_STATS,
        .dt = CSI_RAW10,
      },
    },
    .size = 1,
  },
  .sensor_capability = SENSOR_FEATURE_ZIGZAG_HDR,
  .awb_func_table =
  {
    .sensor_fill_awb_array = sensor_fill_awb_array,
    .awb_table_size = 2,
  },
  .parse_RDI_stats =
  {
    .parse_VHDR_stats = NULL,
    .parse_PDAF_stats = NULL,
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
    #include "imx362_pdaf.h"
  },
};

#endif /* __IMX362_LIB_H__ */
