/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __IMX378_LIB_H__
#define __IMX378_LIB_H__

#include "sensor_lib.h"
#define SENSOR_MODEL "imx378"

/* IMX378 REGISTERS */
#define IMX378_DIG_GAIN_GR_ADDR           0x020E
#define IMX378_DIG_GAIN_R_ADDR            0x0210
#define IMX378_DIG_GAIN_B_ADDR            0x0212
#define IMX378_DIG_GAIN_GB_ADDR           0x0214
#define IMX378_EXP_RATIO_ADDR             0x0222
#define IMX378_ABS_GAIN_R_WORD_ADDR       0x0B90
#define IMX378_ABS_GAIN_B_WORD_ADDR       0x0B92
#define IMX378_SHORT_COARSE_INT_TIME_ADDR 0x0224
#define IMX378_SHORT_GAIN_ADDR            0x0216

#define PD_ENABLE_ADDR                    0x3E37
#define AREA_MODE_ADDR                    0x38A3
#define PD_AREA_X_OFFSET_ADDR             0x38A4
#define PD_AREA_Y_OFFSET_ADDR             0x38A6
#define PD_AREA_WIDTH_ADDR                0x38A8
#define PD_AREA_HEIGHT_ADDR               0x38AA
#define FLEX_AREA_EN_ADDR                 0x38AC
#define FLEX_AREA_XSTA0_ADDR              0x38B4
#define CL_SHIFT_ADDR                     0x38F4


/* IMX378 CONSTANTS */
#define IMX378_MAX_INTEGRATION_MARGIN   10

/* STATS DATA TYPE */
#define IMX378_CSI_HDR_ISTATS           0x35
#define IMX378_CSI_PD_ISTATS            0x36

#define IMX378_DATA_PEDESTAL            0x40    /* 10bit value */

#define IMX378_MIN_AGAIN_REG_VAL        0       /* 1.0x */

/* IMX378_MAX_AGAIN_REG_VAL is 978=0x3D2
 * Maximum Analog Gain: 22.261 times
 * only use Maximum Analog Gain 16 times here.
 */
#define IMX378_MAX_AGAIN_REG_VAL        960     /* 16.0x */

#define IMX378_MIN_DGAIN_REG_VAL        256     /* 1.0x */
#define IMX378_MAX_DGAIN_REG_VAL        511     /* 2.0x */

#define IMX378_MAX_DGAIN_DECIMATOR      256

/* IMX378 FORMULAS */
#define IMX378_MIN_AGAIN    (1024 / (1024 - IMX378_MIN_AGAIN_REG_VAL))
#define IMX378_MAX_AGAIN    (1024 / (1024 - IMX378_MAX_AGAIN_REG_VAL))

#define IMX378_MIN_DGAIN    (IMX378_MIN_DGAIN_REG_VAL / 256)
#define IMX378_MAX_DGAIN    (IMX378_MAX_DGAIN_REG_VAL / 256)

#define IMX378_MIN_GAIN     IMX378_MIN_AGAIN * IMX378_MIN_DGAIN
#define IMX378_MAX_GAIN     IMX378_MAX_AGAIN * IMX378_MAX_DGAIN
#define FULL_SIZE_WIDTH     4056
#define FULL_SIZE_HEIGHT    3040

#define PD_FLEX_WIN_NUM 6

#define PD_AREA_MODE  2 /* AREA_MODE: 0 - 16x12, 1 - 8x6, 2 - Flexible */
#define PD_WIN_X (PD_AREA_MODE==2 ? 1 : (16/(PD_AREA_MODE+1)))
#define PD_WIN_Y (PD_AREA_MODE==2 ? PD_FLEX_WIN_NUM : (12/(PD_AREA_MODE+1)))


/* PD area settings for full-size */
#define PD_AREA_WIDTH     (252*(PD_AREA_MODE+1))
#define PD_AREA_HEIGHT    (251*(PD_AREA_MODE+1))
#define PD_AREA_X_OFFSET  12
#define PD_AREA_Y_OFFSET  16

#define CL_SHIFT 3 /* confidence level shift to prevent overflow */
#define CL_SHIFT_BINNING 2

#define PD_WIN_X_S_0 1420
#define PD_WIN_Y_S_0 1064
#define PD_WIN_X_E_0 2339
#define PD_WIN_Y_E_0 1748
#define PD_WIN_X_S_1 1716
#define PD_WIN_Y_S_1 1064
#define PD_WIN_X_E_1 2635
#define PD_WIN_Y_E_1 1748
#define PD_WIN_X_S_2 1420
#define PD_WIN_Y_S_2 1291
#define PD_WIN_X_E_2 2339
#define PD_WIN_Y_E_2 1975
#define PD_WIN_X_S_3 1716
#define PD_WIN_Y_S_3 1291
#define PD_WIN_X_E_3 2635
#define PD_WIN_Y_E_3 1975
#define PD_WIN_X_S_4 1420
#define PD_WIN_Y_S_4 1064
#define PD_WIN_X_E_4 2635
#define PD_WIN_Y_E_4 1975
#define PD_WIN_X_S_5 1217
#define PD_WIN_Y_S_5 912
#define PD_WIN_X_E_5 2838
#define PD_WIN_Y_E_5 2127


#define SCALE_X(n) (n/(float)FULL_SIZE_WIDTH)
#define SCALE_Y(n) (n/(float)FULL_SIZE_HEIGHT)

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
  {0x0136, 0x18, 0x00}, \
  {0x0137, 0x00, 0x00}, \
  {0xe000, 0x00, 0x00}, \
  {0x4AE9, 0x18, 0x00}, \
  {0x4AEA, 0x08, 0x00}, \
  {0xF61C, 0x04, 0x00}, \
  {0xF61E, 0x04, 0x00}, \
  {0x4AE9, 0x21, 0x00}, \
  {0x4AEA, 0x80, 0x00}, \
  {0x38a8, 0x1F, 0x00}, \
  {0x38a9, 0xFF, 0x00}, \
  {0x38aa, 0x1F, 0x00}, \
  {0x38ab, 0xFF, 0x00}, \
  {0x5d13, 0xC3, 0x00}, \
  {0x5d14, 0x58, 0x00}, \
  {0x5d15, 0xA3, 0x00}, \
  {0x5d16, 0x1D, 0x00}, \
  {0x5d17, 0x65, 0x00}, \
  {0x5d18, 0x8C, 0x00}, \
  {0x5d1a, 0x06, 0x00}, \
  {0x5d1b, 0xA9, 0x00}, \
  {0x5d1c, 0x45, 0x00}, \
  {0x5d1d, 0x3A, 0x00}, \
  {0x5d1e, 0xAB, 0x00}, \
  {0x5d1f, 0x15, 0x00}, \
  {0x5d21, 0x0E, 0x00}, \
  {0x5d22, 0x52, 0x00}, \
  {0x5d23, 0xAA, 0x00}, \
  {0x5d24, 0x7D, 0x00}, \
  {0x5d25, 0x57, 0x00}, \
  {0x5d26, 0xA8, 0x00}, \
  {0x5d37, 0x5A, 0x00}, \
  {0x5d38, 0x5A, 0x00}, \
  {0x7B75, 0x0E, 0x00}, \
  {0x7B76, 0x0B, 0x00}, \
  {0x7B77, 0x08, 0x00}, \
  {0x7B78, 0x0A, 0x00}, \
  {0x7B79, 0x47, 0x00}, \
  {0x7B7C, 0x00, 0x00}, \
  {0x7B7D, 0x00, 0x00}, \
  {0x9004, 0x03, 0x00}, \
  {0x9200, 0x50, 0x00}, \
  {0x9201, 0x6C, 0x00}, \
  {0x9202, 0x71, 0x00}, \
  {0x9203, 0x00, 0x00}, \
  {0x9204, 0x71, 0x00}, \
  {0x9205, 0x01, 0x00}, \
  {0x9371, 0x6A, 0x00}, \
  {0x9373, 0x6A, 0x00}, \
  {0x9375, 0x64, 0x00}, \
  {0x3D8A, 0x01, 0x00}, \
  {0x4421, 0x08, 0x00}, \
  {0x7B3B, 0x01, 0x00}, \
  {0x7B4C, 0x00, 0x00}, \
  {0x9905, 0x00, 0x00}, \
  {0x9907, 0x00, 0x00}, \
  {0x9909, 0x00, 0x00}, \
  {0x990B, 0x00, 0x00}, \
  {0x9944, 0x3C, 0x00}, \
  {0x9947, 0x3C, 0x00}, \
  {0x994A, 0x8C, 0x00}, \
  {0x994B, 0x50, 0x00}, \
  {0x994C, 0x1B, 0x00}, \
  {0x994D, 0x8C, 0x00}, \
  {0x994E, 0x50, 0x00}, \
  {0x994F, 0x1B, 0x00}, \
  {0x9950, 0x8C, 0x00}, \
  {0x9951, 0x1B, 0x00}, \
  {0x9952, 0x0A, 0x00}, \
  {0x9953, 0x8C, 0x00}, \
  {0x9954, 0x1B, 0x00}, \
  {0x9955, 0x0A, 0x00}, \
  {0x9A13, 0x04, 0x00}, \
  {0x9A14, 0x04, 0x00}, \
  {0x9A19, 0x00, 0x00}, \
  {0x9A1C, 0x04, 0x00}, \
  {0x9A1D, 0x04, 0x00}, \
  {0x9A26, 0x05, 0x00}, \
  {0x9A27, 0x05, 0x00}, \
  {0x9A2C, 0x01, 0x00}, \
  {0x9A2D, 0x03, 0x00}, \
  {0x9A2F, 0x05, 0x00}, \
  {0x9A30, 0x05, 0x00}, \
  {0x9A41, 0x00, 0x00}, \
  {0x9A46, 0x00, 0x00}, \
  {0x9A47, 0x00, 0x00}, \
  {0x9C17, 0x35, 0x00}, \
  {0x9C1D, 0x31, 0x00}, \
  {0x9C29, 0x50, 0x00}, \
  {0x9C3B, 0x2F, 0x00}, \
  {0x9C41, 0x6B, 0x00}, \
  {0x9C47, 0x2D, 0x00}, \
  {0x9C4D, 0x40, 0x00}, \
  {0x9C6B, 0x00, 0x00}, \
  {0x9C71, 0xC8, 0x00}, \
  {0x9C73, 0x32, 0x00}, \
  {0x9C75, 0x04, 0x00}, \
  {0x9C7D, 0x2D, 0x00}, \
  {0x9C83, 0x40, 0x00}, \
  {0x9C94, 0x3F, 0x00}, \
  {0x9C95, 0x3F, 0x00}, \
  {0x9C96, 0x3F, 0x00}, \
  {0x9C97, 0x00, 0x00}, \
  {0x9C98, 0x00, 0x00}, \
  {0x9C99, 0x00, 0x00}, \
  {0x9C9A, 0x3F, 0x00}, \
  {0x9C9B, 0x3F, 0x00}, \
  {0x9C9C, 0x3F, 0x00}, \
  {0x9CA0, 0x0F, 0x00}, \
  {0x9CA1, 0x0F, 0x00}, \
  {0x9CA2, 0x0F, 0x00}, \
  {0x9CA3, 0x00, 0x00}, \
  {0x9CA4, 0x00, 0x00}, \
  {0x9CA5, 0x00, 0x00}, \
  {0x9CA6, 0x1E, 0x00}, \
  {0x9CA7, 0x1E, 0x00}, \
  {0x9CA8, 0x1E, 0x00}, \
  {0x9CA9, 0x00, 0x00}, \
  {0x9CAA, 0x00, 0x00}, \
  {0x9CAB, 0x00, 0x00}, \
  {0x9CAC, 0x09, 0x00}, \
  {0x9CAD, 0x09, 0x00}, \
  {0x9CAE, 0x09, 0x00}, \
  {0x9CBD, 0x50, 0x00}, \
  {0x9CBF, 0x50, 0x00}, \
  {0x9CC1, 0x50, 0x00}, \
  {0x9CC3, 0x40, 0x00}, \
  {0x9CC5, 0x40, 0x00}, \
  {0x9CC7, 0x40, 0x00}, \
  {0x9CC9, 0x0A, 0x00}, \
  {0x9CCB, 0x0A, 0x00}, \
  {0x9CCD, 0x0A, 0x00}, \
  {0x9D17, 0x35, 0x00}, \
  {0x9D1D, 0x31, 0x00}, \
  {0x9D29, 0x50, 0x00}, \
  {0x9D3B, 0x2F, 0x00}, \
  {0x9D41, 0x6B, 0x00}, \
  {0x9D47, 0x42, 0x00}, \
  {0x9D4D, 0x5A, 0x00}, \
  {0x9D6B, 0x00, 0x00}, \
  {0x9D71, 0xC8, 0x00}, \
  {0x9D73, 0x32, 0x00}, \
  {0x9D75, 0x04, 0x00}, \
  {0x9D7D, 0x42, 0x00}, \
  {0x9D83, 0x5A, 0x00}, \
  {0x9D94, 0x3F, 0x00}, \
  {0x9D95, 0x3F, 0x00}, \
  {0x9D96, 0x3F, 0x00}, \
  {0x9D97, 0x00, 0x00}, \
  {0x9D98, 0x00, 0x00}, \
  {0x9D99, 0x00, 0x00}, \
  {0x9D9A, 0x3F, 0x00}, \
  {0x9D9B, 0x3F, 0x00}, \
  {0x9D9C, 0x3F, 0x00}, \
  {0x9D9D, 0x1F, 0x00}, \
  {0x9D9E, 0x1F, 0x00}, \
  {0x9D9F, 0x1F, 0x00}, \
  {0x9DA0, 0x0F, 0x00}, \
  {0x9DA1, 0x0F, 0x00}, \
  {0x9DA2, 0x0F, 0x00}, \
  {0x9DA3, 0x00, 0x00}, \
  {0x9DA4, 0x00, 0x00}, \
  {0x9DA5, 0x00, 0x00}, \
  {0x9DA6, 0x1E, 0x00}, \
  {0x9DA7, 0x1E, 0x00}, \
  {0x9DA8, 0x1E, 0x00}, \
  {0x9DA9, 0x00, 0x00}, \
  {0x9DAA, 0x00, 0x00}, \
  {0x9DAB, 0x00, 0x00}, \
  {0x9DAC, 0x09, 0x00}, \
  {0x9DAD, 0x09, 0x00}, \
  {0x9DAE, 0x09, 0x00}, \
  {0x9DC9, 0x0A, 0x00}, \
  {0x9DCB, 0x0A, 0x00}, \
  {0x9DCD, 0x0A, 0x00}, \
  {0x9E17, 0x35, 0x00}, \
  {0x9E1D, 0x31, 0x00}, \
  {0x9E29, 0x50, 0x00}, \
  {0x9E3B, 0x2F, 0x00}, \
  {0x9E41, 0x6B, 0x00}, \
  {0x9E47, 0x2D, 0x00}, \
  {0x9E4D, 0x40, 0x00}, \
  {0x9E6B, 0x00, 0x00}, \
  {0x9E71, 0xC8, 0x00}, \
  {0x9E73, 0x32, 0x00}, \
  {0x9E75, 0x04, 0x00}, \
  {0x9E94, 0x0F, 0x00}, \
  {0x9E95, 0x0F, 0x00}, \
  {0x9E96, 0x0F, 0x00}, \
  {0x9E97, 0x00, 0x00}, \
  {0x9E98, 0x00, 0x00}, \
  {0x9E99, 0x00, 0x00}, \
  {0x9EA0, 0x0F, 0x00}, \
  {0x9EA1, 0x0F, 0x00}, \
  {0x9EA2, 0x0F, 0x00}, \
  {0x9EA3, 0x00, 0x00}, \
  {0x9EA4, 0x00, 0x00}, \
  {0x9EA5, 0x00, 0x00}, \
  {0x9EA6, 0x3F, 0x00}, \
  {0x9EA7, 0x3F, 0x00}, \
  {0x9EA8, 0x3F, 0x00}, \
  {0x9EA9, 0x00, 0x00}, \
  {0x9EAA, 0x00, 0x00}, \
  {0x9EAB, 0x00, 0x00}, \
  {0x9EAC, 0x09, 0x00}, \
  {0x9EAD, 0x09, 0x00}, \
  {0x9EAE, 0x09, 0x00}, \
  {0x9EC9, 0x0A, 0x00}, \
  {0x9ECB, 0x0A, 0x00}, \
  {0x9ECD, 0x0A, 0x00}, \
  {0x9F17, 0x35, 0x00}, \
  {0x9F1D, 0x31, 0x00}, \
  {0x9F29, 0x50, 0x00}, \
  {0x9F3B, 0x2F, 0x00}, \
  {0x9F41, 0x6B, 0x00}, \
  {0x9F47, 0x42, 0x00}, \
  {0x9F4D, 0x5A, 0x00}, \
  {0x9F6B, 0x00, 0x00}, \
  {0x9F71, 0xC8, 0x00}, \
  {0x9F73, 0x32, 0x00}, \
  {0x9F75, 0x04, 0x00}, \
  {0x9F94, 0x0F, 0x00}, \
  {0x9F95, 0x0F, 0x00}, \
  {0x9F96, 0x0F, 0x00}, \
  {0x9F97, 0x00, 0x00}, \
  {0x9F98, 0x00, 0x00}, \
  {0x9F99, 0x00, 0x00}, \
  {0x9F9A, 0x2F, 0x00}, \
  {0x9F9B, 0x2F, 0x00}, \
  {0x9F9C, 0x2F, 0x00}, \
  {0x9F9D, 0x00, 0x00}, \
  {0x9F9E, 0x00, 0x00}, \
  {0x9F9F, 0x00, 0x00}, \
  {0x9FA0, 0x0F, 0x00}, \
  {0x9FA1, 0x0F, 0x00}, \
  {0x9FA2, 0x0F, 0x00}, \
  {0x9FA3, 0x00, 0x00}, \
  {0x9FA4, 0x00, 0x00}, \
  {0x9FA5, 0x00, 0x00}, \
  {0x9FA6, 0x1E, 0x00}, \
  {0x9FA7, 0x1E, 0x00}, \
  {0x9FA8, 0x1E, 0x00}, \
  {0x9FA9, 0x00, 0x00}, \
  {0x9FAA, 0x00, 0x00}, \
  {0x9FAB, 0x00, 0x00}, \
  {0x9FAC, 0x09, 0x00}, \
  {0x9FAD, 0x09, 0x00}, \
  {0x9FAE, 0x09, 0x00}, \
  {0x9FC9, 0x0A, 0x00}, \
  {0x9FCB, 0x0A, 0x00}, \
  {0x9FCD, 0x0A, 0x00}, \
  {0xA14B, 0xFF, 0x00}, \
  {0xA151, 0x0C, 0x00}, \
  {0xA153, 0x50, 0x00}, \
  {0xA155, 0x02, 0x00}, \
  {0xA157, 0x00, 0x00}, \
  {0xA1AD, 0xFF, 0x00}, \
  {0xA1B3, 0x0C, 0x00}, \
  {0xA1B5, 0x50, 0x00}, \
  {0xA1B9, 0x00, 0x00}, \
  {0xA24B, 0xFF, 0x00}, \
  {0xA251, 0x10, 0x00}, \
  {0xA253, 0x60, 0x00}, \
  {0xA257, 0x00, 0x00}, \
  {0xA2AD, 0xFF, 0x00}, \
  {0xA2B4, 0x00, 0x00}, \
  {0xA2B9, 0x00, 0x00}, \
  {0xB21F, 0x04, 0x00}, \
  {0xB35C, 0x00, 0x00}, \
  {0xB35E, 0x08, 0x00}, \
  /* PDAF Flexiable Window Enable */ \
  {PD_ENABLE_ADDR, 0x01, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR, PD_AREA_X_OFFSET>>8, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR+1, PD_AREA_X_OFFSET&0xFF, 0x00}, \
  {PD_AREA_Y_OFFSET_ADDR, PD_AREA_Y_OFFSET>>8, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR+1, PD_AREA_Y_OFFSET&0xFF, 0x00}, \
  {PD_AREA_WIDTH_ADDR, PD_AREA_WIDTH>>8, 0x00}, \
  {PD_AREA_WIDTH_ADDR+1, PD_AREA_WIDTH&0xFF, 0x00}, \
  {PD_AREA_HEIGHT_ADDR, PD_AREA_HEIGHT>>8, 0x00}, \
  {PD_AREA_HEIGHT_ADDR+1, PD_AREA_HEIGHT&0xFF, 0x00}, \
  {AREA_MODE_ADDR, PD_AREA_MODE, 0x00}, /* 0-16x12, 1-8x6, 2-Flexible */ \
  /* Flexiable PDAF windows */ \
  {FLEX_AREA_EN_ADDR,   0x01, 0x00}, \
  {FLEX_AREA_EN_ADDR+1, 0x01, 0x00}, \
  {FLEX_AREA_EN_ADDR+2, 0x01, 0x00}, \
  {FLEX_AREA_EN_ADDR+3, 0x01, 0x00}, \
  {FLEX_AREA_EN_ADDR+4, 0x01, 0x00}, \
  {FLEX_AREA_EN_ADDR+5, 0x01, 0x00}, \
  {FLEX_AREA_EN_ADDR+6, 0x01, 0x00}, \
  {FLEX_AREA_EN_ADDR+7, 0x01, 0x00}, \
}

