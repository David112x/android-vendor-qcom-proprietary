/* module_demux48.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __MODULE_DEMUX48_H__
#define __MODULE_DEMUX48_H__

/* std headers */
#include "pthread.h"

/* isp headers */
#include "isp_defs.h"
#include "isp_common.h"
#include "chromatix.h"
#include "demux_reg.h"

#define DEMUX_GAIN(x) FLOAT_TO_Q(10 ,(x))

/**demux48_t
 * @ISP_DemuxConfigCmd: Register to config demux module
 * @applied_cmd: save applied reg settings for eztune diagnostics
 * @ISP_RImageGainConfigCmd: for 3D, not used now
 * @gain: chanel balance gain
 * @r_gain:  chanel balance gain
 * @dig_gain: digital gain received frm sensor
 * @is_3d: if 3D
 * @trigger_enable: used to disable trigger updates such as bestshot or spl
  *    effects use cases.
 * @remaining_digital_gain: digital gain which overshoot capacity
 * @fmt: camera bayer format of type cam_format_t
 **/
typedef struct {
  ISP_DemuxConfigCmdType                ISP_DemuxConfigCmd;
  ISP_DemuxConfigCmdType                applied_cmd;
  ISP_DemuxGainCfgCmdType               ISP_RImageGainConfigCmd;
  chromatix_channel_balance_gains_type  gain;
  chromatix_channel_balance_gains_type  r_gain;
  float                                 dig_gain;
  uint8_t                               is_3d;
  uint32_t                              blk_lvl_offset;
  int                                   trigger_enable;
  float                                 remaining_digital_gain;
  cam_format_t                          fmt;
  enum ISP_START_PIXEL_PATTERN          pix_pattern;
  float                                 st_data_r_gain;
  float                                 st_data_godd_gain;
  float                                 st_data_geven_gain;
  float                                 st_data_b_gain;
  float                                 applied_dig_gain;
} demux48_t;

#endif
