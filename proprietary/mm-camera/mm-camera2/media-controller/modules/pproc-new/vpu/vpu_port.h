/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef VPU_PORT_H
#define VPU_PORT_H

#include "modules.h"
#include "mct_queue.h"
#include "mct_list.h"
#include "media_controller.h"
#include "mct_port.h"
#include "mct_object.h"
#include "cam_types.h"
#include "mct_module.h"
#include "mct_pipeline.h"
#include "mct_stream.h"

#define VPU_MAX_STREAMS_PER_PORT  2
#define VPU_MAX_DIVERT_INFO_PER_PORT  4

typedef enum {
  VPU_PORT_STATE_UNRESERVED,
  VPU_PORT_STATE_RESERVED,
  VPU_PORT_STATE_LINKED,
  VPU_PORT_STATE_STREAMING
} vpu_port_state_t;

typedef enum {
  VPU_PORT_TYPE_INVALID,
  VPU_PORT_TYPE_STREAMING,
  VPU_PORT_TYPE_BURST,
  VPU_PORT_TYPE_SESSION,
} vpu_port_type_t;

typedef struct _vpu_port_stream_data_t {
  vpu_port_state_t port_state;
  uint32_t identity;
  cam_streaming_mode_t streaming_mode;
} vpu_port_stream_data_t;

typedef struct _vpu_divert_info_t {
  uint32_t identity[VPU_MAX_STREAMS_PER_PORT];
  pproc_divert_info_t config[VPU_MAX_DIVERT_INFO_PER_PORT];
} vpu_divert_info_t;

typedef struct _vpu_port_data_t {
  vpu_port_type_t port_type;
  vpu_port_stream_data_t stream_data[VPU_MAX_STREAMS_PER_PORT];
  int32_t num_streams;
  uint32_t session_id;
  vpu_divert_info_t vpu_divert_info;
  boolean has_4k_stream;
  mct_port_t *int_link_port;
} vpu_port_data_t;

mct_port_t *vpu_port_create(const char* name, mct_port_direction_t dir);

static boolean vpu_port_check_caps_reserve(mct_port_t *port, void *peer_caps,
  void *info);
static boolean vpu_port_check_caps_unreserve(mct_port_t *port,
  uint32_t identity);
static boolean vpu_port_ext_link_func(uint32_t identity,
  mct_port_t* port, mct_port_t *peer);
static void vpu_port_ext_unlink_func(uint32_t identity,
  mct_port_t *port, mct_port_t *peer);
static boolean vpu_port_event_func(mct_port_t *port, mct_event_t *event);

int32_t vpu_port_get_linked_identity(mct_port_t *port, uint32_t identity,
  uint32_t *linked_identity);
#endif
