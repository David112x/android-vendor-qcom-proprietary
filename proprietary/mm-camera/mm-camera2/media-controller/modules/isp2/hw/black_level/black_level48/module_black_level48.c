/* module_black_level48.c
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

/* isp headers */
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_sub_module_port.h"
#include "black_level48.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_BLSS, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_BLSS, fmt, ##args)

#define BLACK_LEVEL48_VERSION "48"

#define BLACK_LEVEL48_MODULE_NAME(n) \
  "black_level"n

static boolean module_black_level48_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_black_level48_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_black_level48_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_black_level48_manual_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_black_level48_set_digital_gain(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static isp_sub_module_private_func_t black_level48_private_func = {
  .isp_sub_module_init_data              = black_level48_init,
  .isp_sub_module_destroy                = black_level48_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = isp_sub_module_port_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = module_black_level48_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] = module_black_level48_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  = module_black_level48_stats_aec_update,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_SET_DIGITAL_GAIN] =
      module_black_level48_set_digital_gain,
    [ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE] =
      isp_set_hdr_mode,
    [ISP_MODULE_EVENT_STATS_AEC_MANUAL_UPDATE]  = module_black_level48_manual_aec_update,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = black_level48_trigger_update,
    [ISP_PRIVATE_FETCH_BLKLVL_OFFSET]    = black_level48_fetch_blklvl_offset,
  },

  .set_param_handler = {
    [ISP_SET_PARM_SET_VFE_COMMAND]  = isp_sub_module_port_set_vfe_command,
    [ISP_SET_META_BLACK_LEVEL_LOCK] = isp_sub_module_set_manual_controls,
  },
};

/** module_black_level48_stats_aec_update:
 *
 * @mod: demosaic module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_black_level48_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return black_level48_stats_aec_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_black_level48_manual_aec_update:
 * Handle Manual AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_black_level48_manual_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return black_level48_manual_aec_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_black_level48_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_black_level48_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event || !module) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, event, module);
    return FALSE;
  }
  return black_level48_streamoff(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_black_level48_set_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_black_level48_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event || !module) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, event, module);
    return FALSE;
  }
  return black_level48_set_chromatix_ptr(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_black_level48_set_digital_gain:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_black_level48_set_digital_gain(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!isp_sub_module || !event || !module) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, event, module);
    return FALSE;
  }
  return black_level48_set_digital_gain(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_black_level48_init:
 *
 *  @name: name of ISP module - "black_level48"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for black_level
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_black_level48_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  if (!name) {
    ISP_ERR("failed name %p", name);
  }
  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, BLACK_LEVEL48_MODULE_NAME(BLACK_LEVEL48_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      BLACK_LEVEL48_MODULE_NAME(BLACK_LEVEL48_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &black_level48_private_func, ISP_MOD_BLS, "blss", ISP_LOG_BLSS);
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

/** module_black_level48_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_black_level48_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_black_level48_init,
  .module_deinit = module_black_level48_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
