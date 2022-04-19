/* bf_down_scaler.c
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* isp headers */
#include "bf_down_scaler.h"
#include "isp_sub_module_util.h"
#include "isp_log.h"
#include "isp_defs.h"

#ifdef BF_DOWN_SCALER_DEBUG
#undef ISP_DBG
#define ISP_DBG ISP_ERR

#undef ISP_HIGH
#define ISP_HIGH ISP_ERR
#endif

/** bf_down_scaler_debug:
 *    @pcmd: Pointer to the reg_cmd struct that needs to be dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 *
 * Return void
 **/
static void bf_down_scaler_debug(bf_down_scaler_t *bf_down_scaler,
  bf_down_scaler_reg_cfg_t *pcmd)
{
   if (!bf_down_scaler || !pcmd) {
     ISP_ERR("failed: bf_down_scaler %p pcmd %p", bf_down_scaler, pcmd);
     return;
   }
  ISP_DBG("Bayer Focus Stats Scale Configurations");
  /* Print debug cmd */
  ISP_DBG("bf_scale_cfg mask:val en=%d:%d",
    bf_down_scaler->enable_mask.scale_en,
    bf_down_scaler->enable_val.scale_en);
  ISP_DBG("bf_scale_cfg hin 0x%x hout 0x%x h_interp_reso 0x%x",
    pcmd->h_in, pcmd->h_out, pcmd->h_interp_reso);
  ISP_DBG("bf_scale_cfg h_phase_mult 0x%x h_mn_init 0x%x h_phase_init 0x%x",
    pcmd->h_phase_mult, pcmd->h_mn_init, pcmd->h_phase_init);
  ISP_DBG("bf_scale_cfg h_skip_cnt 0x%x scale_y_in_width 0x%x",
    pcmd->h_skip_cnt, pcmd->scale_y_in_width);
}

/** bf_down_scaler_calculate_phase
 *    @M:       output width
 *    @N:       input width
 *    @offset:  offset
 *    @interp_reso: actual input width
 *    @mn_init: offset of given mn ratio
 *    @phase_init: initial phase
 *    @phase_mult: phase scaling
 *
 * This function calculates initial phase for scaler
 *
 * Return: nothing
 **/
void bf_down_scaler_calculate_phase(uint32_t  M, uint32_t  N,
  uint32_t  offset, uint32_t *interp_reso, uint32_t *mn_init,
  uint32_t *phase_init, uint32_t *phase_mult)
{
  uint32_t ratio = 0;
  *interp_reso = 3;

  if (M == 0) {
    ISP_ERR("failed, devide by 0 check!  h_out/v_out = %d", M);
    return;
  }

  ratio = N / M;
  if (ratio >= 16)     *interp_reso = 0;
  else if (ratio >= 8) *interp_reso = 1;
  else if (ratio >= 4) *interp_reso = 2;

  *mn_init = offset * M % N;
  *phase_init = (*mn_init << (PHASE_ADDER + *interp_reso)) / M;
  *phase_mult = (N << (PHASE_ADDER + *interp_reso)) / M;
}

/** bf_down_scaler_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bf_down_scaler: bf stats handle
 *
 *  Create hw update list and store it in isp_sub_module handle
 *
 *  Return TRUE on success and FALSE on failure
 **/
static int bf_down_scaler_store_hw_update(isp_sub_module_t *isp_sub_module,
  bf_down_scaler_t *bf_down_scaler)
{
  boolean                        ret = TRUE;
  bf_down_scaler_reg_cfg_t      *pcmd = NULL;
  struct msm_vfe_cfg_cmd2       *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd    *reg_cfg_cmd = NULL;
  bf_down_scaler_reg_cfg_t      *copy_cmd = NULL;
  struct msm_vfe_cfg_cmd_list   *hw_update = NULL;
  uint32_t                      *data = NULL;
  uint32_t                       len = 0;
  uint32_t                       num_reg_cfg_cmd = 0;

  if (!isp_sub_module || !bf_down_scaler) {
    ISP_ERR("failed: %p %p", isp_sub_module, bf_down_scaler);
    return FALSE;
  }
  pcmd = &bf_down_scaler->pcmd;
  bf_down_scaler_debug(bf_down_scaler, pcmd);

  if (bf_down_scaler->enable_val.scale_en) {
    num_reg_cfg_cmd = 2;
  } else {
    num_reg_cfg_cmd = 1;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_CALLOC(sizeof(*hw_update), 1);
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_CALLOC(sizeof(*reg_cfg_cmd) *
    num_reg_cfg_cmd, 1);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }

  cfg_cmd = &hw_update->cfg_cmd;
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = num_reg_cfg_cmd;

  reg_cfg_cmd[0].u.mask_info.reg_offset = BF_DOWN_SCALER_ENABLE_ADDR;
  reg_cfg_cmd[0].u.mask_info.mask       = bf_down_scaler->enable_mask.scale_en;
  reg_cfg_cmd[0].u.mask_info.val        = bf_down_scaler->enable_val.scale_en;
  reg_cfg_cmd[0].cmd_type               = VFE_CFG_MASK;

  if (bf_down_scaler->enable_val.scale_en) {
     copy_cmd = (bf_down_scaler_reg_cfg_t *)CAM_CALLOC(sizeof(*copy_cmd), 1);
     if (!copy_cmd) {
       ISP_ERR("failed: copy_cmd %p", copy_cmd);
       goto ERROR_COPY_CMD;
     }

     *copy_cmd = *pcmd;

     cfg_cmd->cfg_data = (void *)copy_cmd;
     cfg_cmd->cmd_len = sizeof(*copy_cmd);
     reg_cfg_cmd[1].u.rw_info.cmd_data_offset = 0;
     reg_cfg_cmd[1].cmd_type = VFE_WRITE;
     reg_cfg_cmd[1].u.rw_info.reg_offset = BF_DOWN_SCALER_CONFIG_ADDR;
     reg_cfg_cmd[1].u.rw_info.len = sizeof(bf_down_scaler->pcmd);

#ifdef PRINT_REG_VAL_SET
     ISP_ERR("hw_reg_offset %x, len %d", reg_cfg_cmd[0].u.rw_info.reg_offset,
       reg_cfg_cmd[0].u.rw_info.len);
     data = (uint32_t *)cfg_cmd->cfg_data;
     for (len = 0; len < (cfg_cmd->cmd_len / 4); len++) {
       ISP_ERR("data[%d] %x", len, data[len]);
     }
#endif
  }

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;
  }

  return TRUE;

