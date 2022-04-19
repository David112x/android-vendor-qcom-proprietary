/***************************************************************************
* Copyright (c) 2014-2017, 2019 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
***************************************************************************/


#ifndef __FD_CHROMATIX_H__
#define __FD_CHROMATIX_H__

/**
 *   face detection modes
 **/
#define FD_CHROMATIX_MODE_MOTION_DEFAULT 1
#define FD_CHROMATIX_MODE_MOTION_FULL 1
#define FD_CHROMATIX_MODE_MOTION_PARTITION 2
#define FD_CHROMATIX_MODE_MOTION_PROGRESS 3

/**
 *   region definition for dymanic face detection based on distace from center
 *   Currently we impelent 3 regions to filter face
 **/
#define FD_CHROMATIX_MAX_REGION_NUM 3

/**
 *   region definition for dymanic face detection based on face angle
 *   compared to device orientation
 *   Currently we impelent 3 region to filter face
 **/
#define FD_CHROMATIX_MAX_ANGLE_REGION_NUM 3

/* Tracking Accuracy */
#define FD_TRACKING_ACCURACY_NORMAL   (15)
#define FD_TRACKING_ACCURACY_HIGH      (0)

/* Search Density */
#define FD_DENSITY_HIGHEST          (200)
#define FD_DENSITY_HIGH             (150)
#define FD_DENSITY_NORMAL           (100)
#define FD_DENSITY_LOW               (75)
#define FD_DENSITY_LOWEST            (50)

/**
 *   FD_LUX_LEVELS : Number of lux levels
 **/
#define FD_LUX_LEVELS            (3)

/** fd_chromatix_angle_t
 *   FD_FACE_ADJ_FIXED: Use fixed minface size
 *   FD_FACE_ADJ_FLOATING: apply the ratio for the face size
 *
 *   face angles for detection
 **/
typedef enum {
  FD_FACE_ADJ_FIXED,
  FD_FACE_ADJ_FLOATING,
} fd_face_adj_type_t;

/** fd_chromatix_angle_t
 *   FD_ANGLE_NONE: disable angle detection
 *   FD_ANGLE_15_ALL: 15 degree detection in all angles
 *   FD_ANGLE_45_ALL: 45 degree detection in all angles
 *   FD_ANGLE_135_ALL: 135 degree detection in all angles
 *   FD_ANGLE_ALL: detection in all angles
 *
 *   face angles for detection
 **/
typedef enum {
  FD_ANGLE_NONE,
  FD_ANGLE_15_ALL,
  FD_ANGLE_45_ALL,
  FD_ANGLE_135_ALL,
  FD_ANGLE_ALL,
  FD_ANGLE_MANUAL,
} fd_chromatix_angle_t;

/** fd_chromatix_ct_detection_mode_t
 *   FD_CONTOUR_MODE_DEFAULT: All counters are detected
 *   FD_CONTOUR_MODE_EYE: Only eyes are detected
 *
 *   Contour detection mode structure
 **/
typedef enum {
  FD_CONTOUR_MODE_DEFAULT = 0,
  FD_CONTOUR_MODE_EYE = 1,
} fd_chromatix_ct_detection_mode_t;

/** fd_face_stab_filter_t
 *   FD_STAB_NO_FILTER: Without stabilization filter.
 *   FD_STAB_TEMPORAL: Temporal filter.
 *   FD_STAB_HYSTERESIS: Hysteresis.
 *   FD_STAB_AVERAGE: Average filter.
 *   FD_STAB_MEDIAN: Median filter.
 *
 *   Face stabilization filter type.
 **/
typedef enum {
  FD_STAB_NO_FILTER,
  FD_STAB_TEMPORAL,
  FD_STAB_HYSTERESIS,
  FD_STAB_AVERAGE,
  FD_STAB_MEDIAN,
} fd_face_stab_filter_t;

