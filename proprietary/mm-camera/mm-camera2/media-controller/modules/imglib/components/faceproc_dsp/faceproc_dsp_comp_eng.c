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

static unsigned frame_count = 0;
// static const unsigned skip_count = 10;
// static const unsigned initial_skip_count = 5;
static unsigned dump_count = 0;

/**
 * Function: faceproc_dsp_comp_eng_reset_fn_ptrs
 *
 * Description: resets the dynamically loaded function pointers
 *
 * Input parameters:
 *   @p_lib: pointer to faceproc lib
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
static int faceproc_dsp_comp_eng_reset_fn_ptrs(faceproc_dsp_lib_t *p_lib)
{
  if (!p_lib) {
    IDBG_ERROR("invalid param");
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_LOW("Enter");

  memset(&(p_lib->fns), 0x0, sizeof(p_lib->fns));

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_dsp_comp_eng_load_fn_ptrs
 *
 * Description: Loads the dynamically loaded function pointers
 *
 * Input parameters:
 *   @p_lib: pointer to faceproc lib
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
static int faceproc_dsp_comp_eng_load_fn_ptrs(faceproc_dsp_lib_t *p_lib)
{
  int lib_status_rc;
  uint8 fd_minor_version, fd_major_version;
  if (!p_lib || !p_lib->ptr_stub) {
    IDBG_ERROR("libmmcamera_imglib_faceproc_adspstub.so lib is not loaded");
    return IMG_ERR_GENERAL;
  }

  /* Link all the fd dsp functions in adsp stub lib */
  *(void **)&(p_lib->fns.FACEPROC_Dt_VersionDSP) =
  dlsym(p_lib->ptr_stub, "adsp_fd_getVersion");
  if (p_lib->fns.FACEPROC_Dt_VersionDSP == NULL) {
    IDBG_ERROR("Loading FACEPROC_Dt_VersionDSP error");
    return IMG_ERR_GENERAL;
  }

  IDBG_HIGH("ADSP STUB FACEPROC_Dt_VersionDSP loaded fine %d ",
    (int)p_lib->fns.FACEPROC_Dt_VersionDSP);

  /* Check if FD DSP stub library is requested and valid */
  lib_status_rc = p_lib->fns.FACEPROC_Dt_VersionDSP
    (&fd_minor_version, &fd_major_version);

  IDBG_HIGH("ADSP STUB Checking if DSP stub is valid - rc: %d",
    lib_status_rc);

  if (lib_status_rc != FACEPROC_NORMAL) {  /*Is DSP stub lib invalid */
    IDBG_WARN("ADSP STUB FD DSP lib error = %d ", lib_status_rc);
  }

  IDBG_HIGH("ADSP STUB exe FACEPROC_Dt_VersionDSP rc %d  , "
    "minv %d load %d , status %d",
    lib_status_rc, fd_minor_version,
    p_lib->load_dsp_lib,p_lib->status_dsp_lib);

  *(void **)&(p_lib->fns.FACEPROC_DeleteDtResult) =
    dlsym(p_lib->ptr_stub, "adsp_fd_DeleteDtResult");
  if (p_lib->fns.FACEPROC_DeleteDtResult == NULL) {
    IDBG_ERROR("Loading FACEPROC_DeleteDtResult error");
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->fns.FACEPROC_DeleteDetection) =
    dlsym(p_lib->ptr_stub, "adsp_fd_DeleteDetection");
  if (p_lib->fns.FACEPROC_DeleteDetection == NULL) {
    IDBG_ERROR("Loading FACEPROC_DeleteDetection error");
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->fns.FACEPROC_GetDtFaceCount) =
    dlsym(p_lib->ptr_stub, "adsp_fd_GetDtFaceCount");
  if (p_lib->fns.FACEPROC_GetDtFaceCount == NULL) {
    IDBG_ERROR("Loading FACEPROC_GetDtFaceCount error");
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->fns.FACEPROC_GetDtFaceInfo) =
    dlsym(p_lib->ptr_stub, "adsp_fd_GetDtFaceInfo");
  if (p_lib->fns.FACEPROC_GetDtFaceInfo == NULL) {
    IDBG_ERROR("Loading FACEPROC_GetDtFaceInfo error");
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->fns.FACEPROC_CreateDetection) =
    dlsym(p_lib->ptr_stub, "adsp_fd_CreateDetection");
  if (p_lib->fns.FACEPROC_CreateDetection == NULL) {
    IDBG_ERROR("Loading FACEPROC_CreateDetection error");
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->fns.FACEPROC_SetDtMode) =
    dlsym(p_lib->ptr_stub, "adsp_fd_SetDtMode");
  if (p_lib->fns.FACEPROC_SetDtMode == NULL) {
    IDBG_ERROR("Loading FACEPROC_SetDtMode error");
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->fns.FACEPROC_SetDtStep) =
    dlsym(p_lib->ptr_stub, "adsp_fd_SetDtStep");
  if (p_lib->fns.FACEPROC_SetDtStep == NULL) {
    IDBG_ERROR("Loading FACEPROC_SetDtStep error");
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->fns.FACEPROC_SetDtAngle) =
    dlsym(p_lib->ptr_stub, "adsp_fd_SetDtAngle");
  if (p_lib->fns.FACEPROC_SetDtAngle == NULL) {
    IDBG_ERROR("Loading FACEPROC_SetDtAngle error");
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->fns.FACEPROC_SetDtDirectionMask) =
    dlsym(p_lib->ptr_stub, "adsp_fd_SetDtDirectionMask");
  if (p_lib->fns.FACEPROC_SetDtDirectionMask == NULL) {
    IDBG_ERROR("Loading FACEPROC_SetDtDirectionMask error");
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->fns.FACEPROC_SetDtFaceSizeRange) =
    dlsym(p_lib->ptr_stub, "adsp_fd_SetDtFaceSizeRange");
  if (p_lib->fns.FACEPROC_SetDtFaceSizeRange == NULL) {
    IDBG_ERROR("Loading FACEPROC_SetDtFaceSizeRange error");
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->fns.FACEPROC_SetDtThreshold) =
    dlsym(p_lib->ptr_stub, "adsp_fd_SetDtThreshold");
  if (p_lib->fns.FACEPROC_SetDtThreshold == NULL) {
    IDBG_ERROR("Loading FACEPROC_SetDtThreshold error");
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->fns.FACEPROC_CreateDtResult) =
    dlsym(p_lib->ptr_stub, "adsp_fd_CreateDtResult");
  if (p_lib->fns.FACEPROC_CreateDtResult == NULL) {
    IDBG_ERROR("Loading FACEPROC_CreateDtResult error");
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->fns.FACEPROC_DetectionDSP) =
    dlsym(p_lib->ptr_stub, "adsp_fd_Detection");
  if (p_lib->fns.FACEPROC_DetectionDSP == NULL) {
    IDBG_ERROR("Loading FACEPROC_DetectionDSP error");
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->fns.FACEPROC_SetDtRefreshCount) =
    dlsym(p_lib->ptr_stub, "adsp_fd_SetDtRefreshCount");
  if (p_lib->fns.FACEPROC_SetDtRefreshCount == NULL) {
    IDBG_ERROR("Loading FACEPROC_SetDtRefreshCount error");
    return IMG_ERR_GENERAL;
  }

  *(void **)&(p_lib->fns.FACEPROC_SetDSPPowerPref) =
    dlsym(p_lib->ptr_stub, "adsp_fd_SetDSPPowerPref");
  if (p_lib->fns.FACEPROC_SetDSPPowerPref == NULL) {
    IDBG_ERROR("Loading FACEPROC_SetDSPPowerPref error");
    return IMG_ERR_GENERAL;
  }

  FD_DLSYM_ERROR_RET(p_lib, FACEPROC_SetDtLostParam, "adsp_fd_SetDtLostParam");
  FD_DLSYM_ERROR_RET(p_lib, FACEPROC_DtLockID, "adsp_fd_DtLockID");

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_dsp_comp_eng_unload_lib
 *
 * Description: Callback to be called from dsp dl mgr to unload adspstub.so
 *
 * Input parameters:
 *   @handle: handle to faceproc lib
 *   @p_userdata: pointer to userdata
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
static int faceproc_dsp_comp_eng_unload_lib(void *handle, void *p_userdata)
{
  IMG_UNUSED(p_userdata);
  if (g_faceproc_dsp_lib.ptr_stub) {
    dlclose(g_faceproc_dsp_lib.ptr_stub);
    g_faceproc_dsp_lib.ptr_stub = NULL;
  }
  faceproc_dsp_comp_eng_reset_fn_ptrs(&g_faceproc_dsp_lib);

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_dsp_comp_eng_reload_lib
 *
 * Description: Callback to be called from dsp dl mgr to reload adspstub.so
 *
 * Input parameters:
 *   @handle: handle to faceproc lib
 *   @name: library name
 *   @p_userdata: pointer to userdata
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
static int faceproc_dsp_comp_eng_reload_lib(void *handle, const char *name,
  void *p_userdata)
{
  IMG_UNUSED(name);
  IMG_UNUSED(handle);
  IMG_UNUSED(p_userdata);
  faceproc_dsp_lib_t *p_lib = &g_faceproc_dsp_lib;

  IDBG_HIGH("Will Try Loading ADSP STUB NOW libmmcamera_"
    "imglib_faceproc_adspstub.so ");
  /* Load adsp stub lib */
  if (p_lib->ptr_stub == NULL) {
    p_lib->ptr_stub =
      dlopen("libmmcamera_imglib_faceproc_adspstub.so", RTLD_NOW);
    IDBG_HIGH("ptr_stub: %p", p_lib->ptr_stub);
    if (!p_lib->ptr_stub) {
      IDBG_ERROR("Error loading libmmcamera_imglib_faceproc_adspstub.so lib");
      return IMG_ERR_GENERAL;
    }
  }
  return faceproc_dsp_comp_eng_load_fn_ptrs(p_lib);

}

/**
 * Function: faceproc_dsp_comp_eng_load_dt_dsp
 *
 * Description: Loads the faceproc library for DSP
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
static int faceproc_dsp_comp_eng_load_dt_dsp(faceproc_dsp_lib_t *p_lib)
{
  IDBG_HIGH("Will Try Loading ADSP STUB NOW libmmcamera_"
    "imglib_faceproc_adspstub.so ");
  /* Load adsp stub lib */
  faceproc_dsp_comp_eng_reload_lib(p_lib,
    "libmmcamera_imglib_faceproc_adspstub.so", NULL);
  img_dsp_dlopen("libmmcamera_imglib_faceproc_adspstub.so",p_lib,
    faceproc_dsp_comp_eng_unload_lib,
    faceproc_dsp_comp_eng_reload_lib );

  p_lib->status_dsp_lib = 1;  /* FD DSP lib loaded */
  return IMG_SUCCESS;
}

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
  int rc = 0;
  IDBG_HIGH("E, p_lib->load_dsp_lib=%d", p_lib->load_dsp_lib);

  if (p_lib->load_dsp_lib) {
    rc = faceproc_dsp_comp_eng_load_dt_dsp(p_lib);
  }

  if (rc < 0) {
    IDBG_ERROR("comp_eng_load_dt_dsp failed rc=%d", rc);
    faceproc_dsp_comp_eng_unload(p_lib);
  }

