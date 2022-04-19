/*===========================================================================
 * Copyright (c) 2012-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*==========================================================================*/

#include <stdio.h>
#include <dlfcn.h>
#include <math.h>
#include <asm-generic/errno-base.h>
#include <poll.h>
#include "sensor.h"
#include "sensor_util.h"
#include "modules.h"
#include "mct_pipeline.h"
#include "camera_dbg.h"
#include "PdafLibrary.h"
//#include "sensor_thread.h"
#ifdef __ANDROID__
  #include <cutils/properties.h>
#endif

#define QFACTOR 100
#define BUFF_SIZE_255 255

#define MAX_VFE_CLK 320000000

#define NANO_SEC_TO_MICRO_SEC 1000
#define MULTIPLY_FACTOR_FOR_MIN_FPS 4
#define MULTIPLY_FACTOR_FOR_ISO     100
#define MAX_FPS_VARIANCE 1.0f
#define SENSOR_INVALID_RESOLUTION -1
#define SENSOR_GET_VT_PIK_CLK(out_info) \
    (uint32_t)((out_info)->line_length_pclk * (out_info)->frame_length_lines \
    * (out_info)->max_fps)

//#undef ENABLE_DIS_MARGIN
//#define ENABLE_DIS_MARGIN
//#define ENABLE_MANUAL_EXPOSURE_UPDATE
/*===========================================================================
* FUNCTION - LOG_IOCTL -
*
* DESCRIPTION: Wrapper for logging and to trace ioctl calls.
*==========================================================================*/
int LOG_IOCTL(int d, int request, void* par1, char* trace_func)
{
  int ret;
  ATRACE_BEGIN_SNPRINTF(35,"Camera:sensorIoctl %s", trace_func);
  ret = ioctl(d, request, par1);
  ATRACE_END();
  return ret;
}

/*===========================================================================
 * FUNCTION    - sensor_get_meta_out_dim -
 *
 * DESCRIPTION: Returns the dimension of meta data
 *==========================================================================*/
static int32_t sensor_get_meta_out_dim(void *sctrl, void *data)
{
  sensor_ctrl_t    *ctrl;
  sensor_lib_t     *sensor_lib;
  cam_dimension_t  *meta_raw_dim;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  ctrl = (sensor_ctrl_t *)sctrl;
  meta_raw_dim = (cam_dimension_t *)data;

  meta_raw_dim->width = ctrl->s_data->addl_sensor_stats.width;
  meta_raw_dim->height = ctrl->s_data->addl_sensor_stats.height;
  SLOW("width: %d height: %d",
    ctrl->s_data->addl_sensor_stats.width,
    ctrl->s_data->addl_sensor_stats.height);

  return SENSOR_SUCCESS;
}

/*==========================================================================
 * FUNCTION    - sensor_get_cur_fps -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_get_cur_fps(void *sctrl, void *fps_data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  uint32_t *fps = (uint32_t *) fps_data;
  struct sensor_lib_out_info_t *out_info = NULL;
  int16_t res = ctrl->s_data->cur_res;
  uint32_t frame_length_lines = 0;
  double cur_fps = 0.0;

  SLOW("enter");
  if (!fps_data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  if (res >= lib->sensor_lib_ptr->out_info_array.size) {
    SERR("error - res %d >= max res size %d",
      res, lib->sensor_lib_ptr->out_info_array.size);
    return -EINVAL;
  }

  out_info = &lib->sensor_lib_ptr->out_info_array.out_info[res];

  frame_length_lines = out_info->frame_length_lines;
  cur_fps = out_info->max_fps;

  if (ctrl->s_data->current_linecount > frame_length_lines)
    cur_fps = (cur_fps * (double)frame_length_lines) /
        (double)ctrl->s_data->current_linecount;
  *fps = (uint32_t)(cur_fps * Q8);

  ctrl->s_data->realtime_fps = cur_fps;
  SLOW("sensor: %s real-time fps: %f",
    lib->sensor_lib_ptr->sensor_slave_info.sensor_name, cur_fps);

  return SENSOR_SUCCESS;
}


/*===========================================================================
 * FUNCTION    - sensor_set_meta_dim -
 *
 * DESCRIPTION: Sets the meta dimension to sensor control
 *==========================================================================*/

static int32_t sensor_set_meta_dim(void *sctrl, void *data)
{
  sensor_ctrl_t        *ctrl;
  sensor_lib_t         *sensor_lib;
  sensor_meta_info_t   *meta_info;
  sensor_stream_info_t *stream_info;
  int32_t             i = 0;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  ctrl = (sensor_ctrl_t *)sctrl;
  meta_info = (sensor_meta_info_t *)data;
  sensor_lib = ctrl->lib_params->sensor_lib_ptr;
  stream_info = &sensor_lib->sensor_stream_info_array.sensor_stream_info[0];

  for (i = 0; i < stream_info->vc_cfg_size; i++)
  {
    if ((meta_info->dt == stream_info->vc_cfg[i].dt) &&
      (meta_info->vc == stream_info->vc_cfg[i].cid / 4)) {

      ctrl->s_data->addl_sensor_stats.width =
        stream_info->vc_cfg[i].meta_dim.width;
      ctrl->s_data->addl_sensor_stats.height =
        stream_info->vc_cfg[i].meta_dim.height;
      ctrl->s_data->addl_sensor_stats.stats_type =
        stream_info->vc_cfg[i].stats_type;
      ctrl->s_data->addl_sensor_stats.fmt = meta_info->fmt;
      SLOW("meta_width = meta_height = %d",
        ctrl->s_data->addl_sensor_stats.width,
        ctrl->s_data->addl_sensor_stats.height);
      break;
    }
  }

  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_disable_pdaf -
 *
 * DESCRIPTION: To disable pdaf if pdaf needs customer solution but cusotmer
 * lib does not opened successfully
 *==========================================================================*/
static void sensor_disable_pdaf(sensor_lib_params_t *sensor_lib_params)
{
  struct sensor_lib_out_info_array     *out_info_array = NULL;
  unsigned short                       size, csid_size, i, j;
  struct sensor_csid_lut_params_array  *csid_lut_params_array = NULL;
  pdaf_lib_t                           *params = NULL;

  params =&sensor_lib_params->sensor_lib_ptr->pdaf_config;
  csid_lut_params_array =
   &sensor_lib_params->sensor_lib_ptr->csid_lut_params_array;
  out_info_array =
   &sensor_lib_params->sensor_lib_ptr->out_info_array;
  size = out_info_array->size;
  for (i = 0; i < size; i++){
   if (out_info_array->out_info[i].is_pdaf_supported){
    out_info_array->out_info[i].is_pdaf_supported = 0;
    csid_size = csid_lut_params_array->lut_params[i].num_cid;
    for ( j =0; j < csid_size; j++)
    {
     if (csid_lut_params_array->lut_params[i].vc_cfg_a[j].dt == params->stats_dt)
     {
       csid_lut_params_array->lut_params[i].vc_cfg_a[j].dt = 0;
       csid_lut_params_array->lut_params[i].num_cid --;
     }
    }
   }
  }
}

/*===========================================================================
 * FUNCTION    - sensor_load_custom_API -
 *
 * DESCRIPTION: To load custom functions in sensor drivers, e.g. for PDAF
 *==========================================================================*/
void sensor_load_custom_API(sensor_lib_params_t *sensor_lib_params)
{
  sensor_custom_API_t        *API_ptr = &sensor_lib_params->sensor_custom_API;
  sensorlib_pdaf_apis_t      *API_name =
    &sensor_lib_params->sensor_lib_ptr->sensorlib_pdaf_api;

  API_ptr->pdlib_get_defocus =
    dlsym(sensor_lib_params->sensor_lib_handle,
    API_name->pdaf_get_defocus_API);
  API_ptr->pdlib_init =
    dlsym(sensor_lib_params->sensor_lib_handle,
    API_name->pdaf_init_API);
  API_ptr->pdlib_deinit =
    dlsym(sensor_lib_params->sensor_lib_handle,
    API_name->pdaf_deinit_API);
  API_ptr->sensor_custom_calc_defocus =
    dlsym(sensor_lib_params->sensor_lib_handle,
    sensor_lib_params->sensor_lib_ptr->sensorlib_pdaf_api.calcdefocus);

  return;
}

/*===========================================================================
 * FUNCTION    - sensor_load_custom_library -
 *
 * DESCRIPTION: To load functions, e.g. for PDAF,  from an external library
 *==========================================================================*/
void sensor_load_custom_library(sensor_lib_params_t *sensor_lib_params)
{
  sensor_lib_version_t libver = {0, 0};
  sensor_custom_API_t        *API_ptr = &sensor_lib_params->sensor_custom_API;
  sensorlib_pdaf_apis_t      *API_name =
    &sensor_lib_params->sensor_lib_ptr->sensorlib_pdaf_api;

  if(strlen(API_name->libname) == 0) {
     API_ptr->handle = NULL;
     SLOW("no customer pdaf lib name definition");
     return;
  }

  API_ptr->handle = dlopen(API_name->libname, RTLD_NOW);
  if(!API_ptr->handle) {
    SERR("failed to open custom lib %s", API_name->libname);
    sensor_disable_pdaf(sensor_lib_params);
    return;
  }
  SHIGH("sensor custom lib %s loading successful", API_name->libname);

  if(!API_ptr->pdlib_get_defocus)
    API_ptr->pdlib_get_defocus =
      dlsym(sensor_lib_params->sensor_custom_API.handle,
      API_name->pdaf_get_defocus_API);
  if(!API_ptr->pdlib_init)
    API_ptr->pdlib_init =
      dlsym(sensor_lib_params->sensor_custom_API.handle,
      API_name->pdaf_init_API);
  if(!API_ptr->pdlib_deinit)
    API_ptr->pdlib_deinit =
      dlsym(sensor_lib_params->sensor_custom_API.handle,
      API_name->pdaf_deinit_API);

  API_ptr->pdlib_get_version =
    dlsym(sensor_lib_params->sensor_custom_API.handle,
    API_name->pdaf_get_version_API);
  if (API_ptr->pdlib_get_version) {
    API_ptr->pdlib_get_version(&libver);
    SHIGH("%s major_version = %d, minor_version = %d",
      API_name->libname, (int)libver.major_ver,(int)libver.minor_ver);
  } else
    SHIGH("pdaf custom lib version API unavailable");

  return;
}

/*===========================================================================
 * FUNCTION    - sensor_load_binary -
 *
 * DESCRIPTION:
 *==========================================================================*/
boolean sensor_load_binary(const char *name, void *data, char *path)
{
  int32_t              bytes_read = 0;
  FILE                *fp = NULL;
  char                 bin_name[BUFF_SIZE_255] = {0};
  sensor_lib_params_t *sensor_lib_params = (sensor_lib_params_t *)data;

  if (path) {
    snprintf(bin_name, BUFF_SIZE_255, "%s%s_data.bin", path, name);
  } else {
    snprintf(bin_name, BUFF_SIZE_255, "%s/%s_data.bin", CONFIG_XML_PATH, name);
  }

  fp = fopen(bin_name, "rb");
  RETURN_ON_NULL(fp);

  sensor_lib_params->sensor_lib_ptr =
    (sensor_lib_t *)malloc(sizeof(sensor_lib_t));
  JUMP_ON_NULL(sensor_lib_params->sensor_lib_ptr, load_binary_fail);

  bytes_read = fread(sensor_lib_params->sensor_lib_ptr, 1, sizeof(sensor_lib_t),
    fp);
  if (bytes_read != sizeof(sensor_lib_t)) {
    SERR("Invalid number of bytes read. bytes_read = %d expected = %d",
      bytes_read, sizeof(sensor_lib_t));
    goto load_binary_fail;
  }
  fclose(fp);

  SHIGH("binary file loading successful: %s", bin_name);
  return TRUE;

load_binary_fail:
  SERR("binary file loading fail: %s", bin_name);
  free(sensor_lib_params->sensor_lib_ptr);
  fclose(fp);
  return FALSE;

}

/*===========================================================================
 * FUNCTION    - sensor_load_library -
 *
 * DESCRIPTION:
 *==========================================================================*/
int32_t sensor_load_library(const char *name, void *data, char *path)
{
  char lib_name[BUFF_SIZE_255] = {0};
  char open_lib_str[BUFF_SIZE_255] = "sensor_open_lib";
  char retry = FALSE;
  char *abs_path = MOD_SENSOR_LIB_PATH;
  void *(*sensor_open_lib)(void) = NULL;
  sensor_lib_params_t *sensor_lib_params = (sensor_lib_params_t *)data;

  SLOW("enter path = %p", path);

  if (path) {
    snprintf(lib_name, BUFF_SIZE_255, "%slibmmcamera_%s.so",
      path, name);
  } else if (abs_path){
    retry = TRUE;
    snprintf(lib_name, BUFF_SIZE_255, "%s/libmmcamera_%s.so",
      abs_path, name);
  } else {
    snprintf(lib_name, BUFF_SIZE_255, "libmmcamera_%s.so", name);
  }

  SLOW("lib_name %s", lib_name);
  sensor_lib_params->sensor_lib_handle = dlopen(lib_name, RTLD_NOW);
  if (!sensor_lib_params->sensor_lib_handle && retry) {
    snprintf(lib_name, BUFF_SIZE_255, "libmmcamera_%s.so", name);
    sensor_lib_params->sensor_lib_handle = dlopen(lib_name, RTLD_NOW);
  }

  if (!sensor_lib_params->sensor_lib_handle) {
    SERR("dlopen of %s failed NULL handle", lib_name);
    return -EINVAL;
  }

  *(void **)&sensor_open_lib = dlsym(sensor_lib_params->sensor_lib_handle,
    open_lib_str);
  if (!sensor_open_lib) {
    SERR("failed");
    return -EINVAL;
  }
  sensor_lib_params->sensor_lib_ptr = (sensor_lib_t *)sensor_open_lib();
  if (!sensor_lib_params->sensor_lib_ptr) {
    SERR("failed");
    return -EINVAL;
  }

  sensor_load_custom_API(sensor_lib_params);
  sensor_load_custom_library(sensor_lib_params);

  SLOW("exit");
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_unload_library -
 *
 * DESCRIPTION:
 *==========================================================================*/
int32_t sensor_unload_library(sensor_lib_params_t *sensor_lib_params)
{
  SLOW("Enter");
  if (!sensor_lib_params) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  if (sensor_lib_params->sensor_lib_handle) {
    if (sensor_lib_params->sensor_custom_API.handle) {
      dlclose(sensor_lib_params->sensor_custom_API.handle);
      sensor_lib_params->sensor_custom_API.handle = NULL;
    }
    dlclose(sensor_lib_params->sensor_lib_handle);
    sensor_lib_params->sensor_lib_handle = NULL;
    sensor_lib_params->sensor_lib_ptr = NULL;
  }
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_add_delay -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_add_delay(void *sctrl, sensor_exposure_info_t *exp_info)
{
  int32_t rc;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;

  if (ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT) &&
    (ctrl->s_data->ae_bracket_info.ae_bracket_config.mode ==
    CAM_EXP_BRACKETING_ON)) {
    ctrl->s_data->delay_en = 1;
  }

  if ((ctrl->s_data->isp_frame_skip <
    lib->sensor_lib_ptr->sensor_max_pipeline_frame_delay) &&
      ctrl->s_data->delay_en) {

    rc = sensor_delay_in(sctrl, exp_info);
    if (rc < 0) {
      SERR("Sensor delay in: failed");
      return SENSOR_FAILURE;
    }
    rc = sensor_delay_out(sctrl, exp_info);
    if (rc < 0) {
      SERR("Sensor delay out: failed");
      return SENSOR_FAILURE;
    }
  }

  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - updateExtGain -
 *
 * DESCRIPTION: Update the value of gain based on set prop value
 *              adb shell setprop persist.vendor.camera.gain <gain>
 *              to reset set <gain> = 0
 *
 * RETURN: SENSOR_SUCCESS - successfully update to external value
 *         SENSOR_FAILURE - do not update to external value
 *==========================================================*/
static int32_t updateExtGain(float *real_gain)
{
  char ext_real_gain[PROPERTY_VALUE_MAX];

  RETURN_ERROR_ON_NULL(real_gain);

  property_get("persist.vendor.camera.sensor.gain", ext_real_gain, "0");
  *real_gain = atoi(ext_real_gain);
  if (*real_gain > 0) {
    SHIGH("Updated gain: %f", *real_gain);
    return SENSOR_SUCCESS;
  }

    return SENSOR_FAILURE;
}

/*==========================================================
 * FUNCTION    - updateExtLinecount -
 *
 * DESCRIPTION: Update the value of lc based on set prop value
 *              adb shell setprop persist.vendor.camera.linecount <linecount>
 *              to reset set <linecount> = 0
 *
 * RETURN: SENSOR_SUCCESS - successfully update to external value
 *         SENSOR_FAILURE - do not update to external value
 *==========================================================*/
static int32_t updateExtLinecount(uint32_t *linecount)
{
  char ext_linecount[PROPERTY_VALUE_MAX];

  RETURN_ERROR_ON_NULL(linecount);

  property_get("persist.vendor.camera.sensor.linecount", ext_linecount, "0");
  *linecount = atoi(ext_linecount);
  if (*linecount > 0) {
    SHIGH("Updated linecount: %d", *linecount);
    return SENSOR_SUCCESS;
  }

    return SENSOR_FAILURE;
}

/*===========================================================================
 * FUNCTION    - sensor_set_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_exposure(void *sctrl, sensor_exp_t exposure)
{
  int32_t                rc = SENSOR_SUCCESS;
  sensor_ctrl_t          *ctrl;
  sensor_lib_params_t    *lib;
  sensor_exposure_info_t exp_info;
  float                  ext_real_gain = 0;
  uint32_t               ext_linecount = 0;
  float                  real_gain = 0;
  uint32_t               linecount = 0;

  RETURN_ERROR_ON_NULL(sctrl);

  memset(&exp_info, 0 , sizeof(sensor_exposure_info_t));
  ctrl = (sensor_ctrl_t *)sctrl;
  lib = (sensor_lib_params_t *)ctrl->lib_params;
  real_gain = exposure.real_gain;
  linecount = exposure.linecount;

#ifdef ENABLE_MANUAL_EXPOSURE_UPDATE
  if(updateExtGain(&ext_real_gain) == SENSOR_SUCCESS)
    real_gain = ext_real_gain;

  if(updateExtLinecount(&ext_linecount) == SENSOR_SUCCESS)
    linecount = ext_linecount;
#endif

  SLOW("real_gain=%f line_count=%d", real_gain, linecount);

  if (real_gain > 0 && linecount > 0) {
  if (lib->sensor_lib_ptr->exposure_func_table.calc_exp_array_type ==
    CALC_CUSTOM_IN_LIB &&
    lib->sensor_lib_ptr->exposure_func_table.sensor_calculate_exposure) {
    if(ctrl->s_data->hdr_mode == CAM_SENSOR_3EXP_HDR_IN_SENSOR) {
      lib->sensor_lib_ptr->exposure_func_table.sensor_calculate_3exp_HDR_exposure(
         real_gain, linecount, &exp_info, exposure.s_real_gain, exposure.m_real_gain);
    } else {
      lib->sensor_lib_ptr->exposure_func_table.sensor_calculate_exposure(
         real_gain, linecount, &exp_info, exposure.s_real_gain);
    }
  } else {
    sensor_cmn_calc_exposure(lib->sensor_lib_ptr,
      real_gain, linecount, &exp_info, exposure.s_real_gain);
  }

    rc = sensor_add_delay(sctrl, &exp_info);
    if (rc != SENSOR_SUCCESS)
      SERR("Sensor param delay: failed");

    ctrl->s_data->gain_from_aec = exposure.real_gain;
    ctrl->s_data->current_gain = exp_info.reg_gain;
    ctrl->s_data->sensor_real_gain = exp_info.sensor_real_gain;
    ctrl->s_data->sensor_digital_gain = exp_info.sensor_digital_gain;
    ctrl->s_data->current_linecount = exp_info.line_count;
    ctrl->s_data->digital_gain = exp_info.digital_gain;
    if(ctrl->s_data->hdr_mode == CAM_SENSOR_3EXP_HDR_IN_SENSOR) {
      ctrl->s_data->digital_gain *= ctrl->s_data->ev_gain;
      SHIGH("digital gain(after ev gain):%f", ctrl->s_data->digital_gain);

      // Copy the Medium exposure values
      ctrl->s_data->m_reg_gain = exp_info.m_reg_gain;
      ctrl->s_data->m_linecount = exposure.m_linecount;

    }
    ctrl->s_data->current_luma_hdr = exposure.luma_hdr;
    ctrl->s_data->current_fgain_hdr = exposure.fgain_hdr;
    ctrl->s_data->s_reg_gain = exp_info.s_reg_gain;
    ctrl->s_data->s_linecount = exposure.s_linecount;
    ctrl->s_data->sensor_real_digital_gain = exp_info.sensor_real_dig_gain;

    ctrl->s_data->s_dig_reg_gain = exp_info.s_dig_reg_gain;
    ctrl->s_data->m_dig_reg_gain = exp_info.m_dig_reg_gain;

    float isp_gain;
    char value[PROPERTY_VALUE_MAX];
    property_get("persist.vendor.camera.HDR.fix_isp_gain", value, "0.0");
    isp_gain = atof(value);
    if(isp_gain != 0.0f) {
      ctrl->s_data->digital_gain = isp_gain;
    }

    SHIGH("gain from aec:%f, sensor analog gain:%f, sensor digital gain:%f,"
      "ISP digital gain:%f, linecount:%d",
      exposure.real_gain, exp_info.sensor_real_gain,
      exp_info.sensor_real_dig_gain, exp_info.digital_gain,
      exp_info.line_count);

    if(ctrl->s_data->hdr_mode != CAM_SENSOR_HDR_OFF ) {
      if(ctrl->s_data->hdr_mode == CAM_SENSOR_3EXP_HDR_IN_SENSOR) {
        SLOW("HDR: Mode:%d,long gain:%f,middle gain:%f,short gain:%f,"
             "long linecount:%d,middle linecount:%d,short linecount:%d",ctrl->s_data->hdr_mode,
             exposure.real_gain,exposure.m_real_gain,exposure.s_real_gain,exposure.linecount,
             exposure.m_linecount,exposure.s_linecount);
      } else {
        SLOW("HDR: Mode:%d,long gain:%f,middle gain:%f,short gain:%f,"
            "long linecount:%d,middle linecount:%d,short linecount:%d",ctrl->s_data->hdr_mode,
             exposure.real_gain,exposure.s_real_gain,exposure.linecount,
             exposure.s_linecount);
      }
    }
  } else {
    SERR("Invalid input parameters: real_gain=%f line_count=%d",
      real_gain, linecount);
  }
  return SENSOR_SUCCESS;
}


/*===========================================================================
 * FUNCTION    - sensor_get_effect_index -
 *
 * DESCRIPTION: Get index of the special effect
 *==========================================================================*/
inline static int32_t sensor_get_effect_index(
  struct sensor_effect_info *sensor_effect_info,
  sensor_effect_t effect_mode)
{
  int i = 0;

  if(sensor_effect_info->size >= MAX_SENSOR_EFFECT) {
    SERR("Invalid size=%d",sensor_effect_info->size);
    return -1;
  }
  for (i = 0; i < sensor_effect_info->size; i++) {
    if (sensor_effect_info->effect_settings[i].mode == effect_mode)
      return i;
  }

  return -1;
}

/*===========================================================================
 * FUNCTION    - sensor_write_i2c_init_res_setting_array -
 *
 * DESCRIPTION: Write I2C setting array for init and resolution
 *==========================================================================*/
int32_t sensor_write_i2c_init_res_setting_array(
  sensor_ctrl_t *ctrl,
  struct sensor_i2c_reg_setting_array *settings)
{
  uint32_t i = 0;
  struct sensorb_cfg_data cfg;
  struct msm_camera_i2c_reg_setting setting_k;

  PTHREAD_MUTEX_LOCK(&ctrl->s_data->mutex);

  setting_k.addr_type =
    sensor_sdk_util_get_kernel_i2c_addr_type(settings->addr_type);
  setting_k.data_type =
    sensor_sdk_util_get_kernel_i2c_data_type(settings->data_type);
  setting_k.delay = settings->delay;
  setting_k.size = settings->size;

  setting_k.reg_setting = (struct msm_camera_i2c_reg_array *)
    &(settings->reg_setting_a[0]);

  cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
  cfg.cfg.setting = &setting_k;
  if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg, "write_i2c") < 0) {
    SERR("failed");
    PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);
    return -EIO;
  }

  PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_write_i2c_setting_array -
 *
 * DESCRIPTION: Write I2C setting array
 *==========================================================================*/
int32_t sensor_write_i2c_setting_array(
  sensor_ctrl_t *ctrl,
  struct camera_i2c_reg_setting_array *settings)
{
  uint32_t i = 0;
  struct sensorb_cfg_data cfg;
  struct msm_camera_i2c_reg_setting setting_k;

  PTHREAD_MUTEX_LOCK(&ctrl->s_data->mutex);

  setting_k.addr_type =
    sensor_sdk_util_get_kernel_i2c_addr_type(settings->addr_type);
  setting_k.data_type =
    sensor_sdk_util_get_kernel_i2c_data_type(settings->data_type);
  setting_k.delay = settings->delay;
  setting_k.size = settings->size;

  setting_k.reg_setting = (struct msm_camera_i2c_reg_array *)
    &(settings->reg_setting_a[0]);

  cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
  cfg.cfg.setting = &setting_k;
  if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg, "write_i2c") < 0) {
    SERR("failed");
    PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);
    return -EIO;
  }

  PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_write_i2c_setting -
 *
 * DESCRIPTION: Write I2C setting
 *==========================================================================*/
int32_t sensor_write_i2c_setting(
  sensor_ctrl_t *ctrl,
  struct camera_i2c_reg_setting *setting)
{
  uint32_t i = 0;
  struct sensorb_cfg_data cfg;
  struct msm_camera_i2c_reg_setting setting_k;

  RETURN_ERROR_ON_NULL(ctrl);
  RETURN_ERROR_ON_NULL(ctrl->s_data);

  PTHREAD_MUTEX_LOCK(&ctrl->s_data->mutex);

  translate_sensor_reg_setting(&setting_k, setting);

  SLOW("reg array size %d", setting_k.size);
  for (i = 0; i < setting_k.size; i++) {
    SLOW("addr %x data %x delay = %d",
      setting_k.reg_setting[i].reg_addr, setting_k.reg_setting[i].reg_data,
      setting_k.reg_setting[i].delay);
  }

  cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
  cfg.cfg.setting = &setting_k;
  if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg, "write_i2c") < 0) {
    SERR("failed");
    PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);
    return -EIO;
  }

  PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);

  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_write_aec_settings -
 *
 * DESCRIPTION: Write aec settings for YUV sensor based on stream
 *==========================================================================*/
boolean sensor_write_aec_settings(
  sensor_ctrl_t *ctrl, uint32_t stream_mask)
{
  int32_t rc = 0;
  boolean is_non_zsl_snapshot = FALSE;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;

  if (ctrl->s_data->hal_version == CAM_HAL_V1) {
    if (stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT) &&
    !(stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW)))
    is_non_zsl_snapshot = TRUE;
  } else if (ctrl->s_data->hal_version == CAM_HAL_V3 &&
    ctrl->s_data->capture_intent == CAM_INTENT_STILL_CAPTURE) {
    is_non_zsl_snapshot = TRUE;
  }

  SLOW("is_non_zsl_snapshot = %d", is_non_zsl_snapshot);
  if (is_non_zsl_snapshot) {
    if (lib->sensor_lib_ptr->aec_disable_settings.size) {
      rc = sensor_write_i2c_setting_array(ctrl,
        &(lib->sensor_lib_ptr->aec_disable_settings));
      RETURN_FALSE_IF(rc < 0);
    }
  } else {
    if (lib->sensor_lib_ptr->aec_enable_settings.size) {
      rc = sensor_write_i2c_setting_array(ctrl,
        &(lib->sensor_lib_ptr->aec_enable_settings));
      RETURN_FALSE_IF(rc < 0);
    }
  }

  return TRUE;
}

