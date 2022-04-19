/* bf_gamma.c
 *
 * Copyright (c) 2014-2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* isp headers */
#include "bf_gamma.h"
#include "isp_sub_module_util.h"
#include "isp_common.h"
#include "isp_log.h"
#include "isp_defs.h"
#include "isp_pipeline_reg.h"
#include "bf_stats_reg.h"

/*#define GAMMA_DEBUG */
#ifdef GAMMA_DEBUG
#undef ISP_DBG
#define ISP_DBG ISP_ERR
#undef ISP_HIGH
#define ISP_HIGH ISP_ERR
#endif

#define CLAMP(a,b,c) (((a) <= (b))? (b) : (((a) >= (c))? (c) : (a)))

/** bf_gamma_debug_dmi
 *
 *  @lut: table
 *
 *  Print hw table
 */

static void bf_gamma_debug_dmi(uint32_t *lut __unused)
{
  int32_t       i = 0;
  for (i = 0; i <BF_GAMMA_ENTRIES; i++) {
    ISP_DBG("LUT[%d]: %d", i, lut[i]);
  }
}

/** bf_gamma_get_hi_lo_gamma_bits
 *
 *  @table: gamma table
 *  @i: index in gamma table
 *
 *  The higher 12 bits in the configuration contains the delta between the
 *  current GammaTable value and the next value, while the lower 12 bits
 *  contains the current GammaTable value
 *
 *  Return DMI value
 */

static uint32_t bf_gamma_get_hi_lo_gamma_bits(uint32_t *table,
  int i)
{
  int32_t       hw_lut_entry = 0;
  int16_t       delta_lut = 0;

  if (!table || (i >= BF_GAMMA_ENTRIES)) {
    ISP_ERR("failed: table %p i %d max %d", table, i, BF_GAMMA_ENTRIES);
    return FALSE;
  }
  delta_lut = (table[(i+1)] - table[i]);
  delta_lut = CLAMP(delta_lut, BAF_GAMMA_DELTA_MIN_VAL, BAF_GAMMA_DELTA_MAX_VAL);
  hw_lut_entry = (int32_t)((delta_lut << BAF_GAMMA_DMI_DELTA_SHIFT) +
    (table[i]));

  return hw_lut_entry;
}

/** bf_gamma_get_last_gamma_value
 *
 *  @table: gamma table
 *
 *  Return last gamma DMI value
 */
static uint32_t bf_gamma_get_last_gamma_value(uint32_t *table)
{
  int32_t       hw_lut_entry = 0;
  int16_t       delta_lut = 0;

  /* consider the next entry of the last entry 2^12- 8084.
   *                                            2^14- 8994.
   * value suggested by system team
   */
  uint32_t next_entry_final =
    (uint32_t) isp_sub_module_util_power(2.0, BAF_GAMMA_DMI_DELTA_SHIFT);

  /* this is effectively  table[255] - table[254];
   * this part is the delta
   */
  /* use 256 as next entry of last entry */
  delta_lut = next_entry_final -
    (table[(BF_GAMMA_ENTRIES - 1)]);
  delta_lut = CLAMP(delta_lut, BAF_GAMMA_DELTA_MIN_VAL, BAF_GAMMA_DELTA_MAX_VAL);
  /* scale the delta */

  /* form the value:  upper byte is delta,
   * lower byte is the entry itself
   */
  hw_lut_entry = (int32_t)(delta_lut <<  BAF_GAMMA_DMI_DELTA_SHIFT) +
    (table[(BF_GAMMA_ENTRIES - 1)]);

  return hw_lut_entry;
}

