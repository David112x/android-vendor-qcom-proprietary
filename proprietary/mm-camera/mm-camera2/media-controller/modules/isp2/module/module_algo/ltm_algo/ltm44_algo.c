/* ltm_algo.c
 *
 * Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* mctl headers */
#include "mct_event_stats.h"
#include "media_controller.h"
#include "mct_list.h"

/* isp headers */
#include "isp_log.h"
#include "ltm44_algo.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_LTM, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_LTM, fmt, ##args)

#include "autoltm.h"

static boolean isp_ltm_algo_stop_session(mct_module_t *module,
  isp_saved_algo_params_t *algo_session_params)
{
  isp_saved_ltm_params_t *ltm_algo_params = NULL;

  if (!module || !algo_session_params) {
    ISP_ERR("failed: module %p algo_session_params %p", module, algo_session_params);
    return FALSE;
  }

  if (algo_session_params->ltm_saved_algo_parm.autoltm_obj)
  {
    int rc = autoltm_deinit(algo_session_params->ltm_saved_algo_parm.autoltm_obj);
    algo_session_params->ltm_saved_algo_parm.autoltm_obj = NULL;
    if (rc)
      ISP_ERR("failed: module %p algo_session_params %p", module, algo_session_params);
  }
  return TRUE;
}

/** isp_algo_ltm_execute:
 *
 *  @module:         mct module handle
 *  @stats_mask      stats mask
 *  @parsed_stats:   stats after parsing
 *  @algo_parm:      algorithm params
 *  @saved_algo_parm container to hold the output
 *  @output          actual output payload to be sent
 *
 *  Execute ltm algo
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp44_algo_ltm_execute(mct_module_t *module,
  mct_event_stats_isp_t *stats_data,
  isp_algo_params_t *algo_parm,
  isp_saved_algo_params_t *saved_algo_parm,
  void **output,
  uint32_t curr_frame_id,
  boolean svhdr_enb);

isp_algo_t algo_ltm44 = {
  "local tone mapping",                  /* name */
  &isp44_algo_ltm_execute,               /* algo func pointer */
  &isp_ltm_algo_stop_session,           /* stop session func pointer */
  MCT_EVENT_MODULE_ISP_LTM_ALGO_UPDATE,  /* output type */
};


#define CLAMP(a,b,c) (((a) <= (b))? (b) : (((a) >= (c))? (c) : (a)))

#define PRINT_FLOAT(field) ISP_ERR("ltm %s : %f", #field, field)
#define PRINT_INT(field)   ISP_ERR("ltm %s : %d", #field, field)

#define PIX_BITS 12
#define MAX_PIXEL_VAL ((1 << PIX_BITS) - 1)
#define TEMPORAL_ALPHA 0.5f
#define MASTER_SCALE_Q 9
#define BG_TRUNC_BITS 4

/* get_rectification
 *
 *  @module: mct module handle
 *  @trigger: calculate trigger
 *
 *  Execute algo by parsed stats and algo parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
static double get_rectification(uint32_t value, uint32_t *trigger) {

   if (! trigger) {
     ISP_ERR("NULL pointer %p", trigger);
     return 0;
   }
  if (value < trigger[0])
    return 0;
  else if (value < trigger[1])
    return (double)(value - trigger[0]) / (double)(trigger[1] - trigger[0]);
  else if (value < trigger[2])
    return 1.0;
  else if (value < trigger[3])
    return (double)(trigger[3] - value) / (double)(trigger[3] - trigger[2]);
  else
    return 0;
}

/** isp_ltm_algo_update_ltm_mask_clutter:
 *
 *  @module: mct module handle
 *  @ihist_stats: ihist stats
 *  @algo_parm: algo input
 *  @output_ltm_parm: algo output
 *
 *  Execute algo by parsed stats and algo parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_ltm_algo_update_ltm_mask_clutter(mct_module_t *module,
  q3a_ihist_stats_t *ihist_stats, isp_algo_params_t *algo_parm,
  isp_saved_ltm_params_t *output_ltm_parm)
{
  boolean  ret  = TRUE;
  uint32_t *ihist_data = ihist_stats->histogram;
  int      i = 0;
  uint64_t mean_hist_hi = 0;
  uint64_t mean_hist_lo = 0;
  uint64_t std_hist_hi = 0;
  uint64_t std_hist_lo = 0;
  int64_t  tmp = 0;

  if (!module || !ihist_stats || !algo_parm || !output_ltm_parm) {
    ISP_ERR("failed. NULL pointer %p %p %p %p",
      module, ihist_stats, algo_parm, output_ltm_parm);
    return FALSE;
  }

  for (i = 0; i < 128; i++) {
    mean_hist_lo += (ihist_data[i] << (algo_parm->ihist_shift_bits + 1));
  }
  mean_hist_lo >>= 7;

  /*Sample mean*/
  for (i = 128; i < 256; i++) {
    mean_hist_hi += (ihist_data[i] << (algo_parm->ihist_shift_bits + 1));
  }
  mean_hist_hi >>= 7;

  for (i = 0; i < 128; i++) {
    tmp = (int32_t)(ihist_data[i] <<
      (algo_parm->ihist_shift_bits + 1)) - (int32_t)mean_hist_lo;
    std_hist_lo += tmp * tmp;
  }

  /*Unbiased*/
  std_hist_lo = (uint64_t)sqrt((float)std_hist_lo / 127.0);
  std_hist_lo = CLAMP(std_hist_lo, 0, 300000);
  for (i = 128; i < 256; i++) {
    tmp = (int32_t)(ihist_data[i] <<
      (algo_parm->ihist_shift_bits + 1)) - (int32_t)mean_hist_hi;
    std_hist_hi += tmp * tmp;
  }
  std_hist_hi = (uint64_t)sqrt((float)std_hist_hi / 127.0);

  std_hist_hi = CLAMP(std_hist_hi, 0, 300000);
  PRINT_FLOAT((float)std_hist_lo);
  PRINT_FLOAT((float)std_hist_hi);

  output_ltm_parm->high_clutter = std_hist_hi;
  output_ltm_parm->low_clutter = std_hist_lo;

  return ret;
}

