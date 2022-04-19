/*============================================================================

 Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include "base_port.h"
#include "pp_utils.h"
#include "base_module.h"

typedef struct {
  mct_port_t *port;
  uint32_t identity;
} int_link_traverse_data_t;

static inline mct_module_t* get_parent_module(mct_port_t *port)
{
  mct_module_t *p_module;
  mct_list_t *templist = (mct_list_t*)MCT_OBJECT_PARENT(port);
  p_module = (mct_module_t*)(templist->data);
  return p_module;
}

base_port_stream_data_t *base_port_get_stream_data(mct_port_t *port,
                                                uint32_t identity)
{
  base_port_data_t *port_data;
  int32_t i;
  port_data = MCT_OBJECT_PRIVATE(port);
  for (i=0; i<BASE_PORT_MAX_STREAMS; i++) {
    if (port_data->stream_data[i].stream_info &&
        port_data->stream_data[i].stream_info->identity == identity) {
      return &port_data->stream_data[i];
    }
  }
  return NULL;
}

boolean base_port_check_sink_port_compatability(mct_port_t *src_port,
  mct_port_t * sink_port)
{
  base_port_data_t *port_data;
  int32_t i;
  port_data = MCT_OBJECT_PRIVATE(src_port);
  for (i = 0; i < BASE_PORT_MAX_STREAMS; i++) {
    if(port_data->stream_data[i].stream_info)
    if (port_data->stream_data[i].num_int_links) {
      if(strcmp(MCT_OBJECT_NAME(port_data->stream_data[i].int_links->data),
        MCT_OBJECT_NAME(sink_port))) {
        return FALSE;
      }
    }
  }

  return TRUE;
}

static inline boolean compare_data(void *data, void *userdata)
{
  return (data == userdata);
}

int32_t base_port_int_link(mct_port_t *src_port, mct_port_t *sink_port,
                           uint32_t identity)
{
  if (!src_port || !sink_port) {
    PP_ERR("failed, src_port=%p, sink_port=%p", src_port, sink_port);
    return -1;
  }
  assert(src_port->direction == MCT_PORT_SRC);
  assert(sink_port->direction == MCT_PORT_SINK);

  PP_DBG("src=%s, sink=%s, identity=%x", MCT_OBJECT_NAME(src_port),
         MCT_OBJECT_NAME(sink_port), identity);

  base_port_data_t *src_data, *sink_data;
  base_port_stream_data_t *src_stream_data, *sink_stream_data;
  mct_list_t *templist = NULL;

  src_data = MCT_OBJECT_PRIVATE(src_port);
  sink_data = MCT_OBJECT_PRIVATE(sink_port);

  if (!base_port_check_sink_port_compatability(src_port, sink_port)) {
    goto error;
  }


  src_stream_data = base_port_get_stream_data(src_port, identity);
  sink_stream_data = base_port_get_stream_data(sink_port, identity);

  if (!src_stream_data || !sink_stream_data) {
    PP_ERR("failed");
    goto error;
  }
  /* add internal links to both ports without duplication */
  templist = mct_list_find_custom(src_stream_data->int_links,
                                  sink_port, compare_data);
  if (!templist) {
    src_stream_data->int_links =
      mct_list_append(src_stream_data->int_links, sink_port, NULL, NULL);
    src_stream_data->num_int_links++;
  }
  templist = mct_list_find_custom(sink_stream_data->int_links,
                                  src_port, compare_data);
  if (!templist) {
    sink_stream_data->int_links =
      mct_list_append(sink_stream_data->int_links, src_port, NULL, NULL);
    sink_stream_data->num_int_links++;
  }

  PP_DBG("%s <---> %s,  identity %x",
         MCT_OBJECT_NAME(src_port), MCT_OBJECT_NAME(sink_port), identity);

  src_port->caps.u.frame.format_flag = sink_port->caps.u.frame.format_flag;

  return 0;
error:
  PP_ERR("failed, src=%s, sink=%s, identity=%x",
         MCT_OBJECT_NAME(src_port), MCT_OBJECT_NAME(sink_port), identity);
  return -1;
}


static boolean int_link_traverse_func(void *data, void *userdata)
{
  int32_t rc;
  mct_port_t *sinkport = (mct_port_t *)data;
  int_link_traverse_data_t *t_data = (int_link_traverse_data_t*) userdata;

  mct_port_t *srcport = t_data->port;
  uint32_t identity = t_data->identity;

  rc = base_port_int_link(srcport, sinkport, identity);
  if (rc < 0) {
    return false;
  }
  return true;
}

