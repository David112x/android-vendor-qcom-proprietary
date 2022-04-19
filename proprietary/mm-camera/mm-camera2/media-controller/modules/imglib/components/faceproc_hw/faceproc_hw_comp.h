/**********************************************************************
*  Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __FD_HW_COMP_H__
#define __FD_HW_COMP_H__

#include "img_comp_priv.h"
#include "faceproc.h"
#include "fd_chromatix.h"
#include <stdbool.h>
#include <media/msm_fd.h>

/* HW face detection tracking grid */
#define HWFD_TRACKING_GRID 50

#define HWFD_MAX_HIST 15
#define HWFD_LOST_FACE_HIST 1

#define HWFD_LOST_FACE_HOLD 4

/** SWFD_LOST_FACE_HOLD:
 *
 * Number of frames to hold when face is lost from SW tracker
 */
#define SWFD_LOST_FACE_HOLD 10

/* Face detection driver max number of faces which can be detected. */
#define FD_DRV_MAX_FACE_NUM 35

/** faceproc_hw_result_t:
  * @roi: last ROI
  * @hw_detect_count: total number of HW detections for the face
  * @hw_first_frame_id: first detected frameID from HW
  * @hw_last_frame_id: last detected frameID from HW
  * @hw_last_frame_delta: last frame detect delta from HW
  * @hw_total_conf: total adjusted confidence w.r.t HW result
  * @sw_detect_count: total number of SW detections for the face
  * @sw_trial_count: Number of SW trials for the result
  * @sw_first_frame_id: first detected frameID from SW
  * @sw_last_frame_id: last detected frameID from SW
  * @sw_last_frame_delta: last frame detect delta from SW
  * @sw_total_conf: total adjusted confidence w.r.t SW result
  * @final_detect_count: final detect count for the face
  * @final_first_frame_id: first detected frameID
  * @final_last_frame_id: last detected frameID
  * @final_last_frame_delta: last frame detect delta
  * @total_frame_delta: total frame delta;
  * @hw_detect_count: number of times face is detected by HW
  * @overall_total_conf: overall confidence ... if sw fails, add
  *                    rescaled hw conf (0 to sw thresh),
  *                    otherwise add just hw conf or sw conf
  * @overall_detect_status: keep last status.... temproal
  *                       code...  0: initial, 0: tracked, 1:
  *                       reported
  * @face_region: Face region for the grid based tracker
  * @prev_roi: previous ROI of the result
  * @hw_deviation: deviation based on HW result
  * @last_sw_roi: last accepted SW ROI
  * @face_center: center of the face
  *
  * Holds the face result of each unique ID
  */
typedef struct {
  faceproc_info_t roi;
  uint32_t hw_detect_count;
  uint32_t hw_first_frame_id;
  uint32_t hw_last_frame_id;
  uint32_t hw_last_frame_delta;
  uint32_t hw_total_conf;
  uint32_t sw_detect_count;
  uint32_t sw_trial_count;
  uint32_t sw_first_frame_id;
  uint32_t sw_last_frame_id;
  uint32_t sw_last_frame_delta;
  uint32_t sw_total_conf;
  uint32_t final_detect_count;
  uint32_t final_first_frame_id;
  uint32_t final_last_frame_id;
  uint32_t final_last_frame_delta;
  uint32_t overall_total_conf;
  uint32_t overall_detect_status;
  img_rect_t face_region;
  faceproc_info_t prev_roi;
  double hw_deviation;
  faceproc_info_t last_sw_roi;
  img_pixel_t face_center;
} faceproc_hw_result_t;

/** faceproc_hw_tracker_t:
  * @hw_result: list of HW results
  * @pr_face_id: Last prominent face ID
  * @pr_idx: Index of last prominent face ID
  * @largest_size: largest size
  *
  * Holds the face result of each unique ID
  */
typedef struct {
  faceproc_hw_result_t hw_result[HWFD_MAX_HIST];
  uint32_t pr_face_id;
  uint32_t pr_idx;
  uint32_t largest_size;
} faceproc_hw_tracker_t;


