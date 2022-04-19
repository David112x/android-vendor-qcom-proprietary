/* cac47.c
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>

/* mctl headers */
#include "eztune_vfe_diagnostics.h"
#include "camera_dbg.h"

/* isp headers */
#include "module_cac47.h"
#include "cac47.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module_util.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_CAC, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_CAC, fmt, ##args)

#define CAC47_DEFAULT_TRIGGER_INDEX 3

/** cac47_debug:
 *
 *  @cmd: configuration command
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function dumps cac configuration
 *
 *  Return: None
 **/
static void cac47_debug(ISP_CacCfgCmdType *RegCmd)
{
  if (!RegCmd) {
    ISP_ERR("Null pointer %p", RegCmd);
  } else {
    ISP_DBG("RegCmd->cSaturationThr %u", RegCmd->cSaturationThr);
    ISP_DBG("RegCmd->cSpotThr %u", RegCmd->cSpotThr);
    ISP_DBG("RegCmd->ResMode %u", RegCmd->ResMode);
    ISP_DBG("RegCmd->ySaturationThr %u", RegCmd->ySaturationThr);
    ISP_DBG("RegCmd->ySpotThr %u", RegCmd->ySpotThr);
    /* cac reg*/
  }
}

/** cac47_calculate_scale_factor:
 *
 *  @cac_mod: module private
 *
 *  This function calculates the scaling factor.
 *
 *  Return: float
 **/
static float cac47_calculate_scale_factor(cac47_t *cac_mod)
{
  float scale_factor = -1.0;
  int   i            = 0;

  if(!cac_mod) {
    ISP_ERR("failed: %p", cac_mod);
    return 0;
  }

  for (i=0; i < ISP_HW_STREAM_MAX; i++) {
    if (cac_mod->cac_scaler_scale_factor[i].is_stream_on == TRUE) {
      if ((scale_factor < 0) ||
          (scale_factor > cac_mod->cac_scaler_scale_factor[i].scale_factor)) {
        scale_factor = cac_mod->cac_scaler_scale_factor[i].scale_factor;
      }
    }
  }

  return scale_factor;
}

/** cac47_store_hw_update:
 *
 *  @isp_sub_mod: sub module handle
 *  @cac_mod: module private
 *
 *  This function checks and sends configuration update to kernel
 *
 *  Return:  boolean
 **/
static boolean cac47_store_hw_update(
  isp_sub_module_t *isp_sub_module,
  cac47_t *cac_mod)
{
  boolean                      ret = TRUE;
  int                          i, rc = 0;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  ISP_CacCfgCmdType           *reg_cmd = NULL;

  if (!cac_mod || !isp_sub_module) {
    ISP_ERR("failed: %p %p", cac_mod, isp_sub_module);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    goto ERROR;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd) * 1);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR;
  }
  memset(reg_cfg_cmd, 0, (sizeof(struct msm_vfe_reg_cfg_cmd)) * 1);

  reg_cmd = (ISP_CacCfgCmdType *)CAM_MALLOC(sizeof(ISP_CacCfgCmdType));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cmd);
    goto ERROR;
  }
  memset(reg_cmd, 0, sizeof(ISP_CacCfgCmdType));

  *reg_cmd = cac_mod->RegCmd;

  cfg_cmd->cfg_data = (void *)reg_cmd;
  cfg_cmd->cmd_len = sizeof(*reg_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_CAC47_OFF;
  reg_cfg_cmd[0].u.rw_info.len = ISP_CAC47_LEN * sizeof(uint32_t);

  cac47_debug(&cac_mod->RegCmd);
  cac_mod->applied_RegCmd = cac_mod->RegCmd;

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: cac47_util_append_cfg");
    goto ERROR;
  }

  return ret;

ERROR:
  ISP_ERR("store hw update failed!");
  if (reg_cmd != NULL)
    free(reg_cmd);
  if (reg_cfg_cmd != NULL)
    free(reg_cfg_cmd);
  if (hw_update != NULL)
    free(hw_update);

  return FALSE;
}

