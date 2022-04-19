/* port_scaler40.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <stdio.h>
#include <pthread.h>

/* mctl headers */
#include "media_controller.h"
#include "mct_stream.h"

/* ISP headers*/
#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "isp_defs.h"
#include "module_scaler40.h"
#include "port_scaler40.h"
#include "scaler40.h"
#include "scaler40_util.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_SCALER, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_SCALER, fmt, ##args)

/**  port_scaler40_handle_set_stream_config:
 *
 *  @data1: mct stream info handle
 *  @data2: streaming mode mask
 *
 *  Append streaming mode of stream info in streaming mode
 *  mask
 *
 *  Return TRUE
 **/
static boolean port_scaler40_handle_set_stream_config(scaler40_t *scaler,
  mct_event_t *event)
{
  boolean ret = TRUE;
  sensor_out_info_t   *sensor_out_info = NULL;
  ISP_HIGH("E: scaler: %p event: %p", scaler, event);
  if (!scaler || !event) {
    ISP_ERR("failed: %p %p", scaler, event);
    return FALSE;
  }

  sensor_out_info = event->u.module_event.module_event_data;
  if (!sensor_out_info) {
    ISP_ERR("failed: sensor_out_info %p", sensor_out_info);
    return FALSE;
  }

  scaler->sensor_out_info = *sensor_out_info;
  scaler->hw_stream_info.streaming_mode= scaler->hw_stream_info.streaming_mode;

  if (scaler->sensor_out_info.dim_output.width <
     (uint32_t)scaler->hw_stream_info.width) {
    if (scaler->dis_enable) {
      ISP_ERR("sensor output width is smaller than"
              " request size and DIS needs extra margin");
      return FALSE;
    } else {
      scaler->hw_stream_info.width =
        scaler->sensor_out_info.dim_output.width;
    }
  }
  if (scaler->sensor_out_info.dim_output.height <
     (uint32_t)scaler->hw_stream_info.height) {
    if (scaler->dis_enable) {
      ISP_ERR("sensor output height is smaller than"
              " request size and DIS needs extra margin");
      return FALSE;
    } else {
      scaler->hw_stream_info.height =
        scaler->sensor_out_info.dim_output.height;
    }
  }

  /* Update the sensor type and CAMIF dimension */
  scaler->is_bayer_sensor = isp_sub_module_is_bayer_mono_fmt(
    scaler->sensor_out_info.fmt);
  if (scaler->is_bayer_sensor) {
    scaler->camif_dim.width = sensor_out_info->request_crop.last_pixel -
      sensor_out_info->request_crop.first_pixel + 1;
  } else {
    scaler->camif_dim.width = (sensor_out_info->request_crop.last_pixel -
      sensor_out_info->request_crop.first_pixel + 1) >> 1;
  }

  scaler->camif_dim.height = sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1;

  /* Update IFACE if ISP is capable of doing further downscaling for CDS */
  if (((scaler->hw_stream_info.width/2 * ISP_SCALER40_MAX_SCALER_FACTOR) <
    scaler->camif_dim.width) ||
    ((scaler->hw_stream_info.height/2 * ISP_SCALER40_MAX_SCALER_FACTOR) <
    scaler->camif_dim.height)) {
      scaler->cds_cap[scaler->entry_idx] = FALSE;
      ISP_ERR("path %d, CDS disable\n", scaler->entry_idx);
    } else
    scaler->cds_cap[scaler->entry_idx] = TRUE;

  scaler->hw_stream_info.need_uv_subsample = 0;

  scaler40_calculate_dis(scaler);

  ret = scaler40_config(scaler);
  if (ret == FALSE) {
    ISP_ERR("failed: ret == FALSE");
    return FALSE;
  }

  ISP_DBG("X: ret = TRUE");
  return ret;
}

/** port_scaler40_update_streaming_mode:
 *
 *  @data1: mct stream info handle
 *  @data2: streaming mode mask
 *
 *  Append streaming mode of stream info in streaming mode
 *  mask
 *
 *  Return TRUE
 **/
