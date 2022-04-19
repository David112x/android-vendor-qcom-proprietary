/***************************************************************************
Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
***************************************************************************/

#include "faceproc_dsp_comp.h"
#include "faceproc_common.h"
#ifdef FACEPROC_USE_NEW_WRAPPER
#include "faceproc_sw_wrapper.h"
#else
#include "facial_parts_wrapper.h"
#endif
#include <math.h>
#include <dlfcn.h>
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#include <cutils/trace.h>
#include "AEEStdErr.h"

#include "img_thread.h"
#include "img_dsp_dl_mgr.h"

#define PI 3.14159265

extern faceproc_dsp_lib_t g_faceproc_dsp_lib;

/**
 * Function: faceproc_dsp_comp_eng_load
 *
 * Description: Loads the faceproc library
 *
 * Input parameters:
 *   p_lib - The pointer to the faceproc lib object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_dsp_comp_eng_load(faceproc_dsp_lib_t *p_lib)
{
  int rc;
  rc = faceproc_sw_wrapper_load(TRUE);
  if (IMG_SUCCEEDED(rc)) {
    p_lib->lib_loaded = 1;
  }

  p_lib->status_dsp_lib = 1;  /* FD DSP lib loaded */

  return rc;
}

/**
 * Function: faceproc_dsp_comp_eng_unload
 *
 * Description: Unload the faceproc library
 *
 * Input parameters:
 *   p_lib - The pointer to the faceproc lib object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
void faceproc_dsp_comp_eng_unload(faceproc_dsp_lib_t *p_lib)
{
  faceproc_sw_wrapper_unload(TRUE);
}

/**
 * Function: faceproc_fd_output
 *
 * Description: Gets the frameproc output
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   fd_data - Faceproc result data
 *   num_faces - Number of faces
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_fd_output(faceproc_dsp_comp_t *p_comp,
  img_frame_t *p_frame, faceproc_result_t *p_fd_data,
  INT32 *p_num_faces)
{
  int rc;

  rc = faceproc_sw_wrapper_get_fd_info(p_comp->p_sw_wrapper,
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
 *   num_faces - number of faces
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_fd_execute(faceproc_dsp_comp_t *p_comp,
  img_frame_t *p_frame, INT32 * p_num_faces)
{
  int rc;
  IDBG_HIGH("E %dx%d", FD_WIDTH(p_frame), FD_HEIGHT(p_frame));

  p_comp->frame_id = p_frame->frame_id;

  ATRACE_BEGIN_SNPRINTF(32, "FD_DSP_%d", p_frame->frame_id);

  rc = faceproc_sw_wrapper_execute_fd(p_comp->p_sw_wrapper,
    p_frame, p_num_faces);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("sw_wrapper execute fd failed %d", rc);
    return rc;
  }

  ATRACE_END();

  if (p_comp->dump_mode != FACE_FRAME_DUMP_OFF) {
    int tracked = 0;
    int index;
    for (index = 0; index < *p_num_faces; index++) {
      tracked += faceproc_sw_wrapper_is_face_tracked(
        p_comp->p_sw_wrapper, index);
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
int faceproc_dsp_comp_eng_update_cfg(faceproc_dsp_comp_t *p_comp)
{
  if (!p_comp->p_sw_wrapper) {
    // If this event has come even before we create the fd wrappers,
    // we can ignore this. We will anyway do set config with the latest
    // information that we have when creating wrapper
    return IMG_SUCCESS;
  }

  int rc;
  p_comp->need_config_update = FALSE;
  uint32_t min_face_size = faceproc_common_get_face_size(
    p_comp->fd_chromatix.min_face_adj_type,
    p_comp->fd_chromatix.min_face_size,
    p_comp->fd_chromatix.min_face_size_ratio,
    FACEPROC_MIN_FACE_SIZE, /* restricting min facesize */
    MIN(p_comp->width, p_comp->height));

  IDBG_MED("new ###min_face_size %d", min_face_size);

  // Get the default/current config first
  fpsww_config_t sw_config;
  rc = faceproc_sw_wrapper_get_config(p_comp->p_sw_wrapper, &sw_config);
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

  rc = faceproc_sw_wrapper_set_config(p_comp->p_sw_wrapper, &sw_config);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("sw_wrapper set config failed");
    return rc;
  }

  return rc;
}

