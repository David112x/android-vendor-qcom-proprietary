/* bpc44.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_BPC, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_BPC, fmt, ##args)

/* isp headers */
#include "isp_defs.h"
#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "isp_sub_module_common.h"
#include "abcc_algo.h"
#include "bpc44.h"
#include "isp_pipeline_reg.h"


/*forward declartions */
static void bpc44_interpolation_width_update( bpc_4_offset_type *input_offset,
  uint32_t chromatix_bitwidth);

/** bpc44_cmd_debug:
 *
 *  @cmd: bpc config cmd
 *
 *  This function dumps the bpc module configuration set to hw
 *
 *  Returns nothing
 **/
static void bpc44_cmd_debug(ISP_DemosaicDBPC_CmdType *cmd)
{
  if (!cmd) {
    ISP_ERR("failed: cmd %p", cmd);
    return;
  }
  ISP_DBG( "cmd->fminThreshold = %d", cmd->fminThreshold);
  ISP_DBG( "cmd->fmaxThreshold = %d", cmd->fmaxThreshold);
  ISP_DBG( "cmd->rOffsetHi = %d", cmd->rOffsetHi);
  ISP_DBG( "cmd->rOffsetLo = %d", cmd->rOffsetLo);
  ISP_DBG( "cmd->bOffsetHi = %d", cmd->bOffsetHi);
  ISP_DBG( "cmd->bOffsetLo = %d", cmd->bOffsetLo);
  ISP_DBG( "cmd->grOffsetHi = %d", cmd->grOffsetHi);
  ISP_DBG( "cmd->grOffsetLo = %d", cmd->grOffsetLo);
  ISP_DBG( "cmd->gbOffsetHi = %d", cmd->gbOffsetHi);
  ISP_DBG( "cmd->gbOffsetLo = %d", cmd->gbOffsetLo);
} /* bpc44_cmd_debug */

/** bpc44_set_stream_config:
 *
 *  @module: module
 *  @isp_sub_module: isp sub module
 *  @event: mct event
 *
 *  copy sensor out dimention info
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc44_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  bpc44_t                 *bpc = NULL;
  sensor_out_info_t       *sensor_out_info = NULL;

  if (!isp_sub_module || !event || !module) {
    ISP_ERR("failed: %p %p %p", isp_sub_module, event, module);
    return FALSE;
  }

  sensor_out_info =
    (sensor_out_info_t *)event->u.module_event.module_event_data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bpc = (bpc44_t *)isp_sub_module->private_data;
  if (!bpc) {
    ISP_ERR("failed: bpc %p", bpc);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  bpc->abcc_subblock.abcc_info.sensor_width =
    sensor_out_info->dim_output.width;
  bpc->abcc_subblock.abcc_info.sensor_height =
    sensor_out_info->dim_output.height;
  bpc->abcc_subblock.lut_bank_select = 0;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* bpc44_set_stream_config */

/** bpc44_remove_duplicate_defective_pixels:
 *
 *  @defective_pixels: defective pixel array from sensor
 *
 *  remove duplicate pixels if any in the input defective pix
 *  array
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc44_remove_duplicate_defective_pixels(
  defective_pix_array_t *defective_pixels)
{
  defective_pix_array_t out_defective_pixels;
  pix_t *pix = defective_pixels->pix;
  int count = defective_pixels->count;
  int i = 0, j, k = 0;

  if (!defective_pixels) {
    ISP_ERR("failed: defective_pixels %p", defective_pixels);
    return FALSE;
  }
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

  return TRUE;
} /* end bpc44_remove_duplicate_defective_pixels */


/** bpc44_abcc_prepare_lut_tables:
 *
 *  @abcc: abcc subblock data
 *
 *  prepare LUT tables for HW config
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc44_abcc_prepare_lut_tables(abcc_t *abcc)
{
  abcc_packed_lut_t *packed_lut = &abcc->packed_lut;
  int i = 0;
  abcc_entry_t *entry;
  int pix_index;
  int width = abcc->input_width;
  int count = abcc->abcc_info.lut.final_lut_list.actual_count;

  if (!abcc) {
    ISP_ERR("failed: abcc %p", abcc);
    return FALSE;
  }
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
  return TRUE;
}

/** bpc44_run_abcc_algo:
 *
 *  @abcc: abcc sub block info
 *  Runs the abcc algo and prepares the LUT tab
 *
 **/
