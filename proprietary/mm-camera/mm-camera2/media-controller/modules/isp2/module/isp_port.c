/* isp_port.c
 *
 * Copyright (c) 2012-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
/* std headers */
#include <stdio.h>
#include <pthread.h>

/* mctl headers */
#include "media_controller.h"
#include "mct_profiler.h"

/* ISP headers*/
#include "isp_log.h"
#include "isp_defs.h"
#include "isp_port.h"
#include "isp_module.h"
#include "isp_util.h"
#include "isp_handler.h"
#include "camera_dbg.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COMMON, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COMMON, fmt, ##args)

#define ADDITIONAL_SINK_PORTS 3
#define ADDITIONAL_SRC_PORTS 3

/** isp_port_process_downstream_ctrl_event:
 *
 *  @module: module handle
 *  @port: port handle
 *  @event: event to be processed
 *
 *  Handle downstream ctrl event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_port_process_downstream_ctrl_event(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean              ret = TRUE;
  mct_event_control_t *ctrl_event = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  /* Extract ctrl event */
  ctrl_event = &event->u.ctrl_event;

  ISP_HIGH("event type %d", ctrl_event->type);
  switch (ctrl_event->type) {
  case MCT_EVENT_CONTROL_STREAMON:
    ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_ISP_STREAMON);
    ret = isp_handler_control_streamon(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_control_streamon");
    }
    ATRACE_CAMSCOPE_END(CAMSCOPE_ISP_STREAMON);
    break;

  case MCT_EVENT_CONTROL_STREAMOFF:
    ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_ISP_STREAMOFF);
    ret = isp_handler_control_streamoff(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_control_streamoff");
    }
    ATRACE_CAMSCOPE_END(CAMSCOPE_ISP_STREAMOFF);
    break;

  case MCT_EVENT_CONTROL_PARM_STREAM_BUF:
    ret = isp_handler_control_parm_stream_buf(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_control_parm_stream_buf");
    }
    break;

  case MCT_EVENT_CONTROL_SOF:
    MCT_PROF_LOG_BEG(PROF_ISP_SOF);
    ATRACE_BEGIN("ISP SOF");
    ret = isp_handler_control_sof(module, port, event);
    ATRACE_END();
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_control_sof");
    }
    MCT_PROF_LOG_END();
    break;

  case MCT_EVENT_CONTROL_OFFLINE_METADATA:
    ret = isp_handler_module_offline_metadata(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_module_offline_metadata");
    }
    break;

  case MCT_EVENT_CONTROL_SET_PARM: {
    ret = isp_handler_event(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_event");
      break;
    }

    ret = isp_handler_handle_hal_param(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_handle_hal_param");
    }
    break;
  }
  case MCT_EVENT_CONTROL_PREPARE_SNAPSHOT:
  case MCT_EVENT_CONTROL_START_ZSL_SNAPSHOT: {
    isp_handler_prepare_snapshot(module, port, event);
    ret = isp_util_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event");
    }
    break;
  }
  case MCT_EVENT_CONTROL_STOP_ZSL_SNAPSHOT:
  case MCT_EVENT_CONTROL_CANCEL_AF: {
    isp_handler_stop_zsl_snapshot(module, port, event);
    ret = isp_util_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event");
    }
    break;
  }
  case MCT_EVENT_CONTROL_HW_WAKEUP:
    isp_handler_hw_wake(module, port, event);
    ret = isp_util_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_forward_event");
    }
    break;


  case MCT_EVENT_CONTROL_SET_SUPER_PARM: {
    ret = isp_handler_event(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_event");
      break;
    }

    ret = isp_handler_handle_super_param(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_handle_hal_param");
    }
    break;
  }

  case MCT_EVENT_CONTROL_LINK_INTRA_SESSION: {
    ret = isp_handler_link_intra_session(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_link_intra_session");
    }
    ret = isp_util_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: LINK_INTRA_SESSION isp_util_forward_event");
    }
    break;
  }

  default:
    /* Forward event to downstream */
    ret = isp_handler_event(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_event");
    }
    break;
  }
  return ret;
}

