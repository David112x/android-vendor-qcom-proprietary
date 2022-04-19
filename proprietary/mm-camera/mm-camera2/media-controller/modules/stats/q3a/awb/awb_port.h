/* awb_port.h
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AWB_PORT_H__
#define __AWB_PORT_H__
#include "modules.h"
#include "awb.h"
#include "q3a_thread.h"

typedef enum {
  AWB_PORT_STATE_CREATED,
  AWB_PORT_STATE_RESERVED,
  AWB_PORT_STATE_LINKED,
  AWB_PORT_STATE_UNLINKED,
  AWB_PORT_STATE_UNRESERVED
} awb_port_state_t;

typedef struct {
  awb_manual_update_t u;
  boolean valid;
  /* cct, gain or max */
  manual_wb_mode_type         manual_wb_type;
} awb_port_m_gain_t;

typedef struct {
  uint32_t frames_req; /* Tuning: Frames req to be consider converged */
  uint32_t frame_counter; /* Counter of frames while converging */
  int32_t previous_color_temp; /* Previous color temperature reported by algo */
  uint32_t prev_stats_id;
} awb_port_converge_t;

/** awb_frame_capture_t:
 *    @skip_stats_start_id:  Stat id to start skip logic
 *    @skip_count:      Number of stats to skip
**/
typedef struct {
  awb_frame_batch_t   frame_info;
  boolean             frame_capture_mode;
  int8_t              current_batch_count;
} awb_frame_capture_t;

typedef struct {
  uint32_t            max_sensor_delay;
  q3a_sensor_type_t   sensor_type; /* Sensor type use. */
  q3a_lens_type_t     lens_type; /* Type of lens the sensor use */
} awb_port_sensor_info_t;

typedef struct _awb_peer_data_t {
  awb_output_data_t         output;
  boolean                   is_awb_converged;   /**< Convergence status of peer */
  awb_interpolation_gain_params_t peer_gains;
} awb_peer_data_t;

/** awb_dual_cam_bus_msg_info:
 *    @urgent_msg:                            Driving camera of the related camera sub-system
 *    @awb_msg:              3A Sync Mode
 *    @regular_sof_id:                      Camera Type in Dual Camera Mode
**/
typedef struct {
  mct_bus_msg_awb_immediate_t urgent_msg;
  mct_bus_msg_awb_t awb_msg;
  uint32_t urgent_sof_id;
  uint32_t regular_sof_id;
} awb_dual_cam_bus_msg_info;

/** awb_dual_cam_info:
 *    @mode:                              Driving camera of the related camera sub-system
 *    @sync_3a_mode:                3A Sync Mode
 *    @cam_role:                        Camera Type in Dual Camera Mode
 *    @intra_peer_id:                   Intra peer Id
 *    @is_LPM_on:                       Is Low Power  Mode on
 *    @is_aux_sync_enabled:       Can we update Aux Camera with our peer info
 *    @is_aux_update_enabled:    Can Aux camera update its result to outside world
 *    @is_algo_active:                  is Slave Algorithm Active
 *    @peer_update:                    Interpolated stats result from Master
**/
typedef struct {
  cam_sync_mode_t         mode;
  cam_3a_sync_mode_t      sync_3a_mode;
  cam_dual_camera_role_t  cam_role;
  uint32_t                intra_peer_id;
  boolean                 is_LPM_on;
  boolean                 is_aux_sync_enabled;
  boolean                 is_aux_update_enabled;
  boolean                 is_algo_active;
  awb_peer_data_t         peer_update;
  awb_interpolation_gain_params_t interpolated_gains;
  awb_overlap_color_info_t overlap_color_info;
  awb_sync_mode_t         sync_mode;
}awb_dual_cam_info;

/** awb_port_private
 *    @camera_id:      Camera id defined by the algorithm thread, use as an
 *                     index while refering to thread structure.
 *    @awb_object: session index
 *    @port:       stream index
 *
 * Each awb moduld object should be used ONLY for one Bayer
 * serssin/stream set - use this structure to store session
 * and stream indices information.
 **/