/*===========================================================================
 * FUNCTION    - sensor_apply_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_apply_exposure(void *sctrl)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  uint32_t fl_lines;
  uint16_t offset;
  struct sensorb_cfg_data cfg;
  struct camera_i2c_reg_setting exp_gain;
  int32_t hdr_mode = 0;
  struct sensor_lib_out_info_array *out_info_array = NULL;
  int32_t cur_res = ctrl->s_data->cur_res;

  /*Currently short exposure values are provided by stats only for
       HDR_ZIGZAG. Update this if condition if its applicable to other types */
  if ((ctrl->s_data->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR) ||
      (ctrl->s_data->hdr_mode == CAM_SENSOR_3EXP_HDR_IN_SENSOR)) {
    if (ctrl->s_data->prev_gain == ctrl->s_data->current_gain &&
      ctrl->s_data->prev_linecount == ctrl->s_data->current_linecount &&
      ctrl->s_data->prev_sensor_digital_gain == ctrl->s_data->sensor_digital_gain &&
      ctrl->s_data->prev_s_reg_gain == ctrl->s_data->s_reg_gain &&
      ctrl->s_data->prev_s_linecount == ctrl->s_data->s_linecount &&
      ctrl->s_data->prev_m_reg_gain == ctrl->s_data->m_reg_gain &&
      ctrl->s_data->prev_m_linecount == ctrl->s_data->m_linecount &&

      ctrl->s_data->prev_s_dig_reg_gain == ctrl->s_data->s_dig_reg_gain &&
      ctrl->s_data->prev_m_dig_reg_gain == ctrl->s_data->m_dig_reg_gain ) {
      return 0;
    }
  } else if ((ctrl->s_data->hdr_mode == CAM_SENSOR_HDR_ZIGZAG) ||
      (ctrl->s_data->hdr_mode == CAM_SENSOR_HDR_STAGGERED)) {
    if (ctrl->s_data->prev_gain == ctrl->s_data->current_gain &&
      ctrl->s_data->prev_linecount == ctrl->s_data->current_linecount &&
      ctrl->s_data->prev_sensor_digital_gain == ctrl->s_data->sensor_digital_gain &&
      ctrl->s_data->prev_s_reg_gain == ctrl->s_data->s_reg_gain &&
      ctrl->s_data->prev_s_linecount == ctrl->s_data->s_linecount) {
      return 0;
    }
  } else if (ctrl->s_data->prev_gain == ctrl->s_data->current_gain &&
      ctrl->s_data->prev_linecount == ctrl->s_data->current_linecount &&
      ctrl->s_data->prev_sensor_digital_gain == ctrl->s_data->sensor_digital_gain &&
      ctrl->s_data->cur_frame_length_lines == ctrl->s_data->last_updated_fll) {
    return 0;
  }

  if(ctrl->s_data->current_linecount >
     lib->sensor_lib_ptr->aec_info.max_linecount) {
    ctrl->s_data->current_linecount
        = lib->sensor_lib_ptr->aec_info.max_linecount;
  }

  fl_lines = ctrl->s_data->cur_frame_length_lines;
  if (ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW) ||
    ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_VIDEO) ||
    ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_CALLBACK))
    fl_lines = fl_lines * ctrl->s_data->current_fps_div;

  offset = lib->sensor_lib_ptr->exp_gain_info.vert_offset;
  if (ctrl->s_data->current_linecount > (fl_lines - offset)) {
    fl_lines = ctrl->s_data->current_linecount + offset;
  } else {
    out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;
    cur_res = ctrl->s_data->cur_res;
    ctrl->s_data->cur_frame_length_lines =
      out_info_array->out_info[cur_res].frame_length_lines;
  }

  ctrl->s_data->last_updated_fll = fl_lines;

  memset(&exp_gain, 0, sizeof(exp_gain));
  /* Allocate the register memory once rather than
     everytime to update exposure setting */
  if (ctrl->s_data->exposure_regs == NULL) {
    ctrl->s_data->exposure_regs = malloc(MAX_EXPOSURE_REGISTERS *
      sizeof(struct camera_i2c_reg_array));
    if (!ctrl->s_data->exposure_regs) {
      SERR("failed");
      return -ENOMEM;
    }
  }
  exp_gain.reg_setting = ctrl->s_data->exposure_regs;

  if ((ctrl->s_data->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR) ||
      (ctrl->s_data->hdr_mode == CAM_SENSOR_3EXP_HDR_IN_SENSOR)) {
    hdr_mode = SENSOR_HDR_IN_SENSOR;
  } else if ((ctrl->s_data->hdr_mode == CAM_SENSOR_HDR_ZIGZAG) ||
            (ctrl->s_data->hdr_mode == CAM_SENSOR_HDR_STAGGERED)){
    hdr_mode = SENSOR_HDR_RAW;
  }else {
    hdr_mode = SENSOR_HDR_OFF;
  }

  memset(exp_gain.reg_setting, 0, MAX_EXPOSURE_REGISTERS *
    sizeof(struct camera_i2c_reg_array));
  /* TODO: design an interface for 3exp reg setting */
  if (lib->sensor_lib_ptr->exposure_func_table.fill_exp_array_type
    == FILL_CUSTOM_IN_LIB &&
    (lib->sensor_lib_ptr->exposure_func_table.sensor_fill_exposure_array ||
    lib->sensor_lib_ptr->exposure_func_table.sensor_fill_exposure_array_with_res)) {
    if (!strcmp(lib->sensor_lib_ptr->sensor_slave_info.sensor_name, "imx519")) {
      rc = lib->sensor_lib_ptr->exposure_func_table.
        sensor_fill_exposure_array_with_res(ctrl->s_data->current_gain,
        ctrl->s_data->sensor_digital_gain,
        ctrl->s_data->current_linecount, fl_lines,
        (int32_t)ctrl->s_data->current_luma_hdr,
        ctrl->s_data->current_fgain_hdr, &exp_gain,
        ctrl->s_data->s_reg_gain, ctrl->s_data->s_linecount, hdr_mode, cur_res);
    } else if( ctrl->s_data->hdr_mode == CAM_SENSOR_3EXP_HDR_IN_SENSOR) {
      rc = lib->sensor_lib_ptr->exposure_func_table.
        sensor_fill_3exp_HDR_exposure_array(ctrl->s_data->current_gain,
        ctrl->s_data->sensor_digital_gain,
        ctrl->s_data->current_linecount, fl_lines,
        (int32_t)ctrl->s_data->current_luma_hdr,
        ctrl->s_data->current_fgain_hdr, &exp_gain,
        ctrl->s_data->s_reg_gain, ctrl->s_data->s_dig_reg_gain, ctrl->s_data->s_linecount,
        ctrl->s_data->m_reg_gain, ctrl->s_data->m_dig_reg_gain,
        ctrl->s_data->m_linecount, hdr_mode);
    } else {
      rc = lib->sensor_lib_ptr->exposure_func_table.
        sensor_fill_exposure_array(ctrl->s_data->current_gain,
          ctrl->s_data->sensor_digital_gain,
          ctrl->s_data->current_linecount, fl_lines,
          (int32_t)ctrl->s_data->current_luma_hdr,
          ctrl->s_data->current_fgain_hdr, &exp_gain,
          ctrl->s_data->s_reg_gain, ctrl->s_data->s_linecount, hdr_mode);
    }
  } else {
    rc = sensor_cmn_lib_fill_exposure(lib->sensor_lib_ptr,
      ctrl->s_data->current_gain,
      ctrl->s_data->sensor_digital_gain,
      ctrl->s_data->current_linecount, fl_lines,
      (int32_t)ctrl->s_data->current_luma_hdr,
      ctrl->s_data->current_fgain_hdr, &exp_gain,
      ctrl->s_data->s_reg_gain, ctrl->s_data->s_linecount, hdr_mode);
  }

  if ((rc < 0) || (exp_gain.size == 0) ||
    (exp_gain.size > MAX_EXPOSURE_REGISTERS)) {
    SERR("failed rc = %d exp_gain.size = %d", rc, exp_gain.size);
  } else {
    SHIGH("cur_gain=%d, applied gain=%d, cur_linecount=%d, fl_lines=%d",
      ctrl->s_data->current_gain, ctrl->s_data->current_gain,
      ctrl->s_data->current_linecount, fl_lines);
    rc = sensor_write_i2c_setting(ctrl, &exp_gain);
    if (rc < 0) {
        SERR("failed");
        return -EIO;
    }
  }

  ctrl->s_data->prev_gain = ctrl->s_data->current_gain;
  ctrl->s_data->prev_linecount = ctrl->s_data->current_linecount;
  ctrl->s_data->prev_sensor_digital_gain = ctrl->s_data->sensor_digital_gain;
  /*updating the values for any HDR type but comparision with prev values are
       done only ZIGZAG/RAW HDR currently as mentioned above*/
  if (ctrl->s_data->hdr_mode != CAM_SENSOR_HDR_OFF) {
    ctrl->s_data->prev_s_reg_gain = ctrl->s_data->s_reg_gain;
    ctrl->s_data->prev_s_linecount = ctrl->s_data->s_linecount;
    ctrl->s_data->prev_s_dig_reg_gain = ctrl->s_data->s_dig_reg_gain;
    if(ctrl->s_data->hdr_mode == CAM_SENSOR_3EXP_HDR_IN_SENSOR) {
        ctrl->s_data->prev_m_reg_gain = ctrl->s_data->s_reg_gain;
        ctrl->s_data->prev_m_linecount = ctrl->s_data->s_linecount;
        ctrl->s_data->prev_m_dig_reg_gain = ctrl->s_data->m_dig_reg_gain;
    }
  }

  return rc;
}

/*===========================================================================
 * FUNCTION    - sensor_set_stop_stream_settings -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_stop_stream_settings(void *sctrl)
{
  struct sensorb_cfg_data cfg;
  struct msm_camera_i2c_reg_setting stop_setting;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = ctrl->lib_params;
  SLOW("enter");

  if (lib->sensor_lib_ptr->sensor_output.output_format == SENSOR_YCBCR)
  {
    SLOW(" %s YUV sensor does not need stop setting",
         lib->sensor_lib_ptr->sensor_slave_info.sensor_name);
    return SENSOR_SUCCESS;
  }

  stop_setting.addr_type =
    sensor_sdk_util_get_kernel_i2c_addr_type(
    lib->sensor_lib_ptr->stop_settings.addr_type);
  stop_setting.data_type =
    sensor_sdk_util_get_kernel_i2c_data_type(
    lib->sensor_lib_ptr->stop_settings.data_type);

  stop_setting.delay = lib->sensor_lib_ptr->stop_settings.delay;
  stop_setting.size = lib->sensor_lib_ptr->stop_settings.size;
  stop_setting.reg_setting = (struct msm_camera_i2c_reg_array *)(&lib->
    sensor_lib_ptr->stop_settings.reg_setting_a[0]);

  cfg.cfgtype = CFG_SET_STOP_STREAM_SETTING;
  cfg.cfg.setting = (void *)&stop_setting;
  if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg,"stop_stream") < 0) {
    SERR("failed");
    return -EIO;
  }

  SLOW("exit");
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_power_up -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_power_up(void *sctrl)
{
  struct sensorb_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  SLOW("enter");

  cfg.cfgtype = CFG_POWER_UP;
  if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg, "power_up") < 0) {
    SERR("failed");
    return -EIO;
  }

  SLOW("exit");
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_write_init_settings -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_write_init_settings(void *sctrl)
{
  int32_t rc = 0;
  uint16_t index = 0;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = ctrl->lib_params;
  struct sensor_lib_reg_settings_array *init_settings = NULL;
  struct sensorb_cfg_data cfg;
  SLOW("enter");

  init_settings = &(lib->sensor_lib_ptr->init_settings_array);
  for (index = 0; index < init_settings->size; index++) {
    rc = sensor_write_i2c_init_res_setting_array(
      ctrl, &init_settings->reg_settings[index]);
    if (rc < 0) {
      SERR("sensor_write_i2c_init_res_setting_array failed");
      return -EIO;
    }
  }
  SLOW("exit");
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_init -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_init(void *sctrl)
{
  int32_t rc = 0;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  SLOW("enter ctrl %p", ctrl);

  if (ctrl->s_data->fd < 0) {
    SERR("failed");
    return -EINVAL;
  }

  ctrl->s_data->cur_stream_mask = 1 << CAM_STREAM_TYPE_DEFAULT;
  ctrl->s_data->current_fps_div = 1.0;

  rc = sensor_set_stop_stream_settings(ctrl);
  if (rc < 0) {
    SERR("failed");
    return rc;
  }

  rc = sensor_power_up(ctrl);
  if (rc < 0) {
    SERR("failed");
    return rc;
  }

  rc = sensor_write_init_settings(sctrl);
  if (rc < 0) {
    SERR("failed");
    return rc;
  }
  SLOW("exit");
  return SENSOR_SUCCESS;
}

/*==========================================================================
 * FUNCTION    - sensor_get_integration_time -
 *
 * DESCRIPTION: Get current integration time
 *==========================================================================*/
int32_t sensor_get_integration_time(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_t  *lib = NULL;
  float         *int_time = (float *) data;
  struct sensor_lib_out_info_t *out_info = NULL;

  if (!ctrl || !ctrl->lib_params || !ctrl->lib_params->sensor_lib_ptr ||
      !ctrl->s_data || !data) {
    SERR("failed: invalid params");
    return -EINVAL;
  }

  lib = ctrl->lib_params->sensor_lib_ptr;
  out_info = &lib->out_info_array.out_info[ctrl->s_data->cur_res];

  *int_time = (float)ctrl->s_data->current_linecount /
      out_info->frame_length_lines * (1 / out_info->max_fps);
  SLOW("current integration time: %f",*int_time);

  return SENSOR_SUCCESS;
}

/** sensor_set_frame_duration: set frame duration
 *
 *  @sctrl: pointer to sensor control data
 *  @data: int64_t handle that contains frame duration to be
 *       applied to sensor in ns
 *
 *  This function applies frame duration applied by application
 *
 *  Return: 0 for success and negative value for failure
 **/
