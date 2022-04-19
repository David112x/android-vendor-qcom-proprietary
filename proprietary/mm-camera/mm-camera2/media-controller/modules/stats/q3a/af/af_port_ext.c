/* af_port_ext.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "af_port.h"
#include "af_ext.h"
#include "af_ext_test_wrapper.h"
#include "q3a_thread_ext.h"


/** af_port_ext_update_opaque_input_params
 *
 *  @af_port: port information
 *  @type: type of the parameter
 *  @data: payload
 *
 *  Package custom parameter inside opaque payload.
 *
 **/
static boolean af_port_ext_update_opaque_input_params(
  af_port_private_t *af_port,
  int16_t type,
  q3a_custom_data_t *payload)
{
  boolean rc = FALSE;

  if (!payload->size) {
    AF_ERR("Error: payload size zero");
    return rc;
  }

  /* Pass directly as set param call */
  af_set_parameter_t *set_parm;
  q3a_custom_data_t input_param;
  memset(&input_param, 0, sizeof(input_param));
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    type, af_port);
  if (NULL == af_msg) {
    AF_ERR("Memory allocation failure!");
    return rc;
  }

  /* populate af message to post to thread */
  set_parm = &af_msg->u.af_set_parm;
  input_param.size = payload->size;
  input_param.data = malloc(payload->size);
  if (NULL == input_param.data) {
    AF_ERR("Fail to allocate memory to store data!");
    free(af_msg);
    return rc;
  }
  memcpy(input_param.data, payload->data, payload->size);
  set_parm->u.af_custom_data = input_param;
  rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  return rc;
}


/** af_port_ext_handle_set_parm_af_event
 *
 *  @port: port information
 *  @evt_param: set parameter received.
 *
 *  Extend handling of AF specific set parameters as filtered by
 *  stats port.
*/
static stats_ext_return_type af_port_ext_handle_set_parm_af_event(
  mct_port_t *port,
  void *evt_param)
{
  stats_ext_return_type ret = STATS_EXT_HANDLING_PARTIAL;
  boolean rc = TRUE;
  af_port_private_t  *af_port = (af_port_private_t *)(port->port_private);
  af_set_parameter_t *param = (af_set_parameter_t *)evt_param;

  /* Handle other set parameters here if required to extend. */
  return ret;
}


/** af_port_ext_handle_set_parm_common_event
 *
 *  @port: port information
 *  @param: set parameter received.
 *
 *  Extend handling of set parameter call of type common.
 *
 **/
