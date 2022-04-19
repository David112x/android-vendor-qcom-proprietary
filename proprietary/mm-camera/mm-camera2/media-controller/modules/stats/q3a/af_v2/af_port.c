/* af_port.c
 *
 * Copyright (c) 2016-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "af_module.h"
#include "af_port.h"
#include "af_biz.h"
#include "cam_intf.h"
#include "cam_types.h"
#include "af.h"
#include "af_ext.h"
#include "af_fdprio.h"
#include "q3a_thread.h"
#include "q3a_port.h"
#include "mct_stream.h"
#include "mct_module.h"
#include "stats_event.h"
#include "math.h"
#include "sensor_lib.h"
#include "stats_util.h"
#include "q3a_platform.h"

#define AF_PORT_MAX_FOCUS_DISTANCE 10 // in m
#define GRAVITY_VECTOR 9.81

/* Number of AF stats to ignore before moving lens when
   FOVC is enabled. This depends on the pipeline delay */
#define AF_PORT_FOVC_FRAME_SKIP 1
#define AF_PORT_FOVC_CPP_FRAME_ADJUSTMENT 1

distance_entry global_map[SINGLE_MAX_IDX] = {
  {INT_MAX, SINGLE_NEAR_LIMIT_IDX},
  {7,       SINGLE_7CM_IDX},
  {10,      SINGLE_10CM_IDX},
  {14,      SINGLE_14CM_IDX},
  {20,      SINGLE_20CM_IDX},
  {30,      SINGLE_30CM_IDX},
  {40,      SINGLE_40CM_IDX},
  {50,      SINGLE_50CM_IDX},
  {60,      SINGLE_60CM_IDX},
  {120,     SINGLE_120CM_IDX},
  {INT_MAX, SINGLE_HYP_F_IDX},
  {INT_MAX,  SINGLE_INF_LIMIT_IDX}
};

/*Forward Declarations*/
static boolean af_port_link_to_peer(mct_port_t *port,
                                         mct_event_t *event);
static boolean af_port_unlink_from_peer(mct_port_t *port,
                                               mct_event_t *event);
static boolean af_port_local_unlink(mct_port_t *port,
                                        mct_event_t *event);
static boolean af_port_remote_unlink(mct_port_t *port);
static boolean af_port_handle_role_switch(mct_port_t *port,
                                                mct_event_t *event);
static boolean af_port_reset_dual_cam_info(af_port_private_t  *port);
 static boolean af_port_update_aux_sync_and_update(af_port_private_t  *port);
static boolean af_port_update_LPM(mct_port_t *port,cam_dual_camera_perf_control_t* perf_ctrl);
static boolean af_port_set_role_switch(mct_port_t  *port);
static boolean af_port_forward_focus_info_if_linked(mct_port_t  *port);
static boolean af_port_send_wakeup(mct_port_t *port, boolean enable);
static boolean af_port_send_sleep_if_required(mct_port_t *port, boolean swafdisable);
static boolean af_port_wake_up_if_required(mct_port_t  *port,
                                          af_core_focus_info *info);
static boolean af_port_handle_peer_focus_info(mct_port_t  *port,
                                            af_core_focus_info *info);
static boolean af_port_intra_event(mct_port_t *port, mct_event_t *event);

/* Enum to indicate the FOVC crop mode. */
typedef enum {
  FOVC_MODE_DISABLED = 0,
  FOVC_MODE_CPP,
  FOVC_MODE_ISP,
  FOVC_MODE_MAX
} af_port_fovc_mode_t;


static boolean af_port_handle_set_focus_manual_pos_evt(
  mct_port_t * port, af_input_manual_focus_t * manual_pos_info);

/* Static Function Definitions */
static boolean af_port_handle_isp_output_dim_event(af_port_private_t *af_port,
  mct_stream_info_t *stream_info);
static float af_port_get_optimal_focus_distance(af_port_private_t *af_port,
                                                int32_t cur_pos);
static void af_port_get_near_far_field_focus_distance(af_port_private_t *af_port,
                                                      cam_focus_distances_info_t *f_info);

static float af_port_convert_index_to_diopter
  (af_port_private_t *af_port, int32_t index);

static uint32_t af_port_convert_index_to_scale
  (af_port_private_t *af_port, int32_t index);

static boolean af_port_is_mode_continous(cam_focus_mode_type mode);

static boolean af_port_handle_do_af_event(af_port_private_t *af_port);

static void af_port_update_roi_info(mct_port_t *port,
  af_output_data_t *af_out);

static boolean af_port_send_af_config_to_imagelib(mct_port_t *port, af_output_data_t *af_out);

static boolean af_port_send_move_lens_cmd(void *output, void *p);

boolean af_port_buf_touch_in_use(af_port_private_t *af_port,
  isp_buf_divert_t *stats_buff);

boolean af_port_buf_touch_done(af_port_private_t *af_port,
  isp_buf_divert_t *stats_buff);

boolean af_port_pd_zero_touch_lag_update(af_port_private_t *af_port,
  af_port_pdaf_zero_touch_lag_field_t field_type,
  const pd_zero_touch_lag_info_t* info);

boolean af_port_pd_zero_touch_lag_access(af_port_private_t *af_port,
  pd_zero_touch_lag_info_t* info);

void af_port_pd_early_res_update(af_port_private_t *af_port,
  af_port_pdaf_early_res_field_t field_type,
  const pd_lib_early_result_t* info);

void af_port_pd_early_res_access(af_port_private_t *af_port,
  pd_lib_early_result_t* info);

/** af_port_create_msg:
 *    @msg_type:   Type to be set by the msg
 *    @param_type: Parameter type
 *    @private: Contains port private structure containing common info.
 *
 * Create msg intended to be queue to the algorithm thread. Allocate memory,
 *  assign a type and set common parameters to the header of the msg.
 *
 * Return: Address of the allocated msg
 **/
q3a_thread_af_msg_t* af_port_create_msg(q3a_thread_af_msg_type_t msg_type,
  int param_type, af_port_private_t *af_port)
{
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));

  if (af_msg == NULL) {
    return NULL;
  }
  memset(af_msg, 0 , sizeof(q3a_thread_af_msg_t));

  af_msg->type = msg_type;
  af_msg->camera_id = af_port->camera_id;
  if (msg_type == MSG_AF_SET || msg_type == MSG_AF_START ||
      msg_type == MSG_AF_CANCEL) {
    af_msg->u.af_set_parm.type = param_type;
    af_msg->u.af_set_parm.camera_id = af_port->camera_id;
  } else if (msg_type == MSG_AF_GET) {
    af_msg->u.af_get_parm.type = param_type;
    af_msg->u.af_get_parm.camera_id = af_port->camera_id;
  }

  return af_msg;
}

boolean af_port_dummy_set_parameters(af_set_parameter_t *param,
  af_output_data_t *output, uint8_t num_of_out, void *af_obj)
{
  (void)param;
  (void)output;
  (void)num_of_out;
  (void)af_obj;
  AF_ERR("Uninitialized interface been use");
  return FALSE;
}

boolean af_port_dummy_get_parameters(af_get_parameter_t *param,
  void *af_obj)
{
  (void)param;
  (void)af_obj;
  AF_ERR("Uninitialized interface been use");
  return FALSE;
}

void af_port_dummy_process(stats_af_t *stats, af_output_data_t *output,
  uint8_t num_of_out, void *af)
{
  (void)stats;
  (void)output;
  (void)num_of_out;
  (void)af;
  AF_ERR("Uninitialized interface been use");
  return;
}
void *af_port_dummy_init(void *lib)
{
  (void)lib;
  AF_ERR("Uninitialized interface been use");
  return NULL;
}

void af_port_dummy_destroy(void *af_obj)
{
  (void)af_obj;
  AF_ERR("Uninitialized interface been use");
  return;
}

/**
 * af_port_load_dummy_default_func
 *
 * @af_ops: structure with function pointers to be assign
 *
 * Return: TRUE on success
 **/
boolean af_port_load_dummy_default_func(af_ops_t *af_ops)
{
  boolean rc = FALSE;
  if (af_ops) {
    af_ops->set_parameters = af_port_dummy_set_parameters;
    af_ops->get_parameters = af_port_dummy_get_parameters;
    af_ops->process = af_port_dummy_process;
    af_ops->init = af_port_dummy_init;
    af_ops->deinit = af_port_dummy_destroy;
    rc = TRUE;
  }
  return rc;
}

/** af_port_load_function
 *
 *    @aec_object: structure with function pointers to be assign
 *
 * Return: Handler to AF interface library
 **/
void* af_port_load_function(af_ops_t *af_ops)
{
  if (!af_ops) {
    return NULL;
  }

  return af_biz_load_function(af_ops);
}

/** af_port_unload_function
 *
 *    @private: Port private structure
 *
 *  Free resources allocated by af_port_load_function
 *
 * Return: void
 **/
void af_port_unload_function(af_port_private_t *private)
{
  if (!private) {
    return;
  }

  af_biz_unload_function(&private->af_object.af_ops, private->af_iface_lib);
  af_port_load_dummy_default_func(&private->af_object.af_ops);
  if (private->af_iface_lib) {
    private->af_iface_lib = NULL;
  }
  return;
}

/** af_port_set_session_data:
 *    @port: af's sink port to be initialized
 *    @q3a_lib_info: Q3A session data information
 *    @cam_position: Camera position
 *    @sessionid: session identity
 *
 *  Provide session data information for algo library set-up.
 **/
boolean af_port_set_session_data(mct_port_t *port, void *q3a_lib_info,
  mct_pipeline_session_data_t *session_data, unsigned int *sessionid)
{
  af_port_private_t *private = NULL;
  int rc = FALSE;
  unsigned int session_id = (((*sessionid) >> 16) & 0x00ff);
  mct_pipeline_session_data_q3a_t *q3a_session_data = NULL;
  (void*)session_data;

  if (!port || !port->port_private || strcmp(MCT_OBJECT_NAME(port), "af_sink")) {
    return rc;
  }

  q3a_session_data = (mct_pipeline_session_data_q3a_t *)q3a_lib_info;

  AF_HIGH("af_libptr %p session_id %d", q3a_session_data->af_libptr, session_id);

  private = port->port_private;

  /* Query to verify if extension use is required and if using default algo */
  private->af_extension_use =
    af_port_ext_is_extension_required(q3a_session_data->af_libptr,
      session_data->position, &private->use_default_algo);
  if (FALSE == private->af_extension_use) {
    AF_HIGH("Load AF interface functions");
    private->af_iface_lib = af_port_load_function(&private->af_object.af_ops);
  } else { /* Use extension */
    AF_HIGH("Load AF EXTENSION interface functions");
    private->af_iface_lib = af_port_ext_load_function(&private->af_object.af_ops,
      q3a_session_data->af_libptr, session_data->position, private->use_default_algo);
  }

  /* Verify that all basic fields were populater by OEM */
  if (!(private->af_iface_lib && private->af_object.af_ops.init &&
    private->af_object.af_ops.deinit &&
    private->af_object.af_ops.set_parameters &&
    private->af_object.af_ops.get_parameters &&
    private->af_object.af_ops.process)) {
    AF_ERR("loading functions failed");
    /* Reset interface */
    if (FALSE == private->af_extension_use) {
      af_port_unload_function(private);
    } else {
      af_port_ext_unload_function(private);
    }
    return FALSE;
  }

  private->af_object.af =
    private->af_object.af_ops.init(private->af_iface_lib);
  rc = private->af_object.af ? TRUE : FALSE;
  if (FALSE == rc) {
    AF_ERR("fail to init AF algo");
    return rc;
  }

  /* Save lens type, to be provided to algo later */
  rc = q3a_port_map_sensor_format(&private->cam_info.sensor_type,
    session_data->sensor_format);
  if (!rc) {
    AF_ERR("Fail to map sensor_format");
    return rc;
  }
  rc = q3a_port_map_lens_type(&private->cam_info.lens_type,
    session_data->lens_type);
  if (!rc) {
    AF_ERR("Fail to map lens_format");
    return rc;
  }

  if (private->af_extension_use) {
    rc = af_port_ext_update_func_table(private);
    if (rc && private->func_tbl.ext_init) {
      stats_ext_return_type ret = STATS_EXT_HANDLING_FAILURE;
      ret = private->func_tbl.ext_init(port, session_id);
      if (ret != STATS_EXT_HANDLING_FAILURE) {
        rc = TRUE;
      }
    }
  }
  /* Consume HAL 3 applying delay. A bus msg is set at
     cur_sof + applying delay + reporting delay. In cases where
     af controls the application of a setting, it will sent the bus msg
     id such that the msg is sent at cur_sof + reporting delay*/
  if (META_REPORTING_DELAY_MASK & session_data->set_session_mask) {
    private->applying_delay =
    session_data->max_pipeline_frame_applying_delay -1;
  }
  AF_LOW("Max pipeline delay: %d", session_data->max_pipeline_frame_applying_delay);

  return rc;
}

/** af_port_map_input_roi_to_camif: Map input ROI to camif size.
 *
 *  @af_internal: internal af data structure
 *
 **/
static boolean af_port_map_input_roi_to_camif(
  af_port_private_t *af_port,
  af_roi_t *roi) {
  int roi_x, roi_y, roi_x2, roi_y2;
  double h_scale_ratio, v_scale_ratio;

  af_stream_crop_t stream_crop_zero;
  memset(&stream_crop_zero, 0, sizeof(af_stream_crop_t));
  if (!memcmp(&af_port->stream_crop, &stream_crop_zero,
      sizeof(af_stream_crop_t))) {
    AF_LOW("Crop data is not valid, ");
    return FALSE;
  }

  AF_HIGH("Input AF ROI: (%d, %d, %d, %d) Preview-size: %d x %d",
    roi->x, roi->y, roi->dx, roi->dy,
    af_port->preview_size.width, af_port->preview_size.height);
  if (!roi->dx || !roi->dy) {
    memset(roi, 0, sizeof(af_roi_t));
    return TRUE;
  }

  AF_LOW("VFE-MAP: (%d, %d, %d %d) PP-CROP: (%d, %d, %d, %d) ISP-DIM: %dx%d",
     af_port->stream_crop.vfe_map_x, af_port->stream_crop.vfe_map_y,
    af_port->stream_crop.vfe_map_width, af_port->stream_crop.vfe_map_height,
    af_port->stream_crop.pp_x, af_port->stream_crop.pp_y,
    af_port->stream_crop.pp_crop_out_x, af_port->stream_crop.pp_crop_out_y,
    af_port->isp_info.width, af_port->isp_info.height);

  if (!af_port->stream_crop.pp_crop_out_x ||
    !af_port->stream_crop.pp_crop_out_y) {
    h_scale_ratio = 1;
    v_scale_ratio = 1;
    af_port->stream_crop.pp_x = 0;
    af_port->stream_crop.pp_y = 0;
    af_port->stream_crop.pp_crop_out_x = af_port->preview_size.width;
    af_port->stream_crop.pp_crop_out_y = af_port->preview_size.height;
  } else {
    h_scale_ratio = (double)(af_port->preview_size.width) /
      af_port->stream_crop.pp_crop_out_x;
    v_scale_ratio = (double)(af_port->preview_size.height) /
      af_port->stream_crop.pp_crop_out_y;
  }

  // Reverse calculation for cpp output to vfe output
  roi_x = roi->x / h_scale_ratio;
  roi_y = roi->y / v_scale_ratio;
  roi_x += af_port->stream_crop.pp_x;
  roi_y += af_port->stream_crop.pp_y;

  roi_x2 = (roi->dx + roi->x) / h_scale_ratio;
  roi_y2 = (roi->dy + roi->y) / v_scale_ratio;
  roi_x2 += af_port->stream_crop.pp_x;
  roi_y2 += af_port->stream_crop.pp_y;

  // Reverse calculation for vfe output to camif output
  if (!af_port->isp_info.width || !af_port->isp_info.height) {
    AF_LOW("Invalid ISP output");
    return FALSE;
  }

  roi_x = ((roi_x * af_port->stream_crop.vfe_map_width) /
    af_port->isp_info.width) + af_port->stream_crop.vfe_map_x;
  roi_y = ((roi_y * af_port->stream_crop.vfe_map_height) /
    af_port->isp_info.height) + af_port->stream_crop.vfe_map_y;

  roi_x2 = ((roi_x2 * af_port->stream_crop.vfe_map_width) /
    af_port->isp_info.width) + af_port->stream_crop.vfe_map_x;
  roi_y2 = ((roi_y2 * af_port->stream_crop.vfe_map_height) /
    af_port->isp_info.height) + af_port->stream_crop.vfe_map_y;

  roi->x = roi_x;
  roi->y = roi_y;
  roi->dx = roi_x2 - roi_x;
  roi->dy = roi_y2 - roi_y;
  AF_LOW("Mapped ROI: (%d, %d, %d %d)",
    roi->x, roi->y, roi->dx, roi->dy);

  return TRUE;
} /* af_recalc_stats_config_roi_info: */

/** af_port_handle_lock_caf_event:
 *    @af_port:  private AF port data
 *    @set_parm: a message to populate
 *    @lock:     lock or unlock
 *
 * Handle continuous autofocus lock call from application.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_lock_caf_event(af_port_private_t * af_port,
  af_set_parameter_t * set_parm, boolean lock)
{
  (void) af_port;
  boolean rc = TRUE;

  /* Update parameter message to be sent */
  set_parm->type = AF_SET_PARAM_LOCK_CAF;
  set_parm->u.af_lock_caf = lock;

  af_port->caf_locked = lock;
  AF_LOW("Lock/unlock (%d) CAF!", lock);
  return rc;
} /* af_port_handle_lock_caf_event */

/** af_port_lock_caf
 * Lock unlock CAF
 *
 * @af_port: private port info
 *
 * @lock: lock caf if TRUE, unlock if false
 *
 * Return:
 **/
static void af_port_lock_caf(af_port_private_t *af_port, boolean lock)
{
  boolean            rc = TRUE;
  af_set_parameter_t *set_parm;

  if (!af_port) {
    AF_ERR("Invalid parameters!");
    return;
  }

  if (lock == af_port->caf_locked) {
    AF_LOW("No change in CAF lock state (%d). No action required!",
      af_port->caf_locked);
    return;
  }

  /* Allocate memory to create AF message */
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_LOCK_CAF, af_port);
  if (af_msg == NULL) {
    AF_ERR("Memory allocation failure!");
    return;
  }
  /* populate af message to post to thread */
  set_parm = &af_msg->u.af_set_parm;

  rc = af_port_handle_lock_caf_event(af_port, set_parm, lock);
  /* Enqueue the message to the AF thread */
  q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
} /* af_port_lock_caf */

/** af_port_handle_auto_mode_state_update
 * Handle AF state changes and action for focus mode Auto.
 *
 * @af_port: private port number
 *
 * @trans_reason: reason of transition
 *
 * @state (out): new state transitioned to
 * Return: True - Success  False - failure
 **/
static boolean af_port_handle_auto_mode_state_update(
  af_port_private_t *af_port,
  af_port_state_transition_type cause,
  cam_af_state_t *state)
{
  boolean rc = TRUE;
  cam_af_state_t new_state = af_port->af_trans.af_state;

  switch (af_port->af_trans.af_state) {
  case CAM_AF_STATE_INACTIVE: {
    if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Start AF sweep. Lens now moving */
      new_state = CAM_AF_STATE_ACTIVE_SCAN;
    }
  }
    break;

  case CAM_AF_STATE_ACTIVE_SCAN: {
    if (cause == AF_PORT_TRANS_CAUSE_AF_DONE_FOCUSED) {
      /* If AF successful lens now locked */
      new_state = CAM_AF_STATE_FOCUSED_LOCKED;
    } else if (cause == AF_PORT_TRANS_CAUSE_AF_DONE_UNFOCUSED){
      /* AF unfocused. Lens now locked */
      new_state = CAM_AF_STATE_NOT_FOCUSED_LOCKED;
    } else if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Cancel/reset AF. Lens now locked */
      new_state = CAM_AF_STATE_INACTIVE;
    }
  }
    break;

  case CAM_AF_STATE_FOCUSED_LOCKED: {
    if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Cancel/reset AF */
      new_state = CAM_AF_STATE_INACTIVE;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Start new sweep. Lens now moving */
      new_state = CAM_AF_STATE_ACTIVE_SCAN;
    }
  }
    break;

  case CAM_AF_STATE_NOT_FOCUSED_LOCKED: {
    if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Cancel/reset AF */
      new_state = CAM_AF_STATE_INACTIVE;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Start new sweep. Lens now moving */
      new_state = CAM_AF_STATE_ACTIVE_SCAN;
    }
  }
    break;

  default: {
    AF_HIGH("Error: Undefined AF state!");
  }
    break;
  }

  /* New state */
  AF_LOW("Old state: %d New AF state: %d, Cause: %d",
    af_port->af_trans.af_state, new_state, cause);

  *state = new_state;
  return rc;
} /* af_port_handle_auto_mode_state_update */

/** af_port_handle_caf_cam_mode_state_update
 * Handle AF state changes and action for focus mode Continuous
 * Picture.
 *
 * @af_port: private port number
 *
 * @trans_reason: reason of transition
 *
 * @state (out): new state transitioned to
 * Return: True - Success  False - failure
 **/
static boolean af_port_handle_caf_cam_mode_state_update(
  af_port_private_t *af_port,
  af_port_state_transition_type cause,
  cam_af_state_t *state)
{
  boolean rc = TRUE;
  cam_af_state_t new_state = af_port->af_trans.af_state;

  switch (af_port->af_trans.af_state) {
  case CAM_AF_STATE_INACTIVE: {
    if (cause == AF_PORT_TRANS_CAUSE_SCANNING) {
      /* Start AF scan. Lens now moving */
      new_state = CAM_AF_STATE_PASSIVE_SCAN;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Query AF state. Lens now locked. */
      new_state = CAM_AF_STATE_NOT_FOCUSED_LOCKED;
      af_port_lock_caf(af_port, TRUE);
    }
  }
    break;

  case CAM_AF_STATE_PASSIVE_SCAN: {
    if ((cause == AF_PORT_TRANS_CAUSE_AF_DONE_FOCUSED) ||
      (cause == AF_PORT_TRANS_CAUSE_AF_DONE_UNFOCUSED)) {
      /* AF scanning done. Lens now locked */
      if (af_port->af_trigger_called) {
        AF_LOW("AF trigger had been called before!");
        new_state = (af_port->af_status == AF_STATUS_FOCUSED) ?
          CAM_AF_STATE_FOCUSED_LOCKED : CAM_AF_STATE_NOT_FOCUSED_LOCKED;
        af_port->af_trigger_called = FALSE;
        af_port_lock_caf(af_port, TRUE);
      } else {
        new_state = (cause == AF_PORT_TRANS_CAUSE_AF_DONE_FOCUSED) ?
          CAM_AF_STATE_PASSIVE_FOCUSED : CAM_AF_STATE_PASSIVE_UNFOCUSED;
      }
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Eventual transition if focus good/not good. Lock Lens */
      /* We are still scanning. We'll wait till scanning complete comes back. */
      AF_LOW("We are still scanning. Wait till AF is done.");
      af_port->af_trigger_called = TRUE;
      /*Intimate algorithm about do af call when in caf scan*/
      af_port_handle_do_af_event(af_port);
    } else if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Reset lens position. Lens now locked. */
      new_state = CAM_AF_STATE_INACTIVE;
    }
  }
    break;

  case CAM_AF_STATE_PASSIVE_FOCUSED: {
    if (cause == AF_PORT_TRANS_CAUSE_SCANNING) {
      /* Start AF scan. Lens now moving */
      new_state = CAM_AF_STATE_PASSIVE_SCAN;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Immediate transition if focus good/not good. Lock Lens */
      new_state = CAM_AF_STATE_FOCUSED_LOCKED;
      af_port_lock_caf(af_port, TRUE);
    }  else if (cause == AF_PORT_TRANS_CAUSE_AF_DONE_UNFOCUSED) {
      new_state = CAM_AF_STATE_PASSIVE_UNFOCUSED;
    }
  }
    break;

  case CAM_AF_STATE_PASSIVE_UNFOCUSED: {
    if (cause == AF_PORT_TRANS_CAUSE_SCANNING) {
      /* Start AF scan. Lens now moving */
      new_state = CAM_AF_STATE_PASSIVE_SCAN;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Immediate transition if focus not good. Lock Lens */
      new_state = CAM_AF_STATE_NOT_FOCUSED_LOCKED;
      af_port_lock_caf(af_port, TRUE);
    }
  }
    break;

  case CAM_AF_STATE_FOCUSED_LOCKED: {
    if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Do nothing */
      new_state = CAM_AF_STATE_FOCUSED_LOCKED;
    } else if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Restart AF scan */
      new_state = CAM_AF_STATE_PASSIVE_FOCUSED;
      af_port_lock_caf(af_port, FALSE);
    }
  }
    break;

  case CAM_AF_STATE_NOT_FOCUSED_LOCKED: {
    if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Do nothing */
      new_state = CAM_AF_STATE_NOT_FOCUSED_LOCKED;
    } else if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Restart AF scan */
      new_state = CAM_AF_STATE_PASSIVE_UNFOCUSED;
      af_port_lock_caf(af_port, FALSE);
    }
  }
    break;

  default: {
    AF_HIGH("Error: Undefined AF state!");
  }
    break;
  }

  AF_LOW("Old state: %d New AF state: %d, Cause: %d",
    af_port->af_trans.af_state, new_state, cause);

  /* New state */
  *state = new_state;
  return rc;
} /* af_port_handle_caf_cam_mode_state_update */

/** af_port_handle_caf_video_mode_state_update
 * Handle AF state changes and action for focus mode Continuous
 * Video.
 *
 * @af_port: private port number
 *
 * @trans_reason: reason of transition
 *
 * @state (out): new state transitioned to
 * Return: True - Success  False - failure
 **/
static boolean af_port_handle_caf_video_mode_state_update(
  af_port_private_t *af_port,
  af_port_state_transition_type cause,
  cam_af_state_t *state)
{
  boolean rc = TRUE;
  cam_af_state_t new_state = af_port->af_trans.af_state;

  switch (af_port->af_trans.af_state) {
  case CAM_AF_STATE_INACTIVE: {
    if (cause == AF_PORT_TRANS_CAUSE_SCANNING) {
      /* Start AF scan. Lens now moving */
      new_state = CAM_AF_STATE_PASSIVE_SCAN;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Query AF state. Lens now locked. */
      new_state = CAM_AF_STATE_NOT_FOCUSED_LOCKED;
      af_port_lock_caf(af_port, TRUE);
    }
  }
    break;

  case CAM_AF_STATE_PASSIVE_SCAN: {
    if ((cause == AF_PORT_TRANS_CAUSE_AF_DONE_FOCUSED) ||
      (cause == AF_PORT_TRANS_CAUSE_AF_DONE_UNFOCUSED)) {
      /* AF scanning done. Lens now locked */
      new_state = (cause == AF_PORT_TRANS_CAUSE_AF_DONE_FOCUSED) ?
        CAM_AF_STATE_PASSIVE_FOCUSED : CAM_AF_STATE_PASSIVE_UNFOCUSED;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Immediate transition if focus good/not good. Lock Lens */
      new_state = (af_port->af_status == AF_STATUS_FOCUSED) ?
        CAM_AF_STATE_FOCUSED_LOCKED : CAM_AF_STATE_NOT_FOCUSED_LOCKED;
      af_port_lock_caf(af_port, TRUE);
    } else if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Reset lens position. Lens now locked. */
      new_state = CAM_AF_STATE_INACTIVE;
    }
  }
    break;

  case CAM_AF_STATE_PASSIVE_FOCUSED: {
    if (cause == AF_PORT_TRANS_CAUSE_SCANNING) {
      /* Start AF scan. Lens now moving */
      new_state = CAM_AF_STATE_PASSIVE_SCAN;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Immediate transition if focus good/not good. Lock Lens */
      new_state = CAM_AF_STATE_FOCUSED_LOCKED;
      af_port_lock_caf(af_port, TRUE);
    } else if (cause == AF_PORT_TRANS_CAUSE_AF_DONE_UNFOCUSED) {
      new_state = CAM_AF_STATE_PASSIVE_UNFOCUSED;
    }
  }
    break;

  case CAM_AF_STATE_PASSIVE_UNFOCUSED: {
    if (cause == AF_PORT_TRANS_CAUSE_SCANNING) {
      /* Start AF scan. Lens now moving */
      new_state = CAM_AF_STATE_PASSIVE_SCAN;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Immediate transition if focus good/not good. Lock Lens */
      new_state = CAM_AF_STATE_NOT_FOCUSED_LOCKED;
      af_port_lock_caf(af_port, TRUE);
    }
  }
    break;

  case CAM_AF_STATE_FOCUSED_LOCKED: {
    if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Do nothing */
      new_state = CAM_AF_STATE_FOCUSED_LOCKED;
    } else if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Restart AF scan */
      new_state = CAM_AF_STATE_PASSIVE_FOCUSED;
      af_port_lock_caf(af_port, FALSE);
    }
  }
    break;

  case CAM_AF_STATE_NOT_FOCUSED_LOCKED: {
    if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Do nothing */
      new_state = CAM_AF_STATE_NOT_FOCUSED_LOCKED;
    } else if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Restart AF scan */
      new_state = CAM_AF_STATE_PASSIVE_UNFOCUSED;
      af_port_lock_caf(af_port, FALSE);
    }
  }
    break;

  default: {
    AF_HIGH("Error: Undefined AF state!");
  }
    break;
  }

  /* New state */
  AF_LOW("Old state: %d New AF state: %d, Cause: %d",
    af_port->af_trans.af_state, new_state, cause);

  *state = new_state;
  return rc;
} /* af_port_handle_caf_video_mode_state_update */

/** af_port_update_af_state
 * Update AF state to HAL only if it changes
 *
 * @port: port info
 *
 * @trans_reason: reason of transition
 *
 * Return: True - Success  False - failure
 **/
boolean af_port_update_af_state(mct_port_t  *port,
  af_port_state_transition_type cause) {
  boolean rc = TRUE;
  mct_event_t event;
  mct_bus_msg_t bus_msg;
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  cam_af_state_t new_state = af_port->af_trans.af_state;
  cam_af_state_t cur_state = af_port->af_trans.af_state;

  af_port->af_trans.cause = cause;

  /* If transition cause is AF mode change, we'll set the
     state to INACTIVE unless mode change is from CAF to CAF
     on CAF to CAF, send prev state and unlock it. */
  if (cause == AF_PORT_TRANS_CAUSE_MODE_CHANGE) {
    if (af_port_is_mode_continous(af_port->af_mode) &&
      af_port_is_mode_continous(af_port->prev_af_mode)) {
      AF_LOW("CAF to CAF mode change. Unlock if locked");
      if (cur_state == CAM_AF_STATE_FOCUSED_LOCKED) {
        new_state = CAM_AF_STATE_PASSIVE_FOCUSED;
      } else if (cur_state == CAM_AF_STATE_NOT_FOCUSED_LOCKED) {
        new_state = CAM_AF_STATE_PASSIVE_UNFOCUSED;
      }
      /* Set force update for hal1 */
      pthread_mutex_lock(&(af_port->focus_state_mutex));
      af_port->focus_state_changed = TRUE;
      pthread_mutex_unlock(&(af_port->focus_state_mutex));
    } else {
      AF_LOW("AF mode changed. Set AF state to Inactive!");
      new_state = CAM_AF_STATE_INACTIVE;
      af_port->af_status = AF_STATUS_INIT;
    }
    af_port_lock_caf(af_port, FALSE);
  } else {
    /* If current mode is CONTINOUS_PICTURE */
    if (af_port->af_mode == CAM_FOCUS_MODE_CONTINOUS_PICTURE) {
      af_port_handle_caf_cam_mode_state_update(af_port, cause, &new_state);
    } else if (af_port->af_mode == CAM_FOCUS_MODE_CONTINOUS_VIDEO) {
      af_port_handle_caf_video_mode_state_update(af_port, cause, &new_state);
    } else if ((af_port->af_mode == CAM_FOCUS_MODE_AUTO) ||
      (af_port->af_mode == CAM_FOCUS_MODE_MACRO)) {
      af_port_handle_auto_mode_state_update(af_port, cause, &new_state);
    }
    /* For all other modes, we keep AF State INACTIVE */
    else {
      new_state = CAM_AF_STATE_INACTIVE;
    }
  }

  if (new_state != af_port->af_trans.af_state) {
    pthread_mutex_lock(&(af_port->focus_state_mutex));
    af_port->focus_state_changed = TRUE;
    pthread_mutex_unlock(&(af_port->focus_state_mutex));
    AF_LOW("AFCam-[mode -%d role-%d] Af State changed new_state=%d, old_state=%d!, cause=%d, af_mode=%d",
      af_port->dual_cam_info.mode,
      af_port->dual_cam_info.cam_role,
      new_state, af_port->af_trans.af_state, cause, af_port->af_mode);
    af_port->af_trans.af_state = new_state;
  }
  return rc;
}

/** af_send_bus_msg:
 * Post bus message
 *
 * @port: port info
 * @bus_msg_type: message type
 * @payload: message payload
 *
 * @status: focus status
 *
 * Return: TRUE: Success  FALSE: Failure
 **/
boolean af_send_bus_msg(mct_port_t *port,
  mct_bus_msg_type_t bus_msg_type, void *payload,
  int size, int sof_id)
{
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  mct_event_t event;
  mct_bus_msg_t bus_msg;
  boolean rc = TRUE;

  memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.sessionid = (af_port->reserved_id >> 16);
  bus_msg.type = bus_msg_type;
  bus_msg.msg = payload;
  bus_msg.size = size;

  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = af_port->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);
  event.u.module_event.current_frame_id = sof_id;

  rc = MCT_PORT_EVENT_FUNC(port)(port, &event);

  return rc;
}
/** af_send_batch_bus_message:
 *    @port:   af port
 *    @sof_id:
 *
 * Send bus message as a batch.
 *
 * Return nothing
 **/
static void af_send_batch_bus_message(mct_port_t *port, uint32_t urgent_sof_id,
  uint32_t regular_sof_id)
{
  boolean rc = FALSE;
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);

  mct_bus_msg_af_immediate_t af_immediate;
  mct_bus_msg_af_t af_msg;
  mct_bus_msg_af_update_t af_update;
  af_immediate.lens_state = af_port->bus.lens_state;
  af_immediate.af_state = af_port->af_trans.af_state;
  af_immediate.force_update = FALSE;
  pthread_mutex_lock(&(af_port->focus_state_mutex));
  if (af_port->focus_state_changed == TRUE) {
    af_immediate.force_update = TRUE;
    af_port->focus_state_changed = FALSE;
  }
  pthread_mutex_unlock(&(af_port->focus_state_mutex));
  af_immediate.af_trigger.trigger_id = af_port->af_trans.trigger.af_trigger_id;
  af_immediate.af_trigger.trigger = af_port->af_trans.trigger.af_trigger;
  af_immediate.focus_mode = af_port->bus.focus_mode;
  af_immediate.is_depth_based_focus = af_port->af_depth_based_focus;
  af_immediate.focus_value = af_port->focus_value;
  af_immediate.spot_light_detected = af_port->spot_light_detected;
  /* For HAL1 manual af */
  af_immediate.diopter = af_port->diopter;
  af_immediate.scale_ratio = af_port->scale_ratio;
  /* Update bus msg */
  af_update.f_distance = af_port->focus_distance_info;
  if (!af_port->af_initialized) {
    /* don't report AF ROI if AF is not supported */
    af_msg.af_roi_valid = FALSE;
  } else if (af_port->af_mode == CAM_FOCUS_MODE_OFF||
             af_port->af_mode == CAM_FOCUS_MODE_MANUAL) {
    /* Reporting ROI weight as 0 in manual mode */
    memset(&af_msg.af_roi, 0, sizeof(af_msg.af_roi));
    af_msg.default_roi = af_port->bus.default_roi;
    af_msg.af_roi_valid = TRUE;
    /* Report diopter and scale ratio in manual mode */
  } else {
    af_msg.af_roi = af_port->bus.roi_msg;
    af_msg.default_roi = af_port->bus.default_roi;
    af_msg.af_roi_valid = TRUE;
  }
  af_immediate.focus_pos = af_port->af_focus_pos;
  af_immediate.focal_length_ratio = af_port->focal_length_ratio;
  af_immediate.lens_shift_um = af_port->lens_shift_um;
  af_immediate.object_distance_cm = af_port->currentdistanceOfObjectInCm;
  af_immediate.near_field_cm = af_port->current_dist_near_in_cm;
  af_immediate.far_field_cm = af_port->current_dist_far_in_cm;

  // In manual mode set lens state as CAM_AF_LENS_STATE_MOVING for 2 (Max pipeline delay)
  // frames so that app will pick buffer only when lens movement is completed
  if ((af_port->af_mode == CAM_FOCUS_MODE_OFF||
       af_port->af_mode == CAM_FOCUS_MODE_MANUAL)&&(af_port->is_lens_moving_manual > 0)) {
        af_immediate.lens_state = CAM_AF_LENS_STATE_MOVING;
        af_port->is_lens_moving_manual--;
        AF_LOW("lens is moving");
   }


  af_send_bus_msg(port, MCT_BUS_MSG_AF, &af_msg,
    sizeof(mct_bus_msg_af_t), regular_sof_id);
  af_send_bus_msg(port, MCT_BUS_MSG_AF_IMMEDIATE, &af_immediate,
    sizeof(mct_bus_msg_af_immediate_t), urgent_sof_id);
  /*Events where af applies at application delay.
    regular_sof_id - af_port->applying_delay will ensure that this bus msg
    is sent at reporting delay in the stats port.*/
  af_send_bus_msg(port, MCT_BUS_MSG_AF_UPDATE, &af_update,
    sizeof(mct_bus_msg_af_update_t), regular_sof_id - af_port->applying_delay);
}


