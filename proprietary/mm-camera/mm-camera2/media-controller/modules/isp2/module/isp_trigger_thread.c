/* isp_trigger_thread.c
 *
 * Copyright (c) 2012-2014, 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>

/* kernel headers*/
#include <media/msmb_isp.h>

/* mctl headers */
#include "chromatix_common.h"
#include "media_controller.h"
#include "mct_list.h"
#include "mtype.h"
#include "mct_module.h"
#include "mct_profiler.h"

/* isp headers */
#include "isp_trigger_thread.h"
#include "isp_log.h"
#include "isp_util.h"
#include "isp_hw_update_util.h"

#include "pdaf_lib.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COMMON, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COMMON, fmt, ##args)

/* for BET test */
void *bet_hw_update_params;

/** isp_trigger_thread_process_queue:
 *
 *  @session_param: session param
 *
 *  Handle event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_trigger_thread_process_queue(mct_module_t *module,
  isp_session_param_t *session_param, uint32_t frame_id,  isp_resource_t *isp_resource)
{
  boolean                      ret = TRUE;
  void                        *data = NULL;
  uint32_t                     i = 0;
  isp_saved_events_t          *cur_events = NULL;
  isp_reg_update_state_t       reg_update_state;

  if (!session_param) {
    ISP_ERR("failed: %p", session_param);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&session_param->state_mutex);
  if (session_param->state == ISP_STATE_IDLE) {
    PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);
    return ret;
  }
  PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);

  if (session_param->hw_update_params.skip_hw_update == FALSE) {

    /* Call internal hw update event */
    ret = isp_util_trigger_internal_hw_update_event(session_param,
      session_param->session_based_ide, frame_id, session_param->hw_id,
      session_param->num_isp, &session_param->trigger_update_params.cur_events);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_trigger_internal_hw_update_event");
    }
  }

  ret = isp_util_post_cds_update(module, session_param);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_update_hw_param");
  }

  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_VFE_HW_UPDATE);
  if (session_param->hw_update_params.skip_hw_update == FALSE) {
    bet_hw_update_params = (void *)&session_param->hw_update_params;
    ret = isp_hw_update_util_do_ioctl_in_hw_update_params(isp_resource,
      session_param);

    if (ret == FALSE) {
      ISP_ERR("failed: isp_hw_update_util_do_ioctl_in_hw_update_params");
    }

    ret = isp_hw_update_util_request_reg_update(session_param);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_request_reg_update");
    }
  }
  ATRACE_CAMSCOPE_END(CAMSCOPE_VFE_HW_UPDATE);

  isp_util_clear_saved_events(
    &session_param->trigger_update_params.cur_events);

  return ret;
}

/** isp_trigger_thread_clear_saved_event:
 *
 *  @saved_events: handle to saved events
 *
 *  Invalidate set param and module events
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_trigger_thread_clear_saved_event(
  isp_saved_events_t *saved_events)
{
  uint32_t i = 0;

  if (!saved_events) {
    ISP_ERR("failed: saved_events %p", saved_events);
    return FALSE;
  }

  /* Invalidate set param events */
  for (i = 0; i < ISP_SET_MAX; i++) {
    saved_events->set_params_valid[i] = FALSE;
  }

  /* Invalidate module events */
  for (i = 0; i < ISP_MODULE_EVENT_MAX; i++) {
    saved_events->module_events_valid[i] = FALSE;
  }

  isp_util_clear_invalid_saved_event(saved_events);

  return TRUE;
}

/** isp_trigger_thread_clear_events:
 *
 *  @session_param: session param
 *
 *  Free cur_queue and new_queue
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_trigger_thread_clear_events(
  isp_session_param_t *session_param)
{
  boolean                      ret = TRUE;
  isp_trigger_update_params_t *trigger_update_params = NULL;
  isp_hw_id_t                  hw_id = 0;
  isp_hw_update_list_params_t *hw_update_list_params = NULL;

  if (!session_param) {
    ISP_ERR("failed: session_param %p", session_param);
    return FALSE;
  }

  trigger_update_params = &session_param->trigger_update_params;
  PTHREAD_MUTEX_LOCK(&trigger_update_params->mutex);
  ret = isp_trigger_thread_clear_saved_event(
    &trigger_update_params->cur_events);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_trigger_thread_clear_saved_event for cur_events");
  }
  ret = isp_trigger_thread_clear_saved_event(
    &trigger_update_params->new_events);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_trigger_thread_clear_saved_event for new_events");
  }
  PTHREAD_MUTEX_UNLOCK(&trigger_update_params->mutex);

  return TRUE;
} /* isp_trigger_thread_clear_events */

/** isp_trigger_thread_func:
 *
 *  @data: handle to session_param
 *
 *  ISP main thread handler
 *
 *  Returns NULL
 **/
