/**
 * s5k3m2xm_pdaf.h
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
/* uncomment PDAF2D_GAIN to enable 2d calibration pd parser */
#define PDAF2D_GAIN
 .vendor_id = QTI,
 .orientation= PDAF_ORIENTATION_DEFAULT,
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
     {28, 31, PDAF_LEFT_PIXEL,},
     {28, 35, PDAF_RIGHT_PIXEL,},
     {44, 35, PDAF_LEFT_PIXEL,},
     {44, 39, PDAF_RIGHT_PIXEL,},
     {64, 35, PDAF_LEFT_PIXEL,},
     {64, 39, PDAF_RIGHT_PIXEL,},
     {80, 31, PDAF_LEFT_PIXEL,},
     {80, 35, PDAF_RIGHT_PIXEL,},
     {32, 51, PDAF_LEFT_PIXEL,},
     {32, 47, PDAF_RIGHT_PIXEL,},
     {48, 55, PDAF_LEFT_PIXEL,},
     {48, 51, PDAF_RIGHT_PIXEL,},
     {60, 55, PDAF_LEFT_PIXEL,},
     {60, 51, PDAF_RIGHT_PIXEL,},
     {76, 51, PDAF_LEFT_PIXEL,},
     {76, 47, PDAF_RIGHT_PIXEL,},
     {32, 67, PDAF_LEFT_PIXEL,},
     {32, 71, PDAF_RIGHT_PIXEL,},
     {48, 63, PDAF_LEFT_PIXEL,},
     {48, 67, PDAF_RIGHT_PIXEL,},
     {60, 63, PDAF_LEFT_PIXEL,},
     {60, 67, PDAF_RIGHT_PIXEL,},
     {76, 67, PDAF_LEFT_PIXEL,},
     {76, 71, PDAF_RIGHT_PIXEL,},
     {28, 87, PDAF_LEFT_PIXEL,},
     {28, 83, PDAF_RIGHT_PIXEL,},
     {44, 83, PDAF_LEFT_PIXEL,},
     {44, 79, PDAF_RIGHT_PIXEL,},
     {64, 83, PDAF_LEFT_PIXEL,},
     {64, 79, PDAF_RIGHT_PIXEL,},
     {80, 87, PDAF_LEFT_PIXEL,},
     {80, 83, PDAF_RIGHT_PIXEL,},
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
   .af_fix_window = {
    .pdaf_address_start_hori = 0.33333,
    .pdaf_address_start_ver  = 0.33334,
    .pdaf_address_end_hori   = 0.66668,
    .pdaf_address_end_ver    = 0.66668,
   },
   .window_number_hori = 1,
   .window_number_ver  = 1,
  },
 },
 .defocus_confidence_th = 200,
