/* aec_bg_stats_ext48.c
 *
 * Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* isp headers */
#include "aec_bg_stats47.h"
#include "isp_sub_module_util.h"
#include "isp_log.h"
#include "isp_defs.h"
#include "isp_pipeline_reg.h"
#include "aec_bg_stats_reg.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

boolean aec_bg_stats_ext47_hw_update(isp_sub_module_t *isp_sub_module,
  aec_bg_stats47_t *aec_bg_stats) {
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  ISP_Stats_aec_bg_CfgCmdType *copy_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  uint32_t                     num_reg_cmd = 1;
  uint32_t                     setprop_enable = 0;
  char                         value[PROPERTY_VALUE_MAX];
  if (!isp_sub_module || !aec_bg_stats) {
    ISP_ERR("failed: %p %p", isp_sub_module, aec_bg_stats);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(sizeof(*reg_cfg_cmd)*num_reg_cmd);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd)*num_reg_cmd);

  copy_cmd = (ISP_Stats_aec_bg_CfgCmdType *)CAM_MALLOC(sizeof(*copy_cmd));
  if (!copy_cmd) {
    ISP_ERR("failed: copy_cmd %p", copy_cmd);
    goto ERROR_COPY_CMD;
  }
  memset(copy_cmd, 0, sizeof(*copy_cmd));

#ifdef _ANDROID_
  property_get("persist.vendor.camera.saturationext", value, "0");
  setprop_enable = atoi(value);
  if (setprop_enable) {
    copy_cmd->sat_stats_en = 1;
  } else {
    copy_cmd->sat_stats_en = 0;
  }
#else
  copy_cmd->sat_stats_en = 0;
#endif

  copy_cmd->shift_bits = 0;
  copy_cmd->rgn_sample_pattern =     isp_sub_module_util_get_rgn_sample_pattern(
    isp_sub_module->rgn_skip_pattern);

  cfg_cmd = &hw_update->cfg_cmd;
  cfg_cmd->cfg_data = (void *)copy_cmd;
  cfg_cmd->cmd_len = sizeof(*copy_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = num_reg_cmd;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = STATS_AEC_BG_CFG;
  reg_cfg_cmd[0].u.rw_info.len = STATS_AEC_BG_CFG_LEN * sizeof(uint32_t);

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
