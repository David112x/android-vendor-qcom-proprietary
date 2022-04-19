/* isp_sub_module_port.c
 *
 * Copyright (c) 2012-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <stdio.h>
#include <pthread.h>

/* mctl headers */
#include "media_controller.h"
#include "mct_stream.h"
#include "eztune_vfe_diagnostics.h"

/* ISP headers*/
#include "isp_log.h"
#include "isp_defs.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module_util.h"
#include "isp_sub_module_common.h"

static boolean isp_sub_module_port_process_downstream_module_event(
  mct_module_t *module, mct_port_t *port,
  isp_sub_module_private_func_t *private_func,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

boolean isp_sub_module_port_set_param(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event);

static const isp_module_event_type_t
  module_event_mapping[MCT_EVENT_MODULE_MAX] = {
  [MCT_EVENT_MODULE_SET_STREAM_CONFIG] = ISP_MODULE_EVENT_SET_STREAM_CONFIG,
  [MCT_EVENT_MODULE_ISP_CDS_REQUEST] = ISP_MODULE_EVENT_CDS_REQUEST,
  [MCT_EVENT_MODULE_SET_STREAM_CONFIG_FOR_FLASH] = ISP_MODULE_EVENT_SET_STREAM_CONFIG_FOR_FLASH,
  [MCT_EVENT_MODULE_SET_FLASH_MODE] = ISP_MODULE_EVENT_SET_FLASH_MODE,
  [MCT_EVENT_MODULE_STATS_ASD_UPDATE] = ISP_MODULE_EVENT_STATS_ASD_UPDATE,
  [MCT_EVENT_MODULE_ISP_LA_ALGO_UPDATE] = ISP_MODULE_EVENT_LA_ALGO_UPDATE,
  [MCT_EVENT_MODULE_ISP_LTM_ALGO_UPDATE] = ISP_MODULE_EVENT_LTM_ALGO_UPDATE,
  [MCT_EVENT_MODULE_ISP_GTM_ALGO_UPDATE] = ISP_MODULE_EVENT_GTM_ALGO_UPDATE,
  [MCT_EVENT_MODULE_STATS_UPDATE] = ISP_MODULE_EVENT_STATS_UPDATE,
  [MCT_EVENT_MODULE_STATS_DIS_UPDATE] = ISP_MODULE_EVENT_STATS_DIS_UPDATE,
  [MCT_EVENT_MODULE_STATS_AEC_CONFIG_UPDATE] = ISP_MODULE_EVENT_STATS_AEC_CONFIG_UPDATE,
  [MCT_EVENT_MODULE_STATS_AWB_CONFIG_UPDATE] = ISP_MODULE_EVENT_STATS_AWB_CONFIG_UPDATE,
  [MCT_EVENT_MODULE_STATS_AF_CONFIG_UPDATE] = ISP_MODULE_EVENT_STATS_AF_CONFIG_UPDATE,
  [MCT_EVENT_MODULE_STATS_RS_CONFIG_UPDATE] = ISP_MODULE_EVENT_STATS_RS_CONFIG_UPDATE,
  [MCT_EVENT_MODULE_SET_DIGITAL_GAIN] = ISP_MODULE_EVENT_SET_DIGITAL_GAIN,
  [MCT_EVENT_MODULE_STATS_AWB_MANUAL_UPDATE] = ISP_MODULE_EVENT_MANUAL_AWB_UPDATE,
  [MCT_EVENT_MODULE_GET_ISP_TABLES] = ISP_MODULE_EVENT_GET_ISP_TABLES,
  [MCT_EVENT_MODULE_SET_FAST_AEC_CONVERGE_MODE] = ISP_MODULE_EVENT_SET_FAST_AEC_CONVERGE_MODE,
  [MCT_EVENT_MODULE_SET_CHROMATIX_PTR] = ISP_MODULE_EVENT_SET_CHROMATIX_PTR,
  [MCT_EVENT_MODULE_SET_RELOAD_CHROMATIX] = ISP_MODULE_EVENT_SET_CHROMATIX_PTR,
  [MCT_EVENT_MODULE_STATS_AEC_UPDATE] = ISP_MODULE_EVENT_STATS_AEC_UPDATE,
  [MCT_EVENT_MODULE_STATS_AWB_UPDATE] = ISP_MODULE_EVENT_STATS_AWB_UPDATE,
  [MCT_EVENT_MODULE_ISP_PRIVATE_EVENT] = ISP_MODULE_EVENT_ISP_PRIVATE_EVENT,
  [MCT_EVENT_MODULE_SET_AF_ROLLOFF_PARAMS] = ISP_MODULE_EVENT_SET_AF_ROLLOFF_PARAMS,
  [MCT_EVENT_MODULE_SENSOR_LENS_POSITION_UPDATE] = ISP_MODULE_EVENT_SENSOR_LENS_POSITION_UPDATE,
  [MCT_EVENT_MODULE_ISP_TINTLESS_ALGO_UPDATE] = ISP_MODULE_EVENT_TINTLESS_ALGO_UPDATE,
  [MCT_EVENT_MODULE_ISP_OFFLINE_CONFIG_OVERWRITE] = ISP_MODULE_EVENT_OFFLINE_CONFIG_OVERWRITE,
  [MCT_EVENT_MODULE_SET_DEFECTIVE_PIXELS] = ISP_MODULE_EVENT_SET_DEFECTIVE_PIXELS,
  [MCT_EVENT_MODULE_REQUEST_STATS_TYPE] = ISP_MODULE_EVENT_REQUEST_STATS_TYPE,
  [MCT_EVENT_MODULE_ISP_HDR] = ISP_MODULE_EVENT_SET_SENSOR_HDR_MODE,
  [MCT_EVENT_MODULE_SENSOR_PDAF_CONFIG] = ISP_MODULE_EVENT_SET_PDAF_PATTERN,
  [MCT_EVENT_MODULE_ISP_DISABLE_MODULE] = ISP_MODULE_EVENT_ISP_DISABLE_MODULE,
  [MCT_EVENT_MODULE_AF_EXP_COMPENSATE] = ISP_MODULE_EVENT_AF_EXP_COMPENSATE,
  [MCT_EVENT_MODULE_STATS_FOVC_MAGNIFICATION_FACTOR_ISP] = ISP_MODULE_EVENT_STATS_FOVC_MAGNIFICATION_FACTOR,
  [MCT_EVENT_MODULE_STATS_AEC_MANUAL_UPDATE] = ISP_MODULE_EVENT_STATS_AEC_MANUAL_UPDATE,
};

/** isp_sub_module_port_set_chromatix_ptr
 *
 *  @module: mct module handle
 *  @isp_sub_mdoule: isp sub module handle
 *  @event: event handle
 *
 *  Update chromatix ptr
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_port_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  modulesChromatix_t *chromatix_ptrs = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  chromatix_ptrs =
    (modulesChromatix_t *)event->u.module_event.module_event_data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;
  isp_sub_module->trigger_update_pending = TRUE;
  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** isp_sub_module_port_trigger_enable:
 *
 *  @demosaic: demosaic module instance
 *  @enable: true if triger update is enabled
 *  @in_param_size: enable parameter size
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function enables triger update of demosaic module
 *
 *  Return:   TRUE - Success
 *            FALSE - Parameters size mismatch
 **/
boolean isp_sub_module_port_trigger_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  isp_private_event_t   *private_event = NULL;
  isp_mod_set_enable_t  *set_enable = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  if (private_event->data_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    ISP_ERR("size mismatch, expecting = %zu, received = %u",
      sizeof(isp_mod_set_enable_t), private_event->data_size);
    return FALSE;
  }

  set_enable = (isp_mod_set_enable_t *)private_event->data;
  if (!set_enable) {
    ISP_ERR("failed: set_enable %p", set_enable);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if ((set_enable->enable_mask) & (1 << isp_sub_module_priv->hw_module_id)) {
    isp_sub_module->submod_trigger_enable = TRUE;
  } else {
    isp_sub_module->submod_trigger_enable = FALSE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* isp_sub_module_port_trigger_enable */

/** isp_sub_module_port_enable:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event handle

 *  This function runs in ISP HW thread context.
 *
 *  This function enables demosaic module
 *
 *  Return:   0 - Success
 *           -1 - Parameters size mismatch
 **/
boolean isp_sub_module_port_enable(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  isp_private_event_t   *private_event = NULL;
  isp_mod_set_enable_t  *set_enable = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  if (private_event->data_size != sizeof(isp_mod_set_enable_t)) {
    /* size mismatch */
    ISP_ERR("size mismatch, expecting = %zu, received = %u",
      sizeof(isp_mod_set_enable_t), private_event->data_size);
    return FALSE;
  }

  set_enable = (isp_mod_set_enable_t *)private_event->data;
  if (!set_enable) {
    ISP_ERR("failed: set_enable %p", set_enable);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if ((set_enable->enable_mask) & (1 << isp_sub_module_priv->hw_module_id)) {
    isp_sub_module->submod_enable = TRUE;
  } else {
    isp_sub_module->submod_enable = FALSE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* isp_sub_module_port_enable */

/** isp_sub_module_port_append_hw_update_list:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event to be handled
 *
 *  Append hw update in pending hw udpate list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_port_append_hw_update_list(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                       ret = TRUE;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p data %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    return FALSE;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list mod %s",
      MCT_MODULE_NAME(module));
  }

  return ret;
} /* isp_sub_module_port_append_hw_update_list */

/** isp_sub_module_port_loop_module_event:
 *
 *  @module: module handle
 *  @port: port handle
 *  @isp_sub_module: session param
 *  @event: event to be processed
 *
 *  Handle downstream ctrl event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_sub_module_port_loop_module_event(
  mct_module_t *module, isp_sub_module_private_func_t *private_func,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                    ret = TRUE;
  mct_event_module_t        *module_event = NULL;
  isp_sub_module_event_func  event_func = NULL;
  isp_module_event_type_t    int_event = ISP_MODULE_EVENT_MAX;

  if (!module || !private_func || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p %p", module, private_func, isp_sub_module, event);
    return FALSE;
  }

  /* Extract ctrl event */
  module_event = &event->u.module_event;
  ISP_DBG("ide %x event %d mod %s", event->identity,
     module_event->type, MCT_MODULE_NAME(module));

  int_event = module_event_mapping[module_event->type];

  if ((int_event > ISP_MODULE_EVENT_INVALID) &&
    (int_event < ISP_MODULE_EVENT_MAX)) {
    event_func = private_func->module_event_handler[int_event];
    if (event_func) {
      ret = event_func(module, isp_sub_module, event);
      if (ret == FALSE) {
        ISP_DBG("failed: mct event %d int event %d", module_event->type,
          int_event);
      }
    }
  }
  return ret;
}

/** isp_sub_module_port_forward_saved_events:
 *
 *  @session_param: session param
 *  @saved_events: handle to saved events
 *  @reset_valid_flag: flag to indicate whether event valid flag
 *                   should be reset
 *
 *  Forward saved module and set params downstream to sub
 *  modules
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_sub_module_port_forward_saved_events(
  mct_module_t *module, isp_sub_module_t *isp_sub_module,
  isp_saved_events_t *saved_events)
{
  uint32_t            i = 0;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  if (!module || !isp_sub_module || !saved_events) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, saved_events);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }

  /* Forward set params */
  for (i = 0; i < ISP_SET_MAX; i++) {
    if (saved_events->set_params[i] &&
      (saved_events->set_params_valid[i] == TRUE)) {
      isp_sub_module_port_set_param(module, isp_sub_module,
        saved_events->set_params[i]);
    }
  }

  /* Forward module events */
  for (i = 0; i < ISP_MODULE_EVENT_MAX; i++) {
    if (saved_events->module_events[i] &&
      (saved_events->module_events_valid[i] == TRUE)) {
      isp_sub_module_port_loop_module_event(module,
        isp_sub_module_priv->private_func, isp_sub_module,
        saved_events->module_events[i]);
    }
  }

  return TRUE;
}

/*
*
*code to Enable and Disable various ISP modules dynamically
*
*adb shell setprop vendor.camera.isp.module_group 0 <if module_id <= 30>
*adb shell setprop vendor.camera.isp.module_group 1 <if module_id > 30>
*adb shell setprop vendor.camera.isp.toggle_module <sum of 2^(module_id) for group 0 & sum of 2^(module_id-31) for group 1>
*adb shell setprop vendor.camera.isp.switch_it 0 (for disabling the module)
*adb shell setprop vendor.camera.isp.switch_it 1 (for enabling the module)
*adb shell setprop vendor.camera.isp.switch_it 2 (for turning the value of toggle to neutral)
*
*for example- to disable linearization and mesh_rolloff sub-modules together
*type the following commands:
*adb shell setprop vendor.camera.isp.module_group 0    (both these have module_id <= 30)
*adb shell vendor.camera.isp.setprop toggle_module 5   (mod_id of linearization = 0 & mod_id of mesh_rolloff44 = 1)
*adb shell vendor.camera.isp.setprop switch_it 0       (disables both modules)
*adb shell vendor.camera.isp.setprop switch_it 2       (set toggle back to neutral before going on to other modules)
*
*Now, after doing this, to enable back only the linearization module (and keep the rolloff module disabled):
*adb shell vendor.camera.isp.setprop toggle_module 1   (mod_id of linearization = 0 )
*adb shell vendor.camera.isp.setprop switch_it 1       (enable linearization)
*adb shell vendor.camera.isp.setprop switch_it 2       (set toggle back to neutral before typing another toggle_module command)
*
*Now to enable pedestal_correct46 (module_id = 38)
*adb shell setprop vendor.camera.isp.module_group 1    (module_id >30 so belongs in group 1)
*adb shell vendor.camera.isp.setprop toggle_module 128 ( 2^(38-31) = 128 )
*adb shell vendor.camera.isp.setprop switch_it 1       (enable pedestal_correct46)
*adb shell vendor.camera.isp.setprop switch_it 2       (set toggle back to neutral before typing another toggle_module command)
*
*NOTE: Before exiting the camera app
*      set value of toggle_module back to 0
*      to enable normal startup of camera next time.
*
*/

boolean isp_sub_module_manual_toggle(isp_sub_module_t *isp_sub_module)
{
  char mod_id[PROPERTY_VALUE_MAX];
  bool  reg_val[ISP_MOD_ALL];
  char toggle_value[PROPERTY_VALUE_MAX];
  int toggle;
  uint32_t mod_name = 0;
  bool store_submod_enable;
  char mod_grp[PROPERTY_VALUE_MAX];
  int group = 0;
  uint32_t bit = 1;
  int len = 0;
  memset(reg_val, FALSE , sizeof(reg_val));

  if(!isp_sub_module){
    ISP_DBG("failed: isp_sub_module not found");
    return FALSE;
  }

  store_submod_enable = isp_sub_module->submod_enable;
#ifdef _ANDROID_
  len =  property_get("vendor.camera.isp.module_group",mod_grp,"0");
  if(len > PROPERTY_VALUE_MAX){
    ISP_DBG("failed: Invalid module_group");
    return FALSE;
  }
  group = atoi(mod_grp);
  len = property_get("vendor.camera.isp.toggle_module",mod_id, "0");
  if(len > PROPERTY_VALUE_MAX){
    ISP_DBG("failed: Invalid mod_id");
    return FALSE;
  }
  mod_name = atoi(mod_id);
  len = property_get("vendor.camera.isp.switch_it",toggle_value, "2");
  if(len > PROPERTY_VALUE_MAX){
    ISP_DBG("failed: Invalid toggle_value");
    return FALSE;
  }
  toggle = atoi(toggle_value);
#endif
  int i = 0;
  if(mod_name < (bit<<ISP_MOD_SCALER_ENCODER)){
    if(group == 0){
      for(i = 0;i < ISP_MOD_SCALER_ENCODER ;i++){
        reg_val[i] = ((mod_name & (bit << i)) != 0) ? TRUE : FALSE;
      }
    }else if(group == 1){
      for(i = 31;i < ISP_MOD_ALL ;i++){
        reg_val[i] = ((mod_name & (bit << (i-31))) != 0) ? TRUE : FALSE;
      }
    }
  }

  if(mod_name){
    if(reg_val[isp_sub_module->hw_module_id] == TRUE){
         if(toggle == 0){
             //To disable the module
             isp_sub_module->submod_enable = FALSE;
         }else if(toggle == 1){
             //To enable the module
             isp_sub_module->submod_enable = TRUE;
         }

         if(store_submod_enable != isp_sub_module->submod_enable)
           isp_sub_module->update_module_bit = TRUE;
    }
  }

  return TRUE;
}
/** isp_sub_module_port_isp_private_event:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event handle
 *
 *  Handle ISP private event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_port_isp_private_event(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                        ret = TRUE;
  int                            rc = 0;
  isp_private_event_t           *private_event = NULL;
  isp_sub_module_event_func      event_func = NULL;
  isp_sub_module_priv_t         *isp_sub_module_priv = NULL;
  isp_sub_module_private_func_t *private_func = NULL;
  isp_sub_module_output_t       *output = NULL;
  boolean                        man_toggle = TRUE;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }

  private_func = isp_sub_module_priv->private_func;

  event_func = private_func->isp_private_event_handler[private_event->type];
  if (event_func) {
    if (private_event->type == ISP_PRIVATE_SET_TRIGGER_UPDATE) {
      output = (isp_sub_module_output_t *)private_event->data;
      if (output->saved_events) {
        /* Forward saved events */
        isp_sub_module_port_forward_saved_events(module, isp_sub_module,
          output->saved_events);
      }
    }

    ret = event_func(module, isp_sub_module, event);
    if (ret == FALSE) {
      ISP_DBG("failed: module %s private event %d", MCT_MODULE_NAME(module),
        private_event->type);
    }

  man_toggle = isp_sub_module_manual_toggle(isp_sub_module);

  if(man_toggle == FALSE){
    ISP_DBG("failed: isp_sub_module_manual_toggle %s , %p", MCT_MODULE_NAME(module), isp_sub_module);
  }

  if ((private_event->type == ISP_PRIVATE_SET_TRIGGER_UPDATE) &&
      (isp_sub_module->update_module_bit == TRUE)) {
      PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
      ret = isp_sub_module_util_update_module_bit(isp_sub_module,
        private_event->data);
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_sub_module_port_update_module_bit %s",
          MCT_MODULE_NAME(module));
      }
    }
  }

  return ret;
} /* isp_sub_module_port_isp_private_event */

