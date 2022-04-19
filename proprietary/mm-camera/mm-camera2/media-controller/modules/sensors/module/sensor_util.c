/* sensor_util.c
 *
 * Copyright (c) 2012-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "sensor_util.h"
/** sensor_util_pass_fast_aec_mode_event:
 *    @port: mct port to pass fast eac mode event downstream
 *    @identity: identity for the event
 *    @num_frames: num of frames in fast aec
 *
 *  Return: TRUE if downstream event returns TRUE
 *
 *  This function creates fast aec mode event and passes
 *  downstream
 **/

#define V4L2_FOURCC(data_fmt, bit_width) V4L2_PIX_FMT_S##data_fmt##bit_width

boolean sensor_util_pass_fast_aec_mode_event(mct_port_t *port,
  uint32_t identity, uint8_t enable, uint16_t num_frames)
{
  boolean ret = TRUE;
  mct_event_t new_event;
  mct_fast_aec_mode_t aec_mode;

  aec_mode.enable = enable;
  aec_mode.num_frames_in_fast_aec = num_frames;
  new_event.type = MCT_EVENT_MODULE_EVENT;
  new_event.identity = identity;
  new_event.direction = MCT_EVENT_DOWNSTREAM;
  new_event.u.module_event.type = MCT_EVENT_MODULE_SET_FAST_AEC_CONVERGE_MODE;
  new_event.u.module_event.module_event_data = &aec_mode;
  ret = port->event_func(port, &new_event);
  if (ret == FALSE) {
    SERR("failed");
  }

  return ret;
}

/** sensor_util_pass_stream_off_event:
 *    @port: mct port to pass stream off event downstream
 *    @identity: identity for the event
 *    @stream_info: stream info
 *
 *  Return: TRUE if downstream event returns TRUE
 *
 *  This function creates stream off event and passes
 *  downstream
 **/

boolean sensor_util_pass_stream_off_event(mct_port_t *port,
  uint32_t identity, mct_stream_info_t *stream_info)
{
  boolean     ret = TRUE;
  mct_event_t new_event;

  if (!port || !stream_info) {
    SERR("failed: port %p stream_info %p", port, stream_info);
    return FALSE;
  }

  new_event.type = MCT_EVENT_CONTROL_CMD;
  new_event.identity = identity;
  new_event.direction = MCT_EVENT_DOWNSTREAM;
  new_event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMOFF;
  new_event.u.ctrl_event.control_event_data = stream_info;
  ret = port->event_func(port, &new_event);
  if (ret == FALSE) {
    SERR("failed");
  }

  return ret;
}

/** sensor_util_pass_hfr_enable_event:
 *    @port: mct port to pass hfr enable event downstream
 *    @identity: identity for the event
 *
 *  Return: TRUE if downstream event returns TRUE
 *
 *  This function creates fast aec mode event and passes
 *  downstream
 **/

boolean sensor_util_pass_hfr_event(mct_port_t *port,
  uint32_t identity, cam_hfr_mode_t hfr_mode)
{
  boolean ret = TRUE;
  mct_event_t new_event;
  mct_event_control_parm_t event_control;

  event_control.type = CAM_INTF_PARM_HFR;
  event_control.parm_data = (void *)&hfr_mode;
  new_event.type = MCT_EVENT_CONTROL_CMD;
  new_event.identity = identity;
  new_event.direction = MCT_EVENT_DOWNSTREAM;
  new_event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  new_event.u.ctrl_event.control_event_data = (void *)&event_control;
  ret = port->event_func(port, &new_event);
  if (ret == FALSE) {
    SERR("failed");
  }

  return ret;
}

/** sensor_util_pass_bundle_event:
 *    @port: mct port to pass bundle event downstream
 *    @identity: identity for the event
 *    @bundle_config: bundle config sent by HAL
 *
 *  Return: TRUE if downstream event returns TRUE
 *
 *  This function creates bundle event and passes downstream
 **/

boolean sensor_util_pass_bundle_event(mct_port_t *port,
  uint32_t identity, cam_bundle_config_t *bundle_config)
{
  boolean ret = TRUE;
  mct_event_t new_event;
  cam_stream_parm_buffer_t stream_parm;

  if (!port || !bundle_config) {
    SERR("port %p bundle_config %p", port, bundle_config);
    return FALSE;
  }
  stream_parm.type = CAM_STREAM_PARAM_TYPE_SET_BUNDLE_INFO;
  memcpy(&stream_parm.bundleInfo, bundle_config, sizeof(cam_bundle_config_t));
  new_event.type = MCT_EVENT_CONTROL_CMD;
  new_event.identity = identity;
  new_event.direction = MCT_EVENT_DOWNSTREAM;
  new_event.u.ctrl_event.type = MCT_EVENT_CONTROL_PARM_STREAM_BUF;
  new_event.u.ctrl_event.control_event_data = (void *)&stream_parm;
  ret = port->event_func(port, &new_event);
  if (ret == FALSE) {
    SERR("failed");
  }

  return ret;
}

/** sensor_util_find_stream_info_from_type:
 *    @data1: sensor port stream info list
 *    @data2: stream type
 *
 *  Return: TRUE if incoming stream type matches with list
 *  stream type
 *
 **/

boolean sensor_util_find_stream_info_from_type(void *data1, void *data2)
{
  module_sensor_port_stream_info_t *port_stream_info =
    (module_sensor_port_stream_info_t *)data1;
  cam_stream_type_t *type = (cam_stream_type_t *)data2;

  if (!port_stream_info || !type) {
    SERR("failed: port_stream_info %p type %p", port_stream_info, type);
    return FALSE;
  }

  if (port_stream_info->stream_type == *type) {
    return TRUE;
  }

  return FALSE;
}

/** sensor_util_get_ide_from_stream_type:
 *    @port_stream_info_list: sensor port stream info list
 *    @type: stream type
 *
 *  Return: identity for stream type
 *
 *  This function matches session id with sensor video node
 *  id from sensor info passed to it and returns decision
 **/

uint32_t sensor_util_get_ide_from_stream_type(
  mct_list_t *port_stream_info_list, cam_stream_type_t type)
{
  mct_list_t                       *stream_info_list = NULL;
  module_sensor_port_stream_info_t *stream_info = NULL;

  if (!port_stream_info_list) {
    SERR("failed: port_stream_info_list %p", port_stream_info_list);
    return 0;
  }

  stream_info_list = mct_list_find_custom(port_stream_info_list,
    &type, sensor_util_find_stream_info_from_type);

  if (!stream_info_list) {
    SERR("failed: prev_stream_info_list %p", stream_info_list);
    return 0;
  }

  stream_info = stream_info_list->data;
  if (!stream_info) {
    SERR("failed: prev_stream_info %p", stream_info);
    return 0;
  }

  return stream_info->identity;
}

/** sensor_util_find_stream_info_from_stream_id:
 *    @data1: sensor port stream info list
 *    @data2: stream id
 *
 *  Return: TRUE if incoming stream type matches with list
 *  stream type
 *
 **/

boolean sensor_util_find_stream_info_from_stream_id(void *data1, void *data2)
{
  module_sensor_port_stream_info_t *port_stream_info =
    (module_sensor_port_stream_info_t *)data1;
  uint32_t *stream_id = (cam_stream_type_t *)data2;

  if (!port_stream_info || !stream_id) {
    SERR("failed: port_stream_info %p stream_id %p", port_stream_info,
      stream_id);
    return FALSE;
  }

  if ((port_stream_info->identity & 0xFFFF) == *stream_id) {
    return TRUE;
  }

  return FALSE;
}

/** sensor_util_get_stream_info:
 *    @port_stream_info_list: sensor port stream info list
 *    @stream_id: stream id
 *
 *  Return: identity for stream type
 *
 *  This function matches session id with sensor video node
 *  id from sensor info passed to it and returns decision
 **/

mct_stream_info_t *sensor_util_get_stream_info(
  mct_list_t *port_stream_info_list, uint32_t stream_id)
{
  mct_list_t                       *stream_info_list = NULL;
  module_sensor_port_stream_info_t *stream_info = NULL;

  if (!port_stream_info_list) {
    SERR("failed: port_stream_info_list %p", port_stream_info_list);
    return NULL;
  }

  stream_info_list = mct_list_find_custom(port_stream_info_list,
    &stream_id, sensor_util_find_stream_info_from_stream_id);

  if (!stream_info_list) {
    SERR("failed: prev_stream_info_list %p", stream_info_list);
    return NULL;
  }

  stream_info = stream_info_list->data;
  if (!stream_info) {
    SERR("failed: prev_stream_info %p", stream_info);
    return NULL;
  }

  return stream_info->stream_info;
}

/**  sensor_util_unpack_identity:
 *    unpacks the 32-bit identity in session_id and stream_id
 *
 *  Arguments:
 *    @identity: input param
 *    @session_id: output param
 *    @stream_id: output param
 *
 *  Return: TRUE on success
 *          FALSE on failure **/

boolean sensor_util_unpack_identity(uint32_t identity,
  uint32_t *session_id, uint32_t *stream_id)
{
  if (!session_id || !stream_id) {
    SERR("failed session_id %p stream_id %p",
      session_id, stream_id);
    return FALSE;
  }
  *stream_id = (identity & 0x0000FFFF);
  *session_id = ((identity & 0xFFFF0000) >> 16);
  SLOW("session_id %d stream id %d", *session_id, *stream_id);
  return TRUE;
}

/** sensor_util_find_bundle: sensor find bundle
 *
 *  @data1: sensor bundle info
 *  @data2: session id
 *
 *  Return: 1 if session id matches with sensor video node id
 *
 *  This function matches session id with sensor video node
 *  id from sensor info passed to it and returns decision
 **/

boolean sensor_util_find_bundle(void *data1, void *data2)
{
  module_sensor_bundle_info_t *s_bundle = (module_sensor_bundle_info_t *)data1;
  uint32_t *session_id = (uint32_t *)data2;

  if (!s_bundle || !session_id) {
    SERR("failed data1 %p data2 %p", s_bundle,
      session_id);
    return FALSE;
  }
  if (s_bundle->sensor_info->session_id == *session_id)
    return TRUE;

  return FALSE;
}

/** sensor_util_find_identity:
 *    @data1: sensor bundle info
 *    @data2: session id
 *
 *  Return: 1 if child identity matches with current identity
 *
 *  This function matches session id with sensor video node
 *  id from sensor info passed to it and returns decision
 **/

boolean sensor_util_find_identity(void *data1, void *data2)
{
  uint32_t *child_identity = (uint32_t *)data1;
  uint32_t *cur_identity = (uint32_t *)data2;

  if (!child_identity || !cur_identity) {
    SERR("failed data1 %p data2 %p", data1,
      data2);
    return FALSE;
  }
  if (*child_identity == *cur_identity)
    return TRUE;

  return FALSE;
}

static boolean sensor_util_src_port_identity_find_func(void* data,
  void* user_data)
{
  uint32_t* p_identity = (uint32_t*)user_data;
  mct_port_t* port = (mct_port_t *)data;
  if (!p_identity || !port) {
    SERR("failed");
    return FALSE;
  }
  SLOW("port=%p; required identity=0x%x", port, *p_identity);
  mct_list_t* s_list = mct_list_find_custom(MCT_PORT_CHILDREN(port), p_identity,
     sensor_util_find_identity);
  if (s_list == NULL) {
    SLOW("Cannot find port with identity=0x%x", *p_identity);
    return FALSE;
  }
  return TRUE;
}


