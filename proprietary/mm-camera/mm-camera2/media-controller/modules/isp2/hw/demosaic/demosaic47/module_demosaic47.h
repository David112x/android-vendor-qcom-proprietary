/* module_demosaic47.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MODULE_DEMOSAIC47_H__
#define __MODULE_DEMOSAIC47_H__

/* std headers */
#include "pthread.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_common.h"
#include "module_demosaic47.h"
#include "demosaic_reg.h"
#include "chromatix.h"

/** demosaic47_t:
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
  ISP_DemosaicConfigCmdType    reg_cmd;
  ISP_DemosaicConfigCmdType    applied_RegCmd;

  trigger_ratio_t                ratio;

  /* Module Control */
  int                            classifier_cfg_done;
  awb_gain_t                     gain;
  awb_update_t                   awb_update;
} demosaic47_t;

#endif