/** isp_ltm_algo_update_ltm_mask_rectification_curve:
 *
 *  @module: mct module handle
 *  @ihist_stats: stats notify event
 *  @algo_parm: algo input
 *  @output_ltm_parm: algo output
 *
 *  Execute algo by parsed stats and algo parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_ltm_algo_update_ltm_mask_rectification_curve(
  mct_module_t           *module,
  q3a_ihist_stats_t      *ihist_stats,
  isp_algo_params_t      *algo_parm,
  isp_saved_ltm_params_t *output_ltm_parm)
{
  boolean            ret = TRUE;
  int32_t            i = 0;
  int32_t            curve[algo_parm->ltm_lut_size + 1];
  int32_t            smooth_curve[algo_parm->ltm_lut_size + 1];
  int32_t            ltm_lut_size = algo_parm->ltm_lut_size;
  chromatix_LTM_type *ltm_type = &algo_parm->normal_ltm_data;

  if (!module || !ihist_stats) {
      ISP_ERR("failed: module %p ihist_stats %p", module, ihist_stats);
      return FALSE;
  }

  memset(curve, 0, sizeof(curve));

  /* defaults for now */
  uint32_t LoTrigger[4];
  uint32_t HiTrigger[4];
  LoTrigger[0] = ltm_type->reservedData.nLowTrig0;
  LoTrigger[1] = ltm_type->reservedData.nLowTrig1;
  LoTrigger[2] = ltm_type->reservedData.nLowTrig2;
  LoTrigger[3] = ltm_type->reservedData.nLowTrig3;
  HiTrigger[0] = ltm_type->reservedData.nHighTrig0;
  HiTrigger[1] = ltm_type->reservedData.nHighTrig1;
  HiTrigger[2] = ltm_type->reservedData.nHighTrig2;
  HiTrigger[3] = ltm_type->reservedData.nHighTrig3;

  uint32_t StrongLoCut = ltm_type->reservedData.nStrongLoCut;
  uint32_t NormalLoCut = ltm_type->reservedData.nNormalLoCut;
  uint32_t StrongHiCut = ltm_type->reservedData.nStrongHiCut;
  uint32_t NormalHiCut = ltm_type->reservedData.nNormalHiCut;

  /* scale LoCut, HiCut by LUT entries */
  NormalLoCut <<= (algo_parm->ltm_lut_size / 64 - 1);
  StrongLoCut <<= (algo_parm->ltm_lut_size / 64 - 1);
  NormalHiCut <<= (algo_parm->ltm_lut_size / 64 - 1);
  StrongHiCut <<= (algo_parm->ltm_lut_size / 64 - 1);

  double StrongRatioLo,StrongRatioHi;
  StrongRatioLo = get_rectification(output_ltm_parm->low_clutter, LoTrigger);
  double LoCutRatio = (output_ltm_parm->busyness +StrongRatioLo)/2;
  StrongRatioHi = get_rectification(output_ltm_parm->high_clutter, HiTrigger);
  double HiCutRatio = StrongRatioHi;

  int32_t LoCut = (int32_t)((double)(StrongLoCut - NormalLoCut) *
    LoCutRatio + NormalLoCut + 0.5);
  int32_t HiCut = (int32_t)((double)(StrongHiCut - NormalHiCut) *
    HiCutRatio + NormalHiCut + 0.5);
  int32_t nLoStart = LoCut - NormalLoCut;
  int32_t nHiEnd = (algo_parm->ltm_lut_size) - (HiCut - NormalHiCut);
  if (nHiEnd == nLoStart) {
    ISP_ERR("fails ! nHiEnd == nLostart ");
    return FALSE;
  }
  double dSlope = (double)(MAX_PIXEL_VAL) / (double)(nHiEnd - nLoStart);

  /* rough curve generation */
  for (i = nLoStart; i <= nHiEnd; i++) {
    curve[i] = CLAMP((int32_t)(dSlope *
      (double)(i - nLoStart) + 0.5), 0, MAX_PIXEL_VAL);
  }

  for (i = 0; i < (int32_t)LoCut; i++) {
    curve[i] = 0;
  }

  for (i = (ltm_lut_size + 1) - HiCut; i <= ltm_lut_size; i++) {
    curve[i] = MAX_PIXEL_VAL;
  }

  /* smoothing */
  int nSmooth = fmin(LoCut, HiCut) + NormalLoCut;

  for (i=nSmooth; i>0; i--) {
    /* nSmooth times of [1 2 1]/4 smoothing */
    int j = 0;
    smooth_curve[0] = 0;
    for (j = 1; j < ltm_lut_size; j++) {
      smooth_curve[j] =
        CLAMP((curve[j-1] + (curve[j] << 1) + curve[j+1] + 2) >> 2,
        0, MAX_PIXEL_VAL);
    }
    smooth_curve[algo_parm->ltm_lut_size] = MAX_PIXEL_VAL;
    memcpy(curve, smooth_curve,
     (algo_parm->ltm_lut_size + 1) * sizeof(int32_t));
  }

  for (i = 0; i < ltm_lut_size; i++) {
    output_ltm_parm->mask_curve[i] = curve[i];
  }
  /* copy into mask params with temporal filtering*/
