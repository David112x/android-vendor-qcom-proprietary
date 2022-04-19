/* linearization40.c
 *
 * Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
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
#include "linearization40.h"
#include "isp_pipeline_reg.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_LINEARIZATION, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_LINEARIZATION, fmt, ##args)

#define LINEAR_INTERPOLATION_LINEARIZATION(v1, v2, ratio) \
 ((float)(v2 + (v1 - v2) * ratio))

#define TBL_INTERPOLATE_LINEARIZATION(in1, in2, out, ratio, size, i) ({\
  for (i=0; i<size; i++) \
    out[i] = LINEAR_INTERPOLATION_LINEARIZATION(in1[i], in2[i], ratio); })

static const char * const aec_debug_str[] = {
  "LINEAR_AEC_BRIGHT",
  "LINEAR_AEC_BRIGHT_NORMAL",
  "LINEAR_AEC_NORMAL",
  "LINEAR_AEC_NORMAL_LOW",
  "LINEAR_AEC_LOW",
  "LINEAR_AEC_LUX_MAX",
};

static const char * const awb_debug_str[] = {
  "AWB_CCT_TYPE_D65",
  "AWB_CCT_TYPE_D65_TL84",
  "AWB_CCT_TYPE_TL84",
  "AWB_CCT_TYPE_TL84_A",
  "AWB_CCT_TYPE_A",
  "AWB_CCT_TYPE_MAX",
};

/** linearization40_print_tbl
 *
 *  @cmd: handle to ISP_LinearizationCmdType
 *
 *  Print linearization struct info for debug
 *
 *  Returns none
 **/
static void linearization40_print_tbl(ISP_LinearizationCmdType *cmd)
{
  int i = 0;
  if (!cmd) {
    ISP_ERR("failed: cmd %p", cmd);
    return;
  }
  ISP_DBG("Linearization Tbl");

  for (i = 0; i < ISP32_LINEARIZATON_TABLE_LENGTH; i++) {
    ISP_DBG("linear_tbl[%d]= "LINEAR_TABLE_FORMAT"", i, cmd->CfgTbl.Lut[i]);
  }
}

/** linearization40_print_config
 *
 *  @cmd: handle to ISP_LinearizationCmdType
 *
 *  Print linearization struct info for debug
 *
 *  Returns none
 **/
static void linearization40_print_config(ISP_LinearizationCmdType *cmd)
{
  if (!cmd) {
    ISP_ERR("failed: cmd %p", cmd);
    return;
  }
  ISP_DBG("Linearization configurations");

  ISP_DBG("pointSlopeR.kneePoint_P0 = %d",
    cmd->CfgParams.pointSlopeR.kneePoint_P0);
  ISP_DBG("pointSlopeR.kneePoint_P1 = %d",
    cmd->CfgParams.pointSlopeR.kneePoint_P1);
  ISP_DBG("pointSlopeR.kneePoint_P2 = %d",
    cmd->CfgParams.pointSlopeR.kneePoint_P2);
  ISP_DBG("pointSlopeR.kneePoint_P3 = %d",
    cmd->CfgParams.pointSlopeR.kneePoint_P3);
  ISP_DBG("pointSlopeR.kneePoint_P4 = %d",
    cmd->CfgParams.pointSlopeR.kneePoint_P4);
  ISP_DBG("pointSlopeR.kneePoint_P5 = %d",
    cmd->CfgParams.pointSlopeR.kneePoint_P5);
  ISP_DBG("pointSlopeR.kneePoint_P6 = %d",
    cmd->CfgParams.pointSlopeR.kneePoint_P6);
  ISP_DBG("pointSlopeR.kneePoint_P7 = %d",
    cmd->CfgParams.pointSlopeR.kneePoint_P7);

  ISP_DBG("pointSlopeGb.kneePoint_P0 = %d",
    cmd->CfgParams.pointSlopeGb.kneePoint_P0);
  ISP_DBG("pointSlopeGb.kneePoint_P1 = %d",
    cmd->CfgParams.pointSlopeGb.kneePoint_P1);
  ISP_DBG("pointSlopeGb.kneePoint_P2 = %d\n",
    cmd->CfgParams.pointSlopeGb.kneePoint_P2);
  ISP_DBG("pointSlopeGb.kneePoint_P3 = %d\n",
    cmd->CfgParams.pointSlopeGb.kneePoint_P3);
  ISP_DBG("pointSlopeGb.kneePoint_P4 = %d\n",
    cmd->CfgParams.pointSlopeGb.kneePoint_P4);
  ISP_DBG("pointSlopeGb.kneePoint_P5 = %d\n",
    cmd->CfgParams.pointSlopeGb.kneePoint_P5);
  ISP_DBG("pointSlopeGb.kneePoint_P6 = %d\n",
    cmd->CfgParams.pointSlopeGb.kneePoint_P6);
  ISP_DBG("pointSlopeGb.kneePoint_P7 = %d\n",
    cmd->CfgParams.pointSlopeGb.kneePoint_P7);

  ISP_DBG("pointSlopeB.kneePoint_P0 = %d",
    cmd->CfgParams.pointSlopeB.kneePoint_P0);
  ISP_DBG("pointSlopeB.kneePoint_P1 = %d",
    cmd->CfgParams.pointSlopeB.kneePoint_P1);
  ISP_DBG("pointSlopeB.kneePoint_P2 = %d",
    cmd->CfgParams.pointSlopeB.kneePoint_P2);
  ISP_DBG("pointSlopeB.kneePoint_P3 = %d",
    cmd->CfgParams.pointSlopeB.kneePoint_P3);
  ISP_DBG("pointSlopeB.kneePoint_P4 = %d",
    cmd->CfgParams.pointSlopeB.kneePoint_P4);
  ISP_DBG("pointSlopeB.kneePoint_P5 = %d",
    cmd->CfgParams.pointSlopeB.kneePoint_P5);
  ISP_DBG("pointSlopeB.kneePoint_P6 = %d",
    cmd->CfgParams.pointSlopeB.kneePoint_P6);
  ISP_DBG("pointSlopeB.kneePoint_P7 = %d",
    cmd->CfgParams.pointSlopeB.kneePoint_P7);

  ISP_DBG("pointSlopeGr.kneePoint_P0 = %d",
    cmd->CfgParams.pointSlopeGr.kneePoint_P0);
  ISP_DBG("pointSlopeGr.kneePoint_P1 = %d",
    cmd->CfgParams.pointSlopeGr.kneePoint_P1);
  ISP_DBG("pointSlopeGr.kneePoint_P2 = %d",
    cmd->CfgParams.pointSlopeGr.kneePoint_P2);
  ISP_DBG("pointSlopeGr.kneePoint_P3 = %d",
    cmd->CfgParams.pointSlopeGr.kneePoint_P3);
  ISP_DBG("pointSlopeGr.kneePoint_P4 = %d",
    cmd->CfgParams.pointSlopeGr.kneePoint_P4);
  ISP_DBG("pointSlopeGr.kneePoint_P5 = %d",
    cmd->CfgParams.pointSlopeGr.kneePoint_P5);
  ISP_DBG("pointSlopeGr.kneePoint_P6 = %d",
    cmd->CfgParams.pointSlopeGr.kneePoint_P6);
  ISP_DBG("pointSlopeGr.kneePoint_P7 = %d",
    cmd->CfgParams.pointSlopeGr.kneePoint_P7);

} /* linearization40_print_config */

/** linearization40_ez_isp_update
 *
 *  @linear_mod
 *  @linDiag
 *
 **/
static void linearization40_ez_isp_update(
  linearization40_t* mod, linearization_t *linDiag)
{
  ISP_LinearizationLut *linCfg;
  linCfg = &(mod->applied_linear_lut);

  memcpy(linDiag, linCfg, sizeof(linearization_t));
}/* linearization40_ez_isp_update */


