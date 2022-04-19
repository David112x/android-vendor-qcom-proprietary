/* isp_module.h
 *
 * Copyright (c) 2012-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __ISP_MODULE_H__
#define __ISP_MODULE_H__

/* std headers */
#include <pthread.h>

/* kernel headers */
#include <media/msmb_isp.h>

/* mctl headers */
#include "media_controller.h"
#include "mct_stream.h"
#include "mct_queue.h"

/* ISP headers */
#include "isp_defs.h"
#include "isp_pipeline.h"
#include "isp_common.h"
#include "isp_resource.h"

#define BUNDLE_PARAM_MAX 3
/** isp_bundle_params_t:
 *
 *  @bundle_info: copy of HAL bundle info
 *  @state: current state of bundle
 *  @streamon_count: Number of stream in bundle that are
 *                 currently streaming
 *  @caps_unreserve_count: Number of stream for which caps
 *                       unreserve is called
 **/
typedef struct {
  cam_bundle_config_t bundle_info;
  isp_state_t         state;
  uint32_t            streamon_count;
  uint32_t            caps_unreserve_count;
} isp_bundle_params_t;

typedef enum{
  ISP_STREAM_STATE_INACTIVE,
  ISP_STREAM_STATE_ACTIVE,
} isp_stream_state;

/** isp_stream_param_t:
 *
 *  @mutex: stream based mutex
 *  @is_stream_on: boolean to store whether stream in ON
 *  @hw_stream: hw stream id
 *  @num_hw_stream: number of hw streams
 *  @stream_info: stream info handle
 *  @int_stream: internal stream per ISP
 *  @ref_count: ref count
 *  @bundle_params: bundle params
 *  @is_pipeline_supported: pipeline supported flag
 **/
typedef struct {
  pthread_mutex_t      mutex;
  isp_hw_streamid_t    hw_stream[ISP_HW_STREAM_MAX];
  uint32_t             num_hw_stream;
  mct_stream_info_t    stream_info;
  mct_stream_t        *int_stream[ISP_HW_MAX];
  uint32_t             ref_count;
  boolean              is_pipeline_supported;
  isp_bundle_params_t *bundle_params[BUNDLE_PARAM_MAX];
  boolean              need_divert;
  isp_stream_state     stream_state;
  uint32_t             bundle_params_cnt;
} isp_stream_param_t;

typedef enum{
  ISP_REG_UPDATE_STATE_NOT_REQUESTED = 0,
  ISP_REG_UPDATE_STATE_RECEIVED,
  ISP_REG_UPDATE_STATE_REQUESTED,
  ISP_REG_UPDATE_STATE_PENDING
} isp_reg_update_state_t;

/** isp_hw_update_list_params_t:
 *
 *  @fd: fd to communicate to kernel driver
 *  @cur_hw_update_list: hw update list that is updated in
 *                     current frame
 *  @cur_stats_params: cur stats params
 *  @applied_stats_params: applied stats params
 **/
typedef struct {
  int32_t                      fd;
#ifdef VIDIOC_MSM_ISP_AHB_CLK_CFG
  enum msm_vfe_ahb_clk_vote    ahb_clk_vote;
#endif
  struct msm_vfe_cfg_cmd_list *cur_hw_update_list;
  isp_saved_stats_params_t    *stats_params;
  isp_saved_stats_params_t    *applied_stats_params;
  uint32_t                     metadata_dump_enable;
  isp_meta_t                   meta_dump_parms;
  isp_meta_t                   applied_meta_dump_parms;
  isp_meta_t                  *temp_meta_dump_parms;
  uint32_t                    *meta_reg_dump;
  isp_dmi_tbl_meta_t           dmi_tbl;
  isp_dmi_tbl_meta_t           applied_dmi_tbl;
  isp_algo_params_t            algo_parm;
  boolean                      cur_cds_update;
  boolean                      new_cds_update;
  isp_per_frame_meta_t         frame_meta;
  isp_per_frame_meta_t         applied_frame_meta;
} isp_hw_update_list_params_t;

/** isp_hw_update_params_t:
 *
 *  @cond: pthread cond
 *  @mutex: pthread mutex
 *  @hw_update_thread: hw update thread handle
 *  @pipe_fd: pipe fd to talk to master thread
 *  @is_thread_alive: boolean to store whether the thread is
 *                  alive
 *  @hw_update_list_params: hw update list params
 *  @is_thread_busy: flag to indicate whether hw update thread
 *                 is busy
 **/
