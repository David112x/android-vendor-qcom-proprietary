/***************************************************************************
* Copyright (c) 2013-2016 Qualcomm Technologies, Inc.                      *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/
#include "module_imgbase.h"

/** REFOCUS_BURST_CNT:
 *
 *  Burst count
 **/
#define REFOCUS_BURST_CNT 5

/** REFOCUS_HEADER_SIZE:
 *
 *  Refocus max header size
 **/
#define REFOCUS_HEADER_SIZE (25)

/** REFOCUS_MAX_WIDTH:
 *
 *  Refocus max width size
 **/
#define REFOCUS_MAX_WIDTH (640)

/** REFOCUS_MAX_HEIGHT:
 *
 *  Refocus max height size
 **/
#define REFOCUS_MAX_HEIGHT (640)

/** REFOCUS_FULL_META_SIZE:
 *
 *  Refocus max meta/body mask mask size
 **/
#define REFOCUS_FULL_META_SIZE (sizeof(cam_misc_buf_t) +\
  REFOCUS_HEADER_SIZE + REFOCUS_MAX_WIDTH * REFOCUS_MAX_HEIGHT)

/**
 *  Static functions
 **/
static boolean module_refocus_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* /*p_mod*/,unsigned int sessionid);
static boolean module_refocus_init_params(imgbase_client_t *p_client,
  img_init_params_t *p_params);

/** g_focus_steps:
 *
 *  Focus steps
 **/
uint8_t g_focus_steps[MAX_AF_BRACKETING_VALUES] =
  {1, 1, 1, 1, 1};

/** g_caps:
 *
 *  Set the capabilities for refocus module
 **/
static img_caps_t g_caps = {
  .num_input = REFOCUS_BURST_CNT,
  .num_output = 1,
  .num_meta = 1,
  .inplace_algo = 0,
  .num_release_buf = 6,
};

/** g_params: asd
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_query_mod = module_refocus_query_mod,
  .imgbase_client_init_params = module_refocus_init_params,
  .exec_mode = IMG_EXECUTION_SW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .force_cache_op = FALSE,
};

/**
 * Function: module_refocus_init_params
 *
 * Description: This function is used to init parameters
 *
 * Arguments:
 *   p_client - Imgbase client
 *   p_params - refocus init params
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_refocus_init_params(imgbase_client_t *p_client,
  img_init_params_t *p_params)
{
  boolean ret = FALSE;
  if (p_params) {
    p_params->refocus_encode = 1;
    ret = TRUE;
  }
  return ret;
}

/**
 * Function: module_refocus_deinit
 *
 * Description: This function is used to deinit refocus
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
void module_refocus_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}

/**
 * Function: module_refocus_query_mod
 *
 * Description: This function is used to query refocus
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
boolean module_refocus_query_mod(mct_pipeline_cap_t *p_mct_cap,
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
  buf->refocus_af_bracketing_need.burst_count = REFOCUS_BURST_CNT;
  buf->refocus_af_bracketing_need.enable = TRUE;
  buf->refocus_af_bracketing_need.output_count = REFOCUS_BURST_CNT + 1;
  buf->refocus_af_bracketing_need.meta_max_size = REFOCUS_FULL_META_SIZE;
  memcpy(&buf->refocus_af_bracketing_need.focus_steps, &g_focus_steps,
    sizeof(g_focus_steps));
  return TRUE;
}

/** module_refocus_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the refocus
 * module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_refocus_init(const char *name)
{
  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    NULL,
    &g_caps,
    "libmmcamera_ubifocus_lib.so",
    CAM_QCOM_FEATURE_REFOCUS,
    &g_params);
}

