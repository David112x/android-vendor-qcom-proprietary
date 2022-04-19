/* aec_port_ext.c
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "aec_port.h"
#include "aec_ext.h"
#include "aec_ext_test_wrapper.h"
#include "q3a_thread_ext.h"

/** aec_port_ext_update_opaque_input_params
 *
 *  @aec_port: port information
 *  @type: type of the parameter
 *  @data: payload
 *
 *  Package custom parameter inside opaque payload.
 *
 **/
static boolean aec_port_ext_update_opaque_input_params(
  aec_port_private_t *aec_port,
  int16_t type,
  q3a_custom_data_t *payload)
{
  boolean rc = FALSE;

  if (!payload->size) {
    AEC_ERR("Error: payload size zero");
    return rc;
  }

  /* Pass directly as set param call */
  aec_set_parameter_t *set_parm;
  q3a_custom_data_t input_param;
  STATS_MEMSET(&input_param, 0, sizeof(input_param));

  q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET, type,
    aec_port);
  if (NULL == aec_msg) {
    AEC_ERR("Memory allocation failure!");
    return rc;
  }

  /* populate aec message to post to thread */
  set_parm = &aec_msg->u.aec_set_parm;
  input_param.size = payload->size;
  input_param.data = malloc(payload->size);
  if (NULL == input_param.data) {
    AEC_ERR("Failure allocating memory to store data!");
    free(aec_msg);
    return rc;
  }
  STATS_MEMCPY(input_param.data, input_param.size, payload->data, payload->size);
  set_parm->u.aec_custom_data = input_param;
  rc = q3a_aecawb_thread_en_q_msg(aec_port->thread_data, aec_msg);

  return rc;
}

/** aec_port_ext_msg_reset_led_est
 *
 * Handling of Led specific operation
 **/
static boolean aec_port_ext_msg_reset_led_est(aec_port_private_t *private)
{
  boolean rc = FALSE;

  q3a_thread_aecawb_msg_t *aec_msg =
    (q3a_thread_aecawb_msg_t *)malloc(sizeof(q3a_thread_aecawb_msg_t));
  if (aec_msg == NULL) {
    AEC_ERR("Failure allocating memory to store data!");
    return rc;
  }
  memset(aec_msg, 0, sizeof(q3a_thread_aecawb_msg_t));
  aec_msg->type = MSG_AEC_SET;
  aec_msg->u.aec_set_parm.type = AEC_SET_PARAM_RESET_LED_EST;
  rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);

  return rc;
}

/** aec_port_ext_msg_led_reset
 *
 * Handling of Led specific operation
 **/
static boolean aec_port_ext_msg_led_reset(aec_port_private_t *private)
{
  boolean rc = FALSE;

  q3a_thread_aecawb_msg_t *aec_msg =
    (q3a_thread_aecawb_msg_t *)malloc(sizeof(q3a_thread_aecawb_msg_t));
  if (aec_msg == NULL) {
    AEC_ERR("Failure allocating memory to store data!");
    return rc;
  }
  memset(aec_msg, 0, sizeof(q3a_thread_aecawb_msg_t));
  aec_msg->type = MSG_AEC_SET;
  aec_msg->u.aec_set_parm.type = AEC_SET_PARAM_LED_RESET;
  rc = q3a_aecawb_thread_en_q_msg(private->thread_data, aec_msg);

  AEC_HIGH("AEC_SET_PARAM_LED_RESET");

  return rc;
}

/** aec_port_ext_handle_set_parm_aec_event
 *
 *  @port: port information
 *  @evt_param: set parameter received.
 *
 *  Extend handling of AEC specific set parameters as filtered
 *  by stats port.
*/
static stats_ext_return_type aec_port_ext_handle_set_parm_aec_event(
  mct_port_t *port,
  void *evt_param)
{
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  boolean ret = TRUE;
  aec_port_private_t  *aec_port = (aec_port_private_t *)(port->port_private);
  aec_set_parameter_t *param = (aec_set_parameter_t *)evt_param;
  aec_ext_param_t *ext_param = aec_port->ext_param;

  /* Handle other set parameters here if required to extend. */
  switch (param->type) {
  case AEC_SET_PARAM_LED_MODE: {
    ext_param->led_mode = param->u.led_mode;
  }
    break;
  case AEC_SET_PARAM_LOCK: {
    ext_param->lock = param->u.aec_lock;
    AEC_HIGH("AEC_SET_PARAM_LOCK aec_lock[%d]", ext_param->lock);
  }
    break;
  default:
    break;
  }

  return rc;
}

