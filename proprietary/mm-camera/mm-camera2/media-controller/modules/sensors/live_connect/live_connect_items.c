/* live_connect_items.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "sensor_common.h"
#include "live_connect.h"

typedef enum {
  LIVE_CONNECT_START,
  LIVE_CONNECT_CMD_INITIAL_CODE = LIVE_CONNECT_START,
  LIVE_CONNECT_CMD_REGION_SIZE,
  LIVE_CONNECT_SCENARIO_SIZE_MOVE_NEAR,
  LIVE_CONNECT_SCENARIO_SIZE_MOVE_FAR,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_NEAR_0,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_NEAR_1,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_NEAR_2,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_NEAR_3,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_NEAR_4,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_NEAR_5,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_NEAR_6,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_NEAR_7,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_FAR_0,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_FAR_1,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_FAR_2,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_FAR_3,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_FAR_4,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_FAR_5,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_FAR_6,
  LIVE_CONNECT_RINGING_SCENARIO_MOVE_FAR_7,

  LIVE_CONNECT_REGION_PARAM_CODE_PER_STEP_0,
  LIVE_CONNECT_REGION_PARAM_CODE_PER_STEP_1,
  LIVE_CONNECT_REGION_PARAM_CODE_PER_STEP_2,
  LIVE_CONNECT_REGION_PARAM_CODE_PER_STEP_3,
  LIVE_CONNECT_REGION_PARAM_CODE_PER_STEP_4,

  LIVE_CONNECT_REGION_PARAM_MACRO_BOUNDARY_0,
  LIVE_CONNECT_REGION_PARAM_MACRO_BOUNDARY_1,
  LIVE_CONNECT_REGION_PARAM_MACRO_BOUNDARY_2,
  LIVE_CONNECT_REGION_PARAM_MACRO_BOUNDARY_3,
  LIVE_CONNECT_REGION_PARAM_MACRO_BOUNDARY_4,

  LIVE_CONNECT_REGION_PARAM_INFINITY_BOUNDARY_0,
  LIVE_CONNECT_REGION_PARAM_INFINITY_BOUNDARY_1,
  LIVE_CONNECT_REGION_PARAM_INFINITY_BOUNDARY_2,
  LIVE_CONNECT_REGION_PARAM_INFINITY_BOUNDARY_3,
  LIVE_CONNECT_REGION_PARAM_INFINITY_BOUNDARY_4,

  LIVE_CONNECT_DAMPING_MOVE_NEAR_SCENARIO_0_REGION_0_DAMPING_STEP,

  LIVE_CONNECT_DAMPING_MOVE_NEAR_SCENARIO_0_REGION_0_DAMPING_DELAY =
    LIVE_CONNECT_DAMPING_MOVE_NEAR_SCENARIO_0_REGION_0_DAMPING_STEP +
    MAX_ACTUATOR_SCENARIO * MAX_ACTUATOR_REGION,

  LIVE_CONNECT_DAMPING_MOVE_NEAR_SCENARIO_0_REGION_0_HW_PARAMS =
    LIVE_CONNECT_DAMPING_MOVE_NEAR_SCENARIO_0_REGION_0_DAMPING_DELAY +
    MAX_ACTUATOR_SCENARIO * MAX_ACTUATOR_REGION,

  LIVE_CONNECT_DAMPING_MOVE_FAR_SCENARIO_0_REGION_0_DAMPING_STEP =
    LIVE_CONNECT_DAMPING_MOVE_NEAR_SCENARIO_0_REGION_0_HW_PARAMS +
    MAX_ACTUATOR_SCENARIO * MAX_ACTUATOR_REGION,

  LIVE_CONNECT_DAMPING_MOVE_FAR_SCENARIO_0_REGION_0_DAMPING_DELAY =
    LIVE_CONNECT_DAMPING_MOVE_FAR_SCENARIO_0_REGION_0_DAMPING_STEP +
    MAX_ACTUATOR_SCENARIO * MAX_ACTUATOR_REGION,

  LIVE_CONNECT_DAMPING_MOVE_FAR_SCENARIO_0_REGION_0_HW_PARAMS =
    LIVE_CONNECT_DAMPING_MOVE_FAR_SCENARIO_0_REGION_0_DAMPING_DELAY +
    MAX_ACTUATOR_SCENARIO * MAX_ACTUATOR_REGION,

  LIVE_CONNECT_END =
    LIVE_CONNECT_DAMPING_MOVE_FAR_SCENARIO_0_REGION_0_HW_PARAMS +
    MAX_ACTUATOR_SCENARIO * MAX_ACTUATOR_REGION,
  MOVE_FOCUS_NEAR,
  MOVE_FOCUS_FAR,
} live_connect_data_cmd;

/** live_connect_get_cmd_num:
 *
 * Return: number of commands
 **/
uint32_t live_connect_get_cmd_num()
{
  return LIVE_CONNECT_END;
}

/** live_connect_get_start_cmd:
 *
 * Return: First command of list
 **/
uint32_t live_connect_get_start_cmd()
{
  return LIVE_CONNECT_START;
}

/** live_connect_get_end_cmd:
 *
 * Return: Last command of list.
 **/
uint32_t live_connect_get_end_cmd()
{
  return LIVE_CONNECT_END - 1;
}

/** live_connect_fill_map:
 *
 * @item_map: map to live connect items.
 * @actuator_tuned_params_ptr: pointer to actuator tuned parameters.
 *
 * Return: TRUE for success and FALSE on failure.
 **/
