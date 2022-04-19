/* abf40.c
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_ABF, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_ABF, fmt, ##args)

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "isp_common.h"
#include "isp_pipeline_reg.h"
#include "abf40.h"


#define CLAMP_ABF(x1,v1) ((x1 > v1) ? v1 : x1 )

#define PRINT_LUT(dst, len) \
     {\
    for (i = 0; i < (len); i++) { \
      ISP_HIGH("LUT index:[%d]== %x",i,(dst)[i]);\
    } \
  }

#define NORMAL_LIGHT_INDEX 4

/** abf_cmd_debug:
 *
 *  @pCmd: Pointer to the ABF register command
 *  print out params in config cmd
 *
 *  Print the curent abf regiser config
 *
 **/
static void abf_cmd_debug(ISP_DemosaicABF2_CmdType* pCmd)
{
  int i = 0;
  ISP_DemosaicABF2_GCfg*  gCfg = &(pCmd->gCfg);
  ISP_DemosaicABF2_RBCfg* rCfg = &(pCmd->rCfg);
  ISP_DemosaicABF2_RBCfg* bCfg = &(pCmd->bCfg);

  ISP_DemosaicABF2_Lut* gNLut = (pCmd->gNegLut);
  ISP_DemosaicABF2_Lut* rNLut = (pCmd->rNegLut);
  ISP_DemosaicABF2_Lut* bNLut = (pCmd->bNegLut);

  ISP_DemosaicABF2_Lut* gPLut = (pCmd->gPosLut);
  ISP_DemosaicABF2_Lut* rPLut = (pCmd->rPosLut);
  ISP_DemosaicABF2_Lut* bPLut = (pCmd->bPosLut);

  ISP_DBG(" =====Green parametets ===============\n");
  ISP_DBG("abf2 green: cutoff1=%d, cutoff2=%d,cutoff3=%d.\n",
    gCfg->Cutoff1, gCfg->Cutoff2, gCfg->Cutoff3);
  ISP_DBG("abf2 green: multiPositive=%d, multiNegative=%d.\n",
    gCfg->MultPositive, gCfg->MultNegative);
  ISP_DBG("abf2 green: A0 = %d, A1 = %d.\n",
    gCfg->SpatialKernelA0,gCfg->SpatialKernelA1);

  for (i = 0; i < 8; i++)
    ISP_DBG("Green PosLUT: coef%d=%d,coef%d=%d\n",
      2*i, gPLut[i].LUT0, 2*i+1, gPLut[i].LUT1);

  for (i = 0; i < 4; i++)
    ISP_DBG("Green NegLUT: coef%d=%d,coef%d=%d\n",
      2*i, gNLut[i].LUT0, 2*i+1, gNLut[i].LUT1);

  ISP_DBG("=====Red parametets ===============\n");
  ISP_DBG("abf2 red: cutoff1=%d, cutoff2=%d,cutoff3=%d.\n",
    rCfg->Cutoff1, rCfg->Cutoff2, rCfg->Cutoff3);
  ISP_DBG("abf2 red: multiPositive=%d, multiNegative=%d.\n",
    rCfg->MultPositive, rCfg->MultNegative);

  for (i = 0; i < 8; i++)
    ISP_DBG("Red PosLUT: coef%d=%d,coef%d=%d\n",
      2*i, rPLut[i].LUT0, 2*i+1, rPLut[i].LUT1);

  for (i = 0; i < 4; i++)
    ISP_DBG("Red NegLUT: coef%d=%d,coef%d=%d\n",
      2*i, rNLut[i].LUT0, 2*i+1, rNLut[i].LUT1);

  ISP_DBG("=====Blue parametets ===============\n");
  ISP_DBG("abf2 blue: cutoff1=%d, cutoff2=%d,cutoff3=%d.\n",
    bCfg->Cutoff1, bCfg->Cutoff2, bCfg->Cutoff3);
  ISP_DBG("abf2 blue: multiPositive=%d, multiNegative=%d.\n",
    bCfg->MultPositive, bCfg->MultNegative);

  for (i = 0; i < 8; i++)
    ISP_DBG("Blue PosLUT: coef%d=%d,coef%d=%d\n",
      2*i, bPLut[i].LUT0, 2*i+1, bPLut[i].LUT1);

  for (i = 0; i < 4; i++)
    ISP_DBG("Blue NegLUT: coef%d=%d,coef%d=%d\n",
      2*i, bNLut[i].LUT0, 2*i+1, bNLut[i].LUT1);
} /* abf_cmd_debug */

