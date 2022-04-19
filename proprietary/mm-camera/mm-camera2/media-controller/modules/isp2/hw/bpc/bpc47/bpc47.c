/* bpc47.c
 *
 * Copyright (c) 2014-2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "bpc47.h"
#include "abcc_algo.h"
#include "isp_pipeline_reg.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_BPC, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_BPC, fmt, ##args)

#define CLAMP(a,b,c) (((a) <= (b))? (b) : (((a) >= (c))? (c) : (a)))

/** bpc47_cmd_debug:
 *
 *  @cmd: bpc config cmd
 *
 *  This function dumps the bpc module configuration set to hw
 *
 *  Returns nothing
 **/
static void bpc47_cmd_debug(ISP_BPC_Reg_t *cmd)
{
  if (!cmd) {
    ISP_ERR("failed: cmd %p", cmd);
    return;
  }
  ISP_DBG("cmd->BlkLevel = %d", cmd->bpc_config0.val.BlkLevel);
  ISP_DBG("cmd->bpc_cfg0.ColdPixCor_Disable = %d",
          cmd->bpc_config0.val.ColdPixCor_Disable);
  ISP_DBG("cmd->bpc_cfg0.HotPixCor_Disable = %d",
          cmd->bpc_config0.val.HotPixCor_Disable);
  ISP_DBG("cmd->bpc_cfg0.SameChDetect = %d", cmd->bpc_config0.val.SameChDetect);
  ISP_DBG("cmd->bpc_cfg0.SameChRecover = %d", cmd->bpc_config0.val.SameChRecover);
  ISP_DBG("cmd->bpc_cfg0.SqrtSensorGain = %d", cmd->bpc_config0.val.SqrtSensorGain);
  ISP_DBG("cmd->bpc_cfg1.BG_WB_GainRatio = %d",
    cmd->bpc_regcfg.bpc_cfg1.BG_WB_GainRatio);
  ISP_DBG("cmd->bpc_cfg1.RG_WB_GainRatio = %d",
    cmd->bpc_regcfg.bpc_cfg1.RG_WB_GainRatio);
  ISP_DBG("cmd->bpc_cfg2.GB_WB_GainRatio = %d",
    cmd->bpc_regcfg.bpc_cfg2.GB_WB_GainRatio);
  ISP_DBG("cmd->bpc_cfg2.GR_WB_GainRatio = %d",
    cmd->bpc_regcfg.bpc_cfg2.GR_WB_GainRatio);
  ISP_DBG("cmd->bpc_cfg3.BCC_Offset= %d",
    cmd->bpc_regcfg.bpc_cfg3.BCC_Offset);
  ISP_DBG("cmd->bpc_cfg3.BPC_Offset = %d",
    cmd->bpc_regcfg.bpc_cfg3.BPC_Offset);
  ISP_DBG("cmd->bpc_cfg4.CorrectThreshold = %d",
    cmd->bpc_regcfg.bpc_cfg4.CorrectThreshold);
  ISP_DBG("cmd->bpc_cfg4.Fmax = %d", cmd->bpc_regcfg.bpc_cfg4.Fmax);
  ISP_DBG("cmd->bpc_cfg4.Fmin = %d", cmd->bpc_regcfg.bpc_cfg4.Fmin);
  ISP_DBG("cmd->bpc_cfg5.HiglightValueThreshold = %d",
    cmd->bpc_regcfg.bpc_cfg5.HiglightValueThreshold);
  ISP_DBG("cmd->bpc_cfg5.NoiseModelGain = %d",
    cmd->bpc_regcfg.bpc_cfg5.NoiseModelGain);
  ISP_DBG("cmd->bpc_cfg6.HighlightDetectThreshold = %d",
    cmd->bpc_regcfg.bpc_cfg6.HighlightDetectThreshold);
  ISP_DBG("cmd->bpc_cfg7.HighlightDetectParam1 = %d",
    cmd->bpc_regcfg.bpc_cfg7.HighlightDetectParam1);
  ISP_DBG("cmd->bpc_cfg7.HighlightDetectParam2 = %d",
    cmd->bpc_regcfg.bpc_cfg7.HighlightDetectParam2);
  ISP_DBG("cmd->bpc_cfg7.HighlightDetectParam2 = %d",
    cmd->bpc_regcfg.bpc_cfg7.HighlightDetectParam2);
  ISP_DBG("cmd->bpc_cfg7.HighlightDetectParam3 = %d",
    cmd->bpc_regcfg.bpc_cfg7.HighlightDetectParam3);
  ISP_DBG("cmd->bpc_cfg7.HighlightDetectParamSum = %u",
    cmd->bpc_regcfg.bpc_cfg7.HighlightDetectParamSum);
} /* bpc47_cmd_debug */


/** bpc47_set_stream_config:
 *
 *  @module: module
 *  @isp_sub_module: isp sub module
 *  @event: mct event
 *
 *  copy sensor out dimention info
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc47_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  bpc47_t                 *bpc2 = NULL;
  sensor_out_info_t       *sensor_out_info = NULL;

  if (!isp_sub_module || !event || !module) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, event, module);
    return FALSE;
  }

  sensor_out_info =
    (sensor_out_info_t *)event->u.module_event.module_event_data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bpc2 = (bpc47_t *)isp_sub_module->private_data;
  if (!bpc2) {
    ISP_ERR("failed: bpc2 %p", bpc2);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  bpc2->abcc_subblock.abcc_info.sensor_width =
    sensor_out_info->dim_output.width;
  bpc2->abcc_subblock.abcc_info.sensor_height =
    sensor_out_info->dim_output.height;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* bpc47_set_stream_config */

/** bpc47_remove_duplicate_defective_pixels:
 *
 *  @defective_pixels: defective pixel array from sensor
 *
 *  remove duplicate pixels if any in the input defective pix
 *  array
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc47_remove_duplicate_defective_pixels(
  defective_pix_array_t *defective_pixels)
{
  defective_pix_array_t out_defective_pixels;
  pix_t   *pix     = NULL;
  int     count    = 0;
  int     i        = 0;
  int     k        = 0;
  boolean retValue = FALSE;
  int     j;

  if (!defective_pixels) {
    ISP_ERR("failed: defective_pixels %p", defective_pixels);
  } else {
    pix   = defective_pixels->pix;
    count = defective_pixels->count;

    /* Can be optimized by doing this in place*/
    while (i < count) {
      out_defective_pixels.pix[k].x = pix[i].y;
      out_defective_pixels.pix[k].y = pix[i].x;
      k++;
      for(j = i+1; j < count ; j++) {
        if (!PIX_CMP(pix[i], pix[j]))
          break;
      }
      i = j;
    }

    ISP_DBG("%s: count %d", __func__, k);
    out_defective_pixels.count = k;

    *defective_pixels = out_defective_pixels;

    retValue = TRUE;
  }

  return retValue;
} /* end bpc47_remove_duplicate_defective_pixels */


