/* isp_algo.h
 *
 * Copyright (c) 2014, 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ISP_ALGO_H__
#define __ISP_ALGO_H__

/* mctl headers */
#include "mtype.h"

/* isp headers */
#include "isp_module.h"
#include "isp_defs.h"

typedef boolean (*isp_algo_func_t)(
  mct_module_t             *module,
  mct_event_stats_isp_t    *stats_data,
  isp_algo_params_t        *algo_parm,
  isp_saved_algo_params_t  *saved_algo_parm,
  void                    **output,
  uint32_t                  curr_frame_id,
  boolean                   svhdr_en);

typedef boolean (*isp_stop_session_func_t)(
  mct_module_t             *module,
  isp_saved_algo_params_t  *algo_session_params);

typedef struct {
  const char              *name;
  isp_algo_func_t          algo_func;
  isp_stop_session_func_t  stop_session_func;
  mct_event_module_type_t  event_type;
} isp_algo_t;

boolean isp_algo_execute_internal_algo(mct_module_t *module,
  isp_session_param_t *session_param, mct_event_stats_isp_t *stats_data,
  isp_algo_params_t *algo_parm);

boolean isp_algo_stop_session(mct_module_t *module,
  isp_session_param_t *session_param);

#endif
