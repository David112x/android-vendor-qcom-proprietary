/* eeprom.c
 *
 * Copyright (c) 2013-2018 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <dlfcn.h>
#include <math.h>
#include <stdlib.h>
#include <cutils/properties.h>

#include "sensor_common.h"
#include "eeprom_dump.h"
#include "../libs/eeprom_util/eeprom.h"
#define BUFF_SIZE_255 255
#define PATH_SIZE_255 255


/** eeprom_check_property:
 *    @ctrl: format_data_t struct
 *
 * Selectively access the calibration data by setprop value.
 * As a default, we always access.
 **/
static void eeprom_check_property(format_data_t *ctrl)
{
  eeprom_calib_items_t *e_items;
  char                  value[PROPERTY_VALUE_MAX];

  SDBG("Enter");

  if (!ctrl) {
    SERR("NULL pointer detected");
    return;
  }
  e_items = &ctrl->items_for_cal;

  e_items->is_afc = 1;
  e_items->is_wbc = 1;
  e_items->is_lsc = 1;
  e_items->is_dpc = 1;
  e_items->is_ois = 1;
  e_items->is_dual = 1;

  if (property_get("persist.vendor.camera.cal.af", value, "1")) {
    e_items->is_afc = (atoi(value) == 1)? TRUE:FALSE;
  }
  if (property_get("persist.vendor.camera.cal.awb", value, "1")) {
    e_items->is_wbc = (atoi(value) == 1)? TRUE:FALSE;
  }
  if (property_get("persist.vendor.camera.cal.lsc", value, "1")) {
    e_items->is_lsc = (atoi(value) == 1)? TRUE:FALSE;
  }
  if (property_get("persist.vendor.camera.cal.dp", value, "1")) {
    e_items->is_dpc = (atoi(value) == 1)? TRUE:FALSE;
  }
  if (property_get("persist.vendor.camera.cal.dual", value, "1")) {
    e_items->is_dual = (atoi(value) == 1)? TRUE:FALSE;
  }

  SLOW("is_afc: %d,is_wbc: %d,is_lsc: %d, is_dpc: %d, is_dual: %d, is_ois: %d",
    e_items->is_afc,e_items->is_wbc,e_items->is_lsc,
    e_items->is_dpc,e_items->is_dual,e_items->is_ois);

  SDBG("Exit");
}

/** eeprom_af_add_margin
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * performs expanding the DAC range by adding margin to the boundaries
 *
 * Return:
 * TRUE : for success
 * FALSE: for failure
 **/

static boolean eeprom_af_add_margin(sensor_eeprom_data_t *e_ctrl)
{
  afcalib_data_t   *afcalib_data = NULL;
  int               dac_range = 0;

  SDBG("Enter");
  RETURN_ON_NULL(e_ctrl);

  afcalib_data = &(e_ctrl->eeprom_data.afc);
  RETURN_ON_NULL(afcalib_data);

  dac_range = afcalib_data->macro_dac - afcalib_data->infinity_dac;
  afcalib_data->infinity_dac +=
              afcalib_data->infinity_margin * (float)dac_range;
  afcalib_data->macro_dac +=
              afcalib_data->macro_margin * (float)dac_range;
  SDBG("Exit");
  return TRUE;
}
static void eeprom_do_wb_calibration(sensor_eeprom_data_t *ctrl,
  void *chromatix_sym)
{
  eeprom_calib_items_t *items;
  eeprom_calib_items_t *items_for_cal;

  SDBG("Enter");

  if (!ctrl || !chromatix_sym) {
    SERR("NULL pointer detected");
    return;
  }
  items = &ctrl->eeprom_data.items;
  items_for_cal = &ctrl->eeprom_data.items_for_cal;

  ctrl->eeprom_afchroma.chromatix.chromatix_ptr[SENSOR_CHROMATIX_3A] =
    chromatix_sym;

  if (items_for_cal->is_wbc && items->is_wbc) {
    if (ctrl->eeprom_lib.func_tbl->do_wbc_calibration != NULL) {
      ctrl->eeprom_lib.func_tbl->do_wbc_calibration(ctrl);
    } else {
      SWARN("WB calibration method not available");
    }
  } else {
    SWARN("WB calibration not enabled/not available");
  }

  eeprom_dbg_data_dump(ctrl, NULL, EEPROM_DUMP_WB);

  SDBG("Exit");
}

