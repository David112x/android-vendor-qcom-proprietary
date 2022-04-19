/**********************************************************************
*  Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
#include "module_imgbase.h"

/** g_caps:
 *
 *  Set the capabilities for Sample module
*/
static img_caps_t g_caps = {
  .num_input = 1,
  .num_output = 0,
  .inplace_algo = 1,
};

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .streams_to_process = IMG_2_MASK(CAM_STREAM_TYPE_PREVIEW) |
                        IMG_2_MASK(CAM_STREAM_TYPE_VIDEO),
  .exec_mode = IMG_EXECUTION_SW,
  .access_mode = IMG_ACCESS_READ_WRITE,
  .force_cache_op = FALSE,
};

/** module_sample_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: Function used to initialize the SeeMore module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_sample_init(const char *name)
{
  IDBG_ERROR("");
  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    NULL,
    &g_caps,
    "libmmcamera_dummyalgo.so",
    0, /* not feature specific */
    &g_params);
}
