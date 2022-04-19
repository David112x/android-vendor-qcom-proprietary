/* module_hdr46.c
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
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
  ISP_DBG_MOD(ISP_LOG_HDR, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_HDR, fmt, ##args)

/* isp headers */
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "module_hdr46.h"
#include "hdr46.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

/* TODO pass from Android.mk */
#define HDR46_VERSION "46"

#define HDR46_MODULE_NAME(n) \
  "hdr"n

static isp_sub_module_private_func_t hdr46_private_func = {
  .isp_sub_module_init_data = hdr46_init,
  .isp_sub_module_destroy   = hdr46_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]       = hdr46_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]      = hdr46_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]       = isp_sub_module_port_set_param,
    [ISP_CONTROL_EVENT_SET_SUPER_PARM] = isp_sub_module_port_set_super_param,
  },

  .module_event_handler = {
    /*common*/
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      isp_sub_module_port_set_chromatix_ptr,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE] = hdr46_save_aec_params,
    [ISP_MODULE_EVENT_STATS_AWB_UPDATE] = hdr46_save_awb_params,
    [ISP_MODULE_EVENT_MANUAL_AWB_UPDATE] = hdr46_save_awb_params,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
  },

  .isp_private_event_handler = {
    /*common*/
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = hdr46_trigger_update,
  },
  .set_param_handler = {
    /*chroma enhance specific*/
    [ISP_SET_PARM_SET_VFE_COMMAND] = isp_sub_module_port_set_vfe_command,
    [ISP_SET_PARM_UPDATE_DEBUG_LEVEL]   = isp_sub_module_port_set_log_level,
  },
};

/** module_hdr46_init:
 *
 *  @name: name of ISP module - "hdr46"
 *
 * Initializes new instance of ISP module
 *
 * create mct module for hdr
 *
 * Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_hdr46_init(const char *name)
{
  boolean            ret = TRUE;
  mct_module_t      *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_DBG("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, HDR46_MODULE_NAME(HDR46_VERSION),
    strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      HDR46_MODULE_NAME(HDR46_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &hdr46_private_func, ISP_MOD_HDR, "hdr", ISP_LOG_HDR);
  if (!module) {
    ISP_ERR("module %p", module);
    return NULL;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    goto ERROR1;
  }

  isp_sub_module_priv->private_func = &hdr46_private_func;

  return module;

ERROR1:
  mct_module_destroy(module);
  ISP_ERR("failed");
  return NULL;
}

/** module_hdr46_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_hdr46_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_hdr46_init,
  .module_deinit = module_hdr46_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
