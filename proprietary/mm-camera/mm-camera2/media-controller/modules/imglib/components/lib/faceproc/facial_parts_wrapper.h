/***************************************************************************
* Copyright (c) 2014-2016 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
***************************************************************************/

#ifndef __FD_FACIAL_PARTS_WRAPPER_H__
#define __FD_FACIAL_PARTS_WRAPPER_H__

#include "img_comp_priv.h"
#include "faceproc.h"

#include "DetectorComDef.h"
#include "FaceProcAPI.h"
#include "FaceProcCtAPI.h"
#include "FaceProcDtAPI.h"
#include "FaceProcFrAPI.h"
#include "FaceProcGbAPI.h"
#include "FaceProcSmAPI.h"
#include "FaceProcPcAPI.h"
#include "FaceProcPtAPI.h"
#include <inttypes.h>

/** facial_parts_wrapper_lib_t
 *
 *   The function pointers maps to the facial parts engine library
 **/
typedef struct {
  void *ptr;
  /*FaceProcDT API*/
  HDETECTION (*FACEPROC_CreateDetection)(void);
  INT32 (*FACEPROC_SetDtRefreshCount)(HDETECTION hDT, INT32 nMode,
    INT32 nRefreshCount);
  INT32 (*FACEPROC_DeleteDetection)(HDETECTION hDT);
  INT32 (*FACEPROC_SetDtMode)(HDETECTION hDT, INT32 nMode);
  INT32 (*FACEPROC_SetDtStep)(HDETECTION hDT,
    INT32 nNonTrackingStep, INT32 nTrackingStep);
  INT32 (*FACEPROC_SetDtAngle)(HDETECTION hDT,
    UINT32 anNonTrackingAngle[POSE_TYPE_COUNT],
    UINT32 nTrackingAngleExtension);
  INT32 (*FACEPROC_SetDtDirectionMask)(HDETECTION hDT, BOOL bMask);
  INT32 (*FACEPROC_SetDtFaceSizeRange)(HDETECTION hDT, INT32 nMinSize,
    INT32 nMaxSize);
  INT32 (*FACEPROC_SetDtThreshold)(HDETECTION hDT,
    INT32 nNonTrackingThreshold,
    INT32 nTrackingThreshold);
  INT32 (*FACEPROC_GetDtFaceCount)(HDTRESULT hDtResult,
    INT32 *pnCount);
  HDTRESULT (*FACEPROC_CreateDtResult)(INT32 nMaxFaceNumber,
    INT32 nMaxSwapNumber);
  INT32 (*FACEPROC_DeleteDtResult)(HDTRESULT hDtResult);
  INT32 (*FACEPROC_Detection)(HDETECTION hDT, RAWIMAGE *pImage,
    INT32 nWidth, INT32 nHeight,
    INT32 nAccuracy, HDTRESULT hDtResult);
  INT32 (*FACEPROC_GetDtFaceInfo)(HDTRESULT hDtResult,
    INT32 nIndex, FACEINFO *psFaceInfo);
  INT32 (*FACEPROC_SetDtRectangleMask)(HDETECTION hDT,
    RECT rcNonTrackingArea, RECT rcTrackingArea);
  INT32 (*FACEPROC_GetDtRectangleMask)(HDETECTION hDT,
    RECT *prcNonTrackingArea, RECT *prcTrackingArea);
  INT32 (*FACEPROC_ClearDtResult)(HDTRESULT hDtResult);
  /*FaceProcPT API*/
  HPOINTER (*FACEPROC_PT_CreateHandle)(void);
  INT32 (*FACEPROC_PT_DeleteHandle)(HPOINTER hPT);
  HPTRESULT (*FACEPROC_PT_CreateResultHandle)(void);
  INT32 (*FACEPROC_PT_DeleteResultHandle)(HPTRESULT hPtResult);
  INT32 (*FACEPROC_PT_SetPosition)(HPOINTER hPT, POINT *pptLeftTop,
    POINT *pptRightTop, POINT *pptLeftBottom, POINT *pptRightBottom,
    INT32 nPose, DTVERSION DtVersion);
  INT32 (*FACEPROC_PT_SetPositionIP)(HPOINTER hPT, INT32 nCenterX,
    INT32 nCenterY, INT32 nSize, INT32 nAngle, INT32 nScale,
    INT32 nPose, DTVERSION DtVersion);
  INT32 (*FACEPROC_PT_SetMode)(HPOINTER hPT, INT32 nMode);
  INT32 (*FACEPROC_PT_SetConfMode)(HPOINTER hPT, INT32 nConfMode);
  INT32 (*FACEPROC_PT_DetectPoint)(HPOINTER hPT, RAWIMAGE *pImage,
    INT32 nWidth, INT32 nHeight, HPTRESULT hPtResult);
  INT32 (*FACEPROC_PT_GetResult)(HPTRESULT hPtResult,
    INT32 nPointNum, POINT aptPoint[], INT32 anConfidence[]);
  INT32 (*FACEPROC_PT_GetFaceDirection)(HPTRESULT hPtResult,
    INT32 *pnUpDown, INT32 *pnLeftRight, INT32 *pnRoll);
  /*FaceProcCt API */
  HCONTOUR (*FACEPROC_CT_CreateHandle)(void);
  INT32 (*FACEPROC_CT_DeleteHandle)(HCONTOUR hCT);
  HCTRESULT (*FACEPROC_CT_CreateResultHandle)(void);
  INT32 (*FACEPROC_CT_DeleteResultHandle)(HCTRESULT hCtResult);
  INT32 (*FACEPROC_CT_SetPointFromHandle)(HCONTOUR hCT, HPTRESULT hPtResult);
  INT32 (*FACEPROC_CT_DetectContour)(HCONTOUR hCT, RAWIMAGE *pImage,
    INT32 nWidth, INT32 nHeight, HCTRESULT hCtResult);
  INT32 (*FACEPROC_CT_GetResult)(HCTRESULT hCtResult, INT32 nPointNum,
    POINT aptCtPoint[]);
  INT32 (*FACEPROC_CT_SetDetectionMode)(HCONTOUR hCT, INT32 nMode);
  HSMILE (*FACEPROC_SM_CreateHandle)(void);
  INT32 (*FACEPROC_SM_DeleteHandle)(HSMILE hSM);
  HSMRESULT (*FACEPROC_SM_CreateResultHandle)(void);
  INT32 (*FACEPROC_SM_DeleteResultHandle)(HSMRESULT hSmResult);
  INT32 (*FACEPROC_SM_SetPointFromHandle)(HSMILE hSM, HPTRESULT hPtResult);
  INT32 (*FACEPROC_SM_GetResult)(HSMRESULT hSmResult, INT32 *pnSmile,
    INT32 *pnConfidence);
  INT32 (*FACEPROC_SM_Estimate)(HSMILE hSM, RAWIMAGE *pImage, INT32 nWidth,
    INT32 nHeight, HSMRESULT hSmResult);
  /* Gaze/Blink Detect */
  HGAZEBLINK (*FACEPROC_GB_CreateHandle)(void);
  INT32 (*FACEPROC_GB_DeleteHandle)(HGAZEBLINK hGB);
  HGBRESULT (*FACEPROC_GB_CreateResultHandle)(void);
  INT32 (*FACEPROC_GB_DeleteResultHandle)(HGBRESULT hGbResult);
  INT32 (*FACEPROC_GB_SetPointFromHandle)(HGAZEBLINK hGB, HPTRESULT hPtResult);
  INT32 (*FACEPROC_GB_Estimate)(HGAZEBLINK hGB, RAWIMAGE *pImage,
    INT32 nWidth,
    INT32 nHeight, HGBRESULT hGbResult);
  INT32 (*FACEPROC_GB_GetEyeCloseRatio)(HGBRESULT hGbResult,
    INT32 *pnCloseRatioLeftEye, INT32 *pnCloseRatioRightEye);
  INT32 (*FACEPROC_GB_GetGazeDirection)(HGBRESULT hGbResult,
    INT32 *pnGazeLeftRight, INT32 *pnGazeUpDown);
} facial_parts_wrap_lib_t;

