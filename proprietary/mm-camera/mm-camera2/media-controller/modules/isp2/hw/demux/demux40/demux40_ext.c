/* demux40_ext.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


/* isp headers */
#include "isp_log.h"
#include "demux40.h"

/** demux40_ext_get_global_gain:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
float demux40_ext_get_global_gain(void *data1,
  void *data2, void *data3)
{
  demux40_t               *demux = NULL;
  isp_sub_module_t        *isp_sub_module = NULL;
  float                    clamping = 0.0f;
  float                    global_gain = 0.0f;
  float                    stretching_gain = 1.0f;
  chromatix_parms_type    *chromatix_ptr = NULL;
  exposure_struct_type    *exposure_data = NULL;
  boolean                  ret = TRUE;
  isp_sub_module_output_t *output = NULL;
  float                    max_offset = 0.0f;

  if (!data1|| !data2 || !data3) {
    ISP_ERR("failed: %p %p %p", data1, data2, data3);
    return FALSE;
  }
  demux = (demux40_t *)data1;
  isp_sub_module = (isp_sub_module_t *)data2;
  output = (isp_sub_module_output_t *)data3;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }
  exposure_data = &chromatix_ptr->exposure_data;
  if (output->algo_params) {
    clamping = output->algo_params->clamping;
    max_offset = output->algo_params->linearization_max_val;
  }

  if(clamping != 0.0f)
    stretching_gain = (max_offset/(max_offset - clamping))+0.0005f;
  else
    stretching_gain = 1.0f;

  ISP_DBG("demux_dbg clamping: %f stretching_gain: %f", clamping, stretching_gain);

  global_gain = exposure_data->color_correction_global_gain * stretching_gain;
  return global_gain;
}

static ext_override_func demux_override_func_ext = {
  .get_global_gain         = demux40_ext_get_global_gain,
};

boolean demux40_fill_func_table_ext(demux40_t *demux)
{
  demux->ext_func_table = &demux_override_func_ext;
  return TRUE;
} /* demux40_fill_func_table_ext */
