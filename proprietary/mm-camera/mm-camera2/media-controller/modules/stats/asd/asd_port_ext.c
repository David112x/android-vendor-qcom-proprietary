/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <dlfcn.h>
#include "asd_port.h"
#include "asd_ext.h"
#include "asd_ext_algo_wrapper.h"

typedef enum {
  ASD_REQ_NONE        = 0,
  ASD_REQ_DEFAULT     = 1 << 0,
  ASD_REQ_OEM         = 1 << 1,
} asd_port_ext_algo_request_t;

/** asd_port_ext_set_update_type:
 *    @asd_update_type: Type of update that OEM need.
 *
 * In this function, OEM will populate asd_update_type. With the type of update
 * that it would like ASD port to provide to HAL and other modules for scene compensation.
 * Configurations:
 * ASD_DETECTION_ONLY_SCENE_INFO: Avoid compensation from other modules,
 *      update is only send to HAL.
 * ASD_DETECTION_FULL_ENABLE: Default. Send updates to HAL and to other modules
 *      for compensation.
 * ASD_DETECTION_DISABLE: Avoid any updates.
 *
 * Return: void
 **/
static void asd_port_ext_set_update_type(
  asd_port_ext_scene_update_type_t *asd_update_type, asd_port_ext_update_type_t update_type)
{
  asd_update_type->snow_scene_detection = update_type;
  asd_update_type->backlight_detection = update_type;
  asd_update_type->landscape_detection = update_type;
  asd_update_type->portrait_detection = update_type;
  asd_update_type->hazy_detection = update_type;
  asd_update_type->auto_hdr_detect = update_type;

  ASD_HIGH("ASD: Update type, snow %d, backlight %d, landscape %d, portrait %d, hazy %d, HDR auto: %d",
    asd_update_type->snow_scene_detection,
    asd_update_type->backlight_detection,
    asd_update_type->landscape_detection,
    asd_update_type->portrait_detection,
    asd_update_type->hazy_detection,
    asd_update_type->auto_hdr_detect);

  return;
}

/** asd_port_ext_fill_module_update:
 *    @private: ASD private port info
 *    @stats_update: include asd_update to be populated in this function
 *    @output: ASD algo output to be use as input to pupulate asd_update
 *
 * Fill asd_update that will be send to other modules
 *
 * Return TRUE on success
 **/
