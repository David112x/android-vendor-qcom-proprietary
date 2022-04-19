/* isp_trigger_thread.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ISP_TRIGGER_THREAD_H__
#define __ISP_TRIGGER_THREAD_H__

/* mctl headers */
#include "mct_queue.h"

/* isp headers */
#include "isp_module.h"

typedef enum {
  ISP_TRIGGER_THREAD_EVENT_PROCESS_QUEUE,
  ISP_TRIGGER_THREAD_EVENT_CLEAR_EVENTS,
  ISP_TRIGGER_THREAD_EVENT_ABORT_THREAD,
  ISP_TRIGGER_THREAD_EVENT_MAX,
} isp_thread_event_type_t;

/** isp_thread_event_t:
 *
 *  @type: event type
 *  @frame_id: frame id
 **/
typedef struct {
  isp_thread_event_type_t type;
  uint32_t frame_id;
} isp_trigger_thread_event_t;

/** isp_trigger_thread_priv_t:
 *
 *  @module: mct module handle
 *  @session_param: session param handle
 **/
typedef struct {
  mct_module_t        *module;
  isp_session_param_t *session_param;
  isp_resource_t      *isp_resource;
} isp_trigger_thread_priv_t;

boolean isp_trigger_thread_create(mct_module_t *module,
  isp_session_param_t *session_param, isp_resource_t *isp_resource);

void isp_trigger_thread_join(isp_session_param_t *session_param);

boolean isp_trigger_thread_enqueue_event(mct_event_t *event,
  isp_saved_events_t *saved_events, uint32_t session_based_ide);

boolean isp_trigger_thread_post_message(
  isp_trigger_update_params_t *trigger_update_params,
  isp_thread_event_type_t type, uint32_t frame_id,
  isp_session_param_t *session_param);

boolean isp_trigger_thread_handle_sof(
  isp_trigger_update_params_t *trigger_update_params, uint32_t frame_id,
  isp_session_param_t *session_param);

boolean isp_trigger_thread_store_new_params_for_offline(
  isp_session_param_t *session_param,  uint32_t frame_id);

boolean isp_trigger_thread_apply_shared_trigger_params(
  uint32_t frame_id, isp_session_param_t *session_param);

#endif