/** abf40_interpolate:
 *
 *  @pv1: pointer to the abf config for
 *        current lighting condition
 *  @pv2: pointer to the abf config for
 *        normla light
 *  @pv_out: Pointer to the abf_params
 *           that neds to be calaculated
 *  @ratio: AEC ratio
 *
 *  Interpoltae the abf params based on the
 *  lighting condition and aec ratio.
 *
 **/
static void abf40_interpolate(chromatix_adaptive_bayer_filter_data_type2* pv1,
  chromatix_adaptive_bayer_filter_data_type2* pv2, abf2_parms_t* pv_out,
  float ratio)
{
  int32_t i;

  TBL_INTERPOLATE_INT(pv1->threshold_red, pv2->threshold_red,
    pv_out->data.threshold_red, ratio, 3, i);
  TBL_INTERPOLATE_INT(pv1->threshold_green, pv2->threshold_green,
    pv_out->data.threshold_green, ratio, 3, i);
  TBL_INTERPOLATE_INT(pv1->threshold_blue, pv2->threshold_blue,
    pv_out->data.threshold_blue, ratio, 3, i);

  for (i=0; i<16; i++) {
    pv_out->g_table.table_pos[i] =
      LINEAR_INTERPOLATION(pv1->scale_factor_green[0] * pv1->table_pos[i],
      pv2->scale_factor_green[0] * pv2->table_pos[i], ratio);
    pv_out->b_table.table_pos[i] =
      LINEAR_INTERPOLATION(pv1->scale_factor_blue[0] * pv1->table_pos[i],
      pv2->scale_factor_blue[0] * pv2->table_pos[i], ratio);
    pv_out->r_table.table_pos[i] =
      LINEAR_INTERPOLATION(pv1->scale_factor_red[0] * pv1->table_pos[i],
      pv2->scale_factor_red[0] * pv2->table_pos[i], ratio);
  }

  for (i=0; i<8; i++) {
    pv_out->g_table.table_neg[i] =
      LINEAR_INTERPOLATION(pv1->scale_factor_green[1] * pv1->table_neg[i],
      pv2->scale_factor_green[1] * pv2->table_neg[i], ratio);
    pv_out->b_table.table_neg[i] =
      LINEAR_INTERPOLATION(pv1->scale_factor_blue[1] * pv1->table_neg[i],
      pv2->scale_factor_blue[1] * pv2->table_neg[i], ratio);
    pv_out->r_table.table_neg[i] =
      LINEAR_INTERPOLATION(pv1->scale_factor_red[1] * pv1->table_neg[i],
      pv2->scale_factor_red[1] * pv2->table_neg[i], ratio);
  }

  TBL_INTERPOLATE(pv1->a, pv2->a, pv_out->data.a, ratio, 2, i);
} /* abf40_interpolate */


/** abf_set_params_common
 *
 *  @p_cmd: Pointer to abf register command
 *  @abf2_data: pointer to the abf config data
 *
 *  Set the abf register command params which are common
 *
 **/