static boolean bpc44_run_abcc_algo(abcc_t *abcc)
{
  int ret = TRUE;
  if (!abcc) {
    ISP_ERR("failed: abcc %p", abcc);
    return FALSE;
  }

  if (abcc->abcc_info.defect_pixels.count == 0) {
    ISP_ERR("No defective pixels set by sensor event");
    return FALSE;
  }
  abcc->abcc_info.forced_correction = ABCC_FORCED_CORRECTION_ENABLE;

  ret = bpc44_remove_duplicate_defective_pixels(
    &abcc->abcc_info.defect_pixels);
  if (ret == FALSE) {
    ISP_ERR("failed bpc44_remove_duplicate_defective_pixels");
    return FALSE;
  }
  ISP_HIGH("%s: algo start", __func__);
  /* Based on the number of defect pixels received from sensor allocate memory
     for the algo params*/
  abcc_algo_init(&abcc->abcc_info);

  ret = abcc_algo_process(&abcc->abcc_info, ABCC_LUT_COUNT);
  if (ret == FALSE) {
    ISP_ERR("abcc_algo_process failed");
    return FALSE;
  }

  ret = bpc44_abcc_prepare_lut_tables(abcc);
  if (ret == FALSE) {
    ISP_ERR("failed: bpc44_abcc_prepare_lut_tables");
    return FALSE;
  }
  /* Algo params are no more needed as this is run only once per session so
     so free the memory*/
  abcc_algo_deinit(&abcc->abcc_info);

  ISP_HIGH("%s: algo end", __func__);

  abcc->hw_update_pending = 1;

  return ret;
}/* bpc44_run_abcc_algo */

/** bpc44_ez_isp_update
 *
 *  @mod: bpc module handle
 *  @bpcDiag: bpc Diag handle
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void bpc44_ez_isp_update(bpc44_t *mod,
  badcorrection_t *bpcDiag)
{
  ISP_DemosaicDBPC_CmdType *bpcCfg = &(mod->RegCmd);

  bpcDiag->fminThreshold = bpcCfg->fminThreshold;
  bpcDiag->fmaxThreshold = bpcCfg->fmaxThreshold;
  bpcDiag->gbOffsetLo = bpcCfg->gbOffsetLo;
  bpcDiag->gbOffsetHi = bpcCfg->gbOffsetHi;
  bpcDiag->grOffsetLo = bpcCfg->grOffsetLo;
  bpcDiag->grOffsetHi = bpcCfg->grOffsetHi;
  bpcDiag->rOffsetLo = bpcCfg->rOffsetLo;
  bpcDiag->rOffsetHi = bpcCfg->rOffsetHi;
  bpcDiag->bOffsetLo = bpcCfg->bOffsetLo;
  bpcDiag->bOffsetHi = bpcCfg->bOffsetHi;
}/* bpc44_ez_isp_update */

/** bpc44_fill_vfe_diag_data:
 *
 *  @bpc: bpc module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean bpc44_fill_vfe_diag_data(bpc44_t *mod,
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

    bpc44_ez_isp_update(mod, bpcDiag);
  }
  return ret;
}

/** bpc44_cmd_config:
 *
 *  @mod: bpc module struct data
 *
 *  Copy from mod->chromatix params to reg cmd then configure
 *
 *  Returns nothing
 **/
