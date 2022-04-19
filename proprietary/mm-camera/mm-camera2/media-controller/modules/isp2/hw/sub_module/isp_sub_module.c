/* isp_sub_module.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <stdio.h>
#include <string.h>

/* mctl headers */
#include "media_controller.h"
#include "mct_list.h"
#include "mct_module.h"
#include "mct_port.h"
#include "chromatix.h"

/* isp headers */
#include "isp_log.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module.h"
#include "isp_sub_module_util.h"

/** isp_sub_module_set_mod:
 *
 *  @module: module handle
 *  @module_type: module type
 *  @identity: identity of stream
 *
 *  Handle set mod
 *
 *  Returns void
 **/
static void isp_sub_module_set_mod(mct_module_t *module,
  unsigned int module_type, unsigned int identity)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }
  mct_module_add_type(module, module_type, identity);
  return;
}

/** isp_sub_module_query_mod:
 *
 *  @module: module handle
 *  @query_buf: query caps buffer handle
 *
 *  @sessionid: session id
 *
 *  Handle query mod
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_sub_module_query_mod(
    mct_module_t *module,
    void         *query_buf,
    uint32_t     session_id __unused)
{
  boolean ret = TRUE;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;
  isp_sub_module_private_func_t *private_func = NULL;
  isp_sub_module_event_func      event_func = NULL;

  if (!module || !query_buf) {
    ISP_ERR("failed: %p %p", module, query_buf);
    return FALSE;
  }

  ISP_DBG("session id %d", sessionid);

  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));
  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    ret = FALSE;
    goto ERROR;
  }

  private_func = isp_sub_module_priv->private_func;

  if (!private_func) {
    ISP_ERR("failed: private_func %p", private_func);
    ret = FALSE;
    goto ERROR;
  }

  if(private_func->isp_sub_module_query_cap) {
      private_func->isp_sub_module_query_cap(module, query_buf);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_sub_module_query_cap");
      }
  }

ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return ret;
}

/** mct_module_request_port_function:
 *
 *  @stream_info: stream info handle
 *  @direction: mct port direction
 *  @module: mct module handle
 *  @peer_caps: peer caps handle
 *
 *  Add new port for request direction and add it to module's
 *  children
 *
 *  Return new port handle
 **/
static mct_port_t *isp_sub_module_request_new_port(
    void                 *stream_info,
    mct_port_direction_t direction,
    mct_module_t         *module,
    void                 *peer_caps __unused)
{
  ISP_HIGH("E");
  if (!stream_info || !module) {
    ISP_ERR("failed: stream_info %p module %p", stream_info, module);
    return NULL;
  }

  if ((direction != MCT_PORT_SRC) && (direction != MCT_PORT_SINK)) {
    ISP_ERR("failed: invalid direction %d", direction);
    return NULL;
  }

  return isp_sub_module_port_request_new_port(module, direction);
}

/** isp_sub_module_overwrite_funcs:
 *
 *  @module: mct module handle
 *
 *  Overwite module functions
 *
 *  Return NONE
 **/
static void isp_sub_module_overwrite_funcs(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  mct_module_set_set_mod_func(module, isp_sub_module_set_mod);
  mct_module_set_query_mod_func(module, isp_sub_module_query_mod);
  mct_module_set_start_session_func(module, NULL);
  mct_module_set_stop_session_func(module, NULL);
//  mct_module_set_request_new_port_func(module,
//    isp_sub_module_request_new_port);
}

/** isp_sub_module_init:
 *
 *  @name: name of ISP module - "demosaic40"
 *  @num_sink_ports: number of sink ports to be created for this
 *                 module
 *  @num_source_ports: number of source ports to be created for
 *                   this module
 *  @private_func: private function handler
 *  @hw_module_id: unique module id for every module
 *
 * Initializes new instance of ISP module
 *
 * create mct module for demosaic
 *
 * Return mct module handle on success or NULL on failure
 **/
mct_module_t *isp_sub_module_init(const char *name, uint32_t num_sink_ports,
  uint32_t num_source_ports, isp_sub_module_private_func_t *private_func,
  isp_hw_module_id_t hw_module_id, char *mod_name, isp_log_sub_modules_t mod_id)
{
  boolean            ret = TRUE;
  mct_module_t      *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ret = isp_sub_module_util_setloglevel(mod_name, mod_id);
  if (!ret) {
    ISP_ERR("failed: isp_sub_module_util_setloglevel");
  }
  ISP_HIGH("name %s", name);

  if (!name || !private_func) {
    ISP_ERR("failed: name %p %p", name, private_func);
    return NULL;
  }

  /* Create MCT module for demosaic40 */
  module = mct_module_create(name);
  if (!module) {
    ISP_ERR("failed: mct_module_create");
    return NULL;
  }

  /* Create ports */
  ret = isp_sub_module_port_create(module, num_sink_ports, num_source_ports);
  if (ret == FALSE) {
    ISP_ERR("failed: port_isp_create");
    goto ERROR1;
  }

  /* Overwrite module functions */
  isp_sub_module_overwrite_funcs(module);

  /* Create module private */
  isp_sub_module_priv =
    (isp_sub_module_priv_t *)CAM_MALLOC(sizeof(*isp_sub_module_priv));
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    goto ERROR2;
  }
  memset(isp_sub_module_priv, 0, sizeof(*isp_sub_module_priv));
  isp_sub_module_priv->num_sink_ports = num_sink_ports;
  isp_sub_module_priv->num_source_ports = num_source_ports;
  isp_sub_module_priv->private_func = private_func;
  isp_sub_module_priv->hw_module_id = hw_module_id;
  isp_sub_module_priv->mod_id_for_log = mod_id;
  strlcpy(isp_sub_module_priv->module_name, mod_name,
          sizeof(isp_sub_module_priv->module_name));
  MCT_OBJECT_PRIVATE(module) = (void *)isp_sub_module_priv;

  return module;

ERROR2:
  isp_sub_module_port_delete_ports(module);
ERROR1:
  mct_module_destroy(module);
  ISP_ERR("failed");
  return NULL;
}

/** isp_sub_module_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
void isp_sub_module_deinit(mct_module_t *module)
{
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  /* Extract module private */
  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
  }

  /* Delete source and sink ports */
  isp_sub_module_port_delete_ports(module);

  /* Destroy mct module */
  mct_module_destroy(module);

  if (isp_sub_module_priv) {
    free(isp_sub_module_priv);
  }
}
