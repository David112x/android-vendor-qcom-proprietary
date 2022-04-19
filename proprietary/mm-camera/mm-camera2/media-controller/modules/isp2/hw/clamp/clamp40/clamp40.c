/*============================================================================

  Copyright (c) 2013-2017 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "clamp40.h"

/* #define CLAMP_DEBUG */
#ifdef CLAMP_DEBUG
#undef ISP_DBG
#define ISP_DBG ISP_HIGH
#endif

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_CLAMP, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_CLAMP, fmt, ##args)

/** clamp40_set_stream_config:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Store sensor fmt in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean clamp40_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  sensor_out_info_t *sensor_out_info = NULL;
  clamp40_t         *clamp = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  sensor_out_info =
    (sensor_out_info_t *)event->u.module_event.module_event_data;
  if (!sensor_out_info) {
    ISP_ERR("failed: sensor_out_info %p", sensor_out_info);
    return FALSE;
  }

  clamp = (clamp40_t *)isp_sub_module->private_data;
  if (!clamp) {
    ISP_ERR("failed: demux %p", clamp);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  clamp->fmt = sensor_out_info->fmt;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* clamp40_set_stream_config */

/** vfe_clamp_cmd_debug:
 *
 *    @cmd:
 *    @index:
 *
 **/
static void clamp40_cmd_debug(ISP_OutputClampConfigCmdType cmd)
{
  ISP_DBG("CLAMP y max  %d\n", cmd.yChanMax);
  ISP_DBG("CLAMP cb max %d\n", cmd.cbChanMax);
  ISP_DBG("CLAMP cr max %d\n", cmd.crChanMax);

  ISP_DBG("CLAMP y min %d\n", cmd.yChanMin);
  ISP_DBG("CLAMP cb min %d\n", cmd.cbChanMin);
  ISP_DBG("CLAMP cr min %d\n", cmd.crChanMin);

} /* clamp40_cmd_debug */

/** clamp40_store_hw_update:
 *
 *  @clamp_mod: clamp module handle
 *
 *  update hw update list
 **/
static boolean clamp40_store_hw_update(isp_sub_module_t *isp_sub_module,
  clamp40_t *clamp_mod)
{
  boolean                       ret = TRUE;
  struct msm_vfe_cfg_cmd2      *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd   *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list  *hw_update = NULL;
  ISP_OutputClampConfigCmdType *reg_cmd = NULL;

  if (!isp_sub_module || !clamp_mod) {
    ISP_ERR("failed: isp_sub_module %p clamp_mod %p", isp_sub_module,
      clamp_mod);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)
    CAM_MALLOC(sizeof(struct msm_vfe_cfg_cmd_list));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd =
    (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_FREE_HW_UPDATE;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd));

  reg_cmd = (ISP_OutputClampConfigCmdType *)
    CAM_MALLOC(sizeof(ISP_OutputClampConfigCmdType));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cmd %p", reg_cmd);
    goto ERROR_FREE_REG_CFG_CMD;
  }
  memset(reg_cmd, 0, sizeof(*reg_cmd));

  *reg_cmd = clamp_mod->clamp.reg_cmd;

  cfg_cmd->cfg_data = (void *)reg_cmd;
  cfg_cmd->cmd_len = sizeof(*reg_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd->u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd->cmd_type = VFE_WRITE;
  reg_cfg_cmd->u.rw_info.reg_offset = clamp_mod->reg_offset;
  reg_cfg_cmd->u.rw_info.len = clamp_mod->reg_len * sizeof(uint32_t);

  clamp40_cmd_debug(clamp_mod->clamp.reg_cmd);

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_FREE_REG_CMD;
  }

  return TRUE;

ERROR_FREE_REG_CMD:
  free(reg_cmd);
ERROR_FREE_REG_CFG_CMD:
  free(reg_cfg_cmd);
ERROR_FREE_HW_UPDATE:
  free(hw_update);
  return FALSE;
} /* clamp40_store_hw_update */

