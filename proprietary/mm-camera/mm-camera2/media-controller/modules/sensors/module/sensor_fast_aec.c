
/* sensor_fast_aec.c
 *
 * Copyright (c) 2015-2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "sensor_util.h"

/* Fast AEC timeout in seconds */
#define FAST_AEC_TIMEOUT (1)
#define DEFAULT_FAST_AEC_SEN_MODE (CAM_HFR_MODE_120FPS)

/** module_sensor_load_chromatix_on_fastaec:
 *
 * This function loads chromatix for bayer cameras during stream on operation.
 *
 *  Return boolean --> TRUE for success and FALSE for failure
 **/
static boolean module_sensor_load_chromatix_on_fastaec(
    sensor_output_format_t output_format,
    module_sensor_bundle_info_t *s_bundle,
    mct_event_t *event, mct_module_t *module)
{
  sensor_chromatix_params_t chromatix_params;
  int32_t                   rc = SENSOR_FAILURE;

  if (output_format != SENSOR_BAYER)
    return TRUE;

  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(event);
  RETURN_ON_NULL(module);

  SHIGH("load chromatix for sensor %s",
   s_bundle->sensor_lib_params->sensor_lib_ptr->sensor_slave_info.sensor_name);

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_GET_CUR_CHROMATIX_NAME, &chromatix_params, rc);
  RETURN_FALSE_IF(rc < 0);

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_CHROMATIX,
      CHROMATIX_GET_PTR, &chromatix_params, rc);
  RETURN_FALSE_IF(rc < 0);

  RETURN_ON_FALSE(sensor_util_post_chromatix_event_downstream(
    module, s_bundle, event->identity, &chromatix_params,
    CAMERA_CHROMATIX_MODULE_ALL));

  return TRUE;
}

/** module_sensor_set_start_stream_on_fastaec:
 *
 * This function starts the stream during stream on operation.
 *
 *  Return boolean --> TRUE for success and FALSE for failure
 **/
static boolean module_sensor_set_start_stream_on_fastaec(
    sensor_output_format_t  output_format,
    mct_module_t *module, mct_event_t *event,
    module_sensor_bundle_info_t *s_bundle,
    mct_stream_info_t* stream_info)
{
  int32_t rc = SENSOR_SUCCESS;
  boolean ret = TRUE;

  RETURN_ON_NULL(event);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(stream_info);
  RETURN_ON_NULL(module);

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_GET_SENSOR_FORMAT, &output_format, rc);

  if (rc < 0) {
    SERR("failed");
  } else if ((output_format == SENSOR_BAYER)
      && (SECURE != stream_info->is_secure)
      && (CAM_STREAM_TYPE_RAW != stream_info->stream_type)) {
    stats_get_data_t stats_get;
    stats_get_data_t *dest_stats_get;
    memset(&stats_get, 0, sizeof(stats_get_data_t));

    /* get initial gain/linecount from AEC */
    rc = module_sensor_get_stats_data(module, event->identity, &stats_get);
    RETURN_FALSE_IF(rc < 0);

    if ((stats_get.flag & STATS_UPDATE_AEC) == 0x00) {
      /* non-fatal error */
      SERR("Invalid: No AEC update in stats_get");
    } else {
      mct_bus_msg_t bus_msg;
      int32_t bus_index;
      SLOW("bus msg");
      memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
      /* Send bus msg for passing AEC trigger update */
      if (sizeof(stats_get_data_t)
          > sizeof(s_bundle->aec_metadata.private_data)) {
        SERR("failed");
      } else {
        memcpy(s_bundle->aec_metadata.private_data, &stats_get,
            sizeof(stats_get_data_t));
        bus_msg.sessionid = s_bundle->sensor_info->session_id;
        bus_msg.type = MCT_BUS_MSG_SET_STATS_AEC_INFO;
        bus_msg.size = sizeof(stats_get_data_t);
        SLOW("bus msg size %d", bus_msg.size);
        bus_msg.msg = (void *) &s_bundle->aec_metadata;
        if (mct_module_post_bus_msg(module, &bus_msg) == FALSE)
          SERR("failed");
      }
    }

    /* set initial exposure settings, before stream_on */
    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
        SENSOR_SET_AEC_INIT_SETTINGS, (void*) (&(stats_get.aec_get)), rc);

    if (rc < 0) {
      SERR("failed: non fatal");
    }

    if (FALSE == sensor_util_set_digital_gain_to_isp(module, s_bundle,
            event->identity, NULL ))
      SERR("can't set digital gain");
  } /* if bayer */

  SHIGH("ide %x SENSOR_START_STREAM", event->identity);
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_START_STREAM, NULL, rc);
  RETURN_FALSE_IF(rc < 0);
  mct_bus_msg_t bus_msg;
  memset(&bus_msg, 0x00, sizeof(mct_bus_msg_t));
  bus_msg.sessionid = s_bundle->sensor_info->session_id;
  bus_msg.type = MCT_BUS_MSG_SENSOR_STARTING;
  bus_msg.msg = &s_bundle->stream_thread_wait_time;
  SLOW("%s: Sending start bus message\n", __func__);
  if (mct_module_post_bus_msg(module, &bus_msg) == FALSE)
    SERR("failed: non fatal");

  return ret;
}

