/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef CPP_THREAD_H
#define CPP_THREAD_H

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
#include "camera_dbg.h"
#include "cpp_hardware.h"

typedef enum {
  CPP_THREAD_MSG_NEW_EVENT_IN_Q,
  CPP_THREAD_MSG_ABORT
} cpp_thread_msg_type_t;

typedef struct _cpp_thread_msg_t {
  cpp_thread_msg_type_t type;
  void *data;
} cpp_thread_msg_t;

int32_t cpp_thread_create(mct_module_t *module);
#endif
