/**
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define ACTUATOR_MODEL "pseudo"
  {
    .actuator_params =
    {
      .module_name = ACTUATOR_MODEL,
      .actuator_name = ACTUATOR_MODEL,
      .i2c_addr = 0,
      .i2c_freq_mode = SENSOR_I2C_MODE_FAST,
      .i2c_data_type = CAMERA_I2C_WORD_DATA,
      .i2c_addr_type = CAMERA_I2C_BYTE_ADDR,
      .act_type = ACTUATOR_TYPE_BIVCM,
      .data_size = 12,
      .reg_tbl =
      {
        .reg_tbl_size = 0,
        .reg_params = {},
      },
      .init_setting_size = 0,
      .init_settings = {},
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
          336,
        },
        /* MOVE_FAR */
        {
          336,
        },
      },
      .initial_code = 1451,	//0xEFF,
      .region_size = 1,
      .region_params =
      {
        {
          .step_bound =
          {
            336, /* Macro step boundary*/
            0, /* Infinity step boundary*/
          },
          .code_per_step = 4,
          .qvalue = 128,
        },
      },
      /* damping used as direction value */
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
                .damping_step = 0xFFF,
                .damping_delay = 1000,
                .hw_params = 0,
              },
            },
          },
        },
        /* damping[MOVE_FAR] */
        {
          /* Scenario 0 */
          {
            .ringing_params =
            {
              /* Region 0 */
              {
                .damping_step = 0xFFF,
                .damping_delay = 1000,
                .hw_params = 0,
              },
            },
          },
        },
      },
    }, /* actuator_tuned_params */
  },