/** aec_port_ext_handle_set_parm_af_event
 *
 *  @port: port information
 *  @evt_param: set parameter received.
 *
 *  Extend handling of AEC specific set parameters as filtered
 *  by stats port.
*/
static stats_ext_return_type aec_port_ext_handle_set_parm_af_event(
  mct_port_t *port,
  void *evt_param)
{
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  boolean ret = TRUE;
  aec_port_private_t  *aec_port = (aec_port_private_t *)(port->port_private);
  af_set_parameter_t *param = (af_set_parameter_t *)evt_param;
  aec_ext_param_t *ext_param = aec_port->ext_param;

  /* Handle other set parameters here if required to extend. */
  switch (param->type) {
  case AF_SET_PARAM_CANCEL_FOCUS:
    AEC_LOW("AF_SET_PARAM_CANCEL_FOCUS led_state:%d use_led_est:%d led_mode:%d led_needed:%d lls_flag:%d",
        ext_param->led_state,
        ext_param->use_led_est,
        ext_param->led_mode,
        ext_param->led_needed,
        ext_param->lls_flag);

    if (IS_LED_MODE_ON(ext_param) ||
      (IS_LED_MODE_AUTO(ext_param) && IS_LED_NEEDED(ext_param))) {
      if (!ext_param->lock || IS_LED_RESET_NEEDED(ext_param)) {
        ret = aec_port_ext_msg_led_reset(aec_port);
        if (TRUE == ret) {
          ret = aec_port_ext_msg_reset_led_est(aec_port);
        }
        if (FALSE == ret) {
          AEC_ERR("Failure handling reset led est!");
          rc = STATS_EXT_HANDLING_FAILURE;
        }
      }
    }
    AEC_HIGH("AF_SET_PARAM_CANCEL_FOCUS");
    break;

  default:
    break;
  }

  return rc;
}

/** aec_port_ext_handle_set_parm_common_event
 *
 *  @port: port information
 *  @param: set parameter received.
 *
 *  Extend handling of set parameter call of type common.
 *
 **/
static stats_ext_return_type aec_port_ext_handle_set_parm_common_event(
  mct_port_t *port,
  void *evt_param)
{
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  boolean ret = TRUE;
  aec_port_private_t  *aec_port = (aec_port_private_t *)(port->port_private);
  stats_common_set_parameter_t *param =
      (stats_common_set_parameter_t *)evt_param;
  q3a_custom_data_t payload;

  switch (param->type) {
  case COMMON_SET_PARAM_CUSTOM: {
    /* For custom parameters from HAL, we'll save in a list and then
       send to core algorithm every frame at once during stats
       trigger. */
    payload.data = param->u.custom_param.data;
    payload.size = param->u.custom_param.size;
    ret = aec_port_ext_update_opaque_input_params(aec_port,
      AEC_SET_PARM_CUSTOM_EVT_HAL, &payload);
    if (FALSE == ret) {
      AEC_ERR("Failure handling the custom parameter!");
      rc = STATS_EXT_HANDLING_FAILURE;
    }
  }
    break;
  case COMMON_SET_PARAM_BESTSHOT: {
    /* Sample on how to handle existing HAL events */
    q3a_thread_aecawb_msg_t *aec_msg = aec_port_create_msg(MSG_AEC_SET,
      AEC_SET_PARAM_BESTSHOT, aec_port);
    if (NULL == aec_msg) {
      AEC_ERR("Memory allocation failure!");
      ret = FALSE;
    } else {
      /* populate aec message to post to thread */
      /* This value is usually map at port to convert from HAL to algo enum types,
       * in this case passing the value directly since port doesn't know the
       * required mapping for custom algo */
      /* 3rd party could do the mapping here if prefered*/
      aec_msg->u.aec_set_parm.u.bestshot_mode = (aec_bestshot_mode_type_t)param->u.bestshot_mode;

      ret = q3a_aecawb_thread_en_q_msg(aec_port->thread_data, aec_msg);
      if (!ret) {
        AEC_ERR("Fail to queue msg");
      }
    }
    /* Marking as complete since no further processing required */
    rc = STATS_EXT_HANDLING_COMPLETE;
  }
    break;

  default: {
    rc = STATS_EXT_HANDLING_PARTIAL;
  }
    break;
  }

  return rc;
}


