/* module_color_correct40.c
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

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COLOR_CORRECT, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COLOR_CORRECT, fmt, ##args)

/* isp headers */
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "module_color_correct40.h"
#include "color_correct40.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

/* TODO pass from Android.mk */
#define COLOR_CORRECT40_VERSION "40"

#define COLOR_CORRECT40_MODULE_NAME(n) \
  "color_correct"n

static boolean module_color_correct40_set_flash_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static isp_sub_module_private_func_t color_correct40_private_func = {
  .isp_sub_module_init_data = color_correct40_init,
  .isp_sub_module_destroy   = color_correct40_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]       = color_correct40_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]      = color_correct40_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]       = isp_sub_module_port_set_param,
    [ISP_CONTROL_EVENT_SET_SUPER_PARM] = isp_sub_module_port_set_super_param,
  },

  .module_event_handler = {
    /*common*/
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      isp_sub_module_port_set_chromatix_ptr,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    /*color correction specific*/
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE] = color_correct40_save_aec_params,
    [ISP_MODULE_EVENT_STATS_AWB_UPDATE] = color_correct40_save_awb_params,
    [ISP_MODULE_EVENT_MANUAL_AWB_UPDATE] = color_correct40_save_awb_params,
    [ISP_MODULE_EVENT_SET_FLASH_MODE]  = module_color_correct40_set_flash_mode,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
  },

  .isp_private_event_handler = {
    /*common*/
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    /*color correction specific*/
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = color_correct40_trigger_update,
    [ISP_PRIVATE_GET_VFE_DIAG_INFO_USER] =
      color_correct40_get_vfe_diag_info_user,
  },
  .set_param_handler = {
    /*chroma enhance specific*/
    [ISP_SET_PARM_SATURATION]            = color_correct40_set_effect,
    [ISP_SET_PARM_BESTSHOT_MODE]         = color_correct40_set_bestshot,
    [ISP_SET_PARM_SET_VFE_COMMAND]       = isp_sub_module_port_set_vfe_command,
    [ISP_SET_META_MODE]                  = isp_sub_module_set_manual_controls,
    [ISP_SET_META_COLOR_CORRECT_MODE]    = isp_sub_module_set_manual_controls,
    [ISP_SET_PARM_WHITE_BALANCE]         = isp_sub_module_set_manual_controls,
    [ISP_SET_META_COLOR_CORRECT_TRANSFORM] = isp_sub_module_set_manual_controls,
  },
};

/** module_color_correct40_init:
 *
 *  @name: name of ISP module - "color_correct40"
 *
 * Initializes new instance of ISP module
 *
 * create mct module for color correct
 *
 * Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_color_correct40_init(const char *name)
{
  boolean            ret = TRUE;
  mct_module_t      *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_INFO("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, COLOR_CORRECT40_MODULE_NAME(COLOR_CORRECT40_VERSION),
    strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      COLOR_CORRECT40_MODULE_NAME(COLOR_CORRECT40_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &color_correct40_private_func, ISP_MOD_COLOR_CORRECT, "color_correct",
    ISP_LOG_COLOR_CORRECT);
  if (!module) {
    ISP_ERR("module %p", module);
    return NULL;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    goto ERROR1;
  }

  isp_sub_module_priv->private_func = &color_correct40_private_func;

  return module;

ERROR1:
  mct_module_destroy(module);
  ISP_ERR("failed");
  return NULL;
}

/** module_color_correct40_set_flash_mode:
 *
 * @mod: color correct module
 * @isp_sub_module: isp sub module handle
 * @event: mct event handle
 *
 * Handle set flash mode event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_color_correct40_set_flash_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  return color_correct40_set_flash_mode(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_color_correct40_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_color_correct40_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_color_correct40_init,
  .module_deinit = module_color_correct40_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
