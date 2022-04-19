/* module_linearization40.c
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
#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_LINEARIZATION, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_LINEARIZATION, fmt, ##args)

#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "linearization40.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"

/* TODO pass from Android.mk */
#define LINEARIZATION40_VERSION "40"

#define LINEARIZATION40_MODULE_NAME(n) \
  "linearization"n

static boolean module_linearization40_init_data(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static void module_linearization40_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module);
static boolean module_linearization40_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_linearization40_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_linearization40_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_linearization40_set_flash_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_linearization40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_linearization40_update_sudmod_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);
static boolean module_linearization40_stats_aec_manual_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static isp_sub_module_private_func_t linearization40_private_func = {
  .isp_sub_module_init_data              = module_linearization40_init_data,
  .isp_sub_module_destroy                = module_linearization40_destroy,

  .control_event_handler = {
    [ISP_CONTROL_EVENT_STREAMON]         = isp_sub_module_port_streamon,
    [ISP_CONTROL_EVENT_STREAMOFF]        = module_linearization40_streamoff,
    [ISP_CONTROL_EVENT_SET_PARM]         = isp_sub_module_port_set_param,
  },

  .module_event_handler = {
    [ISP_MODULE_EVENT_SET_CHROMATIX_PTR] =
      module_linearization40_set_chromatix_ptr,
    [ISP_MODULE_EVENT_STATS_AEC_UPDATE]  =
      module_linearization40_stats_aec_update,
    [ISP_MODULE_EVENT_SET_FLASH_MODE]  =
      module_linearization40_set_flash_mode,
    [ISP_MODULE_EVENT_ISP_PRIVATE_EVENT] =
      isp_sub_module_port_isp_private_event,
    [ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE] =
      isp_set_hdr_mode,
    [ISP_MODULE_EVENT_ISP_DISABLE_MODULE] =
      isp_sub_module_port_disable_module,
    [ISP_MODULE_EVENT_STATS_AEC_MANUAL_UPDATE]  =
        module_linearization40_stats_aec_manual_update,
  },

  .isp_private_event_handler = {
    [ISP_PRIVATE_SET_MOD_ENABLE] = isp_sub_module_port_enable,
    [ISP_PRIVATE_SET_TRIGGER_ENABLE] = isp_sub_module_port_trigger_enable,
    [ISP_PRIVATE_SET_TRIGGER_UPDATE] = module_linearization40_trigger_update,
    [ISP_PRIVATE_CURRENT_SUBMOD_ENABLE] = module_linearization40_update_sudmod_enable,
  },

  .set_param_handler = {
    [ISP_SET_PARM_SET_VFE_COMMAND]  = isp_sub_module_port_set_vfe_command,
    [ISP_SET_META_BLACK_LEVEL_LOCK] = isp_sub_module_set_manual_controls,
    [ISP_SET_DUAL_LED_CALIBRATION]  = isp_sub_module_port_set_dual_led_calibration,
    [ISP_SET_PARM_UPDATE_DEBUG_LEVEL]   = isp_sub_module_port_set_log_level,
  },
};

