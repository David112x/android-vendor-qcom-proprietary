/* isp_thread.c
 *
 * Copyright (c) 2012-2015, 2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>

/* mctl headers */
#include "media_controller.h"
#include "mct_list.h"
#include "mct_profiler.h"

/* isp headers */
#include <media/msmb_isp.h>
#include "isp_module.h"
#include "isp_hw_update_thread.h"
#include "isp_hw_update_util.h"
#include "isp_log.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COMMON, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COMMON, fmt, ##args)

/** isp_hw_update_thread_process:
 *
 *  @isp_resource: isp resource handle
 *  @session_param: session param
 *
 *  Handle hw update
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_hw_update_thread_process(isp_resource_t *isp_resource,
  isp_session_param_t *session_param)
{
  boolean                 ret = TRUE;
  isp_hw_id_t             hw_id = 0;
  isp_reg_update_state_t  reg_update_state;

  if (!isp_resource || !session_param) {
    ISP_ERR("failed: %p %p", isp_resource, session_param);
    return FALSE;
  }

  ret = isp_hw_update_util_do_ioctl_in_hw_update_params(isp_resource,
    session_param);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_hw_update_do_ioctl");
  }

  ret = isp_hw_update_util_request_reg_update(session_param);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_request_reg_update");
  }

  return ret;
} /* isp_hw_update_thread_process */

/** isp_hw_update_thread_free_hw_udpate_list:
 *
 *  @hw_update_params: hw update params handle
 *
 *  Free cur_hw_update_list and new_hw_update_list
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_hw_update_thread_free_hw_udpate_list(
  isp_session_param_t *session_param, isp_hw_update_params_t *hw_update_params)
{
  boolean                      ret = TRUE;
  isp_hw_id_t                  hw_index = 0;
  isp_hw_update_list_params_t *hw_update_list_params = NULL;

  if (!hw_update_params) {
    ISP_ERR("failed: hw_update_params %p", hw_update_params);
    return FALSE;
  }

  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    isp_hw_id_t session_hw_id = session_param->hw_id[hw_index];
    if ((session_param->offline_num_isp > 0 &&
      session_hw_id == session_param->offline_hw_id[0]) ||
      session_hw_id >= ISP_HW_MAX)
      continue;
    ret = isp_hw_update_util_free_cur(hw_update_params, session_hw_id);
  }

  hw_update_params->skip_hw_update = FALSE;
  hw_update_params->cds_updating = FALSE;
  /* Clear stats params */
  for (hw_index = 0; hw_index < ISP_HW_MAX; hw_index++) {
    isp_hw_id_t session_hw_id = session_param->hw_id[hw_index];
    if ((session_param->offline_num_isp > 0 &&
      session_hw_id == session_param->offline_hw_id[0]) ||
      session_hw_id >= ISP_HW_MAX)
      continue;
    hw_update_list_params = &hw_update_params->hw_update_list_params[session_hw_id];
    /* Do not free stats params. It is handle by start/stop session */
    memset(hw_update_list_params->stats_params, 0,
      sizeof(isp_saved_stats_params_t));
    memset(hw_update_list_params->applied_stats_params, 0,
      sizeof(isp_saved_stats_params_t));
    hw_update_list_params->cur_cds_update = FALSE;
    hw_update_list_params->new_cds_update = FALSE;
  }
  return ret;
} /* isp_hw_update_thread_free_hw_udpate_list */

/** isp_hw_update_thread_func:
 *
 *  @data: handle to session_param
 *
 *  ISP main thread handler
 *
 *  Returns NULL
 **/
