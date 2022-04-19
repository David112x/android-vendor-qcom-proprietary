/* color_correct40.c
 *
 * Copyright (c) 2013-2014,2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>

/* mctl headers */
#include "eztune_vfe_diagnostics.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COLOR_CORRECT, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COLOR_CORRECT, fmt, ##args)

/* isp headers */
#include "module_color_correct40.h"
#include "color_correct40.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module_util.h"

#define DUMP_CC
#define MAX_CC_GAIN 3.9

#define LINEAR_INTERPOLATION_CC_GAIN(v1, v2, ratio, gain) \
  ((((v2)*(1-ratio)) + ((ratio) * (v1))) * gain)

#define GET_CC_MATRIX(CC, M) ({ \
  M[0][0] = CC->c0; \
  M[0][1] = CC->c1; \
  M[0][2] = CC->c2; \
  M[1][0] = CC->c3; \
  M[1][1] = CC->c4; \
  M[1][2] = CC->c5; \
  M[2][0] = CC->c6; \
  M[2][1] = CC->c7; \
  M[2][2] = CC->c8; })

  /* Chromatix stores the coeffs in RGB order whereas              *
   * VFE stores the coeffs in GBR order. Hence c0 maps to M[1][1]  */
#define SET_ISP_CC_MATRIX(CC, M, q) ({ \
  CC->C0 = roundf((float)(1 << q) * (float)(M[1][1])); \
  CC->C1 = roundf((float)(1 << q) * (float)(M[1][2])); \
  CC->C2 = roundf((float)(1 << q) * (float)(M[1][0])); \
  CC->C3 = roundf((float)(1 << q) * (float)(M[2][1])); \
  CC->C4 = roundf((float)(1 << q) * (float)(M[2][2])); \
  CC->C5 = roundf((float)(1 << q) * (float)(M[2][0])); \
  CC->C6 = roundf((float)(1 << q) * (float)(M[0][1])); \
  CC->C7 = roundf((float)(1 << q) * (float)(M[0][2])); \
  CC->C8 = roundf((float)(1 << q) * (float)(M[0][0])); \
})


#define CC_APPLY_GAIN_ROUND(cc, gain) ({ \
  cc->c0 = roundf(cc->c0 * gain); \
  cc->c1 = roundf(cc->c1 * gain); \
  cc->c2 = roundf(cc->c2 * gain); \
  cc->c3 = roundf(cc->c3 * gain); \
  cc->c4 = roundf(cc->c4 * gain); \
  cc->c5 = roundf(cc->c5 * gain); \
  cc->c6 = roundf(cc->c6 * gain); \
  cc->c7 = roundf(cc->c7 * gain); \
  cc->c8 = roundf(cc->c8 * gain); \
  cc->k0 = roundf(cc->k0 * gain); \
  cc->k1 = roundf(cc->k1 * gain); \
  cc->k2 = roundf(cc->k2 * gain); \
})

/* Chromatix stores the coeffs in RGB order whereas             *
 * VFE stores the coeffs in GBR order. Hence c0 maps to M[1][1] */
#define SET_CC_TBL_FROM_AWB_MATRIX(CC_TABLE, M) ({ \
  CC_TABLE->c0 = M[0][0]; \
  CC_TABLE->c1 = M[0][1]; \
  CC_TABLE->c2 = M[0][2]; \
  CC_TABLE->c3 = M[1][0]; \
  CC_TABLE->c4 = M[1][1]; \
  CC_TABLE->c5 = M[1][2]; \
  CC_TABLE->c6 = M[2][0]; \
  CC_TABLE->c7 = M[2][1]; \
  CC_TABLE->c8 = M[2][2]; })

static boolean color_correct40_fill_vfe_diag_data(color_correct40_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output);

/** color_correct40_debug:
 *
 *  @cmd: configuration command
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function dumps color correction configuration
 *
 *  Return: None
 **/
static void color_correct40_debug(void *cmd)
{
  ISP_DBG("VFE_Chroma_enhance40/ color conversion update");

  uint32_t* p_cmd = (uint32_t *)cmd;

  ISP_DBG("C[0-8] = %d, %d, %d, %d, %d, %d, %d, %d, %d\n", p_cmd[0],
    p_cmd[1], p_cmd[2], p_cmd[3], p_cmd[4], p_cmd[5], p_cmd[6],
    p_cmd[7], p_cmd[8]);

  ISP_DBG("K[0-2] = %d, %d, %d, q %d\n", p_cmd[9], p_cmd[10], p_cmd[11],
    p_cmd[12]);

}

/** color_correct40_store_hw_update:
 *
 *  @isp_sub_mod: sub module handle
 *  @color_correct_mod: module private
 *
 *  This function checks and sends configuration update to kernel
 *
 *  Return:  boolean
 **/
static boolean color_correct40_store_hw_update(
  isp_sub_module_t *isp_sub_module,
  color_correct40_t *color_correct_mod)
{
  boolean ret = TRUE;
  int i, rc = 0;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2 *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd = NULL;
  ISP_ColorCorrectionCfgCmdType *reg_cmd = NULL;

  if (!color_correct_mod) {
    ISP_ERR("failed: %p", color_correct_mod);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  cfg_cmd = &hw_update->cfg_cmd;
  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_MALLOC;
  }
  memset(reg_cfg_cmd, 0, (sizeof(struct msm_vfe_reg_cfg_cmd)));

  reg_cmd = (ISP_ColorCorrectionCfgCmdType *)CAM_MALLOC(sizeof(*reg_cmd));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CMD_MALLOC;
  }
  memset(reg_cmd, 0, sizeof(*reg_cmd));

  *reg_cmd = color_correct_mod->RegCmd;

  cfg_cmd->cfg_data = (void *)reg_cmd;
  cfg_cmd->cmd_len = sizeof(*reg_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_COLOR_COR40_OFF;
  reg_cfg_cmd[0].u.rw_info.len = ISP_COLOR_COR40_LEN * sizeof(uint32_t);

  color_correct40_debug(&color_correct_mod->RegCmd);
  color_correct_mod->applied_RegCmd = color_correct_mod->RegCmd;

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: color_correct40_util_append_cfg");
    goto ERROR_APPEND;
  }

  return ret;

ERROR_APPEND:
  free(reg_cmd);
ERROR_REG_CMD_MALLOC:
  free(reg_cfg_cmd);
ERROR_REG_CFG_MALLOC:
  free(hw_update);

  return FALSE;
}

/** color_correct40_set_hw_params:
 *
 *    @ p_cmd: module config
 *    @ effect_matrix: color transform matrix
 *    @ p_cc: color correction table
 *    @ dig_gain: digital gain on CC
 *
 * copy from mod->threshold to reg cmd then configure
 *
 **/
