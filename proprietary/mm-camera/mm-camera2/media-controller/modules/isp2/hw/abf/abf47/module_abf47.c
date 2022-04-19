/* module_abf47.c
 *
 * Copyright (c) 2014-2017 Qualcomm Technologies, Inc. All Rights Reserved.
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
#include "abf47.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module_util.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_ABF, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_ABF, fmt, ##args)

#define ABF_ENABLE_BIT 31

/* TODO pass from Android.mk */
#define ABF47_VERSION "47"

#define ABF_MODULE_NAME(n) \
  "abf"n

static boolean module_abf47_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static boolean module_abf47_module_cfg(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static isp_sub_module_private_func_t abf47_private_func = {
  .isp_sub_module_init_data              = abf47_init,
  .isp_sub_module_destroy                = abf47_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = abf47_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = abf47_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      abf47_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  =
      abf47_aec_update,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_SET_STREAM_CONFIG] =
      module_abf47_set_stream_config,
    [ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE] =
      isp_set_hdr_mode,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
    [ISP_MODULE_EVENT_STATS_AEC_MANUAL_UPDATE]  =
      abf47_aec_update,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = abf47_trigger_update,
    [ISP_PRIVATE_SET_STRIPE_INFO]        = abf47_set_stripe_info,
    [ISP_PRIVATE_SET_STREAM_SPLIT_INFO]  = abf47_set_split_info,
    [ISP_PRIVATE_EVENT_GET_MOD_CFG_MASK] = module_abf47_module_cfg,
  },

  .set_param_handler = {
    [ISP_SET_META_NOISE_REDUCTION_MODE] = isp_sub_module_set_manual_controls,
    [ISP_SET_PARM_SET_VFE_COMMAND] = isp_sub_module_port_set_vfe_command,
    [ISP_SET_PARM_ZSL_MODE]        = isp_sub_module_port_set_zsl_mode,
  },

};

/** module_abf47_init:
 *
 *  @name: name of ISP module - "abf47"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for abf
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_abf47_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, ABF_MODULE_NAME(ABF47_VERSION),
      strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      ABF_MODULE_NAME(ABF47_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &abf47_private_func, ISP_MOD_ABF, "abf", ISP_LOG_ABF);
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

/** module_abf47_module_cfg:
 *
 *  @module: abf47 module pointer
 *  @isp_sub_module: isp_sub_module pointer
 *  @event: event pointer
 *
 *  Checks if hystersis needs to be enabled
 *
 *  create mct module for abf
 *
 *  Return mct module handle on success or NULL on failure
 **/
static boolean module_abf47_module_cfg(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  abf47_t *abf = NULL;
  boolean ret  = TRUE;

  if (!module || !event || !isp_sub_module || !isp_sub_module->private_data) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  abf = (abf47_t*)isp_sub_module->private_data;

  ret = abf47_decide_hysterisis(isp_sub_module, abf->aec_reference);

  ret = isp_sub_module_util_get_cfg_mask(isp_sub_module, event);

  return ret;
}

/** module_abf47_set_stream_config:
 *
 * @mod: demosaic module
 * @isp_sub_module: handle to isp_sub_module_t
 * @event: event to be handled
 *
 * Handle set stream config event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_abf47_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return abf47_set_stream_config(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_abf47_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_abf47_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_abf47_init,
  .module_deinit = module_abf47_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