#define RES0_REG_ARRAY \
{ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0342, 0x11, 0x00}, \
  {0x0343, 0xA0, 0x00}, \
  {0x0340, 0x0D, 0x00}, \
  {0x0341, 0xD8, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x00, 0x00}, \
  {0x0347, 0x00, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0xD7, 0x00}, \
  {0x034A, 0x0B, 0x00}, \
  {0x034B, 0xDF, 0x00}, \
  {0x0220, 0x00, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x00, 0x00}, \
  {0x0901, 0x11, 0x00}, \
  {0x0902, 0x02, 0x00}, \
  {0x3140, 0x02, 0x00}, \
  {0x3c00, 0x00, 0x00}, \
  {0x3c01, 0x03, 0x00}, \
  {0x3c02, 0xDC, 0x00}, \
  {0x3F0D, 0x00, 0x00}, \
  {0x5748, 0x07, 0x00}, \
  {0x5749, 0xFF, 0x00}, \
  {0x574a, 0x00, 0x00}, \
  {0x574b, 0x00, 0x00}, \
  {0x7B53, 0x01, 0x00}, \
  {0x9369, 0x5A, 0x00}, \
  {0x936B, 0x55, 0x00}, \
  {0x936D, 0x28, 0x00}, \
  {0x9304, 0x00, 0x00}, \
  {0x9305, 0x00, 0x00}, \
  {0x9E9A, 0x2F, 0x00}, \
  {0x9E9B, 0x2F, 0x00}, \
  {0x9E9C, 0x2F, 0x00}, \
  {0x9E9D, 0x00, 0x00}, \
  {0x9E9E, 0x00, 0x00}, \
  {0x9E9F, 0x00, 0x00}, \
  {0xA2A9, 0x60, 0x00}, \
  {0xA2B5, 0x50, 0x00}, \
  {0xA2B7, 0x00, 0x00}, \
  {0x0401, 0x00, 0x00}, \
  {0x0404, 0x00, 0x00}, \
  {0x0405, 0x10, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x00, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x00, 0x00}, \
  {0x040C, 0x0F, 0x00}, \
  {0x040D, 0xD8, 0x00}, \
  {0x040E, 0x0B, 0x00}, \
  {0x040F, 0xE0, 0x00}, \
  {0x034C, 0x0F, 0x00}, \
  {0x034D, 0xD8, 0x00}, \
  {0x034E, 0x0B, 0x00}, \
  {0x034F, 0xE0, 0x00}, \
  {0x0301, 0x05, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0xC8, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x01, 0x00}, \
  {0x030D, 0x02, 0x00}, \
  {0x030E, 0x01, 0x00}, \
  {0x030F, 0x5E, 0x00}, \
  {0x0310, 0x00, 0x00}, \
  {0x0820, 0x12, 0x00}, \
  {0x0821, 0xC0, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3E20, 0x01, 0x00}, \
  {0x3E37, 0x01, 0x00}, \
  {0x3F50, 0x00, 0x00}, \
  {0x3F56, 0x00, 0x00}, \
  {0x3F57, 0xE2, 0x00}, \
  /* PDAF Setting */ \
  {PD_AREA_X_OFFSET_ADDR, PD_AREA_X_OFFSET>>8, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR+1, PD_AREA_X_OFFSET&0xFF, 0x00}, \
  {PD_AREA_Y_OFFSET_ADDR, PD_AREA_Y_OFFSET>>8, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR+1, PD_AREA_Y_OFFSET&0xFF, 0x00}, \
  {PD_AREA_WIDTH_ADDR, PD_AREA_WIDTH>>8, 0x00}, \
  {PD_AREA_WIDTH_ADDR+1, PD_AREA_WIDTH&0xFF, 0x00}, \
  {PD_AREA_HEIGHT_ADDR, PD_AREA_HEIGHT>>8, 0x00}, \
  {PD_AREA_HEIGHT_ADDR+1, PD_AREA_HEIGHT&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR, PD_WIN_X_S_0>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+1, PD_WIN_X_S_0&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+2, PD_WIN_Y_S_0>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+3, PD_WIN_Y_S_0&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+4, PD_WIN_X_E_0>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+5, PD_WIN_X_E_0&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+6, PD_WIN_Y_E_0>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+7, PD_WIN_Y_E_0&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+8, PD_WIN_X_S_1>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+9, PD_WIN_X_S_1&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+10, PD_WIN_Y_S_1>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+11, PD_WIN_Y_S_1&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+12, PD_WIN_X_E_1>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+13, PD_WIN_X_E_1&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+14, PD_WIN_Y_E_1>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+15, PD_WIN_Y_E_1&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+16, PD_WIN_X_S_2>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+17, PD_WIN_X_S_2&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+18, PD_WIN_Y_S_2>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+19, PD_WIN_Y_S_2&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+20, PD_WIN_X_E_2>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+21, PD_WIN_X_E_2&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+22, PD_WIN_Y_E_2>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+23, PD_WIN_Y_E_2&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+24, PD_WIN_X_S_3>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+25, PD_WIN_X_S_3&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+26, PD_WIN_Y_S_3>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+27, PD_WIN_Y_S_3&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+28, PD_WIN_X_E_3>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+29, PD_WIN_X_E_3&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+30, PD_WIN_Y_E_3>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+31, PD_WIN_Y_E_3&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+32, PD_WIN_X_S_4>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+33, PD_WIN_X_S_4&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+34, PD_WIN_Y_S_4>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+35, PD_WIN_Y_S_4&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+36, PD_WIN_X_E_4>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+37, PD_WIN_X_E_4&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+38, PD_WIN_Y_E_4>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+39, PD_WIN_Y_E_4&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+40, PD_WIN_X_S_5>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+41, PD_WIN_X_S_5&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+42, PD_WIN_Y_S_5>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+43, PD_WIN_Y_S_5&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+44, PD_WIN_X_E_5>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+45, PD_WIN_X_E_5&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+46, PD_WIN_Y_E_5>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+47, PD_WIN_Y_E_5&0xFF, 0x00}, \
}