static int32_t sensor_set_frame_duration(void *sctrl, void *data)
{
  sensor_ctrl_t                    *ctrl = (sensor_ctrl_t *)sctrl;
  int64_t                          *frame_duration = (int64_t *)data;
  uint16_t                          new_frame_length_lines = 0;
  struct sensor_lib_out_info_array *out_info_array = NULL;
  int16_t                           cur_res = SENSOR_INVALID_RESOLUTION;
  sensor_data_t *s_data;
  sensor_exposure_table_t          *exposure_func_table = NULL;
  uint32_t                          vt_pix_clk = 0;

  /* Validate input parameters */
  if (!ctrl || !ctrl->lib_params || !ctrl->lib_params->sensor_lib_ptr ||
      !ctrl->s_data) {
    SERR("failed: invalid params");
    return -EINVAL;
  }

  if (!frame_duration) {
    SERR("failed: invalid params");
    return -EINVAL;
  }

  if (*frame_duration <= 0) {
    SERR("failed: invalid params, frame_duration %jd", *frame_duration);
    return -EINVAL;
  }

  s_data = ctrl->s_data;
  /* TODO: If frame duration and exposure time conflict, we need to over-ride
   * frame duration to meet requirement of exposure time */
  SLOW("%s: frame_duration = %llu", __func__, *frame_duration);
  out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;
  cur_res = ctrl->s_data->cur_res;

  /* Validate current resolution to find whether sensor is streaming  */
  if ((cur_res >= 0) && (cur_res < out_info_array->size)) {
    vt_pix_clk = SENSOR_GET_VT_PIK_CLK(&out_info_array->out_info[cur_res]);
    new_frame_length_lines = (uint16_t)(((float)vt_pix_clk /
      out_info_array->out_info[cur_res].line_length_pclk) *
      ((float)*frame_duration / NANO_SEC_PER_SEC));
    if (new_frame_length_lines < ctrl->s_data->cur_frame_length_lines) {
      /* Cap it to cur frame length lines */
      new_frame_length_lines = ctrl->s_data->cur_frame_length_lines;
    }
    /* Update fps divider */
    ctrl->s_data->current_fps_div =
      (double)new_frame_length_lines / ctrl->s_data->cur_frame_length_lines;
    SLOW("new frame duration %jd new fll %d cur fll %d", *frame_duration,
      new_frame_length_lines, ctrl->s_data->cur_frame_length_lines);
  } else {
    /* Update max_fps so that sensor will pick proper resolution based
       on new max_fps passed by application */
    ctrl->s_data->max_fps = (float)(NANO_SEC_PER_SEC / *frame_duration);
  }

  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_set_hal_version -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_hal_version(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t *hal_version = (int32_t *)data;
  if (!hal_version) {
    SERR("failed");
    return -EINVAL;
  }
  SLOW("hal version %d", *hal_version);
  ctrl->s_data->hal_version = *hal_version;
  return SENSOR_SUCCESS;
}


/*==========================================================
 * FUNCTION    - sensor_set_capture_intent -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_capture_intent(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t *capture_intent = (int32_t *)data;
  if (!capture_intent) {
    SERR("failed");
    return -EINVAL;
  }
  SHIGH("capture intent %d", *capture_intent);
  ctrl->s_data->capture_intent = *capture_intent;
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_set_delay_cfg -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_delay_cfg(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t *delay_en = (int32_t *)data;

  if (!delay_en) {
    SERR("failed");
    return -EINVAL;
  }
  SLOW("Delay en config: %d", *delay_en);
  ctrl->s_data->delay_en = *delay_en;
  return 0;
}

/*==========================================================
 * FUNCTION    - sensor_get_delay_cfg -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_get_delay_cfg(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t *delay_en = (int32_t *)data;

  if (!delay_en) {
    SERR("failed");
    return -EINVAL;
  }
  SLOW("Sensor delay config: %d", ctrl->s_data->delay_en);
  *delay_en = ctrl->s_data->delay_en;
  return 0;
}


/*===========================================================================
 * FUNCTION    - sensor_set_quadra_mode -
 *
 * DESCRIPTION: Set quadra mode
 *==========================================================================*/
static int32_t sensor_set_quadra_mode(void *sctrl, void *data)
{
  if (!sctrl || !data) {
    SERR("Failed");
    return SENSOR_FAILURE;
  }
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  ctrl->s_data->is_quadra_mode = *((uint32_t *)data);
  SHIGH("Quadra CFA Quadra mode enable: %d", ctrl->s_data->is_quadra_mode);
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_hfr_mode -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_hfr_mode(void *sctrl, void *data)
{
  if (!sctrl || !data) {
    SERR("Failed");
    return SENSOR_FAILURE;
  }
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  cam_hfr_mode_t hfr_mode = *((cam_hfr_mode_t*)data);
  SLOW("hfr_mode = %d", hfr_mode);
  ctrl->s_data->hfr_mode = hfr_mode;
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_hdr_ae_bracket -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_hdr_ae_bracket(void *sctrl, void *data)
{
  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  cam_exp_bracketing_t *ae_bracket_config = (cam_exp_bracketing_t*)data;
  SLOW("ae_bracket_mode=%d, str=%s",
         ae_bracket_config->mode, ae_bracket_config->values);
  /* copy the ae_bracket config in local data structure */
  memcpy(&(ctrl->s_data->ae_bracket_info.ae_bracket_config),
          ae_bracket_config, sizeof(cam_exp_bracketing_t));
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_aec_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_aec_update(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  aec_update_t* aec_update = (aec_update_t*) data;
  sensor_exp_t exposure;
  memset(&exposure, 0, sizeof(exposure));

  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  if (!(ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT)) ||
      !ctrl->s_data->lock_aec) {
    exposure.luma_hdr = 0;
    exposure.fgain_hdr = 0;
    exposure.real_gain = aec_update->sensor_gain;
    exposure.linecount = aec_update->linecount;

    if (ctrl->s_data->hdr_mode != CAM_SENSOR_HDR_OFF) {
      exposure.luma_hdr = aec_update->cur_luma;
      exposure.fgain_hdr = aec_update->luma_delta;
      /* fgain_hdr: lower 16 bits stores luma_delta;
       bit 17 stores hdr_indoor_detected flag */
      exposure.fgain_hdr |= aec_update->hdr_indoor_detected << 16;
      exposure.s_real_gain = aec_update->s_real_gain;
      exposure.s_linecount = aec_update->s_linecount;
      if(ctrl->s_data->hdr_mode == CAM_SENSOR_3EXP_HDR_IN_SENSOR) {
        exposure.m_real_gain = aec_update->m_real_gain;
        exposure.m_linecount = aec_update->m_linecount;
      }
    }

    sensor_set_exposure(sctrl, exposure);
    sensor_apply_exposure(sctrl);
  }

  return SENSOR_SUCCESS;
}

static int32_t sensor_set_aec_manual_update(void *sctrl, void *data)
{
  sensor_ctrl_t       *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_aec_update_t *aec_update = (sensor_aec_update_t*) data;
  sensor_exp_t         exposure;

  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  exposure.luma_hdr = 0;
  exposure.fgain_hdr = 0;
  exposure.s_real_gain = aec_update->s_real_gain;
  exposure.s_linecount = aec_update->s_linecount;
  exposure.real_gain = aec_update->real_gain;
  exposure.linecount = aec_update->linecount;

  sensor_set_exposure(sctrl, exposure);
  sensor_apply_exposure(sctrl);

  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_get_exp_info -
 *
 * DESCRIPTION: fetches exposure info of sensor
 *==========================================================================*/
static int32_t sensor_get_exposure_info(void *sctrl, void *data)
{
  sensor_ctrl_t                    *ctrl;
  sensor_dual_exp_t                *dual_exp = NULL;
  struct sensor_lib_out_info_array *out_info_array = NULL;
  int16_t                          cur_res = SENSOR_INVALID_RESOLUTION;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  ctrl = (sensor_ctrl_t *)sctrl;
  dual_exp = (sensor_dual_exp_t*) data;

  out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;
  cur_res = ctrl->s_data->cur_res;

  dual_exp->master_fl_lines =
    out_info_array->out_info[cur_res].frame_length_lines;
  dual_exp->master_lc = ctrl->s_data->current_linecount;

  return SENSOR_SUCCESS;
}
/*===========================================================================
 * FUNCTION    - sensor_set_aec_update_for_slave -
 *
 * DESCRIPTION: Match the slave sessions fps to master's fps
 * master_fps - this is active session's exp_time
 * linecount  - this is the adjusted lc for slave based on master's exp_time
 *              slave_lc = (master_exp_time)*(vt_pclk_slave / ll_pclk_slave)
 * new_fll    - This is set based on either the phase correction needed
 *              or based on master's exposure to maintain same fps.
 *              This is the adjusted fll for slave based on master_fps
 *              same as linecount when running at max fps, when fps drops
 *              it is adjusted(equal to linecount)
 *              new_fll =(max_fps_slave/master_fps) * max_fll_slave= linecount
 *==========================================================================*/
static int32_t sensor_set_aec_update_for_slave(void *sctrl, void *data)
{
    sensor_ctrl_t                    *ctrl;
    sensor_dual_exp_t                *dual_exp = NULL;
    sensor_exp_t                      exposure;
    struct sensor_lib_out_info_array *out_info_array = NULL;
    uint16_t                          linecount, new_fll;
    int16_t                           cur_res;
    uint32_t                          vt_pix_clk = 0;
    float                             adjusted_gain = 0.0;
    float                             exp_time = 0.0;

    RETURN_ERROR_ON_NULL(sctrl);
    RETURN_ERROR_ON_NULL(data);

    ctrl = (sensor_ctrl_t *)sctrl;
    dual_exp = (sensor_dual_exp_t*) data;

    out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;
    cur_res = ctrl->s_data->cur_res;
    vt_pix_clk = SENSOR_GET_VT_PIK_CLK(&out_info_array->out_info[cur_res]);

    /* Calculate the adjusted linecount for slave based on master's linecount */
    linecount = (uint16_t)(((float)dual_exp->master_lc *
      out_info_array->out_info[cur_res].frame_length_lines) /
      ((float)dual_exp->master_fl_lines));

    exp_time = (float)linecount /
      ((float)out_info_array->out_info[cur_res].frame_length_lines *
      out_info_array->out_info[cur_res].max_fps);

    /* Ensure running at max fps */
    if (linecount > out_info_array->out_info[cur_res].frame_length_lines)
      new_fll = linecount;
    else
      new_fll = out_info_array->out_info[cur_res].frame_length_lines;

    /* Update the current fps for slave session based on master session */
    ctrl->s_data->cur_frame_length_lines = new_fll;

    if (dual_exp->stats_update->aec_update.dual_exp_multiplier != 0 &&
      exp_time != 0 && dual_exp->stats_update->aec_update.exp_time != 0) {
      /* Adjust the gain for slave session based on exp multipler of master */
      adjusted_gain = (dual_exp->stats_update->aec_update.sensor_gain *
        dual_exp->stats_update->aec_update.dual_exp_multiplier) *
        (dual_exp->stats_update->aec_update.exp_time / exp_time);
    } else {
      adjusted_gain = dual_exp->stats_update->aec_update.sensor_gain;
    }
    /*adjusted gain can't be a fraction. We need to adjust that in
      linecount and make sure, that doesn't become zero*/
    if (adjusted_gain < 1) {
      linecount *= adjusted_gain;
      adjusted_gain = 1;
      if (linecount == 0) linecount = 1;
    }

    exposure.luma_hdr = 0;
    exposure.fgain_hdr = 0;

    exposure.real_gain = adjusted_gain;
    exposure.linecount = linecount;
    /* slave sensor does not support hdr */
    exposure.s_real_gain = 0;
    exposure.s_linecount = 0;

    SLOW("[dual] slave aec: cur_frame_length_lines = %d, linecount = %d",
      ctrl->s_data->cur_frame_length_lines,linecount);

    sensor_set_exposure(sctrl, exposure);
    sensor_apply_exposure(sctrl);

    return SENSOR_SUCCESS;
}

static int32_t sensor_set_aec_update_for_dual(void *sctrl, void *data)
{
  sensor_ctrl_t                    *ctrl;
  sensor_dual_exp_t                *dual_exp;
  sensor_exp_t                      exposure;
  struct sensor_lib_out_info_array *out_info_array = NULL;
  float                             multiplier, gain_tune = 1.0, factor, int_gain;
  float                             exp_time, gain, min_gain, line_tune = 1.0;
  uint16_t                          linecount, vert_offset, new_fll;
  int16_t                           cur_res;
  float                             max_analog_gain, min_analog_gain, exp_time_int;
  float                             min_fps_exp_time, res_multiplier;
  char value[PROPERTY_VALUE_MAX];
  uint32_t                          vt_pix_clk = 0;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  ctrl = (sensor_ctrl_t *)sctrl;
  dual_exp = (sensor_dual_exp_t*) data;

  out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;
  cur_res = ctrl->s_data->cur_res;
  vt_pix_clk = SENSOR_GET_VT_PIK_CLK(&out_info_array->out_info[cur_res]);
  vert_offset = ctrl->lib_params->sensor_lib_ptr->exp_gain_info.vert_offset;

  /* (1) calculate multiplier */
  multiplier = dual_exp->exp_multiplier / ctrl->s_data->exp_multiplier;
  SLOW("[dual]peer multiplier %f my_multiplier %f => %f",
    dual_exp->exp_multiplier, ctrl->s_data->exp_multiplier, multiplier);

  /* (2) convert exposure time and gain */
  SLOW("[dual]peer exp time %f gain %f linecount %d",
    dual_exp->stats_update->aec_update.exp_time,
    dual_exp->stats_update->aec_update.sensor_gain,
    dual_exp->stats_update->aec_update.linecount);

  /* (2-1) convert exposure time */
  if (dual_exp->stats_update->aec_update.exp_time == 0) return SENSOR_SUCCESS;

  /* (2-1) gain and integration adjust factor */
#ifdef __ANDROID__
  property_get("persist.vendor.dual.gain_tune", value, "1.0");
  factor = atof(value);
  if (factor > 0) {
    gain_tune = factor;
    SLOW("gain tune factor %f", gain_tune);
  }
  property_get("persist.vendor.dual.line_tune", value, "1.0");
  factor = atof(value);
  if (factor > 0) {
    line_tune = factor;
    SLOW("exposure time tune factor %f", line_tune);
  }
#endif

  min_fps_exp_time = (float)(1.0/out_info_array->out_info[cur_res].min_fps);
  max_analog_gain =  ctrl->lib_params->sensor_lib_ptr->aec_info.max_analog_gain;
  min_analog_gain =  ctrl->lib_params->sensor_lib_ptr->aec_info.min_gain;

  if (multiplier >= 1.0)
  {
    exp_time = dual_exp->stats_update->aec_update.exp_time * multiplier;
    exp_time = MINQ(exp_time, min_fps_exp_time);
    res_multiplier = multiplier * dual_exp->stats_update->aec_update.exp_time /
      exp_time;
    gain = dual_exp->stats_update->aec_update.sensor_gain * res_multiplier;
  }
  else
  {
    gain = dual_exp->stats_update->aec_update.sensor_gain * multiplier * gain_tune;
    gain = MAXQ(min_analog_gain, MINQ(max_analog_gain, gain));
    res_multiplier = multiplier * dual_exp->stats_update->aec_update.sensor_gain *
      gain_tune / gain ;
    exp_time = dual_exp->stats_update->aec_update.exp_time;
    exp_time_int = exp_time * res_multiplier;
    exp_time_int = MINQ(exp_time_int, min_fps_exp_time);
    gain = gain * (exp_time / exp_time_int) * res_multiplier;
  }
  linecount = (1.0 * exp_time * line_tune * vt_pix_clk /
      out_info_array->out_info[cur_res].line_length_pclk);
  if (linecount < 1) {
    linecount = 1;
  }

  /* When FPS changes, change FLL to both sensor's FPS are same.
     1. primary exp time = aux expe time : no need to adjust fps
        set both cur_frame_length_lines back to initial value.
     2. primary exp time > aux expe time : adjust aux fps
     3. primary exp time < aux expe time : adjust primary fps */
  if (dual_exp->stats_update->aec_update.exp_time == exp_time) {
    ctrl->s_data->cur_frame_length_lines =
      out_info_array->out_info[cur_res].frame_length_lines;
    dual_exp->adjusted_frame_duration = 0;
  } else if (dual_exp->stats_update->aec_update.exp_time > exp_time) {
    new_fll = (uint16_t)(((float)vt_pix_clk /
      out_info_array->out_info[cur_res].line_length_pclk) *
      dual_exp->stats_update->aec_update.exp_time);
    if (new_fll < out_info_array->out_info[cur_res].frame_length_lines)
      new_fll = out_info_array->out_info[cur_res].frame_length_lines;

    ctrl->s_data->cur_frame_length_lines = new_fll;
    dual_exp->adjusted_frame_duration = 0;
    SLOW("adjust AUX's FPS : duration %f new fll %d",
      dual_exp->stats_update->aec_update.exp_time, new_fll);
  } else {
    ctrl->s_data->cur_frame_length_lines =
      out_info_array->out_info[cur_res].frame_length_lines;
    dual_exp->adjusted_frame_duration = exp_time;
    SLOW("adjust PRIMARY's FPS : duration %f", exp_time);
  }

  min_gain = MAXQ(1.0, ctrl->lib_params->sensor_lib_ptr->aec_info.min_gain);
  gain = MAXQ(gain, min_gain);
  SLOW("[dual]converted linecount %d gain %f", linecount, gain);

  exposure.luma_hdr = 0;
  exposure.fgain_hdr = 0;

  exposure.real_gain = gain;
  exposure.linecount = linecount;
  /* slave sensor does not support hdr */
  exposure.s_real_gain = 0;
  exposure.s_linecount = 0;

  sensor_set_exposure(sctrl, exposure);
  sensor_apply_exposure(sctrl);

  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_aec_init_settings -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_aec_init_settings(void *sctrl, void *data)
{
  boolean ae_update = TRUE;


  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  aec_get_t* aec_data = (aec_get_t*) data;
  int32_t valid_entries = (int32_t)aec_data->valid_entries;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;

  SLOW("aec_data.valid_entries=%d", valid_entries);
  if (valid_entries <= 0) {
    SERR("no valid entries in aec_get");
    return SENSOR_FAILURE;
  }
  if (ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT) &&
    (ctrl->s_data->ae_bracket_info.ae_bracket_config.mode == CAM_EXP_BRACKETING_ON)) {
    ctrl->s_data->ae_bracket_info.valid_entries = valid_entries;
    int32_t i;
    SLOW("valid entries %d", valid_entries);
    for (i=0; i < valid_entries; i++) {
      SLOW("g%d=%f, lc%d=%d", i,
        aec_data->real_gain[i], i, aec_data->linecount[i]);
      ctrl->s_data->ae_bracket_info.real_gain[i] = aec_data->sensor_gain[i];
      ctrl->s_data->ae_bracket_info.linecount[i] = aec_data->linecount[i];
    }

    if (sensor_delay_init(sctrl) < 0)
      return SENSOR_FAILURE;

    /* to apply entries starting from 1 from next SOF
    (entry 0 will be set in sensor_set_aec_init_settings) */
    if (lib->sensor_lib_ptr->sensor_max_pipeline_frame_delay <=
      lib->sensor_lib_ptr->sensor_num_HDR_frame_skip) {
      ctrl->s_data->ae_bracket_info.apply_index = 0;
      ctrl->s_data->ae_bracket_info.sof_counter =
        (int32_t)aec_data->valid_entries + sensor_delay_get_max(sctrl);
      ae_update = FALSE;
    }
    else {
      ctrl->s_data->ae_bracket_info.apply_index = 1;
      ctrl->s_data->ae_bracket_info.sof_counter =
        (int32_t)aec_data->valid_entries + sensor_delay_get_max(sctrl) - 1;
    }

    ctrl->s_data->sensor_skip_counter = 0;
    ctrl->s_data->ae_bracket_info.post_meta_bus = 0;
  }
  if(ctrl->s_data->hdr_mode != CAM_SENSOR_HDR_OFF){
    ctrl->s_data->s_real_gain = aec_data->s_real_gain;
    ctrl->s_data->s_linecount = aec_data->s_linecount;

    if(ctrl->s_data->hdr_mode == CAM_SENSOR_3EXP_HDR_IN_SENSOR) {
      ctrl->s_data->m_real_gain = aec_data->m_real_gain;
      ctrl->s_data->m_linecount = aec_data->m_linecount;
    }
  }

  if (TRUE == ae_update) {
    /* update exposure, first valid entry */
    sensor_exp_t exposure;
    memset(&exposure, 0, sizeof(exposure));
    exposure.real_gain = aec_data->sensor_gain[0];
    exposure.linecount = aec_data->linecount[0];
    exposure.s_real_gain = aec_data->s_real_gain;
    exposure.s_linecount = aec_data->s_linecount;
    if(ctrl->s_data->hdr_mode == CAM_SENSOR_3EXP_HDR_IN_SENSOR) {
      exposure.m_real_gain = aec_data->m_real_gain;
      exposure.m_linecount = aec_data->m_linecount;
    }
    SHIGH("ae-bracket:idx[%d] of %d real_gain=%f, linecount=%d",
      0, valid_entries,
             exposure.real_gain, exposure.linecount);
    SHIGH("short real gain from 3A = %f, short linecount = %d",
        exposure.s_real_gain, exposure.s_linecount);
    sensor_set_exposure(sctrl, exposure);
    sensor_apply_exposure(sctrl);
  }

  return SENSOR_SUCCESS;
}
/*===========================================================================
 * FUNCTION    - sensor_set_hdr_zsl_mode -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_hdr_zsl_mode(void *sctrl, void *data)
{
  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  sensor_set_hdr_ae_t *hdr_info = (sensor_set_hdr_ae_t *)data;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = ctrl->lib_params;
  uint32_t sensor_max_pipeline_frame_delay;

  sensor_max_pipeline_frame_delay =
    lib->sensor_lib_ptr->sensor_max_pipeline_frame_delay;

  ctrl->s_data->hdr_zsl_mode = hdr_info->hdr_zsl_mode;
  ctrl->s_data->isp_frame_skip = hdr_info->isp_frame_skip;

  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_manual_exposure_mode -
 *
 * DESCRIPTION:
 * ==========================================================================*/
static int8_t sensor_set_manual_exposure_mode(void *sctrl, void *data)
{
  if (!sctrl || !data) {
    SERR("Failed");
    return SENSOR_FAILURE;
  }

  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  uint8_t manual_exposure_mode = *((uint8_t*)data);
  SLOW("manual_exposure_mode = %d", manual_exposure_mode);
  ctrl->s_data->manual_exposure_mode = manual_exposure_mode;
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_post_hdr_meta -
 *
 * DESCRIPTION:
 *==========================================================================*/

static int32_t sensor_post_hdr_meta(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  if (!ctrl || !ctrl->s_data){
    SERR("Failed");
    return SENSOR_FAILURE;
  }
  if (!data)
    return SENSOR_SUCCESS; /*Null hdr meta pointer is not fatal*/
  sensor_hdr_meta_t *hdr_meta = (sensor_hdr_meta_t *)data;
  /*Init default value to post hdr meta*/
  hdr_meta->post_meta_bus = 0;
  if (ctrl->s_data->ae_bracket_info.ae_bracket_config.mode !=
    CAM_EXP_BRACKETING_ON)
    return SENSOR_SUCCESS;

  /*Conditions to determine whether to post hdr meta to bus:
   * - Hdr information needs to be posted to bus only for Zsl case
   *
   * - Current implementation requires meta data to be posted only for
   * first HDR frame from hdr counter. We match the hdr counter against
   * the current sof counter also accounting the isp frame skip.
   */
  if (ctrl->s_data->hdr_zsl_mode) {
      if (!ctrl->s_data->ae_bracket_info.post_meta_bus) {
        hdr_meta->post_meta_bus = 1;
        ctrl->s_data->ae_bracket_info.post_meta_bus = 1;
        hdr_meta->isp_frame_skip = ctrl->s_data->isp_frame_skip;
     }
  }
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_aec_zsl_settings -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_aec_zsl_settings(void *sctrl, void *data)
{
  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  aec_get_t *aec_data = (aec_get_t*) data;
  int32_t valid_entries = (int32_t)aec_data->valid_entries;

  SLOW("zsl aec_data.valid_entries=%d", valid_entries);
  if (valid_entries <= 0) {
    SERR("no valid entries in aec_get");
    return SENSOR_FAILURE;
  }
  if (ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_SNAPSHOT)) {
    if (ctrl->s_data->ae_bracket_info.ae_bracket_config.mode ==
          CAM_EXP_BRACKETING_ON) {
      ctrl->s_data->ae_bracket_info.valid_entries = valid_entries;
      int32_t i;
      for (i=0; i < valid_entries; i++) {
        SHIGH("g%d=%f, lc%d=%d", i,
             aec_data->real_gain[i], i, aec_data->linecount[i]);
        ctrl->s_data->sensor_common_info->ae_bracket_params.real_gain[i] =
          ctrl->s_data->ae_bracket_info.real_gain[i] = aec_data->sensor_gain[i];
        ctrl->s_data->sensor_common_info->ae_bracket_params.linecount[i] =
          ctrl->s_data->ae_bracket_info.linecount[i] = aec_data->linecount[i];
      }

      ctrl->s_data->sensor_common_info->ae_bracket_params.lux_idx =
          aec_data->lux_idx;

      if (sensor_delay_init(sctrl) < 0)
        return SENSOR_FAILURE;

      ctrl->s_data->sensor_skip_counter = 0;
      ctrl->s_data->ae_bracket_info.apply_index = 0;
      ctrl->s_data->ae_bracket_info.sof_counter =
        (int32_t)aec_data->valid_entries;
      ctrl->s_data->ae_bracket_info.post_meta_bus = 0;
    }
  }

  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_vfe_sof -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_vfe_sof(void *sctrl, void *data)
{

  int32_t valid_entries;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_hdr_meta_t *hdr_meta = (sensor_hdr_meta_t *)data;
  int sensor_delay = 0;

  if (!ctrl || !ctrl->s_data || !data) {
    return SENSOR_FAILURE;
  }

  ctrl->last_frame_id = hdr_meta->current_sof;
  ctrl->s_data->isp_frame_skip = hdr_meta->isp_frame_skip;

  valid_entries = ctrl->s_data->ae_bracket_info.valid_entries;
  /* apply exposures in AE bracketing mode */
  if (ctrl->s_data->ae_bracket_info.ae_bracket_config.mode ==
       CAM_EXP_BRACKETING_ON && valid_entries > 1) {

    ctrl->s_data->lock_aec = !!ctrl->s_data->ae_bracket_info.sof_counter;

    if ((ctrl->s_data->ae_bracket_info.sof_counter > 0) &&
      (ctrl->s_data->sensor_skip_counter <= 0)) {

      sensor_exp_t exposure;
      memset(&exposure, 0, sizeof(exposure));
      int32_t idx;
      if (valid_entries > ctrl->s_data->ae_bracket_info.apply_index)
        idx = ctrl->s_data->ae_bracket_info.apply_index % valid_entries;
      else
        idx = valid_entries - 1;

      exposure.real_gain =
        ctrl->s_data->sensor_common_info->ae_bracket_params.real_gain[idx] =
        ctrl->s_data->ae_bracket_info.real_gain[idx];
      exposure.linecount =
        ctrl->s_data->sensor_common_info->ae_bracket_params.linecount[idx] =
        ctrl->s_data->ae_bracket_info.linecount[idx];
      if(ctrl->s_data->hdr_mode == CAM_SENSOR_3EXP_HDR_IN_SENSOR) {
        exposure.s_linecount = exposure.m_linecount = exposure.linecount;
        exposure.s_real_gain = exposure.m_real_gain = exposure.real_gain;
      }
      SHIGH("ae-bracket:idx[%d] of %d real_gain=%f, linecount=%d",
        idx, valid_entries,
               exposure.real_gain, exposure.linecount);
      SLOW("ae-bracket: sof_counter=%d, applying exposure, idx=%d",
        ctrl->s_data->ae_bracket_info.sof_counter, idx);
      hdr_meta->aec_index = idx;
      sensor_set_exposure(sctrl, exposure);
      sensor_apply_exposure(sctrl);
      sensor_post_hdr_meta(sctrl, data);
      ctrl->s_data->ae_bracket_info.sof_counter--;
      ctrl->s_data->ae_bracket_info.apply_index++;
      ctrl->s_data->sensor_skip_counter = ctrl->s_data->isp_frame_skip;
    } else {
      SHIGH("AE bracket exposure not applied:"
        "sof counter=%d, sensor frame skip = %d",
        ctrl->s_data->ae_bracket_info.sof_counter,
        ctrl->s_data->sensor_skip_counter);
      sensor_delay = sensor_delay_get_max(ctrl);
      if (sensor_delay > 0 &&
        ((ctrl->s_data->isp_frame_skip - ctrl->s_data->sensor_skip_counter) <
        sensor_delay)) {
        sensor_exp_t exposure;
        int32_t idx;
        if (valid_entries > ctrl->s_data->ae_bracket_info.apply_index)
          idx = (ctrl->s_data->ae_bracket_info.apply_index - 1) % valid_entries;
        else
          idx = valid_entries - 1;

        exposure.real_gain =
          ctrl->s_data->sensor_common_info->ae_bracket_params.real_gain[idx];
        exposure.linecount =
          ctrl->s_data->sensor_common_info->ae_bracket_params.linecount[idx];
        sensor_set_exposure(sctrl, exposure);
        sensor_apply_exposure(sctrl);
      }
      if (ctrl->s_data->sensor_skip_counter > 0) {
        ctrl->s_data->sensor_skip_counter--;
      }
    }
  } else {
    SDBG("AE Bracketing not enabled");
  }

  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_frame_rate -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_frame_rate(void *sctrl, void *data)
{
  sensor_ctrl_t   *ctrl = (sensor_ctrl_t *)sctrl;
  cam_fps_range_t *fps = (cam_fps_range_t *)data;
  int16_t          cur_res = SENSOR_INVALID_RESOLUTION;
  struct sensor_lib_out_info_array *out_info_array = NULL;
  struct camera_i2c_reg_setting fps_settings;
  int32_t rc = SENSOR_SUCCESS;

  if (!fps) {
    SERR("data NULL");
    return SENSOR_FAILURE;
  }
  SLOW("max fps %f min fps %f", fps->max_fps,
    fps->min_fps);

  ctrl->s_data->max_fps = ctrl->s_data->cur_fps = fps->video_max_fps;
  cur_res = ctrl->s_data->cur_res;
  out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;

  if (ctrl->lib_params->sensor_lib_ptr->sensor_output.is_depth_sensor &&
    cur_res != SENSOR_INVALID_RESOLUTION) {
    if (!ctrl->lib_params->sensor_lib_ptr->fps_func_table.sensor_fill_fps_array) {
      SERR("Varying FPS is not supoprted in this depth sensor");
      return SENSOR_SUCCESS;
    }
    /*This is a depth sensor which controls FPS through direct register writes*/
    if (ctrl->s_data->cur_fps > out_info_array->out_info[cur_res].max_fps) {
      SLOW("set_fps=%f > max_fps=%f, capping to max",
        ctrl->s_data->cur_fps, out_info_array->out_info[cur_res].max_fps);
      ctrl->s_data->cur_fps = out_info_array->out_info[cur_res].max_fps;
    }
    ctrl->s_data->current_fps_div =
      out_info_array->out_info[cur_res].max_fps / ctrl->s_data->cur_fps;
    memset(&fps_settings, 0, sizeof(fps_settings));
    /* Allocate the register memory once rather than
      everytime to update fps setting */
    if (ctrl->s_data->fps_regs == NULL) {
      ctrl->s_data->fps_regs = malloc(MAX_FPS_REGISTERS *
        sizeof(struct camera_i2c_reg_array));
      if (!ctrl->s_data->fps_regs) {
        SERR("failed");
        return -ENOMEM;
      }
    }
    fps_settings.reg_setting = ctrl->s_data->fps_regs;

    rc = ctrl->lib_params->sensor_lib_ptr->fps_func_table.
      sensor_fill_fps_array(ctrl->s_data->cur_fps, &fps_settings);
    if ((rc < 0) || (fps_settings.size == 0) ||
      (fps_settings.size > MAX_EXPOSURE_REGISTERS)) {
      SERR("failed rc = %d fps_settings.size = %d", rc, fps_settings.size);
    } else {
      rc = sensor_write_i2c_setting(ctrl, &fps_settings);
      if (rc < 0) {
        SERR("failed");
        return -EIO;
      }
    }
    return SENSOR_SUCCESS;
  }

  /* Update fps divider if cur stream type is preview or video */
  if ((ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW) ||
    ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_VIDEO) ||
    ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_CALLBACK)) &&
    (ctrl->s_data->cur_res != SENSOR_INVALID_RESOLUTION)) {

    if (ctrl->s_data->cur_fps > out_info_array->out_info[cur_res].max_fps) {
      SLOW("set_fps=%f > max_fps=%f, capping to max",
        ctrl->s_data->cur_fps, out_info_array->out_info[cur_res].max_fps);
      ctrl->s_data->cur_fps = out_info_array->out_info[cur_res].max_fps;
    }
    ctrl->s_data->current_fps_div =
      out_info_array->out_info[cur_res].max_fps / ctrl->s_data->cur_fps;
    ctrl->s_data->prev_gain = 0;
    ctrl->s_data->prev_linecount = 0;
    ctrl->s_data->prev_s_reg_gain = 0;
    ctrl->s_data->prev_s_linecount = 0;

    ctrl->s_data->prev_m_reg_gain = 0;
    ctrl->s_data->prev_m_linecount = 0;
    sensor_set_vfe_sof(sctrl, NULL);
  }

  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_awb_update -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_awb_update(void *sctrl, void *data)
{
  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  int32_t rc = SENSOR_SUCCESS;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  awb_update_t* awb_update = (awb_update_t*) data;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  struct sensorb_cfg_data cfg;
  struct camera_i2c_seq_reg_setting awb_reg;
  struct msm_camera_i2c_seq_reg_setting awb_reg_k;

  if (!lib || !lib->sensor_lib_ptr) {
    SERR("failed: invalid sensor lib ptr");
    return SENSOR_FAILURE;
  }

  if (!lib->sensor_lib_ptr->awb_func_table.awb_table_size) {
    return SENSOR_SUCCESS;
  }

  if ((lib->sensor_lib_ptr->sensor_capability & 0x1) == 0 &&
    ctrl->s_data->hdr_mode == CAM_SENSOR_HDR_OFF) {
    return SENSOR_SUCCESS;
  }

  /*convert AWB gains to sensor register format (u5.8 fixed point)*/
  uint16_t awb_gain_r_reg = (uint16_t)(awb_update->gain.r_gain * 256.0);
  uint16_t awb_gain_b_reg = (uint16_t)(awb_update->gain.b_gain * 256.0);
  SLOW("awb_gain_r_reg: %d, awb_gain_b_reg: %d",awb_gain_r_reg,awb_gain_b_reg);
  awb_reg.reg_setting = malloc(lib->sensor_lib_ptr->
    awb_func_table.awb_table_size *
    sizeof(struct camera_i2c_seq_reg_array));
  if (!awb_reg.reg_setting) {
    SERR("failed");
    return -ENOMEM;
  }

  rc = lib->sensor_lib_ptr->awb_func_table.
    sensor_fill_awb_array(awb_gain_r_reg,
    awb_gain_b_reg, &awb_reg);

  /* Translate seq reg settings from uspace structure to kernel struct */
  translate_sensor_seq_reg_setting(&awb_reg_k, &awb_reg);

  if (rc < 0) {
    SERR("failed");
  } else {
    cfg.cfgtype = CFG_WRITE_I2C_SEQ_ARRAY;
    cfg.cfg.setting = &awb_reg_k;
    if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg, "awb_update") < 0) {
        SERR("failed");
        free(awb_reg.reg_setting);
        return -EIO;
    }
  }
  free(awb_reg.reg_setting);
  /*group hold off */

  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_start_stream -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_start_stream(void *sctrl)
{
  int32_t rc = 0;
  struct sensorb_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;

  SHIGH("Sensor stream ON for %s \n",
    lib->sensor_lib_ptr->sensor_slave_info.sensor_name);

  if (ctrl->s_data->fd < 0)
    return SENSOR_FAILURE;

  rc = sensor_write_i2c_setting_array(ctrl,
    &(ctrl->lib_params->sensor_lib_ptr->start_settings));
  if (rc)
    SERR("sensor_write_i2c_setting_array failed");

  SLOW("exit");
  return rc;
}

/*===========================================================================
 * FUNCTION    - sensor_set_delayed_start_stream -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_delayed_start_stream(void *sctrl)
{
  int32_t rc = 0;
  struct sensorb_cfg_data cfg;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  uint32_t frame_delay;

  SHIGH("Sensor stream ON for %s \n",
    lib->sensor_lib_ptr->sensor_slave_info.sensor_name);

  if (ctrl->s_data->fd < 0)
    return SENSOR_FAILURE;

  rc = sensor_write_i2c_setting_array(ctrl,
    &(ctrl->lib_params->sensor_lib_ptr->start_settings));
  if (rc)
    SERR("sensor_write_i2c_setting_array failed");

  /* Delay one frame */
  frame_delay = (uint32_t) (1000000.0f / ctrl->s_data->cur_fps);

  SLOW("Sensor delayed start time: %d [uS]", frame_delay);

  usleep(frame_delay);

  SLOW("exit");
  return rc;
}

/*===========================================================================
 * FUNCTION    - sensor_set_stop_stream -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_stop_stream(void *sctrl)
{
  int32_t rc = 0;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  struct sensorb_cfg_data cfg;

  SHIGH("Sensor stream OFF for %s \n",
    lib->sensor_lib_ptr->sensor_slave_info.sensor_name);

  if (ctrl->s_data->fd < 0) {
    SERR("stop_stream failed invalid fd = %d", ctrl->s_data->fd);
    return SENSOR_FAILURE;
  }
  ctrl->s_data->lock_aec = 0;
  ctrl->s_data->cur_res = SENSOR_INVALID_RESOLUTION;
  ctrl->s_data->pd_x_win_num = 0;
  ctrl->s_data->pd_y_win_num = 0;

  rc = sensor_write_i2c_setting_array(ctrl,
    &(ctrl->lib_params->sensor_lib_ptr->stop_settings));
  if (rc)
    SERR("sensor_write_i2c_setting_array failed");

  SLOW("exit");
  return rc;
}

static int32_t sensor_get_fast_aec_wait_frames(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  uint32_t i = 0, size = 0;
  struct sensor_lib_out_info_array    *out_info_array;
  struct sensor_lib_out_info_t        *out_info;
  boolean is_hfr = FALSE;
  sensor_output_format_t output_format;
  uint16_t sensor_num_fast_aec_frame_skip;
  char value[PROPERTY_VALUE_MAX];

  if (!ctrl) {
    SERR("Invalid params %p", ctrl);
    return SENSOR_FAILURE;
  }

  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  uint16_t *wait_frame_count = (uint16_t *)data;
  SLOW("enter");
  if (!wait_frame_count) {
    SERR("Invalid params %p or %p", ctrl, wait_frame_count);
    return SENSOR_FAILURE;
  }

  out_info_array = &lib->sensor_lib_ptr->out_info_array;
  size = lib->sensor_lib_ptr->out_info_array.size;

  for (i = 0; i < size; i++) {
    out_info = &out_info_array->out_info[i];
    if (out_info->mode & SENSOR_HFR_MODE) {
      is_hfr = TRUE;
      break;
    }
  }

  output_format = lib->sensor_lib_ptr->sensor_output.output_format;
  sensor_num_fast_aec_frame_skip =
   lib->sensor_lib_ptr->sensor_num_fast_aec_frame_skip;

#ifdef __ANDROID__
  property_get("persist.vendor.camera.fastaec", value, "0");
  if (atoi(value) > 0) {
    SHIGH("FASTAEC mode %d", atoi(value));
    sensor_num_fast_aec_frame_skip = atoi(value);
  }
#endif

  if ((output_format == SENSOR_BAYER) &&
      (sensor_num_fast_aec_frame_skip) &&
      (is_hfr == TRUE))
    *wait_frame_count = sensor_num_fast_aec_frame_skip;
  else
    *wait_frame_count = 0;

  SHIGH("exit: wait_frame_count %d", *wait_frame_count);
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_get_digital_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_get_digital_gain(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  /*sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;*/
  float *digital_gain= (float *)data;
  SLOW("enter");
  if (!ctrl || !digital_gain) {
    SERR("Invalid params %p or %p", ctrl, digital_gain);
    return SENSOR_FAILURE;
  }
  *digital_gain = ctrl->s_data->digital_gain;
  SLOW("digital gain %f", *digital_gain);
  SLOW("exit");
  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_get_aec_digital_gain -
 *
 * DESCRIPTION: Digital gain estimate  from aec data
 *==========================================================================*/
static int32_t sensor_get_aec_digital_gain(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_digital_gain_t *gain_info = (sensor_digital_gain_t *)data;
  SLOW("enter");
  if (!ctrl || !ctrl->lib_params || !gain_info || !gain_info->aec_update) {
    SERR("Invalid params %p or %p", ctrl, gain_info);
    return SENSOR_FAILURE;
  }
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  sensor_exposure_info_t exp_info;
  if (lib->sensor_lib_ptr->exposure_func_table.calc_exp_array_type ==
    CALC_CUSTOM_IN_LIB &&
    lib->sensor_lib_ptr->exposure_func_table.sensor_calculate_exposure) {
    lib->sensor_lib_ptr->exposure_func_table.sensor_calculate_exposure(
       gain_info->aec_update->real_gain, gain_info->aec_update->linecount,
       &exp_info, gain_info->aec_update->s_real_gain);
  } else {
    sensor_cmn_calc_exposure(lib->sensor_lib_ptr,
      gain_info->aec_update->real_gain, gain_info->aec_update->linecount,
      &exp_info, gain_info->aec_update->s_real_gain);
  }
  gain_info->digital_gain = exp_info.digital_gain;
  SLOW("digital gain %f", gain_info->digital_gain);
  SLOW("exit");
  return SENSOR_SUCCESS;
}

/** sensor_get_sensor_format: Get sensor format
 *
 *  @sctrl: sensor control structure
 *  @data: pointer to
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function returns sensor format **/

static int32_t sensor_get_sensor_format(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = NULL;
  sensor_output_format_t *output_format = (sensor_output_format_t *)data;
  SLOW("enter");
  if (!ctrl || !output_format) {
    SERR("Invalid params %p or %p", ctrl, output_format);
    return SENSOR_FAILURE;
  }
  lib = (sensor_lib_params_t *)ctrl->lib_params;
  if (!lib || !lib->sensor_lib_ptr) {
    SERR("Invalid params : %p = ctrl->lib_params", lib);
    return SENSOR_FAILURE;
  }

  *output_format = lib->sensor_lib_ptr->sensor_output.output_format;
  SLOW("output_format %d", *output_format);
  SLOW("exit");
  return SENSOR_SUCCESS;
}

/** sensor_get_property: Get sensor property
 *
 *  @sctrl: sensor control structure
 *  @data: pointer to sensor_property_t
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function returns lens info **/

static int32_t sensor_get_property(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = NULL;
  sensor_property_t *sensor_property = (sensor_property_t *)data;
  SLOW("enter");
  if (!ctrl || !sensor_property) {
    SERR("Invalid params %p or %p", ctrl, sensor_property);
    return SENSOR_FAILURE;
  }
  lib = (sensor_lib_params_t *)ctrl->lib_params;
  if (!lib || !lib->sensor_lib_ptr) {
    SERR("Invalid params");
    return SENSOR_FAILURE;
  }

  *sensor_property = lib->sensor_lib_ptr->sensor_property;
  SLOW("exit");
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_get_cur_csi_cfg -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_get_cur_csi_cfg(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_get_t  *sensor_get = (sensor_get_t *)data;
  uint32_t i = 0;
  if (!data) {
    SERR("data NULL");
    return SENSOR_FAILURE;
  }

  sensor_get->csi_cfg.csi_params =
    &ctrl->lib_params->sensor_lib_ptr->csi_params;
  sensor_get->csi_cfg.lut_params =
    &ctrl->lib_params->sensor_lib_ptr->csid_lut_params_array.lut_params[
    ctrl->s_data->cur_res];
  sensor_get->csi_cfg.data_rate =
    ctrl->lib_params->sensor_lib_ptr->out_info_array.out_info[
    ctrl->s_data->cur_res].data_rate;

  SLOW("csi params lane cnt %d settle cnt %x data rate %llu",
    sensor_get->csi_cfg.csi_params->lane_cnt,
    sensor_get->csi_cfg.csi_params->settle_cnt,
    sensor_get->csi_cfg.data_rate);

  for (i = 0; i < sensor_get->csi_cfg.lut_params->num_cid; i++) {
       if(sensor_get->csi_cfg.is_plain16) {
           sensor_get->csi_cfg.lut_params->vc_cfg_a[i].decode_format =
               CSI_DECODE_10BIT_PLAIN16_LSB;
       }
    SLOW("lut[%d] cid %d dt %x decode format %x", i,
      sensor_get->csi_cfg.lut_params->vc_cfg_a[i].cid,
      sensor_get->csi_cfg.lut_params->vc_cfg_a[i].dt,
      sensor_get->csi_cfg.lut_params->vc_cfg_a[i].decode_format);
  }
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_open -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_open(void **sctrl, void* data)
{
  int32_t          rc = SENSOR_SUCCESS;
  sensor_ctrl_t   *ctrl = NULL;
  char             subdev_string[32];
  sensor_submodule_info_t *info =
      (sensor_submodule_info_t *)data;

  if (!sctrl || !info || !info->data) {
    SERR("failed sctrl %p", sctrl);
    return SENSOR_ERROR_INVAL;
  }

  if (!strlen(info->intf_info[SUBDEV_INTF_PRIMARY].sensor_sd_name)) {
    SERR("failed: sensor_sd_name is 0 length");
    return SENSOR_ERROR_INVAL;
  }

  ctrl = malloc(sizeof(sensor_ctrl_t));
  if (!ctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  memset(ctrl, 0, sizeof(sensor_ctrl_t));

  ctrl->s_data = malloc(sizeof(sensor_data_t));
  if (!ctrl->s_data) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR1;
  }
  memset(ctrl->s_data, 0, sizeof(sensor_data_t));

  /* Set default fps value */
  ctrl->s_data->cur_fps = 30.0;

  /* Initialize mutex */
  pthread_mutex_init(&ctrl->s_data->mutex, NULL);

  /* Set default test pattern mode */
  ctrl->s_data->cur_test_pattern_mode = CAM_TEST_PATTERN_OFF;

  snprintf(subdev_string, sizeof(subdev_string), "/dev/%s",
      info->intf_info[SUBDEV_INTF_PRIMARY].sensor_sd_name);

  ctrl->s_data->sensor_common_info = (sensor_submod_common_info_t *)info->data;
  if (!ctrl->s_data->sensor_common_info) {
    SERR("failed ctrl->s_data->sensor_common_info is NULL");
    rc = SENSOR_FAILURE;
    goto ERROR2;
  }

  ctrl->lib_params = ctrl->s_data->sensor_common_info->sensor_lib_params;
  if (!ctrl->lib_params) {
    SERR("failed ctrl->lib_params is NULL");
    rc = SENSOR_FAILURE;
    goto ERROR2;
  }

  ctrl->s_data->sensor_common_info->output_format =
        ctrl->lib_params->sensor_lib_ptr->sensor_output.output_format;

  /* Open subdev */
  ctrl->s_data->fd = open(subdev_string, O_RDWR);
  if (ctrl->s_data->fd < 0) {
    SERR("failed");
    rc = SENSOR_FAILURE;
    goto ERROR2;
  }
  ctrl->session_count = 0;
  *sctrl = (void *)ctrl;

  ctrl->s_data->cur_res = SENSOR_INVALID_RESOLUTION;
  ctrl->s_data->hfr_mode = CAM_HFR_MODE_OFF;
  ctrl->s_data->hdr_mode = CAM_SENSOR_HDR_OFF;
  ctrl->s_data->delay_en = 1;

  SLOW("ctrl %p", ctrl);
  return rc;

ERROR2:
  free(ctrl->s_data);
ERROR1:
  free(ctrl);
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_set_lib_params -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_lib_params(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  if (!data) {
    SERR("data NULL");
    return SENSOR_FAILURE;
  }
  ctrl->lib_params = (sensor_lib_params_t *)data;
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_set_resolution -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_resolution(void *sctrl, void *data)
{
  int32_t rc = SENSOR_SUCCESS, i = 0;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_set_res_t *set_res =
    (sensor_set_res_t *)data;
  sensor_set_res_cfg_t *res_cfg = &set_res->res_cfg;
  uint32_t width = 0, height = 0;
  int32_t res = SENSOR_INVALID_RESOLUTION;
  enum msm_camera_stream_type_t stream_type = MSM_CAMERA_STREAM_INVALID;
  struct sensor_lib_out_info_array *out_info_array = NULL;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  struct camera_i2c_reg_setting *reg_settings = NULL;
  struct msm_camera_i2c_reg_array *regs = NULL;
  struct sensor_lib_out_info_t *dimension = NULL;
  struct sensorb_cfg_data cfg;
  double fps_d = 0;

  if (!set_res || !res_cfg || !set_res->cfg_done_ptr) {
    SERR("failed - invalid input values");
    goto ERROR0;
  }

  width = res_cfg->width;
  height = res_cfg->height;
  ctrl->s_data->cur_fps = ctrl->s_data->max_fps;
  out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;

  if (((res_cfg->is_fast_aec_mode_on == FALSE) && (!width || !height))) {
    SERR("failed: width %d height %d", width, height);
    goto ERROR0;
  }

#ifdef ENABLE_DIS_MARGIN
  if ((res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO)) &&
      (ctrl->s_data->dis_enable != 0)) {
    /* Add 10% for DIS */
    width = (width * 11) / 10;
    height = (height * 11) / 10;
  }
#endif

  SHIGH("Requested:W*H:%d*%d, stream mask:%x, hfr mode:%d, Cur fps:%f",width,
    height,res_cfg->stream_mask,ctrl->s_data->hfr_mode, ctrl->s_data->cur_fps);

  rc = sensor_pick_resolution(sctrl, res_cfg, &res);
  if (rc < 0) {
    SERR("failed: sensor_pick_resolution rc %d", rc);
    goto ERROR0;
  }

  SHIGH("Curr Res ID:%d New Res ID:%d New FPS %f", ctrl->s_data->cur_res, res,
    ctrl->s_data->cur_fps);
  if ((res >= out_info_array->size) || (res == SENSOR_INVALID_RESOLUTION)) {
    if (out_info_array->out_info[MSM_SENSOR_RES_FULL].x_output >= width &&
        out_info_array->out_info[MSM_SENSOR_RES_FULL].y_output >= height) {
      res = MSM_SENSOR_RES_FULL;
      SHIGH("Hardcode res to %d", res);
    } else {
      SERR("Error: failed to find resolution requested w*h %d*%d fps %f",
        width, height, ctrl->s_data->cur_fps);
      goto ERROR0;
    }
  }

  ctrl->s_data->prev_gain = 0;
  ctrl->s_data->prev_linecount = 0;
  ctrl->s_data->current_gain = 0;
  ctrl->s_data->current_linecount = 0;
  ctrl->s_data->prev_s_reg_gain = 0;
  ctrl->s_data->prev_s_linecount = 0;
  ctrl->s_data->s_reg_gain = 0;
  ctrl->s_data->s_linecount = 0;
  ctrl->s_data->sensor_skip_counter = 0;

  ctrl->s_data->prev_m_reg_gain = 0;
  ctrl->s_data->prev_m_linecount = 0;
  ctrl->s_data->m_reg_gain = 0;
  ctrl->s_data->m_linecount = 0;

  if (ctrl->s_data->cur_res == res) {
    ctrl->s_data->cur_stream_mask = res_cfg->stream_mask;
    if (ctrl->s_data->cur_fps > out_info_array->out_info[res].max_fps) {
      SHIGH("set_fps=%f > max_fps=%f, capping to max",
        ctrl->s_data->cur_fps, out_info_array->out_info[res].max_fps);
      ctrl->s_data->cur_fps = out_info_array->out_info[res].max_fps;
      /* not returning failure here */
    }
    if (lib->sensor_lib_ptr->sensor_output.output_format == SENSOR_YCBCR) {
      if (!sensor_write_aec_settings(ctrl, ctrl->s_data->cur_stream_mask)) {
        SERR("sensor_write_aec_settings failed");
        return SENSOR_FAILURE;
      }
    }

    SLOW("same resolution, returning..");
    *set_res->cfg_done_ptr = 1;
    goto EXIT_1;
  }

  if (res > (int32_t)lib->sensor_lib_ptr->res_settings_array.size) {
    SERR("failed res %d max size %d",
      res, lib->sensor_lib_ptr->res_settings_array.size);
    goto ERROR0;
  }

  ctrl->s_data->last_updated_fll =
    ctrl->s_data->cur_frame_length_lines =
    out_info_array->out_info[res].frame_length_lines;
  ctrl->s_data->cur_line_length_pclk =
    out_info_array->out_info[res].line_length_pclk;
  /* Update fps divider if cur stream type is preview or video */
  if ((res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW) ||
    res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_VIDEO) ||
    res_cfg->stream_mask & (1 << CAM_STREAM_TYPE_DEPTH)) &&
    (lib->sensor_lib_ptr->sensor_output.output_format != SENSOR_YCBCR) &&
    (ctrl->s_data->cur_fps > 0)) {
    if (ctrl->s_data->cur_fps > out_info_array->out_info[res].max_fps) {
      SHIGH("set_fps=%f > max_fps=%f, capping to max",
        ctrl->s_data->cur_fps,
        out_info_array->out_info[res].max_fps);
      ctrl->s_data->cur_fps = out_info_array->out_info[res].max_fps;
      /* not returning failure here */
    }
    if (ctrl->s_data->hfr_mode != CAM_HFR_MODE_OFF ||
      res_cfg->is_fast_aec_mode_on) {
      ctrl->s_data->current_fps_div = 1.0;
    } else {
      ctrl->s_data->current_fps_div =
        out_info_array->out_info[res].max_fps / ctrl->s_data->cur_fps;
    }
  } else {
    ctrl->s_data->current_fps_div = 1.0;
  }

  if (res > lib->sensor_lib_ptr->out_info_array.size) {
    SERR("failed res %d max size %d",
      res, lib->sensor_lib_ptr->out_info_array.size);
      goto ERROR0;
  }

  SLOW("Done mode change");
  ctrl->s_data->prev_res = ctrl->s_data->cur_res;
  ctrl->s_data->cur_res = res;
  ctrl->s_data->cur_stream_mask = res_cfg->stream_mask;

EXIT_1:
  sensor_delay_init(sctrl);
  ctrl->expo.valid = 0;
  SLOW("Exit");
  return rc;
ERROR0:
  SERR("set resolution failed");
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_set_saturation -
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_saturation(void * sctrl , void *data)
{
  int32_t rc = 0;
  int32_t index = -1;
  sensor_effect_t mode = SENSOR_EFFECT_OFF;
  int32_t *saturation_level = (int32_t*)data;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct camera_i2c_reg_setting_array *settings = NULL;

  SHIGH("%s: SATURATION VALUE %d ", __func__, *saturation_level);

  index = sensor_get_effect_index(
    &(ctrl->lib_params->sensor_lib_ptr->effect_info), mode);
  if (index == -1) {
    SHIGH("No settings for mode %d", mode);
    return SENSOR_SUCCESS;
  }

  settings = &(ctrl->lib_params->sensor_lib_ptr->effect_info.effect_settings[
    index].settings);
  rc = sensor_write_i2c_setting_array(ctrl, settings);
  if (rc)
    SERR("sensor_write_i2c_setting_array failed");
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_set_sharpness -
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_sharpness(void * sctrl , void *data)
{
  int32_t rc = 0;
  int32_t index = -1;
  sensor_effect_t mode = SENSOR_EFFECT_OFF;
  int32_t *sharpness_level = (int32_t*)data;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct camera_i2c_reg_setting_array *settings = NULL;

  SHIGH("%s: SHARPNESS VALUE %d ", __func__, *sharpness_level);

  index = sensor_get_effect_index(
    &(ctrl->lib_params->sensor_lib_ptr->effect_info), mode);
  if (index == -1) {
    SHIGH("No settings for mode %d", mode);
    return SENSOR_SUCCESS;
  }

  settings = &(ctrl->lib_params->sensor_lib_ptr->effect_info.effect_settings[
    index].settings);
  rc = sensor_write_i2c_setting_array(ctrl, settings);
  if (rc)
    SERR("sensor_write_i2c_setting_array failed");
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_set_contrast
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_contrast(void * sctrl , void *data)
{
  int32_t rc = 0;
  int32_t index = -1;
  sensor_effect_t mode = SENSOR_EFFECT_OFF;
  int32_t *contrast_level = (int32_t*)data;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct camera_i2c_reg_setting_array *settings = NULL;

  SHIGH("%s: CONTRAST VALUE %d ", __func__, *contrast_level);

  index = sensor_get_effect_index(
    &(ctrl->lib_params->sensor_lib_ptr->effect_info), mode);
  if (index == -1) {
    SHIGH("No settings for mode %d", mode);
    return SENSOR_SUCCESS;
  }

  settings = &(ctrl->lib_params->sensor_lib_ptr->effect_info.effect_settings[
    index].settings);
  rc = sensor_write_i2c_setting_array(ctrl, settings);
  if (rc)
    SERR("sensor_write_i2c_setting_array failed");
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_set_autofocus
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_autofocus(void * sctrl , void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
   sensor_info_t * sensor_info = (sensor_info_t*)data;
   int32_t write_fd = 0;
   struct sensorb_cfg_data cfg;

   if (!ctrl || !sensor_info) {
     SERR("failed: ctrl %p sensor_info %p", ctrl, sensor_info);
     return -EINVAL;
   }

   write_fd = sensor_info->write_fd;
   if (write_fd <= 0) {
     SERR("failed: write_fd %d", write_fd);
     return -EINVAL;
   }

   cfg.cfgtype = CFG_SET_AUTOFOCUS;
   if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg, "af") < 0) {
      SERR("failed");
   }
   sensor_thread_msg_t msg;
   memset(&msg, 0x00, sizeof(sensor_thread_msg_t));
   msg.msgtype = SET_AUTOFOCUS;
   msg.stop_thread = FALSE;
   msg.fd = ctrl->s_data->fd;
   msg.module = sensor_info->module;
   msg.sessionid = sensor_info->session_id;
   ssize_t nwrite = 0;
   SLOW("write fd %d", write_fd);
   nwrite = write(write_fd, &msg, sizeof(sensor_thread_msg_t));
   if(nwrite < 0)
   {
     SERR("Writing into fd failed");
   }

  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_cancel_autofocus
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t __attribute__((unused)) sensor_cancel_autofocus(
  void * sctrl ,
  void __attribute__((unused)) *data)
{
   sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
   struct sensorb_cfg_data cfg;

   cfg.cfgtype = CFG_CANCEL_AUTOFOCUS;
   if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg, "cancel_af") < 0) {
      SERR("failed");
   }
   return SENSOR_SUCCESS;
}
/*==========================================================
 * FUNCTION    - sensor_set_iso100
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_iso100(void * sctrl , void *data)
{
  sensor_ctrl_t           *ctrl          = (sensor_ctrl_t *)sctrl;
  chromatix_3a_parms_type *chromatix_3a_ptr = (chromatix_3a_parms_type *)data;
  float                   iso100_gain, iso100_override;
  char                    prop[PROPERTY_VALUE_MAX];
  sensor_lib_params_t     *lib;

  RETURN_ERROR_ON_NULL(ctrl);

  lib = (sensor_lib_params_t *)ctrl->lib_params;
  if ((chromatix_3a_ptr) && (lib->sensor_lib_ptr->sensor_output.output_format != SENSOR_YCBCR))
  {
     iso100_gain = chromatix_3a_ptr->AEC_algo_data.aec_generic.ISO100_gain;
     if (iso100_gain == 0)
       iso100_gain = 1;
  }
  else{
     iso100_gain = 0.7;
     property_get("persist.vendor.camera.iso100", prop, "0.0");
     iso100_override = atof(prop);
     if(iso100_override > 0.0) {
       SHIGH("setprop override iso100_gain %f ==> iso100_override %f",
             iso100_gain, iso100_override);
       iso100_gain = iso100_override;
     }
  }

  ctrl->s_data->iso100_gain = iso100_gain;
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_set_iso
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_iso(void * sctrl , void *data)
{
  int32_t rc = 0;
  int32_t index = -1;
  sensor_effect_t mode = SENSOR_EFFECT_OFF;
  int32_t *iso_level = (int32_t*)data;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct camera_i2c_reg_setting_array *settings = NULL;
  SHIGH("iso_level %d ", *iso_level);

  if (ctrl->s_data->sensor_common_info->sensor_lib_params->
    sensor_lib_ptr->sensor_output.output_format == SENSOR_BAYER) {

    boolean iso_mode_set = TRUE;
    sensor_special_mode special_mode = SENSOR_SPECIAL_MODE_ISO_100;

    switch(*iso_level) {
      case CAM_ISO_MODE_100:
        special_mode = SENSOR_SPECIAL_MODE_ISO_100;
        break;
      case CAM_ISO_MODE_200:
        special_mode = SENSOR_SPECIAL_MODE_ISO_200;
        break;
      case CAM_ISO_MODE_400:
        special_mode = SENSOR_SPECIAL_MODE_ISO_400;
        break;
      case CAM_ISO_MODE_800:
        special_mode = SENSOR_SPECIAL_MODE_ISO_800;
        break;
      case CAM_ISO_MODE_1600:
        special_mode = SENSOR_SPECIAL_MODE_ISO_1600;
        break;
      default:
        iso_mode_set = FALSE;
        SHIGH("Unsupported ISO mode = %d", *iso_level);
        break;
    }

    sensor_util_set_special_mode(ctrl->s_data->sensor_common_info,
      special_mode, iso_mode_set);
  } else {
    index = sensor_get_effect_index(
      &(ctrl->lib_params->sensor_lib_ptr->effect_info), mode);
    if (index == -1) {
      SHIGH("No settings for mode %d", mode);
      return SENSOR_SUCCESS;
    }

    settings = &(ctrl->lib_params->sensor_lib_ptr->effect_info.effect_settings[
      index].settings);
    rc = sensor_write_i2c_setting_array(ctrl, settings);
    if (rc)
      SERR("sensor_write_i2c_setting_array failed");
  }
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_set_manual_exposure_time
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_manual_exposure_time(void * sctrl , void *data)
{
  int32_t rc = 0;
  int64_t *exposure_time = (int64_t*)data;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct camera_i2c_reg_setting exp_settings;

  SHIGH("%s: exposure_time %d ", __func__, *exposure_time);

  if (ctrl->lib_params->sensor_lib_ptr->sensor_output.is_depth_sensor) {
    if (!ctrl->lib_params->sensor_lib_ptr->exposure_func_table.
      sensor_fill_exp_array_from_value) {
      SERR("Varying exposure is not supoprted in this depth sensor");
      return SENSOR_SUCCESS;
    }
    /*This is a depth sensor which controls exposure through direct register writes*/
    memset(&exp_settings, 0, sizeof(exp_settings));
    /* Allocate the register memory once rather than
       everytime to update fps setting */
    if (ctrl->s_data->exposure_regs== NULL) {
      ctrl->s_data->exposure_regs = malloc(MAX_EXPOSURE_REGISTERS *
        sizeof(struct msm_camera_i2c_reg_array));
      if (!ctrl->s_data->exposure_regs) {
        SERR("failed");
        return -ENOMEM;
      }
    }
    exp_settings.reg_setting = ctrl->s_data->exposure_regs;

    rc = ctrl->lib_params->sensor_lib_ptr->exposure_func_table.
      sensor_fill_exp_array_from_value(*exposure_time, &exp_settings,
      &ctrl->s_data->current_linecount, &ctrl->s_data->cur_frame_length_lines);
    if ((rc < 0) || (exp_settings.size == 0) ||
      (exp_settings.size > MAX_EXPOSURE_REGISTERS)) {
      SERR("failed rc = %d exp_settings.size = %d", rc, exp_settings.size);
    } else {
      rc = sensor_write_i2c_setting(ctrl, &exp_settings);
      if (rc < 0) {
        SERR("failed");
        return -EIO;
      }
    }
    ctrl->s_data->last_updated_fll = ctrl->s_data->cur_frame_length_lines;
  }
  return rc;
}


/*==========================================================
 * FUNCTION    - sensor_set_exposure_compensation
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_exposure_compensation(void * sctrl , void *data)
{
  int32_t rc = 0;
  int32_t index = -1;
  sensor_effect_t mode = SENSOR_EFFECT_OFF;
  int32_t *exposure_comp_level = (int32_t*)data;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct camera_i2c_reg_setting_array *settings = NULL;

  SHIGH("%s: exposure_comp_level %d ", __func__, *exposure_comp_level);

  ctrl->s_data->ev_gain = 1.0f;
  if((ctrl->s_data->sensor_common_info->sensor_lib_params->
    sensor_lib_ptr->sensor_output.output_format == SENSOR_BAYER &&
    ctrl->s_data->hdr_mode == CAM_SENSOR_3EXP_HDR_IN_SENSOR)) {
    if(*exposure_comp_level <= 0) {
      ctrl->s_data->ev_gain = 1.0f;
    } else if(*exposure_comp_level > 0) {
      ctrl->s_data->ev_gain = (float)*exposure_comp_level / 4.0f + 1.0f;
    }
    SHIGH("3HDR EV case, ev_gain = %f", ctrl->s_data->ev_gain);
    return rc;
  } else {
    index = sensor_get_effect_index(
      &(ctrl->lib_params->sensor_lib_ptr->effect_info), mode);
    if (index == -1) {
      SHIGH("No settings for mode %d", mode);
      return SENSOR_SUCCESS;
    }

    settings = &(ctrl->lib_params->sensor_lib_ptr->effect_info.effect_settings[
      index].settings);
    rc = sensor_write_i2c_setting_array(ctrl, settings);
    if (rc)
      SERR("sensor_write_i2c_setting_array failed");
    return rc;
  }
}

/*==========================================================
 * FUNCTION    - sensor_set_antibanding
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_antibanding(void * sctrl , void *data)
{
  int32_t rc = 0;
  int32_t index = -1;
  sensor_effect_t mode = SENSOR_EFFECT_OFF;
  int32_t *antibanding_level = (int32_t*)data;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct camera_i2c_reg_setting_array *settings = NULL;

  SHIGH("%s: antibanding_level %d ", __func__, *antibanding_level);

  index = sensor_get_effect_index(
    &(ctrl->lib_params->sensor_lib_ptr->effect_info), mode);
  if (index == -1) {
    SHIGH("No settings for mode %d", mode);
    return SENSOR_SUCCESS;
  }

  settings = &(ctrl->lib_params->sensor_lib_ptr->effect_info.effect_settings[
    index].settings);
  rc = sensor_write_i2c_setting_array(ctrl, settings);
  if (rc)
    SERR("sensor_write_i2c_setting_array failed");
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_set_effect
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_effect(void * sctrl , void *data)
{
  int32_t rc = 0;
  int32_t index = -1;
  sensor_effect_t mode = SENSOR_EFFECT_OFF;
  int32_t *effect_mode = (int32_t*)data;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct camera_i2c_reg_setting_array *settings = NULL;

  SHIGH("%s: effect mode %d ", __func__, *effect_mode);

  index = sensor_get_effect_index(
    &(ctrl->lib_params->sensor_lib_ptr->effect_info), mode);
  if (index == -1) {
    SHIGH("No settings for mode %d", mode);
    return SENSOR_SUCCESS;
  }

  settings = &(ctrl->lib_params->sensor_lib_ptr->effect_info.effect_settings[
    index].settings);
  rc = sensor_write_i2c_setting_array(ctrl, settings);
  if (rc)
    SERR("sensor_write_i2c_setting_array failed");
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_set_white_balance
 *
 * DESCRIPTION:
 *==========================================================*/

static int32_t sensor_set_white_balance(void * sctrl , void *data)
{
  int32_t rc = 0;
  int32_t index = -1;
  sensor_effect_t mode = SENSOR_EFFECT_OFF;
  int32_t *wb_mode = (int32_t*)data;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct camera_i2c_reg_setting_array *settings = NULL;

  SHIGH("%s: white balance mode %d ", __func__, *wb_mode);

  index = sensor_get_effect_index(
    &(ctrl->lib_params->sensor_lib_ptr->effect_info), mode);
  if (index == -1) {
    SHIGH("No settings for mode %d", mode);
    return SENSOR_SUCCESS;
  }

  settings = &(ctrl->lib_params->sensor_lib_ptr->effect_info.effect_settings[
    index].settings);
  rc = sensor_write_i2c_setting_array(ctrl, settings);
  if (rc)
    SERR("sensor_write_i2c_setting_array failed");
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_get_cur_chromatix_name_for_type
 *
 * index_array[0] = {selected_sensor_resolution_index, selected_special_mode}
 * index_array[1] = {0xFF, selected_special_mode} (0xFF mean CommonChromatix)
 * index_array[2] = {selected_sensor_resolution_index, 0} {no special mode}
 * index_array[2] = {0xFF, 0} {default set}
 *
 * DESCRIPTION:
 *==========================================================*/
static char * sensor_get_cur_chromatix_name_for_type(
  sensor_data_t *s_data, camera_module module,
  sensor_chroamtix_type stream_type)
{
  uint32_t                 i = 0;
  uint32_t                 j = 0;
  char                    *name = NULL;
  module_chromatix_name_t *chromatix_name = NULL;
  module_chromatix_info_t *chromatix_info =
    &s_data->sensor_common_info->camera_config.chromatix_info;
  uint64_t                 special_mode_mask =
    s_data->sensor_common_info->special_mode_mask;
  uint8_t                  res_idx =
    (s_data->cur_res == SENSOR_INVALID_RESOLUTION) ? 0 : s_data->cur_res;
  uint64_t                 index_array[4][2] =
    {
      {res_idx, special_mode_mask}, {0xFF, special_mode_mask},
      {res_idx, 0}, {0xFF, 0},
    };

  for (j = 0; j < 4; j++) {
    for (i = 0; i < chromatix_info->size; i++) {
      chromatix_name = &chromatix_info->chromatix_name[i];

      SLOW("index_array[%d][0] = %lld sensor_resolution_index = %d", j,
        index_array[j][0], chromatix_name->sensor_resolution_index);
      /* Check if the res_idx matches with the the one in chromatix array */
      if (index_array[j][0] != chromatix_name->sensor_resolution_index)
        continue;

      SLOW("index_array[%d][1] = %lld special_mode_mask = %lld", j,
        index_array[j][1], chromatix_name->special_mode_mask);

      /* Check if special_mode_mask matches with the one in chromatix array in xml file */
      if (chromatix_name->special_mode_type) {
        /* If special_mode_type is set, then the chroamtix modes have | (or) */
        if ((index_array[j][1] & chromatix_name->special_mode_mask) !=
          index_array[j][1])
          continue;
      }
      else if (index_array[j][1] != chromatix_name->special_mode_mask)
        continue;

      switch (module) {
      case CAMERA_MODULE_ISP:
        switch (stream_type) {
        case SENSOR_CHROMATIX_TYPE_PREVIEW:
          name = chromatix_name->isp_preview;
          break;
        case SENSOR_CHROMATIX_TYPE_SNAPSHOT:
          name = chromatix_name->isp_snapshot;
          break;
        case SENSOR_CHROMATIX_TYPE_VIDEO:
          name = chromatix_name->isp_video;
          break;
        case SENSOR_CHROMATIX_TYPE_COMMON:
          name = chromatix_name->isp_common;
          break;
        default:
          SERR("Invalid stream type = %d", stream_type);
          break;
        }
        break;
      case CAMERA_MODULE_CPP:
        switch (stream_type) {
        case SENSOR_CHROMATIX_TYPE_PREVIEW:
          name = chromatix_name->cpp_preview;
          break;
        case SENSOR_CHROMATIX_TYPE_SNAPSHOT:
          name = chromatix_name->cpp_snapshot;
          break;
        case SENSOR_CHROMATIX_TYPE_VIDEO:
          name = chromatix_name->cpp_video;
          break;
        case SENSOR_CHROMATIX_TYPE_LIVESHOT:
          name = chromatix_name->cpp_liveshot;
          break;
        default:
          SERR("Invalid stream type = %d", stream_type);
          break;
        }
        break;
      case CAMERA_MODULE_3A:
        switch (stream_type) {
        case SENSOR_CHROMATIX_TYPE_PREVIEW:
        case SENSOR_CHROMATIX_TYPE_SNAPSHOT:
          name = chromatix_name->a3_preview;
          break;
        case SENSOR_CHROMATIX_TYPE_VIDEO:
        case SENSOR_CHROMATIX_TYPE_LIVESHOT:
          name = chromatix_name->a3_video;
          break;
        default:
          SERR("Invalid stream type = %d", stream_type);
          break;
        }
        break;
      case CAMERA_MODULE_SW_PPROC:
        name = chromatix_name->postproc;
        break;
      case CAMERA_MODULE_EXTERNAL:
        name = chromatix_name->external;
        break;
      case CAMERA_MODULE_IOT:
        name = chromatix_name->iot;
        break;
      default:
        SERR("Invalid module = %d", module);
        break;
      }

      if ((name != NULL) && strlen(name)) {
        SLOW("name = %s", name);
        return name;
      }
      else if (special_mode_mask == 0 && j == 1)
        goto CHROMATIX_NULL;
    }
  }

CHROMATIX_NULL:
  SHIGH("chromatix for res_idx = %d special_mode_mask = %lld \
    module = %d chromatix_type = %d is NULL",
    res_idx, special_mode_mask, module, stream_type);
  return NULL;
}

/*==========================================================
 * FUNCTION    - sensor_get_cur_chromatix_name -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_get_cur_chromatix_name(void *sctrl, void *data)
{
  sensor_ctrl_t               *ctrl;
  sensor_chromatix_params_t   *param;
  char                        **name;
  module_chromatix_name_t     *chromatix_name = NULL;
  sensor_submod_common_info_t *sensor_common_info = NULL;
  int16_t cur_res, i;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  ctrl = (sensor_ctrl_t *)sctrl;
  param = (sensor_chromatix_params_t *)data;
  name = param->chromatix_lib_name;

  cur_res = (ctrl->s_data->cur_res <= SENSOR_INVALID_RESOLUTION) ?
    0 : ctrl->s_data->cur_res;
  if (cur_res >
    ctrl->s_data->sensor_common_info->camera_config.chromatix_info.size ||
    cur_res >= MAX_CHROMATIX_ARRAY) {
    SERR("failed cur res %d max size %d", cur_res,
      ctrl->s_data->sensor_common_info->camera_config.chromatix_info.size);
    return SENSOR_ERROR_INVAL;
  }

  sensor_common_info = ctrl->s_data->sensor_common_info;
  chromatix_name =
    &(sensor_common_info->camera_config.chromatix_info.chromatix_name[cur_res]);

  SLOW("special_mode_mask = %lld", sensor_common_info->special_mode_mask);

  for (i = 0; i < SENSOR_CHROMATIX_MAX; i++)
    name[i] = NULL;

  name[SENSOR_CHROMATIX_ISP_COMMON] =
    sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_ISP,
      SENSOR_CHROMATIX_TYPE_COMMON);

  name[SENSOR_CHROMATIX_ISP] = name[SENSOR_CHROMATIX_ISP_SNAPSHOT] =
    sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_ISP,
      SENSOR_CHROMATIX_TYPE_SNAPSHOT);

  name[SENSOR_CHROMATIX_CPP_PREVIEW] =
    sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_CPP,
      SENSOR_CHROMATIX_TYPE_PREVIEW);

  name[SENSOR_CHROMATIX_CPP_SNAPSHOT] =
    sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_CPP,
      SENSOR_CHROMATIX_TYPE_SNAPSHOT);

  sensor_util_set_special_mode(ctrl->s_data->sensor_common_info,
    SENSOR_SPECIAL_MODE_FLASH, TRUE);
  name[SENSOR_CHROMATIX_CPP_FLASH_SNAPSHOT] =
    sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_CPP,
      SENSOR_CHROMATIX_TYPE_SNAPSHOT);
  sensor_util_set_special_mode(ctrl->s_data->sensor_common_info,
    SENSOR_SPECIAL_MODE_FLASH, FALSE);

  sensor_util_set_special_mode(ctrl->s_data->sensor_common_info,
    SENSOR_SPECIAL_MODE_OIS_CAPTURE, TRUE);
  name[SENSOR_CHROMATIX_CPP_OIS_SNAPSHOT] =
    sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_CPP,
      SENSOR_CHROMATIX_TYPE_SNAPSHOT);
  sensor_util_set_special_mode(ctrl->s_data->sensor_common_info,
    SENSOR_SPECIAL_MODE_OIS_CAPTURE, FALSE);

  sensor_util_set_special_mode(ctrl->s_data->sensor_common_info,
    SENSOR_SPECIAL_MODE_ZOOM_DOWNSCALE, TRUE);
  name[SENSOR_CHROMATIX_CPP_OIS_DS_SNAPSHOT] =
    sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_CPP,
      SENSOR_CHROMATIX_TYPE_SNAPSHOT);
  sensor_util_set_special_mode(ctrl->s_data->sensor_common_info,
    SENSOR_SPECIAL_MODE_ZOOM_DOWNSCALE, FALSE);

  sensor_util_set_special_mode(ctrl->s_data->sensor_common_info,
    SENSOR_SPECIAL_MODE_ZOOM_UPSCALE, TRUE);
  name[SENSOR_CHROMATIX_CPP_OIS_US_SNAPSHOT] =
    sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_CPP,
      SENSOR_CHROMATIX_TYPE_SNAPSHOT);
  sensor_util_set_special_mode(ctrl->s_data->sensor_common_info,
    SENSOR_SPECIAL_MODE_ZOOM_UPSCALE, FALSE);

  sensor_util_set_special_mode(ctrl->s_data->sensor_common_info,
    SENSOR_SPECIAL_MODE_ZOOM_DOWNSCALE, TRUE);
  name[SENSOR_CHROMATIX_CPP_DS] =
    sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_CPP,
      SENSOR_CHROMATIX_TYPE_SNAPSHOT);
  sensor_util_set_special_mode(ctrl->s_data->sensor_common_info,
    SENSOR_SPECIAL_MODE_ZOOM_DOWNSCALE, FALSE);

  sensor_util_set_special_mode(ctrl->s_data->sensor_common_info,
    SENSOR_SPECIAL_MODE_ZOOM_UPSCALE, TRUE);
  name[SENSOR_CHROMATIX_CPP_US] =
    sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_CPP,
      SENSOR_CHROMATIX_TYPE_SNAPSHOT);
  sensor_util_set_special_mode(ctrl->s_data->sensor_common_info,
    SENSOR_SPECIAL_MODE_ZOOM_UPSCALE, FALSE);

  if (sensor_common_info->ir_mode)
    sensor_util_set_special_mode(ctrl->s_data->sensor_common_info,
      SENSOR_SPECIAL_MODE_IR, TRUE);

  name[SENSOR_CHROMATIX_CPP_VIDEO] =
    sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_CPP,
      SENSOR_CHROMATIX_TYPE_VIDEO);

  name[SENSOR_CHROMATIX_SW_PPROC] =
    sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_SW_PPROC,
      SENSOR_CHROMATIX_TYPE_COMMON);

  name[SENSOR_CHROMATIX_3A] =
    sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_3A,
      SENSOR_CHROMATIX_TYPE_PREVIEW);

  name[SENSOR_CHROMATIX_EXTERNAL] =
    sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_EXTERNAL,
      SENSOR_CHROMATIX_TYPE_COMMON);

  name[SENSOR_CHROMATIX_IOT] =
    sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_IOT,
      SENSOR_CHROMATIX_TYPE_COMMON);

  if (ctrl->s_data->sensor_common_info->sensor_mode == SENSOR_MODE_VIDEO) {
    name[SENSOR_CHROMATIX_ISP] =
      sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_ISP,
        SENSOR_CHROMATIX_TYPE_VIDEO);
    name[SENSOR_CHROMATIX_CPP_SNAPSHOT] =
      sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_CPP,
        SENSOR_CHROMATIX_TYPE_LIVESHOT);
    name[SENSOR_CHROMATIX_3A] =
      sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_3A,
        SENSOR_CHROMATIX_TYPE_VIDEO);
  } else if(ctrl->s_data->sensor_common_info->sensor_mode ==
    SENSOR_MODE_PREVIEW) {
    name[SENSOR_CHROMATIX_ISP] =
      sensor_get_cur_chromatix_name_for_type(ctrl->s_data, CAMERA_MODULE_ISP,
        SENSOR_CHROMATIX_TYPE_PREVIEW);
  }

  for (i = 0; i < SENSOR_CHROMATIX_MAX; i++)
    if (name[i]) {
      SLOW("res_idx = %d chromatix_lib_name[%d] = %s",cur_res, i, name[i]);
    } else {
      SLOW("res_idx = %d chromatix_lib_name[%d] = %s",cur_res, i, "NULL");
    }

  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_get_capabilities -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_get_capabilities(void *slib, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)slib;
  mct_pipeline_sensor_cap_t *sensor_cap = (mct_pipeline_sensor_cap_t *)data;
  uint32_t i = 0, size = 0, j=0;
  struct sensor_lib_out_info_t *out_info = NULL;
  sensor_dimension_t scale_tbl[MAX_SCALE_SIZES_CNT];
  struct sensor_crop_parms_t *crop_params = NULL;
  uint32_t pix_fmt_fourcc = 0;
  sensor_optical_black_region_t *optical_black_region_ptr = NULL;
  uint8_t  num_meta_channel = 0;

  if (!sensor_cap || !lib || !lib->sensor_lib_ptr) {
      SERR("failed, %p, %p, %p",
        sensor_cap,
        lib,
        lib ? lib->sensor_lib_ptr : NULL);
    return SENSOR_ERROR_INVAL;
  }

  if (!lib->sensor_lib_ptr->out_info_array.size) {
    SERR("failed");
    return SENSOR_ERROR_INVAL;
  }

  memset(sensor_cap, 0, sizeof(*sensor_cap));

  if (lib->sensor_lib_ptr->sensor_capability) {
    /* The last bit : sensor internal use */
    sensor_cap->feature_mask =
      (lib->sensor_lib_ptr->sensor_capability & 0xFFFFFFFFFFFFFFFE);
  }

  if (lib->sensor_lib_ptr->sensor_output.output_format == SENSOR_YCBCR) {
    sensor_cap->ae_lock_supported = FALSE;
    sensor_cap->wb_lock_supported = FALSE;
    sensor_cap->scene_mode_supported = FALSE;
  }
  else {
    sensor_cap->ae_lock_supported = TRUE;
    sensor_cap->wb_lock_supported = TRUE;
    sensor_cap->scene_mode_supported = TRUE;
  }

  if (lib->sensor_lib_ptr->out_info_array.size > SENSOR_MAX_RESOLUTION) {
    SERR("out info array %d > MAX(%d), so making the size equal to MAX",
      lib->sensor_lib_ptr->out_info_array.size,SENSOR_MAX_RESOLUTION);
    size = SENSOR_MAX_RESOLUTION;
  } else {
    size = lib->sensor_lib_ptr->out_info_array.size;
  }
  out_info = lib->sensor_lib_ptr->out_info_array.out_info;
  sensor_cap->dim_fps_table_count = 0;
  for (i = 0; i < size; i++) {
    sensor_cap->dim_fps_table[i].dim.width = out_info[i].x_output;
    if (lib->sensor_lib_ptr->sensor_output.output_format == SENSOR_YCBCR)
      sensor_cap->dim_fps_table[i].dim.width /= 2;
    sensor_cap->dim_fps_table[i].dim.height = out_info[i].y_output;

    sensor_cap->dim_fps_table[i].fps.min_fps = out_info[i].min_fps;
    sensor_cap->dim_fps_table[i].fps.max_fps = out_info[i].max_fps;
    sensor_cap->dim_fps_table[i].fps.video_min_fps =
      sensor_cap->dim_fps_table[i].fps.min_fps;
    sensor_cap->dim_fps_table[i].fps.video_max_fps =
      sensor_cap->dim_fps_table[i].fps.max_fps;
    sensor_cap->dim_fps_table_count++;
    sensor_cap->dim_fps_table[i].mode  = out_info[i].mode;

   /* Max op pixel clock*/
   sensor_cap->op_pixel_clk = MAXQ(sensor_cap->op_pixel_clk,
     out_info[i].op_pixel_clk);
  }

  /* Active array and Pixel Array*/
  sensor_cap->active_array_size.width =
    lib->sensor_lib_ptr->pixel_array_size_info.active_array_size.width;
  sensor_cap->active_array_size.height =
    lib->sensor_lib_ptr->pixel_array_size_info.active_array_size.height;
  sensor_cap->active_array_size.left =
    lib->sensor_lib_ptr->pixel_array_size_info.left_dummy;
  sensor_cap->active_array_size.top =
    lib->sensor_lib_ptr->pixel_array_size_info.top_dummy;

  sensor_cap->pixel_array_size.width =
   lib->sensor_lib_ptr->pixel_array_size_info.active_array_size.width +
   lib->sensor_lib_ptr->pixel_array_size_info.left_dummy +
   lib->sensor_lib_ptr->pixel_array_size_info.right_dummy;
  sensor_cap->pixel_array_size.height =
   lib->sensor_lib_ptr->pixel_array_size_info.active_array_size.height +
   lib->sensor_lib_ptr->pixel_array_size_info.top_dummy +
   lib->sensor_lib_ptr->pixel_array_size_info.bottom_dummy;

  sensor_cap->sensor_physical_size[0] =
    (float)sensor_cap->pixel_array_size.width *
    lib->sensor_lib_ptr->sensor_property.pix_size / 1000;
  sensor_cap->sensor_physical_size[1] =
    (float)sensor_cap->pixel_array_size.height *
    lib->sensor_lib_ptr->sensor_property.pix_size / 1000;

  /* raw fmts */
  sensor_cap->supported_raw_fmts_cnt = 0;

  for (j = 0; j < lib->sensor_lib_ptr->sensor_stream_info_array.size; j++)
  {
    sensor_stream_info_t *sensor_stream_info =
        &(lib->sensor_lib_ptr->sensor_stream_info_array.sensor_stream_info[j]);
    uint32_t num_cid_ch = (uint32_t)sensor_stream_info->vc_cfg_size;
    for (i = 0; i < num_cid_ch; i++) {
      pix_fmt_fourcc = sensor_util_get_fourcc_format(
        sensor_stream_info->pix_data_fmt[i],
        lib->sensor_lib_ptr->sensor_output.filter_arrangement,
        sensor_stream_info->vc_cfg[i].decode_format);

      if (pix_fmt_fourcc == 0) {
        SERR("failed");
        return SENSOR_ERROR_INVAL;
      }

      sensor_cap->supported_raw_fmts[sensor_cap->supported_raw_fmts_cnt++] =
        sensor_util_get_hal_format(pix_fmt_fourcc);

      if (j == 0 && pix_fmt_fourcc == MSM_V4L2_PIX_FMT_META) {
        /* Read only the 0 th set because this has all the meta streams info */
        /* Validate dimensions */
        if (!(sensor_stream_info->vc_cfg[i].meta_dim.width &&
            sensor_stream_info->vc_cfg[i].meta_dim.height)) {

          SINFO("Invalid dimensions width = %d height = %d",
            sensor_stream_info->vc_cfg[i].meta_dim.width,
            sensor_stream_info->vc_cfg[i].meta_dim.height);
        }

        sensor_cap->meta_chnl_info[num_meta_channel].vc =
          sensor_stream_info->vc_cfg[i].cid / 4;
        sensor_cap->meta_chnl_info[num_meta_channel].dt =
          sensor_stream_info->vc_cfg[i].dt;
        sensor_cap->meta_chnl_info[num_meta_channel].width =
          sensor_stream_info->vc_cfg[i].meta_dim.width;
        sensor_cap->meta_chnl_info[num_meta_channel].height =
          sensor_stream_info->vc_cfg[i].meta_dim.height;

        switch (sensor_stream_info->vc_cfg[i].decode_format) {
        case CSI_DECODE_10BIT:
          sensor_cap->meta_chnl_info[num_meta_channel].fmt =
            CAM_FORMAT_META_RAW_10BIT;
        break;
        case CSI_DECODE_8BIT:
          sensor_cap->meta_chnl_info[num_meta_channel].fmt =
            CAM_FORMAT_META_RAW_8BIT;
          break;
        default:
          SERR("Invalid decode_format %d",
            sensor_stream_info->vc_cfg[i].decode_format);
          return SENSOR_ERROR_INVAL;
        }

        switch (sensor_stream_info->vc_cfg[i].stats_type) {
        case HDR_STATS:
          sensor_cap->meta_chnl_info[num_meta_channel].sub_fmt =
            CAM_FORMAT_SUBTYPE_HDR_STATS;
          break;

        case PD_STATS:
          sensor_cap->meta_chnl_info[num_meta_channel].sub_fmt =
            CAM_FORMAT_SUBTYPE_PDAF_STATS;
          break;

        case FLICKER_STATS:
          sensor_cap->meta_chnl_info[num_meta_channel].sub_fmt =
            CAM_FORMAT_SUBTYPE_FLICKER_STATS;
          break;

        default:
          SERR("Invalid stats_type %d",
            sensor_stream_info->vc_cfg[i].stats_type);
          return SENSOR_ERROR_INVAL;
        }

        num_meta_channel++;
      }
    }
  }

  sensor_cap->num_meta_channels = num_meta_channel;

  /*preview fmts*/
  sensor_cap->supported_preview_fmt_cnt = 4;
  sensor_cap->supported_preview_fmts[0] = CAM_FORMAT_YUV_420_NV21;
  sensor_cap->supported_preview_fmts[1] = CAM_FORMAT_YUV_420_YV12;
  sensor_cap->supported_preview_fmts[2] = CAM_FORMAT_YUV_420_NV12_VENUS;
  sensor_cap->supported_preview_fmts[3] = CAM_FORMAT_YUV_420_NV21_VENUS;

  /* Fill frame control info for Bayer sensor from sensor resolution table
   * -exposure information - min of maximum available modes and
   * max of min available modes
   * -maximum frame duration - min of maximum available durations
   */
  sensor_cap->min_exp_time = 0;
  sensor_cap->max_exp_time = (uint64_t)~0;
  sensor_cap->max_frame_duration = ~(((int64_t)1)<<63) ;
  if (lib->sensor_lib_ptr->sensor_output.output_format == SENSOR_BAYER) {
    uint64_t min = 0, max = 0, min_round_down = 0;
    int64_t max_duration = 0;
    uint16_t vert_offset = 0;
    uint32_t vt_pix_clk = 0, min_linecount = 0;

    if (lib->sensor_lib_ptr->exp_gain_info.vert_offset > 0)
      vert_offset = (uint16_t)(vert_offset
              + lib->sensor_lib_ptr->exp_gain_info.vert_offset);

    for (i = 0; i < size; i++) {
      vt_pix_clk = SENSOR_GET_VT_PIK_CLK(&out_info[i]);
      min = (uint64_t)((((float)out_info[i].line_length_pclk *
        NANO_SEC_PER_SEC) / (float)vt_pix_clk) + 0.5);
      min_linecount = lib->sensor_lib_ptr->aec_info.min_linecount;
      if (min_linecount > 0)
        min = min * min_linecount;
      min_round_down = (uint64_t)(((float)out_info[i].line_length_pclk *
        NANO_SEC_PER_SEC) / (float)vt_pix_clk);
      sensor_cap->min_exp_time = MTYPE_MAX(min, sensor_cap->min_exp_time);

      if (lib->sensor_lib_ptr->aec_info.max_linecount > 0) {
        max = min_round_down * lib->sensor_lib_ptr->aec_info.max_linecount;
        max_duration = (int64_t)(min_round_down *
          (lib->sensor_lib_ptr->aec_info.max_linecount + vert_offset));
      } else {
        max = (uint64_t)(min_round_down * out_info[i].frame_length_lines *
          MULTIPLY_FACTOR_FOR_MIN_FPS);
        max_duration = (int64_t)(min_round_down *
          (out_info[i].frame_length_lines *
          (uint64_t)MULTIPLY_FACTOR_FOR_MIN_FPS +
          vert_offset));
      }
      sensor_cap->max_exp_time = MTYPE_MIN(max, sensor_cap->max_exp_time);
      sensor_cap->max_frame_duration = MTYPE_MIN(max_duration,
        sensor_cap->max_frame_duration);
    }

    SLOW("min exp time %ju max exp time %ju", sensor_cap->min_exp_time,
      sensor_cap->max_exp_time);
    /* Gain of 1.0f correspond to ISO 100 */
    if (lib->sensor_lib_ptr->aec_info.max_gain < 1.0 ||
      lib->sensor_lib_ptr->aec_info.min_gain < 1.0) {
      SHIGH("Cannot determine sensitivity range for this sensor");
    } else {
      sensor_cap->max_gain =
        (uint32_t)(lib->sensor_lib_ptr->aec_info.max_gain *
        MULTIPLY_FACTOR_FOR_ISO);
      sensor_cap->max_analog_gain =
        (uint32_t)(lib->sensor_lib_ptr->aec_info.max_analog_gain *
        MULTIPLY_FACTOR_FOR_ISO);
      sensor_cap->min_gain =
        (uint32_t)(lib->sensor_lib_ptr->aec_info.min_gain *
        MULTIPLY_FACTOR_FOR_ISO);
    }
  }

  switch (lib->sensor_lib_ptr->sensor_output.filter_arrangement) {
      case SENSOR_BGGR:
        sensor_cap->color_arrangement = CAM_FILTER_ARRANGEMENT_BGGR;
        sensor_cap->num_color_channels = 4;
        break;
      case SENSOR_GBRG:
        sensor_cap->color_arrangement = CAM_FILTER_ARRANGEMENT_GBRG;
        sensor_cap->num_color_channels = 4;
        break;
      case SENSOR_GRBG:
        sensor_cap->color_arrangement = CAM_FILTER_ARRANGEMENT_GRBG;
        sensor_cap->num_color_channels = 4;
        break;
      case SENSOR_RGGB:
        sensor_cap->color_arrangement = CAM_FILTER_ARRANGEMENT_RGGB;
        sensor_cap->num_color_channels = 4;
        break;
      case SENSOR_UYVY:
        sensor_cap->color_arrangement = CAM_FILTER_ARRANGEMENT_UYVY;
        sensor_cap->num_color_channels = 3;
        break;
      case SENSOR_YUYV:
        sensor_cap->color_arrangement = CAM_FILTER_ARRANGEMENT_YUYV;
        sensor_cap->num_color_channels = 3;
        break;
      case SENSOR_Y:
        sensor_cap->color_arrangement = CAM_FILTER_ARRANGEMENT_Y;
        sensor_cap->num_color_channels = 1;
        break;
      default:
        return SENSOR_ERROR_INVAL;
  }

  sensor_cap->white_level = lib->sensor_lib_ptr->color_level_info.white_level;
  sensor_cap->black_level_pattern[0] =
    lib->sensor_lib_ptr->color_level_info.r_pedestal;
  sensor_cap->black_level_pattern[1] =
    lib->sensor_lib_ptr->color_level_info.gr_pedestal;
  sensor_cap->black_level_pattern[2] =
    lib->sensor_lib_ptr->color_level_info.gb_pedestal;
  sensor_cap->black_level_pattern[3] =
    lib->sensor_lib_ptr->color_level_info.b_pedestal;

  sensor_cap->filter_densities_count = 1;
  sensor_cap->filter_densities[0] = 0;

  /* Fill sensor pipeline delay */
  sensor_cap->max_frame_applying_delay =
      lib->sensor_lib_ptr->sensor_max_pipeline_frame_delay;
  sensor_cap->max_meta_reporting_delay = 0;
  sensor_cap->max_pipeline_frame_delay =
      lib->sensor_lib_ptr->sensor_max_pipeline_frame_delay;

  /* test pattern info */
  if (!lib->sensor_lib_ptr->test_pattern_info.size) {
    /* By default we send CAM_TEST_PATTERN_OFF as supported
     * even though sensor driver reports 0.
     */
    sensor_cap->supported_test_pattern_modes_cnt = 1;
    sensor_cap->supported_test_pattern_modes[0] = CAM_TEST_PATTERN_OFF;
  } else {
    sensor_cap->supported_test_pattern_modes_cnt =
      lib->sensor_lib_ptr->test_pattern_info.size;
    for (i = 0; i < sensor_cap->supported_test_pattern_modes_cnt; i++) {
      switch (
        lib->sensor_lib_ptr->test_pattern_info.test_pattern_settings[i].mode) {
        case SENSOR_TEST_PATTERN_OFF:
          sensor_cap->supported_test_pattern_modes[i] =
            CAM_TEST_PATTERN_OFF;
          break;
        case SENSOR_TEST_PATTERN_SOLID_COLOR:
          sensor_cap->supported_test_pattern_modes[i] =
            CAM_TEST_PATTERN_SOLID_COLOR;
          break;
        case SENSOR_TEST_PATTERN_COLOR_BARS:
          sensor_cap->supported_test_pattern_modes[i] =
            CAM_TEST_PATTERN_COLOR_BARS;
          break;
        case SENSOR_TEST_PATTERN_COLOR_BARS_FADE_TO_GRAY:
          sensor_cap->supported_test_pattern_modes[i] =
            CAM_TEST_PATTERN_COLOR_BARS_FADE_TO_GRAY;
          break;
        case SENSOR_TEST_PATTERN_PN9:
          sensor_cap->supported_test_pattern_modes[i] =
            CAM_TEST_PATTERN_PN9;
         break;
        case SENSOR_TEST_PATTERN_CUSTOM1:
          sensor_cap->supported_test_pattern_modes[i] =
            CAM_TEST_PATTERN_CUSTOM1;
         break;
        default:
          break;
      }
    }
  }

  /* optical black region rectangles */
  optical_black_region_ptr = &(lib->sensor_lib_ptr->optical_black_region_info);
  if (optical_black_region_ptr->size > MAX_SENSOR_OPTICAL_BLACK_REGION) {
    SERR("Invalid optical_black_region_ptr->size = %d",
      optical_black_region_ptr->size);
    return SENSOR_ERROR_INVAL;
  }
  sensor_cap->sensor_optical_black_region_count =
    optical_black_region_ptr->size;
  for (i = 0; i < optical_black_region_ptr->size; i++){
    sensor_cap->sensor_optical_black_region[i].left =
        optical_black_region_ptr->optical_black_region[i].x_start;
    sensor_cap->sensor_optical_black_region[i].top =
        optical_black_region_ptr->optical_black_region[i].y_start;
    sensor_cap->sensor_optical_black_region[i].width =
        optical_black_region_ptr->optical_black_region[i].width;
    sensor_cap->sensor_optical_black_region[i].height =
        optical_black_region_ptr->optical_black_region[i].height;
  }

  /* noise profile co-efficient */
  sensor_cap->gradient_S = lib->sensor_lib_ptr->noise_coeff.gradient_S;
  sensor_cap->offset_S = lib->sensor_lib_ptr->noise_coeff.offset_S;
  sensor_cap->gradient_O = lib->sensor_lib_ptr->noise_coeff.gradient_O;
  sensor_cap->offset_O = lib->sensor_lib_ptr->noise_coeff.offset_O;

  /* Texture and skin tone settings */
  sensor_cap->brightness_ctrl.def_value = 3;
  sensor_cap->brightness_ctrl.max_value = 6;
  sensor_cap->brightness_ctrl.min_value = 0;
  sensor_cap->brightness_ctrl.step = 1;

  sensor_cap->sharpness_ctrl.def_value = 12;
  sensor_cap->sharpness_ctrl.max_value = 36;
  sensor_cap->sharpness_ctrl.min_value = 0;
  sensor_cap->sharpness_ctrl.step = 6;

  sensor_cap->contrast_ctrl.def_value = 5;
  sensor_cap->contrast_ctrl.max_value = 10;
  sensor_cap->contrast_ctrl.min_value = 0;
  sensor_cap->contrast_ctrl.step = 1;

  sensor_cap->saturation_ctrl.def_value = 5;
  sensor_cap->saturation_ctrl.max_value = 10;
  sensor_cap->saturation_ctrl.min_value = 0;
  sensor_cap->saturation_ctrl.step = 1;

  sensor_cap->sce_ctrl.def_value = 0;
  sensor_cap->sce_ctrl.max_value = 100;
  sensor_cap->sce_ctrl.min_value = -100;
  sensor_cap->sce_ctrl.step = 10;

  sensor_cap->sensor_rotation =
  lib->sensor_lib_ptr->sensor_orientation;

  if (lib->sensor_lib_ptr->sensor_output.is_depth_sensor) {
    sensor_cap->is_depth_sensor = TRUE;
  } else {
    sensor_cap->is_depth_sensor = FALSE;
  }

  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_get_resolution_info -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_get_resolution_info(void *sctrl, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  sensor_out_info_t *sensor_out_info = (void *)data;
  int16_t cur_res;
  uint16_t i = 0, j = 0;
  sensor_stream_info_array_t    *sensor_stream_info_array = NULL;
  sensor_rolloff_config_t       *sensor_rolloff_config    = NULL;
  sensor_full_size_info_t       *full_size_info           = NULL;
  struct sensor_lib_out_info_t  *out_info                 = NULL;
  struct sensor_crop_parms_t    *crop_params              = NULL;
  int32_t offset_x = 0, offset_y = 0;
  int32_t enabled = 1;
  char value[PROPERTY_VALUE_MAX];

  if (!sensor_out_info) {
    SERR("failed");
    return SENSOR_ERROR_INVAL;
  }
  cur_res = ctrl->s_data->cur_res;

  /* Fill imglib params */
  sensor_out_info->is_depth_sensor = lib->sensor_lib_ptr->sensor_output.is_depth_sensor;
  sensor_out_info->full_width =
    lib->sensor_lib_ptr->pixel_array_size_info.active_array_size.width;
  sensor_out_info->full_height =
    lib->sensor_lib_ptr->pixel_array_size_info.active_array_size.height;
  sensor_out_info->filter_arrangement =
    lib->sensor_lib_ptr->sensor_output.filter_arrangement;
  sensor_out_info->pedestal =
    lib->sensor_lib_ptr->color_level_info.gb_pedestal;
  if(ctrl->s_data->eeprom_data) {
    sensor_out_info->custom_cal_data =
      ctrl->s_data->eeprom_data->custom_data;
  }
  /* Fill isp params */
  sensor_out_info->parse_RDI_statistics.parse_VHDR_stats_callback =
      lib->sensor_lib_ptr->parse_RDI_stats.parse_VHDR_stats;
  sensor_out_info->parse_RDI_statistics.parse_flicker_stats_callback =
      lib->sensor_lib_ptr->parse_RDI_stats.parse_flicker_stats;
  sensor_stream_info_array = &(lib->sensor_lib_ptr->sensor_stream_info_array);
  sensor_out_info->dim_output.width =
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].x_output;
  sensor_out_info->dim_output.height =
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].y_output;
  sensor_out_info->offset_x =
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].offset_x;
  sensor_out_info->offset_y =
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].offset_y;
  sensor_out_info->request_crop.first_pixel =
    lib->sensor_lib_ptr->crop_params_array.crop_params[cur_res].left_crop;
  sensor_out_info->request_crop.last_pixel =
      (uint32_t)(lib->sensor_lib_ptr->out_info_array.out_info[cur_res].x_output -
    lib->sensor_lib_ptr->crop_params_array.crop_params[cur_res].right_crop - 1);
  sensor_out_info->request_crop.first_line =
    lib->sensor_lib_ptr->crop_params_array.crop_params[cur_res].top_crop;
  sensor_out_info->request_crop.last_line =
    (uint32_t)(lib->sensor_lib_ptr->out_info_array.out_info[cur_res].y_output -
    lib->sensor_lib_ptr->crop_params_array.crop_params[cur_res].bottom_crop -
    1);
  sensor_out_info->op_pixel_clk =
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].op_pixel_clk;
  if (ctrl->s_data->ae_bracket_info.ae_bracket_config.mode ==
     CAM_EXP_BRACKETING_ON) {
    int32_t num_frames_skip = MAXQ(lib->sensor_lib_ptr->sensor_num_frame_skip,
      lib->sensor_lib_ptr->sensor_num_HDR_frame_skip);
    int32_t sensor_max_pipeline_frame_delay =
      lib->sensor_lib_ptr->sensor_max_pipeline_frame_delay;
    int32_t sensor_num_HDR_frame_skip =
      lib->sensor_lib_ptr->sensor_num_HDR_frame_skip;

    if (sensor_num_HDR_frame_skip < sensor_max_pipeline_frame_delay) {
      num_frames_skip -= sensor_max_pipeline_frame_delay;
    }

    if (sensor_max_pipeline_frame_delay > 0)
      sensor_max_pipeline_frame_delay -=1;

    sensor_out_info->num_frames_skip = num_frames_skip < 0 ?
      sensor_max_pipeline_frame_delay : num_frames_skip;
    SHIGH("BRAK frame initial skip: %d", sensor_out_info->num_frames_skip);
  } else if(ctrl->s_data->manual_exposure_mode == 1) {
    sensor_out_info->num_frames_skip = 0;
  } else {
    sensor_out_info->num_frames_skip =
      lib->sensor_lib_ptr->sensor_num_frame_skip;
  }

  /* In order to optimize ISP clock, it needs sensor's original max FPS
     in that usecase */
  sensor_out_info->orig_max_fps =
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].max_fps;

  if (lib->sensor_lib_ptr->sensor_output.output_format != SENSOR_YCBCR) {
    /* Fill 3A params */
    sensor_out_info->max_gain =
      lib->sensor_lib_ptr->aec_info.max_gain;
    sensor_out_info->svhdr_use_separate_gain =
      !!lib->sensor_lib_ptr->aec_info.svhdr_use_separate_gain;
    sensor_out_info->svhdr_use_separate_limits =
      !!lib->sensor_lib_ptr->aec_info.svhdr_use_separate_limits;
    for (i = 0; i < ARRAY_SIZE(sensor_out_info->min_line_cnt); ++i) {
      sensor_out_info->min_line_cnt[i] =
        lib->sensor_lib_ptr->aec_info.min_line_cnt[i];
      sensor_out_info->max_line_cnt[i] =
        lib->sensor_lib_ptr->aec_info.max_line_cnt[i];
    }
    sensor_out_info->max_linecount =
      lib->sensor_lib_ptr->aec_info.max_linecount;
    sensor_out_info->min_linecount =
      lib->sensor_lib_ptr->aec_info.min_linecount;
    if (ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW) ||
      ctrl->s_data->cur_stream_mask & (1 << CAM_STREAM_TYPE_VIDEO)) {
      sensor_out_info->max_fps = ctrl->s_data->cur_fps;
    } else {
      sensor_out_info->max_fps =
        lib->sensor_lib_ptr->out_info_array.out_info[cur_res].max_fps;
    }
  } else {
    sensor_out_info->max_fps =
      lib->sensor_lib_ptr->out_info_array.out_info[cur_res].max_fps;
  }
  sensor_out_info->vt_pixel_clk = SENSOR_GET_VT_PIK_CLK(
    &lib->sensor_lib_ptr->out_info_array.out_info[cur_res]);
  sensor_out_info->ll_pck =
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].line_length_pclk;
  sensor_out_info->fl_lines =
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].frame_length_lines;
  sensor_out_info->min_hbi =
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].min_hbi;
  sensor_out_info->min_vbi =
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].min_vbi;

  /* binning_method: 1 for sum, 0 for average */
  sensor_out_info->binning_factor =
      lib->sensor_lib_ptr->out_info_array.out_info[cur_res].binning_factor;
  if(lib->sensor_lib_ptr->out_info_array.out_info[cur_res].binning_method == 1)
    sensor_out_info->pixel_sum_factor =
      lib->sensor_lib_ptr->out_info_array.out_info[cur_res].binning_factor;
  else
    sensor_out_info->pixel_sum_factor = 1;

  /* Lens related information for AF */
  sensor_out_info->af_lens_info.pix_size =
     lib->sensor_lib_ptr->sensor_property.pix_size;
  /* Fill sensor pipeline delay */
  sensor_out_info->sensor_max_pipeline_frame_delay =
      lib->sensor_lib_ptr->sensor_max_pipeline_frame_delay;

  sensor_rolloff_config = &sensor_out_info->sensor_rolloff_config;
  full_size_info = &(lib->sensor_lib_ptr->rolloff_config.full_size_info);
  out_info = &lib->sensor_lib_ptr->out_info_array.out_info[cur_res];
  crop_params = &lib->sensor_lib_ptr->crop_params_array.crop_params[cur_res];

  sensor_out_info->custom_format.enable =
    out_info->custom_format.enable;
  if (out_info->custom_format.enable) {
    sensor_out_info->custom_format.subframes_cnt =
      out_info->custom_format.subframes_cnt;
    sensor_out_info->custom_format.start_x =
      out_info->custom_format.start_x;
    sensor_out_info->custom_format.start_y =
      out_info->custom_format.start_y;
    sensor_out_info->custom_format.width =
      out_info->custom_format.width;
    sensor_out_info->custom_format.height =
      out_info->custom_format.height;
  }

  memset(sensor_rolloff_config, 0, sizeof(*sensor_rolloff_config));

  if (lib->sensor_lib_ptr->rolloff_config.enable) {
    sensor_rolloff_config->enable = TRUE;
    sensor_rolloff_config->full_width = full_size_info->full_size_width;
    sensor_rolloff_config->full_height = full_size_info->full_size_height;
    sensor_rolloff_config->output_width = out_info->x_output -
      crop_params->left_crop - crop_params->right_crop;
    sensor_rolloff_config->output_height = out_info->y_output -
      crop_params->top_crop - crop_params->bottom_crop;
    offset_x = out_info->offset_x + crop_params->left_crop -
      full_size_info->full_size_left_crop;
    if (offset_x >= 0) {
      sensor_rolloff_config->offset_x = offset_x;
    } else {
      sensor_rolloff_config->offset_x = 0;
      sensor_rolloff_config->output_width =
        sensor_rolloff_config->full_width / out_info->scale_factor;
    }
    offset_y = out_info->offset_y + crop_params->top_crop -
      full_size_info->full_size_top_crop;
    if (offset_y >= 0) {
      sensor_rolloff_config->offset_y = offset_y;
    } else {
      sensor_rolloff_config->offset_y = 0;
      sensor_rolloff_config->output_height =
        sensor_rolloff_config->full_height / out_info->scale_factor;
    }
    sensor_rolloff_config->scale_factor = out_info->scale_factor;
    SHIGH("full %d %d out %d %d offset %d %d scale %d",
      sensor_rolloff_config->full_width, sensor_rolloff_config->full_height,
      sensor_rolloff_config->output_width,
      sensor_rolloff_config->output_height,
      sensor_rolloff_config->offset_x,
      sensor_rolloff_config->offset_y,
      sensor_rolloff_config->scale_factor);
  } else {
    sensor_rolloff_config->enable = FALSE;
  }
  sensor_out_info->sensor_hdr_enable =
    ((ctrl->s_data->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR) ||
     (ctrl->s_data->hdr_mode == CAM_SENSOR_3EXP_HDR_IN_SENSOR)) ? 1 : 0;

  property_get("persist.vendor.camera.pdaf.enable", value, "1");
  enabled = atoi(value);
  SHIGH("pdaf enable: %d, is_secure: %d", enabled, sensor_out_info->is_secure);
  if(enabled && sensor_out_info->is_secure == NON_SECURE){
    sensor_out_info->is_pdaf_supported =
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].is_pdaf_supported;
  } else {
    sensor_out_info->is_pdaf_supported = 0 ;
  }

  /* reset pix channel*/
  for (i = 0;
  i < lib->sensor_lib_ptr->csid_lut_params_array.lut_params[cur_res].num_cid;
  i++) {
    switch (lib->sensor_lib_ptr->csid_lut_params_array.
    lut_params[cur_res].vc_cfg_a[i].dt) {
    case CSI_YUV422_8:
    case CSI_RAW8:
    case CSI_RAW10:
    case CSI_RAW12:
    case CSI_RAW14:
    case CSI_DPCM6:
    case CSI_DPCM8: {
         for (j = 0; j < lib->sensor_lib_ptr->sensor_stream_info_array.
           sensor_stream_info[0].vc_cfg_size; j++) {
           if (lib->sensor_lib_ptr->csid_lut_params_array.
             lut_params[cur_res].vc_cfg_a[i].cid ==
             lib->sensor_lib_ptr->sensor_stream_info_array.sensor_stream_info[0].
             vc_cfg[j].cid)
             break;
         }
         if (j < lib->sensor_lib_ptr->sensor_stream_info_array.sensor_stream_info[0].
           vc_cfg_size && lib->sensor_lib_ptr->sensor_stream_info_array.
           sensor_stream_info[0].pix_data_fmt[j] == SENSOR_META)
           continue;
         else {
           int pix_fmt_fourcc;
            pix_fmt_fourcc = sensor_util_get_fourcc_format(
            lib->sensor_lib_ptr->sensor_output.output_format,
            lib->sensor_lib_ptr->sensor_output.filter_arrangement,
            lib->sensor_lib_ptr->csid_lut_params_array.
            lut_params[cur_res].vc_cfg_a[i].decode_format);
            sensor_out_info->fmt = sensor_util_get_hal_format(pix_fmt_fourcc);
         }
         break;
    }
    default:
     break;
    }
  }
  SLOW("sensor_out_info->fmt %d", sensor_out_info->fmt);

  /* configure meta channel */
  sensor_out_info->meta_cfg.num_meta = 0;
  memset(sensor_out_info->meta_cfg.sensor_meta_info, 0,
      sizeof(sensor_out_info->meta_cfg.sensor_meta_info));

  if (sensor_out_info->is_pdaf_supported) {
    if(ctrl->s_data->eeprom_data != NULL)
      sensor_out_info->pdaf_data = &ctrl->s_data->eeprom_data->pdafc;

    for (i = 0; i < lib->sensor_lib_ptr->meta_data_out_info_array.size; i++)
      if(lib->sensor_lib_ptr->meta_data_out_info_array.
        meta_data_out_info[i].stats_type == PD_STATS) {
      sensor_out_info->meta_cfg.
        sensor_meta_info[sensor_out_info->meta_cfg.num_meta].is_valid = TRUE;
      sensor_out_info->meta_cfg.
        sensor_meta_info[sensor_out_info->meta_cfg.num_meta].dim.width =
        lib->sensor_lib_ptr->meta_data_out_info_array.
        meta_data_out_info[i].width;
      sensor_out_info->meta_cfg.
        sensor_meta_info[sensor_out_info->meta_cfg.num_meta].dim.height =
        lib->sensor_lib_ptr->meta_data_out_info_array.
        meta_data_out_info[i].height;
      sensor_out_info->meta_cfg.
        sensor_meta_info[sensor_out_info->meta_cfg.num_meta].fmt =
        CAM_FORMAT_META_RAW_8BIT;
      sensor_out_info->meta_cfg.
        sensor_meta_info[sensor_out_info->meta_cfg.num_meta].
        stats_type = lib->sensor_lib_ptr->meta_data_out_info_array.
        meta_data_out_info[i].stats_type;
      sensor_out_info->meta_cfg.
        sensor_meta_info[sensor_out_info->meta_cfg.num_meta].dt =
        lib->sensor_lib_ptr->meta_data_out_info_array.meta_data_out_info[i].dt;
      sensor_out_info->meta_cfg.num_meta++;
      }
  }

  if (sensor_out_info->sensor_hdr_enable) {
    for (i = 0; i < lib->sensor_lib_ptr->meta_data_out_info_array.size; i++)
      if(lib->sensor_lib_ptr->meta_data_out_info_array.
        meta_data_out_info[i].stats_type == HDR_STATS) {
      sensor_out_info->meta_cfg.
        sensor_meta_info[sensor_out_info->meta_cfg.num_meta].is_valid = TRUE;
      sensor_out_info->meta_cfg.
        sensor_meta_info[sensor_out_info->meta_cfg.num_meta].dim.width =
        lib->sensor_lib_ptr->meta_data_out_info_array.
        meta_data_out_info[i].width;
      sensor_out_info->meta_cfg.
        sensor_meta_info[sensor_out_info->meta_cfg.num_meta].dim.height =
        lib->sensor_lib_ptr->meta_data_out_info_array.
        meta_data_out_info[i].height;
      sensor_out_info->meta_cfg.
        sensor_meta_info[sensor_out_info->meta_cfg.num_meta].fmt =
        CAM_FORMAT_META_RAW_8BIT;
      sensor_out_info->meta_cfg.
        sensor_meta_info[sensor_out_info->meta_cfg.num_meta].
        stats_type = lib->sensor_lib_ptr->meta_data_out_info_array.
        meta_data_out_info[i].stats_type;
      sensor_out_info->meta_cfg.
        sensor_meta_info[sensor_out_info->meta_cfg.num_meta].dt =
        lib->sensor_lib_ptr->meta_data_out_info_array.meta_data_out_info[i].dt;
      sensor_out_info->meta_cfg.num_meta++;
      } else if(lib->sensor_lib_ptr->meta_data_out_info_array.
        meta_data_out_info[i].stats_type == FLICKER_STATS) {
        sensor_out_info->meta_cfg.
          sensor_meta_info[sensor_out_info->meta_cfg.num_meta].is_valid = TRUE;
        sensor_out_info->meta_cfg.
          sensor_meta_info[sensor_out_info->meta_cfg.num_meta].dim.width =
        lib->sensor_lib_ptr->meta_data_out_info_array.
          meta_data_out_info[i].width;
        sensor_out_info->meta_cfg.
          sensor_meta_info[sensor_out_info->meta_cfg.num_meta].dim.height =
        lib->sensor_lib_ptr->meta_data_out_info_array.
          meta_data_out_info[i].height;
        sensor_out_info->meta_cfg.
          sensor_meta_info[sensor_out_info->meta_cfg.num_meta].fmt =
          CAM_FORMAT_META_RAW_8BIT;
        sensor_out_info->meta_cfg.
          sensor_meta_info[sensor_out_info->meta_cfg.num_meta].
          stats_type = lib->sensor_lib_ptr->meta_data_out_info_array.
          meta_data_out_info[i].stats_type;
        sensor_out_info->meta_cfg.
          sensor_meta_info[sensor_out_info->meta_cfg.num_meta].dt =
          lib->sensor_lib_ptr->meta_data_out_info_array.meta_data_out_info[i].dt;
        sensor_out_info->meta_cfg.num_meta++;
      }
  }

  SINFO("sensor info: name: %s, res: %d, max_fps: %f, w: %d, h: %d "
    "op pix clk: %d, FLL: %d, LLPCK: %d, mode: %d, PDAF support: %d",
    lib->sensor_lib_ptr->sensor_slave_info.sensor_name, cur_res,
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].max_fps,
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].x_output,
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].y_output,
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].op_pixel_clk,
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].frame_length_lines,
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].line_length_pclk,
    lib->sensor_lib_ptr->out_info_array.out_info[cur_res].mode,
    sensor_out_info->is_pdaf_supported);

  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_get_sensor_port_info -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_get_sensor_port_info(void *slib, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)slib;
  sensor_stream_info_array_t **sensor_port_info_array =
    (sensor_stream_info_array_t **)data;
  if (!sensor_port_info_array) {
    SERR("failed");
    return SENSOR_ERROR_INVAL;
  }
  *sensor_port_info_array = &(lib->sensor_lib_ptr->sensor_stream_info_array);
  return rc;
}