static boolean asd_port_ext_fill_module_update(asd_port_private_t *private,
  stats_update_t *stats_update, asd_output_data_t *output)
{
  uint32_t           *scene_severity = NULL;
  cam_auto_scene_t   scene_idx = 0;
  boolean do_update = TRUE;
  asd_ext_param_t *ext_param = (asd_ext_param_t*)private->ext_param;
  asd_port_ext_scene_update_type_t *asd_update_type =
    &(ext_param->asd_update_type);

  /* Set ASD update type */
  stats_update->flag = STATS_UPDATE_ASD;

  /* Main Enable/Disable switch */
  if (output->asd_enable || private->asd_enable != output->asd_enable) {
    /* update if disable, but only once */
    do_update = TRUE;
  } else {
    /* it's disable already, no need to update again */
    do_update = FALSE;
  }

  private->asd_enable = output->asd_enable;
  stats_update->asd_update.asd_enable = output->asd_enable;

  if (FALSE == output->asd_enable) {
    return do_update;
  }

  /* Backlight */
  if (asd_update_type->backlight_detection == ASD_DETECTION_FULL_ENABLE) {
    stats_update->asd_update.histo_backlight_detected =
      output->histo_backlight_detected;
    stats_update->asd_update.backlight_detected = output->backlight_detected;
    stats_update->asd_update.backlight_luma_target_offset =
      output->backlight_luma_target_offset;
    stats_update->asd_update.backlight_scene_severity =
      output->backlight_scene_severity;
    stats_update->asd_update.mixed_light = output->mixed_light;

    if (output->scene == S_BACKLIGHT) {
      stats_update->asd_update.scene = output->scene;
    }

    stats_update->asd_update.asd_enable = TRUE;
  } else {
    if (output->scene == S_BACKLIGHT) {
      stats_update->asd_update.scene = S_NORMAL;
    } else if (output->scene == S_SCENERY_BACKLIGHT &&
        asd_update_type->landscape_detection == ASD_DETECTION_FULL_ENABLE) {
      stats_update->asd_update.scene = S_SCENERY;
    } else if (output->scene == S_PORTRAIT_BACKLIGHT &&
        asd_update_type->portrait_detection == ASD_DETECTION_FULL_ENABLE) {
      stats_update->asd_update.scene = S_PORTRAIT;
    }
  }

  /* Snow */
  if (asd_update_type->snow_scene_detection == ASD_DETECTION_FULL_ENABLE) {
    stats_update->asd_update.snow_or_cloudy_scene_detected =
      output->snow_or_cloudy_scene_detected;
    stats_update->asd_update.snow_or_cloudy_luma_target_offset =
      output->snow_or_cloudy_luma_target_offset;

    stats_update->asd_update.asd_enable = TRUE;
  }

  /* Landscape */
  if (asd_update_type->landscape_detection == ASD_DETECTION_FULL_ENABLE) {
    stats_update->asd_update.landscape_severity = output->landscape_severity;

    if (output->scene == S_SCENERY) {
      stats_update->asd_update.scene = output->scene;
    } else if (output->scene == S_SCENERY_BACKLIGHT &&
        asd_update_type->backlight_detection == ASD_DETECTION_FULL_ENABLE) {
      stats_update->asd_update.scene = output->scene;
    }
    stats_update->asd_update.asd_enable = TRUE;
  } else {
    if (output->scene == S_SCENERY) {
      stats_update->asd_update.scene = S_NORMAL;
    } else if (output->scene == S_SCENERY_BACKLIGHT &&
        asd_update_type->backlight_detection == ASD_DETECTION_FULL_ENABLE) {
      stats_update->asd_update.scene = S_BACKLIGHT;
    }
  }

  /* Portrait */
  if (asd_update_type->portrait_detection == ASD_DETECTION_FULL_ENABLE) {
    stats_update->asd_update.portrait_severity = output->portrait_severity;
    stats_update->asd_update.asd_soft_focus_dgr = output->soft_focus_dgr;

    if (output->scene == S_PORTRAIT) {
      stats_update->asd_update.scene = output->scene;
    } else if (output->scene == S_PORTRAIT_BACKLIGHT &&
        asd_update_type->backlight_detection == ASD_DETECTION_FULL_ENABLE) {
      stats_update->asd_update.scene = output->scene;
    }
    stats_update->asd_update.asd_enable = TRUE;
  } else {
    if (output->scene == S_PORTRAIT) {
      stats_update->asd_update.scene = S_NORMAL;
    } else if (output->scene == S_PORTRAIT_BACKLIGHT &&
        asd_update_type->backlight_detection == ASD_DETECTION_FULL_ENABLE) {
      stats_update->asd_update.scene = S_BACKLIGHT;
    }
  }

  /* HDR */
  stats_update->asd_update.is_hdr_scene = output->is_hdr_scene;
  stats_update->asd_update.hdr_confidence = output->hdr_confidence;

  if (stats_update->asd_update.asd_enable) {
    /* Severity not currently in use, but fill data for reference */
    scene_severity = &stats_update->asd_update.severity[0];
    for (scene_idx = 0; scene_idx < S_MAX; scene_idx++) {
      scene_severity[scene_idx] = output->severity[scene_idx];
    }
  }

  ASD_LOW("ASD enable: %d / %d, scene: %d, reported_scene: %d, do_update: %d",
    output->asd_enable, stats_update->asd_update.asd_enable,
    output->scene,
    stats_update->asd_update.scene, do_update);

  return do_update;
}