/** aec_port_ext_handle_set_parm_event
 *
 *  @port: port information
 *  @param: set parameter received.
 *
 *  Extend handling of set parameter call from HAL.
 *
 **/
static stats_ext_return_type aec_port_ext_handle_set_parm_event(
  mct_port_t *port,
  void *evt_param)
{
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  boolean ret = TRUE;
  aec_port_private_t  *aec_port = (aec_port_private_t *)(port->port_private);
  stats_set_params_type *stat_parm = (stats_set_params_type *)evt_param;

  if (!evt_param || !aec_port) {
    AEC_ERR("Invalid parameters!");
    return FALSE;
  }

  AEC_LOW("Extended handling set param event of type: %d", stat_parm->param_type);
  /* These cases can be extended to handle other parameters here if required. */
  if (stat_parm->param_type == STATS_SET_COMMON_PARAM) {
    rc = aec_port_ext_handle_set_parm_common_event(
      port, (void *)&(stat_parm->u.common_param));
  } else {
    /* Handle aec specific set parameters here if different from aec port handling */
    if (stat_parm->param_type == STATS_SET_Q3A_PARAM) {
      q3a_set_params_type  *q3a_param = &(stat_parm->u.q3a_param);
      if (q3a_param->type == Q3A_SET_AEC_PARAM) {
        rc = aec_port_ext_handle_set_parm_aec_event(
          port, (void *)&q3a_param->u.aec_param);
      } else if (q3a_param->type == Q3A_SET_AF_PARAM) {
        rc = aec_port_ext_handle_set_parm_af_event(
          port, (void *)&q3a_param->u.af_param);
      }
    }
  }

  return rc;
} /* aec_port_ext_handle_set_parm_event */


/** aec_port_ext_handle_control_event:
 *    @port: port info
 *    @ctrl_evt: control event
 *
 * Extension of control event handling. Here OEM can further
 * handle/process control events. The control events can be OEM
 * specific or general. If it's OEM specific, OEM can either
 * process it here and send to core algorithm if required; or
 * just send the payload to core algorithm to process.
 *
 * Return stats_ext_return_type value.
 */