#define RES1_REG_ARRAY \
{ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0342, 0x11, 0x00}, \
  {0x0343, 0xA0, 0x00}, \
  {0x0340, 0x0D, 0x00}, \
  {0x0341, 0xD8, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x01, 0x00}, \
  {0x0347, 0x78, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0xD7, 0x00}, \
  {0x034A, 0x0A, 0x00}, \
  {0x034B, 0x67, 0x00}, \
  {0x0220, 0x00, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x00, 0x00}, \
  {0x0901, 0x11, 0x00}, \
  {0x0902, 0x02, 0x00}, \
  {0x3140, 0x02, 0x00}, \
  {0x3c00, 0x00, 0x00}, \
  {0x3c01, 0x03, 0x00}, \
  {0x3c02, 0xDC, 0x00}, \
  {0x3F0D, 0x00, 0x00}, \
  {0x5748, 0x07, 0x00}, \
  {0x5749, 0xFF, 0x00}, \
  {0x574a, 0x00, 0x00}, \
  {0x574b, 0x00, 0x00}, \
  {0x7B53, 0x01, 0x00}, \
  {0x9369, 0x5A, 0x00}, \
  {0x936B, 0x55, 0x00}, \
  {0x936D, 0x28, 0x00}, \
  {0x9304, 0x00, 0x00}, \
  {0x9305, 0x00, 0x00}, \
  {0x9E9A, 0x2F, 0x00}, \
  {0x9E9B, 0x2F, 0x00}, \
  {0x9E9C, 0x2F, 0x00}, \
  {0x9E9D, 0x00, 0x00}, \
  {0x9E9E, 0x00, 0x00}, \
  {0x9E9F, 0x00, 0x00}, \
  {0xA2A9, 0x60, 0x00}, \
  {0xA2B5, 0x50, 0x00}, \
  {0xA2B7, 0x00, 0x00}, \
  {0x0401, 0x00, 0x00}, \
  {0x0404, 0x00, 0x00}, \
  {0x0405, 0x10, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x00, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x00, 0x00}, \
  {0x040C, 0x0F, 0x00}, \
  {0x040D, 0xD8, 0x00}, \
  {0x040E, 0x08, 0x00}, \
  {0x040F, 0xF0, 0x00}, \
  {0x034C, 0x0F, 0x00}, \
  {0x034D, 0xD8, 0x00}, \
  {0x034E, 0x08, 0x00}, \
  {0x034F, 0xF0, 0x00}, \
  {0x0301, 0x05, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0xC8, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x01, 0x00}, \
  {0x030D, 0x02, 0x00}, \
  {0x030E, 0x01, 0x00}, \
  {0x030F, 0x5E, 0x00}, \
  {0x0310, 0x00, 0x00}, \
  {0x0820, 0x12, 0x00}, \
  {0x0821, 0xC0, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3E20, 0x01, 0x00}, \
  {0x3E37, 0x01, 0x00}, \
  {0x3F50, 0x00, 0x00}, \
  {0x3F56, 0x00, 0x00}, \
  {0x3F57, 0xE2, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR, PD_AREA_X_OFFSET>>8, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR+1, PD_AREA_X_OFFSET&0xFF, 0x00}, \
  {PD_AREA_Y_OFFSET_ADDR, (int)(SCALE_Y(2288)*PD_AREA_Y_OFFSET)>>8, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR+1, (int)(SCALE_Y(2288)*PD_AREA_Y_OFFSET)&0xFF, 0x00},\
  {PD_AREA_WIDTH_ADDR,   PD_AREA_WIDTH>>8, 0x00}, \
  {PD_AREA_WIDTH_ADDR+1, PD_AREA_WIDTH&0xFF, 0x00}, \
  {PD_AREA_HEIGHT_ADDR, (int)(SCALE_Y(2288)*PD_AREA_HEIGHT)>>8, 0x00}, \
  {PD_AREA_HEIGHT_ADDR+1, (int)(SCALE_Y(2288)*PD_AREA_HEIGHT)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR,   PD_WIN_X_S_0>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+1, PD_WIN_X_S_0&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+2, (int)(SCALE_Y(2288)*PD_WIN_Y_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+3, (int)(SCALE_Y(2288)*PD_WIN_Y_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+4, PD_WIN_X_E_0>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+5, PD_WIN_X_E_0&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+6, (int)(SCALE_Y(2288)*PD_WIN_Y_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+7, (int)(SCALE_Y(2288)*PD_WIN_Y_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+8,  PD_WIN_X_S_1>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+9,  PD_WIN_X_S_1&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+10, (int)(SCALE_Y(2288)*PD_WIN_Y_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+11, (int)(SCALE_Y(2288)*PD_WIN_Y_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+12, PD_WIN_X_E_1>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+13, PD_WIN_X_E_1&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+14, (int)(SCALE_Y(2288)*PD_WIN_Y_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+15, (int)(SCALE_Y(2288)*PD_WIN_Y_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+16, PD_WIN_X_S_2>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+17, PD_WIN_X_S_2&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+18, (int)(SCALE_Y(2288)*PD_WIN_Y_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+19, (int)(SCALE_Y(2288)*PD_WIN_Y_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+20, PD_WIN_X_E_2>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+21, PD_WIN_X_E_2&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+22, (int)(SCALE_Y(2288)*PD_WIN_Y_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+23, (int)(SCALE_Y(2288)*PD_WIN_Y_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+24, PD_WIN_X_S_3>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+25, PD_WIN_X_S_3&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+26, (int)(SCALE_Y(2288)*PD_WIN_Y_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+27, (int)(SCALE_Y(2288)*PD_WIN_Y_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+28, PD_WIN_X_E_3>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+29, PD_WIN_X_E_3&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+30, (int)(SCALE_Y(2288)*PD_WIN_Y_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+31, (int)(SCALE_Y(2288)*PD_WIN_Y_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+32, PD_WIN_X_S_4>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+33, PD_WIN_X_S_4&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+34, (int)(SCALE_Y(2288)*PD_WIN_Y_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+35, (int)(SCALE_Y(2288)*PD_WIN_Y_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+36, PD_WIN_X_E_4>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+37, PD_WIN_X_E_4&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+38, (int)(SCALE_Y(2288)*PD_WIN_Y_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+39, (int)(SCALE_Y(2288)*PD_WIN_Y_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+40, PD_WIN_X_S_5>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+41, PD_WIN_X_S_5&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+42, (int)(SCALE_Y(2288)*PD_WIN_Y_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+43, (int)(SCALE_Y(2288)*PD_WIN_Y_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+44, PD_WIN_X_E_5>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+45, PD_WIN_X_E_5&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+46, (int)(SCALE_Y(2288)*PD_WIN_Y_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+47, (int)(SCALE_Y(2288)*PD_WIN_Y_E_5)&0xFF, 0x00}, \
}

#define RES2_REG_ARRAY \
{ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0342, 0x11, 0x00}, \
  {0x0343, 0xA0, 0x00}, \
  {0x0340, 0x0D, 0x00}, \
  {0x0341, 0xD8, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x01, 0x00}, \
  {0x0347, 0x78, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0xD7, 0x00}, \
  {0x034A, 0x0A, 0x00}, \
  {0x034B, 0x67, 0x00}, \
  {0x0220, 0x00, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x00, 0x00}, \
  {0x0901, 0x11, 0x00}, \
  {0x0902, 0x02, 0x00}, \
  {0x3140, 0x02, 0x00}, \
  {0x3c00, 0x00, 0x00}, \
  {0x3c01, 0x03, 0x00}, \
  {0x3c02, 0xDC, 0x00}, \
  {0x3F0D, 0x00, 0x00}, \
  {0x5748, 0x07, 0x00}, \
  {0x5749, 0xFF, 0x00}, \
  {0x574a, 0x00, 0x00}, \
  {0x574b, 0x00, 0x00}, \
  {0x7B53, 0x01, 0x00}, \
  {0x9369, 0x5A, 0x00}, \
  {0x936B, 0x55, 0x00}, \
  {0x936D, 0x28, 0x00}, \
  {0x9304, 0x00, 0x00}, \
  {0x9305, 0x00, 0x00}, \
  {0x9E9A, 0x2F, 0x00}, \
  {0x9E9B, 0x2F, 0x00}, \
  {0x9E9C, 0x2F, 0x00}, \
  {0x9E9D, 0x00, 0x00}, \
  {0x9E9E, 0x00, 0x00}, \
  {0x9E9F, 0x00, 0x00}, \
  {0xA2A9, 0x60, 0x00}, \
  {0xA2B5, 0x50, 0x00}, \
  {0xA2B7, 0x00, 0x00}, \
  {0x0401, 0x00, 0x00}, \
  {0x0404, 0x00, 0x00}, \
  {0x0405, 0x10, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x6C, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x40, 0x00}, \
  {0x040C, 0x0F, 0x00}, \
  {0x040D, 0x00, 0x00}, \
  {0x040E, 0x08, 0x00}, \
  {0x040F, 0x70, 0x00}, \
  {0x034C, 0x0F, 0x00}, \
  {0x034D, 0x00, 0x00}, \
  {0x034E, 0x08, 0x00}, \
  {0x034F, 0x70, 0x00}, \
  {0x0301, 0x05, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0xC8, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x01, 0x00}, \
  {0x030D, 0x02, 0x00}, \
  {0x030E, 0x01, 0x00}, \
  {0x030F, 0x5E, 0x00}, \
  {0x0310, 0x00, 0x00}, \
  {0x0820, 0x12, 0x00}, \
  {0x0821, 0xC0, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3E20, 0x01, 0x00}, \
  {0x3E37, 0x01, 0x00}, \
  {0x3F50, 0x00, 0x00}, \
  {0x3F56, 0x00, 0x00}, \
  {0x3F57, 0xE2, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR, (int)(SCALE_X(3840)*PD_AREA_X_OFFSET)>>8, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR+1, (int)(SCALE_X(3840)*PD_AREA_X_OFFSET)&0xFF, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR, (int)(SCALE_Y(2160)*PD_AREA_Y_OFFSET)>>8, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR+1, (int)(SCALE_Y(2160)*PD_AREA_Y_OFFSET)&0xFF, 0x00},\
  {PD_AREA_WIDTH_ADDR, (int)(SCALE_X(3840)*PD_AREA_WIDTH)>>8, 0x00}, \
  {PD_AREA_WIDTH_ADDR+1, (int)(SCALE_X(3840)*PD_AREA_WIDTH)&0xFF, 0x00}, \
  {PD_AREA_HEIGHT_ADDR, (int)(SCALE_Y(2160)*PD_AREA_HEIGHT)>>8, 0x00}, \
  {PD_AREA_HEIGHT_ADDR+1, (int)(SCALE_Y(2160)*PD_AREA_HEIGHT)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR, (int)(SCALE_X(3840)*PD_WIN_X_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+1, (int)(SCALE_X(3840)*PD_WIN_X_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+2, (int)(SCALE_Y(2160)*PD_WIN_Y_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+3, (int)(SCALE_Y(2160)*PD_WIN_Y_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+4, (int)(SCALE_X(3840)*PD_WIN_X_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+5, (int)(SCALE_X(3840)*PD_WIN_X_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+6, (int)(SCALE_Y(2160)*PD_WIN_Y_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+7, (int)(SCALE_Y(2160)*PD_WIN_Y_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+8, (int)(SCALE_X(3840)*PD_WIN_X_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+9, (int)(SCALE_X(3840)*PD_WIN_X_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+10, (int)(SCALE_Y(2160)*PD_WIN_Y_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+11, (int)(SCALE_Y(2160)*PD_WIN_Y_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+12, (int)(SCALE_X(3840)*PD_WIN_X_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+13, (int)(SCALE_X(3840)*PD_WIN_X_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+14, (int)(SCALE_Y(2160)*PD_WIN_Y_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+15, (int)(SCALE_Y(2160)*PD_WIN_Y_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+16, (int)(SCALE_X(3840)*PD_WIN_X_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+17, (int)(SCALE_X(3840)*PD_WIN_X_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+18, (int)(SCALE_Y(2160)*PD_WIN_Y_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+19, (int)(SCALE_Y(2160)*PD_WIN_Y_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+20, (int)(SCALE_X(3840)*PD_WIN_X_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+21, (int)(SCALE_X(3840)*PD_WIN_X_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+22, (int)(SCALE_Y(2160)*PD_WIN_Y_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+23, (int)(SCALE_Y(2160)*PD_WIN_Y_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+24, (int)(SCALE_X(3840)*PD_WIN_X_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+25, (int)(SCALE_X(3840)*PD_WIN_X_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+26, (int)(SCALE_Y(2160)*PD_WIN_Y_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+27, (int)(SCALE_Y(2160)*PD_WIN_Y_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+28, (int)(SCALE_X(3840)*PD_WIN_X_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+29, (int)(SCALE_X(3840)*PD_WIN_X_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+30, (int)(SCALE_Y(2160)*PD_WIN_Y_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+31, (int)(SCALE_Y(2160)*PD_WIN_Y_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+32, (int)(SCALE_X(3840)*PD_WIN_X_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+33, (int)(SCALE_X(3840)*PD_WIN_X_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+34, (int)(SCALE_Y(2160)*PD_WIN_Y_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+35, (int)(SCALE_Y(2160)*PD_WIN_Y_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+36, (int)(SCALE_X(3840)*PD_WIN_X_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+37, (int)(SCALE_X(3840)*PD_WIN_X_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+38, (int)(SCALE_Y(2160)*PD_WIN_Y_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+39, (int)(SCALE_Y(2160)*PD_WIN_Y_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+40, (int)(SCALE_X(3840)*PD_WIN_X_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+41, (int)(SCALE_X(3840)*PD_WIN_X_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+42, (int)(SCALE_Y(2160)*PD_WIN_Y_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+43, (int)(SCALE_Y(2160)*PD_WIN_Y_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+44, (int)(SCALE_X(3840)*PD_WIN_X_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+45, (int)(SCALE_X(3840)*PD_WIN_X_E_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+46, (int)(SCALE_Y(2160)*PD_WIN_Y_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+47, (int)(SCALE_Y(2160)*PD_WIN_Y_E_5)&0xFF, 0x00}, \
}

#define RES3_REG_ARRAY \
{ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0342, 0x0B, 0x00}, \
  {0x0343, 0xA0, 0x00}, \
  {0x0340, 0x14, 0x00}, \
  {0x0341, 0xFC, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x00, 0x00}, \
  {0x0347, 0x00, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0xD7, 0x00}, \
  {0x034A, 0x0B, 0x00}, \
  {0x034B, 0xDF, 0x00}, \
  {0x0220, 0x00, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x01, 0x00}, \
  {0x0901, 0x22, 0x00}, \
  {0x0902, 0x02, 0x00}, \
  {0x3140, 0x02, 0x00}, \
  {0x3c00, 0x00, 0x00}, \
  {0x3c01, 0x01, 0x00}, \
  {0x3c02, 0x9C, 0x00}, \
  {0x3F0D, 0x00, 0x00}, \
  {0x5748, 0x00, 0x00}, \
  {0x5749, 0x00, 0x00}, \
  {0x574a, 0x00, 0x00}, \
  {0x574b, 0xA4, 0x00}, \
  {0x7B53, 0x00, 0x00}, \
  {0x9369, 0x73, 0x00}, \
  {0x936B, 0x64, 0x00}, \
  {0x936D, 0x5F, 0x00}, \
  {0x9304, 0x00, 0x00}, \
  {0x9305, 0x80, 0x00}, \
  {0x9E9A, 0x2F, 0x00}, \
  {0x9E9B, 0x2F, 0x00}, \
  {0x9E9C, 0x2F, 0x00}, \
  {0x9E9D, 0x00, 0x00}, \
  {0x9E9E, 0x00, 0x00}, \
  {0x9E9F, 0x00, 0x00}, \
  {0xA2A9, 0x27, 0x00}, \
  {0xA2B5, 0x30, 0x00}, \
  {0xA2B7, 0x03, 0x00}, \
  {0x0401, 0x00, 0x00}, \
  {0x0404, 0x00, 0x00}, \
  {0x0405, 0x10, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x00, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x00, 0x00}, \
  {0x040C, 0x07, 0x00}, \
  {0x040D, 0xEC, 0x00}, \
  {0x040E, 0x05, 0x00}, \
  {0x040F, 0xF0, 0x00}, \
  {0x034C, 0x07, 0x00}, \
  {0x034D, 0xEC, 0x00}, \
  {0x034E, 0x05, 0x00}, \
  {0x034F, 0xF0, 0x00}, \
  {0x0301, 0x05, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0xC8, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x01, 0x00}, \
  {0x030D, 0x02, 0x00}, \
  {0x030E, 0x01, 0x00}, \
  {0x030F, 0x5E, 0x00}, \
  {0x0310, 0x00, 0x00}, \
  {0x0820, 0x12, 0x00}, \
  {0x0821, 0xC0, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3E20, 0x01, 0x00}, \
  {0x3E37, 0x01, 0x00}, \
  {0x3F50, 0x00, 0x00}, \
  {0x3F56, 0x00, 0x00}, \
  {0x3F57, 0x95, 0x00}, \
  /*PDAF Settings */ \
  {PD_AREA_X_OFFSET_ADDR, (int)(SCALE_X(2028)*PD_AREA_X_OFFSET)>>8, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR+1, (int)(SCALE_X(2028)*PD_AREA_X_OFFSET)&0xFF, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR, (int)(SCALE_Y(1520)*PD_AREA_Y_OFFSET)>>8, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR+1, (int)(SCALE_Y(1520)*PD_AREA_Y_OFFSET)&0xFF, 0x00},\
  {PD_AREA_WIDTH_ADDR, (int)(SCALE_X(2028)*PD_AREA_WIDTH)>>8, 0x00}, \
  {PD_AREA_WIDTH_ADDR+1, (int)(SCALE_X(2028)*PD_AREA_WIDTH)&0xFF, 0x00}, \
  {PD_AREA_HEIGHT_ADDR, (int)(SCALE_Y(1520)*PD_AREA_HEIGHT)>>8, 0x00}, \
  {PD_AREA_HEIGHT_ADDR+1, (int)(SCALE_Y(1520)*PD_AREA_HEIGHT)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR, (int)(SCALE_X(2028)*PD_WIN_X_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+1, (int)(SCALE_X(2028)*PD_WIN_X_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+2, (int)(SCALE_Y(1520)*PD_WIN_Y_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+3, (int)(SCALE_Y(1520)*PD_WIN_Y_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+4, (int)(SCALE_X(2028)*PD_WIN_X_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+5, (int)(SCALE_X(2028)*PD_WIN_X_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+6, (int)(SCALE_Y(1520)*PD_WIN_Y_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+7, (int)(SCALE_Y(1520)*PD_WIN_Y_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+8, (int)(SCALE_X(2028)*PD_WIN_X_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+9, (int)(SCALE_X(2028)*PD_WIN_X_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+10, (int)(SCALE_Y(1520)*PD_WIN_Y_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+11, (int)(SCALE_Y(1520)*PD_WIN_Y_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+12, (int)(SCALE_X(2028)*PD_WIN_X_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+13, (int)(SCALE_X(2028)*PD_WIN_X_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+14, (int)(SCALE_Y(1520)*PD_WIN_Y_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+15, (int)(SCALE_Y(1520)*PD_WIN_Y_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+16, (int)(SCALE_X(2028)*PD_WIN_X_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+17, (int)(SCALE_X(2028)*PD_WIN_X_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+18, (int)(SCALE_Y(1520)*PD_WIN_Y_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+19, (int)(SCALE_Y(1520)*PD_WIN_Y_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+20, (int)(SCALE_X(2028)*PD_WIN_X_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+21, (int)(SCALE_X(2028)*PD_WIN_X_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+22, (int)(SCALE_Y(1520)*PD_WIN_Y_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+23, (int)(SCALE_Y(1520)*PD_WIN_Y_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+24, (int)(SCALE_X(2028)*PD_WIN_X_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+25, (int)(SCALE_X(2028)*PD_WIN_X_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+26, (int)(SCALE_Y(1520)*PD_WIN_Y_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+27, (int)(SCALE_Y(1520)*PD_WIN_Y_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+28, (int)(SCALE_X(2028)*PD_WIN_X_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+29, (int)(SCALE_X(2028)*PD_WIN_X_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+30, (int)(SCALE_Y(1520)*PD_WIN_Y_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+31, (int)(SCALE_Y(1520)*PD_WIN_Y_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+32, (int)(SCALE_X(2028)*PD_WIN_X_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+33, (int)(SCALE_X(2028)*PD_WIN_X_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+34, (int)(SCALE_Y(1520)*PD_WIN_Y_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+35, (int)(SCALE_Y(1520)*PD_WIN_Y_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+36, (int)(SCALE_X(2028)*PD_WIN_X_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+37, (int)(SCALE_X(2028)*PD_WIN_X_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+38, (int)(SCALE_Y(1520)*PD_WIN_Y_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+39, (int)(SCALE_Y(1520)*PD_WIN_Y_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+40, (int)(SCALE_X(2028)*PD_WIN_X_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+41, (int)(SCALE_X(2028)*PD_WIN_X_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+42, (int)(SCALE_Y(1520)*PD_WIN_Y_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+43, (int)(SCALE_Y(1520)*PD_WIN_Y_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+44, (int)(SCALE_X(2028)*PD_WIN_X_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+45, (int)(SCALE_X(2028)*PD_WIN_X_E_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+46, (int)(SCALE_Y(1520)*PD_WIN_Y_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+47, (int)(SCALE_Y(1520)*PD_WIN_Y_E_5)&0xFF, 0x00}, \
}


#define RES4_REG_ARRAY \
{ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0342, 0x12, 0x00}, \
  {0x0343, 0xC0, 0x00}, \
  {0x0340, 0x0D, 0x00}, \
  {0x0341, 0x04, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x01, 0x00}, \
  {0x0347, 0x78, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0xD7, 0x00}, \
  {0x034A, 0x0A, 0x00}, \
  {0x034B, 0x67, 0x00}, \
  {0x0220, 0x00, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x00, 0x00}, \
  {0x0901, 0x11, 0x00}, \
  {0x0902, 0x02, 0x00}, \
  {0x3140, 0x02, 0x00}, \
  {0x3c00, 0x01, 0x00}, \
  {0x3c01, 0x03, 0x00}, \
  {0x3c02, 0xDC, 0x00}, \
  {0x3F0D, 0x00, 0x00}, \
  {0x5748, 0x07, 0x00}, \
  {0x5749, 0xFF, 0x00}, \
  {0x574a, 0x00, 0x00}, \
  {0x574b, 0x00, 0x00}, \
  {0x7B53, 0x01, 0x00}, \
  {0x9369, 0x5A, 0x00}, \
  {0x936B, 0x55, 0x00}, \
  {0x936D, 0x28, 0x00}, \
  {0x9304, 0x00, 0x00}, \
  {0x9305, 0x00, 0x00}, \
  {0x9E9A, 0x2F, 0x00}, \
  {0x9E9B, 0x2F, 0x00}, \
  {0x9E9C, 0x2F, 0x00}, \
  {0x9E9D, 0x00, 0x00}, \
  {0x9E9E, 0x00, 0x00}, \
  {0x9E9F, 0x00, 0x00}, \
  {0xA2A9, 0x60, 0x00}, \
  {0xA2B5, 0x50, 0x00}, \
  {0xA2B7, 0x00, 0x00}, \
  {0x0401, 0x00, 0x00}, \
  {0x0404, 0x00, 0x00}, \
  {0x0405, 0x10, 0x00}, \
  {0x0408, 0x03, 0x00}, \
  {0x0409, 0xF6, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x00, 0x00}, \
  {0x040C, 0x07, 0x00}, \
  {0x040D, 0xEC, 0x00}, \
  {0x040E, 0x04, 0x00}, \
  {0x040F, 0x78, 0x00}, \
  {0x034C, 0x07, 0x00}, \
  {0x034D, 0xEC, 0x00}, \
  {0x034E, 0x04, 0x00}, \
  {0x034F, 0x78, 0x00}, \
  {0x0301, 0x05, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0xC8, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x02, 0x00}, \
  {0x030D, 0x02, 0x00}, \
  {0x030E, 0x01, 0x00}, \
  {0x030F, 0x5E, 0x00}, \
  {0x0310, 0x00, 0x00}, \
  {0x0820, 0x09, 0x00}, \
  {0x0821, 0x60, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3E20, 0x01, 0x00}, \
  {0x3E37, 0x01, 0x00}, \
  {0x3F50, 0x00, 0x00}, \
  {0x3F56, 0x00, 0x00}, \
  {0x3F57, 0xF0, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR, (int)(SCALE_X(2028)*PD_AREA_X_OFFSET)>>8, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR+1, (int)(SCALE_X(2028)*PD_AREA_X_OFFSET)&0xFF, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR, (int)(SCALE_Y(1144)*PD_AREA_Y_OFFSET)>>8, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR+1, (int)(SCALE_Y(1144)*PD_AREA_Y_OFFSET)&0xFF, 0x00},\
  {PD_AREA_WIDTH_ADDR, (int)(SCALE_X(2028)*PD_AREA_WIDTH)>>8, 0x00}, \
  {PD_AREA_WIDTH_ADDR+1, (int)(SCALE_X(2028)*PD_AREA_WIDTH)&0xFF, 0x00}, \
  {PD_AREA_HEIGHT_ADDR, (int)(SCALE_Y(1144)*PD_AREA_HEIGHT)>>8, 0x00}, \
  {PD_AREA_HEIGHT_ADDR+1, (int)(SCALE_Y(1144)*PD_AREA_HEIGHT)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR, (int)(SCALE_X(2028)*PD_WIN_X_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+1, (int)(SCALE_X(2028)*PD_WIN_X_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+2, (int)(SCALE_Y(1144)*PD_WIN_Y_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+3, (int)(SCALE_Y(1144)*PD_WIN_Y_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+4, (int)(SCALE_X(2028)*PD_WIN_X_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+5, (int)(SCALE_X(2028)*PD_WIN_X_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+6, (int)(SCALE_Y(1144)*PD_WIN_Y_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+7, (int)(SCALE_Y(1144)*PD_WIN_Y_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+8, (int)(SCALE_X(2028)*PD_WIN_X_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+9, (int)(SCALE_X(2028)*PD_WIN_X_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+10, (int)(SCALE_Y(1144)*PD_WIN_Y_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+11, (int)(SCALE_Y(1144)*PD_WIN_Y_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+12, (int)(SCALE_X(2028)*PD_WIN_X_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+13, (int)(SCALE_X(2028)*PD_WIN_X_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+14, (int)(SCALE_Y(1144)*PD_WIN_Y_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+15, (int)(SCALE_Y(1144)*PD_WIN_Y_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+16, (int)(SCALE_X(2028)*PD_WIN_X_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+17, (int)(SCALE_X(2028)*PD_WIN_X_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+18, (int)(SCALE_Y(1144)*PD_WIN_Y_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+19, (int)(SCALE_Y(1144)*PD_WIN_Y_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+20, (int)(SCALE_X(2028)*PD_WIN_X_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+21, (int)(SCALE_X(2028)*PD_WIN_X_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+22, (int)(SCALE_Y(1144)*PD_WIN_Y_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+23, (int)(SCALE_Y(1144)*PD_WIN_Y_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+24, (int)(SCALE_X(2028)*PD_WIN_X_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+25, (int)(SCALE_X(2028)*PD_WIN_X_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+26, (int)(SCALE_Y(1144)*PD_WIN_Y_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+27, (int)(SCALE_Y(1144)*PD_WIN_Y_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+28, (int)(SCALE_X(2028)*PD_WIN_X_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+29, (int)(SCALE_X(2028)*PD_WIN_X_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+30, (int)(SCALE_Y(1144)*PD_WIN_Y_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+31, (int)(SCALE_Y(1144)*PD_WIN_Y_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+32, (int)(SCALE_X(2028)*PD_WIN_X_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+33, (int)(SCALE_X(2028)*PD_WIN_X_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+34, (int)(SCALE_Y(1144)*PD_WIN_Y_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+35, (int)(SCALE_Y(1144)*PD_WIN_Y_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+36, (int)(SCALE_X(2028)*PD_WIN_X_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+37, (int)(SCALE_X(2028)*PD_WIN_X_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+38, (int)(SCALE_Y(1144)*PD_WIN_Y_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+39, (int)(SCALE_Y(1144)*PD_WIN_Y_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+40, (int)(SCALE_X(2028)*PD_WIN_X_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+41, (int)(SCALE_X(2028)*PD_WIN_X_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+42, (int)(SCALE_Y(1144)*PD_WIN_Y_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+43, (int)(SCALE_Y(1144)*PD_WIN_Y_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+44, (int)(SCALE_X(2028)*PD_WIN_X_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+45, (int)(SCALE_X(2028)*PD_WIN_X_E_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+46, (int)(SCALE_Y(1144)*PD_WIN_Y_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+47, (int)(SCALE_Y(1144)*PD_WIN_Y_E_5)&0xFF, 0x00}, \
}

#define RES5_REG_ARRAY \
{ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0342, 0x18, 0x00}, \
  {0x0343, 0x94, 0x00}, \
  {0x0340, 0x08, 0x00}, \
  {0x0341, 0xB0, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x6C, 0x00}, \
  {0x0346, 0x01, 0x00}, \
  {0x0347, 0xB8, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0x6B, 0x00}, \
  {0x034A, 0x0A, 0x00}, \
  {0x034B, 0x27, 0x00}, \
  {0x0220, 0x00, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x00, 0x00}, \
  {0x0901, 0x11, 0x00}, \
  {0x0902, 0x02, 0x00}, \
  {0x3140, 0x02, 0x00}, \
  {0x3c00, 0x00, 0x00}, \
  {0x3c01, 0x03, 0x00}, \
  {0x3c02, 0xDC, 0x00}, \
  {0x3F0D, 0x00, 0x00}, \
  {0x5748, 0x07, 0x00}, \
  {0x5749, 0xFF, 0x00}, \
  {0x574a, 0x00, 0x00}, \
  {0x574b, 0x00, 0x00}, \
  {0x7B53, 0x01, 0x00}, \
  {0x9369, 0x5A, 0x00}, \
  {0x936B, 0x55, 0x00}, \
  {0x936D, 0x28, 0x00}, \
  {0x9304, 0x00, 0x00}, \
  {0x9305, 0x00, 0x00}, \
  {0x9E9A, 0x2F, 0x00}, \
  {0x9E9B, 0x2F, 0x00}, \
  {0x9E9C, 0x2F, 0x00}, \
  {0x9E9D, 0x00, 0x00}, \
  {0x9E9E, 0x00, 0x00}, \
  {0x9E9F, 0x00, 0x00}, \
  {0xA2A9, 0x60, 0x00}, \
  {0xA2B7, 0x00, 0x00}, \
  {0x0401, 0x00, 0x00}, \
  {0x0404, 0x00, 0x00}, \
  {0x0405, 0x10, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x00, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x00, 0x00}, \
  {0x040C, 0x0F, 0x00}, \
  {0x040D, 0x00, 0x00}, \
  {0x040E, 0x08, 0x00}, \
  {0x040F, 0x70, 0x00}, \
  {0x034C, 0x0F, 0x00}, \
  {0x034D, 0x00, 0x00}, \
  {0x034E, 0x08, 0x00}, \
  {0x034F, 0x70, 0x00}, \
  {0x0301, 0x05, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x01, 0x00}, \
  {0x0307, 0x5E, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x01, 0x00}, \
  {0x030D, 0x02, 0x00}, \
  {0x030E, 0x00, 0x00}, \
  {0x030F, 0x72, 0x00}, \
  {0x0310, 0x01, 0x00}, \
  {0x0820, 0x15, 0x00}, \
  {0x0821, 0x60, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3E20, 0x01, 0x00}, \
  {0x3E37, 0x01, 0x00}, \
  {0x3F50, 0x00, 0x00}, \
  {0x3F56, 0x00, 0x00}, \
  {0x3F57, 0xB4, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR, (int)(SCALE_X(3840)*PD_AREA_X_OFFSET)>>8, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR+1, (int)(SCALE_X(3840)*PD_AREA_X_OFFSET)&0xFF, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR, (int)(SCALE_Y(2160)*PD_AREA_Y_OFFSET)>>8, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR+1, (int)(SCALE_Y(2160)*PD_AREA_Y_OFFSET)&0xFF, 0x00},\
  {PD_AREA_WIDTH_ADDR, (int)(SCALE_X(3840)*PD_AREA_WIDTH)>>8, 0x00}, \
  {PD_AREA_WIDTH_ADDR+1, (int)(SCALE_X(3840)*PD_AREA_WIDTH)&0xFF, 0x00}, \
  {PD_AREA_HEIGHT_ADDR, (int)(SCALE_Y(2160)*PD_AREA_HEIGHT)>>8, 0x00}, \
  {PD_AREA_HEIGHT_ADDR+1, (int)(SCALE_Y(2160)*PD_AREA_HEIGHT)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR, (int)(SCALE_X(3840)*PD_WIN_X_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+1, (int)(SCALE_X(3840)*PD_WIN_X_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+2, (int)(SCALE_Y(2160)*PD_WIN_Y_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+3, (int)(SCALE_Y(2160)*PD_WIN_Y_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+4, (int)(SCALE_X(3840)*PD_WIN_X_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+5, (int)(SCALE_X(3840)*PD_WIN_X_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+6, (int)(SCALE_Y(2160)*PD_WIN_Y_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+7, (int)(SCALE_Y(2160)*PD_WIN_Y_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+8, (int)(SCALE_X(3840)*PD_WIN_X_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+9, (int)(SCALE_X(3840)*PD_WIN_X_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+10, (int)(SCALE_Y(2160)*PD_WIN_Y_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+11, (int)(SCALE_Y(2160)*PD_WIN_Y_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+12, (int)(SCALE_X(3840)*PD_WIN_X_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+13, (int)(SCALE_X(3840)*PD_WIN_X_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+14, (int)(SCALE_Y(2160)*PD_WIN_Y_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+15, (int)(SCALE_Y(2160)*PD_WIN_Y_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+16, (int)(SCALE_X(3840)*PD_WIN_X_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+17, (int)(SCALE_X(3840)*PD_WIN_X_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+18, (int)(SCALE_Y(2160)*PD_WIN_Y_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+19, (int)(SCALE_Y(2160)*PD_WIN_Y_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+20, (int)(SCALE_X(3840)*PD_WIN_X_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+21, (int)(SCALE_X(3840)*PD_WIN_X_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+22, (int)(SCALE_Y(2160)*PD_WIN_Y_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+23, (int)(SCALE_Y(2160)*PD_WIN_Y_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+24, (int)(SCALE_X(3840)*PD_WIN_X_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+25, (int)(SCALE_X(3840)*PD_WIN_X_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+26, (int)(SCALE_Y(2160)*PD_WIN_Y_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+27, (int)(SCALE_Y(2160)*PD_WIN_Y_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+28, (int)(SCALE_X(3840)*PD_WIN_X_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+29, (int)(SCALE_X(3840)*PD_WIN_X_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+30, (int)(SCALE_Y(2160)*PD_WIN_Y_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+31, (int)(SCALE_Y(2160)*PD_WIN_Y_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+32, (int)(SCALE_X(3840)*PD_WIN_X_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+33, (int)(SCALE_X(3840)*PD_WIN_X_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+34, (int)(SCALE_Y(2160)*PD_WIN_Y_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+35, (int)(SCALE_Y(2160)*PD_WIN_Y_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+36, (int)(SCALE_X(3840)*PD_WIN_X_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+37, (int)(SCALE_X(3840)*PD_WIN_X_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+38, (int)(SCALE_Y(2160)*PD_WIN_Y_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+39, (int)(SCALE_Y(2160)*PD_WIN_Y_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+40, (int)(SCALE_X(3840)*PD_WIN_X_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+41, (int)(SCALE_X(3840)*PD_WIN_X_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+42, (int)(SCALE_Y(2160)*PD_WIN_Y_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+43, (int)(SCALE_Y(2160)*PD_WIN_Y_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+44, (int)(SCALE_X(3840)*PD_WIN_X_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+45, (int)(SCALE_X(3840)*PD_WIN_X_E_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+46, (int)(SCALE_Y(2160)*PD_WIN_Y_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+47, (int)(SCALE_Y(2160)*PD_WIN_Y_E_5)&0xFF, 0x00}, \
}

#define RES6_REG_ARRAY \
{ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0342, 0x11, 0x00}, \
  {0x0343, 0xA0, 0x00}, \
  {0x0340, 0x06, 0x00}, \
  {0x0341, 0xEC, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x01, 0x00}, \
  {0x0347, 0x78, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0xD7, 0x00}, \
  {0x034A, 0x0A, 0x00}, \
  {0x034B, 0x67, 0x00}, \
  {0x0220, 0x00, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x01, 0x00}, \
  {0x0901, 0x22, 0x00}, \
  {0x0902, 0x02, 0x00}, \
  {0x3140, 0x02, 0x00}, \
  {0x3c00, 0x00, 0x00}, \
  {0x3c01, 0x01, 0x00}, \
  {0x3c02, 0x9C, 0x00}, \
  {0x3F0D, 0x00, 0x00}, \
  {0x5748, 0x00, 0x00}, \
  {0x5749, 0x00, 0x00}, \
  {0x574a, 0x00, 0x00}, \
  {0x574b, 0xA4, 0x00}, \
  {0x7B53, 0x00, 0x00}, \
  {0x9369, 0x73, 0x00}, \
  {0x936B, 0x64, 0x00}, \
  {0x936D, 0x5F, 0x00}, \
  {0x9304, 0x00, 0x00}, \
  {0x9305, 0x80, 0x00}, \
  {0x9E9A, 0x2F, 0x00}, \
  {0x9E9B, 0x2F, 0x00}, \
  {0x9E9C, 0x2F, 0x00}, \
  {0x9E9D, 0x00, 0x00}, \
  {0x9E9E, 0x00, 0x00}, \
  {0x9E9F, 0x00, 0x00}, \
  {0xA2A9, 0x27, 0x00}, \
  {0xA2B5, 0x30, 0x00}, \
  {0xA2B7, 0x03, 0x00}, \
  {0x0401, 0x00, 0x00}, \
  {0x0404, 0x00, 0x00}, \
  {0x0405, 0x10, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x36, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x20, 0x00}, \
  {0x040C, 0x07, 0x00}, \
  {0x040D, 0x80, 0x00}, \
  {0x040E, 0x04, 0x00}, \
  {0x040F, 0x38, 0x00}, \
  {0x034C, 0x07, 0x00}, \
  {0x034D, 0x80, 0x00}, \
  {0x034E, 0x04, 0x00}, \
  {0x034F, 0x38, 0x00}, \
  {0x0301, 0x05, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0xC8, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x02, 0x00}, \
  {0x030D, 0x02, 0x00}, \
  {0x030E, 0x01, 0x00}, \
  {0x030F, 0x5E, 0x00}, \
  {0x0310, 0x00, 0x00}, \
  {0x0820, 0x09, 0x00}, \
  {0x0821, 0x60, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3E20, 0x01, 0x00}, \
  {0x3E37, 0x01, 0x00}, \
  {0x3F50, 0x00, 0x00}, \
  {0x3F56, 0x00, 0x00}, \
  {0x3F57, 0xE2, 0x00}, \
  /*PDAF Setting */ \
  {PD_AREA_X_OFFSET_ADDR, (int)(SCALE_X(1920)*PD_AREA_X_OFFSET)>>8, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR+1, (int)(SCALE_X(1920)*PD_AREA_X_OFFSET)&0xFF, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR, (int)(SCALE_Y(1080)*PD_AREA_Y_OFFSET)>>8, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR+1, (int)(SCALE_Y(1080)*PD_AREA_Y_OFFSET)&0xFF, 0x00},\
  {PD_AREA_WIDTH_ADDR, (int)(SCALE_X(1920)*PD_AREA_WIDTH)>>8, 0x00}, \
  {PD_AREA_WIDTH_ADDR+1, (int)(SCALE_X(1920)*PD_AREA_WIDTH)&0xFF, 0x00}, \
  {PD_AREA_HEIGHT_ADDR, (int)(SCALE_Y(1080)*PD_AREA_HEIGHT)>>8, 0x00}, \
  {PD_AREA_HEIGHT_ADDR+1, (int)(SCALE_Y(1080)*PD_AREA_HEIGHT)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR, (int)(SCALE_X(1920)*PD_WIN_X_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+1, (int)(SCALE_X(1920)*PD_WIN_X_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+2, (int)(SCALE_Y(1080)*PD_WIN_Y_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+3, (int)(SCALE_Y(1080)*PD_WIN_Y_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+4, (int)(SCALE_X(1920)*PD_WIN_X_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+5, (int)(SCALE_X(1920)*PD_WIN_X_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+6, (int)(SCALE_Y(1080)*PD_WIN_Y_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+7, (int)(SCALE_Y(1080)*PD_WIN_Y_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+8, (int)(SCALE_X(1920)*PD_WIN_X_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+9, (int)(SCALE_X(1920)*PD_WIN_X_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+10, (int)(SCALE_Y(1080)*PD_WIN_Y_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+11, (int)(SCALE_Y(1080)*PD_WIN_Y_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+12, (int)(SCALE_X(1920)*PD_WIN_X_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+13, (int)(SCALE_X(1920)*PD_WIN_X_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+14, (int)(SCALE_Y(1080)*PD_WIN_Y_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+15, (int)(SCALE_Y(1080)*PD_WIN_Y_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+16, (int)(SCALE_X(1920)*PD_WIN_X_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+17, (int)(SCALE_X(1920)*PD_WIN_X_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+18, (int)(SCALE_Y(1080)*PD_WIN_Y_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+19, (int)(SCALE_Y(1080)*PD_WIN_Y_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+20, (int)(SCALE_X(1920)*PD_WIN_X_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+21, (int)(SCALE_X(1920)*PD_WIN_X_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+22, (int)(SCALE_Y(1080)*PD_WIN_Y_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+23, (int)(SCALE_Y(1080)*PD_WIN_Y_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+24, (int)(SCALE_X(1920)*PD_WIN_X_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+25, (int)(SCALE_X(1920)*PD_WIN_X_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+26, (int)(SCALE_Y(1080)*PD_WIN_Y_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+27, (int)(SCALE_Y(1080)*PD_WIN_Y_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+28, (int)(SCALE_X(1920)*PD_WIN_X_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+29, (int)(SCALE_X(1920)*PD_WIN_X_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+30, (int)(SCALE_Y(1080)*PD_WIN_Y_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+31, (int)(SCALE_Y(1080)*PD_WIN_Y_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+32, (int)(SCALE_X(1920)*PD_WIN_X_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+33, (int)(SCALE_X(1920)*PD_WIN_X_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+34, (int)(SCALE_Y(1080)*PD_WIN_Y_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+35, (int)(SCALE_Y(1080)*PD_WIN_Y_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+36, (int)(SCALE_X(1920)*PD_WIN_X_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+37, (int)(SCALE_X(1920)*PD_WIN_X_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+38, (int)(SCALE_Y(1080)*PD_WIN_Y_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+39, (int)(SCALE_Y(1080)*PD_WIN_Y_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+40, (int)(SCALE_X(1920)*PD_WIN_X_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+41, (int)(SCALE_X(1920)*PD_WIN_X_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+42, (int)(SCALE_Y(1080)*PD_WIN_Y_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+43, (int)(SCALE_Y(1080)*PD_WIN_Y_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+44, (int)(SCALE_X(1920)*PD_WIN_X_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+45, (int)(SCALE_X(1920)*PD_WIN_X_E_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+46, (int)(SCALE_Y(1080)*PD_WIN_Y_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+47, (int)(SCALE_Y(1080)*PD_WIN_Y_E_5)&0xFF, 0x00} \
}

#define RES7_REG_ARRAY \
{ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0342, 0x11, 0x00}, \
  {0x0343, 0xA0, 0x00}, \
  {0x0340, 0x04, 0x00}, \
  {0x0341, 0xA8, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x01, 0x00}, \
  {0x0347, 0x78, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0xD7, 0x00}, \
  {0x034A, 0x0A, 0x00}, \
  {0x034B, 0x67, 0x00}, \
  {0x0220, 0x00, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x01, 0x00}, \
  {0x0901, 0x22, 0x00}, \
  {0x0902, 0x02, 0x00}, \
  {0x3140, 0x02, 0x00}, \
  {0x3c00, 0x00, 0x00}, \
  {0x3c01, 0x01, 0x00}, \
  {0x3c02, 0x9C, 0x00}, \
  {0x3F0D, 0x00, 0x00}, \
  {0x5748, 0x00, 0x00}, \
  {0x5749, 0x00, 0x00}, \
  {0x574a, 0x00, 0x00}, \
  {0x574b, 0xA4, 0x00}, \
  {0x7B53, 0x00, 0x00}, \
  {0x9369, 0x73, 0x00}, \
  {0x936B, 0x64, 0x00}, \
  {0x936D, 0x5F, 0x00}, \
  {0x9304, 0x00, 0x00}, \
  {0x9305, 0x80, 0x00}, \
  {0x9E9A, 0x2F, 0x00}, \
  {0x9E9B, 0x2F, 0x00}, \
  {0x9E9C, 0x2F, 0x00}, \
  {0x9E9D, 0x00, 0x00}, \
  {0x9E9E, 0x00, 0x00}, \
  {0x9E9F, 0x00, 0x00}, \
  {0xA2A9, 0x27, 0x00}, \
  {0xA2B5, 0x30, 0x00}, \
  {0xA2B7, 0x03, 0x00}, \
  {0x0401, 0x00, 0x00}, \
  {0x0404, 0x00, 0x00}, \
  {0x0405, 0x10, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x36, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x20, 0x00}, \
  {0x040C, 0x07, 0x00}, \
  {0x040D, 0x80, 0x00}, \
  {0x040E, 0x04, 0x00}, \
  {0x040F, 0x38, 0x00}, \
  {0x034C, 0x07, 0x00}, \
  {0x034D, 0x80, 0x00}, \
  {0x034E, 0x04, 0x00}, \
  {0x034F, 0x38, 0x00}, \
  {0x0301, 0x05, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0xCA, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x02, 0x00}, \
  {0x030D, 0x02, 0x00}, \
  {0x030E, 0x01, 0x00}, \
  {0x030F, 0x5E, 0x00}, \
  {0x0310, 0x00, 0x00}, \
  {0x0820, 0x09, 0x00}, \
  {0x0821, 0x78, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3E20, 0x01, 0x00}, \
  {0x3E37, 0x01, 0x00}, \
  {0x3F50, 0x00, 0x00}, \
  {0x3F56, 0x00, 0x00}, \
  {0x3F57, 0xE0, 0x00}, \
    /*PDAF Setting */ \
  {PD_AREA_X_OFFSET_ADDR, (int)(SCALE_X(1920)*PD_AREA_X_OFFSET)>>8, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR+1, (int)(SCALE_X(1920)*PD_AREA_X_OFFSET)&0xFF, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR, (int)(SCALE_Y(1080)*PD_AREA_Y_OFFSET)>>8, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR+1, (int)(SCALE_Y(1080)*PD_AREA_Y_OFFSET)&0xFF, 0x00},\
  {PD_AREA_WIDTH_ADDR, (int)(SCALE_X(1920)*PD_AREA_WIDTH)>>8, 0x00}, \
  {PD_AREA_WIDTH_ADDR+1, (int)(SCALE_X(1920)*PD_AREA_WIDTH)&0xFF, 0x00}, \
  {PD_AREA_HEIGHT_ADDR, (int)(SCALE_Y(1080)*PD_AREA_HEIGHT)>>8, 0x00}, \
  {PD_AREA_HEIGHT_ADDR+1, (int)(SCALE_Y(1080)*PD_AREA_HEIGHT)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR, (int)(SCALE_X(1920)*PD_WIN_X_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+1, (int)(SCALE_X(1920)*PD_WIN_X_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+2, (int)(SCALE_Y(1080)*PD_WIN_Y_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+3, (int)(SCALE_Y(1080)*PD_WIN_Y_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+4, (int)(SCALE_X(1920)*PD_WIN_X_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+5, (int)(SCALE_X(1920)*PD_WIN_X_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+6, (int)(SCALE_Y(1080)*PD_WIN_Y_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+7, (int)(SCALE_Y(1080)*PD_WIN_Y_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+8, (int)(SCALE_X(1920)*PD_WIN_X_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+9, (int)(SCALE_X(1920)*PD_WIN_X_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+10, (int)(SCALE_Y(1080)*PD_WIN_Y_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+11, (int)(SCALE_Y(1080)*PD_WIN_Y_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+12, (int)(SCALE_X(1920)*PD_WIN_X_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+13, (int)(SCALE_X(1920)*PD_WIN_X_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+14, (int)(SCALE_Y(1080)*PD_WIN_Y_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+15, (int)(SCALE_Y(1080)*PD_WIN_Y_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+16, (int)(SCALE_X(1920)*PD_WIN_X_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+17, (int)(SCALE_X(1920)*PD_WIN_X_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+18, (int)(SCALE_Y(1080)*PD_WIN_Y_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+19, (int)(SCALE_Y(1080)*PD_WIN_Y_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+20, (int)(SCALE_X(1920)*PD_WIN_X_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+21, (int)(SCALE_X(1920)*PD_WIN_X_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+22, (int)(SCALE_Y(1080)*PD_WIN_Y_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+23, (int)(SCALE_Y(1080)*PD_WIN_Y_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+24, (int)(SCALE_X(1920)*PD_WIN_X_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+25, (int)(SCALE_X(1920)*PD_WIN_X_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+26, (int)(SCALE_Y(1080)*PD_WIN_Y_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+27, (int)(SCALE_Y(1080)*PD_WIN_Y_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+28, (int)(SCALE_X(1920)*PD_WIN_X_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+29, (int)(SCALE_X(1920)*PD_WIN_X_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+30, (int)(SCALE_Y(1080)*PD_WIN_Y_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+31, (int)(SCALE_Y(1080)*PD_WIN_Y_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+32, (int)(SCALE_X(1920)*PD_WIN_X_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+33, (int)(SCALE_X(1920)*PD_WIN_X_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+34, (int)(SCALE_Y(1080)*PD_WIN_Y_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+35, (int)(SCALE_Y(1080)*PD_WIN_Y_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+36, (int)(SCALE_X(1920)*PD_WIN_X_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+37, (int)(SCALE_X(1920)*PD_WIN_X_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+38, (int)(SCALE_Y(1080)*PD_WIN_Y_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+39, (int)(SCALE_Y(1080)*PD_WIN_Y_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+40, (int)(SCALE_X(1920)*PD_WIN_X_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+41, (int)(SCALE_X(1920)*PD_WIN_X_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+42, (int)(SCALE_Y(1080)*PD_WIN_Y_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+43, (int)(SCALE_Y(1080)*PD_WIN_Y_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+44, (int)(SCALE_X(1920)*PD_WIN_X_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+45, (int)(SCALE_X(1920)*PD_WIN_X_E_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+46, (int)(SCALE_Y(1080)*PD_WIN_Y_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+47, (int)(SCALE_Y(1080)*PD_WIN_Y_E_5)&0xFF, 0x00} \
}

#define RES8_REG_ARRAY \
{ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0342, 0x11, 0x00}, \
  {0x0343, 0xA0, 0x00}, \
  {0x0340, 0x04, 0x00}, \
  {0x0341, 0xAC, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x01, 0x00}, \
  {0x0347, 0x78, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0xD7, 0x00}, \
  {0x034A, 0x0A, 0x00}, \
  {0x034B, 0x67, 0x00}, \
  {0x0220, 0x00, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x01, 0x00}, \
  {0x0901, 0x22, 0x00}, \
  {0x0902, 0x02, 0x00}, \
  {0x3140, 0x02, 0x00}, \
  {0x3c00, 0x00, 0x00}, \
  {0x3c01, 0x01, 0x00}, \
  {0x3c02, 0x9C, 0x00}, \
  {0x3F0D, 0x00, 0x00}, \
  {0x5748, 0x00, 0x00}, \
  {0x5749, 0x00, 0x00}, \
  {0x574a, 0x00, 0x00}, \
  {0x574b, 0xA4, 0x00}, \
  {0x7B53, 0x00, 0x00}, \
  {0x9369, 0x73, 0x00}, \
  {0x936B, 0x64, 0x00}, \
  {0x936D, 0x5F, 0x00}, \
  {0x9304, 0x00, 0x00}, \
  {0x9305, 0x80, 0x00}, \
  {0x9E9A, 0x2F, 0x00}, \
  {0x9E9B, 0x2F, 0x00}, \
  {0x9E9C, 0x2F, 0x00}, \
  {0x9E9D, 0x00, 0x00}, \
  {0x9E9E, 0x00, 0x00}, \
  {0x9E9F, 0x00, 0x00}, \
  {0xA2A9, 0x27, 0x00}, \
  {0xA2B5, 0x30, 0x00}, \
  {0xA2B7, 0x03, 0x00}, \
  {0x0401, 0x00, 0x00}, \
  {0x0404, 0x00, 0x00}, \
  {0x0405, 0x10, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x36, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x20, 0x00}, \
  {0x040C, 0x07, 0x00}, \
  {0x040D, 0x80, 0x00}, \
  {0x040E, 0x04, 0x00}, \
  {0x040F, 0x38, 0x00}, \
  {0x034C, 0x07, 0x00}, \
  {0x034D, 0x80, 0x00}, \
  {0x034E, 0x04, 0x00}, \
  {0x034F, 0x38, 0x00}, \
  {0x0301, 0x05, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x01, 0x00}, \
  {0x0307, 0x0E, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x02, 0x00}, \
  {0x030D, 0x02, 0x00}, \
  {0x030E, 0x01, 0x00}, \
  {0x030F, 0x5E, 0x00}, \
  {0x0310, 0x00, 0x00}, \
  {0x0820, 0x0C, 0x00}, \
  {0x0821, 0xA8, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3E20, 0x01, 0x00}, \
  {0x3E37, 0x01, 0x00}, \
  {0x3F50, 0x00, 0x00}, \
  {0x3F56, 0x00, 0x00}, \
  {0x3F57, 0xA8, 0x00}, \
  /*PDAF Setting */ \
  {PD_AREA_X_OFFSET_ADDR, (int)(SCALE_X(1920)*PD_AREA_X_OFFSET)>>8, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR+1, (int)(SCALE_X(1920)*PD_AREA_X_OFFSET)&0xFF, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR, (int)(SCALE_Y(1080)*PD_AREA_Y_OFFSET)>>8, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR+1, (int)(SCALE_Y(1080)*PD_AREA_Y_OFFSET)&0xFF, 0x00},\
  {PD_AREA_WIDTH_ADDR, (int)(SCALE_X(1920)*PD_AREA_WIDTH)>>8, 0x00}, \
  {PD_AREA_WIDTH_ADDR+1, (int)(SCALE_X(1920)*PD_AREA_WIDTH)&0xFF, 0x00}, \
  {PD_AREA_HEIGHT_ADDR, (int)(SCALE_Y(1080)*PD_AREA_HEIGHT)>>8, 0x00}, \
  {PD_AREA_HEIGHT_ADDR+1, (int)(SCALE_Y(1080)*PD_AREA_HEIGHT)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR, (int)(SCALE_X(1920)*PD_WIN_X_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+1, (int)(SCALE_X(1920)*PD_WIN_X_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+2, (int)(SCALE_Y(1080)*PD_WIN_Y_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+3, (int)(SCALE_Y(1080)*PD_WIN_Y_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+4, (int)(SCALE_X(1920)*PD_WIN_X_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+5, (int)(SCALE_X(1920)*PD_WIN_X_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+6, (int)(SCALE_Y(1080)*PD_WIN_Y_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+7, (int)(SCALE_Y(1080)*PD_WIN_Y_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+8, (int)(SCALE_X(1920)*PD_WIN_X_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+9, (int)(SCALE_X(1920)*PD_WIN_X_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+10, (int)(SCALE_Y(1080)*PD_WIN_Y_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+11, (int)(SCALE_Y(1080)*PD_WIN_Y_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+12, (int)(SCALE_X(1920)*PD_WIN_X_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+13, (int)(SCALE_X(1920)*PD_WIN_X_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+14, (int)(SCALE_Y(1080)*PD_WIN_Y_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+15, (int)(SCALE_Y(1080)*PD_WIN_Y_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+16, (int)(SCALE_X(1920)*PD_WIN_X_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+17, (int)(SCALE_X(1920)*PD_WIN_X_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+18, (int)(SCALE_Y(1080)*PD_WIN_Y_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+19, (int)(SCALE_Y(1080)*PD_WIN_Y_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+20, (int)(SCALE_X(1920)*PD_WIN_X_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+21, (int)(SCALE_X(1920)*PD_WIN_X_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+22, (int)(SCALE_Y(1080)*PD_WIN_Y_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+23, (int)(SCALE_Y(1080)*PD_WIN_Y_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+24, (int)(SCALE_X(1920)*PD_WIN_X_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+25, (int)(SCALE_X(1920)*PD_WIN_X_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+26, (int)(SCALE_Y(1080)*PD_WIN_Y_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+27, (int)(SCALE_Y(1080)*PD_WIN_Y_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+28, (int)(SCALE_X(1920)*PD_WIN_X_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+29, (int)(SCALE_X(1920)*PD_WIN_X_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+30, (int)(SCALE_Y(1080)*PD_WIN_Y_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+31, (int)(SCALE_Y(1080)*PD_WIN_Y_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+32, (int)(SCALE_X(1920)*PD_WIN_X_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+33, (int)(SCALE_X(1920)*PD_WIN_X_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+34, (int)(SCALE_Y(1080)*PD_WIN_Y_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+35, (int)(SCALE_Y(1080)*PD_WIN_Y_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+36, (int)(SCALE_X(1920)*PD_WIN_X_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+37, (int)(SCALE_X(1920)*PD_WIN_X_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+38, (int)(SCALE_Y(1080)*PD_WIN_Y_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+39, (int)(SCALE_Y(1080)*PD_WIN_Y_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+40, (int)(SCALE_X(1920)*PD_WIN_X_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+41, (int)(SCALE_X(1920)*PD_WIN_X_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+42, (int)(SCALE_Y(1080)*PD_WIN_Y_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+43, (int)(SCALE_Y(1080)*PD_WIN_Y_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+44, (int)(SCALE_X(1920)*PD_WIN_X_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+45, (int)(SCALE_X(1920)*PD_WIN_X_E_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+46, (int)(SCALE_Y(1080)*PD_WIN_Y_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+47, (int)(SCALE_Y(1080)*PD_WIN_Y_E_5)&0xFF, 0x00} \
}

#define RES9_REG_ARRAY \
{ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0342, 0x0B, 0x00}, \
  {0x0343, 0xA0, 0x00}, \
  {0x0340, 0x02, 0x00}, \
  {0x0341, 0xC0, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x03, 0x00}, \
  {0x0347, 0x60, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0xD7, 0x00}, \
  {0x034A, 0x08, 0x00}, \
  {0x034B, 0x7F, 0x00}, \
  {0x0220, 0x00, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x01, 0x00}, \
  {0x0901, 0x22, 0x00}, \
  {0x0902, 0x02, 0x00}, \
  {0x3140, 0x02, 0x00}, \
  {0x3c00, 0x00, 0x00}, \
  {0x3c01, 0x01, 0x00}, \
  {0x3c02, 0x9C, 0x00}, \
  {0x3F0D, 0x00, 0x00}, \
  {0x5748, 0x00, 0x00}, \
  {0x5749, 0x00, 0x00}, \
  {0x574a, 0x00, 0x00}, \
  {0x574b, 0xA4, 0x00}, \
  {0x7B53, 0x00, 0x00}, \
  {0x9369, 0x73, 0x00}, \
  {0x936B, 0x64, 0x00}, \
  {0x936D, 0x5F, 0x00}, \
  {0x9304, 0x00, 0x00}, \
  {0x9305, 0x80, 0x00}, \
  {0x9E9A, 0x2F, 0x00}, \
  {0x9E9B, 0x2F, 0x00}, \
  {0x9E9C, 0x2F, 0x00}, \
  {0x9E9D, 0x00, 0x00}, \
  {0x9E9E, 0x00, 0x00}, \
  {0x9E9F, 0x00, 0x00}, \
  {0xA2A9, 0x27, 0x00}, \
  {0xA2B5, 0x30, 0x00}, \
  {0xA2B7, 0x03, 0x00}, \
  {0x0401, 0x00, 0x00}, \
  {0x0404, 0x00, 0x00}, \
  {0x0405, 0x10, 0x00}, \
  {0x0408, 0x01, 0x00}, \
  {0x0409, 0x76, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x00, 0x00}, \
  {0x040C, 0x05, 0x00}, \
  {0x040D, 0x00, 0x00}, \
  {0x040E, 0x02, 0x00}, \
  {0x040F, 0xD0, 0x00}, \
  {0x034C, 0x05, 0x00}, \
  {0x034D, 0x00, 0x00}, \
  {0x034E, 0x02, 0x00}, \
  {0x034F, 0xD0, 0x00}, \
  {0x0301, 0x05, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0xD2, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x02, 0x00}, \
  {0x030D, 0x02, 0x00}, \
  {0x030E, 0x01, 0x00}, \
  {0x030F, 0x5E, 0x00}, \
  {0x0310, 0x00, 0x00}, \
  {0x0820, 0x09, 0x00}, \
  {0x0821, 0xD8, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3E20, 0x01, 0x00}, \
  {0x3E37, 0x01, 0x00}, \
  {0x3F50, 0x00, 0x00}, \
  {0x3F56, 0x00, 0x00}, \
  {0x3F57, 0x8E, 0x00}, \
  /*PDAF Setting */ \
  {PD_AREA_X_OFFSET_ADDR, (int)(SCALE_X(1280)*PD_AREA_X_OFFSET)>>8, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR+1, (int)(SCALE_X(1280)*PD_AREA_X_OFFSET)&0xFF, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR, (int)(SCALE_Y(720)*PD_AREA_Y_OFFSET)>>8, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR+1, (int)(SCALE_Y(720)*PD_AREA_Y_OFFSET)&0xFF, 0x00},\
  {PD_AREA_WIDTH_ADDR, (int)(SCALE_X(1280)*PD_AREA_WIDTH)>>8, 0x00}, \
  {PD_AREA_WIDTH_ADDR+1, (int)(SCALE_X(1280)*PD_AREA_WIDTH)&0xFF, 0x00}, \
  {PD_AREA_HEIGHT_ADDR, (int)(SCALE_Y(720)*PD_AREA_HEIGHT)>>8, 0x00}, \
  {PD_AREA_HEIGHT_ADDR+1, (int)(SCALE_Y(720)*PD_AREA_HEIGHT)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR, (int)(SCALE_X(1280)*PD_WIN_X_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+1, (int)(SCALE_X(1280)*PD_WIN_X_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+2, (int)(SCALE_Y(720)*PD_WIN_Y_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+3, (int)(SCALE_Y(720)*PD_WIN_Y_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+4, (int)(SCALE_X(1280)*PD_WIN_X_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+5, (int)(SCALE_X(1280)*PD_WIN_X_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+6, (int)(SCALE_Y(720)*PD_WIN_Y_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+7, (int)(SCALE_Y(720)*PD_WIN_Y_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+8, (int)(SCALE_X(1280)*PD_WIN_X_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+9, (int)(SCALE_X(1280)*PD_WIN_X_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+10, (int)(SCALE_Y(720)*PD_WIN_Y_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+11, (int)(SCALE_Y(720)*PD_WIN_Y_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+12, (int)(SCALE_X(1280)*PD_WIN_X_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+13, (int)(SCALE_X(1280)*PD_WIN_X_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+14, (int)(SCALE_Y(720)*PD_WIN_Y_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+15, (int)(SCALE_Y(720)*PD_WIN_Y_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+16, (int)(SCALE_X(1280)*PD_WIN_X_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+17, (int)(SCALE_X(1280)*PD_WIN_X_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+18, (int)(SCALE_Y(720)*PD_WIN_Y_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+19, (int)(SCALE_Y(720)*PD_WIN_Y_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+20, (int)(SCALE_X(1280)*PD_WIN_X_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+21, (int)(SCALE_X(1280)*PD_WIN_X_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+22, (int)(SCALE_Y(720)*PD_WIN_Y_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+23, (int)(SCALE_Y(720)*PD_WIN_Y_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+24, (int)(SCALE_X(1280)*PD_WIN_X_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+25, (int)(SCALE_X(1280)*PD_WIN_X_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+26, (int)(SCALE_Y(720)*PD_WIN_Y_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+27, (int)(SCALE_Y(720)*PD_WIN_Y_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+28, (int)(SCALE_X(1280)*PD_WIN_X_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+29, (int)(SCALE_X(1280)*PD_WIN_X_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+30, (int)(SCALE_Y(720)*PD_WIN_Y_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+31, (int)(SCALE_Y(720)*PD_WIN_Y_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+32, (int)(SCALE_X(1280)*PD_WIN_X_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+33, (int)(SCALE_X(1280)*PD_WIN_X_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+34, (int)(SCALE_Y(720)*PD_WIN_Y_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+35, (int)(SCALE_Y(720)*PD_WIN_Y_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+36, (int)(SCALE_X(1280)*PD_WIN_X_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+37, (int)(SCALE_X(1280)*PD_WIN_X_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+38, (int)(SCALE_Y(720)*PD_WIN_Y_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+39, (int)(SCALE_Y(720)*PD_WIN_Y_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+40, (int)(SCALE_X(1280)*PD_WIN_X_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+41, (int)(SCALE_X(1280)*PD_WIN_X_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+42, (int)(SCALE_Y(720)*PD_WIN_Y_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+43, (int)(SCALE_Y(720)*PD_WIN_Y_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+44, (int)(SCALE_X(1280)*PD_WIN_X_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+45, (int)(SCALE_X(1280)*PD_WIN_X_E_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+46, (int)(SCALE_Y(720)*PD_WIN_Y_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+47, (int)(SCALE_Y(720)*PD_WIN_Y_E_5)&0xFF, 0x00} \
}

#define RES10_REG_ARRAY \
{\
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0342, 0x12, 0x00}, \
  {0x0343, 0xC0, 0x00}, \
  {0x0340, 0x0D, 0x00}, \
  {0x0341, 0x04, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x01, 0x00}, \
  {0x0347, 0x78, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0xD7, 0x00}, \
  {0x034A, 0x0A, 0x00}, \
  {0x034B, 0x67, 0x00}, \
  {0x0220, 0x03, 0x00}, \
  {0x0221, 0x12, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x00, 0x00}, \
  {0x0901, 0x11, 0x00}, \
  {0x0902, 0x02, 0x00}, \
  {0x3140, 0x02, 0x00}, \
  {0x3c00, 0x01, 0x00}, \
  {0x3c01, 0x03, 0x00}, \
  {0x3c02, 0xDC, 0x00}, \
  {0x3F0D, 0x00, 0x00}, \
  {0x5748, 0x07, 0x00}, \
  {0x5749, 0xFF, 0x00}, \
  {0x574a, 0x00, 0x00}, \
  {0x574b, 0x00, 0x00}, \
  {0x7B53, 0x01, 0x00}, \
  {0x9369, 0x5A, 0x00}, \
  {0x936B, 0x55, 0x00}, \
  {0x936D, 0x28, 0x00}, \
  {0x9304, 0x00, 0x00}, \
  {0x9305, 0x00, 0x00}, \
  {0x9E9A, 0x2F, 0x00}, \
  {0x9E9B, 0x2F, 0x00}, \
  {0x9E9C, 0x2F, 0x00}, \
  {0x9E9D, 0x00, 0x00}, \
  {0x9E9E, 0x00, 0x00}, \
  {0x9E9F, 0x00, 0x00}, \
  {0xA2A9, 0x60, 0x00}, \
  {0xA2B5, 0x50, 0x00}, \
  {0xA2B7, 0x00, 0x00}, \
  {0x0401, 0x00, 0x00}, \
  {0x0404, 0x00, 0x00}, \
  {0x0405, 0x10, 0x00}, \
  {0x0408, 0x03, 0x00}, \
  {0x0409, 0xF6, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x00, 0x00}, \
  {0x040C, 0x07, 0x00}, \
  {0x040D, 0xEC, 0x00}, \
  {0x040E, 0x04, 0x00}, \
  {0x040F, 0x78, 0x00}, \
  {0x034C, 0x07, 0x00}, \
  {0x034D, 0xEC, 0x00}, \
  {0x034E, 0x04, 0x00}, \
  {0x034F, 0x78, 0x00}, \
  {0x0301, 0x05, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0xC8, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x02, 0x00}, \
  {0x030D, 0x02, 0x00}, \
  {0x030E, 0x01, 0x00}, \
  {0x030F, 0x5E, 0x00}, \
  {0x0310, 0x00, 0x00}, \
  {0x0820, 0x09, 0x00}, \
  {0x0821, 0x60, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3E20, 0x01, 0x00}, \
  {0x3E37, 0x01, 0x00}, \
  {0x3F50, 0x00, 0x00}, \
  {0x3F56, 0x00, 0x00}, \
  {0x3F57, 0xF0, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR, (int)(SCALE_X(2028)*PD_AREA_X_OFFSET)>>8, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR+1, (int)(SCALE_X(2028)*PD_AREA_X_OFFSET)&0xFF, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR, (int)(SCALE_Y(1144)*PD_AREA_Y_OFFSET)>>8, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR+1, (int)(SCALE_Y(1144)*PD_AREA_Y_OFFSET)&0xFF, 0x00},\
  {PD_AREA_WIDTH_ADDR, (int)(SCALE_X(2028)*PD_AREA_WIDTH)>>8, 0x00}, \
  {PD_AREA_WIDTH_ADDR+1, (int)(SCALE_X(2028)*PD_AREA_WIDTH)&0xFF, 0x00}, \
  {PD_AREA_HEIGHT_ADDR, (int)(SCALE_Y(1144)*PD_AREA_HEIGHT)>>8, 0x00}, \
  {PD_AREA_HEIGHT_ADDR+1, (int)(SCALE_Y(1144)*PD_AREA_HEIGHT)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR, (int)(SCALE_X(2028)*PD_WIN_X_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+1, (int)(SCALE_X(2028)*PD_WIN_X_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+2, (int)(SCALE_Y(1144)*PD_WIN_Y_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+3, (int)(SCALE_Y(1144)*PD_WIN_Y_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+4, (int)(SCALE_X(2028)*PD_WIN_X_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+5, (int)(SCALE_X(2028)*PD_WIN_X_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+6, (int)(SCALE_Y(1144)*PD_WIN_Y_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+7, (int)(SCALE_Y(1144)*PD_WIN_Y_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+8, (int)(SCALE_X(2028)*PD_WIN_X_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+9, (int)(SCALE_X(2028)*PD_WIN_X_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+10, (int)(SCALE_Y(1144)*PD_WIN_Y_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+11, (int)(SCALE_Y(1144)*PD_WIN_Y_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+12, (int)(SCALE_X(2028)*PD_WIN_X_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+13, (int)(SCALE_X(2028)*PD_WIN_X_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+14, (int)(SCALE_Y(1144)*PD_WIN_Y_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+15, (int)(SCALE_Y(1144)*PD_WIN_Y_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+16, (int)(SCALE_X(2028)*PD_WIN_X_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+17, (int)(SCALE_X(2028)*PD_WIN_X_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+18, (int)(SCALE_Y(1144)*PD_WIN_Y_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+19, (int)(SCALE_Y(1144)*PD_WIN_Y_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+20, (int)(SCALE_X(2028)*PD_WIN_X_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+21, (int)(SCALE_X(2028)*PD_WIN_X_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+22, (int)(SCALE_Y(1144)*PD_WIN_Y_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+23, (int)(SCALE_Y(1144)*PD_WIN_Y_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+24, (int)(SCALE_X(2028)*PD_WIN_X_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+25, (int)(SCALE_X(2028)*PD_WIN_X_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+26, (int)(SCALE_Y(1144)*PD_WIN_Y_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+27, (int)(SCALE_Y(1144)*PD_WIN_Y_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+28, (int)(SCALE_X(2028)*PD_WIN_X_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+29, (int)(SCALE_X(2028)*PD_WIN_X_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+30, (int)(SCALE_Y(1144)*PD_WIN_Y_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+31, (int)(SCALE_Y(1144)*PD_WIN_Y_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+32, (int)(SCALE_X(2028)*PD_WIN_X_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+33, (int)(SCALE_X(2028)*PD_WIN_X_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+34, (int)(SCALE_Y(1144)*PD_WIN_Y_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+35, (int)(SCALE_Y(1144)*PD_WIN_Y_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+36, (int)(SCALE_X(2028)*PD_WIN_X_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+37, (int)(SCALE_X(2028)*PD_WIN_X_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+38, (int)(SCALE_Y(1144)*PD_WIN_Y_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+39, (int)(SCALE_Y(1144)*PD_WIN_Y_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+40, (int)(SCALE_X(2028)*PD_WIN_X_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+41, (int)(SCALE_X(2028)*PD_WIN_X_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+42, (int)(SCALE_Y(1144)*PD_WIN_Y_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+43, (int)(SCALE_Y(1144)*PD_WIN_Y_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+44, (int)(SCALE_X(2028)*PD_WIN_X_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+45, (int)(SCALE_X(2028)*PD_WIN_X_E_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+46, (int)(SCALE_Y(1144)*PD_WIN_Y_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+47, (int)(SCALE_Y(1144)*PD_WIN_Y_E_5)&0xFF, 0x00}, \
}

#define RES11_REG_ARRAY \
{ \
  {0x0112, 0x0A, 0x00}, \
  {0x0113, 0x0A, 0x00}, \
  {0x0114, 0x03, 0x00}, \
  {0x0342, 0x11, 0x00}, \
  {0x0343, 0xA0, 0x00}, \
  {0x0340, 0x0D, 0x00}, \
  {0x0341, 0xD8, 0x00}, \
  {0x0344, 0x00, 0x00}, \
  {0x0345, 0x00, 0x00}, \
  {0x0346, 0x01, 0x00}, \
  {0x0347, 0x78, 0x00}, \
  {0x0348, 0x0F, 0x00}, \
  {0x0349, 0xD7, 0x00}, \
  {0x034A, 0x0A, 0x00}, \
  {0x034B, 0x67, 0x00}, \
  {0x0220, 0x03, 0x00}, \
  {0x0221, 0x11, 0x00}, \
  {0x0381, 0x01, 0x00}, \
  {0x0383, 0x01, 0x00}, \
  {0x0385, 0x01, 0x00}, \
  {0x0387, 0x01, 0x00}, \
  {0x0900, 0x00, 0x00}, \
  {0x0901, 0x11, 0x00}, \
  {0x0902, 0x02, 0x00}, \
  {0x3140, 0x02, 0x00}, \
  {0x3c00, 0x00, 0x00}, \
  {0x3c01, 0x03, 0x00}, \
  {0x3c02, 0xDC, 0x00}, \
  {0x3F0D, 0x00, 0x00}, \
  {0x5748, 0x07, 0x00}, \
  {0x5749, 0xFF, 0x00}, \
  {0x574a, 0x00, 0x00}, \
  {0x574b, 0x00, 0x00}, \
  {0x7B53, 0x01, 0x00}, \
  {0x9369, 0x5A, 0x00}, \
  {0x936B, 0x55, 0x00}, \
  {0x936D, 0x28, 0x00}, \
  {0x9304, 0x00, 0x00}, \
  {0x9305, 0x00, 0x00}, \
  {0x9E9A, 0x2F, 0x00}, \
  {0x9E9B, 0x2F, 0x00}, \
  {0x9E9C, 0x2F, 0x00}, \
  {0x9E9D, 0x00, 0x00}, \
  {0x9E9E, 0x00, 0x00}, \
  {0x9E9F, 0x00, 0x00}, \
  {0xA2A9, 0x60, 0x00}, \
  {0xA2B5, 0x50, 0x00}, \
  {0xA2B7, 0x00, 0x00}, \
  {0x0401, 0x00, 0x00}, \
  {0x0404, 0x00, 0x00}, \
  {0x0405, 0x10, 0x00}, \
  {0x0408, 0x00, 0x00}, \
  {0x0409, 0x6C, 0x00}, \
  {0x040A, 0x00, 0x00}, \
  {0x040B, 0x40, 0x00}, \
  {0x040C, 0x0F, 0x00}, \
  {0x040D, 0x00, 0x00}, \
  {0x040E, 0x08, 0x00}, \
  {0x040F, 0x70, 0x00}, \
  {0x034C, 0x0F, 0x00}, \
  {0x034D, 0x00, 0x00}, \
  {0x034E, 0x08, 0x00}, \
  {0x034F, 0x70, 0x00}, \
  {0x0301, 0x05, 0x00}, \
  {0x0303, 0x02, 0x00}, \
  {0x0305, 0x04, 0x00}, \
  {0x0306, 0x00, 0x00}, \
  {0x0307, 0xC8, 0x00}, \
  {0x0309, 0x0A, 0x00}, \
  {0x030B, 0x01, 0x00}, \
  {0x030D, 0x02, 0x00}, \
  {0x030E, 0x01, 0x00}, \
  {0x030F, 0x5E, 0x00}, \
  {0x0310, 0x00, 0x00}, \
  {0x0820, 0x12, 0x00}, \
  {0x0821, 0xC0, 0x00}, \
  {0x0822, 0x00, 0x00}, \
  {0x0823, 0x00, 0x00}, \
  {0x3E20, 0x01, 0x00}, \
  {0x3E37, 0x01, 0x00}, \
  {0x3F50, 0x00, 0x00}, \
  {0x3F56, 0x00, 0x00}, \
  {0x3F57, 0xE2, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR, (int)(SCALE_X(3840)*PD_AREA_X_OFFSET)>>8, 0x00}, \
  {PD_AREA_X_OFFSET_ADDR+1, (int)(SCALE_X(3840)*PD_AREA_X_OFFSET)&0xFF, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR, (int)(SCALE_Y(2160)*PD_AREA_Y_OFFSET)>>8, 0x00},\
  {PD_AREA_Y_OFFSET_ADDR+1, (int)(SCALE_Y(2160)*PD_AREA_Y_OFFSET)&0xFF, 0x00},\
  {PD_AREA_WIDTH_ADDR, (int)(SCALE_X(3840)*PD_AREA_WIDTH)>>8, 0x00}, \
  {PD_AREA_WIDTH_ADDR+1, (int)(SCALE_X(3840)*PD_AREA_WIDTH)&0xFF, 0x00}, \
  {PD_AREA_HEIGHT_ADDR, (int)(SCALE_Y(2160)*PD_AREA_HEIGHT)>>8, 0x00}, \
  {PD_AREA_HEIGHT_ADDR+1, (int)(SCALE_Y(2160)*PD_AREA_HEIGHT)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR, (int)(SCALE_X(3840)*PD_WIN_X_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+1, (int)(SCALE_X(3840)*PD_WIN_X_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+2, (int)(SCALE_Y(2160)*PD_WIN_Y_S_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+3, (int)(SCALE_Y(2160)*PD_WIN_Y_S_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+4, (int)(SCALE_X(3840)*PD_WIN_X_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+5, (int)(SCALE_X(3840)*PD_WIN_X_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+6, (int)(SCALE_Y(2160)*PD_WIN_Y_E_0)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+7, (int)(SCALE_Y(2160)*PD_WIN_Y_E_0)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+8, (int)(SCALE_X(3840)*PD_WIN_X_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+9, (int)(SCALE_X(3840)*PD_WIN_X_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+10, (int)(SCALE_Y(2160)*PD_WIN_Y_S_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+11, (int)(SCALE_Y(2160)*PD_WIN_Y_S_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+12, (int)(SCALE_X(3840)*PD_WIN_X_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+13, (int)(SCALE_X(3840)*PD_WIN_X_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+14, (int)(SCALE_Y(2160)*PD_WIN_Y_E_1)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+15, (int)(SCALE_Y(2160)*PD_WIN_Y_E_1)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+16, (int)(SCALE_X(3840)*PD_WIN_X_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+17, (int)(SCALE_X(3840)*PD_WIN_X_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+18, (int)(SCALE_Y(2160)*PD_WIN_Y_S_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+19, (int)(SCALE_Y(2160)*PD_WIN_Y_S_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+20, (int)(SCALE_X(3840)*PD_WIN_X_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+21, (int)(SCALE_X(3840)*PD_WIN_X_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+22, (int)(SCALE_Y(2160)*PD_WIN_Y_E_2)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+23, (int)(SCALE_Y(2160)*PD_WIN_Y_E_2)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+24, (int)(SCALE_X(3840)*PD_WIN_X_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+25, (int)(SCALE_X(3840)*PD_WIN_X_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+26, (int)(SCALE_Y(2160)*PD_WIN_Y_S_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+27, (int)(SCALE_Y(2160)*PD_WIN_Y_S_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+28, (int)(SCALE_X(3840)*PD_WIN_X_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+29, (int)(SCALE_X(3840)*PD_WIN_X_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+30, (int)(SCALE_Y(2160)*PD_WIN_Y_E_3)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+31, (int)(SCALE_Y(2160)*PD_WIN_Y_E_3)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+32, (int)(SCALE_X(3840)*PD_WIN_X_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+33, (int)(SCALE_X(3840)*PD_WIN_X_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+34, (int)(SCALE_Y(2160)*PD_WIN_Y_S_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+35, (int)(SCALE_Y(2160)*PD_WIN_Y_S_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+36, (int)(SCALE_X(3840)*PD_WIN_X_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+37, (int)(SCALE_X(3840)*PD_WIN_X_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+38, (int)(SCALE_Y(2160)*PD_WIN_Y_E_4)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+39, (int)(SCALE_Y(2160)*PD_WIN_Y_E_4)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+40, (int)(SCALE_X(3840)*PD_WIN_X_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+41, (int)(SCALE_X(3840)*PD_WIN_X_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+42, (int)(SCALE_Y(2160)*PD_WIN_Y_S_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+43, (int)(SCALE_Y(2160)*PD_WIN_Y_S_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+44, (int)(SCALE_X(3840)*PD_WIN_X_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+45, (int)(SCALE_X(3840)*PD_WIN_X_E_5)&0xFF, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+46, (int)(SCALE_Y(2160)*PD_WIN_Y_E_5)>>8, 0x00}, \
  {FLEX_AREA_XSTA0_ADDR+47, (int)(SCALE_Y(2160)*PD_WIN_Y_E_5)&0xFF, 0x00}, \
}




/* Sensor Handler */
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
      .sensor_id_reg_addr = 0x0016,
      .sensor_id = 0x0477,
    },
    .power_setting_array =
    {
      .power_setting_a =
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
    .vert_offset = IMX378_MAX_INTEGRATION_MARGIN,
  },
  .aec_info =
  {
    .min_gain = IMX378_MIN_GAIN,
    .max_gain = IMX378_MAX_GAIN,
    .max_analog_gain = IMX378_MAX_AGAIN,
    .max_linecount = 65535 - IMX378_MAX_INTEGRATION_MARGIN,
  },
  .sensor_num_frame_skip = 2,
  .sensor_num_HDR_frame_skip = 2,
  .sensor_max_pipeline_frame_delay = 2,
  .sensor_property =
  {
    .pix_size = 1.0, /* um */
    .sensing_method = SENSOR_SMETHOD_ONE_CHIP_COLOR_AREA_SENSOR,
    .crop_factor = 1.0,
  },
  .pixel_array_size_info =
  {
    .active_array_size =
    {
      .width = 4056,
      .height = 3040,
    },
    .left_dummy = 12,
    .right_dummy = 12,
    .top_dummy = 16,
    .bottom_dummy = 16,
  },
  .color_level_info =
  {
    .white_level = 1023,
    .r_pedestal = IMX378_DATA_PEDESTAL,
    .gr_pedestal = IMX378_DATA_PEDESTAL,
    .gb_pedestal = IMX378_DATA_PEDESTAL,
    .b_pedestal = IMX378_DATA_PEDESTAL,
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
            .cid = 1,
            .dt = IMX378_CSI_PD_ISTATS,
            .decode_format = CSI_DECODE_10BIT,
          },

        },
        .pix_data_fmt =
        {
          SENSOR_BAYER,
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
            {0x0600, 0x0000, 0x00},
          },
          .size = 1,
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
          .size = 1,
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
          .size = 1,
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
          .size = 1,
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
          .size = 1,
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
      /* Res 7 */
      {
        .reg_setting_a = RES7_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 0,
      },
      /* Res 8 */
      {
        .reg_setting_a = RES8_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 0,
      },
      /* Res 9 */
      {
        .reg_setting_a = RES9_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 0,
      },
      /* Res 10 */
      {
        .reg_setting_a = RES10_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 0,
      },
      /* Res 11 */
      {
        .reg_setting_a = RES11_REG_ARRAY,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 0,
      },
    },
    .size = 12,
  },
  .out_info_array =
  {
    .out_info =
    {
      /* Res 0 */
      {
        .x_output = 4056,
        .y_output = 3040,
        .line_length_pclk = 4512,
        .frame_length_lines = 3544,
        .vt_pixel_clk = 480000000,
        .op_pixel_clk = 480000000,
        .binning_factor = 1,
        .min_fps = 7.50,
        .max_fps = 30.01,
        .mode = SENSOR_DEFAULT_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0.000,
        .is_pdaf_supported = 1,
      },
      /* Res 1 */
      {
        .x_output = 4056,
        .y_output = 2288,
        .line_length_pclk = 4512,
        .frame_length_lines = 3544,
        .vt_pixel_clk = 480000000,
        .op_pixel_clk = 480000000,
        .binning_factor = 1,
        .min_fps = 7.50,
        .max_fps = 30.01,
        .mode = SENSOR_DEFAULT_MODE,
        .offset_x = 0,
        .offset_y = 376,
        .scale_factor = 0.000,
        .is_pdaf_supported = 1,
      },
      /* Res 2 */
      {
        .x_output = 3840,
        .y_output = 2160,
        .line_length_pclk = 4512,
        .frame_length_lines = 3544,
        .vt_pixel_clk = 480000000,
        .op_pixel_clk = 480000000,
        .binning_factor = 1,
        .min_fps = 7.50,
        .max_fps = 30.01,
        .mode = SENSOR_DEFAULT_MODE,
        .offset_x = 0,
        .offset_y = 376,
        .scale_factor = 0.000,
        .is_pdaf_supported = 1,
      },
      /* Res 3 */
      {
        .x_output = 2028,
        .y_output = 1520,
        .line_length_pclk = 2976,
        .frame_length_lines = 5372,
        .vt_pixel_clk = 480000000,
        .op_pixel_clk = 480000000,
        .binning_factor = 2,
        .min_fps = 7.50,
        .max_fps = 30.02,
        .mode = SENSOR_DEFAULT_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0.000,
        .is_pdaf_supported = 1,
      },
      /* Res 4 */
      {
        .x_output = 2028,
        .y_output = 1144,
        .line_length_pclk = 4800,
        .frame_length_lines = 3332,
        .vt_pixel_clk = 480000000,
        .op_pixel_clk = 240000000,
        .binning_factor = 1,
        .min_fps = 7.50,
        .max_fps = 30.01,
        .mode = SENSOR_HFR_MODE,
        .offset_x = 0,
        .offset_y = 376,
        .scale_factor = 0.000,
        .is_pdaf_supported = 1,
      },
      /* Res 5 */
      {
        .x_output = 3840,
        .y_output = 2160,
        .line_length_pclk = 6292,
        .frame_length_lines = 2224,
        .vt_pixel_clk = 840000000,
        .op_pixel_clk = 547000000,
        .binning_factor = 1,
        .min_fps = 7.50,
        .max_fps = 60.02,
        .mode = SENSOR_HFR_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0.000,
        .is_pdaf_supported = 1,
      },
      /* Res 6 */
      {
        .x_output = 1920,
        .y_output = 1080,
        .line_length_pclk = 4512,
        .frame_length_lines = 1772,
        .vt_pixel_clk = 480000000,
        .op_pixel_clk = 240000000,
        .binning_factor = 2,
        .min_fps = 7.50,
        .max_fps = 60.03,
        .mode = SENSOR_HFR_MODE,
        .offset_x = 0,
        .offset_y = 376,
        .scale_factor = 0.000,
        .is_pdaf_supported = 1,
      },
      /* Res 7 */
      {
        .x_output = 1920,
        .y_output = 1080,
        .line_length_pclk = 4512,
        .frame_length_lines = 1192,
        .vt_pixel_clk = 485000000,
        .op_pixel_clk = 440000000,
        .binning_factor = 2,
        .min_fps = 7.50,
        .max_fps = 90.13,
        .mode = SENSOR_HFR_MODE,
        .offset_x = 0,
        .offset_y = 376,
        .scale_factor = 0.000,
        .is_pdaf_supported = 1,
      },
      /* Res 8 */
      {
        .x_output = 1920,
        .y_output = 1080,
        .line_length_pclk = 4512,
        .frame_length_lines = 1196,
        .vt_pixel_clk = 648000000,
        .op_pixel_clk = 324000000,
        .binning_factor = 2,
        .min_fps = 7.50,
        .max_fps = 120.00,
        .mode = SENSOR_HFR_MODE,
        .offset_x = 0,
        .offset_y = 376,
        .scale_factor = 0.000,
        .is_pdaf_supported = 1,
      },
      /* Res 9 */
      {
        .x_output = 1280,
        .y_output = 720,
        .line_length_pclk = 2976,
        .frame_length_lines = 704,
        .vt_pixel_clk = 504000000,
        .op_pixel_clk = 252000000,
        .binning_factor = 2,
        .min_fps = 7.50,
        .max_fps = 240.00,
        .mode = SENSOR_HFR_MODE,
        .offset_x = 0,
        .offset_y = 864,
        .scale_factor = 0.000,
        .is_pdaf_supported = 1,
      },
      /* Res 10 */
      {
        .x_output = 2028,
        .y_output = 1144,
        .line_length_pclk = 4800,
        .frame_length_lines = 3332,
        .vt_pixel_clk = 480000000,
        .op_pixel_clk = 240000000,
        .binning_factor = 1,
        .min_fps = 7.50,
        .max_fps = 30.01,
        .mode = SENSOR_RAW_HDR_MODE,
        .offset_x = 0,
        .offset_y = 376,
        .scale_factor = 0.000,
        .is_pdaf_supported = 1,
      },
      /* Res 11 */
      {
        .x_output = 3840,
        .y_output = 2160,
        .line_length_pclk = 4512,
        .frame_length_lines = 3544,
        .vt_pixel_clk = 480000000,
        .op_pixel_clk = 480000000,
        .binning_factor = 1,
        .min_fps = 7.50,
        .max_fps = 30.01,
        .mode = SENSOR_RAW_HDR_MODE,
        .offset_x = 0,
        .offset_y = 0,
        .scale_factor = 0.000,
        .is_pdaf_supported = 1,
      },
    },
    .size = 12,
  },
  .csi_params =
  {
    .lane_cnt = 4,
    .settle_cnt = 0xb,
    .is_csi_3phase = 0,
  },
  .csid_lut_params_array =
  {
    .lut_params =
    {
      /* Res 0 */
      {
        .num_cid = 2,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 1,
            .dt = IMX378_CSI_PD_ISTATS,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
      /* Res 1 */
      {
        .num_cid = 2,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 1,
            .dt = IMX378_CSI_PD_ISTATS,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
      /* Res 2 */
      {
        .num_cid = 2,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 1,
            .dt = IMX378_CSI_PD_ISTATS,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
      /* Res 3 */
      {
        .num_cid = 2,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 1,
            .dt = IMX378_CSI_PD_ISTATS,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
      /* Res 4 */
      {
        .num_cid = 2,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 1,
            .dt = IMX378_CSI_PD_ISTATS,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
      /* Res 5 */
      {
        .num_cid = 2,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 1,
            .dt = IMX378_CSI_PD_ISTATS,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
      /* Res 6 */
      {
        .num_cid = 2,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 1,
            .dt = IMX378_CSI_PD_ISTATS,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
      /* Res 7 */
      {
        .num_cid = 2,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 1,
            .dt = IMX378_CSI_PD_ISTATS,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
      /* Res 8 */
      {
        .num_cid = 2,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 1,
            .dt = IMX378_CSI_PD_ISTATS,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
      /* Res 9 */
      {
        .num_cid = 2,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 1,
            .dt = IMX378_CSI_PD_ISTATS,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
      /* Res 10 */
      {
        .num_cid = 2,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 1,
            .dt = IMX378_CSI_PD_ISTATS,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
      /* Res 11 */
      {
        .num_cid = 2,
        .vc_cfg_a =
        {
          {
            .cid = 0,
            .dt = CSI_RAW10,
            .decode_format = CSI_DECODE_10BIT
          },
          {
            .cid = 1,
            .dt = IMX378_CSI_PD_ISTATS,
            .decode_format = CSI_DECODE_10BIT
          },
        },
      },
    },
    .size = 12,
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
      /* Res 7 */
      {
        .top_crop = 0,
        .bottom_crop = 0,
        .left_crop = 0,
        .right_crop = 0,
      },
      /* Res 8 */
      {
        .top_crop = 0,
        .bottom_crop = 0,
        .left_crop = 0,
        .right_crop = 0,
      },
      /* Res 9 */
      {
        .top_crop = 0,
        .bottom_crop = 0,
        .left_crop = 0,
        .right_crop = 0,
      },
      /* Res 10 */
      {
        .top_crop = 0,
        .bottom_crop = 0,
        .left_crop = 0,
        .right_crop = 0,
      },
      /* Res 11 */
      {
        .top_crop = 0,
        .bottom_crop = 0,
        .left_crop = 0,
        .right_crop = 0,
      },
   },
    .size = 12,
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
        .width = 4056,
        .height = 2,
        .stats_type = PD_STATS,
        .dt = IMX378_CSI_PD_ISTATS,
      },
    },
    .size = 1,
  },
  .sensor_capability = (SENSOR_VIDEO_HDR_FLAG | SENSOR_SNAPSHOT_HDR_FLAG),
  .parse_RDI_stats =
  {
    .pd_data_format = SENSOR_STATS_RAW10_11B_CONF_11B_PD,
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
  .sensorlib_pdaf_api =
  {
    .calcdefocus = "imx378_pdaf_calculate_defocus",
  },
  .noise_coeff = {
    .gradient_S = 3.738032e-06,
    .offset_S = 3.651935e-04,
    .gradient_O = 4.499952e-07,
    .offset_O = -2.968624e-04,
  },
};

#endif /* __IMX378_LIB_H__ */