/** asd_port_ext_callback:
 *    @port: port info
 *    @asd_out: Output parameters from core algorithm
 *    @update: ASD updates to be sent to other modules.
 *    @output_handled: mask of output type handled here.
 *
 * Extension of ASD core callback. Here OEM can process output
 * parameters received from AF core. There might be OEM specific
 * parameters as well as standard output which OEM might want to
 * handle on its own.
 *
 * Return one of the return type defined by
 * stats_ext_return_type
 **/
static stats_ext_return_type asd_port_ext_callback(
  mct_port_t *port,
  void *core_out,
  void *update)
{
  asd_port_private_t *private = (asd_port_private_t *)(port->port_private);
  stats_update_t *stats_update = (stats_update_t*)update;
  asd_output_data_t *asd_out = (asd_output_data_t *)core_out;
  stats_ext_return_type ret = STATS_EXT_HANDLING_COMPLETE;
  mct_event_t event;
  mct_bus_msg_asd_decision_t  asd_scene_msg;
  cam_auto_scene_t   idx = S_NORMAL;
  float              scene_confidence = 0.0;
  boolean            do_asd_update = FALSE;


  /* handle parameters as required. */

  /* Verify if update to other modules is required */
  if (ASD_UPDATE == asd_out->type  || ASD_SKIP_UPDATE == asd_out->type) {
    if (asd_out->asd_eztune.ez_running) {
      asd_port_send_asd_info_to_eztune(port, asd_out);
    }

    do_asd_update = asd_port_ext_fill_module_update(private, stats_update, asd_out);

    if (do_asd_update) {
      /* Send ASD modules update*/
      event.direction = MCT_EVENT_UPSTREAM;
      event.identity  = private->reserved_id;
      event.type      = MCT_EVENT_MODULE_EVENT;
      event.u.module_event.type            = MCT_EVENT_MODULE_STATS_ASD_UPDATE;
      event.u.module_event.module_event_data = (void *)(stats_update);
      MCT_PORT_EVENT_FUNC(port)(port, &event);
    }
  }

  /* Verify if update to HAL is required (BUS_MSG) */
  if (ASD_UPDATE == asd_out->type) {
    asd_scene_msg.detected_scene = asd_out->scene;
    asd_scene_msg.max_n_scenes = asd_out->max_n_scenes;
    asd_scene_msg.scene_info[asd_scene_msg.detected_scene].auto_compensation = TRUE;
    asd_scene_msg.scene_info[asd_scene_msg.detected_scene].detected = TRUE;
    for (idx = 0; idx < asd_scene_msg.max_n_scenes; idx++) {
      if (idx != S_HDR) {
        scene_confidence = (float)asd_out->severity[idx];
        scene_confidence = scene_confidence / 255.0f; /* Convert to % */
        asd_scene_msg.scene_info[idx].confidence = scene_confidence;
      }
    }

    /* Algo handles HDR scene independently, fill data here */
    asd_scene_msg.scene_info[S_HDR].detected = asd_out->is_hdr_scene;
    asd_scene_msg.scene_info[S_HDR].confidence = asd_out->hdr_confidence;
    asd_scene_msg.scene_info[S_HDR].auto_compensation = FALSE;

    ASD_LOW("Report scene: %d, is_hdr: %d, hdr_confidence: %f",
      asd_scene_msg.detected_scene,
      asd_scene_msg.scene_info[S_HDR].detected,
      asd_scene_msg.scene_info[S_HDR].confidence);
    /* Save last update, to be use if no new data is available */
    private->last_asd_decision = asd_scene_msg;

    asd_send_bus_message(port, MCT_BUS_MSG_AUTO_SCENE_INFO,
      (void*)&(asd_scene_msg), sizeof(mct_bus_msg_asd_decision_t),
      STATS_REPORT_IMMEDIATE);

    /* Save the debug data in private data struct to be sent out later */
    private->asd_debug_data_size = asd_out->asd_debug_data_size;
    if (asd_out->asd_debug_data_size) {
      STATS_MEMCPY(private->asd_debug_data_array, private->asd_debug_data_size,
        asd_out->asd_debug_data_array, asd_out->asd_debug_data_size);
    }
  } else if (ASD_SEND_EVENT == asd_out->type) {
    /* This will not be invoked from asd thread.
     * Keeping the asd_out->type to keep the interface same as other modules */
  } else if (ASD_SKIP_UPDATE == asd_out->type &&
    (do_asd_update || asd_out->asd_enable)) {
    ASD_LOW("Report scene: %d, is_hdr: %d, hdr_confidence: %f (saved prev scene)",
      private->last_asd_decision.detected_scene,
      private->last_asd_decision.scene_info[S_HDR].detected,
      private->last_asd_decision.scene_info[S_HDR].confidence);
    asd_send_bus_message(port, MCT_BUS_MSG_AUTO_SCENE_INFO,
      (void*)&(private->last_asd_decision), sizeof(mct_bus_msg_asd_decision_t),
      STATS_REPORT_IMMEDIATE);
  }

  return ret;
}

