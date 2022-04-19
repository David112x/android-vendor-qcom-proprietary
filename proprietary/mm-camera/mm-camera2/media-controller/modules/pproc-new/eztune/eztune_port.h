/*============================================================================

 Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef EZTUNE_PORT_H
#define EZTUNE_PORT_H

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

boolean eztune_port_src_event_func(mct_port_t *port, mct_event_t *event);

boolean eztune_port_sink_event_func(mct_port_t *port, mct_event_t *event);

#endif
