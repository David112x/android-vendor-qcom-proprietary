/* isp_sub_module_util.c
 *
 * Copyright (c) 2012-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <string.h>
/* mctl headers */
#include "mct_module.h"
#include "chromatix.h"
#include "eztune_vfe_diagnostics.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module_util.h"
#include "isp_log.h"
#include "isp_defs.h"
#include "isp_adrc_tune_def.h"
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

/** isp_sub_module_util_free_hw_update_list
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Free hw update list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_free_hw_update_list(
  isp_sub_module_t *isp_sub_module)
{
  struct msm_vfe_cfg_cmd_list *head = NULL,
                              *next = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  head = isp_sub_module->hw_update_list;
  while (head) {
    next = head->next;
    free(head->cfg_cmd.cfg_data);
    head->cfg_cmd.cfg_data = NULL;
    free(head->cfg_cmd.cfg_cmd);
    head->cfg_cmd.cfg_cmd = NULL;
    free(head);
    head = next;
  }
  return TRUE;
}

/** isp_sub_module_util_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @hw_update: hw update to be store
 *
 *  Store hw update in module private
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_store_hw_update(
  isp_sub_module_t *isp_sub_module,
  struct msm_vfe_cfg_cmd_list *hw_update)
{
  struct msm_vfe_cfg_cmd_list *list = NULL;

  if (!isp_sub_module || !hw_update) {
    ISP_ERR("failed: %p %p", isp_sub_module, hw_update);
    return FALSE;
  }

  hw_update->next = NULL;
  list = isp_sub_module->hw_update_list;
  if (!list) {
    isp_sub_module->hw_update_list = hw_update;
  } else {
    while (list->next) {
      list = list->next;
    }
    list->next = hw_update;
    list->next_size = sizeof(*hw_update);
  }

  return TRUE;
}

/** isp_sub_module_util_append_hw_update_list
 *
 *  @isp_sub_module: isp sub module handle
 *  @output: isp sub module output params
 *
 *  Append sub module's hw update in hw update list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_append_hw_update_list(
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *output)
{
  struct msm_vfe_cfg_cmd_list *list = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;

  if (!isp_sub_module || !output) {
    ISP_ERR("failed: %p %p", isp_sub_module, output);
    return FALSE;
  }

  hw_update = isp_sub_module->hw_update_list;
  if (!hw_update) {
    return TRUE;
  }

  ISP_LOG_LIST("************** APPEND ENTER ******************");
  list = output->hw_update_list;
  ISP_LOG_LIST("list %p", list);
  if (!list) {
    output->hw_update_list = hw_update;
  } else {
    ISP_LOG_LIST("list->next %p", list->next);
    while (list->next) {
      list = list->next;
      ISP_LOG_LIST("list->next %p", list->next);
    }
    list->next = hw_update;
    list->next_size = sizeof(*hw_update);
    ISP_LOG_LIST("list->next->next %p", list->next->next);
  }
  isp_sub_module->hw_update_list = NULL;
  ISP_LOG_LIST("************** APPEND EXIT ******************");

  ISP_LOG_LIST("************ PRINT ENTER *******************");
  list = output->hw_update_list;
  ISP_LOG_LIST("list %p cfg cmd %p", list, list->cfg_cmd.cfg_cmd);
  while (list->next) {
    ISP_LOG_LIST("list %p cfg cmd %p", list->next, list->next->cfg_cmd.cfg_cmd);
    list = list->next;
  }
  ISP_LOG_LIST("************ PRINT EXIT *******************");
  return TRUE;
}

/** isp_sub_module_util_compare_identity:
 *
 *  @data1: identity1
 *  @data2: identity2
 *
 *  Return TRUE if identity matches, FALSE otherwise
 **/
boolean isp_sub_module_util_compare_identity(void *data1, void *data2)
{
  uint32_t *identity1 = (uint32_t *)data1;
  uint32_t *identity2 = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }

  if (*identity1 == *identity2) {
    return TRUE;
  }

  return FALSE;
}

/** isp_sub_module_util_find_port_based_on_identity:
 *
 *  @data1: mct port handle
 *  @data2: identity
 *
 *  Retrieve port based on identity
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_sub_module_util_find_port_based_on_identity(void *data1,
  void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  uint32_t   *identity = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }

  if (mct_list_find_custom(MCT_PORT_CHILDREN(port), identity,
    isp_sub_module_util_compare_identity)) {
    return TRUE;
  }

  return FALSE;
}

/** isp_sub_module_util_get_port_from_module:
 *
 *  @module: mct module handle
 *  @port: port to return
 *  @direction: direction of port to be retrieved
 *  @identity: identity to match against
 *
 *  Find port based on direction and identity and return it
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_get_port_from_module(mct_module_t *module,
  mct_port_t **port, mct_port_direction_t direction, uint32_t identity)
{
  mct_list_t *l_port = NULL;

  if (!module || !port) {
    ISP_ERR("failed: module %p sink_port %p", module, port);
    return FALSE;
  }

  /* Reset output param */
  *port = NULL;

  if ((direction != MCT_PORT_SRC) && (direction != MCT_PORT_SINK)) {
    ISP_ERR("failed: invalid direction %d", direction);
    return FALSE;
  }

  if (direction == MCT_PORT_SINK) {
    l_port = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &identity,
      isp_sub_module_util_find_port_based_on_identity);
    if (!l_port) {
      ISP_ERR("failed: mct_list_find_custom for ide %x", identity);
      return FALSE;
    }
  } else {
    l_port = mct_list_find_custom(MCT_MODULE_SRCPORTS(module), &identity,
      isp_sub_module_util_find_port_based_on_identity);
    if (!l_port) {
      ISP_ERR("failed: mct_list_find_custom for ide %x", identity);
      return FALSE;
    }
  }

  *port = (mct_port_t *)l_port->data;

  return TRUE;
}

/** isp_sub_module_util_compare_identity_from_port:
 *
 *  @data1: stream info handle
 *  @data2: identity
 *
 *  Returns TRUE if identity of port's children has identity
 *  passed in 2nd param
 **/
boolean isp_sub_module_util_compare_identity_from_port(void *data1, void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  uint32_t   *identity = (uint32_t *)data2;
  mct_list_t *l_identity = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }

  l_identity = mct_list_find_custom(MCT_PORT_CHILDREN(port),
    identity, isp_sub_module_util_compare_identity);
  if (l_identity) {
    return TRUE;
  }

  return FALSE;
}

/** isp_sub_module_util_compare_identity_from_stream_param:
 *
 *  @data1: stream info handle
 *  @data2: identity
 *
 *  Returns TRUE if identity of stream info matches with
 *  requested identity, FALSE otherwise
 **/
static boolean isp_sub_module_util_compare_identity_from_stream_info(
  void *data1, void *data2)
{
  mct_stream_info_t *stream_info = (mct_stream_info_t *)data1;
  uint32_t          *identity = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }

  if (stream_info->identity == *identity) {
    return TRUE;
  }

  return FALSE;
}

/** isp_sub_module_util_append_stream_info:
 *
 *  @session_param: session param
 *  @stream_info: stream info to be appended
 *
 *  Append stream info handle in session param
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_append_stream_info(
  isp_sub_module_t *session_param, mct_stream_info_t *stream_info)
{
  boolean     ret = TRUE;
  mct_list_t *l_stream_info = NULL;

  if (!session_param || !stream_info) {
    ISP_ERR("failed: %p %p", session_param, stream_info);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&session_param->mutex);
  /* Check whether stream info is already appended for same stream */
  l_stream_info = mct_list_find_custom(session_param->l_stream_info,
    &stream_info->identity,
    isp_sub_module_util_compare_identity_from_stream_info);
  if (l_stream_info) {
    PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
    return TRUE;
  }

  /* Append stream info in module private */
  l_stream_info = mct_list_append(session_param->l_stream_info,
    stream_info, NULL, NULL);
  if (!l_stream_info) {
    ISP_ERR("failed: l_stream_info %p", l_stream_info);
    goto ERROR;
  }
  session_param->l_stream_info = l_stream_info;

  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return FALSE;
}

/** isp_sub_module_util_remove_stream_info:
 *
 *  @session_param: session param
 *  @identity: identity of stream info to be appended
 *
 *  Remove stream info handle in module private's stream info
 *  list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_remove_stream_info(
  isp_sub_module_t *session_param, uint32_t identity)
{
  boolean     ret = TRUE;
  mct_list_t *l_stream_info = NULL;

  if (!session_param) {
    ISP_ERR("failed: %p", session_param);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&session_param->mutex);
  /* Find stream info in module private */
  l_stream_info = mct_list_find_custom(session_param->l_stream_info,
    &identity, isp_sub_module_util_compare_identity_from_stream_info);
  if (!l_stream_info) {
    PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
    return TRUE;
  }

  session_param->l_stream_info = mct_list_remove(session_param->l_stream_info,
    l_stream_info->data);

  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return FALSE;
}

/** isp_sub_module_util_get_stream_info:
 *
 *  @session_param: session params
 *  @stream_info: handle to return stream info
 *
 *  Extract stream info from session param and return it
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_get_stream_info(isp_sub_module_t *session_param,
  uint32_t identity, mct_stream_info_t **stream_info)
{
  mct_list_t *l_stream_info = NULL;

  if (!session_param || !stream_info) {
    ISP_ERR("failed: %p %p", session_param, stream_info);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&session_param->mutex);
  /* Find stream info in module private */
  l_stream_info = mct_list_find_custom(session_param->l_stream_info,
    &identity, isp_sub_module_util_compare_identity_from_stream_info);
  if (!l_stream_info) {
    ISP_ERR("failed: l_stream_info %p identity %x", l_stream_info, identity);
    goto ERROR;
  }

  *stream_info = (mct_stream_info_t *)l_stream_info->data;

  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return FALSE;
}

/** isp_sub_module_util_compare_identity_in_session_param:
 *
 *  @data1: handle to isp_sub_module_t
 *  @data2: handle to session id
 *
 *  Return TRUE if session id in session param matches with
 *  requested session id
 **/
static boolean isp_sub_module_util_compare_identity_in_session_param(
  void *data1, void *data2)
{
  isp_sub_module_t *session_param = (isp_sub_module_t *)data1;
  unsigned int             *session_id = (unsigned int *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }

  if (session_param->session_id == *session_id) {
    return TRUE;
  }

  return FALSE;
}


/** isp_sub_module_util_get_session_param:
 *
 *  @isp_sub_module: handle to isp_sub_module_priv_t
 *  @session_id: session id for which session param need to
 *             be retrieved
 *  @session_param: handle for session param to be retrived
 *
 *  Retrieve session param from module private
 *
 *  Return nonez
 **/