/** faceproc_hw_detect_tracking_t
 *   @enable: Enable face tracking.
 *   @profiles_angle: Allowed profile angle for new detected faces.
 *   @faces_tracked: Number of faces inside tracking grid.
 *   @faces_detected: Last scan detected faces.
 *   @clear_hist_cnt: Clear history count. History is cleared on 0 count.
 *   @id_cnt: Face id counter.
 *   @first_detect_threshold: Threshold of first detected face.
 *   @div_x: x axis grid divider.
 *   @div_y: y axis grid divider.
 *   @table: FD tracking table.
 *   @faces_cnt: face count for each set in batch. applicable
 *             only for batch mode
 *   @frame_id: current frame id
 *
 *   HW Faceproc Face detect lookup table.
 */
typedef struct {
  uint8_t enable;
  uint8_t profiles_angle;
  uint8_t faces_tracked;
  uint8_t faces_detected;
  uint8_t clear_hist_cnt;
  uint8_t id_cnt;
  uint32_t first_detect_threshold;
  uint32_t div_x;
  uint32_t div_y;
  uint8_t table[FRAME_BATCH_SIZE_MAX][HWFD_TRACKING_GRID][HWFD_TRACKING_GRID];
  uint32_t frame_id;
  uint32_t batch_faces_detected[FRAME_BATCH_SIZE_MAX][HWFD_MAX_HIST];
  uint32_t batch_index[FRAME_BATCH_SIZE_MAX];
  uint8_t old_table[HWFD_TRACKING_GRID][HWFD_TRACKING_GRID];
  uint8_t prev_table[HWFD_LOST_FACE_HIST][HWFD_TRACKING_GRID][HWFD_TRACKING_GRID];
  uint32_t cur_tbl_idx;
  uint32_t pt_idx;
} faceproc_hw_detect_tracking_t;

struct _faceproc_hw_comp_t;

/** faceproc_hw_face_holder_t
 *   @tracking_id: Face tracking id.
 *   @p_face_data: Pointer to driver result struct.
 *   @p_tracking_info: Pointer to tracking info struct.
 *
 *   HW Faceproc face holder is used for sorting
 *     the results.
 */
typedef struct {
  uint8_t tracking_id;
  void *p_face_data;
  faceproc_hw_detect_tracking_t *p_tracking_info;
  struct _faceproc_hw_comp_t *p_comp;
} faceproc_hw_face_holder_t;

/** faceproc_hw_msg_mode_t
 *   @FDHW_MSG_MODE_NON_BLOCK: Message will not block .
 *   @FDHW_MSG_MODE_BLOCK: Message will block until is processed
 *
 *   HW Faceproc working thread message mode.
 */
typedef enum {
  FDHW_MSG_MODE_NON_BLOCK,
  FDHW_MSG_MODE_BLOCK,
} faceproc_hw_msg_mode_t;

/** faceproc_hw_msg_type_t
 *   @FDHW_MSG_SET_MODE: Set new mode.
 *   @FDHW_MSG_CFG_UPDATE: Configuration need to be updated.
 *   @FDHW_MSG_CHROMATIX_UPDATE: Chromatix need to be updated.
 *   @FDHW_MSG_DEBUG_UPDATE: Debug info update.
 *   @FDHW_MSG_QUEUE_FRAME: Frame queue is received.
 *   @FDHW_MSG_START: Start HW Faceproc.
 *   @FDHW_MSG_STOP: Stop HW Faceproc.
 *   @FDHW_MSG_EXIT: Worker Thread exit command.
 *   @FDHW_MSG_FRAME_BATCH_MODE: info about batch processing.
 *   @FDHW_MSG_RECOVER: Recovery for HW Faceproc
 *
 *   HW Faceproc working thread message type.
 */
typedef enum {
  FDHW_MSG_SET_MODE,
  FDHW_MSG_CFG_UPDATE,
  FDHW_MSG_CHROMATIX_UPDATE,
  FDHW_MSG_DEBUG_UPDATE,
  FDHW_MSG_QUEUE_FRAME,
  FDHW_MSG_START,
  FDHW_MSG_STOP,
  FDHW_MSG_EXIT,
  FDHW_MSG_FRAME_BATCH_MODE,
  FDHW_MSG_RECOVER,
} faceproc_hw_msg_type_t;

/** faceproc_hw_thread_msg_t
 *   @type: HW Faceproc message type.
 *   @mode: HW Faceproc message mode.
 *   @mode: Union: Faceproc mode is valid on type FDHW_MSG_SET_MODE.
 *   @debug_mode: Union: Faceproc debug mode is valid on type FDHW_MSG_DEBUG_UPDATE.
 *   @frame: Union: Pointer to frame valid on type FDHW_MSG_QUEUE_FRAME".
 *
 *   HW Faceproc working thread message.
 */
