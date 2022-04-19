/* actuator.c
 *
 * Copyright (c) 2012-2018 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <inttypes.h>
#include <string.h>
#include <sys/types.h>
#include <media/msmb_camera.h>
#include <dlfcn.h>
#include "mct_event_stats.h"
#include "sensor_common.h"
#include "actuator.h"
#include "eebin_interface.h"

#define PATH_SIZE_255 255

/** af_actuator_power_up: Power up actuator
 *
 *  @ptr: pointer to actuator_data_t struct
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function moves lens to infinity position **/
static int32_t af_actuator_power_up(void *ptr)
{
  int                          rc = 0;
  uint32_t                     delay = 0;
  actuator_data_t              *af_actuator_ptr = (actuator_data_t *)ptr;
  struct msm_actuator_cfg_data cfg;

  SLOW("enter");
  cfg.cfgtype = CFG_ACTUATOR_POWERUP;
  rc = ioctl(af_actuator_ptr->fd, VIDIOC_MSM_ACTUATOR_CFG, &cfg);
  if (rc < 0) {
    SERR("failed: rc = %d", rc);
    return -EIO;
  }

  delay = af_actuator_ptr->ctrl->driver_params->actuator_params.power_on_delay;
  if (delay)
    usleep (delay);

  SDBG("exit");
  return 0;
}

/** af_actuator_power_down: Power off actuator
 *
 *  @ptr: pointer to actuator_data_t struct
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function moves lens to infinity position **/
static int32_t af_actuator_power_down(void *ptr)
{
  int                          rc = 0;
  actuator_data_t              *af_actuator_ptr = (actuator_data_t *)ptr;
  struct msm_actuator_cfg_data cfg;

  SLOW("enter");

  cfg.cfgtype = CFG_ACTUATOR_POWERDOWN;
  rc = ioctl(af_actuator_ptr->fd, VIDIOC_MSM_ACTUATOR_CFG, &cfg);
  if ( rc < 0) {
    SERR("failed: rc = %d", rc);
    return -EIO;
  }

  SDBG("exit");
  return 0;
}

/** actuator_set_position: function to move lens to desired of
 *  positions with delay
 *
 *  @ptr: pointer to actuator_data_t struct
 *  @data: pointer to af_update_t struct
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function moves lens to set of desired positions as
 *  dictated by 3A algorithm **/
static int actuator_set_position(void *ptr, void *data)
{
  int                          rc = 0;
  int                          index = 0;
  uint32_t                     hw_params;
  uint32_t                     direction;
  actuator_data_t              *af_actuator_ptr = (actuator_data_t *)ptr;
  af_update_t                  *af_update = (af_update_t *)data;
  struct msm_actuator_cfg_data cfg;
  actuator_tuned_params_t      *af_tune_ptr;

  RETURN_ON_NULL(af_actuator_ptr);
  RETURN_ON_NULL(data);

  af_tune_ptr= &(af_actuator_ptr->ctrl->driver_params->actuator_tuned_params);

  if (af_actuator_ptr->fd <= 0) {
    SERR("Invalid fd value %d", af_actuator_ptr->fd);
    return -EINVAL;
  }

  direction = af_update->direction;
  if (direction < NUM_ACTUATOR_DIR) {
    hw_params = af_tune_ptr->damping[direction][0].ringing_params->hw_params;
  } else {
    SERR("Invalid direction %d", direction);
    return -EINVAL;
  }

  cfg.cfgtype = CFG_SET_POSITION;
  cfg.cfg.setpos.number_of_steps = af_update->num_of_interval;
  cfg.cfg.setpos.hw_params       = hw_params;
  for (index = 0; index < cfg.cfg.setpos.number_of_steps; index++) {
     cfg.cfg.setpos.pos[index] = af_update->pos[index];
     cfg.cfg.setpos.delay[index] = af_update->delay[index];
     SLOW("pos:%d, delay:%d\n", cfg.cfg.setpos.pos[index],
       cfg.cfg.setpos.delay[index]);
     af_update->dac_value = af_update->pos[index];
  }

  /* Invoke the IOCTL to set the positions */
  rc = ioctl(af_actuator_ptr->fd, VIDIOC_MSM_ACTUATOR_CFG, &cfg);
  if (rc < 0) {
    SERR("failed: rc = %d", rc);
  }
  return rc;
}

/** af_actuator_set_default_focus: function to move lens to
 *  infinity position
 *
 *  @ptr: pointer to actuator_data_t struct
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function moves lens to infinity position **/

