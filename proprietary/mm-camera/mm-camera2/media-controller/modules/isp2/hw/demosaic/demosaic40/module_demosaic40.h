/* module_demosaic40.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MODULE_DEMOSAIC40_H__
#define __MODULE_DEMOSAIC40_H__

/* std headers */
#include "pthread.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_common.h"
#include "module_demosaic40.h"
#include "demosaic40_reg.h"
#include "chromatix.h"

/** demosaic40_t:
 *
 *  @stream_on_count: stream on count
 *  @streaming_mode_mask: streaming mode mask
 *  @cur_streaming_mode: current streaming mode
 *  @old_streaming_mode: old streaming mode
 *  @mutex: mutex
 *  @chromatix_ptrs: chromatix ptrs
 *  @l_stream_info: stream info list
 *  @stats_update: stats update
 *  @reg_cmd: reg cmd
 *  @applied_RegCmd: applied reg cmd
 *  @mix_reg_cmd: mix reg cmd
 *  @ratio: trigger ratio
 *  @hw_udpate_pending: hw update pending flag
 *  @trigger_enable: trigger enable
 *  @skip_trigger: skip trigger
 *  @enable: enable
 *  @classifier_cfg_done: classifier cfg done
 **/
typedef struct {
  uint32_t                       streaming_mode_mask;
  cam_streaming_mode_t           cur_streaming_mode;
  cam_streaming_mode_t           old_streaming_mode;

  /* Module Params*/
  ISP_Demosaic40ConfigCmdType    reg_cmd;
  ISP_Demosaic40ConfigCmdType    applied_RegCmd;
  ISP_Demosaic40MixConfigCmdType mix_reg_cmd;

  trigger_ratio_t                ratio;

  /* Module Control */
  int                            classifier_cfg_done;
  awb_gain_t                     gain;
} demosaic40_t;

#endif