/** isp_sub_module_port_set_dual_led_calibration
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event handle
 *
 *  Enable / Disable Dual LED calibration mode
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_port_set_dual_led_calibration(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  mct_event_control_parm_t      *set_param = NULL;
  int32_t                       *value = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  set_param =
    (mct_event_control_parm_t *)(event->u.ctrl_event.control_event_data);
  if (!set_param) {
    ISP_ERR("failed: event_control %p", set_param);
    return FALSE;
  }

  value = (int32_t *)set_param->parm_data;
  if (!value) {
    ISP_ERR("failed: set_param->parm_data NULL");
    return FALSE;
  }

  if (*value == 0) {
    isp_sub_module->dual_led_calibration_enabled = FALSE;
  } else {
    isp_sub_module->dual_led_calibration_enabled = TRUE;
  }

  return TRUE;
}

/** isp_sub_module_port_set_parm_tintless:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event handle
 *
 *  Enable / Disable tintless
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_port_set_parm_tintless(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                        ret = TRUE;
  mct_event_control_parm_t      *set_param = NULL;
  int32_t                       *value = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  set_param =
    (mct_event_control_parm_t *)(event->u.ctrl_event.control_event_data);
  if (!set_param) {
    ISP_ERR("failed: event_control %p", set_param);
    return FALSE;
  }

  if (set_param->type != CAM_INTF_PARM_TINTLESS) {
    ISP_ERR("failed: invalid set parm type %d expected %d", set_param->type,
      CAM_INTF_PARM_TINTLESS);
    return FALSE;
  }

  value = (int32_t *)set_param->parm_data;
  if (!value) {
    ISP_ERR("failed: set_param->parm_data NULL");
    return FALSE;
  }

  if (*value == 0) {
    isp_sub_module->tintless_enabled = FALSE;
  } else {
    isp_sub_module->tintless_enabled = TRUE;
  }

  return ret;
}

/** isp_sub_module_set_manual_controls:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event handle
 *
 *  Handle Manual controls
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_set_manual_controls(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                        ret = TRUE;
  mct_event_control_parm_t      *set_param = NULL;
  cam_color_correct_matrix_t    cc_transform;
  cam_rgb_tonemap_curves        *app_curve = NULL;
  int                           i,curve;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  set_param =
    (mct_event_control_parm_t *)(event->u.ctrl_event.control_event_data);
  if (!set_param) {
    ISP_ERR("failed: event_control %p", set_param);
    return FALSE;
  }
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  isp_sub_module->manual_ctrls.manual_update = TRUE;

  switch (set_param->type) {
  case CAM_INTF_META_MODE:
    isp_sub_module->manual_ctrls.app_ctrl_mode =
      *((uint8_t *)set_param->parm_data);
    break;

  case CAM_INTF_META_AEC_MODE:
    isp_sub_module->manual_ctrls.aec_ctrl_mode =
      *((uint8_t *)set_param->parm_data);
    break;

  case CAM_INTF_META_ISP_SENSITIVITY:
    isp_sub_module->manual_ctrls.isp_gain =
      (float)(*((int32_t *)set_param->parm_data)) / MULTIPLY_FACTOR_FOR_ISO;
    break;


  case CAM_INTF_PARM_WHITE_BALANCE:
    isp_sub_module->manual_ctrls.wb_mode = *((uint8_t *)set_param->parm_data);
    break;

  case CAM_INTF_META_NOISE_REDUCTION_MODE:
    isp_sub_module->manual_ctrls.abf_mode =
      *((uint8_t *)set_param->parm_data);
    break;

  case CAM_INTF_META_HOTPIXEL_MODE:
    isp_sub_module->manual_ctrls.hot_pix_mode =
      *((uint8_t *)set_param->parm_data);
    break;

  case CAM_INTF_META_SHADING_MODE:
    isp_sub_module->manual_ctrls.lens_shading_mode =
      *((uint8_t *)set_param->parm_data);
    break;

  case CAM_INTF_META_LENS_SHADING_MAP_MODE:
    isp_sub_module->manual_ctrls.lens_shading_map_mode =
      *((uint8_t *)set_param->parm_data);
    break;

  case CAM_INTF_META_TONEMAP_MODE:
    isp_sub_module->manual_ctrls.tonemap_mode =
      *((uint8_t *)set_param->parm_data);
    break;

  case CAM_INTF_META_COLOR_CORRECT_MODE:
    isp_sub_module->manual_ctrls.cc_mode =
      *((uint8_t *)set_param->parm_data);
    break;

  case CAM_INTF_META_BLACK_LEVEL_LOCK:
    isp_sub_module->manual_ctrls.black_level_lock =
      *((uint8_t *)set_param->parm_data);
    break;

  case CAM_INTF_META_COLOR_CORRECT_GAINS:
    isp_sub_module->manual_ctrls.update_cc_gain = TRUE;
    isp_sub_module->manual_ctrls.cc_gain =
      *((cam_color_correct_gains_t *)set_param->parm_data);
    break;

  case CAM_INTF_META_COLOR_CORRECT_TRANSFORM:
    cc_transform = *((cam_color_correct_matrix_t *)set_param->parm_data);
    for (i = 0; i < 3; i++ ) {
      isp_sub_module->manual_ctrls.cc_transform_matrix[i][0] =
        (float)cc_transform.transform_matrix[i][0].numerator
        / cc_transform.transform_matrix[i][0].denominator;

      isp_sub_module->manual_ctrls.cc_transform_matrix[i][1] =
        (float)cc_transform.transform_matrix[i][1].numerator
        / cc_transform.transform_matrix[i][1].denominator;

      isp_sub_module->manual_ctrls.cc_transform_matrix[i][2] =
        (float)cc_transform.transform_matrix[i][2].numerator
        / cc_transform.transform_matrix[i][2].denominator;

    }
    isp_sub_module->manual_ctrls.update_cc = TRUE;
    break;

  case CAM_INTF_META_TONEMAP_CURVES:
    isp_sub_module->manual_ctrls.update_gamma = TRUE;
    isp_sub_module->manual_ctrls.gamma_curve =
      *((cam_rgb_tonemap_curves*)set_param->parm_data);
    break;

  case CAM_INTF_PARM_CAC:
    isp_sub_module->manual_ctrls.cac_mode =
        *((uint8_t*)set_param->parm_data);
    break;

  default:
    ISP_ERR("Manual control %dnot handled", set_param->type);
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    break;
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
}

/** isp_sub_module_port_set_vfe_command:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event handle
 *
 *  Handle EZTUNE set param
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_port_set_vfe_command(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                        ret = TRUE;
  mct_event_control_parm_t      *set_param = NULL;
  tune_cmd_t                    *tune_cmd = NULL;
  isp_hw_module_id_t             hw_module_id = 0;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  set_param =
    (mct_event_control_parm_t *)(event->u.ctrl_event.control_event_data);
  if (!set_param) {
    ISP_ERR("failed: event_control %p", set_param);
    return FALSE;
  }

  if (set_param->type != CAM_INTF_PARM_SET_VFE_COMMAND) {
    ISP_ERR("failed: invalid set parm type %d expected %d", set_param->type,
      CAM_INTF_PARM_SET_VFE_COMMAND);
    return FALSE;
  }

  tune_cmd = (tune_cmd_t *)set_param->parm_data;
  if (!tune_cmd) {
    ISP_ERR("failed: tune_cmd %p", tune_cmd);
    return FALSE;
  }

  hw_module_id = isp_sub_module_util_convert_ez_to_isp_id(tune_cmd->module);
  if (hw_module_id >= ISP_MOD_MAX_NUM) {
    ISP_ERR("failed: invalid isp id %d", hw_module_id);
    return FALSE;
  }

  if (hw_module_id != ISP_MOD_ALL &&
    hw_module_id != isp_sub_module->hw_module_id) {
    return TRUE;
  }

  /* Unlock this in every return path */
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  switch(tune_cmd->type) {
  case SET_STATUS: {
    ISP_HIGH("vfe_diagnostics module %s enable %d",
      MCT_MODULE_NAME(module), tune_cmd->value);
    if (tune_cmd->value) {
      isp_sub_module->vfe_diag_enable = TRUE;
    } else {
      isp_sub_module->vfe_diag_enable = FALSE;
    }
  }
    break;

  case SET_CONTROLENABLE: {
    isp_sub_module->update_module_bit = TRUE;
    isp_sub_module->trigger_update_pending = TRUE;
    if (tune_cmd->value) {
      isp_sub_module->submod_trigger_enable = TRUE;
    } else {
      isp_sub_module->submod_trigger_enable = FALSE;
    }
  }
    break;

  case SET_ENABLE: {
    isp_sub_module->update_module_bit = TRUE;
    isp_sub_module->trigger_update_pending = TRUE;
    if (tune_cmd->value) {
      isp_sub_module->submod_enable = TRUE;
    } else {
      isp_sub_module->submod_enable = FALSE;
    }
  }
    break;

  default: {
    ISP_ERR("failed: invalid type %d", tune_cmd->type);
    ret = FALSE;
  }
    break;
 }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* isp_sub_module_port_set_vfe_command */