static stats_ext_return_type aec_port_ext_handle_control_event(
  mct_port_t *port,
  mct_event_control_t *ctrl_evt)
{
  aec_port_private_t *aec_port =
    (aec_port_private_t *)(port->port_private);
  q3a_custom_data_t payload;
  mct_custom_data_payload_t *cam_custom_ctr =
    (mct_custom_data_payload_t *)ctrl_evt->control_event_data;
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;

  switch (ctrl_evt->type) {
  case MCT_EVENT_CONTROL_CUSTOM: {
    boolean ret = FALSE;
    payload.data = cam_custom_ctr->data;
    payload.size = cam_custom_ctr->size;
    ret = aec_port_ext_update_opaque_input_params(aec_port,
      AEC_SET_PARM_CUSTOM_EVT_CTRL, &payload);
    if (FALSE == ret) {
      AEC_ERR("Failure handling the custom parameter!");
      rc = STATS_EXT_HANDLING_FAILURE;
    } else {
      rc = STATS_EXT_HANDLING_COMPLETE;
    }
  }
    break;
  case MCT_EVENT_CONTROL_SET_PARM: {
    rc = aec_port_ext_handle_set_parm_event(port,
      ctrl_evt->control_event_data);
  }
    break;
  case MCT_EVENT_CONTROL_STREAMON: {
    rc = STATS_EXT_HANDLING_PARTIAL;
  }
    break;
  case MCT_EVENT_CONTROL_LINK_INTRA_SESSION: {
    boolean ret = FALSE;
    boolean is_thread_created = FALSE;
    boolean set_thread_obj_data = FALSE;
    rc = STATS_EXT_HANDLING_PARTIAL;

    q3a_thread_ext_aecawb_initialize_register();

    do {
      AEC_HIGH("Link request session_id: %u, aec_port: %p, [mode:%d camRole:%d 3AMode:%d]",
        GET_SESSION_ID(aec_port->reserved_id),
        aec_port,
        aec_port->dual_cam_info.mode,
        aec_port->dual_cam_info.cam_role,
        aec_port->dual_cam_info.sync_3a_mode);
      ret = q3a_thread_ext_aecawb_create_single_thread(port);
      if (FALSE == ret) {
        AEC_ERR("Fail to create ext aecawb thread");
        break;
      }
      is_thread_created = TRUE;

      ret = q3a_thread_ext_aecawb_get_thread_obj(port);
      if (FALSE == ret) {
        AEC_ERR("Fail to get AEC or AWB obj data");
        break;
      }
      ret = q3a_thread_ext_aecawb_set_thread_obj(port);
      if (FALSE == ret) {
        AEC_ERR("Fail to set AEC or AWB obj data");
        break;
      }
      set_thread_obj_data = TRUE;
    } while (0);

    if (FALSE == ret) {
      /* Handle error */
      AEC_ERR("Error handling: fail to link: %p", aec_port);
      if (set_thread_obj_data) {
        q3a_thread_ext_aecawb_release_aecawb_obj_ref(port);
      }
      if (is_thread_created) {
        q3a_thread_ext_aecawb_destroy_single_thread();
      }

      aec_port->dual_cam_info.mode                  = CAM_MODE_PRIMARY;
      aec_port->dual_cam_info.sync_3a_mode          = CAM_3A_SYNC_NONE;
      aec_port->dual_cam_info.is_LPM_on             = FALSE;
      aec_port->dual_cam_info.is_aux_sync_enabled   = FALSE;
      aec_port->dual_cam_info.is_aux_update_enabled = FALSE;
      aec_port->dual_cam_info.intra_peer_id         = 0;
      rc = STATS_EXT_HANDLING_FAILURE;
    }
    AEC_HIGH("Link done, session_id: %u, thread: %p",
      GET_SESSION_ID(aec_port->reserved_id), aec_port->thread_data);
  }
    break;
  case MCT_EVENT_CONTROL_UNLINK_INTRA_SESSION: {
    AEC_HIGH("Un-link session session_id: %d, aec_port: %p, link type: %d, [mode:%d camRole:%d 3AMode:%d]",
      GET_SESSION_ID(aec_port->reserved_id), aec_port,
      aec_port->dual_cam_info.mode,
      aec_port->dual_cam_info.cam_role,
      aec_port->dual_cam_info.sync_3a_mode);
    q3a_thread_ext_aecawb_release_aecawb_obj_ref(port);
    q3a_thread_ext_aecawb_destroy_single_thread();

    rc = STATS_EXT_HANDLING_PARTIAL;
  }
    break;

  default: {
    rc = STATS_EXT_HANDLING_FAILURE;
  }
    break;
  }

  return rc;
} /* aec_port_ext_handle_control_event */


static stats_ext_return_type aec_port_ext_intra_event(
  mct_port_t *port, mct_event_t *event)
{
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  aec_port_private_t *private;
  AEC_LOW("Received AEC intra-module peer event");

  /* sanity check */
  if (!port || !port->port_private || !event) {
    return FALSE;
  }

  private = (aec_port_private_t *)(port->port_private);

  switch(event->u.module_event.type) {
    default:
      rc = STATS_EXT_HANDLING_PARTIAL;
      break;
  }

  return rc;
}

