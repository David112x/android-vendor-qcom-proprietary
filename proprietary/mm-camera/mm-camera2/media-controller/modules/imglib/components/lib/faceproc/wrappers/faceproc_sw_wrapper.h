/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __FACEPROC_SW_WRAPPER_H__
#define __FACEPROC_SW_WRAPPER_H__

#include "faceproc.h"
#include "faceproc_sw_wrapper_types.h"

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
int faceproc_sw_wrapper_load(bool need_dsp_lib);

/**
 * Function: faceproc_sw_wrapper_unload.
 *
 * Description: Load sw wrapper, i.e load the sw faceproc library and function pointers.
 *     Called by clients when they no longer use faceproc_sw_wrapper. When ref_count becomes 0,
 *     wrapper unloads faceproc sw library.
 *
 * Input parameters:
 *   need_dsp_lib - whether fd dsp lib is needed to be unloaded
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 **/
int faceproc_sw_wrapper_unload(bool need_dsp_lib);

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
void *faceproc_sw_wrapper_create(fpsww_create_params_t *p_create_params);

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
int faceproc_sw_wrapper_destroy(void *p_handle);

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
int faceproc_sw_wrapper_get_config(void *p_handle, fpsww_config_t *p_config);

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
int faceproc_sw_wrapper_set_config(void *p_handle, fpsww_config_t *p_config);

/**
 * Function: faceproc_sw_wrapper_set_default_config.
 *
 * Description: SW wrapper interface to reset the config to default
 *
 * Input parameters:
 *   p_handle - faceproc sw wrapper handle.
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_xxx on failure
 **/
int faceproc_sw_wrapper_set_default_config(void *p_handle);

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
int faceproc_sw_wrapper_reset_result(void *p_handle);

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
int faceproc_sw_wrapper_execute_fd(void *p_handle, img_frame_t *p_frame, int32_t *p_num_faces);

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
int faceproc_sw_wrapper_get_fd_info(void *p_handle, faceproc_info_t faceinfo[], int max_roi, int32_t *p_num_faces);

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
  int face_tracked, faceproc_info_t * p_old, int32_t face_index, bool run_facial_parts);

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
bool faceproc_sw_wrapper_is_face_tracked(void *p_handle, int index);

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
void faceproc_sw_wrapper_set_debug(void *p_handle, faceproc_debug_settings_t *p_debug_settings);


#endif //__FACEPROC_SW_WRAPPER_H__
