/* is.h
 *
 * Copyright (c) 2013-2017, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __IS_H__
#define __IS_H__
#include "stats_event.h"
#include "dis_interface.h"
#include "eis2_interface.h"
#include "eis3_interface.h"
#include "stats_debug.h"
#include "q3a_stats.h"
#include "stats_util.h"


#define IS_MAX_BUFFER_DELAY 32
#define IS_MAX_STREAMS 2
#define IS_PREVIEW 0
#define IS_VIDEO 1
#define IS_PREVIEW_USE_STATS_MASK 0x01
#define IS_VIDEO_USE_STATS_MASK 0x02


/** _is_info:
 *    @stream_on: Indicates whether stream is on.
 *    @is_type[]: IS method (DIS, gyro-assisted DIS, EIS 2.0, ...)
 *    @rs_cs_frame_id: latest frame id for which RS/CS stats was received
 *    @timestamp: RS/CS stats timestamp
 *    @rs_cs_data: RS/CS stats cache
 *    @gyro_frame_id: latest frame id for which gyro data was received
 *    @gyro_data: gyro data cache
 *    @dis_context: DIS context
 *    @eis2_context: EIS 2.0 context
 *    @eis3_handle: EIS 3.0 handle
 *    @eis3_input: input to EIS 3.0 processing
 *    @vfe_width: VFE width (image width + margin)
 *    @vfe_height: VFE height (image height + margin)
 *    @width[]: image width
 *    @height[]: image height
 *    @fps: frames per second
 *    @num_mesh_x: number of horizontal slices
 *    @num_mesh_y: number of vertical slices
 *    @num_row_sum: number of row sums
 *    @num_col_sum: number of column sums
 *    @margin: size of total margin as a percentage of image width/height
 *    @buffer_delay: number of frame's worth of gyro data before matrix is
 *      generated (applies only to EIS 3.0)
 *    @sensor_mount_angle: sensor mount angle (0, 90, 180, 270)
 *    @camera_position: camera position (front or back)
 *    @is_enabled[]: indicates whether IS is enabled
 *    @is_inited[]: indicates whether IS has been initialized
 *    @initial_sof_id: starting SOF id of video stream
 *    @sns_lib_offset_set: Indicates whether time offset of gyro sensor library
 *       has been set
 *    @dis_bias_correction: Indicates whether DIS bias correction is enabled
 *       (applies only to EIS 2)
 *    @use_stats: Indicates whether RS/CS stats is needed
 *    @run_is: Indicates that IS techonology has been selected for the stream,
 *      i.e., is_type is not IS_TYPE_NONE or IS_TYPE_CROP
 *    @flush_mode: IS module in flush mode when TRUE
 *    @is_chromatix_info: IS tuning parameters
 *    @sns_ref_cnt: reference count to sns_handle
 *    @sns_handle: sensors handle
 *
 * This structure represents the IS internal variables.
 **/
typedef struct _is_info {
  boolean stream_on[IS_MAX_STREAMS];
  cam_is_type_t is_type[IS_MAX_STREAMS];
  unsigned int rs_cs_frame_id;
  struct timeval timestamp;
  rs_cs_data_t rs_cs_data;
  unsigned int gyro_frame_id;
  mct_event_gyro_data_t gyro_data;
  dis_context_type dis_context;
  eis2_context_type eis2_context;
  void *eis3_handle;
  eis3_input_t eis3_input;
  unsigned int transform_type;
  long vfe_width[IS_MAX_STREAMS];
  long vfe_height[IS_MAX_STREAMS];
  long width[IS_MAX_STREAMS];
  long height[IS_MAX_STREAMS];
  uint32_t fps;
  uint32_t num_mesh_x;
  uint32_t num_mesh_y;
  unsigned int num_row_sum;
  unsigned int num_col_sum;
  float margin;
  uint32_t buffer_delay;
  unsigned int sensor_mount_angle;
  enum camb_position_t camera_position;
  unsigned int is_enabled[IS_MAX_STREAMS];
  unsigned int is_inited[IS_MAX_STREAMS];
  unsigned int initial_sof_id;
  unsigned int sns_lib_offset_set;
  uint32_t dis_bias_correction;
  unsigned int use_stats;
  boolean run_is[IS_MAX_STREAMS];
  boolean flush_mode;
  int sof_countdown;
  is_chromatix_info_t is_chromatix_info;
  int sns_ref_cnt;
  void *sns_handle;
} is_info_t;