static void *isp_trigger_thread_func(void *data)
{
  boolean                      ret = TRUE;
  isp_trigger_thread_priv_t   *thread_priv = NULL;
  mct_module_t                *module = NULL;
  isp_session_param_t         *session_param = NULL;
  struct pollfd                pollfds;
  int32_t                      num_fds = 1, ready = 0, i = 0, read_bytes = 0;
  isp_trigger_thread_event_t   event;
  isp_trigger_update_params_t *trigger_params = NULL;
  boolean                      exit_thread = FALSE;
  isp_resource_t              *isp_resource = NULL;

  if (!data) {
    ISP_ERR("failed: data %p", data);
    return NULL;
  }

  ISP_DBG("isp_new_thread trigger thread start");
  thread_priv = (isp_trigger_thread_priv_t *)data;
  module = thread_priv->module;
  session_param = thread_priv->session_param;
  isp_resource = thread_priv->isp_resource;
  trigger_params = &session_param->trigger_update_params;
  PTHREAD_MUTEX_LOCK(&trigger_params->mutex);
  trigger_params->is_thread_alive = TRUE;
  pthread_cond_signal(&trigger_params->cond);
  PTHREAD_MUTEX_UNLOCK(&trigger_params->mutex);

  while (exit_thread == FALSE) {
    pollfds.fd = trigger_params->pipe_fd[READ_FD];
    pollfds.events = POLLIN|POLLPRI;
    ready = poll(&pollfds, (nfds_t)num_fds, -1);
    if (ready > 0) {
      if (pollfds.revents & (POLLIN|POLLPRI)) {
        read_bytes = read(pollfds.fd, &event,
          sizeof(isp_trigger_thread_event_t));
        if ((read_bytes < 0) ||
            (read_bytes != sizeof(isp_trigger_thread_event_t))) {
          ISP_ERR("failed: read_bytes %d", read_bytes);
          continue;
        }
        switch (event.type) {
        case ISP_TRIGGER_THREAD_EVENT_PROCESS_QUEUE:
          ATRACE_BEGIN_SNPRINTF(30, "VFE_TRIG_UPDATE %d", event.frame_id);
          MCT_PROF_LOG_BEG("isp_hw_update_thread_process");
          ret = isp_trigger_thread_process_queue(module,session_param, event.frame_id,
                  isp_resource);
          MCT_PROF_LOG_END();
          ATRACE_END();
          if (ret == FALSE) {
            ISP_ERR("failed: isp_trigger_thread_process_queue");
          }
          break;
        case ISP_TRIGGER_THREAD_EVENT_CLEAR_EVENTS:
          ret = isp_trigger_thread_clear_events(session_param);
          if (ret == FALSE) {
            ISP_ERR("failed: isp_trigger_thread_clear_events");
          }
          PTHREAD_MUTEX_LOCK(&trigger_params->mutex);
          pthread_cond_signal(&trigger_params->cond);
          PTHREAD_MUTEX_UNLOCK(&trigger_params->mutex);
          break;
        case ISP_TRIGGER_THREAD_EVENT_ABORT_THREAD:
          exit_thread = TRUE;
          break;
        default:
          ISP_ERR("failed: invalid event type %d", event.type);
          break;
        }
        PTHREAD_MUTEX_LOCK(&session_param->thread_busy_mutex);
        trigger_params->is_thread_busy = FALSE;
        PTHREAD_MUTEX_UNLOCK(&session_param->thread_busy_mutex);
      }
    } else if (ready <= 0) {
      if (errno != EINTR) {
        ISP_ERR("failed: exit thread");
        break;
      }
    }
  }

  return NULL;
}

/** isp_trigger_thread_alloc_and_copy_control_parm:
 *
 *  @event: source event handle
 *  @control_parm: source payload
 *  @set_parm_size: set parm size
 *  @copy_event: destination event handle
 *
 *  Alloc copy event and control parm in copy_event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_trigger_thread_alloc_and_copy_control_parm(
  mct_event_t *event, mct_event_control_parm_t *control_parm,
  uint32_t set_parm_size, mct_event_t **copy_event, uint32_t session_based_ide)
{
  mct_event_control_parm_t *copy_control_parm = NULL;
  void                     *copy_parm_data = NULL;

  if (!event || !control_parm || !copy_event || !set_parm_size) {
    ISP_ERR("failed: %p %p %p size %d", event, control_parm, copy_event,
      set_parm_size);
    return FALSE;
  }

  if (!control_parm->parm_data) {
    ISP_ERR("failed: control_parm->parm_data %p", control_parm->parm_data);
    return FALSE;
  }

  *copy_event = NULL;

  /* Copy set parm payload */
  copy_parm_data = CAM_MALLOC(set_parm_size);
  RETURN_IF_NULL(copy_parm_data);
  memset(copy_parm_data, 0, set_parm_size);
  memcpy(copy_parm_data, control_parm->parm_data, set_parm_size);

  /* Copy set param */
  copy_control_parm =
    (mct_event_control_parm_t *)CAM_MALLOC(sizeof(mct_event_control_parm_t));
  if (!copy_control_parm) {
    ISP_ERR("failed: copy_control_parm %p", copy_control_parm);
    goto ERROR_COPY_PARM;
  }
  memset(copy_control_parm, 0, sizeof(sizeof(mct_event_control_parm_t)));
  memcpy(copy_control_parm, control_parm,
    sizeof(sizeof(mct_event_control_parm_t)));
  copy_control_parm->parm_data = copy_parm_data;

  /* Copy mct event */
  *copy_event = (mct_event_t *)CAM_MALLOC(sizeof(mct_event_t));
  if (!(*copy_event)) {
    ISP_ERR("failed: copy_event %p", *copy_event);
    goto ERROR_COPY_EVENT;
  }
  memcpy(*copy_event, event, sizeof(mct_event_t));
  (*copy_event)->identity = session_based_ide;
  (*copy_event)->u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  (*copy_event)->u.ctrl_event.control_event_data = copy_control_parm;

  return TRUE;

ERROR_COPY_EVENT:
  free(copy_control_parm);
ERROR_COPY_PARM:
  free(copy_parm_data);
  return FALSE;
}

/** isp_trigger_thread_deep_copy_control_parm:
 *
 *  @event: source event handle
 *  @control_parm: source payload
 *  @set_parm_size: set parm size
 *  @copy_event: destination event handle
 *
 *  Copy source event in copy_event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_trigger_thread_deep_copy_control_parm(
  mct_event_t *event, mct_event_control_parm_t *control_parm,
  uint32_t set_parm_size, mct_event_t *copy_event, uint32_t session_based_ide)
{
  mct_event_control_parm_t *copy_control_parm = NULL;
  void                     *set_parm_data = NULL;
  void                     *copy_set_parm_data = NULL;

  if (!event || !control_parm || !copy_event || !set_parm_size) {
    ISP_ERR("failed: %p %p %p size %d", event, control_parm, copy_event,
      set_parm_size);
    return FALSE;
  }

  set_parm_data = control_parm->parm_data;
  if (!set_parm_data) {
    ISP_ERR("failed: set_parm_data %p", set_parm_data);
    return FALSE;
  }

  copy_control_parm =
    (mct_event_control_parm_t *)copy_event->u.ctrl_event.control_event_data;
  if (!copy_control_parm) {
    ISP_ERR("failed: copy_control_parm %p", copy_control_parm);
    return FALSE;
  }

  copy_set_parm_data = copy_control_parm->parm_data;
  if (!copy_set_parm_data) {
    ISP_ERR("failed: copy_set_parm_data %p", copy_set_parm_data);
    return FALSE;
  }

  /* Take copy of event data */
  *copy_event = *event;
  /* Set event's identity to session based ide */
  copy_event->identity = session_based_ide;
  /* Event type may be set param or super parm, replace it with set param */
  copy_event->u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  /* Since control param pointer will be overwritten, reassign back to copy
     control parm */
  copy_event->u.ctrl_event.control_event_data = (void *)copy_control_parm;
  /* Take copy of control parm */
  *copy_control_parm = *control_parm;
  /* Size set parm data pointer will be overwritten, reassign back to
     copy set parm data */
  copy_control_parm->parm_data = copy_set_parm_data;
  /* Take deep copy of set parm data */
  memcpy(copy_set_parm_data, control_parm->parm_data, set_parm_size);

  return TRUE;
} /* isp_trigger_thread_deep_copy_control_parm */

