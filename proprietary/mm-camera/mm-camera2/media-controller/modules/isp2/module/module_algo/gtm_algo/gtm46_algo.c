/* gtm_algo.c
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <math.h>

/* mctl headers */
#include "mct_event_stats.h"
#include "media_controller.h"
#include "mct_list.h"

/* isp headers */
#include "isp_log.h"
#include "gtm46_algo.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_GTM, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_GTM, fmt, ##args)

#ifndef MIN
  #define MIN(a,b)          (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
  #define MAX(a,b)          (((a) > (b)) ? (a) : (b))
#endif

#ifndef CLAMP
  #define CLAMP(x,min,max)  MAX (MIN (x, max), min)
#endif

#define MAX_PERCENTILE  0.999f
#define MIN_PERCENTILE  0.001f
#define MINVAL_TH       128

uint32_t xin_tbl[] = {
  0,    2,    4,    7,    13,    21,    31,    45,
  62,   83,   107,  137,  169,   201,   233,   265,
  297,  329,  361,  428,  463,   499,   538,   578,
  621,  665,  712,  760,  811,   863,   919,   976,
  1037, 1098, 1164, 1230, 1300,  1371,  1447,  1524,
  1605, 1687, 1774, 1861, 1954,  2047,  2146,  2245,
  2456, 2679, 2915, 3429, 3999,  4630,  5323,  6083,
  6911, 7812, 8787, 9840, 10975, 12193, 13499, 14894,
  16383};

/** isp_algo_gtm_execute:
 *
 *  @module:         mct module handle
 *  @stats_mask      stats mask
 *  @parsed_stats:   stats after parsing
 *  @algo_parm:      algorithm params
 *  @saved_algo_parm container to hold the output
 *  @output          actual output payload to be sent
 *
 *  Execute gtm algo
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_algo_gtm_execute(mct_module_t *module,
  mct_event_stats_isp_t *stats_data,
  isp_algo_params_t *algo_parm,
  isp_saved_algo_params_t *saved_algo_parm,
  void **output,
  uint32_t curr_frame_id, boolean svhdr_enb __unused);

isp_algo_t algo_gtm46 = {
  "global tone mapping",                 /* name */
  &isp_algo_gtm_execute,                 /* algo func pointer */
  NULL,                                  /* stop session func pointer */
  MCT_EVENT_MODULE_ISP_GTM_ALGO_UPDATE,  /* output type */
};

