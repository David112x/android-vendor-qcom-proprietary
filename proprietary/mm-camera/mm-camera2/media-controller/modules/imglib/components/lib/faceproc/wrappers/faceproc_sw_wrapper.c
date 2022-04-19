/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "faceproc_sw_wrapper.h"
#include "faceproc_lib_wrapper.h"
#include "faceproc_common.h"
#include "faceproc.h"
#include "img_dsp_dl_mgr.h"

// A one time load of library and function pointers is enough
// even when multiple instances of wrapper are created from
// difference faceproc components(hw, dsp, sw). Need a counter
// to determine when to unload the library.
// Ideally, need mutex as well.
static fpsww_lib_t g_faceproc_wrap_lib = { .p_lib_handle = 0 };
static int lib_counter = 0;
static int lib_dsp_counter = 0;

/** face_contour_point_eye_t:
 *
 *  Convert lookup from cam_types to faceproc internal interface
 **/
static const int convert_contour_eye[][2] = {
  {FACE_CONTOUR_POINT_EYE_L_PUPIL, CT_POINT_EYE_L_PUPIL},
  {FACE_CONTOUR_POINT_EYE_L_IN,    CT_POINT_EYE_L_IN},
  {FACE_CONTOUR_POINT_EYE_L_OUT,   CT_POINT_EYE_L_OUT},
  {FACE_CONTOUR_POINT_EYE_L_UP,    CT_POINT_EYE_L_UP},
  {FACE_CONTOUR_POINT_EYE_L_DOWN,  CT_POINT_EYE_L_DOWN},
  {FACE_CONTOUR_POINT_EYE_R_PUPIL, CT_POINT_EYE_R_PUPIL},
  {FACE_CONTOUR_POINT_EYE_R_IN,    CT_POINT_EYE_R_IN},
  {FACE_CONTOUR_POINT_EYE_R_OUT,   CT_POINT_EYE_R_OUT},
  {FACE_CONTOUR_POINT_EYE_R_UP,    CT_POINT_EYE_R_UP},
  {FACE_CONTOUR_POINT_EYE_R_DOWN,  CT_POINT_EYE_R_DOWN},
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
  {FACE_CONTOUR_POINT_NOSE,     CT_POINT_NOSE},
  {FACE_CONTOUR_POINT_NOSE_TIP, CT_POINT_NOSE_TIP},
  {FACE_CONTOUR_POINT_NOSE_L,   CT_POINT_NOSE_L},
  {FACE_CONTOUR_POINT_NOSE_R,   CT_POINT_NOSE_R},
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
  {FACE_CONTOUR_POINT_MOUTH_L,    CT_POINT_MOUTH_L},
  {FACE_CONTOUR_POINT_MOUTH_R,    CT_POINT_MOUTH_R},
  {FACE_CONTOUR_POINT_MOUTH_UP,   CT_POINT_MOUTH_UP},
  {FACE_CONTOUR_POINT_MOUTH_DOWN, CT_POINT_MOUTH_DOWN},
};

/** face_contour_point_lip_t:
 *
 *  Convert lookup from cam_types to faceproc internal interface
 **/
static const int convert_contour_lip[][2] = {
  {FACE_CONTOUR_POINT_LIP_UP,   CT_POINT_LIP_UP},
  {FACE_CONTOUR_POINT_LIP_DOWN, CT_POINT_LIP_DOWN},
};

/** face_contour_point_brow_t:
 *
 *  Convert lookup from cam_types to faceproc internal interface
 **/
static const int convert_contour_brow[][2] = {
  {FACE_CONTOUR_POINT_BROW_L_UP,   CT_POINT_BROW_L_UP},
  {FACE_CONTOUR_POINT_BROW_L_DOWN, CT_POINT_BROW_L_DOWN},
  {FACE_CONTOUR_POINT_BROW_L_IN,   CT_POINT_BROW_L_IN},
  {FACE_CONTOUR_POINT_BROW_L_OUT,  CT_POINT_BROW_L_OUT},
  {FACE_CONTOUR_POINT_BROW_R_UP,   CT_POINT_BROW_R_UP},
  {FACE_CONTOUR_POINT_BROW_R_DOWN, CT_POINT_BROW_R_DOWN},
  {FACE_CONTOUR_POINT_BROW_R_IN,   CT_POINT_BROW_R_IN},
  {FACE_CONTOUR_POINT_BROW_R_OUT,  CT_POINT_BROW_R_OUT},
};

/** face_contour_point_chin_t:
 *
 *  Convert lookup from cam_types to faceproc internal interface
 **/
static const int convert_contour_chin[][2] = {
  {FACE_CONTOUR_POINT_CHIN,   CT_POINT_CHIN},
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
  {FACE_CONTOUR_POINT_EAR_L_UP,   CT_POINT_EAR_L_UP},
  {FACE_CONTOUR_POINT_EAR_R_UP,   CT_POINT_EAR_R_UP},
};

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
static int wrapper_validate_and_fix_PT_coordinates(faceproc_info_t *p_output)
{
  if((IMG_IS_POINT_VALID(p_output->fp.face_pt[PT_POINT_LEFT_EYE])) &&
     (!IMG_IS_POINT_WITHIN_RECT(p_output->face_boundary, p_output->fp.face_pt[PT_POINT_LEFT_EYE]))) {
    IMG_SET_POINT_TO_RECT_CENTER(p_output->face_boundary,
      p_output->fp.face_pt[PT_POINT_LEFT_EYE]);
    IDBG_MED("corrected left eye coordinates");
  }
  if((IMG_IS_POINT_VALID(p_output->fp.face_pt[PT_POINT_RIGHT_EYE])) &&
     (!IMG_IS_POINT_WITHIN_RECT(p_output->face_boundary, p_output->fp.face_pt[PT_POINT_RIGHT_EYE]))) {
    IMG_SET_POINT_TO_RECT_CENTER(p_output->face_boundary,
      p_output->fp.face_pt[PT_POINT_RIGHT_EYE]);
    IDBG_MED("corrected right eye coordinates");
  }
  if(IMG_IS_POINT_VALID(p_output->fp.face_pt[PT_POINT_MOUTH]) &&
    !IMG_IS_POINT_WITHIN_RECT(p_output->face_boundary,
    p_output->fp.face_pt[PT_POINT_MOUTH])) {
    IMG_SET_POINT_TO_RECT_CENTER(p_output->face_boundary,
      p_output->fp.face_pt[PT_POINT_MOUTH]);
    IDBG_MED("corrected mouth coordinates");
  }
  return IMG_SUCCESS;
}

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
      angle_mask = ROLL_ANGLE_0;
    } else if (face_angle_roll < 45) {
      angle_mask = ROLL_ANGLE_1;
    } else if (face_angle_roll < 75 ) {
      angle_mask = ROLL_ANGLE_2;
    } else if (face_angle_roll < 105) {
      angle_mask = ROLL_ANGLE_3;
    } else if (face_angle_roll < 135){
      angle_mask = ROLL_ANGLE_4;
    } else if (face_angle_roll < 165) {
      angle_mask = ROLL_ANGLE_5;
    } else if (face_angle_roll < 195) {
      angle_mask = ROLL_ANGLE_6;
    } else if (face_angle_roll < 225) {
      angle_mask = ROLL_ANGLE_7;
    } else if (face_angle_roll < 255) {
      angle_mask = ROLL_ANGLE_8;
    } else if (face_angle_roll < 285) {
      angle_mask = ROLL_ANGLE_9;
    } else if (face_angle_roll < 315) {
      angle_mask = ROLL_ANGLE_10;
    } else if (face_angle_roll < 345) {
      angle_mask = ROLL_ANGLE_11;
    }
    break;
  case 2: // pick one angle or two in case the angle is around a border
    if ((face_angle_roll < 10) || (face_angle_roll >= 350)) {
      angle_mask = ROLL_ANGLE_0;
    } else if (face_angle_roll < 20) {
      angle_mask = ROLL_ANGLE_0 | ROLL_ANGLE_1;
    } else if (face_angle_roll < 40) {
      angle_mask = ROLL_ANGLE_1;
    } else if (face_angle_roll < 50) {
      angle_mask = ROLL_ANGLE_1 | ROLL_ANGLE_2;
    } else if (face_angle_roll < 70){
      angle_mask = ROLL_ANGLE_2;
    } else if (face_angle_roll < 80) {
      angle_mask = ROLL_ANGLE_2 | ROLL_ANGLE_3;
    } else if (face_angle_roll < 100) {
      angle_mask = ROLL_ANGLE_3;
    } else if (face_angle_roll < 110) {
      angle_mask = ROLL_ANGLE_3 | ROLL_ANGLE_4;
    } else if (face_angle_roll < 130) {
      angle_mask = ROLL_ANGLE_4;
    } else if (face_angle_roll < 140) {
      angle_mask = ROLL_ANGLE_4 | ROLL_ANGLE_5;
    } else if (face_angle_roll < 160) {
      angle_mask = ROLL_ANGLE_5;
    } else if (face_angle_roll < 170) {
      angle_mask = ROLL_ANGLE_5 | ROLL_ANGLE_6;
    } else if (face_angle_roll < 190) {
      angle_mask = ROLL_ANGLE_6;
    } else if (face_angle_roll < 200) {
      angle_mask = ROLL_ANGLE_6 | ROLL_ANGLE_7;
    } else if (face_angle_roll < 220) {
      angle_mask = ROLL_ANGLE_7;
    } else if (face_angle_roll < 230) {
      angle_mask = ROLL_ANGLE_7 | ROLL_ANGLE_8;
    } else if (face_angle_roll < 250) {
      angle_mask = ROLL_ANGLE_8;
    } else if (face_angle_roll < 260) {
      angle_mask = ROLL_ANGLE_8 | ROLL_ANGLE_9;
    } else if (face_angle_roll < 280) {
      angle_mask = ROLL_ANGLE_9;
    } else if (face_angle_roll < 290) {
      angle_mask = ROLL_ANGLE_9 | ROLL_ANGLE_10;
    } else if (face_angle_roll < 310) {
      angle_mask = ROLL_ANGLE_10;
    } else if (face_angle_roll < 320) {
      angle_mask = ROLL_ANGLE_10 | ROLL_ANGLE_11;
    } else if (face_angle_roll < 340) {
      angle_mask = ROLL_ANGLE_11;
    } else if (face_angle_roll < 350) {
      angle_mask = ROLL_ANGLE_11 | ROLL_ANGLE_0;
    }
    break;
  case 3: // pick 3 angle
    if ((face_angle_roll < 15) || (face_angle_roll >= 345)) {
      angle_mask = ROLL_ANGLE_11 | ROLL_ANGLE_0 | ROLL_ANGLE_1;
    } else if (face_angle_roll < 45) {
      angle_mask = ROLL_ANGLE_0 | ROLL_ANGLE_1 | ROLL_ANGLE_2;
    } else if (face_angle_roll < 75 ) {
      angle_mask = ROLL_ANGLE_1 | ROLL_ANGLE_2 | ROLL_ANGLE_3;
    } else if (face_angle_roll < 105) {
      angle_mask = ROLL_ANGLE_2 | ROLL_ANGLE_3 | ROLL_ANGLE_4;
    } else if (face_angle_roll < 135){
      angle_mask = ROLL_ANGLE_3 | ROLL_ANGLE_4 | ROLL_ANGLE_5;
    } else if (face_angle_roll < 165) {
      angle_mask = ROLL_ANGLE_4 | ROLL_ANGLE_5 | ROLL_ANGLE_6;
    } else if (face_angle_roll < 195) {
      angle_mask = ROLL_ANGLE_5 | ROLL_ANGLE_6 | ROLL_ANGLE_7;
    } else if (face_angle_roll < 225) {
      angle_mask = ROLL_ANGLE_6 | ROLL_ANGLE_7| ROLL_ANGLE_8;
    } else if (face_angle_roll < 255) {
      angle_mask = ROLL_ANGLE_7 | ROLL_ANGLE_8 | ROLL_ANGLE_9;
    } else if (face_angle_roll < 285) {
      angle_mask = ROLL_ANGLE_8 | ROLL_ANGLE_9 | ROLL_ANGLE_10;
    } else if (face_angle_roll < 315) {
      angle_mask = ROLL_ANGLE_9 | ROLL_ANGLE_10 | ROLL_ANGLE_11;
    } else if (face_angle_roll < 345) {
      angle_mask = ROLL_ANGLE_10 | ROLL_ANGLE_11 | ROLL_ANGLE_0;
    }
    break;
  }

  return angle_mask;
}

