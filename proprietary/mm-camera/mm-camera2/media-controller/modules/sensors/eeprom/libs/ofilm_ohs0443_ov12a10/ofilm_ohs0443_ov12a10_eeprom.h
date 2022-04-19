/*============================================================================

Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../eeprom_util/eeprom.h"
#include "eeprom_lib.h"
#include "eeprom_util.h"
#include "debug_lib.h"
#include "sensor_lib.h"
#include <utils/Log.h>

#undef SLOW
#undef SDBG
#define OFILM_OHS0443_OV12A10_DEBUG
#ifdef OFILM_OHS0443_OV12A10_DEBUG
#define SLOW(fmt, args...) ALOGE("%s:%d," fmt "\n",__func__,__LINE__, ##args)
#define SDBG(fmt, args...) SERR(fmt, ##args)
#else
#define SLOW(fmt, args...) do { } while (0)
#define SDBG(fmt, args...) do { } while (0)
#endif

#define VALID_FLAG              0x01
#define QVALUE                  1024.0
#define INVALID_DATA            0xFFFF

#define MODULE_INFO_OFFSET      0
#define AF_OFFSET               0x0042
#define AWB_OFFSET              0x0056
#define LSC_FLAG                0x0082
#define LSC_OFFSET              0x0086
#define LSC_GRID_SIZE           221 // 17 x 13

#define PD_OFFSET               0x0E72
#define GAIN_OFFSET             0x0E7A
#define DCC_LENGTH              48
#define PDGAIN_WITDH            17
#define PDGAIN_HEIGHT           13
#define PDGAIN_LENGTH2D         (PDGAIN_HEIGHT * PDGAIN_WITDH)


#define DC_OFFSET_VALID     0x1362
#define DC_OFFSET           0x1364
#define DC_SIZE             470
#define DC_MAIN             (DC_OFFSET + 4)
#define DC_AUX              (DC_OFFSET + 20)
#define DC_ROTATION_MATRIX  9
#define DC_GEOMETRIC_MATRIX 32
#define DC_OTHERS           (DC_OFFSET + 200) //0x1442
#define AE_SYNC             0x0E64
#define DC_AF               0x18ED
/* margin is defined corresponding to the margin of golden */
#define INF_MARGIN      -0.15
#define MACRO_MARGIN    0.09

typedef struct {
  unsigned char vendor_id;
  unsigned char year;
  unsigned char month;
  unsigned char day;
  unsigned char hour;
  unsigned char minute;
  unsigned char second;
  unsigned char lens_id;
  unsigned char VCM_id;
  unsigned char driverIC_id;
  unsigned char ois_FW_Ver;
  unsigned char chip_id;
  unsigned char product_id;
  unsigned char module_version;
  unsigned char history_id;
} module_info_t;

typedef struct {
  unsigned char version_num;
  unsigned char mirror_flag;
  unsigned char flip_flag;
  unsigned char r_over_gr_h;
  unsigned char r_over_gr_l;
  unsigned char b_over_gb_h;
  unsigned char b_over_gb_l;
  unsigned char gb_over_gr_h;
  unsigned char gb_over_gr_l;
} awb_data_t;

typedef struct {
  unsigned char version_num;
  unsigned char infinity_h;
  unsigned char infinity_l;
  unsigned char macro_h;
  unsigned char macro_l;
  unsigned char other_posA_h; /* 80cm */
  unsigned char other_posA_l;
  unsigned char other_posB_h; /* 2M */
  unsigned char other_posB_l;
  unsigned char posA_h; /* difference DAC between horizontal and down direction */
  unsigned char posA_l;
  unsigned char posB_h; /* difference DAC between up and horizontal direction */
  unsigned char posB_l;
  unsigned char posC_h; /* difference DAC between up and down direction */
  unsigned char posC_l;
} af_data_t;

static unsigned char awb_present   = 0;
static unsigned char af_present    = 0;
static unsigned char lsc_present   = 0;
static unsigned char dualc_present = 0;
static unsigned char ois_present   = 0;

void ofilm_ohs0443_ov12a10_eeprom_get_calibration_items(void *e_ctrl);
void ofilm_ohs0443_ov12a10_eeprom_format_calibration_data(void *e_ctrl);

static eeprom_lib_func_t ofilm_ohs0443_ov12a10_eeprom_lib_func_ptr = {
  .get_calibration_items = ofilm_ohs0443_ov12a10_eeprom_get_calibration_items,
  .format_calibration_data = ofilm_ohs0443_ov12a10_eeprom_format_calibration_data,
  .do_af_calibration = eeprom_autofocus_calibration,
  .do_wbc_calibration = eeprom_whitebalance_calibration,
  .do_lsc_calibration = eeprom_lensshading_calibration,
  .get_ois_raw_data = NULL,
  .get_raw_data = NULL,
  .eeprom_info =
  {
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
    .i2c_freq_mode = SENSOR_I2C_MODE_FAST,
    .mem_map_array =
    {
      .memory_map =
      {
        {
          .slave_addr = 0xB0,
          .mem_settings =
          {/* 0x0000-0x1902 */
            { 0x0000, CAMERA_I2C_WORD_ADDR,
              6403, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 }
          },
          .memory_map_size = 1,
        },
      },
      .size_map_array = 1,
    },
  },
};
