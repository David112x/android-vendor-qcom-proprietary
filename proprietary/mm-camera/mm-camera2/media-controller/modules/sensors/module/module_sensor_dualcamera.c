/*===========================================================================
 * Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*==========================================================================*/

#include "module_sensor_dualcamera.h"

/*===========================================================================
 * FUNCTION    - module_sensor_dualcamera_reset_pc_params -
 *
 * DESCRIPTION: This function resets phase correction related parameters.
 *==========================================================================*/
void module_sensor_dualcamera_reset_pc_params(
        module_sensor_bundle_info_t* s_bundle){

    if (!s_bundle) {
      SERR("failed to reset the phase correction params");
      return;
    }

    SLOW("Reset phase correction paramertes");
    s_bundle->dualc_info.phase_correction_count = 0;
    s_bundle->dualc_info.reset_correction = FALSE;
    s_bundle->dualc_info.frame_idx = 0;
    s_bundle->dualc_info.sof_timestamp = 0;
}

/*===========================================================================
 * FUNCTION    - module_sensor_dualcamera_phase_sync -
 *
 * DESCRIPTION: This function is used for correcting phase difference in
 *              dual camera application when the upper layer requests sensor to
 *              either take control of the sync or to aid HW sync to correct
 *              out-of-phase sync'd frames.
 *              Frame id of peer session needs to match to the frame id
 *              stored in dualc_info for this session.
 *              This comparion cannot be done using this session's last_idx
 *              because last_idx can be updated when MCT sends set_param
 *              before the slave's C-SOF arrives and
 *              updates the current timestamp.
 *              This race condition can cause incorrect phase error detection
 *              equal to one frame time or more.
 *==========================================================================*/
int32_t module_sensor_dualcamera_phase_sync(mct_module_t* module,
        mct_event_t* event, module_sensor_bundle_info_t* s_bundle)
{
  int32_t rc             = SENSOR_SUCCESS;
  mct_bus_msg_isp_sof_t  *mct_event_data;
  struct timeval         *sof_timestamp;
  module_sensor_ctrl_t   *sensor_ctrl = NULL;
  peer_info_t            peer_info;
  int32_t                phase_diff = 0;

  if (!s_bundle || !event || !module) {
      SERR("failed");
      return SENSOR_FAILURE;
  }

  sensor_ctrl = (module_sensor_ctrl_t *)module->module_private;

  if (event->u.ctrl_event.type != MCT_EVENT_CONTROL_SOF) {
      SERR("wrong event %d :SHOULD BE MCT_EVENT_CONTROL_SOF",
        event->u.ctrl_event.type);
      return SENSOR_FAILURE;
  }

  if((sensor_ctrl->is_dual_cam == TRUE) &&
     (s_bundle->dualc_info.sync_mechanism != CAM_SYNC_NO_SYNC)){
    mct_event_data =
     (mct_bus_msg_isp_sof_t *)(event->u.ctrl_event.control_event_data);
    sof_timestamp = &mct_event_data->mono_timestamp;

    s_bundle->dualc_info.sof_timestamp =
      sof_timestamp->tv_sec * 1000 +
      sof_timestamp->tv_usec / 1000;

    s_bundle->dualc_info.frame_idx = s_bundle->last_idx;

    SLOW("session id %d, frame_id %d, timestamp %d",
     s_bundle->sensor_info->session_id,
     s_bundle->dualc_info.frame_idx,
     s_bundle->dualc_info.sof_timestamp);

     /* Get timestamp from the peer session */
     sensor_util_post_intramode_event(module, event->identity,
       s_bundle->peer_identity,
       MCT_EVENT_MODULE_SENSOR_GET_PEER_INFO,
       &peer_info);

     /* Correct phase if the frame ids match and if we are not
     * waiting for the previous phase correction to finish */
     if(s_bundle->dualc_info.streaming_master == CAM_MODE_PRIMARY){
         sensor_util_post_intramode_event(module, event->identity,
           s_bundle->peer_identity,
           MCT_EVENT_MODULE_SENSOR_CORRECT_PHASE_DIFF,
           &s_bundle->dualc_info);
     }
     else
     {
         module_sensor_dualcamera_calculate_phase_diff(module, event,
           s_bundle, peer_info);
     }

  }

  return rc;
}

