/* fovcrop40_util.c
 *
 * Copyright (c) 2012-2018 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* mctl headers */
#include "mct_module.h"

/* isp headers */
#include "fovcrop40_util.h"
#include "isp_sub_module_log.h"
#include "isp_common.h"

#if defined (_ANDROID_)
#include <cutils/properties.h>
#endif

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_FOVCROP, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_FOVCROP, fmt, ##args)

/** fovcrop40_util_append_cfg:
 *
 *  @hw_update_list: hw update list
 *
 *  @hw_update: hw update cmd to be enqueued
 *
 *  Enqueue hw_update in hw_update_list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean fovcrop40_util_append_cfg(struct msm_vfe_cfg_cmd_list **hw_update_list,
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

/** fovcrop40_util_compare_identity:
 *
 *  @data1: identity1
 *
 *  @data2: identity2
 *
 *  Return TRUE if identity matches, FALSE otherwise
 **/
boolean fovcrop40_util_compare_identity(void *data1, void *data2)
{
  uint32_t *identity1 = (uint32_t *)data1;
  uint32_t *identity2 = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  ISP_DBG("%d %d", *identity1, *identity2);
  if (*identity1 == *identity2) {
    return TRUE;
  }

  return FALSE;
}

/** fovcrop40_util_find_port_based_on_identity:
 *
 *  @data1: mct port handle
 *
 *  @data2: identity
 *
 *  Retrieve port based on identity
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean fovcrop40_util_find_port_based_on_identity(void *data1,
  void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  uint32_t   *identity = (uint32_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: data1 %p data2 %p", data1, data2);
    return FALSE;
  }
  if (mct_list_find_custom(MCT_PORT_CHILDREN(port), identity,
    fovcrop40_util_compare_identity)) {
    return TRUE;
  }

  ISP_HIGH("");
  return FALSE;
}

/** fovcrop40_util_get_port_from_module:
 *
 *  @module: mct module handle
 *
 *  @port: port to return
 *
 *  @direction: direction of port to be retrieved
 *
 *  @identity: identity to match against
 *
 *  Find port based on direction and identity and return it
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean fovcrop40_util_get_port_from_module(mct_module_t *module,
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
      fovcrop40_util_find_port_based_on_identity);
    if (!l_port) {
      ISP_ERR("failed: mct_list_find_custom for ide %x", identity);
      return FALSE;
    }
  } else {
    l_port = mct_list_find_custom(MCT_MODULE_SRCPORTS(module), &identity,
      fovcrop40_util_find_port_based_on_identity);
    if (!l_port) {
      ISP_ERR("failed: mct_list_find_custom for ide %x", identity);
      return FALSE;
    }
  }

  *port = (mct_port_t *)l_port->data;

  return TRUE;
}

 /** fovcrop40_util_single_HW_write:
 *
 *  @fd: module hw fd
 *
 *  @cmd_offset: reg cmd offset
 *
 *  @cmd_len: direction of port to be retrieved
 *
 *  @hw_reg_offset: identity to match
 *
 *  @reg_num : number of registers
 *
 *  @cmd_type : type of cmd
 *
 *  write reg settings for module to hw regs.
 *
 *  Return TRUE on success and FALSE on failure
 **/
int fovcrop40_util_single_HW_write(void* cmd_offset,
  uint32_t cmd_len, uint32_t hw_reg_offset, uint32_t reg_num,
  uint32_t cmd_type, struct msm_vfe_cfg_cmd_list **hw_update_list)
{
  int rc = 0;
  boolean ret = FALSE;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2 *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd;
  uint32_t *data = NULL;
  uint32_t len = 0;

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return -1;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    free(hw_update);
    return -1;
  }
  memset(reg_cfg_cmd, 0, (sizeof(struct msm_vfe_reg_cfg_cmd)));
  cfg_cmd = &hw_update->cfg_cmd;
  cfg_cmd->cfg_data = cmd_offset;
  cfg_cmd->cmd_len = cmd_len;
  cfg_cmd->cfg_cmd = (void *) reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
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
  ret = fovcrop40_util_append_cfg(hw_update_list, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: fovcrop40_util_append_cfg");
    return -1;
  }

  return rc;

}

 /** fovcrop40_util_send_event:
 *
 *  @module: mct module handle
 *  @event: event
 *  @direction: direction of port to be retrieved
 *
 *  Send event based on event identity & direction
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean fovcrop40_util_send_event(mct_module_t *module,
  mct_event_t *event)
{
  boolean rc = TRUE;
  mct_port_t *port = NULL;
  mct_port_direction_t direction;

  if (event->direction == MCT_EVENT_UPSTREAM)
    direction = MCT_PORT_SINK;
  else
    direction = MCT_PORT_SRC;

  rc = fovcrop40_util_get_port_from_module(module, &port, direction,
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

/** fovcrop40_util_setloglevel:
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
boolean fovcrop40_util_setloglevel(const char *name,
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
