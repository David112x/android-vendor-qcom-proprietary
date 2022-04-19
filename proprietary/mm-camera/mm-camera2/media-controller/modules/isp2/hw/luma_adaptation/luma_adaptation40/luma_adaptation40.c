/* luma_adaptation40.c
 *
 * Copyright (c) 2013-2017 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>

/* mctl headers */
#include "eztune_vfe_diagnostics.h"
#include "camera_dbg.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_LA, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_LA, fmt, ##args)

/* isp headers */
#include "module_luma_adaptation40.h"
#include "luma_adaptation40.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module_util.h"
#include "isp_pipeline_reg.h"

#define MIN_SAT_PIXELS_PERCENT .1
#define LA_AVERAGE_WEIGHT 0.8

static boolean la40_fill_vfe_diag_data(luma_adaptation40_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output);

/*default backlight table for bestshot*/
static uint32_t default_backlight_la_tbl[] = {
  0x2d30, 0x1b5d, 0x0e78, 0x0a86, 0x0a90, 0x0d9a, 0x0ca7, 0x05b3, 0xffb8,
  0xfbb7, 0xfab2, 0xfaac, 0xfaa6, 0xfaa0, 0xfb9a, 0xfa95, 0xfb8f, 0xfc8a,
  0xfb86, 0xfc81, 0xfc7d, 0xfc79, 0xfd75, 0xfd72, 0xfd6f, 0xfd6c, 0xfd69,
  0xfe66, 0xfe64, 0xfe62, 0xfe60, 0xfe5e, 0xfe5c, 0xfe5a, 0xff58, 0xfe57,
  0xff55, 0xff54, 0xfe53, 0xff51, 0xff50, 0xff4f, 0xff4e, 0xff4d, 0xff4c,
  0xff4b, 0xff4a, 0x0049, 0xff49, 0xff48, 0xff47, 0x0046, 0xff46, 0xff45,
  0x0044, 0xff44, 0x0043, 0xff43, 0xff42, 0x0041, 0xff41, 0x0040, 0x0040,
  0x0040
};

static const uint8_t solarize_la[64] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 61, 57, 54, 51, 47, 44, 42, 39,
    36, 34, 32, 29, 27, 25, 23, 22, 20, 18,
    17, 15, 13, 12, 11, 9,  8,  7,  5,  4,
    3,  2,  1,  0};

static const uint8_t posterize_la[64] = {
    0,  0,  0,  0,  0,  0,  0,  0, 0,  0,
    64, 58, 53, 50, 46, 43, 41, 38, 36, 34,
    66, 63, 60, 57, 55, 53, 51, 49, 47, 46,
    67, 65, 63, 61, 59, 57, 56, 54, 53, 51,
    67, 65, 64, 62, 61, 60, 58, 57, 56, 55,
    67, 66, 65, 64, 62, 61, 60, 59, 58, 57,
    67, 66, 65, 64};


#define LA_SCALE(v, out_min, out_max, in_min, in_max) \
  (((float)((v) - in_min) * (float)(out_max - out_min)) / \
    ((float)(in_max) - (float)(in_min)) + (float)(out_min))

/** la40_cfg_set_from_chromatix:
 *    @cfg: local luma adaptation configuration
 *    @chromatix: chromatix header configuration
 *
 * Convert chromatix header tuning for local luma adaptation usage.
 **/
static void la40_cfg_set_from_chromatix(isp_la_8k_type *cfg,
  LA_args_type *chromatix)
{
  if (!cfg || !chromatix) {
    ISP_ERR("failed: %p %p", cfg, chromatix);
    return;
  }

  cfg->offset =
    LA_SCALE(chromatix->LA_reduction_fine_tune, 0, 16, 0, 100);
  cfg->low_beam =
    LA_SCALE(chromatix->shadow_boost_fine_tune, 0, 4, 0, 100);
  cfg->high_beam =
    LA_SCALE(chromatix->highlight_suppress_fine_tune, 0, 4, 0, 100);

  /* CDF_50_thr maps in inverse to shadow_boost allowance. It should be 100 when
   * when allowance is min(0) and 70, when allowance is max(70) */
  cfg->CDF_50_thr =
    LA_SCALE(chromatix->shadow_boost_allowance, 100, 70, 0, 100);
  cfg->cap_high =
    LA_SCALE(chromatix->shadow_boost_allowance, 0.05, 0.25, 0, 100);
  cfg->histogram_cap =
    LA_SCALE(chromatix->shadow_boost_allowance, 3, 12, 0, 100);
  cfg->cap_low =
    LA_SCALE(chromatix->shadow_boost_allowance, 1.5, 1.5, 0, 100);

  if (chromatix->shadow_range > 1)
   cfg->cap_adjust = 256.0f / chromatix->shadow_range;
  else
   cfg->cap_adjust = 256.0f;

}

/** la40_prepare_hw_entry
 *
 *    @la_mod: LA handle
 *    @la_curve: the hist cap curve to adjust hist pixels
 *
 *  this function prepare the hw DMI table entries
 **/
static boolean la40_prepare_hw_entry(luma_adaptation40_t *la_mod,
  uint8_t *la_curve)
{
  boolean ret = TRUE;
  int i,j;
  int32_t val;
  int Q_yratio, unit_Q;

  if (!la_mod || !la_curve) {
    CDBG_ERROR("%s: NULL pointer! la_mod = %p, la_curve = %p, ret = FALSE\n",
      __func__, la_mod, la_curve);
    return FALSE;
  }
  /* 8974 V1 is is using 6 bit accuracy(Q6) for y ratio curve
     8974 and above is using 10 bit(Q10) */
  if (GET_ISP_SUB_VERSION(la_mod->isp_version) < 2)
    Q_yratio = 6;
  else
    Q_yratio = 10;

  unit_Q = 1 << Q_yratio;

  /* Cipher 256-entry Y_ratio curve to 64-entry LUT */
  for (i=0; i<64; i++) {
    val = 0;
    for (j=0; j<4; j++) {
      if (i || j)
        val += ((int)la_curve[(i<<2)+j] << Q_yratio) / ((i<<2)+j);
      else // both i & j are 0
        val += unit_Q;
    }
    /* devided by 4, unsign (Qyratio+2) bits */
    la_mod->la_cmd.TblEntry.table[i] = (val >> 2);

    if (la_mod->la_cmd.TblEntry.table[i] > (uint32_t)(3.99 * unit_Q))
      la_mod->la_cmd.TblEntry.table[i] = (uint32_t)(3.99 * unit_Q);

    if (la_mod->la_cmd.TblEntry.table[i] < (uint32_t)(0.75 * unit_Q))
      la_mod->la_cmd.TblEntry.table[i] = (uint32_t)(0.75 * unit_Q);
  }

  /* fill in 0 - 63 entry*/
  for (i=0; i<63; i++) {
    val = la_mod->la_cmd.TblEntry.table[i+1] - la_mod->la_cmd.TblEntry.table[i];

    /*slope: clamp to signed (Qyratio+2) bits*/
    if (val > ((1 << (Q_yratio + 1)) - 1))
      val = (1 << (Q_yratio + 1)) - 1;
    if (val < -(1 << (Q_yratio + 1)))
      val = -(1 << (Q_yratio + 1));

    if (GET_ISP_SUB_VERSION(la_mod->isp_version) < 2) {
      /* 16s */
      la_mod->la_cmd.TblEntry.table[i] =
        (uint32_t)(val << 8) | la_mod->la_cmd.TblEntry.table[i];
    } else {
      la_mod->la_cmd.TblEntry.table[i] = (uint32_t) (
        ((val & 0x00F) << 20) |
        ((la_mod->la_cmd.TblEntry.table[i] & 0x00F) << 16) |
        ((val & 0xFF0) << 4) |
        ((la_mod->la_cmd.TblEntry.table[i] & 0xFF0) >> 4));
    }
  }

  /* Fill in the last entry 64*/
  val = unit_Q - la_mod->la_cmd.TblEntry.table[ISP_LA_TABLE_LENGTH - 1];

  if (val > ((1 << (Q_yratio + 1)) - 1))
    val = (1 << (Q_yratio + 1)) - 1;
  if (val < -(1 << (Q_yratio + 1)))
    val = -(1 << (Q_yratio + 1));

  if (GET_ISP_SUB_VERSION(la_mod->isp_version) < 2) {
    /* 16s */
    la_mod->la_cmd.TblEntry.table[ISP_LA_TABLE_LENGTH - 1] =
      (uint32_t)(val << 8) |
      la_mod->la_cmd.TblEntry.table[ISP_LA_TABLE_LENGTH - 1];
  } else {
    la_mod->la_cmd.TblEntry.table[ISP_LA_TABLE_LENGTH - 1] = (uint32_t) (
      ((val & 0x00F) << 20) |
      ((la_mod->la_cmd.TblEntry.table[ISP_LA_TABLE_LENGTH - 1] & 0x00F) << 16) |
      ((val & 0xFF0) << 4)|
      ((la_mod->la_cmd.TblEntry.table[ISP_LA_TABLE_LENGTH - 1] & 0xFF0) >> 4));
   }

  for (i = 0; i<ISP_LA_TABLE_LENGTH ; i++){
    la_mod->la_cmd.TblEntry.table[i] = la_mod->la_cmd.TblEntry.table[i];
  }

  return ret;
}