static void *isp_hw_update_thread_func(void *data)
{
  boolean                      ret = TRUE;
  isp_session_param_t         *session_param = NULL;
  struct pollfd                pollfds;
  int32_t                      num_fds = 1, ready = 0, i = 0, read_bytes = 0;
  isp_hw_update_thread_event_t event;
  isp_hw_update_params_t      *hw_update_params = NULL;
  boolean                      exit_thread = FALSE;
  isp_hw_update_thread_data_t *thread_data =
    (isp_hw_update_thread_data_t *)data;
  isp_resource_t              *isp_resource = NULL;

  if (!data) {
    ISP_ERR("failed: data %p", data);
    return NULL;
  }

  if (!thread_data->isp_resource || !thread_data->session_param) {
    ISP_ERR("failed: %p %p", thread_data->isp_resource,
      thread_data->session_param);
    return NULL;
  }

  ISP_HIGH("isp_new_thread hw update thread start");
  session_param = thread_data->session_param;
  isp_resource = thread_data->isp_resource;
  hw_update_params = &session_param->hw_update_params;
  PTHREAD_MUTEX_LOCK(&hw_update_params->mutex);
  hw_update_params->is_thread_alive = TRUE;
  pthread_cond_signal(&hw_update_params->cond);
  PTHREAD_MUTEX_UNLOCK(&hw_update_params->mutex);

  while (exit_thread == FALSE) {
    pollfds.fd = hw_update_params->pipe_fd[READ_FD];
    pollfds.events = POLLIN|POLLPRI;
    ready = poll(&pollfds, (nfds_t)num_fds, -1);
    if (ready > 0) {
      if (pollfds.revents & (POLLIN|POLLPRI)) {
        read_bytes = read(pollfds.fd, &event,
          sizeof(isp_hw_update_thread_event_t));
        if ((read_bytes < 0) ||
            (read_bytes != sizeof(isp_hw_update_thread_event_t))) {
          ISP_ERR("failed: read_bytes %d", read_bytes);
          continue;
        }
        switch (event.type) {
        case ISP_HW_UPDATE_EVENT_PROCESS:
          ATRACE_BEGIN_SNPRINTF(30, "VFE_HW_UPDATE %d", session_param->curr_frame_id);
          MCT_PROF_LOG_BEG("isp_hw_update_thread_process");
          ret = isp_hw_update_thread_process(isp_resource, session_param);
          ATRACE_END();
          MCT_PROF_LOG_END();
          if (ret == FALSE) {
            ISP_ERR("failed: isp_hw_update_process");
          }
          break;
        case ISP_HW_UPDATE_EVENT_FREE_HW_UPDATE_LIST:
          ret = isp_hw_update_thread_free_hw_udpate_list(
            session_param, &session_param->hw_update_params);
          if (ret == FALSE) {
            ISP_ERR("failed: isp_hw_update_free_hw_udpate_list");
          }
          PTHREAD_MUTEX_LOCK(&hw_update_params->mutex);
          pthread_cond_signal(&hw_update_params->cond);
          PTHREAD_MUTEX_UNLOCK(&hw_update_params->mutex);
          break;
        case ISP_HW_UPDATE_EVENT_ABORT_THREAD:
          exit_thread = TRUE;
          break;
        default:
          ISP_ERR("invalid event type %d", event.type);
          break;
        }
        PTHREAD_MUTEX_LOCK(&session_param->thread_busy_mutex);
        hw_update_params->is_thread_busy = FALSE;
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

/** isp_hw_update_thread_post_message:
 *
 *  @hw_update_params: hw udpate params
 *  @type: message type to be posted
 *  @session_param: Session param
 *
 *  Post event to session thread
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_hw_update_thread_post_message(
  isp_hw_update_params_t *hw_update_params, isp_hw_update_event_type_t type,
  isp_session_param_t *session_param)
{
  int32_t                      rc = 0;
  isp_hw_update_thread_event_t message;
  if (!hw_update_params || (type >= ISP_HW_UPDATE_EVENT_MAX) ||
      !session_param) {
       ISP_ERR("failed: %p type %d %p", hw_update_params, type, session_param);
        return FALSE;
  }

  memset(&message, 0, sizeof(message));
  message.type = type;
  PTHREAD_MUTEX_LOCK(&session_param->thread_busy_mutex);
  hw_update_params->is_thread_busy = TRUE;
  PTHREAD_MUTEX_UNLOCK(&session_param->thread_busy_mutex);
  if (hw_update_params->pipe_fd[WRITE_FD]) {
    rc = write(hw_update_params->pipe_fd[WRITE_FD], &message, sizeof(message));
    if(rc < 0) {
      ISP_ERR("failed: rc %d", rc);
      return FALSE;
    }
  }

  return TRUE;
} /* isp_hw_update_thread_post_message */

/** isp_hw_update_thread_handle_sof:
 *
 *  @session_param: session param handle
 *
 *  Append new_hw_udpate_list in cur_hw_update_list and
 *  schedule hw update thread to perform ioctl
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_hw_update_thread_handle_sof(isp_session_param_t *session_param)
{
  boolean                      ret = TRUE;
  isp_hw_update_params_t      *hw_update_params = NULL;

  if (!session_param) {
    ISP_ERR("failed: session_param %p", session_param);
    return FALSE;
  }

  MCT_PROF_LOG_BEG(__func__);
  hw_update_params = &session_param->hw_update_params;

  PTHREAD_MUTEX_LOCK(&session_param->state_mutex);
  session_param->reg_update_info.reg_update_state =
    ISP_REG_UPDATE_STATE_PENDING;
  PTHREAD_MUTEX_UNLOCK(&session_param->state_mutex);

  ret = isp_hw_update_thread_post_message(hw_update_params,
    ISP_HW_UPDATE_EVENT_PROCESS, session_param);
  if (ret == FALSE) {
    ISP_ERR("failed: ISP_HW_UPDATE_EVENT_PROCESS");
  }
  MCT_PROF_LOG_END();

  return ret;
} /* isp_hw_update_thread_handle_sof */

/** isp_hw_update_thread_create:
 *
 *  @isp_resource: isp resource handle
 *  @session_param: isp session param handle
 *
 *  Create new ISP thread
 *
 *  Returns TRUE on success and FALSE on failure
 **/
boolean isp_hw_update_thread_create(isp_resource_t *isp_resource,
  isp_session_param_t *session_param)
{
  int32_t                      rc = 0;
  boolean                      ret = TRUE;
  isp_hw_update_params_t      *hw_update_params = NULL;
  isp_hw_update_thread_data_t  thread_data;

  if (!isp_resource || !session_param) {
    ISP_ERR("failed: isp_resource %p session_param %p", isp_resource,
      session_param);
    return FALSE;
  }

  hw_update_params = &session_param->hw_update_params;
  /* Create PIPE to communicate with isp thread */
  rc = pipe(hw_update_params->pipe_fd);
  if(rc < 0) {
    ISP_ERR("pipe() failed");
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&hw_update_params->mutex);
  thread_data.isp_resource = isp_resource;
  thread_data.session_param = session_param;
  hw_update_params->is_thread_alive = FALSE;
  rc = pthread_create(&hw_update_params->hw_update_thread, NULL,
    isp_hw_update_thread_func, &thread_data);
  if(rc < 0) {
    ISP_ERR("pthread_create() failed rc= %d", rc);
    ret = FALSE;
    goto ERROR_THREAD;
  }
  pthread_setname_np(hw_update_params->hw_update_thread, "CAM_hw_update");

  while(hw_update_params->is_thread_alive == FALSE) {
    pthread_cond_wait(&hw_update_params->cond, &hw_update_params->mutex);
  }
  PTHREAD_MUTEX_UNLOCK(&hw_update_params->mutex);

  return ret;

ERROR_THREAD:
  close(hw_update_params->pipe_fd[READ_FD]);
  close(hw_update_params->pipe_fd[WRITE_FD]);
  PTHREAD_MUTEX_UNLOCK(&hw_update_params->mutex);
  return ret;
}

/** isp_hw_update_thread_join:
 *
 *  @session_param: ISP session param
 *
 *  Join ISP thread
 *
 *  Returns: void
 **/
void isp_hw_update_thread_join(isp_session_param_t *session_param)
{
  boolean                 ret = TRUE;
  isp_hw_update_params_t *hw_update_params = NULL;

  if (!session_param) {
    ISP_ERR("failed: session_param %p", session_param);
    return;
  }

  hw_update_params = &session_param->hw_update_params;

  PTHREAD_MUTEX_LOCK(&hw_update_params->mutex);
  hw_update_params->is_thread_alive = FALSE;
  PTHREAD_MUTEX_UNLOCK(&hw_update_params->mutex);

  isp_hw_update_thread_post_message(hw_update_params,
    ISP_HW_UPDATE_EVENT_ABORT_THREAD, session_param);

  /* Join session thread */
  pthread_join(hw_update_params->hw_update_thread, NULL);

  ret = isp_hw_update_thread_free_hw_udpate_list(session_param,
    hw_update_params);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_hw_update_free_hw_udpate_list");
  }

  close(hw_update_params->pipe_fd[READ_FD]);
  hw_update_params->pipe_fd[READ_FD] = 0;
  close(hw_update_params->pipe_fd[WRITE_FD]);
  hw_update_params->pipe_fd[WRITE_FD] = 0;
}
