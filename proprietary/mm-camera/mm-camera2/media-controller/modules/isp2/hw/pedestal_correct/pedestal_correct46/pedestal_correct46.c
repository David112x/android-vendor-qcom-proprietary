/*============================================================================

  Copyright (c) 2016 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */


/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "pedestal_correct46.h"
#include "isp_pipeline_reg.h"

/** pedestal46_cmd_debug:
 *
 *    @mod: pedestal module
 *
 **/
static void pedestal46_cmd_debug(ISP_Pedestal_CfgCmdType *cmd)
{
  ISP_DBG(" scaleBypass     = %d", cmd->scaleBypass);
  ISP_DBG(" hdrEnable       = %d", cmd->hdrEnable);
  ISP_DBG(" lutBankSel      = %d", cmd->lutBankSel);
  ISP_DBG(" blockWidth      = %d", cmd->blockWidth);
  ISP_DBG(" blockHeight     = %d", cmd->blockHeight);
  ISP_DBG(" subGridHeight   = %d", cmd->subGridHeight);
  ISP_DBG(" subGridYDelta   = %d", cmd->subGridYDelta);
  ISP_DBG(" interpFactor    = %d", cmd->interpFactor);
  ISP_DBG(" subGridWidth    = %d", cmd->subGridWidth);
  ISP_DBG(" subGridXDelta   = %d", cmd->subGridXDelta);
  ISP_DBG(" blockWidth_r    = %d", cmd->blockWidth_r);
  ISP_DBG(" blockHeight_r   = %d", cmd->blockHeight_r);
  ISP_DBG(" subGridHeight_r = %d", cmd->subGridHeight_r);
  ISP_DBG(" subGridYDelta_r = %d", cmd->subGridYDelta_r);
  ISP_DBG(" interpFactor_r  = %d", cmd->interpFactor_r);
  ISP_DBG(" subGridWidth_r  = %d", cmd->subGridWidth_r);
  ISP_DBG(" subGridXDelta_r = %d", cmd->subGridXDelta_r);
  ISP_DBG(" blockXIndex     = %d", cmd->blockXIndex);
  ISP_DBG(" blockYIndex     = %d", cmd->blockYIndex);
  ISP_DBG(" yDeltaAccum     = %d", cmd->yDeltaAccum);
  ISP_DBG(" pixelXIndex     = %d", cmd->pixelXIndex);
  ISP_DBG(" pixelYIndex     = %d", cmd->pixelYIndex);
  ISP_DBG(" subGridXIndex   = %d", cmd->subGridXIndex);
  ISP_DBG(" subGridYIndex   = %d", cmd->subGridYIndex);
  ISP_DBG(" blockXIndex_r   = %d", cmd->blockXIndex_r);
  ISP_DBG(" blockYIndex_r   = %d", cmd->blockYIndex_r);
  ISP_DBG(" yDeltaAccum_r   = %d", cmd->yDeltaAccum_r);
  ISP_DBG(" pixelXIndex_r   = %d", cmd->pixelXIndex_r);
  ISP_DBG(" pixelYIndex_r   = %d", cmd->pixelYIndex_r);
  ISP_DBG(" subGridXIndex_r = %d", cmd->subGridXIndex_r);
  ISP_DBG(" subGridYIndex_r = %d", cmd->subGridYIndex_r);
}

/** pedestal46_reset:
 *
 *    @mod:
 *
 **/
static void pedestal46_reset(pedestal46_mod_t *mod)
{
  memset(&mod->pedestal_cfg, 0, sizeof(ISP_Pedestal_CfgCmdType));
  mod->aec_ratio = 0.0f;
  mod->is_hdr_mode = FALSE;
}

/** pedestal_ez_isp_update
 *
 *  @mod: gic module handle
 *  @gicDiag: gic Diag handle
 *
 *  eztune update
 *
 *  Return NONE
 **/
static void pedestal_ez_isp_update(pedestal46_mod_t *mod,
  pedestaldiag_t  *pedestalDiag)
{
  ISP_Pedestal_CfgCmdType *pedestalCfg = &(mod->pedestal_cfg);

  if (pedestalCfg != NULL) {
    pedestalDiag->HDREnable = pedestalCfg->hdrEnable;
    pedestalDiag->ScaleBypass = pedestalCfg->scaleBypass;
    pedestalDiag->IntpFactor = pedestalCfg->interpFactor;
    pedestalDiag->BWidth = pedestalCfg->blockWidth;
    pedestalDiag->BHeight = pedestalCfg->blockHeight;
    //check/confirm the following two mapping
    pedestalDiag->XDelta = pedestalCfg->subGridXDelta;
    pedestalDiag->YDelta = pedestalCfg->subGridYDelta;

    memcpy(&pedestalDiag->MeshTableT1_R[0],
            &mod->chromatix_tbls->pctable[0].channel_black_level_r[0],
             MESH_PedestalTable_SIZE);
    memcpy(&pedestalDiag->MeshTableT1_Gr[0],
            &mod->chromatix_tbls->pctable[0].channel_black_level_gr[0],
             MESH_PedestalTable_SIZE);
    memcpy(&pedestalDiag->MeshTableT1_Gb[0],
            &mod->chromatix_tbls->pctable[0].channel_black_level_gb[0],
             MESH_PedestalTable_SIZE);
    memcpy(&pedestalDiag->MeshTableT1_B[0],
            &mod->chromatix_tbls->pctable[0].channel_black_level_b[0],
             MESH_PedestalTable_SIZE);
    memcpy(&pedestalDiag->MeshTableT2_R[0],
            &mod->chromatix_tbls->pctable[1].channel_black_level_r[0],
             MESH_PedestalTable_SIZE);
    memcpy(&pedestalDiag->MeshTableT2_Gr[0],
            &mod->chromatix_tbls->pctable[1].channel_black_level_gr[0],
             MESH_PedestalTable_SIZE);
    memcpy(&pedestalDiag->MeshTableT2_Gb[0],
            &mod->chromatix_tbls->pctable[1].channel_black_level_gb[0],
             MESH_PedestalTable_SIZE);
    memcpy(&pedestalDiag->MeshTableT2_B[0],
            &mod->chromatix_tbls->pctable[1].channel_black_level_b[0],
             MESH_PedestalTable_SIZE);

    /* 3D camera is not supported yet*/
    pedestalDiag->LeftImageWD = 0;
    pedestalDiag->Enable3D = 0;
    pedestalDiag->MeshGridBWidth = pedestalCfg->subGridWidth;
    pedestalDiag->MeshGridBHeight = pedestalCfg->subGridHeight;
    pedestalDiag->LXStart = pedestalCfg->blockXIndex;
    pedestalDiag->LYStart = pedestalCfg->blockYIndex;
    pedestalDiag->BXStart = pedestalCfg->subGridXIndex;
    pedestalDiag->BYStart = pedestalCfg->subGridYIndex;
    pedestalDiag->BXD1 = pedestalCfg->pixelXIndex;
    pedestalDiag->BYE1 = pedestalCfg->pixelYIndex;
    pedestalDiag->BYInitE1 = pedestalCfg->yDeltaAccum;
  }

}/* pedestal_ez_isp_update */