static void abf_set_params_common(ISP_DemosaicABF2_CmdType* p_cmd,
  chromatix_adaptive_bayer_filter_data_type2* abf2_data)
{
  p_cmd->gCfg.Cutoff1 = ABF2_CUTOFF1(abf2_data->threshold_green[0]);
  p_cmd->bCfg.Cutoff1 = ABF2_CUTOFF1(abf2_data->threshold_blue[0]);
  p_cmd->rCfg.Cutoff1 = ABF2_CUTOFF1(abf2_data->threshold_red[0]);

  p_cmd->gCfg.Cutoff2 = ABF2_CUTOFF2(p_cmd->gCfg.Cutoff1,
    abf2_data->threshold_green[1]);
  p_cmd->bCfg.Cutoff2 = ABF2_CUTOFF2(p_cmd->bCfg.Cutoff1,
    abf2_data->threshold_blue[1]);
  p_cmd->rCfg.Cutoff2 = ABF2_CUTOFF2(p_cmd->rCfg.Cutoff1,
    abf2_data->threshold_red[1]);

  p_cmd->gCfg.Cutoff3 = ABF2_CUTOFF3(p_cmd->gCfg.Cutoff2,
    abf2_data->threshold_green[2]);
  p_cmd->bCfg.Cutoff3 = ABF2_CUTOFF3(p_cmd->bCfg.Cutoff2,
    abf2_data->threshold_blue[2]);
  p_cmd->rCfg.Cutoff3 = ABF2_CUTOFF3(p_cmd->rCfg.Cutoff2,
    abf2_data->threshold_red[2]);
  p_cmd->bCfg.gain_offset = 0;
  p_cmd->rCfg.gain_offset = 0;

  p_cmd->gCfg.MultNegative = ABF2_MULT_NEG(p_cmd->gCfg.Cutoff2,
    p_cmd->gCfg.Cutoff3);
  p_cmd->bCfg.MultNegative = ABF2_MULT_NEG(p_cmd->bCfg.Cutoff2,
    p_cmd->bCfg.Cutoff3);
  p_cmd->rCfg.MultNegative = ABF2_MULT_NEG(p_cmd->rCfg.Cutoff2,
    p_cmd->rCfg.Cutoff3);

  p_cmd->gCfg.MultPositive = ABF2_MULT_POS(p_cmd->gCfg.Cutoff1);
  p_cmd->bCfg.MultPositive = ABF2_MULT_POS(p_cmd->bCfg.Cutoff1);
  p_cmd->rCfg.MultPositive = ABF2_MULT_POS(p_cmd->rCfg.Cutoff1);

  p_cmd->gCfg.SpatialKernelA0 = ABF2_SP_KERNEL(abf2_data->a[0]);
  p_cmd->gCfg.SpatialKernelA1 = ABF2_SP_KERNEL(abf2_data->a[1]);
} /* abf_set_params_common */

/** abf_set_cmd_params_chromatix
 *
 *  @p_cmd: Pointer to abf register command
 *  @abf2_data: pointer to the abf config data
 *
 *  Set the abf register command params
 *
 **/
static boolean abf_set_cmd_params_chromatix(ISP_DemosaicABF2_CmdType* p_cmd,
  chromatix_adaptive_bayer_filter_data_type2* abf2_data)
{
  int i = 0;
  uint32_t temp;
  int8_t rc = TRUE;
  abf_set_params_common(p_cmd, abf2_data);

  for (i=0; i<8; i++) {
    p_cmd->gPosLut[i].LUT0 =
      ABF2_LUT(abf2_data->scale_factor_green[0]*abf2_data->table_pos[2*i]);
    p_cmd->bPosLut[i].LUT0 =
      ABF2_LUT(abf2_data->scale_factor_blue[0]*abf2_data->table_pos[2*i]);
    p_cmd->rPosLut[i].LUT0 =
      ABF2_LUT(abf2_data->scale_factor_red[0]*abf2_data->table_pos[2*i]);
    p_cmd->gPosLut[i].LUT1 =
      ABF2_LUT(abf2_data->scale_factor_green[0]*abf2_data->table_pos[2*i+1]);
    p_cmd->bPosLut[i].LUT1 =
      ABF2_LUT(abf2_data->scale_factor_blue[0]*abf2_data->table_pos[2*i+1]);
    p_cmd->rPosLut[i].LUT1 =
      ABF2_LUT(abf2_data->scale_factor_red[0]*abf2_data->table_pos[2*i+1]);
  }

  for (i=0; i<4; i++) {
    p_cmd->gNegLut[i].LUT0 =
      ABF2_LUT(abf2_data->scale_factor_green[1]*abf2_data->table_neg[2*i]);
    p_cmd->bNegLut[i].LUT0 =
      ABF2_LUT(abf2_data->scale_factor_blue[1]*abf2_data->table_neg[2*i]);
    p_cmd->rNegLut[i].LUT0 =
      ABF2_LUT(abf2_data->scale_factor_red[1]*abf2_data->table_neg[2*i]);
    p_cmd->gNegLut[i].LUT1 =
      ABF2_LUT(abf2_data->scale_factor_green[1]*abf2_data->table_neg[2*i+1]);
    p_cmd->bNegLut[i].LUT1 =
      ABF2_LUT(abf2_data->scale_factor_blue[1]*abf2_data->table_neg[2*i+1]);
    p_cmd->rNegLut[i].LUT1 =
      ABF2_LUT(abf2_data->scale_factor_red[1]*abf2_data->table_neg[2*i+1]);
  }

  return rc;
} /* abf_set_cmd_params_chromatix */

