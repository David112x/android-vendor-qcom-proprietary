/*============================================================================

  Copyright (c) 2013-2016 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/* std headers */
#include <unistd.h>
#include <math.h>

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "snr47.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_SNR, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_SNR, fmt, ##args)

#if !defined ABS
  #define  ABS(x) ((x)>0 ? (x) : -(x))
#endif

#define SNR_CFG_FILTER(c_filter, filter_reg_cmd) \
  (filter_reg_cmd)->coeff0 = (c_filter[3] > ((1 << 8) - 1)) ? ((1 << 8) - 1) : c_filter[3]; \
  (filter_reg_cmd)->coeff1 = (c_filter[2] > ((1 << 7) - 1)) ? ((1 << 7) - 1) : c_filter[2]; \
  (filter_reg_cmd)->coeff2 = (c_filter[1] > ((1 << 7) - 1)) ? ((1 << 7) - 1) : c_filter[1]; \
  (filter_reg_cmd)->coeff3 = (c_filter[0] > ((1 << 7) - 1)) ? ((1 << 7) - 1) : c_filter[0];


#define SNR_CFG_FILTER8(c_filter, filter_reg0, filter_reg1) \
  (filter_reg0)->coeff0 = (c_filter[7] > ((1 << 8) - 1)) ? ((1 << 8) - 1) : c_filter[7]; \
  (filter_reg0)->coeff1 = (c_filter[6] > ((1 << 7) - 1)) ? ((1 << 7) - 1) : c_filter[6]; \
  (filter_reg0)->coeff2 = (c_filter[5] > ((1 << 7) - 1)) ? ((1 << 7) - 1) : c_filter[5]; \
  (filter_reg0)->coeff3 = (c_filter[4] > ((1 << 7) - 1)) ? ((1 << 7) - 1) : c_filter[4]; \
  (filter_reg1)->coeff0 = (c_filter[3] > ((1 << 7) - 1)) ? ((1 << 7) - 1) : c_filter[3]; \
  (filter_reg1)->coeff1 = (c_filter[2] > ((1 << 7) - 1)) ? ((1 << 7) - 1) : c_filter[2]; \
  (filter_reg1)->coeff2 = (c_filter[1] > ((1 << 7) - 1)) ? ((1 << 7) - 1) : c_filter[1]; \
  (filter_reg1)->coeff3 = (c_filter[0] > ((1 << 7) - 1)) ? ((1 << 7) - 1) : c_filter[0];

#ifndef NORMAL_LIGHT_INDEX
  #define NORMAL_LIGHT_INDEX 4
#endif

/** snr47_cmd_debug:
 *
 *  @cmd: container of HW configuration
 *  @snr_path: stream path ENCODER/VIEWFINDER
 *
 *  Print xform coefficients
 *
 *  Return: None
 **/