/** linearization40_fill_vfe_diag_data:
 *
 *  @linearization: linearization module instance
 *
 *  This function fills vfe diagnostics information
 *
 *  Return: TRUE success
 **/
static boolean linearization40_fill_vfe_diag_data(linearization40_t *mod,
  isp_sub_module_t *isp_sub_module, isp_sub_module_output_t *sub_module_output)
{
  boolean                 ret = TRUE;
  linearization_t        *linear_diag = NULL;
  vfe_diagnostics_t      *vfe_diag = NULL;

  if (sub_module_output->frame_meta) {
    sub_module_output->frame_meta->vfe_diag_enable =
      isp_sub_module->vfe_diag_enable;
    vfe_diag = &sub_module_output->frame_meta->vfe_diag;
    linear_diag = &vfe_diag->prev_linear;

    linearization40_ez_isp_update(mod, linear_diag);
  }
  return ret;
}/*linearization40_fill_vfe_diag_data*/

/** linearization40_copy_table_to_float
 *
 *  @input: chromatix where values are unsigned int
 *  @output: converted input int to float
 *
 *  Covert chromatix uinsigned int to float
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean linearization40_copy_table_to_float(
  chromatix_linearization_type * input, Linearization_high_res_Lut_t *output)
{
  int i;
  for (i = 0; i < 8; i++) {
    output->r_lut_p[i] = (float)input->r_lut_p[i];
    output->gr_lut_p[i] = (float)input->gr_lut_p[i];
    output->gb_lut_p[i] = (float)input->gb_lut_p[i];
    output->b_lut_p[i] = (float)input->b_lut_p[i];
    output->r_lut_base[i] = (float)input->r_lut_base[i];
    output->gr_lut_base[i] = (float)input->gr_lut_base[i];
    output->gb_lut_base[i] = (float)input->gb_lut_base[i];
    output->b_lut_base[i] = (float)input->b_lut_base[i];
  }
  output->r_lut_base[8] = (float)input->r_lut_base[8];
  output->gr_lut_base[8] = (float)input->gr_lut_base[8];
  output->gb_lut_base[8] = (float)input->gb_lut_base[8];
  output->b_lut_base[8] = (float)input->b_lut_base[8];

  return TRUE;
}

/** linearization40_compute_delta:
 *
 *  @lut_p: handle to chromatix_linearization_type
 *  @base: base for the linearization curve
  *  @delta: delta for the linearization curve
 *
 *  convert lut_delta to HW format(Q9) fill in reg cmd with
 *  lut_base, slope and delta
 *
 *  Return TRUE on success
 **/
static boolean linearization40_compute_delta(unsigned int *delta,
  float *base, float *lut_p, int is_unity_slope)
{
  boolean                  ret = TRUE;
  int                      i;
  float                    del = 0.0f;

  if (!delta|| !base || !lut_p) {
    ISP_ERR("failed: %p %p %p", delta, base, lut_p);
    return FALSE;
  }

  if(lut_p[0] == 0)
    delta[0] = (1 << DELATQ_FACTOR);
  else {
    del = (float)(base[1]-base[0]) / (float)lut_p[0];
    delta[0] = FLOAT_TO_Q(DELATQ_FACTOR, del);
  }

  for (i = 1; i < 8; i++ ) {
    del = (float)(base[i+1] - base[i]) / (float)(lut_p[i] - lut_p[i-1]);
    delta[i] = FLOAT_TO_Q(DELATQ_FACTOR, del);
  }
  if (lut_p[7] < LINEAR_MAX_VAL) {
    if (is_unity_slope) {
      delta[8] = 1 << DELATQ_FACTOR;
    } else {
      del = (float)(LINEAR_MAX_VAL - base[8]) /
            (float)(LINEAR_MAX_VAL - lut_p[7]);;
      delta[8] = FLOAT_TO_Q(DELATQ_FACTOR, del);
    }
  } else {
     if (is_unity_slope)
       delta[8] = 1 << DELATQ_FACTOR;
     else
    delta[8] = 0;
  }
  return ret;
}

/** linearization40_compute_base
 *
 *  @input_x: handle to chromatix_linearization_type
 *  @input_base: base for the linearization curve
  *  @output_x: delta for the linearization curve
  *..@baseTmp
 *
 *  convert lut_delta to HW format(Q9) fill in reg cmd with
 *  lut_base, slope and delta
 *
 *  Return none
 **/
void linearization40_compute_base(
    float *input_x,
    float *input_base,
    float *output_x,
    float *baseTmp)
{
    float x1, x2, y1, y2;
    int j = 0;

    if (*output_x < input_x[0]) {
      x1 = 0;
      x2 = input_x[0];
      y1 = input_base[0];
      y2 = input_base[1];
    }
    else if (*output_x > input_x[7] ||
        F_EQUAL(*output_x,input_x[7])) {
      x1 = input_x[7];
      x2 = (float)LINEAR_MAX_VAL;
      y1 = input_base[8];
      y2 = (float)LINEAR_MAX_VAL;
    }
    else {
      for (j = 0; j <7; j++) {
        x1 = input_x[j];
        x2 = input_x[j+1];
        if(*output_x >= x1 && *output_x < x2)
          break;
      }
      ++j;
      y1 = input_base[j];
      y2 = input_base[j+1];
    }
    if (!F_EQUAL(x1, x2)) {
      *baseTmp = (y1 + (*output_x - x1) * ((y2 - y1)/(x2 - x1)));
      ISP_DBG("%3.9f = (%3.9f + (%3.9f - %3.9f) * "
         "((%3.9f - %3.9f)/(%3.9f - %3.9f))  j = %d", *baseTmp/4, y1/4,
         *output_x/4, x1/4, y2/4, y1/4,x2/4, x1/4,j);
    } else
      *baseTmp = (float)LINEAR_MAX_VAL;
}

/** linearization40_compute_knee_points
 *
 *  @(x1,y1), (x2,y2): input (x,y) cartesian coordinates
 *  @(xnew, ynew): output cartesian coordinates
 *
 *  From the line formed from points (x1,y1) (x2,y2) find
 *  the y0 for the corresponding x0.
 *
 *  Return None
 **/
static void linearization40_compute_knee_points(float *x1, float *y1, float *x2,
  float *y2, float *xnew, float *ynew, float ratio, float multiplier)
{
  int i = 0;
  float y1New, y2New, xTmp, yTmp;
  float x1Tmp[9], x2Tmp[9], y1Tmp[10], y2Tmp[10];

  for(i = 0; i < 8; i++) {
    x1Tmp[i] = x1[i] * multiplier;
    x2Tmp[i] = x2[i] * multiplier;
  }

  for(i = 0; i < 9; i++) {
   y1Tmp[i] = y1[i] * multiplier;
   y2Tmp[i] = y2[i] * multiplier;
  }

  ynew[0] =0;

  for(i = 0; i < 8; i++) {
    xTmp = LINEAR_INTERPOLATION_LINEARIZATION(x1Tmp[i], x2Tmp[i], ratio);
    linearization40_compute_base(x1Tmp, y1Tmp, &xTmp, &y1New);
    linearization40_compute_base(x2Tmp, y2Tmp, &xTmp, &y2New);
    xnew[i] = xTmp;
    yTmp = LINEAR_INTERPOLATION_LINEARIZATION(y1New, y2New, ratio);
    ynew[i+1] = yTmp;
  }
  if (!F_EQUAL(xnew[0], ynew[1])) {
    ynew[1] = 0;
  }
}

