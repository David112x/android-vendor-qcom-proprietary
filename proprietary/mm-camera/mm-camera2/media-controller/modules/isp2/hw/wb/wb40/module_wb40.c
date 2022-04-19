/* module_wb40.c
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
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "wb40.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_WB, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_WB, fmt, ##args)

/* TODO pass from Android.mk */
#define WB40_VERSION "40"

#define WB40_MODULE_NAME(n) \
  "wb"n

static boolean module_wb40_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static isp_sub_module_private_func_t wb40_private_func = {
  .isp_sub_module_init_data              = wb40_init,
  .isp_sub_module_destroy                = wb40_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = wb40_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = wb40_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_STREAM_CONFIG] =
      module_wb40_set_stream_config,
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      wb40_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AWB_UPDATE]  = wb40_stats_awb_update,
    [ISP_MODULE_EVENT_MANUAL_AWB_UPDATE] = wb40_stats_awb_update,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE] = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE] = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE] = wb40_trigger_update,
  },

  .set_param_handler = {
    [ISP_SET_PARM_SET_VFE_COMMAND]       = isp_sub_module_port_set_vfe_command,
    [ISP_SET_META_COLOR_CORRECT_MODE]    = isp_sub_module_set_manual_controls,
    [ISP_SET_META_MODE]                  = isp_sub_module_set_manual_controls,
    [ISP_SET_PARM_WHITE_BALANCE]         = isp_sub_module_set_manual_controls,
    [ISP_SET_META_COLOR_CORRECT_GAINS]   = isp_sub_module_set_manual_controls,
  },
};


/** module_wb40_set_stream_config:
 *
 * @mod: wb module
 * @isp_sub_module: handle to isp_sub_module_t
 * @event: event to be handled
 *
 * Handle stats config update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_wb40_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return wb40_set_stream_config(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_wb40_init:
 *
 *  @name: name of ISP module - "wb40"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for wb
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_wb40_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, WB40_MODULE_NAME(WB40_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      WB40_MODULE_NAME(WB40_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &wb40_private_func, ISP_MOD_WB, "wb", ISP_LOG_WB);
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

/** module_wb40_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_wb40_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_wb40_init,
  .module_deinit = module_wb40_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
