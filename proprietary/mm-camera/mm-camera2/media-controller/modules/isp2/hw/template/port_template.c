/* port_template.c
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <stdio.h>
#include <pthread.h>

/* mctl headers */
#include "media_controller.h"
#include "mct_stream.h"

/* ISP headers*/
#include "isp_log.h"
#include "isp_defs.h"
#include "module_template.h"
#include "port_template.h"
#include "template_util.h"

/** port_template_update_streaming_mode:
 *
 *  @data1: mct stream info handle
 *  @data2: streaming mode mask
 *
 *  Append streaming mode of stream info in streaming mode
 *  mask
 *
 *  Return TRUE
 **/
static boolean port_template_update_streaming_mode(void *data1, void *data2)
{
  mct_stream_info_t *stream_info = (mct_stream_info_t *)data1;
  uint32_t          *streaming_mode_mask = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return TRUE;
  }

  ISP_APPEND_STREAMING_MODE(*streaming_mode_mask, stream_info->streaming_mode);
  return TRUE;
}

/** port_template_forward_event:
 *
 *  @port: port handle
 *  @event: event to be forwarded
 *
 *  Forward event based on port direction and event direction
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_template_forward_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean                 ret = TRUE;
  port_template_data_t *port_data = NULL;

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

  port_data = MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    return FALSE;
  }

  if (!port_data->int_peer_port) {
    return TRUE;
  }

  ret = mct_port_send_event_to_peer(port_data->int_peer_port, event);
  if (ret == FALSE) {
    if (event->type == MCT_EVENT_CONTROL_CMD) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        event->u.ctrl_event.type);
    } else {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        event->u.module_event.type);
    }
  }
  return ret;
}

/** port_template_handle_streamon:
 *
 *  @template: template handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Handle STREAM ON event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_template_handle_streamon(template_t *template,
  mct_port_t *port, mct_event_t *event)
{
  boolean ret = TRUE;

  if (!template || !port || !event) {
    ISP_ERR("failed: %p %p %p", template, port, event);
    return FALSE;
  }

  /* Forward event to FRAME port */
  ret = port_template_forward_event(port, event);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_port_send_event_to_peer event");
  }

  /* Update streaming mode mask in module private */
  mct_list_traverse(template->l_stream_info,
    port_template_update_streaming_mode, &template->streaming_mode_mask);

  /* Increase stream on count */
  if (!(template->stream_on_count++)) {
    template->state = ISP_STATE_STREAMING;
  }

  return ret;
}

/** port_template_handle_streamoff:
 *
 *  @template: template handle
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Handle STREAM OFF event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_template_handle_streamoff(template_t *template,
  mct_port_t *port, mct_event_t *event)
{
  boolean       ret = TRUE;

  if (!template || !port || !event) {
    ISP_ERR("failed: template %p port %p event %p", template, port, event);
    return FALSE;
  }

  /* Forward event to FRAME port */
  ret = port_template_forward_event(port, event);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_port_send_event_to_peer");
    return ret;
  }

  PTHREAD_MUTEX_LOCK(&template->mutex);

  /* Reset streaming mode */
  ISP_RESET_STREAMING_MODE_MASK(template->streaming_mode_mask);

  if (!(--template->stream_on_count)) {
    template->state = ISP_STATE_IDLE;
  }

  PTHREAD_MUTEX_UNLOCK(&template->mutex);
  return ret;
}

/** port_template_process_downstream_ctrl_event:
 *
 *  @module: module handle
 *  @port: port handle
 *  @template: session param
 *  @event: event to be processed
 *
 *  Handle downstream ctrl event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_template_process_downstream_ctrl_event(
  mct_module_t *module, mct_port_t *port,
  template_t *template, mct_event_t *event)
{
  boolean              ret = TRUE;
  mct_event_control_t *ctrl_event = NULL;

  if (!module || !port || !template || !event) {
    ISP_ERR("failed: %p %p %p %p", module, port, template, event);
    return FALSE;
  }

  /* Extract ctrl event */
  ctrl_event = &event->u.ctrl_event;

  switch (ctrl_event->type) {
  case MCT_EVENT_CONTROL_STREAMON:
    ret = port_template_handle_streamon(template, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_template_handle_streamon");
    }
    break;

  case MCT_EVENT_CONTROL_STREAMOFF:
    ret = port_template_handle_streamoff(template, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_template_handle_streamon");
    }
    break;

  default:
    /* Forward event to FRAME port */
    ret = port_template_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;
  }
  return ret;
}