/**
 * Function: wrapper_discard_face_parts_based
 *
 * Description: Discard face based on facial parts false filtering.
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_output - Faceinfo
 *   p_pt_results - pointer to pt results.
 *
 * Return values:
 *     0 - to accept
 *     1 - to discard
 *
 * Notes: None
 **/
static int wrapper_discard_face_parts_based(faceproc_sw_wrap_t *p_hnd,
  faceproc_info_t *p_output, face_part_detect *p_pt_results)
{
  uint32_t mouth_conf;
  uint32_t eye_conf;
  int32_t face_conf;
  uint32_t nose_conf;
  float ratio;

  if (!p_hnd->config.filter_params.discard_threshold ||  !p_hnd->config.filter_params.min_threshold) {
    IDBG_MED("Dont discard the face thresh %d id %d %d",
      p_hnd->config.filter_params.discard_threshold, p_output->unique_id, p_output->fd_confidence);
    return 0;
  }

  if (p_output->fd_confidence < p_hnd->config.filter_params.min_threshold) {
    IDBG_ERROR("Invalid confidence %d min %d",
      p_output->fd_confidence, p_hnd->config.filter_params.min_threshold);
    return 0;
  }

  ratio = 1000.0 / (float)(p_hnd->config.filter_params.discard_face_below - p_hnd->config.filter_params.min_threshold);
  face_conf = ratio * (float)(p_output->fd_confidence - p_hnd->config.filter_params.min_threshold);

  /* Get nose with confidence level based on filter type */
  if (p_hnd->config.filter_params.nose_use_max_filter) {
    nose_conf = IMG_MAX(p_pt_results->confidence[FACE_PART_NOSE_LEFT], p_pt_results->confidence[FACE_PART_NOSE_RIGHT]);
  } else {
    nose_conf = IMG_AVG(p_pt_results->confidence[FACE_PART_NOSE_LEFT], p_pt_results->confidence[FACE_PART_NOSE_RIGHT]);
  }

  /* Get eye with confidence level based on filter type */
  if (p_hnd->config.filter_params.eyes_use_max_filter) {
    eye_conf = IMG_MAX(p_pt_results->confidence[FACE_PART_LEFT_EYE], p_pt_results->confidence[FACE_PART_RIGHT_EYE]);
  } else {
    eye_conf = IMG_AVG(p_pt_results->confidence[FACE_PART_LEFT_EYE], p_pt_results->confidence[FACE_PART_RIGHT_EYE]);
  }

  eye_conf = ceil((float)eye_conf * p_hnd->config.filter_params.weight_eyes);

  mouth_conf = p_pt_results->confidence[FACE_PART_MOUTH];
  mouth_conf = ceil((float)mouth_conf * p_hnd->config.filter_params.weight_mouth);
  nose_conf  = ceil((float)nose_conf * p_hnd->config.filter_params.weight_nose);
  face_conf  = ceil((float)face_conf * p_hnd->config.filter_params.weight_face);

  if ((mouth_conf + eye_conf + nose_conf + face_conf) < p_hnd->config.filter_params.discard_threshold) {
    IDBG_ERROR("[FD_FALSE_POS_DBG] Discard face eye conf %d mouth_conf %d"
      "nose_conf %d face_conf %d threshold: %d "
      "Default eye conf (%d %d) mouth %d nose(%d %d) face %d",
      eye_conf, mouth_conf, nose_conf, face_conf, p_hnd->config.filter_params.discard_threshold,
      p_pt_results->confidence[FACE_PART_LEFT_EYE],
      p_pt_results->confidence[FACE_PART_RIGHT_EYE],
      p_pt_results->confidence[FACE_PART_MOUTH],
      p_pt_results->confidence[FACE_PART_NOSE_LEFT],
      p_pt_results->confidence[FACE_PART_NOSE_RIGHT],
      face_conf);
    return 1;
  }

  IDBG_INFO("[FD_FALSE_POS_DBG] Allow face eye conf %d mouth_conf %d"
    "nose_conf %d face_conf %d threshold: %d "
    "Default eye conf (%d %d) mouth %d nose(%d %d) face %d",
    eye_conf, mouth_conf, nose_conf, face_conf, p_hnd->config.filter_params.discard_threshold,
    p_pt_results->confidence[FACE_PART_LEFT_EYE],
    p_pt_results->confidence[FACE_PART_RIGHT_EYE],
    p_pt_results->confidence[FACE_PART_MOUTH],
    p_pt_results->confidence[FACE_PART_NOSE_LEFT],
    p_pt_results->confidence[FACE_PART_NOSE_RIGHT],
    face_conf);

  return 0;
}

/**
 * Function: wrapper_load_library
 *
 * Description: Wrapper function to load sw faceproc library and function pointers
 *
 * Input parameters:
 *   p_lib - faceproc sw lib handle
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
static int wrapper_load_library(fpsww_lib_t *p_lib)
{
  int rc = IMG_SUCCESS;

  if (!p_lib) {
    IDBG_ERROR("Null pointer");
    return IMG_ERR_INVALID_INPUT;
  }

  p_lib->p_lib_handle = dlopen("libmmcamera_faceproc2.so", RTLD_NOW);
  if (!p_lib->p_lib_handle) {
    IDBG_ERROR("Error opening libmmcamera_faceproc2.so lib");
    return IMG_ERR_NOT_FOUND;
  }

  rc = fpsww_co_load_fptrs(p_lib->p_lib_handle, &p_lib->co_funcs);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in loading CO function pointers");
    goto error;
  }

  rc = fpsww_dt_load_fptrs(p_lib->p_lib_handle, &p_lib->dt_funcs);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in loading DT function pointers");
    goto error;
  }

  rc = fpsww_pt_load_fptrs(p_lib->p_lib_handle, &p_lib->pt_funcs);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in loading PT function pointers");
    goto error;
  }

  rc = fpsww_ct_load_fptrs(p_lib->p_lib_handle, &p_lib->ct_funcs);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in loading CT function pointers");
    goto error;
  }

  rc = fpsww_sm_load_fptrs(p_lib->p_lib_handle, &p_lib->sm_funcs);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in loading SM function pointers");
    goto error;
  }

  rc = fpsww_gb_load_fptrs(p_lib->p_lib_handle, &p_lib->gb_funcs);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in loading GB function pointers");
    goto error;
  }

  rc = fpsww_fr_load_fptrs(p_lib->p_lib_handle, &p_lib->fr_funcs);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in loading FR function pointers");
    goto error;
  }

  return rc;

error:
  dlclose(p_lib->p_lib_handle);
  p_lib->p_lib_handle = NULL;
  return rc;
}

/**
 * Function: wrapper_dsp_load_library
 *
 * Description: Wrapper function to load dsp faceproc library and function pointers
 *
 * Input parameters:
 *   p_lib - faceproc sw lib handle
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
static int wrapper_dsp_load_library(fpsww_lib_t *p_lib)
{
  int rc = IMG_SUCCESS;

  if (!p_lib) {
    IDBG_ERROR("Null pointer");
    return IMG_ERR_INVALID_INPUT;
  }

  p_lib->p_dsp_lib_handle = dlopen("libmmcamera_imglib_faceproc_adspstub2.so", RTLD_NOW);
  if (!p_lib->p_dsp_lib_handle) {
    IDBG_ERROR("Error opening libmmcamera_imglib_faceproc_adspstub2.so lib");
    return IMG_ERR_NOT_FOUND;
  }

  rc = fpsww_dsp_load_fptrs(p_lib->p_dsp_lib_handle, &p_lib->dsp_funcs);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in loading DSP custom function pointers");
    goto error;
  }

  rc = fpsww_dsp_co_load_fptrs(p_lib->p_dsp_lib_handle, &p_lib->dsp_co_funcs);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in loading CO function pointers");
    goto error;
  }

  rc = fpsww_dsp_dt_load_fptrs(p_lib->p_dsp_lib_handle, &p_lib->dsp_dt_funcs);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in loading DT function pointers");
    goto error;
  }

  return rc;

error:
  dlclose(p_lib->p_dsp_lib_handle);
  p_lib->p_dsp_lib_handle = NULL;
  return rc;
}

/**
 * Function: wrapper_unload_library
 *
 * Description: Wrapper function to unload sw faceproc library
 *
 * Input parameters:
 *   p_lib - faceproc sw lib handle
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_ERR_INVALID_INPUT on failure
 *
 * Notes: None
 **/
static int wrapper_unload_library(fpsww_lib_t *p_lib)
{
  if (!p_lib) {
    IDBG_ERROR("Null pointer");
    return IMG_ERR_INVALID_INPUT;
  }

  dlclose(p_lib->p_lib_handle);
  p_lib->p_lib_handle = NULL;

  return IMG_SUCCESS;
}

