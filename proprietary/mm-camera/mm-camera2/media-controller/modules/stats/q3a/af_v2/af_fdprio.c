/* af_fdprio.c
 *
 * Copyright (c) 2016, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "af_port.h"
#include "af_fdprio.h"
#include "math.h"
/********************************************
  Type Declarations
********************************************/

#define FDPRIO_INDEX_INVALID         0xFFFF
#define FDPRIO_SCN_CHG_THRESH           2

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a)  (((a) > 0) ? (a) : -(a))
#endif


#define PERCENT_CHANGE(a, b) \
    (((float)MAX(a, b)/(float)MAX(MIN(a, b),1))*100.0-100.0)

#define PERCENT_LOC_CHANGE(a, b, c) (((float)ABS(a - b) / MAX(c,1)) * 100)

#define FDPRIO_NO_FACE_COUNT_MAX        0xFF


/********************************************
  FACE DETECT PRIORITY ALGORITHM API
********************************************/

/* Internal helper functions */
static void    fdprio_init(fdprio_t *fdprio_data);
static boolean fdprio_process_counters(fdprio_t *fdprio_data);
static boolean fdprio_process_counters_int(
  fdprio_t *fdprio_data);
static boolean fdprio_process_fd_roi(fdprio_t *fdprio_data);
static int32_t fdprio_find_biggest_face(mct_face_info_t *face_info);
static boolean fdprio_is_tracked_face_present(
  fdprio_t *fdprio_data);
static boolean fdprio_send_default_roi(fdprio_t *fdprio_data);
static int32_t fdprio_get_index_by_id(fdprio_t *fdprio_data,
  int32_t face_id);
static boolean fdprio_is_new_face_big_enough(
  fdprio_t *fdprio_data,
  int32_t curr_biggest_face_id);
static boolean fdprio_send_current_roi(fdprio_t *fdprio_data);
static void    fdprio_begin_new_history(fdprio_t *fdprio_data);
static void fdprio_update_face_info(
  fdprio_face_info_t *fdprio_data,
  mct_face_data_t *face_info,
  int32_t loc_stability_cnt,
  int32_t size_stability_cnt);
static boolean fdprio_check_loc_stability(
  fdprio_t *fdprio_data,
  fdprio_face_info_t *this_face,
  int32_t new_ctr_x,
  int32_t new_ctr_y);
static boolean fdprio_check_size_stability(
  fdprio_t *fdprio_data,
  fdprio_face_info_t *this_face,
  int32_t new_roi_top_left_x,
  int32_t new_roi_top_left_y);
static void fdprio_check_face_stability(
  fdprio_t *fdprio_data,
  fdprio_face_info_t *this_face,
  mct_face_data_t *cur_update,
  boolean *is_loc_stable,
  boolean *is_size_stable);
static boolean fdprio_compare_with_ref_roi(
  fdprio_t *fdprio_data,
  cam_rect_t *ref_roi,
  cam_rect_t *cur_roi,
  int32_t frame_width);

static void fdprio_dbg_print_face_dim(fdprio_t *fdprio_data);
static boolean fdprio_handle_aec_update(fdprio_t *fdprio_data,
  fdprio_aec_set_parm_t *aec_info);
static boolean fdprio_handle_gyro_data(fdprio_t *fdprio_data, float gyro_sqr);

/** af_fdprio_create_msg:
 *    @msg_type:   Type to be set by the msg
 *    @param_type: Parameter type
 *    @private: Contains port private structure containing common info.
 *
 * Create msg intended to be queue to the algorithm thread. Allocate memory,
 *  assign a type and set common parameters to the header of the msg.
 *
 * Return: Address of the allocated msg
 **/
q3a_thread_af_msg_t* fdprio_create_msg(int msg_type,
  int param_type, fdprio_t *af_port)
{
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));

  if (af_msg == NULL) {
    return NULL;
  }
  memset(af_msg, 0 , sizeof(q3a_thread_af_msg_t));

  af_msg->type = msg_type;
  af_msg->camera_id = af_port->camera_id;
  if (msg_type == MSG_AF_SET || msg_type == MSG_AF_CANCEL) {
    af_msg->u.af_set_parm.type = param_type;
    af_msg->u.af_set_parm.camera_id = af_port->camera_id;
  } else if (msg_type == MSG_AF_GET) {
    af_msg->u.af_get_parm.type = param_type;
    af_msg->u.af_get_parm.camera_id = af_port->camera_id;
  }

  return af_msg;
}

/********************************************
  FACE DETECT PRIORITY SCENE CHANGE UTILITY
********************************************/

/** fdprio_scn_chg_create
 *    @scn_mtr:  Scene mtr configuration
 *
 *  This is the  function to create and configure instances of
 *  value monitor.
 *
 *  Return void
 **/