static void bpc44_cmd_config(isp_sub_module_t *isp_sub_module, bpc44_t *bpc)
{
  if (!isp_sub_module || !bpc) {
    ISP_ERR("failed: %p %p", isp_sub_module, bpc);
    return;
  }
  if (!bpc->p_params.p_input_offset) {
    ISP_ERR("failed: mod->p_params.p_input_offset %p",
      bpc->p_params.p_input_offset);
    return;
  }
  bpc->RegCmd.fminThreshold = *bpc->p_params.p_Fmin;
  bpc->RegCmd.fmaxThreshold = *bpc->p_params.p_Fmax;
  bpc->RegCmd.rOffsetHi = bpc->p_params.p_input_offset->bpc_4_offset_r_hi;
  bpc->RegCmd.rOffsetLo = bpc->p_params.p_input_offset->bpc_4_offset_r_lo;
  bpc->RegCmd.bOffsetHi = bpc->p_params.p_input_offset->bpc_4_offset_b_hi;
  bpc->RegCmd.bOffsetLo = bpc->p_params.p_input_offset->bpc_4_offset_b_lo;
  bpc->RegCmd.grOffsetLo = bpc->p_params.p_input_offset->bpc_4_offset_gr_lo;
  bpc->RegCmd.grOffsetHi = bpc->p_params.p_input_offset->bpc_4_offset_gr_hi;
  bpc->RegCmd.gbOffsetLo = bpc->p_params.p_input_offset->bpc_4_offset_gb_lo;
  bpc->RegCmd.gbOffsetHi = bpc->p_params.p_input_offset->bpc_4_offset_gb_hi;
} /* bpc44_cmd_config */

/** bpc44_write_lut_to_dmi:
 *
 *  @isp_sub_module: isp base module
 *  @mod: module data
 *
 *  writes abcc LUT table to DMI
 *
 **/
boolean bpc44_write_lut_to_dmi(isp_sub_module_t *isp_sub_module, abcc_t *mod)
{
  boolean   ret = TRUE;
  uint32_t  lut_size = sizeof(uint64_t) * ABCC_LUT_COUNT;
  uint32_t  lut_bank;
  lut_bank = (mod->lut_bank_select & 1) ? BPC_LUT_RAM_BANK0 : BPC_LUT_RAM_BANK1;
  ret = isp_sub_module_util_write_dmi(
    (void*)&mod->packed_lut, lut_size,
    lut_bank,
    VFE_WRITE_DMI_64BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: bpc44 isp_sub_module_util_write_dmi");
  }
  mod->lut_bank_select ^= 1;
  return ret;
}



/** bpc44_do_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @bpc: bpc struct data
 *
 * update BPC module register to kernel
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean bpc44_store_hw_update(isp_sub_module_t *isp_sub_module,
  bpc44_t *bpc)
{
  boolean                      ret = TRUE;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  ISP_DemosaicDBPC_CmdType    *RegCmd = NULL;

  if (!isp_sub_module || !bpc) {
    ISP_ERR("failed: %p %p", isp_sub_module, bpc);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd));

  RegCmd = (ISP_DemosaicDBPC_CmdType *)CAM_MALLOC(sizeof(*RegCmd));
  if (!RegCmd) {
    ISP_ERR("failed: RegCmd %p", RegCmd);
    goto ERROR_BPC_CMD;
  }
  memset(RegCmd, 0, sizeof(*RegCmd));
  *RegCmd = bpc->RegCmd;

  if (bpc->abcc_subblock.abcc_enable &&
      bpc->abcc_subblock.hw_update_pending == TRUE &&
      bpc->abcc_subblock.one_time_config_done == FALSE) {
    bpc44_write_lut_to_dmi(isp_sub_module, &bpc->abcc_subblock);
    bpc->abcc_subblock.hw_update_pending = FALSE;
    bpc->abcc_subblock.one_time_config_done = TRUE;
  }

  cfg_cmd->cfg_data = (void *)RegCmd;
  cfg_cmd->cmd_len = sizeof(*RegCmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd->u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd->cmd_type = VFE_WRITE;
  reg_cfg_cmd->u.rw_info.reg_offset = ISP_DBPC_CFG_OFF;
  reg_cfg_cmd->u.rw_info.len = ISP_DBPC_LEN * sizeof(uint32_t);

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
} /* bpc44_do_hw_update */