#ifdef FACEPROC_USE_NEW_WRAPPER
    // this is only for facial parts, use ARM facial parts
    rc = faceproc_sw_wrapper_load(FALSE);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Faceproc sw wrapper load failed");
    }
#endif

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
  IDBG_HIGH("E");
  if (p_lib->ptr_stub) {
    dlclose(p_lib->ptr_stub);
    p_lib->ptr_stub = NULL;
  }

#ifdef FACEPROC_USE_NEW_WRAPPER
  faceproc_sw_wrapper_unload(FALSE);
#endif
  memset(p_lib, 0, sizeof(faceproc_dsp_lib_t));
}

#ifdef FACEPROC_USE_NEW_WRAPPER
/**
 * Function: faceproc_dsp_comp_get_fpsww_config
 *
 * Description: To set faceproc sw wrapper config from chromatix
 *
 * Input parameters:
 *   p_chromatix - The pointer to fd chromatix
 *   p_sw_wrapper_config - config struct for fd faceproc sw wrapper
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int faceproc_dsp_comp_get_fpsww_config(fd_chromatix_t *p_chromatix,
  fpsww_config_t *p_sw_wrapper_config)
{
  if (!p_chromatix || !p_sw_wrapper_config) {
    IDBG_ERROR("error p_chromatix=%p, p_sw_wrapper_config=%p",
      p_chromatix, p_sw_wrapper_config);
    return IMG_ERR_INVALID_INPUT;
  }

  /* Fill faceproc sw wrapper tuning */

  // Currently, facial_parts is treated as : enable if one of bsgc is enabled
  // but actually, facial parts can be identified without bsgc. Fix this later.
  p_sw_wrapper_config->enable_facial_parts =
    (p_chromatix->enable_contour_detection |
    p_chromatix->enable_smile_detection |
    p_chromatix->enable_gaze_detection |
    p_chromatix->enable_blink_detection);
  p_sw_wrapper_config->enable_contour =
    p_chromatix->enable_contour_detection;
  p_sw_wrapper_config->enable_smile =
    p_chromatix->enable_smile_detection;
  p_sw_wrapper_config->enable_gaze =
    p_chromatix->enable_gaze_detection;
  p_sw_wrapper_config->enable_blink =
    p_chromatix->enable_blink_detection;
  p_sw_wrapper_config->enable_recog = FALSE;

  // ============= SW Wrapper Filter params =============
  p_sw_wrapper_config->filter_params.discard_facialparts_threshold =
    p_chromatix->facial_parts_threshold;
  p_sw_wrapper_config->filter_params.discard_threshold =
    p_chromatix->assist_facial_discard_threshold;
  p_sw_wrapper_config->filter_params.discard_face_below =
    p_chromatix->assist_below_threshold;

  p_sw_wrapper_config->filter_params.closed_eye_ratio =
    p_chromatix->closed_eye_ratio_threshold;

  p_sw_wrapper_config->filter_params.weight_eyes =
    p_chromatix->assist_facial_weight_eyes;
  p_sw_wrapper_config->filter_params.weight_mouth =
    p_chromatix->assist_facial_weight_mouth;
  p_sw_wrapper_config->filter_params.weight_nose =
    p_chromatix->assist_facial_weight_nose;
  p_sw_wrapper_config->filter_params.weight_face =
    p_chromatix->assist_facial_weight_face;

  p_sw_wrapper_config->filter_params.eyes_use_max_filter =
    (p_chromatix->assist_facial_eyes_filter_type == FD_FILTER_TYPE_MAX);
  p_sw_wrapper_config->filter_params.nose_use_max_filter =
    (p_chromatix->assist_facial_nose_filter_type == FD_FILTER_TYPE_MAX);

  p_sw_wrapper_config->filter_params.sw_face_box_border_per =
    p_chromatix->assist_sw_detect_box_border_perc;
  p_sw_wrapper_config->filter_params.sw_face_discard_border =
    p_chromatix->assist_sw_discard_frame_border;
  p_sw_wrapper_config->filter_params.sw_face_discard_out =
    p_chromatix->assist_sw_discard_out_of_border;

  p_sw_wrapper_config->filter_params.min_threshold =
    p_chromatix->assist_facial_min_face_threshold;

  p_sw_wrapper_config->filter_params.enable_fp_false_pos_filtering =
    p_chromatix->enable_facial_parts_assisted_face_filtering;
  p_sw_wrapper_config->filter_params.enable_sw_false_pos_filtering =
    p_chromatix->enable_sw_assisted_face_filtering;

  p_sw_wrapper_config->filter_params.sw_face_search_dens =
    p_chromatix->assist_sw_detect_search_dens;

  // ============= DT params =============
  if (p_chromatix->assist_sw_detect_threshold) {
    p_sw_wrapper_config->dt_params.nThreshold =
      (int32_t)p_chromatix->assist_sw_detect_threshold;
  }

  // ============= CT params =============
  p_sw_wrapper_config->ct_params.nMode = p_chromatix->ct_detection_mode;

  return IMG_SUCCESS;
}
#else
/**
 * Function: faceproc_dsp_comp_get_facialparts_config
 *
 * Description: To set facial parts config from chromatix
 *
 * Input parameters:
 *   p_chromatix - The pointer to fd chromatix
 *   p_facial_parts_config - config struct for fd facial parts
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int faceproc_dsp_comp_get_facialparts_config(fd_chromatix_t *p_chromatix,
  facial_parts_wrap_config_t *p_facial_parts_config)
{
  if (NULL == p_chromatix || NULL == p_facial_parts_config) {
    IDBG_ERROR("error p_chromatix=%p, p_facial_parts_config=%p",
      p_chromatix, p_facial_parts_config);
    return IMG_ERR_INVALID_INPUT;
  }
  /* Fill facial parts detection tuning */
  p_facial_parts_config->enable_blink =
    p_chromatix->enable_blink_detection;
  p_facial_parts_config->enable_smile =
    p_chromatix->enable_smile_detection;
  p_facial_parts_config->enable_gaze =
    p_chromatix->enable_gaze_detection;
  p_facial_parts_config->detection_threshold =
    p_chromatix->facial_parts_threshold;

  p_facial_parts_config->discard_threshold =
    p_chromatix->assist_facial_discard_threshold;
  p_facial_parts_config->weight_eyes =
    p_chromatix->assist_facial_weight_eyes;
  p_facial_parts_config->weight_mouth =
    p_chromatix->assist_facial_weight_mouth;
  p_facial_parts_config->weight_nose =
    p_chromatix->assist_facial_weight_nose;
  p_facial_parts_config->weight_face =
    p_chromatix->assist_facial_weight_face;
  p_facial_parts_config->discard_face_below =
    p_chromatix->assist_below_threshold;
  p_facial_parts_config->weight_eyes =
    p_chromatix->assist_facial_weight_eyes;
  p_facial_parts_config->eyes_use_max_filter =
    (p_chromatix->assist_facial_eyes_filter_type == FD_FILTER_TYPE_MAX);
  p_facial_parts_config->nose_use_max_filter =
    (p_chromatix->assist_facial_nose_filter_type == FD_FILTER_TYPE_MAX);
  p_facial_parts_config->sw_face_threshold =
    p_chromatix->assist_sw_detect_threshold;
  p_facial_parts_config->sw_face_box_border_per =
    p_chromatix->assist_sw_detect_box_border_perc;
  p_facial_parts_config->sw_face_search_dens =
    p_chromatix->assist_sw_detect_search_dens;
  p_facial_parts_config->sw_face_discard_border =
    p_chromatix->assist_sw_discard_frame_border;
  p_facial_parts_config->sw_face_discard_out =
    p_chromatix->assist_sw_discard_out_of_border;
  p_facial_parts_config->min_threshold =
    p_chromatix->assist_facial_min_face_threshold;
  p_facial_parts_config->enable_contour =
    p_chromatix->enable_contour_detection;
  p_facial_parts_config->contour_detection_mode =
    p_chromatix->ct_detection_mode;
  p_facial_parts_config->enable_fp_false_pos_filtering =
    p_chromatix->enable_facial_parts_assisted_face_filtering;
  p_facial_parts_config->enable_sw_false_pos_filtering =
    p_chromatix->enable_sw_assisted_face_filtering;
  return IMG_SUCCESS;
}
#endif

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
  img_frame_t *p_frame, faceproc_result_t *fd_data,
  INT32 *num_faces)
{
  int rc;
  int status; //for img error
  uint32_t i;
  faceproc_info_t *p_output = NULL;

  /* FD START */
  /* Get the number of faces */
  rc = p_comp->p_lib->fns.FACEPROC_GetDtFaceCount(p_comp->hresult, num_faces);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_GetDtFaceCount returned error: %d", (uint32_t)rc);
    *num_faces = 0;
    return faceproc_dsp_error_to_img_error(rc);
  }
  IDBG_MED("num_faces %d   fd_dsp", *num_faces);

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
  for (i = 0; i < fd_data->num_faces_detected; i++) {
    FACEINFO face_info;
    uint32_t left, top, right, bottom;
    rc = p_comp->p_lib->fns.FACEPROC_GetDtFaceInfo(p_comp->hresult, (int32_t)i,
      &face_info);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_GetDtFaceInfo returned error: %d", (uint32_t)rc);
      fd_data->num_faces_detected--;
      return faceproc_dsp_error_to_img_error(rc);
    }
    IDBG_MED("FACE INFO conf %d, nid %d, nPose %d, LT x %d, LT y %d, "
      "LB x %d, LB y %d lock %d",
      face_info.nConfidence,
      face_info.nID,
      face_info.nPose,
      face_info.ptLeftTop.x,
      face_info.ptLeftTop.y,
      face_info.ptLeftBottom.x,
      face_info.ptLeftBottom.y,
      p_comp->fd_chromatix.lock_faces);

    if (p_comp->fd_chromatix.lock_faces && (face_info.nID > 0)) {
      rc = p_comp->p_lib->fns.FACEPROC_DtLockID(p_comp->hresult, face_info.nID);
      if (FACEPROC_NORMAL != rc) {
        IDBG_ERROR("Error FACEPROC_DtLockID %d ID %u",
          rc, (uint32_t)face_info.nID);
      }
    }

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
      face_len = (uint32_t)sqrt((uint32_t)
        ((pow(y_delta, 2) + pow(x_delta, 2))) >> 1);
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

    /* some logic to caculate correct roll angle */
    float angle = atan2( face_info.ptLeftTop.x - face_info.ptLeftBottom.x,
      face_info.ptLeftTop.y - face_info.ptLeftBottom.y);
    int angle_deg = angle * 180 / PI;
    p_output->face_angle_roll = angle_deg;

    p_output->face_angle_roll = 180 - p_output->face_angle_roll;
    IDBG_LOW("face_angle_roll %d  angle= %f )", angle_deg, angle);