/** port_template_process_downstream_module_event:
 *
 *  @module: module handle
 *  @port: port handle
 *  @template: session param
 *  @event: event to be processed
 *
 *  Handle downstream ctrl event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_template_process_downstream_module_event(
  mct_module_t *module, mct_port_t *port,
  template_t *template, mct_event_t *event)
{
  boolean              ret = TRUE;
  mct_event_module_t  *module_event = NULL;

  if (!module || !port || !template || !event) {
    ISP_ERR("failed: %p %p %p %p", module, port, template, event);
    return FALSE;
  }

  /* Extract ctrl event */
  module_event = &event->u.module_event;

  switch (module_event->type) {
  case MCT_EVENT_MODULE_SET_CHROMATIX_PTR:
    /* Forward event */
    ret = port_template_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;

  case MCT_EVENT_MODULE_STATS_AEC_UPDATE:
    /* Forward event */
    ret = port_template_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;

  case MCT_EVENT_MODULE_STATS_AWB_UPDATE:
    /* Forward event */
    ret = port_template_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;

  case MCT_EVENT_MODULE_ISP_PRIVATE_EVENT:
    /* Forward event */
    ret = port_template_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;

  default:
    /* Forward event */
    ret = port_template_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;
  }
  return ret;
}

/** port_template_process_downstream_event:
 *
 *  @module: module handle
 *  @port: port on which this event arrived
 *  @template: session param
 *  @event: event to be handled
 *
 *  Handle downstream event
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean port_template_process_downstream_event(mct_module_t *module,
  mct_port_t *port, template_t *template, mct_event_t *event)
{
  boolean ret = TRUE;

  if (!module || !port || !template || !event) {
    ISP_ERR("failed: %p %p %p %p", module, port, template, event);
    return FALSE;
  }

  if ((event->type != MCT_EVENT_CONTROL_CMD) &&
      (event->type != MCT_EVENT_MODULE_EVENT)) {
    ISP_ERR("failed: invalid event type %d", event->type);
    return FALSE;
  }

  if (event->type == MCT_EVENT_CONTROL_CMD) {
    ret = port_template_process_downstream_ctrl_event(module, port,
      template, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_template_process_downstream_ctrl_event");
    }
  } else if (event->type == MCT_EVENT_MODULE_EVENT) {
    ret = port_template_process_downstream_module_event(module, port,
      template, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_template_process_downstream_module_event");
    }
  }
  return ret;

ERROR:
  return ret;
}

/** port_template_process_upstream_event:
 *
 *  @module: module handle
 *  @port: port on which this event arrived
 *  @template: session param
 *  @event: event to be handled
 *
 *  Handle downstream event
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean port_template_process_upstream_event(mct_module_t *module,
  mct_port_t *port, template_t *template, mct_event_t *event)
{
  boolean             ret = TRUE;
  mct_event_module_t *module_event = NULL;

  if (!module || !port || !template || !event) {
    ISP_ERR("failed: %p %p %p %p", module, port, template, event);
    return FALSE;
  }

  if (!port || !event) {
    ISP_ERR("failed: port %p event %p", port, event);
    return FALSE;
  }

  if (event->type != MCT_EVENT_MODULE_EVENT) {
    ISP_ERR("failed: invalid event type %d", event->type);
    return FALSE;
  }

  module_event = &event->u.module_event;

  switch (module_event->type) {
  default:
    /* Forward event to upstream FRAME port */
    ret = port_template_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;
  }

  return ret;
}

/** port_template_event:
 *
 *  @port: port handle
 *  @event: event to be processed
 *
 *  Handle ISP event based on direction
 *
 *  Returns TRUE on success and FALSE on failure */
static boolean port_template_event(mct_port_t *port, mct_event_t *event)
{
  boolean            ret = TRUE;
  mct_module_t      *module = NULL;
  template_priv_t *template_priv = NULL;
  template_t      *template = NULL;
  mct_stream_info_t *stream_info = NULL;

  if (!port || !event) {
    ISP_ERR("failed: port %p event %p", port, event);
    return FALSE;
  }

  if ((MCT_PORT_IS_SINK(port) != TRUE) && (MCT_PORT_IS_SRC(port) != TRUE)) {
    ISP_ERR("failed: invalid port direction %d", MCT_PORT_DIRECTION(port));
    return FALSE;
  }

  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  template_priv = (template_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!template_priv) {
    ISP_ERR("failed: template_priv %p", template_priv);
    return FALSE;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));

  /* Get session param */
  ret = template_util_get_session_param(template_priv,
   ISP_GET_SESSION_ID(event->identity), &template);
  if ((ret == FALSE) || !template) {
    ISP_ERR("failed: template_get_session_param %d %p", ret, template);
    goto ERROR;
  }

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  /* Check whether current identity is present in available streams */
  ret = template_util_get_stream_info(template, event->identity,
    &stream_info);
  if ((ret == FALSE) || !stream_info) {
    ISP_ERR("failed: template_util_get_stream_info %d %p", ret, stream_info);
    goto ERROR;
  }

  if (event->direction == MCT_EVENT_DOWNSTREAM) {
    ret = port_template_process_downstream_event(module, port, template,
      event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_template_process_downstream_event");
    }
  } else {
    ret = port_template_process_upstream_event(module, port, template, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_template_process_upstream_event");
    }
  }

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return ret;
}

