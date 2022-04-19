/* bf_stats_ext48.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* isp headers */
#include "bf_stats47.h"
#include "isp_log.h"
#include "isp_sub_module_util.h"

static boolean bf_stats_ext48_fill_filter_params(bf_stats47_t *bf_stats)
{
  bf_stats_filter_cfg1_t *bf_stats_filter_cfg1 = NULL;
  bf_stats_v_iir_cfg1_t  *bf_stats_v_iir_cfg1 = NULL;
  bf_stats_v_iir_cfg_t   *bf_stats_v_iir_cfg = NULL;
  bf_iir_filter_cfg_t    *bf_iir_filter_cfg = NULL;
  bf_stats_reg_cfg_t     *bf_stats_reg_cfg = NULL;

  if (!bf_stats || !bf_stats->ext_data) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    return FALSE;
  }

  bf_stats_filter_cfg1 = (bf_stats_filter_cfg1_t *)bf_stats->ext_data;
  bf_stats_v_iir_cfg1 = &bf_stats_filter_cfg1->bf_stats_v_iir_cfg1;

  bf_stats_reg_cfg = &bf_stats->pcmd;
  bf_stats_v_iir_cfg = &bf_stats_reg_cfg->bf_stats_filter_cfg.bf_stats_v_iir_cfg;
  bf_iir_filter_cfg = &bf_stats->af_config.bf_fw.
    bf_filter_cfg[BF_FILTER_TYPE_V].bf_iir_filter_cfg;

  bf_stats_v_iir_cfg1->b20 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    bf_iir_filter_cfg->b20), IIR_BITS);
  bf_stats_v_iir_cfg1->b21 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    bf_iir_filter_cfg->b21), IIR_BITS);
  bf_stats_v_iir_cfg->b22 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    bf_iir_filter_cfg->b22), IIR_BITS);
  bf_stats_v_iir_cfg1->a21 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    bf_iir_filter_cfg->a21), IIR_BITS);
  bf_stats_v_iir_cfg1->a22 = bf_stats47_adjust_signbit(FLOAT_TO_Q(14,
    bf_iir_filter_cfg->a22), IIR_BITS);

  return TRUE;
}

static boolean bf_stats_ext48_fill_roi_params(bf_stats47_t *bf_stats)
{
  bf_stats_filter_cfg1_t   *bf_stats_filter_cfg1 = NULL;
  bf_stats_active_window_t *bf_stats_active_window = NULL;

  if (!bf_stats || !bf_stats->ext_data) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    return FALSE;
  }

  bf_stats_filter_cfg1 = (bf_stats_filter_cfg1_t *)bf_stats->ext_data;
  bf_stats_active_window = &bf_stats_filter_cfg1->bf_stats_active_window;

  bf_stats_active_window->x_min = bf_stats->x_min & 0x1FFF;
  bf_stats_active_window->y_min = bf_stats->y_min & 0x3FFF;
  bf_stats_active_window->x_max = bf_stats->x_max & 0x1FFF;
  bf_stats_active_window->y_max = bf_stats->y_max & 0x3FFF;

  return TRUE;
}

static void bf_stats_ext48_debug_filter_config(
  bf_stats_filter_cfg1_t *bf_stats_filter_cfg)
{
  bf_stats_v_iir_cfg1_t      *bf_stats_v_iir_cfg1 = NULL;

  if (!bf_stats_filter_cfg) {
    ISP_ERR("failed bf_stats_filter_cfg %p", bf_stats_filter_cfg);
    return;
  }
  /* V IIR cfg 1*/
  bf_stats_v_iir_cfg1 = &bf_stats_filter_cfg->bf_stats_v_iir_cfg1;
    ISP_DBG("v iir b20 = 0x%x b21 = 0x%x a21 = 0x%x a22 = 0x%x",
    bf_stats_v_iir_cfg1->b20, bf_stats_v_iir_cfg1->b21,
    bf_stats_v_iir_cfg1->a21, bf_stats_v_iir_cfg1->a22);
}

