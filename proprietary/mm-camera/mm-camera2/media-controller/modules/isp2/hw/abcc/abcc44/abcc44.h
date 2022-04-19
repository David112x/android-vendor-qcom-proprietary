/* abcc44.h
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ABCC44_H__
#define __ABCC44_H__

/* mctl headers */
#include "chromatix.h"

/* isp headers */
#include "abcc_reg.h"
#include "abcc_algo.h"
#include "isp_sub_module_common.h"

#define ABCC_FORCED_CORRECTION_ENABLE  0
#define ABCC_CGC_OVERRIDE TRUE

typedef struct {
  uint64_t            lut[ABCC_LUT_COUNT];
}__attribute__((packed, aligned(8)))abcc_packed_lut_t;

typedef struct {
  abcc_packed_lut_t              packed_lut;
  uint8_t                        abcc_enable;
  uint8_t                        one_time_config_done;
  abcc_algo_t                    abcc_info;
  uint32_t                       input_width;
  uint32_t                       input_height;
  sensor_dim_output_t            sensor_dim;
  uint8_t                        lut_bank_select;
} abcc_t;

typedef struct {
  abcc_t                   abcc_subblock;
} abcc44_t;

boolean abcc44_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void abcc44_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

boolean abcc44_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abcc44_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abcc44_set_defective_pixel_for_abcc(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean abcc44_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

#endif /* __ABCC44_H__ */
