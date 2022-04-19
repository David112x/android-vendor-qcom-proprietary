/**
 * s5k3m2xm_pdaf.h
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
/* uncomment PDAF2D_GAIN to enable 2d calibration pd parser */
//#define PDAF2D_GAIN
 .vendor_id = QTI,
 .orientation= PDAF_ORIENTATION_MIRROR_AND_FLIP,
 .sensor_native_pattern_info =
 {
  /* Res 0 */
  {
   .block_pattern = {
    .pd_offset_horizontal = 24,
    .pd_offset_vertical = 24,
    .pix_count = 32,
    .block_dim =
    {
     .width= 64,
     .height = 64,
    },
    .pix_coords =
    {
      {83, 80, PDAF_LEFT_PIXEL,},
      {83, 76, PDAF_RIGHT_PIXEL,},
      {67, 76, PDAF_LEFT_PIXEL,},
      {67, 72, PDAF_RIGHT_PIXEL,},
      {47, 76, PDAF_LEFT_PIXEL,},
      {47, 72, PDAF_RIGHT_PIXEL,},
      {31, 80, PDAF_LEFT_PIXEL,},
      {31, 76, PDAF_RIGHT_PIXEL,},
      {79, 60, PDAF_LEFT_PIXEL,},
      {79, 64, PDAF_RIGHT_PIXEL,},
      {63, 56, PDAF_LEFT_PIXEL,},
      {63, 60, PDAF_RIGHT_PIXEL,},
      {51, 56, PDAF_LEFT_PIXEL,},
      {51, 60, PDAF_RIGHT_PIXEL,},
      {35, 60, PDAF_LEFT_PIXEL,},
      {35, 64, PDAF_RIGHT_PIXEL,},
      {79, 44, PDAF_LEFT_PIXEL,},
      {79, 40, PDAF_RIGHT_PIXEL,},
      {63, 48, PDAF_LEFT_PIXEL,},
      {63, 44, PDAF_RIGHT_PIXEL,},
      {51, 48, PDAF_LEFT_PIXEL,},
      {51, 44, PDAF_RIGHT_PIXEL,},
      {35, 44, PDAF_LEFT_PIXEL,},
      {35, 40, PDAF_RIGHT_PIXEL,},
      {83, 24, PDAF_LEFT_PIXEL,},
      {83, 28, PDAF_RIGHT_PIXEL,},
      {67, 28, PDAF_LEFT_PIXEL,},
      {67, 32, PDAF_RIGHT_PIXEL,},
      {47, 28, PDAF_LEFT_PIXEL,},
      {47, 32, PDAF_RIGHT_PIXEL,},
      {31, 24, PDAF_LEFT_PIXEL,},
      {31, 28, PDAF_RIGHT_PIXEL,},
    },
   },
   /* use estimate blk level instead of ref to chromatix */
   .block_count_horizontal = 65,
   .block_count_vertical = 48,
  },
 },
 .buffer_block_pattern_info =
 {
  /* Res 0 */
  {
   .buffer_data_type = PDAF_DATA_TYPE_RAW10_LSB,
   .buffer_type = PDAF_BUFFER_FLAG_SPARSE,
  },
 },
 .black_level = 64,
#ifndef PDAF2D_GAIN
 .cali_version = CALIBRATION_VERSION_1D,
#else
 .cali_version = CALIBRATION_VERSION_2D,
#endif
 .window_configure = {
  .pdaf_sw_window_mode = FIXED_GRID_WINDOW,
  /* ingore the floating window configuration */
  .fixed_grid_window_configure =
  {
   .af_fix_window =
    {0.33333, 0.33334, 0.66668, 0.66668},
   .window_number_hori = 1,
   .window_number_ver  = 1,
  },
 },
 .defocus_confidence_th = 600,