/** module_linearization40_init_data:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the chroma_suppress module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
static boolean module_linearization40_init_data(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module)
{
  if (!module) {
    ISP_ERR("failed: %p", module);
    return FALSE;
  }
  return linearization40_init(isp_sub_module);
}

/** linearization40_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
static void module_linearization40_destroy(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module)
{
  if (!module) {
    ISP_ERR("failed: %p", module);
    return;
  }
  linearization40_destroy(isp_sub_module);
}

/** module_linearization40_stats_aec_manual_update:
 *
 * @mod: demosaic module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_linearization40_stats_aec_manual_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return linearization40_stats_aec_manual_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_linearization40_stats_aec_update:
 *
 * @mod: demosaic module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean module_linearization40_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return linearization40_stats_aec_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_linearization40_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_linearization40_streamoff(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }
  return linearization40_streamoff(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_linearization40_set_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_linearization40_set_chromatix_ptr(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }
  return linearization40_set_chromatix_ptr(isp_sub_module,
    event->u.module_event.module_event_data);
}

static boolean module_linearization40_update_sudmod_enable(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  boolean                   ret = TRUE;

  linearization40_t         *linearization     = NULL;
  uint8_t                   *submod_enable     = NULL;
  isp_private_event_t       *private_event     = NULL;
  chromatix_VFE_common_type *pchromatix_common = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  linearization = (linearization40_t *)isp_sub_module->private_data;
  private_event = event->u.module_event.module_event_data;

  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  if (!private_event->data) {
    ISP_ERR("failed: data %p", private_event->data);
    return FALSE;
  }

  submod_enable = (uint8_t *)private_event->data;
  pchromatix_common = (chromatix_VFE_common_type *)
    isp_sub_module->chromatix_ptrs.chromatixComPtr;

  if (!pchromatix_common) {
    ISP_ERR("failed NULL ptr pchromatix_common %p", pchromatix_common);
    return FALSE;
  }

  ISP_DBG("PEDESTAL mod_enable:%d  ctx_enable:%d",
    submod_enable[ISP_MOD_PEDESTAL],
    pchromatix_common->chromatix_pedestal_correction.pedestalcorrection_enable);
  /* Enable pedestal in linearization only if pedestal is enabled
     in both the VFE pipeline and also the chromatix */
  if (submod_enable[ISP_MOD_PEDESTAL] &&
      pchromatix_common->chromatix_pedestal_correction.
      pedestalcorrection_enable) {
    linearization->pedestal_enable =  TRUE;
  } else {
    linearization->pedestal_enable = FALSE;
  }

  ret = linearization40_update_base_tables(linearization,
    pchromatix_common);
  if (ret == FALSE) {
    ISP_ERR("failed !! linearization40_update_base_tables\n");
  }
  isp_sub_module->trigger_update_pending = TRUE;

  return TRUE;
}

/** module_linearization40_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_linearization40_trigger_update(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  return linearization40_trigger_update(isp_sub_module,
    event->u.module_event.module_event_data);
}

/** module_linearization40_init:
 *
 *  @name: name of ISP module - "linearization40"
 *
 *  Initializes new instance of ISP module
 *
 *  create mct module for linearization
 *
 *  Return mct module handle on success or NULL on failure
 **/
static mct_module_t *module_linearization40_init(const char *name)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  ISP_HIGH("name %s", name);

  if (!name) {
    ISP_ERR("failed: name %s", name);
    return NULL;
  }

  if (strncmp(name, LINEARIZATION40_MODULE_NAME(LINEARIZATION40_VERSION),
      strlen(name))) {
    ISP_ERR("failed: invalid name %s expected %s", name,
      LINEARIZATION40_MODULE_NAME(LINEARIZATION40_VERSION));
    return NULL;
  }

  module = isp_sub_module_init(name, NUM_SINK_PORTS, NUM_SOURCE_PORTS,
    &linearization40_private_func, ISP_MOD_LINEARIZATION, "linearization",
     ISP_LOG_LINEARIZATION);
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

/** module_linearization40_deinit:
 *
 *  @module: isp module handle
 *
 *  Deinit isp module
 *
 *  Returns: void
 **/
static void module_linearization40_deinit(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  isp_sub_module_deinit(module);
}

static isp_submod_init_table_t submod_init_table = {
  .module_init = module_linearization40_init,
  .module_deinit = module_linearization40_deinit,
};

/** module_open:
 *
 *  Return handle to isp_submod_init_table_t
 **/
isp_submod_init_table_t *module_open(void)
{
  return &submod_init_table;
}


/** module_linearization40_set_flash_mode:
 *
 * @mod: demosaic module
 * @isp_sub_module: isp sub module handle
 * @event: mct event handle
 *
 * Handle set flash mode event
 *
 * Return TRUE on success and FALSE on failure
 **/

static boolean module_linearization40_set_flash_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }
  return linearization40_set_flash_mode(isp_sub_module,
    event->u.module_event.module_event_data);
}
