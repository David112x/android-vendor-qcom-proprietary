/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "color_xform40.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COLOR_XFORM, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COLOR_XFORM, fmt, ##args)

/** color_xform40_cmd_debug:
 *
 *  @cmd: container of HW configuration
 *  @color_xform_path: stream path ENCODER/VIEWFINDER
 *
 *  Print xform coefficients
 *
 *  Return: None
 **/
static void color_xform40_cmd_debug(ISP_ColorXformCfgCmdType *cmd,
  isp_color_xform_type color_xform_path)
{
  if (color_xform_path == ISP_COLOR_XFORM_ENCODER) {
    ISP_DBG("%s: xform coefficients for encoder path: \n", __func__);
  } else if (color_xform_path == ISP_COLOR_XFORM_VIEWFINDER) {
    ISP_DBG("%s: xform coefficients for viewfinder path: \n", __func__);
  } else {
    return;
  }

  ISP_DBG("%s: Coefficients: \n"
    "m00 = 0x%x, m01 = 0x%x, m02 = 0x%x, o0 = 0x%x, s0 = 0x%x\n"
    "ml0 = 0x%x, ml1 = 0x%x, ml2 = 0x%x, o1 = 0x%x, s1 = 0x%x\n"
    "m20 = 0x%x, m21 = 0x%x, m22 = 0x%x, o2 = 0x%x, s2 = 0x%x\n"
    "c01 = 0x%x, c11 = 0x%x, c21 = 0x%x, c00 = 0x%x, c10 = 0x%x, c20 =0x%x\n",
    __func__,
    cmd->m00, cmd->m01, cmd->m02, cmd->o0, cmd->s0, cmd->m10, cmd->m11,
    cmd->m12, cmd->o1, cmd->s1, cmd->m20, cmd->m21, cmd->m22, cmd->o2, cmd->s2,
    cmd->c01, cmd->c11, cmd->c21, cmd->c00, cmd->c10, cmd->c20);
} /* color_xform40_cmd_debug */

/** color_xform40_update_s_value
 *    @mod: color xform module control
 *    @in_params: color xform hw pix settings
 *    @in_params_size:
 *
 *  color xform update s0, s1 & s2 value based on scaler ratio
 *
 * Return: TRUE if success FALSE if fail
 **/
static boolean color_xform40_update_s_value(color_xform40_t *color_xform,
    isp_sub_module_t *isp_sub_module)
{
  boolean   rc = TRUE;
  ISP_ColorXformCfgCmdType *reg_cmd = NULL;

 #if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION >= 0x306)
  chromatix_parms_type    *chromatix_ptr = NULL;
  Chromatix_color_space_transform_type *chromatix_CST = NULL;

  if (!isp_sub_module->chromatix_ptrs.chromatixPtr) {
    ISP_ERR("failed: chromatix_ptr %p",
      isp_sub_module->chromatix_ptrs.chromatixPtr);
    return FALSE;
  }
  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;

  chromatix_CST =
    &chromatix_ptr->chromatix_VFE.chromatix_xform_data;

  reg_cmd = &color_xform->reg_cmd;
  if (color_xform->scaling_factor > chromatix_CST->scaling_threshold) {
    reg_cmd->s0 = chromatix_CST->subtractive_offset[0];
    reg_cmd->s1 = chromatix_CST->subtractive_offset[1];
    reg_cmd->s2 = chromatix_CST->subtractive_offset[2];
  } else {
    reg_cmd->s0 = 0;
    reg_cmd->s1 = 0;
    reg_cmd->s2 = 0;
  }
 #else
  reg_cmd = &color_xform->reg_cmd;
  if (color_xform->scaling_factor > COLOR_XFORM_SVALUE_THRESHOLD) {
    reg_cmd->s0 = 0;
    reg_cmd->s1 = -1;
    reg_cmd->s2 = -1;
  } else {
    reg_cmd->s0 = 0;
    reg_cmd->s1 = 0;
    reg_cmd->s2 = 0;
  }
 #endif
  ISP_DBG("path %d scaling_factor %f",
    color_xform->color_xform_path, color_xform->scaling_factor);

  return rc;
}