typedef struct {
  pthread_cond_t              cond;
  pthread_mutex_t             mutex;
  pthread_t                   hw_update_thread;
  int                         pipe_fd[2];
  boolean                     is_thread_alive;
  uint32_t                    metadata_dump_enable;
  boolean                     skip_hw_update;
  boolean                     cds_updating;
  isp_hw_update_list_params_t hw_update_list_params[ISP_HW_MAX];
  volatile boolean            is_thread_busy;
} isp_hw_update_params_t;

/** isp_trigger_update_params_t:
 *
 *  @cond: cond variable
 *  @mutex: mutex lock
 *  @pipe_fd: pipe fd to talk to master thread
 *  @trigger_update_thread: trigger update thread handle
 *  @is_thread_alive: boolean to store whether thread is alive
 *  @cur_events: store trigger events that are
 *            processed in current frame
 *  @new_events: store trigger events that are received in
 *            current frame
 *  @is_thread_busy: flag to indicate whether hw update thread
 *                 is busy
 **/
typedef struct {
  pthread_cond_t          cond;
  pthread_mutex_t         mutex;
  int                     pipe_fd[2];
  pthread_t               trigger_update_thread;
  boolean                 is_thread_alive;
  isp_saved_events_t      cur_events;
  isp_saved_events_t      new_events;
  volatile boolean        is_thread_busy;
} isp_trigger_update_params_t;

typedef enum {
  ISP_BUF_STATE_FREE,
  ISP_BUF_STATE_USED,
  ISP_BUF_STATE_MAX,
} isp_buf_state_t;

/* Track the avalability of MAX_STATS_CONFIG_BUF */
typedef enum {
  ISP_BUNDLED_BUF_STATE_FREE,
  ISP_BUNDLED_BUF_STATE_USED,
  ISP_BUNDLED_BUF_STATE_MAX,
} isp_bundled_buf_state_t;

/** isp_stats_buf_info_t:
 *
 *  @addr: Virtual address of buffer
 *  @buf_size: size of buffer
 *  @buf_state: buffer state
 **/
typedef struct {
  void            *addr;
  uint32_t         buf_size;
  isp_buf_state_t  buf_state;
} isp_stats_buf_info_t;

/** isp_stats_buf_mgr_t:
 *
 *  @mutex: mutex
 *  @stats_buf_info: stats buffer info
 *  @num_stats_buf_info: num of stats buffers per stats
 *  @stats_event_data: stats event payload
 *  @stats_event_data_state: state of stats event palyload
 *  @num_stats_event_data: num of stats event payload
 **/
typedef struct {
  pthread_mutex_t           mutex;
  isp_stats_buf_info_t     *stats_buf_info[MSM_ISP_STATS_MAX];
  uint32_t                  num_stats_buf_info;
  mct_event_stats_isp_t    *stats_event_data;
  isp_buf_state_t          *stats_event_data_state;
  uint32_t                  num_stats_event_data;
  boolean                   init_done;
} isp_stats_buf_mgr_t;

typedef struct {
  iface_raw_stats_buf_info_t raw_stats_info;
  isp_saved_stats_params_t    stats_params[ISP_HW_MAX];
  isp_algo_params_t           algo_parm[ISP_HW_MAX];
  isp_bundled_buf_state_t     buf_state;
} isp_bundled_stats_buf_info_params_t;

/** isp_bundled_stats_buf_mgr_t:
 *  @mutex: mutex
 *  @bundled_stats_buf_info: buffer used to parse param
 *  @from polling thread to parser thread
 **/
typedef struct {
  pthread_mutex_t                      mutex;
  isp_bundled_stats_buf_info_params_t *bundled_stats_buf_info;
  boolean                              init_done;
} isp_bundled_stats_buf_mgr_t;


/** isp_parser_params_t:
 *
 *  @cond: cond variable
 *  @mutex: mutex lock
 *  @pipe_fd: pipe fd to talk to master thread
 *  @trigger_update_thread: trigger update thread handle
 *  @is_thread_alive: boolean to store whether thread is alive
 *  @stats_notify_event: stats notify event to be handled in
 *                     parser thread
 *  @is_stats_notify_event_valid: is stats notify event valid
 *                              flag
 *  @cur_stats_params: cur stats params
 *  @new_stats_params: new saved stats params
 *  @tintless_params: tintless params
 *  @stats_mask: stats mask to store stats that are enabled
 *  @parser_session_params: session params
 *  @buf_mgr: buffer manager handle
 *  @resource_request: iface resource request for online isp
 *  @resource_request_offline: iface resource request for
 *                           offline isp
 **/