/** bpc47_abcc_prepare_lut_tables:
 *
 *  @abcc: abcc subblock data
 *
 *  prepare LUT tables for HW config
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc47_abcc_prepare_lut_tables(abcc_t *abcc)
{
  abcc_packed_lut_t *packed_lut = &abcc->packed_lut;
  abcc_entry_t *entry;
  int          pix_index;
  boolean      retValue = FALSE;
  int          width    = 0;
  int          count    = 0;
  int          i        = 0;

  if (!abcc) {
    ISP_ERR("failed: abcc %p", abcc);
  } else {
    width = abcc->input_width;
    count = abcc->abcc_info.lut.final_lut_list.actual_count;
    ISP_DBG("count %d", count);

    for (i = 0; i < count; i++) {
      entry = &(abcc->abcc_info.lut.final_lut_list.entry[i]);
      pix_index = PIX_IND(entry->pixel_index, width);
      packed_lut->lut[i] = (pix_index << 9);
      packed_lut->lut[i] |= (entry->skip_index[1] << 6);
      packed_lut->lut[i] |= (entry->skip_index[0] << 3);
      packed_lut->lut[i] |= entry->kernel_index;
      ISP_DBG("%s: Final LUT[%d] pix_index %d skip_index %d %d kern_ind %d final %llx",
        __func__, i, pix_index, entry->skip_index[0], entry->skip_index[1],
        entry->kernel_index, packed_lut->lut[i]);
    }
    for (; i < ABCC_LUT_COUNT; i++) {
      entry = &(abcc->abcc_info.lut.final_lut_list.entry[count-1]);
      pix_index = PIX_IND(entry->pixel_index, width);
      packed_lut->lut[i] = (pix_index << 9);
      packed_lut->lut[i] |= (entry->skip_index[1] << 6);
      packed_lut->lut[i] |= (entry->skip_index[0] << 3);
      packed_lut->lut[i] |= entry->kernel_index;
      ISP_DBG("final_lut_list[%d] pix_index %d skip_index %d %d kern_ind %d final %llx",
        i, pix_index, entry->skip_index[0], entry->skip_index[1],
        entry->kernel_index, packed_lut->lut[i]);
    }
    retValue = TRUE;
  }

  return retValue;
}

/** bpc47_run_abcc_algo:
 *
 *  @abcc: abcc sub block info
 *  Runs the abcc algo and prepares the LUT tab
 *
 **/
static boolean bpc47_run_abcc_algo(abcc_t *abcc)
{
  int ret = TRUE;
  if (!abcc) {
    ISP_ERR("failed: abcc %p", abcc);
    return FALSE;
  }

  if (abcc->abcc_info.defect_pixels.count == 0) {
    ISP_DBG("No defective pixels set by sensor event");
    return FALSE;
  }
  abcc->abcc_info.forced_correction = ABCC_FORCED_CORRECTION_ENABLE;

  ret = bpc47_remove_duplicate_defective_pixels(
    &abcc->abcc_info.defect_pixels);
  if (ret == FALSE) {
    ISP_ERR("failed bpc47_remove_duplicate_defective_pixels");
    return FALSE;
  }
  ISP_HIGH("%s: algo start", __func__);
  /* Based on the number of defect pixels received from sensor allocate memory
     for the algo params*/
  ret = abcc_algo_init(&abcc->abcc_info);
  if (ret == FALSE) {
    ISP_ERR("failed abcc_algo_init");
    return FALSE;
  }

  ret = abcc_algo_process(&abcc->abcc_info, ABCC_LUT_COUNT);
  if (ret == FALSE) {
    ISP_ERR("abcc_algo_process failed");
    return FALSE;
  }

  ret = bpc47_abcc_prepare_lut_tables(abcc);
  if (ret == FALSE) {
    ISP_ERR("failed: bpc47_abcc_prepare_lut_tables");
    return FALSE;
  }
  /* Algo params are no more needed as this is run only once per session so
     so free the memory*/
  abcc_algo_deinit(&abcc->abcc_info);

  ISP_HIGH("%s: algo end", __func__);
  abcc->abcc_enable = 1;
  abcc->hw_update_pending = 1;

  return ret;
}/* bpc47_abcc_config */

/** bpc47_convert_to_int:
 *
 *  @val: input signed char variable to be converted to int
 *  Convert char to int
 *
 *  Returns converted int value
 **/
int bpc47_convert_to_int(char val)
{
  int num = val;
  if (num > 127) {
    num = -(256 - num);
  }
  return num;
}
/** bpc47_clamp_min_max:
 *
 *  @val: input char variable to be clamped
 *  @min: minimum value
 *  @max: maximum value
 *  @num_bits: number of bits of the final clamped value
 *
 *  CLAMP the input to min/max and adjust the final bits to the
 *  desired bits in num_bits
 *  Returns clamped value
 **/
uint32_t bpc47_clamp_min_max(char val, int min, int max, int num_bits)
{
  int num = 0;
  num = bpc47_convert_to_int(val);
  num = CLAMP(num, min, max);
  if (num < 0) {
    num = -num;
    num = (1 << num_bits) - num;
  }
  return num;
}

/** bpc47_cmd_config:
 *
 *  @mod: bpc module struct data
 *
 *  Copy from mod->chromatix params to reg cmd then configure
 *
 *  Returns nothing
 **/
