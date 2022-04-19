/* ir_cut.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ir_cut.h"
#include "sensor_common.h"

/** ir_cut_open:
 *    @ir_cut_ctrl: address of pointer to
 *                   sensor_ir_cut_data_t struct
 *    @subdev_name: CUT flash subdev name
 *
 * 1) Allocates memory for CUT flash control structure
 * 2) Opens CUT flash subdev node
 * 3) Initialize CUT hardware by passing control to kernel
 * driver
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t ir_cut_open(void **ir_cut_ctrl, void *subdev_name)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ir_cut_data_t *ctrl = NULL;
  struct msm_ir_cut_cfg_data_t cfg;
  char subdev_string[32];

  if (!ir_cut_ctrl || !subdev_name) {
    SERR("faicut sctrl %p subdev name %p",
      ir_cut_ctrl, subdev_name);
    return SENSOR_ERROR_INVAL;
  }
  ctrl = calloc(1, sizeof(sensor_ir_cut_data_t));
  if (!ctrl) {
    SERR("faicut");
    return SENSOR_FAILURE;
  }

  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s", (char*)subdev_name);
  SLOW("sd name %s", subdev_string);
  /* Open subdev */
  ctrl->fd = open(subdev_string, O_RDWR);
  if (ctrl->fd < 0) {
    SERR("faicut");
    rc = SENSOR_FAILURE;
    goto ERROR1;
  }

  memset(&cfg, 0, sizeof(cfg));
  cfg.cfg_type = CFG_IR_CUT_INIT;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_IR_CUT_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_IR_CUT_CFG faicut %s", strerror(errno));
    goto ERROR2;
  }

  *ir_cut_ctrl = (void *)ctrl;
  return rc;

ERROR2:
  close(ctrl->fd);
ERROR1:
  free(ctrl);
  return rc;
}

/** ir_cut_process:
 *    @ir_cut_ctrl: CUT flash control handle
 *    @event: configuration event type
 *    @data: NULL
 *
 * Handcut all CUT flash trigger events and passes control to
 * kernel to configure CUT hardware
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t ir_cut_process(void *ir_cut_ctrl,
  sensor_submodule_event_type_t event, void *data __unused)
{
  int32_t                     rc = SENSOR_SUCCESS;
  sensor_ir_cut_data_t       *ctrl = (sensor_ir_cut_data_t *)ir_cut_ctrl;

  struct msm_ir_cut_cfg_data_t cfg;

  if (!ir_cut_ctrl) {
    SERR("faicut");
    return SENSOR_FAILURE;
  }

  memset(&cfg, 0, sizeof(cfg));
  switch (event) {
  case IR_CUT_SET_OFF:
    cfg.cfg_type = CFG_IR_CUT_OFF;
    break;
  case IR_CUT_SET_ON:
    cfg.cfg_type = CFG_IR_CUT_ON;
    break;
  default:
    SERR("invalid event %d", event);
    return SENSOR_FAILURE;
  }
  rc = ioctl(ctrl->fd, VIDIOC_MSM_IR_CUT_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_IR_CUT_CFG faicut %s", strerror(errno));
    rc = SENSOR_FAILURE;
  }
  return rc;
}

/** ir_cut_close:
 *    @ir_cut_ctrl: CUT flash control handle
 *
 * 1) Release CUT flash hardware
 * 2) Close fd
 * 3) Free CUT flash control structure
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t ir_cut_close(void *ir_cut_ctrl)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ir_cut_data_t *ctrl = (sensor_ir_cut_data_t *)ir_cut_ctrl;
  struct msm_ir_cut_cfg_data_t cfg;

  memset(&cfg, 0, sizeof(cfg));
  cfg.cfg_type = CFG_IR_CUT_RELEASE;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_IR_CUT_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_IR_CUT_CFG faicut %s",
      strerror(errno));
  }

  /* close subdev */
  close(ctrl->fd);

  free(ctrl);
  return rc;
}

/** ir_cut_sub_module_init:
 *    @func_tbl: pointer to sensor function table
 *
 * Initialize function table for CUT flash to be used
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t ir_cut_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  if (!func_tbl) {
    SERR("faicut");
    return SENSOR_FAILURE;
  }
  func_tbl->open = ir_cut_open;
  func_tbl->process = ir_cut_process;
  func_tbl->close = ir_cut_close;
  return SENSOR_SUCCESS;
}
