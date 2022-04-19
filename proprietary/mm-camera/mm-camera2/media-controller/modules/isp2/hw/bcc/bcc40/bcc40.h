/* bcc40.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BCC40_H__
#define __BCC40_H__

/* mctl headers */
#include "chromatix.h"

/* isp headers */
#include "bcc_reg.h"
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
 *  @Fmin: F min
 *  @Fmax: F max
 **/
typedef struct {
  bpc_4_offset_type *p_input_offset;
  uint8_t Fmin;
  uint8_t Fmax;
} header_params_t;

/** bcc40_t:
 *
 *  @RegCfgCmd: reg cfg cmd
 *  @RegCmd: reg cmd
 *  @aec_ratio: aec ratio
 *  @p_params: header params
 *  @params: DBCC params
 **/
typedef struct {
  ISP_BPCCfg_CmdType       RegCfgCmd;
  ISP_DemosaicDBCC_CmdType RegCmd;
  float                    aec_ratio;
  header_params_t          p_params;
  ISP_DemosaicDBCC_CmdType params;
  int start_index;
  int end_index;
  aec_update_t             aec_update;
  ext_override_func       *ext_func_table;
  uint8_t                  Fmin;
  uint8_t                  Fmin_lowlight;
  uint8_t                  Fmax;
  uint8_t                  Fmax_lowlight;
  bpc_4_offset_type        bcc_normal_input_offset;
  bpc_4_offset_type        bcc_lowlight_input_offset;
} bcc40_t;

boolean bcc40_init(isp_sub_module_t *isp_sub_module);

void bcc40_destroy(isp_sub_module_t *isp_sub_module);

boolean bcc40_trigger_update(isp_sub_module_t *isp_sub_module, void *data);

boolean bcc40_stats_aec_update(isp_sub_module_t *isp_sub_module, void *data);

boolean bcc40_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data);

boolean bcc40_streamoff(isp_sub_module_t *isp_sub_module, void *data);

#if OVERRIDE_FUNC
boolean bcc40_fill_func_table_ext(bcc40_t *bcc);
#define FILL_FUNC_TABLE(field) bcc40_fill_func_table_ext(field)
#else
boolean bcc40_fill_func_table(bcc40_t *bcc);
#define FILL_FUNC_TABLE(field) bcc40_fill_func_table(field)
#endif
#endif /* __BCC40_H__ */
