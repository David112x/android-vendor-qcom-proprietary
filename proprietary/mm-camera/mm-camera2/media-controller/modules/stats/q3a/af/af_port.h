/* af_port.h
 *
 * Copyright (c) 2013, 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __PORT_AF_H__
#define __PORT_AF_H__

#include <mct_stream.h>
#include "q3a_thread.h"
#include "modules.h"
#include "af_fdprio.h"
#include "stats_chromatix_wrapper.h"

/* Every AF sink port ONLY corresponds to ONE session */

/** af_port_pdaf_status: PDAF Extraction Status
 **/
typedef enum {
  AF_PORT_PDAF_STATUS_EXTRACT_FAILURE = 0,
  AF_PORT_PDAF_STATUS_EXTRACT_SUCCESS,
  AF_PORT_PDAF_STATUS_LIBRARY_MISSING,
  AF_PORT_PDAF_STATUS_MAX
} af_port_pdaf_status;


/** af_port_state_transition_type: cause of transition of AF
 *  state
 **/
typedef enum {
  AF_PORT_TRANS_CAUSE_TRIGGER,
  AF_PORT_TRANS_CAUSE_CANCEL,
  AF_PORT_TRANS_CAUSE_AF_DONE_FOCUSED,
  AF_PORT_TRANS_CAUSE_AF_DONE_UNFOCUSED,
  AF_PORT_TRANS_CAUSE_MODE_CHANGE,
  AF_PORT_TRANS_CAUSE_SCANNING,
} af_port_state_transition_type;

/** _af_port_state_trans: Information regarding AF state
*   transition
*    @trigger: trigger structure to report back to HAL
*    @state_changed: true if state has been changed
*    @cause: reason of transition
*    @af_state: current AF state
*
**/
typedef struct _af_port_state_trans {
  af_trigger_t trigger;
  af_port_state_transition_type cause;
  cam_af_state_t    af_state;
} af_port_state_trans_t;
/** af_port_state_t:
 *
 * Enum with the states of the AF port
 *
 **/
typedef enum {
  AF_PORT_STATE_CREATED,
  AF_PORT_STATE_RESERVED,
  AF_PORT_STATE_LINKED,
  AF_PORT_STATE_UNLINKED,
  AF_PORT_STATE_UNRESERVED
} af_port_state_t;

typedef struct {
  boolean is_early_pd_buf_in_use; /* Set if using early PD buffer */
  boolean is_buf_div_done; /* Set once buf divert processing is done */

  isp_buf_divert_t isp_buf_divert; /* Buffer divert data */
} af_port_div_buf_ack_ctrl_t;

typedef struct {
  pthread_mutex_t mutex;
  uint32_t max_buffers; /* Number of buffers allocated for div_buf_ack_ctrl */
  af_port_div_buf_ack_ctrl_t *div_buf_ack_ctrl; /* Control buf divert ack info, per buffer */
} af_port_div_buf_ack_t;

/** _af_pre_init_updates:
 *    @is_stream_info_updated: TRUE if stream info update
 *     received before AF init.
 *    @stream_info: saved isp dim output.
 *
 *  Any updates received before AF is initiliazed will be saved
 *  locally and then updated to AF algorithm once we get tuning
 *  pointer update.
 */
typedef struct _af_pre_init_updates {
  boolean is_stream_info_updated;
  mct_stream_info_t stream_info;

} af_pre_init_updates_t;

/** af_bus_info_t
 *  @status : bus msg for Af state/focus distance
 *  @roi_msg: bus msg for af roi
 *  @focus_mode : bus msg for af mode
 **/
typedef struct _af_bus_info_t {
  cam_area_t roi_msg;
  uint8_t focus_mode;
  cam_af_lens_state_t lens_state;
  cam_rect_t default_roi;
} af_bus_info_t;

/** af_port_tof_data_t
 *  @is_updated: new tof data has been updated
 *  @tof_update: laser data
 **/
typedef struct _af_port_tof_data {
  boolean is_updated;
  tof_update_t tof_update;
} af_port_tof_data_t;

/** af_port_cam_info_t
 **/
typedef struct {
  af_actuator_info_t actuator_info;
  q3a_sensor_type_t   sensor_type; /* Sensor type use. */
  q3a_lens_type_t     lens_type; /* Type of lens the sensor use */
} af_port_cam_info_t;

/** af_port_fovc_params_t
 *  @is_fovc_enable: if true, fovc feature is enabled
 *  @mag_factor: crop factor for current lens position
 *  @delay_lens_move: if true, skip lens movement for
 *                 specified frames
 *  @frame_skip_cnt: number of frames to skip
 *  @move_lens: info to store if lens move is to be delayed
 */
typedef struct {
  boolean is_fovc_enabled;
  float mag_factor;
  boolean delay_lens_move;
  uint32_t frame_skip_cnt;
  af_move_lens_t move_lens;
} af_port_fovc_params_t;

/** af_dual_cam_info:
 *    @mode:                            Driving camera of the related camera sub-system
 *    @sync_3a_mode:              3A Sync Mode
 *    @cam_role:                      Camera Type in Dual Camera Mode
 *    @intra_peer_id:                 Intra peer Id
 *    @is_LPM_on:                     Is Low Power  Mode on
 *    @is_aux_sync_enabled:     Can we update Aux Camera with our peer info
 *    @is_aux_update_enabled:  Can Aux camera update its result to outside world
 *    @af_peer_focus_info:         Peer Focus information from master
**/
typedef struct {
  cam_sync_mode_t         mode;
  cam_3a_sync_mode_t      sync_3a_mode;
  cam_dual_camera_role_t  cam_role;
  uint32_t                intra_peer_id;
  boolean                 is_LPM_on;
  boolean                 is_aux_sync_enabled;
  boolean                 is_aux_update_enabled;
  af_core_focus_info      af_peer_focus_info;
}af_dual_cam_info;