/** la40_weight_average_curve
 *
 *    @la_curve_old: old curve
 *    @la_curve_new: new curve
 *    @la_curve_avg: weight average
 *
 * This function calculate average values between two la_curves
 * We take three parameters, First one is old value of curve,
 * second one is new value of curve and we put average values in
 * the third parameter.
 *
 **/
static void la40_weight_average_curve(uint8_t *la_curve_old,
  uint8_t *la_curve_new, uint8_t *la_curve_avg)
{
  uint32_t i;

  if (!la_curve_old || !la_curve_new) {
    ISP_ERR("NULL pointer! old = %p, new = %p, ret = FALSE\n",
      la_curve_old, la_curve_new);
    return;
  }

  /*weight average: weight 0.8 suggested by system team*/
  for (i=0; i<256; i++){
    la_curve_avg[i] = (uint8_t)((float)la_curve_old[i] * LA_AVERAGE_WEIGHT +
      (float)la_curve_new[i] * (1 - LA_AVERAGE_WEIGHT) + 0.5);

    /* if new and old curve value is too close, average remove the difference,
       this  place add it up, basically it's doing rounding up/down*/
    if(la_curve_avg[i] == la_curve_old[i]
       && la_curve_new[i] != la_curve_old[i])
      la_curve_avg[i] += la_curve_new[i] > la_curve_old[i] ? 1 : -1;
  }
}

/** la40_init_lut_y_ratio
 *    @lut_y_ratio: Y ratio LUT
 *
 *  Fills Y ratio LUT with default values.
 *
 **/
static void la40_init_lut_y_ratio(luma_adaptation40_t *la_mod)
{
  int         i;
  uint8_t     la_curve[256];

  if (!la_mod) {
    ISP_ERR("NULL pointer! la_mod = %p, ret = FALSE\n",
      la_mod);
    return;
  }

  for (i = 0; i < 256; i++)
    la_curve[i] = i;

  la40_prepare_hw_entry(la_mod, la_curve);
}

/** la40_algo_param_debug
 *
 *    @la_mod: LA handle
 *
 * print debug message for LA prameters (for la_curve algorithm)
 *
 **/
static void la40_algo_param_debug(isp_la_8k_type la_8k_algo_parm)
{
  /*LA algo params*/
  ISP_DBG("la_config.offset: %f\n", la_8k_algo_parm.offset);
  ISP_DBG("low beam: %f\n", la_8k_algo_parm.low_beam);
  ISP_DBG("high beam: %f\n", la_8k_algo_parm.high_beam);
  ISP_DBG("histogram cap: %f\n", la_8k_algo_parm.histogram_cap);
  ISP_DBG("cap high: %f\n", la_8k_algo_parm.cap_high);
  ISP_DBG("cap low: %f\n", la_8k_algo_parm.cap_low);
}

/** la40_debug:
 *
 *  @la_mod: la handle
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function dumps demosaic configuration
 *
 *  Return: None
 **/
static void la40_debug(luma_adaptation40_t *la_mod)
{
  ISP_DBG("VFE_Luma_Adaptation40 debug");
  int i;

  if (!la_mod) {
    ISP_ERR("NULL pointer! la_mod = %p, ret = FALSE\n",
      la_mod);
    return;
  }

  /*LA DMI table SEL*/
  ISP_DBG(" lutBankSelect: %d\n", la_mod->la_cmd.CfgCmd.lutBankSelect);
  for (i = 0; i < ISP_LA_TABLE_LENGTH ; i++)
    ISP_DBG("TblEntry.table[%d] = %d\n", i, la_mod->la_cmd.TblEntry.table[i]);
}

/** la40_dmi_tbl_write
 *    @isp_sub_module: submodule handle
 *    @applying_tbl: LA tbl
 *    @tbl_len: LA tble length
 *    @hw_reg_offset: register offset
 *    @reg_num: number of register
 *    @cmd_type: ioctl cmd type
 *
 *  write LA LUT to dmi table channel
 *
 **/
static boolean la40_dmi_tbl_write(isp_sub_module_t *isp_sub_module,
  void *applying_tbl, uint32_t tbl_len,
  uint32_t hw_reg_offset, uint32_t reg_num, uint32_t cmd_type)
{
  int i;
  boolean                     ret = TRUE;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  uint32_t                    *lut_tbl = NULL;
  uint32_t                    num_reg_cmd = 1;

  if (!isp_sub_module || !applying_tbl) {
    ISP_ERR("NULL pointer! isp_sub_module = %p, applied tbl %p ret = FALSE\n",
      isp_sub_module, applying_tbl);
    return FALSE;
  }
  /*generate hw update list struct
    including packing cfg_cmd struct*/
  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  cfg_cmd = &hw_update->cfg_cmd;

  /*generate kernel reg cmd for vfe write*/
  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)
    CAM_MALLOC(num_reg_cmd * sizeof(struct msm_vfe_reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_MALLOC;
  }
  memset(reg_cfg_cmd, 0, (num_reg_cmd * sizeof(struct msm_vfe_reg_cfg_cmd)));

  /*generate lut table data*/
  lut_tbl = (uint32_t *)CAM_MALLOC(tbl_len);
  if (!lut_tbl) {
    ISP_ERR("failed: reg_cfg_cmd %p", lut_tbl);
    goto ERROR_REG_CMD_MALLOC;
  }
  memset(lut_tbl, 0, tbl_len);

  memcpy(lut_tbl, applying_tbl, tbl_len);
  cfg_cmd->cfg_data = lut_tbl;
  cfg_cmd->cmd_len = tbl_len;
  cfg_cmd->cfg_cmd = (void *) reg_cfg_cmd;
  cfg_cmd->num_cfg = num_reg_cmd;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = cmd_type;
  reg_cfg_cmd[0].u.rw_info.reg_offset = hw_reg_offset;
  reg_cfg_cmd[0].u.rw_info.len = reg_num * sizeof(uint32_t);

  reg_cfg_cmd[0].u.dmi_info.hi_tbl_offset = 0;
  reg_cfg_cmd[0].u.dmi_info.lo_tbl_offset = 0;

  reg_cfg_cmd[0].u.dmi_info.len = tbl_len;

  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: la40_util_append_cfg");
    goto ERROR_APPEND;
  }

  return ret;

ERROR_APPEND:
  free(lut_tbl);
ERROR_REG_CMD_MALLOC:
  free(reg_cfg_cmd);
ERROR_REG_CFG_MALLOC:
  free(hw_update);

  return ret;
}

/** la40_reset_dmi_cfg
 *
 *    @isp_sub_module: moduel handle
 *    @la_mod: module private
 *    @la_channel: dmi table channel
 *
 * la_reset_dmi_cfg
 *
 * return: boolean
 *
 **/