/** clamp40_config:
 *
 *  @clamp: clamp handle
 *
 *  configures default values
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean clamp40_config(isp_sub_module_t *isp_sub_module,
  clamp40_t *clamp)
{
  boolean ret = TRUE;

  if (!isp_sub_module || !clamp) {
    ISP_ERR("failed: isp_sub_module %p clamp %p", isp_sub_module, clamp);
    return FALSE;
  }
  if (clamp->clamp_type == ISP_CLAMP_ENCODER) {
    clamp->clamp.reg_cmd.cbChanMax  = 255;
    clamp->clamp.reg_cmd.cbChanMin  = 0;
    clamp->clamp.reg_cmd.crChanMax  = 255;
    clamp->clamp.reg_cmd.crChanMin  = 0;
    clamp->clamp.reg_cmd.yChanMax   = 255;
    clamp->clamp.reg_cmd.yChanMin   = 0;
  } else if (clamp->clamp_type == ISP_CLAMP_VIEWFINDER) {
    clamp->clamp.reg_cmd.cbChanMax  = 255;
    clamp->clamp.reg_cmd.cbChanMin  = 0;
    clamp->clamp.reg_cmd.crChanMax  = 255;
    clamp->clamp.reg_cmd.crChanMin  = 0;
    clamp->clamp.reg_cmd.yChanMax   = 255;
    clamp->clamp.reg_cmd.yChanMin   = 0;
  } else if (clamp->clamp_type == ISP_CLAMP_VIDEO) {
    clamp->clamp.reg_cmd.cbChanMax  = 255;
    clamp->clamp.reg_cmd.cbChanMin  = 0;
    clamp->clamp.reg_cmd.crChanMax  = 255;
    clamp->clamp.reg_cmd.crChanMin  = 0;
    clamp->clamp.reg_cmd.yChanMax   = 255;
    clamp->clamp.reg_cmd.yChanMin   = 0;
  }

  /* Currently display does not support single plane.
   * So if the output buffer is 2 plane, we grey out the chroma
   * plane here for Mono sensor */
  switch (clamp->fmt) {
  case CAM_FORMAT_Y_ONLY:
  case CAM_FORMAT_Y_ONLY_10_BPP:
  case CAM_FORMAT_Y_ONLY_12_BPP:
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
    clamp->clamp.reg_cmd.cbChanMax  = 128;
    clamp->clamp.reg_cmd.cbChanMin  = 128;
    clamp->clamp.reg_cmd.crChanMax  = 128;
    clamp->clamp.reg_cmd.crChanMin  = 128;
    break;

  default:
    break;
  }

  ret = clamp40_store_hw_update(isp_sub_module, clamp);
  if (ret == FALSE) {
    ISP_ERR("failed: ret %d", ret);
  }

  return ret;
} /* clamp40_config */

/** clamp40_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Trigger update
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean clamp40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret = TRUE;
  clamp40_t               *mod = NULL;
  isp_sub_module_priv_t   *isp_sub_module_priv = NULL;
  isp_private_event_t     *private_event = NULL;
  isp_sub_module_output_t *output = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }

  mod = (clamp40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->trigger_update_pending == TRUE) {
    ret = clamp40_config(isp_sub_module, mod);
    if (ret == FALSE) {
      ISP_ERR("failed: clamp40_config ret %d", ret);
      goto ERROR;
    }

    private_event =
      (isp_private_event_t *)event->u.module_event.module_event_data;
    if (!private_event) {
      ISP_ERR("failed: private_event %p", private_event);
      goto ERROR;
    }

    output = (isp_sub_module_output_t *)private_event->data;
    if (!output) {
      ISP_ERR("failed: output %p", output);
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return FALSE;
    }

    ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    }

    isp_sub_module->trigger_update_pending = FALSE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* clamp40_trigger_update */

/** clamp_init:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the clamp module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean clamp40_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  clamp40_t *clamp = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  clamp = (clamp40_t *)CAM_MALLOC(sizeof(clamp40_t));
  if (!clamp) {
    ISP_ERR("failed: clamp %p", clamp);
    return FALSE;
  }

  memset(clamp, 0, sizeof(*clamp));

  /* Initialize hw stream specific params */
  if (!strncmp(MCT_MODULE_NAME(module),
       CLAMP40_VIEWFINDER_NAME(CLAMP40_VERSION),
       strlen(MCT_MODULE_NAME(module)))) {
    ISP_HIGH("viewfinder");
    clamp->clamp_type = ISP_CLAMP_VIEWFINDER;
    clamp->reg_offset = ISP_CLAMP_VIEW_OFF;
    clamp->reg_len = 2;
  } else if (!strncmp(MCT_MODULE_NAME(module),
              CLAMP40_ENCODER_NAME(CLAMP40_VERSION),
              strlen(MCT_MODULE_NAME(module)))) {
    ISP_HIGH("encoder");
    clamp->clamp_type = ISP_CLAMP_ENCODER;
    clamp->reg_offset = ISP_CLAMP_ENC_OFF;
    clamp->reg_len = 2;
  } else if (!strncmp(MCT_MODULE_NAME(module),
              CLAMP40_VIDEO_NAME(CLAMP40_VERSION),
              strlen(MCT_MODULE_NAME(module)))) {
    ISP_HIGH("video");
    clamp->clamp_type = ISP_CLAMP_VIDEO;
    clamp->reg_offset = ISP_CLAMP_VIDEO_OFF;
    clamp->reg_len = 2;
  } else {
    ISP_ERR("failed: invalid module name %s", MCT_MODULE_NAME(module));
    free(clamp);
    return FALSE;
  }

  isp_sub_module->private_data = (void *)clamp;
  isp_sub_module->trigger_update_pending = TRUE;

  return TRUE;
}/* clamp_init */

/** clamp40_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void clamp40_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* clamp40_destroy */
