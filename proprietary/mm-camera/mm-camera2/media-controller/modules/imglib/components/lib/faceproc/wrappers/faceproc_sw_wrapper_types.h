/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __FACEPROC_SW_WRAPPER_TYPES_H__
#define __FACEPROC_SW_WRAPPER_TYPES_H__

#include "libincludes/DetectorComDef.h"
#include "libincludes/FaceProcCoStatus.h"
#include "libincludes/FaceProcCoAPI.h"
#include "libincludes/FaceProcDtAPI.h"

#include "FaceProcPtAPI.h"
#include "FaceProcCtAPI.h"
#include "FaceProcSmAPI.h"
#include "FaceProcGbAPI.h"
#include "FaceProcFrAPI.h"

// Roll angle indices in DT config angle array
#define FPSWW_ANGLE_FRONT       0
#define FPSWW_ANGLE_HALFPROFILE 1
#define FPSWW_ANGLE_FULLPROFILE 2
#define FPSWW_MAX_POSE_ANGLES   3

/** fd_contour_results
 *   @contour_pt : Contour points
 *
 *   FD Contour results
 **/
typedef struct {
 fd_pixel_t contour_pt[CT_POINT_KIND_MAX];
} fd_contour_results;

/** fd_gb_result
 *   @left_blink      : Left eye blink ratio
 *   @right_blink     : Right eye blink ratio
 *   @left_right_gaze : Left eye gaze direction
 *   @top_bottom_gaze : Right eye gaze direction
 *
 *   FD Gaze and Blink results
 **/
typedef struct {
  int left_blink;
  int right_blink;
  int left_right_gaze;
  int top_bottom_gaze;
} fd_gb_result;

/** fpsww_co_fptr_t
 *   Function pointers for Common library functions.
 **/
typedef struct {
  /* Get Version */
  INT32      (*FACEPROC_CO_GetVersion)(UINT8 *pucMajor, UINT8 *pucMinor);

  /* Creation */
  HCOMMON (*FACEPROC_CO_CreateHandle)(void);
  HCOMMON (*FACEPROC_CO_CreateHandleMalloc)(void *(*malloc)(size_t size), void (*free)(void *));
  HCOMMON (*FACEPROC_CO_CreateHandleMemory)(VOID *pBMemoryAddr, UINT32 unBMemorySize,
                                            VOID *pWMemoryAddr, UINT32 unWMemorySize);
  /* Deletion */
  INT32 (*FACEPROC_CO_DeleteHandle)(HCOMMON hCO);

  /* Conversion from Square Points to Center-Form */
  INT32 (*FACEPROC_CO_ConvertSquareToCenter)(POINT ptLeftTop, POINT ptRightTop, POINT ptLeftBottom, POINT ptRightBottom,
                                             POINT *pptCenter, INT32 *pnSize, INT32 *pnAngle);
  /* Convertsion from Center-Form to Square Points */
  INT32 (*FACEPROC_CO_ConvertCenterToSquare)(POINT ptCenter, INT32 nSize, INT32 nAngle, POINT *pptLeftTop,
                                             POINT *pptRightTop, POINT *pptLeftBottom, POINT *pptRightBottom);
} fpsww_co_fptr_t;

/** fpsww_dt_fptr_t
 *   Function pointers for Detection(DT) library functions.
 **/
