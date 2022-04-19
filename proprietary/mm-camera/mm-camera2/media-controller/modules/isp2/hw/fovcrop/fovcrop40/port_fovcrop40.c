/* port_fovcrop40.c
 *
 * Copyright (c) 2012-2014, 2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <stdio.h>
#include <pthread.h>

/* mctl headers */
#include "media_controller.h"
#include "mct_stream.h"

/* ISP headers*/
#include "isp_sub_module_util.h"
#include "isp_log.h"
#include "isp_defs.h"
#include "module_fovcrop40.h"
#include "port_fovcrop40.h"
#include "fovcrop40.h"
#include "fovcrop40_util.h"


/** port_fovcrop40_set_crop_window:
 *
 *  @module:  mct module handle
 *  @fovcrop: fovcrop handle
 *  @identity: identity
 *
 *  Get modified crop window from scalar and configure the FOV
 *  module
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_fovcrop40_set_crop_window(mct_module_t *module,
  fovcrop40_t *fovcrop, uint32_t identity)
{
  mct_event_t         mct_event;
  isp_private_event_t private_event;

  /* Need to update a flag to differentiate between
     Crop window or crop value based FOV config */
  fovcrop->crop_window_based = TRUE;

  /*Get  scalar's output info */
  ISP_DBG("E: Get  fovcrop's output info");
  memset(&private_event, 0, sizeof(isp_private_event_t));
  private_event.type = ISP_PRIVATE_FETCH_SCALER_OUTPUT;
  private_event.data = (void *)&fovcrop->scaler_output;
  private_event.data_size = sizeof(fovcrop->scaler_output);

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.identity = identity;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.direction = MCT_EVENT_UPSTREAM;
  mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  mct_event.u.module_event.module_event_data = (void *)&private_event;
  fovcrop40_util_send_event(module, &mct_event);
  ISP_DBG(" scalar output: Width %d, Height %d, scalefactor %f",
    fovcrop->scaler_output.width,
    fovcrop->scaler_output.height,
    fovcrop->scaler_output.scaling_factor);
  ISP_DBG("Scalar output(crop_window) [%d,%d,%d,%d]\n",
    fovcrop->scaler_output.modified_crop_window.left,
    fovcrop->scaler_output.modified_crop_window.top,
    fovcrop->scaler_output.modified_crop_window.width,
    fovcrop->scaler_output.modified_crop_window.height);

  fovcrop40_config_crop_window(fovcrop);
  ISP_DBG("X: ret = TRUE");
  return TRUE;
}


/** port_fovcrop40_set_fovcrop:
 *
 *  @module:  mct module handle
 *  @fovcrop: fovcrop handle
 *  @identity: identity
 *
 *  Get fovcrop output, fov crop configuration from fovcrop
 *  module
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_fovcrop40_config(mct_module_t *module,
  fovcrop40_t *fovcrop, uint32_t identity)
{
  mct_event_t         mct_event;
  isp_private_event_t private_event;
  boolean             ret = FALSE;

  if (!module || !fovcrop) {
    ISP_ERR("module %p fovcrop %p", module, fovcrop);
    return FALSE;
  }

  /*Get  fovcrop's output info */
  ISP_DBG("E: Get  fovcrop's output info");
  memset(&private_event, 0, sizeof(isp_private_event_t));
  private_event.type = ISP_PRIVATE_FETCH_SCALER_OUTPUT;
  private_event.data = (void *)&fovcrop->scaler_output;
  private_event.data_size = sizeof(fovcrop->scaler_output);

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.identity = identity;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.direction = MCT_EVENT_UPSTREAM;
  mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  mct_event.u.module_event.module_event_data = (void *)&private_event;
  fovcrop40_util_send_event(module, &mct_event);
  ISP_DBG(" fovcrop_output: Width %d, Height %d, scalefactor %f",
    fovcrop->scaler_output.width,
    fovcrop->scaler_output.height,
    fovcrop->scaler_output.scaling_factor);

  /*Fetch info about hw stream output supported by fovcrop*/
  ISP_DBG("E: Fetch HW STREAM INFO");
  memset(&private_event, 0, sizeof(isp_private_event_t));
  private_event.type =   ISP_PRIVATE_FETCH_SCALER_HW_STREAM_INFO;
  private_event.data = (void *)&fovcrop->hw_stream_info;
  private_event.data_size = sizeof(fovcrop->hw_stream_info);

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.identity = identity;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.direction = MCT_EVENT_UPSTREAM;
  mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  mct_event.u.module_event.module_event_data = (void *)&private_event;
  fovcrop40_util_send_event(module, &mct_event);
  ISP_DBG("hw stream info width %d height %d fmt %d identity %x",
    fovcrop->hw_stream_info.width, fovcrop->hw_stream_info.height,
    fovcrop->hw_stream_info.fmt, fovcrop->hw_stream_info.identity);

  /*Fetch info about hw stream output supported by fovcrop*/
  ISP_DBG("E: Fetch crop factor");
  memset(&private_event, 0, sizeof(isp_private_event_t));
  private_event.type = ISP_PRIVATE_FETCH_CROP_FACTOR;
  private_event.data = (void *)&fovcrop->crop_factor;
  private_event.data_size = sizeof(fovcrop->crop_factor);

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.identity = identity;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.direction = MCT_EVENT_UPSTREAM;
  mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  mct_event.u.module_event.module_event_data = (void *)&private_event;
  fovcrop40_util_send_event(module, &mct_event);
  ISP_DBG("crop factor %d", fovcrop->crop_factor);

  /*After get all the scaler output info needed, config fovcrop*/
  if(fovcrop->crop_window_based) {
    ret = fovcrop40_config_crop_window(fovcrop);
    if (ret == FALSE) {
      ISP_ERR("fovcrop40_config failed! ret = FALSE");
      return FALSE;
    }
  } else {
    ret = fovcrop40_config(fovcrop);
    if (ret == FALSE) {
      ISP_ERR("fovcrop40_config failed! ret = FALSE");
      return FALSE;
    }
  }

  ISP_DBG("X: ret = TRUE");
  return TRUE;

}