/** bpc44_config
 *
 *  @isp_sub_module: isp sub module handle
 *  @bpc: bpc handle
 *
 *  BPC module configuration initial settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean bpc44_config(isp_sub_module_t *isp_sub_module, bpc44_t *bpc)
{
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;

  if (!isp_sub_module, !bpc) {
    ISP_ERR("failed: %p %p", isp_sub_module, bpc);
    return FALSE;
  }

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_BPC =
    &chromatix_ptr->chromatix_VFE.chromatix_BPC;

  if (bpc->ext_func_table && bpc->ext_func_table->get_init_min_max) {
    bpc->ext_func_table->get_init_min_max(bpc, isp_sub_module, NULL);
  } else {
    ISP_ERR("failed: %p", bpc->ext_func_table);
    return FALSE;
  }

  return TRUE;
} /* bpc44_config */

/** bpc44_trigger_update
 *
 *  @isp_sub_module: isp sub module handle
 *  @bpc: bpc handle
 *
 *  BPC module modify reg settings as per new input params and
 *  trigger hw update
 *
 * Return TRUE on success and FALSE on failure
 **/
static boolean bpc44_interpolate(isp_sub_module_t *isp_sub_module, bpc44_t *bpc)
{
  boolean                     ret = TRUE;
  chromatix_parms_type       *chromatix_ptr = NULL;
  chromatix_videoHDR_type    *chromatix_VHDR = NULL;
  chromatix_BPC_type         *chromatix_BPC = NULL;
  float                       aec_ratio = 0;
  uint8_t                     Fmin = 0,
                              Fmin_lowlight = 0,
                              Fmax = 0,
                              Fmax_lowlight = 0;
  bpc_4_offset_type          *bpc_normal_input_offset = NULL;
  bpc_4_offset_type          *bpc_lowlight_input_offset = NULL;
  uint32_t                    chromatix_bitwidth =  CHROMATIX_BITWIDTH;

  if (!isp_sub_module || !bpc) {
    ISP_ERR("failed: %p %p", isp_sub_module, bpc);
    return FALSE;
  }

  if (isp_sub_module->trigger_update_pending == FALSE) {
    return TRUE;
  }

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_BPC =
    &chromatix_ptr->chromatix_VFE.chromatix_BPC;
  chromatix_VHDR =
    &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;

  /*when HDR mode, chromatix tuned by 14 bit,
    otherwise keep 12 bit header for backward compatible*/
  if (chromatix_VHDR->videoHDR_reserve_data.hdr_recon_en == 0 &&
      chromatix_VHDR->videoHDR_reserve_data.hdr_mac_en == 0) {
    chromatix_bitwidth = CHROMATIX_BITWIDTH;
  } else {
    chromatix_bitwidth = ISP_PIPELINE_WIDTH;
  }
  ISP_DBG("Calculate table with AEC");

  if (bpc->ext_func_table && bpc->ext_func_table->get_min_max) {
    bpc->ext_func_table->get_min_max(bpc, isp_sub_module, NULL);
  } else {
    ISP_ERR("failed: %p", bpc->ext_func_table);
    return FALSE;
  }

  Fmin = bpc->Fmin;
  Fmax = bpc->Fmax;
  Fmin_lowlight = bpc->Fmin_lowlight;
  Fmax_lowlight = bpc->Fmax_lowlight;
  bpc_normal_input_offset = &bpc->bpc_normal_input_offset;
  bpc_lowlight_input_offset = &bpc->bpc_lowlight_input_offset;

  bpc44_interpolation_width_update(bpc_normal_input_offset, chromatix_bitwidth);
  bpc44_interpolation_width_update(bpc_lowlight_input_offset, chromatix_bitwidth);

  aec_ratio = bpc->aec_ratio;
  if (F_EQUAL(aec_ratio, 0.0)) {
    ISP_DBG("Low Light");
    bpc->p_params.p_input_offset = bpc_lowlight_input_offset;
    bpc->p_params.p_Fmin = &(Fmin_lowlight);
    bpc->p_params.p_Fmax = &(Fmax_lowlight);
    bpc44_cmd_config(isp_sub_module, bpc);
  } else if (F_EQUAL(aec_ratio, 1.0)) {
    ISP_DBG("Normal Light");
    bpc->p_params.p_input_offset = bpc_normal_input_offset;
    bpc->p_params.p_Fmin = &(Fmin);
    bpc->p_params.p_Fmax = &(Fmax);
    bpc44_cmd_config(isp_sub_module, bpc);
  } else {
    ISP_DBG("Interpolate between Nomal and Low Light");
    /* Directly configure reg cmd.*/
    Fmin = (uint8_t)LINEAR_INTERPOLATION_BET(Fmin, Fmin_lowlight, aec_ratio);
    bpc->RegCmd.fminThreshold = Fmin;

    Fmax = (uint8_t)LINEAR_INTERPOLATION_BET(Fmax, Fmax_lowlight, aec_ratio);
    bpc->RegCmd.fmaxThreshold = Fmax;

    bpc->RegCmd.rOffsetHi = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset->bpc_4_offset_r_hi,
      bpc_lowlight_input_offset->bpc_4_offset_r_hi, aec_ratio);
    bpc->RegCmd.rOffsetLo = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset->bpc_4_offset_r_lo,
      bpc_lowlight_input_offset->bpc_4_offset_r_lo, aec_ratio);

    bpc->RegCmd.bOffsetHi = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset->bpc_4_offset_b_hi,
      bpc_lowlight_input_offset->bpc_4_offset_b_hi, aec_ratio);
    bpc->RegCmd.bOffsetLo = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset->bpc_4_offset_b_lo,
      bpc_lowlight_input_offset->bpc_4_offset_b_lo, aec_ratio);

    bpc->RegCmd.grOffsetHi = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset->bpc_4_offset_gr_hi,
      bpc_lowlight_input_offset->bpc_4_offset_gr_hi, aec_ratio);
    bpc->RegCmd.grOffsetLo = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset->bpc_4_offset_gr_lo,
      bpc_lowlight_input_offset->bpc_4_offset_gr_lo, aec_ratio);

    bpc->RegCmd.gbOffsetHi = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset->bpc_4_offset_gb_hi,
      bpc_lowlight_input_offset->bpc_4_offset_gb_hi, aec_ratio);
    bpc->RegCmd.gbOffsetLo = (uint16_t)LINEAR_INTERPOLATION_BET(
      bpc_normal_input_offset->bpc_4_offset_gb_lo,
      bpc_lowlight_input_offset->bpc_4_offset_gb_lo, aec_ratio);
  }
  bpc->aec_ratio = aec_ratio;

  bpc44_cmd_debug(&(bpc->RegCmd));

  ret = bpc44_store_hw_update(isp_sub_module, bpc);
  if (ret == FALSE) {
    ISP_ERR("failed: bpc44_store_hw_update");
    return FALSE;
  }

  isp_sub_module->trigger_update_pending = FALSE;

  return TRUE;
} /* bpc44_trigger_update */

