/* module_clf46.c
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
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

#include "eztune_vfe_diagnostics.h"

#undef ISP_HIGH
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_CLF, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_CLF, fmt, ##args)

/* isp headers */
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "module_clf46.h"
#include "clf46.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

#define CLF_ENABLE_BIT      12

/* TODO pass from Android.mk */
#define CLF46_VERSION "46"

#define CLF46_MODULE_NAME(n) \
  "clf"n

static boolean module_clf46_query_cap(mct_module_t *module,
  void *query_buf);

static isp_sub_module_private_func_t clf46_private_func = {
  .isp_sub_module_init_data              = clf46_init,
  .isp_sub_module_destroy                = clf46_destroy,
  .isp_sub_module_query_cap              = module_clf46_query_cap,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = clf46_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = clf46_streamoff,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      clf46_set_chromatix_ptr,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE]      = isp_set_hdr_mode,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_GET_VFE_DIAG_INFO_USER] = clf46_get_vfe_diag_info_user,
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_ACTION_RESET]           = clf46_reset,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = clf46_trigger_update,
  },
  .set_param_handler = {
    [ISP_SET_PARM_UPDATE_DEBUG_LEVEL]   = isp_sub_module_port_set_log_level,
  },
};

/** module_clf46_query_cap:
 *
 *  @module: mct module handle
 *  @query_buf: query caps buffer handle
 *
 *  Destroy dynamic resources
 *
 *  Return TRUE on success and FALSE on failure
 **/

static boolean module_clf46_query_cap(mct_module_t *module,
  void *query_buf)
{
  if (!module || !query_buf) {
    ISP_ERR("failed: %p %p", module, query_buf);
    return FALSE;
  }
  return clf46_query_cap(module, query_buf);
}



/** module_clf46_init:
 *
 *  @name: name of ISP module - "clf46"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for clf
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_clf46_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, CLF46_MODULE_NAME(CLF46_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      CLF46_MODULE_NAME(CLF46_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &clf46_private_func, ISP_MOD_CLF, "clf", ISP_LOG_CLF);
  if (!module || !MCT_OBJECT_PRIVATE(module)) {
    ISP_ERR("module %p", module);
    goto ERROR1;
  }

  return module;

ERROR1:
  if (module) {
    mct_module_destroy(module);
  }
  ISP_ERR("failed");
  return NULL;
}

/** module_clf46_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_clf46_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_clf46_init,
  .module_deinit = module_clf46_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