static int32_t af_actuator_set_default_focus(void *ptr)
{
  int32_t rc = 0;
  actuator_data_t *af_actuator_ptr = (actuator_data_t *)ptr;
  struct msm_actuator_cfg_data cfg;
  actuator_driver_params_t *af_driver_ptr =
    af_actuator_ptr->ctrl->driver_params;
  struct damping_params_t *ringing_params = NULL;
  uint16_t curr_scene = 0;
  uint16_t scenario_size = 0;
  uint16_t index = 0;

  RETURN_ERR_ON_NULL(af_driver_ptr, -EINVAL);
  if (af_actuator_ptr->fd <= 0)
    return -EINVAL;

  cfg.cfgtype = CFG_SET_DEFAULT_FOCUS;
  cfg.cfg.move.dir = MOVE_FAR;
  cfg.cfg.move.sign_dir = -1;
  cfg.cfg.move.num_steps = af_actuator_ptr->curr_step_pos;
  cfg.cfg.move.dest_step_pos = 0;
  curr_scene = 0;
  /* Determine scenario */
  scenario_size = af_driver_ptr->actuator_tuned_params.
    scenario_size[MOVE_FAR];
  for (index = 0; index < scenario_size; index++) {
    if (af_actuator_ptr->curr_step_pos <=
      af_driver_ptr->actuator_tuned_params.
      ringing_scenario[MOVE_FAR][index]) {
      curr_scene = index;
      break;
    }
  }

  /* Translate ringing params from uspace structure to kernel struct */
  ringing_params = (struct damping_params_t *)malloc(
    sizeof(*ringing_params) * af_driver_ptr->actuator_tuned_params.region_size);
  RETURN_ERR_ON_NULL(ringing_params, -EINVAL);
  translate_actuator_damping_param(ringing_params,
    &(af_driver_ptr->actuator_tuned_params.
    damping[MOVE_FAR][curr_scene].ringing_params[0]),
    af_driver_ptr->actuator_tuned_params.region_size);

  cfg.cfg.move.ringing_params = ringing_params;

  SLOW("dir:%d, steps:%d", cfg.cfg.move.dir, cfg.cfg.move.num_steps);

  /* Invoke the IOCTL to set the default focus */
  rc = ioctl(af_actuator_ptr->fd, VIDIOC_MSM_ACTUATOR_CFG, &cfg);
  if (rc < 0) {
    SERR("failed: rc = %d", rc);
  }

  af_actuator_ptr->curr_lens_pos = cfg.cfg.move.curr_lens_pos;
  af_actuator_ptr->curr_step_pos = 0;
  free(ringing_params);

  return rc;
}

/** af_actuator_move_focus: function to move lens to desired
 *  position
 *
 *  @ptr: pointer to actuator_data_t struct
 *  @data: pointer to af_update_t struct
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function moves lens to desired position as dictated
 *  by 3A algorithm **/

static int32_t af_actuator_move_focus(void *ptr, void *data)
{
  int32_t                       rc = 0;
  actuator_data_t               *af_actuator_ptr = (actuator_data_t *)ptr;
  struct msm_actuator_cfg_data  cfg;
  actuator_driver_params_t      *af_driver_ptr =  NULL;
  struct damping_params_t       *ringing_params = NULL;
  af_update_t                   *af_update = (af_update_t *)data;
  uint16_t                      scenario_size = 0;
  uint16_t                      index = 0;
  uint16_t                      curr_scene = 0;
  int16_t                       dest_step_pos = 0;
  int8_t                        sign_dir = 0;
  int32_t                       direction;
  int32_t                       num_steps;
  int8_t                        bit_shift = 0;

  RETURN_ERR_ON_NULL(af_actuator_ptr, -EINVAL);
  RETURN_ERR_ON_NULL(data, -EINVAL);
  af_driver_ptr = af_actuator_ptr->ctrl->driver_params;
  RETURN_ERR_ON_NULL(af_driver_ptr, -EINVAL);
  if (af_actuator_ptr->fd <= 0) {
    SERR("failed fd %d data %p", af_actuator_ptr->fd, data);
    return -EINVAL;
  }

  if (af_update->reset_lens == TRUE) {
    if (0 == af_actuator_ptr->curr_lens_pos) {
      SLOW("Reset lens: calling actuator_set_position");
      af_update->num_of_interval = 1;
      /*For bi-vcm MOVE_FAR as the lens will be in the centre for other
        types MOVE_NEAR*/
      if (af_driver_ptr->actuator_params.act_type == ACTUATOR_TYPE_BIVCM) {
        af_update->direction = MOVE_FAR;
      } else {
        af_update->direction = MOVE_NEAR;
      }
      af_update->pos[0] = af_driver_ptr->actuator_tuned_params.initial_code;
      af_update->delay[0] = 0;
      rc = actuator_set_position(ptr, data);
      af_update->dac_value = af_driver_ptr->actuator_tuned_params.initial_code;
      if (rc < 0)
        SERR("failed rc %d", rc);
    } else {
      SLOW("calling af_actuator_set_default_focus");
      rc = af_actuator_set_default_focus(ptr);
      af_update->dac_value = af_actuator_ptr->curr_lens_pos;
      if (rc < 0)
        SERR("failed rc %d", rc);
    }
    return rc;
  }

  num_steps = (int32_t)af_update->num_of_steps;
  direction = af_update->direction;

  if (af_update->move_lens != TRUE) {
    SLOW("move_lens not requested");
    return rc;
  }

  SLOW("num_steps %d dir %d",  num_steps, direction);
  if (direction == MOVE_NEAR)
    sign_dir = 1;
  else if (direction == MOVE_FAR)
    sign_dir = -1;
  else {
    SERR("invalid direction: %d", direction);
    return -EINVAL;
  }

  dest_step_pos = (int16_t)(af_actuator_ptr->curr_step_pos +
    (sign_dir * num_steps));

  if (dest_step_pos < 0)
    dest_step_pos = 0;
  else if (dest_step_pos > af_actuator_ptr->total_steps)
    dest_step_pos = (int16_t)af_actuator_ptr->total_steps;

  cfg.cfgtype = CFG_MOVE_FOCUS;
  cfg.cfg.move.dir           = (int8_t)direction;
  cfg.cfg.move.sign_dir      = sign_dir;
  cfg.cfg.move.num_steps     = num_steps;
  cfg.cfg.move.dest_step_pos = dest_step_pos;

  curr_scene = 0;
  /* Determine scenario */
  scenario_size = af_driver_ptr->actuator_tuned_params.scenario_size[direction];
  for (index = 0; index < scenario_size; index++) {
    if (num_steps <=
      af_driver_ptr->actuator_tuned_params.ringing_scenario[direction][index]) {
      curr_scene = index;
      break;
    }
  }

  /* Translate ringing params from uspace structure to kernel struct */
  ringing_params = (struct damping_params_t *)malloc(
    sizeof(*ringing_params) * af_driver_ptr->actuator_tuned_params.region_size);
  RETURN_ERR_ON_NULL(ringing_params, -EINVAL);
  translate_actuator_damping_param(ringing_params,
    &(af_driver_ptr->actuator_tuned_params.
    damping[direction][curr_scene].ringing_params[0]),
    (uint32_t)af_driver_ptr->actuator_tuned_params.region_size);

  cfg.cfg.move.ringing_params = ringing_params;

  /* Invoke the IOCTL to move the focus */
  rc = ioctl(af_actuator_ptr->fd, VIDIOC_MSM_ACTUATOR_CFG, &cfg);
  if (rc < 0) {
    SERR("failed rc: %d", rc);
  }
  af_actuator_ptr->curr_step_pos = dest_step_pos;
  af_actuator_ptr->curr_lens_pos = cfg.cfg.move.curr_lens_pos;

  /* sign extension for BIVCM type to indicate negative value */
  if(af_driver_ptr->actuator_params.act_type == ACTUATOR_TYPE_BIVCM) {
    bit_shift = (sizeof(af_update->dac_value) * 8) -
      af_driver_ptr->actuator_params.data_size;
    af_actuator_ptr->curr_lens_pos =
      ((short)((af_actuator_ptr->curr_lens_pos) <<bit_shift))>>bit_shift;
  }

  af_update->dac_value = af_actuator_ptr->curr_lens_pos;

  SLOW("bit_shift:%d, curr_step_pos:%d, curr_len_pos:%d",
    bit_shift, af_actuator_ptr->curr_step_pos,
    af_actuator_ptr->curr_lens_pos);

  free(ringing_params);

  if (af_actuator_ptr->plot_info.size < MAX_ACTUATOR_PLOT_INFO) {
    af_actuator_ptr->plot_info.step_pos[af_actuator_ptr->plot_info.size] =
      af_actuator_ptr->curr_step_pos;
    af_actuator_ptr->plot_info.lens_pos[af_actuator_ptr->plot_info.size++] =
      af_actuator_ptr->curr_lens_pos;
  }
  else {
    af_actuator_ptr->plot_info.size = 0;
  }
  return rc;
}