/*     --- module_sensor_find_src_port_with_identity ---
 *  Description:
 *    Find the src port in module which supports identity
 *  Arguments:
 *    @module:    sensor module pointer
 *    #identity:  32-bit identity
 *  Returns:
 *    pointer to the corresponding port on success
 *    NULL, on failure or if port is not found
 */
mct_port_t *sensor_util_find_src_port_with_identity(mct_module_t *module,
  uint32_t identity)
{
  mct_port_t* port = NULL;
  if (!module) {
    SERR("failed");
    return NULL;
  }
  mct_list_t* s_list = mct_list_find_custom(MCT_MODULE_SRCPORTS(module),
    &identity, sensor_util_src_port_identity_find_func);
  port = (s_list != NULL) ? s_list->data : NULL;
  return port;
}

static boolean sensor_util_stream_list_print_traverse_func(void *data,
  void __attribute__((unused)) *user_data)
{
  if (!data) {
    SERR("failed");
    return FALSE;
  }
  module_sensor_port_stream_info_t __attribute__((unused)) *stream_info =
    (module_sensor_port_stream_info_t*) data;

  SLOW("stream:bundle_id=%d", stream_info->bundle_id);
  SLOW("stream:identity=0x%x", stream_info->identity);
  SLOW("stream:dim=%dx%d", stream_info->width, stream_info->height);
  SLOW("---------------------------------------------------------------");
  return TRUE;
}

static boolean sensor_util_bundle_list_print_traverse_func(void *data,
  void __attribute__((unused)) *user_data)
{
  if (!data) {
    SERR("failed");
    return FALSE;
  }
  module_sensor_port_bundle_info_t* bundle_info=
    (module_sensor_port_bundle_info_t*) data;
  SLOW("bundle:id=%d, ", bundle_info->bundle_config.bundle_id);
  SLOW("bundle:num_streams=%d",
    bundle_info->bundle_config.num_of_streams);
  int32_t i;
  for (i=0; i< bundle_info->bundle_config.num_of_streams; i++) {
    SLOW("bundle:steram[%d]=%d", i,
      bundle_info->bundle_config.stream_ids[i]);
  }
  SLOW("---------------------------------------------------------------");
  return TRUE;
}

void sensor_util_dump_bundle_and_stream_lists(mct_port_t* port,
  const char __attribute__((unused)) *func,
  int32_t __attribute__((unused)) line)
{
  if (!port) {
    SERR("failed");
    return;
  }
  module_sensor_port_data_t *port_data =
     (module_sensor_port_data_t*) port->port_private;

  SLOW("DUMP at function %s() at line %d", func, line);
  SLOW("port=%p, bundle_list=%p, stream_list=%p",
    port, port_data->bundle_list, port_data->stream_list);

  SLOW("---------------------------------------------------------------");
  SLOW("                       Bundle List");
  SLOW("---------------------------------------------------------------");
  mct_list_traverse(port_data->bundle_list,
    sensor_util_bundle_list_print_traverse_func, NULL);
  SLOW("                       Stream List");
  SLOW("---------------------------------------------------------------");
  mct_list_traverse(port_data->stream_list,
    sensor_util_stream_list_print_traverse_func, NULL);
}

static boolean sensor_util_fill_bundle_params(void* data, void* user_data)
{
  module_sensor_port_stream_info_t* stream_info =
    (module_sensor_port_stream_info_t*) data;
  module_sensor_port_bundle_info_t* bundle_info =
     (module_sensor_port_bundle_info_t*) user_data;

  if (!stream_info || !bundle_info) {
    SERR("failed");
    return FALSE;
  }
  uint32_t session_id, stream_id;
  sensor_util_unpack_identity(stream_info->identity, &session_id, &stream_id);
  int32_t i;
  for (i = 0; i < bundle_info->bundle_config.num_of_streams; i++) {
    if (stream_id == (uint32_t)bundle_info->bundle_config.stream_ids[i]) {
      stream_info->bundle_id = (int32_t)bundle_info->bundle_config.bundle_id;
      SLOW("stream=%d, bundle=%d", stream_id,
        bundle_info->bundle_config.bundle_id);
      break;
    }
  }
  return TRUE;
}

static boolean sensor_util_find_exact_bundle(void* list_data, void* user_data)
{
  boolean ret_val = FALSE;
  module_sensor_port_bundle_info_t *bundle_info = list_data;
  cam_bundle_config_t* bundle = user_data;

  if (bundle_info && bundle) {
    if (!memcmp(&(bundle_info->bundle_config), bundle,
      sizeof(cam_bundle_config_t))) {
        ret_val = TRUE;
    }
  } else
    SERR("Null pointer detected in %s\n", __func__);

  return ret_val;
}

int32_t sensor_util_assign_bundle_id(mct_module_t* module,
  uint32_t identity, cam_bundle_config_t* bundle)
{
  mct_port_t *s_port = NULL;
  mct_list_t* list;
  uint32_t    i = 0;

  SLOW("bundle_id=%d, num_streams=%d", bundle->bundle_id,
    bundle->num_of_streams);
  /* Print all bundle stream id's */
  for (i = 0; i < bundle->num_of_streams; i++) {
    SLOW("bundle stream id %d", bundle->stream_ids[i]);
  }

  /* find a src port which has the given identity */
  s_port = sensor_util_find_src_port_with_identity(module, identity);
  if (!s_port) {
    SERR("failed: cannot find port with identity=0x%x", identity);
    return -EFAULT;
  }

  /* add bundle info in port private data,
     find the correspoding streams in the port_data,
     assign bundle_id to these streams */
  module_sensor_port_data_t* port_data;
  port_data = s_port->port_private;
  if (!port_data) {
    SERR("failed");
    return -EFAULT;
  }

  list = mct_list_find_custom(port_data->bundle_list, bundle,
    sensor_util_find_exact_bundle);

  if (!list) {
    module_sensor_port_bundle_info_t *bundle_info = NULL;
    bundle_info = (module_sensor_port_bundle_info_t *)malloc(
      sizeof(module_sensor_port_bundle_info_t));
    if (!bundle_info) {
      SERR("bundle_info allocation failed");
      return -EFAULT;
    }
    memset(bundle_info, 0, sizeof(module_sensor_port_bundle_info_t));
    memcpy(&(bundle_info->bundle_config), bundle, sizeof(cam_bundle_config_t));
    bundle_info->stream_on_count = 0;
    /* assign the bundle id to streams and get max dimensions for a bundle */
    mct_list_traverse(port_data->stream_list, sensor_util_fill_bundle_params,
      bundle_info);
    SLOW("assigned bundle_id to streams");
    port_data->bundle_list = mct_list_append(port_data->bundle_list, bundle_info,
      NULL, NULL);
    SLOW("added bundle to list");
  }

  sensor_util_dump_bundle_and_stream_lists(s_port, __func__, __LINE__);
  return 0;
}

static boolean sensor_util_traverse_port(void *data, void *user_data)
{
  boolean ret = TRUE;
  mct_port_t *s_port = (mct_port_t *)data;
  mct_event_t *event = (mct_event_t *)user_data;
  mct_list_t *s_list = NULL;

  if (!s_port || !event) {
    SERR("failed port %p event %p", s_port, event);
    return FALSE;
  }

  s_list = mct_list_find_custom(MCT_PORT_CHILDREN(s_port), &event->identity,
    sensor_util_find_identity);
  if (!s_list) {
    return TRUE;
  }

  SLOW("s_port=%p event_func=%p", s_port, s_port->event_func);
  ret = s_port->event_func(s_port, event);
  return ret;
}

boolean sensor_util_post_event_on_src_port(mct_module_t *module,
  mct_event_t *event)
{
  boolean ret = TRUE;

  if (!module || !event) {
    SERR("failed module %p event %p", module, event);
    return FALSE;
  }

  ret = mct_list_traverse(MCT_MODULE_SRCPORTS(module),
    sensor_util_traverse_port, event);
  return ret;
}

static boolean sensor_util_find_stream(void *data, void *user_data)
{
  module_sensor_port_stream_info_t *stream_info =
    (module_sensor_port_stream_info_t *)data;
  uint32_t *identity = (uint32_t *)user_data;
  if (!data || !user_data) {
    SERR("failed");
    return FALSE;
  }
  SLOW("id1=0x%x, id2=0x%x", stream_info->identity, *identity);
  if (stream_info->identity == *identity) {
    return TRUE;
  }
  return FALSE;
}

int32_t sensor_util_find_bundle_id_for_stream(mct_port_t* port,
  uint32_t identity)
{
  mct_list_t                       *s_list = NULL;
  module_sensor_port_stream_info_t *stream_info = NULL;
  module_sensor_port_data_t        *port_data = NULL;
  if (!port) {
    SERR("failed");
    return -1;
  }
  port_data = (module_sensor_port_data_t *)port->port_private;
  if (!port_data) {
    SERR("failed");
    return -1;
  }
  s_list = mct_list_find_custom(port_data->stream_list, &identity,
    sensor_util_find_stream);
  if (!s_list) {
    SLOW("bundle id not found");
    return -1;
  }
  stream_info = (module_sensor_port_stream_info_t *)s_list->data;
  if (!stream_info) {
    SERR("stream info NULL");
    return -1;
  }
  return stream_info->bundle_id;
}

static boolean sensor_util_find_stream_by_type_and_session(void *data,
  void *user_data)
{
  RETURN_ON_NULL(data);
  RETURN_ON_NULL(user_data);

  module_sensor_port_stream_info_t *stream_info =
    (module_sensor_port_stream_info_t *)data;
  module_sensor_util_stream_info_t* search_data =
    (module_sensor_util_stream_info_t *)user_data;

  SLOW("type1=0x%x, type2=0x%x", stream_info->stream_type,
    search_data->stream_type);
  SLOW("identity1=0x%x, session2=0x%x", stream_info->identity,
    search_data->session_id);
  if ((search_data->stream_type == stream_info->stream_type) &&
      (search_data->session_id == SENSOR_SESSIONID(stream_info->identity))) {
    search_data->stream_id = SENSOR_STREAMID(stream_info->identity);
    return TRUE;
  }
  return FALSE;
}

static boolean sensor_util_find_port_by_stream_type_and_session(
  void *data, void *user_data)
{
  mct_port_t                       *port = data;
  module_sensor_util_stream_info_t *search_data = user_data;
  mct_list_t                       *s_list = NULL;
  module_sensor_port_stream_info_t *stream_info = NULL;
  module_sensor_port_data_t        *port_data = NULL;

  if (!port) {
    SERR("failed");
    return FALSE;
  }

  port_data = (module_sensor_port_data_t *)port->port_private;
  if (!port_data) {
    SERR("failed");
    return FALSE;
  }

  s_list = mct_list_find_custom(port_data->stream_list, search_data,
    sensor_util_find_stream_by_type_and_session);
  if (!s_list) {
    SLOW("bundle id not found");
    return FALSE;
  }
  stream_info = (module_sensor_port_stream_info_t *)s_list->data;
  if (!stream_info) {
    SERR("stream info NULL");
    return FALSE;
  }
  return TRUE;
}