/** sensor_get_test_pattern_data
 *
 *  @sctrl: sensor ctrl struct handle
 *
 *  @data: handle to cam_test_pattern_data_t
 *
 *  Get current test pattern data and
 *  return to module layer
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t sensor_get_test_pattern_data(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  cam_test_pattern_data_t *test_pattern_data =
    (cam_test_pattern_data_t *)data;

  if (!sctrl || !data) {
    SERR("failed: sctrl %p data %p", sctrl, data);
    return -EINVAL;
  }

  ctrl = (sensor_ctrl_t *)sctrl;
  if (!ctrl->s_data) {
    SERR("failed: invalid s_data");
    return -EINVAL;
  }

  *test_pattern_data = ctrl->s_data->test_pattern_data;

  return SENSOR_SUCCESS;
}

/** sensor_get_csid_test_mode
 *
 *  @sctrl: sensor control struct handle
 *  @data: handle to return per csid test mode info
 *
 *  Return csid test pattern mode info
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t sensor_get_csid_test_mode(void *sctrl, void *data)
{
  sensor_ctrl_t                         *ctrl;
  sensor_get_t                          *sensor_get;
  sensor_lib_params_t                   *lib;
  struct sensor_csid_testmode_parms     *testmode;
  char                                   prop[PROPERTY_VALUE_MAX];
  uint32_t                               payload_mode;

  if (!sctrl || !data) {
    SERR("failed: sctrl %p data %p", sctrl, data);
    return -EINVAL;
  }

  ctrl = (sensor_ctrl_t *)sctrl;
  sensor_get = (sensor_get_t *)data;

  lib = (sensor_lib_params_t *)ctrl->lib_params;
  testmode = &sensor_get->csid_test_mode_params;

  if (lib->sensor_lib_ptr->sensor_slave_info.slave_addr == 0 &&
    lib->sensor_lib_ptr->sensor_slave_info.power_setting_array.size == 0) {
    SHIGH("sensor output : csid generated patterns");
    testmode->num_bytes_per_line =
      (lib->sensor_lib_ptr->
        out_info_array.out_info[ctrl->s_data->cur_res].x_output * 10) / 8;
    testmode->num_lines =
      lib->sensor_lib_ptr->
        out_info_array.out_info[ctrl->s_data->cur_res].y_output;
    testmode->h_blanking_count =
      lib->sensor_lib_ptr->
        out_info_array.out_info[ctrl->s_data->cur_res].line_length_pclk -
      lib->sensor_lib_ptr->
        out_info_array.out_info[ctrl->s_data->cur_res].x_output;
    testmode->v_blanking_count =
      lib->sensor_lib_ptr->
        out_info_array.out_info[ctrl->s_data->cur_res].frame_length_lines -
      lib->sensor_lib_ptr->
        out_info_array.out_info[ctrl->s_data->cur_res].y_output;

    property_get("persist.vendor.camera.csid.payload", prop, "4");
    payload_mode = atoi(prop);

    testmode->payload_mode = payload_mode;
  } else {
    SLOW("sensor output : notmal sensor output");
    memset(testmode, 0, sizeof(struct sensor_csid_testmode_parms));
  }

  return SENSOR_SUCCESS;
}

/*===========================================================================
 * FUNCTION    - sensor_set_test_pattern -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_set_test_pattern(void *sctrl, void *data)
{
  uint32_t i = 0;
  int32_t selected_index = -1;
  uint8_t no_test_modes = 0;
  struct sensorb_cfg_data cfg;
  sensor_test_pattern_t sensor_test_pattern;
  struct msm_camera_i2c_reg_setting *pSettings = NULL;
  cam_test_pattern_data_t * pTestPattern = (cam_test_pattern_data_t *)data;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  sensor_test_info *test_pattern_info =
    &(lib->sensor_lib_ptr->test_pattern_info);
  struct msm_camera_i2c_reg_setting test_settings;
  sensor_test_pattern_t sensor_test_pattern_mode;
  SLOW("enter");

  if (ctrl->s_data->fd < 0) {
    SERR("fd check failed");
    return FALSE;
  }

  no_test_modes = test_pattern_info->size;

  if (ctrl->s_data->cur_test_pattern_mode == pTestPattern->mode &&
    CAM_TEST_PATTERN_OFF == pTestPattern->mode)
    return TRUE;

  switch(pTestPattern->mode) {
    case CAM_TEST_PATTERN_OFF:
      sensor_test_pattern_mode = SENSOR_TEST_PATTERN_OFF;
      break;
    case CAM_TEST_PATTERN_SOLID_COLOR:
      sensor_test_pattern_mode = SENSOR_TEST_PATTERN_SOLID_COLOR;
      break;
    case CAM_TEST_PATTERN_COLOR_BARS:
      sensor_test_pattern_mode = SENSOR_TEST_PATTERN_COLOR_BARS;
      break;
    case CAM_TEST_PATTERN_COLOR_BARS_FADE_TO_GRAY:
      sensor_test_pattern_mode = SENSOR_TEST_PATTERN_COLOR_BARS_FADE_TO_GRAY;
      break;
    case CAM_TEST_PATTERN_PN9:
      sensor_test_pattern_mode = SENSOR_TEST_PATTERN_PN9;
      break;
    case CAM_TEST_PATTERN_CUSTOM1:
      sensor_test_pattern_mode = SENSOR_TEST_PATTERN_CUSTOM1;
      break;
    default:
      sensor_test_pattern_mode = SENSOR_TEST_PATTERN_MAX;
      break;
  }

  for (i = 0;i < no_test_modes; i++) {
    if (test_pattern_info->test_pattern_settings[i].mode ==
      sensor_test_pattern_mode)
      selected_index = i;
  }

  if (selected_index == -1) {
    SERR("test pattern mode not supported = %d", pTestPattern->mode);
    return SENSOR_FAILURE;
  }

  if (pTestPattern->mode == CAM_TEST_PATTERN_SOLID_COLOR) {
      int32_t shift_bits;
      switch (lib->sensor_lib_ptr->sensor_output.raw_output) {
        case SENSOR_14_BIT_DIRECT:
          shift_bits = 32 - 14;
          break;
        case SENSOR_12_BIT_DIRECT:
          shift_bits = 32 - 12;
          break;
        case SENSOR_10_BIT_DIRECT:
          shift_bits = 32 - 10;
          break;
        case SENSOR_8_BIT_DIRECT:
          shift_bits = 32 -  8;
          break;
        default:
          SERR("raw_output %d is not supported",
            lib->sensor_lib_ptr->sensor_output.raw_output);
          return SENSOR_FAILURE;
      }
      uint32_t delay = 0;
      struct msm_camera_i2c_reg_array reg_array[] = {
          {test_pattern_info->solid_mode_addr.r_addr,
              pTestPattern->r >> shift_bits, delay},
          {test_pattern_info->solid_mode_addr.gr_addr,
              pTestPattern->gr >> shift_bits, delay},
          {test_pattern_info->solid_mode_addr.b_addr,
              pTestPattern->b >> shift_bits, delay},
          {test_pattern_info->solid_mode_addr.gb_addr,
              pTestPattern->gb >> shift_bits, delay},
      };
      test_settings.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
      test_settings.data_type = MSM_CAMERA_I2C_WORD_DATA;
      test_settings.delay = 0;
      test_settings.reg_setting = reg_array;
      test_settings.size = 4;

      cfg.cfg.setting = &test_settings;
      cfg.cfgtype = CFG_WRITE_I2C_ARRAY;

      if (ioctl(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg) < 0) {
        SLOW("failed");
        return 0;
      }
  }

  test_settings.addr_type =
    sensor_sdk_util_get_kernel_i2c_addr_type(test_pattern_info->
    test_pattern_settings[selected_index].settings.addr_type);
  test_settings.data_type =
    sensor_sdk_util_get_kernel_i2c_data_type(test_pattern_info->
    test_pattern_settings[selected_index].settings.data_type);
  test_settings.delay =
    test_pattern_info->test_pattern_settings[selected_index].settings.delay;
  test_settings.reg_setting = (struct msm_camera_i2c_reg_array *)
    &(test_pattern_info->test_pattern_settings[selected_index].
    settings.reg_setting_a[0]);
  test_settings.size =
    test_pattern_info->test_pattern_settings[selected_index].settings.size;

  cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
  cfg.cfg.setting = &test_settings;
  if (ioctl(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg) < 0) {
    SLOW("failed");
    return 0;
  }

  ctrl->s_data->test_pattern_data = *pTestPattern;
  ctrl->s_data->cur_test_pattern_mode = pTestPattern->mode;
  SLOW("exit");
  return TRUE;
}

/*==========================================================
 * FUNCTION    - sensor_set_hdr_mode
 *
 * DESCRIPTION: set the  the hdr_mode
 *==========================================================*/
