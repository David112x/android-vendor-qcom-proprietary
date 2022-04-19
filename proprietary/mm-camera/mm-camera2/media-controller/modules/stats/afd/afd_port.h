/* afd_port.h
 *
 * Copyright (c) 2013, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __AFD_PORT_H__
#define __AFD_PORT_H__

#include "mct_port.h"
#include "afd_module.h"
#include "afd_thread.h"

#define AFD_PORT_STATE_CREATED        0x1
#define AFD_PORT_STATE_RESERVED       0x2
#define AFD_PORT_STATE_UNRESERVED     0x3
#define AFD_PORT_STATE_LINKED         0x4
#define AFD_PORT_STATE_UNLINKED       0x5
#define AFD_PORT_STATE_STREAMON       0x6
#define AFD_PORT_STATE_STREAMOFF      0x7
#define AFD_PORT_STATE_AECAWB_RUNNING (AFD_PORT_STATE_STREAMON | (0x00000001 << 8))
#define AFD_PORT_STATE_AF_RUNNING     (AFD_PORT_STATE_STREAMON | (0x00000001 << 9))

/** afd_dual_cam_bus_msg_info:
**/
typedef struct {
  mct_bus_msg_afd_t  afd_msg;
  stats_update_t stats_update;
  boolean        validUpdate;
} afd_dual_cam_bus_msg_info;

/** afd_dual_cam_info:
 *    @mode:                            Driving camera of the related camera sub-system
 *    @sync_3a_mode:              3A Sync Mode
 *    @cam_role:                      Camera Type in Dual Camera Mode
 *    @intra_peer_id:                 Intra peer Id
 *    @is_LPM_on:                     Is Low Power  Mode on
 *    @is_aux_sync_enabled:     Can we update Aux Camera with our peer info
 *    @is_aux_update_enabled:  Can Aux camera update its result to outside world
 *    @is_algo_active:                Is Slave Algorithm Active
 *    @peer_stats_update:          Stats result from Master
**/
typedef struct {
  cam_sync_mode_t           mode;
  cam_3a_sync_mode_t        sync_3a_mode;
  cam_dual_camera_role_t    cam_role;
  uint32_t                  intra_peer_id;
  boolean                   is_LPM_on;
  boolean                   is_aux_sync_enabled;
  boolean                   is_aux_update_enabled;
  boolean                   is_algo_active;
  afd_dual_cam_bus_msg_info peer_stats_update;
}afd_dual_cam_info;

typedef struct {
  stats_type_t         stats_type_mask;
  uint32_t             flicker_stats_frame_id;
  uint32_t             bg_stats_frame_id;
  q3a_bg_stats_t      *p_q3a_bg_stats;
  q3a_rs_stats_t      *p_q3a_flicker_stats;
}hdr_stats_sync_t;

/** afd_port_private_t
 *    @sub_ports: stats module's sub-modules' sink-ports,
 *                used for internal link function
 **/
typedef struct _afd_port_private {
  unsigned int reserved_id;
  unsigned int stream_type;
  unsigned int state;
  uint8_t meta_scene_flicker;
  cam_antibanding_mode_type antibanding_mode;
  afd_module_object_t afd_object;
  afd_thread_data_t   *thread_data;
  afd_dual_cam_info    dual_cam_info;
  sensor_RDI_parser_func_t  parse_RDI_stats;
  boolean is_in_sensor_HDR;
  hdr_stats_sync_t    hdr_stats_sync;
} afd_port_private_t;

void    afd_port_deinit(mct_port_t *port);
boolean afd_port_find_identity(mct_port_t *port, unsigned int identity);
boolean afd_port_init(mct_port_t *port, unsigned int identity);
#endif /* __AFD_PORT_H__ */
