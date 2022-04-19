/* abf48.c
 *
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_sub_module_util.h"
#include "isp_common.h"
#include "isp_pipeline_reg.h"
#include "abf48.h"
#include "isp_zzhdr_tune_def.h"

isp_zzhdr_2d_lut_type zzhdr_abf_tuning_data =
    #include "isp_zzhdr_tune_param.h"
;

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_ABF, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_ABF, fmt, ##args)


//#define DUMP_ABF

#define PRINT_LUT(dst, len) \
     {\
    for (i = 0; i < (len); i++) { \
    ISP_ERR("LUT index:[%d]== %d",i,(dst)[i]);\
    } \
  }

/** abf48_debug
 *  debug function for abf config
 *
 **/
static void abf48_debug(ISP_ABF34_CmdType *cmd)
{
  ISP_DBG(" filterEn                :%x", cmd->cfg_cmd.cfg.filterEn);
  ISP_DBG(" lutBankSel              :%x", cmd->cfg_cmd.cfg.lutBankSel);
  ISP_DBG(" crossPlEn               :%x", cmd->cfg_cmd.cfg.crossPlEn);
  ISP_DBG(" singleBpcEn             :%x", cmd->cfg_cmd.cfg.singleBpcEn);
  ISP_DBG(" pixelMatchLvGrGb        :%x", cmd->cfg_cmd.cfg.pixelMatchLvGrGb);
  ISP_DBG(" pixelMatchLvRb          :%x", cmd->cfg_cmd.cfg.pixelMatchLvRb);
  ISP_DBG(" distGrGb_0              :%x", cmd->cfg_cmd.cfg.distGrGb_0);
  ISP_DBG(" distGrGb_1              :%x", cmd->cfg_cmd.cfg.distGrGb_1);
  ISP_DBG(" distGrGb_2              :%x", cmd->cfg_cmd.cfg.distGrGb_2);
  ISP_DBG(" distRb_0                :%x", cmd->cfg_cmd.cfg.distRb_0);
  ISP_DBG(" distRb_1                :%x", cmd->cfg_cmd.cfg.distRb_1);
  ISP_DBG(" distRb_2                :%x", cmd->cfg_cmd.cfg.distRb_2);
  ISP_DBG("");
  ISP_DBG(" grCfg_curveOffset       :%u", cmd->cfg_cmd2.grCfg.curveOffset);
  ISP_DBG(" gbCfg_curveOffset       :%u", cmd->cfg_cmd2.gbCfg.curveOffset);
  ISP_DBG(" rCfg_curveOffset        :%u", cmd->cfg_cmd2.rCfg.curveOffset);
  ISP_DBG(" bCfg_curveOffset        :%u", cmd->cfg_cmd2.bCfg.curveOffset);
  ISP_DBG("");
  ISP_DBG(" rnrCfg_bx               :%x", cmd->cfg_cmd2.rnrCfg.bx);
  ISP_DBG(" rnrCfg_by               :%x", cmd->cfg_cmd2.rnrCfg.by);
  ISP_DBG(" rnrCfg_initSquare       :%x", cmd->cfg_cmd2.rnrCfg.initSquare);
  ISP_DBG(" rnrCfg_anchor_0         :%x", cmd->cfg_cmd2.rnrCfg.anchor_0);
  ISP_DBG(" rnrCfg_anchor_1         :%x", cmd->cfg_cmd2.rnrCfg.anchor_1);
  ISP_DBG(" rnrCfg_anchor_2         :%x", cmd->cfg_cmd2.rnrCfg.anchor_2);
  ISP_DBG(" rnrCfg_anchor_3         :%x", cmd->cfg_cmd2.rnrCfg.anchor_3);
  ISP_DBG(" rnrCfg_coeffBase_0      :%x", cmd->cfg_cmd2.rnrCfg.coeffBase_0);
  ISP_DBG(" rnrCfg_coeffSlope_0     :%x", cmd->cfg_cmd2.rnrCfg.coeffSlope_0);
  ISP_DBG(" rnrCfg_coeffShift_0     :%x", cmd->cfg_cmd2.rnrCfg.coeffShift_0);
  ISP_DBG(" rnrCfg_coeffBase_1      :%x", cmd->cfg_cmd2.rnrCfg.coeffBase_1);
  ISP_DBG(" rnrCfg_coeffSlope_1     :%x", cmd->cfg_cmd2.rnrCfg.coeffSlope_1);
  ISP_DBG(" rnrCfg_coeffShift_1     :%x", cmd->cfg_cmd2.rnrCfg.coeffShift_1);
  ISP_DBG(" rnrCfg_coeffBase_2      :%x", cmd->cfg_cmd2.rnrCfg.coeffBase_2);
  ISP_DBG(" rnrCfg_coeffSlope_2     :%x", cmd->cfg_cmd2.rnrCfg.coeffSlope_2);
  ISP_DBG(" rnrCfg_coeffShift_2     :%x", cmd->cfg_cmd2.rnrCfg.coeffShift_2);
  ISP_DBG(" rnrCfg_coeffBase_3      :%x", cmd->cfg_cmd2.rnrCfg.coeffBase_3);
  ISP_DBG(" rnrCfg_coeffSlope_3     :%x", cmd->cfg_cmd2.rnrCfg.coeffSlope_3);
  ISP_DBG(" rnrCfg_coeffShift_3     :%x", cmd->cfg_cmd2.rnrCfg.coeffShift_3);
  ISP_DBG(" rnrCfg_threshBase_0     :%x", cmd->cfg_cmd2.rnrCfg.threshBase_0);
  ISP_DBG(" rnrCfg_threshSlope_0    :%x", cmd->cfg_cmd2.rnrCfg.threshSlope_0);
  ISP_DBG(" rnrCfg_threshShift_0    :%x", cmd->cfg_cmd2.rnrCfg.threshShift_0);
  ISP_DBG(" rnrCfg_threshBase_1     :%x", cmd->cfg_cmd2.rnrCfg.threshBase_1);
  ISP_DBG(" rnrCfg_threshSlope_1    :%x", cmd->cfg_cmd2.rnrCfg.threshSlope_1);
  ISP_DBG(" rnrCfg_threshShift_1    :%x", cmd->cfg_cmd2.rnrCfg.threshShift_1);
  ISP_DBG(" rnrCfg_threshBase_2     :%x", cmd->cfg_cmd2.rnrCfg.threshBase_2);
  ISP_DBG(" rnrCfg_threshSlope_2    :%x", cmd->cfg_cmd2.rnrCfg.threshSlope_2);
  ISP_DBG(" rnrCfg_threshShift_2    :%x", cmd->cfg_cmd2.rnrCfg.threshShift_2);
  ISP_DBG(" rnrCfg_threshBase_3     :%x", cmd->cfg_cmd2.rnrCfg.threshBase_3);
  ISP_DBG(" rnrCfg_threshSlope_3    :%x", cmd->cfg_cmd2.rnrCfg.threshSlope_3);
  ISP_DBG(" rnrCfg_threshShift_3    :%x", cmd->cfg_cmd2.rnrCfg.threshShift_3);
  ISP_DBG(" rsquareShift            :%x", cmd->cfg_cmd2.rnrCfg.rsquareShift);
  ISP_DBG("");
  ISP_DBG(" bpcCfg_fmax             :%x", cmd->cfg_cmd2.bpcCfg.fmax);
  ISP_DBG(" bpcCfg_fmin             :%x", cmd->cfg_cmd2.bpcCfg.fmin);
  ISP_DBG(" bpcCfg_offset           :%x", cmd->cfg_cmd2.bpcCfg.offset);
  ISP_DBG(" bpcCfg_minShift         :%x", cmd->cfg_cmd2.bpcCfg.minShift);
  ISP_DBG(" bpcCfg_maxShift         :%x", cmd->cfg_cmd2.bpcCfg.maxShift);
  ISP_DBG(" bpcCfg_bls              :%x", cmd->cfg_cmd2.bpcCfg.bls);
  ISP_DBG("");
  ISP_DBG(" noisePrsvCfg_anchor_lo  :%x",
    cmd->cfg_cmd2.noisePrsvCfg.anchor_lo);
  ISP_DBG(" noisePrsvCfg_anchor_gap :%x",
    cmd->cfg_cmd2.noisePrsvCfg.anchor_gap);
  ISP_DBG(" noisePrsvCfg_lo_grgb    :%x", cmd->cfg_cmd2.noisePrsvCfg.lo_grgb);
  ISP_DBG(" noisePrsvCfg_slope_grgb :%x",
    cmd->cfg_cmd2.noisePrsvCfg.slope_grgb);
  ISP_DBG(" noisePrsvCfg_shift_grgb :%x",
    cmd->cfg_cmd2.noisePrsvCfg.shift_grgb);
  ISP_DBG(" noisePrsvCfg_lo_rb      :%x", cmd->cfg_cmd2.noisePrsvCfg.lo_rb);
  ISP_DBG(" noisePrsvCfg_slope_rb   :%x", cmd->cfg_cmd2.noisePrsvCfg.slope_rb);
  ISP_DBG(" noisePrsvCfg_shift_rb   :%x", cmd->cfg_cmd2.noisePrsvCfg.shift_rb);

}

/** abf48_config_interpolate_cfg
.*
 *  @abf: abf handle
 *  @abf34_core_type : start
 *  @abf34_core_type : end
 *  @ratio: Ratio for interpolation
 *
 *  abf module reserve channel configuration
 *  This config does not change frame to frame. Set by Chromatix
 *
 **/