/** port_template_ext_link:
 *
 *  @identity: identity of stream
 *  @port: port handle
 *  @peer: peer handle
 *
 *  Handle ext link
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean port_template_ext_link(unsigned int identity, mct_port_t* port,
  mct_port_t *peer)
{
  boolean                 ret = TRUE;
  mct_module_t           *module = NULL;
  mct_port_t             *sink_port = NULL;
  port_template_data_t *port_data = NULL;
  port_template_data_t *port_sink_data = NULL;

  if (!port || !peer) {
    ISP_ERR("failed: port %p peer %p", port, peer);
    return FALSE;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));

  if (!MCT_PORT_PEER(port)) {
    MCT_PORT_PEER(port) = peer;
  } else { /*the link has already been established*/
    if ((MCT_PORT_PEER(port) != peer))
    goto ERROR;
  }
  port_data = MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    goto ERROR;
  }

  if (MCT_PORT_IS_SRC(port) == TRUE) {

    /* Find sink port with same identity */
    ret = template_util_get_port_from_module(
      (mct_module_t *)(MCT_PORT_PARENT(port)->data), &sink_port, MCT_PORT_SINK,
      identity);
    if ((ret == FALSE) || !sink_port) {
      ISP_ERR("failed: template_util_get_port_from_module ret %d port %p",
        ret, sink_port);
      goto ERROR;
    }

    if (port_data->int_peer_port &&
       (port_data->int_peer_port != sink_port)) {
      ISP_ERR("failed: source port linked with different sink port");
      goto ERROR;
    } else {
      port_sink_data = MCT_OBJECT_PRIVATE(sink_port);
      if (!port_sink_data) {
        ISP_ERR("failed: port_data %p", port_sink_data);
        goto ERROR;
      }

      /* Fill internal peer */
      port_data->int_peer_port = sink_port;
      port_sink_data->int_peer_port = port;
    }
  }

  port_data->num_streams++;
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return TRUE;

ERROR:
  ISP_ERR("failed: port_template_ext_link");
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return FALSE;
}

/** port_template_unlink:
 *
 *  @identity: identity of stream
 *  @port: port handle
 *  @peer: peer handle
 *
 *  Handle unlink func
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static void port_template_unlink(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  boolean                 ret = TRUE;
  mct_module_t           *module = NULL;
  mct_port_t             *sink_port = NULL;
  port_template_data_t *port_data = NULL;
  port_template_data_t *port_sink_data = NULL;

  if (!port || !peer) {
    ISP_ERR("failed: port %p peer %p", port, peer);
    return;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));

  port_data = MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    goto ERROR;
  }

  if (MCT_PORT_PEER(port) != peer) {
    ISP_ERR("failed");
    goto ERROR;
  }


  if (!(--port_data->num_streams)) {
    MCT_PORT_PEER(port) = NULL;
    if (MCT_PORT_IS_SRC(port) == TRUE) {
      /* Find sink port with same identity */
      ret = template_util_get_port_from_module(
        (mct_module_t *)(MCT_PORT_PARENT(port)->data), &sink_port,
        MCT_PORT_SINK, identity);
      if ((ret == FALSE) || !sink_port) {
        ISP_ERR("failed: template_util_get_port_from_module ret %d port %p",
          ret, sink_port);
        goto ERROR;
      }

      if (port_data->int_peer_port &&
         (port_data->int_peer_port != sink_port)) {
        ISP_ERR("failed: source port linked with different sink port");
        goto ERROR;
      } else {
        port_sink_data = MCT_OBJECT_PRIVATE(sink_port);
        if (!port_sink_data) {
          ISP_ERR("failed: port_data %p", port_sink_data);
          goto ERROR;
        }

        /* Clear internal peer */
        port_data->int_peer_port = NULL;
        port_sink_data->int_peer_port = NULL;
      }
    }
  }

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return;