static void snr47_cmd_debug(snr47_t *snr)
{
  if (!snr) {
    ISP_ERR("failed: snr %p",snr);
  } else {
    ISP_DBG(" ==== SNR reg dump ====");
    ISP_DBG("SNR enable: %u", snr->reg_cfgCmd.enable);

    ISP_DBG("yMin: %u yMax %u",
      snr->reg_cmd.border_cfg0.ymin, snr->reg_cmd.border_cfg0.ymax);
    ISP_DBG("shymin: %u, shymax: %u",
      snr->reg_cmd.border_cfg0.shymin, snr->reg_cmd.border_cfg0.shymax);
    ISP_DBG("smin_para: %u,smax_para: %u",
      snr->reg_cmd.border_cfg1.smin_para, snr->reg_cmd.border_cfg1.smax_para);
    ISP_DBG("boundary_prob: %u",
      snr->reg_cmd.border_cfg1.boundary_prob);
    ISP_DBG("hmin: %u",
      snr->reg_cmd.border_cfg1.hmin);
    ISP_DBG("hmax: %u", snr->reg_cmd.border_cfg2.hmax);
    ISP_DBG("q_skin: %u,q_nonskin: %u",
      snr->reg_cmd.border_cfg2.q_skin, snr->reg_cmd.border_cfg2.q_nonskin);

    ISP_DBG("soft_thr: %u",
      snr->reg_cmd.skinTone_cfg.soft_thr);
    ISP_DBG("soft_strength: %u",
      snr->reg_cmd.skinTone_cfg.soft_strength);

    ISP_DBG(" ==== SNR filter dump ====");
    ISP_DBG("NR filter coeff: [0]:%u, [1]: %u, [2]: %u, [3]: %u ",
      snr->reg_cmd.nr_filter.coeff0,
      snr->reg_cmd.nr_filter.coeff1,
      snr->reg_cmd.nr_filter.coeff2,
      snr->reg_cmd.nr_filter.coeff3);

    ISP_DBG("Y Filter coeff [0]: %u, [1] %u, [2] %u. [3] %u",
      snr->reg_cmd.y_filter.coeff0,
      snr->reg_cmd.y_filter.coeff1,
      snr->reg_cmd.y_filter.coeff2,
      snr->reg_cmd.y_filter.coeff3);

    ISP_DBG("C Filter  coeff [0]: %u [1]: %u [2]: %u [3]: %u",
      snr->reg_cmd.c_filter.coeff0,
      snr->reg_cmd.c_filter.coeff1,
      snr->reg_cmd.c_filter.coeff2,
      snr->reg_cmd.c_filter.coeff3);

    ISP_DBG("Prob Filter0: coeff [0]: %u [1]: %u [2]: %u [3]: %u",
      snr->reg_cmd.prob_filter.coeff0,
      snr->reg_cmd.prob_filter.coeff1,
      snr->reg_cmd.prob_filter.coeff2,
      snr->reg_cmd.prob_filter.coeff3);

    ISP_DBG("Prob Filter1: coeff [0]: %u [1]: %u [2]: %u [3]: %u",
      snr->reg_cmd.prob_filter1.coeff0,
      snr->reg_cmd.prob_filter1.coeff1,
      snr->reg_cmd.prob_filter1.coeff2,
      snr->reg_cmd.prob_filter1.coeff3);
  }
}

/** snr47_store_hw_update
 *   @isp_sub_module: isp sub module handle
 *
 * Update hw update list
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean snr47_store_hw_update(
  isp_sub_module_t *isp_sub_module, snr47_t *snr)
{
  boolean                          ret = TRUE;
  struct msm_vfe_cfg_cmd2         *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd      *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list     *hw_update = NULL;
  ISP_Snr_CmdType                 *reg_cmd = NULL;

  if (!isp_sub_module || !snr) {
    ISP_ERR("failed: isp_sub_module %p snr %p", isp_sub_module,
      snr);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: out of memory");
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd) * 2);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: out of memory reg_cfg_cmd");
    goto ERROR;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd) * 2);

  reg_cmd = (ISP_Snr_CmdType *)CAM_MALLOC(sizeof(ISP_Snr_CmdType));
  if (!reg_cmd) {
    ISP_ERR("failed: out of memory reg_cmd");
    goto ERROR;
  }
  memset(reg_cmd, 0, sizeof(*reg_cmd));

  *reg_cmd = snr->reg_cmd;

  cfg_cmd->cfg_data = (void *)reg_cmd;
  cfg_cmd->cmd_len = sizeof(snr->reg_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 2;

  snr->reg_cfgCmd.enable = isp_sub_module->submod_enable;

  reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;
  reg_cfg_cmd[0].u.mask_info.reg_offset = ISP_SNR47_CFG_OFF;
  reg_cfg_cmd[0].u.mask_info.mask = snr->reg_cfgCmd.cfg_reg;
  reg_cfg_cmd[0].u.mask_info.val = snr->reg_cfgCmd.cfg_reg;

  reg_cfg_cmd[1].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[1].cmd_type = VFE_WRITE;
  reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_SNR47_OFF;
  reg_cfg_cmd[1].u.rw_info.len = ISP_SNR47_LEN * sizeof(uint32_t);

  snr47_cmd_debug(snr);

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR;
  }

  return TRUE;

ERROR:
  if (reg_cmd) {
    free(reg_cmd);
  }
  if (reg_cfg_cmd) {
    free(reg_cfg_cmd);
  }
  if (hw_update) {
    free(hw_update);
  }
  return FALSE;
}

/** snr47_config_interpolate:
 *
 *  @isp_sub_module: isp sub module handle
 *  @snr: snr module data
 *
 *  Calculate the different values using interpolation
 *
 *  Return TRUE on success and FALSE on failure
 **/