/** isp_port_process_downstream_module_event:
 *
 *  @module: module handle
 *  @port: port handle
 *  @event: event to be processed
 *
 *  Handle downstream ctrl event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_port_process_downstream_module_event(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean              ret = TRUE;
  mct_event_module_t  *module_event = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: %p %p %p", module, port, event);
    return FALSE;
  }

  /* Extract ctrl event */
  module_event = &event->u.module_event;

  ISP_HIGH("event type %d", module_event->type);
  switch (module_event->type) {
  case MCT_EVENT_MODULE_REG_UPDATE_NOTIFY:
    ret = isp_handler_module_handle_reg_update(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_module_handle_reg_update");
    }
    break;

  case MCT_EVENT_MODULE_SET_STREAM_CONFIG:
    ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_ISP_SET_STRM_CFG);
    ret = isp_handler_module_set_stream_config(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_module_set_stream_config");
    }
    ATRACE_CAMSCOPE_END(CAMSCOPE_ISP_SET_STRM_CFG);
    break;

  case MCT_EVENT_MODULE_ISP_OFFLINE_PIPELINE_CONFIG: {
    ret = isp_handler_module_offline_pipeline_config(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_module_set_stream_config");
    }
  }
    break;

  case MCT_EVENT_MODULE_ISP_OFFLINE_INPUT_CONFIG: {
    isp_session_param_t *session_param = NULL;
    sensor_set_dim_t *offline_input_cfg = NULL;
    ret = isp_util_get_session_params(module,
      ISP_GET_SESSION_ID(event->identity), &session_param);
    if (ret == FALSE || !session_param) {
      ISP_ERR("failed: Setting offline input cfg ret %d %p", ret,
        session_param);
      break;
    }

    offline_input_cfg = (sensor_set_dim_t *)event->u.module_event.module_event_data;
    if (!offline_input_cfg) {
       ISP_ERR("failed: etting offline input cfg %p", offline_input_cfg);
       break;
    }

    session_param->offline_input_cfg.dim_output.height =
      offline_input_cfg->dim_output.height;
    session_param->offline_input_cfg.dim_output.width =
      offline_input_cfg->dim_output.width;
    session_param->offline_input_cfg.output_format =
      offline_input_cfg->output_format;
    ISP_DBG("Offline Input Cfg width %d height %d fmt %d",
      offline_input_cfg->dim_output.height, offline_input_cfg->dim_output.height,
      offline_input_cfg->output_format);
  }
    break;

  case MCT_EVENT_MODULE_SET_CHROMATIX_PTR: {
    isp_session_param_t *session_param = NULL;
    ret = isp_util_get_session_params(module,
      ISP_GET_SESSION_ID(event->identity), &session_param);
    if (ret == FALSE || !session_param) {
      ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p", ret,
        session_param);
      break;
    }
    if (session_param->state == ISP_STATE_STREAMING) {
      ret = isp_handler_event(module, port, event);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_handler_event");
      }
    }
    else {
      ret = isp_handler_module_set_chromatix_ptr(module, port, event);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_handler_module_set_chromatix_ptr");
      }
    }
  }
    break;

  case MCT_EVENT_MODULE_BUF_DIVERT:
    ISP_HIGH("MCT_EVENT_MODULE_BUF_DIVERT");
    /* Forward event downstream on frame port */
    ret = isp_util_forward_event_downstream_to_type(module, event,
      MCT_PORT_CAPS_FRAME);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_BUF_DIVERT");
    }
    break;

  case MCT_EVENT_MODULE_ISP_DIVERT_TO_3A: {
    ISP_HIGH("MCT_EVENT_MODULE_BUF_DIVERT_TO_3A");
    /* Forward event downstream on frame port */
    ret = isp_util_forward_event_downstream_to_type(module, event,
      MCT_PORT_CAPS_STATS);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_BUF_DIVERT_TO_3A");
    }
  }
    break;

  case MCT_EVENT_MODULE_IFACE_REQUEST_OUTPUT_RESOURCE:
    ret = isp_handler_module_iface_request_output_resource(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_control_streamoff");
    }
    isp_handler_module_send_initial_crop_info(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_module_set_stream_config");
    }
    break;

  case MCT_EVENT_MODULE_IFACE_REQUEST_OFFLINE_OUTPUT_RESOURCE:
    ret = isp_handler_module_iface_request_offline_output_resource(module, port,
        event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_control_streamoff");
    }
    break;

  case MCT_EVENT_MODULE_IFACE_REQUEST_STREAM_MAPPING_INFO:
    ret = isp_handler_module_iface_request_stream_mapping_info(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: iface_request_stream_mapping_info");
    }
    break;

  case MCT_EVENT_MODULE_STATS_GET_DATA:
    /* Forward event downstream on frame port */
    ret = isp_util_forward_event_downstream_to_type(module, event,
      MCT_PORT_CAPS_STATS);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_STATS_GET_DATA");
    }
    break;

  case MCT_EVENT_MODULE_RAW_STATS_DIVERT:
    ret = isp_handler_raw_stats_divert(module, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_raw_stats_notify");
    }
    break;

  case MCT_EVENT_MODULE_SET_FAST_AEC_CONVERGE_MODE: {
    mct_fast_aec_mode_t *fast_aec =
      (mct_fast_aec_mode_t*)event->u.module_event.module_event_data;
    ISP_HIGH("FASTAEC identity = 0x%x enable= %d",
      event->identity, fast_aec->enable);
    ret = isp_util_set_fast_aec_mode(module, port,
      event, fast_aec);
    }
    break;

  case MCT_EVENT_MODULE_SOF_NOTIFY: {
    isp_session_param_t *session_param = NULL;
    ret = isp_util_get_session_params(module,
      ISP_GET_SESSION_ID(event->identity), &session_param);
    if (ret == FALSE || !session_param) {
      ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p", ret,
        session_param);
      break;
    }

    ret = isp_handler_sof(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_sof");
    }
  }
    break;

  case MCT_EVENT_MODULE_IFACE_REQUEST_PP_DIVERT:
    ret = isp_handler_request_pp_divert(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_request_pp_divert");
    }
    break;

  case MCT_EVENT_MODULE_SET_SENSOR_OUTPUT_INFO:
    ret = isp_handler_sensor_output_dim(module, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_raw_stats_notify");
    }
    break;
  case MCT_EVENT_MODULE_FE_SOF:
    ret = isp_handler_module_fe_sof(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_module_fe_sof");
    }
    break;

  default:
    /* Forward event to downstream */
    ret = isp_handler_event(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_event");
    }
    break;
  }
  return ret;
}