#ifdef USE_TEMPORAL_FILTER
  for (i = 0; i < ltm_lut_size; i++) {
    output_ltm_parm->mask_curve[i] =
      (int32_t)(TEMPORAL_ALPHA * output_ltm_parm->mask_curve[i] +
      (1.0f - TEMPORAL_ALPHA) * curve[i]);
  }
#endif
  return ret;
}

/** GammaLUT:
 *
 *  @module: mct module handle
 *  @gamma: gamma HW table
 *
 *  Execute algo by parsed stats and algo parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
static uint32_t isp_Gamma44_LUT(uint32_t y_in, uint32_t *gamma)
{
  /* Y values have 12 bits, [0-4095] */
  /* gamma table have 512 entries (9 bit for address) */
  uint32_t lut_index = y_in >> 4;
  uint32_t lut_offset = y_in & 0x0f;

  /* gamma 44 packed in 12 | 12 format */
  uint32_t lut_value = gamma[lut_index] & 0xfff;
  uint32_t lut_delta = gamma[lut_index] >> 12;

  /* calculate amount of delta to add (offset / 0x7) */
  lut_value += (lut_offset * lut_delta + 8) >> 4;
  return CLAMP(lut_value, 0, 4095);
}

/** isp_ltm_algo_update_ltm_mask_busyness:
 *
 *  @module: mct module handle
 *  @bg_stats: BG stats
 *  @algo_parm: algo input params
 *  @output_ltm_parm: LTM algo output
 *
 *  Execute algo by parsed stats and algo parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_ltm_algo_update_ltm_mask_busyness(
    mct_module_t           *module,
    q3a_bg_stats_t         *bg_stats,
    isp_algo_params_t      *algo_parm,
    isp_saved_ltm_params_t *output_ltm_parm)
{
  boolean ret = TRUE;
  uint32_t bg_width = bg_stats->bg_region_h_num;
  uint32_t bg_half_width;
  uint32_t bg_height = bg_stats->bg_region_v_num;
  uint32_t bg_half_height;
  uint32_t bg_binning = 0;
  uint32_t i, j, k, l, m, n= 0;
  int32_t Yp[54][72];
  int32_t Haar[54][72];
  uint64_t LL = 0;
  uint64_t LH = 0;
  uint64_t HL = 0;
  uint64_t HH = 0;
  double BusyIdx = 0;
  double HVratio = 0;

  chromatix_LTM_type *ltm_type = &algo_parm->normal_ltm_data;

  if (!module) {
      ISP_ERR("failed: module %p", module);
  }

  /* default values */
  double BusyThr1 = ltm_type->reservedData.fBusyThr1;
  double BusyThr2 = ltm_type->reservedData.fBusyThr2;

  /* scale floats to use as integers */
  int16_t c1 = ltm_type->reservedData.fCV_R * 1024 + 0.5;
  int16_t c2 = ltm_type->reservedData.fCV_G * 1024 + 0.5;
  int16_t c3 = ltm_type->reservedData.fCV_B * 1024 + 0.5;
  int16_t ck = ltm_type->reservedData.fCV_Kc * 512 + 0.5;
  int16_t thr = ltm_type->reservedData.fCV_Cthr * 4 + 0.5;

  if (bg_width >= 48 && bg_height >= 36) {
    // BG mini image is too detailed, better be 32x24 => H2V2 binning
    bg_width  >>= 1;
    bg_height >>= 1;
    bg_binning = 1;
  }
  bg_half_width = bg_width >> 1;
  bg_half_height = bg_height >> 1;

  uint32_t sumYp = 0;
  for (i=0; i<bg_height; i++) {
    for (j=0; j<bg_width; j++) {
      uint32_t R, G, B = 0;
      uint32_t tmp = 0;
      if (bg_binning) {
        // H2V2 BG binning
        k = (i << 1) * (int)bg_stats->bg_region_h_num + (j << 1);
        l = k + 1;
        m = k + (int)bg_stats->bg_region_h_num;
        n = m + 1;
        if (bg_stats->bg_r_num[k] || bg_stats->bg_r_num[l] ||
            bg_stats->bg_r_num[m] || bg_stats->bg_r_num[n]) {
          R = (uint32_t)((((uint64_t)bg_stats->bg_r_sum[k] +
            (uint64_t)bg_stats->bg_r_sum[l] +
            (uint64_t)bg_stats->bg_r_sum[m] +
            (uint64_t)bg_stats->bg_r_sum[n]) << BG_TRUNC_BITS) /
            (bg_stats->bg_r_num[k] + bg_stats->bg_r_num[l] +
            bg_stats->bg_r_num[m] + bg_stats->bg_r_num[n]));
        } else {
          R = 0;
        }

        if (bg_stats->bg_gr_num[k] || bg_stats->bg_gb_num[k] ||
            bg_stats->bg_gr_num[l] || bg_stats->bg_gb_num[l] ||
            bg_stats->bg_gr_num[m] || bg_stats->bg_gb_num[m] ||
            bg_stats->bg_gr_num[n] || bg_stats->bg_gb_num[n]) {
          G = (uint32_t)((((uint64_t)bg_stats->bg_gr_sum[k] +
            (uint64_t)bg_stats->bg_gb_sum[k] +
            (uint64_t)bg_stats->bg_gr_sum[l] +
            (uint64_t)bg_stats->bg_gb_sum[l] +
            (uint64_t)bg_stats->bg_gr_sum[m] +
            (uint64_t)bg_stats->bg_gb_sum[m] +
            (uint64_t)bg_stats->bg_gr_sum[n] +
            (uint64_t)bg_stats->bg_gb_sum[n]) << BG_TRUNC_BITS) /
            (bg_stats->bg_gr_num[k] + bg_stats->bg_gb_num[k] +
             bg_stats->bg_gr_num[l] + bg_stats->bg_gb_num[l] +
             bg_stats->bg_gr_num[m] + bg_stats->bg_gb_num[m] +
             bg_stats->bg_gr_num[n] + bg_stats->bg_gb_num[n]));
        } else {
          G = 0;
        }

        if (bg_stats->bg_b_num[k] || bg_stats->bg_b_num[l] ||
            bg_stats->bg_b_num[m] || bg_stats->bg_b_num[n]) {
          B = (uint32_t)((((uint64_t)bg_stats->bg_b_sum[k] +
            (uint64_t)bg_stats->bg_b_sum[l] +
            (uint64_t)bg_stats->bg_b_sum[m] +
            (uint64_t)bg_stats->bg_b_sum[n]) << BG_TRUNC_BITS) /
            (bg_stats->bg_b_num[k] + bg_stats->bg_b_num[l] +
             bg_stats->bg_b_num[m] + bg_stats->bg_b_num[n]));
        } else {
          B = 0;
        }
      } else {
        k = i * bg_width + j;
        if (bg_stats->bg_r_num[k])
          R = (uint32_t)(((uint64_t)bg_stats->bg_r_sum[k] << BG_TRUNC_BITS) /
                  bg_stats->bg_r_num[k]);
        else R = 0;
        if (bg_stats->bg_gr_num[k] || bg_stats->bg_gb_num[k]) {
          G = (uint32_t)((((uint64_t)bg_stats->bg_gr_sum[k] +
           (uint64_t)bg_stats->bg_gb_sum[k]) << BG_TRUNC_BITS) /
           (bg_stats->bg_gr_num[k] + bg_stats->bg_gb_num[k]));
        } else {
          G = 0;
        }

        if (bg_stats->bg_b_num[k]) {
          B = (uint32_t)(((uint64_t)bg_stats->bg_b_sum[k] << BG_TRUNC_BITS) /
            bg_stats->bg_b_num[k]);
        } else {
          B = 0;
        }
      }

      if (/*gamma->lut_enable*/ 1) {
        R = isp_Gamma44_LUT(R, algo_parm->gamma_r);
        G = isp_Gamma44_LUT(G, algo_parm->gamma_g);
        B = isp_Gamma44_LUT(B, algo_parm->gamma_b);
      }

      tmp = fmax(abs((int32_t)R - (int32_t)G), abs((int32_t)B - (int32_t)G)) -
        ((int32_t)thr << 2);
      tmp = fmax(tmp, 0);

      Yp[i][j] = ((int32_t)(tmp * ck)) >> 9;

#if 0  /* 8994 version, use isp version to differentiate*/
      /* 8994 Version*/
      Yp[i][j] += (c1 * R + c2 * G + c3 * B +
             c4 * fmax(R, fmax(G, B))) >> 10;
#else

      /* 8084 Version */
      Yp[i][j] += (int32_t)((c1 * R + c2 * G + c3 * B)) >> 10;
#endif

      Yp[i][j] = CLAMP(Yp[i][j] >> (PIX_BITS - 8), 0, 255);  // 12u/14u -> 8u
      sumYp += Yp[i][j];
    }
  }
  // Take horizontal Haar transform
  for (i=0; i<bg_height; i++) {
    /* dont touch the rest of defines */
    for (j=0; j<bg_half_width; j++) {
      k = j << 1;
      l = k + 1;
      /* Scaling function */
      Haar[i][j] = Yp[l][j] + Yp[k][j];
      // Wavelet function
      Haar[i+bg_half_height][j] = Yp[l][j] - Yp[k][j];
      // Sum of absolute subband coefficients
      if (j < bg_half_width) {
        LL += abs(Haar[i][j]);
        HL += abs(Haar[i+bg_half_height][j]);
      } else {
        LH += abs(Haar[i][j]);
        HH += abs(Haar[i+bg_half_height][j]);
      }
    }
  }

  // Ratio of HL & LH subbands
  if (HL == 0 || LH == 0) {
    ISP_ERR("Hl =%lld LH = %lld", HL,LH);
    return FALSE;
  }
  HVratio = fmax((double)LH/(double)HL, (double)HL/(double)LH);

  // Image busyness
  if (HVratio >= BusyThr2) {
    BusyIdx = 1.0;
  } else if (HVratio >= BusyThr1){
    BusyIdx = (HVratio - BusyThr1) / (BusyThr2 - BusyThr1);
  } else {
    BusyIdx = 0;
  }
  output_ltm_parm->busyness = BusyIdx;

  return ret;
}


