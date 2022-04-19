/***************************************************************************
Copyright (c) 2015 Qualcomm Technologies, Inc. 
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
***************************************************************************/

#ifndef __FD_DSP_COMP_H__
#define __FD_DSP_COMP_H__

#include "img_comp_priv.h"
#include "faceproc.h"

#include "DetectorComDef.h"
#include "FaceProcAPI.h"
#include "FaceProcDtAPI.h"
#include "FaceProcTime.h"
#include <inttypes.h>
#include "fd_chromatix.h"

#include "adsp_fd.h"

/**
 * CONSTANTS and MACROS
 **/

#define FD_FACEPT_ENABLE(p) ((p)->fd_feature_mask & FACE_PROP_PARTS_ON)
#define FD_ANGLE_ENABLE(p) ((p)->fd_feature_mask & FACE_PROP_ANGLE_ON)

#define KB_SIZE 1024

typedef struct {
#ifndef FACEPROC_USE_NEW_WRAPPER
  /* facial_parts_hndl: Facial parts handle. */
  void *facial_parts_hndl;
#endif

  /* Flag indicate loading DSP for Dt library */
  INT32 load_dsp_lib;
  /* Flag indicating current dsp lib status. 1 - active; 0 - reverted to ARM */
  INT32 status_dsp_lib;

  /* Client ID received from img Thread Pool Mgr*/
  uint32_t client_id;

#ifdef FDDSP_USE_SW_WRAPPER_FOR_ALL
  uint8_t lib_loaded;
#else
  /* ptr_stub - Ptr to the adsp stub lib. Needed for uncoupling FD stub lib */
  void *ptr_stub;

  struct function_ptrs_t {
    /*FaceProcDt APIs*/
    INT32 (*FACEPROC_SetDtRefreshCount )(HDETECTION hDT, INT32 nMode,
      INT32 nRefreshCount);
    INT32 (*FACEPROC_DeleteDtResult )(HDTRESULT hDtResult);
    INT32 (*FACEPROC_DeleteDetection )(HDETECTION  hDT);
    INT32 (*FACEPROC_GetDtFaceCount )(HDTRESULT hDtResult,
      INT32 *pnCount);
    INT32 (*FACEPROC_GetDtFaceInfo )(HDTRESULT hDtResult,
      INT32 nIndex, FACEINFO *psFaceInfo);
    HDETECTION (*FACEPROC_CreateDetection )(VOID);
    INT32 (*FACEPROC_SetDtMode )(HDETECTION hDT, INT32 nMode);
    INT32 (*FACEPROC_SetDtStep )(HDETECTION hDT,
      INT32 nNonTrackingStep, INT32 nTrackingStep);
    INT32 (*FACEPROC_SetDtAngle )(HDETECTION hDT,
      UINT32 anNonTrackingAngle[POSE_TYPE_COUNT],
      UINT32 nTrackingAngleExtension);
    INT32 (*FACEPROC_SetDtDirectionMask )(HDETECTION hDT, BOOL bMask);
    INT32 (*FACEPROC_SetDtFaceSizeRange )(HDETECTION hDT, INT32 nMinSize,
      INT32 nMaxSize);
    INT32 (*FACEPROC_SetDtThreshold )(HDETECTION hDT,
      INT32 nNonTrackingThreshold,
      INT32 nTrackingThreshold);
    HDTRESULT (*FACEPROC_CreateDtResult )(INT32 nMaxFaceNumber,
      INT32 nMaxSwapNumber);
    INT32      (*FACEPROC_DetectionDSP )(HDETECTION hDT, RAWIMAGE *pImage,
      INT32 nLen,
      INT32 nWidth, INT32 nHeight,
      INT32 nAccuracy, HDTRESULT hDtResult);
    INT32  (*FACEPROC_Dt_VersionDSP )(UINT8 *minorVersion, UINT8 *majorVersion);
    INT32  (*FACEPROC_SetDSPPowerPref )(int clock, int bus, int latency,
      int b_absapi);
    INT32 (*FACEPROC_SetDtLostParam)(HDETECTION hDT, INT32 nFaceRetryCount,
      INT32 nHeadRetryCount, INT32 nHoldCount);
    INT32 (*FACEPROC_SetDtModifyMoveRate)(HDETECTION hDT,
      INT32 nModifyMoveRate);
    INT32 (*FACEPROC_DtLockID)(HDTRESULT hDtResult, INT32 nID);
  } fns;
#endif

  int restore_needed_flag;
} faceproc_dsp_lib_t;