#ifdef FACEPROC_USE_NEW_WRAPPER
    /* Detect facial parts */
    if (p_comp->p_sw_wrapper) {
      status = faceproc_sw_wrapper_process_result(p_comp->p_sw_wrapper,
        p_frame, p_output, 0, 0, NULL, i, true);
      if (status == IMG_ERR_NOT_FOUND) {
        IDBG_ERROR("[FD_FALSE_POS_DBG] Filter face %d", i);
        continue;
      }
    }
#else
    /* Detect facial parts */
    if (p_comp->p_lib->facial_parts_hndl) {
      status = facial_parts_wrap_process_result(
        p_comp->p_lib->facial_parts_hndl,
        p_frame, p_output, 0, 0, NULL, true);
      if (status == IMG_ERR_NOT_FOUND) {
        IDBG_ERROR("[FD_FALSE_POS_DBG] Filter face %d", i);
        continue;
      }
    }
#endif

  }  /* end of forloop */
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
  img_frame_t *p_frame, INT32 * num_faces)
{
  ATRACE_BEGIN_SNPRINTF(32, "FD_%d", p_frame->frame_id);
  struct timeval start_time, end_time;
  memset(&start_time, 0, sizeof(start_time));
  memset(&end_time, 0, sizeof(end_time));
  IDBG_MED("E %dx%d", IMG_FD_WIDTH(p_frame), IMG_HEIGHT(p_frame));

  IMG_DBG_TIMER_START(start_time);

  int rc = FACEPROC_NORMAL;
  p_comp->frame_id = p_frame->frame_id;
  if (p_comp->p_lib->status_dsp_lib) {
    IDBG_MED("before FACEPROC_DetectionDSP E");
    IDBG_MED("before FACEPROC_DetectionDSP, hDT: %p, pImage: %p, nWidth: %d,"
      "nHeight: %d, nAccuracy: %d, hResult: %p",
      p_comp->hdt,
      (RAWIMAGE *)IMG_ADDR(p_frame),
      IMG_FD_WIDTH(p_frame),
      IMG_HEIGHT(p_frame),
      ACCURACY_HIGH_TR,
      p_comp->hresult);

    rc = p_comp->p_lib->fns.FACEPROC_DetectionDSP(p_comp->hdt,
      (RAWIMAGE *)IMG_ADDR(p_frame),
      IMG_FD_WIDTH(p_frame)*IMG_HEIGHT(p_frame),
      IMG_FD_WIDTH(p_frame),
      IMG_HEIGHT(p_frame),
      ACCURACY_HIGH_TR,
      p_comp->hresult);

    IDBG_MED("after FACEPROC_DetectionDSP");

    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_DetectionDSP returned error: %d", (uint32_t)rc);
      *num_faces = 0;
      goto fd_ex_end;
    }
  } else {
    IDBG_MED("FACEPROC_DetectionDSP disabled ");
  }

  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_Detection returned error: %d", (uint32_t)rc);
    return faceproc_dsp_error_to_img_error(rc);
  }

  if (frame_count < 10) {
    IMG_DBG_TIMER_END(start_time, end_time, "FD_DSP", IMG_TIMER_MODE_MS);
  }

  ATRACE_END();

  frame_count++;

