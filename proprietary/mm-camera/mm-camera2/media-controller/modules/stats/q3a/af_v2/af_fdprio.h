/* af_fdprio.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef FDPRIO_H_
#define FDPRIO_H_

#include "af_value_monitor.h"
#include "stats_chromatix_wrapper.h"

/** fdprio_cmd_t
 *    Enumeration to inform the command type for
 *    fd priority processing.
 **/
typedef enum {
  FDPRIO_CMD_INIT,
  FDPRIO_CMD_PROC_COUNTERS,
  FDPRIO_CMD_PROC_FD_ROI,
  FDPRIO_CMD_DEINIT,
  FDPRIO_CMD_PROC_FD_MAX
} fdprio_cmd_t;

/** fdprio_scn_chg_t
 *    Enumeration to inform the scene change type
 *    for fd prio scene change detection
 **/
typedef enum {
  FDPRIO_SCN_CHG_SAD,
  FDPRIO_SCN_CHG_GYRO,
  FDPRIO_SCN_CHG_MAX
} fdprio_scn_chg_t;

/** fdprio_set_parm_t
 *    Enumeration to inform the set param type
 *    for fd prio
 **/
typedef enum {
  FDPRIO_SET_AEC_DATA,
  FDPRIO_SET_GYRO_DATA,
  FDPRIO_MAX
} fdprio_set_parm_t;

/** stability_data_t
 *    @face_id: Face roi identification number.
 *    @frame_count: preview frame counter
 **/
typedef struct {
  int32_t   face_id;
  uint32_t  frame_count;
} stability_data_t;

/** fdprio_face_info_t
 *  @is_valid: FD coordinates are valid
 *  @face_id: unique id for FD info
 *  @ctr_x: center x coordinate of Face
 *  @ctr_y: center y coordinate of Face
 *  @top_left_x: top left x coordinate of Face
 *  @top_left_y: top left y coordinate of Face
 *  @loc_stability_cnt: location stability counter
 *  @size_stability_cnt: size stability counter
 */
typedef struct {
  boolean is_valid;
  int32_t face_id;
  int32_t ctr_x;
  int32_t ctr_y;
  int32_t top_left_x;
  int32_t top_left_y;
  uint32_t loc_stability_cnt;
  uint32_t size_stability_cnt;
} fdprio_face_info_t;

/** fdprio_scene_monitor_t
 *  @SY: Luma[256]
 *  @is_valid: is SY coordinate valid
 */
typedef struct {
  int  SY[MAX_YUV_STATS_NUM];
  boolean is_valid;
} fdprio_SY_data;

/** fdprio_scene_monitor_t
 *  @p_vm_sad: Control structure for SAD
 *  @p_vm_gyro: Control structure for SAD
 *  @prev_SY: Stored SY from previous frame.
 */
typedef struct {
  /* Inputs*/
  af_value_monitor_internal_t *p_vm_sad;
  af_value_monitor_internal_t *p_vm_gyro;
  fdprio_SY_data  prev_SY;
} fdprio_scene_monitor_t;

/** fdprio_scn_chg_set_input_t
 *  @fdprio_scn_chg_t: Type of scene chg input param.
 *  @float: Generic data to be set to Value monitor.
 */
typedef struct {
  fdprio_scn_chg_t type;
  float data;
} fdprio_scn_chg_set_input_t;

/** fdprio_t
 *
 *  @noface_cnt: how many consequent frames no faces are detected
 *
 *  @faces_detects: how many faces were detected in the latest frame
 *
 *  @info_pos: where to write the next biggest face info
 *
 *  @biggest_face_info: history for the biggest face detected
 *
 *  @face_info: info for the detected faces in the latest frame
 *              Must not be NULL if the command is FDPRIO_CMD_PROC_FD_ROI
 *
 */
typedef struct {
  uint8_t camera_id;
  boolean fd_enabled;
  uint8_t noface_cnt;
  uint8_t faces_detected;
  cam_rect_t cur_confgd_roi;
  uint32_t current_frame_id;
  uint32_t last_processed_frame_id;
  uint32_t camif_width;
  uint32_t camif_height;
  boolean trigger_search;
  fdprio_face_info_t active_face;
  mct_face_info_t   *pface_info;
  q3a_thread_data_t *thread_data;
  af_algo_tune_parms_adapter_t tuning_info;
  int preview_fps;
  fdprio_scene_monitor_t scn_mtr;
} fdprio_t;

/** fdprio_aec_set_parm_t
 *  @SY_data: Luma Sum data structure
 *  @preview_fps: fps at which preview is running
 */
typedef struct {
  SY_data_t *SY_data;
  float preview_fps;
} fdprio_aec_set_parm_t;

/** fdprio_set_parm
 *  @type: Set param type for FDPRIO
 *  @fdprio_data: Internal structure of FD prio
 *  @u: Union of set param type
 */
typedef struct {
  fdprio_set_parm_t type;
  fdprio_t *fdprio_data;
  union{
    fdprio_aec_set_parm_t aec_info;
    float gyro_sqr;
  } u;
} fdprio_set_parm;

/* External API function definitions */
boolean fdprio_process(fdprio_t *fdprio_data, fdprio_cmd_t cmd);
boolean fdprio_set_param(fdprio_set_parm *parm);
#endif /* FDPRIO_H_ */