static int32_t base_port_get_linked_sink_port(mct_port_t *srcport,
                                              mct_port_t **p_sinkport)
{
  base_port_data_t *port_data = (base_port_data_t *)
    MCT_OBJECT_PRIVATE(srcport);
  int32_t i;
  mct_port_t *sinkport = NULL;
  for (i=0; i<BASE_PORT_MAX_STREAMS; i++) {
    if (port_data->stream_data[i].stream_info != NULL) {
      if (port_data->stream_data[i].num_int_links > 0 &&
          port_data->stream_data[i].int_links) {
        if (!sinkport) {
          sinkport = (mct_port_t *)port_data->stream_data[i].int_links->data;
        } else {
          if (sinkport !=
               (mct_port_t *)port_data->stream_data[i].int_links->data) {
            PP_ERR("invalid condition, two different sink ports linked to one"
                   " src port");
            return -1;
          }
        }
      }
    }
  }
  *p_sinkport = sinkport;
  return 0;
}

static int32_t base_port_update_int_links(mct_port_t *srcport,
                                          uint32_t identity)
{
  mct_list_t *sinkports;
  int_link_traverse_data_t t_data;
  mct_port_t * sink_port;
  t_data.port = srcport;
  t_data.identity = identity;
  int32_t rc=0;

  PP_DBG("srcport=%s, identity=%x", MCT_OBJECT_NAME(srcport), identity);

  sinkports = base_module_get_ports_for_identity(get_parent_module(srcport),
                                                 identity, MCT_PORT_SINK);
  if (!sinkports) {
    PP_ERR("failed");
    rc = -1;
    goto end;
  }

  base_port_stream_data_t *stream_data =
    base_port_get_stream_data(srcport, identity);
  mct_port_t *sinkport = (mct_port_t *)sinkports->data;

  /* Only one int link is allowed in src port. This is because, base module
     can not support internal linking of multiple sink ports
     to a single src port */
  mct_port_t* linked_sinkport;
  rc = base_port_get_linked_sink_port(srcport, &linked_sinkport);
  if (rc < 0) {
    PP_ERR("failed");
    rc = -1;
    goto remove_port;
  }
  if (linked_sinkport != NULL) {
    if (linked_sinkport != sinkport) {
      PP_DBG("for this src port, linked sinkport exists and its different, "
             "linked_sink=%s, sink=%s", MCT_OBJECT_NAME(linked_sinkport),
              MCT_OBJECT_NAME(sinkport));
      rc = -1;
      goto remove_port;
    }
  }

  if (!mct_list_traverse(sinkports, int_link_traverse_func, &t_data)) {
    rc = -1;
    goto remove_port;
  }

remove_port:
  mct_list_free_list(sinkports);
end:
  return rc;
}

static boolean base_port_check_caps_reserve(mct_port_t *port, void *peer_caps,
                                            void *s_info)
{
  int32_t rc;
  if (!port || !s_info) {
    PP_ERR("failed");
    return false;
  }
  mct_stream_info_t *stream_info = (mct_stream_info_t *)s_info;
  mct_port_caps_t *port_caps = (mct_port_caps_t *)(&(port->caps));
  base_port_data_t *port_data = MCT_OBJECT_PRIVATE(port);

  PP_DBG("%s, identity=%x ,streamtype %d", MCT_OBJECT_NAME(port),
    stream_info->identity, stream_info->stream_type);
  if (port_data->stream_count == BASE_PORT_MAX_STREAMS) {
    PP_DBG("max stream limit reached, max=%d", BASE_PORT_MAX_STREAMS);
    return false;
  }

  int32_t spot = -1;
  int32_t i;

  MCT_OBJECT_LOCK(port);
  for (i=0; i<BASE_PORT_MAX_STREAMS; i++) {
    if (port_data->stream_data[i].stream_info == NULL) {
      if (spot == -1) {
        spot = i;
      }
    } else if (port_data->stream_data[i].stream_info->identity ==
                stream_info->identity) {
      /* same identity is already reserved here */
      MCT_OBJECT_UNLOCK(port);
      return false;
    } else if(port_data->stream_data[i].stream_info->stream_type ==
      CAM_STREAM_TYPE_OFFLINE_PROC) {
      MCT_OBJECT_UNLOCK(port);
      return false;
    }
  }
  if (spot == -1) {
    MCT_OBJECT_UNLOCK(port);
    return false;
  }
  port_data->stream_data[spot].stream_info = stream_info;

  port_data->stream_count++;

  /* update internal links when a src port is reserved */
  if (port->direction == MCT_PORT_SRC) {
    rc = base_port_update_int_links(port, stream_info->identity);
    if (rc < 0) {
      port_data->stream_data[spot].stream_info = NULL;
      port_data->stream_count--;
      MCT_OBJECT_UNLOCK(port);
      PP_DBG("could not set int_link for port %s", MCT_OBJECT_NAME(port));
      return false;
    }
  } else {
    if ((stream_info->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) && peer_caps)
      port_caps->u.frame.format_flag = ((mct_port_caps_t *)peer_caps)->u.frame.format_flag;
  }
  MCT_OBJECT_UNLOCK(port);
  PP_DBG("identity=%x reserved on %s", stream_info->identity,
         MCT_OBJECT_NAME(port));
  return true;
}