/** pedestal_fill_vfe_diag_data:
 *
 *  @gic: gic module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean pedestal_fill_vfe_diag_data(pedestal46_mod_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean  ret = TRUE;
  pedestaldiag_t  *pedestalDiag = NULL;
  vfe_diagnostics_t  *vfe_diag = NULL;

  if (sub_module_output && sub_module_output->frame_meta) {
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    pedestalDiag = &(vfe_diag->prev_pedestaldiag);

    pedestal_ez_isp_update(mod, pedestalDiag);
  }

  return ret;
}/* pedestal_fill_vfe_diag_data */

/** pedestal46_do_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @pedestal_mod: pedestal module handle
 *  @hw_update_list: hw update list handle
 *
 *  Create hw update list and append it
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean pedestal46_do_hw_update(isp_sub_module_t *isp_sub_module,
  pedestal46_mod_t *pedestal_mod, isp_sub_module_output_t *output)
{
  boolean                        ret = TRUE;
  struct msm_vfe_cfg_cmd2       *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd    *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list   *hw_update = NULL;
  ISP_Pedestal_CfgCmdType       *pedestal_cfg = NULL;

  if (PEDESTAL_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      PEDESTAL_CGC_OVERRIDE_REGISTER, PEDESTAL_CGC_OVERRIDE_BIT, TRUE);
    if (ret == FALSE) {
      ISP_ERR("failed: enable cgc");
    }
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)
    CAM_MALLOC(sizeof(struct msm_vfe_cfg_cmd_list));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd =
    (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(struct msm_vfe_reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_FREE_HW_UPDATE;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd));

  pedestal_cfg = (ISP_Pedestal_CfgCmdType *)
    CAM_MALLOC(sizeof(ISP_Pedestal_CfgCmdType));
  if (!pedestal_cfg) {
    ISP_ERR("failed: pedestal_cfg %p", pedestal_cfg);
    goto ERROR_FREE_REG_CFG_CMD;
  }
  memset(pedestal_cfg, 0, sizeof(*pedestal_cfg));

  *pedestal_cfg = pedestal_mod->pedestal_cfg;
  cfg_cmd->cfg_data = (void *)pedestal_cfg;
  cfg_cmd->cmd_len = sizeof(*pedestal_cfg);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_PEDESTAL_OFF;
  reg_cfg_cmd[0].u.rw_info.len = ISP_PEDESTAL_LEN;

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  pedestal46_cmd_debug(&pedestal_mod->pedestal_cfg);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update %d", ret);
    goto ERROR_FREE_GIC_CFG_CMD;
  }

  ret = isp_sub_module_util_write_dmi(
    pedestal_mod->hw_tables[0].gr_r, PEDESTAL_LUT_SIZE * sizeof(uint32_t),
    PEDESTAL_F0_RAM_L_GR_R_BANK0 + pedestal_cfg->lutBankSel * 4,
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_write_dmi for F0_GR_R");
    goto ERROR_FREE_GIC_CFG_CMD;
  }

  ret = isp_sub_module_util_write_dmi(
    pedestal_mod->hw_tables[0].gb_b, PEDESTAL_LUT_SIZE * sizeof(uint32_t),
    PEDESTAL_F0_RAM_L_GB_B_BANK0 + pedestal_cfg->lutBankSel * 4,
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_write_dmi for F0_GB_B");
    goto ERROR_FREE_GIC_CFG_CMD;
  }
  if (PEDESTAL_T1_T2 != 1) {
    ret = isp_sub_module_util_write_dmi(
      pedestal_mod->hw_tables[1].gr_r, PEDESTAL_LUT_SIZE * sizeof(uint32_t),
      PEDESTAL_F1_RAM_L_GR_R_BANK0 + pedestal_cfg->lutBankSel * 4,
      VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_sub_module_util_write_dmi for F1_GR_R");
      goto ERROR_FREE_GIC_CFG_CMD;
    }

    ret = isp_sub_module_util_write_dmi(
      pedestal_mod->hw_tables[1].gb_b, PEDESTAL_LUT_SIZE * sizeof(uint32_t),
      PEDESTAL_F1_RAM_L_GB_B_BANK0 + pedestal_cfg->lutBankSel * 4,
      VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_sub_module_util_write_dmi for F1_GB_B");
      goto ERROR_FREE_GIC_CFG_CMD;
    }
  }
  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR_FREE_GIC_CFG_CMD;
  }

  pedestal_mod->pedestal_cfg.lutBankSel ^= 1;

  if (PEDESTAL_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      PEDESTAL_CGC_OVERRIDE_REGISTER, PEDESTAL_CGC_OVERRIDE_BIT, FALSE);
    if (ret == FALSE) {
      ISP_ERR("failed: disable cgc");
    }
  }

  isp_sub_module->trigger_update_pending = FALSE;

  return TRUE;

ERROR_FREE_GIC_CFG_CMD:
  free(pedestal_cfg);
ERROR_FREE_REG_CFG_CMD:
  free(reg_cfg_cmd);
ERROR_FREE_HW_UPDATE:
  free(hw_update);
  return FALSE;
}

/** pedestal46_init:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the pedestal module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean pedestal46_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  pedestal46_mod_t *mod = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);

  mod = (pedestal46_mod_t *)CAM_MALLOC(sizeof(pedestal46_mod_t));
  RETURN_IF_NULL(mod);

  memset(mod, 0, sizeof(*mod));
  pedestal46_reset(mod);

  isp_sub_module->private_data = (void *)mod;
  isp_sub_module->manual_ctrls.black_level_lock = CAM_BLACK_LEVEL_LOCK_OFF;
  return TRUE;
} /* pedestal46_init */

