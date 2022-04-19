/* q3a_port.c
 *
 * Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/

#ifdef _ANDROID_
#pragma GCC diagnostic ignored "-Wswitch"
#endif

#include "q3a_port.h"
#include "aec_port.h"
#include "awb_port.h"
#include "af_port.h"
#include "q3a_thread.h"
#include "stats_module.h"

#include "aec.h"
#include "awb.h"
#include "af.h"

#include <libxml/parser.h>

/* To preserve the aec and af state after TAF till scene does not change */
#define TAF_SAVE_AEC_AF 0

#define IS_Q3A_PORT_IDENTITY(port_private, identity) \
  ((port_private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000))

/* Time to wait after LED estimation sequence (AEC+AF)
 * is complete - in ms*/
#define Q3A_PORT_LED_WAIT_TIME 3000

/* Definitions of local functions */
static boolean q3a_port_process_event_for_led_af(mct_port_t *port,
  mct_event_t *event);

static void q3a_port_led_af_reset_state_flag(q3a_port_private_t *private)
{
  if (private == NULL)
    return;
  pthread_mutex_lock(&private->af_led_data.state_lock);
  if (private->af_led_data.state == AF_WITH_LED_STATE_AF_DONE) {
    private->af_led_data.state = AF_WITH_LED_STATE_IDLE;
  }
  pthread_mutex_unlock(&private->af_led_data.state_lock);

  return;
}

/*
 * Determine if aec is in manual mode
 */
boolean q3a_port_is_aec_manual_mode(mct_port_t *port)
{
  if (port == NULL) {
    Q3A_ERR("NULL port");
    return FALSE;
  }

  q3a_port_private_t *private = port->port_private;
  if (private->q3a_meta_mode == CAM_CONTROL_OFF ||
       (private->q3a_meta_mode == CAM_CONTROL_AUTO &&
         private->q3a_enable_aec == FALSE)) {
    return TRUE;
  }

  return FALSE;
}

boolean q3a_port_map_sensor_format(
  q3a_sensor_type_t *q3a_sensor_type,
  mct_sensor_format_t cam_sensor_format)
{
  switch (cam_sensor_format) {
  case  FORMAT_BAYER:
    *q3a_sensor_type = Q3A_SENSOR_BAYER;
    break;
  case FORMAT_YCBCR:
    *q3a_sensor_type = Q3A_SENSOR_YUV;
    break;
  case FORMAT_GREY:
    *q3a_sensor_type = Q3A_SENSOR_MONO;
    break;
  }

  Q3A_HIGH("sensor format: cam: %d, q3a: %d",
    cam_sensor_format, *q3a_sensor_type);
  return TRUE;
}

boolean q3a_port_map_lens_type(
  q3a_lens_type_t *q3a_lens_type,
  cam_lens_type_t cam_lens_type)
{
  switch (cam_lens_type) {
  case CAM_LENS_NORMAL:
    *q3a_lens_type = Q3A_LENS_NORMAL;
    break;
  case CAM_LENS_WIDE:
    *q3a_lens_type = Q3A_LENS_WIDE;
    break;
  case CAM_LENS_TELE:
    *q3a_lens_type = Q3A_LENS_TELE;
    break;
  }

  Q3A_HIGH("lens type: cam: %d, q3a: %d",
    cam_lens_type, *q3a_lens_type);
  return TRUE;
}

boolean q3a_port_map_perf_type(
  q3a_dual_camera_perf_mode_t *q3a_perf_type,
  cam_dual_camera_perf_mode_t cam_perf_type)
{
  switch (cam_perf_type) {
  case CAM_PERF_NONE:
    *q3a_perf_type = Q3A_PERF_NONE;
    break;
  case CAM_PERF_SENSOR_SUSPEND:
    *q3a_perf_type = Q3A_PERF_SENSOR_SUSPEND;
    break;
  case CAM_PERF_ISPIF_FRAME_DROP:
    *q3a_perf_type = Q3A_PERF_ISPIF_FRAME_DROP;
    break;
  case CAM_PERF_ISPIF_FRAME_SKIP:
    *q3a_perf_type = Q3A_PERF_ISPIF_FRAME_SKIP;
    break;
  case CAM_PERF_STATS_FPS_CONTROL:
    *q3a_perf_type = Q3A_PERF_STATS_FPS_CONTROL;
    break;
  }

  Q3A_HIGH("perf type: cam: %d, q3a: %d", cam_perf_type, *q3a_perf_type);
  return TRUE;
}


/** q3a_port_start_threads
 *    @port:     module's port
 *    @identity: stream/session identity
 *    @type:     the type of the thread - AF or AECAWB
 *
 *  This function will start the thread handlers of the q3a subports.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean q3a_port_start_threads(mct_port_t *port,
  unsigned int identity, q3a_thread_type_t type)
{
  mct_event_t        event;
  q3a_port_private_t *private = port->port_private;
  uint8_t camera_id = 0; /* default */

  switch (type) {
  case Q3A_THREAD_AECAWB: {
    q3a_thread_aecawb_get_port_data_t aecawb_get_data;
    q3a_thread_aecawb_set_port_data_t aecawb_set_data;
    STATS_MEMSET(&aecawb_get_data, 0, sizeof(q3a_thread_aecawb_get_port_data_t));
    STATS_MEMSET(&aecawb_set_data, 0, sizeof(q3a_thread_aecawb_set_port_data_t));


    if (private->aecawb_data != NULL) {
      event.type      = MCT_EVENT_MODULE_EVENT;
      event.identity  = identity;
      event.direction = MCT_EVENT_DOWNSTREAM;

      event.u.module_event.type = MCT_EVENT_MODULE_STATS_AECAWB_GET_THREAD_OBJECT;
      event.u.module_event.module_event_data = (void *)&aecawb_get_data;

      if (MCT_PORT_EVENT_FUNC(private->aec_port)(private->aec_port, &event) ==
        FALSE ||
        MCT_PORT_EVENT_FUNC(private->awb_port)(private->awb_port, &event) ==
        FALSE) {
        Q3A_ERR("NOT Start AECAWB thread");
        q3a_thread_aecawb_deinit(private->aecawb_data);
        return FALSE;
      }
      /* Save requested values */
      private->aecawb_data->camera[camera_id] = aecawb_get_data.camera;

      /* Set thread data in port */
      aecawb_set_data.awbaec_thread_data = private->aecawb_data->thread_data;

      event.u.module_event.type = MCT_EVENT_MODULE_STATS_AECAWB_SET_THREAD_OBJECT;
      event.u.module_event.module_event_data = (void *)&aecawb_set_data;
      if (MCT_PORT_EVENT_FUNC(private->aec_port)(private->aec_port, &event) ==
        FALSE ||
        MCT_PORT_EVENT_FUNC(private->awb_port)(private->awb_port, &event) ==
        FALSE) {
        Q3A_ERR("NOT Start AECAWB thread");
        q3a_thread_aecawb_deinit(private->aecawb_data);
        return FALSE;
      }

      Q3A_LOW(" Start AEAWB thread");
      q3a_thread_aecawb_start(private->aecawb_data, aecawb_thread_handler,
        "CAM_AECAWB");
    }
  } /* case Q3A_THREAD_AECAWB */
    break;

  case Q3A_THREAD_AF: {
    q3a_thread_af_get_port_data_t af_get_data;
    q3a_thread_af_set_port_data_t af_set_data;
    STATS_MEMSET(&af_get_data, 0, sizeof(q3a_thread_af_get_port_data_t));
    STATS_MEMSET(&af_set_data, 0, sizeof(q3a_thread_af_set_port_data_t));

    if (private->af_data != NULL) {
      event.type       = MCT_EVENT_MODULE_EVENT;
      event.identity   = identity;
      event.direction  = MCT_EVENT_DOWNSTREAM;

      event.u.module_event.type = MCT_EVENT_MODULE_STATS_AF_GET_THREAD_OBJECT;
      event.u.module_event.module_event_data = (void *)&af_get_data;

      if (MCT_PORT_EVENT_FUNC(private->af_port)(private->af_port, &event) ==
        FALSE) {
        Q3A_ERR("AF event handler returned FALSE");
        q3a_thread_af_deinit(private->af_data);
        return FALSE;
      }
      /* Save requested values */
      private->af_data->camera[camera_id] = af_get_data.camera;

      /* Set thread data in port */
      af_set_data.af_thread_data = private->af_data->thread_data;

      event.u.module_event.type = MCT_EVENT_MODULE_STATS_AF_SET_THREAD_OBJECT;
      event.u.module_event.module_event_data = (void *)&af_set_data;

      if (MCT_PORT_EVENT_FUNC(private->af_port)(private->af_port, &event) ==
        FALSE) {
        Q3A_ERR("AF event handler returned FALSE");
        q3a_thread_af_deinit(private->af_data);
        return FALSE;
      }

      Q3A_LOW(" Start AF thread");
      q3a_thread_af_start(private->af_data, af_thread_handler, "CAM_AF");
    }
  } /* case Q3A_THREAD_AF */
    break;

  default: {
    Q3A_ERR("Invalid start thread type");
    return FALSE;
  } /* default */
    break;
  } /* switch (type) */

  return TRUE;
} /* q3a_port_start_threads */

/** q3a_port_send_event_to_aec_port
  *    @port:  module's port
  *    @event: the event object to be sent downstream
  *
  *  This function will send an event to the AEC port.
  *
  *  Return TRUE on success, FALSE on failure.
  **/
static boolean q3a_port_send_event_to_aec_port(mct_port_t *port,
  mct_event_t *event)
{
  boolean rc = FALSE;
  q3a_port_private_t *private = port->port_private;

  if (MCT_PORT_EVENT_FUNC(private->aec_port)) {
    rc = MCT_PORT_EVENT_FUNC(private->aec_port)(private->aec_port, event);
    if (rc == FALSE) {
      Q3A_ERR("AEC event handler returned failure!");
    }
  }
  return rc;
}

static boolean q3a_port_send_event_to_awb_port(mct_port_t *port,
  mct_event_t *event)
{
  boolean rc = FALSE;
  q3a_port_private_t *private = port->port_private;

  if (MCT_PORT_EVENT_FUNC(private->awb_port)) {
    rc = MCT_PORT_EVENT_FUNC(private->awb_port)(private->awb_port, event);
    if (rc == FALSE) {
      Q3A_ERR("AWB event handler returned failure!");
    }
  }
  return rc;
}


/** q3a_port_send_event_to_af_port
 *    @event: the event object to be sent downstream
 *
 *  This function will send an event to the AF port.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean q3a_port_send_event_to_af_port(mct_port_t *port,
  mct_event_t *event)
{
  boolean rc = FALSE;
  q3a_port_private_t *private = port->port_private;

  if (MCT_PORT_EVENT_FUNC(private->af_port)) {
    rc = MCT_PORT_EVENT_FUNC(private->af_port)(private->af_port, event);
    if (rc == FALSE) {
      Q3A_ERR("AF event handler returned failure!");
    }
  }
  return rc;
}

/** q3a_port_send_event_downstream
 *    @port:  this is be the module's port
 *    @event: the event object to be sent downstream
 *
 *  This function will send the received event downstream to the
 *  q3a submodules (AF, AWB and AEC).
 *
 *  Return TRUE on success, FALSE on event handler failure.
 **/
