/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
#ifndef __FACEPROC_LIB_WRAPPER_H__
#define __FACEPROC_LIB_WRAPPER_H__

#include "img_comp_priv.h"
#include "faceproc.h"
#include "faceproc_sw_wrapper_types.h"

// CO functions
int fpsww_co_load_fptrs(void *p_lib_ptr, fpsww_co_fptr_t *p_co_funcs);
int fpsww_co_create_handles(faceproc_sw_wrap_t *p_hnd);
int fpsww_co_delete_handles(faceproc_sw_wrap_t *p_hnd);
int fpsww_co_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_co_params_t *p_co_params);
int fpsww_co_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_co_params_t *p_co_params);
int fpsww_co_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_co_params_t *p_co_params);


// DT functions
int fpsww_dt_load_fptrs(void *p_lib_ptr, fpsww_dt_fptr_t *p_dt_funcs);
int fpsww_dt_create_handles(faceproc_sw_wrap_t *p_hnd, int32_t detection_mode, int32_t max_detection_count);
int fpsww_dt_delete_handles(faceproc_sw_wrap_t *p_hnd);
int fpsww_dt_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_dt_params_t *p_dt_params);
int fpsww_dt_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_dt_params_t *p_dt_new_params);
int fpsww_dt_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_dt_params_t *p_dt_params);
int fpsww_dt_execute(faceproc_sw_wrap_t *p_hnd, img_frame_t *p_frame, bool get_results_only, int32_t *p_num_faces);
int fpsww_dt_get_result_info(faceproc_sw_wrap_t *p_hnd, DETECTION_INFO *p_face_info, int32_t face_index, bool raw_result);
int fpsww_dt_lock_unlock_face(faceproc_sw_wrap_t *p_hnd, INT32 faceID, bool lock);
int fpsww_dt_reset_results(faceproc_sw_wrap_t *p_hnd);
int fpsww_dt_is_face_locked(faceproc_sw_wrap_t *p_hnd, int32_t face_index);

// PT functions
int fpsww_pt_load_fptrs(void *p_lib_ptr, fpsww_pt_fptr_t *p_pt_funcs);
int fpsww_pt_create_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles);
int fpsww_pt_delete_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles);
int fpsww_pt_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_pt_params_t *p_pt_params);
int fpsww_pt_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_pt_params_t *p_pt_new_params);
int fpsww_pt_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_pt_params_t *p_pt_params);
int fpsww_pt_execute_from_dthandle(faceproc_sw_wrap_t *p_hnd, img_frame_t *p_frame, int32_t face_index,
                                           bool get_results_only, face_part_detect *p_pt_results);
int fpsww_pt_execute_from_positionIP(faceproc_sw_wrap_t *p_hnd, img_frame_t *p_frame, int32_t face_index,
                                            POINT center, uint32_t size, int face_angle_roll, int32_t scale, int32_t pose,
                                            bool get_results_only, face_part_detect *p_pt_results);

// CT functions
int fpsww_ct_load_fptrs(void *p_lib_ptr, fpsww_ct_fptr_t *p_ct_funcs);
int fpsww_ct_create_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles);
int fpsww_ct_delete_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles);
int fpsww_ct_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_ct_params_t *p_ct_params);
int fpsww_ct_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_ct_params_t *p_ct_new_params);
int fpsww_ct_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_ct_params_t *p_ct_params);
int fpsww_ct_execute_from_pthandle(faceproc_sw_wrap_t *p_hnd, img_frame_t *p_frame, int32_t face_index,
                                          bool get_results_only, fd_contour_results *p_ct_results);

// SM functions
int fpsww_sm_load_fptrs(void *p_lib_ptr, fpsww_sm_fptr_t *p_sm_funcs);
int fpsww_sm_create_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles);
int fpsww_sm_delete_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles);
int fpsww_sm_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_sm_params_t *p_sm_params);
int fpsww_sm_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_sm_params_t *p_sm_params);
int fpsww_sm_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_sm_params_t *p_sm_params);
int fpsww_sm_execute_from_pthandle(faceproc_sw_wrap_t *p_hnd, img_frame_t *p_frame, int32_t face_index,
                                            bool get_results_only, fd_smile_detect *p_sm_results);

// GB functions
int fpsww_gb_load_fptrs(void *p_lib_ptr, fpsww_gb_fptr_t *p_gb_funcs);
int fpsww_gb_create_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles);
int fpsww_gb_delete_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles);
int fpsww_gb_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_gb_params_t *p_gb_params);
int fpsww_gb_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_gb_params_t *p_gb_params);
int fpsww_gb_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_gb_params_t *p_gb_params);
int fpsww_gb_execute_from_pthandle(faceproc_sw_wrap_t *p_hnd, img_frame_t *p_frame, int32_t face_index,
                                           bool get_results_only, fd_gb_result *p_gb_results);

// FR functions
int fpsww_fr_load_fptrs(void *p_lib_ptr, fpsww_fr_fptr_t *p_fr_funcs);
int fpsww_fr_create_handles(faceproc_sw_wrap_t *p_hnd, uint8_t no_of_handles);
int fpsww_fr_delete_handles(faceproc_sw_wrap_t *p_hnd);
int fpsww_fr_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_fr_params_t *p_fr_params);
int fpsww_fr_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_fr_params_t *p_fr_params);
int fpsww_fr_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_fr_params_t *p_fr_params);

// DSP functions
int fpsww_dsp_load_fptrs(void *p_lib_ptr, fpsww_dsp_fptr_t *p_dsp_funcs);
int fpsww_dsp_co_load_fptrs(void *p_lib_ptr, fpsww_co_fptr_t *p_co_funcs);
int fpsww_dsp_dt_load_fptrs(void *p_lib_ptr, fpsww_dt_fptr_t *p_dt_funcs);
int fpsww_dsp_custom_init(faceproc_sw_wrap_t *p_hnd);
int fpsww_dsp_custom_deinit(faceproc_sw_wrap_t *p_hnd);
int fpsww_dsp_get_config(faceproc_sw_wrap_t *p_hnd, fpsww_dsp_params_t *p_dsp_params);
int fpsww_dsp_set_config(faceproc_sw_wrap_t *p_hnd, fpsww_dsp_params_t *p_dsp_params);
int fpsww_dsp_print_config(faceproc_sw_wrap_t *p_hnd, fpsww_dsp_params_t *p_dsp_params);
int fpsww_dsp_dt_get_result_info(faceproc_sw_wrap_t *p_hnd, DETECTION_INFO *p_face_info, int32_t num_faces, bool raw_result);


#endif //__FACEPROC_LIB_WRAPPER_H__