/**  port_fovcrop40_handle_set_stream_config:
 *
 *  @data1: mct stream info handle
 *
 *  @data2: streaming mode mask
 *
 *  Append streaming mode of stream info in streaming mode
 *  mask
 *
 *  Return TRUE
 **/
static boolean port_fovcrop40_handle_set_stream_config(mct_module_t *module,
  fovcrop40_t *fovcrop, mct_event_t *event)
{
  boolean             ret = TRUE;
  sensor_out_info_t  *sensor_out_info = NULL;
  mct_event_t         mct_event;
  isp_private_event_t private_event;

  ISP_HIGH("E: fovcrop40: %p event: %p", fovcrop, event);
  if (!fovcrop || !event) {
    ISP_ERR("failed: %p %p", fovcrop, event);
    return FALSE;
  }

  sensor_out_info = event->u.module_event.module_event_data;
  if (!sensor_out_info) {
    ISP_ERR("failed: sensor_out_info %p", sensor_out_info);
    return FALSE;
  }
  fovcrop->sensor_out_info = *sensor_out_info;

  fovcrop->is_bayer_sensor = isp_sub_module_is_bayer_mono_fmt(
    fovcrop->sensor_out_info.fmt);

  ret = port_fovcrop40_config(module, fovcrop, event->identity);

  if (ret == FALSE) {
    ISP_ERR("failed: port_fovcrop40_get_fovcrop_info");
  }

  return ret;
}

/**  port_fovcrop40_handle_cds_request:
 *
 *  @data1: mct stream info handle
 *
 *  @data2: streaming mode mask
 *
 *  Append streaming mode of stream info in streaming mode
 *  mask
 *
 *  Return TRUE
 **/
boolean port_fovcrop40_handle_cds_request(mct_module_t *module,
  fovcrop40_t *fovcrop, mct_event_t *event)
{
  boolean             ret = TRUE;
  isp_cds_request_t   *isp_cds_request = NULL;
  mct_event_t         mct_event;

  ISP_HIGH("E: fovcrop40: %p event: %p", fovcrop, event);
  if (!fovcrop || !event) {
    ISP_ERR("failed: %p %p", fovcrop, event);
    return FALSE;
  }

  isp_cds_request = event->u.module_event.module_event_data;
  if (!isp_cds_request) {
    ISP_ERR("failed: isp_cds_request %p", isp_cds_request);
    return FALSE;
  }

  fovcrop->hw_stream_info.need_uv_subsample =
    isp_cds_request->cds_request_info.need_cds_subsample;

  ret = port_fovcrop40_config(module, fovcrop, event->identity);
  if (ret == FALSE) {
    ISP_ERR("failed: port_fovcrop40_get_fovcrop_info");
  }

  /* set cds request when its */
  fovcrop->is_cds_update = isp_cds_request->cds_request;

  return ret;
}

/** port_fovcrop40_update_streaming_mode:
 *
 *  @data1: mct stream info handle
 *
 *  @data2: streaming mode mask
 *
 *  Append streaming mode of stream info in streaming mode
 *  mask
 *
 *  Return TRUE
 **/
static boolean port_fovcrop40_update_streaming_mode(void *data1, void *data2)
{
  mct_stream_info_t *stream_info = (mct_stream_info_t *)data1;
  uint32_t          *streaming_mode_mask = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return TRUE;
  }

  ISP_APPEND_STREAMING_MODE(*streaming_mode_mask, stream_info->streaming_mode);
  return TRUE;
}