static boolean la40_reset_dmi_cfg(isp_sub_module_t *isp_sub_module,
  luma_adaptation40_t *la_mod, uint32_t la_channel)
{
  int i;
  boolean                     ret = TRUE;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  uint32_t                    *dmi_cfg = NULL;
  uint32_t                    num_reg_cmd = 2;

  if (!la_mod || !isp_sub_module) {
    ISP_ERR("failed: %p %p", la_mod, isp_sub_module);
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
    CAM_MALLOC(num_reg_cmd * sizeof(struct msm_vfe_reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_MALLOC;
  }
  memset(reg_cfg_cmd, 0, (num_reg_cmd * sizeof(struct msm_vfe_reg_cfg_cmd)));

  dmi_cfg = (uint32_t *)CAM_MALLOC(num_reg_cmd * sizeof(uint32_t));
  if (!dmi_cfg) {
    ISP_ERR("failed: reg_cfg_cmd %p", dmi_cfg);
    goto ERROR_REG_CMD_MALLOC;
  }
  memset(dmi_cfg, 0, num_reg_cmd * sizeof(uint32_t));

  /* reset dmi cfg: config dmi channel and set auto increment*/
  dmi_cfg[0] = ISP_DMI_CFG_DEFAULT;
  dmi_cfg[0] += la_channel;

  /* reset dmi_addr_cfg: dmi address always start form 0 */
  dmi_cfg[1] = 0;

  /* PACK the 2 cfg cmd for 1 ioctl*/
  cfg_cmd->cfg_data = &dmi_cfg[0];
  cfg_cmd->cmd_len = num_reg_cmd * sizeof(uint32_t);
  cfg_cmd->cfg_cmd = (void *) reg_cfg_cmd;
  cfg_cmd->num_cfg = 2;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_DMI_CFG_OFF;
  reg_cfg_cmd[0].u.rw_info.len = 1 * sizeof(uint32_t);

  reg_cfg_cmd[1].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[0].u.rw_info.cmd_data_offset + reg_cfg_cmd[0].u.rw_info.len;
  reg_cfg_cmd[1].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_DMI_ADDR;
  reg_cfg_cmd[1].u.rw_info.len = 1 * sizeof(uint32_t);

  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: la40_util_append_cfg");
    goto ERROR_APPEND;
  }

  return ret;

ERROR_APPEND:
  free(dmi_cfg);
ERROR_REG_CMD_MALLOC:
  free(reg_cfg_cmd);
ERROR_REG_CFG_MALLOC:
  free(hw_update);

  return ret;
}

/** la40_dmi_hw_update
 *
 *    @isp_sub_module: moduel handle
 *    @la_mod: module private
 *    @bank_sel: channel bank select
 *
 * la_dmi_hw_update
 *
 * return:  boolean
 *
 **/
static boolean la40_dmi_hw_update(isp_sub_module_t *isp_sub_module,
  luma_adaptation40_t *la_mod, uint32_t bank_sel)
{
  int ret = TRUE;

  if (!la_mod || !isp_sub_module) {
    ISP_ERR("failed: %p %p", la_mod, isp_sub_module);
    return FALSE;
  }

  uint32_t *tbl = &la_mod->la_cmd.TblEntry.table[0];
  uint32_t tbl_len = sizeof(uint32_t) * ISP_LA_TABLE_LENGTH;

  uint32_t la_channel =
    (bank_sel == 0)? LA_LUT_RAM_BANK0 : LA_LUT_RAM_BANK1;

  /* 1. program DMI default value, write auto increment bit
     2. write DMI table
     3. reset DMI cfg
     4. flip the banksel bit*/

  /* write gamma channel 0 */
  ret = la40_reset_dmi_cfg(isp_sub_module, la_mod, la_channel);
  if (ret == FALSE) {
    ISP_ERR("la40_reset_dmi_cfg error, return false!");
  }

  ret = la40_dmi_tbl_write(isp_sub_module, (void *)tbl, tbl_len,
    ISP_DMI_DATA_LO, 1, VFE_WRITE_DMI_32BIT);
  if (ret == FALSE) {
    ISP_ERR("la40_dmi_tbl_write error, return false!");
  }

  la40_reset_dmi_cfg(isp_sub_module,la_mod, ISP_DMI_NO_MEM_SELECTED);
  if (ret == FALSE) {
    ISP_ERR("la40_reset_dmi_cfg error, return false!");
  }

  return ret;
}

/** la40_store_hw_update:
 *
 *  @color_correct_mod: chroma enhancement module instance
 *  @hw_update_list: hw update list handle
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function checks and sends configuration update to kernel
 *
 *  Return:   0 - Success
 *           -1 - configuration error
 **/
static boolean la40_store_hw_update(isp_sub_module_t *isp_sub_module,
  luma_adaptation40_t *la_mod)
{
  boolean ret = TRUE;
  int i, rc = 0;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  struct msm_vfe_cfg_cmd2 *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd *reg_cfg_cmd = NULL;
  ISP_LABankSelCfg *reg_cmd = NULL;

  if (!la_mod || !isp_sub_module) {
    ISP_ERR("failed: %p %p", la_mod, isp_sub_module);
    return FALSE;
  }

  ret = la40_dmi_hw_update(isp_sub_module, la_mod,
    la_mod->la_cmd.CfgCmd.lutBankSelect);
  if (ret == FALSE) {
    ISP_ERR("la40_dmi_hw_update error! ret = false");
    return ret;
  }

  /* prepare to config bank selection*/
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

  reg_cmd = (ISP_LABankSelCfg *)CAM_MALLOC(sizeof(*reg_cmd));
  if (!reg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cmd);
    goto ERROR_REG_CMD_MALLOC;
  }
  memset(reg_cmd, 0, sizeof(*reg_cmd));

  *reg_cmd = la_mod->la_cmd.CfgCmd;

  cfg_cmd->cfg_data = (void *)reg_cmd;
  cfg_cmd->cmd_len = sizeof(*reg_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_LA40_OFF;
  reg_cfg_cmd[0].u.rw_info.len = ISP_LA40_LEN * sizeof(uint32_t);

  la40_debug(la_mod);
  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: la40_util_append_cfg");
    goto ERROR_APPEND;
  }

  /* update applied config*/
  la_mod->applied_la_cmd = la_mod->la_cmd;
  memcpy(la_mod->applied_LUT_YRatio, la_mod->la_cmd.TblEntry.table,
    sizeof(int32_t) * ISP_LA_TABLE_LENGTH);
  /*flip the banksel bit*/
  la_mod->la_cmd.CfgCmd.lutBankSelect ^= 1;

  return ret;

ERROR_APPEND:
  free(reg_cmd);
ERROR_REG_CMD_MALLOC:
  free(reg_cfg_cmd);
ERROR_REG_CFG_MALLOC:
  free(hw_update);

  return FALSE;
}

/** la40_prep_spl_effect_lut
 *
 *    @la_mod: module private
 *    @spl_eff_lut: special effect table
 *    @in_tbl: input table
 *
 *  Prepare special effects table in Init state.
 *
 **/
static void la40_prep_spl_effect_lut(luma_adaptation40_t *la_mod,
  uint32_t *spl_eff_lut, const uint8_t *in_tbl)
{
  int32_t val, i;
  int Q_yratio, unit_Q;
  uint32_t spl_tbl_adjust;
  uint32_t tbl[64];

  if (!la_mod || !spl_eff_lut|| !in_tbl) {
    ISP_ERR("failed: %p %p %p", la_mod, spl_eff_lut, in_tbl);
    return;
  }

  /* 8974 V1 is is using 6 bit accuracy(Q6) for y ratio curve
     8974 and above is using 10 bit(Q10) */
  if (GET_ISP_SUB_VERSION(la_mod->isp_version) < 2) {
    Q_yratio = 6;
    spl_tbl_adjust = 1;
  } else {
    Q_yratio = 10;
    /*Q10 / Q6*/
    spl_tbl_adjust = 1024 / 64;
  }

  /*depends on differnt hw format, adjust special table value*/
  for (i = 0; i < 64; i++) {
    tbl[i] = in_tbl[i] * spl_tbl_adjust;
  }

  unit_Q = 1 << Q_yratio;

  /* fill in 0 - 63 entry*/
  for (i=0; i<63; i++) {
    val = tbl[i+1] - tbl[i];

    /*slope: clamp to signed (Qyratio+2) bits*/
    if (val > ((1 << (Q_yratio + 1)) - 1))
      val = (1 << (Q_yratio + 1)) - 1;
    if (val < -(1 << (Q_yratio + 1)))
      val = -(1 << (Q_yratio + 1));

    if (GET_ISP_SUB_VERSION(la_mod->isp_version) < 2) {
      spl_eff_lut[i] = (uint32_t)(val << 8) | tbl[i];  /* 16s */
    } else {
      spl_eff_lut[i] = (uint32_t) (
        ((val & 0x00F) << 20) | ((tbl[i] & 0x00F) << 16) |
        ((val & 0xFF0) << 4) | ((tbl[i] & 0xFF0) >> 4));
    }
  }

  /* Fill in the last entry 64*/
  val = unit_Q - tbl[ISP_LA_TABLE_LENGTH - 1];

  if (val > ((1 << (Q_yratio + 1)) - 1))
    val = (1 << (Q_yratio + 1)) - 1;
  if (val < -(1 << (Q_yratio + 1)))
    val = -(1 << (Q_yratio + 1));

  if (GET_ISP_SUB_VERSION(la_mod->isp_version) < 2) {
      spl_eff_lut[ISP_LA_TABLE_LENGTH - 1] =
         (uint32_t)(val << 8) | tbl[ISP_LA_TABLE_LENGTH - 1];  /* 16s */
  } else {
    spl_eff_lut[ISP_LA_TABLE_LENGTH - 1] = (uint32_t) (
      ((val & 0x00F) << 20) |
      ((tbl[ISP_LA_TABLE_LENGTH - 1] & 0x00F) << 16) |
      ((val & 0xFF0) << 4)|
      ((tbl[ISP_LA_TABLE_LENGTH - 1] & 0xFF0) >> 4));
   }
}