/*===========================================================================
 * FUNCTION    - module_sensor_dualcamera_calculate_phase_diff
 *
 * DESCRIPTION:
 *
 * @s_bundle : This is session data for slave sensor
 * @peer_info: This is the frame id and sof_timestamp for master session
 *
 * This function handles phase difference calculation, Ts - Tm.
 *
 * This also keeps a track of the frame count to wait until next correction
 * calculation. This count will decrement every one pair of master-slave
 * frame. If the slave sleeps in the middle of the correction application
 * then this count will be reset during sleep or stream-off operation.
 *
 * This will also call module_sensor_dualcamera_apply_phase_correction
 * function which then decides whether we are within threshold and then
 * applies the phase correction accordingly.
 *
 *=========================================================================*/
void module_sensor_dualcamera_calculate_phase_diff(mct_module_t* module,
      mct_event_t* event, module_sensor_bundle_info_t* s_bundle,
     peer_info_t peer_info)
{

  int32_t phase_diff = 0;

  if (!s_bundle || !event || !module) {
    SERR("failed");
    return;
  }

  if(s_bundle->dualc_info.frame_idx == peer_info.peer_frameid){
    if(!s_bundle->dualc_info.phase_correction_count){

      /* Calculate phase_diff = Ts - Tm */
      phase_diff =
        s_bundle->dualc_info.sof_timestamp - peer_info.peer_timestamp;

      module_sensor_dualcamera_apply_phase_correction(module,
        event, s_bundle, phase_diff);

    } else {

        /* Wait for previous phase correction to be applied */
        if(s_bundle->dualc_info.phase_correction_count)
          s_bundle->dualc_info.phase_correction_count--;

        SLOW("Phase correction count: %d",
          s_bundle->dualc_info.phase_correction_count);
    }
  }

}

/*===========================================================================
 * FUNCTION    - module_sensor_dualcamera_start_sensor
 *
 * DESCRIPTION:
 * This function handles start stream operation and
 * sending bus message to MCT to indicate sensor is starting.
 *
 * Return boolean --> TRUE for success and FALSE for failure
 *
 *=========================================================================*/
int32_t module_sensor_dualcamera_start_sensor(mct_module_t *module,
  module_sensor_bundle_info_t *s_bundle)
{
  int32_t rc    = SENSOR_SUCCESS;
  mct_bus_msg_t bus_msg_sensor_start;

  if (!s_bundle || !module) {
    SERR("failed");
    return SENSOR_FAILURE;
  }

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_START_STREAM, NULL, rc);

  if(rc < 0){
      SERR("SENSOR_START_STREAM failed on session %d",
        s_bundle->sensor_info->session_id);
  } else {
    memset(&bus_msg_sensor_start, 0, sizeof(mct_bus_msg_t));
    bus_msg_sensor_start.sessionid =
      s_bundle->sensor_info->session_id;
    bus_msg_sensor_start.type = MCT_BUS_MSG_SENSOR_STARTING;
    bus_msg_sensor_start.msg = &s_bundle->stream_thread_wait_time;
    mct_module_post_bus_msg(module, &bus_msg_sensor_start);
  }

  return rc;
}

/*===========================================================================
 * FUNCTION    - module_sensor_dualcamera_streamon
 *
 * DESCRIPTION:
 * This function handles dual camera stream on operation.
 *
 * Sync_On - This will be True when HW/SW sync or Hybrid sync
 *           is enabled. We cannot defer the primary sensor
 *           stream on in this case.
 *           This is False when there is no sync required
 *           between the two sessions.
 *
 * Sync_On = TRUE
 * If the first streamon comes for primary sensor, the streamon
 * happens right away.
 * Successful streamon is indicated by setting is_dual_streamon flag.
 * Subsequent streamon on secondary sensor will also be handled
 * right-away in this case.
 * If the first streamon comes for secondary sensor, then sensor
 * will wait for the steamon on primary session and indicate it
 * is ready to be streamed on by setting is_dual_streamon flag.
 * Primary session will check is_dual_streamon flag and start
 * secondary session accordingly.
 *
 * Sync_On = FALSE
 * The stream on events for both the sessions are independent
 * and can happen as they come. No rule on deferring any session.
 *
 *  Return boolean --> TRUE for success and FALSE for failure
 *=========================================================================*/
