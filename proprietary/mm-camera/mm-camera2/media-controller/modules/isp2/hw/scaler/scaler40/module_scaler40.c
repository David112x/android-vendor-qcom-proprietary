/* module_scaler40.c
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
#include "chromatix.h"

/* isp headers */
#include "isp_sub_module_common.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "port_scaler40.h"
#include "module_scaler40.h"
#include "scaler40.h"


#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_SCALER, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_SCALER, fmt, ##args)

/* TODO pass from Android.mk */
#define SCALER40_VERSION "40"

#define SCALER40_VIEWFINDER_NAME(n) \
  "scaler_viewfinder"n

#define SCALER40_ENCODER_NAME(n) \
  "scaler_encoder"n

#define SCALER40_VIEWFINDER_ENABLE_BIT 24
#define SCALER40_ENCODER_ENABLE_BIT    23

/** module_scaler40_set_mod:
 *
 *  @module: module handle
 *  @module_type: module type
 *  @identity: identity of stream
 *
 *  Handle set mod
 *
 *  Returns void
 **/
static void module_scaler40_set_mod(mct_module_t *module,
  unsigned int module_type, unsigned int identity)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }
  mct_module_add_type(module, module_type, identity);
  return;
}

/** module_scaler40_query_mod:
 *
 *  @module: module handle
 *  @query_buf: query caps buffer handle
 *  @sessionid: session id
 *
 *  Handle query mod
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean module_scaler40_query_mod(mct_module_t *module,
  void *query_buf, unsigned int sessionid)
{
  boolean ret = TRUE;

  if (!module || !query_buf) {
    ISP_ERR("failed: %p %p", module, query_buf);
    return FALSE;
  }

  ret = isp_zoom_query_mod_func((mct_pipeline_cap_t *)query_buf);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_zoom_query_mod_func");
  }
  return ret;
}

/** module_scaler40_start_session:
 *
 *  @module: module handle
 *  @sessionid: session id
 *
 *  Handle start session
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean module_scaler40_start_session(mct_module_t *module,
  unsigned int session_id)
{
  boolean       ret = TRUE;
  scaler40_t *scaler = NULL;

  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  ISP_HIGH("module %p", module);
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));
  scaler = MCT_OBJECT_PRIVATE(module);
  if (scaler) {
    ISP_ERR("failed: scaler %p already exists", scaler);
    return FALSE;
  }

  scaler = (scaler40_t *)CAM_MALLOC(sizeof(*scaler));
  if (!scaler) {
    ISP_ERR("failed: scaler %p", scaler);
    return FALSE;
  }
  memset(scaler, 0, sizeof(*scaler));
  pthread_mutex_init(&scaler->mutex, NULL);

  MCT_OBJECT_PRIVATE(module) = (void *)scaler;

  scaler->hw_stream_info.fmt = CAM_FORMAT_MAX;

  /* Initialize hw stream specific params */
  if (!strncmp(MCT_MODULE_NAME(module),
       SCALER40_VIEWFINDER_NAME(SCALER40_VERSION),
       strlen(MCT_MODULE_NAME(module)))) {
    ISP_HIGH("viewfinder");
    scaler->entry_idx = ISP_HW_STREAM_VIEWFINDER;
    scaler->hw_module_id = ISP_MOD_SCALER_VIEWFINDER;
    scaler->reg_off = ISP_SCALER40_VIEW_OFF;
    scaler->reg_len = ISP_SCALER40_VIEW_LEN;
  } else if (!strncmp(MCT_MODULE_NAME(module),
              SCALER40_ENCODER_NAME(SCALER40_VERSION),
              strlen(MCT_MODULE_NAME(module)))) {
    ISP_HIGH("encoder");
    scaler->entry_idx = ISP_HW_STREAM_ENCODER;
    scaler->hw_module_id = ISP_MOD_SCALER_ENCODER;
    scaler->reg_off = ISP_SCALER40_ENC_OFF;
    scaler->reg_len = ISP_SCALER40_ENC_LEN;
  } else {
    ISP_ERR("failed: invalid module name %s", MCT_MODULE_NAME(module));
  }

  ret = isp_zoom_init(&scaler->zoom, session_id, ISP_VERSION_40);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_zoom_init");
  }

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  return ret;

ERROR:
  ISP_ERR("failed: module_scaler40_start_session");
  return ret;
}

/** module_scaler40_stop_session:
 *
 *  @module: module handle
 *  @session_id: session id
 *
 *  Handle stop session
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean module_scaler40_stop_session(mct_module_t *module,
  unsigned int session_id)
{
  boolean       ret = TRUE;
  scaler40_t *scaler = NULL;

  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));

  scaler = MCT_OBJECT_PRIVATE(module);
  if (!scaler) {
    ISP_ERR("failed: scaler %p", scaler);
    return FALSE;
  }
  if (scaler->session_id != session_id) {
    ISP_ERR("failed: invalid session id %d expected %d", session_id,
      scaler->session_id);
  }
  pthread_mutex_destroy(&scaler->mutex);
  free(scaler);
  MCT_OBJECT_PRIVATE(module) = NULL;

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  return TRUE;
}

/** module_scaler40_overwrite_funcs:
 *
 *  @module: module handle
 *
 *  Overwrite functions with scaler 40 specific
 *
 *  Returns nothing
 **/
static void module_scaler40_overwrite_funcs(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }
  mct_module_set_set_mod_func(module, module_scaler40_set_mod);
  mct_module_set_query_mod_func(module, module_scaler40_query_mod);
  mct_module_set_start_session_func(module, module_scaler40_start_session);
  mct_module_set_stop_session_func(module, module_scaler40_stop_session);
}

/** module_scaler40_init:
 *
 *    @name: name of ISP module - "scaler40"
 *
 * Initializes new instance of ISP module
 * create mct module for scaler
 *
 * Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_scaler40_init(const char *name)
{
  boolean       ret = TRUE;
  mct_module_t *module = NULL;
  scaler40_t *scaler = NULL;

  ret = scaler40_util_setloglevel("scaler", ISP_LOG_SCALER);
  if (ret == FALSE) {
    ISP_ERR("failed: fovcrop46_util_setloglevel");
  }
  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %p", name);
    return NULL;
  }

  if (strncmp(name, SCALER40_VIEWFINDER_NAME(SCALER40_VERSION), strlen(name)) &&
      strncmp(name, SCALER40_ENCODER_NAME(SCALER40_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s or %s", name,
      SCALER40_VIEWFINDER_NAME(SCALER40_VERSION),
      SCALER40_ENCODER_NAME(SCALER40_VERSION));
    return NULL;
  }

  /* Create MCT module for scaler40 */
  module = mct_module_create(name);
  if (!module) {
    ISP_ERR("failed: mct_module_create");
    return NULL;
  }

  /* Create ports */
  ret = port_scaler40_create(module);
  if (ret == FALSE) {
    ISP_ERR("failed: port_isp_create");
    goto ERROR;
  }

  /* Overwrite module functions */
  module_scaler40_overwrite_funcs(module);

  ISP_HIGH("module %p", module);
  return module;

ERROR:
  ISP_ERR("failed: module_scaler40_init");
  port_scaler40_delete_ports(module);
  free(scaler);
  mct_module_destroy(module);
  return NULL;
}

/** module_scaler40_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_scaler40_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  /* Delete source and sink ports */
  port_scaler40_delete_ports(module);

  scaler40_destroy(MCT_OBJECT_PRIVATE(module));

  /* Destroy mct module */
  mct_module_destroy(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_scaler40_init,
  .module_deinit = module_scaler40_deinit,
};

isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