#ifdef FD_FPS
  /*Log FD fps every 16 fd_frames */
  if (frame_count % 16 == 0) {
    gettimeofday (&fd_time, NULL);
    end_fd_ms = (fd_time.tv_sec * 1000 + fd_time.tv_usec / 1000);
    total_elapsed_ms = end_fd_ms - start_fd_ms;
    IDBG_HIGH("FD frame rate: %2.1f",
      (16000.0 / ((double)end_fd_ms - (double)start_fd_ms)));
    start_fd_ms = end_fd_ms;
  }
#endif

  /*Set Position for PT */
  rc = p_comp->p_lib->fns.FACEPROC_GetDtFaceCount(p_comp->hresult, num_faces);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_GetDtFaceCount returned error: %d", (uint32_t)rc);
    *num_faces = 0;
    return faceproc_dsp_error_to_img_error(rc);
  }

  IDBG_MED("FACEPROC_GetDtFaceCount executed successfully "
           "returned error: %d", (uint32_t)rc);

  if (p_comp->dump_mode != FACE_FRAME_DUMP_OFF) {
    FACEINFO face_info;
    int index;
    int tracked = 0;

    for (index = 0; index < *num_faces; index++) {
      rc = p_comp->p_lib->fns.FACEPROC_GetDtFaceInfo(p_comp->hresult, (int32_t)index,
        &face_info);
      if (rc != FACEPROC_NORMAL) {
        IDBG_ERROR("FACEPROC_GetDtFaceInfo returned error: %d", rc);
      }
      if (face_info.nID < 0) {
        tracked += 1;
      }
    }

    faceproc_common_frame_dump(p_frame, p_comp->dump_mode,
      tracked, *num_faces);
  }

  IDBG_MED("num faces %d, dsp %d    fd_dsp",
    *num_faces, p_comp->p_lib->status_dsp_lib);

  if (*num_faces <= 0) {
    IDBG_MED("no faces detected X");
    return IMG_SUCCESS;
  }