boolean sensor_util_find_stream_identity_by_stream_type_and_session(
  mct_module_t *module, cam_stream_type_t stream_type,
  uint16_t session_id, uint32_t *stream_identity)
{
  mct_list_t *s_list = NULL;
  module_sensor_util_stream_info_t  search_data;

  if (!module || !session_id || !stream_identity) {
    SERR("failed");
    return FALSE;
  }

  search_data.session_id = session_id;
  search_data.stream_type = stream_type;
  s_list = mct_list_find_custom(MCT_MODULE_SRCPORTS(module), &search_data,
    sensor_util_find_port_by_stream_type_and_session);

  if (!s_list || !s_list->data) {
    SLOW("port not found");
    return FALSE;
  }

  *stream_identity = SENSOR_IDENTITY(search_data.session_id, search_data.stream_id);
  return TRUE;
}

int32_t sensor_util_find_stream_type_for_stream(mct_port_t* port,
  uint32_t identity)
{
  mct_list_t                       *s_list = NULL;
  module_sensor_port_stream_info_t *stream_info = NULL;
  module_sensor_port_data_t        *port_data = NULL;
  if (!port) {
    SERR("failed");
    return -1;
  }
  port_data = (module_sensor_port_data_t *)port->port_private;
  if (!port_data) {
    SERR("failed");
    return -1;
  }
  s_list = mct_list_find_custom(port_data->stream_list, &identity,
    sensor_util_find_stream);
  if (!s_list) {
    SLOW("bundle id not found");
    return -1;
  }
  stream_info = (module_sensor_port_stream_info_t *)s_list->data;
  if (!stream_info) {
    SERR("stream info NULL");
    return -1;
  }
  return (int32_t)stream_info->stream_type;
}

/** sensor_util_find_identity:
 *    @data1: port_stream_info
 *    @data2: NULL
 *
 *  Return: TRUE if this is streaming
 *
 *  This function checks for is_stream_on flag
 **/
static boolean sensor_util_is_stream_on(void *data1,
  __attribute__((unused)) void *data2)
{
  module_sensor_port_stream_info_t *port_stream_info =
   (module_sensor_port_stream_info_t *)data1;

  if (!port_stream_info) {
    SERR("failed: port_stream_info %p", port_stream_info);
    return FALSE;
  }

  return port_stream_info->is_stream_on;
}

boolean sensor_util_find_is_stream_on(mct_port_t* port)
{
  mct_list_t                       *s_list = NULL;
  module_sensor_port_stream_info_t *stream_info = NULL;
  module_sensor_port_data_t        *port_private = NULL;

  if (!port) {
    SERR("failed: port %p", port);
    return FALSE;
  }

  port_private = (module_sensor_port_data_t *)port->port_private;
  if (!port_private) {
    SERR("failed");
    return FALSE;
  }
  s_list = mct_list_find_custom(port_private->stream_list, NULL,
    sensor_util_is_stream_on);
  if (s_list) {
    SLOW("Streaming on is found");
    return TRUE;
  }
  return FALSE;
}

boolean sensor_util_set_stream_on(mct_module_t *module,
  uint32_t identity, boolean st_on)
{
  module_sensor_port_data_t        *port_private = NULL;
  mct_list_t                       *port_stream_info_list = NULL;
  module_sensor_port_stream_info_t *port_stream_info = NULL;
  mct_port_t* port = NULL;


  /* Find port that belongs to incoming identity */
  port = sensor_util_find_src_port_with_identity(module, identity);
  if (!port) {
    SERR("failed: port %p", port);
    return FALSE;
  }

  /* Retrive port private */
  port_private = (module_sensor_port_data_t *)port->port_private;
  if (!port_private) {
    SERR("failed: port_private %p", port_private);
    return FALSE;
  }
  SLOW("port private %p", port_private);

  /* Retrieve stream info list for this identity */
  port_stream_info_list = mct_list_find_custom(port_private->stream_list,
    &identity, sensor_util_find_stream);
  if (!port_stream_info_list) {
    SERR("failed: port_stream_info_list %p", port_stream_info_list);
    return FALSE;
  }

  /* Retrieve stream specific information */
  port_stream_info =
    (module_sensor_port_stream_info_t *)port_stream_info_list->data;
  if (!port_stream_info) {
    SERR("failed: port_stream_info %p", port_stream_info);
    return FALSE;
  }

  port_stream_info->is_stream_on = st_on;

  return TRUE;
}

static boolean sensor_util_bundle_id_find_func(void* data, void* user_data)
{
  uint32_t *bundle_id = (uint32_t *)user_data;
  module_sensor_port_bundle_info_t* bundle_info =
    (module_sensor_port_bundle_info_t*)data;
  if (!data || !user_data) {
    SERR("failed");
    return FALSE;
  }
  if (bundle_info->bundle_config.bundle_id == *bundle_id)
    return TRUE;
  else
    return FALSE;
}

module_sensor_port_bundle_info_t* sensor_util_find_bundle_by_id(
  mct_port_t* port, int32_t bundle_id)
{
  module_sensor_port_bundle_info_t *bundle_info = NULL;
  module_sensor_port_data_t        *port_data =
    (module_sensor_port_data_t *)port->port_private;
  mct_list_t *s_list  = mct_list_find_custom(port_data->bundle_list,
    &bundle_id, sensor_util_bundle_id_find_func);
  if (s_list != NULL) {
    bundle_info = s_list->data;
  }
  return bundle_info;
}

static boolean sensor_util_find_stream_with_streamtype(void *data,
  void *user_data)
{
  module_sensor_port_stream_info_t *stream_info =
    (module_sensor_port_stream_info_t *)data;
  cam_stream_type_t *stream_type = (cam_stream_type_t *)user_data;
  if (!data || !user_data) {
    SERR("failed");
    return FALSE;
  }
  SLOW("id1=%d, id2=%d", stream_info->stream_type, *stream_type);
  if (stream_info->stream_type == *stream_type) {
    return TRUE;
  }
  return FALSE;
}

/** sensor_util_get_session_stream_identity
 *
 *  @module: mct module handle
 *  @identity: current stream's identity
 *
 *  Return: session-based stream's identity
 *
 *  based on the input identity,
 *  return session based stream's identity
 **/
static uint32_t sensor_util_get_session_stream_identity(mct_module_t *module,
  uint32_t identity)
{
  mct_port_t                       *port;
  module_sensor_port_data_t        *port_data;
  mct_list_t                       *s_list;
  module_sensor_port_stream_info_t *stream_info;
  cam_stream_type_t                 stream_type = CAM_STREAM_TYPE_PARM;

  port = sensor_util_find_src_port_with_identity(module, identity);
  if (!port) {
    SERR("failed");
    return identity;
  }
  port_data = (module_sensor_port_data_t *)port->port_private;
  if (!port_data) {
    SERR("failed");
    return identity;
  }

  s_list = mct_list_find_custom(port_data->stream_list, &stream_type,
    sensor_util_find_stream_with_streamtype);
  if (!s_list) {
    SERR("stream not found");
    return identity;
  }

  stream_info = s_list->data;

  SLOW("session based identity 0x%x", stream_info->identity);

  return stream_info->identity;
}

void sensor_util_remove_list_entries_by_identity(mct_port_t *port,
  uint32_t identity)
{
  module_sensor_port_data_t        *port_data = NULL;
  module_sensor_port_stream_info_t *stream_info = NULL;
  module_sensor_port_bundle_info_t *bundle_info = NULL;
  mct_list_t                       *tmp = NULL;
  int32_t                           bundle_id = -1;
  SLOW("port=%p, identity=0x%x", port, identity);
  if (!port) {
    SERR("failed");
    return;
  }
  port_data = port->port_private;
  if (!port_data) {
    return;
  }
  tmp = mct_list_find_custom(port_data->stream_list, &identity,
    sensor_util_find_stream);
  if (!tmp) {
    SLOW("identity=0x%x, not found in stream list", identity);
  } else {
    stream_info = tmp->data;
    bundle_id = stream_info->bundle_id;
    port_data->stream_list = mct_list_remove(port_data->stream_list,
      stream_info);
    free(stream_info);
    if (!port_data->stream_list)
      MCT_PORT_PEER(port) = NULL;
  }
  /* remove entry in bundle list, if stream is part of bundle */
  if (bundle_id != -1) {
    bundle_info = sensor_util_find_bundle_by_id(port, bundle_id);
    if (!bundle_info) {
      SLOW("bundle_id=%d, not found in bundle list, \
might be removed already", bundle_id);
    } else {
      port_data->bundle_list = mct_list_remove(port_data->bundle_list,
        bundle_info);
      free(bundle_info);
    }
  }
}

boolean sensor_util_get_sbundle(mct_module_t *s_module,
  uint32_t identity, sensor_bundle_info_t *bundle_info)
{
  boolean                           ret = FALSE;
  uint32_t                          session_id = 0, stream_id = 0;
  module_sensor_bundle_info_t      *s_bundle = NULL;
  module_sensor_ctrl_t             *module_ctrl = NULL;
  mct_list_t                       *s_list = NULL;

  module_ctrl = (module_sensor_ctrl_t *)s_module->module_private;
  if (!module_ctrl || !bundle_info) {
    SERR("failed module ctrl %p bundle info %p", module_ctrl, bundle_info);
    return FALSE;
  }

  ret = sensor_util_unpack_identity(identity, &session_id, &stream_id);
  if (ret == FALSE) {
    SERR("failed");
    return FALSE;
  }

  s_list = mct_list_find_custom(module_ctrl->sensor_bundle, &session_id,
    sensor_util_find_bundle);
  if (!s_list) {
    SERR("failed");
    return FALSE;
  }

  s_bundle = (module_sensor_bundle_info_t *)s_list->data;
  if (!s_bundle) {
    SERR("failed");
    return FALSE;
  }

  /* Fill bundle info */
  bundle_info->s_bundle = s_bundle;
  bundle_info->session_id = session_id;
  bundle_info->stream_id = stream_id;

  return TRUE;
}

boolean sensor_util_set_digital_gain_to_isp(mct_module_t* module,
  module_sensor_bundle_info_t* s_bundle, uint32_t identity,
  sensor_aec_update_t *aec_update)
{
  float       digital_gain;
  mct_event_t new_event;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(s_bundle);

  module_sensor_params_t *module_sensor_params =
     s_bundle->module_sensor_params[SUB_MODULE_SENSOR];

  if (aec_update == NULL) {
    /* get current digital gain from sensor */
    if (module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_GET_DIGITAL_GAIN, &digital_gain) < 0) {
      SERR("SENSOR_GET_DIGITAL_GAIN failed");
      return FALSE;
    }
    if (digital_gain == s_bundle->digital_gain)
      return TRUE;
  } else {
    /* get digital gain from a given gain, lc */
    sensor_digital_gain_t gain_info;
    gain_info.aec_update = aec_update;

    if (module_sensor_params->func_tbl.process(
      module_sensor_params->sub_module_private,
      SENSOR_GET_AEC_DIGITAL_GAIN, &gain_info) < 0) {
        SERR("SENSOR_GET_AEC_DIGITAL_GAIN failed");
        return FALSE;
    }
    digital_gain = gain_info.digital_gain;
  }

  new_event.type = MCT_EVENT_MODULE_EVENT;
  new_event.identity =
    sensor_util_get_session_stream_identity(module, identity);
  new_event.direction = MCT_EVENT_DOWNSTREAM;
  new_event.u.module_event.type = MCT_EVENT_MODULE_SET_DIGITAL_GAIN;
  new_event.u.module_event.current_frame_id = s_bundle->last_idx;
  new_event.u.module_event.module_event_data = &digital_gain;
  if (sensor_util_post_event_on_src_port(module, &new_event) == FALSE) {
    SERR("failed");
    return FALSE;
  }
  s_bundle->digital_gain = digital_gain;
  return TRUE;
}

