/* abf47.c
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
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
#include "abf47.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_ABF, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_ABF, fmt, ##args)

#define CLAMP_ABF(x1,v1) ((x1 >= v1) ? v1 : x1 )

//#define DUMP_ABF
/* sig2 luts pack delta/2 instead of delta */
#define FILL_SIG2_LUT(dst, src, len) \
  { \
    int32_t base; \
    int32_t delta; \
    int32_t next;\
    for (i = 0; i < (len - 1); i++) { \
      base = CLAMP_ABF(((src)[i]),65535); \
      next = CLAMP_ABF(((src)[i+1]),65535);\
      delta = Clamp((next - base),-32768,32767);\
      delta = roundf(delta / 2.0f); \
      (dst)[i] = ((delta ) << 16) |(base ); \
    } \
    base = ((src)[i]); \
    base = CLAMP_ABF(base, 65535);\
    next = (src)[i+1] ;\
    delta = Clamp((next - base),-32768,32767);\
    delta = roundf(delta / 2.0f); \
    (dst)[i] = ((delta ) << 16) | (base); \
  }

#define FILL_LUT_INTERP(dst, src1, src2, len, ratio, adj) \
  { \
    int32_t base; \
    int32_t next; \
    int32_t delta; \
    for (i = 0; i < len - 1; i++) { \
      base = roundf((((src1)[i] * (1.0f - ratio)) + ((src2)[i] * ratio))* adj); \
      next = roundf((((src1)[i+1]) * (1.0f - ratio) + ((src2)[i+1] * ratio)) * adj); \
      base = CLAMP_ABF(base,65535);\
      next = CLAMP_ABF(next,65535);\
      delta = Clamp((next - base),-32768,32767);\
      (dst)[i] = (delta << 16) | base; \
    } \
    base = roundf(((src1)[i] * adj) * (1.0f - ratio) + ((src2)[i] * adj) * (ratio)); \
    next = roundf(((src1)[i+1] * adj) * (1.0f - ratio) + ((src2)[i+1] * adj) * (ratio)); \
    base = CLAMP_ABF(base,65535);\
    delta = Clamp((next - base),-32768,32767);\
    (dst)[i] = (delta << 16) | base; \
  }

#define FILL_LUT(dst, src, len, adj) \
  { \
    int32_t base; \
    int32_t next; \
    int32_t delta; \
    for (i = 0; i < len; i++) { \
      base = CLAMP_ABF(roundf((src)[i] * adj),65535);\
      next = CLAMP_ABF(roundf((src)[i+1] * adj),65535);\
      delta = Clamp((next - base),-32768,32767);\
      (dst)[i] = (delta << 16) | base; \
    } \
  }

#define PRINT_LUT(dst, len) \
     {\
    for (i = 0; i < (len); i++) { \
    ISP_ERR("LUT index:[%d]== %x",i,(dst)[i]);\
    } \
  }

/** abf47_channel_debug
 *  debug function for channel config
 *
 **/
static void abf47_channel_debug(ISP_ABF3_ChannelCfg *chCfg)
{
  ISP_DBG(" %s %x", "noiseScale0Lv0", chCfg->noiseScale0Lv0);
  ISP_DBG(" %s %x", "wTableAdjLv0", chCfg->wTableAdjLv0);
  ISP_DBG(" %s %x", "noiseScale0Lv1", chCfg->noiseScale0Lv1);
  ISP_DBG(" %s %x", "wTableAdjLv1", chCfg->wTableAdjLv1);
  ISP_DBG(" %s %x", "noiseScale1Lv0", chCfg->noiseScale1Lv0);
  ISP_DBG(" %s %x", "noiseScale1Lv1", chCfg->noiseScale1Lv1);
  ISP_DBG(" %s %x", "noiseScale2Lv0", chCfg->noiseScale2Lv0);
  ISP_DBG(" %s %x", "noiseScale2Lv1", chCfg->noiseScale2Lv1);
  ISP_DBG(" %s %x", "noiseOffLv0", chCfg->noiseOffLv0);
  ISP_DBG(" %s %x", "noiseOffLv1", chCfg->noiseOffLv1);
  ISP_DBG(" %s %x", "curvOffLv0", chCfg->curvOffLv0);
  ISP_DBG(" %s %x", "curvOffLv1", chCfg->curvOffLv1);
  ISP_DBG(" %s %x", "sftThrdNoiseScaleLv0", chCfg->sftThrdNoiseScaleLv0);
  ISP_DBG(" %s %x", "sftThrdNoiseShiftLv0", chCfg->sftThrdNoiseShiftLv0);
  ISP_DBG(" %s %x", "sftThrdNoiseScaleLv1", chCfg->sftThrdNoiseScaleLv1);
  ISP_DBG(" %s %x", "sftThrdNoiseShiftLv1", chCfg->sftThrdNoiseShiftLv1);
  ISP_DBG(" %s %x", "filterNoisePreserveL_Lv0",
    chCfg->filterNoisePreserveL_Lv0);
  ISP_DBG(" %s %x", "filterNoisePreserveR_Lv0",
    chCfg->filterNoisePreserveR_Lv0);
  ISP_DBG(" %s %x", "filterNoisePreserveL_Lv1",
    chCfg->filterNoisePreserveL_Lv1);
  ISP_DBG(" %s %x", "filterNoisePreserveR_Lv1",
    chCfg->filterNoisePreserveR_Lv1);
  ISP_DBG(" %s %x", "spatialScaleL_Lv0", chCfg->spatialScaleL_Lv0);
  ISP_DBG(" %s %x", "spatialScaleR_Lv0", chCfg->spatialScaleR_Lv0);
  ISP_DBG(" %s %x", "spatialScaleL_Lv1", chCfg->spatialScaleL_Lv1);
  ISP_DBG(" %s %x", "spatialScaleR_Lv1", chCfg->spatialScaleR_Lv1);
}

/** abf47_debug
 *  debug function for abf config
 *
 **/
static void abf47_debug(ISP_ABF3_CmdType *cmd)
{
  ISP_DBG("   lutBankSel      %x", cmd->cfg.lutBankSel);
  ISP_DBG("   crossPlLevel0En   :%x", cmd->cfg.crossPlLevel0En);
  ISP_DBG("   crossPlLevel1En   :%x", cmd->cfg.crossPlLevel1En);
  ISP_DBG("   minMaxEn  :%x", cmd->cfg.minMaxEn);
  ISP_DBG(" Level Config:");
  ISP_DBG("   distGrGb_0      :%x", cmd->level1.distGrGb_0);
  ISP_DBG("   distGrGb_1        :%x", cmd->level1.distGrGb_1);
  ISP_DBG("   distGrGb_2        :%x", cmd->level1.distGrGb_2);
  ISP_DBG("   distGrGb_3        :%x", cmd->level1.distGrGb_3);
  ISP_DBG("   distGrGb_4        :%x", cmd->level1.distGrGb_4);
  ISP_DBG("   minMaxSelGrGb0    :%x", cmd->level1.minMaxSelGrGb0);
  ISP_DBG("   minMaxSelGrGb1    :%x", cmd->level1.minMaxSelGrGb1);
  ISP_DBG("   distRb0            :%x", cmd->level1.distRb0);
  ISP_DBG("   distRb1            :%x", cmd->level1.distRb1);
  ISP_DBG("   distRb2            :%x", cmd->level1.distRb2);
  ISP_DBG("   distRb3            :%x", cmd->level1.distRb3);
  ISP_DBG("   distRb4            :%x", cmd->level1.distRb4);
  ISP_DBG("   minMaxSelRb0       :%x", cmd->level1.minMaxSelRb0);
  ISP_DBG("   minMaxSelRb1       :%x", cmd->level1.minMaxSelRb1);
  ISP_DBG("");
  ISP_DBG("Gr Cfg:");
  abf47_channel_debug(&cmd->grCfg);
  ISP_DBG("Gb Cfg:");
  abf47_channel_debug(&cmd->gbCfg);
  ISP_DBG("r Cfg:");
  abf47_channel_debug(&cmd->rCfg);
  ISP_DBG("b Cfg:");
  abf47_channel_debug(&cmd->bCfg);
  ISP_DBG(" Spatial Config:");

  ISP_DBG("   init_CellNum_X_L %u", cmd->spCfg.init_CellNum_X_L);
  ISP_DBG("   init_CellNum_X_R %u", cmd->spCfg.init_CellNum_X_R);
  ISP_DBG("   init_CellNum_Y_L %u", cmd->spCfg.init_CellNum_Y_L);
  ISP_DBG("   init_CellNum_Y_R %u", cmd->spCfg.init_CellNum_Y_R);
  ISP_DBG("   init_DX_L %u", cmd->spCfg.init_DX_L);
  ISP_DBG("   init_DX_R %u", cmd->spCfg.init_DX_R);
  ISP_DBG("   init_DY_L %u", cmd->spCfg.init_DY_L);
  ISP_DBG("   init_DY_R %u", cmd->spCfg.init_DY_R);
  ISP_DBG("   init_PX_L %u", cmd->spCfg.init_PX_L);
  ISP_DBG("   init_PX_R %u", cmd->spCfg.init_PX_R);
  ISP_DBG("   init_PY_L %u", cmd->spCfg.init_PY_L);
  ISP_DBG("   init_PY_R %u", cmd->spCfg.init_PY_R);
  ISP_DBG("   inv_CellWidth_L %u", cmd->spCfg.inv_CellWidth_L );
  ISP_DBG("   inv_CellWidth_R %u", cmd->spCfg.inv_CellWidth_R );
  ISP_DBG("   inv_CellHeight_L %u", cmd->spCfg.inv_CellHeight_L);
  ISP_DBG("   inv_CellHeight_R %u", cmd->spCfg.inv_CellHeight_R);
  ISP_DBG("   cellWidth_L %u", cmd->spCfg.cellWidth_L);
  ISP_DBG("   cellWidth_R %u", cmd->spCfg.cellWidth_R);
  ISP_DBG("   cellHeight_L %u", cmd->spCfg.cellHeight_L);
  ISP_DBG("   cellHeight_R %u", cmd->spCfg.cellHeight_R);

}

