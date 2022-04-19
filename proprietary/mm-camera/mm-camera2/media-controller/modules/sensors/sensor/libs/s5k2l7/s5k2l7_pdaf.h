/**
 * s5k2l7_pdaf.h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
 .vendor_id = QTI,
 .stats_dt    = S5K2L7_CSI_PD_ISTATS,
 .orientation= PDAF_ORIENTATION_DEFAULT,
  .sensor_native_pattern_info = {
   /* Res 0 */
   {
     .block_pattern = {
       .pix_coords = {
         {
           /* LEFT pix comes first */
           .flag = PDAF_LEFT_PIXEL,
         },
       }
     },
     .block_count_horizontal = 4032,
     .block_count_vertical = 756,
   },
   /* Res 1 */
   {
     .block_pattern = {
       .pix_coords = {
         {
           .flag = PDAF_LEFT_PIXEL,
         },
       }
     },
     .block_count_horizontal = 4032,
     .block_count_vertical = 568,
   },
   /* Res 2 */
   {
     .block_pattern = {
       .pix_coords = {
         {
           .flag = PDAF_LEFT_PIXEL,
         },
       }
     },
     .block_count_horizontal = 2016,
     .block_count_vertical = 284,
   },
   /* Res 3 */
   {
     .block_pattern = {
       .pix_coords = {
         {
           .flag = PDAF_LEFT_PIXEL,
         },
       }
     },
     .block_count_horizontal = 2016,
     .block_count_vertical = 284,
   },
   /* Res 4 */
   {
     .block_pattern = {
       .pix_coords = {
         {
           .flag = PDAF_LEFT_PIXEL,
         },
       }
     },
     .block_count_horizontal = 2016,
     .block_count_vertical = 284,
   },
   /* Res 5 */
   {
     .block_pattern = {
       .pix_coords = {
         {
           .flag = PDAF_LEFT_PIXEL,
         },
       }
     },
     .block_count_horizontal = 1344,
     .block_count_vertical = 189,
   },
 },
 .buffer_block_pattern_info = {
   /* Res 0 */
   {
     /* line width in bytes */
     /* 2 byte per pixel with PLAIN16 packing */
#if defined(MSM8952_SENSORS)
     .stride = 4032*10/8,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW10_PACKED,//PDAF_DATA_TYPE_RAW16,
#else
     .stride = 4032*2,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW16,
#endif
   },
   /* Res 1 */
   {
#if defined(MSM8952_SENSORS)
     .stride = 4032*10/8,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW10_PACKED,
#else
     .stride = 4032*2,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW16,
#endif
   },
   /* Res 2 */
   {
#if defined(MSM8952_SENSORS)
     .stride = 2016*10/8,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW10_PACKED,
#else
     .stride = 2016*2,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW16,
#endif
   },
   /* Res 3 */
   {
#if defined(MSM8952_SENSORS)
     .stride = 2016*10/8,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW10_PACKED,
#else
     .stride = 2016*2,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW16,
#endif
   },
   /* Res 4 */
   {
#if defined(MSM8952_SENSORS)
     .stride = 2016*10/8,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW10_PACKED,
#else
     .stride = 2016*2,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW16,
#endif
   },
   /* Res 5 */
   {
#if defined(MSM8952_SENSORS)
     .stride = 1344*10/8,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW10_PACKED,
#else
     .stride = 1344*2,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW16,
#endif
   },
 },
 /* use estimate blk level instead of ref to chromatix */
 .black_level = 64,
 .window_configure = {
  .pdaf_sw_window_mode = FIXED_GRID_WINDOW,
  /* ingore the floating window configuration */
  .fixed_grid_window_configure =
  {
   .af_fix_window = {
    .pdaf_address_start_hori = 0.375,
    .pdaf_address_start_ver  = 0.375,
    .pdaf_address_end_hori   = 0.625,
    .pdaf_address_end_ver    = 0.625,
   },
   .window_number_hori = 1,
   .window_number_ver  = 1,
  },
 },
 .pd_offset_correction = 0.5,
 .defocus_confidence_th = 250,
 .PD_pix_overflow_threshold = 450,
 .pixel_order_type = PDAF_PIXEL_ORDER_DEFAULT,