/** color_xform40_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @color_xform: color_xform module handle
 *
 *  Update hw update list
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean color_xform40_store_hw_update(isp_sub_module_t *isp_sub_module,
  color_xform40_t *color_xform)
{
  boolean                       ret = TRUE;
  struct msm_vfe_cfg_cmd2      *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd   *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list  *hw_update = NULL;
  ISP_ColorXformCfgCmdType     *reg_cmd = NULL;

  if (!isp_sub_module || !color_xform) {
    ISP_ERR("failed: isp_sub_module %p color_xform %p", isp_sub_module,
      color_xform);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_FREE_HW_UPDATE;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd));

  reg_cmd = (ISP_ColorXformCfgCmdType *)CAM_MALLOC(sizeof(*reg_cmd));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cmd %p", reg_cmd);
    goto ERROR_FREE_REG_CFG_CMD;
  }
  memset(reg_cmd, 0, sizeof(*reg_cmd));

  *reg_cmd = color_xform->reg_cmd;

  cfg_cmd->cfg_data = (void *)reg_cmd;
  cfg_cmd->cmd_len = sizeof(*reg_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd->u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd->cmd_type = VFE_WRITE;
  reg_cfg_cmd->u.rw_info.reg_offset = color_xform->reg_offset;
  reg_cfg_cmd->u.rw_info.len = color_xform->reg_len * sizeof(uint32_t);

  color_xform40_cmd_debug(&color_xform->reg_cmd, color_xform->color_xform_path);

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
} /* color_xform40_store_hw_update */

/** color_xform_config_601_to_709
 *
 *  @reg_cmd: container of HW configuration
 *
 *  This function sets hw registers for 601 to 709 color xform
 *  Pix range: (0-255 YCbCr:601) to (16-235 Y, 16-240 CbCr:709
 *  HDTV standard)
 *
 *  Return: nothing
 **/
/* Encoder 601 to 709 coefficients
 * from system team
 * 0x071c = 0x1f98036f, 0xffffffff
 * 0x0720 = 0x000d5f44, 0xffffffff
 * 0x0724 = 0x03980000, 0xffffffff
 * 0x0728 = 0x00000068, 0xffffffff
 * 0x072c = 0x00440000, 0xffffffff
 * 0x0730 = 0x0000039e, 0xffffffff
 * 0x0734 = 0x00f0f0eb, 0xffffffff
 * 0x0738 = 0x00101010, 0xffffffff
 */
static void color_xform_config_601_to_709(ISP_ColorXformCfgCmdType *reg_cmd)
{
  /* VFE_COLOR_XFORM_ENC_Y_MATRIX_0 0x071c = 0x1f98036f */
  reg_cmd->m00 = 0x36f;
  reg_cmd->m01 = 0x1f98;
  /* VFE_COLOR_XFORM_ENC_Y_MATRIX_1 0x0720 = 0x000d5f44 */
  reg_cmd->m02 = 0x1f44;
  reg_cmd->o0 = 0x35;
  reg_cmd->s0 = 0x0;
  /* VFE_COLOR_XFORM_ENC_CB_MATRIX_0 0x0724 = 0x03980000 */
  reg_cmd->m10 = 0x0;
  reg_cmd->m11 = 0x398;
  /* VFE_COLOR_XFORM_ENC_CB_MATRIX_1 0x0728 = 0x00000068 */
  reg_cmd->m12 = 0x68;
  reg_cmd->o1 = 0x0;
  reg_cmd->s1 = 0x0;
  /* VFE_COLOR_XFORM_ENC_CR_MATRIX_0 0x072c = 0x00440000 */
  reg_cmd->m20 = 0x0;
  reg_cmd->m21 = 0x44;
  /* VFE_COLOR_XFORM_ENC_CR_MATRIX_1 0x0730 = 0x0000039e */
  reg_cmd->m22 = 0x39e;
  reg_cmd->o2 = 0x0;
  reg_cmd->s2 = 0x0;
  /* VFE_COLOR_XFORM_ENC_CLAMP_HI 0x0734 = 0x00f0f0eb */
  reg_cmd->c01 = 0xeb;
  reg_cmd->c11 = 0xf0;
  reg_cmd->c21 = 0xf0;
  /* VFE_COLOR_XFORM_ENC_CLAMP_LO 0x0738 = 0x00101010 */
  reg_cmd->c00 = 0x10;
  reg_cmd->c10 = 0x10;
  reg_cmd->c20 = 0x10;
}

