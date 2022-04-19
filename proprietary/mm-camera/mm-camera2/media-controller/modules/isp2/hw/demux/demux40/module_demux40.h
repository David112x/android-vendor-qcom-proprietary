/* module_demux40.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MODULE_DEMUX40_H__
#define __MODULE_DEMUX40_H__

/* std headers */
#include "pthread.h"

/* isp headers */
#include "isp_defs.h"
#include "isp_common.h"
#include "chromatix.h"
#include "demux_reg.h"

#define DEMUX_GAIN(x) FLOAT_TO_Q(7 ,(x))

typedef float (*get_global_gain)(void *,
  void *, void *);

typedef struct {
  get_global_gain get_global_gain;
} ext_override_func;

/**demux40_t
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
  int                                   trigger_enable;
  float                                 remaining_digital_gain;
  cam_format_t                          fmt;
  ext_override_func                    *ext_func_table;
  float                                 applied_dig_gain;
} demux40_t;

#if OVERRIDE_FUNC
boolean demux40_fill_func_table_ext(demux40_t *);
#define FILL_FUNC_TABLE(field) demux40_fill_func_table_ext(field)
#else
boolean demux40_fill_func_table(demux40_t *);
#define FILL_FUNC_TABLE(field) demux40_fill_func_table(field)
#endif

#endif
