/***************************************************************************
* Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
***************************************************************************/

#include <dlfcn.h>
#include <math.h>
#include "facial_parts_wrapper.h"
#include "faceproc_common.h"
#include "faceproc.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#define SW_PROFILE_THRES 50

/** FP_ADDR
 *   @p: pointer to the frame
 *
 *   Returns the Y address from the frame
 **/
#define FP_ADDR(p) (p->frame[0].plane[0].addr)

/** FP_WIDTH
 *   @p: pointer to the frame
 *
 *   Returns the Y plane width.
 *   In Face parts detection stride is not supported, return width as stride
 **/
#define FP_WIDTH(p) ((p)->frame[0].plane[0].stride)

/** FP_HEIGHT
 *   @p: pointer to the frame
 *
 *   Returns the Y plane height
 **/
#define FP_HEIGHT(p) (p->frame[0].plane[0].height)

/** FACIAL_PARTS_IS_WITHIN_RECT_BOUNDARY
 *   @rect: rectangle to check within
 *   @point: point with x y coordinate
 *
 *   Checks if point is in the rectangle.
**/
#define FACIAL_PARTS_IS_WITHIN_RECT_BOUNDARY(rect, point) \
  ((point.x >= (int32_t)rect.x) && (point.x < (int32_t)(rect.x + rect.dx)) \
  && (point.y >= (int32_t)rect.y) && (point.y < (int32_t)(rect.y + rect.dy)))

/** FACIAL_PART_IS_VALID
 *   @point: point with x y coordinate
 *
 *   Checks if point is  a non zero point.
**/
#define FACIAL_PART_IS_VALID(point) (point.x != 0 && point.y != 0)

/** FACIAL_PARTS_SET_POINT_TO_RECT_CENTER
 *   @rect: rectangle to check within
 *   @point: point with x y coordinate
 *
 *   Checks if point is in the rectangle.
**/
#define FACIAL_PARTS_SET_POINT_TO_RECT_CENTER(rect, point) \
  point.x = rect.x + rect.dx / 2; \
  point.y = rect.y + rect.dy / 2;

/** face_contour_point_eye_t:
 *
 *  Convert lookup from cam_types to faceproc internal interface
 **/
static const int convert_contour_eye[][2] = {
  {FACE_CONTOUR_POINT_EYE_L_PUPIL, CT_POINT_EYE_L_PUPIL},
  {FACE_CONTOUR_POINT_EYE_L_IN, CT_POINT_EYE_L_IN},
  {FACE_CONTOUR_POINT_EYE_L_OUT, CT_POINT_EYE_L_OUT},
  {FACE_CONTOUR_POINT_EYE_L_UP, CT_POINT_EYE_L_UP},
  {FACE_CONTOUR_POINT_EYE_L_DOWN, CT_POINT_EYE_L_DOWN},
  {FACE_CONTOUR_POINT_EYE_R_PUPIL, CT_POINT_EYE_R_PUPIL},
  {FACE_CONTOUR_POINT_EYE_R_IN, CT_POINT_EYE_R_IN},
  {FACE_CONTOUR_POINT_EYE_R_OUT, CT_POINT_EYE_R_OUT},
  {FACE_CONTOUR_POINT_EYE_R_UP, CT_POINT_EYE_R_UP},
  {FACE_CONTOUR_POINT_EYE_R_DOWN, CT_POINT_EYE_R_DOWN},
};

/** face_contour_point_forh_t:
 *
 *  Convert lookup from cam_types to faceproc internal interface
 **/
static const int convert_contour_forh[][2] = {
  {FACE_CONTOUR_POINT_FOREHEAD, CT_POINT_FOREHEAD},
};

/** face_contour_point_nose_t:
 *
 *  Convert lookup from cam_types to faceproc internal interface
 **/
static const int convert_contour_nose[][2] = {
  {FACE_CONTOUR_POINT_NOSE, CT_POINT_NOSE},
  {FACE_CONTOUR_POINT_NOSE_TIP, CT_POINT_NOSE_TIP},
  {FACE_CONTOUR_POINT_NOSE_L, CT_POINT_NOSE_L},
  {FACE_CONTOUR_POINT_NOSE_R, CT_POINT_NOSE_R},
  {FACE_CONTOUR_POINT_NOSE_L_0, CT_POINT_NOSE_L_0},
  {FACE_CONTOUR_POINT_NOSE_R_0, CT_POINT_NOSE_R_0},
  {FACE_CONTOUR_POINT_NOSE_L_1, CT_POINT_NOSE_L_1},
  {FACE_CONTOUR_POINT_NOSE_R_1, CT_POINT_NOSE_R_1},
};

/** face_contour_point_mouth_t:
 *
 *  Convert lookup from cam_types to faceproc internal interface
 **/
static const int convert_contour_mouth[][2] = {
  {FACE_CONTOUR_POINT_MOUTH_L, CT_POINT_MOUTH_L},
  {FACE_CONTOUR_POINT_MOUTH_R, CT_POINT_MOUTH_R},
  {FACE_CONTOUR_POINT_MOUTH_UP, CT_POINT_MOUTH_UP},
  {FACE_CONTOUR_POINT_MOUTH_DOWN, CT_POINT_MOUTH_DOWN},
};

/** face_contour_point_lip_t:
 *
 *  Convert lookup from cam_types to faceproc internal interface
 **/
static const int convert_contour_lip[][2] = {
  {FACE_CONTOUR_POINT_LIP_UP, CT_POINT_LIP_UP},
  {FACE_CONTOUR_POINT_LIP_DOWN, CT_POINT_LIP_DOWN},
};

/** face_contour_point_brow_t:
 *
 *  Convert lookup from cam_types to faceproc internal interface
 **/
static const int convert_contour_brow[][2] = {
  {FACE_CONTOUR_POINT_BROW_L_UP, CT_POINT_BROW_L_UP},
  {FACE_CONTOUR_POINT_BROW_L_DOWN, CT_POINT_BROW_L_DOWN},
  {FACE_CONTOUR_POINT_BROW_L_IN, CT_POINT_BROW_L_IN},
  {FACE_CONTOUR_POINT_BROW_L_OUT, CT_POINT_BROW_L_OUT},
  {FACE_CONTOUR_POINT_BROW_R_UP, CT_POINT_BROW_R_UP},
  {FACE_CONTOUR_POINT_BROW_R_DOWN, CT_POINT_BROW_R_DOWN},
  {FACE_CONTOUR_POINT_BROW_R_IN, CT_POINT_BROW_R_IN},
  {FACE_CONTOUR_POINT_BROW_R_OUT, CT_POINT_BROW_R_OUT},
};

/** face_contour_point_chin_t:
 *
 *  Convert lookup from cam_types to faceproc internal interface
 **/
static const int convert_contour_chin[][2] = {
  {FACE_CONTOUR_POINT_CHIN, CT_POINT_CHIN},
  {FACE_CONTOUR_POINT_CHIN_L, CT_POINT_CHIN_L},
  {FACE_CONTOUR_POINT_CHIN_R, CT_POINT_CHIN_R},
};

/** face_contour_point_ear_t:
 *
 *  Convert lookup from cam_types to faceproc internal interface
 **/
static const int convert_contour_ear[][2] = {
  {FACE_CONTOUR_POINT_EAR_L_DOWN, CT_POINT_EAR_L_DOWN},
  {FACE_CONTOUR_POINT_EAR_R_DOWN, CT_POINT_EAR_R_DOWN},
  {FACE_CONTOUR_POINT_EAR_L_UP, CT_POINT_EAR_L_UP},
  {FACE_CONTOUR_POINT_EAR_R_UP, CT_POINT_EAR_R_UP},
};

/**
 * Function: facial_angle_mask.
 *
 * Description: Make face angle mask for face detection
 *
 * Input parameters:
 *   face_angle_roll - face ROI angle.
 *   level - angle extention level (1: 1 angle, 2: 1 or 2 angles, 3: 3 angles)
 *
 * Return values:
 *    angle mask value
 *
 * Notes: none
 **/