/** abf_set_cmd_params_interpolate
 *
 *  @p_cmd: Pointer to abf register command
 *  @abf2_data: pointer to the abf config data
 *
 *  Set the abf register command params
 *
 **/
static int8_t abf_set_cmd_params_interpolate(ISP_DemosaicABF2_CmdType* p_cmd,
  abf2_parms_t* abf2_parms)
{
  int i = 0;
  int8_t rc = TRUE;
  uint32_t temp;

  abf_set_params_common(p_cmd, &abf2_parms->data);

  for (i=0; i<8; i++) {
    p_cmd->gPosLut[i].LUT0 =
      ABF2_LUT(abf2_parms->g_table.table_pos[2*i]);
    p_cmd->bPosLut[i].LUT0 =
      ABF2_LUT(abf2_parms->b_table.table_pos[2*i]);
    p_cmd->rPosLut[i].LUT0 =
      ABF2_LUT(abf2_parms->r_table.table_pos[2*i]);
    p_cmd->gPosLut[i].LUT1 =
      ABF2_LUT(abf2_parms->g_table.table_pos[2*i+1]);
    p_cmd->bPosLut[i].LUT1 =
      ABF2_LUT(abf2_parms->b_table.table_pos[2*i+1]);
    p_cmd->rPosLut[i].LUT1 =
      ABF2_LUT(abf2_parms->r_table.table_pos[2*i+1]);
  }

  for (i=0; i<4; i++) {
    p_cmd->gNegLut[i].LUT0 =
      ABF2_LUT(abf2_parms->g_table.table_neg[2*i]);
    p_cmd->bNegLut[i].LUT0 =
      ABF2_LUT(abf2_parms->b_table.table_neg[2*i]);
    p_cmd->rNegLut[i].LUT0 =
      ABF2_LUT(abf2_parms->r_table.table_neg[2*i]);
    p_cmd->gNegLut[i].LUT1 =
      ABF2_LUT(abf2_parms->g_table.table_neg[2*i+1]);
    p_cmd->bNegLut[i].LUT1 =
      ABF2_LUT(abf2_parms->b_table.table_neg[2*i+1]);
    p_cmd->rNegLut[i].LUT1 =
      ABF2_LUT(abf2_parms->r_table.table_neg[2*i+1]);
  }

  return rc;
} /* abf_set_cmd_params_interpolate */

/** abf40_init_default
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf handle
 *
 *  abf module configuration initial settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean abf40_init_default(isp_sub_module_t *isp_sub_module,
  abf40_t *abf)
{
  int i = 0;
  chromatix_parms_type *chroma_ptr = NULL;

  if (!isp_sub_module || !abf) {
    ISP_ERR("failed: %p %p", isp_sub_module, abf);
    return FALSE;
  }

  chroma_ptr = (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chroma_ptr) {
    ISP_ERR("Null pointer! %p", chroma_ptr);
    return FALSE;
  }

  /* V2 configuration */
    chromatix_adaptive_bayer_filter_data_type2 *abf2_data = NULL;
    chromatix_ABF2_type *chromatix_ABF2 =
      &chroma_ptr->chromatix_VFE.chromatix_ABF2;

    memset(&abf->abf2_parms, 0x0, sizeof(abf2_parms_t));
    abf2_data = &(chromatix_ABF2->abf2_config[NORMAL_LIGHT_INDEX]);
    abf_set_cmd_params_chromatix(&abf->v2RegCmd, abf2_data);

    abf->trigger_index = NORMAL_LIGHT_INDEX;

 return TRUE;
} /* abf40_init_default */