int32_t sensor_util_set_frame_skip_to_isp(mct_module_t* module,
  uint32_t identity, enum msm_vfe_frame_skip_pattern frame_skip_pattern)
{
  boolean ret = TRUE;
  mct_event_t new_event;

  if (!module || !identity) {
    SERR("failed");
    return FALSE;
  }

  new_event.type = MCT_EVENT_MODULE_EVENT;
  new_event.identity = identity;
  new_event.direction = MCT_EVENT_DOWNSTREAM;
  new_event.u.module_event.type = MCT_EVENT_MODULE_REQUEST_FRAME_SKIP;
  new_event.u.module_event.module_event_data = (void *)&frame_skip_pattern;

  SHIGH("Setting frame skip pattern to ISP. Pattern = %d",
    frame_skip_pattern);

  ret = sensor_util_post_event_on_src_port(module, &new_event);

  if (ret == FALSE) {
    SERR("failed");
  }

  return ret;
}

int32_t sensor_util_sw_frame_skip_to_isp(mct_module_t *module,
  uint32_t identity, module_sensor_bundle_info_t *s_bundle,
  iface_skip_mode mode, boolean frame_skip, boolean stats_skip,
  uint32_t max_frame_skip)
{
  boolean ret = TRUE;
  mct_event_t new_event;
  iface_sw_frameskip frameskip;

  if (!module || !identity) {
    SERR("failed");
    return FALSE;
  }

  if (!max_frame_skip)
    return TRUE;

  memset(&frameskip, 0, sizeof(frameskip));
  frameskip.min_frame_id = s_bundle->last_idx + 1;
  frameskip.max_frame_id = frameskip.min_frame_id + max_frame_skip - 1;
  frameskip.skip_mode = mode;
  frameskip.skip_image_frames = frame_skip;
  frameskip.skip_stats_frames = stats_skip;
  new_event.type = MCT_EVENT_MODULE_EVENT;
  new_event.identity = identity;
  new_event.direction = MCT_EVENT_DOWNSTREAM;
  new_event.u.module_event.type = MCT_EVENT_MODULE_REQUEST_SW_FRAME_SKIP;
  new_event.u.module_event.module_event_data = (void *)&frameskip;

  SHIGH("Setting frame skip to ISP. mode = %d, min = %d, max = %d",
    frameskip.skip_mode, frameskip.min_frame_id, frameskip.max_frame_id);

  ret = sensor_util_post_event_on_src_port(module, &new_event);

  if (ret == FALSE) {
    SERR("failed");
  }

  return ret;
}

int32_t sensor_util_is_previous_frame_sent(mct_module_t* module,
  mct_event_t *event, uint32_t identity_s, uint32_t identity_p)
{
  boolean ret = FALSE;
  uint32_t i;
  mct_bus_msg_isp_sof_t *sof_event;
  mct_event_control_t *event_module;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(event);
  RETURN_ON_NULL(event->u.ctrl_event.control_event_data);

  event_module = &event->u.ctrl_event;

  if ((MCT_EVENT_DOWNSTREAM != event->direction) ||
    (MCT_EVENT_CONTROL_CMD != event->type) ||
    (MCT_EVENT_CONTROL_SOF != event_module->type)) {
    SERR("failed worng event");
    return FALSE;
  }

  sof_event = (mct_bus_msg_isp_sof_t*) (event_module->control_event_data);

  for (i = 0; i < ARRAY_SIZE(sof_event->prev_sent_streamids); i++) {
    if (0 == sof_event->prev_sent_streamids[i]) {
      // No more valid previous stream id are present
      break;
    }
    if ((SENSOR_STREAMID(identity_s) == sof_event->prev_sent_streamids[i]) ||
        (SENSOR_STREAMID(identity_p) == sof_event->prev_sent_streamids[i])){
      // Frame was just sent for current stream
      ret = TRUE;
      break;
    }
  }

  return ret;
}

uint32_t sensor_util_get_fourcc_format(
  sensor_output_format_t output_format,
  sensor_filter_arrangement filter_arrangement, uint8_t decode_format)
{
  uint32_t pix_fmt_fourcc = 0;
  uint8_t decode_pack_format = 0;

  /* 4 LSB is for decode_format; 4 MSB is for RDI packing config */
  decode_pack_format = decode_format;

  /* RDI packing config required only for plain16 format, hence sending
     only decode format for the other usecases */
  decode_format = decode_format & 0xF;

  switch (output_format) {
  case SENSOR_META:
    switch (decode_format) {
      case CSI_DECODE_8BIT:
        pix_fmt_fourcc = MSM_V4L2_PIX_FMT_META;
        break;
      case CSI_DECODE_10BIT:
        pix_fmt_fourcc = MSM_V4L2_PIX_FMT_META10;
        break;
    }
    break;
  case SENSOR_GREY:
    switch (decode_format) {
      case CSI_DECODE_8BIT:
        pix_fmt_fourcc = V4L2_PIX_FMT_GREY;
        break;
      case CSI_DECODE_10BIT:
        pix_fmt_fourcc = V4L2_PIX_FMT_Y10;
        break;
      case CSI_DECODE_12BIT:
        pix_fmt_fourcc = V4L2_PIX_FMT_Y12;
        break;
    }
    break;
  case SENSOR_BAYER:
  case SENSOR_YCBCR:
  case SENSOR_BAYER_SVHDR_SUBFRAME:
    switch(filter_arrangement) {
    case SENSOR_BGGR:
      switch (decode_pack_format) {
        case CSI_DECODE_8BIT:
          pix_fmt_fourcc = V4L2_FOURCC(BGGR, 8);
          break;
        case CSI_DECODE_10BIT:
          pix_fmt_fourcc = V4L2_FOURCC(BGGR, 10);
          break;
        case CSI_DECODE_DPCM_10_6_10:
          pix_fmt_fourcc = V4L2_FOURCC(BGGR, 10DPCM6);
          break;
        case CSI_DECODE_DPCM_10_8_10:
          pix_fmt_fourcc = V4L2_FOURCC(BGGR, 10DPCM8);
          break;
        case CSI_DECODE_12BIT:
          pix_fmt_fourcc = V4L2_FOURCC(BGGR, 12);
          break;
        case CSI_DECODE_14BIT:
          pix_fmt_fourcc = V4L2_FOURCC(BGGR, 14);
          break;
        case CSI_DECODE_10BIT_PLAIN16_LSB:
          pix_fmt_fourcc = V4L2_FOURCC(BGGR, PLAIN16);
          break;
      }
      break;
    case SENSOR_GBRG:
      switch (decode_pack_format) {
        case CSI_DECODE_8BIT:
          pix_fmt_fourcc = V4L2_FOURCC(GBRG, 8);
          break;
        case CSI_DECODE_10BIT:
          pix_fmt_fourcc = V4L2_FOURCC(GBRG, 10);
          break;
        case CSI_DECODE_DPCM_10_6_10:
          pix_fmt_fourcc = V4L2_FOURCC(GBRG, 10DPCM6);
          break;
        case CSI_DECODE_DPCM_10_8_10:
          pix_fmt_fourcc = V4L2_FOURCC(GBRG, 10DPCM8);
          break;
        case CSI_DECODE_12BIT:
          pix_fmt_fourcc = V4L2_FOURCC(GBRG, 12);
          break;
        case CSI_DECODE_14BIT:
          pix_fmt_fourcc = V4L2_FOURCC(GBRG, 14);
          break;
        case CSI_DECODE_10BIT_PLAIN16_LSB:
          pix_fmt_fourcc = V4L2_FOURCC(GBRG, PLAIN16);
          break;
      }
      break;
    case SENSOR_GRBG:
      switch (decode_pack_format) {
        case CSI_DECODE_8BIT:
          pix_fmt_fourcc = V4L2_FOURCC(GRBG, 8);
          break;
        case CSI_DECODE_10BIT:
          pix_fmt_fourcc = V4L2_FOURCC(GRBG, 10);
          break;
        case CSI_DECODE_DPCM_10_6_10:
          pix_fmt_fourcc = V4L2_FOURCC(GRBG, 10DPCM6);
          break;
        case CSI_DECODE_DPCM_10_8_10:
          pix_fmt_fourcc = V4L2_FOURCC(GRBG, 10DPCM8);
          break;
        case CSI_DECODE_12BIT:
          pix_fmt_fourcc = V4L2_FOURCC(GRBG, 12);
          break;
        case CSI_DECODE_14BIT:
          pix_fmt_fourcc = V4L2_FOURCC(GRBG, 14);
          break;
        case CSI_DECODE_10BIT_PLAIN16_LSB:
          pix_fmt_fourcc = V4L2_FOURCC(GRBG, PLAIN16);
          break;
      }
      break;
    case SENSOR_RGGB:
      switch (decode_pack_format) {
        case CSI_DECODE_8BIT:
          pix_fmt_fourcc = V4L2_FOURCC(RGGB, 8);
          break;
        case CSI_DECODE_10BIT:
          pix_fmt_fourcc = V4L2_FOURCC(RGGB, 10);
          break;
        case CSI_DECODE_DPCM_10_6_10:
          pix_fmt_fourcc = V4L2_FOURCC(RGGB, 10DPCM6);
          break;
        case CSI_DECODE_DPCM_10_8_10:
          pix_fmt_fourcc = V4L2_FOURCC(RGGB, 10DPCM8);
          break;
        case CSI_DECODE_12BIT:
          pix_fmt_fourcc = V4L2_FOURCC(RGGB, 12);
          break;
        case CSI_DECODE_14BIT:
          pix_fmt_fourcc = V4L2_FOURCC(RGGB, 14);
          break;
        case CSI_DECODE_10BIT_PLAIN16_LSB:
          pix_fmt_fourcc = V4L2_FOURCC(RGGB, PLAIN16);
          break;
      }
      break;
    case SENSOR_UYVY:
      switch (decode_format) {
        case CSI_DECODE_8BIT:
          pix_fmt_fourcc = V4L2_MBUS_FMT_UYVY8_2X8;
          break;
      }
      break;
    case SENSOR_YUYV:
      switch (decode_format) {
        case CSI_DECODE_8BIT:
          pix_fmt_fourcc = V4L2_MBUS_FMT_YUYV8_2X8;
          break;
      }
      break;
    case SENSOR_Y:
      switch (decode_format) {
        case CSI_DECODE_8BIT:
          pix_fmt_fourcc = V4L2_PIX_FMT_GREY;
          break;
        case CSI_DECODE_10BIT:
          pix_fmt_fourcc = V4L2_PIX_FMT_Y10;
          break;
        case CSI_DECODE_12BIT:
          pix_fmt_fourcc = V4L2_PIX_FMT_Y12;
          break;
      }
      break;
    }
    break;
  default:
    break;
  }

  return pix_fmt_fourcc;
}

