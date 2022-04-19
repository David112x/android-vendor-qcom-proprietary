/* mct_port.c
 *
 * This file contains the default infrastructure and implementation for
 * the ports. Some of the functions defined here may be over-ridden
 * by the respective port objects.
 *
 * Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mct_port.h"
#include "mct_module.h"
#include "mct_stream.h"
#include "camera_dbg.h"
#include "mct_profiler.h"

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean compare_port_number(void *data1, void *data2)
{
  unsigned int *ids  = (unsigned int *)data1;
  unsigned int *data = (unsigned int *)data2;
  if (*ids == *data)
    return TRUE;

  return FALSE;
}

/** Name: mct_port_compare_identity
 *
 *  Arguments/Fields:
 *
 *    @d1: Identity of one of port's children
 *    @d2: Identity to be matched
 *
 *  Return:
 *     TRUE on success, FALSE on failure
 *
 *  Description:
 *      Match given identity with current entry in list
 *
 **/
static boolean mct_port_compare_identity(void *data1, void *data2)
{
  uint32_t identity1;
  uint32_t identity2;

  if (!data1 || !data2) {
    CLOGE(CAM_MCT_MODULE, "failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }
  identity1 = *((uint32_t *)data1);
  identity2 =  *((uint32_t *)data2);
  return ((identity1 == identity2) ? TRUE : FALSE);
}


/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean find_linked_port(void *data1, void *data2)
{
  mct_port_t *port   = MCT_PORT_CAST(data1);
  unsigned int *info = (unsigned int *)(data2);
  if (mct_list_find_custom(MCT_PORT_CHILDREN(port),
      info, compare_port_number))
    return TRUE;

  return FALSE;
}

/** Name: find_stream_by_index
 *
 *  Arguments/Fields:
 *    @data1: Pointer to stream
 *    @stream_index: streamid
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 *    Returns the stream in list of module parents whose stream id
 *    matches query argument.
 *
 **/
static boolean find_stream_by_index(void *data1, void *data2)
{
  mct_stream_t *stream = (void *)data1;
  uint32_t *stream_index = (uint32_t *)data2;

  if (!stream || !stream_index) {
    CLOGE(CAM_MCT_MODULE, "NULL ptr detected:  stream [%p], stream_index [%p]",
      stream, stream_index);
    return FALSE;
  }
  if ( (stream->streaminfo.identity & 0x0000FFFF) == *stream_index) {
    return TRUE;
  }
  return FALSE;
}

/** Name: mct_port_find_port_with_matching_identity
 *
 *  Arguments/Fields:
 *    @data1: Current port under module list
 *    @data2: Identity to be matched
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 *    Matches the given identity with all identities
 *    registered with the current port (as children).
 *
 **/
boolean mct_port_find_port_with_matching_identity(void *data1, void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  uint32_t *identity = (uint32_t *)data2;

  if (!data1 || !data2) {
    CLOGE(CAM_MCT_MODULE, "failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }

  if (mct_list_find_custom(MCT_PORT_CHILDREN(port), identity,
    mct_port_compare_identity)){
    return TRUE;
  }
  return FALSE;
}


/** Name: mct_port_find_intra_port_within_session
 *
 *  Arguments/Fields:
 *    @port: Current port pointer
 *    @identity: Identity of session on which to find intra-port
 *    @intra_ports: Container of possible intra-ports for given identity
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 *    !!!CAUTION!!!
 *    Use this function for ports on sub-modules within a module only.
 *    Finds the appropriate intra-port(s) for a given session identity.
 *    For sub-modules within a module these are usually the existing
 *    peer and internal_peer pointers.
 *    For external ports, this function cannot be used as their peer
 *    is already pointing to the external port of its neighboring module.
 *
 **/
boolean mct_port_find_intra_port_within_session(mct_port_t* port,
  mct_port_t ** intra_ports, uint32_t identity)
{
  boolean status = FALSE;
  uint8_t index = 0;
  uint32_t session_id, cur_session_id;
  uint32_t session_stream_index = MCT_SESSION_STREAM_ID;
  mct_stream_t *session_stream = NULL;
  mct_list_t *stream_list = NULL;

  if (!port || !intra_ports) {
    CLOGE(CAM_MCT_MODULE, "NULL ptr detected: port = [%p], intra_ports = [%p]",
      port, intra_ports);
    return FALSE;
  }
  /* Find the identity of current session */
  mct_module_t *module = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!module) {
    CLOGE(CAM_MCT_MODULE, "Port [%p] doesn't have a parent", port);
    return FALSE;
  }

  stream_list = mct_list_find_custom(MCT_MODULE_PARENT(module),
    &session_stream_index, find_stream_by_index);
  if (!stream_list) {
    CLOGE(CAM_MCT_MODULE, "Session stream not linked on module %s",
      MCT_MODULE_NAME(module));
    return FALSE;
  }
  if (!stream_list->data) {
    CLOGE(CAM_MCT_MODULE, "failed stream_list->data %p", stream_list->data);
    return FALSE;
  }
  session_stream = (mct_stream_t *)stream_list->data;
  cur_session_id = UNPACK_SESSION_ID(session_stream->streaminfo.identity);
  session_id = UNPACK_SESSION_ID(identity);

  /*
  'peer' is the intra-port for INTRA_MOD events on the same session.
  'internal_peer' is the set of intra-ports for INTRA_MOD events on a
  different session.
  */
  if (session_id == cur_session_id) {
    intra_ports[0] = port->peer;
    status = TRUE;
  } else {
    for (index = 0; index < MAX_PORT_CONNECTIONS; index++) {
      if (port->internal_peer[index]) {
        intra_ports[index] = port->internal_peer[index];
        status = TRUE;
      }
    }
  }
  return status;
}

/** Name: mct_port_find_intra_port_across_session
 *
 *  Arguments/Fields:
 *    @port_type: Type of port (SRC/SINK) on which to look for
 *       intra-session ports.
 *    @port: Current port pointer
 *    @identity: Identity of session on which to find intra-port
 *
 *  Return:
 *    Pointer to port across session if found successfully, NULL otherwise
 *
 *  Description:
 *    Use this function for finding ports across other session.
 *    Could be used at module or sub-module port interfaces.
 *    Finds the appropriate intra-port(s) for a given session identity
 *    by querying for ports of the specified port_type for matching
 *    identity.
 *
 **/
mct_port_t * mct_port_find_intra_port_across_session(
  mct_port_direction_t port_type, mct_port_t *port, uint32_t identity)
{
  mct_port_t *intra_port = NULL;
  mct_list_t *port_list = NULL;
  mct_list_t *port_holder = NULL;
  if (!port) {
    CLOGE(CAM_MCT_MODULE, "Invalid port ptr");
    return NULL;
  }
  mct_module_t *module = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!module) {
    CLOGE(CAM_MCT_MODULE, "Port [%p] doesn't have a parent", port);
    return NULL;
  }
  /* Choose which type of port to search for inter-session peers */
  if (MCT_PORT_SRC == port_type) {
    port_list = MCT_MODULE_SRCPORTS(module);
  } else if (MCT_PORT_SINK == port_type) {
    port_list = MCT_MODULE_SINKPORTS(module);
  } else {
    CLOGE(CAM_MCT_MODULE, "Invalid port type: %d", port_type);
    return NULL;
  }

  port_holder = mct_list_find_custom(port_list, &identity,
    mct_port_find_port_with_matching_identity);

  if (port_holder) {
    intra_port = (mct_port_t *)port_holder->data;
    CLOGD(CAM_MCT_MODULE, "Found intra_port [%p] for identity 0x%x",
      intra_port, identity);
  } else {
    CLOGD(CAM_MCT_MODULE, "Could not find an intra_port for identity 0x%x",
      identity);
  }
  return intra_port;
}

/** Name: mct_port_send_event_to_peer
 *
 *  Arguments/Fields:
 *    @port: Current port pointer
 *    @event: Event to be forwarded to peer port
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 *    Forwards UPSTREAM/DOWNSTREAM events on the peer port.
 *
 **/

boolean mct_port_send_event_to_peer(mct_port_t *port, mct_event_t *event)
{
  boolean status;

  if (!port || !event) {
    CLOGE(CAM_MCT_MODULE, "NULL ptr detected: port [%p], event [%p]",
      port, event);
    return FALSE;
  }

  if (!MCT_PORT_PEER(port) || !MCT_PORT_EVENT_FUNC(MCT_PORT_PEER(port)))
    return FALSE;

  if ((event->type == MCT_EVENT_CONTROL_CMD) && (KPI_TRACE_DBG)) {
    char traceTag[16];
    snprintf(traceTag, sizeof(traceTag),
             "EventCmd%d", event->u.ctrl_event.type);
    ATRACE_INT(traceTag, ++event->logNestingIndex);
    status =
      MCT_PORT_EVENT_FUNC(MCT_PORT_PEER(port))(MCT_PORT_PEER(port), event);
    ATRACE_INT(traceTag, --event->logNestingIndex);

  } else {
    status =
      MCT_PORT_EVENT_FUNC(MCT_PORT_PEER(port))(MCT_PORT_PEER(port), event);
  }

  return status;
}

/** Name: mct_port_send_intra_module_event
 *
 *  Arguments/Fields:
 *    @port: Current port pointer
 *    @event: Intra-module event to be forwarded
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 *    Forwards INTRA_MOD type event on the correct intra-session
 *    communication port for the current port.
 *
 **/

boolean mct_port_send_intra_module_event(mct_port_t *port, mct_event_t *event)
{
  boolean status =FALSE;
  uint8_t index = 0;
  mct_port_t *intra_port = NULL;

  if (!port || !event) {
    CLOGE(CAM_MCT_MODULE, "NULL ptr detected: port [%p], event [%p]",
      port, event);
    return FALSE;
  }
  if (MCT_EVENT_INTRA_MOD != event->direction) {
    CLOGE(CAM_MCT_MODULE, "Incompatible direction type %d", event->direction);
    return FALSE;
  }
  /* Discover intra-session port to forward event on */
  for (index = 0; index < MAX_PORT_CONNECTIONS; index++) {
    if (port->inter_session_peer[index].identity == event->identity) {
      intra_port = port->inter_session_peer[index].port;
      if (intra_port) {
        status = MCT_PORT_INTRA_EVENT_FUNC(intra_port)(intra_port, event);
      }
    }
  }
  return status;
}

/** Name: mct_port_event_default
 *
 *  Arguments/Fields:
 *  @port:   the #MctPort to handle the event.
 *  @event: the #MctEvent to handle.
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 * Function signature to handle an event for the port.
 *
 */
static boolean mct_port_event_default(mct_port_t *port, mct_event_t *event)
{
  boolean result = FALSE;

  if (!port || !event )
     return FALSE;

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_UPSTREAM: {

    if (MCT_PORT_IS_SINK(port)) {
      /* ... Process Event here ... */

      result = mct_port_send_event_to_peer(port, event);

    } else if (MCT_PORT_IS_SRC(port)) {
      mct_list_t *list;

      /* ... Process Event here ... */

      if (MCT_PORT_INTLINKFUNC(port)){
        list = MCT_PORT_INTLINKFUNC(port)(event->identity, port);

        if (list) {
          /* ... Forward event up to internal sink ports if needed ... */
        }
      }
    } /* port direction */

  } /* case MCT_EVENT_TYPE_UPSTREAM */
    break;

  case MCT_EVENT_DOWNSTREAM: {

    if (MCT_PORT_IS_SRC(port)) {
      result = mct_port_send_event_to_peer(port, event);

    } else if (MCT_PORT_IS_SINK(port)) {
      mct_list_t *list = NULL;

      /* ... Process Event here ... */

      if (MCT_PORT_INTLINKFUNC(port))
        list = MCT_PORT_INTLINKFUNC(port)(event->identity, port);

      if (list) {
        /* ... Forward event down to internal sink ports if needed ... */
      }
    }/* port direction */
  } /* case MCT_EVENT_TYPE_DOWNSTREAM */
    break;

  default:
    result = FALSE;
    break;
  }

  return result;
}

/**
 * mct_port_event_fwd_list_default:
 *
 * Iterate the list of ports to which the given port is linked to inside of
 * the parent Module.
 *
 * This is the default handler, and thus returns an MctList of all of the
 * ports inside the parent module with opposite direction.
 *
 * Port MUST implement this function
 *
 * The caller must free this MctList after use it.
 */
static mct_list_t *
mct_port_event_fwd_list_default(
   unsigned int identity __unused,
   mct_port_t *port __unused)
{
  mct_list_t *selected_mct_list = NULL;
#if 0
  mct_list_t *complete_list;
  mct_port_session_stream_t session_stream;

  mct_module_t *module = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);

  if (MCT_PORT_IS_SRC(port))
    complete_list = MCT_MODULE_SINKPORTS(module);
  else if (MCT_PORT_IS_SINK(port))
    complete_list = MCT_MODULE_SRCPORTS(module);
  else
    return NULL;

  session_stream.sessionid = sessionid;
  session_stream.streamid = streamid;
  selected_mct_list = mct_list_find_and_add_custom(complete_list,
    child_list, &session_stream, find_linked_port);
#endif
  return selected_mct_list;
}

 /** Name: mct_port_external_links_default
 *
 *  Arguments/Fields:
 *  @port: the #MctPort to handle the event.
 *  @peer: the peer #MctPort of the link
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 *    Function signature to handle a new external link on the port. The
 *    defualt implementation here just stores the session and stream ids.
 *    This function should be overridden by every port.
 *
 */
