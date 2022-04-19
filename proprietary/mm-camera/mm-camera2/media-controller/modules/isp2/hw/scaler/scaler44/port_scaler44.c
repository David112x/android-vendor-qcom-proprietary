/* port_scaler44.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <stdio.h>
#include <pthread.h>

/* mctl headers */
#include "media_controller.h"
#include "mct_stream.h"

/* ISP headers*/
#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "isp_defs.h"
#include "module_scaler44.h"
#include "port_scaler44.h"
#include "scaler44.h"
#include "scaler44_util.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_SCALER, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_SCALER, fmt, ##args)

/* Width need to aligned to 16 incase of ISP 44 */
#define FLOOR_ISP44 FLOOR16

/**  port_scaler44_handle_set_stream_config:
 *
 *  @data1: mct stream info handle
 *  @data2: streaming mode mask
 *
 *  Append streaming mode of stream info in streaming mode
 *  mask
 *
 *  Return TRUE
 **/
static boolean port_scaler44_handle_set_stream_config(scaler44_t *scaler,
  mct_event_t *event)
{
  boolean ret = TRUE;
  sensor_out_info_t   *sensor_out_info = NULL;
  float aspect_ratio1,aspect_ratio2;
  int32_t temp_w, temp_h, camif_w, camif_h;
  ISP_HIGH("E: scaler: %p event: %p", scaler, event);
  if (!scaler || !event) {
    ISP_ERR("failed: %p %p", scaler, event);
    return FALSE;
  }

  sensor_out_info = event->u.module_event.module_event_data;
  if (!sensor_out_info) {
    ISP_ERR("failed: sensor_out_info %p", sensor_out_info);
    return FALSE;
  }

  scaler->sensor_out_info = *sensor_out_info;
  scaler->hw_stream_info.streaming_mode= scaler->hw_stream_info.streaming_mode;

  camif_w = scaler->sensor_out_info.request_crop.last_pixel -
    scaler->sensor_out_info.request_crop.first_pixel + 1;
  camif_h = scaler->sensor_out_info.request_crop.last_line -
    scaler->sensor_out_info.request_crop.first_line + 1;
  if (!camif_w || !camif_h) {
    ISP_ERR("failed: camif_w %d camif_h %d", camif_w, camif_h);
    return FALSE;
  }

  if (camif_w < (uint32_t)scaler->hw_stream_info.width ||
    camif_h < (uint32_t)scaler->hw_stream_info.height) {
    if (scaler->dis_enable) {
      ISP_ERR("sensor output width is smaller than"
              " request size and DIS needs extra margin");
      return FALSE;
    }

    temp_w = MIN(camif_w, (uint32_t)scaler->hw_stream_info.width);
    temp_h = MIN(camif_h, (uint32_t)scaler->hw_stream_info.height);

    aspect_ratio1 = (float)scaler->hw_stream_info.width /
        scaler->hw_stream_info.height;
    aspect_ratio2 = (float)temp_w / temp_h;

    if (fabs(aspect_ratio1 - aspect_ratio2) > ASPECT_TOLERANCE) {
      if (aspect_ratio1 < aspect_ratio2) {
        /* Modify the width */
        scaler->hw_stream_info.width = temp_h * aspect_ratio1;
        scaler->hw_stream_info.width = FLOOR_ISP44(scaler->hw_stream_info.width);
        scaler->hw_stream_info.height = scaler->hw_stream_info.width / aspect_ratio1;
      } else if (aspect_ratio1 > aspect_ratio2) {
        scaler->hw_stream_info.width = FLOOR_ISP44(temp_w);
        scaler->hw_stream_info.height = scaler->hw_stream_info.width / aspect_ratio1;
      }
    } else {
      scaler->hw_stream_info.width = FLOOR_ISP44(temp_w);
      scaler->hw_stream_info.height = temp_h;
    }
  }

  /* Update the sensor type and CAMIF dimension */
  scaler->is_bayer_sensor = isp_sub_module_is_bayer_mono_fmt(
    scaler->sensor_out_info.fmt);

  if (scaler->is_bayer_sensor) {
    scaler->camif_dim.width = sensor_out_info->request_crop.last_pixel -
      sensor_out_info->request_crop.first_pixel + 1;
  } else {
    scaler->camif_dim.width = (sensor_out_info->request_crop.last_pixel -
      sensor_out_info->request_crop.first_pixel + 1) >> 1;
  }

  scaler->camif_dim.height = sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1;

  /* Update IFACE if ISP is capable of doing further downscaling for CDS */
  if (((scaler->hw_stream_info.width/2 * ISP_SCALER44_MAX_SCALER_FACTOR) <
    scaler->camif_dim.width) ||
    ((scaler->hw_stream_info.height/2 * ISP_SCALER44_MAX_SCALER_FACTOR) <
    scaler->camif_dim.height)) {
      scaler->cds_cap[scaler->entry_idx] = FALSE;
      ISP_ERR("path %d, CDS disable\n", scaler->entry_idx);
    } else
      scaler->cds_cap[scaler->entry_idx] = TRUE;

  ISP_DBG("path %d, CDS %d\n", scaler->entry_idx, scaler->cds_cap[scaler->entry_idx]);

  /* Calculate max scale ratio for single vfe only. For dual vfe,
   * max scale ratio is calculated when SET_SPLIT_INFO is received
   */
  if (scaler->isp_out_info.is_split == FALSE) {
    /* Check if there is any scope for downscaling. Calculate max scale ratio
     * only if there is possibility for downscaling
     */
    if (camif_w > scaler->hw_limits.max_width) {
      scaler->max_scale_factor = (float)scaler->hw_limits.max_width /
        (float)camif_w;
    } else {
      scaler->max_scale_factor = 1.0f;
    }
  }
  if (scaler->hw_stream_info.need_uv_subsample != 1) {
    scaler->hw_stream_info.need_uv_subsample = 0;
  }
  ret = scaler44_config(scaler);
  if (ret == FALSE) {
    ISP_ERR("failed: ret == FALSE");
    return FALSE;
  }

  ISP_DBG("X: ret = TRUE");
  return ret;
}

