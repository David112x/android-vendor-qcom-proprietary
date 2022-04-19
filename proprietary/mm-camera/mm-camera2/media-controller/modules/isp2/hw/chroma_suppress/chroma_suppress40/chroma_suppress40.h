/* chroma_suppress40.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __CHROMA_SUPPRESS40_H__
#define __CHROMA_SUPPRESS40_H__

/* mctl headers */
#include "chromatix.h"

/* isp headers */
#include "chroma_suppress_reg.h"
#include "isp_sub_module_common.h"

#define Clamp(x, t1, t2) (((x) < (t1))? (t1): ((x) > (t2))? (t2): (x))

/** isp_chroma_suppress_mix_1_cmd_t:
 *
 *  @hw_mask: hw mask
 *  @reg_cmd: mix1 reg cmd
 **/
typedef struct {
  uint32_t hw_mask;
  ISP_ChromaSuppress_Mix1_ConfigCmdType reg_cmd;
}isp_chroma_suppress_mix_1_cmd_t;

/** isp_chroma_suppress_mix_2_cmd_t:
 *
 *  @hw_mask: hw mask
 *  @reg_cmd: mix2 reg cmd
 **/
typedef struct {
  uint32_t hw_mask;
  ISP_ChromaSuppress_Mix2_ConfigCmdType reg_cmd;
}isp_chroma_suppress_mix_2_cmd_t;

typedef struct
{
    float   cs_luma_threshold1;
    float   cs_luma_threshold2;
    float   cs_luma_threshold3;
    float   cs_luma_threshold4;
    float   cs_chroma_threshold1;
    float   cs_chroma_threshold2;
} cs_luma_threshold_out_type;

/** chroma_suppress40_t:
 *
 *  @reg_cmd: chroma suppress reg cmd
 *  @reg_mix_cmd_1: mix1 reg cmd
 *  @reg_mix_cmd2: mix2 reg cmd
 *  @aec_ratio: aec ratio
 *  @thresholds: threadhold
 **/
typedef struct {
  ISP_ChromaSuppress_ConfigCmdType reg_cmd;
  isp_chroma_suppress_mix_1_cmd_t  reg_mix_cmd_1;
  isp_chroma_suppress_mix_2_cmd_t  reg_mix_cmd_2;
  trigger_ratio_t                  aec_ratio;
  cs_luma_threshold_out_type       thresholds;
} chroma_suppress40_t;

boolean chroma_suppress40_init(isp_sub_module_t *isp_sub_module);

void chroma_suppress40_destroy(isp_sub_module_t *isp_sub_module);

boolean chroma_suppress40_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean chroma_suppress40_stats_aec_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean chroma_suppress40_streamoff(isp_sub_module_t *isp_sub_module,
  void *data);

boolean chroma_suppress40_streamon(isp_sub_module_t *isp_sub_module,
  void *data);

#endif /* __CHROMA_SUPPRESS40_H__ */