/** isp_trigger_thread_copy_control_event:
 *
 *  @event: event handle
 *  @saved_events: handle to saved events
 *
 *  Take local copy of control event to be handled by trigger
 *  thread
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_trigger_thread_copy_control_event(mct_event_t *event,
  isp_saved_events_t *saved_events, uint32_t session_based_ide)
{
  boolean                        ret = TRUE;
  uint32_t                       event_size = 0;
  mct_event_control_t           *ctrl_event = NULL;

  if (!event || !saved_events) {
    ISP_ERR("failed: %p %p", event, saved_events);
    return FALSE;
  }

  ctrl_event = &event->u.ctrl_event;

  switch (ctrl_event->type) {
  case MCT_EVENT_CONTROL_SET_PARM: {
    mct_event_control_parm_t *control_parm = NULL;
    isp_set_param_type_t      set_parm_type = ISP_SET_MAX;
    uint32_t                  set_parm_size = 0;

    control_parm = (mct_event_control_parm_t *)ctrl_event->control_event_data;
    if (!control_parm) {
      ISP_ERR("failed: control_parm %p", control_parm);
      break;
    }

    ret = isp_util_convert_set_parm_event_type(control_parm->type,
      &set_parm_type, &set_parm_size);
    if ((ret == FALSE) || (set_parm_type >= ISP_SET_MAX) || !set_parm_size) {
      break;
    }

    if (!saved_events->set_params[set_parm_type]) {
      ret = isp_trigger_thread_alloc_and_copy_control_parm(event,
        control_parm, set_parm_size, &saved_events->set_params[set_parm_type],
        session_based_ide);
      if (ret == FALSE) {
        ISP_ERR("failed: copy isp_trigger_thread_alloc_and_copy_control_event");
        break;
      }
    } else {
      ret = isp_trigger_thread_deep_copy_control_parm(event, control_parm,
        set_parm_size, saved_events->set_params[set_parm_type],
        session_based_ide);
      if (ret == FALSE) {
        ISP_ERR("failed: copy isp_trigger_thread_copy_control_parm");
        break;
      }
    }
    saved_events->set_params_valid[set_parm_type] = TRUE;
  }
    break;

  case MCT_EVENT_CONTROL_SET_SUPER_PARM: {
    mct_event_super_control_parm_t *data = NULL;
    uint32_t                        i = 0;
    mct_event_super_control_parm_t *param = NULL;
    mct_event_control_parm_t       *control_parm = NULL;
    isp_set_param_type_t            set_parm_type = ISP_SET_MAX;
    uint32_t                        set_parm_size = 0;

    param = (mct_event_super_control_parm_t *)ctrl_event->control_event_data;
    if (!param) {
      ISP_ERR("failed: param %p", param);
      break;
    }

    for (i = 0; i < param->num_of_parm_events; i++) {
      control_parm = &param->parm_events[i];
      set_parm_type = ISP_SET_MAX;
      set_parm_size = 0;

      ret = isp_util_convert_set_parm_event_type(control_parm->type,
        &set_parm_type, &set_parm_size);
      if ((ret == FALSE) || (set_parm_type >= ISP_SET_MAX) || !set_parm_size) {
        continue;
      }

      if (!saved_events->set_params[set_parm_type]) {
        ret = isp_trigger_thread_alloc_and_copy_control_parm(event,
          control_parm, set_parm_size,
          &saved_events->set_params[set_parm_type], session_based_ide);
        if (ret == FALSE) {
          ISP_ERR("failed: type %d", control_parm->type);
          continue;
        }
      } else {
        ret = isp_trigger_thread_deep_copy_control_parm(event, control_parm,
          set_parm_size, saved_events->set_params[set_parm_type],
          session_based_ide);
        if (ret == FALSE) {
          ISP_ERR("failed: type %d", control_parm->type);
          continue;
        }
      }
      saved_events->set_params_valid[set_parm_type] = TRUE;
    }
  }
    break;

  default:
    break;
  }
  return TRUE;
}

/** isp_trigger_thread_alloc_and_copy_module_event:
 *
 *  @event: handle to source event
 *  @module_event_data_size: module event data size
 *  @copy_event: handle to destination event
 *
 *  Alloc copy_event and copy souce event to copy event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_trigger_thread_alloc_and_copy_module_event(
  mct_event_t *event, uint32_t module_event_data_size,
  mct_event_t **copy_event, uint32_t session_based_ide)
{
  void *copy_module_event_data = NULL;
  void *module_event_data = NULL;

  if (!event || !copy_event || !module_event_data_size) {
    ISP_ERR("failed: %p %p size %d", event, copy_event, module_event_data_size);
    return FALSE;
  }

  *copy_event = NULL;

  module_event_data = event->u.module_event.module_event_data;
  if (!module_event_data) {
    ISP_ERR("failed: module_event_data %p", module_event_data);
    return FALSE;
  }

  /* Copy module event payload */
  copy_module_event_data = (void *)CAM_MALLOC(module_event_data_size);
  if (!copy_module_event_data) {
    ISP_ERR("failed: copy_module_event_data %p", copy_module_event_data);
    return FALSE;
  }
  memcpy(copy_module_event_data, module_event_data, module_event_data_size);

  /* Copy module event */
  *copy_event = (mct_event_t *)CAM_MALLOC(sizeof(mct_event_t));
  if (!(*copy_event)) {
    ISP_ERR("failed: copy_event %p", *copy_event);
    goto ERROR_COPY_EVENT;
  }
  memcpy(*copy_event, event, sizeof(mct_event_t));
  (*copy_event)->identity = session_based_ide;
  (*copy_event)->u.module_event.module_event_data = copy_module_event_data;

  return TRUE;