/** isp_port_process_downstream_event:
 *
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Handle downstream event
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_port_process_downstream_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean       ret = TRUE;
  mct_module_t *module = NULL;

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

  if (event->type == MCT_EVENT_CONTROL_CMD) {
    ret = isp_port_process_downstream_ctrl_event(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_port_process_downstream_ctrl_event");
    }
  } else {
    ret = isp_port_process_downstream_module_event(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_port_process_downstream_module_event");
    }
  }
  return ret;
}

/** isp_port_process_upstream_event:
 *
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Handle downstream event
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_port_process_upstream_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean              ret = TRUE;
  mct_module_t        *module = NULL;
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

  module_event = &event->u.module_event;

  ISP_HIGH("event type %d", module_event->type);
  switch (module_event->type) {
  case MCT_EVENT_MODULE_FACE_INFO:
  case MCT_EVENT_MODULE_PPROC_GET_AWB_UPDATE:
  case MCT_EVENT_MODULE_PPROC_GET_AEC_UPDATE:
    /* Change event direction to downstream and forward to stats port */
    ret = isp_util_forward_event_downstream_to_type(module, event,
      MCT_PORT_CAPS_STATS);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_divert_event_downstream_to_type type %d",
        module_event->type);
    }
    break;

  case MCT_EVENT_MODULE_ISP_PRIVATE_EVENT:
    /* Do NOT propagate this event to other modules */
    break;

  case MCT_EVENT_MODULE_STATS_DATA_ACK:
    ret = isp_handler_stats_data_ack(module, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_stats_data_ack");
    }
    break;

  case MCT_EVENT_MODULE_SET_PREFERRED_STREAMS_MAPPING:
    ret = isp_util_set_preferred_mapping(module, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_util_set_preferred_mapping type %d",
        module_event->type);
    }
    break;

  default:
    /* Handle event */
    ret = isp_handler_event(module, port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_handler_enqueue_event_to_trigger_queue");
    }
    break;
  }

  return ret;
}

/** isp_port_event:
 *
 *  @port: port handle
 *  @event: event to be processed
 *
 *  Handle ISP event based on direction
 *
 *  Returns TRUE on success and FALSE on failure */
