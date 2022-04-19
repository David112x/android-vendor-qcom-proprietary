/**********************************************************************
*  Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "faceproc_common.h"

/*
 * Function: faceproc_common_get_roll_angle.
 *
 * Description: Get the Roll angle and optional adjacent angles
 *
 * Input parameters:
 *   angle - actual angle.
 *   adjacent - angle extention level
 *       0: no adjacent angles,
 *       1: 1 adjacent angle on each side,
 *       2: 2 adjacent angle on each side,
 *
 * Return values:
 *    angle mask value
 *
 * Notes: none
 **/
unsigned int faceproc_common_get_roll_angle(int angle, int adjacent)
{
  int angle_mask = 0;

  if (angle < 0) {
    // unknown angle or we can't use angle information
    return ROLL_ANGLE_ALL;
  }

  switch (adjacent) {
  case 0: // pick one angle
    if ((angle < 15) || (angle >= 345)) {
      angle_mask = ROLL_ANGLE_0;
    } else if (angle < 45) {
      angle_mask = ROLL_ANGLE_1;
    } else if (angle < 75 ) {
      angle_mask = ROLL_ANGLE_2;
    } else if (angle < 105) {
      angle_mask = ROLL_ANGLE_3;
    } else if (angle < 135){
      angle_mask = ROLL_ANGLE_4;
    } else if (angle < 165) {
      angle_mask = ROLL_ANGLE_5;
    } else if (angle < 195) {
      angle_mask = ROLL_ANGLE_6;
    } else if (angle < 225) {
      angle_mask = ROLL_ANGLE_7;
    } else if (angle < 255) {
      angle_mask = ROLL_ANGLE_8;
    } else if (angle < 285) {
      angle_mask = ROLL_ANGLE_9;
    } else if (angle < 315) {
      angle_mask = ROLL_ANGLE_10;
    } else if (angle < 345) {
      angle_mask = ROLL_ANGLE_11;
    }
    break;
  case 1: // pick 1 adj angle
    if ((angle < 15) || (angle >= 345)) {
      angle_mask = ROLL_ANGLE_11 | ROLL_ANGLE_0 | ROLL_ANGLE_1;
    } else if (angle < 45) {
      angle_mask = ROLL_ANGLE_0 | ROLL_ANGLE_1 | ROLL_ANGLE_2;
    } else if (angle < 75 ) {
      angle_mask = ROLL_ANGLE_1 | ROLL_ANGLE_2 | ROLL_ANGLE_3;
    } else if (angle < 105) {
      angle_mask = ROLL_ANGLE_2 | ROLL_ANGLE_3 | ROLL_ANGLE_4;
    } else if (angle < 135){
      angle_mask = ROLL_ANGLE_3 | ROLL_ANGLE_4 | ROLL_ANGLE_5;
    } else if (angle < 165) {
      angle_mask = ROLL_ANGLE_4 | ROLL_ANGLE_5 | ROLL_ANGLE_6;
    } else if (angle < 195) {
      angle_mask = ROLL_ANGLE_5 | ROLL_ANGLE_6 | ROLL_ANGLE_7;
    } else if (angle < 225) {
      angle_mask = ROLL_ANGLE_6 | ROLL_ANGLE_7| ROLL_ANGLE_8;
    } else if (angle < 255) {
      angle_mask = ROLL_ANGLE_7 | ROLL_ANGLE_8 | ROLL_ANGLE_9;
    } else if (angle < 285) {
      angle_mask = ROLL_ANGLE_8 | ROLL_ANGLE_9 | ROLL_ANGLE_10;
    } else if (angle < 315) {
      angle_mask = ROLL_ANGLE_9 | ROLL_ANGLE_10 | ROLL_ANGLE_11;
    } else if (angle < 345) {
      angle_mask = ROLL_ANGLE_10 | ROLL_ANGLE_11 | ROLL_ANGLE_0;
    }
    break;
  case 3: // pick 3 angle
  default:
    if ((angle < 15) || (angle >= 345)) {
      angle_mask = ROLL_ANGLE_10 | ROLL_ANGLE_11 | ROLL_ANGLE_0 |
        ROLL_ANGLE_1 | ROLL_ANGLE_2;
    } else if (angle < 45) {
      angle_mask = ROLL_ANGLE_11 | ROLL_ANGLE_0 | ROLL_ANGLE_1 |
        ROLL_ANGLE_2 | ROLL_ANGLE_3;
    } else if (angle < 75 ) {
      angle_mask = ROLL_ANGLE_0 | ROLL_ANGLE_1 | ROLL_ANGLE_2 |
        ROLL_ANGLE_3 | ROLL_ANGLE_4;
    } else if (angle < 105) {
      angle_mask = ROLL_ANGLE_1 | ROLL_ANGLE_2 | ROLL_ANGLE_3 |
        ROLL_ANGLE_4 | ROLL_ANGLE_5;
    } else if (angle < 135){
      angle_mask = ROLL_ANGLE_2 | ROLL_ANGLE_3 | ROLL_ANGLE_4 |
        ROLL_ANGLE_5 | ROLL_ANGLE_6;
    } else if (angle < 165) {
      angle_mask = ROLL_ANGLE_3 | ROLL_ANGLE_4 | ROLL_ANGLE_5 |
        ROLL_ANGLE_6 | ROLL_ANGLE_7;
    } else if (angle < 195) {
      angle_mask = ROLL_ANGLE_4 | ROLL_ANGLE_5 | ROLL_ANGLE_6 |
        ROLL_ANGLE_7 | ROLL_ANGLE_8;
    } else if (angle < 225) {
      angle_mask = ROLL_ANGLE_5 | ROLL_ANGLE_6 | ROLL_ANGLE_7|
        ROLL_ANGLE_8 | ROLL_ANGLE_9;
    } else if (angle < 255) {
      angle_mask = ROLL_ANGLE_6 | ROLL_ANGLE_7 | ROLL_ANGLE_8 |
        ROLL_ANGLE_9 | ROLL_ANGLE_10;
    } else if (angle < 285) {
      angle_mask = ROLL_ANGLE_7 | ROLL_ANGLE_8 | ROLL_ANGLE_9 |
        ROLL_ANGLE_10 | ROLL_ANGLE_11;
    } else if (angle < 315) {
      angle_mask = ROLL_ANGLE_8 | ROLL_ANGLE_9 | ROLL_ANGLE_10 |
        ROLL_ANGLE_11 | ROLL_ANGLE_0;
    } else if (angle < 345) {
      angle_mask = ROLL_ANGLE_9 | ROLL_ANGLE_10 | ROLL_ANGLE_11 |
        ROLL_ANGLE_0 | ROLL_ANGLE_1;
    }
    break;
  }

  return angle_mask;
}

