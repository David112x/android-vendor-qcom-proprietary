/* isp_sub_module_zoom.h
 *
 * Copyright (c) 2011-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ISP_SUB_MODULE_ZOOM_H__
#define __ISP_SUB_MODULE_ZOOM_H__

/* mctl headers */
#include "mct_pipeline.h"
#include "q3a_stats_hw.h"

#define MAX_ZOOM_STEPS      182
#define ZOOM_TABLE_MAX_DEF  182

/** isp_zoom_data_t:
 *
 *  @zoom_table: zoom table
 *  @zoom_table_bump: map zoom table
 *  @zoom_table_size: zoom table size
 *  @resize_factor: resize factor
 *  @zoom_step_size: zoom step size
 **/
typedef struct {
  uint32_t *zoom_table;
  uint32_t  zoom_table_bump[MAX_ZOOMS_CNT + MAX_ZOOM_STEPS];
  int       zoom_table_size;
  uint32_t  resize_factor;
  uint32_t  zoom_step_size;
} isp_zoom_data_t;

/** isp_zoom_session_t:
 *
 *  @num_fovs: num of fovs
 *  @session_id: session_id
 **/
typedef struct {
  int      num_fovs;
  uint32_t session_id;
} isp_zoom_session_t;

/** isp_zoom_t:
 *
 *  @zoom_data: handle to isp_zoom_data_t
 *  @sessions: handle to isp_zoom_session_t
 **/
typedef struct {
  isp_zoom_data_t    zoom_data;
  isp_zoom_session_t sessions;
} isp_zoom_t;

boolean isp_zoom_query_mod_func(mct_pipeline_cap_t *cap_buf);

boolean isp_zoom_init(isp_zoom_t *pzoom, uint32_t session_id,
  uint32_t isp_version);

boolean isp_zoom_get_crop_factor(isp_zoom_t *pzoom, int zoom_val,
  uint32_t *crop_factor);

#endif /* __ISP_SUB_MODULE_ZOOM_H__ */
