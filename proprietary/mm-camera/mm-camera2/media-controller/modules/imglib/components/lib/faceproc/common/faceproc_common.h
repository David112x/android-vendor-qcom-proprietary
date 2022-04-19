/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __FACEPROC_COMMON_H__
#define __FACEPROC_COMMON_H__

#include "faceproc.h"
#include "faceproc_comp.h"
#include "faceproc_sw_wrapper.h"

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
unsigned int faceproc_common_get_roll_angle(int angle, int adjacent);

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
  bool enable_upfront, uint32_t upfront_angle, int32_t device_rotation);

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
  uint32_t face_size, float ratio, uint32_t min_size, uint32_t dimension);

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
  bool new_dens_range, uint32_t density, bool use_next_density);

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
  faceproc_result_t *p_result, faceproc_face_criteria_t face_criteria);

/**
 * Function: faceproc_common_feature_flags
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
  boolean *p_enable_gaze);

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
  faceproc_dump_mode_t dump_mode, uint32_t tracked, uint32_t faces_detected);

#endif //__FACEPROC_COMMON_H__