/** color_xform_config_601_to_601
 *
 *  @reg_cmd: container of HW configuration
 *
 *  This function sets hw registers for 601 to 601 color xform
 *  Pix range: (0-255 YCbCr:601) to (16-235 Y, 16-240 CbCr:601
 *  SDTV standard)
 *
 *  Return: nothing
 **/
/* Encoder 601 to 601 coefficients
 * from system team
 * 0x071c = 0x00000400, 0xffffffff
 * 0x0720 = 0000000000, 0xffffffff
 * 0x0724 = 0x04000000, 0xffffffff
 * 0x0728 = 0000000000, 0xffffffff
 * 0x072c = 0000000000, 0xffffffff
 * 0x0730 = 0x00000400, 0xffffffff
 * 0x0734 = 0x00ffffff, 0xffffffff
 * 0x0738 = 0000000000, 0xffffffff
 */
static void color_xform_config_601_to_601(ISP_ColorXformCfgCmdType *reg_cmd)
{
  /* VFE_COLOR_XFORM_ENC_Y_MATRIX_0 0x071c = 0x00000400 */
  reg_cmd->m00 = 0x400;
  reg_cmd->m01 = 0x0;
  /* VFE_COLOR_XFORM_ENC_Y_MATRIX_1 0x0720 = 0000000000 */
  reg_cmd->m02 = 0x0;
  reg_cmd->o0 = 0x0;
  reg_cmd->s0 = 0x0;
  /* VFE_COLOR_XFORM_ENC_CB_MATRIX_0 0x0724 = 0x04000000 */
  reg_cmd->m10 = 0x0;
  reg_cmd->m11 = 0x400;
  /* VFE_COLOR_XFORM_ENC_CB_MATRIX_1 0x0728 = 0000000000 */
  reg_cmd->m12 = 0x0;
  reg_cmd->o1 = 0x0;
  reg_cmd->s1 = 0x0;
  /* VFE_COLOR_XFORM_ENC_CR_MATRIX_0 0x072c = 0000000000 */
  reg_cmd->m20 = 0x0;
  reg_cmd->m21 = 0x0;
  /* VFE_COLOR_XFORM_ENC_CR_MATRIX_1 0x0730 = 0x00000400 */
  reg_cmd->m22 = 0x400;
  reg_cmd->o2 = 0x0;
  reg_cmd->s2 = 0x0;
  /* VFE_COLOR_XFORM_ENC_CLAMP_HI 0x0734 = 0x00ffffff */
  reg_cmd->c01 = 0xff;
  reg_cmd->c11 = 0xff;
  reg_cmd->c21 = 0xff;
  /* VFE_COLOR_XFORM_ENC_CLAMP_LO 0x0738 = 0000000000 */
  reg_cmd->c00 = 0x0;
  reg_cmd->c10 = 0x0;
  reg_cmd->c20 = 0x0;
}

/** color_xform_config_601_to_601_sdtv
 *
 *  @reg_cmd: container of HW configuration
 *
 *  This function sets hw registers for 601 to 601 color xform
 *  Pix range: (0-255 YCbCr:601) to (0-255 YCbCr:601) no change
 *
 *  Return: nothing
 **/
