/* module_snr47.c
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
#include "isp_sub_module_log.h"
#include "snr47.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_SNR, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_SNR, fmt, ##args)

static boolean module_snr47_init_data(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static void module_snr47_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static boolean module_snr47_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_snr47_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_snr47_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_snr47_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_snr47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_snr47_update_submod_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static isp_sub_module_private_func_t snr47_private_func = {
  .isp_sub_module_init_data              = module_snr47_init_data,
  .isp_sub_module_destroy                = module_snr47_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = module_snr47_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = module_snr47_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      module_snr47_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  =
      module_snr47_aec_update,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE]         = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE]     = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE]     = module_snr47_trigger_update,
    [ISP_PRIVATE_CURRENT_SUBMOD_ENABLE]  = module_snr47_update_submod_enable,
  },

  .set_param_handler = {
    [ISP_SET_META_NOISE_REDUCTION_MODE] = isp_sub_module_set_manual_controls,
    [ISP_SET_PARM_ZSL_MODE]             = isp_sub_module_port_set_zsl_mode,
  },

};

/** module_snr47_init_data:
 *
 *  @name: name of ISP module - "snr47"
 *
 *  Return mct module handle on success or NULL on failure
 **/
static boolean module_snr47_init_data(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return FALSE;
  }

  return snr47_init(isp_sub_module);
}

/** module_snr47_destroy:
 *
 *  @name: name of ISP module - "snr47"
 *
 *  Return mct module handle on success or NULL on failure
 **/
static void module_snr47_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return;
  }

  snr47_destroy(isp_sub_module);
  return;
}

/** module_snr47_streamon:
 *
 *  @name: name of ISP module - "snr47"
 *
 *  Return mct module handle on success or NULL on failure
 **/
static boolean module_snr47_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  return snr47_streamon(isp_sub_module);
}

/** module_snr47_streamoff:
 *
 *  @name: name of ISP module - "snr47"
 *
 *  Return mct module handle on success or NULL on failure
 **/
static boolean module_snr47_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  return snr47_streamoff(isp_sub_module);
}
/** module_snr47_set_chromatix_ptr:
 *
 *  @name: name of ISP module - "snr47"
 *
 *  Return mct module handle on success or NULL on failure
 **/
static boolean module_snr47_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  return snr47_set_chromatix_ptr(isp_sub_module,
    (modulesChromatix_t *)event->u.module_event.module_event_data);
}

/** module_snr47_aec_update:
 *
 *  @name: name of ISP module - "snr47"
 *
 **/
static boolean module_snr47_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  return snr47_aec_update(isp_sub_module,
    (stats_update_t *)event->u.module_event.module_event_data);
}

/** module_snr47_trigger_update:
 *
 *  @name: name of ISP module - "snr47"
 *
 *  Return mct module handle on success or NULL on failure
 **/
static boolean module_snr47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  return snr47_trigger_update(isp_sub_module,
    (isp_private_event_t *)event->u.module_event.module_event_data);
}

/** module_snr47_update_submod_enable:
 *
 *  @name: name of ISP module - "snr47"
 *
 *  Return mct module handle on success or NULL on failure
 **/
static boolean module_snr47_update_submod_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  return snr47_update_submod_enable(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_snr47_init:
 *
 *  @name: name of ISP module - "snr47"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for snr
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_snr47_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, SNR_NAME(SNR47_VERSION), strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      SNR_NAME(SNR47_VERSION));
    return NULL;
  }

  if (!strncmp(name, SNR_NAME(SNR47_VERSION), strlen(name))) {
    module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
      &snr47_private_func, ISP_MOD_SNR, "snr", ISP_LOG_SNR);
  }
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

/** module_snr47_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_snr47_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_snr47_init,
  .module_deinit = module_snr47_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}
