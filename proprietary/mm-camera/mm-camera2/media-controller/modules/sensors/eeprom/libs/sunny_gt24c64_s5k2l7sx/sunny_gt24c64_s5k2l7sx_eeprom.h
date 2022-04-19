/*============================================================================

Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
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
#define SUNNY_GT24C64_S5K2L7SX_DEBUG
#ifdef SUNNY_GT24C64_S5K2L7SX_DEBUG
#define SLOW(fmt, args...) ALOGE("%s:%d," fmt "\n",__func__,__LINE__, ##args)
#define SDBG(fmt, args...) SERR(fmt, ##args)
#else
#define SLOW(fmt, args...) do { } while (0)
#define SDBG(fmt, args...) do { } while (0)

#endif

#define MODULE_INFO_OFFSET      0

#define AF_OFFSET               0x0706

#define AWB_OFFSET              0x0009

#define LSC_OFFSET              0x001C
#define LSC_R_OFFSET            0x001D
#define LSC_GR_OFFSET           0x01D7
#define LSC_GB_OFFSET           0x0391
#define LSC_B_OFFSET            0x054B
#define LSC_GRID_SIZE           221 // 17 x 13

#define PD_OFFSET               0x0710
#define GAIN_OFFSET             0x0717
#define DCC_LENGTH              48

#define VALID_FLAG              0x01
#define QVALUE                  1024.0
#define INVALID_DATA            0xFFFF

#define PDGAIN_WITDH            17
#define PDGAIN_HEIGHT           13
#define PDGAIN_LENGTH2D         (PDGAIN_HEIGHT * PDGAIN_WITDH)

#define OIS_OFFSET              0x0AF2
#define OIS_SETTING_SIZE        17

#define DC_OFFSET_VALID     0x0B28
#define DC_OFFSET           0x0B29
#define DC_SIZE             470
#define DC_MAIN             (DC_OFFSET + 4)
#define DC_AUX              (DC_OFFSET + 20)
#define DC_ROTATION_MATRIX  9
#define DC_GEOMETRIC_MATRIX 32
#define DC_OTHERS           (DC_OFFSET + 200)
#define AE_SYNC             0x0CFF

/* OIS Init calibration registers */
#define CURDAT      0x8230
#define HALOFS_X    0x8231
#define HALOFS_Y    0x8232
#define HX_OFS      0x841E
#define HY_OFS      0x849E
#define PSTXOF      0x8239
#define PSTYOF      0x823B
#define GX_OFS      0x8406
#define GY_OFS      0x8486
#define KgxHG       0x8446
#define KgyHG       0x84C6
#define KGXG        0x840F
#define KGYG        0x848F
#define TMP_X_      0x846A
#define TMP_Y_      0x846B
#define KgxH0       0x8470
#define KgyH0       0x8472

/* margin is defined corresponding to the margin of golden */
#define INF_MARGIN      -0.15
#define MACRO_MARGIN    0.09

typedef struct {
  unsigned char vendor_id;
  unsigned char chip_id;
  unsigned char project_id;
  unsigned char module_version;
  unsigned char year;
  unsigned char month;
  unsigned char day;
} module_info_t;

typedef struct {
  unsigned char r_over_gr_h;
  unsigned char r_over_gr_l;
  unsigned char b_over_gb_h;
  unsigned char b_over_gb_l;
  unsigned char gr_over_gb_h;
  unsigned char gr_over_gb_l;
  unsigned char r_over_gr_h_golden;
  unsigned char r_over_gr_l_golden;
  unsigned char b_over_gb_h_golden;
  unsigned char b_over_gb_l_golden;
  unsigned char gr_over_gb_h_golden;
  unsigned char gr_over_gb_l_golden;
} awb_data_t;

typedef struct {
  unsigned char macro_h;
  unsigned char macro_l;
  unsigned char infinity_h;
  unsigned char infinity_l;
} af_data_t;

static unsigned char awb_present   = 0;
static unsigned char af_present    = 0;
static unsigned char lsc_present   = 0;
static unsigned char dualc_present = 0;
static unsigned char ois_present   = 0;

/* OIS Raw Data */
struct camera_i2c_seq_reg_setting ois_reg_setting;
struct camera_i2c_seq_reg_array ois_reg_array[OIS_SETTING_SIZE];

void sunny_gt24c64_s5k2l7sx_eeprom_get_calibration_items(void *e_ctrl);
void sunny_gt24c64_s5k2l7sx_eeprom_format_calibration_data(void *e_ctrl);
int  sunny_gt24c64_s5k2l7sx_eeprom_get_ois_raw_data(void *e_ctrl, void *data);

static eeprom_lib_func_t sunny_gt24c64_s5k2l7sx_eeprom_lib_func_ptr = {
  .get_calibration_items = sunny_gt24c64_s5k2l7sx_eeprom_get_calibration_items,
  .format_calibration_data = sunny_gt24c64_s5k2l7sx_eeprom_format_calibration_data,
  .do_af_calibration = eeprom_autofocus_calibration,
  .do_wbc_calibration = eeprom_whitebalance_calibration,
  .do_lsc_calibration = eeprom_lensshading_calibration,
  .get_ois_raw_data = sunny_gt24c64_s5k2l7sx_eeprom_get_ois_raw_data,
  .get_raw_data = NULL,
  .eeprom_info =
  {
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
       },
      .size = 1,
      .power_down_setting_a =
      {
        {
          .seq_type = CAMERA_POW_SEQ_VREG,
          .seq_val = CAMERA_VIO,
          .config_val = 0,
          .delay = 0,
        },
       },
      .size_down = 1,
    },
    .i2c_freq_mode = SENSOR_I2C_MODE_FAST,
    .mem_map_array =
    {
      .memory_map =
      {
        {
          .slave_addr = 0xA0,
          .mem_settings =
          {
            { 0x0000, CAMERA_I2C_WORD_ADDR,
              3327, CAMERA_I2C_BYTE_DATA, CAMERA_I2C_OP_READ, 0 }
          },
          .memory_map_size = 1,
        },
      },
      .size_map_array = 1,
    },
  },
};
