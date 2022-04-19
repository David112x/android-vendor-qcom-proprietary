/* module_gtm46.c
 *
 * Copyright (c) 2014,2019 Qualcomm Technologies, Inc. All Rights Reserved.
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
  ISP_DBG_MOD(ISP_LOG_GTM, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_GTM, fmt, ##args)

/* isp headers */
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "gtm46.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

/* TODO pass from Android.mk */
#define GTM46_VERSION "46"

#define GTM46_MODULE_NAME(n) \
  "gtm"n

static boolean module_gtm46_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);


static isp_sub_module_private_func_t gtm46_private_func = {
  .isp_sub_module_init_data              = gtm46_init,
  .isp_sub_module_destroy                = gtm46_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = gtm46_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = gtm46_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      module_gtm46_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  = gtm46_stats_aec_update,
    [ISP_MODULE_EVENT_MANUAL_AWB_UPDATE] = gtm46_stats_aec_update,
    [ISP_MODULE_EVENT_GTM_ALGO_UPDATE]   = gtm46_algo_update,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE] = gtm46_set_hdr_mode,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE] = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE] = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE] = gtm46_trigger_update,
    [ISP_PRIVATE_FETCH_GTM_DATA]    = gtm46_fetch_gtm_data,
  },

  .set_param_handler = {
    [ISP_SET_PARM_SET_VFE_COMMAND] = isp_sub_module_port_set_vfe_command,
    [ISP_SET_PARM_UPDATE_DEBUG_LEVEL]   = isp_sub_module_port_set_log_level,
  },
};

/** module_gtm46_init:
 *
 *  @name: name of ISP module - "gtm46"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for gtm
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_gtm46_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, GTM46_MODULE_NAME(GTM46_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      GTM46_MODULE_NAME(GTM46_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &gtm46_private_func, ISP_MOD_GTM, "gtm", ISP_LOG_GTM);
  if (!module) {
    ISP_ERR("module is NULL");
    goto ERROR2;
  }
  if(!MCT_OBJECT_PRIVATE(module)) {
    ISP_ERR("MCT_OBJECT_PRIVATE(module) is NULL");
    goto ERROR1;
  }

  return module;

ERROR1:
  mct_module_destroy(module);
ERROR2:
  ISP_ERR("failed");
  return NULL;
}

/** module_gtm46_set_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_gtm46_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }
  return gtm46_set_chromatix_ptr(isp_sub_module,
    event->u.module_event.module_event_data);
}


/** module_gtm46_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_gtm46_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_gtm46_init,
  .module_deinit = module_gtm46_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