static void color_correct40_set_hw_params(
  ISP_ColorCorrectionCfgCmdType* p_cmd, float effects_matrix[3][3],
  chromatix_color_correction_type *p_cc, float dig_gain)
{
 int i, j;
 float coeff[3][3], out_coeff[3][3];

#ifdef ENABLE_CC_LOGGING
  ISP_ERR("print out_coeff");
  PRINT_2D_MATRIX(3, 3, effects_matrix);
#endif

  if (IS_UNITY_MATRIX(effects_matrix, 3)) {
    ISP_DBG("No effects enabled");
    GET_CC_MATRIX(p_cc, out_coeff);
  } else {
    ISP_DBG("Effects enabled");
    GET_CC_MATRIX(p_cc, coeff);
    MATRIX_MULT(effects_matrix, coeff, out_coeff, 3, 3, 3);
  }

#ifdef ENABLE_CC_LOGGING
  ISP_ERR("print out_coeff");
  PRINT_2D_MATRIX(3, 3, out_coeff);
#endif

  SET_ISP_CC_MATRIX(p_cmd, out_coeff, 7);
  ISP_DBG("dig_gain %5.3f", dig_gain);

  p_cmd->K0 = p_cc->k1 * (1 << 7);
  p_cmd->K1 = p_cc->k2 * (1 << 7);
  p_cmd->K2 = p_cc->k0 * (1 << 7);
  p_cmd->coefQFactor = p_cc->q_factor;


}

/** color_correct40_populate_matrix:
 *    @m: matrix
 *    @s: saturation
 *
 *  This function calculates the CC matrix valuse for given saturation
 *
 *  Return: None
 **/
static void color_correct40_populate_matrix(float m[3][3], float s)
{
  ISP_DBG("satuation = %f\n", s);
  m[0][0] = 0.2990 + 1.4075 * 0.498 * s;
  m[0][1] = 0.5870 - 1.4075 * 0.417 * s;
  m[0][2] = 0.1140 - 1.4075 * 0.081 * s;
  m[1][0] =
    0.2990 + 0.3455 * 0.168 * s - 0.7169 * 0.498 * s;
  m[1][1] =
    0.5870 + 0.3455 * 0.330 * s + 0.7169 * 0.417 * s;
  m[1][2] =
    0.1140 - 0.3455 * 0.498 * s + 0.7169 * 0.081 * s;
  m[2][0] = 0.2990 - 1.7790 * 0.168 * s;
  m[2][1] = 0.5870 - 1.7790 * 0.330 * s;
  m[2][2] = 0.1140 + 1.7790 * 0.498 * s;
}

/** color_correct40_convert_table:
 *    @pInCC: input chromatix CC table
 *    @pOutCC: output module CC table
 *
 *  This function prepares color correction table
 *
 *  Return: None
 **/
#if 0
static void color_correct40_convert_table(
  chromatix_color_correction_type *pInCC, chromatix_color_correction_type  *pOutCC)
{
  ISP_DBG("enter.");
  pOutCC->c0 = CC_COEFF(pInCC->c0, pInCC->q_factor+7);
  pOutCC->c1 = CC_COEFF(pInCC->c1, pInCC->q_factor+7);
  pOutCC->c2 = CC_COEFF(pInCC->c2, pInCC->q_factor+7);
  pOutCC->c3 = CC_COEFF(pInCC->c3, pInCC->q_factor+7);
  pOutCC->c4 = CC_COEFF(pInCC->c4, pInCC->q_factor+7);
  pOutCC->c5 = CC_COEFF(pInCC->c5, pInCC->q_factor+7);
  pOutCC->c6 = CC_COEFF(pInCC->c6, pInCC->q_factor+7);
  pOutCC->c7 = CC_COEFF(pInCC->c7, pInCC->q_factor+7);
  pOutCC->c8 = CC_COEFF(pInCC->c8, pInCC->q_factor+7);
  pOutCC->k0 = pInCC->k0;
  pOutCC->k1 = pInCC->k1;
  pOutCC->k2 = pInCC->k2;
  pOutCC->q_factor = pInCC->q_factor+7;
}
#endif

/** color_correct40_print_cc_params:
 *
 *  @params: color correct params
 *
 *  Print color correct params
 *
 *  Returns void
 **/
static void color_correct40_print_cc_params(
  chromatix_color_correction_type *params)
{
  if (!params) {
    return;
  }
  ISP_DBG("cc %f %f %f %f %f %f %f %f %f", params->c0, params->c1, params->c2,
    params->c3, params->c4, params->c5, params->c6, params->c7, params->c8);

  ISP_DBG("%d %d %d %d", params->q_factor, params->k0, params->k1, params->k2);
}

/** color_correct40_interpolate:
 *    @in1: start color correction
 *    @in2: end color correction
 *    @out: interpolated correction
 *    @ratio: interpolation ratio
 *
 *  This function interpolates between two color correction presets
 *
 *  Return: None
 **/
void color_correct40_interpolate(chromatix_color_correction_type *in1,
  chromatix_color_correction_type  *in2, chromatix_color_correction_type *out,
  float ratio,float gain)
{
  ISP_DBG("gain %f ratio %f", gain, ratio);
  ISP_DBG("print in1");
  color_correct40_print_cc_params(in1);
  ISP_DBG("print in2");
  color_correct40_print_cc_params(in2);
  if (gain > 0) {
    out->c0 = LINEAR_INTERPOLATION_CC_GAIN(in1->c0, in2->c0, ratio, gain);
    out->c1 = LINEAR_INTERPOLATION_CC_GAIN(in1->c1, in2->c1, ratio, gain);
    out->c2 = LINEAR_INTERPOLATION_CC_GAIN(in1->c2, in2->c2, ratio, gain);
    out->c3 = LINEAR_INTERPOLATION_CC_GAIN(in1->c3, in2->c3, ratio, gain);
    out->c4 = LINEAR_INTERPOLATION_CC_GAIN(in1->c4, in2->c4, ratio, gain);
    out->c5 = LINEAR_INTERPOLATION_CC_GAIN(in1->c5, in2->c5, ratio, gain);
    out->c6 = LINEAR_INTERPOLATION_CC_GAIN(in1->c6, in2->c6, ratio, gain);
    out->c7 = LINEAR_INTERPOLATION_CC_GAIN(in1->c7, in2->c7, ratio, gain);
    out->c8 = LINEAR_INTERPOLATION_CC_GAIN(in1->c8, in2->c8, ratio, gain);
  } else {
    out->c0 = LINEAR_INTERPOLATION(in1->c0, in2->c0, ratio) * (float)(1 << 7);
    out->c1 = LINEAR_INTERPOLATION(in1->c1, in2->c1, ratio) * (float)(1 << 7);
    out->c2 = LINEAR_INTERPOLATION(in1->c2, in2->c2, ratio) * (float)(1 << 7);
    out->c3 = LINEAR_INTERPOLATION(in1->c3, in2->c3, ratio) * (float)(1 << 7);
    out->c4 = LINEAR_INTERPOLATION(in1->c4, in2->c4, ratio) * (float)(1 << 7);
    out->c5 = LINEAR_INTERPOLATION(in1->c5, in2->c5, ratio) * (float)(1 << 7);
    out->c6 = LINEAR_INTERPOLATION(in1->c6, in2->c6, ratio) * (float)(1 << 7);
    out->c7 = LINEAR_INTERPOLATION(in1->c7, in2->c7, ratio) * (float)(1 << 7);
    out->c8 = LINEAR_INTERPOLATION(in1->c8, in2->c8, ratio) * (float)(1 << 7);
  }

  out->q_factor = in1->q_factor;
  out->k0 = LINEAR_INTERPOLATION_BET(in1->k0, in2->k0, ratio);
  out->k1 = LINEAR_INTERPOLATION_BET(in1->k1, in2->k1, ratio);
  out->k2 = LINEAR_INTERPOLATION_BET(in1->k2, in2->k2, ratio);
}

