/* black_level48.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __BLACK_LEVEL48_H__
#define __BLACK_LEVEL48_H__

/* mctl headers */
#include "chromatix.h"

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "black_level48_reg.h"
#include "isp_sub_module_common.h"
#include "isp_pipeline_reg.h"

#define CLAMP(a,b,c) (((a) <= (b))? (b) : (((a) >= (c))? (c) : (a)))
#define CLAMP_BLK_LVL(x1,v1) ((x1 > v1) ? v1 : x1 )
#define SET_FIRST_N_BITS(N) ((1 << N) - 1)

typedef boolean (*hw_config)(void *, void *);
typedef void (*update_params)(void *, float,
 int, cam_sensor_hdr_type_t);

typedef struct {
  hw_config hw_config;
  update_params update_params;
} ext_override_func;

/** black_level48_t:
 *
 *  @ISP_black_level_reg_t: Black Level registers
 *  @trigger_index: trigger index
 *  @aec_ratio: aec ratio
 *  @chromatix_channel_balance_gains_type: demux green gain to
 *                                       program black level
 *                                       scale
 **/
typedef struct {
  ISP_black_level_reg_t                 Reg;
  aec_update_t                          aec_update;
  float                                 dig_gain;
  ext_override_func                    *ext_func_table;
  uint32_t                              pipe_cfg_blss_offset;
} black_level48_t;

boolean black_level48_init(mct_module_t *module,
                           isp_sub_module_t *isp_sub_module);

void black_level48_destroy(mct_module_t *module,
                           isp_sub_module_t *isp_sub_module);

boolean black_level48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean black_level48_stats_aec_update(isp_sub_module_t *isp_sub_module, void *data);

boolean black_level48_manual_aec_update(isp_sub_module_t *isp_sub_module, void *data);

boolean black_level48_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data);

boolean black_level48_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean black_level48_fetch_blklvl_offset(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean black_level48_set_digital_gain(isp_sub_module_t *isp_sub_module,
  float *new_dig_gain);

boolean black_level48_hw_config(void *data1,
  void *data2);

void black_level48_update_params(void *data1,
  float BLSS_offset, int BLSS_is_v2, cam_sensor_hdr_type_t hdr_mode);

#if OVERRIDE_FUNC
boolean black_level48_fill_func_table_ext(black_level48_t *black_level);
#define FILL_FUNC_TABLE(field) black_level48_fill_func_table_ext(field)
#else
boolean black_level48_fill_func_table(black_level48_t *black_level);
#define FILL_FUNC_TABLE(field) black_level48_fill_func_table(field)
#endif

#endif /* __BPC44_H__ */
