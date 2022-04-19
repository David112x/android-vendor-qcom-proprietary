/* port_fovcrop46.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __PORT_fovcrop46_H__
#define __PORT_fovcrop46_H__

/* std headers */
#include "pthread.h"
#include "module_fovcrop46.h"

/** port_fovcrop46_data_t:
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
} port_fovcrop46_data_t;

boolean port_fovcrop46_create(mct_module_t *module);

void port_fovcrop46_delete_ports(mct_module_t *module);

boolean port_fovcrop46_handle_hal_set_parm(mct_module_t *module,
  fovcrop46_t *fovcrop, uint32_t identity, mct_event_control_parm_t *hal_param);

boolean port_fovcrop46_handle_cds_request(mct_module_t *module,
  fovcrop46_t *fovcrop, mct_event_t *event);

boolean port_fovcrop46_handle_lens_position_update(mct_module_t* module,
  fovcrop46_t *fovcrop, uint32_t identity , mct_event_t *event);

#endif