boolean sensor_util_check_format(sensor_src_port_cap_t *caps,
  mct_stream_info_t *stream_info)
{
  boolean ret = FALSE;
  int32_t i = 0;

  /* Validate input parameters */
  RETURN_ON_NULL(caps);
  RETURN_ON_NULL(stream_info);

  SLOW("stream fmt %d", stream_info->fmt);
  for (i = 0; i < caps->num_cid_ch; i++) {
    SLOW("caps fmt %d", caps->sensor_cid_ch[i].fmt);
    /* Check whether incoming format request is compabile to current
       port's supported format */
    switch (stream_info->fmt) {
    case CAM_FORMAT_JPEG:
    case CAM_FORMAT_JPEG_RAW_8BIT:
      if ((caps->sensor_cid_ch[i].fmt == CAM_FORMAT_JPEG) ||
          (caps->sensor_cid_ch[i].fmt == CAM_FORMAT_JPEG_RAW_8BIT)) {
        return TRUE;
      }
      break;
    case CAM_FORMAT_YUV_420_NV12:
    case CAM_FORMAT_YUV_420_NV21:
    case CAM_FORMAT_YUV_420_NV21_ADRENO:
    case CAM_FORMAT_YUV_420_YV12:
    case CAM_FORMAT_YUV_422_NV16:
    case CAM_FORMAT_YUV_422_NV61:
    case CAM_FORMAT_YUV_420_NV12_VENUS:
    case CAM_FORMAT_YUV_420_NV12_UBWC:
    case CAM_FORMAT_YUV_420_NV21_VENUS:
    case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
    case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
    case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
    case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
    case CAM_FORMAT_Y_ONLY:
    case CAM_FORMAT_DEPTH8:
    case CAM_FORMAT_DEPTH16:
    case CAM_FORMAT_DEPTH_POINT_CLOUD:
      switch (caps->sensor_cid_ch[i].fmt) {
      case CAM_FORMAT_Y_ONLY:
      case CAM_FORMAT_Y_ONLY_10_BPP:
      case CAM_FORMAT_Y_ONLY_12_BPP:
      case CAM_FORMAT_YUV_420_NV12:
      case CAM_FORMAT_YUV_420_NV21:
      case CAM_FORMAT_YUV_420_NV21_ADRENO:
      case CAM_FORMAT_YUV_420_YV12:
      case CAM_FORMAT_YUV_422_NV16:
      case CAM_FORMAT_YUV_422_NV61:
      case CAM_FORMAT_YUV_420_NV12_VENUS:
      case CAM_FORMAT_YUV_420_NV12_UBWC:
      case CAM_FORMAT_YUV_420_NV21_VENUS:
      case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
      case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
      case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
      case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
      case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GREY:
      case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GREY:
      case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GREY:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GREY:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GREY:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GREY:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GREY:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GREY:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GREY:
      case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
      case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
      case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
      case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
      case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
      case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
      case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
      case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
      case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
      case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
      case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
      case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR:
      case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR:
      case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG:
      case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG:
      case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB:
      case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GBRG:
      case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GRBG:
      case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_RGGB:
      case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_BGGR:
      case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_BGGR:
        return TRUE;
      default:
        ret = FALSE;
        break;
      }
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GREY:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR:
      switch (caps->sensor_cid_ch[i].fmt) {
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
        return TRUE;
      default:
        ret = FALSE;
        break;
      }
      break;
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
      switch (caps->sensor_cid_ch[i].fmt) {
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
        return TRUE;
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
        if (sensor_util_is_plain_packing_supported(
          caps->sensor_cid_ch[i].csid_version))
          return TRUE;
        else
          ret = FALSE;
          break;
        default:
          ret = FALSE;
          break;
      }
      break;
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GREY:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB:
      switch (caps->sensor_cid_ch[i].fmt) {
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
        return TRUE;
      default:
        ret = FALSE;
        break;
      }
      break;
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GREY:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR:
      switch (caps->sensor_cid_ch[i].fmt) {
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
        return TRUE;
      default:
        ret = FALSE;
        break;
      }
      break;
    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_14BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GREY:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_14BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_14BPP_GREY:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_14BPP_BGGR:
      switch (caps->sensor_cid_ch[i].fmt) {
        case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG:
        case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG:
        case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB:
        case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR:
        case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GREY:
          return TRUE;
        default:
          ret = FALSE;
          break;
        }
    break;
    case CAM_FORMAT_META_RAW_8BIT:
    case CAM_FORMAT_META_RAW_10BIT:
      break;
    default:
      ret = FALSE;
      break;
    }
  }
  for (i = 0; i < caps->num_meta_ch; i++) {
  switch (stream_info->fmt) {
    case CAM_FORMAT_META_RAW_8BIT:
      if (caps->meta_ch[i].fmt == CAM_FORMAT_META_RAW_8BIT) {
        return TRUE;
      }
      break;
    case CAM_FORMAT_META_RAW_10BIT:
      if (caps->meta_ch[i].fmt == CAM_FORMAT_META_RAW_10BIT) {
        return TRUE;
      }
      break;
    default:
      return FALSE;
    }
  }
  return ret;
}

/** sensor_util_post_bus_sensor_params: post sensor params to
 *  bus
 *
 *  @data1: sensor bundle info
 *  @data2: session id
 *
 *  Return: TRUE for sucess and FALSE for failure
 *
 *  This function matches session id with sensor video node
 *  id from sensor info passed to it and returns decision
 **/

boolean sensor_util_post_bus_sensor_params(mct_module_t *s_module,
  module_sensor_bundle_info_t *s_bundle,
  uint32_t __attribute__((unused)) identity)
{
  boolean                ret = TRUE;
  mct_bus_msg_t          bus_msg;

  /* Validate input parameters */
  RETURN_ON_NULL(s_module);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(s_bundle->sensor_info);

  SLOW("aperture %f", s_bundle->sensor_params.aperture_value);

  /* Fill bus msg params */
  memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.sessionid = s_bundle->sensor_info->session_id;
  bus_msg.type = MCT_BUS_MSG_SENSOR_INFO;
  bus_msg.msg = &s_bundle->sensor_params;
  bus_msg.size = sizeof(s_bundle->sensor_params);
  ret = mct_module_post_bus_msg(s_module, &bus_msg);
  if (ret == FALSE) {
    SERR("failed");
  }

  return ret;
}

/** sensor_util_post_to_bus: post message to bus
 *
 *
 *  @data1: sensor bundle info
 *  @data2: type
 *  @data3: size
 *  @data4: msg
 *
 *  Return: TRUE for sucess and FALSE for failure
 *
 **/
boolean sensor_util_post_to_bus(mct_module_t* module,
  module_sensor_bundle_info_t *s_bundle, mct_bus_msg_type_t type,
  uint32_t size, void *msg)
{
  mct_bus_msg_t          bus_msg;
  boolean                ret = TRUE;
  /* Validate input parameters */
  RETURN_ON_NULL(module);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(s_bundle->sensor_info);

  SLOW("Post message %d", type);
  /* Fill bus msg params */
  memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.sessionid = s_bundle->sensor_info->session_id;
  bus_msg.type      = type;
  bus_msg.size      = size;
  bus_msg.msg       = msg;
  ret = mct_module_post_bus_msg(module, &bus_msg);
  if (ret == FALSE) {
    SERR("failed");
  }
  return ret;
}

static boolean sensor_util_find_is_bundle_started(void *data1,
  void __attribute__((unused)) *data2)
{
  module_sensor_port_bundle_info_t *bundle =
    (module_sensor_port_bundle_info_t *)data1;

  RETURN_ON_NULL(bundle);

  /* Find whether all streams in this bundle has started streaming */
  if (bundle->stream_on_count == bundle->bundle_config.num_of_streams) {
    return TRUE;
  }

  return FALSE;
}

boolean sensor_util_find_is_any_bundle_started(mct_port_t *port)
{
  module_sensor_port_data_t *port_data = NULL;
  mct_list_t                *blist = NULL;

  /* Validate input paramters */
  RETURN_ON_NULL(port);

  /* Extract port private */
  port_data = (module_sensor_port_data_t *)port->port_private;
  RETURN_ON_NULL(port_data);

  /* Check whether there is any bundle for this session where all streams
     present in that bundle has already streamed ON */
  blist = mct_list_find_custom(port_data->bundle_list, NULL,
   sensor_util_find_is_bundle_started);
  if (!blist) {
    /* Either no bundle exist for this session or there is no bundle where all
       streams in that bundle has started streaming */
    return FALSE;
  }

  return TRUE;
}

/** sensor_util_post_downstream_event: post LED state message on
 *  bus
 *
 *  @s_module: mct module handle
 *  @identity: identity of event to be posted downstream
 *  @type: event type
 *  @data: data
 *
 *  Return: TRUE for success and FALSE on failure
 *
 *  This function creates module event and posts downstream
 **/

boolean sensor_util_post_downstream_event(mct_module_t *s_module,
  uint32_t identity, mct_event_module_type_t type, void *data)
{
  boolean ret = TRUE;
  mct_event_t event;

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = sensor_util_get_session_stream_identity(s_module, identity);
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = type;
  event.u.module_event.module_event_data = data;
  ret = sensor_util_post_event_on_src_port(s_module, &event);
  if (ret == FALSE) {
     SERR("failed");
   }

  return ret;
}

/** sensor_util_post_led_state_msg: post LED state message on
 *  bus
 *
 *  @s_module: mct module handle
 *  @s_bundle: sensor bundle handle
 *
 *  Return: TRUE for success and FALSE on failure
 *
 *  This function creates LED bus message and posts on bus
 **/

boolean sensor_util_post_led_state_msg(mct_module_t *s_module,
  module_sensor_bundle_info_t *s_bundle, uint32_t identity)
{
  boolean ret = TRUE;

  /* Validate input parameters */
  RETURN_ON_NULL(s_module);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(s_bundle->sensor_info);

  /* Post downstream event */
  SHIGH("Sending flash mode downstream - %d",
    s_bundle->sensor_params.flash_mode);
  ret = sensor_util_post_downstream_event(s_module, identity,
    MCT_EVENT_MODULE_SET_FLASH_MODE, &s_bundle->sensor_params.flash_mode);
  if (ret == FALSE) {
    SERR("failed");
  }

  return ret;
}

/** sensor_util_post_braketing_state_event: post bracketing state event
 *
 *  @s_module: mct module handle
 *  @s_bundle: sensor bundle handle
 *  @identity: identity
 *  @bracketing: bracketing state
 *  @index: current bracketing index
 *
 *  Return: TRUE for success and FALSE on failure
 *
 *  This function creates bracketing update event and sends it downstream
 **/

boolean sensor_util_post_braketing_state_event(mct_module_t *s_module,
  module_sensor_bundle_info_t *s_bundle, uint32_t identity, boolean bracketing,
  uint32_t index)
{
  boolean ret = TRUE;
  mct_bracketing_update_t bracketing_update;

  /* Validate input parameters */
  RETURN_ON_NULL(s_module);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(s_bundle->sensor_info);

  if (bracketing) {
    bracketing_update.state = MCT_BRACKETING_STATE_ON;
  } else {
    bracketing_update.state = MCT_BRACKETING_STATE_OFF;
  }
  bracketing_update.index = index;

  /* Post downstream event */
  ret = sensor_util_post_downstream_event(s_module, identity,
    MCT_EVENT_MODULE_BRACKETING_UPDATE, &bracketing_update);
  if (ret == FALSE) {
    SERR("failed");
  }

  return ret;
}

/** sensor_util_send_led_mode_downstream:
 *
 *  @s_module: mct module handle
 *  @flash_params: flash parameters
 *  @identity: identity
 *  @frame_id: frame id
 *
 *  Return: TRUE for success and FALSE on failure
 *
 *  This function sends LED state transition downstream
 **/