/** af_port_process_status_update:
 * Process AF status update to be sent to upper layer. Also prepare focus
 * distance update.
 *
 * @port:   port info
 * @af_out: the output from the algorithm
 *
 * Return: TRUE: Success  FALSE: Failure
 **/
boolean af_port_process_status_update(
  mct_port_t  *port,
  af_output_data_t *af_out) {
  boolean rc = TRUE;
  af_port_state_transition_type cause;
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  af_status_t *af_status = &af_out->focus_status;
  boolean state_transition = TRUE;
  boolean algo_status_changed =
    (af_port->af_status != af_status->status)? TRUE : FALSE;
  if(algo_status_changed) {
    AF_LOW("AFCam-[mode -%d role-%d] Status Update from algo! %d",
      af_port->dual_cam_info.mode,
      af_port->dual_cam_info.cam_role,
      af_status->status);
  }

  af_port->af_status = af_status->status;
  af_port->gravity_vector_data_applied=af_out->grav_applied;
  af_port->af_depth_based_focus = af_out->af_depth_based_focus;
  af_port->dual_cam_info.af_peer_focus_info = af_out->af_peer_focus_info;
  //This is required by HAL for Dual Camera case
  af_port->focal_length_ratio = af_out->af_peer_focus_info.focal_length_ratio;
  af_port->lens_shift_um = af_out->af_peer_focus_info.lens_shift_um;
  af_port->currentdistanceOfObjectInCm = af_out->af_peer_focus_info.currentdistanceOfObject/10;
  af_port->current_dist_far_in_cm = af_out->af_peer_focus_info.current_dist_far/10;
  af_port->current_dist_near_in_cm = af_out->af_peer_focus_info.current_dist_near/10;

  af_port_forward_focus_info_if_linked(port);

  if (!algo_status_changed) {
    return rc;
  }
  /* update AF status */
  switch (af_status->status) {
  case AF_STATUS_FOCUSED: {
    cause = AF_PORT_TRANS_CAUSE_AF_DONE_FOCUSED;
    state_transition = TRUE;
    af_port->focusedAtLeastOnce = TRUE;
  }
    break;
  case AF_STATUS_UNKNOWN: {
    cause = AF_PORT_TRANS_CAUSE_AF_DONE_UNFOCUSED;
    state_transition = TRUE;
  }
    break;
  case AF_STATUS_FOCUSING: {
    cause = AF_PORT_TRANS_CAUSE_SCANNING;
    state_transition = TRUE;
  }
    break;
  default: {
    state_transition = FALSE;
    AF_LOW("No state transition!");
  }
    break;
  }

  /* transition to new AF state */
  if (TRUE == state_transition) {
    af_port_update_af_state(port, cause);
  }

  return rc;
} /* af_port_process_status_update */

boolean af_port_send_af_lens_state(mct_port_t  *port)
{
  af_port_private_t   *af_port = (af_port_private_t *)(port->port_private);
  boolean             rc = TRUE;
  cam_af_lens_state_t lens_state = CAM_AF_LENS_STATE_STATIONARY;

  switch (af_port->af_trans.af_state) {
  case CAM_AF_STATE_INACTIVE:
  case CAM_AF_STATE_PASSIVE_FOCUSED:
  case CAM_AF_STATE_FOCUSED_LOCKED:
  case CAM_AF_STATE_NOT_FOCUSED_LOCKED:
    lens_state = CAM_AF_LENS_STATE_STATIONARY;
    break;
  case CAM_AF_STATE_PASSIVE_SCAN:
  case CAM_AF_STATE_ACTIVE_SCAN:
    lens_state = CAM_AF_LENS_STATE_MOVING;
    break;
  default:
    break;
  }

  af_port->bus.lens_state = lens_state;
  return rc;
}


/** af_port_send_stats_config:
 *    @port:   port info
 *    @af_out: output af data
 *
 * Send AF stats configuration info to ISP.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_send_stats_config(mct_port_t *port,
  af_output_data_t *af_out)
{
  af_config_t       af_conf;
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  mct_event_t       event;
  boolean           rc = TRUE;
  int               i = 0, j = 0;

  memset(&af_conf, 0, sizeof(af_config_t));

  memcpy(&af_conf, &af_out->af_stats_config, sizeof(af_config_t));
  /* Check which kernel to configure in case of multiple
     AF kernels */
  if (!af_out->af_stats_config.mask) {
    AF_ERR("Invalid AF Config mask!");
    return FALSE;
  }

  af_conf.stream_id = af_port->reserved_id;

  if (af_conf.mask & MCT_EVENT_STATS_BF_SINGLE) {
    AF_LOW("Common Parameters:\n"
      "stream_id: %d\n config_id: %d\n h_num: %d\n v_num: %d\n mask: %d\n"
      "r_min: %d b_min: %d gr_min: %d gb_min: %d",
      af_conf.stream_id, af_conf.config_id,
      af_conf.common.grid_info.h_num, af_conf.common.grid_info.v_num,
      af_conf.mask, af_conf.bf.r_min, af_conf.bf.b_min,
      af_conf.bf.gr_min, af_conf.bf.gb_min);

    AF_LOW("Primary AF Kernel:\n"
      "ROI: x: %d y: %d dx: %d dy: %d\n", af_conf.bf.roi.left,
      af_conf.bf.roi.top, af_conf.bf.roi.width, af_conf.bf.roi.height);
    AF_LOW("Primary AF HPF:");
    for (i = 0; i < MAX_HPF_BUFF_SIZE; i++) {
      AF_LOW("hpf[%d]: %d", i, af_conf.bf.hpf[i]);
    }
  }
  if (af_conf.mask & MCT_EVENT_STATS_BF_SCALE) {
    AF_LOW("Secondary AF Kernel:\n"
      "ROI: x: %d y: %d dx: %d dy: %d\n",
      af_conf.bf_scale.roi.left, af_conf.bf_scale.roi.top,
      af_conf.bf_scale.roi.width, af_conf.bf_scale.roi.height);
    AF_LOW("Secondary AF HPF:");
    for (i = 0; i < MAX_HPF_BUFF_SIZE; i++) {
      AF_LOW("hpf[%d]: %d", i, af_conf.bf.hpf[i]);
    }
    AF_LOW("Downscaling factor: %d", af_conf.bf_scale.scale);
  }
  if (af_conf.mask & MCT_EVENT_STATS_BF_FW) {
    bf_fw_config_t *fw_cfg = &af_conf.bf_fw;
    AF_LOW("floating window num_roi=%d",fw_cfg->bf_fw_roi_cfg.num_bf_fw_roi_dim);
    for (i = 0; i < (int)fw_cfg->bf_fw_roi_cfg.num_bf_fw_roi_dim; i++) {
      AF_LOW("FW roi[%d]: x=%d, y=%d, dx=%d, dy=%d", i,
        fw_cfg->bf_fw_roi_cfg.bf_fw_roi_dim[i].x, fw_cfg->bf_fw_roi_cfg.bf_fw_roi_dim[i].y,
        fw_cfg->bf_fw_roi_cfg.bf_fw_roi_dim[i].w, fw_cfg->bf_fw_roi_cfg.bf_fw_roi_dim[i].h);
    }
    for (i = 0; i < BF_FILTER_TYPE_MAX; i++) {
      AF_LOW("filter no:%d: enable=%d, h_scale_en=%d, fir_en=%d, fir_size=%d",
        i, fw_cfg->bf_filter_cfg[i].is_valid, fw_cfg->bf_filter_cfg[i].h_scale_en,
        fw_cfg->bf_filter_cfg[i].bf_fir_filter_cfg.enable,
        fw_cfg->bf_filter_cfg[i].bf_fir_filter_cfg.num_a);
      for (j = 0; j < MAX_BF_FIR_FILTER_SIZE; j++) {
        AF_LOW("fir a[%d]: %d", j,
          fw_cfg->bf_filter_cfg[i].bf_fir_filter_cfg.a[j]);
      }
      AF_LOW("iir enable=%d",
        fw_cfg->bf_filter_cfg[i].bf_iir_filter_cfg.enable);
      AF_LOW("iir a11=%f, a12=%f, a21=%f, a22=%f",
        fw_cfg->bf_filter_cfg[i].bf_iir_filter_cfg.a11,
        fw_cfg->bf_filter_cfg[i].bf_iir_filter_cfg.a12,
        fw_cfg->bf_filter_cfg[i].bf_iir_filter_cfg.a21,
        fw_cfg->bf_filter_cfg[i].bf_iir_filter_cfg.a22);
      AF_LOW("iir b10=%f b11=%f, b12=%f b20=%f a21=%f, a22=%f",
        fw_cfg->bf_filter_cfg[i].bf_iir_filter_cfg.b10,
        fw_cfg->bf_filter_cfg[i].bf_iir_filter_cfg.b11,
        fw_cfg->bf_filter_cfg[i].bf_iir_filter_cfg.b12,
        fw_cfg->bf_filter_cfg[i].bf_iir_filter_cfg.b20,
        fw_cfg->bf_filter_cfg[i].bf_iir_filter_cfg.b21,
        fw_cfg->bf_filter_cfg[i].bf_iir_filter_cfg.b22);
    }
    /* print gamma */
    AF_LOW("Gamma valid=%d, num=%d", fw_cfg->bf_gamma_lut_cfg.is_valid,
      fw_cfg->bf_gamma_lut_cfg.num_gamm_lut);
    for (j = 0; j < (int)fw_cfg->bf_gamma_lut_cfg.num_gamm_lut; j++) {
      AF_LOW("G[%d]:%d", j, fw_cfg->bf_gamma_lut_cfg.gamma_lut[j]);
    }
    /* print input config */
    AF_LOW("Input channel config valid=%d, sel=%d",
      fw_cfg->bf_input_cfg.is_valid, fw_cfg->bf_input_cfg.bf_input_g_sel);
    AF_LOW("Input channel config y_a=%f, y_b=%f, y_c=%f",
      fw_cfg->bf_input_cfg.y_a_cfg[0], fw_cfg->bf_input_cfg.y_a_cfg[1],
      fw_cfg->bf_input_cfg.y_a_cfg[2]);
    AF_LOW("Scalar config valid=%d, enable=%d, m=%d, n=%d",
      fw_cfg->bf_scale_cfg.is_valid, fw_cfg->bf_scale_cfg.bf_scale_en,
      fw_cfg->bf_scale_cfg.scale_m, fw_cfg->bf_scale_cfg.scale_n);
  }
  memcpy(&af_conf.ihist_config.roi, &af_conf.bf.roi, sizeof(cam_rect_t));
  af_conf.ihist_config.is_valid = TRUE;

  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = af_port->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_AF_CONFIG_UPDATE;
  event.u.module_event.module_event_data = (void*)&af_conf;
  event.u.module_event.current_frame_id = af_port->sof_id;

  AF_LOW("send AF_CONFIG data to ISP, port =%p, event =%p",
    port, &event);
  rc = MCT_PORT_EVENT_FUNC(port)(port, &event);

  return rc;
} /* af_port_send_stats_config */

/** af_port_send_af_info_to_metadata
 *  update af peak_location_index to metadata
 *  only AF_STATUS_FOCUSED case, the value is valid
 *  other cases the value will reset to zero
 **/
static void af_port_send_af_info_to_metadata(
  mct_port_t  *port,
  af_output_data_t *output) {
  mct_event_t             event;
  mct_bus_msg_t           bus_msg;
  af_output_eztune_data_t af_info;
  af_port_private_t       *private;
  int size;
  if (!output || !port) {
    AF_ERR("input error");
    return;
  }
  private = (af_port_private_t *)(port->port_private);
  memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.sessionid = (private->reserved_id >> 16);
  bus_msg.type = MCT_BUS_MSG_AF_EZTUNING_INFO;
  bus_msg.msg = (void *)&af_info;
  size = (int)sizeof(af_output_eztune_data_t);
  bus_msg.size = size;

  memcpy(&af_info, &output->eztune.eztune_data, size);

  AF_LOW("peak_location_index:%d", af_info.peak_location_index);
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);
  MCT_PORT_EVENT_FUNC(port)(port, &event);
}

/** af_port_handle_set_focus_mode_to_hal_type:
 * Handle AF mode event to HAL enums.
 *
 * @port: port info
 *
 * @set_parm: a message to populate
 *
 * mode: focus mode to be set
 *
 * Return: TRUE: Success  FALSE: Failure
 **/
uint8_t af_port_handle_set_focus_mode_to_hal_type(af_port_private_t *af_port,
  af_mode_type mode)
{
  uint8_t hal_mode;

  switch (mode) {
  case AF_MODE_AUTO: {
    hal_mode = CAM_FOCUS_MODE_AUTO;
  }
    break;
  case AF_MODE_INFINITY: {
    hal_mode = CAM_FOCUS_MODE_INFINITY;
  }
    break;
  case AF_MODE_MACRO: {
    hal_mode = CAM_FOCUS_MODE_MACRO;
  }
    break;
  case AF_MODE_CAF: {
    if(af_port->video_caf_mode == TRUE) {
      hal_mode = CAM_FOCUS_MODE_CONTINOUS_VIDEO;
    } else {
      hal_mode = CAM_FOCUS_MODE_CONTINOUS_PICTURE;
    }
  }
    break;
  case AF_MODE_NOT_SUPPORTED: {
    hal_mode = CAM_FOCUS_MODE_FIXED;
  }
    break;
  case AF_MODE_MANUAL: {
    hal_mode = CAM_FOCUS_MODE_OFF;
  }
    break;

  case AF_MODE_MANUAL_HAL1: {
    hal_mode = CAM_FOCUS_MODE_MANUAL;
  }
    break;
  default: {
    hal_mode = CAM_FOCUS_MODE_EDOF;
  }
    break;
  }

  return hal_mode;
}


/** af_port_send_pdaf_config:
 *    @port:   port info
 *    @af_out: output af data
 *
 * Send PDAF configuration to the other modules.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_send_pdaf_config(mct_port_t *port,
  af_output_data_t *af_out)
{
  pdaf_window_configure_t pdaf_config;
  pdaf_window_configure_t *p_config = &pdaf_config;
  af_port_private_t      *af_port = (af_port_private_t *)(port->port_private);
  mct_event_t            event;
  boolean                rc = TRUE;
  unsigned int camif_width  = af_port->camif_width;
  unsigned int camif_height = af_port->camif_height;

  /* Check values are proper */
  if (0 == camif_width || 0 == camif_height) {
    AF_ERR("camif_width or camif_height are 0! camif_width = %d, camif_height = %d",
      camif_width, camif_height);
    return FALSE;
  }

  if (af_out->pdaf_config.reset_to_default_config) {
    /* Reset to default config flag set, use default values */
    p_config->pdaf_sw_window_mode   = DEFAULT_WINDOW;
    AF_HIGH("HAF-TAF: Send default config flag ");
  } else {
    af_config_t       af_conf;
    memcpy(&af_conf, &af_out->af_stats_config, sizeof(af_config_t));
    bf_fw_config_t *fw_cfg = &af_conf.bf_fw;
    uint16 num_roi = fw_cfg->bf_fw_roi_cfg.num_bf_fw_roi_dim;

    af_out->pdaf_config.roi.y = fw_cfg->bf_fw_roi_cfg.bf_fw_roi_dim[0].y;
    af_out->pdaf_config.roi.x = fw_cfg->bf_fw_roi_cfg.bf_fw_roi_dim[0].x;
    af_out->pdaf_config.roi.height = (fw_cfg->bf_fw_roi_cfg.bf_fw_roi_dim[num_roi - 1].h +
      fw_cfg->bf_fw_roi_cfg.bf_fw_roi_dim[num_roi - 1].y - af_out->pdaf_config.roi.y) - 10;
    af_out->pdaf_config.roi.width = (fw_cfg->bf_fw_roi_cfg.bf_fw_roi_dim[num_roi - 1].w +
      fw_cfg->bf_fw_roi_cfg.bf_fw_roi_dim[num_roi - 1].x - af_out->pdaf_config.roi.x);

    AF_HIGH("HAF-TAF: PDAF ROI config MCT output: num roi: %d dy %d y %d ROI height %f",
      num_roi, fw_cfg->bf_fw_roi_cfg.bf_fw_roi_dim[num_roi - 1].h,
      fw_cfg->bf_fw_roi_cfg.bf_fw_roi_dim[num_roi - 1].y, af_out->pdaf_config.roi.height);
    AF_HIGH("HAF-TAF: PDAF ROI config MCT output: y %f %d ROI height %f x %f %d Width %f ROI_type %d ",
      af_out->pdaf_config.roi.y, fw_cfg->bf_fw_roi_cfg.bf_fw_roi_dim[0].y, af_out->pdaf_config.roi.height,
      af_out->pdaf_config.roi.x, fw_cfg->bf_fw_roi_cfg.bf_fw_roi_dim[0].x, af_out->pdaf_config.roi.width,
      af_out->pdaf_config.roi_type);
    //}
    /* grid window configuration */
    p_config->pdaf_sw_window_mode   = FIXED_GRID_WINDOW;
    p_config->fixed_grid_window_configure.window_number_hori =
      (unsigned int)af_out->pdaf_config.rgn_h_num;
    p_config->fixed_grid_window_configure.window_number_ver =
      (unsigned int)af_out->pdaf_config.rgn_v_num;
    /* Note: PDAF Lib and sensor expects CAMIF size ratio coordinates, not absolute coordinate */
    p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_start_hori =
      (af_out->pdaf_config.roi.x / (float)camif_width);
    p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_start_ver =
      (af_out->pdaf_config.roi.y / (float)camif_height);
    p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_end_hori =
      (af_out->pdaf_config.roi.x + af_out->pdaf_config.roi.width)
       / ((float)camif_width);
    p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_end_ver =
      (af_out->pdaf_config.roi.y + af_out->pdaf_config.roi.height)
       / ((float)camif_height);
    p_config->fixed_grid_window_configure.roi_type = af_out->pdaf_config.roi_type;

    AF_HIGH("HAF-TAF: PDAF ROI config MCT output: (ratio to camif)"
      "x = %f, y = %f, end_x = %f, end_y = %f, "
      "dx = %f, dy = %f, camif_width = %d, camif_height = %d,"
      " H_num = %d, V_num = %d, p: %p",
      p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_start_hori,
      p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_start_ver,
      p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_end_hori,
      p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_end_ver,
        af_out->pdaf_config.roi.width,
        af_out->pdaf_config.roi.height,
        camif_width, camif_height,
      p_config->fixed_grid_window_configure.window_number_hori,
        p_config->fixed_grid_window_configure.window_number_ver,
        &p_config);
  }

  /* Boundary checks */
  if (p_config->pdaf_sw_window_mode   != DEFAULT_WINDOW &&
      (p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_start_hori < 0.0f ||
      p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_start_ver < 0.0f ||
      p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_end_hori <= 0.0f ||
      p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_end_ver <= 0.0f ||
      af_out->pdaf_config.roi.width  <= 0.0f ||
      af_out->pdaf_config.roi.height <= 0.0f ||
      p_config->fixed_grid_window_configure.window_number_hori <= 0 ||
      p_config->fixed_grid_window_configure.window_number_ver <= 0)) {
      AF_ERR("ERROR! Invalid PDAF ROI config output caught! Cancel sending config");
      AF_LOW("ERROR! HAF-TAF: PDAF ROI config MCT output: "
        "x = %f, y = %f, end_x = %f, end_y = %f, "
        "dx = %f, dy = %f, H_num = %f, V_num = %f, p: %p",
         p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_start_hori,
         p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_start_ver,
         p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_end_hori,
         p_config->fixed_grid_window_configure.af_fix_window.pdaf_address_end_ver,
           af_out->pdaf_config.roi.width,
           af_out->pdaf_config.roi.height,
         p_config->fixed_grid_window_configure.window_number_hori,
         p_config->fixed_grid_window_configure.window_number_ver,
           &p_config);

      rc = FALSE;
      return rc;
  }

  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = af_port->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_PDAF_AF_WINDOW_UPDATE;
  event.u.module_event.module_event_data = (void *)(&pdaf_config);
  event.u.module_event.current_frame_id = af_port->sof_id;
  AF_LOW("Send PDAF ROI config, port =%p, event =%p",
    port, &event);

  rc = MCT_PORT_EVENT_FUNC(port)(port, &event);
  return rc;
} /* af_port_send_pdaf_config */

/** af_port_send_dciaf_config:
 *    @port:   port info
 *    @af_out: output af data
 *
 * Send software AF configuration to the AFS module.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_send_dciaf_config(mct_port_t *port,
  af_output_data_t *af_out)
{
  mct_imglib_af_config_t af_config;
  mct_imglib_dciaf_config_t *imglib_dciaf_config = &(af_config.u.dciaf);
  af_config_t            af_conf;
  af_port_private_t      *af_port = (af_port_private_t *)(port->port_private);
  mct_event_t            event;
  boolean                rc = TRUE;

  af_config.type = AF_CFG_DCIAF;
  /* Copy the Dual Camera AF configuration info from the algorithm output */

  imglib_dciaf_config->focal_length_ratio  = af_out->dciaf_config.focal_length_ratio;
  imglib_dciaf_config->roi.left   = af_out->dciaf_config.roi.x;
  imglib_dciaf_config->roi.top    = af_out->dciaf_config.roi.y;
  imglib_dciaf_config->roi.width  = af_out->dciaf_config.roi.width;
  imglib_dciaf_config->roi.height = af_out->dciaf_config.roi.height;
  imglib_dciaf_config->is_af_search_active = af_out->dciaf_config.is_af_search_active;
  imglib_dciaf_config->requireDistanceInfo = af_out->dciaf_config.requireDistanceInfo;

  /* Copy dual-cam fields for metadata */
  af_port->focal_length_ratio     = af_out->dciaf_config.focal_length_ratio;

  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = af_port->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_IMGLIB_AF_CONFIG;
  event.u.module_event.module_event_data = (void *)(&af_config);
  event.u.module_event.current_frame_id = af_port->sof_id;
  AF_LOW("Send software AF CONFIG data to SAF, port =%p, event =%p",
  port, &event);

  rc = MCT_PORT_EVENT_FUNC(port)(port, &event);
  return rc;
} /* af_port_send_dciaf_config */

/** af_port_send_af_config_to_imagelib:
 *    @port:   port info
 *    @af_out:   af out info
 * Send  AF Focus Parameters to the AFS module.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_send_af_config_to_imagelib(mct_port_t *port, af_output_data_t *af_out)
{
  mct_imglib_af_config_t af_config;
  mct_imglib_af_focus_config_t *imglib_af_config = &(af_config.u.affocus);
  af_port_private_t      *af_port = (af_port_private_t *)(port->port_private);
  mct_event_t            event;
  boolean                rc = TRUE;
  (af_out);
  af_config.type = AF_CFG_FOCUS;

  /* Copy the AF  Focus configuration info from the algorithm output */
  imglib_af_config->far_field_cm = af_port->current_dist_far_in_cm;
  imglib_af_config->near_field_cm = af_port->current_dist_near_in_cm;
  imglib_af_config->lens_shift_um = af_port->lens_shift_um;
  imglib_af_config->focal_length_ratio = af_port->focal_length_ratio;
  imglib_af_config->object_distance_cm = af_port->currentdistanceOfObjectInCm;

  imglib_af_config->af_supported        = !(af_port->af_not_supported);
  imglib_af_config->roi.left            = af_port->bus.default_roi.left;
  imglib_af_config->roi.top             = af_port->bus.default_roi.top;
  imglib_af_config->roi.width           = af_port->bus.default_roi.width;
  imglib_af_config->roi.height          = af_port->bus.default_roi.height;

  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = af_port->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_IMGLIB_AF_CONFIG;
  event.u.module_event.module_event_data = (void *)(&af_config);
  event.u.module_event.current_frame_id = af_port->sof_id;
  AF_LOW("port = %p, event = %p", port, &event);
  AF_HIGH("H = %d, W = %d, T = %d, L = %d, FL_ratio = %f",
  imglib_af_config->roi.height, imglib_af_config->roi.width, imglib_af_config->roi.top, imglib_af_config->roi.left, imglib_af_config->focal_length_ratio);

  rc = MCT_PORT_EVENT_FUNC(port)(port, &event);
  return rc;
} /* af_port_send_af_config_to_imagelib */

/** af_port_send_afs_config:
 *    @port:   port info
 *    @af_out: output af data
 *
 * Send software AF configuration to the AFS module.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_send_afs_config(mct_port_t *port,
  af_output_data_t *af_out)
{
  mct_imglib_af_config_t af_config;
  mct_imglib_swaf_config_t *imglib_af_config = &(af_config.u.swaf);
  af_config_t            af_conf;
  af_port_private_t      *af_port = (af_port_private_t *)(port->port_private);
  mct_event_t            event;
  boolean                rc = TRUE;

  af_config.type = AF_CFG_SWAF;
  /* Copy the ASF configuration info from the algorithm output */
  imglib_af_config->enable     = af_out->swaf_config.enable;
  imglib_af_config->frame_id   = af_out->swaf_config.frame_id;
  imglib_af_config->coeff_len  = af_out->swaf_config.coeff_len;
  imglib_af_config->roi.left   = af_out->swaf_config.roi.x;
  imglib_af_config->roi.top    = af_out->swaf_config.roi.y;
  imglib_af_config->roi.width  = af_out->swaf_config.roi.dx;
  imglib_af_config->roi.height = af_out->swaf_config.roi.dy;
  imglib_af_config->filter_type = af_out->swaf_config.sw_filter_type;
  imglib_af_config->FV_min     = af_out->swaf_config.fv_min;

  memcpy(imglib_af_config->coeffa, &af_out->swaf_config.coeffa,
    sizeof(double) * MAX_SWAF_COEFFA_NUM);
  memcpy(imglib_af_config->coeffb, &af_out->swaf_config.coeffb,
    sizeof(double) * MAX_SWAF_COEFFB_NUM);
  memcpy(imglib_af_config->coeff_fir, &af_out->swaf_config.coeff_fir,
    sizeof(int) * MAX_SWAF_COEFFFIR_NUM);

  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = af_port->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_IMGLIB_AF_CONFIG;
  event.u.module_event.module_event_data = (void *)(&af_config);
  event.u.module_event.current_frame_id = af_port->sof_id;
  AF_LOW("Send software AF CONFIG data to SAF, port =%p, event =%p",
    port, &event);

  rc = MCT_PORT_EVENT_FUNC(port)(port, &event);

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_SW_STATS_POINTER, af_port);
  if (af_msg == NULL) {
    return FALSE;
  }

  af_msg->u.af_set_parm.u.p_sw_stats = af_port->p_sw_stats;
  q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  af_port_send_sleep_if_required(port,af_out->swaf_config.enable);

  return rc;
} /* af_port_send_afs_config */

/** af_port_send_ISP_reconfig_msg:
 *    @af_port:   private port data
 *
 *  Send a message to the AF algorithm to request ISP AF stats reconfiguration
 *  after each streamon.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_send_ISP_reconfig_msg(af_port_private_t *af_port) {
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_RECONFIG_ISP, af_port);
  if (af_msg == NULL) {
    AF_ERR("Not Enough memory.");
    return FALSE;
  }

  return q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
}


/** af_port_send_exif_debug_data:
 *    @port:   TODO
 *    @stats_update_t: TODO
 *
 * TODO description
 *
 * Return nothing
 **/
static void af_port_send_exif_debug_data(mct_port_t *port)
{
  mct_event_t          event;
  mct_bus_msg_t        bus_msg;
  cam_af_exif_debug_t  *af_info;
  af_port_private_t    *private;
  int                  size;

  if (!port) {
    AF_ERR("input error");
    return;
  }
  private = (af_port_private_t *)(port->port_private);
  if (private == NULL) {
    return;
  }


  /* Send exif data if data size is valid */
  af_debug_info_t *debug_info = &private->debug_info;

  if (debug_info->enable) {
    af_info = (cam_af_exif_debug_t *)malloc(sizeof(cam_af_exif_debug_t));
    if (!af_info) {
      AF_ERR("Failure allocating memory for debug data");
      return;
    }
    memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
    bus_msg.sessionid = (private->reserved_id >> 16);
    bus_msg.type = MCT_BUS_MSG_AF_EXIF_DEBUG_INFO;
    bus_msg.msg = (void *)af_info;
    size = (int)sizeof(cam_af_exif_debug_t);
    bus_msg.size = AF_DEBUG_DATA_SIZE;
    memset(af_info, 0, size);
    af_info->af_debug_data_size = AF_DEBUG_DATA_SIZE;
    AF_LOW("debug data size: af: %d", af_info->af_debug_data_size);

    memcpy(af_info->af_private_debug_data,
        debug_info->debug_data, AF_DEBUG_DATA_SIZE);
    event.direction = MCT_EVENT_UPSTREAM;
    event.identity = private->reserved_id;
    event.type = MCT_EVENT_MODULE_EVENT;
    event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
    event.u.module_event.module_event_data = (void *)(&bus_msg);
    MCT_PORT_EVENT_FUNC(port)(port, &event);
    if (af_info) {
      free(af_info);
    }
  }
}

boolean af_port_buf_div_init_buf_divert_ack_ctrl(af_port_private_t *af_port,
  mct_bf_pd_stats_data_t *pd_data, isp_buf_divert_t *isp_buf_divert)
{
  uint32_t max_pdaf_buffers = 0;

  if (pd_data) {
    max_pdaf_buffers = pd_data->max_buffers;
  } else if (isp_buf_divert) {
    max_pdaf_buffers = isp_buf_divert->max_buffers;
  }

  if (max_pdaf_buffers == 0) {
    AF_LOW("PDAF stats not available");
    return FALSE;
  }

  if (af_port->div_buf_ack.max_buffers == 0) {
    AF_LOW("Init buf_div: max_buf: %d, pd_data_max_buffers: %d, ack_ctrl: %p",
      af_port->div_buf_ack.max_buffers,
      max_pdaf_buffers,
      af_port->div_buf_ack.div_buf_ack_ctrl);
    /* Allocate buffers for the first time */
    af_port->div_buf_ack.div_buf_ack_ctrl =
      calloc(1, max_pdaf_buffers * sizeof(af_port_div_buf_ack_ctrl_t));
    if (NULL == af_port->div_buf_ack.div_buf_ack_ctrl) {
      AF_ERR("Fail to allocate div_buf_ack_ctrl");
      return FALSE;
    }

    af_port->div_buf_ack.max_buffers = max_pdaf_buffers;
    AF_LOW("Init buf_div: max_buf: %d, max_pdaf_buffers: %d, ack_ctrl: %p",
      af_port->div_buf_ack.max_buffers, max_pdaf_buffers, af_port->div_buf_ack.div_buf_ack_ctrl);
  }

  return TRUE;
}

boolean af_port_buf_div_add_pd_early(af_port_private_t *af_port,
  mct_bf_pd_stats_data_t *pd_data)
{
  if (af_port->div_buf_ack.max_buffers == 0 || pd_data->max_buffers == 0) {
    /* PD stats may not be enable, return without error */
    return TRUE;
  }

  if (pd_data->buf_idx >= af_port->div_buf_ack.max_buffers) {
    AF_ERR("Error: unexpected PD buf index: %d (%d max)", pd_data->buf_idx,
      af_port->div_buf_ack.max_buffers);
    return FALSE;
  }

  pthread_mutex_lock(&(af_port->div_buf_ack.mutex));
  AF_LOW("pd_early:add: idx %d, pd_buf %p, data_len %d, max_buffers %d",
    pd_data->buf_idx, pd_data->pd_buffer_data,
    pd_data->pd_stats_read_data_length, pd_data->max_buffers);
  af_port->div_buf_ack.div_buf_ack_ctrl[pd_data->buf_idx].is_early_pd_buf_in_use = TRUE;
  pthread_mutex_unlock(&(af_port->div_buf_ack.mutex));

  return TRUE;
}

boolean af_port_buf_div_pd_early_done(af_port_private_t *af_port,
  mct_bf_pd_stats_data_t *pd_data)
{
  if (af_port->div_buf_ack.max_buffers == 0 || pd_data->max_buffers == 0) {
    /* PD stats may not be enable, return without error */
    return TRUE;
  }

  if (pd_data->buf_idx >= af_port->div_buf_ack.max_buffers) {
    AF_ERR("Error: unexpected PD buf index");
    return FALSE;
  }

  pthread_mutex_lock(&(af_port->div_buf_ack.mutex));
  AF_LOW("pd_early:done: idx %d, pd_buf %p, data_len %d!",
    pd_data->buf_idx, pd_data->pd_buffer_data, pd_data->pd_stats_read_data_length);
  af_port->div_buf_ack.div_buf_ack_ctrl[pd_data->buf_idx].is_early_pd_buf_in_use = FALSE;
  pthread_mutex_unlock(&(af_port->div_buf_ack.mutex));

  return TRUE;
}

boolean af_port_buf_div_done(af_port_private_t *af_port,
  isp_buf_divert_t *stats_buff)
{
  uint32_t buf_idx;

  if (!stats_buff) {
      AF_ERR("stats_buff is null");
      return FALSE;
  }

  buf_idx = stats_buff->buffer.index;
  if (buf_idx >= af_port->div_buf_ack.max_buffers) {
    AF_ERR("Error: Unexpected buffer idx: %d", buf_idx);
    return FALSE;
  }

  pthread_mutex_lock(&(af_port->div_buf_ack.mutex));
  AF_LOW("buf_div:done: idx: %d", buf_idx);
  af_port->div_buf_ack.div_buf_ack_ctrl[buf_idx].is_buf_div_done = TRUE;
  pthread_mutex_unlock(&(af_port->div_buf_ack.mutex));

  return TRUE;
}


boolean af_port_buf_touch_in_use(af_port_private_t *af_port,
  isp_buf_divert_t *stats_buff)
{
  uint32_t buf_idx;

  if (!stats_buff) {
    AF_ERR("stats_buff is null");
    return FALSE;
  }

  buf_idx = stats_buff->buffer.index;
  if (buf_idx >= af_port->div_buf_ack.max_buffers) {
    AF_ERR("Error: Unexpected buffer idx: %d", buf_idx);
    return FALSE;
  }

  pthread_mutex_lock(&(af_port->div_buf_ack.mutex));
  AF_LOW("touch in use frame id %d idx: %d",
    stats_buff->buffer.sequence, buf_idx);
  af_port->div_buf_ack.div_buf_ack_ctrl[buf_idx].zero_touch_lag.is_touch_buf_done = FALSE;
  pthread_mutex_unlock(&(af_port->div_buf_ack.mutex));

  return TRUE;
}

boolean af_port_buf_touch_done(af_port_private_t *af_port,
  isp_buf_divert_t *stats_buff)
{
  uint32_t buf_idx;
  AF_HIGH("E");
  if (!stats_buff) {
    AF_ERR("stats_buff is null");
    return FALSE;
  }
  AF_HIGH("stats_buff %p ",
    stats_buff);
  buf_idx = stats_buff->buffer.index;
  if (buf_idx >= af_port->div_buf_ack.max_buffers) {
    AF_ERR("Error: Unexpected buffer idx: %d", buf_idx);
    return FALSE;
  }

  if (NULL == af_port->div_buf_ack.div_buf_ack_ctrl) {
    AF_ERR("div_buf_ack_ctrl is null");
    return FALSE;
  }
  AF_HIGH("stats_buff %p touch buf done frame id %d idx: %d",
    stats_buff, stats_buff->buffer.sequence, buf_idx);
  pthread_mutex_lock(&(af_port->div_buf_ack.mutex));
  af_port->div_buf_ack.div_buf_ack_ctrl[buf_idx].zero_touch_lag.is_touch_buf_done = TRUE;
  pthread_mutex_unlock(&(af_port->div_buf_ack.mutex));

  return TRUE;
}

static void af_port_send_isp_buffer_divert_ack(mct_port_t *port,
  isp_buf_divert_t *stats_buff)
{
  mct_event_t           event;
  isp_buf_divert_ack_t  isp_ack;

  if (!port || !stats_buff) {
    AF_ERR("input error");
    return;
  }

  AF_LOW("Send ISP buffer back, identity: 0x%x frame_id: %d, buf_idx: %d pd_stat: %p",
    stats_buff->identity, stats_buff->buffer.sequence, stats_buff->buffer.index, stats_buff->vaddr);

  isp_ack.identity = stats_buff->identity;
  isp_ack.is_buf_dirty = TRUE;
  isp_ack.frame_id = stats_buff->buffer.sequence;
  isp_ack.channel_id = stats_buff->channel_id;
  isp_ack.buf_idx = stats_buff->buffer.index;
  isp_ack.timestamp = stats_buff->buffer.timestamp;
  isp_ack.meta_data = stats_buff->meta_data;
  isp_ack.bayerdata = FALSE;
  isp_ack.buffer_access = FALSE;

  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = isp_ack.identity;
  event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT_ACK;
  event.u.module_event.module_event_data = &isp_ack;

  MCT_PORT_EVENT_FUNC(port)(port, &event);
}