/**
 * Function: faceproc_common_get_angle
 *
 * Description: Get the faceproc angle
 *
 * Input parameters:
 *   angle: face detection angle macro
 *   enable_upfront: Whether to enable upfront angles only
 *   upfront_angle: angle area for up-front region
 *   device_rotation: current device orientation
 *
 * Return values:
 *     bitmask for engine angle
 *
 * Notes: none
 **/
uint32_t faceproc_common_get_angle(fd_chromatix_angle_t angle,
  bool enable_upfront, uint32_t upfront_angle, int32_t device_rotation)
{
  if ((enable_upfront) && (device_rotation != -1)) {
    if (upfront_angle == 0){
      return ROLL_ANGLE_NONE;
    } else {
      return faceproc_common_get_roll_angle(device_rotation,
        (upfront_angle / 30));
    }
  }

  switch (angle) {
    case FD_ANGLE_ALL:
      return ROLL_ANGLE_ALL;
    case FD_ANGLE_15_ALL:
      return (ROLL_ANGLE_0 | ROLL_ANGLE_3 | ROLL_ANGLE_6 | ROLL_ANGLE_9);
    case FD_ANGLE_45_ALL:
      return (ROLL_ANGLE_0 | ROLL_ANGLE_1 | ROLL_ANGLE_2 |
        ROLL_ANGLE_3 | ROLL_ANGLE_4 | ROLL_ANGLE_8 |
        ROLL_ANGLE_9 | ROLL_ANGLE_10 | ROLL_ANGLE_11);
    case FD_ANGLE_MANUAL:
      /* todo */
      return ROLL_ANGLE_NONE;
    case FD_ANGLE_NONE:
    default:
      return ROLL_ANGLE_NONE;
  }
  return ROLL_ANGLE_NONE;
}

/**
 * Function: faceproc_common_get_face_size
 *
 * Description: Get the face size
 *
 * Input parameters:
 *   face_adj_type: type of face dimension calculation
 *   face_size: face size for fixed adjustment
 *   ratio: facesize ratio for floating adjustment
 *   min_size: minimum face size supported by library
 *   dimension: min(height/width) of the image
 *
 * Return values:
 *     face size based on input parameters
 *
 * Notes: none
 **/
uint32_t faceproc_common_get_face_size(fd_face_adj_type_t face_adj_type,
  uint32_t face_size,
  float ratio,
  uint32_t min_size,
  uint32_t dimension)
{
  switch (face_adj_type) {
    case FD_FACE_ADJ_FLOATING: {
      uint32_t size = (uint32_t)((float)dimension * ratio);;
      return(size < min_size) ? min_size : size;
    }
    case FD_FACE_ADJ_FIXED:
    default:
      return face_size;
  }
  return face_size;
}