/** isp_sub_module_port_set_log_level:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event handle
 *
 *  Handle CAM_INTF_PARM_UPDATE_DEBUG_LEVEL set param
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_port_set_log_level(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                        ret = TRUE;
  mct_event_control_parm_t      *set_param = NULL;
  isp_sub_module_priv_t         *isp_sub_module_priv = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);

  set_param =
    (mct_event_control_parm_t *)(event->u.ctrl_event.control_event_data);
  if (!set_param) {
    ISP_ERR("failed: event_control %p", set_param);
    return FALSE;
  }

  if (set_param->type != CAM_INTF_PARM_UPDATE_DEBUG_LEVEL) {
    ISP_ERR("failed: invalid set parm type %d expected %d", set_param->type,
      CAM_INTF_PARM_UPDATE_DEBUG_LEVEL);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));
  ret = isp_sub_module_util_setloglevel(isp_sub_module_priv->module_name,
                                        isp_sub_module_priv->mod_id_for_log);
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return ret;

} /* isp_sub_module_port_set_log_level */



/** isp_sub_module_port_disable_module:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event handle
 *
 *  Handle ISP_MODULE_EVENT_ISP_DISABLE_MODULE module event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_port_disable_module(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  mct_event_module_t *module_event = NULL;
  uint64_t *mask;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  module_event = &event->u.module_event;

  if (!module_event) {
    ISP_ERR("failed: module_event %p", module_event);
    return FALSE;
  }

  mask = ((uint64_t *)module_event->module_event_data);
  ISP_DBG("mask  %lld hw_id %d ", *mask, isp_sub_module->hw_module_id);
  RETURN_IF_NULL(mask);

  if ( *mask & ((uint64_t)1 << isp_sub_module->hw_module_id)){
    ISP_DBG("Disbale module ID %d ", isp_sub_module->hw_module_id);
    isp_sub_module->submod_enable = FALSE;
    isp_sub_module->update_module_bit = TRUE;
  }

  return TRUE;
}
/** isp_set_hdr_mode:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event handle
 *
 *  Handle CAM_INTF_PARM_SENSOR_HDR set param
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_set_hdr_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  mct_event_module_t *module_event = NULL;
  isp_hdr_mode_t     *isp_hdr_mode = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }
  module_event = &event->u.module_event;

  if (!module_event) {
    ISP_ERR("failed: module_event %p", module_event);
    return FALSE;
  }

  isp_hdr_mode = ((isp_hdr_mode_t *)module_event->module_event_data);
  ISP_DBG("hdr_mode %d hw_id %d ", isp_hdr_mode->hdr_mode, isp_sub_module->hw_module_id);
  RETURN_IF_NULL(isp_hdr_mode);
  isp_sub_module->hdr_mode = isp_hdr_mode->hdr_mode;

  return TRUE;
}

/** isp_sub_module_port_request_stats_type:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event handle
 *
 *  Handle CAM_INTF_PARM_UPDATE_DEBUG_LEVEL set param
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_port_request_stats_type(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                        ret = TRUE;
  mct_event_request_stats_type  *stats_type = NULL;
  enum msm_isp_stats_type        hw_stats_type = MSM_ISP_STATS_MAX;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  stats_type =
    (mct_event_request_stats_type *)(event->u.ctrl_event.control_event_data);
  if (!stats_type) {
    ISP_ERR("failed: stats_type %p", stats_type);
    return FALSE;
  }

  switch (isp_sub_module->hw_module_id) {
  case ISP_MOD_BE_STATS:
    hw_stats_type = MSM_ISP_STATS_BE;
    break;
  case ISP_MOD_BG_STATS:
    hw_stats_type = MSM_ISP_STATS_BG;
    break;
  case ISP_MOD_BF_STATS:
    hw_stats_type = MSM_ISP_STATS_BF;
    break;
  case ISP_MOD_AWB_STATS:
    hw_stats_type = MSM_ISP_STATS_AWB;
    break;
  case ISP_MOD_RS_STATS:
    hw_stats_type = MSM_ISP_STATS_RS;
    break;
  case ISP_MOD_CS_STATS:
    hw_stats_type = MSM_ISP_STATS_CS;
    break;
  case ISP_MOD_IHIST_STATS:
    hw_stats_type = MSM_ISP_STATS_IHIST;
    break;
  case ISP_MOD_SKIN_BHIST_STATS:
    hw_stats_type = MSM_ISP_STATS_BHIST;
    break;
  case ISP_MOD_BF_SCALE_STATS:
    hw_stats_type = MSM_ISP_STATS_BF_SCALE;
    break;
  case ISP_MOD_AEC_BG_STATS:
    hw_stats_type = isp_sub_module->hw_stats_type;
    break;
  case ISP_MOD_HDR_BE_STATS:
    hw_stats_type = isp_sub_module->hw_stats_type;
    break;
  case ISP_MOD_HDR_BHIST_STATS:
    hw_stats_type = MSM_ISP_STATS_HDR_BHIST;
    break;
  default:
    break;
  }

  isp_sub_module->rgn_skip_pattern =
    stats_type->enable_rgn_skip_pattern[hw_stats_type];
  isp_sub_module->stats_tap_location =
    stats_type->requested_tap_location[hw_stats_type];

  return ret;

} /* isp_sub_module_port_request_stats_type */