/** la40_apply_effect:
 *
 *    @module: mct module
 *    @isp_sub_module: module handle
 *    @event: mct event
 *
 *   Set effect
 *   return: boolean
 *
 **/
boolean la40_apply_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, cam_effect_mode_type spl_effect)
{
  boolean                  ret = TRUE;
  int                      i = 0;
  luma_adaptation40_t      *mod = NULL;
  uint32_t                 *pLUT_Yratio = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (luma_adaptation40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  isp_sub_module->submod_trigger_enable = FALSE;
  /* Enable trigger_update_pending */
  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->submod_enable = TRUE;
  isp_sub_module->update_module_bit = TRUE;

  mod->set_effect = TRUE;

  ISP_DBG("effect %d", spl_effect);
  switch (spl_effect) {
  case CAM_EFFECT_MODE_POSTERIZE: {
    pLUT_Yratio = mod->posterize_la_tbl;
  }
    break;

  case CAM_EFFECT_MODE_SOLARIZE: {
    pLUT_Yratio = mod->solarize_la_tbl;
  }
    break;

  default:
    if (mod->set_bestshot == FALSE) {
      isp_sub_module->submod_trigger_enable = TRUE;

      ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_sub_module_util_configure_from_chromatix_bit");
      }
    }
    mod->set_effect = FALSE;
    break;
  }

  if (mod->set_effect == TRUE) {
    for (i = 0; i < ISP_LA_TABLE_LENGTH ; i++)
      mod->la_cmd.TblEntry.table[i] = pLUT_Yratio[i];
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

END:
  return ret;
}

/** la40_set_spl_effect:
 *
 *    @module: mct module
 *    @isp_sub_module: module handle
 *    @event: mct event
 *
 *   Set effect
 *   return: boolean
 *
 **/
boolean la40_set_spl_effect(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                   ret = TRUE;
  luma_adaptation40_t      *mod = NULL;
  cam_effect_mode_type      spl_effect;
  mct_event_control_parm_t *param = NULL;
  chromatix_parms_type     *chromatix_ptr = NULL;

  mod = (luma_adaptation40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  /*get best shot mode from mct event data*/
  param = event->u.ctrl_event.control_event_data;
  if (!param || !param->parm_data) {
    ISP_ERR("failed");
    return FALSE;
  }

  spl_effect = *((cam_effect_mode_type *)param->parm_data);
  if (spl_effect >= CAM_EFFECT_MODE_MAX) {
    ISP_ERR("failed: spl_effect %d", spl_effect);
    return FALSE;
  }

  mod->effect_mode = spl_effect;

  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
     mod->set_effect = TRUE;

    /* Will be handled after set_chromatix_ptr is received */
    return TRUE;
  }

  ret = la40_apply_effect(module, isp_sub_module, mod->effect_mode);
  if (ret == FALSE) {
    ISP_ERR("failed: la40_apply_effect");
  }

  if ((mod->set_effect == FALSE) && (mod->set_bestshot == TRUE)) {
    ret = la40_apply_bestshot(module, isp_sub_module, mod->bestshot_mode);
    if (ret == FALSE) {
      ISP_ERR("failed: la40_apply_effect");
    }
  }

  return ret;
}

/** la40_streamon:
 *
 *  @module: mct module
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function makes initial configuration of demosaic module
 *
 *  Return:   TRUE- Success
 *            FALSE - Parameters size mismatch
 **/
boolean la40_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                 ret = TRUE;
  luma_adaptation40_t     *la_mod = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  la_mod = (luma_adaptation40_t *)isp_sub_module->private_data;
  if (!la_mod) {
    ISP_ERR("failed: mod %p", la_mod);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  /* Update streaming mode mask in module private */
  mct_list_traverse(isp_sub_module->l_stream_info,
    isp_sub_module_util_update_streaming_mode, &la_mod->streaming_mode_mask);

  if ((la_mod->set_effect == FALSE) && (la_mod->set_bestshot == FALSE)) {
    ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_sub_module_util_configure_from_chromatix_bit");
    }
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** la40_streamoff:
 *
 *  @module: mct module
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function resets demosaic module
 *
 *  Return: boolean
 **/
boolean la40_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  luma_adaptation40_t *mod = NULL;
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (luma_adaptation40_t *)isp_sub_module->private_data;
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

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** la40_apply_bestshot:
 *
 *  @module: mct module
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Set BestShot mode
 * return boolean
 *
 **/
boolean la40_apply_bestshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, cam_scene_mode_type bestshot_mode)
{
  boolean                        ret = TRUE;
  luma_adaptation40_t           *mod = NULL;
  mct_event_control_parm_t      *param = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (luma_adaptation40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  /* disable trigger when bestshot mode */
  isp_sub_module->submod_trigger_enable = FALSE;

  /* Enable trigger_update_pending */
  isp_sub_module->trigger_update_pending = TRUE;

  isp_sub_module->submod_enable = TRUE;
  isp_sub_module->update_module_bit = TRUE;
  mod->set_bestshot = TRUE;

  ISP_DBG("bestshot mode %d", bestshot_mode);
  switch (bestshot_mode) {
  case CAM_SCENE_MODE_BACKLIGHT: {
    memcpy(mod->la_cmd.TblEntry.table, default_backlight_la_tbl,
      sizeof(uint32_t) * ISP_LA_TABLE_LENGTH);
  }
    break;

  case CAM_SCENE_MODE_HDR:
  case CAM_SCENE_MODE_SUNSET:
  case CAM_SCENE_MODE_FLOWERS:
  case CAM_SCENE_MODE_CANDLELIGHT: {
    /* Disable Module for these Best Shot Mode */
    la40_init_lut_y_ratio(mod);
  }
    break;
  default:
    if (mod->set_effect == FALSE) {
      la40_init_lut_y_ratio(mod);
      isp_sub_module->submod_trigger_enable = TRUE;

      ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_sub_module_util_configure_from_chromatix_bit");
      }
    }
    mod->set_bestshot = FALSE;

    break;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
}

/** la40_set_bestshot:
 *
 *  @module: mct module
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Set BestShot mode
 * return boolean
 *
 **/
boolean la40_set_bestshot(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                   ret = TRUE;
  luma_adaptation40_t      *mod = NULL;
  mct_event_control_parm_t *param = NULL;
  cam_scene_mode_type       bestshot_mode = CAM_SCENE_MODE_OFF;
  chromatix_parms_type     *chromatix_ptr = NULL;

  mod = (luma_adaptation40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  /*get best shot mode from mct event data*/
  param = event->u.ctrl_event.control_event_data;
  if (!param || !param->parm_data) {
    ISP_ERR("failed");
    return FALSE;
  }

  bestshot_mode = *((cam_scene_mode_type *)param->parm_data);
  if (bestshot_mode >= CAM_SCENE_MODE_MAX) {
    ISP_ERR("failed: bestshot_mode %d", bestshot_mode);
    return FALSE;
  }

  mod->bestshot_mode = bestshot_mode;

  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
     mod->set_bestshot = TRUE;

    /* Will be handled after set_chromatix_ptr is received */
    return TRUE;
  }

  ret = la40_apply_bestshot(module, isp_sub_module, mod->bestshot_mode);
  if (ret == FALSE) {
    ISP_ERR("failed: la40_apply_effect");
  }

  if ((mod->set_effect == TRUE) && (mod->set_bestshot == FALSE)) {
    ret = la40_apply_effect(module, isp_sub_module, mod->effect_mode);
    if (ret == FALSE) {
      ISP_ERR("failed: la40_apply_effect");
    }
  }

  return ret;
}

/** la40_set_chromatix_ptr
 *
 *  @module: mct module handle
 *  @isp_sub_mdoule: isp sub module handle
 *  @event: event handle
 *
 *  Update chromatix ptr
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean la40_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean              ret = TRUE;
  modulesChromatix_t  *chromatix_ptrs = NULL;
  luma_adaptation40_t *mod = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  mod = (luma_adaptation40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  chromatix_ptrs =
    (modulesChromatix_t *)event->u.module_event.module_event_data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;
  isp_sub_module->trigger_update_pending = TRUE;

  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  if (mod->set_effect == TRUE) {
    ret = la40_apply_effect(module, isp_sub_module, mod->effect_mode);
    if (ret == FALSE) {
      ISP_ERR("failed: la40_apply_effect");
    }
  }

  if (mod->set_bestshot == TRUE) {
    ret = la40_apply_bestshot(module, isp_sub_module, mod->bestshot_mode);
    if (ret == FALSE) {
      ISP_ERR("failed: la40_apply_bestshot");
    }
  }

  return TRUE;
}

/** la40_save_asd_param:
 *
 *  @module: mct module
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
**/
boolean la40_save_asd_param(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                 ret = TRUE;
  uint32_t                backlight_scene_severity = 0;
  stats_update_t          *stats_update = NULL;
  luma_adaptation40_t     *mod = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  asd_update_t            *asd_out = NULL;

  ISP_HIGH("E");
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  mod = (luma_adaptation40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  asd_out = &stats_update->asd_update;

  chromatix_ptr =
      (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return TRUE;
  }
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (mod->bestshot_mode == CAM_SCENE_MODE_BACKLIGHT)
    backlight_scene_severity = 255; /* max severity*/
  else
    backlight_scene_severity =
      MIN(255, asd_out->backlight_scene_severity);

  /*if aec not change, and back light not change than return,
    no trigger update if both AEC and ASD not changed*/
  if ((mod->backlight_severity == backlight_scene_severity)) {
    ISP_DBG("skip aec trigger upadate, back light: old = %d, new = %d\n",
      mod->backlight_severity, asd_out->backlight_scene_severity);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  /* need trigger update, Store AEC update in module private */
  mod->backlight_severity = asd_out->backlight_scene_severity;
  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}


/** la40_save_aec_param:
 *
 *  @module: mct module
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
**/
boolean la40_save_aec_param(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                 ret = TRUE;
  float                   aec_trigger_ratio = 0.0;
  stats_update_t          *stats_update = NULL;
  luma_adaptation40_t     *mod = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  chromatix_LA_type       *chromatix_LA = NULL;
  trigger_point_type      *trigger_point = NULL;
  float                    la_ratio = 0.0, total_drc_gain = 0.0;

  ISP_DBG("E");
  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  mod = (luma_adaptation40_t *)isp_sub_module->private_data;
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
  chromatix_LA = &chromatix_ptr->chromatix_VFE.chromatix_LA;
  trigger_point = &(chromatix_LA->la_brightlight_trigger);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  /* Decide the trigger ratio for current lighting condition,
     fill in trigger ratio*/
  aec_trigger_ratio = isp_sub_module_util_get_aec_ratio_bright(
    chromatix_LA->control_la, (void *)trigger_point, &stats_update->aec_update);

  /*protect the ratio if out of bound*/
  if (aec_trigger_ratio > 1.0)
    aec_trigger_ratio = 1.0;
  else if (aec_trigger_ratio < 0.0)
    aec_trigger_ratio = 0.0;

  total_drc_gain = stats_update->aec_update.total_drc_gain;
  la_ratio = stats_update->aec_update.la_ratio;

  mod->enable_adrc =
    isp_sub_module_util_is_adrc_mod_enable(la_ratio, total_drc_gain);

  /*if aec not change, and back light not change than return
    no trigger update if both ASD and AEC not changed*/
  if (!F_EQUAL(aec_trigger_ratio, mod->trigger_ratio) ||
      (mod->enable_adrc &&
       (!F_EQUAL(total_drc_gain, mod->prev_total_drc_gain)||
        !F_EQUAL(la_ratio, mod->prev_la_ratio)))) {
    isp_sub_module->trigger_update_pending = TRUE;
  }
  else {
    ISP_DBG("skip aec trigger upadate, aec ratio: old %f, new %f\n",
      mod->trigger_ratio, aec_trigger_ratio);
  }

  /* need trigger update, Store AEC update in module private */
  mod->trigger_ratio = aec_trigger_ratio;
  mod->stats_update.aec_update = stats_update->aec_update;
  mod->prev_total_drc_gain = total_drc_gain;
  mod->prev_la_ratio = la_ratio;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** la40_trigger_update_aec:
 *
 * @la_mod: LA module
 * @chromatix_ptr: chromatix pointer
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean la40_trigger_update_aec(luma_adaptation40_t *la_mod,
  chromatix_parms_type *chromatix_ptr)
{
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION != 0x304)
  ASD_VFE_struct_type     *ASD_algo_data = NULL;
#else
  ASD_struct_type         *ASD_algo_data = NULL;
#endif
  boolean                 ret = TRUE;
  isp_la_8k_type          la_8k_config_indoor, la_8k_config_outdoor;
  isp_la_8k_type          la_config_compensated, la_config_backlight;
  chromatix_LA_type       *chromatix_LA = NULL;

  if (!la_mod || !chromatix_ptr) {
    ISP_ERR("failed: %p %p", la_mod, chromatix_ptr);
    return FALSE;
  }
  chromatix_LA = &chromatix_ptr->chromatix_VFE.chromatix_LA;
  ASD_algo_data = &chromatix_ptr->ASD_algo_data;

  la40_cfg_set_from_chromatix(&la_8k_config_indoor,
    &chromatix_LA->LA_config);
  la40_cfg_set_from_chromatix(&la_8k_config_outdoor,
    &chromatix_LA->LA_config_outdoor);

  /*interpolate algo parm by aec ratio*/
  ISP_DBG("aec ratio = %f\n", la_mod->trigger_ratio);
  la_mod->la_8k_algo_parm.offset = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.offset, la_8k_config_outdoor.offset,
    la_mod->trigger_ratio));
  la_mod->la_8k_algo_parm.low_beam = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.low_beam, la_8k_config_outdoor.low_beam,
    la_mod->trigger_ratio));
  la_mod->la_8k_algo_parm.high_beam = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.high_beam, la_8k_config_outdoor.high_beam,
    la_mod->trigger_ratio));
  la_mod->la_8k_algo_parm.histogram_cap = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.histogram_cap, la_8k_config_outdoor.histogram_cap,
    la_mod->trigger_ratio));
  la_mod->la_8k_algo_parm.cap_high = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.cap_high, la_8k_config_outdoor.cap_high,
    la_mod->trigger_ratio));
  la_mod->la_8k_algo_parm.cap_low = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.cap_low, la_8k_config_outdoor.cap_low,
    la_mod->trigger_ratio));

  la_mod->la_8k_algo_parm.cap_adjust = (float) (LINEAR_INTERPOLATION(
    la_8k_config_indoor.cap_adjust, la_8k_config_outdoor.cap_adjust,
    la_mod->trigger_ratio));

  la_mod->la_8k_algo_parm.CDF_50_thr = (float)(LINEAR_INTERPOLATION(
    la_8k_config_indoor.CDF_50_thr, la_8k_config_outdoor.CDF_50_thr,
    la_mod->trigger_ratio));

  ISP_DBG(": backlight_scene_severity :%d ", la_mod->backlight_severity);
  if (la_mod->backlight_severity != 0) {
    la_config_compensated = la_mod->la_8k_algo_parm;
    la40_cfg_set_from_chromatix(&la_config_backlight,
      &ASD_algo_data->backlit_scene_detect.backlight_la_8k_config);

    la_mod->la_8k_algo_parm.offset = (la_config_compensated.offset * (255 -
      la_mod->backlight_severity) + la_config_backlight.offset *
      la_mod->backlight_severity)/255.0;

    la_mod->la_8k_algo_parm.low_beam = (la_config_compensated.low_beam * (255 -
      la_mod->backlight_severity) + la_config_backlight.low_beam *
      la_mod->backlight_severity)/255.0;

    la_mod->la_8k_algo_parm.high_beam = (la_config_compensated.high_beam * (255
      - la_mod->backlight_severity) + la_config_backlight.high_beam *
      la_mod->backlight_severity)/255.0;

    la_mod->la_8k_algo_parm.histogram_cap =(la_config_compensated.histogram_cap
      * (255 - la_mod->backlight_severity) + la_config_backlight.
      histogram_cap * la_mod->backlight_severity)/255.0;

    la_mod->la_8k_algo_parm.cap_high = (la_config_compensated.cap_high * (255 -
      la_mod->backlight_severity) + la_config_backlight.cap_high *
      la_mod->backlight_severity)/255;

    la_mod->la_8k_algo_parm.cap_low = (la_config_compensated.cap_low * (255 -
      la_mod->backlight_severity) + la_config_backlight.cap_low *
      la_mod->backlight_severity)/255.0;

    la_mod->la_8k_algo_parm.cap_adjust = (la_config_compensated.cap_adjust *
      (255 - la_mod->backlight_severity) + la_config_backlight.cap_adjust *
      la_mod->backlight_severity)/255.0;

    la_mod->la_8k_algo_parm.CDF_50_thr = (la_config_compensated.CDF_50_thr *
      (255 - la_mod->backlight_severity) +
      la_config_backlight.CDF_50_thr * la_mod->backlight_severity)
        / 255.0;
  }

  return ret;
}