static boolean port_scaler40_update_streaming_mode(void *data1, void *data2)
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

/** port_scaler40_handle_streamon:
 *
 *  @scaler: scaler handle
 *  @event: event to be handled
 *
 *  Handle STREAM ON event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_scaler40_handle_streamon(scaler40_t *scaler,
  mct_event_t *event)
{
  boolean ret = TRUE;

  if (!scaler || !event) {
    ISP_ERR("failed: scaler %p event %p", scaler, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&scaler->mutex);
  /* Update streaming mode mask in module private */
  mct_list_traverse(scaler->l_stream_info,
    port_scaler40_update_streaming_mode, &scaler->streaming_mode_mask);

  PTHREAD_MUTEX_UNLOCK(&scaler->mutex);
  return ret;
}

/** port_scaler40_handle_streamoff:
 *
 *  @scaler: scaler handle
 *  @event: event to be handled
 *  Handle STREAM OFF event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_scaler40_handle_streamoff(scaler40_t *scaler,
  mct_event_t *event)
{
  boolean       ret = TRUE;

  if (!scaler || !event) {
    ISP_ERR("failed: scaler %p event %p", scaler, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&scaler->mutex);
  /* Reset streaming mode */
  ISP_RESET_STREAMING_MODE_MASK(scaler->streaming_mode_mask);

  PTHREAD_MUTEX_UNLOCK(&scaler->mutex);
  return ret;
}

/**  port_scaler40_handle_hal_set_parm:
 *
 *  @scaler: scaler module private
 *  @event: mct event
 *
 *  Append streaming mode of stream info in streaming mode
 *  mask
 *
 *  Return TRUE
 **/
static boolean port_scaler40_handle_hal_set_parm(scaler40_t *scaler,
  mct_event_control_parm_t *hal_param)
{
  boolean ret = TRUE;

  if (!scaler || !hal_param) {
    ISP_ERR("failed: %p %p", scaler, hal_param);
    return FALSE;
  }

  switch (hal_param->type) {
  case CAM_INTF_PARM_DIS_ENABLE: {
    int32_t *dis_enable;
    dis_enable = (int32_t *)hal_param->parm_data;
    scaler->dis_enable = *dis_enable;
  }
    break;

  case CAM_INTF_PARM_USERZOOM: {
    ret = scaler40_parm_zoom(scaler, (cam_zoom_info_t *)hal_param->parm_data);
    if (ret == FALSE) {
      ISP_ERR("failed: scaler40_parm_zoom");
    }
  }
    break;
  default:
    break;
  }

  ISP_DBG("X: ret = TRUE");
  return ret;
}

static boolean port_scaler40_handle_set_super_parm(scaler40_t *scaler,
  mct_event_t *event)
{
  boolean                         ret = TRUE,
                                  func_ret = TRUE;
  uint32_t                        i = 0;
  mct_event_super_control_parm_t *super_param = NULL;

  if (!scaler || !event) {
    ISP_ERR("failed: scaler %p event %p", scaler, event);
    return FALSE;
  }

  super_param =
    (mct_event_super_control_parm_t *)event->u.ctrl_event.control_event_data;
  if (!super_param) {
    ISP_ERR("failed: super_param %p", super_param);
    return FALSE;
  }

  for (i = 0; i < super_param->num_of_parm_events; i++) {
    ret = port_scaler40_handle_hal_set_parm(scaler,
      &super_param->parm_events[i]);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler40_handle_hal_set_parm type %d",
        super_param->parm_events[i].type);
      func_ret = FALSE;
    }
  }
  return func_ret;
}

