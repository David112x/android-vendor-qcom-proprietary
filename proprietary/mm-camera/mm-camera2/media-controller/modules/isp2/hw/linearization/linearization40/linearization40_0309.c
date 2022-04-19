/* linearization40_0309.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


/* isp headers */
#include "isp_log.h"
#include "linearization40.h"


/** lineariuzation40_enab_sub_mod:
 *
 *  @isp_sub_module: isp sub module ptr
 *
 *  Decide to enable or disable sub module
 **/
void linearization40_enab_sub_mod(isp_sub_module_t *isp_sub_module)
{
  chromatix_parms_type      *chromatix_ptr = NULL;
  chromatix_VFE_common_type *chromatix_common = NULL;

  chromatix_common =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;
  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;

  if (chromatix_common->chromatix_L.linearization_enable > 0) {
    isp_sub_module->submod_enable = TRUE;
    isp_sub_module->chromatix_module_enable = TRUE;
  }
}

/** linearization40_calc_clamp:
 *
 *  @af_ratio: output of this method
 *  @linearization: linearization handle
 *
 * Calculate clamping value
 **/
boolean linearization40_calc_clamp(
  linearization40_t *linearization,
  Linearization_high_res_Lut_t *pTbl,
  isp_sub_module_output_t *sub_module_output)
{
  /* Do not change return value */
  if(!linearization || !pTbl || !sub_module_output)
    ISP_DBG("%p %p %p",linearization, pTbl, sub_module_output);

  return TRUE;
}

/** linearization40_get_trigger_idx:
 *
 *  @pchromatix_common: common chromatix pointer
 *  @linearization: linearization handle
 *
 *  Gets the trigger index
 **/
boolean linearization40_get_trigger_idx (
  isp_sub_module_t          *isp_sub_module,
  void *linearization,
  void *pchromatix_common)
{
  /* Do not change return value */
  if(!isp_sub_module || !linearization || !pchromatix_common)
    ISP_DBG("%p %p",linearization, pchromatix_common);

  return FALSE;
}

/** linearization40_get_right_tables
 *
 *  @af_ratio: output of this method
 *  @linearization: linearization handle
 *
 * Calculate clamping value
 **/
boolean linearization40_get_right_tables (
  linearization40_t         *linearization,
  chromatix_VFE_common_type *pchromatix_common,
  Linearization_high_res_Lut_t *output1,
  Linearization_high_res_Lut_t *output2,
  cam_sensor_hdr_type_t        hdr_mode
)
{
  /* Do not change return value */
  if(!output1 || !output2 || !linearization || !pchromatix_common)
    ISP_DBG("%p %p",linearization, pchromatix_common);

  if (hdr_mode < CAM_SENSOR_HDR_OFF || hdr_mode >= CAM_SENSOR_HDR_MAX ) {
    ISP_DBG("Invalid HDR mode, mode:%d",hdr_mode);
  }

  return FALSE;
}

/** linearization40_update_right_base_tables
 *
 *  @af_ratio: output of this method
 *  @linearization: linearization handle
 *
 * Calculate clamping value
 **/
boolean linearization40_update_right_base_tables (
  void *linearization,
  void *pchromatix_common
)
{
  /* Do not change return value */
  if(!linearization || !pchromatix_common)
    ISP_DBG("%p %p",linearization, pchromatix_common);

  return FALSE;
}