/**  port_scaler44_handle_cds_request:
 *
 *  @data1: mct stream info handle
 *  @data2: streaming mode mask
 *
 *  Append streaming mode of stream info in streaming mode
 *  mask
 *
 *  Return TRUE
 **/
boolean port_scaler44_handle_cds_request(scaler44_t *scaler,
  mct_event_t *event)
{
  boolean ret = TRUE;
  isp_cds_request_t   *isp_cds_request = NULL;
  uint32_t             sensor_output_width = 0;
  uint32_t             sensor_output_height = 0;
  float                scale_factor_w = 0.0;
  float                scale_factor_h = 0.0;
  float                max_scale_factor = 0.0;

  ISP_HIGH("E: scaler: %p event: %p", scaler, event);
  if (!scaler || !event) {
    ISP_ERR("failed: %p %p", scaler, event);
    return FALSE;
  }

  isp_cds_request = event->u.module_event.module_event_data;
  if (!isp_cds_request) {
    ISP_ERR("failed: isp_cds_request %p", isp_cds_request);
    return FALSE;
  }

  sensor_output_width = scaler->sensor_out_info.request_crop.last_pixel -
    scaler->sensor_out_info.request_crop.first_pixel + 1;
  sensor_output_height = scaler->sensor_out_info.request_crop.last_line -
    scaler->sensor_out_info.request_crop.first_line + 1;

  /*do w_scaler times more downscaling, so the scale factor times it*/
  scale_factor_w = isp_cds_request->cds_request_info.w_scale *
    ((float)sensor_output_width / (float)scaler->hw_stream_info.width);
  scale_factor_h = isp_cds_request->cds_request_info.v_scale *
    (float)sensor_output_height / (float)scaler->hw_stream_info.height;
  max_scale_factor =
    (scale_factor_w > scale_factor_h) ? scale_factor_w : scale_factor_h;

  if (max_scale_factor > ISP_SCALER44_MAX_SCALER_FACTOR) {
    ISP_ERR("CDS request go over scaler hw limit, give up!! ret = FALSE");
    return FALSE;
  }

  scaler->hw_stream_info.need_uv_subsample =
    isp_cds_request->cds_request_info.need_cds_subsample;

  ret = scaler44_config(scaler);
  if (ret == FALSE) {
    ISP_ERR("failed: ret == FALSE");
    return FALSE;
  }

  /* only set updating flag when config correctly*/
  scaler->is_cds_update = isp_cds_request->cds_request;

  ISP_DBG("X: ret = TRUE");
  return ret;
}

/** port_scaler44_update_streaming_mode:
 *
 *  @data1: mct stream info handle
 *  @data2: streaming mode mask
 *
 *  Append streaming mode of stream info in streaming mode
 *  mask
 *
 *  Return TRUE
 **/
static boolean port_scaler44_update_streaming_mode(void *data1, void *data2)
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