boolean af_port_pd_zero_touch_lag_update(af_port_private_t *af_port,
  af_port_pdaf_zero_touch_lag_field_t field_type,
  const pd_zero_touch_lag_info_t* info)
{
  boolean rc = TRUE;

  if (NULL == af_port || NULL == info) {
    AF_ERR("Null pointer info %p info %p", af_port, info);
    return FALSE;
  }

  pthread_mutex_lock(&(af_port->pd_zero_touch_lag.mutex));
  switch (field_type) {
    case AF_PD_TOUCH_ROI_TYPE: {
      af_port->pd_zero_touch_lag.info.roi_type = info->roi_type;
      memcpy(&af_port->pd_zero_touch_lag.info.pdaf_config,
        &info->pdaf_config, sizeof(pdaf_window_configure_t));
      AF_HIGH("ROI type %d", info->roi_type);
    }
    break;
    case AF_IS_FIRST_TOUCH: {
      af_port->pd_zero_touch_lag.info.is_it_first_touch = info->is_it_first_touch;
      AF_HIGH("is_it_first_touch %d ROI type %d ",
        info->is_it_first_touch, info->roi_type);
    }
    break;
    case AF_IS_FIRST_FRAME_TOUCH: {
      af_port->pd_zero_touch_lag.info.is_it_first_frame =
        info->is_it_first_frame;
      AF_HIGH("is_it_first_frame %d", info->is_it_first_frame);
    }
    break;
    case AF_PD_TOUCH_PREV_FRAME_BUF_IDX: {
      int8 buf_idx = info->prev_frame_idivert_buf_idx;

      // -1 is initial value when there is no valid previous frame
      if (buf_idx != -1) {
        if ((uint8)buf_idx >= af_port->div_buf_ack.max_buffers) {
          AF_ERR("Error: unexpected PD buf index");
          rc = FALSE;
          break;
        }
      }

      AF_HIGH("Update previous frame buf idx %d", buf_idx);
      af_port->pd_zero_touch_lag.info.prev_frame_idivert_buf_idx = buf_idx;
    }
    break;
    default: {
      AF_LOW("No valid field name!");
    }
    break;

  }
  pthread_mutex_unlock(&(af_port->pd_zero_touch_lag.mutex));
  return rc;
}


boolean af_port_pd_zero_touch_lag_access(af_port_private_t *af_port,
  pd_zero_touch_lag_info_t* info)
{

  if (NULL == af_port || NULL == info) {
    AF_ERR("Null pointer info %p info %p", af_port, info);
    return FALSE;
  }

  pthread_mutex_lock(&(af_port->pd_zero_touch_lag.mutex));

  memcpy(info, &af_port->pd_zero_touch_lag.info, sizeof(pd_zero_touch_lag_info_t));

  pthread_mutex_unlock(&(af_port->pd_zero_touch_lag.mutex));
  return TRUE;
}

void af_port_pd_early_res_update(af_port_private_t *af_port,
  af_port_pdaf_early_res_field_t field_type,
  const pd_lib_early_result_t* info)
{
  if (NULL == af_port || NULL == info) {
    AF_ERR("Null pointer info %p info %p", af_port, info);
    return;
  }

  pthread_mutex_lock(&(af_port->pd_lib_early_result.mutex));
  switch (field_type) {
    case AF_PDLIB_PROCESS_RES: {
      af_port->pd_lib_early_result.pdaf_processing_result_early_int =
        info->pdaf_processing_result_early_int;
      AF_HIGH("pdaf_processing_result_early_int %d",
        info->pdaf_processing_result_early_int);
    }
    break;
    case AF_PDLIB_BOTTOM_MULTI_WIN_PROCESS: {
      af_port->pd_lib_early_result.process_bottom_multi_window =
        info->process_bottom_multi_window;
      AF_HIGH("process_bottom_multi_window %d",
        info->process_bottom_multi_window);
    }
    break;
    case ALL: {
      af_port->pd_lib_early_result.pdaf_processing_result_early_int =
        info->pdaf_processing_result_early_int;
      af_port->pd_lib_early_result.process_bottom_multi_window =
        info->process_bottom_multi_window;
      AF_HIGH("pdaf_processing_result_early_int %d process_bottom_multi_window %d",
        info->pdaf_processing_result_early_int, info->process_bottom_multi_window);
    }
    break;
    default: {
      AF_LOW("No valid field name!");
    }
    break;
  }
  pthread_mutex_unlock(&(af_port->pd_lib_early_result.mutex));
}


void af_port_pd_early_res_access(af_port_private_t *af_port,
  pd_lib_early_result_t* info)
{

  if (NULL == af_port || NULL == info) {
    AF_ERR("Null pointer info %p info %p", af_port, info);
    return;
  }

  pthread_mutex_lock(&(af_port->pd_lib_early_result.mutex));

  memcpy(info, &af_port->pd_lib_early_result, sizeof(pd_lib_early_result_t));

  pthread_mutex_unlock(&(af_port->pd_lib_early_result.mutex));
}

boolean af_port_buf_div_send_ack_if_ready(mct_port_t *port,
  uint32_t buf_idx)
{
  boolean rc = FALSE;
  mct_event_t event;
  af_port_div_buf_ack_ctrl_t *ack_ctrl = NULL;
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);

  if (af_port->div_buf_ack.max_buffers == 0) {
    /* PD stats may not be enable, return without error */
    return TRUE;
  }

  if (buf_idx >= af_port->div_buf_ack.max_buffers) {
    AF_ERR("Error: invalid buffer index: %d", buf_idx);
    return FALSE;
  }

  pthread_mutex_lock(&(af_port->div_buf_ack.mutex));
  ack_ctrl = &af_port->div_buf_ack.div_buf_ack_ctrl[buf_idx];
  if (TRUE == ack_ctrl->is_early_pd_buf_in_use ||
      FALSE == ack_ctrl->is_buf_div_done) {
    pthread_mutex_unlock(&(af_port->div_buf_ack.mutex));
    return TRUE; /* no error, but don't send ack */
  }
  /* Reset flag */
  ack_ctrl->is_buf_div_done = FALSE;
  pthread_mutex_unlock(&(af_port->div_buf_ack.mutex));

  af_port_send_isp_buffer_divert_ack(port, &ack_ctrl->isp_buf_divert);

  return rc;
}

static void af_port_stats_done_callback(void* p, void* stats)
{
  mct_port_t        *port = (mct_port_t *)p;
  af_port_private_t *private = NULL;
  stats_af_t*       af_stats = stats;

  if (NULL == port || NULL == port->port_private || NULL == af_stats) {
    AF_ERR("input error");
    return;
  }
  private = (af_port_private_t *)(port->port_private);

  if (FALSE == af_stats->isFake &&
    af_stats->stats_type_mask | STATS_BF &&
    af_stats->u.p_q3a_bf_stats[AF_KERNEL_PRIMARY]->mask == MCT_EVENT_STATS_BF_FW) {
    /* Send buffer divert ACK if ready */
    mct_bf_pd_stats_data_t *pd_data =
      &af_stats->u.p_q3a_bf_stats[AF_KERNEL_PRIMARY]->u.fw_stats.pd_data;

    af_port_buf_div_pd_early_done(private, pd_data);
    af_port_buf_div_send_ack_if_ready(port, pd_data->buf_idx);
  }

  AF_LOW("DONE AF STATS ACK back");
  circular_stats_data_done(af_stats->ack_data, port,
                              private->reserved_id, private->sof_id);
}

/** af_port_is_mode_continous:
 * @mode: AF mode
 *
 * return: TRUE if the <mode> is continous and FALSE if not.
 **/
static boolean af_port_is_mode_continous(cam_focus_mode_type mode)
{
  if (mode == CAM_FOCUS_MODE_CONTINOUS_VIDEO ||
      mode == CAM_FOCUS_MODE_CONTINOUS_PICTURE)
    return TRUE;
  else
    return FALSE;
}

/** af_port_update_fovc_factor:
 *    @port:   port info
 *    @af_out: output af data
 *
 * Send resolution coordinates to CPP or ISP to crop
 *
 * Return TRUE on success, FALSE on failure.
 **/
static void af_port_update_fovc_factor(mct_port_t *port,
  af_output_data_t *af_out)
{
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  mct_event_t event;
  float crop_factor = af_out->move_lens.mag_factor;
  int frame_adjustment = 0;
  af_port->fovc_data.mag_factor = crop_factor;
  AF_LOW("crop_factor: %f", crop_factor);
  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = af_port->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = (FOVC_MODE_ISP == af_port->fovc_data.fovc_mode) ?
    MCT_EVENT_MODULE_STATS_FOVC_MAGNIFICATION_FACTOR_ISP : MCT_EVENT_MODULE_STATS_FOVC_MAGNIFICATION_FACTOR;
  event.u.module_event.module_event_data = (void *)(&crop_factor);
  frame_adjustment =
    (af_port->applying_delay > 0) ? af_port->applying_delay - 1 : AF_PORT_FOVC_CPP_FRAME_ADJUSTMENT;
  event.u.module_event.current_frame_id = (FOVC_MODE_ISP == af_port->fovc_data.fovc_mode) ?
    af_port->sof_id : (MAX((int)af_port->sof_id - frame_adjustment, 0));

  AF_LOW("fovc crop factor=%f, sof_id=%d framed_id=%d, identity=0x%x",
    crop_factor,
    af_port->sof_id,
    event.u.module_event.current_frame_id,
    af_port->reserved_id);

  MCT_PORT_EVENT_FUNC(port)(port, &event);
}

/** af_port_callback: Callback function once AF module has some
 *  updates to send.
 *
 * Callback fucntion from AF algorithm.
 *
 * Return void
 **/
static void af_port_callback(af_output_data_t *af_out, void *p)
{
  mct_port_t        *port = (mct_port_t *)p;
  af_port_private_t *private = NULL;
  mct_event_t       event;
  stats_update_t    stats_update;
  af_update_t       af_update;
  boolean           send_update = FALSE;
  boolean           rc;
  boolean mode_change = FALSE;
  stats_ext_return_type ret;

  if (!af_out || !port || !port->port_private) {
    AF_ERR("input error: port= %p, af_out= %p", port, af_out);
    return;
  }
  private = (af_port_private_t *)(port->port_private);

  /* First reset AF update data structure */
  memset(&af_update, 0, sizeof(af_update_t));

  /* We'll process the output only if output has been updated
   * Should we keep the following order when we process the output?
   * Need to check.
   **/
  AF_LOW("Handle cb output: %p", af_out);

  if (af_out->sof_id) {
    private->sof_id = af_out->sof_id;
    private->focus_value = af_out->focus_value;
  }

  /* First handle callback in extension if available */
  if (private->func_tbl.ext_callback) {
    ret = private->func_tbl.ext_callback(port, af_out, &af_update);
    if (STATS_EXT_HANDLING_COMPLETE == ret) {
      AF_LOW("Callback handled. Skipping rest!");
      return;
    }
  }

  AF_LOW("output: sof_id: %d, result: %d, mask: 0x%x",
    af_out->sof_id, af_out->result, af_out->type);
  if (af_out->result) {
    if (af_out->type & AF_OUTPUT_FOCUS_VALUE) {
      /* Fill the original status first */
      af_update.af_status = private->af_status;
    }
    if (af_out->type & AF_OUTPUT_STATUS) {
      /* Update AF status event.
       * For AF status, we directly post it to bus
       **/
      af_port_process_status_update(port, af_out);
      af_update.af_status = af_out->focus_status.status;
      if (af_out->focus_status.status == AF_STATUS_FOCUSING) {
        af_update.af_active = TRUE;
      }
      send_update = TRUE;
    }
    if (af_out->type & AF_OUTPUT_SPOT_LIGHT_DETECTION) {
      private->spot_light_detected = af_out->spot_light_detected;
    }
    /* Update AF ROI event. */
    if (af_out->type & AF_OUTPUT_STATS_CONFIG) {
      /* Send event to ISP to configure new AF ROI and
       * update image lib with new ROI
       **/
      af_port_update_roi_info(port, af_out);
      af_port_send_af_config_to_imagelib(port, af_out);
      af_port_send_stats_config(port, af_out);
    }

    if (af_out->type & AF_OUTPUT_SWAF_CONFIG) {
      /* Send event to AFS module to configure frame assisted AF */
      af_port_send_afs_config(port, af_out);
      /* At the end of focus completion, imglib will need
       * details about the ROI and focal length ratio
       * for the settled lens position. So, sending the event here*/
      af_port_send_af_config_to_imagelib(port, af_out);
    }

    if (af_out->type & AF_OUTPUT_PDAF_CONFIG) {
      /* Send event to sensor port to configure PDAF ROI */
      AF_LOW("Send event to sensor module to configure PDAF ROI");
      af_port_send_pdaf_config(port, af_out);
    }

    if (af_out->type & AF_OUTPUT_DCIAF_CONFIG) {
      /* Send event to DCRF module to configure Dual camera assisted AF */
      af_port_send_dciaf_config(port, af_out);
    }

    if (af_out->type & AF_OUTPUT_EXP_COMPENSATE) {
      /* pack into an mct_event object*/
      event.direction = MCT_EVENT_UPSTREAM;
      event.identity = private->reserved_id;
      event.type = MCT_EVENT_MODULE_EVENT;
      event.u.module_event.type = MCT_EVENT_MODULE_AF_EXP_COMPENSATE;
      event.u.module_event.module_event_data = (void *)(&af_out->exp_comp_enable);
      event.u.module_event.current_frame_id = private->sof_id;
      AF_LOW("Send AF exposure compensation configure: %s to ISP ",
        af_out->exp_comp_enable ? "Enable" : "Disable");
      rc = MCT_PORT_EVENT_FUNC(port)(port, &event);
    }

    if (af_out->type & AF_OUTPUT_STOP_AF) {
      /* stop AF. */
      af_update.stop_af = af_out->stop_af;
      send_update = TRUE;
    }

    if (af_out->type & AF_OUTPUT_CHECK_LED) {
      /* Send event to AEC to turn ON LED if required */
      af_update.check_led = af_out->check_led;
      send_update = TRUE;
    }
    if (af_out->type & AF_OUTPUT_MOVE_LENS) {
      send_update = TRUE;
      /* Check if fovc compensation is available */
      if ((af_out->move_lens.fovc_enabled && af_out->move_lens.mag_factor)
          || (!af_out->move_lens.fovc_enabled && private->fovc_data.mag_factor!=0)) {
        /* This needs to be set based on where FOV crop is being
           handled. If it's ISP we'll need to account for pipeline
           delay and skip number of frames */
        if (FOVC_MODE_ISP == private->fovc_data.fovc_mode ) {
          private->fovc_data.delay_lens_move = TRUE;
        }
        af_port_update_fovc_factor(port, af_out);
      }

      /* If pipeline delay needs to be accounted for,
         delay lens move */
      if (TRUE == private->fovc_data.delay_lens_move) {
        AF_LOW("FOVC: Delay lens move!");
        private->fovc_data.frame_skip_cnt++;
        private->fovc_data.move_lens = af_out->move_lens;
      }else {
        if (TRUE == af_out->move_lens.reset_lens) {
          /* Ask sensor module to reset the lens */
          AF_LOW("Asking actuator to reset the lens!");
          af_update.reset_lens = af_out->move_lens.reset_lens;
          private->af_focus_pos =
            private->af_algo_tuning.position_far_end;
        } else if (TRUE == af_out->move_lens.move_lens){
          /* If we have to move the lens, ask sensor module */
          if (af_out->move_lens.use_dac_value == FALSE) {
            af_update.use_dac_value = FALSE;
            af_update.move_lens = af_out->move_lens.move_lens;
            af_update.direction = af_out->move_lens.direction;
            af_update.num_of_steps = af_out->move_lens.num_of_steps;

            // update the focus pos with the moved steps
            private->af_focus_pos = af_out->move_lens.cur_pos;
            if (af_update.direction == AF_MOVE_FAR) {
              private->af_focus_pos += af_update.num_of_steps;
            } else {
              private->af_focus_pos -= af_update.num_of_steps;
            }
            af_update.cur_logical_lens_pos = private->af_focus_pos;

            AF_LOW("Asking actuator to move the lens! flag: %d,\
              steps: %d, dir: %d cur_pos %d, new_pos=%d",
              af_update.move_lens,
              af_update.num_of_steps,
              af_update.direction, af_out->move_lens.cur_pos,
              private->af_focus_pos);
          } else {
            unsigned int i;
            AF_LOW("Use DAC. Num of steps: %d", af_out->move_lens.num_of_interval);
            af_update.move_lens = af_out->move_lens.move_lens;
            af_update.use_dac_value = TRUE;
            af_update.num_of_interval = af_out->move_lens.num_of_interval;
            for (i = 0; i < af_update.num_of_interval; i++) {
              af_update.pos[i] = af_out->move_lens.pos[i];
              af_update.delay[i] = af_out->move_lens.delay[i];
              AF_LOW("pos[%d] = %d delay[%d] = %d", i, af_update.pos[i], i, af_update.delay[i]);
            }
            // there is strong logic needed here.
            private->af_focus_pos = af_update.pos[0];
          }
        }
      }
      private->diopter =
        af_port_convert_index_to_diopter(private, private->af_focus_pos);
      private->scale_ratio =
        af_port_convert_index_to_scale(private, private->af_focus_pos);
      if (((af_out->type & AF_OUTPUT_MOVE_LENS) &&
        af_out->move_lens.use_dac_value == FALSE) ||
        (TRUE == af_out->move_lens.reset_lens)) {
        private->focus_distance_info.focus_distance\
          [CAM_FOCUS_DISTANCE_OPTIMAL_INDEX] =
          af_port_get_optimal_focus_distance(private, private->af_focus_pos);

        if ((int32_t)private->focus_distance_info.focus_distance\
          [CAM_FOCUS_DISTANCE_OPTIMAL_INDEX]== AF_PORT_MAX_FOCUS_DISTANCE) {
          // infinity
          private->focus_distance_info.focus_distance\
            [CAM_FOCUS_DISTANCE_OPTIMAL_INDEX] = 0;
          private->focus_distance_info.focus_distance\
            [CAM_FOCUS_DISTANCE_NEAR_INDEX] = 0;
          private->focus_distance_info.focus_distance\
            [CAM_FOCUS_DISTANCE_FAR_INDEX] = 0;
        } else {
          af_port_get_near_far_field_focus_distance(
            private, &private->focus_distance_info);
        }
      }

    }
  }


  if (!private->af_initialized) {
    private->bus.focus_mode = CAM_FOCUS_MODE_FIXED;
    private->af_mode = CAM_FOCUS_MODE_FIXED;
  } else if (af_out->focus_mode_info.mode) {
    private->bus.focus_mode = af_port_handle_set_focus_mode_to_hal_type(private,
      af_out->focus_mode_info.mode);

    if ((cam_focus_mode_type)private->bus.focus_mode != private->af_mode) {
      mode_change = TRUE;
    }
    // update state machine and prev_mode
    if (mode_change) {
      private->prev_af_mode = private->af_mode;
      private->af_mode = private->bus.focus_mode;
      af_port_update_af_state(port, AF_PORT_TRANS_CAUSE_MODE_CHANGE);
    }

    if (af_port_is_mode_continous(private->af_mode)) {
      af_update.cont_af_enabled = TRUE;
      send_update = TRUE;
    }
  }


  af_port_send_af_lens_state(port);

  if(af_out->type & AF_OUTPUT_EZ_METADATA) {
    if (af_out->eztune.eztune_data.ez_running == TRUE) {
      af_port_send_af_info_to_metadata(port, af_out);
    }
  }

  if (af_out->type & AF_OUTPUT_RESET_AEC) {
    af_send_bus_msg(port, MCT_BUS_MSG_SET_AEC_RESET,
      NULL, 0, private->sof_id);
  }
  if (af_out->type & AF_OUTPUT_DEBUG_DATA) {
    memcpy(&private->debug_info, af_out->debug_info, sizeof(af_debug_info_t));
  }
  if (af_out->type & AF_OUTPUT_SEND_EVENT) {
    af_port_send_exif_debug_data(port);
  }

  if (send_update) {
    if (af_out->type & AF_OUTPUT_MOVE_LENS) {
      ATRACE_BEGIN("Camera AF_port:Process lens movement event");
    }
    /* output the af_state anyway */
    af_update.af_state = private->af_trans.af_state;
    stats_update.flag = STATS_UPDATE_AF;
    stats_update.af_update = af_update;

    /* pack into an mct_event object*/
    event.direction = MCT_EVENT_UPSTREAM;
    event.identity = private->reserved_id;
    event.type = MCT_EVENT_MODULE_EVENT;
    event.u.module_event.current_frame_id = private->sof_id;
    event.u.module_event.type = MCT_EVENT_MODULE_STATS_AF_UPDATE;
    event.u.module_event.module_event_data = (void *)(&stats_update);
    event.u.module_event.current_frame_id = private->sof_id;

    AF_LOW("AF send MCT_EVENT_MODULE_STATS_AF_UPDATE cam_id: %u"
      " port =%p, event =%p", private->camera_id, port, &event);

    rc = MCT_PORT_EVENT_FUNC(port)(port, &event);
    if(rc == FALSE) {
      AF_LOW("Msg not sent!");
    } else {
      /* Current lens position in dac is piggy backed by the sensor */
      if(stats_update.af_update.reset_lens ||
        stats_update.af_update.move_lens) {
        depth_service_input_t *input = &(private->ds_input);
        input->info.af.af_focus_pos_dac = stats_update.af_update.dac_value;
      }
    }
    if (af_out->type & AF_OUTPUT_MOVE_LENS) {
      ATRACE_END();
    }
  }
  AF_LOW("X");
}


/** af_port_send_move_lens_cmd:
 *    @output: output AF data
 *    @p:      AF port
 *
 * Callback function that will ask the actuator to move the lens.
 *
 * Return the result of the command: TRUE if the lens is moved, FALSE if not
 **/
static boolean af_port_send_move_lens_cmd(void *output, void *p)
{
  mct_port_t        *port = (mct_port_t *)p;
  af_port_private_t *private = NULL;
  stats_update_t    stats_update;
  af_update_t       af_update;
  mct_event_t       event;
  af_output_data_t  *af_out = (af_output_data_t *)output;
  boolean           rc;

  private = (af_port_private_t *)(port->port_private);
  if (!private) {
    AF_ERR("Pointer to port's private data is NULL!");
    return FALSE;
  }

  memset(&af_update, 0, sizeof(af_update_t));
  af_update.stats_frm_id = af_out->frame_id;

  if (af_out->type & AF_OUTPUT_MOVE_LENS) {
    if (af_out->move_lens.reset_lens == TRUE) {
      /* Ask sensor module to reset the lens */
      AF_LOW("Asking actuator to reset the lens!");
      af_update.reset_lens = af_out->move_lens.reset_lens;
      private->af_focus_pos =
        private->af_algo_tuning.position_far_end;
    }
    /* If we have to move the lens, ask sensor module */
    else if (af_out->move_lens.use_dac_value == FALSE) {
      af_update.use_dac_value = FALSE;
      af_update.move_lens = af_out->move_lens.move_lens;
      af_update.direction = af_out->move_lens.direction;
      af_update.num_of_steps = af_out->move_lens.num_of_steps;


      // update the focus pos with the moved steps
      private->af_focus_pos = af_out->move_lens.cur_pos;
      if (af_update.direction == AF_MOVE_FAR) {
        private->af_focus_pos += af_update.num_of_steps;
      } else {
        private->af_focus_pos -= af_update.num_of_steps;
      }
      AF_LOW("Asking actuator to move the lens! flag: %d,"
          " steps: %d, dir: %d cur_pos %d, new_pos=%d",
        af_update.move_lens,
        af_update.num_of_steps,
        af_update.direction, af_out->move_lens.cur_pos,
        private->af_focus_pos);
    } else {
      unsigned int i;
      AF_LOW("Use DAC. Num of steps: %d",
        af_out->move_lens.num_of_interval);
      af_update.use_dac_value = TRUE;
      af_update.num_of_interval = af_out->move_lens.num_of_interval;
      for (i = 0; i < af_update.num_of_interval; i++) {
        af_update.pos[i] = af_out->move_lens.pos[i];
        af_update.delay[i] = af_out->move_lens.delay[i];
        AF_LOW("pos[%d] = %d delay[%d] = %d",
          i, af_update.pos[i], i, af_update.delay[i]);
      }
      // there is strong logic needed here.
      private->af_focus_pos = af_update.pos[0];
    }
  }
  private->diopter =
    af_port_convert_index_to_diopter(private, private->af_focus_pos);
  private->scale_ratio =
    af_port_convert_index_to_scale(private, private->af_focus_pos);

  if (((af_out->type & AF_OUTPUT_MOVE_LENS) &&
    af_out->move_lens.use_dac_value == FALSE) ||
    af_out->move_lens.reset_lens) {
    private->focus_distance_info.focus_distance\
      [CAM_FOCUS_DISTANCE_OPTIMAL_INDEX] =
      af_port_get_optimal_focus_distance(private, private->af_focus_pos);

    if ((int32_t)private->focus_distance_info.focus_distance\
      [CAM_FOCUS_DISTANCE_OPTIMAL_INDEX]== AF_PORT_MAX_FOCUS_DISTANCE) {
      // infinity
      private->focus_distance_info.focus_distance\
        [CAM_FOCUS_DISTANCE_OPTIMAL_INDEX] = 0;
      private->focus_distance_info.focus_distance\
        [CAM_FOCUS_DISTANCE_NEAR_INDEX] = 0;
      private->focus_distance_info.focus_distance\
        [CAM_FOCUS_DISTANCE_FAR_INDEX] = 0;
    } else {
      af_port_get_near_far_field_focus_distance(
        private, &private->focus_distance_info);
    }
  }

  stats_update.flag = STATS_UPDATE_AF;
  stats_update.af_update = af_update;

  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_AF_UPDATE;
  event.u.module_event.module_event_data = (void *)(&stats_update);
  event.u.module_event.current_frame_id = private->sof_id;

  AF_LOW("send AF_UPDATE to af port, port = %p, event = %p",
     port, &event);
  rc = MCT_PORT_EVENT_FUNC(port)(port, &event);
  if(rc) {
    /* Current lens position in dac is piggy backed by the sensor */
    if(stats_update.af_update.reset_lens || stats_update.af_update.move_lens) {
      depth_service_input_t *input = &(private->ds_input);
      input->info.af.af_focus_pos_dac = stats_update.af_update.dac_value;
    }
  }
  AF_LOW("X");
  return rc;
}

/** af_port_set_af_data:
 *    @af_port: private port data
 *
 * Set/reset some AF internal parameters to default values
 * and reset the lens to known default position.
 *
 * Return void
 **/
static void af_port_set_af_data(af_port_private_t *af_port)
{

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_INIT, af_port);

  if (af_msg != NULL) {
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
}

/** af_port_handle_capture_intent:
 **/
static void af_port_handle_capture_intent(af_port_private_t *af_port,
  cam_intent_t capture_intent) {
  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_RUN_MODE, af_port);

  if (af_msg != NULL) {
    /* Update which mode is this - Preview/Video/Snapshot */
    switch (capture_intent) {
    case CAM_INTENT_VIDEO_RECORD:
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_VIDEO;
      break;
    case CAM_INTENT_PREVIEW:
    case CAM_INTENT_ZERO_SHUTTER_LAG:
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_CAMERA;
      break;
    case CAM_INTENT_VIDEO_SNAPSHOT:
    case CAM_INTENT_STILL_CAPTURE:
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_SNAPSHOT;
      break;
    case CAM_INTENT_CUSTOM:
    default:
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_CAMERA;
      break;
    }
    af_port->run_type = af_msg->u.af_set_parm.u.af_run_mode;
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
} /* af_port_handle_capture_intent */

/** af_port_handle_set_crop_region_event:
 **/
static void af_port_handle_set_crop_region_event(af_port_private_t *af_port,
  cam_crop_region_t *crop_region) {
  af_crop_info_t crop_info;

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_CROP_REGION, af_port);

  Q3A_MEMSET(&crop_info, 0, sizeof(af_crop_info_t));
  if (af_msg != NULL) {
    crop_info.x = crop_region->left;
    crop_info.y = crop_region->top;
    crop_info.dx = crop_region->width;
    crop_info.dy = crop_region->height;

    af_msg->u.af_set_parm.u.crop_info = crop_info;
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
} /* af_port_handle_set_crop_region_event */

/** af_port_handle_stream_mode_update:
 *    @af_port: private port data
 *
 * Update the camera operation mode
 *
 * Return void
 **/
static void af_port_handle_stream_mode_update(af_port_private_t *af_port) {

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_RUN_MODE, af_port);

  if (af_msg != NULL ) {
    /* Update which mode is this - Preview/Video/Snapshot */
    switch (af_port->stream_info.stream_type) {
    case CAM_STREAM_TYPE_VIDEO: {
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_VIDEO;
    }
      break;
    case CAM_STREAM_TYPE_CALLBACK:
    case CAM_STREAM_TYPE_PREVIEW: {
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_CAMERA;
    }
      break;

    case CAM_STREAM_TYPE_RAW:
    case CAM_STREAM_TYPE_SNAPSHOT: {
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_SNAPSHOT;
    }
      break;

    default: {
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_CAMERA;
    }
      break;
    }
    af_port->run_type = af_msg->u.af_set_parm.u.af_run_mode;
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
} /* af_port_handle_stream_mode_update */

static void af_port_populate_local_tuning_params(af_core_tuning_params_t
  *core_tuning)
{
  if (!core_tuning)
    return;

  core_tuning->pdaf_tof_stable_cnt_th = 3;
  core_tuning->pdaf_tof_stable_cnt_th_low_light = 2;
  core_tuning->confi_level_change_th = 3;
  core_tuning->af_caf_trigger_after_taf = AF_CAF_TRIGGER_AFTER_TAF;
}

/** af_port_set_mw_enable:
  *    @port:       af's sink port to be initialized
  *
  *  Enable/disable mw based on setprop
  *
  **/
void af_port_set_mw_enable(af_port_private_t *af_port, boolean enable)
{
  boolean haf_enable = FALSE;
  int i;
  af_haf_enable_type *set_enable;
  int haf_disable_mask = 0;

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_MW_ENABLE, af_port);

  /*Sanity Check*/
  if(!af_msg) {
    AF_ERR("Allocation failure");
    return;
  }

  af_msg->u.af_set_parm.u.mw_enable = enable;
  q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
}

/** af_port_set_haf_enable:
  *    @port:       af's sink port to be initialized
  *
  *  Based on tuning data and internal capability, enable/disable
  *  HAF.
  *
  **/
void af_port_set_haf_enable(af_port_private_t *af_port)
{
  boolean haf_enable = FALSE;
  int i;
  af_haf_enable_type *set_enable;
  af_tuning_haf_adapter_t *p_haf_tuning = &af_port->af_haf_tuning;
  int haf_disable_mask = 0;

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_HAF_ENABLE, af_port);

  /*Sanity Check*/
  if(!af_msg) {
    AF_ERR("Allocation failure");
    return;
  }

  set_enable = &(af_msg->u.af_set_parm.u.haf);

  /* Check HAF disable mask */
  STATS_DISABLE_HAF_ALGO_MASK(haf_disable_mask);
  AF_LOW("HAF Disable Setprop mask: %x", haf_disable_mask);

  /* Copy HAF Algo Tuning parameter*/
  /*If no other algos are enabled, disable HAF */
  for(i = 0; i < AF_HAF_ALGO_MAX; i++){

    set_enable->algo_enable[i] = p_haf_tuning->algo_enable[i];

    switch (i){
      case AF_HAF_ALGO_PDAF:
        if(!(af_port->ds_input.cal_data.is_pdaf_supported &&
          af_port->is_pdaf_lib_available)) {
          AF_LOW("PDAF is being disabled as supported=%d and LIB Presence=%d",
            af_port->ds_input.cal_data.is_pdaf_supported,
            af_port->is_pdaf_lib_available);
          set_enable->algo_enable[AF_HAF_ALGO_PDAF] = 0;
        }
        /* If PDAF is being disabled by setprop */
        set_enable->algo_enable[i] &=
          !(haf_disable_mask & DISABLE_HAF_STATS_PDAF_MASK);
        break;
      case AF_HAF_ALGO_DCIAF:
        if(af_port->dual_cam_info.intra_peer_id == 0) {
          AF_LOW("DCiAF is being disabled");
          set_enable->algo_enable[AF_HAF_ALGO_DCIAF] = 0;
        }
        /* If DCIAF is being disabled by setprop */
        set_enable->algo_enable[i] &=
          !(haf_disable_mask & DISABLE_HAF_STATS_DCIAF_MASK);
        break;
      case AF_HAF_ALGO_TOF:
        /* If TOF is being disabled by setprop */
        set_enable->algo_enable[i] &=
          !(haf_disable_mask & DISABLE_HAF_STATS_TOF_MASK);
        break;
      default:
        break;
    }

    haf_enable |= set_enable->algo_enable[i];

  }

  /*If no other algos are enabled, disable HAF */
  if (!haf_enable) {
    set_enable->enable = 0;
  } else {
    set_enable->enable = p_haf_tuning->enable;
  }

  q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

}

/** af_port_set_fovc_enable:
 *    @port: af's sink port to be initialized
 *    @enable: if TRUE enable fovc
 *  Based on tuning data and internal capability, enable/disable
 *  HAF.
 *
 **/
void af_port_set_fovc_enable(af_port_private_t *af_port, int mode)
{
    /* Allocate memory to create AF message. we'll post it to AF thread.*/
    q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
                malloc(sizeof(q3a_thread_af_msg_t));

    /*Sanity Check*/
    if(!af_msg) {
        AF_ERR("Allocation failure");
        return;
    }
    AF_LOW("Enable FOVC - mode: %d", mode);
    af_port->fovc_data.fovc_mode = mode;
    STATS_MEMSET(af_msg, 0, sizeof(q3a_thread_af_msg_t));
    af_msg->type = MSG_AF_SET;
    af_msg->u.af_set_parm.type = AF_SET_PARAM_FOV_COMP_ENABLE;
    af_msg->u.af_set_parm.u.fov_comp_enable =
      (af_port->fovc_data.fovc_mode > 0) ? TRUE : FALSE;
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

}


/** af_port_set_af_hj_enable:
 *    @port: af's sink port to be initialized
 *    @enable: if TRUE enable af hj
 *  Based on tuning data and internal capability, enable/disable
 *  HAF.
 *
 **/
void af_port_set_af_hj_enable(mct_port_t *port, boolean isEnabled)
{
    af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
    /* Allocate memory to create AF message. we'll post it to AF thread.*/
    q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
                malloc(sizeof(q3a_thread_af_msg_t));
    mct_event_t                        af_gyro_cfg_req_event;
    mct_event_af_gyro_config_request_t af_gyro_config_req;

    /*Sanity Check*/
    if(!af_msg) {
        AF_ERR("Allocation failure");
        return;
    }

    af_port->is_af_hj_enabled = isEnabled
        | af_port->af_algo_tuning.af_haf.haf_extended.hjr_info.HJ_Algorithm_enable
        | af_port->af_algo_tuning.af_haf.haf_extended.caf_hjr_info.caf_HJ_Algorithm_enable;

    AF_LOW("Enable AF HJ %d: property %d, caf %d, single %d",
        af_port->is_af_hj_enabled,
        isEnabled,
        af_port->af_algo_tuning.af_haf.haf_extended.caf_hjr_info.caf_HJ_Algorithm_enable,
        af_port->af_algo_tuning.af_haf.haf_extended.hjr_info.HJ_Algorithm_enable);

    STATS_MEMSET(af_msg, 0, sizeof(q3a_thread_af_msg_t));
    af_msg->type = MSG_AF_SET;
    af_msg->u.af_set_parm.type = AF_SET_PARAM_AF_HJ_ENABLE;
    af_msg->u.af_set_parm.u.af_hj_enable = isEnabled; //only pass property flag down.
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

    // Send an update to gyro port.
    af_gyro_config_req.is_af_hj_enabled                     = af_port->is_af_hj_enabled;
    af_gyro_config_req.af_hj_gyro_sampling_rate            = 200; //TODO: Read from chromatix. af_port->af_algo_tuning.af_haf.haf_extended.hjr_info.sampling_rate;
    af_gyro_config_req.af_hj_gyro_report_rate              = 60 * (1 << 16); //TODO: Read from chromatix. af_port->af_algo_tuning.af_haf.haf_extended.hjr_info.reporting_rate;
    af_gyro_cfg_req_event.type                             = MCT_EVENT_MODULE_EVENT;
    af_gyro_cfg_req_event.direction                        = MCT_EVENT_UPSTREAM;
    af_gyro_cfg_req_event.identity                         = af_port->reserved_id;
    af_gyro_cfg_req_event.u.module_event.type              = MCT_EVENT_MODULE_AF_GYRO_CONFIG_REQUEST;
    af_gyro_cfg_req_event.u.module_event.module_event_data = &af_gyro_config_req;
    MCT_PORT_EVENT_FUNC(port)(port, &af_gyro_cfg_req_event);
}

/** af_port_handle_chromatix_update_evt:
 *    @af_port: private port data
 *    @mod_evt: module event containing tuning header pointer
 *
 * Update the chromatix pointer.
 *
 * Return nothing
 **/
