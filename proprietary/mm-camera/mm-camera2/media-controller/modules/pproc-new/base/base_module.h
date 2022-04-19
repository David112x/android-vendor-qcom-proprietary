/*============================================================================

 Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef BASE_MODULE_H
#define BASE_MODULE_H

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

typedef struct {
  int32_t num_sink_ports;
  int32_t num_src_ports;
  mct_module_t *parent_mod;
} base_module_ctrl_t;

mct_module_t* base_module_init(const char *name);

void base_module_deinit(mct_module_t *mod);

void base_module_set_parent(mct_module_t *p_mct_mod,
                            mct_module_t *p_parent);

mct_list_t * base_module_get_ports_for_identity(mct_module_t *module,
                                                uint32_t identity,
                                                mct_port_direction_t dir);

int32_t base_module_process_downstream_event(mct_module_t *module,
                                             mct_port_t *port,
                                             mct_event_t *event);

int32_t base_module_process_upstream_event(mct_module_t *module,
                                           mct_port_t *port,
                                           mct_event_t *event);

#endif