static boolean base_port_check_caps_unreserve(mct_port_t *port,
                                              uint32_t identity)
{
  PP_DBG("%s, identity=%x", MCT_OBJECT_NAME(port), identity);

  if (!port) {
    PP_ERR("failed");
    return false;
  }

  MCT_OBJECT_LOCK(port);
  base_port_data_t *port_data = MCT_OBJECT_PRIVATE(port);

  base_port_stream_data_t *stream_data =
    base_port_get_stream_data(port, identity);

  if (!stream_data) {
    PP_ERR("identity %x not present", identity);
    MCT_OBJECT_UNLOCK(port);
    return false;
  }

  /* remove current port from int_link list of all int links */
  if (stream_data->num_int_links > 0) {
    mct_list_t *temp = stream_data->int_links;
    while (temp) {
      mct_port_t *link = (mct_port_t *)temp->data;
      base_port_stream_data_t *link_stream_data =
        base_port_get_stream_data(link, identity);
      if (!link_stream_data) {
        MCT_OBJECT_UNLOCK(port);
        PP_ERR("failed");
        return false;
      }
      mct_list_t *templist = mct_list_find_custom(link_stream_data->int_links,
                                                  port, compare_data);
      if (templist && templist->data) {
        link_stream_data->int_links =
          mct_list_remove(link_stream_data->int_links, port);
        link_stream_data->num_int_links--;
        PP_DBG("%s <-/-> %s, identity=%x", MCT_OBJECT_NAME(port),
               MCT_OBJECT_NAME(link), identity);
      }

      if (temp->next_num > 0) {
        temp = temp->next[0];
      } else {
        break;
      }
    }
  }

  port_data->stream_count--;

  if (!port_data->stream_count) {
    PTHREAD_MUTEX_LOCK(&port_data->ack_list.list_mutex);
    while (port_data->ack_list.size) {
      ack_list_entry_t *ack =
        (ack_list_entry_t *)port_data->ack_list.list->data;
      port_data->ack_list.list =
        mct_list_remove(port_data->ack_list.list, ack);
      port_data->ack_list.size--;
      PP_HIGH("pending ack, identity=%x, frame_id=%d, idx=%d, size:%d\n",
        ack->identity, ack->frameid, ack->buf_idx, port_data->ack_list.size);
      free(ack);
    }
    PTHREAD_MUTEX_UNLOCK(&port_data->ack_list.list_mutex);
  }

  mct_list_free_list(stream_data->int_links);
  stream_data->num_int_links = 0;
  stream_data->int_links = NULL;
  stream_data->stream_info = NULL;
  stream_data->state = BASE_PORT_STATE_UNRESERVED;
  stream_data->streamon = FALSE;

  if (port_data->stream_count < 0) {
    PP_ERR("failed");
    MCT_OBJECT_UNLOCK(port);
    return false;
  }

  MCT_OBJECT_UNLOCK(port);

  PP_DBG("identity=%x unreserved on %s", identity,
          MCT_OBJECT_NAME(port));
  return true;
}