/** linearization40_convert_tbl
 *
 *  @tbl: handle to chromatix_linearization_type
 *  @pBlkTbl: handle to ISP_LinearizationLut
 *
 *  convert lut_delta to HW format(Q9) fill in reg cmd with
 *  lut_base, slope and delta
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean linearization40_convert_tbl(Linearization_high_res_Lut_t *tbl,
  ISP_LinearizationLut *pBlkTbl, int is_unity_slope)
{
  uint16_t i = 0;
  boolean ret = TRUE;

  if (!tbl || !pBlkTbl) {
    ISP_ERR("failed: %p %p", tbl, pBlkTbl);
    return FALSE;
  }

  for ( i = 0 ; i < 8 ; i++ ) {
    pBlkTbl->r_lut_p[i] = Round(tbl->r_lut_p[i]);
    pBlkTbl->gr_lut_p[i] = Round(tbl->gr_lut_p[i]);
    pBlkTbl->gb_lut_p[i] = Round(tbl->gb_lut_p[i]);
    pBlkTbl->b_lut_p[i] = Round(tbl->b_lut_p[i]);
  }

  tbl->r_lut_base[0] = tbl->r_lut_base[0] < LINEAR_MAX_VAL ?
                           tbl->r_lut_base[0] : LINEAR_MAX_VAL;
  pBlkTbl->r_lut_base[0] = Round(tbl->r_lut_base[0]);
  tbl->gr_lut_base[0] = tbl->gr_lut_base[0] < LINEAR_MAX_VAL ?
                          tbl->gr_lut_base[0] : LINEAR_MAX_VAL;
  pBlkTbl->gr_lut_base[0] = Round(tbl->gr_lut_base[0]);
  tbl->b_lut_base[0] = tbl->b_lut_base[0] < LINEAR_MAX_VAL ?
                          tbl->b_lut_base[0] : LINEAR_MAX_VAL;
  pBlkTbl->b_lut_base[0] = Round(tbl->b_lut_base[0]);
  tbl->gb_lut_base[0] = tbl->gb_lut_base[0] < LINEAR_MAX_VAL ?
                         tbl->gb_lut_base[0] : LINEAR_MAX_VAL;
  pBlkTbl->gb_lut_base[0] = Round(tbl->gb_lut_base[0]);

  for ( i = 1 ; i < 8 ; i++ ) {
    pBlkTbl->r_lut_base[i] = Round(tbl->r_lut_base[i]);
    pBlkTbl->gr_lut_base[i] = Round(tbl->gr_lut_base[i]);
    pBlkTbl->gb_lut_base[i] = Round(tbl->gb_lut_base[i]);
    pBlkTbl->b_lut_base[i] = Round(tbl->b_lut_base[i]);
  }

  tbl->r_lut_base[8] = tbl->r_lut_base[8] < LINEAR_MAX_VAL ? tbl->r_lut_base[8] :
    LINEAR_MAX_VAL;
  pBlkTbl->r_lut_base[8] = Round(tbl->r_lut_base[8]);
  tbl->gr_lut_base[8] = tbl->gr_lut_base[8] < LINEAR_MAX_VAL ? tbl->gr_lut_base[8] :
    LINEAR_MAX_VAL;
  pBlkTbl->gr_lut_base[8] = Round(tbl->gr_lut_base[8]);
  tbl->b_lut_base[8] = tbl->b_lut_base[8] < LINEAR_MAX_VAL ? tbl->b_lut_base[8] :
    LINEAR_MAX_VAL;
  pBlkTbl->b_lut_base[8] = Round(tbl->b_lut_base[8]);
  tbl->gb_lut_base[8] = tbl->gb_lut_base[8] < LINEAR_MAX_VAL ? tbl->gb_lut_base[8] :
    LINEAR_MAX_VAL;
  pBlkTbl->gb_lut_base[8] = Round(tbl->gb_lut_base[8]);

  ret = linearization40_compute_delta(pBlkTbl->r_lut_delta,
    tbl->r_lut_base, tbl->r_lut_p, is_unity_slope);
  GOTO_ERROR_IF_FALSE(ret);
  ret = linearization40_compute_delta(pBlkTbl->gr_lut_delta,
    tbl->gr_lut_base, tbl->gr_lut_p, is_unity_slope);
  GOTO_ERROR_IF_FALSE(ret);
  ret = linearization40_compute_delta(pBlkTbl->gb_lut_delta,
    tbl->gb_lut_base, tbl->gb_lut_p, is_unity_slope);
  GOTO_ERROR_IF_FALSE(ret);
  ret = linearization40_compute_delta(pBlkTbl->b_lut_delta,
    tbl->b_lut_base, tbl->b_lut_p, is_unity_slope);
  GOTO_ERROR_IF_FALSE(ret);

  return TRUE;
error:
  return FALSE;
} /* linearization40_convert_tbl */

/** linearization40_write_table
 *
 *  @pIn: handle to ISP_LinearizationLut
 *  @blkConfigCmd: handle to ISP_LinearizationCmdType
 *
 *  fill in DMI table HW format into reg cmd
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean linearization40_write_table32(const ISP_LinearizationLut *pIn,
  ISP_LinearizationCmdType *blkConfigCmd)
{
  uint16_t i,j;
  uint32_t data = 0;

  if (!pIn || !blkConfigCmd) {
    ISP_ERR("failed: %p %p", pIn, blkConfigCmd);
    return FALSE;
  }

  const uint16_t* baseR = pIn->r_lut_base;
  const uint16_t* baseGr = pIn->gr_lut_base;
  const uint16_t* baseGb = pIn->gb_lut_base;
  const uint16_t* baseB = pIn->b_lut_base;

  const uint32_t* sR = pIn->r_lut_delta;
  const uint32_t* sGr = pIn->gr_lut_delta;
  const uint32_t* sGb = pIn->gb_lut_delta;
  const uint32_t* sB = pIn->b_lut_delta;

  /* pack the table in HW specific arrangement */
  for (i = 0, j = 0; i < ISP32_LINEARIZATON_TABLE_LENGTH/4; i++, j += 4) {
    blkConfigCmd->CfgTbl.Lut[j] = ((((uint32_t)(*baseR)) & 0x00000FFF) |
      ((*sR & 0x0003FFFF) << 12));
    baseR++;
    sR++;

    blkConfigCmd->CfgTbl.Lut[j+1] = ((((uint32_t)(*baseGr)) & 0x00000FFF) |
      ((*sGr & 0x0003FFFF) << 12));
    baseGr++;
    sGr++;

    blkConfigCmd->CfgTbl.Lut[j+2] = ((((uint32_t)(*baseGb)) & 0x00000FFF) |
      ((*sGb & 0x0003FFFF) << 12));
    baseGb++;
    sGb++;

    blkConfigCmd->CfgTbl.Lut[j+3] = ((((uint32_t)(*baseB)) & 0x00000FFF) |
      ((*sB & 0x0003FFFF) << 12));
    baseB++;
    sB++;
  }
  return TRUE;
} /* linearization40_write_table32 */

