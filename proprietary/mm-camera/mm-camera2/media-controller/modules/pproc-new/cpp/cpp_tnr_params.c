/*============================================================================

Copyright (c) 2014-2017 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/
#include "eztune_diagnostics.h"
#include "cpp_log.h"
#include "chromatix_cpp.h"
#include "chromatix_cpp_stripped.h"
#include "cpp_hw_params.h"
#include "cpp_module.h"
#include <media/msmb_camera.h>
#include <media/msmb_pproc.h>
#include <math.h>

#define Round(x) (int)(x + sign(x)*0.5)


/** cpp_tnr_params_init_tnr_params:
 *
 *  @cpp_hw_params - hardware parameters for current stream
 *
 *  This function sets defaults values to the TNR parameters. They
 *  are formated so they can be used directly to create CPP firmware
 *  frame message.
 *
 *  Return: It returns 0 at success.
 **/
static void cpp_tnr_params_init_tnr_params(cpp_hardware_params_t *cpp_hw_params)
{
  uint32_t j, k;
  uint32_t profile = 8;

  if (!cpp_hw_params) {
    CPP_TNR_ERR("invalid hw_params, failed");
    return;
  }

  /* Default threshold (denoise scale) 12q4 format*/
  cpp_hw_params->tnr_info_Y.noise_threshold[0] = 12.00;
  cpp_hw_params->tnr_info_Y.noise_threshold[1] = 6.00;
  cpp_hw_params->tnr_info_Y.noise_threshold[2] = 3.00;
  cpp_hw_params->tnr_info_Y.noise_threshold[3] = 4.00;
  cpp_hw_params->tnr_info_Y.noise_threshold[4] = 3.00;

  cpp_hw_params->tnr_info_Cb.noise_threshold[0] = 20.00;
  cpp_hw_params->tnr_info_Cb.noise_threshold[1] = 10.00;
  cpp_hw_params->tnr_info_Cb.noise_threshold[2] = 5.00;
  cpp_hw_params->tnr_info_Cb.noise_threshold[3] = 3.00;
  cpp_hw_params->tnr_info_Cb.noise_threshold[4] = 2.00;

  for (j = 0; j < 5; j++)
    cpp_hw_params->tnr_info_Cr.noise_threshold[j] =
        cpp_hw_params->tnr_info_Cb.noise_threshold[j];

  /* Default scale (edge softness) 18q8 format*/
  cpp_hw_params->tnr_info_Y.bilateral_scale[0] = 0.00;
  cpp_hw_params->tnr_info_Y.bilateral_scale[1] = 3.00;
  cpp_hw_params->tnr_info_Y.bilateral_scale[2] = 5.00;
  cpp_hw_params->tnr_info_Y.bilateral_scale[3] = 9.00;

  cpp_hw_params->tnr_info_Cb.bilateral_scale[0] = 0.00;
  cpp_hw_params->tnr_info_Cb.bilateral_scale[1] = 1.00;
  cpp_hw_params->tnr_info_Cb.bilateral_scale[2] = 3.00;
  cpp_hw_params->tnr_info_Cb.bilateral_scale[3] = 5.00;

  for (j = 0; j < 4; j++)
    cpp_hw_params->tnr_info_Cr.bilateral_scale[j] =
        cpp_hw_params->tnr_info_Cb.bilateral_scale[j];


  /* Default weight 5q4 format*/
  for (j = 0; j < 5; j++) {
    cpp_hw_params->tnr_info_Y.weight[j] = 0;
    cpp_hw_params->tnr_info_Cb.weight[j] = 0;
    cpp_hw_params->tnr_info_Cr.weight[j] = 0;
  }
  return;
}

/** cpp_tnr_params_get_upscale_ratio:
 *
 *  @wavelet_chromatix_data - Pointer to the wavelet chromatix data. It holds
 *     scale adjustment's regions and the other TNR parameters.
 *  @scale_ratio - Scale ration calculated between ISP input dimension and CPP
 *     output dimension.
 *  @reg_strt_idx_start - Scale start index .
 *  @reg_strt_idx_end - Scale end index
 *  @reg_strt_interp_ratio - Scale interpolation ratio.
 *
 *  This function finds a downscale region in the chromatix header according to
 *  the scale ratio and calculates the scale interpolation ratio.
 *
 **/
static void cpp_tnr_params_get_downscale_ratio(
  void *downscale_data, float scale_ratio,
  uint32_t *reg_strt_idx_start, uint32_t *reg_strt_idx_end,
  float *reg_strt_interp_ratio)
{
  uint32_t i ;
  uint32_t scale_idx_start ;
  uint32_t scale_idx_end ;
  float scale_strt;
  float scale_end;

  *reg_strt_interp_ratio = 0.0f ;

  for ( i = 0 ; i < DOWN_SCALING_LEVELS ; i ++ ) {
    scale_idx_start = i ;
    scale_idx_end = i+1 ;

    if ( i == ( DOWN_SCALING_LEVELS - 1 )) {
      scale_idx_start = DOWN_SCALING_LEVELS-1 ;
      scale_idx_end = DOWN_SCALING_LEVELS-1 ;
      break ;
    }
    scale_strt = GET_TNR_DOWNSCALE_LEVELS(downscale_data, scale_idx_start);
    scale_end  = GET_TNR_DOWNSCALE_LEVELS(downscale_data, scale_idx_end);
    if ( scale_strt >= scale_end ) {
      CPP_TNR_DBG("TNR Down Scaling Ratio Settings are not in correct order");
      *reg_strt_idx_start = i;
      *reg_strt_idx_end = i;
      return;
    }

    if ( (scale_ratio >= scale_strt) && (scale_ratio < scale_end)) {
      *reg_strt_interp_ratio = (float)(scale_ratio - scale_strt)/
        (float)(scale_end -scale_strt);
      break ;
    } else if ((scale_ratio < scale_strt)) {
      scale_idx_end = i ;
      break ;
    }
  }
  *reg_strt_idx_start = scale_idx_start ;
  *reg_strt_idx_end = scale_idx_end ;
}

