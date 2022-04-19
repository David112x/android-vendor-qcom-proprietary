/* scaler44_util.c
 *
 * Copyright (c) 2012-2018 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* mctl headers */
#include "mct_module.h"

/* isp headers */
#include "module_scaler44.h"
#include "scaler44_util.h"
#include "scaler44.h"
#include "isp_sub_module_log.h"

#if defined (_ANDROID_)
#include <cutils/properties.h>
#endif

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_SCALER, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_SCALER, fmt, ##args)

/** scaler44_util_append_cfg:
 *
 *  @hw_update_list: hw update list
 *  @hw_update: hw update cmd to be enqueued
 *
 *  Enqueue hw_update in hw_update_list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean scaler44_util_append_cfg(struct msm_vfe_cfg_cmd_list **hw_update_list,
  struct msm_vfe_cfg_cmd_list *hw_update)
{
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd_list *list = NULL;

  if (!hw_update_list || !hw_update) {
    ISP_ERR("failed: %p %p", hw_update_list, hw_update);
    return FALSE;
  }

  list = *hw_update_list;
  if (!list) {
    *hw_update_list = hw_update;
  } else {
    while (list->next) {
      list = list->next;
    }
    list->next = hw_update;
    list->next_size = sizeof(*hw_update);
  }
  return ret;
}

/** scaler44_util_compare_identity:
 *
 *  @data1: identity1
 *  @data2: identity2
 *
 *  Return TRUE if identity matches, FALSE otherwise
 **/
boolean scaler44_util_compare_identity(void *data1, void *data2)
{
  uint32_t *identity1 = (uint32_t *)data1;
  uint32_t *identity2 = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }

  if (*identity1 == *identity2) {
    return TRUE;
  }

  return FALSE;
}

/** scaler44_util_find_port_based_on_identity:
 *
 *  @data1: mct port handle
 *  @data2: identity
 *
 *  Retrieve port based on identity
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean scaler44_util_find_port_based_on_identity(void *data1,
  void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  uint32_t   *identity = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }

  if (mct_list_find_custom(MCT_PORT_CHILDREN(port), identity,
    scaler44_util_compare_identity)) {
    return TRUE;
  }

  return FALSE;
}

/** scaler44_util_get_port_from_module:
 *
 *  @module: mct module handle
 *  @port: port to return
 *  @direction: direction of port to be retrieved
 *  @identity: identity to match against
 *
 *  Find port based on direction and identity and return it
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean scaler44_util_get_port_from_module(mct_module_t *module,
  mct_port_t **port, mct_port_direction_t direction, uint32_t identity)
{
  mct_list_t *l_port = NULL;

  if (!module || !port) {
    ISP_ERR("failed: module %p sink_port %p", module, port);
    return FALSE;
  }

  /* Reset output param */
  *port = NULL;

  if ((direction != MCT_PORT_SRC) && (direction != MCT_PORT_SINK)) {
    ISP_ERR("failed: invalid direction %d", direction);
    return FALSE;
  }

  if (direction == MCT_PORT_SINK) {
    l_port = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &identity,
      scaler44_util_find_port_based_on_identity);
    if (!l_port) {
      ISP_ERR("failed: mct_list_find_custom for ide %x", identity);
      return FALSE;
    }
  } else {
    l_port = mct_list_find_custom(MCT_MODULE_SRCPORTS(module), &identity,
      scaler44_util_find_port_based_on_identity);
    if (!l_port) {
      ISP_ERR("failed: mct_list_find_custom for ide %x", identity);
      return FALSE;
    }
  }

  *port = (mct_port_t *)l_port->data;

  return TRUE;
}

/** scaler44_util_compare_identity_from_stream_param:
 *
 *  @data1: stream info handle
 *  @data2: identity
 *
 *  Returns TRUE if identity of stream info matches with
 *  requested identity, FALSE otherwise
 **/
static boolean scaler44_util_compare_identity_from_stream_info(void *data1,
  void *data2)
{
  mct_stream_info_t *stream_info = (mct_stream_info_t *)data1;
  uint32_t          *identity = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }

  if (stream_info->identity == *identity) {
    return TRUE;
  }

  return FALSE;
}

