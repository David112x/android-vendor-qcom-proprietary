/*============================================================================

 Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include "base_module.h"
#include "pp_utils.h"
#include "base_port.h"

static boolean find_port_with_identity_find_func(void * data, void * userdata)
{
  mct_port_t *port = (mct_port_t *)data;
  uint32_t identity = *(uint32_t *)userdata;
  if (base_port_get_stream_data(port, identity) == NULL) {
    return false;
  }
  return true;
}

mct_list_t * base_module_get_ports_for_identity(mct_module_t *module,
                                                uint32_t identity,
                                                mct_port_direction_t dir)
{
  mct_list_t *list = NULL;
  mct_list_t *templist;
  mct_port_t *port = NULL;
  mct_list_t *ports;

  switch (dir) {
    case MCT_PORT_SRC:
      ports = MCT_MODULE_SRCPORTS(module);
      break;
    case MCT_PORT_SINK:
      ports = MCT_MODULE_SINKPORTS(module);
      break;
    default:
      PP_ERR("failed, bad port_direction=%d", dir);
      return NULL;
  }

  /* TODO : Add support for searching multiple ports with same identity */

  if (!ports) {
    PP_ERR("failed");
    return NULL;
  }
  templist = mct_list_find_custom(ports, &identity,
                                  find_port_with_identity_find_func);
  if(templist) {
    port = (mct_port_t*)(templist->data);
    /* append port to a list*/
    list = mct_list_append(list, port, NULL, NULL);
  }

  return list;
}

