/**
 * flash.c
 *
 * Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <cutils/properties.h>
#include <dlfcn.h>
#include "flash.h"
#include "sensor_common.h"
#include "sensor_util.h"
#ifdef __ANDROID__
  #include <cutils/properties.h>
#endif

#define PATH_SIZE_255 255
#define SOC_ID_QM215 386
#define SOC_ID_QM2150 436

/** flash_open:
 *    @flash_ctrl: address of pointer to
 *                   sensor_flash_data_t struct
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

static int32_t flash_open(void **flash_ctrl, void *data)
{
  int32_t                     rc = SENSOR_SUCCESS;
  sensor_flash_data_t    *ctrl = NULL;
  char                        subdev_string[32];
  sensor_submodule_info_t *info =
      (sensor_submodule_info_t *)data;

  SDBG("Enter");
  RETURN_ERROR_ON_NULL(flash_ctrl);
  RETURN_ERROR_ON_NULL(info);
  RETURN_ERROR_ON_NULL(info->intf_info[SUBDEV_INTF_PRIMARY].sensor_sd_name);

  ctrl = malloc(sizeof(sensor_flash_data_t));

  RETURN_ERROR_ON_NULL(ctrl);

  memset(ctrl, 0, sizeof(sensor_flash_data_t));

  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s",
      info->intf_info[SUBDEV_INTF_PRIMARY].sensor_sd_name);
  SLOW("sd name %s", subdev_string);
  /* Open subdev */
  ctrl->fd = open(subdev_string, O_RDWR);
  if (ctrl->fd < 0) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR1;
  }

  ctrl->rer = malloc(sizeof(rer_cfg_t));
  if (!ctrl->rer) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR1;
  }
  memset(ctrl->rer, 0, sizeof(rer_cfg_t));

  ctrl->rer->cfg = malloc(sizeof(red_eye_reduction_type));
  if (!ctrl->rer->cfg) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR2;
  }
  memset(ctrl->rer->cfg, 0, sizeof(red_eye_reduction_type));
  memset(&ctrl->dual_led_setting, 0, sizeof(dual_led_settings_t));
  memset(ctrl->flash_max_duration, 0xFF, sizeof(ctrl->flash_max_duration));

  *flash_ctrl = (void *)ctrl;
  SDBG("Exit");
  return rc;

ERROR2:
  free(ctrl->rer);
ERROR1:
  free(ctrl);
  return rc;
}

static int32_t flash_load_lib(void *ptr, char *name)
{
  char driver_lib_name[PATH_SIZE_255];
  char driver_open_lib_func_name[64];
  void *(*open_lib_func)(void) = NULL;
  sensor_flash_data_t *flash_ptr = (sensor_flash_data_t *)ptr;
  int fd = 0, soc_id = 0;
  char buf[10] = {0};

  fd = open("/sys/devices/soc0/soc_id", O_RDONLY);
  if (fd >= 0) {
    if (read(fd, buf, sizeof(buf) - 1) == -1) {
      SERR("Unable to read soc_id");
    } else {
      soc_id = atoi(buf);
    }
    close(fd);
  }
  if (flash_ptr->driver_lib_handle == NULL) {
    if ((soc_id == SOC_ID_QM215 || soc_id == SOC_ID_QM2150)) {
      snprintf(driver_lib_name, PATH_SIZE_255, "libflash_%s.so", "gpio");
    } else {
      snprintf(driver_lib_name, PATH_SIZE_255, "libflash_%s.so", name);
    }
    snprintf(driver_open_lib_func_name, 64, "flash_driver_open_lib");
  }

  SHIGH("Loading library %s", driver_lib_name);
  flash_ptr->driver_lib_handle = dlopen(driver_lib_name, RTLD_NOW);
  RETURN_ERR_ON_NULL(flash_ptr->driver_lib_handle, -EINVAL,
    "dlopen() failed to load %s", driver_lib_name);

  *(void **)&open_lib_func = dlsym(flash_ptr->driver_lib_handle,
                               driver_open_lib_func_name);
  RETURN_ERR_ON_NULL(open_lib_func, -EINVAL, "flash_driver_open_lib failed");

  flash_ptr->driver_lib_data = (flash_lib_t *)open_lib_func();
  RETURN_ERR_ON_NULL(flash_ptr->driver_lib_data, -EINVAL);

  return SENSOR_SUCCESS;
}