fd_ex_end:
  IDBG_MED("X");

  return faceproc_dsp_error_to_img_error(rc);
}

/**
 * Function: faceproc_dsp_comp_eng_set_facesize
 *
 * Description: Configure the faceproc engine min face size
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   max_width - width of the image
 *   max_height - height of the image
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
int faceproc_dsp_comp_eng_update_cfg(faceproc_dsp_comp_t *p_comp)
{
  uint32_t min_face_size = faceproc_common_get_face_size(
    p_comp->fd_chromatix.min_face_adj_type,
    p_comp->fd_chromatix.min_face_size,
    p_comp->fd_chromatix.min_face_size_ratio,
    FACEPROC_MIN_FACE_SIZE,
    MIN(p_comp->width, p_comp->height));
  IDBG_MED("new ###min_face_size %d", min_face_size);

  /* Set the max and min face size for detection */
  int rc = (int) p_comp->p_lib->fns.FACEPROC_SetDtFaceSizeRange(
    p_comp->hdt, (int32_t)min_face_size,
    (int32_t)p_comp->fd_chromatix.max_face_size);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtFaceSizeRange failed %d", rc);
  }

  return IMG_SUCCESS;
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

  UINT32 an_still_angle[POSE_TYPE_COUNT];
  int rc = IMG_SUCCESS;
  faceproc_config_t *p_cfg ;
  uint32_t rotation_range ;
  uint32_t max_num_face_to_detect;
  uint32_t min_face_size;

  if (!p_comp ) {
    IDBG_ERROR("NULL component");
    return IMG_ERR_GENERAL;
  }
  p_cfg = &p_comp->config;
  rotation_range = p_cfg->face_cfg.rotation_range;

  min_face_size = faceproc_common_get_face_size(
    p_comp->fd_chromatix.min_face_adj_type,
    p_comp->fd_chromatix.min_face_size,
    p_comp->fd_chromatix.min_face_size_ratio,
    FACEPROC_MIN_FACE_SIZE,
    MIN(p_cfg->frame_cfg.max_width, p_cfg->frame_cfg.max_height));

  IDBG_MED("###min_face_size %d", min_face_size);

  max_num_face_to_detect = p_comp->fd_chromatix.max_num_face_to_detect;

  IDBG_MED("Enter: p_comp: [%p, %p]load_dsp: %d, max_faces: %d",
    p_comp, (void *)&p_comp,
    p_comp->p_lib->status_dsp_lib, max_num_face_to_detect);

  if (FD_ANGLE_ENABLE(&p_comp->config)) {
    an_still_angle[POSE_FRONT] = faceproc_common_get_angle(
      p_comp->fd_chromatix.angle_front, false, 0, -1);
    an_still_angle[POSE_HALF_PROFILE] = faceproc_common_get_angle(
      p_comp->fd_chromatix.angle_half_profile, false, 0, -1);
    an_still_angle[POSE_PROFILE] = faceproc_common_get_angle(
      p_comp->fd_chromatix.angle_full_profile, false, 0, -1);
  } else {
    IDBG_MED("###Disable Angle");
    an_still_angle[POSE_FRONT] = ANGLE_NONE;
    an_still_angle[POSE_HALF_PROFILE] = ANGLE_NONE;
    an_still_angle[POSE_PROFILE] = ANGLE_NONE;
  }

  IDBG_MED("Before calling FACEPROC_CreateDetection: p_comp: %p", p_comp);
  p_comp->hdt = p_comp->p_lib->fns.FACEPROC_CreateDetection();
  if (!p_comp->hdt) {
    IDBG_ERROR("FACEPROC_CreateDetection failed");
    return IMG_ERR_GENERAL;
  } else {
    IDBG_MED("calling FACEPROC_SetDSPPowerPref:p_comp:%p, %d %d %d %d",
    p_comp, 522, 240, 1000, 1);
    p_comp->p_lib->fns.FACEPROC_SetDSPPowerPref(522,240,1000,1);
  }
  IDBG_MED("After calling FACEPROC_CreateDetection");


  /* Set best Faceproc detection mode for video */
  rc = (int) p_comp->p_lib->fns.FACEPROC_SetDtMode(
    p_comp->hdt, (int32_t)p_comp->fd_chromatix.detection_mode);


  IDBG_MED("After calling FACEPROC_SetDtMode");
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtMode failed %d", rc);
    return faceproc_dsp_error_to_img_error(rc);
  }

  /* Set search density */
  uint32_t nontracking_dens = faceproc_common_validate_and_get_density(
    false, p_comp->fd_chromatix.search_density_nontracking, 0);
  uint32_t tracking_dens = faceproc_common_validate_and_get_density(
    false, p_comp->fd_chromatix.search_density_tracking, 0);

  rc = (int) p_comp->p_lib->fns.FACEPROC_SetDtStep(
    p_comp->hdt, (int32_t)nontracking_dens, (int32_t)tracking_dens);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtStep failed %d", rc);
    return faceproc_dsp_error_to_img_error(rc);
  }

  /* Set Detection Angles */
  rc = (int) p_comp->p_lib->fns.FACEPROC_SetDtAngle(
    p_comp->hdt, an_still_angle,
    ANGLE_ROTATION_EXT0 | ANGLE_POSE_EXT0);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtAngle failed %d", rc);
    return faceproc_dsp_error_to_img_error(rc);
  }

  /* Set refresh count */
  rc = (int) p_comp->p_lib->fns.FACEPROC_SetDtRefreshCount(
    p_comp->hdt, (int32_t)p_comp->fd_chromatix.detection_mode,
    (int32_t)p_comp->fd_chromatix.refresh_count);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtRefreshCount failed %d", rc);
    return faceproc_dsp_error_to_img_error(rc);
  }

  rc = (int) p_comp->p_lib->fns.FACEPROC_SetDtDirectionMask(
    p_comp->hdt, (int32_t)p_comp->fd_chromatix.direction);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtDirectionMask failed %d", rc);
    return faceproc_dsp_error_to_img_error(rc);
  }

  /* Minimum face size to be detected should be at most half the
    height of the input frame */
  if (min_face_size > (p_cfg->frame_cfg.max_height/2)) {
    IDBG_ERROR("Error, min face size to detect is greater than "
      "half the height of the input frame");
    return IMG_SUCCESS;
  }

  /* Set the max and min face size for detection */
  rc = (int) p_comp->p_lib->fns.FACEPROC_SetDtFaceSizeRange(
    p_comp->hdt, (int32_t)min_face_size,
    (int32_t)p_comp->fd_chromatix.max_face_size);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtFaceSizeRange failed %d", rc);
    return faceproc_dsp_error_to_img_error(rc);
  }
  /* Set Detection Threshold */
  rc = (int) p_comp->p_lib->fns.FACEPROC_SetDtThreshold(
    p_comp->hdt, (int32_t)p_comp->fd_chromatix.threshold,
    (int32_t)p_comp->fd_chromatix.threshold);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtFaceSizeRange failed %d",  rc);
    return faceproc_dsp_error_to_img_error(rc);
  }

  /* FD Configuration logging */
  //IDBG_HIGH("fddsp_config: Parts: (%d, %d), Contour: %d, BGS: %d, Recog: %d",
  //  FD_FACEPT_ENABLE(p_comp), FACE_PART_DETECT, FACE_CONTOUR_DETECT,
  //  FACE_BGS_DETECT, FACE_RECOGNITION);
  IDBG_HIGH("fddsp_config: MAX # of faces: %d", max_num_face_to_detect);
  IDBG_HIGH("fddsp_config: MIN, MAX face size: %d, %d",
    min_face_size, p_comp->fd_chromatix.max_face_size);
  IDBG_HIGH("fddsp_config: DT_mode: %d, Refresh_count: %d",
    p_comp->fd_chromatix.detection_mode, p_comp->fd_chromatix.refresh_count);
  IDBG_HIGH("fddsp_config: Search Density: %d",
    p_comp->fd_chromatix.search_density_tracking);
  IDBG_HIGH("fddsp_config: Detection Threshold: %d",
    p_comp->fd_chromatix.threshold);
  IDBG_HIGH("fddsp_config: Angles: %d, %d, %d, Track: %d",
    an_still_angle[POSE_FRONT], an_still_angle[POSE_HALF_PROFILE],
    an_still_angle[POSE_PROFILE], (ANGLE_ROTATION_EXT0 | ANGLE_POSE_EXT0));

  /* Create Faceproc result handle */
  p_comp->hresult = p_comp->p_lib->fns.FACEPROC_CreateDtResult(
    (int32_t)max_num_face_to_detect ,
    (int32_t)(max_num_face_to_detect/2));
  if (!(p_comp->hresult)) {
    IDBG_ERROR("FACEPROC_CreateDtResult failed");
    return IMG_ERR_GENERAL;
  }