/** bf_gamma_trigger_update:
 *
 *  @isp_sub_module: handle for isp_sub_module
 *  @bf_gamma_cfg: handle for bf_gamma_cfg
 *  @bf_fw: bf_fw config from 3A
 *
 *  Trigger update and store in hw update list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_gamma_trigger_update(isp_sub_module_t *isp_sub_module,
  bf_gamma_cfg_t *bf_gamma_cfg, isp_sub_module_output_t *sub_module_output,
  uint32_t downscale_factor)
{
  boolean                      ret = TRUE;
  uint32_t                     i = 0,
                              *val = NULL;
  bf_gamma_lut_cfg_t          *bf_gamma_lut_cfg = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  uint32_t                     temp_gamma[BF_GAMMA_ENTRIES];
  isp_meta_entry_t            *baf_gamma_dmi_info = NULL;

  if (!isp_sub_module || !bf_gamma_cfg || !sub_module_output) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, bf_gamma_cfg,
      sub_module_output);
    return FALSE;
  }

  if ((bf_gamma_cfg->trigger_update_pending == FALSE) &&
    (isp_sub_module->config_pending == FALSE)) {
    return TRUE;
  }

  if (isp_sub_module->config_pending == TRUE) {
    /* First time config, start with LUT BANK 0 */
    bf_gamma_cfg->bf_stats_enable_mask.gamma_lut_bank_sel = 0x1;
    bf_gamma_cfg->bf_stats_enable_val.gamma_lut_bank_sel = 0x0;
    isp_sub_module->config_pending = FALSE;
  }

  bf_gamma_lut_cfg = &bf_gamma_cfg->bf_gamma_lut_cfg;
  bf_gamma_cfg->bf_stats_enable_val.gamma_lut_en = bf_gamma_lut_cfg->is_valid;

  /* Update DMI only if gamma module is enabled */
  if (bf_gamma_cfg->bf_stats_enable_val.gamma_lut_en) {
     bf_gamma_lut_cfg = &bf_gamma_cfg->bf_gamma_lut_cfg;
     for (i = 0;
       i < (bf_gamma_lut_cfg->num_gamm_lut)/downscale_factor; i++) {
       temp_gamma[i] = CLAMP(bf_gamma_lut_cfg->gamma_lut[i * downscale_factor],
         0, BAF_GAMMA_MAX_VAL);
     }

     for (i; i < (bf_gamma_lut_cfg->num_gamm_lut); i++) {
       /* BAF gamma values are 14bit unsigned */
       temp_gamma[i] = BAF_GAMMA_MAX_VAL;
     }

     for (i = 0; i < (bf_gamma_lut_cfg->num_gamm_lut - 1); i++) {
       bf_gamma_cfg->lut[i] = bf_gamma_get_hi_lo_gamma_bits(
         temp_gamma, i);
     }

     bf_gamma_cfg->lut[i] = bf_gamma_get_last_gamma_value(
       temp_gamma);

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
       bf_gamma_debug_dmi(bf_gamma_cfg->lut);

       if (BF_CGC_OVERRIDE == TRUE) {
         ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
           BF_CGC_OVERRIDE_REGISTER, BF_CGC_OVERRIDE_BIT, TRUE);
         if (ret == FALSE) {
           ISP_ERR("failed: enable cgc");
         }
       }

       ret = isp_sub_module_util_write_dmi((void*)bf_gamma_cfg->lut,
         sizeof(uint32_t) * BF_GAMMA_ENTRIES, STATS_BAF_GAMMA_LUT_BANK0 +
         bf_gamma_cfg->bf_stats_enable_val.gamma_lut_bank_sel,
         VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
       if (ret == FALSE) {
         ISP_ERR("failed: isp_sub_module_util_write_dmi");
         ret = FALSE;
       }

       if (BF_CGC_OVERRIDE == TRUE) {
         ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
           BF_CGC_OVERRIDE_REGISTER, BF_CGC_OVERRIDE_BIT, FALSE);
         if (ret == FALSE) {
           ISP_ERR("failed: disable cgc");
         }
       }

      cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
      cfg_cmd->num_cfg = 1;

      reg_cfg_cmd[0].cmd_type = VFE_CFG_MASK;
      reg_cfg_cmd[0].u.mask_info.reg_offset = BAF_GAMMA_STATS_CFG_ADDR;
      val = (uint32_t *)&bf_gamma_cfg->bf_stats_enable_mask;
      reg_cfg_cmd[0].u.mask_info.mask = *val;
      val = (uint32_t *)&bf_gamma_cfg->bf_stats_enable_val;
      reg_cfg_cmd[0].u.mask_info.val = *val;

      ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
      ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_sub_module_util_store_hw_update");
      }

      /* Switch DMI BANK */
      bf_gamma_cfg->bf_stats_enable_mask.gamma_lut_bank_sel = 1;
      bf_gamma_cfg->bf_stats_enable_val.gamma_lut_bank_sel ^= 1;

    }

    if (sub_module_output->meta_dump_params &&
      sub_module_output->metadata_dump_enable == 1) {
      /*fill in DMI info*/
      baf_gamma_dmi_info = &sub_module_output->
        meta_dump_params->meta_entry[ISP_META_STATS_BF_GAMMA_LUT];
      /*dmi table length*/
      baf_gamma_dmi_info->len =
        sizeof(uint32_t) * BF_GAMMA_ENTRIES;
      /*dmi type */
      baf_gamma_dmi_info->dump_type = ISP_META_STATS_BF_GAMMA_LUT;
      baf_gamma_dmi_info->start_addr = 0;
      sub_module_output->meta_dump_params->frame_meta.num_entry++;

      memcpy(baf_gamma_dmi_info->isp_meta_dump,
        &bf_gamma_cfg->lut, baf_gamma_dmi_info->len);
    }

    if (ret == FALSE) {
      free(reg_cfg_cmd);
      free(hw_update);
    }
  }

  bf_gamma_cfg->trigger_update_pending = FALSE;

  return ret;
}