static void eeprom_do_wb_green_calibration(sensor_eeprom_data_t *ctrl,
  void *chromatix_sym)
{
  eeprom_calib_items_t *items;
  eeprom_calib_items_t *items_for_cal;

  SDBG("Enter");

  if (!ctrl || !chromatix_sym) {
    SERR("NULL pointer detected");
    return;
  }
  items = &ctrl->eeprom_data.items;
  items_for_cal = &ctrl->eeprom_data.items_for_cal;

  ctrl->eeprom_afchroma.chromatix.chromatix_ptr[SENSOR_CHROMATIX_ISP] =
    chromatix_sym;

  if (items_for_cal->is_wbc && items->is_wbc) {
    eeprom_whitebalance_green_calibration(ctrl);
  } else {
    SWARN("WB calibration not enabled");
  }

  SDBG("Exit");
}

static void eeprom_do_lsc_calibration(sensor_eeprom_data_t *ctrl,
  void *chromatix_sym)
{
  eeprom_calib_items_t *items;
  eeprom_calib_items_t *items_for_cal;

  SDBG("Enter");

  if (!ctrl || !chromatix_sym) {
    SERR("NULL pointer detected");
    return;
  }
  items = &ctrl->eeprom_data.items;
  items_for_cal = &ctrl->eeprom_data.items_for_cal;

  ctrl->eeprom_afchroma.chromatix.chromatix_ptr[SENSOR_CHROMATIX_ISP_COMMON] =
    chromatix_sym;

  if (items_for_cal->is_lsc && items->is_lsc) {
    if (ctrl->eeprom_lib.func_tbl->do_lsc_calibration != NULL) {
      ctrl->eeprom_lib.func_tbl->do_lsc_calibration(ctrl);
    } else {
      SWARN("LSC calibration method not available");
    }
  } else {
    SWARN("LSC calibration not enabled/not available");
  }

  SDBG("Exit");
}

/** eeprom_do_af_calibration:
 *    @ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * Kicks off the calibration process
 *
 * This function executes in module sensor context
 *
 * Return:
 * void
 **/

boolean eeprom_do_af_calibration(void *ctrl)
{
  sensor_eeprom_data_t *e_ctrl = (sensor_eeprom_data_t *)ctrl;
  eeprom_calib_items_t *e_items;
  eeprom_calib_items_t *e_items_for_cal;

  SDBG("Enter");

  RETURN_ON_NULL(e_ctrl);

  e_items = &e_ctrl->eeprom_data.items;
  e_items_for_cal = &e_ctrl->eeprom_data.items_for_cal;

  if (e_items_for_cal->is_afc && e_items->is_afc) {
    if (e_ctrl->eeprom_lib.func_tbl->do_af_calibration != NULL) {
      e_ctrl->eeprom_lib.func_tbl->do_af_calibration(ctrl);
    } else {
      SWARN("AF calibration method not available");
    }
  } else {
    SWARN("AF calibration not enabled/not available");
  }

  SDBG("Exit");
  return TRUE;
}

