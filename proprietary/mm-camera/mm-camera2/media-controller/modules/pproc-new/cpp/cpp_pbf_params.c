/*============================================================================

  Copyright (c) 2015-2017 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

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



int32_t cpp_pbf_params_init_pbf_params(cpp_hardware_params_t *cpp_hw_params)
{
  uint32_t j;
  uint32_t profile = 8;

  if (!cpp_hw_params) {
    CPP_TNR_ERR("invalid cpp hw params, failed");
    return -EINVAL;
  }

  for (j = 0; j < CPP_DENOISE_NUM_PLANES; j++) {
    cpp_hw_params->pbf_denoise_info[j].noise_profile = profile;
    cpp_hw_params->pbf_denoise_info[j].weight = 0;
    cpp_hw_params->pbf_denoise_info[j].edge_softness = 12.75;
    cpp_hw_params->pbf_denoise_info[j].denoise_ratio = 12.75;
  }

  return 0;
}


int32_t cpp_pbf_params_interpolate_pbf_params(float interpolation_factor,
  cpp_hardware_params_t *cpp_hw_params, void *ref_noise_profile_i,
  void *ref_noise_profile_iplus1)
{
  uint32_t k, j, offset;

  if (!cpp_hw_params || !ref_noise_profile_i || !ref_noise_profile_iplus1) {
    CPP_TNR_ERR("failed, hw_params %p, noise_profile_i %p, iplus1 %p",
      cpp_hw_params, ref_noise_profile_i, ref_noise_profile_iplus1);
    return -EINVAL;
  }

    /* Weight factor */
    cpp_hw_params->pbf_denoise_info[0].weight =
      LINEAR_INTERPOLATE(interpolation_factor,
        PBF_WEIGHT_Y_DATA(ref_noise_profile_i),
        PBF_WEIGHT_Y_DATA(ref_noise_profile_iplus1));
    cpp_hw_params->pbf_denoise_info[1].weight =
      LINEAR_INTERPOLATE(interpolation_factor,
        PBF_WEIGHT_CHROMA_DATA(ref_noise_profile_i),
        PBF_WEIGHT_CHROMA_DATA(ref_noise_profile_iplus1));
    cpp_hw_params->pbf_denoise_info[2].weight =
      LINEAR_INTERPOLATE(interpolation_factor,
        PBF_WEIGHT_CHROMA_DATA(ref_noise_profile_i),
        PBF_WEIGHT_CHROMA_DATA(ref_noise_profile_iplus1));

    /* denoise ratio */
    cpp_hw_params->pbf_denoise_info[0].denoise_ratio =
      LINEAR_INTERPOLATE(interpolation_factor,
        PBF_SCALE_Y_DATA(ref_noise_profile_i),
        PBF_SCALE_Y_DATA(ref_noise_profile_iplus1));
    cpp_hw_params->pbf_denoise_info[1].denoise_ratio =
      LINEAR_INTERPOLATE(interpolation_factor,
        PBF_SCALE_CHROMA_DATA(ref_noise_profile_i),
        PBF_SCALE_CHROMA_DATA(ref_noise_profile_iplus1));
    cpp_hw_params->pbf_denoise_info[2].denoise_ratio =
      LINEAR_INTERPOLATE(interpolation_factor,
        PBF_SCALE_CHROMA_DATA(ref_noise_profile_i),
        PBF_SCALE_CHROMA_DATA(ref_noise_profile_iplus1));

    /* edge softness factor */
    cpp_hw_params->pbf_denoise_info[0].edge_softness =
      LINEAR_INTERPOLATE(interpolation_factor,
        PBF_EDGE_SOFTNESS_Y_DATA(ref_noise_profile_i),
        PBF_EDGE_SOFTNESS_Y_DATA(ref_noise_profile_iplus1));
    cpp_hw_params->pbf_denoise_info[1].edge_softness =
      LINEAR_INTERPOLATE(interpolation_factor,
        PBF_EDGE_SOFTNESS_CHROMA_DATA(ref_noise_profile_i),
        PBF_EDGE_SOFTNESS_CHROMA_DATA(ref_noise_profile_iplus1));
    cpp_hw_params->pbf_denoise_info[2].edge_softness =
      LINEAR_INTERPOLATE(interpolation_factor,
        PBF_EDGE_SOFTNESS_CHROMA_DATA(ref_noise_profile_i),
        PBF_EDGE_SOFTNESS_CHROMA_DATA(ref_noise_profile_iplus1));

    cpp_hw_params->pbf_denoise_info[0].noise_profile =
      LINEAR_INTERPOLATE(interpolation_factor,
        PBF_NOISE_PROFILE_Y_DATA(ref_noise_profile_i),
        PBF_NOISE_PROFILE_Y_DATA(ref_noise_profile_iplus1));
    cpp_hw_params->pbf_denoise_info[1].noise_profile =
      LINEAR_INTERPOLATE(interpolation_factor,
        PBF_NOISE_PROFILE_CB_DATA(ref_noise_profile_i),
        PBF_NOISE_PROFILE_CB_DATA(ref_noise_profile_iplus1));
    cpp_hw_params->pbf_denoise_info[2].noise_profile =
      LINEAR_INTERPOLATE(interpolation_factor,
        PBF_NOISE_PROFILE_CR_DATA(ref_noise_profile_i),
        PBF_NOISE_PROFILE_CR_DATA(ref_noise_profile_iplus1));

    return 0;
}