/** color_correct40_interpolate_awb:
 *    @in1: start color correction
 *    @in2: end color correction
 *    @out: interpolated correction
 *    @ratio: interpolation ratio
 *
 *  This function interpolates between two color correction presets
 *
 *  Return: None
 **/
void color_correct40_interpolate_awb(chromatix_color_correction_type *in1,
  chromatix_color_correction_type *in2, chromatix_color_correction_type *out,
  float ratio)
{
  ISP_DBG("ratio %f", ratio);
  ISP_DBG("print in1");
  color_correct40_print_cc_params(in1);
  ISP_DBG("print in2");
  color_correct40_print_cc_params(in2);
  out->c0 = LINEAR_INTERPOLATION(in1->c0, in2->c0, ratio);
  out->c1 = LINEAR_INTERPOLATION(in1->c1, in2->c1, ratio);
  out->c2 = LINEAR_INTERPOLATION(in1->c2, in2->c2, ratio);
  out->c3 = LINEAR_INTERPOLATION(in1->c3, in2->c3, ratio);
  out->c4 = LINEAR_INTERPOLATION(in1->c4, in2->c4, ratio);
  out->c5 = LINEAR_INTERPOLATION(in1->c5, in2->c5, ratio);
  out->c6 = LINEAR_INTERPOLATION(in1->c6, in2->c6, ratio);
  out->c7 = LINEAR_INTERPOLATION(in1->c7, in2->c7, ratio);
  out->c8 = LINEAR_INTERPOLATION(in1->c8, in2->c8, ratio);

  out->q_factor = in1->q_factor;
  out->k0 = LINEAR_INTERPOLATION_BET(in1->k0, in2->k0, ratio);
  out->k1 = LINEAR_INTERPOLATION_BET(in1->k1, in2->k1, ratio);
  out->k2 = LINEAR_INTERPOLATION_BET(in1->k2, in2->k2, ratio);
}

/** color_correct40_streamon:
 *
 *  @module: color correction module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function makes initial config of color correction
 *
 *  Return:   TRUE- Success
 *            FALSE - Parameters size mismatch
 **/