boolean eeprom_do_dualcam_data_calibration(void *ctrl,
  cam_related_system_calibration_data_t *dual_data)
{
  sensor_eeprom_data_t *e_ctrl;
  eeprom_calib_items_t *e_items;
  eeprom_calib_items_t *e_items_for_cal;
  dc_calib_data_t      *dc;

  SDBG("Enter");

  RETURN_ON_NULL(ctrl);
  RETURN_ON_NULL(dual_data);

  e_ctrl = (sensor_eeprom_data_t *)ctrl;
  e_items = &e_ctrl->eeprom_data.items;
  e_items_for_cal = &e_ctrl->eeprom_data.items_for_cal;

  dc = &e_ctrl->eeprom_data.dualc;

  if (e_items_for_cal->is_dual && e_items->is_dual) {
    dual_data->calibration_format_version =
      dc->system_calib_params.calibration_format_version;
    memcpy(&dual_data->main_cam_specific_calibration,
      &dc->master_calib_params, sizeof(dc_sensor_lens_calib_data_t));
    memcpy(&dual_data->aux_cam_specific_calibration,
      &dc->aux_calib_params, sizeof(dc_sensor_lens_calib_data_t));
    memcpy(&dual_data->relative_rotation_matrix,
      &dc->system_calib_params.relative_rotation_matrix,
      sizeof(float) * 9);
    memcpy(&dual_data->relative_geometric_surface_parameters,
      &dc->system_calib_params.relative_geometric_surface_parameters,
      sizeof(float) * 32);
    dual_data->relative_principle_point_x_offset =
      dc->system_calib_params.relative_principle_point_x_offset;
    dual_data->relative_principle_point_y_offset =
      dc->system_calib_params.relative_principle_point_y_offset;
    dual_data->relative_position_flag =
      dc->system_calib_params.relative_position_flag;
    dual_data->relative_baseline_distance =
      dc->system_calib_params.relative_baseline_distance;
    dual_data->aux_sensor_mirror_flip_setting =
      dc->system_calib_params.aux_sensor_mirror_flip_setting;
    dual_data->main_sensor_mirror_flip_setting =
      dc->system_calib_params.master_sensor_mirror_flip_setting;
    dual_data->module_orientation_during_calibration =
      dc->system_calib_params.module_orientation_flag;
    dual_data->rotation_flag =
      dc->system_calib_params.rotation_flag;
    dual_data->brightness_ratio =
      dc->system_calib_params.brightness_ratio;
    dual_data->ref_aux_gain =
      dc->system_calib_params.ref_aux_gain;
    dual_data->ref_aux_linecount =
      dc->system_calib_params.ref_aux_linecount;
    dual_data->ref_master_gain =
      dc->system_calib_params.ref_master_gain;
    dual_data->ref_master_linecount =
      dc->system_calib_params.ref_master_linecount;
    dual_data->ref_master_color_temperature =
      dc->system_calib_params.ref_master_color_temperature;
    dual_data->dc_otp_params =
      dc->system_calib_params.dc_otp_params;
    dual_data->dc_otp_size =
      dc->system_calib_params.dc_otp_size;
  }else {
    SWARN("Dual camera calibration not enabled/not available");
  }

  eeprom_dbg_data_dump(e_ctrl, NULL, EEPROM_DUMP_DUALC);

  SDBG("Exit");
  return TRUE;
}

/** eeprom_format_calibration_data:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * Set the formatted data from eeprom
 *
 * This function executes in module sensor context
 *
 * Return:
 * SENSOR_SUCCESS : When eeprom parameters are avilable
 * SENSOR_FAILURE : When eeprom parameters are not avilable
 **/
int32_t eeprom_format_calibration_data(void *e_ctrl)
{
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)e_ctrl;
  int32_t          ret = SENSOR_FAILURE;
  afcalib_data_t   *afcalib_data = NULL;
  int              dac_range = 0;

  SDBG("Enter");
  if (ectrl->eeprom_lib.func_tbl && ectrl->eeprom_params.is_supported) {
    eeprom_check_property(&ectrl->eeprom_data);
    /*initializing margin values to Zero and will be updated in
        corresponding eeprom driver*/
    ectrl->eeprom_data.afc.macro_margin = 0;
    ectrl->eeprom_data.afc.infinity_margin = 0;

    if(ectrl->eeprom_lib.func_tbl->format_calibration_data != NULL) {
      ectrl->eeprom_lib.func_tbl->format_calibration_data(e_ctrl);
    } else {
      SWARN("format_calibration_data method not available");
    }
    if(ectrl->eeprom_lib.func_tbl->get_calibration_items != NULL) {
      ectrl->eeprom_lib.func_tbl->get_calibration_items(e_ctrl);
    } else {
      SWARN("get_calibration_items method not available");
    }
    /* Dump the OTP data */
    eeprom_dbg_data_dump(e_ctrl, NULL, EEPROM_DUMP_OTP);
    ret = SENSOR_SUCCESS;
  }
  SDBG("Exit: ret = %d", ret);
  return ret;
}
/** eeprom_get_formatted_data:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *    @data: pointer to the formatted data
 *
 * get the formatted data from eeprom
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - NA
 **/

int32_t eeprom_get_formatted_data(void *e_ctrl, void *data) {
  sensor_eeprom_data_t *ep = (sensor_eeprom_data_t *)e_ctrl;
  format_data_t **ptr = (format_data_t **)data;

  SDBG("Enter");
  *ptr = &ep->eeprom_data;

  return SENSOR_SUCCESS;
}