/** bf_gamma_stats_config_update:
 *
 *  @isp_sub_module: handle for isp_sub_module
 *  @bf_gamma_cfg: handle for bf_gamma_cfg
 *  @bf_fw: bf_fw config from 3A
 *
 *  Copy bf gamma config
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_gamma_stats_config_update(isp_sub_module_t *isp_sub_module,
  bf_gamma_cfg_t *bf_gamma_cfg, bf_fw_config_t *bf_fw)
{
  boolean              ret = TRUE;
  bf_gamma_lut_cfg_t  *bf_gamma_lut_cfg = NULL;

  if (!isp_sub_module || !bf_gamma_cfg || !bf_fw) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, bf_gamma_cfg, bf_fw);
    return FALSE;
  }

  bf_gamma_lut_cfg = &bf_fw->bf_gamma_lut_cfg;
  if (bf_gamma_lut_cfg->is_valid == FALSE) {
    return TRUE;
  }

  if (bf_gamma_lut_cfg->num_gamm_lut != BF_GAMMA_ENTRIES) {
    ISP_ERR("failed: invalid num_gamma_lut %d max %d",
      bf_gamma_lut_cfg->num_gamm_lut, BF_GAMMA_ENTRIES);
    return FALSE;
  }

  memcpy(&bf_gamma_cfg->bf_gamma_lut_cfg, &bf_fw->bf_gamma_lut_cfg,
    sizeof(bf_gamma_cfg->bf_gamma_lut_cfg));
  bf_gamma_cfg->trigger_update_pending = TRUE;

  return ret;
}

/** bf_gamma_scale_config:
 *
 *  @bf_gamma_cfg: handle for bf_gamma_cfg
 *  @bf_fw_local:  config from 3A
 *  @scale_ratio:  ratio for gamma stretch
 *
 *  stretch gamma by scale ratio
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_gamma_scale_config(bf_gamma_cfg_t *bf_gamma_cfg,
  bf_fw_config_t *bf_fw_local, float scale_ratio)
{
  boolean              ret = TRUE;
  bf_gamma_lut_cfg_t  *bf_gamma_lut_cfg = NULL;
  uint32_t lut_idx = 0, i = 0;
  float cur_x;

  if (!bf_gamma_cfg || !bf_fw_local) {
    ISP_ERR("failed: %p %p", bf_gamma_cfg, bf_fw_local);
    return FALSE;
  }

  bf_gamma_lut_cfg = &bf_fw_local->bf_gamma_lut_cfg;
  if (bf_gamma_lut_cfg->is_valid == FALSE) {
    return TRUE;
  }

  if (bf_gamma_lut_cfg->num_gamm_lut != BF_GAMMA_ENTRIES) {
    ISP_ERR("failed: invalid num_gamma_lut %d max %d",
      bf_gamma_lut_cfg->num_gamm_lut, BF_GAMMA_ENTRIES);
    return FALSE;
  }
  for (i = 0; i < BF_GAMMA_ENTRIES; i++) {
    cur_x = i * scale_ratio;
    lut_idx = (uint32_t)cur_x;
    if (lut_idx > BF_GAMMA_ENTRIES - 2)
      lut_idx = BF_GAMMA_ENTRIES - 2;
    bf_gamma_cfg->bf_gamma_lut_cfg.gamma_lut[i] =
      (uint32_t)isp_sub_module_util_linear_interpolate(
      cur_x, lut_idx, lut_idx + 1, bf_gamma_lut_cfg->gamma_lut[lut_idx],
      bf_gamma_lut_cfg->gamma_lut[lut_idx + 1]);
  }
  bf_gamma_cfg->trigger_update_pending = TRUE;
  return ret;
}


/** bf_gamma_get_stats_capabilities:
 *
 *  @isp_sub_module: handle for isp_sub_module
 *  @stats_info: stats info for get_capabilities
 *
 *  Get stats capabilities
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_gamma_get_stats_capabilities(isp_sub_module_t *isp_sub_module,
  mct_stats_info_t *stats_info)
{
  mct_stats_bf_fw_caps_t *bf_fw_caps = NULL;

  if (!isp_sub_module || !stats_info) {
    ISP_ERR("failed: isp_sub_module %p stats_info %p", isp_sub_module,
      stats_info);
    return FALSE;
  }

  bf_fw_caps = &stats_info->bf_caps.bf_fw_caps;
  bf_fw_caps->is_lut_supported = TRUE;
  bf_fw_caps->lut_table_size = BF_GAMMA_ENTRIES;

  return TRUE;
}

/** bf_gamma_set_stream_config:
 *
 *  @isp_sub_module: handle for isp_sub_module
 *  @bf_gamma_cfg: handle for bf_gamma_cfg
 *  @sensor_out_info: sensor out info
 *
 *  Set stream config
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bf_gamma_set_stream_config(isp_sub_module_t *isp_sub_module,
  bf_gamma_cfg_t *bf_gamma_cfg, sensor_out_info_t *sensor_out_info)
{
  if (!isp_sub_module || !bf_gamma_cfg || !sensor_out_info) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, bf_gamma_cfg, sensor_out_info);
    return FALSE;
  }

  /* Disable gamma by default */
  bf_gamma_cfg->bf_stats_enable_mask.gamma_lut_en = 0x1;
  bf_gamma_cfg->bf_stats_enable_val.gamma_lut_en = 0x0;

  return TRUE;
}
