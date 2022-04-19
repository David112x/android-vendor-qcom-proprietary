/**********************************************************************
*  Copyright (c) 2013-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include <dlfcn.h>
#include <math.h>
#include "faceproc_comp.h"
#include "faceproc_common.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#include <cutils/trace.h>

const char FACE_ALBUM[] =  QCAMERA_DUMP_FRM_LOCATION"fdAlbum";

#define FD_MAX_DUMP_CNT 10
// static const unsigned skip_count = 10;
// static const unsigned initial_skip_count = 5;
static unsigned dump_count = 0;

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

#ifdef FD_WITH_DSP
/**
 * Function: faceproc_comp_eng_load_dt_dsp
 *
 * Description: Loads the faceproc library for DSP
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
static int faceproc_comp_eng_load_dt_dsp(faceproc_lib_t *p_lib)
{

  /* Load adsp stub lib */
  if (p_lib->ptr_stub == NULL) {
    p_lib->ptr_stub =
      dlopen("libmmcamera_imglib_faceproc_adspstub.so", RTLD_NOW);
    IDBG_MED("%s] ptr_stub: %p", __func__, p_lib->ptr_stub);
    if (!p_lib->ptr_stub) {
      IDBG_ERROR("%s:%d] Error loading libmmcamera_imglib_faceproc_adspstub.so lib",
        __func__, __LINE__);
      return IMG_ERR_GENERAL;
    }
  }

  /* Link all the fd dsp functions in adsp stub lib */
  *(void **)&(p_lib->FACEPROC_Dt_VersionDSP) =
    dlsym(p_lib->ptr_stub, "adsp_fd_getVersion");
  if (p_lib->FACEPROC_Dt_VersionDSP == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_Dt_VersionDSP error", __func__);
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->FACEPROC_DeleteDtResult) =
    dlsym(p_lib->ptr_stub, "adsp_fd_DeleteDtResult");
  if (p_lib->FACEPROC_DeleteDtResult == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_DeleteDtResult error", __func__);
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->FACEPROC_DeleteDetection) =
    dlsym(p_lib->ptr_stub, "adsp_fd_DeleteDetection");
  if (p_lib->FACEPROC_DeleteDetection == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_DeleteDetection error", __func__);
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->FACEPROC_GetDtFaceCount) =
    dlsym(p_lib->ptr_stub, "adsp_fd_GetDtFaceCount");
  if (p_lib->FACEPROC_GetDtFaceCount == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_GetDtFaceCount error", __func__);
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->FACEPROC_GetDtFaceInfo) =
    dlsym(p_lib->ptr_stub, "adsp_fd_GetDtFaceInfo");
  if (p_lib->FACEPROC_GetDtFaceInfo == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_GetDtFaceInfo error", __func__);
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->FACEPROC_CreateDetection) =
    dlsym(p_lib->ptr_stub, "adsp_fd_CreateDetection");
  if (p_lib->FACEPROC_CreateDetection == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_CreateDetection error", __func__);
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->FACEPROC_SetDtMode) =
    dlsym(p_lib->ptr_stub, "adsp_fd_SetDtMode");
  if (p_lib->FACEPROC_SetDtMode == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SetDtMode error", __func__);
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->FACEPROC_SetDtStep) =
    dlsym(p_lib->ptr_stub, "adsp_fd_SetDtStep");
  if (p_lib->FACEPROC_SetDtStep == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SetDtStep error", __func__);
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->FACEPROC_SetDtAngle) =
    dlsym(p_lib->ptr_stub, "adsp_fd_SetDtAngle");
  if (p_lib->FACEPROC_SetDtAngle == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SetDtAngle error", __func__);
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->FACEPROC_SetDtDirectionMask) =
    dlsym(p_lib->ptr_stub, "adsp_fd_SetDtDirectionMask");
  if (p_lib->FACEPROC_SetDtDirectionMask == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SetDtDirectionMask error", __func__);
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->FACEPROC_SetDtFaceSizeRange) =
    dlsym(p_lib->ptr_stub, "adsp_fd_SetDtFaceSizeRange");
  if (p_lib->FACEPROC_SetDtFaceSizeRange == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SetDtFaceSizeRange error", __func__);
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->FACEPROC_SetDtThreshold) =
    dlsym(p_lib->ptr_stub, "adsp_fd_SetDtThreshold");
  if (p_lib->FACEPROC_SetDtThreshold == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SetDtThreshold error", __func__);
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->FACEPROC_CreateDtResult) =
    dlsym(p_lib->ptr_stub, "adsp_fd_CreateDtResult");
  if (p_lib->FACEPROC_CreateDtResult == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_CreateDtResult error", __func__);
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->FACEPROC_DetectionDSP) =
    dlsym(p_lib->ptr_stub, "adsp_fd_Detection");
  if (p_lib->FACEPROC_DetectionDSP == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_DetectionDSP error", __func__);
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->FACEPROC_SetDtRefreshCount) =
    dlsym(p_lib->ptr_stub, "adsp_fd_SetDtRefreshCount");
  if (p_lib->FACEPROC_SetDtRefreshCount == NULL) {
    IDBG_ERROR("%s Loading FACEPROC_SetDtRefreshCount error", __func__);
    return IMG_ERR_GENERAL;
  }

  p_lib->status_dsp_lib = 1;  /* FD DSP lib loaded */
  return IMG_SUCCESS;
}

#endif

/**
 * Function: faceproc_comp_eng_load_dt_arm
 *
 * Description: Loads the faceproc library for ARM
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
static int faceproc_comp_eng_load_dt_arm(faceproc_lib_t *p_lib)
{
  int rc = 0;

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GetDtVersion) =
      dlsym(p_lib->ptr, "FACEPROC_GetDtVersion");
    if (p_lib->FACEPROC_GetDtVersion == NULL) {
      IDBG_ERROR("Error Loading FACEPROC_GetDtVersion");
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_DeleteDtResult) =
      dlsym(p_lib->ptr, "FACEPROC_DeleteDtResult");
    if (p_lib->FACEPROC_DeleteDtResult == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_DeleteDtResult ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_DeleteDetection) =
      dlsym(p_lib->ptr, "FACEPROC_DeleteDetection");
    if (p_lib->FACEPROC_DeleteDetection == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_DeleteDetection ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GetDtFaceCount) =
      dlsym(p_lib->ptr, "FACEPROC_GetDtFaceCount");
    if (p_lib->FACEPROC_GetDtFaceCount == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_GetDtFaceCount ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GetDtFaceInfo) =
      dlsym(p_lib->ptr, "FACEPROC_GetDtFaceInfo");
    if (p_lib->FACEPROC_GetDtFaceInfo == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_GetDtFaceInfo ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtMemorySize) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtMemorySize");
    if (p_lib->FACEPROC_SetDtMemorySize == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_SetDtMemorySize ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GetDtRequiredMovieMemSize) =
      dlsym(p_lib->ptr, "FACEPROC_GetDtRequiredMovieMemSize");
    if (p_lib->FACEPROC_GetDtRequiredMovieMemSize == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_GetDtRequiredMovieMemSize ",
        __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CreateDetection) =
      dlsym(p_lib->ptr, "FACEPROC_CreateDetection");
    if (p_lib->FACEPROC_CreateDetection == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_CreateDetection ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtMode) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtMode");
    if (p_lib->FACEPROC_SetDtMode == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_SetDtMode ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtStep) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtStep");
    if (p_lib->FACEPROC_SetDtStep == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_SetDtStep ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtAngle) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtAngle");
    if (p_lib->FACEPROC_SetDtAngle == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_SetDtAngle ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtDirectionMask) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtDirectionMask");
    if (p_lib->FACEPROC_SetDtDirectionMask == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_SetDtDirectionMask ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtFaceSizeRange) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtFaceSizeRange");
    if (p_lib->FACEPROC_SetDtFaceSizeRange == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_SetDtFaceSizeRange ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtThreshold) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtThreshold");
    if (p_lib->FACEPROC_SetDtThreshold == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_SetDtThreshold ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CreateDtResult) =
      dlsym(p_lib->ptr, "FACEPROC_CreateDtResult");
    if (p_lib->FACEPROC_CreateDtResult == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_CreateDtResult ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_Detection) =
      dlsym(p_lib->ptr, "FACEPROC_Detection");
    if (p_lib->FACEPROC_Detection == NULL) {
      IDBG_ERROR("%s Error Loading FACEPROC_Detection ", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FreeBMemoryArea) =
      dlsym(p_lib->ptr, "FACEPROC_FreeBMemoryArea");
    if (p_lib->FACEPROC_FreeBMemoryArea == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FreeBMemoryArea error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FreeWMemoryArea) =
      dlsym(p_lib->ptr, "FACEPROC_FreeWMemoryArea");
    if (p_lib->FACEPROC_FreeWMemoryArea == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FreeWMemoryArea error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetBMemoryArea) =
      dlsym(p_lib->ptr, "FACEPROC_SetBMemoryArea");
    if (p_lib->FACEPROC_SetBMemoryArea == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SetBMemoryArea error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetWMemoryArea) =
      dlsym(p_lib->ptr, "FACEPROC_SetWMemoryArea");
    if (p_lib->FACEPROC_SetWMemoryArea == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SetWMemoryArea error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }

  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtRefreshCount) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtRefreshCount");
    if (p_lib->FACEPROC_SetDtRefreshCount == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SetDtRefreshCount error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  p_lib->status_dsp_lib = 0;  /* FD ARM lib loaded */
  return rc;
}

