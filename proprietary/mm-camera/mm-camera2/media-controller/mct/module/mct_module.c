/* mct_module.c
 *
 * This file contains the default infrastructure and implementation for
 * the modules. Some of the functions defined here may be over-ridden
 * by the respective module objects.
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mct_event_stats.h"
#include "mct_module.h"
#include "camera_dbg.h"
#include "mct_stream.h"
#include "mct_pipeline.h"
#include "mct_profiler.h"
#include "supermct.h"

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
typedef struct _mct_module_custom {
  /* mct_port_session_stream_t session_stream; */
  unsigned int identity;
  void *stream_info;
  mct_module_t *dest_mod;
  mct_port_t *port;
  boolean valid_src_port;
/* mct_list_t *port_list; */
} mct_module_custom_t;

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
static boolean mct_module_compare_port_number(void *data1, void *data2)
{
  unsigned int *ids = (unsigned int *)(data1);
  unsigned int *info = (unsigned int *)(data2);
  if (*ids == *info)
    return TRUE;

  return FALSE;
}

/** mct_module_find_connected_dest_port
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
static boolean mct_module_find_connected_dest_port(void *data1, void *data2)
{
  mct_port_t *port = MCT_PORT_CAST(data1);
  mct_module_custom_t *custom_data = (mct_module_custom_t *)(data2);
  mct_port_t *srcport;

  if (mct_list_find_custom(MCT_PORT_CHILDREN(port), &custom_data->identity,
    mct_module_compare_port_number) == NULL)
    return FALSE;

  srcport = custom_data->port;
  if ((srcport->peer == port) && (port->peer == srcport))
    return TRUE;

  return FALSE;
}

/** mct_module_find_connected_src_port
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
static boolean mct_module_find_connected_src_port(void *data1, void *data2)
{
  mct_port_t *port = MCT_PORT_CAST(data1);
  mct_module_custom_t *custom_data = (mct_module_custom_t *)(data2);
  mct_module_custom_t local_custom;
  mct_list_t *destports, *destport_holder;

  destports = MCT_MODULE_SINKPORTS(custom_data->dest_mod);

  if (mct_list_find_custom(MCT_PORT_CHILDREN(port), &custom_data->identity,
    mct_module_compare_port_number) == NULL)
    return FALSE;

  local_custom = *custom_data;
  local_custom.port = port;
  destport_holder = mct_list_find_custom(destports, &local_custom,
    mct_module_find_connected_dest_port);

  if (!destport_holder)
    return FALSE;

  custom_data->port = MCT_PORT_CAST(destport_holder->data);

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
static boolean mct_module_find_linked_port(void *data1, void *data2)
{
  mct_port_t *port = MCT_PORT_CAST(data1);
  unsigned int *info = (unsigned int *)(data2);
  if (mct_list_find_custom(MCT_PORT_CHILDREN(port), info,
    mct_module_compare_port_number))
    return TRUE;

  return FALSE;
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
boolean mct_module_send_event(mct_module_t *module, mct_event_t *event)
{
  boolean result = FALSE;
  unsigned int info;
  mct_port_t *dest_port = NULL;
  mct_list_t *port_container;

  info = event->identity;

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_UPSTREAM: {
    /* The first event into a SRC module cannot be upstream,
     * since it has no sink ports.
     */
    result = FALSE;
  } /*MCT_EVENT_TYPE_UPSTREAM*/
    break;

  case MCT_EVENT_DOWNSTREAM: {
    if (MCT_MODULE_NUM_SRCPORTS(module) <= 0) {
      return FALSE;
    }
    /*find src dest_port corresponding to this stream ID*/
    port_container = mct_list_find_custom(MCT_MODULE_SRCPORTS(module), &info,
      mct_module_find_linked_port);
    if (!port_container) {
      CLOGE(CAM_MCT_MODULE,
            "Could not find port for module =%s and identity = 0x%x",
        MCT_MODULE_NAME(module), info);
      return FALSE;
    }
    dest_port = MCT_PORT_CAST(port_container->data);
    if (!dest_port || !dest_port->event_func) {
      CLOGE(CAM_MCT_MODULE, "Port cannot handle event");
      return FALSE;
    }
    result = dest_port->event_func(dest_port, event);
    break;
  } /*MCT_EVENT_TYPE_DOWNSTREAM*/

  default: {
    CLOGD(CAM_MCT_MODULE, "Unhandled event %d", MCT_EVENT_DIRECTION(event));
    result = FALSE;
    break;
  } /*default*/
  }

  return result;
}