/** abf47_cubic_f
 *  This is a utility function used by the scale mesh function to do cubic interpolation
 *
 **/
static void abf47_cubic_f(float fs, float *fc0, float *fc1, float *fc2,
  float *fc3)
{
  float fs3, fs2;

  fs2 = fs * fs;
  fs3 = fs * fs2;

  *fc0 = 0.5f * (-fs3 + 2.0f * fs2 - fs);
  *fc1 = 0.5f * (3.0f * fs3 - 5.0f * fs2 + 2.0f);
  *fc2 = 0.5f * (-3 * fs3 + 4.0f * fs2 + fs);
  *fc3 = 0.5f * (fs3 - fs2);
}

/** abf47_mesh_extend_1block
 *  This is a utility function used by the scale mesh function
 *
 * Input paramters:
 *  @MeshIn: the input itable
 *  @MeshOut: the output table
 *  @Nx:
 *  @Ny:
 **/
static void abf47_mesh_extend_1block(float *MeshIn,  float *MeshOut,
  int Nx, int Ny)
{
  int i, j;

  for (i = 1; i<Ny+1; i++) {
    for (j = 1; j < Nx + 1; j++) {
      MeshOut[i * (Nx + 2) + j] = MeshIn[(i - 1) * Nx + j - 1];
    }
  }

  MeshOut[0 * (Nx + 2) + 0] = MeshOut[1 * (Nx + 2) + 1] * 2 -
    MeshOut[2 * (Nx + 2) + 2];
  MeshOut[(Ny + 1) * (Nx + 2) + 0] = MeshOut[Ny * (Nx + 2) + 1] * 2 -
    MeshOut[(Ny - 1) * (Nx + 2) + 2];
  MeshOut[(Ny + 1) * (Nx + 2) + Nx +1] = MeshOut[Ny * (Nx + 2) + Nx] * 2 -
    MeshOut[(Ny - 1) * (Nx + 2) + Nx - 1];
  MeshOut[0 * (Nx + 2) + Nx + 1] = MeshOut[1 * (Nx + 2) + Nx] * 2 -
    MeshOut[2 * (Nx + 2) + Nx -1];

  for (i = 1; i < Ny + 1; i++) {
    MeshOut[i * (Nx + 2) + 0] = MeshOut[i * (Nx + 2) + 1] * 2 -
      MeshOut[i * (Nx + 2) + 2];
    MeshOut[i * (Nx + 2) + Nx +1] = MeshOut[i * (Nx + 2) + Nx] * 2 -
      MeshOut[i * (Nx + 2) + Nx -1];
  }
  for (j = 1; j < Nx + 1; j++) {
    MeshOut[0 * (Nx + 2) + j] = MeshOut[1 * (Nx + 2) + j] * 2 -
      MeshOut[2 * (Nx + 2) + j];
    MeshOut[(Ny + 1) * (Nx + 2) + j] = MeshOut[Ny * (Nx + 2) + j] * 2 -
      MeshOut[(Ny - 1) * (Nx + 2) + j];
  }

}

/** abf47_scale_mesh_table
 *  This function is used to resample the ideal Rolloff table (13x17) to the RNR grid (10x13)
 *
 * Input paramters:
 *  @MeshIn: the input ideal Rolloff table (13x17)at the full sensor
 *  @MeshOut: the output rolloff table (10x13) on RNR
 **/
static void abf47_scale_mesh_table(float *MeshIn,  float *MeshOut)
{
  float cxm, cx0, cx1, cx2, cym, cy0, cy1, cy2;
  float am, a0, a1, a2, bm, b0, b1, b2;
  float tx , ty;
  int ix, iy;
  int i, j;

  int MESH_H, MESH_H_MAX;
  int MESH_V, MESH_V_MAX;

  /* Initialize the roll-off mesh grid */
  MESH_H = 12;
  MESH_V = 9;

  // Calculate the rolloff grid at the full resolution
  MESH_H_MAX = 16;
  MESH_V_MAX = 12;
  float Extend_Mesh[(MESH_H_MAX+3)*(MESH_V_MAX+3)];

  /* outer extend the mesh data 1 block by keeping the same slope */
  abf47_mesh_extend_1block(MeshIn, &Extend_Mesh[0], MESH_H_MAX + 1,
    MESH_V_MAX + 1);

  /*  resample Extended Mesh data onto the roll-off mesh grid */
  for (i = 0; i < (MESH_V + 1); i++) {
    for (j = 0; j < (MESH_H + 1); j++) {
      tx =  (double)(j * MESH_H_MAX) / (double)(MESH_H) + 1;
      ix = floor(tx);
      tx -= (double)ix;

      ty =  (double)(i * MESH_V_MAX) / (double)(MESH_V)  + 1;
      iy = floor(ty);
      ty -= (double)iy;

      if (i == 0 || j == 0  || i == MESH_V || j == MESH_H) {
        /* for boundary points, use bilinear interpolation */
        b1 = (1 - tx) * Extend_Mesh[iy * (MESH_H_MAX + 3) + ix] + tx *
          Extend_Mesh[iy * (MESH_H_MAX + 3) + ix + 1];
        b2 = (1 - tx) * Extend_Mesh[(iy + 1) * (MESH_H_MAX + 3) + ix] + tx *
          Extend_Mesh[(iy + 1) * (MESH_H_MAX + 3) + ix + 1];

        MeshOut[(i * (MESH_H + 1)) + j] = (float)((1 - ty) * b1 + ty * b2);
        MeshOut[(i * (MESH_H + 1)) + j]  =
          fmin(fmax(MeshOut[(i * (MESH_H + 1)) + j], 1.0), 7.9);
      } else {
        // for nonboundary points, use bicubic interpolation
        /*get x direction coeff and y direction coeff*/
        abf47_cubic_f(tx, &cxm, &cx0, &cx1, &cx2);
        abf47_cubic_f(ty, &cym, &cy0, &cy1, &cy2);

        am = Extend_Mesh[(iy - 1) * (MESH_H_MAX + 3) + (ix - 1)];
        a0 = Extend_Mesh[(iy - 1) * (MESH_H_MAX + 3) + (ix)];
        a1 = Extend_Mesh[(iy - 1) * (MESH_H_MAX + 3) + (ix + 1)];
        a2 = Extend_Mesh[(iy - 1) * (MESH_H_MAX + 3) + (ix + 2)];
        bm = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Extend_Mesh[(iy) * (MESH_H_MAX + 3) + (ix - 1)];
        a0 = Extend_Mesh[(iy) * (MESH_H_MAX + 3) + (ix)];
        a1 = Extend_Mesh[(iy) * (MESH_H_MAX + 3) + (ix + 1)];
        a2 = Extend_Mesh[(iy) * (MESH_H_MAX + 3) + (ix + 2)];
        b0 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Extend_Mesh[(iy + 1) * (MESH_H_MAX + 3) + (ix - 1)];
        a0 = Extend_Mesh[(iy + 1) * (MESH_H_MAX + 3) + (ix)];
        a1 = Extend_Mesh[(iy + 1) * (MESH_H_MAX + 3) + (ix + 1)];
        a2 = Extend_Mesh[(iy + 1) * (MESH_H_MAX + 3) + (ix + 2)];
        b1 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        am = Extend_Mesh[(iy + 2) * (MESH_H_MAX + 3) + (ix - 1)];
        a0 = Extend_Mesh[(iy + 2) * (MESH_H_MAX + 3) + (ix)];
        a1 = Extend_Mesh[(iy + 2) * (MESH_H_MAX + 3) + (ix + 1)];
        a2 = Extend_Mesh[(iy + 2) * (MESH_H_MAX + 3) + (ix + 2)];
        b2 = ((cxm * am) + (cx0 * a0) + (cx1 * a1) + (cx2 * a2));

        MeshOut[(i * (MESH_H + 1)) + j] =  (float)((cym * bm) +
          (cy0 * b0) + (cy1 * b1) + (cy2 * b2));
        MeshOut[(i * (MESH_H + 1)) + j]  =
          fmin(fmax(MeshOut[(i * (MESH_H + 1)) + j], 1.0), 7.9);
      }
    }
  }
}

/** abf47_set_split_info
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  trigger update based on 3A event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean abf47_set_split_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret = TRUE;
  abf47_t                  *abf = NULL;
  isp_private_event_t      *private_event = NULL;
  isp_out_info_t           *isp_split_out_info = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  abf = (abf47_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    return FALSE;
  }

  private_event = (isp_private_event_t *)event->u.module_event.module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  isp_split_out_info = (isp_out_info_t *)private_event->data;
  if (!isp_split_out_info) {
    ISP_ERR("failed: split info NULL %p", isp_split_out_info);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  abf->isp_out_info = *isp_split_out_info;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
} /* abf_set_split_info */

/** abf47_set_stripe_info
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  trigger update based on 3A event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean abf47_set_stripe_info(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                  ret = TRUE;
  abf47_t                  *abf = NULL;
  isp_private_event_t      *private_event = NULL;
  ispif_out_info_t         *ispif_out_info = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  abf = (abf47_t *)isp_sub_module->private_data;
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    return FALSE;
  }

  private_event = (isp_private_event_t *)event->u.module_event.
    module_event_data;
  if (!private_event) {
    ISP_ERR("failed: private_event %p", private_event);
    return FALSE;
  }

  ispif_out_info = (ispif_out_info_t *)private_event->data;
  if (!ispif_out_info) {
    ISP_ERR("failed: stripe_limit %p", ispif_out_info);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  abf->ispif_out_info = *ispif_out_info;
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;
} /* abf_set_Stripe_info */