#ifdef FACEPROC_USE_NEW_WRAPPER
  // Create sw_wrapper handle
  fpsww_create_params_t create_params;
  create_params.engine = FACEPROC_ENGINE_DSP;
  create_params.create_face_parts = TRUE;
  create_params.create_face_recog = FALSE;
  create_params.max_face_count = 1;
  create_params.detection_mode = DETECTION_MODE_STILL;
  create_params.no_of_fp_handles = 1;
  create_params.use_dsp_if_available = FALSE;

  p_comp->p_sw_wrapper = faceproc_sw_wrapper_create(&create_params);
  if (!p_comp->p_sw_wrapper) {
    IDBG_ERROR("sw_wrapper creation failed");
    return IMG_ERR_GENERAL;
  }

  fpsww_config_t sw_wrapper_config;
  // Get the default/current config first
  rc = faceproc_sw_wrapper_get_config(p_comp->p_sw_wrapper,
    &sw_wrapper_config);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("sw_wrapper get config failed");
    return IMG_ERR_INVALID_INPUT;
  }

  // Update the the sw wrapper config as per chromatix
  faceproc_dsp_comp_get_fpsww_config(&p_comp->fd_chromatix,
    &sw_wrapper_config);

  rc = faceproc_sw_wrapper_set_config(p_comp->p_sw_wrapper,
    &sw_wrapper_config);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("sw_wrapper set config failed");
    return IMG_ERR_INVALID_INPUT;
  }
