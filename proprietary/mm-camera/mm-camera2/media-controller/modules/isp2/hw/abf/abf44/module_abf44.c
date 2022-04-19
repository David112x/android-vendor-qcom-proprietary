/* module_abf44.c
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
#include "isp_sub_module_log.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_ABF, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_ABF, fmt, ##args)

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_common.h"
#include "abf44.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

#define ABF_ENABLE_BIT 31

/* TODO pass from Android.mk */
#define ABF44_VERSION "44"

#define ABF_MODULE_NAME(n) \
  "abf"n

static boolean module_abf44_init_data(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static void module_abf44_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static boolean module_abf44_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_abf44_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_abf44_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_abf44_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static void abf44_setloglevel(void);


static boolean module_abf44_aec_manual_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static isp_sub_module_private_func_t abf44_private_func = {
  .isp_sub_module_init_data              = module_abf44_init_data,
  .isp_sub_module_destroy                = module_abf44_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = isp_sub_module_port_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = module_abf44_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      module_abf44_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  =
      module_abf44_aec_update,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
    [ISP_MODULE_EVENT_STATS_AEC_MANUAL_UPDATE]  =
        module_abf44_aec_manual_update,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = module_abf44_trigger_update,
  },

  .set_param_handler = {
    [ISP_SET_META_NOISE_REDUCTION_MODE] = isp_sub_module_set_manual_controls,
    [ISP_SET_PARM_SET_VFE_COMMAND] = isp_sub_module_port_set_vfe_command,
    [ISP_SET_PARM_UPDATE_DEBUG_LEVEL]   = isp_sub_module_port_set_log_level,
  },

};

/** module_abf44_init_data:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the abf module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
static boolean module_abf44_init_data(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  return abf44_init(isp_sub_module);
}

/** module_abf44_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
static void module_abf44_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
   abf44_destroy(isp_sub_module);
}

/** module_abf44_stats_aec_update:
 *
 * @mod: demosaic module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_abf44_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return abf44_aec_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_abf44_aec_manual_update:
 *
 * @mod: demosaic module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_abf44_aec_manual_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return abf44_aec_manual_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_abf44_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_abf44_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }
  return abf44_streamoff(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_abf44_set_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_abf44_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }
  return abf44_set_chromatix_ptr(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_abf44_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_abf44_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }
  return abf44_trigger_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_abf44_init:
 *
 *  @name: name of ISP module - "abf44"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for abf
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_abf44_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, ABF_MODULE_NAME(ABF44_VERSION),
      strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      ABF_MODULE_NAME(ABF44_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &abf44_private_func, ISP_MOD_ABF, "abf", ISP_LOG_ABF);
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

/** module_abf44_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_abf44_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}
static isp_submod_init_table_t submod_init_table = {
  .module_init = module_abf44_init,
  .module_deinit = module_abf44_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