ERROR_COPY_EVENT:
  free(copy_module_event_data);
  return FALSE;
}

/** isp_trigger_thread_deep_copy_module_event:
 *
 *  @event: handle to source event
 *  @module_event_data_size: module event data size
 *  @copy_event: handle to destination event
 *
 *  Take deep copy of module event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_trigger_thread_deep_copy_module_event(mct_event_t *event,
  uint32_t module_event_data_size, mct_event_t *copy_event,
  uint32_t session_based_ide)
{
  void *copy_module_event_data = NULL;
  void *module_event_data = NULL;

  if (!event || !module_event_data_size || !copy_event) {
    ISP_ERR("failed: %p %p size %d", event, copy_event, module_event_data_size);
    return FALSE;
  }

  module_event_data = event->u.module_event.module_event_data;
  if (!module_event_data) {
    ISP_ERR("failed: module_event_data %p", module_event_data);
    return FALSE;
  }

  copy_module_event_data = copy_event->u.module_event.module_event_data;
  if (!copy_module_event_data) {
    ISP_ERR("failed: copy_module_event_data %p", copy_module_event_data);
    return FALSE;
  }

  /* Take copy of module event */
  *copy_event = *event;
  /* Set event identity to session based ide */
  copy_event->identity = session_based_ide;
  /* Take copy of module event payload */
  memcpy(copy_module_event_data, module_event_data, module_event_data_size);
  /* Since module event data pointer will be overwritten, replace it
     with proper pointer */
  copy_event->u.module_event.module_event_data = copy_module_event_data;

  return TRUE;
}

/** isp_trigger_thread_copy_other_module_event:
 *
 *  @event: source event
 *  @saved_events: handle to saved events
 *  @isp_event: isp internal event type
 *  @event_size: event size
 *
 *  During first time, allocate and copy event. From second
 *  time, deep copy event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_trigger_thread_copy_other_module_event(
  mct_event_t *event, isp_saved_events_t *saved_events,
  isp_module_event_type_t isp_event, uint32_t event_size,
  uint32_t session_based_ide)
{
  boolean ret = TRUE;

  if (!event || !saved_events || (isp_event >= ISP_MODULE_EVENT_MAX) ||
    !event_size) {
    ISP_ERR("failed: %p %p isp_event %d event_size %d", event, saved_events,
      isp_event, event_size);
    return FALSE;
  }

  if (!saved_events->module_events[isp_event]) {
    ret = isp_trigger_thread_alloc_and_copy_module_event(event, event_size,
      &saved_events->module_events[isp_event], session_based_ide);
    if ((ret == FALSE) || !saved_events->module_events[isp_event]) {
      ISP_ERR("failed: ret %d event %p", ret,
        saved_events->module_events[isp_event]);
      return FALSE;
    }
  } else {
    ret = isp_trigger_thread_deep_copy_module_event(event, event_size,
      saved_events->module_events[isp_event], session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_trigger_thread_deep_copy_module_event");
      return FALSE;
    }
  }
  saved_events->module_events_valid[isp_event] = TRUE;

  return TRUE;
} /* isp_trigger_thread_copy_other_module_event */

/** isp_trigger_thread_copy_af_rolloff_params:
 *
 *  @event: source event
 *  @saved_events: handle to saved events
 *
 *  Copy AF rolloff params event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_trigger_thread_copy_af_rolloff_params(
  mct_event_t *event, isp_saved_events_t *saved_events)
{
  af_rolloff_info_t         *af_rolloff_info = NULL;
  af_rolloff_info_t         *copy_af_rolloff_info = NULL;
  chromatix_VFE_common_type *chromatix_data = NULL;
  isp_module_event_type_t    isp_event = ISP_MODULE_EVENT_SET_AF_ROLLOFF_PARAMS;
  mct_event_t               *copy_event = NULL;
  void                      *copy_rolloff_tables_macro = NULL;

  if (!event || !saved_events) {
    ISP_ERR("failed: %p %p", event, saved_events);
    return FALSE;
  }

  af_rolloff_info =
    (af_rolloff_info_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(af_rolloff_info);

  if (!saved_events->module_events[isp_event]) {
    copy_af_rolloff_info =
      (af_rolloff_info_t *)CAM_MALLOC(sizeof(*copy_af_rolloff_info));
    RETURN_IF_NULL(copy_af_rolloff_info);

    chromatix_data =
      (chromatix_VFE_common_type *)CAM_MALLOC(sizeof(*chromatix_data));
    if (!chromatix_data) {
      ISP_ERR("failed: chromatix_data %p", chromatix_data);
      goto ERROR_CHROMATIX_DATA;
    }
    memset(chromatix_data, 0, sizeof(*chromatix_data));
    memset(copy_af_rolloff_info, 0, sizeof(*copy_af_rolloff_info));
    *copy_af_rolloff_info = *af_rolloff_info;
    memcpy(chromatix_data, af_rolloff_info->rolloff_tables_macro,
      sizeof(*chromatix_data));
    copy_af_rolloff_info->rolloff_tables_macro =
      (void *)chromatix_data;

    saved_events->module_events[isp_event] =
      (mct_event_t *)CAM_MALLOC(sizeof(mct_event_t));
    if (!saved_events->module_events[isp_event]) {
      ISP_ERR("failed: saved_events->module_events[isp_event] %p",
        saved_events->module_events[isp_event]);
      goto ERROR_MODULE_EVENT;
    }
    *saved_events->module_events[isp_event] = *event;
    saved_events->module_events[isp_event]->u.module_event.module_event_data =
      (void *)copy_af_rolloff_info;
  } else {
    copy_event = saved_events->module_events[isp_event];
    copy_af_rolloff_info = copy_event->u.module_event.module_event_data;
    if (!copy_af_rolloff_info) {
      ISP_ERR("copy_af_rolloff_info %p", copy_af_rolloff_info);
      return FALSE;
    }
    copy_rolloff_tables_macro = copy_af_rolloff_info->rolloff_tables_macro;
    if (!copy_rolloff_tables_macro) {
      ISP_ERR("copy_rolloff_tables_macro %p", copy_rolloff_tables_macro);
      return FALSE;
    }
    memcpy(copy_rolloff_tables_macro, af_rolloff_info->rolloff_tables_macro,
      sizeof(chromatix_VFE_common_type));

    *copy_af_rolloff_info = *af_rolloff_info;
    copy_af_rolloff_info->rolloff_tables_macro = copy_rolloff_tables_macro;

    *copy_event = *event;
    copy_event->u.module_event.module_event_data = (void *)copy_af_rolloff_info;
  }
  saved_events->module_events_valid[isp_event] = TRUE;

  return TRUE;

ERROR_MODULE_EVENT:
  free(chromatix_data);
ERROR_CHROMATIX_DATA:
  free(copy_af_rolloff_info);
  return FALSE;
}

/** isp_trigger_thread_check_isp_bit
 *
 *  @data: payload of aec_update event
 *
 *  Consume event only if ISP mask bit is set
 *
 *  switch real gain value to long real gain value
 **/