boolean sensor_util_send_led_mode_downstream(mct_module_t *s_module,
  cam_flash_mode_t *flash_mode, uint32_t identity, uint32_t frame_id)
{
  boolean     ret = TRUE;
  mct_event_t new_event;
  mct_port_t *src_port;

  /* Validate input parameters */
  RETURN_ON_NULL(s_module);
  RETURN_ON_NULL(flash_mode);

  src_port = sensor_util_find_src_port_with_identity(
    s_module, identity);
  if (src_port) {
    new_event.type = MCT_EVENT_MODULE_EVENT;
    new_event.identity =
      sensor_util_get_session_stream_identity(s_module, identity);;
    new_event.direction = MCT_EVENT_DOWNSTREAM;
    new_event.u.module_event.type = MCT_EVENT_MODULE_SET_FLASH_MODE;
    new_event.u.module_event.current_frame_id = frame_id;
    new_event.u.module_event.module_event_data = (void *)flash_mode;

    /* Post downstream event */
    SHIGH("Sending flash mode downstream - %d", *flash_mode);
    ret = mct_port_send_event_to_peer(src_port, &new_event);
    if (ret == FALSE) {
      SERR("failed");
    }
  }

  return ret;
}

/** sensor_util_get_submod:
 *
 *  @s_bundle: handle to sensor params
 *  @submod_type: sub module type
 *
 *  Return: sub module handle if success
 *          NULL for failure
 *
 *  This function returns sub module handle
 **/
module_sensor_params_t *sensor_util_get_submod(
  module_sensor_bundle_info_t *s_bundle,
  enum sensor_sub_module_t submod_type)
{
  RETURN_ON_NULL(s_bundle);

  if (submod_type >= SUB_MODULE_MAX) {
    SERR("failed: submod_type %d", submod_type);
    return NULL;
  }

  return s_bundle->module_sensor_params[submod_type];
}

/** sensor_util_validate_event:
 *
 *  @submod_type: sub module type
 *  @event: sub module event
 *
 *  Return: TRUE for success
 *          FALSE for failure
 *
 *  This function validates whether event belongs to the sub
 *  module
 **/
static boolean sensor_util_validate_event(enum sensor_sub_module_t submod_type,
  sensor_submodule_event_type_t event)
{
  boolean ret = TRUE;

  if ((submod_type >= SUB_MODULE_MAX) || (event >= SENSOR_SUBMOD_EVENT_MAX)) {
    SERR("failed: submod_type %d", submod_type);
    return FALSE;
  }

  switch (submod_type) {
  case SUB_MODULE_SENSOR:
    if ((event <= SENSOR_ENUM_MIN) || (event >= SENSOR_ENUM_MAX)) {
      SERR("failed: invalid event %d submod type %d",
        event, submod_type);
      ret = FALSE;
    }
    break;
  case SUB_MODULE_CHROMATIX:
    if ((event <= CHROMATIX_ENUM_MIN) || (event >= CHROMATIX_ENUM_MAX)) {
      SERR("failed: invalid event %d submod type %d", event, submod_type);
      ret = FALSE;
    }
    break;
  case SUB_MODULE_ACTUATOR:
    if ((event <= ACTUATOR_ENUM_MIN) || (event >= ACTUATOR_ENUM_MAX)) {
      SERR("failed: invalid event %d submod type %d", event, submod_type);
      ret = FALSE;
    }
    break;
  case SUB_MODULE_EEPROM:
    if ((event <= EEPROM_ENUM_MIN) || (event >= EEPROM_ENUM_MAX)) {
      SERR("failed: invalid event %d submod type %d", event, submod_type);
      ret = FALSE;
    }
    break;
  case SUB_MODULE_LED_FLASH:
    if ((event <= LED_FLASH_ENUM_MIN) || (event >= LED_FLASH_ENUM_MAX)) {
      SERR("failed: invalid event %d submod type %d", event, submod_type);
      ret = FALSE;
    }
    break;
  case SUB_MODULE_STROBE_FLASH:
    if ((event <= STROBE_FLASH_ENUM_MIN) || (event >= STROBE_FLASH_ENUM_MAX)) {
      SERR("failed: invalid event %d submod type %d", event, submod_type);
      ret = FALSE;
    }
    break;
  case SUB_MODULE_CSID:
    if ((event <= CSID_ENUM_MIN) || (event >= CSID_ENUM_MAX)) {
      SERR("failed: invalid event %d submod type %d", event, submod_type);
      ret = FALSE;
    }
    break;
  case SUB_MODULE_CSIPHY:
    if ((event <= CSIPHY_ENUM_MIN) || (event >= CSIPHY_ENUM_MAX)) {
      SERR("failed: invalid event %d submod type %d", event, submod_type);
      ret = FALSE;
    }
    break;
  case SUB_MODULE_OIS:
    if ((event <= OIS_ENUM_MIN) || (event >= OIS_ENUM_MAX)) {
      SERR("failed: invalid event %d submod type %d", event, submod_type);
      ret = FALSE;
    }
    break;
  case SUB_MODULE_IR_LED:
    if ((event <= IR_LED_ENUM_MIN) || (event >= IR_LED_ENUM_MAX)) {
      SERR("failed: invalid event %d submod type %d", event, submod_type);
      ret = FALSE;
    }
    break;
  case SUB_MODULE_IR_CUT:
    if ((event <= IR_CUT_ENUM_MIN) || (event >= IR_CUT_ENUM_MAX)) {
      SERR("failed: invalid event %d submod type %d", event, submod_type);
      ret = FALSE;
    }
    break;
  case SUB_MODULE_LASER_LED:
    if ((event <= LASER_LED_ENUM_MIN) || (event >= LASER_LED_ENUM_MAX)) {
      SERR("failed: invalid event %d submod type %d", event, submod_type);
      ret = FALSE;
    }
    break;

  case SUB_MODULE_CSID_3D:
  case SUB_MODULE_CSIPHY_3D:
  default:
    SERR("failed: invalid event %d submod type %d", event, submod_type);
    ret = FALSE;
    break;
  }

  return ret;
}

/** sensor_util_post_submod_event:
 *
 *  @s_bundle: handle to sensor params
 *  @submod_type: sub module type
 *  @event: sub module event
 *  @data: data specific to event
 *
 *  Return: TRUE for success
 *          FALSE for failure
 *
 *  This function retrieves sub module handle and posts event
 *  along with sub module private handle
 **/
boolean sensor_util_post_submod_event(module_sensor_bundle_info_t *s_bundle,
  enum sensor_sub_module_t submod_type,
  sensor_submodule_event_type_t event,
  void *data)
{
  int32_t ret = SENSOR_SUCCESS;
  module_sensor_params_t *submod = NULL;

  /* Validate intput parameters */
  RETURN_ON_NULL(s_bundle);

  if ((submod_type >= SUB_MODULE_MAX) || (event >= SENSOR_SUBMOD_EVENT_MAX)) {
    SERR("failed: submod_type %d", submod_type);
    return FALSE;
  }

  /* Check whether event belongs to sub module supported list of events */
  RETURN_ON_FALSE(sensor_util_validate_event(submod_type, event));

  /* Retrieve sub module handle from sensor params */
  submod = s_bundle->module_sensor_params[submod_type];

  /* Validate submodule parameters */
  RETURN_ON_NULL(submod);
  RETURN_ON_NULL(submod->func_tbl.process);
  RETURN_ON_NULL(submod->sub_module_private);

  /* Call process function on sub module */
  ret = submod->func_tbl.process(submod->sub_module_private, event, data);
  if (ret < SENSOR_SUCCESS) {
    SERR("failed: Call to submodule failed");
    return FALSE;
  }

  return TRUE;
}

boolean sensor_util_post_intramode_event(mct_module_t *module,
  uint32_t identity, uint32_t peer_identity,
  mct_event_module_type_t type, void *data)
{
  mct_event_t              event;
  mct_inter_peer_entity_t *intra_port_entity;
  mct_port_t              *intra_port;
  mct_port_t              *src_port;
  uint8_t                  i;

  RETURN_ON_NULL(module);

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = peer_identity;
  event.direction = MCT_EVENT_INTRA_MOD;
  event.u.module_event.type = type;
  event.u.module_event.module_event_data = data;

  src_port = sensor_util_find_src_port_with_identity(module, identity);
  RETURN_ON_NULL(src_port);
  return mct_port_send_intra_module_event(src_port, &event);
}

boolean sensor_util_link_intra_session(mct_module_t *module, mct_event_t *event,
  uint32_t peer_identity, mct_port_t **this_port, mct_port_t **peer_port)
{
  uint32_t session_id, stream_id;

  RETURN_ON_NULL(module);
  RETURN_ON_NULL(event);
  RETURN_ON_NULL(this_port);
  RETURN_ON_NULL(peer_port);

  sensor_util_unpack_identity(event->identity, &session_id, &stream_id);
  SLOW("this session_id:%d stream:%d", session_id, stream_id);

  *this_port = sensor_util_find_src_port_with_identity(module, event->identity);
  RETURN_ON_NULL(*this_port);

  *peer_port = mct_port_find_intra_port_across_session(MCT_PORT_SRC, *this_port,
    peer_identity);
  if (*peer_port == NULL || *peer_port == *this_port) {
    SERR("can't find peer port(the src port of session id %d)",
      peer_identity >> 16);
    return FALSE;
  }

  SHIGH("LINK sessions %d(port %p) and %d(port %p)",
    session_id, *this_port, peer_identity >> 16, *peer_port);

  return TRUE;
}

module_sensor_bundle_info_t *sensor_util_get_peer_bundle(
  mct_module_t *module, module_sensor_bundle_info_t *my_bundle)
{
  sensor_bundle_info_t peer_bundle;

  if (!module || !my_bundle) {
    SERR("failed:null pointer");
    return NULL;
  }

  if (sensor_util_get_sbundle(module, my_bundle->peer_identity, &peer_bundle)
    == FALSE) {
    SERR("failed:can't find peer bundle");
    return NULL;
  }

  return peer_bundle.s_bundle;
}

static float sRGB2XYZ [3][3] = {
  {0.4360747, 0.3850649, 0.1430804},
  {0.2225045, 0.7168786, 0.0606169},
  {0.0139322, 0.0971045, 0.7141733}
};
static float XYZ2RGB[3][3] = {
  {0.4124564,  0.3575761,  0.1804375},
  {0.2126729,  0.7151522,  0.0721750},
  {0.0193339,  0.1191920,  0.9503041}
};

static float D65_to_ref_A[3][3] = {
  {1.2164557, 0.1109905,  -0.1549325},
  {0.1533326,  0.9152313,  -0.0559953},
  {-0.0239469, 0.0358984,  0.3147529}
};

