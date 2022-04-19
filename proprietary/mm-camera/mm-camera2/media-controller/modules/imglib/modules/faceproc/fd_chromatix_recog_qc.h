/**********************************************************************
*  Copyright (c) 2013-2014, 2016, 2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
.chromatix_id = FD_CHROMATIX_RECOG_QC,
/* Face detection enable */
.enable = 1,
.min_face_adj_type = FD_FACE_ADJ_FIXED,
.min_face_size = 300,
.min_face_size_ratio = 0.1f,
.max_face_size = 1000,
.max_num_face_to_detect = 2,
.angle_front = FD_ANGLE_ALL,
.angle_half_profile = FD_ANGLE_NONE,
.angle_full_profile = FD_ANGLE_NONE,
.enable_upfront = 1,
.upfront_angle_front_profile = 45, /* supported : 0, 15, 45, 75*/
.upfront_angle_half_profile = 45, /* supported : 0, 15, 45, 75*/
.upfront_angle_full_profile = 15, /* supported : 0, 15, 45, 75*/
.detection_mode = FD_CHROMATIX_MODE_MOTION_PROGRESS,
.fd_max_fps = 7,
.enable_smile_detection = 0,
.enable_blink_detection = 0,
.enable_gaze_detection = 0,
.search_density_nontracking = FD_DENSITY_NORMAL,
.search_density_tracking = FD_DENSITY_NORMAL,
.direction = 0,
.input_pending_buf = 0,
#ifdef FD_USE_V65
.threshold = 770,
#else
.threshold = 600,
#endif
.facial_parts_threshold = 500,
.face_retry_count = 3,
.head_retry_count = 3,
.lock_faces = 1,
.move_rate_threshold = 8,
.initial_frame_no_skip_cnt = 15,
.ct_detection_mode = FD_CONTOUR_MODE_EYE,
.stab_enable = 0,
.enable_frame_batch_mode = FACE_FRAME_BATCH_MODE_OFF,
.frame_batch_size = 1,
.stats_filter_lock = 0,
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