/** port_fovcrop40_handle_streamon:
 *
 *  @fovcrop: fovcrop handle
 *
 *  @event: event to be handled
 *
 *  Handle STREAM ON event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_fovcrop40_handle_streamon(mct_module_t *module,
  fovcrop40_t *fovcrop, mct_event_t *event)
{
  boolean ret = TRUE;

  if (!fovcrop || !event) {
    ISP_ERR("failed: fovcrop %p event %p", fovcrop, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&fovcrop->mutex);
  /* Update streaming mode mask in module private */
  mct_list_traverse(fovcrop->l_stream_info,
    port_fovcrop40_update_streaming_mode, &fovcrop->streaming_mode_mask);

  ret = port_fovcrop40_config(module, fovcrop, event->identity);
  if (ret == FALSE) {
    ISP_ERR("failed: port_fovcrop40_get_fovcrop_info");
  }

  PTHREAD_MUTEX_UNLOCK(&fovcrop->mutex);
  return ret;
}

/** port_fovcrop40_handle_streamoff:
 *
 *  @fovcrop: fovcrop handle
 *
 *  @event: event to be handled
 *
 *  Handle STREAM OFF event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_fovcrop40_handle_streamoff(fovcrop40_t *fovcrop,
  mct_event_t *event)
{
  boolean       ret = TRUE;

  if (!fovcrop || !event) {
    ISP_ERR("failed: fovcrop %p event %p", fovcrop, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&fovcrop->mutex);
  /* Reset streaming mode */
  ISP_RESET_STREAMING_MODE_MASK(fovcrop->streaming_mode_mask);
  fovcrop->crop_window_based = FALSE;

  PTHREAD_MUTEX_UNLOCK(&fovcrop->mutex);
  return ret;
}

/** port_fovcrop40_forward_event:
 *
 *  @port: port handle
 *
 *  @event: event to be forwarded
 *
 *  Forward event based on port direction and event direction
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_fovcrop40_forward_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean                 ret = TRUE;
  port_fovcrop40_data_t *port_data = NULL;

  if (!port || !event) {
    ISP_ERR("failed: port %p event %p", port, event);
    return FALSE;
  }

  if (((event->direction == MCT_EVENT_UPSTREAM) &&
       (port->direction != MCT_PORT_SRC)) ||
      ((event->direction == MCT_EVENT_DOWNSTREAM) &&
       (port->direction != MCT_PORT_SINK))) {
    ISP_ERR("failed: invalid types, event dir %d port dir %d",
      event->direction, port->direction);
    return FALSE;
  }

  port_data = MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    return FALSE;
  }

  if (!port_data->int_peer_port) {
    return TRUE;
  }

  ret = mct_port_send_event_to_peer(port_data->int_peer_port, event);
  if (ret == FALSE) {
    if (event->type == MCT_EVENT_CONTROL_CMD) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        event->u.ctrl_event.type);
    } else {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        event->u.module_event.type);
    }
  }
  return ret;
}

/** port_fovcrop40_handle_cds_mode
 *
 *
 *
 *  @fovcrop: fovcrop module provte
 *  @cds_mode: cam_cds_mode_type_t type
 *
 **/
void port_fovcrop40_handle_cds_mode(fovcrop40_t *fovcrop,
  cam_cds_mode_type_t *cds_mode)
{
  if (*cds_mode == CAM_CDS_MODE_ON ||
    *cds_mode == CAM_CDS_MODE_AUTO) {
    fovcrop->cds_enabled = TRUE;
  }
}

/**  port_fovcrop40_handle_hal_set_parm:
 *
 *  @fovcrop: fovcrop module private
 *  @event: mct event
 *
 *  Append streaming mode of stream info in streaming mode
 *  mask
 *
 *  Return TRUE
 **/
boolean port_fovcrop40_handle_hal_set_parm(mct_module_t *module,
  fovcrop40_t *fovcrop, uint32_t identity, mct_event_control_parm_t *hal_param)
{
  boolean                   ret = TRUE;

  if (!fovcrop || !hal_param) {
    ISP_ERR("failed: %p %p", fovcrop, hal_param);
    return FALSE;
  }

  switch (hal_param->type) {
  case CAM_INTF_PARM_USERZOOM: {
    ret = port_fovcrop40_config(module, fovcrop, identity);
    if (ret == FALSE) {
      ISP_ERR("failed: port_fovcrop40_get_fovcrop_info");
    }
  }
    break;

  case CAM_INTF_META_SCALER_CROP_REGION:{
    ret = port_fovcrop40_set_crop_window(module, fovcrop, identity);
    if (ret == FALSE) {
      ISP_ERR("failed: port_fovcrop40_get_fovcrop_info");
    }
  }
  break;

  case CAM_INTF_PARM_DIS_ENABLE: {
    ret = port_fovcrop40_config(module, fovcrop, identity);
    if (ret == FALSE) {
      ISP_ERR("failed: port_fovcrop40_config DIS failed");
    }
  }
   break;

  case CAM_INTF_PARM_CDS_MODE: {
    port_fovcrop40_handle_cds_mode(fovcrop,
      (cam_cds_mode_type_t*)hal_param->parm_data);
  }
    break;

  default:
    break;
  }

  return ret;
}