static unsigned int facial_angle_mask(int face_angle_roll, int level)
{
  int angle_mask = 0;

  switch (level) {
  case 1: // pick one angle
    if ((face_angle_roll < 15) || (face_angle_roll >= 345)) {
      angle_mask = ANGLE_0;
    } else if (face_angle_roll < 45) {
      angle_mask = ANGLE_1;
    } else if (face_angle_roll < 75 ) {
      angle_mask = ANGLE_2;
    } else if (face_angle_roll < 105) {
      angle_mask = ANGLE_3;
    } else if (face_angle_roll < 135){
      angle_mask = ANGLE_4;
    } else if (face_angle_roll < 165) {
      angle_mask = ANGLE_5;
    } else if (face_angle_roll < 195) {
      angle_mask = ANGLE_6;
    } else if (face_angle_roll < 225) {
      angle_mask = ANGLE_7;
    } else if (face_angle_roll < 255) {
      angle_mask = ANGLE_8;
    } else if (face_angle_roll < 285) {
      angle_mask = ANGLE_9;
    } else if (face_angle_roll < 315) {
      angle_mask = ANGLE_10;
    } else if (face_angle_roll < 345) {
      angle_mask = ANGLE_11;
    }
    break;
  case 2: // pick one angle or two in case the angle is around a border
    if ((face_angle_roll < 10) || (face_angle_roll >= 350)) {
      angle_mask = ANGLE_0;
    } else if (face_angle_roll < 20) {
      angle_mask = ANGLE_0 | ANGLE_1;
    } else if (face_angle_roll < 40) {
      angle_mask = ANGLE_1;
    } else if (face_angle_roll < 50) {
      angle_mask = ANGLE_1 | ANGLE_2;
    } else if (face_angle_roll < 70){
      angle_mask = ANGLE_2;
    } else if (face_angle_roll < 80) {
      angle_mask = ANGLE_2 | ANGLE_3;
    } else if (face_angle_roll < 100) {
      angle_mask = ANGLE_3;
    } else if (face_angle_roll < 110) {
      angle_mask = ANGLE_3 | ANGLE_4;
    } else if (face_angle_roll < 130) {
      angle_mask = ANGLE_4;
    } else if (face_angle_roll < 140) {
      angle_mask = ANGLE_4 | ANGLE_5;
    } else if (face_angle_roll < 160) {
      angle_mask = ANGLE_5;
    } else if (face_angle_roll < 170) {
      angle_mask = ANGLE_5 | ANGLE_6;
    } else if (face_angle_roll < 190) {
      angle_mask = ANGLE_6;
    } else if (face_angle_roll < 200) {
      angle_mask = ANGLE_6 | ANGLE_7;
    } else if (face_angle_roll < 220) {
      angle_mask = ANGLE_7;
    } else if (face_angle_roll < 230) {
      angle_mask = ANGLE_7 | ANGLE_8;
    } else if (face_angle_roll < 250) {
      angle_mask = ANGLE_8;
    } else if (face_angle_roll < 260) {
      angle_mask = ANGLE_8 | ANGLE_9;
    } else if (face_angle_roll < 280) {
      angle_mask = ANGLE_9;
    } else if (face_angle_roll < 290) {
      angle_mask = ANGLE_9 | ANGLE_10;
    } else if (face_angle_roll < 310) {
      angle_mask = ANGLE_10;
    } else if (face_angle_roll < 320) {
      angle_mask = ANGLE_10 | ANGLE_11;
    } else if (face_angle_roll < 340) {
      angle_mask = ANGLE_11;
    } else if (face_angle_roll < 350) {
      angle_mask = ANGLE_11 | ANGLE_0;
    }
    break;
  case 3: // pick 3 angle
    if ((face_angle_roll < 15) || (face_angle_roll >= 345)) {
      angle_mask = ANGLE_11 | ANGLE_0 | ANGLE_1;
    } else if (face_angle_roll < 45) {
      angle_mask = ANGLE_0 | ANGLE_1 | ANGLE_2;
    } else if (face_angle_roll < 75 ) {
      angle_mask = ANGLE_1 | ANGLE_2 | ANGLE_3;
    } else if (face_angle_roll < 105) {
      angle_mask = ANGLE_2 | ANGLE_3 | ANGLE_4;
    } else if (face_angle_roll < 135){
      angle_mask = ANGLE_3 | ANGLE_4 | ANGLE_5;
    } else if (face_angle_roll < 165) {
      angle_mask = ANGLE_4 | ANGLE_5 | ANGLE_6;
    } else if (face_angle_roll < 195) {
      angle_mask = ANGLE_5 | ANGLE_6 | ANGLE_7;
    } else if (face_angle_roll < 225) {
      angle_mask = ANGLE_6 | ANGLE_7| ANGLE_8;
    } else if (face_angle_roll < 255) {
      angle_mask = ANGLE_7 | ANGLE_8 | ANGLE_9;
    } else if (face_angle_roll < 285) {
      angle_mask = ANGLE_8 | ANGLE_9 | ANGLE_10;
    } else if (face_angle_roll < 315) {
      angle_mask = ANGLE_9 | ANGLE_10 | ANGLE_11;
    } else if (face_angle_roll < 345) {
      angle_mask = ANGLE_10 | ANGLE_11 | ANGLE_0;
    }
    break;
  }

  return angle_mask;
}


/**
 * Function: facial_parts_wrap_create.
 *
 * Description: Facial parts wrapper create.
 *
 * Return values:
 *     pointer to handler on success.
 *
 * Notes: none
 **/