/** cpp_tnr_params_get_upscale_ratio:
 *
 *  @wavelet_chromatix_data - Pointer to the wavelet chromatix data. It holds
 *     scale adjustment's regions and the other TNR parameters.
 *  @scale_ratio - Scale ration calculated between ISP input dimension and CPP
 *     output dimension.
 *  @reg_strt_idx_start - Scale start index .
 *  @reg_strt_idx_end - Scale end index
 *  @reg_strt_interp_ratio - Scale interpolation ratio.
 *
 *  This function finds a upscale region in the chromatix header according to
 *  the scale ratio and calculates the scale interpolation ratio.
 *
 **/
static void cpp_tnr_params_get_upscale_ratio(
  void *upscale_data, float scale_ratio,
  uint32_t *reg_strt_idx_start, uint32_t *reg_strt_idx_end,
  float *reg_strt_interp_ratio)
{
  uint32_t i ;
  uint32_t scale_idx_start ;
  uint32_t scale_idx_end ;
  float scale_strt;
  float scale_end;

  *reg_strt_interp_ratio = 0.0f ;
  for ( i = 0 ; i < UP_SCALING_LEVELS ; i ++ ) {
    scale_idx_start = i ;
    scale_idx_end = i+1 ;

    if ( i == ( UP_SCALING_LEVELS - 1 )) {
      scale_idx_start = UP_SCALING_LEVELS-1 ;
      scale_idx_end = UP_SCALING_LEVELS-1 ;
      break ;
    }
    scale_strt = GET_TNR_UPSCALE_LEVELS(upscale_data, scale_idx_start);
    scale_end  = GET_TNR_UPSCALE_LEVELS(upscale_data, scale_idx_end);

    if ( scale_strt <= scale_end ) {
      CPP_TNR_DBG("TNR Up Scaling Ratio Settings are not in correct order");
      *reg_strt_idx_start = i;
      *reg_strt_idx_end = i;
      return;
    }

    if ( (scale_ratio < scale_strt) && (scale_ratio >scale_end)) {
      *reg_strt_interp_ratio = (float)(scale_ratio - scale_strt)/
        (float)(scale_end -scale_strt);
      break ;
    } else if (scale_ratio >= scale_strt) {
      scale_idx_end = i ;
      break ;
    }
  }
  *reg_strt_idx_start = scale_idx_start ;
  *reg_strt_idx_end = scale_idx_end ;
}

void cpp_tnr_params_scale_interpolate(cpp_tnr_params_scale_info_t *scale_info,
  chromatix_cpp_type *cpp_chromatix_ptr, float scale_ratio)
{
  uint32_t reg_strt_idx_start;
  uint32_t reg_strt_idx_end;
  float reg_strt_interp_ratio = 0.0f;
  uint32_t i;
  void *upscale_data;
  void *downscale_data;

  GET_TNR_UPSCALE_DATA(cpp_chromatix_ptr, NULL, upscale_data);
  GET_TNR_DOWNSCALE_DATA(cpp_chromatix_ptr, NULL, downscale_data);
  /* Interpolate scale adjustment */
   if ( scale_ratio > 1.0f ) {
     cpp_tnr_params_get_downscale_ratio(downscale_data, scale_ratio,
       &reg_strt_idx_start, &reg_strt_idx_end, &reg_strt_interp_ratio);
     for (i = 0; i < TNR_SCALE_LAYERS; i++) {
       if (i < GET_TNR_SCALE_COUNT()) {
         scale_info->denoise_profile_Y_adj[i] =
           LINEAR_INTERPOLATE(reg_strt_interp_ratio,
             TNR_DOWN_PROFILE_Y_ADJ(downscale_data, reg_strt_idx_start, i),
             TNR_DOWN_PROFILE_Y_ADJ(downscale_data, reg_strt_idx_end, i));
         scale_info->denoise_profile_chroma_adj[i] =
           LINEAR_INTERPOLATE(reg_strt_interp_ratio,
             TNR_DOWN_PROFILE_CBCR_ADJ(downscale_data, reg_strt_idx_start, i),
             TNR_DOWN_PROFILE_CBCR_ADJ(downscale_data, reg_strt_idx_end, i));

         /* interpolate denoise_scale_y */
         scale_info->denoise_weight_y_VFE_adj[i] =
           LINEAR_INTERPOLATE(reg_strt_interp_ratio,
             TNR_DOWN_WEIGHT_Y_ADJ(downscale_data, reg_strt_idx_start, i),
             TNR_DOWN_WEIGHT_Y_ADJ(downscale_data, reg_strt_idx_end, i));
         scale_info->denoise_weight_chroma_VFE_adj[i] =
           LINEAR_INTERPOLATE(reg_strt_interp_ratio,
             TNR_DOWN_WEIGHT_CBCR_ADJ(downscale_data, reg_strt_idx_start, i),
             TNR_DOWN_WEIGHT_CBCR_ADJ(downscale_data, reg_strt_idx_end, i));
       } else {
         scale_info->denoise_profile_Y_adj[i] =
           scale_info->denoise_profile_Y_adj[i - 1];
         scale_info->denoise_profile_chroma_adj[i] =
           scale_info->denoise_profile_chroma_adj[i - 1];
         scale_info->denoise_weight_y_VFE_adj[i] =
           scale_info->denoise_weight_y_VFE_adj[i - 1];
         scale_info->denoise_weight_chroma_VFE_adj[i] =
           scale_info->denoise_weight_chroma_VFE_adj[i - 1];
       }
     }
   } else if ( scale_ratio < 1.0f ) {
     cpp_tnr_params_get_upscale_ratio(upscale_data, scale_ratio,
       &reg_strt_idx_start, &reg_strt_idx_end, &reg_strt_interp_ratio);

     for (i = 0; i < TNR_SCALE_LAYERS; i++) {
        if (i < GET_TNR_SCALE_COUNT()) {
         scale_info-> denoise_profile_Y_adj[i] =
           LINEAR_INTERPOLATE(reg_strt_interp_ratio,
             TNR_UP_PROFILE_Y_ADJ(upscale_data, reg_strt_idx_start, i),
             TNR_UP_PROFILE_Y_ADJ(upscale_data, reg_strt_idx_end, i));
         scale_info->denoise_profile_chroma_adj[i] =
           LINEAR_INTERPOLATE(reg_strt_interp_ratio,
             TNR_UP_PROFILE_CBCR_ADJ(upscale_data, reg_strt_idx_start, i),
             TNR_UP_PROFILE_CBCR_ADJ(upscale_data, reg_strt_idx_end, i));

         /* interpolate denoise_scale_y */
         scale_info->denoise_weight_y_VFE_adj[i] =
           LINEAR_INTERPOLATE(reg_strt_interp_ratio,
             TNR_UP_WEIGHT_Y_ADJ(upscale_data, reg_strt_idx_start, i),
             TNR_UP_WEIGHT_Y_ADJ(upscale_data, reg_strt_idx_end, i));
         scale_info->denoise_weight_chroma_VFE_adj[i] =
           LINEAR_INTERPOLATE(reg_strt_interp_ratio,
             TNR_UP_WEIGHT_CBCR_ADJ(upscale_data, reg_strt_idx_start, i),
             TNR_UP_WEIGHT_CBCR_ADJ(upscale_data, reg_strt_idx_end, i));
       } else {
         scale_info-> denoise_profile_Y_adj[i] =
           scale_info-> denoise_profile_Y_adj[i - 1];
         scale_info->denoise_profile_chroma_adj[i] =
           scale_info->denoise_profile_chroma_adj[i - 1];
         scale_info->denoise_weight_y_VFE_adj[i] =
           scale_info->denoise_weight_y_VFE_adj[i - 1];
         scale_info->denoise_weight_chroma_VFE_adj[i] =
           scale_info->denoise_weight_chroma_VFE_adj[i - 1];
       }
     }

   } else {
     for (i = 0; i < TNR_SCALE_LAYERS; i++) {
       scale_info-> denoise_profile_Y_adj[i] = 1.0f;
       scale_info->denoise_profile_chroma_adj[i] = 1.0f;
       scale_info->denoise_weight_y_VFE_adj[i] = 1.0f;
       scale_info->denoise_weight_chroma_VFE_adj[i] = 1.0f;
     }
   }
}

