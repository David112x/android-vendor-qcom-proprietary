/*============================================================================

Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "eztune_diagnostics.h"
#include "cpp_port.h"
#include "cpp_module.h"
#include "camera_dbg.h"
#include "cpp_log.h"

mct_port_t *cpp_port_create(const char* name, mct_port_direction_t dir)
{
  mct_port_t  *port;
  cpp_port_data_t* port_data;
  uint32_t i;
  port = mct_port_create(name);

  if(!port) {
    CPP_ERR("invalid port, failed");
    goto port_create_error;
  }
  port->direction = dir;
  port->check_caps_reserve = cpp_port_check_caps_reserve;
  port->check_caps_unreserve = cpp_port_check_caps_unreserve;
  port->ext_link = cpp_port_ext_link_func;
  port->un_link = cpp_port_ext_unlink_func;
  port->event_func = cpp_port_event_func;
  port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;

  port_data = (cpp_port_data_t*) CAM_MALLOC(sizeof(cpp_port_data_t));
  if(!port_data) {
    CPP_ERR("invalid port_data, failed");
    goto port_data_error;
  }
  memset(port_data, 0x00, sizeof(cpp_port_data_t));
  for(i=0; i<CPP_MAX_STREAMS_PER_PORT; i++) {
    //port_data->port_state[i] = CPP_PORT_STATE_UNRESERVED;
    port_data->stream_data[i].port_state = CPP_PORT_STATE_UNRESERVED;
  }
  port_data->port_type = CPP_PORT_TYPE_INVALID;
  port_data->num_streams = 0;
  MCT_OBJECT_PRIVATE(port) = port_data;
  return port;

port_data_error:
  mct_port_destroy(port);
port_create_error:
  return NULL;
}
void cpp_port_destroy(mct_port_t *port)
{
  if(!port) {
    return;
  }
  free(MCT_OBJECT_PRIVATE(port));
  mct_port_destroy(port);
}

static boolean cpp_port_check_caps_reserve(mct_port_t *port, void *peer_caps,
  void *info)
{
  if(!port || !info) {
    CPP_ERR("failed, port=%p, info=%p", port, info);
    return FALSE;
  }
  mct_stream_info_t *stream_info = (mct_stream_info_t *)info;
  mct_port_caps_t *port_caps = (mct_port_caps_t *)(&(port->caps));
  cpp_port_data_t *port_data = MCT_OBJECT_PRIVATE(port);
  uint32_t identity = stream_info->identity;
  uint32_t session_id, stream_id;
  int32_t rc;
  uint32_t i;
  mct_module_t* module;
  session_id = CPP_GET_SESSION_ID(identity);
  stream_id = CPP_GET_STREAM_ID(identity);
  CPP_DBG("identity=0x%x\n", identity);

  module = (mct_module_t*)(MCT_PORT_PARENT(port)->data);

  if(port_data->num_streams >= CPP_MAX_STREAMS_PER_PORT) {
    CPP_ERR("failed. max streams reached, num=%d",
      port_data->num_streams);
    return FALSE;
  }
  if(port->direction == MCT_PORT_SINK) {
     /* TODO: For Offline/peerless this is not true because peer caps is NULL */
#if 0
    if(!peer_caps) {
      CPP_ERR("failed, peear_caps=%p", peer_caps);
      return FALSE;
    }
#endif
    /* TODO: Add more peer caps checking logic */
    mct_port_caps_t *peer_port_caps = (mct_port_caps_t *)peer_caps;
    if(peer_caps) {
      if(port_caps->port_caps_type != peer_port_caps->port_caps_type) {
        return FALSE;
      }
    }
  } else if (port->direction == MCT_PORT_SRC && (port_data->num_streams)) {
     if (cpp_port_decide_src(module,port,identity) == FALSE){
        CPP_DBG("return err identity=0x%x\n", identity);
        return FALSE;
     }
  }

  switch(port_data->port_type) {
  case CPP_PORT_TYPE_INVALID: {
    port_data->num_streams = 0;
    if(stream_info->streaming_mode == CAM_STREAMING_MODE_CONTINUOUS) {
      port_data->port_type = CPP_PORT_TYPE_STREAMING;
    } else {
      port_data->port_type = CPP_PORT_TYPE_BURST;
    }
    port_data->session_id = session_id;
    break;
  }
  case CPP_PORT_TYPE_STREAMING:
  case CPP_PORT_TYPE_BURST: {
    if(port_data->session_id != session_id) {
      CPP_DBG("info: session id doesn't match");
      return FALSE;
    }
    break;
  }
  default:
    CPP_ERR("failed, bad port_type=%d\n", port_data->port_type);
    return FALSE;
  }

  if ((session_id >= CPP_MODULE_MAX_SESSIONS) ||
    (stream_id >= CPP_MODULE_MAX_STREAMS)) {
    CPP_ERR("Exceeding the port map entries, session:%d, stream:%d",
      session_id, stream_id);
    return FALSE;
  }

  /* reserve the port for this stream */
  for(i=0; i<CPP_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state == CPP_PORT_STATE_UNRESERVED) {
      port_data->stream_data[i].port_state = CPP_PORT_STATE_RESERVED;
      port_data->stream_data[i].identity = identity;
      port_data->stream_data[i].streaming_mode = stream_info->streaming_mode;
      port_data->num_streams++;
      CPP_HIGH("identity=0x%x, reserved port %p\n", identity,port);
      break;
    }
  }
  if(i == CPP_MAX_STREAMS_PER_PORT) {
    CPP_ERR("failed, unexpected error!");
    return FALSE;
  }
  /* notify the parent module about this new stream, once for the sink port */
  if(port->direction == MCT_PORT_SINK) {
    rc = cpp_module_notify_add_stream(module, port, stream_info);
    if(rc < 0) {
      CPP_ERR("failed, unexpected error!");
      return FALSE;
    }
    cpp_module_port_mapping(module, MCT_PORT_SRC, NULL, identity);
    cpp_module_port_mapping(module, MCT_PORT_SINK, port, identity);
  } else {
    cpp_module_port_mapping(module, MCT_PORT_SRC, port, identity);
  }
  return TRUE;
}

