/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vpu_port.h"
#include "vpu_module.h"
#include "camera_dbg.h"
#include "vpu_log.h"

mct_port_t *vpu_port_create(const char* name, mct_port_direction_t dir)
{
  mct_port_t  *port;
  vpu_port_data_t* port_data;
  uint32_t i;
  port = mct_port_create(name);

  if(!port) {
    VPU_ERR("failed");
    goto port_create_error;
  }
  port->direction = dir;
  port->check_caps_reserve = vpu_port_check_caps_reserve;
  port->check_caps_unreserve = vpu_port_check_caps_unreserve;
  port->ext_link = vpu_port_ext_link_func;
  port->un_link = vpu_port_ext_unlink_func;
  port->event_func = vpu_port_event_func;
  port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;

  port_data = (vpu_port_data_t*) CAM_MALLOC(sizeof(vpu_port_data_t));
  if(!port_data) {
    VPU_ERR("failed");
    goto port_data_error;
  }
  memset(port_data, 0x00, sizeof(vpu_port_data_t));
  for(i=0; i<VPU_MAX_STREAMS_PER_PORT; i++) {
    //port_data->port_state[i] = VPU_PORT_STATE_UNRESERVED;
    port_data->stream_data[i].port_state = VPU_PORT_STATE_UNRESERVED;
  }
  port_data->port_type = VPU_PORT_TYPE_INVALID;
  port_data->num_streams = 0;
  MCT_OBJECT_PRIVATE(port) = port_data;
  return port;

port_data_error:
  mct_port_destroy(port);
port_create_error:
  return NULL;
}
void vpu_port_destroy(mct_port_t *port)
{
  if(!port) {
    return;
  }
  free(MCT_OBJECT_PRIVATE(port));
  mct_port_destroy(port);
}

static inline boolean is_4k_stream(mct_stream_info_t *stream_info)
{
  return (stream_info->dim.width > 3800);
}

/** vpu_port_check_identity
 *    @d1: session+stream identity
 *    @d2: session+stream identity
 *
 *  To find out if both identities are matching;
 *  Return TRUE if matches.
 **/
static boolean vpu_port_check_identity(void *d1, void *d2)
{
  unsigned int v1, v2;

  v1 = *((unsigned int *)d1);
  v2 = *((unsigned int *)d2);

  return ((v1 == v2) ? TRUE : FALSE);
}

/** vpu_port_check_identity_in_port
 *    @data1: this port on which identity needs to be
 *            checked
 *    @data2: session+stream identity
 *
 *  To find out if identities is attached in port;
 *
 *  Return TRUE if matches.
 **/