/** abf40_config_trigger_v2
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf handle
 *
 *  abf module configuration initial settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
static void abf40_config_trigger_v2(isp_sub_module_t *isp_sub_module,
  abf40_t *abf)
{
  int i = 0;
  chromatix_parms_type                       *chromatix_ptr = NULL;
  chromatix_ABF2_type                        *chromatix_ABF2 = NULL;
  chromatix_adaptive_bayer_filter_data_type2 *abf2_config;

  if (!isp_sub_module || !abf) {
    ISP_ERR("failed: %p %p", isp_sub_module, abf);
    return;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return;
  }

  chromatix_ABF2 =
    &chromatix_ptr->chromatix_VFE.chromatix_ABF2;

  abf2_config = &(chromatix_ABF2->abf2_config[abf->trigger_index]);
  abf_set_cmd_params_chromatix(&abf->v2RegCmd, abf2_config);

  return;
}

/** abf40_config_trigger_interpolate_v2
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf handle
 *
 *  abf module configuration initial settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
static void abf40_config_trigger_interpolate_v2(isp_sub_module_t *isp_sub_module,
  abf40_t *abf)
{
  int i = 0;
  chromatix_parms_type                       *chromatix_ptr = NULL;
  chromatix_ABF2_type                        *chromatix_ABF2 = NULL;
  abf2_parms_t                               *abf2_parms = NULL;
  chromatix_adaptive_bayer_filter_data_type2 *abf2_config_start = NULL;
  chromatix_adaptive_bayer_filter_data_type2 *abf2_config_end = NULL;

  if (!isp_sub_module || !abf) {
    ISP_ERR("failed: %p %p", isp_sub_module, abf);
    return;
  }
  abf2_parms = &(abf->abf2_parms);
  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return;
  }

  chromatix_ABF2 =
    &chromatix_ptr->chromatix_VFE.chromatix_ABF2;

  if (abf->trigger_index + 1 >=
    MAX_LIGHT_TYPES_FOR_SPATIAL + NUM_SCENE_MODES) {
    ISP_ERR("invalid trigger index %d for interpolation", abf->trigger_index);
    return;
  }
  abf2_config_start = &(chromatix_ABF2->abf2_config[abf->trigger_index]);
  abf2_config_end = &(chromatix_ABF2->abf2_config[abf->trigger_index + 1]);

  /* the aec ratio here is the distance ratio to the start index,
     therefore, when doing interpolation,
     this ratio should apply to End index, and (1 - ratio) apply to start*/
  abf40_interpolate(abf2_config_end, abf2_config_start,
      abf2_parms, abf->aec_ratio.ratio);
  abf_set_cmd_params_interpolate(&abf->v2RegCmd, abf2_parms);

  return;
}


/** abf40_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf struct data
 *
 *  prepare hw update list and append in isp sub module
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean abf40_store_hw_update(
  isp_sub_module_t *isp_sub_module, abf40_t *abf)
{
  boolean                           ret = TRUE;
  struct msm_vfe_cfg_cmd2          *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd       *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list      *hw_update = NULL;
  ISP_DemosaicABF2_CmdType         *v2RegCmd = NULL;
  int i;
  uint32_t *data;

  if (!isp_sub_module || !abf) {
    ISP_ERR("failed: %p %p", isp_sub_module, abf);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }

  memset(hw_update, 0, sizeof(*hw_update));
  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd =
    (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(2 * sizeof(*reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }

  memset(reg_cfg_cmd, 0, sizeof(struct msm_vfe_reg_cfg_cmd) * 2);
  v2RegCmd = (ISP_DemosaicABF2_CmdType *)CAM_MALLOC(sizeof(*v2RegCmd));
  if (!v2RegCmd) {
    ISP_ERR("failed: reg_cmd %p", v2RegCmd);
    goto ERROR_REG_CMD;
  }

  memset(v2RegCmd, 0, sizeof(*v2RegCmd));
  *v2RegCmd = abf->v2RegCmd;

  cfg_cmd->cfg_data = (void *)v2RegCmd;
  cfg_cmd->cmd_len = sizeof(*v2RegCmd);
  cfg_cmd->cfg_cmd = (void *) reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_ABF2_OFF;
  reg_cfg_cmd[0].u.rw_info.len = ISP_ABF2_LEN * sizeof(uint32_t);

  abf_cmd_debug(&(abf->v2RegCmd));
  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);

  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    free(v2RegCmd);
    goto ERROR_REG_CMD;
  }

  isp_sub_module->trigger_update_pending = FALSE;
  return TRUE;

ERROR_REG_CMD:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD:
  free(hw_update);
  return FALSE;
} /* abf44_store_hw_update */