/** faceproc_dsp_comp_t
 *   @b: base image component
 *   @abort_flag: Flag to indicate whether the abort is issued
 *   @width : Variable to hold the width of the last frame
 *   @height : Variable to hold the height of the last frame
 *   @frame_id : Variable to hold the frame index of the last
 *             frame
 *   @hdt: face detection handle
 *   @hresult: face detection result handle
 *   @hpt: Face parts detection handle
 *   @hptresult: face parts result handle
 *   @hct: countour handle
 *   @hctresult: contour result handle
 *   @hsm: smile detection handle
 *   @hsmresult: smile detection result handle
 *   @hfeature: face feature data handle
 *   @halbum: album data handle
 *   @hgb: gaze/blink estimation handle
 *   @hgbresult: gaze/blink result handle
 *   @bmem: backup memory
 *   @wmem: work memory
 *   @fdBmemorySet: flag to indicate if backup memory is set
 *   @fdWmemorySet: flag to indicate if work memory is set
 *   @recognized: flag to indicate if face is recognized
 *   @last_img_registered_idx: user id of the last registered
 *                           person
 *   @user_id: array of user ids
 *   @confidence: array of confidence degree of the users
 *   @p_lib: pointer to the faceproc library
 *   @config: faceproc configuration
 *   @config_set: flag to indicate if the config is set
 *   @mode: faceproc mode of execution
 *   @facedrop_cnt: face detection drop count
 *   @processing_buff_cnt: number of buffers which are not in in/out queues.
 *   @trans_info: translation info for the face cordinates
 *   @fd_chromatix: chromatix values for FD configuration
 *   @clip_face_data: Flag to indicate whether the face needs to
 *                  be clipped based on face size. For usecases
 *                  where face angle is greater than zero,
 *                  superset of the positions will be taken
 *   @client_id: client identity
 *   @is_chromatix_changed: flag to indicate if the chromatix is changed
 *   @need_config_update: flag to indicate if chromatix is updated
 *   @dynamic_cfg_params: chromatix values which need to be updated for FD
 *   @dump_mode: Dump data mode
 *   @intermediate_in_use_Q: queue to hold frame and result
 *     before facial parts processing.
 *   @intermediate_free_Q: queue to hold empty results and frame
 *     pointer.
 *   @inter_result; fd result after facial parts, copied to
 *     module on get param result event.
 *   @result_mutex: to control exclusive access to copy result
 *   @processing: Flag to indicate whether the FD DSP processing
 *              is in progress
 *   @debug_settings: FD specific debug settings
 *
 *   Faceproc component structure
 **/
typedef struct {
  /*base component*/
  img_component_t b;
  int abort_flag;

  /* Frame dimension */
  uint32_t width;
  uint32_t height;
  uint32_t frame_id;

  /* FaceprocEngine-specific fields */
  HDETECTION hdt;
  HDTRESULT hresult;

  /* pointer to the library */
  faceproc_dsp_lib_t *p_lib;

  faceproc_config_t config;
  int8_t config_set;
  faceproc_mode_t mode;
  int facedrop_cnt;
  int processing_buff_cnt;
  fd_chromatix_t fd_chromatix;
  int8_t clip_face_data;
  int client_id;
  BOOL is_chromatix_changed;
  BOOL need_config_update;
  faceproc_dynamic_cfg_params_t dynamic_cfg_params;
  faceproc_dump_mode_t dump_mode;

  img_queue_t intermediate_in_use_Q;
  img_queue_t intermediate_free_Q;
  faceproc_result_t inter_result;
  pthread_mutex_t result_mutex;
  BOOL processing;
#ifdef FACEPROC_USE_NEW_WRAPPER
  void *p_sw_wrapper;
#endif
  faceproc_debug_settings_t debug_settings;
  int32_t device_rotation;
} faceproc_dsp_comp_t;