/** flash_update_settings_size: Update the setting size in flash library
 *
 *  @flash_lib_ptr: pointer to flash library
 *
 *  return TRUE if success else FALSE
 **/
static boolean flash_update_settings_size(flash_lib_t *flash_lib_ptr)
{
  int32_t size = 0;
  int32_t max_size = I2C_REG_SET_MAX;

  /* Update size for flash_off_settings */
  size = sensor_util_get_setting_size(
    flash_lib_ptr->i2c_flash_info.flash_off_settings.reg_setting_a, max_size);
  RETURN_FALSE_IF(size < 0);
  flash_lib_ptr->i2c_flash_info.flash_off_settings.size = size;

  /* Update size for flash_init_settings */
  size = sensor_util_get_setting_size(
    flash_lib_ptr->i2c_flash_info.flash_init_settings.reg_setting_a, max_size);
  RETURN_FALSE_IF(size < 0);
  flash_lib_ptr->i2c_flash_info.flash_init_settings.size = size;

  /* Update size for flash_low_settings */
  size = sensor_util_get_setting_size(
    flash_lib_ptr->i2c_flash_info.flash_low_settings.reg_setting_a, max_size);
  RETURN_FALSE_IF(size < 0);
  flash_lib_ptr->i2c_flash_info.flash_low_settings.size = size;

  /* Update size for flash_high_settings */
  size = sensor_util_get_setting_size(
    flash_lib_ptr->i2c_flash_info.flash_high_settings.reg_setting_a, max_size);
  RETURN_FALSE_IF(size < 0);
  flash_lib_ptr->i2c_flash_info.flash_high_settings.size = size;

  return TRUE;
}

/** flash_init:
 *    @flash_ctrl: LED flash control handle
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
static int32_t flash_init(void *ptr, void *data)
{
  int32_t rc    = SENSOR_SUCCESS;
  int32_t i     = 0;
  char    *name = (char *)data;

  sensor_flash_data_t *flash_ctrl = (sensor_flash_data_t*)ptr;
  struct msm_flash_init_info_t flash_init_info;
  struct msm_flash_cfg_data_t flash_cfg;
  struct msm_camera_i2c_reg_setting_array *flash_init_settings = NULL;
  struct msm_sensor_power_setting_array *power_setting_array = NULL;

  SDBG("Enter");

  /* memset flash init info */
  memset(&flash_init_info, 0, sizeof(flash_init_info));

  /* Load flash library */
  rc = flash_load_lib(ptr, name);
  if (rc < 0) {
    SERR("flash_load_lib for camera failed");
    return rc;
  }

  if (flash_update_settings_size(flash_ctrl->driver_lib_data) == FALSE) {
    SERR("flash_update_settings_size failed");
    goto flash_unload_lib;
  }

  switch (flash_ctrl->driver_lib_data->flash_driver_type) {
    case FLASH_DRIVER_TYPE_PMIC:
        flash_init_info.flash_driver_type = FLASH_DRIVER_PMIC;
        break;
    case FLASH_DRIVER_TYPE_I2C:
        flash_init_info.flash_driver_type = FLASH_DRIVER_I2C;
        break;
    case FLASH_DRIVER_TYPE_GPIO:
        flash_init_info.flash_driver_type = FLASH_DRIVER_GPIO;
        break;
    case FLASH_DRIVER_TYPE_DEFAULT:
        flash_init_info.flash_driver_type = FLASH_DRIVER_DEFAULT;
        break;
    default:
        SERR("invalid flash driver type = %d",
            flash_ctrl->driver_lib_data->flash_driver_type);
        goto flash_unload_lib;
  }

  flash_init_info.slave_addr =
    flash_ctrl->driver_lib_data->i2c_flash_info.slave_addr;
  flash_init_info.i2c_freq_mode =
    sensor_sdk_util_get_i2c_freq_mode(
    flash_ctrl->driver_lib_data->i2c_flash_info.i2c_freq_mode);
  /* Translate power settings from uspace structure to kernel struct */
  power_setting_array = (struct msm_sensor_power_setting_array *)malloc(
    sizeof(*power_setting_array));
  RETURN_ERROR_ON_NULL(power_setting_array);
  translate_camera_power_setting(power_setting_array,
    &(flash_ctrl->driver_lib_data->power_setting_array));

  /* Translate reg array settings from uspace structure to kernel struct */
  flash_init_settings = (struct msm_camera_i2c_reg_setting_array *)malloc(
    sizeof(*flash_init_settings));

  if (flash_init_settings == NULL) {
    free(power_setting_array);
    SERR("failed flash_init_settings is NULL");
    return SENSOR_FAILURE;
  }
  translate_sensor_reg_setting_array(flash_init_settings,
    &(flash_ctrl->driver_lib_data->i2c_flash_info.flash_init_settings));

  flash_init_info.power_setting_array = power_setting_array;
  flash_init_info.settings = flash_init_settings;

  for (i = 0; i < MAX_LED_TRIGGERS; i++) {
    flash_cfg.flash_current[i] =
      flash_ctrl->driver_lib_data->max_flash_current[i];
    flash_cfg.flash_duration[i] =
      flash_ctrl->driver_lib_data->max_flash_duration[i];
    SLOW("i = %d flash_current = %d flash_duration = %d",
      i, flash_cfg.flash_current[i], flash_cfg.flash_duration[i]);
  }

  flash_cfg.cfg_type = CFG_FLASH_INIT;
  flash_cfg.cfg.flash_init_info = &flash_init_info;

  rc = ioctl(flash_ctrl->fd, VIDIOC_MSM_FLASH_CFG, &flash_cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_FLASH_CFG failed %s", strerror(errno));
    free(power_setting_array);
    free(flash_init_settings);
    return SENSOR_FAILURE;
  }

  for (i = 0; i < MAX_LED_TRIGGERS; i++) {
    flash_ctrl->flash_max_current[i] = flash_cfg.flash_current[i];
    flash_ctrl->flash_max_duration[i] = flash_cfg.flash_duration[i];
    SLOW("i = %d flash_max_current = %d flash_max_duration = %d",
      i, flash_ctrl->flash_max_current[i], flash_ctrl->flash_max_duration[i]);
  }

  SDBG("Exit");

  free(power_setting_array);
  free(flash_init_settings);
  return SENSOR_SUCCESS;