/**
 * Function: wrapper_dsp_unload_library
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
static int wrapper_dsp_unload_library(void *p_lib, void *p_userdata)
{
  dlclose(g_faceproc_wrap_lib.p_dsp_lib_handle);
  g_faceproc_wrap_lib.p_dsp_lib_handle = NULL;

  return IMG_SUCCESS;
}

/**
 * Function: wrapper_dsp_reload_library
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
static int wrapper_dsp_reload_library(void *handle, const char *name,
  void *p_userdata)
{
  int rc;

  wrapper_dsp_unload_library(handle, p_userdata);

  rc = wrapper_dsp_load_library(&g_faceproc_wrap_lib);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("DSP reload failed rc=%d", rc);
  }

  return rc;
}

/**
 * Function: wrapper_get_current_config
 *
 * Description: Gets the current configuration for the given handles.
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_config - Pointer to get config
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
static int wrapper_get_current_config(faceproc_sw_wrap_t *p_hnd, fpsww_config_t *p_config)
{
  if (!p_hnd || !p_config) {
    IDBG_ERROR("NULL param p_hnd=%p, p_config=%p", p_hnd, p_config);
    return IMG_ERR_INVALID_INPUT;
  }

  int rc;

  rc = fpsww_co_get_config(p_hnd, &p_config->co_params);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in getting CO config");
    return rc;
  }

  rc = fpsww_dt_get_config(p_hnd, &p_config->dt_params);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in getting DT config");
    return rc;
  }

  if (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) {
    rc = fpsww_dsp_get_config(p_hnd, &p_config->dsp_params);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in dsp custom get config");
      return rc;
    }
  }

  if (p_hnd->create_params.create_face_parts == TRUE) {
    rc = fpsww_pt_get_config(p_hnd, &p_config->pt_params);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in getting PT config");
      return rc;
    }

    rc = fpsww_ct_get_config(p_hnd, &p_config->ct_params);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in getting CT config");
      return rc;
    }

    rc = fpsww_sm_get_config(p_hnd, &p_config->sm_params);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in getting SM config");
      return rc;
    }

    rc = fpsww_gb_get_config(p_hnd, &p_config->gb_params);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in getting GB config");
      return rc;
    }
  }

  if (p_hnd->create_params.create_face_recog == TRUE) {
    rc = fpsww_fr_get_config(p_hnd, &p_config->fr_params);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in getting FR config");
      return rc;
    }
  }

  return rc;
}

/**
 * Function: wrapper_filter_print_config
 *
 * Description: Print current filter config
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_filter_params - Filter params
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
static int wrapper_filter_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_filter_params_t *p_filter_params)
{
  IDBG_INFO("========================= Filter Config =========================");
  IDBG_INFO("enable_sw_false_pos_filtering : %d", p_filter_params->enable_sw_false_pos_filtering);
  IDBG_INFO("sw_face_size_perc             : %d", p_filter_params->sw_face_size_perc);
  IDBG_INFO("sw_face_box_border_per        : %d", p_filter_params->sw_face_box_border_per);
  IDBG_INFO("sw_face_spread_tol            : %f", p_filter_params->sw_face_spread_tol);
  IDBG_INFO("sw_face_discard_border        : %d", p_filter_params->sw_face_discard_border);
  IDBG_INFO("sw_face_discard_out           : %d", p_filter_params->sw_face_discard_out);

  if (p_hnd->config.enable_facial_parts) {
    IDBG_INFO("enable_fp_false_pos_filtering : %d", p_filter_params->enable_fp_false_pos_filtering);
    IDBG_INFO("discard_threshold             : %d", p_filter_params->discard_threshold);
    IDBG_INFO("min_threshold                 : %d", p_filter_params->min_threshold);
    IDBG_INFO("closed_eye_ratio              : %d", p_filter_params->closed_eye_ratio);
    IDBG_INFO("weights                       : %f(eye), %f(mouth), %f(nose), %f(face)",
      p_filter_params->weight_eyes, p_filter_params->weight_mouth,
      p_filter_params->weight_nose, p_filter_params->weight_face);
    IDBG_INFO("use_max_filter                : %d(eye) %d(nose)",
      p_filter_params->eyes_use_max_filter, p_filter_params->nose_use_max_filter);
    IDBG_INFO("discard_face_below            : %d", p_filter_params->discard_face_below);
    IDBG_INFO("discard_facialparts_threshold : %d", p_filter_params->discard_facialparts_threshold);
  }

  return IMG_SUCCESS;
}

/**
 * Function: wrapper_print_current_config
 *
 * Description: Print current sw wrapper config
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
static int wrapper_print_current_config(faceproc_sw_wrap_t *p_hnd)
{
  if (!p_hnd) {
    IDBG_ERROR("NULL param");
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_INFO("========================= Features Config =========================");
  IDBG_INFO("enable : facial_parts=%d, contour=%d, "
    "smile=%d, gaze=%d, blink=%d, recog=%d",
    p_hnd->config.enable_facial_parts, p_hnd->config.enable_contour,
    p_hnd->config.enable_smile, p_hnd->config.enable_gaze,
    p_hnd->config.enable_blink, p_hnd->config.enable_recog);

  fpsww_co_print_config(p_hnd, &p_hnd->config.co_params);
  fpsww_dt_print_config(p_hnd, &p_hnd->config.dt_params);
  fpsww_dsp_print_config(p_hnd, &p_hnd->config.dsp_params);

  if (p_hnd->config.enable_facial_parts) {
    fpsww_pt_print_config(p_hnd, &p_hnd->config.pt_params);
    fpsww_ct_print_config(p_hnd, &p_hnd->config.ct_params);
    fpsww_sm_print_config(p_hnd, &p_hnd->config.sm_params);
    fpsww_gb_print_config(p_hnd, &p_hnd->config.gb_params);
    fpsww_fr_print_config(p_hnd, &p_hnd->config.fr_params);
  }

  if (p_hnd->create_params.engine == FACEPROC_ENGINE_HW) {
    wrapper_filter_print_config(p_hnd, &p_hnd->config.filter_params);
  }

  return IMG_SUCCESS;
}

/**
 * Function: wrapper_fill_face_info_from_dtresult
 *
 * Description: Fill internal faceproc data structures with face info retrieved from dt results
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_faceinfo - face info to be filled
 *   p_dt_results - face dt results information
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
static int wrapper_fill_face_info_from_dtresult(faceproc_sw_wrap_t *p_hnd, faceproc_info_t *p_faceinfo,
  DETECTION_INFO *p_dt_results)
{
  int32_t frame_width  = p_hnd->width;
  int32_t frame_height = p_hnd->height;

  /* Translate the data */
  /* Clip center of face coordinates to be within the frame boundary - face width/height*/
  CLIP(p_dt_results->ptCenter.x, (0 + (p_dt_results->nWidth >> 1)),  (frame_width - (p_dt_results->nWidth >> 1)));
  CLIP(p_dt_results->ptCenter.y, (0 + (p_dt_results->nHeight >> 1)), (frame_height - (p_dt_results->nHeight >> 1)));

  p_faceinfo->face_boundary.dx = p_dt_results->nWidth;
  p_faceinfo->face_boundary.dy = p_dt_results->nHeight;

  // Do not fill nID, instead fill in the caller. HW comp may not want nId
  p_faceinfo->face_boundary.x = (p_dt_results->ptCenter.x - (p_dt_results->nWidth >> 1));
  p_faceinfo->face_boundary.y = (p_dt_results->ptCenter.y - (p_dt_results->nHeight >> 1));

  p_faceinfo->gaze_angle      = p_dt_results->nPose;
  p_faceinfo->fd_confidence   = p_dt_results->nConfidence;
  p_faceinfo->face_angle_roll = p_dt_results->nAngle;

  IDBG_HIGH("confidence is %d (threshold=%d", p_faceinfo->fd_confidence, p_hnd->config.dt_params.nThreshold);

  return IMG_SUCCESS;
}