/** cac47_trigger_interpolate:
 *
 *  @module:  module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean cac47_trigger_interpolate(isp_sub_module_t *isp_sub_module,
  cac47_t *cac)
{
  int i = 0;
  chromatix_parms_type            *chromatix_ptr = NULL;
  Chromatix_HW_CAC2_type          *chromatix_cac = NULL;
  Chromatix_HW_CAC2_core_type     *cac_data_start = NULL;
  Chromatix_HW_CAC2_core_type     *cac_data_end = NULL;
  float                            ratio = 0.0;
  float                            tmp = 0;

  if (!isp_sub_module || !cac) {
    ISP_ERR("failed: %p %p", isp_sub_module, cac);
    return FALSE;
  }

  if (cac->trigger_index + 1>
      MAX_LIGHT_TYPES_FOR_SPATIAL) {
    ISP_ERR("invalid trrigger_index, no interplate");
    return TRUE;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("Null pointer! %p", chromatix_ptr);
    return FALSE;
  }
  chromatix_cac =
    &chromatix_ptr->chromatix_VFE.chromatix_hw_CAC2_data;

  ISP_DBG("trigger index %d", cac->trigger_index);
  cac_data_start = &(chromatix_cac->CAC2_data[cac->trigger_index]);
  cac_data_end   = &(chromatix_cac->CAC2_data[cac->trigger_index + 1]);

  ratio = cac->aec_ratio.ratio;

  tmp = LINEAR_INTERPOLATION((float)cac_data_start->y_saturation_thr,
    (float)cac_data_end->y_saturation_thr, ratio);
  cac->y_saturation_thr = Round(tmp);

  tmp = LINEAR_INTERPOLATION((float)cac_data_start->y_spot_thr,
    (float)cac_data_end->y_spot_thr, ratio);
  cac->y_spot_thr = Round(tmp);

  tmp = LINEAR_INTERPOLATION((float)cac_data_start->c_saturation_thr,
    (float)cac_data_end->c_saturation_thr, ratio);
  cac->c_saturation_thr = Round(tmp);

  tmp = LINEAR_INTERPOLATION((float)cac_data_start->c_spot_thr,
    (float)cac_data_end->c_spot_thr, ratio);
  cac->c_spot_thr = Round(tmp);

  return TRUE;
}

/** cac47_prepare_hw_config:
 *
 *  @module:  module instance
 *  @isp_sub_module: sub module handle
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean cac47_prepare_hw_config(isp_sub_module_t *isp_sub_module,
  cac47_t *cac)
{
  ISP_CacCfgCmdType              *RegCmd = NULL;

  if (!isp_sub_module || !cac) {
    ISP_ERR("failed: %p %p", isp_sub_module, cac);
    return FALSE;
  }

  RegCmd = &cac->RegCmd;

  cac->c_spot_thr = CLAMP_LIMIT(cac->c_spot_thr,
    0, ISP_CAC47_C_SPOT_TH_LIMIT);
  cac->c_saturation_thr = CLAMP_LIMIT(cac->c_saturation_thr,
    0, ISP_CAC47_C_SATU_TH_LIMIT);
  cac->y_spot_thr = CLAMP_LIMIT(cac->y_spot_thr,
    0, ISP_CAC47_Y_SPOT_TH_LIMIT);
  cac->y_saturation_thr = CLAMP_LIMIT(cac->y_saturation_thr,
    0, ISP_CAC47_Y_SATU_TH_LIMIT);

  RegCmd->ySaturationThr = cac->y_saturation_thr;
  RegCmd->ySpotThr = cac->y_spot_thr;
  RegCmd->cSaturationThr = cac->c_saturation_thr;
  RegCmd->cSpotThr = cac->c_spot_thr;

  /*RES mode select YUV422 and 420*/
  RegCmd->ResMode = 0;

  return TRUE;
}

/** cac47_default_config:
 *
 *  @module: cac mac module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function makes initial config of cac mac
 *
 *  Return:   TRUE- Success
 *            FALSE - Parameters size mismatch
 **/