/** cpp_pbf_params_get_downscale_ratio:
 *
 *  @pbf_chromatix_data - Pointer to the wavelet chromatix data. It holds
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
static void cpp_pbf_params_get_downscale_ratio(void *pbf_downscale_data,
  float scale_ratio, uint32_t *reg_strt_idx_start, uint32_t *reg_strt_idx_end,
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
    scale_strt = GET_PBF_DOWNSCALE_LEVELS(pbf_downscale_data, scale_idx_start);
    scale_end  = GET_PBF_DOWNSCALE_LEVELS(pbf_downscale_data, scale_idx_end);

    if ( scale_strt >= scale_end ) {
      CPP_TNR_DBG("WNR Down Scaling Ratio Settings are not in correct order");
      *reg_strt_idx_start = i;
      *reg_strt_idx_end = i;
      return;
    }

    if ((scale_ratio >= scale_strt) && (scale_ratio < scale_end)) {
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

/** cpp_pbf_params_get_upscale_ratio:
 *
 *  @pbf_upscale_data - Pointer to the wavelet chromatix data. It holds
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
static void cpp_pbf_params_get_upscale_ratio( void *pbf_upscale_data,
  float scale_ratio, uint32_t *reg_strt_idx_start, uint32_t *reg_strt_idx_end,
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
    scale_strt = GET_PBF_UPSCALE_LEVELS(pbf_upscale_data, scale_idx_start);
    scale_end  = GET_PBF_UPSCALE_LEVELS(pbf_upscale_data, scale_idx_end);

    if ( scale_strt <= scale_end ) {
      CPP_TNR_DBG("WNR Up Scaling Ratio Settings are not in correct order");
      *reg_strt_idx_start = i;
      *reg_strt_idx_end = i;
      return;
    }

    if ((scale_ratio < scale_strt) && (scale_ratio >scale_end)) {
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

/** cpp_pbf_params_interpolate_zoom_scale:
 *
 *  @hw_params - hardware parameters of the current stream
 *  @wavelet_chromatix_data - Pointer to the wavelet chromatix data. It holds
 *     scale adjustment's regions and the other TNR parameters.
 *  @scale_ratio - Scale ration calculated between ISP input dimension and CPP
 *     output dimension.
 *  @reg_strt_idx_start - Scale start index .
 *  @reg_strt_idx_end - Scale end index
 *  @reg_strt_interp_ratio - Scale interpolation ratio.
 *
 *  This function interpolates adjustment scale parameters from selected regions.
 *  Interpolation depends on the scale ratio. Upscale and downscale regions are
 *  different. If there is no scale the scale adjustment values are 1.0f.
 *
 **/