static boolean isp_port_event(mct_port_t *port, mct_event_t *event)
{
  boolean ret = TRUE;
  mct_port_t            *int_hw_port = NULL;
  mct_list_t            *l_identity = NULL;
  isp_session_param_t   *session_param = NULL;
  mct_module_t *module = NULL;

  if (!port || !event) {
    ISP_ERR("failed: port %p event %p", port, event);
    return FALSE;
  }

  if ((MCT_PORT_IS_SINK(port) != TRUE) && (MCT_PORT_IS_SRC(port) != TRUE)) {
    ISP_ERR("failed: invalid port direction %d", MCT_PORT_DIRECTION(port));
    return FALSE;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  if ((event->direction != MCT_EVENT_DOWNSTREAM) &&
      (event->direction != MCT_EVENT_UPSTREAM)) {
    ISP_ERR("failed: invalid event dir %d", event->direction);
    return FALSE;
  }

  if (event->direction == MCT_EVENT_DOWNSTREAM) {
    ret = isp_port_process_downstream_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_port_process_downstream_event");
    }
  } else {
    ret = isp_port_process_upstream_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_port_process_upstream_event");
    }
  }

  return ret;
}

/** isp_port_ext_link:
 *
 *  @identity: identity of stream
 *  @port: port handle
 *  @peer: peer handle
 *
 *  Handle ext link
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_port_ext_link(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  boolean               ret = FALSE;
  isp_port_data_t      *port_data = NULL;
  mct_module_t         *module = NULL;
  isp_session_param_t  *session_param = NULL;
  isp_stream_param_t   *stream_param = NULL;
  isp_module_ports_t   *isp_ports = NULL;
  uint32_t              port_num;

  if (!port || !peer) {
    ISP_ERR("failed: port %p peer %p", port, peer);
    return FALSE;
  }

  ISP_HIGH("ide %x port dir %d", identity, port->direction);
  /* Unlock this mutex in every return path */
  //PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(port));

  if (!MCT_PORT_PEER(port)) {
    MCT_PORT_PEER(port) = peer;
  } else { /*the link has already been established*/
    if ((MCT_PORT_PEER(port) != peer))
    goto ERROR;
  }

  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    goto ERROR;
  }

  ret = isp_util_get_session_stream_params(module, identity, &session_param,
    &stream_param);
  if (ret == FALSE || !session_param || !stream_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p %p", ret,
      session_param, stream_param);
    goto ERROR;
  }

  isp_ports = &session_param->isp_ports;

  if (MCT_PORT_IS_SRC(port) == TRUE) {
    /* Update internal links of source port */
    if (port->caps.port_caps_type == MCT_PORT_CAPS_STATS) {
      if (isp_ports->isp_source_stats_port &&
         (isp_ports->isp_source_stats_port != port)) {
        ISP_ERR("failed: isp_source_stats_port %p != port %p",
          isp_ports->isp_source_stats_port, port);
      }
      isp_ports->isp_source_stats_port = port;
    } else if (port->caps.port_caps_type == MCT_PORT_CAPS_FRAME) {
      port_data = (isp_port_data_t *)MCT_OBJECT_PRIVATE(port);
      if (!port_data) {
        ISP_ERR("failed: source_port_data %p", port_data);
        goto ERROR;
      }
      if (isp_ports->isp_source_frame_port[port_data->hw_stream] &&
         (isp_ports->isp_source_frame_port[port_data->hw_stream] != port)) {
        ISP_ERR("failed: hw stream %d isp_source_frame_port %p != port %p",
          port_data->hw_stream,
          isp_ports->isp_source_frame_port[port_data->hw_stream], port);
      }
      isp_ports->isp_source_frame_port[port_data->hw_stream] = port;
    }
  } else if (MCT_PORT_IS_SINK(port) == TRUE) {
    /* Update internal links of sink port */
    if (isp_ports->isp_sink_port && (isp_ports->isp_sink_port != port)) {
      ISP_ERR("failed: isp_sink_port %p != port %p",
        isp_ports->isp_sink_port, port);
    }
    isp_ports->isp_sink_port = port;
  } else {
    ISP_ERR("failed: invalid port type %d", MCT_PORT_DIRECTION(port));
    goto ERROR;
  }

  //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  return TRUE;

ERROR:
  ISP_ERR("failed: isp_port_ext_link");
  //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  return FALSE;
}