/** fd_chromatix_angle_t
 *   FD_STAB_EQUAL: Values will be marked as stable when two consecutive
 *     values are equal.
 *   FD_STAB_BIGGER: Values will be marked as stable if new values are
 *     bigger then old ones.
 *   FD_STAB_SMALLER: Values will be marked as stable if new values are
 *     smaller then old ones:
 *   FD_STAB_CLOSER_TO_REFERENCE:  Values will be marked as stable when
 *     the distance to reference is smaller.
 *   FD_STAB_CONTINUES_EQUAL: The same as FD_STAB_EQUAL but it works in continues mode.
 *   FD_STAB_CONTINUES_BIGGER: The same as FD_STAB_BIGGER but it works in continues
 *     mode, if values are stable and new bigger values are received.
 *     they will be updated.
 *   FD_STAB_CONTINUES_SMALLER: The same as FD_STAB_SMALLER but it works in continues
 *     mode, if values are stable and new smaller values are received.
 *     they will be updated.
 *   FD_STAB_CONTINUES_CLOSER_TO_REFERENCE: The same as FD_STAB_BIGGER but it
 *     works in continues mode, if values are stable and new closer to reference
 *     values are received they will be updated.
 *
 *   Face stabilization mode
 **/
typedef enum {
  FD_STAB_EQUAL,
  FD_STAB_SMALLER,
  FD_STAB_BIGGER,
  FD_STAB_CLOSER_TO_REFERENCE,
  FD_STAB_CONTINUES_EQUAL,
  FD_STAB_CONTINUES_SMALLER,
  FD_STAB_CONTINUES_BIGGER,
  FD_STAB_CONTINUES_CLOSER_TO_REFERENCE,
  FD_STAB_WITHIN_THRESHOLD,
} fd_face_stab_mode_t;


/** fd_face_stab_params_t
 *   @enable: Enable stabilization.
 *   @mode: Stabilization mode.
 *   @min_count_for_stable_state: min state count needed to go to
 *     FD_STAB_STATE_STABLE state from FD_STAB_STATE_STABILIZE.
 *   @stable_threshold: Stabilization threshold to go into STABLE state.
 *     When in STABILIZE state, If the new values are within this threshold,
 *     we go into STABLE state. Applicable only if mode is
 *     FD_STAB_WITHIN_THRESHOLD
 *   @threshold: Stabilization threshold (Within threshold new values will not
 *     be accepted).
 *   @state_cnt: Number of consecutive frames to wait
 *     for entry to became stable.
 *   @use_reference: Stabilize entry by reference
 *       (Current reference are eyes).
 *   @filter_type: Filter type to be used for stabilization
 *   @movingThreshold: With threshold, object is not moved
 *   @movingInitStateCount: Init state count for moving face during stabilizing state
 *   @movingLinkFactor: Factor to determine moving face with previous 2 history
 *     @temp: Temporal filter:
 *       @num: Strength numerator.
 *       @denom: Strength denominator.
 *     @hyst: Hysteresis - Two rectangular zones available Zone A and Zone B.
 *       Requirement - zone A < zone B. Fields:
 *       @start_A: A Start point.
 *       @end_A: A end point.
 *       @start_B: B Start point.
 *       @end_B: B end point.
 *     @average: Average filter
 *       @history_length: History length of the filter.
 *       @moving_history_length: Moving histroy length
 *     @median: Median filter
 *       @history_length: History length of the filter.
 *
 *   Structure which holds face stabilization tuning parameters.
 **/
typedef struct {
  uint32_t enable;
  fd_face_stab_mode_t mode;
  uint32_t min_count_for_stable_state;
  uint32_t stable_threshold;
  uint32_t threshold;
  uint32_t state_cnt;
  uint32_t use_reference;
  fd_face_stab_filter_t filter_type;
  uint32_t   movingThreshold;
  uint32_t   movingInitStateCount;
  float      movingLinkFactor;
  union {
    struct {
      uint32_t num;
      uint32_t denom;
    } temp;
    struct {
      uint32_t start_A;
      uint32_t end_A;
      uint32_t start_B;
      uint32_t end_B;
    } hyst;
    struct {
      uint32_t history_length;
      uint32_t moving_history_length;
    } average;
    struct {
      uint32_t history_length;
    } median;
  };
} fd_face_stab_params_t;