/** faceproc_dsp_comp_struct
 *   @p_comp: pointer to the faceproc component
 *   @return_value: return value from job execution
 *   structure with component ptr for job function
 **/
typedef struct {
  faceproc_dsp_comp_t *p_comp;
  int32_t return_value;
} faceproc_dsp_comp_struct;

/** faceproc_dsp_comp_exec_struct
 *   @p_comp: pointer to the faceproc component
 *   @return_value: return value from job execution
 *   @p_frame: pointer to frame buffer
 *   structure with component ptr and frame ptr for job function
 **/
typedef struct {
  faceproc_dsp_comp_t *p_comp;
  img_frame_t *p_frame;
  int32_t return_value;
} faceproc_dsp_comp_exec_struct;

/** faceproc_buf_done_status_type
 *   @PROC_BUF_DONE_SUCCESS: BUF DONE called because of succes
 *   @PROC_BUF_DONE_ABORT: BUF DONE called because of abort
 *   @PROC_BUF_DONE_ERROR: BUF DONE called because of error
 *   why buf done was called
 **/
typedef enum {
  PROC_BUF_DONE_SUCCESS,
  PROC_BUF_DONE_ABORT,
  PROC_BUF_DONE_ERROR
} faceproc_buf_done_status_type;

/** faceproc_dsp_comp_eng_get_output_struct
 *   @p_comp: pointer to the faceproc component
 *   @return_value: return value from job execution
 *   @p_res: result of FD
 *   @buf_done_staus: buf done for success or abort
 *   job args for fd output job
 **/
typedef struct {
  faceproc_dsp_comp_t *p_comp;
  faceproc_result_t *p_res;
  int32_t return_value;
  faceproc_buf_done_status_type buf_done_status;
} faceproc_dsp_comp_eng_get_output_struct;


/** faceproc_dsp_comp_eng_load_struct
 *   @p_lib: pointer to the faceproc library
 *   @return_value: return value from job execution
 *   job arggs structure for comp load unload jobs
 **/
typedef struct {
  faceproc_dsp_lib_t * p_lib;
  int32_t return_value;
} faceproc_dsp_comp_eng_load_struct;

/** faceproc_internal_queue_struct
 *   @p_frame: pointer to frame buffer
 *   @inter_result: intermediate result before faceparts.
 *   structure with component ptr and frame ptr for job function
 **/
typedef struct {
  img_frame_t *p_frame;
  faceproc_result_t inter_result;
} faceproc_internal_queue_struct;

/* job execute functions*/

/**  these function  do not do any functionality except calling the
      actual functions from thread dedicated to DSP
 **/
/** faceproc_dsp_comp_eng_config_task_exec
 *   @param: The pointer to function/usecase specific params/struct
 *
 *   used as wrapper function for scheduling actual eng_config function as a
 *   job with img_thread_mgr on DSP dedicated thread.
 **/
int faceproc_dsp_comp_eng_config_task_exec(void *param);
/** faceproc_dsp_comp_eng_exec_task_exec
 *   @param: The pointer to function/usecase specific params/struct
 *
 *   used as wrapper function for scheduling actual eng_execute
 *   function as a job with img_thread_mgr on DSP dedicated
 *   thread.
 **/
int faceproc_dsp_comp_eng_exec_task_exec(void *param);
/** faceproc_dsp_comp_eng_get_output_task_exec
 *   @param: The pointer to function/usecase specific params/struct
 *
 *   used as wrapper function for scheduling  BUF_DONE
 *   evnent as a job with img_thread_mgr on ARM dedicated
 *   thread.
 **/
int faceproc_dsp_comp_eng_get_output_task_exec(void *param);
/** faceproc_dsp_comp_eng_destroy_task_exec
 *   @param: The pointer to function/usecase specific params/struct
 *
 *   used as wrapper function for scheduling actual eng_destroy
 *   function as a job with img_thread_mgr on DSP dedicated
 *   thread.
 **/
int faceproc_dsp_comp_eng_destroy_task_exec(void *param);
/** faceproc_dsp_comp_eng_load_task_exec
 *   @param: The pointer to function/usecase specific params/struct
 *
 *   used as wrapper function for scheduling actual eng_load
 *   function as a job with img_thread_mgr on DSP dedicated
 *   thread.
 **/
