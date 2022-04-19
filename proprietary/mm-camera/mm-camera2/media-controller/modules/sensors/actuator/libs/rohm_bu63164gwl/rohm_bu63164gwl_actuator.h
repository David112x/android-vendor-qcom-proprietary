/*============================================================================
  Copyright (c) 2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
============================================================================*/

  {
    .actuator_params =
    {
      .module_name = "rohm",
      .actuator_name = "BU63164GWL",
      .i2c_addr = 0x1C,
      .i2c_data_type = CAMERA_I2C_BYTE_DATA,
      .i2c_addr_type = CAMERA_I2C_BYTE_ADDR,
      .act_type = ACTUATOR_TYPE_HVCM,
      .data_size = 10,
      .reg_tbl =
      {
        .reg_tbl_size = 4,
        .reg_params =
        {
          {
            .reg_write_type = ACTUATOR_WRITE_HW_DAMP,
            .hw_mask = 0x000000B0,
            .reg_addr = 0xF0,
            .hw_shift = 0,
            .data_shift = 0,
          },
          {
            .reg_write_type = ACTUATOR_WRITE_HW_DAMP,
            .hw_mask = 0x00000000,
            .reg_addr = 0xF1,
            .hw_shift = 0,
            .data_shift = 0,
          },
          {
            .reg_write_type = ACTUATOR_WRITE_DAC,
            .hw_mask = 0x00000000,
            .reg_addr = 0xF3,
            .hw_shift = 0,
            .data_shift = 0,
          },
          {
            .reg_write_type = ACTUATOR_WRITE_DAC,
            .hw_mask = 0x00000000,
            .reg_addr = 0xF2,
            .hw_shift = 0,
            .data_shift = 0,
          },
        },
      },
      .init_setting_size = 1,
      .init_settings =
      {
        {
          .reg_addr = 0x8C,// enable DSP
          .addr_type = CAMERA_I2C_BYTE_ADDR,
          .reg_data = 0x01,
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
      .initial_code =160,
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
                .damping_delay = 7000,
                .hw_params = 0x000000B0,
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
                .damping_delay = 7000,
                .hw_params = 0x000000B0,
              },
            },
          },
        },
      },
    }, /* actuator_tuned_params */
  },