static boolean isp_trigger_thread_check_isp_bit(
  void *data)
{
  stats_update_t       *stats_update = NULL;

  if (!data) {
    ISP_ERR("failed: data NULL %p", data);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;

  /* Check new capture mode and destination */
  if (stats_update->flag & STATS_UPDATE_AEC) {
    if ((stats_update->aec_update.aec_dst != STATS_UPDATE_DST_ALL) &&
      !(stats_update->aec_update.aec_dst & STATS_UPDATE_DST_ISP)) {
      ISP_DBG("no need to update AEC");
      return FALSE;
    }
    ISP_DBG("real_gain to isp %f", stats_update->aec_update.real_gain);
  } else if (stats_update->flag & STATS_UPDATE_AWB) {
    if ((stats_update->awb_update.awb_dst != STATS_UPDATE_DST_ALL) &&
      !(stats_update->awb_update.awb_dst & STATS_UPDATE_DST_ISP)) {
      ISP_DBG("no need to update AWB");
      return FALSE;
    }
  }

  return TRUE;
}

/** isp_trigger_thread_copy_module_event:
 *
 *  @module_event: module event handle
 *  @module_event_data: data pointer to store copied event
 *
 *  Take local copy of module event to be handled by trigger
 *  update thread
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_trigger_thread_copy_module_event(
  mct_event_t *event, isp_saved_events_t *saved_events,
  uint32_t session_based_ide)
{
  boolean                  ret = TRUE;
  mct_event_module_t      *module_event = NULL;
  uint32_t                 event_size = 0;
  void                    *data = NULL;
  isp_module_event_type_t  isp_event = 0;

  if (!event || !saved_events) {
    ISP_ERR("failed: %p %p", event, saved_events);
    return FALSE;
  }

  module_event = &event->u.module_event;
  ISP_DBG("module event type %d", module_event->type);
  switch (module_event->type) {
  case MCT_EVENT_MODULE_ISP_CDS_REQUEST:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_CDS_REQUEST, sizeof(isp_cds_request_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: ISP_MODULE_EVENT_CDS_REQUEST");
    }
    break;

  case MCT_EVENT_MODULE_ISP_LA_ALGO_UPDATE: {
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_LA_ALGO_UPDATE, sizeof(isp_saved_la_params_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_ISP_LA_ALGO_UPDATE");
    }
  }
    break;

  case MCT_EVENT_MODULE_ISP_LTM_ALGO_UPDATE: {
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_LTM_ALGO_UPDATE, sizeof(isp_saved_ltm_params_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_ISP_LTM_ALGO_UPDATE");
    }
  }
    break;

  case MCT_EVENT_MODULE_ISP_GTM_ALGO_UPDATE: {
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_GTM_ALGO_UPDATE, sizeof(isp_saved_gtm_params_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_ISP_GTM_ALGO_UPDATE");
    }
  }
    break;

  case MCT_EVENT_MODULE_SET_FLASH_MODE:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_SET_FLASH_MODE, sizeof(cam_flash_mode_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_SET_FLASH_MODE");
    }
    break;

  case MCT_EVENT_MODULE_SET_DIGITAL_GAIN:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_SET_DIGITAL_GAIN, sizeof(float),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_SET_DIGITAL_GAIN");
    }
    break;

  case MCT_EVENT_MODULE_STATS_AWB_MANUAL_UPDATE:
    /* manual event from stats, This is not same as manual control from app
     */
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_MANUAL_AWB_UPDATE, sizeof(stats_update_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_STATS_AWB_MANUAL_UPDATE");
    }
    break;

  case MCT_EVENT_MODULE_SET_FAST_AEC_CONVERGE_MODE:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_SET_FAST_AEC_CONVERGE_MODE, sizeof(mct_fast_aec_mode_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_SET_FAST_AEC_CONVERGE_MODE");
    }
    break;

  case MCT_EVENT_MODULE_SET_CHROMATIX_PTR:
  case MCT_EVENT_MODULE_SET_RELOAD_CHROMATIX:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_SET_CHROMATIX_PTR, sizeof(modulesChromatix_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_SET_CHROMATIX_PTR");
    }
    break;

  case MCT_EVENT_MODULE_STATS_AEC_MANUAL_UPDATE:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_STATS_AEC_MANUAL_UPDATE, sizeof(aec_manual_update_t),
	  session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: ISP_MODULE_EVENT_STATS_AEC_MANUAL_UPDATE");
    }
    break;

  case MCT_EVENT_MODULE_STATS_AEC_UPDATE:
    {
      stats_update_t        *stats_update;
      stats_update = (stats_update_t *)event->u.module_event.module_event_data;
      ISP_DBG("STATS G %f LC %d EXP_TIME %f SGAIN %f TOTDRC-GAIN %f GTM %f"
              " LTM %f LA %f GA %f COLOR_DRC %f)",
        stats_update->aec_update.real_gain,
        stats_update->aec_update.linecount,
        stats_update->aec_update.exp_time,
        stats_update->aec_update.sensor_gain,
        stats_update->aec_update.total_drc_gain,
        stats_update->aec_update.gtm_ratio,
        stats_update->aec_update.ltm_ratio,
        stats_update->aec_update.la_ratio,
        stats_update->aec_update.gamma_ratio,
        stats_update->aec_update.color_drc_gain);
    }
    ret = isp_trigger_thread_check_isp_bit(event->u.module_event.module_event_data);
    if (ret == TRUE) {
      ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
        ISP_MODULE_EVENT_STATS_AEC_UPDATE, sizeof(stats_update_t),
      session_based_ide);
      if (ret == FALSE) {
        ISP_ERR("failed: MCT_EVENT_MODULE_STATS_AEC_UPDATE");
      }
    } else {
      ret = TRUE;
    }
    break;

  case MCT_EVENT_MODULE_STATS_AWB_UPDATE:
    ret = isp_trigger_thread_check_isp_bit(event->u.module_event.module_event_data);
    if (ret == TRUE) {
      ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
        ISP_MODULE_EVENT_STATS_AWB_UPDATE, sizeof(stats_update_t),
      session_based_ide);
      if (ret == FALSE) {
        ISP_ERR("failed: ISP_MODULE_EVENT_STATS_AWB_UPDATE");
      }
    } else {
      ret = TRUE;
    }
    break;

  case MCT_EVENT_MODULE_STATS_ASD_UPDATE:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_STATS_ASD_UPDATE, sizeof(stats_update_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_STATS_ASD_UPDATE");
    }
    break;

  case MCT_EVENT_MODULE_STATS_AEC_CONFIG_UPDATE:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_STATS_AEC_CONFIG_UPDATE, sizeof(aec_config_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_STATS_AEC_CONFIG_UPDATE");
    }
    break;

  case MCT_EVENT_MODULE_STATS_AWB_CONFIG_UPDATE:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_STATS_AWB_CONFIG_UPDATE, sizeof(awb_config_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_STATS_AWB_CONFIG_UPDATE");
    }
    break;

  case MCT_EVENT_MODULE_STATS_AF_CONFIG_UPDATE:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_STATS_AF_CONFIG_UPDATE, sizeof(af_config_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_STATS_AF_CONFIG_UPDATE");
    }
    break;

  case MCT_EVENT_MODULE_AF_EXP_COMPENSATE:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_AF_EXP_COMPENSATE, sizeof(boolean),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_AF_EXP_COMPENSATE");
    }
    break;

  case MCT_EVENT_MODULE_STATS_RS_CONFIG_UPDATE:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_STATS_RS_CONFIG_UPDATE, sizeof(rs_config_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_STATS_RS_CONFIG_UPDATE");
    }
    break;

  case MCT_EVENT_MODULE_SET_AF_ROLLOFF_PARAMS:
    ret = isp_trigger_thread_copy_af_rolloff_params(event, saved_events);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_SET_AF_ROLLOFF_PARAMS");
    }
    break;

  case MCT_EVENT_MODULE_SENSOR_LENS_POSITION_UPDATE:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_SENSOR_LENS_POSITION_UPDATE,
      sizeof(lens_position_update_isp_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_SENSOR_LENS_POSITION_UPDATE");
    }
    break;

  case MCT_EVENT_MODULE_ISP_TINTLESS_ALGO_UPDATE:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_TINTLESS_ALGO_UPDATE,
      sizeof(isp_saved_tintless_params_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_ISP_TINTLESS_ALGO_UPDATE");
    }
    break;

  case MCT_EVENT_MODULE_SET_DEFECTIVE_PIXELS:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_SET_DEFECTIVE_PIXELS,
      sizeof(defective_pix_array_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_SET_DEFECTIVE_PIXELS");
    }
    break;
  case MCT_EVENT_MODULE_ISP_HDR:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE,
      sizeof(isp_hdr_mode_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_ISP_HDR");
    }
    break;

  case MCT_EVENT_MODULE_SENSOR_PDAF_CONFIG:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_SET_PDAF_PATTERN,
      sizeof(pdaf_sensor_native_info_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_SENSOR_PDAF_CONFIG");
    }
    break;

  case MCT_EVENT_MODULE_ISP_DISABLE_MODULE:
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_ISP_DISABLE_MODULE,
      sizeof(uint64_t),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_ISP_DISABLE_MODULE");
    }
    break;

  case MCT_EVENT_MODULE_STATS_FOVC_MAGNIFICATION_FACTOR_ISP:
    ISP_DBG("[zoomisp]: zoom_val %f\n", *(float *)event->u.module_event.module_event_data);
    ret = isp_trigger_thread_copy_other_module_event(event, saved_events,
      ISP_MODULE_EVENT_STATS_FOVC_MAGNIFICATION_FACTOR,
      sizeof(float),
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_STATS_FOVC_MAGNIFICATION_FACTOR_ISP");
    }
    break;

  default:
    break;
  }
  return ret;
}