static void bpc47_init_config(isp_sub_module_t *isp_sub_module, bpc47_t *bpc2)
{
  ISP_BPC_Reg_t                     *Reg = NULL;
  chromatix_parms_type              *chromatix_ptr = NULL;
  Chromatix_BPC2_type               *chromatix_BPC2 = NULL;
  BPC2_reserved_type                *bpc2_reserved_data= NULL;
  BPC2_core_type                    *bpc2_data = NULL;
  chromatix_VFE_common_type         *pchromatix_common = NULL;
  Chromatix_blk_subtract_scale_type *chromatix_blk_lvl = NULL;

  if (!isp_sub_module || !bpc2) {
    ISP_ERR("failed: %p %p", isp_sub_module, bpc2);
    return;
  }

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return;
  }

  bpc2_reserved_data =
    &chromatix_ptr->chromatix_VFE.chromatix_BPC2_data.BPC2_reserved_data;
  chromatix_BPC2 =
    &chromatix_ptr->chromatix_VFE.chromatix_BPC2_data;

  pchromatix_common =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!pchromatix_common ) {
    ISP_ERR("failed: pchromatix_common %p",pchromatix_common);
    return;
  }
  chromatix_blk_lvl =
    &pchromatix_common->Chromatix_BLSS_data;
  if (!chromatix_blk_lvl ) {
    ISP_ERR("failed: chromatix_blk_lvl %p",chromatix_blk_lvl);
    return;
  }

  Reg = &bpc2->Reg;
  Reg->bpc_config0.mask.HotPixCor_Disable = 1;
  Reg->bpc_config0.val.HotPixCor_Disable = CLAMP(
    bpc2_reserved_data->hot_pixel_correction_disable, 0, 1);
  Reg->bpc_config0.mask.ColdPixCor_Disable = 1;
  Reg->bpc_config0.val.ColdPixCor_Disable = CLAMP(
    bpc2_reserved_data->cold_pixel_correction_disable, 0, 1);
  Reg->bpc_regcfg.bpc_cfg7.HighlightDetectParam1 =
    CLAMP(bpc2_reserved_data->highlight_detection_param1, BPC_MIN_HDP1,
    BPC_MAX_HDP1);
  Reg->bpc_regcfg.bpc_cfg7.HighlightDetectParam2 =
    bpc47_clamp_min_max(bpc2_reserved_data->highlight_detection_param2,
    BPC_MIN_HDP, BPC_MAX_HDP, 4);
  Reg->bpc_regcfg.bpc_cfg7.HighlightDetectParam3 =
    bpc47_clamp_min_max(bpc2_reserved_data->highlight_detection_param3,
    BPC_MIN_HDP, BPC_MAX_HDP, 4);
  Reg->bpc_regcfg.bpc_cfg7.HighlightDetectParamSum = CLAMP(
    bpc2_reserved_data->highlight_detection_param1 +
    bpc47_convert_to_int(bpc2_reserved_data->highlight_detection_param2) +
    bpc47_convert_to_int(bpc2_reserved_data->highlight_detection_param3),
    BPC_MIN_HDP_SUM, BPC_MAX_HDP_SUM);

  bpc2_data = &chromatix_BPC2->BPC2_data[DEFAULT_CONFIG_INDEX];
  Reg->bpc_config0.mask.SameChDetect = 1;
  Reg->bpc_config0.val.SameChDetect =
    CLAMP(bpc2_data->same_channel_detection, 0, 1);
  Reg->bpc_config0.mask.SameChRecover = 1;
  Reg->bpc_config0.val.SameChRecover =
    CLAMP(bpc2_data->same_channel_recovery, 0, 1);
  Reg->bpc_regcfg.bpc_cfg4.Fmax =
    CLAMP(bpc2_data->fmax, BPC_MIN_FMAX, BPC_MAX_FMAX);
  Reg->bpc_regcfg.bpc_cfg4.Fmin =
    CLAMP(bpc2_data->fmin, BPC_MIN_FMIN, BPC_MAX_FMIN);
  Reg->bpc_regcfg.bpc_cfg3.BPC_Offset =
    CLAMP(bpc2_data->BPC_offset, BPC_MIN_OFFSET, BPC_MAX_OFFSET);
  Reg->bpc_regcfg.bpc_cfg3.BCC_Offset =
    CLAMP(bpc2_data->BCC_offset, BPC_MIN_OFFSET, BPC_MAX_OFFSET);
  Reg->bpc_regcfg.bpc_cfg4.CorrectThreshold =
    CLAMP(bpc2_data->correction_threshold, BPC_MIN_COR_THR, BPC_MAX_COR_THR);
  Reg->bpc_regcfg.bpc_cfg5.NoiseModelGain =
    CLAMP(bpc2_data->noise_model_gain, BPC_MIN_NMG, BPC_MAX_NMG);
  Reg->bpc_regcfg.bpc_cfg5.HiglightValueThreshold =
    CLAMP(bpc2_data->highlight_value_threshold, BPC_MIN_HVT, BPC_MAX_HVT);
  Reg->bpc_regcfg.bpc_cfg6.HighlightDetectThreshold = CLAMP(
    bpc2_data->highlight_detection_threshold, BPC_MIN_HDT, BPC_MAX_HDT);
  bpc2->Reg.bpc_config0.mask.BlkLevel = (1 << NUM_BLKLEVEL_BITS) - 1;
  if (chromatix_blk_lvl->BLSS_enable) {
    bpc2->Reg.bpc_config0.val.BlkLevel = CLAMP(
      chromatix_blk_lvl->black_level_normal.black_level_offset,
      BPC_MIN_BLKLVL, BPC_MAX_BLKLVL);
  } else {
    bpc2->Reg.bpc_config0.val.BlkLevel = 0;
  }
} /* bpc47_cmd_config */

/** bpc47_write_lut_to_dmi:
 *
 *  @isp_sub_module: isp base module
 *  @mod: module data
 *
 *  writes abcc LUT table to DMI
 *
 **/
boolean bpc47_write_lut_to_dmi(isp_sub_module_t *isp_sub_module, abcc_t *mod)
{
  boolean   ret = TRUE;
  uint32_t  lut_size = sizeof(uint64_t) * ABCC_LUT_COUNT;
  ret = isp_sub_module_util_write_dmi(
    (void*)&mod->packed_lut, lut_size,
    BPC_LUT_RAM_BANK,
    VFE_WRITE_DMI_64BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: bpc47 isp_sub_module_util_write_dmi");
  }
  return ret;
}