void cpp_pbf_params_interpolate_zoom_scale(cpp_hardware_params_t *hw_params,
  void *downscale_data, void *upscale_data, float scale_ratio,
  uint32_t reg_strt_idx_start, uint32_t reg_strt_idx_end,
  float reg_strt_interp_ratio, uint32_t scalor)
{

  if (scale_ratio > 1.0f ) {
    hw_params->pbf_denoise_info[0].profile_adj[scalor] =
      LINEAR_INTERPOLATE(reg_strt_interp_ratio,
        PBF_DOWN_PROFILE_Y_ADJ(downscale_data, reg_strt_idx_start),
        PBF_DOWN_PROFILE_Y_ADJ(downscale_data, reg_strt_idx_end));
    hw_params->pbf_denoise_info[1].profile_adj[scalor] =
      LINEAR_INTERPOLATE(reg_strt_interp_ratio,
        PBF_DOWN_PROFILE_CBCR_ADJ(downscale_data, reg_strt_idx_start),
        PBF_DOWN_PROFILE_CBCR_ADJ(downscale_data, reg_strt_idx_end));
    hw_params->pbf_denoise_info[2].profile_adj[scalor] =
      LINEAR_INTERPOLATE(reg_strt_interp_ratio,
        PBF_DOWN_PROFILE_CBCR_ADJ(downscale_data, reg_strt_idx_start),
        PBF_DOWN_PROFILE_CBCR_ADJ(downscale_data, reg_strt_idx_end));


/* interpolate denoise_scale_y */
    hw_params->pbf_denoise_info[0].weight_VFE_adj[scalor] =
      LINEAR_INTERPOLATE(reg_strt_interp_ratio,
        PBF_DOWN_WEIGHT_Y_ADJ(downscale_data, reg_strt_idx_start),
        PBF_DOWN_WEIGHT_Y_ADJ(downscale_data, reg_strt_idx_end));
    hw_params->pbf_denoise_info[1].weight_VFE_adj[scalor] =
      LINEAR_INTERPOLATE(reg_strt_interp_ratio,
        PBF_DOWN_WEIGHT_CBCR_ADJ(downscale_data, reg_strt_idx_start),
        PBF_DOWN_WEIGHT_CBCR_ADJ(downscale_data, reg_strt_idx_end));
    hw_params->pbf_denoise_info[2].weight_VFE_adj[scalor] =
      LINEAR_INTERPOLATE(reg_strt_interp_ratio,
        PBF_DOWN_WEIGHT_CBCR_ADJ(downscale_data, reg_strt_idx_start),
        PBF_DOWN_WEIGHT_CBCR_ADJ(downscale_data, reg_strt_idx_end));
  } else if ( scale_ratio < 1.0f ) {
    hw_params->pbf_denoise_info[0].profile_adj[scalor] =
      LINEAR_INTERPOLATE(reg_strt_interp_ratio,
        PBF_UP_PROFILE_Y_ADJ(upscale_data, reg_strt_idx_start),
        PBF_UP_PROFILE_Y_ADJ(upscale_data, reg_strt_idx_end));
    hw_params->pbf_denoise_info[1].profile_adj[scalor] =
      LINEAR_INTERPOLATE(reg_strt_interp_ratio,
        PBF_UP_PROFILE_CBCR_ADJ(upscale_data, reg_strt_idx_start),
        PBF_UP_PROFILE_CBCR_ADJ(upscale_data, reg_strt_idx_end));

    hw_params->pbf_denoise_info[2].profile_adj[scalor] =
      LINEAR_INTERPOLATE(reg_strt_interp_ratio,
        PBF_UP_PROFILE_CBCR_ADJ(upscale_data, reg_strt_idx_start),
        PBF_UP_PROFILE_CBCR_ADJ(upscale_data, reg_strt_idx_end));

/* interpolate denoise_scale_y */
    hw_params->pbf_denoise_info[0].weight_VFE_adj[scalor] =
      LINEAR_INTERPOLATE(reg_strt_interp_ratio,
        PBF_UP_WEIGHT_Y_ADJ(upscale_data, reg_strt_idx_start),
        PBF_UP_WEIGHT_Y_ADJ(upscale_data, reg_strt_idx_end));
    hw_params->pbf_denoise_info[1].weight_VFE_adj[scalor] =
      LINEAR_INTERPOLATE(reg_strt_interp_ratio,
        PBF_UP_WEIGHT_CBCR_ADJ(upscale_data, reg_strt_idx_start),
        PBF_UP_WEIGHT_CBCR_ADJ(upscale_data, reg_strt_idx_end));
    hw_params->pbf_denoise_info[2].weight_VFE_adj[scalor] =
      LINEAR_INTERPOLATE(reg_strt_interp_ratio,
        PBF_UP_WEIGHT_CBCR_ADJ(upscale_data, reg_strt_idx_start),
        PBF_UP_WEIGHT_CBCR_ADJ(upscale_data, reg_strt_idx_end));
  } else {
/* No scale */
    hw_params->pbf_denoise_info[0].profile_adj[scalor] = 1.0f;
    hw_params->pbf_denoise_info[1].profile_adj[scalor] = 1.0f;
    hw_params->pbf_denoise_info[2].profile_adj[scalor] = 1.0f;
    hw_params->pbf_denoise_info[0].weight_VFE_adj[scalor] =1.0f;
    hw_params->pbf_denoise_info[1].weight_VFE_adj[scalor] =1.0f;
    hw_params->pbf_denoise_info[2].weight_VFE_adj[scalor] =1.0f;
  }
}