/** asd_port_ext_init:
 *    @port: port info
 *    @session_id: current session id
 *
 * Extension of ASD port init.
 *
 * Return one of the return type defined by stats_ext_return_type
 **/
static stats_ext_return_type asd_port_ext_init(
  mct_port_t *port,
  unsigned int session_id)
{
  (void)session_id;
  asd_port_private_t *private = (asd_port_private_t *)(port->port_private);
  stats_ext_return_type rc = STATS_EXT_HANDLING_PARTIAL;
  asd_ext_param_t *ext_param = NULL;

  ASD_HIGH("Extension init");
  ext_param = (asd_ext_param_t *) calloc(1, sizeof(asd_ext_param_t));
  if (NULL == ext_param) {
    ASD_ERR("Error allocating memory for extension init!");
    rc = STATS_EXT_HANDLING_FAILURE;
    return rc;
  }

  /* Output of core algorithm will have void pointer to point to custom output
     parameters. Assign space to save those parameters. */
  private->asd_object.output.asd_custom_output.data =
    (void *)&ext_param->custom_output;
  private->asd_object.output.asd_custom_output.size =
    sizeof(ext_param->custom_output);
  private->ext_param = ext_param; /* keep reference to this memory */


  /* Save the type of update required from ASD */
  asd_port_ext_set_update_type(&(ext_param->asd_update_type), ASD_DETECTION_ONLY_SCENE_INFO);
  return rc;
}

/** asd_port_ext_deinit:
 *    @port: port info
 *
 * Extension of ASD port de-init.
 *
 * Return: none
 **/
static void asd_port_ext_deinit(mct_port_t *port)
{
  asd_port_private_t *private = (asd_port_private_t *)(port->port_private);
  if (private->ext_param) {
    free(private->ext_param);
    private->ext_param = NULL;
  }

  ASD_HIGH("Extension deinit");
  return;
}

/** asd_port_ext_handle_control_event:
 *    @port: port info
 *    @ctrl_evt: control event
 *
 * Extension of control event handling. Here OEM can further
 * handle/process control events. The control events can be OEM
 * specific or general. If it's OEM specific, OEM can either
 * process it here and send to core algorithm if required; or
 * just send the payload to core algorithm to process.
 *
 * Return stats_ext_return_type value.
 */
