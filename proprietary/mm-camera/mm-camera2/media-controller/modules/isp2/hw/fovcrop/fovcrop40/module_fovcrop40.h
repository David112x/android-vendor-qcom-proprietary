/* module_fovcrop40.h
 *
 * Copyright (c) 2012-2014, 2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MODULE_fovcrop40_H__
#define __MODULE_fovcrop40_H__

/* std headers */
#include "pthread.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_common.h"
#include "isp_defs.h"
#include "fovcrop_reg.h"
#include "chromatix.h"

/** isp_win_t:
 *
 *  @x: top left x coordinate
 *  @y: top left y coordinate
 *  @width: window width
 *  @height: window height
 *  @first_pixel: left
 *  @last_pixel: top
 *  @first_line: width
 *  @last_line: height
 **/
typedef struct {
  int           x;
  int           y;
  int           width;
  int           height;
  uint32_t      first_pixel;
  uint32_t      last_pixel;
  uint32_t      first_line;
  uint32_t      last_line;
} isp_win_t;

/** isp_crop_window_info_t:
 *
 *  @first_pixel: left
 *  @last_pixel: top
 *  @first_line: width
 *  @last_line: height
 **/
typedef struct {
  uint32_t     first_pixel;
  uint32_t     last_pixel;
  uint32_t     first_line;
  uint32_t     last_line;
}isp_pixel_line_info_t;

/** isp_hw_zoom_param_entry_t
 *
 *  @stream_id: stream id
 *  @dim: dimensions
 *  @crop_win: crop window
 *  @roi_map_info: region of interest info
 **/
typedef struct {
  uint32_t                stream_id;
  cam_dimension_t         dim;
  isp_crop_window_info_t  crop_win;
  isp_pixel_line_info_t   roi_map_info;
} isp_hw_zoom_param_entry_t;

/** isp_hw_zoom_param_t
 *
 *  @num: num of entries
 *  @entry: zoom crop, roi , stream id etc info params
 **/
typedef struct {
  int num;
  isp_hw_zoom_param_entry_t entry;
} isp_hw_zoom_param_t;

/** fovcrop40_entry_t:
 *
 *  @reg_cmd: reg cmd information
 *  @hw_update_pending: hw update pending or not flag
 *  @is_used: is this entry used or not
 *  @crop_window: crop window information
 **/
typedef struct {
  ISP_FOV_CropConfigCmdType      reg_cmd;
  uint8_t                        hw_update_pending;
  uint8_t                        is_used;
  isp_crop_window_info_t         crop_window;
  isp_crop_window_info_t         crop_window_calculated;
} fovcrop40_entry_t;

/** fovcrop40_t:
 *
 *  @state: current state of module
 *  @session_id: session id that is currently using this
 *             module
 *  @fov_entry: fov entry
 *  @fov_enable: fov module enable/disable flag
 *  @fov_trigger_enable: fov trigger enable/disable flag
 *  @hw_module_id: hw module id
 *  @update_enable_bit: flag to indicate module enable / disable
 *                    is pending
 **/
typedef struct {
  isp_state_t                    state;
  uint32_t                       session_id;
  uint32_t                       streaming_mode_mask;
  cam_streaming_mode_t           curr_streaming_mode;
  cam_streaming_mode_t           old_streaming_mode;
  pthread_mutex_t                mutex;

  uint8_t                        entry_idx;
  isp_crop_factors_t             crop_factor;
  modulesChromatix_t             chromatix_ptrs;
  ispif_out_info_t               ispif_out_info;
  isp_out_info_t                 isp_out_info;
  mct_list_t                    *l_stream_info;
  sensor_out_info_t              sensor_out_info;
  isp_hw_stream_info_t           hw_stream_info;

  boolean                        is_bayer_sensor;
  fovcrop40_entry_t              fov_entry;
  isp_pixel_window_info_t        scaler_output;
  uint8_t                        fov_enable;
  boolean                        is_cds_update;
  boolean                        cds_enabled;

  uint8_t                        trigger_enable;
  uint8_t                        skip_trigger;
  uint8_t                        hw_update_pending;
  uint32_t                       hw_module_id;
  boolean                        update_enable_bit;
  uint32_t                       session_based_identity;
  uint32_t                       identity[ISP_MAX_STREAMS];
  boolean                        crop_window_based;
  boolean                        skip_fov_moudule;
  uint32_t                       min_left_input;
} fovcrop40_t;

#endif
