/**
 * imx362_pdaf.h
 *
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
 .vendor_id = QTI,
 .stats_dt    = CSI_RAW10,
 .orientation= PDAF_ORIENTATION_DEFAULT,
 .sensor_native_pattern_info = {
   /* Res 0 */
   {
   /* Num of 2PD pixels */
   /* In this sensor, horizontal pixel count == x_output,
     vertical pixel count == y_output / 4 */
     .block_count_horizontal = 4032,
     .block_count_vertical = 756,
   },
   /* Res 1 */
   {
     .block_count_horizontal = 4032,
     .block_count_vertical = 504,
   },
   /* Res 2 */
   {
     .block_count_horizontal = 1920,
     .block_count_vertical = 270,
   },
   /* Res 3 */
   {
     .block_count_horizontal = 1920,
     .block_count_vertical = 270,
   },
   /* Res 4 */
   {
     .block_count_horizontal = 1920,
     .block_count_vertical = 270,
   },
   /* Res 5 */
   {
     .block_count_horizontal = 4032,
     .block_count_vertical = 756,
   },
   /* Res 6 */
   {
     .block_count_horizontal = 4032,
     .block_count_vertical = 504,
   },
 },
 .buffer_block_pattern_info = {
   /* Res 0 */
   {
     /* line width in bytes */
#if defined(MSM8952_SENSORS)
     .stride = 4032*10/8,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW10_PACKED,
#else
     /* 2 byte per pixel with PLAIN16 packing */
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
     .stride = 1920*10/8,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW10_PACKED,
#else
     .stride = 1920*2,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW16,
#endif
   },
   /* Res 3 */
   {
#if defined(MSM8952_SENSORS)
     .stride = 1920*10/8,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW10_PACKED,
#else
     .stride = 1920*2,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW16,
#endif
   },
   /* Res 4 */
   {
#if defined(MSM8952_SENSORS)
     .stride = 1920*10/8,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW10_PACKED,
#else
     .stride = 1920*2,
     .buffer_type = PDAF_BUFFER_FLAG_INTERLEAVE_CHANNEL,
     .buffer_data_type = PDAF_DATA_TYPE_RAW16,
#endif
   },
   /* Res 5 */
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
   /* Res 6 */
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
 },
 /* use estimate blk level instead of ref to chromatix */
 .black_level = 64,
 .window_configure = {
  .pdaf_sw_window_mode = FIXED_GRID_WINDOW,
  /* ignore the floating window configuration */
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
 .defocus_confidence_th = 250,
 .PD_pix_overflow_threshold = 450,
 .pixel_order_type = PDAF_PIXEL_ORDER_DEFAULT,
 .pdaf_reg_info = {
  .orientation_addr = ORIENTATION_ADDR,
  .hdr_en_addr = HDR_EN_ADDR,
  .hdr_binning_addr = HDR_BINNING_ADDR,
  .x_add_sta_addr = X_ADD_STA_ADDR,
  .y_add_sta_addr = Y_ADD_STA_ADDR,
  .x_add_end_addr = X_ADD_END_ADDR,
  .y_add_end_addr = Y_ADD_END_ADDR,
  .x_out_size_addr = X_OUT_SIZE_ADDR,
  .y_out_size_addr = Y_OUT_SIZE_ADDR,
  .x_evn_inc_addr = X_EVN_INC_ADDR,
  .x_odd_inc_addr = X_ODD_INC_ADDR,
  .y_evn_inc_addr = Y_EVN_INC_ADDR,
  .y_odd_inc_addr = Y_ODD_INC_ADDR,
  .scale_mode_addr = SCALE_MODE_ADDR,
  .scale_m_addr = SCALE_M_ADDR,
  .dig_crop_x_offset_addr = DIG_CROP_X_OFFSET_ADDR,
  .dig_crop_y_offset_addr = DIG_CROP_Y_OFFSET_ADDR,
  .dig_crop_image_width_addr = DIG_CROP_IMAGE_WIDTH_ADDR,
  .dig_crop_image_height_addr = DIG_CROP_IMAGE_HEIGHT_ADDR,
  .binning_mode_addr = BINNING_MODE_ADDR,
  .binning_type_addr = BINNING_TYPE_ADDR,
},
