/***************************************************************************
* Copyright (c) 2013-2016 Qualcomm Technologies, Inc.                      *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/

#include "module_imgbase.h"

/** OPTIZOOM_ZOOM_THRESHOLD:
 *
 *  Zoom threshold. 1.1f represents 1.1x
 **/
#define OPTIZOOM_ZOOM_THRESHOLD (1.1f)

/** OPTIZOOM_BURST_CNT:
 *
 *  Burst count
 **/
#define OPTIZOOM_BURST_CNT      8

/**
 *  Static functions
 **/
static boolean module_optizoom_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* /*p_mod*/,unsigned int sessionid);

/** g_caps:
 *
 *  Set the capabilities for optizoom module
*/
static img_caps_t g_caps = {
  .num_input = OPTIZOOM_BURST_CNT,
  .num_output = 1,
  .num_meta = 1,
  .inplace_algo = 0,
  .num_release_buf = 1,
};

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_query_mod = module_optizoom_query_mod,
  .exec_mode = IMG_EXECUTION_SW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .force_cache_op = FALSE,
};

/**
 * Function: module_optizoom_deinit
 *
 * Description: This function is used to deinit optizoom
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
void module_optizoom_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}

/**
 * Function: module_optizoom_query_mod
 *
 * Description: This function is used to query optizoom
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
static boolean module_optizoom_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* p_mod,unsigned int sessionid )
{
  mct_pipeline_imaging_cap_t *buf;

  IMG_UNUSED(p_mod);
  IMG_UNUSED(sessionid);
  if (!p_mct_cap) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return FALSE;
  }

  buf = &p_mct_cap->imaging_cap;
  buf->opti_zoom_settings.zoom_threshold =
    (uint8_t)module_imglib_common_get_zoom_level(p_mct_cap,
    OPTIZOOM_ZOOM_THRESHOLD);
  buf->opti_zoom_settings.burst_count = OPTIZOOM_BURST_CNT;
  buf->opti_zoom_settings.enable = TRUE;

  return TRUE;
}

/** module_optizoom_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the optizoom
 *             module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_optizoom_init(const char *name)
{
  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    NULL,
    &g_caps,
    "libmmcamera_optizoom_lib.so",
    CAM_QCOM_FEATURE_OPTIZOOM,
    &g_params);
}