static void snr47_config_interpolate(isp_sub_module_t *isp_sub_module,
  snr47_t *snr)
{
  int i = 0;
  float tmp = 0, smin_para = 0, smax_para = 0, hmin = 0, hmax = 0;
  float shymin = 0, shymax = 0, slymax = 0, slymin = 0, ymin = 0, ymax = 0;
  float q_skin = 0, q_nonskin = 0;
  uint32_t tmp_int = 0;
  float skin_percent= 0.0;
  float non_skin_to_skin_q_ratio = 0.0;
  float  boundary_probability_non_round = 0.0;
  Chromatix_SkinNR_core_type  *snr_data_start = NULL;
  Chromatix_SkinNR_core_type  *snr_data_end = NULL;
  chromatix_parms_type        *chromatix_ptr = NULL;
  Chromatix_Skintone_NR_type  *chromatix_SNR = NULL;
  float                        ratio = 0.0;
  ISP_Snr_CmdType             *reg_cmd = NULL;


  if (isp_sub_module == NULL || snr == NULL) {
    ISP_ERR("failed: null ptr!!! sub mod %p, snr %p",
      isp_sub_module, snr);
    return;
  }

  if (snr->trigger_index + 1 > MAX_LIGHT_TYPES_FOR_SPATIAL) {
    ISP_ERR("failed: triger index out of bound!!!");
    return;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (chromatix_ptr == NULL) {
    ISP_ERR("failed: null ptr chromatix %p!!!", chromatix_ptr);
    return;
  }

  chromatix_SNR = &chromatix_ptr->chromatix_VFE.chromatix_skin_nr_data;
  if (chromatix_SNR == NULL) {
    ISP_ERR("failed: null ptr chromatix_SNR %p!!!", chromatix_SNR);
    return;
  }

  reg_cmd = &snr->reg_cmd;

  snr_data_start = &(chromatix_SNR->SkinNR_data[snr->trigger_index]);
  snr_data_end   = &(chromatix_SNR->SkinNR_data[snr->trigger_index + 1]);

  ratio = snr->aec_ratio.ratio;
  /*SNR filter should not do interpolate*/
  if (ratio != 0.0) {
  SNR_CFG_FILTER(snr_data_start->skin_nr_filter, &reg_cmd->nr_filter);
  SNR_CFG_FILTER(snr_data_start->skin_detection_y_filter, &reg_cmd->y_filter);
  SNR_CFG_FILTER(snr_data_start->skin_detection_chroma_filter,
    &reg_cmd->c_filter);
  SNR_CFG_FILTER8(snr_data_start->skin_probability_filter,
    &reg_cmd->prob_filter, &reg_cmd->prob_filter1);
  } else {
    /*if ratio = 0, mean completely switch to next region*/
    SNR_CFG_FILTER(snr_data_end->skin_nr_filter, &reg_cmd->nr_filter);
    SNR_CFG_FILTER(snr_data_end->skin_detection_y_filter, &reg_cmd->y_filter);
    SNR_CFG_FILTER(snr_data_end->skin_detection_chroma_filter,
      &reg_cmd->c_filter);
    SNR_CFG_FILTER8(snr_data_end->skin_probability_filter,
      &reg_cmd->prob_filter, &reg_cmd->prob_filter1);
  }

  if (snr->cac_enable) {
    reg_cmd->c_filter.coeff2 = ISP_SNR47_C_COEFF2_DEFAULT;
    reg_cmd->c_filter.coeff3 = ISP_SNR47_C_COEFF3_DEFAULT;
    /*error case handling: in such region, CAC is supposed to be OFF.
      tuning header should follow the rule.
      if tuning failed to follow, SW just set a default values*/
    if (snr->trigger_index >= ISP_SNR47_CAC_ON_REGION) {
      ISP_DBG("error!, SNR trigger region %d,CAC enable %d",
        snr->trigger_index, snr->cac_enable);
      reg_cmd->c_filter.coeff0 = ISP_SNR47_C_COEFF0_DEFAULT;
      reg_cmd->c_filter.coeff1 = ISP_SNR47_C_COEFF1_DEFAULT;
      reg_cmd->c_filter.coeff2 = ISP_SNR47_C_COEFF2_DEFAULT;
      reg_cmd->c_filter.coeff3 = ISP_SNR47_C_COEFF3_DEFAULT;
    }
  }

  tmp_int = (LINEAR_INTERPOLATION_INT(
    (uint32_t)snr_data_start->skin_soft_threshold,
    (uint32_t)snr_data_end->skin_soft_threshold, ratio));
  reg_cmd->skinTone_cfg.soft_thr = (tmp_int > ((1 << 8) - 1)) ? ((1 << 8) - 1) : tmp_int;

  tmp = (float)(LINEAR_INTERPOLATION(snr_data_start->skin_soft_strength,
    snr_data_end->skin_soft_strength, ratio));
  reg_cmd->skinTone_cfg.soft_strength = Clamp(FLOAT_TO_Q(6, tmp), 0, 64);

  ymax = (float)(LINEAR_INTERPOLATION(snr_data_start->Y_max,
    snr_data_end->Y_max, ratio));

  reg_cmd->border_cfg0.ymax = Clamp(FLOAT_TO_Q(8, ymax), 0, (1 << 8) - 1);

  ymin = (float)(LINEAR_INTERPOLATION(snr_data_start->Y_min,
    snr_data_end->Y_min, ratio));
  reg_cmd->border_cfg0.ymin = Clamp(FLOAT_TO_Q(8, ymin), 0, (1 << 8) - 1);

  shymax = (float)(LINEAR_INTERPOLATION(snr_data_start->S_hY_max,
    snr_data_end->S_hY_max, ratio));
  reg_cmd->border_cfg0.shymax = Clamp(FLOAT_TO_Q(8, shymax), 0, (1 << 8) - 1);

  shymin = (float)(LINEAR_INTERPOLATION(snr_data_start->S_hY_min,
    snr_data_end->S_hY_min, ratio));
  reg_cmd->border_cfg0.shymin = Clamp(FLOAT_TO_Q(8, shymin), 0, (1 << 8) - 1);

  slymax = (float)(LINEAR_INTERPOLATION(snr_data_start->S_lY_max,
    snr_data_end->S_lY_max, ratio));
  slymin = (float)(LINEAR_INTERPOLATION(snr_data_start->S_lY_min,
    snr_data_end->S_lY_min, ratio));

  if (ymax - ymin != 0) {
    smax_para = (slymax - shymax) / (ymax - ymin);
    smin_para = (slymin - shymin) / (ymax - ymin);
  } else {
    ISP_ERR("failed, devide by zero check!!! ymax = ymin");
  }

  reg_cmd->border_cfg1.smax_para= Clamp(FLOAT_TO_Q(8, smax_para), 0,
    (1 << 8) - 1);
  reg_cmd->border_cfg1.smin_para= Clamp(FLOAT_TO_Q(8, smin_para), 0,
    (1 << 8) - 1);

  hmax = (float)(LINEAR_INTERPOLATION(snr_data_start->H_max,
    snr_data_end->H_max, ratio));
  hmin = (float)(LINEAR_INTERPOLATION(snr_data_start->H_min,
    snr_data_end->H_min, ratio));

  if (hmax < hmin) {
    ISP_ERR(" Invalid params hmin %f hmax %f", hmin, hmax);
  } else {
    hmax = ABS(hmax);
    hmin = ABS(hmin);
    reg_cmd->border_cfg2.hmax = Clamp(FLOAT_TO_Q(8, hmax), 0, (1 << 8) - 1);
    reg_cmd->border_cfg1.hmin = Clamp(FLOAT_TO_Q(8, hmin), 0, (1 << 10) - 1);
  }

  reg_cmd->border_cfg1.boundary_prob = (uint32_t) (
    (LINEAR_INTERPOLATION_INT(snr_data_start->boundary_probability,
    snr_data_end->boundary_probability, ratio)));

  /*Skin factor, Q8, range 0-128*/
  boundary_probability_non_round =
    LINEAR_INTERPOLATION((float)snr_data_start->boundary_probability,
    (float)snr_data_end->boundary_probability, ratio);

  /*skin factor and non skin factor range [0-128]*/
  skin_percent =
    (LINEAR_INTERPOLATION((float)snr_data_start->skin_percent,
    (float)snr_data_end->skin_percent, ratio));
  q_skin = (100.0 - skin_percent) /
    (200.0 * (16.0 - boundary_probability_non_round));
  reg_cmd->border_cfg2.q_skin =
    Clamp(FLOAT_TO_Q(8, q_skin), 0, MAX_Q_SKIN);

  non_skin_to_skin_q_ratio =
    (float)(LINEAR_INTERPOLATION(snr_data_start->non_skin_to_skin_q_ratio,
    snr_data_end->non_skin_to_skin_q_ratio, ratio));
  q_nonskin = non_skin_to_skin_q_ratio * q_skin;
  reg_cmd->border_cfg2.q_nonskin =
    Clamp(FLOAT_TO_Q(8, q_nonskin), 0, MAX_Q_SKIN);

  ISP_DBG("=== interpolate between trigger index[0-5]  %d ~ %d, ratio %f=====",
    snr->trigger_index, snr->trigger_index + 1, ratio);
  ISP_DBG("skin_percent %f, qskin_reg %d (Q8 * q_skin %f)",
    skin_percent, reg_cmd->border_cfg2.q_skin, q_skin);
  ISP_DBG("non_skin_to_skin ratio %f, q_nonskin(Q8) %d",
    non_skin_to_skin_q_ratio, reg_cmd->border_cfg2.q_nonskin);

  return;
}

/** snr47_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: module event data
 *
 *  Perform trigger update if trigger_update_pending flag is
 *  TRUE and append hw update list in global list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean snr47_trigger_update(isp_sub_module_t *isp_sub_module,
  isp_private_event_t *private_event)
{
  boolean                                ret = TRUE;
  snr47_t                               *snr = NULL;
  isp_sub_module_output_t               *output = NULL;
  chromatix_parms_type                  *chromatix_ptr = NULL;
  isp_sub_module_output_t               *sub_module_output = NULL;
  uint8_t                                module_enable;

  if (!isp_sub_module || !private_event) {
    ISP_ERR("failed: %p %p", isp_sub_module, private_event);
    return FALSE;
  }

  sub_module_output = (isp_sub_module_output_t *)private_event->data;
  if (!sub_module_output) {
    ISP_ERR("failed: sub_module_output %p", sub_module_output);
    return FALSE;
  }

  snr = (snr47_t *)isp_sub_module->private_data;
  if (!snr) {
    ISP_ERR("failed: snr %p", snr);
    return FALSE;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed, NULL ptr, %p", chromatix_ptr);
    return FALSE;
  }

  if (isp_sub_module->manual_ctrls.manual_update &&
      isp_sub_module->chromatix_module_enable) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    module_enable = (isp_sub_module->manual_ctrls.abf_mode ==
      CAM_NOISE_REDUCTION_MODE_OFF) ? FALSE : TRUE;

    if (isp_sub_module->submod_enable != module_enable) {
      isp_sub_module->submod_enable = module_enable;
      sub_module_output->stats_params->
        module_enable_info.reconfig_needed = TRUE;
      sub_module_output->stats_params->module_enable_info.
        submod_enable[isp_sub_module->hw_module_id] = module_enable;
      sub_module_output->stats_params->
        module_enable_info.submod_mask[isp_sub_module->hw_module_id] = 1;
    }
  }

  if (!isp_sub_module->submod_enable) {
    /* SNR module is disabled, do not perform hw update */
    goto FILL_METADATA;
  }

  if ((isp_sub_module->submod_trigger_enable == TRUE) &&
    (isp_sub_module->trigger_update_pending == TRUE)) {
    ISP_DBG("aec_ratio.ratio = %f",snr->aec_ratio.ratio);
    if (snr->aec_ratio.ratio < 0) {
      snr->aec_ratio.ratio = 0;
    }

    isp_sub_module->trigger_update_pending = FALSE;
    snr47_config_interpolate(isp_sub_module, snr);
    snr47_store_hw_update(isp_sub_module, snr);
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
          sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    return FALSE;
  }