flash_unload_lib:
  /* Close the driver handle */
  if (flash_ctrl->driver_lib_handle) {
    dlclose(flash_ctrl->driver_lib_handle);
    flash_ctrl->driver_lib_handle = NULL;
  }
  return SENSOR_FAILURE;
}

/** flash_process:
 *    @flash_ctrl: LED flash control handle
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

static int32_t flash_process(void *ctrl,
  sensor_submodule_event_type_t event, void *data)
{
  int32_t                     rc = SENSOR_SUCCESS;
  sensor_flash_data_t        *flash_ctrl = NULL;
  rer_cfg_t                  *rer = NULL;
  module_sensor_params_t     *led_module_params = NULL;
  red_eye_reduction_type     *rer_chromatix = NULL;
  dual_led_settings_t        *dual_led_setting = NULL;
  int32_t                    *mode = 0;
  uint8_t                     flashPower = 0;
  int32_t                     temp = 0;
  struct msm_flash_cfg_data_t cfg;
  int32_t                     default_current = -1;
  struct msm_camera_i2c_reg_setting_array *flash_settings = NULL;

  RETURN_ERROR_ON_NULL(ctrl);

  flash_ctrl = (sensor_flash_data_t *)ctrl;
  rer = flash_ctrl->rer;
  dual_led_setting = &flash_ctrl->dual_led_setting;

  /* Initilize to invalid value
   * Kernel uses default values if not updated */
  cfg.flash_current[0] = default_current;
  cfg.flash_current[1] = default_current;

  /* Translate reg array settings from uspace structure to kernel struct */
  flash_settings = (struct msm_camera_i2c_reg_setting_array *)malloc(
    sizeof(*flash_settings));
  RETURN_ERROR_ON_NULL(flash_settings);

  SLOW("event %d", event);

  switch (event) {
  case LED_FLASH_GET_MAX_CURRENT: {
    int32_t ** flash_current = (int32_t **)data;
    *flash_current = &(flash_ctrl->flash_max_current[0]);
    goto flash_process_Exit;
  }
  case LED_FLASH_GET_MAX_DURATION: {
    uint32_t* flash_duration = (uint32_t*)data;
    *flash_duration = flash_ctrl->flash_max_duration[0];
    goto flash_process_Exit;
  }
  case LED_FLASH_SET_CURRENT: {
    /* Get currents for dual LED */
    rc = flash_set_current(data, dual_led_setting);
    goto flash_process_Exit;
  }
  case LED_FLASH_QUERY_CURRENT: {
    int32_t* max_avail_current = (int32_t*)data;
#ifdef __ANDROID__
      char value[200];
      property_get("persist.vendor.camera.flashderate", value, "0");
      if (atoi(value) > 0) {
        property_get("persist.vendor.camera.deratecurrent", value, "900");
        *max_avail_current = atoi(value);
        SHIGH("max_avail_current is %d", *max_avail_current);
        goto flash_process_Exit;
      }
#endif
#ifdef FLASH_QUERY_CURRENT
    struct msm_flash_query_data_t cfg_query;
    cfg_query.query_type = FLASH_QUERY_CURRENT;
    rc = ioctl(flash_ctrl->fd, VIDIOC_MSM_FLASH_QUERY_DATA, &cfg_query);
    if (rc < 0) {
      SERR("VIDIOC_MSM_FLASH_QUERY_DATA failed %s", strerror(errno));
      rc = SENSOR_FAILURE;
      goto flash_process_Exit;
    }
    *max_avail_current = cfg_query.max_avail_curr;
    SHIGH("max_avail_curr returned from PMIC is : %d",*max_avail_current);
    goto flash_process_Exit;
  }
#else
    SERR("LED_FLASH_QUERY_CURRENT not supported");
    rc = SENSOR_FAILURE;
    goto flash_process_Exit;
  }