void *facial_parts_wrap_create()
{

  facial_parts_wrap_lib_t *p_lib;
  facial_parts_wrap_t *p_hnd;

  p_hnd = calloc(1, sizeof(*p_hnd));
  if (p_hnd == NULL) {
    IDBG_MED("%s:%d] Fail to allocate handler memory", __func__, __LINE__);
    return NULL;
  }
  p_lib = &p_hnd->p_lib;

  p_lib->ptr = dlopen("libmmcamera_faceproc.so", RTLD_NOW);
  if (!p_lib->ptr) {
    IDBG_ERROR("%s:%d] Error opening libmmcamera_faceproc.so lib",
      __func__, __LINE__);
    goto error;
  }

  p_lib->FACEPROC_PT_CreateHandle =
    dlsym(p_lib->ptr, "FACEPROC_PT_CreateHandle");
  if (p_lib->FACEPROC_PT_CreateHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_PT_CreateHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_PT_DeleteHandle =
    dlsym(p_lib->ptr, "FACEPROC_PT_DeleteHandle");
  if (p_lib->FACEPROC_PT_DeleteHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_PT_DeleteHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_PT_CreateResultHandle =
    dlsym(p_lib->ptr, "FACEPROC_PT_CreateResultHandle");
  if (p_lib->FACEPROC_PT_CreateResultHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_PT_CreateResultHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_PT_DeleteResultHandle =
    dlsym(p_lib->ptr, "FACEPROC_PT_DeleteResultHandle");
  if (p_lib->FACEPROC_PT_DeleteResultHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_PT_DeleteResultHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_PT_SetPosition =
    dlsym(p_lib->ptr, "FACEPROC_PT_SetPosition");
  if (p_lib->FACEPROC_PT_SetPosition == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_PT_SetPositionFrom error",
      __func__);
    goto error;
  }

  p_lib->FACEPROC_PT_SetPositionIP =
    dlsym(p_lib->ptr, "FACEPROC_PT_SetPositionIP");
  if (p_lib->FACEPROC_PT_SetPositionIP == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_PT_SetPositionFrom error",
      __func__);
    goto error;
  }

  p_lib->FACEPROC_PT_SetMode = dlsym(p_lib->ptr, "FACEPROC_PT_SetMode");
  if (p_lib->FACEPROC_PT_SetMode == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_PT_SetMode error", __func__);
    goto error;
  }

  p_lib->FACEPROC_PT_SetConfMode =
    dlsym(p_lib->ptr, "FACEPROC_PT_SetConfMode");
  if (p_lib->FACEPROC_PT_SetConfMode == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_PT_SetConfMode error", __func__);
    goto error;
  }

  p_lib->FACEPROC_PT_DetectPoint =
    dlsym(p_lib->ptr, "FACEPROC_PT_DetectPoint");
  if (p_lib->FACEPROC_PT_DetectPoint == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_PT_DetectPoint error", __func__);
    goto error;
  }

  p_lib->FACEPROC_PT_GetResult =
    dlsym(p_lib->ptr, "FACEPROC_PT_GetResult");
  if (p_lib->FACEPROC_PT_GetResult == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_PT_GetResult error", __func__);
    goto error;
  }

  p_lib->FACEPROC_PT_GetFaceDirection =
    dlsym(p_lib->ptr, "FACEPROC_PT_GetFaceDirection");
  if (p_lib->FACEPROC_PT_GetFaceDirection == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_PT_GetFaceDirection error", __func__);
    goto error;
  }

  p_lib->FACEPROC_CT_CreateHandle =
    dlsym(p_lib->ptr, "FACEPROC_CT_CreateHandle");
  if (p_lib->FACEPROC_CT_CreateHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_CT_CreateHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_CT_DeleteHandle =
    dlsym(p_lib->ptr, "FACEPROC_CT_DeleteHandle");
  if (p_lib->FACEPROC_CT_DeleteHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_CT_DeleteHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_CT_CreateResultHandle =
    dlsym(p_lib->ptr, "FACEPROC_CT_CreateResultHandle");
  if (p_lib->FACEPROC_CT_CreateResultHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_CT_CreateResultHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_CT_DeleteResultHandle =
    dlsym(p_lib->ptr, "FACEPROC_CT_DeleteResultHandle");
  if (p_lib->FACEPROC_CT_DeleteResultHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_CT_DeleteResultHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_CT_SetPointFromHandle =
    dlsym(p_lib->ptr, "FACEPROC_CT_SetPointFromHandle");
  if (p_lib->FACEPROC_CT_SetPointFromHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_CT_SetPointFromHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_CT_DetectContour =
    dlsym(p_lib->ptr, "FACEPROC_CT_DetectContour");
  if (p_lib->FACEPROC_CT_DetectContour == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_CT_DetectContour error", __func__);
    goto error;
  }

  p_lib->FACEPROC_CT_GetResult =
    dlsym(p_lib->ptr, "FACEPROC_CT_GetResult");
  if (p_lib->FACEPROC_CT_GetResult == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_CT_GetResult error", __func__);
    goto error;
  }

  p_lib->FACEPROC_CT_SetDetectionMode =
    dlsym(p_lib->ptr, "FACEPROC_CT_SetDetectionMode");
  if (p_lib->FACEPROC_CT_SetDetectionMode == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_CT_SetDetectionMode error", __func__);
    goto error;
  }

  p_lib->FACEPROC_SM_CreateHandle =
    dlsym(p_lib->ptr, "FACEPROC_SM_CreateHandle");
  if (p_lib->FACEPROC_SM_CreateHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SM_CreateHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_SM_DeleteHandle =
    dlsym(p_lib->ptr, "FACEPROC_SM_DeleteHandle");
  if (p_lib->FACEPROC_SM_DeleteHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SM_DeleteHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_SM_CreateResultHandle =
    dlsym(p_lib->ptr, "FACEPROC_SM_CreateResultHandle");
  if (p_lib->FACEPROC_SM_CreateResultHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SM_CreateResultHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_SM_DeleteResultHandle =
    dlsym(p_lib->ptr, "FACEPROC_SM_DeleteResultHandle");
  if (p_lib->FACEPROC_SM_DeleteResultHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SM_DeleteResultHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_SM_SetPointFromHandle =
      dlsym(p_lib->ptr, "FACEPROC_SM_SetPointFromHandle");
  if (p_lib->FACEPROC_SM_SetPointFromHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SM_SetPointFromHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_SM_Estimate =
    dlsym(p_lib->ptr, "FACEPROC_SM_Estimate");
  if (p_lib->FACEPROC_SM_Estimate == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SM_Estimate error", __func__);
    goto error;
  }

  p_lib->FACEPROC_SM_GetResult =
    dlsym(p_lib->ptr, "FACEPROC_SM_GetResult");
  if (p_lib->FACEPROC_SM_GetResult == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SM_GetResult error", __func__);
    goto error;
  }

  p_lib->FACEPROC_GB_CreateHandle =
    dlsym(p_lib->ptr, "FACEPROC_GB_CreateHandle");
  if (p_lib->FACEPROC_GB_CreateHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_GB_CreateHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_GB_DeleteHandle =
    dlsym(p_lib->ptr, "FACEPROC_GB_DeleteHandle");
  if (p_lib->FACEPROC_GB_DeleteHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_GB_DeleteHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_GB_CreateResultHandle =
    dlsym(p_lib->ptr, "FACEPROC_GB_CreateResultHandle");
  if (p_lib->FACEPROC_GB_CreateResultHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_GB_CreateResultHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_GB_DeleteResultHandle =
    dlsym(p_lib->ptr, "FACEPROC_GB_DeleteResultHandle");
  if (p_lib->FACEPROC_GB_DeleteResultHandle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_GB_DeleteResultHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_GB_SetPointFromHandle =
      dlsym(p_lib->ptr, "FACEPROC_GB_SetPointFromHandle");
  if (p_lib->FACEPROC_GB_SetPointFromHandle== NULL) {
    IDBG_ERROR("%s Loading FACEPROC_GB_SetPointFromHandle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_GB_Estimate =
    dlsym(p_lib->ptr, "FACEPROC_GB_Estimate");
  if (p_lib->FACEPROC_GB_Estimate == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_GB_Estimate error", __func__);
    goto error;
  }

  p_lib->FACEPROC_GB_GetEyeCloseRatio =
    dlsym(p_lib->ptr, "FACEPROC_GB_GetEyeCloseRatio");
  if (p_lib->FACEPROC_GB_GetEyeCloseRatio == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_GB_GetEyeCloseRatio error", __func__);
    goto error;
  }

  p_lib->FACEPROC_GB_GetGazeDirection =
    dlsym(p_lib->ptr, "FACEPROC_GB_GetGazeDirection");
  if (p_lib->FACEPROC_GB_GetGazeDirection == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_GB_GetGazeDirection error", __func__);
    goto error;
  }

  p_lib->FACEPROC_CreateDetection =
    dlsym(p_lib->ptr, "FACEPROC_CreateDetection");
  if (p_lib->FACEPROC_CreateDetection == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_CreateDetection error", __func__);
    goto error;
  }

  p_lib->FACEPROC_DeleteDetection =
    dlsym(p_lib->ptr, "FACEPROC_DeleteDetection");
  if (p_lib->FACEPROC_DeleteDetection == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_DeleteDetection error", __func__);
    goto error;
  }

  p_lib->FACEPROC_SetDtMode =
    dlsym(p_lib->ptr, "FACEPROC_SetDtMode");
  if (p_lib->FACEPROC_SetDtMode == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SetDtMode error", __func__);
    goto error;
  }

  p_lib->FACEPROC_SetDtStep =
    dlsym(p_lib->ptr, "FACEPROC_SetDtStep");
  if (p_lib->FACEPROC_SetDtStep == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SetDtStep error", __func__);
    goto error;
  }

  p_lib->FACEPROC_SetDtAngle =
    dlsym(p_lib->ptr, "FACEPROC_SetDtAngle");
  if (p_lib->FACEPROC_SetDtAngle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SetDtAngle error", __func__);
    goto error;
  }

  p_lib->FACEPROC_SetDtDirectionMask =
    dlsym(p_lib->ptr, "FACEPROC_SetDtDirectionMask");
  if (p_lib->FACEPROC_SetDtDirectionMask == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SetDtDirectionMask error", __func__);
    goto error;
  }

  p_lib->FACEPROC_SetDtFaceSizeRange =
    dlsym(p_lib->ptr, "FACEPROC_SetDtFaceSizeRange");
  if (p_lib->FACEPROC_SetDtFaceSizeRange == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SetDtFaceSizeRange error", __func__);
    goto error;
  }

  p_lib->FACEPROC_SetDtThreshold =
    dlsym(p_lib->ptr, "FACEPROC_SetDtThreshold");
  if (p_lib->FACEPROC_SetDtThreshold == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SetDtThreshold error", __func__);
    goto error;
  }

  p_lib->FACEPROC_CreateDtResult =
    dlsym(p_lib->ptr, "FACEPROC_CreateDtResult");
  if (p_lib->FACEPROC_CreateDtResult == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_CreateDtResult error", __func__);
    goto error;
  }

  p_lib->FACEPROC_DeleteDtResult =
    dlsym(p_lib->ptr, "FACEPROC_DeleteDtResult");
  if (p_lib->FACEPROC_DeleteDtResult == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_DeleteDtResult error", __func__);
    goto error;
  }

  p_lib->FACEPROC_Detection =
    dlsym(p_lib->ptr, "FACEPROC_Detection");
  if (p_lib->FACEPROC_Detection == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_Detection error", __func__);
    goto error;
  }

  p_lib->FACEPROC_GetDtFaceInfo =
    dlsym(p_lib->ptr, "FACEPROC_GetDtFaceInfo");
  if (p_lib->FACEPROC_GetDtFaceInfo == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_GetDtFaceInfo error", __func__);
    goto error;
  }

  p_lib->FACEPROC_GetDtFaceCount =
    dlsym(p_lib->ptr, "FACEPROC_GetDtFaceCount");
  if (p_lib->FACEPROC_GetDtFaceCount == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_GetDtFaceCount error", __func__);
    goto error;
  }

  p_lib->FACEPROC_SetDtRectangleMask =
    dlsym(p_lib->ptr, "FACEPROC_SetDtRectangleMask");
  if (p_lib->FACEPROC_SetDtRectangleMask == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SetDtRectangleMask error", __func__);
    goto error;
  }

  p_lib->FACEPROC_GetDtRectangleMask =
    dlsym(p_lib->ptr, "FACEPROC_SetDtRectangleMask");
  if (p_lib->FACEPROC_GetDtRectangleMask == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_GetDtRectangleMask error", __func__);
    goto error;
  }

  p_lib->FACEPROC_ClearDtResult =
    dlsym(p_lib->ptr, "FACEPROC_ClearDtResult");
  if (p_lib->FACEPROC_ClearDtResult == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_ClearDtResult error", __func__);
    goto error;
  }

  p_lib->FACEPROC_SetDtRefreshCount =
    dlsym(p_lib->ptr, "FACEPROC_SetDtRefreshCount");
  if (p_lib->FACEPROC_SetDtRefreshCount == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SetDtRefreshCount error", __func__);
    goto error;
  }

  p_hnd->hpt = p_lib->FACEPROC_PT_CreateHandle();
  if (p_hnd->hpt == NULL) {
    IDBG_ERROR("%s FACEPROC_PT_CreateHandle failed",  __func__);
    goto error;
  }

  p_hnd->hptresult = p_lib->FACEPROC_PT_CreateResultHandle();
  if (p_hnd->hptresult== NULL) {
    IDBG_ERROR("%s eng_FACEPROC_PT_CreateResultHandle failed",  __func__);
    goto error_face_part;
  }

  p_hnd->hct = p_lib->FACEPROC_CT_CreateHandle();
  if (p_hnd->hct == NULL) {
    IDBG_ERROR("%s eng_FACEPROC_CT_CreateHandle failed",  __func__);
    goto error_countour;
  }

  p_hnd->hctresult = p_lib->FACEPROC_CT_CreateResultHandle();
  if (p_hnd->hctresult == NULL) {
    IDBG_ERROR("%s eng_FACEPROC_CT_CreateResultHandle failed",  __func__);
    goto error_countour;
  }

  p_hnd->hsm = p_lib->FACEPROC_SM_CreateHandle();
  if (!(p_hnd->hsm)) {
    IDBG_ERROR("%s eng_FACEPROC_SM_CreateHandle failed",  __func__);
    goto error_smile;
  }

  p_hnd->hsmresult = p_lib->FACEPROC_SM_CreateResultHandle();
  if (!(p_hnd->hsmresult)) {
    IDBG_ERROR("%s eng_FACEPROC_SM_CreateResultHandle failed",  __func__);
    goto error_smile;
  }

  p_hnd->hgb = p_lib->FACEPROC_GB_CreateHandle();
  if (!(p_hnd->hgb)) {
    IDBG_ERROR("%s eng_FACEPROC_GB_CreateHandle failed",  __func__);
    goto error_gaze_blink;
  }

  p_hnd->hgbresult = p_lib->FACEPROC_GB_CreateResultHandle();
  if (!(p_hnd->hgbresult)) {
    IDBG_ERROR("%s eng_FACEPROC_GB_CreateResultHandle failed",  __func__);
    goto error_gaze_blink;
  }

  p_hnd->hdt = p_lib->FACEPROC_CreateDetection();
  if (!p_hnd->hdt) {
    IDBG_ERROR("%s FACEPROC_CreateDetection failed",  __func__);
    goto error_face_detect;
  }

  p_hnd->hdtresult = p_lib->FACEPROC_CreateDtResult(1, 1);
  if (!p_hnd->hdtresult) {
    IDBG_ERROR("%s FACEPROC_CreateDtResult failed",  __func__);
    goto error_face_detect;
  }

  return p_hnd;

error_face_detect:
  if (p_hnd->hdt) {
    p_lib->FACEPROC_DeleteDetection(p_hnd->hdt);
  }

  if (p_hnd->hdtresult) {
    p_lib->FACEPROC_DeleteDtResult(p_hnd->hdtresult);
  }

error_gaze_blink:
  if (p_hnd->hgb) {
    p_lib->FACEPROC_GB_DeleteHandle(p_hnd->hgb);
    if (p_hnd->hgbresult) {
      p_lib->FACEPROC_GB_DeleteResultHandle(p_hnd->hgbresult);
    }
  }

error_smile:
  if (p_hnd->hsm) {
    p_lib->FACEPROC_SM_DeleteHandle(p_hnd->hsm);
    if (p_hnd->hsmresult) {
      p_lib->FACEPROC_SM_DeleteResultHandle(p_hnd->hsmresult);
    }
  }

error_countour:
  if (p_hnd->hct) {
    p_lib->FACEPROC_CT_DeleteHandle(p_hnd->hct);
    if (p_hnd->hctresult) {
      p_lib->FACEPROC_CT_DeleteResultHandle(p_hnd->hctresult);
    }
  }

error_face_part:
  if (p_hnd->hpt) {
    p_lib->FACEPROC_PT_DeleteHandle(p_hnd->hpt);
    if (p_hnd->hptresult) {
      p_lib->FACEPROC_PT_DeleteResultHandle(p_hnd->hptresult);
    }
  }

error:
  free(p_hnd);
  return NULL;
}

/**
 * Function: facial_parts_wrap_config.
 *
 * Description: Facial part wrapper configuration
 *
 * Input parameters:
 *   handle - Pointer to wrapper handler.
 *   p_conf - Pointer to facial parts configuration.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int facial_parts_wrap_config(void *handle, facial_parts_wrap_config_t *p_conf)
{
  facial_parts_wrap_t *p_hnd;
  facial_parts_wrap_lib_t *p_lib;
  int rc;

  if (!handle || !p_conf) {
    IDBG_ERROR("%s: Invalid input %p %p", __func__, handle, p_conf);
    return IMG_ERR_INVALID_INPUT;
  }
  p_hnd = (facial_parts_wrap_t *)handle;

  if (!p_hnd->hpt) {
    IDBG_ERROR("%s: Handler not ready %p", __func__, p_hnd->hpt);
    return IMG_ERR_INVALID_OPERATION;
  }
  p_lib = &p_hnd->p_lib;

  rc = p_lib->FACEPROC_PT_SetConfMode(p_hnd->hpt, PT_CONF_USE);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s eng_FACEPROC_PT_SetConfMode failed",  __func__);
    return IMG_ERR_GENERAL;
  }

  rc = p_lib->FACEPROC_SetDtMode(p_hnd->hdt, DETECTION_MODE_DEFAULT);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtMode failed",  __func__);
    return IMG_ERR_GENERAL;
  }

  /* Set refresh count */
  rc = p_lib->FACEPROC_SetDtRefreshCount(p_hnd->hdt, DETECTION_MODE_MOTION1, 1);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s:%d] FACEPROC_SetDtRefreshCount failed %d",  __func__, __LINE__, rc);
    return IMG_ERR_GENERAL;
  }

  if (p_conf->enable_contour) {
    rc = p_lib->FACEPROC_CT_SetDetectionMode(p_hnd->hct,
      p_conf->contour_detection_mode);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s eng_FACEPROC_CT_SetDetectionMode failed",  __func__);
      return IMG_ERR_GENERAL;
    }
  }
  p_hnd->config = *p_conf;

  return IMG_SUCCESS;
}

/**
 * Function: facial_parts_wrap_discard_face_parts_based.
 *
 * Description: Discard face based on facial parts false filtering.
 *
 * Input parameters:
 *   p_conf - Pointer to facial parts configuration.
 *   p_output - Pointer to output info.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int facial_parts_wrap_discard_face_parts_based(facial_parts_wrap_config_t *p_conf,
  faceproc_info_t *p_output)
{
  uint32_t mouth_conf;
  uint32_t eye_conf;
  int32_t face_conf;
  uint32_t nose_conf;
  float ratio;

  if (!p_conf->discard_threshold ||  !p_conf->min_threshold) {
    IDBG_MED("%s: Dont discard the face thresh %d id %d %d", __func__,
      p_conf->discard_threshold, p_output->unique_id, p_output->fd_confidence);
    return 0;
  }

  if ((uint32_t)p_output->fd_confidence >= p_conf->discard_face_below) {
    IDBG_MED("%s: Do not discard confidence is fine %d  below %d", __func__,
      p_output->fd_confidence, p_conf->discard_face_below);
    return 0;
  }

  if (p_output->fd_confidence < p_conf->min_threshold) {
    IDBG_ERROR("%s:%d] Invalid confidence %d min %d", __func__,
      __LINE__, p_output->fd_confidence, p_conf->min_threshold);
    return 0;
  }

  ratio = 1000.0 / (float)(p_conf->discard_face_below - p_conf->min_threshold);
  face_conf = ratio * (float)(p_output->fd_confidence - p_conf->min_threshold);

  /* Get nose with confidence level based on filter type */
  if (p_conf->nose_use_max_filter) {
    nose_conf = IMG_MAX(p_output->fp.confidence[FACE_PART_NOSE_LEFT],
      p_output->fp.confidence[FACE_PART_NOSE_RIGHT]);
  } else {
    nose_conf = IMG_AVG(p_output->fp.confidence[FACE_PART_NOSE_LEFT],
      p_output->fp.confidence[FACE_PART_NOSE_RIGHT]);
  }

  /* Get eye with confidence level based on filter type */
  if (p_conf->eyes_use_max_filter) {
    eye_conf = IMG_MAX(p_output->fp.confidence[FACE_PART_LEFT_EYE],
      p_output->fp.confidence[FACE_PART_RIGHT_EYE]);
  } else {
    eye_conf = IMG_AVG(p_output->fp.confidence[FACE_PART_LEFT_EYE],
      p_output->fp.confidence[FACE_PART_RIGHT_EYE]);
  }

  eye_conf = ceil((float)eye_conf * p_conf->weight_eyes);

  mouth_conf = p_output->fp.confidence[FACE_PART_MOUTH];
  mouth_conf = ceil((float)mouth_conf * p_conf->weight_mouth);

  nose_conf = ceil((float)nose_conf * p_conf->weight_nose);

  face_conf = ceil((float)face_conf * p_conf->weight_face);

  if ((mouth_conf + eye_conf + nose_conf + face_conf) <
    p_conf->discard_threshold) {
    IDBG_ERROR("%s: [FD_FALSE_POS_DBG] Discard face eye conf %d mouth_conf %d"
      "nose_conf %d face_conf %d threshold: %d "
      "Default eye conf (%d %d) mouth %d nose(%d %d) face %d", __func__,
      eye_conf, mouth_conf, nose_conf, face_conf, p_conf->discard_threshold,
      p_output->fp.confidence[FACE_PART_LEFT_EYE],
      p_output->fp.confidence[FACE_PART_RIGHT_EYE],
      p_output->fp.confidence[FACE_PART_MOUTH],
      p_output->fp.confidence[FACE_PART_NOSE_LEFT],
      p_output->fp.confidence[FACE_PART_NOSE_RIGHT],
      face_conf);
    return 1;
  }

  IDBG_INFO("%s: [FD_FALSE_POS_DBG] Allow face eye conf %d mouth_conf %d"
    "nose_conf %d face_conf %d threshold: %d "
    "Default eye conf (%d %d) mouth %d nose(%d %d) face %d", __func__,
    eye_conf, mouth_conf, nose_conf, face_conf, p_conf->discard_threshold,
    p_output->fp.confidence[FACE_PART_LEFT_EYE],
    p_output->fp.confidence[FACE_PART_RIGHT_EYE],
    p_output->fp.confidence[FACE_PART_MOUTH],
    p_output->fp.confidence[FACE_PART_NOSE_LEFT],
    p_output->fp.confidence[FACE_PART_NOSE_RIGHT],
    face_conf);

  return 0;
}

/**
 * Function: facial_parts_wrap_discard_face_sw_scan.
 *
 * Description: Discard face based on sw face detection scan.
 *
 * Input parameters:
 *   handle - Pointer to facial wrapper handle.
 *   p_frame - Pointer to img frame.
 *   p_output - Pointer to face detection result.
 *
 * Return values:
 *     IMG_SUCCESS - on Success.
 *     IMG_ERR_NOT_FOUND - When face need to be filtered is not found.
 *     IMG_ERR_GENERAL - General error
 **/
int facial_parts_wrap_discard_face_sw_scan(void *handle,
  img_frame_t *p_frame, faceproc_info_t *p_output, int face_tracked,
  faceproc_info_t *p_old)
{
  facial_parts_wrap_t *p_hnd;
  facial_parts_wrap_lib_t *p_lib;
  uint32_t an_still_angle[POSE_TYPE_COUNT];
  RECT rect_mask;
  FACEINFO face_info;
  uint32_t angle_mask;
  int32_t num_faces;
  int additional_pixels;
  int min_face_size;
  int box_out_of_frame;
  int rc;
  uint8_t prof_angle;
  uint32_t sw_thres;
  uint32_t min_min_size;
  uint32_t density = 0;

  p_hnd = (facial_parts_wrap_t *)handle;
  p_lib = &p_hnd->p_lib;

  if (!p_hnd->config.sw_face_threshold) {
    return IMG_SUCCESS;
  }

  if ((uint32_t)p_output->fd_confidence >= p_hnd->config.discard_face_below) {
    IDBG_MED("%s:%d] [FD_FALSE_POS_DBG] Do not discard confidence is fine %d"
      "  below %d", __func__, __LINE__,
      p_output->fd_confidence, p_hnd->config.discard_face_below);
    return 0;
  }

  box_out_of_frame = 0;
  if (p_output->face_boundary.x + p_output->face_boundary.dx >
    p_frame->frame[0].plane[0].width - p_hnd->config.sw_face_discard_border) {
    box_out_of_frame = 1;
  }

  if (p_output->face_boundary.y + p_output->face_boundary.dy >
    p_frame->frame[0].plane[0].height - p_hnd->config.sw_face_discard_border) {
    box_out_of_frame = 1;
  }

  if (box_out_of_frame) {
    if (p_hnd->config.sw_face_discard_out) {
      IDBG_HIGH("%s:%d][FD_FALSE_POS_DBG] Discard out of border",
        __func__, __LINE__);
      return IMG_ERR_NOT_FOUND;
    } else {
      IDBG_HIGH("%s:%d][FD_FALSE_POS_DBG] Allow out of border",
        __func__, __LINE__);
      return IMG_SUCCESS;
    }
  }

  density = faceproc_common_validate_and_get_density(
    false, p_hnd->config.sw_face_search_dens, face_tracked);

  rc = p_lib->FACEPROC_SetDtStep(p_hnd->hdt, density, density);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtStep failed",  __func__);
    return IMG_ERR_GENERAL;
  }

  sw_thres = p_hnd->config.sw_face_threshold;

  rc = p_lib->FACEPROC_SetDtThreshold(p_hnd->hdt, sw_thres, sw_thres);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtThreshold failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }

  prof_angle = (uint8_t)abs(p_output->gaze_angle);

  memset(an_still_angle, 0x00, sizeof(an_still_angle));
  angle_mask = facial_angle_mask(p_output->face_angle_roll, 2);
  an_still_angle[POSE_FRONT] = angle_mask;
  an_still_angle[POSE_HALF_PROFILE] = angle_mask;
  an_still_angle[POSE_PROFILE] = ANGLE_NONE;

  if (p_old != NULL) { // in case it's not first latch
    if (p_old->sw_fd_result) {
      IDBG_MED("%s:%d] [FD_DBG_SW_OR] HWConf=%03d, HWRoll=%03d, HWPose=%03d, "
        "HWCord=%03dx%03d, HWSize=%03d", __func__, __LINE__,
        p_old->fd_confidence, p_old->face_angle_roll, p_old->gaze_angle,
        p_old->face_boundary.x, p_old->face_boundary.y,
        p_old->face_boundary.dx);

      // same pose : 3 angle,  next pose: 1 or 2 angle
      if (abs(p_old->gaze_angle)==0) {
        an_still_angle[POSE_FRONT] = facial_angle_mask(p_old->face_angle_roll, 3);
        an_still_angle[POSE_HALF_PROFILE] = facial_angle_mask(p_old->face_angle_roll, 2);
      } else if (abs(p_old->gaze_angle)==45) {
        an_still_angle[POSE_FRONT] = facial_angle_mask(p_old->face_angle_roll, 2);
        an_still_angle[POSE_HALF_PROFILE] = facial_angle_mask(p_old->face_angle_roll, 3);
      } else if (abs(p_old->gaze_angle)==90) {
        // never happen -- error
        IDBG_ERROR("%s:%d] [FD_DBG_SW_OLD] Angle error=%03d", __func__, __LINE__, p_old->gaze_angle);
      } else {
        // error
        IDBG_ERROR("%s:%d] [FD_DBG_SW_OLD] Angle error=%03d", __func__, __LINE__, p_old->gaze_angle);
      }
    }
  }

  /* Set Detection Angles */
  rc = (int) p_lib->FACEPROC_SetDtAngle(p_hnd->hdt,
    an_still_angle, ANGLE_ROTATION_EXTALL | ANGLE_POSE_EXT0);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtAngle failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }

  additional_pixels = (p_output->face_boundary.dx *
    p_hnd->config.sw_face_box_border_per) / 100;

  rect_mask.top =  p_output->face_boundary.y;
  rect_mask.left = p_output->face_boundary.x;
  rect_mask.right =  p_output->face_boundary.dx + p_output->face_boundary.x;
  rect_mask.bottom =  p_output->face_boundary.dy + p_output->face_boundary.y;

  rect_mask.top -= additional_pixels;
  if (rect_mask.top < 0) {
    rect_mask.top = 0;
  }

  rect_mask.left -= additional_pixels;
  if (rect_mask.left < 0) {
    rect_mask.left = 0;
  }

  rect_mask.right += additional_pixels;
  /* Rectangle mask should be smaller the image width */
  if (rect_mask.right > (int)FP_WIDTH(p_frame) - 1) {
    rect_mask.right = FP_WIDTH(p_frame) - 1;
  }

  rect_mask.bottom += additional_pixels;
  /* Rectangle mask should be smaller the image height */
  if (rect_mask.bottom > (int)FP_HEIGHT(p_frame) - 1) {
    rect_mask.bottom = FP_HEIGHT(p_frame) - 1;
  }

  min_face_size = p_output->face_boundary.dx + (additional_pixels*2) ;
  if (min_face_size > rect_mask.bottom - rect_mask.top) {
    min_face_size = rect_mask.bottom - rect_mask.top;
  }
  if (min_face_size > rect_mask.right - rect_mask.left) {
    min_face_size = rect_mask.right - rect_mask.left;
  }

  min_min_size = MAX((p_output->face_boundary.dx *
    (100 - p_hnd->config.sw_face_box_border_per) / 100), 20);

  /* Set the max and min face size for detection */
  rc = p_lib->FACEPROC_SetDtFaceSizeRange(p_hnd->hdt,
    min_min_size , min_face_size);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtFaceSizeRange failed %d %d %d",
      __func__, rc, min_face_size,
      min_min_size);
    return IMG_ERR_GENERAL;
  }

  rc = p_lib->FACEPROC_SetDtRectangleMask(p_hnd->hdt, rect_mask, rect_mask);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtRectangleMask failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }


  p_lib->FACEPROC_ClearDtResult(p_hnd->hdtresult);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_ClearDtResult returned error: %d",
      __func__, rc);
    return IMG_ERR_GENERAL;
  }

  rc = p_lib->FACEPROC_Detection(p_hnd->hdt, FP_ADDR(p_frame),
    FP_WIDTH(p_frame), FP_HEIGHT(p_frame), ACCURACY_HIGH_TR,
    p_hnd->hdtresult);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_Detection returned error: %d",
      __func__, rc);
    return IMG_ERR_GENERAL;
  }

  num_faces = 0;
  rc = p_lib->FACEPROC_GetDtFaceCount(p_hnd->hdtresult, &num_faces);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s:%d]FACEPROC_GetDtFaceCount returned error: %d",
      __func__, __LINE__, rc);
    return IMG_ERR_GENERAL;
  }

  if (num_faces == 1) {
    memset(&face_info, 0x00, sizeof(face_info));
    rc = p_lib->FACEPROC_GetDtFaceInfo(p_hnd->hdtresult, 0, &face_info);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s:%d]FACEPROC_GetDtFaceInfo returned error: %d",
        __func__, __LINE__, rc);
      return IMG_ERR_GENERAL;
    }

    IDBG_HIGH("%s:%d][FD_FALSE_POS_DBG] Allow face %s confidence %d roll %d pose %d %d ID %d"
      " min face size (%d %d), ft %d th %d",
      __func__, __LINE__, num_faces ? "yes" : "no", face_info.nConfidence,
      p_output->face_angle_roll, prof_angle, face_info.nPose, face_info.nID,
      min_min_size, min_face_size, face_tracked, sw_thres);

    IDBG_MED("[FD_DBG_SW_HW] HWConf=%03d, HWRoll=%03d, HWPose=%03d, "
      "HWCord=%03dx%03d, HWSize=%03d",
      p_output->fd_confidence, p_output->face_angle_roll, p_output->gaze_angle,
      p_output->face_boundary.x, p_output->face_boundary.y,
      p_output->face_boundary.dx);

    int roll, x, y, size;
    int xd = face_info.ptRightTop.x - face_info.ptLeftTop.x;
    int yd = face_info.ptRightTop.y - face_info.ptLeftTop.y;

    roll = ((int)(atan2((double)yd, (double)xd) * 180 / 3.14159f + 360))%360;
    size = sqrt((xd * xd) + (yd * yd));
    x = (face_info.ptRightBottom.x + face_info.ptLeftTop.x - size) / 2;
    y = (face_info.ptRightBottom.y + face_info.ptLeftTop.y - size) / 2;

    p_output->face_boundary.x = x;
    p_output->face_boundary.y = y;
    p_output->face_boundary.dx = size;
    p_output->face_boundary.dy = size;
    p_output->face_angle_roll = roll;
    p_output->gaze_angle = face_info.nPose;
    p_output->fd_confidence = face_info.nConfidence;

    IDBG_MED("[FD_DBG_SW_HW] SWConf=%03d, SWRoll=%03d, SWPose=%03d, "
      "SWCord=%03dx%03d, SWSize=%03d",
      face_info.nConfidence, roll, face_info.nPose, x, y, size);
  } else if (num_faces > 1) {
    IDBG_MED("[FD_DBG_SW_HW] Too many faces : %d", num_faces);
  } else {
    IDBG_MED("[FD_DBG_SW_HW] No faces");
  }

  return (num_faces == 1) ? IMG_SUCCESS : IMG_ERR_NOT_FOUND;
}

