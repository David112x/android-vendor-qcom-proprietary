/* la_algo.c
 *
 * Copyright (c) 2014, 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* mctl headers */
#include "mct_event_stats.h"
#include "media_controller.h"
#include "mct_list.h"

/* isp headers */
#include "isp_log.h"
#include "la40_algo.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_LA, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_LA, fmt, ##args)

#define BILINEAR_INTERPOLATION(v1, v2, ratio) \
  ( ((v1)*(1-ratio)) + ((ratio) * (v2)) )

const int is12bit=0;
/** isp40_algo_la_execute:
 *
 *  @module:         mct module handle
 *  @stats_mask      stats mask
 *  @parsed_stats:   stats after parsing
 *  @algo_parm:      algorithm params
 *  @saved_algo_parm container to hold the output
 *  @output          actual output payload to be sent
 *
 *  Execute la algo
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp40_algo_la_execute(mct_module_t *module,
  mct_event_stats_isp_t *stats_data,
  isp_algo_params_t *algo_parm,
  isp_saved_algo_params_t *saved_algo_parm,
  void **output,
  uint32_t curr_frame_id, boolean svhdr_enb);

/* algorithm structure to be extern-ed */
isp_algo_t algo_la40 = {
  "luma adaptation",                    /* name */
  &isp40_algo_la_execute,               /* algo func pointer */
  NULL,                                 /* stop session func pointer */
  MCT_EVENT_MODULE_ISP_LA_ALGO_UPDATE,  /* output type */
};

#define MIN_SAT_PIXELS_PERCENT .1
const int MaxPix = 4095;
static const uint16_t la_orig_gaussain_curve[256] = {
  64, 64, 64, 64, 64, 64, 64, 64, 64, 63, 63,
  63, 63, 63, 63, 63, 62, 62, 62, 62, 61, 61,
  61, 61, 60, 60, 60, 59, 59, 59, 58, 58, 58,
  57, 57, 56, 56, 56, 55, 55, 54, 54, 53, 53,
  53, 52, 52, 51, 51, 50, 50, 49, 49, 48, 48,
  47, 46, 46, 45, 45, 44, 44, 43, 43, 42, 42,
  41, 40, 40, 39, 39, 38, 38, 37, 37, 36, 35,
  35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30,
  29, 29, 28, 27, 27, 26, 26, 25, 25, 25, 24,
  24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19,
  19, 18, 18, 17, 17, 17, 16, 16, 15, 15, 15,
  14, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11,
  11, 11, 10, 10, 10,  9,  9,  9,  9,  8,  8,
  8,  8,  7,  7,  7,  7,  7,  6,  6,  6,  6,
  6,  6,  5,  5,  5,  5,  5,  5,  4,  4,  4,
  4,  4,  4,  4,  3,  3,  3,  3,  3,  3,  3,
  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,  2,
  2,  2,  2,  2,  2,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0};

/** isp_algo_get_min_pdf_count:
 *    @num_hist_pixels: number of pixels
 *
 * Return: min of Hist pixels
 **/
static uint64_t isp_algo_get_min_pdf_count(uint32_t num_hist_pixels)
{
  return (uint64_t)(MIN_SAT_PIXELS_PERCENT * (float)num_hist_pixels);
}

/** isp_la_algo_calc_la_curve:
 *
 *  @ihist_stats: ihist stats
 *  @la_algo_parm: algo input
 *  @Offset: la algo parm offset
 *  @output: la output curve
 * This function process histogram stats from the isp and
 * and calculates the LA curve
 *
 **/