typedef struct {
  /* Gets This Library's version */
  INT32 (*FACEPROC_DT_GetVersion)(UINT8 *pucMajor, UINT8 *pucMinor);

  /* Creates/Deletes the Face Detection handle */
  HDETECTION (*FACEPROC_DT_CreateHandle)(HCOMMON hCO, INT32 nDetectionMode, INT32 nMaxDetectionCount);
  INT32 (*FACEPROC_DT_DeleteHandle)(HDETECTION hDT);

  /* Creates/Deletes the Face Detection result handle */
  HDTRESULT (*FACEPROC_DT_CreateResultHandle)(HCOMMON hCO);
  INT32 (*FACEPROC_DT_DeleteResultHandle) (HDTRESULT hDtResult);

  /* Executes detection */
  INT32 (*FACEPROC_DT_Detect_GRAY)(HDETECTION hDT, RAWIMAGE* pImageGRAY, INT32 nWidth, INT32 nHeight,
                                   GRAY_ORDER ImageOrder, HDTRESULT hDtResult);
  INT32 (*FACEPROC_DT_Detect_YUV422)(HDETECTION hDT, RAWIMAGE* pImageYUV, INT32 nWidth, INT32 nHeight,
                                     YUV422_ORDER ImageOrder, HDTRESULT hDtResult);
  INT32 (*FACEPROC_DT_Detect_YUV420SP)(HDETECTION hDT, RAWIMAGE* pImageY, RAWIMAGE* pImageCx,
                                       INT32 nWidth, INT32 nHeight, YUV420SP_ORDER ImageOrder, HDTRESULT hDtResult);
  INT32 (*FACEPROC_DT_Detect_YUV420FP)(HDETECTION hDT, RAWIMAGE* pImageY, RAWIMAGE* pImageCb, RAWIMAGE* pImageCr,
                                       INT32 nWidth, INT32 nHeight, YUV420FP_ORDER ImageOrder, HDTRESULT hDtResult);

  /* Gets Detection Result */
  INT32 (*FACEPROC_DT_GetResultCount)(HDTRESULT hDtResult, INT32 *pnCount);
  INT32 (*FACEPROC_DT_GetResultInfo)(HDTRESULT hDtResult, INT32 nIndex, DETECTION_INFO *psDetectionInfo);
  INT32 (*FACEPROC_DT_GetRawResultInfo)(HDTRESULT hDtResult, INT32 nIndex, DETECTION_INFO *psDetectionInfo);

  /* Sets/Gets the detection size range, i.e. the minimum and maximum Face sizes for detection */
  INT32 (*FACEPROC_DT_SetSizeRange)(HDETECTION hDT, INT32 nMinSize, INT32 nMaxSize);
  INT32 (*FACEPROC_DT_GetSizeRange)(HDETECTION hDT, INT32 *pnMinSize, INT32 *pnMaxSize);

  /* Sets/Gets the pose (yaw) angle and the Face inclination (roll) angle to be used for Face Detection */
  INT32 (*FACEPROC_DT_SetAngle)(HDETECTION hDT, UINT32 nPose, UINT32 nAngle);
  INT32 (*FACEPROC_DT_GetAngle)(HDETECTION hDT, UINT32 nPose, UINT32 *pnAngle);

  /* Sets/Gets an edge mask to restrict the area where Face Detection will be applied */
  INT32 (*FACEPROC_DT_SetEdgeMask)(HDETECTION hDT, RECT rcEdgeMask);
  INT32 (*FACEPROC_DT_GetEdgeMask)(HDETECTION hDT, RECT *prcEdgeMask);

  /* Sets/Gets the search density used by Face Detection */
  INT32 (*FACEPROC_DT_SetSearchDensity)(HDETECTION hDT, INT32 nSearchDensity);
  INT32 (*FACEPROC_DT_GetSearchDensity)(HDETECTION hDT, INT32 *pnSearchDensity);

  /* Sets/Gets the threshold value for the Face Detection results */
  INT32 (*FACEPROC_DT_SetThreshold)(HDETECTION hDT, INT32 nThreshold);
  INT32 (*FACEPROC_DT_GetThreshold)(HDETECTION hDT, INT32 *pnThreshold);

  /* Clears the tracking info stored in the Face Detection handle */
  INT32 (*FACEPROC_DT_MV_ResetTracking)(HDETECTION hDT);

  /* Sets the option to always lock the tracking of a Face designated by its Face ID, or unlock it */
  INT32 (*FACEPROC_DT_MV_ToggleTrackingLock)(HDETECTION hDT, INT32 nID);

  /* Sets/Gets the search cycle, i.e. the frame count used during Initial Face Search */
  /* and New Face Search, and the search interval for New Face Search */
  INT32 (*FACEPROC_DT_MV_SetSearchCycle)(HDETECTION hDT, INT32 nInitialFaceSearchCycle,
                                         INT32 nNewFaceSearchCycle, INT32 nNewFaceSearchInterval);
  INT32 (*FACEPROC_DT_MV_GetSearchCycle)(HDETECTION hDT, INT32 *pnInitialFaceSearchCycle,
                                         INT32 *pnNewFaceSearchCycle, INT32 *pnNewFaceSearchInterval);

  /* Sets/Gets the parameters when a Face is lost during tracking in Movie mode */
  INT32 (*FACEPROC_DT_MV_SetLostParam)(HDETECTION hDT, INT32 nMaxRetryCount, INT32 nMaxHoldCount);
  INT32 (*FACEPROC_DT_MV_GetLostParam)(HDETECTION hDT, INT32 *pnMaxRetryCount, INT32 *pnMaxHoldCount);

  /* Sets/Gets the steadiness parameters of the position and size of the Face rectangle during tracking */
  INT32 (*FACEPROC_DT_MV_SetSteadinessParam)(HDETECTION hDT, INT32 nPosSteadinessParam, INT32 nSizeSteadinessParam);
  INT32 (*FACEPROC_DT_MV_GetSteadinessParam)(HDETECTION hDT, INT32 *pnPosSteadinessParam, INT32 *pnSizeSteadinessParam);

  /* Sets/Gets the tracking swap ratio used when swapping Faces during tracking */
  INT32 (*FACEPROC_DT_MV_SetTrackingSwapParam)(HDETECTION hDT, INT32 nTrackingSwapParam);
  INT32 (*FACEPROC_DT_MV_GetTrackingSwapParam)(HDETECTION hDT, INT32 *pnTrackingSwapParam);

  /* Sets/Gets the delay count used for Initial Face Search and New Face Search */
  INT32 (*FACEPROC_DT_MV_SetDelayCount)(HDETECTION hDT, INT32 nDelayCount);
  INT32 (*FACEPROC_DT_MV_GetDelayCount)(HDETECTION hDT, INT32 *pnDelayCount);

  /* Sets/Gets an edge mask to restrict the area where tracking will be applied */
  INT32 (*FACEPROC_DT_MV_SetTrackingEdgeMask)(HDETECTION hDT, RECT rcEdgeMask);
  INT32 (*FACEPROC_DT_MV_GetTrackingEdgeMask)(HDETECTION hDT, RECT *prcEdgeMask);

  /* Sets/Gets the tracking accuracy */
  INT32 (*FACEPROC_DT_MV_SetAccuracy)(HDETECTION hDT, INT32  nAccuracy);
  INT32 (*FACEPROC_DT_MV_GetAccuracy)(HDETECTION hDT, INT32* pnAccuracy);

  /* Set/Get the angle extension for Face Detection during tracking */
  INT32 (*FACEPROC_DT_MV_SetAngleExtension)(HDETECTION hDT, BOOL  bExtension);
  INT32 (*FACEPROC_DT_MV_GetAngleExtension)(HDETECTION hDT, BOOL* pbExtension);

  /* Set/Get whether to use or not the angle extension for Face Detection during tracking */
  INT32 (*FACEPROC_DT_MV_SetPoseExtension)(HDETECTION hDT, BOOL  bExtension, BOOL  bUseHeadTracking);
  INT32 (*FACEPROC_DT_MV_GetPoseExtension)(HDETECTION hDT, BOOL* pbExtension, BOOL* pbUseHeadTracking);

  /* Sets/Gets a direction mask for New Face Search */
  INT32 (*FACEPROC_DT_MV_SetDirectionMask)(HDETECTION hDT, BOOL  bMask);
  INT32 (*FACEPROC_DT_MV_GetDirectionMask)(HDETECTION hDT, BOOL* pbMask);
} fpsww_dt_fptr_t;

/** fpsww_pt_fptr_t
 *   Function pointers for Points detection(PT) library functions.
 **/
typedef struct {
  /* Get Facial Parts Detection Library API Version */
  INT32 (*FACEPROC_PT_GetVersion)(UINT8 *pucMajor, UINT8 *pucMinor);

 /* Create/Delete Facial Parts Detection Handle */
  HPOINTER (*FACEPROC_PT_CreateHandle)(void);
  INT32 (*FACEPROC_PT_DeleteHandle)(HPOINTER hPT);

  /* Create/Delete Facial Parts Detection Result Handle */
  HPTRESULT (*FACEPROC_PT_CreateResultHandle)(void);
  INT32 (*FACEPROC_PT_DeleteResultHandle)(HPTRESULT hPtResult);

  /* Set face position from Face detection result */
  INT32 (*FACEPROC_PT_SetPositionFromHandle)(HPOINTER hPT, HDTRESULT hDtResult,INT32 nIndex);
  /* Set face position */
  INT32 (*FACEPROC_PT_SetPosition)(HPOINTER hPT, POINT *pptLeftTop, POINT *pptRightTop, POINT *pptLeftBottom,
                                   POINT *pptRightBottom, INT32 nPose, DTVERSION DtVersion);
  /* Set face position from Motion face detection IP result */
  INT32 (*FACEPROC_PT_SetPositionIP)(HPOINTER hPT, INT32 nCenterX, INT32 nCenterY, INT32 nSize, INT32 nAngle,
                                     INT32 nScale, INT32 nPose, DTVERSION DtVersion);

  /* Set/Get Facial Parts Detection Mode */
  INT32 (*FACEPROC_PT_SetMode)(HPOINTER hPT, INT32 nMode);
  INT32 (*FACEPROC_PT_GetMode)(HPOINTER hPT, INT32 *pnMode);

  /* Set/Get Confidence calculation Mode */
  INT32 (*FACEPROC_PT_SetConfMode)(HPOINTER hPT, INT32 nConfMode);
  INT32 (*FACEPROC_PT_GetConfMode)(HPOINTER hPT, INT32 *pnConfMode);

  /* Execute Facial Parts Detection */
  INT32 (*FACEPROC_PT_DetectPoint)(HPOINTER hPT, RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight, HPTRESULT hPtResult);

  /* Get Facial Parts Position Result */
  INT32 (*FACEPROC_PT_GetResult)(HPTRESULT hPtResult, INT32 nPointNum, POINT aptPoint[], INT32 anConfidence[]);
  /* Get the face direction angles(degree) detected by FACEPROC_Pointer() */
  INT32 (*FACEPROC_PT_GetFaceDirection)(HPTRESULT hPtResult, INT32 *pnUpDown, INT32 *pnLeftRight, INT32 *pnRoll);
} fpsww_pt_fptr_t;

