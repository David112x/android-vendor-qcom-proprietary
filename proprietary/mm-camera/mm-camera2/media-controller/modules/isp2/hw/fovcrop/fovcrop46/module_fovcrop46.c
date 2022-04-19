/* module_fovcrop46.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <stdio.h>

/* mctl headers */
#include "media_controller.h"
#include "mct_list.h"
#include "mct_module.h"
#include "mct_port.h"
//#include "chromatix.h"

/* isp headers */
#include "isp_common.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module_log.h"
#include "port_fovcrop46.h"
#include "module_fovcrop46.h"
#include "fovcrop46.h"
#include "fovcrop46_util.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_FOVCROP, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_FOVCROP, fmt, ##args)

/* Pass this from Android.mk */
#define FOVCROP46_VERSION "46"

#define FOVCROP46_VIEWFINDER_NAME(n) \
  "fovcrop_viewfinder"n

#define FOVCROP46_ENCODER_NAME(n) \
  "fovcrop_encoder"n

#define FOVCROP46_VIDEO_NAME(n) \
  "fovcrop_video"n

/** module_fovcrop46_set_mod:
 *
 *  @module: module handle
 *
 *  @module_type: module type
 *
 *  @identity: identity of stream
 *
 *  Handle set mod
 *
 *  Returns void
 **/
static void module_fovcrop46_set_mod(mct_module_t *module,
  unsigned int module_type, unsigned int identity)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }
  mct_module_add_type(module, module_type, identity);
  return;
}

/** module_fovcrop46_query_mod:
 *
 *  @module: module handle
 *
 *  @query_buf: query caps buffer handle
 *
 *  @sessionid: session id
 *
 *  Handle query mod
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean module_fovcrop46_query_mod(mct_module_t *module,
  void *query_buf, unsigned int sessionid)
{
  return TRUE;
}

/** module_fovcrop46_start_session:
 *
 *  @module: module handle
 *
 *  @sessionid: session id
 *
 *  Handle start session
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean module_fovcrop46_start_session(mct_module_t *module,
  unsigned int session_id)
{
  boolean       ret = TRUE;
  fovcrop46_t *fovcrop = NULL;

  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));
  fovcrop = MCT_OBJECT_PRIVATE(module);

  /* Initialize hw stream specific params */
  if (!strncmp(MCT_MODULE_NAME(module),
       FOVCROP46_VIEWFINDER_NAME(FOVCROP46_VERSION),
       strlen(MCT_MODULE_NAME(module)))) {
    ISP_HIGH("viewfinder");
    fovcrop->entry_idx = ISP_HW_STREAM_VIEWFINDER;
    fovcrop->hw_module_id = ISP_MOD_FOV_VIEWFINDER;
  } else if (!strncmp(MCT_MODULE_NAME(module),
              FOVCROP46_ENCODER_NAME(FOVCROP46_VERSION),
              strlen(MCT_MODULE_NAME(module)))) {
    ISP_HIGH("encoder");
    fovcrop->entry_idx = ISP_HW_STREAM_ENCODER;
    fovcrop->hw_module_id = ISP_MOD_FOV_ENCODER;
  } else if (!strncmp(MCT_MODULE_NAME(module),
              FOVCROP46_VIDEO_NAME(FOVCROP46_VERSION),
              strlen(MCT_MODULE_NAME(module)))) {
    ISP_HIGH("video");
    fovcrop->entry_idx = ISP_HW_STREAM_VIDEO;
    fovcrop->hw_module_id = ISP_MOD_FOV_VIDEO;
  } else {
    ISP_ERR("failed: invalid module name %s", MCT_MODULE_NAME(module));
  }

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  return ret;

ERROR:
  ISP_ERR("failed: module_fovcrop46_start_session");
  return ret;
}

