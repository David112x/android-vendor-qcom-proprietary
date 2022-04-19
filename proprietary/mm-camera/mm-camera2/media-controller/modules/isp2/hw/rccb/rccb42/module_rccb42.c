/* module_rccb42.h
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
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
  ISP_DBG_MOD(ISP_LOG_WB, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_WB, fmt, ##args)

/* isp headers */
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "rccb42.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

/* TODO pass from Android.mk */
#define RCCB42_VERSION "42"

#define RCCB42_MODULE_NAME(n) \
  "rccb"n

static boolean module_rccb42_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static isp_sub_module_private_func_t rccb42_private_func = {
  .isp_sub_module_init_data              = rccb42_init,
  .isp_sub_module_destroy                = rccb42_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = rccb42_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = rccb42_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_STREAM_CONFIG] =
      module_rccb42_set_stream_config,
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      rccb42_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AWB_UPDATE]  = rccb42_stats_awb_update,
    [ISP_MODULE_EVENT_MANUAL_AWB_UPDATE] = rccb42_stats_awb_update,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE] = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE] = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE] = rccb42_trigger_update,
  },

  .set_param_handler = {
    [ISP_SET_PARM_SET_VFE_COMMAND]       = isp_sub_module_port_set_vfe_command,
    [ISP_SET_META_COLOR_CORRECT_MODE]    = isp_sub_module_set_manual_controls,
    [ISP_SET_META_MODE]                  = isp_sub_module_set_manual_controls,
    [ISP_SET_PARM_WHITE_BALANCE]         = isp_sub_module_set_manual_controls,
    [ISP_SET_META_COLOR_CORRECT_GAINS]   = isp_sub_module_set_manual_controls,
  },
};

/** module_rccb42_set_stream_config:
 *
 * @mod: rccb module
 * @isp_sub_module: handle to isp_sub_module_t
 * @event: event to be handled
 *
 * Handle stats config update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_rccb42_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return rccb42_set_stream_config(isp_sub_module,
    event->u.module_event.module_event_data);
}

/* module_rccb42_init:
 *
 * @name: name of ISP module - "rccb42"
 *
 * Initializes new instance of ISP module
 *
 * create mct module for rccb
 *
 * Return mct module handle on success or NULL on failure
 */

static mct_module_t *module_rccb42_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, RCCB42_MODULE_NAME(RCCB42_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      RCCB42_MODULE_NAME(RCCB42_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &rccb42_private_func, ISP_MOD_RCCB, "rccb", ISP_LOG_RCCB);
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

/*  module_rccb42_deinit:
 *
 * @module: isp module handle
 *
 * Deinit isp module
 *
 * Returns: void
 */
static void module_rccb42_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_rccb42_init,
  .module_deinit = module_rccb42_deinit,
};

/** module_open:
 *  *
 *   *  Return handle to isp_submod_init_table_t
 *    **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