static boolean port_fovcrop40_handle_set_super_parm(mct_module_t *module,
  fovcrop40_t *fovcrop, mct_event_t *event)
{
  boolean                         ret = TRUE,
                                  func_ret = TRUE;
  uint32_t                        i = 0;
  mct_event_super_control_parm_t *super_param = NULL;

  if (!fovcrop || !event) {
    ISP_ERR("failed: fovcrop %p event %p", fovcrop, event);
    return FALSE;
  }

  super_param =
    (mct_event_super_control_parm_t *)event->u.ctrl_event.control_event_data;
  if (!super_param) {
    ISP_ERR("failed: super_param %p", super_param);
    return FALSE;
  }

  for (i = 0; i < super_param->num_of_parm_events; i++) {
    ret = port_fovcrop40_handle_hal_set_parm(module, fovcrop,
      event->identity, &super_param->parm_events[i]);
    if (ret == FALSE) {
      ISP_ERR("failed: port_fovcrop40_handle_hal_set_parm type %d",
        super_param->parm_events[i].type);
      func_ret = FALSE;
    }
  }
  return func_ret;
}

/** port_fovcrop40_process_downstream_ctrl_event:
 *
 *  @module: module handle
 *
 *  @port: port handle
 *
 *  @fovcrop40: fovcrop40 handle
 *
 *  @event: event to be processed
 *
 *  Handle downstream ctrl event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_fovcrop40_process_downstream_ctrl_event(
  mct_module_t *module, mct_port_t *port, fovcrop40_t *fovcrop,
  mct_event_t *event)
{
  boolean              ret = TRUE;
  mct_event_control_t *ctrl_event = NULL;

  if (!module || !port || !fovcrop || !event) {
    ISP_ERR("failed: %p %p %p %p", module, port, fovcrop, event);
    return FALSE;
  }

  /* Extract ctrl event */
  ctrl_event = &event->u.ctrl_event;

  ISP_HIGH("ide %x event %d mod %s", event->identity,
     ctrl_event->type, MCT_MODULE_NAME(module));
  switch (ctrl_event->type) {
  case MCT_EVENT_CONTROL_SET_PARM:
    ret = port_fovcrop40_handle_hal_set_parm(module, fovcrop, event->identity,
      ctrl_event->control_event_data);
    if (ret == FALSE) {
      ISP_ERR("failed: port_fovcrop40_handle_streamon");
    }
    ret = port_fovcrop40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;

  case MCT_EVENT_CONTROL_SET_SUPER_PARM:
    ret = port_fovcrop40_handle_set_super_parm(module, fovcrop, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_fovcrop40_handle_set_super_parm");
    }
    ret = port_fovcrop40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;

  case MCT_EVENT_CONTROL_STREAMON:
    ret = port_fovcrop40_handle_streamon(module, fovcrop, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_fovcrop40_handle_streamon");
    }

    /* Forward event to FRAME port */
    ret = port_fovcrop40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;

  case MCT_EVENT_CONTROL_STREAMOFF:
    ret = port_fovcrop40_handle_streamoff(fovcrop, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_fovcrop40_handle_streamon");
    }

    /* Forward event to FRAME port */
    ret = port_fovcrop40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;

  default:
    /* Forward event to FRAME port */
    ret = port_fovcrop40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;
  }
  return ret;
}