/** linearization40_write_table64
 *
 *  @pIn: handle to ISP_LinearizationLut
 *  @blkConfigCmd: handle to ISP_LinearizationCmdType
 *
 *  fill in DMI table HW format into reg cmd
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean linearization40_write_table64(const ISP_LinearizationLut *pIn,
  ISP_LinearizationCmdType *blkConfigCmd)
{
  uint16_t i,j;
  uint32_t data = 0;

  if (!pIn || !blkConfigCmd) {
    ISP_ERR("failed: %p %p", pIn, blkConfigCmd);
    return FALSE;
  }

  const uint16_t* baseR = pIn->r_lut_base;
  const uint16_t* baseGr = pIn->gr_lut_base;
  const uint16_t* baseGb = pIn->gb_lut_base;
  const uint16_t* baseB = pIn->b_lut_base;

  const uint32_t* sR = pIn->r_lut_delta;
  const uint32_t* sGr = pIn->gr_lut_delta;
  const uint32_t* sGb = pIn->gb_lut_delta;
  const uint32_t* sB = pIn->b_lut_delta;

  /* pack the table in HW specific arrangement */
  for (i = 0, j = 0; i < ISP32_LINEARIZATON_TABLE_LENGTH/4; i++, j += 4) {

    ISP_DBG("Index i = %d", i);
    blkConfigCmd->CfgTbl.Lut[j] = ((((uint64_t)(*baseR)) & 0x00003FFF) |
      (((uint64_t)(*sR) & 0x003FFFFF) << 14));
    ISP_DBG("R base = %x, R delta = %x", *baseR, *sR);
    baseR++;
    sR++;

    blkConfigCmd->CfgTbl.Lut[j+1] = ((((uint64_t)(*baseGr)) & 0x00003FFF) |
      (((uint64_t)(*sGr) & 0x003FFFFF) << 14));

    ISP_DBG("Gr base = %x, Gr delta = %x", *baseGr, *sGr);
    baseGr++;
    sGr++;

    blkConfigCmd->CfgTbl.Lut[j+2] = ((((uint64_t)(*baseGb)) & 0x00003FFF) |
      (((uint64_t)(*sGb) & 0x003FFFFF) << 14));
    ISP_DBG("Gb base = %x, Gb delta = %x", *baseGb, *sGb);
    baseGb++;
    sGb++;

    blkConfigCmd->CfgTbl.Lut[j+3] = ((((uint64_t)(*baseB)) & 0x00003FFF) |
      (((uint64_t)(*sB) & 0x003FFFFF) << 14));
    ISP_DBG("B base = %x, B delta = %x", *baseB, *sB);
    baseB++;
    sB++;
  }
  return TRUE;
} /* linearization40_write_table */

static boolean linearization40_limit_knee_point(ISP_LinearizationLut *linear_lut)
{
  uint8_t i = 0;

  for (i = 0; i < 8; i++) {
    linear_lut->b_lut_p[i] = MIN(linear_lut->b_lut_p[i], LINEAR_MAX_VAL);
    linear_lut->gb_lut_p[i] = MIN(linear_lut->gb_lut_p[i], LINEAR_MAX_VAL);
    linear_lut->r_lut_p[i] = MIN(linear_lut->r_lut_p[i], LINEAR_MAX_VAL);
    linear_lut->gr_lut_p[i] = MIN(linear_lut->gr_lut_p[i], LINEAR_MAX_VAL);
  }

  return TRUE;
}

/** linearization40_config_cmd -
 *
 *  @linearization: linearization handle
 *  @pTbl: handle to chromatix_linearization_type
 *
 *  default configuration from Chromatix Header
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean linearization40_config_cmd(
  linearization40_t *linearization,
  Linearization_high_res_Lut_t *pTbl)
{
  boolean                   ret = TRUE;
  int                       i = 0;
  int                       is_unity_slope;
  uint32_t                 *ptr = NULL;
  ISP_LinearizationCmdType *cmd = NULL;
  ISP_LinearizationLut     *linear_lut = NULL;

  if (!linearization || !pTbl) {
    ISP_ERR("failed: %p %p", linearization, pTbl);
    return FALSE;
  }

  linear_lut = &linearization->linear_lut;
  
  if ((linearization->ext_func_table && linearization->ext_func_table->apply_2d_interpolation)
    && (linearization->apply_hdr_effects)) {
    linearization->ext_func_table->apply_2d_interpolation(linearization);
  }
  is_unity_slope = linearization->last_region_unity_slope_enable;
  ret = linearization40_convert_tbl(pTbl, linear_lut, is_unity_slope);
  if (ret == FALSE) {
    ISP_ERR("failed: convert_linearization_tbl");
    return FALSE;
  }

  cmd =&linearization->linear_cmd;
  linearization40_limit_knee_point(linear_lut);
  
  /* Configure the Linearization */
  cmd->CfgParams.pointSlopeR.kneePoint_P0 = linear_lut->r_lut_p[0];
  cmd->CfgParams.pointSlopeR.kneePoint_P1 = linear_lut->r_lut_p[1];
  cmd->CfgParams.pointSlopeR.kneePoint_P2 = linear_lut->r_lut_p[2];
  cmd->CfgParams.pointSlopeR.kneePoint_P3 = linear_lut->r_lut_p[3];
  cmd->CfgParams.pointSlopeR.kneePoint_P4 = linear_lut->r_lut_p[4];
  cmd->CfgParams.pointSlopeR.kneePoint_P5 = linear_lut->r_lut_p[5];
  cmd->CfgParams.pointSlopeR.kneePoint_P6 = linear_lut->r_lut_p[6];
  cmd->CfgParams.pointSlopeR.kneePoint_P7 = linear_lut->r_lut_p[7];

  cmd->CfgParams.pointSlopeGb.kneePoint_P0 = linear_lut->gb_lut_p[0];
  cmd->CfgParams.pointSlopeGb.kneePoint_P1 = linear_lut->gb_lut_p[1];
  cmd->CfgParams.pointSlopeGb.kneePoint_P2 = linear_lut->gb_lut_p[2];
  cmd->CfgParams.pointSlopeGb.kneePoint_P3 = linear_lut->gb_lut_p[3];
  cmd->CfgParams.pointSlopeGb.kneePoint_P4 = linear_lut->gb_lut_p[4];
  cmd->CfgParams.pointSlopeGb.kneePoint_P5 = linear_lut->gb_lut_p[5];
  cmd->CfgParams.pointSlopeGb.kneePoint_P6 = linear_lut->gb_lut_p[6];
  cmd->CfgParams.pointSlopeGb.kneePoint_P7 = linear_lut->gb_lut_p[7];

  cmd->CfgParams.pointSlopeB.kneePoint_P0 = linear_lut->b_lut_p[0];
  cmd->CfgParams.pointSlopeB.kneePoint_P1 = linear_lut->b_lut_p[1];
  cmd->CfgParams.pointSlopeB.kneePoint_P2 = linear_lut->b_lut_p[2];
  cmd->CfgParams.pointSlopeB.kneePoint_P3 = linear_lut->b_lut_p[3];
  cmd->CfgParams.pointSlopeB.kneePoint_P4 = linear_lut->b_lut_p[4];
  cmd->CfgParams.pointSlopeB.kneePoint_P5 = linear_lut->b_lut_p[5];
  cmd->CfgParams.pointSlopeB.kneePoint_P6 = linear_lut->b_lut_p[6];
  cmd->CfgParams.pointSlopeB.kneePoint_P7 = linear_lut->b_lut_p[7];

  cmd->CfgParams.pointSlopeGr.kneePoint_P0 = linear_lut->gr_lut_p[0];
  cmd->CfgParams.pointSlopeGr.kneePoint_P1 = linear_lut->gr_lut_p[1];
  cmd->CfgParams.pointSlopeGr.kneePoint_P2 = linear_lut->gr_lut_p[2];
  cmd->CfgParams.pointSlopeGr.kneePoint_P3 = linear_lut->gr_lut_p[3];
  cmd->CfgParams.pointSlopeGr.kneePoint_P4 = linear_lut->gr_lut_p[4];
  cmd->CfgParams.pointSlopeGr.kneePoint_P5 = linear_lut->gr_lut_p[5];
  cmd->CfgParams.pointSlopeGr.kneePoint_P6 = linear_lut->gr_lut_p[6];
  cmd->CfgParams.pointSlopeGr.kneePoint_P7 = linear_lut->gr_lut_p[7];

  if (LINEAR_TABLE_WIDTH == 32 )
    ret = linearization40_write_table32(linear_lut, cmd);
  else if (LINEAR_TABLE_WIDTH  == 64)
    ret = linearization40_write_table64(linear_lut, cmd);
  if (ret == FALSE) {
    ISP_ERR("failed: write_linearization_table");
    return FALSE;
  }

  return TRUE;
} /* linearization40_config_cmd */