/** scaler44_util_append_stream_info:
 *
 *  @port: port handle
 *
 *  @stream_info: stream info to be appended
 *
 *  Append stream info handle in module private's stream info
 *  list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean scaler44_util_append_stream_info(mct_port_t *port,
  mct_stream_info_t *stream_info)
{
  boolean       ret = TRUE;
  mct_module_t *module = NULL;
  scaler44_t *scaler = NULL;
  mct_list_t   *l_stream_info = NULL;
  isp_hw_stream_info_t *hw_stream_info = NULL;
  boolean  same_aspect_ratio;
  float sensor_aspect_ratio, curr_aspect_ratio;
  float aspect_ratio1, aspect_ratio2;

  if (!port || !stream_info) {
    ISP_ERR("failed: %p %p", port, stream_info);
    return FALSE;
  }

  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  scaler = (scaler44_t *)MCT_OBJECT_PRIVATE(module);
  if (!scaler) {
    ISP_ERR("failed: scaler %p", scaler);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&scaler->mutex);
  /* Check whether stream info is already appended for same stream */
  l_stream_info = mct_list_find_custom(scaler->l_stream_info,
    stream_info, scaler44_util_compare_identity_from_stream_info);
  if (l_stream_info) {
    ISP_DBG("stream info ide %x already present in list",
      stream_info->identity);
    PTHREAD_MUTEX_UNLOCK(&scaler->mutex);
    return TRUE;
  }

  /* Append stream info in module private */
  l_stream_info = mct_list_append(scaler->l_stream_info, stream_info, NULL,
    NULL);
  if (!l_stream_info) {
    ISP_ERR("failed: l_stream_info %p", l_stream_info);
    goto ERROR;
  }

  scaler->l_stream_info = l_stream_info;
  ret = scaler44_init(scaler);
  if (ret == FALSE) {
    ISP_ERR("failed: scaler44_open");
    goto ERROR;
  }

  scaler->session_id = ISP_GET_SESSION_ID(stream_info->identity);
  scaler->state = ISP_STATE_IDLE;

  /* Find super dimension */
  hw_stream_info = &scaler->hw_stream_info;

  if (hw_stream_info->width == 0) {
    hw_stream_info->width = stream_info->dim.width;
     hw_stream_info->height = stream_info->dim.height;
  } else {
    aspect_ratio1 = (float)hw_stream_info->width/hw_stream_info->height;
    aspect_ratio2 = (float)stream_info->dim.width/stream_info->dim.height;
    if (fabs(aspect_ratio1 - aspect_ratio2) < ASPECT_TOLERANCE) {
      same_aspect_ratio = TRUE;
    } else {
      same_aspect_ratio = FALSE;
    }

    if (stream_info->dim.width > hw_stream_info->width) {
      hw_stream_info->width = stream_info->dim.width;
    }
    if (stream_info->dim.height > hw_stream_info->height) {
      hw_stream_info->height = stream_info->dim.height;
    }

    /* If the aspect ratio of the streams sharing the HW stream
       don't match then we need to deriver a bigger dimension
       buffer to overlap the FOV for all the streams */
    if (!same_aspect_ratio) {
      sensor_aspect_ratio = (float)scaler->sensor_out_info.dim_output.width /
        scaler->sensor_out_info.dim_output.height;
      curr_aspect_ratio = (float)hw_stream_info->width / hw_stream_info->height;

      if (curr_aspect_ratio > sensor_aspect_ratio) {
        hw_stream_info->height = hw_stream_info->width / sensor_aspect_ratio;
        hw_stream_info->height = FLOOR2(hw_stream_info->height);
      } else {
        hw_stream_info->width = hw_stream_info->height * sensor_aspect_ratio;
        hw_stream_info->width = FLOOR2(hw_stream_info->width);
      }
    }
    ISP_DBG("Dimension After aspect ratio match [%d * %d]\n",
      hw_stream_info->width,hw_stream_info->height);
  }
  if (stream_info->original_dim.width > hw_stream_info->width_before_dis &&
     (stream_info->stream_type == CAM_STREAM_TYPE_PREVIEW ||
      stream_info->stream_type == CAM_STREAM_TYPE_VIDEO))
         scaler->hw_stream_info.width_before_dis =
           stream_info->original_dim.width;
  if (stream_info->original_dim.height > hw_stream_info->height_before_dis &&
     (stream_info->stream_type == CAM_STREAM_TYPE_PREVIEW ||
      stream_info->stream_type == CAM_STREAM_TYPE_VIDEO))
        scaler->hw_stream_info.height_before_dis =
          stream_info->original_dim.height;
  if (((hw_stream_info->fmt >= CAM_FORMAT_MAX) ||
    (hw_stream_info->fmt == CAM_FORMAT_Y_ONLY)) &&
    (stream_info->stream_type != CAM_STREAM_TYPE_PARM)) {
    /* Overwrite new fmt */
    hw_stream_info->fmt = stream_info->fmt;
  }

  hw_stream_info->identity= stream_info->identity;
  /* If batch mode stream is mapped to this hw port, override it */
  if (hw_stream_info->streaming_mode != CAM_STREAMING_MODE_BATCH)
    hw_stream_info->streaming_mode = stream_info->streaming_mode;
  ISP_HIGH("%s dim %dx%d before_dis %dx%d fmt %d iden 0x%x",
    MCT_MODULE_NAME(module),
    hw_stream_info->width,
    hw_stream_info->height,
    scaler->hw_stream_info.width_before_dis,
    scaler->hw_stream_info.height_before_dis,
    hw_stream_info->fmt,
    hw_stream_info->identity);
  PTHREAD_MUTEX_UNLOCK(&scaler->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&scaler->mutex);
  return FALSE;
}