static void abf48_config_interpolate_cfg(abf48_t *abf,
  chromatix_abf34_core_type   *abf34_core_start,
  chromatix_abf34_core_type   *abf34_core_end, float ratio)
{
  int temp;
  float tempf;

  temp = abf34_core_start->distance_ker[0][0];
  temp = Clamp(temp, DISTANCE_KER_IDX_0_MIN, DISTANCE_KER_IDX_0_MAX);
  abf->v34RegCmd.cfg_cmd.cfg.distRb_0 = (uint16_t)temp;

  temp = abf34_core_start->distance_ker[0][1];
  temp = Clamp(temp, DISTANCE_KER_IDX_1_MIN, DISTANCE_KER_IDX_1_MAX);
  abf->v34RegCmd.cfg_cmd.cfg.distRb_1 = (uint16_t)temp;

  temp = abf34_core_start->distance_ker[0][2];
  temp = Clamp(temp, DISTANCE_KER_IDX_2_MIN, DISTANCE_KER_IDX_2_MAX);
  abf->v34RegCmd.cfg_cmd.cfg.distRb_2 = (uint16_t)temp;

  temp = abf34_core_start->distance_ker[1][0];
  temp = Clamp(temp, DISTANCE_KER_IDX_0_MIN, DISTANCE_KER_IDX_0_MAX);
  abf->v34RegCmd.cfg_cmd.cfg.distGrGb_0 = (uint16_t)temp;

  temp = abf34_core_start->distance_ker[1][1];
  temp = Clamp(temp, DISTANCE_KER_IDX_1_MIN, DISTANCE_KER_IDX_1_MAX);
  abf->v34RegCmd.cfg_cmd.cfg.distGrGb_1 = (uint16_t)temp;

  temp = abf34_core_start->distance_ker[1][2];
  temp = Clamp(temp, DISTANCE_KER_IDX_2_MIN, DISTANCE_KER_IDX_2_MAX);
  abf->v34RegCmd.cfg_cmd.cfg.distGrGb_2 = (uint16_t)temp;

  abf->blk_pix_matching_g = (LINEAR_INTERPOLATION(((float)CLAMP_ABF
    (abf34_core_start->blk_pix_matching_g, 4)),
    ((float)CLAMP_ABF(abf34_core_end->blk_pix_matching_g, 4)), ratio));
  temp = Round(abf->blk_pix_matching_g);
  abf->v34RegCmd.cfg_cmd.cfg.pixelMatchLvGrGb =
    Clamp(temp, ABF3_BLKPIX_MIN, ABF3_BLKPIX_MAX);

  abf->blk_pix_matching_rb = (LINEAR_INTERPOLATION(((float)CLAMP_ABF
    (abf34_core_start->blk_pix_matching_rb, 4)),
    ((float)CLAMP_ABF(abf34_core_end->blk_pix_matching_rb, 4)), ratio));
  temp = Round(abf->blk_pix_matching_rb);
  abf->v34RegCmd.cfg_cmd.cfg.pixelMatchLvRb =
    Clamp(temp, ABF3_BLKPIX_MIN, ABF3_BLKPIX_MAX);

  abf->bpc_fmax = (LINEAR_INTERPOLATION((float)abf34_core_start->bpc_fmax,
    (float)abf34_core_end->bpc_fmax, ratio));
  temp = Round(abf->bpc_fmax);
  abf->v34RegCmd.cfg_cmd2.bpcCfg.fmax =
    Clamp(temp, ABF_SBPC_FMAX_MIN, ABF_SBPC_FMAX_MAX);

  abf->bpc_fmin = (LINEAR_INTERPOLATION((float)abf34_core_start->bpc_fmin,
    (float)abf34_core_end->bpc_fmin, ratio));
  temp = Round(abf->bpc_fmin);
  abf->v34RegCmd.cfg_cmd2.bpcCfg.fmin =
    Clamp(temp, ABF_SBPC_FMIN_MIN, ABF_SBPC_FMIN_MAX);

  abf->bpc_maxshft = (LINEAR_INTERPOLATION((float)abf34_core_start->bpc_maxshft,
    (float)abf34_core_end->bpc_maxshft, ratio));
  temp = Round(abf->bpc_maxshft);
  abf->v34RegCmd.cfg_cmd2.bpcCfg.maxShift =
    Clamp(temp, ABF_SBPC_MAXSHFT_MIN, ABF_SBPC_MAXSHFT_MAX);

  abf->bpc_minshft = (LINEAR_INTERPOLATION((float)abf34_core_start->bpc_minshft,
    (float)abf34_core_end->bpc_minshft, ratio));
  temp = Round(abf->bpc_minshft);
  abf->v34RegCmd.cfg_cmd2.bpcCfg.minShift =
    Clamp(temp,ABF_SBPC_MAXSHFT_MIN,    ABF_SBPC_MINSHFT_MAX);

  abf->bpc_offset = (LINEAR_INTERPOLATION(
    (abf34_core_start->bpc_offset * abf->multfactor),
    (abf34_core_end->bpc_offset * abf->multfactor),
    ratio));
  temp = Round(abf->bpc_offset);
  abf->v34RegCmd.cfg_cmd2.bpcCfg.offset    = CLAMP_ABF(temp, ABF_SBPC_BPC_OFF_MAX);

  tempf = (LINEAR_INTERPOLATION((float)abf34_core_start->curve_offset[0],
    (float)abf34_core_end->curve_offset[0], ratio));
  temp = Round(tempf);
  temp = CLAMP_ABF(temp, ABF34_CURVEOFFSET_MAX);
  abf->v34RegCmd.cfg_cmd2.rCfg.curveOffset = (uint16_t)temp;

  tempf = (LINEAR_INTERPOLATION((float)abf34_core_start->curve_offset[1],
    (float)abf34_core_end->curve_offset[1], ratio));
  temp = Round(tempf);
  temp = CLAMP_ABF(temp, ABF34_CURVEOFFSET_MAX);
  abf->v34RegCmd.cfg_cmd2.grCfg.curveOffset = (uint16_t)temp;

  tempf = (LINEAR_INTERPOLATION((float)abf34_core_start->curve_offset[2],
    (float)abf34_core_end->curve_offset[2], ratio));
  temp = Round(tempf);
  temp = CLAMP_ABF(temp, ABF34_CURVEOFFSET_MAX);
  abf->v34RegCmd.cfg_cmd2.gbCfg.curveOffset = (uint16_t)temp;

  tempf = (LINEAR_INTERPOLATION((float)abf34_core_start->curve_offset[3],
    (float)abf34_core_end->curve_offset[3], ratio));
  temp = Round(tempf);
  temp = CLAMP_ABF(temp, ABF34_CURVEOFFSET_MAX);
  abf->v34RegCmd.cfg_cmd2.bCfg.curveOffset = (uint16_t)temp;

  abf->edge_softness = (float)(LINEAR_INTERPOLATION(abf34_core_start->edge_softness,
    abf34_core_end->edge_softness, ratio));

}

/** abf48_config_interpolate_reserve
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf handle
 *  This config does not change frame to frame. Set by Chromatix
 *
 *  abf module reserve configuration
 *
 *  Return None
 **/
static boolean abf48_config_interpolate_reserve(isp_sub_module_t
   *isp_sub_module, abf48_t *abf, uint8_t trigger_index, float ratio)
{
  int i = 0;
  chromatix_abf34_reserve_type   *abf34_reserve = NULL;
  chromatix_parms_type     *chromatix_ptr = NULL;
  chromatix_ABFV34_type    *chromatix_ABF34 = NULL;
  chromatix_abf34_core_type  *abf34_core_start = NULL;
  chromatix_abf34_core_type  *abf34_core_end = NULL;
  chromatix_videoHDR_type    *chromatix_VHDR = NULL;
  chromatix_videoHDR_reserve *chromatix_HDR_reserve = NULL;
  int hdr_enable = 0;
  float  multfactor = 1.0f;

  if (!isp_sub_module || !abf) {
    ISP_ERR("failed: %p %p", isp_sub_module, abf);
    return FALSE;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("Null pointer! %p", chromatix_ptr);
    return FALSE;
  }
  chromatix_ABF34 = &chromatix_ptr->chromatix_VFE.chromatix_ABFV34_data;

  chromatix_VHDR =
    &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR;

  chromatix_HDR_reserve = &chromatix_VHDR->videoHDR_reserve_data;

  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
    isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG) {
    multfactor = 1.0f /(1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH));
  } else {
      multfactor = 1 << (ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH);
  }
  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
    isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG) {
    hdr_enable = 1;
  }
  abf->multfactor = multfactor;
  abf->hdr_enable = hdr_enable;

  abf34_core_start = &(chromatix_ABF34->abf34_config[trigger_index]);
  if ((abf->trigger_index + 1 >= MAX_LIGHT_TYPES_FOR_SPATIAL_ABF) ||
    F_EQUAL(abf->aec_ratio.ratio, 0.0) ||
    ((abf->trigger_index == NORMAL_LIGHT_INDEX) &&
    F_EQUAL(abf->aec_ratio.ratio, 0.0)))
    abf34_core_end = &(chromatix_ABF34->abf34_config[trigger_index]);
  else
    abf34_core_end = &(chromatix_ABF34->abf34_config[trigger_index + 1]);
  abf34_reserve = &(chromatix_ABF34->reserve_data);

  abf->v34RegCmd.cfg_cmd.cfg.filterEn = abf->filterEn;
  abf->v34RegCmd.cfg_cmd.cfg.singleBpcEn = abf->singleBpcEn;
  abf->v34RegCmd.cfg_cmd.cfg.crossPlEn =
    CLAMP_ABF(abf34_reserve->cross_channel_process_en, 1);
  abf48_config_interpolate_cfg(abf, abf34_core_start, abf34_core_end, ratio);

  for (i=0; i < ABFV34_NOISE_STD_LENGTH; i++) {
    abf->noise_std_lut_level[i] = (LINEAR_INTERPOLATION(
    (float)(abf34_core_start->noise_std_lut_level[i]),
    (float)(abf34_core_end->noise_std_lut_level[i]), ratio));
  }

  return TRUE;
}

/** abf48_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean abf48_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data)
{
  abf48_t                 *abf48 = NULL;
  sensor_out_info_t       *sensor_out_info = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sensor_out_info = (sensor_out_info_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  abf48 = (abf48_t *)isp_sub_module->private_data;
  if (!abf48) {
    ISP_ERR("failed: abf48 %p", abf48);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  abf48->sensor_out_info = *sensor_out_info;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;

} /* abf48_set_stream_config */

/** abf48_init_default
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf handle
 *
 *  abf module configuration initial settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean abf48_init_default(isp_sub_module_t *isp_sub_module,
  abf48_t *abf)
{
  int i = 0;

  if (!isp_sub_module || !abf) {
    ISP_ERR("failed: %p %p", isp_sub_module, abf);
    return FALSE;
  }

  abf->trigger_index = NORMAL_LIGHT_INDEX;
  abf->init_h_offset = 0; // TBD Need to modify for Dual VFE
  abf->init_v_offset = 0; // TBD Need to modify for Dual VFE

  return TRUE;
} /* abg47_config */