void isp_sub_module_util_get_session_param(
  isp_sub_module_priv_t *isp_sub_module_priv, unsigned int session_id,
  isp_sub_module_t **session_param)
{
  mct_list_t *l_session_param = NULL;

  if (!isp_sub_module_priv || !session_param) {
    ISP_ERR("failed: %p %p", isp_sub_module_priv, session_param);
    return;
  }

  *session_param = NULL;

  if (!isp_sub_module_priv->l_session_params) {
    return;
  }

  l_session_param = mct_list_find_custom(isp_sub_module_priv->l_session_params,
    &session_id, isp_sub_module_util_compare_identity_in_session_param);
  if (!l_session_param) {
    return;
  }

  *session_param = (isp_sub_module_t *)l_session_param->data;
  return;
}

/** isp_sub_module_util_create_session_param:
 *
 *  @isp_sub_module: handle to isp_sub_module_priv_t
 *  @session id: session id
 *  @isp_sub_module: handle to return isp_sub_module module
 *
 *  Check whether session param already exist for this
 *  session id. If not, create session param and append it to
 *  the list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_create_session_param(
  isp_sub_module_priv_t *isp_sub_module_priv, unsigned int session_id,
  isp_sub_module_t **isp_sub_module)
{
  mct_list_t   *l_session_param = NULL;
  isp_sub_module_t *session_param = NULL;

  if (!isp_sub_module_priv || !isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p %p", isp_sub_module_priv,
      isp_sub_module);
    return FALSE;
  }

  /* Find whether session_param list already has this session id */
  if (isp_sub_module_priv->l_session_params) {
    l_session_param = mct_list_find_custom(
      isp_sub_module_priv->l_session_params, &session_id,
      isp_sub_module_util_compare_identity_in_session_param);
    if (l_session_param) {
      ISP_ERR("failed: session id %d already present", session_id);
      return FALSE;
    }
  }

  /* Create new session param instance */
  session_param =
    (isp_sub_module_t *)CAM_MALLOC(sizeof(isp_sub_module_t));
  if (!session_param) {
    ISP_ERR("failed: session_param %p", session_param);
    return FALSE;
  }

  memset(session_param, 0, sizeof(*session_param));
  session_param->session_id = session_id;
  session_param->hw_module_id = isp_sub_module_priv->hw_module_id;
  pthread_mutex_init(&session_param->mutex, NULL);
  pthread_mutex_init(&session_param->new_mutex, NULL);

  /* Append it in session param list */
  l_session_param = mct_list_append(isp_sub_module_priv->l_session_params,
    session_param, NULL, NULL);
  if (!l_session_param) {
    ISP_ERR("failed: l_session_param %p", l_session_param);
    goto ERROR;
  }
  isp_sub_module_priv->l_session_params = l_session_param;
  *isp_sub_module = session_param;
  return TRUE;

ERROR:
  free(session_param);
  return FALSE;
}

/** isp_sub_module_util_remove_session_param:
 *
 *  @isp_sub_module: handle to isp_sub_module_priv_t
 *  @session id: session id
 *
 *  Remove session param from module private
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_remove_session_param(
  isp_sub_module_priv_t *isp_sub_module_priv, unsigned int session_id)
{
  mct_list_t               *l_session_param = NULL;
  isp_sub_module_t *session_param = NULL;

  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module_priv);
    return FALSE;
  }

  if (!isp_sub_module_priv->l_session_params) {
    ISP_ERR("failed: isp_sub_module->l_session_params %p",
      isp_sub_module_priv->l_session_params);
    return FALSE;
  }

  l_session_param = mct_list_find_custom(isp_sub_module_priv->l_session_params,
    &session_id, isp_sub_module_util_compare_identity_in_session_param);
  if (!l_session_param) {
    ISP_ERR("failed: l_session_param %p", l_session_param);
    return FALSE;
  }

  if (!l_session_param->data) {
    ISP_ERR("failed: l_session_param->data %p", l_session_param->data);
    return FALSE;
  }

  session_param = (isp_sub_module_t *)l_session_param->data;
  pthread_mutex_destroy(&session_param->mutex);
  pthread_mutex_destroy(&session_param->new_mutex);
  isp_sub_module_priv->l_session_params = mct_list_remove(
    isp_sub_module_priv->l_session_params, l_session_param->data);

  free(session_param);
  return TRUE;
}

/** isp_sub_module_traverse_ports:
 *
 *  @data: handle to mct port
 *  @user_data: handle to event
 *
 *  Forward event if this port is assigned to event's
 *  identity
 *
 *  Return TRUE
 **/
boolean isp_sub_module_traverse_ports(void *data, void *user_data)
{
  boolean      ret = TRUE;
  mct_port_t  *port = (mct_port_t *)data;
  mct_event_t *event = (mct_event_t *)user_data;

  if (!data || !user_data) {
    ISP_ERR("failed %p %p", data, user_data);
    /* Return TRUE since this is part of mct_list_traverse */
    return TRUE;
  }

  ret = isp_sub_module_util_compare_identity_from_port(port, &event->identity);
  if (ret == TRUE) {
    /* Forward event to this port */
    ret = mct_port_send_event_to_peer(port, event);
    if (ret == FALSE) {
      if (event->type == MCT_EVENT_CONTROL_CMD) {
        ISP_ERR("failed: mct_port_send_event_to_peer mod event %d",
          event->u.ctrl_event.type);
      } else {
        ISP_ERR("failed: mct_port_send_event_to_peer ctrl event %d",
          event->u.module_event.type);
      }
    }
  }

  /* Return TRUE since this is part of mct_list_traverse */
  return TRUE;
}

/** isp_sub_module_port_forward_event:
 *
 *  @port: port handle
 *  @event: event to be forwarded
 *
 *  Forward event based on port direction and event direction
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_forward_event_from_port(mct_port_t *port,
  mct_event_t *event)
{
  boolean                 ret = TRUE;
  mct_module_t           *module = NULL;

  if (!port || !event) {
    ISP_ERR("failed: port %p event %p", port, event);
    return FALSE;
  }

  if (((event->direction == MCT_EVENT_UPSTREAM) &&
       (port->direction != MCT_PORT_SRC)) ||
      ((event->direction == MCT_EVENT_DOWNSTREAM) &&
       (port->direction != MCT_PORT_SINK))) {
    ISP_ERR("failed: invalid types, event dir %d port dir %d",
      event->direction, port->direction);
    return FALSE;
  }

  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  ret = isp_sub_module_util_forward_event_from_module(module, event);
  if (ret == FALSE) {
    if (event->type == MCT_EVENT_CONTROL_CMD) {
      ISP_DBG("failed: mct_port_send_event_to_peer mod event %d",
        event->u.ctrl_event.type);
    } else {
      ISP_DBG("failed: mct_port_send_event_to_peer ctrl event %d",
        event->u.module_event.type);
    }
  }

  return ret;
}

/** isp_sub_module_util_forward_event_from_module:
 *
 *  @module: module handle
 *  @event: event to be forwarded
 *
 *  Forward event based on port direction and event direction
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_forward_event_from_module(mct_module_t *module,
  mct_event_t *event)
{
  boolean                 ret = TRUE;
  isp_hw_streamid_t       hw_stream_id = 0;
  mct_list_t             *l_identity = NULL;
  mct_port_t             *forward_port = NULL;
  mct_list_t             *l_port = NULL;
  isp_sub_module_store_port_t   *store_port_data = NULL;
  uint32_t                       i = 0, j = 0;
  isp_sub_module_store_src_port_t  *store_source_port = NULL;
  isp_sub_module_priv_t        *isp_sub_module_priv = NULL;

  if (!module || !event) {
    ISP_ERR("failed: module %p event %p", module, event);
    return FALSE;
  }

  /* Extract module private */
  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module_priv);
    return FALSE;
  }

  /* Fill port info in module private */
  store_port_data = &isp_sub_module_priv->port_data;

  if (event->direction == MCT_EVENT_UPSTREAM) {
    if (!store_port_data->sink_port) {
      ISP_ERR("<port_failure> sink_port %p", store_port_data->sink_port);
      return FALSE;
    }
    forward_port = store_port_data->sink_port;

    ret = mct_port_send_event_to_peer(forward_port, event);
    if (ret == FALSE) {
      if (event->type == MCT_EVENT_CONTROL_CMD) {
        ISP_ERR("failed: mod %s ctrl event type %d",
          MCT_MODULE_NAME(module), event->u.ctrl_event.type);
      } else if (event->type == MCT_EVENT_MODULE_EVENT) {
        ISP_ERR("failed: mod %s module event type %d",
          MCT_MODULE_NAME(module), event->u.module_event.type);
      }
    }
  } else if (event->direction == MCT_EVENT_DOWNSTREAM) {
    for (i = 0; i < MAX_SRC_PORTS; i++) {
      store_source_port = &store_port_data->source_port_data[i];
      if (store_source_port->source_port) {
        for (j = 0; j < MAX_STREAMS; j++) {
          forward_port = NULL;
          if (store_source_port->identity[j] == event->identity) {
            forward_port = store_source_port->source_port;
          }

          if (forward_port) {
            ret = mct_port_send_event_to_peer(forward_port, event);
            if (ret == FALSE) {
              if (event->type == MCT_EVENT_CONTROL_CMD) {
                ISP_DBG("failed: mod %s ctrl event type %d",
                  MCT_MODULE_NAME(module), event->u.ctrl_event.type);
              } else if (event->type == MCT_EVENT_MODULE_EVENT) {
                ISP_DBG("failed: mod %s module event type %d",
                  MCT_MODULE_NAME(module), event->u.module_event.type);
              }
            }
          }
        }
      }
    }
  }

  return ret;
}

/** isp_sub_module_util_update_streaming_mode:
 *
 *  @data1: mct stream info handle
 *  @data2: streaming mode mask
 *
 *  Append streaming mode of stream info in streaming mode
 *  mask
 *
 *  Return TRUE
 **/
boolean isp_sub_module_util_update_streaming_mode(void *data1, void *data2)
{
  mct_stream_info_t *stream_info = (mct_stream_info_t *)data1;
  uint32_t          *streaming_mode_mask = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return TRUE;
  }
  /* For session based stream skip streaming mode update*/
  if (stream_info->stream_type == CAM_STREAM_TYPE_PARM)
    return TRUE;

  ISP_APPEND_STREAMING_MODE(*streaming_mode_mask, stream_info->streaming_mode);
  return TRUE;
}

/** isp_sub_module_fmt_to_pix_pattern:
 *
 *  @fmt: sensor format
 *
 *  Map sensor output format to ISP bayer/yuv pix pattern
 *
 *  Returns pix pattern associated with input bayer packing
 **/
