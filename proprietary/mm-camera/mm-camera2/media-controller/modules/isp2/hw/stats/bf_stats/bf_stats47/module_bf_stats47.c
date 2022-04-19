/* module_bf_stats47.c
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
#include "isp_log.h"
#include "bf_stats47.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

/* TODO pass from Android.mk */
#define BF_STATS47_VERSION "47"

#define BF_STATS47_MODULE_NAME(n) \
  "bf_stats"n

static boolean module_bf_stats47_init_data(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static void module_bf_stats47_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static boolean module_bf_stats47_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_bf_stats47_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_bf_stats47_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_bf_stats47_stats_config_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_bf_stats47_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_bf_stats47_af_exp_compensate(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_bf_stats47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_bf_stats47_update_min_stripe_overlap(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);
static boolean module_bf_stats47_get_stats_capabilities(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_bf_stats47_set_split_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_bf_stats47_set_stripe_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static isp_sub_module_private_func_t bf_stats47_private_func = {
  .isp_sub_module_init_data              = module_bf_stats47_init_data,
  .isp_sub_module_destroy                = module_bf_stats47_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = module_bf_stats47_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = module_bf_stats47_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_STREAM_CONFIG] =
      module_bf_stats47_set_stream_config,
    [ISP_MODULE_EVENT_STATS_AF_CONFIG_UPDATE]  =
      module_bf_stats47_stats_config_update,
    [ISP_MODULE_EVENT_AF_EXP_COMPENSATE] =
      module_bf_stats47_af_exp_compensate,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  =
      module_bf_stats47_aec_update,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE] = isp_set_hdr_mode,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = module_bf_stats47_trigger_update,
    [ISP_PRIVATE_REQUEST_STRIPE_LIMITATION] =
      module_bf_stats47_update_min_stripe_overlap,
    [ISP_PRIVATE_SET_STRIPE_INFO] = module_bf_stats47_set_stripe_info,
    [ISP_PRIVATE_SET_STREAM_SPLIT_INFO] = module_bf_stats47_set_split_info,
    [ISP_PRIVATE_HW_GET_STATS_CAPABILITES] =
      module_bf_stats47_get_stats_capabilities,
  },
};

/** module_bf_stats47_init_data:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the chroma_suppress module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
static boolean module_bf_stats47_init_data(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
  }
  return bf_stats47_init(isp_sub_module);
}

/** bf_stats47_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
static void module_bf_stats47_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
  }
  bf_stats47_destroy(isp_sub_module);
}

/** module_bf_stats47_set_stripe_info:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_bf_stats47_set_stripe_info(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  return bf_stats47_set_stripe_info(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_bf_stats47_set_split_info:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_bf_stats47_set_split_info(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  return bf_stats47_set_split_info(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_bf_stats47_update_min_stripe_overlap:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_bf_stats47_update_min_stripe_overlap(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  return bf_stats47_update_min_stripe_overlap(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_bf_stats47_stats_config_update:
 *
 *  @mod: demosaic module
 *  @isp_sub_module: isp sub module handle
 *  @event: event to be handled
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean module_bf_stats47_stats_config_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return bf_stats47_stats_config_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_bf_stats47_aec_update:
 *
 *  @mod: bf stats module
 *  @isp_sub_module: isp sub module handle
 *  @event: event to be handled
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean module_bf_stats47_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return bf_stats47_aec_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_bf_stats47_af_exp_compensate:
 *
 *  @mod: bf stats module
 *  @isp_sub_module: isp sub module handle
 *  @event: event to be handled
 *
 *  Handle af exposure compensate event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean module_bf_stats47_af_exp_compensate(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return bf_stats47_af_exp_compensate(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_bf_stats47_set_stream_config:
 *
 *  @mod: demosaic module
 *  @isp_sub_module: isp sub module handle
 *  @event: event to be handled
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean module_bf_stats47_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return bf_stats47_set_stream_config(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_bf_stats47_streamon:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_bf_stats47_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return bf_stats47_streamon(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_bf_stats47_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_bf_stats47_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return bf_stats47_streamoff(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_bf_stats47_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_bf_stats47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return bf_stats47_trigger_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_bf_stats47_get_stats_capabilities:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function gets stats capabilities
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_bf_stats47_get_stats_capabilities(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }
  return bf_stats47_get_stats_capabilities(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_bf_stats47_init:
 *
 *  @name: name of ISP module - "bf_stats47"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for bf_stats
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_bf_stats47_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, BF_STATS47_MODULE_NAME(BF_STATS47_VERSION),
      strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      BF_STATS47_MODULE_NAME(BF_STATS47_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &bf_stats47_private_func, ISP_MOD_BF_STATS, "bf_stats", ISP_LOG_BF_STATS);
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

/** module_bf_stats47_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_bf_stats47_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_bf_stats47_init,
  .module_deinit = module_bf_stats47_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