/** aec_port_ext_handle_module_event:
 *    @port: port info
 *    @ctrl_evt: control event
 *
 * Extension of module event handling. Here OEM can further
 * handle/process module events. The module events can be OEM
 * specific or general. If it's OEM specific, OEM can either
 * process it here and send to core algorithm if required; or
 * just send the payload to core algorithm to process.
 *
 * Return one of the return type defined by
 * stats_ext_return_type
 **/
static stats_ext_return_type aec_port_ext_handle_module_event(
  mct_port_t *port,
  mct_event_module_t *mod_evt)
{
  q3a_custom_data_t payload;
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;

  aec_port_private_t *aec_port =
    (aec_port_private_t *)(port->port_private);
  mct_custom_data_payload_t *cam_custom_evt =
    (mct_custom_data_payload_t *)mod_evt->module_event_data;

  AEC_LOW("Handle AEC module event of type: %d", mod_evt->type);

  switch (mod_evt->type) {

  case MCT_EVENT_MODULE_CUSTOM_STATS_DATA_AEC: {
    boolean ret = FALSE;
    payload.data = cam_custom_evt->data;
    payload.size = cam_custom_evt->size;
    rc = aec_port_ext_update_opaque_input_params(aec_port,
      AEC_SET_PARM_CUSTOM_EVT_MOD, &payload);
    if (FALSE == ret) {
      AEC_ERR("Failure handling the custom parameter!");
      rc = STATS_EXT_HANDLING_FAILURE;
    } else {
      rc = STATS_EXT_HANDLING_COMPLETE;
    }
  }
    break;

  case MCT_EVENT_MODULE_CUSTOM: {
    boolean ret = FALSE;
    payload.data = cam_custom_evt->data;
    payload.size = cam_custom_evt->size;
    ret = aec_port_ext_update_opaque_input_params(aec_port,
      AEC_SET_PARM_CUSTOM_EVT_MOD, &payload);
    if (FALSE == ret) {
      AEC_ERR("Failure handling the custom parameter!");
      rc = STATS_EXT_HANDLING_FAILURE;
    } else {
      rc = STATS_EXT_HANDLING_COMPLETE;
    }
  }
    break;

  case MCT_EVENT_MODULE_STATS_AECAWB_GET_THREAD_OBJECT: {
    q3a_thread_aecawb_get_port_data_t *aecawb_get_data =
      (q3a_thread_aecawb_get_port_data_t *)(mod_evt->module_event_data);

    /* need it for dual cameras sharing thread, to backup original thread_data */
    aecawb_get_data->camera_id   = aec_port->camera_id;
    aecawb_get_data->thread_data = aec_port->thread_data;

    rc = STATS_EXT_HANDLING_PARTIAL;
  }
    break;

  case MCT_EVENT_MODULE_STATS_AECAWB_SET_THREAD_OBJECT: {
    q3a_thread_aecawb_set_port_data_t *aecawb_set_data =
      (q3a_thread_aecawb_set_port_data_t *)(mod_evt->module_event_data);

    /* need it for dual cameras sharing thread, set the algo instance to be use*/
    if (aecawb_set_data->aec_algo_instance) {
      aec_port->aec_object.aec  = aecawb_set_data->aec_algo_instance;
      AEC_HIGH("AEC: session_id: %d: set camera_id: %d, set algo: %p",
        GET_SESSION_ID(aec_port->reserved_id), aec_port->camera_id,
        aec_port->aec_object.aec);
    }

    rc = STATS_EXT_HANDLING_PARTIAL;
  }
    break;

  case MCT_EVENT_MODULE_STATS_AECAWB_RESTORE_EXT_THREAD_OBJECT: {
    q3a_thread_restore_thread_data_t *restore_obj =
      (q3a_thread_restore_thread_data_t *)(mod_evt->module_event_data);

    AEC_HIGH("AEC port (%p): camera_id: %u & algo %p, restore to camera_id: %u thread data %p algo %p",
      aec_port, aec_port->camera_id, aec_port->aec_object.aec,
      restore_obj->camera_id, restore_obj->q3a_thread_data,
      restore_obj->aec_algo_instance);

    if (NULL == restore_obj) {
      AEC_ERR("Restore obj NULL");
      return FALSE;
    }

    aec_port->camera_id = restore_obj->camera_id;
    aec_port->thread_data = restore_obj->q3a_thread_data;
    aec_port->aec_object.aec = restore_obj->aec_algo_instance;

    rc = STATS_EXT_HANDLING_COMPLETE;
  }
    break;

  case MCT_EVENT_MODULE_START_STOP_STATS_THREADS: {
    uint8_t *start_flag = (uint8_t*)(mod_evt->module_event_data);
    AEC_HIGH("MCT_EVENT_MODULE_START_STOP_STATS_THREADS: start_flag: %d, [mode:%d camRole:%d 3AMode:%d], aec_port: %p",
      *start_flag,
      aec_port->dual_cam_info.mode,
      aec_port->dual_cam_info.cam_role,
      aec_port->dual_cam_info.sync_3a_mode,
      aec_port);
    if (TRUE == *start_flag) {
      AEC_HIGH("Do nothing");
      break;
    }

    if (CAM_3A_SYNC_NONE == aec_port->dual_cam_info.sync_3a_mode) {
      AEC_HIGH("MCT_EVENT_MODULE_START_STOP_STATS_THREADS: in CAM_3A_SYNC_NONE already, nothing to release!!");
      break;
    }

    /* Work around: not getting event from MCT to unlink do it here */
    AEC_HIGH("Unlink and destroy AEC ext thread");
    q3a_thread_ext_aecawb_release_aecawb_obj_ref(port);
    q3a_thread_ext_aecawb_destroy_single_thread();

    aec_port->dual_cam_info.mode                  = CAM_MODE_PRIMARY;
    aec_port->dual_cam_info.sync_3a_mode          = CAM_3A_SYNC_NONE;
    aec_port->dual_cam_info.is_LPM_on             = FALSE;
    aec_port->dual_cam_info.is_aux_sync_enabled   = FALSE;
    aec_port->dual_cam_info.is_aux_update_enabled = FALSE;
    aec_port->dual_cam_info.intra_peer_id         = 0;
    AEC_HIGH("AEC unlink done");

    rc = STATS_EXT_HANDLING_COMPLETE;
  }
    break;
  default: {
    AEC_LOW("Default. no action!");
  }
    break;
  }

  return rc;
} /* aec_port_ext_handle_module_event */