void sensor_generate_transfrom_matrix(
  cam_rational_type_t forward_mat[3][3], cam_rational_type_t color_mat[3][3],
  chromatix_color_correction_type *cc, chromatix_manual_white_balance_type *wb,
  boolean isD65)
{
  float cc_mat[3][3], wb_mat[3][3], out_mat[3][3];
  float *ptr1, *ptr2;
  float tmp[3][3], tmp1[3][3], tmp2[3][3];

  cc_mat[0][0] = cc->c0; cc_mat[0][1] = cc->c1; cc_mat[0][2] = cc->c2;
  cc_mat[1][0] = cc->c3; cc_mat[1][1] = cc->c4; cc_mat[1][2] = cc->c5;
  cc_mat[2][0] = cc->c6; cc_mat[2][1] = cc->c7; cc_mat[2][2] = cc->c8;

  wb_mat[0][0] = wb->r_gain;
  wb_mat[0][1] = 0;
  wb_mat[0][2] = 0;
  wb_mat[1][0] = 0;
  wb_mat[1][1] = wb->g_gain;
  wb_mat[1][2] = 0;
  wb_mat[2][0] = 0;
  wb_mat[2][1] = 0;
  wb_mat[2][2] = wb->b_gain;

  /* Forward Transform: sRGB2XYZ * CC */
  MATRIX_MULT(sRGB2XYZ, cc_mat, out_mat, 3, 3, 3);
  FLOAT_TO_RATIONAL(out_mat, forward_mat, 3, 3);

  //CM = Invert (H * W * CC' * G) 1510
  if (isD65 == TRUE) {
    MATRIX_MULT(XYZ2RGB, cc_mat, tmp1, 3, 3, 3);
  } else {
    MATRIX_MULT(D65_to_ref_A, XYZ2RGB, tmp, 3, 3, 3);
    MATRIX_MULT(tmp, cc_mat, tmp1, 3, 3, 3);
  }
  MATRIX_MULT(tmp1, wb_mat, tmp2, 3, 3, 3);

  ptr1 = (float*) tmp2;
  ptr2 = (float*) out_mat;
  MATRIX_INVERSE_3x3(ptr1, ptr2);

  FLOAT_TO_RATIONAL(out_mat, color_mat, 3, 3);
}

void sensor_generate_calib_trans(cam_rational_type_t matrix[3][3],
  float r_gain, float b_gain)
{
  float wb_cal[3][3] = {{0,0,0},
    {0,0,0},
    {0,0,0}};

  wb_cal[0][0] = r_gain;
  wb_cal[1][1] = 1;
  wb_cal[2][2] = b_gain;

  FLOAT_TO_RATIONAL(wb_cal, matrix, 3, 3);
}

void sensor_generate_unit_matrix(cam_rational_type_t matrix[3][3])
{
  float wb_cal[3][3] = {{1,0,0},
    {0,1,0},
    {0,0,1}};

  FLOAT_TO_RATIONAL(wb_cal, matrix, 3, 3);
}

/** sensor_util_validate_chromatix_params: set chroamtix pointer
 *
 *  @chromatix_params: chromatix_params which has
 *    all chromatix pointers of sub modules.
 *
 *  return TRUE if pointers are non-null else return FALSE
 **/
boolean sensor_util_validate_chromatix_params(
  sensor_chromatix_params_t *chromatix_params)
{
  RETURN_ON_NULL(chromatix_params);
  RETURN_ON_NULL(chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_ISP]);
  RETURN_ON_NULL(chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_ISP_COMMON]);
  RETURN_ON_NULL(chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_CPP_PREVIEW]);
  RETURN_ON_NULL(chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_CPP_VIDEO]);
  RETURN_ON_NULL(
    chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_CPP_SNAPSHOT]);
  RETURN_ON_NULL(chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_SW_PPROC]);
  RETURN_ON_NULL(chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_3A]);
  return TRUE;
}
/** sensor_util_post_chromatix_event_downstream
 *
 *  @module: mct module handle
 *  @identity: identity for which event is to be sent
 *  @chromatix_params: pointer to chroamtix_params which
 *    contain chromatix pointers of down stream modules.
 *  @modules_reloaded: flag that mentions which modules
 *    chromatix has changed.
 *
 *  Return: TRUE on success and FALSE on failure
 *
 *  This function
 *  1) will validate the chroamtix_params
 *  2) find the reloaded modules if modules_reloaded is 0.
 *  3) send the SET_CHROMATIX_PTR event down stream
 **/
boolean sensor_util_post_chromatix_event_downstream(
  mct_module_t *module, module_sensor_bundle_info_t *s_bundle,
  uint32_t identity, sensor_chromatix_params_t *chromatix_params,
  uint16_t modules_reloaded)
{
  int32_t            i = 0;
  mct_bus_msg_t      bus_msg;
  mct_event_t        new_event;
  modulesChromatix_t module_chromatix;

  RETURN_ON_FALSE(module);
  RETURN_ON_FALSE(chromatix_params);
  RETURN_ON_FALSE(sensor_util_validate_chromatix_params(chromatix_params));

  if (!modules_reloaded) {
    for (i = 0; i < SENSOR_CHROMATIX_MAX; i++) {
      if (chromatix_params->chromatix_reloaded[i]) {
        switch(i) {
          case SENSOR_CHROMATIX_ISP:
          case SENSOR_CHROMATIX_ISP_SNAPSHOT:
          case SENSOR_CHROMATIX_ISP_COMMON:
            modules_reloaded |= CAMERA_CHROMATIX_MODULE_ISP;
              break;
          case SENSOR_CHROMATIX_CPP_PREVIEW:
          case SENSOR_CHROMATIX_CPP_SNAPSHOT:
          case SENSOR_CHROMATIX_CPP_FLASH_SNAPSHOT:
          case SENSOR_CHROMATIX_CPP_OIS_SNAPSHOT:
          case SENSOR_CHROMATIX_CPP_OIS_US_SNAPSHOT:
          case SENSOR_CHROMATIX_CPP_OIS_DS_SNAPSHOT:
          case SENSOR_CHROMATIX_CPP_DS:
          case SENSOR_CHROMATIX_CPP_US:
          case SENSOR_CHROMATIX_CPP_VIDEO:
            modules_reloaded |= CAMERA_CHROMATIX_MODULE_CPP;
            break;
          case SENSOR_CHROMATIX_SW_PPROC:
            modules_reloaded |= CAMERA_CHROMATIX_MODULE_SW_PPROC;
            break;
          case SENSOR_CHROMATIX_3A:
            modules_reloaded |= CAMERA_CHROMATIX_MODULE_3A;
            break;
          case SENSOR_CHROMATIX_IOT:
            modules_reloaded |= CAMERA_CHROMATIX_MODULE_IOT;
            break;
        }
      }
    }
  } else {
    for (i = 0; i < SENSOR_CHROMATIX_MAX; i++) {
      if (!chromatix_params->chromatix_lib_name[i]) {
        switch(i) {
          case SENSOR_CHROMATIX_ISP:
          case SENSOR_CHROMATIX_ISP_SNAPSHOT:
          case SENSOR_CHROMATIX_ISP_COMMON:
            modules_reloaded &= ~CAMERA_CHROMATIX_MODULE_ISP;
              break;
          case SENSOR_CHROMATIX_CPP_PREVIEW:
          case SENSOR_CHROMATIX_CPP_SNAPSHOT:
          case SENSOR_CHROMATIX_CPP_FLASH_SNAPSHOT:
          case SENSOR_CHROMATIX_CPP_OIS_SNAPSHOT:
          case SENSOR_CHROMATIX_CPP_OIS_US_SNAPSHOT:
          case SENSOR_CHROMATIX_CPP_OIS_DS_SNAPSHOT:
          case SENSOR_CHROMATIX_CPP_DS:
          case SENSOR_CHROMATIX_CPP_US:
          case SENSOR_CHROMATIX_CPP_VIDEO:
            modules_reloaded &= ~CAMERA_CHROMATIX_MODULE_CPP;
            break;
          case SENSOR_CHROMATIX_SW_PPROC:
            modules_reloaded &= ~CAMERA_CHROMATIX_MODULE_SW_PPROC;
            break;
          case SENSOR_CHROMATIX_3A:
            modules_reloaded &= ~CAMERA_CHROMATIX_MODULE_3A;
            break;
          case SENSOR_CHROMATIX_IOT:
            modules_reloaded &= ~CAMERA_CHROMATIX_MODULE_IOT;
            break;
        }
      }
    }
  }

  if (!modules_reloaded)
  {
    SLOW("Chromatix not reloading");
    return TRUE;
  }

  /* Pass ISP common pointer to bus and downstream */
  module_chromatix.chromatixComPtr =
    s_bundle->chromatix_metadata.common_chromatix_ptr =
      chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_ISP_COMMON];

  /* Pass ISP pointer to down stream */
  module_chromatix.chromatixPtr =
    chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_ISP];

  /* Pass ISP snapshot pointer to bus */
  s_bundle->chromatix_metadata.chromatix_ptr =
    chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_ISP_SNAPSHOT];

  /* Pass CPP preview pointer to bus and downstream */
  module_chromatix.chromatixCppPtr =
    s_bundle->chromatix_metadata.cpp_chromatix_ptr =
      chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_CPP_PREVIEW];

  /* Pass CPP snapshot pointer to bus and downstream */
  module_chromatix.chromatixSnapCppPtr =
    s_bundle->chromatix_metadata.cpp_snapchromatix_ptr =
      chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_CPP_SNAPSHOT];

  /* Pass CPP flash snapshot pointer to bus and downstream */
  module_chromatix.chromatixFlashSnapCppPtr =
    s_bundle->chromatix_metadata.cpp_flsh_snapchromatix_ptr =
      chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_CPP_FLASH_SNAPSHOT];

  /* Pass CPP downscale pointer to bus and downstream */
  module_chromatix.chromatixDsCppPtr =
    s_bundle->chromatix_metadata.cpp_ds_chromatix_ptr =
      chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_CPP_DS];

  /* Pass CPP upscale pointer to bus and downstream */
  module_chromatix.chromatixUsCppPtr =
    s_bundle->chromatix_metadata.cpp_us_chromatix_ptr =
      chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_CPP_US];

  /* Pass CPP video pointer to bus and downstream */
  module_chromatix.chromatixVideoCppPtr =
    s_bundle->chromatix_metadata.cpp_videochromatix_ptr =
      chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_CPP_VIDEO];

  /* Pass POSTPROC liveshot pointer to bus and downstream */
  module_chromatix.chromatixPostProcPtr =
    s_bundle->chromatix_metadata.postproc_chromatix_ptr =
      chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_SW_PPROC];

  /* Pass 3A pointer to bus and downstream */
  module_chromatix.chromatix3APtr =
    s_bundle->chromatix_metadata.a3_chromatix_ptr =
      chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_3A];

  /* Pass IOT pointer to bus and downstream */
  module_chromatix.chromatixIotPtr =
    s_bundle->chromatix_metadata.iot_chromatix_ptr =
      chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_IOT];

  /* Pass CPP OIS snapshot pointer to bus and downstream */
  module_chromatix.chromatixOisSnapCppPtr =
    s_bundle->chromatix_metadata.cpp_ois_snapchromatix_ptr =
      chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_CPP_OIS_SNAPSHOT];

  /* Pass CPP OIS US pointer to bus and downstream */
  module_chromatix.chromatixOisUsCppPtr =
    s_bundle->chromatix_metadata.cpp_ois_us_chromatix_ptr =
      chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_CPP_OIS_US_SNAPSHOT];

  /* Pass CPP OIS DS pointer to bus and downstream */
  module_chromatix.chromatixOisDsCppPtr =
    s_bundle->chromatix_metadata.cpp_ois_ds_chromatix_ptr =
      chromatix_params->chromatix_ptr[SENSOR_CHROMATIX_CPP_OIS_DS_SNAPSHOT];
  /* Pass external name downstream */
  module_chromatix.external =
    chromatix_params->chromatix_lib_name[SENSOR_CHROMATIX_EXTERNAL];

  s_bundle->chromatix_metadata.modules_reloaded =
    module_chromatix.modules_reloaded = modules_reloaded;

  SLOW("chromatixComPtr = %p", module_chromatix.chromatixComPtr);
  SLOW("chromatixPtr = %p", module_chromatix.chromatixPtr);
  SLOW("chromatixCppPtr = %p", module_chromatix.chromatixCppPtr);
  SLOW("chromatixSnapCppPtr = %p", module_chromatix.chromatixSnapCppPtr);
  SLOW("chromatixFlashSnapCppPtr = %p", module_chromatix.chromatixFlashSnapCppPtr);
  SLOW("chromatixDsCppPtr = %p", module_chromatix.chromatixDsCppPtr);
  SLOW("chromatixUsCppPtr = %p", module_chromatix.chromatixUsCppPtr);
  SLOW("chromatixVideoCppPtr = %p", module_chromatix.chromatixVideoCppPtr);
  SLOW("chromatixPostProcPtr = %p", module_chromatix.chromatixPostProcPtr);
  SLOW("chromatix3aPtr = %p", module_chromatix.chromatix3APtr);
  SLOW("chromatixIOTPtr = %p", module_chromatix.chromatixIotPtr);

  /* Send chromatix pointer downstream */
  new_event.type = MCT_EVENT_MODULE_EVENT;
  new_event.identity=sensor_util_get_session_stream_identity(module, identity);
  new_event.direction = MCT_EVENT_DOWNSTREAM;
  new_event.u.module_event.type = MCT_EVENT_MODULE_SET_CHROMATIX_PTR;
  new_event.u.module_event.module_event_data =
    (void *)&module_chromatix;
  RETURN_ON_FALSE(sensor_util_post_event_on_src_port(module, &new_event));

  memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.sessionid = s_bundle->sensor_info->session_id;
  bus_msg.type = MCT_BUS_MSG_SET_SENSOR_INFO;
  bus_msg.msg = (void *)&s_bundle->chromatix_metadata;
  bus_msg.size = sizeof(mct_bus_msg_sensor_metadata_t);

  if(mct_module_post_bus_msg(module, &bus_msg) == FALSE)
    SERR("posting chromatix to bus failed");

  return TRUE;
}

