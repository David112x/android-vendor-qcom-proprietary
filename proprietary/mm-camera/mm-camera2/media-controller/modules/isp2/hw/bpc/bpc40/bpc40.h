/* bpc40.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BPC40_H__
#define __BPC40_H__

/* mctl headers */
#include "chromatix.h"

/* isp headers */
#include "bpc_reg.h"
#include "isp_sub_module_common.h"

typedef boolean (*check_enable_index)(void *,
  void *, void *);
typedef boolean (*get_trigger_ratio)(void *,
  void *, void *);
typedef boolean (*get_init_min_max)(void *,
  void *, void *);
typedef boolean (*get_min_max)(void *,
  void *, void *);

typedef struct {
  check_enable_index check_enable_idx;
  get_trigger_ratio get_trigger_ratio;
  get_min_max get_min_max;
  get_init_min_max get_init_min_max;
} ext_override_func;

/** header_params_t:
 *
 *  @p_input_offset: input offset
 *  @p_Fmin: f min
 *  @p_Fmax: f max
 **/
typedef struct {
  bpc_4_offset_type *p_input_offset;
  unsigned char     *p_Fmin;
  unsigned char     *p_Fmax;
} header_params_t;

/** bpc40_t:
 *
 *  @RegCfgCmd: BPC cfg cmd
 *  @RegCmd: reg cmd
 *  @applied_RegCmd: applied RegCmd
 *  @aec_ratio: aec ratio
 *  @p_params: header params
 **/
typedef struct {
  ISP_BPCCfg_CmdType       RegCfgCmd;
  ISP_DemosaicDBPC_CmdType RegCmd;
  int                      start_index;
  int                      end_index;
  float                    aec_ratio;
  header_params_t          p_params;
  aec_update_t             aec_update;
  ext_override_func       *ext_func_table;
  uint8_t                  Fmin;
  uint8_t                  Fmin_lowlight;
  uint8_t                  Fmax;
  uint8_t                  Fmax_lowlight;
  bpc_4_offset_type        bpc_normal_input_offset;
  bpc_4_offset_type        bpc_lowlight_input_offset;
} bpc40_t;

boolean bpc40_init(isp_sub_module_t *isp_sub_module);

void bpc40_destroy(isp_sub_module_t *isp_sub_module);

boolean bpc40_trigger_update(isp_sub_module_t *isp_sub_module, void *data);

boolean bpc40_stats_aec_update(isp_sub_module_t *isp_sub_module, void *data);

boolean bpc40_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data);

boolean bpc40_streamoff(isp_sub_module_t *isp_sub_module, void *data);

#if OVERRIDE_FUNC
boolean bpc40_fill_func_table_ext(bpc40_t *bpc);
#define FILL_FUNC_TABLE(field) bpc40_fill_func_table_ext(field)
#else
boolean bpc40_fill_func_table(bpc40_t *bpc);
#define FILL_FUNC_TABLE(field) bpc40_fill_func_table(field)
#endif

#endif /* __BPC40_H__ */
