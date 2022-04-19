/**********************************************************************
*  Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "faceproc_hw_comp.h"
#include "faceproc_common.h"
#ifdef FACEPROC_USE_NEW_WRAPPER
#include "faceproc_sw_wrapper.h"
#else
#include "facial_parts_wrapper.h"
#endif
#include "faceproc_hw_tracker_wrapper.h"
#include <media/msm_fd.h>
#include <math.h>
#include <poll.h>

static fphwtw_lib_t g_faceproc_track_lib = { .p_lib_handle = 0 };
static int g_track_lib_counter = 0;

/*
 * Set the confidence to this value if the face is accepted and
 * while sending the face info to upper layers.
 * Some application may draw ROI only if confidence is above some
 * value.
 */
#define FD_MIN_CONFIDENCE 540

/*
 * Generally ROI size from HW is smaller than SW ROI size.
 * Modules who look at both HW, SW ROI for face matching/linking
 * they depend on SW and HW ROIs difference.
 * Reduce the size from SW to compensate the difference.
 */
#define FD_COMPENSATE_SW_TO_HW_ROI_SIZE(size) ((INT32)((size * 0.85) + 0.5))

/**
 * Function: fphwtw_trackerr_to_imgerror
 *
 * Description: Converts faceproc tracker lib error to Imglib error
 *
 * Input parameters:
 *   lib_ret - faceproc tracker lib error code
 *
 * Return values:
 *     Corresponding Imglib error code.
 *     IMG_SUCCESS for RET_NORMAL
 *     other corresponding errors
 *
 * Notes: none
 **/
static int fphwtw_trackerr_to_imgerror(int lib_track_ret)
{
  int img_error;

  switch (lib_track_ret) {
    case RET_NORMAL :
      img_error = IMG_SUCCESS;
      break;
    case RET_ERR_INITIALIZE :
      img_error = IMG_ERR_INVALID_OPERATION;
      break;
    case RET_ERR_INVALIDPARAM :
      img_error = IMG_ERR_INVALID_INPUT;
      break;
    case RET_ERR_ALLOCMEMORY :
      img_error = IMG_ERR_NO_MEMORY;
      break;
    case FACEPROC_ERR_NOHANDLE :
      img_error = IMG_ERR_NOT_FOUND;
      break;

    case RET_ERR_VARIOUS :
    default :
      img_error = IMG_ERR_GENERAL;
      break;
  }

  return img_error;
}

/**
 * Function: fphwtw_convert_hwtdtresult_to_faceinfo
 *
 * Description: Converts hwt dt result information into
 *   faceproc faceinfo data structure
 *
 * Input parameters:
 *   p_hwt_dt_result - hw tracker dt result
 *   p_face_info - faceproc face info
 *   frame_width - frame width
 *   frame_height - frame height
 *
 * Return values:
 *     Corresponding Imglib error code.
 *     IMG_SUCCESS on success
 *     other corresponding errors
 *
 * Notes: none
 **/