static boolean fast_aec_stream_off (module_sensor_bundle_info_t *s_bundle) {

  int  rc = 0;

  /* Reset chromatix names in chromatix submodule */
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_CHROMATIX,
    CHROMATIX_RESET_NAME, NULL, rc);
  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_STOP_STREAM, NULL, rc);
  if (rc < 0) {
    SERR("Sensor stop stream failed while stopping fast aec");
    return FALSE;
  }

  return TRUE;
}

/** port_sensor_restore_stream_on: Restores original stream on after fast AEC
 *
 *  @module: sensor module
 *  @port: sensor port
 *  @event: event
 *  @bundle_info: bundle_info
 *  @bundle: bundle
 *
 *  This function unblocks mctl thread to resume normal stream
 *  ON
 *
 *  Return: TRUE for success and FALSE for failure
 *  **/
static boolean port_sensor_restore_stream_on(mct_module_t *module,
  mct_port_t *port, mct_event_t *event, sensor_bundle_info_t *bundle_info,
  module_sensor_port_bundle_info_t *bundle)
{
  boolean ret = TRUE;
  uint32_t i = 0;
  module_sensor_port_data_t *port_private = NULL;

  /* Retrive port private */
  port_private = (module_sensor_port_data_t *)port->port_private;
  if (!port_private) {
    SERR("failed: port_private %p", port_private);
    return FALSE;
  }

  SHIGH("before module_sensor_stream_on");
  if (bundle) {
    for (i = 0; i < bundle->bundle_config.num_of_streams; i++) {
      SLOW("i %d", i);
      /* Call normal module stream ON */
      /* Fill identity */
      event->identity =
        (bundle_info->session_id << 16) | bundle->bundle_config.stream_ids[i];
      SLOW("event identity %x", event->identity);
      /* Fill control event data */
      event->u.ctrl_event.control_event_data =
        (void *)sensor_util_get_stream_info(port_private->stream_list,
          bundle->bundle_config.stream_ids[i]);
      SHIGH("calling module_sensor_stream_on");
      ret = fast_aec_stream_off(bundle_info->s_bundle);
      if (ret == FALSE) {
        SERR("failed");
        return FALSE;
      }
      ret = module_sensor_stream_on(module, event, bundle_info->s_bundle);
      if (ret == FALSE) {
        SERR("failed");
        return FALSE;
      }
    }
  } else {
    /* Call normal module stream ON */
    ret = fast_aec_stream_off(bundle_info->s_bundle);
    if (ret == FALSE) {
      SERR("failed");
      return FALSE;
    }
    ret = module_sensor_stream_on(module, event, bundle_info->s_bundle);
    if (ret == FALSE) {
      SERR("failed");
      return FALSE;
    }
  }
  return TRUE;
}