/**
 * Function: faceproc_comp_eng_unload
 *
 * Description: Unload the faceproc library
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
void faceproc_comp_eng_unload(faceproc_lib_t *p_lib)
{
  IDBG_HIGH("%s:%d] E", __func__, __LINE__);
  if (p_lib->ptr) {
    dlclose(p_lib->ptr);
    p_lib->ptr = NULL;
  }
  if (p_lib->ptr_stub) {
    dlclose(p_lib->ptr_stub);
    p_lib->ptr_stub = NULL;
  }
  memset(p_lib, 0, sizeof(faceproc_lib_t));
}

/**
 * Function: faceproc_comp_eng_load
 *
 * Description: Loads the faceproc library
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
int faceproc_comp_eng_load(faceproc_lib_t *p_lib)
{
  int rc = 0;
  IDBG_MED("%s:%d] E", __func__, __LINE__);
  p_lib->ptr =
    dlopen("libmmcamera_faceproc.so", RTLD_NOW);
  if (!p_lib->ptr) {
    IDBG_ERROR("%s:%d] Error opening libmmcamera_faceproc.so lib",
      __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

#ifdef FD_WITH_DSP
  if (p_lib->load_dsp_lib) {
    rc = faceproc_comp_eng_load_dt_dsp(p_lib);
  } else {
    rc = faceproc_comp_eng_load_dt_arm(p_lib);
  }
#else
  rc = faceproc_comp_eng_load_dt_arm(p_lib);
#endif

  /*Pt detection */
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_CreateHandle) =
      dlsym(p_lib->ptr, "FACEPROC_PT_CreateHandle");
    if (p_lib->FACEPROC_PT_CreateHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_CreateHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_DeleteHandle) =
      dlsym(p_lib->ptr, "FACEPROC_PT_DeleteHandle");
    if (p_lib->FACEPROC_PT_DeleteHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_DeleteHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_CreateResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_PT_CreateResultHandle");
    if (p_lib->FACEPROC_PT_CreateResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_CreateResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_DeleteResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_PT_DeleteResultHandle");
    if (p_lib->FACEPROC_PT_DeleteResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_DeleteResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_SetPositionFromHandle) =
      dlsym(p_lib->ptr, "FACEPROC_PT_SetPositionFromHandle");
    if (p_lib->FACEPROC_PT_SetPositionFromHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_SetPositionFromHandle error",
        __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_SetMode) =
      dlsym(p_lib->ptr,
      "FACEPROC_PT_SetMode");
    if (p_lib->FACEPROC_PT_SetMode == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_SetMode error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_SetConfMode) =
      dlsym(p_lib->ptr, "FACEPROC_PT_SetConfMode");
    if (p_lib->FACEPROC_PT_SetConfMode == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_SetConfMode error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_DetectPoint) =
      dlsym(p_lib->ptr, "FACEPROC_PT_DetectPoint");
    if (p_lib->FACEPROC_PT_DetectPoint == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_DetectPoint error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_GetResult) =
      dlsym(p_lib->ptr, "FACEPROC_PT_GetResult");
    if (p_lib->FACEPROC_PT_GetResult == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_GetResult error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_PT_GetFaceDirection) =
      dlsym(p_lib->ptr, "FACEPROC_PT_GetFaceDirection");
    if (p_lib->FACEPROC_PT_GetFaceDirection == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_PT_GetFaceDirection error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_CreateHandle) =
      dlsym(p_lib->ptr, "FACEPROC_CT_CreateHandle");
    if (p_lib->FACEPROC_CT_CreateHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_CreateHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_DeleteHandle) =
      dlsym(p_lib->ptr, "FACEPROC_CT_DeleteHandle");
    if (p_lib->FACEPROC_CT_DeleteHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_DeleteHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_CreateResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_CT_CreateResultHandle");
    if (p_lib->FACEPROC_CT_CreateResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_CreateResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_DeleteResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_CT_DeleteResultHandle");
    if (p_lib->FACEPROC_CT_DeleteResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_DeleteResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_SetPointFromHandle) =
      dlsym(p_lib->ptr, "FACEPROC_CT_SetPointFromHandle");
    if (p_lib->FACEPROC_CT_SetPointFromHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_SetPointFromHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_DetectContour) =
      dlsym(p_lib->ptr, "FACEPROC_CT_DetectContour");
    if (p_lib->FACEPROC_CT_DetectContour == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_DetectContour error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_GetResult) =
      dlsym(p_lib->ptr, "FACEPROC_CT_GetResult");
    if (p_lib->FACEPROC_CT_GetResult == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_GetResult error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_SetDetectionMode) =
      dlsym(p_lib->ptr, "FACEPROC_CT_SetDetectionMode");
    if (p_lib->FACEPROC_CT_SetDetectionMode == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_SetDetectionMode error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  /* Smile API */
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SM_CreateHandle) =
      dlsym(p_lib->ptr, "FACEPROC_SM_CreateHandle");
    if (p_lib->FACEPROC_SM_CreateHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SM_CreateHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SM_DeleteHandle) =
      dlsym(p_lib->ptr, "FACEPROC_SM_DeleteHandle");
    if (p_lib->FACEPROC_SM_DeleteHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SM_DeleteHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SM_CreateResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_SM_CreateResultHandle");
    if (p_lib->FACEPROC_SM_CreateResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SM_CreateResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SM_DeleteResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_SM_DeleteResultHandle");
    if (p_lib->FACEPROC_SM_DeleteResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SM_DeleteResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SM_SetPointFromHandle) =
      dlsym(p_lib->ptr, "FACEPROC_SM_SetPointFromHandle");
    if (p_lib->FACEPROC_SM_SetPointFromHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SM_SetPointFromHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SM_Estimate) =
      dlsym(p_lib->ptr, "FACEPROC_SM_Estimate");
    if (p_lib->FACEPROC_SM_Estimate == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SM_Estimate error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SM_GetResult) =
      dlsym(p_lib->ptr, "FACEPROC_SM_GetResult");
    if (p_lib->FACEPROC_SM_GetResult == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_SM_GetResult error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_CT_SetDetectionMode) =
      dlsym(p_lib->ptr, "FACEPROC_CT_SetDetectionMode");
    if (p_lib->FACEPROC_CT_SetDetectionMode == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_CT_SetDetectionMode error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  /* Face Recognition */
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_CreateFeatureHandle) =
      dlsym(p_lib->ptr, "FACEPROC_FR_CreateFeatureHandle");
    if (p_lib->FACEPROC_FR_CreateFeatureHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_CreateFeatureHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_DeleteFeatureHandle) =
      dlsym(p_lib->ptr, "FACEPROC_FR_DeleteFeatureHandle");
    if (p_lib->FACEPROC_FR_DeleteFeatureHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_DeleteFeatureHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_CreateAlbumHandle) =
      dlsym(p_lib->ptr, "FACEPROC_FR_CreateAlbumHandle");
    if (p_lib->FACEPROC_FR_CreateAlbumHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_CreateAlbumHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_DeleteAlbumHandle) =
      dlsym(p_lib->ptr, "FACEPROC_FR_DeleteAlbumHandle");
    if (p_lib->FACEPROC_FR_DeleteAlbumHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_DeleteAlbumHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_ExtractFeatureFromPtHdl) =
      dlsym(p_lib->ptr, "FACEPROC_FR_ExtractFeatureFromPtHdl");
    if (p_lib->FACEPROC_FR_ExtractFeatureFromPtHdl == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_ExtractFeatureFromPtHdl error",
        __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_RegisterData) =
      dlsym(p_lib->ptr, "FACEPROC_FR_RegisterData");
    if (p_lib->FACEPROC_FR_RegisterData == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_RegisterData error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_GetRegisteredUserNum) =
      dlsym(p_lib->ptr, "FACEPROC_FR_GetRegisteredUserNum");
    if (p_lib->FACEPROC_FR_GetRegisteredUserNum == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_GetRegisteredUserNum error",
        __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_GetRegisteredUsrDataNum) =
      dlsym(p_lib->ptr, "FACEPROC_FR_GetRegisteredUsrDataNum");
    if (p_lib->FACEPROC_FR_GetRegisteredUsrDataNum == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_GetRegisteredUsrDataNum error",
        __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_ClearAlbum) =
      dlsym(p_lib->ptr, "FACEPROC_FR_ClearAlbum");
    if (p_lib->FACEPROC_FR_ClearAlbum == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_ClearAlbum error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_GetSerializedAlbumSize) =
      dlsym(p_lib->ptr, "FACEPROC_FR_GetSerializedAlbumSize");
    if (p_lib->FACEPROC_FR_GetSerializedAlbumSize == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_GetSerializedAlbumSize error",
        __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_SerializeAlbum) =
      dlsym(p_lib->ptr, "FACEPROC_FR_SerializeAlbum");
    if (p_lib->FACEPROC_FR_SerializeAlbum == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_SerializeAlbum error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_RestoreAlbum) =
      dlsym(p_lib->ptr, "FACEPROC_FR_RestoreAlbum");
    if (p_lib->FACEPROC_FR_RestoreAlbum == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_RestoreAlbum error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_Identify) =
      dlsym(p_lib->ptr, "FACEPROC_FR_Identify");
    if (p_lib->FACEPROC_FR_Identify == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_Identify error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_Verify) =
      dlsym(p_lib->ptr, "FACEPROC_FR_Verify");
    if (p_lib->FACEPROC_FR_Verify == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_Verify error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_FR_IsRegistered) =
      dlsym(p_lib->ptr, "FACEPROC_FR_IsRegistered");
    if (p_lib->FACEPROC_FR_IsRegistered == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_FR_IsRegistered error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  /* Blink Detect */
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_CreateHandle) =
      dlsym(p_lib->ptr, "FACEPROC_GB_CreateHandle");
    if (p_lib->FACEPROC_GB_CreateHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_CreateHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_DeleteHandle) =
      dlsym(p_lib->ptr, "FACEPROC_GB_DeleteHandle");
    if (p_lib->FACEPROC_GB_DeleteHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_DeleteHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_CreateResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_GB_CreateResultHandle");
    if (p_lib->FACEPROC_GB_CreateResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_CreateResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_DeleteResultHandle) =
      dlsym(p_lib->ptr, "FACEPROC_GB_DeleteResultHandle");
    if (p_lib->FACEPROC_GB_DeleteResultHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_DeleteResultHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_SetPointFromHandle) =
      dlsym(p_lib->ptr, "FACEPROC_GB_SetPointFromHandle");
    if (p_lib->FACEPROC_GB_SetPointFromHandle == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_SetPointFromHandle error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_Estimate) =
      dlsym(p_lib->ptr, "FACEPROC_GB_Estimate");
    if (p_lib->FACEPROC_GB_Estimate == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_Estimate error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_GetEyeCloseRatio) =
      dlsym(p_lib->ptr, "FACEPROC_GB_GetEyeCloseRatio");
    if (p_lib->FACEPROC_GB_GetEyeCloseRatio == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_GetEyeCloseRatio error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_GB_GetGazeDirection) =
      dlsym(p_lib->ptr, "FACEPROC_GB_GetGazeDirection");
    if (p_lib->FACEPROC_GB_GetGazeDirection == NULL) {
      IDBG_ERROR("%s Loading FACEPROC_GB_GetGazeDirection error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
#ifndef FD_WITH_DSP
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtLostParam) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtLostParam");
    if (p_lib->FACEPROC_SetDtLostParam == NULL) {
      IDBG_ERROR("%s GJ Loading FACEPROC_SetDtLostParam error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_SetDtModifyMoveRate) =
      dlsym(p_lib->ptr, "FACEPROC_SetDtModifyMoveRate");
    if (p_lib->FACEPROC_SetDtModifyMoveRate == NULL) {
      IDBG_ERROR("%s GJ Loading FACEPROC_SetDtModifyMoveRate error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!rc) {
    *(void **)&(p_lib->FACEPROC_DtLockID) =
      dlsym(p_lib->ptr, "FACEPROC_DtLockID");
    if (p_lib->FACEPROC_DtLockID == NULL) {
      IDBG_ERROR("%s GJ Loading FACEPROC_DtLockID error", __func__);
      rc = IMG_ERR_GENERAL;
    }
  }
#endif
  if (rc < 0) {
    faceproc_comp_eng_unload(p_lib);
  }
  return rc;
}

/**
 * Function: faceproc_register_frame
 *
 * Description: Register the frame to faceproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc object
 *   p_frame - Input frame
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_register_frame(faceproc_comp_t *p_comp, img_frame_t *p_frame,
  int num_faces)
{
  IMG_UNUSED(p_frame);

  INT32 i;
  INT32 pn_user_data_num;
  INT32 user_idx, data_idx;
  int rc = 0;

  if (num_faces <= 0)
    return IMG_SUCCESS;
  if (p_comp->last_img_registered_idx >= MAX_REGISTER_PPL)
    return IMG_ERR_GENERAL;

  for (i = 0; i < num_faces; i++) {
    if (p_comp->recognized[i] == 0) {
      IDBG_MED("Face not registered. Adding new Entry");
      user_idx = (p_comp->last_img_registered_idx % MAX_REGISTER_PPL);
      p_comp->user_id[i] = user_idx;
      p_comp->last_img_registered_idx++;
      data_idx = 0;
    } else {
      IDBG_MED("Face already registered. Updating Entry");
      rc = p_comp->p_lib->
        FACEPROC_FR_GetRegisteredUsrDataNum(p_comp->halbum,
        p_comp->user_id[i],
        &pn_user_data_num);
      user_idx = p_comp->user_id[i];
      data_idx = pn_user_data_num % MAX_REGISTER_IMG_PER_PERSON;
      IDBG_MED("value of pn_user_data_num %d", pn_user_data_num);
    }

    IDBG_MED("REGISTERING DATA AT user_id %d dataid %d ",
      user_idx, data_idx);
    rc = p_comp->p_lib->FACEPROC_FR_RegisterData(p_comp->halbum,
      p_comp->hfeature, user_idx, data_idx);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s:%d]FACEPROC_FR_RegisterData returned error: %d",
        __func__, __LINE__, (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_save_album
 *
 * Description: Save Registered faces album
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc object
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_save_album(faceproc_comp_t *p_comp)
{
  FILE *fp;
  uint8_t *buffer = NULL;
  UINT32 punSerializedAlbumSize = 0;
  int rc;

  if (!(p_comp && p_comp->halbum)) {
    IDBG_MED("%s:%d] Error No album ", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }
  rc = p_comp->p_lib->FACEPROC_FR_GetSerializedAlbumSize(p_comp->halbum,
    (UINT32 *)&punSerializedAlbumSize);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s:%d] eng_FACEPROC_FR_GetSerializedAlbumSize failed %d",
      __func__, __LINE__, rc);
    return IMG_ERR_GENERAL;
  }

  if (punSerializedAlbumSize > 0 && punSerializedAlbumSize > MAX_ALBUM_SIZE) {
    IDBG_ERROR("%s:%d] eng_FACEPROC_FR_GetSerializedAlbumSize failed %d %d",
      __func__, __LINE__, rc, punSerializedAlbumSize);
    return IMG_ERR_GENERAL;
  }
  buffer = (uint8_t *)malloc(MAX_ALBUM_SIZE);
  if (NULL == buffer) {
    IDBG_ERROR("%s:%d] eng_FACEPROC_FR_GetSerializedAlbumSize failed %d",
      __func__, __LINE__, rc);
    return IMG_ERR_GENERAL;
  }
  memset(buffer, 0, MAX_ALBUM_SIZE);
  rc = p_comp->p_lib->FACEPROC_FR_SerializeAlbum(p_comp->halbum, buffer,
    MAX_ALBUM_SIZE);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s:%d] eng_FACEPROC_FR_SerializeAlbum failed %d",
      __func__, __LINE__, rc);
    free(buffer);
    return IMG_ERR_GENERAL;
  }

  fp = fopen(FACE_ALBUM, "wb");
  if (fp != NULL) {
    fwrite(buffer, 1, MAX_ALBUM_SIZE, fp);
    IDBG_MED("%s:%d] Writing to /data/vendor/camera", __func__, __LINE__);
    fclose(fp);
  } else {
    IDBG_ERROR("%s: cannot open file name: %s", __func__, FACE_ALBUM);
  }

  free(buffer);

  return IMG_SUCCESS;
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
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int faceproc_fd_output(faceproc_comp_t *p_comp,
  faceproc_result_t *fd_data,
  INT32 *num_faces)
{
  POINT ct_points[CT_POINT_KIND_MAX];
  int rc, leye_close_ratio, reye_close_ratio;
  int pn_gaze_left_right, pn_gaze_up_down;
  uint32_t i, ct;
  faceproc_info_t *p_output = NULL;

  /* FD START */
  /* Get the number of faces */
  rc = p_comp->p_lib->FACEPROC_GetDtFaceCount(p_comp->hresult, num_faces);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_GetDtFaceCount returned error: %d", rc);
    *num_faces = 0;
    return IMG_ERR_GENERAL;
  }
  IDBG_MED("num_faces %d", *num_faces);

  /* Parse and store the faces */
  fd_data->num_faces_detected = (uint32_t)*num_faces;
  fd_data->frame_id = p_comp->frame_id;

  if (fd_data->num_faces_detected > MAX_FACE_ROI)
    fd_data->num_faces_detected = MAX_FACE_ROI;

  if (!fd_data->num_faces_detected) {
    for (i = 0; i < MAX_FACE_ROI; i++) {
      p_output = &fd_data->roi[i];
      p_output->blink_detected = 0;
      p_output->left_blink = 0;
      p_output->right_blink = 0;
      p_output->left_right_gaze = 0;
      p_output->top_bottom_gaze = 0;
    }
  }

  boolean enable_face_landmarks;
  boolean enable_contour, enable_smile, enable_blink, enable_gaze;

  faceproc_common_get_feature_flags(&p_comp->config, &p_comp->fd_chromatix,
    &enable_face_landmarks, &enable_contour,
    &enable_smile, &enable_blink, &enable_gaze);

  for (i = 0; i < fd_data->num_faces_detected; i++) {
    FACEINFO face_info;
    uint32_t left, top, right, bottom;
    rc = p_comp->p_lib->FACEPROC_GetDtFaceInfo(p_comp->hresult, (int32_t)i,
      &face_info);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_GetDtFaceInfo returned error: %d", rc);
      fd_data->num_faces_detected--;
      return IMG_ERR_GENERAL;
    }
#ifndef FD_WITH_DSP
    if ( p_comp->fd_chromatix.lock_faces && face_info.nID > 0) {
      rc = p_comp->p_lib->FACEPROC_DtLockID(p_comp->hresult, face_info.nID);
      if (rc != FACEPROC_NORMAL) {
        IDBG_ERROR("FACEPROC_DtLockID returned error: %d", rc);
        return IMG_ERR_GENERAL;
      }
    }
#endif

    /* Translate the data */
    /* Clip each detected face coordinates to be within the frame boundary */
    CLIP(face_info.ptLeftTop.x, 0, (int32_t)p_comp->width);
    CLIP(face_info.ptRightTop.x, 0, (int32_t)p_comp->width);
    CLIP(face_info.ptLeftBottom.x, 0, (int32_t)p_comp->width);
    CLIP(face_info.ptRightBottom.x, 0,
      (int32_t)p_comp->width);
    CLIP(face_info.ptLeftTop.y, 0, (int32_t)p_comp->height);
    CLIP(face_info.ptRightTop.y, 0, (int32_t)p_comp->height);
    CLIP(face_info.ptLeftBottom.y, 0,
      (int32_t)p_comp->height);
    CLIP(face_info.ptRightBottom.y, 0,
      (int32_t)p_comp->height);

    /* Find the bounding box */
    left = (uint32_t)MIN4(face_info.ptLeftTop.x, face_info.ptRightTop.x,
      face_info.ptLeftBottom.x, face_info.ptRightBottom.x);
    top = (uint32_t)MIN4(face_info.ptLeftTop.y, face_info.ptRightTop.y,
      face_info.ptLeftBottom.y, face_info.ptRightBottom.y);
    right = (uint32_t)MAX4(face_info.ptLeftTop.x, face_info.ptRightTop.x,
      face_info.ptLeftBottom.x, face_info.ptRightBottom.x);
    bottom = (uint32_t)MAX4(face_info.ptLeftTop.y, face_info.ptRightTop.y,
      face_info.ptLeftBottom.y, face_info.ptRightBottom.y);

    p_output = &fd_data->roi[i];

    if (p_comp->clip_face_data) {
      POINT center;
      POINT *p_left, *p_right;
      uint32_t face_len;
      p_left = &face_info.ptLeftTop;
      p_right = &face_info.ptRightBottom;

      int32_t x_delta = p_right->x - p_left->x;
      int32_t y_delta = p_right->y - p_left->y;
      face_len = (uint32_t)sqrt((uint32_t)((pow(y_delta, 2) + pow(x_delta, 2))) >> 1);
      center.x = ((p_right->x + p_left->x) + 1) >> 1;
      center.y = ((p_right->y + p_left->y) + 1) >> 1;

      IDBG_MED("face_len %d center (%d %d) old (%d %d)",
        face_len, center.x, center.y,
        left + ((right - left) >> 1),
        top + ((bottom - top) >> 1));
      left = (uint32_t)center.x - (face_len >> 1);
      top = (uint32_t)center.y - (face_len >> 1);
      p_output->face_boundary.dx = face_len;
      p_output->face_boundary.dy = face_len;
    } else {
      p_output->face_boundary.dx = right - left;
      p_output->face_boundary.dy = bottom - top;
    }

    p_output->gaze_angle = face_info.nPose;
    p_output->face_boundary.x = left;
    p_output->face_boundary.y = top;
    p_output->unique_id = abs(face_info.nID);
    p_output->tracking = (face_info.nID < 0) ? true : false;
    p_output->fd_confidence = face_info.nConfidence;

    if (enable_face_landmarks == TRUE) {
      fd_pixel_t face_pt[MAX_FACE_PART];
      memset(face_pt, 0x00, sizeof(face_pt));
      /* Pt result */
#if(FACE_PART_DETECT)
      rc = p_comp->p_lib->FACEPROC_PT_GetResult(p_comp->hptresult[i],
        PT_POINT_KIND_MAX, (POINT *)face_pt,
        p_output->fp.confidence);
      if (rc != FACEPROC_NORMAL) {
        IDBG_ERROR("PT_GetResult failed");
        return IMG_ERR_GENERAL;
      }

      memset(p_output->fp.face_pt, 0x00, sizeof(p_output->fp.face_pt));
      if ((uint32_t)p_output->fp.confidence[FACE_PART_MOUTH] >
        p_comp->fd_chromatix.facial_parts_threshold) {

        p_output->fp.face_pt[FACE_PART_MOUTH] =
          face_pt[FACE_PART_MOUTH];
        p_output->fp.face_pt[FACE_PART_MOUTH_UP] =
          face_pt[FACE_PART_MOUTH_UP];
        p_output->fp.face_pt[FACE_PART_MOUTH_LEFT] =
          face_pt[FACE_PART_MOUTH_LEFT];
        p_output->fp.face_pt[FACE_PART_MOUTH_RIGHT] =
          face_pt[FACE_PART_MOUTH_RIGHT];
      }

      IDBG_MED("Eye confidence %d %d",
        p_output->fp.confidence[FACE_PART_LEFT_EYE],
        p_output->fp.confidence[FACE_PART_RIGHT_EYE]);
      if (((uint32_t)p_output->fp.confidence[FACE_PART_LEFT_EYE] >
        p_comp->fd_chromatix.facial_parts_threshold) &&
        ((uint32_t)p_output->fp.confidence[FACE_PART_RIGHT_EYE] >
        p_comp->fd_chromatix.facial_parts_threshold)) {

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

      if (((uint32_t)p_output->fp.confidence[FACE_PART_NOSE_LEFT] >
        p_comp->fd_chromatix.facial_parts_threshold) &&
        ((uint32_t)p_output->fp.confidence[FACE_PART_NOSE_RIGHT] >
        p_comp->fd_chromatix.facial_parts_threshold)) {

        p_output->fp.face_pt[FACE_PART_NOSE_LEFT] =
          face_pt[FACE_PART_NOSE_LEFT];
        p_output->fp.face_pt[FACE_PART_NOSE_RIGHT] =
          face_pt[FACE_PART_NOSE_RIGHT];
      }

      rc = p_comp->p_lib->FACEPROC_PT_GetFaceDirection(p_comp->hptresult[i],
        &(p_output->fp.direction_up_down),
        &(p_output->fp.direction_left_right),
        &(p_output->fp.direction_roll));
      IDBG_MED("PT cord[%d] LEFT_EYE %d, %d  up %d, left %d, roll %d",
        i, p_output->fp.face_pt[PT_POINT_LEFT_EYE].x,
        p_output->fp.face_pt[PT_POINT_LEFT_EYE].y,
        p_output->fp.direction_up_down,
        p_output->fp.direction_left_right,
        p_output->fp.direction_roll);
      if (rc != FACEPROC_NORMAL) {
        IDBG_ERROR("eng_FACEPROC_PT_GetFaceDirection failed");
        return IMG_ERR_GENERAL;
      }
#endif
#if(FACE_CONTOUR_DETECT)
      if (enable_contour == TRUE) {
        p_output->ct.is_eye_valid = 0;
        p_output->ct.is_forehead_valid = 0;
        p_output->ct.is_nose_valid = 0;
        p_output->ct.is_mouth_valid = 0;
        p_output->ct.is_lip_valid = 0;
        p_output->ct.is_brow_valid = 0;
        p_output->ct.is_chin_valid = 0;
        p_output->ct.is_ear_valid = 0;

        memset(ct_points, 0x00, sizeof(ct_points));
        rc = p_comp->p_lib->FACEPROC_CT_GetResult(p_comp->hctresult[i],
          CT_POINT_KIND_MAX, ct_points);
        if (rc != FACEPROC_NORMAL) {
          IDBG_ERROR("CT_GetResult failed");
          return IMG_ERR_GENERAL;
        }

        p_output->ct.is_eye_valid = 1;
        for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_eye); ct++) {
          p_output->ct.eye_pt[convert_contour_eye[ct][0]].x =
            ct_points[convert_contour_eye[ct][1]].x;
          p_output->ct.eye_pt[convert_contour_eye[ct][0]].y =
            ct_points[convert_contour_eye[ct][1]].y;
        }

        if (p_comp->fd_chromatix.ct_detection_mode == FD_CONTOUR_MODE_DEFAULT) {
          p_output->ct.is_forehead_valid = 1;
          for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_forh); ct++) {
            p_output->ct.forh_pt[convert_contour_forh[ct][0]].x =
              ct_points[convert_contour_forh[ct][1]].x;
            p_output->ct.forh_pt[convert_contour_forh[ct][0]].y =
              ct_points[convert_contour_forh[ct][1]].y;
          }


          p_output->ct.is_nose_valid = 1;
          for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_nose); ct++) {
            p_output->ct.nose_pt[convert_contour_nose[ct][0]].x =
              ct_points[convert_contour_nose[ct][1]].x;
            p_output->ct.nose_pt[convert_contour_nose[ct][0]].y =
              ct_points[convert_contour_nose[ct][1]].y;
          }

          p_output->ct.is_mouth_valid = 1;
          for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_mouth); ct++) {
            p_output->ct.mouth_pt[convert_contour_mouth[ct][0]].x =
              ct_points[convert_contour_mouth[ct][1]].x;
            p_output->ct.mouth_pt[convert_contour_mouth[ct][0]].y =
              ct_points[convert_contour_mouth[ct][1]].y;
          }

          p_output->ct.is_lip_valid = 1;
          for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_lip); ct++) {
            p_output->ct.lip_pt[convert_contour_lip[ct][0]].x =
              ct_points[convert_contour_lip[ct][1]].x;
            p_output->ct.lip_pt[convert_contour_lip[ct][0]].y =
              ct_points[convert_contour_lip[ct][1]].y;
          }

          p_output->ct.is_brow_valid = 1;
          for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_brow); ct++) {
            p_output->ct.brow_pt[convert_contour_brow[ct][0]].x =
              ct_points[convert_contour_brow[ct][1]].x;
            p_output->ct.brow_pt[convert_contour_brow[ct][0]].y =
              ct_points[convert_contour_brow[ct][1]].y;
          }

          p_output->ct.is_chin_valid = 1;
          for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_chin); ct++) {
            p_output->ct.chin_pt[convert_contour_chin[ct][0]].x =
              ct_points[convert_contour_chin[ct][1]].x;
            p_output->ct.chin_pt[convert_contour_chin[ct][0]].y =
              ct_points[convert_contour_chin[ct][1]].y;
          }

          p_output->ct.is_ear_valid = 1;
          for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_ear); ct++) {
            p_output->ct.ear_pt[convert_contour_ear[ct][0]].x =
              ct_points[convert_contour_ear[ct][1]].x;
            p_output->ct.ear_pt[convert_contour_ear[ct][0]].y =
              ct_points[convert_contour_ear[ct][1]].y;
          }
        }

        IDBG_MED("CT Coordinates[%d] for CT_POINT_EYE_L_UP %d, %d",
          i,
          ct_points[CT_POINT_EYE_L_UP].x,
          ct_points[CT_POINT_EYE_L_UP].y);
        IDBG_MED("CT Coordinates[%d] for CT_POINT_EYE_L_DOWN %d, %d",
          i,
          ct_points[CT_POINT_EYE_L_DOWN].x,
          ct_points[CT_POINT_EYE_L_DOWN].y);
      }