boolean module_sensor_dualcamera_streamon(mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t *s_bundle)
{
  boolean rc            = TRUE;
  int32_t ret           = SENSOR_SUCCESS;
  module_sensor_ctrl_t  *sensor_ctrl;
  mct_bus_msg_t         bus_msg_sensor_start;
  boolean sync_on       = TRUE;

  RETURN_ON_NULL(event);
  RETURN_ON_NULL(s_bundle);
  RETURN_ON_NULL(module);

  sensor_ctrl = (module_sensor_ctrl_t *)module->module_private;

  if(s_bundle->dualc_info.sync_mechanism == CAM_SYNC_HW_SYNC ||
     s_bundle->dualc_info.sync_mechanism == CAM_SYNC_SW_SYNC ||
     s_bundle->dualc_info.sync_mechanism == CAM_SYNC_HYBRID_SYNC){
     sync_on = TRUE;
     SLOW("[dual] Sync StreamOn On - Primary first, Secondary later");
  } else {
    sync_on = FALSE;
    SLOW("[dual] Sync StreamOn Off - process stream on as they come");
  }

  PTHREAD_MUTEX_LOCK(&sensor_ctrl->dual_cam_mutex);
  if (s_bundle->dualc_info.dual_mode == CAM_MODE_SECONDARY) {
    if(sync_on == FALSE) {
      SHIGH("[dual] Secondary sensor stream_on for session %d",
        s_bundle->sensor_info->session_id);

      ret = module_sensor_dualcamera_start_sensor(module, s_bundle);
      if(ret < 0)
        goto ERROR;

      sensor_ctrl->is_dual_stream_on = TRUE;
    } else {
      if (sensor_ctrl->is_dual_stream_on == FALSE) {

        SHIGH("[dual]Wait for primary to start streaming");
        sensor_ctrl->is_dual_stream_on = TRUE;

      } else {

        SHIGH("[dual] Secondary sensor stream_on for session %d",
          s_bundle->sensor_info->session_id);
        ret = module_sensor_dualcamera_start_sensor(module, s_bundle);
        if(ret < 0)
          goto ERROR;

      }
    }
  } else {

    SHIGH("[dual]Primary sensor stream_on for session %d",
      s_bundle->sensor_info->session_id);

    ret = module_sensor_dualcamera_start_sensor(module, s_bundle);
    if(ret < 0)
      goto ERROR;

    if (sync_on == TRUE &&
        sensor_ctrl->is_dual_stream_on == TRUE) {
      SHIGH("[dual]send stream_on event to peer session %d",
        s_bundle->peer_identity >> 16);
      sensor_util_post_intramode_event(module, event->identity,
        s_bundle->peer_identity,
        MCT_EVENT_MODULE_SENSOR_STREAM_ON, NULL);
    }

    /* Set primary streaming flag to true */
    sensor_ctrl->is_dual_stream_on = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&sensor_ctrl->dual_cam_mutex);

  return rc;

ERROR:

  SERR("dual streamon failed");
  rc = FALSE;
  PTHREAD_MUTEX_UNLOCK(&sensor_ctrl->dual_cam_mutex);
  return rc;

}

/*===========================================================================
 * FUNCTION    - module_sensor_dualcamera_apply_phase_correction
 *
 * DESCRIPTION: This function calculates the fll to be set for slave session
 *              based on how much out-of-phase it is with respect to the master
 *              session.
 * phase_diff - This is the difference between SOF Timestamp of slave and master
 *              phase_diff = Ts - Tm
 *==========================================================================*/
void module_sensor_dualcamera_apply_phase_correction( mct_module_t *module,
  mct_event_t *event, module_sensor_bundle_info_t* s_bundle,
  int32_t phase_diff)
{

  if (!s_bundle || !module || !event) {
    SERR("failed");
    return;
  }

  if(phase_diff < MIN_DUALC_PHASE_DIFF_THRESHOLD ||
     phase_diff > MAX_DUALC_PHASE_DIFF_THRESHOLD){

      SLOW("Non-preview session, phase diff: %d", phase_diff);

      /* Send event downstream */
      sensor_util_post_downstream_event(module, event->identity,
        MCT_EVENT_MODULE_DUALC_PHASE_CORRECTION, &phase_diff);

      /* Correct the phase if we are in hybrid mode
       * calculate new fll and set new fll */
      if(s_bundle->dualc_info.sync_mechanism == CAM_SYNC_HYBRID_SYNC ){
        sensor_util_post_submod_event(s_bundle, SUB_MODULE_SENSOR,
          SENSOR_DUALC_APPLY_PHASE_CORRECTION, &phase_diff);

        s_bundle->dualc_info.reset_correction = TRUE;
        s_bundle->dualc_info.phase_correction_count =
          DUALC_PHASE_CORRECTION_FRAME_SET;
      }

    } else {
      SLOW("Phase difference is within threshold, diff = %d", phase_diff);
    }

}