/** abf48_fetch_tables
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf struct data
 *
 *  Interpolate and config all registers
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean abf48_fetch_tables(isp_sub_module_t *isp_sub_module,
  abf48_t *abf)
{

  chromatix_ABFV34_type *chromatix_ABF34 = NULL;
  chromatix_parms_type  *chromatix_ptr = NULL;
  chromatix_abf34_core_type  *abf34_core_start = NULL;
  chromatix_abf34_core_type  *abf34_core_end = NULL;
  int j, i;
  float ratio;

  if (!isp_sub_module || !abf) {
    ISP_ERR("failed:sub_module_output %p abf %p", isp_sub_module, abf);
    return FALSE;
  }

  ratio = (1 - abf->aec_ratio.ratio);

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("Null pointer! %p", chromatix_ptr);
    return FALSE;
  }
  chromatix_ABF34 = &chromatix_ptr->chromatix_VFE.chromatix_ABFV34_data;
  abf34_core_start = &(chromatix_ABF34->abf34_config[abf->trigger_index]);
  if ((ratio == 1.0f) || ((abf->trigger_index + 1) == MAX_LIGHT_TYPES_FOR_SPATIAL_ABF))
    abf34_core_end = &(chromatix_ABF34->abf34_config[abf->trigger_index]);
  else
    abf34_core_end = &(chromatix_ABF34->abf34_config[abf->trigger_index + 1]);
  for ( j = 0 ; j < ABF34_RADIAL_POINTS ; j++) {
    abf->anchor_table[j] =chromatix_ABF34->reserve_data.anchor_table[j];
    if ((j < ABF34_RADIAL_POINTS-1 ) &&
      (chromatix_ABF34->reserve_data.anchor_table[j] >
      chromatix_ABF34->reserve_data.anchor_table[j+1])) {
      ISP_ERR("VFE48:: ABF34 :: ERROR :: Anchor Table Should be \
        Monotonically increasing [%d] [%f] [%d] [%f]",
        j,chromatix_ABF34->reserve_data.anchor_table[j],
        j+1,chromatix_ABF34->reserve_data.anchor_table[j+1]);
      return FALSE;
    }
  }

  for ( i = 0 ; i < 2 ; i++) {
    for ( j = 0 ; j < ABF34_RADIAL_POINTS ; j++) {
      abf->base_table[i][j] =Round(LINEAR_INTERPOLATION(
        (float)abf34_core_start->base_table[i][j],
        (float)abf34_core_end->base_table[i][j], ratio));
    }
  }

    for ( i = 0 ; i < 2 ; i++) {
      for ( j = 0 ; j < (ABF34_RADIAL_POINTS-1) ; j++) {
        if  (abf->base_table[i][j] < abf->base_table[i][j+1]) {
          ISP_ERR("VFE48:: ABF34 :: ERROR :: Base Table Should be Monotonically \
            Decreasing [%d][%d]= [%d] , [%d][%d]=[%d]",
            i,j,abf->base_table[i][j],
            i,j+1,abf->base_table[i][j+1]);
            return FALSE;
        }
      }
    }
    for (j=0; j<2; j++) { /* 0(RB)/1(G) */
      abf->filter_strength_hi[j] = (float)LINEAR_INTERPOLATION(
        (float)abf34_core_start->filter_strength_hi[j],
        (float)abf34_core_end->filter_strength_hi[j], ratio);
      abf->filter_strength_lo[j] = (float)LINEAR_INTERPOLATION(
        (float)abf34_core_start->filter_strength_lo[j],
        (float)abf34_core_end->filter_strength_lo[j], ratio);
    }
    abf->filter_strength_anchor_pt_lo = Round((float)LINEAR_INTERPOLATION(
      (float)abf34_core_start->filter_strength_anchor_pt_lo,
      (float)abf34_core_end->filter_strength_anchor_pt_lo, ratio));
    abf->filter_strength_anchor_pt_hi = Round((float)LINEAR_INTERPOLATION(
      (float)abf34_core_start->filter_strength_anchor_pt_hi,
      (float)abf34_core_end->filter_strength_anchor_pt_hi, ratio));

   return TRUE;
}

/** abf48_config_trigger
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf struct data
 *
 *  Interpolate and config all registers
 *
 * Returns TRUE on success and FALSE on failure
 **/
static void abf48_config_trigger(isp_sub_module_t *isp_sub_module,
  abf48_t *abf)
{

  chromatix_ABFV34_type *chromatix_ABF34 = NULL;
  chromatix_parms_type  *chromatix_ptr = NULL;
  chromatix_abf34_core_type  *abf34_core = NULL;
  float ratio = (1 - abf->aec_ratio.ratio);

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("Null pointer! %p", chromatix_ptr);
    return;
  }
  chromatix_ABF34 = &chromatix_ptr->chromatix_VFE.chromatix_ABFV34_data;

  ISP_DBG("trigger index %u, abf->aec_ratio.ratio %f",
    abf->trigger_index, abf->aec_ratio.ratio);

  abf48_config_interpolate_reserve(isp_sub_module, abf, abf->trigger_index, ratio);
}

static int32_t abf_adjust_signbit(int32_t val, uint8_t num_bits)
{
  if (val < 0) {
    val = -(val);
    val &= ((1 << num_bits) - 1);
    val = (1 << num_bits) - val;
  } else {
    val &= ((1 << num_bits) - 1);
  }
  return val;
}

void noise_std_lut_gen(abf48_t *abf)
{
  uint32_t  x, y;
  int lut_delta;
  int i;
  int absflag = 1;
  int nbits = 9;

  for (i = 0; i < ABFV34_NOISE_STD_LENGTH - 1; i++) {
    x = CLAMP_ABF((abf->noise_std_lut_level[i]),
      ABF34_NOISESTDLUT_BASE_MAX);
    if (i < ABFV34_NOISE_STD_LENGTH - 1)
        y = CLAMP_ABF((abf->noise_std_lut_level[i+1]),
          ABF34_NOISESTDLUT_BASE_MAX);
    else
        y = (abf->noise_std_lut_level[i+1]);
    lut_delta = y - x;
      if (absflag) {
        if (lut_delta > 0) {
          ISP_ERR("VFE48:: ABF34 :: ERROR :: Noise Std LUT Should be \
            Monotonically increasing lut[%d]:%d lut[%d] %d",i,x,i+1,y);
        }
        lut_delta = abs(lut_delta);
      }
    lut_delta = CLAMP_ABF(lut_delta, ABF34_NOISESTDLUT_DELTA_MAX);

    abf->abf34_parms.noise_std_lut_level[i] =  (x | (lut_delta *
      (1 << nbits)));
  }
}

