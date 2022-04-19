/**
 * ak7345_actuator.h
 *
 * DESCRIPTION
 *  This file is generated based on ak7345 application note.
 *
 * Copyright (c) 2014 - 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */
  {
    .actuator_params =
    {
     .module_name = "fiberoptics",
     .actuator_name = "ak7345",
     .i2c_addr = 0x18,
     .i2c_data_type = CAMERA_I2C_WORD_DATA,
     .i2c_addr_type = CAMERA_I2C_BYTE_ADDR,
     .act_type = ACTUATOR_TYPE_VCM,
     .data_size = 9,
      .reg_tbl =
      {
        .reg_tbl_size = 1,
        .reg_params =
        {
          {
            .reg_write_type = ACTUATOR_WRITE_DAC,
            .hw_mask = 0x00000000,
            .reg_addr = 0x00,
            .hw_shift = 0,
            .data_shift = 7,
          },
        },
      },
      .init_setting_size = 1,
      .init_settings =
      {
        {
          .reg_addr = 0x02,
          .addr_type = CAMERA_I2C_BYTE_ADDR,
          .reg_data = 0x00,
          .data_type = CAMERA_I2C_BYTE_DATA,
          .i2c_operation = ACTUATOR_I2C_OP_WRITE,
          .delay = 1000,
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
      .initial_code = 119,
      .region_size = 1,
      .region_params =
      {
        {
          .step_bound =
          {  400, /* Macro step boundary*/
             0, /* Infinity step boundary*/
          },
          .code_per_step = 2,
          .qvalue = 1024,
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
                     .damping_step = 0x1FF,
                     .damping_delay = 7000,
                     .hw_params = 0x00000000,
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
                     .damping_step = 0x1FF,
                     .damping_delay = 7000,
                     .hw_params = 0x00000000,
                  },
               },
            },
         },
      },
    }, /* actuator_tuned_params */
  },