#endif
  case LED_FLASH_INIT:
    rc = flash_init(flash_ctrl, data);
    goto flash_process_Exit;
  case LED_FLASH_SET_RER_CHROMATIX: {
    rer_chromatix = (red_eye_reduction_type *)data;
    /* Get (RER) data from chromatix */
    rc = flash_rer_set_chromatix(rer, rer_chromatix);
    goto flash_process_Exit;
  }
  case LED_FLASH_GET_RER_PARAMS: {
    *(int32_t *) data = rer->cfg->red_eye_reduction_led_flash_enable;
    goto flash_process_Exit;
  }
  case LED_FLASH_SET_RER_PARAMS: {
    mode = (int32_t *)data;
    rc = flash_rer_set_parm(rer, *mode);
    goto flash_process_Exit;
  }
  case LED_FLASH_SET_FIRING_POWER:{
    flashPower = *(uint8_t *)data;
    goto flash_process_Exit;
  }
  case LED_FLASH_SET_RER_PROCESS: {
    led_module_params = (module_sensor_params_t *)data;
    rc = flash_rer_sequence_process(rer, led_module_params);
    goto flash_process_Exit;
  }
  case LED_FLASH_SET_OFF:
    SHIGH("Turning off flash");
    translate_sensor_reg_setting_array(flash_settings,
      &(flash_ctrl->driver_lib_data->i2c_flash_info.flash_off_settings));
    cfg.cfg_type = CFG_FLASH_OFF;
    cfg.cfg.settings = flash_settings;
    break;
  case LED_FLASH_SET_TORCH:
  case LED_FLASH_SET_PRE_FLASH: {
    SHIGH("Turning on torch/pre flash");
    /* Pre flash mode */
    translate_sensor_reg_setting_array(flash_settings,
      &(flash_ctrl->driver_lib_data->i2c_flash_info.flash_low_settings));
    cfg.cfg_type = CFG_FLASH_LOW;
    cfg.cfg.settings = flash_settings;

    if (dual_led_setting->low_setting[0] == 0 &&
          dual_led_setting->low_setting[1] == 0) {
      cfg.flash_current[0] = data ? *(int32_t *)data : default_current;
      cfg.flash_current[1] = data ? *(int32_t *)data : default_current;
    } else {
      cfg.flash_current[0] = dual_led_setting->low_setting[0];
      cfg.flash_current[1] = dual_led_setting->low_setting[1];
    }
    break;
  }
  case LED_FLASH_SET_RER_PULSE_FLASH: {
    /* RER flash pulses */
    translate_sensor_reg_setting_array(flash_settings,
      &(flash_ctrl->driver_lib_data->i2c_flash_info.flash_high_settings));
    cfg.cfg_type = CFG_FLASH_HIGH;

    /* Use chromatix current if exist (set in RER_PROCESS)*/
    cfg.flash_current[0] = data ? *(int32_t *)data : default_current;
    cfg.flash_current[1] = data ? *(int32_t *)data : default_current;
    cfg.cfg.settings = flash_settings;

    temp = (dual_led_setting->high_setting[0] +
        dual_led_setting->high_setting[1]);
    if (temp > 0) {
      /* Update with Dual LED current */
      cfg.flash_current[0] = (cfg.flash_current[0] *
          dual_led_setting->high_setting[0]) / temp;
      cfg.flash_current[1] = (cfg.flash_current[1] *
          dual_led_setting->high_setting[1]) / temp;
    }
    break;
  }
  case LED_FLASH_SET_MAIN_FLASH: {
    SHIGH("Turning on main flash");
    /* Main flash mode */
    translate_sensor_reg_setting_array(flash_settings,
      &(flash_ctrl->driver_lib_data->i2c_flash_info.flash_high_settings));
    cfg.cfg_type = CFG_FLASH_HIGH;
    cfg.cfg.settings = flash_settings;

    if (dual_led_setting->high_setting[0] == 0 &&
          dual_led_setting->high_setting[1] == 0) {
      cfg.flash_current[0] = data ? *(int32_t *)data : default_current;
      cfg.flash_current[1] = data ? *(int32_t *)data : default_current;
    } else {
      cfg.flash_current[0] = dual_led_setting->high_setting[0];
      cfg.flash_current[1] = dual_led_setting->high_setting[1];
    }
    rc = flash_rer_wait_pupil_contract(rer, led_module_params);
    break;
  }
  case LED_FLASH_GET_FRAME_SKIP_TIME_PARAMS:
  {
    module_sensor_bundle_info_t* s_bundle = (module_sensor_bundle_info_t*)data;
    rc = flash_get_frame_skip_timing_params(s_bundle,flash_ctrl);
    goto flash_process_Exit;
  }
  default:
    SHIGH("failed:invalid event %d", event);
    rc = SENSOR_FAILURE;
    goto flash_process_Exit;
  }

  rc = ioctl(flash_ctrl->fd, VIDIOC_MSM_FLASH_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_FLASH_CFG failed %s", strerror(errno));
    rc = SENSOR_FAILURE;
  }