ERROR:
  ISP_ERR("failed: port_template_unlink");
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return;
}

/** port_template_check_caps_reserve:
 *
 *  @port: port handle
 *  @peer_caps: peer caps handle
 *  @stream_info: handle to stream_info
 *
 *  Handle caps reserve
 *
 *  Return TRUE if this port is reserved, FALSE otherwise
 **/
static boolean port_template_check_caps_reserve(mct_port_t *port,
  void *peer_caps, void *stream_info)
{
  boolean                   ret = FALSE;
  mct_stream_info_t        *mct_stream_info = (mct_stream_info_t *)stream_info;
  port_template_data_t   *port_data = NULL;
  uint32_t                  session_id = 0;
  mct_module_t             *module = NULL;
  template_priv_t        *template_priv = NULL;
  template_t             *template = NULL;

  if (!port || !stream_info) {
    ISP_ERR("failed: port %p stream_info %p", port, stream_info);
    return FALSE;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));

  /* Check whether */
  port_data = MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    goto ERROR;
  }

  session_id = ISP_GET_SESSION_ID(mct_stream_info->identity);

  /* Check whether this port is already reserved for same session */
  if ((port_data->is_reserved == TRUE) &&
      (port_data->session_id != session_id)) {
    goto ERROR;
  }

  /* Extract module private */
  template_priv = (template_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!template_priv) {
    ISP_ERR("failed: template %p", template_priv);
    goto ERROR;
  }

  /* Get session param */
  ret = template_util_get_session_param(template_priv, session_id, &template);
  if ((ret == FALSE) || !template) {
    ISP_ERR("failed: template_get_session_param %d %p", ret, template);
    goto ERROR;
  }

  ret = template_util_append_stream_info(template, mct_stream_info);
  if (ret == FALSE) {
    ISP_ERR("failed: template_util_append_stream_info");
    goto ERROR;
  }

  port_data->is_reserved = TRUE;
  port_data->session_id = session_id;

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  return TRUE;
ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return FALSE;
}

/** port_template_check_caps_unreserve:
 *
 *  @port: port handle
 *  @identity: identity of stream
 *
 *  Handle caps unreserve
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_template_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  boolean                   ret = TRUE;
  port_template_data_t   *port_data = NULL;
  mct_module_t             *module = NULL;
  template_priv_t        *template_priv = NULL;
  template_t             *template = NULL;
  uint32_t                  session_id = 0;

  if (!port) {
    ISP_ERR("failed: port %p", port);
    return FALSE;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));

  session_id = ISP_GET_SESSION_ID(identity);
  port_data = MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    goto ERROR;
  }

  /* Extract module private */
  template_priv = (template_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!template_priv) {
    ISP_ERR("failed: template_priv %p", template_priv);
    goto ERROR;
  }

  /* Get session param */
  ret = template_util_get_session_param(template_priv, session_id, &template);
  if ((ret == FALSE) || !template) {
    ISP_ERR("failed: template_get_session_param %d %p", ret, template);
    goto ERROR;
  }

  ret = template_util_remove_stream_info(template, identity);
  if (ret == FALSE) {
    ISP_ERR("failed: template_util_append_stream_info");
  }

  if (!(--port_data->num_streams)) {
    port_data->session_id = 0;
    MCT_PORT_PEER(port) = NULL;
    port_data->is_reserved = FALSE;
  }

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  return TRUE;
ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return FALSE;
}

/** port_template_delete_port:
 *
 *  @data: handle to port to be deleted
 *
 *  @user_data: handle to module
 *
 *  1) Delete port private
 *
 *  2) Delete port from module's list
 *
 *  3) Delete port
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_template_delete_port(void *data, void *user_data)
{
  boolean          ret = TRUE;
  mct_module_t    *module = (mct_module_t *)user_data;
  mct_port_t      *port = (mct_port_t *)data;
  port_template_data_t *port_template_data = NULL;

  if (!data || !user_data) {
    ISP_ERR("failed: data %p user_data %p", data, user_data);
    return TRUE;
  }

  port_template_data = MCT_OBJECT_PRIVATE(port);
  if (port_template_data) {
   free(port_template_data);
  }

  ret = mct_module_remove_port(module, port);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_module_remove_port");
  }

  mct_port_destroy(port);

  return TRUE;
}

/** port_template_return_first
 *
 *  @data1: handle to list data
 *
 *  @data2: private data
 *
 *  Return TRUE if first element in list is present
 **/