static int fphwtw_convert_hwtdtresult_to_faceinfo(
  const HWTDtResult *p_hwt_dt_result, faceproc_info_t *p_face_info,
  uint32_t frame_width, uint32_t frame_height)
{
  if (!p_hwt_dt_result || !p_face_info) {
    IDBG_ERROR("Invalid input p_hwt_dt_result=%p p_face_info=%p",
      p_hwt_dt_result, p_face_info);
    return IMG_ERR_INVALID_INPUT;
  }

  INT32 half_size = (p_hwt_dt_result->nSize >> 1);

  // Clip center of face coordinates to be within the frame boundary -
  // (half_size, half_size) to (frame_width-half_size, frame_height-half_size)
  POINT corrected_center = p_hwt_dt_result->ptCenter;

  CLIP(corrected_center.x, half_size, ((int32_t)frame_width - half_size));
  CLIP(corrected_center.y, half_size, ((int32_t)frame_height - half_size));

  p_face_info->face_boundary.dx = (uint32_t)p_hwt_dt_result->nSize;
  p_face_info->face_boundary.dy = (uint32_t)p_hwt_dt_result->nSize;
  p_face_info->face_boundary.x = (uint32_t)corrected_center.x - half_size;
  p_face_info->face_boundary.y = (uint32_t)corrected_center.y - half_size;

  p_face_info->face_angle_roll = p_hwt_dt_result->nAngle;
  p_face_info->gaze_angle = p_hwt_dt_result->nPose;
  p_face_info->fd_confidence = p_hwt_dt_result->nConf;
  p_face_info->unique_id = p_hwt_dt_result->nID_HW;

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_convert_hwtresult_to_faceinfo
 *
 * Description: Converts hw tracker result information into
 *   faceproc faceinfo data structure
 *
 * Input parameters:
 *   p_hwt_result - hw tracker result
 *   p_face_info - faceproc face info
 *   frame_width - frame width
 *   frame_height - frame height
 *
 * Return values:
 *     Corresponding Imglib error code.
 *     IMG_SUCCESS on success
 *     other corresponding errors
 *
 * Notes: none
 **/
static int fphwtw_convert_hwtresult_to_faceinfo(
  const HWTResult *p_hwt_result, faceproc_info_t *p_face_info,
  uint32_t frame_width, uint32_t frame_height)
{
  if (!p_hwt_result || !p_face_info) {
    IDBG_ERROR("Invalid input p_hwt_result=%p p_face_info=%p",
      p_hwt_result, p_face_info);
    return IMG_ERR_INVALID_INPUT;
  }

  INT32 half_size = (p_hwt_result->nSize >> 1);

  // Clip center of face coordinates to be within the frame boundary -
  // (half_size, half_size) to (frame_width-half_size, frame_height-half_size)
  POINT corrected_center = p_hwt_result->ptCenter;

  CLIP(corrected_center.x, half_size, ((int32_t)frame_width - half_size));
  CLIP(corrected_center.y, half_size, ((int32_t)frame_height - half_size));

  p_face_info->face_boundary.dx = (uint32_t)p_hwt_result->nSize;
  p_face_info->face_boundary.dy = (uint32_t)p_hwt_result->nSize;
  p_face_info->face_boundary.x = (uint32_t)corrected_center.x - half_size;
  p_face_info->face_boundary.y = (uint32_t)corrected_center.y - half_size;

  p_face_info->face_angle_roll = (int32_t)p_hwt_result->nAngle;
  p_face_info->gaze_angle = (int32_t)p_hwt_result->nPose;
  p_face_info->fd_confidence = (int32_t)p_hwt_result->nConf_Tr;
  p_face_info->unique_id = (int32_t)p_hwt_result->nID_Tr;

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_convert_faceinfo_to_hwtdtresult
 *
 * Description: Converts faceproc face information into
 *   hw tracker dt data structure
 *
 * Input parameters:
 *   p_face_info - faceproc face info
 *   p_hwt_dt_result - hw tracker dt result
 *
 * Return values:
 *     Corresponding Imglib error code.
 *     IMG_SUCCESS on success
 *     other corresponding errors
 *
 * Notes: none
 **/
static int fphwtw_convert_faceinfo_to_hwtdtresult(
  const faceproc_info_t *p_face_info, HWTDtResult* p_hwt_dt_result)
{
  if (!p_face_info || !p_hwt_dt_result) {
    IDBG_ERROR("Invalid input p_face_info=%p p_hwt_dt_result=%p",
      p_face_info, p_hwt_dt_result);
    return IMG_ERR_INVALID_INPUT;
  }

  p_hwt_dt_result->nID_HW = p_face_info->unique_id;

  p_hwt_dt_result->nAngle = p_face_info->face_angle_roll;
  p_hwt_dt_result->nPose = p_face_info->gaze_angle;
  p_hwt_dt_result->nConf = p_face_info->fd_confidence;
  p_hwt_dt_result->nSize = p_face_info->face_boundary.dx;
  p_hwt_dt_result->ptCenter.x =
    p_face_info->face_boundary.x + (p_face_info->face_boundary.dx >> 1);
  p_hwt_dt_result->ptCenter.y =
    p_face_info->face_boundary.y + (p_face_info->face_boundary.dx >> 1);

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_convert_hwfacedata_to_hwtdtresult
 *
 * Description: Converts hw face data information into
 *   hw tracker dt data structure
 *
 * Input parameters:
 *   p_face_data - hw face data
 *   p_hwt_dt_result - hw tracker dt result
 *
 * Return values:
 *     Corresponding Imglib error code.
 *     IMG_SUCCESS on success
 *     other corresponding errors
 *
 * Notes: none
 **/
static int fphwtw_convert_hwfacedata_to_hwtdtresult(
  const struct msm_fd_face_data *p_face_data, HWTDtResult* p_hwt_dt_result)
{
  if (!p_face_data || !p_hwt_dt_result) {
    IDBG_ERROR("Invalid input p_face_data=%p p_hwt_dt_result=%p",
      p_face_data, p_hwt_dt_result);
    return IMG_ERR_INVALID_INPUT;
  }

  p_hwt_dt_result->nConf = p_face_data->confidence;
  p_hwt_dt_result->nAngle = p_face_data->angle;
  p_hwt_dt_result->nPose =
    faceproc_hw_comp_get_pose_angle(p_face_data->pose);

  p_hwt_dt_result->nSize = p_face_data->face.width;
  p_hwt_dt_result->ptCenter.x =
    p_face_data->face.left + (p_face_data->face.width >> 1);
  p_hwt_dt_result->ptCenter.y =
    p_face_data->face.top + (p_face_data->face.width >> 1);

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_IsSimilar_face
 *
 * Description: checks whether the face detected by SW filter
 *   is similar to what HW detected
 *
 * Input parameters:
 *   FInfo1 - FaceInfo detected by HW
 *   FInfo2 - FaceInfo detected by SW
 *
 * Return values:
 *     TRUE if same face
 *     FALSE if not
 *
 * Notes: none
 **/
static BOOL fphwtw_IsSimilar_face(
  const HWTDtResult *p_FInfo1,
  const HWTDtResult *p_FInfo2)
{
  // dummy function for now.
  return TRUE;
}

/**
 * Function: fphwtw_execute_sw_detection
 *
 * Description: Execute SW detection on the current frame
 *
 * Input parameters:
 *   pnFaceSW - (O) Faces detected by SW
 *   aFaceInfoSW - (O) DT Result of SW Detection
 *   nFaceMax - (I) Max faces from SW detection
 *   p_user_data - (I) SW Detection private data
 *
 * Return values:
 *     RET_NORMAL on success
 *     corresponding error on failure
 *
 * Notes: none
 **/
INT32 fphwtw_execute_sw_detection(
  INT32* pnFaceSW, HWTDtResult aFaceInfoSW[],
  const  INT32 nFaceMax, void* p_user_data)
{
  if (!pnFaceSW || !p_user_data) {
    return RET_ERR_INVALIDPARAM;
  }

  if (nFaceMax == 0) {
    goto done;
  }

  faceproc_info_t roi[MAX_FACE_ROI];
  int i;
  int ret;
  int32_t num_faces = 0;
  faceproc_hw_comp_t *p_comp = (faceproc_hw_comp_t *)p_user_data;
  faceproc_hw_tracker_wrap_t *p_hnd =
    (faceproc_hw_tracker_wrap_t *)p_comp->p_hw_tracker_wrapper;

  ret = faceproc_hw_comp_get_sw_detection_results(
    p_comp, p_hnd->p_frame, roi, MAX_FACE_ROI, &num_faces);
  if (IMG_ERROR(ret)) {
    goto done;
  }

  if (nFaceMax < num_faces) {
    IDBG_WARN("We may miss some faces if this happens");
  }

  for (i = 0; (i < num_faces) && (i < nFaceMax); i++) {
    fphwtw_convert_faceinfo_to_hwtdtresult(&roi[i], &aFaceInfoSW[i]);
    aFaceInfoSW[i].nSize =
      FD_COMPENSATE_SW_TO_HW_ROI_SIZE(aFaceInfoSW[i].nSize);
  }
  *pnFaceSW = i;

  return RET_NORMAL;

done:
  *pnFaceSW = 0;
  return RET_NORMAL;
}

/**
 * Function: fphwtw_execute_sw_filter
 *
 * Description: Execute SW False positive filter on the given face
 *
 * Input parameters:
 *   FInfoHW - FaceInfo detected by HW which needs to verified
 *   pFInfoSW - Pointer to FaceInfo to fill face information detected  by SW
 *   p_user_data - Private user data
 *   filter_mode - false positive filtering mode. normal or hwsalvation
 *
 * Return values:
 *     TRUE if the face is detected by SW
 *     FALSE if the face is not detected by SW, means false positive
 *
 * Notes: none
 **/
BOOL fphwtw_execute_sw_filter(const HWTDtResult FInfoHW,
  HWTDtResult* pFInfoSW, void* p_user_data, const FPFilterMode filter_mode)
{
  faceproc_hw_comp_t *p_comp = (faceproc_hw_comp_t *)p_user_data;
  faceproc_hw_tracker_wrap_t *p_hnd =
    (faceproc_hw_tracker_wrap_t *)p_comp->p_hw_tracker_wrapper;
  int rc;
  faceproc_info_t face_info;

  memset(pFInfoSW, 0x00, sizeof(HWTDtResult));

  // Check if component has been aborted
  if (!faceproc_hw_comp_is_state(p_comp, IMG_STATE_STARTED)) {
    IDBG_HIGH("COMP State is not STARTED");
    return FALSE;
  }

  fphwtw_convert_hwtdtresult_to_faceinfo(&FInfoHW, &face_info,
    IMG_FD_WIDTH(p_hnd->p_frame), IMG_HEIGHT(p_hnd->p_frame));

  IDBG_HIGH("Frame[%d] FaceInfo input to SWFP Filter from Tracker :"
    "nID_HW=%d : "
    "x=%d, y=%d, dx=%d, dy=%d, hw_conf=%d face_angle_roll=%d, gaze_angle=%d",
    p_hnd->p_frame->frame_id,
    face_info.unique_id, face_info.face_boundary.x, face_info.face_boundary.y,
    face_info.face_boundary.dx, face_info.face_boundary.dy,
    face_info.fd_confidence, face_info.face_angle_roll, face_info.gaze_angle);

#ifdef FACEPROC_USE_NEW_WRAPPER
  // we can call get_config and set_config to set any per face
  // based configuration. For exa, for this face use box_border = 50%

  fpsww_config_t sw_wrapper_config;

  // Get the default/current config first
  rc = faceproc_sw_wrapper_get_config(p_comp->p_sw_wrapper,
    &sw_wrapper_config);
  if (IMG_SUCCEEDED(rc)) {
    // Update any per face configuration
    if (filter_mode == FPF_MODE_HWSalvation) {
      sw_wrapper_config.filter_params.sw_face_box_border_per =
        p_comp->fd_chromatix.assist_sw_detect_hw_salvation_box_border_perc;
      sw_wrapper_config.filter_params.sw_face_spread_tol =
        p_comp->fd_chromatix.assist_sw_detect_hw_salvation_face_spread_tol;
    } else {
      sw_wrapper_config.filter_params.sw_face_box_border_per =
        p_comp->fd_chromatix.assist_sw_detect_box_border_perc;
      sw_wrapper_config.filter_params.sw_face_spread_tol =
        p_comp->fd_chromatix.assist_sw_detect_face_spread_tol;
    }

    // Set the config to wrapper so that this is applied on next face
    rc = faceproc_sw_wrapper_set_config(p_comp->p_sw_wrapper,
      &sw_wrapper_config);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("sw_wrapper set config failed");
    }
  }

  rc = faceproc_sw_wrapper_process_result(p_comp->p_sw_wrapper,
    p_hnd->p_frame, &face_info, 1, 0, NULL, 0, false);
#else
  rc = facial_parts_wrap_process_result(p_comp->facial_parts_hndl,
    p_hnd->p_frame, &face_info, 1, 0, NULL, false);
#endif
  if (rc == IMG_ERR_NOT_FOUND) {
    goto false_positive;
  }

  fphwtw_convert_faceinfo_to_hwtdtresult(&face_info, pFInfoSW);

  // Generally ROI from HW is smaller than SW ROI.
  // Particularly when in HWSalvation mode, ROI from SW FP_Filter would be
  // the final ROI. Since, the previous final ROI is from HW, there will be
  // significant difference between previous HW ROI vs current SW ROI.
  // Reduce the size from SW to compensate the difference.
  pFInfoSW->nSize = FD_COMPENSATE_SW_TO_HW_ROI_SIZE(pFInfoSW->nSize);

  if (fphwtw_IsSimilar_face(&FInfoHW, pFInfoSW)) {
    IDBG_HIGH("FPFILTER FACE DETECTED : nID_HW=%d : "
      "x=%d, y=%d, dx=%d, dy=%d, "
      "sw_conf=%d face_angle_roll=%d, gaze_angle=%d",
      face_info.unique_id,
      face_info.face_boundary.x, face_info.face_boundary.y,
      face_info.face_boundary.dx, face_info.face_boundary.dy,
      face_info.fd_confidence, face_info.face_angle_roll,
      face_info.gaze_angle);
  } else {
    goto false_positive;
  }

  return TRUE;

false_positive:
  IDBG_HIGH("FPFILTER FACE REJECTED");
  // If the HW result is filtered, the SW result struct will be invalidated.
  memset(pFInfoSW, 0x0, sizeof(HWTDtResult));
  return FALSE;
}

/**
 * Function: fphwtw_track_status
 *
 * Description: Select track status to output
 *
 * Input parameters:
 *   status - Tracker status
 *
 * Return values:
 *     TRUE if the face need to be considered detected
 *     FALSE if the face needs to be ignored
 *
 * Notes: none
 **/
static BOOL  fphwtw_track_status(HWT_TRACKSTATUS status)
{
  if((status == HWT_CONSISTENT) ||
    (status == HWT_LOST_HW) || (status == HWT_LOST_SW)) {
    return TRUE;
  }else{
    return FALSE;
  }
}

/**
 * Function: fphwtw_tracker_load_fptrs
 *
 * Description: Load facedetection tracker library function pointers
 *
 * Input parameters:
 *   p_lib_ptr - library handle
 *   p_tracker_funcs - pointer to save function pointers
 *
 * Return values:
 *     IMG_ERR_NOT_FOUND if failed to load functions
 *     IMG_SUCCESS if success
 *
 * Notes: none
 **/
int fphwtw_tracker_load_fptrs(void *p_lib_ptr,
  fphwtw_tracker_fptr_t *p_tracker_funcs)
{
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_CreateHandle,
    "HWTracker_CreateHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_DeleteHandle,
    "HWTracker_DeleteHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_CreateResultHandle,
    "HWTracker_CreateResultHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_DeleteResultHandle,
    "HWTracker_DeleteResultHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_SetDetectCount,
    "HWTracker_SetDetectCount");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_SetDetectResult,
    "HWTracker_SetDetectResult");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_Execute,
    "HWTracker_Execute");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_GetConsistentCount,
    "HWTracker_GetConsistentCount");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_GetConsistentResult,
    "HWTracker_GetConsistentResult");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_SetHoldCount,
    "HWTracker_SetHoldCount");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_GetHoldCount,
    "HWTracker_GetHoldCount");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_SetDelayCount,
    "HWTracker_SetDelayCount");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_GetDelayCount,
    "HWTracker_GetDelayCount");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_SetDeviceOrientation,
    "HWTracker_SetDeviceOrientation");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_GetDeviceOrientation,
    "HWTracker_GetDeviceOrientation");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs,
    HWTracker_SetAngleDiffForStrictThresholds,
    "HWTracker_SetAngleDiffForStrictThresholds");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs,
    HWTracker_GetAngleDiffForStrictThresholds,
    "HWTracker_GetAngleDiffForStrictThresholds");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_SetSwThresholds,
    "HWTracker_SetSwThresholds");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_GetSwThresholds,
    "HWTracker_GetSwThresholds");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_SetStrictSwThresholds,
    "HWTracker_SetStrictSwThresholds");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_GetStrictSwThresholds,
    "HWTracker_GetStrictSwThresholds");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_SetFaceLinkTolerance,
    "HWTracker_SetFaceLinkTolerance");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_GetFaceLinkTolerance,
    "HWTracker_GetFaceLinkTolerance");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_ResetTracking,
    "HWTracker_ResetTracking");


  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_set_config
 *
 * Description: Set tracker config
 *
 * Input parameters:
 *   p_hnd - Tracker handle
 *   p_new_config - New config to set
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: Caller needs to make sure incoming args are non NULL
 **/
