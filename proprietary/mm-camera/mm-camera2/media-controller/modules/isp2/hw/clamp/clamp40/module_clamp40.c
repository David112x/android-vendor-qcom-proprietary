/* module_clamp40.c
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc. All Rights Reserved.
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
  ISP_DBG_MOD(ISP_LOG_CLAMP, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_CLAMP, fmt, ##args)

/* isp headers */
#include "isp_sub_module_log.h"
#include "clamp40.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

static isp_sub_module_private_func_t clamp40_private_func = {
  .isp_sub_module_init_data              = clamp40_init,
  .isp_sub_module_destroy                = clamp40_destroy,

  .module_event_handler = {
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_SET_STREAM_CONFIG]  =
      clamp40_set_stream_config,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_TRIGGER_UPDATE] = clamp40_trigger_update,
  },
  .set_param_handler = {
    [ISP_SET_PARM_UPDATE_DEBUG_LEVEL]   = isp_sub_module_port_set_log_level,
  },
};

/** module_clamp40_init:
 *
 *  @name: name of ISP module - "clamp40"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for clamp
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_clamp40_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;
  isp_hw_module_id_t     hw_module_id = ISP_MOD_CLAMP_VIEWFINDER;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, CLAMP40_VIEWFINDER_NAME(CLAMP40_VERSION), strlen(name)) &&
      strncmp(name, CLAMP40_ENCODER_NAME(CLAMP40_VERSION), strlen(name))    &&
      strncmp(name, CLAMP40_VIDEO_NAME(CLAMP40_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s or %s or %s", name,
      CLAMP40_VIEWFINDER_NAME(CLAMP40_VERSION),
      CLAMP40_ENCODER_NAME(CLAMP40_VERSION),
      CLAMP40_VIDEO_NAME(CLAMP40_VERSION));
    return NULL;
  }

  if (!strncmp(name, CLAMP40_VIEWFINDER_NAME(CLAMP40_VERSION), strlen(name))){
    hw_module_id = ISP_MOD_CLAMP_VIEWFINDER;
  } else if (!strncmp(name, CLAMP40_ENCODER_NAME(CLAMP40_VERSION), strlen(name))) {
    hw_module_id = ISP_MOD_CLAMP_ENCODER;
  } else if (!strncmp(name, CLAMP40_VIDEO_NAME(CLAMP40_VERSION), strlen(name))) {
    hw_module_id = ISP_MOD_CLAMP_VIDEO;
  }


  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &clamp40_private_func, hw_module_id, "clamp", ISP_LOG_CLAMP);
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

/** module_clamp40_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_clamp40_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_clamp40_init,
  .module_deinit = module_clamp40_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
