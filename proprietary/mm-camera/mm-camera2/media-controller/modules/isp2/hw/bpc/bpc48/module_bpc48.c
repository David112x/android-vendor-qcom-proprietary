/* module_bpc48.c
 *
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
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

/* isp headers */
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "bpc48.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module_util.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_BPC, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_BPC, fmt, ##args)

#define NUM_SINK_PORTS   1

/* TODO pass from Android.mk */
#define BPC48_VERSION "48"

#define BPC48_MODULE_NAME(n) \
  "bpc"n

static boolean module_bpc48_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_bpc48_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_bpc48_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_bpc48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_bpc48_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_bpc48_module_cfg(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static isp_sub_module_private_func_t bpc48_private_func = {
  .isp_sub_module_init_data              = bpc48_init,
  .isp_sub_module_destroy                = bpc48_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = bpc48_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = module_bpc48_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] = module_bpc48_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  = module_bpc48_stats_aec_update,
    [ISP_MODULE_EVENT_STATS_AWB_UPDATE]  = module_bpc48_stats_awb_update,
    [ISP_MODULE_EVENT_MANUAL_AWB_UPDATE] = module_bpc48_stats_awb_update,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE] = isp_set_hdr_mode,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = module_bpc48_trigger_update,
    [ISP_PRIVATE_EVENT_GET_MOD_CFG_MASK] = module_bpc48_module_cfg,
  },

  .set_param_handler = {
    [ISP_SET_PARM_SET_VFE_COMMAND] = isp_sub_module_port_set_vfe_command,
    [ISP_SET_INTF_HOTPIXEL_MODE]   = isp_sub_module_set_manual_controls,
    [ISP_SET_PARM_ZSL_MODE]        = isp_sub_module_port_set_zsl_mode,
  },
};

/** module_bpc48_stats_awb_update:
 *
 * @mod: bpc48 module
 * @data: handle to stats_update_t
 *
 * Handle AWB update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_bpc48_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return bpc48_stats_awb_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_bpc48_stats_aec_update:
 *
 * @mod: bpc48 module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_bpc48_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return bpc48_stats_aec_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_bpc48_module_cfg:
 *
 *  @module: bpc48 module pointer
 *  @isp_sub_module: isp_sub_module pointer
 *  @event: event pointer
 *
 *  Checks if hystersis needs to be enabled
 *
 *  create mct module for bpc
 *
 *  Return mct module handle on success or NULL on failure
 **/
static boolean module_bpc48_module_cfg(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  bpc48_t *bpc = NULL;
  boolean ret  = TRUE;

  if (!module || !event || !isp_sub_module || !isp_sub_module->private_data) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  bpc = (bpc48_t*)isp_sub_module->private_data;

  ret = bpc48_decide_hysterisis(isp_sub_module, bpc->aec_reference);

  ret = isp_sub_module_util_get_cfg_mask(isp_sub_module, event);

  return ret;
}

/** module_bpc48_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_bpc48_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event ) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }
  if (!module) {
    ISP_ERR("failed: module %p", module);
  }
  return bpc48_streamoff(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_bpc48_set_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Store chromatix ptr and make initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_bpc48_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }
  if (!module) {
    ISP_ERR("failed: module %p", module);
  }
  return bpc48_set_chromatix_ptr(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_bpc48_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  trigger update based on 3A event
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_bpc48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event || !module) {
    ISP_ERR("failed: isp_sub_module %p event %p module %p", isp_sub_module,
      event, module);
    return FALSE;
  }
  return bpc48_trigger_update(module, isp_sub_module, event);
}

/** module_bpc48_init:
 *
 *  @name: name of ISP module - "bpc48"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for bpc
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_bpc48_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, BPC48_MODULE_NAME(BPC48_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      BPC48_MODULE_NAME(BPC48_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &bpc48_private_func, ISP_MOD_BPC, "bpc", ISP_LOG_BPC);
  if (!module || !MCT_OBJECT_PRIVATE(module)) {
    ISP_ERR("failed");
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

/** module_bpc48_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_bpc48_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_bpc48_init,
  .module_deinit = module_bpc48_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