FILL_METADATA:
   if (sub_module_output->frame_meta)
     sub_module_output->frame_meta->abf_mode =
       isp_sub_module->manual_ctrls.abf_mode;

  return ret;
}

/** snr47_update_sudmod_enable:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: module event data
 *
 *  This function updates the cac enable flag from the pipeline submod enable array
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean snr47_update_submod_enable(isp_sub_module_t *isp_sub_module,
  isp_private_event_t *private_event)
{
  boolean                       retValue          = FALSE;
  snr47_t                      *snr               = NULL;
  uint8_t                      *submod_enable     = NULL;
  chromatix_VFE_common_type    *pchromatix_common = NULL;

  if (!isp_sub_module || !private_event || !private_event->data) {
    ISP_ERR("failed: %p %p", isp_sub_module,
      private_event);
  } else if (!private_event->data) {
      ISP_ERR("failed: %p", private_event->data);
  } else {
      snr = (snr47_t *)isp_sub_module->private_data;
      submod_enable = (uint8_t *)private_event->data;

      ISP_DBG("CAC enable %d", submod_enable[ISP_MOD_CAC]);
      snr->cac_enable = submod_enable[ISP_MOD_CAC];

      isp_sub_module->trigger_update_pending = TRUE;
      retValue = TRUE;
  }

  return retValue;
}

/** snr47_set_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: module event data
 *
 *  This function sets the chromatic pointers and enable bits
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean snr47_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  modulesChromatix_t *chromatix_ptrs)
{
  boolean  ret = TRUE;
  snr47_t *snr = NULL;

  if (!isp_sub_module || !chromatix_ptrs) {
    ISP_ERR("failed: %p %p", isp_sub_module, chromatix_ptrs);
    return FALSE;
  }

  snr = (snr47_t *)isp_sub_module->private_data;
  if (!snr) {
    ISP_ERR("failed: mod %p", snr);
    return FALSE;
  }

  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;

  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
    return FALSE;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  return ret;
} /* snr_set_chromatix_ptr */