static stats_ext_return_type af_port_ext_handle_set_parm_common_event(
  mct_port_t *port,
  void *evt_param)
{
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  boolean ret = TRUE;
  af_port_private_t  *af_port = (af_port_private_t *)(port->port_private);
  stats_common_set_parameter_t *param =
      (stats_common_set_parameter_t *)evt_param;
  q3a_custom_data_t payload;

  switch (param->type) {
  case COMMON_SET_PARAM_CUSTOM: {
    /* For custom parameters from HAL, we'll save in a list and then send
    to core algorithm every frame at once during stats trigger. */
    payload.data = param->u.custom_param.data;
    payload.size = param->u.custom_param.size;
    ret = af_port_ext_update_opaque_input_params(af_port,
      AF_SET_PARM_CUSTOM_EVT_HAL, &payload);
    if (FALSE == ret) {
      AF_ERR("Failure handling the custom parameter!");
      rc = STATS_EXT_HANDLING_FAILURE;
    }
  }
    break;
  case COMMON_SET_PARAM_BESTSHOT: {
    /* Sample on how to handle existing HAL events */
    q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
      AF_SET_PARAM_BESTSHOT, af_port);
    if (NULL == af_msg) {
      AF_ERR("Memory allocation failure!");
      ret = FALSE;
    } else {
      /* populate af message to post to thread */
      /* This value is usually map at port to convert from HAL to algo enum types,
       * in this case passing the value directly since port doesn't know the
       * required mapping for custom algo */
      /* 3rd party could do the mapping here if prefered*/
      af_msg->u.af_set_parm.u.af_bestshot_mode = (af_bestshot_mode_type_t)param->u.bestshot_mode;

      ret = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
      if (!ret) {
        AF_ERR("Fail to queue msg");
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


/** af_port_ext_handle_set_parm_event
 *
 *  @port: port information
 *  @param: set parameter received.
 *
 *  Extend handling of set parameter call from HAL.
 *
 **/
static stats_ext_return_type af_port_ext_handle_set_parm_event(
  mct_port_t *port,
  void *evt_param)
{
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  af_port_private_t  *af_port = (af_port_private_t *)(port->port_private);
  stats_set_params_type *stat_parm = (stats_set_params_type *)evt_param;

  if (!stat_parm || !af_port) {
    AF_ERR("Invalid parameters!");
    return FALSE;
  }

  AF_LOW("Extended handling set param event of type: %d", stat_parm->param_type);
  /* These cases can be extended to handle other parameters here
  if required. */
  if (stat_parm->param_type == STATS_SET_COMMON_PARAM) {
    rc = af_port_ext_handle_set_parm_common_event(
      port, (void *)&(stat_parm->u.common_param));
  } else {
    /* Handle af specific set parameters here if different from af port handling */
    if (stat_parm->param_type == STATS_SET_Q3A_PARAM) {
      q3a_set_params_type  *q3a_param = &(stat_parm->u.q3a_param);
      if (q3a_param->type == Q3A_SET_AF_PARAM) {
        rc = af_port_ext_handle_set_parm_af_event(
          port, (void *)&q3a_param->u.af_param);
      }
    }
  }

  return rc;
} /* af_port_ext_handle_set_parm_event */


/** af_port_ext_handle_control_event:
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
static stats_ext_return_type af_port_ext_handle_control_event(
  mct_port_t *port,
  mct_event_control_t *ctrl_evt)
{
  af_port_private_t *af_port =
    (af_port_private_t *)(port->port_private);
  q3a_custom_data_t payload;
  mct_custom_data_payload_t *cam_custom_ctr =
    (mct_custom_data_payload_t *)ctrl_evt->control_event_data;
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;

  switch (ctrl_evt->type) {
  case MCT_EVENT_CONTROL_CUSTOM: {
    boolean ret = FALSE;
    payload.data = cam_custom_ctr->data;
    payload.size = cam_custom_ctr->size;
    ret = af_port_ext_update_opaque_input_params(af_port,
      AF_SET_PARM_CUSTOM_EVT_CTRL, &payload);
    if (FALSE == ret) {
      AF_ERR("Failure handling the custom parameter!");
      rc = STATS_EXT_HANDLING_FAILURE;
    } else {
      rc = STATS_EXT_HANDLING_COMPLETE;
    }
  }
    break;
  case MCT_EVENT_CONTROL_SET_PARM: {
    rc = af_port_ext_handle_set_parm_event(port, ctrl_evt->control_event_data);
  }
    break;
  case MCT_EVENT_CONTROL_STREAMON: {
    rc = STATS_EXT_HANDLING_PARTIAL;
  }
  break;
  case MCT_EVENT_CONTROL_LINK_INTRA_SESSION: {
    boolean ret = FALSE;
    cam_sync_related_sensors_event_info_t *link_param =
      (cam_sync_related_sensors_event_info_t *)(ctrl_evt->control_event_data);;
    boolean is_thread_created = FALSE;
    boolean set_thread_obj_data = FALSE;

    AF_HIGH("AF link request session_id: %u, aec_port: %p, link type: %d, 3A_sync: %d",
      GET_SESSION_ID(af_port->reserved_id), af_port, link_param->type,
      link_param->sync_3a_config.sync_mode_af);

    do {
      ret = q3a_thread_ext_af_create_single_thread(port);
      if (FALSE == ret) {
        AF_ERR("Fail to create ext AF thread");
        break;
      }
      is_thread_created = TRUE;

      ret = q3a_thread_ext_af_get_thread_obj(port);
      if (FALSE == ret) {
        AF_ERR("Fail to get AF obj data");
        break;
      }
      ret = q3a_thread_ext_af_set_thread_obj(port);
      if (FALSE == ret) {
        AF_ERR("Fail to set AF obj data");
        break;
      }
      set_thread_obj_data = TRUE;
    } while (0);

    if (FALSE == ret) {
      /* Handle error */
      AF_ERR("Error handling: fail to link: %p", af_port);
      if (set_thread_obj_data) {
        q3a_thread_ext_af_release_af_obj_ref(port);
      }
      if (is_thread_created) {
        q3a_thread_ext_af_destroy_single_thread();
      }

      rc = STATS_EXT_HANDLING_FAILURE;
    }

    rc = STATS_EXT_HANDLING_PARTIAL;
  }
    break;
  case MCT_EVENT_CONTROL_UNLINK_INTRA_SESSION: {
    AF_HIGH("Un-link & destroy request cam: %d, 3A_sync: %d mode :%d, af_port: %p",
      af_port->dual_cam_info.sync_3a_mode,
      af_port->dual_cam_info.mode,
      af_port);

    q3a_thread_ext_af_release_af_obj_ref(port);
    q3a_thread_ext_af_destroy_single_thread();

    AF_HIGH("AF unlink & destroy done");

    rc = STATS_EXT_HANDLING_PARTIAL;
  }
    break;

  default: {
    rc = STATS_EXT_HANDLING_PARTIAL;
  }
    break;
  }

  return rc;
} /* af_port_ext_handle_control_event */