boolean color_correct40_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  color_correct40_t *mod = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_CC_type       *chromatix_CC = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (color_correct40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_CC = &chromatix_ptr->chromatix_VFE.chromatix_CC;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  /* Update streaming mode mask in module private */
  mct_list_traverse(isp_sub_module->l_stream_info,
    isp_sub_module_util_update_streaming_mode, &mod->streaming_mode_mask);

  mod->final_table = chromatix_CC->TL84_color_correction;

  mod->trigger_enable = TRUE;
  mod->skip_trigger = FALSE;
  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->config_pending = TRUE;

  mod->dig_gain = 1;
  mod->aec_ratio.lighting = TRIGGER_NORMAL;
  mod->aec_ratio.ratio = 1.0;
  mod->color_temp = 2850;
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;

  color_correct40_set_hw_params(&mod->RegCmd,
    mod->effects_matrix, &(mod->final_table), mod->dig_gain);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** color_correct40_streamoff:
 *
 *  @module: color correction module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function resets color correction module
 *
 *  Return: boolean
 **/
boolean color_correct40_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  color_correct40_t *mod = NULL;
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (color_correct40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  /* Update streaming mode mask in module private */
  mct_list_traverse(isp_sub_module->l_stream_info,
    isp_sub_module_util_update_streaming_mode, &mod->streaming_mode_mask);

  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  memset(&mod->RegCmd, 0, sizeof(mod->RegCmd));
  memset(&mod->aec_ratio, 0, sizeof(mod->aec_ratio));
  SET_UNITY_MATRIX(mod->effects_matrix, 3);
  mod->dig_gain = 1.0;
  mod->color_temp = 0;
  mod->trigger_enable = 0; /* enable trigger update feature flag from PIX */
  mod->skip_trigger = 0;

  isp_sub_module->manual_ctrls.cc_mode = CAM_COLOR_CORRECTION_FAST;
  isp_sub_module->manual_ctrls.app_ctrl_mode = CAM_CONTROL_USE_SCENE_MODE;
  isp_sub_module->manual_ctrls.wb_mode = CAM_WB_MODE_AUTO;
  isp_sub_module->manual_ctrls.update_cc = FALSE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** color_correct40_set_bestshot:
 *
 *  @module: color correction module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Set BestShot mode
 *
 **/
boolean color_correct40_set_bestshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  float s = 0.0;
  color_correct40_t             *mod = NULL;
  cam_scene_mode_type            bestshot_mode = CAM_SCENE_MODE_OFF;
  mct_event_control_parm_t      *param = NULL;
  chromatix_parms_type          *chromatix_ptr = NULL;
  chromatix_CC_type             *chromatix_CC = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (color_correct40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }
  chromatix_CC = &chromatix_ptr->chromatix_VFE.chromatix_CC;


  /*get best shot mode from mct event data*/
  param = event->u.ctrl_event.control_event_data;
  bestshot_mode =
    *((cam_scene_mode_type *)param->parm_data);

  mod->bestshot_mode = bestshot_mode;

  /*disable trigger when bestshot mode*/
  isp_sub_module->submod_trigger_enable = FALSE;
  ISP_HIGH("bestshot mode %d", bestshot_mode);
  switch (bestshot_mode) {
    case CAM_SCENE_MODE_NIGHT:
      mod->final_table = chromatix_CC->lowlight_color_correction;
      break;

    default:
      isp_sub_module->submod_trigger_enable = TRUE;
      break;
  }

  color_correct40_set_hw_params(&mod->RegCmd, mod->effects_matrix,
    &(mod->final_table), mod->dig_gain);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  ret = color_correct40_store_hw_update(isp_sub_module, mod);
  if (ret == FALSE) {
    ISP_ERR("failed: color_correct40_do_hw_update");
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}


/** color_correct40_enhan_set_effect:
 *
 *  @module: color correction module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Set effect
 *
 **/
boolean color_correct40_set_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret;
  float                    s;
  float                    float_saturation = 0.0;
  int32_t                  *hal_stauration = NULL;
  mct_event_control_t      *ctrl_event = NULL;
  mct_event_control_parm_t *ctrl_parm =NULL;
  color_correct40_t         *mod = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (color_correct40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  if (mod->bestshot_mode != CAM_SCENE_MODE_OFF) {
    ISP_ERR("Best shot enabled, skip set effect");
    return 0;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  ctrl_event = &event->u.ctrl_event;
  ctrl_parm = ctrl_event->control_event_data;

  if(ctrl_parm->type == CAM_INTF_PARM_SATURATION) {
    hal_stauration = (int32_t *)ctrl_parm->parm_data;
    float_saturation = (float)*hal_stauration / 10.0f;

    ISP_DBG("CAM_INTF_PARM_SATURATION!! hal parm %d, satuation = %f",
      *hal_stauration, float_saturation);
    s = 2.0 * float_saturation;
    color_correct40_populate_matrix(mod->effects_matrix, s);
  }

#ifdef ENABLE_CV_LOGGING
  ISP_ERR("print effects matrix");
  PRINT_2D_MATRIX(2, 2, mod->effects_matrix);
#endif

  color_correct40_set_hw_params(&mod->RegCmd,
  mod->effects_matrix, &(mod->final_table), mod->dig_gain);

  ret = color_correct40_store_hw_update(isp_sub_module, mod);
  if (ret == FALSE) {
    ISP_ERR("failed: chroma_enhan40_do_hw_update");
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

END:
  return ret;
}

/** chroma_enhan40_save_aec_param:
 *
 *  @module: color correction module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean color_correct40_save_aec_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                 ret = TRUE;
  stats_update_t          *stats_update = NULL;
  color_correct40_t       *mod = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_CC_type       *chromatix_CC = NULL;
  trigger_ratio_t         trigger_ratio;
  trigger_point_type      *lowlight_trigger_point = NULL;
  trigger_point_type      *outdoor_trigger_point = NULL;
  chromatix_gamma_type    *chromatix_gamma_ptr = NULL;
  ISP_DBG("E");
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  mod = (color_correct40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return TRUE;
  }
  chromatix_CC = &chromatix_ptr->chromatix_VFE.chromatix_CC;
  chromatix_gamma_ptr = &chromatix_ptr->chromatix_VFE.chromatix_gamma;
  outdoor_trigger_point = &(GAMMA_OUTDOOR_TRIGGER(chromatix_gamma_ptr));
  lowlight_trigger_point = &(chromatix_CC->cc_trigger);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  /* Store AEC update in module private*/
  mod->stats_update.aec_update = stats_update->aec_update;

  if (mod->stats_update.aec_update.strobe_enable == TRUE) {
     mod->flash_type = CAMERA_FLASH_STROBE;
  }
  /* Decide the trigger ratio for current lighting condition,
     fill in trigger ratio*/
  ret = isp_sub_module_util_get_aec_ratio_bright_low(
    chromatix_CC->control_cc,
    outdoor_trigger_point, lowlight_trigger_point,
    &stats_update->aec_update, &trigger_ratio, 1);
  if (ret != TRUE) {
    ISP_DBG("get aec ratio error");
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }
  if (!mod->awb_ccm_flag){
   if (F_EQUAL(trigger_ratio.ratio, mod->aec_ratio.ratio)
     && (trigger_ratio.lighting == mod->aec_ratio.lighting)) {
     ISP_DBG("skip aec trigger upadate, "
       "old: ratio = %f, lighting = %d, new ratio = %f, lighting = %d",
       mod->aec_ratio.ratio, mod->aec_ratio.lighting,
       trigger_ratio.ratio, trigger_ratio.lighting);
     PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
     return TRUE;
   }

   mod->aec_ratio = trigger_ratio;

   isp_sub_module->trigger_update_pending = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** color_correct40_set_flash_mode:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to cam_flash_mode_t
 *
 *  Handle set flash mode event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean color_correct40_set_flash_mode(isp_sub_module_t *isp_sub_module,
  void *data)
{
  color_correct40_t         *color_correct = NULL;
  cam_flash_mode_t          *flash_mode = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  color_correct = (color_correct40_t *)isp_sub_module->private_data;
  if (!color_correct) {
    ISP_ERR("failed: color_correct %p", color_correct);
    return FALSE;
  }

  flash_mode = (cam_flash_mode_t *)data;
  if (!flash_mode) {
    ISP_ERR("failed: flash_mode %p", flash_mode);
    return FALSE;
  }

  if (color_correct->cur_flash_mode != *flash_mode)
    isp_sub_module->trigger_update_pending = TRUE;

  ISP_DBG("<flash_dbg> get new flash mode = %d", *flash_mode);
  color_correct->cur_flash_mode = *flash_mode;

  return TRUE;
}

/** color_correct40_trigger_update_aec:
 *
 * @mod: color correction module
 * @tblIn: input CC table
 * @tblOut: output CC table
 * @chromatix_ptr: chromatix pointer
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean color_correct40_trigger_update_aec(color_correct40_t *mod,
  chromatix_color_correction_type *tblIn, chromatix_color_correction_type *tblOut,
  chromatix_parms_type *chromatix_ptr)
{
  boolean                 ret = TRUE;
  chromatix_CC_type       *chromatix_CC = NULL;
  chromatix_color_correction_type *tblTmp = NULL;

  chromatix_CC = &chromatix_ptr->chromatix_VFE.chromatix_CC;

  /* aec_ratio_type.ratio is the ratio to tbl_CCT */
  switch (mod->aec_ratio.lighting) {
  case TRIGGER_LOWLIGHT:
    /* interpolate between normal CCT tbl and lowlight tbl */
    tblTmp = &chromatix_CC->lowlight_color_correction;
    break;

  case TRIGGER_OUTDOOR:
    /* interpolate between normal CCT tbl and lowlight tbl */
    tblTmp = &chromatix_CC->outdoor_color_correction;
    break;

  case TRIGGER_NORMAL:
    tblTmp = tblIn;
    break;

  default:
    ISP_ERR("invalid lighting type, lighting type %d\n",
      mod->aec_ratio.lighting);
    ret = FALSE;
    break;
  }

  if (TRUE == ret) {
    color_correct40_interpolate(tblIn,
      tblTmp, tblOut, mod->aec_ratio.ratio, mod->dig_gain);
  }
  ISP_DBG("print out");
  color_correct40_print_cc_params(tblOut);

  return ret;
}

/** color_correct40_save_awb_params:
 *
 *  @module: color correction module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Handle AWB update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean color_correct40_save_awb_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  float              ratio = 0.0;
  stats_update_t     *stats_update = NULL;
  color_correct40_t   *mod = NULL;

  ISP_DBG("E");
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  mod = (color_correct40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  mod->awb_ccm_flag = stats_update->awb_update.ccm_update.awb_ccm_enable;
  mod->awb_ccm_update_flag = stats_update->awb_update.ccm_update.ccm_update_flag;

  /*check CCM first, if CCM then no more interpolate*/
  if (!mod->awb_ccm_flag) {
    if (mod->color_temp == stats_update->awb_update.color_temp) {
      ISP_DBG("skip awb trigger update, color temp old = %d, new = %d",
      mod->color_temp, stats_update->awb_update.color_temp);
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return TRUE;
    }
  }
  /* need to update since the color temp is used on meta reporting */
  mod->color_temp = stats_update->awb_update.color_temp;

  if (mod->awb_ccm_flag != 0) {
    mod->awb_ccm_update = stats_update->awb_update.ccm_update;
    if (mod->awb_ccm_update_flag!= 0) {
      isp_sub_module->trigger_update_pending = TRUE;
    }

    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }
  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** color_correct40_trigger_update_awb:
 *
 * @mod: color_correct module
 * @tbl_out: output CCT CC table
 * @chromatix_ptr: chromatix pointer
 *
 * Handle AWB update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean color_correct40_trigger_update_awb(color_correct40_t *mod,
  chromatix_color_correction_type *tbl_out, chromatix_parms_type *chromatix_ptr)
{
  float ratio = 0.0;
  cct_trigger_info   trigger_info;
  awb_cct_type       cct_type;
  float color_temp = mod->color_temp;
  chromatix_CC_type *chromatix_CC = &chromatix_ptr->chromatix_VFE.chromatix_CC;
  chromatix_CCT_trigger_type   CC_Daylight_trigger;
  chromatix_CCT_trigger_type   CC_D50_trigger;
  chromatix_CCT_trigger_type   CC_A_trigger;
  chromatix_CCT_trigger_type   CC_H_trigger;
  float gain = 0.0f;

  memset(&trigger_info, 0, sizeof(trigger_info));
  /* fill in trigger_info by saved color temp*/
  if (mod->color_temp) {
    MIRED(mod->color_temp, trigger_info.mired_color_temp);
  }

  CC_Daylight_trigger = chromatix_CC->CC_Daylight_trigger;
  CC_D50_trigger = chromatix_CC->CC_D50_trigger;
  CC_A_trigger = chromatix_CC->CC_A_trigger;
  CC_H_trigger = chromatix_CC->CC_H_trigger;

  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_d65, CC_Daylight_trigger);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_d50, CC_D50_trigger);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_A, CC_A_trigger);
  CALC_CCT_TRIGGER_MIRED(trigger_info.trigger_H, CC_H_trigger);

  /* get cct type by color temp*/
  cct_type =
    isp_sub_module_util_get_awb_ext_h_cct_type(&trigger_info, chromatix_ptr);

  switch (cct_type) {
    case AWB_CCT_TYPE_H:
     *tbl_out = chromatix_CC->H_color_correction;
      break;

    case AWB_CCT_TYPE_A_H:
      ratio = GET_INTERPOLATION_RATIO(1.0f / color_temp,
                 1.0f / CC_H_trigger.CCT_start,
                 1.0f / CC_H_trigger.CCT_end);
      color_correct40_interpolate_awb(&chromatix_CC->A_color_correction,
        &chromatix_CC->H_color_correction, tbl_out, ratio);
      break;

    case AWB_CCT_TYPE_A:
      *tbl_out = chromatix_CC->A_color_correction;
      break;

    case AWB_CCT_TYPE_TL84_A:
      ratio = GET_INTERPOLATION_RATIO(1.0f / color_temp,
                 1.0f/CC_A_trigger.CCT_start,
                 1.0f/CC_A_trigger.CCT_end);
      ISP_DBG("AWB_CCT_TYPE_TL84_A ratio %f", ratio);
      color_correct40_interpolate_awb(&chromatix_CC->TL84_color_correction,
        &chromatix_CC->A_color_correction, tbl_out, ratio);
      break;

    case AWB_CCT_TYPE_D65_TL84:
      ratio = GET_INTERPOLATION_RATIO(1.0f / color_temp,
                1.0f/CC_Daylight_trigger.CCT_end,
                1.0f/CC_Daylight_trigger.CCT_start);
      ISP_DBG("AWB_CCT_TYPE_D65_TL84 ratio %f", ratio);
      color_correct40_interpolate_awb(&chromatix_CC->D65_color_correction,
        &chromatix_CC->TL84_color_correction, tbl_out, ratio);
      break;

    case AWB_CCT_TYPE_D50_D65:
      ratio = GET_INTERPOLATION_RATIO(1.0f / color_temp,
                1.0f/CC_Daylight_trigger.CCT_end,
                1.0f/CC_Daylight_trigger.CCT_start);
      ISP_DBG("AWB_CCT_TYPE_D65_D50 ratio %f", ratio);
      color_correct40_interpolate_awb(&chromatix_CC->D65_color_correction,
        &chromatix_CC->D50_color_correction, tbl_out, ratio);
      break;

    case AWB_CCT_TYPE_D50_TL84:
      ratio = GET_INTERPOLATION_RATIO(1.0f / color_temp,
                1.0f/CC_D50_trigger.CCT_end,
                1.0f/CC_D50_trigger.CCT_start);
      ISP_DBG("AWB_CCT_TYPE_D50_TL84 ratio %f", ratio);
      color_correct40_interpolate_awb(&chromatix_CC->D50_color_correction,
        &chromatix_CC->TL84_color_correction, tbl_out, ratio);
      break;

    case AWB_CCT_TYPE_D65:
      *tbl_out = chromatix_CC->D65_color_correction;
      break;

    case AWB_CCT_TYPE_D50:
        ISP_DBG("AWB_CCT_TYPE_D50 ratio");
        *tbl_out = chromatix_CC->D50_color_correction;
        break;

    default:
    case AWB_CCT_TYPE_TL84:
      *tbl_out = chromatix_CC->TL84_color_correction;
      break;
  }
  ISP_DBG("print out");
  color_correct40_print_cc_params(tbl_out);
  return TRUE;
}

/** util_color_correct_calc_flash_trigger:
 *    @mod: CC module instance
 *    @tblCCT: color correction input coefficients
 *    @tblOut: color correction input coefficients
 *    @in_params: input paramters
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function interpolates te CC coefficients according flash usage
 *  Return: None
 **/
static void color_correct40_trigger_update_flash(
  color_correct40_t *mod, chromatix_color_correction_type *tblCCT,
  chromatix_color_correction_type *tblOut, chromatix_parms_type *chromatix_ptr)
{
  float ratio = 0;
  float p_ratio = 0;
  float flash_start, flash_end;
  chromatix_color_correction_type *tblFlash = NULL;
  flash_sensitivity_t flash_params = mod->stats_update.aec_update.flash_sensitivity;
  chromatix_CC_type *chromatix_CC = &chromatix_ptr->chromatix_VFE.chromatix_CC;

  ISP_DBG("off %f, high %f", mod->stats_update.aec_update.flash_sensitivity.off,
           mod->stats_update.aec_update.flash_sensitivity.high);
  if ((int)mod->cur_flash_mode == CAMERA_FLASH_STROBE) {
    tblFlash = &(chromatix_CC->STROBE_color_correction);
    flash_start = chromatix_CC->CC_strobe_start;
    flash_end = chromatix_CC->CC_strobe_end;
  } else {
    tblFlash = &(chromatix_CC->LED_color_correction);
    flash_start = chromatix_CC->CC_LED_start;
    flash_end = chromatix_CC->CC_LED_end;
  }
  if (mod->cur_flash_mode == CAM_FLASH_MODE_TORCH) {
     if (flash_params.high != 0)
       ratio = flash_params.off/flash_params.high;
  } else if ((mod->cur_flash_mode == CAM_FLASH_MODE_ON) ||
     (mod->cur_flash_mode == CAM_FLASH_MODE_SINGLE)) {
     if (flash_params.high != 0)
       ratio = flash_params.off/flash_params.high;
  } else {
    /* assume flash off. To be changed when AUTO mode is added */
    ratio = flash_start;
  }
  ISP_DBG("flash_start %5.2f flash_end %5.2f ratio %f",flash_start,
    flash_end, ratio);

  if (ratio < flash_start) {
     ratio = flash_start;
  } else if (ratio > flash_end) {
      ratio = flash_end;
  }
  ISP_ERR("ratio %f", ratio);

  /*if (ratio >= flash_end)
    *tblOut = *tblFlash;
  else if (ratio <= flash_start)
    *tblOut = *tblCCT;
  else{*/
    ISP_DBG("C[0-8] = %f, %f, %f, %f, %f, %f, %f, %f, %f\n", tblFlash->c0,
              tblFlash->c1, tblFlash->c2, tblFlash->c3, tblFlash->c4, tblFlash->c5, tblFlash->c6,
              tblFlash->c7, tblFlash->c8);
    p_ratio = GET_INTERPOLATION_RATIO(ratio, flash_start, flash_end);
    ISP_ERR("p ratio %f, dig gain %f", p_ratio, mod->dig_gain);
    color_correct40_interpolate(tblCCT, tblFlash, tblOut, p_ratio,mod->dig_gain);

  //}
  return;
} /* color_correct40_trigger_update_flash */

/** color_correct40_manual_update:
 *
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function uses the manual CC matrix values and update
 *  the HW
 *
 *  Return:   TRUE - Success
 *            FALSE - Parameters size mismatch
 **/
boolean color_correct40_manual_update(
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  color_correct40_t       *color_correct_mod = NULL;
  isp_sub_module_output_t *sub_module_output = NULL;
  isp_private_event_t     *private_event = NULL;

  if (!isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }

 /*get chroa enhance hw update list*/
  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  sub_module_output = (isp_sub_module_output_t *)private_event->data;
  if (!sub_module_output) {
    ISP_ERR("failed: output %p", sub_module_output);
    return FALSE;
  }


  /* get chroma enhance mod private*/
  color_correct_mod = (color_correct40_t *)isp_sub_module->private_data;
  if (!color_correct_mod) {
    ISP_ERR("failed: mod %p", color_correct_mod);
    return FALSE;
  }

  color_correct_mod->dig_gain = 1.0;
  SET_UNITY_MATRIX(color_correct_mod->effects_matrix, 3);

  color_correct_mod->final_table.c0 =
    isp_sub_module->manual_ctrls.cc_transform_matrix[0][0];
  color_correct_mod->final_table.c1 =
    isp_sub_module->manual_ctrls.cc_transform_matrix[0][1];
  color_correct_mod->final_table.c2 =
    isp_sub_module->manual_ctrls.cc_transform_matrix[0][2];
  color_correct_mod->final_table.c3 =
    isp_sub_module->manual_ctrls.cc_transform_matrix[1][0];
  color_correct_mod->final_table.c4 =
    isp_sub_module->manual_ctrls.cc_transform_matrix[1][1];
  color_correct_mod->final_table.c5 =
    isp_sub_module->manual_ctrls.cc_transform_matrix[1][2];
  color_correct_mod->final_table.c6 =
    isp_sub_module->manual_ctrls.cc_transform_matrix[2][0];
  color_correct_mod->final_table.c7 =
    isp_sub_module->manual_ctrls.cc_transform_matrix[2][1];
  color_correct_mod->final_table.c8 =
    isp_sub_module->manual_ctrls.cc_transform_matrix[2][2];

  color_correct_mod->final_table.k0 = 0;
  color_correct_mod->final_table.k1 = 0;
  color_correct_mod->final_table.k2 = 0;
  color_correct_mod->final_table.q_factor = 0;

  color_correct40_set_hw_params(&color_correct_mod->RegCmd,
    color_correct_mod->effects_matrix, &(color_correct_mod->final_table),
    color_correct_mod->dig_gain);

  ret = color_correct40_store_hw_update(isp_sub_module, color_correct_mod);
  if (ret == FALSE) {
    ISP_ERR("failed: color correct40_do_hw_update");
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
    sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
  }

  return ret;
}


/** color_correct40_trigger_update:
 *
 *  @module: color correction module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function checks and initiates triger update of module
 *
 *  Return:   TRUE - Success
 *            FALSE - Parameters size mismatch
 **/
boolean color_correct40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  isp_private_event_t     *private_event = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_CC_type       *chromatix_CC = NULL;
  color_correct40_t       *color_correct_mod = NULL;
  chromatix_color_correction_type      tblCCT;
  float                    unity_effects_matrix[3][3];
  isp_sub_module_output_t *sub_module_output = NULL;
  cam_rational_type_t    (*transform_matrix)[3] =  NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  /*get chroa enhance hw update list*/
  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  sub_module_output = (isp_sub_module_output_t *)private_event->data;
  if (!sub_module_output) {
    ISP_ERR("failed: output %p", sub_module_output);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  /* get chroma enhance mod private*/
  color_correct_mod = (color_correct40_t *)isp_sub_module->private_data;
  if (!color_correct_mod) {
    ISP_ERR("failed: mod %p", color_correct_mod);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (isp_sub_module->manual_ctrls.manual_update &&
      isp_sub_module->chromatix_module_enable &&
      !isp_sub_module->config_pending) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;

    if (isp_sub_module->manual_ctrls.cc_mode == CAM_COLOR_CORRECTION_TRANSFORM_MATRIX &&
      ((isp_sub_module->manual_ctrls.wb_mode == CAM_WB_MODE_OFF &&
      isp_sub_module->manual_ctrls.app_ctrl_mode == CAM_CONTROL_AUTO) ||
      isp_sub_module->manual_ctrls.app_ctrl_mode == CAM_CONTROL_OFF)) {
      isp_sub_module->submod_trigger_enable = FALSE;
      if (isp_sub_module->manual_ctrls.update_cc) {
        isp_sub_module->manual_ctrls.update_cc = FALSE;
        ret = color_correct40_manual_update(isp_sub_module, event);
        goto FILL_METADATA;
      }

    } else {
      if (isp_sub_module->manual_ctrls.cc_mode !=
          CAM_COLOR_CORRECTION_TRANSFORM_MATRIX){
        isp_sub_module->submod_trigger_enable = TRUE;
      }

      if (isp_sub_module->manual_ctrls.update_cc) {
        ISP_ERR("Manual CC matrix not applied");
        isp_sub_module->manual_ctrls.update_cc = FALSE;
        goto FILL_METADATA;
      }
    }
  }

  if(isp_sub_module->config_pending)
    isp_sub_module->config_pending = FALSE;

  /* get chromatix pointer*/
  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  chromatix_CC =
    &chromatix_ptr->chromatix_VFE.chromatix_CC;

  if (!isp_sub_module->submod_enable) {
      ISP_DBG("Color correction module is disabled");
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return ret;
  }
  if (isp_sub_module->trigger_update_pending == TRUE &&
      isp_sub_module->submod_trigger_enable == TRUE) {
     /* If the ccm flag is set, use the ccm matrix inside AWB Update and
      *  skip all interpolation including awb, aec, effects and write to hw */
    if (color_correct_mod->awb_ccm_flag != 0) {
      color_correct_mod->dig_gain = 1.0;
      SET_CC_TBL_FROM_AWB_MATRIX((&color_correct_mod->final_table),
        color_correct_mod->awb_ccm_update.ccm);
      color_correct_mod->final_table.k0 =
        color_correct_mod->awb_ccm_update.ccm_offset[0];
      color_correct_mod->final_table.k1 =
        color_correct_mod->awb_ccm_update.ccm_offset[1];
      color_correct_mod->final_table.k2 =
        color_correct_mod->awb_ccm_update.ccm_offset[2];
    } else {
      /* do awb trigger update*/
      ret = color_correct40_trigger_update_awb(color_correct_mod,
        &tblCCT, chromatix_ptr);

      /* do aec trigger update/ flash trigger update*/
      if (color_correct_mod->cur_flash_mode != CAM_FLASH_MODE_OFF) {
        color_correct40_trigger_update_flash(color_correct_mod,
          &tblCCT, &color_correct_mod->final_table, chromatix_ptr);
      } else {
        ret = color_correct40_trigger_update_aec(color_correct_mod,
          &tblCCT, &color_correct_mod->final_table, chromatix_ptr);
       }
    }

    /* If Bestshot enabled, use all 1 unity effect matrix
       if hard 3A ccm set, means algo does not allow any modification*/
    if ((color_correct_mod->bestshot_mode != CAM_SCENE_MODE_OFF) ||
        (color_correct_mod->awb_ccm_update.hard_awb_ccm_flag == TRUE)) {
      SET_UNITY_MATRIX(unity_effects_matrix, 3);
      color_correct40_set_hw_params(&color_correct_mod->RegCmd,
        unity_effects_matrix, &(color_correct_mod->final_table),
        color_correct_mod->dig_gain);
    } else {
      color_correct40_set_hw_params(&color_correct_mod->RegCmd,
        color_correct_mod->effects_matrix, &(color_correct_mod->final_table),
        color_correct_mod->dig_gain);
    }

    ret = color_correct40_store_hw_update(isp_sub_module, color_correct_mod);
    if (ret == FALSE) {
      ISP_ERR("failed: color correct40_do_hw_update");
    }
    isp_sub_module->trigger_update_pending = FALSE;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
    sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
  }

FILL_METADATA:

  if (sub_module_output->meta_dump_params &&
    sub_module_output->metadata_dump_enable == 1) {
    sub_module_output->meta_dump_params->frame_meta.addlinfo.CCM_enabled =
      color_correct_mod->awb_ccm_flag;
    sub_module_output->meta_dump_params->frame_meta.color_temp =
      color_correct_mod->color_temp;
  }
  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->bestshot_mode=
      color_correct_mod->bestshot_mode;
    if (isp_sub_module->vfe_diag_enable) {
      ret = color_correct40_fill_vfe_diag_data(color_correct_mod,
        isp_sub_module, sub_module_output);
      if (ret == FALSE) {
        ISP_ERR("failed: cannot fill vfe diag data");
      }
    }

    sub_module_output->frame_meta->app_ctrl_mode =
      isp_sub_module->manual_ctrls.app_ctrl_mode;
    sub_module_output->frame_meta->cc_mode =
      isp_sub_module->manual_ctrls.cc_mode;
    transform_matrix =
      sub_module_output->frame_meta->color_correction_matrix.transform_matrix;
    transform_matrix[0][0].numerator =
      FLOAT_TO_Q(7, color_correct_mod->final_table.c0);
    transform_matrix[0][0].denominator = FLOAT_TO_Q(7, 1);
    transform_matrix[0][1].numerator =
      FLOAT_TO_Q(7, color_correct_mod->final_table.c1);
    transform_matrix[0][1].denominator = FLOAT_TO_Q(7, 1);
    transform_matrix[0][2].numerator =
      FLOAT_TO_Q(7, color_correct_mod->final_table.c2);
    transform_matrix[0][2].denominator = FLOAT_TO_Q(7, 1);

    transform_matrix[1][0].numerator =
      FLOAT_TO_Q(7, color_correct_mod->final_table.c3);
    transform_matrix[1][0].denominator = FLOAT_TO_Q(7, 1);
    transform_matrix[1][1].numerator =
      FLOAT_TO_Q(7, color_correct_mod->final_table.c4);
    transform_matrix[1][1].denominator = FLOAT_TO_Q(7, 1);
    transform_matrix[1][2].numerator =
      FLOAT_TO_Q(7, color_correct_mod->final_table.c5);
    transform_matrix[1][2].denominator = FLOAT_TO_Q(7, 1);

    transform_matrix[2][0].numerator =
      FLOAT_TO_Q(7, color_correct_mod->final_table.c6);
    transform_matrix[2][0].denominator = FLOAT_TO_Q(7, 1);
    transform_matrix[2][1].numerator =
      FLOAT_TO_Q(7, color_correct_mod->final_table.c7);
    transform_matrix[2][1].denominator = FLOAT_TO_Q(7, 1);
    transform_matrix[2][2].numerator =
      FLOAT_TO_Q(7, color_correct_mod->final_table.c8);
    transform_matrix[2][2].denominator = FLOAT_TO_Q(7, 1);
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** color_correct40_update_streaming_mode_mask:
 *
 *  @module: color correction module instance
 *  @isp_sub_module: sub module handle
 *  @streaming_mode_mask: streaming mode mask
 *
 *  update stream mask
 *  Return none
 **/
void color_correct40_update_streaming_mode_mask(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, uint32_t streaming_mode_mask)
{
  color_correct40_t *color_correct = NULL;
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return;
  }

  color_correct = (color_correct40_t *)isp_sub_module->private_data;
  if (!color_correct) {
    ISP_ERR("failed: color correction %p", color_correct);
    return;
  }
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  color_correct->streaming_mode_mask = streaming_mode_mask;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
}

/** color_correct40_ez_isp_update
 *
 *  @color correction_module: color correction module handle
 *  @color correctionDiag: color correction Diag handle
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void color_correct40_ez_isp_update(isp_sub_module_t *isp_sub_module,
  color_correct40_t *color_correct_mod, colorcorrection_t *color_correct_Diag)
{
  int index;
  ISP_ColorCorrectionCfgCmdType *colorcorrCfg = NULL;

  if (!isp_sub_module || !color_correct_mod || !color_correct_Diag) {
    ISP_ERR("failed: %p %p %p", isp_sub_module,
      color_correct_mod, color_correct_Diag);
    return;
  }

  colorcorrCfg = &color_correct_mod->RegCmd;

  color_correct_Diag->coef_qfactor = colorcorrCfg->coefQFactor;
  color_correct_Diag->coef_rtor    = colorcorrCfg->C0;
  color_correct_Diag->coef_gtor    = colorcorrCfg->C1;
  color_correct_Diag->coef_btor    = colorcorrCfg->C2;
  color_correct_Diag->coef_rtog    = colorcorrCfg->C3;
  color_correct_Diag->coef_gtog    = colorcorrCfg->C4;
  color_correct_Diag->coef_btog    = colorcorrCfg->C5;
  color_correct_Diag->coef_rtob    = colorcorrCfg->C6;
  color_correct_Diag->coef_gtob    = colorcorrCfg->C7;
  color_correct_Diag->coef_btob    = colorcorrCfg->C8;
  color_correct_Diag->roffset      = colorcorrCfg->K0;
  color_correct_Diag->boffset      = colorcorrCfg->K1;
  color_correct_Diag->goffset      = colorcorrCfg->K2;
}

/** color_correct40_get_vfe_diag_info_user
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Get VFE diag info
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean color_correct40_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  isp_private_event_t *private_event = NULL;
  color_correct40_t   *color_correct_mod = NULL;
  vfe_diagnostics_t   *vfe_diag = NULL;
  colorcorrection_t   *color_correct_Diag = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p data %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  color_correct_mod = (color_correct40_t *)isp_sub_module->private_data;
  if (!color_correct_mod) {
    ISP_ERR("failed: chroma_enhan_mod %p", color_correct_mod);
    return FALSE;
  }

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  vfe_diag = (vfe_diagnostics_t *)private_event->data;
  if (!vfe_diag) {
    ISP_ERR("failed: vfe_diag %p", vfe_diag);
    return FALSE;
  }

  if (sizeof(vfe_diagnostics_t) != private_event->data_size) {
    ISP_ERR("failed: out_param_size mismatch, param_id = %d",
      private_event->type);
    return FALSE;
  }
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  color_correct_Diag = &vfe_diag->prev_colorcorr;
  color_correct40_ez_isp_update(isp_sub_module,
    color_correct_mod, color_correct_Diag);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** color_correct40_fill_vfe_diag_data:
   *
   *  @mod: color correction  module instance
   *
   *  This function fills vfe diagnostics information
   *
   *  Return: TRUE success
   **/
static boolean color_correct40_fill_vfe_diag_data(color_correct40_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean              ret = TRUE;
  demosaic3_t         *demosaicDiag = NULL;
  vfe_diagnostics_t   *vfe_diag = NULL;
  colorcorrection_t   *color_correct_Diag = NULL;

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    color_correct_Diag = &vfe_diag->prev_colorcorr;
    color_correct40_ez_isp_update(isp_sub_module,
      mod, color_correct_Diag);

  }
  return ret;
}