/**
 * Function: wrapper_fill_fp_info_from_ptresult
 *
 * Description: Fill internal faceproc data structures with facial parts info retrieved from pt results
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_fp - facial parts info to be filled
 *   face_indx - face index
 *   p_pt_results - face pt results information
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
static int wrapper_fill_fp_info_from_ptresult(faceproc_sw_wrap_t *p_hnd, face_part_detect *p_fp, int face_indx,
  face_part_detect *p_pt_results)
{
  uint32_t discard_facialparts_threshold = p_hnd->config.filter_params.discard_facialparts_threshold;

  IDBG_HIGH("PT : Chromatix discard_facialparts_threshold %d", discard_facialparts_threshold);

  IDBG_HIGH("PT : Mouth confidence %d", p_pt_results->confidence[FACE_PART_MOUTH]);

  if ((uint32_t)p_pt_results->confidence[FACE_PART_MOUTH] > discard_facialparts_threshold) {
    p_fp->face_pt[FACE_PART_MOUTH]       = p_pt_results->face_pt[FACE_PART_MOUTH];
    p_fp->face_pt[FACE_PART_MOUTH_UP]    = p_pt_results->face_pt[FACE_PART_MOUTH_UP];
    p_fp->face_pt[FACE_PART_MOUTH_LEFT]  = p_pt_results->face_pt[FACE_PART_MOUTH_LEFT];
    p_fp->face_pt[FACE_PART_MOUTH_RIGHT] = p_pt_results->face_pt[FACE_PART_MOUTH_RIGHT];

    p_fp->face_pt_valid[FACE_PART_MOUTH]       = 1;
    p_fp->face_pt_valid[FACE_PART_MOUTH_UP]    = 1;
    p_fp->face_pt_valid[FACE_PART_MOUTH_LEFT]  = 1;
    p_fp->face_pt_valid[FACE_PART_MOUTH_RIGHT] = 1;

    IDBG_HIGH("PT : Mouth co-ordinates MOUTH:(%d, %d), MOUTH_UP=(%d, %d), MOUTH_LEFT=(%d, %d), MOUTH_RIGHT=(%d, %d)",
      p_fp->face_pt[FACE_PART_MOUTH].x,       p_fp->face_pt[FACE_PART_MOUTH].y,
      p_fp->face_pt[FACE_PART_MOUTH_UP].x,    p_fp->face_pt[FACE_PART_MOUTH_UP].y,
      p_fp->face_pt[FACE_PART_MOUTH_LEFT].x,  p_fp->face_pt[FACE_PART_MOUTH_LEFT].y,
      p_fp->face_pt[FACE_PART_MOUTH_RIGHT].x, p_fp->face_pt[FACE_PART_MOUTH_RIGHT].y);
  }

  IDBG_HIGH("PT : Eye confidence %d(left) %d(right)",
    p_pt_results->confidence[FACE_PART_LEFT_EYE], p_pt_results->confidence[FACE_PART_RIGHT_EYE]);

  if (((uint32_t)p_pt_results->confidence[FACE_PART_LEFT_EYE]  > discard_facialparts_threshold) ||
      ((uint32_t)p_pt_results->confidence[FACE_PART_RIGHT_EYE] > discard_facialparts_threshold)) {
    p_fp->face_pt[FACE_PART_LEFT_EYE]      = p_pt_results->face_pt[FACE_PART_LEFT_EYE];
    p_fp->face_pt[FACE_PART_RIGHT_EYE]     = p_pt_results->face_pt[FACE_PART_RIGHT_EYE];
    p_fp->face_pt[FACE_PART_LEFT_EYE_IN]   = p_pt_results->face_pt[FACE_PART_LEFT_EYE_IN];
    p_fp->face_pt[FACE_PART_LEFT_EYE_OUT]  = p_pt_results->face_pt[FACE_PART_LEFT_EYE_OUT];
    p_fp->face_pt[FACE_PART_RIGHT_EYE_IN]  = p_pt_results->face_pt[FACE_PART_RIGHT_EYE_IN];
    p_fp->face_pt[FACE_PART_RIGHT_EYE_OUT] = p_pt_results->face_pt[FACE_PART_RIGHT_EYE_OUT];

    p_fp->face_pt_valid[FACE_PART_LEFT_EYE]      = 1;
    p_fp->face_pt_valid[FACE_PART_RIGHT_EYE]     = 1;
    p_fp->face_pt_valid[FACE_PART_LEFT_EYE_IN]   = 1;
    p_fp->face_pt_valid[FACE_PART_LEFT_EYE_OUT]  = 1;
    p_fp->face_pt_valid[FACE_PART_RIGHT_EYE_IN]  = 1;
    p_fp->face_pt_valid[FACE_PART_RIGHT_EYE_OUT] = 1;

    IDBG_HIGH("PT : Eye co-ordinates LEFT_EYE:(%d, %d), RIGHT_EYE=(%d, %d), "
      "LEFT_EYE_IN=(%d, %d), LEFT_EYE_OUT=(%d, %d), RIGHT_EYE_IN=(%d, %d), LEFT_EYE_OUT=(%d, %d)",
      p_fp->face_pt[FACE_PART_LEFT_EYE].x,      p_fp->face_pt[FACE_PART_LEFT_EYE].y,
      p_fp->face_pt[FACE_PART_RIGHT_EYE].x,     p_fp->face_pt[FACE_PART_RIGHT_EYE].y,
      p_fp->face_pt[FACE_PART_LEFT_EYE_IN].x,   p_fp->face_pt[FACE_PART_LEFT_EYE_IN].y,
      p_fp->face_pt[FACE_PART_LEFT_EYE_OUT].x,  p_fp->face_pt[FACE_PART_LEFT_EYE_OUT].y,
      p_fp->face_pt[FACE_PART_RIGHT_EYE_IN].x,  p_fp->face_pt[FACE_PART_RIGHT_EYE_IN].y,
      p_fp->face_pt[FACE_PART_RIGHT_EYE_OUT].x, p_fp->face_pt[FACE_PART_RIGHT_EYE_OUT].y);
  }

  IDBG_HIGH("PT : Nose confidence %d(left) %d(right)",
    p_pt_results->confidence[FACE_PART_NOSE_LEFT], p_pt_results->confidence[FACE_PART_NOSE_RIGHT]);

  if (((uint32_t)p_pt_results->confidence[FACE_PART_NOSE_LEFT] > discard_facialparts_threshold) ||
      ((uint32_t)p_pt_results->confidence[FACE_PART_NOSE_RIGHT] > discard_facialparts_threshold)) {
    p_fp->face_pt[FACE_PART_NOSE_LEFT]  = p_pt_results->face_pt[FACE_PART_NOSE_LEFT];
    p_fp->face_pt[FACE_PART_NOSE_RIGHT] = p_pt_results->face_pt[FACE_PART_NOSE_RIGHT];

    p_fp->face_pt_valid[FACE_PART_NOSE_LEFT]  = 1;
    p_fp->face_pt_valid[FACE_PART_NOSE_RIGHT] = 1;

    IDBG_HIGH("PT : Nose co-ordinates NOSE_LEFT:(%d, %d), NOSE_RIGHT=(%d, %d)",
      p_fp->face_pt[FACE_PART_NOSE_LEFT].x,  p_fp->face_pt[FACE_PART_NOSE_LEFT].y,
      p_fp->face_pt[FACE_PART_NOSE_RIGHT].x, p_fp->face_pt[FACE_PART_NOSE_RIGHT].y);
  }

  p_fp->direction_up_down    = p_pt_results->direction_up_down;
  p_fp->direction_left_right = p_pt_results->direction_left_right;
  p_fp->direction_roll       = p_pt_results->direction_roll;

  IDBG_HIGH("PT : Face Direction info : %d(up_down) %d(left_right) %d(roll)",
    p_fp->direction_up_down, p_fp->direction_left_right,
    p_fp->direction_roll);

  return IMG_SUCCESS;
}

/**
 * Function: wrapper_fill_ct_info_from_ctresult
 *
 * Description: Fill internal faceproc data structures with contour points info retrieved from ct results
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_ct - contour info to be filled
 *   face_indx - face index
 *   p_ct_results - face ct results information
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
static int wrapper_fill_ct_info_from_ctresult(faceproc_sw_wrap_t *p_hnd, contour_detect *p_ct, int face_indx,
  fd_contour_results *p_ct_results)
{
  uint32_t ct;

  p_ct->is_eye_valid = 1;
  for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_eye); ct++) {
    p_ct->eye_pt[convert_contour_eye[ct][0]].x = p_ct_results->contour_pt[convert_contour_eye[ct][1]].x;
    p_ct->eye_pt[convert_contour_eye[ct][0]].y = p_ct_results->contour_pt[convert_contour_eye[ct][1]].y;

    IDBG_HIGH("CT : Face[%d] CT_POINT_EYE[%d] is (%d, %d)", face_indx, convert_contour_eye[ct][0],
      p_ct->eye_pt[convert_contour_eye[ct][0]].x, p_ct->eye_pt[convert_contour_eye[ct][0]].y);
  }

  if (p_hnd->config.ct_params.nMode == FD_CONTOUR_MODE_DEFAULT) {
    p_ct->is_forehead_valid = 1;
    for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_forh); ct++) {
      p_ct->forh_pt[convert_contour_forh[ct][0]].x = p_ct_results->contour_pt[convert_contour_forh[ct][1]].x;
      p_ct->forh_pt[convert_contour_forh[ct][0]].y = p_ct_results->contour_pt[convert_contour_forh[ct][1]].y;

      IDBG_HIGH("CT : Face[%d] CT_POINT_FORH[%d] is (%d, %d)", face_indx, convert_contour_forh[ct][0],
        p_ct->forh_pt[convert_contour_forh[ct][0]].x, p_ct->forh_pt[convert_contour_forh[ct][0]].y);
    }

    p_ct->is_nose_valid = 1;
    for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_nose); ct++) {
      p_ct->nose_pt[convert_contour_nose[ct][0]].x = p_ct_results->contour_pt[convert_contour_nose[ct][1]].x;
      p_ct->nose_pt[convert_contour_nose[ct][0]].y = p_ct_results->contour_pt[convert_contour_nose[ct][1]].y;

      IDBG_HIGH("CT : Face[%d] CT_POINT_NOSE[%d] is (%d, %d)", face_indx, convert_contour_nose[ct][0],
        p_ct->nose_pt[convert_contour_nose[ct][0]].x, p_ct->nose_pt[convert_contour_nose[ct][0]].y);
    }

    p_ct->is_mouth_valid = 1;
    for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_mouth); ct++) {
      p_ct->mouth_pt[convert_contour_mouth[ct][0]].x = p_ct_results->contour_pt[convert_contour_mouth[ct][1]].x;
      p_ct->mouth_pt[convert_contour_mouth[ct][0]].y = p_ct_results->contour_pt[convert_contour_mouth[ct][1]].y;

      IDBG_HIGH("CT : Face[%d] CT_POINT_MOUTH[%d] is (%d, %d)", face_indx, convert_contour_mouth[ct][0],
        p_ct->mouth_pt[convert_contour_mouth[ct][0]].x, p_ct->mouth_pt[convert_contour_mouth[ct][0]].y);
    }

    p_ct->is_lip_valid = 1;
    for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_lip); ct++) {
      p_ct->lip_pt[convert_contour_lip[ct][0]].x = p_ct_results->contour_pt[convert_contour_lip[ct][1]].x;
      p_ct->lip_pt[convert_contour_lip[ct][0]].y = p_ct_results->contour_pt[convert_contour_lip[ct][1]].y;

      IDBG_HIGH("CT : Face[%d] CT_POINT_LIP[%d] is (%d, %d)", face_indx, convert_contour_lip[ct][0],
        p_ct->lip_pt[convert_contour_lip[ct][0]].x, p_ct->lip_pt[convert_contour_lip[ct][0]].y);
    }

    p_ct->is_brow_valid = 1;
    for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_brow); ct++) {
      p_ct->brow_pt[convert_contour_brow[ct][0]].x = p_ct_results->contour_pt[convert_contour_brow[ct][1]].x;
      p_ct->brow_pt[convert_contour_brow[ct][0]].y = p_ct_results->contour_pt[convert_contour_brow[ct][1]].y;

      IDBG_HIGH("CT : Face[%d] CT_POINT_BROW[%d] is (%d, %d)", face_indx, convert_contour_brow[ct][0],
        p_ct->brow_pt[convert_contour_brow[ct][0]].x, p_ct->brow_pt[convert_contour_brow[ct][0]].y);
    }

    p_ct->is_chin_valid = 1;
    for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_chin); ct++) {
      p_ct->chin_pt[convert_contour_chin[ct][0]].x = p_ct_results->contour_pt[convert_contour_chin[ct][1]].x;
      p_ct->chin_pt[convert_contour_chin[ct][0]].y = p_ct_results->contour_pt[convert_contour_chin[ct][1]].y;

      IDBG_HIGH("CT : Face[%d] CT_POINT_CHIN[%d] is (%d, %d)", face_indx, convert_contour_chin[ct][0],
        p_ct->chin_pt[convert_contour_chin[ct][0]].x, p_ct->chin_pt[convert_contour_chin[ct][0]].y);
    }

    p_ct->is_ear_valid = 1;
    for (ct = 0; ct < IMGLIB_ARRAY_SIZE(convert_contour_ear); ct++) {
      p_ct->ear_pt[convert_contour_ear[ct][0]].x = p_ct_results->contour_pt[convert_contour_ear[ct][1]].x;
      p_ct->ear_pt[convert_contour_ear[ct][0]].y = p_ct_results->contour_pt[convert_contour_ear[ct][1]].y;

      IDBG_HIGH("CT : Face[%d] CT_POINT_EAR[%d] is (%d, %d)", face_indx, convert_contour_ear[ct][0],
        p_ct->ear_pt[convert_contour_ear[ct][0]].x, p_ct->ear_pt[convert_contour_ear[ct][0]].y);
    }
  }
  return IMG_SUCCESS;
}

// Use to get results only or execute+results
// Do not use this for execute only

/**
 * Function: wrapper_execute_bsgc_from_pthandle
 *
 * Description: Execute bsgc
 *     This function can be called in different ways to get the desired behavior:
 *       1. To get results info only for already executed results : Call with get_results_only=TRUE, p_ct_results=non NULL
 *       2. To execute + get results at the same time : Call this function with get_results_only=FALSE, p_ct_results=non NULL
 *    For execute only, do not use this function, instead call corresponding bsgc execute functions directly.
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle
 *   p_frame - pointer to img_frame
 *   face_indx - face index
 *   get_results_only - whether to get results only
 *   p_faceinfo - results to be filled if non-NULL
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
static int wrapper_execute_bsgc_from_pthandle(faceproc_sw_wrap_t *p_hnd, img_frame_t *p_frame, int32_t face_indx,
  bool get_results_only, faceproc_info_t *p_faceinfo)
{
  int rc = IMG_SUCCESS;

  // Contour
  if (p_hnd->config.enable_contour == TRUE) {
    IDBG_MED("Executing CONTOUR");
    p_faceinfo->ct.is_eye_valid      = 0;
    p_faceinfo->ct.is_forehead_valid = 0;
    p_faceinfo->ct.is_nose_valid     = 0;
    p_faceinfo->ct.is_mouth_valid    = 0;
    p_faceinfo->ct.is_lip_valid      = 0;
    p_faceinfo->ct.is_brow_valid     = 0;
    p_faceinfo->ct.is_chin_valid     = 0;
    p_faceinfo->ct.is_ear_valid      = 0;

    fd_contour_results ct_results;

    // Get Results only
    rc = fpsww_ct_execute_from_pthandle(p_hnd, p_frame, face_indx, get_results_only, &ct_results);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Failed in CT Get result rc=%d", rc);
      return rc;
    }

    rc = wrapper_fill_ct_info_from_ctresult(p_hnd, &p_faceinfo->ct, face_indx, &ct_results);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Failed in filling CT info, rc=%d", rc);
      return rc;
    }
  }

  if (p_hnd->config.enable_smile == TRUE) {
    fd_smile_detect sm_results;
    IDBG_MED("Executing SMILE");

    // Get Results only
    rc = fpsww_sm_execute_from_pthandle(p_hnd, p_frame, face_indx, get_results_only, &sm_results);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Failed in SM Get result rc=%d", rc);
      return rc;
    }

    p_faceinfo->sm.smile_degree = sm_results.smile_degree;
    p_faceinfo->sm.confidence   = sm_results.confidence;

    IDBG_HIGH("SM : Face[%d] smile_degree=%d, smile_confidence=%d", face_indx,
      p_faceinfo->sm.smile_degree, p_faceinfo->sm.confidence);
  }

  p_faceinfo->blink_detected = 0;
  if ((p_hnd->config.enable_blink == TRUE) || (p_hnd->config.enable_gaze == TRUE)) {
    fd_gb_result gb_results;
    IDBG_MED("Executing BLINK, GAZE");

    // Get Results only
    rc = fpsww_gb_execute_from_pthandle(p_hnd, p_frame, face_indx, get_results_only, &gb_results);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Failed in CT Get result rc=%d", rc);
      return rc;
    }

    p_faceinfo->left_blink  = gb_results.left_blink;
    p_faceinfo->right_blink = gb_results.right_blink;
    IDBG_HIGH("GB : Face[%d] BLINK Ratio %d(left), %d(right)", face_indx,
      p_faceinfo->left_blink, p_faceinfo->right_blink);

    if ((p_faceinfo->left_blink  > FACE_EYE_CLOSED_THRESHOLD) &&
        (p_faceinfo->right_blink > FACE_EYE_CLOSED_THRESHOLD)) {
      IDBG_MED("EYES CLOSED");
      p_faceinfo->blink_detected = 1;
    }

    p_faceinfo->left_right_gaze = gb_results.left_right_gaze;
    p_faceinfo->top_bottom_gaze = gb_results.top_bottom_gaze;
    IDBG_HIGH("GB : Face[%d] : Gaze Ratio  left_right %d, top_bottom %d", face_indx,
      gb_results.left_right_gaze, gb_results.top_bottom_gaze);
  }

  return IMG_SUCCESS;
}

/**
 * Function: wrapper_discard_face_sw_scan.
 *
 * Description: Discard face based on sw face detection scan.
 *
 * Input parameters:
 *   p_hnd - faceproc sw wrapper handle.
 *   p_frame - Pointer to img frame.
 *   p_output - Pointer to face detection result.
 *   face_tracked - Whether the face is tracked.
 *   p_old - Pointer to old results for this face.
 *
 * Return values:
 *     IMG_SUCCESS - on Success.
 *     IMG_ERR_NOT_FOUND - When face need to be filtered is not found.
 *     IMG_ERR_GENERAL - General error
 **/