/** port_fovcrop40_process_downstream_module_event:
 *
 *  @module: module handle
 *
 *  @port: port handle
 *
 *  @fovcrop40: fovcrop40 handle
 *
 *  @event: event to be processed
 *
 *  Handle downstream ctrl event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_fovcrop40_process_downstream_module_event(
  mct_module_t *module, mct_port_t *port, fovcrop40_t *fovcrop,
  mct_event_t *event)
{
  boolean              ret = TRUE;
  mct_event_module_t  *module_event = NULL;

  if (!module || !port || !fovcrop || !event) {
    ISP_ERR("failed: %p %p %p %p", module, port, fovcrop, event);
    return FALSE;
  }

  /* Extract ctrl event */
  module_event = &event->u.module_event;
  ISP_HIGH("ide %x event %d mod %s", event->identity,
     module_event->type, MCT_MODULE_NAME(module));
  switch (module_event->type) {
  case MCT_EVENT_MODULE_SET_STREAM_CONFIG:
    ret = port_fovcrop40_handle_set_stream_config(module, fovcrop, event);
    if (ret == FALSE) {
      ISP_ERR("failed: fovcrop40_handle_set_stream_config");
    }
    /* Forward event */
    ret = port_fovcrop40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;

  case MCT_EVENT_MODULE_ISP_CDS_REQUEST:
    ret = port_fovcrop40_handle_cds_request(module, fovcrop, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_fovcrop40_handle_cds_request");
    } else {
       /* Forward event only when the config and validate is good*/
       ret = port_fovcrop40_forward_event(port, event);
       if (ret == FALSE) {
         ISP_ERR("failed: mct_port_send_event_to_peer event %d",
           module_event->type);
       }
    }
    break;
  case MCT_EVENT_MODULE_ISP_PRIVATE_EVENT: {
    ISP_HIGH("MCT_EVENT_MODULE_ISP_PRIVATE_EVENT");

    isp_private_event_t *private_event = (isp_private_event_t *)module_event->module_event_data;

    switch (private_event->type) {
    case ISP_PRIVATE_SET_STREAM_SPLIT_INFO: {
      isp_out_info_t *isp_out_info =
        (isp_out_info_t *)private_event->data;
      fovcrop->isp_out_info = *isp_out_info;

      ret = port_fovcrop40_config(module, fovcrop, event->identity);

      if (ret == FALSE) {
        ISP_ERR("failed: port_fovcrop40_get_fovcrop_info");
      }
    }
    break;

    case ISP_PRIVATE_REQUEST_STREAM_SPLIT_INFO: {
      isp_out_info_t *isp_out_info =
        (isp_out_info_t *)private_event->data;
      isp_out_info->is_split = fovcrop->ispif_out_info.is_split;
      uint32_t sensor_output_width =
        fovcrop->sensor_out_info.request_crop.last_pixel -
        fovcrop->sensor_out_info.request_crop.first_pixel + 1;

      /* Calculate left output width based on each stream split input*/
      isp_out_info->left_output_width = EVEN_CEIL((fovcrop->hw_stream_info.width *
        fovcrop->min_left_input) / sensor_output_width);

      /* consider CDS and AXI alignment,
         fov need to output correct alignment */
      isp_out_info->left_output_width =
        PAD_TO_SIZE(isp_out_info->left_output_width / 2, 16) * 2;
      isp_out_info->right_output_width =
        fovcrop->hw_stream_info.width - isp_out_info->left_output_width;
      /*right stripe offset for ISP should not consider sensor request crop*/
      isp_out_info->right_stripe_offset =
        fovcrop->ispif_out_info.right_stripe_offset -
        fovcrop->sensor_out_info.request_crop.first_pixel;
    }
    break;

    default:
      ret = fovcrop40_handle_isp_private_event(module, fovcrop,
      module_event->module_event_data, event->identity);
      if (ret == FALSE) {
        ISP_ERR("failed: MCT_EVENT_MODULE_ISP_PRIVATE_EVENT");
      }
      break;
    }

    /* Forward event */
    ret = port_fovcrop40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
  }
    break;


  default:
    /* Forward event */
    ret = port_fovcrop40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;
  }
  return ret;
}

/** port_fovcrop40_process_downstream_event:
 *
 *  @port: port on which this event arrived
 *
 *  @event: event to be handled
 *
 *  Handle downstream event
 *
 *  Returns TRUE on success and FALSE on failure
 **/
boolean port_fovcrop40_process_downstream_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean       ret = TRUE;
  mct_module_t *module = NULL;
  fovcrop40_t *fovcrop40 = NULL;

  if (!port || !event) {
    ISP_ERR("failed: port %p event %p", port, event);
    return FALSE;
  }

  if ((event->type != MCT_EVENT_CONTROL_CMD) &&
      (event->type != MCT_EVENT_MODULE_EVENT)) {
    ISP_ERR("failed: invalid event type %d", event->type);
    return FALSE;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  /* Extract module private */
  fovcrop40 = (fovcrop40_t *)MCT_OBJECT_PRIVATE(module);
  if (!fovcrop40) {
    ISP_ERR("failed: fovcrop40 %p", fovcrop40);
    return FALSE;
  }

  if (event->type == MCT_EVENT_CONTROL_CMD) {
    ret = port_fovcrop40_process_downstream_ctrl_event(module, port, fovcrop40,
      event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_fovcrop40_process_downstream_ctrl_event");
    }
  } else if (event->type == MCT_EVENT_MODULE_EVENT) {
    ret = port_fovcrop40_process_downstream_module_event(module, port,
      fovcrop40, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_fovcrop40_process_downstream_module_event");
    }
  }
  return ret;
}

/** port_fovcrop40_process_upstream_event:
 *
 *  @port: port on which this event arrived
 *
 *  @event: event to be handled
 *
 *  Handle downstream event
 *
 *  Returns TRUE on success and FALSE on failure
 **/
boolean port_fovcrop40_process_upstream_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean              ret = TRUE;
  mct_module_t        *module = NULL;
  fovcrop40_t        *fovcrop = NULL;
  mct_event_module_t  *module_event = NULL;

  if (!port || !event) {
    ISP_ERR("failed: port %p event %p", port, event);
    return FALSE;
  }

  if (event->type != MCT_EVENT_MODULE_EVENT) {
    ISP_ERR("failed: invalid event type %d", event->type);
    return FALSE;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  /* Extract module private */
  fovcrop = MCT_OBJECT_PRIVATE(module);
  if (!fovcrop) {
    ISP_ERR("failed: fovcrop40 %p", fovcrop);
    return FALSE;
  }

  module_event = &event->u.module_event;
  ISP_HIGH("ide %x event %d mod %s", event->identity,
     module_event->type, MCT_MODULE_NAME(module));

  switch (module_event->type) {
  case MCT_EVENT_MODULE_ISP_PRIVATE_EVENT: {
    ret = fovcrop40_handle_isp_private_event(module, fovcrop,
      module_event->module_event_data, event->identity);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_ISP_PRIVATE_EVENT");
    }
  }
    break;

  default:
    /* Forward event to upstream FRAME port */
    ret = port_fovcrop40_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;
  }

  return ret;
}

