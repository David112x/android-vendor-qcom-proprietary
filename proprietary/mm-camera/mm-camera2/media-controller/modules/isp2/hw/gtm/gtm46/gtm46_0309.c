/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "gtm46.h"
#include "gtm_curve.h"

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
  chromatix_VFE_common_type *chromatix_common = NULL;

  chromatix_common =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;
  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;

  isp_sub_module->submod_enable = FALSE;
  isp_sub_module->chromatix_module_enable = FALSE;

  if (chromatix_ptr->chromatix_VFE.chromatix_gtm.gtm_enable > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
  }

  return TRUE;
}

/** gtm46_interpolate:
 *
 *  Return TRUE on success and FALSE on failure
 **/
void gtm46_interpolate_type1(chromatix_GTM *in1,
  chromatix_GTM *in2, chromatix_GTM *out, float ratio)
{
  out->gtm_reserve.GTM_maxval_th =
    LINEAR_INTERPOLATION_INT(in1->gtm_reserve.GTM_maxval_th,
                             in2->gtm_reserve.GTM_maxval_th, ratio);
  out->gtm_reserve.GTM_key_min_th =
    LINEAR_INTERPOLATION_INT(in1->gtm_reserve.GTM_key_min_th,
                             in2->gtm_reserve.GTM_key_min_th, ratio);
  out->gtm_reserve.GTM_key_max_th =
    LINEAR_INTERPOLATION_INT(in1->gtm_reserve.GTM_key_max_th,
                             in2->gtm_reserve.GTM_key_max_th, ratio);
  out->gtm_reserve.GTM_key_hist_bin_weight =
    LINEAR_INTERPOLATION(in1->gtm_reserve.GTM_key_hist_bin_weight,
                         in2->gtm_reserve.GTM_key_hist_bin_weight, ratio);
  out->gtm_reserve.GTM_Yout_maxval =
    LINEAR_INTERPOLATION_INT(in1->gtm_reserve.GTM_Yout_maxval,
                             in2->gtm_reserve.GTM_Yout_maxval, ratio);
  out->gtm_core.GTM_a_middletone =
    LINEAR_INTERPOLATION(in1->gtm_core.GTM_a_middletone,
                         in2->gtm_core.GTM_a_middletone, ratio);
  out->gtm_core.GTM_temporal_w =
    LINEAR_INTERPOLATION(in1->gtm_core.GTM_temporal_w,
                         in2->gtm_core.GTM_temporal_w, ratio);
  out->gtm_core.GTM_middletone_w =
    LINEAR_INTERPOLATION(in1->gtm_core.GTM_middletone_w,
                         in2->gtm_core.GTM_middletone_w, ratio);
  out->gtm_core.GTM_max_percentile=
    LINEAR_INTERPOLATION(in1->gtm_core.GTM_max_percentile,
                         in2->gtm_core.GTM_max_percentile, ratio);
  out->gtm_core.GTM_min_percentile =
    LINEAR_INTERPOLATION(in1->gtm_core.GTM_min_percentile,
                         in2->gtm_core.GTM_min_percentile, ratio);
  out->gtm_reserve.GTM_minval_th =
    LINEAR_INTERPOLATION(in1->gtm_reserve.GTM_minval_th,
                         in2->gtm_reserve.GTM_minval_th, ratio);
}