static boolean q3a_port_send_event_downstream(mct_port_t *port,
  mct_event_t *event)
{
  boolean rc = FALSE;
  q3a_port_private_t *private = port->port_private;

  if (MCT_PORT_EVENT_FUNC(private->aec_port)) {
    rc = MCT_PORT_EVENT_FUNC(private->aec_port)(private->aec_port, event);
    if (rc == FALSE) {
      Q3A_ERR("AEC event handler returned failure!");
      goto send_done;
    }
  }

  if (MCT_PORT_EVENT_FUNC(private->awb_port)) {
    rc = MCT_PORT_EVENT_FUNC(private->awb_port)(private->awb_port, event);
    if (rc == FALSE) {
      Q3A_ERR("AWB event handler returned failure!");
      goto send_done;
    }
  }

  if (MCT_PORT_EVENT_FUNC(private->af_port)) {
    rc = MCT_PORT_EVENT_FUNC(private->af_port)(private->af_port, event);
    if (rc == FALSE) {
      Q3A_ERR("AF event handler returned failure!");
      goto send_done;
    }
  }

send_done:
  return rc;
} /* q3a_port_send_event_downstream */

/** q3a_port_proc_upstream_mod_event
 *    @port:  this is be the module's port
 *    @event: the event object to be sent upstream
 *
 *  This function will send the received event upstream and will propagate
 *  it to the other q3a submodules.
 *
 *  Return TRUE on success, FALSE on event handler failure.
 **/
static boolean q3a_port_proc_upstream_mod_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean            rc = TRUE;
  q3a_port_private_t *private = port->port_private;
  mct_event_stats_t  *stats =
    (mct_event_stats_t *)(event->u.module_event.module_event_data);

  /* Always send UPDATE or REQUEST upstream first */
    mct_port_send_event_to_peer(port, event);

  /* Check to see if need to redirect this event to sub-ports */
  event->direction = MCT_EVENT_DOWNSTREAM;
  switch (event->u.module_event.type) {
  case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
    if (MCT_PORT_EVENT_FUNC(private->awb_port)) {
      MCT_PORT_EVENT_FUNC(private->awb_port)(private->awb_port, event);
    }

    if (MCT_PORT_EVENT_FUNC(private->af_port)) {
      MCT_PORT_EVENT_FUNC(private->af_port)(private->af_port, event);
    }
  }
    break;

  case MCT_EVENT_MODULE_STATS_AEC_MANUAL_UPDATE: {
    if (MCT_PORT_EVENT_FUNC(private->af_port)) {
      MCT_PORT_EVENT_FUNC(private->af_port)(private->af_port, event);
    }
  }
    break;

  case MCT_EVENT_MODULE_FAST_AEC_CONVERGE_ACK: {
    if (MCT_PORT_EVENT_FUNC(private->awb_port)) {
      MCT_PORT_EVENT_FUNC(private->awb_port)(private->awb_port, event);
    }

    if (MCT_PORT_EVENT_FUNC(private->af_port)) {
      MCT_PORT_EVENT_FUNC(private->af_port)(private->af_port, event);
    }
  }
    break;

  case MCT_EVENT_MODULE_STATS_AWB_UPDATE: {
    if (MCT_PORT_EVENT_FUNC(private->aec_port)) {
      MCT_PORT_EVENT_FUNC(private->aec_port)(private->aec_port, event);
    }

    if (MCT_PORT_EVENT_FUNC(private->af_port)) {
      MCT_PORT_EVENT_FUNC(private->af_port)(private->af_port, event);
    }
  }
    break;

  case MCT_EVENT_MODULE_STATS_AF_UPDATE: {
    if (MCT_PORT_EVENT_FUNC(private->aec_port)) {
      MCT_PORT_EVENT_FUNC(private->aec_port)(private->aec_port, event);
    }

    if (MCT_PORT_EVENT_FUNC(private->awb_port)) {
      MCT_PORT_EVENT_FUNC(private->awb_port)(private->awb_port, event);
    }
  }
    break;

  case MCT_EVENT_MODULE_STATS_AECAWB_GET_THREAD_OBJECT:
  case MCT_EVENT_MODULE_STATS_AECAWB_SET_THREAD_OBJECT:
  case MCT_EVENT_MODULE_STATS_AECAWB_RESTORE_EXT_THREAD_OBJECT: {
    boolean aec_rc = FALSE;
    boolean awb_rc = FALSE;
    if (MCT_PORT_EVENT_FUNC(private->aec_port)) {
      aec_rc = MCT_PORT_EVENT_FUNC(private->aec_port)(private->aec_port, event);
    }

    if (MCT_PORT_EVENT_FUNC(private->awb_port)) {
      awb_rc = MCT_PORT_EVENT_FUNC(private->awb_port)(private->awb_port, event);
    }
    rc = aec_rc && awb_rc;
  }
    break;

  case MCT_EVENT_MODULE_STATS_AF_GET_THREAD_OBJECT:
  case MCT_EVENT_MODULE_STATS_AF_SET_THREAD_OBJECT:
  case MCT_EVENT_MODULE_STATS_AF_RESTORE_EXT_THREAD_OBJECT: {
    if (MCT_PORT_EVENT_FUNC(private->af_port)) {
      rc = MCT_PORT_EVENT_FUNC(private->af_port)(private->af_port, event);
    }
  }
    break;

  default: {
    rc = FALSE;
  }
    break;
  }

  return rc;
} /* q3a_port_proc_upstream_mod_event */

/** q3a_port_is_led_needed
 *    @port:  this is the module's port
 *
 *  This function will check if the LED is needed to assist the AF
 *
 *  Return TRUE if LED is needed, FALSE if not.
 **/
static boolean q3a_port_is_led_needed(mct_port_t *port)
{
  q3a_port_private_t *private = port->port_private;
  return private->af_led_data.led_needed;
}

/** q3a_port_is_led_needed_for_af_mode
 *    @af_mode:  current af mode
 *
 *  This function will check if the LED is needed to assist the AF
 *
 *  Return TRUE if LED is needed, FALSE if not.
 **/
static boolean q3a_port_is_led_needed_for_af_mode(
  q3a_port_private_t *private, q3a_port_af_led_t *af_led_data) {
  boolean rc = TRUE;

  pthread_mutex_lock(&private->af_led_data.state_lock);
  if ((af_led_data->af_mode == CAM_FOCUS_MODE_CONTINOUS_VIDEO ||
      af_led_data->capture_intent == CAM_INTENT_VIDEO_RECORD) ||
      af_led_data->af_mode ==  CAM_FOCUS_MODE_FIXED ) {
    Q3A_LOW(" Skip LED AF sequence");
    rc = FALSE;
  }
  pthread_mutex_unlock(&private->af_led_data.state_lock);

  return rc;
}


/** q3a_port_update_led_af_to_sensor
  *   @port:  this is the module's port
  *   @type:  this is the led af type
  *
  * This function will send the led af type to sensor
  *
  *  Return TRUE on success, FALSE on failure.
 **/
static boolean q3a_port_update_led_af_to_sensor(mct_port_t *port, boolean flag)
{
  boolean            rc = TRUE;
  q3a_port_private_t *private = port->port_private;
  mct_event_t        q3a_event;
  int                led_af_flag = flag;

  q3a_event.direction           = MCT_EVENT_UPSTREAM;
  q3a_event.identity            = private->reserved_id;
  q3a_event.type                = MCT_EVENT_MODULE_EVENT;

  q3a_event.u.module_event.type = MCT_EVENT_MODULE_LED_AF_UPDATE;
  q3a_event.u.module_event.current_frame_id = private->cur_sof_id;
  q3a_event.u.module_event.module_event_data = &led_af_flag;

  rc = mct_port_send_event_to_peer(port, &q3a_event);

  return rc;
}

static boolean q3a_port_update_fast_aec_converge_ack(mct_port_t *port)
{
  boolean              rc = TRUE;
  q3a_port_private_t   *private = port->port_private;
  mct_event_t          q3a_event;

  q3a_event.direction       = MCT_EVENT_UPSTREAM;
  q3a_event.identity        = private->reserved_id;
  q3a_event.type            = MCT_EVENT_MODULE_EVENT;
  q3a_event.u.module_event.type = MCT_EVENT_MODULE_FAST_AEC_CONVERGE_ACK;
  q3a_event.u.module_event.current_frame_id = private->cur_sof_id;
  rc = mct_port_send_event_to_peer(port, &q3a_event);

  return rc;
}

/** q3a_port_af_wait_for_aec_update
 *    @port: this is the module's port
 *    @wait: if AF should wait
 *
 *  This function will send event to the AF telling it to wait or to stop
 *  waiting for the AEC to converge.
 *
 *  Return TRUE on success, FALSE on event handler failure.
 **/
static boolean q3a_port_af_wait_for_aec_update(mct_port_t *port, boolean wait)
{
  boolean rc = TRUE;
  q3a_port_private_t *private = port->port_private;
  stats_set_params_type stats_set_param;
    mct_event_t event;

  Q3A_LOW("AF wait for AEC: %d", wait);
  event.u.ctrl_event.control_event_data =
    (stats_set_params_type*)&stats_set_param;

  event.identity  = private->reserved_id;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  stats_set_param.param_type = STATS_SET_Q3A_PARAM;
  stats_set_param.u.q3a_param.type = Q3A_SET_AF_PARAM;
  stats_set_param.u.q3a_param.u.af_param.type = AF_SET_PARAM_WAIT_FOR_AEC_EST;
  stats_set_param.u.q3a_param.u.af_param.u.af_wait_for_aec_est = wait;

  rc = q3a_port_send_event_to_af_port(port, &event);

  return rc;
}

/** q3a_port_request_do_led_estimation_for_af
  *    @port: this is the module's port
  *    @mode: TRUE/FALSE, AEC should do a LED estimation helping the
  *           AF in a very low lighting condition. Sending this
  *           information to AWB in case it requires the information (optional)
  *
  *  This function will send event to the AEC to do LED estimation for AF.
  *  AWB could also use this information (optional).
  *
  *  Return TRUE on success, FALSE on event handler failure.
 **/
static boolean q3a_port_request_do_led_estimation_for_af(
  mct_port_t *port, boolean mode)
{
  boolean               rc = TRUE;
  q3a_port_private_t    *private = port->port_private;
  stats_set_params_type stats_set_param;
  mct_event_t           event;

  Q3A_LOW("Request AEC for LED AF: %d", mode);
  event.u.ctrl_event.control_event_data =
    (stats_set_params_type*)&stats_set_param;

  event.identity  = private->reserved_id;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  stats_set_param.param_type = STATS_SET_Q3A_PARAM;
  stats_set_param.u.q3a_param.type = Q3A_ALL_SET_PARAM;
  stats_set_param.u.q3a_param.u.q3a_all_param.type =
    Q3A_ALL_SET_DO_LED_EST_FOR_AF;
  stats_set_param.u.q3a_param.u.q3a_all_param.u.est_for_af = mode;

  rc = q3a_port_send_event_downstream(port, &event);

  return rc;
}


static boolean q3a_port_request_aec_roi_off(mct_port_t *port)
{
  boolean                 rc = TRUE;
  q3a_port_private_t      *private = port->port_private;
  stats_set_params_type   stats_set_param;
  mct_event_t             event;

  Q3A_LOW(" E");
  event.u.ctrl_event.control_event_data =
    (stats_set_params_type*)&stats_set_param;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  stats_set_param.param_type = STATS_SET_Q3A_PARAM;
  stats_set_param.u.q3a_param.type = Q3A_SET_AEC_PARAM;
  stats_set_param.u.q3a_param.u.aec_param.type =
    AEC_SET_PARAM_ROI;
  stats_set_param.u.q3a_param.u.aec_param.u.aec_roi.enable = FALSE;

  rc = q3a_port_send_event_to_aec_port(port, &event);

  return rc;
}