/** isp_trigger_thread_enqueue_event:
 *
 *  @event: event to be enqueued
 *  @saved_events: handle to saved_events
 *
 *  Take local copy of event, enqueue in trigger update queue
 *  and post msg to trigger thread to wake up
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_trigger_thread_enqueue_event(mct_event_t *event,
  isp_saved_events_t *saved_events, uint32_t session_based_ide)
{
  boolean                      ret = TRUE;
  mct_event_t                 *copy_event = NULL;
  isp_trigger_update_params_t *trigger_update_params = NULL;

  if (!event || !saved_events) {
    ISP_ERR("failed: %p %p", event, saved_events);
    return FALSE;
  }

  if ((event->type != MCT_EVENT_CONTROL_CMD) &&
      (event->type != MCT_EVENT_MODULE_EVENT)) {
    ISP_ERR("failed: invalid event->type %d", event->type);
    return FALSE;
  }

  if (event->type == MCT_EVENT_CONTROL_CMD) {
    ret = isp_trigger_thread_copy_control_event(event, saved_events,
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_trigger_thread_copy_ctrl_event");
      return FALSE;
    }
  } else if (event->type == MCT_EVENT_MODULE_EVENT) {
    ret = isp_trigger_thread_copy_module_event(event, saved_events,
      session_based_ide);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_trigger_thread_copy_module_event");
      return FALSE;
    }
  }

  return ret;
}

/** isp_trigger_thread_post_message:
 *
 *  @trigger_update_params: trigger update params handle
 *  @type: type of message to be posted
 *  @frame_id: frame id
 *  @session_param: Session param
 *
 *  Create message and post to trigger update thread
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_trigger_thread_post_message(
  isp_trigger_update_params_t *trigger_update_params,
  isp_thread_event_type_t type, uint32_t frame_id,
  isp_session_param_t *session_param)
{
  int32_t                    rc = 0;
  boolean                    ret = TRUE;
  isp_trigger_thread_event_t trigger_thread_event;
  isp_trigger_update_params_t *trigger_params = NULL;

  if (!trigger_update_params || (type >= ISP_TRIGGER_THREAD_EVENT_MAX) || !session_param) {
    ISP_ERR("failed: params %p type %d", trigger_update_params, type);
    return FALSE;
  }
  trigger_params = &session_param->trigger_update_params;
  /* wake up trigger update thread to process the event */
  memset(&trigger_thread_event, 0, sizeof(trigger_thread_event));
  trigger_thread_event.type = type;
  trigger_thread_event.frame_id = frame_id;

  PTHREAD_MUTEX_LOCK(&session_param->thread_busy_mutex);
  trigger_params->is_thread_busy = TRUE;
  PTHREAD_MUTEX_UNLOCK(&session_param->thread_busy_mutex);
  rc = write(trigger_update_params->pipe_fd[WRITE_FD], &trigger_thread_event,
    sizeof(trigger_thread_event));
  if(rc < 0) {
    ISP_ERR("failed: rc %d", rc);
    return FALSE;
  }

  return ret;
}

