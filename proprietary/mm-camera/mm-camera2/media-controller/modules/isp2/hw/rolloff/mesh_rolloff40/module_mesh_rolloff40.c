/* module_mesh_rolloff40.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
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
  ISP_DBG_MOD(ISP_LOG_ROLLOFF, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_ROLLOFF, fmt, ##args)

/* isp headers */
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "mesh_rolloff40.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

/*TODO pass from Android.mk */
#define MESH_ROLLOFF40_VERSION "40"

#define MESH_ROLLOFF40_MODULE_NAME(n) \
  "mesh_rolloff"n

static boolean module_mesh_rolloff40_init_data(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static void module_mesh_rolloff40_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static boolean module_mesh_rolloff40_query_cap(mct_module_t *module,
  void *query_buf);
static boolean module_mesh_rolloff40_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_mesh_rolloff40_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_mesh_rolloff40_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_mesh_rolloff40_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_mesh_rolloff40_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_mesh_rolloff40_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_mesh_rolloff40_set_flash_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_mesh_rolloff40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_mesh_rolloff40_set_af_rolloff_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_mesh_rolloff40_sensor_lens_position_update(
  mct_module_t *module, isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_mesh_rolloff40_update_min_stripe_overlap(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);
static boolean module_mesh_rolloff40_set_stripe_info(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);
static boolean module_mesh_rolloff40_set_split_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_mesh_rolloff40_set_tintless_table(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);
static boolean module_mesh_rolloff40_set_parm_tintless(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);
static boolean module_mesh_rolloff40_set_hdr(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);
static boolean module_mesh_rolloff40_request_stats_type(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);
static boolean module_mesh_rolloff40_set_stream_config_overwrite(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static isp_sub_module_private_func_t mesh_rolloff40_private_func = {
  .isp_sub_module_init_data              = module_mesh_rolloff40_init_data,
  .isp_sub_module_destroy                = module_mesh_rolloff40_destroy,
  .isp_sub_module_query_cap              = module_mesh_rolloff40_query_cap,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = module_mesh_rolloff40_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = module_mesh_rolloff40_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_STREAM_CONFIG] =
      module_mesh_rolloff40_set_stream_config,
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      module_mesh_rolloff40_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  =
      module_mesh_rolloff40_stats_aec_update,
    [ISP_MODULE_EVENT_STATS_AWB_UPDATE]  =
      module_mesh_rolloff40_stats_awb_update,
    [ISP_MODULE_EVENT_MANUAL_AWB_UPDATE] =
      module_mesh_rolloff40_stats_awb_update,
    [ISP_MODULE_EVENT_SET_FLASH_MODE]  =
      module_mesh_rolloff40_set_flash_mode,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_SET_AF_ROLLOFF_PARAMS] =
      module_mesh_rolloff40_set_af_rolloff_params,
    [ISP_MODULE_EVENT_SENSOR_LENS_POSITION_UPDATE] =
      module_mesh_rolloff40_sensor_lens_position_update,
    [ISP_MODULE_EVENT_TINTLESS_ALGO_UPDATE] =
      module_mesh_rolloff40_set_tintless_table,
    [ISP_MODULE_EVENT_REQUEST_STATS_TYPE] =
      module_mesh_rolloff40_request_stats_type,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
    [ISP_MODULE_EVENT_OFFLINE_CONFIG_OVERWRITE] =
      module_mesh_rolloff40_set_stream_config_overwrite,

  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE] = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE] = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE] = module_mesh_rolloff40_trigger_update,
    [ISP_PRIVATE_REQUEST_STRIPE_LIMITATION] = module_mesh_rolloff40_update_min_stripe_overlap,
    [ISP_PRIVATE_SET_STRIPE_INFO] = module_mesh_rolloff40_set_stripe_info,
    [ISP_PRIVATE_SET_STREAM_SPLIT_INFO] = module_mesh_rolloff40_set_split_info,
  },

  .set_param_handler = {
    [ISP_SET_PARM_SET_VFE_COMMAND]       = isp_sub_module_port_set_vfe_command,
    [ISP_SET_PARM_TINTLESS]              = module_mesh_rolloff40_set_parm_tintless,
    [ISP_SET_META_COLOR_CORRECT_MODE]    = isp_sub_module_set_manual_controls,
    [ISP_SET_META_MODE]                  = isp_sub_module_set_manual_controls,
    [ISP_SET_PARM_WHITE_BALANCE]         = isp_sub_module_set_manual_controls,
    [ISP_SET_META_LENS_SHADING_MAP_MODE] = isp_sub_module_set_manual_controls,
    [ISP_SET_META_LENS_SHADING_MODE]     = isp_sub_module_set_manual_controls,
    [ISP_SET_PARM_HDR]                   = module_mesh_rolloff40_set_hdr,
  },
};