/** isp_port_unlink:
 *
 *  @identity: identity of stream
 *  @port: port handle
 *  @peer: peer handle
 *
 *  Handle unlink func
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static void isp_port_unlink(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  boolean               ret = TRUE;
  mct_module_t         *module = NULL;
  isp_session_param_t  *session_param = NULL;
  isp_port_data_t      *port_data = NULL;
  isp_module_ports_t   *isp_ports = NULL;

  if (!port || !peer) {
    ISP_ERR("failed: port %p peer %p", port, peer);
    return;
  }

  ISP_HIGH("ide %x port dir %d", identity, port->direction);
  /* Unlock this mutex in every return path */
  //PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(port));

  if (MCT_PORT_PEER(port) != peer) {
    ISP_ERR("failed");
    //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
    return;
  }

  /*
   * Check whether there is only on identity in children of this port
   * if so, this is the only identity that is linked to current port,
   * reset int link
   */
  if (MCT_PORT_NUM_CHILDREN(port) == 1) {
    module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
    if (!module) {
      ISP_ERR("failed: module %p", module);
      goto ERROR;
    }

    ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(identity),
      &session_param);
    if (ret == FALSE || !session_param) {
      ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
        session_param);
      goto ERROR;
    }

    isp_ports = &session_param->isp_ports;

    if (MCT_PORT_IS_SRC(port) == TRUE) {
      if (port->caps.port_caps_type == MCT_PORT_CAPS_STATS) {
        isp_ports->isp_source_stats_port = NULL;
      } else if (port->caps.port_caps_type == MCT_PORT_CAPS_FRAME) {
        port_data = (isp_port_data_t *)MCT_OBJECT_PRIVATE(port);
        if (!port_data) {
          ISP_ERR("failed: source_port_data %p", port_data);
          goto ERROR;
        }
        if (port_data->is_session_based == FALSE &&
            port_data->hw_stream >= ISP_HW_STREAM_MAX) {
          ISP_ERR("failed: hw_stream %d", port_data->hw_stream);
          goto ERROR;
        }
        isp_ports->isp_source_frame_port[port_data->hw_stream] = NULL;
      }
    } else if (MCT_PORT_IS_SINK(port) == TRUE) {
      isp_ports->isp_sink_port = NULL;
    } else {
      ISP_ERR("failed: invalid port type %d", MCT_PORT_DIRECTION(port));
      goto ERROR;
    }
  }

  //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  return;

ERROR:
  ISP_ERR("failed: isp_port_unlink");
  //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  return;
}

/** isp_port_reserve_src_port
 *
 *  @port: mct port handle
 *  @port_data: port private data
 *  @mct_stream_info: stream info handle
 *
 *  Reserve source port for isp
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_port_reserve_src_port(mct_port_t *port,
  isp_port_data_t *port_data, mct_stream_info_t *mct_stream_info)
{
  boolean           ret = TRUE;
  isp_hw_streamid_t hw_stream;
  isp_hw_id_t       hw_id;

  if (!port || !port_data || !mct_stream_info) {
    ISP_ERR("failed: invalid params %p %p %p", port, port_data,
      mct_stream_info);
    return FALSE;
  }

  if (port->caps.port_caps_type == MCT_PORT_CAPS_STATS) {
    return TRUE;
  } else {
    /* Find which hw stream is assigned to this stream */
    ret = isp_handler_get_hw_params(port, mct_stream_info, &hw_stream, &hw_id);
    if (ret == FALSE) {
      ISP_ERR("failed");
      return FALSE;
    }
    /* For offline proc stream use different port */
    if (mct_stream_info->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC &&
      port_data->is_reserved)
      return FALSE;

    /* Check whether this port is compatible with the hw stream */
    if (hw_stream == port_data->hw_stream && hw_id == port_data->hw_id) {
      return TRUE;
    }
  }

  return FALSE;
}

/** isp_port_check_caps_reserve:
 *
 *  @port: port handle
 *  @peer_caps: peer caps handle
 *  @stream_info: handle to stream_info
 *
 *  Handle caps reserve
 *
 *  Return TRUE if this port is reserved, FALSE otherwise
 **/
