/**
 * imx519_pdaf.h
 *
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
 .vendor_id = QTI,
 .stats_dt    = 0x34,
 .orientation= PDAF_ORIENTATION_DEFAULT,
 .sensor_native_pattern_info =
 {
  /* res 0 */
  {
   .block_pattern = {
    .pd_offset_horizontal = 90,
    .pd_offset_vertical = 80,
    .pix_count = 8,
    .block_dim = {
      .width = 16,
      .height = 16,
    },
    .pix_coords =
    {
     {
      .x = 1 + 90,
      .y = 7 + 80,
      .flag = PDAF_LEFT_DIODE_PIX,
     },
     {
      .x = 9 + 90,
      .y = 7 + 80,
      .flag = PDAF_LEFT_DIODE_PIX,
     },
     {
      .x = 5 + 90,
      .y = 15 + 80,
      .flag = PDAF_LEFT_DIODE_PIX,
     },
     {
      .x = 13 + 90,
      .y = 15 + 80,
      .flag = PDAF_LEFT_DIODE_PIX,
     },
     {
      .x = 2 + 90,
      .y = 7 + 80,
      .flag = PDAF_RIGHT_DIODE_PIX,
     },
     {
      .x = 10 + 90,
      .y = 7 + 80,
      .flag = PDAF_RIGHT_DIODE_PIX,
     },
     {
      .x = 6 + 90,
      .y = 15 + 80,
      .flag = PDAF_RIGHT_DIODE_PIX,
     },
     {
      .x = 14 + 90,
      .y = 15 + 80,
      .flag = PDAF_RIGHT_DIODE_PIX,
     },
    },
   },
   .block_count_horizontal = 280,
   .block_count_vertical = 208,
  },
 },
 .buffer_block_pattern_info =
 {
  /* Res 0 */
  {
   .block_pattern = {
    .pix_count = 8,
     /* input pattern (every pd line)
     * total 416 lines, each line is: (pixel or 2 bytes)
     * 0   ~ 559:   left pd data
     * 560 ~ 1119:  right pd data
     */
    .pix_coords =
    {
     {
      .x = 0,
      .y = 0,
      .flag = PDAF_LEFT_DIODE_PIX,
     },
     {
      .x = 1,
      .y = 0,
      .flag = PDAF_LEFT_DIODE_PIX,
     },
     {
      .x = 0,
      .y = 1,
      .flag = PDAF_RIGHT_DIODE_PIX,
     },
     {
      .x = 1,
      .y = 1,
      .flag = PDAF_RIGHT_DIODE_PIX,
     },
     {
      .x = 0,
      .y = 2,
      .flag = PDAF_LEFT_DIODE_PIX,
     },
     {
      .x = 1,
      .y = 2,
      .flag = PDAF_LEFT_DIODE_PIX,
     },
     {
      .x = 0,
      .y = 3,
      .flag = PDAF_RIGHT_DIODE_PIX,
     },
     {
      .x = 1,
      .y = 3,
      .flag = PDAF_RIGHT_DIODE_PIX,
     },
    },
    .pd_offset_horizontal = 0,
    .pd_offset_vertical = 0,
    .block_dim = {
      .width = 2,
      .height = 4,
    },
   },
   .stride = 1120,
   .buffer_data_type = PDAF_DATA_TYPE_RAW16,
   .buffer_type = PDAF_BUFFER_FLAG_SEQUENTIAL_LINE,
  },
 },
 /* use estimate blk level instead of ref to chromatix */
 .black_level = 64,
 .cali_version = CALIBRATION_VERSION_2D,
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