/**
 * Function: facial_parts_validate_and_fix_PT_coordinates.
 *
 * Description: Correct the corrdinates of Facial parts if they
 * are out of bounds of face rectangle.
 *
 * Input parameters:
 *   p_output - pointer to faceproc face result
 *
 * Return values:
 *     IMG_SUCCESS - on Success.
 *
 * Notes: none
 **/
int facial_parts_validate_and_fix_PT_coordinates(faceproc_info_t *p_output)
{
  if(FACIAL_PART_IS_VALID(p_output->fp.face_pt[PT_POINT_LEFT_EYE]) &&
    !FACIAL_PARTS_IS_WITHIN_RECT_BOUNDARY(p_output->face_boundary,
    p_output->fp.face_pt[PT_POINT_LEFT_EYE])) {
    FACIAL_PARTS_SET_POINT_TO_RECT_CENTER(p_output->face_boundary,
      p_output->fp.face_pt[PT_POINT_LEFT_EYE]);
    IDBG_MED("%s,%d]: corrected left eye coordinates", __func__, __LINE__);
  }
  if(FACIAL_PART_IS_VALID(p_output->fp.face_pt[PT_POINT_RIGHT_EYE]) &&
    !FACIAL_PARTS_IS_WITHIN_RECT_BOUNDARY(p_output->face_boundary,
    p_output->fp.face_pt[PT_POINT_RIGHT_EYE])) {
    FACIAL_PARTS_SET_POINT_TO_RECT_CENTER(p_output->face_boundary,
      p_output->fp.face_pt[PT_POINT_RIGHT_EYE]);
    IDBG_MED("%s,%d]: corrected right eye coordinates", __func__, __LINE__);
  }
  if(FACIAL_PART_IS_VALID(p_output->fp.face_pt[PT_POINT_MOUTH]) &&
    !FACIAL_PARTS_IS_WITHIN_RECT_BOUNDARY(p_output->face_boundary,
    p_output->fp.face_pt[PT_POINT_MOUTH])) {
    FACIAL_PARTS_SET_POINT_TO_RECT_CENTER(p_output->face_boundary,
      p_output->fp.face_pt[PT_POINT_MOUTH]);
    IDBG_MED("%s,%d]: corrected mouth coordinates", __func__, __LINE__);
  }
  return IMG_SUCCESS;
}




