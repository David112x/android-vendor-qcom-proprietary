/* module_demosaic48.h
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __MODULE_DEMOSAIC48_H__
#define __MODULE_DEMOSAIC48_H__

/* std headers */
#include "pthread.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_common.h"
#include "module_demosaic48.h"
#include "demosaic_reg.h"
#include "chromatix.h"

typedef boolean (*get_2d_interpolation)(void *, void *);
typedef void (*apply_2d_interpolation)(void *);

typedef struct {
  get_2d_interpolation get_2d_interpolation;
  apply_2d_interpolation apply_2d_interpolation;
} ext_override_func;

typedef struct {
  /* Module Params*/
  float      demosaic_ak;
} demosaic48_ext_t;

/** demosaic48_t:
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
  float                          aec_gain_reference;
  float                          aec_lux_reference;
  float                          hdr_sen_trigger_ref;
  float                          hdr_time_trigger_ref;
  ext_override_func             *ext_func_table;
  unsigned int                   demosaic_ak;
  demosaic48_ext_t              demosaic48_ext_params;
  uint32_t                      multi_factor;
  uint8_t                       apply_hdr_effects;  
} demosaic48_t;

#if OVERRIDE_FUNC
boolean demosaic48_fill_func_table_ext(demosaic48_t *gic);
#define FILL_FUNC_TABLE(field) demosaic48_fill_func_table_ext(field)
#else
boolean demosaic48_fill_func_table(demosaic48_t *gic);
#define FILL_FUNC_TABLE(field) demosaic48_fill_func_table(field)
#endif

#endif