/** abf40_trigger_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Perform trigger update if trigger_update_pending flag is
 *  TRUE and append hw update list in global list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean abf40_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                                    ret = TRUE;
  abf40_t                                    *abf = NULL;
  isp_private_event_t                        *private_event = NULL;
  isp_sub_module_output_t                    *output = NULL;
  chromatix_parms_type                       *chromatix_ptr = NULL;
  isp_sub_module_output_t                    *sub_module_output = NULL;
  isp_meta_entry_t                           *abf_dmi_info = NULL;
  uint32_t                                   *abf_dmi_tbl = NULL;
  uint8_t                                     module_enable;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  private_event = (isp_private_event_t *)data;
  if (!private_event) {
     ISP_ERR("failed: private_event %p", private_event);
     return FALSE;
  }

  sub_module_output = (isp_sub_module_output_t *)private_event->data;
  if (!sub_module_output) {
     ISP_ERR("failed:sub_module_output  %p", sub_module_output);
     return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  abf = (abf40_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    goto ERROR;
  }

  if (isp_sub_module->manual_ctrls.manual_update &&
      isp_sub_module->chromatix_module_enable) {
    isp_sub_module->manual_ctrls.manual_update = FALSE;
    module_enable = (isp_sub_module->manual_ctrls.abf_mode ==
      CAM_NOISE_REDUCTION_MODE_OFF) ? FALSE : TRUE;

    if (isp_sub_module->submod_enable != module_enable) {
      isp_sub_module->submod_enable = module_enable;
      sub_module_output->stats_params->
        module_enable_info.reconfig_needed = TRUE;
      sub_module_output->stats_params->module_enable_info.
      submod_enable[isp_sub_module->hw_module_id] = module_enable;
      sub_module_output->stats_params->
        module_enable_info.submod_mask[isp_sub_module->hw_module_id] = 1;

      if (!isp_sub_module->submod_enable) {
        goto FILL_METADATA;
      }
    }
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: stats_update %p", chromatix_ptr);
    goto ERROR;
  }

  if (isp_sub_module->submod_enable == FALSE) {
    /* ABF module is disabled, do not perform hw update */
    goto FILL_METADATA;
  }

  if ((isp_sub_module->submod_trigger_enable == TRUE) &&
    (isp_sub_module->trigger_update_pending == TRUE)) {
    ISP_DBG("aec_ratio.ratio = %f",abf->aec_ratio.ratio);
      if (abf->aec_ratio.ratio > 0)
          abf40_config_trigger_interpolate_v2(isp_sub_module, abf);
      else
          abf40_config_trigger_v2(isp_sub_module, abf);

    abf40_store_hw_update(isp_sub_module, abf);
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
          sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR;
  }

  if (sub_module_output->metadata_dump_enable == 1) {

    abf_dmi_info = &sub_module_output->
      meta_dump_params->meta_entry[ISP_META_ABF_TBL];

     abf_dmi_info->len =
        sizeof(abf2_table_t) * 3;
     /*dmi type */
     abf_dmi_info->dump_type  = ISP_META_ABF_TBL;
     abf_dmi_info->start_addr = 0;
     sub_module_output->meta_dump_params->frame_meta.num_entry++;

#ifdef DYNAMIC_DMI_ALLOC
      abf_dmi_tbl =
        (uint32_t *)CAM_MALLOC(abf_dmi_info->len);
      if (!abf_dmi_tbl) {
        ISP_ERR("failed: %p", abf_dmi_tbl);
        ret = FALSE;
        goto ERROR;
      }
#endif
      memcpy(abf_dmi_info->isp_meta_dump,
        &abf->abf2_parms.r_table,
        sizeof(abf2_table_t));
      memcpy(abf_dmi_info->isp_meta_dump +
        sizeof(abf2_table_t),
        &abf->abf2_parms.g_table,
        sizeof(abf2_table_t));
      memcpy(abf_dmi_info->isp_meta_dump +
        sizeof(abf2_table_t) * 2,
        &abf->abf2_parms.b_table,
        sizeof(abf2_table_t));
  }