/*===========================================================================
 * FUNCTION    - module_sensor_dualcamera_handle_aec_update
 *
 * DESCRIPTION: This function handles aec update for dual camera sessions.
 *==========================================================================*/
boolean module_sensor_dualcamera_handle_aec_update(mct_module_t *module,
        mct_port_t *port, mct_event_t *event, sensor_bundle_info_t *bundle_info)
{
    boolean ret            = FALSE;
    sensor_output_format_t peer_output_format;
    mct_event_module_t     *event_module = NULL;
    module_sensor_ctrl_t   *sensor_ctrl = NULL;

    if (!module || !port || !event || !bundle_info) {
      SERR("failed: %p %p %p %p", module, port, event, bundle_info);
      return FALSE;
    }

    sensor_ctrl = (module_sensor_ctrl_t *)module->module_private;
    event_module = &event->u.module_event;

    /* Get peer sensor's output format */
    sensor_util_post_intramode_event(module, event->identity,
      bundle_info->s_bundle->peer_identity,
      MCT_EVENT_MODULE_SENSOR_GET_PEER_FORMAT, &peer_output_format);

    switch(peer_output_format){
    case SENSOR_YCBCR:
      ret = module_sensor_dualcamera_handle_yuv_aec_update(module, port,
        event,bundle_info);
      break;
    case SENSOR_BAYER:
        ret = module_sensor_dualcamera_handle_bayer_aec_update(module, port,
        event,bundle_info);
      break;
    default:
      /* Send aec update to this session if dual is streaming */
      if(sensor_ctrl->is_dual_streaming == TRUE) {
        ret = port_sensor_handle_aec_update(module,
          port, event, bundle_info, event_module);
        if (ret == FALSE)
          SERR("failed");
      }
      break;
    }

    return ret;
}

/*===========================================================================
 * FUNCTION    - module_sensor_dualcamera_handle_yuv_aec_update
 *
 * DESCRIPTION:
 *  @module: module handle
 *  @port: port handle
 *  @event: event to be handled
 *  @bundle_info: sensor bundle info
 *
 *  Handle aec update when bayer is master and
 *  YUV is slave.
 *  Get the exposure multiplier and send the aec
 *  update to YUV session
 *
 *  Return TRUE on success and FALSE on failure
 *==========================================================================*/
boolean module_sensor_dualcamera_handle_yuv_aec_update(mct_module_t *module,
  mct_port_t *port, mct_event_t *event, sensor_bundle_info_t *bundle_info)
{
    int32_t              rc = 0;
    boolean              ret = TRUE;
    mct_event_module_t   *event_module = NULL;
    module_sensor_ctrl_t *sensor_ctrl = NULL;
    sensor_dual_exp_t    dual_exp;

    if (!module || !port || !event || !bundle_info) {
      SERR("failed: %p %p %p %p", module, port, event, bundle_info);
      return FALSE;
    }

    event_module = &event->u.module_event;
    sensor_ctrl = (module_sensor_ctrl_t *)module->module_private;
    dual_exp.stats_update = event_module->module_event_data;

    /* when dual camera mode, apply master's expousre to AUX sensor
       only when both sensors are streaming out. */
    if (sensor_ctrl->is_dual_cam == TRUE &&
      sensor_ctrl->is_dual_streaming == TRUE) {

      /* send the current exposure mutiplier info + aec update to peer */
      SENSOR_SUB_MODULE_PROCESS_EVENT(bundle_info->s_bundle, SUB_MODULE_SENSOR,
        SENSOR_GET_EXP_MULTIPLIER, &dual_exp.exp_multiplier, rc);
      if (rc < 0) {
        SERR("failed");
      } else {
          /* Send aec update to YUV session */
          sensor_util_post_intramode_event(module, event->identity,
            bundle_info->s_bundle->peer_identity,
            MCT_EVENT_MODULE_STATS_AEC_UPDATE, &dual_exp);
      }

      /* Continue with Bayer's aec update */
      ret = port_sensor_handle_aec_update(module,
        port, event, bundle_info, event_module);
      if (ret == FALSE)
        SERR("failed");
    }
    return ret;
}

