/**********************************************************************
*  Copyright (c) 2013-2014, 2016, 2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

/* Face detection enable */
.enable = 1,
.min_face_adj_type = FD_FACE_ADJ_FLOATING,
.min_face_size = 90,
.min_face_size_ratio = 0.1,
.max_face_size = 1000,
.max_num_face_to_detect = 5,
.angle_front = FD_ANGLE_ALL,
.angle_half_profile = FD_ANGLE_15_ALL,
.angle_full_profile = FD_ANGLE_NONE,
.enable_upfront = 1,
.upfront_angle_front_profile = 45, /* supported : 0, 15, 45, 75*/
.upfront_angle_half_profile = 45, /* supported : 0, 15, 45, 75*/
.upfront_angle_full_profile = 15, /* supported : 0, 15, 45, 75*/
.detection_mode = FD_CHROMATIX_MODE_MOTION_PROGRESS,
.fd_max_fps = 15,
.enable_smile_detection = 1,
.enable_blink_detection = 1,
.enable_gaze_detection = 1,
.search_density_nontracking = FD_DENSITY_NORMAL,
.search_density_tracking = FD_DENSITY_NORMAL,
.direction = 0,
.input_pending_buf = 0,
#ifdef FD_USE_V65
.threshold = 770,
#else
.threshold = 600,
#endif
.face_retry_count = 3,
.head_retry_count = 3,
.lock_faces = 1,
.move_rate_threshold = 8,
.initial_frame_no_skip_cnt = 15,
.ct_detection_mode = FD_CONTOUR_MODE_EYE,
.delay_count = 0,
.hold_count = 1,
.no_face_search_cycle = 10,
.new_face_search_cycle = 10,
.search_cycle_dyn_increase = 5,
.refresh_count = 0,
.accuracy = FD_TRACKING_ACCURACY_HIGH,
.pos_steadiness_param = 0,
.size_steadiness_param = 0,
.roll_angle_extension = 1,
.yaw_angle_extension = 1,

// Used when FD DSP is enabled
.dsp_clock = 100,
.dsp_bus = 240,
.dsp_latency = 1000,
.dsp_absapi = 1,
.dsp_dcvs_clock = 4,
.dsp_dcvs_min_clock =4,
#ifdef _ENABLE_DCVS_
.dcvs_enable = 1,
#else
.dcvs_enable = 0,
#endif


/* Stabilization parameters */
.stab_enable = 1,
.stab_history = 3,
/* Position stabilization tuning params */
.stab_pos = {
  .enable = 1,
  .mode = FD_STAB_EQUAL,
  .state_cnt = 0,
  .threshold = 4,
  .use_reference = 1,
  .filter_type = FD_STAB_NO_FILTER,
},
/* Size stabilization tuning params */
.stab_size = {
  .enable = 1,
  .mode = FD_STAB_CONTINUES_SMALLER,
  .state_cnt = 3,
  .threshold = 250,
  .use_reference = 0,
  .filter_type = FD_STAB_TEMPORAL,
  .temp = {
    .num = 8,
    .denom = 6,
  },
},
/* Mouth stabilization tuning params */
.stab_mouth = {
  .enable = 1,
  .mode = FD_STAB_CONTINUES_CLOSER_TO_REFERENCE,
  .state_cnt = 1,
  .threshold = 4,
  .use_reference = 1,
  .filter_type = FD_STAB_NO_FILTER,
},
/* Smile stabilization tuning params */
.stab_smile = {
  .enable = 1,
  .mode = FD_STAB_EQUAL,
  .state_cnt = 3,
  .threshold = 0,
  .use_reference = 0,
  .filter_type = FD_STAB_HYSTERESIS,
  .hyst = {
    .start_A = 25,
    .end_A = 35,
    .start_B = 55,
    .end_B = 65,
  },
},