/** fpsww_ct_fptr_t
 *   Function pointers for Contour detection(CT) library functions.
 **/
typedef struct {
 /* Get Version Information */
  INT32 (*FACEPROC_CT_GetVersion)(UINT8 *pucMajor, UINT8 *pucMinor);

  /* Create/Delete Face Contour Detection Handle */
  HCONTOUR (*FACEPROC_CT_CreateHandle)(void);
  INT32 (*FACEPROC_CT_DeleteHandle)(HCONTOUR hCT);

  /* Create/Delete Face Contour Detection Result Handle */
  HCTRESULT (*FACEPROC_CT_CreateResultHandle)(void);
  INT32 (*FACEPROC_CT_DeleteResultHandle)(HCTRESULT hCtResult);

  /* Set Feature Points */
  INT32 (*FACEPROC_CT_SetPoint)(HCONTOUR hCT, INT32 nPointNum, POINT aptPoint[], INT32 anConfidence[],
                                INT32 nUpDown, INT32 nLeftRight, INT32 nRoll);
  INT32 (*FACEPROC_CT_SetPointFromHandle)(HCONTOUR hCT, HPTRESULT hPtResult);

  /* Execute Face Contour Detection */
  INT32 (*FACEPROC_CT_DetectContour)(HCONTOUR hCT, RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight, HCTRESULT hCtResult);

  /* Get Result of Face Contour Detection */
  INT32 (*FACEPROC_CT_GetResult)(HCTRESULT hCtResult, INT32 nPointNum, POINT aptCtPoint[]);

  /* Set/Get Face Contour Detection Mode */
  INT32 (*FACEPROC_CT_SetDetectionMode)(HCONTOUR hCT, INT32 nMode);
  INT32 (*FACEPROC_CT_GetDetectionMode)(HCONTOUR hCT, INT32 *pnMode);
} fpsww_ct_fptr_t;

/** fpsww_sm_fptr_t
 *   Function pointers for Smile detection(SM) library functions.
 **/
typedef struct {
  /* Get Smile Degree Estimation Library API Version */
  INT32 (*FACEPROC_SM_GetVersion)(UINT8 *pbyMajor, UINT8 *pbyMinor);

  /* Create/Delete Smile Degree Estimation handle */
  HSMILE (*FACEPROC_SM_CreateHandle)(void);
  INT32 (*FACEPROC_SM_DeleteHandle)(HSMILE hSM);

  /* Create/Delete Smile Degree Estimation result handle */
  HSMRESULT (*FACEPROC_SM_CreateResultHandle)(void);
  INT32 (*FACEPROC_SM_DeleteResultHandle)(HSMRESULT hSmResult);

  /* Set the feature points for Smile Degree Estimation */
  INT32 (*FACEPROC_SM_SetPoint)(HSMILE hSM, INT32 nPointNum, POINT aptPoint[], INT32 anConfidence[],
                                INT32 nUpDown, INT32 nLeftRight);
  /* Set the feature points for Smile Degree Estimation from PT result handle */
  INT32 (*FACEPROC_SM_SetPointFromHandle)(HSMILE hSM, HPTRESULT hPtResult);

  /* Estimate the smile degree */
  INT32 (*FACEPROC_SM_Estimate)(HSMILE hSM, RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight, HSMRESULT hSmResult);

  /* Get the estimated smile degree and its confidence level */
  INT32 (*FACEPROC_SM_GetResult)(HSMRESULT hSmResult, INT32 *pnSmile, INT32 *pnConfidence);
} fpsww_sm_fptr_t;

/** fpsww_gb_fptr_t
 *   Function pointers for GazeBlink detection(GB) library functions.
 **/
typedef struct {
  /* Get Version */
  INT32 (*FACEPROC_GB_GetVersion)(UINT8 *pucMajor, UINT8 *pucMinor);

  /* Create/Delete Gaze Blink Estimation handle */
  HGAZEBLINK (*FACEPROC_GB_CreateHandle)(void);
  INT32 (*FACEPROC_GB_DeleteHandle)(HGAZEBLINK hGB);

  /* Create/Delete Gaze Blink Estimation result handle */
  HGBRESULT (*FACEPROC_GB_CreateResultHandle)(void);
  INT32 (*FACEPROC_GB_DeleteResultHandle)(HGBRESULT hGbResult);

  /* Set facial parts postion */
  INT32 (*FACEPROC_GB_SetPoint)(HGAZEBLINK hGB, INT32 nPointNum, POINT aptPoint[], INT32 anConfidence[],
                                INT32 nUpDown, INT32 nLeftRight);
  /* Set facial parts position from PT result handle */
  INT32 (*FACEPROC_GB_SetPointFromHandle)(HGAZEBLINK hGB, HPTRESULT hPtResult);

  /* Execute Gaze Blink Estimation */
  INT32 (*FACEPROC_GB_Estimate)(HGAZEBLINK hGB, RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight, HGBRESULT hGbResult);

  /* Get the estimation result for blink */
  INT32 (*FACEPROC_GB_GetEyeCloseRatio)(HGBRESULT hGbResult, INT32 *pnCloseRatioLeftEye, INT32 *pnCloseRatioRightEye);
  /* Get the estimation result for gaze */
  INT32 (*FACEPROC_GB_GetGazeDirection)(HGBRESULT hGbResult, INT32 *pnGazeLeftRight, INT32 *pnGazeUpDown);
} fpsww_gb_fptr_t;

/** fpsww_fr_fptr_t
 *   Function pointers for Face Recognization(FR) library functions.
 **/