typedef struct {
  faceproc_hw_msg_type_t type;
  faceproc_hw_msg_mode_t mode;
  union {
    faceproc_mode_t mode;
    faceproc_dump_mode_t debug_mode;
    img_frame_t *frame;
  } u;
} faceproc_hw_thread_msg_t;

/** faceproc_hw_sw_detection_t
 *   @p_comp: pointer to hw component
 *   @p_frame: Frame for sw detection
 *   @frame_id: frame id
 *   @skip_sw_detect: whether to skip sw detection for this frame
 *   @num_faces: sw detection result - num faces detected
 *   @roi: sw detection result - face info
 *
 *   structure with component, frame, face results information for SW job
 **/
typedef struct {
  void *p_comp;
  img_frame_t *p_frame;
  uint32_t frame_id;
  bool skip_sw_detect;
  int32_t num_faces;
  faceproc_info_t roi[MAX_FACE_ROI];
} faceproc_hw_sw_detection_t;

/** faceproc_hw_sw_detection_feedback_t
 *   @hw_raw_face_cnt: Total number of faces that HW has detected
 *   @final_faces_detected_by_hw: Number of faces that HW has detected
 *       in the final face count
 *   @final_faces_detected_by_sw_only: Number of faces that SW alone detected
 *       in the final face count
 *   @sw_result: face result information detected by sw in the last frame
 *   @tracker_result: final face result from tracker
 *
 *   information used for configuring sw detection
 **/
typedef struct {
  uint32_t hw_raw_face_cnt;
  uint8_t final_faces_detected_by_hw;
  uint8_t final_faces_detected_by_sw_only;
  faceproc_result_t sw_result;
  faceproc_result_t tracker_result;
} faceproc_hw_sw_detection_feedback_t;

/** faceproc_hw_comp_t
 *   @b: base image component.
 *   @cmd_pipefd: Command pipe for thread communication 0 - read, 1 - write.
 *   @ack_pipefd: Acknowledgment pipe for thread communication 0 - read, 1 write.
 *   @p_drv_name: Pointer to face detect device name.
 *   @fd_drv: Driver file descriptor.
 *   @client_id: Client identity.
 *   @config: Faceproc configuration.
 *   @valid_chromatix: Flag indicating valid chromatix.
 *   @fd_chromatix: chromatix values for FD configuration.
 *   @mode: Faceproc mode of execution.
 *   @last_result: Face detection last result, this is returned in get params.
 *   @result_mutex: Mutex protecting last result.
 *   @start_in_progress: Flag indicating FD start in progress.
 *   @abort_in_progress: Flag indicating FD abort in progress.
 *   @sync_threadstart_proccount_mutex: Mutex to syncronize FD start and QBUF.
 *   @sync_threadstart_proccount_cond: Condition to syncronize FD start and QBUF.
 *   @tracking_info: Face detection tracking table struct.
 *   @dump_mode: Face detection dump mode.
 *   @workbuf_handle: Work buffer handler.
 *   @p_sw_detection: sw wrapper handle for sw detection
 *   @p_sw_wrapper: sw wrapper handle for false positive detection,
 *     facial parts detection.
 *   @facial_parts_hndl: Facial parts handle.
 *   @skip_on_busy_cnt: Number of continuos frames skipped
 *   @fd_frame_counter: counter for frame numbers that are being process by FD HW comp
 *   @debug_settings: FD specific debug settings
 *   @fd_profile: FD profile data
 *   @device_rotation: current device orientation
 *   @sw_thread_id: thread for sw frame processing such as results, tracking,
 *       false positive filtering, facial parts execution.
 *   @sw_tracker_pending_jobs: counter indicating the number of frames
 *       that are yet to be scheduled for sw tracker processing.
 *   @sw_detection_pending_jobs: counter indicating the number of frames
 *       that are yet to be scheduled for sw detection.
 *   @sw_detect_input_q: input queue for sw detection
 *   @sw_detect_results_q: reults queue for sw detection results
 *   @sw_detection_feedback: information/feedback used
 *     for configuring sw detection
 *   @need_hw_tracker_config_update: flag to indicate if chromatix is
 *     updated for Hw tracker
 *   @dynamic_cfg_params: Holds dynamically configurable parameters
 *
 *   HW Faceproc component structure.
 */