/** isp_sub_module_port_set_param:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event handle
 *
 *  Handle control set param
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_port_set_param(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                        ret = TRUE;
  mct_event_control_parm_t      *set_param = NULL;
  isp_sub_module_event_func      event_func = NULL;
  isp_sub_module_priv_t         *isp_sub_module_priv = NULL;
  isp_sub_module_private_func_t *private_func = NULL;
  isp_set_param_type_t           int_event = ISP_SET_MAX;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  set_param =
    (mct_event_control_parm_t *)(event->u.ctrl_event.control_event_data);
  if (!set_param) {
    ISP_ERR("failed: event_control %p", set_param);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }

  private_func = isp_sub_module_priv->private_func;
  ISP_DBG("set_param->type %d", set_param->type);
  switch (set_param->type) {
  case CAM_INTF_PARM_CONTRAST:
    int_event = ISP_SET_PARM_CONTRAST;
    break;

  case CAM_INTF_PARM_SATURATION:
    int_event = ISP_SET_PARM_SATURATION;
    break;

  case CAM_INTF_PARM_USERZOOM:
    int_event = ISP_SET_PARM_ZOOM;
    break;

  case CAM_INTF_META_SCALER_CROP_REGION:
    int_event = ISP_SET_META_SCALER_CROP_REGION;
    break;

  case CAM_INTF_PARM_EFFECT:
    int_event = ISP_SET_PARM_EFFECT;
    break;

  case CAM_INTF_META_MODE:
    int_event = ISP_SET_META_MODE;
    break;

  case CAM_INTF_META_AEC_MODE:
    int_event = ISP_SET_AEC_MODE;
    break;

  case CAM_INTF_META_ISP_SENSITIVITY:
    int_event = ISP_SET_MANUAL_GAIN;
    break;

  case CAM_INTF_PARM_WHITE_BALANCE:
    int_event = ISP_SET_PARM_WHITE_BALANCE;
    break;

  case CAM_INTF_META_NOISE_REDUCTION_MODE:
    int_event = ISP_SET_META_NOISE_REDUCTION_MODE;
    break;

  case CAM_INTF_META_HOTPIXEL_MODE:
    int_event = ISP_SET_INTF_HOTPIXEL_MODE;
    break;

  case CAM_INTF_META_LENS_SHADING_MAP_MODE:
    int_event = ISP_SET_META_LENS_SHADING_MAP_MODE;
    break;

  case CAM_INTF_META_SHADING_MODE:
      int_event = ISP_SET_META_LENS_SHADING_MODE;
      break;

  case CAM_INTF_META_TONEMAP_MODE:
    int_event = ISP_SET_META_TONEMAP_MODE;
    break;

  case CAM_INTF_META_TONEMAP_CURVES:
    int_event = ISP_SET_META_TONEMAP_CURVES;
    break;

  case CAM_INTF_META_COLOR_CORRECT_MODE:
    int_event = ISP_SET_META_COLOR_CORRECT_MODE;
    break;

  case CAM_INTF_META_COLOR_CORRECT_GAINS:
    int_event = ISP_SET_META_COLOR_CORRECT_GAINS;
    break;

  case CAM_INTF_META_COLOR_CORRECT_TRANSFORM:
    int_event = ISP_SET_META_COLOR_CORRECT_TRANSFORM;
    break;

  case CAM_INTF_META_BLACK_LEVEL_LOCK:
    int_event = ISP_SET_META_BLACK_LEVEL_LOCK;
    break;

  case CAM_INTF_PARM_BESTSHOT_MODE:
    int_event = ISP_SET_PARM_BESTSHOT_MODE;
    break;

  case CAM_INTF_PARM_SCE_FACTOR:
    int_event = ISP_SET_PARM_SCE_FACTOR;
    break;

  case CAM_INTF_PARM_HFR:
    int_event = ISP_SET_PARM_HFR;
    break;

  case CAM_INTF_PARM_DIS_ENABLE:
    int_event = ISP_SET_PARM_DIS_ENABLE;
    break;

  case CAM_INTF_PARM_SENSOR_HDR:
    int_event = ISP_SET_PARM_SENSOR_HDR_MODE;
    break;

  case CAM_INTF_PARM_FRAMESKIP:
    int_event = ISP_SET_PARM_FRAMESKIP;
    break;

  case CAM_INTF_PARM_RECORDING_HINT:
    int_event = ISP_SET_PARM_RECORDING_HINT;
    break;

  case CAM_INTF_PARM_TINTLESS:
    int_event = ISP_SET_PARM_TINTLESS;
    break;

  case CAM_INTF_PARM_SET_VFE_COMMAND:
    int_event = ISP_SET_PARM_SET_VFE_COMMAND;
    break;

  case CAM_INTF_PARM_VT:
    int_event = ISP_SET_PARM_VT;
    break;

  case CAM_INTF_META_USE_AV_TIMER:
    int_event = ISP_SET_PARM_VT;
    break;

  case CAM_INTF_PARM_HDR:
    int_event = ISP_SET_PARM_HDR;
    break;

  case CAM_INTF_PARM_TONE_MAP_MODE:
    int_event = ISP_SET_INTF_PARM_LTM_MODE;
    break;

  case CAM_INTF_PARM_CAC:
    int_event = ISP_SET_INTF_PARM_CAC;
    break;

  case CAM_INTF_PARM_UPDATE_DEBUG_LEVEL:
    int_event = ISP_SET_PARM_UPDATE_DEBUG_LEVEL;
    break;

  case CAM_INTF_PARM_LONGSHOT_ENABLE:
    int_event = ISP_SET_PARM_LONGSHOT_ENABLE;
    break;

  case CAM_INTF_META_STATS_HISTOGRAM_MODE:
  case CAM_INTF_PARM_HISTOGRAM:
    int_event = ISP_SET_PARM_BHIST_META_ENABLE;
    break;

  case CAM_INTF_PARM_ZSL_MODE:
    int_event = ISP_SET_PARM_ZSL_MODE;
    break;

  default:
    int_event = ISP_SET_MAX;
    break;
  }

  if (int_event < ISP_SET_MAX) {
    event_func = private_func->set_param_handler[int_event];
    if (event_func) {
      ret = event_func(module, isp_sub_module, event);
      if (ret == FALSE) {
        ISP_ERR("failed: module %s set parm %d int set parm %d", MCT_MODULE_NAME(module),
          set_param->type, int_event);
      }
    }
  }

  return ret;
} /* isp_sub_module_port_set_param */

