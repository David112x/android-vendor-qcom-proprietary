/*============================================================================

 Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "base_module.h"
#include "base_port.h"
#include "modules.h"
#include "pp_log.h"

#define BASE_MODULE_NUM_DEFAULT_PORTS 16

static int32_t base_module_add_port(mct_module_t *module,
                                    mct_port_direction_t dir)
{
  mct_port_t *port;
  char name[64];
  base_module_ctrl_t *ctrl = (base_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);

  if (dir == MCT_PORT_SRC) {
    snprintf(name, 64, "%s_src_%d", MCT_OBJECT_NAME(module),
             ctrl->num_src_ports);
  } else {
    snprintf(name, 64, "%s_sink_%d", MCT_OBJECT_NAME(module),
             ctrl->num_sink_ports);
  }

  port = base_port_create(name, dir);
  if(!port) {
    PP_ERR("failed.");
    return -1;
  }

  if (dir == MCT_PORT_SRC) {
    module->srcports = mct_list_append(module->srcports, port, NULL, NULL);
    ctrl->num_src_ports++;
  } else {
    module->sinkports = mct_list_append(module->sinkports, port, NULL, NULL);
    ctrl->num_sink_ports++;
  }
  MCT_PORT_PARENT(port) = mct_list_append(MCT_PORT_PARENT(port), module,
                                          NULL, NULL);
  return 0;
}

static void base_module_set_mod(mct_module_t *module,
                                mct_module_type_t module_type,
                                uint32_t identity)
{
  PP_DBG("module_type=%d", module_type);
  if(!module) {
    PP_ERR("failed");
    return;
  }
  if (mct_module_find_type(module, identity) != MCT_MODULE_FLAG_INVALID) {
    mct_module_remove_type(module, identity);
  }
  mct_module_add_type(module, module_type, identity);
  return;
}

static boolean base_module_query_mod(mct_module_t *module,
                                     void *buf,
                                     uint32_t sessionid __unused)
{
  int rc;
  if(!module || !buf) {
    PP_ERR("failed, module=%p, query_buf=%p", module, buf);
    return false;
  }
  mct_pipeline_cap_t *query_buf = (mct_pipeline_cap_t *)buf;
  mct_pipeline_pp_cap_t *pp_cap = &(query_buf->pp_cap);

  /* todo: add hook for query cap */

  return true;
}

static boolean base_module_set_session_data(mct_module_t *module __unused,
                                            void *set_buf __unused,
                                            unsigned int sessionid __unused)
{
  return true;
}

static boolean base_module_start_session(mct_module_t *module __unused,
                                         uint32_t sessionid __unused)
{
  PP_HIGH("E, session=%d", sessionid);
  return true;
}

static boolean base_module_stop_session(mct_module_t *module __unused,
                                        uint32_t sessionid __unused)
{
  PP_HIGH("E, session=%d", sessionid);
  return true;
}

int32_t base_module_process_downstream_event(mct_module_t *module __unused,
                                             mct_port_t *port,
                                             mct_event_t *event)
{
  return base_port_fwd_event_to_intlinks(port, event);
}

int32_t base_module_process_upstream_event(mct_module_t *module __unused,
                                           mct_port_t *port,
                                           mct_event_t *event)
{
  return base_port_fwd_event_to_intlinks(port, event);
}

base_module_ctrl_t* base_module_create_ctrl()
{
  base_module_ctrl_t *ctrl = (base_module_ctrl_t *)
    CAM_MALLOC(sizeof(base_module_ctrl_t));
  if (!ctrl) {
    PP_ERR("failed");
    return NULL;
  }

  memset(ctrl, 0x00, sizeof(base_module_ctrl_t));

  return ctrl;
}

void base_module_set_parent(mct_module_t *module,
                            mct_module_t *parent)
{
  base_module_ctrl_t *mod_base = NULL;

  mod_base = (base_module_ctrl_t*)MCT_OBJECT_PRIVATE(module);
  mod_base->parent_mod = parent;
}

mct_module_t* base_module_init(const char *name)
{
  mct_module_t *module;
  PP_HIGH("name=%s", name);
  module = mct_module_create(name);
  if(!module) {
    PP_ERR("failed");
    return NULL;
  }

  module->set_mod = base_module_set_mod;
  module->query_mod = base_module_query_mod;
  module->start_session = base_module_start_session;
  module->stop_session = base_module_stop_session;
  module->set_session_data = base_module_set_session_data;

  base_module_ctrl_t *ctrl = base_module_create_ctrl();
  if (!ctrl) {
    PP_ERR("failed");
    goto cleanup;
  }
  memset(ctrl, 0x00, sizeof(base_module_ctrl_t));
  MCT_OBJECT_PRIVATE(module) = ctrl;

  int32_t i;

  for (i=0; i<BASE_MODULE_NUM_DEFAULT_PORTS; i++) {
    base_module_add_port(module, MCT_PORT_SRC);
    base_module_add_port(module, MCT_PORT_SINK);
  }
  return module;
cleanup:
  mct_module_destroy(module);
  return NULL;
}

void base_module_deinit(mct_module_t *module)
{
  base_module_ctrl_t *ctrl = (base_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  free(ctrl);
  mct_module_destroy(module);
  return;
}