boolean vpu_port_check_identity_in_port(void *data1, void *data2)
{
  boolean       rc = FALSE;
  mct_port_t   *port = (mct_port_t *)data1;
  uint32_t *identity = (uint32_t *)data2;

  CDBG("%s:%d] E\n", __func__, __LINE__);
  /* check for sanity */
  if (!data1 || !data2) {
    CDBG_ERROR("%s:%d] error data1: %p, data2: %p\n", __func__, __LINE__,
      data1, data2);
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  if (mct_list_find_custom(MCT_OBJECT_CHILDREN(port), identity,
    vpu_port_check_identity) != NULL) {
    rc = TRUE;
  }

  MCT_OBJECT_UNLOCK(port);
  CDBG("%s:%d] X rc: %d\n", __func__, __LINE__, rc);
  return rc;
}

static boolean vpu_port_check_caps_reserve(mct_port_t *port, void *peer_caps,
  void *info)
{
  if(!port || !info) {
    VPU_ERR("failed, port=%p, info=%p", port, info);
    return FALSE;
  }
  mct_stream_info_t *stream_info = (mct_stream_info_t *)info;
  mct_port_caps_t   *port_caps = (mct_port_caps_t *)(&(port->caps));
  vpu_port_data_t   *port_data = MCT_OBJECT_PRIVATE(port);
  uint32_t           identity = stream_info->identity;
  uint32_t           session_id, stream_id;
  int32_t            rc;
  mct_module_t      *module = (mct_module_t*)(MCT_PORT_PARENT(port)->data);
  mct_list_t        *p_list = NULL;
  mct_port_t        *sink_port = NULL;
  vpu_port_data_t   *sink_port_data = NULL;

  session_id = VPU_GET_SESSION_ID(identity);
  stream_id = VPU_GET_STREAM_ID(identity);
  VPU_LOW("identity=0x%x", identity);

  if(port_data->num_streams >= VPU_MAX_STREAMS_PER_PORT) {
    VPU_ERR("failed. max streams reached, num=%d", port_data->num_streams);
    return FALSE;
  }
  if(port->direction == MCT_PORT_SINK) {
    /* TODO: Add more peer caps checking logic */
    mct_port_caps_t *peer_port_caps = (mct_port_caps_t *)peer_caps;
    if(peer_caps) {
      if(port_caps->port_caps_type != peer_port_caps->port_caps_type) {
        return FALSE;
      }
    }
  } else {
    /* Get the module's sink ports and retrieve the existing stream */
    module = (mct_module_t *)MCT_PORT_PARENT(port)->data;
    p_list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module),
      &stream_info->identity, vpu_port_check_identity_in_port);
    if (!p_list) {
      CDBG_ERROR("%s:%d] error no matching sink port found\n", __func__,
        __LINE__);
      return FALSE;
    }

    sink_port = (mct_port_t *)p_list->data;
    sink_port_data = MCT_OBJECT_PRIVATE(sink_port);
    if (((sink_port_data->int_link_port) &&
      (sink_port_data->int_link_port != port)) ||
      ((port_data->int_link_port) &&
      (port_data->int_link_port != sink_port))) {
      return FALSE;
    }
  }

  switch(port_data->port_type) {
  case VPU_PORT_TYPE_INVALID: {
    port_data->num_streams = 0;
    if(stream_info->streaming_mode == CAM_STREAMING_MODE_CONTINUOUS) {
      port_data->port_type = VPU_PORT_TYPE_STREAMING;
      if (stream_info->stream_type == CAM_STREAM_TYPE_PARM) {
        CDBG("%s: CAM_STREAM_TYPE_PRAM\n", __func__);
        port_data->port_type = VPU_PORT_TYPE_SESSION;
      }
    } else {
      port_data->port_type = VPU_PORT_TYPE_BURST;
    }
    port_data->session_id = session_id;
    break;
  }
  case VPU_PORT_TYPE_STREAMING: {
    if(stream_info->streaming_mode != CAM_STREAMING_MODE_CONTINUOUS) {
      VPU_DBG("info: streaming mode doesn't match");
      return FALSE;
    }
    if(port_data->session_id != session_id) {
      VPU_DBG("info: session id doesn't match");
      return FALSE;
    }
    break;
  }
  case VPU_PORT_TYPE_BURST: {
    /* allow only one stream on burst port */
    VPU_LOW("info: only one stream allowed on burst port");
    return FALSE;
  }
  case VPU_PORT_TYPE_SESSION:
    CDBG("%s: VPU_PORT_TYPE_SESSION\n", __func__);
    return FALSE;
  default:
    VPU_ERR("failed, bad port_type=%d", port_data->port_type);
    return FALSE;
  }

  uint32_t i;

  VPU_DBG("dim=%dx%d", stream_info->dim.width, stream_info->dim.height);

  /* Special Case for 4k. Due to ISP limitations,
     4k streams are not bundled on one port for video recording.
     VPU too, needs to keep one stream per port in this case */
  if(port_data->has_4k_stream == TRUE) {
    VPU_DBG("no space for new stream here");
    return FALSE;
  }

  /* reserve the port for this stream */
  for(i=0; i<VPU_MAX_STREAMS_PER_PORT; i++) {
    if (port_data->stream_data[i].port_state == VPU_PORT_STATE_UNRESERVED) {
      port_data->stream_data[i].port_state = VPU_PORT_STATE_RESERVED;
      port_data->stream_data[i].identity = identity;
      port_data->stream_data[i].streaming_mode = stream_info->streaming_mode;
      port_data->num_streams++;
      VPU_DBG("identity=0x%x, reserved on %s", identity,
        MCT_OBJECT_NAME(port));
      break;
    }
  }
  if(i == VPU_MAX_STREAMS_PER_PORT) {
    VPU_ERR("failed, unexpected error!");
    return FALSE;
  }
  port_data->has_4k_stream = is_4k_stream(stream_info);
  /* notify the parent module about this new stream, once for the sink port */
  if(port->direction == MCT_PORT_SINK) {
    rc = vpu_module_notify_add_stream(module, port, stream_info);
    if(rc < 0) {
      VPU_ERR("failed, unexpected error!");
      return FALSE;
    }
  } else if (sink_port_data) {
    sink_port_data->int_link_port = port;
    port_data->int_link_port = sink_port;
  }
  return TRUE;
}

static boolean vpu_port_check_caps_unreserve(mct_port_t *port,
  uint32_t identity)
{
  if(!port) {
    VPU_ERR("failed");
    return FALSE;
  }
  VPU_DBG("identity=0x%x", identity);
  vpu_port_data_t *port_data = (vpu_port_data_t *) MCT_OBJECT_PRIVATE(port);
  int32_t i, rc;
  for(i=0; i<VPU_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state == VPU_PORT_STATE_RESERVED &&
       port_data->stream_data[i].identity == identity) {
      port_data->stream_data[i].port_state = VPU_PORT_STATE_UNRESERVED;
      port_data->num_streams--;
      VPU_DBG("identity=0x%x, unreserved on %s",
        identity, MCT_OBJECT_NAME(port));
      if(port_data->num_streams == 0) {
        port_data->port_type = VPU_PORT_TYPE_INVALID;
        port_data->session_id = 0x00;
        port_data->has_4k_stream = FALSE;
      }
      break;
    }
  }
  if(i == VPU_MAX_STREAMS_PER_PORT) {
    VPU_ERR("can't find matching identity, unexpected!!");
    return FALSE;
  }

  /* notify the parent module about removal of this stream,
     once for the sink port */
  if(port->direction == MCT_PORT_SINK) {
    mct_module_t* module = (mct_module_t*)(MCT_PORT_PARENT(port)->data);
    rc = vpu_module_notify_remove_stream(module, identity);
    if(rc < 0) {
      VPU_ERR("failed, unexpected error!");
      return FALSE;
    }
  }
  return TRUE;
}