/** module_sensor_handle_fast_aec_ack: handle fast aec ack sent
 *  by 3A
 *
 *  @module: sensor module
 *  @identity: identity
 *
 *  This function unblocks mctl thread to resume normal stream
 *  ON
 *
 *  Return: TRUE for success and FALSE for failure
 *  **/
boolean module_sensor_handle_fast_aec_ack(mct_module_t *module,
  uint32_t identity)
{
  boolean              ret = TRUE;
  module_sensor_ctrl_t *module_ctrl = NULL;
  sensor_bundle_info_t bundle_info;

  SLOW("E");
  if (!module) {
    SERR("failed: module %p", module);
    return FALSE;
  }

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  if (!module_ctrl) {
    SERR("failed: module_ctrl %p", module_ctrl);
    return FALSE;
  }

  SLOW("before sensor_util_get_sbundle");
  ret = sensor_util_get_sbundle(module, identity, &bundle_info);
  RETURN_ON_FALSE(ret);

  SLOW("before PTHREAD_MUTEX_LOCK");
  PTHREAD_MUTEX_LOCK(&bundle_info.s_bundle->fast_aec_mutex);

  SLOW("before pthread_cond_signal");
  pthread_cond_signal(&bundle_info.s_bundle->fast_aec_cond);

  SLOW("before PTHREAD_MUTEX_UNLOCK");
  PTHREAD_MUTEX_UNLOCK(&bundle_info.s_bundle->fast_aec_mutex);

  SLOW("X");
  return ret;
}

/** sensor_fast_aec_timeout_ack: sends ack for fast aec in case of timeout
 *
 *  @module: sensor module
 *  @event: event
 *  @s_bundle: bundle info
 *
 *
 *  Return: None
 *  **/
void sensor_fast_aec_timeout_ack(mct_module_t* module,
  mct_event_t* event, module_sensor_bundle_info_t* s_bundle)
{
  boolean ret = TRUE;

  if (FALSE == s_bundle->fast_aec_is_running)
    return;

  /* logic for fast aec */
  if (s_bundle->fast_aec_wait_count <=
    s_bundle->sensor_num_fast_aec_frame_skip) {
    if (s_bundle->fast_aec_wait_count ==
      s_bundle->sensor_num_fast_aec_frame_skip) {

      SHIGH("Fast Aec converge\n");
      boolean ret = module_sensor_handle_fast_aec_ack(module, event->identity);
      if (ret == FALSE) {
        SERR("MCT_EVENT_MODULE_FAST_AEC_CONVERGE_ACK failed");
      }
    }
    s_bundle->fast_aec_wait_count++;
    SLOW("count %d timeout %d\n",
    s_bundle->fast_aec_wait_count,s_bundle->sensor_num_fast_aec_frame_skip);
  }
}

static cam_hfr_mode_t module_sensor_get_fast_aec_sensor_mode(
  __attribute__((unused)) mct_module_t *module)
{
  cam_hfr_mode_t mode = DEFAULT_FAST_AEC_SEN_MODE;

  /* Downstream event to get HFR sensor mode from STATS */

  return mode;
}

/** module_sensor_stream_on_fastaec: sensor stream on
 *
 *  @module: mct module handle
 *  @event: event associated with stream on
 *  @s_bundle: sensor bundle handle
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function executes stream on sequence based on the
 *  following order:
 *  SENSOR_SET_NEW_RESOLUTION
 *  SENSOR_SET_CSIPHY_CFG
 *  SENSOR_SET_CSID_CFG
 *  SENSOR_LOAD_CHROMATIX --> Chromatix will be loaded only for Bayer cameras
 *  SENSOR_SEND_EVENT
 *  SENSOR_SET_START_STREAM **/