int32_t cpp_pbf_params_update_params(modulesChromatix_t *module_chromatix,
  cpp_hardware_params_t *hw_params, cpp_params_aec_trigger_info_t *aec_trigger)
{
  void                        *pbf_denoise;
  void                        *ref_noise_profile_i;
  void                        *ref_noise_profile_iplus1;
  void                        *upscale_data;
  void                        *downscale_data;
  uint32_t                    i;
  float                       trigger_start, trigger_end;
  float                       interpolation_factor;
  float                       numerator, denominator;
  float                       trigger_input;
  chromatix_cpp_type *        chromatix_cpp = NULL;
  uint32_t                    reg_strt_idx_start = 0;
  uint32_t                    reg_strt_idx_end = 0;
  float                       reg_strt_interp_ratio = 0.0f;
  float                       scale_ratio = 1.0f;

  if (!hw_params) {
    CPP_TNR_ERR("invalid, hwparams, failed");
    return -EINVAL;
  }

  if (hw_params->denoise_lock) {
    CPP_TNR_ERR("Wavelet denoise is locked by Chromatix");
    return 0;
  }

  if (!hw_params->pbf_enable) {
    CPP_TNR_DBG("Prescale BF is not enabled");
    return 0;
  }

  if (!module_chromatix) {
    CPP_TNR_ERR("invalid module_chromatix, failed");
    return -1;
  }

  chromatix_cpp = (chromatix_cpp_type *)module_chromatix->chromatixCppPtr;
  if (!chromatix_cpp) {
    cpp_pbf_params_init_pbf_params(hw_params);
    return 0;
  } else {
    GET_PBF_DENOISE_POINTER(chromatix_cpp, hw_params, pbf_denoise);
    GET_PBF_UPSCALE_DATA(chromatix_cpp, hw_params, upscale_data);
    GET_PBF_DOWNSCALE_DATA(chromatix_cpp, hw_params, downscale_data);
    if (GET_PBF_TRIGGER_TYPE(pbf_denoise) == 0) {
      CPP_TNR_DBG("lux triggered");
      /* Lux index based */
      trigger_input = aec_trigger->lux_idx;
    } else {
      CPP_TNR_DBG("gain triggered");
      /* Gain based */
      trigger_input = aec_trigger->gain;
    }

    if (trigger_input <= 0.0f) {
      CPP_TNR_ERR("invalid trigger input %f", trigger_input);
      return 0;
    }

    /* Find the range in the availble grey patches */
    for (i = 0; i < NUM_GRAY_PATCHES - 1; i++) {
      if (GET_PBF_TRIGGER_TYPE(pbf_denoise) == 0) {
        trigger_start = GET_PBF_LUXIDX_TRIGER_START(pbf_denoise, i);
        trigger_end = GET_PBF_LUXIDX_TRIGER_END(pbf_denoise, i);
      } else {
        trigger_start = GET_PBF_GAIN_TRIGER_START(pbf_denoise, i);
        trigger_end = GET_PBF_GAIN_TRIGER_END(pbf_denoise, i);
      }

      if ( hw_params->isp_scale_ratio > 1.0f ) {
        cpp_pbf_params_get_downscale_ratio(downscale_data,
          hw_params->isp_scale_ratio, &reg_strt_idx_start, &reg_strt_idx_end,
          &reg_strt_interp_ratio);
      } else if ( hw_params->isp_scale_ratio < 1.0f ) {
        cpp_pbf_params_get_upscale_ratio(upscale_data, hw_params->isp_scale_ratio,
          &reg_strt_idx_start, &reg_strt_idx_end, &reg_strt_interp_ratio);
      }

      cpp_pbf_params_interpolate_zoom_scale(hw_params,downscale_data,
        upscale_data, hw_params->isp_scale_ratio, reg_strt_idx_start,
        reg_strt_idx_end, reg_strt_interp_ratio,ISP_SCALOR);

      reg_strt_idx_start = 0;
      reg_strt_idx_end = 0;
      reg_strt_interp_ratio = 0;

      if ( hw_params->cpp_scale_ratio > 1.0f ) {
        cpp_pbf_params_get_downscale_ratio(downscale_data,
          hw_params->cpp_scale_ratio, &reg_strt_idx_start, &reg_strt_idx_end,
          &reg_strt_interp_ratio);
      } else if ( hw_params->cpp_scale_ratio < 1.0f ) {
        cpp_pbf_params_get_upscale_ratio(upscale_data, hw_params->cpp_scale_ratio,
          &reg_strt_idx_start, &reg_strt_idx_end, &reg_strt_interp_ratio);
      }

      cpp_pbf_params_interpolate_zoom_scale(hw_params, downscale_data,
        upscale_data, hw_params->cpp_scale_ratio, reg_strt_idx_start,
        reg_strt_idx_end, reg_strt_interp_ratio,CPP_SCALOR);

      if (trigger_input <= trigger_start) {
        CPP_TNR_DBG("non interpolate");
        GET_PBF_NOISE_PROFILE(pbf_denoise, i, ref_noise_profile_i);
        cpp_pbf_params_interpolate_pbf_params(0, hw_params, ref_noise_profile_i,
          ref_noise_profile_i);
        return 0;
      }
      if (trigger_input < trigger_end) {
        /* Interpolate all the values */
        numerator = (trigger_input - trigger_start);
        denominator = (trigger_end - trigger_start);
        if (denominator == 0.0f) {
          return 0;
        }
        interpolation_factor = numerator / denominator;
        GET_PBF_NOISE_PROFILE(pbf_denoise, i, ref_noise_profile_i);
        GET_PBF_NOISE_PROFILE(pbf_denoise, i+1, ref_noise_profile_iplus1);

        CPP_TNR_DBG("interpolate, factor=%f, i=%d, i+1=%d", interpolation_factor, i, i+1);

        cpp_pbf_params_interpolate_pbf_params(interpolation_factor,
          hw_params, ref_noise_profile_i,
          ref_noise_profile_iplus1);
        return 0;
      } /* else iterate */
    }

    if (i == (NUM_GRAY_PATCHES - 1)) {
      CPP_TNR_DBG("default non interpolate");
      GET_PBF_NOISE_PROFILE(pbf_denoise, NUM_GRAY_PATCHES - 1,
        ref_noise_profile_i);
      cpp_pbf_params_interpolate_pbf_params(0, hw_params, ref_noise_profile_i,
        ref_noise_profile_i);
      return 0;
    }
  }

  return 0;
}