/** linearization40_interpolate_linear_table
 *
 *  @input1: handle to chromatix_linearization_type
 *  @input2: handle to chromatix_linearization_type
 *  @ratio: aec ratio
 *  @output: handle to chromatix_linearization_type
 *
 *  interpolate tables
 *
 *  Return none
 **/
static void linearization40_interpolate_linear_table(
  Linearization_high_res_Lut_t *input1, Linearization_high_res_Lut_t *input2,
  float ratio, Linearization_high_res_Lut_t *output,int multiFactor, boolean is_lsb_alligned)
{
  float multiplier = 1.0f;

  if (is_lsb_alligned){
    multiplier = 1.0f/(float)(1 << multiFactor);
  } else {
    multiplier = (float)(1 << multiFactor);
  }

  ISP_DBG("%s: R compute_knee_points \n", __func__);
  linearization40_compute_knee_points(input1->r_lut_p, input1->r_lut_base,
    input2->r_lut_p, input2->r_lut_base, output->r_lut_p, output->r_lut_base,
    ratio, multiplier);

  ISP_DBG("%s: Gr compute_knee_points \n", __func__);
  linearization40_compute_knee_points(input1->gr_lut_p, input1->gr_lut_base,
    input2->gr_lut_p, input2->gr_lut_base, output->gr_lut_p,
    output->gr_lut_base, ratio, multiplier);

  ISP_DBG("%s: B compute_knee_points \n", __func__);
  linearization40_compute_knee_points(input1->b_lut_p, input1->b_lut_base,
    input2->b_lut_p, input2->b_lut_base, output->b_lut_p, output->b_lut_base,
    ratio, multiplier);

  ISP_DBG("%s: Gb compute_knee_points \n", __func__);
  linearization40_compute_knee_points(input1->gb_lut_p, input1->gb_lut_base,
    input2->gb_lut_p, input2->gb_lut_base, output->gb_lut_p,
    output->gb_lut_base, ratio, multiplier);

} /* linearization40_interpolate_linear_table */

/** linearization40_BitWidthUpdate
 *
 *  @ip: Input chromatix table type
 *  @multifactor :  pipeline width mutlifactor
 *
 **/

void linearization40_BitWidthUpdate (Linearization_high_res_Lut_t *ip,
  int multifactor, boolean is_lsb_alligned)
{
    int i =0;
    float multiplier = 1.0f;

    if (is_lsb_alligned){
      multiplier = 1.0f/(float)(1 << multifactor);
    } else {
      multiplier = (float)(1 << multifactor);
    }
    for(i =0 ; i < 8 ; i++){
      ip->r_lut_p[i] *= multiplier;
      ip->b_lut_p[i] *= multiplier;
      ip->gb_lut_p[i] *= multiplier;
      ip->gr_lut_p[i] *= multiplier;
    }
    for(i =0 ; i < 9 ; i++){
      ip->r_lut_base[i] *= multiplier;
      ip->b_lut_base[i] *= multiplier;
      ip->gb_lut_base[i] *= multiplier;
      ip->gr_lut_base[i] *= multiplier;
    }

}

/** linearization40_store_hw_update
 *
 *  @isp_sub_module: isp sub module handle
 *  @lineariztion: linearization handle
 *
 *  Create hw update list and store in isp sub module
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean linearization40_store_hw_update(isp_sub_module_t *isp_sub_module,
  linearization40_t *linearization)
{
  boolean                      ret = TRUE;
  int                          i = 0;
  struct msm_vfe_cfg_cmd2     *cfg_cmd = NULL;
  struct msm_vfe_reg_cfg_cmd  *reg_cfg_cmd = NULL;
  uint32_t                     linear_lut_channel = 0;
  struct msm_vfe_cfg_cmd_list *hw_update = NULL;
  ISP_LinearizationCmdType    *linear_cmd = NULL;

  if (!isp_sub_module || !linearization) {
    ISP_ERR("failed: %p %p", isp_sub_module, linearization);
    return FALSE;
  }

  hw_update = (struct msm_vfe_cfg_cmd_list *)CAM_MALLOC(sizeof(*hw_update));
  if (!hw_update) {
    ISP_ERR("failed: hw_update %p", hw_update);
    return FALSE;
  }
  memset(hw_update, 0, sizeof(*hw_update));

  cfg_cmd = &hw_update->cfg_cmd;

  reg_cfg_cmd = (struct msm_vfe_reg_cfg_cmd *)CAM_MALLOC(6 * sizeof(*reg_cfg_cmd));
  if (!reg_cfg_cmd) {
    ISP_ERR("failed: reg_cfg_cmd %p", reg_cfg_cmd);
    goto ERROR_REG_CFG_CMD;
  }
  memset(reg_cfg_cmd, 0, sizeof(*reg_cfg_cmd));

  linear_cmd = (ISP_LinearizationCmdType *)CAM_MALLOC(sizeof(*linear_cmd));
  if (!linear_cmd) {
    ISP_ERR("failed: linear_cmd %p", linear_cmd);
    goto ERROR_LINEAR_CMD;
  }
  memset(linear_cmd, 0, sizeof(*linear_cmd));

  linear_lut_channel =
    (linearization->linear_cmd.CfgParams.lutBankSel == 0)?
      BLACK_LUT_RAM_BANK0 : BLACK_LUT_RAM_BANK1;

  /* prepare dmi_set and dmi_reset fields */
  linearization->linear_cmd.dmi_set[0] =
    ISP_DMI_CFG_DEFAULT + linear_lut_channel;
  linearization->linear_cmd.dmi_set[1] = 0;

  linearization->linear_cmd.dmi_reset[0] =
    ISP_DMI_CFG_DEFAULT + ISP_DMI_NO_MEM_SELECTED;
  linearization->linear_cmd.dmi_reset[1] = 0;

  /*prepare config cmd for hw update list*/
  *linear_cmd = linearization->linear_cmd;

  cfg_cmd->cfg_data = (void *)linear_cmd;
  cfg_cmd->cmd_len = sizeof(*linear_cmd);
  cfg_cmd->cfg_cmd = (void *)reg_cfg_cmd;
  cfg_cmd->num_cfg = 6;

  /* set dmi to proper linearization bank */
  reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd[0].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[0].u.rw_info.reg_offset = ISP_DMI_CFG_OFF;
  reg_cfg_cmd[0].u.rw_info.len = 1 * sizeof(uint32_t);

  reg_cfg_cmd[1].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[0].u.rw_info.cmd_data_offset +
      reg_cfg_cmd[0].u.rw_info.len;
  reg_cfg_cmd[1].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[1].u.rw_info.reg_offset = ISP_DMI_ADDR;
  reg_cfg_cmd[1].u.rw_info.len = 1 * sizeof(uint32_t);

  /* send dmi data */
  reg_cfg_cmd[2].cmd_type = DMI_WRITE_MODE;
  reg_cfg_cmd[2].u.dmi_info.lo_tbl_offset =
    reg_cfg_cmd[1].u.rw_info.cmd_data_offset +
      reg_cfg_cmd[1].u.rw_info.len;
  reg_cfg_cmd[2].u.dmi_info.hi_tbl_offset =
    reg_cfg_cmd[2].u.dmi_info.lo_tbl_offset + sizeof(uint32_t);
  reg_cfg_cmd[2].u.dmi_info.len = sizeof(DMI_WRITE_TYPE) *
    ISP32_LINEARIZATON_TABLE_LENGTH;

  /* reset dmi to no bank*/
  reg_cfg_cmd[3].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[2].u.dmi_info.lo_tbl_offset +
      reg_cfg_cmd[2].u.dmi_info.len;
  reg_cfg_cmd[3].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[3].u.rw_info.reg_offset = ISP_DMI_CFG_OFF;
  reg_cfg_cmd[3].u.rw_info.len = 1 * sizeof(uint32_t);

  reg_cfg_cmd[4].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[3].u.rw_info.cmd_data_offset +
      reg_cfg_cmd[3].u.rw_info.len;
  reg_cfg_cmd[4].cmd_type = VFE_WRITE_MB;
  reg_cfg_cmd[4].u.rw_info.reg_offset = ISP_DMI_ADDR;
  reg_cfg_cmd[4].u.rw_info.len = 1 * sizeof(uint32_t);

  /* linearization configuration */
  reg_cfg_cmd[5].u.rw_info.cmd_data_offset =
    reg_cfg_cmd[4].u.rw_info.cmd_data_offset +
      reg_cfg_cmd[4].u.rw_info.len;
  reg_cfg_cmd[5].cmd_type = VFE_WRITE;
  reg_cfg_cmd[5].u.rw_info.reg_offset = ISP_LINEARIZATION_OFF;
  reg_cfg_cmd[5].u.rw_info.len = ISP_LINEARIZATION_LEN * sizeof(uint32_t);

  linearization40_print_tbl(linear_cmd);
  linearization40_print_config(linear_cmd);

  if (LINEAR_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      LINEAR_CGC_OVERRIDE_REGISTER, LINEAR_CGC_OVERRIDE_BIT, TRUE);
    if (ret == FALSE) {
      ISP_ERR("failed: enable cgc");
    }
  }

  ISP_LOG_LIST("hw_update %p cfg_cmd %p", hw_update, cfg_cmd->cfg_cmd);
  ret = isp_sub_module_util_store_hw_update(isp_sub_module, hw_update);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_store_hw_update");
    goto ERROR_APPEND;
  }

  if (LINEAR_CGC_OVERRIDE == TRUE) {
    ret = isp_sub_module_util_update_cgc_mask(isp_sub_module,
      LINEAR_CGC_OVERRIDE_REGISTER, LINEAR_CGC_OVERRIDE_BIT, FALSE);
    if (ret == FALSE) {
      ISP_ERR("failed: disable cgc");
    }
  }

  /*SW LUT table for diag, HW LUT table for meta dump*/
  linearization->applied_linear_lut = linearization->linear_lut;
  linearization->applied_hw_lut = linearization->linear_cmd.CfgTbl;
  linearization->linear_cmd.CfgParams.lutBankSel ^= 1;

  return TRUE;