int fphwtw_set_config(faceproc_hw_tracker_wrap_t *p_hnd,
  fphwtw_config_t *p_new_config)
{
  if (!p_hnd->hHWTR) {
    IDBG_MED("Tracker is not created");
    return IMG_SUCCESS;
  }

  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;
  fphwtw_tracker_fptr_t *p_tracker_funcs = &p_lib->tracker_funcs;
  INT32 lib_track_ret;
  fphwtw_config_t *p_current_config = &p_hnd->config;

  if (p_current_config->nHoldCount != p_new_config->nHoldCount) {
    lib_track_ret = p_tracker_funcs->HWTracker_SetHoldCount(p_hnd->hHWTR,
      p_new_config->nHoldCount);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in HWTracker_SetHoldCount, lib_track_ret=%d",
        lib_track_ret);
      return fphwtw_trackerr_to_imgerror(lib_track_ret);
    }
    p_current_config->nHoldCount = p_new_config->nHoldCount;
  }

  if (p_current_config->nDelayCount != p_new_config->nDelayCount) {
    lib_track_ret = p_tracker_funcs->HWTracker_SetDelayCount(p_hnd->hHWTR,
      p_new_config->nDelayCount);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in HWTracker_SetDelayCount, lib_track_ret=%d",
        lib_track_ret);
      return fphwtw_trackerr_to_imgerror(lib_track_ret);
    }
    p_current_config->nDelayCount = p_new_config->nDelayCount;
  }

  if (p_current_config->nAngleDiffForStrictThreshold !=
    p_new_config->nAngleDiffForStrictThreshold) {

    lib_track_ret = p_tracker_funcs->HWTracker_SetAngleDiffForStrictThresholds(
      p_hnd->hHWTR,
      p_new_config->nAngleDiffForStrictThreshold);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in HWTracker_SetAngleDiffForStrictThresholds, "
        "lib_track_ret=%d",
        lib_track_ret);
      return fphwtw_trackerr_to_imgerror(lib_track_ret);
    }

    p_current_config->nAngleDiffForStrictThreshold =
      p_new_config->nAngleDiffForStrictThreshold;
  }

  if ((p_current_config->nSwGoodFaceThreshold !=
    p_new_config->nSwGoodFaceThreshold) ||
    (p_current_config->nSwThreshold !=
    p_new_config->nSwThreshold) ||
    (p_current_config->nSwTrackingThreshold !=
    p_new_config->nSwTrackingThreshold)) {

    lib_track_ret = p_tracker_funcs->HWTracker_SetSwThresholds(
      p_hnd->hHWTR,
      p_new_config->nSwGoodFaceThreshold,
      p_new_config->nSwThreshold,
      p_new_config->nSwTrackingThreshold);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in HWTracker_SetSwThresholds, lib_track_ret=%d",
        lib_track_ret);
      return fphwtw_trackerr_to_imgerror(lib_track_ret);
    }

    p_current_config->nSwGoodFaceThreshold =
      p_new_config->nSwGoodFaceThreshold;
    p_current_config->nSwThreshold =
      p_new_config->nSwThreshold;
    p_current_config->nSwTrackingThreshold =
      p_new_config->nSwTrackingThreshold;
  }

  if ((p_current_config->nStrictSwGoodFaceThreshold !=
    p_new_config->nStrictSwGoodFaceThreshold) ||
    (p_current_config->nStrictSwThreshold !=
    p_new_config->nStrictSwThreshold) ||
    (p_current_config->nStrictSwTrackingThreshold !=
    p_new_config->nStrictSwTrackingThreshold)) {

    lib_track_ret = p_tracker_funcs->HWTracker_SetStrictSwThresholds(
      p_hnd->hHWTR,
      p_new_config->nStrictSwGoodFaceThreshold,
      p_new_config->nStrictSwThreshold,
      p_new_config->nStrictSwTrackingThreshold);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in HWTracker_SetStrictSwThresholds, lib_track_ret=%d",
        lib_track_ret);
      return fphwtw_trackerr_to_imgerror(lib_track_ret);
    }

    p_current_config->nStrictSwGoodFaceThreshold =
      p_new_config->nStrictSwGoodFaceThreshold;
    p_current_config->nStrictSwThreshold =
      p_new_config->nStrictSwThreshold;
    p_current_config->nStrictSwTrackingThreshold =
      p_new_config->nStrictSwTrackingThreshold;
  }

  if ((p_current_config->nTrkTolMoveDist != p_new_config->nTrkTolMoveDist) ||
    (p_current_config->nTrkTolSizeRatioMin !=
    p_new_config->nTrkTolSizeRatioMin) ||
    (p_current_config->nTrkTolSizeRatioMax !=
    p_new_config->nTrkTolSizeRatioMax) ||
    (p_current_config->nTrkTolAngle != p_new_config->nTrkTolAngle)) {

    lib_track_ret = p_tracker_funcs->HWTracker_SetFaceLinkTolerance(
      p_hnd->hHWTR,
      p_new_config->nTrkTolMoveDist, p_new_config->nTrkTolSizeRatioMin,
      p_new_config->nTrkTolSizeRatioMax, p_new_config->nTrkTolAngle);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in HWTracker_SetFaceLinkTolerance, lib_track_ret=%d",
        lib_track_ret);
      return fphwtw_trackerr_to_imgerror(lib_track_ret);
    }

    p_current_config->nTrkTolMoveDist = p_new_config->nTrkTolMoveDist;
    p_current_config->nTrkTolSizeRatioMin = p_new_config->nTrkTolSizeRatioMin;
    p_current_config->nTrkTolSizeRatioMax = p_new_config->nTrkTolSizeRatioMax;
    p_current_config->nTrkTolAngle = p_new_config->nTrkTolAngle;
  }

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_set_device_orientation
 *
 * Description: Set device orientation information to tracker
 *
 * Input parameters:
 *   p_hnd - Tracker handle
 *   curr_device_orientation - current device orientation
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: Caller needs to make sure incoming args are non NULL
 **/
