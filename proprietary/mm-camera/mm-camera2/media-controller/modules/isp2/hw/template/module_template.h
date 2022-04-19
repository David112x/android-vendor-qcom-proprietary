/* module_template40.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MODULE_DEMOSAIC40_H__
#define __MODULE_DEMOSAIC40_H__

/* std headers */
#include "pthread.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_common.h"
#include "module_template.h"
#include "chromatix.h"

/** template_session_param_t:
 *
 *  @state: ISP state
 *  @session_id: session id
 *  @stream_on_count: stream on count
 *  @streaming_mode_mask: streaming mode mask
 *  @mutex: mutex
 *  @l_stream_info: stream info list
 *  @hw_udpate_pending: hw update pending flag
 **/
typedef struct {
  isp_state_t                    state;
  uint32_t                       session_id;
  uint32_t                       stream_on_count;
  uint32_t                       streaming_mode_mask;
  pthread_mutex_t                mutex;
  mct_list_t                    *l_stream_info; /* mct_stream_info_t */
  uint8_t                        hw_update_pending;
} template_t;

/** template_priv_t:
 *
 *  @l_session_params: list of session params
 **/
typedef struct {
  mct_list_t *l_session_params; /* template_t */
} template_priv_t;

#endif