/** bpc47_do_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bpc: bpc struct data
 *
 * update BPC module register to kernel
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean bpc47_store_hw_update(isp_sub_module_t *isp_sub_module,
  bpc47_t *bpc2)
{
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  ISP_BPCRegConfig            *RegCmd = NULL;
  uint32_t                    *val = NULL;

  if (!isp_sub_module || !bpc2) {
    ISP_ERR("failed: %p %p", isp_sub_module, bpc2);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd)
    * ISP_BPC_TOTAL_NUM_REG);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd) * ISP_BPC_TOTAL_NUM_REG);

  RegCmd = (ISP_BPCRegConfig *)CAM_MALLOC(sizeof(*RegCmd));
  if (!RegCmd) {
    ISP_ERR("failed: RegCmd %p", RegCmd);
    goto ERROR_BPC_CMD;
  }
  *RegCmd = bpc2->Reg.bpc_regcfg;
  if (bpc2->abcc_subblock.abcc_enable &&
      bpc2->abcc_subblock.hw_update_pending == TRUE &&
      bpc2->abcc_subblock.one_time_config_done == FALSE) {
    bpc47_write_lut_to_dmi(isp_sub_module, &bpc2->abcc_subblock);
    bpc2->abcc_subblock.hw_update_pending = FALSE;
    bpc2->abcc_subblock.one_time_config_done = TRUE;
  }
  cfg_cmd->cfg_data = (void *)RegCmd;
  cfg_cmd->cmd_len = sizeof(*RegCmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = ISP_BPC_TOTAL_NUM_REG;

  /* write ISP_BPCCfg0 register */

  reg_cfg_cmd[0].u.mask_info.reg_offset = ISP_BPC_CFG0_OFF;
  val = (uint32_t *)&bpc2->Reg.bpc_config0.mask;
  reg_cfg_cmd[0].u.mask_info.mask = *val;
  val = (uint32_t *)&bpc2->Reg.bpc_config0.val;
  reg_cfg_cmd[0].u.mask_info.val = *val;
  reg_cfg_cmd[0].cmd_type  = VFE_CFG_MASK;

  /* write ISP_BPCCfg1 register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[1],
    0, sizeof(RegCmd->bpc_cfg1), VFE_WRITE, ISP_BPC_CFG1_OFF);

  /* write ISP_BPCCfg2 register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[2],
    reg_cfg_cmd[1].u.rw_info.cmd_data_offset + reg_cfg_cmd[1].u.rw_info.len,
    sizeof(RegCmd->bpc_cfg2), VFE_WRITE, ISP_BPC_CFG2_OFF);

  /* write ISP_BPCCfg3 register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[3],
    reg_cfg_cmd[2].u.rw_info.cmd_data_offset + reg_cfg_cmd[2].u.rw_info.len,
    sizeof(RegCmd->bpc_cfg3), VFE_WRITE, ISP_BPC_CFG3_OFF);

  /* write ISP_BPCCfg4 register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[4],
    reg_cfg_cmd[3].u.rw_info.cmd_data_offset + reg_cfg_cmd[3].u.rw_info.len,
    sizeof(RegCmd->bpc_cfg4), VFE_WRITE, ISP_BPC_CFG4_OFF);

  /* write ISP_BPCCfg5 register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[5],
    reg_cfg_cmd[4].u.rw_info.cmd_data_offset + reg_cfg_cmd[4].u.rw_info.len,
    sizeof(RegCmd->bpc_cfg5), VFE_WRITE, ISP_BPC_CFG5_OFF);

  /* write ISP_BPCCfg6 register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[6],
    reg_cfg_cmd[5].u.rw_info.cmd_data_offset + reg_cfg_cmd[5].u.rw_info.len,
    sizeof(RegCmd->bpc_cfg6), VFE_WRITE, ISP_BPC_CFG6_OFF);

  /* write ISP_BPCCfg7 register */
  isp_sub_module_util_pack_cfg_cmd(&reg_cfg_cmd[7],
    reg_cfg_cmd[6].u.rw_info.cmd_data_offset + reg_cfg_cmd[6].u.rw_info.len,
    sizeof(RegCmd->bpc_cfg6), VFE_WRITE, ISP_BPC_CFG7_OFF);

  bpc47_cmd_debug(&bpc2->Reg);

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;
  }

  return TRUE;

ERROR_APPEND:
  free(RegCmd);
ERROR_BPC_CMD:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD:
  free(hw_update);
  return FALSE;
} /* bpc47_do_hw_update */