static int32_t sensor_set_hdr_mode(void *sctrl, void *data)
{
  sensor_ctrl_t          *ctrl = (sensor_ctrl_t *)sctrl;
  cam_sensor_hdr_type_t  *hdr_mode = (cam_sensor_hdr_type_t *)data;
  int32_t                ret = SENSOR_SUCCESS;

  RETURN_ERROR_ON_NULL(hdr_mode);
  RETURN_ERROR_ON_NULL(ctrl);

  SHIGH("Requested hdr_mode: %d", *hdr_mode);

  if (*hdr_mode < CAM_SENSOR_HDR_OFF || *hdr_mode >= CAM_SENSOR_HDR_MAX ) {
    SERR("Invalid HDR mode requested, mode:%d",*hdr_mode);
    return SENSOR_ERROR_INVAL;
  }

  if(ctrl->s_data->hdr_mode != *hdr_mode) {
    ctrl->s_data->hdr_mode = *hdr_mode;
    ret = SENSOR_SUCCESS;
  } else {
    /* not an error condition, just indicate that we are already in this mode */
    SLOW("Requested HDR mode is already set");
    ret = SENSOR_ERROR_INVAL;
  }
  return ret;
}

/*==========================================================
 * FUNCTION    - sensor_set_dis_enable -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_dis_enable(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  int32_t *dis_enable = (int32_t *)data;
  if (!dis_enable) {
    SERR("failed");
    return -EINVAL;
  }
  SLOW("dis enable %d", *dis_enable);
  ctrl->s_data->dis_enable = *dis_enable;
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_set_op_pixel_clk_change -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_op_pixel_clk_change(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  uint32_t *op_pixel_clk = (uint32_t *)data;
  if (!ctrl || !ctrl->s_data || !op_pixel_clk) {
    SERR("failed");
    return -EINVAL;
  }

  PTHREAD_MUTEX_LOCK(&ctrl->s_data->mutex);

  SLOW("dis enable %d", *op_pixel_clk);
  ctrl->s_data->isp_pixel_clk_max = *op_pixel_clk;

  PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);

  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_set_calibration_data -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_calibration_data(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct sensorb_cfg_data cfg;
  struct msm_camera_i2c_reg_setting* setting =
    (struct msm_camera_i2c_reg_setting*)data;

  if (!ctrl || !ctrl->s_data) {
    SERR("failed");
    return -EINVAL;
  }

  if(!setting || setting->size == 0){
    SERR("Golden module or OTP data is null");
    return SENSOR_SUCCESS;
  }

  PTHREAD_MUTEX_LOCK(&ctrl->s_data->mutex);

  cfg.cfgtype = CFG_WRITE_I2C_ARRAY;
  cfg.cfg.setting = data;
  if (LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg, "calibrate") < 0) {
    SERR("failed");
    PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);
    return -EIO;
  }

  PTHREAD_MUTEX_UNLOCK(&ctrl->s_data->mutex);

  return SENSOR_SUCCESS;
}


/** sensor_set_max_raw_dimension: set max dimension for sensor
 *
 *  @sctrl: handle to sensor control structure
 *  @data: handle to cam_dimension_t
 *
 *  This function stores max raw dimension passed by HAL
 *
 *  Return: SENSOR_SUCCESS for success
 *          Negative error for failure
 **/