/*Virtual function default implementations start here*/

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
static boolean mct_module_process_event_default(mct_module_t *module,
  mct_event_t *event)
{
  boolean result = FALSE;

  if (!module || !event) {
    return FALSE;
  }
  /*process event here; the default has no implementation*/
  result = mct_module_send_event(module, event);

  return result;
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
static void mct_module_set_mod_default(mct_module_t *module,
  unsigned int module_type, unsigned int identity)
{
  mct_module_add_type(module, module_type, identity);
  if (module_type == MCT_MODULE_FLAG_SOURCE) {
    mct_module_set_process_event_func(module,
      mct_module_process_event_default);
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
static boolean mct_module_query_mod_default(mct_module_t *module __unused,
  void *query_buf __unused, unsigned int sessionid __unused)
{
  /*TO DO*/
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
static mct_port_t *mct_module_request_port_default(void *stream_info __unused,
  unsigned int direction __unused, mct_module_t *module __unused)
{
  /*TO DO*/
  return NULL;
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
static boolean mct_module_start_session_default(mct_module_t *module __unused,
  unsigned int sessionid __unused)
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
static boolean mct_module_stop_session_default(mct_module_t *module __unused,
  unsigned int sessionid __unused)
{
  return TRUE;
}

/** mct_module_get_stream_info:
 *    @
 *    @
 *
 **/
void *mct_module_get_stream_info(mct_module_t *module, unsigned int session_id,
  int32_t stream_id)
{
  mct_stream_t *stream = NULL;

  stream = mct_pipeline_find_stream(module, session_id);
  if (stream) {
    mct_pipeline_t *pipeline =
      MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
    if (!pipeline) {
      CLOGE(CAM_MCT_MODULE, "Pipeline not found");
      return NULL;
    }
    stream = mct_pipeline_find_stream_from_stream_id(pipeline, stream_id);
    if (!stream) {
      CLOGE(CAM_MCT_MODULE, "Stream NULL for session id =%d stream id =%d",
        session_id, stream_id);
      return NULL;
    }
  } else {
      CLOGE(CAM_MCT_MODULE, "Stream not found for session id =%d", session_id);
      return NULL;
  }

  return &stream->streaminfo;
}

/** mct_module_get_buffer_ptr:
 *    @
 *    @
 *
 **/
void *mct_module_get_buffer_ptr(uint32_t buf_idx, mct_module_t *module,
  unsigned int session_id, unsigned int stream_id)
{
  mct_stream_map_buf_t *current_buf;

  current_buf = mct_module_get_buffer(buf_idx, module, session_id, stream_id);

  return current_buf ? current_buf->buf_planes[0].buf : NULL;
}

/** mct_module_get_buffer:
 *    @
 *    @
 *
 *    Get the buffer of type mct_stream_map_buf_t
 *    based on stream id and session id
 **/
void *mct_module_get_buffer(uint32_t buf_idx, mct_module_t *module,
  unsigned int session_id, unsigned int stream_id)
{
  mct_stream_t *stream = NULL;

  stream = mct_pipeline_find_stream(module, session_id);
  if (stream) {
    CLOGD(CAM_MCT_MODULE, "found stream: stream=%p", stream);
    mct_pipeline_t *pipeline =
      MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
    if (!pipeline)
      return FALSE;
    return mct_pipeline_get_buffer(pipeline, buf_idx, stream_id);
  }
  return NULL;
}

/** mct_module_post_bus_msg:
 *    @
 *    @
 *
 **/
boolean mct_module_post_bus_msg(mct_module_t *module, mct_bus_msg_t *bus_msg)
{
  mct_stream_t *stream = NULL;
  mct_stream_t *meta_stream = NULL;
  mct_stream_t *offline_meta_stream = NULL;
  mct_pipeline_get_stream_info_t info;
  mct_pipeline_t *pipeline = NULL;
  boolean rc = TRUE;

  if (!module || !bus_msg) {
    CLOGE(CAM_MCT_MODULE, "Null pointer detected: module [%p] or bus_msg [%p]",
       module, bus_msg);
    return FALSE;
  }

  stream = mct_pipeline_find_stream(module, bus_msg->sessionid);
  if (!stream) {
    CLOGE(CAM_MCT_MODULE, "Couldn't find stream for sessionid %d",
      bus_msg->sessionid);
    return FALSE;
  }

  if (MCT_STREAM_PARENT(stream))
    pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);

  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Not valid pipeline for this stream 0x%x",
      stream->streaminfo.identity);
    return FALSE;
  }

  mct_bus_t *bus = pipeline->bus;
  if (!bus || !bus->post_msg_to_bus) {
    CLOGE(CAM_MCT_MODULE, "Not valid bus in pipeline for session id %d",
      pipeline->session);
    return FALSE;
  }

  /* Call Fill metadata v3 function for Offline metadata collection */
  if (bus_msg->metadata_collection_type == MCT_BUS_OFFLINE_METADATA) {
    info.check_type    = CHECK_FEATURE_MASK;
    info.stream_type   = CAM_STREAM_TYPE_OFFLINE_PROC;
    info.feature_mask  = CAM_QCOM_FEATURE_METADATA_PROCESSING;
    offline_meta_stream = mct_pipeline_get_stream(pipeline, &info);
    if (!offline_meta_stream) {
      CLOGE(CAM_MCT_MODULE, "Couldn't find offline metadata stream");
      return FALSE;
    }
    rc = mct_stream_fill_metadata_buffer(
       pipeline->offline_meta.output_metadata_buffer,bus_msg->msg,
       bus_msg->type,bus_msg->size,
       &offline_meta_stream->metadata_stream.session_meta);
    return TRUE;
  } else if (bus_msg->type == MCT_BUS_MSG_ISP_RD_DONE) {
      info.check_type    = CHECK_FEATURE_MASK;
      info.stream_type   = CAM_STREAM_TYPE_OFFLINE_PROC;
      info.feature_mask  = CAM_QCOM_FEATURE_METADATA_PROCESSING;
      offline_meta_stream = mct_pipeline_get_stream(pipeline, &info);
      if (!offline_meta_stream) {
        CLOGH(CAM_MCT_MODULE, "Ignoring the ISP_RD_DONE message for Offline ISP\
          Online Metadata usecase.");
        return FALSE;
      } else {
        CLOGH(CAM_MCT_MODULE, "Posting the ISP_RD_DONE to queue for Offline \
          ISP Offline Metadata usecase.");
        rc = bus->post_msg_to_bus(bus, bus_msg);
        return rc;
      }
    }

  /*Push the priority bus msg into priority queue */
  switch (bus_msg->type) {
    case MCT_BUS_MSG_ERROR_MESSAGE:
    case MCT_BUS_MSG_SEND_HW_ERROR:
    case MCT_BUS_MSG_CONTROL_REQUEST_FRAME:
    case MCT_BUS_MSG_EZTUNE_JPEG:
    case MCT_BUS_MSG_EZTUNE_RAW:
    case MCT_BUS_MSG_DELAY_SUPER_PARAM:
    case MCT_BUS_MSG_FRAME_SKIP:
    case MCT_BUS_MSG_SENSOR_STARTING:
    case MCT_BUS_MSG_SENSOR_STOPPING:
      rc = bus->post_msg_to_bus(bus, bus_msg);
      return rc;
      break;

    case MCT_BUS_MSG_ISP_SOF:
      if((pipeline->hal_version == CAM_HAL_V3) && (is_supermct_exist(pipeline))) {
        /* Reset watchdog */
        if (bus->thread_run == 1) {
          CLOGL(CAM_MCT_MODULE, "Reset watchdog sessionid %d", bus_msg->sessionid);
          pthread_mutex_lock(&bus->bus_sof_msg_lock);
          pthread_cond_signal(&bus->bus_sof_msg_cond);
          pthread_mutex_unlock(&bus->bus_sof_msg_lock);
        }
        /* Notify super mct thread */
        rc = supermct_notify(bus_msg);
        return rc;
      } else if((pipeline->hal_version == CAM_HAL_V1) && (is_dual_zone(pipeline))) {
        /* Reset watchdog */
        if (bus->thread_run == 1) {
          CLOGL(CAM_MCT_MODULE, "Reset watchdog sessionid %d", bus_msg->sessionid);
          pthread_mutex_lock(&bus->bus_sof_msg_lock);
          pthread_cond_signal(&bus->bus_sof_msg_cond);
          pthread_mutex_unlock(&bus->bus_sof_msg_lock);
        }
        /* Notify super mct thread */
        rc = supermct_notify(bus_msg);
        return rc;

      } else {
        rc = bus->post_msg_to_bus(bus, bus_msg);
        return rc;
      }
      break;

    default:
     break;
  }

  /* Find Metadata Stream */
  info.check_type = CHECK_TYPE;
  info.stream_type = CAM_STREAM_TYPE_METADATA;
  meta_stream = mct_pipeline_get_stream(pipeline, &info);

  /* If the metadata stream is running only then fill the metadata buffer */
  if (meta_stream && meta_stream->state == MCT_ST_STATE_RUNNING) {
    pthread_mutex_lock(&meta_stream->metadata_stream.metadata_buf_lock);
    if (meta_stream->valid_buffer) {
      rc  = mct_stream_fill_metadata_buffer(
        meta_stream->metadata_stream.metadata_buffer,bus_msg->msg,
        bus_msg->type,bus_msg->size,
        &meta_stream->metadata_stream.session_meta);
      if (!rc) {
        CLOGE(CAM_MCT_MODULE, "Error while filling metadata buffer bus_msg=%d",
          bus_msg->type);
        pthread_mutex_unlock(&meta_stream->metadata_stream.metadata_buf_lock);
        return FALSE;
      }
    }
    /*If the metadata buffer is not available push into the queue */
    else {
     CLOGL(CAM_MCT_MODULE, "metadata buffer is not available push to queue");
      rc = bus->post_msg_to_bus(bus, bus_msg);
    }
    pthread_mutex_unlock(&meta_stream->metadata_stream.metadata_buf_lock);
  }
  else {
    /* Push bus msgs to queue when metadata stream is not ON */
    CLOGL(CAM_MCT_MODULE, "metadata stream is not ON,Push busmsgs to queue");
    rc = bus->post_msg_to_bus(bus, bus_msg);
  }
  return rc;
}

static boolean mct_module_type_check(void *data1, void *data2)
{
  mct_module_type_identity_t *type_data = (mct_module_type_identity_t *)data1;
  unsigned int *data = (unsigned int *)(data2);
  if (!type_data || !data)
      return FALSE;
  if (type_data->identity == *data)
      return TRUE;

  return FALSE;
}

void mct_module_add_type(mct_module_t *module, mct_module_type_t type,
  unsigned int identity)
{
  mct_module_type_identity_t *type_entry =
    malloc(sizeof(mct_module_type_identity_t));
  if (!type_entry) {
    CLOGE(CAM_MCT_MODULE, "malloc failed");
    return;
  }
  type_entry->type = type;
  type_entry->identity = identity;
  pthread_mutex_lock(&module->type_list_lock);
  module->type_list =
    mct_list_append(module->type_list, type_entry, NULL, NULL);
  pthread_mutex_unlock(&module->type_list_lock);
}

mct_module_type_t mct_module_find_type(mct_module_t *module,
  unsigned int identity)
{
  mct_list_t *holder;
  mct_module_type_identity_t *data;
  pthread_mutex_lock(&module->type_list_lock);
  holder = mct_list_find_custom(module->type_list, &identity,
    mct_module_type_check);
  pthread_mutex_unlock(&module->type_list_lock);
  if (holder && holder->data) {
    data = (mct_module_type_identity_t *)holder->data;
    return data->type;
  }
  return MCT_MODULE_FLAG_INVALID;
}

void mct_module_remove_type(mct_module_t *module, unsigned int identity)
{
  mct_list_t *holder;
  mct_module_type_identity_t *data;
  pthread_mutex_lock(&module->type_list_lock);
  holder = mct_list_find_custom(module->type_list, &identity,
    mct_module_type_check);
  pthread_mutex_unlock(&module->type_list_lock);
  if (holder && holder->data) {
    pthread_mutex_lock(&module->type_list_lock);
    data = (mct_module_type_identity_t *)holder->data;
    module->type_list = mct_list_remove(module->type_list, data);
    free(data);
    pthread_mutex_unlock(&module->type_list_lock);
  }
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
boolean mct_module_add_port(mct_module_t *module, mct_port_t *port)
{
  if (!module || !port)
    return FALSE;

#if 0
  /* look at port name */
  if (!mct_object_check_uniqueness(MCT_MODULE_CHILDREN(module),
          (const char *)(MCT_PORT_NAME(port)))) {
    CLOGE(CAM_MCT_MODULE, "Failed uniqueness test");
    return FALSE;
  }
#endif
  if (!mct_object_set_parent(MCT_OBJECT_CAST(port), MCT_OBJECT_CAST(module))) {
    CLOGE(CAM_MCT_MODULE, "Set parent failed");
    return FALSE;
  }

  /* add it to the list */
  switch (MCT_PORT_DIRECTION(port)) {
  case MCT_PORT_SRC:
    module->srcports = mct_list_append(module->srcports, port, NULL, NULL);
    module->numsrcports++;
    break;

  case MCT_PORT_SINK:
    module->sinkports = mct_list_append(module->sinkports, port, NULL, NULL);
    module->numsinkports++;
    break;

  default:
    return FALSE;
  }

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
/*TODO: Take care of unlinking the ports before removing them*/
boolean mct_module_remove_port(mct_module_t *module, mct_port_t *port)
{
  boolean ret;

  if (!module || !port || !MCT_PORT_PARENT(port))
    return FALSE;

  if (MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data) != module)
    return FALSE;

  switch (MCT_PORT_DIRECTION(port)) {
  case MCT_PORT_SRC: {
    module->srcports = mct_list_remove(module->srcports, port);
    module->numsrcports--;
    break;
  }

  case MCT_PORT_SINK: {
    module->sinkports = mct_list_remove(module->sinkports, port);
    module->numsinkports--;
    break;
  }

  default:
    break;
  }

  ret = mct_object_unparent(MCT_OBJECT_CAST(port), MCT_OBJECT_CAST(module));
  if (FALSE == ret) {
    CLOGE(CAM_MCT_MODULE, "Can not unparent port %s from module %s",
       MCT_OBJECT_NAME(port), MCT_OBJECT_NAME(module));
  }

  return ret;
}

/** mct_get_compatible_dest_port:
 *    @data1: destination port as type of mct_port_t
 *    @data2: information for matching in mct_module_custom_t
 **/
boolean mct_get_compatible_dest_port(void *data1, void *data2)
{
  mct_port_t *destport = (mct_port_t *)data1;
  mct_module_custom_t *sent_custom = (mct_module_custom_t *)data2;
  mct_port_t *srcport = sent_custom->port;
  boolean ret = FALSE;
  void *temp_caps;

  if (!mct_port_check_link(srcport, destport)) {
    CLOGD(CAM_MCT_MODULE, "Check link failed");
    return FALSE;
  }

  if (destport->caps.port_caps_type == MCT_PORT_CAPS_OPAQUE) {
    temp_caps = srcport->caps.u.data;
  } else {
    temp_caps = &srcport->caps;
  }

  /* DO COMPATIBILITY CHECK between destport and srcport */
  if (destport->check_caps_reserve) {
    if ((ret = destport->check_caps_reserve(destport, temp_caps,
      sent_custom->stream_info)) == TRUE) {

      if (destport->caps.port_caps_type == MCT_PORT_CAPS_STATS) {
        if (srcport->caps.u.stats.flag & destport->caps.u.stats.flag)
          ret = TRUE;
      } else if (destport->caps.port_caps_type == MCT_PORT_CAPS_FRAME) {
        if ((srcport->caps.u.frame.format_flag
          & destport->caps.u.frame.format_flag)
          && (srcport->caps.u.frame.size_flag
            <= destport->caps.u.frame.size_flag))
          ret = TRUE;
      } else {
        ret = TRUE;
      }
    }/*check_caps*/
  }

  return ret;
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
boolean mct_get_compatible_src_port(void *data1, void *data2)
{
  mct_port_t *srcport = (mct_port_t *)data1;
  mct_module_custom_t *custom = (mct_module_custom_t *)data2;
  mct_module_custom_t local_custom;
  mct_list_t *destports;
  mct_list_t *request_dest_holder;
  mct_port_t *request_dest = NULL;

  if (srcport->check_caps_reserve(srcport, NULL, custom->stream_info) == FALSE)
    return FALSE;

  custom->valid_src_port = TRUE;
  destports = MCT_MODULE_SINKPORTS(custom->dest_mod);
  /* Now, loop through destports and try to find a port that
   * 1) Can meet stream needs
   * 2) Is compatible with this srcport
   */
  local_custom = *custom;
  local_custom.port = srcport;

  request_dest_holder = mct_list_find_custom(destports, &local_custom,
    mct_get_compatible_dest_port);

  if (!request_dest_holder) {
    if ((MCT_PORT_DIRECTION(srcport) == MCT_PORT_SRC) &&
      (MCT_PORT_PEER(srcport) == NULL)) {
      /* it has to be SRC port to link to SINK port,
       * reverse link might be supported in future but not now */
      if ((custom->dest_mod)->request_new_port)
      {
        request_dest =
          (custom->dest_mod)->request_new_port(custom->stream_info,
           MCT_PORT_SINK, custom->dest_mod, &srcport->caps);
      }
    }
  } else {
    request_dest = MCT_PORT_CAST(request_dest_holder->data);
  }

  if (!request_dest) {
    srcport->check_caps_unreserve(srcport, custom->identity);
    return FALSE;
  }

  custom->port = request_dest;

  return TRUE;
}

/** mct_module_link_ports_internal:
 *    @sessionid:   session index;
 *    @streamid:    stream index;
 *    @stream_info: buffer pointer of type cam_stream_info_t*
 *      Defined in common HAL header (cam_intf.h)
 *    @src: source module to be linked
 *    @dest: destination module to be linked
 *
 * We first need to make sure all modules are under same umbrella(Pipeline),
 * this should be taken care of in mct_pipeline_add_modules
 *
 * Note mct_module_link_ports_internal can be called in some other places
 *
 * Returns: TRUE if the ports could be linked, FALSE otherwise.
 */
boolean mct_module_link(void *stream_info, mct_module_t *src,
  mct_module_t *dest)
{
  mct_list_t *srcports, *destports;
  mct_port_t *srcport = NULL, *destport = NULL;
  mct_list_t *srcport_holder, *destport_holder;
  mct_module_custom_t custom_data;
  unsigned int identity;
  if (!src || !dest)
    return FALSE;

  identity = ((mct_stream_info_t *)stream_info)->identity;
  srcports = MCT_MODULE_SRCPORTS(src);
  destports = MCT_MODULE_SINKPORTS(dest);

  memset(&custom_data, 0, sizeof(custom_data));
  custom_data.identity = identity;
  custom_data.dest_mod = dest;
  custom_data.stream_info = stream_info;

  /* traverse through the allowed ports in the source, trying to find a
   * compatible destination port
   */
  srcport_holder = mct_list_find_custom(srcports, &custom_data,
    mct_get_compatible_src_port);

  if (!srcport_holder) {
    if (custom_data.valid_src_port == TRUE) {
      /* not able to find a valid destination port */
      goto fail;
    } else if (custom_data.valid_src_port == FALSE) {
      if (src->request_new_port) {
        srcport = src->request_new_port(stream_info, MCT_PORT_SRC, src, NULL);
        if (!srcport)
          goto fail;
        memset(&custom_data, 0, sizeof(custom_data));
        custom_data.identity = identity;
        custom_data.stream_info = stream_info;
        custom_data.port = srcport;

        destport_holder = mct_list_find_custom(destports, &custom_data,
          mct_get_compatible_dest_port);
        if (!destport_holder) {
          if (dest->request_new_port)
            destport = dest->request_new_port(stream_info,
                                              MCT_PORT_SINK, dest, NULL);
        } else {
          destport = MCT_PORT_CAST(destport_holder->data);
        }
        if (!destport)
          /*un-request src port*/
          goto fail;
      } else { /*src module doesnt support request port*/
        goto fail;
      }
    }
  } else { /*already found src port*/
    srcport = (mct_port_t *)srcport_holder->data;
    destport = custom_data.port;
  }

  if (srcport && destport) {
    if (mct_port_establish_link(identity, srcport, destport))
      return TRUE;

    /*if linking fails, unreserve the ports*/
    srcport->check_caps_unreserve(srcport, identity);
    destport->check_caps_unreserve(destport, identity);
  }

fail:
  return FALSE;
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
void mct_module_unlink(unsigned int identity, mct_module_t *src,
  mct_module_t *dest)
{
  mct_port_t *srcport;
  mct_port_t *sinkport;
  mct_list_t *port_container;
  mct_module_custom_t custom_data;

  memset(&custom_data, 0, sizeof(custom_data));
  custom_data.identity = identity;
  custom_data.dest_mod = dest;

  /* first src port in src module and sink port in dest module
   * that supports this session/stream combination */
  port_container = mct_list_find_custom(MCT_MODULE_SRCPORTS(src), &custom_data,
    mct_module_find_connected_src_port);

  if (!port_container) {
    CLOGE(CAM_MCT_MODULE, "Modules aren't linked");
    return;
  }

  srcport = MCT_PORT_CAST(port_container->data);
  sinkport = custom_data.port;

  mct_port_destroy_link(identity, srcport, sinkport);

  srcport->check_caps_unreserve(srcport, identity);
  sinkport->check_caps_unreserve(sinkport, identity);

  mct_port_remove_child(identity, srcport);
  mct_port_remove_child(identity, sinkport);

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
mct_module_t* mct_module_create(const char *name)
{
  mct_module_t *new_module;
  new_module = malloc(sizeof(mct_module_t));
  if (!new_module) {
    /*print error code here strerror(errno)*/
    return FALSE;
  }

  memset(new_module, 0, sizeof(mct_module_t));
  pthread_mutex_init(MCT_OBJECT_GET_LOCK(new_module), NULL);
  pthread_mutex_init(&new_module->type_list_lock, NULL);
  mct_object_set_name(MCT_OBJECT_CAST(new_module), name);
  mct_module_set_set_mod_func(new_module, mct_module_set_mod_default);
  mct_module_set_query_mod_func(new_module, mct_module_query_mod_default);
  //mct_module_set_request_port_func(new_module,
  //  mct_module_request_port_default);
  mct_module_set_start_session_func(new_module,
    mct_module_start_session_default);
  mct_module_set_stop_session_func(new_module,
                                   mct_module_stop_session_default);
  return new_module;
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
void mct_module_destroy(mct_module_t *module)
{
  pthread_mutex_destroy(MCT_OBJECT_GET_LOCK(module));
  pthread_mutex_destroy(&module->type_list_lock);
  free(MCT_MODULE_NAME(module));
  free(module);
  module = NULL;
  return;
}