/* Encoder 601 to 601_sdtv coefficients
 * from system team
 * 0x071c = 0x00000370
 * 0x0720 = 0x00040000
 * 0x0724 = 0x03870000
 * 0x0728 = 0x0003c000
 * 0x072c = 0x00010000
 * 0x0730 = 0x0003c387
 * 0x0734 = 0x00f0f0eb
 * 0x0738 = 0x00101010
 */
static void color_xform_config_601_to_601_sdtv(ISP_ColorXformCfgCmdType *reg_cmd)
{
  /* VFE_COLOR_XFORM_ENC_Y_MATRIX_0 0x071c = 0x00000370 */
  reg_cmd->m00 = 0x0370;
  reg_cmd->m01 = 0x0;
  /* VFE_COLOR_XFORM_ENC_Y_MATRIX_1 0x0720 = 0x00040000 */
  reg_cmd->m02 = 0x0;
  reg_cmd->o0  = 0x10;
  reg_cmd->s0  = 0x0;
  /* VFE_COLOR_XFORM_ENC_CB_MATRIX_0 0x0724 = 0x03870000 */
  reg_cmd->m10 = 0x0;
  reg_cmd->m11 = 0x387;
  /* VFE_COLOR_XFORM_ENC_CB_MATRIX_1 0x0728 = 0x0003c000 */
  reg_cmd->m12 = 0x0;
  reg_cmd->o1  = 0x0f;
  reg_cmd->s1  = 0x0;
  /* VFE_COLOR_XFORM_ENC_CR_MATRIX_0 0x072c = 0x00010000 */
  reg_cmd->m20 = 0x0;
  reg_cmd->m21 = 0x1;
  /* VFE_COLOR_XFORM_ENC_CR_MATRIX_1 0x0730 = 0x0003c387 */
  reg_cmd->m22 = 0x387;
  reg_cmd->o2  = 0x0f;
  reg_cmd->s2  = 0x0;
  /* VFE_COLOR_XFORM_ENC_CLAMP_HI 0x0734 = 0x00f0f0eb */
  reg_cmd->c01 = 0xeb;
  reg_cmd->c11 = 0xf0;
  reg_cmd->c21 = 0xf0;
  /* VFE_COLOR_XFORM_ENC_CLAMP_LO 0x0738 = 0x00101010 */
  reg_cmd->c00 = 0x10;
  reg_cmd->c10 = 0x10;
  reg_cmd->c20 = 0x10;
}

/** color_xform40_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @color_xform: color_xform handle
 *
 *  Configures default values
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean color_xform40_config(isp_sub_module_t *isp_sub_module,
  color_xform40_t *color_xform)
{
  color_xform_type_t color_xform_type = XFORM_MAX;
  boolean ret = TRUE;

  if (!isp_sub_module || !color_xform) {
    ISP_ERR("failed: isp_sub_module %p color_xform %p", isp_sub_module, color_xform);
    return FALSE;
  }

  color_xform_type = XFORM_601_601;

  color_xform->streaming_mode_burst =
    IS_BURST_STREAMING(color_xform->streaming_mode_mask);


  switch (color_xform_type) {
  case XFORM_601_601:
    color_xform_config_601_to_601(&color_xform->reg_cmd);
    break;

  case XFORM_601_601_SDTV:
    if (color_xform->streaming_mode_burst)
      color_xform_config_601_to_601(&color_xform->reg_cmd);
    else
      color_xform_config_601_to_601_sdtv(&color_xform->reg_cmd);
    break;

  case XFORM_601_709:
    if (color_xform->streaming_mode_burst)
      color_xform_config_601_to_601(&color_xform->reg_cmd);
    else
      color_xform_config_601_to_709(&color_xform->reg_cmd);
    break;

  case XFORM_MAX:
  default:
    ISP_ERR("%s: unsupported color xform: %d", __func__, color_xform_type);
    return FALSE;
  }

  ret = color_xform40_update_s_value(color_xform,isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: upadte_s_value %d", ret);
  }

  ret = color_xform40_store_hw_update(isp_sub_module, color_xform);
  if (ret == FALSE) {
    ISP_ERR("failed: ret %d", ret);
  }

  return ret;
} /* color_xform40_config */