/** module_mesh_rolloff40_query_cap:
 *
 *  @module: mct module handle
 *  @query_buf: query caps buffer handle
 *
 *  Destroy dynamic resources
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_query_cap(mct_module_t *module,
  void *query_buf)
{
  if (!module || !query_buf) {
    ISP_ERR("failed: %p %p", module, query_buf);
    return FALSE;
  }
  return mesh_rolloff40_query_cap(module, query_buf);
}

/** module_mesh_rolloff40_init_data:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the chroma_suppress module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
static boolean module_mesh_rolloff40_init_data(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  return mesh_rolloff40_init(isp_sub_module);
}

/** mesh_rolloff40_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
static void module_mesh_rolloff40_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  mesh_rolloff40_destroy(isp_sub_module);
}

/** module_mesh_rolloff40_stats_aec_update:
 *
 * @mod: demosaic module
 * @isp_sub_module: isp sub module handle
 * @event: mct event handle
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return mesh_rolloff40_stats_aec_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_mesh_rolloff40_stats_awb_update:
 *
 * @mod: demosaic module
 * @isp_sub_module: isp sub module handle
 * @event: mct event handle
 *
 * Handle AWB update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return mesh_rolloff40_stats_awb_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_mesh_rolloff40_set_flash_mode:
 *
 * @mod: demosaic module
 * @isp_sub_module: isp sub module handle
 * @event: mct event handle
 *
 * Handle set flash mode event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_set_flash_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return mesh_rolloff40_set_flash_mode(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_mesh_rolloff40_streamon:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }
  return mesh_rolloff40_streamon(isp_sub_module,
    event->u.module_event.module_event_data);
} /* module_mesh_rolloff40_streamon */

/** module_mesh_rolloff40_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }
  return mesh_rolloff40_streamoff(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_mesh_rolloff40_set_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }
  return mesh_rolloff40_set_chromatix_ptr(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_mesh_rolloff40_set_stream_config_overwrite:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_set_stream_config_overwrite(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  ISP_ERR("received stream config rolloff");
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }
  return mesh_rolloff40_set_stream_config_overwrite(isp_sub_module);
}

/** module_mesh_rolloff40_set_stream_config:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  ISP_DBG("received stream config rolloff");
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }
  return mesh_rolloff40_set_stream_config(isp_sub_module,
    event->u.module_event.module_event_data);
}


/** module_mesh_rolloff40_update_min_stripe_overlap:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_update_min_stripe_overlap(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }

  return mesh_rolloff40_update_min_stripe_overlap(isp_sub_module,
    event->u.module_event.module_event_data);
}


/** module_mesh_rolloff40_set_stripe_info:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_set_stripe_info(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }

  return mesh_rolloff40_set_stripe_info(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_mesh_rolloff40_set_split_info:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_set_split_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }
  return mesh_rolloff40_set_split_info(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_mesh_rolloff40_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }
  return mesh_rolloff40_trigger_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_mesh_rolloff40_set_af_rolloff_params:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Handle AF rolloff params event
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_set_af_rolloff_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }
  return mesh_rolloff40_set_af_rolloff_params(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_mesh_rolloff40_sensor_lens_position_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Handle AF rolloff params event
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_sensor_lens_position_update(
  mct_module_t *module, isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }
  return mesh_rolloff40_sensor_lens_position_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_mesh_rolloff40_set_tintless_table:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Set tintless rolloff table
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_set_tintless_table(
  mct_module_t *module, isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }
  return mesh_rolloff40_set_tintless_table(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_mesh_rolloff40_set_parm_tintless:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Set tintless params
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_set_parm_tintless(
  mct_module_t *module, isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }
  return mesh_rolloff40_set_parm_tintless(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_mesh_rolloff40_set_hdr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Set hdr params
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_set_hdr(
  mct_module_t *module, isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  return mesh_rolloff40_set_parm_hdr(isp_sub_module,event);
}

/** module_mesh_rolloff40_request_stats_type:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Get stats type for tintless
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_mesh_rolloff40_request_stats_type(
  mct_module_t *module, isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }
  return mesh_rolloff40_request_stats_type(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_mesh_rolloff40_init:
 *
 *  @name: name of ISP module - "mesh_rolloff40"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for mesh_rolloff
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_mesh_rolloff40_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, MESH_ROLLOFF40_MODULE_NAME(MESH_ROLLOFF40_VERSION),
      strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      MESH_ROLLOFF40_MODULE_NAME(MESH_ROLLOFF40_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &mesh_rolloff40_private_func, ISP_MOD_ROLLOFF, "rolloff", ISP_LOG_ROLLOFF);
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

/** module_mesh_rolloff40_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_mesh_rolloff40_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_mesh_rolloff40_init,
  .module_deinit = module_mesh_rolloff40_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