enum ISP_START_PIXEL_PATTERN isp_sub_module_fmt_to_pix_pattern(
  cam_format_t fmt)
{
  ISP_HIGH("%s: format %d", __func__, fmt);

  switch (fmt) {
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB:
  /* Grey formats need to be set as any one of the below types
   * because enum maps to CORE_CFG register, PIX_PATTERN fields.
   * So we cannot add new enum value for Grey. Arbitarily pick
   * RGRG */
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB:
    return ISP_BAYER_RGRGRG;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG:
    return ISP_BAYER_GRGRGR;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR:
    return ISP_BAYER_BGBGBG;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG:
    return ISP_BAYER_GBGBGB;

  case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
  case CAM_FORMAT_YUV_422_NV16:
    return ISP_YUV_YCbYCr;

  case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
  case CAM_FORMAT_YUV_422_NV61:
    return ISP_YUV_YCrYCb;

  case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
    return ISP_YUV_CbYCrY;

  case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
    return ISP_YUV_CrYCbY;

  default:
    CDBG_ERROR("%s: invalid fmt=%d", __func__, fmt);
    return ISP_PIX_PATTERN_MAX;
  }
  return ISP_PIX_PATTERN_MAX;
}

/** isp_sub_module_is_bayer_mono_fmt:
 *
 *  @fmt: sensor format
 *
 *  Checks if sensor is of type Bayer or Mono based on sensor format
 *
 *  Returns TRUE if Bayer/Mono sensor format, FALSE if other sensor like YUV
 **/
boolean isp_sub_module_is_bayer_mono_fmt(cam_format_t sensor_fmt)
{
  switch (sensor_fmt) {
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB:
  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR:
  case CAM_FORMAT_Y_ONLY:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB:
  case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG:
    ISP_DBG("%s: bayer format %d", __func__, sensor_fmt);
    return TRUE;
  default:
    ISP_DBG("%s: non bayer fmt=%d", __func__, sensor_fmt);
    break;
  }

  return FALSE;
}

/** isp_sub_util_calc_interpolation_weight
 * @value: input value
 * @start: start value (min)
 * @end: end value (max)
 *
 * This function finds how far input avlue is as compared to start and end
 * points. If value is below start point it returns zero, if its beyond end
 * outout is one. Otherwise it calulates % of inputvalue against start-end range
 **/
float isp_sub_module_util_calc_interpolation_weight(float value,
  float start, float end)
{
  /* return value is a ratio to the start point,
    "start" point is always the smaller gain/lux one.
    thus,
    "start" could be lowlight trigger start, and bright light trigger end*/
  if (start != end) {
    if (value  <= start)
      return 0.0;
    else if (value  >= end)
      return 1.0;
    else
      return(value  - start) / (end - start);
  } else {
    ISP_DBG("Trigger Warning: same value %f", start);
    return 0.0;
  }
}

/** isp_sub_module_util_get_aec_ratio_lowlight
 *
 *    @tunning_type:
 *    @trigger_ptr:
 *    @aec_out:
 *
 *   Get trigger ratio based on lowlight trigger.
 *   Please note that ratio is the weight of the normal light.
 *    LOW           Mix            Normal
 *    ----------|----------------|----------
 *        low_end(ex: 400)    low_start(ex: 300)
 *
 **/
float isp_sub_module_util_get_aec_ratio_lowlight(unsigned char tunning_type,
  void *trigger_ptr, aec_update_t* aec_out, boolean use_sensor_gain)
{
  float normal_light_ratio = 0.0, gain;
  float ratio_to_low_start = 0.0;
  tuning_control_type tunning = (tuning_control_type)tunning_type;
  trigger_point_type *trigger = (trigger_point_type *)trigger_ptr;

  switch (tunning) {
  /* 0 is Lux Index based */
  case 0: {
    ratio_to_low_start =isp_sub_module_util_calc_interpolation_weight(
      aec_out->lux_idx, trigger->lux_index_start, trigger->lux_index_end);
    }
      break;
  /* 1 is Gain Based */
  case 1: {
    if (use_sensor_gain) {
      gain = aec_out->sensor_gain;
    } else {
      gain = aec_out->real_gain;
    }
    ratio_to_low_start = isp_sub_module_util_calc_interpolation_weight(
      gain, trigger->gain_start, trigger->gain_end);
  }
    break;

  default: {
    ISP_ERR("get_trigger_ratio: tunning type %d is not supported",
      tunning);
  }
    break;
  }

  /*ratio_to_low_start is the sitance to low start,
    the smaller distance to low start,
    the higher ratio applied normal light*/
  normal_light_ratio = 1 - ratio_to_low_start;

  if (normal_light_ratio < 0) {
    normal_light_ratio = 0;
  } else if (normal_light_ratio > 1.0) {
    normal_light_ratio = 1.0;
  }

  return normal_light_ratio;
}

/** isp_sub_module_util_get_aec_ratio_bright
 *
 *    @tunning_type:
 *    @trigger_ptr:
 *    @aec_out:
 *    @is_snap_mode
 *
 *   Get trigger ratio based on lowlight trigger.
 *   Please note that ratio is the weight of the normal light.
 *    NORMAL          Mix              BRIGHT(OUTDOOR)
 *    ------------|-----------------|-----------------
 *        bright_start(ex: 150)    bright_end(ex: 100)
 *
 **/
float isp_sub_module_util_get_aec_ratio_bright(unsigned char tunning_type,
  void *trigger_ptr, aec_update_t* aec_out)
{
  float normal_light_ratio = 0.0, real_gain;
  float ratio_to_birhgt_end = 0.0;
  tuning_control_type tunning = (tuning_control_type)tunning_type;
  trigger_point_type *trigger = (trigger_point_type *)trigger_ptr;

  if (!aec_out || !trigger_ptr) {
    ISP_ERR("aec_out = %p, trigger_ptr = %p, return default ratio\n",
      aec_out, trigger_ptr);
    return 1.0;
  }

  switch (tunning) {
  /* 0 is Lux Index based */
  case 0: {
    ratio_to_birhgt_end = isp_sub_module_util_calc_interpolation_weight(
      aec_out->lux_idx, trigger->lux_index_end, trigger->lux_index_start);
    }
      break;
  /* 1 is Gain Based */
  case 1: {
    real_gain = aec_out->real_gain;
    ratio_to_birhgt_end = isp_sub_module_util_calc_interpolation_weight(
      real_gain, trigger->gain_end, trigger->gain_start);
  }
    break;

  default: {
    ISP_ERR("get_trigger_ratio: tunning type %d is not supported",
      tunning);
  }
    break;
  }

  /*ratio_to_birhgt_end is the sitance to bright_end,
    the smaller distance to bright_end,
    the lower ratio applied on normal light*/
  normal_light_ratio = ratio_to_birhgt_end;

  if (normal_light_ratio < 0) {
    normal_light_ratio = 0;
  } else if (normal_light_ratio > 1.0) {
    normal_light_ratio = 1.0;
  }

  return normal_light_ratio;
}

/** isp40_util_get_aec_ratio_bright_low
 *
 *    @tuning_type:
 *    @outdoor_trigger_ptr:
 *    @lowlight_trigger_ptr:
 *    @aec_out:
 *    @is_snap_mode:
 *    @rt:
 *
 *   Get trigger ratio based on outdoor trigger & lowlight
 *   trigger Please note that rt.ratio means the weight of the normal light.
 *
 *   LOW      MIX      NORMAL   LIGHT             MIX            BRIGHT
 *   ------|---------|---------------------------|-------------|-------------
 *       low_end   low_start                bright_start    bright_end
 *
 **/
boolean isp_sub_module_util_get_aec_ratio_bright_low(unsigned char tuning_type,
  void *outdoor_trigger_ptr, void *lowlight_trigger_ptr, aec_update_t* aec_out,
  trigger_ratio_t *rt, boolean use_sensor_gain)
{
  boolean ret = TRUE;
  float gain;
  tuning_control_type tuning = (tuning_control_type)tuning_type;
  trigger_point_type *outdoor_trigger =
    (trigger_point_type *)outdoor_trigger_ptr;
  trigger_point_type *lowlight_trigger =
    (trigger_point_type *)lowlight_trigger_ptr;

  rt->ratio = 0.0;
  rt->lighting = TRIGGER_NORMAL;

  ISP_DBG("lux_idx %f, real_gain %f, sensor_gain %f",
    aec_out->lux_idx, aec_out->real_gain, aec_out->sensor_gain);

  /* 0 is Lux Index based, 1 is gain base */
  switch (tuning) {
  case 0: {
    if (aec_out->lux_idx < outdoor_trigger->lux_index_start) {
      rt->ratio = isp_sub_module_util_calc_interpolation_weight(
        aec_out->lux_idx, outdoor_trigger->lux_index_end,
        outdoor_trigger->lux_index_start);
      rt->lighting = TRIGGER_OUTDOOR;
    } else if (aec_out->lux_idx > lowlight_trigger->lux_index_start) {
      rt->ratio = 1.0 - isp_sub_module_util_calc_interpolation_weight(
        aec_out->lux_idx, lowlight_trigger->lux_index_start,
        lowlight_trigger->lux_index_end);
      rt->lighting = TRIGGER_LOWLIGHT;
    } else {
      rt->ratio = 1.0;
      rt->lighting = TRIGGER_NORMAL;
    }
  }
    break;

  case 1: {
    if (use_sensor_gain) {
      gain = aec_out->sensor_gain;
    } else {
      gain = aec_out->real_gain;
    }

    if (gain < outdoor_trigger->gain_start) {
      rt->ratio = isp_sub_module_util_calc_interpolation_weight(gain,
        outdoor_trigger->gain_end, outdoor_trigger->gain_start);
      rt->lighting = TRIGGER_OUTDOOR;
    } else if (gain > lowlight_trigger->gain_start) {
      rt->ratio = 1.0 - isp_sub_module_util_calc_interpolation_weight(gain,
        lowlight_trigger->gain_start, lowlight_trigger->gain_end);
      rt->lighting = TRIGGER_LOWLIGHT;
    } else {
      rt->ratio = 1.0;
      rt->lighting = TRIGGER_NORMAL;
    }
  }
    break;

  default: {
    ISP_ERR("get_trigger_ratio: tunning type %d is not supported",
      tuning);
  }
    break;
  }

  return ret;
}

/** isp_sub_module_util_get_awb_cct_type
 *
 *    @trigger:
 *    @chromatix_ptr:
 *
 **/