/** pedestal46_calc_grid_config:
 *
 * system algo for subgrid number
 *
 *  Return subgrid number
 **/
static boolean pedestal46_calc_grid_config(int raw_width, int raw_height,
  int *scale_cubic, int *deltah, int *deltav, int *subgridh, int *subgridv,
  int *nh, int *nv)
{
  int scale, level, w, h, sgh, sgv, gh, gv, dh, dv, nx, ny;
  int e, temp;

  nx = PEDESTAL_H_GRIDS;
  ny = PEDESTAL_V_GRIDS;

  w = raw_width >> 1;  /* per-channel image width */
  h = raw_height >> 1;  /* per-channel image height */

  level = 4; /* Initial bicubic level level as 1 more than maximum 3 */

  /* currently in pedestal num of block is not progamble,
     so PEDESTAL_H_GRIDS = PED_MIN_BICUBIC_H_GRID_NUM
     so once level lower to 0(which cant find valid num of subgrid),
     it will automatically go to the above error case.
     use same rolloff algo for better maintainence,
     also for future possible extension.*/
  do {
      if (level == 0 && nx <= PED_MIN_BICUBIC_H_GRID_NUM &&
        ny <= PED_MIN_BICUBIC_V_GRID_NUM) {
        ISP_ERR("Error: small image");
        return FALSE;
      }

      /* once the sub grid reach limit, try lower block number,
         if still fit the HW limitation. explain as above */
      if (level > 0) {
        level--;
      } else if ((nx > PED_MIN_BICUBIC_H_GRID_NUM) &&
          (ny > PED_MIN_BICUBIC_V_GRID_NUM)) {
        nx -= 4;
        ny -= 3;
        level = 3;
      }

      sgh = (w + nx - 1) / nx;  /* Ceil */
      sgh = (sgh + (1 << level) - 1) >> level;  /* Ceil */
      gh = sgh << level;     /* Bayer grid width */
      dh = gh * nx - w; /* two-side overhead */
      sgv = (h + ny - 1) / ny;  /* Ceil */
      sgv = (sgv + (1 << level) - 1) >> level;   /* Ceil */
      gv = sgv << level;     /* Bayer grid height */
      dv = gv * ny - h; /* two-side overhead */
    } while ((level > 0) && ((gh < 18) || (gv <9) || (sgh < 9) || (sgv <9) ||
             (dh >= gh) || (dv >= gv) || (gh - (dh + 1) / 2 < 18) ||
             (sgh - (((dh + 1) / 2) % sgh) < 9)));

  ISP_DBG(" %s level = %d, sgh = %d, sgv = %d\n", __func__, level ,sgh, sgv);
  ISP_DBG(" %s nx = %d, ny = %d,\n", __func__, nx, ny);

  *scale_cubic = 1 << level;
  *deltah = (dh + 1) >> 1;
  *deltav = (dv + 1) >> 1;
  *subgridh = sgh;
  *subgridv = sgv;
  *nh = nx;
  *nv = ny;
  return TRUE;
}


void pedestal46_bicubic (float fs, float* fc0, float* fc1, float* fc2,
  float* fc3)
{
  /* calculate interpolation */
  double fs2 = fs*fs;
  double fs3 = fs*fs2;
  *fc0 = 0.5*(-fs3+2.0*fs2-fs);
  *fc1 = 0.5*(3.0*fs3-5.0*fs2+2.0);
  *fc2 = 0.5*(-3.0*fs3+4.0*fs2+fs);
  *fc3 = 0.5*(fs3-fs2);
}

/** pedestal46_mesh_sensor_calc_table_debug:
 *
 *  @meshtbl: Input 13x10 table to be printed
 *
 *  Print the 13x10 Rolloff table
 *
 *  Return void
 **/
static void pedestal46_mesh_sensor_calc_table_debug(short unsigned int *meshtbl)
{
  int  i, j;
  char line_str[128], *p_str;

  if (!meshtbl) {
    ISP_ERR("failed: %p", meshtbl);
    return;
  }

  line_str[0] = '\0';
  p_str = line_str;

  ISP_DBG("Pedestal Tbl R, H_grid %d, V_grid %d",
    HW_PED_NUM_ROW, HW_PED_NUM_COL);
  for (i = 0; i < HW_PED_NUM_COL; i++) {
    for(j = 0; j < HW_PED_NUM_ROW; j++) {
      snprintf(p_str, 8, "%d ", meshtbl[(i*HW_PED_NUM_ROW)+j]);
      p_str += 5;
    }
    ISP_DBG("");
    line_str[0] = '\0';
    p_str = line_str;
  }

  return;
} /* pedestal46_mesh_sensor_calc_table_debug */


/**  pedestal46_extend_1block :

 * MeshIn: the input ideal Rolloff table (31x 41)at the full sensor
 * MeshOut: the output rolloff table at the current output resolution
 *
  **/