/** snr47_aec_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: module event data
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean snr47_aec_update(isp_sub_module_t *isp_sub_module,
  stats_update_t *stats_update)
{
  snr47_t   *snr = NULL;
  trigger_ratio_t   aec_ratio;
  chromatix_parms_type  *chromatix_ptr = NULL;
  Chromatix_Skintone_NR_type *chromatix_SNR = NULL;
  uint8_t trigger_index = MAX_LIGHT_TYPES_FOR_SPATIAL + 1;
  float aec_reference = 0.0;
  float ratio = 0.0;

  int i = 0;

  if (!isp_sub_module || !stats_update) {
    ISP_ERR("failed: %p %p", isp_sub_module, stats_update);
    return FALSE;
  }

  snr = (snr47_t *)isp_sub_module->private_data;
  if (snr == NULL) {
    ISP_ERR("failed: snr %p", snr);
    return FALSE;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: %p", chromatix_ptr);
    return TRUE;
  }

  chromatix_SNR =
    &chromatix_ptr->chromatix_VFE.chromatix_skin_nr_data;
  if (chromatix_SNR->control_skintone_nr == 0) {
    /* lux index based */
    aec_reference = stats_update->aec_update.lux_idx;
  } else {
    /* Gain based */
    aec_reference = stats_update->aec_update.real_gain;
  }

  ISP_DBG("aec_reference :%f", aec_reference);

  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
    float start = 0;
    float end = 0;
    trigger_point_type *snr_trigger =
      &(chromatix_SNR->SkinNR_data[i].skintone_nr_trigger);

    if (i == MAX_LIGHT_TYPES_FOR_SPATIAL - 1) {
      ISP_DBG("last region, region idenx %d, ratio %f", trigger_index, ratio);
      /* falls within region 6 but we do not use trigger points in the region */
      ratio = 0.0;
      break;
    }
    trigger_index = i;

    if (chromatix_SNR->control_skintone_nr == 0) {
      start = snr_trigger->lux_index_start;
      end   = snr_trigger->lux_index_end;
    } else {
      start = snr_trigger->gain_start;
      end   = snr_trigger->gain_end;
    }

    ISP_DBG("start :%f end %f, aec_reference %f, trigger_index %d",
      start, end, aec_reference, trigger_index);

    /* already scanned past the lux index */
    if (aec_reference < start) {
      ratio = 1.0;
    } else {
      /* index is within interpolation range, find ratio */
      if (aec_reference >= start && aec_reference < end) {
        /* ratio = ratio to the start, thus 1 - (distance to start)*/
        ratio = 1.0 - (aec_reference - start)/(end - start);
      }
    }

    /* already scanned past the lux index */
    if (aec_reference < end) {
      ISP_DBG("found trigger region");
      break;
    }
  }

  if(!snr47_decide_hysterisis(isp_sub_module, aec_reference)){
    ISP_ERR("SNR Hysterisis failure");
    return FALSE;
  }

  if (trigger_index >= MAX_LIGHT_TYPES_FOR_SPATIAL) {
    ISP_ERR("no trigger match for SNR trigger value: %f\n", aec_reference);
    return FALSE;
  }

  if ((trigger_index != snr->trigger_index) ||
    (!F_EQUAL(ratio, snr->aec_ratio.ratio))) {
    snr->trigger_index = trigger_index;
    snr->aec_ratio.ratio = ratio;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  return TRUE;
}