typedef struct {
  pthread_cond_t              cond;
  pthread_mutex_t             mutex;
  int                         pipe_fd[2];
  pthread_t                   parser_thread;
  boolean                     is_thread_alive;
  mct_event_t                *stats_notify_event;
  isp_saved_stats_params_t    stats_params[ISP_HW_MAX];
  mct_queue_t                *in_stats_queue;
  isp_algo_params_t           algo_parm[ISP_HW_MAX];
  isp_saved_algo_params_t     algo_saved_parm;
  isp_stats_mask_ctrl_t       stats_ctrl;
  isp_parser_session_params_t parser_session_params;
  isp_stats_buf_mgr_t         buf_mgr[ISP_STREAMING_MAX];
  iface_resource_request_t    resource_request[ISP_STREAMING_MAX];
  isp_bundled_stats_buf_mgr_t bundled_stats_buf_mgr;
} isp_parser_params_t;

/** isp_module_ports_t:
 *
 *  @isp_sink_port: ISP sink port
 *  @isp_source_stats_port: ISP source port for stats
 *  @isp_source_frame_port: ISP source frame port for frame
 *  @isp_submod_ports: ISP submodule ports
 **/
 #define ADDITIONAL_SRC_PORTS 3
typedef struct {
  mct_port_t *isp_sink_port;
  mct_port_t *isp_source_stats_port;
  mct_port_t *isp_source_frame_port[ISP_HW_STREAM_MAX + ADDITIONAL_SRC_PORTS];
  mct_port_t *isp_submod_ports[ISP_HW_MAX];
} isp_module_ports_t;

/** isp_stream_port_map_info_t:
 *
 *  @hw_stream: HW stream ID
 *  @stream_sizes: stream dimension
 *  @stream_type: stream type
 *  @native_buffer: show if native buffer will be used
 **/
typedef struct {
  isp_hw_streamid_t hw_stream;
  cam_dimension_t   stream_sizes;
  cam_stream_type_t stream_type;
  uint32_t          native_buffer;
  uint32_t          use_duplicate_output;
  uint32_t          controlable_output;
  uint32_t          shared_output;
  uint32_t          mapped_mask;
  cam_dimension_t   orig_stream_sizes;
  cam_dimension_t   changed_stream_sizes;
  boolean           is_changed;
} isp_stream_port_map_info_t;

/** isp_stream_port_map_t:
 *
 *  @num_streams: number for streams
 *  @streams: handle stream dimension and mapping info
 **/
typedef struct{
  boolean raw_stream_exists;
  uint32_t num_streams;
  isp_stream_port_map_info_t streams[ISP_MAX_STREAMS];
} isp_stream_port_map_t;

/** meta_isp_stream_crop:
 *
 *  @msg: stream crop info
 *  @valid_entry: stream exists for the index
 **/
typedef struct {
  mct_bus_msg_stream_crop_t    msg;
  boolean                      valid_entry;
}meta_isp_stream_crop;

/** isp_algo_sesion_params_t:
 *
 *  @tintless_params: tintless params
 *
 **/
typedef struct {
  void *tintless_params;
} isp_algo_sesion_params_t;

/* isp_bundled_stats_buf_info_params_t
 * @raw_stats_info - Raw stats before parsing
 * @stats_params - stats parmas
 * @algo_params - algo parameters
 **/