ERROR_APPEND:
  free(copy_cmd);
ERROR_COPY_CMD:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD:
  free(hw_update);
  return FALSE;
}

/** bf_down_scaler_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bf_down_scaler: bf down scaler handle
 *  @sub_module_output: sub module output handle
 *
 *  Configure the bf down scaler registers
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_down_scaler_trigger_update(isp_sub_module_t *isp_sub_module,
  bf_down_scaler_t *bf_down_scaler, isp_sub_module_output_t *sub_module_output)
{
  boolean                        ret = TRUE;
  bf_down_scaler_reg_cfg_t      *pcmd = NULL;
  uint32_t                       interp_reso = 0,
                                 mn_init = 0,
                                 phase_init = 0,
                                 phase_mult = 0;
  bf_scale_cfg_t                *bf_scale_cfg = NULL;

  if (!isp_sub_module || !bf_down_scaler || !sub_module_output) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, bf_down_scaler,
      sub_module_output);
    return FALSE;
  }

  if (!isp_sub_module->stream_on_count ||
    (bf_down_scaler->trigger_update_pending == FALSE)) {
    return TRUE;
  }

  pcmd = &bf_down_scaler->pcmd;
  bf_scale_cfg = &bf_down_scaler->bf_scale_cfg;

  /* Validate input */
  if ((bf_scale_cfg->bf_scale_en == TRUE) && (!bf_scale_cfg->scale_n ||
    (bf_scale_cfg->scale_n <= bf_scale_cfg->scale_m))) {
     ISP_ERR("invalid scale M %d scale N %d", bf_scale_cfg->scale_m,
       bf_scale_cfg->scale_n);
     bf_scale_cfg->scale_m = bf_scale_cfg->scale_n;
  }

  /* Read scaling ratio from bf_scale_config*/
  if ((bf_scale_cfg->bf_scale_en == FALSE) || !bf_scale_cfg->scale_n) {
     bf_down_scaler->enable_mask.scale_en = 0x1;
     bf_down_scaler->enable_val.scale_en = 0x0;
  } else {
     bf_down_scaler->enable_mask.scale_en = 0x1;
     bf_down_scaler->enable_val.scale_en = 0x1;

     pcmd->h_in = MIN(ISP_STATS_BF_SCALE_MAX_H_CFG,
       bf_down_scaler->camif_width);
     ISP_DBG("h_in %d ISP_STATS_BF_SCALE_MAX_H_CFG %d camif_width %d",
       pcmd->h_in, ISP_STATS_BF_SCALE_MAX_H_CFG, bf_down_scaler->camif_width);
     pcmd->h_out = (pcmd->h_in * bf_scale_cfg->scale_m) / bf_scale_cfg->scale_n;
     ISP_DBG("h_out %d h_in %d scale_m %d scale_n %d", pcmd->h_out,
       pcmd->h_in, bf_scale_cfg->scale_m, bf_scale_cfg->scale_n);

     bf_down_scaler_calculate_phase(pcmd->h_out, pcmd->h_in, 0,
       &interp_reso, &mn_init, &phase_init, &phase_mult);
     pcmd->h_interp_reso = interp_reso;
     pcmd->h_phase_mult = phase_mult;
     /* Change to 0 for left */
     pcmd->h_mn_init = mn_init;
     pcmd->h_phase_init = phase_init;
     /* Hardcode to 0, will be taken care with dual vfe changes */
     pcmd->h_skip_cnt = 0;
     pcmd->scale_y_in_width = pcmd->h_in;
  }

  ret = bf_down_scaler_store_hw_update(isp_sub_module, bf_down_scaler);
  if (ret == FALSE) {
    ISP_ERR("failed: bf_down_scaler_store_hw_update");
    goto ERROR;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
    sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR;
  }

  bf_down_scaler->trigger_update_pending = FALSE;
  return TRUE;

