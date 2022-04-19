/*
* Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "gtm46.h"
#include "gtm_curve.h"
#include "isp_zzhdr_tune_def.h"

isp_zzhdr_2d_lut_type zzhdr_gtm_tuning_data =
    #include "isp_zzhdr_tune_param.h"
;

static uint32_t gtm_xin_tbl[] = {
  0,    2,    4,    7,    13,    21,    31,    45,
  62,   83,   107,  137,  169,   201,   233,   265,
  297,  329,  361,  428,  463,   499,   538,   578,
  621,  665,  712,  760,  811,   863,   919,   976,
  1037, 1098, 1164, 1230, 1300,  1371,  1447,  1524,
  1605, 1687, 1774, 1861, 1954,  2047,  2146,  2245,
  2456, 2679, 2915, 3429, 3999,  4630,  5323,  6083,
  6911, 7812, 8787, 9840, 10975, 12193, 13499, 14894,
  16383};

/** gtm46_configure_from_chromatix_bit:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Enable / disable module based on chromatix bit
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean gtm46_configure_from_chromatix_bit(
  isp_sub_module_t *isp_sub_module)
{
  chromatix_parms_type      *chromatix_ptr = NULL;

  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;

  isp_sub_module->submod_enable = FALSE;
  isp_sub_module->chromatix_module_enable = FALSE;

  if (chromatix_ptr->chromatix_VFE.chromatix_gtm.gtm_enable > 0 ||
      chromatix_ptr->chromatix_VFE.chromatix_gtm_v2.gtm_v2_enable > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
  }

  return TRUE;
}
/** gtm46_interpolate_v2:
 *
 *
 **/
static void gtm46_interpolate(gtm46_algo_params_t *in1,
  gtm46_algo_params_t *in2, gtm46_algo_params_t *out, float ratio)
{
  out->reserve_params.GTM_maxval_th =
    LINEAR_INTERPOLATION_INT(in1->reserve_params.GTM_maxval_th,
                             in2->reserve_params.GTM_maxval_th, ratio);
  out->reserve_params.GTM_key_min_th =
    LINEAR_INTERPOLATION_INT(in1->reserve_params.GTM_key_min_th,
                             in2->reserve_params.GTM_key_min_th, ratio);
  out->reserve_params.GTM_key_max_th =
    LINEAR_INTERPOLATION_INT(in1->reserve_params.GTM_key_max_th,
                             in2->reserve_params.GTM_key_max_th, ratio);
  out->reserve_params.GTM_key_hist_bin_weight =
    LINEAR_INTERPOLATION(in1->reserve_params.GTM_key_hist_bin_weight,
                         in2->reserve_params.GTM_key_hist_bin_weight, ratio);
  out->reserve_params.GTM_Yout_maxval =
    LINEAR_INTERPOLATION_INT(in1->reserve_params.GTM_Yout_maxval,
                             in2->reserve_params.GTM_Yout_maxval, ratio);
  out->core_params.GTM_a_middletone =
    LINEAR_INTERPOLATION(in1->core_params.GTM_a_middletone,
                         in2->core_params.GTM_a_middletone, ratio);
  out->core_params.GTM_temporal_w =
    LINEAR_INTERPOLATION(in1->core_params.GTM_temporal_w,
                         in2->core_params.GTM_temporal_w, ratio);
  out->core_params.GTM_middletone_w =
    LINEAR_INTERPOLATION(in1->core_params.GTM_middletone_w,
                         in2->core_params.GTM_middletone_w, ratio);
  out->reserve_params.reserved_1 =
    LINEAR_INTERPOLATION(in1->reserve_params.reserved_1,
                         in2->reserve_params.reserved_1, ratio);
  out->reserve_params.reserved_2 =
    LINEAR_INTERPOLATION(in1->reserve_params.reserved_2,
                         in2->reserve_params.reserved_2, ratio);
  out->core_params.GTM_max_percentile=
    LINEAR_INTERPOLATION(in1->core_params.GTM_max_percentile,
                         in2->core_params.GTM_max_percentile, ratio);
  out->core_params.GTM_min_percentile =
    LINEAR_INTERPOLATION(in1->core_params.GTM_min_percentile,
                         in2->core_params.GTM_min_percentile, ratio);
  out->reserve_params.GTM_minval_th =
    LINEAR_INTERPOLATION(in1->reserve_params.GTM_minval_th,
                         in2->reserve_params.GTM_minval_th, ratio);
}