/** af_port_ext_handle_module_event:
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
static stats_ext_return_type af_port_ext_handle_module_event(
  mct_port_t *port,
  mct_event_module_t *mod_evt)
{
  af_port_private_t *af_port =
    (af_port_private_t *)(port->port_private);
  q3a_custom_data_t payload;
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;

  AF_LOW("Handle AF module event of type: %d", mod_evt->type);

  switch (mod_evt->type) {
  case MCT_EVENT_MODULE_CUSTOM_STATS_DATA_AF: {
    /* Stats pre-processing or syncronization is optional
     * but could be done here if prefered */
    mct_custom_data_payload_t *cam_custom_evt =
      (mct_custom_data_payload_t *)mod_evt->module_event_data;
    if (af_port->af_initialized) {
      boolean ret = FALSE;
      payload.data = cam_custom_evt->data;
      payload.size = cam_custom_evt->size;
      ret = af_port_ext_update_opaque_input_params(af_port,
        AF_SET_PARM_CUSTOM_EVT_MOD, &payload);
      if (FALSE == ret) {
        AF_ERR("Failure handling the custom parameter!");
        rc = STATS_EXT_HANDLING_FAILURE;
      } else {
        rc = STATS_EXT_HANDLING_COMPLETE;
      }
    }
  }
    break;
  case MCT_EVENT_MODULE_CUSTOM: {
    boolean ret = FALSE;
    mct_custom_data_payload_t *cam_custom_evt =
      (mct_custom_data_payload_t *)mod_evt->module_event_data;
    payload.data = cam_custom_evt->data;
    payload.size = cam_custom_evt->size;
    ret = af_port_ext_update_opaque_input_params(af_port,
      AF_SET_PARM_CUSTOM_EVT_MOD, &payload);
    if (FALSE == ret) {
      AF_ERR("Failure handling the custom parameter!");
      rc = STATS_EXT_HANDLING_FAILURE;
    } else {
      rc = STATS_EXT_HANDLING_COMPLETE;
    }
  }
    break;

  case MCT_EVENT_MODULE_STATS_AF_GET_THREAD_OBJECT: {
    q3a_thread_af_get_port_data_t *af_get_data =
      (q3a_thread_af_get_port_data_t *)(mod_evt->module_event_data);

    /* need it for dual cameras sharing thread, to backup original thread_data */
    af_get_data->camera_id    = af_port->camera_id;
    af_get_data->thread_data  = af_port->thread_data;
  }
    break;

  case MCT_EVENT_MODULE_STATS_AF_SET_THREAD_OBJECT: {
    q3a_thread_af_set_port_data_t *af_set_data =
      (q3a_thread_af_set_port_data_t *)(mod_evt->module_event_data);

    /* need it for dual cameras sharing thread, set the algo instance to be use*/
    if (af_set_data->af_algo_instance) {
      af_port->af_object.af  = af_set_data->af_algo_instance;
      AF_HIGH("AF: session_id: %d: set camera_id: %d, set algo: %p",
        GET_SESSION_ID(af_port->reserved_id), af_port->camera_id,
        af_port->af_object.af);
    }
  }
    break;

  case MCT_EVENT_MODULE_STATS_AF_RESTORE_EXT_THREAD_OBJECT: {
    q3a_thread_restore_thread_data_t *restore_obj =
      (q3a_thread_restore_thread_data_t *)(mod_evt->module_event_data);

    AF_HIGH("AF port (%p): camera_id: %u algo: %p, restore to camera_id: %u thread data %p, algo: %p",
      af_port, af_port->camera_id, af_port->af_object.af, restore_obj->camera_id,
      restore_obj->q3a_thread_data, restore_obj->af_algo_instance);

    if (NULL == restore_obj) {
      AF_ERR("Restore obj NULL");
      return FALSE;
    }

    af_port->camera_id = restore_obj->camera_id;
    af_port->thread_data = restore_obj->q3a_thread_data;
    af_port->af_object.af = restore_obj->af_algo_instance;

    rc = STATS_EXT_HANDLING_COMPLETE;
  }
    break;

  case MCT_EVENT_MODULE_START_STOP_STATS_THREADS: {
    uint8_t *start_flag = (uint8_t*)(mod_evt->module_event_data);
    AF_HIGH("MCT_EVENT_MODULE_START_STOP_STATS_THREADS: start_flag: %d,"
      " link mode: %d, 3A_sync: %d sync_mode: %d, af_port: %p",
      *start_flag, af_port->dual_cam_info.sync_3a_mode,
      af_port->dual_cam_info.sync_3a_mode,
      af_port->dual_cam_info.mode,
      af_port);
    if (TRUE == *start_flag) {
      AF_HIGH("Do not start ext thread here");
      break;
    }

    if (CAM_3A_SYNC_NONE == af_port->dual_cam_info.sync_3a_mode) {
      AF_HIGH("MCT_EVENT_MODULE_START_STOP_STATS_THREADS: in CAM_TYPE_STANDALONE already, nothing to release!!");
      break;
    }

    q3a_thread_ext_af_release_af_obj_ref(port);
    q3a_thread_ext_af_destroy_single_thread();
    af_port->dual_cam_info.mode = CAM_MODE_PRIMARY;
    af_port->dual_cam_info.sync_3a_mode = CAM_3A_SYNC_NONE;
    af_port->dual_cam_info.is_LPM_on = FALSE;
    af_port->dual_cam_info.is_aux_sync_enabled = FALSE;
    af_port->dual_cam_info.is_aux_update_enabled = FALSE;
    af_port->dual_cam_info.intra_peer_id = 0;

    AF_HIGH("AF got STOP unlink, destroy ext AF thread done");
    rc = STATS_EXT_HANDLING_COMPLETE;
  }
    break;
  default: {
    AF_LOW("Default. no action!");
    rc = STATS_EXT_HANDLING_PARTIAL;
  }
    break;
  }

  return rc;
} /* af_port_ext_handle_module_event */

