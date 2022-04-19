/*
 * Copyright (c) 2016, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "pdaf48.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
 ISP_DBG_MOD(ISP_LOG_PDAF, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
 ISP_HIGH_MOD(ISP_LOG_PDAF, fmt, ##args)

static boolean module_pdaf48_init_data(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static void module_pdaf48_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static boolean module_pdaf48_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_pdaf48_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_pdaf48_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_pdaf48_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_pdaf48_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_pdaf48_set_pdaf_pixel(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_pdaf48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_pdaf48_update_offset(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_pdaf48_set_split_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);



static isp_sub_module_private_func_t pdaf48_private_func = {
  .isp_sub_module_init_data              = module_pdaf48_init_data,
  .isp_sub_module_destroy                = module_pdaf48_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = module_pdaf48_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = module_pdaf48_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      module_pdaf48_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  =
      module_pdaf48_stats_aec_update,
    [ISP_MODULE_EVENT_STATS_AWB_UPDATE]  =
      module_pdaf48_stats_awb_update,
    [ISP_MODULE_EVENT_SET_PDAF_PATTERN] =
      module_pdaf48_set_pdaf_pixel,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE] =
      isp_set_hdr_mode,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = module_pdaf48_trigger_update,
    [ISP_PRIVATE_REQUEST_STRIPE_LIMITATION] = module_pdaf48_update_offset,
    [ISP_PRIVATE_SET_STREAM_SPLIT_INFO]  = module_pdaf48_set_split_info,
  },
};

/** module_pdaf48_init_data:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the pdpc module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
static boolean module_pdaf48_init_data(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
   if (!module || !isp_sub_module) {
     ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
     return FALSE;
   }

  return pdaf48_init(isp_sub_module);
}

/** module_pdaf48_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
static void module_pdaf48_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
   if (!module || !isp_sub_module) {
     ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
     return;
   }
  pdaf48_destroy(isp_sub_module);
}

/** module_pdaf48_stats_aec_update:
 *
 * @module: mct module handle
 * @isp_sub_module: isp sub module handle
 * @event: mct event handle
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_pdaf48_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return pdaf48_stats_aec_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_pdaf48_stats_awb_update:
 *
 * @module: mct module handle
 * @isp_sub_module: isp sub module handle
 * @event: mct event handle
 *
 * Handle AWB update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_pdaf48_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return pdaf48_stats_awb_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_pdaf48_streamon:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_pdaf48_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  return pdaf48_streamon(isp_sub_module,
    event->u.module_event.module_event_data);
} /* module_pdaf48_streamon */

/** module_pdaf48_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  TThis function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_pdaf48_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  return pdaf48_streamoff(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_pdaf48_set_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Store chromatix ptr and make initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_pdaf48_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  return pdaf48_set_chromatix_ptr(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_pdaf48_set_pdaf_pixel:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Set PDAF pixel pattern
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_pdaf48_set_pdaf_pixel(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  return pdaf48_set_pdaf_pixel(isp_sub_module,
    event->u.module_event.module_event_data);
}
/** module_pdaf48_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function handles trigger update
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_pdaf48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  return pdaf48_trigger_update(module, isp_sub_module, event);
}

/** module_pdaf48_update_offset:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function handles trigger update
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_pdaf48_update_offset(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  return pdaf48_update_offset(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_pdaf48_set_split_info:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_pdaf48_set_split_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }
  return pdaf48_set_split_info(isp_sub_module,
    event->u.module_event.module_event_data);
}


/** module_pdaf48_init:
 *
 *  @name: name of ISP module - "pdaf48"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for pdaf
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_pdaf48_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, PDAF48_MODULE_NAME(PDAF48_VERSION),
      strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      PDAF48_MODULE_NAME(PDAF48_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &pdaf48_private_func, ISP_MOD_PDAF, "pdaf", ISP_LOG_PDAF);
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

/** module_pdaf48_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_pdaf48_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }
  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init   = module_pdaf48_init,
  .module_deinit = module_pdaf48_deinit,
};

/** module_open:
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