static void af_port_handle_chromatix_update_evt(mct_port_t * port,
  mct_event_module_t *mod_evt)
{
  af_algo_tune_parms_t *tuning_info;
  int resv_enabled = 0;
  int enabled = 0;
  af_port_private_t *af_port =
      (af_port_private_t *)(port->port_private);

  chromatix_3a_parms_wrapper *chromatix_wrapper = mod_evt->module_event_data;

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_INIT_CHROMATIX_PTR, af_port);

  if (af_msg != NULL) {
    af_msg->u.af_set_parm.u.af_init_param.tuning_info = chromatix_wrapper;

    pthread_rwlock_rdlock(&chromatix_wrapper->lock);
    /* Enable/Disable HAF based on tuning data and internal capabilities */
    af_port->af_haf_tuning = chromatix_wrapper->AF.af_algo.af_haf;
     pthread_rwlock_unlock(&chromatix_wrapper->lock);
    af_port_set_haf_enable(af_port);

    //Populating local tuning params that cannot be put in chromatics
    af_port_populate_local_tuning_params(
      &af_msg->u.af_set_parm.u.af_init_param.core_tuning_params);

    af_msg->u.af_set_parm.u.af_init_param.op_mode = af_port->run_type;

    af_msg->u.af_set_parm.u.af_init_param.preview_size.width =
      af_port->stream_info.dim.width;
    af_msg->u.af_set_parm.u.af_init_param.preview_size.height =
      af_port->stream_info.dim.height;

    /* Reserved section parameters to be updated in next chromatix version. */
    pthread_rwlock_rdlock(&chromatix_wrapper->lock);
       pthread_rwlock_unlock(&chromatix_wrapper->lock);

    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

    /* Also update tuning header for fd_prio AF */
    pthread_rwlock_rdlock(&chromatix_wrapper->lock);
    af_port->fd_prio_data.tuning_info = chromatix_wrapper->AF;
    pthread_rwlock_unlock(&chromatix_wrapper->lock);

    /* And save preview_size */
    af_port->preview_size.width = af_port->stream_info.dim.width;
    af_port->preview_size.height = af_port->stream_info.dim.height;

    AF_LOW("Preview Size: %dx%d", af_port->preview_size.width,
      af_port->preview_size.height);

    /* Update Depth Service cal data*/
    pthread_rwlock_rdlock(&chromatix_wrapper->lock);
    af_tuning_lens_sag_comp_adapter_t *tuning_ptr = &(chromatix_wrapper->AF.af_algo.lens_sag_comp);
    af_port->ds_input.cal_data.af_cal_inf_fd = tuning_ptr->f_dac[AF_TUNING_DAC_FACE_DOWN];
    af_port->ds_input.cal_data.af_cal_inf_ff = tuning_ptr->f_dac[AF_TUNING_DAC_FACE_FORWARD];
    af_port->ds_input.cal_data.af_cal_inf_fu = tuning_ptr->f_dac[AF_TUNING_DAC_FACE_UP];
    pthread_rwlock_unlock(&chromatix_wrapper->lock);

    /* Also update any pre-init module updates we've received so far */
    if (af_port->pre_init_updates.is_stream_info_updated == TRUE) {
      AF_LOW("Update saved ISP info!");
      af_port_handle_isp_output_dim_event(af_port,
        &af_port->pre_init_updates.stream_info);
      af_port->pre_init_updates.is_stream_info_updated = FALSE;
    }

    /* Now we can start sending events to AF algorithm to process */
    if (!af_port->af_not_supported) {
      af_port->af_initialized = TRUE;
    }

    /* Read setprop and enable/disable FOVC feature. The reason to add this here is
       so as to read steprop once during camera init. */
    af_port->af_algo_tuning = chromatix_wrapper->AF.af_algo;
    af_port->p_af_tuning = &chromatix_wrapper->AF;

    // set FOVC enable/disble flag
    STATS_ENABLE_FOVC(enabled);
    switch (enabled) {
      case 0: {
        enabled = chromatix_wrapper->AF.af_algo.fovc.enable;
      } break;
      case 1: {
        enabled = 1;
      } break;
      case 2: {
        enabled = 0;
      } break;
      default: {
        enabled = chromatix_wrapper->AF.af_algo.fovc.enable;
      } break;
    }
    af_port_set_fovc_enable(af_port, (int)enabled);

    /* Enable/disable mw based on setprop */
    STATS_ENABLE_MULTIWINDOW(enabled);
    af_port_set_mw_enable(af_port, enabled);

 /* Read setprop and enable/disable HJ AF feature. The reason to add this here is
    so as to read steprop once during chromatix load/reload. */
    STATS_ENABLE_AF_HJ(enabled);
    af_port_set_af_hj_enable(port, enabled);
  }
} /* af_port_handle_chromatix_update_evt */

/** af_port_handle_current_pos_update_evt:
 *    @af_port: private port data
 *    int : Current Postion
 * Update the chromatix pointer if the chromatix is reloaded.
 *
 * Return void
 **/
static void af_port_handle_current_pos_update_evt(af_port_private_t *af_port, int current_position)
{
  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_LENS_POS_FOCUS, af_port);

  if (af_msg != NULL) {
    af_msg->u.af_set_parm.u.cur_pos_comp = current_position;
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
}
/** af_port_handle_awb_update:
 *    @af_port: private port data
 *    @mod_evt: module event structure
 *
 * AWB has some updates. Save ones AF needs.
 *
 * Return void
 **/
static void af_port_handle_awb_update(af_port_private_t * af_port,
  mct_event_module_t * mod_evt) {
  boolean rc = FALSE;
  stats_update_t *stats_update = (stats_update_t *)mod_evt->module_event_data;

  /* Only sending AWB update for custom params */
  if (stats_update->awb_update.awb_custom_param_update.data &&
    stats_update->awb_update.awb_custom_param_update.size) {
    /* Allocate memory to create AF message. we'll post it to AF thread. */
    q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
      AF_SET_PARAM_UPDATE_AWB_INFO, af_port);
    if (af_msg != NULL) {
      /* Handle custom parameters update (3a ext) */
      af_msg->u.af_set_parm.u.awb_info.custom_param_af.data =
        malloc(stats_update->awb_update.awb_custom_param_update.size);
      if (af_msg->u.af_set_parm.u.awb_info.custom_param_af.data) {
        af_msg->u.af_set_parm.u.awb_info.custom_param_af.size =
          stats_update->awb_update.awb_custom_param_update.size;
        memcpy(af_msg->u.af_set_parm.u.awb_info.custom_param_af.data,
          stats_update->awb_update.awb_custom_param_update.data,
          af_msg->u.af_set_parm.u.awb_info.custom_param_af.size);
      } else {
        AF_ERR("Fail to allocate memory for custom parameters");
        free(af_msg);
        af_msg = NULL;
        rc = FALSE;
        return;
      }

      rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
      if (!rc) {
        AF_ERR("Fail to queue AWB update info");
      }
    }
  }
  /* Save for Depth service */
  af_port->ds_input.info.awb.color_temp = stats_update->awb_update.color_temp;
  af_port->ds_input.info.awb.gain = stats_update->awb_update.gain;
  af_port->ds_input.info.awb.decision = stats_update->awb_update.decision;
}

/** af_port_handle_aec_manual_update:
*    @af_port: private port data
*    @mod_evt: module event structure
*
* AEC has some updates. Save ones AF needs.
*
* Return void
**/
static void af_port_handle_aec_manual_update(af_port_private_t * af_port,
                                     mct_event_module_t * mod_evt) {

    af_input_from_aec_t *aec_info = NULL;
    aec_manual_update_t *aec_manual_update = (aec_manual_update_t *) mod_evt->module_event_data;
    /* Allocate memory to create AF message. we'll post it to AF thread.*/
    q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
                                                     AF_SET_PARAM_AEC_MANUAL_UPDATE, af_port);
    if (af_msg != NULL) {

        /* Copy aec data now */
        aec_info = &(af_msg->u.af_set_parm.u.aec_info);
        aec_info->aec_settled = aec_manual_update->aec_settled;
        q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
    }
}


/** af_port_handle_aec_update:
 *    @af_port: private port data
 *    @mod_evt: module event structure
 *
 * AEC has some updates. Save ones AF needs.
 *
 * Return void
 **/
static void af_port_handle_aec_update(af_port_private_t * af_port,
  mct_event_module_t * mod_evt) {
  af_input_from_aec_t *aec_info = NULL;
  stats_update_t *stats_update = (stats_update_t *)mod_evt->module_event_data;

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_UPDATE_AEC_INFO, af_port);
  if (af_msg != NULL) {
    /* Handle custom parameters update (3a ext) */
    if (stats_update->aec_update.aec_custom_param_update.data &&
      stats_update->aec_update.aec_custom_param_update.size) {
      af_msg->u.af_set_parm.u.aec_info.custom_param_af.data =
        malloc(stats_update->aec_update.aec_custom_param_update.size);
      if (af_msg->u.af_set_parm.u.aec_info.custom_param_af.data) {
        af_msg->u.af_set_parm.u.aec_info.custom_param_af.size =
          stats_update->aec_update.aec_custom_param_update.size;
        memcpy(af_msg->u.af_set_parm.u.aec_info.custom_param_af.data,
          stats_update->aec_update.aec_custom_param_update.data,
          af_msg->u.af_set_parm.u.aec_info.custom_param_af.size);
      } else {
        AF_ERR("Error: Fail to allocate memory for custom parameters");
        free(af_msg);
        af_msg = NULL;
        return;
      }
    }

    /* Copy aec data now */
    aec_info = &(af_msg->u.af_set_parm.u.aec_info);

    aec_info->aec_settled = stats_update->aec_update.settled;
    aec_info->cur_luma = stats_update->aec_update.cur_luma;
    aec_info->target_luma = stats_update->aec_update.target_luma;
    aec_info->luma_settled_cnt = stats_update->aec_update.luma_settled_cnt;
    aec_info->cur_real_gain = stats_update->aec_update.real_gain;
    aec_info->exp_index = stats_update->aec_update.exp_index;
    aec_info->lux_idx = stats_update->aec_update.lux_idx;
    aec_info->exp_tbl_val = stats_update->aec_update.exp_tbl_val;
    aec_info->comp_luma = stats_update->aec_update.comp_luma;
    aec_info->pixels_per_region = stats_update->aec_update.pixelsPerRegion;
    aec_info->num_regions = stats_update->aec_update.numRegions;
    aec_info->exp_time = stats_update->aec_update.exp_time;
    aec_info->preview_fps = stats_update->aec_update.preview_fps;
    aec_info->preview_linesPerFrame =
      stats_update->aec_update.preview_linesPerFrame;
    aec_info->linecnt = stats_update->aec_update.linecount;
    aec_info->target_luma = stats_update->aec_update.target_luma;
    if (stats_update->aec_update.SY_data.is_valid &&
        stats_update->aec_update.SY_data.SY) {
      memcpy(aec_info->SY_data.SY, stats_update->aec_update.SY_data.SY,
        sizeof(uint32_t) * MAX_YUV_STATS_NUM);
      aec_info->SY_data.is_valid = stats_update->aec_update.SY_data.is_valid;
    }

    aec_info->Av_af = stats_update->aec_update.Av;
    aec_info->Tv_af = stats_update->aec_update.Tv;
    aec_info->Bv_af = stats_update->aec_update.Bv;
    aec_info->Sv_af = stats_update->aec_update.Sv;
    aec_info->Ev_af = stats_update->aec_update.Ev;
    aec_info->total_drc_gain = stats_update->aec_update.total_drc_gain;
    AF_LOW("preview_fps %f, exp time %f, Bv_af: %f, SY_data? %d",
      (float)aec_info->preview_fps / 256.0, aec_info->exp_time,
      aec_info->Bv_af, aec_info->SY_data.is_valid);
    af_port->latest_aec_info = *aec_info;
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
    /* Update Preview fps for FD-AF too */
    af_port->fd_prio_data.preview_fps = (int) stats_update->aec_update.preview_fps / 256;

    /* Save for Depth service */
    af_port->ds_input.info.aec.lux_idx = stats_update->aec_update.lux_idx;
    af_port->ds_input.info.aec.real_gain = stats_update->aec_update.real_gain;
    af_port->ds_input.info.aec.line_count = stats_update->aec_update.linecount;
    /* Send to FD prio */
    if (stats_update->aec_update.SY_data.is_valid &&
       stats_update->aec_update.SY_data.SY) {
       fdprio_set_parm fdprio_parm;
       fdprio_parm.type = FDPRIO_SET_AEC_DATA;
       fdprio_parm.fdprio_data = &af_port->fd_prio_data;
       fdprio_parm.u.aec_info.SY_data = &(stats_update->aec_update.SY_data);
       fdprio_parm.u.aec_info.preview_fps =
         stats_update->aec_update.preview_fps;
       fdprio_set_param(&fdprio_parm);
    }
  }
} /* af_port_handle_aec_update */

/** af_port_handle_gyro_update:
 *    @af_port: private port data
 *    @mod_evt: module event structure
 *
 * Update gyro data.
 *
 * Return void.
 **/
static void af_port_handle_gyro_update(af_port_private_t * af_port,
  mct_event_module_t * mod_evt)
{
  af_input_from_gyro_t   *gyro_info = NULL;
  mct_event_gyro_stats_t *gyro_update =
      (mct_event_gyro_stats_t *)mod_evt->module_event_data;
  uint32_t i = 0;

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_UPDATE_GYRO_INFO, af_port);
  if (af_msg != NULL) {
    /* Copy gyro data now */
    gyro_info = &(af_msg->u.af_set_parm.u.gyro_info);

    AF_LOW("frame id %d samples %d", gyro_update->is_gyro_data.frame_id, gyro_update->is_gyro_data.sample_len);
    gyro_info->sample_len = gyro_update->is_gyro_data.sample_len;
    for(i=0; i<gyro_update->is_gyro_data.sample_len; i++) {
      gyro_info->sample[i] = gyro_update->is_gyro_data.sample[i];
      AF_LOW("gyro sample %u, (%d, %d, %d) timestamp %llu",
          i,
          gyro_update->is_gyro_data.sample[i].value[0],
          gyro_update->is_gyro_data.sample[i].value[1],
          gyro_update->is_gyro_data.sample[i].value[2],
          gyro_update->is_gyro_data.sample[i].timestamp);
    }

    gyro_info->q16_ready = TRUE;
    gyro_info->float_ready = TRUE;

    for (i = 0; i < 3; i++)
    {
      gyro_info->q16[i] = (long) gyro_update->q16_angle[i];
      gyro_info->flt[i] = (float)gyro_update->q16_angle[i] / (1 << 16);
      AF_LOW("i: %d q16: %ld flt: %f", i,
        gyro_info->q16[i], gyro_info->flt[i]);
    }
   /* PDAF update for gyro */
   af_port->ds_input.info.gyro.float_ready = TRUE;
   af_port->ds_input.info.gyro.q16_ready   = TRUE;

   for (i = 0; i < 3; i++) {
      af_port->ds_input.info.gyro.q16[i] =  gyro_info->q16[i];
      af_port->ds_input.info.gyro.flt[i] = gyro_info->flt[i];
    }

    if (gyro_info->float_ready) {
        fdprio_set_parm fdprio_parm;
        fdprio_parm.type = FDPRIO_SET_GYRO_DATA;
        fdprio_parm.fdprio_data = &af_port->fd_prio_data;
        float gyro_sqr =  gyro_info->flt[0] * gyro_info->flt[0] +
          gyro_info->flt[1] * gyro_info->flt[1] + gyro_info->flt[2] * gyro_info->flt[2];
        fdprio_parm.u.gyro_sqr = (float) sqrt(gyro_sqr);
        fdprio_set_param(&fdprio_parm);
    }
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
} /* af_port_handle_gyro_update */

/** af_port_get_lens_sag_compensation:
 *    @af_port: private port data
 *    @mod_evt: module event structure
 *
 * Update gravity data.
 *
 * Return void.
 **/
int af_port_get_lens_sag_compensation(af_port_private_t * af_port, float gravity_vector_data){
  int comp = 0;
  af_tuning_lens_sag_comp_adapter_t *tuning_ptr =
    &af_port->fd_prio_data.tuning_info.af_algo.lens_sag_comp;
  int face_up_sag_in_dac = 0, face_dwn_sag_in_dac = 0;
  float gravity_vector_z = 0.0, um_per_dac = 1.0;

  AF_LOW("Lens Sag Comp Enabled: %d",
    tuning_ptr->enable);
  if (tuning_ptr->enable && af_port->gravity_data_ready) {
    AF_LOW("f_dac - FU: %u FF: %u FD: %u fdist: %f",
    tuning_ptr->f_dac[AF_TUNING_DAC_FACE_UP],
    tuning_ptr->f_dac[AF_TUNING_DAC_FACE_FORWARD],
    tuning_ptr->f_dac[AF_TUNING_DAC_FACE_DOWN],
    tuning_ptr->f_dist);

    face_up_sag_in_dac = tuning_ptr->f_dac[AF_TUNING_DAC_FACE_FORWARD] -
    tuning_ptr->f_dac[AF_TUNING_DAC_FACE_UP];
    face_dwn_sag_in_dac = tuning_ptr->f_dac[AF_TUNING_DAC_FACE_DOWN] -
    tuning_ptr->f_dac[AF_TUNING_DAC_FACE_FORWARD];

    /* Get gravity vector and normalize */
    AF_LOW("Input gravity vector: %f",
    gravity_vector_data);
    gravity_vector_z = gravity_vector_data / 9.81;

    /* Gravity sensor axes are opposite of image sensor axes. So for sensor
     face-up the gravity vector received is positive where as for
     sensor face-down the gravity vector is negative. */
    if (gravity_vector_z > 0) {
      comp = (int) (face_up_sag_in_dac * gravity_vector_z * um_per_dac);
    } else {
      comp = (int) (face_dwn_sag_in_dac * gravity_vector_z * um_per_dac);
    }
  }

  /* If sensor is facing down, we'll need to apply less current, i.e.
     smaller DAC value. Similarly, if sensor is facing up we'll need to
     apply more current to reach any specific lens position. However,
     logical range and actual DAC range is exactly opposite. Logical
     index 0 means macro and higher the number farther the object.
     DAC code 0 means lens is at rest posiiton and DAC code increases
     as we try to focus on macro object.
     Hence the compensation factor should be added if we need to
     compensate lens towards infy position and subtracted if compensation
     is towards macro. */
  comp *= (-1);

  return comp;

}

/** af_port_handle_gravity_update:
 *    @af_port: private port data
 *    @mod_evt: module event structure
 *
 * Update gravity data.
 *
 * Return void.
 **/
static void af_port_handle_gravity_update(mct_port_t *port,
  mct_event_module_t * mod_evt)
{
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  af_input_from_gravity_vector_t   *gravity_data = NULL;
  mct_event_gravity_vector_update_t *gravity_update =
    (mct_event_gravity_vector_update_t *)mod_evt->module_event_data;
  af_tuning_single_adapter_t* af_single = &af_port->fd_prio_data.tuning_info.af_algo.af_single;
  af_tuning_lens_sag_comp_adapter_t *tuning_ptr =
    &af_port->fd_prio_data.tuning_info.af_algo.lens_sag_comp;
  boolean gravity_send_data = FALSE;
  if(af_port == NULL){
    return;
  }
  /* PDAF update for gravity/accelormeter */
  uint8_t i;
  af_port->ds_input.info.gravity.accuracy = gravity_update->accuracy;
  for (i = 0; i < 3; i++){
    af_port->ds_input.info.gravity.gravity[i] = gravity_update->gravity[i];
    af_port->ds_input.info.gravity.lin_accel[i] = gravity_update->lin_accel[i];
  }
  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_UPDATE_GRAVITY_VECTOR, af_port);
  if (af_msg != NULL) {
    af_port->gravity_data_ready = TRUE;
    af_port->gravity_vector_data = gravity_update->gravity[2];
    float diff =fabsf(af_port->gravity_vector_data -
      af_port->gravity_vector_data_applied);
    AF_LOW("gravity_vector_data =%f, gravity_vector_data_applied =%f,diff=%f ",
    af_port->gravity_vector_data,
        af_port->gravity_vector_data_applied,diff);

   switch(af_port->af_mode) {
  case CAM_FOCUS_MODE_MANUAL:
  case CAM_FOCUS_MODE_OFF: {
   if(af_port->manual.manual_af_state == TRUE && tuning_ptr->enable ){
     if(diff > ( tuning_ptr->trigger_threshold * GRAVITY_VECTOR) ){
           af_port_handle_set_focus_manual_pos_evt(port, &af_port->manual);
       }
      }
   }
  break;
  case CAM_FOCUS_MODE_MACRO:
  case CAM_FOCUS_MODE_AUTO: {
    if( (af_port->af_trans.af_state == CAM_AF_STATE_FOCUSED_LOCKED) && tuning_ptr->enable ){
     if(diff > ( tuning_ptr->trigger_threshold * GRAVITY_VECTOR) ){
         af_port_manual_lens_sag(port);
       }
   }else {
   gravity_send_data = TRUE;
   }
    }
  break;

  case CAM_FOCUS_MODE_INFINITY: {
     if(tuning_ptr->enable){
     if(diff > ( tuning_ptr->trigger_threshold * GRAVITY_VECTOR) ){
          af_port_manual_lens_sag(port);
        }
     }
  }
  break;

  case CAM_FOCUS_MODE_CONTINOUS_VIDEO:
  case CAM_FOCUS_MODE_CONTINOUS_PICTURE: {
     gravity_send_data = TRUE;
  }
  break;
    default: {
    AF_LOW("Default. no action!");
    }
    break;
  }

  if(gravity_send_data == TRUE) {
    gravity_send_data = FALSE;
    /* Copy gyro data now */
    gravity_data = &(af_msg->u.af_set_parm.u.gravity_info);
    gravity_data->is_ready = TRUE;
    int i = 0;
    for (i = 0; i < 3; i++)
    {
      gravity_data->g_vector[i] = gravity_update->gravity[i];
      gravity_data->lin_accel[i] = gravity_update->lin_accel[i];
    }
    gravity_data->accuracy = gravity_update->accuracy;

    AF_LOW("gravity - (%f, %f, %f) lin_accel - (%f, %f, %f)"
      "accuracy: %d", gravity_data->g_vector[0],
      gravity_data->g_vector[1], gravity_data->g_vector[2],
      gravity_data->lin_accel[0], gravity_data->lin_accel[1],
      gravity_data->lin_accel[2], gravity_data->accuracy);

    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
   } else {
     free(af_msg);
     af_msg = NULL;
   }
  }
} /* af_port_handle_gravity_update */

/** af_port_handle_sensor_update:
 *    @af_port: private port data
 *    @mod_evt: module event structure
 *
 * Sensor has some updates. Save once AF needs.
 *
 * Return void.
 **/
static void af_port_handle_sensor_update(af_port_private_t * af_port,
  mct_event_module_t * mod_evt)
{
  af_input_from_sensor_t *sensor_info = NULL;
  sensor_out_info_t      *sensor_update =
    (sensor_out_info_t *)(mod_evt->module_event_data);
  boolean rc = TRUE;

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_UPDATE_SENSOR_INFO, af_port);
  if (af_msg != NULL) {
    /* Copy af data now */
    sensor_info = &(af_msg->u.af_set_parm.u.sensor_info);

    sensor_info->af_not_supported =
      !(sensor_update->af_lens_info.af_supported);
    //sensor_info->preview_fps = ;
    sensor_info->max_preview_fps = sensor_update->max_fps;
    sensor_info->actuator_info.focal_length =
      sensor_update->af_lens_info.focal_length;
    sensor_info->actuator_info.af_f_num = sensor_update->af_lens_info.f_number;
    sensor_info->actuator_info.af_f_pix = sensor_update->af_lens_info.pix_size;
    sensor_info->actuator_info.af_total_f_dist =
      sensor_update->af_lens_info.total_f_dist;
    sensor_info->actuator_info.hor_view_angle =
      sensor_update->af_lens_info.hor_view_angle;
    sensor_info->actuator_info.ver_view_angle =
      sensor_update->af_lens_info.ver_view_angle;
    sensor_info->actuator_info.min_focus_distance =
      sensor_update->af_lens_info.min_focus_distance;
    sensor_info->sensor_res_height =
      sensor_update->request_crop.last_line -
      sensor_update->request_crop.first_line + 1;
    sensor_info->sensor_res_width =
      sensor_update->request_crop.last_pixel -
      sensor_update->request_crop.first_pixel + 1;

    af_port->cam_info.actuator_info = sensor_info->actuator_info;
    sensor_info->actuator_sensitivity = sensor_update->actuator_sensitivity;
    AF_LOW("Sensor Res width: %d height: %d FocalLength: %f and FNum: %f",
      sensor_info->sensor_res_width,
      sensor_info->sensor_res_height,
      sensor_info->actuator_info.focal_length,
      sensor_info->actuator_info.af_f_num);
    /* update depth service cal data */
    af_port->ds_input.cal_data.eff_focal_length = sensor_info->actuator_info.focal_length;
    af_port->ds_input.cal_data.f_num = sensor_info->actuator_info.af_f_num;
    af_port->ds_input.cal_data.pixel_size  = sensor_info->actuator_info.af_f_pix;
    af_port->ds_input.cal_data.is_pdaf_supported = sensor_update->is_pdaf_supported;
    af_port->ds_input.cal_data.actuator_sensitivity = sensor_update->actuator_sensitivity;
    if(sensor_update->actuator_sensitivity <= 0) {
      AF_ERR("Depth_Err: actuator sensitivity: %f is invalid",
        sensor_update->actuator_sensitivity);
      rc = FALSE;
    }

    if(sensor_update->is_pdaf_supported && FALSE == rc) {
      AF_LOW("Depth_Err: pdaf cal data from sensor supported %d pdaf_ptr %p",
        sensor_update->is_pdaf_supported, sensor_update->pdaf_data);
       af_port->ds_input.cal_data.is_pdaf_supported = 0;
    }

    /* Update CAMIF size for FD-AF and store in AF port for general use cases*/
    af_port->fd_prio_data.camif_height = sensor_info->sensor_res_height;
    af_port->fd_prio_data.camif_width = sensor_info->sensor_res_width;
    af_port->camif_width = sensor_info->sensor_res_width;
    af_port->camif_height = sensor_info->sensor_res_height;

    /* Also if AF isn't supported */
    af_port->af_not_supported = sensor_info->af_not_supported;
    if (sensor_info->af_not_supported == TRUE) {
      af_port->af_initialized = FALSE;
    }
    sensor_info->sensor_type = af_port->cam_info.sensor_type;
    sensor_info->lens_type = af_port->cam_info.lens_type;

    sensor_info->frame_time = (uint32_t)
          ((sensor_update->ll_pck * sensor_update->dim_output.height) /
           (sensor_update->vt_pixel_clk / USEC_PER_SEC));

    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
} /* af_port_handle_sensor_update */

/** af_port_handle_fd_event:
 *    @af_port:   private port data
 *    @mod_event: module event
 *
 * Handle new face detection ROIs
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_fd_event(af_port_private_t * af_port,
  mct_event_module_t * mod_evt) {
  boolean         rc = TRUE;
  mct_face_info_t *face_info;
  int             i = 0;

  face_info = (mct_face_info_t *)mod_evt->module_event_data;
  if (!face_info) {
    AF_ERR("failed");
    return FALSE;
  }

  af_port->fd_prio_data.pface_info = face_info;
  af_port->fd_prio_data.current_frame_id = face_info->frame_id;
  fdprio_process(&af_port->fd_prio_data, FDPRIO_CMD_PROC_FD_ROI);

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg != NULL) {
    memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
    af_msg->type = MSG_AF_SET;
    /* Update message contents - first set param type*/
    af_msg->u.af_set_parm.type = AF_SET_PARAM_UPDATE_FACE_INFO;
    af_msg->u.af_set_parm.u.face_detected = (boolean)face_info->face_count;
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }

  return rc;
} /* af_port_handle_fd_event */

/** af_port_handle_send_fake_stats_on_ihist
 * Send Fake Stats on iHist event with Frame ID
 *
 * @af_port: private port number
 *
 * @mod_event: module event
 *
 **/
static boolean af_port_handle_send_fake_stats_on_ihist(af_port_private_t * af_port,
  mct_event_stats_isp_t * stats_evt) {
  boolean rc = FALSE;
  q3a_thread_af_msg_t *af_msg = NULL;
  stats_af_t *af_stats = NULL;

  do {
    af_msg = af_port_create_msg(MSG_BF_STATS, 0, af_port);
    if(!af_msg) {
      AF_ERR("Error allocating memory.");
      rc = FALSE;
      break;
    }

    af_stats = (stats_af_t *)malloc(sizeof(stats_af_t));
    if (!af_stats) {
      AF_ERR("Error allocating memory.");
      rc = FALSE;
      break;
    }
    memset(af_stats, 0, sizeof(stats_af_t));
    af_stats->frame_id = stats_evt->frame_id;
    af_stats->camera_id = af_port->camera_id;
    af_stats->stats_type_mask = STATS_BF;
    af_stats->isFake = TRUE;
    /* Need to call this function for every frame */
    af_port->fd_prio_data.current_frame_id = stats_evt->frame_id;

    if (af_port->af_initialized) {
      fdprio_process(&af_port->fd_prio_data, FDPRIO_CMD_PROC_COUNTERS);
    }
    af_msg->u.stats = af_stats;
    /* Enqueue the message to the AF thread */
    if(FALSE==q3a_af_thread_en_q_msg(af_port->thread_data, af_msg)) {
      /* If the AF thread is inactive, it will not enqueue our
       * message and instead will free it. Then we need to manually
       * free the payload */
      af_stats = NULL;
    }
    rc = TRUE;
  } while (0);
  if(rc == FALSE) {
    if (af_stats) {
      free(af_stats);
      af_stats = NULL;
    }
    if (af_msg) {
      free(af_msg);
      af_msg = NULL;
    }
  }
  return rc;
}

/** af_port_handle_sof_event
 * Send Sof id
 *
 * @af_port: private port number
 *
 * @mod_event: module event
 *
 **/
static boolean af_port_handle_sof_event(mct_port_t * port,
  mct_event_control_t * ctrl_evt) {
  boolean rc = TRUE;
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  mct_bus_msg_isp_sof_t *sof_event =
    (mct_bus_msg_isp_sof_t *)(ctrl_evt->control_event_data);
  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_SOF, af_port);

  if (af_msg != NULL) {
    /* Update message contents - first set param type */
    af_msg->u.af_set_parm.u.af_set_sof_id = sof_event->frame_id;
    rc &= q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }

  /* Enqueue Send event msg here to send exif af_debug_data
   * through cb function */
  af_msg = af_port_create_msg(MSG_AF_SEND_EVENT, 0, af_port);
  if (af_msg != NULL) {
    rc &= q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
  af_send_batch_bus_message(port, STATS_REPORT_IMMEDIATE, sof_event->frame_id);
  return rc;
} /* af_port_handle_sof_event */

/** af_port_handle_tof_stats_info:
 *    @af_port: private port data
 *
 * Send TOF stats capabilities to the library.
 *
 * Return void.
 **/
static void af_port_handle_tof_stats_info(af_port_private_t * af_port)
{

  boolean rc = TRUE;
  depth_service_input_t *input = &(af_port->ds_input);
  q3a_thread_af_msg_t *af_msg = NULL;


  af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_DEPTH_SERVICE, af_port);
  if (af_msg == NULL) {
    AF_ERR("ERROR af_msg is NULL!");
    return;
  }

  /* Convert Raw data to Depth Map output*/
  /* Pack data to depthmap input*/

  input->info.type = DEPTH_SERVICE_TOF;
  input->info.frame_id = af_port->sof_id;

  memcpy(&input->u.tof_info, &af_port->tof_data.tof_update, sizeof(tof_update_t));

  if (depth_service_process(input, &(af_msg->u.af_set_parm.u.depth_service)) ==
    TRUE) {
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  } else {
    free(af_msg);
    af_msg = NULL;
  }
} /* af_port_handle_tof_stats_info */


/** af_port_handle_stats_event
 *    @af_port:   private port data
 *    @mod_event: module event
 *
 * Once we receive AF stats, we need to ask af algorithm to process it.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_stats_event(mct_port_t *port,
  mct_event_module_t * mod_evt)
{
  af_port_private_t * af_port =
    (af_port_private_t *)(port->port_private);
  boolean               rc = TRUE;
  mct_event_stats_ext_t* stats_ext_event =
    (mct_event_stats_ext_t *)mod_evt->module_event_data;
  mct_event_stats_isp_t *stats_event = stats_ext_event->stats_data;

  if (stats_event) {
    /* First check if these are AF stats. No need to allocate and free
     * memory if we are going to ignore this event.
     */
    if ((stats_event->stats_mask & (1 << MSM_ISP_STATS_BF)) ||
      (stats_event->stats_mask & (1 << MSM_ISP_STATS_BF_SCALE))) {

      /* If FOVC feature, account for pipeline delay by delaying lens
         move */
      if (FOVC_MODE_ISP == af_port->fovc_data.fovc_mode &&
        (TRUE == af_port->fovc_data.delay_lens_move)) {
        if (af_port->fovc_data.frame_skip_cnt < AF_PORT_FOVC_FRAME_SKIP) {
          AF_LOW("FOVC - delay move lens!");
          af_port->fovc_data.frame_skip_cnt++;
          return TRUE;
        } else {
          af_output_data_t output;
          AF_LOW("FOVC - Move lens! Mag factor: %f",
            af_port->fovc_data.mag_factor);
          memset(&output, 0, sizeof(output));
          output.type |= AF_OUTPUT_MOVE_LENS;
          output.move_lens = af_port->fovc_data.move_lens;
          af_port_send_move_lens_cmd(&output, port);
          af_port->fovc_data.delay_lens_move = FALSE;
          af_port->fovc_data.frame_skip_cnt = 0;
        }
      }

      /* Allocate memory to create AF message. we'll post it to AF thread.*/
      q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_STATS,
        0, af_port);
      if (af_msg == NULL) {
        AF_ERR("Error allocating memory.");
        return FALSE;
      }

      stats_af_t *af_stats = (stats_af_t *)malloc(sizeof(stats_af_t));
      if (af_stats == NULL) {
        AF_ERR("Error allocating memory.");
        free(af_msg);
        af_msg = NULL;
        return FALSE;
      }

      Q3A_MEMSET(af_stats, 0, sizeof(stats_af_t));
      /* No need to memset to 0 the allocated buffer. It will be set by
       * copying the stats into it. The rest of the fields are also
       * set appropriately.
       */
      af_stats->frame_id = stats_event->frame_id;
      af_stats->camera_id = af_port->camera_id;
      af_stats->isFake = FALSE;
      /* Copy field by field - the two structures are NOT of the same type */
      af_stats->time_stamp.time_stamp_sec = stats_event->timestamp.tv_sec;
      af_stats->time_stamp.time_stamp_us  = stats_event->timestamp.tv_usec;
      af_stats->sof_timestamp.time_stamp_sec = stats_event->sof_timestamp.tv_sec;
      af_stats->sof_timestamp.time_stamp_us  = stats_event->sof_timestamp.tv_usec;

      AF_LOW("Stats mask: %x", stats_event->stats_mask);

      if (stats_event->stats_mask & (1 << MSM_ISP_STATS_BF)) {
        /* We'll limit tof updates to every frame */
        if (af_port->tof_data.is_updated) {
          af_port_handle_tof_stats_info(af_port);
          af_port->tof_data.is_updated = FALSE;
        }

        af_msg->type = MSG_BF_STATS;
        af_stats->stats_type_mask = STATS_BF;

        AF_LOW("none bayer stats_buf: %p",
          stats_event->stats_data[MSM_ISP_STATS_BF].stats_buf);
        af_stats->u.p_q3a_bf_stats[AF_KERNEL_PRIMARY] = stats_event->stats_data[MSM_ISP_STATS_BF].stats_buf;

        // If PD early interrupt is supported and enabled and
        // PDlib suport is in sensor side
        // If PD lib is in algo  side, this flag should be false
        // In the following PD stat comes from baf interrupt but PDLib
        // is in algo side
        boolean PD_early_interrupt_enabled =
          af_port->af_haf_tuning.pdaf.pd_extended.PD_early_interrupt_enabled;

        if (af_stats->u.p_q3a_bf_stats[AF_KERNEL_PRIMARY]->mask == MCT_EVENT_STATS_BF_FW &&
          !PD_early_interrupt_enabled) {
          rc = af_port_buf_div_init_buf_divert_ack_ctrl(af_port,
                 &af_stats->u.p_q3a_bf_stats[AF_KERNEL_PRIMARY]->u.fw_stats.pd_data,
                 NULL);
          AF_LOW("af_port_buf_div_init_buf_divert_ack_ctrl rc %d", rc);
          if (rc) {
            af_port_buf_div_add_pd_early(af_port,
              &af_stats->u.p_q3a_bf_stats[AF_KERNEL_PRIMARY]->u.fw_stats.pd_data);
          }
          else {
            AF_LOW("af_port_buf_div_init_buf_divert_ack_ctrl rc %d", rc);
          }
        }
      }

      if (stats_event->stats_mask & (1 << MSM_ISP_STATS_BF_SCALE)) {
        af_stats->stats_type_mask |= STATS_BF_SCL;
        AF_LOW("BF_SCALE stats_buf: %p",
            stats_event->stats_data[MSM_ISP_STATS_BF_SCALE].stats_buf);
        af_stats->u.p_q3a_bf_stats[AF_KERNEL_SCALE] =
            stats_event->stats_data[MSM_ISP_STATS_BF_SCALE].stats_buf;
      }

      /* Need to call this function for every frame */
      af_port->fd_prio_data.current_frame_id = stats_event->frame_id;
      if (af_port->af_initialized) {
        fdprio_process(&af_port->fd_prio_data, FDPRIO_CMD_PROC_COUNTERS);
      }

      af_msg->u.stats = af_stats;

      af_stats->ack_data = stats_ext_event;
      circular_stats_data_use(stats_ext_event);

      /* Enqueue the message to the AF thread */
      rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
      /* If the AF thread is inactive, it will not enqueue our
       * message and instead will free it. Then we need to manually
       * free the payload */
      if (rc == FALSE) {
        circular_stats_data_done(stats_ext_event, 0, 0, 0);
        /* In enqueue fail, memory is free inside q3a_af_thread_en_q_msg() */
        af_stats = NULL;
      }
    } else if (stats_event->stats_mask & (1 << MSM_ISP_STATS_BG) ||
      stats_event->stats_mask & (1 << MSM_ISP_STATS_AEC_BG)) {
      q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
        AF_SET_PARAM_BG_STATS, af_port);
      if (af_msg == NULL) {
        return FALSE;
      }
      af_bg_stats_data_t *bg_stats = &af_msg->u.af_set_parm.u.bg_stats;
      bg_stats->dummy_af_stats.isFake = TRUE;
      bg_stats->dummy_af_stats.ack_data = stats_ext_event;
      circular_stats_data_use(stats_ext_event);

      if (stats_event->stats_mask & (1 << MSM_ISP_STATS_BG)) {
        bg_stats->p_q3a_bg_stats =
          stats_event->stats_data[MSM_ISP_STATS_BG].stats_buf;
      } else {
        bg_stats->p_q3a_bg_stats =
          stats_event->stats_data[MSM_ISP_STATS_AEC_BG].stats_buf;
      }
      bg_stats->af_port = port;
      bg_stats->af_stats_cb = af_port_stats_done_callback;

      /* Enqueue the message to the AF thread */
      rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
    } else if (stats_event->stats_mask & (1 << MSM_ISP_STATS_IHIST)) {
      af_port_handle_send_fake_stats_on_ihist(af_port, stats_event);
    }
  }

  return rc;
} /* af_port_handle_stats_event */