static void pedestal46_extend_1block(unsigned short *MeshIn,  float *MeshOut, int Nx,
  int Ny, float hdr_factor)
{
  ISP_DBG(" Entered %s \n", __func__);

  int i, j;
  for (i=1; i<Ny+1; i++)
    for (j=1; j<Nx+1; j++)
      MeshOut[i*(Nx+2)+j] = (float)MeshIn[(i-1)*Nx+j-1] * hdr_factor;

  MeshOut[0*(Nx+2)+0] = MeshOut[1*(Nx+2)+1]*2 - MeshOut[2*(Nx+2)+2];
  MeshOut[(Ny+1)*(Nx+2)+0] = MeshOut[(Ny)*(Nx+2)+1]*2 -
    MeshOut[(Ny-1)*(Nx+2)+2];
  MeshOut[(Ny+1)*(Nx+2)+Nx+1] = MeshOut[(Ny)*(Nx+2)+Nx]*2 -
    MeshOut[(Ny-1)*(Nx+2)+Nx-1];
  MeshOut[0*(Nx+2)+Nx+1] = MeshOut[1*(Nx+2)+Nx]*2 - MeshOut[2*(Nx+2)+Nx-1];


  for (i=1; i<Ny+1; i++)
  {
    MeshOut[i*(Nx+2)+0] = MeshOut[i*(Nx+2)+1]*2 - MeshOut[i*(Nx+2)+2];
    MeshOut[i*(Nx+2)+Nx+1] = MeshOut[i*(Nx+2)+Nx]*2 - MeshOut[i*(Nx+2)+Nx-1];
  }

  for (j=1; j<Nx+1; j++)
  {
    MeshOut[0*(Nx+2)+j] = MeshOut[1*(Nx+2)+j]*2 - MeshOut[2*(Nx+2)+j];
    MeshOut[(Ny+1)*(Nx+2)+j] = MeshOut[(Ny)*(Nx+2)+j]*2 -
      MeshOut[(Ny-1)*(Nx+2)+j];
  }
}

/** pedestal46_scale_table:
 *    @meshOut: output new meshtbl according to camif info
 *
 *  @MeshIn: the input Rolloff table (10x13) at the full sensor
 *  @MeshOut: the output rolloff table (10x13) at the current
 *           output resolution
 *  @full_width: the full-resolution width
 *  @full_height: the full-resolution height
 *  @output_width: the output width
 *  @output_height: the output height
 *  @offset_x: x-index of the top-left corner of output image on
 *            the full-resolution sensor
 *  @offset_y: y-index of the top-left corner of output image on
 *            the full-resolution sensor
 *  @scale_factor: the sensor scaling factor
 *  (= binning_factor digal_sampling_factor)
 *  @hdr_factor: multiplication factor for in-sensor hdr mode
 *
 *  This function is used to resample the Rolloff table from
 *  the full-resolution sensor to the (CAMIF) output size, which
 *  is decided by the sensor mode.
 *
 *  Return void
 **/