/** q3a_port_request_aec_precapture_start
 *    @port: this is the module's port
 *
 *  This function will send event to the AEC to update the precapture start event, after
     LED flash AF done state.
 *
 *  Return TRUE on success, FALSE on event handler failure.
 **/
static boolean q3a_port_request_aec_precapture_start(mct_port_t *port)
{
  boolean               rc = TRUE;
  q3a_port_private_t    *private = port->port_private;
  stats_set_params_type stats_set_param;
  mct_event_t           event;

  Q3A_LOW("E");
  event.u.ctrl_event.control_event_data =
    (stats_set_params_type*)&stats_set_param;

  event.identity  = private->reserved_id;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  stats_set_param.param_type = STATS_SET_Q3A_PARAM;
  stats_set_param.u.q3a_param.type = Q3A_SET_AEC_PARAM;
  stats_set_param.u.q3a_param.u.aec_param.type =
    AEC_SET_PARAM_PRECAPTURE_START;

  rc = q3a_port_send_event_to_aec_port(port, &event);

  return rc;
}

/** q3a_port_request_led_off
 *    @port: Port handle
 *
 *  This function sets the flag to restore the LED off value in next AEC update.
 **/
static void q3a_port_request_led_off(mct_port_t *port)
{
  q3a_port_private_t *private = port->port_private;

  pthread_mutex_lock(&private->af_led_data.state_lock);
  private->af_led_data.restore_led_off = TRUE;
  pthread_mutex_unlock(&private->af_led_data.state_lock);
}

/** q3a_port_start_timer
 *    @port: this is the module's port
 *
 *  Once LED sequence is complete, we
 *  wait for some time before resetting our state to idle.
 *  This helps to avoid the duplicate LED sequence pre-capture
 *  comes right after auto-focus.
 *
 *  Return nothing
 **/
static void q3a_port_start_timer(mct_port_t *port)
{
  int frames_to_wait = 0;
  q3a_port_private_t *private = (q3a_port_private_t *)(port->port_private);
  int preview_fps = (private->af_led_data.preview_fps >> 8);

  /* Compute number of frames to wait post LED AF for sanpshot */
  frames_to_wait = preview_fps * (Q3A_PORT_LED_WAIT_TIME / 1000);

  Q3A_HIGH("Preview_FPS: %d Time_to_wait: %d ms Frame_count: %d",
    preview_fps, Q3A_PORT_LED_WAIT_TIME, frames_to_wait);

  pthread_mutex_lock(&private->af_led_data.timer_lock);
  private->af_led_data.led_wait_count = frames_to_wait;
  pthread_mutex_unlock(&private->af_led_data.timer_lock);
} /* q3a_port_start_timer */

/** q3a_port_check_timer
 *    @port: this is the module's port
 *
 *  Check if our timer is still on. If
 *  we have timed out move to IDLE state.
 *
 *  Return nothing
 **/
static void q3a_port_check_timer_and_update(mct_port_t *port)
{
  q3a_port_private_t *private = (q3a_port_private_t *)(port->port_private);
  boolean            send_event = FALSE;
  mct_event_t        q3a_event;

  pthread_mutex_lock(&private->af_led_data.timer_lock);
  if (private->af_led_data.led_wait_count > 0) {
    private->af_led_data.led_wait_count -= 1;
    Q3A_LOW("Frames to wait before resetting the LED state: %d",
      private->af_led_data.led_wait_count);
    send_event =  FALSE;
  } else if (private->af_led_data.led_wait_count == 0) {
    private->af_led_data.led_wait_count = -1;
    send_event = TRUE;
  } else {
    send_event = FALSE;
  }
  pthread_mutex_unlock(&private->af_led_data.timer_lock);

  if (send_event) {
    /* Request Sensor to reset their LED state too */
    Q3A_HIGH("Timeout! Request to reset LED state.",
      MCT_EVENT_MODULE_LED_STATE_TIMEOUT);
    q3a_event.direction           = MCT_EVENT_UPSTREAM;
    q3a_event.identity            = private->reserved_id;
    q3a_event.type                = MCT_EVENT_MODULE_EVENT;
    q3a_event.u.module_event.type = MCT_EVENT_MODULE_LED_STATE_TIMEOUT;
    q3a_event.u.module_event.current_frame_id = private->cur_sof_id;
    mct_port_send_event_to_peer(port, &q3a_event);

    /* Send downstream too */
    q3a_event.direction           = MCT_EVENT_DOWNSTREAM;
    q3a_port_send_event_downstream(port, &q3a_event);

    /* If we are in AF_DONE state now, time to move to IDLE state */
    pthread_mutex_lock(&private->af_led_data.state_lock);
    private->af_led_data.state = AF_WITH_LED_STATE_IDLE;
    pthread_mutex_unlock(&private->af_led_data.state_lock);
  }
} /* q3a_port_check_timer_and_update */

/** q3a_port_stop_timer
 *    @port: this is the module's port
 *
 *  Reset the timer
 *
 *  Return nothing
 **/
static void q3a_port_stop_timer(mct_port_t *port)
{
  q3a_port_private_t *private = (q3a_port_private_t *)(port->port_private);

  Q3A_LOW("Stop timer");
  pthread_mutex_lock(&private->af_led_data.timer_lock);
  private->af_led_data.led_wait_count = 0;
  pthread_mutex_unlock(&private->af_led_data.timer_lock);

  q3a_port_check_timer_and_update(port);
} /* q3a_port_stop_timer */

static boolean q3a_port_send_fast_aec_state(mct_port_t *port)
{
  boolean            rc = FALSE;
  q3a_port_private_t *private = (q3a_port_private_t *)(port->port_private);
  stats_set_params_type   stats_set_param;
  mct_event_t             event;

  event.u.ctrl_event.control_event_data =
    (stats_set_params_type*)&stats_set_param;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  stats_set_param.param_type = STATS_SET_Q3A_PARAM;

  /*First send to aec */
  pthread_mutex_lock(&private->fast_aec_data.state_lock);
  stats_set_param.u.q3a_param.type = Q3A_SET_AEC_PARAM;
  stats_set_param.u.q3a_param.u.aec_param.type = AEC_SET_PARM_FAST_AEC_DATA;
  stats_set_param.u.q3a_param.u.aec_param.u.fast_aec_data.enable =
    private->fast_aec_data.enable;
  stats_set_param.u.q3a_param.u.aec_param.u.fast_aec_data.state =
    private->fast_aec_data.state;
  stats_set_param.u.q3a_param.u.aec_param.u.fast_aec_data.num_frames =
    private->fast_aec_data.num_frames;
  pthread_mutex_unlock(&private->fast_aec_data.state_lock);
  rc = q3a_port_send_event_to_aec_port(port, &event);

  /* Then send to awb */
  pthread_mutex_lock(&private->fast_aec_data.state_lock);
  stats_set_param.u.q3a_param.type = Q3A_SET_AWB_PARAM;
  stats_set_param.u.q3a_param.u.awb_param.type = AWB_SET_PARM_FAST_AEC_DATA;
  stats_set_param.u.q3a_param.u.awb_param.u.fast_aec_data.enable =
    private->fast_aec_data.enable;
  stats_set_param.u.q3a_param.u.awb_param.u.fast_aec_data.state =
    private->fast_aec_data.state;
  stats_set_param.u.q3a_param.u.awb_param.u.fast_aec_data.num_frames =
    private->fast_aec_data.num_frames;
  pthread_mutex_unlock(&private->fast_aec_data.state_lock);
  rc = q3a_port_send_event_to_awb_port(port, &event);

  return rc;
}

static void q3a_port_cancel_af(mct_port_t *port)
{
  af_with_led_state_t state;
  q3a_port_private_t *private = (q3a_port_private_t*)(port->port_private);

  /* Get current LED AF state */
  pthread_mutex_lock(&private->af_led_data.state_lock);
  state = private->af_led_data.state;
  pthread_mutex_unlock(&private->af_led_data.state_lock);

  Q3A_LOW("Current LED AF state=%d", state);

  switch (state) {
  case AF_WITH_LED_STATE_IDLE: {
    /* Do nothing */
  }
    break;

  case AF_WITH_LED_STATE_AEC_RUNNING: {
    /* AEC is running, AF is waiting - stop AEC and turn OFF
     * the LED.
     * AF will be stopped by the cancel message itself
     */
    q3a_port_af_wait_for_aec_update(port, FALSE);
    q3a_port_request_do_led_estimation_for_af(port, FALSE);
    q3a_port_request_led_off(port);
  }
    break;

  case AF_WITH_LED_STATE_AF_RUNNING: {
    q3a_port_request_do_led_estimation_for_af(port, FALSE);
    /* AEC is done, but the LED is still ON, so turn it OFF
     * AF will be canceled by the message itself.
     */
    q3a_port_request_led_off(port);
  }
    break;

  case AF_WITH_LED_STATE_AF_DONE: {
  }
    break;

  default: {
  }
    break;
  }

  /* Reset the LED AF state machine */
  pthread_mutex_lock(&private->af_led_data.state_lock);
  private->af_led_data.state = AF_WITH_LED_STATE_IDLE;
  pthread_mutex_unlock(&private->af_led_data.state_lock);

  return;
}

/* This function updates the current led state to new state */
inline static boolean q3a_port_update_led_state(q3a_port_private_t *private,
  const af_with_led_state_t current, const af_with_led_state_t next) {

  boolean updated = FALSE;
  pthread_mutex_lock(&private->af_led_data.state_lock);
  if (current == private->af_led_data.state) {
    private->af_led_data.state = next;
    updated = TRUE;
  }
  pthread_mutex_unlock(&private->af_led_data.state_lock);

  return updated;
}

/* This function starts or reset the led af state machine */
inline static void q3a_port_start_reset_led_af(q3a_port_private_t *private,
  const boolean start) {

  private->af_led_data.state = (start) ?
    AF_WITH_LED_STATE_AEC_RUNNING : AF_WITH_LED_STATE_IDLE;
}