static int32_t sensor_set_max_dimension(void *sctrl, void *data)
{
  sensor_ctrl_t   *ctrl = (sensor_ctrl_t *)sctrl;
  cam_dimension_t *max_dim = (cam_dimension_t *)data;

  if (!ctrl || !ctrl->s_data || !max_dim) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  if ((max_dim->width <= 0) || (max_dim->height <= 0)) {
    SERR("failed: invalid params raw dim w %d h %d", max_dim->width,
      max_dim->height);
    return SENSOR_FAILURE;
  }

  SHIGH("raw w %d h %d", max_dim->width, max_dim->height);
  ctrl->s_data->max_dim = *max_dim;

  return SENSOR_SUCCESS;
}

/** sensor_get_raw_dimension: get raw dimension for sensor
 *
 *  @sctrl: handle to sensor control structure
 *  @data: handle to cam_dimension_t
 *
 *  This function stores retrieves raw dimension for the
 *  resolution passed by HAL
 *
 *  Return: SENSOR_SUCCESS for success
 *          Negative error for failure
 **/
static int32_t sensor_get_raw_dimension(void *sctrl, void *data)
{
  int32_t                           rc = SENSOR_SUCCESS;
  sensor_ctrl_t                    *ctrl = (sensor_ctrl_t *)sctrl;
  cam_sensor_config_t              *raw_dim = NULL;
  cam_dimension_t                  *max_dim = NULL;
  sensor_set_res_cfg_t             *res_cfg;
  int32_t                           pick_res = SENSOR_INVALID_RESOLUTION;
  sensor_get_raw_dimension_t       *sensor_get =
    (sensor_get_raw_dimension_t *)data;
  struct sensor_lib_out_info_array *out_info_array = NULL;
  struct sensor_lib_out_info_t     *out_info = NULL;
  struct sensor_crop_parms_t       *crop_params = NULL;

  if (!ctrl || !ctrl->s_data || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  if (!ctrl->lib_params || !ctrl->lib_params->sensor_lib_ptr) {
    SERR("failed: invalid params");
    return SENSOR_FAILURE;
  }

  res_cfg = sensor_get->res_cfg;
  out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;
  raw_dim = (cam_sensor_config_t *)sensor_get->raw_dim;
  max_dim = &ctrl->s_data->max_dim;
  if ((max_dim->width <= 0) || (max_dim->height <= 0)) {
    SERR("failed: invalid params maw dim w %d h %d", max_dim->width,
      max_dim->height);
    return SENSOR_FAILURE;
  }

  res_cfg->width  = (uint32_t)max_dim->width;
  res_cfg->height = (uint32_t)max_dim->height;

  if (!res_cfg->aspect_r) {
    res_cfg->aspect_r = res_cfg->width * QFACTOR / res_cfg->height;
  }

  res_cfg->stream_mask = (sensor_get->stream_mask | (1 << CAM_STREAM_TYPE_RAW));
  res_cfg->is_fast_aec_mode_on = FALSE;
  res_cfg->fast_aec_sensor_mode = CAM_HFR_MODE_OFF;
  rc = sensor_pick_resolution(sctrl, res_cfg, &pick_res);
  if (rc < 0) {
    SERR("failed: sensor_pick_resolution rc %d", rc);
    return rc;
  }

  SHIGH("pick New Res %d", pick_res);
  if ((pick_res >= out_info_array->size) ||
      (pick_res == SENSOR_INVALID_RESOLUTION)) {
    SERR("Error: failed to find resolution requested w*h %d*%d fps %f",
      res_cfg->width, res_cfg->height, ctrl->s_data->cur_fps);
    return SENSOR_FAILURE;
  }

  out_info = &out_info_array->out_info[pick_res];
  crop_params =
    &ctrl->lib_params->sensor_lib_ptr->crop_params_array.crop_params[pick_res];
  raw_dim->width =
    out_info->x_output - crop_params->left_crop - crop_params->right_crop;
  raw_dim->height =
    out_info->y_output - crop_params->top_crop - crop_params->bottom_crop;
  raw_dim->opClock = out_info->op_pixel_clk;
  SHIGH("raw w %d h %d", raw_dim->width, raw_dim->height);

  return SENSOR_SUCCESS;
}

/** sensor_get_sensor_mode: Get sensor mode
 *
 *  @sctrl: sensor control structure
 *  @data: pointer to
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function returns sensor mode **/

static int32_t sensor_get_sensor_mode(void *sctrl, void *data)
{
  int32_t             rc = SENSOR_SUCCESS;
  sensor_ctrl_t       *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_data_t       *sdata;
  uint16_t            *sensor_mode = (uint16_t *)data;

  RETURN_ERROR_ON_NULL(ctrl);
  RETURN_ERROR_ON_NULL(data);

  sdata        = ctrl->s_data;
  *sensor_mode = sdata->cur_res;
  SLOW("cur %d, data %d", sdata->cur_res, *sensor_mode);

  return SENSOR_SUCCESS;
}

/** sensor_get_exposure_time: Get sensor exposure time
 *
 *  @sctrl: sensor control structure
 *  @data: pointer to
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function returns sensor exposure time in nanosecond **/

static int32_t sensor_get_exposure_time(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct sensor_lib_out_info_array *out_info_array = NULL;
  int16_t cur_res = SENSOR_INVALID_RESOLUTION;
  int64_t *exp = (int64_t *)data;
  uint32_t vt_pix_clk = 0;

  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;
  cur_res = ctrl->s_data->cur_res;
  if (cur_res == SENSOR_INVALID_RESOLUTION) {
    SERR("resolution is invalid, it seems stream has stopped.");
    return SENSOR_FAILURE;
  }
  vt_pix_clk = SENSOR_GET_VT_PIK_CLK(&out_info_array->out_info[cur_res]);

  SLOW("%s: line_length_pclk = %d, current_linecount = %d, vt_pixel_clk = %d",
    __func__, ctrl->s_data->cur_line_length_pclk,
    ctrl->s_data->current_linecount, vt_pix_clk);
  if (vt_pix_clk > 0) {
    *exp =(int64_t)NANO_SEC_PER_SEC *
      ((float)ctrl->s_data->current_linecount *
      ctrl->s_data->cur_line_length_pclk / vt_pix_clk);
  }
  ctrl->s_data->cur_exposure_time = (uint32_t)(*exp / NANO_SEC_TO_MICRO_SEC);
  SLOW("exp %lld" , *exp);

  return SENSOR_SUCCESS;
}

/** sensor_get_sensitivity: Get sensor sensitivity
 *
 *  @sctrl: sensor control structure
 *  @data: pointer to
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function returns sensor sensitivity in ISO uniti */
static int32_t sensor_get_sensitivity(void *sctrl, void *data)
{
  sensor_ctrl_t        *ctrl = NULL;
  int32_t              *sensitivity = NULL;

  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  ctrl = (sensor_ctrl_t *)sctrl;
  sensitivity = (int32_t *)data;
  *sensitivity = (int32_t)roundf((ctrl->s_data->sensor_real_gain * 100 *
    ctrl->s_data->sensor_real_digital_gain) / ctrl->s_data->iso100_gain);

  return SENSOR_SUCCESS;
}

/** sensor_get_frame_duration: Get sensor frame duration
 *
 *  @sctrl: sensor control structure
 *  @data: pointer to frame duration variable
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function returns sensor exposure time in nanosecond **/

static int32_t sensor_get_frame_duration(void *sctrl, void *data)
{
  sensor_ctrl_t                    *ctrl;
  struct sensor_lib_out_info_array *out_info_array;
  int16_t                           cur_res;
  int64_t                          *frame_duration;
  double                            fl_lines, line_count;
  uint32_t                          vertical_offset;
  uint32_t                          vt_pix_clk = 0;

  if (!sctrl || !data) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  ctrl = (sensor_ctrl_t *)sctrl;
  frame_duration = (int64_t *)data;

  out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;
  cur_res = ctrl->s_data->cur_res;
  vertical_offset =
    ctrl->lib_params->sensor_lib_ptr->exp_gain_info.vert_offset;

  if (cur_res == SENSOR_INVALID_RESOLUTION) {
    SERR("resolution is invalid, it seems stream has stopped.");
    return SENSOR_FAILURE;
  }

  vt_pix_clk = SENSOR_GET_VT_PIK_CLK(&out_info_array->out_info[cur_res]);

  SLOW("cur_fps_div %f cur_lc %d fll %d llpclk %d vt %d",
    ctrl->s_data->current_fps_div,
    ctrl->s_data->current_linecount,
    out_info_array->out_info[cur_res].frame_length_lines,
    out_info_array->out_info[cur_res].line_length_pclk, vt_pix_clk);

  fl_lines = ctrl->s_data->cur_frame_length_lines;
  fl_lines *= ctrl->s_data->current_fps_div;
  SLOW("fl_lines %f", fl_lines);

  if (ctrl->s_data->current_linecount > fl_lines - vertical_offset)
    line_count = ctrl->s_data->current_linecount + vertical_offset;
  else
    line_count = fl_lines;

  if (vt_pix_clk != 0)
    *frame_duration = (int64_t)(NANO_SEC_PER_SEC *
    (double)line_count * out_info_array->out_info[cur_res].line_length_pclk) /
    vt_pix_clk;

  SLOW("frame duration %lld", *frame_duration);
  return SENSOR_SUCCESS;
}

/** sensor_get_output_info
 *
 *  @sctrl: sensor ctrl struct handle
 *
 *  @data: handle to sensor_get_output_info_t
 *
 *  Pick sensor output resolution based on requested dim and
 *  return to module layer
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t sensor_get_output_info(void *sctrl, void *data)
{
  int32_t                           rc = 0;
  sensor_ctrl_t                    *ctrl = NULL;
  sensor_get_output_info_t         *get_output_dim = NULL;
  int32_t                           res = SENSOR_INVALID_RESOLUTION;
  sensor_dim_output_t              *dim_output = NULL, *full_res = NULL;
  struct sensor_lib_out_info_array *out_info_array = NULL;
  sensor_stream_info_t             *sensor_stream_info;
  sensor_set_res_cfg_t             *res_cfg = NULL;
  uint32_t                         pix_fmt_fourcc = 0;
  uint16_t                         i = 0;
  int32_t                          enabled = 1;
  sensor_lib_params_t              *lib = NULL;
  char                             value[PROPERTY_VALUE_MAX];
  uint32_t                         is_pdaf_supported;
  int32_t                          buffer_type;

  if (!sctrl || !data) {
    SERR("failed: sctrl %p data %p", sctrl, data);
    return -EINVAL;
  }

  ctrl = (sensor_ctrl_t *)sctrl;
  if (!ctrl->lib_params || !ctrl->lib_params->sensor_lib_ptr) {
    SERR("failed: invalid ctrl params");
    return -EINVAL;
  }

  get_output_dim = (sensor_get_output_info_t *)data;
  dim_output = &get_output_dim->output_info.dim_output;
  full_res = &get_output_dim->output_info.full_res;
  res_cfg = &get_output_dim->res_cfg;
  out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;
  lib = (sensor_lib_params_t *)ctrl->lib_params;

  SHIGH("requested dim %d %d stream mask %x", res_cfg->width, res_cfg->height,
    res_cfg->stream_mask);

  /* update current fps */
  ctrl->s_data->cur_fps = ctrl->s_data->max_fps;

  full_res->width =
    ctrl->lib_params->sensor_lib_ptr->pixel_array_size_info.active_array_size.width;
  full_res->height =
    ctrl->lib_params->sensor_lib_ptr->pixel_array_size_info.active_array_size.height;

  /* Clean output dim */
  dim_output->width = 0;
  dim_output->height = 0;

  /* Get sensor ouput resolution for requested dimension */
  rc = sensor_pick_resolution(ctrl, &get_output_dim->res_cfg, &res);
  if (rc < 0) {
    SERR("failed: sensor_pick_resolution");
    return SENSOR_FAILURE;
  }

  if ((res >= out_info_array->size) || (res == SENSOR_INVALID_RESOLUTION)) {
    SERR("Error: failed");
    return SENSOR_FAILURE;
  }

  /* Send sensor name */
  get_output_dim->output_info.sensor_name =
    ctrl->lib_params->sensor_lib_ptr->sensor_slave_info.sensor_name;

  /* Send the resolution index */
  get_output_dim->output_info.res_idx = res;

  /* Send crop data */
  get_output_dim->request_crop.first_pixel =
    ctrl->lib_params->sensor_lib_ptr->crop_params_array.
    crop_params[res].left_crop;
  get_output_dim->request_crop.last_pixel =
      (uint32_t)(ctrl->lib_params->sensor_lib_ptr->out_info_array.
      out_info[res].x_output - ctrl->lib_params->sensor_lib_ptr->
      crop_params_array.crop_params[res].right_crop - 1);
  get_output_dim->request_crop.first_line =
    ctrl->lib_params->sensor_lib_ptr->crop_params_array.
    crop_params[res].top_crop;
  get_output_dim->request_crop.last_line =
    (uint32_t)(ctrl->lib_params->sensor_lib_ptr->out_info_array.
    out_info[res].y_output - ctrl->lib_params->sensor_lib_ptr->
    crop_params_array.crop_params[res].bottom_crop - 1);

  /* Populate sensor output in requested struct
     dimension, vt pixel clock, output format */
  dim_output->width = out_info_array->out_info[res].x_output;
  dim_output->height = out_info_array->out_info[res].y_output;
  get_output_dim->output_info.op_pixel_clk =
    out_info_array->out_info[res].op_pixel_clk;

  get_output_dim->output_info.custom_format.enable =
    out_info_array->out_info[res].custom_format.enable;
  if (out_info_array->out_info[res].custom_format.enable) {
    get_output_dim->output_info.custom_format.subframes_cnt =
      out_info_array->out_info[res].custom_format.subframes_cnt;
    get_output_dim->output_info.custom_format.start_x =
      out_info_array->out_info[res].custom_format.start_x;
    get_output_dim->output_info.custom_format.start_y =
      out_info_array->out_info[res].custom_format.start_y;
    get_output_dim->output_info.custom_format.width =
      out_info_array->out_info[res].custom_format.width;
    get_output_dim->output_info.custom_format.height =
      out_info_array->out_info[res].custom_format.height;
  }

  SHIGH("pick res %d dim %dX%d op clk %d subframes cnt %d",
    res, dim_output->width, dim_output->height,
    get_output_dim->output_info.op_pixel_clk,
    out_info_array->out_info[res].custom_format.subframes_cnt);

  sensor_stream_info = &(ctrl->lib_params->sensor_lib_ptr->
    sensor_stream_info_array.sensor_stream_info[0]);

  pix_fmt_fourcc = sensor_util_get_fourcc_format(
    sensor_stream_info->pix_data_fmt[0],
    ctrl->lib_params->sensor_lib_ptr->sensor_output.filter_arrangement,
    sensor_stream_info->vc_cfg[0].decode_format);

  if (pix_fmt_fourcc == 0) {
    SERR("failed");
    return SENSOR_ERROR_INVAL;
  }

  get_output_dim->output_info.output_format = pix_fmt_fourcc;

  get_output_dim->output_info.need_camif_raw = FALSE;
  property_get("persist.vendor.camera.pdaf.enable", value, "1");
  enabled = atoi(value);
  SHIGH("pdaf enable: %d, is_secure: %d", enabled, get_output_dim->is_secure);
  if(enabled && get_output_dim->is_secure == NON_SECURE){
    is_pdaf_supported =
      lib->sensor_lib_ptr->out_info_array.out_info[res].is_pdaf_supported;
  } else {
    is_pdaf_supported = 0 ;
  }

  if (is_pdaf_supported) {
    buffer_type = lib->sensor_lib_ptr->
      pdaf_config.buffer_block_pattern_info[res].buffer_type;
    if (buffer_type == PDAF_BUFFER_FLAG_SPARSE)
      get_output_dim->output_info.need_camif_raw = TRUE;
  }

  return SENSOR_SUCCESS;
}