/** aec_port_ext_callback:
 *    @port: port info
 *    @aec_out: Output parameters from core algorithm
 *    @update: AEC updates to be sent to other modules.
 *    @output_handled: mask of output type handled here.
 *
 * Extension of AEC core callback. Here OEM can process output
 * parameters received from AEC core. There might be OEM specific
 * parameters as well as standard output which OEM might want to
 * handle on its own.
 *
 * Return one of the return type defined by
 * stats_ext_return_type
 **/
static stats_ext_return_type aec_port_ext_callback(
  mct_port_t *port,
  void *core_out,
  void *update)
{
  aec_port_private_t *private = (aec_port_private_t *)(port->port_private);
  aec_update_t *aec_update = (aec_update_t *)update;
  aec_output_data_t *aec_out = (aec_output_data_t *)core_out;
  stats_ext_return_type ret = STATS_EXT_HANDLING_COMPLETE;

  /* handle custom params */
  /* Send the custom parameters as it is. Receiver will handle as required. */
  if (NULL == aec_update) {
    aec_update = &aec_out->stats_update.aec_update;
  }
  aec_update->aec_custom_param_update.data = aec_out->aec_custom_param.data;
  aec_update->aec_custom_param_update.size = aec_out->aec_custom_param.size;

  if (CAM_3A_SYNC_NONE != private->dual_cam_info.sync_3a_mode) {
    /* Return now to avoid any cb processing while using QC algo & extension.
       Feel free to remove this and continue handling cb here */
    ret = STATS_EXT_HANDLING_PARTIAL;
    return ret;
  }
  /* handle other parameters if required. */
  if (AEC_UPDATE == aec_out->type) {
    MCT_OBJECT_LOCK(port);
    private->aec_update_flag = TRUE;
    MCT_OBJECT_UNLOCK(port);
    ret = STATS_EXT_HANDLING_COMPLETE;

  } else if (AEC_SEND_EVENT == aec_out->type) {
    aec_ext_param_t *ext_param = private->ext_param;

    aec_port_pack_exif_info(port, aec_out);
    aec_port_pack_update(port, aec_out, 0);

    AEC_LOW("origin: led_mode:%d led_needed:%d flash_needed:%d use_led_est:%d est_state:%d touch_ev_status:%d aec_settled:%d",
        ext_param->led_mode,
        aec_out->stats_update.aec_update.led_needed,
        aec_out->stats_update.aec_update.flash_needed,
        aec_out->stats_update.aec_update.use_led_estimation,
        aec_out->stats_update.aec_update.est_state,
        aec_out->stats_update.aec_update.touch_ev_status,
        aec_out->stats_update.aec_update.settled);

    ext_param->led_state = aec_out->stats_update.aec_update.est_state;
    ext_param->use_led_est = aec_out->stats_update.aec_update.use_led_estimation;
    ext_param->led_needed = aec_out->stats_update.aec_update.led_needed;
    ext_param->lls_flag = aec_out->stats_update.aec_update.low_light_shutter_flag;
    ext_param->led_mode = aec_out->stats_update.aec_update.led_mode;

    /* For triggering Flash during preflash */
    if (ext_param->led_mode == CAM_FLASH_MODE_ON)
      aec_out->stats_update.aec_update.led_needed = 1;

    /* For triggering AEC during preflash */
    if (aec_out->stats_update.aec_update.use_led_estimation)
      aec_out->stats_update.aec_update.settled = 1;
    aec_port_print_log(aec_out, "CB-EXT_AEC_UP", private, -1);
    aec_port_send_event(port, MCT_EVENT_MODULE_EVENT,
      MCT_EVENT_MODULE_STATS_AEC_UPDATE,
      (void *)(&aec_out->stats_update),aec_out->stats_update.aec_update.sof_id);
    if (aec_out->need_config && private->fast_aec_data.enable == FALSE) {
      aec_port_configure_stats(aec_out, port);
      aec_out->need_config = 0;
    }
    ret = STATS_EXT_HANDLING_COMPLETE;
  }

  return ret;
}