/** port_scaler40_forward_event:
 *
 *  @port: port handle
 *  @event: event to be forwarded
 *
 *  Forward event based on port direction and event direction
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_scaler40_forward_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean                 ret = TRUE;
  port_scaler40_data_t *port_data = NULL;

  if (!port || !event) {
    ISP_ERR("failed: port %p event %p", port, event);
    return FALSE;
  }

  if (((event->direction == MCT_EVENT_UPSTREAM) &&
       (MCT_PORT_DIRECTION(port) != MCT_PORT_SRC)) ||
      ((event->direction == MCT_EVENT_DOWNSTREAM) &&
       (MCT_PORT_DIRECTION(port) != MCT_PORT_SINK))) {
    ISP_ERR("failed: invalid types, event dir %d port dir %d",
      event->direction, MCT_PORT_DIRECTION(port));
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

/** port_scaler40_process_downstream_ctrl_event:
 *
 *  @module: module handle
 *  @port: port handle
 *  @scaler: scaler handle
 *  @event: event to be processed
 *
 *  Handle downstream ctrl event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_scaler40_process_downstream_ctrl_event(
  mct_module_t *module, mct_port_t *port, scaler40_t *scaler,
  mct_event_t *event)
{
  boolean              ret = TRUE;
  mct_event_control_t *ctrl_event = NULL;

  if (!module || !port || !scaler || !event) {
    ISP_ERR("failed: %p %p %p %p", module, port, scaler, event);
    return FALSE;
  }

  /* Extract ctrl event */
  ctrl_event = &event->u.ctrl_event;

  ISP_HIGH("ide %x event %d mod %s", event->identity,
     ctrl_event->type, MCT_MODULE_NAME(module));
  switch (ctrl_event->type) {
  case MCT_EVENT_CONTROL_SET_PARM:
    ret = port_scaler40_handle_hal_set_parm(scaler,
      event->u.ctrl_event.control_event_data);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler40_handle_streamon");
    }
    /* per stats module request, for this event,
     * ISP needs to change the direction and
     * redirect it to downstream */
    /* Forward event to FRAME port */
    ret = port_scaler40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;

  case MCT_EVENT_CONTROL_SET_SUPER_PARM:
    ret = port_scaler40_handle_set_super_parm(scaler, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler40_handle_set_super_parm");
    }
    ret = port_scaler40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;

#if 0
  case MCT_EVENT_CONTROL_STREAMON:
    ret = port_scaler40_handle_streamon(scaler, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler40_handle_streamon");
    }

    /* Forward event to FRAME port */
    ret = port_scaler40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;

  case MCT_EVENT_CONTROL_STREAMOFF:
    ret = port_scaler40_handle_streamoff(scaler, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler40_handle_streamon");
    }

    /* Forward event to FRAME port */
    ret = port_scaler40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;
#endif

  default:
    /* Forward event to FRAME port */
    ret = port_scaler40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;
  }
  return ret;
}

/** port_scaler40_process_downstream_module_event:
 *
 *  @module: module handle
 *  @port: port handle
 *  @scaler: scaler handle
 *  @event: event to be processed
 *
 *  Handle downstream ctrl event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_scaler40_process_downstream_module_event(
  mct_module_t *module, mct_port_t *port, scaler40_t *scaler,
  mct_event_t *event)
{
  boolean              ret = TRUE;
  mct_event_module_t  *module_event = NULL;

  if (!module || !port || !scaler || !event) {
    ISP_ERR("failed: %p %p %p %p", module, port, scaler, event);
    return FALSE;
  }

  /* Extract ctrl event */
  module_event = &event->u.module_event;

  ISP_HIGH("ide %x event %d mod %s", event->identity,
     module_event->type, MCT_MODULE_NAME(module));
  switch (module_event->type) {
  case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
    ret = port_scaler40_handle_set_stream_config(scaler, event);
    if (ret == FALSE) {
      ISP_ERR("failed: scaler40_handle_set_stream_config");
    }
    /* Forward event */
    ret = port_scaler40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
  }
    break;

  case MCT_EVENT_MODULE_ISP_PRIVATE_EVENT:
    ret = scaler40_handle_isp_private_event(scaler,
      module_event->module_event_data, event->identity);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_ISP_PRIVATE_EVENT");
    }
    /* Forward event */
    ret = port_scaler40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;

  default:
    /* Forward event */
    ret = port_scaler40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;
  }
  return ret;
}

