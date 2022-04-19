/* stats_util.c
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "stats_util.h"

static void circular_stats_data_ack(mct_port_t *port, mct_event_stats_ext_t* stats_ext_event,
                                    uint32_t identity, uint32_t frame_id);

/* Call this function before use the stats_ext_event, it shall add 1 to the ref_count
 * @return  the count of stats_ext_event
 */
int32_t circular_stats_data_use(mct_event_stats_ext_t* stats_ext_event)
{
  int32_t count = 0;
  pthread_mutex_lock(stats_ext_event->stats_mutex);
  count = ++stats_ext_event->ref_count;
  pthread_mutex_unlock(stats_ext_event->stats_mutex);

  STATS_LOW("count %d stats_ext_event %p isp_event %p stats_mask = 0x%x",
            count, stats_ext_event, stats_ext_event->stats_data,
            stats_ext_event->stats_data->stats_mask);
  return count;
}

/* Call this function after finish using the stats data, it shall decrease ref_count by 1
 * @return  the count of stats_ext_event after decreasing
 */
int32_t circular_stats_data_done(mct_event_stats_ext_t* stats_ext_event, mct_port_t *port,
                                 uint32_t identity, uint32_t frame_id)
{
  if (!stats_ext_event) {
    STATS_HIGH("empty parameter!!!");
    return 0;
  }

  int32_t count = 0;

  pthread_mutex_lock(stats_ext_event->stats_mutex);
  count = --stats_ext_event->ref_count;

  STATS_LOW("count %d stats_ext_event %p isp_event %p stats_mask = 0x%x",
            count, stats_ext_event, stats_ext_event->stats_data,
            stats_ext_event->stats_data->stats_mask);
  pthread_mutex_unlock(stats_ext_event->stats_mutex);

  if (count == 0) {
    circular_stats_data_ack(port, stats_ext_event, identity, frame_id);
    free(stats_ext_event);
    stats_ext_event = NULL;
  }

  return count;
}

static void circular_stats_data_ack(mct_port_t *port, mct_event_stats_ext_t* stats_ext_event, uint32_t identity, uint32_t frame_id)
{
  if (!port || !stats_ext_event) {
    STATS_ERR("empty parameter!!!");
    return;
  }

  mct_event_t       ack_event;

  /* pack into an mct_event object*/
  ack_event.direction = MCT_EVENT_UPSTREAM;
  ack_event.identity = identity;
  ack_event.type = MCT_EVENT_MODULE_EVENT;
  ack_event.u.module_event.current_frame_id = frame_id;
  ack_event.u.module_event.type = MCT_EVENT_MODULE_STATS_DATA_ACK;
  ack_event.u.module_event.module_event_data = stats_ext_event->stats_data;

  STATS_LOW("send MCT_EVENT_MODULE_STATS_DATA_ACK upstream,"
    "port =%p, stats_ext_event %p isp_event %p stats_mask = 0x%x", port,
    ack_event.u.module_event.module_event_data, stats_ext_event->stats_data,
    stats_ext_event->stats_data->stats_mask);
  MCT_PORT_EVENT_FUNC(port)(port, &ack_event);

  return;
}

/*Below Utility functionality has been taken from sensor_util.c
* Ideally MCT Should be having this as a utility functionality for
* All the modules to be using
* Stats is just using the functions  with slight modifications on
* using port instead of module
*/

/**  stats_util_unpack_identity:
 *    unpacks the 32-bit identity in session_id and stream_id
 *
 *  Arguments:
 *    @identity: input param
 *    @session_id: output param
 *    @stream_id: output param
 *
 *  Return: TRUE on success
 *          FALSE on failure **/

boolean stats_util_unpack_identity(uint32_t identity,
  uint32_t *session_id, uint32_t *stream_id)
{
  if (!session_id || !stream_id) {
    STATS_ERR("failed session_id %p stream_id %p",
      session_id, stream_id);
    return FALSE;
  }
  *stream_id = (identity & 0x0000FFFF);
  *session_id = ((identity & 0xFFFF0000) >> 16);
  STATS_LOW("session_id %d stream id %d", *session_id, *stream_id);
  return TRUE;
}

boolean stats_util_post_intramode_event(mct_port_t *src_port,
  uint32_t peer_identity,mct_event_module_type_t type, void *data)
{
  mct_event_t              event;
  mct_inter_peer_entity_t *intra_port_entity;
  mct_port_t              *intra_port;
  uint8_t                  i = 0;

  if (!src_port) {
    STATS_ERR("failed NULL pointer detected ");
    return FALSE;
  }

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = peer_identity;
  event.direction = MCT_EVENT_INTRA_MOD;
  event.u.module_event.type = type;
  event.u.module_event.module_event_data = data;

  return mct_port_send_intra_module_event(src_port, &event);
}

boolean stats_util_get_peer_port(mct_event_t *event,
  uint32_t peer_identity, mct_port_t *this_port, mct_port_t **peer_port)
{
  uint32_t session_id = 0, stream_id = 0;

  if (!event || !this_port || !peer_port) {
    STATS_ERR("failed NULL pointer detected ");
    return FALSE;
  }

  stats_util_unpack_identity(event->identity, &session_id, &stream_id);
  STATS_LOW("this session_id:%d stream:%d", session_id, stream_id);

  *peer_port = mct_port_find_intra_port_across_session(MCT_PORT_SINK, this_port,
    peer_identity);

  if (*peer_port == NULL || *peer_port == this_port) {
    STATS_ERR("can't find peer port(the src port of session id %d)",
      peer_identity >> 16);
    return FALSE;
  }

  STATS_HIGH("LINK sessions %d(port %p) and %d(port %p)",
    session_id, *this_port, peer_identity >> 16, *peer_port);

  return TRUE;
}