void fdprio_scn_chg_create(fdprio_scene_monitor_t *scn_mtr)
{
  int sens_level;
  af_vm_config_adapter_t vm_config;
  vm_config.detection_type = Q3A_VM_DETECTOR_UNSTABLE;
  vm_config.absolute_mode_cfg.enable = 1;
  vm_config.absolute_mode_cfg.baseline = 0.000000f;

  vm_config.sens_profile_low.trigger_threshold = 12.77f;
  vm_config.sens_profile_low.consecutive_count_threshold = 3;
  vm_config.sens_profile_low.frame2armed = 3;
  vm_config.sens_profile_low.median_fltr_cfg.enable = 1;
  vm_config.sens_profile_low.median_fltr_cfg.num_of_samples = 5;
  vm_config.sens_profile_low.mvavg_fltr_cfg.enable = 1;
  vm_config.sens_profile_low.mvavg_fltr_cfg.num_of_samples = 5;
  vm_config.sens_profile_low.iir_fltr_cfg.enable = 0;
  vm_config.sens_profile_low.iir_fltr_cfg.num_of_samples = 2;
  vm_config.sens_profile_low.iir_fltr_cfg.coeff_numerator = 0.245200f;
  vm_config.sens_profile_low.iir_fltr_cfg.coeff_denominator = -0.509500f;

  vm_config.sens_profile_med.trigger_threshold = 9.7f;
  vm_config.sens_profile_med.consecutive_count_threshold = 3;
  vm_config.sens_profile_med.frame2armed = 3;
  vm_config.sens_profile_med.median_fltr_cfg.enable = 1;
  vm_config.sens_profile_med.median_fltr_cfg.num_of_samples = 3;
  vm_config.sens_profile_med.mvavg_fltr_cfg.enable = 1;
  vm_config.sens_profile_med.mvavg_fltr_cfg.num_of_samples = 3;
  vm_config.sens_profile_med.iir_fltr_cfg.enable = 0;
  vm_config.sens_profile_med.iir_fltr_cfg.num_of_samples = 2;
  vm_config.sens_profile_med.iir_fltr_cfg.coeff_numerator = 0.245200f;
  vm_config.sens_profile_med.iir_fltr_cfg.coeff_denominator = -0.509500f;

  vm_config.sens_profile_high.trigger_threshold = 7.29f;
  vm_config.sens_profile_high.consecutive_count_threshold = 3;
  vm_config.sens_profile_high.frame2armed = 3;
  vm_config.sens_profile_high.median_fltr_cfg.enable = 0;
  vm_config.sens_profile_high.median_fltr_cfg.num_of_samples = 0;
  vm_config.sens_profile_high.mvavg_fltr_cfg.enable = 1;
  vm_config.sens_profile_high.mvavg_fltr_cfg.num_of_samples = 2;
  vm_config.sens_profile_high.iir_fltr_cfg.enable = 0;
  vm_config.sens_profile_high.iir_fltr_cfg.num_of_samples = 2;
  vm_config.sens_profile_high.iir_fltr_cfg.coeff_numerator = 0.245200f;
  vm_config.sens_profile_high.iir_fltr_cfg.coeff_denominator = -0.509500f;

  /*For SADR */
  /* Initialize  SAD Scene Monitor*/
  scn_mtr->p_vm_sad = af_value_monitor_create("FD_SADR");
  /* Obtain logical sensitivity level for SADR value monitor */
  sens_level = 50;
  /* Send the selected Value Monitor Tuning to Value Monitor Object */
  scn_mtr->p_vm_sad->set_config(scn_mtr->p_vm_sad, &vm_config, sens_level);


  /* SCENE CHANGE : GYRO */
  /* Initialize  SAD Scene Monitor*/
  scn_mtr->p_vm_gyro = af_value_monitor_create("FD_GYRO");
  vm_config.sens_profile_low.trigger_threshold = 0.1693f;
  vm_config.sens_profile_low.consecutive_count_threshold = 1;
  vm_config.sens_profile_med.trigger_threshold = 0.1539f;
  vm_config.sens_profile_med.consecutive_count_threshold = 1;
  vm_config.sens_profile_high.trigger_threshold = 0.1386f;
  vm_config.sens_profile_high.consecutive_count_threshold = 1;
  scn_mtr->p_vm_gyro->set_config(scn_mtr->p_vm_gyro, &vm_config, sens_level);
}
/** fdprio_scn_chg_destroy
 *    @scn_mtr:  Scene mtr configuration
 *
 *  This function deletes the instances of value monitor.
 *
 *  Return void
 **/

void fdprio_scn_chg_destroy(fdprio_scene_monitor_t *scn_mtr)
{
  af_value_monitor_destroy(scn_mtr->p_vm_sad);
  af_value_monitor_destroy(scn_mtr->p_vm_gyro);
}

/** fdprio_is_scn_chg
 *    @scn_mtr:  Scene mtr configuration
 *
 *  This function returns the status of scene change.It queries
 *  individual instances of Value monitor to detect scene change
 *
 *  Return TRUE if scene change is detected , FALSE if not.
 **/
boolean fdprio_is_scn_chg(fdprio_scene_monitor_t *scn_mtr)
{
  boolean is_sad_scn_chg = scn_mtr->p_vm_sad->get_trigger(scn_mtr->p_vm_sad);
  boolean is_gyro_scn_chg = scn_mtr->p_vm_gyro->get_trigger(scn_mtr->p_vm_gyro);

  return (is_sad_scn_chg || is_gyro_scn_chg);
}

/** fdprio_scn_chg_rebase_ref
 *    @scn_mtr:  Scene mtr configuration
 *
 *  This function resets all the internal states and values
 *  of the value monitor
 *
 *  Return Void
 **/

void fdprio_scn_chg_rebase_ref(fdprio_scene_monitor_t *scn_mtr)
{
  scn_mtr->p_vm_sad->rebase_ref(scn_mtr->p_vm_sad);
  scn_mtr->p_vm_gyro->rebase_ref(scn_mtr->p_vm_gyro);
}

/** fdprio_scn_chg_set_input
 *    @scn_mtr:  Scene mtr configuration
 *    @param: Input parms to set Value monitor inputs
 *
 *  This function handles all set params to set inputs to value
 *  monitor.
 *
 *  Return Void
 **/
void fdprio_scn_chg_set_input(fdprio_scene_monitor_t *scn_mtr, fdprio_scn_chg_set_input_t *param)
{
    if (NULL == scn_mtr || NULL == param) {
      return;
    }
    switch (param->type) {
    case FDPRIO_SCN_CHG_SAD:
        scn_mtr->p_vm_sad->set_input(scn_mtr->p_vm_sad, param->data);
        break;
    case FDPRIO_SCN_CHG_GYRO:
        scn_mtr->p_vm_gyro->set_input(scn_mtr->p_vm_gyro, param->data);
        break;
    default:
      AF_ERR("AF error: invalid param type: %d", param->type);
      break;
    }
}
/********************************************
  FACE DETECT PRIORITY ALGORITHM API IMPLEMENTATION
********************************************/

