/* bpc44.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BCC44_H__
#define __BCC44_H__

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

/** bcc44_t:
 *
 *  @RegCmd: reg cmd
 *  @aec_ratio: aec ratio
 *  @p_params: header params
 *  @params: DBCC params
 **/
typedef struct {
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
} bcc44_t;

boolean bcc44_init(isp_sub_module_t *isp_sub_module);

void bcc44_destroy(isp_sub_module_t *isp_sub_module);

boolean bcc44_trigger_update(isp_sub_module_t *isp_sub_module, void *data);

boolean bcc44_stats_aec_update(isp_sub_module_t *isp_sub_module, void *data);

boolean bcc44_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data);

boolean bcc44_streamoff(isp_sub_module_t *isp_sub_module, void *data);

#if OVERRIDE_FUNC
boolean bcc44_fill_func_table_ext(bcc44_t *bcc);
#define FILL_FUNC_TABLE(field) bcc44_fill_func_table_ext(field)
#else
boolean bcc44_fill_func_table(bcc44_t *bcc);
#define FILL_FUNC_TABLE(field) bcc44_fill_func_table(field)
#endif

#endif /* __BCC44_H__ */