/** sensor_get_bet_metadata
 *
 *  @sctrl: sensor control struct handle
 *  @data: handle to return per frame BET metadata
 *
 *  Return BET metadata applied on current frame
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t sensor_get_bet_metadata(void *sctrl, void *data)
{
  sensor_ctrl_t                       *ctrl = NULL;
  sensor_bet_metadata_t               *metadata = NULL;
  int16_t                              cur_res = SENSOR_INVALID_RESOLUTION;
  struct sensor_lib_out_info_array    *out_info_array = NULL;
  struct sensor_lib_crop_params_array *crop_info_array = NULL;
  int64_t                              exp_time = 0;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  ctrl = (sensor_ctrl_t *)sctrl;
  metadata = (sensor_bet_metadata_t *)data;

  RETURN_ERROR_ON_NULL(ctrl->lib_params);
  RETURN_ERROR_ON_NULL(ctrl->lib_params->sensor_lib_ptr);
  RETURN_ERROR_ON_NULL(ctrl->s_data);

  out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;
  crop_info_array = &ctrl->lib_params->sensor_lib_ptr->crop_params_array;
  cur_res = ctrl->s_data->cur_res;

  metadata->width = out_info_array->out_info[cur_res].x_output -
    crop_info_array->crop_params[cur_res].left_crop -
    crop_info_array->crop_params[cur_res].right_crop;
  metadata->height = out_info_array->out_info[cur_res].y_output -
    crop_info_array->crop_params[cur_res].top_crop -
    crop_info_array->crop_params[cur_res].bottom_crop;
  metadata->real_gain = ctrl->s_data->gain_from_aec;
  metadata->isp_digital_gain = ctrl->s_data->digital_gain;
  metadata->exposure_time = ctrl->s_data->cur_exposure_time;
  // Short Exposure (for future use)
  metadata->se_real_gain = ctrl->s_data->sensor_real_gain;
  metadata->se_digital_gain = ctrl->s_data->digital_gain;
  metadata->se_exposure_time = ctrl->s_data->cur_exposure_time;

  return SENSOR_SUCCESS;
}

static int32_t sensor_get_exposure_starttime(void *sctrl, void *data)
{
  sensor_ctrl_t                    *ctrl;
  sensor_exposure_starttime_t      *exposure_start;
  int64_t                           line_readout = 0;
  int16_t                           cur_res;
  struct sensor_lib_out_info_array *out_info_array;
  uint32_t                          vt_pix_clk = 0;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  ctrl = (sensor_ctrl_t *)sctrl;
  exposure_start = (sensor_exposure_starttime_t *)data;

  /* line readout time */
  out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;
  cur_res = ctrl->s_data->cur_res;
  vt_pix_clk = SENSOR_GET_VT_PIK_CLK(&out_info_array->out_info[cur_res]);

  if(vt_pix_clk != 0)
    line_readout = (int64_t)out_info_array->out_info[cur_res].line_length_pclk *
      NANO_SEC_PER_SEC / vt_pix_clk;

  SLOW("sof %lld exposure time %lld line_readout %lld adc_readout %lld",
    exposure_start->sof_timestamp, exposure_start->exposure_time,
    line_readout, ctrl->lib_params->sensor_lib_ptr->adc_readout_time);

  if (ctrl->lib_params->sensor_lib_ptr->sensor_output.is_depth_sensor &&
    ctrl->lib_params->sensor_lib_ptr->exposure_func_table.
    sensor_calculate_exposure_start_time) {
    exposure_start->exposure_start_timestamp =
      ctrl->lib_params->sensor_lib_ptr->exposure_func_table.
      sensor_calculate_exposure_start_time(exposure_start->sof_timestamp,
      exposure_start->exposure_time, cur_res);
  } else {
    /* TIMESTAMP for exposure start =
       SOF_timestamp - exposure time - line_readout time - adc_readout time */
    exposure_start->exposure_start_timestamp =
      exposure_start->sof_timestamp -
      exposure_start->exposure_time -
      line_readout -
      ctrl->lib_params->sensor_lib_ptr->adc_readout_time;
  }
  SLOW("exposure timestamp %lld", exposure_start->exposure_start_timestamp);

  /* frame readout time */
  exposure_start->frame_readout_time =
  line_readout * (int64_t)ctrl->s_data->last_updated_fll;

  SLOW("frame readout time %lld", exposure_start->frame_readout_time);

  return SENSOR_SUCCESS;
}

static int32_t sensor_pdaf_parse_pd(void *sctrl, void *data)
{
  sensor_ctrl_t             *ctrl;
  pdaf_params_t             *s_pdaf;
  sensor_lib_params_t       *lib;
  int32_t                   rc = 0;
  sensor_RDI_parser_stats_t *parser = NULL;
  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  ctrl = (sensor_ctrl_t *)sctrl;
  lib = (sensor_lib_params_t *)ctrl->lib_params;
  s_pdaf = (pdaf_params_t *)data;
  parser = &lib->sensor_lib_ptr->parse_RDI_stats;
  if(parser->pd_data_format != SENSOR_STATS_CUSTOM)
    rc = pdaf_parse_pd(parser->pd_data_format, s_pdaf);
  else if(parser->pd_data_format == SENSOR_STATS_CUSTOM
    && parser->parse_PDAF_stats)
    rc = parser->parse_PDAF_stats(NULL, (void *)s_pdaf);

  if(rc < 0)
    return SENSOR_FAILURE;

  return SENSOR_SUCCESS;
}