static boolean base_port_ext_link_func(uint32_t identity, mct_port_t* port,
                                       mct_port_t *peer)
{
  PP_DBG("port=%s, peer=%s", MCT_OBJECT_NAME(port), MCT_OBJECT_NAME(peer));
  if (!port || !peer) {
    PP_ERR("failed, port=%p, peer=%p", port, peer);
    return false;
  }
  MCT_OBJECT_LOCK(port);
  if (port->peer && port->peer != peer) {
    PP_ERR("port peer already exists and it's different");
    MCT_OBJECT_UNLOCK(port);
    return false;
  }
  if (MCT_OBJECT_REFCOUNT(port) == 0) {
    port->peer = peer;
  }
  MCT_OBJECT_REFCOUNT(port)++;
  MCT_OBJECT_UNLOCK(port);
  PP_DBG("identity=%x, %s linked to %s", identity, MCT_OBJECT_NAME(port),
         MCT_OBJECT_NAME(peer));
  return true;
}

static void base_port_ext_unlink_func(uint32_t identity, mct_port_t *port,
                                      mct_port_t *peer)
{
  if (!port || !peer) {
    PP_ERR("failed, port=%p, peer=%p", port, peer);
    return;
  }

  MCT_OBJECT_LOCK(port);

  if (!port->peer) {
    PP_ERR("no peer for port %s", MCT_OBJECT_NAME(port));
    MCT_OBJECT_UNLOCK(port);
    return;
  }
  if (port->peer != peer) {
    PP_ERR("linked port %s, trying to unlink %s", MCT_OBJECT_NAME(port->peer),
           MCT_OBJECT_NAME(peer));
    MCT_OBJECT_UNLOCK(port);
    return;
  }

  MCT_OBJECT_REFCOUNT(port)--;

  if (MCT_OBJECT_REFCOUNT(port) == 0) {
    port->peer = NULL;
  }
  MCT_OBJECT_UNLOCK(port);
  PP_DBG("identity=%x, %s unlinked with %s", identity, MCT_OBJECT_NAME(port),
          MCT_OBJECT_NAME(peer));
}

static boolean forward_to_int_link_trav_func(void* data, void* userdata)
{
  boolean ret;
  mct_port_t *port = (mct_port_t *)data;
  mct_event_t *event = (mct_event_t *)userdata;

  ret = mct_port_send_event_to_peer(port, event);
  return ret;
}

int32_t base_port_fwd_event_to_intlinks(mct_port_t *port, mct_event_t *event)
{
  bool ret;
  base_port_stream_data_t *stream_data;
  stream_data = base_port_get_stream_data(port, event->identity);
  if (!stream_data) {
    PP_ERR("failed");
    return -1;
  }
  ret = mct_list_traverse(stream_data->int_links,
                    forward_to_int_link_trav_func, event);
  if (!ret) {
    return -1;
  }
  return 0;
}

static boolean ack_find_func(void *data, void* userdata)
{
  ack_list_entry_t *ack = (ack_list_entry_t *) data;
  ack_list_entry_t *ack_key = (ack_list_entry_t *) userdata;
  if (ack->buf_idx == ack_key->buf_idx && ack->frameid == ack_key->frameid) {
    return true;
  }
  return false;
}

