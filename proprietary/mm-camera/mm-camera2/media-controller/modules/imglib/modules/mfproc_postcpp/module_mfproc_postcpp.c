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
static boolean module_mfproc_postcpp_query_mod(mct_pipeline_cap_t* p_mct_cap,
  void* p_mod, unsigned int sessionid);
static int32_t module_mfproc_postcpp_client_created(imgbase_client_t *p_client);

/** g_caps:
 *
 *  Set the capabilities for MultiFrame Process Postcpp module
*/
static img_caps_t g_caps = {
  .num_input = MAX_MFPROC_FRAMECOUNT,
  .num_output = 1,
  .num_meta = 1,
  .inplace_algo = 0,
  .num_release_buf = 1,//single output buffer given to HAL
};

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_query_mod = module_mfproc_postcpp_query_mod,
  .imgbase_client_created = module_mfproc_postcpp_client_created,
  .streams_to_process = IMG_2_MASK(CAM_STREAM_TYPE_OFFLINE_PROC),
  .exec_mode = IMG_EXECUTION_SW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .force_cache_op = FALSE,
};

/**
 * Function: module_mfproc_postcpp_client_created
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
int32_t module_mfproc_postcpp_client_created(imgbase_client_t *p_client)
{
   /* process algo on all frames regardless of if HAL3 has a
    request on it */
  p_client->process_all_frames = TRUE;
  return IMG_SUCCESS;
}


/**
 * Function: module_mfproc_postcpp_query_mod
 *
 * Description: This function is used to query MultiFrame Process Postcpp module caps
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
boolean module_mfproc_postcpp_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* p_mod, unsigned int sessionid)
{
  mct_pipeline_pp_cap_t *buf;
  IDBG_ERROR("%s()  E ", __func__);

  IMG_UNUSED(p_mod);
  IMG_UNUSED(sessionid);
  if (!p_mct_cap) {
    IDBG_ERROR("Error");
    return FALSE;
  }

  buf = &(p_mct_cap->pp_cap);
  return TRUE;
}

/** module_mfproc_postcpp_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: Function used to initialize the MultiFrame Process Postcpp module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_mfproc_postcpp_init(const char *name)
{
  IDBG_ERROR(" %s(),  E ", __func__);
  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    NULL,
    &g_caps,
    "libmmcamera_mfproc_postcpp.so",
    CAM_QTI_FEATURE_MFPROC_POSTCPP,
    &g_params);
}


/**
 * Function: module_mfproc_postcpp_deinit
 *
 * Description: This function is used to deinit MultiFrame Process Postcpp module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_mfproc_postcpp_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}
