/* module_clf44.c
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

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_CLF, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_CLF, fmt, ##args)

/* isp headers */
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "module_clf44.h"
#include "clf44.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

/* TODO pass from Android.mk */
#define CLF44_VERSION "44"

#define CLF44_MODULE_NAME(n) \
  "clf"n

static boolean module_clf44_query_cap(mct_module_t *module,
  void *query_buf);

static isp_sub_module_private_func_t clf44_private_func = {
  .isp_sub_module_init_data              = clf44_init,
  .isp_sub_module_destroy                = clf44_destroy,
  .isp_sub_module_query_cap              = module_clf44_query_cap,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = clf44_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = clf44_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      clf44_set_chromatix_ptr,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
     [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_GET_VFE_DIAG_INFO_USER] = clf44_get_vfe_diag_info_user,
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_ACTION_RESET]           = clf44_reset,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = clf44_trigger_update,
  },

  .set_param_handler = {
    [ISP_SET_PARM_SET_VFE_COMMAND] = isp_sub_module_port_set_vfe_command,
    [ISP_SET_INTF_PARM_CAC]        = isp_sub_module_set_manual_controls,

  },

};


/** module_clf44_query_cap:
 *
 *  @module: mct module handle
 *  @query_buf: query caps buffer handle
 *
 *  Destroy dynamic resources
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean module_clf44_query_cap(mct_module_t *module,
  void *query_buf)
{
  if (!module || !query_buf) {
    ISP_ERR("failed: %p %p", module, query_buf);
    return FALSE;
  }
  return clf44_query_cap(module, query_buf);
}

/** module_clf44_init:
 *
 *  @name: name of ISP module - "clf44"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for clf
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_clf44_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, CLF44_MODULE_NAME(CLF44_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      CLF44_MODULE_NAME(CLF44_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &clf44_private_func, ISP_MOD_CLF, "clf", ISP_LOG_CLF);
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

/** module_clf44_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_clf44_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_clf44_init,
  .module_deinit = module_clf44_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
