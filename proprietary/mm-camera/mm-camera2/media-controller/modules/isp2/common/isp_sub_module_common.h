/* isp_submodule_common.h
 *
 * Copyright (c) 2014-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ISP_SUB_MODULE_COMMON_H__
#define __ISP_SUB_MODULE_COMMON_H__

/* mctl headers */
#include "mct_port.h"

/* isp headers */
#include "isp_common.h"
#include "isp_log.h"

#define NUM_SINK_PORTS             1
#define NUM_SOURCE_PORTS           3
#define MAX_SRC_PORTS 3
#define MAX_STREAMS   8
#define MAX_SUB_MODULE_NAME        100

/** isp_submod_init_table_t:
 *
 *  @module_init: module init handle
 *  @module_deinit: module deinit handle
 **/
typedef struct {
  mct_module_t *(*module_init)(const char *);
  void (*module_deinit)(mct_module_t *);
} isp_submod_init_table_t;

extern uint32_t isp_modules_loglevel[];

/** isp_manual_mode_ctrls_t: Generic structure used for saving
 * per frame manual controls
 *
 *  @app_ctrl_mode:            manual meta mode
 *  @black_level_lock:   Enable/Disable trigger for black level
 *  @abf_mode:                 Enable/Disable ABF module
 *  @lens_shading_mode:        Enable/Disable LSM module
 *  @lens_shading_map_mode:    LSM map mode, to send map or not
 *  @wb_mode:                  manual WB mode
 *  @cc_mode:                  cc mode fast/manual
 *  @color_correction_matrix:  manual CC matrix
 *  @cc_transform_matrix:      CC matrix converted to float
 *  @update_cc:                flag to update CC matrix
 *  @cc_gain:                  manual WB gains
 *  @update_cc_gain:           flag to update WB gain
 *  @update_gamma:             flag to update gamma
 *  @gamma_curve:              manual tone map curve
 *  @tonemap_mode:             Tone map mode fast/manual
 *  @manual_update:            Flag to indicate manual update
 **/
typedef struct {
  cam_control_mode_t               app_ctrl_mode;
  cam_ae_mode_type                 aec_ctrl_mode;
  float                            isp_gain;
  cam_black_level_lock_t           black_level_lock;
  cam_hotpixel_mode_t              hot_pix_mode;
  cam_noise_reduction_mode_t       abf_mode;

  cam_lens_shading_mode_t          lens_shading_mode;
  cam_lens_shading_map_mode_t      lens_shading_map_mode;

  cam_wb_mode_type                 wb_mode;
  cam_color_correct_mode_t         cc_mode;
  cam_color_correct_matrix_t       color_correction_matrix;
  float                            cc_transform_matrix[3][3];
  int8_t                           update_cc;
  cam_color_correct_gains_t        cc_gain;
  int8_t                           update_cc_gain;

  int8_t                           update_gamma;
  cam_rgb_tonemap_curves           gamma_curve;
  cam_tonemap_mode_t               tonemap_mode;
  cam_aberration_mode_t            cac_mode;
  int8_t                           manual_update;
}isp_manual_mode_ctrls_t;

/** isp_sub_module_store_src_port_t:
 *
 *  @source_port: handle to store source port
 *  @source_port_count: number of streams associated with
 *                    source port
 *  @identity: array to store identity associated with this port
 **/
typedef struct {
  mct_port_t *source_port;
  uint32_t    source_port_count;
  uint32_t    identity[MAX_STREAMS];
} isp_sub_module_store_src_port_t;

/** isp_sub_module_store_port_t:
 *
 *  @sink_port: sink port handle
 *  @sink_port_count: number of streams associated with sink
 *                  port
 *  @source_port_data: array to store source port information
 **/
typedef struct {
  mct_port_t                      *sink_port;
  uint32_t                         sink_port_count;
  isp_sub_module_store_src_port_t  source_port_data[MAX_SRC_PORTS];
} isp_sub_module_store_port_t;

/** isp_sub_module_snp_strm_t:
 *
 *  @is_snapshot_stream_on: snpashot stream on
 *  @snapshot_stream_count: keeps count of number of snapshot stream
 **/
typedef struct {
  boolean            is_snapshot_stream_on;
  int                snapshot_stream_count;
  boolean            is_cb_stream_on;
  int                cb_stream_count;
} isp_sub_module_snp_strm_t;