static stats_ext_return_type asd_port_ext_handle_control_event(
  mct_port_t *port,
  mct_event_control_t *ctrl_evt)
{
  (void)ctrl_evt;
  asd_port_private_t *asd_port =
    (asd_port_private_t *)(port->port_private);
  stats_ext_return_type ret = STATS_EXT_HANDLING_PARTIAL;

  switch (ctrl_evt->type) {
  case MCT_EVENT_CONTROL_SET_PARM: {
    stats_set_params_type *stat_parm =
      (stats_set_params_type *)ctrl_evt->control_event_data;

    /* If it's common params shared by many modules */
    if (stat_parm->param_type == STATS_SET_COMMON_PARAM) {
      stats_common_set_parameter_t *common_param =
        &(stat_parm->u.common_param);
      boolean asd_enable = FALSE;
      if (common_param->type == COMMON_SET_PARAM_BESTSHOT) {
        ASD_LOW("MSG_ASD_SET: Enable/disable ASD? %d, HAL(best_shot) %d",
          asd_enable, common_param->u.bestshot_mode);
        asd_ext_param_t *ext_param = (asd_ext_param_t *)asd_port->ext_param;
        if (common_param->u.bestshot_mode == CAM_SCENE_MODE_AUTO) {
           // set to full mode
           asd_port_ext_set_update_type(&ext_param->asd_update_type, ASD_DETECTION_FULL_ENABLE);
        } else if (common_param->u.bestshot_mode == CAM_SCENE_MODE_FACE_PRIORITY){
           // set to partial mode
           asd_port_ext_set_update_type(&ext_param->asd_update_type, ASD_DETECTION_ONLY_SCENE_INFO);
        } else {
           // disable it
           asd_port_ext_set_update_type(&ext_param->asd_update_type, ASD_DETECTION_DISABLE);
        }
      }
    }
  }
    break;
  default:
    break;
  }

  ASD_LOW("Handled %d, control event: %d", ret, ctrl_evt->type);
  return ret;
}

/** asd_port_ext_handle_module_event:
 *    @port: port info
 *    @ctrl_evt: control event
 *
 * Extension of module event handling. Here OEM can further
 * handle/process module events. The module events can be OEM
 * specific or general. If it's OEM specific, OEM can either
 * process it here and send to core algorithm if required; or
 * just send the payload to core algorithm to process.
 *
 * Return one of the return type defined by
 * stats_ext_return_type
 **/
static stats_ext_return_type asd_port_ext_handle_module_event(
  mct_port_t *port,
  mct_event_module_t *mod_evt)
{
  (void)mod_evt;
  stats_ext_return_type ret = STATS_EXT_HANDLING_PARTIAL;
  asd_port_private_t *asd_port = (asd_port_private_t *)(port->port_private);

  ASD_LOW("Handled %d, module event: %d", ret, mod_evt->type);
  return ret;
}

/** asd_port_ext_unload_oem_iface:
 *
 * OEM should modify this function to unload the dynamic link library
 * and free resourced allocated at asd_port_ext_load_oem_iface()
 *
 * Return: The handle to the algo obtained via dlopen()
 **/
static void asd_port_ext_unload_oem_iface(asd_ops_t *asd_ops, void *asd_handle)
{
  if (asd_handle) {
    dlclose(asd_handle);
  }

  asd_ops->set_parameters = NULL;
  asd_ops->get_parameters = NULL;
  asd_ops->process = NULL;
  asd_ops->init = NULL;
  asd_ops->deinit = NULL;

  ASD_HIGH("Extension unload");
  return;
}

/** asd_port_ext_load_oem_iface:
 *
 * OEM should modify this function to load the dynamic link library
 * and init the interface to the OEM algo.
 *
 * Return: The handle to the algo obtained via dlopen()
 **/
static void * asd_port_ext_load_oem_iface(asd_ops_t *asd_ops)
{
  /* OEM must populate this function pointers */
  asd_ops->set_parameters = NULL;
  asd_ops->get_parameters = NULL;
  asd_ops->process = NULL;
  asd_ops->init = NULL;
  asd_ops->deinit = NULL;

  ASD_HIGH("Extension load iface");
  return NULL;
}

/** asd_port_ext_required_algos:
 *
 * OEM should modify this function to add ASD_REQ_OEM if needed to the list
 * of return values.
 *
 * Return: The list of algos to be enable in bitwise format falgs type:
 *    asd_port_ext_algo_request_t
 **/
static asd_port_ext_algo_request_t asd_port_ext_required_algos()
{
  asd_port_ext_algo_request_t asd_req_algos;
  /* OEM should modify this function to add ASD_REQ_OEM if needed to the list
     of return values.
     ex. return ASD_REQ_DEFAULT | ASD_REQ_OEM */

  asd_req_algos = ASD_REQ_DEFAULT;

  ASD_HIGH("Extension req algos: 0x%x", asd_req_algos);
  return asd_req_algos;
}