static int isp_la_algo_calc_la_curve(uint32_t *ihist_stats,
  isp_la_8k_type *la_algo_parm, uint32_t Offset, void *output)
{
  int32_t rc = 0;
  uint16_t shift, CDF_50_threshold;
  uint64_t size = 0, capped_count, cap_inc, tmp64;
  uint32_t *hist = NULL;
  uint32_t *threshold = NULL;
  uint32_t avg_inc;
  /* full_bin: hist==thld, high_bin: hist>thld */
  uint32_t high_bin, full_bin, iter_cnt;
  uint64_t *H = NULL;
  uint64_t num_hist_pixels;
  float tmp0, tmp1, tmp, cap_adjust, cap_ratio, cap_orig, cap_max, cap;
  float backlight = la_algo_parm->low_beam;
  float frontlight = la_algo_parm->high_beam;
  uint8_t *la_curve = output;
  register int i;

  /* LA adjustable range [0, EqRange); y=x for luma values [EqRange, 255] */
  const int EqRange = 255;  /* Original: 255 */

  /* Adjusts width of Gaussian cap curve, default: 3.6 */
  cap_adjust = la_algo_parm->cap_adjust;
  /* Normal: 0.05; Backlight: 0.25 */
  cap_ratio  = la_algo_parm->cap_high;
  /* Height of Gaussian above 1.0, default: 1.5 */
  cap_orig   = la_algo_parm->cap_high;
  /* Normal: 3; Backlight: 12 */
  cap_max    = la_algo_parm->histogram_cap;
  /* Normal: 100; Backlight: 70 */
  CDF_50_threshold = la_algo_parm->CDF_50_thr;

  hist = (uint32_t*)CAM_MALLOC(256 * sizeof(uint32_t));
  if (!hist) {
    ISP_ERR("isp_stats_calc_hist_curve CAM_MALLOC failed");
    return -1;
  }

  H = (uint64_t*)CAM_MALLOC(256 * sizeof(uint64_t));
  if (!H) {
    ISP_ERR("isp_stats_calc_hist_curve CAM_MALLOC failed");
    rc = -1;
    goto ERROR;
  }

  threshold = (uint32_t*)CAM_MALLOC(256 * sizeof(uint32_t));
  if (!threshold) {
    ISP_ERR("isp_stats_calc_hist_curve CAM_MALLOC failed");
    rc = -1;
    goto ERROR;
  }

  ISP_DBG("cap_adjust %f,cap_high %f, histogram_cap %f, CDF_50_thr %d",
    la_algo_parm->cap_adjust, la_algo_parm->cap_high,
    la_algo_parm->histogram_cap, la_algo_parm->CDF_50_thr);

  /* Total histogram counts */
  for (i=0; i<256; i++) {
    /* Original histogram bins */
    hist[i] = ihist_stats[i];
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
  if (H[250] < isp_algo_get_min_pdf_count(num_hist_pixels)) {
    ISP_DBG("pdf count %llu", H[250]);
    rc = -1;
    goto ERROR;
  }

  if (H[250] == 0) {
    ISP_ERR("ALL Ihist stats = 0!!");
    rc = -1;
    goto ERROR;
  }

 /*Avoid the saturated pixels*/
  for (i=1; i<=250; i++) {
    H[i] = H[i] * 255 / H[250];
  }

  cap = cap_orig;
  if (H[50] > CDF_50_threshold) {
    cap = cap_orig + (H[50] - CDF_50_threshold) * cap_ratio;
    cap = (cap < cap_max)? cap : cap_max;
  }

  /* new curve */
  for (i=0; i<256; i++) {
    /* Nonlinear cap curve */
    /* interpolation */
    if ((int)(i * cap_adjust) < 255) {
      tmp0 = (float)la_orig_gaussain_curve[(int16_t)(i * cap_adjust)];
      tmp1 = (float)la_orig_gaussain_curve[(int16_t)(i * cap_adjust) + 1];
      tmp  = tmp0 + (tmp1 - tmp0) * (i * cap_adjust -
        (int16_t)(i * cap_adjust));
      threshold[i] =
        (uint32_t)((((uint32_t)(tmp * cap) + 64) * (size >> 8)) >> 6);
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

      ISP_DBG("iterartion %d,", iter_cnt);

      /* Distribute capped histogram counts to uncapped bins */
      if ((full_bin+high_bin) < 256)  /*alway true*/
        avg_inc = capped_count / (256-full_bin-high_bin);

      iter_cnt++;
      ISP_DBG("full_bin, %d, high_bin, %d, avg_inc, %d",
        full_bin, high_bin, avg_inc);
  } while (high_bin > 0 && iter_cnt < 10);

  /* Adjust histogram: Offset, Low light boost, High light boost */
  /* adjusted histogram */
  size = 0;
  for (i = 0; i < 256; i++) {
    hist[i] += Offset;
    /*assert((32 + backlight * 4) <= (256 - 32- (frontlight * 4)));*/
    if (i < (32 + (int)(backlight*4))) {
      /* Low light boost */
      hist[i] = (uint32_t)((hist[i]) *
        (1.0f + (float)(32 + backlight * 4 - i) * backlight * 0.4f/36.0f));
    }
    if (i > (256 - 32 - (int)(frontlight * 4))) {
      /* High light boost */
      hist[i] = (uint32_t)((hist[i])*(1.0f + (float)(i - (256 - 32 -
        (frontlight * 4))) * frontlight * 0.6f / 32.0f));
    }
    /* New total count */
    size += hist[i];
  }

  /* Compute LA curve / Compute target CDF */
  H[0] = hist[0];
  for (i = 1; i <= EqRange; i++) {
    H[i] = H[i-1] + hist[i];
  }

  /* Scale target CDF with enacted equalization range (default full 255) */
  /*scaled CDF */
  for (i = 0; i <= EqRange; i++) {
    if (H[EqRange] == 0) {
      ISP_ERR("H[EqRange] = 0\n");
      rc = -1;
      goto ERROR;
    }
    if (is12bit) {
      H[i] = MaxPix*EqRange*H[i]/(255*H[EqRange]);
    }
    else {
      H[i] = EqRange*H[i]/H[EqRange];
    }
  }

  for ( ; i < 256; i++) {
    /* Straight line y=x of slope 1 after EqRange */
    if (is12bit) {
      H[i] = MaxPix*i/255;
    } else {
      H[i] = i;
    }
  }

  /* Smooth target mapping function */
  for (iter_cnt = 0; iter_cnt < 1; iter_cnt++) {
    H[0]=0;
    H[1]=(0+H[1]+H[2])/3;
    for (i=2;i<254;i++) {
      H[i] = (H[i-2]+H[i-1]+H[i]+H[i+1]+H[i+2])/5;
    }
    if (is12bit) {
      H[254]=(H[253]+H[254]+MaxPix)/3;
      H[255] = MaxPix;
    }
    else {
      H[254]=(H[253]+H[254]+255)/3;
      H[255]=255;
    }
  }

  /* smoothed CDF (final output) */
  for (i=0; i < 256; i++) {
    la_curve[i] = H[i];
  }

ERROR:
  if (threshold) {
    free(threshold);
    threshold = NULL;
  }
  if (hist) {
    free(hist);
    hist = NULL;
  }
  if (H) {
    free(H);
    H = NULL;
  }

  return rc;
}

/** isp40_algo_la_execute:
 *
 *  @module:         mct module handle
 *  @stats_mask      stats mask
 *  @parsed_stats:   stats after parsing
 *  @algo_parm:      algorithm params
 *  @saved_algo_parm container to hold the output
 *
 *  Execute la algo
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp40_algo_la_execute(mct_module_t *module,
  mct_event_stats_isp_t *stats_data,
  isp_algo_params_t *algo_parm,
  isp_saved_algo_params_t *saved_algo_parm,
  void **output,
  uint32_t curr_frame_id, boolean svhdr_enb __unused)
{
  if (!module || !stats_data || !algo_parm || !output ||
      !saved_algo_parm) {
    ISP_ERR("failed, null pointer, %p %p %p %p %p",
      module, stats_data, algo_parm, output,
      saved_algo_parm);
    return FALSE;
  }
  if (!curr_frame_id) {
    ISP_ERR("failed: curr_frame_id 0");
    return FALSE;
  }

  *output = NULL;
  if ((stats_data->stats_mask & (1 << MSM_ISP_STATS_IHIST)) &&
    stats_data->stats_data[MSM_ISP_STATS_IHIST].stats_buf) {
    boolean  ret = TRUE;
    int32_t  N_pixels = 0, offset = 0;
    uint8_t *la_output_curve = saved_algo_parm->la_saved_algo_parm.la_curve;
    uint32_t i;
    q3a_ihist_stats_t *ihist_stats = NULL;

    ihist_stats = (q3a_ihist_stats_t *)
      stats_data->stats_data[MSM_ISP_STATS_IHIST].stats_buf;

    /* Reset N_pixel to match histogram for histogram offset */
    for (i=0; i < 256; i++){
      N_pixels += ihist_stats->histogram[i];
    }
    offset = (N_pixels / 256) * algo_parm->la_8k_algo_parm.offset;
    memset(la_output_curve, 0 , sizeof(uint8_t) * 256);
    if (algo_parm->la_8k_algo_parm.CDF_50_thr) {
       int rc = isp_la_algo_calc_la_curve(&ihist_stats->histogram[0],
              &algo_parm->la_8k_algo_parm, offset, la_output_curve);
       if (rc < 0) {
         ISP_ERR("failed: isp_algo_calc_la_curve, rc = %d", rc);
         return FALSE;
       }
    } else {
      ISP_DBG("invalid CDF_50_thr = %d",
        algo_parm->la_8k_algo_parm.CDF_50_thr);
    }
    saved_algo_parm->la_saved_algo_parm.is_la_curve_valid = TRUE;
    saved_algo_parm->la_saved_algo_parm.curve_size = sizeof(uint8_t) * 256;
    *output = (void *)&saved_algo_parm->la_saved_algo_parm;
  }

  return TRUE;
}