/** af_actuator_get_info: function to return whether af is
 *  supported
 *
 *  @ptr: pointer to actuator_data_t struct
 *  @data: pointer to uint8_t
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function returns 1 if af is supported, 0 otherwise **/

static int32_t __attribute__((unused)) af_actuator_get_info(void *ptr, void *data)
{
  int32_t rc = 0;
  actuator_data_t *af_actuator_ptr = (actuator_data_t *)ptr;
  uint8_t *af_support = (uint8_t *)data;
  if (!af_support) {
    SERR("failed");
    return -EINVAL;
  }
  *af_support = af_actuator_ptr->is_af_supported;
  return rc;
}

/** af_actuator_set_params: set the header params to the
 *  af driver in kernel
 *
 *  @ptr: pointer to actuator_data_t struct
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function returns 1 if af is supported, 0 otherwise **/

static int32_t af_actuator_set_params(void *ptr)
{
  int32_t rc = 0;
  struct msm_actuator_cfg_data cfg;
  actuator_data_t *af_actuator_ptr = (actuator_data_t *)ptr;
  uint16_t total_steps = 0;
  actuator_driver_params_t *af_driver_ptr = NULL;
  actuator_tuned_params_t *actuator_tuned_params = NULL;
  actuator_params_t *actuator_params = NULL;
  struct msm_actuator_reg_params_t *reg_params = NULL;
  struct reg_settings_t *init_settings = NULL;
  struct region_params_t *region_params = NULL;

  /* Validate parameters */
  RETURN_ERR_ON_NULL(af_actuator_ptr, -EINVAL);
  RETURN_ERR_ON_NULL(af_actuator_ptr->ctrl, -EINVAL);

  af_driver_ptr = af_actuator_ptr->ctrl->driver_params;
  RETURN_ERR_ON_NULL(af_driver_ptr, -EINVAL);
  if (af_actuator_ptr->is_af_supported) {
    actuator_tuned_params = &af_driver_ptr->actuator_tuned_params;
    actuator_params = &af_driver_ptr->actuator_params;

    SLOW("Enter");
    memset(&cfg, 0, sizeof(struct msm_actuator_cfg_data));
    cfg.cfgtype = CFG_SET_ACTUATOR_INFO;
    total_steps = actuator_tuned_params->region_params[
        actuator_tuned_params->region_size - 1].step_bound[0] -
        actuator_tuned_params->region_params[0].step_bound[1];

    if (total_steps <= 0) {
      SERR("Invalid total_steps");
      return -EFAULT;
    }

    /* Translate reg params from uspace structure to kernel struct */
    reg_params = (struct msm_actuator_reg_params_t *) malloc(
      sizeof(*reg_params) * actuator_params->reg_tbl.reg_tbl_size);
    RETURN_ERR_ON_NULL(reg_params, -EINVAL, "Null ptr - reg_params");
    translate_actuator_reg_params(reg_params,
      &(actuator_params->reg_tbl.reg_params[0]),
      actuator_params->reg_tbl.reg_tbl_size);

    /* Translate reg settings from uspace structure to kernel struct */
    init_settings = (struct reg_settings_t *) malloc(
      sizeof(*init_settings) * actuator_params->init_setting_size);

    if (init_settings == NULL) {
        free(reg_params);
        SERR("failed: init_settings is NULL");
        return SENSOR_FAILURE;
    }
    translate_actuator_reg_settings(init_settings,
      &(actuator_params->init_settings[0]),
      actuator_params->init_setting_size);

    /* Translate region params from uspace structure to kernel struct */
    region_params = (struct region_params_t *) malloc(
      sizeof(*region_params) * actuator_tuned_params->region_size);
    if (region_params == NULL) {
        free(reg_params);
        free(init_settings);
        SERR("failed: region_params is NULL");
        return SENSOR_FAILURE;
   }
    translate_actuator_region_params(region_params,
      &(actuator_tuned_params->region_params[0]),
      actuator_tuned_params->region_size);

    af_actuator_ptr->total_steps = total_steps;
    cfg.cfg.set_info.af_tuning_params.total_steps = total_steps;

    switch (actuator_params->act_type) {
      case ACTUATOR_TYPE_VCM:
        cfg.cfg.set_info.actuator_params.act_type = ACTUATOR_VCM;
        break;
      case ACTUATOR_TYPE_PIEZO:
        cfg.cfg.set_info.actuator_params.act_type = ACTUATOR_PIEZO;
        break;
      case ACTUATOR_TYPE_HVCM:
        cfg.cfg.set_info.actuator_params.act_type = ACTUATOR_HVCM;
        break;
      case ACTUATOR_TYPE_BIVCM:
        cfg.cfg.set_info.actuator_params.act_type = ACTUATOR_BIVCM;
        break;
      default:
        SERR("invalid act_type = %d", actuator_params->act_type);
        break;
    }

    cfg.cfg.set_info.af_tuning_params.initial_code =
      actuator_tuned_params->initial_code;
    cfg.cfg.set_info.actuator_params.reg_tbl_size =
      actuator_params->reg_tbl.reg_tbl_size;
    cfg.cfg.set_info.actuator_params.reg_tbl_params = reg_params;
    cfg.cfg.set_info.actuator_params.data_size =
      actuator_params->data_size;
    cfg.cfg.set_info.actuator_params.i2c_addr =
      actuator_params->i2c_addr;
    cfg.cfg.set_info.actuator_params.i2c_freq_mode =
      sensor_sdk_util_get_i2c_freq_mode(
      actuator_params->i2c_freq_mode);
    cfg.cfg.set_info.actuator_params.i2c_addr_type =
      sensor_sdk_util_get_kernel_i2c_addr_type(actuator_params->i2c_addr_type);

    cfg.cfg.set_info.af_tuning_params.region_size =
      actuator_tuned_params->region_size;
    cfg.cfg.set_info.af_tuning_params.region_params = region_params;
    cfg.cfg.set_info.actuator_params.init_setting_size =
      actuator_params->init_setting_size;
    cfg.cfg.set_info.actuator_params.i2c_data_type =
      sensor_sdk_util_get_kernel_i2c_data_type(actuator_params->i2c_data_type);
    cfg.cfg.set_info.actuator_params.init_settings = init_settings;

    /* Lens parking data */
    cfg.cfg.set_info.actuator_params.park_lens.damping_step =
      actuator_tuned_params->damping[0][0].ringing_params[0].damping_step;
    cfg.cfg.set_info.actuator_params.park_lens.damping_delay =
      actuator_tuned_params->damping[0][0].ringing_params[0].damping_delay;
    cfg.cfg.set_info.actuator_params.park_lens.hw_params =
      actuator_tuned_params->damping[0][0].ringing_params[0].hw_params;
    cfg.cfg.set_info.actuator_params.park_lens.max_step = ACTUATOR_PARK_STEP;


    /* Invoke the IOCTL to set the af parameters to the kernel driver */
    rc = ioctl(af_actuator_ptr->fd, VIDIOC_MSM_ACTUATOR_CFG, &cfg);
    if (rc < 0) {
      SERR("failed rc %d", rc);
    }

    free(reg_params);
    free(init_settings);
    free(region_params);
  }

  SDBG("Exit");
  return rc;
}

