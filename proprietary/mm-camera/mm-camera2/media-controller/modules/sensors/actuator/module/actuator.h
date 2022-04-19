/*============================================================================

  Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __ACTUATOR_H__
#define __ACTUATOR_H__

#define ACTUATOR_NUM_MODES_MAX 4

#include "actuator_driver.h"
#include "sensor_common.h"

#define ACTUATOR_NEW 1
#define ACTUATOR_PARK_STEP (20)

typedef struct {
  actuator_driver_params_t  *driver_params;
  actuator_driver_ctrl_t    *driver_ctrl;
} actuator_ctrl_t;

typedef struct {
  int32_t               fd;
  actuator_ctrl_t       *ctrl;
  int16_t               curr_step_pos;
  int16_t               cur_restore_pos;
  uint16_t              total_steps;
  uint8_t               is_af_supported;
  uint8_t               cam_name;
  uint8_t               load_params;
  int16_t               curr_lens_pos;
  char                  *name;
  uint8_t               params_loaded;
  void                  *driver_lib_handle;
  void                  *eebin_hdl;
  actuator_plot_t       plot_info;
} actuator_data_t;

static int32_t af_actuator_move_focus(void *ptr, void *data) __attribute__ ((unused));

#endif
