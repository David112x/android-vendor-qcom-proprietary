/* template_util.c
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* mctl headers */
#include "mct_module.h"

/* isp headers */
#include "module_template.h"
#include "template_util.h"
#include "isp_log.h"

/** template_util_append_cfg:
 *
 *  @hw_update_list: hw update list
 *  @hw_update: hw update cmd to be enqueued
 *
 *  Enqueue hw_update in hw_update_list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean template_util_append_cfg(struct msm_vfe_cfg_cmd_list **hw_update_list,
  struct msm_vfe_cfg_cmd_list *hw_update)
{
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd_list *list = NULL;

  if (!hw_update) {
    ISP_ERR("failed: %p", hw_update);
    return FALSE;
  }

  list = *hw_update_list;
  if (!list) {
    *hw_update_list = hw_update;
  } else {
    while (list->next) {
      list = list->next;
    }
    list->next = hw_update;
    list->next_size = sizeof(*hw_update);
  }
  return ret;
}

/** template_util_compare_identity:
 *
 *  @data1: identity1
 *  @data2: identity2
 *
 *  Return TRUE if identity matches, FALSE otherwise
 **/
static boolean template_util_compare_identity(void *data1, void *data2)
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

/** template_util_find_port_based_on_identity:
 *
 *  @data1: mct port handle
 *  @data2: identity
 *
 *  Retrieve port based on identity
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean template_util_find_port_based_on_identity(void *data1,
  void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  uint32_t   *identity = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }

  if (mct_list_find_custom(MCT_PORT_CHILDREN(port), identity,
    template_util_compare_identity)) {
    return TRUE;
  }

  return FALSE;
}

/** template_util_get_port_from_module:
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
boolean template_util_get_port_from_module(mct_module_t *module,
  mct_port_t **port, mct_port_direction_t direction, uint32_t identity)
{
  mct_list_t *l_port = NULL;

  if (!module || !port) {
    ISP_ERR("failed: module %p sink_port %p", module, port);
    return FALSE;
  }

  /* Reset output param */
  *port = NULL;

  if ((direction != MCT_PORT_SRC) || (direction != MCT_PORT_SINK)) {
    ISP_ERR("failed: invalid direction %d", direction);
    return FALSE;
  }

  if (direction == MCT_PORT_SINK) {
    l_port = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &identity,
      template_util_find_port_based_on_identity);
    if (!l_port) {
      ISP_ERR("failed: mct_list_find_custom for ide %x", identity);
      return FALSE;
    }
  } else {
    l_port = mct_list_find_custom(MCT_MODULE_SRCPORTS(module), &identity,
      template_util_find_port_based_on_identity);
    if (!l_port) {
      ISP_ERR("failed: mct_list_find_custom for ide %x", identity);
      return FALSE;
    }
  }

  *port = (mct_port_t *)l_port->data;

  return TRUE;
}

/** template_util_compare_identity_from_stream_param:
 *
 *  @data1: stream info handle
 *  @data2: identity
 *
 *  Returns TRUE if identity of stream info matches with
 *  requested identity, FALSE otherwise
 **/
static boolean template_util_compare_identity_from_stream_info(void *data1,
  void *data2)
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

/** template_util_append_stream_info:
 *
 *  @session_param: session param
 *  @stream_info: stream info to be appended
 *
 *  Append stream info handle in session param
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean template_util_append_stream_info(
  template_t *session_param, mct_stream_info_t *stream_info)
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
    stream_info, template_util_compare_identity_from_stream_info);
  if (l_stream_info) {
    ISP_ERR("stream info ide %x already present in list",
      stream_info->identity);
    goto ERROR;
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

/** template_util_append_stream_info:
 *
 *  @session_param: session param
 *  @identity: identity of stream info to be appended
 *
 *  Remove stream info handle in module private's stream info
 *  list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean template_util_remove_stream_info(
  template_t *session_param, uint32_t identity)
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
    &identity, template_util_compare_identity_from_stream_info);
  if (!l_stream_info) {
    ISP_ERR("failed: l_stream_info %p", l_stream_info);
    goto ERROR;
  }

  session_param->l_stream_info = mct_list_remove(session_param->l_stream_info,
    l_stream_info->data);

  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return FALSE;
}

/** template_util_get_stream_info:
 *
 *  @session_param: session params
 *  @stream_info: handle to return stream info
 *
 *  Extract stream info from session param and return it
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean template_util_get_stream_info(template_t *session_param,
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
    &identity, template_util_compare_identity_from_stream_info);
  if (!l_stream_info) {
    ISP_ERR("failed: l_stream_info %p", l_stream_info);
    goto ERROR;
  }

  *stream_info = (mct_stream_info_t *)l_stream_info->data;

  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return FALSE;
}

/** template_util_compare_identity_in_session_param:
 *
 *  @data1: handle to template_t
 *  @data2: handle to session id
 *
 *  Return TRUE if session id in session param matches with
 *  requested session id
 **/
