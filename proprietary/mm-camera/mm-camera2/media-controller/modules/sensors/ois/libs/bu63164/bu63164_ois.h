/*============================================================================
  Copyright (c) 2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/
{
  .ois_params =
  {
    .module_name = "rohm",
    .ois_name = "bu63164",
    .i2c_addr = 0x1c,
    .i2c_freq_mode = SENSOR_I2C_MODE_FAST,
    .fw_flag = 1,
    .opcode =
    {
      .prog = 0x80,
      .coeff = 0x88,
      .pheripheral = 0x82,
      .memory = 0x84,
    },
    .init_setting_size = 12,
    .init_settings =
    {
      {
        .reg_addr = 0x8262,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = ((DIV_N & 0xFF) << 8)|( DIV_N >> 8),
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x8263,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = ((DIV_M & 0xFF) << 8)|( DIV_M >> 8),
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x8264,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x6040,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x8260,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x1130,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x8265,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x8000,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x8261,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0280,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x8261,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0380,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x8261,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0988,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 30
      },
      /* delay is for waitting for PLL lock*/
      {
        .reg_addr = 0x8205,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0C00,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 30
      },
      {
        .reg_addr = 0x8205,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0D00,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      /* turn on the DSP after change the clk*/
      {
        .reg_addr = 0x8C,
        .addr_type = CAMERA_I2C_BYTE_ADDR,
        .reg_data = 0x01,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      /* disable OIS for ois_thread init*/
      {
        .reg_addr = EQCTL,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0C0C,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
    },
    .enable_ois_setting_size = 1,
    .enable_ois_settings =
    {
       /* 0D0D means Y-axis position servo on, Y-axis OIS on
                 X-axis position servo on, Y-axis OIS on */
      {
        .reg_addr = EQCTL,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0D0D,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay =0
      },
    },
    .disable_ois_setting_size = 1,
    .disable_ois_settings =
    {
       /* 0D0D means Y-axis position servo on, Y-axis OIS off
                X-axis position servo on, Y-axis OIS off  */
      {
        .reg_addr = EQCTL,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0C0C,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay =0
      },
    },
    .scene_ois_setting_size = 70,
    .scene_ois_settings =
    {
       /* SCENE_PARAM_NIGHT1 */
       {EQCTL, CAMERA_I2C_WORD_ADDR, 0x0C0C, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr, CAMERA_I2C_WORD_ADDR, 0xFE7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT, CAMERA_I2C_WORD_ADDR, 0x3008, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr+0x80, CAMERA_I2C_WORD_ADDR, 0xFE7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT+0x80, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT+0x80, CAMERA_I2C_WORD_ADDR, 0x3008, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
        /* SCENE_PARAM_NIGHT2 */
       {EQCTL, CAMERA_I2C_WORD_ADDR, 0x0C0C, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr, CAMERA_I2C_WORD_ADDR, 0xFC7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT, CAMERA_I2C_WORD_ADDR, 0x3008, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr+0x80, CAMERA_I2C_WORD_ADDR, 0xFC7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT+0x80, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT+0x80, CAMERA_I2C_WORD_ADDR, 0x3008, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
        /* SCENE_PARAM_NIGHT3 */
       {EQCTL, CAMERA_I2C_WORD_ADDR, 0x0C0C, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr, CAMERA_I2C_WORD_ADDR, 0xFA7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT, CAMERA_I2C_WORD_ADDR, 0x3008, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr+0x80, CAMERA_I2C_WORD_ADDR, 0xFA7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT+0x80, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT+0x80, CAMERA_I2C_WORD_ADDR, 0x3008, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
        /* SCENE_PARAM_DAY1 */
       {EQCTL, CAMERA_I2C_WORD_ADDR, 0x0C0C, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr, CAMERA_I2C_WORD_ADDR, 0xFE7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT, CAMERA_I2C_WORD_ADDR, 0x7814, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr+0x80, CAMERA_I2C_WORD_ADDR, 0xFA7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT+0x80, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT+0x80, CAMERA_I2C_WORD_ADDR, 0x3008, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       /* SCENE_PARAM_DAY2 */
       {EQCTL, CAMERA_I2C_WORD_ADDR, 0x0C0C, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr, CAMERA_I2C_WORD_ADDR, 0xFA7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT, CAMERA_I2C_WORD_ADDR, 0x7814, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr+0x80, CAMERA_I2C_WORD_ADDR, 0xFA7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT+0x80, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT+0x80, CAMERA_I2C_WORD_ADDR, 0x7814, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       /* SCENE_PARAM_DAY3 */
       {EQCTL, CAMERA_I2C_WORD_ADDR, 0x0C0C, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr, CAMERA_I2C_WORD_ADDR, 0xF07F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT, CAMERA_I2C_WORD_ADDR, 0x7814, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr+0x80, CAMERA_I2C_WORD_ADDR, 0xF07F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT+0x80, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT+0x80, CAMERA_I2C_WORD_ADDR, 0x7814, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       /* SCENE_PARAM_SPORT1 */
       {EQCTL, CAMERA_I2C_WORD_ADDR, 0x0C0C, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr, CAMERA_I2C_WORD_ADDR, 0xFE7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT, CAMERA_I2C_WORD_ADDR, 0xB41E, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr+0x80, CAMERA_I2C_WORD_ADDR, 0xFE7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT+0x80, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT+0x80, CAMERA_I2C_WORD_ADDR, 0xB41E, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       /* SCENE_PARAM_SPORT2 */
       {EQCTL, CAMERA_I2C_WORD_ADDR, 0x0C0C, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr, CAMERA_I2C_WORD_ADDR, 0xF07F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT, CAMERA_I2C_WORD_ADDR, 0xB41E, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr+0x80, CAMERA_I2C_WORD_ADDR, 0xF07F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT+0x80, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT+0x80, CAMERA_I2C_WORD_ADDR, 0xB41E, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       /* SCENE_PARAM_SPORT3 */
       {EQCTL, CAMERA_I2C_WORD_ADDR, 0x0C4C, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr, CAMERA_I2C_WORD_ADDR, 0xE07F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT, CAMERA_I2C_WORD_ADDR, 0xB41E, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr+0x80, CAMERA_I2C_WORD_ADDR, 0xE07F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT+0x80, CAMERA_I2C_WORD_ADDR, 0x0624, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT+0x80, CAMERA_I2C_WORD_ADDR, 0xB41E, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       /* SCENE_PARAM_TEST */
       {EQCTL, CAMERA_I2C_WORD_ADDR, 0x0C0C, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr, CAMERA_I2C_WORD_ADDR, 0xF07F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT, CAMERA_I2C_WORD_ADDR, 0xFF7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT, CAMERA_I2C_WORD_ADDR, 0xFF7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgxdr+0x80, CAMERA_I2C_WORD_ADDR, 0xF07F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_LMT+0x80, CAMERA_I2C_WORD_ADDR, 0xFF7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {X_TGT+0x80, CAMERA_I2C_WORD_ADDR, 0xFF7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
      },
      .scene_filter_on_setting_size = 13,
      .scene_filter_on_settings =
      {
       {EQCTL, CAMERA_I2C_WORD_ADDR, 0x0C4C, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgx13, CAMERA_I2C_WORD_ADDR, 0x0059, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgx14, CAMERA_I2C_WORD_ADDR, 0x00C7, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgx13, CAMERA_I2C_WORD_ADDR, 0x0059, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgx14, CAMERA_I2C_WORD_ADDR, 0x00C7, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},

       {wDgx02, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {wDgx03, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {wDgx06, CAMERA_I2C_WORD_ADDR, 0xFF7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgx15, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {wDgy02, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {wDgy03, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {wDgy06, CAMERA_I2C_WORD_ADDR, 0xFF7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgy15, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
      },
      .scene_filter_off_setting_size = 13,
      .scene_filter_off_settings =
      {
       {EQCTL, CAMERA_I2C_WORD_ADDR, 0x0C0C, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgx13, CAMERA_I2C_WORD_ADDR, 0x7074, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgx14, CAMERA_I2C_WORD_ADDR, 0x90CB, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgx13, CAMERA_I2C_WORD_ADDR, 0x7074, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgx14, CAMERA_I2C_WORD_ADDR, 0x90CB, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},

       {wDgx02, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {wDgx03, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {wDgx06, CAMERA_I2C_WORD_ADDR, 0xFF7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgx15, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {wDgy02, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {wDgy03, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {wDgy06, CAMERA_I2C_WORD_ADDR, 0xFF7F, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {Kgy15, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
      },
      .scene_range_on_setting_size = 2,
      .scene_range_on_settings =
       /* need to write eeprom FactAdjName_HALOFS_X.data to FactAdjName_HALOFS_X.address
                 FactAdjName_HALOFS_Y.data to FactAdjName_HALOFS_Y.address
                 for now, set the data to 0x0000 */
      {
       {FactAdjName_HALOFS_X, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {FactAdjName_HALOFS_Y, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
      },
      .scene_range_off_setting_size = 2,
      .scene_range_off_settings =
      /* need to write eeprom FactAdjName_SFTHAL_X.data to FactAdjName_SFTHAL_X.address
               FactAdjName_SFTHAL_Y.data to FactAdjName_SFTHAL_Y.address
               for now, set the data to 0x0000 */
      {
       {FactAdjName_HALOFS_X, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
       {FactAdjName_HALOFS_Y, CAMERA_I2C_WORD_ADDR, 0x0000, CAMERA_I2C_WORD_DATA, OIS_WRITE, 0},
      },
    }, /* ois_params_t */
  },