#endif
#if (FACE_SMILE_DETECT)
      if (enable_smile == TRUE) {
        rc = p_comp->p_lib->FACEPROC_SM_GetResult(p_comp->hsmresult[i],
          &(p_output->sm.smile_degree), &(p_output->sm.confidence));
        if (rc != FACEPROC_NORMAL) {
          IDBG_ERROR("CT_GetResult failed");
          return IMG_ERR_GENERAL;
        }
        IDBG_MED("SM Coordinates[%d] for %d, %d", i,
          p_output->sm.smile_degree,
          p_output->sm.confidence);
      }
#endif
      p_output->blink_detected = 0;
#if (FACE_BGS_DETECT)
      if ((enable_blink == TRUE) || (enable_gaze == TRUE)) {
        rc = p_comp->p_lib->FACEPROC_GB_GetEyeCloseRatio(p_comp->hgbresult[i],
          &(leye_close_ratio), &(reye_close_ratio));
        if (rc != FACEPROC_NORMAL) {
          IDBG_ERROR("FACEPROC_GB_GetEyeCloseRatio failed %d", rc);
          return IMG_ERR_GENERAL;
        }
        IDBG_MED("BLINK Ratio  l %d, r %d",
          leye_close_ratio,
          reye_close_ratio);
        p_output->left_blink = leye_close_ratio;
        p_output->right_blink = reye_close_ratio;
        if (leye_close_ratio > FACE_EYE_CLOSED_THRESHOLD &&
          reye_close_ratio > FACE_EYE_CLOSED_THRESHOLD) {
          IDBG_MED("EYES CLOSED");
          p_output->blink_detected = 1;
        }
        rc = p_comp->p_lib->FACEPROC_GB_GetGazeDirection(p_comp->hgbresult[i],
          &(pn_gaze_left_right),&(pn_gaze_up_down));
        if (rc != FACEPROC_NORMAL) {
          IDBG_ERROR("FACEPROC_GB_GetGazeDirection failed %d", rc);
          return IMG_ERR_GENERAL;
        }
        IDBG_MED("Gaze Ratio  left_right %d, top_bottom %d",
          pn_gaze_left_right,
          pn_gaze_up_down);
        p_output->left_right_gaze = pn_gaze_left_right;
        p_output->top_bottom_gaze = pn_gaze_up_down;
      }