ERROR_APPEND:
  free(linear_cmd);
ERROR_LINEAR_CMD:
  free(reg_cfg_cmd);
ERROR_REG_CFG_CMD:
  free(hw_update);
  return FALSE;
} /* linearization40_store_hw_update */

/** linearization40_apply_aec_update
 *
 *  @lineariztion: linearization handle
 *  @output1: corresponding table from chromatix based on light
 *  @output2: corresponding table from chromatix based on light
 *
 *  Apply aec update and prepare the tables
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean linearization40_apply_aec_update(linearization40_t *linearization,
  Linearization_high_res_Lut_t *output1,
  Linearization_high_res_Lut_t *output2,
  Linearization_high_res_Lut_t *output_final)
{
  boolean                ret = TRUE;
  float                  aec_ratio = 0;

  aec_ratio = linearization->aec_ratio;
  ISP_DBG("aec_ratio = %f", aec_ratio);

  // TBD switched from output2, output1
  linearization40_interpolate_linear_table(output2, output1,
    aec_ratio, output_final, 0, FALSE);

  return ret;
}

/** linearization40_get_tables
 *
 *  @lineariztion: linearization handle
 *  @output1: corresponding table from chromatix based
 *                  on light
 *  @output2: corresponding table from chromatix based on
 *                light
 *  @isp_sub_module: isp sub module handle
 *
 *  Prepare/Get the proper tables
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean linearization40_get_tables(isp_sub_module_t *isp_sub_module,
  linearization40_t *linearization,
  Linearization_high_res_Lut_t *output1,
  Linearization_high_res_Lut_t *output2)
{
  boolean                       ret = TRUE;
  chromatix_parms_type         *pchromatix = NULL;
  chromatix_videoHDR_type      *chromatix_VHDR = NULL;
  chromatix_VFE_common_type    *pchromatix_common = NULL;
  int  multifactor = ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH;
  boolean                       is_lsb_alligned = FALSE;

  pchromatix =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;
  pchromatix_common = (chromatix_VFE_common_type *)
      isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!pchromatix || !pchromatix_common) {
    ISP_ERR("failed: %p %p", pchromatix, pchromatix_common);
    return FALSE;
  }

  /*when HDR mode, chromatix tuned by 14 bit,
    otherwise keep 12 bit header for backward compatible*/
  chromatix_VHDR =
    &pchromatix->chromatix_post_processing.chromatix_video_HDR;
  if (chromatix_VHDR->videoHDR_reserve_data.hdr_recon_en == 0 &&
      chromatix_VHDR->videoHDR_reserve_data.hdr_mac_en == 0) {
    multifactor = ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH;
  } else {
    multifactor = 0;
  }

  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR){
    multifactor = ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH;;
    is_lsb_alligned = TRUE;
  } else if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG){
    multifactor = ISP_PIPELINE_WIDTH - CHROMATIX_BITWIDTH;;
    is_lsb_alligned = FALSE;
  }

  if (!linearization40_get_right_tables (linearization, pchromatix_common,
        output1, output2, isp_sub_module->hdr_mode)) {
    linearization40_update_base_tables(linearization, pchromatix_common);
    linearization40_BitWidthUpdate(&linearization->linear_table_output1,
        multifactor, is_lsb_alligned);
    linearization40_BitWidthUpdate(&linearization->linear_table_output2,
        multifactor, is_lsb_alligned);
    *output1 = linearization->linear_table_output1;
    *output2 = linearization->linear_table_output2;
  }

  return ret;
}