static boolean port_template_return_first(void *data1, void *data2)
{
  if (data1) {
    return TRUE;
  }
  return FALSE;
}

/** port_template_delete_ports:
 *
 *  @port: port handle
 *
 *  Delete port and its private data
 *
 *  Returns: void
 **/
void port_template_delete_ports(mct_module_t *module)
{
  mct_list_t *l_port = NULL;
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  do {
    l_port = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), module,
      port_template_return_first);
    if (l_port)
      port_template_delete_port(l_port->data, module);
  } while (l_port);
  do {
    l_port = mct_list_find_custom(MCT_MODULE_SRCPORTS(module), module,
      port_template_return_first);
    if (l_port)
      port_template_delete_port(l_port->data, module);
  } while (l_port);

}

/** port_template_overwrite_funcs
 *
 *   @port: mct port instance
 *
 *   @private_data: port private
 *
 *  Assign mct port function pointers to respective template port
 *  functions
 *
 *  Returns nothing
 **/
static void port_template_overwrite_funcs(mct_port_t *port,
  void *private_data)
{
  if (!port) {
    ISP_ERR("failed: port %p", port);
    return;
  }

  mct_port_set_event_func(port, port_template_event);
  mct_port_set_ext_link_func(port, port_template_ext_link);
  mct_port_set_unlink_func(port, port_template_unlink);
  mct_port_set_check_caps_reserve_func(port,
    port_template_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port,
    port_template_check_caps_unreserve);
  MCT_OBJECT_PRIVATE(port) = private_data;
}

/** port_template_create_port
 *
 *   @direction: direction of port
 *
 *  Creates port and add it to parent
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean port_template_create_port(mct_module_t *module,
  mct_port_direction_t direction,
  mct_port_caps_type_t caps_type)
{
  boolean          ret = TRUE;
  port_template_data_t *port_template_data = NULL;
  char             port_name[32];
  mct_port_t      *mct_port = NULL;

  if (!module ||
    ((direction != MCT_PORT_SINK) && (direction != MCT_PORT_SRC)) ||
    ((caps_type != MCT_PORT_CAPS_STATS) &&
     (caps_type != MCT_PORT_CAPS_FRAME))) {
    ISP_ERR("failed: module %p direction %d caps_type %d", module, direction,
      caps_type);
    return FALSE;
  }

  port_template_data =
    (port_template_data_t *)CAM_MALLOC(sizeof(port_template_data_t));
  if (!port_template_data) {
    ISP_ERR("failed: CAM_MALLOC");
    return FALSE;
  }
  memset(port_template_data, 0, sizeof(*port_template_data));

  if (direction == MCT_PORT_SINK) {
    snprintf(port_name, sizeof(port_name), "template_sink");
  } else if (direction == MCT_PORT_SRC) {
    snprintf(port_name, sizeof(port_name), "template_src");
  }

  mct_port = mct_port_create(port_name);
  if (!mct_port) {
    ISP_ERR("failed: mct_port_create");
    ret = FALSE;
    goto ERROR;
  }

  mct_port->direction = direction;
  ret = mct_module_add_port(module, mct_port);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_module_add_port");
    goto ERROR;
  }

  mct_port->caps.port_caps_type = caps_type;
  port_template_overwrite_funcs(mct_port, (void *)port_template_data);

  return ret;
ERROR:
  free(port_template_data);
  return ret;
}

/** port_template_create:
 *
 *  @module: module handle
 *
 *  1) Create one sink port per ISP
 *
 *  2) Create one source port for STATS
 *
 *  3) Create one source port for FRAME
 **/
boolean port_template_create(mct_module_t *module)
{
  boolean  ret = TRUE;

  if (!module) {
    ISP_ERR("failed: invalid params %p", module);
    return FALSE;
  }

  /* Create sink port */
  ret = port_template_create_port(module, MCT_PORT_SINK, MCT_PORT_CAPS_FRAME);
  if (ret == FALSE) {
    ISP_ERR("failed: port_template_create_port");
    goto ERROR;
  }

  /* Create source port */
  ret = port_template_create_port(module, MCT_PORT_SRC, MCT_PORT_CAPS_FRAME);
  if (ret == FALSE) {
    ISP_ERR("failed: port_template_create_port");
    goto ERROR;
  }

  return ret;

ERROR:
  ISP_ERR("failed: port_template_create");
  port_template_delete_ports(module);
  return ret;
}
