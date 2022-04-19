/* gtm46_ext.c
 *
 * Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <unistd.h>
#include <math.h>

/* mctl headers */

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_GTM, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_GTM, fmt, ##args)

/* isp headers */
#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "gtm46.h"
#include "gtm_curve.h"
#include "isp_pipeline_reg.h"
#include "isp_defs.h"

#ifdef ZZHDR_CHROMATIX_EXTN
/** gtm46_copy_core_reserve_param
  * Copy the reserved GTM params
  *
  **/
static boolean gtm46_copy_core_reserve_params(chromatix_GTM_core   *out_core,
  chromatix_GTM_reserve  *out_reserve,
  chromatix_GTM_hdr_core *in_core,
  chromatix_GTM_hdr_reserve *in_reserve)
{
  if (!out_core || !out_reserve || !in_core || !in_reserve){
    ISP_ERR("out %p %p  in_core %p in_reserve %p ",
            out_core, out_reserve, in_core, in_reserve);
    return FALSE;
  }

  out_core->GTM_a_middletone = in_core->GTM_a_middletone;
  out_core->GTM_max_percentile = in_core->GTM_max_percentile;
  out_core->GTM_middletone_w = in_core->GTM_middletone_w;
  out_core->GTM_min_percentile = in_core->GTM_min_percentile;
  out_core->GTM_temporal_w = in_core->GTM_temporal_w;

  out_reserve->GTM_key_hist_bin_weight = in_reserve->GTM_key_hist_bin_weight;
  out_reserve->GTM_key_max_th  = in_reserve->GTM_key_max_th;
  out_reserve->GTM_key_min_th  = in_reserve->GTM_key_min_th;
  out_reserve->GTM_maxval_th   = in_reserve->GTM_maxval_th;
  out_reserve->GTM_minval_th   = in_reserve->GTM_minval_th;
  out_reserve->GTM_Yout_maxval = in_reserve->GTM_Yout_maxval;
  out_reserve->reserved_1      = in_reserve->reserved_1;
  out_reserve->reserved_2      = in_reserve->reserved_2;

  return TRUE;
}