awb_cct_type isp_sub_module_util_get_awb_cct_type(
  cct_trigger_info* trigger, void *chromatix_ptr)
{
  chromatix_parms_type *p_chromatix = chromatix_ptr;
  awb_cct_type cct_type = AWB_CCT_TYPE_TL84;

  ISP_DBG("%s: CCT %f D65 %f %f A %f %f", __func__,
    trigger->mired_color_temp,
    trigger->trigger_d65.mired_end,
    trigger->trigger_d65.mired_start,
    trigger->trigger_A.mired_start,
    trigger->trigger_A.mired_end);

  if (trigger->mired_color_temp <= trigger->trigger_d65.mired_end) {
    cct_type = AWB_CCT_TYPE_D65;
  } else if ((trigger->mired_color_temp > trigger->trigger_d65.mired_end) &&
             (trigger->mired_color_temp <= trigger->trigger_d65.mired_start)) {
    cct_type = AWB_CCT_TYPE_D65_TL84;
  } else if ((trigger->mired_color_temp >= trigger->trigger_A.mired_start) &&
             (trigger->mired_color_temp < trigger->trigger_A.mired_end)) {
    cct_type = AWB_CCT_TYPE_TL84_A;
  } else if (trigger->mired_color_temp >= trigger->trigger_A.mired_end) {
    cct_type = AWB_CCT_TYPE_A;
  }
  /* else its TL84*/

  return cct_type;
}


/** isp_sub_module_util_get_awb_cct_H_type
 *
 *    @trigger:
 *    @chromatix_ptr:
 *
 **/
awb_cct_type isp_sub_module_util_get_awb_cct_with_H_type(
   cct_trigger_info* trigger)
{
  awb_cct_type cct_type = AWB_CCT_TYPE_TL84;

  ISP_DBG("%s: CCT %f D65 %f %f A %f %f H %f %f", __func__,
    trigger->mired_color_temp,
    trigger->trigger_d65.mired_end,
    trigger->trigger_d65.mired_start,
    trigger->trigger_A.mired_start,
    trigger->trigger_A.mired_end,
    trigger->trigger_H.mired_end,
    trigger->trigger_H.mired_start);

  if (trigger->mired_color_temp <= trigger->trigger_d65.mired_end) {
    cct_type = AWB_CCT_TYPE_D65;
  } else if ((trigger->mired_color_temp > trigger->trigger_d65.mired_end) &&
             (trigger->mired_color_temp <= trigger->trigger_d65.mired_start)) {
    cct_type = AWB_CCT_TYPE_D65_TL84;
  } else if ((trigger->mired_color_temp >= trigger->trigger_A.mired_start) &&
             (trigger->mired_color_temp < trigger->trigger_A.mired_end)) {
    cct_type = AWB_CCT_TYPE_TL84_A;
  } else if ((trigger->mired_color_temp >= trigger->trigger_A.mired_end) &&
             (trigger->mired_color_temp < trigger->trigger_H.mired_start)) {
    cct_type = AWB_CCT_TYPE_A;
  } else if ((trigger->mired_color_temp >= trigger->trigger_H.mired_start) &&
             (trigger->mired_color_temp < trigger->trigger_H.mired_end)) {
    cct_type = AWB_CCT_TYPE_A_H;
  } else if (trigger->mired_color_temp >= trigger->trigger_H.mired_end) {
    cct_type = AWB_CCT_TYPE_H;
  }
  /* else its TL84*/

  return cct_type;
}

/** isp_sub_module_util_get_awb_ext_cct_type
 *
 *    @trigger:
 *    @chromatix_ptr:
 *
 **/
awb_cct_type isp_sub_module_util_get_awb_ext_cct_type(
  cct_trigger_info* trigger, void *chromatix_ptr)
{
  chromatix_parms_type *p_chromatix = chromatix_ptr;
  awb_cct_type cct_type = AWB_CCT_TYPE_TL84;

  ISP_DBG("%s: CCT %f D65 %f %f A %f %f D50 %f %f", __func__,
    trigger->mired_color_temp,
    trigger->trigger_d65.mired_end,
    trigger->trigger_d65.mired_start,
    trigger->trigger_A.mired_start,
    trigger->trigger_A.mired_end,
    trigger->trigger_d50.mired_start,
    trigger->trigger_d50.mired_end);

  if (trigger->mired_color_temp <= trigger->trigger_d65.mired_end) {
    cct_type = AWB_CCT_TYPE_D65;
  } else if ((trigger->mired_color_temp > trigger->trigger_d65.mired_end) &&
             (trigger->mired_color_temp <= trigger->trigger_d65.mired_start)) {
    cct_type = AWB_CCT_TYPE_D50_D65;
  } else if ((trigger->mired_color_temp > trigger->trigger_d65.mired_start) &&
             (trigger->mired_color_temp <= trigger->trigger_d50.mired_end)) {
    cct_type = AWB_CCT_TYPE_D50;
  } else if ((trigger->mired_color_temp > trigger->trigger_d50.mired_end) &&
             (trigger->mired_color_temp <= trigger->trigger_d50.mired_start)) {
    cct_type = AWB_CCT_TYPE_D50_TL84;
  } else if ((trigger->mired_color_temp >= trigger->trigger_A.mired_start) &&
             (trigger->mired_color_temp < trigger->trigger_A.mired_end)) {
    cct_type = AWB_CCT_TYPE_TL84_A;
  } else if ((trigger->mired_color_temp >= trigger->trigger_A.mired_end) &&
             (trigger->mired_color_temp < trigger->trigger_H.mired_start)) {
    cct_type = AWB_CCT_TYPE_A;
  } else if ((trigger->mired_color_temp >= trigger->trigger_H.mired_start) &&
             (trigger->mired_color_temp < trigger->trigger_H.mired_end)) {
    cct_type = AWB_CCT_TYPE_A_H;
  } else if (trigger->mired_color_temp >= trigger->trigger_H.mired_end) {
    cct_type = AWB_CCT_TYPE_H;
  }
  /* else its TL84*/

  return cct_type;
}


/** isp_sub_module_util_get_awb_ext_h_cct_type
 *
 *    @trigger:
 *    @chromatix_ptr:
 *
 **/
awb_cct_type isp_sub_module_util_get_awb_ext_h_cct_type(
  cct_trigger_info* trigger, void *chromatix_ptr)
{
  chromatix_parms_type *p_chromatix = chromatix_ptr;
  awb_cct_type cct_type = AWB_CCT_TYPE_TL84;

  ISP_DBG("%s: CCT %f D65 %f %f A %f %f D50 %f %f", __func__,
    trigger->mired_color_temp,
    trigger->trigger_d65.mired_end,
    trigger->trigger_d65.mired_start,
    trigger->trigger_A.mired_start,
    trigger->trigger_A.mired_end,
    trigger->trigger_d50.mired_start,
    trigger->trigger_d50.mired_end);

  if (trigger->mired_color_temp <= trigger->trigger_d65.mired_end) {
    cct_type = AWB_CCT_TYPE_D65;
  } else if ((trigger->mired_color_temp > trigger->trigger_d65.mired_end) &&
             (trigger->mired_color_temp <= trigger->trigger_d65.mired_start)) {
    cct_type = AWB_CCT_TYPE_D50_D65;
  } else if ((trigger->mired_color_temp > trigger->trigger_d65.mired_start) &&
             (trigger->mired_color_temp <= trigger->trigger_d50.mired_end)) {
     cct_type = AWB_CCT_TYPE_D50;
  } else if ((trigger->mired_color_temp > trigger->trigger_d50.mired_end) &&
             (trigger->mired_color_temp <= trigger->trigger_d50.mired_start)) {
    cct_type = AWB_CCT_TYPE_D50_TL84;
  } else if ((trigger->mired_color_temp > trigger->trigger_d50.mired_start) &&
             (trigger->mired_color_temp <= trigger->trigger_A.mired_start)) {
    cct_type = AWB_CCT_TYPE_TL84;
  } else if ((trigger->mired_color_temp >= trigger->trigger_A.mired_start) &&
             (trigger->mired_color_temp < trigger->trigger_A.mired_end)) {
    cct_type = AWB_CCT_TYPE_TL84_A;
  } else if ((trigger->mired_color_temp >= trigger->trigger_A.mired_end) &&
            (trigger->mired_color_temp < trigger->trigger_H.mired_start)) {
    cct_type = AWB_CCT_TYPE_A;
  } else if ((trigger->mired_color_temp >= trigger->trigger_H.mired_start) &&
            (trigger->mired_color_temp < trigger->trigger_H.mired_end)) {
    cct_type = AWB_CCT_TYPE_A_H;
  } else {
    cct_type = AWB_CCT_TYPE_H;
  }

  return cct_type;
}
/** isp_sub_module_util_calculate_shift_bits:
 *
 *  @pixels: input pixels
 *  @input_depth: input_depth in bits
 *  @output_depth: output_depth in bits
 *  Calulate shift bits for given input pixels
 *
 *  Return shift_bits
 **/
uint32_t isp_sub_module_util_calculate_shift_bits(uint32_t pixels,
  uint32_t input_depth, uint32_t output_depth)
{
  uint32_t log2_val = 0;
  uint32_t shift_bits = 0;
  uint32_t diff = output_depth - input_depth;

  while (pixels) {
    log2_val++;
    pixels = pixels >> 1;
  }

  if (log2_val > diff) {
    shift_bits = log2_val - diff;
  } else {
    shift_bits = 0;
  }

  return shift_bits;
} /* isp_sub_module_util_calculate_shift_bits */

/** isp_sub_module_util_power:
 *
 *  @base: base
 *  @exponent: exponent
 *
 *  Calulate base raised to the exponent power.
 *
 *  Return power result
 **/
double isp_sub_module_util_power(double base, double exponent)
{
  double result = 0.0;

  if (exponent == 0.0) {
    result = 1.0;
  } else if (base != 0.0) {
    result = pow(base, exponent);
  } else {
    result = 0.0;
  }
  ISP_DBG("power: %f^%f = %f", base, exponent, result);

  return result;
} /* isp_sub_module_util_power */

/** isp_sub_module_util_pack_cfg_cmd
 *
 *    @reg_cfg_cmd: reg config cmd
 *    @cmd_offset:command
 *    @cmd_len: command lengths
 *    @cmd_type: type of command
 *    @hw_reg_offset: HW register offset
 **/
void isp_sub_module_util_pack_cfg_cmd(
  struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd,
  uint32_t cmd_offset, uint32_t cmd_len,
  uint32_t cmd_type, uint32_t hw_reg_offset)
{
  reg_cfg_cmd->u.rw_info.cmd_data_offset = cmd_offset;
  reg_cfg_cmd->cmd_type = cmd_type;
  reg_cfg_cmd->u.rw_info.reg_offset = hw_reg_offset;
  reg_cfg_cmd->u.rw_info.len = cmd_len;
}

/** isp_sub_module_util_pack_dmi_cmd
 *
 *    @reg_cfg_cmd: reg config cmd
 *    @cmd_offset:command
 *    @cmd_len: command lengths
 *    @cmd_type: type of command
 *    @hw_reg_offset: HW register offset
 **/