flash_process_Exit:
  free(flash_settings);
  return rc;
}

/** flash_get_frame_skip_timing_params:
 *    @flash_ctrl: LED flash control handle
 *    @s_bundle: sensor bundle parameters
 *
 * This function returns the frame skip timing parameters from the falsh driver
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/
int32_t flash_get_frame_skip_timing_params(
    module_sensor_bundle_info_t* s_bundle, sensor_flash_data_t *flash_ctrl)
{
  if(!s_bundle || !flash_ctrl)
  {
    return SENSOR_FAILURE;
  }

  s_bundle->main_flash_on_frame_skip =
      flash_ctrl->driver_lib_data->main_flash_on_frame_skip;
  s_bundle->main_flash_off_frame_skip =
      flash_ctrl->driver_lib_data->main_flash_off_frame_skip;
  s_bundle->torch_on_frame_skip =
      flash_ctrl->driver_lib_data->torch_on_frame_skip;
  s_bundle->torch_off_frame_skip =
      flash_ctrl->driver_lib_data->torch_off_frame_skip;

  SLOW("Flash frame skip timing params: main flash on = %d, "
      "main flash off = %d, torch on = %d, torch off = %d",
      s_bundle->main_flash_on_frame_skip, s_bundle->main_flash_off_frame_skip,
      s_bundle->torch_on_frame_skip, s_bundle->torch_off_frame_skip);

  return SENSOR_SUCCESS;
}

/** flash_close:
 *    @flash_ctrl: LED flash control handle
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

static int32_t flash_close(void *flash_ctrl)
{
  int32_t                     rc = SENSOR_SUCCESS;
  sensor_flash_data_t    *ctrl;
  struct msm_flash_cfg_data_t cfg;

  SDBG("Enter");
  RETURN_ERROR_ON_NULL(flash_ctrl);
  ctrl = (sensor_flash_data_t *)flash_ctrl;

  /* Close the driver handle */
  if (ctrl->driver_lib_handle) {
    dlclose(ctrl->driver_lib_handle);
    ctrl->driver_lib_handle = NULL;
  }

  cfg.cfg_type = CFG_FLASH_RELEASE;
  rc = ioctl(ctrl->fd, VIDIOC_MSM_FLASH_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_FLASH_CFG failed %s",
      strerror(errno));
  }

  /* close subdev */
  close(ctrl->fd);

  if (ctrl->rer != NULL) {
    if (ctrl->rer->cfg != NULL) {
      /* Free rer->cfg */
      free(ctrl->rer->cfg);
      ctrl->rer->cfg = NULL;
    }
    /* Free rer */
    free(ctrl->rer);
    ctrl->rer = NULL;
  }

  free(ctrl);
  SDBG("Exit");
  return rc;
}