/** aec_port_ext_init:
 *    @port: port info
 *    @session_id: current session id
 *
 * Extension of AEC port init.
 *
 * Return one of the return type defined by
 * stats_ext_return_type
 **/
static stats_ext_return_type aec_port_ext_init(
  mct_port_t *port,
  unsigned int session_id)
{
  (void)session_id;
  aec_port_private_t *private = (aec_port_private_t *)(port->port_private);
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  aec_ext_param_t *ext_param = NULL;

  ext_param = (aec_ext_param_t *) calloc(1, sizeof(aec_ext_param_t));
  if (NULL == ext_param) {
    AEC_ERR("Error allocating memory for extension init!");
    rc = STATS_EXT_HANDLING_FAILURE;
  } else {
    /* Output of core algorithm will have void pointer to point to custom output
       parameters. Assign space to save those parameters. */
    private->aec_object.aec_custom_param.data =
      (void *)&ext_param->custom_output;
    private->aec_object.aec_custom_param.size =
      sizeof(ext_param->custom_output);
    private->ext_param = ext_param;
  }

  return rc;
}

/** aec_port_ext_deinit:
 *    @port: port info
 *
 * Extension of AEC port de-init.
 *
 * Return: none
 **/
void aec_port_ext_deinit(mct_port_t *port)
{
  aec_port_private_t *private = (aec_port_private_t *)(port->port_private);
  if (private->ext_param) {
    free(private->ext_param);
  }
}