static boolean vpu_port_ext_link_func(uint32_t identity,
  mct_port_t* port, mct_port_t *peer)
{
  if(!port || !peer) {
    VPU_ERR("failed, port=%p, peer=%p", port, peer);
    return FALSE;
  }
  MCT_OBJECT_LOCK(port);

  if (MCT_PORT_PEER(port) && (MCT_PORT_PEER(port) != peer)) {
    VPU_ERR("error old_peer:%s, new_peer:%s",
            MCT_OBJECT_NAME(MCT_PORT_PEER(port)), MCT_OBJECT_NAME(peer));
    MCT_OBJECT_UNLOCK(port);
    return FALSE;
  }

  vpu_port_data_t* port_data = (vpu_port_data_t *)MCT_OBJECT_PRIVATE(port);
  uint32_t i;
  for(i=0; i<VPU_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state == VPU_PORT_STATE_RESERVED &&
        port_data->stream_data[i].identity == identity) {
      port_data->stream_data[i].port_state = VPU_PORT_STATE_LINKED;
      if (MCT_OBJECT_REFCOUNT(port) == 0) {
        MCT_PORT_PEER(port) = peer;
      }
      MCT_OBJECT_REFCOUNT(port) += 1;
      MCT_OBJECT_UNLOCK(port);
      return TRUE;
    }
  }
  MCT_OBJECT_UNLOCK(port);
  return FALSE;
}


static void vpu_port_ext_unlink_func(uint32_t identity,
  mct_port_t *port, mct_port_t *peer)
{
  if(!port || !peer) {
    VPU_ERR("failed, port=%p, peer=%p", port, peer);
    return;
  }

  MCT_OBJECT_LOCK(port);
  if (MCT_PORT_PEER(port) != peer) {
    VPU_ERR("failed peer:%p, unlink_peer:%p", MCT_PORT_PEER(port), peer);
    MCT_OBJECT_UNLOCK(port);
    return;
  }

  if (MCT_OBJECT_REFCOUNT(port) == 0) {
    VPU_ERR("failed zero refcount on port");
    MCT_OBJECT_UNLOCK(port);
    return;
  }

  vpu_port_data_t* port_data = (vpu_port_data_t *)MCT_OBJECT_PRIVATE(port);
  uint32_t i;
  for(i=0; i<VPU_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state == VPU_PORT_STATE_LINKED &&
        port_data->stream_data[i].identity == identity) {
      port_data->stream_data[i].port_state = VPU_PORT_STATE_RESERVED;
      MCT_OBJECT_REFCOUNT(port) -= 1;
      if (MCT_OBJECT_REFCOUNT(port) == 0) {
        MCT_PORT_PEER(port) = NULL;
      }
      MCT_OBJECT_UNLOCK(port);
      return;
    }
  }
  MCT_OBJECT_UNLOCK(port);
  VPU_ERR("failed");
  return;
}

static boolean vpu_port_event_func(mct_port_t *port,
                                   mct_event_t *event)
{
  int32_t rc = -EINVAL;

  if(!port || !event) {
    VPU_ERR("failed, port=%p, event=%p", port, event);
    return FALSE;
  }
  mct_module_t *module;
  mct_list_t *templist = (mct_list_t*)MCT_PORT_PARENT(port);
  module = (mct_module_t*)(templist->data);

  switch(port->direction) {
  case MCT_PORT_SRC:
    rc = vpu_module_process_upstream_event(module, event);
    break;
  case MCT_PORT_SINK:
    rc = vpu_module_process_downstream_event(module, event);
    break;
  default:
    VPU_ERR("failed, bad port->direction=%d", port->direction);
  }
  return rc == 0 ? TRUE : FALSE;
}


/* vpu_port_get_linked_identity:
 *
 *  finds another identity mapped on the port which is not equal to @identity
 **/
int32_t vpu_port_get_linked_identity(mct_port_t *port, uint32_t identity,
  uint32_t *linked_identity)
{
  if(!port || !linked_identity) {
    VPU_ERR("failed, port=%p, linked_identity=%p\n",
      port, linked_identity);
    return -1;
  }
  vpu_port_data_t *port_data = (vpu_port_data_t *) MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    VPU_ERR("failed");
    return -1;
  }
  if(port_data->num_streams > 0) {
    uint32_t i;
    for (i=0; i<VPU_MAX_STREAMS_PER_PORT; i++) {
      if (port_data->stream_data[i].port_state != VPU_PORT_STATE_UNRESERVED) {
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