int fphwtw_set_device_orientation(faceproc_hw_tracker_wrap_t *p_hnd,
  int32_t curr_device_orientation)
{
  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;
  fphwtw_tracker_fptr_t *p_tracker_funcs = &p_lib->tracker_funcs;
  INT32 lib_track_ret;

  lib_track_ret = p_tracker_funcs->HWTracker_SetDeviceOrientation(p_hnd->hHWTR,
    curr_device_orientation);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in HWTracker_SetDeviceOrientation, lib_track_ret=%d",
      lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  return IMG_SUCCESS;
}


/**
 * Function: fphwtw_get_config
 *
 * Description: Get the current tracker config
 *
 * Input parameters:
 *   p_hnd - Tracker handle
 *   p_tracker_params - pointer to save the current config
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: Caller needs to make sure incoming args are non NULL
 **/
int fphwtw_get_config(faceproc_hw_tracker_wrap_t *p_hnd,
  fphwtw_config_t *p_tracker_params)
{
  if (!p_hnd->hHWTR) {
    IDBG_ERROR("Invalid handle");
    return IMG_ERR_INVALID_INPUT;
  }

  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;
  fphwtw_tracker_fptr_t *p_tracker_funcs = &p_lib->tracker_funcs;
  INT32 lib_track_ret;

  lib_track_ret = p_tracker_funcs->HWTracker_GetHoldCount(p_hnd->hHWTR,
    &p_tracker_params->nHoldCount);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in Tracker_GetHoldCount, lib_ret=%d", lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  lib_track_ret = p_tracker_funcs->HWTracker_GetDelayCount(p_hnd->hHWTR,
    &p_tracker_params->nDelayCount);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in Tracker_GetDelayCount, lib_ret=%d", lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  lib_track_ret = p_tracker_funcs->HWTracker_GetAngleDiffForStrictThresholds(
    p_hnd->hHWTR,
    &p_tracker_params->nAngleDiffForStrictThreshold);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in HWTracker_GetAngleDiffForStrictThresholds, "
      "lib_ret=%d", lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  lib_track_ret = p_tracker_funcs->HWTracker_GetSwThresholds(
    p_hnd->hHWTR,
    &p_tracker_params->nSwGoodFaceThreshold,
    &p_tracker_params->nSwThreshold,
    &p_tracker_params->nSwTrackingThreshold);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in HWTracker_GetSwThresholds, lib_ret=%d",
      lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  lib_track_ret = p_tracker_funcs->HWTracker_GetStrictSwThresholds(
    p_hnd->hHWTR,
    &p_tracker_params->nStrictSwGoodFaceThreshold,
    &p_tracker_params->nStrictSwThreshold,
    &p_tracker_params->nStrictSwTrackingThreshold);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in HWTracker_GetSwThresholds, lib_ret=%d",
      lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  lib_track_ret = p_tracker_funcs->HWTracker_GetFaceLinkTolerance(
    p_hnd->hHWTR,
    &p_tracker_params->nTrkTolMoveDist,
    &p_tracker_params->nTrkTolSizeRatioMin,
    &p_tracker_params->nTrkTolSizeRatioMax,
    &p_tracker_params->nTrkTolAngle);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in HWTracker_GetFaceLinkTolerance, lib_ret=%d",
      lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_print_current_config
 *
 * Description: Print current tracker config
 *
 * Input parameters:
 *   p_hnd - Tracker handle
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: Caller needs to make sure incoming args are non NULL
 **/