/** af_port_handle_stream_crop_event:
 *    @af_port:   private port data
 *    @mod_event: module event
 *
 * When we receive the crop info, we need to save it to be able
 * to calculate the right ROI when the HAL sends the command to
 * set the ROI.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_stream_crop_event(af_port_private_t * af_port,
  mct_event_module_t * mod_evt)
{
  boolean                   rc = TRUE;
  mct_bus_msg_stream_crop_t *stream_crop;

  stream_crop = (mct_bus_msg_stream_crop_t *)mod_evt->module_event_data;
  if (!stream_crop) {
    AF_ERR("failed");
    return FALSE;
  }

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_STREAM_CROP_INFO, af_port);
  if (af_msg == NULL) {
    return FALSE;
  }

  af_msg->u.af_set_parm.u.stream_crop.pp_x           = stream_crop->x;
  af_msg->u.af_set_parm.u.stream_crop.pp_y           = stream_crop->y;
  af_msg->u.af_set_parm.u.stream_crop.pp_crop_out_x  = stream_crop->crop_out_x;
  af_msg->u.af_set_parm.u.stream_crop.pp_crop_out_y  = stream_crop->crop_out_y;
  af_msg->u.af_set_parm.u.stream_crop.vfe_map_x      = stream_crop->x_map;
  af_msg->u.af_set_parm.u.stream_crop.vfe_map_y      = stream_crop->y_map;
  af_msg->u.af_set_parm.u.stream_crop.vfe_map_width  = stream_crop->width_map;
  af_msg->u.af_set_parm.u.stream_crop.vfe_map_height = stream_crop->height_map;
  af_msg->u.af_set_parm.u.stream_crop.vfe_out_width  =
    af_port->stream_info.dim.width;
  af_msg->u.af_set_parm.u.stream_crop.vfe_out_height  =
    af_port->stream_info.dim.height;

  /* Also save the crop info locally */
  memcpy(&af_port->stream_crop, &af_msg->u.af_set_parm.u.stream_crop,
    sizeof(af_stream_crop_t));

  rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  if (af_port->reconfigure_ISP_pending == TRUE) {
    // TODO we should also remap roi and send to algo, we may need to do store roi
    /* Keep trying if failure occurs while sending the message. */
    if (af_port_send_ISP_reconfig_msg(af_port) == TRUE) {
      af_port->reconfigure_ISP_pending = FALSE;
    }
  }

  return rc;
} /* af_port_handle_stream_crop_event */

/** af_port_handle_isp_output_dim_event:
 *    @af_port:   private port data
 *    @mod_event: module event
 *
 * When we receive the isp dim info, we need to save it to be able
 * to calculate the right ROI when the HAL sends the command to
 * set the ROI.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_isp_output_dim_event(af_port_private_t *af_port,
  mct_stream_info_t *stream_info) {
  boolean rc = TRUE;
  af_input_from_isp_t *isp_info = NULL;

  if (!stream_info) {
    AF_ERR("failed");
    return FALSE;
  }

  if (stream_info->stream_type != CAM_STREAM_TYPE_PREVIEW) {
    return FALSE;
  }

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_UPDATE_ISP_INFO, af_port);
  if (af_msg == NULL) {
    return FALSE;
  }

  isp_info = &(af_msg->u.af_set_parm.u.isp_info);
  isp_info->width = stream_info->dim.width;
  isp_info->height = stream_info->dim.height;

  /* save the isp info here too */
  memcpy(&af_port->isp_info, isp_info, sizeof(af_input_from_isp_t));

  rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);


  return rc;
} /* af_port_handle_isp_output_dim_event */

/** af_port_set_move_lens_cb_data:
 *    @af_port: private port data
 *
 * Send the lens move command function to the library along with the port.
 * The port is needed to call the event function.
 *
 * Return void.
 **/
static void af_port_set_move_lens_cb_data(mct_port_t *port)
{
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_MOVE_LENS_CB, af_port);

  if (af_msg != NULL) {
    af_msg->u.af_set_parm.u.move_lens_cb_info.move_lens_cb =
      af_port_send_move_lens_cmd;
    af_msg->u.af_set_parm.u.move_lens_cb_info.object_id = (void *)port;

    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
} /* af_port_set_move_lens_cb_data */

/** af_port_handle_imglib_output:
 *    @af_port:   private port data
 *    @mod_event: module event
 *
 * Handle new face detection ROIs
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_imglib_output(af_port_private_t * af_port,
  mct_event_module_t * mod_evt)
{
  boolean                rc = TRUE;
  mct_imglib_af_output_t *imglib_af_output;
  af_sw_stats_t          *sw_stats;

  imglib_af_output = (mct_imglib_af_output_t *)mod_evt->module_event_data;
  if (!imglib_af_output) {
    AF_ERR("failed");
    return FALSE;
  }

  /* Copy stats from imglib to AF Port */
  af_port->p_sw_stats->frame_id = imglib_af_output->frame_id;
  af_port->p_sw_stats->fV = imglib_af_output->fV;
  af_port->p_sw_stats->pending = FALSE;
  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_IMGLIB_OUTPUT, af_port);
  if (af_msg == NULL) {
    return FALSE;
  }
  /* Pack imglib stats to Core */
  sw_stats = &(af_msg->u.af_set_parm.u.sw_stats);
  sw_stats->frame_id = imglib_af_output->frame_id;
  sw_stats->fV = imglib_af_output->fV;
  sw_stats->pending = FALSE;

  rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  return rc;
} /* af_port_handle_imglib_output */

/** af_port_handle_imglib_dcrf_output:
 *    @af_port:   private port data
 *    @mod_event: module event
 *
 * Handle DCRF event
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_imglib_dcrf_output(af_port_private_t *af_port,
  mct_event_module_t *mod_evt)
{
  boolean                rc = FALSE;
  mct_imglib_dcrf_result_t *dcrf_output;
  depth_service_input_t *input = &(af_port->ds_input);
  mct_imglib_dcrf_result_t *dciaf_set_parm = &(input->u.dciaf_info);
  dcrf_output = (mct_imglib_dcrf_result_t *)mod_evt->module_event_data;
  if (!dcrf_output) {
    AF_ERR("failed\n");
    return rc;
  }
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_DEPTH_SERVICE, af_port);
  if (af_msg == NULL) {
    AF_ERR("ERROR af_msg is NULL!");
    return rc;
  }

  /* Convert Raw data to Depth Map output*/
  /* Pack data to depthmap input*/
  input->info.frame_id = dcrf_output->id;
  input->info.type = DEPTH_SERVICE_DUAL_CAM_AF;
  input->info.x_win_num = 1;
  input->info.y_win_num = 1;
  memcpy(dciaf_set_parm, dcrf_output, sizeof(mct_imglib_dcrf_result_t));
  rc = depth_service_process(input, &(af_msg->u.af_set_parm.u.depth_service));
  if (TRUE == rc) {
    memcpy(&(af_msg->u.af_set_parm.u.depth_service.input.cal_data),
          &(input->cal_data), sizeof(depth_cal_data_t));

    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  } else {
    free(af_msg);
    af_msg = NULL;
  }
  return rc;
}

static boolean af_port_pdaf_proc_callback(void *data,
  void *output)
{
  boolean rc = FALSE;
  depth_service_output_t *depth_output = NULL;
  af_pdaf_data_t *pdaf_data = NULL;
  mct_port_t *port = NULL;
  af_port_private_t *af_port = NULL;
  isp_buf_divert_t *stats_buff = NULL;
  q3a_bf_stats_t* early_stats_buff = NULL;
  isp_buf_divert_t* idivert_stats_buff = NULL;
  pdaf_update_t *pdaf;
  mct_event_t       event;
  af_port_pdaf_status pdaf_status = AF_PORT_PDAF_STATUS_EXTRACT_FAILURE;
  boolean depth_map_conversion_status = FALSE;

  if (NULL == data || NULL == output) {
    AF_ERR("Invalid input");
    return FALSE;
  }

  pdaf_data = (af_pdaf_data_t *)data;
  depth_output = (depth_service_output_t *)output;

  if (NULL == pdaf_data) {
    AF_ERR("Invalid pointer: %p", pdaf_data);
    return FALSE;
  }

  port = (mct_port_t*)pdaf_data->object_id;
  stats_buff = (isp_buf_divert_t *)pdaf_data->stats_buff;
  if (NULL == stats_buff || NULL == port || NULL == port->port_private) {
    AF_ERR("Invalid pointer: %p, %p,...", port, stats_buff);
    return FALSE;
  }

  af_port = (af_port_private_t *)port->port_private;
  if (NULL == af_port) {
    AF_ERR("Invalid pointer: %p",af_port );
    return FALSE;
  }

  depth_service_input_t *input = &(af_port->ds_input);
  if (NULL == input) {
    AF_ERR("Invalid pointer: %p", input);
    return FALSE;
  }

  uint32_t buf_idx = 0;
  do {
    if (af_port->thread_data->thread_ctrl[af_port->camera_id].no_stats_mode) {
      AF_HIGH("In no stats mode, skip stats");
      pdaf_data->stats_buff = NULL;
      return rc;
    }

    depth_service_input_t *input = &(af_port->ds_input);
    pdaf = &(input->u.pdaf_info);

    af_pdaf_buf_type buf_type = pdaf_data->pdaf_early_interrupt_data.buf_type;
    // Initialize stat_process_result.
    // It gets updated when PD stat will be processed by PD lib
    pdaf->stat_process_result = PDAF_PDLIB_RESULT_PARTIAL;

    /* Pack data to sensor for parsing the buffer*/
    switch (buf_type) {
      // PD stat is from early interrupt
      case AF_PD_EARLY_BAF_BUF: {

        early_stats_buff =
          (q3a_bf_stats_t*)pdaf_data->stats_buff;
        if (!early_stats_buff) {
          AF_ERR("early_stats_buff is NULL pointer.");
          return FALSE;
        }

        pdaf->pd_stats =
          (uint8_t*)(early_stats_buff->u.fw_stats.pd_data.pd_buffer_data);
        input->u.pdaf_info.num_of_valid_data_wrt_camif =
          early_stats_buff->u.fw_stats.pd_data.pd_stats_read_data_length;
        input->info.frame_id = pdaf_data->pdaf_early_interrupt_data.frame_id;
        // set flag to mention early buf is in use
        af_port_buf_div_add_pd_early(af_port,
          &early_stats_buff->u.fw_stats.pd_data);
        buf_idx = early_stats_buff->u.fw_stats.pd_data.buf_idx;
        AF_LOW("PD early interrupt.");

      }
      break;
      case AF_PD_PREV_IDIVERT_BUF:
      case AF_PD_IDIVERT_BUF: {

        AF_LOW((buf_type == AF_PD_PREV_IDIVERT_BUF) ?
          "Buf type touch":"Buf type idivert");
        idivert_stats_buff =
          (isp_buf_divert_t *)pdaf_data->stats_buff;
        if (!idivert_stats_buff) {
          AF_ERR("idivert_stats_buff is NULL pointer.");
          return FALSE;
        }
        pdaf->pd_stats = idivert_stats_buff->vaddr;
        pdaf->num_of_valid_data_wrt_camif =
          input->u.pdaf_info.num_of_valid_data_wrt_camif =
          af_port->camif_height;
        input->info.frame_id = idivert_stats_buff->buffer.sequence;
        buf_idx = idivert_stats_buff->buffer.index;
        stats_buff = idivert_stats_buff;
      }
      break;
    }

    AF_HIGH("frame_id %d buf_idx %d pd_stats %p pd_stats_read_data_length %d buf_type: %d",
      input->info.frame_id, buf_idx, pdaf->pd_stats,
      input->u.pdaf_info.num_of_valid_data_wrt_camif, buf_type);

    if (pdaf->pd_stats == NULL || pdaf->num_of_valid_data_wrt_camif <= 0) {
      AF_ERR("Wrong information being recieved from ISP. frame id %d pd buffer %p number of valid line %d",
        input->info.frame_id, pdaf->pd_stats, pdaf->num_of_valid_data_wrt_camif);
      break;
    }


    pd_zero_touch_lag_info_t touch_info;
    af_port_pd_zero_touch_lag_access(af_port, &touch_info);
    AF_HIGH("is_it_first_touch %d", touch_info.is_it_first_touch);

    // If zero touch lag is supported and it is first frame which
    // user has touched it. Send touch ROI coordinate to PDLib
    if (AF_PD_PREV_IDIVERT_BUF == buf_type &&
      touch_info.is_it_first_touch) {
      /* pack ROI into an mct_event object*/
      event.direction = MCT_EVENT_UPSTREAM;
      event.identity = af_port->reserved_id;
      event.type = MCT_EVENT_MODULE_EVENT;
      event.u.module_event.type = MCT_EVENT_MODULE_STATS_PDAF_AF_WINDOW_UPDATE;
      event.u.module_event.module_event_data = (void *)(&touch_info.pdaf_config);
      event.u.module_event.current_frame_id = af_port->sof_id;
      AF_HIGH("Send ROI config to PDlib to handle ZTL, port =%p, event =%p",
        port, &event);

      rc = MCT_PORT_EVENT_FUNC(port)(port, &event);
      if (!rc) {
        AF_ERR("Error send ROI pdaf failed");
      }

      touch_info.is_it_first_touch = FALSE;
      af_port_pd_zero_touch_lag_update(af_port,
        AF_IS_FIRST_TOUCH, &touch_info);

    }

    /* Pack into an mct_event object*/
    event.direction = MCT_EVENT_UPSTREAM;
    event.identity = af_port->reserved_id;
    event.type = MCT_EVENT_MODULE_EVENT;
    event.u.module_event.type = MCT_EVENT_MODULE_STATS_PDAF_UPDATE;
    event.u.module_event.module_event_data = (void *)(pdaf);
    event.u.module_event.current_frame_id = af_port->sof_id;
    AF_LOW("send PDAF data to sensor, port =%p, event =%p",
      port, &event);
    MCT_PORT_EVENT_FUNC(port)(port, &event);

    pdaf_status = (af_port_pdaf_status)pdaf->status;

    AF_HIGH("frame_id %d buf_idx %d pd_stats %p "
      "pdaf_processing_result_early_int %d buf_type %d number of valid line %d",
      input->info.frame_id, buf_idx, pdaf->pd_stats,
      pdaf->stat_process_result, buf_type, pdaf->num_of_valid_data_wrt_camif);

    if (pdaf_status != AF_PORT_PDAF_STATUS_EXTRACT_SUCCESS) {
      AF_ERR("Depth_Err PDAF Error returned from sensor %d", pdaf->status);
      if (pdaf_status == AF_PORT_PDAF_STATUS_LIBRARY_MISSING) {
        af_port->is_pdaf_lib_available = FALSE;
        af_port_set_haf_enable(af_port);
      }
      break;
    }

    /* Convert Raw data to Depth Map output*/
    /* Pack data to depthmap input*/
    input->info.type = DEPTH_SERVICE_PDAF;
    input->info.x_win_num = pdaf->x_win_num;
    input->info.y_win_num = pdaf->y_win_num;
    depth_map_conversion_status =
      depth_service_process(input, depth_output);
    if(!depth_map_conversion_status) {
      AF_ERR("Error Converting Raw Data To Depth Map Output");
      break;
    }

    AF_LOW("pdaf proc succeed.");
    rc = TRUE;
  } while(0);

  pd_lib_early_result_t info;
  /* Ack the isp buffer back */
  // If pd stat is from baf interrupt, set early buffer done
  // If pd sata is from end of frame, set buffer divert done
  switch (pdaf_data->pdaf_early_interrupt_data.buf_type) {
    case AF_PD_EARLY_BAF_BUF: {

      info.pdaf_processing_result_early_int = pdaf->stat_process_result;

      af_tuning_multiwindow_adapter_t *mw_tuning =
        &af_port->af_algo_tuning.af_multiwindow;
      info.process_bottom_multi_window = FALSE;

      boolean multi_window_set_prop_enable = FALSE;
      /* Enable/disable mw based on setprop */
      STATS_ENABLE_MULTIWINDOW(multi_window_set_prop_enable);
      if (mw_tuning->enable || multi_window_set_prop_enable) { // multi window is enabled
        if (PDAF_PDLIB_RESULT_PARTIAL == pdaf->stat_process_result) { //pd lib is not confident
          info.process_bottom_multi_window = TRUE;
        }
      }

      af_port_pd_early_res_update(af_port, ALL, &info);
      af_port_buf_div_pd_early_done(
        af_port, &early_stats_buff->u.fw_stats.pd_data);
    }
      break;

    case AF_PD_PREV_IDIVERT_BUF: {

      info.process_bottom_multi_window = FALSE;
      info.pdaf_processing_result_early_int =
        pdaf->stat_process_result;
      af_port_pd_early_res_update(af_port, ALL, &info);
      af_port_buf_touch_done(af_port, idivert_stats_buff);
      af_port_buf_div_send_ack_if_ready(port, buf_idx);
    }
      break;

    case AF_PD_IDIVERT_BUF: {

      af_port_pd_early_res_access(af_port, &info);
      if (info.process_bottom_multi_window) { // multi window is enabled
        if (PDAF_PDLIB_RESULT_PARTIAL == pdaf->stat_process_result) { // There is detail on bottom window
          info.process_bottom_multi_window = FALSE;
          info.pdaf_processing_result_early_int =
            pdaf->stat_process_result;
          af_port_pd_early_res_update(af_port, ALL, &info);
        }
      }

      AF_LOW("af_port_buf_div_done");
      af_port_buf_div_done(af_port, idivert_stats_buff);
      af_port_buf_div_send_ack_if_ready(port, buf_idx);
    }
      break;
  }

  AF_HIGH("frame_id %d af_port_buf_div_pd_early_done result %d process_bottom_multi_window %d",
    input->info.frame_id, pdaf->stat_process_result,
    info.process_bottom_multi_window);
  pdaf_data->stats_buff = NULL;
  return rc;
}

/** af_port_handle_pdaf_stats:
 *    @af_port: private port data
 *    @mod_evt: module event structure
 *
 *
 * Return void.
 **/
static boolean af_port_handle_pdaf_stats(mct_port_t *port,
  af_port_private_t * af_port, mct_event_module_t * mod_evt,
  boolean is_early_interrupt)
{
  q3a_thread_af_msg_t *af_msg = NULL;
  af_pdaf_data_t *pdaf_data;
  boolean rc = FALSE;
  // This flag remain TRUE unless a logic make it FALSE
  boolean enqueue_pdaf_process_msg = TRUE;
  af_pdaf_buf_type buf_type;

  /* Sanity check 1*/
  if(af_port->ds_input.cal_data.is_pdaf_supported != TRUE) {
    AF_HIGH("Depth_Err Pdaf not supported. Shouldn't get PDAF stats");
    return rc;
  }

  /* Sanity check 2*/
  if(!af_port->is_pdaf_lib_available) {
    AF_HIGH("PDAF Lib is not present. Cannot give to sensor");
    return rc;
  }

  /* Optimization: Do not initialize local buffer, use the same set param buffer
   * to populate pdaf data*/
  af_msg = af_port_create_msg(MSG_AF_SET,
          AF_SET_PARAM_PDAF_DATA, af_port);
  if (af_msg == NULL) {
    AF_ERR("Depth_Err af_msg is NULL");
    return rc;
  }
  pdaf_data = &af_msg->u.af_set_parm.u.pdaf_data;
  pdaf_data->object_id = (void*)port;
  pdaf_data->pdaf_proc_cb = af_port_pdaf_proc_callback;
  void* stats_buff = NULL;
  uint32_t buf_idx = 0;
  uint32_t frame_id = 0;
  q3a_bf_stats_t* early_stats_buff = NULL;

  pd_zero_touch_lag_info_t touch_info;
  af_port_pd_zero_touch_lag_access(af_port, &touch_info);
  AF_HIGH("ROI type %d is_it_first_frame %d prev frame buf idx %d",
    touch_info.roi_type, touch_info.is_it_first_frame,
    touch_info.prev_frame_idivert_buf_idx);

  boolean PD_early_interrupt_enabled =
    af_port->af_haf_tuning.pdaf.pd_extended.PD_early_interrupt_enabled;

  // If this method gets called from early/BAF interrupt
  if (is_early_interrupt) {
    mct_event_stats_ext_t* stats_ext_event =
      (mct_event_stats_ext_t *)mod_evt->module_event_data;
    mct_event_stats_isp_t *stats_event = stats_ext_event->stats_data;

    if (!(stats_event->stats_mask & (1 << MSM_ISP_STATS_BF))) {
      AF_LOW("It is not a baf stat.(MSM_ISP_STATS_BF) stats_mask %d",
        stats_event->stats_mask);
      free(af_msg);
      af_msg = NULL;
      return rc;
    }

    int8_t prev_frame_buf_idx = touch_info.prev_frame_idivert_buf_idx;

    // Frame 0 doesn't have a previous frame
    isp_buf_divert_t  *prev_isp_buff = NULL;
    if (-1 != prev_frame_buf_idx) {
      prev_isp_buff =
        &af_port->div_buf_ack.div_buf_ack_ctrl[prev_frame_buf_idx].isp_buf_divert;
      if (prev_isp_buff == NULL) {
        AF_ERR("Error accessing memory.");
        free(af_msg);
        af_msg = NULL;
        return FALSE;
      }
    }

    early_stats_buff =
      (q3a_bf_stats_t*)stats_event->stats_data[MSM_ISP_STATS_BF].stats_buf;

    if (!early_stats_buff){
      AF_ERR("bf_stats is NULL frame_id %d", stats_event->frame_id);
      free(af_msg);
      af_msg = NULL;
      return rc;
    }

    rc = af_port_buf_div_init_buf_divert_ack_ctrl(af_port,
      &early_stats_buff->u.fw_stats.pd_data, NULL);
    if (!rc) {
      AF_ERR("Fail to init buf divert ack ctrl");
      free(af_msg);
      af_msg = NULL;
      return rc;
    }

    // Handle buffer book keeping
    if (!PD_early_interrupt_enabled) {

      af_port_buf_div_pd_early_done(af_port, &early_stats_buff->u.fw_stats.pd_data);
      if (-1 != prev_frame_buf_idx) {
        // Since ZTL is not enabled, mark previous frame buffer done.
        af_port_buf_touch_done(af_port, prev_isp_buff);
        af_port_buf_div_send_ack_if_ready(port, prev_frame_buf_idx);
      }
      enqueue_pdaf_process_msg = FALSE;
    }
    else { // This method get called from early/baf interrupt and ZTL and early interrupt are enabled

      if (AF_ROI_TYPE_TOUCH == touch_info.roi_type){ // Handle ZTL

        pdaf_data->pdaf_early_interrupt_data.buf_type =
          AF_PD_PREV_IDIVERT_BUF;

        // For touch, current frame PD stat won't be processed, so early_buf should be marked done
        af_port_buf_div_pd_early_done(af_port, &early_stats_buff->u.fw_stats.pd_data);

        if (-1 == prev_frame_buf_idx) {
          AF_HIGH("Zero touch lag won't work on first frame");
          free(af_msg);
          af_msg = NULL;
          return TRUE;
        }

        // It is touch, so need to send PD stat from previous frame
        buf_idx = (uint8_t)prev_frame_buf_idx;
        stats_buff = (void*)prev_isp_buff;
        frame_id = prev_isp_buff->buffer.sequence;

        AF_HIGH("Zero Touch lag: current frame id %d buf_idx %d "
          "process previous idivert frame_id %d buf_idx %d pd_data %p "
          "stats_buff %p isp_buff %p is_early_interrupt: %d ",
              stats_event->frame_id, buf_idx, frame_id, buf_idx,
              prev_isp_buff->vaddr, stats_buff, prev_isp_buff, is_early_interrupt);

        // Just for first touch frame the previous buffer wil be
        // sent to PDLib. After first frame the flow will be back to normal.
        // That's why ROI get changed to none touch.
        touch_info.roi_type = AF_ROI_TYPE_GENERAL;
        af_port_pd_zero_touch_lag_update(af_port,
          AF_PD_TOUCH_ROI_TYPE, &touch_info);

      }
      else { // if this method get called from early interrupt handleing none touch ROI

        // Release previous idivert buffer if it is not needed anymore
        // frame 0 doesn't have valid previous frame (prev_frame_buf_idx == -1)
        if (-1 != prev_frame_buf_idx) {
          // Since it is not Touch ROI, no need to keep previous frame buffer.
          // Mark that buffer done
          af_port_buf_touch_done(af_port, prev_isp_buff);
          af_port_buf_div_send_ack_if_ready(port, prev_frame_buf_idx);
        }

        pd_lib_early_result_t info;
        af_port_pd_early_res_access(af_port, &info);
        // skip processing current frame PD stat, if previous PD stat frame
        // has processed PD stat for bottom multi window
        if (info.process_bottom_multi_window) {
          af_port_buf_div_pd_early_done(af_port, &early_stats_buff->u.fw_stats.pd_data);
          // Since ZTL is not enabled, mark previous frame buffer done.
          af_port_buf_touch_done(af_port, prev_isp_buff);
          af_port_buf_div_send_ack_if_ready(port, prev_frame_buf_idx);
          AF_HIGH("Skip processing current frame %d PD stat, Previous frame %d bottom Window of multi window has been processed.",
            stats_event->frame_id, prev_frame_buf_idx);
          info.process_bottom_multi_window = FALSE;
          af_port_pd_early_res_update(
            af_port, AF_PDLIB_BOTTOM_MULTI_WIN_PROCESS, &info);
          free(af_msg);
          return rc;
        }

        pdaf_data->pdaf_early_interrupt_data.buf_type =
          AF_PD_EARLY_BAF_BUF;

        if (!(early_stats_buff->mask == MCT_EVENT_STATS_BF_FW)) {
          AF_LOW("It is not a baf stat.(MCT_EVENT_STATS_BF_FW)  mask %d",
            early_stats_buff->mask);
          free(af_msg);
          af_msg = NULL;
          return rc;
        }

        frame_id = stats_event->frame_id;
        pdaf_data->pdaf_early_interrupt_data.frame_id = frame_id;
        buf_idx = early_stats_buff->u.fw_stats.pd_data.buf_idx;
        stats_buff = (void*)early_stats_buff;

        AF_HIGH("PD early int. is_early_interrupt %d frame_id %d buf_idx %d "
          "stats_buff %p pd_data %p",
          is_early_interrupt, frame_id, buf_idx, stats_buff,
          early_stats_buff->u.fw_stats.pd_data.pd_buffer_data);
      }
    }
  } else { // PD stat from end of frame

    pdaf_data->pdaf_early_interrupt_data.buf_type =
      AF_PD_IDIVERT_BUF;

    isp_buf_divert_t  *idivert_stats_buff =
      (isp_buf_divert_t *)mod_evt->module_event_data;
    isp_buf_divert_t  *isp_buff = NULL;

    isp_buff =
      &af_port->div_buf_ack.div_buf_ack_ctrl[idivert_stats_buff->buffer.index].isp_buf_divert;
    if (isp_buff == NULL) {
      AF_ERR("Error allocating memory.");
      free(af_msg);
      af_msg = NULL;
      return rc;
    }

    memcpy(isp_buff, idivert_stats_buff, sizeof(isp_buf_divert_t));
    buf_idx = isp_buff->buffer.index;
    stats_buff = (void*)isp_buff;
    frame_id = isp_buff->buffer.sequence;

    // update buf index for touch process in next fram in baf interrupt
    touch_info.prev_frame_idivert_buf_idx = buf_idx;
    af_port_pd_zero_touch_lag_update(af_port,
      AF_PD_TOUCH_PREV_FRAME_BUF_IDX, &touch_info);

    AF_HIGH("idivert frame_id %d buf_idx %d pd_data %p stats_buff %p "
      "isp_buff %p is_early_interrupt: %d ",
      frame_id, buf_idx, isp_buff->vaddr, stats_buff, isp_buff,
      is_early_interrupt);

    pd_lib_early_result_t info;
    af_port_pd_early_res_access(af_port, &info);
    // There is no need to process PD stat at the end of the frame again,
    // If pd stat has been already processed in early interrupt successfully or
    // if PD stat has been processed for touch.
    if (touch_info.is_it_first_frame ||
      (TRUE == PD_early_interrupt_enabled &&
        info.pdaf_processing_result_early_int == PDAF_PDLIB_RESULT_DONE)) {
      AF_HIGH("End of frame: frame_id %d PD stat has been processed. "
        "No process needed.af_port->pdaf_processing_result_early_int %d"
        , frame_id, info.pdaf_processing_result_early_int);
      enqueue_pdaf_process_msg = FALSE;

      // This buffer is not needed, release it.
      af_port_buf_div_done(af_port, (isp_buf_divert_t *)stats_buff);
      af_port_buf_div_send_ack_if_ready(
        port, ((isp_buf_divert_t *)stats_buff)->buffer.index);

      //first touch frame has been processed so make it false
      if (touch_info.is_it_first_frame) {
        touch_info.is_it_first_frame = FALSE;
        af_port_pd_zero_touch_lag_update(af_port,
          AF_IS_FIRST_FRAME_TOUCH, &touch_info);
      }
    }
  }

  pdaf_data->stats_buff = stats_buff;

  if (enqueue_pdaf_process_msg) {
    buf_type = pdaf_data->pdaf_early_interrupt_data.buf_type;
    rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
  else {
    // This is not an actual error. Its just that the
    // end of frame PD stats need not be processed.
    // Whenever there is no need to process the stats, the
    // buffer ack has already been handled. so set rc = TRUE to avoid sending duplicate ack
    // to ISP resulting in false error log messages
    rc = TRUE;
    AF_HIGH("enqueue_pdaf_process_msg is FALSE. No need process PD stat. "
      "is_early_interrupt %d",
      is_early_interrupt);
    free(af_msg);
    af_msg = NULL;
  }

  // if the message has been failed to be queued, release the buffer
  if (!rc) {
    AF_HIGH("The msg hasn't been queued. rc %d", rc);
    switch (buf_type) {
      case AF_PD_EARLY_BAF_BUF: {
        af_port_buf_div_pd_early_done(
          af_port, &early_stats_buff->u.fw_stats.pd_data);
      }
        break;

      case AF_PD_PREV_IDIVERT_BUF:
      case AF_PD_IDIVERT_BUF: {
        af_port_buf_div_done(af_port, (isp_buf_divert_t *)stats_buff);
        af_port_buf_div_send_ack_if_ready(port, ((isp_buf_divert_t *)stats_buff)->buffer.index);
      }
        break;
    }
  }

  return rc;
}
/** af_port_handle_isp_stats_info:
 *    @af_port: private port data
 *    @mod_evt: module event structure
 *
 * Send AF stats capabilities to the library.
 *
 * Return void.
 **/
static void af_port_handle_isp_stats_info(af_port_private_t * af_port,
  mct_event_module_t * mod_evt)
{
  mct_stats_info_t *stats_info =
    (mct_stats_info_t *)mod_evt->module_event_data;

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_UPDATE_STATS_CAPS, af_port);
  if (af_msg != NULL) {
    memcpy(&af_msg->u.af_set_parm.u.af_stats_caps_info, stats_info,
      sizeof(mct_stats_info_t));
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }

} /* af_port_handle_stats_info */


/** af_port_handle_ISP_stats_request:
 *    @af_port:   private port data
 *    @mod_event: module event
 *
 * Handle new face detection ROIs
 *
 * Return None
 **/
void af_port_handle_ISP_stats_request(
  mct_port_t *port, mct_event_module_t * mod_evt)
{
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  mct_event_request_stats_type *stats_info =
    (mct_event_request_stats_type *)mod_evt->module_event_data;

  if (ISP_STREAMING_OFFLINE == stats_info->isp_streaming_type) {
    AF_HIGH("AF doesn't support offline processing yet. Returning.");
    return;
  }

  if (stats_info->supported_stats_mask & (1 << MSM_ISP_STATS_BF)) {
    //Set the stats req for ISP
    stats_info->enable_stats_mask |= (1 << MSM_ISP_STATS_BF);
    stats_info->enable_stats_parse_mask |= (1 << MSM_ISP_STATS_BF);
    stats_info->stats_comp_grp_mask[STATS_GROUP_1] |= (1 << MSM_ISP_STATS_BF);
    //Store this locally
    af_port->enable_stats_mask |= (1 << MSM_ISP_STATS_BF);
  } else if (stats_info->supported_stats_mask & (1 << MSM_ISP_STATS_AF)){
    //Set the stats req for ISP
    stats_info->enable_stats_mask |= (1 << MSM_ISP_STATS_AF);
    stats_info->enable_stats_parse_mask |= (1 << MSM_ISP_STATS_AF);
    stats_info->stats_comp_grp_mask[STATS_GROUP_1] |= (1 << MSM_ISP_STATS_AF);
    //Store this locally
    af_port->enable_stats_mask |= (1 << MSM_ISP_STATS_AF);
  }

  /*Enable iHist only if no other AF Stats Mask is enabled*/
  if(!af_port->enable_stats_mask && stats_info->supported_stats_mask & (1 << MSM_ISP_STATS_IHIST)) {
    stats_info->enable_stats_mask |= (1 << MSM_ISP_STATS_IHIST);
    /* We only need the IRQ from IHIST stats */
    stats_info->enable_stats_parse_mask &= ~(uint32_t)(1 << MSM_ISP_STATS_IHIST);
    stats_info->stats_comp_grp_mask[STATS_GROUP_1] |= (1 << MSM_ISP_STATS_IHIST);
    af_port->enable_stats_mask |= (1 << MSM_ISP_STATS_IHIST);
  }

  /* Check if bf scale is supported.*/
  if (stats_info->supported_stats_mask & (1 << MSM_ISP_STATS_BF_SCALE)) {
    //Set the stats req for ISP
    stats_info->enable_stats_mask |= (1 << MSM_ISP_STATS_BF_SCALE);
    stats_info->enable_stats_parse_mask |= (1 << MSM_ISP_STATS_BF_SCALE);
    stats_info->stats_comp_grp_mask[STATS_GROUP_1] |= (1 << MSM_ISP_STATS_BF_SCALE);
    //Store this locally
    af_port->enable_stats_mask |= (1 << MSM_ISP_STATS_BF_SCALE);
  }

  AF_LOW("MCT_EVENT_MODULE_REQUEST_STATS_TYPE:Required AF stats mask = 0x%x parsemask=0x%x",
    stats_info->enable_stats_mask,stats_info->enable_stats_parse_mask);

} /* af_port_handle_imglib_output */

/** af_port_handle_module_event:
 *    @port:    AF port data
 *    @mod_evt: module event
 *
 * Handle module event received at AF port.
 *
 * Return void
 **/