/** isp_sub_module_port_streamon:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function adds ref count for stream on flag and sets
 *  trigger_update_pending flag to TRUE for first STREAM ON
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_port_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->config_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

} /* isp_sub_module_port_config */

/** isp_sub_module_port_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function decrements ref count for stream on flag
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_port_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  if (!(--isp_sub_module->stream_on_count)) {
    /* Last stream OFF */
    if (isp_sub_module->private_data && isp_sub_module->private_data_size) {
      memset(isp_sub_module->private_data, 0x0,
        isp_sub_module->private_data_size);
    }
  }

  return ret;

} /* isp_sub_module_port_streamoff */

/** isp_sub_module_port_set_super_param:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: event handle
 *
 *  Handle control set param
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_port_set_super_param(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                         ret = TRUE;
  isp_sub_module_event_func       event_func = NULL;
  isp_sub_module_priv_t          *isp_sub_module_priv = NULL;
  isp_sub_module_private_func_t  *private_func = NULL;
  uint32_t                        index = 0;
  mct_event_super_control_parm_t  *param = NULL;
  mct_event_control_parm_t       *event_control = NULL;
  mct_event_control_parm_t        ctrl_param;
  mct_event_t                     dummy_event;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  RETURN_IF_NULL(isp_sub_module_priv);

  param = (mct_event_super_control_parm_t *)
    event->u.ctrl_event.control_event_data;
  RETURN_IF_NULL(param);

  private_func = isp_sub_module_priv->private_func;
  ISP_HIGH("param->num_of_parm_events %d", param->num_of_parm_events);
  /* Iterate through all set params list, convert event
        to regular set parm event */
  dummy_event.type = event->type;
  dummy_event.identity = event->identity;
  dummy_event.direction =event->direction;
  dummy_event.u.ctrl_event.type=  MCT_EVENT_CONTROL_SET_PARM;
  dummy_event.u.ctrl_event.control_event_data = &ctrl_param;

  /* Handle all set params */
  for (index = 0; index < param->num_of_parm_events; index++) {
    event_control = &param->parm_events[index];
    ctrl_param.type = event_control->type;
    ctrl_param.parm_data = event_control->parm_data;
    ISP_HIGH("event type =%d", event_control->type);
    event_func =
        private_func->control_event_handler[ISP_CONTROL_EVENT_SET_PARM];
    if (event_func) {
      ret = event_func(module, isp_sub_module, &dummy_event);
      if (ret == FALSE) {
        ISP_ERR("failed: event_control->type %d", event_control->type);
      }
    }
  }
  return ret;
} /* isp_sub_module_port_set_param */

/** isp_sub_module_port_set_snaphsot_stream_on_if_exists:
 *
 *  @module: module handle
 *  @port: port handle
 *  @isp_sub_module: session param
 *  @event: event to be processed
 *
 *  Sets the snapshot on variable in sub module
 *
 *  Return TRUE on success and FALSE on failure
 **/
void isp_sub_module_port_set_snaphsot_stream_on(
    isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  mct_stream_info_t        *stream_info = NULL;

  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return;
  }

  stream_info = (mct_stream_info_t*)event->u.ctrl_event.control_event_data;

  switch (event->u.ctrl_event.type) {
    case MCT_EVENT_CONTROL_STREAMON: {
      if (stream_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT) {
        isp_sub_module->stream_data.is_snapshot_stream_on = TRUE;
        isp_sub_module->stream_data.snapshot_stream_count++;
      }
      if (stream_info->stream_type == CAM_STREAM_TYPE_CALLBACK) {
        isp_sub_module->stream_data.is_cb_stream_on = TRUE;
        isp_sub_module->stream_data.cb_stream_count++;
      }
    }
    break;
    case MCT_EVENT_CONTROL_STREAMOFF: {
      if (stream_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT) {
        isp_sub_module->stream_data.snapshot_stream_count--;
        if(isp_sub_module->stream_data.snapshot_stream_count <= 0) {
          isp_sub_module->stream_data.is_snapshot_stream_on = FALSE;
          isp_sub_module->stream_data.snapshot_stream_count = 0;
       }
      }
      if (stream_info->stream_type == CAM_STREAM_TYPE_CALLBACK) {
        isp_sub_module->stream_data.cb_stream_count--;
        if(isp_sub_module->stream_data.cb_stream_count <= 0) {
          isp_sub_module->stream_data.is_cb_stream_on = FALSE;
          isp_sub_module->stream_data.cb_stream_count = 0;
        }
      }
    }
    break;
    default:
      ISP_ERR("failed: Wrong event sent");
      break;
  }
}

/** isp_sub_module_port_set_zsl_mode:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean isp_sub_module_port_set_zsl_mode(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                         ret = TRUE;
  mct_event_control_parm_t *set_param = NULL;
  int32_t                      *value = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  set_param =
    (mct_event_control_parm_t *)(event->u.ctrl_event.control_event_data);
  if (!set_param) {
    ISP_ERR("failed: event_control %p", set_param);
    return FALSE;
  }

  if (set_param->type != CAM_INTF_PARM_ZSL_MODE) {
    ISP_ERR("failed: invalid set parm type %d expected %d", set_param->type,
      CAM_INTF_PARM_ZSL_MODE);
    return FALSE;
  }

  value = (int32_t *)set_param->parm_data;
  if (!value) {
    ISP_ERR("failed: set_param->parm_data NULL");
    return FALSE;
  }

  if (*value == 0) {
    isp_sub_module->zsl_mode = FALSE;
  } else {
    isp_sub_module->zsl_mode = TRUE;
  }

  return ret;

} /* isp_sub_module_port_set_zsl_mode */

/** isp_sub_module_port_process_downstream_ctrl_event:
 *
 *  @module: module handle
 *  @port: port handle
 *  @isp_sub_module: session param
 *  @event: event to be processed
 *
 *  Handle downstream ctrl event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_sub_module_port_process_downstream_ctrl_event(
  mct_module_t *module, mct_port_t *port,
  isp_sub_module_private_func_t *private_func, isp_sub_module_t *isp_sub_module,
  mct_event_t *event)
{
  boolean                    ret = TRUE;
  mct_event_control_t       *ctrl_event = NULL;
  isp_sub_module_event_func  event_func = NULL;
  isp_control_event_type_t   int_event = ISP_CONTROL_EVENT_MAX;

  if (!module || !port || !private_func || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p %p %p", module, port, private_func,
      isp_sub_module, event);
    return FALSE;
  }

  /* Extract ctrl event */
  ctrl_event = &event->u.ctrl_event;
  ISP_DBG("ide %x event %d mod %s", event->identity,
     ctrl_event->type, MCT_MODULE_NAME(module));
  switch (ctrl_event->type) {
  case MCT_EVENT_CONTROL_STREAMON: {
    isp_sub_module_port_set_snaphsot_stream_on(isp_sub_module,
      event);
    int_event = ISP_CONTROL_EVENT_STREAMON;
  }
    break;

  case MCT_EVENT_CONTROL_STREAMOFF: {
    isp_sub_module_port_set_snaphsot_stream_on(isp_sub_module,
      event);
    int_event = ISP_CONTROL_EVENT_STREAMOFF;
  }
    break;

  case MCT_EVENT_CONTROL_SET_SUPER_PARM:
    int_event = ISP_CONTROL_EVENT_SET_SUPER_PARM;
    break;

  case MCT_EVENT_CONTROL_SET_PARM:
    int_event = ISP_CONTROL_EVENT_SET_PARM;
    break;

  default:
    int_event = ISP_CONTROL_EVENT_MAX;
    break;
  }

  if (int_event < ISP_CONTROL_EVENT_MAX) {
    event_func = private_func->control_event_handler[int_event];
    if (event_func) {
      PTHREAD_MUTEX_LOCK(&isp_sub_module->new_mutex);
      ret = event_func(module, isp_sub_module, event);
      if (ret == FALSE) {
        ISP_DBG("failed: mct event %d int_event %d", ctrl_event->type,
          int_event);
      }
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->new_mutex);
    }
  }
  /* Forward event */
  ret = isp_sub_module_util_forward_event_from_port(port, event);
  if (ret == FALSE) {
    ISP_DBG("failed: mct event %d int_event %d", ctrl_event->type, int_event);
  }

  return ret;
} /* isp_sub_module_port_process_downstream_ctrl_event */