boolean live_connect_fill_map(live_connect_items_map *item_map,
  actuator_tuned_params_t *actuator_tuned_params_ptr)
{
  int i = 0, j = 0, index = 0;
  /* Validate input parameters */
  RETURN_ON_NULL(item_map);
  RETURN_ON_NULL(actuator_tuned_params_ptr);

  item_map[LIVE_CONNECT_CMD_INITIAL_CODE].size = 2;
  item_map[LIVE_CONNECT_CMD_INITIAL_CODE].pValue =
    &actuator_tuned_params_ptr->initial_code;
  SLOW("pValue = %p", item_map[LIVE_CONNECT_CMD_INITIAL_CODE].pValue);

  item_map[LIVE_CONNECT_CMD_REGION_SIZE].size = 2;
  item_map[LIVE_CONNECT_CMD_REGION_SIZE].pValue =
    &actuator_tuned_params_ptr->region_size;

  item_map[LIVE_CONNECT_SCENARIO_SIZE_MOVE_NEAR].size = 2;
  item_map[LIVE_CONNECT_SCENARIO_SIZE_MOVE_NEAR].pValue =
    &actuator_tuned_params_ptr->scenario_size[MOVE_NEAR];

  item_map[LIVE_CONNECT_SCENARIO_SIZE_MOVE_FAR].size = 2;
  item_map[LIVE_CONNECT_SCENARIO_SIZE_MOVE_FAR].pValue =
    &actuator_tuned_params_ptr->scenario_size[MOVE_FAR];

  for (i = 0; i < MAX_ACTUATOR_SCENARIO; i++) {
    item_map[LIVE_CONNECT_RINGING_SCENARIO_MOVE_NEAR_0 + i].size = 2;
    item_map[LIVE_CONNECT_RINGING_SCENARIO_MOVE_NEAR_0 + i].pValue =
      &actuator_tuned_params_ptr->ringing_scenario[MOVE_NEAR][i];

    item_map[LIVE_CONNECT_RINGING_SCENARIO_MOVE_FAR_0 + i].size = 2;
    item_map[LIVE_CONNECT_RINGING_SCENARIO_MOVE_FAR_0 + i].pValue =
      &actuator_tuned_params_ptr->ringing_scenario[MOVE_FAR][i];
  }

  for (i = 0; i < MAX_ACTUATOR_REGION; i++) {
    item_map[LIVE_CONNECT_REGION_PARAM_CODE_PER_STEP_0 + i].size = 2;
    item_map[LIVE_CONNECT_REGION_PARAM_CODE_PER_STEP_0 + i].pValue =
      &actuator_tuned_params_ptr->region_params[i].code_per_step;

    item_map[LIVE_CONNECT_REGION_PARAM_MACRO_BOUNDARY_0 + i].size = 2;
    item_map[LIVE_CONNECT_REGION_PARAM_MACRO_BOUNDARY_0 + i].pValue =
      &actuator_tuned_params_ptr->region_params[i].step_bound[0];

    item_map[LIVE_CONNECT_REGION_PARAM_INFINITY_BOUNDARY_0 + i].size = 2;
    item_map[LIVE_CONNECT_REGION_PARAM_INFINITY_BOUNDARY_0 + i].pValue =
      &actuator_tuned_params_ptr->region_params[i].step_bound[1];
  }

  for (i = 0; i < MAX_ACTUATOR_SCENARIO; i++) {
    for (j = 0; j < MAX_ACTUATOR_REGION; j++) {

      index = LIVE_CONNECT_DAMPING_MOVE_NEAR_SCENARIO_0_REGION_0_DAMPING_STEP +
        (i * MAX_ACTUATOR_REGION) + j;
      item_map[index].size = 4;
      item_map[index].pValue = &actuator_tuned_params_ptr->
        damping[MOVE_NEAR][i].ringing_params[j].damping_step;

      index = LIVE_CONNECT_DAMPING_MOVE_NEAR_SCENARIO_0_REGION_0_DAMPING_DELAY +
        (i * MAX_ACTUATOR_REGION) + j;
      item_map[index].size = 4;
      item_map[index].pValue = &actuator_tuned_params_ptr->
        damping[MOVE_NEAR][i].ringing_params[j].damping_delay;

      index = LIVE_CONNECT_DAMPING_MOVE_NEAR_SCENARIO_0_REGION_0_HW_PARAMS +
        (i * MAX_ACTUATOR_REGION) + j;
      item_map[index].size = 4;
      item_map[index].pValue = &actuator_tuned_params_ptr->
        damping[MOVE_NEAR][i].ringing_params[j].hw_params;

      index = LIVE_CONNECT_DAMPING_MOVE_FAR_SCENARIO_0_REGION_0_DAMPING_STEP +
        (i * MAX_ACTUATOR_REGION) + j;
      item_map[index].size = 4;
      item_map[index].pValue = &actuator_tuned_params_ptr->
        damping[MOVE_FAR][i].ringing_params[j].damping_step;

      index = LIVE_CONNECT_DAMPING_MOVE_FAR_SCENARIO_0_REGION_0_DAMPING_DELAY +
        (i * MAX_ACTUATOR_REGION) + j;
      item_map[index].size = 4;
      item_map[index].pValue = &actuator_tuned_params_ptr->
        damping[MOVE_FAR][i].ringing_params[j].damping_delay;

      index = LIVE_CONNECT_DAMPING_MOVE_FAR_SCENARIO_0_REGION_0_HW_PARAMS +
        (i * MAX_ACTUATOR_REGION) + j;
      item_map[index].size = 4;
      item_map[index].pValue = &actuator_tuned_params_ptr->
        damping[MOVE_FAR][i].ringing_params[j].hw_params;
    }
  }
  return TRUE;
}