boolean cac47_default_config(isp_sub_module_t *isp_sub_module, cac47_t *cac)
{
  boolean                  ret = TRUE;
  chromatix_parms_type    *chromatix_ptr = NULL;
  Chromatix_HW_CAC2_type  *chromatix_cac = NULL;

  if (!cac || !isp_sub_module) {
    ISP_ERR("failed: %p %p", cac, isp_sub_module);
    return FALSE;
  }

  chromatix_ptr = (chromatix_parms_type *)
    isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_cac = &chromatix_ptr->chromatix_VFE.chromatix_hw_CAC2_data;

  ret = cac47_trigger_interpolate(isp_sub_module, cac);
  if (ret == FALSE) {
    ISP_ERR("failed: cac trigger interpolate for default");
    return FALSE;
  }
  ret = cac47_prepare_hw_config(isp_sub_module, cac);
  if (ret == FALSE) {
    ISP_ERR("failed: cac trigger interpolate for default");
    return FALSE;
  }

  return ret;
}

/** cac47_set_chromatix_ptr:
 *
 *  @module: cac module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function makes initial config of cac
 *
 *  Return:   TRUE- Success
 *            FALSE - Parameters size mismatch
 **/
boolean cac47_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret = TRUE;
  cac47_t                 *cac = NULL;
  modulesChromatix_t      *chromatix_ptrs = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  chromatix_ptrs =
    (modulesChromatix_t *)event->u.module_event.module_event_data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    return FALSE;
  }

  cac = (cac47_t *)isp_sub_module->private_data;
  if (!cac) {
    ISP_ERR("failed: cac %p", cac);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;
  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }

  ret = cac47_default_config(isp_sub_module, cac);
  if (ret == FALSE) {
    ISP_ERR("prepare hw cfg failed!");
  }

  isp_sub_module->submod_trigger_enable = TRUE;
  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** cac47_streamon:
 *
 *  @module: cac module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function makes initial config of cac
 *
 *  Return:   TRUE- Success
 *            FALSE - Parameters size mismatch
 **/
boolean cac47_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret = TRUE;
  cac47_t                 *cac = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  cac = (cac47_t *)isp_sub_module->private_data;
  if (!cac) {
    ISP_ERR("failed: cac %p", cac);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  /* Update streaming mode mask in module private */
  mct_list_traverse(isp_sub_module->l_stream_info,
    isp_sub_module_util_update_streaming_mode, &cac->streaming_mode_mask);

  cac->old_streaming_mode = CAM_STREAMING_MODE_MAX;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** cac47_streamoff:
 *
 *  @module: cac module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function resets cac module
 *
 *  Return: boolean
 **/
boolean cac47_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  cac47_t *mod = NULL;
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (cac47_t *)isp_sub_module->private_data;
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
  isp_sub_module->submod_trigger_enable = FALSE; /* enable trigger update feature flag from PIX */
    isp_sub_module->manual_ctrls.cac_mode = CAM_COLOR_CORRECTION_ABERRATION_FAST;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** cac47_aec_update:
 *
 *  @module:  module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean cac47_aec_udpate(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  boolean                 ret = TRUE;
  stats_update_t          *stats_update = NULL;
  cac47_t                 *cac_mod = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  Chromatix_HW_CAC2_type  *chromatix_cac = NULL;

  ISP_HIGH("E");

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return FALSE;
  }

  cac_mod = (cac47_t *)isp_sub_module->private_data;
  if (!cac_mod) {
    ISP_ERR("failed: mod %p", cac_mod);
    return FALSE;
  }

  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return TRUE;
  }

  chromatix_cac =
    &chromatix_ptr->chromatix_VFE.chromatix_hw_CAC2_data;

  /* Store AEC update in module private*/
  cac_mod->aec_update;

  return TRUE;
}