/** port_scaler44_handle_streamon:
 *
 *  @scaler: scaler handle
 *  @event: event to be handled
 *
 *  Handle STREAM ON event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_scaler44_handle_streamon(mct_module_t *module,
  scaler44_t *scaler, mct_event_t *event)
{
  boolean ret = TRUE;

  if (!module || !scaler || !event) {
    ISP_ERR("failed: module %p scaler %p event %p", module, scaler, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&scaler->mutex);
  /* Update streaming mode mask in module private */
  mct_list_traverse(scaler->l_stream_info,
    port_scaler44_update_streaming_mode, &scaler->streaming_mode_mask);

  ret = scaler44_config(scaler);
  if (ret == FALSE) {
    ISP_ERR("failed: scaler44_config");
  }

  scaler->stream_cnt++;

  PTHREAD_MUTEX_UNLOCK(&scaler->mutex);

  ret = module_scaler44_pass_scale_factor(module, scaler, TRUE, event->identity);

  return ret;
}

/** port_scaler44_handle_streamoff:
 *
 *  @scaler: scaler handle
 *  @event: event to be handled
 *  Handle STREAM OFF event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_scaler44_handle_streamoff(mct_module_t *module,
  scaler44_t *scaler, mct_event_t *event)
{
  boolean       ret = TRUE;

  if (!module || !scaler || !event) {
    ISP_ERR("failed: module %p scaler %p event %p", module, scaler, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&scaler->mutex);
  if (scaler->dis_enable) {
    scaler->dis_enable = FALSE;
  }

  /* Reset streaming mode */
  ISP_RESET_STREAMING_MODE_MASK(scaler->streaming_mode_mask);

  if (scaler->stream_cnt > 0) {
    scaler->stream_cnt--;
  }

  PTHREAD_MUTEX_UNLOCK(&scaler->mutex);

  ret = module_scaler44_pass_scale_factor(module, scaler, FALSE, event->identity);

  return ret;
}

/**  port_scaler44_handle_hal_set_parm:
 *
 *  @scaler: scaler module private
 *  @event: mct event
 *
 *  Append streaming mode of stream info in streaming mode
 *  mask
 *
 *  Return TRUE
 **/
boolean port_scaler44_handle_hal_set_parm(mct_module_t *module,
  scaler44_t *scaler, mct_event_control_parm_t *hal_param, uint32_t identity)
{
  boolean ret = TRUE;
  int i = 0;

  if (!module || !scaler || !hal_param) {
    ISP_ERR("failed: %p %p %p", module, scaler, hal_param);
    return FALSE;
  }

  switch (hal_param->type) {
  case CAM_INTF_PARM_DIS_ENABLE: {
   for (i = 0; i < ISP_MAX_STREAMS; i++) {
       if (scaler->stream_is_type[i] > IS_TYPE_NONE &&
           scaler->stream_is_type[i] < IS_TYPE_MAX ) {
        scaler->dis_enable = *(int32_t *)hal_param->parm_data;
        scaler44_calculate_dis(scaler);
        ISP_DBG("hw_stream %d, dis_enable %d",
          scaler->entry_idx, scaler->dis_enable);
      }
    }
  }
    break;

  case CAM_INTF_PARM_USERZOOM: {
    ret = scaler44_parm_zoom(module, scaler, (int32_t *)hal_param->parm_data,
      identity);
    if (ret == FALSE) {
      ISP_ERR("failed: scaler44_parm_zoom");
    }
  }
    break;

  case CAM_INTF_META_SCALER_CROP_REGION: {
    ret = scaler44_parm_crop_region(module, scaler,
      (cam_crop_region_t *)hal_param->parm_data, identity);
    if (ret == FALSE) {
      ISP_ERR("failed: scaler44_parm_crop_region");
    }
  }
  break;
  case CAM_INTF_PARM_BOKEH_MODE: {
    scaler->rtb_mode = *(uint8_t *)hal_param->parm_data;
  }
  break;

  default:
    break;
  }

  ISP_DBG("X: ret = TRUE");
  return ret;
}