/** cpp_tnr_params_interpolate:
 *
 *  @tnr_chrmtx_ptr - Pointer to the TNR structure in cpp chromatix file.
 *  @denoise_y_cfg - pointer to the structure with TNR parameters for Y plane
 *  @denoise_cb_cfg - pointer to the structure with TNR parameters for Cb plane
 *  @denoise_cr_cfg - pointer to the structure with TNR parameters for Cr plane
 *  @aec_trigger - pointer to the structure that holds the value for lux_idx and
 *     gsin for the current frame.
 *
 *
 *  This function interpolates data from chromatix file and calculates the
 *  values of the parameters that have to be loaded in CPP hardware. The result
 *  is saved in the structures denoise_y_cfg,denoise_cb_cfg,denoise_cr_cfg.
 *
 *  Return: It is void function.
 **/
void cpp_tnr_params_interpolate(chromatix_cpp_type *cpp_chromatix_ptr,
  cpp_tnr_info_t *denoise_y_cfg, cpp_tnr_info_t *denoise_cb_cfg,
  cpp_tnr_info_t *denoise_cr_cfg, cpp_params_aec_trigger_info_t *aec_trigger,
  float isp_scale_ratio, float cpp_scale_ratio)
{


  float noise_profileData[NUM_NOISE_PROFILE];
  float scaleY[WAVELET_LEVEL];
  float scaleChroma[WAVELET_LEVEL];
  float edge_softnessY[WAVELET_LEVEL];
  float edge_softnessChroma[WAVELET_LEVEL];
  float weightY[WAVELET_LEVEL];
  float weightChroma[WAVELET_LEVEL];

  float scaleY_5th;
  float scaleChroma_5th;
  float weightY_5th;
  float weightChroma_5th;

  float trigger_start[MAX_LIGHT_TYPES_FOR_SPATIAL];
  float trigger_end[MAX_LIGHT_TYPES_FOR_SPATIAL];
  float interp_ratio = 0.0f ;
  uint32_t i;
  uint32_t start_idx = 0;
  uint32_t end_idx = 0;

  float trigger;
  float lux_idx = aec_trigger->lux_idx;
  float total_gain = aec_trigger->gain;
  chromatixWNRDenoise_type *tnr_chrmtx_ptr;

  cpp_tnr_params_scale_info_t isp_scale_info;
  cpp_tnr_params_scale_info_t cpp_scale_info;

  tnr_chrmtx_ptr = &cpp_chromatix_ptr->chromatix_temporal_denoise;
  /*control_denoise: 0 use lux index trigger, 1 use gain trigger*/
  trigger = (tnr_chrmtx_ptr->control_WNR_denoise == 0) ? lux_idx :
    total_gain;

  for ( i = 0 ; i < MAX_LIGHT_TYPES_FOR_SPATIAL ; i ++ )
  {
    if (tnr_chrmtx_ptr->control_WNR_denoise == 0 ) //lux index
    {
      trigger_start[i] =
        tnr_chrmtx_ptr->referenceNoiseProfileData[i].
          WNR_trigger.lux_index_start;
      trigger_end[i] =
        tnr_chrmtx_ptr->referenceNoiseProfileData[i].
          WNR_trigger.lux_index_end;
    } else {
      trigger_start[i] =
        tnr_chrmtx_ptr->referenceNoiseProfileData[i].WNR_trigger.gain_start;
      trigger_end[i] =
        tnr_chrmtx_ptr->referenceNoiseProfileData[i].WNR_trigger.gain_end;
    }
  }

  for ( i = 0 ; i < MAX_LIGHT_TYPES_FOR_SPATIAL ; i ++ ) {
    if(trigger_start[i] > trigger_end[i]) {
      CPP_TNR_ERR("Strat trigger point is bigger than end trigger point");
      continue;
    }

    if ( i == ( MAX_LIGHT_TYPES_FOR_SPATIAL - 1 )) {
      start_idx = MAX_LIGHT_TYPES_FOR_SPATIAL-1 ;
      end_idx = MAX_LIGHT_TYPES_FOR_SPATIAL-1 ;
      break ;
    }
    if ( trigger <= trigger_start[i]) {
      start_idx = i ;
      end_idx = i ;
      interp_ratio = 0.0 ;
      break ;
    }
    else if ( trigger < trigger_end[i]) {
      start_idx = i ;
      end_idx = i+1 ;
      interp_ratio = (trigger - trigger_start[i]) /
        (trigger_end[i] -trigger_start[i]);
      break ;
    }
  }

  if (interp_ratio > 1.0) {
    CPP_TNR_ERR("Invalid interpolation ratio %f", interp_ratio);
    interp_ratio = 1.0;
  }

WNR_noise_profile_type *start_noise_profile_data =
  &tnr_chrmtx_ptr->referenceNoiseProfileData[start_idx];

WNR_noise_profile_type *end_noise_profile_data =
  &tnr_chrmtx_ptr->referenceNoiseProfileData[end_idx];

  for (i = 0; i < NUM_NOISE_PROFILE; i++)
    noise_profileData[i] = LINEAR_INTERPOLATE(interp_ratio,
      start_noise_profile_data->referenceNoiseProfileData[i],
      end_noise_profile_data->referenceNoiseProfileData[i]);

  for (i = 0; i < WAVELET_LEVEL; i++) {
    /* interpolate denoise_scale_y */
    scaleY[i] = LINEAR_INTERPOLATE(interp_ratio,
      start_noise_profile_data->denoise_scale_y[i],
      end_noise_profile_data->denoise_scale_y[i]);

    scaleChroma[i] = LINEAR_INTERPOLATE(interp_ratio,
      start_noise_profile_data->denoise_scale_chroma[i],
      end_noise_profile_data->denoise_scale_chroma[i]);

    edge_softnessY[i ] = LINEAR_INTERPOLATE(interp_ratio,
      start_noise_profile_data->denoise_edge_softness_y[i],
      end_noise_profile_data->denoise_edge_softness_y[i]);

    edge_softnessChroma[i] = LINEAR_INTERPOLATE(interp_ratio,
      start_noise_profile_data->denoise_edge_softness_chroma[i],
      end_noise_profile_data->denoise_edge_softness_chroma[i]);

    weightY[i] = LINEAR_INTERPOLATE(interp_ratio,
      start_noise_profile_data->denoise_weight_y[i],
      end_noise_profile_data->denoise_weight_y[i]);

    weightChroma[i]= LINEAR_INTERPOLATE(interp_ratio,
      start_noise_profile_data->denoise_weight_chroma[i],
      end_noise_profile_data->denoise_weight_chroma[i]);
  }

  scaleY_5th = LINEAR_INTERPOLATE(interp_ratio,
    start_noise_profile_data->denoise_scale_y_5th,
    end_noise_profile_data->denoise_scale_y_5th);

  weightY_5th = LINEAR_INTERPOLATE(interp_ratio,
    start_noise_profile_data->denoise_weight_y_5th,
    end_noise_profile_data->denoise_weight_y_5th);

  scaleChroma_5th = LINEAR_INTERPOLATE(interp_ratio,
    start_noise_profile_data->denoise_scale_chroma_5th,
    end_noise_profile_data->denoise_scale_chroma_5th);

  weightChroma_5th= LINEAR_INTERPOLATE(interp_ratio,
    start_noise_profile_data->denoise_weight_chroma_5th,
    end_noise_profile_data->denoise_weight_chroma_5th);

  cpp_tnr_params_scale_interpolate(&isp_scale_info,
    cpp_chromatix_ptr, isp_scale_ratio);

  cpp_tnr_params_scale_interpolate(&cpp_scale_info,
    cpp_chromatix_ptr, cpp_scale_ratio);

  for (i = 0; i < 4; i++) {
    weightY[i] = weightY[i] * isp_scale_info.denoise_weight_y_VFE_adj[i] *
      cpp_scale_info.denoise_weight_y_VFE_adj[i];
    weightChroma[i] = weightChroma[i] *
      isp_scale_info.denoise_weight_chroma_VFE_adj[i] *
      cpp_scale_info.denoise_weight_chroma_VFE_adj[i];
  }
  /* TODO */
  weightY_5th = weightY_5th *
    isp_scale_info.denoise_weight_y_VFE_adj[TNR_SCALE_LAYERS - 1] *
    cpp_scale_info.denoise_weight_y_VFE_adj[TNR_SCALE_LAYERS - 1];
  weightChroma_5th = weightChroma_5th *
    isp_scale_info.denoise_weight_chroma_VFE_adj[TNR_SCALE_LAYERS - 1] *
    cpp_scale_info.denoise_weight_chroma_VFE_adj[TNR_SCALE_LAYERS - 1];


    /* threshold (denoise scale) 12q4 format*/
  for (i = 0; i < 4; i++) {
    denoise_y_cfg->noise_threshold[i] =
      scaleY[i] * noise_profileData[i+4] *
      isp_scale_info.denoise_profile_Y_adj[i] *
      cpp_scale_info.denoise_profile_Y_adj[i];
    denoise_cb_cfg->noise_threshold[i] =
      scaleChroma[i] * noise_profileData[i+12] *
      isp_scale_info.denoise_profile_chroma_adj[i] *
      cpp_scale_info.denoise_profile_chroma_adj[i];
    denoise_cr_cfg->noise_threshold[i] =
      scaleChroma[i] * noise_profileData[i+20] *
      isp_scale_info.denoise_profile_chroma_adj[i] *
      cpp_scale_info.denoise_profile_chroma_adj[i];
  }

  denoise_y_cfg->noise_threshold[4] =
    scaleY_5th * noise_profileData[3+4] / 2 *
    isp_scale_info.denoise_profile_Y_adj[TNR_SCALE_LAYERS - 1] *
    cpp_scale_info.denoise_profile_Y_adj[TNR_SCALE_LAYERS - 1];
  denoise_cb_cfg->noise_threshold[4] =
    scaleChroma_5th * noise_profileData[3+12]/2 *
    isp_scale_info.denoise_profile_chroma_adj[TNR_SCALE_LAYERS - 1] *
    cpp_scale_info.denoise_profile_chroma_adj[TNR_SCALE_LAYERS - 1];
  denoise_cr_cfg->noise_threshold[4] =
    scaleChroma_5th * noise_profileData[3+20]/2 *
    isp_scale_info.denoise_profile_chroma_adj[TNR_SCALE_LAYERS - 1] *
    cpp_scale_info.denoise_profile_chroma_adj[TNR_SCALE_LAYERS - 1];


  /* scale (edge softness) 18q8 format*/
  for (i = 0; i < 4; i++) {
    denoise_y_cfg->bilateral_scale[i] = (64.0/3.0) * sqrt(2*3.14) /
      (edge_softnessY[i] * noise_profileData[i+4] *
      isp_scale_info.denoise_profile_Y_adj[i] *
      cpp_scale_info.denoise_profile_Y_adj[i]);

    denoise_cb_cfg->bilateral_scale[i] = (64.0/3.0) * sqrt(2*3.14) /
      (edge_softnessChroma[i] * noise_profileData[i+12] *
      isp_scale_info.denoise_profile_chroma_adj[i] *
      cpp_scale_info.denoise_profile_chroma_adj[i]);

    denoise_cr_cfg->bilateral_scale[i] = (64.0/3.0) * sqrt(2*3.14) /
      (edge_softnessChroma[i] * noise_profileData[i+20] *
      isp_scale_info.denoise_profile_chroma_adj[i] *
      cpp_scale_info.denoise_profile_chroma_adj[i]);
  }


  /* weight 5q4 format*/
  for (i = 0; i < 4; i++) {
    denoise_y_cfg->weight[i] = 1.0 - weightY[i];
    denoise_cb_cfg->weight[i] = 1.0 - weightChroma[i];
    denoise_cr_cfg->weight[i] = 1.0 - weightChroma[i];
  }

  denoise_y_cfg->weight[4] = 1.0 - weightY_5th;
  denoise_cb_cfg->weight[4] = 1.0 - weightChroma_5th;
  denoise_cr_cfg->weight[4] = 1.0 - weightChroma_5th;
}


