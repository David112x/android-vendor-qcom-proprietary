/*============================================================================

  Copyright (c) 2013, 2017 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "c2d_port.h"
#include "c2d_module.h"
#include "camera_dbg.h"
#include "c2d_log.h"

mct_port_t *c2d_port_create(const char* name, mct_port_direction_t dir)
{
  mct_port_t  *port;
  c2d_port_data_t* port_data;
  uint32_t i;
  C2D_HIGH(" Enter: name=%s, dir=%d", name, dir);
  port = mct_port_create(name);

  if(!port) {
    C2D_ERR("failed");
    goto port_create_error;
  }
  port->direction = dir;
  port->check_caps_reserve = c2d_port_check_caps_reserve;
  port->check_caps_unreserve = c2d_port_check_caps_unreserve;
  port->ext_link = c2d_port_ext_link_func;
  port->un_link = c2d_port_ext_unlink_func;
  port->event_func = c2d_port_event_func;
  port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;

  port_data = (c2d_port_data_t*) CAM_MALLOC(sizeof(c2d_port_data_t));
  if(!port_data) {
    C2D_ERR("failed");
    goto port_data_error;
  }
  memset(port_data, 0x00, sizeof(c2d_port_data_t));
  for(i=0; i<C2D_MAX_STREAMS_PER_PORT; i++) {
    //port_data->port_state[i] = C2D_PORT_STATE_UNRESERVED;
    port_data->stream_data[i].port_state = C2D_PORT_STATE_UNRESERVED;
  }
  port_data->port_type = C2D_PORT_TYPE_INVALID;
  port_data->num_streams = 0;
  MCT_OBJECT_PRIVATE(port) = port_data;
  return port;

port_data_error:
  mct_port_destroy(port);
port_create_error:
  return NULL;
}
void c2d_port_destroy(mct_port_t *port)
{
  if(!port) {
    return;
  }
  free(MCT_OBJECT_PRIVATE(port));
  mct_port_destroy(port);
}

static boolean c2d_port_check_caps_reserve(mct_port_t *port, void *peer_caps,
  void *info)
{
  if(!port || !info) {
    C2D_ERR("failed, port=%p, info=%p", port, info);
    return FALSE;
  }
  mct_stream_info_t *stream_info = (mct_stream_info_t *)info;
  mct_port_caps_t *port_caps = (mct_port_caps_t *)(&(port->caps));
  c2d_port_data_t *port_data = MCT_OBJECT_PRIVATE(port);
  uint32_t identity = stream_info->identity;
  uint32_t session_id, stream_id;
  int32_t rc;
  uint32_t interleaved;
  session_id = C2D_GET_SESSION_ID(identity);
  stream_id = C2D_GET_STREAM_ID(identity);
  C2D_HIGH("identity=0x%x\n", identity);

  if(port_data->num_streams >= C2D_MAX_STREAMS_PER_PORT) {
    C2D_ERR("failed. max streams reached, num=%d",
      port_data->num_streams);
    return FALSE;
  }
  if(port->direction == MCT_PORT_SINK) {
    /* TODO: For Offline/peerless this is not true because peer caps is NULL */