/** bpc47_interpolate:
 *
 *  @isp_sub_mod: isp sub module handle
 *  @bpc2: bpc struct data
 *
 * interpolate bpc tate with the aec ratio
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean bpc47_interpolate(isp_sub_module_t *isp_sub_mod, bpc47_t *bpc2)
{
  BPC2_core_type           *bpc2_data_start = NULL;
  BPC2_core_type           *bpc2_data_end = NULL;
  BPC2_core_type           *bpc2_data = NULL;
  chromatix_parms_type     *chromatix_ptr = NULL;
  Chromatix_BPC2_type      *chromatix_bpc2 = NULL;
  chromatix_VFE_common_type *pchromatix_common = NULL;
  float                     aec_ratio_bpc = 0;
  awb_gain_t               *awb_gain;
  chromatix_videoHDR_type  *chromatix_VHDR = NULL;
  bpc_params_t             *bpc_params = NULL;
  float                       multifactor = 0;

  if (!isp_sub_mod || !bpc2) {
    ISP_ERR("failed isp_sub_mod %p bpc2 %p", isp_sub_mod, bpc2);
    return FALSE;
  }

  awb_gain = &bpc2->curr_awb_gain;
  bpc_params = &bpc2->bpc_params;
  aec_ratio_bpc = bpc2->aec_ratio_bpc.ratio;
  chromatix_ptr = isp_sub_mod->chromatix_ptrs.chromatixPtr;
  pchromatix_common =
    (chromatix_VFE_common_type *)isp_sub_mod->chromatix_ptrs.chromatixComPtr;
  if (!chromatix_ptr || !pchromatix_common) {
    ISP_ERR("failed: chromatix_ptr %p, pchromatix_common %p",
      chromatix_ptr, pchromatix_common);
    return FALSE;
  }

  chromatix_bpc2 = &chromatix_ptr->chromatix_VFE.chromatix_BPC2_data;
  if (!chromatix_bpc2 ) {
    ISP_ERR("failed: chromatix_bpc2 %p",chromatix_bpc2);
    return FALSE;
  }

  bpc2_data_start = &chromatix_bpc2->BPC2_data[bpc2->trigger_index];

  if ((aec_ratio_bpc > 0) &&
      ((bpc2->trigger_index + 1) <= MAX_LIGHT_TYPES_FOR_SPATIAL)) {
    bpc2_data_end = &chromatix_bpc2->BPC2_data[bpc2->trigger_index + 1];

    bpc_params->Fmax = roundf(LINEAR_INTERPOLATION((float)bpc2_data_end->fmax,
      (float)bpc2_data_start->fmax, aec_ratio_bpc));
    bpc_params->Fmin = roundf(LINEAR_INTERPOLATION((float)bpc2_data_end->fmin,
      (float)bpc2_data_start->fmin, aec_ratio_bpc));
    bpc_params->BPC_offset = LINEAR_INTERPOLATION((float)bpc2_data_end->BPC_offset,
      (float)bpc2_data_start->BPC_offset, aec_ratio_bpc);
    bpc_params->BCC_offset = LINEAR_INTERPOLATION((float)bpc2_data_end->BCC_offset,
      (float)bpc2_data_start->BCC_offset, aec_ratio_bpc);
    bpc_params->correction_threshold = LINEAR_INTERPOLATION(
      (float)bpc2_data_end->correction_threshold,
      (float)bpc2_data_start->correction_threshold,
      aec_ratio_bpc);
    bpc_params->noise_model_gain = roundf(LINEAR_INTERPOLATION(
      (float)bpc2_data_end->noise_model_gain, (float)bpc2_data_start->noise_model_gain,
      aec_ratio_bpc));
    bpc_params->highlight_detection_threshold =
      LINEAR_INTERPOLATION((float)bpc2_data_end->highlight_detection_threshold,
      (float)bpc2_data_start->highlight_detection_threshold, aec_ratio_bpc);
    bpc_params->highlight_value_threshold =
      LINEAR_INTERPOLATION((float)bpc2_data_end->highlight_value_threshold,
      (float)bpc2_data_start->highlight_value_threshold, aec_ratio_bpc);
  } else {
    bpc_params->Fmax = bpc2_data_start->fmax;
    bpc_params->Fmin = bpc2_data_start->fmin;
    bpc_params->BPC_offset = bpc2_data_start->BPC_offset;
    bpc_params->BCC_offset = bpc2_data_start->BCC_offset;
    bpc_params->correction_threshold = bpc2_data_start->correction_threshold;
    bpc_params->noise_model_gain = bpc2_data_start->noise_model_gain;
    bpc_params->highlight_detection_threshold =
      bpc2_data_start->highlight_detection_threshold;
    bpc_params->highlight_value_threshold = bpc2_data_start->highlight_value_threshold;
  }
  /*when HDR mode, chromatix tuned by 14 bit,
    otherwise keep 12 bit header for backward compatible*/
  chromatix_VHDR =
    &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;
  if (chromatix_VHDR->videoHDR_reserve_data.hdr_recon_en == 0 &&
      chromatix_VHDR->videoHDR_reserve_data.hdr_mac_en == 0) {
      //MSB alligned data
    multifactor = 1.0f * (1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH));
  } else{
    multifactor = 1.0f;
  }
  if (isp_sub_mod->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
      isp_sub_mod->hdr_mode == CAM_SENSOR_HDR_ZIGZAG ){
    //LSB alligned data
      multifactor = 1.0f /(1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH));
  }

  bpc_params->BPC_offset = roundf(CLAMP(bpc_params->BPC_offset * multifactor,
    BPC_MIN_OFFSET, BPC_MAX_OFFSET));
  bpc_params->BCC_offset = roundf(CLAMP(bpc_params->BCC_offset * multifactor,
    BPC_MIN_OFFSET, BPC_MAX_OFFSET));
  bpc_params->correction_threshold = roundf(CLAMP(
    bpc_params->correction_threshold * multifactor, BPC_MIN_COR_THR,
    BPC_MAX_COR_THR));
  bpc_params->highlight_detection_threshold =
     roundf(CLAMP(bpc_params->highlight_detection_threshold * multifactor,
     BPC_MIN_HDT, BPC_MAX_HDT));
  bpc_params->highlight_value_threshold =
     roundf(CLAMP(bpc_params->highlight_value_threshold * multifactor,
     BPC_MIN_HVT, BPC_MAX_HVT));

  bpc_params->Fmax = CLAMP(bpc_params->Fmax, BPC_MIN_FMAX, BPC_MAX_FMAX);
  bpc_params->Fmin = CLAMP(bpc_params->Fmin, BPC_MIN_FMIN, BPC_MAX_FMIN);
  bpc_params->noise_model_gain = CLAMP(bpc_params->noise_model_gain,
    BPC_MIN_NMG, BPC_MAX_NMG);
  bpc_params->SameChannelDetect = CLAMP(bpc2_data_start->same_channel_detection,
    0, 1);
  bpc_params->SameChannelRecovery = CLAMP(bpc2_data_start->same_channel_recovery,
    0, 1);

  return TRUE;
}

/** bpc47_hw_config:
 *
 *  @isp_sub_mod: isp sub module handle
 *  @bpc2: bpc struct data
 *
 *  Configure BPC2 hawrdware registers
 * Returns TRUE on success and FALSE on failure
 **/