void cpp_tnr_params_fill_stripped(cpp_hardware_params_t *cpp_hw_params,
  chromatix_cpp_stripped_type *chromatix)
{
  float noise_profileData[NUM_NOISE_PROFILE];
  float scaleY[WAVELET_LEVEL];
  float scaleChroma[WAVELET_LEVEL];
  float edge_softnessY[WAVELET_LEVEL];
  float edge_softnessChroma[WAVELET_LEVEL];
  float weightY[WAVELET_LEVEL];
  float weightChroma[WAVELET_LEVEL];
  float scaleY_5th;
  float scaleChroma_5th;
  float weightY_5th;
  float weightChroma_5th;

  uint32_t i;

  CPP_TNR_HIGH("using stripped tnr");

  cpp_tnr_info_t *denoise_y_cfg = &cpp_hw_params->tnr_info_Y;
  cpp_tnr_info_t *denoise_cb_cfg = &cpp_hw_params->tnr_info_Cb;
  cpp_tnr_info_t *denoise_cr_cfg = &cpp_hw_params->tnr_info_Cr;

  WNR_noise_profile_type *noise_profile =
    &chromatix->chromatix_temporal_denoise.referenceNoiseProfileData;

  for (i = 0; i < NUM_NOISE_PROFILE; i++)
    noise_profileData[i] = noise_profile->referenceNoiseProfileData[i];

  for (i = 0; i < WAVELET_LEVEL; i++) {
    /* interpolate denoise_scale_y */
    scaleY[i] = noise_profile->denoise_scale_y[i];
    scaleChroma[i] = noise_profile->denoise_scale_chroma[i];
    edge_softnessY[i] = noise_profile->denoise_edge_softness_y[i];
    edge_softnessChroma[i] = noise_profile->denoise_edge_softness_chroma[i];
    weightY[i] = noise_profile->denoise_weight_y[i];
    weightChroma[i] = noise_profile->denoise_weight_chroma[i];
  }

  scaleY_5th = noise_profile->denoise_scale_y_5th;
  weightY_5th = noise_profile->denoise_weight_y_5th;
  scaleChroma_5th = noise_profile->denoise_scale_chroma_5th;
  weightChroma_5th = noise_profile->denoise_weight_chroma_5th;

  /* threshold (denoise scale) 12q4 format*/
  for (i = 0; i < 4; i++) {
    denoise_y_cfg->noise_threshold[i] =
      scaleY[i] * noise_profileData[i+4];
    denoise_cb_cfg->noise_threshold[i] =
      scaleChroma[i] * noise_profileData[i+12];
    denoise_cr_cfg->noise_threshold[i] =
      scaleChroma[i] * noise_profileData[i+20];
  }

  denoise_y_cfg->noise_threshold[4] =
    scaleY_5th * noise_profileData[3+4]/2;
  denoise_cb_cfg->noise_threshold[4] =
    scaleChroma_5th * noise_profileData[3+12]/2;
  denoise_cr_cfg->noise_threshold[4] =
    scaleChroma_5th * noise_profileData[3+20]/2;


  /* scale (edge softness) 18q8 format*/
  for (i = 0; i < 4; i++) {
    denoise_y_cfg->bilateral_scale[i] = (64.0/3.0) * sqrt(2*3.14) /
      (edge_softnessY[i] * noise_profileData[i+4]);

    denoise_cb_cfg->bilateral_scale[i] = (64.0/3.0) * sqrt(2*3.14) /
      (edge_softnessChroma[i] * noise_profileData[i+12]);

    denoise_cr_cfg->bilateral_scale[i] = (64.0/3.0) * sqrt(2*3.14) /
      (edge_softnessChroma[i] * noise_profileData[i+20]);
  }


  /* weight 5q4 format*/
  for (i = 0; i < 4; i++) {
    denoise_y_cfg->weight[i] = 1.0 - weightY[i];
    denoise_cb_cfg->weight[i] = 1.0 - weightChroma[i];
    denoise_cr_cfg->weight[i] = 1.0 - weightChroma[i];
  }

  denoise_y_cfg->weight[4] = 1.0 - weightY_5th;
  denoise_cb_cfg->weight[4] = 1.0 - weightChroma_5th;
  denoise_cr_cfg->weight[4] = 1.0 - weightChroma_5th;
}