/** fdprio_init
 * Initialize the internal data for the FD priority AF feature
 *
 * @fdprio_data: internal data to control the FD priority AF feature
 *
 **/
static void fdprio_init(fdprio_t *fdprio_data)
{
  memset(fdprio_data, 0, sizeof(fdprio_t));
  /* We need to initialize noface_cnt to threshold so that we don't
     trigger unnecessary "reset to default" search in the beginning.
     We need to reset it only we detect first face */
  fdprio_data->noface_cnt = FDPRIO_NO_FACE_COUNT_MAX;
  fdprio_scn_chg_create(&(fdprio_data->scn_mtr));
  AF_LOW("Initialized");
}

/** fdprio_deinit
 * Deinitialize the internal data for the FD priority AF feature
 *
 * @fdprio_data: internal data to control the FD priority AF feature
 *
 **/
static void fdprio_deinit(fdprio_t *fdprio_data)
{

  fdprio_scn_chg_destroy(&(fdprio_data->scn_mtr));

  AF_LOW("Deinitialized");
}

/** fdprio_set_param
 * This function handles set params to set inputs to the fd
 * priority algorithm
 *
 * @fdprio_set_parm: Set param control data for clients to
 * set inputs to fd prio algorithm
 *
 **/
boolean fdprio_set_param(fdprio_set_parm *param)
{
  boolean rc            = TRUE;
  if (!param) {
   rc = FALSE;
   goto done;
  }
  fdprio_t *fdprio_data = param->fdprio_data;
  switch (param->type) {
  case FDPRIO_SET_AEC_DATA:
      rc = fdprio_handle_aec_update(fdprio_data, &(param->u.aec_info));
      break;
  case FDPRIO_SET_GYRO_DATA:
      rc = fdprio_handle_gyro_data(fdprio_data, param->u.gyro_sqr);
      break;
  default:
      rc = FALSE;
      break;
  }
done:
    return rc;
}
/** fdprio_process
 * This is the entry function to process face detection.
 *
 * @fdprio_data: Internal control data for fd priority
 *
 **/

boolean fdprio_process(
  fdprio_t *fdprio_data,
  fdprio_cmd_t cmd)
{
  boolean rc = FALSE;

  if (!fdprio_data) {
    AF_ERR("Null pointer passed for fdprio_data");
    return FALSE;
  }

  switch (cmd)
  {
  case FDPRIO_CMD_INIT:
    fdprio_init(fdprio_data);
    rc = TRUE;
    break;
  case FDPRIO_CMD_DEINIT:
    fdprio_deinit(fdprio_data);
    rc = TRUE;
    break;
  case FDPRIO_CMD_PROC_COUNTERS:
    rc = fdprio_process_counters(fdprio_data);
    break;
  case FDPRIO_CMD_PROC_FD_ROI:
    rc = fdprio_process_fd_roi(fdprio_data);
    break;
  default:
    break;
  }

  return rc;
}

/********************************************
 FACE DETECT PRIORITY ALGORITHM  IMPLEMENTATION
********************************************/

/** fdprio_adjust_stability_count:
 * adjust stability count based on the threshold.
 *
 *  @af: internal AF data structure
 *
 **/
int fdprio_adjust_stability_count(fdprio_t *fdprio_data,
  int input_stability_count)
{
  af_fd_priority_caf_adapter_t *fd_tuning =
    &fdprio_data->tuning_info.af_algo.fd_prio;
  fdprio_scene_monitor_t *scn_mtr =  &fdprio_data->scn_mtr;
  int adjusted_count = input_stability_count;
  AF_LOW("Preview-fps: %d Stability count: %d th: %d adj count %d",
    fdprio_data->preview_fps, input_stability_count,
    fd_tuning->fps_adjustment_th, adjusted_count);

  if (fdprio_data->preview_fps <= fd_tuning->fps_adjustment_th) {
    adjusted_count /= 2;
    if (fdprio_data->preview_fps < (fd_tuning->fps_adjustment_th / 2)) {
      adjusted_count /= 2;
    }
  }

  if (adjusted_count < 1) {
    adjusted_count = 1;
  }

  if (fdprio_is_scn_chg(scn_mtr)) {
    adjusted_count = FDPRIO_SCN_CHG_THRESH;
  }

  AF_LOW("stability_count after adjustment: %d", adjusted_count);

  return adjusted_count;
} /* fdprio_adjust_stability_count */

/** fdprio_process_counters:
 * Heartbeat trigger to the fd priority algorithm which is
 * called every frame. Used for internally to handle per frame
 * processing
 *
 *  @af: internal AF data structure
 *
 **/
static boolean fdprio_process_counters(fdprio_t *fdprio_data)
{
  boolean rc;

  if(fdprio_data->last_processed_frame_id ==
      fdprio_data->current_frame_id) {
    AF_LOW("Already processed, returning...");
    return TRUE;
  }

  rc = fdprio_process_counters_int(fdprio_data);
  return rc;
}

/** fdprio_process_counters_int:
 * Called when Fd priority is initialized and is used to
 * initialize internal counters
 *
 *  @af: internal AF data structure
 *
 **/
