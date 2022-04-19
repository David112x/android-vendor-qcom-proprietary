/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
#ifndef __FACEPROC_HW_TRACKER_WRAPPER_H__
#define __FACEPROC_HW_TRACKER_WRAPPER_H__

#include "faceproc_hw_comp.h"
#include "HWTrackerAPI.h"


/** fphwtw_tracker_fptr_t
 *   Function pointers for Tracker library functions.
 **/
typedef struct {
  /* Create/delete handle */
  HHWTRACKER (*HWTracker_CreateHandle)(INT32 nMaxTrackCount);
  INT32 (*HWTracker_DeleteHandle)(HHWTRACKER hHWTR);

  /* Create/delete result handle */
  HHWTRACKERRESULT (*HWTracker_CreateResultHandle)(void);
  INT32 (*HWTracker_DeleteResultHandle)(HHWTRACKERRESULT hHWTR);

  /* Set the number of detected faces */
  INT32 (*HWTracker_SetDetectCount)(HHWTRACKER hHWTR, INT32 nCount);
  /* Set each face detection result */
  INT32 (*HWTracker_SetDetectResult)(HHWTRACKER hHWTR,
    INT32 nI,HWTDtResult* psDtRes);

  /* Execute tracking */
  INT32 (*HWTracker_Execute)(HHWTRACKER hHWTR, HHWTRACKERRESULT hHWTresult,
    UINT32 unFrameNo, UINT32 unTimeStamp, INT32 nWidth, INT32 nHeight,
    VOID* pFPFilter);

  /* Get a number of results */
  INT32 (*HWTracker_GetConsistentCount)(HHWTRACKERRESULT hHWTR,
    INT32 *pnCount);

  /* Get each result */
  INT32 (*HWTracker_GetConsistentResult)(HHWTRACKERRESULT hHWTR,
    INT32 nID, HWTResult* psDtRes);

  /* Set tracking parameters                                */
  INT32 (*HWTracker_SetHoldCount)(HHWTRACKER hHWTracker, INT32 nHoldCount);
  INT32 (*HWTracker_GetHoldCount)(HHWTRACKER hHWTracker, INT32* pnHoldCount);

  INT32 (*HWTracker_SetDelayCount)(HHWTRACKER hHWTracker, INT32 nDelayCount);
  INT32 (*HWTracker_GetDelayCount)(HHWTRACKER hHWTracker, INT32* pnDelayCount);

  INT32 (*HWTracker_SetDeviceOrientation)(HHWTRACKER hHWTracker,
    INT32 nCurrDeviceOrientation);
  INT32 (*HWTracker_GetDeviceOrientation)(HHWTRACKER hHWTracker,
    INT32* pnCurrDeviceOrientation);

  INT32 (*HWTracker_SetAngleDiffForStrictThresholds)(HHWTRACKER hHWTracker,
    INT32 nAngleDiffForStrictThreshold);
  INT32 (*HWTracker_GetAngleDiffForStrictThresholds)(HHWTRACKER hHWTracker,
    INT32* pnAngleDiffForStrictThreshold);

  INT32 (*HWTracker_SetSwThresholds)(HHWTRACKER hHWTracker,
    INT32 nSwGoodFaceThreshold,
    INT32 nSwThreshold,
    INT32 nSwTrackingThreshold);
  INT32 (*HWTracker_GetSwThresholds)(HHWTRACKER hHWTracker,
    INT32* pnSwGoodFaceThreshold,
    INT32* pnSwThreshold,
    INT32* pnSwTrackingThreshold);

  INT32 (*HWTracker_SetStrictSwThresholds)(HHWTRACKER hHWTracker,
    INT32 nStrictSwGoodFaceThreshold,
    INT32 nStrictSwThreshold,
    INT32 nStrictSwTrackingThreshold);
  INT32 (*HWTracker_GetStrictSwThresholds)(HHWTRACKER hHWTracker,
    INT32* pnStrictSwGoodFaceThreshold,
    INT32* pnStrictSwThreshold,
    INT32* pnStrictSwTrackingThreshold);

  INT32 (*HWTracker_SetFaceLinkTolerance)(HHWTRACKER hHWTracker,
    FLOAT32 nTrkTolMoveDist,
    FLOAT32 nTrkTolSizeRatioMin,
    FLOAT32 nTrkTolSizeRatioMax,
    FLOAT32 nTrkTolAngle);
  INT32 (*HWTracker_GetFaceLinkTolerance)(HHWTRACKER hHWTracker,
    FLOAT32* pnTrkTolMoveDist,
    FLOAT32* pnTrkTolSizeRatioMin,
    FLOAT32* pnTrkTolSizeRatioMax,
    FLOAT32* pnTrkTolAngle);

  INT32 (*HWTracker_ResetTracking)(HHWTRACKER hHWTracker);
} fphwtw_tracker_fptr_t;