/** port_scaler40_process_downstream_event:
 *
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Handle downstream event
 *
 *  Returns TRUE on success and FALSE on failure
 **/
boolean port_scaler40_process_downstream_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean       ret = TRUE;
  mct_module_t *module = NULL;
  scaler40_t *scaler = NULL;

  if (!port || !event) {
    ISP_ERR("failed: port %p event %p", port, event);
    return FALSE;
  }

  if ((event->type != MCT_EVENT_CONTROL_CMD) &&
      (event->type != MCT_EVENT_MODULE_EVENT)) {
    ISP_ERR("failed: invalid event type %d", event->type);
    return FALSE;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  /* Extract module private */
  scaler = (scaler40_t *)MCT_OBJECT_PRIVATE(module);
  if (!scaler) {
    ISP_ERR("failed: scaler %p", scaler);
    return FALSE;
  }

  if (event->type == MCT_EVENT_CONTROL_CMD) {
    ret = port_scaler40_process_downstream_ctrl_event(module, port, scaler,
      event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler40_process_downstream_ctrl_event");
    }
  } else if (event->type == MCT_EVENT_MODULE_EVENT) {
    ret = port_scaler40_process_downstream_module_event(module, port,
      scaler, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler40_process_downstream_module_event");
    }
  }
  return ret;
}

/** port_scaler40_process_upstream_event:
 *
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Handle downstream event
 *
 *  Returns TRUE on success and FALSE on failure
 **/
boolean port_scaler40_process_upstream_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean              ret = TRUE;
  mct_module_t        *module = NULL;
  scaler40_t        *scaler = NULL;
  mct_event_module_t  *module_event = NULL;

  if (!port || !event) {
    ISP_ERR("failed: port %p event %p", port, event);
    return FALSE;
  }

  if (event->type != MCT_EVENT_MODULE_EVENT) {
    ISP_ERR("failed: invalid event type %d", event->type);
    return FALSE;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  /* Extract module private */
  scaler = MCT_OBJECT_PRIVATE(module);
  if (!scaler) {
    ISP_ERR("failed: scaler %p", scaler);
    return FALSE;
  }

  module_event = &event->u.module_event;
  ISP_HIGH("ide %x event %d mod %s", event->identity,
     module_event->type, MCT_MODULE_NAME(module));

  switch (module_event->type) {
  case MCT_EVENT_MODULE_ISP_PRIVATE_EVENT: {
    ret = scaler40_handle_isp_private_event(scaler,
      module_event->module_event_data, event->identity);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_ISP_PRIVATE_EVENT");
    }
  }
    break;

  default:
    /* Forward event to upstream FRAME port */
    ret = port_scaler40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;
  }

  return ret;
}

/** port_scaler40_event:
 *
 *  @port: port handle
 *  @event: event to be processed
 *
 *  Handle ISP event based on direction
 *
 *  Returns TRUE on success and FALSE on failure */
static boolean port_scaler40_event(mct_port_t *port, mct_event_t *event)
{
  boolean       ret = TRUE;
  mct_module_t *module = NULL;
  scaler40_t *scaler = NULL;

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

  scaler = MCT_OBJECT_PRIVATE(module);
  if (!scaler) {
    ISP_ERR("failed: scaler %p", scaler);
    return FALSE;
  }

  if (scaler->session_id != ISP_GET_SESSION_ID(event->identity)) {
    return FALSE;
  }

  if (event->direction == MCT_EVENT_DOWNSTREAM) {
    ret = port_scaler40_process_downstream_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler40_process_downstream_event");
    }
  } else {
    ret = port_scaler40_process_upstream_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler40_process_upstream_event");
    }
  }

  return ret;
}