/** cpp_tnr_params_update_params:
 *
 *  @module_chromatix - Structure that holds all chromatix pointers.
 *  @hw_params - structure That holds all hardware parameters for the
 *      current stream and frame.
 *  @aec_trigger - structure that holds lux_idx and gain for current frame.
 *
 *  The function validates  input data and calls
 *  function to prepare CPP TNR data. If input data is missing, CPP TNR data
 *  is initialized with default values.
 *
 *  Return: It returns 0 at success.
 **/
static int32_t cpp_tnr_params_update_params(modulesChromatix_t *module_chromatix,
  cpp_hardware_params_t *hw_params, cpp_params_aec_trigger_info_t *aec_trigger)
{

  float isp_scale_ratio, cpp_scale_ratio;

  if (!hw_params) {
    CPP_TNR_ERR("invalid hw_params, failed");
    return -EINVAL;
  }

  if (!hw_params->tnr_enable) {
    CPP_TNR_DBG("Temporal wavelet denoise is not enabled");
    return 0;
  }

  if (hw_params->tnr_denoise_lock) {
    CPP_TNR_ERR("Temporal wavelet denoise is locked by Chromatix");
    return 0;
  }

  if (!module_chromatix) {
    cpp_tnr_params_init_tnr_params(hw_params);
    return 0;
  }
  if (!module_chromatix->chromatixCppPtr) {
    if (!hw_params->def_chromatix) {
      CPP_TNR_DBG("no default chromatix set");
      cpp_tnr_params_init_tnr_params(hw_params);
      return -1;
    }
    cpp_tnr_params_fill_stripped(hw_params,
      hw_params->def_chromatix);
    return 0;
  }

  cpp_tnr_params_interpolate(
    module_chromatix->chromatixCppPtr, &hw_params->tnr_info_Y,
    &hw_params->tnr_info_Cb, &hw_params->tnr_info_Cr,aec_trigger,
    hw_params->isp_scale_ratio, hw_params->cpp_scale_ratio);

  return 0;
}