/* This function updates the LED AF state based on the AEC update */
static void q3a_port_handle_led_state(
  mct_port_t *port, aec_update_t *aec) {

  q3a_port_private_t *private = (q3a_port_private_t*)port->port_private;

  /* Update aec information */
  pthread_mutex_lock(&private->af_led_data.state_lock);
  private->af_led_data.preview_fps = aec->preview_fps;
  private->af_led_data.led_needed = aec->led_needed;
  private->led_state = aec->led_state;
  pthread_mutex_unlock(&private->af_led_data.state_lock);

  /* Update q3a internal led state based on aec estimation state */
  switch (aec->est_state) {

    case AEC_EST_OFF:
      /* Store led off parameters. This will be used during restoration */
      pthread_mutex_lock(&private->af_led_data.state_lock);
      private->af_led_data.led_off_params = aec->led_off_params;
      pthread_mutex_unlock(&private->af_led_data.state_lock);
      break;

    case AEC_EST_DONE:
      /* Its related to preflash. Nothing to do with LED AF */
      private->aec_estimation = FALSE;
      Q3A_HIGH("AEC_EST_DONE");
      break;

    case AEC_EST_DONE_FOR_AF:
      /* AEC estimation is done and AF will start the search. Reset AF waiting for AEC */
      if (q3a_port_update_led_state(private,
        AF_WITH_LED_STATE_AEC_RUNNING, AF_WITH_LED_STATE_AF_RUNNING)) {

        Q3A_HIGH("AEC_EST_DONE_FOR_AF, start AF");
        q3a_port_af_wait_for_aec_update(port, FALSE);
      }
      break;

    case AEC_EST_NO_LED_DONE:
      /* There are the chances, q3a started the led state based on the previous aec update,
       * but core sends the no led needed, because of sudden change in scene.
       * In this case, reset the q3a led state machine */
      if (q3a_port_update_led_state(private,
        AF_WITH_LED_STATE_AEC_RUNNING, AF_WITH_LED_STATE_IDLE)) {

        Q3A_HIGH("AEC_EST_NO_LED_DONE, reset LED state");
        q3a_port_request_do_led_estimation_for_af(port, FALSE);
        q3a_port_af_wait_for_aec_update(port, FALSE);
      }
      Q3A_HIGH("AEC_EST_NO_LED_DONE. LED not needed");
      break;

    case AEC_EST_START:
      Q3A_HIGH("AEC_EST_START");
      break;

    case AEC_EST_DONE_SKIP:
      Q3A_HIGH("AEC_EST_DONE_SKIP. Preflash not needed");
      break;

    case AEC_EST_DONE_FOR_FD:
      Q3A_HIGH("AEC_EST_DONE_FOR_FD, start led fd");
      break;

    default:
      break;
  }
}

static void q3a_port_handle_roi(mct_port_t *port, aec_update_t *aec) {
  q3a_port_private_t *private = (q3a_port_private_t*)(port->port_private);

  if (private->aec_roi_enable) {
    if (aec->luma_settled_cnt > 3){
      private->aec_settled = TRUE;
    }

    /* If luma is unsettled, even if aec is settled for certain number of frames,
     * disable ROI to AEC consistent */
    if (aec->luma_settled_cnt == 0
      && private->aec_settled == TRUE){
      private->aec_ocsillate_cnt++;
      if (private->aec_ocsillate_cnt == 5) {
        private->aec_roi_enable = FALSE;
        private->aec_settled = FALSE;
        q3a_port_request_aec_roi_off(port);
      }
    } else {
      private->aec_ocsillate_cnt=0;
    }
  }
}

/* This function handles the fast AEC state */
static void q3a_port_handle_fast_aec_state(mct_port_t *port, mct_event_t *event) {

  q3a_port_private_t *private = (q3a_port_private_t*)(port->port_private);
  const mct_event_direction event_dir = MCT_EVENT_DIRECTION(event);

  if (MCT_EVENT_UPSTREAM == event_dir) {
    if (private->fast_aec_data.enable) {
      if (MCT_EVENT_MODULE_EVENT == event->type) {
        const mct_event_module_type_t event_type = event->u.module_event.type;

        if (MCT_EVENT_MODULE_STATS_AEC_UPDATE == event_type) {
          boolean send_event = FALSE;
          pthread_mutex_lock(&private->fast_aec_data.state_lock);

          if (Q3A_FAST_AEC_STATE_AEC_RUNNING == private->fast_aec_data.state) {
            stats_update_t *stats_update =
              (stats_update_t *)event->u.module_event.module_event_data;
            if (stats_update->aec_update.settled) {
              Q3A_LOW("Fast-AEC: Change to AWB converge, fid %d",
                stats_update->aec_update.stats_frm_id);
              private->fast_aec_data.state = Q3A_FAST_AEC_STATE_AWB_RUNNING;
              send_event = TRUE;
            }
          }
          pthread_mutex_unlock(&private->fast_aec_data.state_lock);

          if (send_event) {
            q3a_port_send_fast_aec_state(port);
          }

        } else if (MCT_EVENT_MODULE_STATS_AWB_UPDATE == event_type) {
          boolean send_event = FALSE;
          pthread_mutex_lock(&private->fast_aec_data.state_lock);

          if (private->fast_aec_data.state == Q3A_FAST_AEC_STATE_AWB_RUNNING) {

            Q3A_HIGH("Fast-AEC: Converge Done");
            private->fast_aec_data.state = Q3A_FAST_AEC_STATE_DONE;
            send_event = TRUE;
          }
          pthread_mutex_unlock(&private->fast_aec_data.state_lock);

          if (send_event) {
            q3a_port_send_fast_aec_state(port);
            q3a_port_update_fast_aec_converge_ack(port);
          }
        }
      } // if (MCT_EVENT_MODULE_EVENT == event->type)
    } // if (private->fast_aec_data.enable)
  } else if (MCT_EVENT_DOWNSTREAM ==  event_dir) {

    if (MCT_EVENT_MODULE_EVENT == event->type) {
      mct_event_module_t *mod_evt = &(event->u.module_event);
      if (MCT_EVENT_MODULE_SET_FAST_AEC_CONVERGE_MODE == mod_evt->type) {
        mct_fast_aec_mode_t *fast_aec_mode =
          (mct_fast_aec_mode_t *)(mod_evt->module_event_data);
        q3a_port_fast_aec_t *fast_aec = &private->fast_aec_data;
        boolean send_event = FALSE;

        pthread_mutex_lock(&fast_aec->state_lock);
        fast_aec->enable = fast_aec_mode->enable;
        fast_aec->num_frames = fast_aec_mode->num_frames_in_fast_aec;
        Q3A_LOW("Fast-AEC mode : %d, num %d", fast_aec->enable, fast_aec->num_frames);
        if (fast_aec->enable) {
          if (fast_aec->state == Q3A_FAST_AEC_STATE_IDLE) {
            fast_aec->state = Q3A_FAST_AEC_STATE_AEC_RUNNING;
          } else {
            Q3A_ERR("Fast-AEC: Invalid state: %d", fast_aec->state);
            fast_aec->state = Q3A_FAST_AEC_STATE_DONE;
            send_event = TRUE;
          }
        } else {
          if (fast_aec->state != Q3A_FAST_AEC_STATE_DONE) {
            Q3A_ERR("Fast-AEC: Invalid state: %d", fast_aec->state);
          }
          fast_aec->enable = FALSE;
          fast_aec->state = Q3A_FAST_AEC_STATE_IDLE;
        }
        pthread_mutex_unlock(&fast_aec->state_lock);

        if (send_event) {
          q3a_port_update_fast_aec_converge_ack(port);
        }
        q3a_port_send_fast_aec_state(port);
      }
    } // if (MCT_EVENT_MODULE_EVENT == event->type)
  } // if (MCT_EVENT_UPSTREAM == event_dir)
}

/* This restores the LED off exposure settings after LED AF completion */
static void q3a_port_restore_aec(mct_port_t *port, aec_update_t *aec) {

  q3a_port_private_t *private = (q3a_port_private_t*)(port->port_private);
  boolean restored = FALSE;

  pthread_mutex_lock(&private->af_led_data.state_lock);
  if (private->af_led_data.restore_led_off) {

    /* Prepare snapshot event can come before sending the estimation done
     * state to sensor. In this case, only send the prepare snapshot
     * related state and ignore the estimation done state.
     * This change saves one frame time in flash.*/
    if (private->af_led_data.send_est_skip) {
      aec->est_state = AEC_EST_DONE_SKIP;
      private->af_led_data.send_est_skip = FALSE;
      Q3A_HIGH("Ignore AEC_EST_DONE and send AEC_EST_DONE_SKIP");
    } else {
      aec->est_state = AEC_EST_DONE;
    }

    private->af_led_data.restore_led_off = FALSE;
    restored = TRUE;
  } else if (private->af_led_data.send_est_skip) {
    aec->est_state = AEC_EST_DONE_SKIP;
    private->af_led_data.send_est_skip = FALSE;
  }
  pthread_mutex_unlock(&private->af_led_data.state_lock);

  if (restored) {
    aec_sensitivity_data_t *off = &private->af_led_data.led_off_params;

    /* Restore current update with LED off information. This logic is handled in
     * q3a port side for faster restoartion of LED off */
    aec->lux_idx = off->lux_index;
    aec->real_gain = off->real_gain;
    aec->sensor_gain = off->sensor_gain;
    aec->linecount = off->linecnt;
    aec->s_real_gain = off->s_gain;
    aec->s_linecount = off->s_linecnt;
    aec->l_real_gain = off->l_gain;
    aec->l_linecount = off->l_linecnt;

    aec->total_drc_gain = off->total_drc_gain;
    aec->color_drc_gain = off->color_drc_gain;
    aec->gtm_ratio = off->gtm_ratio;
    aec->ltm_ratio = off->ltm_ratio;
    aec->la_ratio = off->la_ratio;
    aec->gamma_ratio = off->gamma_ratio;

    Q3A_HIGH("Restore AEC LED Off:RG=%f,SG=%f,LC=%d,Lux=%f,ET=%f",
      aec->real_gain, aec->sensor_gain, aec->linecount, aec->lux_idx, aec->exp_time);
  }
}


static boolean q3a_port_is_af_completed(const cam_af_state_t af_state) {
  const boolean completed =
    (af_state == CAM_AF_STATE_FOCUSED_LOCKED ||
     af_state == CAM_AF_STATE_NOT_FOCUSED_LOCKED ||
     af_state == CAM_AF_STATE_PASSIVE_FOCUSED ||
     af_state == CAM_AF_STATE_PASSIVE_UNFOCUSED) ? TRUE : FALSE;
  return completed;
}

static boolean q3a_port_is_af_active(q3a_port_private_t *private) {
  boolean active;
  pthread_mutex_lock(&private->af_led_data.state_lock);
  active =
    (private->af_led_data.af_scene_mode_block == FALSE &&
     private->af_led_data.af_focus_mode_block == FALSE &&
     private->af_supported) ? TRUE : FALSE;
  pthread_mutex_unlock(&private->af_led_data.state_lock);
  return active;
}

/* This function checks the AF state and its used to complete the LED AF */
static void q3a_port_handle_af_state(mct_port_t *port,
  mct_event_t *event) {

  mct_bus_msg_t *bus_msg =
    (mct_bus_msg_t *)event->u.module_event.module_event_data;
  q3a_port_private_t *private = (q3a_port_private_t*)(port->port_private);

  if (MCT_BUS_MSG_AF_IMMEDIATE == bus_msg->type) {

    /* Indicates LED AF is done */
    const cam_af_state_t af_state =
      ((mct_bus_msg_af_immediate_t*)bus_msg->msg)->af_state;

    if (q3a_port_is_af_completed(af_state)) {
      //Q3A_LOW("AF is DONE, AF state=%d", af_state);

      if (q3a_port_update_led_state(private,
        AF_WITH_LED_STATE_AF_RUNNING, AF_WITH_LED_STATE_AF_DONE)) {

        Q3A_HIGH("LED AF completed. Start timer");

        /* Reset AEC estimation post LED AF */
        q3a_port_request_do_led_estimation_for_af(port, FALSE);

        /* Set the AEC restore flag to send the LED off settings in next AEC update */
        q3a_port_request_led_off(port);

        pthread_mutex_lock(&private->af_led_data.state_lock);
        if (TRUE == private->af_led_data.prepare_snapshot_trigger){

          q3a_port_start_reset_led_af(private, FALSE);
          private->af_led_data.prepare_snapshot_trigger = FALSE;
        }

        /* Start timer and wait till timeout to check if any snapshot request */
        q3a_port_start_timer(port);

        pthread_mutex_unlock(&private->af_led_data.state_lock);
      }
    }
  }// if (MCT_BUS_MSG_AF_IMMEDIATE == bus_msg->type)
}

