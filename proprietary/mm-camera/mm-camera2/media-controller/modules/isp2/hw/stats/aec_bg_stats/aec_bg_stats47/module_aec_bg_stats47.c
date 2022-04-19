/* module_aec_bg_stats47.c
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
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
#include "aec_bg_stats47.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

/* TODO pass from Android.mk */
#define AEC_BG_STATS47_VERSION "47"

#define AEC_BG_STATS47_MODULE_NAME(n) \
  "aec_bg_stats"n

static boolean module_aec_bg_stats47_init_data(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static void module_aec_bg_stats47_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static boolean module_aec_bg_stats47_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_aec_bg_stats47_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_aec_bg_stats47_stats_config_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_aec_bg_stats47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_aec_bg_stats47_set_split_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_aec_bg_stats47_set_stripe_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static isp_sub_module_private_func_t aec_bg_stats47_private_func = {
  .isp_sub_module_init_data              = module_aec_bg_stats47_init_data,
  .isp_sub_module_destroy                = module_aec_bg_stats47_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = isp_sub_module_port_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = module_aec_bg_stats47_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_STREAM_CONFIG] =
      module_aec_bg_stats47_set_stream_config,
    [ISP_MODULE_EVENT_STATS_AEC_CONFIG_UPDATE]  =
      module_aec_bg_stats47_stats_config_update,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_REQUEST_STATS_TYPE] =
     isp_sub_module_port_request_stats_type,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = module_aec_bg_stats47_trigger_update,
    [ISP_PRIVATE_SET_STRIPE_INFO] = module_aec_bg_stats47_set_stripe_info,
    [ISP_PRIVATE_SET_STREAM_SPLIT_INFO] = module_aec_bg_stats47_set_split_info,

  },

  .set_param_handler = {
    [ISP_SET_PARM_TINTLESS] = isp_sub_module_port_set_parm_tintless,
  },

};

/** module_aec_bg_stats47_init_data:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the chroma_suppress module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
static boolean module_aec_bg_stats47_init_data(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  return aec_bg_stats47_init(isp_sub_module);
}

/** aec_bg_stats47_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
static void module_aec_bg_stats47_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  aec_bg_stats47_destroy(isp_sub_module);
}

/** module_aec_bg_stats47_stats_config_update:
 *
 * @mod: demosaic module
 * @isp_sub_module: handle to isp_sub_module_t
 * @event: event to be handled
 *
 * Handle stats config update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_aec_bg_stats47_stats_config_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return aec_bg_stats47_stats_config_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_aec_bg_stats47_set_stream_config:
 *
 * @mod: demosaic module
 * @isp_sub_module: handle to isp_sub_module_t
 * @event: event to be handled
 *
 * Handle stats config update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_aec_bg_stats47_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return aec_bg_stats47_set_stream_config(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_aec_bg_stats47_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_aec_bg_stats47_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }
  return aec_bg_stats47_streamoff(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_aec_bg_stats47_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_aec_bg_stats47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }
  return aec_bg_stats47_trigger_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_aec_bg_stats47_set_stripe_info:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_aec_bg_stats47_set_stripe_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }
  return aec_bg_stats47_set_stripe_info(isp_sub_module,
    event->u.module_event.module_event_data);
}
/** module_aec_bg_stats47_set_split_info:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_aec_bg_stats47_set_split_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }
  return aec_bg_stats47_set_split_info(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_aec_bg_stats47_init:
 *
 *  @name: name of ISP module - "aec_bg_stats47"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for aec_bg_stats
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_aec_bg_stats47_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, AEC_BG_STATS47_MODULE_NAME(AEC_BG_STATS47_VERSION),
      strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      AEC_BG_STATS47_MODULE_NAME(AEC_BG_STATS47_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
   &aec_bg_stats47_private_func, ISP_MOD_AEC_BG_STATS, "aec_bg", ISP_LOG_AEC_BG);

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

/** module_aec_bg_stats47_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_aec_bg_stats47_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_aec_bg_stats47_init,
  .module_deinit = module_aec_bg_stats47_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