/** abf48_prepare_table:
 *
 *  @abf: abf struct data
 *
 *  prepare hw update list and append in isp sub module
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean abf48_prepare_table(isp_sub_module_t *isp_sub_module,
  abf48_t *abf)
{
  boolean ret = TRUE;
  int i, j;
  int32_t bx_temp, by_temp, temp;
  uint16_t anchor_table[5];
  uint16_t base_table[2][5];
  int16_t  slope_table[2][ABF34_RADIAL_POINTS];
  int16_t  shift_table[2][ABF34_RADIAL_POINTS];
  uint16_t dd1, bb1;
  float   tmp0;
  int32_t tmp00;
  uint32_t noise_offset_max_val_chromatix = (1<<16)-1;
  uint16_t bb = 13;
  uint16_t noise_prsv_hi[2];
  uint32_t camif_width = 0, camif_height = 0;

  if (!abf  || !isp_sub_module) {
    ISP_ERR("failed: %p %p ", abf, isp_sub_module);
    return FALSE;
  }
  camif_width = abf->sensor_out_info.request_crop.last_pixel
   - abf->sensor_out_info.request_crop.first_pixel + 1;
  camif_height = abf->sensor_out_info.request_crop.last_line
   - abf->sensor_out_info.request_crop.first_line + 1;

  temp = Round(abf->BLSS_offset * abf->multfactor);
  abf->v34RegCmd.cfg_cmd2.bpcCfg.bls = CLAMP_ABF(temp, ABF34_BPC_BLS_MAX);

  bx_temp = (int32_t)(abf->init_h_offset - camif_width/2);
  bx_temp = Clamp(bx_temp,
    ABF34_BPC_BX_MIN, ABF34_BPC_BX_MAX);
  abf->v34RegCmd.cfg_cmd2.rnrCfg.bx = abf_adjust_signbit(bx_temp, 14);

  by_temp = (int32_t)(abf->init_v_offset - camif_height/2);
  by_temp = Clamp(by_temp,
    ABF34_BPC_BY_MIN, ABF34_BPC_BY_MAX);

  abf->v34RegCmd.cfg_cmd2.rnrCfg.by = abf_adjust_signbit(by_temp, 14);

  abf->v34RegCmd.cfg_cmd2.rnrCfg.initSquare = bx_temp
    * bx_temp + by_temp * by_temp;
  abf->v34RegCmd.cfg_cmd2.rnrCfg.initSquare = Clamp(abf->v34RegCmd.cfg_cmd2.rnrCfg.initSquare,
    ABF34_RSQRE_INIT_MIN, ABF34_RSQRE_INIT_MAX);
  abf->v34RegCmd.cfg_cmd2.rnrCfg.rsquareShift = 0;
  for (i=0; i<16; i++) {
    if ( abf->v34RegCmd.cfg_cmd2.rnrCfg.initSquare <= (uint32_t)(4096 << i)) {
      abf->v34RegCmd.cfg_cmd2.rnrCfg.rsquareShift = i;
      break;
    }
  }
  abf->v34RegCmd.cfg_cmd2.rnrCfg.rsquareShift = Clamp
    (abf->v34RegCmd.cfg_cmd2.rnrCfg.rsquareShift,
    ABF34_RSQRE_SHFT_MIN,ABF34_RSQRE_SHFT_MAX);


    for (i=0; i<5; i++) {
      base_table[0][i] = Clamp(abf->base_table[0][i],
        ABF34_BASE_MIN,ABF34_BASE_MAX);
      base_table[1][i] = Clamp(abf->base_table[1][i],
        ABF34_BASE_MIN,ABF34_BASE_MAX);
    }
    abf->v34RegCmd.cfg_cmd2.rnrCfg.rsquareShift =
      Clamp(abf->v34RegCmd.cfg_cmd2.rnrCfg.rsquareShift,0,15);

    // from 5-entry table in header to 4-entry table in cfg
    for (i=0; i<4; i++) {
      anchor_table[i+1] = Round((abf->anchor_table[i+1]
        * (abf->v34RegCmd.cfg_cmd2.rnrCfg.initSquare >>
        abf->v34RegCmd.cfg_cmd2.rnrCfg.rsquareShift)));
      anchor_table[i] = Round((abf->anchor_table[i]
        * (abf->v34RegCmd.cfg_cmd2.rnrCfg.initSquare >>
        abf->v34RegCmd.cfg_cmd2.rnrCfg.rsquareShift)));

      dd1 = Clamp(anchor_table[i+1] - anchor_table[i], 1, (1<<12)-1);

      bb1 = Clamp(base_table[0][i] - base_table[0][i+1], 1, 255);
      shift_table[0][i] = (bb1==0)? 0 : (uint16_t)((float)(log(dd1)+log(255)
        - log(bb1))/(float)log(2));
      slope_table[0][i] = (uint16_t)Round(((bb1<<(shift_table[0][i]))/dd1));
      shift_table[0][i] = Clamp(shift_table[0][i],
        ABF34_SHIFT_MIN,ABF34_SHIFT_MAX);
      slope_table[0][i] = Clamp(slope_table[0][i],
        ABF34_SLOPE_MIN,ABF34_SLOPE_MAX);

      bb1 = Clamp(base_table[1][i] - base_table[1][i+1], 1, 255);
      shift_table[1][i] = (bb1==0)? 0 : (uint16_t)((float)(log(dd1)
        + log(255)-log(bb1))/(float)log(2));
      slope_table[1][i] = (uint16_t)Round(((bb1<<(shift_table[1][i]))/dd1));
      shift_table[1][i] = Clamp(shift_table[1][i],
        ABF34_SHIFT_MIN,ABF34_SHIFT_MAX);
      slope_table[1][i] = Clamp(slope_table[1][i],
        ABF34_SLOPE_MIN,ABF34_SLOPE_MAX);
    }
    abf->v34RegCmd.cfg_cmd2.rnrCfg.coeffSlope_0 = slope_table[0][0];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.coeffSlope_1 = slope_table[0][1];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.coeffSlope_2 = slope_table[0][2];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.coeffSlope_3 = slope_table[0][3];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.coeffShift_0 = shift_table[0][0];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.coeffShift_1 = shift_table[0][1];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.coeffShift_2 = shift_table[0][2];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.coeffShift_3 = shift_table[0][3];

    abf->v34RegCmd.cfg_cmd2.rnrCfg.threshSlope_0 = slope_table[1][0];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.threshSlope_1 = slope_table[1][1];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.threshSlope_2 = slope_table[1][2];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.threshSlope_3 = slope_table[1][3];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.threshShift_0 = shift_table[1][0];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.threshShift_1 = shift_table[1][1];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.threshShift_2 = shift_table[1][2];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.threshShift_3 = shift_table[1][3];

    for (i=0; i<4; i++) {
      base_table[0][i] = Clamp(abf->base_table[0][i],
        ABF34_BASE_MIN,ABF34_BASE_MAX);
      base_table[1][i] = Clamp(abf->base_table[1][i],
        ABF34_BASE_MIN,ABF34_BASE_MAX);
      anchor_table[i] = Clamp(Round((1<<12)*abf->anchor_table[i]),
        ABF34_ANCHOR_MIN,ABF34_ANCHOR_MAX);
    }

    abf->v34RegCmd.cfg_cmd2.rnrCfg.coeffBase_0 = base_table[0][0];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.coeffBase_1 = base_table[0][1];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.coeffBase_2 = base_table[0][2];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.coeffBase_3 = base_table[0][3];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.threshBase_0 = base_table[1][0];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.threshBase_1 = base_table[1][1];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.threshBase_2 = base_table[1][2];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.threshBase_3 = base_table[1][3];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.anchor_0 = anchor_table[0];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.anchor_1 = anchor_table[1];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.anchor_2 = anchor_table[2];
    abf->v34RegCmd.cfg_cmd2.rnrCfg.anchor_3 = anchor_table[3];

    if ((abf->ext_func_table && abf->ext_func_table->apply_2d_interpolation)
      && (abf->apply_hdr_effects)) {
      abf->ext_func_table->apply_2d_interpolation(abf);
    }

    for (j=0; j < ABFV34_NOISE_STD_LENGTH; j++) {
      abf->noise_std_lut_level[j] = CLAMP_ABF(abf->noise_std_lut_level[j],
        (float)noise_offset_max_val_chromatix);
      tmp0 = abf->noise_std_lut_level[j] *(abf->edge_softness);
      tmp00 = (int32_t)Round(((tmp0==0.0f)? (1<<bb) : (float)(1<<bb)/(float)tmp0));
      abf->noise_std_lut_level[j] = tmp00;
    }
    abf->edge_softness = Round(abf->edge_softness*(1<<8));

    abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.anchor_lo =
      CLAMP_ABF(abf->filter_strength_anchor_pt_lo, ((1<<10)-1));
    if (abf->filter_strength_anchor_pt_hi ==
      abf->filter_strength_anchor_pt_lo) {
      abf->filter_strength_anchor_pt_hi =
        abf->filter_strength_anchor_pt_lo + 1;
    }

    abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.anchor_gap = CLAMP_ABF
      ((abf->filter_strength_anchor_pt_hi - abf->filter_strength_anchor_pt_lo),
       ((1<<10)-1)); // in 10u

    abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.lo_grgb = Clamp(Round((float)(1<<8)
      * abf->filter_strength_lo[0]),0,ABF34_FILTERNOISEPRESERVE_MAX);
    abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.lo_rb = Clamp( Round((float)(1<<8)
      * abf->filter_strength_lo[1]),0,ABF34_FILTERNOISEPRESERVE_MAX);
    noise_prsv_hi[0] = Clamp( Round((float)(1<<8)*abf->filter_strength_hi[0])
      ,0,ABF34_FILTERNOISEPRESERVE_MAX);
    noise_prsv_hi[1] = Clamp( Round((float)(1<<8)*abf->filter_strength_hi[1])
      ,0,ABF34_FILTERNOISEPRESERVE_MAX);

    if (noise_prsv_hi[0] >= abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.lo_grgb) {
      for ( i=10; i>=0; i--) {
        if ((((noise_prsv_hi[0]-abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.lo_grgb)<<i) /
          abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.anchor_gap)<=256) {
          abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.shift_grgb = i;
          abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.slope_grgb = Round
            ((float)((noise_prsv_hi[0] - abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.lo_grgb)
            << i) / (float)abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.anchor_gap);
          break;
        }
      }
    } else {
      for ( i=10; i>=0; i--) {
        if ((((noise_prsv_hi[0]-abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.lo_grgb) << i)
          /abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.anchor_gap)>=-256) {
          abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.shift_grgb = i;
          abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.slope_grgb = Round
            ((float)((noise_prsv_hi[0]
            - abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.lo_grgb) << i)
            / (float)abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.anchor_gap);
          break;
        }
      }
    }

    if (noise_prsv_hi[1]>=abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.lo_rb) {
      for (i=10; i>=0; i--) {
        if ((((noise_prsv_hi[1]-abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.lo_rb) << i)
          /abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.anchor_gap)<=256) {
          abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.shift_rb = i;
          abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.slope_rb = Round
            ((float)((noise_prsv_hi[1]
            - abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.lo_rb) << i)
            /(float)abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.anchor_gap);
          break;
        }
      }
    } else {
      for (i=10; i>=0; i--) {
        if ((((noise_prsv_hi[1] - abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.lo_rb) << i)
          /abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.anchor_gap)>=-256) {
          abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.shift_rb = i;
          abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.slope_rb =
            Round((float)((noise_prsv_hi[1]
            - abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.lo_rb) << i)
            /(float)abf->v34RegCmd.cfg_cmd2.noisePrsvCfg.anchor_gap);
          break;
        }
      }
    }

   noise_std_lut_gen(abf);


  return TRUE;
} /* abf48_prepare_table */


/** abf48_store_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf struct data
 *
 *  prepare hw update list and append in isp sub module
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean abf48_store_hw_update(
  isp_sub_module_t *isp_sub_module, abf48_t *abf)
{
  boolean                           ret = TRUE;
  struct msm_vfe_cfg_cmd2          *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd       *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list      *hw_update = NULL;
  ISP_ABF34_CmdType                *v34RegCmd = NULL;
  uint32_t bank;
  uint32_t channel[2] = {
      ABF_STD2_L0_BANK0,
      ABF_STD2_L0_BANK1,
    };

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));
  cfg_cmd = &hw_update->cfg_cmd;

  if (ABF_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      ABF_CGC_OVERRIDE_REGISTER, ABF_CGC_OVERRIDE_BIT, TRUE);
    if (ret == FALSE) {
      ISP_ERR("failed: enable cgc");
    }
  }

  abf->v34RegCmd.cfg_cmd.cfg.lutBankSel ^= 1;
  bank = abf->v34RegCmd.cfg_cmd.cfg.lutBankSel;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd) * 2);
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd));

  v34RegCmd = (ISP_ABF34_CmdType *)CAM_MALLOC(sizeof(*v34RegCmd));
  if (!v34RegCmd) {
    ISP_ERR("failed: reg_cmd %p", v34RegCmd);
    goto ERROR;
  }
  memset(v34RegCmd, 0, sizeof(*v34RegCmd));

  ret = isp_sub_module_util_write_dmi(
   (void *)&abf->abf34_parms.noise_std_lut_level,
   (sizeof(noise_std2_lut_t) - sizeof(uint32_t)),
    channel[bank],
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_g");
    goto ERROR;
  }

  *v34RegCmd = abf->v34RegCmd;
  /* configurations */
  cfg_cmd->cfg_data = (void *)v34RegCmd;
  cfg_cmd->cmd_len = sizeof(*v34RegCmd);
  cfg_cmd->cfg_cmd = (void *) reg_cfg_cmd;
  cfg_cmd->num_cfg = 2;

  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_ABF34_CFG;
  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].u.rw_info.len = ISP_ABF34_CFG_LEN * sizeof(uint32_t);

  reg_cfg_cmd[1].cmd_type = VFE_WRITE;
  reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_ABF34_CFG2;
  reg_cfg_cmd[1].u.rw_info.cmd_data_offset =
  reg_cfg_cmd[0].u.rw_info.cmd_data_offset + sizeof(ISP_ABF34_cfg_cmd_t);
  reg_cfg_cmd[1].u.rw_info.len = ISP_ABF34_CFG2_LEN * sizeof(uint32_t);

  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR;
  }

  if (ABF_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      ABF_CGC_OVERRIDE_REGISTER, ABF_CGC_OVERRIDE_BIT, FALSE);
    if (ret == FALSE) {
      ISP_ERR("failed: disable cgc");
    }
  }

  isp_sub_module->trigger_update_pending = FALSE;
  abf->apply_hdr_effects = 0;
  return TRUE;