static void af_port_handle_module_event(mct_port_t *port,
  mct_event_module_t *mod_evt)
{
  af_port_private_t *af_port =
    (af_port_private_t *)(port->port_private);
  stats_ext_return_type ret = STATS_EXT_HANDLING_PARTIAL;

  AF_LOW("Handle AF module event of type: %d", mod_evt->type);

  /* Check if extended handling to be performed */
  if (af_port->func_tbl.ext_handle_module_event) {
    ret = af_port->func_tbl.ext_handle_module_event(port, mod_evt);
    if (STATS_EXT_HANDLING_COMPLETE == ret) {
      AF_LOW("Module event handled in extension function!");
      return;
    }
  }

  switch (mod_evt->type) {
    /* Event to update chromatix pointer */
  case MCT_EVENT_MODULE_SET_CHROMATIX_WRP: {
    af_port_handle_chromatix_update_evt(port, mod_evt);
    af_port_set_move_lens_cb_data(port);
    af_port_set_af_data(af_port);
  }
    break;

  case MCT_EVENT_MODULE_STATS_AF_GET_THREAD_OBJECT: {
    q3a_thread_af_get_port_data_t *af_get_data =
      (q3a_thread_af_get_port_data_t *)(mod_evt->module_event_data);

    /* Getting data for thread */
    af_get_data->camera.af_port = port;
    af_get_data->camera.af_cb = af_port_callback;
    af_get_data->camera.af_stats_cb = af_port_stats_done_callback;
    af_get_data->camera.af_obj = &(af_port->af_object);
  }
    break;

  case MCT_EVENT_MODULE_STATS_AF_SET_THREAD_OBJECT: {
    q3a_thread_af_set_port_data_t *af_set_data =
      (q3a_thread_af_set_port_data_t *)(mod_evt->module_event_data);

    /* Set thread to be use to en_q msg*/
    af_port->camera_id                = af_set_data->camera_id;
    af_port->thread_data              = af_set_data->af_thread_data;
    af_port->fd_prio_data.camera_id   = af_set_data->camera_id;
    af_port->fd_prio_data.thread_data = af_set_data->af_thread_data;

    AF_HIGH("Set thread object: camera_id: %u", af_port->camera_id);
  }
    break;

  case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
    /* Update sensor info */
    af_port_handle_sensor_update(af_port, mod_evt);
    af_port_handle_stream_mode_update(af_port);
  }
    break;

    /* Event when new AF stats is available */
  case MCT_EVENT_MODULE_STATS_EXT_DATA: {
    if (af_port->af_initialized) {
      boolean rc = FALSE;

      // PDaf stat handleing should always gets called before BAF stat handleing,
      // Becuase PDLib output will be pushed to AF algo in BAF stat handleing
      //pdaf stat is from baf early interrupt
      boolean is_early_interrupt = TRUE;
      rc = af_port_handle_pdaf_stats(port, af_port, mod_evt, is_early_interrupt);

      af_port_handle_stats_event(port, mod_evt);
    }
  } /* case MCT_EVENT_MODULE_EXT_STATS_DATA */
  break;

  /* Event when AEC has some update to share*/
  case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
    if (af_port->af_initialized) {
      af_port_handle_aec_update(af_port, mod_evt);
    }
  }
    break;

  case MCT_EVENT_MODULE_STATS_AEC_MANUAL_UPDATE: {
    if (af_port->af_initialized) {
       af_port_handle_aec_manual_update(af_port, mod_evt);
    }
  }
   break;

  /* Event when AEC has some update to share*/
  case MCT_EVENT_MODULE_STATS_AWB_UPDATE: {
    if (af_port->af_initialized) {
      af_port_handle_awb_update(af_port, mod_evt);
    }
  }
    break;
  /* Event on every frame containing the crop info */
  case MCT_EVENT_MODULE_STREAM_CROP: {
    if (af_port->af_initialized) {
      af_port_handle_stream_crop_event(af_port, mod_evt);
    }
  }
    break;

  case MCT_EVENT_MODULE_ISP_OUTPUT_DIM: {
    mct_stream_info_t *stream_info =
      (mct_stream_info_t *)mod_evt->module_event_data;

    if (af_port->af_initialized) {
      AF_LOW("Handle ISP output dim event");
      af_port_handle_isp_output_dim_event(af_port, stream_info);
    } else {
      /* save the stream info locally */
      AF_LOW("AF Not initialized. Save output dim locally!");
      af_port->pre_init_updates.is_stream_info_updated = TRUE;
      memcpy(&af_port->pre_init_updates.stream_info, stream_info,
        sizeof(mct_stream_info_t));
    }
  }
    break;

  case MCT_EVENT_MODULE_MODE_CHANGE: {
    /* Stream mode has changed */
    af_port->stream_info.stream_type =
      ((stats_mode_change_event_data*)(mod_evt->module_event_data))->stream_type;
    af_port->reserved_id =
      ((stats_mode_change_event_data*)(mod_evt->module_event_data))->reserved_id;
  }
    break;

  case MCT_EVENT_MODULE_PREVIEW_STREAM_ID: {
    mct_stream_info_t  *stream_info =
          (mct_stream_info_t *)(mod_evt->module_event_data);
    af_port->stream_info.dim.width = stream_info->dim.width;
    af_port->stream_info.dim.height = stream_info->dim.height;
  }
    break;

  case MCT_EVENT_MODULE_FACE_INFO: {
    if (af_port->af_initialized) {
      AF_LOW("New Face ROI received");
      af_port_handle_fd_event(af_port, mod_evt);
    }
  }
    break;

  case MCT_EVENT_MODULE_STATS_GYRO_STATS_AF:
    if (af_port->af_initialized && af_port->is_af_hj_enabled) {
      af_port_handle_gyro_update(af_port, mod_evt);
    }
    break;

  case MCT_EVENT_MODULE_STATS_GYRO_STATS:
    if (af_port->af_initialized && af_port->is_af_hj_enabled) {
      af_port_handle_gyro_update(af_port, mod_evt);
    }
    break;

  case MCT_EVENT_MODULE_IMGLIB_AF_OUTPUT: {
    if (af_port->af_initialized) {
      af_port_handle_imglib_output(af_port, mod_evt);
    }
  }
    break;

 case MCT_EVENT_MODULE_IMGLIB_DCRF_OUTPUT: {
   if (af_port->af_initialized) {
     af_port_handle_imglib_dcrf_output(af_port, mod_evt);
   }
 }
   break;

  case MCT_EVENT_MODULE_GET_AF_SW_STATS_FILTER_TYPE: {
    boolean rc = TRUE;
    /* Allocate memory to create AF message. we'll post it to AF thread.*/
    q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_GET,
      AF_GET_PARAM_SW_STATS_FILTER_TYPE, af_port);
    if (af_msg == NULL) {
      return;
    }

    af_msg->sync_flag = TRUE;
    rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
    if (rc) {
      af_sw_filter_type *sw_stats_type =
        (af_sw_filter_type *)mod_evt->module_event_data;
      *sw_stats_type = af_msg->u.af_get_parm.u.af_sw_stats_filter_type;
    }
    free(af_msg);
    af_msg = NULL;
  }
    break;

  case MCT_EVENT_MODULE_HFR_MODE_NOTIFY: {
    int32_t *hfr_mode = (int32_t *)(mod_evt->module_event_data);
    boolean rc = TRUE;
    AF_LOW("HFR Mode: %d!", *hfr_mode);

    /* Allocate memory to create AF message. we'll post it to AF thread.*/
    q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
      AF_SET_PARAM_HFR_MODE, af_port);
    if (af_msg == NULL) {
      return;
    }

    af_msg->u.af_set_parm.u.hfr_mode = *hfr_mode;
    af_msg->sync_flag = TRUE;
    rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
    free(af_msg);
    af_msg = NULL;
  }
    break;
  case MCT_EVENT_MODULE_GET_AEC_LUX_INDEX: {
    float *lux_index = (float *)mod_evt->module_event_data;
    *lux_index = af_port->latest_aec_info.lux_idx;
  }
    break;
  case MCT_EVENT_MODULE_ISP_STATS_INFO: {
    AF_LOW("Handle ISP stats info event!");
    af_port_handle_isp_stats_info(af_port, mod_evt);
  }
    break;

  case MCT_EVENT_MODULE_GRAVITY_VECTOR_UPDATE: {
    if (af_port->af_initialized) {
      af_port_handle_gravity_update(port, mod_evt);
    }
  }
   break;

  case MCT_EVENT_MODULE_REQUEST_STATS_TYPE: {
    af_port_handle_ISP_stats_request(port, mod_evt);
  }
  break;

  case MCT_EVENT_MODULE_TOF_UPDATE: {
    tof_update_t *tof_update;

    // just save the information locally
    tof_update = (tof_update_t *)mod_evt->module_event_data;
    af_port->tof_data.tof_update = *tof_update;
    af_port->tof_data.is_updated = TRUE;
  }
  break;

  case MCT_EVENT_MODULE_ISP_DIVERT_TO_3A: {
    isp_buf_divert_t  *stats_buff =
      (isp_buf_divert_t *)mod_evt->module_event_data;
    isp_buf_divert_t  *isp_buff = NULL;
    boolean rc = FALSE;
    if(stats_buff->stats_type != PD_STATS) {
      /* AF module only handles PD_STATS buffer */
      break;
    }

    rc = af_port_buf_div_init_buf_divert_ack_ctrl(af_port, NULL, stats_buff);
    if (FALSE == rc) {
      AF_ERR("Fail to init buf divert ack ctrl");
      break;
    }

    isp_buff =
      &af_port->div_buf_ack.div_buf_ack_ctrl[stats_buff->buffer.index].isp_buf_divert;
    if (isp_buff == NULL) {
      AF_ERR("Error allocating memory.");
      break;
    }

    memcpy(isp_buff, stats_buff, sizeof(isp_buf_divert_t));
    AF_HIGH("Received PDAF stats end of frame: "
      "frame_id %d buf_idx %d pd_sata %p stats_buff %p pd_sata %p",
      stats_buff->buffer.sequence, stats_buff->buffer.index,
      stats_buff->vaddr, stats_buff, isp_buff->vaddr);

    boolean is_early_interrupt = FALSE;
    rc = af_port_handle_pdaf_stats(port, af_port, mod_evt, is_early_interrupt);
  }
  break;

  case MCT_EVENT_MODULE_SET_FAST_AEC_CONVERGE_MODE: {
    mct_fast_aec_mode_t *fast_aec_mode =
      (mct_fast_aec_mode_t *)(mod_evt->module_event_data);
    /* Allocate memory to create AF message. we'll post it to AF thread.*/
    q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
      AF_SET_PARAM_FAST_AEC_ENABLED, af_port);
    if (af_msg != NULL) {
      af_msg->u.af_set_parm.u.fast_aec_mode = fast_aec_mode->enable;
      q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
    }
  }
    break;

  default: {
    AF_LOW("Default. no action!");
  }
    break;
  }
} /* af_port_handle_module_event */

/** af_port_handle_set_roi_evt:
 *   @af_port:   private AF port structure
 *   @set_parm:  a message to populate
 *   @roi_input: new ROI information
 *
 * Handle set parameter event to set AF ROI.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_set_roi_evt(
  af_port_private_t *af_port,
  af_set_parameter_t *set_parm,
  af_roi_info_t *roi_input) {
  af_roi_info_t temp_roi;
  boolean rc = FALSE;

  AF_HIGH("frm_id: %d num_roi: %d, type:%d, roi_update: %d",
    roi_input->frm_id, roi_input->num_roi, roi_input->type, roi_input->roi_updated);

  /* set updated flag to false to avoid duplicate roi calls.
     * that comes because of issue in stats_port where it sends
     * multiple event for same event during stream on. */
  if (roi_input->roi_updated == true) {
    roi_input->roi_updated = false;
  } else {
    AF_LOW("InValid ROI. return!");
    return FALSE;
  }

  /* Copy roi data into temp before making adjustment for preview to
     * camif conversion. this will avoid overwriting event data. */
  memcpy(&temp_roi, roi_input, sizeof(af_roi_info_t));

  pd_zero_touch_lag_info_t touch_info;

  // Touch ROI
  if (roi_input->num_roi > 0) {
    memcpy(&temp_roi.roi[0], &roi_input->roi[0], sizeof(af_roi_t));
    rc = af_port_map_input_roi_to_camif(af_port, &temp_roi.roi[0]);

    if (rc == FALSE) {
      AF_LOW("Failure mapping ROI. return!");
      return FALSE;
    }

    // Upper layer always send touch ROI type
    // but when real touch happen, the number of ROI will be more
    // than 0. When number of ROI is 0, eventough upper layer send
    // touch ROI, it will be chnaged to default ROI on the lower layer.
    touch_info.roi_type = roi_input->type;

    boolean PD_early_interrupt_enabled =
      af_port->af_haf_tuning.pdaf.pd_extended.PD_early_interrupt_enabled;

    // Handleing ROI for zero touch lag feature
    // Save ROI info and adjust ROI for touch
    if (PD_early_interrupt_enabled) {

      touch_info.is_it_first_frame = TRUE;
      touch_info.is_it_first_touch = TRUE;
      af_port_pd_zero_touch_lag_update(af_port,
        AF_IS_FIRST_TOUCH, &touch_info);
      af_port_pd_zero_touch_lag_update(af_port,
        AF_IS_FIRST_FRAME_TOUCH, &touch_info);

      unsigned int camif_width = af_port->camif_width;
      unsigned int camif_height = af_port->camif_height;
      touch_info.pdaf_config.pdaf_sw_window_mode = FIXED_GRID_WINDOW;
      touch_info.pdaf_config.fixed_grid_window_configure.window_number_hori = (unsigned int)roi_input->num_roi;
      touch_info.pdaf_config.fixed_grid_window_configure.window_number_ver = (unsigned int)roi_input->num_roi;


      af_algo_tune_parms_adapter_t  *p_tuning = af_port->p_af_tuning;
      //using normal light config for now even for low light
      float h_clip_ratio = p_tuning->af_vfe[Q3A_AF_KERNEL_PRIMARY].config.h_clip_ratio_normal_light;
      float v_clip_ratio = p_tuning->af_vfe[Q3A_AF_KERNEL_PRIMARY].config.v_clip_ratio_normal_light;
      float touch_factor = p_tuning->af_vfe[Q3A_AF_KERNEL_PRIMARY].config.touch_scaling_factor_normal_light;
      float zoom_factor = 1.0;
      zoom_factor = (float)af_port->stream_crop.vfe_map_width / (float)camif_width;

      int dx = (camif_width * h_clip_ratio * touch_factor * zoom_factor);
      int dy = (camif_height * v_clip_ratio * touch_factor * zoom_factor);
      int ctr_x;
      int ctr_y;

      /* calculate center ROI coordinatet */
      if ((((temp_roi.roi[0].x + temp_roi.roi[0].dx) >= camif_width) ||
        ((temp_roi.roi[0].y + temp_roi.roi[0].dy) >= camif_height)) ||
        (!(temp_roi.roi[0].dx) && !(temp_roi.roi[0].dy))) {
        AF_MSG_LOW(" ROI limit check failed. Using Center ROI!");
        ctr_x = camif_width / 2;
        ctr_y = camif_height / 2;
      }
      else {
        ctr_x = temp_roi.roi[0].x + (temp_roi.roi[0].dx / 2);
        ctr_y = temp_roi.roi[0].y + (temp_roi.roi[0].dy / 2);
      }

      int x = (uint16)((ctr_x < (dx / 2)) ?
        0 : (ctr_x - (dx / 2)));
      int y = (uint16)((ctr_y < (dy / 2)) ?
        0 : (ctr_y - (dy / 2)));

      /* Note: PDAF Lib and sensor expects CAMIF size ratio coordinates, not absolute coordinate */
      touch_info.pdaf_config.fixed_grid_window_configure.af_fix_window.pdaf_address_start_hori =
        (x / (float)camif_width);
      touch_info.pdaf_config.fixed_grid_window_configure.af_fix_window.pdaf_address_start_ver =
        (y / (float)camif_height);
      touch_info.pdaf_config.fixed_grid_window_configure.af_fix_window.pdaf_address_end_hori =
        (x + dx) / ((float)camif_width);
      touch_info.pdaf_config.fixed_grid_window_configure.af_fix_window.pdaf_address_end_ver =
        (y + dy) / ((float)camif_height);

      if (touch_info.pdaf_config.fixed_grid_window_configure.af_fix_window.pdaf_address_end_hori > 1)
        touch_info.pdaf_config.fixed_grid_window_configure.af_fix_window.pdaf_address_end_hori = 1.0;

      if (touch_info.pdaf_config.fixed_grid_window_configure.af_fix_window.pdaf_address_end_ver > 1)
        touch_info.pdaf_config.fixed_grid_window_configure.af_fix_window.pdaf_address_end_ver = 1.0;

      AF_HIGH("Touch ROI after map to camif (x %d y %d dx %d dy %d zoom_factor %f)",
        temp_roi.roi[0].x, temp_roi.roi[0].y, temp_roi.roi[0].dx,
        temp_roi.roi[0].dy, zoom_factor);
      AF_HIGH("Adjusted Touch ROI (x %f y %f dx %f dy %f) "
        "pdaf_sw_window_mode %d num HXV %dX%d",
        touch_info.pdaf_config.fixed_grid_window_configure.af_fix_window.pdaf_address_start_hori,
        touch_info.pdaf_config.fixed_grid_window_configure.af_fix_window.pdaf_address_start_ver,
        touch_info.pdaf_config.fixed_grid_window_configure.af_fix_window.pdaf_address_end_hori,
        touch_info.pdaf_config.fixed_grid_window_configure.af_fix_window.pdaf_address_end_ver,
        touch_info.pdaf_config.pdaf_sw_window_mode,
        touch_info.pdaf_config.fixed_grid_window_configure.window_number_hori,
        touch_info.pdaf_config.fixed_grid_window_configure.window_number_ver);

    }
  } else { // None touch ROI
    touch_info.roi_type = AF_ROI_TYPE_GENERAL;
    AF_HIGH("Default ROI");
  }

  af_port_pd_zero_touch_lag_update(af_port,
    AF_PD_TOUCH_ROI_TYPE, &touch_info);

  /* Update parameter message to be sent */
  set_parm->type = AF_SET_PARAM_ROI;
  memcpy(&set_parm->u.af_roi_info, &temp_roi, sizeof(af_roi_info_t));

  return TRUE;
} /* af_port_handle_set_roi_evt */

/** af_port_handle_set_sensor_roi_evt:
 *   @af_port:   private AF port structure
 *   @set_parm:  a message to populate
 *   @roi_input: new ROI information
 *
 * Handle set parameter event to set AF ROI in CAMIF context.
 *
 * Return TRUE on if roi has changed, FALSE if no change
 **/
static boolean af_port_handle_set_sensor_roi_evt(
  af_port_private_t *af_port,
  af_set_parameter_t *set_parm,
  af_roi_info_t *roi_input) {
  (void) af_port;
  AF_LOW("frm_id: %d num_roi: %d",
    roi_input->frm_id, roi_input->num_roi);

  /* Update parameter message to be sent */
  set_parm->type = AF_SET_PARAM_ROI;
  memcpy(&set_parm->u.af_roi_info, roi_input, sizeof(af_roi_info_t));

  AF_HIGH("update roi from HAL x =%d, y=%d, dx=%d, dy=%d Weight = %d",
    roi_input->roi[0].x,roi_input->roi[0].y,
    roi_input->roi[0].dx, roi_input->roi[0].dy, roi_input->weight[0]);

  // save the roi for HAL3, will using this roi when report back to HAL
  if(roi_input->weight[0] > 0) {
    af_port->bus.roi_msg.weight = roi_input->weight[0];
    af_port->bus.roi_msg.rect.left = roi_input->roi[0].x;
    af_port->bus.roi_msg.rect.top = roi_input->roi[0].y;
    af_port->bus.roi_msg.rect.width = roi_input->roi[0].dx;
    af_port->bus.roi_msg.rect.height = roi_input->roi[0].dy;
  }

  return TRUE;
} /* af_port_handle_set_sensor_roi_evt */


/** af_port_handle_set_focus_mode_evt:
 *   @af_port:  private AF port data
 *   @set_parm: a message to populate
 *   @mode:     focus mode to be set
 *
 * Handle set parameter event to set focus mode.
 *
 * Return TRUE if received a new mode, FALSE if received same AF mode.
 **/
static boolean af_port_handle_set_focus_mode_evt(mct_port_t *port,
  af_set_parameter_t * set_parm, int mode)
{
  boolean mode_change = FALSE;
  af_port_private_t *af_port =
    (af_port_private_t *)(port->port_private);

  if ((cam_focus_mode_type)mode != af_port->af_mode) {
    AF_LOW("AFCam-[mode -%d role-%d] Set Focus mode: %d old_mode: %d",
      af_port->dual_cam_info.mode,
      af_port->dual_cam_info.cam_role,
      mode, af_port->af_mode);
    mode_change = TRUE;
  }

  /* Update parameter message to be sent */
  set_parm->type = AF_SET_PARAM_FOCUS_MODE;
  af_port->video_caf_mode = FALSE;

  /* We need to translate Android focus_mode Macro to the one
     AF algorithm understands.*/
  switch (mode) {
  case CAM_FOCUS_MODE_AUTO: {
    set_parm->u.af_mode = AF_MODE_AUTO;
  }
    break;

  case CAM_FOCUS_MODE_INFINITY: {
    set_parm->u.af_mode = AF_MODE_INFINITY;
  }
    break;

  case CAM_FOCUS_MODE_MACRO: {
    set_parm->u.af_mode = AF_MODE_MACRO;
  }
    break;

  case CAM_FOCUS_MODE_CONTINOUS_VIDEO:
    af_port->video_caf_mode = TRUE;
  case CAM_FOCUS_MODE_CONTINOUS_PICTURE: {
    set_parm->u.af_mode = AF_MODE_CAF;
  }
    break;

  case CAM_FOCUS_MODE_MANUAL: {
    /* Additional  mode is added instead of re-using AF_MODE_MANUAL
     * in order to distinguish between HAL1 and HAL3 and maintain
     * 1 to 1 mapping between HAL focus mode and algo focus mode.
     **/
    set_parm->u.af_mode = AF_MODE_MANUAL_HAL1;
    af_port->manual.manual_af_state = FALSE;
  }
    break;

  case CAM_FOCUS_MODE_OFF: {
    set_parm->u.af_mode = AF_MODE_MANUAL;
    af_port->manual.manual_af_state = FALSE;
  }
    break;

  case CAM_FOCUS_MODE_FIXED:
  case CAM_FOCUS_MODE_EDOF:
  default: {
    set_parm->u.af_mode = AF_MODE_NOT_SUPPORTED;
  }
    break;
  }

  return mode_change;
} /* af_port_handle_set_focus_mode_evt */

/** af_port_handle_set_metering_mode_evt:
 *    @af_port:  private AF port data
 *    @set_parm: a message to populate
 *    @mode:     metering mode to be set
 *
 * Handle set parameter event to set the metering mode.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_set_metering_mode_evt(af_port_private_t * af_port,
  af_set_parameter_t * set_parm, int mode)
{
  (void) af_port;
  boolean rc = TRUE;
  /* Update parameter message to be sent */
  set_parm->type = AF_SET_PARAM_METERING_MODE;

  /* We need to translate Android metering mode Macro to the one
   * AF algorithm understands.
   **/
  switch (mode) {
  case CAM_FOCUS_ALGO_AUTO: {
    set_parm->u.af_metering_mode = AF_METER_AUTO;
  }
    break;

  case CAM_FOCUS_ALGO_SPOT: {
    set_parm->u.af_metering_mode = AF_METER_SPOT;
  }
    break;

  case CAM_FOCUS_ALGO_CENTER_WEIGHTED: {
    set_parm->u.af_metering_mode = AF_METER_CTR_WEIGHTED;
  }
    break;

  case CAM_FOCUS_ALGO_AVERAGE: {
    set_parm->u.af_metering_mode = AF_METER_AVERAGE;
  }
    break;

  default: {
    set_parm->u.af_metering_mode = AF_METER_AUTO;
    rc = FALSE;
  }
    break;
  }
  return rc;
} /* af_port_handle_set_metering_mode_evt */

/** af_port_handle_set_bestshot_mode_evt:
 *    @af_port:  AF port data structure
 *    @set_parm: a message to populate
 *    @mode:     scene mode to be set
 *
 * Handle set parameter event to set the bestshot mode.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_set_bestshot_mode_evt(
  af_port_private_t * af_port,
  af_set_parameter_t * set_parm,
  int mode)
{
  (void) af_port;
  boolean rc = TRUE;

  /* Update parameter message to be sent */
  set_parm->type = AF_SET_PARAM_BESTSHOT;

  /* We need to translate Android bestshot mode Macro to the one
   * AF algorithm understands.
   **/
  switch (mode) {
  case CAM_SCENE_MODE_OFF: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_OFF;
  }
    break;

  case CAM_SCENE_MODE_AUTO: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_AUTO;
  }
    break;

  case CAM_SCENE_MODE_LANDSCAPE: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_LANDSCAPE;
  }
    break;

  case CAM_SCENE_MODE_SNOW: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_SNOW;
  }
    break;

  case CAM_SCENE_MODE_BEACH: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_BEACH;
  }
    break;

  case CAM_SCENE_MODE_SUNSET: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_SUNSET;
  }
    break;

  case CAM_SCENE_MODE_NIGHT: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_NIGHT;
  }
    break;

  case CAM_SCENE_MODE_PORTRAIT: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_PORTRAIT;
  }
    break;

  case CAM_SCENE_MODE_BACKLIGHT: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_BACKLIGHT;
  }
    break;

  case CAM_SCENE_MODE_SPORTS: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_SPORTS;
  }
    break;

  case CAM_SCENE_MODE_ANTISHAKE: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_ANTISHAKE;
  }
    break;

  case CAM_SCENE_MODE_FLOWERS: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_FLOWERS;
  }
    break;

  case CAM_SCENE_MODE_CANDLELIGHT: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_CANDLELIGHT;
  }
    break;

  case CAM_SCENE_MODE_FIREWORKS: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_FIREWORKS;
  }
    break;

  case CAM_SCENE_MODE_PARTY: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_PARTY;
  }
    break;

  case CAM_SCENE_MODE_NIGHT_PORTRAIT: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_NIGHT_PORTRAIT;
  }
    break;

  case CAM_SCENE_MODE_THEATRE: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_THEATRE;
  }
    break;

  case CAM_SCENE_MODE_ACTION: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_ACTION;
  }
    break;

  case CAM_SCENE_MODE_AR: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_AR;
  }
    break;

  case CAM_SCENE_MODE_FACE_PRIORITY: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_FACE_PRIORITY;
  }
    break;

  case CAM_SCENE_MODE_BARCODE: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_BARCODE;
  }
    break;

  case CAM_SCENE_MODE_HDR: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_HDR;
  }
    break;

  default: {
    set_parm->u.af_bestshot_mode = AF_BESTSHOT_OFF;
    rc = FALSE;
  }
    break;
  }

  return rc;
} /* af_port_handle_set_bestshot_mode_evt */

/** af_port_handle_wait_for_aec_est
 * Tell the AF to wait for the AEC to complete estimation when LED is ON.
 *
 * @af_port: private AF port data
 *
 * @set_parm: a message to populate
 *
 * @wait: wait or don't wait
 *
 * Return: TRUE: Success  FALSE: Failure
 **/
static boolean af_port_handle_wait_for_aec_est(
  af_port_private_t * af_port,
  af_set_parameter_t * set_parm,
  boolean wait)
{
  boolean rc = TRUE;

  /* Update parameter message to be sent */
  set_parm->type = AF_SET_PARAM_WAIT_FOR_AEC_EST;
  set_parm->u.af_wait_for_aec_est = wait;

  af_port->af_led_assist = wait;

  return rc;
} /* af_port_handle_wait_for_aec_est */


/** af_port_handle_ez_enable_af_event
 * handle eztuing enable call from application.
 *
 * @af_port: private AF port data
 *
 * @set_parm: a message to populate
 *
 * @enable: enable or unenable
 *
 * Return: TRUE: Success  FALSE: Failure
 **/
static boolean af_port_handle_ez_enable_af_event(
  af_port_private_t * af_port,
  af_set_parameter_t * set_parm,
  boolean enable)
{
  (void) af_port;
  boolean rc = TRUE;

  /* Update parameter message to be sent to alg */
  set_parm->type = AF_SET_PARAM_EZ_ENABLE;
  set_parm->u.af_ez_enable = enable;

  AF_LOW("enable/unenable (%d) AF!", enable);
  return rc;
} /* af_port_handle_ez_enable_af_event */

/** af_port_handle_do_af_event
 *    @af_port:  private port data
 *
 * handle autofocus call from application.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_do_af_event(af_port_private_t *af_port)
{
  boolean rc = TRUE;

  AF_LOW("E");

  /* Allocate memory to create AF message. we'll post it to AF thread. */
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_START,
      AF_SET_PARAM_START, af_port);
  if (af_msg == NULL) {
    AF_ERR("Not enough memory.");
    return FALSE;
  }

  /* Enqueue the message to the AF thread */
  rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  return rc;
} /* af_port_handle_do_af_event */

/** af_port_send_reset_caf_event
 * When LED assisted AF is running in CAF mode, reset internal CAF state machine
 * to make it start from scratch.
 *
 * @af_port: private port number
 *
 * Return: TRUE: Success  FALSE: Failure
 **/
static boolean af_port_send_reset_caf_event(af_port_private_t * af_port)
{
  boolean rc = TRUE;


  /* Allocate memory to create AF message. we'll post it to AF thread. */
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
      AF_SET_PARAM_RESET_CAF, af_port);
  if (af_msg == NULL) {
    AF_ERR("Not enough memory.");
    return FALSE;
  }

  /* Enqueue the message to the AF thread */
  rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  return rc;
} /* af_port_handle_reset_caf_event */

/** af_port_handle_cancel_af_event:
 *    @af_port:  private port data
 *
 * handle autofocus cancel call from application.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_cancel_af_event(af_port_private_t *af_port)
{
  boolean rc = TRUE;

  AF_LOW("E");

  /* Allocate memory to create AF message. we'll post it to AF thread. */
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_CANCEL,
      AF_SET_PARAM_CANCEL_FOCUS, af_port);
  if (af_msg == NULL) {
    AF_ERR("Not enough memory.");
    return FALSE;
  }

  /* Enqueue the message to the AF thread */
  rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  return rc;
} /* af_port_handle_cancel_af_event */

/* return hyper focus distance value in mm */
static float af_port_get_hyper_focal(af_port_private_t *af_port)
{
  float coc, hyperD;

  af_actuator_info_t *actr_info = &(af_port->cam_info.actuator_info);

  float f_len = actr_info->focal_length;
  float f_num = actr_info->af_f_num;
  float f_pix = actr_info->af_f_pix;

  coc = 2.0 * f_pix; // Circle of confusion diameter limit (microns)
  hyperD = f_len * f_len / (f_num * coc) * 1000.0; // Hyperfocal distance (mm)
  AF_LOW("coc: %f, hyperD: %f", coc, hyperD);

  return hyperD;
}

/* Using current lens pos to calculate the optimal focus distance in m */
static float af_port_get_optimal_focus_distance(af_port_private_t *af_port, int32_t cur_pos)
{
  float opt_focus_dist = 0;
  float hyperD = 0;

  af_algo_tune_parms_adapter_t *fptr = &af_port->fd_prio_data.tuning_info;
  af_tuning_single_adapter_t* af_single = &fptr->af_algo.af_single;
  uint16_t focus_index[SINGLE_MAX_IDX];
  int32_t comp = 0,index_check = 0;

  distance_entry distanceMap[SINGLE_MAX_IDX];
  memcpy (&distanceMap, &global_map, sizeof(global_map));

  // apply the lens sag to the index map
  memcpy(&focus_index, af_single->index, sizeof(af_single->index));

  comp = af_port_get_lens_sag_compensation(af_port,
           af_port->gravity_vector_data_applied);
  AF_LOW("comp =%d", comp);
  if ((af_port->gravity_data_ready == TRUE) &&
     fptr->af_algo.lens_sag_comp.enable) {
    int32_t i = 0;
    for (i = 0 ; i< SINGLE_MAX_IDX; i++) {
      index_check = focus_index[i] + comp;
      if (index_check <= af_single->index[SINGLE_NEAR_LIMIT_IDX]) {
        index_check = af_single->index[SINGLE_NEAR_LIMIT_IDX];
      } else if (index_check >= af_single->index[SINGLE_INF_LIMIT_IDX]) {
        index_check = af_single->index[SINGLE_INF_LIMIT_IDX];
      }
      focus_index[i] = index_check;
    }
  }
  hyperD = af_port_get_hyper_focal(af_port);
  distanceMap[SINGLE_HYP_F_IDX].distance = (int32_t)hyperD / 10;

  int32_t i = 0;
  if (cur_pos > focus_index[SINGLE_HYP_F_IDX]) {
    // infinite
    opt_focus_dist = AF_PORT_MAX_FOCUS_DISTANCE;
  } else {
    if (cur_pos < focus_index[SINGLE_7CM_IDX]) {
      cur_pos = focus_index[SINGLE_7CM_IDX];
    }
    for (i = SINGLE_120CM_IDX; i >= SINGLE_7CM_IDX ; i--) {
      if (cur_pos >= focus_index[i]) {
        // interpolate the distance using the distance map
        if (focus_index[i + 1] != focus_index[i]) {
          opt_focus_dist = distanceMap[i].distance +
            1.0 * (cur_pos - focus_index[i]) * (distanceMap[i + 1].distance - distanceMap[i].distance) / (focus_index[i + 1] - focus_index[i]);
          opt_focus_dist /= 100;
        } else {
          // in case of compensation applied, focus_index[i] == focus_index[i + 1]
          opt_focus_dist = (1.0 * distanceMap[i + 1].distance) / 100;
        }
        break;
      }
    }
  }

  AF_LOW("idx %d value %d cur_pos %d focus_dist %f", i,
         focus_index[i], cur_pos, opt_focus_dist);
  return opt_focus_dist;
}

/* Use the focus distance info to calculate the near/far field of depth */
static void af_port_get_near_far_field_focus_distance(af_port_private_t *af_port,
    cam_focus_distances_info_t *f_info)
{
  if (af_port == NULL || f_info == NULL) {
    return;
  }

  float opt_focus_dist, near_focus_dist, far_focus_dist;
  float hyperD;
  af_actuator_info_t *actr_info = &af_port->cam_info.actuator_info;

  if (!actr_info || !actr_info->focal_length || !actr_info->af_f_num ||
    !actr_info->af_f_pix) {
    /* There is no support to check focus distance is supported for this
     * sensor or not. so returning fixed focus distances to pass the test */
    f_info->focus_distance[FOCUS_DISTANCE_NEAR_INDEX] = 0.10;
    f_info->focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX] = 0.15;
    f_info->focus_distance[FOCUS_DISTANCE_FAR_INDEX] = 0.17;
    AF_ERR("NO AVAILABLE LENS INFO FOR NEAR/FAR FIELD OF DEPTH\n");
    return;
  }

  float f_len = actr_info->focal_length; // in mm

  AF_LOW("f_length %f f_num=%f f_pix=%f",
     actr_info->focal_length, actr_info->af_f_num,
    actr_info->af_f_pix);

  hyperD = af_port_get_hyper_focal(af_port);// in mm

  opt_focus_dist = f_info->focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX] * 1000;
  AF_LOW("distance %f hyperD %f", opt_focus_dist, hyperD);

  near_focus_dist = (hyperD * opt_focus_dist) /
    (hyperD + (opt_focus_dist - f_len));

  far_focus_dist = (hyperD * opt_focus_dist) /
    (hyperD - (opt_focus_dist - f_len));

  if (near_focus_dist == 0 || opt_focus_dist == 0 || far_focus_dist == 0) {
    AF_HIGH("zero focus distance value FD %f NF %f FF %f!!!",
                opt_focus_dist, near_focus_dist, far_focus_dist);
  } else {
    // return the values in reciprocal of distance
    f_info->focus_distance[FOCUS_DISTANCE_NEAR_INDEX] =
      1000 / near_focus_dist;  /* in meters */

    f_info->focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX] =
      1000 / opt_focus_dist;  /* in meters */

    f_info->focus_distance[FOCUS_DISTANCE_FAR_INDEX] =
      1000 / far_focus_dist;  /* in meters */
  }

  AF_LOW("FD %f, NF %f, FF %f",
    opt_focus_dist / 1000, near_focus_dist / 1000,
    far_focus_dist / 1000);

  return;
}

/*
 * Use the focus_distance to reverse calculate the logic index for sensor
 */