/** bpc44_interpolate_width_update
 *
 * input_offset - data from chromatix
 * 8994 - Pipeline bit width is 14
 * 8084 - pipeline bitwidth is 12
 * performs corresponding update on input data
 **/
void bpc44_interpolation_width_update( bpc_4_offset_type *input_offset,
  uint32_t chromatix_bitwidth)
{
  input_offset->bpc_4_offset_r_hi  <<= (ISP_PIPELINE_WIDTH - chromatix_bitwidth);
  input_offset->bpc_4_offset_r_lo  <<= (ISP_PIPELINE_WIDTH - chromatix_bitwidth);
  input_offset->bpc_4_offset_b_hi  <<= (ISP_PIPELINE_WIDTH - chromatix_bitwidth);
  input_offset->bpc_4_offset_b_lo  <<= (ISP_PIPELINE_WIDTH - chromatix_bitwidth);
  input_offset->bpc_4_offset_gb_hi <<= (ISP_PIPELINE_WIDTH - chromatix_bitwidth);
  input_offset->bpc_4_offset_gb_lo <<= (ISP_PIPELINE_WIDTH - chromatix_bitwidth);
  input_offset->bpc_4_offset_gr_hi <<= (ISP_PIPELINE_WIDTH - chromatix_bitwidth);
  input_offset->bpc_4_offset_gr_lo <<= (ISP_PIPELINE_WIDTH - chromatix_bitwidth);
}