/** la40_trigger_update:
 *
 *  @module: mct module
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 *  This function prepare the hw update list and do trigger update
 *
 *  Return:   TRUE - Success
 *            FALSE - Parameters size mismatch
 **/
boolean la40_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret = TRUE;
  isp_private_event_t     *private_event = NULL;
  chromatix_parms_type    *chromatix_ptr = NULL;
  luma_adaptation40_t     *la_mod = NULL;
  isp_sub_module_output_t *sub_module_output = NULL;
  isp_meta_entry_t        *la_dmi_info = NULL;
  uint32_t                *la_dmi_tbl = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  /* get chroma enhance mod private*/
  la_mod = (luma_adaptation40_t *)isp_sub_module->private_data;
  if (!la_mod) {
    ISP_ERR("failed: mod %p", la_mod);
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

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if(!isp_sub_module->stream_on_count){
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if ((isp_sub_module->trigger_update_pending == TRUE) &&
    (isp_sub_module->submod_trigger_enable == FALSE)) {

    ret = la40_store_hw_update(isp_sub_module, la_mod);
    if (ret == FALSE) {
      ISP_ERR("failed: la40_do_hw_update");
    }
    isp_sub_module->trigger_update_pending = FALSE;

  } else if (isp_sub_module->trigger_update_pending == TRUE &&
    (isp_sub_module->submod_trigger_enable == TRUE)) {

    if (la_mod->enable_adrc == TRUE) {
      ret = la_update_adrc_la_curve(module, isp_sub_module, event);
      if (ret == FALSE) {
        ISP_ERR("failed: la40_update_adrc_la_curve");
      }
    } else {
    /*in trigger update, update the la algo parm
      based on aec and back light*/
    ret = la40_trigger_update_aec(la_mod, chromatix_ptr);
    if (ret == FALSE) {
       ISP_ERR("la40_trigger_update_aec error, ret = FALSE!\n");
    }
   }

    if (isp_sub_module->submod_enable == TRUE) {
      /* Update DMI table based on LA curve */
      la40_prepare_hw_entry(la_mod, la_mod->la_curve);

      ret = la40_store_hw_update(isp_sub_module, la_mod);
      if (ret == FALSE) {
        ISP_ERR("failed: la40_do_hw_update");
      }
    }
    isp_sub_module->trigger_update_pending = FALSE;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
    sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
  }

  /*pass algo parm to parser/algo thread*/
  sub_module_output->algo_params->la_8k_algo_parm = la_mod->la_8k_algo_parm;
  sub_module_output->algo_params->is_la_algo_parm_valid = TRUE;
  la40_algo_param_debug(sub_module_output->algo_params->la_8k_algo_parm);

FILL_METADATA:
   if (sub_module_output->frame_meta) {
     if (la_mod->set_effect == TRUE)
         sub_module_output->frame_meta->special_effect = la_mod->effect_mode;
    }

  if (sub_module_output->metadata_dump_enable == 1) {
    /*fill in DMI info*/
    la_dmi_info = &sub_module_output->
      meta_dump_params->meta_entry[ISP_META_LA_TBL];
    /*dmi table length*/
    la_dmi_info->len =
      sizeof(uint32_t) * ISP_LA_TABLE_LENGTH;
    /*dmi type*/
    la_dmi_info->dump_type  = ISP_META_LA_TBL;
    la_dmi_info->start_addr = 0;
    sub_module_output->meta_dump_params->frame_meta.num_entry++;

    sub_module_output->meta_dump_params->frame_meta.adrc_info.la_ratio =
     la_mod->stats_update.aec_update.la_ratio;
    sub_module_output->meta_dump_params->frame_meta.adrc_info.exposure_ratio =
     la_mod->stats_update.aec_update.hdr_sensitivity_ratio;
    sub_module_output->meta_dump_params->frame_meta.adrc_info.
      exposure_time_ratio = la_mod->stats_update.aec_update.hdr_exp_time_ratio;
    sub_module_output->meta_dump_params->frame_meta.adrc_info.
      reserved_data[0] = la_mod->stats_update.aec_update.total_drc_gain;

#ifdef DYNAMIC_DMI_ALLOC
    la_dmi_tbl =
      (uint32_t *)CAM_MALLOC(la_dmi_info->len);
    if (!la_dmi_tbl) {
      ISP_ERR("failed: %p", la_dmi_tbl);
      ret = FALSE;
      goto ERROR;
    }
#endif
    memcpy(la_dmi_info->isp_meta_dump,
      &la_mod->applied_la_cmd.TblEntry.table, la_dmi_info->len);
    //la_dmi_info->hw_dmi_tbl = la_dmi_tbl;
  }

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->bestshot_mode =
      la_mod->bestshot_mode;
    if (isp_sub_module->vfe_diag_enable) {
      ret = la40_fill_vfe_diag_data(la_mod, isp_sub_module,
        sub_module_output);
      if (ret == FALSE) {
        ISP_ERR("failed: la40_fill_vfe_diag_data");
      }
    }
  }

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
}