void isp_sub_module_util_pack_dmi_cmd(
  struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd,
  uint32_t hi_tbl_offset, uint32_t lo_tbl_offset,uint32_t cmd_len,
  uint32_t cmd_type)
{
  reg_cfg_cmd->u.dmi_info.hi_tbl_offset = hi_tbl_offset;
  reg_cfg_cmd->u.dmi_info.lo_tbl_offset = lo_tbl_offset;
  reg_cfg_cmd->cmd_type = cmd_type;
  reg_cfg_cmd->u.dmi_info.len = cmd_len;
}

/** isp_sub_module_util_reset_hist_dmi
 *
 *    @isp_sub_module: isp sub module object
 *    @dmi_channel : dmi channel
 *
 **/
boolean isp_sub_module_util_reset_hist_dmi(isp_sub_module_t *isp_sub_module,
  uint32_t dmi_channel, uint32_t dmi_cfg_default, uint32_t dmi_cfg_off,
  uint32_t dmi_addr)
{
  boolean                      ret = TRUE;
  Hist_DMI_CfgCmdType          *hist_reset_cfg = NULL;
  uint32_t                     cmd_offset, cmd_len, tbl_len;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: %p", isp_sub_module);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd)*5);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd)*5);

  hist_reset_cfg = (Hist_DMI_CfgCmdType *)CAM_MALLOC(sizeof(*hist_reset_cfg));
  if (!hist_reset_cfg) {
    ISP_ERR("failed: hist_reset_cfg %p", hist_reset_cfg);
    goto ERROR_COPY_CMD;
  }
  memset(hist_reset_cfg, 0, sizeof(*hist_reset_cfg));

  /* 1. program DMI default value, write auto increment bit
     2. write DMI table
     3. reset DMI cfg */
  cfg_cmd = &hw_update->cfg_cmd;
  cfg_cmd->cfg_data = (void *)hist_reset_cfg;
  cfg_cmd->cmd_len = sizeof(*hist_reset_cfg);
  cfg_cmd->cfg_cmd = (void *) reg_cfg_cmd;
  cfg_cmd->num_cfg = 5;

  /* set dmi to proper hist stats bank */
  hist_reset_cfg->set_channel = dmi_cfg_default + dmi_channel;
  cmd_offset = 0;
  cmd_len = 1 * sizeof(uint32_t);

  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[0], cmd_offset, cmd_len,
    VFE_WRITE_MB, dmi_cfg_off);

  /* set start addr = 0*/
  hist_reset_cfg->set_start_addr = 0;
  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[1], cmd_offset, cmd_len,
    VFE_WRITE_MB, dmi_addr);

  /* memset hi and lo tbl = all 0,
     64 bit write: regarding interlieve uint64 table
       hi_tbl_offset = table offset
       lo_tbl_offset = table offset + sizeof(uint32_t) */
  tbl_len = sizeof(uint64_t) * 256;
  cmd_offset += cmd_len;
  cmd_len = tbl_len;
  isp_sub_module_util_pack_dmi_cmd(&reg_cfg_cmd[2], cmd_offset + sizeof(uint32_t),
    cmd_offset, cmd_len, VFE_WRITE_DMI_64BIT);

  /* reset the sart addr = 0 */
  hist_reset_cfg->reset_start_addr = 0;
  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[3], cmd_offset, cmd_len,
    VFE_WRITE_MB, dmi_addr);

  /* set dmi to proper hist stats bank */
  hist_reset_cfg->reset_channel = dmi_cfg_default;
  cmd_offset += cmd_len;
  cmd_len = 1 * sizeof(uint32_t);
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[4], cmd_offset, cmd_len,
    VFE_WRITE_MB, dmi_cfg_off);

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;
  }

  return TRUE;

  ERROR_APPEND:
    free(hist_reset_cfg);
  ERROR_COPY_CMD:
    free(reg_cfg_cmd);
  ERROR_REG_CFG_CMD:
    free(hw_update);
  return FALSE;
}

/** isp_sub_module_util_write_dmi:
 *
 *    @tbl: table pointer
 *    @tbl_len: table length in bytes
 *    @channel: dmi channel to write to
 *    @isp_sub_module: base module
 *
 *  This function updates DMI tables
 *
 *  Return:   TRUE - Success FALSE- failure
 **/
boolean isp_sub_module_util_write_dmi(void *tbl,
  uint32_t tbl_len, uint32_t channel,
  enum msm_vfe_reg_cfg_type dmi_write_type,
  uint32_t dmi_cfg_reg_offset, uint32_t dmi_addr_reg_offset,
  isp_sub_module_t *isp_sub_module)
{
  boolean                      ret         = TRUE;
  uint32_t                    *data        = NULL;
  uint32_t                     payld_len   = 0;
  uint32_t                    *cfg_payload = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd     = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update   = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: %p", isp_sub_module);
    return FALSE;
  }

  #define PAYLD_IDX_DMI_CHNL  0
  #define PAYLD_IDX_DMI_ADDR  1
  #define PAYLD_IDX_NO_MEM    2
  #define PAYLD_IDX_TBL       3

  payld_len = sizeof(uint32_t) * PAYLD_IDX_TBL + tbl_len;
  cfg_payload = (uint32_t *)CAM_MALLOC(payld_len);
  if (!cfg_payload) {
    ISP_ERR("failed: cfg_payload %p", cfg_payload);
    return FALSE;
  }
  memset(cfg_payload, 0, payld_len);
  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    goto ERROR_FREE_PAYLOAD;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd) * 5);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_FREE_HW_UPDATE;
  }
  memset(reg_cfg_cmd, 0, (sizeof(*reg_cfg_cmd)));

  cfg_cmd = &hw_update->cfg_cmd;
  cfg_cmd->cfg_data = cfg_payload;
  cfg_cmd->cmd_len = payld_len;
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 5;

  /* Configure DMI CFG with channel and auto increment */
  cfg_payload[PAYLD_IDX_DMI_CHNL] = 0x100 + (channel & 0xff);
  isp_sub_module_util_pack_cfg_cmd(reg_cfg_cmd + 0,
    sizeof(uint32_t) * PAYLD_IDX_DMI_CHNL,
    sizeof(uint32_t) * 1, VFE_WRITE_MB,
    dmi_cfg_reg_offset);

  /* Configure DMI addr to 0 */
  cfg_payload[PAYLD_IDX_DMI_ADDR] = 0;
  isp_sub_module_util_pack_cfg_cmd(reg_cfg_cmd + 1,
    sizeof(uint32_t) * PAYLD_IDX_DMI_ADDR,
    sizeof(uint32_t) * 1, VFE_WRITE_MB,
    dmi_addr_reg_offset);

  /* Actual writing of DMI table */
  memcpy(&cfg_payload[PAYLD_IDX_TBL], tbl, tbl_len);
  isp_sub_module_util_pack_dmi_cmd(reg_cfg_cmd + 2,
    sizeof(uint32_t) + sizeof(uint32_t) * PAYLD_IDX_TBL,
    sizeof(uint32_t) * PAYLD_IDX_TBL,
    tbl_len,
    dmi_write_type);

  /* NO_MEM channel with auto increment */
  cfg_payload[PAYLD_IDX_NO_MEM] = 0x100;
  isp_sub_module_util_pack_cfg_cmd(reg_cfg_cmd + 3,
    sizeof(uint32_t) * PAYLD_IDX_NO_MEM,
    sizeof(uint32_t) * 1, VFE_WRITE_MB,
    dmi_cfg_reg_offset);

  /* Configure DMI addr to 0 */
  reg_cfg_cmd[4] = reg_cfg_cmd[1];

  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_FREE_REG_CFG_CMD;
  }
  return ret;

ERROR_FREE_REG_CFG_CMD:
  free(reg_cfg_cmd);
ERROR_FREE_HW_UPDATE:
  free(hw_update);
ERROR_FREE_PAYLOAD:
  free(cfg_payload);
  return FALSE;
} /* isp_sub_module_util_write_dmi */

/** isp_sub_module_util_hw_reg_set:
 *    @data: ptr to data
 *    @len :length of data.
 *
 *  This function prints the register values set by data ptr.
*/
boolean isp_sub_module_util_hw_reg_set(uint32_t *data, uint32_t len)
{
  uint32_t i = 0;
  for (i = 0; i < len/4; i++) {
    ISP_ERR("data[%d] %x", i, data[i]);
  }
  return TRUE;
}

/** isp_sub_module_util_update_module_bit:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: payload
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function enables / disables module
 *
 *  Return: TRUE on success and FALSE on failure
 *
 *  NOTE: THE CALLER SHOULD LOCK THE SUBMODULE MUTEX
 **/
boolean isp_sub_module_util_update_module_bit(isp_sub_module_t *isp_sub_module,
  void *data)
{
  isp_sub_module_output_t  *sub_module_output = NULL;
  isp_module_enable_info_t *module_enable_info = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  if (isp_sub_module->hw_module_id >= ISP_MOD_MAX_NUM) {
    ISP_ERR("failed: Invalid module index %d", isp_sub_module->hw_module_id);
    return FALSE;
  }

  sub_module_output = (isp_sub_module_output_t *)data;
  if (!sub_module_output) {
    ISP_ERR("failed: output %p", sub_module_output);
    return FALSE;
  }

  if (!sub_module_output->stats_params) {
    ISP_ERR("failed: sub_module_output->stats_params %p",
      sub_module_output->stats_params);
    return FALSE;
  }

  module_enable_info = &sub_module_output->stats_params->module_enable_info;
  module_enable_info->reconfig_needed = TRUE;
  module_enable_info->submod_mask[isp_sub_module->hw_module_id] = 1;
  if (isp_sub_module->submod_enable == TRUE) {
    module_enable_info->submod_enable[isp_sub_module->hw_module_id] = 1;
  } else {
    module_enable_info->submod_enable[isp_sub_module->hw_module_id] = 0;
  }

  isp_sub_module->update_module_bit = FALSE;

  return TRUE;
} /* isp_sub_module_util_update_module_bit */

/** isp_sub_module_util_convert_ez_to_isp_id
 *
 *  @ez_isp_id: Eztune isp id
 *
 *  Convert eztune isp id to internal enum
 *
 *  Return internal ISP id
 **/
