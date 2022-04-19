/* gic46.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __GIC46_H__
#define __GIC46_H__

/* mctl headers */
#include "chromatix.h"

/* isp headers */
#include "isp_sub_module_common.h"
#include "isp_defs.h"
#include "gic_reg.h"

#define GIC_SIGNAL_SQR_LUT_SIZE     32
#define GIC_NOISE_STD2_LUT_SIZE     128

#define GIC_SIGNAL_SQR_LUT_SIZE_CHROMATIX   (GIC_SIGNAL_SQR_LUT_SIZE+1)
#define GIC_NOISE_STD2_LUT_SIZE_CHROMATIX   (GIC_NOISE_STD2_LUT_SIZE+1)

/** gic46_t:
 *
 *  @ISP_GICCfgCmd: GIC register setting
 *  @old_streaming_mode: old streaming mode
 **/
typedef struct  {
  uint32_t noiseStd2Table[GIC_NOISE_STD2_LUT_SIZE_CHROMATIX];
  uint32_t signalSqrTable[GIC_SIGNAL_SQR_LUT_SIZE_CHROMATIX];
  uint32_t packed_noiseStd2Table[GIC_NOISE_STD2_LUT_SIZE];
  uint32_t packed_signalSqrTable[GIC_SIGNAL_SQR_LUT_SIZE];
  ISP_GIC_CfgCmdType      ISP_GICCfgCmd;
  float                   aec_ratio;
  uint8_t                 trigger_index;
} gic46_t;

boolean gic46_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gic46_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gic46_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gic46_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean gic46_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void gic46_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

#endif //__GIC46_H__
