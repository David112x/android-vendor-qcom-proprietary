/* demosaic40.c
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>

/* mctl headers */
#include "eztune_vfe_diagnostics.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_DEMOSAIC, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_DEMOSAIC, fmt, ##args)

#define DEMOSAIC_RCCB_GAIN(x) FLOAT_TO_Q(7, (x))
/* isp headers */
#include "module_demosaic40.h"
#include "demosaic40.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"


static uint16_t wInterpDefault[ISP_DEMOSAIC40_CLASSIFIER_CNT] =
    { 137, 91, 91, 1023, 922, 93, 195, 99, 64, 319, 197, 88, 84, 109, 151, 98,
       66, 76 };
static uint16_t bInterpDefault[ISP_DEMOSAIC40_CLASSIFIER_CNT] =
    { 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1 };
static uint16_t lInterpDefault[ISP_DEMOSAIC40_CLASSIFIER_CNT] =
    { 0, 0, 1, 2, 2, 3, 9, 9, 9, 4, 4, 5, 6, 7, 8, 8, 10, 10 };
static int16_t tInterpDefault[ISP_DEMOSAIC40_CLASSIFIER_CNT] =
    { 2, 1, 0, 0, -1, 2, 0, -1, 1, 0, -1, 2, 0, 2, 2, 1, 0, 100 };


/** demosaic40_debug:
 *
 *  @cmd: configuration command
 *  @update: is it update flag
 *
 *  This function dumps demosaic configuration
 *
 *  Return: None
 **/
static void demosaic40_debug(void *cmd, uint8_t update)
{
  ISP_DBG("VFE_Demosaic40 config = %d or update = %d", !update, update);

  ISP_Demosaic40ConfigCmdType* pcmd = (ISP_Demosaic40ConfigCmdType *)cmd;

  ISP_DBG("ISP_Demosaic40CmdType rgWbGain %d", pcmd->rgWbGain);
  ISP_DBG("ISP_Demosaic40CmdType bgWbGain %d", pcmd->bgWbGain);
  ISP_DBG("ISP_Demosaic40CmdType grWbGain %d", pcmd->grWbGain);
  ISP_DBG("ISP_Demosaic40CmdType gbWbGain %d", pcmd->gbWbGain);
  ISP_DBG("ISP_Demosaic40CmdType bl %d", pcmd->bl);
  ISP_DBG("ISP_Demosaic40CmdType bu %d", pcmd->bu);
  ISP_DBG("ISP_Demosaic40CmdType dblu %d", pcmd->dblu);
  ISP_DBG("ISP_Demosaic40CmdType a %d", pcmd->a);

}/*demosaic40_debug*/

/** demosaic40_set_chromatix_params:
 *
 *  @demosaic: demosaic module instance
 *  @pix_setting: PIX settings
 *  @is_burst: is it in burst mode - obsolete
 *
 *  This function prepares demosaic configuration based on
 *  chromatix parameters
 *
 *  Return: None
 **/
static void demosaic40_set_chromatix_params(demosaic40_t *demosaic,
  chromatix_parms_type *chromatix_ptr, uint8_t is_burst)
{
  int                            i = 0, temp = 0;
  demosaic3_type                 *classifier = NULL;
  demosaic3_Reserve              *reserve = NULL;
  demosaic3_tune                 *data = NULL;
  chromatix_demosaic_type        *chromatix_demosaic;
  ISP_Demosaic40ConfigCmdType    *p_cmd = NULL;
  ISP_Demosaic40MixConfigCmdType *mix_cmd = NULL;

  float bL = 0.0;
  float aG = 0.0;

  if (!demosaic || !chromatix_ptr) {
    ISP_ERR("failed: %p %p", demosaic, chromatix_ptr);
    return;
  }
  p_cmd = &demosaic->reg_cmd;
  mix_cmd = &demosaic->mix_reg_cmd;

  chromatix_demosaic = &chromatix_ptr->chromatix_VFE.chromatix_demosaic;
  classifier = &(chromatix_demosaic->demosaic3_data);
  reserve = &(classifier->reserve);
  data = &(classifier->data[1]);
  bL = data->bL;
  aG = data->aG;
  /* default values */
  mix_cmd->abccLutBankSel = 0;
  mix_cmd->pipeFlushCount = 0;
  mix_cmd->pipeFlushOvd = 0;
  mix_cmd->flushHaltOvd = 0;
  mix_cmd->cositedRgbEnable = FALSE;

  /* Classifier */
  for (i=0 ; i<ISP_DEMOSAIC40_CLASSIFIER_CNT ; ++i) {
    p_cmd->interpClassifier[i].w_n = FLOAT_TO_Q(10,reserve->wk[i]);
    p_cmd->interpClassifier[i].t_n = reserve->Tk[i];
    p_cmd->interpClassifier[i].l_n = reserve->lk[i];
    p_cmd->interpClassifier[i].b_n = reserve->bk[i];
  }

  temp = FLOAT_TO_Q(8, bL);
  p_cmd->bl = MIN(MAX(0, temp), 118);
  temp = FLOAT_TO_Q(8, (1.0 - bL));
  p_cmd->bu = MIN(MAX(138, temp), 255);
  p_cmd->gain = DEMOSAIC_RCCB_GAIN(1);
  temp = FLOAT_TO_Q(5, (1.0/(1.0 - 2 * bL)));
  p_cmd->dblu = MIN(MAX(0, temp), 511);
  temp = FLOAT_TO_Q(6, aG);
  p_cmd->a = MIN(MAX(0, temp), 63);
  p_cmd->gain_offset = 0;
} /* demosaic40_set_cfg_params */