isp_hw_module_id_t isp_sub_module_util_convert_ez_to_isp_id(uint8_t ez_isp_id)
{
   switch (ez_isp_id) {
   case VFE_MODULE_LINEARIZATION: {
     return ISP_MOD_LINEARIZATION;
   }
     break;

   case VFE_MODULE_COLORCORRECTION: {
     return ISP_MOD_COLOR_CORRECT;
   }
     break;

   case VFE_MODULE_COLORCONVERSION: {
     return ISP_MOD_CHROMA_ENHANCE;
   }
     break;

   case VFE_MODULE_GAMMA: {
     return ISP_MOD_GAMMA;
   }
     break;

   case VFE_MODULE_BLACKLEVEL: {
     return ISP_MOD_LINEARIZATION;
   }
     break;

   case VFE_MODULE_ASF5X5: {
     return ISP_MOD_ASF;
   }
     break;

   case VFE_MODULE_LUMAADAPTATION: {
     return ISP_MOD_LA;
   }
     break;

   case VFE_MODULE_ROLLOFF: {
     return ISP_MOD_ROLLOFF;
   }
     break;

   case VFE_MODULE_BPC: {
     return ISP_MOD_BPC;
   }
     break;

   case VFE_MODULE_BCC: {
     return ISP_MOD_BCC;
   }
     break;

   case VFE_MODULE_ABCC: {
     return ISP_MOD_ABCC;
   }
     break;

   case VFE_MODULE_CHROMASUPPRESSION: {
     return ISP_MOD_CHROMA_SUPPRESS;
   }
     break;

   case VFE_MODULE_MCE: {
     return ISP_MOD_MCE;
   }
     break;

   case VFE_MODULE_SCE: {
     return ISP_MOD_SCE;
   }
     break;

   case VFE_MODULE_DEMOSAIC: {
     return ISP_MOD_DEMOSAIC;
   }
     break;

   case VFE_MODULE_DEMUX: {
     return ISP_MOD_DEMUX;
   }
     break;

   case VFE_MODULE_CLFILTER: {
     return ISP_MOD_CLF;
   }
     break;

   case VFE_MODULE_GIC: {
     return ISP_MOD_GIC;
   }
     break;

   case VFE_MODULE_GTM: {
     return ISP_MOD_GTM;
   }
     break;

   case VFE_MODULE_PEDESTAL: {
     return ISP_MOD_PEDESTAL;
   }
     break;

   case VFE_MODULE_ALL: {
     return ISP_MOD_ALL;
   }
     break;

   default: {
     ISP_ERR("failed: invalid ez_isp_id %d", ez_isp_id);
     return ISP_MOD_MAX_NUM;
   }
  }
} /* isp_sub_module_util_convert_ez_to_isp_id */