static boolean bpc47_hw_config(isp_sub_module_t *isp_sub_mod, bpc47_t *bpc2)
{
  awb_gain_t               *awb_gain;
  bpc_params_t             *bpc_params;
  if (!isp_sub_mod || !bpc2) {
    ISP_ERR("failed isp_sub_mod %p bpc2 %p", isp_sub_mod, bpc2);
    return FALSE;
  }
  bpc_params = &bpc2->bpc_params;
  awb_gain = &bpc2->curr_awb_gain;

  bpc2->Reg.bpc_regcfg.bpc_cfg4.Fmax = bpc_params->Fmax;
  bpc2->Reg.bpc_regcfg.bpc_cfg4.Fmin = bpc_params->Fmin;
  bpc2->Reg.bpc_regcfg.bpc_cfg3.BPC_Offset = bpc_params->BPC_offset;
  bpc2->Reg.bpc_regcfg.bpc_cfg3.BCC_Offset = bpc_params->BCC_offset;
  bpc2->Reg.bpc_regcfg.bpc_cfg4.CorrectThreshold = bpc_params->correction_threshold;
  bpc2->Reg.bpc_regcfg.bpc_cfg5.NoiseModelGain = bpc_params->noise_model_gain;
  bpc2->Reg.bpc_regcfg.bpc_cfg5.HiglightValueThreshold =
   bpc_params->highlight_value_threshold;
  bpc2->Reg.bpc_regcfg.bpc_cfg6.HighlightDetectThreshold =
   bpc_params->highlight_detection_threshold;
  bpc2->Reg.bpc_config0.mask.BlkLevel = (1 << NUM_BLKLEVEL_BITS) - 1;
  bpc2->Reg.bpc_config0.val.BlkLevel = bpc_params->BlkLvl_offset;
  bpc2->Reg.bpc_config0.mask.SameChDetect = 1;
  bpc2->Reg.bpc_config0.val.SameChDetect = bpc_params->SameChannelDetect;
  bpc2->Reg.bpc_config0.mask.SameChRecover = 1;
  bpc2->Reg.bpc_config0.val.SameChRecover = bpc_params->SameChannelRecovery;

  bpc2->Reg.bpc_regcfg.bpc_cfg1.RG_WB_GainRatio = CLAMP(
    FLOAT_TO_Q(12, awb_gain->r_gain/awb_gain->g_gain), BPC_MIN_AWB_GAIN_RATIO,
    BPC_MAX_AWB_GAIN_RATIO);
  bpc2->Reg.bpc_regcfg.bpc_cfg1.BG_WB_GainRatio = CLAMP(
    FLOAT_TO_Q(12, awb_gain->b_gain/awb_gain->g_gain), BPC_MIN_AWB_GAIN_RATIO,
    BPC_MAX_AWB_GAIN_RATIO);
  bpc2->Reg.bpc_regcfg.bpc_cfg2.GB_WB_GainRatio = CLAMP(
    FLOAT_TO_Q(12, awb_gain->g_gain/awb_gain->b_gain), BPC_MIN_AWB_GAIN_RATIO,
    BPC_MAX_AWB_GAIN_RATIO);
  bpc2->Reg.bpc_regcfg.bpc_cfg2.GR_WB_GainRatio = CLAMP(
    FLOAT_TO_Q(12, awb_gain->g_gain/awb_gain->r_gain), BPC_MIN_AWB_GAIN_RATIO,
    BPC_MAX_AWB_GAIN_RATIO);

  bpc2->Reg.bpc_config0.mask.SqrtSensorGain = (1 << SQRT_SENSOR_GAIN_BITS) - 1;
  bpc2->Reg.bpc_config0.val.SqrtSensorGain = CLAMP(
    FLOAT_TO_Q(4, sqrtf(bpc2->curr_sensor_gain)), BPC_MIN_SENSOR_GAIN,
    BPC_MAX_SENSOR_GAIN);

  return TRUE;
}


/** bpc47_set_defective_pixel_for_abcc:
 *
 * @mod: bpc module
 * @isp_sub_module: isp_sub_module
 * @event: mct event
 *
 * Handle Set Defective pixel event from sensor
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean bpc47_set_defective_pixel_for_abcc(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  bpc47_t              *bpc2 = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  bpc2 = (bpc47_t *)isp_sub_module->private_data;
  if (!bpc2) {
    ISP_ERR("failed: bpc2 %p", bpc2);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bpc2->abcc_subblock.abcc_info.defect_pixels =
    *(defective_pix_array_t *)event->u.module_event.module_event_data;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** bpc47_streamon:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function adds ref count for stream on flag and sets
 *  trigger_update_pending flag to TRUE for first STREAM ON
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc47_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;
  bpc47_t               *bpc2;
  abcc_t                *abcc;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp_sub_module_priv) {
    ISP_ERR("failed: isp_sub_module_priv %p", isp_sub_module_priv);
    return FALSE;
  }
  bpc2 = (bpc47_t *)isp_sub_module->private_data;
  if (!bpc2) {
    ISP_ERR("failed: bpc2 %p", bpc2);
    return FALSE;
  }
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  abcc = &bpc2->abcc_subblock;
  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }
  /* Abcc Algo and config is only done once per session.
     Run the algo if it is 1st streamon */
  if (isp_sub_module->stream_on_count == 1) {
    abcc->one_time_config_done = FALSE;
    ret = bpc47_run_abcc_algo(abcc);
  }

  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->config_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

} /* bpc47_streamon */

/** bpc47_ez_isp_update
 *
 *  @mod: bpc module handle
 *  @bpcDiag: bpc Diag handle
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void bpc47_ez_isp_update(bpc47_t *mod,
  badcorrection_t *bpcDiag)
{
  ISP_BPC_Reg_t *bpcCfg = &(mod->Reg);

  /* TODO: Filling only known metadata in extune structure.
   * When the new eztune header file is released, need
   * to fill all the required metadata */
  bpcDiag->fminThreshold = bpcCfg->bpc_regcfg.bpc_cfg4.Fmin;
  bpcDiag->fmaxThreshold = bpcCfg->bpc_regcfg.bpc_cfg4.Fmax;
}/* bpc47_ez_isp_update */

/** bpc47_fill_vfe_diag_data:
 *
 *  @bpc: bpc module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean bpc47_fill_vfe_diag_data(bpc47_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean              ret = TRUE;
  badcorrection_t     *bpcDiag = NULL;
  vfe_diagnostics_t   *vfe_diag = NULL;

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    bpcDiag = &(vfe_diag->prev_bpc);

    bpc47_ez_isp_update(mod, bpcDiag);
  }
  return ret;
}
/** bpc47_fetch_blkLvl_offset:
 *
 *  @module: mct module
 *  @identity: stream identity
 *  @gain:
 *  Fetch Black Level offset
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bpc47_fetch_blkLvl_offset(mct_module_t *module,
  uint32_t identity, uint32_t *BlkLvl_offset)
{
  boolean                               ret = TRUE;
  mct_event_t                           mct_event;
  isp_private_event_t                   private_event;
  uint32_t                              blklvl;

  if (!module || !BlkLvl_offset) {
    ISP_ERR("failed: %p %p", module, BlkLvl_offset);
    return FALSE;
  }

  /*Get black level offset info */
  ISP_DBG("E: Get black level offset info");
  memset(&private_event, 0, sizeof(isp_private_event_t));
  private_event.type = ISP_PRIVATE_FETCH_BLKLVL_OFFSET;
  private_event.data = (void *)&blklvl;
  private_event.data_size = sizeof(uint32_t);

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.identity = identity;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.direction = MCT_EVENT_UPSTREAM;
  mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  mct_event.u.module_event.module_event_data = (void *)&private_event;
  ret = isp_sub_mod_util_send_event(module, &mct_event);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_mod_util_send_event");
    return ret;
  }
  *BlkLvl_offset = blklvl;
  ISP_DBG("Black Level offset %d", *BlkLvl_offset);
  return ret;
}