/** bpc44_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Perform trigger update if trigger_update_pending flag is
 *  TRUE and append hw update list in global list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean bpc44_trigger_update(isp_sub_module_t *isp_sub_module, void *data)
{
  boolean                       ret = TRUE;
  bpc44_t                      *bpc = NULL;
  isp_private_event_t          *private_event = NULL;
  isp_sub_module_output_t      *output = NULL;
  uint8_t                      module_enable;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bpc = (bpc44_t *)isp_sub_module->private_data;
  if (!bpc) {
    ISP_ERR("failed: bpc %p", bpc);
    goto ERROR;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    goto ERROR;
  }

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    goto ERROR;
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

  if (!isp_sub_module->submod_enable ||
      !isp_sub_module->submod_trigger_enable) {
    ISP_DBG( "skip trigger update enable %d, trig_enable %d",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);

    goto FILL_METADATA;
  }

  if (bpc->ext_func_table && bpc->ext_func_table->check_enable_idx)
    bpc->ext_func_table->check_enable_idx(bpc, isp_sub_module, output);

  ret = bpc44_interpolate(isp_sub_module, bpc);
  if (ret == FALSE) {
    ISP_ERR("failed: bpc44_interpolate");
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto FILL_METADATA;
  }

FILL_METADATA:
  if (output && isp_sub_module->vfe_diag_enable) {
    ret = bpc44_fill_vfe_diag_data(bpc, isp_sub_module, output);
    if (ret == FALSE) {
      ISP_ERR("failed: bpc44_fill_vfe_diag_data");
    }
  }

  if (output->frame_meta)
    output->frame_meta->hot_pix_mode =
      isp_sub_module->manual_ctrls.hot_pix_mode;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** bpc44_stats_aec_update:
 *
 * @mod: demosaic module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean bpc44_stats_aec_update(isp_sub_module_t *isp_sub_module, void *data)
{
  stats_update_t       *stats_update = NULL;
  bpc44_t              *bpc = NULL;
  float                 aec_ratio = 0;
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  boolean               ret = TRUE;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  bpc = (bpc44_t *)isp_sub_module->private_data;
  if (!bpc) {
    ISP_ERR("failed: bpc %p", bpc);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  bpc->aec_update = stats_update->aec_update;
  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: stats_update %p", stats_update);
    goto ERROR;
  }

  chromatix_BPC = &chromatix_ptr->chromatix_VFE.chromatix_BPC;

  if (bpc->ext_func_table && bpc->ext_func_table->get_trigger_ratio) {
    ret = bpc->ext_func_table->get_trigger_ratio(bpc, isp_sub_module, NULL);
  }  else {
    ISP_ERR("failed: %p", bpc->ext_func_table);
    ret =  FALSE;
  }
  if (FALSE == ret) {
    ISP_ERR("failed");
    goto ERROR;
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** bpc44_set_chromatix_ptr:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc44_set_chromatix_ptr(isp_sub_module_t *isp_sub_module, void *data)
{
  boolean             ret = TRUE;
  bpc44_t            *bpc = NULL;
  modulesChromatix_t *chromatix_ptrs = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  bpc = (bpc44_t *)isp_sub_module->private_data;
  if (!bpc) {
    ISP_ERR("failed: mod %p", bpc);
    return FALSE;
  }

  chromatix_ptrs = (modulesChromatix_t *)data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;

  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }

  ret = bpc44_config(isp_sub_module, bpc);
  if (ret == FALSE) {
    ISP_ERR("failed: bpc44_config ret %d", ret);
    goto ERROR;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* bpc44_set_chromatix_ptr */