/** port_scaler40_ext_link:
 *
 *  @identity: identity of stream
 *  @port: port handle
 *  @peer: peer handle
 *
 *  Handle ext link
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean port_scaler40_ext_link(unsigned int identity, mct_port_t* port,
  mct_port_t *peer)
{
  boolean                 ret = TRUE;
  mct_module_t           *module = NULL;
  mct_port_t             *sink_port = NULL;
  port_scaler40_data_t *port_data = NULL;
  port_scaler40_data_t *port_sink_data = NULL;

  if (!port || !peer) {
    ISP_ERR("failed: port %p peer %p", port, peer);
    return FALSE;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(port));

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
    ret = scaler40_util_get_port_from_module(
      (mct_module_t *)(MCT_PORT_PARENT(port)->data), &sink_port, MCT_PORT_SINK,
      identity);
    if ((ret == FALSE) || !sink_port) {
      ISP_ERR("failed: scaler40_util_get_port_from_module ret %d port %p",
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
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  ISP_HIGH("port %p dir %d parent %p %s", port, MCT_PORT_DIRECTION(port),
    (MCT_PORT_PARENT(port)->data),
    MCT_MODULE_NAME(MCT_PORT_PARENT(port)->data));
  ISP_HIGH("ide %x", identity);

  return TRUE;

ERROR:
  ISP_ERR("failed: port_scaler40_ext_link");
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  return FALSE;
}

/** port_scaler40_unlink:
 *
 *  @identity: identity of stream
 *  @port: port handle
 *  @peer: peer handle
 *
 *  Handle unlink func
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static void port_scaler40_unlink(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  boolean                 ret = TRUE;
  mct_port_t             *sink_port = NULL;
  port_scaler40_data_t *port_data = NULL;
  port_scaler40_data_t *port_sink_data = NULL;

  if (!port || !peer) {
    ISP_ERR("failed: port %p peer %p", port, peer);
    return;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(port));

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
    port_data->int_peer_port = NULL;
#if 0
    if (MCT_PORT_IS_SRC(port) == TRUE) {
      /* Find sink port with same identity */
      ret = scaler40_util_get_port_from_module(
        (mct_module_t *)(MCT_PORT_PARENT(port)->data), &sink_port,
        MCT_PORT_SINK, identity);
      if ((ret == FALSE) || !sink_port) {
        ISP_ERR("failed: scaler40_util_get_port_from_module ret %d port %p",
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
#endif
  }

ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  ISP_HIGH("port %p dir %d parent %p %s", port, MCT_PORT_DIRECTION(port),
    (MCT_PORT_PARENT(port)->data),
    MCT_MODULE_NAME(MCT_PORT_PARENT(port)->data));

  ISP_HIGH("ide %x", identity);
  return;
}

/** port_scaler40_check_caps_reserve:
 *
 *  @port: port handle
 *  @peer_caps: peer caps handle
 *  @stream_info: handle to stream_info
 *
 *  Handle caps reserve
 *
 *  Return TRUE if this port is reserved, FALSE otherwise
 **/
