/* hdr_bhist_stats_ext48.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* isp headers */
#include "bhist_stats44.h"
#include "isp_log.h"
#include "isp_sub_module_util.h"

boolean bhist_stats_ext44_hw_update(isp_sub_module_t *isp_sub_module,
  bhist_stats44_t *bhist_stats)
{
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  ISP_Stats_CfgType            bhist_cfg_mask, bhist_cfg_val;

  if (!isp_sub_module || !bhist_stats) {
    ISP_ERR("failed: %p %p", isp_sub_module, bhist_stats);
    return FALSE;
  }

  /* Create VFE mask */
  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_CALLOC(sizeof(*hw_update), 1);
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }

  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_CALLOC(sizeof(*reg_cfg_cmd),
    1);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    ret = FALSE;
  }

  if (ret == TRUE) {
    cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
    cfg_cmd->num_cfg = 1;

    /* Initialize mask and val */
    bhist_cfg_mask.mask = 0;
    bhist_cfg_val.mask = 0;

    /* Set both bits in mask */
    bhist_cfg_mask.hdr_bhist_field_sel = 0x3;
    bhist_cfg_mask.hdr_bhist_channel_sel = 1;

    /* Fill val */
    bhist_cfg_val.hdr_bhist_field_sel = 0x0; /* All lines */
    bhist_cfg_val.hdr_bhist_channel_sel = 0x0; /* Gr */

    reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;
    reg_cfg_cmd[0].u.mask_info.reg_offset = BHIST_STATS_CFG_OFF;
    reg_cfg_cmd[0].u.mask_info.mask = bhist_cfg_mask.mask;
    reg_cfg_cmd[0].u.mask_info.val = bhist_cfg_val.mask;

    ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
    ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    }
  }

  if (ret == FALSE) {
    free(reg_cfg_cmd);
    free(hw_update);
  }

  return ret;
}