ERROR:
  if (v34RegCmd) {
    free(v34RegCmd);
  }
  if (reg_cfg_cmd) {
    free(reg_cfg_cmd);
  }
  if (hw_update) {
    free(hw_update);
  }

  return FALSE;
} /* abf48_store_hw_update */

/** abf48_calcualte_hdr_ratio
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf module private data
 *  @event_identity: identity of trigger update mct event
 *
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean abf48_calcualte_hdr_ratio(isp_sub_module_t
   *isp_sub_module, abf48_t *abf)
{
  boolean ret = FALSE;
  float start = 0;
  float end = 0;
  float ratio = 0;
  uint8_t trigger_index = MAX_SETS_FOR_TONE_NOISE_ADJ + 1;
  int i = 0;
  chromatix_ABFV34_type *chromatix_ABF34 = NULL;
  chromatix_parms_type  *chromatix_ptr = NULL;
  Chromatix_HDR_ABF3_adj_type *abf3_hdr_adj_data = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed:sub_module_output  %p", isp_sub_module);
    return FALSE;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("Null pointer! %p", chromatix_ptr);
    return FALSE;
  }
  chromatix_ABF34 = &chromatix_ptr->chromatix_VFE.chromatix_ABFV34_data;
  abf3_hdr_adj_data = &chromatix_ptr->
    chromatix_VFE.chromatix_hdr_abf3_adj_data;

  for (i = 0; i < MAX_SETS_FOR_TONE_NOISE_ADJ; i++) {
    start = 0;
    end = 0;
    trigger_point2_type *abf34_hdr_trigger;
    trigger_index = i;

    if (i == MAX_SETS_FOR_TONE_NOISE_ADJ - 1) {
      /* falls within region 6 but we do not use trigger points in the region */
      ratio = 0;
      break;
    }

    if (abf3_hdr_adj_data->control_hdr_abf3 ==
      CONTROL_AEC_EXP_SENSITIVITY_RATIO) {
      abf34_hdr_trigger = &(abf3_hdr_adj_data->hdr_abf3_adj_core_data[i]
        .aec_sensitivity_ratio);
      start = abf34_hdr_trigger->start;
      end = abf34_hdr_trigger->end;
    } else {
      abf34_hdr_trigger = &(abf3_hdr_adj_data->hdr_abf3_adj_core_data[i]
        .exp_time_trigger);
      start = abf34_hdr_trigger->start;
      end   = abf34_hdr_trigger->end;
      ISP_HIGH("gain_start :%f", start);
      ISP_HIGH("gain_end :%f", end);
    }

    /* index is within interpolation range, find ratio */
    if (abf->hdr_trigger_ref >= start && abf->hdr_trigger_ref < end) {
      ratio = (abf->hdr_trigger_ref - start)/(end - start);
    }

    /* already scanned past the lux index */
    if (abf->hdr_trigger_ref < end) {
          break;
    }
  }

  if (trigger_index >= MAX_SETS_FOR_TONE_NOISE_ADJ) {
    ISP_ERR("no trigger match for ABF trig value: %f\n", abf->aec_reference);
    return FALSE;
  }

  ISP_DBG("%s hdr ratio [%f - %f - %f] = %f trigger_index %d", __func__, start, abf->hdr_trigger_ref, end,
    ratio, trigger_index);

  abf->hdr_trigger_ratio = ratio;
  abf->hdr_trigger_index = trigger_index;

  return TRUE;
}

#if !OVERRIDE_FUNC
/** compute_abf34_denoise_adj
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf module private data
 *  @event_identity: identity of trigger update mct event
 *
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean compute_abf34_denoise_adj(isp_sub_module_t
   *isp_sub_module, void *data)
{

  boolean ret = FALSE;
  int i, j, idx;
  chromatix_ABFV34_type *chromatix_ABF34 = NULL;
  chromatix_parms_type  *chromatix_ptr = NULL;
  Chromatix_HDR_ABF3_adj_type *abf3_hdr_adj_data = NULL;
  abf48_t   *abf = NULL;
  float       noise_profile_adj[2];
  float       edge_softness_adj[2][2];    // [][0] for R/B [][1] for GR/GB
  float       filter_strength_adj[2][2];  // [][0] for R/B [][1] for GR/GB
  float ratio;
  uint8_t hdr_trigger_index_start, hdr_trigger_index_end = 0;

  if (!isp_sub_module) {
    ISP_ERR("failed:sub_module_output  %p", isp_sub_module);
    return FALSE;
  }
  abf = (abf48_t *)data;
  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("Null pointer! %p", chromatix_ptr);
    return FALSE;
  }
  chromatix_ABF34 = &chromatix_ptr->chromatix_VFE.chromatix_ABFV34_data;
  abf3_hdr_adj_data = &chromatix_ptr->chromatix_VFE.chromatix_hdr_abf3_adj_data;

  abf48_calcualte_hdr_ratio(isp_sub_module, abf);
  ratio = (1 - abf->hdr_trigger_ratio);
  hdr_trigger_index_start = abf->hdr_trigger_index;
  if (ratio == 1)
    hdr_trigger_index_end = abf->hdr_trigger_index;
  else if (abf->hdr_trigger_index < MAX_SETS_FOR_TONE_NOISE_ADJ - 1)
    hdr_trigger_index_end = abf->hdr_trigger_index + 1;

  for (i = 0 ; i < 2 ; i++ ) {
    noise_profile_adj[i] = LINEAR_INTERPOLATION
      (abf3_hdr_adj_data->hdr_abf3_adj_core_data[hdr_trigger_index_start]
      .noise_profile_adj[i],
      abf3_hdr_adj_data->hdr_abf3_adj_core_data[hdr_trigger_index_end]
      .noise_profile_adj[i],ratio);
    for(j = 0 ; j <2 ;j++) {
      edge_softness_adj[i][j] = LINEAR_INTERPOLATION
        (abf3_hdr_adj_data->hdr_abf3_adj_core_data[hdr_trigger_index_start]
        .edge_softness_adj[i][j],
        abf3_hdr_adj_data->hdr_abf3_adj_core_data[hdr_trigger_index_end]
        .edge_softness_adj[i][j], ratio);
      filter_strength_adj[i][j] = LINEAR_INTERPOLATION
        (abf3_hdr_adj_data->hdr_abf3_adj_core_data[hdr_trigger_index_start]
        .filter_strength_adj[i][j],
        abf3_hdr_adj_data->hdr_abf3_adj_core_data[hdr_trigger_index_end]
        .filter_strength_adj[i][j], ratio);
    }
  }
  for ( i = 0 ; i < 1 ; i++ ) { // Levels , 8998 1 level
    abf->edge_softness  *= edge_softness_adj[i][0];

    abf->filter_strength_lo[0]  *= filter_strength_adj[i][0];
    abf->filter_strength_hi[0]  *= filter_strength_adj[i][0];

    abf->filter_strength_lo[1]  *= filter_strength_adj[i][1];
    abf->filter_strength_hi[1]  *= filter_strength_adj[i][1];

    for ( idx = 0 ; idx < ABFV34_NOISE_STD_LENGTH ; idx++ )
      abf->noise_std_lut_level[idx] =
        (abf->noise_std_lut_level[idx] *noise_profile_adj[i]);
  }

  return TRUE;

}
#endif
/** abf48_cfg_mesh_table
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf module private data
 *  @event_identity: identity of trigger update mct event
 *
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean abf48_cfg_table(isp_sub_module_t *isp_sub_module,
  abf48_t *abf)
{
  int i = 0;
  chromatix_abf34_reserve_type   *abf34_reserve = NULL;
  chromatix_parms_type     *chromatix_ptr = NULL;
  chromatix_ABFV34_type    *chromatix_ABF34 = NULL;
  chromatix_abf34_core_type  *abf34_core_start = NULL;
  chromatix_abf34_core_type  *abf34_core_end = NULL;
  Chromatix_HDR_ABF3_adj_type *abf3_hdr_adj_data = NULL;
  float ratio;

  if (!isp_sub_module || !abf) {
    ISP_ERR("failed: %p %p", isp_sub_module, abf);
    return FALSE;
  }

  ratio = (1 - abf->aec_ratio.ratio);

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("Null pointer! %p", chromatix_ptr);
    return FALSE;
  }
  chromatix_ABF34 = &chromatix_ptr->chromatix_VFE.chromatix_ABFV34_data;
  abf3_hdr_adj_data = &chromatix_ptr->chromatix_VFE
    .chromatix_hdr_abf3_adj_data;

  if ((abf3_hdr_adj_data->enable) && (abf->hdr_enable)) {
    if (abf->ext_func_table && abf->ext_func_table->denoise_adj) {
      abf->ext_func_table->denoise_adj(isp_sub_module, abf);
    }
  }

  return TRUE;
}

/** abf48_fetch_blkLvl_offset:
 *
 *  @module: mct module
 *  @identity: stream identity
 *  @gain:
 *  Fetch Black Level offset
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean abf48_fetch_blkLvl_offset(mct_module_t *module,
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


/** abf48_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: module event data
 *
 *  Perform trigger update if trigger_update_pending flag is
 *  TRUE and append hw update list in global list
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean abf48_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                                    ret = TRUE;
  abf48_t                                   *abf = NULL;
  isp_private_event_t                       *private_event = NULL;
  isp_sub_module_output_t                   *sub_module_output = NULL;
  isp_meta_entry_t                          *abf_dmi_info      = NULL;
  chromatix_parms_type  *chromatix_ptr = NULL;
  uint32_t              BlkLvl_offset = 0;
  uint8_t                 module_enable;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }
  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  abf = (abf48_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }
  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  sub_module_output = (isp_sub_module_output_t *)private_event->data;
  if (!sub_module_output) {
    ISP_ERR("failed:sub_module_output  %p", sub_module_output);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  if (isp_sub_module->manual_ctrls.manual_update) {
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
    }
  }

  abf48_fetch_blkLvl_offset(module, event->identity,
    &BlkLvl_offset);
  if (abf->BLSS_offset != BlkLvl_offset) {
    abf->BLSS_offset = BlkLvl_offset;
    isp_sub_module->submod_trigger_enable = TRUE;
  }

  if (((isp_sub_module->submod_trigger_enable == TRUE) &&
    (isp_sub_module->trigger_update_pending == TRUE)) ||
	(abf->apply_hdr_effects)){
    ret = abf48_fetch_tables(isp_sub_module, abf);
    ISP_DBG("aec_ratio.ratio = %f",abf->aec_ratio.ratio);
    abf48_config_trigger(isp_sub_module, abf);
    abf48_cfg_table(isp_sub_module, abf);
    abf48_prepare_table(isp_sub_module, abf);
    /* do hw update*/
    abf48_store_hw_update(isp_sub_module, abf);
  }

  abf48_debug(&abf->v34RegCmd);

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return ret;
  }

  if (sub_module_output->metadata_dump_enable == 1) {
    /*fill in DMI info*/
    abf_dmi_info = &sub_module_output->
    meta_dump_params->meta_entry[ISP_META_ABF_TBL];

    abf_dmi_info->len = sizeof(noise_std2_lut_t) - sizeof(uint32_t);
    /*dmi type */
    abf_dmi_info->dump_type  = ISP_META_ABF_TBL;
    abf_dmi_info->start_addr = 0;
    sub_module_output->meta_dump_params->frame_meta.num_entry++;
    memcpy(abf_dmi_info->isp_meta_dump,
      &abf->abf34_parms.noise_std_lut_level, abf_dmi_info->len);
  }