static boolean af_port_handle_set_focus_distance(mct_port_t *port, float focus_distance)
{

  if (port == NULL)
    return FALSE;

  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  af_tuning_lens_sag_comp_adapter_t *tuning_ptr =
    &af_port->fd_prio_data.tuning_info.af_algo.lens_sag_comp;

  if (af_port->af_mode != CAM_FOCUS_MODE_OFF && af_port->af_mode
       != CAM_FOCUS_MODE_MANUAL) {
    AF_HIGH("not in CAM_FOCUS_MODE_OFF(Manual) mode, ignore the settings");
    return FALSE;
  }

  af_tuning_single_adapter_t* af_single =
    &af_port->fd_prio_data.tuning_info.af_algo.af_single;
  int32_t i = 0;
  int32_t index = 0;
  uint16_t focus_index[SINGLE_MAX_IDX];
  int comp=0,index_check=0;

  distance_entry distanceMap[SINGLE_MAX_IDX];
  memcpy (&distanceMap, &global_map, sizeof(global_map));

  memcpy(&focus_index,af_single->index,sizeof(af_single->index));
  comp = af_port_get_lens_sag_compensation(af_port,af_port->gravity_vector_data);
  if( (af_port->gravity_data_ready == TRUE) && tuning_ptr->enable ){
      for (i =0 ; i< SINGLE_MAX_IDX ;i++ ){
       index_check = focus_index[i] + comp;
     if( index_check <= af_single->index[SINGLE_NEAR_LIMIT_IDX] ) {
           index_check = af_single->index[SINGLE_NEAR_LIMIT_IDX];
        }else if (index_check >=af_single->index[SINGLE_INF_LIMIT_IDX] ) {
          index_check = af_single->index[SINGLE_INF_LIMIT_IDX];
        }
    focus_index[i] = index_check;
      }
  }
  af_port->gravity_vector_data_applied = af_port->gravity_vector_data;

  distanceMap[SINGLE_HYP_F_IDX].distance =
    (int32_t)af_port_get_hyper_focal(af_port) / 10;

  float distance = focus_distance;
  if (distance == 0) {
    /* focus to infinity */
    index = focus_index[SINGLE_HYP_F_IDX];
  } else if (distance > 0) {
    af_port->manual.u.af_manual_diopter = distance;
    distance = (1 / distance) * 100; // to cm
    int32_t section_h_steps = 0;
    int32_t section_l_steps = 0;
    int32_t section_h_distance = 0;
    int32_t section_l_distance = 0;

    /* interpolate the index from 7cm to infinity
     * use 60 - 120 cm as standard to interpolated the distance
     * > 120cm */
    int32_t j = 0;
    boolean in_range = FALSE;
    for (j = SINGLE_120CM_IDX; j >= SINGLE_7CM_IDX; j--) {
      if (distance > distanceMap[j].distance) {
        section_h_steps = focus_index[j + 1];
        section_l_steps = focus_index[j];
        section_h_distance = distanceMap[j + 1].distance;
        section_l_distance = distanceMap[j].distance;
        in_range = TRUE;
        break;
      }
    }

    /* we found the distance fall in 7cm ~ infinity */
    if (in_range) {
      index = section_l_steps + (section_h_steps - section_l_steps) *
        (distance - section_l_distance) /
        (section_h_distance - section_l_distance) + 0.5;
    } else {
      /* if the focus distance < 7cm */
      if (distance < distanceMap[SINGLE_7CM_IDX].distance) {
        index = focus_index[SINGLE_7CM_IDX];
      }
    }

    /* make sure the index is in the range */
    if (index < focus_index[SINGLE_NEAR_LIMIT_IDX])
      index = focus_index[SINGLE_NEAR_LIMIT_IDX];
    if (index > focus_index[SINGLE_INF_LIMIT_IDX])
      index = focus_index[SINGLE_INF_LIMIT_IDX];

    af_port->diopter = focus_distance;
    af_port->scale_ratio = ((index - af_single->index[SINGLE_NEAR_LIMIT_IDX])/
      (af_single->index[SINGLE_INF_LIMIT_IDX] -
       af_single->index[SINGLE_NEAR_LIMIT_IDX]))
      * 100 + 0.5;
  } else {
    /* error case */
    AF_ERR("ERROR focus distance is negative %f", focus_distance);
    return FALSE;
  }
  AF_LOW("focus_distance=%f, comp=%d, output index %d",
    focus_distance, comp, index);

  /* ok now we have the index, let's move the Lens */
  if (index - af_port->af_focus_pos) {
    int32_t steps = index - af_port->af_focus_pos;

    af_output_data_t output;
    memset(&output, 0, sizeof(output));

    output.type |= AF_OUTPUT_MOVE_LENS;
    output.move_lens.move_lens = TRUE;
    output.move_lens.direction = steps > 0 ? AF_MOVE_FAR : AF_MOVE_NEAR;

    if (steps < 0)
      steps = -steps;
    output.move_lens.num_of_steps = steps;
    output.move_lens.use_dac_value = FALSE;
    output.move_lens.cur_pos = af_port->af_focus_pos;

    af_port->is_lens_moving_manual = 2; // Set max_pipeline_frame_applying_delay
    af_port_send_move_lens_cmd(&output, port);
  } else {
    /* same postion, we just do nothing */
  }

  return TRUE;
}

/* index to diopter and pos ratio */
static uint32_t af_port_convert_index_to_scale(af_port_private_t *af_port,
  int32_t index)
{
  /* get near end and far end dac values from tuning table */
  af_tuning_single_adapter_t* af_single =
    &af_port->fd_prio_data.tuning_info.af_algo.af_single;

  unsigned short near_limit,far_limit;
  near_limit = af_single->index[SINGLE_NEAR_LIMIT_IDX];
  far_limit  = af_single->index[SINGLE_INF_LIMIT_IDX];
  return (uint32_t ) ((index - near_limit)
    / (float)(far_limit - near_limit) * 100 +0.5);
}

static float af_port_convert_index_to_diopter(af_port_private_t *af_port,
  int index)
{
  float diopter = 0;
  af_tuning_single_adapter_t* af_single =
    &af_port->fd_prio_data.tuning_info.af_algo.af_single;

  distance_entry distanceMap[SINGLE_MAX_IDX];
  memcpy (&distanceMap, &global_map, sizeof(global_map));

  int cur_pos = index;
  if(cur_pos >= af_single->index[SINGLE_INF_LIMIT_IDX] ){
    diopter = 0.0;
    return diopter;
  } else {
    int32_t section_h_steps = 0;
    int32_t section_l_steps = 0;
    int32_t section_h_distance = 0;
    int32_t section_l_distance = 0;
    int32_t j=0;
    boolean in_range = FALSE;
    /* if the index is greater than 120 cm use
          index of 60 cm &120 cm for extrapolation*/
    for (j = SINGLE_120CM_IDX - 1; j >= SINGLE_7CM_IDX; j--) {
      if (cur_pos >= af_single->index[j]) {
        if(distanceMap[j].index_enum + 1 >= SINGLE_MAX_IDX) {
          AF_ERR(" index out of range");
          return 0.0;
        }

        section_h_steps = af_single->index[distanceMap[j].index_enum + 1];
        section_l_steps = af_single->index[distanceMap[j].index_enum];
        section_h_distance = distanceMap[j + 1].distance;
        section_l_distance = distanceMap[j].distance;
        in_range = TRUE;
        break;
      }
    }
    if (in_range) {
      diopter = section_l_distance + (section_h_distance - section_l_distance) *
        ((float)cur_pos - section_l_steps) /
        (section_h_steps - section_l_steps);
    } else {
    /* if the focus distance < 7cm */
      if (cur_pos < af_single->index[SINGLE_7CM_IDX]) {
      diopter = distanceMap[SINGLE_7CM_IDX].distance;
      }
    }
  }
/* diopter is inverse of distance */
  diopter = 100.0/diopter;
  return diopter;
}

/** af_port_handle_set_focus_manual_pos_evt:
 *   @af_port:  private AF port data
 *   @pos:      manual focus pos to be set
 *
 * Handle set parameter event to set focus mode.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_set_focus_manual_pos_evt(
  mct_port_t * port, af_input_manual_focus_t * manual_pos_info) {

  if (!manual_pos_info) {
    return FALSE;
  }

  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);

  if (af_port->af_mode != CAM_FOCUS_MODE_OFF &&
      af_port->af_mode != CAM_FOCUS_MODE_MANUAL) {
    AF_HIGH("not in CAM_FOCUS_MODE_OFF(Manual) mode,\
           ignore the settings");
    return FALSE;
  }

  af_tuning_single_adapter_t* af_single =
    &af_port->fd_prio_data.tuning_info.af_algo.af_single;

  boolean rc = TRUE;
  /*All modes except diopter; use ratio for log*/
  if(manual_pos_info->flag == AF_MANUAL_FOCUS_MODE_POS_RATIO) {
    float ratio = (manual_pos_info->u.af_manual_lens_position_ratio* 1.0) / 100;
    af_tuning_single_adapter_t* af_single =
      &af_port->fd_prio_data.tuning_info.af_algo.af_single;
    af_tuning_lens_sag_comp_adapter_t *tuning_ptr =
    &af_port->fd_prio_data.tuning_info.af_algo.lens_sag_comp;
    int32_t i = 0;
    uint16_t focus_index[SINGLE_MAX_IDX];
    int comp=0,index_check=0;
    memcpy(&focus_index,af_single->index,sizeof(af_single->index));

    comp = af_port_get_lens_sag_compensation(af_port,af_port->gravity_vector_data);
      AF_LOW("comp =%d",comp);

    if( (af_port->gravity_data_ready == TRUE) && tuning_ptr->enable ){
      for (i =0 ; i< SINGLE_MAX_IDX ;i++ ){
       index_check = focus_index[i] + comp;
     if( index_check <= af_single->index[SINGLE_NEAR_LIMIT_IDX] ) {
           index_check = af_single->index[SINGLE_NEAR_LIMIT_IDX];
        }else if (index_check >=af_single->index[SINGLE_INF_LIMIT_IDX] ) {
          index_check = af_single->index[SINGLE_INF_LIMIT_IDX];
        }
      focus_index[i] = index_check;
      }
    }
    int index = ((focus_index[SINGLE_INF_LIMIT_IDX] -
      focus_index[SINGLE_NEAR_LIMIT_IDX]) * ratio);
    /* Calculate diopter */
    af_port->diopter = af_port_convert_index_to_diopter(af_port, index);
    af_port->scale_ratio = manual_pos_info->u.af_manual_lens_position_ratio;

    /* ok now we have the index, let's move the Lens */
    if (index - af_port->af_focus_pos) {
      int32_t steps = index - af_port->af_focus_pos;

      af_output_data_t output;
      memset(&output, 0, sizeof(output));

      output.type |= AF_OUTPUT_MOVE_LENS;
      output.move_lens.move_lens = TRUE;
      output.move_lens.direction = steps > 0 ? AF_MOVE_FAR : AF_MOVE_NEAR;

      if (steps < 0)
        steps = -steps;
      output.move_lens.num_of_steps = steps;
      output.move_lens.use_dac_value = FALSE;
      output.move_lens.cur_pos = af_port->af_focus_pos;

      af_port_send_move_lens_cmd(&output, port);
      af_port_handle_current_pos_update_evt(af_port,af_port->af_focus_pos);
    } else {
      /* same postion, we just do nothing */
    }

  } else if (manual_pos_info->flag == AF_MANUAL_FOCUS_MODE_DIOPTER){
    /* Use same function as HAL 3 */
    af_port_handle_set_focus_distance (port, manual_pos_info->u.af_manual_diopter);
  }
  /* Update parameter message to be sent */

  return rc;
}

/** af_port_manual_lens_sag:
 *    @af_port: private port data
 *    @mod_evt: module event structure
 *
 * Update gravity data.
 *
 * Return void.
 **/
static void af_port_manual_lens_sag(mct_port_t *port){

  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  int comp = 0, actual_cur_pos = 0;
  af_tuning_single_adapter_t* af_single =
    &af_port->fd_prio_data.tuning_info.af_algo.af_single;
  uint16_t focus_index[SINGLE_MAX_IDX];
  int i =0, index_check=0,final_pos=0;

  comp = af_port_get_lens_sag_compensation(af_port,
    af_port->gravity_vector_data_applied);

  switch(af_port->af_mode) {
    case CAM_FOCUS_MODE_MACRO:
    case CAM_FOCUS_MODE_AUTO: {
      actual_cur_pos = af_port->af_focus_pos - comp + af_port->over_flow_data;
     }
     break;

    case CAM_FOCUS_MODE_INFINITY:{
      actual_cur_pos = af_single->index[SINGLE_HYP_F_IDX];
     }
     break;
     default: {
      AF_LOW("Default. no action!");
     }
     break;
   }

/* make sure the index is in the range */
   if (actual_cur_pos < af_single->index[SINGLE_NEAR_LIMIT_IDX]){
     actual_cur_pos = af_single->index[SINGLE_NEAR_LIMIT_IDX];
   }else if (actual_cur_pos > af_single->index[SINGLE_INF_LIMIT_IDX]){
     actual_cur_pos = af_single->index[SINGLE_INF_LIMIT_IDX];
   }
  AF_LOW("actual_cur_pos = %d, af_focus_pos=%d ,comp_applied=%d",
    actual_cur_pos,af_port->af_focus_pos,comp);

  memcpy(&focus_index,af_single->index,sizeof(af_single->index));
  comp = af_port_get_lens_sag_compensation(af_port,af_port->gravity_vector_data);
  for (i =0 ; i< SINGLE_MAX_IDX ;i++ ){
    index_check = focus_index[i] + comp;
    if( index_check <= af_single->index[SINGLE_NEAR_LIMIT_IDX] ) {
     index_check = af_single->index[SINGLE_NEAR_LIMIT_IDX];
    }else if (index_check >=af_single->index[SINGLE_INF_LIMIT_IDX] ) {
     index_check= af_single->index[SINGLE_INF_LIMIT_IDX];
    }
    focus_index[i] = index_check;
   }
  af_port->gravity_vector_data_applied = af_port->gravity_vector_data;
  af_port->over_flow_data = 0;
  final_pos = actual_cur_pos + comp;
            /* make sure the index is in the range */
  if (final_pos < focus_index[SINGLE_NEAR_LIMIT_IDX]){
    af_port->over_flow_data = actual_cur_pos + comp -
    focus_index[SINGLE_NEAR_LIMIT_IDX];
    final_pos = focus_index[SINGLE_NEAR_LIMIT_IDX];
  }else if (final_pos > focus_index[SINGLE_INF_LIMIT_IDX]){
    af_port->over_flow_data = actual_cur_pos + comp -
    focus_index[SINGLE_INF_LIMIT_IDX];
    final_pos = focus_index[SINGLE_INF_LIMIT_IDX];
  }

  int steps = 0;
  steps = final_pos - af_port->af_focus_pos;
  af_output_data_t output;
  memset(&output, 0, sizeof(output));

  AF_LOW("Final_pos = %d,steps=%d,comp=%d,gravity_vector_data=%f",
    final_pos,steps,comp,af_port->gravity_vector_data);

  output.type |= AF_OUTPUT_MOVE_LENS;
  output.move_lens.move_lens = TRUE;
  output.move_lens.direction = steps > 0 ? AF_MOVE_FAR : AF_MOVE_NEAR;

  if (steps < 0)
    steps = -steps;
  output.move_lens.num_of_steps = steps;
  output.move_lens.use_dac_value = FALSE;
  output.move_lens.cur_pos = af_port->af_focus_pos;

  af_port_send_move_lens_cmd(&output, port);
  af_port_handle_current_pos_update_evt(af_port,af_port->af_focus_pos);

}

/** af_port_handle_set_parm_event
 * Handle AF related set parameter calls from upper layer.
 *
 * Handle AF related set parameter calls from upper layer.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_set_parm_event(mct_port_t *port,
  af_set_parameter_t * parm)
{
  boolean            rc = TRUE;
  boolean            sent = FALSE;
  af_set_parameter_t *set_parm;
  af_port_private_t  *af_port = (af_port_private_t *)(port->port_private);

  if (!parm || !af_port) {
    AF_ERR("Invalid parameters!");
    return FALSE;
  }

  AF_LOW("Handle set param event of type: %d", parm->type);

  /* Allocate memory to create AF message. we'll post it to AF thread. */
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
      parm->type, af_port);
  if (af_msg == NULL) {
    AF_ERR("Memory allocation failure!");
    return FALSE;
  }
  /* populate af message to post to thread */
  set_parm = &af_msg->u.af_set_parm;

  switch (parm->type) {
  case AF_SET_PARAM_ROI: {
    rc = af_port_handle_set_roi_evt(af_port, set_parm, &parm->u.af_roi_info);
  }
    break;

  case AF_SET_PARAM_SENSOR_ROI: {
    boolean is_new_roi = FALSE;
    is_new_roi = af_port_handle_set_sensor_roi_evt(af_port, set_parm,
      &parm->u.af_roi_info);
    sent = is_new_roi ? FALSE : TRUE;
  }
    break;

  case AF_SET_PARAM_FOCUS_MODE: {
    boolean is_mode_changed = FALSE;
    rc = TRUE;
    is_mode_changed = af_port_handle_set_focus_mode_evt(port, set_parm,
      (int)parm->u.af_mode);
    if (is_mode_changed == FALSE) {
      sent = TRUE; /* No need to send same mode over and over again */
    } else {
      af_msg->sync_flag = TRUE;
    }
  }
    break;

  case AF_SET_PARAM_METERING_MODE: {
    rc = af_port_handle_set_metering_mode_evt(af_port, set_parm,
      (int)parm->u.af_metering_mode);
  }
    break;

  case AF_SET_PARAM_BESTSHOT: {
    rc = af_port_handle_set_bestshot_mode_evt(af_port, set_parm,
      (int)parm->u.af_bestshot_mode);
  }
    break;

  case AF_SET_PARAM_SUPER_EVT: {
    /* Update parameter message to be sent */
    set_parm->type = parm->type;
    set_parm->u.af_set_parm_id = parm->u.af_set_parm_id;
  }
    break;

  case AF_SET_PARAM_META_MODE: {
  /* Update parameter message to be sent */
    set_parm->type = parm->type;
    set_parm->u.af_set_meta_mode = parm->u.af_set_meta_mode;
  }
    break;

  case AF_SET_PARAM_LOCK_CAF: {
    rc = af_port_handle_lock_caf_event(af_port, set_parm,
      (int)parm->u.af_lock_caf);
  }
    break;

  case AF_SET_PARAM_STATS_DEBUG_MASK: {
    set_parm->type = parm->type;
    set_parm->u.stats_debug_mask = parm->u.stats_debug_mask;
  }
    break;

  case AF_SET_PARAM_PAAF: {
    set_parm->type = parm->type;
    set_parm->u.paaf_mode = parm->u.paaf_mode;
  }
    break;

  case AF_SET_PARAM_EZ_ENABLE: {
    rc = af_port_handle_ez_enable_af_event(af_port, set_parm,
      (int)parm->u.af_ez_enable);
  }
    break;

  case AF_SET_PARAM_WAIT_FOR_AEC_EST: {
    /* prioritize the massage in order to not skip stats
     *  after the AEC reports it is ready */
    af_msg->is_priority = TRUE;
    af_port_handle_wait_for_aec_est(af_port, set_parm,
      (int)parm->u.af_wait_for_aec_est);
  }
    break;
  case AF_SET_PARAM_IDLE_TRIGGER: { //HAL3
    /* Store trigger id */
    af_port->af_trans.trigger.af_trigger_id = parm->u.trigger.af_trigger_id;
    af_port->af_trans.trigger.af_trigger = parm->u.trigger.af_trigger;
  }
    break;
  case AF_SET_PARAM_START: { // HAL3
    /* Store trigger id */
    af_port->af_trans.trigger.af_trigger_id = parm->u.trigger.af_trigger_id;
    af_port->af_trans.trigger.af_trigger = parm->u.trigger.af_trigger;
    /* Make do_af call */
    /* If it's CAF mode we don't need to explicitly call auto-focus.
       We just check the current CAF status and based on that make decision. */
    AF_LOW("AFCam-[mode -%d role-%d] TRIGGER_START ID: %d, focus mode: %d, af_led_assist: %d",
      af_port->dual_cam_info.mode,
      af_port->dual_cam_info.cam_role,
      af_port->af_trans.trigger.af_trigger_id, af_port->af_mode, af_port->af_led_assist);
    if ((af_port->af_mode != CAM_FOCUS_MODE_CONTINOUS_PICTURE) &&
      (af_port->af_mode != CAM_FOCUS_MODE_CONTINOUS_VIDEO)) {
      rc = af_port_handle_do_af_event(af_port);
      if (rc) {
        sent = TRUE;
      }
    } else if (af_port->af_led_assist == TRUE &&
      af_port->af_trans.af_state != CAM_AF_STATE_FOCUSED_LOCKED &&
      af_port->af_trans.af_state != CAM_AF_STATE_NOT_FOCUSED_LOCKED) {
      /** We are in continuous mode, so fool the HAL it is scanning.
       *  It is actually active scan, but we fool the HAL to believe
       *  it is passive and it will wait until focusing is completed
       **/
      af_port->af_trans.af_state = CAM_AF_STATE_PASSIVE_SCAN;
      af_port->af_status = AF_STATUS_FOCUSING;
      AF_LOW("Replace current AF state with CAM_AF_STATE_PASSIVE_SCAN");
      rc = af_port_send_reset_caf_event(af_port);
    }

    /* Update AF transition */
    if (rc) {
      sent = TRUE;
      af_port_update_af_state(port, AF_PORT_TRANS_CAUSE_TRIGGER);
    }
  }
    break;

  case AF_SET_PARAM_CANCEL_FOCUS: {// HAL3
    /* Store trigger id */
    af_port->af_trans.trigger.af_trigger_id = parm->u.trigger.af_trigger_id;
    af_port->af_trans.trigger.af_trigger = parm->u.trigger.af_trigger;
    AF_LOW("AFCam-[mode -%d role-%d] TRIGGER_CANCEL: ID: %d",
      af_port->dual_cam_info.mode,
      af_port->dual_cam_info.cam_role,
      af_port->af_trans.trigger.af_trigger_id);
    af_port->af_trigger_called = FALSE;
    /* Do not need to do cancel if not locked or scanning */
    if (af_port->af_trans.af_state != CAM_AF_STATE_PASSIVE_FOCUSED &&
      af_port->af_trans.af_state != CAM_AF_STATE_PASSIVE_UNFOCUSED &&
      af_port->af_trans.af_state != CAM_AF_STATE_INACTIVE) {
      rc = af_port_handle_cancel_af_event(af_port);
      /* change AF state */
      if (rc) {
        sent = TRUE;
        af_port_update_af_state(port, AF_PORT_TRANS_CAUSE_CANCEL);
      }
    } else {
      /* if caf is locked, unlock it */
      if (TRUE == af_port->caf_locked) {
        af_port_lock_caf(af_port, FALSE);
      }
    }
  }
    break;

    /* Both HAL1 and HAL3 */
  case AF_SET_PARAM_FOCUS_MANUAL_POSITION: {
    memcpy (&af_port->manual, (af_input_manual_focus_t *)&parm->u.af_manual_focus_info,
      sizeof (af_input_manual_focus_t));
    rc = af_port_handle_set_focus_manual_pos_evt(port, &af_port->manual);
    af_port->manual.manual_af_state = TRUE;
  }
    break;

  case AF_SET_PARAM_HFR_MODE: {
    set_parm->type = parm->type;
    set_parm->u.hfr_mode = parm->u.hfr_mode;
  }
    break;

  case AF_SET_PARAM_FD_ENABLED: {
    set_parm->type = AF_SET_PARAM_FD_ENABLED;
    set_parm->u.fd_enabled = parm->u.fd_enabled;
  }
    break;

  case AF_SET_PARAM_SKIP_FINE_SCAN: {
      set_parm->type = parm->type;
      set_parm->u.skip_fine_scan = parm->u.skip_fine_scan;
  }
    break;

  default: {
    free(af_msg);
    af_msg = NULL;
    return FALSE;
  }
    break;
  }
  /* Enqueue the message to the AF thread */
  if (sent == FALSE) {
    int rc = 0;
    boolean free_sync_msg = af_msg->sync_flag;
    rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
    if (free_sync_msg) {
      free(af_msg);
    }
    af_msg = NULL;
  } else {
    free(af_msg);
    af_msg = NULL;
  }

  return TRUE;
} /* af_port_handle_set_parm_event */

/** af_port_handle_led_cal_event
 *    @port: private AF port data
 *    @enable: led calibration enable flag
 *
 * Return: TRUE on success, or FALSE on failure
 **/
static boolean af_port_handle_led_cal_event(mct_port_t *port,
  boolean enable)
{
  boolean rc = TRUE;
  af_port_private_t * af_port = (af_port_private_t *)port->port_private;

  /* Allocate memory to create AF message. we'll post it to AF thread. */
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
      AF_SET_PARAM_FOCUS_MODE, af_port);
  if (af_msg == NULL) {
    AF_ERR("Memory allocation failure!");
    return FALSE;
  }

  /* led calibration only enable in factory mode, so there is no need to
   * add disabled usecase to recovery previous focus mode as it won't be
   * called in normal camera usecase */
  if (enable) {
    af_port_handle_set_focus_mode_evt(port,
      &af_msg->u.af_set_parm, CAM_FOCUS_MODE_MANUAL);
    af_msg->sync_flag = TRUE;
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
    /* Set lens to hyperfocal */
    rc = af_port_handle_set_focus_distance(port, 0.0f);
  }
  free(af_msg);
  af_msg = NULL;
  return rc;
}

/** af_port_handle_set_allparm_event
 * Handle Q3A related set parameter calls from upper layer.
 *
 * Handle Q3A related set parameter calls from upper layer an translate it
 * to AF set parameter.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_set_allparm_event(mct_port_t *port,
  q3a_all_set_param_type * parm)
{
  boolean            rc = TRUE;
  af_set_parameter_t *set_parm;
  af_port_private_t * af_port = NULL;

  if (!parm || !port) {
    AF_ERR("Invalid parameters!");
    return FALSE;
  }

  af_port = (af_port_private_t*)port->port_private;
  AF_LOW("Handle set param event of type: %d", parm->type);

  /* Allocate memory to create AF message. we'll post it to AF thread. */
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
      parm->type, af_port);
  if (af_msg == NULL) {
    AF_ERR("Memory allocation failure!");
    return FALSE;
  }
  /* populate af message to post to thread */
  set_parm = &af_msg->u.af_set_parm;

  switch (parm->type) {
  case Q3A_ALL_SET_EZTUNE_RUNNIG: {
    af_msg->u.af_set_parm.type = AF_SET_PARAM_EZ_TUNE_RUNNING;
    af_msg->u.af_set_parm.u.ez_running = parm->u.ez_runnig;
  }
    break;

  case Q3A_ALL_SET_CONFIG_AE_SCAN_TEST: {
    af_msg->u.af_set_parm.type = AF_SET_PARAM_CONFIG_AE_SCAN_TEST;
    af_msg->u.af_set_parm.u.ae_scan_test_config = parm->u.ae_scan_test_config.test_config;
  }
  break;

  case Q3A_ALL_SET_LED_CAL: {
    boolean led_cal_enable = (boolean)parm->u.led_cal_enable;
    af_port_handle_led_cal_event(port, led_cal_enable);
  }
    break;

  default: {
    free(af_msg);
    af_msg = NULL;
    return FALSE;
  }
    break;
  }
  /* Enqueue the message to the AF thread */
  q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  return TRUE;
} /* af_port_handle_set_allparm_event */

/** af_port_handle_control_event:
 *    @af_port: private AF port data
 *    @mod_evt: module event received
 *
 * Handle control events received at AF port.
 *
 * Return void
 **/
static void af_port_handle_control_event(mct_port_t * port,
   mct_event_t * event) {
  mct_event_control_t *ctrl_evt =
    (mct_event_control_t *)&(event->u.ctrl_event);
  af_port_private_t *af_port =
    (af_port_private_t *)(port->port_private);

  /* Check if there's need for extended handling. */
  if (af_port->func_tbl.ext_handle_control_event) {
    stats_ext_return_type ret;
    ret = af_port->func_tbl.ext_handle_control_event(port, ctrl_evt);
    /* Check if this event has been completely handled. If not we'll
       process it further here. */
    if (ret == STATS_EXT_HANDLING_COMPLETE) {
      AF_LOW("Control event %d handled by extended functionality!",
        ctrl_evt->type);
      return;
    }
  }

  /* for front facing camera (fixed), AF won't be initialized, but we shall
     still send the af state to HAL on SOF and process intra link commands */
  if (!af_port->af_initialized &&
    ctrl_evt->type != MCT_EVENT_CONTROL_SOF &&
    ctrl_evt->type != MCT_EVENT_CONTROL_LINK_INTRA_SESSION &&
    ctrl_evt->type != MCT_EVENT_CONTROL_MASTER_INFO ) {
    AF_LOW("AF not supported yet! control evt=%d",ctrl_evt->type);
    return;
  }
  AF_LOW("Handle Control event of type: %d",
    ctrl_evt->type);
  switch (ctrl_evt->type) {
  case MCT_EVENT_CONTROL_SET_PARM: {
    stats_set_params_type *stat_parm =
      (stats_set_params_type *)ctrl_evt->control_event_data;
    if (stat_parm->param_type == STATS_SET_Q3A_PARAM) {
      q3a_set_params_type  *q3a_param = &(stat_parm->u.q3a_param);
      if (q3a_param->type == Q3A_SET_AF_PARAM) {
        af_port_handle_set_parm_event(port, &q3a_param->u.af_param);
      } else if (q3a_param->type == Q3A_ALL_SET_PARAM) {
        af_port_handle_set_allparm_event(port, &q3a_param->u.q3a_all_param);
      }
    }
    /* If it's common params shared by many modules */
    else if (stat_parm->param_type == STATS_SET_COMMON_PARAM) {
      stats_common_set_parameter_t *common_param =
        &(stat_parm->u.common_param);
      af_set_parameter_t af_param;
      if (common_param->type == COMMON_SET_PARAM_BESTSHOT) {
        af_param.type = AF_SET_PARAM_BESTSHOT;
        af_param.u.af_bestshot_mode = (af_bestshot_mode_type_t)common_param->u.bestshot_mode;
        af_port_handle_set_parm_event(port, &af_param);
      } else if (common_param->type == COMMON_SET_PARAM_STATS_DEBUG_MASK) {
        af_param.type = AF_SET_PARAM_STATS_DEBUG_MASK;
        af_port_handle_set_parm_event(port, &af_param);
      } else if (common_param->type == COMMON_SET_PARAM_SUPER_EVT) {
        af_param.type = AF_SET_PARAM_SUPER_EVT;
        af_param.u.af_set_parm_id = common_param->u.current_frame_id;
        af_port_handle_set_parm_event(port, &af_param);
      } else if (common_param->type == COMMON_SET_PARAM_META_MODE) {
        af_param.type = AF_SET_PARAM_META_MODE;
        af_param.u.af_set_meta_mode = common_param->u.meta_mode;
        af_port_handle_set_parm_event(port, &af_param);
      } else if (common_param->type == COMMON_SET_CAPTURE_INTENT) {
        af_port_handle_capture_intent(af_port, common_param->u.capture_type);
      } else if (common_param->type == COMMON_SET_CROP_REGION) {
        af_port_handle_set_crop_region_event(af_port,
          &common_param->u.crop_region);
      } else if (common_param->type == COMMON_SET_PARAM_HAL_VERSION) {
        af_port->hal_version = common_param->u.hal_version;
        AF_LOW("HAL Version: %d", af_port->hal_version);
      } else if (common_param->type == COMMON_SET_PARAM_STREAM_ON_OFF) {
        uint8_t camera_id = af_port->camera_id;
        AF_LOW("COMMON_SET_PARAM_STREAM_ON_OFF %d", common_param->u.stream_on);
        af_port->thread_data->thread_ctrl[camera_id].no_stats_mode = !common_param->u.stream_on;

        // stream off, need to flush existing stats
        // send a sync msg here to flush the stats & other msg
        if (!common_param->u.stream_on) {
          af_port_unlink_from_peer(port, event);
          q3a_thread_af_msg_t af_msg;
          memset(&af_msg, 0, sizeof(q3a_thread_af_msg_t));
          af_msg.type = MSG_AF_STATS_MODE;
          af_msg.camera_id = camera_id;
          af_msg.sync_flag = TRUE;
          q3a_af_thread_en_q_msg(af_port->thread_data, &af_msg);

          /* reset the saved the ROI to an invalid value */
          af_port->bus.roi_msg.rect.left = -1;
        }
      } else if (common_param->type == COMMON_SET_PARAM_FD) {
        af_port->fd_prio_data.fd_enabled = common_param->u.fd_enabled;
        /* Also update AF lib */
        af_param.type = AF_SET_PARAM_FD_ENABLED;
        af_param.u.fd_enabled = common_param->u.fd_enabled;
        af_port_handle_set_parm_event(port, &af_param);
      } else if(common_param->type == COMMON_SET_PARAM_FOV_COMP) {
        int force_mode = 0;
        AF_LOW("Enable/disable FOVC mode: %d",
          common_param->u.fovc_mode);
        /* Enable/Disable FOV compensation */
        /*Check if it has not been forcefully enabled using setprop. That case
          even if HAL wants to disable it, we won't */
        STATS_ENABLE_FOVC(force_mode);
        AF_INFO("Enable/disable FOVC compensation : %d FOVC force enable : %d",
          common_param->u.fovc_mode, force_mode);
        if (!force_mode) {
          af_port_set_fovc_enable(af_port, common_param->u.fovc_mode);
        }
      }
    }
  }
    break;

  case MCT_EVENT_CONTROL_DO_AF: {
    af_port_handle_do_af_event(af_port);
  }
    break;

  case MCT_EVENT_CONTROL_CANCEL_AF: {
    af_port_handle_cancel_af_event(af_port);
  }
    break;

  case MCT_EVENT_CONTROL_STREAMON: {
    af_port->reconfigure_ISP_pending = TRUE;
  }
    break;

  case MCT_EVENT_CONTROL_STREAMOFF: {
  }
    break;

  case MCT_EVENT_CONTROL_SOF: {
    af_port_handle_sof_event(port, ctrl_evt);
  }
    break;

  case MCT_EVENT_CONTROL_MASTER_INFO: {
     af_port_handle_role_switch(port,event);
   }
     break;

  case MCT_EVENT_CONTROL_LINK_INTRA_SESSION: {
    af_port_link_to_peer(port,event);
  }
    break;

  case MCT_EVENT_CONTROL_UNLINK_INTRA_SESSION: {
    af_port_unlink_from_peer(port,event);
  }
    break;

  case MCT_EVENT_CONTROL_HW_SLEEP:
  case MCT_EVENT_CONTROL_HW_WAKEUP: {
    af_port_update_LPM(port,
      (cam_dual_camera_perf_control_t*)event->u.ctrl_event.control_event_data);
  }
    break;

  default: {
  }
    break;
  }
} /* af_port_handle_control_event */

/** af_port_check_session
 *    @data1: port's existing identity;
 *    @data2: new identity to compare.
 *
 * Check if two session index in the identities are equal
 *
 * Return TRUE if two session index in the identities are equal, FALSE if not.
 **/
static boolean af_port_check_session(void *data1, void *data2)
{
  return (((*(unsigned int *)data1) & 0xFFFF0000) ==
    ((*(unsigned int *)data2) & 0xFFFF0000) ?
    TRUE : FALSE);
}

/** af_port_event:
 *    @port:  port of AF module to handle event
 *    @event: event to be handled
 *
 * af sink module's event processing function. Received events could be:
 * AEC/AWB/AF Bayer stats;
 * Gyro sensor stats;
 * Information request event from other module(s);
 * Information update event from other module(s);
 * It ONLY takes MCT_EVENT_DOWNSTREAM event.
 *
 * Return TRUE if the event is processed successfully, FALSE on failure.
 **/
static boolean af_port_event(mct_port_t * port, mct_event_t * event)
{
  boolean           rc = TRUE;
  af_port_private_t *private;

  /* sanity check */
  if (!port || !event) {
    AF_ERR("mct-event or port is NULL");
    return FALSE;
  }

  private = (af_port_private_t *)(port->port_private);
  if (!private){
    AF_ERR("NULL private port");
    return FALSE;
  }

  /* sanity check: ensure event is meant for port with same identity*/
  if ((private->reserved_id & 0xFFFF0000) != (event->identity & 0xFFFF0000)) {
    AF_ERR("Identity does not match");
    return FALSE;
  }

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_DOWNSTREAM: {
    switch (event->type) {
    case MCT_EVENT_MODULE_EVENT: {
      mct_event_module_t *mod_evt =
        (mct_event_module_t *)&(event->u.module_event);
      af_port_handle_module_event(port, mod_evt);
    }
      break;

    case MCT_EVENT_CONTROL_CMD: {
      af_port_handle_control_event(port,event);
    }
      break;

    default:
      break;
    }
  } /* case MCT_EVENT_DOWNSTREAM */
    break;

  case MCT_EVENT_UPSTREAM: {
    mct_port_t *peer = MCT_PORT_PEER(port);
    MCT_PORT_EVENT_FUNC(peer)(peer, event);
  }
    break;

  default: {
    rc = FALSE;
  }
    break;
  }
  return rc;
}

/** af_port_ext_link:
 *    @identity: session id + stream id
 *    @port:     af module's sink port
 *    @peer:     q3a module's sink port
 *
 * link AF port to Q3A port
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_ext_link(unsigned int identity,  mct_port_t *port,
  mct_port_t *peer)
{
  boolean           rc = FALSE;
  af_port_private_t *private;

  AF_LOW("Link AF port to q3a");
  /* af sink port's external peer is always q3a module's sink port */
  if (!port || !peer ||
    strcmp(MCT_OBJECT_NAME(port), "af_sink") ||
    strcmp(MCT_OBJECT_NAME(peer), "q3a_sink")) {
    AF_LOW("NULL parameters");
    return FALSE;
  }

  private = (af_port_private_t *)port->port_private;
  if (!private){
    AF_LOW("private port NULL");
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  switch (private->state) {
  case AF_PORT_STATE_RESERVED:
  /* Fall through, no break */
  case AF_PORT_STATE_UNLINKED:
  /* Fall through, no break */
  case AF_PORT_STATE_LINKED: {
    if ((private->reserved_id & 0xFFFF0000) != (identity & 0xFFFF0000)) {
      AF_LOW("AF Port Identity does not match!");
      break;
    }
  }
  /* Fall through, no break */
  case AF_PORT_STATE_CREATED: {
    rc = TRUE;
  }
    break;

  default: {
  }
    break;
  }
  if (rc == TRUE) {
    private->state = AF_PORT_STATE_LINKED;
    MCT_PORT_PEER(port) = peer;
    MCT_OBJECT_REFCOUNT(port) += 1;
  }
  MCT_OBJECT_UNLOCK(port);
  mct_port_add_child(identity, port);

  AF_LOW("rc = %d port_state: %d", rc,
    private->state);
  return rc;
}

