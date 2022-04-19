/* external.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "external.h"
#include "sensor_common.h"

/** external_open: external open sub module
 *
 *  Return: SENSOR_SUCCESS on success and SENSOR_FAILURE on failure
 **/
static int32_t external_open(void **external_ctrl, void* data)
{
  int32_t retVal = SENSOR_SUCCESS;
  char  subdev_string[32];
  external_ctrl_t *ctrl = NULL;
  sensor_submodule_info_t *info =
    (sensor_submodule_info_t *)data;
  if(!external_ctrl || !info || !info->data) {
    SERR("failed extctrl %p", external_ctrl);
     return SENSOR_ERROR_INVAL;
  }

  if (!strlen(info->intf_info[SUBDEV_INTF_PRIMARY].sensor_sd_name)) {
    SERR("failed: sensor_sd_name is 0 length");
    return SENSOR_ERROR_INVAL;
  }

  ctrl = malloc(sizeof(external_ctrl_t));
  if (!ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  memset(ctrl, 0, sizeof(external_ctrl_t));
  ctrl->pipeline_delay = 1;
  *external_ctrl = (void *)ctrl;

  return retVal;
}

/** external_process: external process function
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static int32_t external_process(void *external_ctrl,
  sensor_submodule_event_type_t event, void *data)
{
  int32_t retVal = SENSOR_SUCCESS;
  external_ctrl_t *ctrl = (external_ctrl_t *)external_ctrl;
  if(!ctrl)
    return SENSOR_FAILURE;

  switch(event) {
  case EXT_AEC_UPDATE:
    break;
  case EXT_GET_INFO: {
    RETURN_ERROR_ON_NULL(data);
    *(int *)data = ctrl->pipeline_delay;
    break;
  }
  default:
    retVal = SENSOR_FAILURE;
    break;
  }
  return retVal;
}

/** external_close: external close sub module
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static int32_t external_close(void *external_ctrl)
{
  int32_t retVal = SENSOR_SUCCESS;
  RETURN_ERROR_ON_NULL(external_ctrl);

  return retVal;
}

/** external_sub_module_init:
 *
 *  Return: TRUE on success and FALSE on failure
 **/
int32_t external_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  RETURN_ERROR_ON_NULL(func_tbl);

  func_tbl->open = external_open;
  func_tbl->process = external_process;
  func_tbl->close = external_close;

  return SENSOR_SUCCESS;
}