/** asd_port_ext_update_port_func_table:
 *    @ptr: pointer to internal aec pointer object
 *
 * Update extendable function pointers as needed.
 *
 * Return: True on success
 **/
boolean asd_port_ext_update_port_func_table(void *ptr)
{
  asd_port_private_t *private = (asd_port_private_t*)ptr;
  if (private) {
    private->func_tbl.ext_init = asd_port_ext_init;
    private->func_tbl.ext_deinit = asd_port_ext_deinit;
    private->func_tbl.ext_callback = asd_port_ext_callback;
    private->func_tbl.ext_handle_module_event = asd_port_ext_handle_module_event;
    private->func_tbl.ext_handle_control_event = asd_port_ext_handle_control_event;
  }

  return TRUE;
}

/** asd_port_ext_is_extension_required:
 *    @private: pointer to private data of ASD port
 *
 * Helper function to know if extension is required.
 * OEM should modify this function to enable ASD port extension.
 *
 * Return: True if extension is required.
 **/
boolean asd_port_ext_is_extension_required(asd_port_private_t *private)
{
  /* Decide if ASD port extension is requried or not by adding custom logic */
  (void) private;
  boolean rc = TRUE;
  ASD_HIGH("ext required? %s", rc ? "Yes" : "No");
  return rc;
}

void asd_port_ext_unload_interface(asd_ops_t *asd_ops, void* asd_handle)
{
  asd_wrapper_iface_handle_t *iface_handle =
    (asd_wrapper_iface_handle_t*) asd_handle;

  if (NULL == iface_handle) {
    return;
  }

  if (iface_handle->dafault_lib_handle) {
    asd_port_unload_interface(&(iface_handle->default_ops),
      iface_handle->dafault_lib_handle);
  }
  if (iface_handle->oem_lib_handle) {
    asd_port_ext_unload_oem_iface(&(iface_handle->oem_ops),
      iface_handle->oem_lib_handle);
  }

  asd_wrapper_unlink_algo(asd_ops);
  free(iface_handle);

  ASD_HIGH("Extension unload interface");
  return;
}

void * asd_port_ext_load_interface(asd_object_t *asd_object)
{
  asd_wrapper_iface_handle_t *iface_handle = NULL;
  asd_port_ext_algo_request_t algo_req = ASD_REQ_NONE;

  iface_handle = calloc(1, sizeof(asd_wrapper_iface_handle_t));
  if (NULL == iface_handle) {
    return NULL;
  }

  ASD_HIGH("Extension load iface: %p", iface_handle);
  do {
    algo_req = asd_port_ext_required_algos();

    if (!algo_req) {
      ASD_ERR("Error: No ASD algo selected");
      break;
    }

    if (ASD_REQ_DEFAULT & algo_req) {
      iface_handle->dafault_lib_handle =
        asd_port_load_interface(&(iface_handle->default_ops));
      if (NULL == iface_handle->dafault_lib_handle) {
        ASD_ERR("Fail to load default algo");
        break;
      }
    }

    if (ASD_REQ_OEM & algo_req) {
      iface_handle->oem_lib_handle =
        asd_port_ext_load_oem_iface(&(iface_handle->oem_ops));
      if (NULL == iface_handle->oem_lib_handle) {
        ASD_ERR("Fail to load ASD OEM algo");
        break;
      }
    }

    asd_wrapper_link_algo(&(asd_object->asd_ops));

    return iface_handle;
  } while (0);

  /* Handle errors */
  if ((ASD_REQ_DEFAULT & algo_req) && iface_handle->dafault_lib_handle) {
    asd_port_unload_interface(&(iface_handle->default_ops),
      iface_handle->dafault_lib_handle);
  }
  if ((ASD_REQ_OEM & algo_req) && iface_handle->oem_lib_handle) {
    asd_port_ext_unload_oem_iface(&(iface_handle->oem_ops),
      iface_handle->oem_lib_handle);
  }

  if (iface_handle) {
    free(iface_handle);
    iface_handle = NULL;
  }

  return NULL;
}
