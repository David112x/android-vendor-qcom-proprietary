/* module_template.c
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <stdio.h>

/* mctl headers */
#include "media_controller.h"
#include "mct_list.h"
#include "mct_module.h"
#include "mct_port.h"
#include "chromatix.h"

/* isp headers */
#include "isp_sub_module_common.h"
#include "isp_common.h"
#include "isp_log.h"
#include "port_template.h"
#include "module_template.h"
#include "template_util.h"

/* TODO pass from Android.mk */
#define TEMPLATE40_VERSION "40"

#define TEMPLATE40_MODULE_NAME(n) \
  "template_"n

/** module_template_set_mod:
 *
 *  @module: module handle
 *  @module_type: module type
 *  @identity: identity of stream
 *
 *  Handle set mod
 *
 *  Returns void
 **/
static void module_template_set_mod(mct_module_t *module,
  unsigned int module_type, unsigned int identity)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }
  mct_module_add_type(module, module_type, identity);
  return;
}

/** module_template_query_mod:
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
static boolean module_template_query_mod(mct_module_t *module,
  void *query_buf, unsigned int sessionid)
{
  return TRUE;
}

/** module_template_start_session:
 *
 *  @module: module handle
 *  @sessionid: session id
 *
 *  Handle start session
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean module_template_start_session(mct_module_t *module,
  unsigned int session_id)
{
  boolean            ret = TRUE;
  template_priv_t *template_priv = NULL;
  template_t      *template = NULL;

  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  ISP_HIGH("session id %d", session_id);
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));
  template_priv = (template_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!template_priv) {
    ISP_ERR("failed: template_priv %p", template_priv);
    goto ERROR;
  }

  ret = template_util_create_session_param(template_priv, session_id,
    &template);
  if ((ret == FALSE) || !template) {
    ISP_ERR("failed: template_util_create_session_param %d %p", ret,
      template);
    goto ERROR;
  }

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  ISP_DBG("template_priv %p", template_priv);
  return ret;

ERROR:
  ISP_ERR("failed");
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return ret;
}

/** module_template_stop_session:
 *
 *  @module: module handle
 *  @session_id: session id
 *
 *  Handle stop session
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean module_template_stop_session(mct_module_t *module,
  unsigned int session_id)
{
  boolean            ret = TRUE;
  template_priv_t *template_priv = NULL;

  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  ISP_HIGH("session id %d", session_id);
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));

  template_priv = (template_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!template_priv) {
    ISP_ERR("failed: template_priv %p", template_priv);
    goto ERROR;
  }

  ret = template_util_remove_session_param(template_priv, session_id);
  if (ret == FALSE) {
    ISP_ERR("failed: template_util_remove_session_param");
    goto ERROR;
  }
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return FALSE;
}

/** module_template_overwrite_funcs:
 *
 *  @module: mct module handle
 *
 *  Overwite module functions
 *
 *  Return NONE
 **/
static void module_template_overwrite_funcs(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }
  mct_module_set_set_mod_func(module, module_template_set_mod);
  mct_module_set_query_mod_func(module, module_template_query_mod);
  mct_module_set_start_session_func(module, module_template_start_session);
  mct_module_set_stop_session_func(module, module_template_stop_session);
}

/** module_template_init:
 *
 *  @name: name of ISP module - "template"
 *
 * Initializes new instance of ISP module
 *
 * create mct module for template
 *
 * Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_template_init(const char *name)
{
  boolean            ret = TRUE;
  mct_module_t      *module = NULL;
  template_priv_t *template_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %p", name);
    return NULL;
  }

  if (strncmp(name, TEMPLATE40_MODULE_NAME(TEMPLATE40_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      TEMPLATE40_MODULE_NAME(TEMPLATE40_VERSION));
    return NULL;
  }

  /* Create MCT module for template */
  module = mct_module_create(name);
  if (!module) {
    ISP_ERR("failed: mct_module_create");
    return NULL;
  }

  /* Create ports */
  ret = port_template_create(module);
  if (ret == FALSE) {
    ISP_ERR("failed: port_isp_create");
    goto ERROR1;
  }

  /* Overwrite module functions */
  module_template_overwrite_funcs(module);

  /* Create module private */
  template_priv = (template_priv_t *)CAM_MALLOC(sizeof(*template_priv));
  if (!template_priv) {
    ISP_ERR("failed: template_priv %p", template_priv);
    goto ERROR2;
  }
  memset(template_priv, 0, sizeof(*template_priv));
  MCT_OBJECT_PRIVATE(module) = (void *)template_priv;

  return module;

ERROR2:
  port_template_delete_ports(module);
ERROR1:
  mct_module_destroy(module);
  ISP_ERR("failed");
  return NULL;
}

/** module_template_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_template_deinit(mct_module_t *module)
{
  template_priv_t *template_priv = NULL;

  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  /* Extract module private */
  template_priv = (template_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!template_priv) {
    ISP_ERR("failed: template_priv %p", template_priv);
  }

  /* Delete source and sink ports */
  port_template_delete_ports(module);

  /* Destroy mct module */
  mct_module_destroy(module);

  if (template_priv) {
    free(template_priv);
  }
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_template_init,
  .module_deinit = module_template_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