/** abf47_channel_config_reserve
.*
 *  @chCfg: handle to ISP_ABF3_ChannelCfg
 *  @abf3_reserve: Handle to Chromatix reserve config struct
 *  @idx: index corresponding to channel in chromatix struct
 *
 *  abf module reserve channel configuration
 *  This config does not change frame to frame. Set by Chromatix
 *
 **/
static void abf47_channel_config_reserve(isp_sub_module_t *isp_sub_module,
  ISP_ABF3_ChannelCfg *chCfg, chromatix_ABF3_reserve   *abf3_reserve, int idx)
{
  int32_t noise_offset = ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH;
  boolean  lsb_allignment = FALSE;

  chCfg->noiseScale0Lv0 =
    CLAMP_ABF(Round(((float)abf3_reserve->shot_noise_scale[0][idx]/
    SHORT_NOISE_SCALE)),4095);
  chCfg->noiseScale0Lv1 =
    CLAMP_ABF(Round(((float)abf3_reserve->shot_noise_scale[1][idx]/
    SHORT_NOISE_SCALE)),4095);
  chCfg->noiseScale1Lv0 =
    CLAMP_ABF(abf3_reserve->fpn_noise_scale[0][idx],MAX_FPN_NOISE_SCALE_LEVEL);
  chCfg->noiseScale1Lv1 =
    CLAMP_ABF(abf3_reserve->fpn_noise_scale[1][idx],MAX_FPN_NOISE_SCALE_LEVEL);
  chCfg->wTableAdjLv0 = WTABLE_ADJ_VAL;
  chCfg->wTableAdjLv1 = WTABLE_ADJ_VAL;
  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
      isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG ){
    lsb_allignment = TRUE;
  }
  if (!lsb_allignment){
      //MSB alligned data
      chCfg->noiseOffLv0 =
        CLAMP_ABF(abf3_reserve->noise_offset[0][idx]<<noise_offset,
        NOISE_OFFSET_MAX_VAL);
      chCfg->noiseOffLv1 =
        CLAMP_ABF(abf3_reserve->noise_offset[1][idx]<<noise_offset,
        NOISE_OFFSET_MAX_VAL);
  } else {
      //LSB alligned data
      chCfg->noiseOffLv0 =
        CLAMP_ABF(abf3_reserve->noise_offset[0][idx]>> noise_offset,
        NOISE_OFFSET_MAX_VAL);
      chCfg->noiseOffLv1 =
        CLAMP_ABF(abf3_reserve->noise_offset[1][idx]>> noise_offset,
        NOISE_OFFSET_MAX_VAL);
  }
  chCfg->sftThrdNoiseShiftLv0 =
    Clamp(abf3_reserve->softthld_noise_shift[0][idx],5,10);
  chCfg->sftThrdNoiseShiftLv1 =
    Clamp(abf3_reserve->softthld_noise_shift[1][idx],5,10);
}

/** abf47_config_reserve
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf handle
 *  This config does not change frame to frame. Set by Chromatix
 *
 *  abf module reserve configuration
 *
 *  Return None
 **/