static int wrapper_discard_face_sw_scan(faceproc_sw_wrap_t *p_hnd,
  img_frame_t *p_frame, faceproc_info_t *p_output, int face_tracked,
  faceproc_info_t *p_old)
{
  uint32_t an_still_angle[FPSWW_MAX_POSE_ANGLES];
  RECT     rect_mask;
  uint32_t angle_mask;
  int32_t  num_faces;
  int      additional_pixels;
  int      max_face_size;
  int      rc;
  uint8_t  prof_angle;
  uint32_t min_face_size;
  int      i;
//  uint32_t dens_offset = 0;

  if (!p_hnd->config.dt_params.nThreshold) {
    return IMG_SUCCESS;
  }

  // Get current params
  fpsww_dt_params_t dt_params = p_hnd->config.dt_params;

  dt_params.nSearchDensity = faceproc_common_validate_and_get_density(
    true, p_hnd->config.filter_params.sw_face_search_dens, face_tracked);

  prof_angle = (uint8_t)abs(p_output->gaze_angle);

  memset(an_still_angle, 0x00, sizeof(an_still_angle));
  angle_mask = facial_angle_mask(p_output->face_angle_roll, 3);

  switch (p_output->gaze_angle) {
    case FD_POSE_FRONT_ANGLE :
      an_still_angle[FPSWW_ANGLE_FRONT]       = angle_mask;
      an_still_angle[FPSWW_ANGLE_HALFPROFILE] = angle_mask;
      an_still_angle[FPSWW_ANGLE_FULLPROFILE] = ROLL_ANGLE_NONE;
      break;
    case FD_POSE_LEFT_DIAGONAL_ANGLE :
    case FD_POSE_RIGHT_DIAGONAL_ANGLE :
      an_still_angle[FPSWW_ANGLE_FRONT]       = angle_mask;
      an_still_angle[FPSWW_ANGLE_HALFPROFILE] = angle_mask;
      an_still_angle[FPSWW_ANGLE_FULLPROFILE] = angle_mask;
      break;
    case FD_POSE_LEFT_ANGLE :
    case FD_POSE_RIGHT_ANGLE :
      an_still_angle[FPSWW_ANGLE_FRONT]       = ROLL_ANGLE_NONE;
      an_still_angle[FPSWW_ANGLE_HALFPROFILE] = angle_mask;
      an_still_angle[FPSWW_ANGLE_FULLPROFILE] = angle_mask;
      break;
    default:
      an_still_angle[FPSWW_ANGLE_FRONT]       = angle_mask;
      an_still_angle[FPSWW_ANGLE_HALFPROFILE] = angle_mask;
      an_still_angle[FPSWW_ANGLE_FULLPROFILE] = angle_mask;
      break;
  }

  dt_params.nAngle[FPSWW_ANGLE_FRONT]       = an_still_angle[FPSWW_ANGLE_FRONT];
  dt_params.nAngle[FPSWW_ANGLE_HALFPROFILE] = an_still_angle[FPSWW_ANGLE_HALFPROFILE];
  dt_params.nAngle[FPSWW_ANGLE_FULLPROFILE] = an_still_angle[FPSWW_ANGLE_FULLPROFILE];

  additional_pixels = (p_output->face_boundary.dx * p_hnd->config.filter_params.sw_face_box_border_per) / 100;

  rect_mask.top    = p_output->face_boundary.y;
  rect_mask.left   = p_output->face_boundary.x;
  rect_mask.right  = p_output->face_boundary.x + p_output->face_boundary.dx;
  rect_mask.bottom = p_output->face_boundary.y + p_output->face_boundary.dy;

  rect_mask.top    -= additional_pixels;
  rect_mask.left   -= additional_pixels;
  rect_mask.right  += additional_pixels;
  rect_mask.bottom += additional_pixels;

  CLIP(rect_mask.left,   0, ((int)IMG_WIDTH(p_frame) - 1));
  CLIP(rect_mask.right,  0, ((int)IMG_WIDTH(p_frame) - 1));
  CLIP(rect_mask.top,    0, ((int)IMG_HEIGHT(p_frame) - 1));
  CLIP(rect_mask.bottom, 0, ((int)IMG_HEIGHT(p_frame) - 1));

  dt_params.faceSearchEdgeMask = rect_mask;

  additional_pixels = (p_output->face_boundary.dx * p_hnd->config.filter_params.sw_face_size_perc) / 100;

  max_face_size = p_output->face_boundary.dx + (additional_pixels * 2) ;
  if (max_face_size > rect_mask.bottom - rect_mask.top) {
    max_face_size = rect_mask.bottom - rect_mask.top;
  }

  if (max_face_size > rect_mask.right - rect_mask.left) {
    max_face_size = rect_mask.right - rect_mask.left;
  }

  min_face_size = MAX((p_output->face_boundary.dx *
    (100 - p_hnd->config.filter_params.sw_face_size_perc) / 100), 20);

  dt_params.nMinSize = min_face_size;
  dt_params.nMaxSize = max_face_size;

  rc = fpsww_dt_set_config(p_hnd, &dt_params);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("fpsww_dt_set_config returned error: %d", rc);
    return IMG_ERR_GENERAL;
  }

  if (IMG_LOG_LEVEL_MED) {
    IDBG_INFO("Config for the current face false positive verification");
    wrapper_print_current_config(p_hnd);
  }

  IDBG_HIGH("Executing SW FD for false positive filtering (threshold is %d)", p_hnd->config.dt_params.nThreshold);

  num_faces = 0;
  rc = fpsww_dt_execute(p_hnd, p_frame, FALSE, &num_faces);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in DT execute");
    return rc;
  }

  DETECTION_INFO detection_info;
  memset(&detection_info, 0x00, sizeof(detection_info));

  IDBG_HIGH("SW FP filtering, num_faces detected %d", num_faces);

  double   nearest_distance = 0;
  double   FaceSqDist       = 0;
  boolean  face_found       = false;
  boolean  update_face_info = false;
  double   fTol             = p_hnd->config.filter_params.sw_face_spread_tol;
  uint32_t hw_size          = p_output->face_boundary.dx;
  POINT    hw_center;

  hw_center.x = p_output->face_boundary.x + (p_output->face_boundary.dx >> 1);
  hw_center.y = p_output->face_boundary.y + (p_output->face_boundary.dx >> 1);

  for (i = 0; i < num_faces; i++) {
    rc = fpsww_dt_get_result_info(p_hnd, &detection_info, i, FALSE);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("fpsww_dt_get_result_info returned error: %d", rc);
      return IMG_ERR_GENERAL;
    }

    // invalidate the face if spread tolerance is set and face info is beyond tolerance
    if (p_hnd->config.filter_params.sw_face_spread_tol != 0) {
      long DistX = hw_center.x - detection_info.ptCenter.x;
      long DistY = hw_center.y - detection_info.ptCenter.y;
      FaceSqDist = (double)(DistX * DistX + DistY * DistY);
      if( FaceSqDist > (pow( (hw_size * fTol) , 2.0))){
        IDBG_HIGH("Rejecting due to spread tolerance");
        continue;
      }
    }

    // If there are multiple faces detected within the detection area,
    // accept the one which is nearer to the input Face ROI
    if (face_found == false) {
      nearest_distance = FaceSqDist;
      face_found = true;
      update_face_info = true;
    } else if (nearest_distance > FaceSqDist) {
      nearest_distance = FaceSqDist;
      update_face_info = true;
    } else {
      update_face_info = false;
    }

    IDBG_MED("FP filterResults : Face[%d] (Distance=%ld, update=%d) : ID=%d, Confidence=%d, "
      "Center =(%d, %d) w=%d, h=%d, Angle=%d, Pose=%d",
      i, FaceSqDist, update_face_info,
      detection_info.nID, detection_info.nConfidence,
      detection_info.ptCenter.x, detection_info.ptCenter.y, detection_info.nWidth, detection_info.nHeight,
      detection_info.nAngle, detection_info.nPose);

    if (update_face_info == true) {
      rc = wrapper_fill_face_info_from_dtresult(p_hnd, p_output, &detection_info);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("Failed in filling FacialParts info, rc=%d", rc);
        return rc;
      }
    }
  }

  if (face_found == true) {
    return IMG_SUCCESS;
  } else {
    return IMG_ERR_NOT_FOUND;
  }
}

/**
 * Function: faceproc_sw_wrapper_process_result.
 *
 * Description: Facial part wrapper process.
 *     Executes false positive filtering based on flag, executes and gets
 *     facial parts results (based on enable flags in config). This function needs to be called for every face by the client.
 *
 * Input parameters:
 *   p_handle - faceproc sw wrapper handle.
 *   p_frame - Pointer to img frame.
 *   p_output - Pointer to face detection result.
 *   face_tracked - Whether the face is tracked.
 *   p_old - Pointer to old results for this face.
 *   face_index - Face index
 *   run_facial_parts - Whether to run facial parts.
 *
 * Return values:
 *     IMG_SUCCESS - on Success.
 *     IMG_ERR_NOT_FOUND - When face need to be filtered is not found.
 *     IMG_ERR_GENERAL - General error
 **/