#endif
    }
  }  /* end of forloop */
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_fr_output
 *
 * Description: Gets the frameproc recognition output
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   fd_data - Faceproc result data
 *   num_faces - Number of faces
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int faceproc_fr_output(faceproc_comp_t *p_comp,
  faceproc_result_t *fd_data, INT32 num_faces)
{
  int i;
  faceproc_info_t *p_output;
  for (i=0; i < num_faces;i++) {
    p_output = &fd_data->roi[i];
    p_output->is_face_recognised = p_comp->recognized[i];
    p_output->unique_id = p_comp->user_id[i];
    p_output->fd_confidence = p_comp->confidence[i];
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_register_output
 *
 * Description: Register frameproc output
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   fd_data - pointer to frame out data
 *   num_faces - number of faces
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int faceproc_register_output(faceproc_comp_t *p_comp,
  faceproc_result_t *fd_data, INT32 num_faces)
{
  int i;
  for (i = 0; i < num_faces; i++) {
    fd_data->roi[i].unique_id = p_comp->user_id[i];
  }
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
static int faceproc_fd_execute(faceproc_comp_t *p_comp, img_frame_t *p_frame,
  INT32 * num_faces)
{
  ATRACE_BEGIN_SNPRINTF(32, "FD_%d", p_frame->frame_id);
  INT32 i;
  IDBG_MED("E %dx%d", FD_WIDTH(p_frame), FD_HEIGHT(p_frame));
  uint32_t processing_time, detection_time, facial_parts_time;
  bool profile_fd =
    (p_comp->debug_settings.profile_fd_level >= FD_PROFILE_LEVEL_FRAME) ?
    TRUE : FALSE;

  if (profile_fd == TRUE) {
    IMG_TIMER_START(p_comp->fd_profile.start_time[FD_FRAME_PROCESSING_TIME]);
    IMG_TIMER_START(p_comp->fd_profile.start_time[FD_FRAME_SW_DT_TIME]);
  }

  int rc;
  p_comp->frame_id = p_frame->frame_id;
#ifdef FD_WITH_DSP
  if (p_comp->p_lib->status_dsp_lib) {
    IDBG_MED("before FACEPROC_DetectionDSP, hDT: %p, pImage: %p, nWidth: %d,"
      "nHeight: %d, nAccuracy: %d, hResult: %p",
      p_comp->hdt,
      (RAWIMAGE *)FD_ADDR(p_frame),
      FD_WIDTH(p_frame),
      FD_HEIGHT(p_frame),
      ACCURACY_HIGH_TR,
      p_comp->hresult);
    rc = p_comp->p_lib->FACEPROC_DetectionDSP(p_comp->hdt,
      (RAWIMAGE *)FD_ADDR(p_frame),
      FD_WIDTH(p_frame)*FD_HEIGHT(p_frame),
      FD_WIDTH(p_frame),
      FD_HEIGHT(p_frame),
      ACCURACY_HIGH_TR,
      p_comp->hresult);
    IDBG_MED("after FACEPROC_DetectionDSP");
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_DetectionDSP returned error: %d", rc);
      *num_faces = 0;
      goto fd_ex_end;
    }
  } else {
    rc = p_comp->p_lib->FACEPROC_Detection(p_comp->hdt,
      (RAWIMAGE *)FD_ADDR(p_frame),
      FD_WIDTH(p_frame),
      FD_HEIGHT(p_frame),
      ACCURACY_HIGH_TR,
      p_comp->hresult);
  }
#else
  rc = p_comp->p_lib->FACEPROC_Detection(p_comp->hdt,
    (RAWIMAGE *)FD_ADDR(p_frame),
    (int32_t)FD_WIDTH(p_frame),
    (int32_t)FD_HEIGHT(p_frame),
    ACCURACY_HIGH_TR,
    p_comp->hresult);
#endif
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_Detection returned error: %d", rc);
    return IMG_ERR_GENERAL;
  }

  ATRACE_END();

  /*Set Position for PT */
  rc = p_comp->p_lib->FACEPROC_GetDtFaceCount(p_comp->hresult, num_faces);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_GetDtFaceCount returned error: %d", rc);
    *num_faces = 0;
    return IMG_ERR_GENERAL;
  }

  if (profile_fd == TRUE) {
    detection_time = IMG_TIMER_END(
      p_comp->fd_profile.start_time[FD_FRAME_SW_DT_TIME],
      p_comp->fd_profile.end_time[FD_FRAME_SW_DT_TIME],
      "FD_Detection", IMG_TIMER_MODE_MS);
    IMG_TIMER_START(p_comp->fd_profile.start_time[FD_FRAME_SW_FP_TIME]);
  }

  if (p_comp->dump_mode != FACE_FRAME_DUMP_OFF) {
    FACEINFO face_info;
    int index;
    int tracked = 0;

    for (index = 0; index < *num_faces; index++) {
      rc = p_comp->p_lib->FACEPROC_GetDtFaceInfo(p_comp->hresult, (int32_t)i,
        &face_info);
      if (rc != FACEPROC_NORMAL) {
        IDBG_ERROR("FACEPROC_GetDtFaceInfo returned error: %d", rc);
      }
      if (face_info.nID < 0) {
        tracked++;
      }
    }

    faceproc_common_frame_dump(p_frame, p_comp->dump_mode,
      tracked, *num_faces);
  }

  IDBG_MED("num faces %d, dsp %d", *num_faces, p_comp->p_lib->status_dsp_lib);
  if (*num_faces <= 0) {
    IDBG_MED("no faces detected X");
    rc = IMG_SUCCESS;
    goto end;
  }

  boolean enable_face_landmarks;
  boolean enable_contour, enable_smile, enable_blink, enable_gaze;

  faceproc_common_get_feature_flags(&p_comp->config, &p_comp->fd_chromatix,
    &enable_face_landmarks, &enable_contour,
    &enable_smile, &enable_blink, &enable_gaze);

  if (enable_face_landmarks == TRUE) {

    for (i = 0; ((i < *num_faces) && (i < MAX_FACE_ROI)); i++) {
#if(FACE_PART_DETECT)
      rc = p_comp->p_lib->FACEPROC_PT_SetPositionFromHandle(p_comp->hpt,
        p_comp->hresult, i);
      if (rc != FACEPROC_NORMAL) {
        IDBG_ERROR("eng_FACEPROC_PT_SetPositionFromHandle returned error:"
          "%d", rc);
        return IMG_ERR_GENERAL;
      }
      rc = p_comp->p_lib->FACEPROC_PT_DetectPoint(p_comp->hpt,
        (RAWIMAGE *)FD_ADDR(p_frame),
        (int32_t)FD_WIDTH(p_frame),
        (int32_t)FD_HEIGHT(p_frame),
        p_comp->hptresult[i]);
      if (rc != FACEPROC_NORMAL) {
        IDBG_ERROR("eng_FACEPROC_PT_DetectPoint returned error: %d", rc);
        return IMG_ERR_GENERAL;
      }
#endif
#if(FACE_CONTOUR_DETECT)
      if (enable_contour == TRUE) {
        rc = p_comp->p_lib->FACEPROC_CT_SetPointFromHandle(p_comp->hct,
          p_comp->hptresult[i]);
        if (rc != FACEPROC_NORMAL) {
          IDBG_ERROR("eng_FACEPROC_CT_SetPositionFromHandle error: %d", rc);
          return IMG_ERR_GENERAL;
        }
        rc = p_comp->p_lib->FACEPROC_CT_DetectContour(p_comp->hct,
          (RAWIMAGE *)FD_ADDR(p_frame),
          (int32_t)FD_WIDTH(p_frame),
          (int32_t)FD_HEIGHT(p_frame),
          p_comp->hctresult[i]);
        if (rc != FACEPROC_NORMAL) {
          IDBG_ERROR("eng_FACEPROC_PT_DetectPoint returned error: %d", rc);
          return IMG_ERR_GENERAL;
        }
      }
#endif
#if(FACE_BGS_DETECT)
      if (enable_smile == TRUE) {
        rc = p_comp->p_lib->FACEPROC_SM_SetPointFromHandle(p_comp->hsm,
          p_comp->hptresult[i]);
        if (rc != FACEPROC_NORMAL) {
          IDBG_ERROR("eng_FACEPROC_SM_SetPointFromHandle error: %d", rc);
          return IMG_ERR_GENERAL;
        }
        rc = p_comp->p_lib->FACEPROC_SM_Estimate(p_comp->hsm,
          (RAWIMAGE *)FD_ADDR(p_frame),
          (int32_t)FD_WIDTH(p_frame),
          (int32_t)FD_HEIGHT(p_frame),
          p_comp->hsmresult[i]);
        if (rc != FACEPROC_NORMAL) {
          IDBG_ERROR("eng_FACEPROC_SM_Estimate returned error: %d", rc);
          return IMG_ERR_GENERAL;
        }
      }

      if ((enable_blink == TRUE) || (enable_gaze == TRUE)) {
        /* Blink/Gaze Detection */
        rc = p_comp->p_lib->FACEPROC_GB_SetPointFromHandle(p_comp->hgb,
          p_comp->hptresult[i]);
        if (rc != FACEPROC_NORMAL) {
          IDBG_ERROR("eng_FACEPROC_GB_SetPointFromHandle error: %d", rc);
          return IMG_ERR_GENERAL;
        }
        rc = p_comp->p_lib->FACEPROC_GB_Estimate(p_comp->hgb,
          (RAWIMAGE *)FD_ADDR(p_frame),
          (int32_t)FD_WIDTH(p_frame),
          (int32_t)FD_HEIGHT(p_frame),
          p_comp->hgbresult[i]);
        if (rc != FACEPROC_NORMAL) {
          IDBG_ERROR("eng_FACEPROC_GB_Estimate returned error: %d", rc);
          return IMG_ERR_GENERAL;
        }
      }
#endif

    }
  }  /* end of forloop */

end:

  if (profile_fd == TRUE) {
    processing_time = IMG_TIMER_END(
      p_comp->fd_profile.start_time[FD_FRAME_PROCESSING_TIME],
      p_comp->fd_profile.end_time[FD_FRAME_PROCESSING_TIME],
      "FD_FrameProcess", IMG_TIMER_MODE_MS);
    facial_parts_time = IMG_TIMER_END(
      p_comp->fd_profile.start_time[FD_FRAME_SW_FP_TIME],
      p_comp->fd_profile.end_time[FD_FRAME_SW_FP_TIME],
      "FD_FacialParts", IMG_TIMER_MODE_MS);

    p_comp->fd_profile.total_time_in_ms[FD_FRAME_SW_DT_TIME] +=
      detection_time;
    p_comp->fd_profile.total_time_in_ms[FD_FRAME_PROCESSING_TIME] +=
      processing_time;
    p_comp->fd_profile.total_time_in_ms[FD_FRAME_SW_FP_TIME] +=
      facial_parts_time;

    p_comp->fd_profile.num_frames_processed++;

    IDBG_INFO("FDPROFILE : Frame[%d] FrameCounter[%d] %dx%d :\t faces=%d "
      "Time=%u (DT=%u FP=%u),\t Avg=%u (DT=%u, FP=%u)",
      p_frame->frame_id, p_comp->fd_profile.num_frames_processed,
      (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
      *num_faces,
      processing_time, detection_time, facial_parts_time,
      p_comp->fd_profile.total_time_in_ms[FD_FRAME_PROCESSING_TIME] /
      p_comp->fd_profile.num_frames_processed,
      p_comp->fd_profile.total_time_in_ms[FD_FRAME_SW_DT_TIME] /
      p_comp->fd_profile.num_frames_processed,
      p_comp->fd_profile.total_time_in_ms[FD_FRAME_SW_FP_TIME] /
      p_comp->fd_profile.num_frames_processed);
  }

  IDBG_MED("X");

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_fr_execute
 *
 * Description: Executes the face recognition algorithm
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
static int faceproc_fr_execute(faceproc_comp_t *p_comp,
  img_frame_t *p_frame, int num_faces)
{
  int i, rc;
  INT32 an_user_id[MAX_REGISTER_PPL], pn_user_num;
  INT32 an_score[MAX_REGISTER_PPL], pn_result_num;

  if (num_faces <= 0)
    return IMG_SUCCESS;

  /* FR START */
  for (i = 0; i < num_faces; i++) {
    p_comp->user_id[i] = -1;
    p_comp->confidence[i] = 0;
#if(FACE_RECOGNITION)
    rc = p_comp->p_lib->FACEPROC_FR_ExtractFeatureFromPtHdl(p_comp->hfeature,
      (RAWIMAGE *)FD_ADDR(p_frame),
      (int32_t)FD_WIDTH(p_frame),
      (int32_t)FD_HEIGHT(p_frame),
      p_comp->hptresult[i]);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s eng_FACEPROC_FR_ExtractFeatureFromPtHdl returned error: %d",
        __func__, (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
#if(FACE_RECOGNIZE_TEST_REGISTER)
    /*TODO move recognie to different fn */
    rc = p_comp->p_lib->FACEPROC_FR_RegisterData(p_comp->halbum,
      p_comp->hfeature, i, 0);
    return IMG_SUCCESS;
#endif

    /* Find if album is empty */
    rc = p_comp->p_lib->FACEPROC_FR_GetRegisteredUserNum(p_comp->halbum,
      &pn_user_num);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s eng_FACEPROC_FR_GetRegisteredUsrNum returned error: %d",
        __func__, (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
    p_comp->last_img_registered_idx = pn_user_num;
    if (!pn_user_num) {
      IDBG_ERROR("%s Face Recognition database/Album is empty",  __func__);
      return IMG_ERR_NOT_FOUND;
    } else {
      IDBG_MED("Number of User entries in album is %d", pn_user_num);
    }

    if (!p_comp->hfeature ||!p_comp->halbum) {
      IDBG_ERROR("%s hfeature and halbum is null, failed",  __func__);
    }

    IDBG_MED("Executing Face Recognition");
    rc = p_comp->p_lib->FACEPROC_FR_Identify(p_comp->hfeature,
      p_comp->halbum, MAX_REGISTER_PPL,
      an_user_id, an_score, &pn_result_num);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("eng_FACEPROC_FR_Identify returned error: %d", (uint32_t)rc);
      return IMG_ERR_GENERAL;
    }
    IDBG_MED("%s USER ID %d  score %d", __func__, an_user_id[0], an_score[0]);
    if (pn_result_num <= MAX_REGISTER_PPL && (an_score[0] >= FR_THRESHOLD)) {
      p_comp->recognized[i] = 1;
      p_comp->user_id[i] = an_user_id[0];
      p_comp->confidence[i] = an_score[0];
    } else {
      p_comp->recognized[i] = 0;
      p_comp->user_id[i] = -1;
      p_comp->confidence[i] = 0;
    }
#endif
    /*FR END */
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
  if (!p_comp->hdt) {
    // If this event has come even before we create the fd lib handles,
    // we can ignore this. We will anyway set config with the latest
    // information that we have when creating handles
    return IMG_SUCCESS;
  }

  uint32_t min_face_size = faceproc_common_get_face_size(
    p_comp->fd_chromatix.min_face_adj_type,
    p_comp->fd_chromatix.min_face_size,
    p_comp->fd_chromatix.min_face_size_ratio,
    FACEPROC_MIN_FACE_SIZE, /* restricting min facesize */
    MIN(p_comp->width, p_comp->height));
  IDBG_MED("new ###min_face_size %d", min_face_size);

  /* Set the max and min face size for detection */
  int rc = (int) p_comp->p_lib->FACEPROC_SetDtFaceSizeRange(
    p_comp->hdt, (int32_t)min_face_size,
    (int32_t)p_comp->fd_chromatix.max_face_size);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtFaceSizeRange failed %d", rc);
    return IMG_ERR_GENERAL;
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
  IDBG_LOW("%s] Enter: %p", __func__, (void *)p_comp);

  UINT32 an_still_angle[POSE_TYPE_COUNT];
  UINT8 *buffer;
  FILE *fp;
  FR_ERROR pError;
  int i;
  UINT32 bmem_size_required = 0, wmem_size_required_min = 0,
    wmem_size_required_max;
  RECT nil_edge = {-1, -1, -1, -1};
  int rc = IMG_SUCCESS;
  faceproc_config_t *p_cfg = &p_comp->config;
  uint32_t max_num_face_to_detect;
  uint32_t min_face_size;
  uint8 fd_minor_version, fd_major_version;
  int lib_status_rc;


  if (!p_comp) {
    IDBG_ERROR("%s:%d] NULL component", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  /* Check if FD DSP stub library is requested and valid */
  if (p_comp->p_lib->load_dsp_lib && p_comp->p_lib->status_dsp_lib) {
    lib_status_rc = p_comp->p_lib->FACEPROC_Dt_VersionDSP
      (&fd_minor_version, &fd_major_version);
    IDBG_HIGH("%s], Checking if DSP stub is valid - rc: %d",
      __func__, lib_status_rc);
    if (lib_status_rc < IMG_SUCCESS) {  /*Is DSP stub lib invalid */
      IDBG_HIGH("%s], FD DSP lib error possibly due to prior ADSP crash:"
        "Attemping to reload FD lib", __func__);
      /* Load FD Dt ARM library */
      lib_status_rc = faceproc_comp_eng_load_dt_arm(p_comp->p_lib);
      if (lib_status_rc < IMG_SUCCESS)
        return IMG_ERR_GENERAL;
      IDBG_HIGH("%s], FD ARM lib loaded: rc: %d", __func__, lib_status_rc);
    }
  }

  min_face_size = faceproc_common_get_face_size(
    p_comp->fd_chromatix.min_face_adj_type,
    p_comp->fd_chromatix.min_face_size,
    p_comp->fd_chromatix.min_face_size_ratio,
    FACEPROC_MIN_FACE_SIZE, /* restricting min facesize */
    MIN(p_cfg->frame_cfg.max_width, p_cfg->frame_cfg.max_height));

  IDBG_MED("%s:%d] ###min_face_size %d", __func__, __LINE__,
    min_face_size);

  max_num_face_to_detect = faceproc_get_max_number_to_detect(&p_comp->fd_chromatix);

  IDBG_MED("%s], Enter: p_comp: [%p, %p]load_dsp: %d, max_faces: %d",
    __func__,    p_comp, (void *)&p_comp,
    p_comp->p_lib->status_dsp_lib, max_num_face_to_detect);

  if (FD_ANGLE_ENABLE(&p_comp->config)) {
    an_still_angle[POSE_FRONT] = faceproc_common_get_angle(
      p_comp->fd_chromatix.angle_front, false, 0, -1);
    an_still_angle[POSE_HALF_PROFILE] = faceproc_common_get_angle(
      p_comp->fd_chromatix.angle_half_profile, false, 0, -1);
    an_still_angle[POSE_PROFILE] = faceproc_common_get_angle(
      p_comp->fd_chromatix.angle_full_profile, false, 0, -1);
  } else {
    IDBG_MED("%s:%d] ###Disable Angle", __func__, __LINE__);
    an_still_angle[POSE_FRONT] = ANGLE_NONE;
    an_still_angle[POSE_HALF_PROFILE] = ANGLE_NONE;
    an_still_angle[POSE_PROFILE] = ANGLE_NONE;
  }

#ifndef FD_WITH_DSP
  /* Determine the size of memory needed */
  uint32_t nontracking_dens = faceproc_common_validate_and_get_density(
    false, p_comp->fd_chromatix.search_density_nontracking, 0);
  uint32_t tracking_dens = faceproc_common_validate_and_get_density(
    false, p_comp->fd_chromatix.search_density_tracking, 0);

  rc = p_comp->p_lib->FACEPROC_GetDtRequiredMovieMemSize(
    (int32_t)p_cfg->frame_cfg.max_width,
    (int32_t)p_cfg->frame_cfg.max_height,
    (int32_t)min_face_size,
    (int32_t)max_num_face_to_detect +
    (int32_t)max_num_face_to_detect / 2,
    nil_edge, (int32_t)nontracking_dens,
    nil_edge, (int32_t)tracking_dens,
    &bmem_size_required,
    &wmem_size_required_min,
    &wmem_size_required_max);

#ifdef FD_USE_INTERNAL_MEM
  /* Allocate scratch memory for Faceproc FD */
  IDBG_MED("GetDtRequiredMovieMemSiz bmem %d wmem %d rc=%d",
    bmem_size_required, wmem_size_required_max, rc);
  /* Pt API */
  bmem_size_required += (3 * KB_SIZE);
  wmem_size_required_max += (110 * KB_SIZE);
  /* Face Contour API */
  bmem_size_required += (3 * KB_SIZE);
  wmem_size_required_max += (250 * KB_SIZE);
  /* Smile Detect API */
  bmem_size_required += (3 * KB_SIZE);
  wmem_size_required_max += (45 * KB_SIZE);
  /* Face Recognition API */
  bmem_size_required += MAX_ALBUM_SIZE;
  wmem_size_required_max += (150 * KB_SIZE);
  IDBG_MED("After GetDtRequiredMovieMemSiz bmem %d wmem %d rc = %d",
    bmem_size_required, wmem_size_required_max, rc);

  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  p_comp->bmem = malloc(bmem_size_required);
  if (NULL == p_comp->bmem) {
    IDBG_ERROR("%s:%d] bmem failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }
  p_comp->wmem = malloc(wmem_size_required_max);
  if (NULL == p_comp->bmem) {
    IDBG_ERROR("%s:%d] wmem failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  IDBG_MED("GetDtRequiredMovieMemSiz1 value rc=%d", rc);

  /* Pass the scratch memory to Faceproc's FD */

  rc = (int) p_comp->p_lib->FACEPROC_SetWMemoryArea(p_comp->wmem,
    wmem_size_required_max);
  IDBG_MED("SetWMemoryArea rc=%d", rc);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s:%d] SetWMemoryArea failed %d", __func__, __LINE__, rc);
    return IMG_ERR_NO_MEMORY;
  }
  p_comp->fdWmemorySet = TRUE;
  rc = (int) p_comp->p_lib->FACEPROC_SetBMemoryArea(p_comp->bmem,
    bmem_size_required);

  IDBG_MED("SetBMemoryArea rc=%d", rc);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s:%d] SetBMemoryArea failed %d", __func__, __LINE__, rc);
    return IMG_ERR_NO_MEMORY;
  }
  p_comp->fdBmemorySet = TRUE;
#endif
#endif

  bool profile_fd =
      (p_comp->debug_settings.profile_fd_level >= FD_PROFILE_LEVEL_ALL) ?
      TRUE : FALSE;

  memset(&p_comp->fd_profile, 0x0, sizeof(img_profiling_t));
  if (profile_fd == TRUE) {
    IMG_TIMER_START(p_comp->fd_profile.start_time[FD_HANDLE_CREATE_TIME]);
  }


  IDBG_MED("%s], Before calling FACEPROC_CreateDetection: p_comp: %p",
    __func__, p_comp);

  UINT8 major, minor;

  p_comp->p_lib->FACEPROC_GetDtVersion(&major, &minor);
  IDBG_HIGH("DT version : %d.%d", major, minor);

  p_comp->hdt = p_comp->p_lib->FACEPROC_CreateDetection();

  if (!p_comp->hdt) {
    IDBG_ERROR("%s FACEPROC_CreateDetection failed",  __func__);
    return IMG_ERR_GENERAL;
  }
  IDBG_MED("After calling FACEPROC_CreateDetection");
  /* Set best Faceproc detection mode for video */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtMode(
    p_comp->hdt, (int32_t)p_comp->fd_chromatix.detection_mode);


  IDBG_MED("After calling FACEPROC_SetDtMode");
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtMode failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }

  /* Set search density */
  nontracking_dens = faceproc_common_validate_and_get_density(
    false, p_comp->fd_chromatix.search_density_nontracking, 0);
  tracking_dens = faceproc_common_validate_and_get_density(
    false, p_comp->fd_chromatix.search_density_tracking, 0);

  rc = (int) p_comp->p_lib->FACEPROC_SetDtStep(
    p_comp->hdt, (int32_t)nontracking_dens, (int32_t)tracking_dens);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtStep failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }

  /* Set Detection Angles */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtAngle(
    p_comp->hdt, an_still_angle,
    ANGLE_ROTATION_EXT0 | ANGLE_POSE_EXT0);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtAngle failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }

  /* Set refresh count */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtRefreshCount(
    p_comp->hdt, (int32_t)p_comp->fd_chromatix.detection_mode,
    (int32_t)p_comp->fd_chromatix.refresh_count);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtRefreshCount failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }

  rc = (int) p_comp->p_lib->FACEPROC_SetDtDirectionMask(
    p_comp->hdt, (int32_t)p_comp->fd_chromatix.direction);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtDirectionMask failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }

  /* Minimum face size to be detected should be at most half the
    height of the input frame */
  if (min_face_size > (p_cfg->frame_cfg.max_height/2)) {
    IDBG_ERROR("%s:%d] Error, min face size to detect is greater than "
      "half the height of the input frame", __func__, __LINE__);
    return IMG_SUCCESS;
  }

  /* Set the max and min face size for detection */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtFaceSizeRange(
    p_comp->hdt, (int32_t)min_face_size,
    (int32_t)p_comp->fd_chromatix.max_face_size);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtFaceSizeRange failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }
  /* Set Detection Threshold */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtThreshold(
    p_comp->hdt, (int32_t)p_comp->fd_chromatix.threshold,
    (int32_t)p_comp->fd_chromatix.threshold);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtFaceSizeRange failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }

  /* FD Configuration logging */
  IDBG_MED("fd_arm_config: Parts: (%d, %d), Contour: %d, BGS: %d, Recog: %d",
    FD_FACEPT_ENABLE(&p_comp->config), FACE_PART_DETECT, FACE_CONTOUR_DETECT,
    FACE_BGS_DETECT, FACE_RECOGNITION);
  IDBG_MED("fd_arm_config: MAX # of faces: %d", max_num_face_to_detect);
  IDBG_MED("fd_arm_config: MIN, MAX face size: %d, %d",
    min_face_size, p_comp->fd_chromatix.max_face_size);
  IDBG_MED("fd_arm_config: DT_mode: %d, Refresh_count: %d",
    p_comp->fd_chromatix.detection_mode, p_comp->fd_chromatix.refresh_count);
  IDBG_MED("fd_arm_config: Search Density: %d",
    p_comp->fd_chromatix.search_density_tracking);
  IDBG_MED("fd_arm_config: Detection Threshold: %d",
    p_comp->fd_chromatix.threshold);
  IDBG_MED("fd_arm_config: Angles: %d, %d, %d, Track: %d",
    an_still_angle[POSE_FRONT], an_still_angle[POSE_HALF_PROFILE],
    an_still_angle[POSE_PROFILE], (ANGLE_ROTATION_EXT0 | ANGLE_POSE_EXT0));

  /* Create Faceproc result handle */
  p_comp->hresult = p_comp->p_lib->FACEPROC_CreateDtResult(
    (int32_t)max_num_face_to_detect ,
    (int32_t)(max_num_face_to_detect/2));
  if (!(p_comp->hresult)) {
    IDBG_ERROR("%s FACEPROC_CreateDtResult failed",  __func__);
    return IMG_ERR_GENERAL;
  }
  /* Create PT handle */
  if (FD_FACEPT_ENABLE(&p_comp->config)) {
#if(FACE_PART_DETECT)
    p_comp->hpt = p_comp->p_lib->FACEPROC_PT_CreateHandle();
    if (!(p_comp->hpt)) {
      IDBG_ERROR("%s eng_FACEPROC_PT_CreateHandle failed",  __func__);
      return IMG_ERR_GENERAL;
    }
    for (i = 0; i< MAX_FACE_ROI; i++) {
      p_comp->hptresult[i] = p_comp->p_lib->FACEPROC_PT_CreateResultHandle();
      if (!(p_comp->hptresult[i])) {
        IDBG_ERROR("%s eng_FACEPROC_PT_CreateResultHandle failed",  __func__);
        return IMG_ERR_GENERAL;
      }
    }
    rc = (int) p_comp->p_lib->FACEPROC_PT_SetMode(p_comp->hpt,
      PT_MODE_DEFAULT);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s eng_FACEPROC_PT_SetMode failed",  __func__);
      return IMG_ERR_GENERAL;
    }
    rc = (int)p_comp->p_lib->FACEPROC_PT_SetConfMode(p_comp->hpt,
      PT_CONF_USE);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s eng_FACEPROC_PT_SetConfMode failed",  __func__);
      return IMG_ERR_GENERAL;
    }