/** bpc44_set_defective_pixel_for_abcc:
 *
 * @mod: bpc module
 * @isp_sub_module: isp_sub_module
 * @event: mct event
 *
 * Handle Set Defective pixel event from sensor
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean bpc44_set_defective_pixel_for_abcc(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  bpc44_t              *bpc = NULL;

  if (!module || !event || !isp_sub_module) {
    ISP_ERR("failed: %p %p %p", module, event, isp_sub_module);
    return FALSE;
  }

  bpc = (bpc44_t *)isp_sub_module->private_data;
  if (!bpc) {
    ISP_ERR("failed: bpc %p", bpc);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  bpc->abcc_subblock.abcc_info.defect_pixels =
    *(defective_pix_array_t *)event->u.module_event.module_event_data;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}


/** bpc44_streamon:
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
boolean bpc44_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;
  bpc44_t               *bpc;
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
  bpc = (bpc44_t *)isp_sub_module->private_data;
  if (!bpc) {
    ISP_ERR("failed: bpc %p", bpc);
    return FALSE;
  }
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  abcc = &bpc->abcc_subblock;
  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }
  /* Abcc Algo and config is only done once per session.
     Run the algo if it is 1st streamon */
  if (isp_sub_module->stream_on_count == 1) {
    abcc->one_time_config_done = FALSE;
    ret = bpc44_run_abcc_algo(abcc);
  }

  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->config_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

} /* bpc44_streamon */


/** bpc44_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean bpc44_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  bpc44_t *bpc = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  bpc = (bpc44_t *)isp_sub_module->private_data;
  if (!bpc) {
    ISP_ERR("failed: bpc %p", bpc);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(bpc, 0, sizeof(*bpc));
  FILL_FUNC_TABLE(bpc); /* Keep func table */
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
} /* bpc44_streamoff */

/** bpc44_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the bpc module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean bpc44_init(isp_sub_module_t *isp_sub_module)
{
  bpc44_t *bpc = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  bpc = (bpc44_t *)CAM_MALLOC(sizeof(bpc44_t));
  if (!bpc) {
    ISP_ERR("failed: bpc %p", bpc);
    return FALSE;
  }

  memset(bpc, 0, sizeof(*bpc));
  isp_sub_module->private_data = (void *)bpc;
  FILL_FUNC_TABLE(bpc);
  isp_sub_module->manual_ctrls.hot_pix_mode = CAM_HOTPIXEL_MODE_FAST;

  bpc->abcc_subblock.abcc_enable = ABCC_ENABLE;
  bpc->abcc_subblock.lut_bank_select = 0;
  return TRUE;
}/* bpc44_init */

/** bpc44_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void bpc44_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* bpc44_destroy */