static boolean cpp_port_check_caps_unreserve(mct_port_t *port,
  uint32_t identity)
{
  mct_module_t* module;
  if(!port) {
    CPP_ERR("invalid port, failed\n");
    return FALSE;
  }
  CPP_HIGH("identity=0x%x\n", identity);
  cpp_port_data_t *port_data = (cpp_port_data_t *) MCT_OBJECT_PRIVATE(port);
  uint32_t i;
  int32_t  rc;
  for(i=0; i<CPP_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state == CPP_PORT_STATE_RESERVED &&
       port_data->stream_data[i].identity == identity) {
      port_data->stream_data[i].port_state = CPP_PORT_STATE_UNRESERVED;
      port_data->num_streams--;
      CPP_HIGH("identity=0x%x, unreserved\n", identity);
      if(port_data->num_streams == 0) {
        port_data->port_type = CPP_PORT_TYPE_INVALID;
        port_data->session_id = 0x00;
      }
      break;
    }
  }
  if(i == CPP_MAX_STREAMS_PER_PORT) {
    CPP_ERR("can't find matching identity, unexpected !!");
    return FALSE;
  }

  module = (mct_module_t*)(MCT_PORT_PARENT(port)->data);
  /* notify the parent module about removal of this stream,
     once for the sink port */
  if(port->direction == MCT_PORT_SINK) {
    rc = cpp_module_notify_remove_stream(module, identity);
    if(rc < 0) {
      CPP_ERR("failed, unexpected error!");
      return FALSE;
    }
    cpp_module_port_mapping(module, MCT_PORT_SINK, NULL, identity);
  } else {
    cpp_module_port_mapping(module, MCT_PORT_SRC, NULL, identity);
  }
  return TRUE;
}

static boolean cpp_port_ext_link_func(uint32_t identity,
  mct_port_t* port, mct_port_t *peer)
{
  if(!port || !peer) {
    CPP_ERR("failed, port=%p, peer=%p", port, peer);
    return FALSE;
  }
  CPP_LOW("E");
  MCT_OBJECT_LOCK(port);

  if (MCT_PORT_PEER(port) && (MCT_PORT_PEER(port) != peer)) {
    CPP_ERR("error old_peer:%s, new_peer:%s\n",
      MCT_OBJECT_NAME(MCT_PORT_PEER(port)), MCT_OBJECT_NAME(peer));
    MCT_OBJECT_UNLOCK(port);
    return FALSE;
  }

  cpp_port_data_t* port_data = (cpp_port_data_t *)MCT_OBJECT_PRIVATE(port);
  uint32_t i;
  for(i=0; i<CPP_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state == CPP_PORT_STATE_RESERVED &&
        port_data->stream_data[i].identity == identity) {
      port_data->stream_data[i].port_state = CPP_PORT_STATE_LINKED;
      if (MCT_OBJECT_REFCOUNT(port) == 0) {
        MCT_PORT_PEER(port) = peer;
      }
      MCT_OBJECT_REFCOUNT(port) += 1;
      MCT_OBJECT_UNLOCK(port);
      CPP_LOW("X");
      return TRUE;
    }
  }
  MCT_OBJECT_UNLOCK(port);
  CPP_LOW("X");
  return FALSE;
}