/** isp_ltm_algo_update_ltm_tone_curves:
 *
 *  @module: mct module handle
 *  @algo_parm: algo input
 *  @saved_algo_output: algo output
 *
 *  Execute algo by parsed stats and algo parm
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_ltm_algo_update_ltm_tone_curves(
    mct_module_t            *module,
    isp_algo_params_t       *algo_parm,
    isp_saved_algo_params_t *saved_algo_output)
{
  boolean ret = TRUE;
  int32_t i = 0;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION != 0x304)
  chromatix_LTM_core *backlit_ltm = NULL;
#else
  chromatix_LTM_type *backlit_ltm = &algo_parm->backlit_ltm_data;
#endif
  chromatix_LTM_type *normal_ltm = &algo_parm->normal_ltm_data;

  int32_t PrincipalCurve[256];
  uint16_t ManualPDF[256];
  int32_t ManualCurve[256];
  int32_t Pm[256];
  double Constraint1[256];
  double Constraint2[256];
  double ScaleConstraint[256];
  double dPrincipal[255]; /* derivative of principal curve */

  /* resulting curves */
  double *MasterCurve = saved_algo_output->ltm_saved_algo_parm.master_curve;
  double *MasterScale = saved_algo_output->ltm_saved_algo_parm.master_scale;
  double *ShiftCurve = saved_algo_output->ltm_saved_algo_parm.shift_curve;
  double *ShiftScale = saved_algo_output->ltm_saved_algo_parm.shift_scale;

  /* temp variables */
  int ScaleStart = 0;
  uint32_t ManuCapBias = 0;
  double MaxScaleCons = 0;
  double dPmin = 1.0e10; /* minimum principle curve derivative */
  double sumpdf = 0;
  double ScaleSigma = 0;
  double ScaleVar = 0;
  double GaussMagnitude = 0;
  double MasterScaleMag = 0;
  double MasterScaleBase = 0;
  double StdHist = 0;  /* STD of zero symmetric p' */
  double ShiftGradient = 0;
  double ShiftScaleMag = 0;
  double ShiftScaleBase = 0;

  /* LTM LA curve is on differnt accuracy*/
  uint32_t LA_curve[256];

  if (!module) {
      ISP_ERR("failed: module %p", module);
      return FALSE;
  }

  /*before update la curve algo, temp boost value and ignore the accuracy
    TODO: update new LA algo to get 12 bit accuracy LA curve*/
  for (i = 0; i < 256; i++) {
    LA_curve[i] = saved_algo_output->la_saved_algo_parm.la_curve[i];
    //ISP_ERR("LA_curve[%d] = 0x%x",i,LA_curve[i]);
  }


  /* variables from chromatix reserved */

