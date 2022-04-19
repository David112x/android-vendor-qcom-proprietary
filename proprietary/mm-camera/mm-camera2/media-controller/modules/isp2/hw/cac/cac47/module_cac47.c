/* module_cac47.c
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

/* isp headers */
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "module_cac47.h"
#include "cac47.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module_util.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_CAC, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_CAC, fmt, ##args)

/* TODO pass from Android.mk */
#define CAC47_VERSION "47"

#define CAC47_MODULE_NAME(n) \
  "cac"n

static isp_sub_module_private_func_t cac47_private_func = {
  .isp_sub_module_init_data = cac47_init,
  .isp_sub_module_destroy   = cac47_destroy,
  .isp_sub_module_query_cap = cac47_query_cap,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = cac47_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = cac47_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
    [ISP_CONTROL_EVENT_SET_SUPER_PARM]   = isp_sub_module_port_set_super_param,
  },

  .module_event_handler = {
    /*common*/
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      cac47_set_chromatix_ptr,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE] = cac47_save_aec_params,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
  },

  .isp_private_event_handler = {
    /*common*/
    [ISP_PRIVATE_SET_MOD_ENABLE]           = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]       = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]       = cac47_trigger_update,
    [ISP_PRIVATE_PASS_SCALER_SCALE_FACTOR] = cac47_set_scaler_scale_factor,
  },
  .set_param_handler = {
    /*chroma enhance specific*/
    [ISP_SET_PARM_SET_VFE_COMMAND] = isp_sub_module_port_set_vfe_command,
    [ISP_SET_INTF_PARM_CAC]        = isp_sub_module_set_manual_controls,
  },
};

/** module_cac47_init:
 *
 *  @name: name of ISP module - "cac47"
 *
 * Initializes new instance of ISP module
 *
 * create mct module for cac47
 *
 * Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_cac47_init(const char *name)
{
  boolean            ret = TRUE;
  mct_module_t      *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_DBG("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, CAC47_MODULE_NAME(CAC47_VERSION),
    strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      CAC47_MODULE_NAME(CAC47_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &cac47_private_func, ISP_MOD_CAC, "cac", ISP_LOG_CAC);
  if (!module) {
    ISP_ERR("module %p", module);
    return NULL;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    goto ERROR1;
  }

  isp_sub_module_priv->private_func = &cac47_private_func;

  return module;

ERROR1:
  mct_module_destroy(module);
  ISP_ERR("failed");
  return NULL;
}

/** module_cac47_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_cac47_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_cac47_init,
  .module_deinit = module_cac47_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