/** actuator_load_bin: function to load the actuator binary
 *
 *  @ptr: pointer to actuator_data_t struct
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function reads the actuator driver data from the binary file
 **/
static int32_t actuator_load_bin(void *ptr)
{
  FILE *fp = NULL;
  actuator_data_t *af_actuator_ptr = (actuator_data_t *)ptr;

  RETURN_ERR_ON_NULL(af_actuator_ptr, -EINVAL);

  SHIGH("name=%s", af_actuator_ptr->name);

  if (af_actuator_ptr->ctrl->driver_ctrl == NULL) {
    int32_t bytes_read;
    char binary_name[PATH_SIZE_255];

    snprintf(binary_name, PATH_SIZE_255, "%sactuator_%s.bin",
      QCAMERA_DUMP_FRM_LOCATION, af_actuator_ptr->name);

    fp = fopen(binary_name, "rb");
    RETURN_ERR_ON_NULL(fp, -EINVAL, "fopen %s failed", binary_name);

    af_actuator_ptr->ctrl->driver_ctrl = (actuator_driver_ctrl_t *)malloc(
      sizeof(actuator_driver_ctrl_t));
    JUMP_ON_NULL(af_actuator_ptr->ctrl->driver_ctrl, mem_alloc_fail);

    bytes_read = fread(af_actuator_ptr->ctrl->driver_ctrl, 1,
      sizeof(actuator_driver_ctrl_t), fp);
    if (bytes_read != sizeof(actuator_driver_ctrl_t)) {
      SERR("Invalid number of bytes read. bytes_read = %d expected = %d",
        bytes_read, sizeof(actuator_driver_ctrl_t));
      goto read_fail;
    }
    fclose(fp);

    af_actuator_ptr->ctrl->driver_params =
      &(af_actuator_ptr->ctrl->driver_ctrl->actuator_driver_params);
  }

  return 0;

read_fail:
  free(af_actuator_ptr->ctrl->driver_ctrl);
  af_actuator_ptr->ctrl->driver_ctrl = NULL;
mem_alloc_fail:
  fclose(fp);
  return -EINVAL;
}