/**
 * Function: facial_parts_wrap_process_result.
 *
 * Description: Facial part wrapper process. Fill facial part result.
 *   information based on detected faces.
 *
 * Input parameters:
 *   handle - Pointer to wrapper handler.
 *   p_frame - Pointer to frame for scanning.
 *   p_output - Faceproc result data.
 *   verify_face - Flag indicating that we need to verify the face.
 *   face_tracked - Whether the current face is tracked.
 *   p_old - previous result
 *   run_facial_parts - Whether to run facial parts
 *
 * Return values:
 *     IMG_SUCCESS - on Success.
 *     IMG_ERR_NOT_FOUND - Face verification failed.
 *     IMG_ERR_GENERAL - General error
 *
 * Notes: none
 **/
int facial_parts_wrap_process_result(void *handle, img_frame_t *p_frame,
  faceproc_info_t *p_output, int verify_face, int face_tracked,
  faceproc_info_t *p_old, bool run_facial_parts)
{
  facial_parts_wrap_t *p_hnd;
  facial_parts_wrap_lib_t *p_lib;
  int center_x, center_y, size, angle;
  int rc, leye_close_ratio, reye_close_ratio;
  int pn_gaze_left_right, pn_gaze_up_down, pose;
  fd_pixel_t face_pt[MAX_FACE_PART];
  unsigned int i;


  if (!handle || !p_output || !p_frame ) {
    IDBG_ERROR("%s:Invalid input %p %p %p", __func__,
      handle, p_frame, p_output);
    return IMG_ERR_INVALID_INPUT;
  }
  p_hnd = (facial_parts_wrap_t *)handle;
  p_lib = &p_hnd->p_lib;

  IDBG_LOW("%s:%d] [OFFLINE_FD] myface x %d , y %d, dx %d, dy %d verify_face %d %d %d",
    __func__, __LINE__,
    p_output->face_boundary.x,
    p_output->face_boundary.y,
    p_output->face_boundary.dx,
    p_output->face_boundary.dy,
    verify_face,
    p_hnd->config.enable_sw_false_pos_filtering,
    p_hnd->config.enable_fp_false_pos_filtering);

  /* Disable face verification if false positive filtering is disabled */
  if (!p_hnd->config.enable_sw_false_pos_filtering &&
    !p_hnd->config.enable_fp_false_pos_filtering) {
    verify_face = 0;
  }

  /* Check if if we need to discard this face based on sw scan */

  if (verify_face && p_hnd->config.enable_sw_false_pos_filtering) {
    rc = facial_parts_wrap_discard_face_sw_scan(handle, p_frame, p_output, face_tracked, p_old);
    if (rc == IMG_ERR_NOT_FOUND) {
      IDBG_MED("%s: Skip Face detection unique id %d", __func__,
        p_output->unique_id);
      return IMG_ERR_NOT_FOUND;
    }
    verify_face = 0; /* skip fp verification */
  }

  if (!run_facial_parts) {
    // Do not run facial parts even if enabled.
    // Caller will call again to execute facial parts
    return IMG_SUCCESS;
  }

  /* Check if we need facial part scanning for this frame face */
  if (!verify_face && !p_hnd->config.enable_blink &&
    !p_hnd->config.enable_gaze && !p_hnd->config.enable_smile &&
    !p_hnd->config.enable_fp_false_pos_filtering) {
    IDBG_MED("%s: Skip facial part detection", __func__);
    return IMG_SUCCESS;
  }

  memset(face_pt, 0x00, sizeof(face_pt));

  /* initialize output fields */
  p_output->blink_detected = 0;
  p_output->left_blink = 0;
  p_output->right_blink = 0;
  p_output->left_right_gaze = 0;
  p_output->top_bottom_gaze = 0;
  p_output->sm.smile_degree = 0;
  p_output->sm.confidence = 0;

  pose = p_output->gaze_angle;
  angle = p_output->face_angle_roll;
  center_x = p_output->face_boundary.x + p_output->face_boundary.dx / 2;
  center_y = p_output->face_boundary.y + p_output->face_boundary.dy / 2;
  size = p_output->face_boundary.dx;
  IDBG_LOW("pose %d, angle %d, center_x %d, center_y %d, size %d",
    pose,
    angle,
    center_x,
    center_y,
    size);

  /* Face part detection */
  rc = p_lib->FACEPROC_PT_SetPositionIP(p_hnd->hpt, center_x, center_y,
    size, angle, 1, pose, DTVERSION_UNKNOWN);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s: PT_PT_SetPositionIP failed", __func__);
    return IMG_ERR_INVALID_OPERATION;
  }

  rc = p_lib->FACEPROC_PT_DetectPoint(p_hnd->hpt,
    (RAWIMAGE *)FP_ADDR(p_frame),
    (INT32)FP_WIDTH(p_frame),
    (INT32)FP_HEIGHT(p_frame),
    p_hnd->hptresult);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s: FACEPROC_PT_DetectPoint failed", __func__);
    return IMG_ERR_INVALID_OPERATION;
  }

  rc = p_lib->FACEPROC_PT_GetResult(p_hnd->hptresult,
    PT_POINT_KIND_MAX, (POINT *)face_pt,
    p_output->fp.confidence);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s: PT_GetResult failed", __func__);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (verify_face) {
    /* Check if if we need to discard this face */
    if (facial_parts_wrap_discard_face_parts_based(&p_hnd->config, p_output)) {
      return IMG_ERR_NOT_FOUND;
    }
  }

  /* Fill face parts based on threshold */
  memset(p_output->fp.face_pt, 0x00, sizeof(p_output->fp.face_pt));
  if (p_output->fp.confidence[FACE_PART_MOUTH] >
    p_hnd->config.detection_threshold) {

    p_output->fp.face_pt[FACE_PART_MOUTH] =
      face_pt[FACE_PART_MOUTH];
    p_output->fp.face_pt[FACE_PART_MOUTH_UP] =
      face_pt[FACE_PART_MOUTH_UP];
    p_output->fp.face_pt[FACE_PART_MOUTH_LEFT] =
      face_pt[FACE_PART_MOUTH_LEFT];
    p_output->fp.face_pt[FACE_PART_MOUTH_RIGHT] =
      face_pt[FACE_PART_MOUTH_RIGHT];
  }
  IDBG_LOW("lefteye conf %d, right eye conf %d, threshold %d",
    p_output->fp.confidence[FACE_PART_LEFT_EYE],
    p_output->fp.confidence[FACE_PART_RIGHT_EYE],
    p_hnd->config.detection_threshold
    );

  if ((p_output->fp.confidence[FACE_PART_LEFT_EYE] >
    p_hnd->config.detection_threshold) &&
    (p_output->fp.confidence[FACE_PART_RIGHT_EYE] >
    p_hnd->config.detection_threshold)) {

    p_output->fp.face_pt[FACE_PART_LEFT_EYE] =
      face_pt[FACE_PART_LEFT_EYE];
    p_output->fp.face_pt[FACE_PART_RIGHT_EYE] =
      face_pt[FACE_PART_RIGHT_EYE];

    p_output->fp.face_pt[FACE_PART_LEFT_EYE_IN] =
      face_pt[FACE_PART_LEFT_EYE_IN];
    p_output->fp.face_pt[FACE_PART_LEFT_EYE_OUT] =
      face_pt[FACE_PART_LEFT_EYE_OUT];

    p_output->fp.face_pt[FACE_PART_RIGHT_EYE_IN] =
      face_pt[FACE_PART_RIGHT_EYE_IN];
    p_output->fp.face_pt[FACE_PART_RIGHT_EYE_OUT] =
      face_pt[FACE_PART_RIGHT_EYE_OUT];
  }

  if ((p_output->fp.confidence[FACE_PART_NOSE_LEFT] >
    p_hnd->config.detection_threshold) &&
    (p_output->fp.confidence[FACE_PART_NOSE_RIGHT] >
    p_hnd->config.detection_threshold)) {

    p_output->fp.face_pt[FACE_PART_NOSE_LEFT] =
      face_pt[FACE_PART_NOSE_LEFT];
    p_output->fp.face_pt[FACE_PART_NOSE_RIGHT] =
      face_pt[FACE_PART_NOSE_RIGHT];
  }

  rc = p_lib->FACEPROC_PT_GetFaceDirection(p_hnd->hptresult,
    &(p_output->fp.direction_up_down),
    &(p_output->fp.direction_left_right),
    &(p_output->fp.direction_roll));
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s:%d] eng_FACEPROC_PT_GetFaceDirection failed",
      __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  IDBG_MED("%s: PT cord LEFT_EYE %d, %d  up %d, left %d, roll %d",
    __func__, p_output->fp.face_pt[PT_POINT_LEFT_EYE].x,
    p_output->fp.face_pt[PT_POINT_LEFT_EYE].y,
    p_output->fp.direction_up_down,
    p_output->fp.direction_left_right,
    p_output->fp.direction_roll);

  facial_parts_validate_and_fix_PT_coordinates(p_output);

  /* Countour detection */
  if (p_hnd->config.enable_contour) {
    POINT ct_points[CT_POINT_KIND_MAX];

    p_output->ct.is_eye_valid = 0;
    p_output->ct.is_forehead_valid = 0;
    p_output->ct.is_nose_valid = 0;
    p_output->ct.is_mouth_valid = 0;
    p_output->ct.is_lip_valid = 0;
    p_output->ct.is_brow_valid = 0;
    p_output->ct.is_chin_valid = 0;
    p_output->ct.is_ear_valid = 0;

    rc = p_lib->FACEPROC_CT_SetPointFromHandle(p_hnd->hct,
      p_hnd->hptresult);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s:%d] eng_FACEPROC_CT_SetPositionFromHandle returned error: %d",
        __func__, __LINE__, rc);
      return IMG_ERR_INVALID_OPERATION;
    }

    rc = p_lib->FACEPROC_CT_DetectContour(p_hnd->hct,
      (RAWIMAGE *)FP_ADDR(p_frame),
      (INT32)FP_WIDTH(p_frame),
      (INT32)FP_HEIGHT(p_frame),
      p_hnd->hctresult);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s:%d] FACEPROC_CT_DetectContour returned error: %d",
        __func__, __LINE__, (uint32_t)rc);
      return IMG_ERR_INVALID_OPERATION;
    }

    rc = p_lib->FACEPROC_CT_GetResult(p_hnd->hctresult,
      CT_POINT_KIND_MAX, (POINT *)ct_points);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s:%d] CT_GetResult failed", __func__, __LINE__);
      return IMG_ERR_INVALID_OPERATION;
    }

    p_output->ct.is_eye_valid = 1;
    for (i = 0; i < IMGLIB_ARRAY_SIZE(convert_contour_eye); i++) {
      p_output->ct.eye_pt[convert_contour_eye[i][0]].x =
        ct_points[convert_contour_eye[i][1]].x;
      p_output->ct.eye_pt[convert_contour_eye[i][0]].y =
        ct_points[convert_contour_eye[i][1]].y;
    }


    if (p_hnd->config.contour_detection_mode == CT_DETECTION_MODE_DEFAULT) {
      p_output->ct.is_forehead_valid = 1;
      for (i = 0; i < IMGLIB_ARRAY_SIZE(convert_contour_forh); i++) {
        p_output->ct.forh_pt[convert_contour_forh[i][0]].x =
          ct_points[convert_contour_forh[i][1]].x;
        p_output->ct.forh_pt[convert_contour_forh[i][0]].y =
          ct_points[convert_contour_forh[i][1]].y;
      }


      p_output->ct.is_nose_valid = 1;
      for (i = 0; i < IMGLIB_ARRAY_SIZE(convert_contour_nose); i++) {
        p_output->ct.nose_pt[convert_contour_nose[i][0]].x =
          ct_points[convert_contour_nose[i][1]].x;
        p_output->ct.nose_pt[convert_contour_nose[i][0]].y =
          ct_points[convert_contour_nose[i][1]].y;
      }

      p_output->ct.is_mouth_valid = 1;
      for (i = 0; i < IMGLIB_ARRAY_SIZE(convert_contour_mouth); i++) {
        p_output->ct.mouth_pt[convert_contour_mouth[i][0]].x =
          ct_points[convert_contour_mouth[i][1]].x;
        p_output->ct.mouth_pt[convert_contour_mouth[i][0]].y =
          ct_points[convert_contour_mouth[i][1]].y;
      }

      p_output->ct.is_lip_valid = 1;
      for (i = 0; i < IMGLIB_ARRAY_SIZE(convert_contour_lip); i++) {
        p_output->ct.lip_pt[convert_contour_lip[i][0]].x =
          ct_points[convert_contour_lip[i][1]].x;
        p_output->ct.lip_pt[convert_contour_lip[i][0]].y =
          ct_points[convert_contour_lip[i][1]].y;
      }

      p_output->ct.is_brow_valid = 1;
      for (i = 0; i < IMGLIB_ARRAY_SIZE(convert_contour_brow); i++) {
        p_output->ct.brow_pt[convert_contour_brow[i][0]].x =
          ct_points[convert_contour_brow[i][1]].x;
        p_output->ct.brow_pt[convert_contour_brow[i][0]].y =
          ct_points[convert_contour_brow[i][1]].y;
      }

      p_output->ct.is_chin_valid = 1;
      for (i = 0; i < IMGLIB_ARRAY_SIZE(convert_contour_chin); i++) {
        p_output->ct.chin_pt[convert_contour_chin[i][0]].x =
          ct_points[convert_contour_chin[i][1]].x;
        p_output->ct.chin_pt[convert_contour_chin[i][0]].y =
          ct_points[convert_contour_chin[i][1]].y;
      }

      p_output->ct.is_ear_valid = 1;
      for (i = 0; i < IMGLIB_ARRAY_SIZE(convert_contour_ear); i++) {
        p_output->ct.ear_pt[convert_contour_ear[i][0]].x =
          ct_points[convert_contour_ear[i][1]].x;
        p_output->ct.ear_pt[convert_contour_ear[i][0]].y =
          ct_points[convert_contour_ear[i][1]].y;
      }
    }

    IDBG_MED("%s:%d] CT Coordinates for CT_POINT_EYE_L_UP %d, %d",
      __func__, __LINE__,
      ct_points[CT_POINT_EYE_L_UP].x,
      ct_points[CT_POINT_EYE_L_UP].y);
    IDBG_MED("%s:%d] CT Coordinates for CT_POINT_EYE_L_DOWN %d, %d",
      __func__, __LINE__,
      ct_points[CT_POINT_EYE_L_DOWN].x,
      ct_points[CT_POINT_EYE_L_DOWN].y);
  }

  /* Smile detection */
  if (p_hnd->config.enable_smile) {
    memset(&p_output->sm, 0x0, sizeof(fd_smile_detect));
    rc = p_lib->FACEPROC_SM_SetPointFromHandle(p_hnd->hsm,
      p_hnd->hptresult);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s:%d] eng_FACEPROC_SM_SetPointFromHandle returned error: %d",
        __func__, __LINE__, rc);
      return IMG_ERR_INVALID_OPERATION;
    }

    rc = p_lib->FACEPROC_SM_Estimate(p_hnd->hsm,
      (RAWIMAGE *)FP_ADDR(p_frame),
      (INT32)FP_WIDTH(p_frame),
      (INT32)FP_HEIGHT(p_frame),
      p_hnd->hsmresult);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s:%d] eng_FACEPROC_SM_Estimate returned error: %d",
        __func__, __LINE__, rc);
      return IMG_ERR_INVALID_OPERATION;
    }

    rc = p_lib->FACEPROC_SM_GetResult(p_hnd->hsmresult,
      &(p_output->sm.smile_degree), &(p_output->sm.confidence));
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s:%d] CT_GetResult failed", __func__, __LINE__);
      return IMG_ERR_INVALID_OPERATION;
    }
    IDBG_MED("%s:%d] SM Coordinates for %d, %d", __func__, __LINE__,
      p_output->sm.smile_degree,
      p_output->sm.confidence);
  }

  /* Gaze blink detection */
  if (p_hnd->config.enable_gaze || p_hnd->config.enable_blink) {
    p_output->blink_detected = 0;
    p_output->left_right_gaze = 0;
    p_output->top_bottom_gaze = 0;
    rc = p_lib->FACEPROC_GB_SetPointFromHandle(p_hnd->hgb,
      p_hnd->hptresult);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s:%d] eng_FACEPROC_GB_SetPointFromHandle returned error: %d",
        __func__, __LINE__, rc);
      return IMG_ERR_INVALID_OPERATION;
    }

    rc = p_lib->FACEPROC_GB_Estimate(p_hnd->hgb,
      (RAWIMAGE *)FP_ADDR(p_frame),
      (INT32)FP_WIDTH(p_frame),
      (INT32)FP_HEIGHT(p_frame),
      p_hnd->hgbresult);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s:%d] eng_FACEPROC_GB_Estimate returned error: %d",
        __func__, __LINE__, rc);
      return IMG_ERR_INVALID_OPERATION;
    }

    /* Blink detection */
    if (p_hnd->config.enable_blink) {
      rc = p_lib->FACEPROC_GB_GetEyeCloseRatio(p_hnd->hgbresult,
        &(leye_close_ratio), &(reye_close_ratio));
      if (rc != FACEPROC_NORMAL) {
        IDBG_ERROR("%s:%d] eng_FACEPROC_GB_GetEyeCloseRatio failed",
          __func__, __LINE__);
        return IMG_ERR_INVALID_OPERATION;
      }
      IDBG_MED("%s:%d] BLINK Ratio  l %d, r %d", __func__, __LINE__,
        leye_close_ratio,
        reye_close_ratio);

      p_output->left_blink = leye_close_ratio;
      p_output->right_blink = reye_close_ratio;
      if (leye_close_ratio > p_hnd->config.closed_eye_ratio ||
        reye_close_ratio > p_hnd->config.closed_eye_ratio) {

        IDBG_MED("%s:%d] EYES CLOSED", __func__, __LINE__);
        p_output->blink_detected = 1;
      } else {
        p_output->blink_detected = 0;
      }
    }

    /* Gaze detection */
    if (p_hnd->config.enable_gaze) {
      rc = p_lib->FACEPROC_GB_GetGazeDirection(p_hnd->hgbresult,
        &(pn_gaze_left_right),&(pn_gaze_up_down));
      if (rc != FACEPROC_NORMAL) {
        IDBG_ERROR("%s:%d] eng_FACEPROC_GB_GetGazeDirection failed",
          __func__, __LINE__);
        return IMG_ERR_INVALID_OPERATION;
      }
      IDBG_MED("%s:%d] Gaze Ratio left_right %d, top_bottom %d",
        __func__, __LINE__,
        pn_gaze_left_right,
        pn_gaze_up_down);
      p_output->left_right_gaze = pn_gaze_left_right;
      p_output->top_bottom_gaze = pn_gaze_up_down;
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: facial_parts_wrap_destroy.
 *
 * Description: Facial part wrapper destroy.
 *
 * Input parameters:
 *   handle - Pointer to wrapper handler.
 *
 * Notes: none
 **/
void facial_parts_wrap_destroy(void *handle)
{
  facial_parts_wrap_t *p_hnd;
  facial_parts_wrap_lib_t *p_lib;

  if (handle == NULL) {
    return;
  }
  p_hnd = (facial_parts_wrap_t *)handle;
  p_lib = &p_hnd->p_lib;

  if (p_hnd->hgb) {
    p_lib->FACEPROC_GB_DeleteHandle(p_hnd->hgb);
    if (p_hnd->hgbresult) {
      p_lib->FACEPROC_GB_DeleteResultHandle(p_hnd->hgbresult);
    }
  }

  if (p_hnd->hsm) {
    p_lib->FACEPROC_SM_DeleteHandle(p_hnd->hsm);
    if (p_hnd->hsmresult) {
      p_lib->FACEPROC_SM_DeleteResultHandle(p_hnd->hsmresult);
    }
  }

  if (p_hnd->hct) {
    p_lib->FACEPROC_CT_DeleteHandle(p_hnd->hct);
    if (p_hnd->hctresult) {
      p_lib->FACEPROC_CT_DeleteResultHandle(p_hnd->hctresult);
    }
  }

  if (p_hnd->hpt) {
    p_lib->FACEPROC_PT_DeleteHandle(p_hnd->hpt);
    if (p_hnd->hptresult) {
      p_lib->FACEPROC_PT_DeleteResultHandle(p_hnd->hptresult);
    }
  }

  if (p_hnd->hdt) {
    p_lib->FACEPROC_DeleteDetection(p_hnd->hdt);
  }

  if (p_hnd->hdtresult) {
    p_lib->FACEPROC_DeleteDtResult(p_hnd->hdtresult);
  }

  free(p_hnd);
}
