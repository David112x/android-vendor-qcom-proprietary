/* abf40.h
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __DEMOSAIC_ABF40_H__
#define __DEMOSAIC_ABF40_H__

#include "chromatix.h"

#include "abf_reg.h"
#include "isp_sub_module_common.h"
#include "../common/abf_common.h"

typedef struct {
  float table_pos[16];
  float table_neg[8];
} abf2_table_t;

typedef struct {
  abf2_table_t r_table;
  abf2_table_t g_table;
  abf2_table_t b_table;
  chromatix_adaptive_bayer_filter_data_type2 data;
  int8_t table_updated;
} abf2_parms_t;

typedef boolean (*get_trigger_index)(void *, void *);

typedef struct {
  get_trigger_index  get_trigger_index;
} ext_override_func;

typedef struct {
  /* Module Params*/
  ISP_DemosaicABF2_CmdType v2RegCmd;
  trigger_ratio_t aec_ratio;
  uint8_t trigger_index; /*AEC trigger index*/
  abf2_parms_t abf2_parms;
  aec_update_t aec_update;
  ext_override_func *ext_func_table;
} abf40_t;

boolean abf40_init(isp_sub_module_t *isp_sub_module);

void abf40_destroy(isp_sub_module_t *isp_sub_module);

boolean abf40_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean abf40_aec_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean abf40_aec_update_v2(isp_sub_module_t *isp_sub_module,
  void *data);

boolean abf40_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  void *data);

boolean abf40_streamoff(isp_sub_module_t *isp_sub_module,
  void *data);
#endif //__DEMOSAIC_ABF40_H__