/** demosaic40_calc_interpolation_weight:
 *
 *  @value: value
 *  @start: start
 *  @end: end
 *
 *  Interpolate based on start and end
 *
 *  Return value after interpolation
 **/
static float demosaic40_calc_interpolation_weight(float value,
  float start, float end)
{
  /* return value is a ratio to the start point,
    "start" point is always the smaller gain/lux one.
    thus,
    "start" could be lowlight trigger start, and bright light trigger end*/
  if (start != end) {
    if (value  <= start)
      return 0.0;
    else if (value  >= end)
      return 1.0;
    else
      return(value  - start) / (end - start);
  } else {
    ISP_HIGH("Trigger Warning: same value %f\n", start);
    return 0.0;
  }
} /* demosaic40_calc_interpolation_weight */

/** demosaic40_get_aec_ratio_lowlight
 *
 *  @tunning_type: tuning type (gain based or lux index based)
 *  @trigger_ptr: trigger point type
 *  @aec_out: aec out handle
 *
 *   Get trigger ratio based on lowlight trigger.
 *   Please note that ratio is the weight of the normal light.
 *    LOW           Mix            Normal
 *    ----------|----------------|----------
 *        low_end(ex: 400)    low_start(ex: 300)
 *
 *  Return aec ratio
 **/
static float demosaic40_get_aec_ratio_lowlight(unsigned char tunning_type,
  void *trigger_ptr, aec_update_t* aec_out)
{
  float               normal_light_ratio = 0.0, sensor_gain;
  float               ratio_to_low_start = 0.0;
  tuning_control_type tunning = (tuning_control_type)tunning_type;
  trigger_point_type *trigger = (trigger_point_type *)trigger_ptr;

  if (!trigger_ptr || !aec_out) {
    ISP_ERR("failed: %p %p", trigger_ptr, aec_out);
    return 0.0f;
  }

  switch (tunning) {
  /* 0 is Lux Index based */
  case 0: {
    ratio_to_low_start = demosaic40_calc_interpolation_weight(
      aec_out->lux_idx, trigger->lux_index_start, trigger->lux_index_end);
    }
      break;
  /* 1 is Gain Based */
  case 1: {
    sensor_gain = aec_out->sensor_gain;
    ratio_to_low_start = demosaic40_calc_interpolation_weight(sensor_gain,
      trigger->gain_start, trigger->gain_end);
  }
    break;

  default: {
    ISP_ERR("get_trigger_ratio: tunning type %d is not supported.\n",
      tunning);
  }
    break;
  }

  /*ratio_to_low_start is the sitance to low start,
    the smaller distance to low start,
    the higher ratio applied normal light*/
  normal_light_ratio = 1 - ratio_to_low_start;

  if (normal_light_ratio < 0) {
    normal_light_ratio = 0;
  } else if (normal_light_ratio > 1.0) {
    normal_light_ratio = 1.0;
  }

  return normal_light_ratio;
} /* demosaic40_get_aec_ratio_lowlight */

