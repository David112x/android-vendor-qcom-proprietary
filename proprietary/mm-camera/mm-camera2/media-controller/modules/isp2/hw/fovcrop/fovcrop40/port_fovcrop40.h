/* port_fovcrop40.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __PORT_fovcrop40_H__
#define __PORT_fovcrop40_H__

/* std headers */
#include "pthread.h"
#include "module_fovcrop40.h"

/** port_fovcrop40_data_t:
 *
 *  @mutex: mutex lock
 *
 *  @num_streams: number of streams using this port
 *
 *  @int_peer_port: internal peer port
 **/
typedef struct {
  pthread_mutex_t mutex;
  boolean         is_reserved;
  uint32_t        session_id;
  uint32_t        num_streams;
  mct_port_t     *int_peer_port;
} port_fovcrop40_data_t;

boolean port_fovcrop40_create(mct_module_t *module);

void port_fovcrop40_delete_ports(mct_module_t *module);

boolean port_fovcrop40_handle_hal_set_parm(mct_module_t *module,
  fovcrop40_t *fovcrop, uint32_t identity, mct_event_control_parm_t *hal_param);

boolean port_fovcrop40_handle_cds_request(mct_module_t *module,
  fovcrop40_t *fovcrop, mct_event_t *event);

#endif