/** port_fovcrop40_event:
 *
 *  @port: port handle
 *
 *  @event: event to be processed
 *
 *  Handle ISP event based on direction
 *
 *  Returns TRUE on success and FALSE on failure */
static boolean port_fovcrop40_event(mct_port_t *port, mct_event_t *event)
{
  boolean       ret = TRUE;
  mct_module_t *module = NULL;
  fovcrop40_t *fovcrop40 = NULL;

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

  fovcrop40 = MCT_OBJECT_PRIVATE(module);
  if (!fovcrop40) {
    ISP_ERR("failed: fovcrop40 %p", fovcrop40);
    return FALSE;
  }

  if (event->direction == MCT_EVENT_DOWNSTREAM) {
    ret = port_fovcrop40_process_downstream_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_fovcrop40_process_downstream_event");
    }
  } else {
    ret = port_fovcrop40_process_upstream_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_fovcrop40_process_upstream_event");
    }
  }

  return ret;
}

/** port_fovcrop40_ext_link:
 *
 *  @identity: identity of stream
 *
 *  @port: port handle
 *
 *  @peer: peer handle
 *
 *  Handle ext link
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean port_fovcrop40_ext_link(unsigned int identity, mct_port_t* port,
  mct_port_t *peer)
{
  boolean                 ret = TRUE;
  mct_module_t           *module = NULL;
  mct_port_t             *sink_port = NULL;
  port_fovcrop40_data_t *port_data = NULL;
  port_fovcrop40_data_t *port_sink_data = NULL;

  if (!port || !peer) {
    ISP_ERR("failed: port %p peer %p", port, peer);
    return FALSE;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(port));

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

  if (MCT_PORT_IS_SRC(port) == TRUE) {

    /* Find sink port with same identity */
    ret = fovcrop40_util_get_port_from_module(
      (mct_module_t *)(MCT_PORT_PARENT(port)->data), &sink_port, MCT_PORT_SINK,
      identity);
    if ((ret == FALSE) || !sink_port) {
      ISP_ERR("failed: fovcrop40_util_get_port_from_module ret %d port %p",
        ret, sink_port);
      goto ERROR;
    }

    if (port_data->int_peer_port &&
       (port_data->int_peer_port != sink_port)) {
      ISP_ERR("failed: source port linked with different sink port");
      goto ERROR;
    } else {
      port_sink_data = MCT_OBJECT_PRIVATE(sink_port);
      if (!port_sink_data) {
        ISP_ERR("failed: port_data %p", port_sink_data);
        goto ERROR;
      }

      /* Fill internal peer */
      port_data->int_peer_port = sink_port;
      port_sink_data->int_peer_port = port;
    }
  }

  port_data->num_streams++;
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  ISP_HIGH("port %p dir %d parent %p %s", port, MCT_PORT_DIRECTION(port),
    (MCT_PORT_PARENT(port)->data),
    MCT_MODULE_NAME(MCT_PORT_PARENT(port)->data));
  ISP_HIGH("stream ide %x", identity);

  return TRUE;

ERROR:
  ISP_ERR("failed: port_fovcrop40_ext_link");
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  return FALSE;
}

/** port_fovcrop40_unlink:
 *
 *  @identity: identity of stream
 *
 *  @port: port handle
 *
 *  @peer: peer handle
 *
 *  Handle unlink func
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static void port_fovcrop40_unlink(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  boolean                 ret = TRUE;
  mct_port_t             *sink_port = NULL;
  port_fovcrop40_data_t *port_data = NULL;
  port_fovcrop40_data_t *port_sink_data = NULL;

  if (!port || !peer) {
    ISP_ERR("failed: port %p peer %p", port, peer);
    return;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(port));

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
    port_data->int_peer_port = NULL;
#if 0
    if (MCT_PORT_IS_SRC(port) == TRUE) {
      /* Find sink port with same identity */
      ret = fovcrop40_util_get_port_from_module(
        (mct_module_t *)(MCT_PORT_PARENT(port)->data), &sink_port,
        MCT_PORT_SINK, identity);
      if ((ret == FALSE) || !sink_port) {
        ISP_ERR("failed: fovcrop40_util_get_port_from_module ret %d port %p",
          ret, sink_port);
        goto ERROR;
      }

      if (port_data->int_peer_port &&
         (port_data->int_peer_port != sink_port)) {
        ISP_ERR("failed: source port linked with different sink port");
        goto ERROR;
      } else {
        port_sink_data = MCT_OBJECT_PRIVATE(sink_port);
        if (!port_sink_data) {
          ISP_ERR("failed: port_data %p", port_sink_data);
          goto ERROR;
        }

        /* Clear internal peer */
        port_data->int_peer_port = NULL;
        port_sink_data->int_peer_port = NULL;
      }
    }