/** isp_trigger_thread_store_new_params_for_offline:
 *
 *  @session_param: session params
 *  @frame_id: frame id
 *
 *  Enqueue new_queue params in cur_queue and schedule
 *  trigger update thread
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_trigger_thread_store_new_params_for_offline(
  isp_session_param_t *session_param, uint32_t frame_id)
{
  boolean                      ret = TRUE;
  void                        *data = NULL;
  uint32_t                     i = 0;
  isp_trigger_update_params_t *trigger_update_params = NULL;
  isp_saved_events_t          *cur_events = NULL;
  isp_saved_events_t          *new_events = NULL;

  if (!session_param) {
    ISP_ERR("failed: %p", session_param);
    return FALSE;
  }

  MCT_PROF_LOG_BEG(__func__);

   trigger_update_params = &session_param->trigger_update_params;

   cur_events = CAM_MALLOC(sizeof(*cur_events));
   if (!cur_events) {
     ISP_ERR("failed: param CAM_MALLOC");
     return FALSE;
   }
   memset(cur_events, 0, sizeof(*cur_events));

   PTHREAD_MUTEX_LOCK(&trigger_update_params->mutex);
   cur_events->frame_id = frame_id;
   new_events = &trigger_update_params->new_events;
   CDBG("Queue trigger params for frame_id %d", frame_id);
   /* Copy set params */
   for (i = 0; i < ISP_SET_MAX; i++) {
     if (new_events->set_params[i] &&
       (new_events->set_params_valid[i] == TRUE)) {
       ret = isp_trigger_thread_copy_control_event(new_events->set_params[i],
         cur_events, session_param->session_based_ide);
       if (ret == FALSE) {
         ISP_ERR("failed");
       }
       new_events->set_params_valid[i] = FALSE;/* Clear valid flag of new events */
     }
   }
   /* Copy module events */
   for (i = 0; i < ISP_MODULE_EVENT_MAX; i++) {
     if (new_events->module_events[i] &&
       (new_events->module_events_valid[i] == TRUE)) {
       ret = isp_trigger_thread_copy_module_event(new_events->module_events[i],
         cur_events, session_param->session_based_ide);
       if (ret == FALSE) {
         ISP_ERR("failed");
       }
       new_events->module_events_valid[i] = FALSE;/* Clear valid flag of new events */
     }
   }
   mct_queue_push_tail(session_param->offline_trigger_param_q, (void*)cur_events);

   PTHREAD_MUTEX_UNLOCK(&trigger_update_params->mutex);

  MCT_PROF_LOG_END();

  return TRUE;
}

/** isp_trigger_thread_handle_sof:
 *
 *  @trigger_update_params: trigger update params
 *  @frame_id: frame id
 *
 *  Enqueue new_queue params in cur_queue and schedule
 *  trigger update thread
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_trigger_thread_handle_sof(
  isp_trigger_update_params_t *trigger_update_params, uint32_t frame_id,
  isp_session_param_t *session_param)
{
  boolean             ret = TRUE;
  void               *data = NULL;
  uint32_t            i = 0;
  isp_saved_events_t *cur_events = NULL;
  isp_saved_events_t *new_events = NULL;

  if (!trigger_update_params || !session_param) {
    ISP_ERR("failed: %p %p", trigger_update_params, session_param);
    return FALSE;
  }

  MCT_PROF_LOG_BEG(__func__);

  PTHREAD_MUTEX_LOCK(&trigger_update_params->mutex);

  if (session_param->hw_update_params.skip_hw_update == FALSE) {
    cur_events = &trigger_update_params->cur_events;
    new_events = &trigger_update_params->new_events;
    /* Copy set params */
    for (i = 0; i < ISP_SET_MAX; i++) {
      if (new_events->set_params[i] &&
        (new_events->set_params_valid[i] == TRUE)) {
        ret = isp_trigger_thread_copy_control_event(new_events->set_params[i],
          cur_events, session_param->session_based_ide);
        if (ret == FALSE) {
          ISP_ERR("failed");
        }
        new_events->set_params_valid[i] = FALSE;
      }
    }
    /* Copy module events */
    for (i = 0; i < ISP_MODULE_EVENT_MAX; i++) {
      if (new_events->module_events[i] &&
        (new_events->module_events_valid[i] == TRUE)) {
        ret = isp_trigger_thread_copy_module_event(new_events->module_events[i],
          cur_events, session_param->session_based_ide);
        if (ret == FALSE) {
          ISP_ERR("failed");
        }
        new_events->module_events_valid[i] = FALSE;
      }
    }
  }
  ret = isp_trigger_thread_post_message(trigger_update_params,
    ISP_TRIGGER_THREAD_EVENT_PROCESS_QUEUE, frame_id, session_param);
  if (ret == FALSE) {
    ISP_ERR("failed: ISP_TRIGGER_THREAD_EVENT_PROCESS_QUEUE");
  }

  PTHREAD_MUTEX_UNLOCK(&trigger_update_params->mutex);

  MCT_PROF_LOG_END();

  return TRUE;
}