int faceproc_dsp_comp_eng_load_task_exec(void *param);
/** faceproc_dsp_comp_eng_unload_task_exec
 *   @param: The pointer to function/usecase specific params/struct
 *
 *   used as wrapper function for scheduling actual eng_unload
 *   function as a job with img_thread_mgr on DSP dedicated
 *   thread.
 **/
int faceproc_dsp_comp_eng_unload_task_exec(void *param);
/** faceproc_dsp_comp_eng_test_dsp_connection_task_exec
 *   @param: The pointer to function/usecase specific params/struct
 *
 *   used as wrapper function for scheduling actual
 *   eng_test_dsp_connection function as a job with
 *   img_thread_mgr on DSP dedicated thread.
 **/
int faceproc_dsp_comp_eng_test_dsp_connection_task_exec(void *param);


/* Prototypes of the faceproc engine functions */

/** faceproc_dsp_comp_eng_config
 *   @p_comp: pointer to the faceproc component
 *
 *   Configures the faceproc engine
 **/
int faceproc_dsp_comp_eng_config(faceproc_dsp_comp_t *p_comp);

/** faceproc_dsp_comp_eng_exec
 *   @p_comp: pointer to the faceproc component
 *   @p_frame: pointer to the frame
 *
 *   Main face proc execution function. This function needs to
 *   be called in the context of the component thread
 **/
int faceproc_dsp_comp_eng_exec(faceproc_dsp_comp_t *p_comp,
  img_frame_t *p_frame);

/** faceproc_dsp_comp_eng_get_output
 *   @p_comp: pointer to the faceproc component
 *   @p_frame: current frame
 *   @p_res: pointer to the faceproc result
 *
 *   This function returns the results of faceproc for the last
 *   frame.
 *
 *   Note that this function needs to be called after the
 *   execution is completed
 **/
int faceproc_dsp_comp_eng_get_output(faceproc_dsp_comp_t *p_comp,
  img_frame_t *p_frame, faceproc_result_t *p_res);

/** faceproc_dsp_comp_eng_exec
 *   @p_comp: pointer to the faceproc component
 *
 *   Destroys the faceproc engine
 **/
int faceproc_dsp_comp_eng_destroy(faceproc_dsp_comp_t *p_comp);

/** faceproc_dsp_comp_eng_reset
 *   @p_comp: pointer to the faceproc component
 *
 *   Resets the faceproc engine
 **/
int faceproc_dsp_comp_eng_reset(faceproc_dsp_comp_t *p_comp);

/** faceproc_dsp_comp_eng_unload
 *   @p_lib: pointer to the faceproc library
 *
 *   Unloads the faceproclibrary
 **/
void faceproc_dsp_comp_eng_unload(faceproc_dsp_lib_t *p_lib);

/** faceproc_dsp_comp_eng_load
 *   @p_lib: pointer to the faceproc library
 *
 *   Loads the faceproclibrary
 **/
int faceproc_dsp_comp_eng_load(faceproc_dsp_lib_t *p_lib);
/** faceproc_dsp_comp_eng_test_dsp_connection
 *   @p_lib: pointer to the faceproc library
 *
 *   Test of DSP responds to test FD-DSP function
 **/

int faceproc_dsp_comp_eng_test_dsp_connection(faceproc_dsp_lib_t *p_lib);

/**
 * Function: faceproc_dsp_comp_eng_update_cfg
 *
 * Description: Configure the faceproc engine min face size
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
int faceproc_dsp_comp_eng_update_cfg(faceproc_dsp_comp_t *p_comp);

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
  bool reset_results);

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
  img_frame_t *p_frame, INT32 * num_faces);

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
  INT32 *num_faces);

/**
 * Function: faceproc_dsp_error_to_img_error
 *
 * Description: Converts DSP error to Img error
 *
 * Input parameters:
 *   dsp_error - Error returned from DSP function
 *
 * Return values:
 *     IMG_xx error corresponds to DSP error
 *
 * Notes: none
 **/
int faceproc_dsp_error_to_img_error(int dsp_error);

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
  INT32 *num_faces);

#endif //__FD_DSP_COMP_H__
