/*============================================================================

 Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "modules.h"
#include "pp_log.h"
#include "eztune.h"
#include "base_module.h"
#include "eztune_module.h"
#include "eztune_port.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "mm-camera-eztune"

// declare these here so they are not public outside of this library
extern void eztune_server_config (mct_module_t *module);
extern void eztune_server_stop(mct_module_t *module);

static boolean eztune_port_override(
  void *data,
  void *usr_data __unused)
{
  mct_port_t *port = (mct_port_t *)data;

  port->event_func = (port->direction == MCT_PORT_SRC) ?
    eztune_port_src_event_func : eztune_port_sink_event_func;

  return true;
}

static boolean eztune_module_start_session(
  mct_module_t *module,
  uint32_t sessionid __unused)
{
  PP_HIGH("E, session=%d", sessionid);
  eztune_server_config(module);
  return true;
}

static boolean eztune_module_stop_session(
  mct_module_t *module,
  uint32_t sessionid __unused)
{
  PP_HIGH("E, session=%d", sessionid);
  eztune_server_stop(module);
  return true;
}

mct_module_t* eztune_module_init(const char *name)
{
  boolean rv;
  int32_t i;
  mct_module_t *module;

  module = base_module_init(name);
  if(module == NULL) {
    PP_ERR("base_module_init failed");
    return NULL;
  }

  // override module functions
  module->start_session = eztune_module_start_session;
  module->stop_session = eztune_module_stop_session;

  // override specific functionality
  rv = mct_list_traverse(module->srcports, eztune_port_override, NULL);
  if (!rv) {
    PP_ERR("eztune_port_override_src failed");
    eztune_module_deinit(module);
    return NULL;
  }
  rv = mct_list_traverse(module->sinkports, eztune_port_override, NULL);
  if (!rv) {
    PP_ERR("eztune_port_override_sink failed");
    eztune_module_deinit(module);
    return NULL;
  }

  return module;
}

void eztune_module_deinit(mct_module_t *module)
{
  base_module_deinit(module);
}