static boolean fdprio_process_counters_int(fdprio_t *fdprio_data)
{
  int no_face_wait_th = fdprio_data->tuning_info.af_algo.fd_prio.no_face_wait_th;
  fdprio_scene_monitor_t *scn_mtr =  &fdprio_data->scn_mtr;

  if (fdprio_is_scn_chg(scn_mtr)) {
      no_face_wait_th = FDPRIO_SCN_CHG_THRESH;
  }
  if (fdprio_data->faces_detected) {
    fdprio_data->noface_cnt = 0;
  } else {
    /* increment only once above threshold */
    if (fdprio_data->noface_cnt <= no_face_wait_th) {
      fdprio_data->noface_cnt++;
      AF_LOW("No face counter: %d",
        fdprio_data->noface_cnt);
      /* check if if the threshold is reached */
      if (fdprio_data->noface_cnt > no_face_wait_th) {
        AF_LOW("Trigger default ROI AF_START");
        fdprio_data->noface_cnt = fdprio_data->tuning_info.af_algo.fd_prio.no_face_wait_th + 1;
        fdprio_send_default_roi(fdprio_data);
        fdprio_scn_chg_rebase_ref(&(fdprio_data->scn_mtr));
      }
    }
  }

  fdprio_data->last_processed_frame_id = fdprio_data->current_frame_id;

  return TRUE;
} /* af_port_process_fdaf_counters */

/** fdprio_dbg_print_face_dim:
 * Function to print face information. Used for debugging
 * issues
 *
 *  @af: internal AF data structure
 *
 **/

static void fdprio_dbg_print_face_dim(fdprio_t *fdprio_data)
{
  uint32_t i;
  mct_face_info_t *face_info;

  face_info = fdprio_data->pface_info;

  AF_LOW("Face info (count: %d): ", face_info->face_count);
  for(i = 0; i < face_info->face_count; i++) {
    AF_LOW("Face-id: %d ROI: x: %d y: %d dx: %d dy: %d",
      face_info->faces[i].face_id, face_info->faces[i].roi.left,
      face_info->faces[i].roi.top, face_info->faces[i].roi.width,
      face_info->faces[i].roi.height);
  }
}

/** fdprio_update_face_info:
 * Function to update Face information in the fd priority
 * logic.
 *
 *  @af: internal AF data structure
 *
 **/
static void fdprio_update_face_info(
  fdprio_face_info_t *fdprio_data,
  mct_face_data_t *face_info,
  int32_t loc_stability_cnt,
  int32_t size_stability_cnt)
{
  fdprio_data->is_valid = TRUE;
  fdprio_data->face_id = face_info->face_id;
  fdprio_data->ctr_x = face_info->roi.left + face_info->roi.width / 2;
  fdprio_data->ctr_y = face_info->roi.top + face_info->roi.height / 2;
  fdprio_data->top_left_x = face_info->roi.left;
  fdprio_data->top_left_y = face_info->roi.top;
  fdprio_data->loc_stability_cnt = loc_stability_cnt;
  fdprio_data->size_stability_cnt = size_stability_cnt;
  AF_LOW("face-id: %d Face-center (%d, %d) top_left_coord: (%d,%d)",
    fdprio_data->face_id, fdprio_data->ctr_x, fdprio_data->ctr_y,
    fdprio_data->top_left_x, fdprio_data->top_left_y);
  AF_LOW("Stability Count - Location: %d Size: %d",
    fdprio_data->loc_stability_cnt, fdprio_data->size_stability_cnt);
} /* fdprio_update_face_info */

/** fdprio_compare_with_ref_roi:
 * Function to detect if new face coordinates are within
 * reference ROI.
 *
 *  @af: internal AF data structure
 *
 **/
static boolean fdprio_compare_with_ref_roi(
  fdprio_t *fdprio_data,
  cam_rect_t *ref_roi,
  cam_rect_t *cur_roi,
  int32_t frame_width)
{
  boolean roi_x_stable = FALSE, roi_y_stable = TRUE;
  float percent_change = 0.0;
  int32_t ref_ctr_x = ref_roi->left + ref_roi->width / 2;
  int32_t ref_ctr_y = ref_roi->top + ref_roi->height/ 2;
  int32_t cur_ctr_x = cur_roi->left + cur_roi->width / 2;
  int32_t cur_ctr_y = cur_roi->top + cur_roi->height / 2;
  af_fd_priority_caf_adapter_t *fd_tuning =
    &fdprio_data->tuning_info.af_algo.fd_prio;

  AF_LOW("Input - Ref ROI x:%d y:%d dx:%d dy:%d"
    "Cur ROI x:%d y:%d dx:%d dy:%d frame-width: %d",
    ref_roi->left, ref_roi->top, ref_roi->width, ref_roi->height,
    cur_roi->left, cur_roi->top, cur_roi->width, cur_roi->height,
    frame_width);

  /* Check difference in x-axis */
  percent_change = PERCENT_LOC_CHANGE(ref_ctr_x, cur_ctr_x, frame_width);
  AF_LOW("Percent X-Axis difference: %f Th: %f",
     percent_change, fd_tuning->pos_change_th);
  roi_x_stable =
    (percent_change > fd_tuning->pos_change_th) ? FALSE : TRUE;

  /* Check difference in Y-axis */
  /* Note: use frame_width here too as it remains fixed for different resolution (4:3, 16:9) */
  percent_change = PERCENT_LOC_CHANGE(ref_ctr_y, cur_ctr_y, frame_width);
  AF_LOW("Percent Y-Axis difference: %f Threshold: %f",
    percent_change, fd_tuning->pos_change_th);
  roi_y_stable =
    (percent_change > fd_tuning->pos_change_th) ? FALSE : TRUE;

  return (roi_x_stable & roi_y_stable);
} /* fdprio_check_loc_stability */

/** fdprio_check_loc_stability:
 * Function to detect if the position coordinates of new face
 * coordinates are stable
 *
 *  @af: internal AF data structure
 *
 **/