/** af_load_header: function to load the actuator header
 *
 *  @ptr: pointer to actuator_data_t struct
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function gets cam name index and initializes actuator
 *  control pointer **/
static int32_t actuator_load_lib(void *ptr)
{
  int32_t rc = 0;
  actuator_driver_ctrl_t* driver_lib_data = NULL;
  actuator_data_t *af_actuator_ptr = (actuator_data_t *)ptr;

  RETURN_ERR_ON_NULL(af_actuator_ptr, -EINVAL);

  if (af_actuator_ptr->is_af_supported) {

    void *(*open_lib_func)(void) = NULL;
    char driver_lib_name[PATH_SIZE_255];
    char binary_name[PATH_SIZE_255];
    char driver_open_lib_func_name[64];
    eebin_ctl_t bin_ctl;
    char retry = FALSE;
    char *abs_path = MOD_SENSOR_LIB_PATH;

    RETURN_ERR_ON_NULL(af_actuator_ptr->name, -EINVAL);

    SLOW("name=%s", af_actuator_ptr->name);

    if (af_actuator_ptr->driver_lib_handle == NULL) {
      snprintf(driver_lib_name, PATH_SIZE_255, "libactuator_%s.so",
        af_actuator_ptr->name);
      snprintf(driver_open_lib_func_name, 64, "actuator_driver_open_lib");

      bin_ctl.cmd = EEPROM_BIN_GET_LIB_NAME_DATA;
      bin_ctl.ctl.q_num.type = EEPROM_BIN_LIB_ACTUATOR;
      bin_ctl.ctl.name_data.lib_name = driver_lib_name;
      bin_ctl.ctl.name_data.path = NULL;
      if (af_actuator_ptr->eebin_hdl)
        if (eebin_interface_control(af_actuator_ptr->eebin_hdl, &bin_ctl) < 0)
          SHIGH("No Camera Multimodule data.");

      if (bin_ctl.ctl.name_data.path) {
        snprintf(driver_lib_name, PATH_SIZE_255, "%slibactuator_%s.so",
          bin_ctl.ctl.name_data.path, af_actuator_ptr->name);
      } else if (abs_path){
        retry = TRUE;
        snprintf(driver_lib_name, PATH_SIZE_255, "%s/libactuator_%s.so",
          abs_path, af_actuator_ptr->name);
      } else {
        snprintf(driver_lib_name, PATH_SIZE_255, "libactuator_%s.so",
          af_actuator_ptr->name);
      }

      /* open actuator driver library */
      af_actuator_ptr->driver_lib_handle = dlopen(driver_lib_name, RTLD_NOW);
      if (!af_actuator_ptr->driver_lib_handle && retry) {
        snprintf(driver_lib_name, PATH_SIZE_255, "libactuator_%s.so",
          af_actuator_ptr->name);
        af_actuator_ptr->driver_lib_handle = dlopen(driver_lib_name, RTLD_NOW);
      }
      RETURN_ERR_ON_NULL(af_actuator_ptr->driver_lib_handle, -EINVAL,
        "dlopen() failed to load %s", driver_lib_name);

      *(void **)&open_lib_func = dlsym(af_actuator_ptr->driver_lib_handle,
                                   driver_open_lib_func_name);
      RETURN_ERR_ON_NULL(open_lib_func, -EINVAL,
        "actuator_driver_open_lib failed for %s",driver_lib_name);

      driver_lib_data = (actuator_driver_ctrl_t *)open_lib_func();
      RETURN_ERR_ON_NULL(driver_lib_data, -EINVAL,
        "actuator_lib_data failed for %s",driver_lib_name);

      af_actuator_ptr->ctrl->driver_params =
        &(driver_lib_data->actuator_driver_params);
      SHIGH("library %s successfully loaded", driver_lib_name);
    }
  }
  return rc;
}

/** af_actuator_init: function to initialize actuator
 *
 *  @ptr: pointer to actuator_data_t struct
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function checks whether actuator is supported, gets
 *  cam name index and initializes actuator control pointer **/