static boolean abf47_config_reserve(isp_sub_module_t *isp_sub_module,
  abf47_t *abf)
{
  int i = 0;
  chromatix_ABF3_reserve   *abf3_reserve = NULL;
  chromatix_parms_type     *chromatix_ptr = NULL;
  chromatix_ABF3_type      *chromatix_ABF3 = NULL;

  if (!isp_sub_module || !abf) {
    ISP_ERR("failed: %p %p", isp_sub_module, abf);
    return FALSE;
  }

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;

  if (!chromatix_ptr) {
    ISP_ERR("failed: %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_ABF3 = &chromatix_ptr->chromatix_VFE.chromatix_ABF3;

  if (!chromatix_ABF3) {
    ISP_ERR("failed: %p", chromatix_ABF3);
    return FALSE;
  }

  abf3_reserve = &(chromatix_ABF3->abf3_reserved_data);

  abf->v3RegCmd.cfg.lutBankSel =0;
  abf->v3RegCmd.cfg.crossPlLevel0En =
    (abf3_reserve->filter_options >> CROSSP1LEVEL0EN) & 1;
  abf->v3RegCmd.cfg.crossPlLevel1En =
    (abf3_reserve->filter_options >> CROSSP1LEVEL1EN) & 1;

  abf->v3RegCmd.level1.distGrGb_0 = CLAMP_ABF(abf3_reserve->distance_ker[0][0], 3);
  abf->v3RegCmd.level1.distGrGb_1 = CLAMP_ABF(abf3_reserve->distance_ker[0][1], 3);
  abf->v3RegCmd.level1.distGrGb_2 = CLAMP_ABF(abf3_reserve->distance_ker[0][2], 3);
  abf->v3RegCmd.level1.distGrGb_3 = CLAMP_ABF(abf3_reserve->distance_ker[0][3], 3);
  abf->v3RegCmd.level1.distGrGb_4 = Clamp(abf3_reserve->distance_ker[0][4], 1, 6);
  abf->v3RegCmd.level1.distRb0    = CLAMP_ABF(abf3_reserve->distance_ker[1][0], 3);
  abf->v3RegCmd.level1.distRb1    = CLAMP_ABF(abf3_reserve->distance_ker[1][1], 3);
  abf->v3RegCmd.level1.distRb2    = CLAMP_ABF(abf3_reserve->distance_ker[1][2], 3);
  abf->v3RegCmd.level1.distRb3    = CLAMP_ABF(abf3_reserve->distance_ker[1][3], 3);
  abf->v3RegCmd.level1.distRb4    = Clamp(abf3_reserve->distance_ker[1][4], 1, 6);
  abf->v3RegCmd.level1.minMaxSelGrGb0 = abf3_reserve->minmax_sel[0][0];
  abf->v3RegCmd.level1.minMaxSelGrGb1 = abf3_reserve->minmax_sel[0][1];
  abf->v3RegCmd.level1.minMaxSelRb0   = abf3_reserve->minmax_sel[1][0];
  abf->v3RegCmd.level1.minMaxSelRb1   = abf3_reserve->minmax_sel[1][1];

  abf47_channel_config_reserve(isp_sub_module, &abf->v3RegCmd.rCfg, abf3_reserve, 0);
  abf47_channel_config_reserve(isp_sub_module, &abf->v3RegCmd.grCfg, abf3_reserve, 1);
  abf47_channel_config_reserve(isp_sub_module, &abf->v3RegCmd.gbCfg, abf3_reserve, 2);
  abf47_channel_config_reserve(isp_sub_module, &abf->v3RegCmd.bCfg, abf3_reserve, 3);

  FILL_SIG2_LUT(abf->abf3_parms.signal2_lut_lv0,
    abf3_reserve->signal2_lut_level0, 32);
  FILL_SIG2_LUT(abf->abf3_parms.signal2_lut_lv1,
    abf3_reserve->signal2_lut_level1, 32);
  return TRUE;
}
/** abf47_set_stream_config:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to sensor_out_info_t
 *
 *  Copy camif dimension in module private
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean abf47_set_stream_config(isp_sub_module_t *isp_sub_module,
  void *data)
{
  abf47_t                 *abf47 = NULL;
  sensor_out_info_t       *sensor_out_info = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  sensor_out_info = (sensor_out_info_t *)data;

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  abf47 = (abf47_t *)isp_sub_module->private_data;
  if (!abf47) {
    ISP_ERR("failed: abf47 %p", abf47);
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return FALSE;
  }

  abf47->sensor_out_info = *sensor_out_info;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return TRUE;
} /* abf47_t44_set_stream_config */

/** abf47_init_default
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf handle
 *
 *  abf module configuration initial settings
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean abf47_init_default(isp_sub_module_t *isp_sub_module,
  abf47_t *abf)
{
  int i = 0;

  if (!isp_sub_module || !abf) {
    ISP_ERR("failed: %p %p", isp_sub_module, abf);
    return FALSE;
  }

  if (!abf47_config_reserve(isp_sub_module, abf))
    return FALSE;

  abf->trigger_index = NORMAL_LIGHT_INDEX;

  return TRUE;
} /* abg47_config */


/** abf47_config_spatial_cfg:
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf struct data
 *
 *  Config the values of Spatial cfg
 *
 * Returns TRUE on success and FALSE on failure
 **/
static void abf47_config_grid_position(abf47_t *abf)
{
  uint32_t             camif_width = 0;
  uint32_t             camif_height = 0;
  uint32_t             overhead_half_w = 0;
  uint32_t             overhead_half_h = 0;
  uint32_t             right_offset = 0;
  sensor_out_info_t    *sensor_out_info = NULL;
  ISP_ABF3_SpatialCfg *spatial_cfg = NULL;

  if (abf == NULL) {
    ISP_ERR(" failed: NULL ptr %p", abf);
    return;
  }

  sensor_out_info = &abf->sensor_out_info;
  spatial_cfg = &abf->v3RegCmd.spCfg;

  camif_width = sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1;
  camif_height = sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1;

  overhead_half_w = ((spatial_cfg->cellWidth_L * ABF_HORI_GRID_NUM) - camif_width) / 2;
  overhead_half_h = ((spatial_cfg->cellHeight_L * ABF_VERT_GRID_NUM) - camif_height) / 2;

  /*ideally single vfe need to consider position,
    system team RNR designer suggest no need to consider it for single vfe
    since the denoise impact is low.
    leave it until documented and simulator changed.*/
  if (abf->ispif_out_info.is_split) {
    /*check the definition of the position registers*/
    spatial_cfg->init_CellNum_X_L = overhead_half_w / spatial_cfg->cellWidth_L;
    spatial_cfg->init_CellNum_Y_L = overhead_half_h / spatial_cfg->cellHeight_L;
    spatial_cfg->init_DX_L = (overhead_half_w % spatial_cfg->cellWidth_L);
    spatial_cfg->init_DY_L = (overhead_half_h % spatial_cfg->cellHeight_L);
    spatial_cfg->init_PX_L = spatial_cfg->init_DX_L *
      spatial_cfg->inv_CellWidth_L;
    spatial_cfg->init_PY_L = spatial_cfg->init_DY_L *
      spatial_cfg->inv_CellHeight_L;
  }

  if (abf->ispif_out_info.is_split &&
      abf->isp_out_info.stripe_id == ISP_STRIPE_RIGHT) {
    spatial_cfg->init_CellNum_X_L = abf->isp_out_info.right_stripe_offset /
      spatial_cfg->cellWidth_L;
    spatial_cfg->init_DX_L = (abf->isp_out_info.right_stripe_offset %
      spatial_cfg->cellWidth_L);
    spatial_cfg->init_PX_L = spatial_cfg->init_DX_L *
      spatial_cfg->inv_CellWidth_L;
  }

  /* 3D currently not support */
  spatial_cfg->init_CellNum_X_R = 0;
  spatial_cfg->init_CellNum_Y_R = 0;
  spatial_cfg->init_DX_R = 0;
  spatial_cfg->init_DY_R = 0;
  spatial_cfg->init_PX_R = 0;
  spatial_cfg->init_PY_R = 0;

  return;
}

/** abf47_config_spatial_cfg:
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf struct data
 *
 *  Config the values of Spatial cfg
 *
 * Returns TRUE on success and FALSE on failure
 **/
static void abf47_config_spatial_cfg(abf47_t *abf)
{
  /* These are values for single VFE */
  boolean  ret           = TRUE;
  float    tmp           = 0;
  uint32_t camif_width  = 0;
  uint32_t camif_height = 0;
  uint32_t width         = 0;
  uint32_t height        = 0;
  sensor_out_info_t   *sensor_out_info = NULL;
  ISP_ABF3_SpatialCfg *spatial_cfg     = NULL;

  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    return;
  }
  spatial_cfg     = &abf->v3RegCmd.spCfg;
  sensor_out_info = &abf->sensor_out_info;

  spatial_cfg->init_CellNum_X_L = 0;
  spatial_cfg->init_CellNum_X_R = 0;
  spatial_cfg->init_CellNum_Y_L = 0;
  spatial_cfg->init_CellNum_Y_R = 0;

  spatial_cfg->init_DX_L = 0;
  spatial_cfg->init_DX_R = 0;

  spatial_cfg->init_DY_L = 0;
  spatial_cfg->init_DY_R = 0;

  spatial_cfg->init_PX_L = 0;
  spatial_cfg->init_PX_R = 0;

  spatial_cfg->init_PY_L = 0;
  spatial_cfg->init_PY_R = 0;

  /* The table is 13x10. So there are 12x9 grids.          *
   * CellWidth is width of each cell = camif_width/12     *
   * CellHeight is height of each each = camif_height/9   */

  /*Width ceiling*/
  camif_width = sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1;
  width = (uint32_t)(camif_width + ABF_HORI_GRID_NUM - 1) / ABF_HORI_GRID_NUM;
  /*Height ceiling*/
  camif_height = sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1;
  height = (uint32_t)(camif_height + ABF_VERT_GRID_NUM - 1) / ABF_VERT_GRID_NUM;

  spatial_cfg->cellWidth_L = CLAMP_ABF(width,
    (1 << ABF_CELL_BITWIDTH) - 1);
  spatial_cfg->cellHeight_L = CLAMP_ABF(height,
    (1 << ABF_CELL_BITWIDTH) - 1);

  /* 3D not supported, set some default as Left */
  spatial_cfg->cellWidth_R = spatial_cfg->cellWidth_L;
  spatial_cfg->cellHeight_R = spatial_cfg->cellHeight_L;

  if (spatial_cfg->cellWidth_L == 0 || spatial_cfg->cellHeight_L == 0) {
    ISP_ERR(" ERROR! cell width %u height %u", spatial_cfg->cellWidth_L,
      spatial_cfg->cellHeight_L);
    return;
  }
  width = (1 << ABF_INVCELL_BITSHIFT) / spatial_cfg->cellWidth_L;
  spatial_cfg->inv_CellWidth_L = CLAMP_ABF(width,
    (1 << ABF_INVCELL_BITWIDTH) - 1);

  spatial_cfg->inv_CellWidth_R = CLAMP_ABF(width,
    (1 << ABF_INVCELL_BITWIDTH) - 1);

  height = (1 << ABF_INVCELL_BITSHIFT) / spatial_cfg->cellHeight_L;
  spatial_cfg->inv_CellHeight_L = CLAMP_ABF(height,
    (1 << ABF_INVCELL_BITWIDTH) - 1);

  spatial_cfg->inv_CellHeight_R = CLAMP_ABF(height,
    (1 << ABF_INVCELL_BITWIDTH) - 1);

  abf47_config_grid_position(abf);

  return;
}
/** abf47_config_channel
 *
 *  @chCfg: handle to ISP_ABF3_ChannelCfg
 *  @abf3_data: Handle to Chromatix core config struct
 *  @idx: index corresponding to channel in chromatix struct
 *
 *  Interpolate and config all registers
 *
 * Returns TRUE on success and FALSE on failure
 **/
static void abf47_config_channel(
  ISP_ABF3_ChannelCfg      *chCfg,
  chromatix_abf3_core_type *abf3_data,
  abf_adjustment_t          abf_adj,
  int                       idx)
{
  float tmp = 0.0f;
  unsigned short tmp2 = 0;

  /*edge_softness */
  chCfg->noiseScale2Lv0 = CLAMP_ABF(FLOAT_TO_Q(8,
    abf3_data->edge_softness[0][idx] * abf_adj.edge_softness_adj_0),4095);

  chCfg->noiseScale2Lv1 = CLAMP_ABF(FLOAT_TO_Q(8,
    abf3_data->edge_softness[1][idx] * abf_adj.edge_softness_adj_1),4095);

  /*curve offset*/
  chCfg->curvOffLv0 = abf3_data->curve_offset[0][idx];

  chCfg->curvOffLv1 = abf3_data->curve_offset[1][idx];

  /*filterNoisePreserveL Lv0*/
  chCfg->filterNoisePreserveL_Lv0 = Clamp(FLOAT_TO_Q(9,
    abf3_data->filter_strength[0][idx] * abf_adj.filter_strength_0), 0, DENOISE_STRENGTH_MAX_VAL);
  chCfg->filterNoisePreserveR_Lv0 =
    chCfg->filterNoisePreserveL_Lv0;

  /*filterNoisePreserveL Lv1*/
  chCfg->filterNoisePreserveL_Lv1 = Clamp(FLOAT_TO_Q(9,
    abf3_data->filter_strength[1][idx] * abf_adj.filter_strength_1), 0, DENOISE_STRENGTH_MAX_VAL);
  chCfg->filterNoisePreserveR_Lv1 =
    chCfg->filterNoisePreserveL_Lv1;

  /*sftThrdNoiseScaleLv0*/
  chCfg->sftThrdNoiseScaleLv0 = Clamp(FLOAT_TO_Q(8,
    abf3_data->softthld_noise_scale[0][idx]), 0, (1 << 12) - 1);

  /*sftThrdNoiseScaleLv1*/
  chCfg->sftThrdNoiseScaleLv1 = Clamp(FLOAT_TO_Q(8,
    abf3_data->softthld_noise_scale[1][idx]), 0, (1 << 12) - 1);

  /*spatial config*/
  chCfg->spatialScaleL_Lv0 = Clamp(FLOAT_TO_Q(8,
    abf3_data->spatial_scale[0][idx]), 0, 256);
  chCfg->spatialScaleR_Lv0 = chCfg->spatialScaleL_Lv0;

  chCfg->spatialScaleL_Lv1 = Clamp(FLOAT_TO_Q(8,
    abf3_data->spatial_scale[1][idx]), 0, 256);
  chCfg->spatialScaleR_Lv1 = chCfg->spatialScaleL_Lv1;
}


/** abf47_config_channel_adjustment
 *
 *  @chCfg: handle to ISP_ABF3_ChannelCfg
 *  @abf3_data_start_adj: Handle to Chromatix adjustment struct
 *  @abf3_data_start_adj: Handle to Chromatix adjustment struct
 *  @ratio: ratio used to interpolate
 *  @idx: index corresponding to channel in chromatix struct
 *
 *  Interpolate and config all registers
 *
 * Returns TRUE on success and FALSE on failure
 **/
static void abf47_config_channel_interpolate_adjustment(
  hdr_abf3_adj_core_type *abf3_data_start_adj,
  hdr_abf3_adj_core_type *abf3_data_end_adj,
  float                   ratio,
  int                     idx,
  abf_adjustment_t        *abf_adj,
  boolean                 enable_abf_adjustment)
{
  abf_adj->edge_softness_adj_0 = (enable_abf_adjustment == TRUE) ?
      (float)(LINEAR_INTERPOLATION(abf3_data_start_adj->edge_softness_adj[0][idx],
      abf3_data_end_adj->edge_softness_adj[0][idx], ratio)) : 1.0f;

  abf_adj->edge_softness_adj_1 = (enable_abf_adjustment == TRUE) ?
      (float)(LINEAR_INTERPOLATION(abf3_data_start_adj->edge_softness_adj[1][idx],
      abf3_data_end_adj->edge_softness_adj[1][idx], ratio)) : 1.0f;

  /*filterNoisePreserveL Lv0*/
  abf_adj->filter_strength_0 = (enable_abf_adjustment == TRUE) ?
      (float)(LINEAR_INTERPOLATION(abf3_data_start_adj->filter_strength_adj[0][idx],
      abf3_data_end_adj->filter_strength_adj[0][idx], ratio)) : 1.0f;

  /*filterNoisePreserveL Lv0*/
  abf_adj->filter_strength_1 = (enable_abf_adjustment == TRUE) ?
      (float)(LINEAR_INTERPOLATION(abf3_data_start_adj->filter_strength_adj[1][idx],
      abf3_data_end_adj->filter_strength_adj[1][idx], ratio)) : 1.0f;

  abf_adj->noise_profile_adj_0 = (enable_abf_adjustment == TRUE) ?
      (float)(LINEAR_INTERPOLATION(abf3_data_start_adj->noise_profile_adj[0],
      abf3_data_end_adj->noise_profile_adj[0], ratio)) : 1.0f;

  abf_adj->noise_profile_adj_1 = (enable_abf_adjustment == TRUE) ?
      (float)(LINEAR_INTERPOLATION(abf3_data_start_adj->noise_profile_adj[1],
      abf3_data_end_adj->noise_profile_adj[1], ratio)) : 1.0f;
}

/** abf47_config_channel_adjustment
 *
 *  @chCfg: handle to ISP_ABF3_ChannelCfg
 *  @abf3_data_start_adj: Handle to Chromatix adjustment struct
 *  @abf3_data_start_adj: Handle to Chromatix adjustment struct
 *  @ratio: ratio used to interpolate
 *  @idx: index corresponding to channel in chromatix struct
 *
 *  Interpolate and config all registers
 *
 * Returns TRUE on success and FALSE on failure
 **/
static void abf47_config_channel_adjustment(
  hdr_abf3_adj_core_type *abf3_data_start_adj,
  int                     idx,
  abf_adjustment_t        *abf_adj,
  boolean                 enable_abf_adjustment)
{
  abf_adj->edge_softness_adj_0 = (enable_abf_adjustment == TRUE) ?
      abf3_data_start_adj->edge_softness_adj[0][idx] : 1.0f;

  abf_adj->edge_softness_adj_1 = (enable_abf_adjustment == TRUE) ?
      abf3_data_start_adj->edge_softness_adj[1][idx] : 1.0f;

  /*filterNoisePreserveL Lv0*/
  abf_adj->filter_strength_0 = (enable_abf_adjustment == TRUE) ?
      abf3_data_start_adj->filter_strength_adj[0][idx] : 1.0f;

  /*filterNoisePreserveL Lv0*/
  abf_adj->filter_strength_1 = (enable_abf_adjustment == TRUE) ?
      abf3_data_start_adj->filter_strength_adj[1][idx] : 1.0f;
  abf_adj->noise_profile_adj_0 = (enable_abf_adjustment == TRUE) ?
      abf3_data_start_adj->noise_profile_adj[0] : 1.0f;

  abf_adj->noise_profile_adj_1 = (enable_abf_adjustment == TRUE) ?
      abf3_data_start_adj->noise_profile_adj[1] : 1.0f;
}

/** abf47_config_channel_interpolate
 *
 *  @chCfg: handle to ISP_ABF3_ChannelCfg
 *  @abf3_data_start: Handle to Chromatix core config struct
 *  @abf3_data_end: Handle to Chromatix core config struct
 *  @ratio: ratio used to interpolate
 *  @idx: index corresponding to channel in chromatix struct
 *
 *  Interpolate and config all registers
 *
 * Returns TRUE on success and FALSE on failure
 **/
static void abf47_config_channel_interpolate(
  ISP_ABF3_ChannelCfg      *chCfg,
  chromatix_abf3_core_type *abf3_data_start,
  chromatix_abf3_core_type *abf3_data_end,
  float                     ratio,
  int                       idx,
  abf_adjustment_t          abf_adj)
{
  float             tmp  = 0;
  unsigned short    tmp2 = 0;

  /*edge_softness */
  tmp = (float)(LINEAR_INTERPOLATION(abf3_data_start->edge_softness[0][idx],
    abf3_data_end->edge_softness[0][idx], ratio));
  chCfg->noiseScale2Lv0 = CLAMP_ABF(FLOAT_TO_Q(8, (tmp * abf_adj.edge_softness_adj_0)),4095);

  tmp = (float)(LINEAR_INTERPOLATION(abf3_data_start->edge_softness[1][idx],
    abf3_data_end->edge_softness[1][idx], ratio));
  chCfg->noiseScale2Lv1 = CLAMP_ABF(FLOAT_TO_Q(8, (tmp * abf_adj.edge_softness_adj_1)),4095);

  /*curve offset*/
  tmp2 = (unsigned short)Round(LINEAR_INTERPOLATION(
    (float)abf3_data_start->curve_offset[0][idx],
    (float)abf3_data_end->curve_offset[0][idx], ratio));
  chCfg->curvOffLv0 = tmp2;

  tmp2 = (unsigned short)Round(LINEAR_INTERPOLATION(
    (float)abf3_data_start->curve_offset[1][idx],
    (float)abf3_data_end->curve_offset[1][idx], ratio));
  chCfg->curvOffLv1 = tmp2;

  /*filterNoisePreserveL Lv0*/
  tmp = (float)LINEAR_INTERPOLATION(abf3_data_start->filter_strength[0][idx],
    abf3_data_end->filter_strength[0][idx], ratio);
  chCfg->filterNoisePreserveL_Lv0 =
    Clamp(FLOAT_TO_Q(9, (tmp * abf_adj.filter_strength_0)), 0, DENOISE_STRENGTH_MAX_VAL);
  chCfg->filterNoisePreserveR_Lv0 =
    chCfg->filterNoisePreserveL_Lv0;

  /*filterNoisePreserveL Lv1*/
  tmp = (float)LINEAR_INTERPOLATION(abf3_data_start->filter_strength[1][idx],
    abf3_data_end->filter_strength[1][idx], ratio);
  chCfg->filterNoisePreserveL_Lv1 =
    Clamp(FLOAT_TO_Q(9, (tmp * abf_adj.filter_strength_1)), 0, DENOISE_STRENGTH_MAX_VAL);
  chCfg->filterNoisePreserveR_Lv1 =
    chCfg->filterNoisePreserveL_Lv1;

  /*sftThrdNoiseScaleLv0*/
  tmp = (float)LINEAR_INTERPOLATION(
    abf3_data_start->softthld_noise_scale[0][idx],
    abf3_data_end->softthld_noise_scale[0][idx], ratio);
  chCfg->sftThrdNoiseScaleLv0 = Clamp(FLOAT_TO_Q(8, tmp),
    0, (1 << 12) - 1);

  /*sftThrdNoiseScaleLv1*/
  tmp = (float)LINEAR_INTERPOLATION(
    abf3_data_start->softthld_noise_scale[1][idx],
    abf3_data_end->softthld_noise_scale[1][idx], ratio);
  chCfg->sftThrdNoiseScaleLv1 = Clamp(FLOAT_TO_Q(8, tmp),
    0, (1 << 12) - 1);

  /*spatial config*/
  tmp = (float)LINEAR_INTERPOLATION(abf3_data_start->spatial_scale[0][idx],
    abf3_data_end->spatial_scale[0][idx], ratio);
  chCfg->spatialScaleL_Lv0 = Clamp(FLOAT_TO_Q(8, tmp), 0, 256);
  chCfg->spatialScaleR_Lv0 = chCfg->spatialScaleL_Lv0;

  tmp = (float)LINEAR_INTERPOLATION(abf3_data_start->spatial_scale[1][idx],
    abf3_data_end->spatial_scale[1][idx], ratio);
  chCfg->spatialScaleL_Lv1 = Clamp(FLOAT_TO_Q(8, tmp), 0, 256);
  chCfg->spatialScaleR_Lv1 = chCfg->spatialScaleL_Lv1;
}


/** abf47_config_trigger_interpolate
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf struct data
 *
 *  Interpolate and config all registers
 *
 * Returns TRUE on success and FALSE on failure
 **/
static void abf47_config_trigger_interpolate(isp_sub_module_t *isp_sub_module,
  abf47_t *abf)
{
  int i = 0;
  chromatix_abf3_core_type    *abf3_data_start      = NULL;
  chromatix_abf3_core_type    *abf3_data_end        = NULL;
  hdr_abf3_adj_core_type      *abf3_data_start_adj  = NULL;
  hdr_abf3_adj_core_type      *abf3_data_end_adj    = NULL;
  chromatix_parms_type        *chromatix_ptr        = NULL;
  chromatix_ABF3_type         *chromatix_ABF3       = NULL;
  Chromatix_HDR_ABF3_adj_type *chromatix_ABF3_type2 = NULL;
  chromatix_ABF3_reserve      *abf3_reserve         = NULL;

  abf_adjustment_t abf_adj_rCfg;
  abf_adjustment_t abf_adj_grCfg;
  abf_adjustment_t abf_adj_gbCfg;
  abf_adjustment_t abf_adj_bCfg;

  float   ratio     = 1 - abf->aec_ratio.ratio;
  float   ratio_adj = 1 - abf->aec_ratio.ratio_adj;

  ISP_HIGH("%s", __func__);
  chromatix_ptr        = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return;
  }

  chromatix_ABF3       = &chromatix_ptr->chromatix_VFE.chromatix_ABF3;
  chromatix_ABF3_type2 = &chromatix_ptr->chromatix_VFE.chromatix_hdr_abf3_adj_data;

  abf3_reserve = &(chromatix_ABF3->abf3_reserved_data);

  abf3_data_start     = &(chromatix_ABF3->abf3_config[abf->trigger_index]);
  abf3_data_start_adj = &(chromatix_ABF3_type2->hdr_abf3_adj_core_data[abf->trigger_index_adj]);
  abf->v3RegCmd.cfg.minMaxEn = abf3_data_start->min_max_filter_en;

  ISP_DBG("trigger index %u, ratio %f",
    abf->trigger_index, abf->aec_ratio.ratio);

  if (abf->trigger_index_adj + 1 >= MAX_SETS_FOR_TONE_NOISE_ADJ) {
    abf47_config_channel_adjustment(abf3_data_start_adj,
    0, &abf_adj_rCfg, chromatix_ABF3_type2->enable);

    abf47_config_channel_adjustment(abf3_data_start_adj,
    1, &abf_adj_grCfg, chromatix_ABF3_type2->enable);

    abf47_config_channel_adjustment(abf3_data_start_adj,
    1, &abf_adj_gbCfg, chromatix_ABF3_type2->enable);

    abf47_config_channel_adjustment(abf3_data_start_adj,
    0, &abf_adj_bCfg, chromatix_ABF3_type2->enable);
  } else {
    abf3_data_end_adj = &(chromatix_ABF3_type2->hdr_abf3_adj_core_data[abf->trigger_index_adj + 1]);

    abf47_config_channel_interpolate_adjustment(abf3_data_start_adj,
    abf3_data_end_adj, ratio_adj, 0, &abf_adj_rCfg, chromatix_ABF3_type2->enable);

    abf47_config_channel_interpolate_adjustment(abf3_data_start_adj,
    abf3_data_end_adj, ratio_adj, 1, &abf_adj_grCfg, chromatix_ABF3_type2->enable);

    abf47_config_channel_interpolate_adjustment(abf3_data_start_adj,
    abf3_data_end_adj, ratio_adj, 1, &abf_adj_gbCfg, chromatix_ABF3_type2->enable);

    abf47_config_channel_interpolate_adjustment(abf3_data_start_adj,
    abf3_data_end_adj, ratio_adj, 0, &abf_adj_bCfg, chromatix_ABF3_type2->enable);
  }

  if (abf->trigger_index + 1 >= MAX_LIGHT_TYPES_FOR_SPATIAL_ABF){

    abf47_config_channel(&abf->v3RegCmd.rCfg, abf3_data_start, abf_adj_rCfg, 0);

    abf47_config_channel(&abf->v3RegCmd.grCfg, abf3_data_start, abf_adj_grCfg, 1);

    abf47_config_channel(&abf->v3RegCmd.gbCfg, abf3_data_start, abf_adj_gbCfg, 2);

    abf47_config_channel(&abf->v3RegCmd.bCfg, abf3_data_start, abf_adj_bCfg, 3);

    FILL_LUT(abf->abf3_parms.noise_std2_lut_lv0,
      abf3_data_start->noise_std2_lut_level0, 128, abf_adj_bCfg.noise_profile_adj_0);
    FILL_LUT(abf->abf3_parms.noise_std2_lut_lv1,
      abf3_data_start->noise_std2_lut_level1, 128, abf_adj_bCfg.noise_profile_adj_1);
  } else {
    abf3_data_end = &(chromatix_ABF3->abf3_config[abf->trigger_index + 1]);

    abf47_config_channel_interpolate(&abf->v3RegCmd.rCfg, abf3_data_start,
      abf3_data_end, ratio, 0, abf_adj_rCfg);

    abf47_config_channel_interpolate(&abf->v3RegCmd.grCfg, abf3_data_start,
      abf3_data_end, ratio, 1, abf_adj_grCfg);

    abf47_config_channel_interpolate(&abf->v3RegCmd.gbCfg, abf3_data_start,
      abf3_data_end, ratio, 2, abf_adj_gbCfg);

    abf47_config_channel_interpolate(&abf->v3RegCmd.bCfg, abf3_data_start,
      abf3_data_end, ratio, 3, abf_adj_bCfg);

    FILL_LUT_INTERP(abf->abf3_parms.noise_std2_lut_lv0,
      abf3_data_start->noise_std2_lut_level0,
      abf3_data_end->noise_std2_lut_level0,
      128,
      abf->aec_ratio.ratio,
      abf_adj_bCfg.noise_profile_adj_0);

    FILL_LUT_INTERP(abf->abf3_parms.noise_std2_lut_lv1,
      abf3_data_start->noise_std2_lut_level1,
      abf3_data_end->noise_std2_lut_level1,
      128, abf->aec_ratio.ratio,
      abf_adj_bCfg.noise_profile_adj_1);
/*   PRINT_LUT(abf->abf3_parms.noise_std2_lut_lv1, 128); */
  }
}