#else
  p_comp->p_lib->facial_parts_hndl = facial_parts_wrap_create();
  if (!p_comp->p_lib->facial_parts_hndl) {
    IDBG_ERROR("Facial create failed.");
    return IMG_ERR_GENERAL;
  }

  facial_parts_wrap_config_t fp_config;
  memset(&fp_config, 0x0, sizeof(fp_config));
  faceproc_dsp_comp_get_facialparts_config(&p_comp->fd_chromatix,
    &fp_config);

  rc = facial_parts_wrap_config(p_comp->p_lib->facial_parts_hndl,
    &fp_config);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Can not config face parts, status=%d", status);
    return IMG_ERR_INVALID_INPUT;
  }
#endif

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
  IDBG_MED("Enter: %p", (void *)p_comp);

  UINT32 an_still_angle[POSE_TYPE_COUNT];
  int rc = IMG_SUCCESS;
  faceproc_config_t *p_cfg ;
  uint32_t rotation_range ;
  uint32_t max_num_face_to_detect;
  uint32_t min_face_size;

  if (!p_comp) {
    IDBG_ERROR("NULL component");
    return IMG_ERR_GENERAL;
  }
  p_cfg = &p_comp->config;
  rotation_range = p_cfg->face_cfg.rotation_range;

  min_face_size = faceproc_common_get_face_size(
    p_comp->fd_chromatix.min_face_adj_type,
    p_comp->fd_chromatix.min_face_size,
    p_comp->fd_chromatix.min_face_size_ratio,
    FACEPROC_MIN_FACE_SIZE,
    MIN(p_cfg->frame_cfg.max_width, p_cfg->frame_cfg.max_height));

  IDBG_MED("###min_face_size %d", min_face_size);

  max_num_face_to_detect = p_comp->fd_chromatix.max_num_face_to_detect;

  IDBG_MED("Enter: p_comp: [%p, %p]load_dsp: %d, max_faces: %d",
    p_comp, (void *)&p_comp, p_comp->p_lib->status_dsp_lib,
    max_num_face_to_detect);

  if (FD_ANGLE_ENABLE(&p_comp->config)) {
    an_still_angle[POSE_FRONT] = faceproc_common_get_angle(
      p_comp->fd_chromatix.angle_front, false, 0, -1);
    an_still_angle[POSE_HALF_PROFILE] = faceproc_common_get_angle(
      p_comp->fd_chromatix.angle_half_profile, false, 0, -1);
    an_still_angle[POSE_PROFILE] = faceproc_common_get_angle(
      p_comp->fd_chromatix.angle_full_profile, false, 0, -1);
  } else {
    IDBG_MED("###Disable Angle");
    an_still_angle[POSE_FRONT] = ANGLE_NONE;
    an_still_angle[POSE_HALF_PROFILE] = ANGLE_NONE;
    an_still_angle[POSE_PROFILE] = ANGLE_NONE;
  }

  /* Set best Faceproc detection mode for video */
  rc = (int) p_comp->p_lib->fns.FACEPROC_SetDtMode(
    p_comp->hdt, (int32_t)p_comp->fd_chromatix.detection_mode);


  IDBG_MED("After calling FACEPROC_SetDtMode");
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtMode failed %d", rc);
    return faceproc_dsp_error_to_img_error(rc);
  }

  /* Set search density */
  uint32_t nontracking_dens = faceproc_common_validate_and_get_density(
    false, p_comp->fd_chromatix.search_density_nontracking, 0);
  uint32_t tracking_dens = faceproc_common_validate_and_get_density(
    false, p_comp->fd_chromatix.search_density_tracking, 0);

  rc = (int) p_comp->p_lib->fns.FACEPROC_SetDtStep(
    p_comp->hdt, (int32_t)nontracking_dens, (int32_t)tracking_dens);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtStep failed %d", rc);
    return faceproc_dsp_error_to_img_error(rc);
  }

  /* Set Detection Angles */
  rc = (int) p_comp->p_lib->fns.FACEPROC_SetDtAngle(
    p_comp->hdt, an_still_angle,
    ANGLE_ROTATION_EXT0 | ANGLE_POSE_EXT0);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtAngle failed %d", rc);
    return faceproc_dsp_error_to_img_error(rc);
  }

  /* Set refresh count */
  rc = (int) p_comp->p_lib->fns.FACEPROC_SetDtRefreshCount(
    p_comp->hdt, (int32_t)p_comp->fd_chromatix.detection_mode,
    (int32_t)p_comp->fd_chromatix.refresh_count);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtRefreshCount failed %d", rc);
    return faceproc_dsp_error_to_img_error(rc);
  }

  rc = (int) p_comp->p_lib->fns.FACEPROC_SetDtDirectionMask(
    p_comp->hdt, (BOOL)p_comp->fd_chromatix.direction);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtDirectionMask failed %d", rc);
    return faceproc_dsp_error_to_img_error(rc);
  }

  /* Minimum face size to be detected should be at most half the
    height of the input frame */
  if (min_face_size > (p_cfg->frame_cfg.max_height / 2)) {
    IDBG_ERROR("min face size error %d, %d ",
      min_face_size, (p_cfg->frame_cfg.max_height / 2));
    return IMG_ERR_INVALID_INPUT;
  }

  /* Set the max and min face size for detection */
  rc = (int) p_comp->p_lib->fns.FACEPROC_SetDtFaceSizeRange(
    p_comp->hdt, (int32_t)min_face_size,
    (int32_t)p_comp->fd_chromatix.max_face_size);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtFaceSizeRange failed %d", rc);
    return faceproc_dsp_error_to_img_error(rc);
  }
  /* Set Detection Threshold */
  rc = (int) p_comp->p_lib->fns.FACEPROC_SetDtThreshold(
    p_comp->hdt, (int32_t)p_comp->fd_chromatix.threshold,
    (int32_t)p_comp->fd_chromatix.threshold);
  if (rc != FACEPROC_NORMAL) {
    IDBG_ERROR("FACEPROC_SetDtFaceSizeRange failed %d", rc);
    return faceproc_dsp_error_to_img_error(rc);
  }

  /* FD Configuration logging */
  IDBG_HIGH("fddsp_reconfig: MAX # of faces: %d", max_num_face_to_detect);
  IDBG_HIGH("fddsp_reconfig: MIN, MAX face size: %d, %d",
    min_face_size, p_comp->fd_chromatix.max_face_size);
  IDBG_HIGH("fddsp_reconfig: DT_mode: %d, Refresh_count: %d",
    p_comp->fd_chromatix.detection_mode, p_comp->fd_chromatix.refresh_count);
  IDBG_HIGH("fddsp_reconfig: Search Density: %d",
    p_comp->fd_chromatix.search_density_tracking);
  IDBG_HIGH("fddsp_reconfig: Detection Threshold: %d",
    p_comp->fd_chromatix.threshold);
  IDBG_HIGH("fddsp_reconfig: Angles: %d, %d, %d, Track: %d",
    an_still_angle[POSE_FRONT], an_still_angle[POSE_HALF_PROFILE],
    an_still_angle[POSE_PROFILE], (ANGLE_ROTATION_EXT0 | ANGLE_POSE_EXT0));

  /* Delete Old Result handle */
  if (p_comp->hresult) {
    rc = p_comp->p_lib->fns.FACEPROC_DeleteDtResult(p_comp->hresult);
    if (rc != FACEPROC_NORMAL) {
      IDBG_ERROR("FACEPROC_DeleteDtResult failed");
      return faceproc_dsp_error_to_img_error(rc);
    }
    p_comp->hresult = NULL;
  }
  /* Create Faceproc result handle */
  p_comp->hresult = p_comp->p_lib->fns.FACEPROC_CreateDtResult(
    (int32_t)max_num_face_to_detect ,
    (int32_t)(max_num_face_to_detect/2));
  if (!(p_comp->hresult)) {
    IDBG_ERROR("FACEPROC_CreateDtResult failed");
    return IMG_ERR_GENERAL;
  }