/** bpc47_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Perform trigger update if trigger_update_pending flag is
 *  TRUE and append hw update list in global list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bpc47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                       ret = TRUE;
  bpc47_t                      *bpc2 = NULL;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;
  abcc_t                       *abcc;
  uint32_t                      BlkLvl_offset = 0;
  uint8_t                      module_enable;

  if (!isp_sub_module || !event || !module) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, event, module);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bpc2 = (bpc47_t *)isp_sub_module->private_data;

  if (!bpc2) {
    ISP_ERR("failed: bpc2 %p", bpc2);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  private_event = (isp_private_event_t *)
                    (event->u.module_event.module_event_data);
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (!isp_sub_module->chromatix_module_enable ||
      !isp_sub_module->submod_trigger_enable) {
    ISP_DBG("skip trigger chromatix enable %d, trig_enable %d",
      isp_sub_module->chromatix_module_enable,
      isp_sub_module->submod_trigger_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    goto FILL_METADATA;
  }

  if (bpc2->ext_func_table && bpc2->ext_func_table->check_enable_idx) {
    bpc2->ext_func_table->check_enable_idx(bpc2, isp_sub_module, output);
    if (isp_sub_module->submod_enable) {
      bpc47_decide_hysterisis(isp_sub_module, bpc2->aec_reference);
    }
  }
  else {
    bpc47_decide_hysterisis(isp_sub_module, bpc2->aec_reference);
  }

  private_event = (isp_private_event_t *)event->u.module_event.
    module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    goto FILL_METADATA;
  }

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (isp_sub_module->manual_ctrls.manual_update &&
      isp_sub_module->chromatix_module_enable) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    module_enable = (isp_sub_module->manual_ctrls.hot_pix_mode ==
                     CAM_HOTPIXEL_MODE_OFF) ? FALSE : TRUE;

    if (isp_sub_module->submod_enable != module_enable) {
      isp_sub_module->submod_enable = module_enable;
      output->stats_params->module_enable_info.reconfig_needed = TRUE;
      output->stats_params->module_enable_info.
        submod_enable[isp_sub_module->hw_module_id] = module_enable;
      output->stats_params->module_enable_info.
        submod_mask[isp_sub_module->hw_module_id] = 1;
    }
  }

  ret = bpc47_fetch_blkLvl_offset(module, event->identity,
    &BlkLvl_offset);
  if (bpc2->bpc_params.BlkLvl_offset != BlkLvl_offset) {
    bpc2->bpc_params.BlkLvl_offset = BlkLvl_offset;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  if ((isp_sub_module->submod_trigger_enable == TRUE) &&
    (isp_sub_module->trigger_update_pending == TRUE)) {
    ret = bpc47_interpolate(isp_sub_module, bpc2);
    if (ret == FALSE) {
      ISP_ERR("failed: bpc47_interpolate");
      goto FILL_METADATA;
    }
    ret = bpc47_hw_config(isp_sub_module, bpc2);
    if (ret == FALSE) {
      ISP_ERR("failed: bpc47_config");
      goto FILL_METADATA;
    }
    isp_sub_module->trigger_update_pending = FALSE;

    bpc47_store_hw_update(isp_sub_module, bpc2);
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto FILL_METADATA;
  }

FILL_METADATA:
  if (output && isp_sub_module->vfe_diag_enable) {
    ret = bpc47_fill_vfe_diag_data(bpc2, isp_sub_module, output);
    if (ret == FALSE) {
      ISP_ERR("failed: bpc44_fill_vfe_diag_data");
    }
  }
  if (output->frame_meta)
    output->frame_meta->hot_pix_mode =
      isp_sub_module->manual_ctrls.hot_pix_mode;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
}

/** bpc47_stats_awb_update:
 *
 * @mod: bpc47 module
 * @data: handle to stats_update_t
 *
 * Handle AWB update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean bpc47_stats_awb_update(isp_sub_module_t *isp_sub_module, void *data)
{
  stats_update_t                      *stats_update = NULL;
  awb_gain_t                          *cur_awb_gain = NULL;
  awb_gain_t                          *new_awb_gain = NULL;
  bpc47_t                             *bpc47;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }
  stats_update = (stats_update_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  bpc47 = isp_sub_module->private_data;
  if (!bpc47) {
    ISP_ERR("failed %p", bpc47);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  new_awb_gain = &stats_update->awb_update.gain;
  cur_awb_gain = &bpc47->curr_awb_gain;

  if (!AWB_GAIN_EQUAL(new_awb_gain, cur_awb_gain) &&
       !AWB_GAIN_EQ_ZERO(new_awb_gain)) {
     isp_sub_module->trigger_update_pending = TRUE;

     ISP_DBG("old gain g=%f b=%f r=%f new g=%f b=%f r=%f",
       cur_awb_gain->g_gain, cur_awb_gain->b_gain,
       cur_awb_gain->r_gain, new_awb_gain->g_gain,
       new_awb_gain->b_gain, new_awb_gain->r_gain);
      *cur_awb_gain = *new_awb_gain;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

}

/** bpc47_decide_hysterisis:
 *
 *  @module:  module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean bpc47_decide_hysterisis(
  isp_sub_module_t *isp_sub_module, float aec_ref)
{
  chromatix_parms_type *chromatix_ptr  = NULL;
  Chromatix_BPC2_type  *chromatix_bpc  = NULL;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_bpc =
    &chromatix_ptr->chromatix_VFE.chromatix_BPC2_data;

  return isp_sub_module_util_decide_hysterisis(isp_sub_module,
    aec_ref, chromatix_bpc->BPC2_en, chromatix_bpc->bpc2_snapshot_hyst_en_flag,
    chromatix_bpc->control_BPC2, &chromatix_bpc->bpc2_hysteresis_trigger);
}

/** bpc47_stats_aec_update:
 *
 * @mod: bpc47 module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean bpc47_stats_aec_update(isp_sub_module_t *isp_sub_module, void *data)
{
  stats_update_t                       *stats_update = NULL;
  bpc47_t                              *bpc2 = NULL;
  chromatix_parms_type                 *chromatix_ptr = NULL;
  Chromatix_BPC2_type                  *chromatix_BPC2 = NULL;
  Chromatix_blk_subtract_scale_type    *chromatix_blk_lvl = NULL;
  chromatix_VFE_common_type            *pchromatix_common = NULL;
  uint8_t                               trigger_index =
                                        MAX_LIGHT_TYPES_FOR_SPATIAL + 1;
  float                                 aec_reference;
  float                                 aec_ratio = 0;
  float                                 aec_ratio_blklvl = 0;
  float                                 ratio = 0;
  float                                 new_sensor_gain = 0;
  int                                   i = 0;
  float start = 0;
  float end = 0;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  bpc2 = (bpc47_t *)isp_sub_module->private_data;
  if (!bpc2) {
    ISP_ERR("failed: bpc %p", bpc2);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
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

  bpc2->aec_update = stats_update->aec_update;

  chromatix_BPC2 =
    &chromatix_ptr->chromatix_VFE.chromatix_BPC2_data;
  pchromatix_common =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!chromatix_BPC2 || !pchromatix_common) {
    ISP_ERR("failed: chromatix_BPC2 %p, pchromatix_common %p",
      chromatix_BPC2, pchromatix_common);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  chromatix_blk_lvl =
    &pchromatix_common->Chromatix_BLSS_data;

  if (chromatix_BPC2->control_BPC2 == 0) {
    /* lux index based */
    aec_reference = stats_update->aec_update.lux_idx;
  } else {
    /* Gain based */
    aec_reference = stats_update->aec_update.sensor_gain;
  }

  ISP_DBG("Lux index %f Gain %f", stats_update->aec_update.lux_idx,
    stats_update->aec_update.sensor_gain);

  new_sensor_gain = stats_update->aec_update.sensor_gain;
  ISP_HIGH("aec_reference :%f", aec_reference);

  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
    start = 0;
    end = 0;
    trigger_point_type *bpc2_trigger =
      &(chromatix_BPC2->BPC2_data[i].BPC2_trigger);
    trigger_index = i;

    if (i == MAX_LIGHT_TYPES_FOR_SPATIAL - 1) {
      /* falls within region 6 but we do not use trigger points in the region */
      ratio = 0;
      break;
    }

    if (chromatix_BPC2->control_BPC2 == 0) {
      start = bpc2_trigger->lux_index_start;
      end   = bpc2_trigger->lux_index_end;
    } else {
      start = bpc2_trigger->gain_start;
      end   = bpc2_trigger->gain_end;
      ISP_HIGH("gain_start :%f", start);
      ISP_HIGH("gain_end :%f", end);
    }

    /* index is within interpolation range, find ratio */
     if (aec_reference >= start && aec_reference < end) {
       ratio = (aec_reference - start)/(end - start);
       ISP_DBG("%s [%f - %f - %f] = %f", __func__, start, aec_reference, end,
         ratio);
       break;
     }

     /* already scanned past the lux index */
     if (aec_reference < end) {
           break;
     }
  }

  bpc2->aec_reference = aec_reference;

  ISP_DBG("[%f - %f - %f] = %f index %d", start, aec_reference, end,
    ratio, i);

  if (trigger_index >= MAX_LIGHT_TYPES_FOR_SPATIAL) {
    ISP_ERR("no trigger match for BPC2 trigger value: %f\n", aec_reference);
    goto ERROR;
  }
  if ((trigger_index != bpc2->trigger_index) ||
     (!F_EQUAL(ratio, bpc2->aec_ratio_bpc.ratio)) ||
     (!F_EQUAL(new_sensor_gain, bpc2->curr_sensor_gain))) {
    bpc2->trigger_index = trigger_index;
    bpc2->aec_ratio_bpc.ratio = ratio;
    bpc2->curr_sensor_gain = new_sensor_gain;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}