/** flash_sub_module_init:
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

int32_t led_flash_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  RETURN_ERROR_ON_NULL(func_tbl);

  func_tbl->open = flash_open;
  func_tbl->process = flash_process;
  func_tbl->close = flash_close;
  return SENSOR_SUCCESS;
}

/** flash_set_current:
 *    @awb_update/aec_update: Input parameter
 *    @dual_led_setting: Output parameter
 *
 * 1) Copy dual LED currents from awb_update/aec_update to
 *    dual_led_setting
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t flash_set_current(
  dual_led_settings_t *flash_update,
  dual_led_settings_t *dualled_setting)
{

  RETURN_ERR_ON_NULL(flash_update, SENSOR_FAILURE);
  RETURN_ERR_ON_NULL(dualled_setting, SENSOR_FAILURE);

  *dualled_setting = *flash_update;

  /* Verify current values from stats
   * the curret of both leds can not be 0.
   * in that case, set both to initial values
  */
  if (dualled_setting->low_setting[0] == 0 &&
    dualled_setting->low_setting[1] == 0) {
    dualled_setting->low_setting[0] = -1;
    dualled_setting->low_setting[1] = -1;
  }

  if (dualled_setting->high_setting[0] == 0 &&
    dualled_setting->high_setting[1] == 0) {
    dualled_setting->high_setting[0] = -1;
    dualled_setting->high_setting[1] = -1;
  }

  SLOW("dual led current h0: %d, h1: %d, l0: %d, l1: %d",
    dualled_setting->high_setting[0],
    dualled_setting->high_setting[1],
    dualled_setting->low_setting[0],
    dualled_setting->low_setting[1]);

  return SENSOR_SUCCESS;
}

/** flash_rer_set_chromatix:
 *    @rer_cfg: Internal flash data for RER
 *    @rer_chromatix: RER data from Chromatix
 *
 * 1) Get Red eye reduction (RER) data from chromatix
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t flash_rer_set_chromatix(
  rer_cfg_t *rer,
  red_eye_reduction_type *rer_chromatix)
{
  int32_t flash_enable;
  red_eye_reduction_type *rer_cfg = NULL;

  RETURN_ERROR_ON_NULL(rer);
  RETURN_ERROR_ON_NULL(rer_chromatix);

  rer_cfg = (red_eye_reduction_type *)rer->cfg;

  /* Save red_eye_reduction_led_flash_enable state */
  flash_enable = rer_cfg->red_eye_reduction_led_flash_enable;
  memcpy(rer_cfg, rer_chromatix, sizeof(red_eye_reduction_type));
  /* Restore red_eye_reduction_led_flash_enable state */
  rer_cfg->red_eye_reduction_led_flash_enable = flash_enable;

  return SENSOR_SUCCESS;
}