/*===========================================================================
 * FUNCTION    - module_sensor_dualcamera_handle_bayer_aec_update
 *
 * DESCRIPTION:
 *  @module: module handle
 *  @port: port handle
 *  @event: event to be handled
 *  @bundle_info: sensor bundle info
 *
 *  Handle aec update for bayer+bayer
 *  dual camera system.
 *  1. For the secondary sensor, check if the linecount
 *     from AEC update is greater than adjusted fll.
 *     If yes, then update the linecount to adjusted
 *     fll to ensure frame rate matching between master
 *     and slave session.
 *  2. Apply the aec update of the current session
 *  3. Send an intraport event to the peer session:
 *     i.  Adjust the FPS if the peer is non-streaming slave
 *     ii. Apply aec update if the slave is sleeping
 *  Return TRUE on success and FALSE on failure
 *==========================================================================*/
boolean module_sensor_dualcamera_handle_bayer_aec_update(mct_module_t *module,
  mct_port_t *port, mct_event_t *event, sensor_bundle_info_t *bundle_info)
{
    int32_t              rc = 0;
    boolean              ret = TRUE;
    mct_event_module_t   *event_module = NULL;
    module_sensor_ctrl_t *sensor_ctrl = NULL;
    sensor_dual_exp_t    dual_exp;

    if (!module || !port || !event || !bundle_info) {
      SERR("failed: %p %p %p %p", module, port, event, bundle_info);
      return FALSE;
    }

    event_module = &event->u.module_event;
    dual_exp.stats_update = event_module->module_event_data;
    sensor_ctrl = (module_sensor_ctrl_t *)module->module_private;

    if (sensor_ctrl->is_dual_cam == TRUE &&
      sensor_ctrl->is_dual_streaming == TRUE) {

      /* 1. Ensure slave and master session fps match in low light */
      if(bundle_info->s_bundle->dualc_info.streaming_master!=CAM_MODE_PRIMARY){
         SENSOR_SUB_MODULE_PROCESS_EVENT(bundle_info->s_bundle,
          SUB_MODULE_SENSOR,
          SENSOR_ADJUST_DUALC_SLAVE_LINECOUNT,
          &dual_exp, rc);
         if (rc == SENSOR_FAILURE)
           SERR("failed");
      }

      /* 2. Send aec update to current session */
      ret = port_sensor_handle_aec_update(module,
        port, event, bundle_info, event_module);
      if (ret == FALSE)
        SERR("failed");

      SENSOR_SUB_MODULE_PROCESS_EVENT(bundle_info->s_bundle,
        SUB_MODULE_SENSOR,
        SENSOR_GET_EXPOSURE_INFO,
        &dual_exp, rc);
      if (rc == SENSOR_FAILURE)
        SERR("failed");

      /* 3. Send aec update to peer session */
      sensor_util_post_intramode_event(module, event->identity,
        bundle_info->s_bundle->peer_identity,
        MCT_EVENT_MODULE_STATS_AEC_UPDATE, &dual_exp);

    }
    return ret;
}

/*===========================================================================
 * FUNCTION    - module_sensor_dualcamera_handle_peer_aec_update -
 *
 * DESCRIPTION: This function is invoked when aec update on one session
 *              invokes handling aec update on peer session.
 *              Intramode on streaming slave session:
 *              If the FPS matching is enabled and sensor is not in phase
 *              correction frame then it adjusts the slave FLL based on
 *              master's exposure time. If sensor is in phase correction frame
 *              then this will reset the flag so that next regular AEC update
 *              can resume.
 *              If the sensor is sleeping then send AEC update on the sleeping
 *              session to maintain the fps.
 *              Intramode on streaming master session:
 *              Since this session will always be awake, this function does
 *              nothing.
 *              Intramode on YUV session:
 *              Send AEC update for YUV session and adjusts the bayer sensor's
 *              fps accordingly.
 *==========================================================================*/