/** gtm46_interpolate_v2:
  * interpolate the values based on ratio
  *
  **/
 static void gtm46_interpolate_hdr_v2(isp_gtm_algo_params_t *in1,
   isp_gtm_algo_params_t *in2, isp_gtm_algo_params_t *out, float ratio)
 {

   out->params.gtm_reserve.GTM_maxval_th =
     LINEAR_INTERPOLATION_INT(in1->params.gtm_reserve.GTM_maxval_th,
                              in2->params.gtm_reserve.GTM_maxval_th, ratio);
   out->params.gtm_reserve.GTM_key_min_th =
     LINEAR_INTERPOLATION_INT(in1->params.gtm_reserve.GTM_key_min_th,
                              in2->params.gtm_reserve.GTM_key_min_th, ratio);
   out->params.gtm_reserve.GTM_key_max_th =
     LINEAR_INTERPOLATION_INT(in1->params.gtm_reserve.GTM_key_max_th,
                              in2->params.gtm_reserve.GTM_key_max_th, ratio);
   out->params.gtm_reserve.GTM_key_hist_bin_weight =
     LINEAR_INTERPOLATION(in1->params.gtm_reserve.GTM_key_hist_bin_weight,
                          in2->params.gtm_reserve.GTM_key_hist_bin_weight, ratio);
   out->params.gtm_reserve.GTM_Yout_maxval =
     LINEAR_INTERPOLATION_INT(in1->params.gtm_reserve.GTM_Yout_maxval,
                              in2->params.gtm_reserve.GTM_Yout_maxval, ratio);
   out->params.gtm_core.GTM_a_middletone =
     LINEAR_INTERPOLATION(in1->params.gtm_core.GTM_a_middletone,
                          in2->params.gtm_core.GTM_a_middletone, ratio);
   out->params.gtm_core.GTM_temporal_w =
     LINEAR_INTERPOLATION(in1->params.gtm_core.GTM_temporal_w,
                          in2->params.gtm_core.GTM_temporal_w, ratio);
   out->params.gtm_core.GTM_middletone_w =
     LINEAR_INTERPOLATION(in1->params.gtm_core.GTM_middletone_w,
                          in2->params.gtm_core.GTM_middletone_w, ratio);
   out->params.gtm_core.GTM_max_percentile=
     LINEAR_INTERPOLATION(in1->params.gtm_core.GTM_max_percentile,
                          in2->params.gtm_core.GTM_max_percentile, ratio);
   out->params.gtm_core.GTM_min_percentile =
     LINEAR_INTERPOLATION(in1->params.gtm_core.GTM_min_percentile,
                          in2->params.gtm_core.GTM_min_percentile, ratio);
   out->params.gtm_reserve.GTM_minval_th =
     LINEAR_INTERPOLATION(in1->params.gtm_reserve.GTM_minval_th,
                          in2->params.gtm_reserve.GTM_minval_th, ratio);
 }


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
   chromatix_GTM_v2_hdr_exp_type *gtm_exp_v2,
   isp_gtm_algo_params_t *algo_params) {

   boolean                  ret               = TRUE;
   trigger_point_linear_type *gtm_trigger     = NULL;
   trigger_point2_type     *gtm_trigger2      = NULL;
   float                    start             = 0.0;
   float                    end               = 0.0;
   float                    ratio             = 0.0;
   float                    aec_reference     = 1.0;
   uint32_t                 i                 = 0;
   uint8_t                  trigger_index     = MAX_SETS_FOR_TONE_NOISE_ADJ + 1;
   uint8_t                  control_type      = CONTROL_GAIN;
   boolean                  use_gtm_v2        = TRUE;
   isp_gtm_algo_params_t    local_params_1 , local_params_2;

   if (!gtm_exp_v2) {
     ISP_ERR("Incorrect Params: Both GTM v1 and v2 are NULL");
     return FALSE;
   }

   RETURN_IF_NULL(algo_params);

   if (gtm_exp_v2) {
     use_gtm_v2 = TRUE;
     /* For v2 version the input corresponds to 6 regions that
        need to triggered based on exp_ratio */
     control_type = CONTROL_AEC_EXP_SENSITIVITY_RATIO;
   }

   ISP_DBG(" use_gtm_v2 %d ", use_gtm_v2);

   for (i = 0; i < MAX_SETS_FOR_TONE_NOISE_ADJ; i++) {

     if (i == MAX_SETS_FOR_TONE_NOISE_ADJ - 1) {
       /* falls within region 6 but we do not use trigger points in the region */
       ratio         = 0.0;
       trigger_index = MAX_SETS_FOR_TONE_NOISE_ADJ - 1;
       break;
     }

     switch (control_type) {
     case CONTROL_AEC_EXP_SENSITIVITY_RATIO:
       /* AEC sensitivity ratio based*/
       aec_reference = gtm->aec_update.hdr_sensitivity_ratio;
       if (use_gtm_v2)
         gtm_trigger2 = &(gtm_exp_v2[i].aec_sensitivity_ratio_v2);
       start         = gtm_trigger2->start;
       end           = gtm_trigger2->end;
       ISP_DBG("exposure base, exp_ratio %f",aec_reference);
       break;
     default:
       ISP_DBG("Incorrect control type%d", control_type);
       ret = FALSE;
       goto error;
     }

     /* index is within interpolation range, find ratio */
     if (aec_reference >= start && aec_reference < end) {
       ratio = (end - aec_reference)/(end - start);
       trigger_index = i;
       ISP_DBG("%s [%f - %f - %f] = %f", __func__, start, aec_reference, end,
         ratio);
       break;
     } else if (aec_reference < start) {
         ratio         = 0.0;
         trigger_index = i;
         ISP_DBG("%s [%f - %f - %f] = %f", __func__, start, aec_reference, end,
           ratio);
         break;
      }
   }

   if (trigger_index + 1 >= MAX_SETS_FOR_TONE_NOISE_ADJ) {
     ISP_ERR("invalid trigger_index, no interpolation");
     ret = FALSE;
     goto error;
   }

   if (use_gtm_v2) {
     gtm46_copy_core_reserve_params(&local_params_1.params.gtm_core,
       &local_params_1.params.gtm_reserve,
       &gtm_exp_v2[trigger_index].gtm_core_v2,
       &gtm_exp_v2[trigger_index].gtm_reserve_v2);
     gtm46_copy_core_reserve_params(&local_params_2.params.gtm_core,
       &local_params_2.params.gtm_reserve,
       &gtm_exp_v2[trigger_index + 1].gtm_core_v2,
       &gtm_exp_v2[trigger_index + 1].gtm_reserve_v2);
   }
    if (ratio != 0.0){
       ISP_DBG("Ratio is NOT 0");
       gtm46_interpolate_hdr_v2(&local_params_1,
                         &local_params_2,
                         algo_params,
                         ratio);
    }
    else if (ratio == 0.0) { // When interpolation it is not needed.
      ISP_DBG("Ratio is 0");
      gtm46_interpolate_hdr_v2(&local_params_1,
                       &local_params_1,
                       algo_params,
                       ratio);
      }


 error:
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
 static boolean gtm46_v2_compute_algo_params(gtm46_t *gtm,
   isp_gtm_algo_params_t *algo_param, chromatix_GTM_v2_hdr_type *chromatix_gtm_v2) {
   boolean              ret = TRUE;
   trigger_ratio_t      trigger_ratio;
   isp_gtm_algo_params_t  local_params_1, local_params_2;


   RETURN_IF_NULL(algo_param);
   memset(&local_params_1, 0, sizeof(isp_gtm_algo_params_t));
   memset(&local_params_2, 0, sizeof(isp_gtm_algo_params_t));


   if (!gtm->is_aec_update_valid) {
     /* default to NORMAL_LIGHT when aec_update isn't available yet */

     gtm46_copy_core_reserve_params(&algo_param->params.gtm_core,
       &algo_param->params.gtm_reserve,
       &chromatix_gtm_v2->gtm_v2[NORMAL_LIGHT].gtm_exp_data[2].gtm_core_v2,
       &chromatix_gtm_v2->gtm_v2[NORMAL_LIGHT].gtm_exp_data[2].gtm_reserve_v2);
   } else {
     /* interpolate to get the chromatix parameters for algo */
     ret = isp_sub_module_util_get_aec_ratio_bright_low(
       chromatix_gtm_v2->control_gtm_v2,
       (void *)&chromatix_gtm_v2->gtm_v2[GTM_BRIGHT_LIGHT].gtm_v2_trigger,
       (void *)&chromatix_gtm_v2->gtm_v2[GTM_LOW_LIGHT].gtm_v2_trigger,
       &gtm->aec_update, &trigger_ratio);
     if (ret != TRUE) {
       ISP_ERR("get aec ratio error");
       return TRUE;
     }
     /* The GTM V2 version has 3x6 regions i.e. has
        [MAX_SETS_FOR_GTM_EXP_ADJ][MAX_SETS_FOR_TONE_NOISE_ADJ]
        The inner/2nd dimension is triggered based on exposure ratio/aec sensitivity ratio
        (aec_sensitivity_ratio_v2) and has 6 regions.
        The outer is based on gain or lux index (gtm_v2_trigger) and has 3 regions.
     */
     ISP_DBG("lighting %d ", trigger_ratio.lighting);
     switch (trigger_ratio.lighting) {
     case TRIGGER_NORMAL:
       ret = gtm46_v2_process_regions(gtm,
         chromatix_gtm_v2->gtm_v2[GTM_NORMAL_LIGHT].gtm_exp_data,
       algo_param);
       break;
     case TRIGGER_LOWLIGHT:
       /* Compute the params based on exp_ratio for each lighting*/
       ret = gtm46_v2_process_regions(gtm,
         chromatix_gtm_v2->gtm_v2[GTM_NORMAL_LIGHT].gtm_exp_data,
       &local_params_1);
       ret |= gtm46_v2_process_regions(gtm,
         chromatix_gtm_v2->gtm_v2[GTM_LOW_LIGHT].gtm_exp_data,
       &local_params_2);
       /* Interpolate the algo params obtained above based on
          gain/lux index */
       gtm46_interpolate_hdr_v2(&local_params_1,
                         &local_params_2,
                         algo_param,
                         trigger_ratio.ratio);
       break;
     case TRIGGER_OUTDOOR:
       /* Compute the params based on exp_ratio for each lighting*/
       ret = gtm46_v2_process_regions(gtm,
         chromatix_gtm_v2->gtm_v2[GTM_NORMAL_LIGHT].gtm_exp_data,
       &local_params_1);
       ret |= gtm46_v2_process_regions(gtm,
         chromatix_gtm_v2->gtm_v2[GTM_BRIGHT_LIGHT].gtm_exp_data,
       &local_params_2);
       /* Interpolate the algo params obtained above based on
          gain/lux index */
       gtm46_interpolate_hdr_v2(&local_params_1,
                         &local_params_2,
                         algo_param,
                         trigger_ratio.ratio);
       break;
     }

   }
   return ret;
 }


/** gtm_trigger_update:
 *
 *  @module: mct module handle
 *  @isp_sub_module: ISP sub module handle
 *  @event: mct event handle
 *
 *  Perform trigger update using aec_update
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean gtm46_trigger_hdr_params_ext(void *data1,
  void *data2, void *data3)
{
  boolean                  ret               = TRUE;
  gtm46_t                 *gtm               = NULL;
  isp_private_event_t     *private_event     = NULL;
  isp_sub_module_output_t *output            = NULL;
  chromatix_hdr_ext_type  *chromatix_ptr     = NULL;
  chromatix_GTM_v2_hdr_type *chromatix_gtm_v2 = NULL;
  isp_gtm_algo_params_t   *algo_param        = NULL;
  isp_hw_read_info_t      *gtm_dmi_info      = NULL;
  trigger_point_linear_type *gtm_trigger     = NULL;
  trigger_point2_type     *gtm_trigger2      = NULL;
  float                    start             = 0.0;
  float                    end               = 0.0;
  float                    ratio             = 0.0;
  uint32_t                 i                 = 0;
  float                    aec_reference     = 0.0;
  uint8_t                  trigger_index     = MAX_SETS_FOR_TONE_NOISE_ADJ + 1;
  mct_module_t *module = NULL;
  isp_sub_module_t *isp_sub_module = NULL;
  mct_event_t *event = NULL;

  module = (mct_module_t *)data1;
  isp_sub_module = (isp_sub_module_t *)data2;
  event = (mct_event_t *) data3;

  RETURN_IF_NULL(module);
  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(event);

  gtm = (gtm46_t *)isp_sub_module->private_data;
  RETURN_IF_NULL(gtm);

  private_event =
    (isp_private_event_t *)event->u.module_event.module_event_data;
  RETURN_IF_NULL(private_event);

  output = (isp_sub_module_output_t *)private_event->data;
  RETURN_IF_NULL(output);

  algo_param = &(output->algo_params->gtm);

  chromatix_ptr =
    gtm->zzhdr_chromatix_extn;
  RETURN_IF_NULL(chromatix_ptr);

  chromatix_gtm_v2 = &chromatix_ptr->chromatix_post_processing.chromatix_gtm_v2;

  if (chromatix_gtm_v2->gtm_v2_enable) {
    ret = gtm46_v2_compute_algo_params(gtm, algo_param, chromatix_gtm_v2);
  }
  return TRUE;
} /* gtm_trigger_update */



static ext_override_gtm_hdr_func gtm_override_func_hdr_ext = {
  .gtm_trigger_params = gtm46_trigger_hdr_params_ext,
};

boolean gtm46_fill_func_table_hdr_ext(gtm46_t *gtm)
{
  gtm->ext_func_hdr_table = &gtm_override_func_hdr_ext;
  return TRUE;
} /* gtm46_fill_func_table */

#endif
