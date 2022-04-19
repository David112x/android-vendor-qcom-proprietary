/* module_ltm44.c
 *
 * Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
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
  ISP_DBG_MOD(ISP_LOG_LTM, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_LTM, fmt, ##args)

/* isp headers */
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "ltm44.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

/* TODO pass from Android.mk */
#define LTM44_VERSION "44"

#define LTM44_MODULE_NAME(n) \
  "ltm"n
static boolean module_ltm44_calculate_min_overlap(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);

static boolean module_ltm44_set_stripe_info(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);

static boolean module_ltm44_set_split_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static boolean module_ltm44_set_frame_skip(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static boolean module_ltm44_set_flash_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static boolean module_ltm44_set_longshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static isp_sub_module_private_func_t ltm44_private_func = {
  .isp_sub_module_init_data              = ltm44_init,
  .isp_sub_module_destroy                = ltm44_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = ltm44_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = ltm44_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_STREAM_CONFIG] = ltm44_set_stream_config,
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] = ltm44_handle_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  = ltm44_stats_aec_update,
    [ISP_MODULE_EVENT_STATS_ASD_UPDATE]  = ltm44_stats_asd_update,
    [ISP_MODULE_EVENT_LTM_ALGO_UPDATE]   = ltm44_algo_curve_update,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_SET_FLASH_MODE]  = module_ltm44_set_flash_mode,
    [ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE] = isp_set_hdr_mode,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
    [ISP_MODULE_EVENT_OFFLINE_CONFIG_OVERWRITE] =
       ltm44_set_stream_config_overwrite,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE] = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE] = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE] = ltm44_trigger_update,
    [ISP_PRIVATE_REQUEST_STRIPE_OFFSET] = module_ltm44_calculate_min_overlap,
    [ISP_PRIVATE_SET_STRIPE_INFO] = module_ltm44_set_stripe_info,
    [ISP_PRIVATE_SET_STREAM_SPLIT_INFO] = module_ltm44_set_split_info,
    [ISP_PRIVATE_REQUEST_FRAME_SKIP] = module_ltm44_set_frame_skip,
  },

  .set_param_handler = {
    [ISP_SET_PARM_SET_VFE_COMMAND] = isp_sub_module_port_set_vfe_command,
    [ISP_SET_INTF_PARM_LTM_MODE]   = ltm44_set_ltm_enable,
    [ISP_SET_META_TONEMAP_MODE]    = isp_sub_module_set_manual_controls,
    [ISP_SET_PARM_UPDATE_DEBUG_LEVEL]   = isp_sub_module_port_set_log_level,
    [ISP_SET_PARM_LONGSHOT_ENABLE] = module_ltm44_set_longshot,
  },
};


/** module_ltm44_calculate_min_overlap:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_ltm44_calculate_min_overlap(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }

  return ltm44_calculate_min_overlap(isp_sub_module,
    event->u.module_event.module_event_data);
}


/** module_ltm44_set_stripe_info:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_ltm44_set_stripe_info(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }

  return ltm44_set_stripe_info(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_ltm44_set_split_info:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_ltm44_set_split_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }
  return ltm44_set_split_info(isp_sub_module,
    event->u.module_event.module_event_data);
}


/** module_ltm44_set_frame_skip:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function tells frame it want AXI to skip
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_ltm44_set_frame_skip(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event) {
    ISP_ERR("failed: isp_sub_module %p event %p", isp_sub_module, event);
    return FALSE;
  }
  return ltm44_set_frame_skip(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_ltm44_set_flash_mode:
 *
 * @mod: ltm module
 * @isp_sub_module: isp sub module handle
 * @event: mct event handle
 *
 * Handle set flash mode event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_ltm44_set_flash_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return ltm44_set_flash_mode(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_ltm44_set_longshot:
 *
 * @mod: ltm module
 * @isp_sub_module: isp sub module handle
 * @event: mct event handle
 *
 * Handle set flash mode event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_ltm44_set_longshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return ltm44_set_longshot(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_ltm44_init:
 *
 *  @name: name of ISP module - "ltm44"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for ltm
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_ltm44_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, LTM44_MODULE_NAME(LTM44_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      LTM44_MODULE_NAME(LTM44_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &ltm44_private_func, ISP_MOD_LTM, "ltm", ISP_LOG_LTM);
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

/** module_ltm44_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_ltm44_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_ltm44_init,
  .module_deinit = module_ltm44_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