/** abf47_do_hw_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf struct data
 *
 *  prepare hw update list and append in isp sub module
 *
 * Returns TRUE on success and FALSE on failure
 **/
static boolean abf47_store_hw_update(
  isp_sub_module_t *isp_sub_module, abf47_t *abf)
{
  boolean                           ret = TRUE;
  struct msm_vfe_cfg_cmd2          *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd       *reg_cfg_cmd = NULL;
  struct msm_vfe_cfg_cmd_list      *hw_update = NULL;
  ISP_ABF3_CmdType                 *v3RegCmd = NULL;
  int i;
  uint32_t *data;
  uint32_t bank;

  uint32_t channel[2][5] = {
    {
      ABF_STD2_L0_BANK0,
      ABF_STD2_L1_BANK0,
      ABF_SIG2_L0_BANK0,
      ABF_SIG2_L1_BANK0,
      ABF_MESH_LUT_BANK0
    },
    {
      ABF_STD2_L0_BANK1,
      ABF_STD2_L1_BANK1,
      ABF_SIG2_L0_BANK1,
      ABF_SIG2_L1_BANK1,
      ABF_MESH_LUT_BANK1
    }
  };

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

  if (ABF_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      ABF_CGC_OVERRIDE_REGISTER, ABF_CGC_OVERRIDE_BIT, TRUE);
    if (ret == FALSE) {
      ISP_ERR("failed: enable cgc");
    }
  }

  abf->v3RegCmd.cfg.lutBankSel ^= 1;
  bank = abf->v3RegCmd.cfg.lutBankSel;
  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(sizeof(*reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd));

  v3RegCmd = (ISP_ABF3_CmdType *)CAM_MALLOC(sizeof(*v3RegCmd));
  if (!v3RegCmd) {
    ISP_ERR("failed: reg_cmd %p", v3RegCmd);
    goto ERROR;
  }
  memset(v3RegCmd, 0, sizeof(*v3RegCmd));
  *v3RegCmd = abf->v3RegCmd;

  ret = isp_sub_module_util_write_dmi(
   (void *)&abf->abf3_parms.noise_std2_lut_lv0, sizeof(noise_std2_lut_t),
    channel[bank][0],
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_g");
    goto ERROR;
  }
  ret = isp_sub_module_util_write_dmi(
     (void *)&abf->abf3_parms.noise_std2_lut_lv1, sizeof(noise_std2_lut_t),
      channel[bank][1],
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_g");
    goto ERROR;
  }
  ret = isp_sub_module_util_write_dmi(
    (void *)&abf->abf3_parms.signal2_lut_lv0, sizeof(signal2_lut_t),
    channel[bank][2],
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_g");
    goto ERROR;
  }
  ret = isp_sub_module_util_write_dmi(
    (void *)&abf->abf3_parms.signal2_lut_lv1, sizeof(signal2_lut_t),
    channel[bank][3],
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_g");
    goto ERROR;
  }
  ret = isp_sub_module_util_write_dmi(
    (void *)&abf->abf3_parms.abf_mesh_lut, sizeof(abf_mesh_lut_t),
    channel[bank][4],
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_g");
    goto ERROR;
  }
  /* configurations */
  cfg_cmd->cfg_data = (void *)v3RegCmd;
  cfg_cmd->cmd_len = sizeof(*v3RegCmd);
  cfg_cmd->cfg_cmd = (void *) reg_cfg_cmd;
  cfg_cmd->num_cfg = 1;

  reg_cfg_cmd[0].cmd_type = VFE_WRITE;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_ABF3_OFF;
  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].u.rw_info.len = ISP_ABF3_LEN * sizeof(uint32_t);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
      free(v3RegCmd);
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
  return TRUE;