static boolean port_scaler40_check_caps_reserve(mct_port_t *port,
  void *peer_caps, void *stream_info)
{
  boolean                 ret = FALSE;
  mct_stream_info_t      *mct_stream_info = (mct_stream_info_t *)stream_info;
  port_scaler40_data_t *port_data = NULL;
  uint32_t                session_id = 0;
  mct_module_t           *module = NULL;
  scaler40_t             *scaler = NULL;
  uint32_t                i = 0;

  if (!port || !stream_info) {
    ISP_ERR("failed: port %p stream_info %p", port, stream_info);
    return FALSE;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(port));

  port_data = MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    goto ERROR;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    goto ERROR;
  }

  scaler = (scaler40_t *)MCT_OBJECT_PRIVATE(module);
  if (!scaler) {
    ISP_ERR("failed: scaler %p", scaler);
    goto ERROR;
  }
  session_id = ISP_GET_SESSION_ID(mct_stream_info->identity);

  /* Check whether this port is already reserved for same session */
  if ((port_data->is_reserved == TRUE) &&
      (port_data->session_id != session_id)) {
    ISP_ERR("port %p assigned for different session %d", port,
      port_data->session_id);
    goto ERROR;
  }

  ret = scaler40_util_append_stream_info(port, mct_stream_info);
  if (ret == FALSE) {
    ISP_ERR("failed: scaler40_util_append_stream_info");
    goto ERROR;
  }

  port_data->is_reserved = TRUE;
  port_data->session_id = session_id;

  if (mct_stream_info->stream_type == CAM_STREAM_TYPE_PARM) {
    scaler->session_based_identity = mct_stream_info->identity;
  }

  if (port->direction == MCT_PORT_SINK) {
    /* Store identity */
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (!scaler->identity[i]) {
        scaler->identity[i] = mct_stream_info->identity;
        break;
      }
    }
  }

  scaler->update_enable_bit = TRUE;
  scaler->stream_type = mct_stream_info->stream_type;

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));

  ISP_HIGH("port %p dir %d parent %p %s", port, MCT_PORT_DIRECTION(port),
    (MCT_PORT_PARENT(port)->data),
    MCT_MODULE_NAME(MCT_PORT_PARENT(port)->data));
  ISP_HIGH("stream ide %x type %d", mct_stream_info->identity,
    mct_stream_info->stream_type);

  return TRUE;
ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  return FALSE;
}

/** port_scaler40_check_caps_unreserve:
 *
 *  @port: port handle
 *  @identity: identity of stream
 *
 *  Handle caps unreserve
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_scaler40_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  boolean                   ret = TRUE;
  port_scaler40_data_t   *port_data = NULL;
  mct_module_t           *module = NULL;
  scaler40_t             *scaler = NULL;
  uint32_t                i = 0;

  if (!port) {
    ISP_ERR("failed: port %p", port);
    return FALSE;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(port));

  port_data = MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    goto ERROR;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    goto ERROR;
  }

  scaler = (scaler40_t *)MCT_OBJECT_PRIVATE(module);
  if (!scaler) {
    ISP_ERR("failed: scaler %p", scaler);
    goto ERROR;
  }

  ret = scaler40_util_remove_stream_info(port, identity);
  if (ret == FALSE) {
    ISP_ERR("failed: scaler40_util_append_stream_info");
  }

  if (!port_data->num_streams) {
    port_data->session_id = 0;
    MCT_PORT_PEER(port) = NULL;
    port_data->is_reserved = FALSE;
    scaler->session_based_identity = 0x0;
  }

  if (port->direction == MCT_PORT_SRC) {
    /* Remove identity */
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (scaler->identity[i] == identity) {
        scaler->identity[i] = 0x0;
        break;
      }
    }
  }

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  ISP_HIGH("add port %p dir %d parent %p", port, MCT_PORT_DIRECTION(port),
    (MCT_PORT_PARENT(port)->data));
  ISP_HIGH("stream ide %x", identity);

  return TRUE;
ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  return FALSE;
}

/** port_scaler40_delete_port:
 *
 *  @data: handle to port to be deleted
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
static boolean port_scaler40_delete_port(void *data, void *user_data)
{
  boolean          ret = TRUE;
  mct_module_t    *module = (mct_module_t *)user_data;
  mct_port_t      *port = (mct_port_t *)data;
  port_scaler40_data_t *port_scaler40_data = NULL;

  if (!data || !user_data) {
    ISP_ERR("failed: data %p user_data %p", data, user_data);
    return TRUE;
  }

  port_scaler40_data = MCT_OBJECT_PRIVATE(port);
  if (port_scaler40_data) {
   free(port_scaler40_data);
  }

  ret = mct_module_remove_port(module, port);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_module_remove_port");
  }

  mct_port_destroy(port);

  return TRUE;
}

/** port_scaler40_delete_ports:
 *
 *  @port: port handle
 *
 *  Delete port and its private data
 *
 *  Returns: void
 **/