static boolean port_scaler44_handle_set_super_parm(mct_module_t* module,
  scaler44_t *scaler, mct_event_t *event)
{
  boolean                         ret = TRUE,
                                  func_ret = TRUE;
  uint32_t                        i = 0;
  mct_event_super_control_parm_t *super_param = NULL;
  uint32_t                        identity = 0;

  if (!module || !scaler || !event) {
    ISP_ERR("failed: module %p scaler %p event %p", module, scaler, event);
    return FALSE;
  }

  super_param =
    (mct_event_super_control_parm_t *)event->u.ctrl_event.control_event_data;
  if (!super_param) {
    ISP_ERR("failed: super_param %p", super_param);
    return FALSE;
  }

  identity = event->identity;

  for (i = 0; i < super_param->num_of_parm_events; i++) {
    ret = port_scaler44_handle_hal_set_parm(module, scaler,
      &super_param->parm_events[i], identity);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler44_handle_hal_set_parm type %d",
        super_param->parm_events[i].type);
      func_ret = FALSE;
    }
  }
  return func_ret;
}

/** port_scaler44_forward_event:
 *
 *  @port: port handle
 *  @event: event to be forwarded
 *
 *  Forward event based on port direction and event direction
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_scaler44_send_scaling_factor_update(
  mct_module_t *module, mct_port_t *port,
  uint32_t identity, scaler44_t *scaler)
{
  boolean                 ret = TRUE;
  port_scaler44_data_t   *port_data = NULL;
  mct_event_t             mct_event;
  isp_private_event_t     private_event;

  if (!module || !port || !scaler) {
    ISP_ERR("failed: module %p port %p scaler %p", module, port, scaler);
    return FALSE;
  }

  /*Get  fovcrop's output info */
  ISP_DBG("scaling_factor_update %f",
     scaler->scalers.scaling_factor);
  memset(&private_event, 0, sizeof(isp_private_event_t));
  private_event.type = ISP_PRIVATE_SCALER_OUTPUT_UPDATE;
  private_event.data = (void *)&scaler->scalers.scaling_factor;
  private_event.data_size = sizeof(scaler->scalers.scaling_factor);

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.identity = identity;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.direction = MCT_EVENT_UPSTREAM;
  mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  mct_event.u.module_event.module_event_data = (void *)&private_event;

  ret = scaler44_util_send_event(module, &mct_event);
  if (ret == FALSE) {
    ISP_ERR("failed: scaler44_util_send_event");
  }
  ISP_DBG("X");
  return ret;
}

/** port_scaler44_forward_event:
 *
 *  @port: port handle
 *  @event: event to be forwarded
 *
 *  Forward event based on port direction and event direction
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_scaler44_forward_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean                 ret = TRUE;
  port_scaler44_data_t *port_data = NULL;

  if (!port || !event) {
    ISP_ERR("failed: port %p event %p", port, event);
    return FALSE;
  }

  if (((event->direction == MCT_EVENT_UPSTREAM) &&
       (MCT_PORT_DIRECTION(port) != MCT_PORT_SRC)) ||
      ((event->direction == MCT_EVENT_DOWNSTREAM) &&
       (MCT_PORT_DIRECTION(port) != MCT_PORT_SINK))) {
    ISP_ERR("failed: invalid types, event dir %d port dir %d",
      event->direction, MCT_PORT_DIRECTION(port));
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

/** port_scaler44_process_downstream_ctrl_event:
 *
 *  @module: module handle
 *  @port: port handle
 *  @scaler: scaler handle
 *  @event: event to be processed
 *
 *  Handle downstream ctrl event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_scaler44_process_downstream_ctrl_event(
  mct_module_t *module, mct_port_t *port, scaler44_t *scaler,
  mct_event_t *event)
{
  boolean              ret = TRUE;
  mct_event_control_t *ctrl_event = NULL;

  if (!module || !port || !scaler || !event) {
    ISP_ERR("failed: %p %p %p %p", module, port, scaler, event);
    return FALSE;
  }

  /* Extract ctrl event */
  ctrl_event = &event->u.ctrl_event;

  ISP_DBG("ide %x event %d mod %s", event->identity,
     ctrl_event->type, MCT_MODULE_NAME(module));
  switch (ctrl_event->type) {
  case MCT_EVENT_CONTROL_SET_PARM:
    ISP_DBG("set parm  ide %x mod %s", event->identity,
      MCT_MODULE_NAME(module));
    ret = port_scaler44_handle_hal_set_parm(module, scaler,
      event->u.ctrl_event.control_event_data, event->identity);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler44_handle_hal_set_parm");
    }
    if (scaler->scaling_factor_update) {
      scaler->scaling_factor_update = FALSE;
      port_scaler44_send_scaling_factor_update(module, port, event->identity,
        scaler);
    }
     /* per stats module request, for this event,
     * ISP needs to change the direction and
     * redirect it to downstream */
    /* Forward event to FRAME port */
    ret = port_scaler44_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;

  case MCT_EVENT_CONTROL_SET_SUPER_PARM:
   ISP_HIGH("set super parm  ide %x mod %s", event->identity,
      MCT_MODULE_NAME(module));
    ret = port_scaler44_handle_set_super_parm(module, scaler, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler44_handle_set_super_parm");
    }
    if (scaler->scaling_factor_update) {
      scaler->scaling_factor_update = FALSE;
      port_scaler44_send_scaling_factor_update(module, port, event->identity,
        scaler);
    }
    ret = port_scaler44_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;

  case MCT_EVENT_CONTROL_STREAMON:
   ISP_HIGH("stream on ide %x mod %s", event->identity,
      MCT_MODULE_NAME(module));
    ret = port_scaler44_handle_streamon(module, scaler, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler44_handle_streamon");
    }

    /* Forward event to FRAME port */
    ret = port_scaler44_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;

  case MCT_EVENT_CONTROL_STREAMOFF:
    ISP_HIGH("stream off ide %x mod %s", event->identity,
      MCT_MODULE_NAME(module));
    ret = port_scaler44_handle_streamoff(module, scaler, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler44_handle_streamoff");
    }

    /* Forward event to FRAME port */
    ret = port_scaler44_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;

  default:
    /* Forward event to FRAME port */
    ret = port_scaler44_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        ctrl_event->type);
    }
    break;
  }
  return ret;
}