typedef struct {
  mct_bus_msg_isp_gamma_t         meta_set_isp_gamma_info;
  boolean                         valid_set_isp_gamma_info;
  cam_scene_mode_type             meta_meta_current_scene;
  boolean                         valid_meta_current_scene;
  vfe_diagnostics_t               meta_isp_chromatix_lite;
  boolean                         valid_isp_chromatix_lite;
  awb_update_t                    meta_set_isp_stats_awb_info;
  boolean                         valid_set_isp_stats_awb_info;
  cam_lens_shading_map_t          meta_set_isp_lens_shading_info;
  boolean                         valid_set_isp_lens_shading_info;
  cam_lens_shading_mode_t         meta_set_isp_lens_shading_mode;
  boolean                         valid_set_isp_lens_shading_mode;
  cam_lens_shading_map_mode_t     meta_set_isp_lens_shading_map_mode;
  boolean                         valid_set_isp_lens_shading_map_mode;
  cam_tonemap_mode_t              meta_set_isp_tone_map_mode;
  boolean                         valid_set_isp_tone_map_mode;
  cam_rgb_tonemap_curves          meta_set_isp_tone_map;
  boolean                         valid_set_isp_tone_map;
  cam_color_correct_mode_t        meta_set_isp_cc_mode;
  boolean                         valid_set_isp_cc_mode;
  cam_color_correct_matrix_t      meta_set_isp_cc_transform;
  boolean                         valid_set_isp_cc_transform;
  cam_color_correct_gains_t       meta_set_isp_cc_gain;
  boolean                         valid_set_isp_cc_gain;
  uint8_t                         meta_set_isp_control_mode;
  boolean                         valid_set_isp_control_mode;
  cam_crop_region_t               meta_isp_crop_region;
  boolean                         valid_isp_crop_region;
  uint8_t                         meta_set_isp_abf_mode;
  boolean                         valid_set_isp_abf_mode;
  uint8_t                         meta_set_isp_hot_pix_mode;
  boolean                         valid_set_isp_hot_pix_mode;
  uint8_t                         meta_set_isp_black_level_lock;
  boolean                         valid_set_isp_black_level_lock;
  int32_t                         meta_set_isp_zoom_value;
  boolean                         valid_set_isp_zoom_value;
  cam_effect_mode_type            meta_set_isp_special_effect;
  boolean                         valid_set_isp_special_effect;
  int32_t                         meta_set_isp_contrast;
  boolean                         valid_set_isp_contrast;
  int32_t                         meta_set_isp_saturation;
  boolean                         valid_set_isp_saturation;
  cam_aberration_mode_t           meta_set_isp_cac_mode;
  boolean                         valid_set_isp_cac_mode;
  uint8_t                         meta_set_isp_capture_intent;
  boolean                         valid_set_isp_capture_intent;
  cam_profile_tone_curve          meta_profile_tone_curve;
  boolean                         valid_profile_tone_curve;
  isp_meta_t                      meta_isp_meta;
  boolean                         valid_isp_meta;
  mct_bus_msg_session_crop_info_t session_crop;
  boolean                         valid_isp_stream_crop;
  meta_isp_stream_crop            stream_crop[ISP_MAX_STREAMS_TO_HAL];
  mct_bus_msg_isp_bhist_stats_t   meta_hist_stats_info;
  boolean                         valid_hist_stats_info;
  cam_neutral_col_point_t         meta_neutral_col_point;
  boolean                         valid_neutral_col_point;
  cam_stream_crop_info_t          meta_isp_snap_crop;
  boolean                         valid_isp_snap_crop;
  cam_stream_crop_info_t          meta_pp_snap_crop;
  boolean                         valid_pp_snap_crop;
  mct_bus_msg_isp_low_power_mode_t meta_set_isp_lpm_info;
  boolean                         valid_set_isp_lpm_info;
  cam_sensor_hdr_type_t           meta_set_sensor_hdr_info;
  boolean                         valid_set_sensor_hdr_info;
  int32_t                         isp_dig_gain;
  boolean                         valid_isp_dig_gain;
  mct_bus_msg_hdr_data            meta_hdr_data;
  boolean                         valid_hdr_data;
  int32_t                         meta_tintless_enable;
  boolean                         valid_tintless;
} isp_meta_container_t;

typedef struct{
  unsigned int                     last_reg_update_frame_id;
  volatile isp_reg_update_state_t  reg_update_state;
} isp_reg_update_info_t;

typedef struct{
  boolean                      isupdated;
  boolean                      storedenableLPM;
  boolean                      enableLPM;
  boolean                      meta_report_pending;
} isp_lpm_info_t;

#define MAX_ISP_STATS (6*2)
#define ISP_START_FRAME_ID  100
#define ISP_STATS_FILE "//data//misc//camera//camstats.bin"

typedef struct{
  mct_event_stats_isp_t saved_stats[MAX_ISP_STATS];
  uint8_t frame_index;
} isp_stats_data_t;

typedef struct{
  boolean  csidtg_enable;
  boolean  dump_stats;
} isp_property_t;

typedef struct{
  FILE *stats_fd;
  uint8_t frame_wr_cnt;
  isp_stats_data_t *isp_stats;
} isp_stats_data_csidtg_t;

