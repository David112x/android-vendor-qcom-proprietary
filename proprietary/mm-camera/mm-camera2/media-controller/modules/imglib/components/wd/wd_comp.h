/**********************************************************************
* Copyright (c) 2013,2015 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __WD_COMP_H__
#define __WD_COMP_H__

#include "denoise.h"
#include "img_comp_priv.h"
#include "chromatix.h"
#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || \
  defined (CHROMATIX_307) || defined (CHROMATIX_308) || \
  defined(CHROMATIX_308E) || defined (CHROMATIX_309) || \
  defined (CHROMATIX_310) || defined (CHROMATIX_310E)
#include "chromatix_cpp.h"
#endif
#include "wavelet_denoise_lib.h"

/** MODULE_MASK:
 *
 * Mask to enable dynamic logging
 **/
#undef MODULE_MASK
#define MODULE_MASK IMGLIB_WNR_SHIFT

/** wd_comp_t
 *   @b: base image component
 *   @lines_to_pad: number of lines to pad the image
 *   @p_noiseprof: pointer to the noise profile
 *   @mode: wavelet denoise mode
 *   @y_complexity: luma complexity
 *   @cbcr_complexity: chroma complexity
 *   @gamma_set: flag to indicate whether the gamma is set
 *   @lowlight_gamma_set: flag to indicate whether the low light
 *                      gamma is set
 *   @lowlight_gamma: lowlight gamma table
 *   @current_gamma: gamma table of the current frame
 *   @info_3a: 3A info passed by the client
 *   @p_chromatix: chromatix pointer passed by the client
 *   @mapiondsp: DSP ION buffer mapping data (buffer)
 *   @mapiondspheap: DSP ION buffer mapping data (heap)
 *   @mapiondspnoiseprofile: DSP ION buffer mapping data (noise profile)
 *   @dspInitFlag: Is DSP ION buffer initialized
 *   @mapiongpu: GPU ION buffer mapping data
 *   @gpuInitFlag: Is GPU ION buffer initialized
 *   @denoiseCtrl: denoise library control structure
 *
 *   @early_cb_enabled: Indicates if early callback enabled for
 *                    burst mode
 *   Wavelet denoise component structure
 **/
typedef struct {
  /*base component*/
  img_component_t b;
  uint32_t lines_to_pad;
  CameraDenoisingType *p_noiseprof;
  wd_mode_t mode;
  int y_complexity;
  int cbcr_complexity;
  uint8_t gamma_set;
  uint8_t lowlight_gamma_set;
  img_gamma_t lowlight_gamma;
  img_gamma_t current_gamma;
  wd_3a_info_t info_3a;
  chromatix_parms_type *p_chromatix;
#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || \
  defined (CHROMATIX_307) || defined (CHROMATIX_308) || \
  defined(CHROMATIX_308E) || defined (CHROMATIX_309) || \
  defined (CHROMATIX_310) || defined (CHROMATIX_310E)
  chromatix_cpp_type   *p_cpp_chromatix;
#endif
  img_mmap_info_ion mapiondsp;
  img_mmap_info_ion mapiondspheap;
  img_mmap_info_ion mapiondspnoiseprofile;
  uint32_t dspInitFlag;
  img_mmap_info_ion mapiongpu;
  uint32_t gpuInitFlag;
  uint8_t early_cb_enabled;
  denoise_lib_t denoiseCtrl;
} wd_comp_t;

#endif //__WD_COMP_H__