FILL_METADATA:
   if (sub_module_output->frame_meta)
     sub_module_output->frame_meta->abf_mode =
       isp_sub_module->manual_ctrls.abf_mode;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** abf40_aec_update:
 *
 * @mod: abf module
 * @data: handle to stats_update_t
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean abf40_aec_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  int                    i = 0;
  stats_update_t        *stats_update = NULL;
  abf40_t               *abf = NULL;
  trigger_ratio_t        aec_ratio;
  chromatix_parms_type  *chromatix_ptr = NULL;
  chromatix_ABF2_type   *chromatix_ABF2 = NULL;
  trigger_point_type    *out_trigger_point = NULL;
  trigger_point_type    *low_trigger_point = NULL;
  uint8_t                trigger_index = MAX_LIGHT_TYPES_FOR_SPATIAL + 1;
  float                  aec_reference;
  float                  ratio = 0;
  tuning_control_type    abf_tuning_control;
  trigger_point_type     *trigger_point = NULL;
  boolean                ret = TRUE;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  abf = (abf40_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
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

  chromatix_ABF2 =
    &chromatix_ptr->chromatix_VFE.chromatix_ABF2;

  abf_tuning_control = chromatix_ABF2->control_abf2;

  if (abf_tuning_control == 0) {
    /* lux index based */
    aec_reference = stats_update->aec_update.lux_idx;
  } else {
    /* Gain based */
    aec_reference = stats_update->aec_update.sensor_gain;
  }

  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
    float start = 0;
    float end = 0;
    trigger_point =
      &(chromatix_ABF2->abf2_config[i].abf2_trigger);

    trigger_index = i;

    if (i == MAX_LIGHT_TYPES_FOR_SPATIAL - 1) {
      /* falls within region 6 but we do not use trigger points in the region */
      ratio = 0;
      break;
    }

    if (abf_tuning_control == 0) {
      start = trigger_point->lux_index_start;
      end   = trigger_point->lux_index_end;
    } else {
      start = trigger_point->gain_start;
      end   = trigger_point->gain_end;
    }

    /* index is within interpolation range, find ratio */
    if (aec_reference >= start && aec_reference < end) {
      ratio = (aec_reference - start)/(end - start);
      ISP_DBG("%s [%f - %f - %f] = %f", __func__, start, aec_reference, end,
         ratio);
    }

    /* already scanned past the lux index */
    if (aec_reference < end) {
       break;
    }
  }

  if (trigger_index >= MAX_LIGHT_TYPES_FOR_SPATIAL) {
    ISP_ERR("no trigger match for ABF trigger value: %f\n", aec_reference);
    goto ERROR;
  }
  if (abf->ext_func_table && abf->ext_func_table->get_trigger_index) {
    ret = abf->ext_func_table->get_trigger_index(abf, &trigger_index);
    if (ret == FALSE) {
      ISP_ERR("failed: get_trigger_index");
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return FALSE;
    }
    if (trigger_index >= MAX_LIGHT_TYPES_FOR_SPATIAL + NUM_SCENE_MODES) {
      ISP_ERR("no trigger match for ABF trigger value: %f\n", aec_reference);
      goto ERROR;
    }
  }

  if ((trigger_index != abf->trigger_index) ||
    (!F_EQUAL(ratio, abf->aec_ratio.ratio))) {
    abf->trigger_index = trigger_index;
    abf->aec_ratio.ratio = ratio;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** abf40_set_chromatix_ptr:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean abf40_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean              ret = TRUE;
  abf40_t *abf = NULL;
  modulesChromatix_t  *chromatix_ptrs = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  abf = (abf40_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: mod %p", abf);
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

  ret = abf40_init_default(isp_sub_module, abf);
  if (ret == FALSE) {
    ISP_ERR("failed: abf40_config ret %d", ret);
    goto ERROR;
  }

  isp_sub_module->trigger_update_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* abf_set_chromatix_ptr */

/** abf40_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean abf40_streamoff(isp_sub_module_t *isp_sub_module,
  void *data)
{
  abf40_t *abf = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  abf = (abf40_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  memset(abf, 0, sizeof(*abf));

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* abf40_streamoff */

/** abf40_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the abf module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean abf40_init(isp_sub_module_t *isp_sub_module)
{
  abf40_t *abf = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  abf = (abf40_t *)CAM_MALLOC(sizeof(abf40_t));
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    return FALSE;
  }

  memset(abf, 0, sizeof(*abf));

  isp_sub_module->private_data = (void *)abf;
  isp_sub_module->manual_ctrls.abf_mode = CAM_NOISE_REDUCTION_MODE_FAST;

  return TRUE;
}/* abf40_init */

/** abf40_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void abf40_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* abf40_destroy */

#if !OVERRIDE_FUNC
static ext_override_func abf_override_func = {
  .get_trigger_index = NULL,
};

boolean abf40_fill_func_table(abf40_t *abf)
{
  abf->ext_func_table = &abf_override_func;
  return TRUE;
}
#endif
