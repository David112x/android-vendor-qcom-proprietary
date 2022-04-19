/*==========================================================
 Copyright (c) 2014-2015 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ===========================================================*/
{
  .ois_params =
  {
    .module_name = "onsemi", /* module name */
    .ois_name = "lc898122",  /* ois name */
    .i2c_addr = 0x48,        /* I2C Address */
    .i2c_freq_mode = SENSOR_I2C_MODE_FAST, /* I2C frequency mode */
    .init_setting_size = 749,
    .init_settings =
    {
      {
        .reg_addr = 0x00FF,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = MDL_VER,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x02D0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = FW_VER,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0256,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0257,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x90,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0258,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x020B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x1F,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x020C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0210,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0211,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0212,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x03,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0213,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0214,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x04,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x00A0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0081,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x20,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0083,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0084,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x80,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0090,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0088,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x80,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0082,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0085,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0091,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x01,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0099,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x20,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x00A1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x40,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0250,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0264,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0302,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = FSTMODE_AF,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0396,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = RWEXD1_L_AF,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0398,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = RWEXD2_L_AF,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x039A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = RWEXD3_L_AF,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0303,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = FSTCTIME_AF,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0304,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0400,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0250,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x80,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0264,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x05,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0081,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x20,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x00A0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x80,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0256,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0257,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x90,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0258,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x020B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x1F,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x020C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0210,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0211,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0212,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x03,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0213,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0214,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x04,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0220,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0221,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0222,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0F,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0223,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0F,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0231,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0230,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0232,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0233,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0234,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0235,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0236,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0248,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0249,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x028F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x01,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0280,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x01,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0281,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x80,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0281,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x04,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0283,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x6A,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x028A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x10,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0282,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x10,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0282,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_POLL,
        .delay = 0
      },
      {
        .reg_addr = 0x0283,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x1B,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x028A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x18,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0282,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x10,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0282,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_POLL,
        .delay = 0
      },
      {
        .reg_addr = 0x0281,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x028B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x7C,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0283,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = GYROX_INI,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0284,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = GYROY_INI,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0280,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0030,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0270,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x5C,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0271,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x5D,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0272,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0273,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01C0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01C1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01C2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01C3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01C5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x04,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01C4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x40,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01C7,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x04,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01C6,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xC0,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01C9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01C8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01CB,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01CA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0030,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x80,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0101,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0102,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x018F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x018E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0102,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x03,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0102,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_POLL,
        .delay = 0
      },
      {
        .reg_addr = 0x0170,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0171,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x018C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1239,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x123A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x123B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x123C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10E6,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11E6,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },

      {
        .reg_addr = 0x0178,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x017A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xFF,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10EC,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11EC,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x017C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x017D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x06,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x017E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x01,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10ED,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xBC800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11ED,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xBC800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10EE,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3AE90466,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11EE,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3AE90466,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0174,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x11,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10BA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = A3_IEXP3,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10BB,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10BC,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = A1_IEXP1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10BD,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10BE,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11BA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = A3_IEXP3,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11BB,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11BC,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = A1_IEXP1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11BD,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11BE,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10FA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = A3_IEXP3,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10FB,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10FC,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = A1_IEXP1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10FD,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10FE,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11FA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = A3_IEXP3,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11FB,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11FC,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = A1_IEXP1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11FD,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11FE,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0180,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0181,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0182,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x60,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0183,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0184,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0188,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x06,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0190,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0191,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0192,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x08,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0193,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0194,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0195,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0196,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0197,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0198,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0199,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01A0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01A2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x08,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01A3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01A4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01A5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01A6,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01A7,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01A8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01A9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01B0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01B1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01B2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01B3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01B4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01B5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01B6,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01B7,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01B8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01B9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01BA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01BB,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01BC,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01BD,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01BE,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01BF,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0105,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01CE,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xFF,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x018C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0001,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x30,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0002,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x90,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0003,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xFF,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0004,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xFF,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0011,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x2D,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0010,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0012,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x04,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0013,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x04,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0005,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0006,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0014,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0015,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x001A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x001B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0020,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xC0,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0021,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x22,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0022,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x80,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0023,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0024,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0250,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x80,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0110,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x03,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0107,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x10,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0117,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x011C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x011B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0118,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0119,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x12,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0116,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x06,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1028,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3DCCCCCD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1128,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3DCCCCCD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1029,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3ECCCCCD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1129,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3ECCCCCD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x102A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3ECCCCCD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x112A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3ECCCCCD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x112B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x40000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x102B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x40000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x102C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x40000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x112C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x40000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0130,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x12,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0131,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x09,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1226,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x109D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x109E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x104F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x105F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3FE00000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x106F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3CA3D70A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x107F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3E4CCCCD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x108F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3DCCCCCD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x109F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10AF,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3C23D70A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10BF,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3CA3D70A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x119D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x119E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x114F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x115F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3FE00000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x116F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3CA3D70A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x117F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3E4CCCCD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x118F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3DCCCCCD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x119F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11AF,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3C23D70A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11BF,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3CA3D70A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0133,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0140,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0141,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0142,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0143,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0144,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x01,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0145,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0146,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0147,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x07,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0148,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0149,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x014A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x11,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x014B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x014C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x014D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x014E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x01,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x014F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0150,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0151,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0152,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x44,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0153,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x04,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x015B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x015C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x015D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x015E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x03,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0132,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x11,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0154,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0155,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x54,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0156,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x14,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0157,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x94,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0158,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x015F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x013C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x01,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x013D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x013E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x013F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x07,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x015A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10AE,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3A031280,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11AE,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3A031280,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1094,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1095,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1096,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xBD4CCCCD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1097,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x38D1B717,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1194,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1195,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1196,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xBD4CCCCD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1197,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x38D1B717,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0120,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0A,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0123,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x5D,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0121,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x50,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0122,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x05,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0128,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0B,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0108,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0129,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xA3,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x012A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xA3,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0111,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvWG_DWNSMP1,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0113,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvWG_DWNSMP3,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0114,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvWG_DWNSMP4,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0172,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvWH_DWNSMP1,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01E3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvWAF_DWNSMP1,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01E4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvWAF_DWNSMP2,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1000,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGX45G,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1001,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGX45X,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1002,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGX45Y,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1003,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXGYRO,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1004,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXIA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1005,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXIB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1006,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXIC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1007,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXGGAIN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1008,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXIGAIN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1009,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXIGAIN2,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x100A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGX2X4XF,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x100B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXADJ,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x100C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXGAIN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x100E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXHCTMP,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1010,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXH1A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1011,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXH1B,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1012,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXH1C,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1013,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXH2A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1014,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXH2B,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1015,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXH2C,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1016,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXH3A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1017,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXH3B,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1018,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXH3C,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1019,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x101A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x101B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x101C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXHGAIN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x101D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLGAIN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x101E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXIGAINSTP,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1020,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXZOOM,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1021,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGX2X4XB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1022,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLENS,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1023,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXTA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1024,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXTB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1025,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXTC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1026,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXTD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1027,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXTE,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1030,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXJ1A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1031,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXJ1B,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1032,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXJ1C,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1033,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXJ2A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1034,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXJ2B,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1035,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXJ2C,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1036,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXK1A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1037,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXK1B,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1038,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXK1C,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1039,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXK2A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x103A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXK2B,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x103B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXK2C,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x103C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXOA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x103D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXOB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x103E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXOC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1043,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXIA1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1044,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXIB1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1045,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXIC1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1046,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXIAA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1047,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXIBA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1048,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXICA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1049,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXIAB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x104A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXIBB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x104B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXICB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x104C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXIAC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x104D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXIBC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x104E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXICC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1053,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLA1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1054,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLB1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1055,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLC1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1056,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLAA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1057,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLBA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1058,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLCA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1059,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLAB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x105A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLBB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x105B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLCB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x105C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLAC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x105D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLBC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x105E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLCC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1063,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXGYRO1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1066,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXGYROA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1069,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXGYROB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x106C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXGYROC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1073,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXGAIN1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1076,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXGAINA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1079,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXGAINB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x107C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXGAINC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1083,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXISTP1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1086,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXISTPA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1089,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXISTPB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x108C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXISTPC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1093,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXISTP,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1098,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXOG1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1099,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXOG2,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x109A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXOG3,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10A1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvPXMAA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10A2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvPXMAB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10A3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvPXMAC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10A4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvPXMBA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10A5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvPXMBB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10A6,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvPXMBC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10A7,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXMA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10A8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXMB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10A9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXMC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10AA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXMG,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10AB,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLEVA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10AC,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLEVB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10AD,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGXLEVC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10B0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXRIA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10B1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXRIB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10B2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXRIC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10B3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXINX,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10B4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXINY,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10B5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXGGF,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10B6,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXAG,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10B8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXGX,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10B9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXGY,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10C0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXDA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10C1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXDB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10C2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXDC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10C3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXEA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10C4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXEB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10C5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXEC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10C6,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXUA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10C7,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXUB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10C8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXUC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10C9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXIA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10CA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXIB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10CB,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXIC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10CC,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXJA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10CD,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXJB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10CE,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXJC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10D0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXFA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10D1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXFB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10D2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXFC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10D3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXG,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10D4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXG2,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10D5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXSIN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10D7,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXSA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10D8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXSB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10D9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXSC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10DA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXOA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10DB,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXOB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10DC,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXOC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10DD,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXOD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10DE,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXOE,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10E0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXPA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10E1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXPB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10E2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXPC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10E3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXPD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10E4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXPE,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10E5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSXQ,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10E8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSMXGA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10E9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSMXGB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10EA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSMXA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10EB,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSMXB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10F0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES1AA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10F1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES1AB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10F2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES1AC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10F3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES1AD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10F4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES1AE,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10F5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES1BA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10F6,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES1BB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10F7,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES1BC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10F8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES1BD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10F9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES1BE,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1100,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGY45G,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1101,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGY45Y,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1102,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGY45X,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1103,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYGYRO,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1104,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYIA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1105,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYIB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1106,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYIC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1107,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYGGAIN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1108,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYIGAIN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1109,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYIGAIN2,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x110A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGY2X4XF,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x110B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYADJ,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x110C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYGAIN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x110E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYHCTMP,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1110,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYH1A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1111,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYH1B,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1112,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYH1C,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1113,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYH2A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1114,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYH2B,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1115,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYH2C,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1116,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYH3A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1117,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYH3B,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1118,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYH3C,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1119,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x111A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x111B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x111C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYHGAIN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x111D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLGAIN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x111E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYIGAINSTP,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1120,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYZOOM,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1121,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGY2X4XB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1122,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLENS,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1123,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYTA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1124,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYTB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1125,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYTC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1126,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYTD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1127,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYTE,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1130,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYJ1A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1131,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYJ1B,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1132,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYJ1C,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1133,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYJ2A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1134,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYJ2B,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1135,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYJ2C,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1136,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYK1A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1137,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYK1B,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1138,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYK1C,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1139,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYK2A,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x113A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYK2B,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x113B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYK2C,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x113C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYOA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x113D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYOB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x113E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYOC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1143,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYIA1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1144,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYIB1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1145,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYIC1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1146,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYIAA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1147,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYIBA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1148,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYICA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1149,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYIAB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x114A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYIBB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x114B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYICB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x114C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYIAC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x114D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYIBC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x114E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYICC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1153,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLA1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1154,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLB1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1155,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLC1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1156,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLAA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1157,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLBA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1158,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLCA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1159,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLAB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x115A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLBB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x115B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLCB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x115C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLAC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x115D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLBC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x115E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLCC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1163,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYGYRO1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1166,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYGYROA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1169,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYGYROB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x116C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYGYROC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1173,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYGAIN1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1176,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYGAINA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1179,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYGAINB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x117C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYGAINC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1183,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYISTP1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1186,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYISTPA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1189,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYISTPB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x118C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYISTPC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1193,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYISTP,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1198,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYOG1,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1199,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYOG2,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x119A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYOG3,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11A1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvPYMAA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11A2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvPYMAB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11A3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvPYMAC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11A4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvPYMBA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11A5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvPYMBB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11A6,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvPYMBC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11A7,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYMA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11A8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYMB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11A9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYMC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11AA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYMG,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11AB,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLEVA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11AC,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLEVB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11AD,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvGYLEVC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11B0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYRIA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11B1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYRIB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11B2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYRIC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11B3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYINY,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11B4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYINX,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11B5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYGGF,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11B6,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYAG,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11B8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYGY,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11B9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYGX,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11C0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYDA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11C1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYDB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11C2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYDC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11C3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYEA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11C4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYEB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11C5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYEC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11C6,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYUA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11C7,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYUB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11C8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYUC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11C9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYIA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11CA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYIB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11CB,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYIC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11CC,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYJA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11CD,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYJB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11CE,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYJC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11D0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYFA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11D1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYFB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11D2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYFC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11D3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYG,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11D4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYG2,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11D5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYSIN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11D7,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYSA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11D8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYSB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11D9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYSC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11DA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYOA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11DB,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYOB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11DC,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYOC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11DD,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYOD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11DE,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYOE,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11E0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYPA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11E1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYPB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11E2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYPC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11E3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYPD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11E4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYPE,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11E5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSYQ,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11E8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSMYGA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11E9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSMYGB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11EA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSMYA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11EB,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvSMYB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11F0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES2AA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11F1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES2AB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11F2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES2AC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11F3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES2AD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11F4,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES2AE,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11F5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES2BA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11F6,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES2BB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11F7,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES2BC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11F8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES2BD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11F9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvMES2BE,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1200,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFSIN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1201,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFING,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1202,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFSTMG,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1203,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFAG,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1204,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFDA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1205,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFDB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1206,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFDC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1207,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFEA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1208,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFEB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1209,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFEC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x120A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFUA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x120B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFUB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x120C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFUC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x120D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFIA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x120E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFIB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x120F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFIC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1210,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFJA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1211,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFJB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1212,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFJC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1213,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFFA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1214,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFFB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1215,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFFC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1216,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFG,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1217,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFG2,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1218,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFPA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1219,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFPB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x121A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFPC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x121B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFPD,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x121C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFPE,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x121D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFSTMA,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x121E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFSTMB,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x121F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvAFSTMC,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1235,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvST1MEAN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1236,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvST2MEAN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1237,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvST3MEAN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1238,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = fvST4MEAN,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x018D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1083,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x39A5CB40,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1183,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x39A5CB40,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0134,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0135,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0136,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x90,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0137,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x01,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0138,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x64,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0139,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x013A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x013B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1235,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1236,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3B23D700,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1237,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3C23D700,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1238,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0251,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = BIAS_CUR_OIS,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0253,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = AMP_GAIN_X,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0254,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = AMP_GAIN_Y,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0252,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0255,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0257,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x2E,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x02A0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x02A1,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x02A2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x02A3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x018C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x31,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1479,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0000,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x147A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xE000,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x14F9,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0000,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x14FA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xE000,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1450,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0000,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x14D0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0000,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10D3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3000,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11D3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3000,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1529,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0000,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x152A,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x8001,
        .data_type = CAMERA_I2C_WORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x018C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1020,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = GXGAIN_INI,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1120,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = GYGAIN_INI,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10E5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = SXQ_INI,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11E5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = SYQ_INI,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1022,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1122,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0010,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xC0,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0250,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xDF,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01E0,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0193,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0192,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01A3,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01A2,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0109,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1012,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F7FFE00,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1112,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F7FFE00,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0154,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0155,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x54,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0156,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x14,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0157,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x94,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0158,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x102D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = MINLMT,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x112D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = MINLMT,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10AA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = CHGCOEF,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11AA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = CHGCOEF,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x100E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F7FFE00,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x110E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F7FFE00,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x011B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x12,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0154,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0155,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x54,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0156,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x14,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0157,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x94,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0158,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x102D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = MINLMT,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x102E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = MAXLMT,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x112D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = MINLMT,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x112E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = MAXLMT,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x100E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F7FFE00,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10AA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = CHGCOEF,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x110E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F7FFE00,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11AA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = CHGCOEF,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x011B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x12,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0001,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0xF0,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0101,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x01,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
    }, //end init_setings
    .enable_ois_setting_size = 11,
    .enable_ois_settings =
    {
      {
        .reg_addr = 0x0170,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x03,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10B5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0171,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x03,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11B5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0107,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x018F,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x018E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x0E,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0102,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0102,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_POLL,
        .delay = 0
      },
      {
        .reg_addr = 0x10B5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11B5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F800000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
    }, //end ois settings
    .disable_ois_setting_size = 2,
    .disable_ois_settings =
    {
      {
        .reg_addr = 0x10B5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11B5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
    }, //end disable ois settings
    .movie_mode_ois_setting_size = 12,
    .movie_mode_ois_settings =
    {
      {
        .reg_addr = 0x0154,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0155,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0156,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x14,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0157,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x94,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0158,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x102D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = MINLMT_MOV,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x112D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = MINLMT_MOV,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10AA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = CHGCOEF_MOV,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11AA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = CHGCOEF_MOV,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x100E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F7FFE00,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x110E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F7FFE00,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x011B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x12,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
    }, // end movie_mode_ois_settings
    .still_mode_ois_setting_size = 12,
    .still_mode_ois_settings =
    {
      {
        .reg_addr = 0x0154,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0155,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x54,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0156,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x14,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0157,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x94,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0158,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x102D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = MINLMT,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x112D,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = MINLMT,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10AA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = CHGCOEF,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11AA,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = CHGCOEF,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x100E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F7FFE00,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x110E,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x3F7FFE00,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x011B,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x12,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
    }, //end still_mode_ois_settings
    .centering_on_ois_setting_size = 10,
    .centering_on_ois_settings =
    {
      {
        .reg_addr = 0x0107,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10B5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11B5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x017C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x01,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0170,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x03,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x10B5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0171,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x03,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x11B5,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x01F8,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x66,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_POLL,
        .delay = 0
      },
      {
        .reg_addr = 0x017C,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
    }, //end centering_on_ois_settings
    .centering_off_ois_setting_size = 4,
    .centering_off_ois_settings =
    {
      {
        .reg_addr = 0x0170,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x1477,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x0171,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x02,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
      {
        .reg_addr = 0x14F7,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x00000000,
        .data_type = CAMERA_I2C_DWORD_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
    }, //end centering_off_ois_settings
    .pantilt_on_ois_setting_size = 1,
    .pantilt_on_ois_settings =
    {
      {
        .reg_addr = 0x0109,
        .addr_type = CAMERA_I2C_WORD_ADDR,
        .reg_data = 0x01,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .i2c_operation = OIS_WRITE,
        .delay = 0
      },
    }, //end pantilt_on_ois_setting_size
  }, //end ois_params
}, //end ois_driver_params_t