ERROR:
  if (v3RegCmd) {
    free(v3RegCmd);
  }
  if (reg_cfg_cmd) {
    free(reg_cfg_cmd);
  }
  if (hw_update) {
    free(hw_update);
  }

  return FALSE;
} /* abf47_store_hw_update */

/** abf47_fetch_mesh_table
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf module private data
 *  @event_identity: identity of trigger update mct event
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean abf47_fetch_mesh_table(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, abf47_t *abf, uint32_t event_identity)
{
  boolean ret = TRUE;
  mct_event_t                    event;
  mct_port_t                    *out_port = NULL;
  isp_private_event_t            private_event;
  /* Fetch rolloff table from rolloff module. Size is 17 * 13 */
  float                          mesh_table[MESH_ROLLOFF_SIZE];

  memset(&event, 0, sizeof(event));
  event.direction = MCT_EVENT_UPSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.module_event_data = &private_event;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.identity = event_identity;

  private_event.data = &mesh_table;
  private_event.data_size = sizeof(mesh_table);
  private_event.type = ISP_PRIVATE_FETCH_ROLLOFF_INPUT_TABLE;

  ret = isp_sub_module_util_forward_event_from_module(module, &event);
  if (ret == FALSE) {
    ISP_ERR(" failed to fetch rolloff table ");
    return ret;
  }
  if (memcmp(&mesh_table, &abf->fetched_mesh_table, sizeof(mesh_table))) {
    memcpy(&abf->fetched_mesh_table, &mesh_table, sizeof(mesh_table));
    /*isp_sub_module mutex is locked in trigger update*/
    isp_sub_module->trigger_update_pending = TRUE;
  }
  return ret;
}

/** abf47_adjust_mesh_table
 *
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf module private data
 *
 *  Return TRUE on success and FALSE on failure
 **/