static int32_t sensor_pdaf_cal_defocus(void *sctrl, void *data)
{
  sensor_ctrl_t             *ctrl;
  sensor_lib_params_t       *lib;
  int32_t                   rc = SENSOR_SUCCESS;
  pdaf_params_t             *s_pdaf;
  sensor_custom_API_t       *sensor_custom_API = NULL;
  signed long (*get_defocus)(void *input, void *output) = NULL;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  ctrl = (sensor_ctrl_t *)sctrl;
  lib = (sensor_lib_params_t *)ctrl->lib_params;
  s_pdaf = (pdaf_params_t *)data;

  s_pdaf->sensor_real_gain = ctrl->s_data->sensor_real_gain;
  s_pdaf->cur_res = ctrl->s_data->cur_res;
  sensor_custom_API = &ctrl->lib_params->sensor_custom_API;

  if(ctrl->s_data->pd_x_win_num && ctrl->s_data->pd_y_win_num) {
  /* Updated win_num from MCT_EVENT_MODULE_STATS_PDAF_AF_WINDOW_UPDATE*/
    s_pdaf->x_win_num = ctrl->s_data->pd_x_win_num;
    s_pdaf->y_win_num = ctrl->s_data->pd_y_win_num;
  }

  if(sensor_custom_API->sensor_custom_calc_defocus) {
    /* T1 using custom function */
    if(lib->sensor_custom_API.pdlib_get_defocus)
      get_defocus = lib->sensor_custom_API.pdlib_get_defocus;
    else
      get_defocus = &PdLibGetDefocus;
    rc = sensor_custom_API->sensor_custom_calc_defocus(data, get_defocus);
  } else
    /* T1 using common function, T2 and T3 */
    rc = pdaf_calc_defocus(sctrl, data);

  if(rc < 0)
    SERR("pdaf defocus calculation fail");

  return rc;
}

static int32_t sensor_set_exp_multiplier(void *sctrl)
{
  sensor_ctrl_t *ctrl;
  sensor_lib_t  *sensor_lib;
  uint32_t       base_gain;

  RETURN_ERROR_ON_NULL(sctrl);

  ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib = ctrl->lib_params->sensor_lib_ptr;

  /* calculate exposure multiplier based on iso100 gain */
  base_gain = sensor_lib->aec_info.min_gain * 100 / ctrl->s_data->iso100_gain;

  ctrl->s_data->exp_multiplier = (float)base_gain /
    (ctrl->s_data->sensor_common_info->camera_config.lens_info.f_number *
     ctrl->s_data->sensor_common_info->camera_config.lens_info.f_number);

  SLOW("iso 100 gain %f exp_multiplier %f",
    ctrl->s_data->iso100_gain, ctrl->s_data->exp_multiplier);

  return SENSOR_SUCCESS;
}

static int32_t sensor_get_exp_multiplier(void *sctrl, void *data)
{
  sensor_ctrl_t *ctrl;
  float         *exp_multiplier;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  ctrl = (sensor_ctrl_t *)sctrl;
  exp_multiplier = (float *)data;

  *exp_multiplier = ctrl->s_data->exp_multiplier;

  return SENSOR_SUCCESS;
}

static int32_t sensor_get_filter_arrangement(void *sctrl, void *data)
{
  sensor_ctrl_t             *ctrl;
  sensor_filter_arrangement *filter_arrangement;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  ctrl = (sensor_ctrl_t *)sctrl;
  filter_arrangement = (sensor_filter_arrangement *)data;

  *filter_arrangement =
    ctrl->lib_params->sensor_lib_ptr->sensor_output.filter_arrangement;
  return SENSOR_SUCCESS;
}

static int32_t sensor_get_raw_output_bit_width(
  void *sctrl, void *data)
{
  sensor_ctrl_t       *ctrl;
  sensor_raw_output_t *out_bit_width;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  ctrl = (sensor_ctrl_t *)sctrl;
  out_bit_width = (sensor_raw_output_t *)data;

  *out_bit_width =
    ctrl->lib_params->sensor_lib_ptr->sensor_output.raw_output;
  return SENSOR_SUCCESS;
}

static int32_t sensor_set_cur_fll(void *sctrl, void *data)
{
  sensor_ctrl_t                    *ctrl;
  struct sensor_lib_out_info_array *out_info_array;
  int16_t                           cur_res;
  uint16_t                          new_fll;
  float                             new_frame_duration;
  uint32_t                          vt_pix_clk = 0;
  sensor_dual_exp_t                *dual_exp = NULL;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  ctrl = (sensor_ctrl_t *)sctrl;
  dual_exp = (sensor_dual_exp_t*) data;
  new_frame_duration = dual_exp->stats_update->aec_update.exp_time;

  out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;
  cur_res = ctrl->s_data->cur_res;
  vt_pix_clk = SENSOR_GET_VT_PIK_CLK(&out_info_array->out_info[cur_res]);

  if (new_frame_duration == 0) {
    /* back to initial value */
    ctrl->s_data->cur_frame_length_lines =
      out_info_array->out_info[cur_res].frame_length_lines;
  } else {
    new_fll = (uint16_t)(((float)dual_exp->master_lc *
      out_info_array->out_info[cur_res].frame_length_lines) /
      ((float)dual_exp->master_fl_lines));

    if (new_fll < out_info_array->out_info[cur_res].frame_length_lines)
      new_fll = out_info_array->out_info[cur_res].frame_length_lines;

    ctrl->s_data->cur_frame_length_lines = new_fll;
  }

  SLOW("updated cur_fll %d", ctrl->s_data->cur_frame_length_lines);

  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_set_dualcam_settings -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_dualcam_settings(void *sctrl, void* data)
{
  int32_t  rc = 0;
  struct   sensorb_cfg_data cfg;
  int32_t  dual_mode;
  char     prop[PROPERTY_VALUE_MAX];
  uint32_t enable_hw_sync;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  dual_mode = *(int32_t *)data;

  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.camera.hwsync.enable", prop, "1");
  enable_hw_sync  = atoi(prop);

  if(!enable_hw_sync){
    SHIGH("[dual] HW-Sync disabled");
    return SENSOR_SUCCESS;
  }

  SHIGH("sensor_set_dualcam_settings for %s \n",
    lib->sensor_lib_ptr->sensor_slave_info.sensor_name);

  if (ctrl->s_data->fd < 0)
    return SENSOR_FAILURE;

  if(dual_mode == CAM_MODE_PRIMARY){
  SLOW("[dual] Configuring PRIMARY sensor");
  rc = sensor_write_i2c_setting_array(ctrl,
    &(ctrl->lib_params->sensor_lib_ptr->dualcam_master_settings));
  }
  else{
  SLOW("[dual] Configuring SECONDARY sensor");
  rc = sensor_write_i2c_setting_array(ctrl,
    &(ctrl->lib_params->sensor_lib_ptr->dualcam_slave_settings));
  }

  if (rc)
    SERR("sensor_write_i2c_setting_array failed");

  return rc;
}

static int32_t sensor_set_formatted_cal_data(void *sctrl, void *data)
{
  sensor_ctrl_t                    *ctrl = (sensor_ctrl_t *)sctrl;
  int                               rc = SENSOR_SUCCESS;

  SLOW("ENTER");
  ctrl->s_data->eeprom_data = data;
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_set_standby_stream -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_set_standby_stream(void *sctrl)
{
  int32_t rc = 0;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  sensor_lib_params_t *lib = (sensor_lib_params_t *)ctrl->lib_params;
  struct sensorb_cfg_data cfg;

  SHIGH("Sensor standby stream for %s \n",
    lib->sensor_lib_ptr->sensor_slave_info.sensor_name);

  if (ctrl->s_data->fd < 0) {
    SERR("stop_stream failed invalid fd = %d", ctrl->s_data->fd);
    return SENSOR_FAILURE;
  }

  ctrl->s_data->lock_aec = 0;

  rc = sensor_write_i2c_setting_array(ctrl,
    &(ctrl->lib_params->sensor_lib_ptr->stop_settings));
  if (rc)
    SERR("sensor_write_i2c_setting_array failed");

  SLOW("exit");
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_adjust_dualc_slave_linecount -
 *
 * DESCRIPTION: When the linecount from AEC update is greater
 *              than the adjusted fll of the slave sensor then
 *              update the linecount on the slave sensor to
 *              maintain same fps on master and slave, thereby
 *              maintaining HW-Sync. If the frame rate on slave
 *              drops lower than master then HW-Sync will break.
 *
 *==========================================================*/
static int32_t sensor_adjust_dualc_slave_linecount(void *sctrl, void *data)
{
    int32_t rc = SENSOR_SUCCESS;
    uint32_t linecount;
    sensor_ctrl_t *ctrl = NULL;
    sensor_lib_params_t *lib = NULL;
    float exp_time = 0.0;
    struct sensor_lib_out_info_array *out_info_array = NULL;
    int16_t                           cur_res;
    sensor_dual_exp_t                *dual_exp = NULL;

    RETURN_ERROR_ON_NULL(sctrl);
    RETURN_ERROR_ON_NULL(data);

    ctrl = (sensor_ctrl_t *)sctrl;
    lib = (sensor_lib_params_t *)ctrl->lib_params;
    out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;
    cur_res = ctrl->s_data->cur_res;

    dual_exp = (sensor_dual_exp_t*) data;
    linecount = dual_exp->stats_update->aec_update.linecount;

    if (linecount > ctrl->s_data->cur_frame_length_lines) {
      SERR("Adjusted %s linecount to maintain frame rate from %d to %d",
        lib->sensor_lib_ptr->sensor_slave_info.sensor_name,
        linecount, ctrl->s_data->cur_frame_length_lines);

      linecount = ctrl->s_data->cur_frame_length_lines;

      exp_time = (float)linecount /
        ((float)out_info_array->out_info[cur_res].frame_length_lines *
        out_info_array->out_info[cur_res].max_fps);

      dual_exp->stats_update->aec_update.sensor_gain *=
        (dual_exp->stats_update->aec_update.exp_time / exp_time);

      if (dual_exp->stats_update->aec_update.sensor_gain < 1) {
        linecount *=
          dual_exp->stats_update->aec_update.sensor_gain;
        dual_exp->stats_update->aec_update.sensor_gain = 1;
        if (linecount == 0) linecount = 1;
      }
      dual_exp->stats_update->aec_update.linecount = linecount;
    }

    return rc;
}


/*===========================================================================
 * FUNCTION    - sensor_alternative_slave -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t sensor_alternative_slave(void *sctrl, void* data)
{
  struct sensorb_cfg_data           cfg;
  struct msm_camera_i2c_reg_setting settings;
  struct msm_camera_i2c_reg_array   reg_array;
  sensor_ctrl_t                     *ctrl = (sensor_ctrl_t *) sctrl;
  sensor_lib_params_t               *lib  = ctrl->lib_params;
  unsigned short                    sensor_slave_addr = *(unsigned short *) data;

  struct camera_alt_slave_info *sensor_alt_slave_info =
    &lib->sensor_lib_ptr->sensor_slave_info.sensor_alt_slave_info;

  SLOW("enter");

  SLOW("Sensor slave addr: 0x%x", sensor_slave_addr);

  if(sensor_slave_addr == 0) {
    return SENSOR_SUCCESS;
  }

  if(sensor_alt_slave_info->alt_slave_mode == SENSOR_ALT_SLAVE_DUAL) {
    if(sensor_slave_addr == sensor_alt_slave_info->slave1_addr) {
      reg_array.reg_addr = sensor_alt_slave_info->slave2_reg;
      reg_array.reg_data = sensor_alt_slave_info->slave1_addr;
      reg_array.delay = 0;
      SLOW("Override alt slave2: 0x%x", sensor_alt_slave_info->slave2_addr);
    } else if (sensor_slave_addr == sensor_alt_slave_info->slave2_addr) {
      reg_array.reg_addr = sensor_alt_slave_info->slave1_reg;
      reg_array.reg_data = sensor_alt_slave_info->slave2_addr;
      reg_array.delay = 0;
      SLOW("Override alt slave1: 0x%x", sensor_alt_slave_info->slave1_addr);
    }

    settings.addr_type = MSM_CAMERA_I2C_WORD_ADDR;
    settings.data_type = MSM_CAMERA_I2C_WORD_DATA;
    settings.delay = 0;
    settings.reg_setting = &reg_array;
    settings.size = 1;

    cfg.cfg.setting = &settings;
    cfg.cfgtype = CFG_WRITE_I2C_ARRAY;

    if (ioctl(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg) < 0) {
      SLOW("failed");
      return 0;
    }
  }

  SLOW("exit");
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_dualc_apply_phase_correction -
 *
 * DESCRIPTION: phase_diff is the difference between slave and master timestamp
*               Positive phase difference indicates slave is lagging
*               we will have to extend the fll such that one frame is skipped
*               so that the 6th frame is potentially in sync.
*
*               Negative phase difference indicates slave is leading
*               we will have to extend the fll only a little bit,
*               no frame will be skipped
*               so that the 5th frame is potentially in sync.
 *==========================================================*/
static int32_t sensor_dualc_apply_phase_correction(void *sctrl, void* data)
{

  int32_t rc = SENSOR_SUCCESS;
  int32_t phase_diff = 0;
  sensor_ctrl_t *ctrl = NULL;
  uint32_t frame_time = 0, fps = 0, pdiff = 0;
  float ffps = 0.0;
  struct sensor_lib_out_info_array *out_info_array;
  int16_t  cur_res = 0;
  uint16_t pc_fll = 0;

  RETURN_ERROR_ON_NULL(sctrl);
  RETURN_ERROR_ON_NULL(data);

  ctrl = (sensor_ctrl_t *)sctrl;
  phase_diff = *(int32_t*)data;

  out_info_array = &ctrl->lib_params->sensor_lib_ptr->out_info_array;
  cur_res = ctrl->s_data->cur_res;

  sensor_get_cur_fps(sctrl, &fps);
  ffps = (float)(fps / Q8);

  /* frame_time in ms */
  frame_time = (uint32_t) ( (1 / ffps) * 1000);
  SLOW("fps = %f, frame_time = %d", ffps, frame_time);

  if(phase_diff > 0){
      pdiff = phase_diff;
      phase_diff = frame_time - phase_diff;

      pc_fll = (out_info_array->out_info[cur_res].frame_length_lines / frame_time) *
        phase_diff;

  } else {
      pdiff = (phase_diff * (-1));
      pc_fll = (out_info_array->out_info[cur_res].frame_length_lines / frame_time) *
        (pdiff);
  }

  /* This ensures that if the phase difference was calculated
   * between two consecutive pairs, then no correction will
   * be applied.
   * This would mean that frame idx for slave sensor was updated
   * before slave C-SOF was received.
  */
  if(frame_time == pdiff){
    pc_fll = 0;
  }

  ctrl->s_data->cur_frame_length_lines =
    ctrl->s_data->last_updated_fll + pc_fll;

  SLOW("phase_diff = %d, adjustment %d, updated fll = %d",
       phase_diff, pc_fll, ctrl->s_data->cur_frame_length_lines);

  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_process -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_process(void *sctrl,
  sensor_submodule_event_type_t event, void *data)
{
  int32_t rc = SENSOR_SUCCESS;
  if (!sctrl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  SLOW("sctrl %p event %d", sctrl, event);
  switch (event) {
  /* Get enums */
  case SENSOR_GET_CAPABILITIES:
    rc = sensor_get_capabilities(sctrl, data);
    break;
  case SENSOR_GET_CUR_CSI_CFG:
    rc = sensor_get_cur_csi_cfg(sctrl, data);
    break;
  case SENSOR_GET_CUR_CHROMATIX_NAME:
    rc = sensor_get_cur_chromatix_name(sctrl, data);
    break;
  case SENSOR_GET_INTEGRATION_TIME:
    rc = sensor_get_integration_time(sctrl, data);
    break;
  case SENSOR_GET_CUR_FPS:
    rc = sensor_get_cur_fps(sctrl, data);
    break;
  case SENSOR_GET_RESOLUTION_INFO:
    rc = sensor_get_resolution_info(sctrl, data);
    break;
  case SENSOR_GET_SENSOR_PORT_INFO:
    rc = sensor_get_sensor_port_info(sctrl, data);
    break;
  case SENSOR_GET_DIGITAL_GAIN:
    rc = sensor_get_digital_gain(sctrl, data);
    break;
  case SENSOR_GET_AEC_DIGITAL_GAIN:
    rc = sensor_get_aec_digital_gain(sctrl, data);
    break;
  case SENSOR_GET_SENSOR_FORMAT:
    rc = sensor_get_sensor_format(sctrl, data);
    break;
  case SENSOR_GET_PROPERTY:
    rc = sensor_get_property(sctrl, data);
    break;
  case SENSOR_GET_RAW_DIMENSION:
    rc = sensor_get_raw_dimension(sctrl, data);
    break;
  case SENSOR_GET_BET_METADATA:
    rc = sensor_get_bet_metadata(sctrl, data);
    break;
  case SENSOR_GET_EXPOSURE_TIME:
    rc = sensor_get_exposure_time(sctrl, data);
    break;
  case SENSOR_GET_SENSITIVITY:
    rc = sensor_get_sensitivity(sctrl, data);
    break;
  case SENSOR_GET_FRAME_DURATION:
    rc = sensor_get_frame_duration(sctrl, data);
    break;
  case SENSOR_GET_OUTPUT_INFO:
    rc = sensor_get_output_info(sctrl, data);
    break;
  case SENSOR_GET_EXPOSURE_START_TIME:
    rc = sensor_get_exposure_starttime(sctrl, data);
    break;
  case SENSOR_GET_TEST_PATTERN_DATA:
    rc = sensor_get_test_pattern_data(sctrl, data);
    break;
  case SENSOR_GET_CSID_TEST_MODE:
    rc = sensor_get_csid_test_mode(sctrl, data);
    break;
  case SENSOR_GET_EXP_MULTIPLIER:
    rc = sensor_get_exp_multiplier(sctrl, data);
    break;
  case SENSOR_GET_META_OUT_DIM:
    rc = sensor_get_meta_out_dim(sctrl, data);
    break;
  case SENSOR_GET_FILTER_ARRANGEMENT:
    rc = sensor_get_filter_arrangement(sctrl, data);
    break;
  case SENSOR_GET_RAW_OUTPUT_BIT_WIDTH:
    rc = sensor_get_raw_output_bit_width(sctrl, data);
    break;

  /* Set enums */
  case SENSOR_SET_LIB_PARAMS:
    rc = sensor_set_lib_params(sctrl, data);
    break;
  case SENSOR_INIT:
    rc = sensor_init(sctrl);
    break;
  case SENSOR_STOP_STREAM:
    rc = sensor_set_stop_stream(sctrl);
    break;
  case SENSOR_START_STREAM:
    rc = sensor_set_start_stream(sctrl);
    break;
  case SENSOR_DELAYED_START_STREAM:
    rc = sensor_set_delayed_start_stream(sctrl);
    break;
  case SENSOR_SET_RESOLUTION:
    rc = sensor_set_resolution(sctrl, data);
    break;
  case SENSOR_SET_AEC_UPDATE:
    rc = sensor_set_aec_update(sctrl, data);
    break;
  case SENSOR_SET_AEC_MANUAL_UPDATE:
    rc = sensor_set_aec_manual_update(sctrl, data);
    break;
  case SENSOR_SET_AEC_UPDATE_FOR_DUAL:
    rc = sensor_set_aec_update_for_dual(sctrl, data);
    break;
  case SENSOR_SET_AEC_UPDATE_FOR_SLAVE:
    rc = sensor_set_aec_update_for_slave(sctrl, data);
    break;
  case SENSOR_GET_EXPOSURE_INFO:
    rc = sensor_get_exposure_info(sctrl, data);
    break;
  case SENSOR_SET_AWB_UPDATE:
    rc = sensor_set_awb_update(sctrl, data);
    break;
  case SENSOR_SET_AEC_INIT_SETTINGS:
    rc = sensor_set_aec_init_settings(sctrl, data);
    break;
  case SENSOR_SET_VFE_SOF:
    rc = sensor_set_vfe_sof(sctrl, data);
    break;
  case SENSOR_SET_FPS:
    rc = sensor_set_frame_rate(sctrl, data);
    break;
  case SENSOR_SET_HFR_MODE:
    rc = sensor_set_hfr_mode(sctrl, data);
    break;
  case SENSOR_SET_QUADRA_MODE:
    rc = sensor_set_quadra_mode(sctrl, data);
    break;
  case SENSOR_SET_HDR_AE_BRACKET:
    rc = sensor_set_hdr_ae_bracket(sctrl, data);
    break;
  case SENSOR_SET_HDR_ZSL_MODE:
    rc = sensor_set_hdr_zsl_mode(sctrl, data);
    break;
  case SENSOR_SET_MANUAL_EXPOSURE_MODE:
    rc = sensor_set_manual_exposure_mode(sctrl, data);
    break;
  case SENSOR_SET_HDR_MODE:
    rc = sensor_set_hdr_mode(sctrl, data);
    break;
  case SENSOR_SET_DIS_ENABLE:
    rc = sensor_set_dis_enable(sctrl, data);
    break;
  case SENSOR_SET_OP_PIXEL_CLK_CHANGE:
    rc = sensor_set_op_pixel_clk_change(sctrl, data);
    break;
  case SENSOR_SET_CALIBRATION_DATA:
    rc = sensor_set_calibration_data(sctrl, data);
    break;
  case SENSOR_SET_SATURATION:
    rc = sensor_set_saturation(sctrl, data);
    break;
  case SENSOR_SET_CONTRAST:
    rc = sensor_set_contrast(sctrl, data);
    break;
  case SENSOR_SET_SHARPNESS:
    rc = sensor_set_sharpness(sctrl, data);
    break;
  case SENSOR_SET_AUTOFOCUS:
    rc = sensor_set_autofocus(sctrl, data);
    break;
  case SENSOR_CANCEL_AUTOFOCUS:
    //rc = sensor_cancel_autofocus(sctrl, data);
    break;
  case SENSOR_SET_ISO:
    rc = sensor_set_iso(sctrl, data);
    break;
  case SENSOR_SET_EXPOSURE_COMPENSATION:
    rc = sensor_set_exposure_compensation(sctrl, data);
    break;
  case SENSOR_SET_MANUAL_EXPOSURE_TIME:
    rc = sensor_set_manual_exposure_time(sctrl, data);
    break;
  case SENSOR_SET_ANTIBANDING:
    rc = sensor_set_antibanding(sctrl, data);
    break;
  case SENSOR_SET_EFFECT:
    rc = sensor_set_effect(sctrl, data);
    break;
  case SENSOR_SET_WHITE_BALANCE:
    rc = sensor_set_white_balance(sctrl, data);
    break;
  case SENSOR_SET_MAX_DIMENSION:
    rc = sensor_set_max_dimension(sctrl, data);
    break;
  case SENSOR_SET_FRAME_DURATION:
    rc = sensor_set_frame_duration(sctrl, data);
    break;
  case SENSOR_SET_HAL_VERSION:
    rc = sensor_set_hal_version(sctrl, data);
    break;
  case SENSOR_SET_CAPTURE_INTENT:
    rc = sensor_set_capture_intent(sctrl, data);
    break;
  case SENSOR_SET_AEC_ZSL_SETTINGS:
    rc = sensor_set_aec_zsl_settings(sctrl, data);
    break;
  case SENSOR_GET_FAST_AEC_WAIT_FRAMES:
    rc = sensor_get_fast_aec_wait_frames(sctrl, data);
    break;
  case SENSOR_SET_TEST_PATTERN:
    rc = sensor_set_test_pattern(sctrl, data);
    break;
  case SENSOR_PDAF_PARSE_PD:
    rc = sensor_pdaf_parse_pd(sctrl, data);
    break;
  case SENSOR_PDAF_CAL_DEFOCUS:
    rc = sensor_pdaf_cal_defocus(sctrl, data);
    break;
  case SENSOR_SET_DELAY_CFG:
    rc = sensor_set_delay_cfg(sctrl, data);
    break;
  case SENSOR_GET_DELAY_CFG:
    rc = sensor_get_delay_cfg(sctrl, data);
    break;
  case SENSOR_SET_EXP_MULTIPLIER:
    rc = sensor_set_exp_multiplier(sctrl);
    break;
  case SENSOR_SET_CUR_FLL:
    rc = sensor_set_cur_fll(sctrl, data);
    break;
  case SENSOR_PDAF_INIT:
    rc = pdaf_init(sctrl, data);
    break;
  case SENSOR_SET_FORMATTED_CAL_DATA:
    rc = sensor_set_formatted_cal_data(sctrl, data);
    break;
  case SENSOR_PDAF_SET_BUF_DATA_TYPE:
    rc = pdaf_set_buf_data_type(sctrl, data);
    break;
  case SENSOR_PDAF_DEINIT:
    rc = pdaf_deinit(sctrl);
    break;
  case SENSOR_PDAF_GET_TYPE:
    rc = pdaf_get_type(sctrl, data);
    break;
  case SENSOR_PDAF_GET_NATIVE_PATTERN:
    rc = pdaf_get_native_pattern(sctrl, data);
    break;
  case SENSOR_PDAF_SET_AF_WINDOW:
    rc = pdaf_set_window_update(sctrl, data);
    break;
  case SENSOR_GET_SENSOR_MODE:
     rc = sensor_get_sensor_mode(sctrl, data);
     break;
  case SENSOR_SET_ISO100:
     rc = sensor_set_iso100(sctrl, data);
     break;
  case SENSOR_SET_DUALCAM_SETTINGS:
    rc = sensor_set_dualcam_settings(sctrl, data);
    break;
  case SENSOR_SET_META_DIM:
    rc = sensor_set_meta_dim(sctrl, data);
    break;
  case SENSOR_STANDBY_STREAM:
    rc = sensor_set_standby_stream(sctrl);
    break;
  case SENSOR_SET_ALTERNATIVE_SLAVE:
    rc = sensor_alternative_slave(sctrl, data);
    break;
  case SENSOR_ADJUST_DUALC_SLAVE_LINECOUNT:
    rc = sensor_adjust_dualc_slave_linecount(sctrl, data);
    break;
  case SENSOR_DUALC_APPLY_PHASE_CORRECTION:
     rc = sensor_dualc_apply_phase_correction(sctrl, data);
    break;
    default:
    SERR("invalid event %d", event);
    rc = SENSOR_FAILURE;
   break;
  }
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_close -
 *
 * DESCRIPTION:
 *==========================================================*/
static int32_t sensor_close(void *sctrl)
{
  int32_t rc = SENSOR_SUCCESS;
  sensor_ctrl_t *ctrl = (sensor_ctrl_t *)sctrl;
  struct sensorb_cfg_data cfg;
  RETURN_ERROR_ON_NULL(ctrl);

  cfg.cfgtype = CFG_POWER_DOWN;
  rc = LOG_IOCTL(ctrl->s_data->fd, VIDIOC_MSM_SENSOR_CFG, &cfg, "close");
  if (rc < 0) {
    SERR("VIDIOC_MSM_SENSOR_CFG failed");
  }

  /* close subdev */
  close(ctrl->s_data->fd);

  if (ctrl->s_data->exposure_regs)
    free(ctrl->s_data->exposure_regs);

  if (ctrl->s_data->fps_regs)
    free(ctrl->s_data->fps_regs);

  free(ctrl->s_data);
  free(ctrl);
  return rc;
}

/*==========================================================
 * FUNCTION    - sensor_sub_module_init -
 *
 * DESCRIPTION:
 *==========================================================*/
int32_t sensor_sub_module_init(sensor_func_tbl_t *func_tbl)
{
  if (!func_tbl) {
    SERR("failed");
    return SENSOR_FAILURE;
  }
  func_tbl->open = sensor_open;
  func_tbl->process = sensor_process;
  func_tbl->close = sensor_close;
  return SENSOR_SUCCESS;
}

/*==========================================================
 * FUNCTION    - sensor_get_hfr_mode_fps -
 *
 * DESCRIPTION: Convert HFR mode enum to fps value
 *==========================================================*/
float sensor_get_hfr_mode_fps(cam_hfr_mode_t mode) {

  switch (mode) {
  case CAM_HFR_MODE_60FPS: return 60.0f;
  case CAM_HFR_MODE_90FPS: return 90.0f;
  case CAM_HFR_MODE_120FPS: return 120.0f;
  case CAM_HFR_MODE_150FPS: return 150.0f;
  default:
    SERR("Error: Invalid HFR mode");
    return 0.0f;
  }
}