static void af_port_ext_status_translate_update(mct_port_t *port)
{
  mct_event_t event;
  mct_bus_msg_af_status_t af_msg;
  af_port_private_t *priv = (af_port_private_t *)(port->port_private);
  af_msg.focus_state = (cam_autofocus_state_t)priv->af_trans.af_state;
  cam_af_state_t state = priv->af_trans.af_state;
  cam_focus_distances_info_t distance = priv->focus_distance_info;

  af_msg.focus_mode = priv->bus.focus_mode;
  af_msg.f_distance = distance;
  switch (state) {
  case CAM_AF_STATE_ACTIVE_SCAN:
    af_msg.focus_state = CAM_AF_SCANNING;
    break;
  case CAM_AF_STATE_FOCUSED_LOCKED:
    af_msg.focus_state = CAM_AF_FOCUSED;
    break;
  case CAM_AF_STATE_NOT_FOCUSED_LOCKED:
    af_msg.focus_state = CAM_AF_NOT_FOCUSED;
    break;
  case CAM_AF_STATE_INACTIVE:
    af_msg.focus_state = CAM_AF_INACTIVE;
    break;
  case CAM_AF_STATE_PASSIVE_SCAN:
    af_msg.focus_state = CAM_CAF_SCANNING;
    break;
  case CAM_AF_STATE_PASSIVE_FOCUSED:
    af_msg.focus_state = CAM_CAF_FOCUSED;
    break;
  case CAM_AF_STATE_PASSIVE_UNFOCUSED:
    af_msg.focus_state = CAM_CAF_NOT_FOCUSED;
    break;
  default:
    AF_ERR("TODO: not handled %d", af_msg.focus_state);
    break;
  }

  af_send_bus_msg(port, MCT_BUS_MSG_Q3A_AF_STATUS, &af_msg,
    sizeof(mct_bus_msg_af_status_t), STATS_REPORT_IMMEDIATE);
  AF_LOW("AF Status - state:%d->%d, mode:%d, dis[%f:%f:%f]",
    state,
    af_msg.focus_state, af_msg.focus_mode,
    af_msg.f_distance.focus_distance[0],
    af_msg.f_distance.focus_distance[1],
    af_msg.f_distance.focus_distance[2]);
}