/** demosaic40_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @demosaic_mod: demosaic module instance
 *
 *  This function checks and sends configuration update to kernel
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean demosaic40_store_hw_update(isp_sub_module_t *isp_sub_module,
  demosaic40_t *demosaic_mod)
{
  boolean                      ret = TRUE;
  int                          i, rc = 0;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  ISP_Demosaic40ConfigCmdType *reg_cmd = NULL;

  if (!isp_sub_module || !demosaic_mod) {
    ISP_ERR("failed: %p %p", isp_sub_module, demosaic_mod);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  cfg_cmd = &hw_update->cfg_cmd;

  /*regular hw write for Demosaic*/
  demosaic40_debug(&demosaic_mod->reg_cmd,
    demosaic_mod->classifier_cfg_done);

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd) * 3);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_MALLOC;
  }
  memset(reg_cfg_cmd, 0, (sizeof(struct msm_vfe_reg_cfg_cmd) * 3));

  reg_cmd = (ISP_Demosaic40ConfigCmdType *)CAM_MALLOC(sizeof(*reg_cmd));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CMD_MALLOC;
  }
  memset(reg_cmd, 0, sizeof(*reg_cmd));
  *reg_cmd = demosaic_mod->reg_cmd;
  cfg_cmd->cfg_data = (void *)reg_cmd;
  cfg_cmd->cmd_len = sizeof(*reg_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;

  if (demosaic_mod->classifier_cfg_done == 0)
    cfg_cmd->num_cfg = 3;
  else
    cfg_cmd->num_cfg = 2;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_DEMOSAIC40_WB_GAIN_OFF;
  reg_cfg_cmd[0].u.rw_info.len = ISP_DEMOSAIC40_WB_GAIN_LEN *
    sizeof(uint32_t);

   /* 20st reg in demosaic_mod->reg_cmd*/
  reg_cfg_cmd[1].u.rw_info.cmd_data_offset = 20 * sizeof(uint32_t);
  reg_cfg_cmd[1].cmd_type = VFE_WRITE;
  reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_DEMOSAIC_INTERP_GAIN_OFF;
  reg_cfg_cmd[1].u.rw_info.len = ISP_DEMOSAIC_INTERP_GAIN_LEN *
    sizeof(uint32_t);

  if (demosaic_mod->classifier_cfg_done == 0) {
    /* 2nd register in demosaic_mod->reg_cmd */
    reg_cfg_cmd[2].u.rw_info.cmd_data_offset = 2 * sizeof(uint32_t);
    reg_cfg_cmd[2].cmd_type = VFE_WRITE;
    reg_cfg_cmd[2].u.rw_info.reg_offset = ISP_DEMOSAIC_CLASSIFIER_OFF;
    reg_cfg_cmd[2].u.rw_info.len = ISP_DEMOSAIC_CLASSIFIER_LEN *
      sizeof(uint32_t);
  }

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: demosaic40_util_append_cfg");
    goto ERROR_APPEND;

  }
  demosaic_mod->classifier_cfg_done = 1;
  isp_sub_module->trigger_update_pending = FALSE;

  return ret;

ERROR_APPEND:
  free(reg_cmd);
ERROR_REG_CMD_MALLOC:
  free(reg_cfg_cmd);
ERROR_REG_CFG_MALLOC:
  free(hw_update);
  return FALSE;
} /* demosaic40_do_hw_update */

/** demosaic40_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module instance
 *  @event: mct event for trigger update
 *
 *  This function checks and initiates triger update of module
 *
 *  Return TRUE on Success and FALSE Parameters size mismatch
 **/