boolean module_sensor_stream_on_fastaec(mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t *s_bundle)
{
  boolean                        ret = TRUE;
  int32_t                        rc = SENSOR_SUCCESS;
  int32_t                        i = 0;
  module_sensor_params_t        *module_sensor_params = NULL;
  sensor_get_t                   sensor_get;
  sensor_output_format_t         output_format;
  mct_stream_info_t* stream_info =
    (mct_stream_info_t*) event->u.ctrl_event.control_event_data;
  sensor_set_res_cfg_t stream_on_cfg;
  int32_t bundle_id = -1;
  module_sensor_port_bundle_info_t *bundle_info = NULL;


  memset(&stream_on_cfg, 0, sizeof(sensor_set_res_cfg_t));

  SLOW("ide %x SENSOR_START_STREAM", event->identity);
  mct_port_t *port = sensor_util_find_src_port_with_identity(
                          module, event->identity);
  if (!port) {
    SERR("cannot find matching port with identity=0x%x",
      event->identity);
    return FALSE;
  }
  sensor_util_dump_bundle_and_stream_lists(port, __func__, __LINE__);
  bundle_id = sensor_util_find_bundle_id_for_stream(port, event->identity);

  stream_on_cfg.is_fast_aec_mode_on = TRUE;
  stream_on_cfg.fast_aec_sensor_mode =
    module_sensor_get_fast_aec_sensor_mode(module);

  if (bundle_id == -1) {
    /* Non bundled stream */
    stream_on_cfg.stream_mask = s_bundle->stream_mask;
  } else {
    /* bundled stream */
    bundle_info = sensor_util_find_bundle_by_id(port, bundle_id);
    if (bundle_info) {
      stream_on_cfg.stream_mask = bundle_info->stream_mask;
    }
  }

  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  RETURN_ON_NULL(module_sensor_params);

  rc = module_sensor_params->func_tbl.process(
  module_sensor_params->sub_module_private,
  SENSOR_GET_SENSOR_FORMAT, &output_format);
  if(rc<0)
  {
    SERR("failed");
  }

  ret = module_sensor_set_new_resolution_stream_on(module, event, s_bundle,
      module_sensor_params, &stream_on_cfg, stream_info);
  if(ret == FALSE){
     SERR("set new resolution failed");
     return FALSE;
  }

  ret = module_sensor_set_cfg_stream_on(&sensor_get, s_bundle);
  if (ret == FALSE) {
    SERR("set cfg failed");
    return FALSE;
  }

  ret = module_sensor_load_chromatix_on_fastaec(output_format, s_bundle,
    event, module);
  if (ret == FALSE) {
    SERR("set load chromatix failed");
    return FALSE;
  }

  ret = module_sensor_send_event_stream_on(module, event);
  if (ret == FALSE) {
    SERR("sens event failed");
    return FALSE;
  }

  ret = module_sensor_set_start_stream_on_fastaec(output_format, module,
      event, s_bundle, stream_info);
  if (ret == FALSE) {
    SERR("start stream failed");
    return FALSE;
  }

  SLOW("SUCCESS");
  return TRUE;
}

/** port_sensor_handle_fast_aec_mode: handle FAST AEC session
 *
 *  @module: sensor module
 *  @event: incoming event
 *  @bundle_info: bundle that has sensor information for this
 *              session
 *
 *  This function decides configures sensor for FAST AEC mode
 *
 *  Return TRUE on success and FALSE on failure **/

