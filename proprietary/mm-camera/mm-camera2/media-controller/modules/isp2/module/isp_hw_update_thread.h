/* isp_hw_update_thread.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ISP_HW_UPDATE_THREAD_H__
#define __ISP_HW_UPDATE_THREAD_H__

typedef enum {
  ISP_HW_UPDATE_EVENT_PROCESS,
  ISP_HW_UPDATE_EVENT_FREE_HW_UPDATE_LIST,
  ISP_HW_UPDATE_EVENT_ABORT_THREAD,
  ISP_HW_UPDATE_EVENT_MAX,
} isp_hw_update_event_type_t;

/** isp_hw_update_thread_event_t:
 *
 *  @type: hw update thread event type
 **/
typedef struct {
  isp_hw_update_event_type_t type;
} isp_hw_update_thread_event_t;

/** isp_hw_update_thread_data_t:
 *
 *  @isp_resource: ISP resource handle
 *  @session_param: session param handle
 **/
typedef struct {
  isp_resource_t      *isp_resource;
  isp_session_param_t *session_param;
} isp_hw_update_thread_data_t;

boolean isp_hw_update_thread_create(isp_resource_t *isp_resource,
  isp_session_param_t *session_param);

void isp_hw_update_thread_join(isp_session_param_t *session_param);

boolean isp_hw_update_thread_post_message(
  isp_hw_update_params_t *hw_update_params, isp_hw_update_event_type_t type,
  isp_session_param_t *session_param);

boolean isp_hw_update_thread_handle_sof(isp_session_param_t *session_param);

#endif /* __ISP_HW_UPDATE_THREAD_H__ */
