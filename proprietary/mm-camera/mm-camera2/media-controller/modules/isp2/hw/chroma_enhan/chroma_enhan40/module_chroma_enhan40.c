/* module_chroma_enhan40.c
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
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
#include "module_chroma_enhan40.h"
#include "chroma_enhan40.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"
#include "chroma_enhan40.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_CHROMA_ENHANCE, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_CHROMA_ENHANCE, fmt, ##args)

/* TODO pass from Android.mk */
#define CHROMA_ENHAN40_VERSION "40"

#define CHROMA_ENHAN40_MODULE_NAME(n) \
  "chroma_enhan"n

static isp_sub_module_private_func_t chroma_enhan40_private_func = {
  .isp_sub_module_init_data = chroma_enhan40_init,
  .isp_sub_module_destroy = chroma_enhan40_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]       = chroma_enhan40_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]      = chroma_enhan40_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]       = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    /*common*/
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      chroma_enhan_set_chromatix_ptr,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    /*chroma enhna specific*/
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  = chroma_enhan40_save_aec_params,
    [ISP_MODULE_EVENT_STATS_AWB_UPDATE]  = chroma_enhan40_save_awb_params,
    [ISP_MODULE_EVENT_MANUAL_AWB_UPDATE] = chroma_enhan40_save_awb_params,
    [ISP_MODULE_EVENT_SET_FLASH_MODE]    = chroma_enhan40_set_flash_mode,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
  },

  .isp_private_event_handler = {
    /*common*/
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    /*chroma enhance specific*/
    [ISP_PRIVATE_GET_VFE_DIAG_INFO_USER] =
      chroma_enhan40_get_vfe_diag_info_user,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = chroma_enhan40_trigger_update,
  },
  .set_param_handler = {
    /*chroma enhance specific*/
    [ISP_SET_PARM_SATURATION]            = chroma_enhan_set_effect,
    [ISP_SET_PARM_BESTSHOT_MODE]         = chroma_enhan_set_bestshot,
    [ISP_SET_PARM_EFFECT]                = chroma_enhan_set_spl_effect,
    [ISP_SET_PARM_SET_VFE_COMMAND]       = isp_sub_module_port_set_vfe_command,
    [ISP_SET_PARM_UPDATE_DEBUG_LEVEL]   = isp_sub_module_port_set_log_level,
  },

};

/** module_chroma_enhan40_init:
 *
 *  @name: name of ISP module - "chroma_enhan40"
 *
 * Initializes new instance of ISP module
 *
 * create mct module for chroma enhancement
 *
 * Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_chroma_enhan40_init(const char *name)
{
  boolean            ret = TRUE;
  mct_module_t      *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_DBG("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, CHROMA_ENHAN40_MODULE_NAME(CHROMA_ENHAN40_VERSION),
    strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      CHROMA_ENHAN40_MODULE_NAME(CHROMA_ENHAN40_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &chroma_enhan40_private_func, ISP_MOD_CHROMA_ENHANCE,
    "chroma_enhance", ISP_LOG_CHROMA_ENHANCE);
  if (!module) {
    ISP_ERR("module %p", module);
    return NULL;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    goto ERROR1;
  }

  isp_sub_module_priv->private_func = &chroma_enhan40_private_func;

  return module;

ERROR1:
  mct_module_destroy(module);
  ISP_ERR("failed");
  return NULL;
}

/** module_chroma_enhan40_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_chroma_enhan40_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_chroma_enhan40_init,
  .module_deinit = module_chroma_enhan40_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