/** isp_sub_module_t:
 *
 *  @state: ISP state
 *  @ref_count: ref count
 *  @session_id: session id
 *  @stream_on_count: stream on count
 *  @mutex: mutex
 *  @l_stream_info: list of stream info
 *  @chroamtix ptrs: struct to store chroamtix ptrs
 *  @hw_update_pending: hw update pending flag
 *  @submod_enable: submod enable
 *  @submod_trigger_enable: submod trigger enable
 *  @update_module_bit: Update module bit to enable or disable
 *                    module
 *  @hw_module_id: hw module id
 *  @hw_update_list: hw update list
 *  @private_data: module specific private data
 *  @private_data_size: module specific private data size
 *  @rgn_skip_pattern: rgn skip pattern for version 48 and above
 **/
typedef struct {
  isp_state_t                  state;
  uint32_t                     ref_count;
  uint32_t                     session_id;
  uint32_t                     stream_on_count;
  pthread_mutex_t              mutex;
  pthread_mutex_t              new_mutex;
  mct_list_t                  *l_stream_info; /* mct_stream_info_t */
  modulesChromatix_t           chromatix_ptrs;
  boolean                      trigger_update_pending;
  boolean                      config_pending;
  boolean                      submod_enable;
  boolean                      chromatix_module_enable;
  boolean                      disable_module;
  boolean                      submod_trigger_enable;
  boolean                      update_module_bit;
  boolean                      vfe_diag_enable;
  boolean                      tintless_enabled;
  boolean                      zsl_mode;
  boolean                      dual_led_calibration_enabled;
  isp_hw_module_id_t           hw_module_id;
  isp_manual_mode_ctrls_t      manual_ctrls;
  struct msm_vfe_cfg_cmd_list *hw_update_list;
  uint32_t                     region_idx;
  boolean                      ae_bracketing_enable;
  void                        *private_data;
  uint32_t                     private_data_size;
  isp_sub_module_snp_strm_t    stream_data;
  cam_sensor_hdr_type_t        hdr_mode;
  isp_rgn_skip_pattern         rgn_skip_pattern;
  enum msm_isp_stats_type      hw_stats_type;
  isp_stats_tap_loc            stats_tap_location;
} isp_sub_module_t;

typedef boolean (*isp_sub_module_init_data_func)(mct_module_t *,
  isp_sub_module_t *);

typedef void (*isp_sub_module_destroy_func)(mct_module_t *,
  isp_sub_module_t *);

typedef boolean (*isp_sub_module_event_func)(mct_module_t *,
  isp_sub_module_t *, mct_event_t *);

typedef boolean (*isp_sub_module_query_cap_func) (mct_module_t *module,
  void *query_buf);

/** isp_sub_module_private_func_t:
 *
 *  @isp_sub_module_init_data: function pointer for isp
 *                           submodule init data
 *  @isp_sub_module_destroy: function pointer for isp submodule
 *                         destroy
 *  @control_event_handler: function pointer for control event
 *                        handlers
 *  @module_event_handler: function pointer for module event
 *                       handlers
 *  @isp_private_event_handler: function pointer for isp private
 *                            event handlers
 **/
typedef struct {
  isp_sub_module_init_data_func isp_sub_module_init_data;
  isp_sub_module_destroy_func   isp_sub_module_destroy;
  isp_sub_module_query_cap_func isp_sub_module_query_cap;
  isp_sub_module_event_func     control_event_handler[ISP_CONTROL_EVENT_MAX];
  isp_sub_module_event_func     module_event_handler[ISP_MODULE_EVENT_MAX];
  isp_sub_module_event_func     isp_private_event_handler[ISP_PRIVATE_MAX];
  isp_sub_module_event_func     set_param_handler[ISP_SET_MAX];
} isp_sub_module_private_func_t;

/** isp_sub_module_priv_t:
 *
 *  @l_session_params: list of session params
 *  @num_sink_ports: number of sink ports
 *  @num_source_ports: number of source ports
 *  @hw_module_id: hw module id
 *  @private_func: private function table
 *  @port_data: cache port information
 **/
typedef struct {
  mct_list_t                    *l_session_params; /* isp_sub_module_t */
  uint32_t                       num_sink_ports;
  uint32_t                       num_source_ports;
  isp_hw_module_id_t             hw_module_id;
  char                           module_name[MAX_SUB_MODULE_NAME];
  isp_log_sub_modules_t          mod_id_for_log;
  isp_sub_module_private_func_t *private_func;
  isp_sub_module_store_port_t    port_data;
} isp_sub_module_priv_t;

typedef struct
{
  uint8_t index;
  float   scale_factor;
  boolean is_stream_on;
}scaler_scale_factor_for_cac;

#endif
