/* is_port.h
 *
 * Copyright (c) 2013-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __IS_PORT_H__
#define __IS_PORT_H__
#include "mct_port.h"
#include "is_thread.h"
#include "stats_util.h"
#ifdef _ANDROID_
#include <cutils/properties.h>

#define IS_PROPERTY_MAX_LEN 96

#define IS_PROP_GET_MARGIN(margin) {\
  char prop[IS_PROPERTY_MAX_LEN];\
  property_get("persist.vendor.camera.is.margin", prop, "0.20");\
  margin = atof(prop);\
}

#define IS_PROP_GET_BUFFER_DELAY(buffer_delay) {\
  char prop[IS_PROPERTY_MAX_LEN];\
  property_get("persist.vendor.camera.is.buffer_delay", prop, "15");\
  buffer_delay = atoi(prop);\
}

#define IS_PROP_GET_NUM_MESH_X(num_mesh_x) {\
  char prop[IS_PROPERTY_MAX_LEN];\
  property_get("persist.vendor.camera.is.num_mesh_x", prop, "1");\
  num_mesh_x = atoi(prop);\
}

#define IS_PROP_GET_NUM_MESH_Y(num_mesh_y) {\
  char prop[IS_PROPERTY_MAX_LEN];\
  property_get("persist.vendor.camera.is.num_mesh_y", prop, "10");\
  num_mesh_y = atoi(prop);\
}
#else
#define IS_PROP_GET_MARGIN(margin) {margin = 0.10;}
#define IS_PROP_GET_BUFFER_DELAY(buffer_delay) {buffer_delay = 15;}
#define IS_PROP_GET_NUM_MESH_X(num_mesh_x) {num_mesh_x = 1;}
#define IS_PROP_GET_NUM_MESH_Y(num_mesh_y) {num_mesh_y = 10;}
#endif


typedef enum {
  IS_PORT_STATE_CREATED = 1,
  IS_PORT_STATE_RESERVED,
  IS_PORT_STATE_LINKED,
  IS_PORT_STATE_UNLINKED,
  IS_PORT_STATE_UNRESERVED
} is_port_state_t;

/** is_dual_cam_info:
 *    @mode:                   Driving camera of the related camera sub-system
 *    @sync_3a_mode:           3A Sync Mode
 *    @cam_role:               Camera Type in Dual Camera Mode
 *    @intra_peer_id:          Intra peer Id
 *    @is_LPM_on:              Is Low Power Mode on
 *    @is_aux_sync_enabled:    Can we update Aux Camera with our peer info
 *    @is_aux_update_enabled:  Can Aux camera update its result to outside world
 *    @is_algo_active:         is Slave Algorithm Active
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
}is_dual_cam_info;

/** _is_port_private_t:
 *    @reserved_id[]: session id + stream id
 *    @state: IS port state
 *    @RSCS_stats_ready: Indicates whether row and column sum stats has arrived
 *      for the current frame (composite stats assumed)
 *    @is_output: DIS/EIS output/results
 *    @is_output_flush: EIS3.0 flush buffer output
 *    @stream_type: Indicates camera or camcorder mode
 *    @is_info: IS-related information
 *    @is_process_output: Output from IS processing
 *    @is_set_output: Output from setting IS parameter
 *    @set_parameters: pointer to function that sets IS parameters
 *    @process: pointer to function that leads to running IS algorithm
 *    @set_callback: callback function into IS port upon setting IS parameter
 *    @process_callback: callback function into IS port upon event processing
 *    @is_stats_cb:
 *    @thread_data: IS thread data
 *    @dual_cam_sensor_info:
 *    @intra_peer_id:
 *
 * This structure represents the IS port's internal variables.
 **/
typedef struct _is_port_private {
  unsigned int reserved_id[IS_MAX_STREAMS];
  is_port_state_t state;
  int RSCS_stats_ready;
  is_output_type is_output[IS_MAX_STREAMS];
  is_output_type is_output_flush[IS_MAX_BUFFER_DELAY];
  cam_stream_type_t stream_type;
  is_info_t is_info;
  is_process_output_t is_process_output;
  is_set_output_t is_set_output;
  /* Hook to extend functionality */
  stats_util_override_func_t func_tbl;
  is_set_parameters_func set_parameters;
  is_process_func process;
  is_set_callback_func set_callback;
  is_process_callback_func process_callback;
  is_stats_done_callback_func is_stats_cb;
  is_thread_data_t *thread_data;
  cam_sync_type_t dual_cam_sensor_info;
  is_dual_cam_info dual_cam_info;
} is_port_private_t;


void is_port_deinit(mct_port_t *port);
boolean is_port_find_identity(mct_port_t *port, unsigned int identity);
boolean is_port_init(mct_port_t *port, unsigned int session_id);
#endif /* __IS_PORT_H__ */