typedef struct _faceproc_hw_comp_t {
  img_component_t b;
  int cmd_pipefd[2];
  int ack_pipefd[2];
  char *p_drv_name;
  int fd_drv;
  int client_id;
  faceproc_config_t config;
  fd_rect_t crop;
  int valid_chromatix;
  fd_chromatix_t fd_chromatix;
  faceproc_mode_t mode;
  faceproc_result_t last_result;

  faceproc_result_t intermediate_result[FRAME_BATCH_SIZE_MAX];
  uint32_t batch_result_count;
  uint32_t batch_frame_count;

  faceproc_batch_mode_info_t frame_batch_info;
  pthread_mutex_t result_mutex;
  bool start_in_progress;
  bool abort_in_progress;
  pthread_mutex_t sync_threadstart_proccount_mutex;
  pthread_cond_t sync_threadstart_proccount_cond;
  faceproc_hw_detect_tracking_t tracking_info;
  faceproc_dump_mode_t dump_mode;
  img_mem_handle_t workbuf_handle;
  void *p_sw_detection;
#ifdef FACEPROC_USE_NEW_WRAPPER
  void *p_sw_wrapper;
#else
  void *facial_parts_hndl;
#endif
  faceproc_hw_tracker_t face_tracker;
  void *p_hw_tracker_wrapper;

  bool use_tracker_v2;

  int skip_on_busy_cnt;
  uint32_t fd_frame_counter;
  faceproc_debug_settings_t debug_settings;
  img_profiling_t fd_profile;
  int32_t device_rotation;
  uint32_t sw_thread_id;
  uint32_t sw_tracker_pending_jobs;
  uint32_t sw_detection_pending_jobs;
  img_queue_t sw_detect_input_q;
  img_queue_t sw_detect_results_q;
  faceproc_hw_sw_detection_feedback_t sw_detection_feedback;
  bool need_sw_detection_config_update;
  bool need_hw_tracker_config_update;
  faceproc_dynamic_cfg_params_t dynamic_cfg_params;
} faceproc_hw_comp_t;

/** faceproc_hw_exec_sw_job_t
 *   @p_comp: pointer to the faceproc component
 *   @p_frame: pointer to frame
 *   @face_hold: current face hold information
 *   @face_data: current face results from HW
 *   @face_cnt: current face count from HW
 *   @return_value: return value
 *
 *   structure with component, frame, face results information for SW job
 **/
typedef struct {
  faceproc_hw_comp_t *p_comp;
  img_frame_t *p_frame;

  faceproc_hw_face_holder_t face_hold[FD_DRV_MAX_FACE_NUM];
  struct msm_fd_face_data face_data[FD_DRV_MAX_FACE_NUM];
  uint8_t face_cnt;

  int32_t return_value;
} faceproc_hw_exec_sw_job_t;

/**
 * Function: faceproc_hw_comp_is_state
 *
 * Description: Check if component is in given state.
 *
 * Arguments:
 *   @p_comp: Pointer to HW Faceproc component instance.
 *   @state: State need to be checked.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 **/
int faceproc_hw_comp_is_state(faceproc_hw_comp_t *p_comp, comp_state_t state);

/**
 * Function: faceproc_hw_comp_get_pose_angle
 *
 * Description: Convert hw pose angle value to SW gaze angle
 *
 * Arguments:
 *   @hw_pose: HW Pose angle value.
 *
 * Return values:
 *   SW gaze angle
 **/
int faceproc_hw_comp_get_pose_angle(uint32_t hw_pose);

/**
 * Function: faceproc_hw_comp_get_sw_detection_results
 *
 * Description: Get the sw detection results
 *
 * Input parameters:
 *   p_comp - faceproc hw component
 *   p_curr_frame - current frame requiring sw detection results
 *   p_roi - faceproc results handle
 *   max_faces - size of p_roi
 *   p_num_faces - pointer to return num of faces detected
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
int faceproc_hw_comp_get_sw_detection_results(
  faceproc_hw_comp_t *p_comp, img_frame_t *p_curr_frame,
  faceproc_info_t *p_roi, int32_t max_faces, int32_t *p_num_faces);

#endif //__FD_HW_COMP_H__
