/* module_gamma44.c
 *
 * Copyright (c) 2012-2014,2019 Qualcomm Technologies, Inc. All Rights Reserved.
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
#include "gamma44.h"

/* TODO pass from Android.mk */
#define GAMMA44_VERSION "44"

#define GAMMA44_MODULE_NAME(n) \
  "gamma"n

static boolean module_gamma44_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static isp_sub_module_private_func_t gamma44_private_func = {
  .isp_sub_module_init_data              = gamma44_init,
  .isp_sub_module_destroy                = gamma44_destroy,
  .isp_sub_module_query_cap              = gamma44_query_cap,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = module_gamma44_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = isp_sub_module_port_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]        =
        isp_sub_module_port_set_param,
    [ISP_CONTROL_EVENT_SET_SUPER_PARM]  =
        isp_sub_module_port_set_super_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      gamma44_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  = gamma44_stats_aec_update,
    [ISP_MODULE_EVENT_MANUAL_AWB_UPDATE] = gamma44_stats_aec_update,

    [ISP_MODULE_EVENT_STATS_ASD_UPDATE]  = gamma44_stats_asd_update,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
    [ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE] = isp_set_hdr_mode,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_GET_VFE_DIAG_INFO_USER] = gamma44_get_vfe_diag_info_user,
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = gamma44_trigger_update,
    [ISP_PRIVATE_GET_GAMMA_INTERPOLATED_TABLE] = gamma44_get_interpolated_table,
  },

  .set_param_handler = {
    [ISP_SET_PARM_CONTRAST]              = gamma44_set_contrast,
    [ISP_SET_PARM_BESTSHOT_MODE]         = gamma44_set_bestshot,
    [ISP_SET_PARM_EFFECT]                = gamma44_set_spl_effect,
    [ISP_SET_PARM_SET_VFE_COMMAND]       = isp_sub_module_port_set_vfe_command,
    [ISP_SET_META_TONEMAP_MODE]          = isp_sub_module_set_manual_controls,
    [ISP_SET_META_TONEMAP_CURVES]        = isp_sub_module_set_manual_controls,
    [ISP_SET_PARM_UPDATE_DEBUG_LEVEL]    = isp_sub_module_port_set_log_level,
  },
};

/** module_gamma44_streamon:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_gamma44_streamon(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }
  return gamma44_streamon(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_gamma44_init:
 *
 *  @name: name of ISP module - "gamma44"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for gamma
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_gamma44_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, GAMMA44_MODULE_NAME(GAMMA44_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      GAMMA44_MODULE_NAME(GAMMA44_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &gamma44_private_func, ISP_MOD_GAMMA, "gamma", ISP_LOG_GAMMA);
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

/** module_gamma44_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_gamma44_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_gamma44_init,
  .module_deinit = module_gamma44_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
