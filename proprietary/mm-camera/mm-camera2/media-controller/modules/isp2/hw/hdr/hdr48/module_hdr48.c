/* module_hdr48.c
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
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "hdr48.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

#define hdr48_VERSION "48"

#define hdr48_MODULE_NAME(n) \
  "hdr"n

/* isp headers */
#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_HDR, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_HDR, fmt, ##args)

static boolean module_hdr48_init_data(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static void module_hdr48_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static boolean module_hdr48_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_hdr48_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_hdr48_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_hdr48_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_hdr48_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_hdr48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_hdr48_update_sudmod_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);


static isp_sub_module_private_func_t hdr48_private_func = {
  .isp_sub_module_init_data              = module_hdr48_init_data,
  .isp_sub_module_destroy                = module_hdr48_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = module_hdr48_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = module_hdr48_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      module_hdr48_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  =
      module_hdr48_stats_aec_update,
    [ISP_MODULE_EVENT_STATS_AWB_UPDATE]  =
      module_hdr48_stats_awb_update,
    [ISP_MODULE_EVENT_MANUAL_AWB_UPDATE] =
       module_hdr48_stats_awb_update,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE] =
      isp_set_hdr_mode,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE] = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE] = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE] = module_hdr48_trigger_update,
    [ISP_PRIVATE_CURRENT_SUBMOD_ENABLE] = module_hdr48_update_sudmod_enable,
  },

  .set_param_handler = {
    [ISP_SET_PARM_SET_VFE_COMMAND]  = isp_sub_module_port_set_vfe_command,
    [ISP_SET_PARM_UPDATE_DEBUG_LEVEL]   = isp_sub_module_port_set_log_level,
   },
};

/** module_hdr48_init_data:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the chroma_suppress module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
static boolean module_hdr48_init_data(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module)
{
  if (!module) {
    ISP_ERR("failed: %p", module);
    return FALSE;
  }
  return hdr48_init(isp_sub_module);
}

/** hdr48_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
static void module_hdr48_destroy(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module)
{
  if (!module) {
    ISP_ERR("failed: %p", module);
    return;
  }
  hdr48_destroy(isp_sub_module);
}

/** module_hdr48_stats_aec_update:
 *
 * @mod: hdr48 module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_hdr48_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return hdr48_stats_aec_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_hdr48_stats_awb_update:
 *
 * @mod: hdr48 module
 * @data: handle to stats_update_t
 *
 * Handle AWB update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_hdr48_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return hdr48_stats_awb_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_hdr48_streamon:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_hdr48_streamon(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }
  return hdr48_streamon(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_hdr48_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_hdr48_streamoff(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }
  return hdr48_streamoff(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_hdr48_set_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function updates module enable flag
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_hdr48_set_chromatix_ptr(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }
  return hdr48_set_chromatix_ptr(isp_sub_module,
    event->u.module_event.module_event_data);
}

static boolean module_hdr48_update_sudmod_enable(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  boolean                   ret = TRUE;
  uint8_t                   *submod_enable     = NULL;
  isp_private_event_t       *private_event     = NULL;
  chromatix_VFE_common_type *pchromatix_common = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  private_event = event->u.module_event.module_event_data;

  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  if (!private_event->data) {
    ISP_ERR("failed: data %p", private_event->data);
    return FALSE;
  }

  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->config_pending = TRUE;

  return TRUE;
}

/** module_hdr48_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_hdr48_trigger_update(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  return hdr48_trigger_update(module, isp_sub_module,
    event->u.module_event.module_event_data, event->identity);
}

/** module_hdr48_init:
 *
 *  @name: name of ISP module - "hdr48"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for hdr
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_hdr48_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, hdr48_MODULE_NAME(hdr48_VERSION),
      strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      hdr48_MODULE_NAME(hdr48_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &hdr48_private_func, ISP_MOD_HDR, "hdr",
     ISP_LOG_HDR);
  if (!module || !MCT_OBJECT_PRIVATE(module)) {
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

/** module_hdr48_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_hdr48_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_hdr48_init,
  .module_deinit = module_hdr48_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}