/** linearization40_correct_for_pedestal
 *
 *  @linearization: linearization handle
 *  @ptable:  linearization table
 *
 *  correct table values if pedestal enabled
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean linearization40_correct_for_pedestal(linearization40_t *linearization,
  Linearization_high_res_Lut_t *ptable_in)
{
  boolean         ret = TRUE;
  int             i;

  if (!linearization || !ptable_in) {
    ISP_ERR("failed NULL input- linearization %p table_in %p ",
      linearization, ptable_in);
    return FALSE;
  }

  if (linearization->pedestal_enable) {
    linearization->blk_level_applied.r_val = ptable_in->r_lut_p[0];
    linearization->blk_level_applied.b_val = ptable_in->b_lut_p[0];
    linearization->blk_level_applied.gr_val = ptable_in->gr_lut_p[0];
    linearization->blk_level_applied.gb_val = ptable_in->gb_lut_p[0];
    for (i = 0; i < 8; i++) {
      ptable_in->r_lut_p[i] -= linearization->blk_level_applied.r_val;
      ptable_in->b_lut_p[i] -= linearization->blk_level_applied.b_val;
      ptable_in->gr_lut_p[i] -= linearization->blk_level_applied.gr_val;
      ptable_in->gb_lut_p[i] -= linearization->blk_level_applied.gb_val;
    }
  }
  return ret;
}

/** linearization40_trigger_update
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  trigger update based on 3A event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean linearization40_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                        ret = TRUE;
  linearization40_t             *linearization = NULL;
  Linearization_high_res_Lut_t   output1, output2;
  isp_private_event_t           *private_event = NULL;
  isp_sub_module_output_t       *sub_module_output = NULL;
  isp_meta_entry_t              *linearization_dmi_info = NULL;
  uint32_t                      *linearization_dmi_tbl = NULL;
  int                            i;

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
    ISP_ERR("failed: output %p", sub_module_output);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  linearization = (linearization40_t *)isp_sub_module->private_data;
  if (!linearization) {
    ISP_ERR("failed: linearization %p", linearization);
    goto ERROR;
  }

  if (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_STAGGERED &&
    isp_sub_module->trigger_update_pending) {
     CDBG("Disable linearization for SVHDR");
     isp_sub_module->submod_enable = FALSE;
     isp_sub_module->update_module_bit = TRUE;
     PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
     return ret;
  }

  /* TODO: Need to propagate black level lock to meta data 'every frame' */
  if (isp_sub_module->manual_ctrls.manual_update &&
      isp_sub_module->chromatix_module_enable &&
      !isp_sub_module->config_pending) {
    isp_sub_module->submod_trigger_enable =
      (isp_sub_module->manual_ctrls.black_level_lock ==
      CAM_BLACK_LEVEL_LOCK_ON)? FALSE: TRUE;
    isp_sub_module->manual_ctrls.manual_update = FALSE;
  }

  if (sub_module_output->meta_dump_params) {
    linearization->metadump_enable =
      sub_module_output->metadata_dump_enable;
  }
  if(isp_sub_module->config_pending)
    isp_sub_module->config_pending = FALSE;

  if (!isp_sub_module->submod_enable ||
      !isp_sub_module->submod_trigger_enable ||
      !isp_sub_module->stream_on_count) {
    ISP_DBG("no trigger update for Linearization, enabled %d, trig_enable %d\n",
      isp_sub_module->submod_enable, isp_sub_module->submod_trigger_enable);
    goto FILL_METADATA;
  }

  memset(&output1, 0, sizeof(Linearization_high_res_Lut_t));
  memset(&output2, 0, sizeof(Linearization_high_res_Lut_t));
  memset(&linearization->final_output, 0, sizeof(Linearization_high_res_Lut_t));

  if ((isp_sub_module->trigger_update_pending == TRUE) ||
    (linearization->apply_hdr_effects)) {
    linearization40_get_tables(isp_sub_module, linearization, &output1,
      &output2);

    if (isp_sub_module->dual_led_calibration_enabled == FALSE) {
      linearization40_apply_aec_update(linearization, &output1,
        &output2, &linearization->final_output);
    } else {
      /* For v1 version output2 will be normal light and for v2 version
         it will be based on trigger_index + 1 */
      memcpy(&linearization->final_output, &output2, sizeof(Linearization_high_res_Lut_t));
    }

    /* don't perform black level subtraction if pedestal is enabled
     * pedestal will do the black level subtraction */
    linearization40_correct_for_pedestal(linearization, &linearization->final_output);
    ret = linearization40_config_cmd(linearization,
      &linearization->final_output);
    if (ret == FALSE) {
      ISP_ERR("failed: config_linearization_cmd");
      goto ERROR;
    }

    ret = linearization40_store_hw_update(isp_sub_module, linearization);
    if (ret == FALSE) {
      ISP_ERR("failed: linearization40_store_hw_update");
      goto ERROR;
    }
    isp_sub_module->trigger_update_pending = FALSE;
	linearization->apply_hdr_effects = 0;
  }

  ret = isp_sub_module_util_append_hw_update_list(isp_sub_module,
    sub_module_output);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_sub_module_util_append_hw_update_list");
    goto ERROR;
  }

FILL_METADATA:
  /*even without trigger update, we still need to calculate the algo parm*/

  ret = linearization40_calc_clamp(linearization, &linearization->final_output, sub_module_output);
  if (FALSE == ret) {
    ISP_ERR("failed");
    goto ERROR;
  }

  if (sub_module_output->meta_dump_params &&
    sub_module_output->metadata_dump_enable == 1) {
    /*fill in DMI info*/
    linearization_dmi_info = &sub_module_output->
      meta_dump_params->meta_entry[ISP_META_LINEARIZATION_TBL];
    /*dmi table length*/
    linearization_dmi_info->len =
      sizeof(DMI_WRITE_TYPE) * ISP32_LINEARIZATON_TABLE_LENGTH;
    /*dmi type */
    linearization_dmi_info->dump_type = ISP_META_LINEARIZATION_TBL;
    linearization_dmi_info->start_addr = 0;
    sub_module_output->meta_dump_params->frame_meta.num_entry++;

#ifdef DYNAMIC_DMI_ALLOC
    linearization_dmi_tbl =
      (uint32_t *)CAM_MALLOC(linearization_dmi_info->len);
    if (!linearization_dmi_tbl) {
      ISP_ERR("failed: %p", linearization_dmi_tbl);
      ret = FALSE;
      goto ERROR;
    }
#endif
    memcpy(linearization_dmi_info->isp_meta_dump,
      &linearization->applied_hw_lut.Lut, linearization_dmi_info->len);
    //linearization_dmi_info->hw_dmi_tbl = linearization_dmi_tbl;
  }
  if (sub_module_output && isp_sub_module->vfe_diag_enable) {
    ret =   linearization40_fill_vfe_diag_data(linearization,
      isp_sub_module, sub_module_output);
    if (ret == FALSE) {
      ISP_ERR("failed: linearization40_fill_vfe_diag_data");
    }
  }

  if (sub_module_output->frame_meta)
    sub_module_output->frame_meta->black_level_lock =
      isp_sub_module->manual_ctrls.black_level_lock;
ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* linearization40_trigger_update */


/** linearization40_update_base_tables
 *
 *  @linearization: linearization handle
 *  @pchromatix_L: chromatix linearization structure
 *
 *  copy chromatix table to local
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean linearization40_update_base_tables(linearization40_t *linearization,
  chromatix_VFE_common_type *pchromatix_common)
{
  boolean              ret = TRUE;
  chromatix_L_type    *pchromatix_L = NULL;

  if (!linearization || !pchromatix_common) {
    ISP_ERR("failed! NULL input linearization %p pchromatix_L %p",
      linearization, pchromatix_L);
    return FALSE;
  }

  pchromatix_L = &pchromatix_common->chromatix_L;

  if (linearization->ext_func_table &&
    linearization->ext_func_table->update_right_base_tables) {
    if(!linearization->ext_func_table->update_right_base_tables
      (linearization, pchromatix_common)) {
      /* Load the lowlight and normal light tables */
      linearization40_copy_table_to_float(&pchromatix_L->linear_table_lowlight,
        &linearization->linear_table_output1);
      linearization40_copy_table_to_float(&pchromatix_L->linear_table_normal,
        &linearization->linear_table_output2);
    }
  }

  return ret;
}

/** linearization_config
 *
 *  @isp_sub_module: isp sub module handle
 *  @linearization: linearization handle
 *
 *  config default params
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean linearization40_config(isp_sub_module_t *isp_sub_module,
  linearization40_t *linearization)
{
  boolean                    ret;
  chromatix_VFE_common_type *pchromatix_common = NULL;

  if (!isp_sub_module || !linearization) {
    ISP_ERR("failed: %p %p", isp_sub_module, linearization);
    return FALSE;
  }

  pchromatix_common =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;

  ret = linearization40_update_base_tables(linearization, pchromatix_common);
  if (!ret) {
    ISP_ERR("failed linearization40_update_base_tables");
    return FALSE;
  }

  return ret;
} /* linearization40_config */