/** port_scaler44_process_downstream_module_event:
 *
 *  @module: module handle
 *  @port: port handle
 *  @scaler: scaler handle
 *  @event: event to be processed
 *
 *  Handle downstream ctrl event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_scaler44_process_downstream_module_event(
  mct_module_t *module, mct_port_t *port, scaler44_t *scaler,
  mct_event_t *event)
{
  boolean              ret = TRUE;
  mct_event_module_t  *module_event = NULL;

  if (!module || !port || !scaler || !event) {
    ISP_ERR("failed: %p %p %p %p", module, port, scaler, event);
    return FALSE;
  }

  /* Extract ctrl event */
  module_event = &event->u.module_event;

  ISP_DBG("ide %x event %d mod %s", event->identity,
     module_event->type, MCT_MODULE_NAME(module));
  switch (module_event->type) {
  case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
    ret = port_scaler44_handle_set_stream_config(scaler, event);
    if (ret == FALSE) {
      ISP_ERR("failed: scaler44_handle_set_stream_config");
    }
    /* Forward event */
    ret = port_scaler44_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
  }
    break;

  case MCT_EVENT_MODULE_ISP_CDS_REQUEST: {
    ret = port_scaler44_handle_cds_request(scaler, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler44_handle_cds_request");
    } else {
      /* Forward event  only when validate/config is good*/
      ret = port_scaler44_forward_event(port, event);
      if (ret == FALSE) {
        ISP_ERR("failed: mct_port_send_event_to_peer event %d",
          module_event->type);
      }
    }
  }
    break;
  case MCT_EVENT_MODULE_ISP_PRIVATE_EVENT:
    ret = scaler44_handle_isp_private_event(module, scaler,
      module_event->module_event_data, event->identity);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_ISP_PRIVATE_EVENT");
    }
    /* Forward event */
    ret = port_scaler44_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;

  default:
    /* Forward event */
    ret = port_scaler44_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;
  }
  return ret;
}

/** port_scaler44_process_downstream_event:
 *
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Handle downstream event
 *
 *  Returns TRUE on success and FALSE on failure
 **/
boolean port_scaler44_process_downstream_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean       ret = TRUE;
  mct_module_t *module = NULL;
  scaler44_t *scaler = NULL;

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
  scaler = (scaler44_t *)MCT_OBJECT_PRIVATE(module);
  if (!scaler) {
    ISP_ERR("failed: scaler %p", scaler);
    return FALSE;
  }

  if (event->type == MCT_EVENT_CONTROL_CMD) {
    ret = port_scaler44_process_downstream_ctrl_event(module, port, scaler,
      event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler44_process_downstream_ctrl_event");
    }
  } else if (event->type == MCT_EVENT_MODULE_EVENT) {
    ret = port_scaler44_process_downstream_module_event(module, port,
      scaler, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler44_process_downstream_module_event");
    }
  }
  return ret;
}

/** port_scaler44_process_upstream_event:
 *
 *  @port: port on which this event arrived
 *  @event: event to be handled
 *
 *  Handle downstream event
 *
 *  Returns TRUE on success and FALSE on failure
 **/
