/* port_template40.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __PORT_DEMOSAIC40_H__
#define __PORT_DEMOSAIC40_H__

/* std headers */
#include "pthread.h"

/** port_template_data_t:
 *
 *  @mutex: mutex lock
 *  @is_reserved: is reserved flag
 *  @session_id: session id
 *  @num_streams: number of streams
 *  @int_peer_port: handle to store int peer port
 **/
typedef struct {
  pthread_mutex_t mutex;
  boolean         is_reserved;
  uint32_t        session_id;
  uint32_t        num_streams;
  mct_port_t     *int_peer_port;
} port_template_data_t;

boolean port_template_create(mct_module_t *module);

void port_template_delete_ports(mct_module_t *module);

#endif