/**
 * Function: faceproc_dsp_comp_eng_config
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
int faceproc_dsp_comp_eng_config(faceproc_dsp_comp_t *p_comp)
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
  create_params.engine = FACEPROC_ENGINE_DSP;
  create_params.create_face_parts = TRUE;
  create_params.create_face_recog = FALSE;
  create_params.max_face_count = max_num_face_to_detect;
  create_params.detection_mode =
    (p_comp->fd_chromatix.detection_mode == FD_CHROMATIX_MODE_MOTION_FULL) ?
    DETECTION_MODE_STILL : DETECTION_MODE_MOVIE;
  create_params.no_of_fp_handles = MAX_FACE_ROI;
  create_params.use_dsp_if_available = TRUE;

  p_comp->p_sw_wrapper = faceproc_sw_wrapper_create(&create_params);
  if (!p_comp->p_sw_wrapper) {
    IDBG_ERROR("sw_wrapper creation failed");
    return IMG_ERR_GENERAL;
  }

  faceproc_sw_wrapper_set_debug(p_comp->p_sw_wrapper, &p_comp->debug_settings);

  rc = faceproc_dsp_comp_eng_reconfig_core(p_comp, FALSE);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("failed in configuring sw_wrapper");
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_dsp_comp_eng_reconfig_core
 *
 * Description: Re-Configure the faceproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   reset_results - whether to reset current results
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_dsp_comp_eng_reconfig_core(faceproc_dsp_comp_t *p_comp,
  bool reset_results)
{
  IDBG_LOW("Enter: %p", (void *)p_comp);

  uint32_t an_still_angle[POSE_TYPE_COUNT];
  int rc = IMG_SUCCESS;
  faceproc_config_t *p_cfg = &p_comp->config;
  uint32_t min_face_size;

  // Get the default/current config first
  fpsww_config_t sw_config;
  rc = faceproc_sw_wrapper_get_config(p_comp->p_sw_wrapper, &sw_config);
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

  /************* DSP Params *************/
  if ((p_comp->fd_chromatix.dsp_clock && p_comp->fd_chromatix.dsp_bus) ||
    (p_comp->fd_chromatix.dsp_dcvs_clock && p_comp->fd_chromatix.dcvs_enable
    && p_comp->fd_chromatix.dsp_dcvs_min_clock)) {
    sw_config.dsp_params.absapi = p_comp->fd_chromatix.dsp_absapi;
    sw_config.dsp_params.clock = p_comp->fd_chromatix.dsp_clock;
    sw_config.dsp_params.bus = p_comp->fd_chromatix.dsp_bus;
    sw_config.dsp_params.latency = p_comp->fd_chromatix.dsp_latency;
    sw_config.dsp_params.dcvs_clock = p_comp->fd_chromatix.dsp_dcvs_clock;
    sw_config.dsp_params.dcvs_enable = p_comp->fd_chromatix.dcvs_enable;
    sw_config.dsp_params.dcvs_min_clock =
      p_comp->fd_chromatix.dsp_dcvs_min_clock;
  }

  /************* PT Params *************/
  sw_config.pt_params.nMode = PT_MODE_DEFAULT;
  sw_config.pt_params.nConfMode = PT_CONF_USE;

  /************* CT Params *************/
  sw_config.ct_params.nMode = p_comp->fd_chromatix.ct_detection_mode;

  /************* Wrapper filter params *************/
  sw_config.filter_params.lock_faces = p_comp->fd_chromatix.lock_faces;
  sw_config.filter_params.discard_facialparts_threshold =
    p_comp->fd_chromatix.facial_parts_threshold;

  rc = faceproc_sw_wrapper_set_config(p_comp->p_sw_wrapper, &sw_config);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("sw_wrapper set config failed");
    return rc;
  }

  if (reset_results == TRUE) {
    rc = faceproc_sw_wrapper_reset_result(p_comp->p_sw_wrapper);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("sw_wrapper reset results failed");
      return rc;
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_dsp_comp_eng_reset
 *
 * Description: Reset the faceproc engine
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
int faceproc_dsp_comp_eng_reset(faceproc_dsp_comp_t *p_comp)
{
  int rc;

  if (!p_comp)
    return IMG_ERR_GENERAL;

  IDBG_LOW("faceproc engine clean p_comp=%p, p_comp->p_sw_wrapper=%p",
    p_comp, p_comp->p_sw_wrapper);

  if (p_comp->p_sw_wrapper) {
    rc = faceproc_sw_wrapper_destroy(p_comp->p_sw_wrapper);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("sw_wrapper destroy failed %d", rc);
    }
    p_comp->p_sw_wrapper = NULL;
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_dsp_comp_eng_test_dsp_connection
 *
 * Description: to call test function to find if DSP working
 * well
 *
 * Input parameters:
 *   p_lib - The pointer to the faceproc lib object
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_dsp_comp_eng_test_dsp_connection(faceproc_dsp_lib_t *p_lib)
{
  return IMG_SUCCESS;
}