/** la40_update_streaming_mode_mask:
 *
 *  @module: mct module
 *  @isp_sub_module: sub module handle
 *  @streaming_mode_mask: stream mode mask
 *
 *  update streaming mask
 *
 **/
void la40_update_streaming_mode_mask(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, uint32_t streaming_mode_mask)
{
  luma_adaptation40_t *la_mod = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: %p %p", module, isp_sub_module);
    return;
  }

  la_mod = (luma_adaptation40_t *)isp_sub_module->private_data;
  if (!la_mod) {
    ISP_ERR("failed: demosaic %p", la_mod);
    return;
  }
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  la_mod->streaming_mode_mask = streaming_mode_mask;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return;
}

/** la40_ez_isp_update
 *
 *  @isp_sub_module: sub module handle
 *  @la_mod: module private
 *  @la_Diag: diag data
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void la40_ez_isp_update(isp_sub_module_t *isp_sub_module,
  luma_adaptation40_t *la_mod, lumaadaptation_t *la_Diag)
{
  int index;

  if (!isp_sub_module || !la_mod || !la_Diag) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, la_mod, la_Diag);
    return;
  }

  int idx;
  for (idx = 0; idx < ISP_LA_TABLE_LENGTH; idx++)
    la_Diag->lut_yratio[idx] = la_mod->applied_LUT_YRatio[idx];
}

/** la40_update_la_curve
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Get VFE diag info
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean la40_update_la_curve(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  isp_private_event_t   *private_event = NULL;
  luma_adaptation40_t   *la_mod = NULL;
  isp_saved_la_params_t *la_algo_output = NULL;
  uint32_t              i = 0;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p data %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  la_mod = (luma_adaptation40_t *)isp_sub_module->private_data;
  if (!la_mod) {
    ISP_ERR("failed: luma_adaptation40_t %p", la_mod);
    return FALSE;
  }

  la_algo_output =
    (isp_saved_la_params_t *)event->u.module_event.module_event_data;
  if (!la_algo_output) {
    ISP_ERR("failed: la algo_output %p", la_algo_output);
    return FALSE;
  }

  /*curve debug*/
  ISP_DBG("LADEBUG LA_curve size = %d", la_algo_output->curve_size);
  for (i = 0; i < 256; i++) {
    ISP_DBG("LA_curve[%d] = %d", i, la_algo_output->la_curve[i]);
  }

  /*copy la curve*/
  memcpy(&la_mod->la_curve[0], &la_algo_output->la_curve[0],
    sizeof(uint8_t) * 256);

  if ((la_mod->set_effect == FALSE) && (la_mod->set_bestshot == FALSE)) {
    isp_sub_module->trigger_update_pending = TRUE;
  }

  return TRUE;
}