/** bpc47_set_chromatix_ptr:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc47_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data)
{
  boolean             ret = TRUE;
  bpc47_t            *bpc = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  bpc = (bpc47_t *)isp_sub_module->private_data;
  if (!bpc) {
    ISP_ERR("failed: mod %p", bpc);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  isp_sub_module->chromatix_ptrs = *(modulesChromatix_t *)data;
  isp_sub_module->trigger_update_pending = TRUE;
  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }
  bpc47_init_config(isp_sub_module, bpc);

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* bpc47_set_chromatix_ptr */

/** bpc47_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc47_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  bpc47_t *bpc = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* bpc47_streamoff */

/** bpc47_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the bpc module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean bpc47_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  bpc47_t *bpc = NULL;

  if (!isp_sub_module || !module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  bpc = (bpc47_t *)CAM_MALLOC(sizeof(bpc47_t));
  if (!bpc) {
    ISP_ERR("failed: bpc %p", bpc);
    return FALSE;
  }

  memset(bpc, 0, sizeof(*bpc));

  isp_sub_module->private_data = (void *)bpc;
  /* Disable abcc by default*/
  bpc->abcc_subblock.abcc_enable = ABCC_ENABLE;
  FILL_FUNC_TABLE(bpc);
  isp_sub_module->manual_ctrls.hot_pix_mode = CAM_HOTPIXEL_MODE_FAST;

  return TRUE;
}/* bpc47_init */

/** bpc47_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void bpc47_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module || !module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* bpc47_destroy */

#if !OVERRIDE_FUNC
static ext_override_func bpc_override_func = {
  .check_enable_idx            = NULL,
};

boolean bpc47_fill_func_table(bpc47_t *bpc2)
{
  bpc2->ext_func_table = &bpc_override_func;
  return TRUE;
} /* bpc47_fill_func_table */
#endif