boolean module_sensor_dualcamera_handle_peer_aec_update(mct_module_t *module,
   mct_event_t *event, sensor_bundle_info_t *bundle_info)
{
  boolean ret            = TRUE;
  sensor_output_format_t output_format = SENSOR_BAYER;
  sensor_dual_exp_t      *dual_exp;
  aec_update_t           *aec_update;
  module_sensor_ctrl_t   *sensor_ctrl;
  mct_event_module_t     *event_module;
  int32_t rc             = SENSOR_SUCCESS;
  module_sensor_bundle_info_t *s_bundle;

  if (!event || !module || !bundle_info) {
    SERR("failed");
    return FALSE;
  }

  s_bundle = bundle_info->s_bundle;
  event_module = &event->u.module_event;
    dual_exp = (sensor_dual_exp_t *)event_module->module_event_data;
  aec_update = &dual_exp->stats_update->aec_update;
  sensor_ctrl = (module_sensor_ctrl_t *)module->module_private;

  SLOW("stats gain %f lnct %d exp_t %f", aec_update->dig_gain,
    aec_update->linecount, aec_update->exp_time);

  SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
    SENSOR_GET_SENSOR_FORMAT, &output_format, rc);
  if (rc < 0) {
    SERR("failed");
    return FALSE;
  }

  if (output_format == SENSOR_YCBCR) {

    SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
      SENSOR_SET_AEC_UPDATE_FOR_DUAL, dual_exp, rc);
    if (rc < 0) {
      SERR("failed");
      return FALSE;
    }

    /* For the FPS adjustment, send adjusted frame duration time to peer. */
    sensor_util_post_intramode_event(module, event->identity,
      bundle_info->s_bundle->peer_identity,
      MCT_EVENT_MODULE_SENSOR_ADJUST_FPS,
      &dual_exp->adjusted_frame_duration);

  } else {

    /* Adjust FPS of slave sensor based on preview sensor*/
    if(s_bundle->dualc_info.streaming_master != CAM_MODE_PRIMARY &&
     s_bundle->dualc_info.fps_match_enabled == TRUE)
    {
      if (s_bundle->dualc_info.reset_correction == TRUE) {

        SHIGH("Phase correction in progress, do nothing in this frame");
        s_bundle->dualc_info.reset_correction = FALSE;

      } else {

        SLOW("Adjusting the fll of secondary sensor");
        SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
          SENSOR_SET_CUR_FLL,
          dual_exp, rc);
      }
    }

    /* Send AEC update to sleeping sensor */
    if(s_bundle->dualc_info.sensor_sleeping == TRUE) {
      SLOW("session %d : sensor sleeping write aec update",
        s_bundle->sensor_info->session_id);

      SENSOR_SUB_MODULE_PROCESS_EVENT(s_bundle, SUB_MODULE_SENSOR,
        SENSOR_SET_AEC_UPDATE_FOR_SLAVE, dual_exp, rc);
      if (rc < 0) {
        SERR("failed");
        return FALSE;
      }

    } else {
      SLOW("session %d : sensor awake do nothing",
        s_bundle->sensor_info->session_id);
    }
  }
  return ret;
}


/*===========================================================================
 * FUNCTION    - module_sensor_dualcamera_link_sessions -
 *
 * DESCRIPTION: This function is invoked when dual camera sessions have to be
 *              linked. It stores all the link parameters related to the link.
 *==========================================================================*/
