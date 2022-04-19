/* isp_port.h
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __ISP_PORT_H__
#define __ISP_PORT_H__

/* isp headers */
#include "isp_pipeline.h"

/** isp_port_t:
 *
 *  @is_reserved: is reserved for a session /stream
 *  @session_id: store session id
 *  @num_streams: number of streams currently using this port
 *  @hw_stream: For src port, the hw stream that will link
 *  @sink_data: sink private data
 *  @source_data: source private data
 *  @session_based: boolean flag to indicate session based
 *                stream mapped to this port
 **/
typedef struct {
  boolean                is_reserved;
  uint32_t               session_id;
  uint32_t               num_streams;
  isp_hw_streamid_t      hw_stream;
  isp_hw_id_t            hw_id;
  boolean                is_session_based;
} isp_port_data_t;

boolean isp_port_create(mct_module_t *module);

void isp_port_delete_ports(mct_module_t *module);

#endif