/* This function start the LED AF sequence */
static void q3a_port_start_led_af_sequence(mct_port_t *port) {

  q3a_port_private_t *private = (q3a_port_private_t*)(port->port_private);

  /* Inform AF to wait for AEC to finish the estimation */
  q3a_port_af_wait_for_aec_update(port, TRUE);

  /* Initiate the AEC estimation */
  q3a_port_request_do_led_estimation_for_af(port, TRUE);

  /* Start LED AF state machine */
  pthread_mutex_lock(&private->af_led_data.state_lock);
  q3a_port_start_reset_led_af(private, TRUE);
  pthread_mutex_unlock(&private->af_led_data.state_lock);
}

/* This function handles all related triggers */
static boolean q3a_port_handle_af_trigger(mct_port_t *port,
  q3a_set_params_type  *q3a_param) {

  boolean result = FALSE;
  q3a_port_private_t *private = (q3a_port_private_t*)(port->port_private);
  af_set_parameter_t *parm = (af_set_parameter_t *)&q3a_param->u.af_param;

  if (AF_SET_PARAM_START == parm->type) {
    const boolean led_needed = q3a_port_is_led_needed(port);
    const boolean camera_mode =
      q3a_port_is_led_needed_for_af_mode(private, &private->af_led_data);


    Q3A_HIGH("Received DO AF. LED AF needed=%d,camera=%d,Stream=%d",
      led_needed, camera_mode, private->stream_on);

    if (private->stream_on == FALSE) {
      Q3A_HIGH("Stream is OFF, skipping DO AF");
      result = TRUE;

    } else if (led_needed && camera_mode) {
      /* Inform sensor, this LED for AF */
      q3a_port_update_led_af_to_sensor(port, TRUE);

      /* Initiate LED AF sequence */
      q3a_port_start_led_af_sequence(port);

      Q3A_HIGH("LED AF Started");
    } else {

      /* Reset LED AF state machine */
      pthread_mutex_lock(&private->af_led_data.state_lock);
      q3a_port_start_reset_led_af(private, FALSE);
      pthread_mutex_unlock(&private->af_led_data.state_lock);
      Q3A_HIGH("LED AF not needed");
    }
  }

  else if (AF_SET_PARAM_CANCEL_FOCUS == parm->type) {

    Q3A_HIGH("Received cancel focus");
    q3a_port_cancel_af(port);
    q3a_port_stop_timer(port);
  }
  return result;
}

/* This function update the parameters of prepare snapshot skip */
static void q3a_port_handle_prepare_snap_skip(q3a_port_private_t *private) {

  Q3A_HIGH("Presnapshot is already done. Send estimation skip");

  pthread_mutex_lock(&private->af_led_data.state_lock);
  private->af_led_data.send_est_skip = TRUE;

  /* Reset the LED AF state machine */
  q3a_port_start_reset_led_af(private, FALSE);
  pthread_mutex_unlock(&private->af_led_data.state_lock);
}

/* This function handles the AEC presnapshot triggers */
static boolean q3a_port_handle_aec_presnapshot(mct_port_t *port,
  stats_set_params_type *stat_parm) {

  boolean result = FALSE;
  q3a_set_params_type *q3a_param = &(stat_parm->u.q3a_param);
  q3a_port_private_t *private = (q3a_port_private_t*)(port->port_private);
  aec_set_parameter_t *parm =
    (aec_set_parameter_t*)&q3a_param->u.aec_param;

  if (AEC_SET_PARAM_PREPARE_FOR_SNAPSHOT == parm->type) {

    Q3A_HIGH("Received prepare for snapshot");

    if (q3a_port_is_aec_manual_mode(port)) {
      Q3A_HIGH("Ignore prepare for sanpshot in manual mode");
      result = TRUE;

    } else if (AEC_PRECAPTURE_TRIGGER_START != parm->u.aec_trigger.trigger) {
      Q3A_HIGH("Ignore non TRIGGER_START of prepare for snapshot");

    } else {
      af_with_led_state_t led_state;
      pthread_mutex_lock(&private->af_led_data.state_lock);
      led_state = private->af_led_data.state;
      pthread_mutex_unlock(&private->af_led_data.state_lock);

      if (AF_WITH_LED_STATE_AF_DONE == led_state) {
        q3a_port_handle_prepare_snap_skip(private);

        /* Send precature start to AEC port */
        parm->type =  AEC_SET_PARAM_PRECAPTURE_START;
        q3a_port_request_aec_precapture_start(port);

        result = TRUE;
      } else if (AF_WITH_LED_STATE_IDLE != led_state) {

        Q3A_HIGH("Send snapshot notify to AEC");
        parm->type = AEC_SET_PARAM_PREP_FOR_SNAPSHOT_NOTIFY;
      } else {

        Q3A_HIGH("Do not send snapshot notify to AEC");
        private->aec_estimation = TRUE;
      }

      /* Inform sensor to reset LED AF flag */
      q3a_port_update_led_af_to_sensor(port,FALSE);
    }
  }

  else if (AEC_SET_PARAM_PREP_FOR_SNAPSHOT_LEGACY == parm->type) {

    Q3A_HIGH("Received legacy prepare for snapshot");

    if (private->stream_on == FALSE) {
      Q3A_HIGH("Stream is OFF, skipping for AEC");
      result = TRUE;

    } else {
      af_with_led_state_t led_state;
      pthread_mutex_lock(&private->af_led_data.state_lock);
      led_state = private->af_led_data.state;
      pthread_mutex_unlock(&private->af_led_data.state_lock);

      if (AF_WITH_LED_STATE_AF_DONE == led_state) {
        q3a_port_handle_prepare_snap_skip(private);
        result = TRUE;

      } else if (AF_WITH_LED_STATE_IDLE == led_state) {
        /* Prevent hanging by sending a normal prep snapshot event.
         * It will not start the AF. This should not happen - HAL/App is
         * responsible for the right sequence, but we still need protection. */

        const boolean af_active = q3a_port_is_af_active(private);
        const boolean led_needed = q3a_port_is_led_needed(port);

        /* Inform sensor this is LED AF in pre snapshot */
        q3a_port_update_led_af_to_sensor(port,FALSE);

        Q3A_LOW("Led needed=%d, af active=%d",led_needed, af_active);

        if (led_needed && af_active) {
          /* HACK: Trigger the AF by changing the current event */
          parm->type = (aec_set_parameter_type)AF_SET_PARAM_START;
          stat_parm->param_type = STATS_SET_Q3A_PARAM;
          stat_parm->u.q3a_param.type = Q3A_SET_AF_PARAM;

          /* Initiate LED AF sequence */
          q3a_port_start_led_af_sequence(port);

          pthread_mutex_lock(&private->af_led_data.state_lock);
          private->af_led_data.prepare_snapshot_trigger = TRUE;
          /*
           * add this for fixing that:
           * when cancelAutoFocus and preSnasShot come from the same sof.
           * the preSnapShot will be blocked always at HAL layer.
           *
           * we need reset restore_led_off to FALSE when start the snapshot.
           * to make sure the led af run normally.
           */
          private->af_led_data.restore_led_off = FALSE;
          pthread_mutex_unlock(&private->af_led_data.state_lock);

          Q3A_HIGH("LED AF started");

        } else {
          parm->type = AEC_SET_PARAM_PREPARE_FOR_SNAPSHOT;
          Q3A_HIGH("Send prepare snapshot to AEC");
        }
      } else {

        /* This is a precapture command sent during estimation for AF */
        Q3A_HIGH("Precapture command sent during estimation for AF");
        q3a_port_update_led_af_to_sensor(port, FALSE);
        result = TRUE;
      }
    }
  }

  return result;
}

/* This function update the internal LED AF state based on AEC update */
static void q3a_port_handle_aec_update(mct_port_t *port,
  mct_event_t *event) {
  aec_update_t *aec_update =
    &(((stats_update_t*)event->u.module_event.module_event_data)->aec_update);

  /* In touch AF, do not move to CAF until scene is changed */
#if TAF_SAVE_AEC_AF
  q3a_port_handle_roi(port, aec_update);
#endif

  /* Restore AEC with LED off parameter if LED AF state machine is completed */
  q3a_port_restore_aec(port, aec_update);

  /* Update LED AF state machine using AEC update estimation state */
  q3a_port_handle_led_state(port, aec_update);
}

/* This function handles the capture intent */
void q3a_port_handle_capture_intent(mct_port_t *port,
  stats_common_set_parameter_t *common_param){

  af_with_led_state_t state;
  q3a_port_private_t *private = (q3a_port_private_t*)(port->port_private);

  pthread_mutex_lock(&private->af_led_data.state_lock);
  state = private->af_led_data.state;
  private->af_led_data.capture_intent = common_param->u.capture_type;
  pthread_mutex_unlock(&private->af_led_data.state_lock);

  if (CAM_INTENT_STILL_CAPTURE == common_param->u.capture_type) {
    if (q3a_port_update_led_state(private,
      AF_WITH_LED_STATE_AF_DONE, AF_WITH_LED_STATE_IDLE)) {
      Q3A_HIGH("Reset LED AF state");

    } else if (AF_WITH_LED_STATE_IDLE != state) {
      Q3A_ERR("Unexpected LED AF state");
      q3a_port_cancel_af(port);

    } else if (private->aec_estimation) {
      Q3A_ERR("Unexpected AEC state");

      if (private->led_state == Q3A_LED_LOW) {

        Q3A_ERR("Reset LED AF sequence");
        /* To reset complete LED AF sequence, initialize state to AEC running */
        pthread_mutex_lock(&private->af_led_data.state_lock);
        private->af_led_data.state = AF_WITH_LED_STATE_AEC_RUNNING;
        pthread_mutex_unlock(&private->af_led_data.state_lock);
        q3a_port_cancel_af(port);
      }
      private->aec_estimation = FALSE;
    }
  }
}

/* This function updates the led AF related parameters using upstream event */
static boolean q3a_port_update_led_af_params_upstream(mct_port_t *port,
  mct_event_t *event) {

  boolean updated = FALSE;
  q3a_port_private_t *private = (q3a_port_private_t*)(port->port_private);

  /* Update AEC bus message */
  if (MCT_EVENT_MODULE_STATS_POST_TO_BUS == event->u.module_event.type) {

    mct_bus_msg_t *bus_msg =
      (mct_bus_msg_t *)event->u.module_event.module_event_data;

    if (MCT_BUS_MSG_SET_AEC_RESET == bus_msg->type) {
      if (private->aec_roi_enable == TRUE){
        private->aec_roi_enable = FALSE;
        q3a_port_request_aec_roi_off(port);
        Q3A_LOW("AEC bus message, ROI is disabled");
      }
      updated = TRUE;
    }
  }

  return updated;
}