boolean port_scaler44_process_upstream_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean              ret = TRUE;
  mct_module_t        *module = NULL;
  scaler44_t        *scaler = NULL;
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
  scaler = MCT_OBJECT_PRIVATE(module);
  if (!scaler) {
    ISP_ERR("failed: scaler %p", scaler);
    return FALSE;
  }

  module_event = &event->u.module_event;
  ISP_DBG("ide %x event %d mod %s", event->identity,
     module_event->type, MCT_MODULE_NAME(module));

  switch (module_event->type) {
  case MCT_EVENT_MODULE_ISP_PRIVATE_EVENT: {
    ret = scaler44_handle_isp_private_event(module, scaler,
      module_event->module_event_data, event->identity);
    if (ret == FALSE) {
      ISP_ERR("failed: MCT_EVENT_MODULE_ISP_PRIVATE_EVENT");
    }
  }
    break;

  default:
    /* Forward event to upstream FRAME port */
    ret = port_scaler44_forward_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_port_send_event_to_peer event %d",
        module_event->type);
    }
    break;
  }

  return ret;
}

/** port_scaler44_event:
 *
 *  @port: port handle
 *  @event: event to be processed
 *
 *  Handle ISP event based on direction
 *
 *  Returns TRUE on success and FALSE on failure */
static boolean port_scaler44_event(mct_port_t *port, mct_event_t *event)
{
  boolean       ret = TRUE;
  mct_module_t *module = NULL;
  scaler44_t *scaler = NULL;

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

  scaler = MCT_OBJECT_PRIVATE(module);
  if (!scaler) {
    ISP_ERR("failed: scaler %p", scaler);
    return FALSE;
  }

  if (event->direction == MCT_EVENT_DOWNSTREAM) {
    ret = port_scaler44_process_downstream_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler44_process_downstream_event");
    }
  } else {
    ret = port_scaler44_process_upstream_event(port, event);
    if (ret == FALSE) {
      ISP_ERR("failed: port_scaler44_process_upstream_event");
    }
  }

  return ret;
}