static void abf47_adjust_mesh_table(isp_sub_module_t *isp_sub_module,
  abf47_t *abf)
{
  uint32_t i, j;
  float                    *rolloff_table = NULL;
  float                     distance_percentage, gain_percentage;
  uint32_t                  m_nW = ABF3_MESH_WIDTH;
  uint32_t                  m_nH = ABF3_MESH_HEIGHT;
  double                    m_fCtrX, m_fCtrY ;
  double                    m_fRmax, m_fR0,m_fRate ;
  chromatix_parms_type     *chromatix_ptr = NULL;
  chromatix_ABF3_type      *chromatix_ABF3 = NULL;
  chromatix_abf3_core_type *abf3_data_start = NULL;
  chromatix_abf3_core_type *abf3_data_end = NULL;
  float                     ratio = 1 - abf->aec_ratio.ratio;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return ;
  }
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return;
  }

  chromatix_ABF3 = &chromatix_ptr->chromatix_VFE.chromatix_ABF3;

  abf3_data_start = &(chromatix_ABF3->abf3_config[abf->trigger_index]);

  if (abf->trigger_index + 1 >= MAX_LIGHT_TYPES_FOR_SPATIAL_ABF) {
    abf3_data_end   = &(chromatix_ABF3->abf3_config[abf->trigger_index]);
  } else {
      abf3_data_end   = &(chromatix_ABF3->abf3_config[abf->trigger_index + 1]);
  }

  rolloff_table = &abf->mesh_table[0];
  distance_percentage = (float)(LINEAR_INTERPOLATION(
    abf3_data_start->distance_percentage,
    abf3_data_end->distance_percentage, ratio));
  gain_percentage = (float)(LINEAR_INTERPOLATION(
    abf3_data_start->gain_percentage,
    abf3_data_end->gain_percentage, ratio));

  m_fCtrX = (double)(m_nW - 1) / 2.0;
  m_fCtrY = (double)(m_nH - 1) / 2.0;

  m_fRmax = sqrt((double) (m_fCtrX * m_fCtrX + m_fCtrY * m_fCtrY));
  /* radius */
  m_fR0 = m_fRmax * (double) (distance_percentage);
  /* gain */
  m_fRate = (double) (gain_percentage );

  if (m_fR0 == m_fRmax) {
    ISP_DBG(" R0 == RMax. No change required");
  } else {
    for(i = 0; i < m_nH; i++) {
      for(j = 0; j < m_nW; j++) {
        double x = (double)i - m_fCtrY;
        double y = (double)j - m_fCtrX;
        double rr = sqrt((double) (x * x + y * y));
        if (m_fR0 == m_fRmax) {
          rolloff_table[i * m_nW + j] *= 1.0;
        } else if(rr <= m_fR0) {
          rolloff_table[i * m_nW + j] *= 1.0;
        } else {
          rolloff_table[i * m_nW + j] *=
            (m_fRate + (1.0 - m_fRate)* (rr - m_fRmax) / (m_fR0 - m_fRmax));
        }
      }
    }
  }
}

/** abf47_pack_mesh_table
 *
 *  @abf: abf module private data
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean abf47_pack_mesh_table(abf47_t *abf)
{
  int i = 0;
  for (i=0; i < 130; i++)
    abf->abf3_parms.abf_mesh_lut[i] = Clamp(Round(abf->mesh_table[i] * Q9),
      512, ABF_MESH_TABLE_MAX);

  return TRUE;
}

/** abf47_cfg_mesh_table
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @abf: abf module private data
 *  @event_identity: identity of trigger update mct event
 *
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean abf47_cfg_mesh_table(isp_sub_module_t *isp_sub_module,
  abf47_t *abf)
{
  boolean ret = TRUE;

  abf47_scale_mesh_table(abf->fetched_mesh_table, abf->mesh_table);
  abf47_adjust_mesh_table(isp_sub_module, abf);
  abf47_pack_mesh_table(abf);

  return ret;
}

/** abf47_trigger_update:
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
boolean abf47_trigger_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean                 ret                = TRUE;
  abf47_t                 *abf               = NULL;
  isp_private_event_t     *private_event     = NULL;
  isp_sub_module_output_t *output            = NULL;
  isp_sub_module_output_t *sub_module_output = NULL;
  isp_meta_entry_t        *abf_dmi_info      = NULL;
  uint8_t                 module_enable;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  abf = (abf47_t *)isp_sub_module->private_data;
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

  if (!isp_sub_module->chromatix_module_enable) {
    /* do not config HW, but report meta if any manual control mode*/
    goto FILL_METADATA;
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

  ret = abf47_fetch_mesh_table(module, isp_sub_module, abf, event->identity);
  if (ret == FALSE) {
    ISP_ERR("abf47_fetch_mesh_table failed");
  }
  if ((isp_sub_module->submod_trigger_enable == TRUE) &&
    (isp_sub_module->trigger_update_pending == TRUE)) {
    ISP_DBG("aec_ratio.ratio = %f",abf->aec_ratio.ratio);
    if (abf->aec_ratio.ratio < 0) {
      abf->aec_ratio.ratio = 0;
    }
    abf47_config_spatial_cfg(abf);
    abf47_config_trigger_interpolate(isp_sub_module, abf);
    abf47_cfg_mesh_table(isp_sub_module, abf);
    /* do hw update*/
    abf47_store_hw_update(isp_sub_module, abf);
  }

  abf47_debug(&abf->v3RegCmd);

  output = (isp_sub_module_output_t *)private_event->data;
  if (!output) {
    ISP_ERR("failed: output %p", output);
    ret = FALSE;
    goto FILL_METADATA;
  }
  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module, output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    ret = FALSE;
    goto FILL_METADATA;
  }

  if (sub_module_output->metadata_dump_enable == 1) {
    /*fill in DMI info*/
    abf_dmi_info = &sub_module_output->
      meta_dump_params->meta_entry[ISP_META_ABF_TBL];
      /* v3 */
      abf_dmi_info->len =
        sizeof(signal2_lut_t) * 2 +
        sizeof(noise_std2_lut_t) * 2 +
        sizeof(abf_mesh_lut_t);
      /*dmi type */
      abf_dmi_info->dump_type  = ISP_META_ABF_TBL;
      abf_dmi_info->start_addr = 0;
      sub_module_output->meta_dump_params->frame_meta.num_entry++;

      memcpy(abf_dmi_info->isp_meta_dump,
        &abf->abf3_parms.noise_std2_lut_lv0,
        sizeof(noise_std2_lut_t));
      memcpy(abf_dmi_info->isp_meta_dump +
        sizeof(noise_std2_lut_t),
        &abf->abf3_parms.noise_std2_lut_lv1,
        sizeof(noise_std2_lut_t));
      memcpy(abf_dmi_info->isp_meta_dump +
        sizeof(noise_std2_lut_t) * 2,
        &abf->abf3_parms.signal2_lut_lv0,
        sizeof(signal2_lut_t));
      memcpy(abf_dmi_info->isp_meta_dump +
        (sizeof(noise_std2_lut_t) * 2) +
        sizeof(signal2_lut_t),
        &abf->abf3_parms.signal2_lut_lv1,
        sizeof(signal2_lut_t));
      memcpy(abf_dmi_info->isp_meta_dump +
        (sizeof(noise_std2_lut_t) * 2) +
        (sizeof(signal2_lut_t) * 2),
        &abf->abf3_parms.abf_mesh_lut,
        sizeof(abf_mesh_lut_t));
  }

FILL_METADATA:
  if (sub_module_output->frame_meta) {
    sub_module_output->enableLPM = !(isp_sub_module->submod_enable);
    sub_module_output->frame_meta->abf_mode =
      isp_sub_module->manual_ctrls.abf_mode;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
}

/** abf47_decide_hysterisis:
 *
 *  @module:  module instance
 *  @isp_sub_module: sub module handle
 *  @event: mct event
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean abf47_decide_hysterisis(
  isp_sub_module_t *isp_sub_module, float aec_ref)
{
  chromatix_parms_type *chromatix_ptr  = NULL;
  chromatix_ABF3_type  *chromatix_ABF3 = NULL;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_ABF3 =
    &chromatix_ptr->chromatix_VFE.chromatix_ABF3;

  return isp_sub_module_util_decide_hysterisis(isp_sub_module,
    aec_ref, chromatix_ABF3->abf3_enable,
    chromatix_ABF3->abf3_snapshot_hyst_en_flag,
    chromatix_ABF3->control_abf3, &chromatix_ABF3->abf3_hysteresis_trigger);

}

/** abf47_aec_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: module event data
 *
 * Handle AEC update event
 *
 * Return TRUE on success and FALSE on failure
 **/