/** isp_trigger_thread_apply_shared_trigger_params:
 *
 *  @trigger_update_params: trigger update params
 *  @frame_id: frame id
 *
 *  Enqueue new_queue params in cur_queue and schedule
 *  trigger update thread
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_trigger_thread_apply_shared_trigger_params(
  uint32_t frame_id, isp_session_param_t *session_param)
{
  boolean                      ret = TRUE;
  void                        *data = NULL;
  uint32_t                     i = 0;
  isp_saved_events_t          *cur_events = NULL;
  isp_saved_events_t          *new_events = NULL;
  isp_trigger_update_params_t *trigger_update_params = NULL;
  uint32_t                     evt_frame_id;

  if (!session_param) {
    ISP_ERR("failed: %p", session_param);
    return FALSE;
  }
  trigger_update_params = &session_param->trigger_update_params;

  MCT_PROF_LOG_BEG(__func__);

  PTHREAD_MUTEX_LOCK(&trigger_update_params->mutex);

  cur_events = &trigger_update_params->cur_events;
  do {
    /* Check if we need to dequeue next configuration */
    new_events = mct_queue_look_at_head(session_param->offline_trigger_param_q);
    if (!new_events || (new_events->frame_id > frame_id)) {
      if (!new_events) {
        CDBG("New event is NULL");
      } else {
        CDBG("compare evt_frame_id %d cur_frame_id %d", new_events->frame_id, frame_id);
      }
      break;
    }
    /* dequeue configuration */
    new_events = mct_queue_pop_head(session_param->offline_trigger_param_q);
    if (!new_events) {
        CDBG_ERROR("mct_queue_pop_head failed");
        break;
    }
    evt_frame_id = new_events->frame_id;
    CDBG("DeQueue trigger params for evt_frame_id %d cur_frame_id %d", evt_frame_id, frame_id);

    /* Copy set params */
    for (i = 0; i < ISP_SET_MAX; i++) {
      if (new_events->set_params[i] &&
        (new_events->set_params_valid[i] == TRUE)) {
        ret = isp_trigger_thread_copy_control_event(new_events->set_params[i],
          cur_events, session_param->session_based_ide);
        if (ret == FALSE) {
          ISP_ERR("failed");
        }
        free(new_events->set_params[i]->u.ctrl_event.control_event_data);
        free(new_events->set_params[i]);
      }
    }
    /* Copy module events */
    for (i = 0; i < ISP_MODULE_EVENT_MAX; i++) {
      if (new_events->module_events[i] &&
        (new_events->module_events_valid[i] == TRUE)) {
        ret = isp_trigger_thread_copy_module_event(new_events->module_events[i],
          cur_events, session_param->session_based_ide);
        if (ret == FALSE) {
          ISP_ERR("failed");
        }
        free(new_events->module_events[i]->u.module_event.module_event_data);
        free(new_events->module_events[i]);
      }
    }
    free(new_events);
  } while (evt_frame_id <= frame_id);

  PTHREAD_MUTEX_UNLOCK(&trigger_update_params->mutex);

  MCT_PROF_LOG_END();

  return TRUE;
}

/** isp_trigger_thread_create:
 *
 *  @module: mct module handle
 *  @session_param: session param handle
 *
 *  Create new ISP thread
 *
 *  Returns TRUE on success and FALSE on failure
 **/
boolean isp_trigger_thread_create(mct_module_t *module,
  isp_session_param_t *session_param, isp_resource_t *isp_resource)
{
  int32_t                      rc = 0;
  boolean                      ret = TRUE;
  isp_trigger_update_params_t *trigger_params = NULL;
  isp_trigger_thread_priv_t    thread_priv;

  if (!module || !session_param) {
    ISP_ERR("failed: module %p session_param %p", module, session_param);
    return FALSE;
  }

  trigger_params = &session_param->trigger_update_params;
  /* Create PIPE to communicate with isp thread */
  rc = pipe(trigger_params->pipe_fd);
  if(rc < 0) {
    ISP_ERR("pipe() failed");
    return FALSE;
  }

  thread_priv.module = module;
  thread_priv.session_param = session_param;
  thread_priv.isp_resource = isp_resource;
  PTHREAD_MUTEX_LOCK(&trigger_params->mutex);
  trigger_params->is_thread_alive = FALSE;
  rc = pthread_create(&trigger_params->trigger_update_thread, NULL,
    isp_trigger_thread_func, &thread_priv);
  pthread_setname_np(trigger_params->trigger_update_thread, "CAM_isp_trigger");
  if(rc < 0) {
    ISP_ERR("pthread_create() failed rc= %d", rc);
    ret = FALSE;
    goto ERROR_THREAD;
  }

  while(trigger_params->is_thread_alive == FALSE) {
    pthread_cond_wait(&trigger_params->cond, &trigger_params->mutex);
  }
  PTHREAD_MUTEX_UNLOCK(&trigger_params->mutex);

  return ret;

ERROR_THREAD:
  close(trigger_params->pipe_fd[READ_FD]);
  close(trigger_params->pipe_fd[WRITE_FD]);
  PTHREAD_MUTEX_UNLOCK(&trigger_params->mutex);
  return ret;
}

/** isp_trigger_thread_join:
 *
 *  @session_param: ISP session param
 *
 *  Join ISP thread
 *
 *  Returns: void
 **/
void isp_trigger_thread_join(isp_session_param_t *session_param)
{
  isp_trigger_update_params_t *trigger_params = NULL;
  uint32_t            i = 0;
  isp_saved_events_t *saved_events = NULL;

  if (!session_param) {
    ISP_ERR("failed: session_param %p", session_param);
    return;
  }

  trigger_params = &session_param->trigger_update_params;

  PTHREAD_MUTEX_LOCK(&trigger_params->mutex);
  trigger_params->is_thread_alive = FALSE;
  PTHREAD_MUTEX_UNLOCK(&trigger_params->mutex);

  isp_trigger_thread_post_message(trigger_params,
    ISP_TRIGGER_THREAD_EVENT_ABORT_THREAD, 0, session_param);

  /* Join session thread */
  pthread_join(trigger_params->trigger_update_thread, NULL);

  PTHREAD_MUTEX_LOCK(&trigger_params->mutex);
  /* Free cur events */
  isp_util_free_saved_events(&session_param->trigger_update_params.cur_events);
  /* Free new events */
  isp_util_free_saved_events(&session_param->trigger_update_params.new_events);
  PTHREAD_MUTEX_UNLOCK(&trigger_params->mutex);
  close(trigger_params->pipe_fd[READ_FD]);
  close(trigger_params->pipe_fd[WRITE_FD]);
}