/** isp_sub_module_port_process_downstream_module_event:
 *
 *  @module: module handle
 *  @port: port handle
 *  @isp_sub_module: session param
 *  @event: event to be processed
 *
 *  Handle downstream ctrl event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_sub_module_port_process_downstream_module_event(
  mct_module_t *module, mct_port_t *port,
  isp_sub_module_private_func_t *private_func,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                    ret = TRUE;
  mct_event_module_t        *module_event = NULL;
  isp_sub_module_event_func  event_func = NULL;
  isp_module_event_type_t    int_event = ISP_MODULE_EVENT_MAX;

  if (!module || !port || !private_func || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p %p %p", module, port, private_func,
      isp_sub_module, event);
    return FALSE;
  }

  /* Extract ctrl event */
  module_event = &event->u.module_event;
  ISP_DBG("ide %x event %d mod %s", event->identity,
     module_event->type, MCT_MODULE_NAME(module));

  int_event = module_event_mapping[module_event->type];

  if ((int_event > ISP_MODULE_EVENT_INVALID) &&
    (int_event < ISP_MODULE_EVENT_MAX)) {
    event_func = private_func->module_event_handler[int_event];
    if (event_func) {
      PTHREAD_MUTEX_LOCK(&isp_sub_module->new_mutex);
      ret = event_func(module, isp_sub_module, event);
      if (ret == FALSE) {
        ISP_DBG("failed: mct event %d int event %d", module_event->type,
          int_event);
      }
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->new_mutex);
    }
  }
  /* Forward event */
  ret = isp_sub_module_util_forward_event_from_port(port, event);
  if (ret == FALSE) {
    ISP_DBG("failed: mct event %d int event %d", module_event->type,
      int_event);
  }
  return ret;
}

/** isp_sub_module_port_process_downstream_event:
 *
 *  @module: module handle
 *  @port: port on which this event arrived
 *  @isp_sub_module: session param
 *  @event: event to be handled
 *
 *  Handle downstream event
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_sub_module_port_process_downstream_event(
  mct_module_t *module, mct_port_t *port,
  isp_sub_module_private_func_t *private_func, isp_sub_module_t *isp_sub_module,
  mct_event_t *event)
{
  boolean ret = TRUE;

  if (!module || !port || !private_func || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p %p %p", module, port, private_func,
      isp_sub_module, event);
    return FALSE;
  }

  if ((event->type != MCT_EVENT_CONTROL_CMD) &&
      (event->type != MCT_EVENT_MODULE_EVENT)) {
    ISP_DBG("failed: invalid event type %d", event->type);
    return FALSE;
  }

  if (event->type == MCT_EVENT_CONTROL_CMD) {
    ret = isp_sub_module_port_process_downstream_ctrl_event(module, port,
      private_func, isp_sub_module, event);
    if (ret == FALSE) {
      ISP_DBG("failed: isp_sub_module_port_process_downstream_ctrl_event");
    }
  } else if (event->type == MCT_EVENT_MODULE_EVENT) {
    ret = isp_sub_module_port_process_downstream_module_event(module, port,
      private_func, isp_sub_module, event);
    if (ret == FALSE) {
      ISP_DBG("failed: isp_sub_module_port_process_downstream_module_event");
    }
  }
  return ret;

ERROR:
  return ret;
}

/** isp_sub_module_port_process_upstream_event:
 *
 *  @module: module handle
 *  @port: port on which this event arrived
 *  @isp_sub_module: session param
 *  @event: event to be handled
 *
 *  Handle downstream event
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_sub_module_port_process_upstream_event(mct_module_t *module,
  mct_port_t *port, isp_sub_module_private_func_t *private_func,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                    ret = TRUE;
  mct_event_module_t        *module_event = NULL;
  isp_module_event_type_t    int_event = ISP_MODULE_EVENT_MAX;
  isp_sub_module_event_func  event_func = NULL;

  if (!module || !port || !isp_sub_module || !event || !private_func) {
    ISP_ERR("failed: %p %p %p %p %p", module, port, isp_sub_module, event, private_func);
    return FALSE;
  }

  if (!port || !event) {
    ISP_ERR("failed: port %p event %p", port, event);
    return FALSE;
  }

  if (event->type != MCT_EVENT_MODULE_EVENT) {
    ISP_ERR("failed: invalid event type %d", event->type);
    return FALSE;
  }

  module_event = &event->u.module_event;
  ISP_DBG("ide %x event %d mod %s", event->identity,
     module_event->type, MCT_MODULE_NAME(module));

  switch (module_event->type) {
  case MCT_EVENT_MODULE_ISP_PRIVATE_EVENT: {
    int_event = ISP_MODULE_EVENT_ISP_PRIVATE_EVENT;
    /* Forward event to upstream FRAME port */
    ret = isp_sub_module_util_forward_event_from_port(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
  }
    break;

  default:
    /* Forward event to upstream FRAME port */
    ret = isp_sub_module_util_forward_event_from_port(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;
  }

  if (int_event < ISP_MODULE_EVENT_MAX) {
    event_func = private_func->module_event_handler[int_event];
    if (event_func) {
      PTHREAD_MUTEX_LOCK(&isp_sub_module->new_mutex);
      ret = event_func(module, isp_sub_module, event);
      if (ret == FALSE) {
        ISP_ERR("failed: mct event %d int event %d", module_event->type,
          int_event);
      }
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->new_mutex);
    }
  }

  return ret;
}

/** isp_sub_module_port_event:
 *
 *  @port: port handle
 *  @event: event to be processed
 *
 *  Handle ISP event based on direction
 *
 *  Returns TRUE on success and FALSE on failure */
static boolean isp_sub_module_port_event(mct_port_t *port, mct_event_t *event)
{
  boolean                ret = TRUE;
  mct_module_t          *module = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;
  isp_sub_module_t      *isp_sub_module = NULL;

  if (!port || !event) {
    ISP_ERR("failed: port %p event %p", port, event);
    return FALSE;
  }

  if ((MCT_PORT_IS_SINK(port) != TRUE) && (MCT_PORT_IS_SRC(port) != TRUE)) {
    ISP_ERR("failed: invalid port direction %d", MCT_PORT_DIRECTION(port));
    return FALSE;
  }

  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));

  /* Get session param */
  isp_sub_module_util_get_session_param(isp_sub_module_priv,
   ISP_GET_SESSION_ID(event->identity), &isp_sub_module);
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module_get_session_param %d %p", ret,
      isp_sub_module);
    goto ERROR;
  }

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  if (event->direction == MCT_EVENT_DOWNSTREAM) {
    ret = isp_sub_module_port_process_downstream_event(module, port,
      isp_sub_module_priv->private_func, isp_sub_module, event);
    if (ret == FALSE) {
      ISP_DBG("failed: mod %s ctrl event type %d", MCT_MODULE_NAME(module),
        event->u.ctrl_event.type);
    }
  } else {
    ret = isp_sub_module_port_process_upstream_event(module, port,
      isp_sub_module_priv->private_func, isp_sub_module, event);
    if (ret == FALSE) {
      ISP_DBG("failed: mod %s mod event type %d", MCT_MODULE_NAME(module),
        event->u.module_event.type);
    }
  }

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return ret;
}

/** isp_sub_module_port_ext_link:
 *
 *  @identity: identity of stream
 *  @port: port handle
 *  @peer: peer handle
 *
 *  Handle ext link
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_sub_module_port_ext_link(
    unsigned int identity __unused,
    mct_port_t   *port,
    mct_port_t   *peer)
{
  boolean                 ret = TRUE;
  mct_module_t           *module = NULL;
  mct_port_t             *sink_port = NULL;
  isp_sub_module_port_data_t *port_data = NULL;
  isp_hw_streamid_t       hw_stream_id = 0;

  if (!port || !peer) {
    ISP_ERR("failed: port %p peer %p", port, peer);
    return FALSE;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));

  if (!MCT_PORT_PEER(port)) {
    MCT_PORT_PEER(port) = peer;
  } else { /*the link has already been established*/
    if ((MCT_PORT_PEER(port) != peer))
    goto ERROR;
  }

  port_data = MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    goto ERROR;
  }

  port_data->num_streams++;

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  ISP_HIGH("port %p dir %d parent %p %s", port, MCT_PORT_DIRECTION(port),
    (MCT_PORT_PARENT(port)->data),
    MCT_MODULE_NAME(MCT_PORT_PARENT(port)->data));
  ISP_HIGH("stream ide %x", identity);

  return TRUE;

ERROR:
  ISP_ERR("failed: isp_sub_module_port_ext_link");
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return FALSE;
}

/** isp_sub_module_port_unlink:
 *
 *  @identity: identity of stream
 *  @port: port handle
 *  @peer: peer handle
 *
 *  Handle unlink func
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static void isp_sub_module_port_unlink(
    unsigned int identity __unused,
    mct_port_t   *port,
    mct_port_t   *peer)
{
  boolean                    ret             = TRUE;
  isp_hw_streamid_t          hw_stream_id    = 0;
  mct_module_t               *module         = NULL;
  mct_port_t                 *sink_port      = NULL;
  isp_sub_module_port_data_t *port_data      = NULL;
  isp_sub_module_port_data_t *port_sink_data = NULL;
  mct_list_t                 *l_identity     = NULL;

  if (!port || !peer) {
    ISP_ERR("failed: port %p peer %p", port, peer);
    return;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));

  port_data = MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    goto ERROR;
  }

  if (MCT_PORT_PEER(port) != peer) {
    ISP_ERR("failed");
    goto ERROR;
  }


  if (!(--port_data->num_streams)) {
    MCT_PORT_PEER(port) = NULL;
  }

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  ISP_HIGH("port %p dir %d parent %p %s", port, MCT_PORT_DIRECTION(port),
    (MCT_PORT_PARENT(port)->data),
    MCT_MODULE_NAME(MCT_PORT_PARENT(port)->data));
  ISP_HIGH("stream ide %x", identity);

  return;

ERROR:
  ISP_ERR("failed: isp_sub_module_port_unlink");
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  return;
}

/** isp_sub_module_port_check_caps_reserve:
 *
 *  @port: port handle
 *  @peer_caps: peer caps handle
 *  @stream_info: handle to stream_info
 *
 *  Handle caps reserve
 *
 *  Return TRUE if this port is reserved, FALSE otherwise
 **/