typedef struct {
  /* Gets Version */
  INT32 (*FACEPROC_FR_GetVersion)(UINT8 *pbyMajor, UINT8 *pbyMinor);

  /* Create/Delete Face Feature Data Handle */
  HFEATURE (*FACEPROC_FR_CreateFeatureHandle)(void);
  INT32 (*FACEPROC_FR_DeleteFeatureHandle)(HFEATURE hFeature);

  /* Extracts Face Feature from Facial Part Positions and the Image */
  INT32 (*FACEPROC_FR_ExtractFeature)(HFEATURE hFeature, RAWIMAGE *pImage, INT32 nWidth, INT32 nHeight,
                                      INT32 nPointNum, POINT aptPoint[], INT32 anConfidence[]);
  /* Extracts Face Feature from Facial Parts Detection Result Handle ans the Image */
  INT32 (*FACEPROC_FR_ExtractFeatureFromPtHdl)(HFEATURE hFeature, RAWIMAGE *pImage,
                                               INT32 nWidth, INT32 nHeight, HPTRESULT hPtResult);

  /* Writes Face Feature Data into Memory */
  INT32 (*FACEPROC_FR_WriteFeatureToMemory)(HFEATURE hFeature, UINT8 *pbyBuffer, UINT32 unBufSize);
  /* Reads Face Feature Data from Memory */
  INT32 (*FACEPROC_FR_ReadFeatureFromMemory)(HFEATURE hFeature, UINT8 *pbyBuffer, UINT32 unBufSize, FR_ERROR *pError);

  /* Creates Album Data Handle */
  HALBUM (*FACEPROC_FR_CreateAlbumHandle)(INT32 nMaxUserNum, INT32 nMaxDataNumPerUser);
  /* Deletes Album Data Handle */
  INT32 (*FACEPROC_FR_DeleteAlbumHandle)(HALBUM hAlbum);

  /* Gets the maximal numbers of Users and Feature Data per User */
  INT32 (*FACEPROC_FR_GetAlbumMaxNum)(HALBUM hAlbum, INT32 *pnMaxUserNum, INT32 *pnMaxDataNumPerUser);

  /* Registers Face Feature Data into Album Data */
  INT32 (*FACEPROC_FR_RegisterData)(HALBUM hAlbum, HFEATURE hFeature, INT32 nUserID, INT32 nDataID);

  /* Gets the total number of Data registered in Album Data */
  INT32 (*FACEPROC_FR_GetRegisteredAllDataNum)(HALBUM hAlbum, INT32 *pnAllDataNum);
  /* Gets the number of Users with registered Data */
  INT32 (*FACEPROC_FR_GetRegisteredUserNum)(HALBUM hAlbum, INT32 *pnUserNum);
  /* Gets the number of registered Feature Data of a specified User */
  INT32 (*FACEPROC_FR_GetRegisteredUsrDataNum)(HALBUM hAlbum, INT32 nUserID, INT32 *pnUserDataNum);
  /* Gets Data Registration Status of a specified User */
  INT32 (*FACEPROC_FR_IsRegistered)(HALBUM hAlbum, INT32 nUserID, INT32 nDataID, BOOL *pIsRegistered);

  /* Clears all the Data from Album Data */
  INT32 (*FACEPROC_FR_ClearAlbum)(HALBUM hAlbum);
  /* Clears all the Data of a specified User from Album Data */
  INT32 (*FACEPROC_FR_ClearUser)(HALBUM hAlbum, INT32 nUserID);
  /* Clears a specified Data of a specified User from Album Data */
  INT32 (*FACEPROC_FR_ClearData)(HALBUM hAlbum, INT32 nUserID, INT32 nDataID);

  /* Gets the size of Serialized Album Data  */
  INT32 (*FACEPROC_FR_GetSerializedAlbumSize)(HALBUM hAlbum, UINT32 *punSerializedAlbumSize);
  /* Serializes Album Data */
  INT32 (*FACEPROC_FR_SerializeAlbum)(HALBUM hAlbum, UINT8 *pbyBuffer, UINT32 unBufSize);
  /* Restores Album Data */
  HALBUM (*FACEPROC_FR_RestoreAlbum)(UINT8 *pbyBuffer, UINT32 unBufSize, FR_ERROR *pError);

  /* Gets Feature Data from Album Data */
  INT32 (*FACEPROC_FR_GetFeatureFromAlbum)(HALBUM hAlbum, INT32 nUserID, INT32 nDataID, HFEATURE hFeature);

  /* Verification */
  INT32 (*FACEPROC_FR_Verify)(HFEATURE hFeature, HALBUM hAlbum, INT32 nUserID, INT32 *pnScore);
  /* Identification */
  INT32 (*FACEPROC_FR_Identify)(HFEATURE hFeature, HALBUM hAlbum, INT32 nMaxResultNum,
                                        INT32 anUserID[], INT32 anScore[], INT32 *pnResultNum);
} fpsww_fr_fptr_t;

/** fpsww_co_params_t
 *   @majorVersion : Major version of SW CO library
 *   @minorVersion : Minor version of SW CO library
 *
 *   SW library CO config params. Currently do not have any. Versions are read only.
 **/
typedef struct {
  UINT8 majorVersion;
  UINT8 minorVersion;
} fpsww_co_params_t;