static boolean isp_algo_gtm_compute_yout(
  q3a_bhist_stats_t *bhist,
  isp_gtm_algo_params_t *param,
  isp_saved_gtm_params_t *output)
{
  uint32_t i, k;
  float middletone          = param->params.gtm_core.GTM_a_middletone;
  float key_min_th          = param->params.gtm_reserve.GTM_key_min_th;
  float key_max_th          = param->params.gtm_reserve.GTM_key_max_th;
  float temporal_w          = param->params.gtm_core.GTM_temporal_w;
  float middletone_w        = param->params.gtm_core.GTM_middletone_w;
  int32_t maxval_th         = param->params.gtm_reserve.GTM_maxval_th;
  int32_t yout_maxval       = param->params.gtm_reserve.GTM_Yout_maxval;

#if defined(CHROMATIX_VERSION) && ((CHROMATIX_VERSION >= 0x305))
  float  max_percentile    = param->params.gtm_core.GTM_max_percentile;
  float  min_percentile    = param->params.gtm_core.GTM_min_percentile;
  int32_t minval_th         = param->params.gtm_reserve.GTM_minval_th;
#else
  float  max_percentile    = MAX_PERCENTILE;
  float min_percentile    = MIN_PERCENTILE;
  int32_t  minval_th         = MINVAL_TH;
#endif
  uint32_t *gr_hist         = bhist->bayer_gr_hist;
  uint32_t num_bins         = bhist->num_bins;

  int32_t sum_hist = 0, n_hist = 0, max_v_hist = 0, min_v_hist = 0;
  int32_t dynamic_range;
  float sum = 0.0f;
  float max_v, key;
  int  yratio_i, yratio_q, yrslope_i, yrslope_q;
  int bin_size = 1;
  uint32_t bin_to_luma[1025];

  /* 8998  Only one channel (Y-channel) information is present
     and it is sent as part of the R-channel from the ISP.
     Other channels are invalid */
  if ((TRUE == bhist->bayer_r_hist_valid) &&
      (FALSE == bhist->bayer_gr_hist_valid) &&
      (FALSE == bhist->bayer_gb_hist_valid) &&
      (FALSE == bhist->bayer_b_hist_valid))
    {
      gr_hist = bhist->bayer_r_hist;
    }

  // map bin index to luma
  for (i = k = 0; i < 4096; i += bin_size)
  {
    bin_size = (i <  512) ? 1 : ((i < 1024) ? 4 : 8);
    bin_to_luma[k++]   = 4*i;
  }
  bin_to_luma[k] = 16384;

  /* use histogram to compute key */
  for (i = 0; i < num_bins; i++)
    n_hist += gr_hist[i];

  for (i = 0; i < num_bins; i++) {
    sum_hist += gr_hist[i];
    if (sum_hist <= n_hist * max_percentile)
      max_v_hist = bin_to_luma[i+1] - 1; ;
    if (sum_hist <= n_hist * min_percentile)
      min_v_hist = bin_to_luma[i];

    if(sum_hist == n_hist)
      sum += (gr_hist[i]) *  log((float)(1.0 * bin_to_luma[i+1] - 1));
    else
      sum += (gr_hist[i]) *  log((float)((bin_to_luma[i] + bin_to_luma[i+1])/2.0));
  }
  max_v_hist = MAX(max_v_hist, maxval_th);
  min_v_hist = MIN(min_v_hist, minval_th);
  dynamic_range = MAX(maxval_th, max_v_hist - min_v_hist);
  if (n_hist == 0 || dynamic_range == 0) {
    ISP_ERR("fails! n_hist = %d dynamic_range = %d", n_hist,dynamic_range);
    return FALSE;
  }
  key = exp(sum / n_hist);
  key = (maxval_th *key * middletone_w) / dynamic_range;
  key = CLAMP(key, key_min_th, key_max_th);

  /* temporal fileter control from module*/
  if (param->temporal_filter_enable == TRUE) {
    if (!param->is_prev_key_valid) {
      param->prev_key = key;
      param->prev_max_v_hist = max_v_hist;
      param->prev_min_v_hist = min_v_hist;
    }

    /* apply damping on key and max_v_hist */
    key = key + (param->prev_key - key) * temporal_w;
    max_v_hist = (int32_t)(max_v_hist + (param->prev_max_v_hist - max_v_hist) * temporal_w);
    min_v_hist = (int32_t)(min_v_hist + (param->prev_min_v_hist - min_v_hist) * temporal_w);
    param->prev_max_v_hist = max_v_hist;
    param->prev_min_v_hist = min_v_hist;
    param->prev_key = key;
     if (key  == 0.0f ) {
      ISP_ERR("fails! key = %f ", key);
      return FALSE;
    }
  }

  max_v = (middletone * max_v_hist) / key;
  /* compute yout */
  for (i = 0; i < sizeof(xin_tbl) / sizeof(uint32_t); i++) {
    int X_tmp = MAX(1, xin_tbl[i]);
    float xin = (middletone * X_tmp) / key;
    output->gtm_yout[i] = (yout_maxval * (1.0 + xin / (max_v * max_v)) * xin / (1.0f + xin));
  }
  output->is_key_valid = 1;
  output->key = key;
  output->max_v_hist = max_v_hist;
  output->min_v_hist = min_v_hist;

  return TRUE;
}

static boolean isp_algo_gtm_execute(
    mct_module_t            *module,
    mct_event_stats_isp_t   *stats_data,
    isp_algo_params_t       *algo_parm,
    isp_saved_algo_params_t *saved_algo_parm,
    void                    **output,
    uint32_t                curr_frame_id,
    boolean svhdr_enb __unused)
{
  RETURN_IF_NULL(output);
  *output = NULL;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(stats_data);
  RETURN_IF_NULL(algo_parm);
  RETURN_IF_NULL(saved_algo_parm);
  if (!curr_frame_id) {
    ISP_ERR("failed: curr_frame_id 0");
    return FALSE;
  }

  if (stats_data->stats_mask & (1 << MSM_ISP_STATS_BHIST) &&
    algo_parm->gtm.is_valid) {
    if (!isp_algo_gtm_compute_yout(
      (q3a_bhist_stats_t *)stats_data->
      stats_data[MSM_ISP_STATS_BHIST].stats_buf,
      &algo_parm->gtm, &saved_algo_parm->gtm_saved_algo_parm)) {
      ISP_ERR("Failed in gtm compute yout");
      return FALSE;
    }
    *output = &saved_algo_parm->gtm_saved_algo_parm;
  }

  return TRUE;
}
