/* demux40.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __DEMUX40_H__
#define __DEMUX40_H__

/* isp headers */
#include "module_demux40.h"
#include "isp_sub_module_common.h"

boolean demux40_init(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void demux40_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module);

void demux40_update_streaming_mode_mask(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, uint32_t streaming_mode_mask);

boolean demux40_set_digital_gain(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux40_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux40_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux40_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux40_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux40_trigger_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux40_reset(mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);

boolean demux40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux40_fetch_demux_gain(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean demux40_query_cap(mct_module_t *module, void *query_buf);
#endif //__DEMUX40_H__