/** fpsww_dt_params_t
 *   @majorVersion : Major version of SW DT library. Read only param.
 *   @minorVersion : Minor version of SW DT library. Read only param.
 *   @nMinSize : Minimum Face size for detection.
 *               Range : 20 - 8192. Default : 40
 *               Affect on performance : smaller the size, longer the processing time.
 *               Affect on Detection   : smaller the size, more the detection rate.
 *               Applies to (affects)
 *                 STILL Mode : Whole face search : Yes
 *                 MV Mode    : Face Search       : Yes (Initial face search), Yes (New face search)
 *                              Tracking          : No
 *               Exa : If nMinSize is set to 100, faces of size above 100x100 pixels will be detected.
 *   @nMaxSize : Maximum Face size for detection.
 *               Range : 20 - 8192. Default : 8192
 *               Affect on performance : larger the size, longer the processing time.
 *               Affect on Detection   : larger the size, more the detection rate.
 *               Applies to (affects)
 *                 SILL Mode  : Whole face search : Yes
 *                 MV Mode    : Face Search       : Yes (Initial face search), Yes (New face search)
 *                              Tracking          : No
 *                   Exa : If nMaxSize is set to 200, faces of size below 200x200 pixels will be detected.
 *   @nAngle : Detection angles for whole face search and face search. Array of angle values for different profiles.
 *               POSE_ANGLE_FRONT        - index 0
 *               POSE_ANGLE_HALF_PROFILE - index 1
 *               POSE_ANGLE_FULL_PROFILE - index 2
 *             Range : see the ROLL_ANGLE_xxx macros. (0 to 360 degrees).
 *             Default :
 *               POSE_ANGLE_FRONT        - ROLL_ANGLE_UP
 *               POSE_ANGLE_HALF_PROFILE - ROLL_ANGLE_NONE
 *               POSE_ANGLE_FULL_PROFILE - ROLL_ANGLE_NONE
 *             Affect on performance : larger the angles, longer the processing time.
 *             Affect on Detection   : larger the angles, higher the detection rate with faces angled in different directions.
 *             Applies to (affects)
 *               STILL Mode : Whole face search : Yes
 *               MV Mode    : Face Search       : Yes (Initial face search), Yes (New face search)
 *                            Tracking          : No
 *             Exa : If ROLL_ANGLE_UP is set for POSE_ANGLE_FRONT, front facing faces with -15 to +15 degress are detected.
 *   @faceSearchEdgeMask : Search area for whole face search and face search (initial and new).
 *                           Crop area to restrict the area targeted for face search.
 *                         Range : -1 or 0 - 8191 for each element. Default : -1 for all (full frame)
 *                         Affect on performance : smaller the crop area, faster the face search processing time.
 *                         Affect on Detection   : smaller the crop area, lesser the detection rate.
 *                         Applies to (affects)
 *                                STILL Mode : Whole face search : Yes
 *                                MV Mode    : Face Search       : Yes (Initial face search), Yes (New face search)
 *                                             Tracking          : No
 *                         Exa : If Frame is of 500x500, setting left=50, top=50, right=450, bottom=450 will result in
 *                               detecting the faces within the region (50,50) to (450, 450) while face search
 *   @nSearchDensity : Search density for whole face search and face search(initial and new).
 *                     Range : one of (DENSITY_HIGHEST, DENSITY_HIGH, DENSITY_NORMAL, DENSITY_LOW, DENSITY_LOWEST).
 *                     Default : DENSITY_NORMAL
 *                     Affect on performance : more the search density, more the processing time.
 *                     Affect on Detection   : more the search density, more the detection rate and more false positives.
 *                     Applies to (affects)
 *                       STILL Mode : Whole face search : Yes
 *                       MV Mode    : Face Search       : Yes (Initial face search), Yes (New face search)
 *                                    Tracking          : No
 *                     Exa : setting HIGHEST, takes more time for processing and detects more faces.
 *   @nThreshold : Faces with a degree of confidence lower than 'nThreshold' value will not be output.
 *                 Range : 1 to 1000. Default : 500(STILL MODE), 700 (MOVIE_MODE)
 *                 Affect on performance : nothing
 *                 Affect on Detection   : Higher the value, lesser the false positives and also decreases the detection rate.
 *                 Applies to (affects)
 *                   STILL Mode : Whole face search : Yes
 *                   MV Mode    : Face Search       : Yes (Initial face search), Yes (New face search)
 *                                Tracking          : Yes
 *                 Exa : setting threshold to 600, library will only give the result faces having confidence greater than 600.
 *   @nInitialFaceSearchCycle : Initial face search cycle i.e the number of frames required to complete initial face search.
 *                              Range : 1 to 45. Default : 3
 *                              Affect on performance : smaller the value, longer the processing time of each frame.
 *                              Affect on Detection   : smaller the value, need more frames to complete one search cycle.
 *                              Applies to (affects)
 *                                STILL Mode : Whole face search : No
 *                                MV Mode    : Face Search       : Yes (Initial face search), No (New face search)
 *                                             Tracking          : No
 *                              Exa : If set to 3, search area is divided into 3 parts and each part is processed in one frame
 *                                    i.e need 3 frames to complete the initial face search for the given FOV.
 *   @nNewFaceSearchCycle : New face search cycle i.e the number of frames required to complete new face search.
 *                          Range : 1 to 45. Default : 15
 *                          Affect on performance : smaller the value, longer the processing time of each frame.
 *                          Affect on Detection   : smaller the value, need more frames to complete one search cycle.
 *                          Applies to (affects)
 *                            STILL Mode : Whole face search : No
 *                            MV Mode    : Face Search       : No (Initial face search), Yes (New face search)
 *                                         Tracking          : No
 *                          Exa : If set to 15, search area is divided into 15 parts and each part is processed in one frame.
 *                                i.e need 15 frames to complete the new face search for the give FOV.
 *   @nNewFaceSearchInterval : Search interval for new face search. These number of frames are skipped for new face search
 *                               between each new face search cycle.
 *                             Range : -1 to 45. Default : 0
 *                             Affect on performance : smaller the value, smaller the no.of frame skipped.
 *                             Affect on Detection   : smaller the value, smaller the delay in detecting new faces.
 *                             Applies to (affects)
 *                               STILL Mode : Whole face search : No
 *                               MV Mode    : Face Search       : No (Initial face search), Yes (New face search)
 *                                            Tracking          : No
 *                             Exa : If value is set to 2, 2 frames are skipped in between two new face search cycles.
 *                             Exa (for above 3 params) :
 *                               setting nInitialFaceSearchCycle=3, nNewFaceSearchCycle=15, nNewFaceSearchInterval=2 :
 *                               Library runs full face search on partial regions of frame 1,2,3 and then runs
 *                               new face search on 4 to 18 frames, no face searches on 19, 20 (only tracking on 19,20)
 *                               and then new face search on 21 to 35 frames, skips face search on 36, 37 and so on.
 *   @nMaxRetryCount : Max retry count for when a face is lost during tracking in Movie mode.
 *                       Indicates the no.of frames to go forward while searching for a face that was lost during tracking,
 *                       if tracking fails in following 'nMaxRetryCount' frames, the face will be considered lost then
 *                       and tracking will end there.
 *                     Range : 0 to 300. Default : 2
 *                     Affect on performance : greater the number, greater the processing in keeping track of lost faces.
 *                     Affect on Detection   : greater the number, better the stability and also greater the delay in
 *                                             removing a face from tracker that is actually out of FOV now.
 *                     Applies to (affects)
 *                       STILL Mode : Whole face search : No
 *                       MV Mode    : Face Search       : No (Initial face search), No (New face search)
 *                                    Tracking          : Yes
 *                     Exa : If set to 2, the face will be removed from tracking only if it is still not found
 *                           in the following 2 frames.
 *   @nMaxHoldCount : Max hold count for when a face is lost during tracking in Movie mode.
 *                      Indicates the no.of frames to keep outputting the results info of a face that was lost
 *                      during tracking. The output info is the last info when the face was detected.
 *                    Range : 0 to nMaxRetryCount. Default : 2
 *                    Affect on performance : nothing
 *                    Affect on Detection   : greater the number, better the stability and also greater the delay in
 *                                            removing a face from results that is actually out of FOV now.
 *                    Applies to (affects)
 *                      STILL Mode : Whole face search : No
 *                      MV Mode    : Face Search       : No (Initial face search), No (New face search)
 *                                   Tracking          : Yes
 *                    Exa : If value is set to 2, the face will be removed from results info only if it is still not found
 *                          in the following 2 frames.
 *   @nPosSteadinessParam : Steadiness param for the position of the results rectangle during tracking.
 *                            Allows the library to smoothly change the position of the result rectangle. If the
 *                            ratio of displacement is below the set value, the center is set back to the previous center.
 *                          Range : 0 to 30. Default : 10
 *                          Affect on performance : nothing
 *                          Affect on Detection   : greater the number, less the fluctuations in center result and
 *                                                  also greater the jump when center is changed beyond the set value.
 *                          Applies to (affects)
 *                            STILL Mode : Whole face search : No
 *                            MV Mode    : Face Search       : No (Initial face search), No (New face search)
 *                                         Tracking          : Yes
 *                          Exa : If value is set to 15, the center given in result info will not be changed
 *                                until the displacement is 15% in actual FOV.
 *   @nSizeSteadinessParam : same as nPosSteadinessParam, it describes the size steadiness
 *   @nDelayCount : Delay count used to output only continuously detected faces.
 *                  Allows the library to suppress false detection results like those of faces that appeared in only 1 frame.
 *                  Range : 0 to 10. Default : 0
 *                  Affect on performance : nothing
 *                  Affect on Detection   : greater the number, better the accuracy and longer the delay in outputting a face.
 *                  Applies to (affects)
 *                    STILL Mode : Whole face search : No
 *                    MV Mode    : Face Search       : Yes (Initial face search), Yes (New face search)
 *                                 Tracking          : No
 *                  Exa : setting the delay count to 2 will make library only output detection results for faces
 *                        that were detected in 3 consecutive frames.
 *   @nTrackingSwapParam : Parameter used when swapping faces during tracking. Allows the library to decide which faces are
 *                           to be discarded when the number of faces detected exceeds the max detection count.
 *                         Range : 100 to 10000. Default : 400
 *                         Affect on performance : nothing
 *                         Affect on Detection   : Greater the value, more is the chance that we discard the new face.
 *                         Applies to (affects)
 *                           STILL Mode : Whole face search : No
 *                           MV Mode    : Face Search       : No (Initial face search), Yes (New face search)
 *                                        Tracking          : Yes
 *                         Exa : If nTrackingSwapParam=200, max_detect_count=2, the 2 faces with sizes 60, 50 being tracked;
 *                               when new face search finds
 *                                1) 1 face with size 90 : this face will be discarded. (since 90 < 200% of 50)
 *                                2) 1 face with size 100 : the face with size 50 will be discarded. (since 100 > 200% of 50)
 *                                3) 2 faces with size 130, 121 : the faces with sizes 50, 60 will be discarded.
 *                                   (since 130, 121 > 200% of 50, 60)
 *                                4) 2 faces with size 130, 120 : the faces with sizes 50, 120 will be discarded.
 *                                   (first check 130 > 200% of 50, so discard 50, then checks 120 > 200% 60, so discard 120)
 *   @trackingEdgeMask : Search area for tracking. Crop area to restrict the area targeted for face tracking.
 *                       Range : -1 or 0 - 8191 for each element. Default : -1 for all (full frame)
 *                       Affect on performance : smaller the crop area, faster the tracking processing time.
 *                       Affect on Detection   : smaller the crop area, lesser the tracking detection rate.
 *                       Applies to (affects)
 *                         STILL Mode : Whole face search : No
 *                         MV Mode    : Face Search       : No (Initial face search), No (New face search)
 *                                      Tracking          : Yes
 *                       Exa : If Frame is of 500x500, setting left=50, top=50, right=450, bottom=450 will result in
 *                             detecting the faces within the region (50,50) to (450, 450) while tracking
 *   @nAccuracy : Accuracy used during tracking.
 *                Range : TRACKING_ACCURACY_NORMAL or TRACKING_ACCURACY_HIGH. Default : TRACKING_ACCURACY_NORMAL
 *                Affect on performance : Tracking with high accuracy will increase the processing time per fame
 *                Affect on Detection   : Tracking with high accuracy will decrease false detections
 *                Applies to (affects)
 *                  STILL Mode : Whole face search : No
 *                  MV Mode    : Face Search       : No (Initial face search), No (New face search)
 *                               Tracking          : Yes
 *                Exa : set ACCURACY_NORMAL to track with normal accuracy. ACCURACY_HIGH to track with high accuracy.
 *   @rollAngleExtension : Boolean flag to set restriction on roll angle while tracking.
 *                           set to TRUE, to activate the roll angle extension during tracking. The library will extend the
 *                             roll angle used when tracking a face to an additional roll angle to the left and right.
 *                           set to FALSE, to not to use roll angle extension.
 *                         Range : TRUE or FALSE. Default : TRUE.
 *                         Affect on performance : setting TRUE will increase the processing time a little.
  *                        Affect on Detection   : setting to TRUE may improve tracking a face with little angle changes.
 *                         Applies to (affects)
 *                           STILL Mode : Whole face search : No
 *                           MV Mode    : Face Search       : No (Initial face search), No (New face search)
 *                                        Tracking          : Yes
 *                         Exa : If set to TRUE, for a face detected with angle 0, tracking uses adjacent angles
 *                               also while tracking.
 *   @yawAngleExtension : same as rollAngleExtension. used to set flag for yaw angle.
 *   @bUseHeadTracking  : flag whether to activate head tracking.
 *   @bMask   : Boolean flag to set restriction on the detection roll angles for new face search.
 *                If bMask is set to TRUE, the library will restrict the search area for new face search to the detection angle
 *                  in which a face is tracked along with its left and right adjascent detection angles.
 *                If bMask is set to FALSE, new face search will be done on all angles set in 'nAngle'
 *              Range : TRUE or FALSE. Default : TRUE.
 *              Affect on performance : setting to TRUE will improve 'new face search' processing time.
 *                                      setting FALSE, keep the processing time inline(same) with 'Initial face search'
 *              Affect on Detection   : setting to TRUE may not detect the current face if
 *                                      face's angle has changed a lot in the FOV.
 *              Applies to (affects)
 *                STILL Mode : Whole face search : No
 *                MV Mode    : Face Search       : No (Initial face search), Yes (New face search)
 *                             Tracking          : No
 *              Exa : If Roll angle in nAngle is set as ANGLE_ALL and bMask is set to TRUE, if a face is detected in
 *                    'initial face search' and being tracked with angle_0, detection roll angle for 'new face search'
 *                    is masked to ANGLE_11 | ANGLE_0 | ANGLE_1  (do not search for ALL angles).
 *
 *   SW library DT config params that are tunable.
 **/