int fphwtw_print_current_config(faceproc_hw_tracker_wrap_t *p_hnd)
{
  IDBG_INFO("===================== Tracker Config =====================");
  IDBG_INFO("nHoldCount                   : %d",
    p_hnd->config.nHoldCount);
  IDBG_INFO("nDelayCount                  : %d",
    p_hnd->config.nDelayCount);
  IDBG_INFO("nSwGoodFaceThreshold         : %d",
    p_hnd->config.nSwGoodFaceThreshold);
  IDBG_INFO("nSwThreshold                 : %d",
    p_hnd->config.nSwThreshold);
  IDBG_INFO("nSwTrackingThreshold         : %d",
    p_hnd->config.nSwTrackingThreshold);
  IDBG_INFO("nStrictSwGoodFaceThreshold   : %d",
    p_hnd->config.nStrictSwGoodFaceThreshold);
  IDBG_INFO("nStrictSwThreshold           : %d",
    p_hnd->config.nStrictSwThreshold);
  IDBG_INFO("nStrictSwTrackingThreshold   : %d",
    p_hnd->config.nStrictSwTrackingThreshold);
  IDBG_INFO("nAngleDiffForStrictThreshold : %d",
    p_hnd->config.nAngleDiffForStrictThreshold);
  IDBG_INFO("nTrkTolMoveDist              : %f",
    p_hnd->config.nTrkTolMoveDist);
  IDBG_INFO("nTrkTolSizeRatioMin          : %f",
    p_hnd->config.nTrkTolSizeRatioMin);
  IDBG_INFO("nTrkTolSizeRatioMax          : %f",
    p_hnd->config.nTrkTolSizeRatioMax);
  IDBG_INFO("nTrkTolAngle                 : %f",
    p_hnd->config.nTrkTolAngle);

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_create_handles
 *
 * Description: Create HW tracker handles
 *
 * Input parameters:
 *   p_hnd - Tracker handle
 *   max_track_count - max number of faces to track
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: Caller needs to make sure incoming args are non NULL
 **/
int fphwtw_create_handles(faceproc_hw_tracker_wrap_t *p_hnd,
  int32_t max_track_count)
{
  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;
  fphwtw_tracker_fptr_t *p_tracker_funcs = &p_lib->tracker_funcs;

  p_hnd->hHWTR = p_tracker_funcs->HWTracker_CreateHandle(max_track_count);
  if (!p_hnd->hHWTR) {
    IDBG_ERROR("Failed in creating Tracker handle : max_track_count=%d",
      max_track_count);
    return IMG_ERR_GENERAL;
  }

  p_hnd->hHWTresult = p_tracker_funcs->HWTracker_CreateResultHandle();
  if (!p_hnd->hHWTresult) {
    IDBG_ERROR("Failed in creating Tracker Results handle");
    return IMG_ERR_GENERAL;
  }

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_delete_handles
 *
 * Description: Delete HW tracker handles
 *
 * Input parameters:
 *   p_hnd - Tracker handle
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: Caller needs to make sure incoming args are non NULL
 **/
int fphwtw_delete_handles(faceproc_hw_tracker_wrap_t *p_hnd)
{
  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;
  fphwtw_tracker_fptr_t *p_tracker_funcs = &p_lib->tracker_funcs;
  INT32 lib_track_ret = RET_NORMAL;

  if (p_hnd->hHWTR) {
    lib_track_ret = p_tracker_funcs->HWTracker_DeleteHandle(p_hnd->hHWTR);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in deleting Tracker handle %p, lib_ret=%d",
        p_hnd->hHWTR, lib_track_ret);
    }
    p_hnd->hHWTR = NULL;
  }

  if (p_hnd->hHWTresult) {
    lib_track_ret =
      p_tracker_funcs->HWTracker_DeleteResultHandle(p_hnd->hHWTresult);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in deleting Tracker Results handle %p, lib_ret=%d",
        p_hnd->hHWTresult, lib_track_ret);
    }
    p_hnd->hHWTresult = NULL;
  }

  return fphwtw_trackerr_to_imgerror(lib_track_ret);
}