static boolean isp_port_check_caps_reserve(mct_port_t *port,
  void *peer_caps, void *stream_info)
{
  boolean            ret = FALSE, stream_mapping_done = FALSE;
  mct_stream_info_t *mct_stream_info = (mct_stream_info_t *)stream_info;
  isp_port_data_t   *port_data = NULL;
  uint32_t           session_id = 0;

  if (!port || !stream_info) {
    ISP_ERR("failed: port %p stream_info %p", port, stream_info);
    return FALSE;
  }

  ISP_HIGH("port dir %d stream info ide %x type %d dim %d x %d port %p",
    port->direction, mct_stream_info->identity, mct_stream_info->stream_type,
    mct_stream_info->dim.width, mct_stream_info->dim.height, port);
  if ((MCT_PORT_IS_SINK(port) != TRUE) && (MCT_PORT_IS_SRC(port) != TRUE)) {
    ISP_ERR("failed: invalid port type %d", MCT_PORT_DIRECTION(port));
    return FALSE;
  }

  if (MCT_PORT_IS_SINK(port) == TRUE) {
    ISP_INFO("port %p ide %x type %d dim %d %d", port, mct_stream_info->identity,
    mct_stream_info->stream_type, mct_stream_info->dim.width,
    mct_stream_info->dim.height);
  }

  port_data = (isp_port_data_t *)MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    return FALSE;
  }

  session_id = ISP_GET_SESSION_ID(mct_stream_info->identity);

  /* Unlock this mutex in every return path */
  //PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(port));

  /* Check whether this port is already reserved for different session */
  if ((port_data->is_reserved == TRUE) &&
      (port_data->session_id != session_id)) {
    //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
    return FALSE;
  }

  ret = isp_handler_create_stream_param(port, peer_caps, mct_stream_info);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_port_create_stream_param");
    return FALSE;
  }

  if (MCT_PORT_IS_SINK(port) == TRUE) {
    if (mct_stream_info->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC &&
      mct_stream_info->stream_type != CAM_STREAM_TYPE_RAW) {
      ret = isp_handler_create_internal_link(port, stream_info);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_handler_create_internal_link");
        //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
        goto ERROR;
      }
    }
  } else {
    /* Check hw_stream match only for frame port, not for stats port */
    if (port->caps.port_caps_type == MCT_PORT_CAPS_FRAME) {
      ret = isp_port_reserve_src_port(port, port_data, mct_stream_info);
      if (ret == FALSE) {
        //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
        goto ERROR;
      }
    }
  }

  port_data->is_reserved = TRUE;
  port_data->session_id = session_id;
  port_data->num_streams++;
  if (mct_stream_info->stream_type == CAM_STREAM_TYPE_PARM) {
    port_data->is_session_based = TRUE;
  }
  //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));

  return TRUE;

ERROR:
  isp_handler_destroy_stream_param(port, mct_stream_info->identity);
  return FALSE;
}

/** isp_port_check_caps_unreserve:
 *
 *  @port: port handle
 *  @identity: identity of stream
 *
 *  Handle caps unreserve
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  boolean                   ret = TRUE;
  isp_port_data_t          *port_data = NULL;
  uint32_t                  isp_id = 0;
  mct_module_t             *module = NULL;
  isp_session_param_t      *session_param = NULL;
  isp_stream_param_t       *stream_param = NULL;
  mct_list_t               *l_identity = NULL;
  isp_util_identity_list_t  identity_list;

  if (!port) {
    ISP_ERR("failed: port %p", port);
    return FALSE;
  }

  if (MCT_PORT_IS_SINK(port) == TRUE) {
    ISP_INFO("ide %x port %p", identity, port);
  }
  port_data = (isp_port_data_t *)MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    return FALSE;
  }

  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("module %p", module);
    return FALSE;
  }

  isp_util_get_session_stream_param_from_type(module, CAM_STREAM_TYPE_PARM,
      &session_param, &stream_param, ISP_GET_SESSION_ID(identity));

  //PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(port));
  if (MCT_PORT_IS_SINK(port) == TRUE) {
    if (!stream_param || stream_param->stream_info.identity != identity) {
        isp_resource_destroy_internal_link(port, identity);
    } else if (session_param->num_isp) {
      /* session based stream unlinking wihtout deallocate resources,
       * "CAM_INTF_META_STREAM_INFO with 0 streams" is not received,
       * deallocate resources
       */
      ISP_DBG("ISP session resource not dellocated in time");
      ret = isp_util_unlink_session_stream(module, session_param);
       if (ret == FALSE) {
         ISP_ERR("failed: isp_util_link_session_stream");
       }

       /* clean up stream mapping */
       session_param->stream_port_map.num_streams = 0;
    }
  }

  /* At this point we should clear only data filled by reserve i.e. session,
   * is_reserved and is_session based and not data filled by stream mapping like
   * hw_stream and hw_id(currently not used). The latter are cleared when ports
   * are unreserved */
  if (!(--port_data->num_streams)) {
    port_data->session_id = 0;
    MCT_PORT_PEER(port) = NULL;
    port_data->is_reserved = FALSE;
    port_data->is_session_based = FALSE;
  }

  isp_handler_destroy_stream_param(port, identity);

  //PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  return TRUE;
}