static boolean fdprio_check_loc_stability(
  fdprio_t *fdprio_data,
  fdprio_face_info_t *this_face,
  int32_t new_ctr_x,
  int32_t new_ctr_y)
{
  boolean loc_x_stable = FALSE, loc_y_stable = FALSE;
  float percent_change = 0.0;
  int32_t frame_width = fdprio_data->camif_width;
  int32_t last_ctr_x = this_face->ctr_x;
  int32_t last_ctr_y = this_face->ctr_y;
  af_fd_priority_caf_adapter_t *fd_tuning =
    &fdprio_data->tuning_info.af_algo.fd_prio;
  int stable_count_pos = fd_tuning->stable_count_pos;

  /* Adjust stability count threshold based on current fps */
  stable_count_pos = fdprio_adjust_stability_count(fdprio_data,
    fd_tuning->stable_count_pos);
  AF_LOW("Input - Last Ctr (%d, %d) Cur Ctr (%d, %d) frame-width: %d",
     last_ctr_x, last_ctr_y,
    new_ctr_x, new_ctr_y, frame_width);

  /* Check location stability in x-axis */
  percent_change = PERCENT_LOC_CHANGE(new_ctr_x, last_ctr_x, frame_width);
  AF_LOW("Percent Loc Change in X-axis: %f Pos-Stable-Th Hi: %f Low: %f",
     percent_change, fd_tuning->pos_stable_th_hi,
    fd_tuning->pos_stable_th_low);
  /* this percent_change between position of last and current ROI of the face
     * determine whether it is still moving or stable.
     * However we will use two thresholds - THRESHLD_HI and THRESHOLD_LOW.
     * THRESHLD_HI is checked to detect motion. If percent_change is more than
     * this threshold we'll indicate that face is moving.
     * THRESHLD_LO is checked while slowing down. Only after percent_change is
     * lower than this threshold we will assume face is stable.
     * Using two thresholds give better stability and flexibility for tuning */

  if (percent_change > fd_tuning->pos_stable_th_hi) {
    loc_x_stable = FALSE;
  } else if (percent_change < fd_tuning->pos_stable_th_low) {
    loc_x_stable = TRUE;
  } else {
    /* If percent_change is within LO and HI threshold stability is kept as before */
    loc_x_stable =
      (this_face->loc_stability_cnt < (uint32_t) stable_count_pos) ? FALSE : TRUE;
  }

  /* Check location stability in Y-axis */
  /* Note: use frame_width here too as it remains fixed for different resolution (4:3, 16:9) */
  percent_change = PERCENT_LOC_CHANGE(new_ctr_y, last_ctr_y, frame_width);
  AF_LOW("Percent Loc Change in Y-axis: %f Pos-Stable-Th Hi: %f Low: %f",
     percent_change, fd_tuning->pos_stable_th_hi,
    fd_tuning->pos_stable_th_low);
  if (percent_change > fd_tuning->pos_stable_th_hi) {
    loc_y_stable = FALSE;
  } else if (percent_change < fd_tuning->pos_stable_th_low) {
    loc_y_stable = TRUE;
  } else {
    loc_y_stable =
      (this_face->loc_stability_cnt < (uint32_t) stable_count_pos) ? FALSE : TRUE;
  }

  return (loc_x_stable & loc_y_stable);
} /* fdprio_check_loc_stability */


/** fdprio_check_loc_stability:
 * Function to detect if the size new face coordinates are
 * stable
 *
 *  @af: internal AF data structure
 *
 **/
static boolean fdprio_check_size_stability(
  fdprio_t *fdprio_data,
  fdprio_face_info_t *this_face,
  int32_t new_roi_top_left_x,
  int32_t new_roi_top_left_y)
{
  boolean rc = FALSE;
  float percent_change = 0.0;
  int32_t delta_x = 0, delta_y = 0;
  int32_t new_size_delta = 0;
  af_fd_priority_caf_adapter_t *fd_tuning =
    &fdprio_data->tuning_info.af_algo.fd_prio;

  AF_LOW("Top-Left coordinate - Ref ROI: (%d, %d) New ROI: (%d, %d)",
     this_face->top_left_x, this_face->top_left_y,
    new_roi_top_left_x, new_roi_top_left_y);

  delta_x = new_roi_top_left_x - this_face->top_left_x;
  delta_y = new_roi_top_left_y - this_face->top_left_y;
  new_size_delta = (uint32_t) sqrt(delta_x * delta_x + delta_y * delta_y);

  percent_change = ((float)new_size_delta / fdprio_data->camif_width) * 100;
  AF_LOW("Percent Size Change : %f Threshold: %f",
    percent_change, fd_tuning->size_change_th);
  rc = (percent_change > fd_tuning->size_change_th) ? FALSE : TRUE;

  return rc;
} /* fdprio_check_loc_stability */

/** fdprio_check_face_stability:
 * Function to detect both position and size of
 * new face coordinates are stable.
 *
 *  @af: internal AF data structure
 *
 **/
static void fdprio_check_face_stability(
  fdprio_t *fdprio_data,
  fdprio_face_info_t *this_face,
  mct_face_data_t *cur_update,
  boolean *is_loc_stable,
  boolean *is_size_stable)
{
  int32_t new_roi_top_left_x = 0, new_roi_top_left_y;
  int32_t new_ctr_x = 0, new_ctr_y = 0;
  boolean loc_stability = FALSE, size_stability = FALSE;
  cam_rect_t *new_roi = &cur_update->roi;

  new_ctr_x = new_roi->left + new_roi->width / 2;
  new_ctr_y = new_roi->top + new_roi->height/ 2;

  new_roi_top_left_x = new_roi->left;
  new_roi_top_left_y = new_roi->top;


  /* Location stability check:
     * This is to detect the movement of face in horizontal and vertical plane.
     * For location stability we'll check by how much center of current face changes
     * from previous face in x-axis and y-axis */
  loc_stability = fdprio_check_loc_stability(
    fdprio_data, this_face, new_ctr_x, new_ctr_y);

  /* Size stability check:
     * This is to detect the movement towards and away from the camera (z-axis)
     * For size stability, we'll calculate a simple delta of top-left coordinates of current
     * and previous faces. Then percentage change of this delta will be compared with
     * our threshold to determine size stability. */
  size_stability = fdprio_check_size_stability(
    fdprio_data, this_face, new_roi_top_left_x, new_roi_top_left_y);

  AF_LOW("Stability check: Location: %d Size: %d",
    loc_stability, size_stability);
  /* Params that need to be output */
  *is_loc_stable = loc_stability;
  *is_size_stable = size_stability;
}
/** fdprio_process_fd_roi
 * This is the entry function to process face detection.
 *
 * @fdprio_data: Internal control data for fd priority
 *
 **/