boolean port_sensor_handle_fast_aec_mode(mct_module_t *module,
  mct_port_t *port, mct_event_t *event, sensor_bundle_info_t *bundle_info,
  module_sensor_port_bundle_info_t *bundle)
{
  boolean                   ret = TRUE;
  uint32_t                  i = 0;
  module_sensor_port_data_t *port_private = NULL;
  mct_stream_info_t         *stream_info = NULL;
  struct timespec ts;
  cam_hfr_mode_t fast_aec_sensor_mode;
  uint16_t num_frames_fast_aec =
    bundle_info->s_bundle->sensor_num_fast_aec_frame_skip;

  SLOW("E");

  /* Retrive port private */
  port_private = (module_sensor_port_data_t *)port->port_private;
  if (!port_private) {
    SERR("failed: port_private %p", port_private);
    return FALSE;
  }

  /* For bundle usecase, pass identity of PREVIEW */
  if (bundle) {
    event->identity = sensor_util_get_ide_from_stream_type(
      port_private->stream_list, CAM_STREAM_TYPE_PREVIEW);
    SLOW("preview event identity %x", event->identity);
    stream_info = sensor_util_get_stream_info(port_private->stream_list,
      event->identity & 0xFFFF);
    if (!stream_info)
      return FALSE;
    SLOW("stream identity %x", stream_info->identity);
    event->u.ctrl_event.control_event_data = (void *)stream_info;
  }

  SLOW("before sensor_util_pass_hfr_event");
  /* 1. Pass HFR enable event downstream */
  fast_aec_sensor_mode = module_sensor_get_fast_aec_sensor_mode(module);
  ret = sensor_util_pass_hfr_event(port, event->identity, fast_aec_sensor_mode);
  RETURN_ON_FALSE(ret);

  SLOW("before sensor_util_pass_fast_aec_mode_event");
  /* 2. Pass FAST AEC mode event downstream */
  ret = sensor_util_pass_fast_aec_mode_event(port, event->identity, 1,
    num_frames_fast_aec);
  RETURN_ON_FALSE(ret);

  bundle_info->s_bundle->fast_aec_is_running = TRUE;

  SLOW("before module_sensor_stream_on_fastaec for fast AEC");
  /* 3. Call fast aec mode stream ON */
  ret = module_sensor_stream_on_fastaec(module, event, bundle_info->s_bundle);
  RETURN_ON_FALSE(ret);

  SLOW("before PTHREAD_MUTEX_LOCK");

  /* 4. Block this thread until AEC is converged */
  PTHREAD_MUTEX_LOCK(&bundle_info->s_bundle->fast_aec_mutex);
  clock_gettime(CLOCK_MONOTONIC, &ts);
  ts.tv_sec  += FAST_AEC_TIMEOUT;
  SLOW("before pthread_cond_wait");
  pthread_cond_timedwait(&bundle_info->s_bundle->fast_aec_cond,
    &bundle_info->s_bundle->fast_aec_mutex, &ts);
  bundle_info->s_bundle->fast_aec_is_running = FALSE;
  SLOW("before PTHREAD_MUTEX_UNLOCK");
  PTHREAD_MUTEX_UNLOCK(&bundle_info->s_bundle->fast_aec_mutex);

  SLOW("before sensor_util_pass_stream_off_event for fast AEC");
  /* 5. Call module stream OFF for preview */
  ret = sensor_util_pass_stream_off_event(port, event->identity,
    (mct_stream_info_t *)event->u.ctrl_event.control_event_data);
  RETURN_ON_FALSE(ret);


  SLOW("before sensor_util_pass_bundle_event");
  /* 6. Pass bundle event downstream */
  if (bundle) {
    ret = sensor_util_pass_bundle_event(port, event->identity,
      &bundle->bundle_config);
    RETURN_ON_FALSE(ret);
  }

  SLOW("before sensor_util_pass_hfr_event");
  /* 6. Pass HFR disable event downstream */
  ret = sensor_util_pass_hfr_event(port, event->identity, CAM_HFR_MODE_OFF);
  RETURN_ON_FALSE(ret);

  SLOW("before sensor_util_pass_fast_aec_mode_event");
  /* 7. Pass FAST AEC mode event downstream */
  ret = sensor_util_pass_fast_aec_mode_event(port, event->identity, 0,
    num_frames_fast_aec);
  RETURN_ON_FALSE(ret);

  ret = port_sensor_restore_stream_on(module, port, event, bundle_info, bundle);
  RETURN_ON_FALSE(ret);

  bundle_info->s_bundle->fast_aec_required = FALSE;

  SLOW("X");

  return TRUE;
}

