/**********************************************************************
*  Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "module_imgbase.h"

/**
 *  Static functions
 **/
static int32_t module_sw2d_client_created(imgbase_client_t *p_client);
static int32_t module_sw2d_update_meta(imgbase_client_t *p_client, img_meta_t *p_meta);

/** g_caps:
 *
 *  Set the capabilities for module
*/
static img_caps_t g_caps = {
  .num_input = 1,
  .num_output = 1,
  .num_meta = 1,
  .inplace_algo = 0,
  .num_release_buf = 0,
  .use_internal_bufs = 1,
};

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_client_created = module_sw2d_client_created,
  .imgbase_client_update_meta = module_sw2d_update_meta,
  .exec_mode = IMG_EXECUTION_SW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .force_cache_op = FALSE,
};

/**
 * Function: module_sw2d_client_created
 *
 * Description: function called after client creation
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_sw2d_client_created(imgbase_client_t *p_client)
{
  IDBG_MED("%s %d: E", __func__, __LINE__);
  p_client->before_cpp = TRUE;

  return IMG_SUCCESS;
}

/**
 * Function: module_sw2d_update_meta
 *
 * Description: This function is used to called when the base
 *   module updates the metadata
 *
 * Arguments:
 *   @p_client - pointer to imgbase client
 *   @p_meta - pointer to the image meta
 *
 * Return values:
 *   error values
 *
 * Notes: none
 **/
int32_t module_sw2d_update_meta(imgbase_client_t *p_client,
  img_meta_t *p_meta)
{
  int rc = IMG_SUCCESS;
  uint32_t sw2d_mask;

  if (!p_client || !p_meta) {
    IDBG_ERROR("%s:%d] Error, invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  sw2d_mask = SW2D_OPS_DOWNSCALE;
  rc = img_set_meta(p_meta, IMG_META_SW2D_OPS, &sw2d_mask);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Error, rc %d", __func__, __LINE__, rc);
    return IMG_ERR_INVALID_INPUT;
  }
  IDBG_HIGH("%s:%d] sw2d mask %d", __func__, __LINE__, sw2d_mask);

  return IMG_SUCCESS;
}

/**
 * Function: module_sw2d_deinit
 *
 * Description: This function is used to deinit sw2d module
 *
 * Arguments:
 *   @p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_sw2d_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}

/** module_sw2d_init:
 *
 * Arguments:
 *   @name - name of the module
 *
 * Description: This function is used to initialize the sw2d
 *   module
 *
 * Return values:
 *   MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_sw2d_init(const char *name)
{
  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    NULL,
    &g_caps,
    "libmmcamera_sw2d_lib.so",
    CAM_QCOM_FEATURE_SW2D,
    &g_params);
}

/** module_sw2d_set_parent:
 *
 * Arguments:
 *   @p_mct_mod - MCT module instance pointer
 *   @p_parent - parent module pointer
 *
 * Description: This function is used to set the parent pointer
 *   of the module
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_sw2d_set_parent(mct_module_t *p_mct_mod, mct_module_t *p_parent)
{
  return module_imgbase_set_parent(p_mct_mod, p_parent);
}

