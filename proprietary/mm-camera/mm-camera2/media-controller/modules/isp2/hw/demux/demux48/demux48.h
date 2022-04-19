/* demux48.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __DEMUX48_H__
#define __DEMUX48_H__

/* isp headers */
#include "module_demux48.h"
#include "isp_sub_module_common.h"

boolean demux48_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void demux48_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void demux48_update_streaming_mode_mask(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, uint32_t streaming_mode_mask);

boolean demux48_set_digital_gain(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux48_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux48_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux48_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux48_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux48_trigger_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux48_reset(mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);

boolean demux48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux48_fetch_demux_gain(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux48_query_cap(mct_module_t *module, void *query_buf);
#endif //__DEMUX48_H__