/* This function updates the led AF related parameters using downstream event */
static boolean q3a_port_update_led_af_params_downstream(mct_port_t *port,
  mct_event_t *event) {

  boolean updated = FALSE;
  q3a_port_private_t *private = (q3a_port_private_t*)(port->port_private);

  /* Handle module events */
  if (MCT_EVENT_MODULE_EVENT == event->type) {
    mct_event_module_t *mod_evt = &(event->u.module_event);

    if (MCT_EVENT_MODULE_SOF_NOTIFY == mod_evt->type) {
      private->cur_sof_id =
        ((mct_bus_msg_isp_sof_t*)(mod_evt->module_event_data))->frame_id;

      /* Update timer and check for timeout for snapshot */
      if (AF_WITH_LED_STATE_AF_DONE == private->af_led_data.state) {
        q3a_port_check_timer_and_update(port);
      }
      updated = TRUE;
    }
  }

  /* Handle control commands */
  else if (MCT_EVENT_CONTROL_CMD == event->type) {
    mct_event_control_t *ctrl_evt =
      (mct_event_control_t *)&(event->u.ctrl_event);

    if (MCT_EVENT_CONTROL_SOF == ctrl_evt->type) {
      /* Update timer and check for timeout for snapshot */
      private->cur_sof_id = ctrl_evt->current_frame_id;
      if (AF_WITH_LED_STATE_AF_DONE == private->af_led_data.state) {
        q3a_port_check_timer_and_update(port);
      }
      updated = TRUE;
    }

    /* Handle set params commands */
    else if (MCT_EVENT_CONTROL_SET_PARM == ctrl_evt->type) {
      stats_set_params_type *stat_parm =
        (stats_set_params_type*)ctrl_evt->control_event_data;

      if (STATS_SET_Q3A_PARAM == stat_parm->param_type) {
        q3a_set_params_type *q3a_param = &(stat_parm->u.q3a_param);

        /* Update AF parameters */
        if (q3a_param->type == Q3A_SET_AF_PARAM) {
          af_set_parameter_t *parm = (af_set_parameter_t *)&q3a_param->u.af_param;

          /* Update the AF mode. This is needed to decide whether to trigger LED AF or not */
          if (AF_SET_PARAM_FOCUS_MODE == parm->type) {
            /**
             * led focus running, focus mode change without cancelAutofocus.
             * will cause the led focus timeout issue.
             * check the led af status. if led af status is not idle,
             * we need reset the led focus status.
             */
            if (AF_WITH_LED_STATE_IDLE != private->af_led_data.state
                && private->af_led_data.af_mode != parm->u.af_mode) {
              Q3A_HIGH("focus mode change without cancel previous led focus!, reset led focus");
              q3a_port_cancel_af(port);
              q3a_port_stop_timer(port);
            }

            pthread_mutex_lock(&private->af_led_data.state_lock);
            private->af_led_data.af_mode = parm->u.af_mode;
            if (parm->u.af_mode == CAM_FOCUS_MODE_INFINITY ||
               parm->u.af_mode == CAM_FOCUS_MODE_MANUAL) {
              private->af_led_data.af_focus_mode_block = TRUE;
            } else {
              private->af_led_data.af_focus_mode_block = FALSE;
            }
            pthread_mutex_unlock(&private->af_led_data.state_lock);
            Q3A_LOW("Focus mode set to %d, af block %d",
              parm->u.af_mode, private->af_led_data.af_focus_mode_block);
            updated = TRUE;
          }
        }

        /* Update AEC parameters */
        else if (q3a_param->type == Q3A_SET_AEC_PARAM) {
          aec_set_parameter_t *parm = (aec_set_parameter_t*)&q3a_param->u.aec_param;

          if (AEC_SET_PARAM_ON_OFF == parm->type) {
            private->q3a_enable_aec = parm->u.enable_aec;
            Q3A_LOW("Enable AEC: %d", private->q3a_enable_aec);
            updated = TRUE;
          }

          else if (AEC_SET_PARAM_ROI  == parm->type) {
            if (parm->u.aec_roi.enable) {
              private->aec_roi_enable = TRUE;
            }
            private->aec_settled = FALSE;
            Q3A_LOW("ROI enable: %d", parm->u.aec_roi.enable);
            updated = TRUE;
          }
        }
      }// if (stat_parm->param_type == STATS_SET_Q3A_PARAM)

      /* Update common parameters */
      else if (stat_parm->param_type == STATS_SET_COMMON_PARAM) {
        stats_common_set_parameter_t *common_param =
          &(stat_parm->u.common_param);

        if (COMMON_SET_PARAM_BESTSHOT == common_param->type) {
          pthread_mutex_lock(&private->af_led_data.state_lock);
          if (common_param->u.bestshot_mode == CAM_SCENE_MODE_SUNSET ||
              common_param->u.bestshot_mode == CAM_SCENE_MODE_LANDSCAPE) {
            private->af_led_data.af_scene_mode_block = TRUE;
          } else {
            private->af_led_data.af_scene_mode_block = FALSE;
          }
          pthread_mutex_unlock(&private->af_led_data.state_lock);
          Q3A_LOW("Received BS. Set AF scene mode block to %d",
            private->af_led_data.af_scene_mode_block);
          updated = TRUE;
        }

        else if (COMMON_SET_PARAM_META_MODE == common_param->type) {
          private->q3a_meta_mode = common_param->u.meta_mode;
          Q3A_LOW("Meta mode: %d", private->q3a_meta_mode);
          updated = TRUE;
        }

        else if (COMMON_SET_PARAM_STREAM_ON_OFF == common_param->type) {
          private->stream_on = common_param->u.stream_on;
          Q3A_LOW("Stream on: %d", private->stream_on);
          updated = TRUE;

          /* If all streams are OFF, reset the restore flag to avoid sending the
           * EST DONE in next stream ON */
          if (FALSE == private->stream_on) {
            pthread_mutex_lock(&private->af_led_data.state_lock);
            private->af_led_data.restore_led_off = FALSE;
            pthread_mutex_unlock(&private->af_led_data.state_lock);
          }
        }
      } // else if (stat_parm->param_type == STATS_SET_COMMON_PARAM)
    } // else if (MCT_EVENT_CONTROL_SET_PARM == ctrl_evt->type)
  } // End of MCT_EVENT_CONTROL_CMD

  return updated;
}

/* This function updates the led AF related parameters */
static boolean q3a_port_update_led_af_params(mct_port_t *port,
  mct_event_t *event)
{
  boolean updated = FALSE;
  q3a_port_private_t *private = (q3a_port_private_t*)(port->port_private);
  const mct_event_direction event_dir = MCT_EVENT_DIRECTION(event);

  /* Handle upstream events */
  if (MCT_EVENT_UPSTREAM == event_dir && MCT_EVENT_MODULE_EVENT == event->type) {

    updated = q3a_port_update_led_af_params_upstream(port, event);
  }

  /* Handle downstream events */
  else if (MCT_EVENT_DOWNSTREAM == event_dir) {

    updated = q3a_port_update_led_af_params_downstream(port, event);
  }

  return updated;
}

/** q3a_port_process_event_for_led_af
 *    @port: This is be the module's port.
 *    @event: The event object.
 *
 *  This is the main function to handle the AF+LED feature. It will check
 *  if the LED is needed for the AF to focus and will start a sequence of
 *  events tracking current states to manage the feature.
 *
 *  Return TRUE if the event is sent downstream, FALSE if not.
 **/
static boolean q3a_port_process_event_for_led_af(mct_port_t *port,
  mct_event_t *event) {
  boolean rc = FALSE;
  q3a_port_private_t *private = (q3a_port_private_t*)(port->port_private);
  const mct_event_direction event_dir = MCT_EVENT_DIRECTION(event);
  const boolean param_updated = q3a_port_update_led_af_params(port, event);

  if (param_updated) {
    /* If parameters are updated in previous call, no need to check it further. Simply return */
    return rc;
  }

  /* In upstream event, LED AF state machine is getting updated */
  if (MCT_EVENT_UPSTREAM == event_dir && MCT_EVENT_MODULE_EVENT == event->type) {

    /* LED AF state machine is handled using aec update from aec port */
    if (MCT_EVENT_MODULE_STATS_AEC_UPDATE == event->u.module_event.type) {
      q3a_port_handle_aec_update(port, event);
    }

    /* LED AF will be completed once it recieves the AF done event from af port */
    else if (MCT_EVENT_MODULE_STATS_POST_TO_BUS == event->u.module_event.type) {
      q3a_port_handle_af_state(port, event);
    }
  }

  /* Downstream control commands, triggers the LED AF */
  else if (MCT_EVENT_DOWNSTREAM == event_dir && MCT_EVENT_CONTROL_CMD == event->type) {

    mct_event_control_t *ctrl_evt =
      (mct_event_control_t *)&(event->u.ctrl_event);

    if (MCT_EVENT_CONTROL_SET_PARM == ctrl_evt->type) {
      stats_set_params_type *stat_parm =
        (stats_set_params_type*)ctrl_evt->control_event_data;

      /* Handle q3a set params */
      if (STATS_SET_Q3A_PARAM == stat_parm->param_type) {
        q3a_set_params_type *q3a_param = &(stat_parm->u.q3a_param);

        if (Q3A_SET_AF_PARAM == q3a_param->type) {

          /* Handle AF commands such as Trigger and cancel AF */
          rc = q3a_port_handle_af_trigger(port, q3a_param);

        } else if (Q3A_SET_AEC_PARAM == q3a_param->type) {

          /* Handle AEC presnapshot command */
          rc = q3a_port_handle_aec_presnapshot(port, stat_parm);
        }

      }

      /* Handle common set params */
      else if (STATS_SET_COMMON_PARAM == stat_parm->param_type) {
        stats_common_set_parameter_t *common_param =
          &(stat_parm->u.common_param);

        /* Handle capture intent */
        if (COMMON_SET_CAPTURE_INTENT == common_param->type) {
          q3a_port_handle_capture_intent(port, common_param);
        }
      }
    } // if (MCT_EVENT_CONTROL_SET_PARM == ctrl_evt->type)

    else if ((MCT_EVENT_CONTROL_STOP_ZSL_SNAPSHOT == ctrl_evt->type) ||
      (MCT_EVENT_CONTROL_STREAMOFF == ctrl_evt->type)){
      q3a_port_led_af_reset_state_flag(private);
    }
  } // else if (MCT_EVENT_DOWNSTREAM == event_dir)

  return rc;
}

/** q3a_port_event
 *    @port:  this port from where the event should go
 *    @event: event object to send upstream or downstream
 *
 *  Because stats interface module works only as an event pass through module,
 *  hence its downstream event handling should be fairly straightforward,
 *  but upstream event will need a little bit processing.
 *
 *  Return TRUE for successful event processing, FALSE on failure.
 **/
