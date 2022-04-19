/* ov12a10_pdaf.h
 *
 *Copyright (c) 2017 Qualcomm Technologies, Inc.
 *All Rights Reserved.
 *Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/*mirror off and flip off*/
 .vendor_id = QTI,
 .orientation= PDAF_ORIENTATION_DEFAULT,
 .sensor_native_pattern_info =
 {
  /* Res 0 */
  {
   .block_pattern = {
    .pd_offset_horizontal = 0,
    .pd_offset_vertical = 0,
    .pix_count = 16,
    .block_dim =
    {
     .width= 32,
     .height = 32,
    },
    .pix_coords =
    {
     {6, 10, PDAF_LEFT_PIXEL,},
     {14, 6, PDAF_LEFT_PIXEL,},
     {22, 10, PDAF_LEFT_PIXEL,},
     {30, 6, PDAF_LEFT_PIXEL,},
     {6, 26, PDAF_LEFT_PIXEL,},
     {14, 22, PDAF_LEFT_PIXEL,},
     {22, 26, PDAF_LEFT_PIXEL,},
     {30, 22, PDAF_LEFT_PIXEL,},
     {6, 14, PDAF_RIGHT_PIXEL,},
     {14, 2, PDAF_RIGHT_PIXEL,},
     {22, 14, PDAF_RIGHT_PIXEL,},
     {30, 2, PDAF_RIGHT_PIXEL,},
     {6, 30, PDAF_RIGHT_PIXEL,},
     {14, 18, PDAF_RIGHT_PIXEL,},
     {22, 30, PDAF_RIGHT_PIXEL,},
     {30, 18, PDAF_RIGHT_PIXEL,}

    },
   },
   /* use estimate blk level instead of ref to chromatix */
   .block_count_horizontal = 128,
   .block_count_vertical = 96,
  },
  /* Res 1 */
  {
   .block_pattern = {
    .pd_offset_horizontal = 0,
    .pd_offset_vertical = 0,
    .pix_count = 16,
    .block_dim =
    {
     .width= 32,
     .height = 32,
    },
    .pix_coords =
    {
     {6, 10, PDAF_LEFT_PIXEL,},
     {14, 6, PDAF_LEFT_PIXEL,},
     {22, 10, PDAF_LEFT_PIXEL,},
     {30, 6, PDAF_LEFT_PIXEL,},
     {6, 26, PDAF_LEFT_PIXEL,},
     {14, 22, PDAF_LEFT_PIXEL,},
     {22, 26, PDAF_LEFT_PIXEL,},
     {30, 22, PDAF_LEFT_PIXEL,},
     {6, 14, PDAF_RIGHT_PIXEL,},
     {14, 2, PDAF_RIGHT_PIXEL,},
     {22, 14, PDAF_RIGHT_PIXEL,},
     {30, 2, PDAF_RIGHT_PIXEL,},
     {6, 30, PDAF_RIGHT_PIXEL,},
     {14, 18, PDAF_RIGHT_PIXEL,},
     {22, 30, PDAF_RIGHT_PIXEL,},
     {30, 18, PDAF_RIGHT_PIXEL,}

    },
   },
   /* use estimate blk level instead of ref to chromatix */
   .block_count_horizontal = 128,
   .block_count_vertical = 72,
  },
 },
 .buffer_block_pattern_info =
 {
  /* Res 0 */
  {
   .buffer_data_type = PDAF_DATA_TYPE_RAW10_LSB,
   .buffer_type = PDAF_BUFFER_FLAG_SPARSE,
  },
  /* Res 1 */
  {
   .buffer_data_type = PDAF_DATA_TYPE_RAW10_LSB,
   .buffer_type = PDAF_BUFFER_FLAG_SPARSE,
  },
 },
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