void pedestal46_scale_table(
    unsigned short *MeshIn,       // Input Rolloff table (10x13) at the full sensor
    unsigned short *MeshOut,      // Output rolloff table (10x13) at current o/p resolution
    int            full_width,    // Full-resolution width
    int            full_height,   // Full-resolution height
    int            output_width,  // Output width
    int            output_height, // Output height
    int            offset_x,      // x-index of top-left corner of o/p image
    int            offset_y,      // y-index of top-left corner of o/p image
    int            scale_factor,  // Sensor scaling factor(=binning_factor * digal_sampling_factor)
    float          hdr_factor)
{
  float cxm, cx0, cx1, cx2, cym, cy0, cy1, cy2;
  float am, a0, a1, a2, bm, b0, b1, b2;
  float tx , ty;
  int ix, iy;
  int i, j;
  int MESH_H, MESH_V;
  int index1, index2;
  int PED_H_MAX;
  int PED_V_MAX;

  /* Initialize the roll-off mesh grid */
  int level, scale, w, h, sgh, sgv, gh, gv, dh, dv;
  int gh_up, gv_up, dh_up, dv_up;
  int gh_full, gv_full, dh_full, dv_full;

  if (full_width - (output_width * scale_factor) - (2 * offset_x) < 0) {
   offset_x = (full_width - (output_width * scale_factor)) / 2;
   offset_x = (offset_x /2)  *2; /* floor_2*/
  }

  if (full_height - (output_height * scale_factor) - (2* offset_y) < 0) {
   offset_y = (full_height - (output_height * scale_factor)) / 2;
   offset_y = (offset_y /2) * 2; /*floor_2*/
  }

  pedestal46_calc_grid_config(
      output_width,
      output_height,
      &scale,
      &dh,
      &dv,
      &sgh,
      &sgv,
      &MESH_H,
      &MESH_V);

  gh = sgh * scale;
  gv = sgv * scale;

  /*  Upsampling the output roll-off mesh grid by scale factor */
  gh_up = gh * scale_factor;
  gv_up = gv * scale_factor;
  dh_up = dh * scale_factor;
  dv_up = dv * scale_factor;

  ISP_DBG(" full_width=%d full_height=%d output_width=%d output_height=%d offset_x=%d offset_y=%d scale_factor=%d gh_up =%d, "
          "gv_up =%d, dh_up=%d, dv_up=%d", full_width, full_height,
          output_width, output_height, offset_x, offset_y, scale_factor, gh_up, gv_up, dh_up, dv_up);

  pedestal46_calc_grid_config(
      full_width,
      full_height,
      &scale,
      &dh,
      &dv,
      &sgh,
      &sgv,
      &MESH_H,
      &MESH_V);

  gh = sgh * scale;
  gv = sgv * scale;

  gh_full = gh;
  gv_full = gv;
  dh_full = dh;
  dv_full = dv;

  float Extend_Mesh[(MESH_H + 3) * (MESH_V + 3)];

  pedestal46_extend_1block(MeshIn,  &Extend_Mesh[0], MESH_H + 1,
    MESH_V + 1, hdr_factor);

  /*  resample Extended Mesh data onto the roll-off mesh grid */
  for (i = 0; i < (MESH_V + 1); i++) {
    for (j = 0; j < (MESH_H + 1); j++) {
      /* */
      tx = (double)(j*gh_up-dh_up + offset_x/2 + dh_full + gh_full)/(double)(gh_full);
      ix = (int)floor(tx);
      tx = (double)tx - (double)ix;
      ty = (double)(i*gv_up-dv_up + offset_y/2 + dv_full + gv_full)/(double)(gv_full);
      iy = (int)floor(ty);
      ty = (double)ty - (double)iy;

      if (i == 0 || j == 0  || i == MESH_V || j == MESH_H) {
        /* for boundary points, use bilinear interpolation */
        index1 = iy    *(MESH_H+3) + ix;

        index2 = (iy+1)*(MESH_H+3) + ix;

        b1 = (1 - tx)* Extend_Mesh[index1] + tx* Extend_Mesh[index1+1];

        b2 = (1 - tx)* Extend_Mesh[index2] + tx* Extend_Mesh[index2+1];

        MeshOut[(i * (MESH_H + 1)) + j] = (unsigned short)Clamp(Round((float)((1 - ty)*b1 + ty*b2)),
                                                                0, PEDESTAL_MAX_VALUE);

      } else {
        /* for nonboundary points, use bicubic interpolation */
        /* get x direction coeff and y direction coeff */

        pedestal46_bicubic(tx,  &cxm, &cx0, &cx1, &cx2);
        pedestal46_bicubic(ty,  &cym, &cy0, &cy1, &cy2);

        am = Extend_Mesh[(iy - 1) * (MESH_H + 3) +  (ix - 1)];
        a0 = Extend_Mesh[(iy - 1) * (MESH_H + 3) +  (ix)];
        a1 = Extend_Mesh[(iy - 1) * (MESH_H + 3) +  (ix + 1)];
        a2 = Extend_Mesh[(iy - 1) * (MESH_H + 3) +  (ix + 2)];
        bm = ((cxm * am)  +  (cx0 * a0)  +  (cx1 * a1)  +  (cx2 * a2));

        am = Extend_Mesh[(iy) * (MESH_H + 3) +  (ix - 1)];
        a0 = Extend_Mesh[(iy) * (MESH_H + 3) +  (ix)];
        a1 = Extend_Mesh[(iy) * (MESH_H + 3) +  (ix + 1)];
        a2 = Extend_Mesh[(iy) * (MESH_H + 3) +  (ix + 2)];
        b0 = ((cxm * am)  +  (cx0 * a0)  +  (cx1 * a1)  +  (cx2 * a2));

        am = Extend_Mesh[(iy + 1) * (MESH_H + 3) +  (ix - 1)];
        a0 = Extend_Mesh[(iy + 1) * (MESH_H + 3) +  (ix)];
        a1 = Extend_Mesh[(iy + 1) * (MESH_H + 3) +  (ix + 1)];
        a2 = Extend_Mesh[(iy + 1) * (MESH_H + 3) +  (ix + 2)];
        b1 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Extend_Mesh[(iy + 2) * (MESH_H + 3) +  (ix - 1)];
        a0 = Extend_Mesh[(iy + 2) * (MESH_H + 3) +  (ix)];
        a1 = Extend_Mesh[(iy + 2) * (MESH_H + 3) +  (ix + 1)];
        a2 = Extend_Mesh[(iy + 2) * (MESH_H + 3) +  (ix + 2)];
        b2 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        MeshOut[(i * (MESH_H + 1)) + j] = (unsigned short)Clamp(Round((float)
                                                                      ((cym * bm) +
                                                                       (cy0 * b0) +
                                                                       (cy1 * b1) +
                                                                       (cy2 * b2))),
                                                                0,PEDESTAL_MAX_VALUE);
      }
    }
  }
}

/** pedestal46_sensor_calc_tbl:
 *
 *  @inTbl: input table
 *  @outTbl: output table
 *  @sensor_info: handle to sensor_rolloff_config_t
 *
 *  Get the tables from the chromatix pointer and normalize the values to ensure
 *  all values are >1
 *
 *  Return void
 **/
static void pedestal46_sensor_calc_tbl(chromatix_pedestalcorrection_type *inTbl,
  chromatix_pedestalcorrection_type *outTbl, sensor_rolloff_config_t* sensor_info,
  float hdr_factor, pedestal46_mod_t *mod)
{
  if (!inTbl || !outTbl || !sensor_info) {
    ISP_ERR("failed: %p %p %p", inTbl, outTbl, sensor_info);
    return;
  }

  if (sensor_info->full_width == 0 || sensor_info->output_width == 0 ||
      sensor_info->scale_factor == 0) {
    ISP_DBG("sensor send wrong config!! not exec algo!"
      "full_w = %d, out_w = %d, scale_factor = %d",
      sensor_info->full_width, sensor_info->output_width,
      sensor_info->scale_factor);
    return;
  }

  if (sensor_info->full_width <
     (sensor_info->scale_factor * sensor_info->output_width +
     sensor_info->offset_x)) {
    ISP_DBG("sensor send wrong config!! not exec algo!"
      "full_w = %d, out_w = %d, offset_x = %d, scale_factor = %d",
      sensor_info->full_width,
      sensor_info->output_width, sensor_info->offset_x,
      sensor_info->scale_factor);
    return;
  }

  outTbl->pedestalcorrection_control_enable = inTbl->pedestalcorrection_control_enable;
  pedestal46_scale_all_tables(inTbl, outTbl, sensor_info, hdr_factor);
}

/** pedestal46_table_interpolate:
 *
 *  @in1: input table 1
 *  @in2: input table 2
 *  @out: output interpolated table
 *  ratio: ratio to be used for interpolation
 *
 *  Interpolate table 1 and table 2 using input ratio
 *
 *  Return void
 **/