int32_t base_port_handle_buf_divert(mct_port_t *port,
                                    mct_event_t *event)
{
  isp_buf_divert_t *buf_div = (isp_buf_divert_t *)
    (event->u.module_event.module_event_data);
  if (!buf_div) {
    PP_ERR("failed");
    return -1;
  }

  PP_DBG("identity=%x, frameid=%d, buf_idx=%d divert identity %x",
    event->identity, buf_div->buffer.sequence, buf_div->buffer.index,
    buf_div->identity);

  base_port_stream_data_t *stream_data =
    base_port_get_stream_data(port, event->identity);
  base_port_data_t *port_data = MCT_OBJECT_PRIVATE(port);

  int32_t ack_list_refcount=0, piggy_back_refcount=0;
  bool ack_buf_dirty=false;
  uint32_t buffer_access = buf_div->buffer_access;
  uint32_t orig_identity = event->identity;

  mct_list_t *event_port_list = NULL;
  int32_t i;

  /* calculate number of downstream modules */
  for (i=0; i<BASE_PORT_MAX_STREAMS; i++) {
    stream_data = &port_data->stream_data[i];
    if (stream_data->stream_info != NULL &&
      stream_data->streamon) {
      event->identity = stream_data->stream_info->identity;
      mct_list_t *temp = stream_data->int_links;
      while (temp) {
        mct_port_t *link = (mct_port_t *)temp->data;
        mct_list_t *templist;
        /* we dont want to send event to same port twice. for this we need to
        keep track of the ports which have already got the event */
        templist = mct_list_find_custom(event_port_list, link, compare_data);
        if (!templist) {
            event_port_list = mct_list_append(event_port_list,
                link, NULL, NULL);
            ack_list_refcount++;
        }
        if (temp->next_num > 0) {
          temp = temp->next[0];
        } else {
          break;
        }
      }
    }
  }
  /* Add ack with calcualted downsteam modules */
  if (ack_list_refcount > 0) {
    PP_DBG("ack_list_refcount=%d", ack_list_refcount);
    ack_list_entry_t* ack = (ack_list_entry_t * )
      CAM_MALLOC(sizeof(ack_list_entry_t));
    if (!ack) {
      PP_ERR("CAM_MALLOC failed");
      mct_list_free_list(event_port_list);
      event_port_list = NULL;
      return -1;
    }
    PTHREAD_MUTEX_LOCK(&port_data->ack_list.list_mutex);
    ack->buf_idx = buf_div->buffer.index;
    ack->frameid = buf_div->buffer.sequence;
    ack->refcount = ack_list_refcount;
    ack->is_buf_dirty = ack_buf_dirty;
    ack->buffer_access = buf_div->buffer_access;
    ack->identity = orig_identity;
    port_data->ack_list.list = mct_list_append(port_data->ack_list.list,
                               ack, NULL, NULL);
    port_data->ack_list.size++;
    PP_DBG("add in ack list, identity=%x, frame_id=%d, idx=%d, size:%d\n",
      ack->identity, ack->frameid, ack->buf_idx, port_data->ack_list.size);
    buf_div->ack_flag = false;
    buf_div->is_buf_dirty = false;
    PTHREAD_MUTEX_UNLOCK(&port_data->ack_list.list_mutex);
  }
  mct_list_free_list(event_port_list);
  event_port_list = NULL;
  /* fwd event for all identities in this port,
     translate identities */
  for (i=0; i<BASE_PORT_MAX_STREAMS; i++) {
    stream_data = &port_data->stream_data[i];
    if (stream_data->stream_info != NULL &&
      stream_data->streamon) {
      event->identity = stream_data->stream_info->identity;
      mct_list_t *temp = stream_data->int_links;
      while (temp) {
        mct_port_t *link = (mct_port_t *)temp->data;
        mct_list_t *templist;
        /* we dont want to send event to same port twice. for this we need to
           keep track of the ports which have already got the event */
        templist = mct_list_find_custom(event_port_list, link, compare_data);
        if (!templist) {
          PP_DBG("%s translated identity=%x",MCT_MODULE_NAME(port), event->identity);
          mct_port_send_event_to_peer(link, event);
          if (buf_div->ack_flag == true) {
            /* count number of modules which gives
            piggy back */
            piggy_back_refcount++;
          }
          ack_buf_dirty = ack_buf_dirty || buf_div->is_buf_dirty;
          // update the current state of buffer_access.
          // Do not OR because
          // say if base is connected to SW1, HW1, HW2 parallely,
          // SW1 sets READ flag
          // HW1 upon seeing READ flag, does invalidate and
          //   reset the buffer_access flags.
          // so now, HW2 doesn't need to invalidate again
          // So, we just need the latest status of buffer_access to be stored
          // in ack structure to handle later.
          buffer_access = buf_div->buffer_access;
          event_port_list = mct_list_append(event_port_list, link, NULL, NULL);
          /* Reset piggy back ack flags before checking for next identity */
          buf_div->ack_flag = false;
          buf_div->is_buf_dirty = false;
        }
        if (temp->next_num > 0) {
          temp = temp->next[0];
        } else {
          break;
        }
      }
    }
  }

  /* subtract ack reference with piggy back count */
  PTHREAD_MUTEX_LOCK(&port_data->ack_list.list_mutex);
  ack_list_entry_t ack_key;
  ack_key.buf_idx = buf_div->buffer.index;
  ack_key.frameid = buf_div->buffer.sequence;
  mct_list_t *templist = mct_list_find_custom(port_data->ack_list.list,
                        &ack_key, ack_find_func);
  if (templist && templist->data) {
    ack_list_entry_t *ack = (ack_list_entry_t *) templist->data;
    PP_DBG("rem_ack_refcount=%d ack->refcount %d", piggy_back_refcount,
        ack->refcount);

    if (ack->refcount >= piggy_back_refcount) {
        ack->refcount -= piggy_back_refcount;
    } else {
      ack->refcount = 0;
    }
    ack->is_buf_dirty = ack->is_buf_dirty || ack_buf_dirty;
    ack->buffer_access = buffer_access;
    if (ack->refcount == 0) {
      port_data->ack_list.list =
        mct_list_remove(port_data->ack_list.list, ack);
      PP_DBG("refcount=%d, piggy-back ack", ack->refcount);
      buf_div->ack_flag = true;
      buf_div->is_buf_dirty = true;
      if (port_data->ack_list.size) {
        port_data->ack_list.size--;
      }
      PP_DBG("remove ack list, identity=%x, frame_id=%d, idx=%d, size:%d\n",
        ack->identity, ack->frameid, ack->buf_idx, port_data->ack_list.size);
      free(ack);
    } else {
      buf_div->ack_flag = false;
      buf_div->is_buf_dirty = false;
    }
  }
  PTHREAD_MUTEX_UNLOCK(&port_data->ack_list.list_mutex);
  mct_list_free_list(event_port_list);
  event->identity = orig_identity;
  return 0;
}