/** af_port_ext_unlink:
 *    @identity: session id + stream id
 *    @port:     af module's sink port
 *    @peer:     q3a module's sink port
 *
 * Unlink AF port from Q3A port
 *
 * Return TRUE on success, FALSE on failure.
 **/
static void af_port_ext_unlink(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  af_port_private_t *private;

  if (!port || !peer || MCT_PORT_PEER(port) != peer) {
    return;
  }

  private = (af_port_private_t *)port->port_private;
  if (!private) {
    return;
  }

  MCT_OBJECT_LOCK(port);
  if (private->state == AF_PORT_STATE_LINKED &&
    (private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000)) {

    MCT_OBJECT_REFCOUNT(port) -= 1;
    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state = AF_PORT_STATE_UNLINKED;
    }
  }
  MCT_OBJECT_UNLOCK(port);
  mct_port_remove_child(identity, port);

  return;
}

/** af_port_set_caps
 *    @port: af module's sink port
 *    @caps: pointer to be set to point to the port's capabilities
 *
 * set caps of AF module's sink module
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_set_caps(mct_port_t *port, mct_port_caps_t *caps)
{
  if (strcmp(MCT_PORT_NAME(port), "af_sink")) {
    return FALSE;
  }

  port->caps = *caps;
  return TRUE;
}

/** af_port_check_caps_reserve
 *    @port:        af module's sink port
 *    @caps:        port's capabilities
 *    @stream_info: the stream info
 *
 *  AF sink port can ONLY be re-used by ONE session. If this port
 *  has been in use, AF module has to add an extra port to support
 *  any new session(via module_af_request_new_port).
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_check_caps_reserve(mct_port_t *port, void *caps,
  void *stream_nfo)
{
  boolean           rc = FALSE;
  mct_port_caps_t   *port_caps;
  af_port_private_t *private;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)stream_nfo;

  if (!port || !caps || !stream_info ||
    strcmp(MCT_OBJECT_NAME(port), "af_sink")) {
    AF_ERR("NULL info");
    return rc;
  }

  MCT_OBJECT_LOCK(port);

  port_caps = (mct_port_caps_t *)caps;
  if (port_caps->port_caps_type != MCT_PORT_CAPS_STATS) {
    AF_ERR("Wrong port caps type");
    rc = FALSE;
    goto reserve_done;
  }

  private = (af_port_private_t *)port->port_private;
  switch (private->state) {
  case AF_PORT_STATE_LINKED: {
    AF_LOW("AF Port state linked");
    if ((private->reserved_id & 0xFFFF0000) ==
      (stream_info->identity & 0xFFFF0000))
    rc = TRUE;
  }
    break;

  case AF_PORT_STATE_CREATED:
  case AF_PORT_STATE_UNRESERVED: {
    AF_LOW("AF Port state created/unreseved");
    private->reserved_id = stream_info->identity;
    private->stream_info = *stream_info;
    private->state       = AF_PORT_STATE_RESERVED;
    rc = TRUE;
  }
    break;

  case AF_PORT_STATE_RESERVED: {
    AF_LOW("AF Port state reserved");
    if ((private->reserved_id & 0xFFFF0000) ==
      (stream_info->identity & 0xFFFF0000))
    rc = TRUE;
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
}

/** af_port_check_caps_unreserve:
 *    @port:     af module's sink port
 *    @identity: session id + stream id
 *
 * Unreserves the AF port.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  af_port_private_t *private;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "af_sink")) {
    return FALSE;
  }

  private = (af_port_private_t *)port->port_private;
  if (!private) {
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  if (((private->state == AF_PORT_STATE_UNLINKED)   ||
    (private->state == AF_PORT_STATE_LINKED) ||
    (private->state == AF_PORT_STATE_RESERVED)) &&
    ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000))) {

    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state       = AF_PORT_STATE_UNRESERVED;
      private->reserved_id = (private->reserved_id & 0xFFFF0000);
    }
  }
  MCT_OBJECT_UNLOCK(port);

  return TRUE;
}

/** af_port_find_identity:
 *    @port:     af port to be checked
 *    @identity: identity to be compared with
 *
 * Compare af port's session info with identity
 *
 * Return TRUE if equal, FALSE if not.
 **/
boolean af_port_find_identity(mct_port_t *port, unsigned int identity)
{
  af_port_private_t *private;

  if ( !port || strcmp(MCT_OBJECT_NAME(port), "af_sink")) {
    return FALSE;
  }

  private = port->port_private;

  if (private) {
    return ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000) ?
      TRUE : FALSE);
  }

  return FALSE;
}

/** af_port_deinit
 *    @port: af module's sink port to be deinited
 *
 * deinit af module's port
 *
 * Return void
 **/
void af_port_deinit(mct_port_t *port)
{

  af_port_private_t *private;
  int ret;

  if (!port) {
    return;
  }

  if(strcmp(MCT_OBJECT_NAME(port), "af_sink")) {
    return;
  }

  private = (af_port_private_t *)port->port_private;
  if (!private) {
    return;
  }

  AF_HIGH("De-init: camera_id: %u, session_id: %u",
    private->camera_id, GET_SESSION_ID(private->reserved_id));
  ret = pthread_mutex_destroy(&private->focus_state_mutex);
  if (ret) {
    if (ret == EBUSY) {
      AF_ERR("Unable to destroy focus_state_mutex, lock is acquired");
    } else if (ret == EINVAL) {
      AF_ERR("Unable to destroy focus_state_mutex, lock is invalid");
    }
  }
  ret = pthread_mutex_destroy(&private->div_buf_ack.mutex);
  if (ret) {
    if (ret == EBUSY) {
      AF_ERR("Unable to destroy div_buf_ack.mutex, lock is acquired");
    } else if (ret == EINVAL) {
      AF_ERR("Unable to destroy div_buf_ack.mutex, lock is invalid");
    }
  }
  if (private->p_sw_stats) {
    free(private->p_sw_stats);
    private->p_sw_stats = NULL;
  }

  private->af_object.af_ops.deinit(private->af_object.af);
  if (private->func_tbl.ext_deinit) {
    private->func_tbl.ext_deinit(port);
  }
  if (FALSE == private->af_extension_use) {
    af_port_unload_function(private);
  } else {
    af_port_ext_unload_function(private);
  }
  fdprio_process(&private->fd_prio_data, FDPRIO_CMD_DEINIT);
  if (private->div_buf_ack.div_buf_ack_ctrl) {
    free(private->div_buf_ack.div_buf_ack_ctrl);
  }
  free(private);
  AF_LOW("X");
}

/** af_port_update_func_table:
 *    @ptr: pointer to internal af pointer object
 *
 * Update extendable function pointers.
 *
 * Return: TRUE on success
 **/
boolean af_port_update_func_table(af_port_private_t *private)
{
  private->func_tbl.ext_init = NULL;
  private->func_tbl.ext_deinit = NULL;
  private->func_tbl.ext_callback = NULL;
  private->func_tbl.ext_handle_module_event = NULL;
  private->func_tbl.ext_handle_control_event = NULL;
  return TRUE;
}

/** af_port_init:
 *    @port:       af's sink port to be initialized
 *    @session_id: session id to be set into af's sink port
 *
 *  af port initialization entry point. Because AF module/port is
 *  pure software object, defer af_port_init when session starts.
 *
 * Return TRUE on success, FALSE on failure.
 **/
boolean af_port_init(mct_port_t *port, unsigned int *session_id)
{
  boolean           rc = TRUE;
  mct_port_caps_t   caps;
  unsigned int      *session;
  mct_list_t        *list;
  af_port_private_t *private;

  AF_LOW("E");

  if (!port || strcmp(MCT_OBJECT_NAME(port), "af_sink")) {
    AF_ERR("Port name does not match");
    return FALSE;
  }

  private = (void *)calloc(1, sizeof(af_port_private_t));
  if (!private) {
    AF_ERR("Failure allocating memory for port data ");
    return FALSE;
  }

  private->p_sw_stats = (af_sw_stats_t *)malloc(sizeof(af_sw_stats_t));
  private->reserved_id = *session_id;
  private->state       = AF_PORT_STATE_CREATED;
  private->focusedAtLeastOnce = FALSE;
  private->af_initialized = FALSE;
  private->af_not_supported = TRUE;
  private->af_mode     = CAM_FOCUS_MODE_MAX;
  private->af_status   = AF_STATUS_INIT;
  private->manual.manual_af_state = FALSE;
  private->is_pdaf_lib_available = TRUE;
  port->port_private   = private;
  port->direction      = MCT_PORT_SINK;
  caps.port_caps_type  = MCT_PORT_CAPS_STATS;
  caps.u.stats.flag    = MCT_PORT_CAP_STATS_AF;
  private->focus_state_changed = FALSE;
  private->focal_length_ratio = 1.0;
  pthread_mutex_init(&private->focus_state_mutex, NULL);
  pthread_mutex_init(&private->div_buf_ack.mutex, NULL);
  memset(&private->af_trans, 0, sizeof(af_port_state_trans_t));
  private->af_trans.af_state = CAM_AF_STATE_INACTIVE;
  private->stats_caps = 0;

  private->bus.focus_mode = CAM_FOCUS_MODE_FIXED;
  private->bus.lens_state = CAM_AF_LENS_STATE_STATIONARY;
  private->tof_data.is_updated = FALSE;
  private->fovc_data.frame_skip_cnt = 0;
  private->is_lens_moving_manual = 0;


  private->pd_lib_early_result.pdaf_processing_result_early_int =
    PDAF_PDLIB_RESULT_PARTIAL;
  private->pd_lib_early_result.process_bottom_multi_window = FALSE;

  pd_zero_touch_lag_info_t touch_info;
  touch_info.is_it_first_frame = touch_info.is_it_first_touch = FALSE;
  // prev_frame_idivert_buf_idx is initilialized to -1 which means
  // this is the first frame. roi_type valid values is [0-2] so
  // the intila value is -1 to mention roi_type hasn't been updated yet.
  touch_info.prev_frame_idivert_buf_idx = touch_info.roi_type = -1;
  af_port_pd_zero_touch_lag_update(private,
    AF_PD_TOUCH_PREV_FRAME_BUF_IDX,
    &touch_info);
  af_port_pd_zero_touch_lag_update(private,
    AF_PD_TOUCH_ROI_TYPE, &touch_info);
  af_port_pd_zero_touch_lag_update(private,
    AF_IS_FIRST_FRAME_TOUCH, &touch_info);
  af_port_pd_zero_touch_lag_update(private,
    AF_IS_FIRST_TOUCH, &touch_info);

  /* Set default functions to keep clean & bug free code*/
  rc &= af_port_load_dummy_default_func(&private->af_object.af_ops);
  rc &= af_port_update_func_table(private);

  fdprio_process(&private->fd_prio_data, FDPRIO_CMD_INIT);

  /* this is sink port of af module */
  mct_port_set_event_func(port, af_port_event);
  mct_port_set_intra_event_func(port, af_port_intra_event);
  mct_port_set_ext_link_func(port, af_port_ext_link);
  mct_port_set_unlink_func(port, af_port_ext_unlink);
  mct_port_set_set_caps_func(port, af_port_set_caps);
  mct_port_set_check_caps_reserve_func(port, af_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, af_port_check_caps_unreserve);

  if (port->set_caps) {
    port->set_caps(port, &caps);
  }
  AF_HIGH("Init AF: session_id: %u", GET_SESSION_ID(private->reserved_id));

  AF_LOW("X");
  return rc;
}

/** af_port_update_roi_info:
 *    @port:       af's sink port to be initialized
 *    @af_out: the output from the algorithm
 *
 *  af port gets updated roi information from algorithm
 *  report this info for meta data
 *
 **/
static void af_port_update_roi_info(mct_port_t *port,
  af_output_data_t *af_out){
  /* Update parameter message to be sent */
  af_port_private_t  *af_port = (af_port_private_t *)(port->port_private);
  /* af_out->roi_info caontains the final ROI which is configured by ISP
     As this information is currently needed for real time bokeh feature.
     will be using this roi when reporting back as bus msg to meta data*/
  AF_HIGH("update roi to meta data x =%d, y=%d, dx=%d, dy=%d, weight=%d",
  af_out->roi_info.roi[0].x,af_out->roi_info.roi[0].y,
  af_out->roi_info.roi[0].dx, af_out->roi_info.roi[0].dy, af_out->roi_info.weight[0]);
  af_port->bus.default_roi.left   = af_out->roi_info.roi[0].x;
  af_port->bus.default_roi.top    = af_out->roi_info.roi[0].y;
  af_port->bus.default_roi.width  = af_out->roi_info.roi[0].dx;
  af_port->bus.default_roi.height = af_out->roi_info.roi[0].dy;
}/*af_port_update_roi_info*/

/*Start of AF Dual Camera Port handling*/

/* Summary of code changes:
 */

/** af_port_link_to_peer:
 *    @port: MCT port data
 *    @event: module event received
 *
 * Links to peer AF Port (Master/Slave)
 *
 * Return boolean
 **/
boolean af_port_link_to_peer(mct_port_t *port,
                               mct_event_t *event)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  uint32_t                               peer_identity = 0;
  int                                    syncOverride = 0;
  af_port_private_t  *private = (af_port_private_t *)(port->port_private);

  link_param = (cam_sync_related_sensors_event_info_t *)
    (event->u.ctrl_event.control_event_data);
  peer_identity = link_param->related_sensor_session_id;

  rc = stats_util_get_peer_port(event, peer_identity,this_port,
    &peer_port);

  if (rc == FALSE) {
    AF_ERR("FAIL to Get Peer Port");
    return rc;
  }
  private->dual_cam_info.cam_role = link_param->cam_role;
  private->dual_cam_info.mode = link_param->mode;
  private->dual_cam_info.sync_3a_mode = link_param->sync_3a_config.sync_mode_af;
  syncOverride = q3a_prop_get("persist.vendor.camera.af.sync","0");
  if(syncOverride == 1) {
    private->dual_cam_info.sync_3a_mode = CAM_3A_SYNC_FOLLOW;
  } else if(syncOverride == 2) {
    private->dual_cam_info.sync_3a_mode = CAM_3A_SYNC_NONE;
  }
  private->dual_cam_info.is_LPM_on = FALSE;
  private->dual_cam_info.intra_peer_id = peer_identity;
  private->dual_cam_info.need_wakeup_trigger = (link_param->hal_lpm_control == 0);
  af_port_handle_role_switch(port,NULL);
  MCT_PORT_INTRALINKFUNC(this_port)(peer_identity, this_port, peer_port);
  AF_HIGH("AFDualCam-[mode -%d role-%d 3aSync-%d AuxSync-%d AuxUpdate-%d] Linking to session %x Success",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role,
    private->dual_cam_info.sync_3a_mode,
    private->dual_cam_info.is_aux_sync_enabled,
    private->dual_cam_info.is_aux_update_enabled,
    peer_identity);
  return rc;
}

/** af_port_unlink_from_peer:
 *    @port: MCT port data
 *    @event: module event received
 *
 * UnLinks from peer AF Port (Master/Slave)
 *
 * Return boolean
 **/
boolean af_port_unlink_from_peer(mct_port_t *port,
                                     mct_event_t *event)
{
  af_port_private_t  *private = (af_port_private_t *)(port->port_private);
  if(private->dual_cam_info.intra_peer_id){
    AF_HIGH("AFDualCam-[mode -%d role-%d] Unlink started",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role);
    af_port_local_unlink(port,event);
    af_port_remote_unlink(port);
    AF_HIGH("AFDualCam-[mode -%d role-%d] Unlink complete",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role);
    af_port_reset_dual_cam_info(private);
  }
  return TRUE;
}

/** af_port_local_unlink:
 *    @port: MCT port data
 *    @event: module event received
 *
 * UnLinks from peer AF Port locally (Master/Slave)
 *
 * Return boolean
 **/
boolean af_port_local_unlink(mct_port_t *port,
                             mct_event_t *event)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  af_port_private_t                     *private = (af_port_private_t *)(port->port_private);
  uint32_t                               peer_identity = private->dual_cam_info.intra_peer_id;

  if (peer_identity) {
    rc = stats_util_get_peer_port(event, peer_identity, this_port, &peer_port);
    if (rc == TRUE) {
      MCT_PORT_INTRAUNLINKFUNC(peer_port);
    }
    AF_HIGH("AFDualCam-[mode -%d role-%d] Unlinking from peer session",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role);
  } else {
    rc = TRUE;
  }

  return rc;
}

/** af_port_remote_unlink:
 *    @port: MCT port data
 *    @event: module event received
 *
 * Asks Peer to do unlink (Master/Slave)
 *
 * Return boolean
 **/
boolean af_port_remote_unlink(mct_port_t *port)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  uint32_t                               peer_identity = 0;
  af_port_private_t  *private = (af_port_private_t *)(port->port_private);
  /* Forward the AF update info to the slave session */
  AF_HIGH("AFDualCam-[mode -%d role-%d] Remote Unlink issued",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);
  rc = stats_util_post_intramode_event(port,
                                       private->dual_cam_info.intra_peer_id,
                                       MCT_EVENT_MODULE_STATS_PEER_UNLINK,
                                       NULL);
  return rc;
}

/** af_port_remote_LPM:
 *    @port: MCT port data
 *    @event: module event received
 *
 * Informs peer that we have gone to LPM mode
 *
 * Return boolean
 **/
boolean af_port_remote_LPM(mct_port_t *port,cam_dual_camera_perf_control_t* perf_ctrl)
{
  boolean                                rc = FALSE;
  mct_port_t                            *this_port = port;
  mct_port_t                            *peer_port = NULL;
  cam_sync_related_sensors_event_info_t *link_param = NULL;
  uint32_t                               peer_identity = 0;
  af_port_private_t  *private = (af_port_private_t *)(port->port_private);

  if((private->dual_cam_info.cam_role == CAM_ROLE_MONO) &&
    (private->dual_cam_info.mode == CAM_MODE_SECONDARY)) {
    return rc;
  }

  /* Forward the LPM to the Master session */
  AF_HIGH("AFDualCam-[mode -%d role-%d] Remote LPM issued",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);
  rc = stats_util_post_intramode_event(port,
                                       private->dual_cam_info.intra_peer_id,
                                       MCT_EVENT_MODULE_STATS_PEER_LPM_EVENT,
                                       perf_ctrl);
  return rc;
}

/** af_port_LPM_from_peer:
 *    @port: MCT port data
 *    @event: module event received
 *
 * LPM from peer AF Port (Master/Slave)
 *
 * Return boolean
 **/
boolean af_port_LPM_from_peer(mct_port_t *port,
                                     cam_dual_camera_perf_control_t* perf_ctrl)
{
  af_port_private_t  *private = (af_port_private_t *)(port->port_private);
  if(private->dual_cam_info.is_LPM_on != perf_ctrl->enable) {
    private->dual_cam_info.is_LPM_on = perf_ctrl->enable;
    AF_HIGH("AFDualCam-[mode -%d role-%d] LPM Updated=%d",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role,
      perf_ctrl->enable);
    af_port_update_aux_sync_and_update(private);
  }
  return TRUE;
}

/** af_port_handle_role_switch:
 *    @port: MCT port data
 *
 * Handles Role switch from Master to Slave and Vice versa (Master/Slave)
 *
 * Return boolean
 **/
boolean af_port_handle_role_switch(mct_port_t *port, mct_event_t *event)
{
  mct_port_t       *this_port = port;
  af_port_private_t  *private = (af_port_private_t *)(port->port_private);

  if (event) {
    cam_dual_camera_master_info_t *dual_master_info = (cam_dual_camera_master_info_t *)
      (event->u.ctrl_event.control_event_data);
    if (dual_master_info){
      private->dual_cam_info.mode = dual_master_info->mode;
    }
  }

  af_port_update_aux_sync_and_update(private);
  af_port_set_role_switch(port);

  AF_HIGH("AFDualCam-[mode -%d role-%d 3aSync-%d AuxSync-%d AuxUpdate-%d] Role Switch Success",
    private->dual_cam_info.cam_role,
    private->dual_cam_info.mode,
    private->dual_cam_info.sync_3a_mode,
    private->dual_cam_info.is_aux_sync_enabled,
    private->dual_cam_info.is_aux_update_enabled);
  return TRUE;
}

/** af_port_reset_dual_cam_info:
 *    @port: AF Private  port data
 *
 * Resets Dual Cam Info (Master/Slave)
 *
 * Return boolean
 **/
boolean af_port_reset_dual_cam_info(af_port_private_t  *port)
{
  AF_HIGH("AFDualCam-[mode -%d role-%d] Reset",
    port->dual_cam_info.mode,
    port->dual_cam_info.cam_role);
  port->dual_cam_info.mode = CAM_MODE_PRIMARY;
  port->dual_cam_info.sync_3a_mode = CAM_3A_SYNC_NONE;
  port->dual_cam_info.is_LPM_on = FALSE;
  port->dual_cam_info.is_aux_sync_enabled = FALSE;
  port->dual_cam_info.is_aux_update_enabled = FALSE;
  port->dual_cam_info.intra_peer_id = 0;
  return TRUE;
}

/** af_port_update_aux_sync_and_update:
 *    @port: AF Private  port data
 *
 * Updates Aux sync and update conditions (Master/Slave)
 *
 * Return boolean
 **/
boolean af_port_update_aux_sync_and_update(af_port_private_t  *port)
{
  boolean is_aux_sync_enabled = FALSE;
  boolean is_aux_update_enabled = FALSE;

  if(port->dual_cam_info.is_LPM_on) {
    port->dual_cam_info.is_aux_sync_enabled = FALSE;
    port->dual_cam_info.is_aux_update_enabled = FALSE;
    AF_HIGH("AFDualCam-[mode -%d role-%d] Aux Sync Enabled=%d Aux Update Enabled=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_aux_sync_enabled,
      is_aux_update_enabled);
    return TRUE;
  }

  if ((port->dual_cam_info.is_LPM_on == FALSE)
    && (port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW)
    && (port->dual_cam_info.mode == CAM_MODE_PRIMARY)
    && (port->dual_cam_info.intra_peer_id != 0)) {
      is_aux_sync_enabled = TRUE;
  }

  if ((port->dual_cam_info.is_LPM_on == FALSE)
      &&(port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_FOLLOW)
      && (port->dual_cam_info.mode == CAM_MODE_SECONDARY)
      && (port->dual_cam_info.intra_peer_id != 0)) {
     is_aux_update_enabled = TRUE;
  }

  if(is_aux_sync_enabled != port->dual_cam_info.is_aux_sync_enabled) {
    AF_HIGH("AFDualCam-[mode -%d role-%d] Aux Sync Enabled=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_aux_sync_enabled);
    port->dual_cam_info.is_aux_sync_enabled = is_aux_sync_enabled;
  }

  if(is_aux_update_enabled != port->dual_cam_info.is_aux_update_enabled) {
    AF_HIGH("AFDualCam-[mode -%d role-%d] Aux Update Enabled=%d",
      port->dual_cam_info.mode,
      port->dual_cam_info.cam_role,
      is_aux_update_enabled);
    port->dual_cam_info.is_aux_update_enabled = is_aux_update_enabled;
  }
  return TRUE;
}


/** af_port_update_LPM:
 *    @port: port data
 *
 * Update LPM mode  (Master/Slave)
 *
 * Return boolean
 **/
boolean af_port_update_LPM(mct_port_t *port,cam_dual_camera_perf_control_t* perf_ctrl)
{
  boolean                  rc = FALSE;
  af_port_private_t  *private = (af_port_private_t *)(port->port_private);
  q3a_thread_af_msg_t *af_msg = NULL;

  af_msg = af_port_create_msg(MSG_AF_SET, AF_SET_PARAM_LOW_POWER_MODE, private);
  if (NULL == af_msg) {
    AF_ERR("LPM malloc fail");
      return rc;
  }

  af_msg->u.af_set_parm.u.low_power_mode.enable = perf_ctrl->enable;
  af_msg->u.af_set_parm.u.low_power_mode.priority = perf_ctrl->priority;
  q3a_port_map_perf_type(&af_msg->u.af_set_parm.u.low_power_mode.perf_mode,
    perf_ctrl->perf_mode);

  rc = q3a_af_thread_en_q_msg(private->thread_data, af_msg);

  if(private->dual_cam_info.is_LPM_on != perf_ctrl->enable) {
    private->dual_cam_info.is_LPM_on = perf_ctrl->enable;
    AF_HIGH("AFDualCam-[mode -%d role-%d] LPM Updated=%d",
      private->dual_cam_info.mode,
      private->dual_cam_info.cam_role,
      perf_ctrl->enable);
    af_port_update_aux_sync_and_update(private);
  }
  af_port_remote_LPM(port,perf_ctrl);
  return rc;
}

/** af_port_forward_focus_info
 * Forwards AF info to Peer Algorithm
 *
 * @port: port info
 *
 * @data: Af Information of Primary Algorithm
 *
 * Return: True - Success  False - failure
 **/
boolean af_port_forward_focus_info_if_linked(mct_port_t  *port) {
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  boolean rc = TRUE;
  if(af_port->dual_cam_info.is_aux_sync_enabled) {
    AF_LOW("AFDualCam-[mode -%d role-%d] Forwarding Focus Info",
      af_port->dual_cam_info.mode,
      af_port->dual_cam_info.cam_role);
    rc = stats_util_post_intramode_event(port,af_port->dual_cam_info.intra_peer_id,
      MCT_EVENT_MODULE_STATS_PEER_UPDATE_EVENT,&af_port->dual_cam_info.af_peer_focus_info);
  }
  return rc;
}

/** af_port_send_sleep_if_required:
 *    @port:   port info
 *    @swaf: if sw stats is enabled or not
 *
 * Send sleep command if we are mono and secondary and swaf is disabled
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_send_sleep_if_required(mct_port_t *port, boolean swaf)
{
  af_port_private_t      *af_port = (af_port_private_t *)(port->port_private);
  if((swaf == FALSE) &&
    (af_port->dual_cam_info.cam_role == CAM_ROLE_MONO) &&
    (af_port->dual_cam_info.mode == CAM_MODE_SECONDARY)) {
    AF_LOW("AFDualCam-[mode -%d role-%d] sending AF_SLEEP",
      af_port->dual_cam_info.mode,
      af_port->dual_cam_info.cam_role,
      af_port->dual_cam_info.mode);
    af_port_send_wakeup(port,FALSE);
  }
  return TRUE;
}

/** af_port_send_wakeup:
 *    @port:   port info
 *    @enable: to wake up or to sleep
 *
 * Send start/stop request for Diverting frames.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_send_wakeup(mct_port_t *port, boolean enable)
{
  af_port_private_t      *af_port = (af_port_private_t *)(port->port_private);
  boolean                rc = TRUE;
  mct_bus_msg_ctrl_request_frame_t req_frame_msg;
  memset(&req_frame_msg, 0, sizeof(mct_bus_msg_ctrl_request_frame_t));

  if (enable == TRUE) {
    AF_LOW("AFDualCam-[mode -%d role-%d] Send AF_WAKEUP sID=%d",
      af_port->dual_cam_info.mode,
      af_port->dual_cam_info.cam_role,
      (af_port->reserved_id >> 16),
      af_port->dual_cam_info.cam_role);
    req_frame_msg.request_flag = TRUE;
    req_frame_msg.req_mode = FRM_REQ_MODE_CONTINUOUS;
  } else {
    AF_LOW("AFDualCam-[mode -%d role-%d] Send AF_SLEEP sID=%d",
      af_port->dual_cam_info.mode,
      af_port->dual_cam_info.cam_role,
      (af_port->reserved_id >> 16),
      af_port->dual_cam_info.cam_role);
    req_frame_msg.request_flag = FALSE;
    req_frame_msg.req_mode = FRM_REQ_MODE_NONE;
  }
  req_frame_msg.lpm_req_bit = LPM_REQUEST_BIT_STATS;
  rc = af_send_bus_msg(port, MCT_BUS_MSG_CONTROL_REQUEST_FRAME, &req_frame_msg,
    sizeof(mct_bus_msg_ctrl_request_frame_t),0);
  return rc;
}

/** af_port_wake_up_if_required
 * Handles AF Focus Peer event and checks if we need to do a wake up
 *
 * @port: port info
 *
 * @data: Af Information of Primary Algorithm
 *
 * Return: True - Success  False - failure
 **/
boolean af_port_wake_up_if_required(mct_port_t  *port,
  af_core_focus_info *info) {
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  boolean rc = FALSE;
  boolean isValidTransition = FALSE;

  // If not B+M and M being slave, do not take control
  // of LPM Sleep and Wakeup
  if(!((af_port->dual_cam_info.cam_role == CAM_ROLE_MONO) &&
      (af_port->dual_cam_info.mode == CAM_MODE_SECONDARY)) ||
      (af_port->dual_cam_info.need_wakeup_trigger == FALSE)) {
      return rc;
  }

  // Even for B+M configuration, MONO wakeup/sleep needs to
  // be handled by AF only for HAL3. For HAL1, Ignore for now
  if(af_port->hal_version == CAM_HAL_V1) {
      return rc;
  }

  // When the master is transitioning from FOCUSED to UNFOCUSED
  // because of scene change or luma instability, there is no
  // need to Wakeup the Slave untill there is an active search
  // from the master. This check is to safe guard such invalid
  // transitions from FOCUSED to UNFOCUSED or vice versa
  if (!((af_port->dual_cam_info.af_peer_focus_info.focusStatus == AF_CORE_STATUS_FOCUSED &&
     info->focusStatus == AF_CORE_STATUS_NOT_FOCUSED) ||
     (af_port->dual_cam_info.af_peer_focus_info.focusStatus == AF_CORE_STATUS_NOT_FOCUSED &&
     info->focusStatus == AF_CORE_STATUS_FOCUSED))) {
     isValidTransition = TRUE;
  }

  if((af_port->focusedAtLeastOnce == FALSE) ||
    ((af_port->dual_cam_info.af_peer_focus_info.focusStatus != info->focusStatus) && isValidTransition)) {
    AF_LOW("AFDualCam-[mode -%d role-%d] WakeupRequired Master Prev and Cur Focus Status=[%d %d] "
      "FocussedatLeastOnce=%d",
      af_port->dual_cam_info.mode,
      af_port->dual_cam_info.cam_role,
      af_port->dual_cam_info.af_peer_focus_info.focusStatus, info->focusStatus,
      af_port->focusedAtLeastOnce);
    rc = af_port_send_wakeup(port,TRUE);
   } else if ((af_port->dual_cam_info.af_peer_focus_info.focusStatus == info->focusStatus) && (info->focusStatus == AF_CORE_STATUS_FOCUSED)) {
    AF_LOW("AFDualCam-[mode -%d role-%d] SleepRequired Master Prev and Cur Focus Status=[%d %d] "
      "FocussedatLeastOnce=%d",
      af_port->dual_cam_info.mode,
      af_port->dual_cam_info.cam_role,
      af_port->dual_cam_info.af_peer_focus_info.focusStatus, info->focusStatus,
      af_port->focusedAtLeastOnce);
    rc = af_port_send_wakeup(port,FALSE);
   }
  return rc;
}

/** af_port_handle_focus_info
 * Handles AF Focus Peer event
 *
 * @port: port info
 *
 * @data: Af Information of Primary Algorithm
 *
 * Return: True - Success  False - failure
 **/
boolean af_port_handle_peer_focus_info(mct_port_t  *port,
  af_core_focus_info *info) {
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  boolean rc = FALSE;
  AF_HIGH("AFDualCam-[mode -%d role-%d] Handle Peer focus",
    af_port->dual_cam_info.mode,
    af_port->dual_cam_info.cam_role);

  af_port_wake_up_if_required(port,info);
  af_port->dual_cam_info.af_peer_focus_info = *info;
  /* Allocate memory to create AF message. we'll post it to AF thread. */
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_SET_PEER_FOCUS_INFO, af_port);
  if (af_msg == NULL) {
    AF_ERR("Not enough memory.");
    return rc;
  }
  /* populate af message to post to thread */
  af_msg->u.af_set_parm.u.af_peer_focus_info = *info;

  /* Enqueue the message to the AF thread */
  rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  return rc;
}

/** af_port_set_role_switch
 * Set AF Algorithm Role
 *
 * @port: port info
 *
 * @followMode: Follow Mode True or False
 *
 * Return: True - Success  False - failure
 **/
boolean af_port_set_role_switch(mct_port_t  *port)
{

  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  boolean rc = FALSE;

  /* Allocate memory to create AF message. we'll post it to AF thread. */
  q3a_thread_af_msg_t *af_msg = af_port_create_msg(MSG_AF_SET,
    AF_SET_PARAM_SET_ROLE_SWITCH, af_port);
  if (af_msg == NULL) {
    AF_ERR("Not enough memory.");
    return rc;
  }

  Q3A_MEMSET(&af_msg->u.af_set_parm.u.role_switch,0,sizeof(af_role_switch_params_t));
  /* populate af message to post to thread */
  /* If sync mode is none both camera has to run independently so
     both are master else primary camera is the master camera.*/
  if (af_port->dual_cam_info.sync_3a_mode == CAM_3A_SYNC_NONE) {
    af_msg->u.af_set_parm.u.role_switch.master = TRUE;
    Q3A_MEMCPY(&af_msg->u.af_set_parm.u.role_switch.af_peer_focus_info,
      &af_port->dual_cam_info.af_peer_focus_info,
      sizeof(af_core_focus_info));
  } else if(af_port->dual_cam_info.mode == CAM_MODE_PRIMARY) {
    af_msg->u.af_set_parm.u.role_switch.master = TRUE;
    Q3A_MEMCPY(&af_msg->u.af_set_parm.u.role_switch.af_peer_focus_info,
      &af_port->dual_cam_info.af_peer_focus_info,
      sizeof(af_core_focus_info));
  }

  AF_HIGH("AFDualCam-[mode -%d role-%d] Set Role Switch =%d",
    af_port->dual_cam_info.mode,
    af_port->dual_cam_info.cam_role,
    af_msg->u.af_set_parm.u.role_switch.master);

  /* Enqueue the message to the AF thread */
  rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  return rc;
}



/** af_port_intra_event:
 *    @port:  MCT port
 *    @event: MCT module
 *
 * Handles the intra-module events sent between AF master and slave sessions
 *
 * Return TRUE if the event is processed successfully.
 **/
boolean af_port_intra_event(mct_port_t *port, mct_event_t *event)
{
  af_port_private_t *private;

  /* sanity check */
  if (!port || !event) {
    return FALSE;
  }

  private = (af_port_private_t *)(port->port_private);
  if (!private) {
    return FALSE;
  }

  AF_LOW("AFDualCam-[mode -%d role-%d] Received AF intra-module peer event",
    private->dual_cam_info.mode,
    private->dual_cam_info.cam_role);

  /* sanity check: ensure event is meant for port with same identity*/
  if ((private->reserved_id & 0xFFFF0000) != (event->identity & 0xFFFF0000)) {
    return FALSE;
  }


  /* check if there's need for extended handling. */
  if (private->func_tbl.ext_handle_intra_event) {
    stats_ext_return_type ret;
    AF_LOW("Handle extended intra port event!");
    ret = private->func_tbl.ext_handle_intra_event(port, event);
    /* Check if this event has been completely handled. If not we'll process it further here. */
    if (STATS_EXT_HANDLING_COMPLETE == ret) {
      AF_LOW("Intra port event %d handled by extended functionality!",
        event->u.module_event.type);
      return TRUE;
    }
  }

  switch(event->u.module_event.type) {
    case MCT_EVENT_MODULE_STATS_PEER_UPDATE_EVENT:
      AF_LOW("AFDualCam-[mode -%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_UPDATE_EVENT",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      af_port_handle_peer_focus_info(port,
          (af_core_focus_info*)event->u.module_event.module_event_data);
      break;
    case MCT_EVENT_MODULE_STATS_PEER_UNLINK:
      AF_LOW("AFDualCam-[mode -%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_UNLINK",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      af_port_local_unlink(port,event);
      af_port_reset_dual_cam_info(private);
      break;
    case MCT_EVENT_MODULE_STATS_PEER_LPM_EVENT:
      AF_LOW("AFDualCam-[mode -%d role-%d] Received MCT_EVENT_MODULE_STATS_PEER_LPM_EVENT",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role);
      af_port_LPM_from_peer(port,
        (cam_dual_camera_perf_control_t*)event->u.ctrl_event.control_event_data);
      break;
    default:
      AF_ERR("AFDualCam-[mode -%d role-%d] Error! Received unknown intra-module event type: %d",
        private->dual_cam_info.mode,
        private->dual_cam_info.cam_role,
        event->u.module_event.type);
      break;
  }
  return TRUE;
}
/*End of AF Dual Camera Port handling*/
