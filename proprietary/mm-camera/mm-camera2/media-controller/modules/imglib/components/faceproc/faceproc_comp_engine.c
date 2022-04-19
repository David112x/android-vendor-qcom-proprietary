/**********************************************************************
*  Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "faceproc_comp.h"
#include "faceproc_sw_wrapper.h"
#include "faceproc_common.h"

/**
 * Function: faceproc_fd_output
 *
 * Description: Gets the frameproc output
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   p_fd_data - Faceproc result data
 *   p_num_faces - Pointer to number of faces
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int faceproc_fd_output(faceproc_comp_t *p_comp,
  faceproc_result_t *p_fd_data,
  int32_t *p_num_faces)
{
  int rc;

  rc = faceproc_sw_wrapper_get_fd_info(p_comp->p_wrapper,
    p_fd_data->roi, MAX_FACE_ROI, p_num_faces);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in getting fd info");
    return rc;
  }

  /* Parse and store the faces */
  p_fd_data->num_faces_detected = (uint32_t)*p_num_faces;
  p_fd_data->frame_id = p_comp->frame_id;

  if (p_fd_data->num_faces_detected > MAX_FACE_ROI)
    p_fd_data->num_faces_detected = MAX_FACE_ROI;

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_fd_execute
 *
 * Description: Executes the face detecttion algorithm
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   p_frame - pointer to input frame
 *   p_num_faces - number of faces
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int faceproc_fd_execute(faceproc_comp_t *p_comp, img_frame_t *p_frame,
  int32_t *p_num_faces)
{
  int rc;
  IDBG_HIGH("E %dx%d", FD_WIDTH(p_frame), FD_HEIGHT(p_frame));

  p_comp->frame_id = p_frame->frame_id;

  ATRACE_BEGIN_SNPRINTF(32, "FD_%d", p_frame->frame_id);

  rc = faceproc_sw_wrapper_execute_fd(p_comp->p_wrapper, p_frame, p_num_faces);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("sw_wrapper execute fd failed %d", rc);
    return rc;
  }

  ATRACE_END();

  if (p_comp->dump_mode != FACE_FRAME_DUMP_OFF) {
    int tracked = 0;
    int index;
    for (index = 0; index < *p_num_faces; index++) {
      tracked += faceproc_sw_wrapper_is_face_tracked(p_comp->p_wrapper, index);
    }

    faceproc_common_frame_dump(p_frame, p_comp->dump_mode,
      tracked, *p_num_faces);
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_eng_update_cfg
 *
 * Description: Update the faceproc config when
 *   there is a dynamic update
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_error otherwise
 *
 * Notes: none
 **/
int faceproc_comp_eng_update_cfg(faceproc_comp_t *p_comp)
{
  if (!p_comp->p_wrapper) {
    // If this event has come even before we create the fd wrappers,
    // we can ignore this. We will anyway do set config with the latest
    // information that we have when creating wrapper
    return IMG_SUCCESS;
  }
  p_comp->need_config_update = FALSE;
  int rc;
  uint32_t min_face_size = faceproc_common_get_face_size(
    p_comp->fd_chromatix.min_face_adj_type,
    p_comp->fd_chromatix.min_face_size,
    p_comp->fd_chromatix.min_face_size_ratio,
    FACEPROC_MIN_FACE_SIZE, /* restricting min facesize */
    MIN(p_comp->width, p_comp->height));

  IDBG_MED("new ###min_face_size %d", min_face_size);

  // Get the default/current config first
  fpsww_config_t sw_config;
  rc = faceproc_sw_wrapper_get_config(p_comp->p_wrapper, &sw_config);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("sw_wrapper get config failed");
    return rc;
  }

  if (p_comp->dynamic_cfg_params.lux_index != -1) {
    sw_config.dt_params.nThreshold = (int32_t)(p_comp->fd_chromatix.threshold +
      p_comp->fd_chromatix.dynamic_lux_config.lux_offsets
      [p_comp->dynamic_cfg_params.lux_index]);
    IDBG_MED("Threshold to algo %d", sw_config.dt_params.nThreshold);
  }

  // Update the config params as required/tuned

  // Update min, max sizes
  sw_config.dt_params.nMinSize = (int32_t)min_face_size;
  sw_config.dt_params.nMaxSize = (int32_t)p_comp->fd_chromatix.max_face_size;

  //serach cycle updates
  if ((p_comp->device_rotation == -1) ||
    (p_comp->fd_chromatix.enable_upfront == FALSE)) {
    sw_config.dt_params.nInitialFaceSearchCycle =
      p_comp->fd_chromatix.no_face_search_cycle +
      p_comp->fd_chromatix.search_cycle_dyn_increase;
    sw_config.dt_params.nNewFaceSearchCycle =
      p_comp->fd_chromatix.new_face_search_cycle +
      p_comp->fd_chromatix.search_cycle_dyn_increase;
  } else {
    sw_config.dt_params.nInitialFaceSearchCycle =
      p_comp->fd_chromatix.no_face_search_cycle;
    sw_config.dt_params.nNewFaceSearchCycle =
      p_comp->fd_chromatix.new_face_search_cycle ;
  }


  // Rotation angles
  sw_config.dt_params.nAngle[FPSWW_ANGLE_FRONT] =
    faceproc_common_get_angle(p_comp->fd_chromatix.angle_front,
    p_comp->fd_chromatix.enable_upfront,
    p_comp->fd_chromatix.upfront_angle_front_profile,
    p_comp->device_rotation);
  sw_config.dt_params.nAngle[FPSWW_ANGLE_HALFPROFILE] =
    faceproc_common_get_angle(p_comp->fd_chromatix.angle_half_profile,
    p_comp->fd_chromatix.enable_upfront,
    p_comp->fd_chromatix.upfront_angle_half_profile,
    p_comp->device_rotation);
  sw_config.dt_params.nAngle[FPSWW_ANGLE_FULLPROFILE] =
    faceproc_common_get_angle(p_comp->fd_chromatix.angle_full_profile,
    p_comp->fd_chromatix.enable_upfront,
    p_comp->fd_chromatix.upfront_angle_full_profile,
    p_comp->device_rotation);

  // Update facial parts features
  boolean enable_face_landmarks;
  boolean enable_contour, enable_smile, enable_blink, enable_gaze;

  faceproc_common_get_feature_flags(&p_comp->config, &p_comp->fd_chromatix,
    &enable_face_landmarks, &enable_contour,
    &enable_smile, &enable_blink, &enable_gaze);

  // features
  sw_config.enable_facial_parts = enable_face_landmarks;
  sw_config.enable_contour = enable_contour;
  sw_config.enable_smile = enable_smile;
  sw_config.enable_gaze  = enable_gaze;
  sw_config.enable_blink = enable_blink;

  rc = faceproc_sw_wrapper_set_config(p_comp->p_wrapper, &sw_config);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("sw_wrapper set config failed");
    return rc;
  }

  return rc;
}