/** cac47_decide_hyster_point_enable:
 *
 *  @module:  module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean cac47_decide_hyster_point_enable(isp_sub_module_t *isp_sub_module,
   float aec_ref)
{
  boolean                  ret            = TRUE;
  chromatix_parms_type    *chromatix_ptr  = NULL;
  Chromatix_HW_CAC2_type  *chromatix_cac  = NULL;
  trigger_point_type      *hyster_trigger = NULL;
  cac47_t                 *cac_mod        = NULL;
  float                    hyster_start   = 0.0;
  float                    hyster_end     = 0.0;

  if (isp_sub_module == NULL) {
    ISP_ERR("failed: NULL pointer %p", isp_sub_module);
    return FALSE;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  cac_mod = (cac47_t *)isp_sub_module->private_data;
  if (!cac_mod) {
    ISP_ERR("failed: mod %p", cac_mod);
    return FALSE;
  }

  chromatix_cac =
    &chromatix_ptr->chromatix_VFE.chromatix_hw_CAC2_data;

  hyster_trigger = &chromatix_cac->CAC2_hysteresis_point;

  if (chromatix_cac->control_cac2 == 0) {
    /* lux index based */
    hyster_start = hyster_trigger->lux_index_start;
    hyster_end   = hyster_trigger->lux_index_end;
  } else {
    /* Gain based */
    hyster_start = hyster_trigger->gain_start;
    hyster_end   = hyster_trigger->gain_end;
  }

  if (aec_ref > hyster_end) {
    if (isp_sub_module->submod_enable == TRUE) {
      isp_sub_module->submod_enable     = FALSE;
      isp_sub_module->update_module_bit = TRUE;
      cac_mod->hyster_en                = FALSE;
      ISP_HIGH("disable CAC by Hyster point, CAC enable %d, aec_ref %f," \
        " hyster end %f", isp_sub_module->submod_enable, aec_ref, hyster_end);
    }
  } else if (aec_ref < hyster_start) {
    cac_mod->hyster_en                = TRUE;
    if ((cac_mod->scaling_en == TRUE) &&
      (isp_sub_module->submod_enable !=
      isp_sub_module->chromatix_module_enable)) {
      isp_sub_module->submod_enable = isp_sub_module->chromatix_module_enable;
      isp_sub_module->update_module_bit = TRUE;
      ISP_HIGH("enable ON CAC by Hyster point, CAC enable %d, aec_ef %f," \
        " hyster_start %f", isp_sub_module->submod_enable, aec_ref,
        hyster_start);
    }
  }

  return TRUE;
}