typedef enum {
  IS_PROCESS_STREAM_EVENT = 1,
  IS_PROCESS_RS_CS_STATS,
  IS_PROCESS_GYRO_STATS,
  IS_PROCESS_FLUSH_MODE,
  IS_PROCESS_NO_GYRO_DATA
} is_process_parameter_type;


typedef enum {
  IS_VIDEO_STREAM_OFF = 1,
  IS_VIDEO_STREAM_ON,
  IS_PREVIEW_STREAM_OFF,
  IS_PREVIEW_STREAM_ON,
  IS_OTHER_STREAM_ON_OFF
} is_stream_event_t;


typedef struct _is_stream_event_data {
  is_stream_event_t stream_event;
  is_info_t *is_info;
} is_stream_event_data_t;


typedef struct _is_stats_data {
  unsigned int frame_id;
  unsigned int identity;
  is_info_t *is_info;
  stats_yuv_stats_t yuv_rs_cs_data;
  void*             ack_data;
} is_stats_data_t;


typedef struct _is_gyro_data {
  unsigned int frame_id;
  is_info_t *is_info;
  mct_event_gyro_data_t gyro_data;
} is_gyro_data_t;


typedef struct _is_flush_mode {
  is_info_t *is_info;
  boolean flush_mode;
} is_flush_mode_t;


typedef struct _is_no_gyro_data {
  unsigned int sof_id;
  is_info_t *is_info;
} is_no_gyro_data_t;


typedef struct _is_process_parameter {
  is_process_parameter_type type;

  union {
    is_stream_event_data_t stream_event_data;
    is_stats_data_t stats_data;
    is_gyro_data_t gyro_data;
    is_flush_mode_t flush_mode;
    is_no_gyro_data_t no_gyro_data;
  } u;
} is_process_parameter_t;


/** is_process_output_type
 **/
typedef enum {
  IS_PROCESS_OUTPUT_STREAM_EVENT = 1,
  IS_PROCESS_OUTPUT_RS_CS_STATS,
  IS_PROCESS_OUTPUT_GYRO_STATS,
  IS_PROCESS_OUTPUT_FLUSH_MODE,
  IS_PROCESS_OUTPUT_NO_GYRO_DATA
} is_process_output_type_t;


typedef struct _is_process_output {
  is_process_output_type_t type;
  is_stream_event_t is_stream_event;
  int num_output;
  is_output_type *is_output;
  is_output_type *is_output_flush;
} is_process_output_t;


/** is_set_output_type
 **/
typedef enum {
  IS_SET_OUTPUT_ROLE_SWITCH = 1
} is_set_output_type_t;


typedef struct _is_set_output {
  is_set_output_type_t type;
  boolean request_frame;
} is_set_output_t;


typedef boolean (*is_set_parameters_func)(is_set_parameter_t *param, is_info_t *is_info, is_set_output_t *output);

typedef boolean (*is_process_func)(is_process_parameter_t *param,
  is_process_output_t *output);

typedef void (*is_process_callback_func)(mct_port_t *port, is_process_output_t *output);

typedef void (*is_set_callback_func)(mct_port_t *port, is_set_output_t *output);

typedef void (*is_stats_done_callback_func)(void* port, void *stats);

boolean is_set_parameters(is_set_parameter_t *param, is_info_t *is_info, is_set_output_t *output);
boolean is_process(is_process_parameter_t *param, is_process_output_t *output);

#endif /* __IS_H__ */