#endif
  /* Config CT API */
#if(FACE_CONTOUR_DETECT)
    p_comp->hct = p_comp->p_lib->FACEPROC_CT_CreateHandle();
    if (!(p_comp->hct)) {
      IDBG_ERROR("%s eng_FACEPROC_CT_CreateHandle failed",  __func__);
      return IMG_ERR_GENERAL;
    }
    for (i = 0; i< MAX_FACE_ROI; i++) {
      p_comp->hctresult[i] = p_comp->p_lib->FACEPROC_CT_CreateResultHandle();
      if (!(p_comp->hctresult[i])) {
        IDBG_ERROR("%s eng_FACEPROC_CT_CreateResultHandle failed",  __func__);
        return IMG_ERR_GENERAL;
      }
    }
    rc = (int) p_comp->p_lib->FACEPROC_CT_SetDetectionMode(p_comp->hct,
      p_comp->fd_chromatix.ct_detection_mode);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s eng_FACEPROC_CT_SetDetectionMode failed",  __func__);
      return IMG_ERR_GENERAL;
    }
#endif
#if(FACE_BGS_DETECT)
    p_comp->hsm = p_comp->p_lib->FACEPROC_SM_CreateHandle();
    if (!(p_comp->hsm)) {
      IDBG_ERROR("%s eng_FACEPROC_SM_CreateHandle failed",  __func__);
      return IMG_ERR_GENERAL;
    }
    for (i = 0; i < MAX_FACE_ROI; i++) {
      p_comp->hsmresult[i] = p_comp->p_lib->FACEPROC_SM_CreateResultHandle();
      if (!(p_comp->hsmresult[i])) {
        IDBG_ERROR("%s eng_FACEPROC_SM_CreateResultHandle failed",  __func__);
        return IMG_ERR_GENERAL;
      }
    }