/** fphwtw_lib_t
 *   @p_lib_handle : Library handle
 *   @tracker_funcs : tracker_funcs function pointer
 *
 *   HW Tracker library handle.
 **/
typedef struct {
  void *p_lib_handle;
  fphwtw_tracker_fptr_t tracker_funcs;
} fphwtw_lib_t;

/** fphwtw_config_t
 *   @nHoldCount : Hold count
 *   @nDelayCount : Delay count
 *   @nSwGoodFaceThreshold : Good face detection threshold
 *   @nSwThreshold : face detection threshold
 *   @nSwTrackingThreshold : face tracking threshold
 *   @nStrictSwGoodFaceThreshold : Strict Good face detection threshold
 *   @nStrictSwThreshold : Strict face detection threshold
 *   @nStrictSwTrackingThreshold : Strict face tracking threshold
 *   @nAngleDiffForStrictThreshold : Angle difference for Strict thresholds
 *   @nTrkTolMoveDist : Tolerance for moving distance
 *   @nTrkTolSizeRatioMin : Minimum allowance of size ratio
 *   @nTrkTolSizeRatioMax : Maximum allowance of size ratio
 *   @nTrkTolAngle : Tolerance for angle variation
 *
 *   HW Tracker config
 **/
typedef struct {
  INT32 nHoldCount;
  INT32 nDelayCount;
  INT32 nSwGoodFaceThreshold;
  INT32 nSwThreshold;
  INT32 nSwTrackingThreshold;
  INT32 nStrictSwGoodFaceThreshold;
  INT32 nStrictSwThreshold;
  INT32 nStrictSwTrackingThreshold;
  INT32 nAngleDiffForStrictThreshold;
  FLOAT32 nTrkTolMoveDist;
  FLOAT32 nTrkTolSizeRatioMin;
  FLOAT32 nTrkTolSizeRatioMax;
  FLOAT32 nTrkTolAngle;
} fphwtw_config_t;

/** faceproc_hw_tracker_wrap_t
 *   @hHWTR : Tracker handle
 *   @hHWTresult : Tracker Results handle
 *   @p_lib : Tracker library handle
 *   @config : Tracker current config
 *   @fp_filter : false positive filter handle
 *   @p_frame : current frame
 *   @width : current frame width
 *   @height : current frame height
 *   @frame_id : current frame id
 *   @rotation : current device orientation
 *
 *   HW Tracker config
 **/
typedef struct {
  HHWTRACKER hHWTR;
  HHWTRACKERRESULT hHWTresult;

  fphwtw_lib_t *p_lib;
  fphwtw_config_t config;

  img_frame_t *p_frame;

  uint32_t width;
  uint32_t height;
  uint32_t frame_id;

  int32_t curr_device_orientation;

} faceproc_hw_tracker_wrap_t;

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
int faceproc_hw_tracker_wrapper_resetTracking(faceproc_hw_comp_t *p_comp);

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
  img_frame_t *p_frame);

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
  fphwtw_config_t *p_config);

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
  fphwtw_config_t *p_config);

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
  int32_t max_track_count);

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
int faceproc_hw_tracker_wrapper_destroy(faceproc_hw_comp_t *p_comp);

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
int faceproc_hw_tracker_wrapper_load_library();

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
int faceproc_hw_tracker_wrapper_unload_library();

/**
 * Function: faceproc_hw_tracker_wrapper_set_orientation
 *
 * Description: Set device orientation
 *
 * Input parameters:
 *   p_comp - HW component handle
 *   rotation - device orientation
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_set_orientation(faceproc_hw_comp_t *p_comp,
  int32_t rotation);
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
int faceproc_hw_tracker_wrapper_update_config(faceproc_hw_comp_t * p_comp);


#endif //__FACEPROC_HW_TRACKER_WRAPPER_H__