static boolean bf_stats_ext48_write_ext_params(isp_sub_module_t *isp_sub_module,
  bf_stats47_t *bf_stats)
{
  boolean                      ret = TRUE;
  bf_stats_filter_cfg1_t      *bf_stats_filter_cfg = NULL;
  bf_stats_filter_cfg1_t      *copy_bf_stats_filter_cfg = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;

  if (!isp_sub_module || !bf_stats) {
    ISP_ERR("failed: %p %p", isp_sub_module, bf_stats);
    return FALSE;
  }

  bf_stats_filter_cfg = (bf_stats_filter_cfg1_t *)bf_stats->ext_data;
  bf_stats_ext48_debug_filter_config(bf_stats_filter_cfg);

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_CALLOC(sizeof(*hw_update), 1);
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_CALLOC(sizeof(*reg_cfg_cmd), 1);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }

  copy_bf_stats_filter_cfg =
    (bf_stats_filter_cfg1_t *)CAM_CALLOC(sizeof(*copy_bf_stats_filter_cfg), 1);
  if (!copy_bf_stats_filter_cfg) {
    ISP_ERR("failed: copy_bf_stats_filter_cfg %p", copy_bf_stats_filter_cfg);
    goto ERROR_COPY_CMD;
  }

  *copy_bf_stats_filter_cfg = *bf_stats_filter_cfg;
  cfg_cmd = &hw_update->cfg_cmd;
  cfg_cmd->cfg_data = (void *)copy_bf_stats_filter_cfg;
  cfg_cmd->cmd_len = sizeof(*copy_bf_stats_filter_cfg);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd->cmd_type = VFE_WRITE;
  reg_cfg_cmd->u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd->u.rw_info.reg_offset = BF_STATS_OFF_1;
  reg_cfg_cmd->u.rw_info.len = sizeof(*copy_bf_stats_filter_cfg);

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;
  }

  return TRUE;

ERROR_APPEND:
  free(copy_bf_stats_filter_cfg);
ERROR_COPY_CMD:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD:
  free(hw_update);
  return FALSE;
}

static void bf_stats_ext48_destroy(bf_stats47_t *bf_stats)
{
  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    return;
  }

  free(bf_stats->ext_func_table);
  free(bf_stats->ext_data);

  return;
} /* bf_stats_ext48_destroy */

boolean bf_stats_ext47_init(bf_stats47_t *bf_stats)
{
  boolean                       ret = TRUE;
  bf_stats47_ext_func_table_t  *ext_func_table = NULL;

  if (!bf_stats) {
    ISP_ERR("failed: bf_stats %p", bf_stats);
    return FALSE;
  }

  bf_stats->ext_data = CAM_CALLOC(sizeof(bf_stats_filter_cfg1_t), 1);
  if (!bf_stats->ext_data) {
    ISP_ERR("failed: bf_stats->ext_data %p", bf_stats->ext_data);
    return FALSE;
  }

  ext_func_table = (bf_stats47_ext_func_table_t *)
    CAM_CALLOC(sizeof(bf_stats47_ext_func_table_t), 1);
  if (!ext_func_table) {
    ISP_ERR("failed: ext_func_table %p", ext_func_table);
    free(ext_func_table);
    return FALSE;
  }

  ext_func_table->bf_stats47_ext_fill_filter_params =
    bf_stats_ext48_fill_filter_params;
  ext_func_table->bf_stats47_ext_fill_roi_params =
    bf_stats_ext48_fill_roi_params;
  ext_func_table->bf_stats47_ext_write_ext_params =
    bf_stats_ext48_write_ext_params;
  ext_func_table->bf_stats47_ext_destroy =
    bf_stats_ext48_destroy;
  bf_stats->ext_func_table = (void *)ext_func_table;

  return TRUE;
}/* bf_stats_ext47_init */