#define BACKLIT_INTERP(field) ((backlit_ltm->field * backlight_scene_severity \
    + normal_ltm->field * (255 - backlight_scene_severity))/255)

  uint32_t backlight_scene_severity = algo_parm->backlight_scene_severity;

 /* all data must be interp between backlit and normal */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION != 0x304)  //To revisit
  double ScaleConsLB = 0.0; //BACKLIT_INTERP(reservedData.fScaleConstraintLB);
  double ScaleSigmaLB = 0.0; //BACKLIT_INTERP(reservedData.fScaleWidthLB);
  double ScaleSigmaMult = 0.0; //BACKLIT_INTERP(reservedData.fScaleSigmaMult);
  double MasterScaleAdj = 0.0; //BACKLIT_INTERP(reservedData.fMasterScaleAdj);
  int32_t SoftThr_0 = 0.0; //BACKLIT_INTERP(reservedData.nSoftThr0);
  int32_t SoftThr_1 = 0.0; //BACKLIT_INTERP(reservedData.nSoftThr1);
  int32_t SoftThrSlope = 0.0; //BACKLIT_INTERP(reservedData.nSoftThrSlope);
  double ShiftScaleAdj = 0.0; //BACKLIT_INTERP(reservedData.fShiftScaleAdj);

  /* variables from chromatix tuning */
  float ManuGTMgain = 0.1f; //0.01f + (float)BACKLIT_INTERP(nGlobalToneStrength)/10.f;
  float ManuGTMsigma = 0.01f; //0.01f + (float)BACKLIT_INTERP(nGlobalToneContrast)/20.f;
  double w1 =  0.01f; //(float)(BACKLIT_INTERP(nGlobalToneWeight))/100.f;
  double w2 = 1.0 - w1;
  float ShiftMag =  0.01f; //(float)BACKLIT_INTERP(nLocalToneStrength) * 3.f/100.f;
  float ShiftSigma =  0.05f ;//+ (float)BACKLIT_INTERP(nLocalToneContrast)/200.f;
#else
  double ScaleConsLB = BACKLIT_INTERP(reservedData.fScaleConstraintLB);
  double ScaleSigmaLB = BACKLIT_INTERP(reservedData.fScaleWidthLB);
  double ScaleSigmaMult = BACKLIT_INTERP(reservedData.fScaleSigmaMult);
  double MasterScaleAdj = BACKLIT_INTERP(reservedData.fMasterScaleAdj);
  int32_t SoftThr_0 = BACKLIT_INTERP(reservedData.nSoftThr0);
  int32_t SoftThr_1 = BACKLIT_INTERP(reservedData.nSoftThr1);
  int32_t SoftThrSlope = BACKLIT_INTERP(reservedData.nSoftThrSlope);
  double ShiftScaleAdj = BACKLIT_INTERP(reservedData.fShiftScaleAdj);

  /* variables from chromatix tuning */
  float ManuGTMgain = 0.01f + (float)BACKLIT_INTERP_CORE(nGlobalToneStrength, 2)/10.f;
  float ManuGTMsigma = 0.01f + (float)BACKLIT_INTERP_CORE(nGlobalToneContrast, 2)/20.f;
  double w1 =  (float)(BACKLIT_INTERP_CORE(nGlobalToneWeight, 2))/100.f;
  double w2 = 1.0 - w1;
  float ShiftMag =  (float)BACKLIT_INTERP_CORE(nLocalToneStrength, 2) * 3.f/100.f;
  float ShiftSigma =  0.05f + (float)BACKLIT_INTERP_CORE(nLocalToneContrast, 2)/200.f;