/** la40_get_vfe_diag_info_user
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  Get VFE diag info
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean la40_get_vfe_diag_info_user(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  isp_private_event_t *private_event = NULL;
  luma_adaptation40_t   *la_mod = NULL;
  vfe_diagnostics_t   *vfe_diag = NULL;
  lumaadaptation_t   *la_Diag = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p data %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  la_mod = (luma_adaptation40_t *)isp_sub_module->private_data;
  if (!la_mod) {
    ISP_ERR("failed: chroma_enhan_mod %p", la_mod);
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

  la_Diag = &(vfe_diag->prev_lumaadaptation);
  if (la_mod->old_streaming_mode == CAM_STREAMING_MODE_BURST) {
    la_Diag = &(vfe_diag->snap_lumaadaptation);
  }
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  la40_ez_isp_update(isp_sub_module, la_mod, la_Diag);
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** la40_fill_vfe_diag_data:
 *
 *  @mod: la  module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean la40_fill_vfe_diag_data(luma_adaptation40_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean             ret = TRUE;
  lumaadaptation_t   *la_Diag = NULL;
  vfe_diagnostics_t  *vfe_diag = NULL;

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    la_Diag = &(vfe_diag->prev_lumaadaptation);
    if (mod->old_streaming_mode == CAM_STREAMING_MODE_BURST) {
      la_Diag = &(vfe_diag->snap_lumaadaptation);
    }

    la40_ez_isp_update(isp_sub_module, mod, la_Diag);
  }
  return ret;
}

/** la40_init:
 *
 *  @module: mct module
 *  @isp_sub_module : submodule handle
 *
 *  This function runs in ISP HW thread context.
 *
 *  This function instantiates a demosaic module
 *
 *  Return:   NULL - not enough memory
 *            Otherwise handle to module instance
 **/
boolean la40_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  luma_adaptation40_t *mod = NULL;

  ISP_HIGH("E");
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  mod = (luma_adaptation40_t *)CAM_MALLOC(sizeof(luma_adaptation40_t));
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return FALSE;
  }

  memset(mod, 0, sizeof(*mod));
  memset(&mod->la_cmd, 0, sizeof(mod->la_cmd));
  memset(&mod->trigger_ratio, 0, sizeof(mod->trigger_ratio));

  /* enable trigger update feature flag from PIX */
  mod->trigger_ratio = 1.0;
  isp_sub_module->submod_enable = TRUE; /* enable flag from PIX */
  isp_sub_module->submod_trigger_enable = TRUE;
  isp_sub_module->trigger_update_pending = TRUE;
  memset(&mod->solarize_la_tbl, 0, sizeof(mod->solarize_la_tbl));
  memset(&mod->posterize_la_tbl, 0, sizeof(mod->posterize_la_tbl));
  memset(&mod->la_8k_algo_parm, 0, sizeof(mod->la_8k_algo_parm));
  mod->old_streaming_mode = CAM_STREAMING_MODE_MAX;

  isp_sub_module->private_data = (void *)mod;
  mod->gamma_table_g = NULL;

  la40_init_lut_y_ratio(mod);
  la40_prep_spl_effect_lut(mod, mod->solarize_la_tbl, solarize_la);
  la40_prep_spl_effect_lut(mod, mod->posterize_la_tbl, posterize_la);

  return TRUE;
}

/** la40_destroy:
 *
 *  @module: mct module
 *  @isp_sub_module : submodule handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void la40_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  luma_adaptation40_t *mod = NULL;

  ISP_HIGH("E");

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }
  /* get Luma adaptation mod private*/
  mod = (luma_adaptation40_t *)isp_sub_module->private_data;
  if (!mod) {
    ISP_ERR("failed: mod %p", mod);
    return;
  }
  /* Free Gamma interpolated table */
  if (mod->gamma_table_g) {
    free(mod->gamma_table_g);
  }

  free(isp_sub_module->private_data);
  return;
}

#if 0    // LA algo, move out, input = ihist stats + la_cfg
/** la_get_min_pdf_count:
 *    @num_hist_pixels: number of pixels
 *
 * Return: min of Hist pixels
 **/
static uint64_t la_get_min_pdf_count(uint32_t num_hist_pixels)
{
  return (uint64_t)(MIN_SAT_PIXELS_PERCENT * (float)num_hist_pixels);
}

/** stats_calc_hist_curve:
 *
 * This function process histogram stats from the isp and
 * and calculates the LA curve
 *
 **/