#endif
  }

ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  ISP_HIGH("port %p dir %d parent %p %s", port, MCT_PORT_DIRECTION(port),
    (MCT_PORT_PARENT(port)->data),
    MCT_MODULE_NAME(MCT_PORT_PARENT(port)->data));
  ISP_HIGH("stream ide %x", identity);

  return;
}

/** port_fovcrop40_check_caps_reserve:
 *
 *  @port: port handle
 *
 *  @peer_caps: peer caps handle
 *
 *  @stream_info: handle to stream_info
 *
 *  Handle caps reserve
 *
 *  Return TRUE if this port is reserved, FALSE otherwise
 **/
static boolean port_fovcrop40_check_caps_reserve(mct_port_t *port,
  void *peer_caps, void *stream_info)
{
  boolean                 ret = FALSE;
  mct_stream_info_t      *mct_stream_info = (mct_stream_info_t *)stream_info;
  port_fovcrop40_data_t *port_data = NULL;
  uint32_t                session_id = 0;
  mct_module_t           *module = NULL;
  fovcrop40_t            *fovcrop = NULL;
  uint32_t                i = 0;

  if (!port || !stream_info) {
    ISP_ERR("failed: port %p stream_info %p", port, stream_info);
    return FALSE;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(port));

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

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    goto ERROR;
  }

  fovcrop = (fovcrop40_t *)MCT_OBJECT_PRIVATE(module);
  if (!fovcrop) {
    ISP_ERR("failed: fovcrop %p", fovcrop);
    goto ERROR;
  }
  ret = fovcrop40_init(fovcrop);
  if (ret == FALSE) {
    ISP_ERR("failed: fovcrop40_init");
    goto ERROR;
  }

  fovcrop->session_id = session_id;
  fovcrop->state = ISP_STATE_IDLE;
  fovcrop->update_enable_bit = TRUE;

  port_data->is_reserved = TRUE;
  port_data->session_id = session_id;

  if (mct_stream_info->stream_type == CAM_STREAM_TYPE_PARM) {
    fovcrop->session_based_identity = mct_stream_info->identity;
  }

  if (port->direction == MCT_PORT_SINK) {
    /* Store identity */
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (!fovcrop->identity[i]) {
        fovcrop->identity[i] = mct_stream_info->identity;
        break;
      }
    }
  }

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  ISP_HIGH("port %p dir %d parent %p %s", port, MCT_PORT_DIRECTION(port),
    (MCT_PORT_PARENT(port)->data),
    MCT_MODULE_NAME(MCT_PORT_PARENT(port)->data));
  ISP_HIGH("stream ide %x", mct_stream_info->identity);

  return TRUE;
ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  return FALSE;
}

/** port_fovcrop40_check_caps_unreserve:
 *
 *  @port: port handle
 *
 *  @identity: identity of stream
 *
 *  Handle caps unreserve
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_fovcrop40_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  port_fovcrop40_data_t   *port_data = NULL;
  mct_module_t            *module = NULL;
  fovcrop40_t             *fovcrop = NULL;
  uint32_t                 i = 0;

  if (!port) {
    ISP_ERR("failed: port %p", port);
    return FALSE;
  }

  /* Unlock this mutex in every return path */
  PTHREAD_MUTEX_LOCK(MCT_OBJECT_GET_LOCK(port));
  module = (mct_module_t *)MCT_PORT_PARENT(port)->data;
  if (!module) {
    ISP_ERR("failed null poiunter %p", module);
    goto ERROR;
  }

  port_data = MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    goto ERROR;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    goto ERROR;
  }

  fovcrop = (fovcrop40_t *)MCT_OBJECT_PRIVATE(module);
  if (!fovcrop) {
    ISP_ERR("failed: fovcrop %p", fovcrop);
    goto ERROR;
  }

  if (!port_data->num_streams) {
    port_data->session_id = 0;
    MCT_PORT_PEER(port) = NULL;
    if (MCT_OBJECT_PRIVATE(module))
      fovcrop40_reset((fovcrop40_t *)MCT_OBJECT_PRIVATE(module));
    port_data->is_reserved = FALSE;
    fovcrop->session_based_identity = 0x0;
  }

  if (port->direction == MCT_PORT_SRC) {
    /* Remove identity */
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (fovcrop->identity[i] == identity) {
        fovcrop->identity[i] = 0x0;
        break;
      }
    }
  }

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  ISP_HIGH("port %p dir %d parent %p %s", port, MCT_PORT_DIRECTION(port),
    (MCT_PORT_PARENT(port)->data),
    MCT_MODULE_NAME(MCT_PORT_PARENT(port)->data));
  ISP_HIGH("stream ide %x", identity);

  return TRUE;
ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  return FALSE;
}