static boolean fdprio_process_fd_roi(fdprio_t *fdprio_data) {
  boolean rc = FALSE;
  mct_face_info_t *face_info;
  int32_t curr_biggest_face_id = -1;
  int32_t curr_biggest_face_index = 0;
  int32_t active_face_index = 0;
  boolean loc_stability = FALSE, size_stability = FALSE;
  uint32_t loc_stability_cnt = 0, size_stability_cnt = 0;
  boolean roi_stability = FALSE;
  int32_t face_id_ret = 0;
  af_fd_priority_caf_adapter_t *fd_tuning =
    &fdprio_data->tuning_info.af_algo.fd_prio;
  int stable_count_pos = fd_tuning->stable_count_pos;
  int stable_count_size = fd_tuning->stable_count_size;

  /* Adjust stability count threshold based on current fps */
  stable_count_pos = fdprio_adjust_stability_count(fdprio_data,
    fd_tuning->stable_count_pos);
  stable_count_size = fdprio_adjust_stability_count(fdprio_data,
    fd_tuning->stable_count_size);

  AF_LOW("Process FD_ROI data");

  if(!fdprio_data->pface_info) {
    AF_ERR("Null pointer passed for face_info");
    return FALSE;
  }

  face_info = fdprio_data->pface_info;
  fdprio_data->faces_detected = face_info->face_count;
  AF_LOW("Number of faces detected: %d",
    fdprio_data->faces_detected);


  fdprio_dbg_print_face_dim(fdprio_data);

  /* Check if faces are still detected. If no faces are detected for
       number of frames, we'll revert back to default ROI */
  fdprio_process_counters_int(fdprio_data);

  if(!fdprio_data->faces_detected) {
    memset(&fdprio_data->active_face, 0, sizeof(fdprio_face_info_t));
    AF_LOW("Face_count: 0");
    return TRUE;
  }

  /* Find the biggest face */
  if (face_info->region_priority)
    curr_biggest_face_id = 0;
  else {
    curr_biggest_face_id = fdprio_find_biggest_face(face_info);
    AF_LOW("Biggest Face-id: %d", curr_biggest_face_id);
  }
  curr_biggest_face_index = fdprio_get_index_by_id(fdprio_data,
    curr_biggest_face_id);

  /* faces[] array is of size MAX_ROI */
  if (curr_biggest_face_index == FDPRIO_INDEX_INVALID || curr_biggest_face_index >= MAX_ROI) {
    curr_biggest_face_index = 0;
  }
  AF_LOW("Index of current biggest face: %d",
    curr_biggest_face_index);
  rc = fdprio_is_new_face_big_enough(fdprio_data,
    curr_biggest_face_id);
  face_id_ret = fdprio_get_index_by_id(fdprio_data,
    fdprio_data->active_face.face_id);

  if (((fdprio_data->active_face.face_id !=  curr_biggest_face_id) && (rc))
     ||
    ((FDPRIO_INDEX_INVALID == face_id_ret) && (fdprio_data->active_face.face_id != 0))){

    /* Either this is a new face detected for the first time, OR A "larger" face is detected then
        * the previously tracked active face. */
    AF_LOW("Active face is not big enough. Or this is the only face."
      "Active face id:%d, curr_biggest_face_id: %d",
       fdprio_data->active_face.face_id, curr_biggest_face_id);

    fdprio_update_face_info(&fdprio_data->active_face,
      &face_info->faces[curr_biggest_face_index], 0, 0);

    AF_LOW("Set trigger_search for New Active Face");
    fdprio_data->trigger_search = TRUE;
    loc_stability                  = FALSE;
    size_stability                 = FALSE;
    active_face_index              = curr_biggest_face_index;
  } else {
    /* Check face stability and increase the count to compare to tuned value.*/
    AF_LOW("Check stability of active face (id: %d)",
      fdprio_data->active_face.face_id);
    active_face_index = fdprio_get_index_by_id(fdprio_data,
      fdprio_data->active_face.face_id);

    if (active_face_index == FDPRIO_INDEX_INVALID || active_face_index >= MAX_ROI) {
      active_face_index = 0;
    }
    fdprio_check_face_stability(
      fdprio_data,
      &fdprio_data->active_face,
      &face_info->faces[active_face_index],
      &loc_stability,
      &size_stability);

    if (loc_stability == TRUE) {
      fdprio_data->active_face.loc_stability_cnt++;
    }
    if (size_stability == TRUE) {
      fdprio_data->active_face.size_stability_cnt++;
    }
    /* update the face struct with active face data in current frame. This is needed to compare
        * the face information of next frame with the current frame face info */
    fdprio_update_face_info(&fdprio_data->active_face,
      &face_info->faces[active_face_index],
      fdprio_data->active_face.loc_stability_cnt,
      fdprio_data->active_face.size_stability_cnt);
  }

  /* Also we need to check if current face has moved significantly away from the
     * last configured ROI. Sometime if face moves very slowly we may not detect
     * it has moved and assume it is stable. However after sometime face ROI might
     * have changed significantly even without triggering instability */
  if ((loc_stability == TRUE) &&
    (fdprio_data->trigger_search == FALSE)) {
    roi_stability = fdprio_compare_with_ref_roi(
      fdprio_data,
      &fdprio_data->cur_confgd_roi,
      &face_info->faces[active_face_index].roi,
      fdprio_data->camif_width);
    if (roi_stability == FALSE) {
      AF_LOW("Set trigger_search for Diff between Ref and Current ROI!");
      fdprio_data->trigger_search = TRUE;
      loc_stability_cnt = 0;
    }
  }

  /* Now check both location and size stability to determine whether we need to refocus
     * or not */
  if ((fdprio_data->trigger_search == TRUE) &&
    (fdprio_data->active_face.loc_stability_cnt >
    (uint32_t) stable_count_pos) &&
    (fdprio_data->active_face.size_stability_cnt >
    (uint32_t) stable_count_size)) {
    AF_LOW("We have stable ROI! Send it to AF library!");
    memcpy(&fdprio_data->cur_confgd_roi,
      &face_info->faces[active_face_index].roi,
      sizeof(cam_rect_t));
    fdprio_send_current_roi(fdprio_data);
    fdprio_data->trigger_search = FALSE;
    fdprio_scn_chg_rebase_ref(&(fdprio_data->scn_mtr));
  }
  return TRUE;
}