static int32_t af_actuator_init(void *ptr, void* data)
{
  int                          i = 0;
  int32_t                      rc = 0;
  actuator_data_t              *af_actuator_ptr = (actuator_data_t *)ptr;
  char                         *name = (char *)data;
  struct msm_actuator_cfg_data cfg;

  /* Validate parameters */
  RETURN_ERR_ON_NULL(af_actuator_ptr, -EINVAL,
    "Invalid Argument - af_actuator_ptr");
  RETURN_ERR_ON_NULL(name, -EINVAL, "Invalid actuator name");

  SLOW("name = %s", (name) ? name : "null");

  af_actuator_ptr->ctrl = NULL;
  af_actuator_ptr->curr_step_pos = 0;
  af_actuator_ptr->cur_restore_pos = 0;
  af_actuator_ptr->name = name;
  af_actuator_ptr->is_af_supported = 1;
  af_actuator_ptr->params_loaded = 0;

  af_actuator_ptr->ctrl = calloc(1, sizeof(actuator_ctrl_t));
  RETURN_ERR_ON_NULL(af_actuator_ptr->ctrl, -EINVAL,
    "Null ptr - af_actuator_ptr->ctrl");

  rc = actuator_load_lib(ptr);
  if (rc < 0) {
    SERR("actuator_load_lib failed so loading binary");
    rc = actuator_load_bin(ptr);
    if (rc < 0) {
      SERR("actuator_load_bin failed: rc = %d", rc);
      return rc;
    }
  }

  rc = af_actuator_power_up(af_actuator_ptr);
  if (rc < 0) {
    SERR("failed rc %d", rc);
  }

  cfg.cfgtype = CFG_ACTUATOR_INIT;

  /* Invoke the IOCTL to initialize the actuator */
  rc = ioctl(af_actuator_ptr->fd, VIDIOC_MSM_ACTUATOR_CFG, &cfg);
  if (rc < 0) {
    SERR("CFG_ACTUATOR_INIT failed: rc = %d",rc);
    return rc;
  }

  return rc;
}

/** af_actuator_linear_test: function for linearity test
 *
 *  @ptr: pointer to actuator_data_t struct
 *  @stepsize: step size for linearity test
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function runs linearity test by moving alternatively in
 *  both direction with above mentioned step size **/

static int32_t af_actuator_linear_test(void *ptr, uint8_t stepsize,
  uint32_t delay)
{
  int32_t rc = 0;
  actuator_data_t *af_actuator_ptr = (actuator_data_t *)ptr;
  uint16_t index;
  af_update_t af_update;
  SLOW("set default focus");
  rc = af_actuator_set_default_focus(ptr);
  usleep(1000000);

  if (rc < 0) {
      SERR("failed rc %d",rc);
      return rc;
  }

  if(stepsize == 0)
     return rc;

  SLOW("linear test MOVE_NEAR");
  for (index = 0; index < af_actuator_ptr->total_steps; index += stepsize) {
    af_update.move_lens = TRUE;
    af_update.direction = MOVE_NEAR;
    af_update.num_of_steps = stepsize;
    rc = af_actuator_move_focus(ptr, &af_update);
    usleep(delay);
  }

  SLOW("linear test MOVE_FAR");
  for (index = 0; index < af_actuator_ptr->total_steps; index += stepsize) {
    af_update.move_lens = TRUE;
    af_update.direction = MOVE_FAR;
    af_update.num_of_steps = stepsize;
    rc = af_actuator_move_focus(ptr, &af_update);
    usleep(delay);
  }
  return rc;
}

/** af_actuator_ring_test: function for ringing test
 *
 *  @ptr: pointer to actuator_data_t struct
 *  @stepsize: step size for linearity test
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function runs ringing test by moving lens from macro
 *  position to infintity position **/

static int32_t af_actuator_ring_test(void *ptr, uint8_t stepsize,
  uint32_t delay)
{
  int32_t rc = 0;
  actuator_data_t *af_actuator_ptr = (actuator_data_t *)ptr;
  uint16_t index;
  af_update_t af_update;

  rc = af_actuator_set_default_focus(ptr);
  usleep(1000000);

  if (rc < 0) {
      SERR("failed rc %d",rc);
      return rc;
  }

  if(stepsize == 0)
     return rc;

  for (index = 0; index < af_actuator_ptr->total_steps; index += stepsize) {
    af_update.move_lens = TRUE;
    af_update.direction = MOVE_NEAR;
    af_update.num_of_steps = stepsize;
    rc = af_actuator_move_focus(ptr, &af_update);
    usleep(delay);
  }

  rc = af_actuator_set_default_focus(ptr);
  usleep(1000000);

  return rc;
}

/** actuator_get_lens_moving_range: function to return lens moving range
 *
 *  @ptr: pointer to actuator_data_t struct
 *  @data: pointer to sensor_get_af_algo_ptr_t *
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function returns the lens moving range
 **/