typedef struct _awb_port_private {
  uint8_t              camera_id;
  unsigned int         reserved_id;
  cam_stream_type_t    stream_type;
  void                 *awb_iface_lib;
  boolean              awb_extension_use;
  boolean              use_default_algo;
  awb_port_state_t     state;
  boolean              awb_update_flag;
  awb_object_t         awb_object;
  q3a_thread_data_t    *thread_data;
  unsigned int         cur_sof_id;
  uint32_t             cur_stats_id;
  cam_awb_state_t      awb_state;
  cam_awb_state_t      awb_last_state;
  boolean              awb_locked;
  awb_operation_mode_t op_mode;
  int32_t              current_wb;
  cam_area_t           awb_roi;
  awb_port_sensor_info_t sensor_info;
  boolean              bg_stats_enabled;
  char                 awb_debug_data_array[AWB_DEBUG_DATA_SIZE];
  uint32_t             awb_debug_data_size;
  boolean              is_awb_converge;
  uint32_t             vfe_out_width;
  uint32_t             vfe_out_height;
  uint32_t             preview_width;
  uint32_t             preview_height;
  uint32_t             required_stats_mask;
  uint32_t             required_stats_mask_offline;
  float                conv_speed;
  /* HAL3 */
  awb_port_m_gain_t    manual;
  uint8_t              awb_auto_mode;
  uint8_t              awb_on_off_mode;
  uint8_t              awb_meta_mode;
  boolean              awb_ccm_enable;
  /* save the output, so we can send out the awb update for snapshot right away */
  awb_output_data_t    awb_output;
  /* flag to indicate if there is a capture intent for current SOF */
  uint32_t             awb_still_capture_sof;
  awb_frame_capture_t  stats_frame_capture;
  q3a_fast_aec_data_t  fast_aec_data;
  sem_t                sem_offline_proc;
  awb_output_data_t    offline_output;
  /* Hook to extend functionality */
  stats_util_override_func_t func_tbl;
  void                 *ext_param;
  boolean              is_still_capture;
  boolean              flash_on;
  /* Params will get updated in current sessiona and will be used in next
  session for improving the launch convergence */
  awb_stored_params_type *stored_params;
  uint16_t             fast_aec_forced_cnt;
  uint32               capture_intent_skip;
  aec_convergence_type instant_aec_type;
  awb_output_data_t    output_buffer;
  awb_dual_cam_info    dual_cam_info;
} awb_port_private_t;


void    awb_port_deinit(mct_port_t *port);
boolean awb_port_find_identity(mct_port_t *port, unsigned int identity);
boolean awb_port_init(mct_port_t *port, unsigned int *session_id);
void    awb_port_set_stored_parm(mct_port_t *port, awb_stored_params_type* stored_params);
void awb_send_bus_msg(mct_port_t *port, mct_bus_msg_type_t bus_msg_type,
  void *payload, int size , int sof_id, mct_bus_metadata_collection_type_t stream_type);
void * awb_port_load_function(awb_ops_t *awb_ops);
void awb_port_unload_function(awb_port_private_t *private);
boolean awb_port_set_session_data(mct_port_t *port, void *q3a_lib_info,
  mct_pipeline_session_data_t *session_data, unsigned int *sessionid);
boolean awb_port_load_dummy_default_func(awb_ops_t *awb_ops);
q3a_thread_aecawb_msg_t* awb_port_create_msg(q3a_thread_aecawb_msg_type_t msg_type,
  int param_type, awb_port_private_t *private);
void awb_port_callback(awb_output_data_t *output, void *p);
void awb_port_stats_done_callback(void* p, void* stats);
void awb_port_handle_output_data(mct_port_t *port, awb_output_data_t  *output);
void awb_port_callback_offline_proc(mct_port_t *port,
  awb_output_data_t *output);
void awb_port_send_event(mct_port_t *port,
  int evt_type,
  int sub_evt_type,
  void *data,
  uint32_t sof_id);
void awb_port_configure_stats(awb_output_data_t *output,
  mct_port_t *port);
void awb_port_send_awb_info_to_metadata(mct_port_t *port,
  awb_output_data_t *output);
void awb_port_update_state(awb_port_private_t *private);
void awb_port_update_wb(awb_port_private_t *private,
  awb_output_data_t *output);

#endif /* __AWB_PORT_H__ */