void pedestal46_table_interpolate(pedestalcorrection_table *in1,
  pedestalcorrection_table *in2, pedestalcorrection_table *out, float ratio)
{
  int i = 0;
  int mesh_pedestal_table_size = MESH_PedestalTable_SIZE;

  if (!in1 || !in2 || !out) {
    ISP_ERR("failed: %p %p %p", in1, in2, out);
    return;
  }

  TBL_INTERPOLATE_INT(in1->channel_black_level_r, in2->channel_black_level_r,
    out->channel_black_level_r, ratio, mesh_pedestal_table_size, i);
  TBL_INTERPOLATE_INT(in1->channel_black_level_gb, in2->channel_black_level_gb,
    out->channel_black_level_gb, ratio, mesh_pedestal_table_size, i);
  TBL_INTERPOLATE_INT(in1->channel_black_level_gr, in2->channel_black_level_gr,
    out->channel_black_level_gr, ratio, mesh_pedestal_table_size, i);
  TBL_INTERPOLATE_INT(in1->channel_black_level_b, in2->channel_black_level_b,
    out->channel_black_level_b, ratio, mesh_pedestal_table_size, i);
} /* pedestal46_table_interpolate */


/** pedestal46_prepare_hw_tables:
 *
 *
 *    @mod: pedestal module
 *
 **/
static void pedestal46_prepare_hw_tables(pedestal46_mod_t *mod)
{
  uint32_t i = 0, k = 0, gr = 0, r = 0, gb = 0, b =0;
  pedestalcorrection_table tblOut;

  for (i = 0; i < PEDESTAL_T1_T2; i++) {
    if (F_EQUAL(mod->aec_ratio, 1.0)) {
      ISP_DBG("Normal Light ");
      tblOut = mod->chromatix_tbls_output.pctable[i];
    } else {
      pedestal_correct46_get_table(mod, &tblOut, i);
    }
    for (k = 0; k < PEDESTAL_LUT_SIZE; k++) {
      gr = MIN(0xFFF, tblOut.channel_black_level_gr[k]);
      r  = MIN(0xFFF, tblOut.channel_black_level_r [k]);
      gb = MIN(0xFFF, tblOut.channel_black_level_gb[k]);
      b  = MIN(0xFFF, tblOut.channel_black_level_b [k]);

      mod->hw_tables[i].gr_r[k] = ((gr << 12) & 0xFFF000) | (r & 0xFFF);
      mod->hw_tables[i].gb_b[k] = ((gb << 12) & 0xFFF000) | (b & 0xFFF);
      ISP_DBG("hw_table[%d].gr_r[%d] = 0x%x gb_b[] = 0x%x", i, k, mod->hw_tables[i].gr_r[k],
              mod->hw_tables[i].gb_b[k]);
    }
  }
}

/** pedestal46_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Store stream configuration
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pedestal46_set_stream_config(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  pedestal46_mod_t   *mod = NULL;
  sensor_out_info_t  *sensor_out_info = NULL;
  sensor_rolloff_config_t   *sensor_rolloff_config = NULL;
  sensor_dim_output_t       *dim_output = NULL;
  sensor_request_crop_t     *request_crop = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  mod = (pedestal46_mod_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  sensor_out_info = (sensor_out_info_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(sensor_out_info);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  mod->sensor_out_info = *sensor_out_info;
  sensor_rolloff_config = &mod->sensor_out_info.sensor_rolloff_config;

  isp_sub_module_util_fill_sensor_roll_off_cfg(sensor_out_info,
    sensor_rolloff_config);

  isp_sub_module->trigger_update_pending = TRUE;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* pedestal46_set_stream_config */


/**pedestal46_calc_sub_grid:
 *
 *  @camif_width:
 *  @camif_height:
 *  @interp_factor: output interpolation factor
 *  @block_width: output block width
 *  @block height: output block height
 *  @sub_grid_width: output sub grid width
 *  @sub_grid_height: output sub grid height
 *
 *  Calculate parameters related to sub grid
 *
 *  Return void
 **/