/**
 * Function: faceproc_common_validate_density
 *
 * Description: Validate density value
 *
 * Input parameters:
 *   new_dens_range: which dens range to validate.
 *       true - v6.x based, false - v5.1 based
 *   density: input density value
 *   use_next_density: whether to use next high dens value
 *
 * Return values:
 *     validated density value
 *
 * Notes: none
 **/
uint32_t faceproc_common_validate_and_get_density(
  bool new_dens_range, uint32_t density, bool use_next_density)
{
  if (new_dens_range) {
    switch (density) {
      case FD_DENSITY_HIGHEST :
        density = FD_DENSITY_HIGHEST;
        break;
      case FD_DENSITY_HIGH :
        density = (use_next_density == 0) ? density : FD_DENSITY_HIGHEST;
        break;
      case FD_DENSITY_NORMAL :
        density = (use_next_density == 0) ? density : FD_DENSITY_HIGH;
        break;
      case FD_DENSITY_LOW :
        density = (use_next_density == 0) ? density : FD_DENSITY_NORMAL;
        break;
      case FD_DENSITY_LOWEST :
        density = (use_next_density == 0) ? density : FD_DENSITY_LOWEST;
        break;
      default :
      IDBG_WARN("Incorrect chromatix value for density, should be "
        "one of FD_DENSITY_HIGHEST, FD_DENSITY_HIGH, FD_DENSITY_NORMAL, "
        "FD_DENSITY_LOW, FD_DENSITY_LOWEST");
        density = (use_next_density == 0) ? FD_DENSITY_NORMAL : FD_DENSITY_HIGH;
        break;
    }
  } else {
    if ((density >= 20) && (density <= 40)) {
      density = (use_next_density == 0) ? density : (density - 5);
      if (density < 20)
        density = 20;
    } else {
      IDBG_WARN("Incorrect chromatix value for density, should be "
        "in the range of [20, 40]");
      density = (use_next_density == 0) ? 33 : 28;
    }
  }

  return density;
}

/**
 * Function: faceproc_common_get_face_index
 *
 * Description: Get the face index matching criteria from the face results
 *
 * Input parameters:
 *   p_feedback - feedback from previous frame processing
 *   face_criteria - criteria to select face
 *
 * Return values:
 *   face index. -1 if no faces
 *
 * Notes: none
 **/
int faceproc_common_get_face_index(
  faceproc_result_t *p_result, faceproc_face_criteria_t face_criteria)
{
  if (!p_result) {
    return -1;
  }

  if ((p_result->num_faces_detected == 0) ||
    (p_result->num_faces_detected >= MAX_FACE_ROI)) {
    return -1;
  }

  faceproc_info_t *p_roi = &p_result->roi[0];
  int face_index = 0;
  uint32_t index;
  uint32_t face_size;
  int face_id;

  if (face_criteria == FACE_BIGGEST) {
    face_size = p_roi[0].face_boundary.dx;
    for (index = 1; index < p_result->num_faces_detected; index++) {
      if (face_size < p_roi[index].face_boundary.dx) {
        face_size = p_roi[index].face_boundary.dx;
        face_index = index;
      }
    }
  } else if (face_criteria == FACE_SMALLEST) {
    face_size = p_roi[0].face_boundary.dx;
    for (index = 1; index < p_result->num_faces_detected; index++) {
      if (face_size > p_roi[index].face_boundary.dx) {
        face_size = p_roi[index].face_boundary.dx;
        face_index = index;
      }
    }
  } else if (face_criteria == FACE_OLDEST) {
    face_id = p_roi[0].unique_id;
    for (index = 1; index < p_result->num_faces_detected; index++) {
      if (face_id > p_roi[index].unique_id) {
        face_id = p_roi[index].unique_id;
        face_index = index;
      }
    }
  } else if (face_criteria == FACE_NEWEST) {
    face_id = p_roi[0].unique_id;
    for (index = 1; index < p_result->num_faces_detected; index++) {
      if (face_id < p_roi[index].unique_id) {
        face_id = p_roi[index].unique_id;
        face_index = index;
      }
    }
  }

  return face_index;
}

/**
 * Function: faceproc_common_get_feature_flags
 *
 * Description: Get the enable/disable flags for facial part features
 *
 * Input parameters:
 *   p_config - The pointer to the faceproc config
 *   p_fd_chromatix - pointer to the chromatix
 *   p_enable_face_landmarks - Pointer to set flag whether to enable landmarks
 *   p_enable_contour - Pointer to set flag whether to enable contour
 *   p_enable_smile - Pointer to set flag whether to enable smile
 *   p_enable_blink - Pointer to set flag whether to enable blink
 *   p_enable_gaze - Pointer to set flag whether to enable gaze
 *
   * Return values: None
 *
 * Notes: none
 **/
