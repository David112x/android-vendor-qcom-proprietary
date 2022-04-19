/**
 *Copyright (c) 2017 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

  {
    .actuator_params =
    {
      .module_name = "dongwoon",
      .actuator_name = "dw9763b",
      .i2c_addr = 0x1C,
      .i2c_freq_mode = SENSOR_I2C_MODE_FAST,
      .i2c_data_type = CAMERA_I2C_WORD_DATA,
      .i2c_addr_type = CAMERA_I2C_BYTE_ADDR,
      .act_type = ACTUATOR_TYPE_VCM,
      .data_size = 10,
      .reg_tbl =
      {
        .reg_tbl_size = 1,
        .reg_params =
        {
          {
            .reg_write_type = ACTUATOR_WRITE_DAC,
            .hw_mask = 0x00000000,
            .reg_addr = 0x03,
            .hw_shift = 0,
            .data_shift = 0,
          },
        },
      },
      .init_setting_size = 5,
      .init_settings =
      {
        {
          .reg_addr = 0x02,
          .addr_type = CAMERA_I2C_BYTE_ADDR,
          .reg_data = 0x02, /* sw reset */
          .data_type = CAMERA_I2C_BYTE_DATA,
          .i2c_operation = ACTUATOR_I2C_OP_WRITE,
          .delay = 0,
        },
        {
          .reg_addr = 0x02,
          .addr_type = CAMERA_I2C_BYTE_ADDR,
          .reg_data = 0x02, /* set ring ctrl */
          .data_type = CAMERA_I2C_BYTE_DATA,
          .i2c_operation = ACTUATOR_I2C_OP_WRITE,
          .delay = 0,
        },
        {
          .reg_addr = 0x06,
          .addr_type = CAMERA_I2C_BYTE_ADDR,
          .reg_data = 0x41, /* set SAC2.5 mode & PRESC default 1 */
          .data_type = CAMERA_I2C_BYTE_DATA,
          .i2c_operation = ACTUATOR_I2C_OP_WRITE,
          .delay = 0,
        },
        {
          .reg_addr = 0x07,
          .addr_type = CAMERA_I2C_BYTE_ADDR,
          .reg_data = 0x3F, /* SAC Reg SAC period 12.6ms Tvib
                             (TSAC)=6.3ms+SACT[5:0]*0.1ms */
          .data_type = CAMERA_I2C_BYTE_DATA,
          .i2c_operation = ACTUATOR_I2C_OP_WRITE,
          .delay = 0,
        },
        {
          .reg_addr = 0x08,
          .addr_type = CAMERA_I2C_BYTE_ADDR,
          .reg_data = 0x00, /* VCM preload current */
          .data_type = CAMERA_I2C_BYTE_DATA,
          .i2c_operation = ACTUATOR_I2C_OP_WRITE,
          .delay = 0,
        },
      },
    }, /* actuator_params */

    .actuator_tuned_params =
    {
      .scenario_size =
      {
        1, /* MOVE_NEAR */
        1, /* MOVE_FAR */
      },
      .ringing_scenario =
      {
        /* MOVE_NEAR */
        {
          400,
        },
        /* MOVE_FAR */
        {
          400,
        },
      },
      .initial_code = 300,
      .region_size = 1,
      .region_params =
      {
        {
          .step_bound =
          {
            400, /* Macro step boundary*/
            0, /* Infinity step boundary*/
          },
          .code_per_step = 1,
          .qvalue = 128,
        },
      },
     .damping =
      {
        /* damping[MOVE_NEAR] */
        {
          /* Scenario 0 */
          {
            .ringing_params =
            {
              /* Region 0 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 8000,
                .hw_params = 0x00000400,
              },
            },
          },
        },
        /* damping[MOVE_NEAR] */
        {
          /* Scenario 0 */
          {
            .ringing_params =
            {
              /* Region 0 */
              {
                .damping_step = 0x3FF,
                .damping_delay = 12000,
                .hw_params = 0x00000400,
              },
            },
          },
        },
      },
    }, /* actuator_tuned_params */
  },