boolean abf47_aec_update(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  stats_update_t              *stats_update         = NULL;
  aec_manual_update_t         *stats_manual_update  = NULL;
  abf47_t                     *abf                  = NULL;
  chromatix_parms_type        *chromatix_ptr        = NULL;
  chromatix_ABF3_type         *chromatix_ABF3       = NULL;
  Chromatix_HDR_ABF3_adj_type *chromatix_ABF3_type2 = NULL;
  trigger_point_type          *out_trigger_point    = NULL;
  trigger_point_type          *low_trigger_point    = NULL;

  float   aec_reference     = 0.0f;
  float   aec_reference_adj = 0.0f;
  float   ratio             = 0.0f;
  float   ratio_adj         = 0.0f;
  float   start             = 0.0f;
  float   end               = 0.0f;
  boolean ret               = TRUE;
  int     i                 = 0;
  uint8_t trigger_index     = MAX_LIGHT_TYPES_FOR_SPATIAL_ABF + 1;
  uint8_t trigger_index_adj = MAX_SETS_FOR_TONE_NOISE_ADJ + 1;

  trigger_ratio_t aec_ratio;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  abf = (abf47_t *)isp_sub_module->private_data;
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


  if (!stats_update && !stats_manual_update ) {
    ISP_ERR("failed: stats_update %p stats_manual_update %p",
      stats_update, stats_manual_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);
  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("failed: chromatix_ptr %p", chromatix_ptr);
    goto ERROR;
  }

  chromatix_ABF3       = &chromatix_ptr->chromatix_VFE.chromatix_ABF3;
  chromatix_ABF3_type2 = &chromatix_ptr->chromatix_VFE.chromatix_hdr_abf3_adj_data;

  if (stats_update) {
    abf->aec_update = stats_update->aec_update;
    if (chromatix_ABF3->control_abf3 == CONTROL_LUX_IDX) {
      /* lux index based */
    aec_reference = stats_update->aec_update.lux_idx;
    } else {
      /* Gain based */
    aec_reference = stats_update->aec_update.sensor_gain;
    }

    if (chromatix_ABF3_type2->control_hdr_abf3 == CONTROL_LUX_IDX) {
      /* lux index based */
      aec_reference = stats_update->aec_update.lux_idx;
    } else if (chromatix_ABF3_type2->control_hdr_abf3 == CONTROL_GAIN) {
        /* Gain based */
      aec_reference_adj = stats_update->aec_update.sensor_gain;
    } else if (chromatix_ABF3_type2->control_hdr_abf3 == CONTROL_AEC_EXP_SENSITIVITY_RATIO) {
      /* Exposure sensitivity ratio based */
      aec_reference_adj = stats_update->aec_update.hdr_sensitivity_ratio;
    } else if (chromatix_ABF3_type2->control_hdr_abf3 == CONTROL_EXP_TIME_RATIO) {
      /* Exposure time ratio based */
      aec_reference_adj = stats_update->aec_update.hdr_exp_time_ratio;
    } else if (chromatix_ABF3_type2->control_hdr_abf3 == CONTROL_DRC_GAIN) {
      /* DRC gain based */
      aec_reference_adj = stats_update->aec_update.total_drc_gain;
    }
  } else if (stats_manual_update) {
    abf->aec_manual_update = *stats_manual_update;
    if (chromatix_ABF3->control_abf3 == CONTROL_LUX_IDX) {
      /* lux index based */
      aec_reference = stats_manual_update->lux_idx;
    } else {
      /* Gain based */
      aec_reference = stats_manual_update->sensor_gain;
    }
  }

  ISP_HIGH("aec_reference :%f, aec_reference_adj :%f",
           aec_reference, aec_reference_adj);

  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL_ABF; i++) {
    trigger_point_type *abf3_trigger =
      &(chromatix_ABF3->abf3_config[i].ABF3_trigger);

    trigger_index = i;

    if (i == MAX_LIGHT_TYPES_FOR_SPATIAL_ABF - 1) {
      /* falls within region 6 but we do not use trigger points in the region */
      ratio         = 0;
      trigger_index = MAX_LIGHT_TYPES_FOR_SPATIAL_ABF - 1;
      break;
    }

    if (chromatix_ABF3->control_abf3 == CONTROL_LUX_IDX) {
      start = abf3_trigger->lux_index_start;
      end   = abf3_trigger->lux_index_end;
      ISP_HIGH("lux_start :%f, lux_end :%f", start, end);
    } else {
      start = abf3_trigger->gain_start;
      end   = abf3_trigger->gain_end;
      ISP_HIGH("gain_start :%f, gain_end :%f", start, end);
    }

    if (aec_reference < start) {
      ratio = 0.0f;
      break;
    }

    /* index is within interpolation range, find ratio */
     if (aec_reference >= start && aec_reference < end) {
       ratio         = (aec_reference - start)/(end - start);
       ISP_DBG("%s [%f - %f - %f] = %f", __func__, start, aec_reference, end,
         ratio);
       break;
     }
  }

  for (i = 0; i < MAX_SETS_FOR_TONE_NOISE_ADJ; i++) {
    trigger_point2_type *abf3_trigger_type2 = NULL;

    trigger_index_adj = i;

    if (i == MAX_SETS_FOR_TONE_NOISE_ADJ - 1) {
      /* falls within region 6 but we do not use trigger points in the region */
      trigger_index_adj = MAX_SETS_FOR_TONE_NOISE_ADJ - 1;
      ratio_adj         = 0;
      break;
    }

    if (chromatix_ABF3_type2->control_hdr_abf3 == CONTROL_AEC_EXP_SENSITIVITY_RATIO) {
      abf3_trigger_type2 = &(chromatix_ABF3_type2->hdr_abf3_adj_core_data[i].aec_sensitivity_ratio);
    } else if (chromatix_ABF3_type2->control_hdr_abf3 == CONTROL_EXP_TIME_RATIO) {
      abf3_trigger_type2 = &(chromatix_ABF3_type2->hdr_abf3_adj_core_data[i].exp_time_trigger);
    }

    if (abf3_trigger_type2) {
      start = abf3_trigger_type2->start;
      end   = abf3_trigger_type2->end;
    }

    ISP_HIGH("start :%f, end :%f", start, end);

    if (aec_reference_adj < start) {
      ratio_adj = 0.0f;
      break;
    }

    /* index is within interpolation range, find ratio */
     if (aec_reference_adj >= start && aec_reference_adj < end) {
       ratio_adj         = (aec_reference_adj - start)/(end - start);
       ISP_DBG("%s [%f - %f - %f] = %f", __func__, start, aec_reference_adj, end,
         ratio);
       break;
     }
  }

  if (trigger_index >= MAX_LIGHT_TYPES_FOR_SPATIAL_ABF) {
    ISP_ERR("no trigger match for ABF trigger value: %f\n", aec_reference);
    goto ERROR;
  }

  if (trigger_index_adj >= MAX_SETS_FOR_TONE_NOISE_ADJ) {
    ISP_ERR("no trigger match for ABF trigger value: %f\n", aec_reference_adj);
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

  abf->aec_reference     = aec_reference;
  abf->aec_reference_adj = aec_reference_adj;

  abf47_decide_hysterisis(isp_sub_module, aec_reference);

  ISP_DBG("%s [%f - %f - %f] = %f trigger index %u", __func__,
    start, aec_reference, end, ratio, trigger_index);

  if ((trigger_index != abf->trigger_index) ||
    (!F_EQUAL(ratio, abf->aec_ratio.ratio))) {
    abf->trigger_index = trigger_index;
    abf->aec_ratio.ratio = ratio;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  if ((trigger_index_adj != abf->trigger_index_adj) ||
    (!F_EQUAL(ratio_adj, abf->aec_ratio.ratio_adj))) {
    abf->trigger_index_adj   = trigger_index_adj;
    abf->aec_ratio.ratio_adj = ratio_adj;
    isp_sub_module->trigger_update_pending = TRUE;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
}

/** abf47_set_chromatix_ptr:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: module event data
 *
 *  This function makes initial configuration
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean abf47_set_chromatix_ptr(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean              ret = TRUE;
  abf47_t *abf = NULL;
  modulesChromatix_t  *chromatix_ptrs = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  abf = (abf47_t *)isp_sub_module->private_data;
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
  ret = isp_sub_module_util_configure_from_chromatix_bit(isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed: updating module enable bit for hw %d",
      isp_sub_module->hw_module_id);
  }
  ret = abf47_init_default(isp_sub_module, abf);
  if (ret == FALSE) {
    ISP_ERR("failed: abf47_config ret %d", ret);
  } else {
    isp_sub_module->trigger_update_pending = TRUE;
  }

  return ret;
} /* abf_set_chromatix_ptr */

/** abf47_streamon:
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
boolean abf47_streamon(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  boolean ret  = TRUE;
  abf47_t *abf = NULL;

  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p %p", module, isp_sub_module, event);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (!isp_sub_module->stream_on_count) {
    abf = (abf47_t *)isp_sub_module->private_data;
    if (!abf) {
      ISP_ERR("failed: mod %p", abf);
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return FALSE;
    }
    if (!abf47_config_reserve(isp_sub_module, abf)) {
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return FALSE;
    }
  }

  if (isp_sub_module->stream_on_count++) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->config_pending = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

} /*abf47_streamon */

/** abf47_streamoff:
 *
 *  @module: mct module handle
 *  @isp_sub_module: isp sub module handle
 *  @event: module event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean abf47_streamoff(mct_module_t *module,
  isp_sub_module_t *isp_sub_module, mct_event_t *event)
{
  abf47_t *abf = NULL;
  if (!module || !isp_sub_module || !event) {
    ISP_ERR("failed: %p %p", isp_sub_module, event);
    return FALSE;
  }

  abf = (abf47_t *)isp_sub_module->private_data;
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
} /* abf47_streamoff */

/** abf47_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the abf module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean abf47_init(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  abf47_t *abf = NULL;

  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return FALSE;
  }

  abf = (abf47_t *)CAM_MALLOC(sizeof(abf47_t));
  if (!abf) {
    ISP_ERR("failed: abf %p", abf);
    return FALSE;
  }

  memset(abf, 0, sizeof(*abf));

  isp_sub_module->private_data = (void *)abf;
  FILL_FUNC_TABLE(abf);
  isp_sub_module->manual_ctrls.abf_mode = CAM_NOISE_REDUCTION_MODE_FAST;
  return TRUE;
}/* abf47_init */

/** abf47_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void abf47_destroy(mct_module_t *module, isp_sub_module_t *isp_sub_module)
{
  if (!module || !isp_sub_module) {
    ISP_ERR("failed: module %p isp_sub_module %p", module, isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* abf47_destroy */

#if !OVERRIDE_FUNC
static ext_override_func abf_override_func = {
  .get_trigger_index = NULL,
};

boolean abf47_fill_func_table(abf47_t *abf)
{
  abf->ext_func_table = &abf_override_func;
  return TRUE;
} /* abf47_fill_func_table */
#endif
