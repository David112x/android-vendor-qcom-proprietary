/* module_ltm47.c
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

/* isp headers */
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "ltm47.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_LTM, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_LTM, fmt, ##args)

/* TODO pass from Android.mk */
#define LTM47_VERSION "47"

#define LTM47_MODULE_NAME(n) \
  "ltm"n
static boolean module_ltm47_calculate_min_overlap(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);

static boolean module_ltm47_set_stripe_info(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event);

static boolean module_ltm47_set_split_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static boolean module_ltm47_set_frame_skip(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static boolean module_ltm47_set_flash_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static boolean module_ltm47_set_longshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static isp_sub_module_private_func_t ltm47_private_func = {
  .isp_sub_module_init_data              = ltm47_init,
  .isp_sub_module_destroy                = ltm47_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = ltm47_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = ltm47_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_STREAM_CONFIG] = ltm47_set_stream_config,
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] = ltm47_handle_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  = ltm47_stats_aec_update,
    [ISP_MODULE_EVENT_MANUAL_AWB_UPDATE] = ltm47_stats_aec_update,
    [ISP_MODULE_EVENT_STATS_ASD_UPDATE]  = ltm47_stats_asd_update,
    [ISP_MODULE_EVENT_LTM_ALGO_UPDATE]   = ltm47_algo_curve_update,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_SET_FLASH_MODE]    = module_ltm47_set_flash_mode,
    [ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE] = isp_set_hdr_mode,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
    [ISP_MODULE_EVENT_OFFLINE_CONFIG_OVERWRITE] =
       ltm47_set_stream_config_overwrite,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE] = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE] = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE] = ltm47_trigger_update,
    [ISP_PRIVATE_REQUEST_STRIPE_OFFSET] = module_ltm47_calculate_min_overlap,
    [ISP_PRIVATE_SET_STRIPE_INFO] = module_ltm47_set_stripe_info,
    [ISP_PRIVATE_SET_STREAM_SPLIT_INFO] = module_ltm47_set_split_info,
    [ISP_PRIVATE_REQUEST_FRAME_SKIP] = module_ltm47_set_frame_skip,
  },

  .set_param_handler = {
    [ISP_SET_PARM_SET_VFE_COMMAND] = isp_sub_module_port_set_vfe_command,
    [ISP_SET_PARM_EFFECT]          = ltm47_set_spl_effect,
    [ISP_SET_META_TONEMAP_MODE]    = isp_sub_module_set_manual_controls,
    [ISP_SET_PARM_LONGSHOT_ENABLE] = module_ltm47_set_longshot,
    [ISP_SET_PARM_HDR]             = ltm47_set_hdr,
  },
};


/** module_ltm47_calculate_min_overlap:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_ltm47_calculate_min_overlap(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  return ltm47_calculate_min_overlap(isp_sub_module,
    event->u.module_event.module_event_data);
}


/** module_ltm47_set_stripe_info:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_ltm47_set_stripe_info(
  mct_module_t     *module,
  isp_sub_module_t *isp_sub_module,
  mct_event_t      *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  return ltm47_set_stripe_info(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_ltm47_set_split_info:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_ltm47_set_split_info(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  return ltm47_set_split_info(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_ltm47_set_frame_skip:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function tells frame it want AXI to skip
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_ltm47_set_frame_skip(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  return ltm47_set_frame_skip(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_ltm47_set_flash_mode:
 *
 * @mod: ltm module
 * @isp_sub_module: isp sub module handle
 * @event: mct event handle
 *
 * Handle set flash mode event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_ltm47_set_flash_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return ltm47_set_flash_mode(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_ltm47_set_longshot:
 *
 * @mod: ltm module
 * @isp_sub_module: isp sub module handle
 * @event: mct event handle
 *
 * Handle set flash mode event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_ltm47_set_longshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return ltm47_set_longshot(isp_sub_module,
    event->u.module_event.module_event_data);
}


/** module_ltm47_init:
 *
 *  @name: name of ISP module - "ltm47"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for ltm
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_ltm47_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, LTM47_MODULE_NAME(LTM47_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      LTM47_MODULE_NAME(LTM47_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &ltm47_private_func, ISP_MOD_LTM, "ltm", ISP_LOG_LTM);
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

/** module_ltm47_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_ltm47_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_ltm47_init,
  .module_deinit = module_ltm47_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