#if !OVERRIDE_FUNC
/** bpc44_get_min_max:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bpc44_get_min_max(void *data1,
  void *data2, void *data3)
{
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  uint8_t normal_light_idx = (MAX_LIGHT_TYPES_FOR_SPATIAL/2);
  bpc44_t              *bpc = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  bpc = (bpc44_t *)data1;
  isp_sub_module = (isp_sub_module_t *)data2;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_BPC = &chromatix_ptr->chromatix_VFE.chromatix_BPC;
  if (!chromatix_BPC) {
    ISP_ERR("failed: chromatix_BPC %p", chromatix_BPC);
    return FALSE;
  }
  if ((chromatix_BPC->bcc_Fmin > chromatix_BPC->bpc_Fmax) ||
      (chromatix_BPC->bcc_Fmin_lowlight > chromatix_BPC->bpc_Fmax_lowlight)) {
    ISP_ERR("Error min>max: %d/%d; %d/%d\n",
      chromatix_BPC->bcc_Fmin, chromatix_BPC->bpc_Fmax,
      chromatix_BPC->bcc_Fmin_lowlight, chromatix_BPC->bpc_Fmax_lowlight);
    return FALSE;
  }

  bpc->Fmin = chromatix_BPC->bpc_Fmin;
  bpc->Fmax = chromatix_BPC->bpc_Fmax;
  bpc->Fmin_lowlight = chromatix_BPC->bpc_Fmin_lowlight;
  bpc->Fmax_lowlight = chromatix_BPC->bpc_Fmax_lowlight;
  bpc->bpc_normal_input_offset =
    chromatix_BPC->bpc_4_offset[BPC_NORMAL_LIGHT];
  bpc->bpc_lowlight_input_offset =
    chromatix_BPC->bpc_4_offset[BPC_LOW_LIGHT];
  return TRUE;
}

/** bpc4_get_init_min_max:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bpc44_get_init_min_max(void *data1,
  void *data2, void *data3)
{
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  uint8_t normal_light_idx = (MAX_LIGHT_TYPES_FOR_SPATIAL/2);
  bpc44_t              *bpc = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  bpc = (bpc44_t *)data1;
  isp_sub_module = (isp_sub_module_t *)data2;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_BPC = &chromatix_ptr->chromatix_VFE.chromatix_BPC;
  if (!chromatix_BPC) {
    ISP_ERR("failed: chromatix_BPC %p", chromatix_BPC);
    return FALSE;
  }
  if ((chromatix_BPC->bpc_Fmin > chromatix_BPC->bpc_Fmax) ||
      (chromatix_BPC->bpc_Fmin_lowlight > chromatix_BPC->bpc_Fmax_lowlight)) {
    ISP_ERR("Error min>max: %d/%d; %d/%d",
      chromatix_BPC->bpc_Fmin, chromatix_BPC->bpc_Fmax,
      chromatix_BPC->bpc_Fmin_lowlight, chromatix_BPC->bpc_Fmax_lowlight);
    return FALSE;
  }

  bpc->p_params.p_input_offset =
    &(chromatix_BPC->bpc_4_offset[BPC_NORMAL_LIGHT]);
  bpc->p_params.p_Fmin = &(chromatix_BPC->bpc_Fmin);
  bpc->p_params.p_Fmax = &(chromatix_BPC->bpc_Fmax);
  return TRUE;
}

/** bpc44_get_trigger_ratio:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean bpc44_get_trigger_ratio(void *data1,
  void *data2, void *data3)
{
  trigger_ratio_t trigger_ratio;
  chromatix_parms_type *chromatix_ptr = NULL;
  chromatix_BPC_type   *chromatix_BPC = NULL;
  float                 aec_ratio = 0.0;
  bpc44_t              *bpc = NULL;
  isp_sub_module_t     *isp_sub_module = NULL;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  bpc = (bpc44_t *)data1;
  isp_sub_module = (isp_sub_module_t *)data2;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_BPC = &chromatix_ptr->chromatix_VFE.chromatix_BPC;
  if (!chromatix_BPC) {
    ISP_ERR("failed: chromatix_BPC %p", chromatix_BPC);
    return FALSE;
  }
  aec_ratio = isp_sub_module_util_get_aec_ratio_lowlight(
    chromatix_BPC->control_bpc, &chromatix_BPC->bpc_lowlight_trigger,
    &bpc->aec_update, 1);

  if ((isp_sub_module->trigger_update_pending == FALSE) &&
       !F_EQUAL(bpc->aec_ratio, aec_ratio)) {
    isp_sub_module->trigger_update_pending = TRUE;
  }

  bpc->aec_ratio = aec_ratio;
  return TRUE;
}

static ext_override_func bpc_override_func = {
  .check_enable_idx  = NULL,
  .get_trigger_ratio = bpc44_get_trigger_ratio,
  .get_init_min_max  = bpc44_get_init_min_max,
  .get_min_max       = bpc44_get_min_max,
};

boolean bpc44_fill_func_table(bpc44_t *bpc)
{
  bpc->ext_func_table = &bpc_override_func;
  return TRUE;
} /* bpc44_fill_func_table */

#endif