/** cpp_pbf_params_set_parm:
 *
 *  @ctrl - module control structure
 *  @identity - Identity of the current stream
 *
 *  This function is called when a set event from upstream is sent. It enables
 *  or disables PBF module for the particular stream.
 *
 *  Return: It returns 0 at success.
 **/
int32_t cpp_pbf_params_set_parm(cpp_module_ctrl_t *ctrl,
  uint32_t identity, uint32_t enable)
{
  uint32_t                          i,j;
  cpp_module_stream_params_t  *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  int32_t                      rc = 0;

  if(!ctrl) {
    CPP_TNR_ERR("invalid cpp ctrl, failed");
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

  for (i=0; i<CPP_MODULE_MAX_STREAMS; i++){
    if (session_params->stream_params[i]) {
      stream_params = session_params->stream_params[i];
      if(stream_params->identity == identity) {
        if((boolean)enable != stream_params->hw_params.pbf_enable) {
          PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
          stream_params->hw_params.pbf_enable = enable;
          PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
          if (stream_params->hw_params.pbf_enable) {
            cpp_pbf_params_update_params(&stream_params->hw_params.module_chromatix,
              &stream_params->hw_params, &session_params->aec_trigger);
          }
        }
        break;
      }
    }
  }

  if (i == CPP_MODULE_MAX_STREAMS) {
    CPP_TNR_ERR("Stream with identity 0x%x is not found", identity);
  }

  return 0;
}

/** cpp_module_pbf_init:
 *
 *  @hw_version - Holds CPP hardware version
 *  @pbf_module_func_tbl - TNR module function table
 *
 *  This function initialized the module's function table. If the module present
 *  in the current architecture this function should be called first before the
 *  module methods (set, update and private) should be used.
 *
 *  Return: It is void function.
 **/
void cpp_module_pbf_init(uint32_t hw_caps,
  cpp_submodule_func_tbl_t *pbf_module_func_tbl)
{
  /* PBF is used with TNR , Use TNR caps for initializing PBF table */
  if (!(hw_caps & TNR_CAPS)) {
    CPP_TNR_DBG("Not supported hardware version\n\r");
    pbf_module_func_tbl->update = NULL;
    pbf_module_func_tbl->set = NULL;
    pbf_module_func_tbl->prepare = NULL;
    pbf_module_func_tbl->unprepare = NULL;
    pbf_module_func_tbl->private = NULL;
  } else {
    CPP_TNR_DBG("Init tnr function table\n\r");
    pbf_module_func_tbl->update = cpp_pbf_params_update_params;
    pbf_module_func_tbl->set = cpp_pbf_params_set_parm;
    pbf_module_func_tbl->prepare = NULL;
    pbf_module_func_tbl->unprepare = NULL;
    pbf_module_func_tbl->private = NULL;
  }
}


void cpp_prepare_pbf_info(struct cpp_frame_info_t *frame_info)
{
  uint32_t i, j;
  cpp_prescaler_bf_info_t *pbf_info;

  for (j = 0; j < CPP_DENOISE_NUM_PLANES; j++) {
    pbf_info = &frame_info->prescaler_info[j];
    frame_info->pbf_noise_profile[j] *= frame_info->pbf_profile_adj[j];
    frame_info->pbf_weight[j] *= frame_info->pbf_weight_VFE_adj[j];

    if ((frame_info->pbf_edge_softness[j] == 0.00f) ||
       (frame_info->pbf_noise_profile[j] == 0.00f)) {
      pbf_info->bilateral_scale = 0;
    } else {
      pbf_info->bilateral_scale = ((double) 64.0/3.0) /
        (sqrt((double) frame_info->pbf_edge_softness[j] / 1.31) *
        frame_info->pbf_noise_profile[j]) / 9;
    }

    pbf_info->weight = 1.0 - frame_info->pbf_weight[j];
    pbf_info->noise_threshold = frame_info->pbf_denoise_ratio[j] *
      frame_info->pbf_noise_profile[j];

  }
}