/** port_fovcrop40_delete_port:
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
static boolean port_fovcrop40_delete_port(void *data, void *user_data)
{
  boolean          ret = TRUE;
  mct_module_t    *module = (mct_module_t *)user_data;
  mct_port_t      *port = (mct_port_t *)data;
  port_fovcrop40_data_t *port_fovcrop40_data = NULL;

  if (!data || !user_data) {
    ISP_ERR("failed: data %p user_data %p", data, user_data);
    return TRUE;
  }

  port_fovcrop40_data = MCT_OBJECT_PRIVATE(port);
  if (port_fovcrop40_data) {
   free(port_fovcrop40_data);
  }

  ret = mct_module_remove_port(module, port);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_module_remove_port");
  }

  mct_port_destroy(port);

  return TRUE;
}

/** port_fovcrop40_delete_ports:
 *
 *  @port: port handle
 *
 *  Delete port and its private data
 *
 *  Returns: void
 **/
void port_fovcrop40_delete_ports(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  mct_list_traverse(module->sinkports, port_fovcrop40_delete_port, module);
  mct_list_traverse(module->srcports, port_fovcrop40_delete_port, module);
}

/** port_fovcrop40_overwrite_funcs
 *
 *   @port: mct port instance
 *
 *   @private_data: port private
 *
 *  Assign mct port function pointers to respective fovcrop40 port
 *  functions
 *
 *  Returns nothing
 **/
static void port_fovcrop40_overwrite_funcs(mct_port_t *port,
  void *private_data)
{
  if (!port) {
    ISP_ERR("failed: port %p", port);
    return;
  }

  mct_port_set_event_func(port, port_fovcrop40_event);
  mct_port_set_ext_link_func(port, port_fovcrop40_ext_link);
  mct_port_set_unlink_func(port, port_fovcrop40_unlink);
  mct_port_set_check_caps_reserve_func(port,
    port_fovcrop40_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port,
    port_fovcrop40_check_caps_unreserve);
  MCT_OBJECT_PRIVATE(port) = private_data;
}

/** port_fovcrop40_create_port
 *
 *   @fovcrop40: fovcrop40 instance
 *
 *   @direction: direction of port
 *
 *  Creates port and add it to parent
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean port_fovcrop40_create_port(mct_module_t *module,
  mct_port_direction_t direction, mct_port_caps_type_t caps_type)
{
  boolean          ret = TRUE;
  port_fovcrop40_data_t *port_fovcrop40_data = NULL;
  char             port_name[32];
  mct_port_t      *mct_port = NULL;

  if (!module ||
    ((direction != MCT_PORT_SINK) && (direction != MCT_PORT_SRC)) ||
    ((caps_type != MCT_PORT_CAPS_STATS) &&
     (caps_type != MCT_PORT_CAPS_FRAME))) {
    ISP_ERR("failed: module %p direction %d caps_type %d", module, direction,
      caps_type);
    return FALSE;
  }

  port_fovcrop40_data = CAM_MALLOC(sizeof(port_fovcrop40_data_t));
  if (!port_fovcrop40_data) {
    ISP_ERR("failed: CAM_MALLOC");
    return FALSE;
  }
  memset(port_fovcrop40_data, 0, sizeof(*port_fovcrop40_data));

  if (direction == MCT_PORT_SINK) {
    snprintf(port_name, sizeof(port_name), "fovcrop40_sink");
  } else if (direction == MCT_PORT_SRC) {
    snprintf(port_name, sizeof(port_name), "fovcrop40_src");
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

  mct_port->caps.port_caps_type = caps_type;
  port_fovcrop40_overwrite_funcs(mct_port, (void *)port_fovcrop40_data);

  return ret;
ERROR:
  free(port_fovcrop40_data);
  return ret;
}

/** port_fovcrop40_create:
 *
 *  @module: module handle
 *
 *  1) Create one sink port per ISP
 *
 *  2) Create one source port for STATS
 *
 *  3) Create one source port for FRAME
 **/
boolean port_fovcrop40_create(mct_module_t *module)
{
  boolean  ret = TRUE;

  if (!module) {
    ISP_ERR("failed: invalid params %p", module);
    return FALSE;
  }

  /* Create sink port */
  ret = port_fovcrop40_create_port(module, MCT_PORT_SINK, MCT_PORT_CAPS_FRAME);
  if (ret == FALSE) {
    ISP_ERR("failed: port_fovcrop40_create_port");
    goto ERROR;
  }

  /* Create source port */
  ret = port_fovcrop40_create_port(module, MCT_PORT_SRC, MCT_PORT_CAPS_FRAME);
  if (ret == FALSE) {
    ISP_ERR("failed: port_fovcrop40_create_port");
    goto ERROR;
  }

  return ret;

ERROR:
  ISP_ERR("failed: port_fovcrop40_create");
  port_fovcrop40_delete_ports(module);
  return ret;
}