typedef struct {
  UINT8  majorVersion;
  UINT8  minorVersion;
  INT32  nMinSize;
  INT32  nMaxSize;
  UINT32 nAngle[FPSWW_MAX_POSE_ANGLES];
  RECT   faceSearchEdgeMask;
  INT32  nSearchDensity;
  INT32  nThreshold;
  INT32  nInitialFaceSearchCycle;
  INT32  nNewFaceSearchCycle;
  INT32  nNewFaceSearchInterval;
  INT32  nMaxRetryCount;
  INT32  nMaxHoldCount;
  INT32  nPosSteadinessParam;
  INT32  nSizeSteadinessParam;
  INT32  nTrackingSwapParam;
  INT32  nDelayCount;
  RECT   trackingEdgeMask;
  INT32  nAccuracy;
  BOOL   rollAngleExtension;
  BOOL   yawAngleExtension;
  BOOL   bUseHeadTracking;
  BOOL   bMask;
} fpsww_dt_params_t;


/* PT
  Supported properties :
    Image/Frame size Range : 64x64 to 8192x8192
    Face size Range        : Minimum : 64 pixels
                             Maximum : less than the shorter side of the image, in pixels
    Face Inclination (roll angle): 360 degrees
    Face Direction (yaw and pitch angles) :
        Default mode    - Left-Right : +/- 30 degress, Up-Down : +/- 20 degrees
        Fast mode       - Left-Right : +/- 30 degress, Up-Down : +/- 15 degrees
        Super Fast mode - Left-Right : +/- 20 degress, Up-Down : +/- 15 degrees
    Format : 8-bit grayscale image in raw format with top left corner as the
             starting point.
  Memory size requirements :
*/

/** fpsww_pt_params_t
 *   @majorVersion : Major version of SW PT library. Read only param.
 *   @minorVersion : Minor version of SW PT library. Read only param.
 *   @nMode        : Mode in which PT will be running
 *                   Range   : one of PT_MODE_DEFAULT, PT_MODE_FAST, PT_MODE_SUPER_FAST
 *                   Default : PT_MODE_DEFAULT
 *   @nConfMode    : Whether the Confidence Mode will be used or not during
 *                   Range   : one of PT_CONF_USE, PT_CONF_NOUSE
 *                   Default : PT_CONF_USE
 *
 *   SW library PT config params that are tunable.
 **/
typedef struct {
  UINT8 majorVersion;
  UINT8 minorVersion;
  INT32 nMode;
  INT32 nConfMode;
} fpsww_pt_params_t;

/* CT
  Supported properties :
    Image/Frame size Range : 64x64 to 8192x8192
    Face size Range        : Minimum : 64 pixels
                             Maximum : less than the shorter side of the image, in pixels
    Face Inclination (roll angle): 360 degrees
    Face Direction (yaw and pitch angles) :
        Left-Right : +/- 10 degress, Up-Down : +/- 10 degrees
    Format : 8-bit grayscale image in raw format with top left corner as the
             starting point.
             YCbCr 4:2:2
             Grayscale y (y3y2y1y0y7y6y5y4...)
  Memory size requirements :
*/