boolean module_sensor_dualcamera_link_sessions(mct_module_t *module,
mct_event_t *event, sensor_bundle_info_t *bundle_info)
{

  boolean ret = TRUE;
  int32_t rc  = SENSOR_SUCCESS;
  mct_port_t  *this_port;
  mct_port_t  *peer_port;
  uint32_t    peer_identity;
  module_sensor_params_t* module_sensor_params = NULL;
  module_sensor_ctrl_t*   module_ctrl = NULL;
  cam_sync_related_sensors_event_info_t *link_param;

  if (!event || !module || !bundle_info) {
    SERR("failed");
    return FALSE;
  }

  module_sensor_params =
    bundle_info->s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  link_param = (cam_sync_related_sensors_event_info_t *)
    (event->u.ctrl_event.control_event_data);
  peer_identity = link_param->related_sensor_session_id;
  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  RETURN_ON_NULL(module_ctrl);

  SLOW("MCT_EVENT_CONTROL_LINK_INTRA_SESSION to session %x", peer_identity);

  ret = sensor_util_link_intra_session(module, event, peer_identity,
    &this_port, &peer_port);
  if (ret == FALSE) {

    SERR("FAIL to link sessions");

  } else {

    bundle_info->s_bundle->peer_identity = peer_identity;
    MCT_PORT_INTRALINKFUNC(this_port)(peer_identity, this_port, peer_port);

    REPORT_ON_FALSE(sensor_util_post_event_on_src_port(module, event), ret);

    module_ctrl->is_dual_cam = TRUE;
    bundle_info->s_bundle->dualc_info.dual_mode = link_param->mode;
    bundle_info->s_bundle->dualc_info.streaming_master = link_param->mode;
    bundle_info->s_bundle->dualc_info.sync_mechanism = link_param->sync_mechanism;

    if(bundle_info->s_bundle->dualc_info.sync_mechanism != CAM_SYNC_NO_SYNC)
      bundle_info->s_bundle->dualc_info.fps_match_enabled = TRUE;

    if(bundle_info->s_bundle->dualc_info.sync_mechanism == CAM_SYNC_HW_SYNC ||
       bundle_info->s_bundle->dualc_info.sync_mechanism == CAM_SYNC_HYBRID_SYNC){
        rc = module_sensor_params->func_tbl.process(
          module_sensor_params->sub_module_private,
          SENSOR_SET_DUALCAM_SETTINGS,
          (void*)&bundle_info->s_bundle->dualc_info.dual_mode);

        if(rc < 0)
            SERR("SENSOR_SET_DUALCAM_SETTINGS failed");

    } else {
        SLOW("MCT_EVENT_CONTROL_LINK_INTRA_SESSION HW/Hybrid Sync disabled");
    }

    /* Call send_event to propagate event to next module*/
    ret = sensor_util_post_event_on_src_port(module, event);
    if (ret == FALSE)
      SERR("failed");

  }

  return ret;
}

/*===========================================================================
 * FUNCTION    - module_sensor_dualcamera_hw_wakeup -
 *
 * DESCRIPTION: This function is invoked when sleeping sensor has to wake up.
 *==========================================================================*/
boolean module_sensor_dualcamera_hw_wakeup(mct_module_t *module,
mct_event_t *event, sensor_bundle_info_t *bundle_info)
{
  boolean ret = TRUE;
  module_sensor_ctrl_t* module_ctrl = NULL;
  cam_dual_camera_perf_control_t* perf_ctrl;

  if (!event || !module || !bundle_info) {
    SERR("failed");
    return FALSE;
  }

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  RETURN_ON_NULL(module_ctrl);

  perf_ctrl = (cam_dual_camera_perf_control_t*)
    event->u.ctrl_event.control_event_data;
  RETURN_ON_NULL(perf_ctrl);

  if ((perf_ctrl->enable == FALSE && module_ctrl->is_dual_cam == TRUE) ||
    (module_ctrl->is_dual_cam == FALSE)) {
    /* Call send_event to propagate event to next module*/
    ret = sensor_util_post_event_on_src_port(module, event);
    if (ret == FALSE)
      SERR("failed");

    sensor_util_post_submod_event(bundle_info->s_bundle,SUB_MODULE_SENSOR,
      SENSOR_START_STREAM, NULL);
    bundle_info->s_bundle->dualc_info.sensor_sleeping = FALSE;

    if(bundle_info->s_bundle->dualc_info.sync_mechanism != CAM_SYNC_NO_SYNC)
      bundle_info->s_bundle->dualc_info.fps_match_enabled = TRUE;

    SLOW("[dual] MCT_EVENT_CONTROL_HW_WAKEUP session %d sensor_sleeping %d",
     bundle_info->s_bundle->sensor_info->session_id,
     bundle_info->s_bundle->dualc_info.sensor_sleeping);


  }

  return ret;
}

/*===========================================================================
 * FUNCTION    - module_sensor_dualcamera_hw_sleep -
 *
 * DESCRIPTION: This function is invoked when sensor needs to go to sleep.
 *              CAM_PERF_SENSOR_SUSPEND - this will suspend the sensor hw and
 *              no frames will be output from sensor hw.
 *              CAM_PERF_ISPIF_FRAME_SKIP - no action at sensor, ISP will drop
 *              frames depending on how many frames are configured to be
 *              dropped.
 *              CAM_PERF_ISPIF_FRAME_DROP- no action at sensor, ISP will drop
 *              frames all frames from this sensor.
 *              CAM_PERF_STATS_FPS_CONTROL - sensors will run in non-sync mode
 *              driven by 3A stats; sensor fps matching and phase correction
 *              will be disabled in this mode.
 *==========================================================================*/