/** cac47_save_aec_param:
 *
 *  @module:  module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean cac47_save_aec_params(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret = TRUE;
  stats_update_t          *stats_update = NULL;
  cac47_t                 *cac = NULL;
  trigger_ratio_t          aec_ratio;
  chromatix_parms_type    *chromatix_ptr = NULL;
  Chromatix_HW_CAC2_type  *chromatix_cac = NULL;
  trigger_point_type      *cac_trigger = NULL;
  uint8_t                 trigger_index = MAX_LIGHT_TYPES_FOR_SPATIAL + 1;
  float                   aec_reference;
  float                   ratio = 0;
  int                     i = 0;
  float                   start = 0.0;
  float                   end = 0.0;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, module, event);
    return FALSE;
  }

  cac = (cac47_t *)isp_sub_module->private_data;
  if (!cac) {
    ISP_ERR("failed: cac %p", cac);
    return FALSE;
  }

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: stats_update %p", stats_update);
    goto ERROR;
  }

  chromatix_cac =
    &chromatix_ptr->chromatix_VFE.chromatix_hw_CAC2_data;

  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
    start = 0.0;
    end = 0.0;
    cac_trigger =
      &(chromatix_cac->CAC2_data[i].CAC2_trigger);

    if (i == MAX_LIGHT_TYPES_FOR_SPATIAL - 1) {
      /* falls within region 6 but we do not use trigger points in the region */
      ratio = 0.0;
      trigger_index = MAX_LIGHT_TYPES_FOR_SPATIAL - 1;
      break;
    }

    if (chromatix_cac->control_cac2 == 0) {
      /* lux index based */
      aec_reference = stats_update->aec_update.lux_idx;
      start = cac_trigger->lux_index_start;
      end   = cac_trigger->lux_index_end;
      ISP_DBG("lux base, lux idx %f",aec_reference);
    } else {
      /* Gain based */
      aec_reference = stats_update->aec_update.real_gain;
      start = cac_trigger->gain_start;
      end   = cac_trigger->gain_end;
      ISP_DBG("gain base, gain %f",aec_reference);
    }

    /* index is within interpolation range, find ratio */
     if (aec_reference >= start && aec_reference < end) {
       ratio = (aec_reference - start)/(end - start);
       ISP_HIGH("%s [%f - %f - %f] = %f", __func__, start, aec_reference, end,
         ratio);
     }

     /* already scanned past the lux index */
     if (aec_reference < end) {
        trigger_index = i;
        break;
     }
  }

  ret = cac47_decide_hyster_point_enable(isp_sub_module, aec_reference);
  if (ret == FALSE) {
    ISP_ERR("cac47_decide_hyster_point_enable failed");
    goto ERROR;
  }

  ISP_DBG("decide trigger index = %d", i);
  if (trigger_index >= MAX_LIGHT_TYPES_FOR_SPATIAL) {
    ISP_ERR("no trigger match for ABF trigger value: %f\n", aec_reference);
    goto ERROR;
  }

  if ((trigger_index != cac->trigger_index) ||
    (!F_EQUAL((1 - ratio), cac->aec_ratio.ratio))) {
    cac->trigger_index = trigger_index;
    /*aec ratio store the ratio to the start, now the distance to the start,
      thus do 1 - ratio here*/
    cac->aec_ratio.ratio = 1 - ratio;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** cac47_decide_scaling_threshold:
 *
 *  @isp_sub_module: sub module handle
 *
 * Enable or disable CAC based on scaling threshold
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean cac47_decide_scaling_threshold(
  isp_sub_module_t *isp_sub_module, float scale_factor)
{
  boolean                  ret = TRUE;
  chromatix_parms_type    *chromatix_ptr = NULL;
  Chromatix_HW_CAC2_type  *chromatix_cac = NULL;
  cac47_t                 *cac_mod       = NULL;
  float                    scaling_threshold = 0.0;

  if (isp_sub_module == NULL) {
    ISP_ERR("failed: NULL pointer %p", isp_sub_module);
    return FALSE;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  /* get chroma enhance mod private*/
  cac_mod = (cac47_t *)isp_sub_module->private_data;
  if (!cac_mod) {
    ISP_ERR("failed: mod %p", cac_mod);
    return FALSE;
  }

  chromatix_cac =
    &chromatix_ptr->chromatix_VFE.chromatix_hw_CAC2_data;

  scaling_threshold = chromatix_cac->CAC2_scaling_threshold;

  // disable module if current down scaling value > threshold
  if (scale_factor > scaling_threshold) {
    if (isp_sub_module->submod_enable != FALSE) {
      isp_sub_module->submod_enable = FALSE;
      isp_sub_module->update_module_bit = TRUE;
      cac_mod->scaling_en = FALSE;
      ISP_HIGH("disable CAC by scaling threshold, CAC enable %d," \
        " scale factor %f, scaling threshold %f",
        isp_sub_module->submod_enable, scale_factor, scaling_threshold);
    }
  } else if (isp_sub_module->submod_enable !=
              isp_sub_module->chromatix_module_enable) {
    cac_mod->scaling_en = TRUE;
    if (cac_mod->hyster_en == TRUE) {
      isp_sub_module->submod_enable = isp_sub_module->chromatix_module_enable;
      isp_sub_module->update_module_bit = TRUE;
      ISP_HIGH("enable ON CAC by scaling threshold, CAC enable %d," \
        " scale factor %f, scaling threshold %f",
        isp_sub_module->submod_enable, scale_factor, scaling_threshold);
    }
  }

  return TRUE;
}

/** cac47_trigger_update:
 *
 *  @module: cac module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function checks and initiates triger update of module
 *
 *  Return:   TRUE - Success
 *            FALSE - Parameters size mismatch
 **/
boolean cac47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  isp_private_event_t     *private_event = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  cac47_t                 *cac_mod = NULL;
  isp_sub_module_output_t *sub_module_output = NULL;
  float                   scale_factor = 0.0;
  uint8_t                 module_enable = 0;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  /* get chroma enhance mod private*/
  cac_mod = (cac47_t *)isp_sub_module->private_data;
  if (!cac_mod) {
    ISP_ERR("failed: mod %p", cac_mod);
    return FALSE;
  }

  /* get chromatix pointer*/
  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return TRUE;
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


  if (cac_mod->cac_scaler_scale_factor_updated == TRUE) {
    scale_factor = cac47_calculate_scale_factor(cac_mod);
    ret = cac47_decide_scaling_threshold(isp_sub_module,
      scale_factor);
    if (ret == FALSE) {
      ISP_ERR("isp_sub_module_util_cac47_decide_scaling_threshold failed");
      return FALSE;
    }
    cac_mod->cac_scaler_scale_factor_updated = FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->manual_ctrls.manual_update &&
      isp_sub_module->chromatix_module_enable ) {
    module_enable = ((isp_sub_module->manual_ctrls.cac_mode ==
      CAM_COLOR_CORRECTION_ABERRATION_FAST) ||
      (isp_sub_module->manual_ctrls.cac_mode ==
       CAM_COLOR_CORRECTION_ABERRATION_HIGH_QUALITY))? TRUE : FALSE;

    isp_sub_module->manual_ctrls.manual_update = FALSE;
    if (isp_sub_module->submod_enable != module_enable) {
      isp_sub_module->submod_enable = module_enable;
      sub_module_output->stats_params->
        module_enable_info.reconfig_needed = TRUE;
      sub_module_output->stats_params->module_enable_info.
        submod_enable[isp_sub_module->hw_module_id] = module_enable;
      sub_module_output->stats_params->module_enable_info.
        submod_mask[isp_sub_module->hw_module_id] = 1;
    }
  }

  if (!isp_sub_module->submod_enable) {
      ISP_DBG("skip trigger update: enable = %d",
        isp_sub_module->submod_enable);
      goto FILL_META;
  }

  /*disable HW block if both bit set 0 from header */
  if (isp_sub_module->trigger_update_pending == TRUE &&
      isp_sub_module->submod_trigger_enable == TRUE) {
    ret = cac47_trigger_interpolate(isp_sub_module, cac_mod);
    if (ret == FALSE) {
      ISP_ERR("trigger_interpolate failed!");
    }
    ret = cac47_prepare_hw_config(isp_sub_module, cac_mod);
    if (ret == FALSE) {
      ISP_ERR("prepare hw cfg failed!");
    }
    ret = cac47_store_hw_update(isp_sub_module, cac_mod);
    if (ret == FALSE) {
      ISP_ERR("failed: cac47_do_hw_update");
    }
    isp_sub_module->trigger_update_pending = FALSE;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
    sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
  }

FILL_META:
  if (sub_module_output->frame_meta)
    sub_module_output->frame_meta->cac_mode =
      isp_sub_module->manual_ctrls.cac_mode;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
}

/** cac47_set_scaler_scale_factor:
 *
 *  @module: cac module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function gets the scaler crop factor and stores it
 *
 *  Return:   TRUE - Success
 *            FALSE - Parameters size mismatch
 **/
boolean cac47_set_scaler_scale_factor(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                      ret           = TRUE;
  isp_private_event_t         *private_event = NULL;
  cac47_t                     *cac_mod       = NULL;
  scaler_scale_factor_for_cac *cac_scaler_scale_factor = NULL;
  uint8_t                      entry_idx;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  cac_mod = (cac47_t *)isp_sub_module->private_data;
  if (!cac_mod) {
    ISP_ERR("failed: mod %p", cac_mod);
    return FALSE;
  }

  /*get the scaler crop factor*/
  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  cac_scaler_scale_factor = (scaler_scale_factor_for_cac *)private_event->data;
  if (!cac_scaler_scale_factor) {
    ISP_ERR("failed: output %p", cac_scaler_scale_factor);
    return FALSE;
  }

  entry_idx = cac_scaler_scale_factor->index;
  if (entry_idx >= ISP_HW_STREAM_MAX) {
    ISP_ERR("failed: entry_idx %d", entry_idx);
    return FALSE;
  }

  cac_mod->cac_scaler_scale_factor[entry_idx] = *cac_scaler_scale_factor;
  cac_mod->cac_scaler_scale_factor_updated = TRUE;

  return ret;
}

/** cac47_update_streaming_mode_mask:
 *
 *  @module: cac module instance
 *  @isp_sub_module: sub module handle
 *  @streaming_mode_mask: streaming mode mask
 *
 *  update stream mask
 *  Return none
 **/
void cac47_update_streaming_mode_mask(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, uint32_t streaming_mode_mask)
{
  cac47_t *cac = NULL;
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return;
  }

  cac = (cac47_t *)isp_sub_module->private_data;
  if (!cac) {
    ISP_ERR("failed: cac %p", cac);
    return;
  }
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  cac->streaming_mode_mask = streaming_mode_mask;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
}

