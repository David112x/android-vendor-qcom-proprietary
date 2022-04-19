/* module_abcc44.c
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
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
#include "isp_common.h"
#include "isp_log.h"
#include "abcc44.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

#define NUM_SINK_PORTS   1

/* TODO pass from Android.mk */
#define ABCC44_VERSION "44"

#define ABCC44_MODULE_NAME(n) \
  "abcc"n

static isp_sub_module_private_func_t abcc44_private_func = {
  .isp_sub_module_init_data              = abcc44_init,
  .isp_sub_module_destroy                = abcc44_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = abcc44_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = isp_sub_module_port_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_SET_DEFECTIVE_PIXELS] =
      abcc44_set_defective_pixel_for_abcc,
    [ISP_MODULE_EVENT_SET_STREAM_CONFIG] =
      abcc44_set_stream_config,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = abcc44_trigger_update,
  },

  .set_param_handler = {
    [ISP_SET_PARM_SET_VFE_COMMAND] = isp_sub_module_port_set_vfe_command,
  },
};

/** module_abcc44_init:
 *
 *  @name: name of ISP module - "abcc44"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for bpc
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_abcc44_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, ABCC44_MODULE_NAME(ABCC44_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      ABCC44_MODULE_NAME(ABCC44_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &abcc44_private_func, ISP_MOD_ABCC, "abcc", ISP_LOG_ABCC);
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

/** module_abcc44_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_abcc44_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_abcc44_init,
  .module_deinit = module_abcc44_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