int32_t base_port_handle_buf_divert_ack(mct_port_t *port,
                                        mct_event_t *event)
{
  int32_t rc;
  boolean ret;
  isp_buf_divert_ack_t* buf_div_ack =
    (isp_buf_divert_ack_t*)(event->u.module_event.module_event_data);

  base_port_data_t *port_data = MCT_OBJECT_PRIVATE(port);

  PP_DBG("%s identity=%x, frameid=%d, buf_idx=%d",MCT_MODULE_NAME(port), event->identity,
          buf_div_ack->frame_id, buf_div_ack->buf_idx);

  base_port_stream_data_t *stream_data =
    base_port_get_stream_data(port, event->identity);
  if (!stream_data) {
    PP_ERR("failed");
    return -1;
  }
  uint32_t orig_identity = event->identity;

  mct_port_t *int_link = stream_data->int_links->data;
  base_port_data_t *int_link_data = MCT_OBJECT_PRIVATE(int_link);

  PTHREAD_MUTEX_LOCK(&int_link_data->ack_list.list_mutex);
  ack_list_entry_t ack_key;
  ack_key.buf_idx = buf_div_ack->buf_idx;
  ack_key.frameid = buf_div_ack->frame_id;
  mct_list_t *templist = mct_list_find_custom(int_link_data->ack_list.list,
                                              &ack_key, ack_find_func);

  if (templist && templist->data) {
    ack_list_entry_t *ack = (ack_list_entry_t *) templist->data;
    ack->refcount--;
    ack->is_buf_dirty = ack->is_buf_dirty || buf_div_ack->is_buf_dirty;
    // We should OR the buffer_access flags here because, the ACK events are
    // coming async and buffer_access flag from one pipeline is not being
    // handled in other. We may be doing few duplicate operations, but this
    // cannot be avoided.
    ack->buffer_access = ack->buffer_access | buf_div_ack->buffer_access;
    if (ack->refcount == 0) {
      int_link_data->ack_list.list =
        mct_list_remove(int_link_data->ack_list.list, ack);
      buf_div_ack->is_buf_dirty =
        buf_div_ack->is_buf_dirty || ack->is_buf_dirty;
      buf_div_ack->buffer_access =
        buf_div_ack->buffer_access | ack->buffer_access;
      event->identity = ack->identity; /* send event on different identity */
      if (int_link_data->ack_list.size) {
        int_link_data->ack_list.size--;
      }
      PP_DBG("remove ack list, identity=%x, frame_id=%d, idx=%d, size:%d\n",
        ack->identity, ack->frameid, ack->buf_idx, port_data->ack_list.size);
      PTHREAD_MUTEX_UNLOCK(&int_link_data->ack_list.list_mutex);

      free(ack);
      /* send the event to internal links */
      ret = mct_list_traverse(stream_data->int_links,
                              forward_to_int_link_trav_func, event);
      if (!ret) {
        PP_ERR("failed");
        return -1;
      }
      event->identity = orig_identity; /* restore original identity */
    } else {
      PTHREAD_MUTEX_UNLOCK(&int_link_data->ack_list.list_mutex);
    }
  } else {
    PP_ERR("invalid condition, no ack found in list "
           "for identity=%x, frameid=%d", event->identity,
            buf_div_ack->frame_id);
    PTHREAD_MUTEX_UNLOCK(&int_link_data->ack_list.list_mutex);
    return -1;
  }
  return 0;
}