/** isp_port_delete_port:
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
static boolean isp_port_delete_port(void *data, void *user_data)
{
  boolean          ret = TRUE;
  mct_module_t    *module = (mct_module_t *)user_data;
  mct_port_t      *port = (mct_port_t *)data;
  isp_port_data_t *isp_port_data = NULL;

  if (!data || !user_data) {
    ISP_ERR("failed: data %p user_data %p", data, user_data);
    return TRUE;
  }

  isp_port_data = (isp_port_data_t *)MCT_OBJECT_PRIVATE(port);
  if (isp_port_data) {
   free(isp_port_data);
  }
#if 1
  ret = mct_module_remove_port(module, port);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_module_remove_port");
  }
#endif
  mct_port_destroy(port);
  return TRUE;
}

/** isp_port_return_first
 *
 *  @data1: handle to list data
 *  @data2: private data
 *
 *  Return TRUE if first element in list is present
 **/
static boolean isp_port_return_first(
    void *data1,
    void *data2 __unused)
{
  if (data1) {
    return TRUE;
  }
  return FALSE;
}

/** isp_port_delete_ports:
 *
 *  @module: isp module handle
 *
 *  Delete port and its private data
 *
 *  Returns: void
 **/
void isp_port_delete_ports(mct_module_t *module)
{
  mct_list_t *l_port = NULL;
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }
  do {
    l_port = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), module,
      isp_port_return_first);
    if (l_port)
      isp_port_delete_port(l_port->data, module);
  } while (l_port);
  do {
    l_port = mct_list_find_custom(MCT_MODULE_SRCPORTS(module), module,
      isp_port_return_first);
    if (l_port)
      isp_port_delete_port(l_port->data, module);
  } while (l_port);
}

/** isp_port_overwrite_funcs
 *
 *   @port: mct port instance
 *   @private_data: port private
 *
 *  Assign mct port function pointers to respective isp port
 *  functions
 *
 *  Returns nothing
 **/
static void isp_port_overwrite_funcs(mct_port_t *port, void *private_data)
{
  if (!port) {
    ISP_ERR("failed: port %p", port);
    return;
  }

  mct_port_set_event_func(port, isp_port_event);
  mct_port_set_ext_link_func(port, isp_port_ext_link);
  mct_port_set_unlink_func(port, isp_port_unlink);
  mct_port_set_check_caps_reserve_func(port, isp_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, isp_port_check_caps_unreserve);
  MCT_OBJECT_PRIVATE(port) = private_data;
}

/** isp_port_create_port
 *
 *  @module: mct module handle
 *  @direction: direction of port
 *  @caps_type: caps type
 *  @port_id: port id
 *  @hw_id: ISP id
 *  @hw_stream_id: HW stream id
 *
 *  Creates port and add it to parent
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_port_create_port(mct_module_t *module,
  mct_port_direction_t direction, mct_port_caps_type_t caps_type,
  uint32_t port_id, isp_hw_id_t hw_id, isp_hw_streamid_t hw_stream_id)
{
  boolean          ret = TRUE;
  isp_port_data_t *isp_port_data = NULL;
  char             port_name[32];
  mct_port_t      *mct_port = NULL;

  if (!module ||
    ((direction != MCT_PORT_SINK) && (direction != MCT_PORT_SRC)) ||
    ((caps_type != MCT_PORT_CAPS_STATS) &&
     (caps_type != MCT_PORT_CAPS_FRAME))) {
    ISP_ERR("failed: module %p direction %d caps_type %d",
      module, direction, caps_type);
    return FALSE;
  }

  isp_port_data = (isp_port_data_t *)CAM_MALLOC(sizeof(isp_port_data_t));
  if (!isp_port_data) {
    ISP_ERR("failed: CAM_MALLOC");
    return FALSE;
  }
  memset(isp_port_data, 0, sizeof(*isp_port_data));
  isp_port_data->hw_stream = hw_stream_id;
  isp_port_data->hw_id = hw_id;

  if (direction == MCT_PORT_SINK) {
    snprintf(port_name, sizeof(port_name), "isp_sink%d", port_id);
  } else if (direction == MCT_PORT_SRC) {
    snprintf(port_name, sizeof(port_name), "isp_src%d", port_id);
  }

  mct_port = mct_port_create(port_name);
  if (!mct_port) {
    ISP_ERR("failed: mct_port_create");
    ret = FALSE;
    goto ERROR_PORT_CREATE;
  }

  mct_port->direction = direction;
  ret = mct_module_add_port(module, mct_port);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_module_add_port");
    goto ERROR_ADD_PORT;
  }

  mct_port->caps.port_caps_type = caps_type;
  /* Initialize ISP port format flag to typical value */
  mct_port->caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR;
  isp_port_overwrite_funcs(mct_port, (void *)isp_port_data);
  ISP_DBG("<port_dbg> name %s port %p dir %d (1 - SRC / 2 - SINK)"
    "type %d (1 - stats / 2 - frame) *********", port_name, mct_port,
    mct_port->direction, mct_port->caps.port_caps_type);
  return ret;