/** color_correct40_init:
 *
 *  @module: color correct module handle
 *  @isp_sub_module: submodule data
 *
 *  This function instantiates a color correction module
 *
 *  Return: boolean
 **/
boolean color_correct40_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  color_correct40_t *mod = NULL;

  ISP_HIGH("E");
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (color_correct40_t *)CAM_MALLOC(sizeof(color_correct40_t));
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  memset(mod, 0, sizeof(*mod));
  memset(&mod->RegCmd, 0, sizeof(mod->RegCmd));
  memset(&mod->aec_ratio, 0, sizeof(mod->aec_ratio));

  /* enable trigger update feature flag from PIX */
  mod->trigger_enable = 1;
  mod->skip_trigger = 0;
  mod->dig_gain = 1;
  mod->aec_ratio.lighting = TRIGGER_NORMAL;
  mod->aec_ratio.ratio = 1.0;
  mod->color_temp = 2850;
  mod->flash_type = CAMERA_FLASH_LED;
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  mod->color_temp = DEFAULT_COLOR_TEMP;
  mod->bestshot_mode = CAM_SCENE_MODE_OFF;

  /* set old cfg to invalid value to trigger the first trigger update */
  SET_UNITY_MATRIX(mod->effects_matrix, 3);

  isp_sub_module->private_data = (void *)mod;
  isp_sub_module->manual_ctrls.cc_mode = CAM_COLOR_CORRECTION_FAST;
  isp_sub_module->manual_ctrls.app_ctrl_mode = CAM_CONTROL_USE_SCENE_MODE;
  isp_sub_module->manual_ctrls.wb_mode = CAM_WB_MODE_AUTO;
  isp_sub_module->manual_ctrls.update_cc = FALSE;

  return TRUE;
}

/** color_correct40_destroy:
 *
 *  @module: color correct module handle
 *  @isp_sub_module: submodule data
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void color_correct40_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  color_correct40_t *mod = NULL;

  ISP_HIGH("E");

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
}