/** fpsww_ct_params_t
 *   @majorVersion : Major version of SW CT library. Read only param.
 *   @minorVersion : Minor version of SW CT library. Read only param.
 *   @nMode        : Mode in which CT will be running
 *                   Range   : one of CT_DETECTION_MODE_DEFAULT, CT_DETECTION_MODE_EYE
 *                   Default : CT_DETECTION_MODE_DEFAULT
 *
 *   SW library CT config params that are tunable.
 **/
typedef struct {
  UINT8 majorVersion;
  UINT8 minorVersion;
  INT32 nMode;
} fpsww_ct_params_t;

/** fpsww_sm_params_t
 *   @majorVersion : Major version of SW SM library. Read only param.
 *   @minorVersion : Minor version of SW SM library. Read only param.
 *
 *   SW library SM config params (nothing tunable currenlty).
 **/
typedef struct {
  UINT8 majorVersion;
  UINT8 minorVersion;
} fpsww_sm_params_t;

/** fpsww_gb_params_t
 *   @majorVersion : Major version of SW GB library. Read only param.
 *   @minorVersion : Minor version of SW GB library. Read only param.
 *
 *   SW library GB config params (nothing tunable currenlty).
 **/
typedef struct {
  UINT8 majorVersion;
  UINT8 minorVersion;
} fpsww_gb_params_t;

/** fpsww_sm_params_t
 *   @majorVersion : Major version of SW FR library. Read only param.
 *   @minorVersion : Minor version of SW FR library. Read only param.
 *
 *   SW library FR config params (nothing tunable currenlty).
 **/
typedef struct {
  UINT8 majorVersion;
  UINT8 minorVersion;
} fpsww_fr_params_t;

/** fpsww_filter_params_t
 *   @closed_eye_ratio              : min closed eye ration determine whether to determine blink detected
 *   @weight_mouth                  : mouth confidence weight considering to filter out facial parts
 *   @weight_eyes                   : eye confidence weight considering to filter out facial parts
 *   @weight_nose                   : nose confidence weight considering to filter out facial parts
 *   @weight_face                   : face confidence weight considering to filter out facial parts
 *   @eyes_use_max_filter           : Whether to use maximum confidence value of right, left eye. If not set, avg is used.
 *   @nose_use_max_filter           : Whether to use maximum confidence value of right, left nose. If not set, avg is used.
 *   @discard_face_below            : Threshold value to discard faces having confidence less than this.
 *   @discard_facialparts_threshold : Threshold value to discard facial parts having confidence less than this.
 *   @discard_threshold             : Threshold value to discard facial parts having confidence based on facial parts weights
 *                                    less than this.
 *   @sw_face_size_perc             : face size percentage to calculte min, max face size to set in dt config
 *   @sw_face_box_border_per        : Additional box area to set as CROP rect for face detection dt config
 *   @sw_face_spread_tol            : face spread tolerance that can be allowed while accepting the face
 *   @sw_face_discard_border        : Params used to determine sw_face_discard_out
 *   @sw_face_discard_out           : Whether to discard faces for which face:()x + dx) is less than
 *                                    (frame_height - sw_face_discard_border)
 *   @min_threshold                 : Minimum confidence of faces to determine whether to ignore face or not.
 *   @enable_fp_false_pos_filtering : Whether to enable facial parts false positive filtering.
 *   @enable_sw_false_pos_filtering : Whether to enable sw false positive filtering.
 *   @lock_faces                    : Whether to lock faces during tracking that were detected. Locked faces will not be ignored
 *                                    if num_faces detected exceeds max number of faces
 *
 *   SW Wrapper filter config params, used inside wrapper to program SW library config params and to filter results internally.
 **/
typedef struct {
  int      closed_eye_ratio;
  float    weight_mouth;
  float    weight_eyes;
  float    weight_nose;
  float    weight_face;
  bool     eyes_use_max_filter;
  bool     nose_use_max_filter;
  uint32_t discard_face_below;
  uint32_t discard_facialparts_threshold;
  uint32_t discard_threshold;
  uint32_t sw_face_size_perc;
  uint32_t sw_face_box_border_per;
  float    sw_face_spread_tol;
  uint32_t sw_face_discard_border;
  uint32_t sw_face_discard_out;
  int      min_threshold;
  bool     enable_fp_false_pos_filtering;
  bool     enable_sw_false_pos_filtering;
  bool     lock_faces;
  int      sw_face_search_dens;
} fpsww_filter_params_t;

/** fpsww_co_params_t
 *   @majorVersion  : Major version of DSP custom library
 *   @minorVersion  : Minor version of DSP custom library
 *   @absapi        : whether to use dsp abs api
 *   @dsp_clock     : dsp clock speed.
 *   @dsp_bus       : dsp bus.
 *   @dsp_latency   : dsp latency configuration.
 *   @dcvs_clock: dcvs dsp target corner clock Range 0 to 6
 *   @dcvs_min_clock: dcvs dsp minimum corner frequency
 *
 *   DSP custom config params.
 **/
typedef struct {
  UINT8 majorVersion;
  UINT8 minorVersion;

  INT32 absapi;
  INT32 clock;
  INT32 bus;
  INT32 latency;
  INT32 dcvs_clock;
  INT32 dcvs_enable;
  INT32 dcvs_min_clock;
} fpsww_dsp_params_t;

/** fpsww_config_t
 *   @enable_facial_parts : Whether to enable facial parts execution, results
 *   @enable_contour      : Whether to enable contour execution, results
 *   @enable_smile        : Whether to enable smile execution, results
 *   @enable_gaze         : Whether to enable gaze execution, results
 *   @enable_blink        : Whether to enable blink execution, results
 *   @enable_recog        : Whether to enable face recognization execution, results
 *
 *   @co_params : CO config params
 *   @dt_params : DT config params
 *   @pt_params : PT config params
 *   @ct_params : CT config params
 *   @sm_params : SM config params
 *   @gb_params : GB config params
 *   @fr_params : FR config params
 *
 *   @dsp_params : DSP custom params
 *
 *   @filter_params : CO config params
 *
 *   SW Wrapper config params.
 **/
typedef struct {
  bool enable_facial_parts;
  bool enable_contour;
  bool enable_smile;
  bool enable_gaze;
  bool enable_blink;
  bool enable_recog;

  fpsww_co_params_t co_params;
  fpsww_dt_params_t dt_params;
  fpsww_pt_params_t pt_params;
  fpsww_ct_params_t ct_params;
  fpsww_sm_params_t sm_params;
  fpsww_gb_params_t gb_params;
  fpsww_fr_params_t fr_params;

  fpsww_dsp_params_t dsp_params;

  fpsww_filter_params_t filter_params;
} fpsww_config_t;

/** fpsww_dsp_fptr_t
 *   Function pointers for DSP custom functions.
 **/