/** scaler44_util_append_stream_info:
 *
 *  @port: port handle
 *  @identity: identity of stream info to be appended
 *
 *  Remove stream info handle in module private's stream info
 *  list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean scaler44_util_remove_stream_info(mct_port_t *port,
  uint32_t identity)
{
  boolean       ret = TRUE;
  mct_module_t *module = NULL;
  scaler44_t   *scaler = NULL;
  mct_list_t   *l_stream_info = NULL;

  if (!port) {
    ISP_ERR("failed: %p", port);
    return FALSE;
  }

  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed: module %p", module);
    return FALSE;
  }

  scaler = (scaler44_t *)MCT_OBJECT_PRIVATE(module);
  if (!scaler) {
    ISP_ERR("failed: scaler %p", scaler);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&scaler->mutex);
  /* Find stream info in module private */
  l_stream_info = mct_list_find_custom(scaler->l_stream_info,
    &identity, scaler44_util_compare_identity_from_stream_info);
  if (!l_stream_info) {
    PTHREAD_MUTEX_UNLOCK(&scaler->mutex);
    return TRUE;
  }

  scaler->l_stream_info = mct_list_remove(scaler->l_stream_info,
    l_stream_info->data);
  if (!scaler->l_stream_info) {
    memset(&scaler->hw_stream_info, 0, sizeof(scaler->hw_stream_info));
    scaler->hw_stream_info.fmt = CAM_FORMAT_MAX;
  }

  PTHREAD_MUTEX_UNLOCK(&scaler->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&scaler->mutex);
  return FALSE;
}

 /** scaler44_util_single_HW_write:
 *
 *  @cmd_offset: reg cmd offset
 *  @cmd_len: cfg cmd length
 *  @hw_reg_offset: hw reg offset
 *  @reg_num : number of registers
 *  @cmd_data_offset: offset
 *  @cmd_type : type of cmd
 *  @hw_update_list: linked list passed from pipline layer to append
 *        hw update register and data info to update hardware
 *
 *  write reg settings for module to hw regs.
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean scaler44_util_single_HW_write(void* cmd_offset,
  uint32_t cmd_len, uint32_t hw_reg_offset, uint32_t reg_num,
  uint32_t cmd_data_offset, uint32_t cmd_type,
  struct msm_vfe_cfg_cmd_list **hw_update_list)
{
  int rc = 0;
  boolean ret = TRUE;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2 *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd = NULL;
  uint32_t *data = NULL;
  uint32_t len = 0;

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    free(hw_update);
    return FALSE;
  }
  memset(reg_cfg_cmd, 0, (sizeof(*reg_cfg_cmd)));

  cfg_cmd = &hw_update->cfg_cmd;
  cfg_cmd->cfg_data = cmd_offset;
  cfg_cmd->cmd_len = cmd_len;
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = cmd_data_offset;
  reg_cfg_cmd[0].cmd_type = cmd_type;
  reg_cfg_cmd[0].u.rw_info.reg_offset = hw_reg_offset;
  reg_cfg_cmd[0].u.rw_info.len = reg_num * sizeof(uint32_t);

  ISP_HIGH("hw_reg_offset %x, len %d", reg_cfg_cmd[0].u.rw_info.reg_offset,
    reg_cfg_cmd[0].u.rw_info.len);
  data = (uint32_t *)cmd_offset;
  for (len = 0; len < (cmd_len / 4); len++) {
    ISP_HIGH("data[%d] %x", len, data[len]);
  }
  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = scaler44_util_append_cfg(hw_update_list, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: scaler44_util_append_cfg");
    return FALSE;
  }
  return ret;
}


 /** scaler44_util_send_event:
 *
 *  @module: mct module handle
 *  @event: event
 *  @direction: direction of port to be retrieved
 *
 *  Send event based on event identity & direction
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean scaler44_util_send_event(mct_module_t *module,
  mct_event_t *event)
{
  boolean rc = TRUE;
  mct_port_t *port = NULL;
  mct_port_direction_t direction;

  if (event->direction == MCT_EVENT_UPSTREAM)
    direction = MCT_PORT_SINK;
  else
    direction = MCT_PORT_SRC;

  rc = scaler44_util_get_port_from_module(module, &port, direction,
    event->identity);
  if (rc == FALSE || !port ) {
    ISP_ERR("failed: %p rc= %d", port, rc);
    return rc;
  }
  rc = mct_port_send_event_to_peer(port, event);
  if (rc == FALSE) {
    ISP_ERR("failed: rc = FALSE");
    return rc;
  }
  return rc;
}

/** scaler46_util_setloglevel:
 *
 *  @name: ISP hw module unique name
 *  @isp_log_sub_modules_t: module id of the hardware module
 *
 *  This function sets the log level of the hardware module id
 *  passed based on the setprop.
 *  persist.vendor.camera.debug.xxx (xxx = hw module name)
 *  It also arbitrates betweeen the global setprop
 *  persist.vendor.camera.global.debug
 *  The final log level is decided on whichever is higher
 *  0 - ERR Logs
 *  1 - HIGH Logs + ERR logs
 *  2 - HIGH Logs + ERR Logs + DBG logs
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean scaler44_util_setloglevel(const char *name,
  isp_log_sub_modules_t mod_id)
{
  uint32_t      globalloglevel = 0;
  char          prop[PROPERTY_VALUE_MAX];
  char          buf[255];
  uint32_t      isp_module_loglevel = 0,
                isploglevel;

  if (!name) {
    ISP_ERR("failed: %p", name);
    return FALSE;
  }
  strlcpy(buf, "persist.vendor.camera.debug.", sizeof(buf));
  strlcat(buf, name, sizeof(buf));

  property_get(buf, prop, "0");
  isp_module_loglevel = atoi(prop);
  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.camera.global.debug", prop, "0");
  globalloglevel = atoi(prop);
  if (globalloglevel > isp_module_loglevel) {
    isp_module_loglevel = globalloglevel;
  }
  memset(prop, 0, sizeof(prop));
  property_get("persist.vendor.camera.isp.debug", prop, "0");
  isploglevel = atoi(prop);
  if (isploglevel > isp_module_loglevel) {
    isp_module_loglevel = isploglevel;
  }
  isp_modules_loglevel[mod_id] = isp_module_loglevel;
  ISP_DBG("%s:%d ###ISP_Loglevel %d", __func__, __LINE__, isp_module_loglevel);
  return TRUE;
}