ERROR:
  return FALSE;
}

/** bf_down_scaler_stats_config_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: stats data with new config
 *
 *  Handle stats config update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_down_scaler_stats_config_update(isp_sub_module_t *isp_sub_module,
  bf_down_scaler_t *bf_down_scaler, bf_fw_config_t *bf_fw)
{
  boolean         ret = TRUE;
  bf_scale_cfg_t *bf_scale_cfg = NULL;

  if (!isp_sub_module || !bf_down_scaler || !bf_fw) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, bf_down_scaler, bf_fw);
    return FALSE;
  }

  bf_scale_cfg = &bf_fw->bf_scale_cfg;
  if (bf_scale_cfg->is_valid == TRUE) {
     /* Validate input */
     if ((bf_scale_cfg->bf_scale_en == TRUE) && (!bf_scale_cfg->scale_n ||
       (bf_scale_cfg->scale_n <= bf_scale_cfg->scale_m))) {
       ISP_ERR("invalid scale M %d scale N %d", bf_scale_cfg->scale_m,
         bf_scale_cfg->scale_n);
       ret = FALSE;
     }
     if (ret == TRUE) {
        memcpy(&bf_down_scaler->bf_scale_cfg, bf_scale_cfg,
          sizeof(bf_scale_cfg_t));
       bf_down_scaler->trigger_update_pending = TRUE;
     }
  }

  return ret;
} /* bf_down_scaler_stats_config_update */

/** bf_down_scaler_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bf_down_scaler: handle to bf down scaler
 *  @sensor_out_info: handle to payload of set_stream_config
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bf_down_scaler_set_stream_config(isp_sub_module_t *isp_sub_module,
  bf_down_scaler_t *bf_down_scaler, sensor_out_info_t *sensor_out_info)
{
  bf_scale_cfg_t *bf_scale_cfg = NULL;

  if (!isp_sub_module || !bf_down_scaler || !sensor_out_info) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, bf_down_scaler,
      sensor_out_info);
    return FALSE;
  }

  bf_down_scaler->sensor_out_info = *sensor_out_info;
  bf_down_scaler->camif_width = (sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1);
  ISP_DBG("bf_scale_cfg camif w %d fp %d lp %d", bf_down_scaler->camif_width,
    sensor_out_info->request_crop.first_pixel,
    sensor_out_info->request_crop.last_pixel);
  bf_down_scaler->camif_height = (sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1);
  ISP_DBG("bf_scale_cfg camif h %d fl %d ll %d", bf_down_scaler->camif_height,
    sensor_out_info->request_crop.first_line,
    sensor_out_info->request_crop.last_line);

  bf_scale_cfg = &bf_down_scaler->bf_scale_cfg;

  /* Initialize default cfg */
  bf_scale_cfg->is_valid = TRUE;
  bf_scale_cfg->bf_scale_en = TRUE;
  bf_scale_cfg->scale_m = 1;
  bf_scale_cfg->scale_n = 2;

  bf_down_scaler->trigger_update_pending = TRUE;

  ISP_DBG("bf_scale_cfg en %d m %d n %d", bf_scale_cfg->bf_scale_en,
    bf_scale_cfg->scale_m, bf_scale_cfg->scale_n);
  return TRUE;
} /* bf_down_scaler_set_stream_config */


/** bf_down_scaler_get_downscaler_params:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bf_down_scaler: handle to bf down scaler
 *  @bf_out_scale_cfg: bf scale cfg to return
 *
 *  Return bf scale cfg
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bf_down_scaler_get_downscaler_params(isp_sub_module_t *isp_sub_module,
  bf_down_scaler_t *bf_down_scaler, bf_scale_cfg_t *bf_out_scale_cfg)
{
  bf_scale_cfg_t *bf_in_scale_cfg = NULL;

  if (!isp_sub_module || !bf_down_scaler || !bf_out_scale_cfg) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, bf_down_scaler,
      bf_out_scale_cfg);
    return FALSE;
  }

  bf_out_scale_cfg->is_valid = TRUE;

  bf_in_scale_cfg = &bf_down_scaler->bf_scale_cfg;

  if (bf_in_scale_cfg->is_valid == TRUE) {
    *bf_out_scale_cfg = *bf_in_scale_cfg;
  } else {
     bf_out_scale_cfg->bf_scale_en = FALSE;
  }

  return TRUE;
} /* bf_down_scaler_get_downscaler_params */