FILL_METADATA:
  if (sub_module_output->frame_meta) {
    sub_module_output->enableLPM = !(abf->filterEn);
    sub_module_output->frame_meta->abf_mode =
      isp_sub_module->manual_ctrls.abf_mode;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

}


uint32_t abf48_util_decide_hysterisis(isp_sub_module_t *isp_sub_module,
  float aec_ref, uint32_t prev_value,  int mod_enable,
  tuning_control_type control_type,
  int hyst_en_flag,
  trigger_point_type *hyster_trigger)
{
  float     hyster_start   = 0.0;
  float     hyster_end     = 0.0;
  boolean   hyst_en        = TRUE;
  uint32_t  enable         = prev_value;


  if (isp_sub_module->stream_data.is_snapshot_stream_on ||
     (isp_sub_module->stream_data.is_cb_stream_on &&
      isp_sub_module->zsl_mode)) {
    if (hyst_en_flag == 0) {
        enable = mod_enable;
        hyst_en = FALSE;
    }
  }

  if (hyst_en){
    if (control_type == 0) {
      /* lux index based */
      hyster_start = hyster_trigger->lux_index_start;
      hyster_end   = hyster_trigger->lux_index_end;
    } else {
      /* Gain based */
      hyster_start = hyster_trigger->gain_start;
      hyster_end   = hyster_trigger->gain_end;
    }
    if (aec_ref > hyster_end) {
      enable = mod_enable;
    } else if (aec_ref < hyster_start) {
      enable = 0;
    }
  }

  ISP_DBG("enable %d, aec_ref %f, hyster start %f end %f",
      enable, aec_ref, hyster_start, hyster_end);

   return enable;

}


/** abf48_decide_hysterisis:
 *
 *  @module:  module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean abf48_decide_hysterisis(isp_sub_module_t *isp_sub_module)
{
  chromatix_parms_type *chromatix_ptr  = NULL;
  chromatix_ABFV34_type  *chromatix_ABF34 = NULL;
  abf48_t               *abf           = NULL;
  float                 hyster_start   = 0.0;
  float                 hyster_end     = 0.0;
  boolean               hyst_en        = TRUE;
  int                   enable;

  if (!isp_sub_module) {
    ISP_ERR("failed: %p ", isp_sub_module);
    return FALSE;
  }

  abf = (abf48_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    return FALSE;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_ABF34 =
    &chromatix_ptr->chromatix_VFE.chromatix_ABFV34_data;

  abf->filterEn = abf48_util_decide_hysterisis(isp_sub_module,
    abf->aec_reference,
    abf->filterEn,
    chromatix_ABF34->abf34_enable,
    chromatix_ABF34->control_abf3,
    chromatix_ABF34->abf34_snapshot_hyst_en_flag,
    &chromatix_ABF34->abf3_hysteresis_pt);

  abf->singleBpcEn = abf48_util_decide_hysterisis(isp_sub_module,
    abf->aec_reference,
    abf->singleBpcEn,
    (chromatix_ABF34->abf34_enable & chromatix_ABF34->single_bpc_en),
    chromatix_ABF34->control_abf3,
    chromatix_ABF34->sbpc_snapshot_hyst_en_flag,
    &chromatix_ABF34->sbpc_hysteresis_pt);

  abf->abf34_enable = abf->filterEn | abf->singleBpcEn;

  switch (abf->abf34_enable) {
    case 1: {
      if (isp_sub_module->submod_enable != abf->abf34_enable) {
        isp_sub_module->submod_enable = abf->abf34_enable;
        isp_sub_module->update_module_bit = TRUE;
      }
    }
    break;
    case 0: {
      if (isp_sub_module->submod_enable == TRUE) {
        isp_sub_module->submod_enable = FALSE;
        isp_sub_module->update_module_bit = TRUE;
      }
    }
    break;
    default: {
      ISP_HIGH("Maintain as before");
    }
    break;
  }

  ISP_DBG(" Hys decision ABF module %d", isp_sub_module->submod_enable);

  return TRUE;

}



/** abf48_aec_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: module event data
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean abf48_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  stats_update_t              *stats_update = NULL;
  aec_manual_update_t         *stats_manual_update  = NULL;
  abf48_t                     *abf = NULL;
  trigger_ratio_t             aec_ratio;
  chromatix_parms_type        *chromatix_ptr = NULL;
  chromatix_ABFV34_type       *chromatix_ABF34 = NULL;
  Chromatix_HDR_ABF3_adj_type *abf3_hdr_adj_data = NULL;
  uint8_t trigger_index = MAX_LIGHT_TYPES_FOR_SPATIAL_ABF + 1;
  float   aec_reference = 0.0f;
  float   ratio         = 0;
  float   start         = 0;
  float   end           = 0;
  boolean ret           = TRUE;
  int i                 = 0;
  uint32_t  filterEn;
  uint32_t  singleBpcEn;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  abf = (abf48_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    return FALSE;
  }

  if (event->u.module_event.type ==
    MCT_EVENT_MODULE_STATS_AEC_MANUAL_UPDATE) {
    stats_manual_update = (aec_manual_update_t *)event->u.module_event.module_event_data;
  } else if (event->u.module_event.type ==
    MCT_EVENT_MODULE_STATS_AEC_UPDATE) {
    stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  }

  if (!stats_update && !stats_manual_update) {
    ISP_ERR("failed: stats_update %p manual_update %p ",
      stats_update, stats_manual_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    goto ERROR;
  }



  chromatix_ABF34 = &chromatix_ptr->chromatix_VFE.chromatix_ABFV34_data;
  abf3_hdr_adj_data =
    &chromatix_ptr->chromatix_VFE.chromatix_hdr_abf3_adj_data;

  if (stats_update){
    abf->aec_update = stats_update->aec_update;
    if (chromatix_ABF34->control_abf3 == 0) {
      /* lux index based */
      aec_reference = stats_update->aec_update.lux_idx;
    } else {
      /* Gain based */
      aec_reference = stats_update->aec_update.sensor_gain;
    }

    abf->aec_lux_reference = stats_update->aec_update.lux_idx;
    abf->aec_gain_reference = stats_update->aec_update.sensor_gain;
    abf->hdr_sensitivity_ratio = stats_update->aec_update.hdr_sensitivity_ratio;
    abf->hdr_exp_ratio = stats_update->aec_update.hdr_exp_time_ratio;

    if (abf3_hdr_adj_data->control_hdr_abf3 ==
      CONTROL_AEC_EXP_SENSITIVITY_RATIO) {
        /* EXP Sensitivity based */
      abf->hdr_trigger_ref = stats_update->aec_update.hdr_sensitivity_ratio;
    } else {
      /* Exp time ratio based */
      abf->hdr_trigger_ref = stats_update->aec_update.hdr_exp_time_ratio;
    }
  } else if (stats_manual_update){
    abf->aec_manual_update = *stats_manual_update;
    if (chromatix_ABF34->control_abf3 == 0) {
      /* lux index based */
      aec_reference = stats_manual_update->lux_idx;
    } else {
     /* Gain based */
      aec_reference = stats_manual_update->sensor_gain;
    }
    abf->aec_lux_reference = stats_manual_update->lux_idx;
    abf->aec_gain_reference = stats_manual_update->sensor_gain;
  }

  ISP_DBG(" aec_reference :%f", aec_reference);
  ISP_DBG(" hdr_reference :%f", abf->hdr_trigger_ref);

  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL_ABF; i++) {
    start = 0;
    end = 0;
    trigger_point_type *abf34_trigger =
      &(chromatix_ABF34->abf34_config[i].ABF34_trigger);
    trigger_index = i;

    if (i == MAX_LIGHT_TYPES_FOR_SPATIAL_ABF - 1) {
      /* falls within region 6 but we do not use trigger points in the region */
      ratio = 0;
      break;
    }

    if (chromatix_ABF34->control_abf3 == 0) {
      start = abf34_trigger->lux_index_start;
      end   = abf34_trigger->lux_index_end;
    } else {
      start = abf34_trigger->gain_start;
      end   = abf34_trigger->gain_end;
    ISP_HIGH("gain_start :%f", start);
    ISP_HIGH("gain_end :%f", end);
    }

    /* index is within interpolation range, find ratio */
     if (aec_reference >= start && aec_reference < end) {
       ratio = (aec_reference - start)/(end - start);
     }

     /* already scanned past the lux index */
     if (aec_reference < end) {
           break;
     }
  }

  if (trigger_index >= MAX_LIGHT_TYPES_FOR_SPATIAL_ABF) {
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
    if (trigger_index >= MAX_LIGHT_TYPES_FOR_SPATIAL_ABF + NUM_SCENE_MODES) {
      ISP_ERR("no trigger match for ABF trigger value: %f\n", aec_reference);
      goto ERROR;
    }
  }

  abf->aec_reference = aec_reference;

  filterEn = abf->filterEn;
  singleBpcEn = abf->singleBpcEn;
  abf48_decide_hysterisis(isp_sub_module);

  ISP_DBG("%s aec ratio [%f - %f - %f] = %f trigger index %u", __func__,
    start, aec_reference, end, ratio, trigger_index);

  if ((trigger_index != abf->trigger_index) ||
    (filterEn != abf->filterEn) ||
    (singleBpcEn != abf->singleBpcEn) ||
    (!F_EQUAL(ratio, abf->aec_ratio.ratio))) {
    abf->trigger_index = trigger_index;
    abf->aec_ratio.ratio = ratio;
    isp_sub_module->trigger_update_pending = TRUE;
    if (stats_update)
      abf->aec_update = stats_update->aec_update;
    else if (stats_manual_update)
      abf->aec_manual_update = *stats_manual_update;
  }


  if (abf->ext_func_table && abf->ext_func_table->get_2d_interpolation) {
    ret = abf->ext_func_table->get_2d_interpolation(isp_sub_module, abf);
    if (ret == FALSE) {
      ISP_ERR("failed: get_2d_interpolation");
      goto ERROR;
    }
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}