#if !OVERRIDE_FUNC

boolean gtm46_2d_apply_dmi(void *data1,
 void *data2)
{
  isp_sub_module_t *isp_sub_module;
  gtm46_t  *gtm = NULL;
  chromatix_parms_type  *chromatix_ptr = NULL;
  chromatix_videoHDR_reserve *chromatix_HDR_reserve = NULL;

  if (!data1 ||  !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }

  isp_sub_module = (isp_sub_module_t *)data1;
  gtm = (gtm46_t *)data2;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("Null pointer! %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_HDR_reserve = &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR.
    videoHDR_reserve_data;
  if (!chromatix_HDR_reserve) {
    ISP_ERR("Null pointer! %p", chromatix_HDR_reserve);
    return FALSE;
  }

  if ((zzhdr_gtm_tuning_data.zzhdr_v2_enable) &&
    (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
    isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG) &&
    (chromatix_HDR_reserve->hdr_MSB_align == 0)) {
     gtm46_pack_dmi_fix_curve(gtm->dmi_tbl, gtm->base, gtm->slope);
     return TRUE;
  }

  return FALSE;

}

boolean gtm46_v2_process_adj_regions(void *data,
  void *gtm46, uint32_t *trigger_idx_start, uint32_t *trigger_idx_end,
  float *ratio) {
  boolean                  ret               = TRUE;
  trigger_point_type      *gtm_trigger       = NULL;
  float                    start             = 0.0;
  float                    end               = 0.0;
  float                    aec_reference     = 0.0;
  uint32_t                 i                 = 0;
  gtm46_t                  *gtm = NULL;
  chromatix_GTM_v2_type *gtm_v2 = NULL;

  if (!data || !gtm46) {
    ISP_ERR("failed: %p %p", data, gtm46);
    return FALSE;
  }

  gtm = (gtm46_t*)gtm46;
  gtm_v2 = (chromatix_GTM_v2_type *)data;
  for (i = 0; i < MAX_SETS_FOR_GTM_EXP_ADJ; i++) {

    if (i == MAX_SETS_FOR_GTM_EXP_ADJ - 1) {
      /* falls within region 3 but we do not have any more
         adjacent regions */
      *ratio         = 0.0f;
      *trigger_idx_start = MAX_SETS_FOR_GTM_EXP_ADJ - 1;
      *trigger_idx_end = MAX_SETS_FOR_GTM_EXP_ADJ - 1;
      break;
    }

    switch (gtm_v2->control_gtm_v2) {
    case CONTROL_LUX_IDX:
      /* lux index based */
      aec_reference = gtm->aec_update.lux_idx;
      gtm_trigger   = &(gtm_v2->gtm_v2[i].gtm_v2_trigger);
      start         = gtm_trigger->lux_index_start;
      end           = gtm_trigger->lux_index_end;
      ISP_DBG("lux base, lux idx %f start:%f end:%f",aec_reference, start, end);
      break;
    case CONTROL_GAIN:
      /* Gain based */
      aec_reference = gtm->aec_update.sensor_gain;
      gtm_trigger   = &(gtm_v2->gtm_v2[i].gtm_v2_trigger);
      start         = gtm_trigger->gain_start;
      end           = gtm_trigger->gain_end;
      break;
    default:
      ISP_ERR("Incorrect tuning control type %d", gtm_v2->control_gtm_v2);
      ret = FALSE;
      goto error;
    }

    /* index is within interpolation range, find ratio */
    if ((aec_reference > start) && (aec_reference < end)) {
      *ratio = (end - aec_reference)/(end - start);
      *trigger_idx_start = i;
      *trigger_idx_end = i+1;
      break;
    }
    if (aec_reference < start || (F_EQUAL(aec_reference, start))) {
      *trigger_idx_start = i;
      *trigger_idx_end = i;
      *ratio = 0.0f;
      break;
    }
  }

  ISP_DBG("%s [%f - %f - %f] = %f, t_idx1=%d t_idx2=%d", __func__, start, aec_reference, end,
    *ratio, *trigger_idx_start, *trigger_idx_end);

error:
  return ret;
}

void gtm_generate_manual_lut(gtm46_t  *gtm,
  gtm46_algo_params_t   *algo_params)
{
  float GTM_a_middletone;
  float GTM_middletone_w;
  int32_t Yratio_base[GTM_LUT_NUM_BIN] = {0};
  int32_t  Yratio_slope[GTM_LUT_NUM_BIN-1] = {0};
  int  x_delta, temp;
  int i;
  int yratio_q, yrslope_i, yrslope_q;
  //fixed uneven 65 LUT X_in points
  int LUT_bin   = GTM_LUT_NUM_BIN;

  int    x_in[65] = {
    0,    2,   4,    7,    13,    21,    31,    45,
    62,   83,  107,  137,  169,   201,   233,   265,
    297,  329,  361,  428,  463,  499,   538,   578,
    621,  665,  712,  760,  811,  863,   919,   976,
    1037, 1098, 1164, 1230, 1300, 1371,  1447,  1524,
    1605, 1687, 1774, 1861, 1954, 2047,  2146,  2245,
    2456, 2679, 2915, 3429, 3999, 4630,  5323,  6083,
    6911, 7812, 8787, 9840, 10975, 12193, 13499, 14894,
    16383};
  yratio_q     = 12;
  yrslope_i    = 6;
  yrslope_q    = 20;


  GTM_a_middletone  = algo_params->core_params.GTM_a_middletone;
  GTM_a_middletone   = Clamp(GTM_a_middletone, 0.0, 1.0);

  for ( i = 0; i < GTM_LUT_NUM_BIN; i++ )
    {
        Yratio_base[i] = gtm->yratio_base_manual[i];
    }

  for ( i = 0; i < GTM_LUT_NUM_BIN; i++ )
    {
        Yratio_base[i] = Round(GTM_a_middletone*(float)(gtm->yratio_base_manual[i]-4096)+4096);
    }

  for (i=0; i < (LUT_bin-1)-1; i++)
   {
       x_delta = x_in[i+1] - x_in[i];

       if (x_delta > 0)
       {
           temp = (Yratio_base[i+1] - Yratio_base[i])
             * (1 << (yrslope_q - yratio_q));
           Yratio_slope[i] = (temp >= 0) ? ((temp + x_delta/2) / x_delta)
                             : ((temp - x_delta/2) / x_delta);
       }
       else
       {
           ISP_ERR("Error: vfe_gtm_hw_prelim sees x_delta = %d <= 0", x_delta);
           Yratio_slope[i] = 0;
       }

       if (Yratio_slope[i] >= ( 1 << (yrslope_i + yrslope_q - 1))
               || Yratio_slope[i] <  (-1 << (yrslope_i + yrslope_q - 1)))
           Yratio_slope[i] = (Yratio_slope[i] > 0) ?
                             ((1 << (yrslope_i+yrslope_q-1)) - 1) /* clip to max */
                             : -(1 << (yrslope_i+yrslope_q-1));  /* clip to max */
   }
   //printf("Yratio_slope");
   //copy to hardware config, gtm_en and lut_bank filled by read_config
   for (i=0; i < (GTM_LUT_NUM_BIN - 1); i++)
   {
       gtm->dmi_tbl[i]   = x_in[i];
       gtm->base[i]  = Yratio_base[i];
       gtm->slope[i] = Yratio_slope[i];
       //printf("%d, ", gtmout->yratio_slope[i]);
   }

}


void gtm_generate_lut(gtm46_t  *gtm,
  gtm46_algo_params_t   *algo_params, int hdr_mode)
{
  float GTM_a_middletone;
  float GTM_middletone_w;
  int32_t Yratio_base[GTM_LUT_NUM_BIN] ={0} ;
  int32_t  Yratio_slope[GTM_LUT_NUM_BIN-1] = {0};
  int  x_delta, temp;
  int i;
  int  yratio_q, yrslope_i, yrslope_q;
  float base_ratio = 1.0f, extra_ratio;
  //fixed uneven 65 LUT X_in points
  int LUT_bin   = GTM_LUT_NUM_BIN;
  float hdr_exp_ratio;

  int    x_in[65] = {
    0,    2,   4,    7,    13,    21,    31,    45,
    62,   83,  107,  137,  169,   201,   233,   265,
    297,  329,  361,  428,  463,  499,   538,   578,
    621,  665,  712,  760,  811,  863,   919,   976,
    1037, 1098, 1164, 1230, 1300, 1371,  1447,  1524,
    1605, 1687, 1774, 1861, 1954, 2047,  2146,  2245,
    2456, 2679, 2915, 3429, 3999, 4630,  5323,  6083,
    6911, 7812, 8787, 9840, 10975, 12193, 13499, 14894,
    16383};
  yratio_q     = 12;
  yrslope_i    = 6;
  yrslope_q    = 20;

  GTM_a_middletone  = algo_params->core_params.GTM_a_middletone;
  GTM_middletone_w  = algo_params->core_params.GTM_middletone_w;
  hdr_exp_ratio = gtm->hdr_sensitivity_ratio;
  if (hdr_mode) {
    ///  GTM experiment start - GUOL
    if (hdr_exp_ratio!=0)
        base_ratio = 8.0f/hdr_exp_ratio;
    else
        base_ratio = 1.0;
  }
  GTM_a_middletone   = Clamp(GTM_a_middletone, 0.0, 2.0);
  GTM_middletone_w   = Clamp(GTM_middletone_w, 0.0, 64.0);
  extra_ratio = hdr_exp_ratio * GTM_a_middletone; // 0.25

  for (i=0; i < GTM_LUT_NUM_BIN; i++) {
      if (i <= (int)(GTM_middletone_w+0.5)) // 8
          Yratio_base[i] = 4096*(base_ratio+extra_ratio);
      else
          Yratio_base[i] = 4096*(base_ratio+extra_ratio*(float)
            ((GTM_LUT_NUM_BIN-1)-1-i)/
            ((GTM_LUT_NUM_BIN -1)-(int)(GTM_middletone_w+0.5)-1));
  }

  for (i=0; i < (LUT_bin-1); i++)
   {
       x_delta = x_in[i+1] - x_in[i];

       if (x_delta > 0)
       {
           temp = (Yratio_base[i+1] - Yratio_base[i])
             * (1 << (yrslope_q - yratio_q));
           Yratio_slope[i] = (temp >= 0) ? ((temp + x_delta/2) / x_delta)
                             : ((temp - x_delta/2) / x_delta);
       }
       else
       {
           ISP_ERR("Error: vfe_gtm_hw_prelim sees x_delta = %d <= 0", x_delta);
           Yratio_slope[i] = 0;
       }

       if (Yratio_slope[i] >= ( 1 << (yrslope_i + yrslope_q - 1))
               || Yratio_slope[i] <  (-1 << (yrslope_i + yrslope_q - 1)))
           Yratio_slope[i] = (Yratio_slope[i] > 0) ?
                             ((1 << (yrslope_i+yrslope_q-1)) - 1) /* clip to max */
                             : -(1 << (yrslope_i+yrslope_q-1));  /* clip to max */
   }
   //printf("Yratio_slope");
   //copy to hardware config, gtm_en and lut_bank filled by read_config
   for (i=0; i < (GTM_LUT_NUM_BIN - 1); i++)
   {
       gtm->dmi_tbl[i]   = x_in[i];
       gtm->base[i]  = Yratio_base[i];
       gtm->slope[i] = Yratio_slope[i];
       //printf("%d, ", gtmout->yratio_slope[i]);
   }

}

boolean gtm46_apply_hdr_effects(void *data1,
 void *data2, void *data3)
{
  isp_sub_module_t *isp_sub_module;
  gtm46_t          *gtm = NULL;
  gtm46_algo_params_t      *algo_params;
  boolean   ret  = TRUE;
  float     ratio = 0.0f;
  int hdr_mode = 0;
  chromatix_parms_type  *chromatix_ptr = NULL;
  chromatix_videoHDR_reserve *chromatix_HDR_reserve = NULL;

  if (!data1 || !data2|| !data3) {
    ISP_ERR("failed: %p %p", data1, data2, data3);
    return FALSE;
  }

  isp_sub_module = (isp_sub_module_t *)data1;
  gtm = (gtm46_t *)data2;
  algo_params = (gtm46_algo_params_t *)data3;

  chromatix_ptr = isp_sub_module->chromatix_ptrs.chromatixPtr;
  if (!chromatix_ptr) {
    ISP_ERR("Null pointer! %p", chromatix_ptr);
    return FALSE;
  }

  chromatix_HDR_reserve = &chromatix_ptr->chromatix_post_processing.chromatix_video_HDR.
    videoHDR_reserve_data;
  if (!chromatix_HDR_reserve) {
    ISP_ERR("Null pointer! %p", chromatix_HDR_reserve);
    return FALSE;
  }

  if ((zzhdr_gtm_tuning_data.zzhdr_v2_enable) &&
    (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR ||
    isp_sub_module->hdr_mode == CAM_SENSOR_HDR_ZIGZAG) &&
    (chromatix_HDR_reserve->hdr_MSB_align == 0)) {
      hdr_mode = 1;
  }

  if (hdr_mode)
  {
    gtm_generate_lut(gtm, algo_params, hdr_mode);
  }

  if (gtm->gtm_manual_curve_enable)
  {
    gtm_generate_manual_lut(gtm, algo_params);
  }

  return ret;
}
#endif

/** gtm46_v2_process_regions:
 *
 *  @gtm_v1: pointer to the regions used in the v1 version.
 *           NULL value indicates use of v2 version
 *  @gtm_exp_v2: regions used in v2 version. If this is NULL
 *           use the v1 version.
 *  @local_algo_params: the output algo params that are
 *           computed/interpolated
 *  Use the control type (gain, lux index, exp ratio) to compute
 *  or interpolate the algo parameters.
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean gtm46_v2_process_regions(gtm46_t *gtm,
  chromatix_GTM_type *gtm_v1, chromatix_GTM_v2_type *gtm_v2,
  chromatix_GTM_v2_exp_type *gtm_exp_v2,
  gtm46_algo_params_t *local_algo_params) {

  boolean                  ret               = TRUE;
  trigger_point_type      *gtm_trigger       = NULL;
  trigger_point2_type     *gtm_trigger2      = NULL;
  float                    start             = 0.0;
  float                    end               = 0.0;
  float                    ratio             = 0.0;
  float                    aec_reference     = 0.0;
  uint32_t                 i                 = 0;
  uint8_t                  trigger_index     = MAX_SETS_FOR_TONE_NOISE_ADJ + 1;
  uint8_t                  control_type      = CONTROL_GAIN;
  boolean                  use_gtm_v2        = TRUE;
  gtm46_algo_params_t      algo_params_1 , algo_params_2;

  memset(&algo_params_1, 0, sizeof(algo_params_1));
  memset(&algo_params_2, 0, sizeof(algo_params_2));
  gtm->mod_enable = TRUE;

  RETURN_IF_NULL(local_algo_params);

  if (gtm_exp_v2) {
    use_gtm_v2 = TRUE;
    /* For v2 version the input corresponds to 6 regions that
       need to triggered based on exp_ratio */
    control_type = CONTROL_AEC_EXP_SENSITIVITY_RATIO;
  } else {
    use_gtm_v2 = FALSE;
    control_type = gtm_v1->control_gtm;
  }

  for (i = 0; i < MAX_SETS_FOR_TONE_NOISE_ADJ; i++) {

    if (i == MAX_SETS_FOR_TONE_NOISE_ADJ - 1) {
      /* falls within region 6 but we do not use trigger points in the region */
      ratio         = 0.0f;
      trigger_index = MAX_SETS_FOR_TONE_NOISE_ADJ - 1;
      break;
    }

    switch (control_type) {
    case CONTROL_LUX_IDX:
      /* lux index based */
      aec_reference = gtm->aec_update.lux_idx;
      gtm_trigger   = &(gtm_v1->gtm[i].gtm_trigger);
      start         = gtm_trigger->lux_index_start;
      end           = gtm_trigger->lux_index_end;
      ISP_DBG("LUX reference:%f start:%f end:%f",aec_reference, start, end);
      break;
    case CONTROL_GAIN:
      /* Gain based */
      aec_reference = gtm->aec_update.sensor_gain;
      gtm_trigger   = &(gtm_v1->gtm[i].gtm_trigger);
      start         = gtm_trigger->gain_start;
      end           = gtm_trigger->gain_end;
      ISP_DBG("GAIN reference:%f start:%f end:%f",aec_reference, start, end);
      break;
    case CONTROL_AEC_EXP_SENSITIVITY_RATIO:
      /* AEC sensitivity ratio based*/
      aec_reference = gtm->aec_update.hdr_sensitivity_ratio;
      if (use_gtm_v2)
        gtm_trigger2 = &(gtm_exp_v2[i].aec_sensitivity_ratio_v2);
      else
        gtm_trigger2 = &(gtm_v1->gtm[i].aec_sensitivity_ratio);
      start         = gtm_trigger2->start;
      end           = gtm_trigger2->end;
      ISP_DBG("EXP reference:%f start:%f end:%f",aec_reference, start, end);
      break;
    default:
      ret = FALSE;
      goto error;
    }

    /* index is within interpolation range, find ratio */
    if ((aec_reference > start) && (aec_reference < end)) {
      ratio = (end - aec_reference)/(end - start);
      trigger_index = i;
      ISP_HIGH("%s [%f - %f - %f] = %f, trigger_idx=%d", __func__, start, aec_reference, end,
        ratio, trigger_index);
      break;
    }
    if (aec_reference < start || (F_EQUAL(aec_reference, start))){
      trigger_index = i;
      ratio = 1.0f;
      break;
    }
  }

  if (!use_gtm_v2) {
    /* Region enable/disable is only available for v1 version. If the region
       is disabled skip operations and disable module */
    gtm->mod_enable = gtm_v1->gtm[trigger_index].rgn_enable;
    if (!gtm->mod_enable) {
        ISP_ERR("Region %d is disabled in chromatix", trigger_index);
        goto error;
    }
  }

  if (trigger_index + 1 >= MAX_SETS_FOR_TONE_NOISE_ADJ) {
    ISP_HIGH("Reached final region %d, no interpolation aec_ratio:%f", trigger_index,
      aec_reference);
    if (use_gtm_v2) {
      memcpy(&local_algo_params->core_params, &gtm_exp_v2[trigger_index].gtm_core_v2,
        sizeof(chromatix_GTM_core));
      memcpy(&local_algo_params->reserve_params, &gtm_exp_v2[trigger_index].gtm_reserve_v2,
        sizeof(chromatix_GTM_reserve));
    } else {
      memcpy(&local_algo_params->core_params, &gtm_v1->gtm[trigger_index].gtm_core,
        sizeof(chromatix_GTM_core));
      memcpy(&local_algo_params->reserve_params, &gtm_v1->gtm[trigger_index].gtm_reserve,
        sizeof(chromatix_GTM_reserve));
    }
    goto error;
  }

  /* Update the params only if the region is enabled */
  if (use_gtm_v2) {
    algo_params_1.core_params = gtm_exp_v2[trigger_index].gtm_core_v2;
    algo_params_1.reserve_params = gtm_exp_v2[trigger_index].gtm_reserve_v2;
    algo_params_2.core_params = gtm_exp_v2[trigger_index + 1].gtm_core_v2;
    algo_params_2.reserve_params = gtm_exp_v2[trigger_index + 1].gtm_reserve_v2;
  } else {
    algo_params_1.core_params = gtm_v1->gtm[trigger_index].gtm_core;
    algo_params_1.reserve_params = gtm_v1->gtm[trigger_index].gtm_reserve;
    algo_params_2.core_params = gtm_v1->gtm[trigger_index + 1].gtm_core;;
    algo_params_2.reserve_params = gtm_v1->gtm[trigger_index + 1].gtm_reserve;
  }

  gtm46_interpolate(&algo_params_1,
                    &algo_params_2,
                    local_algo_params,
                    ratio);