static int32_t actuator_get_lens_moving_range(void *ptr, void *data)
{
  actuator_data_t              *af_actuator_ptr = (actuator_data_t *)ptr;
  sensor_get_lens_lens_range_t *lens_range =
    (sensor_get_lens_lens_range_t *)data;
  actuator_driver_params_t     *driver_params;
  struct region_params_t       *region_params;

  /* Validate input parameters */
  RETURN_ERR_ON_NULL(af_actuator_ptr, -EINVAL,
    "Invalid Argument - af_actuator_ptr");
  RETURN_ERR_ON_NULL(af_actuator_ptr->ctrl, -EINVAL,
    "Invalid Argument - af_actuator_ptr");
  RETURN_ERR_ON_NULL(af_actuator_ptr->ctrl->driver_params, -EINVAL,
    "Invalid Argument - af_actuator_ptr");
  RETURN_ERR_ON_NULL(lens_range, -EINVAL,
    "Invalid Argument - lens_range");

  driver_params =
    (actuator_driver_params_t *)af_actuator_ptr->ctrl->driver_params;
  region_params =
    (struct region_params_t *)&driver_params->actuator_tuned_params.region_params;

  lens_range->position_far_end = region_params->step_bound[0];
  lens_range->position_near_end = region_params->step_bound[1];

  return 0;
}

/** actuator_get_af_driver_param_ptr: function to return af driver
 *  pointer
 *
 *  @ptr: pointer to actuator_data_t struct
 *  @data: pointer to actuator_driver_params_t *
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function runs ringing test by moving lens from macro
 *  position to infintity position **/

static int32_t actuator_get_af_driver_param_ptr(void *ptr, void *data)
{
  actuator_data_t *af_actuator_ptr = (actuator_data_t *)ptr;
  actuator_driver_params_t **af_driver_ptr = (actuator_driver_params_t **)data;
  if (!af_actuator_ptr || !af_driver_ptr) {
    SERR("failed af_actuator_ptr %p af_tune %p",
      af_actuator_ptr, af_driver_ptr);
    return -EINVAL;
  }
  *af_driver_ptr =
    af_actuator_ptr->ctrl->driver_params;
  return 0;
}

/** actuator_open: function for actuator open
 *
 *  @ptr: pointer to actuator_data_t *
 *  @data: pointer to subdevice name
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function open subdevice and initializes actuator **/

static int32_t actuator_open(void **actuator_ctrl, void *data)
{
  int32_t rc = 0;
  actuator_data_t *ctrl = NULL;
  char subdev_string[32];
  sensor_submodule_info_t *info =
      (sensor_submodule_info_t *)data;

  RETURN_ERROR_ON_NULL(actuator_ctrl);
  RETURN_ERROR_ON_NULL(info);
  RETURN_ERROR_ON_NULL(info->intf_info[SUBDEV_INTF_PRIMARY].sensor_sd_name);

  ctrl = malloc(sizeof(actuator_data_t));
  if (!ctrl) {
    SERR("malloc failed");
    return -EINVAL;
  }

  memset(ctrl, 0, sizeof(actuator_data_t));

  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s",
       info->intf_info[SUBDEV_INTF_PRIMARY].sensor_sd_name);
  /* Open subdev */
  ctrl->fd = open(subdev_string, O_RDWR);
  if (ctrl->fd < 0) {
    SERR("failed");
    rc = -EINVAL;
    goto ERROR;
  }

  *actuator_ctrl = (void *)ctrl;
  return rc;

ERROR:
  free(ctrl);
  return rc;
}

/** actuator_set_af_tuning: function to perform af tuning
 *
 *  @ptr: pointer to actuator_data_t struct
 *  @data: pointer to tune_actuator_t *
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function runs different tuning test for actuator
 *  tuning based on the input parameters **/

static int32_t actuator_set_af_tuning(void *actuator_ctrl, void *data)
{
  int32_t rc = 0;
  tune_actuator_t *tdata = (tune_actuator_t *)data;
  actuator_tuning_type_t ttype = (actuator_tuning_type_t)tdata->ttype;
  SLOW("ttype =%d tdata->stepsize=%d", ttype, tdata->stepsize);
  switch (ttype) {
  case ACTUATOR_TUNE_RELOAD_PARAMS:
    rc = af_actuator_set_params(actuator_ctrl);
    break;
  case ACTUATOR_TUNE_TEST_LINEAR:
    rc = af_actuator_linear_test(actuator_ctrl, tdata->stepsize, 1000000);
    break;
  case ACTUATOR_TUNE_TEST_RING:
    rc = af_actuator_ring_test(actuator_ctrl, tdata->stepsize, 1000000);
    break;
  case ACTUATOR_TUNE_DEF_FOCUS:
    rc = af_actuator_set_default_focus(actuator_ctrl);
    break;
  case ACTUATOR_TUNE_MOVE_FOCUS: {
    af_update_t movedata;
    movedata.move_lens = TRUE;
    movedata.reset_lens = FALSE;
    movedata.direction = tdata->direction;
    movedata.num_of_steps = tdata->num_steps;
    rc = af_actuator_move_focus(actuator_ctrl, &movedata);
    }
    break;
  }
  return rc;
}

static int32_t actuator_set_eebin_data(void *actuator_ctrl, void *data)
{
  int32_t rc = 0;
  actuator_data_t *ctrl = (actuator_data_t *)actuator_ctrl;
  ctrl->eebin_hdl = data;
  return rc;
}