#if 0
    if(!peer_caps) {
      C2D_ERR("failed, peear_caps=%p", peer_caps);
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
  } else if (port->direction == MCT_PORT_SRC) {
    mct_module_t* module = (mct_module_t*)(MCT_PORT_PARENT(port)->data);
    interleaved = c2d_module_check_interleaved_stream(module, identity);
    C2D_HIGH("Interleaved flag is %d, identity %d port type %d",interleaved,
      identity, port_data->port_type);
    if (port_data->port_type == C2D_PORT_TYPE_PARAM) {
      return FALSE;
    }
    if ((port_data->num_streams) &&
      (((stream_info->is_type != IS_TYPE_EIS_2_0) ||
         (stream_info->dim.width*stream_info->dim.height >=
           (VIDEO_4K_WIDTH * VIDEO_4K_HEIGHT) )) &&
      ((port_data->port_type != C2D_PORT_TYPE_STREAMING) || !interleaved))) {
      C2D_ERR("It is not special case");
      return FALSE;
    }
  }

  switch(port_data->port_type) {
  case C2D_PORT_TYPE_INVALID: {
    port_data->num_streams = 0;
    if(stream_info->streaming_mode == CAM_STREAMING_MODE_CONTINUOUS) {
      port_data->port_type = C2D_PORT_TYPE_STREAMING;
      if (stream_info->stream_type == CAM_STREAM_TYPE_PARM) {
        port_data->port_type = C2D_PORT_TYPE_PARAM;
      }
    } else {
      port_data->port_type = C2D_PORT_TYPE_BURST;
    }
    port_data->session_id = session_id;
    break;
  }
  case C2D_PORT_TYPE_STREAMING:
  case C2D_PORT_TYPE_BURST: {
    if(port_data->session_id != session_id) {
      C2D_DBG("info: session id doesn't match");
      return FALSE;
    }
    break;
  }
  case C2D_PORT_TYPE_PARAM:
    C2D_DBG("C2D_PORT_TYPE_PARAM\n");
    rc = FALSE;
    break;
  default:
    C2D_ERR("failed, bad port_type=%d\n", port_data->port_type);
    return FALSE;
  }
  uint32_t i;
  /* reserve the port for this stream */
  for(i=0; i<C2D_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state == C2D_PORT_STATE_UNRESERVED) {
      port_data->stream_data[i].port_state = C2D_PORT_STATE_RESERVED;
      port_data->stream_data[i].identity = identity;
      port_data->stream_data[i].streaming_mode = stream_info->streaming_mode;
      port_data->num_streams++;
      C2D_HIGH("identity=0x%x, reserved\n", identity);
      break;
    }
  }
  if(i == C2D_MAX_STREAMS_PER_PORT) {
    C2D_ERR("failed, unexpected error!");
    return FALSE;
  }
  /* notify the parent module about this new stream, once for the sink port */
  if(port->direction == MCT_PORT_SINK) {
    mct_module_t* module = (mct_module_t*)(MCT_PORT_PARENT(port)->data);
    rc = c2d_module_notify_add_stream(module, port, stream_info, peer_caps);
    if(rc < 0) {
      C2D_ERR("failed, unexpected error!");
      return FALSE;
    }
  }
  return TRUE;
}

static boolean c2d_port_check_caps_unreserve(mct_port_t *port,
  uint32_t identity)
{
  if(!port) {
    C2D_ERR("failed\n");
    return FALSE;
  }
  C2D_HIGH("identity=0x%x\n", identity);
  c2d_port_data_t *port_data = (c2d_port_data_t *) MCT_OBJECT_PRIVATE(port);
  uint32_t i;
  int32_t rc;
  for(i=0; i<C2D_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state == C2D_PORT_STATE_RESERVED &&
       port_data->stream_data[i].identity == identity) {
      port_data->stream_data[i].port_state = C2D_PORT_STATE_UNRESERVED;
      port_data->num_streams--;
      C2D_HIGH("identity=0x%x, unreserved\n", identity);
      if(port_data->num_streams == 0) {
        port_data->port_type = C2D_PORT_TYPE_INVALID;
        port_data->session_id = 0x00;
      }
      break;
    }
  }
  if(i == C2D_MAX_STREAMS_PER_PORT) {
    C2D_ERR("can't find matching identity, unexpected !!");
    return FALSE;
  }

  /* notify the parent module about removal of this stream,
     once for the sink port */
  if(port->direction == MCT_PORT_SINK) {
    mct_module_t* module = (mct_module_t*)(MCT_PORT_PARENT(port)->data);
    rc = c2d_module_notify_remove_stream(module, identity);
    if(rc < 0) {
      C2D_ERR("failed, unexpected error!");
      return FALSE;
    }
  }
  return TRUE;
}