static boolean pedestal46_calc_sub_grid(pedestal46_mod_t *mod)
{
  int SGwidth, BlockWidth, OverWidthHalf;
  int SGheight, BlockHeight, OverHeightHalf;
  int subgrid_num = 0;
  int NumHorizontalGrids = PEDESTAL_H_GRIDS;
  int NumVerticalGrids =  PEDESTAL_V_GRIDS;
  uint32_t camif_width  = 0;
  uint32_t camif_height = 0;
  ISP_Pedestal_CfgCmdType *config = &mod->pedestal_cfg;

  camif_width  = mod->sensor_out_info.request_crop.last_pixel -
    mod->sensor_out_info.request_crop.first_pixel + 1;
  camif_height = mod->sensor_out_info.request_crop.last_line -
    mod->sensor_out_info.request_crop.first_line + 1;

  if (FALSE == pedestal46_calc_grid_config(camif_width, camif_height,
    &subgrid_num , &OverWidthHalf, &OverHeightHalf, &SGwidth,
    &SGheight, &NumHorizontalGrids, &NumVerticalGrids))
  {
    ISP_ERR("pedestal47_calc_grid_config is failed");
    return FALSE;
  }

  config->blockWidth = (uint32_t)(SGwidth * subgrid_num  - 1);
  config->blockHeight = (uint32_t)(SGheight * subgrid_num  - 1);

  config->subGridWidth = (uint32_t)(SGwidth - 1);
  config->subGridHeight = (uint32_t)(SGheight - 1);
  config->interpFactor = (uint32_t)((log10((float)subgrid_num ) /
    log10(2.0))+0.5);

  if (!SGwidth || !SGheight || !subgrid_num) {
    ISP_ERR("failed! devide by 0 check! %d %d %d",
      SGwidth, SGheight, subgrid_num);
    return FALSE;
  }

  config->subGridXDelta = roundf((float)(1 << PEDESTAL_X_DELTA_Q_LEN) / SGwidth);
  config->subGridYDelta = roundf((float)(1 << PEDESTAL_Y_DELTA_Q_LEN) / SGheight);
  config->pixelXIndex = (uint32_t)(((OverWidthHalf * 2) >> 1) % SGwidth);
  config->pixelYIndex = (uint32_t)(((OverHeightHalf * 2) >> 1) % SGheight);
  config->blockXIndex = (uint32_t)(((OverWidthHalf * 2) >> 1) /
    (SGwidth * subgrid_num));
  config->blockYIndex = (uint32_t)(((OverHeightHalf * 2) >> 1) /
    (SGheight * subgrid_num));
  config->yDeltaAccum = (uint32_t)(config->subGridYDelta * config->pixelYIndex);
  config->subGridXIndex = (uint32_t)(((OverWidthHalf * 2) >> 1) / SGwidth);
  config->subGridYIndex = (uint32_t)(((OverHeightHalf * 2) >> 1) /
    SGheight);

  /* calculate right stripe settings if needed based on the whole frame
     setting */
  if (mod->ispif_out_info.is_split &&
      mod->isp_out_info.stripe_id == ISP_STRIPE_RIGHT) {
    uint32_t block_w      = (config->blockWidth + 1) * 2;
    uint32_t subgrid_w    = (config->subGridWidth + 1) * 2;
    uint32_t right_offset = config->pixelXIndex +
      block_w * config->blockXIndex +
      subgrid_w * config->subGridXIndex +
      mod->isp_out_info.right_stripe_offset;

    config->blockXIndex   = right_offset / block_w;
    config->subGridXIndex =
      (right_offset - config->blockXIndex * block_w) / subgrid_w;
    config->pixelXIndex   =
      right_offset - config->blockXIndex * block_w -
      config->subGridXIndex * subgrid_w;
  }

  return TRUE;

} /* pedestal_calc_sub_grid */

/** pedestal46_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @mod: Pointer to pedestal module
 *
 *  Configure the pedestal module

 *  Return TRUE on success and FALSE on failure
 **/
static boolean pedestal46_config(isp_sub_module_t *isp_sub_module,
  pedestal46_mod_t *mod)
{
  uint32_t i, j;
  float hdr_factor = 1.0;

  pedestal46_calc_sub_grid(mod);

  sensor_rolloff_config_t   *sensor_rolloff_config = &mod->sensor_out_info.sensor_rolloff_config;

  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR) {
    hdr_factor = (float)1.0f/((float)( 1 << (ISP_PIPELINE_WIDTH - INSENSOR_HDR_BIT_WIDTH)));
  }
  /* exec algo if sensor config are valid*/
  pedestal46_sensor_calc_tbl(
      mod->chromatix_tbls,
      &mod->chromatix_tbls_output,
      sensor_rolloff_config, hdr_factor, mod);

  mod->aec_ratio = 0.0f;
  pedestal46_prepare_hw_tables(mod);


  isp_sub_module->trigger_update_pending = TRUE;
  return TRUE;
}

