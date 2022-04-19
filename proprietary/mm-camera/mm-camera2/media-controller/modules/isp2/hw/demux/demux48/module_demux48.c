/* module_demux48.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
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
  ISP_DBG_MOD(ISP_LOG_DEMUX, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_DEMUX, fmt, ##args)

/* isp headers */
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "module_demux48.h"
#include "demux48.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

/* TODO pass from Android.mk */
#define DEMUX48_VERSION "48"

#define DEMUX48_MODULE_NAME(n) \
  "demux"n

static isp_sub_module_private_func_t demux48_private_func = {
  .isp_sub_module_init_data              = demux48_init,
  .isp_sub_module_destroy                = demux48_destroy,
  .isp_sub_module_query_cap              = demux48_query_cap,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = demux48_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = demux48_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_STREAM_CONFIG]  =
      demux48_set_stream_config,
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      isp_sub_module_port_set_chromatix_ptr,
    [ISP_MODULE_EVENT_SET_DIGITAL_GAIN] =
      demux48_set_digital_gain,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE] =
      isp_set_hdr_mode,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE] = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE] = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE] = demux48_trigger_update,
    [ISP_PRIVATE_FETCH_DEMUX_GAIN]   = demux48_fetch_demux_gain,
  },

  .set_param_handler = {
    [ISP_SET_PARM_SET_VFE_COMMAND] = isp_sub_module_port_set_vfe_command,
    [ISP_SET_PARM_UPDATE_DEBUG_LEVEL]   = isp_sub_module_port_set_log_level,
    [ISP_SET_META_MODE]                 = isp_sub_module_set_manual_controls,
    [ISP_SET_AEC_MODE]                  = isp_sub_module_set_manual_controls,
    [ISP_SET_MANUAL_GAIN]               = isp_sub_module_set_manual_controls,
  },
};

/** module_demux48_init:
 *
 *  @name: name of ISP module - "demux48"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for demux
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_demux48_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, DEMUX48_MODULE_NAME(DEMUX48_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      DEMUX48_MODULE_NAME(DEMUX48_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &demux48_private_func, ISP_MOD_DEMUX, "demux", ISP_LOG_DEMUX);
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

/** module_demux48_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_demux48_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_demux48_init,
  .module_deinit = module_demux48_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