/** fill_hw_buff_list:
 *  @hw_buff_info - pointer to buffer info structure
 *  @stream_params - pointer to stream parameters.
 *
 *  this function populates buffer info structure with data for TNR scratch
 *  buffers. This structure is used to send buffer data during ENQUEUE or
 *  DEQUEUE IOCTL calls to kernel.
 *
 *  On success retuns 0.
 */
static int32_t fill_hw_buff_list(struct msm_cpp_stream_buff_info_t *hw_buff_info,
  cpp_module_stream_params_t  *stream_params)
{
  uint32_t i;

  memset(hw_buff_info, 0, sizeof(struct msm_cpp_stream_buff_info_t));
  /* create and translate to hardware buffer array */
  hw_buff_info->buffer_info = (struct msm_cpp_buffer_info_t *)CAM_MALLOC(
    sizeof(struct msm_cpp_buffer_info_t) *CPP_TNR_SCRATCH_BUFF_COUNT);
  if(NULL == hw_buff_info->buffer_info) {
    CPP_TNR_ERR("error creating hw buff list\n");
    return -EFAULT;
  }
  hw_buff_info->identity = stream_params->identity;
  hw_buff_info->num_buffs = CPP_TNR_SCRATCH_BUFF_COUNT;
  for (i = 0; i < CPP_TNR_SCRATCH_BUFF_COUNT; i++) {
    hw_buff_info->buffer_info[i].fd =
      stream_params->hw_params.tnr_scratch_bfr_array.
      buff_array[i].fd;
    hw_buff_info->buffer_info[i].index =
      stream_params->hw_params.tnr_scratch_bfr_array.
      buff_array[i].index;
    hw_buff_info->buffer_info[i].offset =
      stream_params->hw_params.tnr_scratch_bfr_array.
      buff_array[i].offset;
    hw_buff_info->buffer_info[i].native_buff =
      stream_params->hw_params.tnr_scratch_bfr_array.
      buff_array[i].native_buff;
    hw_buff_info->buffer_info[i].processed_divert =
      stream_params->hw_params.tnr_scratch_bfr_array.
      buff_array[i].processed_divert;
  }
  return 0;
}

/** cpp_tnr_params_set_parm:
 *
 *  @ctrl - module control structure
 *  @identity - Identity of the current stream
 *  @enable - flag the defines whether TNR is enabled or not.
 *
 *  This function is called when a set event from upstream is sent. It enables
 *  or disables TNR module according to the parameter sent from upstream.
 *
 *  Return: It returns 0 at success and error(-ve) on failure.
 **/