/** af_port_ext_handle_intramod_event:
 *    @port:    AF port data
 *    @mod_evt: module event
 *
 * Handle module event received at AF port from peer AF port
 *
 * Return one of the return type defined by
 * stats_ext_return_type
 **/
static stats_ext_return_type af_port_ext_handle_intra_event(mct_port_t *port,
  mct_event_t *event)
{
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  af_port_private_t *private = NULL;
  mct_event_module_t *mod_evt = NULL;
  AF_LOW("Received AF intra-module peer event");

  /* sanity check */
  if (!port || !port->port_private || !event) {
    return FALSE;
  }

  private = (af_port_private_t *)(port->port_private);
  mod_evt = (mct_event_module_t *)&(event->u.module_event);

  switch (mod_evt->type) {
    default:
        rc = STATS_EXT_HANDLING_PARTIAL;
      break;
  }
  return rc;
}

/** af_port_ext_callback:
 *    @port: port info
 *    @af_out: Output parameters from core algorithm
 *    @af_update: AF updates to be sent to other modules.
 *
 * Extension of AF core callback. Here OEM can process output
 * parameters received from AF core. There might be OEM specific
 * parameters as well as standard output which OEM might want to
 * handle on its own.
 *
 * Return one of the return type defined by
 * stats_ext_return_type
 **/
static stats_ext_return_type af_port_ext_callback(
  mct_port_t *port,
  void *core_out,
  void *update)
{
  af_port_private_t *private = (af_port_private_t *)(port->port_private);
  af_update_t *af_update = (af_update_t *)update;
  af_output_data_t *af_out = (af_output_data_t *)core_out;
  stats_ext_return_type ret = STATS_EXT_HANDLING_PARTIAL;
  if (private->dual_cam_info.sync_3a_mode) {
    /* Return now to avoid any cb processing while using QC algo & extension.
       Feel free to remove this and continue handling cb here */
    return ret;
  }

  /* handle custom params */
  if (af_out->type & AF_OUTPUT_CUSTOM) {
    /* Send the custom parameters as it is. Receiver will handle as required. */
    af_update->af_custom_param_update.data = af_out->af_custom_param.data;
    af_update->af_custom_param_update.size = af_out->af_custom_param.size;
    af_out->type &= ~AF_OUTPUT_CUSTOM;
  }

  /* handle other parameters if required */
  if (af_out->type & AF_OUTPUT_STATUS) {
    af_port_process_status_update(port, af_out);
  }

  /* Send ext bus msg with translated status */
  af_port_ext_status_translate_update(port);

  return ret;
}


/** af_port_ext_init:
 *    @port: port info
 *    @session_id: current session id
 *
 * Handle extended initialization here.
 *
 * Return one of the return type defined by
 * stats_ext_return_type
 **/
static stats_ext_return_type af_port_ext_init(
  mct_port_t *port,
  unsigned int session_id)
{
  af_port_private_t *private = (af_port_private_t *)(port->port_private);
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  af_ext_param_t *ext_param = NULL;
  (void)session_id;

  ext_param = (af_ext_param_t *) calloc(1, sizeof(af_ext_param_t));
  if (NULL == ext_param) {
    AF_ERR("Error allocating memory for extension init!");
    rc = STATS_EXT_HANDLING_FAILURE;
  } else {
    /* Output of core algorithm will have void pointer to point to custom output
       parameters. Assign space to save those parameters. */
    private->af_object.output.af_custom_param.data =
      (void *)&ext_param->custom_output;
    private->af_object.output.af_custom_param.size =
      sizeof(ext_param->custom_output);
    private->ext_param = ext_param;
  }

  q3a_thread_ext_af_initialize_register();

  return rc;
}