/** fd_dynamic_lux_config_t
*   @lux_enable: Enable lux based dynamic configuration
*   @lux_levels: Different lux levels
*   @lux_offsets: offsets corresponding to lux levels
*
*   structure which holds the dynamic lux configuration params
**/
typedef struct {
  int8_t lux_enable;
  int32_t lux_levels[FD_LUX_LEVELS];
  int32_t lux_offsets[FD_LUX_LEVELS];
} fd_dynamic_lux_config_t;

/** fd_chromatix_region_filter_t
 *   @max_face_num: max face num to detect
 *   @p_region[N]: pecentage of region N
 *   @w_region[N]: weight of the region N
 *   @size_region[N]: minimun face size of region N
 *
 *   structure which holds the region filter tuning params
 **/
typedef struct {
  uint32_t enable;
  uint32_t max_face_num;
  float p_region[FD_CHROMATIX_MAX_REGION_NUM];
  float w_region[FD_CHROMATIX_MAX_REGION_NUM];
  uint32_t size_region[FD_CHROMATIX_MAX_REGION_NUM];
} fd_face_region_filter_t;

/** fd_face_angle_filter_t
*   @enable: Enable region filter based on angle.
*   @rip_angle_diff: rotation in plane angle diff w.r.t device orientation
*   @rip_confidence_offset: confidence offset for rotation in plane faces
*   @rop_angle_diff: rotation out of plane angle diff w.r.t device orientation
*   @rop_confidence_offset: confidence offset for rotation out of plane faces
*
*   structure which holds the region filter tuning params
**/
typedef struct {
  uint32_t enable;
  uint32_t max_face_num;
  int32_t rip_angle_diff[FD_CHROMATIX_MAX_ANGLE_REGION_NUM];
  int32_t rip_confidence_offset[FD_CHROMATIX_MAX_ANGLE_REGION_NUM];
  int32_t rop_angle_diff[FD_CHROMATIX_MAX_ANGLE_REGION_NUM];
  int32_t rop_confidence_offset[FD_CHROMATIX_MAX_ANGLE_REGION_NUM];
} fd_face_angle_filter_t;


/** fd_chromatix_filter_type_t
 *   FD_FILTER_TYPE_AVG: Average filter
 *   FD_FILTER_TYPE_MAX: Max filter
 *   FD_FILTER_TYPE_MIN: Min filter
 *   FD_FILTER_TYPE_MEDIAN: Median filter
 *   FD_FILTER_TYPE_TEMPORAL: Temporal filter
 *
 *   Filter type
 **/
typedef enum {
  FD_FILTER_TYPE_AVG = 0,
  FD_FILTER_TYPE_MAX = 1,
  FD_FILTER_TYPE_MIN = 2,
  FD_FILTER_TYPE_MEDIAN = 3,
  FD_FILTER_TYPE_TEMPORAL = 4,
} fd_chromatix_filter_type_t;