/** abf48_set_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: module event data
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean abf48_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean              ret = TRUE;
  abf48_t              *abf = NULL;
  modulesChromatix_t   *chromatix_ptrs = NULL;
  chromatix_parms_type *chromatix_ptr = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  abf = (abf48_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: mod %p", abf);
    return FALSE;
  }

  chromatix_ptrs =
    (modulesChromatix_t *)event->u.module_event.module_event_data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    return FALSE;
  }

  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("Null pointer! %p", chromatix_ptr);
    return FALSE;
  }

  if ((chromatix_ptr->chromatix_VFE.chromatix_ABFV34_data.abf34_enable > 0)||
    (chromatix_ptr->chromatix_VFE.chromatix_ABFV34_data.single_bpc_en > 0)) {
    isp_sub_module->submod_enable = TRUE;
  }

  abf->singleBpcEn = chromatix_ptr->chromatix_VFE.
                       chromatix_ABFV34_data.single_bpc_en &
                       chromatix_ptr->chromatix_VFE.chromatix_ABFV34_data.
                       abf34_enable;
  abf->filterEn = chromatix_ptr->chromatix_VFE.chromatix_ABFV34_data.
                       abf34_enable;

  ret = abf48_init_default(isp_sub_module, abf);
  if (ret == FALSE) {
    ISP_ERR("failed: abf48_init_default ret %d", ret);
  } else {
    isp_sub_module->trigger_update_pending = TRUE;
  }

  return ret;
} /* abf_set_chromatix_ptr */

/** abf48_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: module event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean abf48_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  abf48_t *abf = NULL;
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }

  abf = (abf48_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    return FALSE;
  }

  if (--isp_sub_module->stream_on_count) {
    return TRUE;
  }
  memset(abf, 0, sizeof(*abf));
  isp_sub_module->manual_ctrls.abf_mode = CAM_NOISE_REDUCTION_MODE_FAST;
  FILL_FUNC_TABLE(abf);

  return TRUE;
} /* abf48_streamoff */

/** abf48_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the abf module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean abf48_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  abf48_t *abf = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  abf = (abf48_t *)CAM_MALLOC(sizeof(abf48_t));
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    return FALSE;
  }

  memset(abf, 0, sizeof(*abf));
  isp_sub_module->private_data = (void *)abf;
  FILL_FUNC_TABLE(abf);
  isp_sub_module->manual_ctrls.abf_mode = CAM_NOISE_REDUCTION_MODE_FAST;

  return TRUE;
}/* abf48_init */

/** abf48_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void abf48_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* abf48_destroy */

#if !OVERRIDE_FUNC

void abf48_apply_2d_interpolation(void *data) {

  abf48_t   *abf;
  abf = (abf48_t*)data;
  float t1_es_ratio, noise_adj;
  int j;
  t1_es_ratio = abf->abf48_ext_params.abf_noise_profile_adjust_ratio;
  t1_es_ratio = Clamp(t1_es_ratio, 0.0, 16.0);
  float t1 = abf->abf48_ext_params.abf_HDR_thr_1;
  float t2 = abf->abf48_ext_params.abf_HDR_thr_2;
  float noise_offset = abf->abf48_ext_params.abf_noise_offset;

  for (j=0; j<(ABFV34_NOISESTD_LENGTH-1); j++) {

      if (j<t1) // t1 noise profile
          noise_adj = t1_es_ratio;
      else if  (j<t2) // transition t1->t2 noise profile
          noise_adj = t1_es_ratio + (t1 - j)/(t1 - t2)*(1.0f - t1_es_ratio);
      else
          noise_adj = 1.0f;

      abf->noise_std_lut_level[j] *= noise_adj;
      abf->noise_std_lut_level[j] += noise_offset;
  }
  abf->bpc_fmax *= abf->abf48_ext_params.abf_Fmax_adj;
  abf->v34RegCmd.cfg_cmd2.bpcCfg.fmax =
    Clamp(Round(abf->bpc_fmax), ABF_SBPC_FMAX_MIN, ABF_SBPC_FMAX_MAX);
  abf->bpc_fmin *= abf->abf48_ext_params.abf_Fmin_adj;
  abf->v34RegCmd.cfg_cmd2.bpcCfg.fmin =
    Clamp(Round(abf->bpc_fmin), ABF_SBPC_FMIN_MIN, ABF_SBPC_FMIN_MAX);
  abf->bpc_maxshft = abf->abf48_ext_params.bpc_maxshft;
  abf->v34RegCmd.cfg_cmd2.bpcCfg.maxShift =
    Clamp(Round(abf->bpc_maxshft), ABF_SBPC_MAXSHFT_MIN, ABF_SBPC_MAXSHFT_MAX);
  abf->bpc_minshft = abf->abf48_ext_params.bpc_minshft;
  abf->v34RegCmd.cfg_cmd2.bpcCfg.minShift =
    Clamp(Round(abf->bpc_minshft),ABF_SBPC_MAXSHFT_MIN, ABF_SBPC_MINSHFT_MAX);
  abf->bpc_offset *= abf->abf48_ext_params.abf_bpc_offset_adj;
  abf->v34RegCmd.cfg_cmd2.bpcCfg.offset    = CLAMP_ABF(Round(abf->bpc_offset),
    ABF_SBPC_BPC_OFF_MAX);
}

void abf48_2d_interpolate(abf48_ext_t *local_params,
  abf48_ext_t *local_params1, abf48_ext_t *local_params2, float ratio) {

  local_params->abf_noise_profile_adjust_ratio = (LINEAR_INTERPOLATION(
    local_params1->abf_noise_profile_adjust_ratio,
    local_params2->abf_noise_profile_adjust_ratio,
    ratio));

  local_params->abf_noise_offset = (LINEAR_INTERPOLATION((float)
    local_params1->abf_noise_offset, local_params2->abf_noise_offset,
    ratio));

  local_params->abf_bpc_offset_adj = (LINEAR_INTERPOLATION((float)
    local_params1->abf_bpc_offset_adj, local_params2->abf_bpc_offset_adj,
    ratio));

  local_params->abf_Fmax_adj = (LINEAR_INTERPOLATION((float)
    local_params1->abf_Fmax_adj, local_params2->abf_Fmax_adj,
    ratio));

  local_params->abf_Fmin_adj = (LINEAR_INTERPOLATION((float)
    local_params1->abf_Fmin_adj, local_params2->abf_Fmin_adj,
    ratio));

  local_params->bpc_maxshft = (LINEAR_INTERPOLATION((float)
    local_params1->bpc_maxshft, local_params2->bpc_maxshft,
    ratio));

  local_params->bpc_minshft = (LINEAR_INTERPOLATION((float)
    local_params1->bpc_minshft, local_params2->bpc_minshft,
    ratio));

  local_params->abf_HDR_thr_1 = (LINEAR_INTERPOLATION((float)
    local_params1->abf_HDR_thr_1, local_params2->abf_HDR_thr_1,
    ratio));

  local_params->abf_HDR_thr_2 = (LINEAR_INTERPOLATION((float)
    local_params1->abf_HDR_thr_2, local_params2->abf_HDR_thr_2,
    ratio));

  }

void abf_interpolate_region_parameters(
  abf_gic_demosaic_2d_exp_type *abf_gic_demosaic_exp_data,
  uint32_t trigger_idx_start, uint32_t trigger_idx_end,
  abf48_ext_t *local_params, float ratio) {

    local_params->abf_noise_profile_adjust_ratio = (LINEAR_INTERPOLATION(
      (float)abf_gic_demosaic_exp_data[trigger_idx_start].
      abf_gic_demosaic_2d_reserve_data.
      abf_noise_profile_adjust_ratio,
      (float)abf_gic_demosaic_exp_data[trigger_idx_end].
      abf_gic_demosaic_2d_reserve_data.
      abf_noise_profile_adjust_ratio,
      ratio));

    local_params->abf_HDR_thr_1 = (LINEAR_INTERPOLATION((float)
      abf_gic_demosaic_exp_data[trigger_idx_start].
      abf_gic_demosaic_2d_reserve_data.
      abf_HDR_thr_1,
      (float)abf_gic_demosaic_exp_data[trigger_idx_end].
      abf_gic_demosaic_2d_reserve_data.
      abf_HDR_thr_1,
      ratio));

    local_params->abf_HDR_thr_2 = (LINEAR_INTERPOLATION((float)
      abf_gic_demosaic_exp_data[trigger_idx_start].
      abf_gic_demosaic_2d_reserve_data.
      abf_HDR_thr_2,
      (float)abf_gic_demosaic_exp_data[trigger_idx_end].
      abf_gic_demosaic_2d_reserve_data.
      abf_HDR_thr_2,
      ratio));

    local_params->abf_noise_offset = (LINEAR_INTERPOLATION((float)
      abf_gic_demosaic_exp_data[trigger_idx_start].
      abf_gic_demosaic_2d_reserve_data.
      abf_noise_offset,
      (float)abf_gic_demosaic_exp_data[trigger_idx_end].
      abf_gic_demosaic_2d_reserve_data.
      abf_noise_offset,
      ratio));

    local_params->abf_bpc_offset_adj = (LINEAR_INTERPOLATION((float)
      abf_gic_demosaic_exp_data[trigger_idx_start].
      abf_gic_demosaic_2d_reserve_data.
      abf_bpc_offset_adj,
      (float)abf_gic_demosaic_exp_data[trigger_idx_end].
      abf_gic_demosaic_2d_reserve_data.
      abf_bpc_offset_adj,
      ratio));

    local_params->abf_Fmax_adj = (LINEAR_INTERPOLATION((float)
      abf_gic_demosaic_exp_data[trigger_idx_start].
      abf_gic_demosaic_2d_reserve_data.
      abf_Fmax_adj,
      (float)abf_gic_demosaic_exp_data[trigger_idx_end].
      abf_gic_demosaic_2d_reserve_data.
      abf_Fmax_adj,
      ratio));

    local_params->abf_Fmin_adj = (LINEAR_INTERPOLATION((float)
      abf_gic_demosaic_exp_data[trigger_idx_start].
      abf_gic_demosaic_2d_reserve_data.abf_Fmin_adj,
      (float)abf_gic_demosaic_exp_data[trigger_idx_end].
      abf_gic_demosaic_2d_reserve_data.abf_Fmin_adj,
      ratio));

    local_params->bpc_maxshft = (LINEAR_INTERPOLATION((float)
      abf_gic_demosaic_exp_data[trigger_idx_start].
      abf_gic_demosaic_2d_reserve_data.
      bpc_maxshft,
      (float)abf_gic_demosaic_exp_data[trigger_idx_end].
      abf_gic_demosaic_2d_reserve_data.
      bpc_maxshft,
      ratio));

    local_params->bpc_minshft = (LINEAR_INTERPOLATION((float)
      abf_gic_demosaic_exp_data[trigger_idx_start].
      abf_gic_demosaic_2d_reserve_data.
      bpc_minshft,
      (float)abf_gic_demosaic_exp_data[trigger_idx_end].
      abf_gic_demosaic_2d_reserve_data.
      bpc_minshft,
      ratio));

  }