boolean faceproc_hw_tracker_filter_face(faceproc_hw_comp_t *p_comp,
  int32_t roll_angle)
{
  IDBG_LOW("enable_upfront=%d, device_rotation=%d, roll_angle=%d, "
    "diff=%d, threshold=%d",
    p_comp->fd_chromatix.enable_upfront, p_comp->device_rotation,
    roll_angle, abs(p_comp->device_rotation - roll_angle),
    p_comp->fd_chromatix.assist_tr_anglediff_for_strict_threshold);

  if ((p_comp->device_rotation == -1) ||
    (p_comp->fd_chromatix.enable_upfront == 0)) {
    return FALSE;
  }

  if ((abs(p_comp->device_rotation - roll_angle) <=
    (int32_t)(p_comp->fd_chromatix.assist_tr_anglediff_for_strict_threshold)) ||
    (abs(p_comp->device_rotation - roll_angle) >=
    (360 -
    (int32_t)p_comp->fd_chromatix.assist_tr_anglediff_for_strict_threshold))) {
    // do not filter
    return FALSE;
  }

  // filter the face
  return TRUE;
}
/**
 * Function: faceproc_hw_tracker_wrapper_resetTracking
 *
 * Description: Reset FD tracker
 *
 * Input parameters:
 *   p_comp - HW component handle
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/

int faceproc_hw_tracker_wrapper_resetTracking(faceproc_hw_comp_t *p_comp)
{
  IDBG_MED("Reset tracking E");
  if (!p_comp || !p_comp->p_hw_tracker_wrapper) {
    IDBG_MED("No reset operation p_comp=%p", p_comp);
    /* This function can be called after destroying
       wrapper handles as well. In such cases, just
       return success, no need to do reset tracker. */
    return IMG_SUCCESS;
  }

  faceproc_hw_tracker_wrap_t *p_tracker_hnd =
    (faceproc_hw_tracker_wrap_t *)p_comp->p_hw_tracker_wrapper;
  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;
  INT32 lib_track_ret = RET_NORMAL;

  if (!p_lib->p_lib_handle) {
    IDBG_ERROR("Error opening libmmcamera_facedetection_lib.so lib");
    return IMG_ERR_NOT_FOUND;
  }

  fphwtw_tracker_fptr_t *p_tracker_funcs = &p_lib->tracker_funcs;

  if (!p_tracker_hnd->hHWTR) {
    IDBG_ERROR("Invalid handles hTrack=%p", p_tracker_hnd->hHWTR);
    return IMG_ERR_INVALID_INPUT;
  }

  lib_track_ret = p_tracker_funcs->HWTracker_ResetTracking(
    p_tracker_hnd->hHWTR);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in HWTracker_ResetTracking lib_ret=%d",lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  IDBG_MED("Reset tracking X");
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_hw_tracker_wrapper_fill_result
 *
 * Description: Execute FD tracker
 *
 * Input parameters:
 *   p_comp - HW component handle
 *   face_cnt - number faces that HW detected
 *   p_face_holder - Pointer to face info that HW detected
 *   p_frame - Current frame
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_fill_result(faceproc_hw_comp_t *p_comp,
  uint32_t face_cnt, faceproc_hw_face_holder_t *p_face_holder,
  img_frame_t *p_frame)
{
  if (!p_comp || !p_face_holder || !p_frame ||
    !p_comp->p_hw_tracker_wrapper) {
    IDBG_ERROR("Invalid input p_comp=%p p_face_holder=%p p_frame=%p",
      p_comp, p_face_holder, p_frame);
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_result_t final_result;
  struct msm_fd_face_data *p_face_data;
  int i;
  uint8_t tracked_faces = 0;
  uint8_t face_tracked;
  int rc;
  HWTDtResult hwt_dt_result[HWTRACKER_FACE_MAX_NUM];
  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;
  fphwtw_tracker_fptr_t *p_tracker_funcs = &p_lib->tracker_funcs;
  INT32 lib_track_ret = RET_NORMAL;
  faceproc_hw_tracker_wrap_t *p_tracker_hnd =
    (faceproc_hw_tracker_wrap_t *)p_comp->p_hw_tracker_wrapper;
  uint32_t max_face_cnt =
    (p_comp->config.face_cfg.max_num_face_to_detect > MAX_FACE_ROI) ?
    MAX_FACE_ROI : p_comp->config.face_cfg.max_num_face_to_detect;
  INT32 track_face_count = 0;
  HWTResult hwt_final_result;
  INT32 final_face_count = 0;
  int32_t threshold = 0;
  uint8_t faces_detected_by_hw = 0;
  uint8_t faces_detected_by_sw_only = 0;
  uint32_t filter_face_cnt = 0;

  /* Return failure in case of overflow */
  if (p_comp->batch_result_count >= FRAME_BATCH_SIZE_MAX) {
    IDBG_ERROR("Batch Result idx overflow %d ", p_comp->batch_result_count);
    return IMG_ERR_OUT_OF_BOUNDS;
  }

  p_comp->fd_frame_counter++;

  IDBG_HIGH("Frame [frame_id %d][counter %d] start : HW detected %d faces",
    p_frame->frame_id, p_comp->fd_frame_counter, face_cnt);

  // Max faces that Tracker can track
  if (face_cnt > HWTRACKER_FACE_MAX_NUM) {
    face_cnt = HWTRACKER_FACE_MAX_NUM;
  }

  if (!p_tracker_hnd->hHWTR || !p_tracker_hnd->hHWTresult) {
    IDBG_ERROR("Invalid handles hTrack=%p, hHWTresult=%p",
      p_tracker_hnd->hHWTR, p_tracker_hnd->hHWTresult);
    return IMG_ERR_INVALID_INPUT;
  }

  // Set the current device orientation before starting tracker
  faceproc_hw_tracker_wrapper_set_orientation(p_comp, p_comp->device_rotation);
  //Update tracker configuration
  if (p_comp->need_hw_tracker_config_update == TRUE) {
    faceproc_hw_tracker_wrapper_update_config(p_comp);
    p_comp->need_hw_tracker_config_update = FALSE;
  }

  filter_face_cnt = 0;

  // Apply angle filter if required and save accepted face info
  for (i = 0; i < (int)face_cnt; i++) {
    p_face_data = (struct msm_fd_face_data *)p_face_holder[i].p_face_data;

    if ((p_comp->device_rotation != -1) &&
      (p_comp->fd_chromatix.enable_upfront != 0)) {

      if (faceproc_hw_tracker_filter_face(p_comp, (int32_t)p_face_data->angle)) {
        IDBG_HIGH("FaceInfo rejected : Face[%d] : x=%d, y=%d, dx=%d, dy=%d, "
          "hw_conf=%d face_angle_roll=%d, gaze_angle=%d",
          filter_face_cnt, p_face_data->face.left, p_face_data->face.top,
          p_face_data->face.width, p_face_data->face.height,
          p_face_data->confidence, p_face_data->angle, p_face_data->pose);
        continue;
      }
    }

    fphwtw_convert_hwfacedata_to_hwtdtresult(p_face_data,
      &hwt_dt_result[filter_face_cnt]);

    hwt_dt_result[filter_face_cnt].nID_HW = filter_face_cnt;

    IDBG_HIGH("FaceInfo Detected : Face[%d] : x=%d, y=%d, dx=%d, dy=%d, "
      "hw_conf=%d face_angle_roll=%d, gaze_angle=%d",
      filter_face_cnt, p_face_data->face.left, p_face_data->face.top,
      p_face_data->face.width, p_face_data->face.height,
      p_face_data->confidence, p_face_data->angle, p_face_data->pose);

    filter_face_cnt++;
  }

  IDBG_HIGH("Frame [frame_id %d][counter %d] HW faces after filtering %d",
    p_frame->frame_id, p_comp->fd_frame_counter, filter_face_cnt);

  // Set the HW detect count to Tracker
  lib_track_ret =
    p_tracker_funcs->HWTracker_SetDetectCount(p_tracker_hnd->hHWTR,
    filter_face_cnt);
  if(lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in HWTracker_SetDetectCount handle %p, lib_ret=%d",
      p_tracker_hnd->hHWTR, lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  // Set the HW detect face results to Tracker
  for (i = 0; i < (int)filter_face_cnt; i++) {
    lib_track_ret = p_tracker_funcs->HWTracker_SetDetectResult(
      p_tracker_hnd->hHWTR, i, &hwt_dt_result[i]);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in HWTracker_SetDetectResult hHWTR %p, lib_ret=%d",
        p_tracker_hnd->hHWTR, lib_track_ret);
      return fphwtw_trackerr_to_imgerror(lib_track_ret);
    }
  }


  // Setup the Callback information.

  // Save the current frame in tracker_hndle, this will be used in FP callback
  p_tracker_hnd->p_frame = p_frame;

  // Callback function pointer, userdata
  FPFILTER_HNDL filter_handle;
  filter_handle.pUserData = p_comp;
  // Callback for false positive filtering
  filter_handle.FP_filter = fphwtw_execute_sw_filter;
  // Callback for intermittent sw detection
  filter_handle.FP_SWDetection = fphwtw_execute_sw_detection;

  // Execute Track
  lib_track_ret = p_tracker_funcs->HWTracker_Execute(p_tracker_hnd->hHWTR,
    p_tracker_hnd->hHWTresult, p_comp->fd_frame_counter, 0,
    IMG_FD_WIDTH(p_frame), IMG_HEIGHT(p_frame), &filter_handle);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in HWTracker_Execute handle %p, lib_ret=%d",
      p_tracker_hnd->hHWTR, lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  // Get the results from Tracker
  lib_track_ret = p_tracker_funcs->HWTracker_GetConsistentCount(
    p_tracker_hnd->hHWTresult, &track_face_count);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in GetConsistentCount hHWTresult %p, lib_ret=%d",
      p_tracker_hnd->hHWTresult, lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  IDBG_HIGH("Tracker Consistent face Count %d", track_face_count);

  memset(&final_result, 0x0, sizeof(faceproc_result_t));

  for (i = 0;
    ((i < track_face_count) && (final_face_count < (int32_t)max_face_cnt));
    i++) {

    lib_track_ret = p_tracker_funcs->HWTracker_GetConsistentResult(
      p_tracker_hnd->hHWTresult, i, &hwt_final_result);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in HWTracker_GetConsistentResult i=%d, lib_ret=%d",
        i, lib_track_ret);
      return fphwtw_trackerr_to_imgerror(lib_track_ret);
    }

    IDBG_HIGH("FaceInfo from Tracker [%d] "
      "nID_Tr=%d (nID_HW=%d, nDstID_HW=%d, nDstID_IM=%d), "
      "nConf_Tr=%d (nConf_HW=%d, nConf_SW_IM=%d, nConf_SW=%d, nConf_SW_FP=%d),"
      " Detected by %s, "
      "STATUS=%d, nHold=%d, "
      "nDelay=%d, nHistory=%d, gaze_angle=%d, face_angle_roll=%d, "
      "nSize=%d, Center=(%d, %d)",
      i,
      hwt_final_result.nID_Tr, hwt_final_result.nID_HW,
      hwt_final_result.nDstID_HW, hwt_final_result.nDstID_IM,
      hwt_final_result.nConf_Tr, hwt_final_result.nConf_HW,
      hwt_final_result.nConf_SW_IM, hwt_final_result.nConf_SW,
      hwt_final_result.nConf_SW_FP,
      (hwt_final_result.dstType == DET_TYPE_HWNG_SWOK) ? "SW" :
      (hwt_final_result.dstType == DET_TYPE_HWOK_SWNG) ? "HW" :
      (hwt_final_result.dstType == DET_TYPE_HWOK_SWOK) ? "HW+SW" : "NONE",
      hwt_final_result.STATUS,
      hwt_final_result.nHold, hwt_final_result.nDelay,
      hwt_final_result.nHistory, hwt_final_result.nPose,
      hwt_final_result.nAngle, hwt_final_result.nSize,
      hwt_final_result.ptCenter.x, hwt_final_result.ptCenter.y);

    if ((fphwtw_track_status(hwt_final_result.STATUS)) &&
      (hwt_final_result.nConf_Tr >= threshold)) {

      fphwtw_convert_hwtresult_to_faceinfo(&hwt_final_result,
        &final_result.roi[final_face_count],
        IMG_FD_WIDTH(p_frame), IMG_HEIGHT(p_frame));

      // Accept the face if the state is good, even if final tracking
      // confidence is low. But hardcode the confidence to a higher
      // value so that the upper layers wont reject this face info.
      // Even if we are getting tracking confidence less than
      // SET_TO_MIN_CONFIDENCE, it doesn't
      // mean SW confidence is low. It can happen in cases where
      // HW_LOST/SW_LOST. Since tracking confidence is combination of
      // HW, SW confidence, this can happen.
      if (final_result.roi[final_face_count].fd_confidence <=
        FD_MIN_CONFIDENCE) {
        final_result.roi[final_face_count].fd_confidence = FD_MIN_CONFIDENCE;
      }

      if (hwt_final_result.nHistory <=
        (p_tracker_hnd->config.nDelayCount + 1)) {
        face_tracked = 0;
        final_result.roi[final_face_count].tracking = true;
        IDBG_HIGH("New Face Detected");
      } else {
        face_tracked = 1;
        final_result.roi[final_face_count].tracking = false;
        tracked_faces++;
        IDBG_HIGH("Old Face.. tracking..");
      }

      if ((hwt_final_result.dstType == DET_TYPE_HWOK_SWNG) ||
        (hwt_final_result.dstType == DET_TYPE_HWOK_SWOK)) {
        faces_detected_by_hw++;
      } else if (hwt_final_result.dstType == DET_TYPE_HWNG_SWOK) {
        faces_detected_by_sw_only++;
      } else {
        // Should not happen, Tracker not setting type correctly
      }

      // Run facial parts
#ifdef FACEPROC_USE_NEW_WRAPPER
      rc = faceproc_sw_wrapper_process_result(p_comp->p_sw_wrapper,
        p_frame, &final_result.roi[final_face_count], 0, face_tracked,
        NULL, final_face_count, true);
#else
      rc = facial_parts_wrap_process_result(p_comp->facial_parts_hndl,
        p_frame, &final_result.roi[final_face_count], 0, 0, NULL, true);
#endif
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("Error in Facial parts");
      }

      IDBG_HIGH("FinalFaceInfo output : Face[%d] : unique_id(tracker id)=%d, "
        "x=%d, y=%d, dx=%d, dy=%d, confidence(tracker conf)=%d "
        "face_angle_roll=%d, gaze_angle=%d",
        final_face_count,
        final_result.roi[final_face_count].unique_id,
        final_result.roi[final_face_count].face_boundary.x,
        final_result.roi[final_face_count].face_boundary.y,
        final_result.roi[final_face_count].face_boundary.dx,
        final_result.roi[final_face_count].face_boundary.dy,
        final_result.roi[final_face_count].fd_confidence,
        final_result.roi[final_face_count].face_angle_roll,
        final_result.roi[final_face_count].gaze_angle);

      final_face_count++;
    }
  }

  final_result.frame_id = p_frame->frame_id;
  final_result.num_faces_detected = final_face_count;

  if (p_comp->fd_chromatix.sw_detect_optimize_config) {
    // Save number of faces detected by HW
    p_comp->sw_detection_feedback.hw_raw_face_cnt = face_cnt;

    // Save face count in final result that are detected by HW, by SW alone
    p_comp->sw_detection_feedback.final_faces_detected_by_hw =
      faces_detected_by_hw;
    p_comp->sw_detection_feedback.final_faces_detected_by_sw_only =
      faces_detected_by_sw_only;

    // Save the final tracker information so that we can use this info
    // while configuring sw detection config for the next frame
    p_comp->sw_detection_feedback.tracker_result.frame_id =
      final_result.frame_id;
    p_comp->sw_detection_feedback.tracker_result.num_faces_detected =
      final_result.num_faces_detected;
    memcpy(&p_comp->sw_detection_feedback.tracker_result.roi[0],
      &final_result.roi[0],
      final_result.num_faces_detected * sizeof(final_result.roi[0]));
  }

  pthread_mutex_lock(&p_comp->result_mutex);
  if (p_comp->frame_batch_info.batch_mode) {
    p_comp->intermediate_result[p_comp->tracking_info.cur_tbl_idx] =
      final_result;
    p_comp->batch_result_count++;
  } else {
    p_comp->last_result = final_result;
  }
  pthread_mutex_unlock(&p_comp->result_mutex);

  IDBG_HIGH("Frame [frame_id %d][counter %d] Done : "
    "Final faces = %d (HW=%d + SW=%d)",
    p_frame->frame_id, p_comp->fd_frame_counter, final_face_count,
    faces_detected_by_hw, faces_detected_by_sw_only);

  faceproc_common_frame_dump(p_frame, p_comp->dump_mode,
    tracked_faces, final_face_count);

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_hw_tracker_wrapper_get_config
 *
 * Description: Get tracker config
 *
 * Input parameters:
 *   p_comp - HW component handle
 *   p_config - config to get
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_get_config(faceproc_hw_comp_t *p_comp,
  fphwtw_config_t *p_config)
{
  int ret = IMG_SUCCESS;

  if (!p_comp || !p_config) {
    IDBG_HIGH("Invalid input param p_comp=%p, p_config=%p", p_comp, p_config);
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_hw_tracker_wrap_t *p_tracker_hnd =
    (faceproc_hw_tracker_wrap_t *)p_comp->p_hw_tracker_wrapper;

  *p_config = p_tracker_hnd->config;

  return ret;
}

/**
 * Function: faceproc_hw_tracker_wrapper_set_config
 *
 * Description: Set tracker config
 *
 * Input parameters:
 *   p_comp - HW component handle
 *   p_config - config to set
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_set_config(faceproc_hw_comp_t *p_comp,
  fphwtw_config_t *p_config)
{
  int ret = IMG_SUCCESS;

  if (!p_comp || !p_config) {
    IDBG_HIGH("Invalid input param p_comp=%p, p_config=%p", p_comp, p_config);
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_hw_tracker_wrap_t *p_tracker_hnd =
    (faceproc_hw_tracker_wrap_t *)p_comp->p_hw_tracker_wrapper;

  IDBG_HIGH("Set Config. p_tracker_hnd=%p, hHWTR=%p, hHWTresult=%p",
    p_tracker_hnd, p_tracker_hnd->hHWTR, p_tracker_hnd->hHWTresult);

  ret = fphwtw_set_config(p_tracker_hnd, p_config);
  if (IMG_ERROR(ret)) {
    IDBG_ERROR("Failed in setting new tracker config, ret=%d", ret);
    return ret;
  }

  if (IMG_LOG_LEVEL_HIGH) {
    IDBG_INFO("Set Tracker Config");
    fphwtw_print_current_config(p_tracker_hnd);
  }

  return ret;
}

/**
 * Function: faceproc_hw_tracker_wrapper_create
 *
 * Description: Create tracker handles
 *
 * Input parameters:
 *   p_comp - HW component handle
 *   max_track_count - maximum number of faces to track
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_create(faceproc_hw_comp_t *p_comp,
  int32_t max_track_count)
{
  int ret = IMG_SUCCESS;
  faceproc_hw_tracker_wrap_t *p_hnd;

  IDBG_HIGH("Create Wrapper. p_comp=%p, max_track_count=%d, "
    "p_lib_handle=%p, p_hw_tracker_wrapper=%p",
    p_comp, max_track_count, g_faceproc_track_lib.p_lib_handle,
    p_comp->p_hw_tracker_wrapper);

  // Check if the library is loaded or not.
  // Caller must call faceproc_hw_tracker_wrapper_load before calling this.
  if ((!g_faceproc_track_lib.p_lib_handle) || (g_track_lib_counter <= 0)) {
    IDBG_ERROR("HW tracker lib not loaded yet : lib handle %p, counter %d",
      g_faceproc_track_lib.p_lib_handle, g_track_lib_counter);
    return IMG_ERR_INVALID_INPUT;
  }

  if (p_comp->p_hw_tracker_wrapper) {
    IDBG_ERROR("HW tracker is already created, something wrong");
    return IMG_ERR_INVALID_INPUT;
  }

  p_hnd = calloc(1, sizeof(*p_hnd));
  if (p_hnd == NULL) {
    IDBG_ERROR("Fail to allocate handler memory");
    return IMG_ERR_NO_MEMORY;
  }

  ret = fphwtw_create_handles(p_hnd, max_track_count);
  if (IMG_ERROR(ret)) {
    IDBG_ERROR("Failed in creating tracker handles, ret=%d", ret);
    goto error_create_handle;
  }

  ret = fphwtw_get_config(p_hnd, &p_hnd->config);
  if (ret != IMG_SUCCESS) {
    IDBG_ERROR("Failed in getting default config");
    goto error_config;
  }

  if (IMG_LOG_LEVEL_HIGH) {
    IDBG_INFO("Default tracker config");
    fphwtw_print_current_config(p_hnd);
  }

  p_hnd->curr_device_orientation = -1;

  p_comp->p_hw_tracker_wrapper = (void*)p_hnd;

  IDBG_HIGH("Created Wrapper. p_hw_tracker_wrapper=%p, "
    "hHWTR=%p, hHWTresult=%p",
    p_comp->p_hw_tracker_wrapper, p_hnd->hHWTR, p_hnd->hHWTresult);

  return ret;

error_config:
  fphwtw_delete_handles(p_hnd);

error_create_handle:
  free(p_hnd);
  return ret;
}

/**
 * Function: faceproc_hw_tracker_wrapper_destroy
 *
 * Description: Destroy tracker handles
 *
 * Input parameters:
 *   p_comp - HW component handle
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_destroy(faceproc_hw_comp_t *p_comp)
{
  int ret = IMG_SUCCESS;

  if (!p_comp || !p_comp->p_hw_tracker_wrapper) {
    IDBG_HIGH("HW tracker not created p_comp=%p", p_comp);
    return IMG_SUCCESS;
  }

  faceproc_hw_tracker_wrap_t *p_tracker_hnd =
    (faceproc_hw_tracker_wrap_t *)p_comp->p_hw_tracker_wrapper;

  IDBG_HIGH("Destroying Wrapper. p_tracker_hnd=%p, "
    "hHWTR=%p, hHWTresult=%p",
    p_tracker_hnd, p_tracker_hnd->hHWTR, p_tracker_hnd->hHWTresult);

  ret = fphwtw_delete_handles(p_tracker_hnd);
  if (IMG_ERROR(ret)) {
    IDBG_ERROR("Failed in Deleting tracker handles, ret=%d", ret);
    // fall through
  }

  free(p_comp->p_hw_tracker_wrapper);

  p_comp->p_hw_tracker_wrapper = NULL;

  return ret;
}

/**
 * Function: faceproc_hw_tracker_wrapper_load_library
 *
 * Description: Load tracker library
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_load_library()
{
  int rc = IMG_SUCCESS;

  IDBG_HIGH("Loading library. p_lib_handle=%p",
    g_faceproc_track_lib.p_lib_handle);

  if (g_faceproc_track_lib.p_lib_handle) {
    IDBG_HIGH("Tracker library already loaded");
    g_track_lib_counter++;
    return IMG_SUCCESS;
  }

  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;

  p_lib->p_lib_handle = dlopen("libmmcamera_facedetection_lib.so", RTLD_NOW);
  if (!p_lib->p_lib_handle) {
    IDBG_HIGH("Error opening libmmcamera_facedetection_lib.so lib");
    return IMG_ERR_NOT_FOUND;
  }

  rc = fphwtw_tracker_load_fptrs(p_lib->p_lib_handle, &p_lib->tracker_funcs);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in loading Tracker function pointers");
    goto error;
  }

  g_track_lib_counter++;

  IDBG_HIGH("Loaded library. p_lib_handle=%p",
    g_faceproc_track_lib.p_lib_handle);

  return rc;

error:
  dlclose(p_lib->p_lib_handle);
  return rc;
}

/**
 * Function: faceproc_hw_tracker_wrapper_unload_library
 *
 * Description: Unload tracker library
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_unload_library()
{
  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;

  IDBG_HIGH("UnLoading library. p_lib_handle=%p",
    g_faceproc_track_lib.p_lib_handle);

  g_track_lib_counter--;

  if (g_track_lib_counter == 0) {
    dlclose(p_lib->p_lib_handle);
    memset(&g_faceproc_track_lib, 0x0, sizeof(fphwtw_lib_t));

    IDBG_HIGH("UnLoaded library. p_lib_handle=%p",
      g_faceproc_track_lib.p_lib_handle);
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_hw_tracker_wrapper_set_orientation
 *
 * Description: Set device orientation
 *
 * Input parameters:
 *   p_comp - HW component handle
 *   curr_device_orientation - device orientation
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_set_orientation(faceproc_hw_comp_t *p_comp,
  int32_t curr_device_orientation)
{
  int ret = IMG_SUCCESS;

  if (!p_comp) {
    IDBG_HIGH("Invalid input param p_comp=%p", p_comp);
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_hw_tracker_wrap_t *p_tracker_hnd =
    (faceproc_hw_tracker_wrap_t *)p_comp->p_hw_tracker_wrapper;

  p_tracker_hnd->curr_device_orientation = curr_device_orientation;

  fphwtw_set_device_orientation(p_tracker_hnd, curr_device_orientation);

  return ret;
}
/**
 * Function: faceproc_hw_tracker_wrapper_update_config
 *
 * Description: update tracker config
 *
 * Input parameters:
 *   p_comp - faceproc hw component
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx errors on failure
 *
 * Notes: none
 **/
int faceproc_hw_tracker_wrapper_update_config(
 faceproc_hw_comp_t *p_comp)
{
  fphwtw_config_t new_config;
  int ret;

  ret = faceproc_hw_tracker_wrapper_get_config(p_comp, &new_config);
  if (IMG_ERROR(ret)) {
    IDBG_ERROR("tracker get config failed");
    return ret;
  }

  if (p_comp->dynamic_cfg_params.lux_index != -1) {
    if (p_comp->fd_chromatix.assist_tr_good_face_threshold) {
      new_config.nSwGoodFaceThreshold =
        p_comp->fd_chromatix.assist_tr_good_face_threshold +
        p_comp->fd_chromatix.dynamic_lux_config.lux_offsets
        [p_comp->dynamic_cfg_params.lux_index];
    }

    if (p_comp->fd_chromatix.assist_tr_detect_threshold) {
      new_config.nSwThreshold =
        p_comp->fd_chromatix.assist_tr_detect_threshold +
        p_comp->fd_chromatix.dynamic_lux_config.lux_offsets
        [p_comp->dynamic_cfg_params.lux_index];
    }

    if (p_comp->fd_chromatix.assist_tr_strict_good_face_threshold) {
      new_config.nStrictSwGoodFaceThreshold =
        p_comp->fd_chromatix.assist_tr_strict_good_face_threshold +
        p_comp->fd_chromatix.dynamic_lux_config.lux_offsets
        [p_comp->dynamic_cfg_params.lux_index];
    }

    if (p_comp->fd_chromatix.assist_tr_strict_detect_threshold) {
      new_config.nStrictSwThreshold =
        p_comp->fd_chromatix.assist_tr_strict_detect_threshold +
        p_comp->fd_chromatix.dynamic_lux_config.lux_offsets
        [p_comp->dynamic_cfg_params.lux_index];
    }
    ret = faceproc_hw_tracker_wrapper_set_config(p_comp,
      &new_config);

    if (IMG_ERROR(ret)) {
      IDBG_ERROR("tracker set config failed");
      return ret;
    }
  }
  return IMG_SUCCESS;
}