/** pedestal46_set_chromatix_ptr
 *
 *  @module: mct module handle
 *  @isp_sub_mdoule: isp sub module handle
 *  @event: event handle
 *
 *  Update chromatix ptr
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pedestal46_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                     ret = TRUE;
  modulesChromatix_t         *chromatix_ptr = NULL;
  chromatix_VFE_common_type  *chromatix_common = NULL;
  pedestal46_mod_t           *mod = NULL;

  mod = (pedestal46_mod_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  chromatix_ptr =
    (modulesChromatix_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(chromatix_ptr);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  isp_sub_module->chromatix_ptrs = *chromatix_ptr;

  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }
  ISP_DBG("pedestal enable %d", isp_sub_module->submod_enable);
  chromatix_common = (chromatix_VFE_common_type *)
    chromatix_ptr->chromatixComPtr;

  mod->chromatix_tbls = &chromatix_common->chromatix_pedestal_correction;

  if (isp_sub_module->stream_on_count > 0) {
    ret = pedestal46_config(isp_sub_module, mod);
    if (ret == FALSE) {
      ISP_ERR("failed: pedestal46_config ret %d", ret);
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return FALSE;
    }
  }

  isp_sub_module->submod_trigger_enable = TRUE;
  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** pedestal46_set_stripe_info:
 *
 *  @module: mct module handle
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Perform set stripe info
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pedestal46_set_stripe_info(
  mct_module_t     *module,
  isp_sub_module_t *isp_sub_module,
  mct_event_t      *event)
{
  uint32_t            block_width          = 0;
  uint32_t            sensor_output_width  = 0;
  uint32_t            sensor_output_height = 0;
  uint32_t            distance_from_grid   = 0;
  int                 rc                   = 0;
  boolean             ret                  = TRUE;

  pedestal46_mod_t    *pedestal46     = NULL;
  isp_private_event_t *private_event  = NULL;
  ispif_out_info_t    *ispif_out_info = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  void *data = event->u.module_event.module_event_data;

  if (!data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  if (!isp_sub_module->submod_enable) {
    ISP_DBG("skip, pedestal enabled %d", isp_sub_module->submod_enable);
    return TRUE;
  }

  pedestal46 = (pedestal46_mod_t *)isp_sub_module->private_data;
  if (!pedestal46) {
    ISP_ERR("failed: pedestal 46 %p", pedestal46);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  ispif_out_info = (ispif_out_info_t *)private_event->data;
  if (!ispif_out_info) {
    ISP_ERR("failed: stripe_limit %p", ispif_out_info);
    return FALSE;
  }

  pedestal46->ispif_out_info = *ispif_out_info;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
}

/** pedestal46_set_split_info:
 *
 *  @module: mct module handle
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Perform set split info
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pedestal46_set_split_info(
    mct_module_t     *module,
    isp_sub_module_t *isp_sub_module,
    mct_event_t      *event)
{

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: module %p isp_sub_module %p event %p", module,
      isp_sub_module, event);
    return FALSE;
  }

  pedestal46_mod_t    *pedestal46_mod = NULL;
  isp_out_info_t      *isp_split_out_info = NULL;
  isp_private_event_t *private_event = NULL;

  void *data = event->u.module_event.module_event_data;

  if (!data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event      = (isp_private_event_t *)data;
  isp_split_out_info = (isp_out_info_t *)private_event->data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  pedestal46_mod = (pedestal46_mod_t *)isp_sub_module->private_data;
  if (!pedestal46_mod) {
    ISP_ERR("failed: pedestal46 %p", pedestal46_mod);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  pedestal46_mod->isp_out_info = *isp_split_out_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
}

/** pedestal46_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Perform trigger update using aec_update
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pedestal46_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                     ret = TRUE;
  pedestal46_mod_t           *mod = NULL;
  isp_private_event_t        *private_event = NULL;
  isp_sub_module_output_t    *output = NULL;
  uint8_t                     module_enable = 0;
  isp_meta_entry_t           *pedestal_dmi_info = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  mod = (pedestal46_mod_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(private_event);

  output = (isp_sub_module_output_t *)private_event->data;
  RETURN_IF_NULL(output);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->manual_ctrls.manual_update &&
      isp_sub_module->chromatix_module_enable) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    isp_sub_module->submod_trigger_enable =
      (isp_sub_module->manual_ctrls.black_level_lock ==
       CAM_BLACK_LEVEL_LOCK_ON) ? FALSE : TRUE;
  }

  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("enable = %d, ",isp_sub_module->submod_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }
  if ((isp_sub_module->submod_trigger_enable == FALSE) ||
        (isp_sub_module->trigger_update_pending == FALSE)) {
        ISP_DBG(" trigger_enable = %d hw update pending %d",
               isp_sub_module->submod_trigger_enable,
               isp_sub_module->trigger_update_pending);
      goto FILL_METADATA;
   }

  pedestal46_prepare_hw_tables(mod);

  ret = pedestal46_do_hw_update(isp_sub_module, mod, output);
  if (ret == FALSE) {
    ISP_ERR("failed: pedestal_do_hw_update ret %d", ret);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

FILL_METADATA:
  if (output->metadata_dump_enable == 1) {
    /*fill in DMI info*/
    pedestal_dmi_info = &output->
      meta_dump_params->meta_entry[ISP_META_PEDESTAL_TBL];
    /*dmi type*/
    pedestal_dmi_info->dump_type  = ISP_META_PEDESTAL_TBL;
    pedestal_dmi_info->start_addr = 0;
    output->meta_dump_params->frame_meta.num_entry++;
    if (PEDESTAL_T1_T2 != 1) {
      /*dmi table length*/
      pedestal_dmi_info->len = sizeof(mod->hw_tables);
      ISP_DBG("<new_dmi_debug> 2 TBL pedestal read length %d",
        pedestal_dmi_info->len);
      memcpy(pedestal_dmi_info->isp_meta_dump, mod->hw_tables,
       pedestal_dmi_info->len);
    } else {
      /*dmi table length*/
      pedestal_dmi_info->len = sizeof(mod->hw_tables[0]);
      ISP_DBG("<new_dmi_debug> 1 TBL pedestal read length %d",
        pedestal_dmi_info->len);
      memcpy(pedestal_dmi_info->isp_meta_dump, &mod->hw_tables[0],
       pedestal_dmi_info->len);
    }
 }

  if (output && isp_sub_module->vfe_diag_enable) {
    ret = pedestal_fill_vfe_diag_data(mod, isp_sub_module, output);
    if (ret == FALSE) {
      ISP_ERR("failed: pedestal_fill_vfe_diag_data");
    }
  }

  if (output->frame_meta)
    output->frame_meta->black_level_lock =
      isp_sub_module->manual_ctrls.black_level_lock;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* pedestal_trigger_update */

/** pedestal46_stats_aec_update:
 *
 *  @module: pedestal module
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Handle AEC update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean pedestal46_stats_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  float                       ratio = 0.0f;
  pedestal46_mod_t           *mod = NULL;
  stats_update_t             *stats_update = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  mod = (pedestal46_mod_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(stats_update);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if(mod->chromatix_tbls)
    ratio = pedestal_correct46_get_aec_ratio(mod, stats_update);

  if (!F_EQUAL(mod->aec_ratio, ratio)) {
    isp_sub_module->trigger_update_pending = TRUE;
    mod->aec_ratio = ratio;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;
}

/** pedestal46_streamon:
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
boolean pedestal46_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                ret = TRUE;
  pedestal46_mod_t      *mod = NULL;
  isp_sub_module_priv_t *isp_sub_module_priv = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  isp_sub_module_priv = (isp_sub_module_priv_t *)MCT_OBJECT_PRIVATE(module);
  RETURN_IF_NULL(isp_sub_module_priv);

  mod = (pedestal46_mod_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  if (isp_sub_module->submod_enable == FALSE) {
    ISP_DBG("pedestal enable = %d", isp_sub_module->submod_enable);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  ret = pedestal46_config(isp_sub_module, mod);
  if (ret == FALSE) {
    ISP_ERR("failed: pedestal46_config ret %d", ret);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* pedestal46_streamon */

/** pedestal46_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: mct event handle
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean pedestal46_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  pedestal46_mod_t *mod = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  mod = (pedestal46_mod_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(mod);

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  pedestal46_reset(mod);
  isp_sub_module->trigger_update_pending = FALSE;
  isp_sub_module->manual_ctrls.black_level_lock = CAM_BLACK_LEVEL_LOCK_OFF;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* pedestal46_streamoff */

/** pedestal46_destroy:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void pedestal46_destroy(mct_module_t *module,
  isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p",
      module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
} /* pedestal46_destroy */