/** cac47_module_cfg:
*
*  @module: cac47 module pointer
*  @isp_sub_module: isp_sub_module pointer
*  @event: event pointer
*
*  Checks if module needs to be enabled based on scaling threshold
*
*  create mct module for bpc
*
*  Return TRUE on success or FALSE on failure
**/
boolean cac47_module_cfg(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  cac47_t *cac_mod      = NULL;
  float    scale_factor = 0.0;
  int      i            = 0;
  boolean  ret          = TRUE;

  if (!module || !event || !isp_sub_module || !isp_sub_module->private_data) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  cac_mod = (cac47_t*)isp_sub_module->private_data;

  if (cac_mod->cac_scaler_scale_factor_updated == TRUE) {
    scale_factor = cac47_calculate_scale_factor(cac_mod);
    ret = cac47_decide_scaling_threshold(isp_sub_module, scale_factor);
    if (ret == FALSE) {
      ISP_ERR("cac47_decide_scaling_threshold failed");
      return FALSE;
    }
    cac_mod->cac_scaler_scale_factor_updated = FALSE;
    ret = isp_sub_module_util_get_cfg_mask(isp_sub_module, event);
  }

  return ret;
}

/** cac47_query_cap:
 *
 *    @module: mct module instance
 *    @query_buf: query caps buffer handle
 *
 *  @sessionid: session id
 *  Returns TRUE
 *
 **/