/** gtm46_process_regions:
 *
 *  @chromatix_ptr: chromatix pointer
 *  @local_algo_params: the output algo params that are
 *           computed/interpolated
 *  Use the control type (gain, lux index, exp ratio) to compute
 *  or interpolate the algo parameters.
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean gtm46_process_regions(
  isp_sub_module_t *isp_sub_module,
  gtm46_t *gtm,
  chromatix_parms_type *chromatix_ptr,
  gtm46_algo_params_t  *local_algo_params) {

  RETURN_IF_NULL(isp_sub_module);
  RETURN_IF_NULL(gtm);
  RETURN_IF_NULL(chromatix_ptr);
  RETURN_IF_NULL(local_algo_params);

  return TRUE;
}

/** gtm46_compute_algo_params:
 *
 *  Call the function to compute the algo parameters.
 *  The v1 version has 6 regions that can be used to
 *  get/interpolate the parameters.
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean gtm46_compute_algo_params(
  isp_sub_module_t *isp_sub_module,
  gtm46_t *gtm,
  isp_gtm_algo_params_t *algo_param,
  chromatix_parms_type  *chromatix_ptr)
{
  boolean              ret               = TRUE;
  gtm46_algo_params_t *local_algo_params = NULL;
  chromatix_GTM_type  *chromatix_gtm     = NULL;

  RETURN_IF_NULL(algo_param);
  RETURN_IF_NULL(chromatix_gtm);

  return ret;
}


boolean gtm46_v2_process_adj_regions(void *data __unused,
  void *gtm46 __unused, uint32_t *trigger_idx_start __unused,
  uint32_t *trigger_idx_end __unused, float *ratio __unused) {

  return TRUE;
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
    boolean                  default_curve_valid = TRUE;
    boolean                  ret = TRUE;
    chromatix_GTM_type      *chromatix_gtm     = NULL;

    chromatix_gtm = &chromatix_ptr->chromatix_VFE.chromatix_gtm;

    if (gtm->enable_adrc) {
      gtm46_derive_and_pack_lut_adrc_based(gtm);
    } else {
      /* prepare algo parameters */
      if (gtm->use_gtm_fix_curve == 0 ){
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
        /*enable temporal filter once confirm with system team*/
        algo_param->temporal_filter_enable   = TRUE;

        if (!gtm->is_aec_update_valid) {
          /* default to NORMAL_LIGHT when aec_update isn't available yet */
          algo_param->params = chromatix_gtm->gtm[2];
        } else {
          if( (isp_sub_module->hdr_mode == CAM_SENSOR_HDR_IN_SENSOR) && gtm->ext_func_hdr_table &&
            gtm->ext_func_hdr_table->gtm_trigger_params){
            ret = gtm->ext_func_hdr_table->gtm_trigger_params(module, isp_sub_module, event);
            if (ret == FALSE){
              ISP_ERR("failed: gtm_trigger_params ret %d", ret);
              return FALSE;
            }
          } else {
             /* interpolate to get the chromatix parameters for algo */

            for (i = 0; i < MAX_SETS_FOR_TONE_NOISE_ADJ; i++) {

             if (i == MAX_SETS_FOR_TONE_NOISE_ADJ - 1) {
                /* falls within region 6 but we do not use trigger points in the region */
                ratio         = 0.0;
                trigger_index = MAX_SETS_FOR_TONE_NOISE_ADJ - 1;
                break;
              }

              if (chromatix_gtm->control_gtm == CONTROL_LUX_IDX) {
                /* lux index based */
                aec_reference = gtm->aec_update.lux_idx;
                gtm_trigger   = &(chromatix_gtm->gtm[i].gtm_trigger);
                start         = gtm_trigger->lux_index_start;
                end           = gtm_trigger->lux_index_end;
                ISP_DBG("lux base, lux idx %f",aec_reference);
              } else if (chromatix_gtm->control_gtm == CONTROL_GAIN) {
                /* Gain based */
                aec_reference = gtm->aec_update.real_gain;
                gtm_trigger   = &(chromatix_gtm->gtm[i].gtm_trigger);
                start         = gtm_trigger->gain_start;
                end           = gtm_trigger->gain_end;
                ISP_DBG("gain base, gain %f",aec_reference);
              } else if (chromatix_gtm->control_gtm == CONTROL_AEC_EXP_SENSITIVITY_RATIO) {
                /* AEC sensitivity ratio based*/
                aec_reference = gtm->aec_update.real_gain; // Need to use aec_sensitivity_ratio here from 3A
                gtm_trigger2  = &(chromatix_gtm->gtm[i].aec_sensitivity_ratio);
                start         = gtm_trigger2->start;
                end           = gtm_trigger2->end;
                ISP_DBG("gain base, gain %f",aec_reference);
              }

              /* index is within interpolation range, find ratio */
               if (aec_reference >= start && aec_reference < end) {
                 ratio         = (end - aec_reference)/(end - start);
                 trigger_index = i;
                 ISP_DBG("%s [%f - %f - %f] = %f", __func__, start, aec_reference, end,
                   ratio);
                 break;
               } else if (aec_reference < start) {
                 ratio = 0.0;
                 trigger_index = i;
                 ISP_DBG("%s [%f - %f - %f] = %f", __func__, start, aec_reference, end,
                   ratio);
               }
            }

            if (trigger_index + 1 >= MAX_SETS_FOR_TONE_NOISE_ADJ) {
               ISP_ERR("invalid trigger_index, no interpolation");
            } else {
                if (ratio != 0.0){
                  gtm46_interpolate_type1(&chromatix_gtm->gtm[trigger_index],
                                 &chromatix_gtm->gtm[trigger_index + 1],
                                 &algo_param->params,
                                 ratio);
                } else if (ratio == 0.0){
                  gtm46_interpolate_type1(&chromatix_gtm->gtm[trigger_index],
                                 &chromatix_gtm->gtm[trigger_index],
                                 &algo_param->params,
                                 ratio);
                }
            }
          }
        }
        if (!gtm->algo_update_valid){
         /* Calculate the default curve */
          if (!gtm46_calculate_default_cuve(gtm,algo_param)){
            ISP_ERR("failed in calculating default curve \n");
            default_curve_valid = FALSE;
          }
        }

        if (!gtm->algo_update_valid){
         /* Calculate the default curve */
          if (!gtm46_calculate_default_cuve(gtm,algo_param)){
            ISP_ERR("failed in calculating default curve \n");
            default_curve_valid = FALSE;
          }
        }

        /* pack DMI table */
        gtm46_pack_dmi(gtm->algo_output.gtm_yout, gtm_xin_tbl,
          gtm->dmi_tbl, gtm->base, gtm->slope);
      } else {
        /* pack DMI table */
        gtm46_pack_dmi_fix_curve(gtm->dmi_tbl,
          yratio_base, yratio_slope);
      }
        }
  return ret;
}