/** fdprio_generate_sad:
 * Function to generate sad for scene change detection.
 *
 *  @af: internal AF data structure
 *
 **/

float fdprio_generate_sad(unsigned int *luma, int *ref_luma, int num_region)
{
  float sad = 0;
  int i, j, diff;
  if (num_region <= 0) {
    return sad;
  }
  int af_num_divisions_at_each_side = (int)sqrt(num_region);
  int region_start = af_num_divisions_at_each_side * 0.25;/* ie 4 */
  int region_end = af_num_divisions_at_each_side * 0.75; /* ie 12 */
  int af_num_focus_regions = abs((region_start - region_end) *
                             (region_start - region_end));

  for (i = region_start; i < region_end; i++) {
    for (j = region_start; j < region_end; j++) {
      int region = (i * af_num_divisions_at_each_side + j);
      diff = (int)(luma[region] - ref_luma[region]);
      sad += (float)abs(diff);
    }
  }

  AF_MSG_LOW(" (before scaling)sad %f", sad);

  /* normalize SAD value */
  sad = (sad / (float)af_num_focus_regions);

  return sad;
}

/** fdprio_handle_aec_update:
 * Function to handle aec update.
 *
 *  @fdprio_data: internal FD PRIO data structure
 *  @SY_data: Luma Sum data from AEC.
 *
 **/
static boolean fdprio_handle_aec_update(fdprio_t *fdprio_data,
  fdprio_aec_set_parm_t *aec_info)
{
     /* TBD: Config Scene monitor */
  boolean rc = TRUE;
  fdprio_scene_monitor_t *scn_mtr =  &fdprio_data->scn_mtr;
  fdprio_data->preview_fps = aec_info->preview_fps;
  if (aec_info->SY_data->is_valid) {
    /* first time if prev_SY is not initialized*/
    if (!scn_mtr->prev_SY.is_valid) {
      memcpy(scn_mtr->prev_SY.SY, aec_info->SY_data->SY, sizeof(int) * MAX_YUV_STATS_NUM);
      scn_mtr->prev_SY.is_valid = aec_info->SY_data->is_valid;
      goto done;
    }
    fdprio_scn_chg_set_input_t input;
    input.type = FDPRIO_SCN_CHG_SAD;
    input.data =
      fdprio_generate_sad(aec_info->SY_data->SY, scn_mtr->prev_SY.SY, MAX_YUV_STATS_NUM);
    fdprio_scn_chg_set_input(scn_mtr, &input);

    memcpy(scn_mtr->prev_SY.SY, aec_info->SY_data->SY, sizeof(int) * MAX_YUV_STATS_NUM);
    scn_mtr->prev_SY.is_valid = aec_info->SY_data->is_valid;
  }
done:
    return rc;
}

/** fdprio_handle_gyro_data:
 * Function to handle gyro update.
 *
 *  @fdprio_data: internal FD PRIO data structure
 *  @gyro_sqr: Luma Sum data from AEC.
 *
 **/
static boolean fdprio_handle_gyro_data(fdprio_t *fdprio_data, float gyro_sqr)
{
  boolean rc = TRUE;
  fdprio_scene_monitor_t *scn_mtr =  &fdprio_data->scn_mtr;
  fdprio_scn_chg_set_input_t input;
  input.type = FDPRIO_SCN_CHG_GYRO;
  input.data = gyro_sqr;
  fdprio_scn_chg_set_input(scn_mtr, &input);
  return rc;
}

/** fdprio_find_biggest_face:
 * Function to find the biggest face from an array of faces.
 *
 *  @mct_face_info_t: Face roi information
 *
 **/
static int32_t fdprio_find_biggest_face(mct_face_info_t *face_info) {
  int32_t biggest_face = 0;
  cam_rect_t *biggest_fd_roi;
  int32_t i;

  biggest_fd_roi = &face_info->faces[0].roi;
  biggest_face = face_info->faces[0].face_id;

  for(i = 1; i < face_info->face_count; i++) {
    if(biggest_fd_roi->width < face_info->faces[i].roi.width ||
        biggest_fd_roi->height < face_info->faces[i].roi.height) {
      biggest_fd_roi = &face_info->faces[i].roi;
      biggest_face = face_info->faces[i].face_id;
    }
  }
  AF_LOW("Biggest Face: ID: %d x: %d y: %d dx: %d dy: %d",
    biggest_face, biggest_fd_roi->left, biggest_fd_roi->top,
    biggest_fd_roi->width, biggest_fd_roi->height);

  return biggest_face;
}

static boolean fdprio_is_tracked_face_present(
  fdprio_t *fdprio_data)
{
  int32_t i;
  mct_face_info_t *face_info;

  face_info = fdprio_data->pface_info;

  for(i = 0; i < face_info->face_count; i++) {
    /* compare with the curr_biggest_face_id field because this is the id */
    /* we have been tracking so far */
    if(face_info->faces[i].face_id == fdprio_data->active_face.face_id) {
      return TRUE;
    }
  }

  return FALSE;
}

