/**********************************************************************
*  Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
.chromatix_id = FD_HW_CHROMATIX_DETECT_VIDEO_QC,
/* Face detection enable */
.enable = 1,
.min_face_size = 32,
.max_face_size = 1000,
.max_num_face_to_detect = 10,
.angle_front = FD_ANGLE_ALL,
.angle_half_profile = FD_ANGLE_45_ALL,
.angle_full_profile = FD_ANGLE_45_ALL,
.enable_upfront = 1,
.fd_max_fps = 30,
.enable_smile_detection = 0,
.enable_blink_detection = 0,
.enable_gaze_detection = 0,
.threshold = 100,
.non_tracking_threshold = 100,
.facial_parts_threshold = 700,
.closed_eye_ratio_threshold = 600,
.initial_frame_no_skip_cnt = 15,
.ct_detection_mode = FD_CONTOUR_MODE_EYE,
.lock_faces = 1,
.speed = 2,
.input_pending_buf = 1,
.enable_facial_parts_assisted_face_filtering = 0,
.assist_below_threshold = 1000,
.assist_facial_discard_threshold = 400,
.assist_facial_weight_mouth = 0.25,
.assist_facial_weight_eyes = 0.25,
.assist_facial_weight_nose = 0.25,
.assist_facial_weight_face = 0.25,
.assist_facial_eyes_filter_type = FD_FILTER_TYPE_MAX,
.assist_facial_nose_filter_type = FD_FILTER_TYPE_MAX,
.assist_facial_min_face_threshold = 400,
.enable_contour_detection = 0,
.assist_sw_detect_threshold = 500,
.assist_sw_detect_face_size_perc = 25,
.assist_sw_detect_box_border_perc = 25,
.assist_sw_detect_hw_salvation_box_border_perc = 100,
.assist_sw_detect_face_spread_tol = 0.3,
.assist_sw_detect_hw_salvation_face_spread_tol = 0.9,
.assist_sw_detect_search_dens = FD_DENSITY_NORMAL,
.assist_sw_discard_frame_border = 0,
.assist_sw_discard_out_of_border = 0,
#ifdef FD_USE_V65
.assist_tr_good_face_threshold = 770,
.assist_tr_detect_threshold = 690,
.assist_tr_tracking_threshold = 500,
.assist_tr_strict_good_face_threshold = 900,
.assist_tr_strict_detect_threshold = 720,
.assist_tr_strict_tracking_threshold = 500,
#else
.assist_tr_good_face_threshold = 625,
.assist_tr_detect_threshold = 600,
.assist_tr_tracking_threshold = 500,
.assist_tr_strict_good_face_threshold = 720,
.assist_tr_strict_detect_threshold = 680,
.assist_tr_strict_tracking_threshold = 500,
#endif
.assist_tr_anglediff_for_strict_threshold = 55,
.assist_tr_facelinktol_movedist_ratio = 1.1,
.assist_tr_facelinktol_minsize_ratio = 0.666666,
.assist_tr_facelinktol_maxsize_ratio = 1.5,
.assist_tr_facelinktol_angle_diff = 60.0,
.enable_sw_assisted_face_filtering = 1,
.enable_frame_batch_mode = FACE_FRAME_BATCH_MODE_OFF,
.enable_boost = 0,
.frame_batch_size = 1,
.backlite_boost_factor = 1.0f,
.stats_filter_lock = 0,
.stats_filter_max_hold = 0,
.ui_filter_max_hold = 0,
.delay_count = 2,
#ifdef FD_USE_V65
.hold_count = 3,
#else
.hold_count = 5,
#endif

.sw_detect_enable = 0,
.sw_detect_skip_if_no_orientaiton_info = 1,
.sw_detect_no_face_skip = 4,
.sw_detect_new_face_skip = 2,
.sw_detect_allow_serial_execution = 0,
.sw_detect_optimize_config = 1,
.sw_detect_min_face_size = 40,
.sw_detect_max_face_size = 400,
.sw_detect_search_density = FD_DENSITY_NORMAL,
.sw_detect_threshold = 600,
.sw_detect_angle_front = FD_ANGLE_ALL,
.sw_detect_angle_half_profile = FD_ANGLE_15_ALL,
.sw_detect_angle_full_profile = FD_ANGLE_NONE,
.no_face_search_cycle = 7,
.new_face_search_cycle = 10,
.refresh_count = 0,
.sw_detect_enable_upfront = 1,
.upfront_angle_front_profile = 45, /* supported : 0, 15, 45, 75*/
.upfront_angle_half_profile = 45, /* supported : 0, 15, 45, 75*/
.upfront_angle_full_profile = 0, /* supported : 0, 15, 45, 75*/

/* Stabilization parameters */
.stab_enable = 1,
.stab_history = 10,
/* Confidence tuning params, used to filter false positives */
.stab_conf = {
  .enable = 0,
  .filter_type = FD_STAB_HYSTERESIS,
  .hyst = {
    .start_A = 520,
    .end_A = 560,
  },
},
/* Position stabilization tuning params */
.stab_pos = {
  .enable = 1,
  .mode = FD_STAB_WITHIN_THRESHOLD,
  .min_count_for_stable_state = 6,
  .stable_threshold = 3,
  .state_cnt = 4,
  .threshold = 20,//45,//15,
  .filter_type = FD_STAB_AVERAGE,
  .average = {
    .history_length = 5,
  },
},
/* Size stabilization tuning params */
.stab_size = {
  .enable = 1,
  .mode = FD_STAB_WITHIN_THRESHOLD,
  .min_count_for_stable_state = 6,
  .stable_threshold = 4,
  .state_cnt = 4,
  .threshold = 200,//250,
  .use_reference = 0,
  .filter_type = FD_STAB_AVERAGE,
  .average = {
    .history_length = 5,
  },
},
/* Mouth stabilization tuning params */
.stab_mouth = {
  .enable = 0,
  .mode = FD_STAB_CONTINUES_CLOSER_TO_REFERENCE,
  .state_cnt = 1,
  .threshold = 10,
  .use_reference = 1,
  .filter_type = FD_STAB_NO_FILTER,
},
/* Smile stabilization tuning params */
.stab_smile = {
  .enable = 0,
  .mode = FD_STAB_EQUAL,
  .state_cnt = 0,
  .threshold = 4,
  .use_reference = 0,
  .filter_type = FD_STAB_HYSTERESIS,
  .hyst = {
    .start_A = 25,
    .end_A = 35,
    .start_B = 55,
    .end_B = 70,
  },
},
/* region filter tuning params */
  .region_filter = {
  .enable = 0,
  .max_face_num = 10,
  .p_region = {0.5, 0.8, 1},
  .w_region = {100, 0.8, 0.5},
  .size_region = {40, 80, 120},
},
/* Lux based dynamic threshold */
.dynamic_lux_config = {
  .lux_enable = 0,
  .lux_levels = {400, 280, 0},
  .lux_offsets = {-50, 0, 0},
},