static boolean mct_port_external_links_default(unsigned int identity __unused,
  mct_port_t *port, mct_port_t *peer)
{
  if (!MCT_PORT_PEER(port)) {
    MCT_PORT_PEER(port)  = peer;
  } else { /*the link has already been established*/
    if ((MCT_PORT_PEER(port) != peer))
    goto FAIL;
  }

  return TRUE;
FAIL:
  return FALSE;
}

/** Name: mct_port_unlink_default
*
*  Arguments/Fields:
*  @port: the #MctPort to handle the event.
*  @peer: the peer #MctPort of the link
*
*  Return:
*    TRUE on success, FALSE on failure
*
*  Description:
*    Function signature to unlink any established links on the port.
*
*/

static void mct_port_unlink_default(unsigned int identity __unused,
  mct_port_t *port, mct_port_t *peer)
{
  uint8_t index = 0;
  if (port->peer != peer)
    return;

  if (port->port_private == NULL)
    port->peer  = NULL;
  /* Clear inter-session entries if any */
  for (index = 0; index < MAX_PORT_CONNECTIONS; index++) {
    MCT_OBJECT_LOCK(port);
    port->inter_session_peer[index].is_filled = FALSE;
    port->inter_session_peer[index].identity = 0;
    port->inter_session_peer[index].port = NULL;
    MCT_OBJECT_UNLOCK(port);
  }
  return;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_port_set_caps_default(mct_port_t *port __unused,
  mct_port_caps_t *caps __unused)
{
  return TRUE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_port_check_caps_reserve_default(mct_port_t *port __unused,
  void *peer_caps __unused, void *stream_info __unused)
{
  return TRUE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_port_check_caps_unreserve_default(
  mct_port_t *port __unused, unsigned int identity __unused)
{
  return TRUE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_port_check_link_internal(mct_port_t **src,
  mct_port_t **sink)
{
  /* Port should have only one parent:
   * These parents are modules */
  mct_object_t *src_parent, *sink_parent;

  src_parent  = MCT_OBJECT_CAST(MCT_PORT_PARENT(*src)->data);
  sink_parent = MCT_OBJECT_CAST(MCT_PORT_PARENT(*sink)->data);

  if (!src_parent || !sink_parent)
    return FALSE;

  if (mct_object_find_common_parent(src_parent, sink_parent))
    /* it should be good if both modules are on the same stream */
    return TRUE;

  return FALSE;
}

/** Name: mct_port_check_link
 *
 *  Arguments/Fields:
 *    @srcport: Pointer to source port
 *    @sinkport: Pointer to sink port
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 *    Perform basic validations on source port
 *    and sink port.
 *
 **/
boolean mct_port_check_link(mct_port_t *srcport, mct_port_t *sinkport)
{
  /* generic checks */
  if (!srcport || !sinkport)
    return FALSE;

  if (!MCT_PORT_IS_SRC(srcport))
    return FALSE;

  if (!MCT_PORT_IS_SINK(sinkport))
    return FALSE;

  if (MCT_PORT_PARENT(srcport) == NULL)
    return FALSE;

  if (MCT_PORT_PARENT(sinkport) == NULL)
    return FALSE;

  if (srcport->peer != NULL && srcport->peer != sinkport) {
    return FALSE;
  }

  if (sinkport->peer != NULL && sinkport->peer != srcport) {
    return FALSE;
  }
/*
  if (!mct_port_check_link_internal(&srcport, &sinkport))
    return FALSE;
*/

  return TRUE;
}

/** Name: mct_port_add_child
 *
 *  Arguments/Fields:
 *    @identity: Identity of stream to be removed from given port
 *    @port: Pointer to port.
 *
 *  Return: NONE
 *
 *  Description:
 *    Appends entry corresponding to given identity into the list
 *    of port's children.
 *
 **/

boolean mct_port_add_child(unsigned int identity, mct_port_t *port)
{
  unsigned int *p_identity;
  p_identity = malloc(sizeof(unsigned int));
  if (!p_identity)
    return FALSE;
  *p_identity = identity;
  MCT_OBJECT_LOCK(port);
  MCT_PORT_CHILDREN(port) = mct_list_append(
    MCT_PORT_CHILDREN(port), p_identity, NULL, NULL);
  if (!MCT_PORT_CHILDREN(port)) {
    MCT_OBJECT_UNLOCK(port);
    return FALSE;
  }
  MCT_OBJECT_NUM_CHILDREN(port)++;

  MCT_OBJECT_UNLOCK(port);
  return TRUE;
}

/** Name: mct_port_remove_child
 *
 *  Arguments/Fields:
 *    @identity: Identity of stream to be removed from given port
 *    @port: Pointer to port.
 *
 *  Return: NONE
 *
 *  Description:
 *    Removes entry corresponding to given identity from the list
 *    of port's children.
 *
 **/
void mct_port_remove_child(unsigned int identity, mct_port_t *port)
{
  unsigned int *p_identity;
  mct_list_t *identity_holder;

  MCT_OBJECT_LOCK(port);
  identity_holder = mct_list_find_custom(MCT_PORT_CHILDREN(port),
    &identity, compare_port_number);

  if (!identity_holder) {
    MCT_OBJECT_UNLOCK(port);
    return;
  }
  p_identity = (unsigned int *)identity_holder->data;
  MCT_PORT_CHILDREN(port) = mct_list_remove(
    MCT_PORT_CHILDREN(port), p_identity);
  MCT_OBJECT_NUM_CHILDREN(port)--;
  MCT_OBJECT_UNLOCK(port);
  free(p_identity);
  p_identity = NULL;
}

/** Name:mct_port_establish_link
 *
 *  Arguments/Fields:
 *    @identity: Identity of stream on which ports are linked.
 *    @srcport: Pointer to source port in linking.
 *    @sinkport: Pointer to sink port in linking.
 *
 *  Return:
 *      TRUE on success, FALSE on failure
 *
 *  Description:
 *      Sets up external linkage between srcport and sinkport.
 *      Also adds given identity into the list of port children.
 *
 **/
boolean mct_port_establish_link(unsigned int identity,
  mct_port_t *srcport, mct_port_t *sinkport)
{
  if (!srcport || !sinkport)
    return FALSE;

  if (!MCT_PORT_IS_SRC(srcport) || !MCT_PORT_IS_SINK(sinkport))
    return FALSE;

  if (srcport->ext_link && sinkport->ext_link) {
    if (srcport->ext_link(identity, srcport, sinkport) == FALSE)
      goto FAIL;
    if (sinkport->ext_link(identity, sinkport, srcport) == FALSE)
      goto FAIL1;
    if (mct_port_add_child(identity, srcport) == FALSE)
      goto FAIL2;
    if (mct_port_add_child(identity, sinkport) == FALSE)
      goto FAIL3;
  } else
    goto FAIL;

  return TRUE;
FAIL3:
  mct_port_remove_child(identity, srcport);
FAIL2:
  srcport->un_link(identity, sinkport, srcport);
FAIL1:
  srcport->un_link(identity, srcport, sinkport);
FAIL:
  return FALSE;
}

/** Name: mct_port_establish_intra_link_default
 *
 *  Arguments/Fields:
 *    @identity: Identity of session to set up intra-module linkage
 *    @srcport: Pointer of Port establishing intra-mod connection
 *    @sinkport: Pointer of Port on other session to be linked
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *  Description:
 *    This function establishes unique intra-session port linkage
 *    on the session that srcport belongs to. It does so by storing
 *    info about the sinkport including the intra-session identity.
 **/

boolean mct_port_establish_intra_link_default(unsigned int identity,
  mct_port_t *srcport, mct_port_t *sinkport)
{
  uint8_t index = 0;
  boolean ret = FALSE;
  if(!srcport || !sinkport) {
    CLOGE(CAM_MCT_MODULE, "NULL ptr detected: srcport [%p], sinkport [%p]",
      srcport, sinkport);
    return FALSE;
  }
  for (index = 0; index < MAX_PORT_CONNECTIONS; index++) {
    if (TRUE == srcport->inter_session_peer[index].is_filled) {
      /* Check for duplicate entries */
      if ( (srcport->inter_session_peer[index].identity == identity) &&
        (srcport->inter_session_peer[index].port == sinkport) )
        break;
    } else {
      /* Store new entry in inter_session_peer array */
      MCT_OBJECT_LOCK(srcport);
      srcport->inter_session_peer[index].is_filled = TRUE;
      srcport->inter_session_peer[index].identity = identity;
      srcport->inter_session_peer[index].port = sinkport;
      MCT_OBJECT_UNLOCK(srcport);
      return TRUE;
    }
  }
  if (index == MAX_PORT_CONNECTIONS) {
    CLOGE(CAM_MCT_MODULE, "Max intra-session connections [%d] reached!",
      index);
  } else {
    CLOGD(CAM_MCT_MODULE,
          "Entry (%p, 0x%x) is already in the list of intra-peers",
          sinkport, identity);
    ret = TRUE;
  }
  return ret;
}

/** Name: mct_port_remove_intra_link_default
 *
 *  Arguments/Fields:
 *    @identity: Identity of intra-session port to be removed
 *    @port: Pointer to port whose intra-port entry is to be cleared
 *    @intra_port: Pointer to port that needs to be removed.
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 *    Removes the entry in inter_session_peer[] collection that matches
 *    the identity passed as argument to the method.
 **/
boolean mct_port_remove_intra_link_default(unsigned int identity,
  mct_port_t *port)
{
  boolean status = FALSE;
  uint8_t index = 0;
  if(!port) {
    CLOGE(CAM_MCT_MODULE, "Invalid port pointer");
    return FALSE;
  }
  for (index = 0; index < MAX_PORT_CONNECTIONS; index++) {
    if (port->inter_session_peer[index].identity == identity) {
      MCT_OBJECT_LOCK(port);
      port->inter_session_peer[index].is_filled = FALSE;
      port->inter_session_peer[index].identity = 0;
      port->inter_session_peer[index].port = NULL;
      MCT_OBJECT_UNLOCK(port);
      status = TRUE;
    }
  }
  return status;
}

/** Name: mct_port_destroy_link
 *
 *  Arguments/Fields:
 *    @identity: Identity on which ports are linked
 *    @srcport: SRC port on which unlink is issued to remove SINK port
 *    @sinkport: SINK port on which unlink is issued to remove SRC port
 *
 *  Return: NONE
 *
 *  Description:
 *    Performs the unlinking of ports in the specified order.
 **/

void mct_port_destroy_link(unsigned int identity,
  mct_port_t *srcport, mct_port_t *sinkport)
{
  if (!srcport || !sinkport ||
      !MCT_PORT_IS_SRC(srcport) ||
      !MCT_PORT_IS_SINK(sinkport))
    return;

  if (srcport->un_link)
    srcport->un_link(identity, srcport, sinkport);
  if (sinkport->un_link)
    sinkport->un_link(identity, sinkport, srcport);
  return;
}

/** Name: mct_port_init_default
 *
 *  Arguments/Fields:
 *    @port: Port pointer to initialize
 *
 *  Return: NONE
 *
 *  Description:
 *    Initializes port fields and assigns default functions
 *    to exposed function pointers.
 **/
static void mct_port_init_default(mct_port_t *port)
{
  MCT_PORT_DIRECTION(port) = MCT_PORT_UNKNOWN;
  memset(port->inter_session_peer, 0,
    MAX_PORT_CONNECTIONS * sizeof (mct_inter_peer_entity_t));
  mct_port_set_event_func(port, mct_port_event_default);
  mct_port_set_intra_event_func(port, mct_port_send_intra_module_event);
  mct_port_set_int_link_func(port, mct_port_event_fwd_list_default);
  mct_port_set_ext_link_func(port, mct_port_external_links_default);
  mct_port_set_intra_link_func(port, mct_port_establish_intra_link_default);
  mct_port_set_unlink_func(port, mct_port_unlink_default);
  mct_port_set_intra_unlink_func(port, mct_port_remove_intra_link_default);
  mct_port_set_set_caps_func(port, mct_port_set_caps_default);
  mct_port_set_check_caps_reserve_func
    (port, mct_port_check_caps_reserve_default);
  mct_port_set_check_caps_unreserve_func
    (port, mct_port_check_caps_unreserve_default);
}

/** Name: mct_port_create
 *
 *  Arguments/Fields:
 *    @name: Port Name
 *
 *  Return:
 *    Pointer to newly created port of type mct_port_t
 *  Description:
 *    Allocates memory for new port and assigns name provided as argument
 **/
mct_port_t *mct_port_create(const char *name)
{
  mct_port_t *port;

  port = malloc(sizeof(mct_port_t));
  if (!port)
    return port;

  memset(port, 0, sizeof(mct_port_t));
  pthread_mutex_init(MCT_OBJECT_GET_LOCK(port), NULL);
  mct_object_set_name(MCT_OBJECT_CAST(port), name);
  mct_port_init_default(port);

  return port;
}

/** Name: mct_port_destroy
 *
 *  Arguments/Fields:
 *    @name: Port Name
 *
 *  Return: NONE
 *
 *  Description:
 *    Destroys port pointer.
 **/
void mct_port_destroy(mct_port_t *port)
{
  pthread_mutex_destroy(MCT_OBJECT_GET_LOCK(port));
  free(MCT_PORT_NAME(port));
  free(port);
  port = NULL;
  return;
}