/**
 * Function: faceproc_comp_eng_reconfig_core
 *
 * Description: Re-Configure the faceproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   reset_results - Whether to reset the current detection results
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_reconfig_core(faceproc_comp_t *p_comp,
  bool reset_results)
{
  IDBG_LOW("Enter: %p", (void *)p_comp);

  uint32_t an_still_angle[POSE_TYPE_COUNT];
  int rc = IMG_SUCCESS;
  faceproc_config_t *p_cfg = &p_comp->config;
  uint32_t min_face_size;

  // Get the default/current config first
  fpsww_config_t sw_config;
  rc = faceproc_sw_wrapper_get_config(p_comp->p_wrapper, &sw_config);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("sw_wrapper get config failed");
    return rc;
  }

  /* Get the tuning params from chromatix */

  // minimum face size
  min_face_size = faceproc_common_get_face_size(
    p_comp->fd_chromatix.min_face_adj_type,
    p_comp->fd_chromatix.min_face_size,
    p_comp->fd_chromatix.min_face_size_ratio,
    FACEPROC_MIN_FACE_SIZE, /* restricting min facesize */
    MIN(p_cfg->frame_cfg.max_width, p_cfg->frame_cfg.max_height));

  IDBG_MED("###min_face_size %d", min_face_size);

  /* Minimum face size to be detected should be at most half the
    height of the input frame */
  if (min_face_size > (p_cfg->frame_cfg.max_height / 2)) {
    IDBG_WARN("Error, min face size to detect is greater than "
      "half the height of the input frame");
  }

  // POSE angles
  if (FD_ANGLE_ENABLE(&p_comp->config)) {
    an_still_angle[FPSWW_ANGLE_FRONT] =
      faceproc_common_get_angle(p_comp->fd_chromatix.angle_front,
      p_comp->fd_chromatix.enable_upfront,
      p_comp->fd_chromatix.upfront_angle_front_profile,
      p_comp->device_rotation);
    an_still_angle[FPSWW_ANGLE_HALFPROFILE] =
      faceproc_common_get_angle(p_comp->fd_chromatix.angle_half_profile,
      p_comp->fd_chromatix.enable_upfront,
      p_comp->fd_chromatix.upfront_angle_half_profile,
      p_comp->device_rotation);
    an_still_angle[FPSWW_ANGLE_FULLPROFILE] =
      faceproc_common_get_angle(p_comp->fd_chromatix.angle_full_profile,
      p_comp->fd_chromatix.enable_upfront,
      p_comp->fd_chromatix.upfront_angle_full_profile,
      p_comp->device_rotation);
  } else {
    IDBG_MED("###Disable Angle");
    an_still_angle[FPSWW_ANGLE_FRONT] = ROLL_ANGLE_NONE;
    an_still_angle[FPSWW_ANGLE_HALFPROFILE] = ROLL_ANGLE_NONE;
    an_still_angle[FPSWW_ANGLE_FULLPROFILE] = ROLL_ANGLE_NONE;
  }

  uint32_t search_density = faceproc_common_validate_and_get_density(
    true, p_comp->fd_chromatix.search_density_nontracking, 0);

  // Update the config params as required/tuned

  boolean enable_face_landmarks;
  boolean enable_contour, enable_smile, enable_blink, enable_gaze;
  faceproc_common_get_feature_flags(&p_comp->config, &p_comp->fd_chromatix,
    &enable_face_landmarks, &enable_contour,
    &enable_smile, &enable_blink, &enable_gaze);

  // features
  sw_config.enable_facial_parts = enable_face_landmarks;
  sw_config.enable_contour = enable_contour;
  sw_config.enable_smile = enable_smile;
  sw_config.enable_gaze  = enable_gaze;
  sw_config.enable_blink = enable_blink;

  /************* DT Params *************/
  sw_config.dt_params.nMinSize = min_face_size;
  sw_config.dt_params.nMaxSize = (int32_t)p_comp->fd_chromatix.max_face_size;

  sw_config.dt_params.nAngle[FPSWW_ANGLE_FRONT] =
    an_still_angle[FPSWW_ANGLE_FRONT];
  sw_config.dt_params.nAngle[FPSWW_ANGLE_HALFPROFILE] =
    an_still_angle[FPSWW_ANGLE_HALFPROFILE];
  sw_config.dt_params.nAngle[FPSWW_ANGLE_FULLPROFILE] =
    an_still_angle[FPSWW_ANGLE_FULLPROFILE];

  sw_config.dt_params.nThreshold = (int32_t)p_comp->fd_chromatix.threshold;

  sw_config.dt_params.nSearchDensity = search_density;
  sw_config.dt_params.bMask = (int32_t)p_comp->fd_chromatix.direction;

  // Modify config as per the chromatix request
  if ((p_comp->device_rotation == -1) ||
    (p_comp->fd_chromatix.enable_upfront == FALSE)) {
    sw_config.dt_params.nInitialFaceSearchCycle =
      p_comp->fd_chromatix.no_face_search_cycle +
      p_comp->fd_chromatix.search_cycle_dyn_increase;
    sw_config.dt_params.nNewFaceSearchCycle =
      p_comp->fd_chromatix.new_face_search_cycle +
      p_comp->fd_chromatix.search_cycle_dyn_increase;
  } else {
    sw_config.dt_params.nInitialFaceSearchCycle =
      p_comp->fd_chromatix.no_face_search_cycle;
    sw_config.dt_params.nNewFaceSearchCycle =
      p_comp->fd_chromatix.new_face_search_cycle ;
  }
  sw_config.dt_params.nNewFaceSearchInterval =
    p_comp->fd_chromatix.refresh_count;

  sw_config.dt_params.nDelayCount = p_comp->fd_chromatix.delay_count;
  sw_config.dt_params.nMaxHoldCount = p_comp->fd_chromatix.hold_count;
  sw_config.dt_params.nMaxRetryCount = p_comp->fd_chromatix.hold_count + 1;
  sw_config.dt_params.nAccuracy = p_comp->fd_chromatix.accuracy;
  sw_config.dt_params.nPosSteadinessParam =
    p_comp->fd_chromatix.pos_steadiness_param;
  sw_config.dt_params.nSizeSteadinessParam =
    p_comp->fd_chromatix.size_steadiness_param;
  sw_config.dt_params.rollAngleExtension =
    p_comp->fd_chromatix.roll_angle_extension;
  sw_config.dt_params.yawAngleExtension =
    p_comp->fd_chromatix.yaw_angle_extension;

  /************* PT Params *************/
  sw_config.pt_params.nMode = PT_MODE_DEFAULT;
  sw_config.pt_params.nConfMode = PT_CONF_USE;

  /************* CT Params *************/
  sw_config.ct_params.nMode = p_comp->fd_chromatix.ct_detection_mode;

  /************* Wrapper filter params *************/
  sw_config.filter_params.lock_faces = p_comp->fd_chromatix.lock_faces;
  sw_config.filter_params.discard_facialparts_threshold =
    p_comp->fd_chromatix.facial_parts_threshold;

  rc = faceproc_sw_wrapper_set_config(p_comp->p_wrapper, &sw_config);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("sw_wrapper set config failed");
    return rc;
  }

  if (reset_results == TRUE) {
    rc = faceproc_sw_wrapper_reset_result(p_comp->p_wrapper);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("sw_wrapper reset results failed");
      return rc;
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_eng_config
 *
 * Description: Configure the faceproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_config(faceproc_comp_t *p_comp)
{
  IDBG_LOW("Enter: %p", (void *)p_comp);

  if (!p_comp) {
    IDBG_ERROR("NULL component");
    return IMG_ERR_GENERAL;
  }

  int rc = IMG_SUCCESS;
  uint32_t max_num_face_to_detect =
    faceproc_get_max_number_to_detect(&p_comp->fd_chromatix);

  // Create sw_wrapper handle
  fpsww_create_params_t create_params;
  create_params.engine = FACEPROC_ENGINE_SW;
  create_params.create_face_parts = TRUE;
  create_params.create_face_recog = FALSE;
  create_params.max_face_count = max_num_face_to_detect;
  create_params.detection_mode =
    (p_comp->fd_chromatix.detection_mode == FD_CHROMATIX_MODE_MOTION_FULL) ?
    DETECTION_MODE_STILL : DETECTION_MODE_MOVIE;
  create_params.no_of_fp_handles = MAX_FACE_ROI;
  create_params.use_dsp_if_available = FALSE;

  p_comp->p_wrapper = faceproc_sw_wrapper_create(&create_params);
  if (!p_comp->p_wrapper) {
    IDBG_ERROR("sw_wrapper creation failed");
    return IMG_ERR_GENERAL;
  }

  faceproc_sw_wrapper_set_debug(p_comp->p_wrapper, &p_comp->debug_settings);

  rc = faceproc_comp_eng_reconfig_core(p_comp, FALSE);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("failed in configuring sw_wrapper");
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_eng_exec
 *
 * Description: main algorithm execution function for face processing
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   p_frame - Input frame
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_exec(faceproc_comp_t *p_comp,
  img_frame_t *p_frame)
{
  int32_t num_faces;
  int rc = IMG_SUCCESS;

  switch (p_comp->mode) {
    case FACE_DETECT_BSGC:
    case FACE_DETECT:
    case FACE_DETECT_LITE:
      if (p_comp->is_chromatix_changed == TRUE) {
        p_comp->is_chromatix_changed = FALSE;
        rc = faceproc_comp_eng_reconfig_core(p_comp, TRUE);
        if (IMG_ERROR(rc)) {
          IDBG_ERROR("Failed in faceproc_comp_eng_reconfig_core, rc = %d", rc);
        }
      }

      if (p_comp->need_config_update == TRUE) {
        faceproc_comp_eng_update_cfg(p_comp);
      }

      if (IMG_SUCCEEDED(rc)) {
        rc = faceproc_fd_execute(p_comp, p_frame, &num_faces);
        if (IMG_ERROR(rc))
          IDBG_ERROR("Failed in faceproc_fd_execute, rc = %d", rc);
      }
      break;
    case FACE_RECOGNIZE:
    case FACE_REGISTER:
    default :
      IDBG_ERROR("MODE not selected/recognized");
      rc = IMG_ERR_NOT_SUPPORTED;
  }

  return rc;
}

/**
 * Function: faceproc_comp_eng_get_output
 *
 * Description: Get the output from the frameproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   p_fd_data - Input frame
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_get_output(faceproc_comp_t *p_comp,
  faceproc_result_t *p_fd_data)
{
  int32_t num_faces;
  int status;

  IDBG_LOW("p_comp:%p, p_res:%p", p_comp, p_fd_data);

  switch (p_comp->mode) {
    case FACE_DETECT_BSGC:
    case FACE_DETECT:
    case FACE_DETECT_LITE:
      status = faceproc_fd_output(p_comp, p_fd_data, &num_faces);
      if (IMG_ERROR(status)) {
        IDBG_ERROR("Failed in faceproc_fd_output, rc = %d", status);
      }
      break;

    case FACE_RECOGNIZE:
    case FACE_REGISTER:
    default:
      IDBG_ERROR("Unsupported mode selected");
      status = IMG_ERR_INVALID_INPUT;
  }

  IDBG_LOW("After rc: %d, p_comp:%p, p_res:%p",
    status, p_comp, p_fd_data);

  return status;
}

/**
 * Function: faceproc_comp_eng_destroy
 *
 * Description: Destroy the faceproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_destroy(faceproc_comp_t *p_comp)
{
  int rc;

  if (!p_comp)
    return IMG_ERR_GENERAL;

  IDBG_LOW("faceproc engine clean p_comp=%p, p_comp->p_wrapper=%p",
    p_comp, p_comp->p_wrapper);

  if (p_comp->p_wrapper) {
    rc = faceproc_sw_wrapper_destroy(p_comp->p_wrapper);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("sw_wrapper destroy failed %d", rc);
    }
    p_comp->p_wrapper = NULL;
  }

  return IMG_SUCCESS;
}