#ifdef FACEPROC_USE_NEW_WRAPPER
  fpsww_config_t sw_wrapper_config;
  int status;

  if (p_comp->p_sw_wrapper) {
    // Get the default/current config first
    status = faceproc_sw_wrapper_get_config(p_comp->p_sw_wrapper,
      &sw_wrapper_config);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("sw_wrapper get config failed");
      return status;
    }

    // Update the the sw wrapper config as per chromatix
    faceproc_dsp_comp_get_fpsww_config(&p_comp->fd_chromatix,
      &sw_wrapper_config);

    status = faceproc_sw_wrapper_set_config(p_comp->p_sw_wrapper,
      &sw_wrapper_config);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("sw_wrapper set config failed");
      return status;
    }
  }
#else
  facial_parts_wrap_config_t fp_config;
  memset(&fp_config,0,sizeof(fp_config));
  faceproc_dsp_comp_get_facialparts_config(&p_comp->fd_chromatix,
    &fp_config);
  if (p_comp->p_lib->facial_parts_hndl) {
    status = facial_parts_wrap_config(p_comp->p_lib->facial_parts_hndl,
      &fp_config);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("Can not config face parts");
      return status;
    }
  }
#endif
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
  IDBG_MED("Enter");
  int ret;
  int status;

  if (!p_comp || !p_comp->p_lib)
    return IMG_ERR_GENERAL;

  /* Delete Result handle */
  if (p_comp->hresult) {
    ret = p_comp->p_lib->fns.FACEPROC_DeleteDtResult(p_comp->hresult);
    if (ret != FACEPROC_NORMAL)
      return faceproc_dsp_error_to_img_error(ret);
    p_comp->hresult = NULL;
  }
  /* Delete Handle */
  if (p_comp->hdt) {
    ret = p_comp->p_lib->fns.FACEPROC_DeleteDetection(p_comp->hdt);
    if (ret != FACEPROC_NORMAL)
      return faceproc_dsp_error_to_img_error(ret);
    p_comp->hdt = NULL;
  }

#ifdef FACEPROC_USE_NEW_WRAPPER
  if (p_comp->p_sw_wrapper) {
    status = faceproc_sw_wrapper_destroy(p_comp->p_sw_wrapper);
    if (IMG_ERROR(status)) {
      IDBG_ERROR("sw_wrapper destroy failed %d", status);
    }
    p_comp->p_sw_wrapper = NULL;
  }
#else
  if (p_comp->p_lib->facial_parts_hndl) {
    facial_parts_wrap_destroy(p_comp->p_lib->facial_parts_hndl);
    p_comp->p_lib->facial_parts_hndl = NULL;
  }
#endif

  frame_count = 0;
  dump_count = 0;
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
  uint8 fd_minor_version, fd_major_version;
  int lib_status_rc;

  IDBG_MED("E");

  if (!p_lib->ptr_stub) {
    IDBG_ERROR("dsp lib NOT loaded ");
    return IMG_ERR_GENERAL;
  }

  if (!p_lib->fns.FACEPROC_Dt_VersionDSP) {
    IDBG_ERROR("dsp lib NOT loaded fn ");
    return IMG_ERR_GENERAL;
  }

  /* Check if FD DSP stub library is requested and valid */
  lib_status_rc = p_lib->fns.FACEPROC_Dt_VersionDSP(
       &fd_minor_version, &fd_major_version);

  if (lib_status_rc != IMG_SUCCESS) {  /*Is DSP stub lib invalid */
    IDBG_ERROR("ADSP STUB FD DSP lib error, lib_status_rc=%d",
      lib_status_rc);
    return faceproc_dsp_error_to_img_error(lib_status_rc);
  } else {
    IDBG_MED("ADSP STUB exe FACEPROC_Dt_VersionDSP rc %d  ,"
      " minv %d load %d , status %d",
      lib_status_rc, fd_minor_version, p_lib->load_dsp_lib,
      p_lib->status_dsp_lib);
    return IMG_SUCCESS;
  }
}