int faceproc_sw_wrapper_process_result(void *p_handle, img_frame_t *p_frame, faceproc_info_t *p_output, int verify_face,
  int face_tracked, faceproc_info_t *p_old, int32_t face_index, bool run_facial_parts)
{
  if (!p_handle || !p_output || !p_frame ) {
    IDBG_ERROR("Invalid input %p %p %p", p_handle, p_frame, p_output);
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_sw_wrap_t *p_hnd = (faceproc_sw_wrap_t *)p_handle;
  int                 rc    = IMG_SUCCESS;
  uint32_t            delta;
  bool                profile_fd =
    (p_hnd->debug_settings.profile_fd_level >= FD_PROFILE_LEVEL_FACE) ?
    TRUE : FALSE;

  if (profile_fd == TRUE) {
    if (p_hnd->frame_id != p_frame->frame_id) {
      // New frame, increment frames processed
      p_hnd->fd_profile.num_frames_processed++;
    }
    IMG_TIMER_START(p_hnd->fd_profile.start_time[FD_FRAME_PROCESSING_TIME]);
  }

  p_hnd->width    = IMG_WIDTH(p_frame);
  p_hnd->height   = IMG_HEIGHT(p_frame);
  p_hnd->frame_id = p_frame->frame_id;

  IDBG_LOW("Create Params : engine=%s, face_index=%d",
    (p_hnd->create_params.engine == FACEPROC_ENGINE_HW) ? "FACEPROC_ENGINE_HW" :
    ((p_hnd->create_params.engine == FACEPROC_ENGINE_DSP) ? "FACEPROC_ENGINE_DSP" : "FACEPROC_ENGINE_SW"),
    face_index);

  IDBG_HIGH("[OFFLINE_FD] myface x %d , y %d, dx %d, dy %d verify_face %d %d %d",
    p_output->face_boundary.x,
    p_output->face_boundary.y,
    p_output->face_boundary.dx,
    p_output->face_boundary.dy,
    verify_face,
    p_hnd->config.filter_params.enable_sw_false_pos_filtering,
    p_hnd->config.filter_params.enable_fp_false_pos_filtering);

  /* Disable face verification if false positive filtering is disabled */
  if (!p_hnd->config.filter_params.enable_sw_false_pos_filtering &&
      !p_hnd->config.filter_params.enable_fp_false_pos_filtering) {
    verify_face = 0;
  }

  /* Check if if we need to discard this face based on sw scan */
  if (verify_face && p_hnd->config.filter_params.enable_sw_false_pos_filtering) {
    rc = wrapper_discard_face_sw_scan(p_hnd, p_frame, p_output, face_tracked, p_old);
    if (rc != IMG_SUCCESS) {
      IDBG_HIGH("False Possitive, Skip Face detection unique id %d", p_output->unique_id);
      rc = IMG_ERR_NOT_FOUND;
      goto end;
    }

    IDBG_HIGH("[SW_FD_FILTER] myface x %d , y %d, dx %d, dy %d",
      p_output->face_boundary.x,
      p_output->face_boundary.y,
      p_output->face_boundary.dx,
      p_output->face_boundary.dy);
    verify_face = 0; /* skip fp verification */
  }

  if (!run_facial_parts) {
    // Do not run facial parts even if enabled. Caller will call again to execute facial parts
    rc = IMG_SUCCESS;
    goto end;
  }

  // Todo : This check should actually based on config.enable_facial_parts
  /* Check if we need facial part scanning for this frame face */
  if (!verify_face && !p_hnd->config.enable_blink &&
    !p_hnd->config.enable_gaze && !p_hnd->config.enable_smile &&
    !p_hnd->config.filter_params.enable_fp_false_pos_filtering) {
    IDBG_MED("Skip facial part detection");
    rc = IMG_SUCCESS;
    goto end;
  }

  /* initialize output fields */
  p_output->blink_detected  = 0;
  p_output->left_blink      = 0;
  p_output->right_blink     = 0;
  p_output->left_right_gaze = 0;
  p_output->top_bottom_gaze = 0;
  p_output->sm.smile_degree = 0;
  p_output->sm.confidence   = 0;

  /* Face part detection */
  face_part_detect pt_results;
  POINT            center;
  center.x = p_output->face_boundary.x + p_output->face_boundary.dx / 2;
  center.y = p_output->face_boundary.y + p_output->face_boundary.dy / 2;

  IDBG_HIGH("pose %d, angle %d, center_x %d, center_y %d, size %d",
    p_output->gaze_angle, p_output->face_angle_roll, center.x, center.y, p_output->face_boundary.dx);

  rc = fpsww_pt_execute_from_positionIP(p_hnd, p_frame, 0,
    center, p_output->face_boundary.dx, p_output->face_angle_roll, 1, p_output->gaze_angle,
    FALSE, &pt_results);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("PT_PT_SetPositionIP failed");
    rc = IMG_ERR_INVALID_OPERATION;
    goto end;
  }

  if (verify_face) {
    /* Check if if we need to discard this face */
    if (wrapper_discard_face_parts_based(p_hnd, p_output, &pt_results)) {
      IDBG_HIGH("Discard faceparts in filtering");
      rc = IMG_ERR_NOT_FOUND;
      goto end;
    }
  }

  memset(&p_output->fp, 0x00, sizeof(face_part_detect));
  rc = wrapper_fill_fp_info_from_ptresult(p_hnd, &p_output->fp, 0, &pt_results);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Failed in filling FacialParts info, rc=%d", rc);
    goto end;
  }

  // Todo : Fixing only few points?. can be done for all points.
  // do this for sw fd execution as well, can be inside wrapper_fill_fp_info_from_ptresult
  wrapper_validate_and_fix_PT_coordinates(p_output);

  // Execute + Get Results for Contour, Smile, Blink, Gaze based on feature config.
  rc = wrapper_execute_bsgc_from_pthandle(p_hnd, p_frame, 0, FALSE, p_output);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Failed in getting BSGC results, info, rc=%d", rc);
    goto end;
  }

end:

  if (profile_fd == TRUE) {
    delta = IMG_TIMER_END(p_hnd->fd_profile.start_time[FD_FRAME_PROCESSING_TIME],
      p_hnd->fd_profile.end_time[FD_FRAME_PROCESSING_TIME], "FD_FaceProcess", IMG_TIMER_MODE_MS);
    p_hnd->fd_profile.total_time_in_ms[FD_FRAME_PROCESSING_TIME] += delta;
    IDBG_INFO("FDPROFILE : Frame[%d] FrameCounter[%d] Face[%d] %dx%d :\t FaceProcessingTime=%u,\t AvgFrameProcessingTime=%u",
      p_frame->frame_id, p_hnd->fd_profile.num_frames_processed, face_index,
      (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
      delta, p_hnd->fd_profile.total_time_in_ms[FD_FRAME_PROCESSING_TIME] / p_hnd->fd_profile.num_frames_processed);
  }

  return rc;
}

/**
 * Function: faceproc_sw_wrapper_is_face_tracked.
 *
 * Description: Checks whether the face with the given index is tracked or not
 *
 * Input parameters:
 *   p_handle - faceproc sw wrapper handle.
 *   index - face index
 *
 * Return values:
 *     TRUE if being tracked
 *     FALSE if not tracked
 **/
bool faceproc_sw_wrapper_is_face_tracked(void *p_handle, int index)
{
  faceproc_sw_wrap_t *p_hnd = (faceproc_sw_wrap_t *)p_handle;

  // This is assuming we always lock the faces that were detected before.
  // So we can treat the face is tracked which was locked before
  return fpsww_dt_is_face_locked(p_hnd, index);
}

/**
 * Function: faceproc_sw_wrapper_reset_result.
 *
 * Description: Reset the detection results. Tracking will be reset.
 *
 * Input parameters:
 *   p_handle - faceproc sw wrapper handle.
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 **/
int faceproc_sw_wrapper_reset_result(void *p_handle)
{
  if (!p_handle) {
    IDBG_ERROR("NULL pointer input");
    return IMG_ERR_INVALID_INPUT;
  }

  return fpsww_dt_reset_results((faceproc_sw_wrap_t *)p_handle);
}