/** module_fovcrop46_stop_session:
 *
 *  @module: module handle
 *
 *  @session_id: session id
 *
 *  Handle stop session
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean module_fovcrop46_stop_session(mct_module_t *module,
  unsigned int session_id)
{
  boolean       ret = TRUE;
  fovcrop46_t *fovcrop = NULL;

  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));



  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  return TRUE;
}

static void module_fovcrop46_overwrite_funcs(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }
  mct_module_set_set_mod_func(module, module_fovcrop46_set_mod);
  mct_module_set_query_mod_func(module, module_fovcrop46_query_mod);
  mct_module_set_start_session_func(module, module_fovcrop46_start_session);
  mct_module_set_stop_session_func(module, module_fovcrop46_stop_session);
}

/** module_fovcrop46_init:
 *    @name: name of ISP module - "fovcrop46"
 *
 * Initializes new instance of ISP module
 *
 * create mct module for fovcrop46
 *
 * Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_fovcrop46_init(const char *name)
{
  boolean       ret = TRUE;
  mct_module_t *module = NULL;
  fovcrop46_t *fovcrop = NULL;

  ret = fovcrop46_util_setloglevel("fovcrop", ISP_LOG_FOVCROP);
  if (ret == FALSE) {
    ISP_ERR("failed: fovcrop46_util_setloglevel");
  }
  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %p", name);
    return NULL;
  }

  if (strncmp(name, FOVCROP46_VIEWFINDER_NAME(FOVCROP46_VERSION),strlen(name)) &&
      strncmp(name, FOVCROP46_ENCODER_NAME(FOVCROP46_VERSION), strlen(name)) &&
      strncmp(name, FOVCROP46_VIDEO_NAME(FOVCROP46_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s or %s or %s", name,
      FOVCROP46_VIEWFINDER_NAME(FOVCROP46_VERSION),
      FOVCROP46_ENCODER_NAME(FOVCROP46_VERSION),
      FOVCROP46_VIDEO_NAME(FOVCROP46_VERSION));
    return NULL;
  }

  /* Create MCT module for fovcrop46 */
  module = mct_module_create(name);
  if (!module) {
    ISP_ERR("failed: mct_module_create");
    return NULL;
  }

  /* Create ports */
  ret = port_fovcrop46_create(module);
  if (ret == FALSE) {
    ISP_ERR("failed: port_isp_create");
    goto ERROR;
  }

  /* Overwrite module functions */
  module_fovcrop46_overwrite_funcs(module);

  if (fovcrop) {
    ISP_ERR("failed: fovcrop46 %p already exists", fovcrop);
    return FALSE;
  }

  fovcrop = (fovcrop46_t *)CAM_MALLOC(sizeof(*fovcrop));
  if (!fovcrop) {
    ISP_ERR("failed: fovcrop46 %p", fovcrop);
    return FALSE;
  }
  memset(fovcrop, 0, sizeof(*fovcrop));
  pthread_mutex_init(&fovcrop->mutex, NULL);

  MCT_OBJECT_PRIVATE(module) = (void *)fovcrop;

  return module;

ERROR:
  ISP_ERR("failed: module_fovcrop46_init");
  port_fovcrop46_delete_ports(module);
  free(fovcrop);
  mct_module_destroy(module);
  return NULL;
}

/** module_fovcrop46_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_fovcrop46_deinit(mct_module_t *module)
{
  fovcrop46_t *fovcrop = NULL;

  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  fovcrop = MCT_OBJECT_PRIVATE(module);
  if (!fovcrop) {
    ISP_ERR("failed: fovcrop46 %p", fovcrop);
    return;
  }

  pthread_mutex_destroy(&fovcrop->mutex);
  free(fovcrop);
  MCT_OBJECT_PRIVATE(module) = NULL;

  /* Delete source and sink ports */
  port_fovcrop46_delete_ports(module);

  //fovcrop46_destroy(fovcrop46);

  /* Destroy mct module */
  mct_module_destroy(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_fovcrop46_init,
  .module_deinit = module_fovcrop46_deinit,
};

isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
