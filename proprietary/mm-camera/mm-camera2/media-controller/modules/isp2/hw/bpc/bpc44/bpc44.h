/* bpc44.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __BPC44_H__
#define __BPC44_H__

/* mctl headers */
#include "chromatix.h"

/* isp headers */
#include "bpc_reg.h"
#include "abcc_algo.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module_log.h"

#define ABCC_ENABLE                    0
#define ABCC_FORCED_CORRECTION_ENABLE  0

typedef struct {
  uint64_t            lut[ABCC_LUT_COUNT];
}__attribute__((packed, aligned(8)))abcc_packed_lut_t;

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

typedef struct {
  abcc_packed_lut_t              packed_lut;
  uint8_t                        abcc_enable;
  uint8_t                        one_time_config_done;
  abcc_algo_t                    abcc_info;
  uint32_t                       input_width;
  uint32_t                       input_height;
  uint8_t                        hw_update_pending;
  sensor_dim_output_t            sensor_dim;
  uint8_t                        lut_bank_select;
} abcc_t;

/** bpc44_t:
 *
 *  @RegCfgCmd: BPC cfg cmd
 *  @RegCmd: reg cmd
 *  @aec_ratio: aec ratio
 *  @p_params: header params
 **/
typedef struct {
  ISP_DemosaicDBPC_CmdType RegCmd;
  float                    aec_ratio;
  int                       start_index;
  int                       end_index;
  header_params_t          p_params;
  aec_update_t             aec_update;
  ext_override_func       *ext_func_table;
  uint8_t                  Fmin;
  uint8_t                  Fmin_lowlight;
  uint8_t                  Fmax;
  uint8_t                  Fmax_lowlight;
  bpc_4_offset_type        bpc_normal_input_offset;
  bpc_4_offset_type        bpc_lowlight_input_offset;
  abcc_t                   abcc_subblock;
} bpc44_t;

boolean bpc44_init(isp_sub_module_t *isp_sub_module);

void bpc44_destroy(isp_sub_module_t *isp_sub_module);

boolean bpc44_trigger_update(isp_sub_module_t *isp_sub_module, void *data);

boolean bpc44_stats_aec_update(isp_sub_module_t *isp_sub_module, void *data);

boolean bpc44_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data);
boolean bpc44_set_defective_pixel_for_abcc(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
boolean bpc44_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
boolean bpc44_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
boolean bpc44_streamoff(isp_sub_module_t *isp_sub_module, void *data);

#if OVERRIDE_FUNC
boolean bpc44_fill_func_table_ext(bpc44_t *bpc);
#define FILL_FUNC_TABLE(field) bpc44_fill_func_table_ext(field)
#else
boolean bpc44_fill_func_table(bpc44_t *bpc);
#define FILL_FUNC_TABLE(field) bpc44_fill_func_table(field)
#endif

#endif /* __BPC44_H__ */