static boolean base_port_src_event_func(mct_port_t *port, mct_event_t *event)
{
  int32_t rc=0;
  switch (event->direction) {
    case MCT_EVENT_UPSTREAM:
      if (event->type == MCT_EVENT_MODULE_EVENT &&
          event->u.module_event.type == MCT_EVENT_MODULE_BUF_DIVERT_ACK) {
          rc = base_port_handle_buf_divert_ack(port, event);
      } else {
        rc = base_port_fwd_event_to_intlinks(port, event);
        if (rc < 0) {
          PP_ERR("failed");
        }
      }
      break;
    default:
      PP_ERR("invalid");
      return mct_port_send_event_to_peer(port, event);
  }
  if (rc < 0) {
    return false;
  }
  return true;
}

static boolean base_port_sink_event_func(mct_port_t *port, mct_event_t *event)
{
  int32_t rc=0;
  switch (event->direction) {
    case MCT_EVENT_DOWNSTREAM:
      if (event->type == MCT_EVENT_MODULE_EVENT &&
          event->u.module_event.type == MCT_EVENT_MODULE_BUF_DIVERT) {
          rc = base_port_handle_buf_divert(port, event);
      } else if (event->type == MCT_EVENT_CONTROL_CMD &&
        (event->u.ctrl_event.type == MCT_EVENT_CONTROL_STREAMON ||
        event->u.ctrl_event.type == MCT_EVENT_CONTROL_STREAMOFF)) {
        base_port_stream_data_t *stream_data =
          base_port_get_stream_data(port, event->identity);
        if (stream_data) {
          stream_data->streamon =
            (event->u.ctrl_event.type == MCT_EVENT_CONTROL_STREAMON);
        }
        rc = base_port_fwd_event_to_intlinks(port, event);
        if (rc < 0)
          PP_ERR("failed");
      } else {
        rc = base_port_fwd_event_to_intlinks(port, event);
        if (rc < 0) {
          PP_ERR("failed");
        }
      }
      break;
    default:
      PP_ERR("invalid");
      return mct_port_send_event_to_peer(port, event);
  }
  if (rc < 0) {
    return false;
  }
  return true;
}

int32_t base_port_init_port_data(mct_port_t *port)
{
  base_port_data_t *port_data = MCT_OBJECT_PRIVATE(port);
  int32_t i;

  for (i=0; i<BASE_PORT_MAX_STREAMS; i++) {
    port_data->stream_data[i].state = BASE_PORT_STATE_UNRESERVED;
  }

  return 0;
}

mct_port_t *base_port_create(const char* name, mct_port_direction_t dir)
{
  mct_port_t  *port;
  base_port_data_t* port_data;
  uint32_t i;
  port = mct_port_create(name);

  if(!port) {
    PP_ERR("failed");
    goto port_create_error;
  }
  port->direction = dir;
  port->check_caps_reserve = base_port_check_caps_reserve;
  port->check_caps_unreserve = base_port_check_caps_unreserve;
  port->ext_link = base_port_ext_link_func;
  port->un_link = base_port_ext_unlink_func;

  port->event_func = (dir == MCT_PORT_SRC) ?
    base_port_src_event_func : base_port_sink_event_func;

  port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;

  port_data = (base_port_data_t*) CAM_MALLOC(sizeof(base_port_data_t));
  if(!port_data) {
    PP_ERR("failed");
    goto port_data_error;
  }
  memset(port_data, 0x00, sizeof(base_port_data_t));
  pthread_mutex_init(&port_data->ack_list.list_mutex, NULL);
  MCT_OBJECT_PRIVATE(port) = port_data;

  base_port_init_port_data(port);

  return port;

port_data_error:
  mct_port_destroy(port);
port_create_error:
  return NULL;
}

int32_t base_port_destroy(mct_port_t *port)
{
  base_port_data_t* port_data;

  if(!port) {
    PP_ERR("failed");
    return -1;
  }

  port_data = (base_port_data_t *) MCT_OBJECT_PRIVATE(port);

  if (!port_data) {
    PP_ERR("failed");
    return -1;
  }
  pthread_mutex_destroy(&port_data->ack_list.list_mutex);
  free(port_data);
  return 0;
}