static int32_t cpp_tnr_params_set_parm(cpp_module_ctrl_t *ctrl,
  uint32_t identity, uint32_t enable)
{
  uint32_t i;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;

  if(!ctrl) {
    CPP_TNR_ERR("invalid ctrl, failed");
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CPP_TNR_ERR("failed session_params:%p, stream_params:%p\n",
      session_params, stream_params);
    return -EFAULT;
  }

  if (stream_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
    if (stream_params->hw_params.tnr_mask) {
      stream_params->hw_params.tnr_enable = enable;
      stream_params->hw_params.interpolate_mask |= CPP_INTERPOLATE_TNR;
    }
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  } else {
    session_params->hw_params.tnr_enable = enable;
    for (i = 0; i < CPP_MODULE_MAX_STREAMS; i++) {
      if (session_params->stream_params[i]) {
        if (session_params->stream_params[i]->hw_params.tnr_mask) {
          switch (session_params->stream_params[i]->stream_type) {
          case CAM_STREAM_TYPE_PREVIEW:
          case CAM_STREAM_TYPE_VIDEO:
          case CAM_STREAM_TYPE_SNAPSHOT:
          case CAM_STREAM_TYPE_CALLBACK:
              PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
              session_params->stream_params[i]->hw_params.tnr_enable = enable;
              session_params->stream_params[i]->hw_params.interpolate_mask |=
                CPP_INTERPOLATE_TNR;
              PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
            break;
          case CAM_STREAM_TYPE_OFFLINE_PROC:
            //Offline handled through stream params, not an error case
            break;
          default:
            CPP_TNR_ERR("TNR not supported for stream %d",
              session_params->stream_params[i]->stream_type);
            break;
          }
        }
      }
    }
  }
  return 0;
}

/** cpp_tnr_params_prepare:
 *
 *  @control - Holds cpp module control data structure
 *  @identity - Identity on which the prepare request is made.
 *
 *  This function takes care of allocation and queueing of scratch buffers
 *  for tnr. This is called from stream on after iommu attach
 *
 *  Return: It returns 0 on success and error(-ve) on failure.
 **/
static int32_t cpp_tnr_params_prepare(cpp_module_ctrl_t *ctrl,
  uint32_t identity)
{
  uint32_t i, j;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  mct_list_t *tnr_buff_list = NULL;
  struct msm_cpp_stream_buff_info_t hw_strm_buff_info;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  cam_frame_len_offset_t plane_info;
  int rc = 0;

  if(!ctrl) {
    CPP_TNR_ERR("invalid ctrl, failed");
    rc = -EFAULT;
    goto end;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CPP_TNR_ERR("failed session_params:%p, stream_params:%p\n",
     session_params, stream_params);
    rc = -EFAULT;
    goto end;
  }

  if (!stream_params->hw_params.tnr_mask) {
    CPP_TNR_ERR("TNR feature mask not enabled %d for stream %d iden %x",
      stream_params->hw_params.tnr_mask,
      stream_params->hw_params.stream_type, identity);
    rc = 0;
    goto end;
  }

  if (stream_params->hw_params.downsample_mask || (stream_params->native_buff > 0)) {
    rc = cpp_module_util_update_plane_info(&stream_params->hw_params,
      &stream_params->hw_params.input_info, &plane_info);
    if (rc < 0)
      goto end;
  } else {
    plane_info = stream_params->stream_info->buf_planes.plane_info;
  }


  rc = pp_native_buf_mgr_allocate_buff(&ctrl->pp_tnr_buf_mgr,
    CPP_TNR_SCRATCH_BUFF_COUNT,
    CPP_GET_SESSION_ID(stream_params->identity),
    CPP_GET_STREAM_ID(stream_params->identity),
    &plane_info, 1);

  if (rc < 0) {
    CPP_TNR_ERR("Buffer allocation failed\n");
    rc =  -EFAULT;
    goto end;
  }

  rc = pp_native_manager_get_bufs(&ctrl->pp_tnr_buf_mgr,
    CPP_TNR_SCRATCH_BUFF_COUNT, CPP_GET_SESSION_ID(stream_params->identity),
    CPP_GET_STREAM_ID(stream_params->identity),  &tnr_buff_list);
  if ((rc < 0) || (tnr_buff_list == NULL)) {
    CPP_TNR_ERR("Fail to get buffers\n");
    rc = -EFAULT;
    goto tnr_buf_list_error;
  }

  mct_list_traverse(tnr_buff_list,cpp_module_utils_fetch_native_bufs,
    &stream_params->hw_params.tnr_scratch_bfr_array);

  rc = fill_hw_buff_list(&hw_strm_buff_info, stream_params);
  if (rc < 0) {
    CPP_TNR_ERR("fill_hw_buff_list error %d\n", rc);
    goto tnr_buf_list_error;
  }

  PTHREAD_MUTEX_LOCK(&(ctrl->cpphw->mutex));
  v4l2_ioctl.len = sizeof(hw_strm_buff_info);
  v4l2_ioctl.ioctl_ptr = (void *)&hw_strm_buff_info;
  rc = ioctl(ctrl->cpphw->subdev_fd,
    VIDIOC_MSM_CPP_ENQUEUE_STREAM_BUFF_INFO, &v4l2_ioctl);
  if (rc < 0) {
    CPP_TNR_ERR("v4l2 ioctl() failed, rc=%d\n", rc);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->cpphw->mutex));
      rc = -EIO;
      goto buff_info_error;
  }
  PTHREAD_MUTEX_UNLOCK(&(ctrl->cpphw->mutex));


buff_info_error:
  if (hw_strm_buff_info.buffer_info)
    free(hw_strm_buff_info.buffer_info);
tnr_buf_list_error:
  mct_list_free_list(tnr_buff_list);
  if (rc < 0)
    pp_native_buf_mgr_free_queue(&ctrl->pp_tnr_buf_mgr,
      CPP_GET_SESSION_ID(stream_params->identity),
      CPP_GET_STREAM_ID(stream_params->identity));
end:
  return rc;
}

/** cpp_tnr_params_unprepare:
 *
 *  @control - Holds cpp module control data structure
 *  @identity - Identity on which the prepare request is made.
 *
 *  This function takes care of clean up of the scratch buffers
 *  allocated for tnr.
 *
 *  Return: It is returns 0 on success and error(-ve) on failure.
 **/