/** port_scaler44_ext_link:
 *
 *  @identity: identity of stream
 *  @port: port handle
 *  @peer: peer handle
 *
 *  Handle ext link
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean port_scaler44_ext_link(unsigned int identity, mct_port_t* port,
  mct_port_t *peer)
{
  boolean                 ret = TRUE;
  mct_module_t           *module = NULL;
  mct_port_t             *sink_port = NULL;
  port_scaler44_data_t *port_data = NULL;
  port_scaler44_data_t *port_sink_data = NULL;

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
    ret = scaler44_util_get_port_from_module(
      (mct_module_t *)(MCT_PORT_PARENT(port)->data), &sink_port, MCT_PORT_SINK,
      identity);
    if ((ret == FALSE) || !sink_port) {
      ISP_ERR("failed: scaler44_util_get_port_from_module ret %d port %p",
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
  ISP_DBG("port %p dir %d parent %p %s", port, MCT_PORT_DIRECTION(port),
    (MCT_PORT_PARENT(port)->data),
    MCT_MODULE_NAME(MCT_PORT_PARENT(port)->data));
  ISP_DBG("ide %x", identity);

  return TRUE;

ERROR:
  ISP_ERR("failed: port_scaler44_ext_link");
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  return FALSE;
}

/** port_scaler44_unlink:
 *
 *  @identity: identity of stream
 *  @port: port handle
 *  @peer: peer handle
 *
 *  Handle unlink func
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static void port_scaler44_unlink(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  boolean                 ret = TRUE;
  mct_port_t             *sink_port = NULL;
  port_scaler44_data_t *port_data = NULL;
  port_scaler44_data_t *port_sink_data = NULL;

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
    ISP_HIGH("port %p dir %d parent %p %s", port, MCT_PORT_DIRECTION(port),
      (MCT_PORT_PARENT(port)->data),
      MCT_MODULE_NAME(MCT_PORT_PARENT(port)->data));
    goto ERROR;
  }


  if (!(--port_data->num_streams)) {
    MCT_PORT_PEER(port) = NULL;
    port_data->int_peer_port = NULL;
#if 0
    if (MCT_PORT_IS_SRC(port) == TRUE) {
      /* Find sink port with same identity */
      ret = scaler44_util_get_port_from_module(
        (mct_module_t *)(MCT_PORT_PARENT(port)->data), &sink_port,
        MCT_PORT_SINK, identity);
      if ((ret == FALSE) || !sink_port) {
        ISP_ERR("failed: scaler44_util_get_port_from_module ret %d port %p",
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
  ISP_HIGH("ide %x", identity);
  return;
}

/** port_scaler44_check_caps_reserve:
 *
 *  @port: port handle
 *  @peer_caps: peer caps handle
 *  @stream_info: handle to stream_info
 *
 *  Handle caps reserve
 *
 *  Return TRUE if this port is reserved, FALSE otherwise
 **/
static boolean port_scaler44_check_caps_reserve(mct_port_t *port,
  void *peer_caps, void *stream_info)
{
  boolean                 ret = FALSE;
  mct_stream_info_t      *mct_stream_info = (mct_stream_info_t *)stream_info;
  port_scaler44_data_t *port_data = NULL;
  uint32_t                session_id = 0;
  mct_module_t           *module = NULL;
  scaler44_t             *scaler = NULL;
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
    ISP_ERR("port %p assigned for different session %d", port,
      port_data->session_id);
    goto ERROR;
  }

  /* Extract module handle */
  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    goto ERROR;
  }

  scaler = (scaler44_t *)MCT_OBJECT_PRIVATE(module);
  if (!scaler) {
    ISP_ERR("failed: scaler %p", scaler);
    goto ERROR;
  }
  scaler->update_enable_bit = TRUE;

  ret = scaler44_util_append_stream_info(port, mct_stream_info);
  if (ret == FALSE) {
    ISP_ERR("failed: scaler44_util_append_stream_info");
    goto ERROR;
  }

  port_data->is_reserved = TRUE;
  port_data->session_id = session_id;

  if (mct_stream_info->stream_type == CAM_STREAM_TYPE_PARM) {
    scaler->session_based_identity = mct_stream_info->identity;
  }

  if (port->direction == MCT_PORT_SINK) {
    /* Store identity */
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (!scaler->identity[i]) {
        scaler->identity[i] = mct_stream_info->identity;
        scaler->stream_is_type[i] = mct_stream_info->is_type;
        break;
      }
    }
  }

  scaler->stream_type = mct_stream_info->stream_type;

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));

  ISP_DBG("port %p dir %d parent %p %s", port, MCT_PORT_DIRECTION(port),
    (MCT_PORT_PARENT(port)->data),
    MCT_MODULE_NAME(MCT_PORT_PARENT(port)->data));
  ISP_DBG("stream ide %x type %d", mct_stream_info->identity,
    mct_stream_info->stream_type);

  return TRUE;
ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  return FALSE;
}

/** port_scaler44_check_caps_unreserve:
 *
 *  @port: port handle
 *  @identity: identity of stream
 *
 *  Handle caps unreserve
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_scaler44_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  boolean                   ret = TRUE;
  port_scaler44_data_t   *port_data = NULL;
  mct_module_t           *module = NULL;
  scaler44_t             *scaler = NULL;
  uint32_t                i = 0;

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

  scaler = (scaler44_t *)MCT_OBJECT_PRIVATE(module);
  if (!scaler) {
    ISP_ERR("failed: scaler %p", scaler);
    goto ERROR;
  }

  ret = scaler44_util_remove_stream_info(port, identity);
  if (ret == FALSE) {
    ISP_ERR("failed: scaler44_util_append_stream_info");
  }

  if (!port_data->num_streams) {
    port_data->session_id = 0;
    MCT_PORT_PEER(port) = NULL;
    if (MCT_OBJECT_PRIVATE(module))
      scaler44_reset((scaler44_t *)MCT_OBJECT_PRIVATE(module));
    port_data->is_reserved = FALSE;
    scaler->session_based_identity = 0x0;
  }

  if (port->direction == MCT_PORT_SRC) {
    /* Remove identity */
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (scaler->identity[i] == identity) {
        scaler->identity[i] = 0x0;
        scaler->stream_is_type[i] = 0;
        break;
      }
    }
  }

  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  ISP_HIGH("stream ide %x", identity);

  return TRUE;
ERROR:
  PTHREAD_MUTEX_UNLOCK(MCT_OBJECT_GET_LOCK(port));
  return FALSE;
}