/** snr47_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @snr: snr handle
 *
 *  Configures default values
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean snr47_config(isp_sub_module_t *isp_sub_module,
  snr47_t *snr)
{
  boolean ret = TRUE;

  snr47_config_interpolate(isp_sub_module, snr);
  isp_sub_module->trigger_update_pending = TRUE;

  return ret;
} /* snr47_config */

/** snr47_streamon:
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
boolean snr47_streamon(isp_sub_module_t *isp_sub_module)
{
  boolean                ret = TRUE;
  snr47_t               *mod = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: %p", isp_sub_module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  mod = (snr47_t *)isp_sub_module->private_data;
  if (mod == NULL) {
    ISP_ERR("failed: snr mod %p", mod);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("snr enable = %d", isp_sub_module->submod_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  ret = snr47_config(isp_sub_module, mod);
  if (ret == FALSE) {
    ISP_ERR("failed: snr47_config ret %d", ret);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return ret;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* snr47_config */

/** snr47_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean snr47_streamoff(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: %p", isp_sub_module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (!isp_sub_module->stream_on_count) {
    ISP_ERR(" Error! count already 0");
  /* since all streamoff done, its not fatal. return TRUE so other streamoff process can go on */
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  isp_sub_module->trigger_update_pending = FALSE;
  isp_sub_module->manual_ctrls.abf_mode = CAM_NOISE_REDUCTION_MODE_FAST;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
} /* snr47_streamoff */

/** snr47_init:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the snr module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean snr47_init(isp_sub_module_t *isp_sub_module)
{
  snr47_t *snr = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  snr = (snr47_t *)CAM_MALLOC(sizeof(snr47_t));
  if (!snr) {
    ISP_ERR("failed: snr %p", snr);
    return FALSE;
  }

  memset(snr, 0, sizeof(*snr));
  snr->trigger_index = NORMAL_LIGHT_INDEX;
  snr->aec_ratio.ratio = 0;

  isp_sub_module->private_data = (void *)snr;
  isp_sub_module->manual_ctrls.abf_mode = CAM_NOISE_REDUCTION_MODE_FAST;

  return TRUE;
}/* snr_init */

/** snr47_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void snr47_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  if (isp_sub_module->stream_on_count != 0) {
    ISP_ERR("destroy destroy while stream on cnt = %d!",
      isp_sub_module->stream_on_count);
    isp_sub_module->stream_on_count = 0;
  }
  free(isp_sub_module->private_data);
  isp_sub_module->private_data = NULL;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
} /* snr47_destroy */
