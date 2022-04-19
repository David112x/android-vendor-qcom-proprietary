/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "faceproc_lib_wrapper.h"

int fpsww_dsp_dt_execute(faceproc_sw_wrap_t *p_hnd, img_frame_t *p_frame,
  bool get_results_only, int32_t *p_num_faces);

/**
 * Function: fpsww_liberr_to_imgerror
 *
 * Description: Converts faceproc lib error to Imglib error
 *
 * Input parameters:
 *   lib_ret - faceproc lib error code
 *
 * Return values:
 *     Corresponding Imglib error code.
 *     IMG_SUCCESS for FACEPROC_NORMAL
 *     other corresponding errors
 *
 * Notes: none
 **/
static int fpsww_liberr_to_imgerror(int lib_ret)
{
  int img_error;

  switch (lib_ret) {
    case FACEPROC_NORMAL               : img_error = IMG_SUCCESS;               break;
    case FACEPROC_NOTIMPLEMENTED       : img_error = IMG_ERR_NOT_SUPPORTED;     break;
    case FACEPROC_TIMEOUT              : img_error = IMG_ERR_TIMEOUT;           break;
    case FACEPROC_ERR_INVALIDPARAM     : img_error = IMG_ERR_INVALID_INPUT;     break;
    case FACEPROC_ERR_MODEDIFFERENT    : img_error = IMG_ERR_INVALID_OPERATION; break;
    case FACEPROC_ERR_NOHANDLE         : img_error = IMG_ERR_INVALID_INPUT;     break;

    case FACEPROC_ERR_ALLOCMEMORY      :
    case FACEPROC_MEMORYSHORTAGE       : img_error = IMG_ERR_NO_MEMORY;         break;

    case FACEPROC_HALT                 :
    case FACEPROC_ERR_VARIOUS          :
    case FACEPROC_ERR_INITIALIZE       :
    case FACEPROC_ERR_NOALLOC          :
    case FACEPROC_ERR_PROCESSCONDITION :
    default                            : img_error = IMG_ERR_GENERAL;           break;
  }

  return img_error;
}

/**
 * Function: fpsww_co_load_fptrs
 *
 * Description: Load CO function pointers
 *
 * Input parameters:
 *   p_lib_ptr - library handle
 *   p_co_funcs - CO fptr structure to hold the function pointers
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_NOT_FOUND if symbol is not found in the library
 *
 * Notes:
 *     Caller has to make sure the passed args are not NULL
 *     All the function pointers are must currently.
 **/
int fpsww_co_load_fptrs(void *p_lib_ptr, fpsww_co_fptr_t *p_co_funcs)
{
  IMG_DLSYM_MUST(p_lib_ptr, p_co_funcs, FACEPROC_CO_GetVersion,            "FACEPROC_CO_GetVersion");

  IMG_DLSYM_MUST(p_lib_ptr, p_co_funcs, FACEPROC_CO_CreateHandle,          "FACEPROC_CO_CreateHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_co_funcs, FACEPROC_CO_CreateHandleMalloc,    "FACEPROC_CO_CreateHandleMalloc");

  IMG_DLSYM_MUST(p_lib_ptr, p_co_funcs, FACEPROC_CO_CreateHandleMemory,    "FACEPROC_CO_CreateHandleMemory");
  IMG_DLSYM_MUST(p_lib_ptr, p_co_funcs, FACEPROC_CO_DeleteHandle,          "FACEPROC_CO_DeleteHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_co_funcs, FACEPROC_CO_ConvertSquareToCenter, "FACEPROC_CO_ConvertSquareToCenter");
  IMG_DLSYM_MUST(p_lib_ptr, p_co_funcs, FACEPROC_CO_ConvertCenterToSquare, "FACEPROC_CO_ConvertCenterToSquare");

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dt_load_fptrs
 *
 * Description: Load DT function pointers
 *
 * Input parameters:
 *   p_lib_ptr - library handle
 *   p_dt_funcs - DT fptr structure to hold the function pointers
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_NOT_FOUND if symbol is not found in the library
 *
 * Notes:
 *     Caller has to make sure the passed args are not NULL
 *     All the function pointers are must currently.
 **/
int fpsww_dt_load_fptrs(void *p_lib_ptr, fpsww_dt_fptr_t *p_dt_funcs)
{
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetVersion,              "FACEPROC_DT_GetVersion");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_CreateHandle,            "FACEPROC_DT_CreateHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_DeleteHandle,            "FACEPROC_DT_DeleteHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_CreateResultHandle,      "FACEPROC_DT_CreateResultHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_DeleteResultHandle,      "FACEPROC_DT_DeleteResultHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_Detect_GRAY,             "FACEPROC_DT_Detect_GRAY");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_Detect_YUV422,           "FACEPROC_DT_Detect_YUV422");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_Detect_YUV420SP,         "FACEPROC_DT_Detect_YUV420SP");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_Detect_YUV420FP,         "FACEPROC_DT_Detect_YUV420FP");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetResultCount,          "FACEPROC_DT_GetResultCount");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetResultInfo,           "FACEPROC_DT_GetResultInfo");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetRawResultInfo,        "FACEPROC_DT_GetRawResultInfo");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_SetSizeRange,            "FACEPROC_DT_SetSizeRange");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetSizeRange,            "FACEPROC_DT_GetSizeRange");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_SetAngle,                "FACEPROC_DT_SetAngle");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetAngle,                "FACEPROC_DT_GetAngle");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_SetEdgeMask,             "FACEPROC_DT_SetEdgeMask");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetEdgeMask,             "FACEPROC_DT_GetEdgeMask");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_SetSearchDensity,        "FACEPROC_DT_SetSearchDensity");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetSearchDensity,        "FACEPROC_DT_GetSearchDensity");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_SetThreshold,            "FACEPROC_DT_SetThreshold");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetThreshold,            "FACEPROC_DT_GetThreshold");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_ResetTracking,        "FACEPROC_DT_MV_ResetTracking");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_ToggleTrackingLock,   "FACEPROC_DT_MV_ToggleTrackingLock");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetSearchCycle,       "FACEPROC_DT_MV_SetSearchCycle");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetSearchCycle,       "FACEPROC_DT_MV_GetSearchCycle");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetLostParam,         "FACEPROC_DT_MV_SetLostParam");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetLostParam,         "FACEPROC_DT_MV_GetLostParam");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetSteadinessParam,   "FACEPROC_DT_MV_SetSteadinessParam");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetSteadinessParam,   "FACEPROC_DT_MV_GetSteadinessParam");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetTrackingSwapParam, "FACEPROC_DT_MV_SetTrackingSwapParam");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetTrackingSwapParam, "FACEPROC_DT_MV_GetTrackingSwapParam");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetDelayCount,        "FACEPROC_DT_MV_SetDelayCount");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetDelayCount,        "FACEPROC_DT_MV_GetDelayCount");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetTrackingEdgeMask,  "FACEPROC_DT_MV_SetTrackingEdgeMask");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetTrackingEdgeMask,  "FACEPROC_DT_MV_GetTrackingEdgeMask");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetAccuracy,          "FACEPROC_DT_MV_SetAccuracy");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetAccuracy,          "FACEPROC_DT_MV_GetAccuracy");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetAngleExtension,    "FACEPROC_DT_MV_SetAngleExtension");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetAngleExtension,    "FACEPROC_DT_MV_GetAngleExtension");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetPoseExtension,     "FACEPROC_DT_MV_SetPoseExtension");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetPoseExtension,     "FACEPROC_DT_MV_GetPoseExtension");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetDirectionMask,     "FACEPROC_DT_MV_SetDirectionMask");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetDirectionMask,     "FACEPROC_DT_MV_GetDirectionMask");

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_pt_load_fptrs
 *
 * Description: Load PT function pointers
 *
 * Input parameters:
 *   p_lib_ptr - library handle
 *   p_pt_funcs - PT fptr structure to hold the function pointers
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_NOT_FOUND if symbol is not found in the library
 *
 * Notes:
 *     Caller has to make sure the passed args are not NULL
 *     All the function pointers are must currently.
 **/