/** sensor_util_clear_special_mode_range: clear the bits in the range specified
 *
 *  @sensor_common_info: pointer to common sensor subdev info
 *  @special_mode_mask: specail mode to be set
 *  @start: start
 *
 *  return TRUE if pointers are non-null else return FALSE
 **/
inline static void sensor_util_clear_special_mode_range(
  sensor_submod_common_info_t *sensor_common_info,
  sensor_special_mode special_mode, uint64_t start, uint64_t end) {

  uint32_t i = 0;

  if (special_mode >= start && special_mode <= end)
    for (i = start; i <= end; i++)
      sensor_common_info->special_mode_mask &= ~(1 << i);
}

/** sensor_util_set_special_mode: set the special mode mask
 *
 *  @sensor_common_info: pointer to common sensor subdev info
 *  @special_mode_mask: specail mode to be set
 *  @enable: flag to set / unset the special mode
 *
 *  return TRUE if pointers are non-null else return FALSE
 **/
void sensor_util_set_special_mode(
  sensor_submod_common_info_t *sensor_common_info,
  sensor_special_mode special_mode, boolean enable)
{
  sensor_util_clear_special_mode_range(sensor_common_info, special_mode,
    SENSOR_SPECIAL_GROUP_ISO_BEGIN, SENSOR_SPECIAL_GROUP_ISO_END);

  sensor_util_clear_special_mode_range(sensor_common_info, special_mode,
    SENSOR_SPECIAL_GROUP_DIM_BEGIN, SENSOR_SPECIAL_GROUP_DIM_END);

  sensor_util_clear_special_mode_range(sensor_common_info, special_mode,
    SENSOR_SPECIAL_GROUP_SCENE_BEGIN, SENSOR_SPECIAL_GROUP_SCENE_END);

  sensor_util_clear_special_mode_range(sensor_common_info, special_mode,
    SENSOR_SPECIAL_GROUP_GLOBAL_BEGIN, SENSOR_SPECIAL_GROUP_GLOBAL_END);

  if (enable) {
    sensor_common_info->special_mode_mask |= (1LL << special_mode);
    sensor_common_info->prev_special_mode_mask =
      sensor_common_info->special_mode_mask;

    if (special_mode >= SENSOR_SPECIAL_GROUP_GLOBAL_BEGIN &&
      special_mode <= SENSOR_SPECIAL_GROUP_GLOBAL_END)
        sensor_common_info->special_mode_mask = (1LL << special_mode);
  }
  else {
    if (special_mode >= SENSOR_SPECIAL_GROUP_GLOBAL_BEGIN &&
      special_mode <= SENSOR_SPECIAL_GROUP_GLOBAL_END)
        sensor_common_info->special_mode_mask =
          sensor_common_info->prev_special_mode_mask;
    sensor_common_info->special_mode_mask &= ~(1LL << special_mode);
  }
}

/** sensor_util_get_setting_size: Get number of settings in the array
 *
 *  @settings: pointer to camera_i2c_reg_setting_array
 *
 *  return number of settings is success else return -1
 **/
int sensor_util_get_setting_size(
  struct camera_i2c_reg_array *settings, int32_t max_size)
{
  int32_t i = 0;
  for(i = 0; i < max_size; i++) {
    if (!settings[i].delay && !settings[i].reg_addr && !settings[i].reg_data)
      return i;
  }
  return -1;
}

/** sensor_util_get_format: Get the HAL cam format from fourcc format
 *
 *  @pix_fmt_fourcc: pixel fourcc format
 *
 *  return the HAL camera format
 **/
cam_format_t sensor_util_get_hal_format(uint32_t pix_fmt_fourcc)
{
  cam_format_t fmt;
  switch (pix_fmt_fourcc)
  {
    case V4L2_PIX_FMT_GREY:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY;
      break;
    case V4L2_PIX_FMT_Y10:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY;
      break;
    case V4L2_PIX_FMT_Y12:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY;
      break;
    case V4L2_PIX_FMT_SBGGR8:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR;
      break;
    case V4L2_PIX_FMT_SGBRG8:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG;
      break;
    case V4L2_PIX_FMT_SGRBG8:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG;
      break;
    case V4L2_PIX_FMT_SRGGB8:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB;
      break;

    case V4L2_PIX_FMT_SBGGR10:
    case V4L2_PIX_FMT_SBGGR10DPCM6:
    case V4L2_PIX_FMT_SBGGR10DPCM8:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR;
      break;
    case V4L2_PIX_FMT_SGBRG10:
    case V4L2_PIX_FMT_SGBRG10DPCM6:
    case V4L2_PIX_FMT_SGBRG10DPCM8:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG;
      break;
    case V4L2_PIX_FMT_SGRBG10:
    case V4L2_PIX_FMT_SGRBG10DPCM6:
    case V4L2_PIX_FMT_SGRBG10DPCM8:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG;
      break;
    case V4L2_PIX_FMT_SRGGB10:
    case V4L2_PIX_FMT_SRGGB10DPCM6:
    case V4L2_PIX_FMT_SRGGB10DPCM8:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB;
      break;

    case V4L2_PIX_FMT_SBGGR12:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR;
      break;
    case V4L2_PIX_FMT_SGBRG12:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG;
      break;
    case V4L2_PIX_FMT_SGRBG12:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG;
      break;
    case V4L2_PIX_FMT_SRGGB12:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB;
      break;
    case V4L2_PIX_FMT_SBGGRPLAIN16:
      fmt= CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_BGGR;
      break;
    case V4L2_PIX_FMT_SGBRGPLAIN16:
      fmt= CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GBRG;
      break;
    case V4L2_PIX_FMT_SGRBGPLAIN16:
      fmt= CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_GRBG;
      break;
    case V4L2_PIX_FMT_SRGGBPLAIN16:
      fmt= CAM_FORMAT_BAYER_RAW_PLAIN16_10BPP_RGGB;
      break;
    case MSM_V4L2_PIX_FMT_SBGGR14:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR;
      break;
    case MSM_V4L2_PIX_FMT_SGBRG14:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG;
      break;
    case MSM_V4L2_PIX_FMT_SGRBG14:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG;
      break;
    case MSM_V4L2_PIX_FMT_SRGGB14:
      fmt = CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB;
      break;

    case V4L2_PIX_FMT_NV12:
      fmt = CAM_FORMAT_YUV_420_NV12;
      break;
    case V4L2_PIX_FMT_NV16:
      fmt = CAM_FORMAT_YUV_422_NV16;
      break;
    case V4L2_PIX_FMT_NV61:
      fmt = CAM_FORMAT_YUV_422_NV61;
      break;
    case V4L2_MBUS_FMT_YUYV8_2X8:
      fmt = CAM_FORMAT_YUV_RAW_8BIT_YUYV;
      break;
    case V4L2_MBUS_FMT_YVYU8_2X8:
      fmt = CAM_FORMAT_YUV_RAW_8BIT_YVYU;
      break;
    case V4L2_MBUS_FMT_UYVY8_2X8:
      fmt = CAM_FORMAT_YUV_RAW_8BIT_UYVY;
      break;
    case V4L2_MBUS_FMT_VYUY8_2X8:
      fmt = CAM_FORMAT_YUV_RAW_8BIT_VYUY;
      break;
    case MSM_V4L2_PIX_FMT_META:
        fmt = CAM_FORMAT_META_RAW_8BIT;
      break;
    case MSM_V4L2_PIX_FMT_META10:
        fmt = CAM_FORMAT_META_RAW_10BIT;
      break;
    default:
    SERR("invalid fourcc format %d", pix_fmt_fourcc);
      fmt = CAM_FORMAT_MAX;
      break;
  }
  return fmt;
}

/** sensor_util_get_decode_fmt: Get the decode format from raw output format
 *
 *  @raw_output: raw output format mentioned in sensor driver
 *
 *  return the decode format
 **/
int8_t sensor_util_get_decode_fmt(sensor_raw_output_t raw_output)
{
  int8_t decode_format = -1;
  switch (raw_output)
  {
    case SENSOR_8_BIT_DIRECT:
      decode_format = CSI_DECODE_8BIT;
      break;
    case SENSOR_10_BIT_DIRECT:
      decode_format = CSI_DECODE_10BIT;
      break;
    case SENSOR_12_BIT_DIRECT:
      decode_format = CSI_DECODE_12BIT;
      break;
    case SENSOR_14_BIT_DIRECT:
      decode_format = CSI_DECODE_14BIT;
      break;
    default:
      break;
  }
  return decode_format;
}

sensor_pack_mode_t sensor_util_get_pack_mode(
  cam_format_t stream_format, cam_format_t sensor_format)
{
    int32_t pack_mode = SENSOR_PACK_MODE_NONE;

    SLOW("Stream format:%d, sensor format:%d", stream_format, sensor_format);
    if (stream_format != sensor_format) {
      switch (stream_format) {
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:

        switch (sensor_format) {
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
        case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
          pack_mode = SENSOR_PACK_MODE_NONE;
          break;
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
        case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
        case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
          pack_mode = SENSOR_PACK_MODE_PLAIN8;
          break;
        default:
          pack_mode = SENSOR_PACK_MODE_NONE;
          break;
        }
        break;

      default:
        break;
      }
    }
    return pack_mode;
}

/* This function will check if PLAIN packing is supported based on the CSID
  * hardware version provided. */
boolean sensor_util_is_plain_packing_supported(uint32_t csid_version) {
  if (csid_version >= 0x30050000)
    return TRUE;
  else
    return FALSE;
}