/** isp_session_params_t:
 *
 *  @state: ISP state
 *  @state_mutex: mutex only to be used with above state
 *  @thread_busy_mutex: mutex to change / read thread busy flag
 *                    for trigger update and hw update thread
 *  @mutex: pthread mutex
 *  @metadata_mutex: mutex only to be used with meta data
 *  @session_id: session id
 *  @num_isp: number of ISP's used by this session
 *  @hw_id: ISP hw id used
 *  @stream_on_count: number of streams that is streaming
 *  @stream_port_map: stream mapping info
 *  @sensor_output_info: sensor output descrtiption
 *  @hal_version: HAL version
 *  @curr_frame_id: current frame id
 *  @max_apply_delay: maximum applying delay of all modules
 *  @max_reporting_delay: maximum reporting delay of all moudules
 *  @metadata: metadata container
 *  @l_stream_params: list of stream params
 *  @trigger_update_params: trigger update params
 *  @hw_update_params: hw update params
 *  @parser_params: parser params
 *  @l_bundle_params: list of bundle params
 *  @isp_ports: ISP ports for current session
 *  @saved_events: saved events to be passed before every stream
 *               on
 *  @session_based_ide: session based identity
 **/
typedef struct {
  isp_state_t                  state;
  pthread_mutex_t              state_mutex;
  pthread_mutex_t              mutex;
  pthread_mutex_t              thread_busy_mutex;
  pthread_mutex_t              metadata_mutex;
  uint32_t                     session_id;
  uint32_t                     num_isp;
  /* Fill this and remove from hw_update_list_params */
  isp_hw_id_t                  hw_id[ISP_HW_MAX];
  uint32_t                     stream_on_count;
  isp_stream_port_map_t        stream_port_map;
  boolean                      hw_stream_linked[ISP_HW_STREAM_MAX];
  sensor_set_dim_t             sensor_output_info;
  sensor_set_dim_t             offline_input_cfg;
  cam_hal_version_t            hal_version;
  uint32_t                     curr_frame_id;
  uint32_t                     offline_frame_id;
  uint32_t                     max_apply_delay;
  uint32_t                     max_reporting_delay;
  isp_meta_container_t         metadata[ISP_MAX_META_REPORTING_QUEUE_DEPTH];

  mct_list_t                  *l_stream_params; /* isp_stream_param_t */
  isp_trigger_update_params_t  trigger_update_params;
  isp_hw_update_params_t       hw_update_params;
  isp_parser_params_t          parser_params;
  mct_list_t                  *l_bundle_params; /* isp_bundle_params_t */
  isp_module_ports_t           isp_ports;
  isp_saved_events_t           saved_events;
  boolean                      fast_aec_mode;
  uint32_t                     session_based_ide;
  uint32_t                     offline_num_isp;
  isp_hw_id_t                  offline_hw_id[ISP_HW_MAX];
  isp_fetch_eng_cfg_data_t     fetch_eng_cfg_data;
  uint32_t                     snapshot_path;
  isp_reg_update_info_t        reg_update_info;
  void                         *online_meta_hw_update_list;
  boolean                      is_hw_update_overwrite;
  isp_preferred_streams        preferred_mapping;
  isp_submod_hw_limit_t        isp_hw_limit;
  isp_lpm_info_t               lpm_info;
  cam_format_t                 sensor_fmt;
  isp_saved_events_t           cur_super_events;
  isp_saved_events_t           new_super_events;
  boolean                      multi_pass;
  isp_property_t               isp_props;
  isp_saved_tintless_params_t  offline_tinltess_params;
  isp_stats_data_csidtg_t      csidtg_data;
  /* In svhdr mode Linearization disabled, AutoLTM enabled,
  *  3A updates delayed and applyed on FE SOF */
  boolean                      svhdr_enb;
  mct_queue_t                 *offline_trigger_param_q; /* isp_saved_events_t */
  boolean                      rdi_only;
  mct_hdr_data                 hdr_data_cur;
  mct_hdr_data                 hdr_data_stored;
  /* binncorr flag is used to delay 3A updpates
  *  and apply on FE sof */
  boolean                      binncorr_mode;
  cam_sync_type_t              sync_type;
  boolean                      is_dualCam;
  boolean                      is_depth;
} isp_session_param_t;

/** isp_update_dim_t:
 *
 *  @session_param: session param handle
 *  @dim: handle to store dimension
 **/
typedef struct {
  isp_session_param_t *session_param;
  isp_dim_t           *dim;
} isp_update_dim_t;

/** isp_t:
 *
 *  @isp_resource: ISP resource
 *  @isp cap: ISP static capability
 *  @l_session_params: list of session params of type
 *              isp_session_param_t
 **/
typedef struct {
  isp_resource_t  isp_resource;
  mct_list_t     *l_session_params; /* isp_session_param_t */
  mct_pipeline_isp_cap_t isp_cap;
} isp_t;

/* For test app */
mct_module_t *module_isp2_init(const char *name);
void module_isp2_deinit(mct_module_t *module);

#endif