int fpsww_pt_load_fptrs(void *p_lib_ptr, fpsww_pt_fptr_t *p_pt_funcs)
{
  IMG_DLSYM_MUST(p_lib_ptr, p_pt_funcs, FACEPROC_PT_GetVersion,            "FACEPROC_PT_GetVersion");

  IMG_DLSYM_MUST(p_lib_ptr, p_pt_funcs, FACEPROC_PT_CreateHandle,          "FACEPROC_PT_CreateHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_pt_funcs, FACEPROC_PT_DeleteHandle,          "FACEPROC_PT_DeleteHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_pt_funcs, FACEPROC_PT_CreateResultHandle,    "FACEPROC_PT_CreateResultHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_pt_funcs, FACEPROC_PT_DeleteResultHandle,    "FACEPROC_PT_DeleteResultHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_pt_funcs, FACEPROC_PT_SetPositionFromHandle, "FACEPROC_PT_SetPositionFromHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_pt_funcs, FACEPROC_PT_SetPosition,           "FACEPROC_PT_SetPosition");
  IMG_DLSYM_MUST(p_lib_ptr, p_pt_funcs, FACEPROC_PT_SetPositionIP,         "FACEPROC_PT_SetPositionIP");

  IMG_DLSYM_MUST(p_lib_ptr, p_pt_funcs, FACEPROC_PT_SetMode,               "FACEPROC_PT_SetMode");
  IMG_DLSYM_MUST(p_lib_ptr, p_pt_funcs, FACEPROC_PT_GetMode,               "FACEPROC_PT_GetMode");

  IMG_DLSYM_MUST(p_lib_ptr, p_pt_funcs, FACEPROC_PT_SetConfMode,           "FACEPROC_PT_SetConfMode");
  IMG_DLSYM_MUST(p_lib_ptr, p_pt_funcs, FACEPROC_PT_GetConfMode,           "FACEPROC_PT_GetConfMode");

  IMG_DLSYM_MUST(p_lib_ptr, p_pt_funcs, FACEPROC_PT_DetectPoint,           "FACEPROC_PT_DetectPoint");
  IMG_DLSYM_MUST(p_lib_ptr, p_pt_funcs, FACEPROC_PT_GetResult,             "FACEPROC_PT_GetResult");
  IMG_DLSYM_MUST(p_lib_ptr, p_pt_funcs, FACEPROC_PT_GetFaceDirection,      "FACEPROC_PT_GetFaceDirection");

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_ct_load_fptrs
 *
 * Description: Load CT function pointers
 *
 * Input parameters:
 *   p_lib_ptr - library handle
 *   p_ct_funcs - CT fptr structure to hold the function pointers
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_NOT_FOUND if symbol is not found in the library
 *
 * Notes:
 *     Caller has to make sure the passed args are not NULL
 *     All the function pointers are must currently.
 **/
int fpsww_ct_load_fptrs(void *p_lib_ptr, fpsww_ct_fptr_t *p_ct_funcs)
{
  IMG_DLSYM_MUST(p_lib_ptr, p_ct_funcs, FACEPROC_CT_GetVersion,         "FACEPROC_CT_GetVersion");

  IMG_DLSYM_MUST(p_lib_ptr, p_ct_funcs, FACEPROC_CT_CreateHandle,       "FACEPROC_CT_CreateHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_ct_funcs, FACEPROC_CT_DeleteHandle,       "FACEPROC_CT_DeleteHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_ct_funcs, FACEPROC_CT_CreateResultHandle, "FACEPROC_CT_CreateResultHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_ct_funcs, FACEPROC_CT_DeleteResultHandle, "FACEPROC_CT_DeleteResultHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_ct_funcs, FACEPROC_CT_SetPoint,           "FACEPROC_CT_SetPoint");
  IMG_DLSYM_MUST(p_lib_ptr, p_ct_funcs, FACEPROC_CT_SetPointFromHandle, "FACEPROC_CT_SetPointFromHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_ct_funcs, FACEPROC_CT_DetectContour,      "FACEPROC_CT_DetectContour");
  IMG_DLSYM_MUST(p_lib_ptr, p_ct_funcs, FACEPROC_CT_GetResult,          "FACEPROC_CT_GetResult");

  IMG_DLSYM_MUST(p_lib_ptr, p_ct_funcs, FACEPROC_CT_SetDetectionMode,   "FACEPROC_CT_SetDetectionMode");
  IMG_DLSYM_MUST(p_lib_ptr, p_ct_funcs, FACEPROC_CT_GetDetectionMode,   "FACEPROC_CT_GetDetectionMode");

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_sm_load_fptrs
 *
 * Description: Load SM function pointers
 *
 * Input parameters:
 *   p_lib_ptr - library handle
 *   p_sm_funcs - SM fptr structure to hold the function pointers
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_NOT_FOUND if symbol is not found in the library
 *
 * Notes:
 *     Caller has to make sure the passed args are not NULL
 *     All the function pointers are must currently.
 **/
int fpsww_sm_load_fptrs(void *p_lib_ptr, fpsww_sm_fptr_t *p_sm_funcs)
{
  IMG_DLSYM_MUST(p_lib_ptr, p_sm_funcs, FACEPROC_SM_GetVersion,         "FACEPROC_SM_GetVersion");

  IMG_DLSYM_MUST(p_lib_ptr, p_sm_funcs, FACEPROC_SM_CreateHandle,       "FACEPROC_SM_CreateHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_sm_funcs, FACEPROC_SM_DeleteHandle,       "FACEPROC_SM_DeleteHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_sm_funcs, FACEPROC_SM_CreateResultHandle, "FACEPROC_SM_CreateResultHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_sm_funcs, FACEPROC_SM_DeleteResultHandle, "FACEPROC_SM_DeleteResultHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_sm_funcs, FACEPROC_SM_SetPoint,           "FACEPROC_SM_SetPoint");
  IMG_DLSYM_MUST(p_lib_ptr, p_sm_funcs, FACEPROC_SM_SetPointFromHandle, "FACEPROC_SM_SetPointFromHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_sm_funcs, FACEPROC_SM_Estimate,           "FACEPROC_SM_Estimate");
  IMG_DLSYM_MUST(p_lib_ptr, p_sm_funcs, FACEPROC_SM_GetResult,          "FACEPROC_SM_GetResult");

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_gb_load_fptrs
 *
 * Description: Load GB function pointers
 *
 * Input parameters:
 *   p_lib_ptr - library handle
 *   p_gb_funcs - GB fptr structure to hold the function pointers
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_NOT_FOUND if symbol is not found in the library
 *
 * Notes:
 *     Caller has to make sure the passed args are not NULL
 *     All the function pointers are must currently.
 **/
int fpsww_gb_load_fptrs(void *p_lib_ptr, fpsww_gb_fptr_t *p_gb_funcs)
{
  IMG_DLSYM_MUST(p_lib_ptr, p_gb_funcs, FACEPROC_GB_GetVersion,         "FACEPROC_GB_GetVersion");

  IMG_DLSYM_MUST(p_lib_ptr, p_gb_funcs, FACEPROC_GB_CreateHandle,       "FACEPROC_GB_CreateHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_gb_funcs, FACEPROC_GB_DeleteHandle,       "FACEPROC_GB_DeleteHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_gb_funcs, FACEPROC_GB_CreateResultHandle, "FACEPROC_GB_CreateResultHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_gb_funcs, FACEPROC_GB_DeleteResultHandle, "FACEPROC_GB_DeleteResultHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_gb_funcs, FACEPROC_GB_SetPoint,           "FACEPROC_GB_SetPoint");
  IMG_DLSYM_MUST(p_lib_ptr, p_gb_funcs, FACEPROC_GB_SetPointFromHandle, "FACEPROC_GB_SetPointFromHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_gb_funcs, FACEPROC_GB_Estimate,           "FACEPROC_GB_Estimate");
  IMG_DLSYM_MUST(p_lib_ptr, p_gb_funcs, FACEPROC_GB_GetEyeCloseRatio,   "FACEPROC_GB_GetEyeCloseRatio");
  IMG_DLSYM_MUST(p_lib_ptr, p_gb_funcs, FACEPROC_GB_GetGazeDirection,   "FACEPROC_GB_GetGazeDirection");

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_fr_load_fptrs
 *
 * Description: Load FR function pointers
 *
 * Input parameters:
 *   p_lib_ptr - library handle
 *   p_fr_funcs - FR fptr structure to hold the function pointers
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_NOT_FOUND if symbol is not found in the library
 *
 * Notes:
 *     Caller has to make sure the passed args are not NULL
 *     Not implemented
 **/
int fpsww_fr_load_fptrs(void *p_lib_ptr, fpsww_fr_fptr_t *p_fr_funcs)
{
  return IMG_SUCCESS;
}

/**
 * Function: fpsww_co_create_handles
 *
 * Description: Create CO handles
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_GENERAL on handle creation failure
 *
 * Notes:
 *     Caller has to make sure the passed args are not NULL
 **/
int fpsww_co_create_handles(faceproc_sw_wrap_t *p_hnd)
{
  fpsww_co_fptr_t *p_co_funcs = (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) ?
                                &p_hnd->p_lib->dsp_co_funcs : &p_hnd->p_lib->co_funcs;

  p_hnd->hCo = p_co_funcs->FACEPROC_CO_CreateHandle();
  if (!p_hnd->hCo) {
    IDBG_ERROR("Failed in creating CO handle");
    return IMG_ERR_GENERAL;
  }
  IDBG_MED("p_hnd->hCo=%p", p_hnd->hCo);

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dt_create_handles
 *
 * Description: Create DT handles
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   detection_mode - Detection mode that DT needs to run in.
 *   max_detection_count - maximum number of faces to detect
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_GENERAL on handle creation failure
 *
 * Notes:
 *     Caller has to make sure the passed args are not NULL
 *     If failed, caller has to call delete handles which were created succesfully.
 **/
int fpsww_dt_create_handles(faceproc_sw_wrap_t *p_hnd, int32_t detection_mode, int32_t max_detection_count)
{
  fpsww_dt_fptr_t *p_dt_funcs = (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) ?
                                &p_hnd->p_lib->dsp_dt_funcs : &p_hnd->p_lib->dt_funcs;

  p_hnd->hdt = p_dt_funcs->FACEPROC_DT_CreateHandle(p_hnd->hCo, detection_mode, max_detection_count);
  if (!p_hnd->hdt) {
    IDBG_ERROR("Failed in creating DT handle : detection_mode=%d, max_detection_count=%d", detection_mode, max_detection_count);
    return IMG_ERR_GENERAL;
  }
  IDBG_MED("p_hnd->hdt=%p", p_hnd->hdt);

  p_hnd->hdtresult = p_dt_funcs->FACEPROC_DT_CreateResultHandle(p_hnd->hCo);
  if (!p_hnd->hdtresult) {
    IDBG_ERROR("Failed in creating DT Results handle");
    return IMG_ERR_GENERAL;
  }
  IDBG_MED("p_hnd->hdtresult=%p", p_hnd->hdtresult);

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_pt_create_handles
 *
 * Description: Create PT handles
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   no_of_handles - number of results handles to create
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_GENERAL on handle creation failure
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 *     If failed, caller has to call delete handles which were created succesfully.
 **/
int fpsww_pt_create_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles)
{
  fpsww_pt_fptr_t *p_pt_funcs = &p_hnd->p_lib->pt_funcs;
  uint8_t          i;

  p_hnd->hpt = p_pt_funcs->FACEPROC_PT_CreateHandle();
  if (!(p_hnd->hpt)) {
    IDBG_ERROR("Failed in creating PT handle");
    return IMG_ERR_GENERAL;
  }
  IDBG_MED("p_hnd->hpt=%p", p_hnd->hpt);

  for (i = 0; i < no_of_handles; i++) {
    p_hnd->hptresult[i] = p_pt_funcs->FACEPROC_PT_CreateResultHandle();
    if (!(p_hnd->hptresult[i])) {
      IDBG_ERROR("Failed in creating PT Results handle %d", i);
      return IMG_ERR_GENERAL;
    }
    IDBG_MED("p_hnd->hptresult[%d]=%p", i, p_hnd->hptresult[i]);
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_ct_create_handles
 *
 * Description: Create CT handles
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   no_of_handles - number of results handles to create
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_GENERAL on handle creation failure
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 *     If failed, caller has to call delete handles which were created succesfully.
 **/
int fpsww_ct_create_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles)
{
  fpsww_ct_fptr_t *p_ct_funcs = &p_hnd->p_lib->ct_funcs;
  uint8_t          i;

  p_hnd->hct = p_ct_funcs->FACEPROC_CT_CreateHandle();
  if (!(p_hnd->hct)) {
    IDBG_ERROR("Failed in creating CT handle");
    return IMG_ERR_GENERAL;
  }
  IDBG_MED("p_hnd->hct=%p", p_hnd->hct);

  for (i = 0; i < no_of_handles; i++) {
    p_hnd->hctresult[i] = p_ct_funcs->FACEPROC_CT_CreateResultHandle();
    if (!(p_hnd->hctresult[i])) {
      IDBG_ERROR("Failed in creating CT Results handle %d", i);
      return IMG_ERR_GENERAL;
    }
    IDBG_MED("p_hnd->hctresult[%d]=%p", i, p_hnd->hctresult[i]);
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_sm_create_handles
 *
 * Description: Create SM handles
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   no_of_handles - number of results handles to create
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_GENERAL on handle creation failure
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 *     If failed, caller has to call delete handles which were created succesfully.
 **/
int fpsww_sm_create_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles)
{
  fpsww_sm_fptr_t *p_sm_funcs = &p_hnd->p_lib->sm_funcs;
  uint8_t          i;

  p_hnd->hsm = p_sm_funcs->FACEPROC_SM_CreateHandle();
  if (!(p_hnd->hsm)) {
    IDBG_ERROR("Failed in creating SM handle");
    return IMG_ERR_GENERAL;
  }
  IDBG_MED("p_hnd->hsm=%p", p_hnd->hsm);

  for (i = 0; i < no_of_handles; i++) {
    p_hnd->hsmresult[i] = p_sm_funcs->FACEPROC_SM_CreateResultHandle();
    if (!(p_hnd->hctresult[i])) {
      IDBG_ERROR("Failed in creating SM Results handle %d", i);
      return IMG_ERR_GENERAL;
    }
    IDBG_MED("p_hnd->hsmresult[%d]=%p", i, p_hnd->hsmresult[i]);
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_gb_create_handles
 *
 * Description: Create GB handles
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   no_of_handles - number of results handles to create
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_GENERAL on handle creation failure
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 *     If failed, caller has to call delete handles which were created succesfully.
 **/

int fpsww_gb_create_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles)
{
  fpsww_gb_fptr_t *p_gb_funcs = &p_hnd->p_lib->gb_funcs;
  uint8_t          i;

  p_hnd->hgb = p_gb_funcs->FACEPROC_GB_CreateHandle();
  if (!(p_hnd->hgb)) {
    IDBG_ERROR("Failed in creating SM handle");
    return IMG_ERR_GENERAL;
  }
  IDBG_MED("p_hnd->hgb=%p", p_hnd->hgb);

  for (i = 0; i < no_of_handles; i++) {
    p_hnd->hgbresult[i] = p_gb_funcs->FACEPROC_GB_CreateResultHandle();
    if (!(p_hnd->hgbresult[i])) {
      IDBG_ERROR("Failed in creating SM Results handle %d", i);
      return IMG_ERR_GENERAL;
    }
    IDBG_MED("p_hnd->hgbresult[%d]=%p", i, p_hnd->hgbresult[i]);
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_fr_create_handles
 *
 * Description: Create FR handles
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   no_of_handles - number of results handles to create
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_GENERAL on handle creation failure
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 *     If failed, caller has to call delete handles which were created succesfully.
 **/
int fpsww_fr_create_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles)
{
  return IMG_SUCCESS;
}

/**
 * Function: fpsww_co_delete_handles
 *
 * Description: Delete CO handles
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_co_delete_handles(faceproc_sw_wrap_t *p_hnd)
{
  if (!p_hnd->hCo) {
    IDBG_MED("CO handle not created");
    return IMG_SUCCESS;
  }

  fpsww_co_fptr_t *p_co_funcs = (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) ?
                                &p_hnd->p_lib->dsp_co_funcs : &p_hnd->p_lib->co_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  IDBG_MED("p_hnd->hCo=%p", p_hnd->hCo);
  lib_ret = p_co_funcs->FACEPROC_CO_DeleteHandle(p_hnd->hCo);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in deleting CO handle, lib_ret=%d", lib_ret);
  }

  p_hnd->hCo = NULL;
  return fpsww_liberr_to_imgerror(lib_ret);
}

/**
 * Function: fpsww_dt_delete_handles
 *
 * Description: Delete DT handles
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_dt_delete_handles(faceproc_sw_wrap_t *p_hnd)
{
  fpsww_dt_fptr_t *p_dt_funcs = (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) ?
                                &p_hnd->p_lib->dsp_dt_funcs : &p_hnd->p_lib->dt_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  if (p_hnd->hdt) {
    IDBG_MED("p_hnd->hdt=%p", p_hnd->hdt);
    lib_ret = p_dt_funcs->FACEPROC_DT_DeleteHandle(p_hnd->hdt);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in deleting DT handle %p, lib_ret=%d", p_hnd->hdt, lib_ret);
    }
    p_hnd->hdt = NULL;
  }

  if (p_hnd->hdtresult) {
    IDBG_MED("p_hnd->hdtresult=%p", p_hnd->hdtresult);
    lib_ret = p_dt_funcs->FACEPROC_DT_DeleteResultHandle(p_hnd->hdtresult);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in deleting DT Results handle %p, lib_ret=%d", p_hnd->hdtresult, lib_ret);
    }
    p_hnd->hdtresult = NULL;
  }

  return fpsww_liberr_to_imgerror(lib_ret);
}

/**
 * Function: fpsww_pt_delete_handles
 *
 * Description: Delete PT handles
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   no_of_handles - number of results handles to delete
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_pt_delete_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles)
{
  fpsww_pt_fptr_t *p_pt_funcs = &p_hnd->p_lib->pt_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;
  uint8_t          i;

  if (p_hnd->hpt) {
    for (i = 0; i < no_of_handles; i++) {
      if (p_hnd->hptresult[i]) {
        IDBG_MED("p_hnd->hptresult[%d]=%p", i, p_hnd->hptresult[i]);
        lib_ret = p_pt_funcs->FACEPROC_PT_DeleteResultHandle(p_hnd->hptresult[i]);
        if (lib_ret != FACEPROC_NORMAL) {
          IDBG_ERROR("Failed in deleting PT Results handle index %d, handle=%p, lib_ret=%d", i, p_hnd->hptresult[i], lib_ret);
        }
        p_hnd->hptresult[i] = NULL;
      }
    }

    IDBG_MED("p_hnd->hpt=%p", p_hnd->hpt);
    lib_ret = p_pt_funcs->FACEPROC_PT_DeleteHandle(p_hnd->hpt);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in deleting PT handle %p, lib_ret=%d", p_hnd->hpt, lib_ret);
    }
    p_hnd->hpt = NULL;
  }

  return fpsww_liberr_to_imgerror(lib_ret);
}

/**
 * Function: fpsww_ct_delete_handles
 *
 * Description: Delete CT handles
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   no_of_handles - number of results handles to delete
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_ct_delete_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles)
{
  fpsww_ct_fptr_t *p_ct_funcs = &p_hnd->p_lib->ct_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;
  uint8_t          i;

  if (p_hnd->hct) {
    for (i = 0; i < no_of_handles; i++) {
      if (p_hnd->hctresult[i]) {
        IDBG_MED("p_hnd->hctresult[%d]=%p", i, p_hnd->hctresult[i]);
        lib_ret = p_ct_funcs->FACEPROC_CT_DeleteResultHandle(p_hnd->hctresult[i]);
        if (lib_ret != FACEPROC_NORMAL) {
          IDBG_ERROR("Failed in deleting CT Results handle index %d, handle=%p, lib_ret=%d", i, p_hnd->hctresult[i], lib_ret);
        }
        p_hnd->hctresult[i] = NULL;
      }
    }

    IDBG_MED("p_hnd->hct=%p", p_hnd->hct);
    lib_ret = p_ct_funcs->FACEPROC_CT_DeleteHandle(p_hnd->hct);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in deleting CT handle %p, lib_ret=%d", p_hnd->hct, lib_ret);
    }
    p_hnd->hct = NULL;
  }

  return fpsww_liberr_to_imgerror(lib_ret);
}

/**
 * Function: fpsww_sm_delete_handles
 *
 * Description: Delete SM handles
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   no_of_handles - number of results handles to delete
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_sm_delete_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles)
{
  fpsww_sm_fptr_t *p_sm_funcs = &p_hnd->p_lib->sm_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;
  uint8_t          i;

  if (p_hnd->hsm) {
    for (i = 0; i < no_of_handles; i++) {
      if (p_hnd->hsmresult[i]) {
        IDBG_MED("p_hnd->hsmresult[%d]=%p", i, p_hnd->hsmresult[i]);
        lib_ret = p_sm_funcs->FACEPROC_SM_DeleteResultHandle(p_hnd->hsmresult[i]);
        if (lib_ret != FACEPROC_NORMAL) {
          IDBG_ERROR("Failed in deleting SM Results handle index %d, handle=%p, lib_ret=%d", i, p_hnd->hsmresult[i], lib_ret);
        }
        p_hnd->hsmresult[i] = NULL;
      }
    }

    IDBG_MED("p_hnd->hsm=%p", p_hnd->hsm);
    lib_ret = p_sm_funcs->FACEPROC_SM_DeleteHandle(p_hnd->hsm);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in deleting SM handle %p, lib_ret=%d", p_hnd->hsm, lib_ret);
    }
    p_hnd->hsm = NULL;
  }

  return fpsww_liberr_to_imgerror(lib_ret);
}

/**
 * Function: fpsww_gb_delete_handles
 *
 * Description: Delete GB handles
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   no_of_handles - number of results handles to delete
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_gb_delete_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles)
{
  fpsww_gb_fptr_t *p_gb_funcs = &p_hnd->p_lib->gb_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;
  uint8_t          i;

  if (p_hnd->hgb) {
    for (i = 0; i < no_of_handles; i++) {
      if (p_hnd->hgbresult[i]) {
        IDBG_MED("p_hnd->hgbresult[%d]=%p", i, p_hnd->hgbresult[i]);
        lib_ret = p_gb_funcs->FACEPROC_GB_DeleteResultHandle(p_hnd->hgbresult[i]);
        if (lib_ret != FACEPROC_NORMAL) {
          IDBG_ERROR("Failed in deleting GB Results handle index %d, handle=%p, lib_ret=%d", i, p_hnd->hgbresult[i], lib_ret);
        }
        p_hnd->hgbresult[i] = NULL;
      }
    }

    IDBG_MED("p_hnd->hgb=%p", p_hnd->hgb);
    lib_ret = p_gb_funcs->FACEPROC_GB_DeleteHandle(p_hnd->hgb);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in deleting GB handle %p, lib_ret=%d", p_hnd->hgb, lib_ret);
    }
    p_hnd->hgb = NULL;
  }

  return fpsww_liberr_to_imgerror(lib_ret);
}

/**
 * Function: fpsww_fr_delete_handles
 *
 * Description: Delete FR handles
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_fr_delete_handles(faceproc_sw_wrap_t *p_hnd)
{
  return IMG_SUCCESS;
}

/**
 * Function: fpsww_co_get_config
 *
 * Description: Get current CO configuration
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_co_params - Pointer to fill CO configuration values
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_co_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_co_params_t *p_co_params)
{
  if (!p_hnd->hCo) {
    IDBG_ERROR("Invalid Co handle");
    return IMG_ERR_INVALID_INPUT;
  }

  fpsww_co_fptr_t *p_co_funcs = (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) ?
                                &p_hnd->p_lib->dsp_co_funcs : &p_hnd->p_lib->co_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  lib_ret = p_co_funcs->FACEPROC_CO_GetVersion(&p_co_params->majorVersion, &p_co_params->minorVersion);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in CO_GetVersion, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dt_get_config
 *
 * Description: Get current DT configuration
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_dt_params - Pointer to fill DTconfiguration values
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_dt_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_dt_params_t *p_dt_params)
{
  if (!p_hnd->hdt) {
    IDBG_ERROR("Invalid DT handle");
    return IMG_ERR_INVALID_INPUT;
  }

  fpsww_dt_fptr_t *p_dt_funcs = (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) ?
                                &p_hnd->p_lib->dsp_dt_funcs : &p_hnd->p_lib->dt_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  // If DSP use FACEPROC_DT_GetDTConfig
  if (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) {
    fpsww_dsp_fptr_t *p_dsp_funcs = &p_hnd->p_lib->dsp_funcs;

    lib_ret = p_dsp_funcs->FACEPROC_DT_GetDTConfig(p_hnd->hdt, p_hnd->create_params.detection_mode, p_dt_params);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in FACEPROC_DT_GetDTConfig, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    return FACEPROC_NORMAL;
  }

  lib_ret = p_dt_funcs->FACEPROC_DT_GetVersion(&p_dt_params->majorVersion, &p_dt_params->minorVersion);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in DT_GetVersion, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  lib_ret = p_dt_funcs->FACEPROC_DT_GetSizeRange(p_hnd->hdt, &p_dt_params->nMinSize, &p_dt_params->nMaxSize);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in DT_GetSizeRange, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  lib_ret = p_dt_funcs->FACEPROC_DT_GetAngle(p_hnd->hdt, POSE_ANGLE_FRONT, &p_dt_params->nAngle[FPSWW_ANGLE_FRONT]);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in DT_GetAngle, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  lib_ret = p_dt_funcs->FACEPROC_DT_GetAngle(p_hnd->hdt, POSE_ANGLE_HALF_PROFILE,
    &p_dt_params->nAngle[FPSWW_ANGLE_HALFPROFILE]);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in DT_GetAngle, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  lib_ret = p_dt_funcs->FACEPROC_DT_GetAngle(p_hnd->hdt, POSE_ANGLE_PROFILE, &p_dt_params->nAngle[FPSWW_ANGLE_FULLPROFILE]);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in DT_GetAngle, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  lib_ret = p_dt_funcs->FACEPROC_DT_GetEdgeMask(p_hnd->hdt, &p_dt_params->faceSearchEdgeMask);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in DT_GetEdgeMask, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  lib_ret = p_dt_funcs->FACEPROC_DT_GetSearchDensity(p_hnd->hdt, &p_dt_params->nSearchDensity);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in DT_GetSearchDensity, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  lib_ret = p_dt_funcs->FACEPROC_DT_GetThreshold(p_hnd->hdt, &p_dt_params->nThreshold);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in DT_GetThreshold, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  if ((p_hnd->create_params.detection_mode == DETECTION_MODE_MOVIE) ||
    (p_hnd->create_params.detection_mode == CUSTOM_MODE_PARTIAL_STILL)) {
    lib_ret = p_dt_funcs->FACEPROC_DT_MV_GetSearchCycle(p_hnd->hdt,
                                                        &p_dt_params->nInitialFaceSearchCycle,
                                                        &p_dt_params->nNewFaceSearchCycle,
                                                        &p_dt_params->nNewFaceSearchInterval);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_MV_GetSearchCycle, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  if (p_hnd->create_params.detection_mode == DETECTION_MODE_MOVIE) {
    lib_ret = p_dt_funcs->FACEPROC_DT_MV_GetLostParam(p_hnd->hdt, &p_dt_params->nMaxRetryCount, &p_dt_params->nMaxHoldCount);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in FACEPROC_DT_MV_GetLostParam, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_dt_funcs->FACEPROC_DT_MV_GetSteadinessParam(p_hnd->hdt,
                                                            &p_dt_params->nPosSteadinessParam,
                                                            &p_dt_params->nSizeSteadinessParam);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_MV_GetSteadinessParam, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_dt_funcs->FACEPROC_DT_MV_GetTrackingSwapParam(p_hnd->hdt, &p_dt_params->nTrackingSwapParam);
      if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_MV_GetTrackingSwapParam, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_dt_funcs->FACEPROC_DT_MV_GetDelayCount(p_hnd->hdt, &p_dt_params->nDelayCount);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_MV_GetDelayCount, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_dt_funcs->FACEPROC_DT_MV_GetTrackingEdgeMask(p_hnd->hdt, &p_dt_params->trackingEdgeMask);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_MV_GetDelayCount, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_dt_funcs->FACEPROC_DT_MV_GetAccuracy(p_hnd->hdt, &p_dt_params->nAccuracy);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_MV_GetAccuracy, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_dt_funcs->FACEPROC_DT_MV_GetAngleExtension(p_hnd->hdt, &p_dt_params->rollAngleExtension);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_MV_GetAngleExtension, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_dt_funcs->FACEPROC_DT_MV_GetPoseExtension(p_hnd->hdt,
                                                          &p_dt_params->yawAngleExtension, &p_dt_params->bUseHeadTracking);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_MV_GetPoseExtension, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_dt_funcs->FACEPROC_DT_MV_GetDirectionMask(p_hnd->hdt, &p_dt_params->bMask);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_MV_GetDirectionMask, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_pt_get_config
 *
 * Description: Get current PT configuration
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_pt_params - Pointer to fill PT configuration values
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_pt_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_pt_params_t *p_pt_params)
{
  if (!p_hnd->hpt) {
    IDBG_MED("PT is disabled, handle is not created");
    return IMG_SUCCESS;
  }

  fpsww_pt_fptr_t *p_pt_funcs = &p_hnd->p_lib->pt_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  lib_ret = p_pt_funcs->FACEPROC_PT_GetVersion(&p_pt_params->majorVersion, &p_pt_params->minorVersion);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in PT_GetVersion, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  lib_ret = p_pt_funcs->FACEPROC_PT_GetMode(p_hnd->hpt, &p_pt_params->nMode);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in PT_GetMode, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  lib_ret = p_pt_funcs->FACEPROC_PT_GetConfMode(p_hnd->hpt, &p_pt_params->nConfMode);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in PT_GetConfMode, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_ct_get_config
 *
 * Description: Get current CT configuration
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_ct_params - Pointer to fill CT configuration values
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_ct_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_ct_params_t *p_ct_params)
{
  if (!p_hnd->hct) {
    IDBG_MED("CT is disabled, handle is not created");
    return IMG_SUCCESS;
  }

  fpsww_ct_fptr_t *p_ct_funcs = &p_hnd->p_lib->ct_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  lib_ret = p_ct_funcs->FACEPROC_CT_GetVersion(&p_ct_params->majorVersion, &p_ct_params->minorVersion);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in CT_GetVersion, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  lib_ret = p_ct_funcs->FACEPROC_CT_GetDetectionMode(p_hnd->hct, &p_ct_params->nMode);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in CT_GetDetectionMode, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_sm_get_config
 *
 * Description: Get current SM configuration
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_sm_params - Pointer to fill SM configuration values
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 *     No tunable configuration params currently
 **/
int fpsww_sm_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_sm_params_t *p_sm_params)
{
  if (!p_hnd->hsm) {
    IDBG_MED("SM is disabled, handle is not created");
    return IMG_SUCCESS;
  }

  fpsww_sm_fptr_t *p_sm_funcs = &p_hnd->p_lib->sm_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  lib_ret = p_sm_funcs->FACEPROC_SM_GetVersion(&p_sm_params->majorVersion, &p_sm_params->minorVersion);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in SM_GetVersion, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_gb_get_config
 *
 * Description: Get current GB configuration
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_gb_params - Pointer to fill GB configuration values
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 *     No tunable configuration params currently
 **/
int fpsww_gb_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_gb_params_t *p_gb_params)
{
  if (!p_hnd->hgb) {
    IDBG_MED("GB is disabled, handle is not created");
    return IMG_SUCCESS;
  }

  fpsww_gb_fptr_t *p_gb_funcs = &p_hnd->p_lib->gb_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  lib_ret = p_gb_funcs->FACEPROC_GB_GetVersion(&p_gb_params->majorVersion, &p_gb_params->minorVersion);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in GB_GetVersion, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_fr_get_config
 *
 * Description: Get current FR configuration
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_fr_params - Pointer to fill FR configuration values
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 *     No tunable configuration params currently
 **/
int fpsww_fr_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_fr_params_t *p_fr_params)
{
  if (!p_hnd->hfeature) {
    IDBG_MED("FR is disabled, handle is not created");
    return IMG_SUCCESS;
  }

  fpsww_fr_fptr_t *p_fr_funcs = &p_hnd->p_lib->fr_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  lib_ret = p_fr_funcs->FACEPROC_FR_GetVersion(&p_fr_params->majorVersion, &p_fr_params->minorVersion);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in FR_GetVersion, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_co_set_config
 *
 * Description: Set CO config params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_co_params - Pointer to CO config params which need to be set to sw library
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 *     No tunable configuration params currently
 **/
int fpsww_co_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_co_params_t *p_co_params)
{
  // nothing to set for CO
  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dt_set_config
 *
 * Description: Set DT config params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_dt_new_params - Pointer to DT config params which need to be set to sw library
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_dt_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_dt_params_t *p_dt_new_params)
{
  if (!p_hnd->hdt) {
    IDBG_ERROR("Invalid DT handle");
    return IMG_ERR_INVALID_INPUT;
  }

  fpsww_dt_fptr_t   *p_dt_funcs          = (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) ?
                                           &p_hnd->p_lib->dsp_dt_funcs : &p_hnd->p_lib->dt_funcs;
  fpsww_dt_params_t *p_dt_current_params = &p_hnd->config.dt_params;
  INT32              lib_ret             = FACEPROC_NORMAL;

  // If DSP use FACEPROC_DT_SetDTConfig
  if (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) {
    fpsww_dsp_fptr_t *p_dsp_funcs = &p_hnd->p_lib->dsp_funcs;

    lib_ret = p_dsp_funcs->FACEPROC_DT_SetDTConfig(p_hnd->hdt, p_hnd->create_params.detection_mode, p_dt_new_params);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in FACEPROC_DT_SetDTConfig, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    p_hnd->config.dt_params = *p_dt_new_params;

    return FACEPROC_NORMAL;
  }

  if ((p_dt_current_params->nMinSize != p_dt_new_params->nMinSize) ||
      (p_dt_current_params->nMaxSize != p_dt_new_params->nMaxSize)) {
    lib_ret = p_dt_funcs->FACEPROC_DT_SetSizeRange(p_hnd->hdt, p_dt_new_params->nMinSize, p_dt_new_params->nMaxSize);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_SetSizeRange(%p, %d, %d), lib_ret=%d",
        p_hnd->hdt, p_dt_new_params->nMinSize, p_dt_new_params->nMaxSize, lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
    p_dt_current_params->nMinSize = p_dt_new_params->nMinSize;
    p_dt_current_params->nMaxSize = p_dt_new_params->nMaxSize;
  }

  if (p_dt_current_params->nAngle[FPSWW_ANGLE_FRONT] != p_dt_new_params->nAngle[FPSWW_ANGLE_FRONT]) {
    lib_ret = p_dt_funcs->FACEPROC_DT_SetAngle(p_hnd->hdt, POSE_ANGLE_FRONT, p_dt_new_params->nAngle[FPSWW_ANGLE_FRONT]);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_SetAngle(POSE_ANGLE_FRONT, %x), lib_ret=%d",
        p_dt_new_params->nAngle[FPSWW_ANGLE_FRONT], lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
    p_dt_current_params->nAngle[FPSWW_ANGLE_FRONT] = p_dt_new_params->nAngle[FPSWW_ANGLE_FRONT];
  }

  if (p_dt_current_params->nAngle[FPSWW_ANGLE_HALFPROFILE] != p_dt_new_params->nAngle[FPSWW_ANGLE_HALFPROFILE]) {
    lib_ret = p_dt_funcs->FACEPROC_DT_SetAngle(p_hnd->hdt, POSE_ANGLE_HALF_PROFILE,
      p_dt_new_params->nAngle[FPSWW_ANGLE_HALFPROFILE]);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_SetAngle(POSE_ANGLE_HALF_PROFILE, %x), lib_ret=%d",
        p_dt_new_params->nAngle[FPSWW_ANGLE_HALFPROFILE], lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
    p_dt_current_params->nAngle[FPSWW_ANGLE_HALFPROFILE] = p_dt_new_params->nAngle[FPSWW_ANGLE_HALFPROFILE];
  }

  if (p_dt_current_params->nAngle[FPSWW_ANGLE_FULLPROFILE] != p_dt_new_params->nAngle[FPSWW_ANGLE_FULLPROFILE]) {
    lib_ret = p_dt_funcs->FACEPROC_DT_SetAngle(p_hnd->hdt, POSE_ANGLE_PROFILE,
      p_dt_new_params->nAngle[FPSWW_ANGLE_FULLPROFILE]);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_SetAngle(POSE_ANGLE_PROFILE, %x), lib_ret=%d",
        p_dt_new_params->nAngle[FPSWW_ANGLE_FULLPROFILE], lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
    p_dt_current_params->nAngle[FPSWW_ANGLE_FULLPROFILE] = p_dt_new_params->nAngle[FPSWW_ANGLE_FULLPROFILE];
  }

  if ((p_dt_current_params->faceSearchEdgeMask.left   != p_dt_new_params->faceSearchEdgeMask.left)  ||
      (p_dt_current_params->faceSearchEdgeMask.top    != p_dt_new_params->faceSearchEdgeMask.top)   ||
      (p_dt_current_params->faceSearchEdgeMask.right  != p_dt_new_params->faceSearchEdgeMask.right) ||
      (p_dt_current_params->faceSearchEdgeMask.bottom != p_dt_new_params->faceSearchEdgeMask.bottom)) {
    lib_ret = p_dt_funcs->FACEPROC_DT_SetEdgeMask(p_hnd->hdt, p_dt_new_params->faceSearchEdgeMask);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_SetEdgeMask(%d, %d, %d, %d) lib_ret=%d",
        p_dt_new_params->faceSearchEdgeMask.left, p_dt_new_params->faceSearchEdgeMask.top,
        p_dt_new_params->faceSearchEdgeMask.right, p_dt_new_params->faceSearchEdgeMask.bottom,
        lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
    p_dt_current_params->faceSearchEdgeMask = p_dt_new_params->faceSearchEdgeMask;
  }

  if (p_dt_current_params->nSearchDensity != p_dt_new_params->nSearchDensity) {
    lib_ret = p_dt_funcs->FACEPROC_DT_SetSearchDensity(p_hnd->hdt, p_dt_new_params->nSearchDensity);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_SetSearchDensity(%d), lib_ret=%d", p_dt_new_params->nSearchDensity, lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
    p_dt_current_params->nSearchDensity = p_dt_new_params->nSearchDensity;
  }

  if (p_dt_current_params->nThreshold != p_dt_new_params->nThreshold) {
    lib_ret = p_dt_funcs->FACEPROC_DT_SetThreshold(p_hnd->hdt, p_dt_new_params->nThreshold);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in DT_SetThreshold(%d), lib_ret=%d", p_dt_new_params->nThreshold, lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
    p_dt_current_params->nThreshold = p_dt_new_params->nThreshold;
  }

  if ((p_hnd->create_params.detection_mode == DETECTION_MODE_MOVIE) ||
    (p_hnd->create_params.detection_mode == CUSTOM_MODE_PARTIAL_STILL)) {

    if ((p_dt_current_params->nInitialFaceSearchCycle != p_dt_new_params->nInitialFaceSearchCycle) ||
        (p_dt_current_params->nNewFaceSearchCycle     != p_dt_new_params->nNewFaceSearchCycle)     ||
        (p_dt_current_params->nNewFaceSearchInterval  != p_dt_new_params->nNewFaceSearchInterval)) {
      lib_ret = p_dt_funcs->FACEPROC_DT_MV_SetSearchCycle(p_hnd->hdt,
                                                          p_dt_new_params->nInitialFaceSearchCycle,
                                                          p_dt_new_params->nNewFaceSearchCycle,
                                                          p_dt_new_params->nNewFaceSearchInterval);
      if (lib_ret != FACEPROC_NORMAL) {
        IDBG_ERROR("Failed in DT_MV_SetSearchCycle(%d, %d, %d), lib_ret=%d",
          p_dt_new_params->nInitialFaceSearchCycle, p_dt_new_params->nNewFaceSearchCycle,
          p_dt_new_params->nNewFaceSearchInterval, lib_ret);
        return fpsww_liberr_to_imgerror(lib_ret);
      }
      p_dt_current_params->nInitialFaceSearchCycle = p_dt_new_params->nInitialFaceSearchCycle;
      p_dt_current_params->nNewFaceSearchCycle     = p_dt_new_params->nNewFaceSearchCycle;
      p_dt_current_params->nNewFaceSearchInterval  = p_dt_new_params->nNewFaceSearchInterval;
    }
  }

  if (p_hnd->create_params.detection_mode == DETECTION_MODE_MOVIE) {
    if ((p_dt_current_params->nMaxRetryCount != p_dt_new_params->nMaxRetryCount) ||
        (p_dt_current_params->nMaxHoldCount  != p_dt_new_params->nMaxHoldCount)) {
      lib_ret = p_dt_funcs->FACEPROC_DT_MV_SetLostParam(p_hnd->hdt,
                                                        p_dt_new_params->nMaxRetryCount, p_dt_new_params->nMaxHoldCount);
      if (lib_ret != FACEPROC_NORMAL) {
        IDBG_ERROR("Failed in FACEPROC_DT_MV_SetLostParam(%d, %d), lib_ret=%d",
          p_dt_new_params->nMaxRetryCount, p_dt_new_params->nMaxHoldCount, lib_ret);
        return fpsww_liberr_to_imgerror(lib_ret);
      }
      p_dt_current_params->nMaxRetryCount = p_dt_new_params->nMaxRetryCount;
      p_dt_current_params->nMaxHoldCount  = p_dt_new_params->nMaxHoldCount;
    }

    if ((p_dt_current_params->nPosSteadinessParam  != p_dt_new_params->nPosSteadinessParam) ||
        (p_dt_current_params->nSizeSteadinessParam != p_dt_new_params->nSizeSteadinessParam)) {
      lib_ret = p_dt_funcs->FACEPROC_DT_MV_SetSteadinessParam(p_hnd->hdt,
                                                              p_dt_new_params->nPosSteadinessParam,
                                                              p_dt_new_params->nSizeSteadinessParam);
      if (lib_ret != FACEPROC_NORMAL) {
        IDBG_ERROR("Failed in DT_MV_SetSteadinessParam(%d, %d), lib_ret=%d",
          p_dt_new_params->nPosSteadinessParam, p_dt_new_params->nSizeSteadinessParam, lib_ret);
        return fpsww_liberr_to_imgerror(lib_ret);
      }
      p_dt_current_params->nPosSteadinessParam  = p_dt_new_params->nPosSteadinessParam;
      p_dt_current_params->nSizeSteadinessParam = p_dt_new_params->nSizeSteadinessParam;
    }

    if (p_dt_current_params->nTrackingSwapParam != p_dt_new_params->nTrackingSwapParam) {
      lib_ret = p_dt_funcs->FACEPROC_DT_MV_SetTrackingSwapParam(p_hnd->hdt, p_dt_new_params->nTrackingSwapParam);
      if (lib_ret != FACEPROC_NORMAL) {
        IDBG_ERROR("Failed in DT_MV_SetTrackingSwapParam(%d), lib_ret=%d", p_dt_new_params->nTrackingSwapParam, lib_ret);
        return fpsww_liberr_to_imgerror(lib_ret);
      }
      p_dt_current_params->nTrackingSwapParam = p_dt_new_params->nTrackingSwapParam;
    }

    if (p_dt_current_params->nDelayCount != p_dt_new_params->nDelayCount) {
      lib_ret = p_dt_funcs->FACEPROC_DT_MV_SetDelayCount(p_hnd->hdt, p_dt_new_params->nDelayCount);
      if (lib_ret != FACEPROC_NORMAL) {
        IDBG_ERROR("Failed in DT_MV_SetDelayCount(%d), lib_ret=%d", p_dt_new_params->nDelayCount, lib_ret);
        return fpsww_liberr_to_imgerror(lib_ret);
      }
      p_dt_current_params->nDelayCount = p_dt_new_params->nDelayCount;
    }

    if ((p_dt_current_params->trackingEdgeMask.left   != p_dt_new_params->trackingEdgeMask.left)  ||
        (p_dt_current_params->trackingEdgeMask.top    != p_dt_new_params->trackingEdgeMask.top)   ||
        (p_dt_current_params->trackingEdgeMask.right  != p_dt_new_params->trackingEdgeMask.right) ||
        (p_dt_current_params->trackingEdgeMask.bottom != p_dt_new_params->trackingEdgeMask.bottom)) {
      lib_ret = p_dt_funcs->FACEPROC_DT_MV_SetTrackingEdgeMask(p_hnd->hdt, p_dt_new_params->trackingEdgeMask);
      if (lib_ret != FACEPROC_NORMAL) {
        IDBG_ERROR("Failed in FACEPROC_DT_MV_SetTrackingEdgeMask(%d, %d, %d, %d), lib_ret=%d",
          p_dt_new_params->trackingEdgeMask.left, p_dt_new_params->trackingEdgeMask.top,
          p_dt_new_params->trackingEdgeMask.right, p_dt_new_params->trackingEdgeMask.bottom,
          lib_ret);
        return fpsww_liberr_to_imgerror(lib_ret);
      }
      p_dt_current_params->trackingEdgeMask = p_dt_new_params->trackingEdgeMask;
    }

    if (p_dt_current_params->nAccuracy != p_dt_new_params->nAccuracy) {
      lib_ret = p_dt_funcs->FACEPROC_DT_MV_SetAccuracy(p_hnd->hdt, p_dt_new_params->nAccuracy);
      if (lib_ret != FACEPROC_NORMAL) {
        IDBG_ERROR("Failed in DT_MV_SetAccuracy(%d), lib_ret=%d", p_dt_new_params->nAccuracy, lib_ret);
        return fpsww_liberr_to_imgerror(lib_ret);
      }
      p_dt_current_params->nAccuracy = p_dt_new_params->nAccuracy;
    }

    if (p_dt_current_params->rollAngleExtension != p_dt_new_params->rollAngleExtension) {
      lib_ret = p_dt_funcs->FACEPROC_DT_MV_SetAngleExtension(p_hnd->hdt, p_dt_new_params->rollAngleExtension);
      if (lib_ret != FACEPROC_NORMAL) {
        IDBG_ERROR("Failed in DT_MV_SetAngleExtension(%d), lib_ret=%d", p_dt_new_params->rollAngleExtension, lib_ret);
        return fpsww_liberr_to_imgerror(lib_ret);
      }
      p_dt_current_params->rollAngleExtension = p_dt_new_params->rollAngleExtension;
    }

    if ((p_dt_current_params->yawAngleExtension  != p_dt_new_params->yawAngleExtension) ||
        (p_dt_current_params->bUseHeadTracking   != p_dt_new_params->bUseHeadTracking)) {
      lib_ret = p_dt_funcs->FACEPROC_DT_MV_SetPoseExtension(p_hnd->hdt,
                                                            p_dt_new_params->yawAngleExtension,
                                                            p_dt_new_params->bUseHeadTracking);
      if (lib_ret != FACEPROC_NORMAL) {
        IDBG_ERROR("Failed in DT_MV_SetPoseExtension(%d, %d), lib_ret=%d",
          p_dt_new_params->yawAngleExtension, p_dt_new_params->bUseHeadTracking, lib_ret);
        return fpsww_liberr_to_imgerror(lib_ret);
      }
      p_dt_current_params->yawAngleExtension = p_dt_new_params->yawAngleExtension;
      p_dt_current_params->bUseHeadTracking  = p_dt_new_params->bUseHeadTracking;
    }

    if (p_dt_current_params->bMask != p_dt_new_params->bMask) {
      lib_ret = p_dt_funcs->FACEPROC_DT_MV_SetDirectionMask(p_hnd->hdt, p_dt_new_params->bMask);
      if (lib_ret != FACEPROC_NORMAL) {
        IDBG_ERROR("Failed in DT_MV_SetDirectionMask(%d), lib_ret=%d", p_dt_new_params->bMask, lib_ret);
        return fpsww_liberr_to_imgerror(lib_ret);
      }
      p_dt_current_params->bMask = p_dt_new_params->bMask;
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_pt_set_config
 *
 * Description: Set PT config params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_pt_new_params - Pointer to PT config params which need to be set to sw library
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_pt_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_pt_params_t *p_pt_new_params)
{
  if (!p_hnd->hpt) {
    IDBG_MED("PT is disabled, handle is not created");
    return IMG_SUCCESS;
  }

  fpsww_pt_fptr_t   *p_pt_funcs          = &p_hnd->p_lib->pt_funcs;
  fpsww_pt_params_t *p_pt_current_params = &p_hnd->config.pt_params;
  INT32              lib_ret             = FACEPROC_NORMAL;

  if (p_pt_current_params->nMode != p_pt_new_params->nMode) {
    lib_ret = p_pt_funcs->FACEPROC_PT_SetMode(p_hnd->hpt, p_pt_new_params->nMode);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in PT_GetMode, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
    p_pt_current_params->nMode = p_pt_new_params->nMode;
  }

  if (p_pt_current_params->nConfMode != p_pt_new_params->nConfMode) {
    lib_ret = p_pt_funcs->FACEPROC_PT_SetConfMode(p_hnd->hpt, p_pt_new_params->nConfMode);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in PT_GetConfMode, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
    p_pt_current_params->nConfMode = p_pt_new_params->nConfMode;
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_ct_set_config
 *
 * Description: Set CT config params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_ct_new_params - Pointer to CT config params which need to be set to sw library
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_ct_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_ct_params_t *p_ct_new_params)
{
  if (!p_hnd->hct) {
    IDBG_MED("CT is disabled, handle is not created");
    return IMG_SUCCESS;
  }

  fpsww_ct_fptr_t   *p_ct_funcs          = &p_hnd->p_lib->ct_funcs;
  fpsww_ct_params_t *p_ct_current_params = &p_hnd->config.ct_params;
  INT32              lib_ret             = FACEPROC_NORMAL;

  if (p_ct_current_params->nMode != p_ct_new_params->nMode) {
    lib_ret = p_ct_funcs->FACEPROC_CT_SetDetectionMode(p_hnd->hct, p_ct_new_params->nMode);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in CT_SetDetectionMode, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
    p_ct_current_params->nMode = p_ct_new_params->nMode;
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_sm_set_config
 *
 * Description: Set SM config params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_sm_params - Pointer to SM config params which need to be set to sw library
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 *     No tunable configuration params currently
 **/
int fpsww_sm_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_sm_params_t *p_sm_params)
{
  //nothing to set now
  return IMG_SUCCESS;
}

/**
 * Function: fpsww_gb_set_config
 *
 * Description: Set GB config params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_gb_params - Pointer to GB config params which need to be set to sw library
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 *     No tunable configuration params currently
 **/
int fpsww_gb_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_gb_params_t *p_gb_params)
{
  //nothing to set now
  return IMG_SUCCESS;
}

/**
 * Function: fpsww_fr_set_config
 *
 * Description: Set FR config params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_fr_params - Pointer to FR config params which need to be set to sw library
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 *     No tunable configuration params currently
 **/
int fpsww_fr_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_fr_params_t *p_fr_params)
{
  //nothing to set now
  return IMG_SUCCESS;
}

/**
 * Function: fpsww_co_print_config
 *
 * Description: Print current CO config params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_co_params - Pointer to CO params structure
 *
 * Return values:
 *     IMG_SUCCESS on success
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_co_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_co_params_t *p_co_params)
{
  IDBG_INFO("========================= CO Config =========================");
  IDBG_INFO("Version : %d.%d",  p_co_params->majorVersion, p_co_params->minorVersion);

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dt_print_config
 *
 * Description: Print current DTconfig params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_dt_params - Pointer to DT params structure
 *
 * Return values:
 *     IMG_SUCCESS on success
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_dt_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_dt_params_t *p_dt_params)
{
  IDBG_INFO("========================= DT Config =========================");
  IDBG_INFO("Version                   : %d.%d",  p_dt_params->majorVersion, p_dt_params->minorVersion);
  IDBG_INFO("Min, Max face sizes       : %d, %d", p_dt_params->nMinSize, p_dt_params->nMaxSize);
  IDBG_INFO("POSE_ANGLES               : 0x%x (POSE_ANGLE_FRONT), 0x%x (POSE_ANGLE_HALF_PROFILE), 0x%x (POSE_ANGLE_PROFILE)",
                                              p_dt_params->nAngle[FPSWW_ANGLE_FRONT],
                                              p_dt_params->nAngle[FPSWW_ANGLE_HALFPROFILE],
                                              p_dt_params->nAngle[FPSWW_ANGLE_FULLPROFILE]);
  IDBG_INFO("FaceSearch EdgeMask       : left=%d, top=%d, right=%d, bottom=%d",
                                              p_dt_params->faceSearchEdgeMask.left, p_dt_params->faceSearchEdgeMask.top,
                                              p_dt_params->faceSearchEdgeMask.right, p_dt_params->faceSearchEdgeMask.bottom);
  IDBG_INFO("Search Density            : %d", p_dt_params->nSearchDensity);
  IDBG_INFO("Threshold                 : %d", p_dt_params->nThreshold);

  if ((p_hnd->create_params.detection_mode == DETECTION_MODE_MOVIE) ||
    (p_hnd->create_params.detection_mode == CUSTOM_MODE_PARTIAL_STILL)) {
    IDBG_INFO("MV InitialFaceSearchCycle : %d, NewFaceSearchCycle=%d, nNewFaceSearchInterval=%d",
                                                p_dt_params->nInitialFaceSearchCycle,
                                                p_dt_params->nNewFaceSearchCycle,
                                                p_dt_params->nNewFaceSearchInterval);
  }

  if (p_hnd->create_params.detection_mode == DETECTION_MODE_MOVIE) {
    IDBG_INFO("MV MaxRetryCount          : %d", p_dt_params->nMaxRetryCount);
    IDBG_INFO("MV MaxHoldCount           : %d", p_dt_params->nMaxHoldCount);
    IDBG_INFO("MV SteadinessParam        : %d (Position), %d (Size)",
                                                p_dt_params->nPosSteadinessParam,
                                                p_dt_params->nSizeSteadinessParam);
    IDBG_INFO("MV TrackingSwapParam      : %d", p_dt_params->nTrackingSwapParam);
    IDBG_INFO("MV DelayCount             : %d", p_dt_params->nDelayCount);
    IDBG_INFO("MV Tracking EdgeMask      : left=%d, top=%d, right=%d, bottom=%d",
                                                p_dt_params->trackingEdgeMask.left, p_dt_params->trackingEdgeMask.top,
                                                p_dt_params->trackingEdgeMask.right, p_dt_params->trackingEdgeMask.bottom);
    IDBG_INFO("MV Accuracy               : %d", p_dt_params->nAccuracy);
    IDBG_INFO("MV RollAngleExtension     : %d", p_dt_params->rollAngleExtension);
    IDBG_INFO("MV yawAngleExtension      : %d", p_dt_params->yawAngleExtension);
    IDBG_INFO("MV bUseHeadTracking       : %d", p_dt_params->bUseHeadTracking);
    IDBG_INFO("MV Mask                   : %d", p_dt_params->bMask);
  }
  return IMG_SUCCESS;
}

/**
 * Function: fpsww_pt_print_config
 *
 * Description: Print current PTconfig params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_pt_params - Pointer to PT params structure
 *
 * Return values:
 *     IMG_SUCCESS on success
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_pt_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_pt_params_t *p_pt_params)
{
  if (!p_hnd->hpt) {
    IDBG_MED("PT is disabled, handle is not created");
    return IMG_SUCCESS;
  }

  IDBG_INFO("========================= PT Config =========================");
  IDBG_INFO("Version         : %d.%d", p_pt_params->majorVersion, p_pt_params->minorVersion);
  IDBG_INFO("Mode            : %d",    p_pt_params->nMode);
  IDBG_INFO("Confidence Mode : %d",    p_pt_params->nConfMode);

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_ct_print_config
 *
 * Description: Print current CTconfig params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_ct_params - Pointer to CT params structure
 *
 * Return values:
 *     IMG_SUCCESS on success
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_ct_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_ct_params_t *p_ct_params)
{
  if (!p_hnd->hct) {
    IDBG_MED("CT is disabled, handle is not created");
    return IMG_SUCCESS;
  }

  IDBG_INFO("========================= CT Config =========================");
  IDBG_INFO("Version : %d.%d", p_ct_params->majorVersion, p_ct_params->minorVersion);
  IDBG_INFO("Mode    : %d",    p_ct_params->nMode);

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_sm_print_config
 *
 * Description: Print current SM config params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_sm_params - Pointer to SM params structure
 *
 * Return values:
 *     IMG_SUCCESS on success
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_sm_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_sm_params_t *p_sm_params)
{
  if (!p_hnd->hsm) {
    IDBG_MED("SM is disabled, handle is not created");
    return IMG_SUCCESS;
  }

  IDBG_INFO("========================= SM Config =========================");
  IDBG_INFO("Version : %d.%d",  p_sm_params->majorVersion, p_sm_params->minorVersion);

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_gb_print_config
 *
 * Description: Print current GB config params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_gb_params - Pointer to GB params structure
 *
 * Return values:
 *     IMG_SUCCESS on success
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_gb_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_gb_params_t *p_gb_params)
{
  if (!p_hnd->hgb) {
    IDBG_MED("GB is disabled, handle is not created");
    return IMG_SUCCESS;
  }

  IDBG_INFO("========================= GB Config =========================");
  IDBG_INFO("Version : %d.%d",  p_gb_params->majorVersion, p_gb_params->minorVersion);

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_fr_print_config
 *
 * Description: Print current FR config params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_fr_params - Pointer to FR params structure
 *
 * Return values:
 *     IMG_SUCCESS on success
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_fr_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_fr_params_t *p_fr_params)
{
  if (!p_hnd->hfeature) {
    IDBG_MED("FR is disabled, handle is not created");
    return IMG_SUCCESS;
  }

  IDBG_INFO("========================= FR Config =========================");
  IDBG_INFO("Version : %d.%d",  p_fr_params->majorVersion, p_fr_params->minorVersion);

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dt_execute
 *
 * Description: Execute face detection.
 *     This will only execute the detection and gets the number of faces detected. Doesn't get
 *     the faceinfo results for faces. Call fpsww_dt_get_result_info for getting faceinfo.
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_frame - pointer to current img_frame
 *   get_results_only - when TRUE, gets only the result (num of faces), doesn't execute detection
 *   p_num_faces - If not NULL, this function saves the number of faces detected into the pointer.
 *
 * Return values:
 *     IMG_SUCCESS on success
 *
 * Notes:
 *     Caller has to make sure the required incoming args are not NULL
 **/
int fpsww_dt_execute(faceproc_sw_wrap_t *p_hnd, img_frame_t *p_frame,
  bool get_results_only, int32_t *p_num_faces)
{
  if (!p_hnd->hdt || !p_hnd->hdtresult) {
    IDBG_ERROR("Invalid handles p_hnd->hdt=%p, p_hnd->hdtresult=%p", p_hnd->hdt, p_hnd->hdtresult);
    return IMG_ERR_INVALID_INPUT;
  }

  // If DSP use fpsww_dsp_dt_execute
  if (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) {
    return fpsww_dsp_dt_execute(p_hnd, p_frame, get_results_only, p_num_faces);
  }

  fpsww_dt_fptr_t *p_dt_funcs = &p_hnd->p_lib->dt_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  // Execute DT if caller doesn't set get_results_only
  if (get_results_only == FALSE) {
    if (!p_frame) {
      IDBG_ERROR("NULL p_frame");
      return IMG_ERR_INVALID_INPUT;
    }

    if ((p_frame->frame[0].plane_cnt == 1) && (p_frame->frame[0].plane[0].plane_type == PLANE_Y)) {
      // Y
      lib_ret = p_dt_funcs->FACEPROC_DT_Detect_GRAY(p_hnd->hdt, (RAWIMAGE *)IMG_ADDR(p_frame),
                                                    (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
                                                    GRAY_ORDER_Y0Y1Y2Y3, p_hnd->hdtresult);
    } else if (p_frame->frame[0].plane_cnt == 1) {
      // YUV422 formats
      YUV422_ORDER yuv_order;
      switch (p_frame->frame[0].plane[0].plane_type) {
        case PLANE_Y_CB_Y_CR : yuv_order = YUV422_ORDER_Y0CBY1CR; break;
        case PLANE_Y_CR_Y_CB : yuv_order = YUV422_ORDER_Y0CRY1CB; break;
        case PLANE_CB_Y_CR_Y : yuv_order = YUV422_ORDER_CBY0CRY1; break;
        case PLANE_CR_Y_CB_Y : yuv_order = YUV422_ORDER_CRY0CBY1; break;
        default : return IMG_ERR_NOT_SUPPORTED;
      }

      lib_ret = p_dt_funcs->FACEPROC_DT_Detect_YUV422(p_hnd->hdt, (RAWIMAGE *)IMG_ADDR(p_frame),
                                                      (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
                                                      yuv_order, p_hnd->hdtresult);
    } else if (p_frame->frame[0].plane_cnt == 2) {
      // YUV420 semiplanar formats
      YUV420SP_ORDER yuv_order;
      // check the U, V positions in UV plane.
      switch (p_frame->frame[0].plane[1].plane_type) {
        case PLANE_CB_CR : yuv_order = YUV420SP_ORDER_Y0Y1Y2Y3_CBCR; break;
        case PLANE_CR_CB : yuv_order = YUV420SP_ORDER_Y0Y1Y2Y3_CRCB; break;
        default : return IMG_ERR_NOT_SUPPORTED;
      }

      lib_ret = p_dt_funcs->FACEPROC_DT_Detect_YUV420SP(p_hnd->hdt,
                                                        (RAWIMAGE *)IMG_PLANE_ADDR(p_frame, 0),
                                                        (RAWIMAGE *)IMG_PLANE_ADDR(p_frame, 1),
                                                        (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
                                                        yuv_order, p_hnd->hdtresult);
    } else if (p_frame->frame[0].plane_cnt == 3) {
      // YUV420 full planar formats, supports only Y_Cb_Cr. faceproc lib doesn't support Y_Cr_Cb
      YUV420FP_ORDER yuv_order;
      switch (p_frame->frame[0].plane[1].plane_type) {
        case PLANE_CB : yuv_order = YUV420FP_ORDER_Y0Y1Y2Y3_CB_CR; break;
        default : return IMG_ERR_NOT_SUPPORTED;
      }

      lib_ret = p_dt_funcs->FACEPROC_DT_Detect_YUV420FP(p_hnd->hdt,
                                                        (RAWIMAGE *)IMG_PLANE_ADDR(p_frame, 0),
                                                        (RAWIMAGE *)IMG_PLANE_ADDR(p_frame, 1),
                                                        (RAWIMAGE *)IMG_PLANE_ADDR(p_frame, 2),
                                                        (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
                                                        yuv_order, p_hnd->hdtresult);
    } else {
      return IMG_ERR_NOT_SUPPORTED;
    }

    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC Detection returned error: %d, plane_cnt = %d, plane0 type=%d",
        lib_ret, p_frame->frame[0].plane_cnt, p_frame->frame[0].plane[0].plane_type);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  if (p_num_faces) {
    *p_num_faces = 0;

    lib_ret = p_dt_funcs->FACEPROC_DT_GetResultCount(p_hnd->hdtresult, p_num_faces);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_DT_GetResultCount returned error: %d", lib_ret);
      *p_num_faces = 0;
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dt_get_result_info
 *
 * Description: Gets the face detection results info. Caller must call fpsww_dt_execute before
 *     calling this function. This will only get the results information of previously ran face detection.
 *     This function needs to be called multiple times for each face to get the information of each face.
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_face_info - face information to be filled
 *   face_index - face index for which the results are asked for.
 *   raw_result - Whether to get raw results information for this face.
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the required incoming args are not NULL
 **/
int fpsww_dt_get_result_info(faceproc_sw_wrap_t *p_hnd, DETECTION_INFO *p_face_info, int32_t face_index, bool raw_result)
{
  if (!p_hnd->hdt || !p_hnd->hdtresult || !p_face_info) {
    IDBG_ERROR("Invalid handles p_hnd->hdt=%p, p_hnd->hdtresult=%p", p_hnd->hdt, p_hnd->hdtresult);
    return IMG_ERR_INVALID_INPUT;
  }

  fpsww_dt_fptr_t *p_dt_funcs = (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) ?
                                &p_hnd->p_lib->dsp_dt_funcs : &p_hnd->p_lib->dt_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  if (raw_result == FALSE) {
    lib_ret = p_dt_funcs->FACEPROC_DT_GetResultInfo(p_hnd->hdtresult, face_index, p_face_info);
  } else {
    lib_ret = p_dt_funcs->FACEPROC_DT_GetRawResultInfo(p_hnd->hdtresult, face_index, p_face_info);
  }

  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_DT_GetResultInfo returned error: %d, raw_result=%d", lib_ret, raw_result);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_pt_execute_from_dthandle
 *
 * Description: Execute PT and get results.
 *     This function can be called in different ways to get the desired behavior:
 *       1. To execute only : Call this function with get_results_only=FALSE, p_pt_results=NULL
 *       2. To get results info only for already executed results : Call with get_results_only=TRUE, p_pt_results=non NULL
 *       3. To execute + get results at the same time : Call this function with get_results_only=FALSE, p_pt_results=non NULL
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_frame - pointer to current img_frame
 *   face_index - face index
 *   get_results_only - Whether to get results only for the previously executed PT.
 *   p_pt_results - PT results to be filled in.
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the required incoming args are not NULL
 **/
int fpsww_pt_execute_from_dthandle(faceproc_sw_wrap_t *p_hnd, img_frame_t *p_frame, int32_t face_index,
  bool get_results_only, face_part_detect *p_pt_results)
{
  if (!p_hnd->hpt) {
    IDBG_HIGH("PT handle not created, PT not enabled");
    return IMG_SUCCESS;
  }

  if (!p_hnd->hdtresult) {
    IDBG_HIGH("Invalid DT Results handle");
    return IMG_ERR_INVALID_INPUT;
  }

  fpsww_pt_fptr_t *p_pt_funcs = &p_hnd->p_lib->pt_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  if (get_results_only == FALSE) {
    if (!p_frame) {
      IDBG_ERROR("p_frame is NULL");
      return IMG_ERR_INVALID_INPUT;
    }

    lib_ret = p_pt_funcs->FACEPROC_PT_SetPositionFromHandle(p_hnd->hpt, p_hnd->hdtresult, face_index);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_PT_SetPositionFromHandle returned error:%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_pt_funcs->FACEPROC_PT_DetectPoint(p_hnd->hpt, (RAWIMAGE *)IMG_ADDR(p_frame),
                                                  (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
                                                  p_hnd->hptresult[face_index]);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_PT_DetectPoint returned error: %d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  // Get the results here itself if p_pt_results
  if (p_pt_results) {
    memset(p_pt_results, 0x00, sizeof(face_part_detect));

    lib_ret = p_pt_funcs->FACEPROC_PT_GetResult(p_hnd->hptresult[face_index], PT_POINT_KIND_MAX,
                                                (POINT *)p_pt_results->face_pt, p_pt_results->confidence);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("PT_GetResult failed, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_pt_funcs->FACEPROC_PT_GetFaceDirection(p_hnd->hptresult[face_index],
                                                       &(p_pt_results->direction_up_down),
                                                       &(p_pt_results->direction_left_right),
                                                       &(p_pt_results->direction_roll));
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_PT_GetFaceDirection failed, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_pt_execute_from_positionIP
 *
 * Description: Execute PT based on positionIP and get results.
 *     This function can be called in different ways to get the desired behavior:
 *       1. To execute only : Call this function with get_results_only=FALSE, p_pt_results=NULL
 *       2. To get results info only for already executed results : Call with get_results_only=TRUE, p_pt_results=non NULL
 *       3. To execute + get results at the same time : Call this function with get_results_only=FALSE, p_pt_results=non NULL
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_frame - pointer to current img_frame
 *   center - center of the face
 *   size - size of the face
 *   face_angle_roll - face_angle_roll of the face
 *   scale - scale of the face
 *   pose - pose of the face
 *   get_results_only - Whether to get results only for the previously executed PT.
 *   p_pt_results - PT results to be filled in.
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the required incoming args are not NULL
 **/
int fpsww_pt_execute_from_positionIP(faceproc_sw_wrap_t *p_hnd, img_frame_t *p_frame, int32_t face_index,
  POINT center, uint32_t size, int face_angle_roll, int32_t scale, int32_t pose,
  bool get_results_only, face_part_detect *p_pt_results)
{
  if (!p_hnd->hpt) {
    IDBG_HIGH("PT handle not created, PT not enabled");
    return IMG_SUCCESS;
  }

  fpsww_pt_fptr_t *p_pt_funcs = &p_hnd->p_lib->pt_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  if (get_results_only == FALSE) {
    if (!p_frame) {
      IDBG_ERROR("p_frame is NULL");
      return IMG_ERR_INVALID_INPUT;
    }

    lib_ret = p_pt_funcs->FACEPROC_PT_SetPositionIP(p_hnd->hpt, center.x, center.y,
                                                    size, face_angle_roll, scale, pose, DTVERSION_UNKNOWN);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_PT_SetPositionIP returned error:%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_pt_funcs->FACEPROC_PT_DetectPoint(p_hnd->hpt, (RAWIMAGE *)IMG_ADDR(p_frame),
                                                  (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
                                                  p_hnd->hptresult[face_index]);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_PT_DetectPoint returned error: %d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  // Get the results here itself if p_pt_results
  if (p_pt_results) {
    memset(p_pt_results, 0x00, sizeof(face_part_detect));

    lib_ret = p_pt_funcs->FACEPROC_PT_GetResult(p_hnd->hptresult[face_index], PT_POINT_KIND_MAX,
                                                (POINT *)p_pt_results->face_pt, p_pt_results->confidence);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("PT_GetResult failed, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_pt_funcs->FACEPROC_PT_GetFaceDirection(p_hnd->hptresult[face_index],
                                                       &(p_pt_results->direction_up_down),
                                                       &(p_pt_results->direction_left_right),
                                                       &(p_pt_results->direction_roll));
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_PT_GetFaceDirection failed, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_ct_execute_from_pthandle
 *
 * Description: Execute CT and get results.
 *     This function can be called in different ways to get the desired behavior:
 *       1. To execute only : Call this function with get_results_only=FALSE, p_ct_results=NULL
 *       2. To get results info only for already executed results : Call with get_results_only=TRUE, p_ct_results=non NULL
 *       3. To execute + get results at the same time : Call this function with get_results_only=FALSE, p_ct_results=non NULL
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_frame - pointer to current img_frame
 *   face_index - face index
 *   get_results_only - Whether to get results only for the previously executed PT.
 *   p_ct_results - CT results to be filled in.
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the required incoming args are not NULL
 **/
int fpsww_ct_execute_from_pthandle(faceproc_sw_wrap_t *p_hnd, img_frame_t *p_frame, int32_t face_index,
  bool get_results_only, fd_contour_results *p_ct_results)
{
  if (!p_hnd->hct) {
    IDBG_HIGH("CT handle not created, not enabled");
    return IMG_SUCCESS;
  }

  if (!p_hnd->hptresult[face_index]) {
    IDBG_HIGH("PT Results handle %d not created", face_index);
    return IMG_ERR_INVALID_INPUT;
  }

  fpsww_ct_fptr_t *p_ct_funcs = &p_hnd->p_lib->ct_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  if (get_results_only == FALSE) {
    if (!p_frame) {
      IDBG_ERROR("p_frame is NULL");
      return IMG_ERR_INVALID_INPUT;
    }

    lib_ret = p_ct_funcs->FACEPROC_CT_SetPointFromHandle(p_hnd->hct, p_hnd->hptresult[face_index]);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_CT_SetPositionFromHandle returned error:%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_ct_funcs->FACEPROC_CT_DetectContour(p_hnd->hct, (RAWIMAGE *)IMG_ADDR(p_frame),
                                                    (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
                                                    p_hnd->hctresult[face_index]);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_CT_DetectContour returned error: %d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  if (p_ct_results) {
    memset(p_ct_results, 0x00, sizeof(fd_contour_results));

    lib_ret = p_ct_funcs->FACEPROC_CT_GetResult(p_hnd->hctresult[face_index], 
                                                CT_POINT_KIND_MAX, (POINT *)p_ct_results->contour_pt);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("CT_GetResult failed");
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_sm_execute_from_pthandle
 *
 * Description: Execute SM and get results.
 *     This function can be called in different ways to get the desired behavior:
 *       1. To execute only : Call this function with get_results_only=FALSE, p_sm_results=NULL
 *       2. To get results info only for already executed results : Call with get_results_only=TRUE, p_sm_results=non NULL
 *       3. To execute + get results at the same time : Call this function with get_results_only=FALSE, p_sm_results=non NULL
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_frame - pointer to current img_frame
 *   face_index - face index
 *   get_results_only - Whether to get results only for the previously executed PT.
 *   p_sm_results - SM results to be filled in.
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the required incoming args are not NULL
 **/
int fpsww_sm_execute_from_pthandle(faceproc_sw_wrap_t *p_hnd, img_frame_t *p_frame, int32_t face_index,
  bool get_results_only, fd_smile_detect *p_sm_results)
{
  if (!p_hnd->hsm) {
    IDBG_HIGH("SM handle not created, not enabled");
    return IMG_SUCCESS;
  }

  if (!p_hnd->hptresult[face_index]) {
    IDBG_HIGH("PT Results handle %d not created", face_index);
    return IMG_ERR_INVALID_INPUT;
  }

  fpsww_sm_fptr_t *p_sm_funcs = &p_hnd->p_lib->sm_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  if (get_results_only == FALSE) {
    if (!p_frame) {
      IDBG_ERROR("p_frame is NULL");
      return IMG_ERR_INVALID_INPUT;
    }

    lib_ret = p_sm_funcs->FACEPROC_SM_SetPointFromHandle(p_hnd->hsm, p_hnd->hptresult[face_index]);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_SM_SetPositionFromHandle returned error:%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_sm_funcs->FACEPROC_SM_Estimate(p_hnd->hsm, (RAWIMAGE *)IMG_ADDR(p_frame),
                                               (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
                                               p_hnd->hsmresult[face_index]);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_SM_Estimate returned error: %d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  if (p_sm_results) {
    p_sm_results->smile_degree = 0;
    p_sm_results->confidence   = 0;

    lib_ret = p_sm_funcs->FACEPROC_SM_GetResult(p_hnd->hsmresult[face_index],
                                                &p_sm_results->smile_degree, &p_sm_results->confidence);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("SM_GetResult failed");
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_gb_execute_from_pthandle
 *
 * Description: Execute GB and get results.
 *     This function can be called in different ways to get the desired behavior:
 *       1. To execute only : Call this function with get_results_only=FALSE, p_gb_results=NULL
 *       2. To get results info only for already executed results : Call with get_results_only=TRUE, p_gb_results=non NULL
 *       3. To execute + get results at the same time : Call this function with get_results_only=FALSE, p_gb_results=non NULL
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_frame - pointer to current img_frame
 *   face_index - face index
 *   get_results_only - Whether to get results only for the previously executed PT.
 *   p_gb_results - GB results to be filled in.
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the required incoming args are not NULL
 **/
int fpsww_gb_execute_from_pthandle(faceproc_sw_wrap_t *p_hnd, img_frame_t *p_frame, int32_t face_index,
  bool get_results_only, fd_gb_result *p_gb_results)
{
  if (!p_hnd->hgb) {
    IDBG_HIGH("GB handle not created, not enabled");
    return IMG_SUCCESS;
  }

  if (!p_hnd->hptresult[face_index]) {
    IDBG_HIGH("PT Results handle %d not created", face_index);
    return IMG_ERR_INVALID_INPUT;
  }

  fpsww_gb_fptr_t *p_gb_funcs = &p_hnd->p_lib->gb_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  if (get_results_only == FALSE) {
    if (!p_frame) {
      IDBG_ERROR("p_frame is NULL");
      return IMG_ERR_INVALID_INPUT;
    }

    lib_ret = p_gb_funcs->FACEPROC_GB_SetPointFromHandle(p_hnd->hgb, p_hnd->hptresult[face_index]);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_GB_SetPositionFromHandle returned error:%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_gb_funcs->FACEPROC_GB_Estimate(p_hnd->hgb, (RAWIMAGE *)IMG_ADDR(p_frame),
                                               (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
                                               p_hnd->hgbresult[face_index]);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_GB_Estimate returned error: %d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  if (p_gb_results) {
    p_gb_results->left_blink      = 0;
    p_gb_results->right_blink     = 0;
    p_gb_results->left_right_gaze = 0;
    p_gb_results->top_bottom_gaze = 0;

    lib_ret = p_gb_funcs->FACEPROC_GB_GetEyeCloseRatio(p_hnd->hgbresult[face_index],
                                                       &p_gb_results->left_blink, &p_gb_results->right_blink);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("GB_GetResult failed");
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    lib_ret = p_gb_funcs->FACEPROC_GB_GetGazeDirection(p_hnd->hgbresult[face_index],
                                                       &p_gb_results->left_right_gaze, &p_gb_results->top_bottom_gaze);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("GB_GetGazeDirection failed");
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dt_reset_results
 *
 * Description: Reset DT results
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the required incoming args are not NULL
 **/
int fpsww_dt_reset_results(faceproc_sw_wrap_t *p_hnd)
{
  fpsww_dt_fptr_t *p_dt_funcs = (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) ?
                                &p_hnd->p_lib->dsp_dt_funcs : &p_hnd->p_lib->dt_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;

  lib_ret = p_dt_funcs->FACEPROC_DT_MV_ResetTracking(p_hnd->hdt);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in resetting DT Tracking");
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dt_lock_unlock_face
 *
 * Description: Lock/Unlock a face
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   faceID - face id to be locked or unlocked
 *   lock - Whether to lock or to unlcok
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the required incoming args are not NULL
 **/
int fpsww_dt_lock_unlock_face(faceproc_sw_wrap_t *p_hnd, INT32 faceID, bool lock)
{
  fpsww_dt_fptr_t *p_dt_funcs = (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) ?
                                &p_hnd->p_lib->dsp_dt_funcs : &p_hnd->p_lib->dt_funcs;
  INT32 lib_ret = FACEPROC_NORMAL;

  // if faceId > 0 means, its not locked.
  // if faceId < 0 means, its locked already

  if ((faceID > 0) && (lock == TRUE)) {
    // Face is not locked, lock is required. Toggle the lock
    lib_ret = p_dt_funcs->FACEPROC_DT_MV_ToggleTrackingLock(p_hnd->hdt, faceID);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("DT_MV_ToggleTrackingLock returned error: %d, faceID=%d", lib_ret, faceID);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  } else if ((faceID < 0) && (lock == FALSE)) {
    // Face is locked but unlock is required. Toggle the lock
    lib_ret = p_dt_funcs->FACEPROC_DT_MV_ToggleTrackingLock(p_hnd->hdt, faceID);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("DT_MV_ToggleTrackingLock returned error: %d, faceID=%d", lib_ret, faceID);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dt_is_face_locked
 *
 * Description: Checks whether the face with the given index is already locked or not.
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   face_index - face index
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the required incoming args are not NULL
 **/
int fpsww_dt_is_face_locked(faceproc_sw_wrap_t *p_hnd, int32_t face_index)
{
  if (!p_hnd->hdt || !p_hnd->hdtresult) {
    IDBG_ERROR("Invalid handles p_hnd->hdt=%p, p_hnd->hdtresult=%p", p_hnd->hdt, p_hnd->hdtresult);
    return FALSE;
  }

  fpsww_dt_fptr_t *p_dt_funcs = (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) ?
                                &p_hnd->p_lib->dsp_dt_funcs : &p_hnd->p_lib->dt_funcs;
  INT32            lib_ret    = FACEPROC_NORMAL;
  DETECTION_INFO   face_info;

  lib_ret = p_dt_funcs->FACEPROC_DT_GetResultInfo(p_hnd->hdtresult, face_index, &face_info);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_DT_GetResultInfo returned error: %d", lib_ret);
    return FALSE;
  }

  // nID would be negative if the face was locked
  if (face_info.nID < 0)
    return TRUE;
  else
    return FALSE;
}

/**
 * Function: fpsww_dsp_custom_load_fptrs
 *
 * Description: Load DSP CO function pointers
 *
 * Input parameters:
 *   p_lib_ptr - library handle
 *   p_co_funcs - CO fptr structure to hold the function pointers
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_NOT_FOUND if symbol is not found in the library
 *
 * Notes:
 *     Caller has to make sure the passed args are not NULL
 *     All the function pointers are must currently.
 **/
int fpsww_dsp_load_fptrs(void *p_lib_ptr, fpsww_dsp_fptr_t *p_dsp_funcs)
{
  IMG_DLSYM_MUST(p_lib_ptr, p_dsp_funcs, FDDSP_GetVersion,            "adsp_fd_FDDSP_GetVersion");
  IMG_DLSYM_MUST(p_lib_ptr, p_dsp_funcs, FDDSP_InitDetection,         "adsp_fd_FDDSP_InitDetection");
  IMG_DLSYM_MUST(p_lib_ptr, p_dsp_funcs, FDDSP_DeInitDetection,       "adsp_fd_FDDSP_DeInitDetection");

  IMG_DLSYM_MUST(p_lib_ptr, p_dsp_funcs, FDDSP_SetClockConfig,        "adsp_fd_FDDSP_SetClockConfig");
  IMG_DLSYM_MUST(p_lib_ptr, p_dsp_funcs, FDDSP_GetClockConfig,        "adsp_fd_FDDSP_GetClockConfig");

  IMG_DLSYM_MUST(p_lib_ptr, p_dsp_funcs, FACEPROC_DT_Detect_GRAY,     "adsp_fd_FACEPROC_DT_Detect_GRAY");
  IMG_DLSYM_MUST(p_lib_ptr, p_dsp_funcs, FACEPROC_DT_Detect_YUV422,   "adsp_fd_FACEPROC_DT_Detect_YUV422");
  IMG_DLSYM_MUST(p_lib_ptr, p_dsp_funcs, FACEPROC_DT_Detect_YUV420SP, "adsp_fd_FACEPROC_DT_Detect_YUV420SP");
  IMG_DLSYM_MUST(p_lib_ptr, p_dsp_funcs, FACEPROC_DT_Detect_YUV420FP, "adsp_fd_FACEPROC_DT_Detect_YUV420FP");
  IMG_DLSYM_MUST(p_lib_ptr, p_dsp_funcs, FACEPROC_DT_GetAllResultInfo,    "adsp_fd_FACEPROC_DT_GetAllResultInfo");
  IMG_DLSYM_MUST(p_lib_ptr, p_dsp_funcs, FACEPROC_DT_GetAllRawResultInfo, "adsp_fd_FACEPROC_DT_GetAllRawResultInfo");

  IMG_DLSYM_MUST(p_lib_ptr, p_dsp_funcs, FACEPROC_DT_SetDTConfig,     "adsp_fd_FACEPROC_DT_SetDTConfig");
  IMG_DLSYM_MUST(p_lib_ptr, p_dsp_funcs, FACEPROC_DT_GetDTConfig,     "adsp_fd_FACEPROC_DT_GetDTConfig");

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dsp_co_load_fptrs
 *
 * Description: Load DSP CO function pointers
 *
 * Input parameters:
 *   p_lib_ptr - library handle
 *   p_co_funcs - CO fptr structure to hold the function pointers
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_NOT_FOUND if symbol is not found in the library
 *
 * Notes:
 *     Caller has to make sure the passed args are not NULL
 *     All the function pointers are must currently.
 **/
int fpsww_dsp_co_load_fptrs(void *p_lib_ptr, fpsww_co_fptr_t *p_co_funcs)
{
  IMG_DLSYM_MUST(p_lib_ptr, p_co_funcs, FACEPROC_CO_GetVersion,            "adsp_fd_FACEPROC_CO_GetVersion");

  IMG_DLSYM_MUST(p_lib_ptr, p_co_funcs, FACEPROC_CO_CreateHandle,          "adsp_fd_FACEPROC_CO_CreateHandle");
  // IMG_DLSYM_MUST(p_lib_ptr, p_co_funcs, FACEPROC_CO_CreateHandleMalloc,    "adsp_fd_FACEPROC_CO_CreateHandleMalloc");

  IMG_DLSYM_MUST(p_lib_ptr, p_co_funcs, FACEPROC_CO_CreateHandleMemory,    "adsp_fd_FACEPROC_CO_CreateHandleMemory");
  IMG_DLSYM_MUST(p_lib_ptr, p_co_funcs, FACEPROC_CO_DeleteHandle,          "adsp_fd_FACEPROC_CO_DeleteHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_co_funcs, FACEPROC_CO_ConvertSquareToCenter, "adsp_fd_FACEPROC_CO_ConvertSquareToCenter");
  IMG_DLSYM_MUST(p_lib_ptr, p_co_funcs, FACEPROC_CO_ConvertCenterToSquare, "adsp_fd_FACEPROC_CO_ConvertCenterToSquare");

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dsp_dt_load_fptrs
 *
 * Description: Load DSP DT function pointers
 *
 * Input parameters:
 *   p_lib_ptr - library handle
 *   p_dt_funcs - DT fptr structure to hold the function pointers
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_NOT_FOUND if symbol is not found in the library
 *
 * Notes:
 *     Caller has to make sure the passed args are not NULL
 *     All the function pointers are must currently.
 **/
int fpsww_dsp_dt_load_fptrs(void *p_lib_ptr, fpsww_dt_fptr_t *p_dt_funcs)
{
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetVersion,              "adsp_fd_FACEPROC_DT_GetVersion");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_CreateHandle,            "adsp_fd_FACEPROC_DT_CreateHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_DeleteHandle,            "adsp_fd_FACEPROC_DT_DeleteHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_CreateResultHandle,      "adsp_fd_FACEPROC_DT_CreateResultHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_DeleteResultHandle,      "adsp_fd_FACEPROC_DT_DeleteResultHandle");

#if 0 // not required, will use Detect func pointers from dsp_custom_funcs
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_Detect_GRAY,             "adsp_fd_FACEPROC_DT_Detect_GRAY");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_Detect_YUV422,           "adsp_fd_FACEPROC_DT_Detect_YUV422");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_Detect_YUV420SP,         "adsp_fd_FACEPROC_DT_Detect_YUV420SP");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_Detect_YUV420FP,         "adsp_fd_FACEPROC_DT_Detect_YUV420FP");
#endif

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetResultCount,          "adsp_fd_FACEPROC_DT_GetResultCount");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetResultInfo,           "adsp_fd_FACEPROC_DT_GetResultInfo");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetRawResultInfo,        "adsp_fd_FACEPROC_DT_GetRawResultInfo");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_SetSizeRange,            "adsp_fd_FACEPROC_DT_SetSizeRange");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetSizeRange,            "adsp_fd_FACEPROC_DT_GetSizeRange");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_SetAngle,                "adsp_fd_FACEPROC_DT_SetAngle");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetAngle,                "adsp_fd_FACEPROC_DT_GetAngle");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_SetEdgeMask,             "adsp_fd_FACEPROC_DT_SetEdgeMask");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetEdgeMask,             "adsp_fd_FACEPROC_DT_GetEdgeMask");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_SetSearchDensity,        "adsp_fd_FACEPROC_DT_SetSearchDensity");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetSearchDensity,        "adsp_fd_FACEPROC_DT_GetSearchDensity");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_SetThreshold,            "adsp_fd_FACEPROC_DT_SetThreshold");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_GetThreshold,            "adsp_fd_FACEPROC_DT_GetThreshold");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_ResetTracking,        "adsp_fd_FACEPROC_DT_MV_ResetTracking");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_ToggleTrackingLock,   "adsp_fd_FACEPROC_DT_MV_ToggleTrackingLock");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetSearchCycle,       "adsp_fd_FACEPROC_DT_MV_SetSearchCycle");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetSearchCycle,       "adsp_fd_FACEPROC_DT_MV_GetSearchCycle");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetLostParam,         "adsp_fd_FACEPROC_DT_MV_SetLostParam");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetLostParam,         "adsp_fd_FACEPROC_DT_MV_GetLostParam");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetSteadinessParam,   "adsp_fd_FACEPROC_DT_MV_SetSteadinessParam");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetSteadinessParam,   "adsp_fd_FACEPROC_DT_MV_GetSteadinessParam");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetTrackingSwapParam, "adsp_fd_FACEPROC_DT_MV_SetTrackingSwapParam");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetTrackingSwapParam, "adsp_fd_FACEPROC_DT_MV_GetTrackingSwapParam");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetDelayCount,        "adsp_fd_FACEPROC_DT_MV_SetDelayCount");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetDelayCount,        "adsp_fd_FACEPROC_DT_MV_GetDelayCount");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetTrackingEdgeMask,  "adsp_fd_FACEPROC_DT_MV_SetTrackingEdgeMask");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetTrackingEdgeMask,  "adsp_fd_FACEPROC_DT_MV_GetTrackingEdgeMask");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetAccuracy,          "adsp_fd_FACEPROC_DT_MV_SetAccuracy");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetAccuracy,          "adsp_fd_FACEPROC_DT_MV_GetAccuracy");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetAngleExtension,    "adsp_fd_FACEPROC_DT_MV_SetAngleExtension");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetAngleExtension,    "adsp_fd_FACEPROC_DT_MV_GetAngleExtension");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetPoseExtension,     "adsp_fd_FACEPROC_DT_MV_SetPoseExtension");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetPoseExtension,     "adsp_fd_FACEPROC_DT_MV_GetPoseExtension");

  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_SetDirectionMask,     "adsp_fd_FACEPROC_DT_MV_SetDirectionMask");
  IMG_DLSYM_MUST(p_lib_ptr, p_dt_funcs, FACEPROC_DT_MV_GetDirectionMask,     "adsp_fd_FACEPROC_DT_MV_GetDirectionMask");

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dsp_custom_init
 *
 * Description: Init FD DSP custom setup
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_GENERAL on handle creation failure
 *
 * Notes:
 *     Caller has to make sure the passed args are not NULL
 **/
int fpsww_dsp_custom_init(faceproc_sw_wrap_t *p_hnd)
{
  if (!p_hnd->create_params.use_dsp_if_available || !p_hnd->p_lib->p_dsp_lib_handle) {
    IDBG_HIGH("DSP not initialized");
    return IMG_ERR_INVALID_INPUT;
  }

  fpsww_dsp_fptr_t *p_dsp_funcs = &p_hnd->p_lib->dsp_funcs;
  INT32             lib_ret     = FACEPROC_NORMAL;

  lib_ret = p_dsp_funcs->FDDSP_InitDetection();
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in DSP custom Init");
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  // Default values. Do not change here, instead configure
  // the required values in chromatix
  lib_ret = p_dsp_funcs->FDDSP_SetClockConfig(522, 240, 1000, 1, 4, 4, 1);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in DSP SetClockConfig");
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  return fpsww_liberr_to_imgerror(lib_ret);
}

/**
 * Function: fpsww_dsp_custom_deinit
 *
 * Description: DeInit FD DSP custom setup
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_GENERAL on handle creation failure
 *
 * Notes:
 *     Caller has to make sure the passed args are not NULL
 **/
int fpsww_dsp_custom_deinit(faceproc_sw_wrap_t *p_hnd)
{
  if (!p_hnd->create_params.use_dsp_if_available || !p_hnd->p_lib->p_dsp_lib_handle) {
    IDBG_HIGH("DSP not initialized");
    return IMG_ERR_INVALID_INPUT;
  }

  fpsww_dsp_fptr_t *p_dsp_funcs = &p_hnd->p_lib->dsp_funcs;
  INT32             lib_ret     = FACEPROC_NORMAL;

  lib_ret = p_dsp_funcs->FDDSP_SetClockConfig(0, 0, 0, 1, 0, 0, 1);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in Set Clock config");
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  lib_ret = p_dsp_funcs->FDDSP_DeInitDetection();
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in DSP custom DeInit");
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  return fpsww_liberr_to_imgerror(lib_ret);
}

/**
 * Function: fpsww_dsp_print_config
 *
 * Description: Print current DSP config params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_dsp_params - Pointer to DSP params structure
 *
 * Return values:
 *     IMG_SUCCESS on success
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_dsp_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_dsp_params_t *p_dsp_params)
{
  if (!p_hnd->create_params.use_dsp_if_available || !p_hnd->p_lib->p_dsp_lib_handle) {
    IDBG_HIGH("DSP not initialized");
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_INFO("========================= DSP Config =========================");
  IDBG_INFO("Version    : %d.%d",  p_dsp_params->majorVersion, p_dsp_params->minorVersion);
  IDBG_INFO("Clock Info : clock=%d, bus=%d, latency=%d, absapi=%d "
    "dcvs_clock = %d, dcvs_min_clock = %d, dcvs_enable = %d",
    p_dsp_params->clock, p_dsp_params->bus, p_dsp_params->latency, p_dsp_params->absapi,
    p_dsp_params->dcvs_clock, p_dsp_params->dcvs_min_clock, p_dsp_params->dcvs_enable);

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dsp_set_config
 *
 * Description: Set DSP config params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_dsp_new_params - Pointer to DSP config params which need to be set to dsp library
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_dsp_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_dsp_params_t *p_dsp_new_params)
{
  if (!p_hnd->create_params.use_dsp_if_available || !p_hnd->p_lib->p_dsp_lib_handle) {
    IDBG_HIGH("DSP not initialized");
    return IMG_ERR_INVALID_INPUT;
  }

  fpsww_dsp_fptr_t   *p_dsp_funcs          = &p_hnd->p_lib->dsp_funcs;
  fpsww_dsp_params_t *p_dsp_current_params = &p_hnd->config.dsp_params;
  INT32               lib_ret              = FACEPROC_NORMAL;

  if ((p_dsp_current_params->absapi != p_dsp_new_params->absapi)       ||
    (p_dsp_current_params->clock    != p_dsp_new_params->clock)        ||
    (p_dsp_current_params->bus      != p_dsp_new_params->bus)          ||
    (p_dsp_current_params->latency  != p_dsp_new_params->latency)      ||
    (p_dsp_current_params->dcvs_clock != p_dsp_new_params->dcvs_clock) ||
    (p_dsp_current_params->dcvs_enable != p_dsp_new_params->dcvs_enable) ||
    (p_dsp_current_params->dcvs_min_clock!= p_dsp_new_params->dcvs_min_clock)) {
    lib_ret = p_dsp_funcs->FDDSP_SetClockConfig(
      p_dsp_new_params->clock, p_dsp_new_params->bus, p_dsp_new_params->latency,
      p_dsp_new_params->absapi, p_dsp_new_params->dcvs_clock,
      p_dsp_new_params->dcvs_min_clock, p_dsp_new_params->dcvs_enable);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("Failed in FDDSP_SetClockConfig, lib_ret=%d", lib_ret);
      return fpsww_liberr_to_imgerror(lib_ret);
    }
    p_dsp_current_params->absapi         = p_dsp_new_params->absapi;
    p_dsp_current_params->clock          = p_dsp_new_params->clock;
    p_dsp_current_params->bus            = p_dsp_new_params->bus;
    p_dsp_current_params->latency        = p_dsp_new_params->latency;
    p_dsp_current_params->dcvs_clock     = p_dsp_new_params->dcvs_clock;
    p_dsp_current_params->dcvs_min_clock = p_dsp_new_params->dcvs_min_clock;
    p_dsp_current_params->dcvs_enable    = p_dsp_new_params->dcvs_enable;

  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dsp_get_config
 *
 * Description: Get DSP config params
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_dsp_params - Pointer to DSP config params
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the incoming args are not NULL
 **/
int fpsww_dsp_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_dsp_params_t *p_dsp_params)
{
  if (!p_hnd->create_params.use_dsp_if_available || !p_hnd->p_lib->p_dsp_lib_handle) {
    IDBG_HIGH("DSP not initialized");
    return IMG_ERR_INVALID_INPUT;
  }

  fpsww_dsp_fptr_t   *p_dsp_funcs  = &p_hnd->p_lib->dsp_funcs;
  INT32               lib_ret      = FACEPROC_NORMAL;

  lib_ret = p_dsp_funcs->FDDSP_GetVersion(&p_dsp_params->majorVersion, &p_dsp_params->minorVersion);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in FDDSP_GetVersion, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  lib_ret = p_dsp_funcs->FDDSP_GetClockConfig(
    &p_dsp_params->clock, &p_dsp_params->bus, &p_dsp_params->latency, &p_dsp_params->absapi,
    &p_dsp_params->dcvs_clock, &p_dsp_params->dcvs_min_clock,&p_dsp_params->dcvs_enable);
  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("Failed in FDDSP_GetClockConfig, lib_ret=%d", lib_ret);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dsp_dt_execute
 *
 * Description: Execute face detection.
 *     This will only execute the detection and gets the number of faces detected. Doesn't get
 *     the faceinfo results for faces. Call fpsww_dt_get_result_info for getting faceinfo.
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_frame - pointer to current img_frame
 *   get_results_only - when TRUE, gets only the result (num of faces), doesn't execute detection
 *   p_num_faces - If not NULL, this function saves the number of faces detected into the pointer.
 *
 * Return values:
 *     IMG_SUCCESS on success
 *
 * Notes:
 *     Caller has to make sure the required incoming args are not NULL
 **/
int fpsww_dsp_dt_execute(faceproc_sw_wrap_t *p_hnd, img_frame_t *p_frame,
  bool get_results_only, int32_t *p_num_faces)
{
  if (!p_hnd->hdt || !p_hnd->hdtresult) {
    IDBG_ERROR("Invalid handles p_hnd->hdt=%p, p_hnd->hdtresult=%p", p_hnd->hdt, p_hnd->hdtresult);
    return IMG_ERR_INVALID_INPUT;
  }

  if (!p_hnd->create_params.use_dsp_if_available || !p_hnd->p_lib->p_dsp_lib_handle) {
    IDBG_ERROR("DSP not enabled, use fpsww_dt_execute, p_dsp_lib_handle=%p, use_dsp_if_available=%d",
      p_hnd->p_lib->p_dsp_lib_handle, p_hnd->create_params.use_dsp_if_available);
    return IMG_ERR_INVALID_INPUT;
  }

  fpsww_dsp_fptr_t *p_dsp_custom_funcs = &p_hnd->p_lib->dsp_funcs;
  INT32             lib_ret            = FACEPROC_NORMAL;
  INT32             num_faces          = 0;

  // Execute DT if caller doesn't set get_results_only
  if (get_results_only == FALSE) {
    if (!p_frame) {
      IDBG_ERROR("NULL p_frame");
      return IMG_ERR_INVALID_INPUT;
    }

    if ((p_frame->frame[0].plane_cnt == 1) && (p_frame->frame[0].plane[0].plane_type == PLANE_Y)) {
      // Y
      lib_ret = p_dsp_custom_funcs->FACEPROC_DT_Detect_GRAY(p_hnd->hdt, (RAWIMAGE *)IMG_ADDR(p_frame),
                                                            IMG_FD_WIDTH(p_frame) * IMG_HEIGHT(p_frame),
                                                            (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
                                                            GRAY_ORDER_Y0Y1Y2Y3, p_hnd->hdtresult, &num_faces);
    } else if (p_frame->frame[0].plane_cnt == 1) {
      // YUV422 formats
      YUV422_ORDER yuv_order;
      switch (p_frame->frame[0].plane[0].plane_type) {
        case PLANE_Y_CB_Y_CR : yuv_order = YUV422_ORDER_Y0CBY1CR; break;
        case PLANE_Y_CR_Y_CB : yuv_order = YUV422_ORDER_Y0CRY1CB; break;
        case PLANE_CB_Y_CR_Y : yuv_order = YUV422_ORDER_CBY0CRY1; break;
        case PLANE_CR_Y_CB_Y : yuv_order = YUV422_ORDER_CRY0CBY1; break;
        default : return IMG_ERR_NOT_SUPPORTED;
      }

      lib_ret = p_dsp_custom_funcs->FACEPROC_DT_Detect_YUV422(p_hnd->hdt, (RAWIMAGE *)IMG_ADDR(p_frame), IMG_FRAME_LEN(p_frame),
                                                              (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
                                                              yuv_order, p_hnd->hdtresult, &num_faces);
    } else if (p_frame->frame[0].plane_cnt == 2) {
      // YUV420 semiplanar formats
      YUV420SP_ORDER yuv_order;
      // check the U, V positions in UV plane.
      switch (p_frame->frame[0].plane[1].plane_type) {
        case PLANE_CB_CR : yuv_order = YUV420SP_ORDER_Y0Y1Y2Y3_CBCR; break;
        case PLANE_CR_CB : yuv_order = YUV420SP_ORDER_Y0Y1Y2Y3_CRCB; break;
        default : return IMG_ERR_NOT_SUPPORTED;
      }

      lib_ret = p_dsp_custom_funcs->FACEPROC_DT_Detect_YUV420SP(p_hnd->hdt,
                                                                (RAWIMAGE *)IMG_PLANE_ADDR(p_frame, 0), QIMG_LEN(p_frame, 0),
                                                                (RAWIMAGE *)IMG_PLANE_ADDR(p_frame, 1), QIMG_LEN(p_frame, 1),
                                                                (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
                                                                yuv_order, p_hnd->hdtresult, &num_faces);
    } else if (p_frame->frame[0].plane_cnt == 3) {
      // YUV420 full planar formats, supports only Y_Cb_Cr. faceproc lib doesn't support Y_Cr_Cb
      YUV420FP_ORDER yuv_order;
      switch (p_frame->frame[0].plane[1].plane_type) {
        case PLANE_CB : yuv_order = YUV420FP_ORDER_Y0Y1Y2Y3_CB_CR; break;
        default : return IMG_ERR_NOT_SUPPORTED;
      }

      lib_ret = p_dsp_custom_funcs->FACEPROC_DT_Detect_YUV420FP(p_hnd->hdt,
                                                                (RAWIMAGE *)IMG_PLANE_ADDR(p_frame, 0), QIMG_LEN(p_frame, 0),
                                                                (RAWIMAGE *)IMG_PLANE_ADDR(p_frame, 1), QIMG_LEN(p_frame, 1),
                                                                (RAWIMAGE *)IMG_PLANE_ADDR(p_frame, 2), QIMG_LEN(p_frame, 2),
                                                                (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
                                                                yuv_order, p_hnd->hdtresult, &num_faces);
    } else {
      return IMG_ERR_NOT_SUPPORTED;
    }

    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC Detection returned error: %d, plane_cnt = %d, plane0 type=%d",
        lib_ret, p_frame->frame[0].plane_cnt, p_frame->frame[0].plane[0].plane_type);
      return fpsww_liberr_to_imgerror(lib_ret);
    }

    if (p_num_faces) {
      *p_num_faces = num_faces;
    }

    return IMG_SUCCESS;
  }

  if (p_num_faces) {
    fpsww_dt_fptr_t  *p_dt_funcs = &p_hnd->p_lib->dsp_dt_funcs;

    *p_num_faces = 0;

    lib_ret = p_dt_funcs->FACEPROC_DT_GetResultCount(p_hnd->hdtresult, p_num_faces);
    if (lib_ret != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_DT_GetResultCount returned error: %d", lib_ret);
      *p_num_faces = 0;
      return fpsww_liberr_to_imgerror(lib_ret);
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: fpsww_dsp_dt_get_result_info
 *
 * Description: Gets the face detection results info. Caller must call fpsww_dt_execute before
 *     calling this function. This will only get the results information of previously ran face detection.
 *     This function needs to be called once to get the information of all faces.
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_face_info - face information array to be filled
 *   num_faces - number of faces information to get.
 *   raw_result - Whether to get raw results information for this face.
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_XXX error corresponds to error code returned by faceproc library
 *
 * Notes:
 *     Caller has to make sure the required incoming args are not NULL
 **/
int fpsww_dsp_dt_get_result_info(faceproc_sw_wrap_t *p_hnd, DETECTION_INFO *p_face_info, int32_t num_faces, bool raw_result)
{
  if (!p_hnd->hdt || !p_hnd->hdtresult || !p_face_info) {
    IDBG_ERROR("Invalid handles p_hnd->hdt=%p, p_hnd->hdtresult=%p", p_hnd->hdt, p_hnd->hdtresult);
    return IMG_ERR_INVALID_INPUT;
  }

  if (!p_hnd->create_params.use_dsp_if_available || !p_hnd->p_lib->p_dsp_lib_handle) {
    IDBG_ERROR("DSP not enabled, use fpsww_dt_execute, p_dsp_lib_handle=%p, use_dsp_if_available=%d",
      p_hnd->p_lib->p_dsp_lib_handle, p_hnd->create_params.use_dsp_if_available);
    return IMG_ERR_INVALID_INPUT;
  }

  fpsww_dsp_fptr_t *p_dsp_funcs = &p_hnd->p_lib->dsp_funcs;
  INT32             lib_ret     = FACEPROC_NORMAL;

  if (raw_result == FALSE) {
    lib_ret = p_dsp_funcs->FACEPROC_DT_GetAllResultInfo(p_hnd->hdtresult, p_face_info, num_faces);
  } else {
    lib_ret = p_dsp_funcs->FACEPROC_DT_GetAllRawResultInfo(p_hnd->hdtresult, p_face_info, num_faces);
  }

  if (lib_ret != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_DT_GetResultInfo returned error: %d, raw_result=%d", lib_ret, raw_result);
    return fpsww_liberr_to_imgerror(lib_ret);
  }

  return IMG_SUCCESS;
}