/** port_sensor_handle_stream_on_fastaec: handle stream on in port to
 *  figure out FAST AEC session
 *
 *  @module: sensor module
 *  @event: incoming event
 *  @bundle_info: bundle that has sensor information for this
 *              session
 *
 *  This function decides whether fast AEC is required. If
 *  so, it start Fast AEC session and book keeps this stream
 *  that will be used later during normal stream on
 *
 *  Return TRUE on success and FALSE on failure **/
boolean port_sensor_handle_stream_on_fastaec(mct_module_t *module,
  mct_event_t *event,
  sensor_bundle_info_t *bundle_info)
{
  boolean                          ret = TRUE;
  mct_port_t                       *port = NULL;
  int32_t                          bundle_id = -1;
  module_sensor_port_bundle_info_t *bundle = NULL;

  SLOW("E");

  if (!module || !event || !bundle_info) {
    SERR("failed: module %p event %p bundle_info %p", module, event,
      bundle_info);
    return FALSE;
  }

  bundle_info->s_bundle->fast_aec_wait_count = 0;

  /* Find port that belongs to incoming identity */
  port = sensor_util_find_src_port_with_identity(module, event->identity);
  if (!port) {
    SERR("failed: port %p", port);
    return FALSE;
  }
  SLOW("port %p", port);

  /* Check whether this stream is bundled */
  bundle_id = sensor_util_find_bundle_id_for_stream(port, event->identity);
  SLOW("bundle id %d", bundle_id);
  if (bundle_id == -1) {
    /* Non bundled stream */
    /* Find if there is already one stream that is currently streaming */
    if (FALSE == sensor_util_find_is_stream_on(port)) {
      /* This is the first stream ON for this port. Start Fast AEC session */
      ret = port_sensor_handle_fast_aec_mode(module, port, event, bundle_info,
        NULL);
      RETURN_ON_FALSE(ret);
    } else {
      /* There is already one stream that is streaming. Forward current stream
         on event downstream without doing anything */
      SLOW("calling module_sensor_stream_on");
      /* Call normal module stream ON */
      ret = module_sensor_stream_on(module, event, bundle_info->s_bundle);
      RETURN_ON_FALSE(ret);
    }
  } else {
    /* bundled stream */
    /* Get bundle from bundle id */
    bundle = sensor_util_find_bundle_by_id(port, bundle_id);
    if (!bundle) {
      SERR("failed: bundle %p", bundle);
      return FALSE;
    }

    /* Check whether preview stream is present in this bundle. FAST AEC mode
       applies only if preview stream is present in this bundle */
    if (bundle_info->s_bundle->stream_mask & (1 << CAM_STREAM_TYPE_PREVIEW)){
      /* Preview stream is present in this bundle, FAST AEC mode is required */
      /* Increament fast AEC count */
      bundle->fast_aec_mode_count++;
      SLOW("fast aec mode count %d", bundle->fast_aec_mode_count);
      SLOW("num of streams %d", bundle->bundle_config.num_of_streams);

      /* Find if this streamon is the last streamon in the bundled case */
      if (bundle->fast_aec_mode_count == bundle->bundle_config.num_of_streams) {
        /* This is the last stream that requires stream ON */

        SLOW("call port_sensor_handle_fast_aec_mode");
        ret = port_sensor_handle_fast_aec_mode(module, port, event, bundle_info,
          bundle);
        RETURN_ON_FALSE(ret);

        /* Reset fast aec mode count */
        bundle->fast_aec_mode_count = 0;
      }
    } else {
      /* Preview stream is not present in this bundle. Continue with normal
         stream ON */
      /* Call normal module stream ON */
      ret = module_sensor_stream_on(module, event, bundle_info->s_bundle);
      RETURN_ON_FALSE(ret);
    }
  }

  SLOW("X");

  return ret;
}