static void cpp_port_ext_unlink_func(uint32_t identity,
  mct_port_t *port, mct_port_t *peer)
{
  if(!port || !peer) {
    CPP_ERR("failed, port=%p, peer=%p", port, peer);
    return;
  }

  MCT_OBJECT_LOCK(port);
  if (MCT_PORT_PEER(port) != peer) {
    CPP_ERR("failed peer:%p, unlink_peer:%p\n", MCT_PORT_PEER(port), peer);
    MCT_OBJECT_UNLOCK(port);
    return;
  }

  if (MCT_OBJECT_REFCOUNT(port) == 0) {
    CPP_ERR("failed zero refcount on port\n");
    MCT_OBJECT_UNLOCK(port);
    return;
  }

  cpp_port_data_t* port_data = (cpp_port_data_t *)MCT_OBJECT_PRIVATE(port);
  uint32_t i;
  for(i=0; i<CPP_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state == CPP_PORT_STATE_LINKED &&
        port_data->stream_data[i].identity == identity) {
      port_data->stream_data[i].port_state = CPP_PORT_STATE_RESERVED;
      MCT_OBJECT_REFCOUNT(port) -= 1;
      if (MCT_OBJECT_REFCOUNT(port) == 0) {
        MCT_PORT_PEER(port) = NULL;
      }
      MCT_OBJECT_UNLOCK(port);
      return;
    }
  }
  MCT_OBJECT_UNLOCK(port);
  CPP_ERR("failed");
  return;
}

static boolean cpp_port_event_func(mct_port_t *port,
                                   mct_event_t *event)
{
  int32_t rc = -EINVAL;

  if(!port || !event) {
    CPP_ERR("failed, port=%p, event=%p", port, event);
    return FALSE;
  }
  mct_module_t *module;
  mct_list_t *templist = (mct_list_t*)MCT_PORT_PARENT(port);
  module = (mct_module_t*)(templist->data);

  switch(port->direction) {
  case MCT_PORT_SRC:
    rc = cpp_module_process_upstream_event(module, event);
    break;
  case MCT_PORT_SINK:
    rc = cpp_module_process_downstream_event(module, event);
    break;
  default:
    CPP_ERR("failed, bad port->direction=%d", port->direction);
  }
  return rc == 0 ? TRUE : FALSE;
}

/* cpp_port_get_linked_identity:
 *
 *  finds another identity mapped on the port which is not equal to @identity
 **/
int32_t cpp_port_get_linked_info(cpp_module_ctrl_t *ctrl,
  mct_port_t *port, uint32_t identity, cpp_module_link_info_t *linked_info)
{
  uint32_t i = 0;
  int rc = 0;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_port_data_t *port_data = NULL;

  if(!port || !linked_info) {
    CPP_ERR("failed, port=%p, linked_info=%p\n", port, linked_info);
    return -EINVAL;
  }

  port_data = (cpp_port_data_t *) MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    CPP_ERR("invalid port data, failed\n");
    return -EFAULT;
  }

  if(port_data->num_streams > 0) {
    for (i = 0; i < CPP_MAX_STREAMS_PER_PORT; i++) {
      if (port_data->stream_data[i].port_state != CPP_PORT_STATE_UNRESERVED
          && port_data->stream_data[i].identity != identity) {
        rc = cpp_module_get_params_for_identity(ctrl,
          port_data->stream_data[i].identity, &session_params, &stream_params);
        if (!stream_params) {
          CPP_ERR("failed, identity %x\n", port_data->stream_data[i].identity);
          return -EINVAL;
        }

        linked_info->linked_stream_params[linked_info->num_linked_streams] =
          stream_params;
        linked_info->num_linked_streams++;
      }
    }
  }

  return 0;
}

boolean cpp_port_decide_src(mct_module_t *module,
  mct_port_t *port, uint32_t identity)
{
  uint32_t i = 0;
  int rc = 0;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_port_data_t *port_data = NULL;
  cpp_module_ctrl_t *ctrl = NULL;
  uint32_t stream_id;
  uint32_t session_id;

  if(!port) {
    CPP_ERR("failed, port=%p\n", port);
    return FALSE;
  }

  port_data = (cpp_port_data_t *) MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    CPP_ERR("invalid port data, failed\n");
    return FALSE;
  }

  if(port_data->num_streams == 0)
    return TRUE;

  ctrl = (cpp_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if (!ctrl) {
    CPP_ERR("failed, module=%p\n", module);
    return FALSE;
  }

  stream_id = CPP_GET_STREAM_ID(identity);
  session_id = CPP_GET_STREAM_ID(identity);

  if (port_data->num_streams > 0) {
    for (i = 0; i < CPP_MAX_STREAMS_PER_PORT; i++) {
      if ((port_data->stream_data[i].port_state != CPP_PORT_STATE_UNRESERVED)
          && (port_data->stream_data[i].identity != identity)) {
         mct_port_t *sink1 =
           cpp_module_find_port_with_identity(module,
           MCT_PORT_SINK,port_data->stream_data[i].identity);
         mct_port_t *sink2 =
           cpp_module_find_port_with_identity(module, MCT_PORT_SINK, identity);
         if (sink1 == sink2 ) {
            return TRUE;
         }
      }
    }
  }

  return FALSE;
}