#endif
  for (i=0; i<32; i++) {
    if (LA_curve[i] <= (i * (uint32_t)(0.75f * (float)(1<<(PIX_BITS-8))))) {
      ManuCapBias = i;  /* 0.75x slope of 4096/256=16 or 16384/256=64 */
    }
    else {
      break;
    }
  }

  /* temporal smoothing of cap bias for video usecase */
  ManuCapBias =
    (uint32_t)(TEMPORAL_ALPHA *
    saved_algo_output->ltm_saved_algo_parm.prevManuCapBias +
    (1.0f - TEMPORAL_ALPHA) * ManuCapBias);
  saved_algo_output->ltm_saved_algo_parm.prevManuCapBias = ManuCapBias;

  /* PDF Q6*/
  for (i = 0; i < 256; i++) {
    double ManuCapShape = (double)(i - ManuCapBias) / ManuGTMsigma;
    ManualPDF[i] = (uint16_t)(64.0 *
      (ManuGTMgain * exp(-0.5 * ManuCapShape * ManuCapShape) + 1.0) + 0.5);
  }

  /* CDF */
  ManualCurve[0] = ManualPDF[0];
  for (i=1; i<256; i++)
    ManualCurve[i] = ManualCurve[i-1] + ManualPDF[i];  /* CDF */

  /* Normalized CDF */
  for (i = 0; i < 256; i++) {
    if (ManualCurve[255] == 0) {
        ISP_ERR("ManualCurve[255] == 0");
        return FALSE;
    }
    ManualCurve[i] = (int32_t)((uint64_t)MAX_PIXEL_VAL *
      (uint64_t)ManualCurve[i] / ManualCurve[255]);
  }

  ManualCurve[0] = 0;
  ManualCurve[1] = (0+ManualCurve[1]+ManualCurve[2]) / 3;

  for (i = 2; i < 254; i++) {
    ManualCurve[i] = (ManualCurve[i-2] + ManualCurve[i-1] + ManualCurve[i]+
      ManualCurve[i+1] + ManualCurve[i+2]) / 5;
  }

  /* Smoothed CDF -> Output manual GTM curve, [0, MAX_PIXEL_VAL] */
  ManualCurve[254] = (ManualCurve[253] + ManualCurve[254] + MAX_PIXEL_VAL) / 3;
  ManualCurve[255] = MAX_PIXEL_VAL;

  /* Combined GTM curves as the principal curve */
  for (i = 0; i < 256; i++) {
    PrincipalCurve[i] = (int32_t)(w1 * (double)LA_curve[i] +
      w2 * (double)ManualCurve[i])/100.0f;
  }

  /* Xiaoyun's constraints of master scaling factor curve */
  for (i = 0; i < 256; i++) {
    /* Pm curve = Principal curve - (0:MAX_PIXEL_VAL) */
     /* 12s, 0 when i = 0 or 255 */
    Pm[i] = PrincipalCurve[i] - ((MAX_PIXEL_VAL * i) / 255);
    /*BG_ERROR("yinlokh Pm[%d] = PC(%d) - %d = %d",
      i, PrincipalCurve[i], (MAX_PIXEL_VAL * i / 255), Pm[i]); */
    if (i < 255) {
      /* 1st master scale constraint: prevent saturation */
      Constraint1[i] = (double)Pm[i] / ((double)MAX_PIXEL_VAL -
        ((double)MAX_PIXEL_VAL * i /255.0));

      /* Principal curve derivative P'=dP/dx ~capped iHist, where dx=di*16 */
      dPrincipal[i] = (double)(PrincipalCurve[i+1] -
        PrincipalCurve[i]) * 255 / (double)MAX_PIXEL_VAL;

      if (dPrincipal[i] < dPmin) {
        dPmin = dPrincipal[i];
      }

      /* 2nd master scale constraint : monotonically increasing curve */
        /* -Pm' = 1 - P' = 1 - dP/dx, dx=di*16 */
      Constraint2[i] = 1 - dPrincipal[i];
    } else {
      /* Duplicate constraints on [255]
         because curve sets should be on MAX_PIXEL_VAL */
      Constraint1[i] = Constraint1[i-1];  /* 0/0 */
      /* 1 - (double)(MAX_PIXEL_VAL -
       * PrincipalCurve[i]) * 255 / MAX_PIXEL_VAL;
       */
      Constraint2[i] = Constraint2[i-1];
    }

    /* Combined master scale constraint */
    ScaleConstraint[i] = fmax(Constraint1[i], Constraint2[i]);
    ScaleConstraint[i] = fmax(ScaleConstraint[i], ScaleConsLB);
    /* Bound master scale from below */

    /* Find the index of the first non-negative Pm */
    if (Pm[i] < 0) {
      ScaleStart = i + 1;
    }

    /* Find the maximum master scale constraint value */
    if (ScaleConstraint[i] > MaxScaleCons) {
      MaxScaleCons = ScaleConstraint[i];
    }
  }
  ScaleStart = ((ScaleStart + 3) >> 2) << 2;  /* Round up to the nearest 4s */
  MaxScaleCons = fmax(MaxScaleCons, 0.05);  /* Avoid divide by zero later */

  /* Estimate a Gaussian envelope of iHist as the master scale curve */
  for (i = 0; i < 255; i++) {
    /* Compute the sigma of zero symmetric pdf P'(254,...,1,0,1,...,254) */
    double pdf = dPrincipal[i] - dPmin;
    if (i == 0) {
      sumpdf += pdf;
    } else {
      sumpdf += 2 * pdf;
      StdHist += 2 * (i * i) * pdf;  // Zero symmetric, zero mean  Var(i)
    }

  }

  StdHist = sqrt(StdHist / sumpdf);
  /* Shape of Gaussian with a minimum lobe */
  ScaleSigma = fmax(ScaleSigmaMult * StdHist, ScaleSigmaLB);
  /* Variance of Gaussian curve */
  ScaleVar = ScaleSigma * ScaleSigma;
  /* Magnitude of Gaussian down from 1.0 */
  GaussMagnitude = 1.0 - MaxScaleCons;

  /* Adjust Gaussian magnitude for master scale curve */
  MasterScaleMag  = GaussMagnitude * MasterScaleAdj;
  MasterScaleMag  = CLAMP(MasterScaleMag, 0.0, 1.0);
  MasterScaleBase = 1.0 - MasterScaleMag;

  /* Master and master scale curves */
  for (i = 0; i < ScaleStart; i++) {
    MasterCurve[i] = (double)Pm[i];  /* 12s, MasterScale = 1.0 */
    MasterScale[i] = (double)(1 << MASTER_SCALE_Q);  /* 1.0 Q9/Q10 */
  }

  for ( ; i < LTM_MC_TBL_SIZE; i++) {
    int tmp = i - ScaleStart;
    MasterScale[i] = MasterScaleBase +
      MasterScaleMag * exp(-0.5 * (double)(tmp * tmp) / ScaleVar);
    MasterCurve[i] = (double)Pm[i] / MasterScale[i];  /* 12s */
    MasterScale[i] *= (double)(1 << MASTER_SCALE_Q);  /* Scale to Q9/Q10 */
  }

  /*  Shift and shift scale curves */
  /* Adjust the estimated Gaussian scale */
  ShiftScaleMag  = GaussMagnitude * ShiftScaleAdj;
  ShiftScaleMag  = CLAMP(ShiftScaleMag, 0.0, 1.0);
  ShiftScaleBase = 1.0 - ShiftScaleMag;

  /* Shift sigmoid curve: s(x)=1/(1+exp(-x)) */
  ShiftGradient = (double)(1<<PIX_BITS) /
    (1.0 + exp(-255.0 / (ShiftSigma * 256))) -
    (double)(1 <<(PIX_BITS - 1));  /* s(i=255) */

  for (i = 0; i < LTM_SC_TBL_SIZE; i++) {
    ShiftCurve[i] = (double)(1 << PIX_BITS) /
      (1.0 + exp(-1 * (double)i / (ShiftSigma * 256))) -
      (double)(1 << (PIX_BITS - 1));
    ShiftCurve[i] = ShiftMag *
      (ShiftCurve[i] - ShiftGradient * (double)i / 255);

    /* OPTIONAL: Add soft threshold curve to shift sigmoid */
    if (i >= SoftThr_0 && i < SoftThr_1) {
      /* SoftThrSlope is Q8 integer for 12bit/14bit output,
       * so Q4/Q2 equivalent
       */
      ShiftCurve[i] +=
        (double)(SoftThrSlope * (i - SoftThr_0)) / (double)(1<<(16-PIX_BITS));
    } else if (i >= SoftThr_1) {
      ShiftCurve[i] += (double)(SoftThrSlope * (SoftThr_1 - SoftThr_0)) /
        (double)(1<<(16-PIX_BITS));
    }
  }

  /* Shift scale curve */
  for (i = 0; i < ScaleStart; i++) {
    ShiftScale[i] = 1024;  /* 1.0 Q10 */
  }

  for (i = ScaleStart; i < LTM_SS_TBL_SIZE; i++) {
    int32_t tmp = i - ScaleStart;
    /* scaled to Q10 */
    ShiftScale[i] = ShiftScaleBase +
      ShiftScaleMag * exp(-0.5 * (double)(tmp * tmp) / ScaleVar);
    ShiftScale[i] *= 1024;
  }

  return ret;
}