void faceproc_common_get_feature_flags(faceproc_config_t *p_config,
  fd_chromatix_t *p_fd_chromatix,
  boolean *p_enable_face_landmarks,
  boolean *p_enable_contour,
  boolean *p_enable_smile,
  boolean *p_enable_blink,
  boolean *p_enable_gaze)
{
  *p_enable_face_landmarks = FALSE;
  *p_enable_contour = FALSE;
  *p_enable_smile = FALSE;
  *p_enable_blink = FALSE;
  *p_enable_gaze = FALSE;

  if (FD_FACEPT_ENABLE(p_config)) {
    if ((p_config->fd_face_info_mask & FACE_INFO_MASK_CONTOUR) &&
      (p_fd_chromatix->enable_contour_detection)) {
      *p_enable_contour = TRUE;
    }

    if ((p_config->fd_face_info_mask & FACE_INFO_MASK_SMILE) &&
      (p_fd_chromatix->enable_smile_detection)) {
      *p_enable_smile = TRUE;
    }

    if ((p_config->fd_face_info_mask & FACE_INFO_MASK_BLINK) &&
      (p_fd_chromatix->enable_blink_detection)) {
      *p_enable_blink = TRUE;
    }

    if ((p_config->fd_face_info_mask & FACE_INFO_MASK_GAZE) &&
      (p_fd_chromatix->enable_gaze_detection)) {
      *p_enable_gaze = TRUE;
    }

    // We need to run PT (face landmarks) to run any of BSGC.
    if ((p_config->fd_face_info_mask & FACE_INFO_MASK_FACE_LANDMARKS) ||
      (*p_enable_contour == TRUE) ||
      (*p_enable_smile == TRUE) ||
      (*p_enable_blink == TRUE) ||
      (*p_enable_gaze == TRUE)) {
      *p_enable_face_landmarks = TRUE;
    }
  }

  IDBG_HIGH("FD_FACEPT_ENABLE(p_comp)=%d, fd_face_info_mask=0x%x, "
    "Facial Parts features: landmarks=%d, contour=%d, "
    "smile=%d, blink=%d, gaze=%d",
    FD_FACEPT_ENABLE(p_config), p_config->fd_face_info_mask,
    *p_enable_face_landmarks, *p_enable_contour,
    *p_enable_smile, *p_enable_blink, *p_enable_gaze);

  return;
}

/** fd_frame_dump_count
 *
 *   number of frames that have been dumped from fd hw debugging.
 */
static int32_t fd_frame_dump_count = 0;

/**
 * Function: faceproc_common_frame_dump
 *
 * Description: Dump frames based on dump configuration
 *
 * Arguments:
 *   @p_frame: Pointer to frame which need to be dumped.
 *   @dump_mode: Frame dump mode.
 *   @tracked: Set if face is tracked.
 *   @faces_detected: Number of detected faces.
 *
 * Return values:
 *   None
 **/
void faceproc_common_frame_dump(img_frame_t *p_frame,
  faceproc_dump_mode_t dump_mode, uint32_t tracked, uint32_t faces_detected)
{
  int32_t fdhw_frame_dump_set_count = 0;
  if (!p_frame) {
    IDBG_ERROR("Invalid frame dump input");
    return;
  }

  if (dump_mode != FACE_FRAME_DUMP_OFF) {
    switch (dump_mode) {
    case FACE_FRAME_DUMP_NON_TRACKED:
      if (faces_detected && !tracked) {
        img_dump_frame(p_frame, FACE_DEBUG_PATH, faces_detected, NULL);
      }
      break;
    case FACE_FRAME_DUMP_TRACKED:
      if (tracked) {
        img_dump_frame(p_frame, FACE_DEBUG_PATH, tracked, NULL);
      }
      break;
    case FACE_FRAME_DUMP_NOT_DETECTED:
      if (faces_detected == 0) {
        img_dump_frame(p_frame, FACE_DEBUG_PATH, 0, NULL);
      }
      break;
    case FACE_FRAME_DUMP_ALL:
      img_dump_frame(p_frame, FACE_DEBUG_PATH, faces_detected, NULL);
      break;
    case FACE_FRAME_DUMP_NUMBERED_SET:
      fdhw_frame_dump_set_count = img_common_get_prop(
        "persist.vendor.camera.fd.frdump.ct", "0");
      IDBG_MED("%s %d, set_count %d dump_count %d",
        __func__, __LINE__,
        fdhw_frame_dump_set_count,
        fd_frame_dump_count );
        if (0 == fdhw_frame_dump_set_count) {
          fd_frame_dump_count = 0;
        }
        else if (fd_frame_dump_count < fdhw_frame_dump_set_count) {
          img_dump_frame(p_frame, FACE_DEBUG_PATH, ++fd_frame_dump_count,
            NULL);
        }
      break;
    default:
      return;
    }
  }
}