static int32_t fdprio_get_index_by_id(fdprio_t *fdprio_data,
  int32_t face_id) {
  int32_t i;
  mct_face_info_t *face_info;
  int32_t index = FDPRIO_INDEX_INVALID;
  face_info = fdprio_data->pface_info;

  for(i = 0; i < face_info->face_count; i++) {
    if(face_info->faces[i].face_id == face_id) {
      index = i;
      break;
    }
  }

  AF_LOW("For Face-id: %d Index: %d", face_id, index);
  return index;
}

static boolean fdprio_is_new_face_big_enough(
  fdprio_t *fdprio_data,
  int32_t curr_biggest_face_id)
{
  boolean rc = TRUE;
  int32_t old_face_idx;
  int32_t new_face_idx;
  mct_face_info_t *face_info;
  int32_t old_face_size = 0, new_face_size = 0;
  cam_rect_t *old_face = NULL;
  cam_rect_t *new_face = NULL;
  float percent_change = 0.0;
  af_fd_priority_caf_adapter_t *fd_tuning =
    &fdprio_data->tuning_info.af_algo.fd_prio;

  face_info = fdprio_data->pface_info;

  old_face_idx = fdprio_get_index_by_id(fdprio_data,
    fdprio_data->active_face.face_id);
  new_face_idx = fdprio_get_index_by_id(fdprio_data,
    curr_biggest_face_id);

  if(old_face_idx == FDPRIO_INDEX_INVALID || old_face_idx >= MAX_ROI)
    return TRUE;
  if (new_face_idx == FDPRIO_INDEX_INVALID || new_face_idx >= MAX_ROI) {
    return FALSE;
  }

  old_face = &face_info->faces[old_face_idx].roi;
  new_face = &face_info->faces[new_face_idx].roi;

  old_face_size = sqrt((old_face->width * old_face->width) +
    (old_face->height * old_face->height)) / 2;

  new_face_size = sqrt((new_face->width * new_face->width) +
    (new_face->height * new_face->height)) / 2;

  percent_change = PERCENT_LOC_CHANGE(old_face_size, new_face_size,
    fdprio_data->camif_width);

  AF_LOW("Face Size: Old: %d New: %d Th: %f Percent_change: %f",
    old_face_size, new_face_size, fd_tuning->old_new_size_diff_th,
    percent_change);
  rc = (percent_change > fd_tuning->old_new_size_diff_th) ? TRUE : FALSE;

  return rc;
}


static boolean fdprio_send_default_roi(fdprio_t *fdprio_data) {
  boolean rc = FALSE;
  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg =
    fdprio_create_msg(MSG_AF_SET, AF_SET_PARAM_ROI, fdprio_data);
  if (af_msg == NULL) {
    return FALSE;
  }

  af_msg->u.af_set_parm.u.af_roi_info.roi_updated = TRUE;
  af_msg->u.af_set_parm.u.af_roi_info.frm_id = fdprio_data->current_frame_id;
  af_msg->u.af_set_parm.u.af_roi_info.num_roi = 0;
  af_msg->u.af_set_parm.u.af_roi_info.type = AF_ROI_TYPE_GENERAL;

  af_msg->u.af_set_parm.u.af_roi_info.roi[0].x = 0;
  af_msg->u.af_set_parm.u.af_roi_info.roi[0].y = 0;
  af_msg->u.af_set_parm.u.af_roi_info.roi[0].dx = 0;
  af_msg->u.af_set_parm.u.af_roi_info.roi[0].dy = 0;

  fdprio_data->cur_confgd_roi.left = 0;
  fdprio_data->cur_confgd_roi.top = 0;
  fdprio_data->cur_confgd_roi.width = 0;
  fdprio_data->cur_confgd_roi.height = 0;

  /* Clear our active face info */
  memset(&fdprio_data->active_face, 0, sizeof(fdprio_face_info_t));

  AF_LOW("Sending default ROI");
  rc = q3a_af_thread_en_q_msg(fdprio_data->thread_data, af_msg);

  return rc;
}

static boolean fdprio_send_current_roi(fdprio_t *fdprio_data) {
  boolean rc = FALSE;
  uint8_t roi_pos;
  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg =
    fdprio_create_msg(MSG_AF_SET, AF_SET_PARAM_ROI, fdprio_data);
  if (af_msg == NULL) {
    return FALSE;
  }

  af_msg->u.af_set_parm.u.af_roi_info.roi_updated = TRUE;
  af_msg->u.af_set_parm.u.af_roi_info.frm_id = fdprio_data->current_frame_id;
  af_msg->u.af_set_parm.u.af_roi_info.num_roi = 1;
  af_msg->u.af_set_parm.u.af_roi_info.type = AF_ROI_TYPE_FACE;

  af_msg->u.af_set_parm.u.af_roi_info.roi[0].x =
    fdprio_data->cur_confgd_roi.left;
  af_msg->u.af_set_parm.u.af_roi_info.roi[0].y =
    fdprio_data->cur_confgd_roi.top;
  af_msg->u.af_set_parm.u.af_roi_info.roi[0].dx =
    fdprio_data->cur_confgd_roi.width;
  af_msg->u.af_set_parm.u.af_roi_info.roi[0].dy =
    fdprio_data->cur_confgd_roi.height;


  AF_LOW("Sending New Face ROI: x: %d y: %d dx: %d dy: %d",
    fdprio_data->cur_confgd_roi.left, fdprio_data->cur_confgd_roi.top,
    fdprio_data->cur_confgd_roi.width,
    fdprio_data->cur_confgd_roi.height);
  rc = q3a_af_thread_en_q_msg(fdprio_data->thread_data, af_msg);

  return rc;
}