static boolean c2d_port_ext_link_func(uint32_t identity,
  mct_port_t* port, mct_port_t *peer)
{
  if(!port || !peer) {
    C2D_ERR("failed, port=%p, peer=%p", port, peer);
    return FALSE;
  }
  C2D_LOW("E");
  MCT_OBJECT_LOCK(port);

  if (MCT_PORT_PEER(port) && (MCT_PORT_PEER(port) != peer)) {
    C2D_ERR("error old_peer:%s, new_peer:%s\n",
      MCT_OBJECT_NAME(MCT_PORT_PEER(port)), MCT_OBJECT_NAME(peer));
    MCT_OBJECT_UNLOCK(port);
    return FALSE;
  }

  c2d_port_data_t* port_data = (c2d_port_data_t *)MCT_OBJECT_PRIVATE(port);
  uint32_t i;
  for(i=0; i<C2D_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state == C2D_PORT_STATE_RESERVED &&
        port_data->stream_data[i].identity == identity) {
      port_data->stream_data[i].port_state = C2D_PORT_STATE_LINKED;
      if (MCT_OBJECT_REFCOUNT(port) == 0) {
        MCT_PORT_PEER(port) = peer;
      }
      MCT_OBJECT_REFCOUNT(port) += 1;
      MCT_OBJECT_UNLOCK(port);
      C2D_LOW("X");
      return TRUE;
    }
  }
  MCT_OBJECT_UNLOCK(port);
  C2D_LOW("X");
  return FALSE;
}


static void c2d_port_ext_unlink_func(uint32_t identity,
  mct_port_t *port, mct_port_t *peer)
{
  if(!port || !peer) {
    C2D_ERR("failed, port=%p, peer=%p", port, peer);
    return;
  }

  MCT_OBJECT_LOCK(port);
  if (MCT_PORT_PEER(port) != peer) {
    C2D_ERR("failed peer:%p, unlink_peer:%p\n",
      MCT_PORT_PEER(port), peer);
    MCT_OBJECT_UNLOCK(port);
    return;
  }

  if (MCT_OBJECT_REFCOUNT(port) == 0) {
    C2D_ERR("failed zero refcount on port\n");
    MCT_OBJECT_UNLOCK(port);
    return;
  }

  c2d_port_data_t* port_data = (c2d_port_data_t *)MCT_OBJECT_PRIVATE(port);
  uint32_t i;
  for(i=0; i<C2D_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state == C2D_PORT_STATE_LINKED &&
        port_data->stream_data[i].identity == identity) {
      port_data->stream_data[i].port_state = C2D_PORT_STATE_RESERVED;
      MCT_OBJECT_REFCOUNT(port) -= 1;
      if (MCT_OBJECT_REFCOUNT(port) == 0) {
        MCT_PORT_PEER(port) = NULL;
      }
      MCT_OBJECT_UNLOCK(port);
      return;
    }
  }
  MCT_OBJECT_UNLOCK(port);
  C2D_ERR("failed");
  return;
}

static boolean c2d_port_event_func(mct_port_t *port,
                                   mct_event_t *event)
{
  int32_t rc = -EINVAL;

  if(!port || !event) {
    C2D_ERR("failed, port=%p, event=%p", port, event);
    return FALSE;
  }
  mct_module_t *module;
  mct_list_t *templist = (mct_list_t*)MCT_PORT_PARENT(port);
  module = (mct_module_t*)(templist->data);

  switch(port->direction) {
  case MCT_PORT_SRC:
    rc = c2d_module_process_upstream_event(module, event);
    break;
  case MCT_PORT_SINK:
    rc = c2d_module_process_downstream_event(module, event);
    break;
  default:
    C2D_ERR("failed, bad port->direction=%d", port->direction);
  }
  return rc == 0 ? TRUE : FALSE;
}

/* c2d_port_get_linked_identity:
 *
 *  finds another identity mapped on the port which is not equal to @identity
 **/
int32_t c2d_port_get_linked_identity(mct_port_t *port, uint32_t identity,
  uint32_t *linked_identity)
{
  if(!port || !linked_identity) {
    C2D_ERR("failed, port=%p, linked_identity=%p\n",
      port, linked_identity);
    return -EINVAL;
  }
  c2d_port_data_t *port_data = (c2d_port_data_t *) MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    C2D_ERR("failed\n");
    return -EFAULT;
  }
  if(port_data->num_streams > 0) {
    uint32_t i;
    for (i=0; i<C2D_MAX_STREAMS_PER_PORT; i++) {
      if (port_data->stream_data[i].port_state != C2D_PORT_STATE_UNRESERVED) {
        if (port_data->stream_data[i].identity != identity) {
          *linked_identity = port_data->stream_data[i].identity;
          return 1;
        }
      }
    }
  }
  *linked_identity=0x00;
  return 0;
}