/** port_scaler44_delete_port:
 *
 *  @data: handle to port to be deleted
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
static boolean port_scaler44_delete_port(void *data, void *user_data)
{
  boolean          ret = TRUE;
  mct_module_t    *module = (mct_module_t *)user_data;
  mct_port_t      *port = (mct_port_t *)data;
  port_scaler44_data_t *port_scaler44_data = NULL;

  if (!data || !user_data) {
    ISP_ERR("failed: data %p user_data %p", data, user_data);
    return TRUE;
  }

  port_scaler44_data = MCT_OBJECT_PRIVATE(port);
  if (port_scaler44_data) {
   free(port_scaler44_data);
  }

  ret = mct_module_remove_port(module, port);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_module_remove_port");
  }

  mct_port_destroy(port);

  return TRUE;
}

/** port_scaler44_delete_ports:
 *
 *  @port: port handle
 *
 *  Delete port and its private data
 *
 *  Returns: void
 **/
void port_scaler44_delete_ports(mct_module_t *module)
{
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return;
  }

  mct_list_traverse(module->sinkports, port_scaler44_delete_port, module);
  mct_list_traverse(module->srcports, port_scaler44_delete_port, module);
}

/** port_scaler44_overwrite_funcs
 *
 *   @port: mct port instance
 *   @private_data: port private
 *
 *  Assign mct port function pointers to respective scaler port
 *  functions
 *
 *  Returns nothing
 **/
static void port_scaler44_overwrite_funcs(mct_port_t *port,
  void *private_data)
{
  if (!port) {
    ISP_ERR("failed: port %p", port);
    return;
  }

  mct_port_set_event_func(port, port_scaler44_event);
  mct_port_set_ext_link_func(port, port_scaler44_ext_link);
  mct_port_set_unlink_func(port, port_scaler44_unlink);
  mct_port_set_check_caps_reserve_func(port,
    port_scaler44_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port,
    port_scaler44_check_caps_unreserve);
  MCT_OBJECT_PRIVATE(port) = private_data;
}

/** port_scaler44_create_port
 *
 *   @scaler: scaler instance
 *   @direction: direction of port
 *
 *  Creates port and add it to parent
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean port_scaler44_create_port(mct_module_t *module,
  mct_port_direction_t direction, mct_port_caps_type_t caps_type)
{
  boolean          ret = TRUE;
  port_scaler44_data_t *port_scaler44_data = NULL;
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

  port_scaler44_data =
    (port_scaler44_data_t *)CAM_MALLOC(sizeof(port_scaler44_data_t));
  if (!port_scaler44_data) {
    ISP_ERR("failed: CAM_MALLOC");
    return FALSE;
  }
  memset(port_scaler44_data, 0, sizeof(*port_scaler44_data));

  if (direction == MCT_PORT_SINK) {
    snprintf(port_name, sizeof(port_name), "scaler44_sink");
  } else if (direction == MCT_PORT_SRC) {
    snprintf(port_name, sizeof(port_name), "scaler44_src");
  }

  mct_port = mct_port_create(port_name);
  if (!mct_port) {
    ISP_ERR("failed: mct_port_create");
    ret = FALSE;
    goto ERROR;
  }

  MCT_PORT_DIRECTION(mct_port) = direction;
  ret = mct_module_add_port(module, mct_port);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_module_add_port");
    goto ERROR;
  }

  ISP_HIGH("add port %p dir %d mod %p parent %p", mct_port, direction, module,
    (MCT_PORT_PARENT(mct_port)->data));
  mct_port->caps.port_caps_type = caps_type;
  port_scaler44_overwrite_funcs(mct_port, (void *)port_scaler44_data);

  return ret;
ERROR:
  free(port_scaler44_data);
  return ret;
}

/** port_scaler44_create:
 *
 *  @module: module handle
 *
 *  1) Create one sink port per ISP
 *
 *  2) Create one source port for STATS
 *
 *  3) Create one source port for FRAME
 **/
boolean port_scaler44_create(mct_module_t *module)
{
  boolean  ret = TRUE;

  if (!module) {
    ISP_ERR("failed: invalid params %p", module);
    return FALSE;
  }

  /* Create sink port */
  ret = port_scaler44_create_port(module, MCT_PORT_SINK, MCT_PORT_CAPS_FRAME);
  if (ret == FALSE) {
    ISP_ERR("failed: port_scaler44_create_port");
    goto ERROR;
  }

  /* Create source port */
  ret = port_scaler44_create_port(module, MCT_PORT_SRC, MCT_PORT_CAPS_FRAME);
  if (ret == FALSE) {
    ISP_ERR("failed: port_scaler44_create_port");
    goto ERROR;
  }

  return ret;

ERROR:
  ISP_ERR("failed: port_scaler44_create");
  port_scaler44_delete_ports(module);
  return ret;
}