/** fd_chromatix_id_t
 *   FD_CHROMATIX_NOT_SET: ID not set in chromatix
 *   FD_CHROMATIX_DETECT_QC: fd_chromatix_detect_qc.h
 *   FD_CHROMATIX_DETECT_LITE_QC: fd_chromatix_detect_lite_qc.h
 *   FD_HW_CHROMATIX_DETECT_QC: fd_hw_chromatix_detect_qc.h
 *   FD_HW_CHROMATIX_DETECT_LITE_QC: fd_hw_chromatix_detect_lite_qc.h
 *   FD_CHROMATIX_DETECT_VIDEO_QC: fd_chromatix_detect_video_qc.h
 *   FD_CHROMATIX_DETECT_VIDEO_LITE_QC: fd_chromatix_detect_video_lite_qc.h
 *   FD_HW_CHROMATIX_DETECT_VIDEO_QC: fd_hw_chromatix_detect_video_qc.h
 *   FD_HW_CHROMATIX_DETECT_VIDEO_LITE_QC:
 *     fd_hw_chromatix_detect_video_lite_qc.h
 *   FD_CHROMATIX_DETECT_MULTI_CLIENT_QC:
 *     fd_chromatix_detect_multi_client_qc.h
 *   FD_CHROMATIX_DETECT_MULTI_CLIENT_LITE_QC:
 *     fd_chromatix_detect_multi_client_lite_qc.h
 *   FD_HW_CHROMATIX_DETECT_MULTI_CLIENT_QC:
 *     fd_hw_chromatix_detect_multi_client_qc.h
 *   FD_HW_CHROMATIX_DETECT_MULTI_CLIENT_LITE_QC:
 *     fd_hw_chromatix_detect_multi_client_lite_qc.h
 *   FD_CHROMATIX_BSGC: fd_chromatix_bsgc.h
 *   FD_HW_CHROMATIX_BSGC: fd_hw_chromatix_bsgc.h
 *   FD_CHROMATIX_RECOG_QC: fd_chromatix_recog_qc.h
 *   FD_CHROMATIX_DETECT_TURBO_QC: fd_chromatix_detect_turbo_qc.h
 *   FD_HW_CHROMATIX_DETECT_TURBO_QC: fd_hw_chromatix_detect_turbo_qc.h
 *   FD_CHROMATIX_BSGC_LITE: fd_chromatix_bsgc_lite.h
 *   FD_HW_CHROMATIX_BSGC_LITE: fd_hw_chromatix_bsgc_lite.h
 *
 *   FD Chromatix id
 **/
typedef enum {
  FD_CHROMATIX_NOT_SET = 0,
  FD_CHROMATIX_DETECT_QC = 1,
  FD_CHROMATIX_DETECT_LITE_QC = 2,
  FD_HW_CHROMATIX_DETECT_QC = 3,
  FD_HW_CHROMATIX_DETECT_LITE_QC = 4,
  FD_CHROMATIX_DETECT_VIDEO_QC = 5,
  FD_CHROMATIX_DETECT_VIDEO_LITE_QC = 6,
  FD_HW_CHROMATIX_DETECT_VIDEO_QC = 7,
  FD_HW_CHROMATIX_DETECT_VIDEO_LITE_QC = 8,
  FD_CHROMATIX_DETECT_MULTI_CLIENT_QC = 9,
  FD_CHROMATIX_DETECT_MULTI_CLIENT_LITE_QC = 10,
  FD_HW_CHROMATIX_DETECT_MULTI_CLIENT_QC = 11,
  FD_HW_CHROMATIX_DETECT_MULTI_CLIENT_LITE_QC = 12,
  FD_CHROMATIX_BSGC = 13,
  FD_HW_CHROMATIX_BSGC = 14,
  FD_CHROMATIX_RECOG_QC = 15,
  FD_CHROMATIX_DETECT_TURBO_QC = 16,
  FD_HW_CHROMATIX_DETECT_TURBO_QC = 17,
  FD_CHROMATIX_BSGC_LITE = 18,
  FD_HW_CHROMATIX_BSGC_LITE = 19,
} fd_chromatix_id_t;