/** isp44_algo_ltm_execute:
 *
 *  @module:         mct module handle
 *  @stats_mask      stats mask
 *  @parsed_stats:   stats after parsing
 *  @algo_parm:      algorithm params
 *  @saved_algo_parm container to hold the output
 *
 *  Execute ltm algo
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp44_algo_ltm_execute(mct_module_t *module,
  mct_event_stats_isp_t *stats_data,
  isp_algo_params_t *algo_parm, isp_saved_algo_params_t *saved_algo_parm,
  void **output, uint32_t curr_frame_id, boolean svhdr_enb)
{
  boolean ret = TRUE;

  if (!saved_algo_parm || !curr_frame_id || !algo_parm) {
    ISP_ERR("failed: saved_algo_parm %p curr_frame_id %d algo_parm %p",
            saved_algo_parm, curr_frame_id, algo_parm);
    return FALSE;
  }

  *output = NULL;

  if (svhdr_enb && algo_parm->autoltm_params.auto_ltm_enable) {
    if (!saved_algo_parm->ltm_saved_algo_parm.autoltm_obj) {
      saved_algo_parm->ltm_saved_algo_parm.autoltm_obj = autoltm_init();
    }
    if(!saved_algo_parm->ltm_saved_algo_parm.autoltm_obj)
        return FALSE;
    /*LTM curve algo*/
    if ((stats_data->stats_mask & (1 << MSM_ISP_STATS_BHIST)) &&
      (stats_data->stats_data[MSM_ISP_STATS_BHIST].stats_buf) &&
      (stats_data->stats_mask & (1 << MSM_ISP_STATS_BG)) &&
      (stats_data->stats_data[MSM_ISP_STATS_BG].stats_buf))
    {
      ltm_curves_t ltm_curves;
      memset(&ltm_curves, 0, sizeof(ltm_curves));
      q3a_bg_stats_t* bg_stats = (q3a_bg_stats_t *) stats_data->
        stats_data[MSM_ISP_STATS_BG].stats_buf;
      q3a_bhist_stats_t* bhist_stats = (q3a_bhist_stats_t *)stats_data->
        stats_data[MSM_ISP_STATS_BHIST].stats_buf;

      autoltm_input_t autoltm_input;
      memset(&autoltm_input, 0, sizeof(autoltm_input_t));
      ISP_ERR("%s Executing Auto LTM",__func__);

      autoltm_input.statsR = bg_stats->bg_r_sum;
      autoltm_input.statsG = bg_stats->bg_gr_sum;
      autoltm_input.statsB = bg_stats->bg_b_sum;
      autoltm_input.statsR_cnt = bg_stats->bg_r_num;
      autoltm_input.statsG_cnt = bg_stats->bg_gr_num;
      autoltm_input.statsB_cnt = bg_stats->bg_b_num;
      autoltm_input.bhist = bhist_stats->bayer_gr_hist;
      ltm_curves.mask_rect_curve = (int *)saved_algo_parm->ltm_saved_algo_parm.mask_curve;
      ltm_curves.master_curve = saved_algo_parm->ltm_saved_algo_parm.master_curve;
      ltm_curves.master_scale = saved_algo_parm->ltm_saved_algo_parm.master_scale;
      ltm_curves.shift_curve = saved_algo_parm->ltm_saved_algo_parm.shift_curve;
      ltm_curves.shift_scale = saved_algo_parm->ltm_saved_algo_parm.shift_scale;

      autoltm_tuning_t autoltm_tuning_data;
      memset(&autoltm_tuning_data, 0, sizeof(autoltm_tuning_data));

      autoltm_tuning_data.halo_score_target = algo_parm->autoltm_params.halo_score_target;
      autoltm_tuning_data.brightness_target = algo_parm->autoltm_params.brightness_target;
      autoltm_tuning_data.contrast_target  = algo_parm->autoltm_params.contrast_target;
      autoltm_tuning_data.ltm_gain_limit  = algo_parm->autoltm_params.ltm_gain_limit;
      autoltm_tuning_data.low_code_tone_end  = algo_parm->autoltm_params.low_code_tone_end;
      autoltm_tuning_data.mid_tone_start  = algo_parm->autoltm_params.mid_tone_start;
      autoltm_tuning_data.mid_tone_end  = algo_parm->autoltm_params.mid_tone_end;
      autoltm_tuning_data.smear_prev_low_limit  = algo_parm->autoltm_params.smear_prev_low_limit;
      autoltm_tuning_data.smear_prev_high_limit  = algo_parm->autoltm_params.smear_prev_high_limit;

      ret = autoltm_main(saved_algo_parm->ltm_saved_algo_parm.autoltm_obj,
        &autoltm_input, &autoltm_tuning_data, &ltm_curves);
      if (ret < 0) {
        ret = FALSE;
      }
    }
  } else {
    q3a_ihist_stats_t *ihist_stats = NULL;

    /* decide if we need to execute ltm algo
      1. la curve
      2. ltm algo parm
      3. gamma table
      if either is missing, LTM algo will not execute. We will return
      TRUE while leaving output as NULL, indicating no output is generated */
    if (!saved_algo_parm->la_saved_algo_parm.is_la_curve_valid ||
        !algo_parm->is_ltm_algo_parm_valid ||
        !algo_parm->is_algo_gamma_valid) {
      return TRUE;
    }

    /*LTM curve algo*/
    if ((stats_data->stats_mask & (1 << MSM_ISP_STATS_IHIST)) &&
        (stats_data->stats_data[MSM_ISP_STATS_IHIST].stats_buf)) {
      ihist_stats = (q3a_ihist_stats_t *)
        stats_data->stats_data[MSM_ISP_STATS_IHIST].stats_buf;

      if (ret == TRUE) {
        /*calculate LTM mask clutter, save it for rectif_curve with BG stats*/
        ret = isp_ltm_algo_update_ltm_mask_clutter(module,
          ihist_stats, algo_parm,
          &saved_algo_parm->ltm_saved_algo_parm);
        if (ret == FALSE) {
          ISP_ERR("failed, update_ltm_mask_clutter, no LTM algo output update");
        }
      }

      if (ret == TRUE) {
        ret = isp_ltm_algo_update_ltm_mask_rectification_curve(module,
           ihist_stats, algo_parm,
           &saved_algo_parm->ltm_saved_algo_parm);
        if (ret == FALSE) {
          ISP_ERR("failed, update_ltm_mask_rectifi_curve, no LTM algo update");
        }
      }
    }

    if ((stats_data->stats_mask & (1 << MSM_ISP_STATS_BG)) &&
      (stats_data->stats_data[MSM_ISP_STATS_BG].stats_buf)) {
      if (ret == TRUE) {
        ret = isp_ltm_algo_update_ltm_mask_busyness(module,
          (q3a_bg_stats_t *)stats_data->stats_data[MSM_ISP_STATS_BG].stats_buf,
          algo_parm, &saved_algo_parm->ltm_saved_algo_parm);
        if (ret == FALSE) {
          ISP_ERR("failed, update_ltm_mask_busyness, no LTM algo output update");
        }
      }
    }

    if (ret == TRUE) {
      ret = isp_ltm_algo_update_ltm_tone_curves(module,
        algo_parm, saved_algo_parm);
      if (ret == FALSE) {
        ISP_ERR("failed, update_ltm_tone_curves, no LTM algo output update");
      }
    }
  }

  /* If everything passed, set the output */
  if (ret == TRUE) {
    saved_algo_parm->ltm_saved_algo_parm.mask_curve_size = LTM_MASK_TBL_SIZE;
    saved_algo_parm->ltm_saved_algo_parm.master_curve_size = LTM_MC_TBL_SIZE;
    saved_algo_parm->ltm_saved_algo_parm.master_scale_size = LTM_MS_TBL_SIZE;
    saved_algo_parm->ltm_saved_algo_parm.shift_curve_size = LTM_SC_TBL_SIZE;
    saved_algo_parm->ltm_saved_algo_parm.shift_scale_size = LTM_SS_TBL_SIZE;

    *output = (void *)&saved_algo_parm->ltm_saved_algo_parm;
  }

  return ret;
}
