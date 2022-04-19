/**********************************************************************
*  Copyright (c) 2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "module_imgbase.h"

#define MAX_MFPROC_FRAMECOUNT 5

/**
 *  Static functions
 **/
static boolean module_mfproc_precpp_query_mod(mct_pipeline_cap_t* p_mct_cap,
  void* p_mod, unsigned int sessionid);
static int32_t module_mfproc_precpp_client_created(imgbase_client_t *p_client);

/** g_caps:
 *
 *  Set the capabilities for mfns module
*/
static img_caps_t g_caps = {
  .num_input = MAX_MFPROC_FRAMECOUNT,
  .num_output = 1,
  .num_meta = 1,
  .inplace_algo = 0,
  .num_release_buf = 0,    //single output(native) buffer given to cpp
  .is_offline_proc = 1,
  .use_internal_bufs = 1,
  .internal_buf_cnt = 1,
  .before_cpp = true,
};

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_query_mod = module_mfproc_precpp_query_mod,
  .imgbase_client_created = module_mfproc_precpp_client_created,
  .streams_to_process = IMG_2_MASK(CAM_STREAM_TYPE_OFFLINE_PROC),
  .exec_mode = IMG_EXECUTION_SW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .force_cache_op = FALSE,
};

/**
 * Function: module_mfproc_precpp_client_created
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
int32_t module_mfproc_precpp_client_created(imgbase_client_t *p_client)
{
  /* process algo on all frames regardless of if HAL3 has a request on it */
  p_client->process_all_frames = TRUE;
  return IMG_SUCCESS;
}


/**
 * Function: module_mfproc_precpp_query_mod
 *
 * Description: This function is used to query MultiFrame Process Precpp module caps
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
boolean module_mfproc_precpp_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* p_mod, unsigned int sessionid)
{
  mct_pipeline_pp_cap_t *buf;
  IDBG_LOW("E ");

  IMG_UNUSED(p_mod);
  IMG_UNUSED(sessionid);
  if (!p_mct_cap) {
    IDBG_ERROR("Error");
    return FALSE;
  }

  buf = &(p_mct_cap->pp_cap);

  return TRUE;
}

/** module_mfproc_precpp_set_parent:
 *
 *  Arguments:
 *  @p_parent - parent module pointer
 *
 * Description: This function is used to set the parent pointer
 *              of the mfproc_precpp  module
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_mfproc_precpp_set_parent(mct_module_t *p_mct_mod, mct_module_t *p_parent)
{
    return module_imgbase_set_parent(p_mct_mod, p_parent);
}

/** module_mfproc_precpp_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: Function used to initialize the MultiFrame Process Precpp module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_mfproc_precpp_init(const char *name)
{
  IDBG_LOW("E ");
  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    NULL,
    &g_caps,
    "libmmcamera_mfproc_precpp.so",
    CAM_QTI_FEATURE_MFPROC_PRECPP,
    &g_params);
}


/**
 * Function: module_mfproc_precpp_deinit
 *
 * Description: This function is used to deinit MultiFrame Process Precpp module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_mfproc_precpp_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}