/** isp_sub_module_util_configure_from_chromatix_bit:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Enable / disable module based on chromatix bit
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_configure_from_chromatix_bit(
  isp_sub_module_t *isp_sub_module)
{
  boolean                    ret = TRUE;
  chromatix_parms_type      *chromatix_ptr = NULL;
  chromatix_VFE_common_type *chromatix_common = NULL;
  char                       value[PROPERTY_VALUE_MAX];
  int                        mod_disable = 0;

  if (!isp_sub_module || !isp_sub_module->chromatix_ptrs.chromatixComPtr ||
    !isp_sub_module->chromatix_ptrs.chromatixPtr) {
    ISP_ERR("failed: %p", isp_sub_module);
    return FALSE;
  }

  chromatix_common =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;
  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;

  /* Set update_module_bit to TRUE so that module will be enabled /
   * disabled in the next trigger update
   */
  isp_sub_module->update_module_bit = TRUE;
  isp_sub_module->submod_enable = FALSE;
  isp_sub_module->chromatix_module_enable = FALSE;

  switch (isp_sub_module->hw_module_id) {
  case ISP_MOD_LINEARIZATION:
    if (chromatix_common->chromatix_L.linearization_enable > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    break;

  case ISP_MOD_BLS:
    if (chromatix_common->Chromatix_BLSS_data.BLSS_enable > 0) {
      isp_sub_module->submod_enable = TRUE;
    }
    break;

  case ISP_MOD_ROLLOFF:
    if (chromatix_ptr->chromatix_VFE.chromatix_rolloff_trigger.rolloff_enable >
      0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }

    property_get("persist.vendor.camera.isp.rolloff_dsbl", value, "0");
    mod_disable = atoi(value);
    if ( mod_disable == 1) {
      isp_sub_module->submod_enable = FALSE;
      isp_sub_module->chromatix_module_enable = FALSE;
    }

    break;

  case ISP_MOD_BPC:
  if (chromatix_ptr->chromatix_VFE.chromatix_BPC2_data.BPC2_en > 0 ) {
    isp_sub_module->submod_enable = TRUE;
    isp_sub_module->chromatix_module_enable = TRUE;
  }
  break;

  case ISP_MOD_BCC:
  if (CHROMATIX_BPC_BCC(chromatix_ptr, bcc) > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    break;

  case ISP_MOD_ABF:
    if ((chromatix_ptr->chromatix_VFE.chromatix_ABF3.abf3_enable > 0) ||
      (chromatix_ptr->chromatix_VFE.chromatix_ABF2.ABF2_enable > 0)) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    break;

  case ISP_MOD_DEMOSAIC:
    if (chromatix_ptr->chromatix_VFE.chromatix_demosaic.demosaic_enable> 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    break;

  case ISP_MOD_GAMMA:
    if (chromatix_ptr->chromatix_VFE.chromatix_gamma.gamma_enable > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    break;

  case ISP_MOD_LTM:
    if (chromatix_ptr->chromatix_post_processing.chromatix_LTM_data.enable) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    } else {
      isp_sub_module->submod_enable = FALSE;
      isp_sub_module->chromatix_module_enable = FALSE;
    }

    property_get("persist.vendor.camera.isp.ltm_disable", value, "0");
    mod_disable = atoi(value);
    if (mod_disable == 1) {
      isp_sub_module->submod_enable = FALSE;
      isp_sub_module->chromatix_module_enable = FALSE;
    }
    break;

  case ISP_MOD_LA:
    if (chromatix_ptr->chromatix_VFE.chromatix_LA.LA_enable > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    break;

  case ISP_MOD_CHROMA_ENHANCE:
    if (chromatix_ptr->chromatix_VFE.chromatix_CV.cv_enable > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    break;

  case ISP_MOD_COLOR_CORRECT:
    if (chromatix_ptr->chromatix_VFE.chromatix_CC.cc_enable > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    break;

  case ISP_MOD_CHROMA_SUPPRESS:
    if (chromatix_ptr->chromatix_VFE.chromatix_CS_MCE.CS_enable > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    break;

  case ISP_MOD_MCE:
    if (chromatix_ptr->chromatix_VFE.chromatix_CS_MCE.MCE_enable > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    break;

  case ISP_MOD_SCE:
    if (chromatix_ptr->chromatix_VFE.chromatix_SCE.sce_enable > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    } else {
      isp_sub_module->submod_enable = FALSE;
      isp_sub_module->chromatix_module_enable = FALSE;
    }
    break;

  case ISP_MOD_GTM:
    if (chromatix_ptr->chromatix_VFE.chromatix_gtm.gtm_enable > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    break;

  case ISP_MOD_GIC:
    if (chromatix_ptr->chromatix_VFE.chromatix_GIC.gic_enable > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    break;

  case ISP_MOD_SNR:
    if (chromatix_ptr->chromatix_VFE.chromatix_skin_nr_data.skintone_nr_en > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    break;

  case ISP_MOD_HDR:
    if ((chromatix_ptr->chromatix_post_processing.chromatix_video_HDR.
        videoHDR_reserve_data.hdr_mac_en > 0) ||
        (chromatix_ptr->chromatix_post_processing.chromatix_video_HDR.
        videoHDR_reserve_data.hdr_recon_en > 0)) {

      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    break;

  case ISP_MOD_PEDESTAL:
    if (chromatix_common->chromatix_pedestal_correction.
      pedestalcorrection_enable > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    break;

  case ISP_MOD_CLF:
    if (chromatix_ptr->chromatix_post_processing.
      chromatix_chroma_aliasing_correction.ThEdge >= 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
    }
    property_get("persist.vendor.camera.isp.cac_disable", value, "0");
    mod_disable = atoi(value);
    if (mod_disable == 1) {
      isp_sub_module->submod_enable = FALSE;
      isp_sub_module->chromatix_module_enable = FALSE;
    }
    break;

  case ISP_MOD_CAC:
    /* enable and disable from chromatix only happen once in beginning.
    after that, module can enalbe and disable themselves by chromatix en bit*/
    if ((chromatix_ptr->chromatix_VFE.chromatix_hw_CAC2_data.CAC2_en > 0)) {
      isp_sub_module->chromatix_module_enable = TRUE;
      isp_sub_module->submod_enable = TRUE;
    } else {
      isp_sub_module->chromatix_module_enable = FALSE;
      isp_sub_module->submod_enable = FALSE;
    }
    break;

  case ISP_MOD_DEMUX:
  case ISP_MOD_BE_STATS:
  case ISP_MOD_BG_STATS:
  case ISP_MOD_BF_STATS:
  case ISP_MOD_AWB_STATS:
  case ISP_MOD_RS_STATS:
  case ISP_MOD_CS_STATS:
  case ISP_MOD_IHIST_STATS:
  case ISP_MOD_SKIN_BHIST_STATS:
  case ISP_MOD_BF_SCALE_STATS:
  case ISP_MOD_WB:
  case ISP_MOD_COLOR_XFORM_VIEWFINDER:
  case ISP_MOD_SCALER_VIEWFINDER:
  case ISP_MOD_FOV_VIEWFINDER:
  case ISP_MOD_COLOR_XFORM_ENCODER:
  case ISP_MOD_SCALER_ENCODER:
  case ISP_MOD_FOV_ENCODER:
  case ISP_MOD_HDR_BE_STATS:
  case ISP_MOD_HDR_BHIST_STATS:
  case ISP_MOD_COLOR_XFORM_VIDEO:
  case ISP_MOD_SCALER_VIDEO:
  case ISP_MOD_FOV_VIDEO:
  case ISP_MOD_CLAMP_VIEWFINDER:
  case ISP_MOD_CLAMP_ENCODER:
  case ISP_MOD_CLAMP_VIDEO:
  case ISP_MOD_ABCC:
    /* Enable by default */
    isp_sub_module->submod_enable = TRUE;
    isp_sub_module->chromatix_module_enable = TRUE;
    break;

  case ISP_MOD_ASF:
  case ISP_MOD_ALL:
  case ISP_MOD_MAX_NUM:
  default:
    ISP_ERR("failed: invalid module type %d",
      isp_sub_module->hw_module_id);
    isp_sub_module->update_module_bit = FALSE;
    ret = FALSE;
    break;
  }

  return ret;
} /* isp_sub_module_util_configure_from_chromatix_bit */


/** isp_sub_mod_util_compare_identity:
 *
 *  @data1: identity1
 *
 *  @data2: identity2
 *
 *  Return TRUE if identity matches, FALSE otherwise
 **/
boolean isp_sub_mod_util_compare_identity(void *data1, void *data2)
{
  uint32_t *identity1 = (uint32_t *)data1;
  uint32_t *identity2 = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  ISP_HIGH("%d %d", *identity1, *identity2);
  if (*identity1 == *identity2) {
    return TRUE;
  }

  return FALSE;
}

/** isp_sub_mod_util_find_port_based_on_identity:
 *
 *  @data1: mct port handle
 *
 *  @data2: identity
 *
 *  Retrieve port based on identity
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_mod_util_find_port_based_on_identity(void *data1,
  void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  uint32_t   *identity = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }
  if (mct_list_find_custom(MCT_PORT_CHILDREN(port), identity,
    isp_sub_mod_util_compare_identity)) {
  ISP_HIGH("");
    return TRUE;
  }

  return FALSE;
}

/** isp_sub_mod_get_port_from_module:
 *
 *  @module: mct module handle
 *
 *  @port: port to return
 *
 *  @direction: direction of port to be retrieved
 *
 *  @identity: identity to match against
 *
 *  Find port based on direction and identity and return it
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_mod_get_port_from_module(mct_module_t *module,
  mct_port_t **port, mct_port_direction_t direction, uint32_t identity)
{
  mct_list_t *l_port = NULL;

  if (!module || !port) {
    ISP_ERR("failed: module %p sink_port %p", module, port);
    return FALSE;
  }

  /* Reset output param */
  *port = NULL;

  if (direction == MCT_PORT_SINK) {
    l_port = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &identity,
      isp_sub_mod_util_find_port_based_on_identity);
    if (!l_port) {
      ISP_ERR("failed: mct_list_find_custom for ide %x", identity);
      return FALSE;
    }
  } else  if (direction == MCT_PORT_SRC) {
    l_port = mct_list_find_custom(MCT_MODULE_SRCPORTS(module), &identity,
      isp_sub_mod_util_find_port_based_on_identity);
    if (!l_port) {
      ISP_ERR("failed: mct_list_find_custom for ide %x", identity);
      return FALSE;
    }
  } else {
    ISP_ERR("failed: invalid direction %d", direction);
    return FALSE;
  }

  *port = (mct_port_t *)l_port->data;

  return TRUE;
}

/** isp_sub_mod_util_send_event:
 *
 *  @module: mct module
 *
 *  @event: mct_event
 *
 *  Send event to other isp sub modules
 *  Return TRUE on success and FALSE on failure
 **/

boolean isp_sub_mod_util_send_event(mct_module_t *module,
  mct_event_t *event)
{
  boolean rc = TRUE;
  mct_port_t *port = NULL;
  mct_list_t *l_port = NULL;
  mct_port_direction_t direction;

  if (!module || !event) {
    ISP_ERR("failed module %p event %p", module, event);
    return FALSE;
  }
  if (event->direction == MCT_EVENT_UPSTREAM)
    direction = MCT_PORT_SINK;
  else
    direction = MCT_PORT_SRC;

  rc = isp_sub_mod_get_port_from_module(module, &port, direction,
    event->identity);
  if (rc == FALSE || !port ) {
    ISP_ERR("failed: %p rc= %d", port, rc);
    return rc;
  }

  rc = mct_port_send_event_to_peer(port, event);
  if (rc == FALSE) {
    ISP_ERR("failed: rc = FALSE");
    return rc;
  }
  return rc;
}
/** isp_sub_module_util_temporal_lux_calc:
 *
 *  @mesh_rolloff;
 *  @lux_idx: lux index
 *
 *  Calculating the temporal based lux index
 *
 *  Return none
 **/

void isp_sub_module_util_temporal_lux_calc(isp_temporal_luxfilter_params_t
  *lux_filter, float *lux_idx)
{

  float delta         = 0.0f;
  float prev_lux      = 0.0f;
  float curr_lux      = 0.0f;
  float temporal_lux  = 0.0f;

  if (!lux_filter || !lux_idx) {
    ISP_ERR("Invalid argument mesh_rolloff %p lux_idx %p",
      lux_filter, lux_idx );
    return;
  }
  curr_lux = *lux_idx;

  if(!lux_filter->init_flag) {
    temporal_lux = prev_lux = curr_lux;
    lux_filter->init_flag = TRUE;
  } else {
    temporal_lux = lux_filter->temporal_lux;
    prev_lux     = lux_filter->prev_lux;
  }

  /* 75% of temporal + 25% of current lux */
  temporal_lux = (temporal_lux) * (1-TEMPORAL_WEIGHTS) +
    (curr_lux * TEMPORAL_WEIGHTS);
  delta = temporal_lux - prev_lux;
  curr_lux = prev_lux + TEMPORAL_STRENGTH * delta;
  if (curr_lux < 0)
    curr_lux = 0;
  /* storing curr_lux t previous */
  lux_filter->temporal_lux = temporal_lux;
  lux_filter->prev_lux = curr_lux;
  *lux_idx = curr_lux;
}


/** isp_sub_module_util_setloglevel:
 *
 *  @name: ISP hw module unique name
 *  @isp_log_sub_modules_t: module id of the hardware module
 *
 *  This function sets the log level of the hardware module id
 *  passed based on the setprop.
 *  persist.vendor.camera.debug.xxx  (xxx = hw module name)
 *  It also arbitrates betweeen the global setprop
 *  persist.vendor.camera.global.debug
 *  The final log level is decided on whichever is higher
 *  1 - ERR Logs
 *  3 - HIGH Logs + ERR logs
 *  4 - DBG logs + HIGH Logs + ERR Logs
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_setloglevel(const char *name,
  isp_log_sub_modules_t mod_id)
{
  uint32_t      globalloglevel = 0;
  char          prop[PROPERTY_VALUE_MAX];
  char          buf[255];
  uint32_t      isp_module_loglevel = 0,
                isploglevel;
  char          id[PROPERTY_VALUE_MAX];

  if (!name) {
    ISP_ERR("failed: %p", name);
    return FALSE;
  }

  strlcpy(buf, "persist.vendor.camera.debug.", sizeof(buf));
  snprintf(id, sizeof(id),"%d",mod_id);
  strlcat(buf, id, sizeof(buf));

  property_get(buf, prop, "0");
  isp_module_loglevel = atoi(prop);
  memset(prop, 0, sizeof(prop));

  property_get("persist.vendor.camera.global.debug", prop, "0");
  globalloglevel = atoi(prop);
  if (globalloglevel > isp_module_loglevel) {
     isp_module_loglevel = globalloglevel;
  }

  isp_modules_loglevel[mod_id] = isp_module_loglevel;
  ISP_DBG("%s: ###ISP_Loglevel %d", buf, isp_module_loglevel);
  return TRUE;
}

/** isp_sub_module_util_update_cgc_mask:
 *
 *  @isp_sub_module: isp sub module handle
 *  @reg_offset: register offset
 *  @cgc_bit: cgc mask bit
 *  @enable: enable / disable cgc
 *
 *  Create hw update list to enable / disable cgc
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_update_cgc_mask(isp_sub_module_t *isp_sub_module,
  uint32_t reg_offset, uint32_t cgc_bit, boolean enable)
{
  boolean                      ret = TRUE;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update   = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd     = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: %p %p", isp_sub_module, reg_cfg_cmd);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    goto ERROR;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_FREE_HW_UPDATE;
  }
  memset(reg_cfg_cmd, 0, (sizeof(*reg_cfg_cmd)));

  cfg_cmd = &hw_update->cfg_cmd;
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd->cmd_type = VFE_CFG_MASK;
  reg_cfg_cmd->u.mask_info.reg_offset = reg_offset;
  reg_cfg_cmd->u.mask_info.mask = 1 << cgc_bit;
  if (enable == TRUE) {
    reg_cfg_cmd->u.mask_info.val = 1 << cgc_bit;
  } else {
    reg_cfg_cmd->u.mask_info.val = 0;
  }

  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_FREE_REG_CFG_CMD;
  }

  return ret;

ERROR_FREE_REG_CFG_CMD:
  if (reg_cfg_cmd) {
    free(reg_cfg_cmd);
  }
ERROR_FREE_HW_UPDATE:
  if (hw_update) {
    free(hw_update);
  }
ERROR:
  return FALSE;
}

/** isp_sub_module_getscalefactor
 *
 *     @fmt: module handle
 *
 **/
void isp_sub_module_getscalefactor(cam_format_t fmt, float *scale_w, float *scale_h)
{
  *scale_w = 1;
  *scale_h = 1;

  switch (fmt) {
    case CAM_FORMAT_YUV_420_NV12:
    case CAM_FORMAT_YUV_420_NV12_UBWC:
    case CAM_FORMAT_YUV_420_NV21:
    case CAM_FORMAT_YUV_420_NV12_VENUS:
    case CAM_FORMAT_YUV_420_NV21_VENUS:
    case CAM_FORMAT_YUV_420_NV21_ADRENO:
    case CAM_FORMAT_YUV_420_YV12:
    case CAM_FORMAT_Y_ONLY: {
      *scale_w = 2;
      *scale_h = 2;
    }
      break;

    case CAM_FORMAT_YUV_422_NV61:
    case CAM_FORMAT_YUV_422_NV16: {
      *scale_w = 2;
      *scale_h = 1;
    }
      break;

    case CAM_FORMAT_YUV_444_NV24:
    case CAM_FORMAT_YUV_444_NV42: {
      *scale_w = 0.5;
      *scale_h = 1;
    }

    break;

    default: {
      ISP_HIGH(" Incompatible Format: %d",  format);
    }
      break;
  }
}

/** isp_sub_module_util_get_cfg_mask:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event handle
 *
 *  Gets the module config mask
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_get_cfg_mask(isp_sub_module_t *isp_sub_module,
  mct_event_t *event)
{
  isp_module_enable_LPM_info_t  *enable_bit_info = NULL;
  boolean                   ret = TRUE;
  uint32_t                  count = 0;
  isp_private_event_t      *private_event = NULL;

  if(!isp_sub_module || !event) {
    ISP_ERR("ERROR: cannot get cfg mask %p %p",
      isp_sub_module, event);
    return FALSE;
  }
  ISP_DBG("get cfg mask mod id %d",
    isp_sub_module->hw_module_id);
  private_event = (isp_private_event_t*)event->u.module_event.module_event_data;

  enable_bit_info = (isp_module_enable_LPM_info_t*)private_event->data;
  enable_bit_info->enableLPM = FALSE;

  switch (isp_sub_module->hw_module_id) {
  case ISP_MOD_ABF:
    if (!isp_sub_module->submod_enable)
      enable_bit_info->enableLPM = TRUE;
  case ISP_MOD_BPC:
  case ISP_MOD_CAC:
    enable_bit_info->cfg_info.reconfig_needed =
      isp_sub_module->update_module_bit;

    enable_bit_info->cfg_info.submod_enable[isp_sub_module->hw_module_id]
       = isp_sub_module->submod_enable;

    enable_bit_info->cfg_info.submod_mask[isp_sub_module->hw_module_id] =
      isp_sub_module->submod_enable;

   ISP_DBG("get cfg mask mod id %d enable lpm %d",
    isp_sub_module->hw_module_id, enable_bit_info->enableLPM);

   break;
  default:
    break;
  }

  return TRUE;
}

/** isp_sub_module_util_fill_sensor_roll_off_cfg:
 *
 *  @sensor_out_info: sensor config
 *  @sensor_rolloff_config: sensor rolloffcfg
  *
 *  Fill sensor rolloff config
 *
**/
void isp_sub_module_util_fill_sensor_roll_off_cfg(
  sensor_out_info_t  *sensor_out_info,
  sensor_rolloff_config_t   *sensor_rolloff_config)
{
  sensor_dim_output_t       *dim_output = NULL;
  sensor_request_crop_t     *request_crop = NULL;

  if (!sensor_out_info || !sensor_rolloff_config) {
    ISP_ERR("invalid args");
    return;
  }

  dim_output = &sensor_out_info->dim_output;
  request_crop = &sensor_out_info->request_crop;

  if (sensor_out_info->sensor_rolloff_config.enable == 0) {
    sensor_rolloff_config->enable = TRUE;
    sensor_rolloff_config->full_width = dim_output->width;
    sensor_rolloff_config->full_height = dim_output->height;
    sensor_rolloff_config->output_width = request_crop->last_pixel -
      request_crop->first_pixel + 1;
    sensor_rolloff_config->output_height = request_crop->last_line -
      request_crop->first_line + 1;
    sensor_rolloff_config->offset_x = request_crop->first_pixel;
    sensor_rolloff_config->offset_y = request_crop->first_line;
    sensor_rolloff_config->scale_factor = 1;
    ISP_DBG("full %d %d out %d %d offset %d %d scale %d",
      sensor_rolloff_config->full_width, sensor_rolloff_config->full_height,
      sensor_rolloff_config->output_width,
      sensor_rolloff_config->output_height,
      sensor_rolloff_config->offset_x,
      sensor_rolloff_config->offset_y,
      sensor_rolloff_config->scale_factor);
  }
}

/** isp_sub_module_util_decide_hysterisis:
 *
 *  @isp_sub_module: sub module handle
 *  @aec_ref: AEC reference gain / lux
 *  @mod_enable: sub module enable flag from chromatix
 *  @snapshot_hyst_en_flag: snapshot hysteresis enable flag
 *  @control_type: trigger control type
 *  @hyster_trigger: hysteresis trigger
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_util_decide_hysterisis(
  isp_sub_module_t *isp_sub_module, float aec_ref, int mod_enable,
  int snapshot_hyst_en_flag, tuning_control_type control_type,
  trigger_point_type *hyster_trigger)
{
  boolean               ret            = TRUE;
  float                 hyster_start   = 0.0;
  float                 hyster_end     = 0.0;
  uint32_t              enable         = 2;
  boolean               hyst_en        = TRUE;

  if (!isp_sub_module || !hyster_trigger) {
    ISP_ERR("failed: isp_sub_module %p hyster_trigger %p", isp_sub_module,
      hyster_trigger);
    return FALSE;
  }

  if (isp_sub_module->stream_data.is_snapshot_stream_on ||
     (isp_sub_module->stream_data.is_cb_stream_on &&
      isp_sub_module->zsl_mode)) {
    if (snapshot_hyst_en_flag == 0) {
        enable = mod_enable;
        hyst_en = FALSE;
    }
  }

  if (hyst_en){
    if (control_type == 0) {
      /* lux index based */
      hyster_start = hyster_trigger->lux_index_start;
      hyster_end   = hyster_trigger->lux_index_end;
    } else {
      /* Gain based */
      hyster_start = hyster_trigger->gain_start;
      hyster_end   = hyster_trigger->gain_end;
    }
    if (aec_ref > hyster_end) {
      enable = 1;
    } else if (aec_ref < hyster_start) {
      enable = 0;
    }
  }
  ISP_DBG("enable %d, aec_ref %f, hyster start %f end %f",
      enable, aec_ref, hyster_start, hyster_end);

  switch (enable) {
    case 1: {
      if (isp_sub_module->submod_enable != mod_enable) {
        isp_sub_module->submod_enable = mod_enable;
        isp_sub_module->update_module_bit = TRUE;
      }
    }
    break;
    case 0: {
      if (isp_sub_module->submod_enable == TRUE) {
        isp_sub_module->submod_enable = FALSE;
        isp_sub_module->update_module_bit = TRUE;
      }
    }
    break;
    default: {
      ISP_HIGH("Maintain as before");
    }
    break;
  }

  return TRUE;
}

/** isp_sub_module_util_is_adrc_mod_enable
 *
 *  if ratio is -1 which means ADRC feature is disabled
 *  for that module.
 *
 **/
boolean isp_sub_module_util_is_adrc_mod_enable(float ratio,
  float total_drc_gain)
{
  if (ratio >= 0.0 && ratio <= 1.0 && total_drc_gain >= 1.0) {
    return TRUE;
  }
  return FALSE;
}

/** isp_sub_module_util_is_adrc_gain_set
 *
 *  return status based on DRC_Gain.
 *
 **/
boolean isp_sub_module_util_is_adrc_gain_set(float col_drc_gain ,
  float total_adrc_gain)
{
  if (((F_EQUAL(col_drc_gain, 1.0)) || col_drc_gain > 1.0) &&
      ((F_EQUAL(total_adrc_gain, 1.0)) || total_adrc_gain > 1.0)) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/** isp_sub_module_util_get_lut_index
 *
 *  Return the ADRC knee LUT index.
 *
 **/
uint32_t isp_sub_module_util_get_lut_index(float drc_gain)
{
  if (drc_gain < 1.0f) {
    return 0;
  }

  return MIN((unsigned int)(int)(log(drc_gain) / log(UNIT_GAIN_STEP) + 0.5f),
    (ISP_ADRC_GAIN_TABLE_LENGTH - 1));
}

/** isp_sub_module_util_linear_interpolate
 *
 *  return linear interpolated values.
 *
 **/
float isp_sub_module_util_linear_interpolate(float x,
  unsigned int x_left, unsigned int x_right, float val_left,
  float val_right)
{
    float ratio = 0;
    if (x_right != x_left) {
      ratio = (x - (float)x_left) / (float)(x_right - x_left);
    }
    return ratio * val_right + (1 - ratio) * val_left;
}

/** isp_sub_module_util_linear_interpolate_float
 *
 *  return linear interpolated values.
 *
 **/
float isp_sub_module_util_linear_interpolate_float(float x,
  float x_left, float x_right, float val_left,
  float val_right)
{
    float ratio = 0;
    if (!F_EQUAL(x_left, x_right)) {
      ratio = (x - x_left) / (x_right - x_left);
    }
    return ratio * val_right + (1 - ratio) * val_left;
}


/** isp_sub_module_util_gain_lut_with_drc_gain_weight: convert
 *  knee_gain[], knee_index[] to knee_gain_lut[], with the
 *  knowledge of rate and num_knee.
 *
 *  input: knee_gain[], knee_index[] rate, num_knee,
 *
 *  output : knee_gain_lut[] (and unmodified knee_index[])
 *
 **/
void isp_sub_module_util_gain_lut_with_drc_gain_weight( float *knee_gain_in,
  float *knee_index,unsigned short num_knee,float rate,float *knee_lut_out)
{
    int i;
    float temp_gain;

    /* Generate direct LUT by gain and rate */
    for (i = 0; i < num_knee; i++) {
      temp_gain = (float) pow((double)knee_gain_in[i], (double) rate);
      knee_lut_out[i] = (temp_gain * knee_index[i]);

      ISP_DBG("knee_index[%d] knee_lut_out[%d] %f\n",
              i,knee_index[i],i,knee_lut_out[i]);
    }

  return;
}
/** isp_sub_module_util_find_region_index_spatial
 *
 *  @trigger: trigger handle
 *  @trigger_start: trigger start
 *  @trigger_end: trigger end
 *
 *  find region index
 *
 *  update the region index values.
 **/
int isp_sub_module_util_find_region_index_spatial(float trigger,
  float *trigger_start,float *trigger_end,float *Interp_ratio,
  int *RegionIdxStrt,int *RegionIdxEnd,int total_regions)
{
  int i;
  for (i = 0; i < total_regions; i++) {
    if((trigger_start[i] < trigger_end[i]) ||
       F_EQUAL(trigger_start[i],trigger_end[i])) {
      if (i == (total_regions - 1)) {
        *Interp_ratio = 0.0;
        *RegionIdxStrt = total_regions - 1;
        *RegionIdxEnd = total_regions - 1;
        break;
      }
      if ((trigger < trigger_start[i]) ||
          F_EQUAL(trigger, trigger_start[i])) {
        *RegionIdxStrt = i;
        *RegionIdxEnd = i;
        *Interp_ratio = 0.0;
        break;
      } else if (trigger < trigger_end[i]) {
        *RegionIdxStrt = i;
        *RegionIdxEnd = i + 1;
        *Interp_ratio = (trigger - trigger_start[i]) /
          (trigger_end[i] - trigger_start[i]);
        break;
      }
    }
  }
  return 0;
}

static const uint32_t hw_skip_pattern[] = {
  [RGN_SKIP_PATTERN_0] = 0xFFFF,
  [RGN_SKIP_PATTERN_1] = 0xA5A5,
  [RGN_SKIP_PATTERN_2] = 0xAAAA,
  [RGN_SKIP_PATTERN_3] = 0xA5A1,
  [RGN_SKIP_PATTERN_4] = 0xA149,
  [RGN_SKIP_PATTERN_5] = 0x41A1,
  [RGN_SKIP_PATTERN_6] = 0x1A0A,
  [RGN_SKIP_PATTERN_7] = 0x8421,
  [RGN_SKIP_PATTERN_8] = 0xA0A0,
  [RGN_SKIP_PATTERN_9] = 0x8204,
  [RGN_SKIP_PATTERN_10] = 0x1820,
  [RGN_SKIP_PATTERN_11] = 0x8020,
  [RGN_SKIP_PATTERN_12] = 0x4020,
  [RGN_SKIP_PATTERN_13] = 0x8000,
  [RGN_SKIP_PATTERN_MAX] = 0xFFFF,
};

uint32_t isp_sub_module_util_get_rgn_sample_pattern(
  isp_rgn_skip_pattern skip_pattern) {
  if (skip_pattern > RGN_SKIP_PATTERN_MAX) return 0xFFFF;
  else return hw_skip_pattern[skip_pattern];
}