/**
 * Function: faceproc_sw_wrapper_execute_fd.
 *
 * Description: Main sw wrapper function to execute face detection and other features.
 *     This will execute detection and optionally facial parts, contour, blink, smile and gaze
 *      based on enable flags in config  Call faceproc_sw_wrapper_get_fd_info to get the results information.
 *
 * Input parameters:
 *   p_handle - faceproc sw wrapper handle.
 *   p_frame - pointer to img_frame
 *   p_num_faces - if not NULL, this function will fill the number faces detected with this execution
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 **/
int faceproc_sw_wrapper_execute_fd(void *p_handle, img_frame_t *p_frame, int32_t *p_num_faces)
{
  if (!p_handle || !p_frame) {
    IDBG_ERROR("Null pointer p_handle=%p, p_frame=%p", p_handle, p_frame);
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_sw_wrap_t *p_hnd     = (faceproc_sw_wrap_t *)p_handle;
  int32_t             num_faces = 0;
  int                 rc;
  uint32_t            processing_time, detection_time, facial_parts_time;
  bool                profile_fd =
    (p_hnd->debug_settings.profile_fd_level >= FD_PROFILE_LEVEL_FRAME) ?
    TRUE : FALSE;

  if (profile_fd == TRUE) {
    IMG_TIMER_START(p_hnd->fd_profile.start_time[FD_FRAME_PROCESSING_TIME]);
    IMG_TIMER_START(p_hnd->fd_profile.start_time[FD_FRAME_SW_DT_TIME]);
  }

  // Store current width, height information
  p_hnd->width    = IMG_WIDTH(p_frame);
  p_hnd->height   = IMG_HEIGHT(p_frame);
  p_hnd->frame_id = p_frame->frame_id;

  rc = fpsww_dt_execute(p_hnd, p_frame, FALSE, &num_faces);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in DT execute");
    return rc;
  }

  IDBG_HIGH("DT executed on frameId %d, num_of_faces = %d", p_frame->frame_id, num_faces);

  if (num_faces > MAX_FACE_ROI) {
    num_faces = MAX_FACE_ROI;
  }

  if (p_num_faces) {
    *p_num_faces = num_faces;
  }

  if (profile_fd == TRUE) {
    detection_time = IMG_TIMER_END(p_hnd->fd_profile.start_time[FD_FRAME_SW_DT_TIME],
      p_hnd->fd_profile.end_time[FD_FRAME_SW_DT_TIME], "FD_DTProcess", IMG_TIMER_MODE_MS);
    IMG_TIMER_START(p_hnd->fd_profile.start_time[FD_FRAME_SW_FP_TIME]);
  }

  if ((p_hnd->config.enable_facial_parts == TRUE) && (num_faces > 0)) {
    uint8_t i;
    for (i = 0; i < num_faces; i++) {
      // first execute PT, in this case we will have DT handles, so execute PT based on DT handles
      rc = fpsww_pt_execute_from_dthandle(p_hnd, p_frame, i, FALSE, NULL);
      if (rc != IMG_SUCCESS) {
        IDBG_ERROR("Failed in PT execute with DT handle");
        return rc;
      }

      // CT, SM, GB can be run independently if we executed PT

      if (p_hnd->config.enable_contour == TRUE) {
        rc = fpsww_ct_execute_from_pthandle(p_hnd, p_frame, i, FALSE, NULL);
        if (rc != IMG_SUCCESS) {
          IDBG_ERROR("Failed in CT execute with DT handle");
          return rc;
        }
      }

      if (p_hnd->config.enable_smile == TRUE) {
        rc = fpsww_sm_execute_from_pthandle(p_hnd, p_frame, i, FALSE, NULL);
        if (rc != IMG_SUCCESS) {
          IDBG_ERROR("Failed in SM execute with DT handle");
          return rc;
        }
      }

      if ((p_hnd->config.enable_gaze == TRUE) || (p_hnd->config.enable_blink == TRUE)) {
        rc = fpsww_gb_execute_from_pthandle(p_hnd, p_frame, i, FALSE, NULL);
        if (rc != IMG_SUCCESS) {
          IDBG_ERROR("Failed in GB execute with DT handle");
          return rc;
        }
      }
    }
  }

  if (profile_fd == TRUE) {
    processing_time = IMG_TIMER_END(p_hnd->fd_profile.start_time[FD_FRAME_PROCESSING_TIME],
      p_hnd->fd_profile.end_time[FD_FRAME_PROCESSING_TIME], "FD_FrameProcess", IMG_TIMER_MODE_MS);
    facial_parts_time = IMG_TIMER_END(p_hnd->fd_profile.start_time[FD_FRAME_SW_FP_TIME],
      p_hnd->fd_profile.end_time[FD_FRAME_SW_FP_TIME], "FD_FrameProcess", IMG_TIMER_MODE_MS);

    p_hnd->fd_profile.total_time_in_ms[FD_FRAME_PROCESSING_TIME] += processing_time;
    p_hnd->fd_profile.total_time_in_ms[FD_FRAME_SW_DT_TIME]      += detection_time;
    p_hnd->fd_profile.total_time_in_ms[FD_FRAME_SW_FP_TIME]      += facial_parts_time;
    p_hnd->fd_profile.num_frames_processed++;

    IDBG_INFO("FDPROFILE : FrameId[%d] FrameCounter[%d] %dx%d :\t faces=%d "
      "Time=%u (DT=%u FP=%u),\t Avg=%u (DT=%u, FP=%u)",
      p_frame->frame_id, p_hnd->fd_profile.num_frames_processed,
      (int32_t)IMG_FD_WIDTH(p_frame), (int32_t)IMG_HEIGHT(p_frame),
      num_faces,
      processing_time, detection_time, facial_parts_time,
      p_hnd->fd_profile.total_time_in_ms[FD_FRAME_PROCESSING_TIME] / p_hnd->fd_profile.num_frames_processed,
      p_hnd->fd_profile.total_time_in_ms[FD_FRAME_SW_DT_TIME] / p_hnd->fd_profile.num_frames_processed,
      p_hnd->fd_profile.total_time_in_ms[FD_FRAME_SW_FP_TIME] / p_hnd->fd_profile.num_frames_processed);
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_sw_wrapper_get_fd_info.
 *
 * Description: Main sw wrapper function to get the face detection and other features results.
 *     This will not execute the detection and other features. faceproc_sw_wrapper_execute_fd must be called before
 *     calling this function.
 *
 * Input parameters:
 *   p_handle - faceproc sw wrapper handle.
 *   faceinfo - face information to be filled
 *   max_roi - max faces. array size of faceinfo[]
 *   p_num_faces - num of faces
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 **/
int faceproc_sw_wrapper_get_fd_info(void *p_handle, faceproc_info_t faceinfo[], int max_roi, int32_t *p_num_faces)
{
  if (!p_handle || !p_num_faces) {
    IDBG_ERROR("NULL pointer input : p_handle=%p, p_num_faces=%p", p_handle, p_num_faces);
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_sw_wrap_t *p_hnd     = (faceproc_sw_wrap_t *)p_handle;
  int32_t             num_faces = 0;
  int                 rc;
  int                 face_indx;
  DETECTION_INFO      detection_info[MAX_FACE_ROI];

  rc = fpsww_dt_execute(p_hnd, NULL, TRUE, &num_faces);
  if (rc != IMG_SUCCESS) {
    *p_num_faces = 0;
    IDBG_ERROR("Failed in DT execute");
    return rc;
  }

  num_faces = (num_faces > max_roi) ? max_roi : (num_faces > MAX_FACE_ROI) ? MAX_FACE_ROI : num_faces;

  *p_num_faces = num_faces;

  IDBG_HIGH("====================== no.of. faces %d======================", num_faces);
  IDBG_HIGH("Create Params : engine=%s",
    (p_hnd->create_params.engine == FACEPROC_ENGINE_HW) ? "FACEPROC_ENGINE_HW" :
    ((p_hnd->create_params.engine == FACEPROC_ENGINE_DSP) ? "FACEPROC_ENGINE_DSP" : "FACEPROC_ENGINE_SW"));

  IDBG_HIGH("Features : facial parts=%d, contour=%d, smile=%d, blink=%d, gaze=%d",
    p_hnd->config.enable_facial_parts, p_hnd->config.enable_contour,
    p_hnd->config.enable_smile, p_hnd->config.enable_blink, p_hnd->config.enable_gaze);

  if (num_faces == 0) {
    for (face_indx = 0; face_indx < MAX_FACE_ROI; face_indx++) {
      faceinfo[face_indx].blink_detected  = 0;
      faceinfo[face_indx].left_blink      = 0;
      faceinfo[face_indx].right_blink     = 0;
      faceinfo[face_indx].left_right_gaze = 0;
      faceinfo[face_indx].top_bottom_gaze = 0;
      return IMG_SUCCESS;
    }
  }

  if (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) {
    rc = fpsww_dsp_dt_get_result_info(p_hnd, detection_info, num_faces, FALSE);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Failed in DT Get result info");
      return rc;
    }
  } else {
    for (face_indx = 0; face_indx < num_faces; face_indx++) {
      rc = fpsww_dt_get_result_info(p_hnd, &detection_info[face_indx], face_indx, FALSE);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("Failed in DT Get result info");
        (*p_num_faces)--;
        return rc;
      }
    }
  }

  for (face_indx = 0; face_indx < num_faces; face_indx++) {
    IDBG_HIGH("library Results : Face[%d] : ID=%d, Confidence=%d, Center =(%d, %d) w=%d, h=%d, Angle=%d, Pose=%d",
      face_indx,
      detection_info[face_indx].nID, detection_info[face_indx].nConfidence,
      detection_info[face_indx].ptCenter.x, detection_info[face_indx].ptCenter.y,
      detection_info[face_indx].nWidth, detection_info[face_indx].nHeight,
      detection_info[face_indx].nAngle, detection_info[face_indx].nPose);

    if (p_hnd->config.filter_params.lock_faces == TRUE) {
      rc = fpsww_dt_lock_unlock_face(p_hnd, detection_info[face_indx].nID, TRUE);
      if (IMG_ERROR(rc)) {
        IDBG_WARN("Failed in lock face");
        // not a fatal, we can continue.
      }
    }

    // Fill the face info
    faceinfo[face_indx].unique_id = abs(detection_info[face_indx].nID);
    faceinfo[face_indx].tracking = (detection_info[face_indx].nID < 0) ? true : false;;
    rc = wrapper_fill_face_info_from_dtresult(p_hnd, &faceinfo[face_indx], &detection_info[face_indx]);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Failed in filling FacialParts info, rc=%d", rc);
      return rc;
    }

    IDBG_HIGH("FaceInfo Results : Face[%d] : ID=%d, Confidence=%d, Rect =[(%d,%d) (%d,%d)] FaceAngleRoll=%d, GazeAngle=%d",
      face_indx,
      faceinfo[face_indx].unique_id, faceinfo[face_indx].fd_confidence,
      faceinfo[face_indx].face_boundary.x, faceinfo[face_indx].face_boundary.y,
      faceinfo[face_indx].face_boundary.dx, faceinfo[face_indx].face_boundary.dy,
      faceinfo[face_indx].face_angle_roll, faceinfo[face_indx].gaze_angle);

    if (p_hnd->config.enable_facial_parts == TRUE) {
      face_part_detect pt_results;

      // Get results only
      rc = fpsww_pt_execute_from_dthandle(p_hnd, NULL, face_indx, TRUE, &pt_results);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("Failed in PT Get result rc=%d", rc);
        return rc;
      }

      memset(&faceinfo[face_indx].fp, 0x00, sizeof(face_part_detect));

      rc = wrapper_fill_fp_info_from_ptresult(p_hnd, &faceinfo[face_indx].fp, face_indx, &pt_results);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("Failed in filling FacialParts info, rc=%d", rc);
        return rc;
      }

      // Get Results(only) for Contour, Smile, Blink, Gaze based on feature config.
      rc = wrapper_execute_bsgc_from_pthandle(p_hnd, NULL, face_indx, TRUE, &faceinfo[face_indx]);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("Failed in getting BSGC results, info, rc=%d", rc);
        return rc;
      }
    }
  }  /* end of forloop */

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_sw_wrapper_get_config.
 *
 * Description: SW wrapper interface to get the current library configuration.
 *     Get the current configuration (tuning params) of sw wrapper, includes library's
 *     configuration (dt, ct, etc) and wrapper's params.
 *
 * Input parameters:
 *   p_handle - faceproc sw wrapper handle.
 *   p_config - pointer to config structure to be filled by this function
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 **/
int faceproc_sw_wrapper_get_config(void *p_handle, fpsww_config_t *p_config)
{
  if (!p_handle || !p_config) {
    IDBG_ERROR("Null pointer p_handle=%p, p_config=%p", p_handle, p_config);
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_sw_wrap_t *p_hnd = (faceproc_sw_wrap_t *)p_handle;

  *p_config = p_hnd->config;

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_sw_wrapper_set_config.
 *
 * Description: SW wrapper interface to set the new library configuration.
 *     Set the configuration (tuning params). Since, wrapper exposes all tuning parameters, it tries to set all
 *     the parameters that are coming with this call. If client wants to set only few params, it is advised to get
 *     the current configuration first using faceproc_sw_wrapper_get_config() and modify whatever params it
 *     wants and call faceproc_sw_wrapper_set_config().
 *
 * Input parameters:
 *   p_handle - faceproc sw wrapper handle.
 *   p_config - pointer to config structure to be set
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 **/
int faceproc_sw_wrapper_set_config(void *p_handle, fpsww_config_t *p_config)
{
  if (!p_handle || !p_config) {
    IDBG_ERROR("Null pointer p_handle=%p, p_config=%p", p_handle, p_config);
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_sw_wrap_t *p_hnd = (faceproc_sw_wrap_t *)p_handle;
  int                 rc;

  p_hnd->config.enable_facial_parts = p_config->enable_facial_parts;
  p_hnd->config.enable_contour      = p_config->enable_contour;
  p_hnd->config.enable_smile        = p_config->enable_smile;
  p_hnd->config.enable_gaze         = p_config->enable_gaze;
  p_hnd->config.enable_blink        = p_config->enable_blink;

  p_hnd->config.filter_params = p_config->filter_params;

  rc = fpsww_co_set_config(p_hnd, &p_config->co_params);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in setting CO config");
    return rc;
  }

  rc = fpsww_dt_set_config(p_hnd, &p_config->dt_params);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in setting DT config");
    return rc;
  }

  if (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) {
    rc = fpsww_dsp_set_config(p_hnd, &p_config->dsp_params);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in dsp set config");
      return rc;
    }
  }

  // Set facial parts related config if client has asked to create handles. If not, there is no point in setting the config.
  // Its not mandatory to set these only if enable_facial_parts, enable_contour, etc are enabled.
  // Allow clients to set config params anytime and to enable facial parts at a later time.
  if (p_hnd->create_params.create_face_parts == TRUE) {
    rc = fpsww_pt_set_config(p_hnd, &p_config->pt_params);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in setting PT config");
      return rc;
    }

    rc = fpsww_ct_set_config(p_hnd, &p_config->ct_params);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in setting CT config");
      return rc;
    }

    rc = fpsww_sm_set_config(p_hnd, &p_config->sm_params);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in setting SM config");
      return rc;
    }

    rc = fpsww_gb_set_config(p_hnd, &p_config->gb_params);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in setting GB config");
      return rc;
    }
  }

  // Set config if we already created face_recog handles on client's request
  if (p_hnd->create_params.create_face_recog == TRUE) {
    rc = fpsww_fr_set_config(p_hnd, &p_config->fr_params);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in setting FR config");
      return rc;
    }
  }

  if (IMG_LOG_LEVEL_MED) {
    IDBG_INFO("Setting config as per client request");
    wrapper_print_current_config(p_hnd);
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_sw_wrapper_create.
 *
 * Description: Creates sw wrapper handle.
 *     Create the features' handles, result handles that will be used while processing the frames for face detection,
 *     facial parts, bsgc. Note that, setting create_facial_parts to TRUE results in creating the handles for
 *     PT, CT, SM, GB features so that client can at any time enable/disable these features' execution.
 *
 * Input parameters:
 *   p_create_params - Create configuration for this handle
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 **/
void *faceproc_sw_wrapper_create(fpsww_create_params_t *p_create_params)
{
  faceproc_sw_wrap_t *p_hnd;
  int                 rc            = IMG_SUCCESS;
  uint8_t             no_of_handles = p_create_params->no_of_fp_handles;

  // Check if the library is loaded or not. Caller must call faceproc_sw_wrapper_load before calling this.
  if ((!g_faceproc_wrap_lib.p_lib_handle) || (lib_counter <= 0)) {
    IDBG_ERROR("SW wrapper not loaded yet");
    return NULL;
  }

  p_hnd = calloc(1, sizeof(*p_hnd));
  if (p_hnd == NULL) {
    IDBG_ERROR("Fail to allocate handler memory");
    return NULL;
  }

  memset(&p_hnd->fd_profile, 0x0, sizeof(img_profiling_t));
  if (p_hnd->debug_settings.profile_fd_level >= FD_PROFILE_LEVEL_ALL) {
    IMG_TIMER_START(p_hnd->fd_profile.start_time[FD_HANDLE_CREATE_TIME]);
  }

  p_hnd->p_lib         = &g_faceproc_wrap_lib;
  p_hnd->create_params = *p_create_params;

  IDBG_HIGH("Create Params : engine=%d, create_face_parts=%d, create_face_recog=%d, "
    "max_face_count=%d, detection_mode=%d, handles=%d, use_dsp_if_available=%d",
    p_create_params->engine, p_create_params->create_face_parts, p_create_params->create_face_recog,
    p_create_params->max_face_count, p_create_params->detection_mode, p_create_params->no_of_fp_handles,
    p_create_params->use_dsp_if_available);

  if (p_create_params->use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) {
    rc = fpsww_dsp_custom_init(p_hnd);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in dsp custom init");
      goto error_dsp_init;
    }
  }

  ATRACE_BEGIN_SNPRINTF(32, "FD_CREATE_HANDLES_%d_%d", p_create_params->engine, p_create_params->detection_mode);

  rc = fpsww_co_create_handles(p_hnd);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in creating CO handles");
    goto error_co;
  }

  rc = fpsww_dt_create_handles(p_hnd, (int32_t)p_create_params->detection_mode, p_create_params->max_face_count);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in creating DT handles");
    goto error_dt;
  }

  if (p_hnd->create_params.create_face_parts == TRUE) {
    rc = fpsww_pt_create_handles(p_hnd, no_of_handles);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in creating PT handles");
      goto error_pt;
    }

    rc = fpsww_ct_create_handles(p_hnd, no_of_handles);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in creating CT handles");
      goto error_ct;
    }

    rc = fpsww_sm_create_handles(p_hnd, no_of_handles);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in creating SM handles");
      goto error_sm;
    }

    rc = fpsww_gb_create_handles(p_hnd, no_of_handles);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in creating GB handles");
      goto error_gb;
    }
  }

  // Create FaceRecog handles if client has requested for.
  if (p_hnd->create_params.create_face_recog == TRUE) {
    rc = fpsww_fr_create_handles(p_hnd, no_of_handles);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Failed in creating co handles");
      goto error_fr;
    }
  }

  rc = wrapper_get_current_config(p_hnd, &p_hnd->config);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in getting default config");
    goto error_config;
  }

  if (p_hnd->debug_settings.profile_fd_level >= FD_PROFILE_LEVEL_ALL) {
    uint32_t create_time = IMG_TIMER_END(p_hnd->fd_profile.start_time[FD_HANDLE_CREATE_TIME],
      p_hnd->fd_profile.end_time[FD_HANDLE_CREATE_TIME], "FD CreateHandles", IMG_TIMER_MODE_MS);
    IDBG_INFO("FDPROFILE : SWLibraryCreateHandlesTime = %u", create_time);
  }

  if (IMG_LOG_LEVEL_MED) {
    IDBG_INFO("Default config for mode=%d", p_create_params->detection_mode);
    wrapper_print_current_config(p_hnd);
  }

  ATRACE_END();

  return p_hnd;