/** facial_parts_wrap_config_t
 *
 *   Faceproc config structure
 **/
typedef struct {
  int enable_gaze;
  int enable_blink;
  int enable_smile;
  int detection_threshold;
  int closed_eye_ratio;
  uint32_t discard_face_below;
  uint32_t discard_threshold;
  float weight_mouth;
  float weight_eyes;
  float weight_nose;
  float weight_face;
  uint32_t eyes_use_max_filter;
  uint32_t nose_use_max_filter;
  uint32_t sw_face_threshold;
  uint32_t sw_face_box_border_per;
  uint32_t sw_face_search_dens;
  uint32_t sw_face_discard_border;
  uint32_t sw_face_discard_out;
  int min_threshold;
  int enable_contour;
  int contour_detection_mode;
  uint32_t enable_fp_false_pos_filtering;
  uint32_t enable_sw_false_pos_filtering;
} facial_parts_wrap_config_t;

/** faceproc_comp_t
 *
 *   Faceproc component structure
 **/
typedef struct {
  HDETECTION hdt;
  HDTRESULT hdtresult;
  HPOINTER hpt;
  HPTRESULT hptresult;
  HCONTOUR hct;
  HCTRESULT hctresult;
  HSMILE hsm;
  HSMRESULT hsmresult;
  HGAZEBLINK hgb;
  HGBRESULT hgbresult;
  facial_parts_wrap_lib_t p_lib;
  facial_parts_wrap_config_t config;
} facial_parts_wrap_t;

void *facial_parts_wrap_create();

int facial_parts_wrap_config(void *handle,
  facial_parts_wrap_config_t *p_config);

int facial_parts_wrap_process_result(void *handle, img_frame_t *p_frame,
  faceproc_info_t *p_output, int verify_face, int face_tracked,
  faceproc_info_t * p_old, bool run_facial_parts);

void facial_parts_wrap_destroy(void *handle);

#endif //__FD_FACIAL_PARTS_WRAPPER_H__