boolean cac47_query_cap(mct_module_t *module,
  void *query_buf)
{
  mct_pipeline_cap_t *cap_buf;
  mct_pipeline_isp_cap_t *mod_cap = NULL;

  if (module == NULL || query_buf == NULL) {
    ISP_ERR(" fatal: NULL_ptr %p %p", module, query_buf);
      return FALSE;
  }

  cap_buf = (mct_pipeline_cap_t *)query_buf;
  mod_cap = &cap_buf->isp_cap;
  mod_cap->aberration_modes_count = 3;
  mod_cap->aberration_modes[0] = CAM_COLOR_CORRECTION_ABERRATION_OFF;
  mod_cap->aberration_modes[1] = CAM_COLOR_CORRECTION_ABERRATION_FAST;
  mod_cap->aberration_modes[2] = CAM_COLOR_CORRECTION_ABERRATION_HIGH_QUALITY;
  return TRUE;
}

/** cac47_init:
 *
 *  @module: cac module handle
 *  @isp_sub_module: submodule data
 *
 *  This function instantiates a cac module
 *
 *  Return: boolean
 **/
boolean cac47_init(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  cac47_t *mod = NULL;

  ISP_HIGH("E");
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (cac47_t *)CAM_MALLOC(sizeof(cac47_t));
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  memset(mod, 0, sizeof(*mod));
  memset(&mod->RegCmd, 0, sizeof(mod->RegCmd));

  /* enable trigger update feature flag from PIX */
  isp_sub_module->submod_trigger_enable = TRUE;
  mod->trigger_index = CAC47_DEFAULT_TRIGGER_INDEX;
  mod->aec_ratio.ratio = 1.0;

  mod->hyster_en  = true;
  mod->scaling_en = true;

  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;

  isp_sub_module->private_data = (void *)mod;

  return TRUE;
}

/** cac47_destroy:
 *
 *  @module: cac module handle
 *  @isp_sub_module: submodule data
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void cac47_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  cac47_t *mod = NULL;

  ISP_HIGH("E");

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
}