boolean demosaic40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                       ret = TRUE;
  int                           temp = 0;
  float                         ratio = 0.0,
                                aG = 0.0,
                                bL = 0.0,
                                aG_lowlight = 0.0,
                                bL_lowlight = 0.0;
  ISP_Demosaic40ConfigCmdType  *p_cmd = NULL;
  chromatix_parms_type         *chromatix_ptr = NULL;
  chromatix_demosaic_type      *chromatix_demosaic = NULL;
  awb_gain_t                   *wb_gain = NULL;
  aec_update_t                 *aec_output = NULL;
  tuning_control_type          *tc = NULL;
  trigger_point_type           *tp = NULL;
  demosaic40_t                 *mod = NULL;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_priv_t        *isp_sub_module_priv = NULL;
  float                         new_aG = 0.0, new_bL = 0.0;
  isp_sub_module_output_t      *output = NULL;
  demosaic3_type               *classifier = NULL;
  demosaic3_Reserve            *reserve = NULL;
  demosaic3_tune               *data = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }

  mod = (demosaic40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
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

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if ((isp_sub_module->submod_enable == FALSE) ||
      (isp_sub_module->submod_trigger_enable == FALSE) ||
      (isp_sub_module->stream_on_count == 0)) {
    ISP_DBG("No trigger update for Demosaic: enable = %d, trigger_en = %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }
  if (isp_sub_module->trigger_update_pending == TRUE) {
    p_cmd = &mod->reg_cmd;
    chromatix_demosaic = &chromatix_ptr->chromatix_VFE.chromatix_demosaic;
    classifier = &(chromatix_demosaic->demosaic3_data);
    reserve = &(classifier->reserve);
    data = &(classifier->data[1]);

    wb_gain = &mod->gain;
    ratio = mod->ratio.ratio;

    aG = data->aG;
    bL = data->bL;
    data = &(classifier->data[0]);
    aG_lowlight = data->aG;
    bL_lowlight = data->bL;

    new_aG = LINEAR_INTERPOLATION(aG, aG_lowlight, ratio);
    new_bL = LINEAR_INTERPOLATION(bL, bL_lowlight, ratio);
    /*calculate bl*/
    temp = FLOAT_TO_Q(8, new_bL);
    p_cmd->bl = MIN(MAX(0, temp), 118);
    /*calculate bu*/
    temp = FLOAT_TO_Q(8, (1.0-new_bL));
    p_cmd->bu = MIN(MAX(138, temp), 255);
    p_cmd->gain = DEMOSAIC_RCCB_GAIN(1);
    /*calculate dblu*/
    temp = FLOAT_TO_Q(5, (1.0/(1.0-2*new_bL)));
    p_cmd->dblu = MIN(MAX(0, temp), 511);
    /*calculate a*/
    temp = FLOAT_TO_Q(6, new_aG);
    p_cmd->a = MIN(MAX(0, temp), 63);
    p_cmd->gain_offset = 0;
    /* update wb gains */
    ISP_DBG("gains r %f g %f b %f", wb_gain->r_gain, wb_gain->g_gain,
      wb_gain->b_gain);
    p_cmd->rgWbGain = FLOAT_TO_Q(7, (wb_gain->r_gain / wb_gain->g_gain));
    p_cmd->bgWbGain = FLOAT_TO_Q(7, (wb_gain->b_gain / wb_gain->g_gain));
    p_cmd->grWbGain = FLOAT_TO_Q(7, (wb_gain->g_gain / wb_gain->r_gain));
    p_cmd->gbWbGain = FLOAT_TO_Q(7, (wb_gain->g_gain / wb_gain->b_gain));

    ret = demosaic40_store_hw_update(isp_sub_module, mod);
    if (ret == FALSE) {
      ISP_ERR("failed: demosaic40_do_hw_update");
    }
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list mod %s",
      MCT_MODULE_NAME(module));
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* demosaic40_trigger_update */

/** demosaic40_streamon:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function makes initial configuration during first
 *  stream ON
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean demosaic40_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  demosaic40_t          *mod = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }

  mod = (demosaic40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  demosaic40_set_chromatix_params(mod,
    isp_sub_module->chromatix_ptrs.chromatixPtr,
    IS_BURST_STREAMING(mod->streaming_mode_mask));

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
} /* demosaic40_config */

/** demosaic40_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean demosaic40_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  demosaic40_t *mod = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (demosaic40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(&mod->reg_cmd, 0, sizeof(mod->reg_cmd));
  memset(&mod->ratio, 0, sizeof(mod->ratio));
  mod->classifier_cfg_done = 0;
  isp_sub_module->trigger_update_pending = FALSE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* demosaic40_streamoff */

/** demosaic40_reset:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets demosaic module
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean demosaic40_reset(mct_module_t *module, isp_sub_module_t *isp_sub_module,
  mct_event_t *event)
{
  demosaic40_t *mod = NULL;
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (demosaic40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  memset(&mod->reg_cmd, 0, sizeof(mod->reg_cmd));
  memset(&mod->ratio, 0, sizeof(mod->ratio));
  mod->classifier_cfg_done = 0;
  return TRUE;
} /* demosaic40_reset */

/** demosaic40_stats_aec_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean demosaic40_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  stats_update_t          *stats_update = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_demosaic_type *chromatix_demosaic = NULL;
  demosaic40_t            *mod = NULL;
  tuning_control_type     *tc = NULL;
  trigger_point_type      *tp = NULL;
  float                    ratio = 0.0;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  mod = (demosaic40_t *)isp_sub_module->private_data;
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

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    /* Put hw update flag to TRUE */
    isp_sub_module->trigger_update_pending = TRUE;
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  chromatix_demosaic = &chromatix_ptr->chromatix_VFE.chromatix_demosaic;

  tc = &chromatix_demosaic->control_demosaic3;
  tp = &chromatix_demosaic->demosaic3_trigger_lowlight;
  /*Do interpolation by the aec ratio*/
  ratio = demosaic40_get_aec_ratio_lowlight(*tc, tp, &stats_update->aec_update);
  ISP_DBG("aec ratio %f", ratio);

  if ((isp_sub_module->trigger_update_pending == FALSE) &&
      !F_EQUAL(mod->ratio.ratio, ratio)) {
    isp_sub_module->trigger_update_pending = TRUE;
  }

  mod->ratio.ratio = ratio;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** demosaic40_stats_awb_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Handle AWB update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean demosaic40_stats_awb_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  stats_update_t *stats_update = NULL;
  awb_gain_t     *cur_awb_gain = NULL;
  awb_gain_t     *new_awb_gain = NULL;
  demosaic40_t   *mod = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  mod = (demosaic40_t *)isp_sub_module->private_data;
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

  cur_awb_gain = &mod->gain;
  new_awb_gain = &stats_update->awb_update.gain;

  if (!F_EQUAL(cur_awb_gain->r_gain, new_awb_gain->r_gain) ||
      !F_EQUAL(cur_awb_gain->g_gain, new_awb_gain->g_gain) ||
      !F_EQUAL(cur_awb_gain->b_gain, new_awb_gain->b_gain)) {
      isp_sub_module->trigger_update_pending = TRUE;

      /* Store AWB update in module private */
      *cur_awb_gain = *new_awb_gain;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** demosaic40_ez_isp_update
 *
 *  @isp_sub_module: isp sub module handle
 *  @demosaic_module: demosaic module handle
 *  @demosaicDiag: demosaic Diag handle
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void demosaic40_ez_isp_update(isp_sub_module_t *isp_sub_module,
  demosaic40_t *demosaic_module, demosaic3_t *demosaicDiag)
{
  ISP_Demosaic40ConfigCmdType *demosaicCfg = NULL;
  int index;
  if (!isp_sub_module || !demosaic_module || !demosaicDiag) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, demosaic_module, demosaicDiag);
    return;
  }
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  demosaicCfg = &(demosaic_module->applied_RegCmd);
  demosaicDiag->aG = demosaicCfg->a;
  demosaicDiag->bL = demosaicCfg->bl;

  for(index = 0; index < ISP_DEMOSAIC40_CLASSIFIER_CNT; index++) {
    demosaicDiag->lut[index].bk = demosaicCfg->interpClassifier[index].b_n;
    demosaicDiag->lut[index].wk = demosaicCfg->interpClassifier[index].w_n;
    demosaicDiag->lut[index].lk = demosaicCfg->interpClassifier[index].l_n;
    demosaicDiag->lut[index].tk = demosaicCfg->interpClassifier[index].t_n;
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

}

/** demosaic40_get_vfe_diag_info_user
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Get VFE diag info
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean demosaic40_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  isp_private_event_t *private_event = NULL;
  demosaic40_t        *demosaic = NULL;
  vfe_diagnostics_t   *vfe_diag = NULL;
  demosaic3_t         *demosaicDiag = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p data %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  demosaic = (demosaic40_t *)isp_sub_module->private_data;
  if (!demosaic) {
    ISP_ERR("failed: demosaic %p", demosaic);
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

  demosaicDiag = &(vfe_diag->prev_demosaic);
  if (demosaic->old_streaming_mode == CAM_STREAMING_MODE_BURST) {
    demosaicDiag = &(vfe_diag->snap_demosaic);
  }

  demosaic40_ez_isp_update(isp_sub_module, demosaic, demosaicDiag);

  return TRUE;
}

/** demosaic40_open:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function instantiates a demosaic module
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean demosaic40_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  demosaic40_t *mod = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (demosaic40_t *)CAM_MALLOC(sizeof(demosaic40_t));
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  memset(mod, 0, sizeof(*mod));
  memset(&mod->reg_cmd, 0, sizeof(mod->reg_cmd));
  memset(&mod->ratio, 0, sizeof(mod->ratio));
  /* enable trigger update feature flag from PIX */
  mod->classifier_cfg_done = 0;

  isp_sub_module->private_data = (void *)mod;

  return TRUE;
} /* demosaic40_init */

/** demosaic40_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void demosaic40_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* demosaic40_destroy */