static int stats_calc_hist_curve(isp_ihist_params_t *ihist_stats,
  isp_la_mod_t *la_mod, uint32_t Offset, uint8_t *la_curve)
{
  uint16_t shift, CDF_50_threshold;
  uint64_t size = 0, capped_count, cap_inc, tmp64;
  uint32_t *hist, *threshold, avg_inc;
  /* full_bin: hist==thld, high_bin: hist>thld */
  uint32_t high_bin, full_bin, iter_cnt;
  uint64_t *H;
  uint64_t num_hist_pixels;
  float tmp0, tmp1, tmp, cap_adjust, cap_ratio, cap_orig, cap_max, cap;
  float backlight = la_mod->la_config.low_beam;
  float frontlight = la_mod->la_config.high_beam;
  register int i;

  /* LA adjustable range [0, EqRange); y=x for luma values [EqRange, 255] */
  const int EqRange = 255;  /* Original: 255 */

  /* Adjusts width of Gaussian cap curve, default: 3.6 */
  cap_adjust = la_mod->la_config.cap_adjust;
  /* Normal: 0.05; Backlight: 0.25 */
  cap_ratio  = la_mod->la_config.cap_high;
  /* Height of Gaussian above 1.0, default: 1.5 */
  cap_orig   = la_mod->la_config.cap_high;
  /* Normal: 3; Backlight: 12 */
  cap_max    = la_mod->la_config.histogram_cap;
  /* Normal: 100; Backlight: 70 */
  CDF_50_threshold = la_mod->la_config.CDF_50_thr;

  hist = (uint32_t*)CAM_MALLOC(256*sizeof(uint32_t));
  if (!hist) {
    CDBG_ERROR("isp_stats_calc_hist_curve CAM_MALLOC failed \n");
    return -1;
  }
  H = (uint64_t*)CAM_MALLOC(256*sizeof(uint64_t));
  if (!H) {
    free(hist);
    hist = NULL;
    CDBG_ERROR("isp_stats_calc_hist_curve CAM_MALLOC failed \n");
    return -1;
  }
  threshold = (uint32_t*)CAM_MALLOC(256*sizeof(uint32_t));
  if (!threshold) {
    free(hist);
    free(H);
    CDBG_ERROR("isp_stats_calc_hist_curve CAM_MALLOC failed \n");
    return -1;
  }

  /* Total histogram counts */
  for (i=0; i<256; i++) {
     /* Original histogram bins */
     hist[i] = ihist_stats->isp_ihist_data[i];
     /* New total count */
     size += hist[i];
  }

  /*compute original CDF, then calculate cap*/
  H[0] = hist[0];
  num_hist_pixels = H[0];

  /*Avoid the saturated pixels*/
  for (i=1; i<=255; i++) {
     if (i <= 250) {
        H[i] = H[i-1] + hist[i];
     }
     num_hist_pixels += hist[i];
  }
  /*if overall histogram is bright and saturated, then return from LA*/
  if (H[250] < la_get_min_pdf_count(num_hist_pixels)) {
    CDBG("%s: pdf count %llu", __func__, H[250]);
    free(hist);
    free(H);
    free(threshold);
    return -1;
  }

  if (H[250] == 0) {
    CDBG_ERROR("%s: ALL Ihist stats = 0!!\n", __func__);
    return -1;
  }
  /*Avoid the saturated pixels*/
  for (i=1; i<=250; i++) {
     H[i] = H[i] * 255 / H[250];
  }
  /* check in scale of bin 50's */
  cap = cap_orig;
  if (H[50] > CDF_50_threshold) {
        cap = cap_orig + (H[50] - CDF_50_threshold) * cap_ratio;
        cap = (cap < cap_max)? cap : cap_max;
  }
  /* new curve */
  for (i=0; i<256; i++) {
     /* Nonlinear cap curve */
     tmp = 0;
     /* interpolation */
     if ((int)(i * cap_adjust) < 255) {
       tmp0 = (float)curve[(int16_t)(i * cap_adjust)];
       tmp1 = (float)curve[(int16_t)(i * cap_adjust) + 1];
       tmp  = tmp0 + (tmp1 - tmp0) * (i * cap_adjust -
         (int16_t)(i * cap_adjust));
       threshold[i] = (uint32_t)((((uint32_t)
         (tmp * cap) + 64) * (size >> 8)) >> 6);
     } else {
       threshold[i] = (uint32_t)(size >> 8);  /* (64*(size>>8))>>6 */
     }
  }
  /* apply cap to histogram curve */
  avg_inc = 0;
  iter_cnt = 0;
  do {
    for (i=0; i<256; i++) {
       /* Add back average capped histogram counts to uncapped bins */
       if (hist[i] < threshold[i])
         hist[i] += avg_inc;  /* KSH: Changed from < to <= for uncapped bins */
       }
       for (i=0, capped_count = high_bin = full_bin = 0; i<256; i++) {
          /* Cap the histogram if bin count > threshold */
          if (hist[i] > threshold[i]) {
            high_bin++;
            capped_count += (hist[i] - threshold[i]);
            hist[i] = threshold[i];
          } else if (hist[i] == threshold[i]) {
                    full_bin++;
          }
       }
       CDBG("iterartion %d,", iter_cnt);
       if ((full_bin+high_bin) < 256)  /*alway true*/
         /* Distribute capped histogram counts to uncapped bins */
         avg_inc = capped_count / (256-full_bin-high_bin);
         iter_cnt++;
        CDBG("full_bin, %d, high_bin, %d, avg_inc, %d\n",
          full_bin, high_bin, avg_inc);
  } while (high_bin > 0 && iter_cnt < 10);
  /* Adjust histogram: Offset, Low light boost, High light boost */
  /* adjusted histogram */
  size = 0;
  for (i=0; i<256; i++) {
     hist[i] += Offset;
     /*assert((32 + backlight * 4) <= (256 - 32- (frontlight * 4)));*/
     if (i < (32+(int)(backlight*4))) {
       /* Low light boost */
       hist[i] = (uint32_t)((hist[i]) *
         (1.0f + (float)(32 + backlight * 4 - i) * backlight * 0.4f/36.0f));
     }
     if (i > (256-32-(int)(frontlight * 4))) {
       /* High light boost */
       hist[i] = (uint32_t)((hist[i])*(1.0f + (float)(i - (256 - 32 -
         (frontlight * 4))) * frontlight * 0.6f/32.0f));
     }
     /* New total count */
     size += hist[i];
  }
  /* Compute LA curve / Compute target CDF */
  H[0] = hist[0];
  for (i=1; i <= EqRange; i++) {
     H[i] = H[i-1] + hist[i];
  }
  /* Scale target CDF with enacted equalization range (default full 255) */
  /*scaled CDF */
  for (i=0; i <= EqRange; i++) {
     if (H[EqRange] == 0) {
        CDBG_ERROR("%s: H[EqRange] = 0\n", __func__);
        return -1;
     }
     H[i] = EqRange * H[i]/H[EqRange];
  }
  for ( ; i<256; i++) {
     /* Straight line y=x of slope 1 after EqRange */
     H[i] = i;
  }
  /* Smooth target mapping function */
  for (iter_cnt=0; iter_cnt<1; iter_cnt++) {
     H[0]=0;
     H[1]=(0+H[1]+H[2])/3;
     for (i=2;i<254;i++) {
        H[i] = (H[i-2]+H[i-1]+H[i]+H[i+1]+H[i+2])/5;
     }
     H[254]=(H[253]+H[254]+255)/3;
     H[255]=255;
  }
  /* smoothed CDF (final output) */
  for (i=0; i<256; i++) {
     la_curve[i] = H[i];
  }
  if (threshold) free(threshold);
  if (hist) free(hist);
  if (H) free(H);
  threshold = hist = NULL;
  H = NULL;
  return 0;
}

/** la_hist_trigger_update
 *
 * This function process histogram stats from the isp and
 * updates the Luma Adaptation LUT, this function is called by
 * media controller when stats obtained from kernel
 *
 **/
static int la_hist_trigger_update(isp_la_mod_t *la_mod,
  isp_pix_trigger_update_input_t *in_params, uint32_t in_param_size)
{
  int rc = 0;
  uint8_t la_curve[256];
  uint8_t la_curve_avg[256];
  uint32_t i,j;
  float luma_off_ratio;
  int32_t N_pixels, offset;
  float equalize;
  isp_ihist_params_t *ihist_stats =
    &(in_params->trigger_input.stats_update.ihist_params);
  chromatix_parms_type *pchromatix =
    (chromatix_parms_type *)in_params->cfg.chromatix_ptrs.chromatixPtr;
  equalize = la_mod->la_config.offset;
  chromatix_LA_type *chromatix_LA =
    &pchromatix->chromatix_VFE.chromatix_LA;

  if (!la_mod->la_enable || !la_mod->la_trigger_enable) {
    CDBG("%s: no trigger update fo LA:LA enable = %d, trigger_enable = %d",
         __func__, la_mod->la_enable, la_mod->la_trigger_enable);
    return 0;
  }

  /* Reset N_pixel to match histogram for histogram offset */
  N_pixels = 0;

  for (i=0; i<256; i++){
    N_pixels += ihist_stats->isp_ihist_data[i];
  }
  offset = (N_pixels * equalize) / 256;

  /* System Team Suggestion:
    Decide when to execute the LA algo.
    Only compute LA curve when luma is within a range of the target 0.5 ~ 2*/
  if (in_params->trigger_input.stats_update.aec_update.target_luma != 0) {
    luma_off_ratio =
      (float)in_params->trigger_input.stats_update.aec_update.cur_luma * 1.0
      / (float)in_params->trigger_input.stats_update.aec_update.target_luma;
  } else {
    /* default value if devide by 0, no execute the algo*/
    ISP_ERR("luma target from AEC = 0! use default LA curve");
    luma_off_ratio = 0;
  }

  if (luma_off_ratio > 0.5 && luma_off_ratio < 2 && chromatix_LA->LA_enable) {
    /* system algo: Compute LA Y->Ynew curve */
    rc = stats_calc_hist_curve(ihist_stats, la_mod, offset, la_curve);
  } else {
    ISP_DBG("not run LA algo,use default curve! cur_luma = %d, tar_luma = %d",
      in_params->trigger_input.stats_update.aec_update.cur_luma,
      in_params->trigger_input.stats_update.aec_update.target_luma);
    /* Initialize a default la curve */
    for (i=0; i<256; i++)
      la_curve[i] = i;
    /* Fake the return value to be normal */
    rc = 0;
  }
  if (rc != 0) {
    /* since we cannot calculate the new curve, use the old table */
    CDBG("%s: calculate new la curve fail, use previous table\n", __func__);
    return 0;
  }

  /* Calculate average only after we rise la_mod->la_curve_is_valid flag */
  if (la_mod->la_curve_is_valid)
    average_la_curve(la_mod->la_curve, la_curve, la_curve_avg);
  else
    memcpy(la_curve_avg, la_curve, sizeof(la_curve_avg));

  /* system algo: to pack la_curve to hw dmi entry*/
  rc = la_prepare_hw_entry(la_mod, la_curve_avg);
  if (rc != 0) {
    CDBG("%s: pack la curve to hw entry fail, use previous table\n", __func__);
    return 0;
  }

  /* save la_curve in isp la mod structure */
  memcpy(la_mod->la_curve, la_curve_avg, sizeof(la_mod->la_curve));
  la_mod->la_curve_is_valid = TRUE;

  la_mod->hw_update_pending = TRUE;
  return rc;
}

#endif
