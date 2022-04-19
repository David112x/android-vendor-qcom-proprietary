/* module_gamma40.c
 *
 * Copyright (c) 2012-2014, 2016 Qualcomm Technologies, Inc.
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
  ISP_DBG_MOD(ISP_LOG_GAMMA, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_GAMMA, fmt, ##args)

/* isp headers */
#include "isp_sub_module_common.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"
#include "module_gamma.h"
#include "gamma40.h"

/* TODO pass from Android.mk */
#define GAMMA40_VERSION "40"

#define GAMMA40_MODULE_NAME(n) \
  "gamma"n

static isp_sub_module_private_func_t gamma40_private_func = {
  .isp_sub_module_init_data              = gamma40_init,
  .isp_sub_module_destroy                = gamma40_destroy,
  .isp_sub_module_query_cap              = gamma40_query_cap,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_SET_PARM]        =
        isp_sub_module_port_set_param,
    [ISP_CONTROL_EVENT_SET_SUPER_PARM]  =
        isp_sub_module_port_set_super_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      gamma40_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  = gamma40_stats_aec_update,
    [ISP_MODULE_EVENT_STATS_ASD_UPDATE]  = gamma40_stats_asd_update,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_GET_VFE_DIAG_INFO_USER] = gamma40_get_vfe_diag_info_user,
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = gamma40_trigger_update,
    [ISP_PRIVATE_GET_GAMMA_INTERPOLATED_TABLE] = gamma40_get_interpolated_table,
  },

  .set_param_handler = {
    [ISP_SET_PARM_CONTRAST]              = gamma40_set_contrast,
    [ISP_SET_PARM_BESTSHOT_MODE]         = gamma40_set_bestshot,
    [ISP_SET_PARM_EFFECT]                = gamma40_set_spl_effect,
    [ISP_SET_PARM_SET_VFE_COMMAND]       = isp_sub_module_port_set_vfe_command,
    [ISP_SET_META_TONEMAP_MODE]          = isp_sub_module_set_manual_controls,
    [ISP_SET_META_TONEMAP_CURVES]        = isp_sub_module_set_manual_controls,
  },
};

/** module_gamma40_init:
 *
 *  @name: name of ISP module - "gamma40"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for gamma
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_gamma40_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, GAMMA40_MODULE_NAME(GAMMA40_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      GAMMA40_MODULE_NAME(GAMMA40_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &gamma40_private_func, ISP_MOD_GAMMA, "gamma", ISP_LOG_GAMMA);
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

/** module_gamma40_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_gamma40_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_gamma40_init,
  .module_deinit = module_gamma40_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