static boolean q3a_port_event(mct_port_t *port, mct_event_t *event)
{
  boolean            rc = TRUE;
  q3a_port_private_t *private;

  /* Sanity check */
  if (!port || !event) {
    return FALSE;
  }

  private = (q3a_port_private_t *)(port->port_private);
  if (!private) {
    return FALSE;
  }

  /* Sanity check: ensure event is meant for port with the same identity */
  if (!IS_Q3A_PORT_IDENTITY(private, event->identity)) {
    Q3A_ERR("IDENTYTY does not match!");
    return FALSE;
  }

#if 0 /* Enable it only if needed */
  Q3A_LOW("Q3A_EVENT: %s Dir %d",
    event->type == MCT_EVENT_CONTROL_CMD ?
    stats_port_get_mct_event_ctrl_string(event->u.ctrl_event.type):
    (event->type == MCT_EVENT_MODULE_EVENT ?
    stats_port_get_mct_event_module_string(event->u.module_event.type):
    "INVALID EVENT"), MCT_EVENT_DIRECTION(event));
#endif

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_UPSTREAM: {

    /* Handle fast AEC state machine */
    q3a_port_handle_fast_aec_state(port, event);

    /* Process Q3A events to handle Q3A statuses and functionalities */
    if ((rc = q3a_port_process_event_for_led_af(port, event)) == TRUE) {
      break;
    }
    /* The upstream events could come from AEC, AWB or AF module.
     *
     * Need to check event and see if it has to redirect the event
     * to downstream.
     */
    rc = q3a_port_proc_upstream_mod_event(port,event);

    /* Print the AEC update and state variable .*/
    if (event->type == MCT_EVENT_MODULE_EVENT &&
        event->u.module_event.type == MCT_EVENT_MODULE_STATS_AEC_UPDATE) {
        const stats_update_t *out = event->u.module_event.module_event_data;
        if (out->flag == STATS_UPDATE_AEC) {
          const aec_update_t *upd = &out->aec_update;
          Q3A_HIGH("Q3A-AECUpdate: StatsID:%04d,RG:%03.3f,SG:%03.3f,LC:%05d,ET:%03.3f,Lux:%03.0f,"
            "Settled:%d,State:Est:%d,AfLed:%d,fastAec:%d",
            event->u.module_event.current_frame_id, upd->real_gain,
            upd->sensor_gain, upd->linecount, upd->exp_time, upd->lux_idx,
            upd->settled, upd->est_state, private->af_led_data.state,
            private->fast_aec_data.state);
        }
    }
  } /* case MCT_EVENT_UPSTREAM */
    break;

  case MCT_EVENT_DOWNSTREAM: {
    if (event->type == MCT_EVENT_MODULE_EVENT &&
      event->u.module_event.type == MCT_EVENT_MODULE_SET_STREAM_CONFIG) {

      // Save the sensor af support info, used in led af
      sensor_out_info_t *sensor_info =
        (sensor_out_info_t *)(event->u.module_event.module_event_data);
      private->af_supported = sensor_info->af_lens_info.af_supported;
      Q3A_LOW("AF supported =%d",private->af_supported);
    }
    if (event->type == MCT_EVENT_MODULE_EVENT &&
      event->u.module_event.type == MCT_EVENT_MODULE_START_STOP_STATS_THREADS) {

      uint8_t *start_flag = (uint8_t*)(event->u.module_event.module_event_data);
      Q3A_LOW("MCT_EVENT_MODULE_START_STOP_STATS_THREADS start_flag: %d",
        *start_flag);

      if (*start_flag) {
        Q3A_LOW(" Starting AEC/AWB thread!");
        if (q3a_port_start_threads(port, event->identity,
          Q3A_THREAD_AECAWB) == FALSE) {
          rc = FALSE;
          Q3A_ERR("aec thread failed");
        }
        Q3A_LOW(" Starting AF thread!");
        if (q3a_port_start_threads(port, event->identity,
          Q3A_THREAD_AF) == FALSE) {
          Q3A_ERR("Starting AF thread failed!");
          rc = FALSE;
        }
      } else {
        Q3A_LOW(" aecawb_data=%p, af_data: %p",
          private->aecawb_data, private->af_data);
        if (private->aecawb_data) {
          /* Stop AECAWB thread */
          q3a_thread_aecawb_stop(private->aecawb_data);
        }
        if (private->af_data) {
          /* Stop AF thread */
          q3a_thread_af_stop(private->af_data);
        }
      }
    }
    if(MCT_EVENT_MODULE_EVENT == event->type
      && event->u.module_event.type == MCT_EVENT_MODULE_PREVIEW_STREAM_ID) {

        mct_stream_info_t  *stream_info =
          (mct_stream_info_t *)(event->u.module_event.module_event_data);

        private->preview_stream_id =
          (stream_info->identity & 0x0000FFFF);
        rc = q3a_port_send_event_downstream(port, event);
        if (rc == FALSE) {
          Q3A_LOW("Send downstream event failed.");
        }

        break;
    }
    if(MCT_EVENT_MODULE_EVENT == event->type
      && event->u.module_event.type == MCT_EVENT_MODULE_STREAM_CROP ) {
       mct_bus_msg_stream_crop_t *stream_crop =
        (mct_bus_msg_stream_crop_t *)event->u.module_event.module_event_data;

      if(stream_crop->stream_id != private->preview_stream_id) {
        break;
      }
    }
    if (private->state == Q3A_PORT_STATE_LINKED ||
      (MCT_EVENT_MODULE_EVENT == event->type &&
        event->u.module_event.type == MCT_EVENT_MODULE_MODE_CHANGE)) {

      /* This event is received when the stream type changes.
       * Should pass on the event to the corresponding sub-ports
       * through downstream.
       */
      if(MCT_EVENT_MODULE_EVENT == event->type &&
        event->u.module_event.type == MCT_EVENT_MODULE_MODE_CHANGE) {
        private->stream_type =
          ((stats_mode_change_event_data*)
          (event->u.module_event.module_event_data))->stream_type;
        private->reserved_id =
          ((stats_mode_change_event_data*)
          (event->u.module_event.module_event_data))->reserved_id;
      }

      /* Handle fast AEC state machine */
      q3a_port_handle_fast_aec_state(port, event);

      /* Process Q3A events to handle Q3A statuses and functionalities */
      if ((rc = q3a_port_process_event_for_led_af(port, event)) == TRUE) {
        /* Downstream event processed, no need to process again */
        break;
      }

      if ((rc = q3a_port_send_event_downstream(port, event)) == FALSE) {
        Q3A_LOW("Send downstream event failed.");
        break;
      }
    }
  } /* case MCT_EVENT_TYPE_DOWNSTREAM */
    break;

  default: {
  }
    break;
  } /* switch (MCT_EVENT_DIRECTION(event)) */

  return rc;
} /* q3a_port_event */

/** q3a_port_set_caps
 *    @port: port object which the caps to be set;
 *    @caps: this port's capabilities.
 *
 *  Function overwrites a ports capability.
 *
 *  Return TRUE if it is valid source port.
 **/
static boolean q3a_port_set_caps(mct_port_t *port, mct_port_caps_t *caps)
{
  if (strcmp(MCT_PORT_NAME(port), "q3a_sink")) {
    Q3A_ERR("Port name mismatch: %s != q3a_sink",
       MCT_PORT_NAME(port));
    return FALSE;
  }

  port->caps = *caps;
  return TRUE;
} /* q3a_port_set_caps */

/** q3a_port_check_caps_reserve
 *    @port:        this interface module's port;
 *    @peer_caps:   the capability of peer port which wants to match
 *                  interface port;
 *    @stream_info: the info for this stream
 *
 *  Stats modules are pure software modules, and every port can
 *  support one identity. If the identity is different, support
 *  can be provided via creating a new port. Regardless source or
 *  sink port, once capabilities are matched,
 *  - If this port has not been used, it can be supported;
 *  - If the requested stream is in existing identity, return failure
 *  - If the requested stream belongs to a different session, the port
 *    can not be used.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean q3a_port_check_caps_reserve(mct_port_t *port, void *caps,
  void *stream_nfo)
{
  boolean            rc = FALSE;
  mct_port_caps_t    *port_caps;
  q3a_port_private_t *private;
  mct_stream_info_t  *stream_info = stream_nfo;


  if (!port || !caps || !stream_info ||
    strcmp(MCT_OBJECT_NAME(port), "q3a_sink")) {
    Q3A_ERR("Invalid parameters!");
    return rc;
  }

  MCT_OBJECT_LOCK(port);

  port_caps = (mct_port_caps_t *)caps;
  if (port_caps->port_caps_type != MCT_PORT_CAPS_STATS) {
    rc = FALSE;
    goto reserve_done;
  }

  private = (q3a_port_private_t *)port->port_private;

  Q3A_LOW(" state %d", private->state);
  switch (private->state) {
    case Q3A_PORT_STATE_LINKED: {
    if (IS_Q3A_PORT_IDENTITY(private, stream_info->identity)) {
      rc = TRUE;
    }
  }
    break;

  case Q3A_PORT_STATE_CREATED:
  case Q3A_PORT_STATE_UNRESERVED: {

    if (private->aec_port->check_caps_reserve(private->aec_port,
      caps, stream_info) == FALSE) {
      rc = FALSE;
      break;
    }

    if (private->awb_port->check_caps_reserve(private->awb_port,
      caps, stream_info) == FALSE) {
      rc = FALSE;
      break;
    }

    if (private->af_port->check_caps_reserve(private->af_port,
      caps, stream_info) == FALSE) {
      rc = FALSE;
      break;
    }

    private->reserved_id = stream_info->identity;
    private->state       = Q3A_PORT_STATE_RESERVED;
    private->stream_type = stream_info->stream_type;
    rc = TRUE;
  }
    break;

  case Q3A_PORT_STATE_RESERVED: {
    if (IS_Q3A_PORT_IDENTITY(private, stream_info->identity)) {
      rc = TRUE;
    }
  }
    break;

  default: {
    rc = FALSE;
  }
    break;
  }

reserve_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
} /* q3a_port_check_caps_reserve */

/** module_stats_check_caps_unreserve
 *    @port:     this port object to remove the session/stream;
 *    @identity: session+stream identity.
 *
 *  This function frees the identity from port's children list.
 *
 *  Return FALSE if the identity is not existing, else return is TRUE
 **/
static boolean q3a_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  boolean            rc = FALSE;
  q3a_port_private_t *private;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "q3a_sink")) {
    return FALSE;
  }

  private = (q3a_port_private_t *)port->port_private;
  if (!private) {
    return FALSE;
  }

  if (private->state == Q3A_PORT_STATE_UNRESERVED) {
    return TRUE;
  }

  MCT_OBJECT_LOCK(port);
  if ((private->state == Q3A_PORT_STATE_UNLINKED ||
    private->state == Q3A_PORT_STATE_RESERVED) &&
    (IS_Q3A_PORT_IDENTITY(private, identity))) {

    if (private->aec_port->check_caps_unreserve(private->aec_port,
      identity) == FALSE) {
      rc = FALSE;
      goto unreserve_done;
    }

    if (private->awb_port->check_caps_unreserve(private->awb_port,
      identity) == FALSE) {
      rc = FALSE;
      goto unreserve_done;
    }

    if (private->af_port->check_caps_unreserve(private->af_port,
      identity) == FALSE) {
      rc =  FALSE;
      goto unreserve_done;
    }

    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state       = Q3A_PORT_STATE_UNRESERVED;
      private->reserved_id = (private->reserved_id & 0xFFFF0000);
    }
    rc = TRUE;
  }

unreserve_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
} /* q3a_port_check_caps_unreserve */