static boolean isp_sub_module_port_check_caps_reserve(
    mct_port_t *port,
    void       *peer_caps __unused,
    void       *stream_info)
{
  boolean                   ret = FALSE;
  mct_stream_info_t        *mct_stream_info = (mct_stream_info_t *)stream_info;
  isp_sub_module_port_data_t   *port_data = NULL;
  uint32_t                  session_id = 0;
  mct_module_t             *module = NULL;
  isp_sub_module_priv_t        *isp_sub_module_priv = NULL;
  isp_sub_module_t             *isp_sub_module = NULL;
  isp_sub_module_private_func_t *private_func = NULL;
  isp_sub_module_store_port_t   *store_port_data = NULL;
  uint32_t                       i = 0, j = 0, k = 0;
  isp_sub_module_store_src_port_t  *store_source_port = NULL;
  boolean                           is_reserved = FALSE;

  if (!port || !stream_info) {
    ISP_ERR("failed: port %p stream_info %p", port, stream_info);
    return FALSE;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));

  /* Check whether */
  port_data = MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    goto ERROR;
  }

  session_id = ISP_GET_SESSION_ID(mct_stream_info->identity);

  /* Check whether this port is already reserved for same session */
  if ((port_data->is_reserved == TRUE) &&
      (port_data->session_id != session_id)) {
    goto ERROR;
  }

  /* Extract module private */
  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module_priv);
    goto ERROR;
  }

  store_port_data = &isp_sub_module_priv->port_data;
  if (MCT_PORT_IS_SRC(port) == TRUE) {
    /* Check whether this source port is already saved in slot */
    for (i = 0; i < MAX_SRC_PORTS; i++) {
      store_source_port = &store_port_data->source_port_data[i];
      if (store_source_port->source_port == port) {
        /* port is already saved, increase ref count */
        /* Check whether identity is already stored */
        for (j = 0; j < MAX_STREAMS; j++) {
          if (store_source_port->identity[j] == mct_stream_info->identity) {
            is_reserved = TRUE;
            break;
          }
        }
        break;
      }
    }
    if (is_reserved == TRUE) {
      /* Same port is already assigned for this stream, return FALSE
       * so that next port will be picked
       */
      goto ERROR;
    }
  }

  /* Get session param */
  isp_sub_module_util_get_session_param(isp_sub_module_priv, session_id,
    &isp_sub_module);
  if (!isp_sub_module) {
    ret = isp_sub_module_util_create_session_param(isp_sub_module_priv,
      session_id, &isp_sub_module);
    if ((ret == FALSE) || !isp_sub_module) {
      ISP_ERR("failed: isp_sub_module_util_create_session_param %d %p", ret,
        isp_sub_module);
      goto ERROR;
    }

    /* Enable enable and trigger enable by default */
    isp_sub_module->submod_enable = TRUE;
    isp_sub_module->submod_trigger_enable = TRUE;
    isp_sub_module->trigger_update_pending = TRUE;
    isp_sub_module->update_module_bit = TRUE;

    private_func = isp_sub_module_priv->private_func;
    if (private_func && private_func->isp_sub_module_init_data) {
      ret = private_func->isp_sub_module_init_data(module, isp_sub_module);
      if (ret == FALSE) {
        ISP_DBG("failed: isp_sub_module_open");
        goto ERROR_FREE;
      }
    }
  }

  isp_sub_module->ref_count++;

  ret = isp_sub_module_util_append_stream_info(isp_sub_module, mct_stream_info);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_stream_info");
    goto ERROR;
  }

  port_data->is_reserved = TRUE;
  port_data->session_id = session_id;

  /* Fill port info in module private */
  store_port_data = &isp_sub_module_priv->port_data;
  if (MCT_PORT_IS_SINK(port) == TRUE) {
    if (!store_port_data->sink_port) {
      /* Port slot is free, store it */
      store_port_data->sink_port = port;
      /* Increase ref count */
      store_port_data->sink_port_count++;
    } else if (store_port_data->sink_port == port) {
      /* port already saved in slot, Increase ref count */
      store_port_data->sink_port_count++;
    } else {
      ISP_ERR("saved source port %p port %p",
        store_port_data->sink_port, port);
      goto ERROR_FREE;
    }
  } else if (MCT_PORT_IS_SRC(port) == TRUE) {
    /* Check whether this source port is already saved in slot */
    for (i = 0; i < MAX_SRC_PORTS; i++) {
      store_source_port = &store_port_data->source_port_data[i];
      if (store_source_port->source_port == port) {
        /* port is already saved, increase ref count */
        /* Check whether identity is already stored */
        for (j = 0; j < MAX_STREAMS; j++) {
          if (store_source_port->identity[j] == mct_stream_info->identity) {
            /* Identity already cached -> error case */
            goto ERROR_FREE;
          }
        }
        /* Identity is not saved, find free slot to save identity */
        for (k = 0; k < MAX_STREAMS; k++) {
          if (!store_source_port->identity[k]) {
            /* Found free slot, Save identity */
            store_source_port->identity[k] = mct_stream_info->identity;
            store_source_port->source_port_count++;
            break;
          }
        }
        if (k >= MAX_STREAMS) {
          /* Could not find free slot for identity */
          ISP_ERR("failed: no empty slot for ide %x source port %p",
            mct_stream_info->identity, port);
          goto ERROR_FREE;
        }
        break;
      }
    }
    /* Check whether port is already saved */
    if (i >= MAX_SRC_PORTS) {
      /* Port is not saved, find empty slot and save it */
      for (i = 0; i < MAX_SRC_PORTS; i++) {
        store_source_port = &store_port_data->source_port_data[i];
        if (!store_source_port->source_port) {
          /* Found free slot to save port */
          store_source_port->source_port = port;
          /* Save identity and increase identity_count */
          for (j = 0; j < MAX_STREAMS; j++) {
            if (!store_source_port->identity[j]) {
              /* Found free slot to save identity */
              store_source_port->identity[j] = mct_stream_info->identity;
              store_source_port->source_port_count++;
              break;
            }
          }
          if (j >= MAX_STREAMS) {
            ISP_ERR("no slot to save ide %x j %d port %p",
              mct_stream_info->identity, j, port);
          }
          break;
        }
      }
      if (i >= MAX_SRC_PORTS) {
        ISP_ERR("no slot to save source port %p", port);
      }
    }
  }

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  ISP_HIGH("port %p dir %d parent %p %s", port, MCT_PORT_DIRECTION(port),
    (MCT_PORT_PARENT(port)->data),
    MCT_MODULE_NAME(MCT_PORT_PARENT(port)->data));

  ISP_HIGH("stream ide %x", mct_stream_info->identity);

  return TRUE;

ERROR_FREE:
  isp_sub_module_util_remove_session_param(isp_sub_module_priv,
      session_id);
ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  return FALSE;
}

/** isp_sub_module_port_check_caps_unreserve:
 *
 *  @port: port handle
 *  @identity: identity of stream
 *
 *  Handle caps unreserve
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_sub_module_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  boolean                   ret = TRUE;
  isp_sub_module_port_data_t   *port_data = NULL;
  mct_module_t             *module = NULL;
  isp_sub_module_priv_t        *isp_sub_module_priv = NULL;
  isp_sub_module_t             *isp_sub_module = NULL;
  uint32_t                  session_id = 0;
  isp_sub_module_private_func_t *private_func = NULL;
  isp_sub_module_store_port_t   *store_port_data = NULL;
  uint32_t                       i = 0, j = 0;
  isp_sub_module_store_src_port_t  *store_source_port = NULL;

  if (!port) {
    ISP_ERR("failed: port %p", port);
    return FALSE;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(module));

  session_id = ISP_GET_SESSION_ID(identity);
  port_data = MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    goto ERROR;
  }

  /* Extract module private */
  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    goto ERROR;
  }

  /* Get session param */
  isp_sub_module_util_get_session_param(isp_sub_module_priv, session_id,
    &isp_sub_module);
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module_get_session_param %d %p", ret,
      isp_sub_module);
    goto ERROR;
  }

  ret = isp_sub_module_util_remove_stream_info(isp_sub_module, identity);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_stream_info");
  }

  if (!(--isp_sub_module->ref_count)) {
    private_func = isp_sub_module_priv->private_func;
    if (private_func && private_func->isp_sub_module_destroy) {
      private_func->isp_sub_module_destroy(module, isp_sub_module);
    }

    ret = isp_sub_module_util_free_hw_update_list(isp_sub_module);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_sub_module_util_free_hw_update_list");
    }

    ret = isp_sub_module_util_remove_session_param(isp_sub_module_priv,
      session_id);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_sub_module_util_remove_session_param");
      goto ERROR;
    }
  }

  if (!port_data->num_streams) {
    port_data->session_id = 0;
    MCT_PORT_PEER(port) = NULL;
    port_data->is_reserved = FALSE;
  }

  /* Fill port info in module private */
  store_port_data = &isp_sub_module_priv->port_data;
  if (MCT_PORT_IS_SINK(port) == TRUE) {
    if (!store_port_data->sink_port) {
      /* sink port is NULL */
      ISP_ERR("sink port %p count %d",
        store_port_data->sink_port, store_port_data->sink_port_count);
    } else if (store_port_data->sink_port != port) {
      /* saved sink port does not match with incoming port */
      ISP_ERR("saved sink port %p in port %p",
        store_port_data->sink_port, port);
    } else {
      /* Decrease sink_port_count */
      if (!(--store_port_data->sink_port_count)) {
        /* sink_port_count 0, clear sink port slot */
        store_port_data->sink_port = NULL;
      }
    }
  } else if (MCT_PORT_IS_SRC(port) == TRUE) {
    /* Check whether this source port is already saved in slot */
    for (i = 0; i < MAX_SRC_PORTS; i++) {
      store_source_port = &store_port_data->source_port_data[i];
      if (store_source_port->source_port == port) {
        /* Found source port slot */
        /* Find identity slot where this identity is saved */
        for (j = 0; j < MAX_STREAMS; j++) {
          if (store_source_port->identity[j] == identity) {
            /* Found identity slot */
            store_source_port->identity[j] = 0x0;
            if (!(--store_source_port->source_port_count)) {
              /* source port count is 0, clear source port slot */
              store_source_port->source_port = NULL;
            }
            break;
          }
        }
        if (j >= MAX_STREAMS) {
          /* Could not find identity slot */
          ISP_ERR("no identity slot ide %x port %p", identity, port);
        }
        break;
      }
    }
    /* Check whether port slot was found */
    if (i >= MAX_SRC_PORTS) {
      /* Could not find port slot */
      ISP_ERR("no port slot for ide %x port %p", identity, port);
    }
  }

  if (!isp_sub_module_priv->l_session_params) {
    /* no more streams in session -> memset port_data */
    memset(&isp_sub_module_priv->port_data, 0,
      sizeof(isp_sub_module_priv->port_data));
  }

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));
  ISP_HIGH("port %p dir %d parent %p %s", port, MCT_PORT_DIRECTION(port),
    (MCT_PORT_PARENT(port)->data),
    MCT_MODULE_NAME(MCT_PORT_PARENT(port)->data));
  ISP_HIGH("stream ide %x", identity);

  return TRUE;
ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(module));

  return FALSE;
}