void port_scaler40_delete_ports(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  mct_list_traverse(module->sinkports, port_scaler40_delete_port, module);
  mct_list_traverse(module->srcports, port_scaler40_delete_port, module);
}

/** port_scaler40_overwrite_funcs
 *
 *   @port: mct port instance
 *   @private_data: port private
 *
 *  Assign mct port function pointers to respective scaler port
 *  functions
 *
 *  Returns nothing
 **/
static void port_scaler40_overwrite_funcs(mct_port_t *port,
  void *private_data)
{
  if (!port) {
    ISP_ERR("failed: port %p", port);
    return;
  }

  mct_port_set_event_func(port, port_scaler40_event);
  mct_port_set_ext_link_func(port, port_scaler40_ext_link);
  mct_port_set_unlink_func(port, port_scaler40_unlink);
  mct_port_set_check_caps_reserve_func(port,
    port_scaler40_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port,
    port_scaler40_check_caps_unreserve);
  MCT_OBJECT_PRIVATE(port) = private_data;
}

/** port_scaler40_create_port
 *
 *   @scaler: scaler instance
 *   @direction: direction of port
 *
 *  Creates port and add it to parent
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean port_scaler40_create_port(mct_module_t *module,
  mct_port_direction_t direction, mct_port_caps_type_t caps_type)
{
  boolean          ret = TRUE;
  port_scaler40_data_t *port_scaler40_data = NULL;
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

  port_scaler40_data =
    (port_scaler40_data_t *)CAM_MALLOC(sizeof(port_scaler40_data_t));
  if (!port_scaler40_data) {
    ISP_ERR("failed: CAM_MALLOC");
    return FALSE;
  }
  memset(port_scaler40_data, 0, sizeof(*port_scaler40_data));

  if (direction == MCT_PORT_SINK) {
    snprintf(port_name, sizeof(port_name), "scaler40_sink");
  } else if (direction == MCT_PORT_SRC) {
    snprintf(port_name, sizeof(port_name), "scaler40_src");
  }

  mct_port = mct_port_create(port_name);
  if (!mct_port) {
    ISP_ERR("failed: mct_port_create");
    ret = FALSE;
    goto ERROR;
  }

  MCT_PORT_DIRECTION(mct_port) = direction;
  ret = mct_module_add_port(module, mct_port);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_module_add_port");
    goto ERROR;
  }

  ISP_HIGH("add port %p dir %d mod %p parent %p", mct_port, direction, module,
    (MCT_PORT_PARENT(mct_port)->data));
  mct_port->caps.port_caps_type = caps_type;
  port_scaler40_overwrite_funcs(mct_port, (void *)port_scaler40_data);

  return ret;
ERROR:
  free(port_scaler40_data);
  return ret;
}

/** port_scaler40_create:
 *
 *  @module: module handle
 *
 *  1) Create one sink port per ISP
 *
 *  2) Create one source port for STATS
 *
 *  3) Create one source port for FRAME
 **/
boolean port_scaler40_create(mct_module_t *module)
{
  boolean  ret = TRUE;

  if (!module) {
    ISP_ERR("failed: invalid params %p", module);
    return FALSE;
  }

  /* Create sink port */
  ret = port_scaler40_create_port(module, MCT_PORT_SINK, MCT_PORT_CAPS_FRAME);
  if (ret == FALSE) {
    ISP_ERR("failed: port_scaler40_create_port");
    goto ERROR;
  }

  /* Create source port */
  ret = port_scaler40_create_port(module, MCT_PORT_SRC, MCT_PORT_CAPS_FRAME);
  if (ret == FALSE) {
    ISP_ERR("failed: port_scaler40_create_port");
    goto ERROR;
  }

  return ret;

ERROR:
  ISP_ERR("failed: port_scaler40_create");
  port_scaler40_delete_ports(module);
  return ret;
}