/** eeprom_get_raw_data:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *    @data: pointer to the raw data
 *
 * get the raw data from eeprom
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t eeprom_get_raw_data(void *e_ctrl, void *data) {
  sensor_eeprom_data_t *ep = (sensor_eeprom_data_t *)e_ctrl;

  SDBG("Enter");
  if(ep->eeprom_lib.func_tbl->get_raw_data != NULL) {
    ep->eeprom_lib.func_tbl->get_raw_data(ep, data);
  } else {
  SWARN("get_raw_data method not available");
  }
  SDBG("Exit");
  return 0;
}

/** eeprom_get_ois_raw_data:
 *    @e_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *    @data: pointer to the raw data
 *
 * get the ois raw data from eeprom
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t eeprom_get_ois_raw_data(void *e_ctrl, void *data) {
  sensor_eeprom_data_t *ep = (sensor_eeprom_data_t *)e_ctrl;

  SDBG("Enter");
  if(ep->eeprom_lib.func_tbl->get_ois_raw_data != NULL) {
    ep->eeprom_lib.func_tbl->get_ois_raw_data(ep, data);
  } else {
    SWARN("get_ois_raw_data method not available");
  }
  SDBG("Exit");
  return 0;
}

/** eeprom_load_library:
 *    @name: eeprom device library name
 *    @data: eeprom library parameters
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t eeprom_load_library(sensor_eeprom_data_t *e_ctrl, char *path)
{
  char lib_name[BUFF_SIZE_255] = {0};
  char open_lib_str[BUFF_SIZE_255] = {0};
  void *(*eeprom_open_lib)(void) = NULL;
  char *abs_path = MOD_SENSOR_LIB_PATH;
  const char *name = (char *)e_ctrl->eeprom_params.eeprom_name;
  char retry = FALSE;

  SDBG("enter");

  if (path) {
    snprintf(lib_name, sizeof(lib_name), "%slibmmcamera_%s_eeprom.so",
      path, name);
  } else if (abs_path){
    retry = TRUE;
    snprintf(lib_name, sizeof(lib_name), "%s/libmmcamera_%s_eeprom.so",
      abs_path, name);
  } else {
    snprintf(lib_name, sizeof(lib_name), "libmmcamera_%s_eeprom.so",
      name);
  }

  SHIGH("lib_name %s",lib_name);
  e_ctrl->eeprom_lib.eeprom_lib_handle = dlopen(lib_name, RTLD_NOW);
  if (!e_ctrl->eeprom_lib.eeprom_lib_handle && retry) {
    snprintf(lib_name, sizeof(lib_name), "libmmcamera_%s_eeprom.so",
      name);
    e_ctrl->eeprom_lib.eeprom_lib_handle = dlopen(lib_name, RTLD_NOW);
  }
  if (!e_ctrl->eeprom_lib.eeprom_lib_handle) {
    SERR("dlopen failed");
    return -EINVAL;
  }

  snprintf(open_lib_str, sizeof(open_lib_str), "%s_eeprom_open_lib", name);
  *(void **)&eeprom_open_lib  = dlsym(e_ctrl->eeprom_lib.eeprom_lib_handle,
    open_lib_str);
  if (!eeprom_open_lib) {
    SERR("dlsym failed");
    return -EINVAL;
  }

  e_ctrl->eeprom_lib.func_tbl = (eeprom_lib_func_t *)eeprom_open_lib();
  if (!e_ctrl->eeprom_lib.func_tbl) {
    SERR("failed : invalid func_tbl");
    return -EINVAL;
  }

  SLOW("e_ctrl->eeprom_lib.func_tbl =%p",e_ctrl->eeprom_lib.func_tbl);

  SDBG("exit");
  return 0;
}

/** eeprom_unload_library:
 *    @eeprom_lib_params: eeprom library parameters
 *
 * unload eeprom driver library and free the allocated memory
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/
int32_t eeprom_unload_library(sensor_eeprom_data_t *e_ctrl)
{
  eeprom_lib_params_t *eeprom_lib_params;
  eeprom_params_t     *eeprom_params;

  SDBG("enter");

  RETURN_ERR_ON_NULL(e_ctrl, SENSOR_FAILURE);

  eeprom_lib_params = &e_ctrl->eeprom_lib;
  eeprom_params = &e_ctrl->eeprom_params;

  if (eeprom_lib_params->eeprom_lib_handle) {
    dlclose(eeprom_lib_params->eeprom_lib_handle);
    eeprom_lib_params->eeprom_lib_handle = NULL;
  }

  if (eeprom_params->is_supported && eeprom_params->buffer)
    free(eeprom_params->buffer);

  SDBG("exit");

  return SENSOR_SUCCESS;
}

/** eeprom_get_info:
 *    @ptr: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * 1) Gets the details about the number bytes read in kernel
 * 2) Reads the actual data from the kerenl.
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t eeprom_get_info(void *ptr)
{
  int32_t rc = 0;

  sensor_eeprom_data_t *ep = (sensor_eeprom_data_t *)ptr;
  struct msm_eeprom_cfg_data cfg;

  SDBG("enter");

  RETURN_ERR_ON_NULL(ep, -EINVAL);

  ep->eeprom_params.is_supported = 0;
  cfg.cfgtype = CFG_EEPROM_GET_CAL_DATA;
  rc = ioctl(ep->fd, VIDIOC_MSM_EEPROM_CFG, &cfg);
  if (rc < 0) {
    SERR("VIDIOC_MSM_EEPROM_CFG(%d) failed!", ep->fd);
    goto ERROR;
  }

  ep->eeprom_params.num_bytes = cfg.cfg.get_data.num_bytes;
  if (ep->eeprom_params.num_bytes) {
    ep->eeprom_params.buffer = (uint8_t *)malloc(ep->eeprom_params.num_bytes);
    ep->eeprom_params.is_supported = 1;
    if (!ep->eeprom_params.buffer) {
      SERR("%s failed allocating memory\n",__func__);
      rc = -ENOMEM;
      goto ERROR;
    }

    cfg.cfgtype = CFG_EEPROM_READ_CAL_DATA;
    cfg.cfg.read_data.num_bytes = ep->eeprom_params.num_bytes;
    cfg.cfg.read_data.dbuffer = ep->eeprom_params.buffer;
    rc = ioctl(ep->fd, VIDIOC_MSM_EEPROM_CFG, &cfg);
    if (rc < 0) {
      SERR("CFG_EEPROM_READ_CAL_DATA(%d) failed!", ep->fd);
      goto ERROR;
    }
    eeprom_dbg_data_dump(&ep->eeprom_params, NULL, EEPROM_DUMP_KBUF);
    SLOW("kernel returned read buffer =%p\n", cfg.cfg.read_data.dbuffer);
  }

  SDBG("Exit");
  return 0;

ERROR:
  ep->eeprom_params.is_supported = 0;
  if (ep->eeprom_params.buffer)
    free(ep->eeprom_params.buffer);
  return rc;
}

/** eeprom_open:
 *    @eeprom_ctrl: address of pointer to
 *                   sensor_eeprom_data_t struct
 *    @subdev_name: EEPROM subdev name
 *
 * 1) Allocates memory for EEPROM control structure
 * 2) Opens EEPROM subdev node
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/
static int32_t eeprom_open(void **eeprom_ctrl, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_eeprom_data_t *ctrl;
  char subdev_string[32];
  sensor_submodule_info_t *info =
      (sensor_submodule_info_t *)data;

  SDBG("Enter");
  if (!eeprom_ctrl || !info) {
    SERR("failed eeprom_ctrl = 0x%p info = 0x%p", eeprom_ctrl, info);
    return SENSOR_ERROR_INVAL;
  }

  if (!strlen(info->intf_info[SUBDEV_INTF_PRIMARY].sensor_sd_name)) {
    SERR("failed: sensor_sd_name is 0 length");
    return SENSOR_ERROR_INVAL;
  }

  ctrl = (sensor_eeprom_data_t *)(*eeprom_ctrl);
  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s",
      info->intf_info[SUBDEV_INTF_PRIMARY].sensor_sd_name);
  SLOW("sd name %s", subdev_string);
  /* Open subdev */
  ctrl->fd = open(subdev_string, O_RDWR);
  if (ctrl->fd < 0) {
    SERR("failed");
    rc = SENSOR_FAILURE;
  }
  SDBG("Exit");
  return rc;
}