static int32_t cpp_tnr_params_unprepare(cpp_module_ctrl_t *ctrl,
  uint32_t identity)
{
  uint32_t i, j;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_hardware_buffer_info_t *buff_holder;
  struct msm_cpp_stream_buff_info_t hw_strm_buff_info;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  int rc = 0;

  if(!ctrl) {
    CPP_TNR_ERR("invalid ctrl, failed");
    rc = -EFAULT;
    goto end;
  }
  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CPP_TNR_ERR("failed session_params:%p, stream_params:%p\n",
      session_params, stream_params);
    rc = -EFAULT;
    goto end;
  }

  if (!stream_params->hw_params.tnr_mask) {
    CPP_TNR_ERR("TNR feature mask not enabled %d",
      stream_params->hw_params.tnr_mask);
    rc = 0;
    goto end;
  }

  rc = fill_hw_buff_list(&hw_strm_buff_info, stream_params);
  if (rc < 0) {
    goto buff_info_free;
  }

  PTHREAD_MUTEX_LOCK(&(ctrl->cpphw->mutex));
  v4l2_ioctl.len =
    sizeof(hw_strm_buff_info);
  v4l2_ioctl.ioctl_ptr =
    (void *)&hw_strm_buff_info;
  rc = ioctl(ctrl->cpphw->subdev_fd,
    VIDIOC_MSM_CPP_DELETE_STREAM_BUFF, &v4l2_ioctl);
  if (rc < 0) {
    CPP_TNR_ERR("v4l2 ioctl() failed, rc=%d\n", rc);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->cpphw->mutex));
    rc = -EIO;
    goto buff_info_free;
  }
  PTHREAD_MUTEX_UNLOCK(&(ctrl->cpphw->mutex));

  for (j = 0; j < CPP_TNR_SCRATCH_BUFF_COUNT; j++) {
    buff_holder =
      &stream_params->hw_params.tnr_scratch_bfr_array.buff_array[j];

    rc = pp_native_manager_put_buf(&ctrl->pp_tnr_buf_mgr,
      CPP_GET_SESSION_ID(stream_params->identity),
      CPP_GET_STREAM_ID(stream_params->identity),
      buff_holder->index);

    if (rc < 0) {
      CPP_TNR_ERR("Fail to put buffer index %d fd %d",buff_holder->index,
        buff_holder->fd);
      continue;
    }
    buff_holder->fd = 0;
  }


buff_info_free:
  if(pp_native_buf_mgr_queue_exists(&ctrl->pp_tnr_buf_mgr,
    CPP_GET_SESSION_ID(stream_params->identity),
    CPP_GET_STREAM_ID(stream_params->identity))) {
    pp_native_buf_mgr_free_queue(&ctrl->pp_tnr_buf_mgr,
      CPP_GET_SESSION_ID(stream_params->identity),
      CPP_GET_STREAM_ID(stream_params->identity));
  }

  if (hw_strm_buff_info.buffer_info)
    free(hw_strm_buff_info.buffer_info);
end:
   return rc;
}

/** cpp_tnr_params_swap_scratch_buff:
 *
 *  @ctrl - module control structure
 *  @identity - Identity of the current stream
 *
 *  This function is used to swap the scratch buffers used by TNR module. The
 *  buffer used to wirte in the current frame is placed for read for the next.
 *  The function is called every frame done event.
 *
 *  Return: It returns 0 at success.
 **/
static int32_t cpp_tnr_params_swap_scratch_buff(cpp_module_ctrl_t *ctrl,
  uint32_t identity)
{
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;

  /* get parameters based on the event identity */
  cpp_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params || !stream_params) {
    CPP_TNR_ERR("failed session_params:%p, stream_params:%p\n",
      session_params, stream_params);
    return -EFAULT;
  }

  if (stream_params->hw_params.tnr_enable) {
    cpp_hardware_buffer_info_t swap_buf;

    memcpy(&swap_buf, &stream_params->hw_params.
      tnr_scratch_bfr_array.buff_array[0],
      sizeof(cpp_hardware_buffer_info_t));
    memcpy(&stream_params->hw_params.tnr_scratch_bfr_array.buff_array[0],
      &stream_params->hw_params.tnr_scratch_bfr_array.buff_array[1],
      sizeof(cpp_hardware_buffer_info_t));
    memcpy(&stream_params->hw_params.tnr_scratch_bfr_array.buff_array[1],
      &swap_buf, sizeof(cpp_hardware_buffer_info_t));

  }

  return 0;
}

/** cpp_module_tnr_init:
 *
 *  @hw_version - Holds CPP hardware version
 *  @tnr_module_func_tbl - TNR module function table
 *
 *  This function initialized the module's function table. If the module present
 *  in the current architecture this function should be called first before the
 *  module methods (set, update and private) should be used.
 *
 *  Return: It is void function.
 **/
void cpp_module_tnr_init(uint32_t hw_caps,
  cpp_submodule_func_tbl_t *tnr_module_func_tbl)
{
  if (!(hw_caps & TNR_CAPS)) {
    CPP_TNR_DBG("Not supported hardware version\n\r");
    tnr_module_func_tbl->update = NULL;
    tnr_module_func_tbl->set = NULL;
    tnr_module_func_tbl->prepare = NULL;
    tnr_module_func_tbl->unprepare = NULL;
    tnr_module_func_tbl->private = NULL;
  } else {
    CPP_TNR_DBG("Init tnr function table\n\r");
    tnr_module_func_tbl->update = cpp_tnr_params_update_params;
    tnr_module_func_tbl->set = cpp_tnr_params_set_parm;
    tnr_module_func_tbl->prepare = cpp_tnr_params_prepare;
    tnr_module_func_tbl->unprepare = cpp_tnr_params_unprepare;
    tnr_module_func_tbl->private = cpp_tnr_params_swap_scratch_buff;
  }
}