static int32_t actuator_process_live_tuning(
  void *actuator_ctrl, void *data)
{
  actuator_data_t *ctrl = (actuator_data_t *)actuator_ctrl;
  actuator_live_tune_ctrl_t *live_tune_data =
    (actuator_live_tune_ctrl_t *)data;
  int32_t rc = 0;

  switch (live_tune_data->tuning_cmd) {
  case ACTUATOR_LIVE_TUNE_GET_CUR_STEP_POSITION:
    live_tune_data->u.step_position = ctrl->curr_step_pos;
    break;
  case ACTUATOR_LIVE_TUNE_GET_CUR_LENS_POSITION:
    live_tune_data->u.lens_position = ctrl->curr_lens_pos;
    break;
  case ACTUATOR_LIVE_TUNE_LINEARITY_TEST:
    rc = af_actuator_linear_test(actuator_ctrl, live_tune_data->step_size,
      live_tune_data->u.delay);
    break;
  case ACTUATOR_LIVE_TUNE_RINGING_TEST:
    rc = af_actuator_ring_test(actuator_ctrl, live_tune_data->step_size,
      live_tune_data->u.delay);
    break;
  case ACTUATOR_LIVE_TUNE_START_PLOT:
    ctrl->plot_info.size = 0;
    break;
  case ACTUATOR_LIVE_TUNE_STOP_PLOT:
    live_tune_data->u.plot_info = ctrl->plot_info;
    break;
  case ACTUATOR_LIVE_TUNE_SET_DEFAULT_FOCUS:
    rc = af_actuator_set_default_focus(actuator_ctrl);
    break;
  default:
    SHIGH("Invalid tuning command");
  }

  return 0;
}

/** actuator_process: function to drive actuator config
 *
 *  @ptr: pointer to actuator_data_t
 *  @data: pointer to data sent by other modules
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function calls corresponding config function based on
 *  event type **/

static int32_t actuator_process(void *actuator_ctrl,
  sensor_submodule_event_type_t event, void *data)
{
  int32_t rc = 0;

  RETURN_ERR_ON_NULL(actuator_ctrl, -EINVAL);

  switch (event) {
  /* Set params */
  case ACTUATOR_INIT:
    rc = af_actuator_init(actuator_ctrl, data);
    break;
  case ACTUATOR_MOVE_FOCUS:
    rc = af_actuator_move_focus(actuator_ctrl, data);
    break;
  case ACTUATOR_SET_POSITION:
    rc = actuator_set_position(actuator_ctrl, data);
    break;
  case ACTUATOR_SET_PARAMETERS: {
    actuator_data_t *af_actuator_ptr = (actuator_data_t *)actuator_ctrl;
    if (af_actuator_ptr->params_loaded == 0) {
      rc = af_actuator_set_params(actuator_ctrl);
      if (!rc)
        af_actuator_ptr->params_loaded = 1;
    }
    break;
  }
  case ACTUATOR_FOCUS_TUNING:
    SLOW("ACTUATOR_FOCUS_TUNING");
    rc = actuator_set_af_tuning(actuator_ctrl, data);
    break;
    /* Get params */
  case ACTUATOR_GET_LENS_MOVING_RANGE:
    rc = actuator_get_lens_moving_range(actuator_ctrl, data);
    break;
  case ACTUATOR_GET_AF_DRIVER_PARAM_PTR:
    rc = actuator_get_af_driver_param_ptr(actuator_ctrl, data);
    break;
  case ACTUATOR_GET_DAC_VALUE: {
    int16_t *dac_value = (int16_t *)data;
    actuator_data_t *af_actuator_ptr = (actuator_data_t *)actuator_ctrl;
    *dac_value = af_actuator_ptr->curr_lens_pos;
    break;
  }
  case ACTUATOR_SET_EEBIN_DATA:
    rc = actuator_set_eebin_data(actuator_ctrl, data);
    break;
  case ACTUATOR_FOCUS_LIVE_TUNING:
    SHIGH("ACTUATOR_FOCUS_LIVE_TUNING");
    rc = actuator_process_live_tuning(actuator_ctrl, data);
    break;
  default:
    SHIGH("invalid event %d",  event);
    rc = -EINVAL;
    break;
  }
  if (rc < 0) {
    SERR("failed rc %d",  rc);
  }
  return rc;
}

/** actuator_close: function for actuator close
 *
 *  @ptr: pointer to actuator_data_t
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function calls close file description and frees all
 *  control data **/

static int32_t actuator_close(void *actuator_ctrl)
{
  int32_t rc = 0;
  actuator_data_t *ctrl = (actuator_data_t *)actuator_ctrl;
  RETURN_ERROR_ON_NULL(ctrl);

  rc = af_actuator_power_down(ctrl);
  if (rc < 0) {
    SERR("failed rc %d", rc);
  }

  if (ctrl->driver_lib_handle) {
    dlclose(ctrl->driver_lib_handle);
    ctrl->driver_lib_handle = NULL;
  }

  if (ctrl->ctrl) {
    if (ctrl->ctrl->driver_ctrl) {
      free(ctrl->ctrl->driver_ctrl);
      ctrl->ctrl->driver_ctrl = NULL;
    }
    free(ctrl->ctrl);
    ctrl->ctrl = NULL;
  }
  /* close subdev */
  close(ctrl->fd);

  free(ctrl);
  return rc;
}

/** actuator_sub_module_init: function for initializing actuator
 *  sub module
 *
 *  @ptr: pointer to sensor_func_tbl_t
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function initializes sub module function table with
 *  actuator specific functions **/

int32_t actuator_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  SDBG("Enter");
  if (!func_tbl) {
    SERR("failed");
    return -EINVAL;
  }
  func_tbl->open = actuator_open;
  func_tbl->process = actuator_process;
  func_tbl->close = actuator_close;
  return 0;
}
