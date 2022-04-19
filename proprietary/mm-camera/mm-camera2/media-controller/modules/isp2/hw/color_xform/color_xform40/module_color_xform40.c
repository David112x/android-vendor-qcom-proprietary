/* module_color_xform40.c
 *
 * Copyright (c) 2014444 Qualcomm Technologies, Inc. All Rights Reserved.
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
  ISP_DBG_MOD(ISP_LOG_COLOR_XFORM, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COLOR_XFORM, fmt, ##args)

/* isp headers */
#include "isp_sub_module_log.h"
#include "color_xform40.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

static isp_sub_module_private_func_t color_xform40_private_func = {
  .isp_sub_module_init_data              = color_xform40_init,
  .isp_sub_module_destroy                = color_xform40_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .set_param_handler = {
    [ISP_SET_PARM_UPDATE_DEBUG_LEVEL]   = isp_sub_module_port_set_log_level,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      isp_sub_module_port_set_chromatix_ptr,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_TRIGGER_UPDATE] =
      color_xform40_trigger_update,
    [ISP_PRIVATE_SCALER_OUTPUT_UPDATE] =
      color_xform40_scaler_output_update,
  },
};

/** module_color_xform40_init:
 *
 *  @name: name of ISP module - "color_xform40"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for color_xform
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_color_xform40_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;
  isp_hw_module_id_t     hw_module_id = ISP_MOD_MAX_NUM;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, COLOR_XFORM40_VIEWFINDER_NAME(COLOR_XFORM40_VERSION), strlen(name)) &&
      strncmp(name, COLOR_XFORM40_ENCODER_NAME(COLOR_XFORM40_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s or %s", name,
      COLOR_XFORM40_VIEWFINDER_NAME(COLOR_XFORM40_VERSION),
      COLOR_XFORM40_ENCODER_NAME(COLOR_XFORM40_VERSION));
    return NULL;
  }

  if (!strncmp(name, COLOR_XFORM40_VIEWFINDER_NAME(COLOR_XFORM40_VERSION),
    strlen(name))) {
     hw_module_id = ISP_MOD_COLOR_XFORM_VIEWFINDER;
  } else if (!strncmp(name, COLOR_XFORM40_ENCODER_NAME(COLOR_XFORM40_VERSION),
     strlen(name))) {
     hw_module_id = ISP_MOD_COLOR_XFORM_ENCODER;
  }
  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &color_xform40_private_func, hw_module_id, "color_xform",
    ISP_LOG_COLOR_XFORM);
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

/** module_color_xform40_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_color_xform40_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_color_xform40_init,
  .module_deinit = module_color_xform40_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