typedef struct {
  INT32 (*FDDSP_GetVersion)(UINT8* pucMajor, UINT8* pucMinor);
  INT32 (*FDDSP_InitDetection)(void);
  INT32 (*FDDSP_DeInitDetection)(void);
  INT32 (*FDDSP_SetClockConfig)(INT32 clock, INT32 bus, INT32 latency, INT32 b_absapi,
                INT32 dcvs_clock,INT32 dcvs_min_clock, INT32 dcvs_enable);
  INT32 (*FDDSP_GetClockConfig)(INT32* p_clock, INT32* p_bus, INT32* p_latency, INT32* p_b_absapi,
                INT32* dcvs_clock, INT32* dcvs_min_clock, INT32* dcvs_enable);

  /* Executes detection */
  /* Buffer length need to be passed to DSP - additional Len params compared to ARM funcs*/
  INT32 (*FACEPROC_DT_Detect_GRAY)(HDETECTION hDT, RAWIMAGE* pImageGRAY, INT32 pImageGRAYLen, INT32 nWidth, INT32 nHeight,
                                   GRAY_ORDER ImageOrder, HDTRESULT hDtResult, INT32* pnCount);
  INT32 (*FACEPROC_DT_Detect_YUV422)(HDETECTION hDT, RAWIMAGE* pImageYUV, INT32 pImageYUVLen, INT32 nWidth, INT32 nHeight,
                                     YUV422_ORDER ImageOrder, HDTRESULT hDtResult, INT32* pnCount);
  INT32 (*FACEPROC_DT_Detect_YUV420SP)(HDETECTION hDT, RAWIMAGE* pImageY, INT32 pImageYLen, RAWIMAGE* pImageCx,
                                       INT32 pImageCxLen, INT32 nWidth, INT32 nHeight, YUV420SP_ORDER ImageOrder,
                                       HDTRESULT hDtResult, INT32* pnCount);
  INT32 (*FACEPROC_DT_Detect_YUV420FP)(HDETECTION hDT, RAWIMAGE* pImageY, INT32 pImageYLen,
                                       RAWIMAGE* pImageCb, INT32 pImageCbLen, RAWIMAGE* pImageCr, INT32 pImageCrLen,
                                       INT32 nWidth, INT32 nHeight, YUV420FP_ORDER ImageOrder, HDTRESULT hDtResult,
                                       INT32* pnCount);
  INT32 (*FACEPROC_DT_GetAllResultInfo)(HDTRESULT hDtResult,
                                        DETECTION_INFO* psDetectionInfo, INT32 psDetectionInfoLen);
  INT32 (*FACEPROC_DT_GetAllRawResultInfo)(HDTRESULT hDtResult,
                                           DETECTION_INFO* psDetectionInfo, INT32 psDetectionInfoLen);

  INT32 (*FACEPROC_DT_SetDTConfig)(HDETECTION hDT, INT32 detection_mode, const fpsww_dt_params_t* p_dt_params);
  INT32 (*FACEPROC_DT_GetDTConfig)(HDETECTION hDT, INT32 detection_mode, fpsww_dt_params_t* p_dt_new_params);
} fpsww_dsp_fptr_t;

/** fpsww_lib_t
 *   @p_lib_handle : Library handle
 *   @p_dsp_lib_handle : DSP Stub Library handle
 *   @co_funcs     : CO function pointer
 *   @dt_funcs     : DT function pointer
 *   @pt_funcs     : PT function pointer
 *   @ct_funcs     : CT function pointer
 *   @sm_funcs     : SM function pointer
 *   @gb_funcs     : GB function pointer
 *   @fr_funcs     : FR function pointer
 *   @dsp_funcs    : DSP function pointer
 *   @dsp_co_funcs : GB function pointer
 *   @dsp_dt_funcs : FR function pointer
 *
 *   SW library handle.
 **/
typedef struct {
  void *p_lib_handle;
  void *p_dsp_lib_handle;

  fpsww_co_fptr_t  co_funcs;
  fpsww_dt_fptr_t  dt_funcs;
  fpsww_pt_fptr_t  pt_funcs;
  fpsww_ct_fptr_t  ct_funcs;
  fpsww_sm_fptr_t  sm_funcs;
  fpsww_gb_fptr_t  gb_funcs;
  fpsww_fr_fptr_t  fr_funcs;

  fpsww_dsp_fptr_t dsp_funcs;
  fpsww_co_fptr_t  dsp_co_funcs;
  fpsww_dt_fptr_t  dsp_dt_funcs;
} fpsww_lib_t;

/** fpsww_lib_t
 *   @engine            : Client engine type
 *   @create_face_parts : Whether to create handles for facial parts features
 *                          If FALSE, cannot dynamically enable/run facial parts
 *                          If TRUE, can dynamically enable/run facial parts
 *   @create_face_recog : Whether to create handles for face recog
 *   @max_face_count    : Maximum no.of faces to be detected for face detection.
 *   @detection_mode    : Face detection mode.
 *                        STILL mode : Whole face search always.
 *                        MOVIE mode : Face search (initial + new) + Tracking
 *   @no_of_fp_handles  : How many handles to be created for facial parts results.
 *   @use_dsp_if_available  : Use dsp library if available.
 *
 *   Create parameters while creating SW Wrapper handle
 **/
typedef struct {
  faceproc_engine_t engine;
  bool              create_face_parts;
  bool              create_face_recog;
  uint32_t          max_face_count;
  int32_t           detection_mode;
  uint32_t          no_of_fp_handles;
  bool              use_dsp_if_available;
} fpsww_create_params_t;

/** faceproc_sw_wrap_t
 *   @create_params : Create params passed by Client while creating the wrapper handle
 *
 *   @hCo       : CO handle
 *   @hdt       : DT handle
 *   @hdtresult : DT Results handle
 *   @hpt       : PT handle
 *   @hptresult : PT Results handle per face
 *   @hct       : CT handle
 *   @hctresult : CT Results handle per face
 *   @hsm       : SM handle
 *   @hsmresult : SM Results handle per face
 *   @hgb       : GB handle
 *   @hgbresult : GB Results handle per face
 *   @hfeature  : FR handle
 *   @halbum    : Album handle
 *
 *   @p_lib    : Faceporc sw library lib handle
 *   @config   : Current config
 *   @width    : Current frame width
 *   @height   : Current frame height
 *   @frame_id : Current camera frame id
 *
 *   @debug_settings: FD specific debug settings
 *   @fd_profile: FD profile data
 *
 *   Main SW Wrapper handle
 **/
typedef struct {
  fpsww_create_params_t create_params;

  HCOMMON    hCo;
  HDETECTION hdt;
  HDTRESULT  hdtresult;
  HPOINTER   hpt;
  HPTRESULT  hptresult[MAX_FACE_ROI];
  HCONTOUR   hct;
  HCTRESULT  hctresult[MAX_FACE_ROI];
  HSMILE     hsm;
  HSMRESULT  hsmresult[MAX_FACE_ROI];
  HGAZEBLINK hgb;
  HGBRESULT  hgbresult[MAX_FACE_ROI];
  HFEATURE   hfeature;
  HALBUM     halbum;

  fpsww_lib_t   *p_lib;
  fpsww_config_t config;

  uint32_t width;
  uint32_t height;
  uint32_t frame_id;

  faceproc_debug_settings_t debug_settings;
  img_profiling_t fd_profile;
} faceproc_sw_wrap_t;

#endif //__FACEPROC_SW_WRAPPER_TYPES_H__