/** color_xform40_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Perform trigger update if trigger_update_pending flag is
 *  TRUE and append hw update list in global list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean color_xform40_scaler_output_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                       ret = TRUE;
  color_xform40_t              *color_xform = NULL;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;
  float                         scaling_factor = 0.0;

  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  color_xform = (color_xform40_t *)isp_sub_module->private_data;
  if (!color_xform) {
    ISP_ERR("failed: color_xform %p", color_xform);
    goto ERROR;
  }

  private_event = (isp_private_event_t *)
    event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  scaling_factor = *(float *)private_event->data;

  if (color_xform->scaling_factor != scaling_factor) {
    ISP_HIGH("path %d scaling factor changed %f",
      color_xform->color_xform_path, scaling_factor);
    color_xform->scaling_factor = scaling_factor;
    isp_sub_module->trigger_update_pending = TRUE;
  }

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
}

/** color_xform40_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Perform trigger update if trigger_update_pending flag is
 *  TRUE and append hw update list in global list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean color_xform40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                       ret = TRUE;
  color_xform40_t              *color_xform = NULL;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;
  float                         scaling_factor = 0.0;

  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  color_xform = (color_xform40_t *)isp_sub_module->private_data;
  if (!color_xform) {
    ISP_ERR("failed: color_xform %p", color_xform);
    goto ERROR;
  }

  private_event = (isp_private_event_t *)
    event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (isp_sub_module->trigger_update_pending) {
     ret = color_xform40_config(isp_sub_module, color_xform);
     if (ret == FALSE) {
       ISP_ERR("failed: color_xform40_config ret %d", ret);
       goto ERROR;
     }

    isp_sub_module->trigger_update_pending = FALSE;
  }
  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** color_xform40_init:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the color_xform module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean color_xform40_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  color_xform40_t *color_xform = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  color_xform = (color_xform40_t *)CAM_MALLOC(sizeof(color_xform40_t));
  if (!color_xform) {
    ISP_ERR("failed: color_xform %p", color_xform);
    return FALSE;
  }

  memset(color_xform, 0, sizeof(*color_xform));

  /* Initialize hw stream specific params */
  if (!strncmp(MCT_MODULE_NAME(module),
       COLOR_XFORM40_VIEWFINDER_NAME(COLOR_XFORM40_VERSION),
       strlen(MCT_MODULE_NAME(module)))) {
    ISP_HIGH("viewfinder");
    color_xform->color_xform_path = ISP_COLOR_XFORM_VIEWFINDER;
    color_xform->reg_offset = ISP_COLOR_XFORM40_VIEW_OFF;
    color_xform->reg_len = ISP_COLOR_XFORM40_VIEW_LEN;
  } else if (!strncmp(MCT_MODULE_NAME(module),
              COLOR_XFORM40_ENCODER_NAME(COLOR_XFORM40_VERSION),
              strlen(MCT_MODULE_NAME(module)))) {
    ISP_HIGH("encoder");
    color_xform->color_xform_path = ISP_COLOR_XFORM_ENCODER;
    color_xform->reg_offset = ISP_COLOR_XFORM40_ENC_OFF;
    color_xform->reg_len = ISP_COLOR_XFORM40_VIEW_LEN;
  } else {
    ISP_ERR("failed: invalid module name %s", MCT_MODULE_NAME(module));
  }

  isp_sub_module->private_data = (void *)color_xform;
  isp_sub_module->trigger_update_pending = TRUE;

  return TRUE;
}/* color_xform_init */

/** color_xform40_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void color_xform40_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* color_xform40_destroy */
