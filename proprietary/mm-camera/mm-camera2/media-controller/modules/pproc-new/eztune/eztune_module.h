/*============================================================================

 Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef EZTUNE_MODULE_H
#define EZTUNE_MODULE_H

#include <stdbool.h>
#include <stdint.h>
#include "modules.h"
#include "mct_list.h"
#include "media_controller.h"
#include "mct_port.h"
#include "mct_object.h"
#include "cam_types.h"
#include "cam_intf.h"
#include "mct_module.h"
#include "mct_pipeline.h"
#include "mct_stream.h"

#if defined(__cplusplus)
extern "C" {
#endif

mct_module_t* eztune_module_init(const char *name);

void eztune_module_deinit(mct_module_t *mod);

mct_pipeline_t *eztune_get_pipeline();

mct_stream_t *eztune_get_stream_by_id(uint32_t stream_id);

mct_stream_t *eztune_get_stream_by_type(cam_stream_type_t stream_id);

mct_port_t *eztune_get_port_by_type(cam_stream_type_t stream_type);

boolean eztune_post_msg_to_bus(mct_bus_msg_type_t type, int32_t size, void *msg);

boolean eztune_send_event(mct_event_module_type_t type, uint32_t identity, void *data);

#if defined(__cplusplus)
}
#endif

#endif