/** af_port_ext_deinit:
 *     @port: port info
 *
 * Extension of AF port de-init.
 *
 * Return: none
 **/
void af_port_ext_deinit(mct_port_t *port)
{
  af_port_private_t *private = (af_port_private_t *)(port->port_private);
  if (private->ext_param) {
    free(private->ext_param);
  }
}

/** af_port_ext_update_func_table:
 *    @ptr: pointer to internal af pointer object
 *
 * Update extendable function pointers.
 *
 * Return: TRUE in success
 **/
boolean af_port_ext_update_func_table(void *ptr)
{
  af_port_private_t *private = (af_port_private_t *) ptr;
  private->func_tbl.ext_init = af_port_ext_init;
  private->func_tbl.ext_deinit = af_port_ext_deinit;
  private->func_tbl.ext_callback = af_port_ext_callback;
  private->func_tbl.ext_handle_module_event = af_port_ext_handle_module_event;
  private->func_tbl.ext_handle_control_event =
    af_port_ext_handle_control_event;
  private->func_tbl.ext_handle_intra_event = af_port_ext_handle_intra_event;

  return TRUE;
}

/**
 * af_port_ext_is_extension_required
 *
 * @af_libptr: Pointer to the vendor library
 * @cam_position: Camera position
 * @use_default_algo: The decision to use or not default (QC) algo is returned by this flag
 *
 * Return: TRUE is extension is required
 **/
boolean af_port_ext_is_extension_required(void *af_libptr,
  cam_position_t cam_position, boolean *use_default_algo)
{
  boolean rc = FALSE;
  int test_dual_out = 0;
  (void)cam_position;
  *use_default_algo = FALSE;
  if (af_libptr) {
    rc = TRUE;
  } else {
#ifdef _AF_EXTENSION_
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
 * af_port_ext_load_function
 *
 * @af_ops: structure with function pointers to be assign
 * @af_libptr: Parameter to provide the pointer to the af library (optional)
 * @cam_position: Camera position
 * @use_default_algo: Using or not default algo flag
 *
 *  This function is intended to be used by OEM.
 *  The OEM must use this fuction to populate the algo interface
 *  function pointers.
 *
 * Return: Returns handler to AF interface lib
 **/
void * af_port_ext_load_function(af_ops_t *af_ops, void *af_libptr,
  cam_position_t cam_position, boolean use_default_algo)
{
  void *af_handle = NULL;
  int test_dual_out = 0;
  (void)af_libptr;
  (void)cam_position;

  if (use_default_algo) {
    AF_HIGH("Load default algo functions");
    STATS_TEST_DUAL_OUTPUT(test_dual_out);
    if (test_dual_out) {
      af_handle = af_ext_test_wrapper_load_function(af_ops);
    } else {
      af_handle = af_port_load_function(af_ops);
    }
  } else {
    AF_ERR("Error: This is a DUMMY function, used only for reference");
    af_handle = NULL;
  }

  return af_handle;
}

/** af_port_ext_unload_function
 *
 *    @private: Port private structure
 *
 *  This function is intended to be used by OEM.
 *  The OEM must use this fuction to free resources allocated at
 *  af_port_ext_load_function()
 *
 * Return: void
 **/
void af_port_ext_unload_function(af_port_private_t *private)
{
  int test_dual_out = 0;
  if (!private) {
    return;
  }

  if (private->use_default_algo) {
    STATS_TEST_DUAL_OUTPUT(test_dual_out);
    if (test_dual_out) {
      af_ext_test_wrapper_unload_function(&private->af_object.af_ops, private->af_iface_lib);
      af_port_load_dummy_default_func(&private->af_object.af_ops);
      private->af_iface_lib = NULL;
    } else {
      af_port_unload_function(private);
    }
  } else {
    AF_ERR("Error: This is a DUMMY function, used only for reference");
    /* Reset to default interface values */
    af_port_load_dummy_default_func(&private->af_object.af_ops);
  }

  return;
}