#endif
  /* Face Recognition */
#if(FACE_RECOGNITION)
    p_comp->hfeature = p_comp->p_lib->FACEPROC_FR_CreateFeatureHandle();
    if (!(p_comp->hfeature)) {
      IDBG_ERROR("%s eng_FACEPROC_FR_CreateFeatureHandle failed",  __func__);
      return IMG_ERR_GENERAL;
    }
    fp = fopen(FACE_ALBUM, "r");
    if (fp == NULL) {
      IDBG_MED("%s Face Album FILE DOESNT EXIST",  __func__);
      p_comp->halbum = p_comp->p_lib->FACEPROC_FR_CreateAlbumHandle(
        MAX_REGISTER_PPL,
        MAX_REGISTER_IMG_PER_PERSON);
    } else {
      IDBG_MED("%s Face Album FILE EXISTS",  __func__);
      buffer = (uint8_t *)malloc(MAX_ALBUM_SIZE);
      if (buffer != NULL) {
        fread(buffer, 1, MAX_ALBUM_SIZE, fp);
      } else {
        IDBG_ERROR("%s%d]: buffer is null", __func__, __LINE__);
      }
      p_comp->halbum = p_comp->p_lib->FACEPROC_FR_RestoreAlbum((UINT8 *)buffer,
        MAX_ALBUM_SIZE, &pError);
      if (p_comp->halbum == NULL) {
        IDBG_ERROR("%s Fd album is Corrupted %d, hence recreating new one",
          __func__, pError);
        p_comp->halbum = p_comp->p_lib->FACEPROC_FR_CreateAlbumHandle(
          MAX_REGISTER_PPL,
          MAX_REGISTER_IMG_PER_PERSON);
      }
      if (buffer) {
        free(buffer);
      }
      fclose(fp);
    }
    if (!(p_comp->halbum)) {
      IDBG_ERROR("%s:%d] eng_FACEPROC_FR_CreateAlbumHandle failed",
        __func__, __LINE__);
    }
    p_comp->last_img_registered_idx = 0;