/** isp_sub_module_port_delete_port:
 *
 *  @data: handle to port to be deleted
 *
 *  @user_data: handle to module
 *
 *  1) Delete port private
 *
 *  2) Delete port from module's list
 *
 *  3) Delete port
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_sub_module_port_delete_port(void *data, void *user_data)
{
  boolean          ret = TRUE;
  mct_module_t    *module = (mct_module_t *)user_data;
  mct_port_t      *port = (mct_port_t *)data;
  isp_sub_module_port_data_t *isp_sub_module_port_data = NULL;

  if (!data || !user_data) {
    ISP_ERR("failed: data %p user_data %p", data, user_data);
    return TRUE;
  }

  isp_sub_module_port_data = MCT_OBJECT_PRIVATE(port);
  if (isp_sub_module_port_data) {
   free(isp_sub_module_port_data);
  }

  ret = mct_module_remove_port(module, port);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_module_remove_port");
  }

  mct_port_destroy(port);

  return TRUE;
}

/** isp_sub_module_port_return_first
 *
 *  @data1: handle to list data
 *
 *  @data2: private data
 *
 *  Return TRUE if first element in list is present
 **/
static boolean isp_sub_module_port_return_first(
    void *data1,
    void *user_data __unused)
{
  if (data1) {
    return TRUE;
  }
  return FALSE;
}

/** isp_sub_module_port_delete_ports:
 *
 *  @port: port handle
 *
 *  Delete port and its private data
 *
 *  Returns: void
 **/
void isp_sub_module_port_delete_ports(mct_module_t *module)
{
  mct_list_t *l_port = NULL;
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  do {
    l_port = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), module,
      isp_sub_module_port_return_first);
    if (l_port)
      isp_sub_module_port_delete_port(l_port->data, module);
  } while (l_port);
  do {
    l_port = mct_list_find_custom(MCT_MODULE_SRCPORTS(module), module,
      isp_sub_module_port_return_first);
    if (l_port)
      isp_sub_module_port_delete_port(l_port->data, module);
  } while (l_port);

}

/** isp_sub_module_port_overwrite_funcs
 *
 *   @port: mct port instance
 *
 *   @private_data: port private
 *
 *  Assign mct port function pointers to respective isp_sub_module port
 *  functions
 *
 *  Returns nothing
 **/
static void isp_sub_module_port_overwrite_funcs(mct_port_t *port,
  void *private_data)
{
  if (!port) {
    ISP_ERR("failed: port %p", port);
    return;
  }

  mct_port_set_event_func(port, isp_sub_module_port_event);
  mct_port_set_ext_link_func(port, isp_sub_module_port_ext_link);
  mct_port_set_unlink_func(port, isp_sub_module_port_unlink);
  mct_port_set_check_caps_reserve_func(port,
    isp_sub_module_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port,
    isp_sub_module_port_check_caps_unreserve);
  MCT_OBJECT_PRIVATE(port) = private_data;
}

/** isp_sub_module_port_create_port
 *
 *   @direction: direction of port
 *
 *  Creates port and add it to parent
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_sub_module_port_create_port(mct_module_t *module,
  mct_port_direction_t direction)
{
  boolean          ret = TRUE;
  isp_sub_module_port_data_t *isp_sub_module_port_data = NULL;
  char             port_name[32];
  mct_port_t      *mct_port = NULL;

  if (!module ||
    ((direction != MCT_PORT_SINK) && (direction != MCT_PORT_SRC))) {
    ISP_ERR("failed: module %p direction %d", module, direction);
    return FALSE;
  }

  isp_sub_module_port_data = CAM_MALLOC(sizeof(isp_sub_module_port_data_t));
  if (!isp_sub_module_port_data) {
    ISP_ERR("failed: CAM_MALLOC");
    return FALSE;
  }
  memset(isp_sub_module_port_data, 0, sizeof(*isp_sub_module_port_data));

  if (direction == MCT_PORT_SINK) {
    snprintf(port_name, sizeof(port_name), "isp_sub_module_sink");
  } else if (direction == MCT_PORT_SRC) {
    snprintf(port_name, sizeof(port_name), "isp_sub_module_src");
  }

  mct_port = mct_port_create(port_name);
  if (!mct_port) {
    ISP_ERR("failed: mct_port_create");
    ret = FALSE;
    goto ERROR;
  }

  mct_port->direction = direction;
  ret = mct_module_add_port(module, mct_port);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_module_add_port");
    goto ERROR;
  }

  mct_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;
  isp_sub_module_port_overwrite_funcs(mct_port,
    (void *)isp_sub_module_port_data);

  return ret;
ERROR:
  free(isp_sub_module_port_data);
  return ret;
}

/** isp_sub_module_port_create_port
 *
 *   @direction: direction of port
 *
 *  Creates port and add it to parent
 *
 *  Returns port handle on success and NULL on failure
 **/
mct_port_t *isp_sub_module_port_request_new_port(mct_module_t *module,
  mct_port_direction_t direction)
{
  boolean                 ret = TRUE;
  isp_sub_module_port_data_t *isp_sub_module_port_data = NULL;
  char                    port_name[32];
  mct_port_t             *mct_port = NULL;

  if (!module ||
    ((direction != MCT_PORT_SINK) && (direction != MCT_PORT_SRC))) {
    ISP_ERR("failed: module %p direction %d", module, direction);
    return NULL;
  }

  isp_sub_module_port_data = CAM_MALLOC(sizeof(isp_sub_module_port_data_t));
  if (!isp_sub_module_port_data) {
    ISP_ERR("failed: CAM_MALLOC");
    return NULL;
  }
  memset(isp_sub_module_port_data, 0, sizeof(*isp_sub_module_port_data));

  if (direction == MCT_PORT_SINK) {
    snprintf(port_name, sizeof(port_name), "isp_sub_module_sink");
  } else if (direction == MCT_PORT_SRC) {
    snprintf(port_name, sizeof(port_name), "isp_sub_module_src");
  }

  mct_port = mct_port_create(port_name);
  if (!mct_port) {
    ISP_ERR("failed: mct_port_create");
    goto ERROR;
  }

  mct_port->direction = direction;
  ret = mct_module_add_port(module, mct_port);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_module_add_port");
    goto ERROR;
  }

  mct_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;
  isp_sub_module_port_overwrite_funcs(mct_port,
    (void *)isp_sub_module_port_data);

  ISP_ERR("return port %p", mct_port);
  return mct_port;
ERROR:
  free(isp_sub_module_port_data);
  ISP_ERR("return port NULL");
  return NULL;
}

/** isp_sub_module_port_create:
 *
 *  @module: module handle
 *
 *  1) Create one sink port per ISP
 *
 *  2) Create one source port for STATS
 *
 *  3) Create one source port for FRAME
 **/
boolean isp_sub_module_port_create(mct_module_t *module,
  uint32_t num_sink_ports, uint32_t num_source_ports)
{
  boolean  ret = TRUE;
  uint32_t index = 0;

  if (!module) {
    ISP_ERR("failed: invalid params %p", module);
    return FALSE;
  }

  for (index = 0; index < num_sink_ports; index++) {
    /* Create sink port */
    ret = isp_sub_module_port_create_port(module, MCT_PORT_SINK);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_sub_module_port_create_port");
      goto ERROR;
    }
  }

  for (index = 0; index < num_source_ports; index++) {
    /* Create source port */
    ret = isp_sub_module_port_create_port(module, MCT_PORT_SRC);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_sub_module_port_create_port");
      goto ERROR;
    }
  }

  return ret;

ERROR:
  ISP_ERR("failed: isp_sub_module_port_create");
  isp_sub_module_port_delete_ports(module);
  return ret;
}