/** eeprom_init:
 *    @ptr: address of pointer to
 *                   sensor_eeprom_data_t struct
 *
 * 1) Parse the eeprom library and passes the
 *    structure to the kernel
 *
 * This function executes in module sensor context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t eeprom_init(void *ptr)
{
  int32_t rc = SENSOR_SUCCESS;
  struct msm_eeprom_memory_map_array eeprom_map_array;
  struct msm_eeprom_info_t eeprom_info;
  sensor_eeprom_data_t *ectrl = (sensor_eeprom_data_t *)ptr;
  struct msm_eeprom_cfg_data cfg;
  struct msm_sensor_power_setting_array *power_setting_array = NULL;

  RETURN_ON_NULL(ectrl);

  if (ectrl->eeprom_lib.func_tbl == NULL) {
    SERR("Eeprom func_tbl NULL here\n");
    return SENSOR_FAILURE;
  }
  power_setting_array = (struct msm_sensor_power_setting_array *)malloc(
    sizeof(struct msm_sensor_power_setting_array));
  RETURN_ERROR_ON_NULL(power_setting_array);
  if ((ectrl->eeprom_lib.func_tbl->eeprom_info.power_setting_array.size == 0) ||
    ectrl->eeprom_lib.func_tbl->eeprom_info.mem_map_array.size_map_array == 0){
    SERR("Eeprom Driver has either Power setting size or mem map size as 0");
    rc = SENSOR_FAILURE;
    goto CLEANUP;
  }
  translate_camera_power_setting(power_setting_array,
    &(ectrl->eeprom_lib.func_tbl->eeprom_info.power_setting_array));

  translate_eeprom_memory_map(&eeprom_map_array,
    &(ectrl->eeprom_lib.func_tbl->eeprom_info.mem_map_array));

  eeprom_info.i2c_freq_mode =
    sensor_sdk_util_get_i2c_freq_mode(
    ectrl->eeprom_lib.func_tbl->eeprom_info.i2c_freq_mode);

  eeprom_info.power_setting_array = power_setting_array;
  eeprom_info.mem_map_array = &eeprom_map_array;

  cfg.cfgtype = CFG_EEPROM_INIT;
  cfg.cfg.eeprom_info = eeprom_info;
  rc = ioctl(ectrl->fd, VIDIOC_MSM_EEPROM_CFG, &cfg);
  if (rc < 0) {
    SERR("EEPROM Init ioctl failed");
  }

CLEANUP:
  free(power_setting_array);
  return rc;
}


/** eeprom_process:
 *    @eeprom_ctrl: EEEPROM control handle
 *    @event: configuration event type
 *    @data: NULL
 *
 * Handle all EERPOM events
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t eeprom_process(void *eeprom_ctrl,
  sensor_submodule_event_type_t event, void *data)
{
  int32_t rc = SENSOR_SUCCESS;

  RETURN_ERR_ON_NULL(eeprom_ctrl,SENSOR_FAILURE);

  sensor_eeprom_data_t *e_ctrl = (sensor_eeprom_data_t *)eeprom_ctrl;
  switch(event) {
  case EEPROM_READ_DATA:
      rc = eeprom_get_info(e_ctrl);
      break;
  case EEPROM_SET_FORMAT_DATA:
      rc = eeprom_format_calibration_data(e_ctrl);
      eeprom_af_add_margin(e_ctrl);
      break;
  case EEPROM_CALIBRATE_WB:
    eeprom_do_wb_calibration(e_ctrl, data);
    break;
  case EEPROM_CALIBRATE_WB_GREEN:
    eeprom_do_wb_green_calibration(e_ctrl, data);
    break;
  case EEPROM_CALIBRATE_LSC:
    eeprom_do_lsc_calibration(e_ctrl, data);
    break;
  case EEPROM_CALIBRATE_FOCUS_DATA:
      e_ctrl->eeprom_afchroma = *((eeprom_set_chroma_af_t *)data);
      eeprom_do_af_calibration(e_ctrl);
      break;
  case EEPROM_GET_ISINSENSOR_CALIB: {
      int32_t *is_insensor = (int32_t *)data;
      if (e_ctrl->eeprom_lib.func_tbl &&
          e_ctrl->eeprom_params.is_supported) {
        if (e_ctrl->eeprom_lib.func_tbl->get_calibration_items != NULL)
          e_ctrl->eeprom_lib.func_tbl->get_calibration_items(e_ctrl);
        *is_insensor = e_ctrl->eeprom_data.items.is_insensor;
      }
  }
    break;
  case EEPROM_GET_ISOIS_CALIB:{
      int32_t *is_ois = (int32_t *)data;
      if (e_ctrl->eeprom_lib.func_tbl &&
          e_ctrl->eeprom_params.is_supported) {
        if (e_ctrl->eeprom_lib.func_tbl->get_calibration_items != NULL)
          e_ctrl->eeprom_lib.func_tbl->get_calibration_items(e_ctrl);
        *is_ois = e_ctrl->eeprom_data.items.is_ois;
      }
  }
    break;
  case EEPROM_GET_FORMATTED_DATA:
    rc = eeprom_get_formatted_data(e_ctrl, data);
    break;
  case EEPROM_GET_RAW_DATA:
    rc = eeprom_get_raw_data(e_ctrl, data);
    break;
  case EEPROM_GET_OIS_RAW_DATA:
    rc = eeprom_get_ois_raw_data(e_ctrl, data);
    break;
  case EEPROM_GET_WB_GRGB:
    if (e_ctrl->eeprom_data.wbc.gr_over_gb < 1)
      *(float*)data = 1 / e_ctrl->eeprom_data.wbc.gr_over_gb;
    else
      *(float*)data = e_ctrl->eeprom_data.wbc.gr_over_gb;
    break;
  case EEPROM_GET_WB_CAL:
    if (!e_ctrl->eeprom_data.items.is_insensor)
      *(wbcalib_data_t**)data = &e_ctrl->eeprom_wbc_factor;
    break;
case EEPROM_GET_ISDPC_CALIB:{
    int32_t *is_dpc = (int32_t *)data;
    if (e_ctrl->eeprom_lib.func_tbl) {
      if (e_ctrl->eeprom_lib.func_tbl->get_calibration_items != NULL){
        e_ctrl->eeprom_lib.func_tbl->get_calibration_items(e_ctrl);
        *is_dpc = e_ctrl->eeprom_data.items.is_dpc;
      }
    }
    break;
  }
  case EEPROM_SET_CALIBRATE_DUALCAM_PARAM: {
    cam_related_system_calibration_data_t *dual_data =
      (cam_related_system_calibration_data_t *)data;

    rc = eeprom_do_dualcam_data_calibration(e_ctrl, dual_data);
    break;
  }
  case EEPROM_DUMP_CALIB_DATA:
  {
    sensor_chromatix_params_t* chromatix_params;
    chromatix_params = (sensor_chromatix_params_t*) data;
    eeprom_dbg_data_dump(e_ctrl, chromatix_params, EEPROM_DUMP_CALIB);
  }
  break;
  case EEPROM_INIT:
    rc = eeprom_init(e_ctrl);
    if (rc < 0) {
      SERR("eeprom_init failed");
    }
   break;
  default:
      SHIGH("Invalid event : %d",event);
      rc = SENSOR_FAILURE;
      break;
  }
  return rc;
}

/** eeprom_close:
 *    @eeprom_ctrl: ERPOM control handle
 *
 * 2) Close fd
 * 3) Free EEPROM control structure
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

static int32_t eeprom_close(void *eeprom_ctrl)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_eeprom_data_t *ctrl = (sensor_eeprom_data_t *)eeprom_ctrl;

  SDBG("Enter");

  /* Validate input parameters */
  if (!ctrl) {
    SERR("failed: ctrl %p", ctrl);
    return -EINVAL;
  }

  /* close subdev */
  close(ctrl->fd);
  SDBG("Exit");
  return rc;
}

/** eeprom_sub_module_init:
 *    @func_tbl: pointer to sensor function table
 *
 * Initialize function table for EEPROM device to be used
 *
 * This function executes in sensor module context
 *
 * Return:
 * Success - SENSOR_SUCCESS
 * Failure - SENSOR_FAILURE
 **/

int32_t eeprom_sub_module_init(sensor_func_tbl_t *func_tbl)
{

  RETURN_ERR_ON_NULL(func_tbl,SENSOR_FAILURE);

  func_tbl->open = eeprom_open;
  func_tbl->process = eeprom_process;
  func_tbl->close = eeprom_close;
  return SENSOR_SUCCESS;
}