boolean module_sensor_dualcamera_hw_sleep(mct_module_t *module,
mct_event_t *event, sensor_bundle_info_t *bundle_info)
{
  boolean ret = TRUE;
  cam_dual_camera_perf_control_t* perf_ctrl;
  module_sensor_ctrl_t* module_ctrl = NULL;

  if (!event || !module || !bundle_info) {
    SERR("failed");
    return FALSE;
  }

  module_ctrl = (module_sensor_ctrl_t *)module->module_private;
  RETURN_ON_NULL(module_ctrl);

  perf_ctrl = (cam_dual_camera_perf_control_t*)
    event->u.ctrl_event.control_event_data;
  RETURN_ON_NULL(perf_ctrl);

  /* Mark this session is sleeping now */
  bundle_info->s_bundle->dualc_info.sensor_sleeping = TRUE;

  /* Reset phase correction parameters for sleeping sensor */
  module_sensor_dualcamera_reset_pc_params(bundle_info->s_bundle);

  SLOW("[dual] MCT_EVENT_CONTROL_HW_SLEEP session %d sensor_sleeping %d",
    bundle_info->s_bundle->sensor_info->session_id,
    bundle_info->s_bundle->dualc_info.sensor_sleeping);

  if ((perf_ctrl->enable == TRUE && module_ctrl->is_dual_cam == TRUE &&
    bundle_info->s_bundle->dualc_info.dual_mode != CAM_MODE_PRIMARY) ||
    (module_ctrl->is_dual_cam == FALSE)) {

    switch(perf_ctrl->perf_mode){

      case CAM_PERF_SENSOR_SUSPEND: {
        SLOW("CAM_PERF_SENSOR_SUSPEND");
        sensor_util_post_submod_event(bundle_info->s_bundle,
          SUB_MODULE_SENSOR, SENSOR_STANDBY_STREAM, NULL);
      }
      break;
      case CAM_PERF_ISPIF_FRAME_DROP:
        SLOW("CAM_PERF_ISPIF_FRAME_DROP - no action at the sensor");
      break;
      case CAM_PERF_ISPIF_FRAME_SKIP:
        SLOW("CAM_PERF_ISPIF_FRAME_SKIP - no action at the sensor");
      break;
      case CAM_PERF_STATS_FPS_CONTROL: {
        SLOW("CAM_PERF_STATS_FPS_CONTROL - disabling fps match and phase correction");

        /* Disable FPS matching in sensor */
        bundle_info->s_bundle->dualc_info.fps_match_enabled = FALSE;

        /* Reset phase correction params */
        module_sensor_dualcamera_reset_pc_params(bundle_info->s_bundle);
      }
      break;
      default:
        SLOW("Invalid sleep type %d  - no action at the sensor",
        perf_ctrl->perf_mode);
      break;
    }
  }

  /* Call send_event to propagate event to next module*/
  ret = sensor_util_post_event_on_src_port(module, event);
  if (ret == FALSE)
    SERR("failed");

  return ret;
}


/*===========================================================================
 * FUNCTION    - module_sensor_dualcamera_handle_master_info -
 *
 * DESCRIPTION: This function handles role switch between streaming master and
 *              slave.
 *==========================================================================*/
boolean module_sensor_dualcamera_handle_master_info(mct_module_t *module,
mct_event_t *event, sensor_bundle_info_t *bundle_info)
{
  boolean ret = TRUE;
  cam_dual_camera_master_info_t* p_dual_master_info = NULL;

  if (!event || !module || !bundle_info) {
    SERR("failed");
    return FALSE;
  }

  p_dual_master_info =
    (cam_dual_camera_master_info_t *)(event->u.ctrl_event.control_event_data);

  if(p_dual_master_info)
      bundle_info->s_bundle->dualc_info.streaming_master = p_dual_master_info->mode;

  SLOW("MCT_EVENT_CONTROL_MASTER_INFO session %d, role switched to %d",
    bundle_info->s_bundle->sensor_info->session_id,
    bundle_info->s_bundle->dualc_info.streaming_master);

  /* Call send_event to propagate event to next module*/
  ret = sensor_util_post_event_on_src_port(module, event);
  if (ret == FALSE)
   SERR("failed");

  return ret;
}
