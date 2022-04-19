/* af_biz.h
*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef __BIZ_AF_H__
#define __BIZ_AF_H__

#include "af.h"
#include <dlfcn.h>

#define MOVE_LENS_PROFILE   0
#define AF_Q8       0x00000100
#define SEC_TO_MS   1000
#define MAX_AF_ALGO_OUTPUTS 1

#define GYRO_HISTORY_LENGTH 128

#define IS_CAF_ENABLED(mode)     \
  ((mode == AF_MODE_CAF) || (mode == AF_MODE_CAF_MACRO) ||\
  (mode == AF_MODE_CAF_NORMAL))

/* Create function pointers matching af_core.h function definitions */
typedef q3a_core_result_type (* af_init_core_func)
  (
    af_core_handle_type* handle
  );
typedef q3a_core_result_type (* af_get_parameters_core_func)
  (
      af_core_handle_type                 handle,
      const af_core_get_param_info_type*  get_param_info,
      af_core_get_param_output_type*      get_param_output
  );
typedef q3a_core_result_type (* af_set_parameters_core_func)
(
    af_core_handle_type                 handle,
    const af_core_set_param_info_type*  set_param_info,
    af_core_output_params_type*         output
);
typedef q3a_core_result_type (* af_process_core_func)
(
    af_core_handle_type              handle,
    const af_core_input_params_type *in,
    af_core_output_params_type       *out
);
typedef void (* af_deinit_core_func)
(
    af_core_handle_type handle
);
/* Function pointer to set log level */
typedef void (*af_set_log_core_func)(void);


/**
 * af_biz_algo_ops_t:Defines a structure with all the AF algo interface operations
 *
 * @set_parameters: Set operation
 * @get_parameters: Get operation
 * @process: Process stats operation
 * @init: Initialization
 * @deinit: De-init
 * @set_log: Set logs in algo
 **/
typedef struct {
  af_set_parameters_core_func    set_parameters;
  af_get_parameters_core_func    get_parameters;
  af_process_core_func           process;
  af_init_core_func              init;
  af_deinit_core_func            deinit;
  af_set_log_core_func           set_log;
} af_biz_algo_ops_t;

/**
 *
 * AF Biz Floating window internal sturcture
 *
 **/
typedef struct _af_biz_fw_int_param {
  uint32_t       roi_w;
  uint32_t       roi_h;
  uint32_t       total_row;
  uint32_t       total_col;
  int32_t        gap_w;
  int32_t        gap_h;
} af_biz_fw_int_param_t;

/**
 *
 * AF Biz logic internal sturcture
 *
 **/
typedef struct _af_biz_internal {
  af_biz_algo_ops_t                   af_algo_ops;
  af_core_handle_type                 af_core;
  af_core_input_params_type           af_input;
  af_core_output_params_type          af_core_out;
  af_output_data_t                    af_out;
  boolean                             af_inited;
  af_preview_size_t                   preview_size;
  boolean                             wait_for_aec_est;
  af_mode_type                        af_UI_mode;
  af_bestshot_data_t                  bestshot_d;
  af_stream_crop_t                    af_stream_crop;
  uint32_t                            sof_id;
  uint32_t                            af_ctrl_mode;
  int32_t                             hfr_mode;
  af_input_from_sensor_t              sensor_input;
  af_sw_filter_type                   sw_filter_type;
  boolean                             isp_reconfig_needed;
  boolean                             af_mode_changed;
  int                                 status;
  af_config_t                         stats_config;
  af_mode_info_t                      af_mode;
  uint32_t                            metering_mode;
  uint32_t                            frame_id;
  /* Note: To be removed once it moves to tuning
     header in next chromatix revision. */
  af_chromatix_search_limit_wrapper_t search_limit[AF_FOCUS_MODE_MAX];
  boolean                             enableTuningDebug;
  boolean                             lens_reset_on_init;
  boolean                             move_lens_to_def_pos;
  depth_service_output_t              depth_output;
  boolean                             is_fovc_enabled;
  float                               fovc_factor;
  boolean                             is_af_hj_enabled;
  boolean                             is_fine_scan_skipped;
  uint32                              stats_depth;
  uint32_t                            gyro_history_idx;
  gyro_samples_t                      gyro_history[GYRO_HISTORY_LENGTH];
  af_roi_info_t                       cur_roi_info;
} af_biz_internal;

void * af_biz_load_function(af_ops_t *af_ops);
void af_biz_unload_function(af_ops_t *af_ops, void *lib_handler);

#endif /* __BIZ_AF_H__ */