/** aec_port_ext_update_func_table:
 *    @ptr: pointer to internal aec pointer object
 *
 * Update extendable function pointers.
 *
 * Return: True on success
 **/
boolean aec_port_ext_update_func_table(void *ptr)
{
  aec_port_private_t *private = (aec_port_private_t *)ptr;
  private->func_tbl.ext_init = aec_port_ext_init;
  private->func_tbl.ext_deinit = aec_port_ext_deinit;
  private->func_tbl.ext_callback = aec_port_ext_callback;
  private->func_tbl.ext_handle_module_event = aec_port_ext_handle_module_event;
  private->func_tbl.ext_handle_control_event = aec_port_ext_handle_control_event;
  private->func_tbl.ext_handle_intra_event = aec_port_ext_intra_event;

  return TRUE;
}

/**
 * aec_port_ext_is_extension_required
 *
 * @aec_libptr: Pointer to the vendor library
 * @cam_position: Camera position
 * @use_default_algo: The decision to use or not default (QC) algo in extension is returned by this flag
 *
 * Return: TRUE is extension is required
 **/
boolean aec_port_ext_is_extension_required(void *aec_libptr,
  cam_position_t cam_position, boolean *use_default_algo)
{
  boolean rc = FALSE;
  (void)cam_position;
  *use_default_algo = FALSE;
  int test_dual_out = 0;
  if (aec_libptr) {
    rc = TRUE;
  } else {
#ifdef _AEC_EXTENSION_
    *use_default_algo = TRUE;
    rc = TRUE;
#endif
  }

  STATS_TEST_DUAL_OUTPUT(test_dual_out);
  if (test_dual_out) {
    /* use default algo */
    *use_default_algo = TRUE;
    /* Enable extension */
    rc =  TRUE;
  }

  return rc;
}

/**
 * aec_ext_load_function
 *
 * @aec_object: structure with function pointers to be assign
 * @aec_lib: Parameter to provide the pointer to the aec library (optional)
 * @cam_position: Camera position
 * @use_default_algo: Using or not default algo flag
 *
 *  This function is intended to be used by OEM.
 *  The OEM must use this fuction to populate the algo interface
 *  function pointers.
 *
 * Return: Handler to AEC interface
 **/
void * aec_port_ext_load_function(aec_object_t *aec_object, void *aec_lib,
  cam_position_t cam_position, boolean use_default_algo)
{
  void *aec_handle = NULL;
  int test_dual_out = 0;
  (void)aec_lib;
  (void)cam_position;

  if (use_default_algo) {
    AEC_HIGH("Load default algo functions");
    STATS_TEST_DUAL_OUTPUT(test_dual_out);
    if (test_dual_out) {
      aec_handle = aec_ext_test_wrapper_load_function(aec_object);
    } else {
      aec_handle = aec_port_load_function(aec_object);
    }
  } else {
    AEC_ERR("Error: This is a DUMMY function, used only for reference");
    aec_handle = NULL;
  }

  return aec_handle;
}

/** aec_port_ext_unload_function
 *
 *    @private: Port private structure
 *
 *  This function is intended to be used by OEM.
 *  The OEM must use this fuction to free resources allocated at
 *  aec_port_ext_load_function()
 *
 * Return: void
 **/
void aec_port_ext_unload_function(aec_port_private_t *private)
{
  int test_dual_out = 0;

  if (!private) {
    return;
  }

  if (private->use_default_algo) {
    STATS_TEST_DUAL_OUTPUT(test_dual_out);
    if (test_dual_out) {
      aec_ext_test_wrapper_unload_function(&private->aec_object, private->aec_iface_lib);
      aec_port_load_dummy_default_func(&private->aec_object);
      private->aec_iface_lib = NULL;
    } else {
      aec_port_unload_function(private);
    }
  } else {
    AEC_ERR("Error: This is a DUMMY function, used only for reference");
    /* Reset original value of interface */
    aec_port_load_dummy_default_func(&private->aec_object);
  }

  return;
}