/** fd_chromatix_t
 *   @chromatix_id: Chromatix id
 *   @enable: flag to enable face detection
 *   @min_face_adj_type: minimum face size adjustment type
 *   @min_face_size: minimum face size
 *   @min_face_size_ratio: ratio of minimum face size w.r.t
 *                       image size
 *   @max_face_size: maximum face size
 *   &max_num_face_to_detect: maximum number of faces to detect
 *   @angle_front: angles to detect from front pose
 *   @angle_half_profile: angles to detect from front pose
 *   @angle_full_profile: angles to detect from front pose
 *   @enable_upfront: whether to enable up-front angles only
 *   @upfront_angle_front_profile: angle diff for up-front  front profile
 *             region used when upfront is enbled
 *   @upfront_angle_half_profile: angle diff for up-front half profile
 *             region used when upfront is enbled
 *   @upfront_angle_full_profile: angle diff for up-front full profile
 *             region used when upfront is enbled
 *   @detection_mode: detection mode
 *   @fd_max_fps: Maximum fps at which FD has to run
 *   @enable_smile_detection: flag to enable smile detection
 *   @enable_blink_detection: flag to enable blink detection
 *   @enable_gaze_detection: flag to enable gaze detection
 *   @search_density_nontracking: search density for face proc
 *                  algorithm for non-tracking mode 20-40.
 *   @search_density_tracking: search density for face proc
 *                  algorithm for tracking mode 20-40.
 *   @direction: enable/disable face tracking direction
 *   @threshold: detection threshold
 *   @non_tracking_threshold: Threshold for non-tracking faces first detected
 *   @facial_parts_threshold: Facial parts threshold
 *   @face_retry_count: Face retry count (when face is lost during motion)
 *   @head_retry_count: Head retry count (when head is lost during motion)
 *   @lock_faces: lock Faces Enable
 *               (Faces are locked not allow other faces to replace them)
 *   @move_rate_threshold: Position Modify rate threshold
 *                (in percents) how smoothly face changes coordinates
 *   @initial_frame_no_skip_cnt: initial number of frame which
 *                             should not be skipped. This field
 *                             is needed depending on initial
 *                             detection latency. Note that the
 *                             field is valid only when
 *                             fd_max_fps > 0.
 *   @speed: Used only for HW FD. If set to 0 this parameter is ignored.
 *     Range: starting from 1 in incremental order (slower->faster).
 *   @input_pending_buf: Max number of allowed input pending buffers.
 *   @enable_facial_parts_assisted_face_filtering: flag to
 *                                               enable/disable
 *                                               facial parts
 *                                               assisted false
 *                                               positive
 *                                               filtering
 *   @enable_sw_assisted_face_filtering: flag to enable/disable
 *                                     SW assisted false
 *                                     positive filtering
 *   @assist_facial_weight_nose: weight for the nose confidence
 *                             to be considered for false
 *                             positive filtering
 *   @assist_facial_weight_face: weight for the face confidence
 *                             to be considered for false
 *                             positive filtering
 *   @assist_facial_eyes_filter_type: type of filter to be used
 *                                  for getting combined
 *                                  confidence for left/right
 *                                  eyes
 *   @assist_facial_nose_filter_type: type of filter to be used
 *                                  for getting combined
 *                                  confidence for left/right
 *                                  nose
 *   @assist_facial_min_face_threshold: minimum threshold to
 *                                    scale teh face confidence
 *                                    values
 *   @assist_sw_detect_threshold: Assisted sw face detection threshold
 *     0..1000
 *   @assist_sw_detect_face_size_perc: face size percentage to calculate
 *     min, max face sizes while calling sw false positive verification.
 *     in percents. 0..100.
 *   @assist_sw_detect_box_border_perc: Frame box rect percentage to
 *     calculate the crop region to search for while false positive filtering.
 *     Used when HW find the face.
 *    in percents. 0..100.
 *   @assist_sw_detect_hw_salvation_box_border_perc: Frame box rect percentage
 *     to calculate the crop region to search for while false positive
 *     filtering.
 *     Used when HW loses the face, search in bigger box
 *    in percents. 0..100.
 *   @assist_sw_detect_face_spread_tol: face spread tolerance. If the
 *     displacement is greater than the tolerance, face is rejected.
 *     Used when HW loses the face, allow bigger margin
 *   @assist_sw_detect_hw_salvation_face_spread_tol: face spread tolerance.
 *     If the displacement is greater than the tolerance, face is rejected.
 *     Used when HW finds the face
 *   @assist_sw_detect_search_dens: Assisted sw face detection
 *    search density one of .FD_DENSITY_HIGHEST, FD_DENSITY_HIGH,
 *    FD_DENSITY_NORMAL, FD_DENSITY_LOW, FD_DENSITY_LOWEST
 *   @assist_sw_discard_frame_border: Border of the frame used for sw face
 *     detection.
 *   @assist_sw_discard_out_of_border: Assisted sw face detection
 *     discard faces if they are out of frame boundary.
 *   @assist_tr_good_face_threshold: Confidence threshold to treat
 *     the face as a high confidence good face.
 *   @assist_tr_detect_threshold: Confidence threshold to
 *     accept the face for entry.
 *   @assist_tr_tracking_threshold: Confidence threshold to
 *     accept the face while tracking.
 *   @assist_tr_strict_good_face_threshold: Strict Confidence threshold to
 *     treat the face as a high confidence good face.
 *   @assist_tr_strict_detect_threshold: Strict Confidence threshold to
 *     accept the face for entry.
 *   @assist_tr_strict_tracking_threshold: Strict Confidence threshold to
 *     accept the face while tracking.
 *   @assist_tr_anglediff_for_strict_threshold: Angle difference to use
 *     the strict thresholds for a face.
 *   @assist_tr_facelinktol_movedist_ratio: Tolerance for moving distance;
 *     deny linkage when the distance is greater than the value
 *   @assist_tr_facelinktol_minsize_ratio: Minimum allowance of size ratio;
 *     deny linkage when the ratio of their sizes is smaller than the value
 *   @assist_tr_facelinktol_maxsize_ratio: Maximum allowance of size ratio;
 *     deny linkage when the ratio of their sizes is larger  than the value
 *   @assist_tr_facelinktol_angle_diff: Tolerance for angle variation;
 *     deny linkage when the angle variation is greater than the value
 *   @enable_contour_detection: flag to enable/disable contour
 *                            detection.
 *   @enable_low_light: Enable low light face detection.
 *   @low_light_start_lux_idx: Low light hysteresis region start.
 *   @low_light_end_lux_idx:  Low light hysteresis region end.
 *   @stab_enable: enable face stabilization
 *   @stab_history: Stabilization history depth (min is 2)
 *   @stab_conf: Face confidence stabilization parameters.
 *   @stab_pos: Face position stabilization parameters.
 *   @stab_size: Face size stabilization parameters.
 *   @stab_mouth: Mouth position stabilization parameters.
 *   @stab_smile: Smile degree stabilization parameters.
 *   @region_filter: filter the face in different region
 *   @angle_filter: filter the face based on rip, rop angles
 *   @dynamic_lux_config: lux based structure variable.
 *   @enable_frame_batch_mode: Flag to enable frame batch mode
 *   @frame_batch_size: Batch size
 *   @backlite_boost_factor: Boost factor for backlite filtering
 *   @stats_filter_max_hold: Max frames to hold for stats
 *                         temporal filter
 *   @stats_filter_lock: Lock the stats filter
 *   @delay_count: delay count to build up the
 *     confidence before showing the face
 *   @hold_count: Hold count (output previous face when face is lost)
 *   @no_face_search_cycle: search cycle count when no faces found
 *   @new_face_search_cycle: search cycle count to find new faces
 *     (once at least a face found and being tracked)
 *   @search_cycle_dyn_increase: search cycle offset to be used when device
       oreintation is not present.
 *   @refresh_count: interval count after which fresh face
 *                 search is done
 *   @accuracy: Accuracy while tracking
 *   @pos_steadiness_param: Position steadiness. ROI position will not be
 *       changed until the center has displaced by this percentage.
 *       Either this or stabilization(stab_pos) has to be enabled.
 *   @size_steadiness_param: Size steadiness. ROI size will not be changed
 *       until the size has changed by this percentage.
 *       Either this or stabilization(stab_size) has to be enabled.
 *   @roll_angle_extension: Whether to extend the search in additional left
 *       and right roll angles while tracking a face.
 *   @yaw_angle_extension: Whether to extend the search in additional left
 *       and right yaw angles while tracking a face.
 *
 *   @sw_detect_enable: Whether to enable intermittent sw detection.
 *   @sw_detect_skip_if_no_orientaiton_info: Whether to disable running
 *       "sw intermittent" when device orienation information is not
 *       available. When sw_detect_enable is 1,
 *       If value of this param is 1, skips running sw detection if
 *       device_orienation value is -1, starts running if device_orienation is
 *       some value other than -1.
 *       If value is 0, runs sw detection always, but angle configuration is
 *       as follows :
 *         If device_orienation is not available (-1) : configured as per
 *         sw_detect_angle_front, sw_detect_angle_half_profile,
 *         sw_detect_angle_full_profile
 *         If device_orientation is available : configured as per
 *         upfront_angle
 *   @sw_detect_no_face_skip: frame skip rate for sw detection when no faces.
 *   @sw_detect_new_face_skip: frame skip rate for sw detection when faces.
 *   @sw_detect_allow_serial_execution: allow serial execution of sw detection
 *       when parallel (HW, SW) processing not possible.
 *   @sw_detect_optimize_config : optimize sw detection config
 *       based on current results
 *   @sw_detect_min_face_size: min face size for sw detection.
 *   @sw_detect_max_face_size: max face size for sw detection.
 *   @sw_detect_search_density: search density for sw detection.
 *   @sw_detect_threshold: sw detection threshold.
 *   @sw_detect_angle_front: face angle config for front profile.
 *   @sw_detect_angle_half_profile: face angle config for half profile.
 *   @sw_detect_angle_full_profile: face angle config for full profile.
 *   @sw_detect_enable_upfront: whether to enable up-front angles only
 *
 *   @dsp_absapi: whether to use dsp abs api.
 *   @dsp_clock: dsp clock speed.
 *   @dsp_bus: dsp bus.
 *   @dsp_latency: dsp latency configuration.
 *   @dsp_dcvs_clock: dcvs dsp target corner clock Range 0 to 6
 *     TURBO = 0, NOMPLUS = 1, NOM = 2, SVSPLUS = 3,
 *     SVS=4,SVS2 = 5.
 *   @dsp_dcvs_min_clock: dcvs dsp minimum corner frequency
 *   @dcvs_enable: DCVS enable or disable[enabled from 8998 targets]
 *
 *   faceproc chromatix header
 **/