#endif

  /* Blink Detection */
#if(FACE_BGS_DETECT)
    p_comp->hgb = p_comp->p_lib->FACEPROC_GB_CreateHandle();
    if (!(p_comp->hgb)) {
      IDBG_ERROR("%s eng_FACEPROC_GB_CreateHandle failed",  __func__);
      return IMG_ERR_GENERAL;
    }
    for (i = 0; i < MAX_FACE_ROI; i++) {
      p_comp->hgbresult[i] = p_comp->p_lib->FACEPROC_GB_CreateResultHandle();
      if (!(p_comp->hgbresult[i])) {
        IDBG_ERROR("%s eng_FACEPROC_GB_CreateResultHandle failed",  __func__);
        return IMG_ERR_GENERAL;
      }
    }
#endif
  }

  if (profile_fd == TRUE) {
    p_comp->fd_profile.total_time_in_ms[FD_HANDLE_CREATE_TIME] =
      IMG_TIMER_END(p_comp->fd_profile.start_time[FD_HANDLE_CREATE_TIME],
      p_comp->fd_profile.end_time[FD_HANDLE_CREATE_TIME],
      "FD CreateHandles", IMG_TIMER_MODE_MS);

    IDBG_INFO("FDPROFILE : SWLibraryCreateHandles + InitialConfig time = %u",
      p_comp->fd_profile.total_time_in_ms[FD_HANDLE_CREATE_TIME]);
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_eng_reconfig_core
 *
 * Description: Re-Configure the faceproc engine
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
int faceproc_comp_eng_reconfig_core(faceproc_comp_t *p_comp)
{
  IDBG_MED("%s] Enter: %p", __func__, (void *)p_comp);

  UINT32 an_still_angle[POSE_TYPE_COUNT];
  int rc = IMG_SUCCESS;
  faceproc_config_t *p_cfg = &p_comp->config;
  uint32_t max_num_face_to_detect;
  uint32_t min_face_size;

  if (!p_comp) {
    IDBG_ERROR("%s:%d] NULL component", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  min_face_size = faceproc_common_get_face_size(
    p_comp->fd_chromatix.min_face_adj_type,
    p_comp->fd_chromatix.min_face_size,
    p_comp->fd_chromatix.min_face_size_ratio,
    FACEPROC_MIN_FACE_SIZE, /* restricting min facesize */
    MIN(p_cfg->frame_cfg.max_width, p_cfg->frame_cfg.max_height));

  IDBG_MED("%s:%d] ###min_face_size %d", __func__, __LINE__,
    min_face_size);

  max_num_face_to_detect = faceproc_get_max_number_to_detect(&p_comp->fd_chromatix);

  IDBG_MED("%s], Enter: p_comp: [%p, %p]load_dsp: %d, max_faces: %d",
    __func__, p_comp, (void *)&p_comp, p_comp->p_lib->status_dsp_lib,
    max_num_face_to_detect);

  if (FD_ANGLE_ENABLE(&p_comp->config)) {
    an_still_angle[POSE_FRONT] = faceproc_common_get_angle(
      p_comp->fd_chromatix.angle_front, false, 0, -1);
    an_still_angle[POSE_HALF_PROFILE] = faceproc_common_get_angle(
      p_comp->fd_chromatix.angle_half_profile, false, 0, -1);
    an_still_angle[POSE_PROFILE] = faceproc_common_get_angle(
      p_comp->fd_chromatix.angle_full_profile, false, 0, -1);
  } else {
    IDBG_MED("%s:%d] ###Disable Angle", __func__, __LINE__);
    an_still_angle[POSE_FRONT] = ANGLE_NONE;
    an_still_angle[POSE_HALF_PROFILE] = ANGLE_NONE;
    an_still_angle[POSE_PROFILE] = ANGLE_NONE;
  }

  /* Set best Faceproc detection mode for video */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtMode(
    p_comp->hdt, (int32_t)p_comp->fd_chromatix.detection_mode);


  IDBG_MED("After calling FACEPROC_SetDtMode");
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtMode failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }

  /* Set search density */
  uint32_t nontracking_dens = faceproc_common_validate_and_get_density(
    false, p_comp->fd_chromatix.search_density_nontracking, 0);
  uint32_t tracking_dens = faceproc_common_validate_and_get_density(
    false, p_comp->fd_chromatix.search_density_tracking, 0);

  rc = (int) p_comp->p_lib->FACEPROC_SetDtStep(
    p_comp->hdt, (int32_t)nontracking_dens, (int32_t)tracking_dens);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtStep failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }

  /* Set Detection Angles */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtAngle(
    p_comp->hdt, an_still_angle,
    ANGLE_ROTATION_EXT0 | ANGLE_POSE_EXT0);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtAngle failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }

  /* Set refresh count */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtRefreshCount(
    p_comp->hdt, (int32_t)p_comp->fd_chromatix.detection_mode,
    (int32_t)p_comp->fd_chromatix.refresh_count);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtRefreshCount failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }

  rc = (int) p_comp->p_lib->FACEPROC_SetDtDirectionMask(
    p_comp->hdt, (BOOL)p_comp->fd_chromatix.direction);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtDirectionMask failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }

  /* Minimum face size to be detected should be at most half the
    height of the input frame */
  if (min_face_size > (p_cfg->frame_cfg.max_height/2)) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  /* Set the max and min face size for detection */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtFaceSizeRange(
    p_comp->hdt, (int32_t)min_face_size,
    (int32_t)p_comp->fd_chromatix.max_face_size);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtFaceSizeRange failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }
  /* Set Detection Threshold */
  rc = (int) p_comp->p_lib->FACEPROC_SetDtThreshold(
    p_comp->hdt, (int32_t)p_comp->fd_chromatix.threshold,
    (int32_t)p_comp->fd_chromatix.threshold);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("%s FACEPROC_SetDtFaceSizeRange failed %d",  __func__, rc);
    return IMG_ERR_GENERAL;
  }

  /* FD Configuration logging */
  IDBG_MED("fd_arm_config: Parts: (%d, %d), Contour: %d, BGS: %d, Recog: %d",
    FD_FACEPT_ENABLE(&p_comp->config), FACE_PART_DETECT, FACE_CONTOUR_DETECT,
    FACE_BGS_DETECT, FACE_RECOGNITION);
  IDBG_MED("fd_arm_config: MAX # of faces: %d", max_num_face_to_detect);
  IDBG_MED("fd_arm_config: MIN, MAX face size: %d, %d",
    min_face_size, p_comp->fd_chromatix.max_face_size);
  IDBG_MED("fd_arm_config: DT_mode: %d, Refresh_count: %d",
    p_comp->fd_chromatix.detection_mode, p_comp->fd_chromatix.refresh_count);
  IDBG_MED("fd_arm_config: Search Density: %d",
    p_comp->fd_chromatix.search_density_tracking);
  IDBG_MED("fd_arm_config: Detection Threshold: %d",
    p_comp->fd_chromatix.threshold);
  IDBG_MED("fd_arm_config: Angles: %d, %d, %d, Track: %d",
    an_still_angle[POSE_FRONT], an_still_angle[POSE_HALF_PROFILE],
    an_still_angle[POSE_PROFILE], (ANGLE_ROTATION_EXT0 | ANGLE_POSE_EXT0));

  /* Delete Old Result handle */
  if (p_comp->hresult) {
    rc = p_comp->p_lib->FACEPROC_DeleteDtResult(p_comp->hresult);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("%s FACEPROC_DeleteDtResult failed",  __func__);
      return IMG_ERR_GENERAL;
    }
    p_comp->hresult = NULL;
  }
  /* Create Faceproc result handle */
  p_comp->hresult = p_comp->p_lib->FACEPROC_CreateDtResult(
    (int32_t)max_num_face_to_detect ,
    (int32_t)(max_num_face_to_detect/2));
  if (!(p_comp->hresult)) {
    IDBG_ERROR("%s FACEPROC_CreateDtResult failed",  __func__);
    return IMG_ERR_GENERAL;
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
  INT32 num_faces;
  int rc;
  switch (p_comp->mode) {
    case FACE_DETECT_BSGC:
    case FACE_DETECT:
    case FACE_DETECT_LITE:
      if (p_comp->is_chromatix_changed == TRUE) {
        p_comp->is_chromatix_changed = FALSE;
        rc = faceproc_comp_eng_reconfig_core(p_comp);
        if (rc != IMG_SUCCESS)
          return IMG_ERR_GENERAL;
      }

      rc = faceproc_fd_execute(p_comp, p_frame, &num_faces);
      if (rc != IMG_SUCCESS)
        return IMG_ERR_GENERAL;
      break;
    case FACE_RECOGNIZE:
      rc = faceproc_fd_execute(p_comp, p_frame, &num_faces);
      if (rc != IMG_SUCCESS)
        return IMG_ERR_GENERAL;
      rc = faceproc_fr_execute(p_comp, p_frame, num_faces);
      if (rc != IMG_SUCCESS)
        return IMG_ERR_GENERAL;
      break;
    case FACE_REGISTER:
      rc = faceproc_fd_execute(p_comp, p_frame, &num_faces);
      if (rc != IMG_SUCCESS)
        return IMG_ERR_GENERAL;
      rc = faceproc_fr_execute(p_comp, p_frame, num_faces);
      if (rc == IMG_ERR_GENERAL)
        return IMG_ERR_GENERAL;
      rc = faceproc_register_frame(p_comp, p_frame, num_faces);
      if (rc != IMG_SUCCESS)
        return IMG_ERR_GENERAL;
      break;
    default :
      IDBG_ERROR("%s MODE not selected/recognized", __func__);
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_eng_get_output
 *
 * Description: Get the output from the frameproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   fd_data - Input frame
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_get_output(faceproc_comp_t *p_comp,
  faceproc_result_t *fd_data)
{
  INT32 num_faces;
  int status = IMG_SUCCESS;
  IDBG_LOW("%s, p_comp:%p, p_res:%p",  __func__, p_comp, fd_data);
  switch (p_comp->mode) {
  case FACE_DETECT_BSGC:
  case FACE_DETECT:
  case FACE_DETECT_LITE:
    status = faceproc_fd_output(p_comp, fd_data, &num_faces);
    break;
  case FACE_RECOGNIZE:
    status = faceproc_fd_output(p_comp, fd_data, &num_faces);
    if (IMG_SUCCEEDED(status))
      status = faceproc_fr_output(p_comp, fd_data, num_faces);
    break;
  case FACE_REGISTER:
    status = faceproc_fd_output(p_comp, fd_data, &num_faces);
    if (IMG_SUCCEEDED(status))
      status = faceproc_register_output(p_comp, fd_data, num_faces);
    break;
  default:
    IDBG_ERROR("%s: Unsupported mode selected", __func__);
    return IMG_ERR_INVALID_INPUT;
  }
  IDBG_LOW("%s, After rc: %d, p_comp:%p, p_res:%p",  __func__,
    status, p_comp, fd_data);
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

#if(FACE_RECOGNITION)
  /* Save album */
  rc = faceproc_save_album(p_comp);
  if (rc != IMG_SUCCESS)
    IDBG_MED("%s: Album save failed %d", __func__, rc);
#endif

  IDBG_MED("%s:%d] faceproc engine clean", __func__, __LINE__);
  rc = faceproc_comp_eng_reset(p_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s: faceproc_comp_eng_reset failed %d", __func__, rc);
  }
  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_eng_reset
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
int faceproc_comp_eng_reset(faceproc_comp_t *p_comp)
{
  IDBG_MED("%s:%d]", __func__, __LINE__);
  INT32 ret, i;
  uint32_t delta;

  if (!p_comp)
    return IMG_ERR_GENERAL;

  bool profile_fd =
    (p_comp->debug_settings.profile_fd_level >= FD_PROFILE_LEVEL_ALL) ?
    TRUE : FALSE;

  if (profile_fd == TRUE) {
    delta =
      p_comp->fd_profile.total_time_in_ms[FD_FRAME_PROCESSING_TIME];

    if (delta && p_comp->fd_profile.num_frames_processed) {
      IDBG_INFO("FDPROFILE : TotalFrames=%u, TotalTimeTaken=%u, "
        "AvgSWPrcessingTime=%f (FPS=%d)",
        p_comp->fd_profile.num_frames_processed, delta,
        (float)delta/(float)p_comp->fd_profile.num_frames_processed,
        (p_comp->fd_profile.num_frames_processed * 1000) / delta);
    }
    IMG_TIMER_START(p_comp->fd_profile.start_time[FD_HANDLE_DELETE_TIME]);
  }

  /* Delete Result handle */
  if (p_comp->hresult) {
    ret = p_comp->p_lib->FACEPROC_DeleteDtResult(p_comp->hresult);
    if (ret != FACEPROC_NORMAL)
      return IMG_ERR_GENERAL;
    p_comp->hresult = NULL;
  }
  /* Delete Handle */
  if (p_comp->hdt) {
    ret = p_comp->p_lib->FACEPROC_DeleteDetection(p_comp->hdt);
    if (ret != FACEPROC_NORMAL)
      return IMG_ERR_GENERAL;
    p_comp->hdt = NULL;
  }

  if (FD_FACEPT_ENABLE(&p_comp->config)) {
#if(FACE_PART_DETECT)
    if (p_comp->hpt) {
      ret = p_comp->p_lib->FACEPROC_PT_DeleteHandle(p_comp->hpt);
      if (ret != FACEPROC_NORMAL)
        return IMG_ERR_GENERAL;
      p_comp->hpt = NULL;
    }
    for (i = 0; i < MAX_FACE_ROI; i++) {
      if (p_comp->hptresult[i]) {
        ret = p_comp->p_lib->FACEPROC_PT_DeleteResultHandle(
          p_comp->hptresult[i]);
        if (ret != FACEPROC_NORMAL)
          return IMG_ERR_GENERAL;
        p_comp->hptresult[i] = NULL;
      }
    }
#endif

#if(FACE_CONTOUR_DETECT)
    if (p_comp->hct) {
      ret = p_comp->p_lib->FACEPROC_CT_DeleteHandle(p_comp->hct);
      if (ret != FACEPROC_NORMAL)
        return IMG_ERR_GENERAL;
      p_comp->hct = NULL;
    }
    for (i = 0; i < MAX_FACE_ROI; i++) {
      if (p_comp->hctresult[i]) {
        ret = p_comp->p_lib->FACEPROC_CT_DeleteResultHandle(
          p_comp->hctresult[i]);
        if (ret != FACEPROC_NORMAL)
          return IMG_ERR_GENERAL;
        p_comp->hctresult[i] = NULL;
      }
    }
#endif
#if(FACE_BGS_DETECT)
    if (p_comp->hsm) {
      ret = p_comp->p_lib->FACEPROC_SM_DeleteHandle(p_comp->hsm);
      if (ret != FACEPROC_NORMAL)
        return IMG_ERR_GENERAL;
      p_comp->hsm = NULL;
    }
    for (i = 0; i < MAX_FACE_ROI; i++) {
      if (p_comp->hsmresult[i]) {
        ret = p_comp->p_lib->FACEPROC_SM_DeleteResultHandle(
          p_comp->hsmresult[i]);
        if (ret != FACEPROC_NORMAL)
          return IMG_ERR_GENERAL;
        p_comp->hsmresult[i] = NULL;
      }
    }
#endif
  /* Face Recognition */
#if(FACE_RECOGNITION)
    if (p_comp->hfeature) {
      ret = p_comp->p_lib->FACEPROC_FR_DeleteFeatureHandle(
        p_comp->hfeature);
      if (ret != FACEPROC_NORMAL)
        return IMG_ERR_GENERAL;
      p_comp->hfeature = NULL;
    }
    if (p_comp->halbum) {
      ret = p_comp->p_lib->FACEPROC_FR_DeleteAlbumHandle(
        p_comp->halbum);
      if (ret != FACEPROC_NORMAL)
        return IMG_ERR_GENERAL;
      p_comp->halbum = NULL;
    }
#endif
  /* Blink/Gaze Detection */
#if(FACE_BGS_DETECT)
    if (p_comp->hgb) {
      ret = p_comp->p_lib->FACEPROC_GB_DeleteHandle(p_comp->hgb);
      if (ret != FACEPROC_NORMAL)
        return IMG_ERR_GENERAL;
      p_comp->hgb = NULL;
    }
    for (i = 0; i < MAX_FACE_ROI; i++) {
      if (p_comp->hgbresult[i]) {
        ret = p_comp->p_lib->FACEPROC_GB_DeleteResultHandle(
          p_comp->hgbresult[i]);
        if (ret != FACEPROC_NORMAL)
          return IMG_ERR_GENERAL;
        p_comp->hgbresult[i] = NULL;
      }
    }
#endif
  }

#ifndef FD_WITH_DSP
  /* API to detach allocated memory from Faceproc engine */
  if (p_comp->fdBmemorySet) {
    ret = p_comp->p_lib->FACEPROC_FreeBMemoryArea();
    p_comp->fdBmemorySet = FALSE;
  }
  /* API to detach allocated memory from Faceproc */
  if (p_comp->fdWmemorySet) {
    ret = p_comp->p_lib->FACEPROC_FreeWMemoryArea();
    IDBG_MED("Free Wmemory %d", ret);
    p_comp->fdWmemorySet = FALSE;
  }
#endif
  /* Scratch buffers */
  if (p_comp->bmem) {
    free(p_comp->bmem);
    p_comp->bmem = NULL;
  }

  if (p_comp->wmem) {
    free(p_comp->wmem);
    p_comp->wmem = NULL;
  }
  dump_count = 0;

  if (profile_fd == TRUE) {
    delta = IMG_TIMER_END(p_comp->fd_profile.start_time[FD_HANDLE_DELETE_TIME],
      p_comp->fd_profile.end_time[FD_HANDLE_DELETE_TIME],
      "FD_DeleteHandles", IMG_TIMER_MODE_MS);

    IDBG_INFO("FDPROFILE : SWLibraryDeleteHandlesTime %u", delta);
  }

  return IMG_SUCCESS;
}