/** input_check:
 *    @value: Input value
 *    @min: Minimum value
 *    @max: Maximum value
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

inline int32_t input_check(int32_t value, int32_t min, int32_t max)
{
  if(value < min)
    return SENSOR_FAILURE;
  else if(value > max)
    return SENSOR_FAILURE;
  else
    return SENSOR_SUCCESS;
}

/** flash_rer_sequence_process:
 *    @led_module_params: Led module parameters
 *    @rer_cfg: Internal flash data for RER
 *
 * 1) Execute Red eye reduction (RER) sequence
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t flash_rer_sequence_process(
  rer_cfg_t *rer,
  module_sensor_params_t *led_module_params)
{
  int32_t  rc = SENSOR_SUCCESS;
  red_eye_reduction_type *rer_cfg = NULL;
  int32_t flash_enable;
  int32_t preflash_cycles;
  int32_t LED_pulse_duration_ms;
  int32_t interval_pulese_ms;
  int32_t LED_current_mA;
  int32_t sequence_time;

  RETURN_ERROR_ON_NULL(led_module_params);
  RETURN_ERROR_ON_NULL(rer);

  rer_cfg = (red_eye_reduction_type *)rer->cfg;

  /* Get chromatix RER data from rer_cfg */
  flash_enable = rer_cfg->red_eye_reduction_led_flash_enable;
  preflash_cycles  = rer_cfg->number_of_preflash_cycles;
  LED_pulse_duration_ms = rer_cfg->preflash_LED_pulse_duration;
  interval_pulese_ms = rer_cfg->preflash_interval_between_pulese;
  LED_current_mA = rer_cfg->preflash_LED_current;

  rer->status = RER_START;

  /* Before RER sequence is enabled flash need to be turned off */
  led_module_params->func_tbl.process(
        led_module_params->sub_module_private,
        LED_FLASH_SET_OFF , NULL);


  if (flash_enable == 1) {
    /* Red eye procedure is Enabled */
    rc = SENSOR_SUCCESS;
    sequence_time = ((preflash_cycles
        * (LED_pulse_duration_ms + interval_pulese_ms))
        - interval_pulese_ms + RER_PUPIL_CONTRACT_TIME);

    /* Check Red Eye Tuning parameters */
    rc += input_check(preflash_cycles, PREFLASH_CYCLES_MIN, PREFLASH_CYCLES_MAX);
    rc += input_check(LED_pulse_duration_ms, LED_ON_MS_MIN, LED_ON_MS_MAX);
    rc += input_check(interval_pulese_ms, LED_OFF_MS_MIN, LED_OFF_MS_MAX);
    rc += input_check(sequence_time, RER_DURATION_MS_MIN, RER_DURATION_MS_MAX);

    if (rc < 0) {
      SERR("Error: RER parameters out of range \n");
      rer->status = RER_DONE;
      return SENSOR_FAILURE;
    }

    char  value[PROPERTY_VALUE_MAX];
    struct timeval ts_start = {0, 0}, ts_on = {0, 0};
    struct timeval ts_off = {0, 0}, ts_diff = {0, 0};
    boolean  cnt = FALSE;
    property_get("persist.vendor.camera.led.rerdbg", value, "0");
    if (1 == atoi(value)) {
      SLOW("[RER] cycles %d cur %d[mA] p_dur %d[ms] int_p %d[ms] rer_dur %d[ms] \n",
        preflash_cycles,
        LED_current_mA,
        LED_pulse_duration_ms,
        interval_pulese_ms,
        sequence_time
      );

      gettimeofday(&ts_start, NULL);
      rer->rer_start_ts = ts_start;
      SLOW("[RER] [%10ld]us Start RER procedure",
        ts_start.tv_sec*1000000L + ts_start.tv_usec);
    }

    /* RER procedure */
    while (preflash_cycles) {
      property_get("persist.vendor.camera.led.rerdbg", value, "0");
      if (1 == atoi(value)) {
        gettimeofday(&ts_on, NULL);
        timersub(&ts_on, &ts_off, &ts_diff);
        if (cnt == FALSE) {
          SLOW("[RER] [%10ld]us FLASH_ON",
            ts_on.tv_sec*1000000L + ts_on.tv_usec);
          cnt = TRUE;
        } else {
          SLOW("[RER] [%10ld]us FLASH_ON  (Off dur %7ld us)",
            ts_on.tv_sec*1000000L + ts_on.tv_usec,
            ts_diff.tv_sec*1000000L + ts_diff.tv_usec);
        }
      }
      led_module_params->func_tbl.process(
        led_module_params->sub_module_private,
        LED_FLASH_SET_RER_PULSE_FLASH , &LED_current_mA);

      usleep((uint32_t)LED_pulse_duration_ms*1000);

      property_get("persist.vendor.camera.led.rerdbg", value, "0");
      if (1 == atoi(value)) {
        gettimeofday(&ts_off, NULL);
        timersub(&ts_off, &ts_on, &ts_diff);
        SLOW("[RER] [%10ld]us FLASH_OFF (On  dur %7ld us)",
          ts_off.tv_sec*1000000L + ts_off.tv_usec,
          ts_diff.tv_sec*1000000L + ts_diff.tv_usec);
      }
      led_module_params->func_tbl.process(
        led_module_params->sub_module_private,
        LED_FLASH_SET_OFF , NULL);

      if (preflash_cycles <= 1) {
        /* Last flash pulse */
        rer->status = RER_WAIT_PUPIL_CONTRACT;
        gettimeofday(&rer->last_rer_flash_ts, NULL);
        break;
      } else {
        /* Generate interval between the pulses */
        usleep((uint32_t)interval_pulese_ms*1000);
      }
      preflash_cycles--;
    }
  }

  if (rer->status == RER_START) {
    rer->status = RER_DONE;
  }

  return SENSOR_SUCCESS;
}