ERROR_ADD_PORT:
  mct_port_destroy(mct_port);
ERROR_PORT_CREATE:
  free(isp_port_data);
  return ret;
}

/** isp_port_create:
 *
 *  @module: module handle
 *
 *  1) Create one sink port per ISP
 *
 *  2) Create one source port for STATS per ISP
 *
 *  3) Create one source port for FRAME per ISP
 **/
boolean isp_port_create(mct_module_t *module)
{
  boolean  ret = TRUE;
  uint32_t hw_id = 0, hw_stream = 0, src_port_cnt = 0, i;
  isp_submod_hw_streams_info_t hw_streams;
  isp_t   *isp = NULL;

  if (!module) {
    ISP_ERR("failed: invalid params %p", module);
    return FALSE;
  }

  isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed: isp %p", isp);
    return FALSE;
  }
  src_port_cnt = 0;
  for (i = 0; i < (isp->isp_resource.num_isp + ADDITIONAL_SINK_PORTS); ++i) {
    /* Create sink port */
    ret = isp_port_create_port(module, MCT_PORT_SINK, MCT_PORT_CAPS_FRAME,
      i, hw_id, ISP_HW_STREAM_MAX);
    if (ret == FALSE) {
      ISP_ERR("failed: hw_id %d isp_port_create_port", hw_id);
      goto ERROR;
    }
  }

  for (hw_id = 0; hw_id < isp->isp_resource.num_isp; hw_id++) {
    ret = isp_resource_get_hw_streams(&isp->isp_resource, &hw_streams, hw_id);
    if (ret == FALSE) {
       ISP_ERR("failed");
       return FALSE;
    }

    for (hw_stream = 0; hw_stream < hw_streams.num_hw_streams; hw_stream++) {
      /* Create source port for FRAME */
      ret = isp_port_create_port(module, MCT_PORT_SRC, MCT_PORT_CAPS_FRAME,
        src_port_cnt++, hw_id, hw_stream);
      if (ret == FALSE) {
        ISP_ERR("failed: hw_id %d isp_port_create_port", hw_id);
        goto ERROR;
      }
    }

    /* Create source port for offline ISP reprocessing */
    ret = isp_port_create_port(module, MCT_PORT_SRC, MCT_PORT_CAPS_FRAME,
      src_port_cnt++, ISP_HW_MAX, ISP_HW_STREAM_OFFLINE);
    if (ret == FALSE) {
      ISP_ERR("failed: hw_id %d isp_port_create_port", hw_id);
      goto ERROR;
    }

    for (i = 0; i < ADDITIONAL_SRC_PORTS; i++) {
      ret = isp_port_create_port(module, MCT_PORT_SRC, MCT_PORT_CAPS_FRAME,
        src_port_cnt++, ISP_HW_MAX, ISP_HW_STREAM_MAX);
      if (ret == FALSE) {
        ISP_ERR("failed: hw_id %d isp_port_create_port", hw_id);
        goto ERROR;
      }
    }
  }
    /* Create source port for STATS */
    for (hw_id = 0; hw_id < isp->isp_resource.num_isp + ADDITIONAL_SINK_PORTS; hw_id++) {
    ret = isp_port_create_port(module, MCT_PORT_SRC, MCT_PORT_CAPS_STATS,
      src_port_cnt++, hw_id, ISP_HW_STREAM_MAX);
    if (ret == FALSE) {
      ISP_ERR("failed: hw_id %d isp_port_create_port", hw_id);
      goto ERROR;
    }
  }

  return ret;

ERROR:
  ISP_ERR("failed: isp_port_create");
  isp_port_delete_ports(module);
  return ret;
}