/** linearization40_stats_aec_manual_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_update_t
 *
 *  Handle Manual AEC update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean linearization40_stats_aec_manual_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  linearization40_t         *linearization = NULL;
  aec_manual_update_t       *manual_stats_update = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  linearization = (linearization40_t *)isp_sub_module->private_data;
  if (!linearization) {
    ISP_ERR("failed: linearization %p", linearization);
    return FALSE;
  }

  manual_stats_update = (aec_manual_update_t *)data;
  if (!manual_stats_update) {
    ISP_ERR("failed: manual_stats_update %p", manual_stats_update);
    return FALSE;
  }


  linearization->aec_update.real_gain = manual_stats_update->sensor_gain;
  isp_sub_module->trigger_update_pending = TRUE;

  return TRUE;
}
/** linearization40_stats_aec_update:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to stats_update_t
 *
 *  Handle AEC update event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean linearization40_stats_aec_update(isp_sub_module_t *isp_sub_module,
  void *data)
{
  stats_update_t            *stats_update = NULL;
  linearization40_t         *linearization = NULL;
  trigger_point_type        *linearization_trigger = NULL;
  float aec_reference;
  float ratio = 0;
  float start = 0;
  chromatix_VFE_common_type *pchromatix_common = NULL;
  float end = 0;
  boolean ret = TRUE;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  linearization = (linearization40_t *)isp_sub_module->private_data;
  if (!linearization) {
    ISP_ERR("failed: linearization %p", linearization);
    return FALSE;
  }

  stats_update = (stats_update_t *)data;
  if (!stats_update) {
    ISP_ERR("failed: stats_update %p", stats_update);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  pchromatix_common =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;
  if (!pchromatix_common) {
    ISP_ERR("failed: pchromatix_common %p", pchromatix_common);
    /* Put hw update flag to TRUE */
    isp_sub_module->trigger_update_pending = TRUE;
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  linearization->aec_update = stats_update->aec_update;
  linearization->hdr_exp_ratio = stats_update->aec_update.hdr_exp_time_ratio;
  linearization->hdr_sensitivity_ratio = stats_update->aec_update.hdr_sensitivity_ratio;

  if (linearization->ext_func_table &&
    linearization->ext_func_table->get_trigger_index) {
    if(!linearization->ext_func_table->get_trigger_index(isp_sub_module,
      linearization, pchromatix_common)) {
      ratio = isp_sub_module_util_get_aec_ratio_lowlight(
        pchromatix_common->chromatix_L.control_linearization,
        &pchromatix_common->chromatix_L.linearization_lowlight_trigger,
        &linearization->aec_update, 1);

      if (!F_EQUAL(linearization->aec_ratio, ratio)) {
        linearization->aec_ratio = ratio;
        isp_sub_module->trigger_update_pending = TRUE;
      }
    }
  }
  
  if (linearization->ext_func_table && linearization->ext_func_table->get_2d_interpolation) {
    ret = linearization->ext_func_table->get_2d_interpolation(isp_sub_module, linearization);
    if (ret == FALSE) {
      ISP_ERR("failed: get_2d_interpolation");
      PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
      return FALSE;
    }
  }

  ISP_DBG("aec rato %f", ratio);

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return FALSE;
} /* linearization40_stats_aec_update */

/** linearization40set_flash_mode:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: handle to cam_flash_mode_t
 *
 *  Handle set flash mode event
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean linearization40_set_flash_mode(isp_sub_module_t *isp_sub_module,
  void *data)
{
  linearization40_t      *linearization = NULL;
  cam_flash_mode_t       *flash_mode = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  linearization = (linearization40_t *)isp_sub_module->private_data;
  if (!linearization) {
    ISP_ERR("failed: linearization %p", linearization);
    return FALSE;
  }

  flash_mode = (cam_flash_mode_t *)data;
  if (!flash_mode) {
    ISP_ERR("failed: flash_mode %p", flash_mode);
    return FALSE;
  }

  ISP_DBG("<flash_dbg> get new flash mode = %d", *flash_mode);
  linearization->cur_flash_mode = *flash_mode;
  return TRUE;
}

/** linearization40_set_chromatix_ptr:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: module event data
 *
 *  Store chromatix ptr and make initial configuration
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean linearization40_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  void *data)
{
  boolean                ret = TRUE;
  linearization40_t     *linearization = NULL;
  modulesChromatix_t    *chromatix_ptrs = NULL;

  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  chromatix_ptrs = (modulesChromatix_t *)data;
  if (!chromatix_ptrs) {
    ISP_ERR("failed: chromatix_ptrs %p", chromatix_ptrs);
    return FALSE;
  }

  linearization = (linearization40_t *)isp_sub_module->private_data;
  if (!linearization) {
    ISP_ERR("failed: linearization %p", linearization);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  isp_sub_module->chromatix_ptrs = *chromatix_ptrs;

  linearization40_enab_sub_mod(isp_sub_module);

  ret = linearization40_config(isp_sub_module, linearization);
  if (ret == FALSE) {
    ISP_ERR("failed: linearization40_config ret %d", ret);
    goto ERROR;
  }

  isp_sub_module->trigger_update_pending = TRUE;
  isp_sub_module->update_module_bit = TRUE;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  return ret;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
  return ret;
} /* linearization40_set_chromatix_ptr */

/** linearization40_streamoff:
 *
 *  @isp_sub_module: isp sub module handle
 *  @data: control event data
 *
 *  This function resets configuration during last stream OFF
 *
 *  Return: TRUE on success and FALSE on failure
 **/
boolean linearization40_streamoff(isp_sub_module_t *isp_sub_module, void *data)
{
  linearization40_t *linearization = NULL;
  if (!isp_sub_module || !data) {
    ISP_ERR("failed: %p %p", isp_sub_module, data);
    return FALSE;
  }

  linearization = (linearization40_t *)isp_sub_module->private_data;
  if (!linearization) {
    ISP_ERR("failed: linearization %p", linearization);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_sub_module->mutex);

  if (--isp_sub_module->stream_on_count) {
    PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);
    return TRUE;
  }

  isp_sub_module->trigger_update_pending = FALSE;

  memset(linearization, 0, sizeof(*linearization));

  linearization->cur_cct_type = AWB_CCT_TYPE_TL84;
  isp_sub_module->manual_ctrls.black_level_lock = CAM_BLACK_LEVEL_LOCK_OFF;

  PTHREAD_MUTEX_UNLOCK(&isp_sub_module->mutex);

  FILL_FUNC_TABLE(linearization);
  return TRUE;
} /* linearization40_streamoff */

/** linearization_init:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Initialize the linearization module
 *
 *  Return TRUE on Success, FALSE on failure
 **/
boolean linearization40_init(isp_sub_module_t *isp_sub_module)
{
  linearization40_t *linearization = NULL;

  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return FALSE;
  }

  linearization = (linearization40_t *)CAM_MALLOC(sizeof(linearization40_t));
  if (!linearization) {
    ISP_ERR("failed: linearization %p", linearization);
    return FALSE;
  }

  memset(linearization, 0, sizeof(*linearization));

  isp_sub_module->private_data = (void *)linearization;
  isp_sub_module->manual_ctrls.black_level_lock = CAM_BLACK_LEVEL_LOCK_OFF;

  linearization->cur_cct_type = AWB_CCT_TYPE_TL84;

  linearization->lux_filter.init_flag = FALSE;
  linearization->cur_flash_mode =  CAM_FLASH_MODE_OFF;
  /* default aec_ratio = 1 will pick normal table in trigger update */
  linearization->aec_ratio = 1;
  linearization->trigger_index = NORMAL_LIGHT_INDEX;
  linearization->last_region_unity_slope_enable = FALSE;

  FILL_FUNC_TABLE(linearization);
  return TRUE;
}/* linearization40_init */

/** linearization40_destroy:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Destroy dynamic resources
 *
 *  Return none
 **/
void linearization40_destroy(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: isp_sub_module %p", isp_sub_module);
    return;
  }

  free(isp_sub_module->private_data);
  return;
} /* linearization40_destroy */

#if !OVERRIDE_FUNC
static ext_override_func linearization_override_func = {
  .get_trigger_index = linearization40_get_trigger_idx,
  .update_right_base_tables = linearization40_update_right_base_tables,
  .get_2d_interpolation = NULL,
  .apply_2d_interpolation = NULL,
};

boolean linearization40_fill_func_table(linearization40_t *linearization)
{
  linearization->ext_func_table = &linearization_override_func;
  return TRUE;
} /* linearization40_fill_func_table */
#endif