/** flash_rer_wait_pupil_contract:
 *    @rer_cfg: Internal flash data for RER
 *    @led_module_params: Led module parameters
 *
 * 1) Wait before to start the
 *    main flash after RER sequence
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t flash_rer_wait_pupil_contract(
  rer_cfg_t *rer,
  module_sensor_params_t *led_module_params __attribute__((unused)))
{
  RETURN_ERROR_ON_NULL(rer);

  if (rer->status == RER_WAIT_PUPIL_CONTRACT) {
    /* Wait before to start the main flash after RER sequence */
    int64_t  delay;
    char     value[PROPERTY_VALUE_MAX];
    struct timeval ts_now, ts, ts_diff;

    ts = rer->last_rer_flash_ts;

    gettimeofday(&ts_now, NULL);
    timersub(&ts_now, &ts, &ts_diff);
    delay = (ts_diff.tv_sec * 1000000) + ts_diff.tv_usec;
    delay = (int64_t)(RER_PUPIL_CONTRACT_TIME * 1000) - delay;

    if (delay > 0) {
      /* Wait until Pupil contraction time RER_PUPIL_CONTRACT_TIME */
      if (delay <= (RER_PUPIL_CONTRACT_TIME * 1000)) {
        SLOW("Wait %jd us to reach RER_PUPIL_CONTRACT_TIME\n", delay);
        usleep((uint32_t)delay);
      } else {
        /* Error - delay can not be bigger than RER_PUPIL_CONTRACT_TIME */
        SERR("Error - RER delay out of range %jd us\n", delay);
      }
    } else {
      /* Delay is bigger than the requested RER_PUPIL_CONTRACT_TIME */
      SHIGH("RER Over delay %jd us (total delay %jd us)\n",
        -(int64_t)delay,
        (int64_t)(RER_PUPIL_CONTRACT_TIME * 1000) - delay);
    }

    property_get("persist.vendor.camera.led.rerdbg", value, "0");
    if (1 == atoi(value)) {
      gettimeofday(&ts_now, NULL);
      SLOW("[RER] Done");
      /* Pupil Contract Time */
      timersub(&ts_now, &rer->last_rer_flash_ts, &ts_diff);
      SLOW("[RER] Pupil_contract_time : %ld ms",
        ts_diff.tv_sec*1000 + ts_diff.tv_usec/1000);
      /* RER duration */
      timersub(&ts_now, &rer->rer_start_ts, &ts_diff);
      SLOW("[RER] RER_Duration : %ld ms",
        ts_diff.tv_sec*1000 + ts_diff.tv_usec/1000);
    }

    rer->status = RER_DONE;
  }
  return SENSOR_SUCCESS;
}

/** flash_rer_set_parm:
 *    @rer_cfg:
 *    @mode:
 *
 * 1) Execute Red eye reduction (RER) sequence
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t flash_rer_set_parm(
  rer_cfg_t *rer,
  int32_t mode)
{
  red_eye_reduction_type *rer_cfg = NULL;

  RETURN_ERROR_ON_NULL(rer);

  rer_cfg = (red_eye_reduction_type *)rer->cfg;

  /* Enable/Disable - Red Eye Reduction procedure (RER) */
  rer_cfg->red_eye_reduction_led_flash_enable = mode;
  rer_cfg->red_eye_reduction_xenon_strobe_enable = mode;

  return SENSOR_SUCCESS;
}