typedef struct {
  fd_chromatix_id_t chromatix_id;
  int8_t enable;
  fd_face_adj_type_t min_face_adj_type;
  uint32_t min_face_size;
  float min_face_size_ratio;
  uint32_t max_face_size;
  uint32_t max_num_face_to_detect;
  fd_chromatix_angle_t angle_front;
  fd_chromatix_angle_t angle_half_profile;
  fd_chromatix_angle_t angle_full_profile;
  bool enable_upfront;
  uint32_t upfront_angle_front_profile;
  uint32_t upfront_angle_half_profile;
  uint32_t upfront_angle_full_profile;
  uint32_t detection_mode;
  uint32_t fd_max_fps;
  uint32_t enable_smile_detection;
  uint32_t enable_blink_detection;
  uint32_t enable_gaze_detection;
  uint32_t search_density_nontracking;
  uint32_t search_density_tracking;
  uint32_t direction;
  uint32_t threshold;
  uint32_t non_tracking_threshold;
  uint32_t facial_parts_threshold;
  uint32_t closed_eye_ratio_threshold;
  uint32_t face_retry_count;
  uint32_t head_retry_count;
  uint32_t lock_faces;
  uint32_t move_rate_threshold;
  uint32_t initial_frame_no_skip_cnt;
  fd_chromatix_ct_detection_mode_t ct_detection_mode;
  uint32_t speed;
  uint32_t input_pending_buf;
  uint32_t enable_facial_parts_assisted_face_filtering;
  uint32_t enable_sw_assisted_face_filtering;
  uint32_t assist_below_threshold;
  uint32_t assist_facial_discard_threshold;
  float assist_facial_weight_mouth;
  float assist_facial_weight_eyes;
  float assist_facial_weight_nose;
  float assist_facial_weight_face;
  fd_chromatix_filter_type_t assist_facial_eyes_filter_type;
  fd_chromatix_filter_type_t assist_facial_nose_filter_type;
  uint32_t assist_facial_min_face_threshold;
  uint32_t assist_sw_detect_threshold;
  uint32_t assist_sw_detect_face_size_perc;
  uint32_t assist_sw_detect_box_border_perc;
  uint32_t assist_sw_detect_hw_salvation_box_border_perc;
  float assist_sw_detect_face_spread_tol;
  float assist_sw_detect_hw_salvation_face_spread_tol;
  uint32_t assist_sw_detect_search_dens;
  uint32_t assist_sw_discard_frame_border;
  uint32_t assist_sw_discard_out_of_border;
  uint32_t assist_tr_good_face_threshold;
  uint32_t assist_tr_detect_threshold;
  uint32_t assist_tr_tracking_threshold;
  uint32_t assist_tr_strict_good_face_threshold;
  uint32_t assist_tr_strict_detect_threshold;
  uint32_t assist_tr_strict_tracking_threshold;
  uint32_t assist_tr_anglediff_for_strict_threshold;
  float assist_tr_facelinktol_movedist_ratio;
  float assist_tr_facelinktol_minsize_ratio;
  float assist_tr_facelinktol_maxsize_ratio;
  float assist_tr_facelinktol_angle_diff;
  uint32_t enable_contour_detection;
  uint32_t enable_low_light;
  float low_light_start_lux_idx;
  float low_light_end_lux_idx;
  int8_t stab_enable;
  uint32_t stab_history;
  fd_face_stab_params_t stab_conf;
  fd_face_stab_params_t stab_pos;
  fd_face_stab_params_t stab_size;
  fd_face_stab_params_t stab_mouth;
  fd_face_stab_params_t stab_smile;
  fd_face_region_filter_t region_filter;
  fd_face_angle_filter_t angle_filter;
  fd_dynamic_lux_config_t dynamic_lux_config;
  uint32_t enable_frame_batch_mode;
  uint32_t frame_batch_size;
  uint32_t enable_boost;
  float backlite_boost_factor;
  uint32_t stats_filter_max_hold;
  uint32_t stats_filter_lock;
  uint32_t ui_filter_max_hold;

  uint32_t delay_count;
  uint32_t hold_count;
  uint32_t no_face_search_cycle;
  uint32_t new_face_search_cycle;
  uint32_t search_cycle_dyn_increase;
  uint32_t refresh_count;
  uint32_t accuracy;
  uint32_t pos_steadiness_param;
  uint32_t size_steadiness_param;
  bool roll_angle_extension;
  bool yaw_angle_extension;

  int8_t sw_detect_enable;
  int8_t sw_detect_skip_if_no_orientaiton_info;
  int8_t sw_detect_no_face_skip;
  int8_t sw_detect_new_face_skip;
  int8_t sw_detect_allow_serial_execution;
  int8_t sw_detect_optimize_config;
  uint32_t sw_detect_min_face_size;
  uint32_t sw_detect_max_face_size;
  uint32_t sw_detect_search_density;
  uint32_t sw_detect_threshold;
  fd_chromatix_angle_t sw_detect_angle_front;
  fd_chromatix_angle_t sw_detect_angle_half_profile;
  fd_chromatix_angle_t sw_detect_angle_full_profile;
  bool sw_detect_enable_upfront;

  int32_t dsp_absapi;
  int32_t dsp_clock;
  int32_t dsp_bus;
  int32_t dsp_latency;
  int32_t dsp_dcvs_clock;
  int32_t dsp_dcvs_min_clock;
  int32_t dcvs_enable;

} fd_chromatix_t;

#endif //__FD_CHROMATIX_H__