error:
  return ret;
}

/** gtm46_compute_algo_params:
 *
 *  Call the function to compute the algo parameters.
 *  The v1 version has 6 regions that can be used to
 *  get/interpolate the parameters.
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean gtm46_compute_algo_params(gtm46_t *gtm,
  isp_gtm_algo_params_t *algo_param, chromatix_GTM_type *chromatix_gtm) {

  boolean              ret               = TRUE;
  gtm46_algo_params_t  local_algo_params;

  RETURN_IF_NULL(algo_param);

  if (!gtm->is_aec_update_valid) {
    /* default to NORMAL_LIGHT when aec_update isn't available yet */
    algo_param->params = chromatix_gtm->gtm[2];
  } else {
    ret = gtm46_v2_process_regions(gtm, chromatix_gtm, NULL, NULL,
        &local_algo_params);
  }

  algo_param->params.gtm_core = local_algo_params.core_params;
  algo_param->params.gtm_reserve = local_algo_params.reserve_params;
  gtm->gtm_manual_curve_enable = 0;

  return ret;
}

/** gtm46_compute_algo_params:
 *
 *  Call the function to compute the algo parameters.
 *  The v2 version has 3x6 regions that can be used to
 *  get/interpolate the parameters.
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean gtm46_v2_compute_algo_params(isp_sub_module_t  *isp_sub_module,
  gtm46_t *gtm, isp_gtm_algo_params_t *algo_param,
  chromatix_GTM_v2_type *chromatix_gtm_v2) {
  boolean              ret = TRUE;
  trigger_ratio_t      trigger_ratio;
  gtm46_algo_params_t  algo_params_1, algo_params_2;
  gtm46_algo_params_t  local_algo_params;
  uint32_t             trigger_idx_start = 0, trigger_idx_end = 0;
  float                adj_rgn_ratio = 0.0f;

  RETURN_IF_NULL(algo_param);
  memset(&algo_params_1, 0, sizeof(gtm46_algo_params_t));
  memset(&algo_params_2, 0, sizeof(gtm46_algo_params_t));

  ISP_DBG("lux_idx %f, real_gain %f, sensor_gain %f",
    gtm->aec_update.lux_idx, gtm->aec_update.real_gain,
    gtm->aec_update.sensor_gain);

  /* The GTM V2 version has 3x6 regions i.e. has
     [MAX_SETS_FOR_GTM_EXP_ADJ][MAX_SETS_FOR_TONE_NOISE_ADJ]
     The inner/2nd dimension is triggered based on exposure ratio/aec sensitivity ratio
     (aec_sensitivity_ratio_v2) and has 6 regions.
     The outer is based on gain or lux index (gtm_v2_trigger) and has 3 regions.
  */
    if (gtm->ext_func_table &&
      gtm->ext_func_table->gtm_process_adj_regions) {
      ret = gtm->ext_func_table->gtm_process_adj_regions(chromatix_gtm_v2, gtm,
      &trigger_idx_start, &trigger_idx_end, &adj_rgn_ratio);
	}
  if (ret != TRUE) {
    ISP_ERR("get adj region ratio error");
    return TRUE;
  }

  if (trigger_idx_start != trigger_idx_end) {
    /* Compute the params based on exp_ratio for each lighting*/
    ret = gtm46_v2_process_regions(gtm, NULL, chromatix_gtm_v2,
      chromatix_gtm_v2->gtm_v2[trigger_idx_start].gtm_exp_data,
      &algo_params_1);
    ret |= gtm46_v2_process_regions(gtm, NULL, chromatix_gtm_v2,
      chromatix_gtm_v2->gtm_v2[trigger_idx_end].gtm_exp_data,
      &algo_params_2);
    /* Interpolate the algo params obtained above based on
       gain/lux index */
    gtm46_interpolate(&algo_params_1,
                      &algo_params_2,
                      &local_algo_params,
                      adj_rgn_ratio);
  } else {
    ret = gtm46_v2_process_regions(gtm, NULL, chromatix_gtm_v2,
      chromatix_gtm_v2->gtm_v2[trigger_idx_start].gtm_exp_data,
      &local_algo_params);
  }

  gtm->gtm_data.GTM_key_max_th =
    local_algo_params.reserve_params.GTM_key_max_th;
  gtm->gtm_data.GTM_max_percentile =
    local_algo_params.core_params.GTM_max_percentile;
  gtm->gtm_data.GTM_min_percentile =
    local_algo_params.core_params.GTM_min_percentile;
  gtm->gtm_data.GTM_key_hist_bin_weight =
    local_algo_params.reserve_params.GTM_key_hist_bin_weight;
  algo_param->params.gtm_core = local_algo_params.core_params;
  algo_param->params.gtm_reserve = local_algo_params.reserve_params;

  if (gtm->ext_func_table && gtm->ext_func_table->gtm_apply_hdr_effects) {
    ret = gtm->ext_func_table->gtm_apply_hdr_effects(isp_sub_module, gtm, &local_algo_params);
    if (ret == FALSE) {
      ISP_ERR("failed: get_2d_interpolation");
      return FALSE;
    }
  }
  return ret;
}

