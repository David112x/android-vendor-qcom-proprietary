/*============================================================================

 Copyright (c) 2014, 2016 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/

#ifndef BASE_PORT_H
#define BASE_PORT_H

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

#define BASE_PORT_MAX_INT_LINKS  8
#define BASE_PORT_MAX_STREAMS    4

#define PTHREAD_MUTEX_LOCK(m)   pthread_mutex_lock(m)
#define PTHREAD_MUTEX_UNLOCK(m) pthread_mutex_unlock(m)

typedef enum {
  BASE_PORT_STATE_UNRESERVED,
  BASE_PORT_STATE_RESERVED,
  BASE_PORT_STATE_LINKED,
} base_port_state_t;

typedef struct {
  base_port_state_t state;
  mct_stream_info_t *stream_info;
  mct_list_t *int_links;
  int32_t num_int_links;
  bool streamon;
} base_port_stream_data_t;

typedef struct {
  int32_t frameid;
  int32_t buf_idx;
  uint32_t identity;
  int32_t refcount;
  bool is_buf_dirty;
  uint32_t buffer_access;
} ack_list_entry_t;

typedef struct {
  mct_list_t *list;
  int32_t size;
  pthread_mutex_t list_mutex;
} base_port_ack_list_t;

typedef struct {
  base_port_stream_data_t stream_data[BASE_PORT_MAX_STREAMS];
  base_port_ack_list_t ack_list;
  int32_t stream_count;
} base_port_data_t;

typedef struct {
  mct_event_t *event;
  int32_t ack_count;
} base_port_mct_event_data_t;


mct_port_t *base_port_create(const char* name, mct_port_direction_t dir);

base_port_stream_data_t *base_port_get_stream_data(mct_port_t *port,
                                                uint32_t identity);

int32_t base_port_fwd_event_to_intlinks(mct_port_t *port, mct_event_t *event);

int32_t base_port_handle_buf_divert(mct_port_t *port,
                                    mct_event_t *event);

int32_t base_port_handle_buf_divert_ack(mct_port_t *port,
                                        mct_event_t *event);

#endif