static boolean template_util_compare_identity_in_session_param(
  void *data1, void *data2)
{
  template_t *session_param = (template_t *)data1;
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

/** template_util_get_session_param:
 *
 *  @template: handle to template_priv_t
 *  @session_id: session id for which session param need to
 *             be retrieved
 *  @session_param: handle for session param to be retrived
 *
 *  Retrieve session param from module private
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean template_util_get_session_param(template_priv_t *template_priv,
  unsigned int session_id, template_t **session_param)
{
  mct_list_t *l_session_param = NULL;

  if (!template_priv || !session_param) {
    ISP_ERR("failed: %p %p", template_priv, session_param);
    return FALSE;
  }

  if (!template_priv->l_session_params) {
    ISP_ERR("failed: template->l_session_params %p",
      template_priv->l_session_params);
    return FALSE;
  }

  l_session_param = mct_list_find_custom(template_priv->l_session_params,
    &session_id, template_util_compare_identity_in_session_param);
  if (!l_session_param) {
    ISP_ERR("failed: session_param not found for session id %d", session_id);
    return FALSE;
  }

  *session_param = (template_t *)l_session_param->data;
  return TRUE;
}

/** template_util_create_session_param:
 *
 *  @template: handle to template_priv_t
 *  @session id: session id
 *  @template: handle to return template module
 *
 *  Check whether session param already exist for this
 *  session id. If not, create session param and append it to
 *  the list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean template_util_create_session_param(template_priv_t *template_priv,
  unsigned int session_id, template_t **template)
{
  mct_list_t   *l_session_param = NULL;
  template_t *session_param = NULL;

  if (!template_priv || !template) {
    ISP_ERR("failed: template %p %p", template_priv, template);
    return FALSE;
  }

  /* Find whether session_param list already has this session id */
  if (template_priv->l_session_params) {
    l_session_param = mct_list_find_custom(template_priv->l_session_params,
      &session_id, template_util_compare_identity_in_session_param);
    if (l_session_param) {
      ISP_ERR("failed: session id %d already present", session_id);
      return FALSE;
    }
  }

  /* Create new session param instance */
  session_param =
    (template_t *)CAM_MALLOC(sizeof(template_t));
  if (!session_param) {
    ISP_ERR("failed: session_param %p", session_param);
    return FALSE;
  }

  memset(session_param, 0, sizeof(*session_param));
  session_param->session_id = session_id;
  pthread_mutex_init(&session_param->mutex, NULL);

  /* Append it in session param list */
  l_session_param = mct_list_append(template_priv->l_session_params,
    session_param, NULL, NULL);
  if (!l_session_param) {
    ISP_ERR("failed: l_session_param %p", l_session_param);
    goto ERROR;
  }
  template_priv->l_session_params = l_session_param;
  *template = session_param;
  return TRUE;

ERROR:
  free(session_param);
  return FALSE;
}

/** template_util_remove_session_param:
 *
 *  @template: handle to template_priv_t
 *  @session id: session id
 *
 *  Remove session param from module private
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean template_util_remove_session_param(template_priv_t *template_priv,
  unsigned int session_id)
{
  mct_list_t               *l_session_param = NULL;
  template_t *session_param = NULL;

  if (!template_priv) {
    ISP_ERR("failed: template %p", template_priv);
    return FALSE;
  }

  if (!template_priv->l_session_params) {
    ISP_ERR("failed: template->l_session_params %p",
      template_priv->l_session_params);
    return FALSE;
  }

  l_session_param = mct_list_find_custom(template_priv->l_session_params,
    &session_id, template_util_compare_identity_in_session_param);
  if (!l_session_param) {
    ISP_ERR("failed: l_session_param %p", l_session_param);
    return FALSE;
  }

  if (!l_session_param->data) {
    ISP_ERR("failed: l_session_param->data %p", l_session_param->data);
    return FALSE;
  }

  session_param = (template_t *)l_session_param->data;
  pthread_mutex_destroy(&session_param->mutex);
  template_priv->l_session_params = mct_list_remove(
    template_priv->l_session_params, l_session_param->data);

  free(l_session_param->data);
  return TRUE;
}