boolean gtm46_fill_dmi_table(
  mct_module_t *module,
  gtm46_t               *gtm,
  isp_gtm_algo_params_t *algo_param,
  isp_sub_module_t      *isp_sub_module,
  chromatix_parms_type  *chromatix_ptr,
  mct_event_t *event
  )
{
    trigger_point_type      *gtm_trigger       = NULL;
    trigger_point2_type     *gtm_trigger2      = NULL;
    float                    start             = 0.0;
    float                    end               = 0.0;
    float                    ratio             = 0.0;
    uint32_t                 i                 = 0;
    float                    aec_reference     = 1.0;
    uint8_t                  trigger_index     = MAX_SETS_FOR_TONE_NOISE_ADJ + 1;
    boolean                  ret = TRUE;
    chromatix_GTM_type      *chromatix_gtm     = NULL;
    chromatix_GTM_v2_type   *chromatix_gtm_v2 =  NULL;

    chromatix_gtm = &chromatix_ptr->chromatix_VFE.chromatix_gtm;
    chromatix_gtm_v2 = &chromatix_ptr->chromatix_VFE.chromatix_gtm_v2;
    gtm->mod_enable = isp_sub_module->submod_enable;

    if (gtm->enable_adrc) {
      gtm46_derive_and_pack_lut_adrc_based(gtm);
    } else if (gtm->use_gtm_fix_curve == 0){
        algo_param->is_valid = 1;
        if (gtm->algo_update_valid){
          algo_param->is_prev_key_valid = gtm->algo_output.is_key_valid;
          algo_param->prev_key          = gtm->algo_output.key;
          algo_param->prev_max_v_hist   = gtm->algo_output.max_v_hist;
          algo_param->prev_min_v_hist   = gtm->algo_output.min_v_hist;
        } else {
          algo_param->is_prev_key_valid = 1;
          algo_param->prev_key = 170.06;
          algo_param->prev_max_v_hist = 2863;
          algo_param->prev_min_v_hist = 0;
        }
        /*enable temporal filter only in video use case*/
        algo_param->temporal_filter_enable = FALSE;

        if (chromatix_gtm_v2->gtm_v2_enable) {
            if (!gtm->is_aec_update_valid) {
              /* default to NORMAL_LIGHT when aec_update isn't available yet */
              algo_param->params.gtm_core =
                chromatix_gtm_v2->gtm_v2[NORMAL_LIGHT].gtm_exp_data[2].gtm_core_v2;
              algo_param->params.gtm_reserve =
                chromatix_gtm_v2->gtm_v2[NORMAL_LIGHT].gtm_exp_data[2].gtm_reserve_v2;
            } else
              ret = gtm46_v2_compute_algo_params(isp_sub_module, gtm, algo_param, chromatix_gtm_v2);
        } else {
            if (!gtm->is_aec_update_valid) {
                  algo_param->params = chromatix_gtm->gtm[2];
            } else
              ret = gtm46_compute_algo_params(gtm, algo_param, chromatix_gtm);
        }
        if (ret == FALSE) {
          ISP_ERR("failed: compute_algo_params ret %d, gtm_v2_enable:%d",
            ret, chromatix_gtm_v2->gtm_v2_enable);
          return FALSE;
        }

        if (chromatix_gtm_v2->gtm_v2_enable &&
            gtm->ext_func_table && gtm->ext_func_table->gtm_apply_hdr_dmi) {
          ret = gtm->ext_func_table->gtm_apply_hdr_dmi(isp_sub_module, gtm);
          if (ret == FALSE) {
			  if (!gtm->algo_update_valid){
			   /* Calculate the default curve */
				if (!gtm46_calculate_default_cuve(gtm,algo_param)){
				  ISP_ERR("failed in calculating default curve \n");
				}
			  }

			  gtm46_pack_dmi(gtm->algo_output.gtm_yout, gtm_xin_tbl,
				  gtm->dmi_tbl, gtm->base, gtm->slope);
          }
        } else {
        /* pack DMI table */
        if (!gtm->algo_update_valid){
         /* Calculate the default curve */
          if (!gtm46_calculate_default_cuve(gtm,algo_param)){
            ISP_ERR("failed in calculating default curve \n");
          }
        }

        gtm46_pack_dmi(gtm->algo_output.gtm_yout, gtm_xin_tbl,
          gtm->dmi_tbl, gtm->base, gtm->slope);
      }
    } else {
      /* Calling this function just to check for region enable/disable in case of v1.
         In practice GTM module itself might be disabled in chromatix so execution
         never reaches this point */
      if (!chromatix_gtm_v2->gtm_v2_enable){
        ret = gtm46_compute_algo_params(gtm, algo_param, chromatix_gtm);
        if (ret == FALSE)
          ISP_ERR("failed: compute_algo_params ret %d, gtm_v2_enable:%d",
            ret, chromatix_gtm_v2->gtm_v2_enable);
      }
      /* pack DMI table */
      gtm46_pack_dmi_fix_curve(gtm->dmi_tbl,
        yratio_base, yratio_slope);
    }

    if (!gtm->mod_enable) {
      ISP_ERR("Region not enabled in GTM. Disable Module");
      isp_sub_module->submod_enable = FALSE;
      isp_sub_module->update_module_bit = TRUE;
      algo_param->is_valid = 0;
      ret = FALSE;
    }

    return ret;
}

