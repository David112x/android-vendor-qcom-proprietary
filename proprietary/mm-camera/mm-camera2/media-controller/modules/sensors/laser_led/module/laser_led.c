/* laser_led.c
 *
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "laser_led.h"
#include "sensor_common.h"
#include "sensor_util.h"

#define ON_REG_ARRAY \
{ \
  {0x001C, 0x02, 0x0}, \
  {0x0010, 0xF1, 0x0}, \
  {0x0011, 0x32, 0x0}, \
  {0x0012, 0xFF, 0x0}, \
  {0x0013, 0x30, 0x0}, \
  {0x0014, 0x0 , 0x0}, \
  {0x0015, 0x18, 0x0}, \
  {0x0016, 0xFF, 0x0}, \
  {0x0017, 0xFF, 0x0}, \
  {0x0018, 0x30, 0x0}, \
  {0x0019, 0x00, 0x0}, \
  {0x001A, 0x00, 0x0}, \
  {0x001B, 0x01, 0x0}, \
  {0x001D, 0x0E, 0x0}, \
  {0x001E, 0x00, 0x0}, \
  {0x001F, 0x00, 0x0}, \
  {0x001C, 0x01, 0x0}, \
}

#define OFF_REG_ARRAY \
{ \
  {0x001C, 0x02, 0x0}, \
  {0x0010, 0xF9, 0x0}, \
  {0x0011, 0x32, 0x0}, \
  {0x0012, 0xFF, 0x0}, \
  {0x0013, 0x30, 0x0}, \
  {0x0014, 0x0 , 0x0}, \
  {0x0015, 0x18, 0x0}, \
  {0x0016, 0xFF, 0x0}, \
  {0x0017, 0xFF, 0x0}, \
  {0x0018, 0x30, 0x0}, \
  {0x0019, 0x00, 0x0}, \
  {0x001A, 0x00, 0x0}, \
  {0x001B, 0x01, 0x0}, \
  {0x001D, 0x0E, 0x0}, \
  {0x001E, 0x00, 0x0}, \
  {0x001F, 0x00, 0x0}, \
  {0x001C, 0x01, 0x0}, \
}

#define DEBUG_REG_ARRAY {0x0000, 0x0001, 0x0002}

#define SET_CURRENT_REG_ARRAY \
{ \
  {0x0013, 0x30, 0x0}, \
  {0x0014, 0x00, 0x0}, \
  {0x001C, 0x01, 0x0}, \
}
#define CURRENT_VALUE_INDEX 1
#define CURRENT_PREFIX_INDEX 0
#define CURRENT_STEP_SIZE 0xFF

/** laser_led_init:
 *    @ptr: address of pointer to
 *                   sensor_laser_led_data_t struct
 *
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t laser_led_init(void *ptr)
{
  int                          i = 0;
  int32_t                      rc = 0;
  sensor_laser_led_data_t      *laser_led_ptr = (sensor_laser_led_data_t *)ptr;
  struct msm_laser_led_cfg_data_t cfg;

  SLOW("laser_led_init : E");

  /* Validate parameters */
  RETURN_ERR_ON_NULL(laser_led_ptr, -EINVAL,
    "Invalid Argument - af_actuator_ptr");

  cfg.cfg_type = CFG_LASER_LED_INIT;
  cfg.i2c_addr = 0xC0;
  cfg.i2c_freq_mode = I2C_FAST_MODE;

  /* Invoke the IOCTL to initialize the laser LED */
  rc = ioctl(laser_led_ptr->fd, VIDIOC_MSM_LASER_LED_CFG, &cfg);
  if (rc < 0) {
    SERR("CFG_ACTUATOR_INIT failed: rc = %d",rc);
    return rc;
  }

  return rc;
}


