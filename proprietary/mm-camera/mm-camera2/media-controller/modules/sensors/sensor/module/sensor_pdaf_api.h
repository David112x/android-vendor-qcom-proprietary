/* sensor_pdaf_api.h
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef __SENSOR_PDAF_API_H__
#define __SENSOR_PDAF_API_H__

#include "sensor_lib.h"
#include "sensor_common.h"


enum scale_mode {
  NO_SCALING = 0,
  H_SCALING = 1,
  H_AND_V_SCALING = 2,
};

boolean pdaf_deinit(void *sctrl);
boolean pdaf_init(void *sctrl, void *data);
boolean pdaf_calc_defocus(void *sctrl, void *data);
boolean pdaf_get_type(void *sctrl, void *data);
boolean pdaf_get_native_pattern(void *sctrl, void *data);
boolean pdaf_set_window_update(void *sctrl, void *data);
int32_t pdaf_parse_pd(sensor_stats_format_t format,
 pdaf_params_t *s_pdaf);

signed long pdaf_get_defocus_with_pd_data(void *arg1,
  void *arg2);

boolean pdaf_set_buf_data_type(void *sctrl, void *data);

#endif /* __SENSOR_PDAF_API_H__ */
