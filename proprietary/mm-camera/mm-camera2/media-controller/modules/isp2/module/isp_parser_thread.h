/* isp_parser_thread.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ISP_PARSER_THREAD_H__
#define __ISP_PARSER_THREAD_H__

typedef enum {
  ISP_PARSER_EVENT_PROCESS,
  ISP_PARSER_THREAD_EVENT_FREE_QUEUE,
  ISP_PARSER_EVENT_ABORT_THREAD,
  ISP_PARSER_EVENT_MAX,
} isp_parser_event_type_t;

/** isp_parser_thread_event_t:
 *
 *  @type: hw update thread event type
 **/
typedef struct {
  isp_parser_event_type_t type;
} isp_parser_thread_event_t;

/** isp_parser_thread_priv_t:
 *
 *  @module: mct module handle
 *  @isp_resource: isp resource handle
 *  @session_param: session param handle
 **/
typedef struct {
  mct_module_t        *module;
  isp_resource_t      *isp_resource;
  isp_session_param_t *session_param;
} isp_parser_thread_priv_t;

boolean isp_parser_thread_create(mct_module_t *module,
  isp_resource_t *isp_resource, isp_session_param_t *session_param);

void isp_parser_thread_join(isp_session_param_t *session_param);

boolean isp_parser_thread_save_stats_nofity_event(mct_module_t *module,
  isp_session_param_t *session_param, mct_event_t *event);

boolean isp_parser_thread_post_message(
  isp_parser_params_t *parser_params, isp_parser_event_type_t type);

#endif /* __ISP_PARSER_THREAD_H__ */