/** q3a_port_ext_link
 *    @identity:  Identity of session/stream
 *    @port:      SRC/SINK of stats ports
 *    @peer:      For stats sink - peer is most likely isp port
 *                For src module - peer is submodules sink.
 *
 *  Set stats port's external peer port.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean q3a_port_ext_link(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  boolean            rc = FALSE;
  boolean            thread_init = FALSE;
  q3a_port_private_t *private;
  mct_event_t        event;

  if (strcmp(MCT_OBJECT_NAME(port), "q3a_sink")) {
    Q3A_ERR("Q3A port name does not match!");
    return FALSE;
  }

  private = (q3a_port_private_t *)port->port_private;
  if (!private) {
    Q3A_ERR("Private port is NULL!");
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  switch (private->state) {
  case Q3A_PORT_STATE_RESERVED:
  case Q3A_PORT_STATE_UNLINKED: {
    if (!IS_Q3A_PORT_IDENTITY(private, identity)) {
      break;
    }
  }
  /* Fall through, no break */
  case Q3A_PORT_STATE_CREATED: {
    Q3A_LOW("q3a port state created");
    thread_init = TRUE;
    rc = TRUE;
  }
    break;
  case Q3A_PORT_STATE_LINKED: {
    Q3A_LOW("q3a port state linked");
    if (IS_Q3A_PORT_IDENTITY(private, identity)) {
      rc = TRUE;
      thread_init = FALSE;
    }
  }
    break;

  default: {
  }
    break;
  }

  if (rc == TRUE) {
    /* Invoke sub ports' ext link */
    Q3A_LOW(" Invoke sub-ports ext link");
    if (MCT_PORT_EXTLINKFUNC(private->aec_port)) {
      if (MCT_PORT_EXTLINKFUNC(private->aec_port)(identity,
        private->aec_port, port) == FALSE) {
        rc = FALSE;
        Q3A_ERR("AEC external link failed!");
        goto aec_link_fail;
      }
    }

    if (MCT_PORT_EXTLINKFUNC(private->awb_port)) {
      if (MCT_PORT_EXTLINKFUNC(private->awb_port)(identity,
        private->awb_port, port) == FALSE) {
        rc = FALSE;
        Q3A_ERR("AWB external link failed!");
        goto awb_link_fail;
      }
    }

    if (MCT_PORT_EXTLINKFUNC(private->af_port)) {
      if (MCT_PORT_EXTLINKFUNC(private->af_port)(identity,
        private->af_port, port) == FALSE){
        rc = FALSE;
        Q3A_ERR("AF external link failed!");
        goto af_link_fail;
      }
    }

    if ((NULL == private->aecawb_data)|| (NULL == private->af_data)) {
      Q3A_ERR("Q3A Thread init failed");
      goto init_thread_fail;
    }

    private->state = Q3A_PORT_STATE_LINKED;
    MCT_PORT_PEER(port) = peer;
    MCT_OBJECT_REFCOUNT(port) += 1;
  }

  MCT_OBJECT_UNLOCK(port);
  mct_port_add_child(identity, port);
  Q3A_LOW("rc=%d", rc);

  return rc;

init_thread_fail:
  MCT_PORT_EXTUNLINKFUNC(private->af_port)(identity, private->af_port, port);
af_link_fail:
  MCT_PORT_EXTUNLINKFUNC(private->awb_port)(identity, private->awb_port, port);
awb_link_fail:
  MCT_PORT_EXTUNLINKFUNC(private->aec_port)(identity, private->aec_port, port);
aec_link_fail:
  MCT_OBJECT_UNLOCK(port);
  return rc;
} /* q3a_port_ext_link */

/** q3a_port_unlink
 *    @identity: Identity of session/stream
 *    @port:     q3a module's sink port
 *    @peer:     peer of stats sink port
 *
 * This function unlink the peer ports of stats sink, src ports
 * and its peer submodule's port
 *
 * Return void
 **/
static void q3a_port_unlink(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  q3a_port_private_t *private;

  if (!port || !peer || MCT_PORT_PEER(port) != peer) {
    return;
  }

  private = (q3a_port_private_t *)port->port_private;
  if (!private) {
    return;
  }

  MCT_OBJECT_LOCK(port);
  if (private->state == Q3A_PORT_STATE_LINKED) {
    Q3A_LOW(" Invoke sub-ports ext un link");
    if (private->aec_port->un_link)
      private->aec_port->un_link(identity, private->aec_port, port);

    if (private->awb_port->un_link)
      private->awb_port->un_link(identity, private->awb_port, port);

    if (private->af_port->un_link)
      private->af_port->un_link(identity, private->af_port, port);

    MCT_OBJECT_REFCOUNT(port) -= 1;
    if (!MCT_OBJECT_REFCOUNT(port)) {
      Q3A_LOW("deinit aecawb: %p,  af_data: %p",
        private->aecawb_data, private->af_data);
      private->state = Q3A_PORT_STATE_UNLINKED;
    }
  }
  MCT_OBJECT_UNLOCK(port);
  mct_port_remove_child(identity, port);

  return;
} /* q3a_port_unlink */

/** q3a_port_find_identity
 *    @port:     module's port
 *    @identity: stream/session identity
 *
 *  This function will try to find the port's identity
 *
 *  Return TRUE if this is the identity of the Q3A port, FALSE if not.
 **/
boolean q3a_port_find_identity(mct_port_t *port, unsigned int identity)
{
  q3a_port_private_t *private;

  if ( !port || strcmp(MCT_OBJECT_NAME(port), "q3a_sink")) {
    return FALSE;
  }

  private = port->port_private;
  if (private) {
    return (IS_Q3A_PORT_IDENTITY(private, identity));
  }

  return FALSE;
} /* q3a_port_find_identity */

/** q3a_port_deinit
 *    @port: port object to be deinitialized
 *
 *  This function will free the private port data.
 *
 *  Return void
 **/
void q3a_port_deinit(mct_port_t *port)
{
  q3a_port_private_t *private;
  int ret;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "q3a_sink"))
    return;

  private = port->port_private;
  if (private) {
    q3a_thread_aecawb_deinit(private->aecawb_data);
    private->aecawb_data = NULL;
    q3a_thread_af_deinit(private->af_data);
    private->af_data = NULL;
    ret = pthread_mutex_destroy(&private->af_led_data.state_lock);
    if (ret) {
      if (ret == EBUSY) {
        Q3A_ERR("Unable to destroy af_led_data.state_lock, lock is acquired");
      } else if (ret == EINVAL) {
        Q3A_ERR("Unable to destroy af_led_data.state_lock, lock is invalid");
      }
    }
    ret = pthread_mutex_destroy(&private->af_led_data.timer_lock);
    if (ret) {
      if (ret == EBUSY) {
        Q3A_ERR("Unable to destroy af_led_data.timer_lock, lock is acquired");
      } else if (ret == EINVAL) {
        Q3A_ERR("Unable to destroy af_led_data.timer_lock, lock is invalid");
      }
    }
    free(private);
    private = NULL;
  }
} /* q3a_port_deinit */

/** q3a_port_init
 *    @port: port object to be initialized
 *
 *  Port initialization, use this function to overwrite
 *  default port methods and install capabilities. Stats
 *  module should have ONLY sink port.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
boolean q3a_port_init(mct_port_t *port, mct_port_t *aec_port,
  mct_port_t *awb_port, mct_port_t *af_port, unsigned int identity)
{
  mct_port_caps_t    caps;
  q3a_port_private_t *private;
  mct_list_t         *list;

  private = malloc(sizeof(q3a_port_private_t));
  if (private == NULL) {
    return FALSE;
  }
  STATS_MEMSET(private, 0, sizeof(q3a_port_private_t));

  private->reserved_id = identity;
  private->state       = Q3A_PORT_STATE_CREATED;
  private->aec_port    = aec_port;
  private->awb_port    = awb_port;
  private->af_port     = af_port;

  port->port_private  = private;
  port->direction     = MCT_PORT_SINK;
  caps.port_caps_type = MCT_PORT_CAPS_STATS;
  caps.u.stats.flag   = (MCT_PORT_CAP_STATS_Q3A | MCT_PORT_CAP_STATS_CS_RS |
    MCT_PORT_CAP_STATS_HIST);

  STATS_MEMSET(&private->af_led_data, 0, sizeof(q3a_port_af_led_t));
  pthread_mutex_init(&private->af_led_data.state_lock, NULL);
  pthread_mutex_init(&private->af_led_data.timer_lock, NULL);

  /* Init Q3A port fast aec data. */
  pthread_mutex_init(&private->fast_aec_data.state_lock, NULL);
  private->fast_aec_data.state = Q3A_FAST_AEC_STATE_IDLE;

  private->aecawb_data = q3a_thread_aecawb_init();
  Q3A_LOW("aecawb data: %p", private->aecawb_data);
  if (NULL == private->aecawb_data) {
    Q3A_ERR("aecawb init failed");
  }
  private->af_data = q3a_thread_af_init();
  Q3A_LOW("af data: %p", private->af_data);
  if (NULL == private->af_data) {
    Q3A_ERR("af init failed");
  }

  mct_port_set_event_func(port, q3a_port_event);
  mct_port_set_set_caps_func(port, q3a_port_set_caps);
  mct_port_set_ext_link_func(port, q3a_port_ext_link);
  mct_port_set_unlink_func(port, q3a_port_unlink);
  mct_port_set_check_caps_reserve_func(port, q3a_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, q3a_port_check_caps_unreserve);

  if (port->set_caps) {
    port->set_caps(port, &caps);
  }

  return TRUE;
} /* q3a_port_init */

/** q3a_port_load_values_from_xml
 *    @nodeName:    Node(AEC/AWB/AF) of which data to retrieve.
 *    @xml_values:  XML keys and placeholder to return the value.
 *    @max_size:    Max number of keys.
 *
 *  This function parse the XML file and returns the values
 *  associated with the keys.
 *
 *  Return TRUE on success, FALSE if node or file is not found.
 **/
boolean q3a_port_load_values_from_xml(
  const char *nodeName, q3a_port_xml_values_t* xml_values, const uint32 max_size) {

  xmlDocPtr docPtr = NULL;
  xmlNodePtr rootPtr = NULL, subRootPtr = NULL, curPtr = NULL;
  char *str = NULL;
  uint32 i;
  const char *file_name = QCAMERA_DUMP_FRM_LOCATION"3A_Fixed_Settings.xml";

  /* Load the file */
  docPtr = xmlParseFile(file_name);
  if (!docPtr) {
    Q3A_HIGH("xmlParseFile failed. Please validate the xml: %s", file_name);
    return FALSE;
  }

  /* Get root element */
  rootPtr = xmlDocGetRootElement(docPtr);
  if (!rootPtr) {
    Q3A_HIGH("xmlDocGetRootElement failed rootPtr NULL");
    xmlFreeDoc(docPtr);
    return FALSE;
  }

  Q3A_HIGH("Root name = %s", rootPtr->name);

  /* Get the AWB/AEC/AF root node */
  for (subRootPtr = xmlFirstElementChild(rootPtr);
    subRootPtr; subRootPtr = xmlNextElementSibling(subRootPtr)) {

    if (!xmlStrncmp(subRootPtr->name, (const xmlChar *)nodeName,
    xmlStrlen(subRootPtr->name))) {
      /* Found subroot node */
      break;
    }
  }

  if (subRootPtr) {
    Q3A_HIGH("Sub root name = %s", subRootPtr->name);
  } else {
    /* Subroot not exist, return from function */
    Q3A_HIGH("Unable to find the sub root = %s", nodeName);
    xmlFreeDoc(docPtr);
    return FALSE;
  }

  for (i = 0; i < max_size; i++) {

    /* Match the key from xml with the requested key list */
    for (curPtr = xmlFirstElementChild(subRootPtr); curPtr && xmlStrncmp(
      curPtr->name, (const xmlChar *)xml_values[i].key,
        xmlStrlen(curPtr->name)); curPtr = xmlNextElementSibling(curPtr));

    /* Check if we found a match */
    if (!curPtr) {
      Q3A_HIGH("Parameter %d  %s not present in XML file", i, xml_values[i].key);

    } else {
      /* Retrieve the data */
      str = (char *)xmlNodeListGetString(docPtr, curPtr->xmlChildrenNode, 1);
      if (!str) {
        Q3A_ERR("Parameter invalid");
      } else {
        *(xml_values[i].value) = atof(str);
        Q3A_HIGH("Parameter %d  %s = %f", i, xml_values[i].key, *xml_values[i].value);
      }
    }
  }

  xmlFreeDoc(docPtr);
  return TRUE;
}

