/* ir_led.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ir_led.h"
#include "sensor_common.h"

/** ir_led_open:
 *    @ir_led_ctrl: address of pointer to
 *                   sensor_ir_led_data_t struct
 *    @subdev_name: LED flash subdev name
 *
 * 1) Allocates memory for LED flash control structure
 * 2) Opens LED flash subdev node
 * 3) Initialize LED hardware by passing control to kernel
 * driver
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t ir_led_open(void **ir_led_ctrl, void *subdev_name)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ir_led_data_t *ctrl = NULL;
  struct msm_ir_led_cfg_data_t cfg;
  char subdev_string[32];

  if (!ir_led_ctrl || !subdev_name) {
    SERR("failed sctrl %p subdev name %p",
      ir_led_ctrl, subdev_name);
    return SENSOR_ERROR_INVAL;
  }
  ctrl = calloc(1, sizeof(sensor_ir_led_data_t));
  if (!ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s", (char*)subdev_name);
  SLOW("sd name %s", subdev_string);
  /* Open subdev */
  ctrl->fd = open(subdev_string, O_RDWR);
  if (ctrl->fd < 0) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR1;
  }

  memset(&cfg, 0, sizeof(cfg));
  cfg.cfg_type = CFG_IR_LED_INIT;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_IR_LED_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_IR_LED_CFG failed %s", strerror(errno));
    goto ERROR2;
  }

  *ir_led_ctrl = (void *)ctrl;
  return rc;

ERROR2:
  close(ctrl->fd);
ERROR1:
  free(ctrl);
  return rc;
}

/** ir_led_process:
 *    @ir_led_ctrl: LED flash control handle
 *    @event: configuration event type
 *    @data: NULL
 *
 * Handled all LED flash trigger events and passes control to
 * kernel to configure LED hardware
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t ir_led_process(void *ir_led_ctrl,
  sensor_submodule_event_type_t event, void *data)
{
  int32_t                     rc = SENSOR_SUCCESS;
  sensor_ir_led_data_t       *ctrl = (sensor_ir_led_data_t *)ir_led_ctrl;
  sensor_ir_led_cfg_t        *ir_led_cfg = (sensor_ir_led_cfg_t *)data;

  struct msm_ir_led_cfg_data_t cfg;
  if (!ir_led_ctrl || ((IR_LED_SET_ON == event) && !data)) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  memset(&cfg, 0, sizeof(cfg));
  switch (event) {
  case IR_LED_SET_OFF:
    cfg.cfg_type = CFG_IR_LED_OFF;
    break;
  case IR_LED_SET_ON:
    cfg.cfg_type = CFG_IR_LED_ON;
    cfg.pwm_duty_on_ns = (int32_t)IR_LED_DUTY_NS(ir_led_cfg->intensity);
    cfg.pwm_period_ns = IR_LED_PERIOD_NS;
    SHIGH("pwm_duty_on_ns %d pwm_period_ns %d", cfg.pwm_duty_on_ns, cfg.pwm_period_ns);
    break;
  default:
    SERR("invalid event %d", event);
    return SENSOR_FAILURE;
  }
  rc = ioctl(ctrl->fd, VIDIOC_MSM_IR_LED_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_IR_LED_CFG failed %s", strerror(errno));
    rc = SENSOR_FAILURE;
  }
  return rc;
}

/** ir_led_close:
 *    @ir_led_ctrl: LED flash control handle
 *
 * 1) Release LED flash hardware
 * 2) Close fd
 * 3) Free LED flash control structure
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t ir_led_close(void *ir_led_ctrl)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ir_led_data_t *ctrl = (sensor_ir_led_data_t *)ir_led_ctrl;
  struct msm_ir_led_cfg_data_t cfg;

  memset(&cfg, 0, sizeof(cfg));
  cfg.cfg_type = CFG_IR_LED_RELEASE;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_IR_LED_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_IR_LED_CFG failed %s",
      strerror(errno));
  }

  /* close subdev */
  close(ctrl->fd);

  free(ctrl);
  return rc;
}

/** ir_led_sub_module_init:
 *    @func_tbl: pointer to sensor function table
 *
 * Initialize function table for LED flash to be used
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t ir_led_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  if (!func_tbl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  func_tbl->open = ir_led_open;
  func_tbl->process = ir_led_process;
  func_tbl->close = ir_led_close;
  return SENSOR_SUCCESS;
}