boolean abf_interpolate_parameters(
  abf_gic_demosaic_2d_adj_type *chromatix_2d,  abf48_t   *abf,
  abf_gic_demosaic_2d_exp_type *abf_gic_demosaic_exp_data,
  abf48_ext_t *local_params) {

  abf_gic_demosaic_2d_exp_type  *abf_exp_data = NULL;
  trigger_point2_type      *abf_trigger       = NULL;
  float                    start             = 0.0;
  float                    end               = 0.0;
  float                    hdr_trigger_ref     = 0.0;
  uint32_t                 i                 = 0;
  uint32_t             trigger_idx_start = 0, trigger_idx_end = 0;
  float                ratio = 0.0f;
  boolean ret = TRUE;

  for (i = 0; i < MAX_SETS_FOR_TONE_NOISE_ADJ; i++) {
    if (i == MAX_SETS_FOR_TONE_NOISE_ADJ - 1) {
      /* falls within last region but we do not have any more
         adjacent regions */
      ratio         = 0.0f;
      trigger_idx_start = MAX_SETS_FOR_TONE_NOISE_ADJ - 1;
      trigger_idx_end = MAX_SETS_FOR_TONE_NOISE_ADJ - 1;
      break;
    }

    switch (chromatix_2d->hdr_exp_control) {
    case  CONTROL_AEC_EXP_SENSITIVITY_RATIO:
        /* aec senstivity ratio  based */
      hdr_trigger_ref = abf->hdr_sensitivity_ratio;
      abf_trigger   = &(abf_gic_demosaic_exp_data[i].aec_sensitivity_ratio);
      start         = abf_trigger->start;
      end           = abf_trigger->end;
      break;
    case  CONTROL_EXP_TIME_RATIO:
      /* exp time based */
      hdr_trigger_ref = abf->hdr_exp_ratio;
      abf_trigger   = &(abf_gic_demosaic_exp_data[i].exp_time_trigger);
      start         = abf_trigger->start;
      end           = abf_trigger->end;
    break;
    default:
      ISP_ERR("Incorrect tuning control type %d", chromatix_2d->exp_control);
      ret = FALSE;
      goto error;
    }
    /* index is within interpolation range, find ratio */
    if (((hdr_trigger_ref > start) || (F_EQUAL(hdr_trigger_ref, start)))
      && hdr_trigger_ref < end) {
      ratio = (end - hdr_trigger_ref)/(end - start);
      trigger_idx_start = i;
      trigger_idx_end = i+1;
      break;
    }
    if (hdr_trigger_ref < start) {
      trigger_idx_start = i;
      trigger_idx_end = i;
      ratio = 0.0f;
      break;
    }
  }
  ISP_DBG("%s [%f - %f - %f] = %f, t_idx1=%d t_idx2=%d", __func__,
    start, hdr_trigger_ref, end,
    ratio, trigger_idx_start, trigger_idx_end);

  abf_interpolate_region_parameters(abf_gic_demosaic_exp_data,
    trigger_idx_start, trigger_idx_end, local_params, ratio);
error:
  return ret;
}


boolean abf48_process_regions(abf_gic_demosaic_2d_adj_type
  *chromatix_2d, abf48_t   *abf, uint32_t *trigger_idx_start,
  uint32_t *trigger_idx_end,
  float *ratio, tuning_control_type control_type) {
  boolean                  ret               = TRUE;
  trigger_point_type      *abf_trigger       = NULL;
  float                    start             = 0.0;
  float                    end               = 0.0;
  float                    aec_reference     = 0.0;
  uint32_t                 i                 = 0;


  if (!chromatix_2d || !abf) {
    ISP_ERR("failed: %p %p", chromatix_2d, abf);
    return FALSE;
  }

  for (i = 0; i < MAX_SETS_FOR_2D_EXP_ADJ; i++) {
    if (i == MAX_SETS_FOR_2D_EXP_ADJ - 1) {
      /* falls within last region but we do not have any more
         adjacent regions */
      *ratio         = 0.0f;
      *trigger_idx_start = MAX_SETS_FOR_2D_EXP_ADJ - 1;
      *trigger_idx_end = MAX_SETS_FOR_2D_EXP_ADJ - 1;
      break;
    }

    switch (control_type) {
    case CONTROL_LUX_IDX:
        /* lux index based */
      aec_reference = abf->aec_lux_reference;
      abf_trigger   = &(chromatix_2d->abf_gic_demosaic_rgn_data[i].trigger);
      start         = abf_trigger->lux_index_start;
      end           = abf_trigger->lux_index_end;
      ISP_ERR("lux base, lux idx %f start:%f end:%f",aec_reference, start, end);
      break;
    case CONTROL_GAIN:
      /* Gain based */
      aec_reference = abf->aec_gain_reference;
      abf_trigger   = &(chromatix_2d->abf_gic_demosaic_rgn_data[i].trigger);
      start         = abf_trigger->gain_start;
      end           = abf_trigger->gain_end;
    break;
    default:
      ISP_ERR("Incorrect tuning control type %d", control_type);
      ret = FALSE;
      goto error;
    }
    /* index is within interpolation range, find ratio */
    if (((aec_reference > start) || (F_EQUAL(aec_reference, start)))
      && aec_reference < end) {
      *ratio = (end - aec_reference)/(end - start);
      *trigger_idx_start = i;
      *trigger_idx_end = i+1;
      break;
    }
    if (aec_reference < start) {
      *trigger_idx_start = i;
      *trigger_idx_end = i;
      *ratio = 0.0f;
      break;
    }
  }
  ISP_DBG("%s [%f - %f - %f] = %f, t_idx1=%d t_idx2=%d", __func__, start,
    aec_reference, end,     *ratio, *trigger_idx_start, *trigger_idx_end);

error:
  return ret;
}

/** abf48_ext_2d_interpolation:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Ext changes
 *
 *  Return none
 **/
boolean abf48_2d_interpolation(void *data1,
  void *data2 )
{
  isp_sub_module_t *isp_sub_module;
  abf48_t   *abf = NULL;
  boolean   ret  = TRUE;
  chromatix_parms_type  *chromatix_ptr = NULL;
  chromatix_ABFV34_type *chromatix_ABF34 = NULL;
  abf_gic_demosaic_2d_adj_type *abf_2d_adj_type =  NULL;
  uint32_t             trigger_idx_start = 0, trigger_idx_end = 0;
  float     ratio = 0.0f;
  abf48_ext_t  abf48_ext_params_1, abf48_ext_params_2;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }
  memset(&abf48_ext_params_1, 0, sizeof(abf48_ext_t));
  memset(&abf48_ext_params_1, 0, sizeof(abf48_ext_t));
  isp_sub_module = (isp_sub_module_t *)data1;
  abf = (abf48_t *)data2;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("Null pointer! %p", chromatix_ptr);
    return FALSE;
  }
  chromatix_ABF34 = &chromatix_ptr->chromatix_VFE.chromatix_ABFV34_data;
  abf_2d_adj_type = &zzhdr_abf_tuning_data.abf_2d_data;

  if (abf_2d_adj_type->abf_adj_enable &&
    (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
    isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG)) {
    abf48_process_regions(abf_2d_adj_type, abf, &trigger_idx_start,
      &trigger_idx_end, &ratio, abf_2d_adj_type->exp_control);

    if (trigger_idx_start != trigger_idx_end) {
      /* Compute the params based on exp_time_trigger/ aec_sensitivity_ratio */
      abf_interpolate_parameters(abf_2d_adj_type, abf,
        abf_2d_adj_type->abf_gic_demosaic_rgn_data[trigger_idx_start].
        abf_gic_demosaic_exp_data, &abf48_ext_params_1);
      abf_interpolate_parameters(abf_2d_adj_type, abf,
        abf_2d_adj_type->abf_gic_demosaic_rgn_data[trigger_idx_end].
        abf_gic_demosaic_exp_data, &abf48_ext_params_2);

      abf48_2d_interpolate(&abf->abf48_ext_params, &abf48_ext_params_1, &abf48_ext_params_2,
         ratio);
    } else {
      abf_interpolate_parameters(abf_2d_adj_type, abf,
        abf_2d_adj_type->abf_gic_demosaic_rgn_data[trigger_idx_start].
        abf_gic_demosaic_exp_data, &abf->abf48_ext_params);
    }
    abf->apply_hdr_effects = 1;
    //Assign local_params to abf parameters. One more function or do a staright copy
  }
  return TRUE;
}
static ext_override_func abf_override_func = {
  .get_trigger_index = NULL,
  .get_2d_interpolation = abf48_2d_interpolation,
  .apply_2d_interpolation = abf48_apply_2d_interpolation,
  .denoise_adj = compute_abf34_denoise_adj,
};

boolean abf48_fill_func_table(abf48_t *abf)
{
  abf->ext_func_table = &abf_override_func;
  return TRUE;
} /* abf48_fill_func_table */
#endif