error_config:
error_fr:
  fpsww_fr_delete_handles(p_hnd);

error_gb:
  fpsww_gb_delete_handles(p_hnd, no_of_handles);

error_sm:
  fpsww_sm_delete_handles(p_hnd, no_of_handles);

error_ct:
  fpsww_ct_delete_handles(p_hnd, no_of_handles);

error_pt:
  fpsww_pt_delete_handles(p_hnd, no_of_handles);

error_dt:
  fpsww_dt_delete_handles(p_hnd);

error_co:
  fpsww_co_delete_handles(p_hnd);

error_dsp_init:

  free(p_hnd);
  return NULL;
}

/**
 * Function: faceproc_sw_wrapper_destroy.
 *
 * Description: Destroys sw wrapper handle.
 *     Destroys all handles that were created in faceproc_sw_wrapper_create()
 *
 * Input parameters:
 *   p_handle - faceproc sw wrapper handle
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 **/
int faceproc_sw_wrapper_destroy(void *p_handle)
{
  if (!p_handle) {
    IDBG_ERROR("NULL pointer input");
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_sw_wrap_t *p_hnd         = (faceproc_sw_wrap_t *)p_handle;
  int                 rc            = IMG_SUCCESS;
  uint8_t             no_of_handles = p_hnd->create_params.no_of_fp_handles;
  uint32_t            delta;
  bool                profile_fd =
    (p_hnd->debug_settings.profile_fd_level >= FD_PROFILE_LEVEL_ALL) ?
    TRUE : FALSE;

  if (profile_fd == TRUE) {
    delta = p_hnd->fd_profile.total_time_in_ms[FD_FRAME_PROCESSING_TIME];

    if (delta && p_hnd->fd_profile.num_frames_processed) {
      IDBG_INFO("FDPROFILE : TotalFrames=%u, TotalTimeTaken=%u, AvgSWPrcessingTimeOfSingleFrame=%f (FPS=%d)",
        p_hnd->fd_profile.num_frames_processed, delta,
        (float)delta/(float)p_hnd->fd_profile.num_frames_processed,
        (p_hnd->fd_profile.num_frames_processed * 1000) / delta);
    }

    IMG_TIMER_START(p_hnd->fd_profile.start_time[FD_HANDLE_DELETE_TIME]);
  }

  // Logs are printed inside upon error, no need to print here.
  // Even if some function fails, we want to continue deleting.
  rc |= fpsww_fr_delete_handles(p_hnd);
  rc |= fpsww_gb_delete_handles(p_hnd, no_of_handles);
  rc |= fpsww_sm_delete_handles(p_hnd, no_of_handles);
  rc |= fpsww_ct_delete_handles(p_hnd, no_of_handles);
  rc |= fpsww_pt_delete_handles(p_hnd, no_of_handles);
  rc |= fpsww_dt_delete_handles(p_hnd);
  rc |= fpsww_co_delete_handles(p_hnd);

  if (p_hnd->create_params.use_dsp_if_available && p_hnd->p_lib->p_dsp_lib_handle) {
    rc |= fpsww_dsp_custom_deinit(p_hnd);
  }

  if (profile_fd == TRUE) {
    delta = IMG_TIMER_END(p_hnd->fd_profile.start_time[FD_HANDLE_DELETE_TIME],
      p_hnd->fd_profile.end_time[FD_HANDLE_DELETE_TIME], "FD_DeleteHandles", IMG_TIMER_MODE_MS);
    IDBG_INFO("FDPROFILE : SWLibraryDeleteHandlesTime %u", delta);
  }

  free(p_hnd);

  return rc;
}

/**
 * Function: faceproc_sw_wrapper_load.
 *
 * Description: Load sw wrapper, i.e load the sw faceproc library and function pointers.
 *     Note that, function pointers for all features will be loaded by default since the load is executed only once,
 *     the next clients may require those features. Maintains ref_count on no.of active clients.
 *
 * Input parameters:
 *   need_dsp_lib - whether fd dsp lib is needed to be loaded
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 **/
int faceproc_sw_wrapper_load(bool need_dsp_lib)
{
  int rc = IMG_SUCCESS;

  if (g_faceproc_wrap_lib.p_lib_handle == NULL) {
    IDBG_INFO("Loading faceproc library");
    rc = wrapper_load_library(&g_faceproc_wrap_lib);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Fail to load faceproc sw library, rc=%d", rc);
      return rc;
    }
  }

  lib_counter++;
  IDBG_MED("faceproc sw library loaded, no.of instances %d", lib_counter);

  if (need_dsp_lib == true) {
    if (g_faceproc_wrap_lib.p_dsp_lib_handle == NULL) {
      rc = wrapper_dsp_load_library(&g_faceproc_wrap_lib);
      if (rc != IMG_SUCCESS) {
        IDBG_ERROR("Fail to load faceproc dsp library, rc=%d", rc);
      } else {
        img_dsp_dlopen("libmmcamera_imglib_faceproc_adspstub2.so", &g_faceproc_wrap_lib,
          wrapper_dsp_unload_library,
          wrapper_dsp_reload_library);
      }
    }

    if (rc == IMG_SUCCESS) {
      lib_dsp_counter++;
      IDBG_MED("faceproc dsp library loaded, no.of instances %d", lib_dsp_counter);
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_sw_wrapper_unload.
 *
 * Description: Load sw wrapper, i.e load the sw faceproc library and function pointers.
 *     Called by clients when they no longer use faceproc_sw_wrapper. When ref_count becomes 0,
 *     wrapper unloads faceproc sw library.
 *
 * Input parameters:
 *   unload_dsp_lib - whether fd dsp lib is needed to be unloaded
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 **/
int faceproc_sw_wrapper_unload(bool unload_dsp_lib)
{
  int rc = IMG_SUCCESS;

  if ((g_faceproc_wrap_lib.p_lib_handle == NULL) ||
    (lib_counter == 0)) {
    IDBG_ERROR("faceproc sw library is already unloaded. ptr=0x%x, lib_counter=%d",
      g_faceproc_wrap_lib.p_lib_handle, lib_counter);
    return IMG_ERR_INVALID_OPERATION;
  }

  lib_counter--;
  IDBG_MED("faceproc sw library instances %d", lib_counter);

  if (lib_counter == 0) {
    IDBG_INFO("Unloading faceproc library");
    rc = wrapper_unload_library(&g_faceproc_wrap_lib);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("Fail to unload faceproc sw library, rc=%d", rc);
      return rc;
    }
  }

  if (unload_dsp_lib == TRUE) {
    if ((g_faceproc_wrap_lib.p_dsp_lib_handle == NULL) ||
      (lib_dsp_counter == 0)) {
      IDBG_ERROR("faceproc dsp library is already unloaded. ptr=0x%x, lib_counter=%d",
        g_faceproc_wrap_lib.p_dsp_lib_handle, lib_dsp_counter);
      return IMG_ERR_INVALID_OPERATION;
    }

    lib_dsp_counter--;
    IDBG_MED("faceproc sw library instances %d", lib_dsp_counter);

    if (lib_dsp_counter == 0) {
      IDBG_INFO("Unloading faceproc dsp library");
      rc = wrapper_dsp_unload_library(&g_faceproc_wrap_lib, NULL);
      if (rc != IMG_SUCCESS) {
        IDBG_ERROR("Fail to unload faceproc dsp library, rc=%d", rc);
        return rc;
      }
      img_dsp_dlclose(&g_faceproc_wrap_lib);
    }
  }

  memset(&g_faceproc_wrap_lib, 0x0, sizeof(fpsww_lib_t));

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_sw_wrapper_set_debug.
 *
 * Description: Set the debug settings
 *
 * Input parameters:
 *   p_handle - faceproc sw wrapper handle.
 *   p_debug_settings - debug settings
 *
  * Return values: None
 **/
void faceproc_sw_wrapper_set_debug(void *p_handle, faceproc_debug_settings_t *p_debug_settings)
{
  faceproc_sw_wrap_t *p_hnd = (faceproc_sw_wrap_t *)p_handle;

  if (!p_hnd) {
    // not an error
    return;
  }

  p_hnd->debug_settings = *p_debug_settings;

  return;
}

