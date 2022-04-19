/* abf47.h
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __DEMOSAIC_ABF47_H__
#define __DEMOSAIC_ABF47_H__

#include "chromatix.h"

#include "abf47_reg.h"
#include "isp_sub_module_common.h"
#include "../common/abf_common.h"

#define NORMAL_LIGHT_INDEX 4
#define ABF3_MESH_WIDTH    13
#define ABF3_MESH_HEIGHT   10
#define ABF3_MESH_LENGTH   ABF3_MESH_WIDTH * ABF3_MESH_HEIGHT
#define ABF_MESH_TABLE_MAX (1 << 12) - 1

typedef uint32_t signal2_lut_t[32];
typedef uint32_t noise_std2_lut_t[128];
typedef uint32_t abf_mesh_lut_t[130];

typedef struct {
  signal2_lut_t     signal2_lut_lv0;
  signal2_lut_t     signal2_lut_lv1;
  noise_std2_lut_t  noise_std2_lut_lv0;
  noise_std2_lut_t  noise_std2_lut_lv1;
  abf_mesh_lut_t    abf_mesh_lut;
} abf3_parms_t;

typedef boolean (*get_trigger_index)(void *, void *);

typedef struct {
  get_trigger_index  get_trigger_index;
} ext_override_func;

typedef struct {
  float edge_softness_adj_0;
  float edge_softness_adj_1;
  float filter_strength_0;
  float filter_strength_1;
  float noise_profile_adj_0;
  float noise_profile_adj_1;
} abf_adjustment_t;

typedef struct {
  /* Module Params*/
  ISP_ABF3_CmdType         v3RegCmd;
  trigger_ratio_t          aec_ratio;
  uint8_t                  trigger_index;
  uint8_t                  trigger_index_adj;
  float                    mesh_table[ABF3_MESH_LENGTH];
  abf3_parms_t             abf3_parms;
  sensor_out_info_t        sensor_out_info;
  float                    fetched_mesh_table[MESH_ROLLOFF_SIZE];
  ispif_out_info_t         ispif_out_info;
  isp_out_info_t           isp_out_info;
  float                    aec_reference;
  float                    aec_reference_adj;
  aec_update_t             aec_update;
  aec_manual_update_t      aec_manual_update;
  ext_override_func       *ext_func_table;
} abf47_t;

boolean abf47_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void abf47_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean abf47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abf47_decide_hysterisis(
  isp_sub_module_t *isp_sub_module, float aec_ref);

boolean abf47_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abf47_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abf47_set_split_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abf47_set_stripe_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abf47_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abf47_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abf47_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abf47_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data);

boolean abf47_decide_hyster_point_enable(void* data1, void* data2);

#if OVERRIDE_FUNC
boolean abf47_fill_func_table_ext(abf47_t *abf);
#define FILL_FUNC_TABLE(field) abf47_fill_func_table_ext(field)
#else
boolean abf47_fill_func_table(abf47_t *abf);
#define FILL_FUNC_TABLE(field) abf47_fill_func_table(field)
#endif

#endif //__DEMOSAIC_ABF47_H__