/** laser_led_open:
 *    @laser_led_ctrl: address of pointer to
 *                   sensor_laser_led_data_t struct
 *    @subdev_name: LED flash subdev name
 *
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t laser_led_open(void **laser_led_ctrl, void *data)
{
  int32_t rc = 0;
  sensor_laser_led_data_t *ctrl = NULL;
  char subdev_string[32];
  sensor_submodule_info_t *info =
    (sensor_submodule_info_t *)data;

  SLOW("laser_led_open : E");

  RETURN_ERROR_ON_NULL(laser_led_ctrl);
  RETURN_ERROR_ON_NULL(info);
  RETURN_ERROR_ON_NULL(info->intf_info[SUBDEV_INTF_PRIMARY].sensor_sd_name);

  ctrl = malloc(sizeof(sensor_laser_led_data_t));
  if (!ctrl) {
    SERR("malloc failed");
    return -EINVAL;
  }

  memset(ctrl, 0, sizeof(sensor_laser_led_data_t));

  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s",
    info->intf_info[SUBDEV_INTF_PRIMARY].sensor_sd_name);
  /* Open subdev */
  ctrl->fd = open(subdev_string, O_RDWR);
  if (ctrl->fd < 0) {
    SERR("failed");
    rc = -EINVAL;
    goto ERROR;
  }

  *laser_led_ctrl= (void *)ctrl;

  return rc;

ERROR:
  free(ctrl);
  return rc;
}

/** LOG_IOCTL_LED:
*
* DESCRIPTION: Wrapper for logging and to trace ioctl calls.
**/
int LOG_IOCTL_LED(int d, int request, void* par1, char* trace_func)
{
  int ret;
  ATRACE_BEGIN_SNPRINTF(35,"Camera:sensorIoctl %s", trace_func);
  ret = ioctl(d, request, par1);
  ATRACE_END();
  return ret;
}

/** laser_write_i2c_setting_array:
 *    @laser_led_ptr: address of pointer to
 *                   sensor_laser_led_data_t struct
 *    @settings:I2C settings array
 *
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t laser_write_i2c_setting_array(
  sensor_laser_led_data_t      *laser_led_ptr,
  struct sensor_i2c_reg_setting_array *settings)
{
  struct msm_laser_led_cfg_data_t cfg;
  struct msm_camera_i2c_reg_setting setting_k;
  uint32_t debug_reg[] = DEBUG_REG_ARRAY;
  uint32_t debug_reg_size = sizeof(debug_reg)/sizeof(uint32_t);

  setting_k.addr_type =
    sensor_sdk_util_get_kernel_i2c_addr_type(settings->addr_type);
  setting_k.data_type =
    sensor_sdk_util_get_kernel_i2c_data_type(settings->data_type);
  setting_k.delay = settings->delay;
  setting_k.size = settings->size;

  setting_k.reg_setting = (struct msm_camera_i2c_reg_array *)
    &(settings->reg_setting_a[0]);

  cfg.cfg_type = CFG_LASER_LED_CONTROL;
  cfg.setting = &setting_k;
  cfg.debug_reg = debug_reg;
  cfg.debug_reg_size= debug_reg_size;
  /* Invoke IOCTL to write the I2C settings */
  if (LOG_IOCTL_LED(laser_led_ptr->fd, VIDIOC_MSM_LASER_LED_CFG, &cfg, "write_i2c") < 0) {
    SERR("failed");
    return -EIO;
  }

  return SENSOR_SUCCESS;
}

/** laser_led_on:
 *    @ptr: address of pointer to
 *                   sensor_laser_led_data_t struct
 *
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t laser_led_on(void * ptr)
{
  int                          rc = 0;
  sensor_laser_led_data_t      *laser_led_ptr = (sensor_laser_led_data_t *)ptr;
  int32_t                      size = 0;
  /* Filling I2C structure with on settings*/
  struct sensor_i2c_reg_setting_array setting = {
        .reg_setting_a = ON_REG_ARRAY,
        .addr_type = CAMERA_I2C_BYTE_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 0,
  };

  size = sensor_util_get_setting_size(
    setting.reg_setting_a, MAX_SENSOR_SETTING_I2C_REG);

  if (size < 0) return SENSOR_FAILURE;
  setting.size = (unsigned short) size;

  rc = laser_write_i2c_setting_array (laser_led_ptr, &setting);

  return rc;
}

