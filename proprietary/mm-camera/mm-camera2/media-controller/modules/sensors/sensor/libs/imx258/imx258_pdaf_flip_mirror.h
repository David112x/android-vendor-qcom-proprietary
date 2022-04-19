/**
 * imx258_pdaf.h
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
 .vendor_id = QTI,
 .stats_dt    = 0x2F,
 .orientation= PDAF_ORIENTATION_MIRROR_AND_FLIP,
 .sensor_native_pattern_info =
 {
  /* Res 0 */
  {
   .block_pattern = {
    .pd_offset_horizontal = 24,
    .pd_offset_vertical = 24,
    .pix_count = 8,
    .block_dim = {
      .width = 32,
      .height = 32,
    },
    .pix_coords =
    {
     {
      .x = 32 - 2 + 24,
      .y = 32 - 5 + 24,
      .flag = PDAF_LEFT_PIXEL,
     },
     {
      .x = 32 - 18 + 24,
      .y = 32 - 5 + 24,
      .flag = PDAF_LEFT_PIXEL,
     },
     {
      .x = 32 - 9 + 24,
      .y = 32 - 24 + 24,
      .flag = PDAF_LEFT_PIXEL,
     },
     {
      .x = 32 - 25 + 24,
      .y = 32 - 24 + 24,
      .flag = PDAF_LEFT_PIXEL,
     },
     {
      .x = 32 - 1 + 24,
      .y = 32 - 8 + 24,
      .flag = PDAF_RIGHT_PIXEL,
     },
     {
      .x = 32 - 17 + 24,
      .y = 32 - 8 + 24,
      .flag = PDAF_RIGHT_PIXEL,
     },
     {
      .x = 32 - 10 + 24,
      .y = 32 - 21 + 24,
      .flag = PDAF_RIGHT_PIXEL,
     },
     {
      .x = 32 - 26 + 24,
      .y = 32 - 21 + 24,
      .flag = PDAF_RIGHT_PIXEL,
     },
    },
   },
   .block_count_horizontal = 130,
   .block_count_vertical = 96,
  },
  /* res 1 */
  {
   .block_pattern = {
    .pd_offset_horizontal = 24,
    .pd_offset_vertical = 24,
    .pix_count = 8,
    .block_dim = {
      .width = 32,
      .height = 32,
    },
    .pix_coords =
    {
     {
      .x = 32 - 2 + 24,
      .y = 32 - 5 + 24,
      .flag = PDAF_LEFT_PIXEL,
     },
     {
      .x = 32 - 18 + 24,
      .y = 32 - 5 + 24,
      .flag = PDAF_LEFT_PIXEL,
     },
     {
      .x = 32 - 9 + 24,
      .y = 32 - 24 + 24,
      .flag = PDAF_LEFT_PIXEL,
     },
     {
      .x = 32 - 25 + 24,
      .y = 32 - 24 + 24,
      .flag = PDAF_LEFT_PIXEL,
     },
     {
      .x = 32 - 1 + 24,
      .y = 32 - 8 + 24,
      .flag = PDAF_RIGHT_PIXEL,
     },
     {
      .x = 32 - 17 + 24,
      .y = 32 - 8 + 24,
      .flag = PDAF_RIGHT_PIXEL,
     },
     {
      .x = 32 - 10 + 24,
      .y = 32 - 21 + 24,
      .flag = PDAF_RIGHT_PIXEL,
     },
     {
      .x = 32 - 26 + 24,
      .y = 32 - 21 + 24,
      .flag = PDAF_RIGHT_PIXEL,
     },
    },
   },
   .block_count_horizontal = 130,
   .block_count_vertical = 72,
  },
 },
 .buffer_block_pattern_info =
 {
  /* Res 0 */
  {
   .block_pattern = {
    .pix_count = 8,
     /* input pattern (every one PD block height)
     * for 96 lines, each line is: (pixel or 2 bytes)
     * 0   ~ 399:   zeros 80   ;pd left 80 ; pd left 80;
                    pd left 80             ; pd left 20 + zeros 60;
     * 400 ~ 799:   pd right 80;pd right 80; pd right 80;
                    pd right 20 + zeros 60; zeros 80;
     * 800 ~ 1199:  zeros 80   ;pd right 80; pd right 80;
                    pd right 80; pd right 20 + zeros 60;
     * 1200 ~ 1599: pd left 80 ;pd left 80 ; pd left 80;
                    pd left 20 + zeros 60 ; zeros 80;
     */
    .pix_coords =
    {
     {
      .x = 80,
      .y = 0,
      .flag = PDAF_LEFT_PIXEL,
     },
     {
      .x = 81,
      .y = 0,
      .flag = PDAF_LEFT_PIXEL,
     },
     {
      .x = 0,
      .y = 1,
      .flag = PDAF_RIGHT_PIXEL,
     },
     {
      .x = 1,
      .y = 1,
      .flag = PDAF_RIGHT_PIXEL,
     },
     {
      .x = 80,
      .y = 2,
      .flag = PDAF_RIGHT_PIXEL,
     },
     {
      .x = 81,
      .y = 2,
      .flag = PDAF_RIGHT_PIXEL,
     },
     {
      .x = 0,
      .y = 3,
      .flag = PDAF_LEFT_PIXEL,
     },
     {
      .x = 1,
      .y = 3,
      .flag = PDAF_LEFT_PIXEL,
     },
    },
    .pd_offset_horizontal = 0,
    .pd_offset_vertical = 0,
    .block_dim = {
      .width = 2,
      .height = 4,
    },
   },
   .stride = 800,
   .buffer_data_type = PDAF_DATA_TYPE_RAW16,
   .buffer_type = PDAF_BUFFER_FLAG_SEQUENTIAL_LINE,
  },
  /* res 1 */
  {
   .block_pattern = {
    .pix_count = 8,
     /* input pattern (every one PD block height)
     * for 96 lines, each line is: (pixel or 2 bytes)
     * 0   ~ 399:   zeros 80   ;pd left 80 ; pd left 80;
                    pd left 80             ; pd left 20 + zeros 60;
     * 400 ~ 799:   pd right 80;pd right 80; pd right 80;
                    pd right 20 + zeros 60; zeros 80;
     * 800 ~ 1199:  zeros 80   ;pd right 80; pd right 80;
                    pd right 80; pd right 20 + zeros 60;
     * 1200 ~ 1599: pd left 80 ;pd left 80 ; pd left 80;
                    pd left 20 + zeros 60 ; zeros 80;
     */
    .pix_coords =
    {
     {
      .x = 80,
      .y = 0,
      .flag = PDAF_LEFT_PIXEL,
     },
     {
      .x = 81,
      .y = 0,
      .flag = PDAF_LEFT_PIXEL,
  },
     {
      .x = 0,
      .y = 1,
      .flag = PDAF_RIGHT_PIXEL,
     },
     {
      .x = 1,
      .y = 1,
      .flag = PDAF_RIGHT_PIXEL,
     },
     {
      .x = 80,
      .y = 2,
      .flag = PDAF_RIGHT_PIXEL,
     },
     {
      .x = 81,
      .y = 2,
      .flag = PDAF_RIGHT_PIXEL,
 },
     {
      .x = 0,
      .y = 3,
      .flag = PDAF_LEFT_PIXEL,
     },
     {
      .x = 1,
      .y = 3,
      .flag = PDAF_LEFT_PIXEL,
     },
    },
    .pd_offset_horizontal = 0,
    .pd_offset_vertical = 0,
    .block_dim = {
       .width = 2,
       .height = 4,
    },
   },
   .stride = 800,
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
 .defocus_confidence_th = 600,
