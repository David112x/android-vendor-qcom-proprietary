/**********************************************************************
*  Copyright (c) 2013-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "module_imgbase.h"

/** CF_DYN_PARAMS:
 *
 * Defines whether to use tuning parameters from setprop
 *
 * use tuning parameters from setprop
 **/
//#define CF_DYN_PARAMS

/** MODULE_CHROMA_FLASH_VERSION_2:
 *
 * Defines module chroma flash version 2
 *
 * module chroma flash version 2
 **/
#define MODULE_CHROMA_FLASH_VERSION_2 (2)

/** MODULE_CHROMA_FLASH_VERSION_3:
 *
 * Defines module chroma flash version 3
 *
 * module chroma flash version 3
 **/
#define MODULE_CHROMA_FLASH_VERSION_3 (3)

/**
 *  Static functions
 **/
static int32_t module_chroma_flash_update_meta(imgbase_client_t *,
  img_meta_t *);
static boolean module_chroma_flash_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* /*p_mod*/,unsigned int sessionid);

/** g_override_cf_default_params:
 *
 *  flag to control whether to override default parameters or
 *  not
*/
static int32_t g_override_cf_default_params = TRUE;

/** g_cf_ctrl:
 *
 *  if g_override_cf_default_params is set to TRUE,
 *  use these values to override the chromaflash default
 *  parameters
*/
static img_chromaflash_ctrl_t g_cf_ctrl = {
  .deghost_enable = TRUE,
  .flash_weight = 5.0,
  .br_intensity = 1.0,
  .contrast_enhancement = 0.5,
  .sharpen_beta = 0.5,
  .br_color = 0.2,
  .max_amb_lux_idx = 400.0,
};

/** g_caps:
 *
 *  Set the capabilities for chroma flash module
*/
static img_caps_t g_caps = {
  .num_input = 2,
  .num_output = 1,
  .num_meta = 2,
  .inplace_algo = 0,
  .num_release_buf = 1,
};

/** g_caps3:
 *
 *  Set the capabilities for chroma flash 3 module
*/
static img_caps_t g_caps3 = {
  .num_input = 3,
  .num_output = 1,
  .num_meta = 3,
  .inplace_algo = 0,
  .num_release_buf = 1,
};

/** g_lib_version:
 *
 *  Specifies chroma flash lirbary version
*/
static uint32_t g_lib_version;

/**
 * Function: module_chroma_flash_get_dyn_cf_params
 *
 * Description: This function is used to called get chroma flash params for tuning
 *
 * Arguments:
 *   @p_cf_ctrl: chromaflash control
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/

void module_chroma_flash_get_dyn_cf_params(img_chromaflash_ctrl_t *p_cf_ctrl)
{
  char value[PROPERTY_VALUE_MAX];

  property_get("persist.vendor.imglib.cf.tune", value, "0");
  g_override_cf_default_params = atoi(value);

  if (g_override_cf_default_params) {
    property_get("persist.vendor.imglib.cf.deghost", value, "1");
    p_cf_ctrl->deghost_enable = atoi(value);
    property_get("persist.vendor.imglib.cf.flash_weight", value, "5.0");
    p_cf_ctrl->flash_weight = atof(value);
  }
}


/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_client_update_meta = module_chroma_flash_update_meta,
  .imgbase_query_mod = module_chroma_flash_query_mod,
  .exec_mode = IMG_EXECUTION_SW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .force_cache_op = FALSE,
};

/**
 * Function: module_chroma_flash_query_mod
 *
 * Description: This function is used to query chroma flash
 *               caps
 *
 * Arguments:
 *   @p_mct_cap - capababilities
 *   @p_mod - pointer to the module
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_chroma_flash_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* p_mod,unsigned int sessionid)
{
  mct_pipeline_imaging_cap_t *buf;

  IMG_UNUSED(p_mod);
  IMG_UNUSED(sessionid);
  if (!p_mct_cap) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return FALSE;
  }

  buf = &p_mct_cap->imaging_cap;
  memset(&buf->chroma_flash_settings_need, 0,
    sizeof(buf->chroma_flash_settings_need));
  if (MODULE_CHROMA_FLASH_VERSION_3 == g_lib_version) {
    buf->chroma_flash_settings_need.flash_bracketing[0] = 0;
    buf->chroma_flash_settings_need.flash_bracketing[1] = 1;
    buf->chroma_flash_settings_need.flash_bracketing[2] = 0;
    buf->chroma_flash_settings_need.output_count = g_caps3.num_output;
    buf->chroma_flash_settings_need.burst_count = g_caps3.num_input;
    buf->chroma_flash_settings_need.metadata_index = 1;
  } else {
    buf->chroma_flash_settings_need.flash_bracketing[0] = 1;
    buf->chroma_flash_settings_need.flash_bracketing[1] = 0;
    buf->chroma_flash_settings_need.output_count = g_caps.num_output;
    buf->chroma_flash_settings_need.burst_count = g_caps.num_input;
    buf->chroma_flash_settings_need.metadata_index = 0;
  }

  return TRUE;
}

/**
 * Function: module_chroma_flash_update_meta
 *
 * Description: This function is used to called when the base
 *                       module updates the metadata
 *
 * Arguments:
 *   @p_client - pointer to imgbase client
 *   @p_meta: pointer to the image meta
 *
 * Return values:
 *     error values
 *
 * Notes: none
 **/
int32_t module_chroma_flash_update_meta(imgbase_client_t *p_client,
  img_meta_t *p_meta)
{
  int rc;
  if (!p_client || !p_meta) {
    IDBG_ERROR("%s:%d] invalid input %p %p", __func__, __LINE__,
      p_client, p_meta);
    rc = IMG_ERR_INVALID_INPUT;
    goto error;
  }

#ifdef CF_DYN_PARAMS
  module_chroma_flash_get_dyn_cf_params(&g_cf_ctrl);
#endif

  if (g_override_cf_default_params) {
    rc = img_set_meta(p_meta, IMG_META_CHROMAFLASH_CTRL, &g_cf_ctrl);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
      goto error;
    }
  }
  IDBG_HIGH("%s:%d] Success %d", __func__, __LINE__, g_override_cf_default_params);
  return IMG_SUCCESS;

error:
  return rc;
}

/**
 * Function: module_chroma_flash_deinit
 *
 * Description: This function is used to deinit chroma flash
 *               module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_chroma_flash_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}

/** module_chroma_flash_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the chroma
 *            flash module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_chroma_flash_init(const char *name)
{
  char value[PROPERTY_VALUE_MAX];
  char default_value[PROPERTY_VALUE_MAX];
  void* caps;

  snprintf(default_value, sizeof(default_value), "%d",
    MODULE_CHROMA_FLASH_VERSION_3);
  property_get("persist.vendor.camera.imglib.cf.ver", value, default_value);
  g_lib_version = atoi(value);

  if (MODULE_CHROMA_FLASH_VERSION_3 == g_lib_version) {
    caps = &g_caps3;
  } else {
    caps = &g_caps;
  }

  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    NULL,
    caps,
    "libmmcamera_chromaflash_lib.so",
    CAM_QCOM_FEATURE_CHROMA_FLASH,
    &g_params);
}