/** laser_led_off:
 *    @ptr: address of pointer to
 *                   sensor_laser_led_data_t struct
 *
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t laser_led_off(void * ptr)
{
  int                          rc = 0;
  sensor_laser_led_data_t      *laser_led_ptr = (sensor_laser_led_data_t *)ptr;
  int32_t                      size = 0;
  /* Filling I2C structure with off settings*/
  struct sensor_i2c_reg_setting_array setting = {
        .reg_setting_a = OFF_REG_ARRAY,
        .addr_type = CAMERA_I2C_BYTE_ADDR,
        .data_type = CAMERA_I2C_BYTE_DATA,
        .delay = 0,
  };

  size = sensor_util_get_setting_size(
    setting.reg_setting_a, MAX_SENSOR_SETTING_I2C_REG);

  if (size < 0) return SENSOR_FAILURE;
  setting.size = (unsigned short) size;

  rc = laser_write_i2c_setting_array (laser_led_ptr, &setting);

  return rc;
}

/** laser_led_set_current:
 *    @ptr: address of pointer to
 *                   sensor_laser_led_data_t struct
 *    @data: current value
 *
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t laser_led_set_current(void * ptr, void *data)
{
  int                          rc = 0;
  sensor_laser_led_data_t      *laser_led_ptr = (sensor_laser_led_data_t *)ptr;
  uint32_t                     current = *(uint32_t *)data;
  int32_t                      size = 0;

  /* Filling I2C structure with set_current settings*/
  struct sensor_i2c_reg_setting_array setting = {
                  .reg_setting_a = SET_CURRENT_REG_ARRAY,
                  .addr_type = CAMERA_I2C_BYTE_ADDR,
                  .data_type = CAMERA_I2C_BYTE_DATA,
                  .delay = 0,
  };

  size = sensor_util_get_setting_size(
    setting.reg_setting_a, MAX_SENSOR_SETTING_I2C_REG);

  if (size < 0) return SENSOR_FAILURE;
  setting.size = (unsigned short) size;

  setting.reg_setting_a[CURRENT_VALUE_INDEX].reg_data = current;
  /* Filling value register data with current required */
  if (current > CURRENT_STEP_SIZE) {
    setting.reg_setting_a[CURRENT_PREFIX_INDEX].reg_data++;
  }

  rc = laser_write_i2c_setting_array (laser_led_ptr, &setting);
  return rc;
}

/** laser_led_process:
 *    @laser_led_ctrl: LED flash control handle
 *    @event: configuration event type
 *    @data: NULL
 *
 * Handles all LED flash trigger events and passes control to
 * kernel to configure LED hardware
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t laser_led_process(void *laser_led_ctrl,
  sensor_submodule_event_type_t event, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_laser_led_data_t *ctrl = (sensor_laser_led_data_t *)laser_led_ctrl;

  switch (event) {
  case LASER_LED_INIT:
    rc = laser_led_init ((void *)ctrl);
    break;
  case LASER_LED_SET_OFF:
    rc = laser_led_off ((void *)ctrl);
    break;
  case LASER_LED_SET_ON:
    rc = laser_led_on ((void *)ctrl);
    break;
  case LASER_LED_SET_CURRENT:
    rc = laser_led_set_current ((void *)ctrl, data);
    break;
  default:
    SERR("invalid event %d", event);
    return SENSOR_FAILURE;
  }

  return rc;
}

/** laser_led_close:
 *    @laser_led_ctrl: LED flash control handle
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

static int32_t laser_led_close(void *laser_led_ctrl)
{
  SLOW("laser_led_close : E");
  sensor_laser_led_data_t *ctrl = (sensor_laser_led_data_t *)laser_led_ctrl;
  RETURN_ERROR_ON_NULL(ctrl);

  /* close subdev */
  close(ctrl->fd);

  free(ctrl);
  return 0;
}

/** laser_led_sub_module_init:
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

int32_t laser_led_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  SLOW("laser_led_sub_module_init : E");
  if (!func_tbl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  func_tbl->open = laser_led_open;
  func_tbl->process = laser_led_process;
  func_tbl->close = laser_led_close;

  return SENSOR_SUCCESS;
}