/** _af_port_private:
 *    @camera_id:      Camera id defined by the algorithm thread, use as an
 *                     index while refering to thread structure.
 *    @reserved_id:    session index
 *    @stream_info:    stream parameters
 *    @state:          AF port state
 *    @af_object:      The AF object
 *    @thread_data:    AF thread data
 *    @dual_thread     Used by port extension on dual camera, to save original
 *                     thread info before overwrites it.
 *    @af_initialized: flag to indicate if AF has been initialized
 *                     and it's ok to forward events to the AF algorithm
 *    @fd_prio_data:   information about fd roi
 *    @isp_status:     data that holds info to be sent to isp once it says
 *                     it is ready.
 *    @enable_stats_mask: AF is configured for these stats
 *
 *  The private data structure of the AF port
 **/
typedef struct _af_port_private {
  uint8_t           camera_id;
  unsigned int      reserved_id;
  mct_stream_info_t stream_info;
  void              *af_iface_lib;
  boolean           af_extension_use;
  boolean           use_default_algo;
  af_port_state_t   state;
  af_object_t       af_object;
  q3a_thread_data_t *thread_data;
  boolean           af_initialized;
  boolean           af_not_supported;
  af_fdprio_t       fd_prio_data;
  cam_focus_mode_type prev_af_mode;
  cam_focus_mode_type af_mode;
  af_status_type    af_status;
  boolean           af_depth_based_focus;
  boolean           af_trigger_called;
  af_port_state_trans_t af_trans;
  pthread_mutex_t focus_state_mutex;
  boolean           focus_state_changed;
  uint32_t sof_id;
  int32_t           af_focus_pos;
  boolean           af_led_assist; /* if LED assisted AF is running */
  af_run_mode_type  run_type;
  af_stream_crop_t  stream_crop;
  af_input_from_isp_t isp_info;
  af_preview_size_t preview_size;
  af_sw_stats_t     *p_sw_stats;
  af_port_div_buf_ack_t div_buf_ack;
  af_input_from_aec_t latest_aec_info;
  int32_t hal_version;
  af_bus_info_t       bus;
  af_pre_init_updates_t pre_init_updates;
  uint32_t            stats_caps;
  boolean             video_caf_mode;
  boolean            gravity_data_ready;
  float              gravity_vector_data;
  float              gravity_vector_data_applied;
  int                over_flow_data;
  af_port_cam_info_t cam_info;
  boolean            reconfigure_ISP_pending;
  af_debug_info_t    debug_info;
  int                currentdistanceOfObjectInCm;
  float              lens_shift_um;
  float              focal_length_ratio;
  /* the focus distance & range info send to HAL3 */
  cam_focus_distances_info_t focus_distance_info;
  depth_service_input_t ds_input;
  af_input_manual_focus_t manual;
  float diopter;
  uint32_t scale_ratio;
  af_port_tof_data_t tof_data;
  /* Hook to extend functionality */
  stats_util_override_func_t    func_tbl;
  void                          *ext_param;
  boolean                       is_pdaf_lib_available;
  af_tuning_haf_adapter_t       af_haf_tuning;
  boolean                       caf_locked;
  uint32                        enable_stats_mask;
  uint32_t                      camif_width;
  uint32_t                      camif_height;
  int                           af_focus_pos_dac;
  uint32_t                      applying_delay;
  boolean                       focusedAtLeastOnce;
  float                         focus_value;
  boolean                       spot_light_detected;
  af_port_fovc_params_t         fovc_data;
  af_dual_cam_info              dual_cam_info;
} af_port_private_t;

void    af_port_deinit(mct_port_t *port);
boolean af_port_find_identity(mct_port_t *port, unsigned int identity);
boolean af_port_init(mct_port_t *port, unsigned int *session_id);
void* af_port_load_function(af_ops_t *af_ops);
void af_port_unload_function(af_port_private_t *private);
boolean af_port_load_dummy_default_func(af_ops_t *af_ops);
boolean af_port_set_session_data(mct_port_t *port, void *q3a_lib_info,
  mct_pipeline_session_data_t *session_data, unsigned int *sessionid);
static void af_port_manual_lens_sag(mct_port_t *port);
static boolean af_port_handle_set_focus_distance(mct_port_t *port, float focus_distance);
boolean af_send_bus_msg(mct_port_t *port, mct_bus_msg_type_t bus_msg_type,
  void *payload, int size, int sof_id);
q3a_thread_af_msg_t* af_port_create_msg(q3a_thread_af_msg_type_t msg_type,
  int param_type, af_port_private_t *af_port);
boolean af_port_process_status_update(mct_port_t  *port, af_output_data_t *af_out);
boolean af_port_update_af_state(mct_port_t  *port,
  af_port_state_transition_type cause);
boolean af_port_send_af_lens_state(mct_port_t  *port);
boolean af_port_buf_div_init_buf_divert_ack_ctrl(af_port_private_t *af_port,
  mct_bf_pd_stats_data_t *pd_data);
boolean af_port_buf_div_save_ack_data(af_port_private_t *af_port,
  isp_buf_divert_t *stats_buff);
boolean af_port_buf_div_done(af_port_private_t *af_port,
  isp_buf_divert_t *stats_buff);
boolean af_port_buf_div_send_ack_if_ready(mct_port_t *port,
  uint32_t buf_idx);
boolean af_port_buf_div_add_pd_early(af_port_private_t *af_port,
  mct_bf_pd_stats_data_t *pd_data);
boolean af_port_buf_div_pd_early_done(af_port_private_t *af_port,
  mct_bf_pd_stats_data_t *pd_data);

#endif /* __PORT_AF_H__ */
