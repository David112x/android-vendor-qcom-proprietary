/**********************************************************************
*  Copyright (c) 2013-2014, 2018-2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "wd_comp.h"
#include <math.h>

#ifdef USE_SMMU_BUFFERS_FOR_WNR
#include "remote.h"
#endif

/**
 * CONSTANTS and MACROS
 **/
#ifndef Round
#define Round(x) (int)(x + sign(x)*0.5)
#endif

#ifndef sign
#define sign(x) ((x < 0) ?(-1) : (1))
#endif

#ifndef BILINEAR_INTERPOLATION
#define BILINEAR_INTERPOLATION(v1, v2, ratio) ((v1) + ((ratio) * ((v2) - (v1))))
#endif

#define NUM_LUMA_NOISE_PROFILE 8
#define SZ_2M  (2000000)
#define SZ_4M  (4000000)
#define SZ_8M  (8000000)
#define SZ_11M  (11000000)

static const int q_factor_8bit = 7;
static const int q_factor_10bit = 5;
// static const int parameter_q_factor = 15;
// static const int parameter_roundoff = (1 << (15 - 1));

const CameraDenoisingType noiseProfileData_Q20_default =
{
  24,
  { 20709376,    36558852,    34905636,    49914316,
    10485759,    29917176,    27668980,    39617552,
    5505033,    16054288,    33724468,    47268448,
    2359297,     6307848,    24566306,    49660284,
    6291455,    20389880,    43197724,    55904252,
    2621442,     9580551,    26350442,    51002688 },
    1,
    {6554,6554,6554,6554},
    {10,10,10,10},
    {0,0,0,0},
    {15,15,15,15},
    204,
    204
};

static core_type_t g_yc_core_aff_full[3][MAX_SEGMENTS] = {
  {CORE_DSP, CORE_DSP, CORE_DSP, CORE_DSP, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY},
  {CORE_ARM, CORE_ARM, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY},
  {CORE_ARM, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY}
};
static core_type_t g_yc_core_aff_chroma[3][MAX_SEGMENTS] = {
  {CORE_DSP, CORE_DSP, CORE_DSP, CORE_DSP, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY},
  {CORE_DSP, CORE_DSP, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY},
  {CORE_DSP, CORE_ARM, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY, CORE_ANY}
};

typedef struct {
  void *ptr;
  int (*camera_denoising_manager)(denoise_lib_t  *p_lib);
  int (*camera_denoising_get_alloc_prop)(const denoise_lib_t *p_lib,
    denoise_alloc_prop_t* p_denoise_alloc_prop);
  int (*camera_denoising_init)();
} wd_lib_info_t;

static wd_lib_info_t g_wd_lib;

int wd_comp_abort(void *handle, void *p_data);

extern void remote_register_buf(void* buf, int size, int fd);
#pragma weak  remote_register_buf

/**
 * Function: register_buf
 *
 * Description: Register the DSP buffer to SMMU
 *
 * Input parameters:
 *   buf  - The virtual DSP buffer pointer.
 *   size - size of the buffer
 *   fd   - ION FD of the DSP buf
 *
 * Return values:
 *     None
 *
 * Notes: none
 **/
void register_buf(void* buf, int size, int fd) {
  if (remote_register_buf) {
    remote_register_buf(buf, size, fd);
   }
}

/**
 * Function: wd_comp_init
 *
 * Description: Initializes the Qualcomm Wavelet denoise component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_userdata - the handle which is passed by the client
 *   p_data - The pointer to the parameter which is required during the
 *            init phase
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int wd_comp_init(void *handle, void* p_userdata, void *p_data)
{
  wd_comp_t *p_comp = (wd_comp_t *)handle;
  int status = IMG_SUCCESS;
  uint32_t lines_to_pad;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  status = p_comp->b.ops.init(&p_comp->b, p_userdata, p_data);
  if (status < 0)
    return status;

  if (NULL != p_data) {
    p_comp->mode = *((wd_mode_t *)p_data);
    IDBG_HIGH("%s:%d] mode %d", __func__, __LINE__, p_comp->mode);
  }

  p_comp->p_noiseprof =
      (CameraDenoisingType *)malloc(sizeof(CameraDenoisingType));

  if (p_comp->p_noiseprof == NULL ) {
    IDBG_ERROR("%s:%d] Error\n", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }
  memcpy(p_comp->p_noiseprof, &noiseProfileData_Q20_default,
    sizeof(CameraDenoisingType));

  /*lines required for padding (2*(2^n)-2)
  for number of levels < 4 we add 16 lines to make it integer number of MCU row
  for number of levels> 4 we add 2*2^n which is multiple of MCU rows*/
  if (NUMBER_OF_LEVELS < 4) {
    lines_to_pad = 64;
  } else {
    lines_to_pad = 2 * (1 << (NUMBER_OF_LEVELS + 1));
  }
  p_comp->lines_to_pad = lines_to_pad;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: wd_comp_deinit
 *
 * Description: Un-initializes the HDR component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int wd_comp_deinit(void *handle)
{
  wd_comp_t *p_comp = (wd_comp_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] \n", __func__, __LINE__);
  status = wd_comp_abort(handle, NULL);
  if (status < 0)
    return status;

  status = p_comp->b.ops.deinit(&p_comp->b);
  if (status < 0)
    return status;

  if (TRUE == p_comp->dspInitFlag) {
#ifdef USE_SMMU_BUFFERS_FOR_WNR
    register_buf(p_comp->mapiondsp.virtual_addr, p_comp->mapiondsp.bufsize, -1);
#endif
    img_free_ion(&p_comp->mapiondsp, 1);
  }
  if (TRUE == p_comp->gpuInitFlag)
    img_free_ion(&p_comp->mapiongpu, 1);

  free(p_comp->p_noiseprof);
  free(p_comp);
  return IMG_SUCCESS;
}

/**
 * Function: wd_alloc_resize_buffers
 *
 * Description: Reallocate ION buffers
 *
 * Input parameters:
 *   p_comp - The pointer to the wavelet component
 *
 * Return values:
 *     IMG_SUCCESS
 **/
static int wd_alloc_resize_buffers(wd_comp_t *p_comp,
  const denoise_alloc_prop_t *p_alloc_prop)
{
  uint32_t ionheapid;
  int rc = IMG_SUCCESS;

  //Allocate ion memory
  //Allocate input segment buffer memory
  if (TRUE == p_alloc_prop->dsp.enabled) {
    p_comp->dspInitFlag = TRUE;

    if (p_comp->mapiondsp.bufsize < p_alloc_prop->dsp.bufferSize) {
      if (p_comp->mapiondsp.bufsize) {
#ifdef USE_SMMU_BUFFERS_FOR_WNR
        register_buf(p_comp->mapiondsp.virtual_addr, p_comp->mapiondsp.bufsize, -1);
#endif
        rc = img_free_ion(&p_comp->mapiondsp, 1);
        if (IMG_SUCCESS != rc)
          return rc;
      }
      p_comp->mapiondsp.bufsize = (uint32_t)p_alloc_prop->dsp.bufferSize;
      if (p_alloc_prop->smmuOnly)
#ifndef TARGET_ION_ABI_VERSION
        ionheapid = ION_HEAP(ION_IOMMU_HEAP_ID);
#else
        ionheapid = ION_HEAP(ION_SYSTEM_HEAP_ID);
#endif
      else
        ionheapid = ION_HEAP(ION_ADSP_HEAP_ID);
      rc = img_alloc_ion(&p_comp->mapiondsp, 1, ionheapid, TRUE);
      if ( (IMG_SUCCESS != rc) || (NULL == p_comp->mapiondsp.virtual_addr) ) {
        IDBG_ERROR("%s:%d] DSP output ION buffer Memory allocation failed",
        __func__, __LINE__);
        p_comp->dspInitFlag = FALSE;
        p_comp->mapiondsp.bufsize = 0;
        return rc;
      } else {
        p_comp->mapiondspheap.virtual_addr =
          (void*)((uint32_t)(uintptr_t)p_comp->mapiondsp.virtual_addr +
          p_alloc_prop->dsp.heapSize);
        p_comp->mapiondspnoiseprofile.virtual_addr =
          (void*)((uint32_t)(uintptr_t)p_comp->mapiondsp.virtual_addr +
          3 * p_alloc_prop->dsp.heapSize);

#ifdef USE_SMMU_BUFFERS_FOR_WNR
          register_buf(p_comp->mapiondsp.virtual_addr, p_comp->mapiondsp.bufsize,
            p_comp->mapiondsp.ion_info_fd.fd);
#endif
      }
    }
  }

  if (TRUE == p_alloc_prop->gpu.enabled) {
    p_comp->gpuInitFlag = TRUE;

    if (p_comp->mapiongpu.bufsize < p_alloc_prop->gpu.bufferSize) {
      if (p_comp->mapiongpu.bufsize) {
        rc = img_free_ion(&p_comp->mapiongpu, 1);
        if (IMG_SUCCESS != rc)
          return rc;
      }
      p_comp->mapiongpu.bufsize = (uint32_t)p_alloc_prop->gpu.bufferSize;
#ifndef TARGET_ION_ABI_VERSION
      ionheapid = ION_HEAP(ION_IOMMU_HEAP_ID);
#else
      ionheapid = ION_HEAP(ION_SYSTEM_HEAP_ID);
#endif
      rc = img_alloc_ion(&p_comp->mapiongpu, 1, ionheapid, TRUE);
      if ( (IMG_SUCCESS != rc) || (NULL == p_comp->mapiongpu.virtual_addr) ) {
        IDBG_ERROR("%s:%d] GPU output ION buffer Memory allocation failed",
          __func__, __LINE__);
        p_comp->gpuInitFlag = FALSE;
        p_comp->mapiongpu.bufsize = 0;
        return rc;
      }
    }
  }

  return rc;
}

/**
 * Function: calc_inverse_gamma_curve
 *
 * Description: Calculate inverse gamma curve
 *
 * Input parameters:
 *   input - The pointer to the input gamma
 *   output - The pointer to the output gamma curve
 *
 * Return values:
 *     NULL
 *
 * Notes: none
 **/
static void __unused calc_inverse_gamma_curve(int16_t *input, int *output)
{
  int32_t i,j;
  int32_t count[257];
  for (i = 0; i < 257; i++) {
    output[i] = 0;
    count[i] = 0;
  }

  for (i = 0; i < GAMMA_TABLE_ENTRIES - 1; i++){
    int slope = input[i+1] - input[i];
    int temp = input[i]<<4;
    for (j=0; j<(1<<4); j++, temp+=slope) {
      int32_t floor_temp = temp>>(q_factor_8bit+4);
      int32_t weight = temp - (floor_temp<<(q_factor_8bit+4));
      output[floor_temp] += ((1<<(q_factor_8bit+4))-weight)*((i<<4)+j);
      output[floor_temp+1] += weight*((i<<4)+j);
      count[floor_temp] += (1<<(q_factor_8bit+4))-weight;
      count[floor_temp+1] += weight;
    }
  }

  if (count[0] > 0) {
    output[0] = (output[0]<<q_factor_10bit)/count[0];
  }

  for (i = 1; i < 257; i++) {
    if (count[i] > 0) {
      output[i] = (output[i]<<q_factor_10bit)/count[i];
    } else {
      output[i] = output[i - 1];
    }
  }
}

/**
 * Function: inverse_gamma_correction
 *
 * Description: Find equivalent point in inverse gamma curve
 *
 * Input parameters:
 *   input - The input point in the curve
 *   output - The pointer to inverse gamma correction curve
 *
 * Return values:
 *     Equivalent point of type double
 *
 * Notes: none
 **/
static double __unused inverse_gamma_correction(double input,
  int *inverse_gamma_correction_curve)
{
  int temp;
  double weight;
  temp = (int)(floor(input));
  weight = input - temp;
  return (inverse_gamma_correction_curve[temp] * (1 - weight) +
    inverse_gamma_correction_curve[temp + 1] * weight) /
    (1 << q_factor_10bit);
}

/**
 * Function: gamma_correction
 *
 * Description: Find equivalent point in gamma curve
 *
 * Input parameters:
 *   input - The input point in the curve
 *   output - The pointer to gamma correction curve
 *
 * Return values:
 *     Equivalent point of type double
 *
 * Notes: none
 **/
static double __unused gamma_correction (double input, int16_t *gamma_correction_curve)
{
  int temp ;
  double weight;
  temp = (int)(floor(input/16));
  weight = (input/16) - temp;
  return (gamma_correction_curve[temp] * (1 - weight) +
    gamma_correction_curve[temp + 1] * weight) /
    (1 << q_factor_8bit);
}

/**
 * Function: interpolate_noise_profile
 *
 * Description: Interpolate the noise profiles
 *
 * Input parameters:
 *   noise_profile - Array of noise profiles
 *   patch_index - the patch index
 *   calibration_level - The calibration level
 *   p_comp - The pointer to the wavelet component
 *   use_lux_idx - use lux idx
 *
 * Return values:
 *     None
 *
 * Notes: none
 **/
static void interpolate_noise_profile(
  ReferenceNoiseProfile_type *noise_profile, int patch_index_start,
  int patch_index_end,
  double calibration_level, wd_comp_t *p_comp, uint8_t use_lux_idx)
{
  int i;
  float Start, End;
#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || defined (CHROMATIX_307) || \
  defined (CHROMATIX_308) || defined (CHROMATIX_309) || defined(CHROMATIX_308E) || \
  defined (CHROMATIX_310) || defined (CHROMATIX_310E)
  if (use_lux_idx) {
    if (patch_index_start < patch_index_end) {
      Start = (float)noise_profile[patch_index_start].WNR_trigger.lux_index_end;
      End = (float)noise_profile[patch_index_end].WNR_trigger.lux_index_start;
    }
    else {
      Start = (float)noise_profile[patch_index_start].WNR_trigger.lux_index_start;
      End = (float)noise_profile[patch_index_end].WNR_trigger.lux_index_end;
    }
  }
  else {
    if (patch_index_start < patch_index_end) {
      Start = noise_profile[patch_index_start].WNR_trigger.gain_end;
      End = noise_profile[patch_index_end].WNR_trigger.gain_start;
    }
    else {
      Start = noise_profile[patch_index_start].WNR_trigger.gain_start;
      End = noise_profile[patch_index_end].WNR_trigger.gain_end;
    }
  }
#else
  Start = noise_profile[patch_index_start].trigger_value;
  End = noise_profile[patch_index_end].trigger_value;
#endif
  float noise_data;

  float interp_ratio;
  float denoise_scale_luma;
  float denoise_scale_chroma;

  if (patch_index_start == patch_index_end)
    interp_ratio = 0.0f;
  else
    interp_ratio = (float)((calibration_level- Start)/(End - Start));

  for (i = 0; i < NUM_NOISE_PROFILE; i++) {
    noise_data = noise_profile[patch_index_start].referenceNoiseProfileData[i] +
    (interp_ratio*(noise_profile[patch_index_end].referenceNoiseProfileData[i] -
    noise_profile[patch_index_start].referenceNoiseProfileData[i]));

    denoise_scale_luma = BILINEAR_INTERPOLATION(
      noise_profile[patch_index_start].denoise_scale_y[i&3],
      noise_profile[patch_index_end].denoise_scale_y[i&3],interp_ratio);

    denoise_scale_chroma =BILINEAR_INTERPOLATION(
      noise_profile[patch_index_start].denoise_scale_chroma[i&3],
      noise_profile[patch_index_end].denoise_scale_chroma[i&3],interp_ratio);

    if ( i <  NUM_LUMA_NOISE_PROFILE ) {
      p_comp->p_noiseprof->referenceNoiseProfileData[i] =
        (uint32_t)Round (noise_data  * (1 << Q20)*denoise_scale_luma);
    } else {
      p_comp->p_noiseprof->referenceNoiseProfileData[i] =
        (uint32_t)Round (noise_data  * (1 << Q20)*denoise_scale_chroma);
    }
  }
}

/**
 * Function: fill_noise_chromatix_params
 *
 * Description: Fills noise chromatix parameters
 *
 * Input parameters:
 *   noise_profile - Array of noise profiles
 *   patch_index - the patch index
 *   calibration_level - The calibration level
 *   p_comp - The pointer to the wavelet component
 *
 * Return values:
 *     None
 *
 * Notes: none
 **/
static void fill_noise_chromatix_params(
  ReferenceNoiseProfile_type *noise_profile, int patch_index_start,
  int patch_index_end, double calibration_level,
  wd_comp_t *p_comp, uint8_t use_lux_idx)
{
  int i;
  float Start, End;
#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || defined (CHROMATIX_307) || \
  defined (CHROMATIX_308) || defined (CHROMATIX_309) || defined(CHROMATIX_308E)|| \
  defined (CHROMATIX_310) || defined (CHROMATIX_310E)
  if (use_lux_idx) {
    if (patch_index_start < patch_index_end) {
      Start = (float)noise_profile[patch_index_start].WNR_trigger.lux_index_end;
      End = (float)noise_profile[patch_index_end].WNR_trigger.lux_index_start;
    }
    else {
      Start = (float)noise_profile[patch_index_start].WNR_trigger.lux_index_start;
      End = (float)noise_profile[patch_index_end].WNR_trigger.lux_index_end;
    }
  }
  else {
    if (patch_index_start < patch_index_end) {
      Start = noise_profile[patch_index_start].WNR_trigger.gain_end;
      End = noise_profile[patch_index_end].WNR_trigger.gain_start;
    }
    else {
      Start = noise_profile[patch_index_start].WNR_trigger.gain_start;
      End = noise_profile[patch_index_end].WNR_trigger.gain_end;
    }
  }
#else
  Start = noise_profile[patch_index_start].trigger_value;
  End = noise_profile[patch_index_end].trigger_value;
#endif
  float interp_ratio ;
  float luma_denoise_weight,chroma_denoise_weight;
  float luma_edge_softness,chroma_edge_softness;

  if (patch_index_start == patch_index_end)
    interp_ratio = 0.0f ;
  else
    interp_ratio = (float)(( calibration_level- Start )/( End - Start )) ;

  p_comp->p_noiseprof->fineTuningFlag = TRUE;

  p_comp->p_noiseprof->lumaEpsilonThreshold =
    (uint32_t)Round(BILINEAR_INTERPOLATION(
    (float)noise_profile[patch_index_start].sw_denoise_edge_threshold_y,
    (float)noise_profile[patch_index_end].sw_denoise_edge_threshold_y,
    interp_ratio));

  p_comp->p_noiseprof->chromaEpsilonThreshold =
    (uint32_t)Round(BILINEAR_INTERPOLATION(
    (float)noise_profile[patch_index_start].sw_denoise_edge_threshold_chroma,
    (float)noise_profile[patch_index_end].sw_denoise_edge_threshold_chroma,
    interp_ratio));

  for (i = 0; i < WAVELET_LEVEL; i++) {
    luma_denoise_weight =
      (float)(BILINEAR_INTERPOLATION(
      noise_profile[patch_index_start].denoise_weight_y[i],
      noise_profile[patch_index_end].denoise_weight_y[i], interp_ratio));
    chroma_denoise_weight =
      (float)(BILINEAR_INTERPOLATION(
      noise_profile[patch_index_start].denoise_weight_chroma[i],
      noise_profile[patch_index_end].denoise_weight_chroma[i], interp_ratio));
    luma_edge_softness =
     (float)(BILINEAR_INTERPOLATION(
      noise_profile[patch_index_start].denoise_edge_softness_y[i],
      noise_profile[patch_index_end].denoise_edge_softness_y[i],
      interp_ratio));
    chroma_edge_softness =
      (float)(BILINEAR_INTERPOLATION(
      noise_profile[patch_index_start].denoise_edge_softness_chroma[i],
      noise_profile[patch_index_end].denoise_edge_softness_chroma[i],
      interp_ratio));

    p_comp->p_noiseprof->lumaEdgeWeight[i] =
      (uint32_t)Round(luma_denoise_weight*(1<<15));
    p_comp->p_noiseprof->lumaEdgeLimitFactor[i] =
      (uint32_t)Round(2*luma_edge_softness);
    p_comp->p_noiseprof->chromaEdgeWeight[i] =
      (uint32_t)Round(chroma_denoise_weight*(1<<15));
    p_comp->p_noiseprof->chromaEdgeLimitFactor[i] =
      (uint32_t)Round(2*chroma_edge_softness);
  }
}

/**
 * Function: wd_comp_calibrate
 *
 * Description: Calibrate the wavelet denoise parameters
 *
 * Input parameters:
 *   p_comp - The pointer to the wavelet component
 *
 * Return values:
 *     None
 *
 * Notes: IMG_SUCCESS
 **/
static int wd_comp_calibrate(wd_comp_t *p_comp)
{
#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || defined (CHROMATIX_307) || \
  defined (CHROMATIX_308) || defined (CHROMATIX_309) || defined(CHROMATIX_308E)|| \
  defined (CHROMATIX_310) || defined (CHROMATIX_310E)
  chromatix_cpp_type *p_cpp_chromatix = p_comp->p_cpp_chromatix;
#else
  chromatix_parms_type *p_chromatix = p_comp->p_chromatix;
#endif

  /* AWB green gain * digital gain * Analog gain */
  double d_new = (p_comp->info_3a.wb_g_gain *
    p_comp->info_3a.aec_real_gain);
  double gain;
  double calibration_level;
  ReferenceNoiseProfile_type* noise_profile;
  uint8_t use_lux_idx;

  noise_profile =
#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || defined (CHROMATIX_307) || \
  defined (CHROMATIX_308) || defined (CHROMATIX_309) || defined(CHROMATIX_308E)|| \
  defined (CHROMATIX_310) || defined (CHROMATIX_310E)
    &p_cpp_chromatix->chromatix_wavelet.wavelet_denoise_SW_420.noise_profile[0];
#else
    &p_chromatix->chromatix_VFE.chromatix_wavelet.wavelet_denoise_SW_420.noise_profile[0];
#endif
  IDBG_HIGH("%s:%d] new_gain %f ", __func__, __LINE__, d_new);

  /* if control_denoise is 0 , use lux_index to interpolate noise data */
#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || defined (CHROMATIX_307) || \
  defined (CHROMATIX_308) || defined (CHROMATIX_309) || defined(CHROMATIX_308E)|| \
  defined (CHROMATIX_310) || defined (CHROMATIX_310E)
  if (p_cpp_chromatix->chromatix_wavelet.wavelet_denoise_SW_420.control_denoise == 0) {
#else
  if (p_chromatix->chromatix_VFE.chromatix_wavelet.wavelet_denoise_SW_420.control_denoise == 0) {
#endif
    calibration_level = p_comp->info_3a.lux_idx;
    gain = 1.0;
    use_lux_idx = TRUE;
  } else {
    calibration_level = p_comp->info_3a.aec_real_gain;
    gain = 1.0;
    use_lux_idx = FALSE;
  }

#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || defined (CHROMATIX_307) || \
  defined (CHROMATIX_308) || defined (CHROMATIX_309) || defined(CHROMATIX_308E)|| \
  defined (CHROMATIX_310) || defined (CHROMATIX_310E)
  IDBG_HIGH("%s:%d] WNR trigger mode= %d, calibration_level = %f", __func__, __LINE__,
    p_cpp_chromatix->chromatix_wavelet.wavelet_denoise_SW_420.control_denoise,
    calibration_level);
#else
  IDBG_HIGH("%s:%d] WNR trigger mode= %d, calibration_level = %f", __func__, __LINE__,
    p_chromatix->chromatix_VFE.chromatix_wavelet.wavelet_denoise_SW_420.control_denoise,
    calibration_level);
#endif

  /*  Calculate avg of noise patch average */
  int binStart = 0;
  int binEnd = 0;

#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || defined (CHROMATIX_307) || \
  defined (CHROMATIX_308) || defined (CHROMATIX_309) || defined(CHROMATIX_308E)|| \
  defined (CHROMATIX_310) || defined (CHROMATIX_310E)
  if (use_lux_idx) {
      if (calibration_level <= noise_profile[0].WNR_trigger.lux_index_end) {
        binStart  = 0;
        binEnd    = 0;
      } else if (calibration_level <= noise_profile[1].WNR_trigger.lux_index_start
        && calibration_level > noise_profile[0].WNR_trigger.lux_index_end) {
        binStart  = 0;
        binEnd    = 1;
      } else if (calibration_level <= noise_profile[1].WNR_trigger.lux_index_end
        && calibration_level > noise_profile[1].WNR_trigger.lux_index_start) {
        binStart  = 1;
        binEnd    = 1;
      } else if (calibration_level <= noise_profile[2].WNR_trigger.lux_index_start
        && calibration_level > noise_profile[1].WNR_trigger.lux_index_end) {
        binStart  = 1;
        binEnd    = 2;
      } else if (calibration_level <= noise_profile[2].WNR_trigger.lux_index_end
        && calibration_level > noise_profile[2].WNR_trigger.lux_index_start) {
        binStart  = 2;
        binEnd    = 2;
      } else if (calibration_level <= noise_profile[3].WNR_trigger.lux_index_start
        && calibration_level > noise_profile[2].WNR_trigger.lux_index_end) {
        binStart  = 2;
        binEnd    = 3;
      } else if (calibration_level <= noise_profile[3].WNR_trigger.lux_index_end
        && calibration_level > noise_profile[3].WNR_trigger.lux_index_start) {
        binStart  = 3;
        binEnd    = 3;
      } else if (calibration_level <= noise_profile[4].WNR_trigger.lux_index_start
        && calibration_level > noise_profile[3].WNR_trigger.lux_index_end) {
        binStart  = 3;
        binEnd    = 4;
      } else if (calibration_level <= noise_profile[4].WNR_trigger.lux_index_end
        && calibration_level > noise_profile[4].WNR_trigger.lux_index_start) {
        binStart  = 4;
        binEnd    = 4;
      } else if (calibration_level <= noise_profile[5].WNR_trigger.lux_index_start
        && calibration_level > noise_profile[4].WNR_trigger.lux_index_end) {
        binStart  = 4;
        binEnd    = 5;
      } else if (calibration_level > noise_profile[5].WNR_trigger.lux_index_start) {
        binStart  = 5;
        binEnd    = 5;
      }
  }
  else {
      if (calibration_level <= noise_profile[0].WNR_trigger.gain_end) {
        binStart  = 0;
        binEnd    = 0;
      } else if (calibration_level <= noise_profile[1].WNR_trigger.gain_start
        && calibration_level > noise_profile[0].WNR_trigger.gain_end) {
        binStart  = 0;
        binEnd    = 1;
      } else if (calibration_level <= noise_profile[1].WNR_trigger.gain_end
        && calibration_level > noise_profile[1].WNR_trigger.gain_start) {
        binStart  = 1;
        binEnd    = 1;
      } else if (calibration_level <= noise_profile[2].WNR_trigger.gain_start
        && calibration_level > noise_profile[1].WNR_trigger.gain_end) {
        binStart  = 1;
        binEnd    = 2;
      } else if (calibration_level <= noise_profile[2].WNR_trigger.gain_end
        && calibration_level > noise_profile[2].WNR_trigger.gain_start) {
        binStart  = 2;
        binEnd    = 2;
      } else if (calibration_level <= noise_profile[3].WNR_trigger.gain_start
        && calibration_level > noise_profile[2].WNR_trigger.gain_end) {
        binStart  = 2;
        binEnd    = 3;
      } else if (calibration_level <= noise_profile[3].WNR_trigger.gain_end
        && calibration_level > noise_profile[3].WNR_trigger.gain_start) {
        binStart  = 3;
        binEnd    = 3;
      } else if (calibration_level <= noise_profile[4].WNR_trigger.gain_start
        && calibration_level > noise_profile[3].WNR_trigger.gain_end) {
        binStart  = 3;
        binEnd    = 4;
      } else if (calibration_level <= noise_profile[4].WNR_trigger.gain_end
        && calibration_level > noise_profile[4].WNR_trigger.gain_start) {
        binStart  = 4;
        binEnd    = 4;
      } else if (calibration_level <= noise_profile[5].WNR_trigger.gain_start
        && calibration_level > noise_profile[4].WNR_trigger.gain_end) {
        binStart  = 4;
        binEnd    = 5;
      } else if (calibration_level > noise_profile[5].WNR_trigger.gain_start) {
        binStart  = 5;
        binEnd    = 5;
      }
  }
#else
  if (calibration_level <= noise_profile[0].trigger_value) {
    binStart  = 0;
    binEnd    = 0;
  } else if (calibration_level <= noise_profile[1].trigger_value
    && calibration_level > noise_profile[0].trigger_value) {
    binStart  = 0;
    binEnd    = 1;
  } else if (calibration_level <=noise_profile[2].trigger_value
    && calibration_level > noise_profile[1].trigger_value) {
    binStart  = 1;
    binEnd    = 2;
  } else if (calibration_level <= noise_profile[3].trigger_value
    && calibration_level > noise_profile[2].trigger_value) {
    binStart  = 2;
    binEnd    = 3;
  } else if (calibration_level <= noise_profile[4].trigger_value
    && calibration_level > noise_profile[3].trigger_value) {
    binStart  = 3;
    binEnd    = 4;
  }
  else if (calibration_level <= noise_profile[5].trigger_value
    && calibration_level > noise_profile[4].trigger_value) {
    binStart  = 4;
    binEnd    = 5;
  } else {
    binStart  = 5;
    binEnd    = 5;
  }
#endif

  IDBG_HIGH("%s:%d] binStart = %d, binEnd = %d ", __func__, __LINE__, binStart,
    binEnd);

  interpolate_noise_profile(noise_profile, binStart, binEnd, calibration_level,
    p_comp, use_lux_idx);

  fill_noise_chromatix_params(noise_profile, binStart,binEnd, calibration_level,
    p_comp, use_lux_idx);

  IDBG_MED("%s:%d]:X", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: wd_comp_buffers_realloc
 *
 * Description: Reallocate library buffers if needed
 *
 * Input parameters:
 *   p_comp - The pointer to the component handle.
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int wd_comp_buffers_realloc(wd_comp_t *p_comp,
  wd_buffers_realloc_info_t *p_info)
{
  int rc = IMG_SUCCESS;
  denoise_alloc_prop_t allocProp;
  memset(&allocProp, 0x0, sizeof(denoise_alloc_prop_t));

  /*Dynamically set segment line size depending on image height */
  uint32_t size = p_info->height * p_info->width;

  if (size <= SZ_2M)
    p_comp->denoiseCtrl.segmentLineSize = p_info->height;
  else if (size <= SZ_4M)
    p_comp->denoiseCtrl.segmentLineSize = ((p_info->height / 2 + 16) >> 4) << 4;
  else if (size <= SZ_8M)
    p_comp->denoiseCtrl.segmentLineSize = ((p_info->height / 4 + 16) >> 4) << 4;
  else if (size <= SZ_11M)
    p_comp->denoiseCtrl.segmentLineSize = ((p_info->height / 6 + 16) >> 4) << 4;
  else
    p_comp->denoiseCtrl.segmentLineSize = ((p_info->height / 8 + 16) >> 4) << 4;

  switch (p_info->mode) {
  case WD_MODE_YCBCR_PLANE:
    p_comp->y_complexity = 1;
    p_comp->cbcr_complexity = 1;
    memcpy(&p_comp->denoiseCtrl.core_aff, g_yc_core_aff_full, sizeof(g_yc_core_aff_full));
    break;
  case WD_MODE_CBCR_ONLY:
    p_comp->y_complexity = 0;
    p_comp->cbcr_complexity = 1;
    memcpy(&p_comp->denoiseCtrl.core_aff, g_yc_core_aff_chroma, sizeof(g_yc_core_aff_chroma));
    break;
  case WD_MODE_STREAMLINE_YCBCR:
    p_comp->y_complexity = 0;
    p_comp->cbcr_complexity = 0;
    memcpy(&p_comp->denoiseCtrl.core_aff, g_yc_core_aff_full, sizeof(g_yc_core_aff_full));
    break;
  case WD_MODE_STREAMLINED_CBCR:
    p_comp->y_complexity = 0;
    p_comp->cbcr_complexity = 0;
    memcpy(&p_comp->denoiseCtrl.core_aff, g_yc_core_aff_chroma, sizeof(g_yc_core_aff_chroma));
    break;
  default:
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  //Populate denoise object to be passed into the lib
  p_comp->denoiseCtrl.Y_width = p_info->width;
  p_comp->denoiseCtrl.Y_height = p_info->height;
  p_comp->denoiseCtrl.lines_to_pad = p_comp->lines_to_pad;
  p_comp->denoiseCtrl.y_complexity = p_comp->y_complexity;
  p_comp->denoiseCtrl.cbcr_complexity = p_comp->cbcr_complexity;

  if (!g_wd_lib.camera_denoising_get_alloc_prop(&p_comp->denoiseCtrl,
    &allocProp)) {
      IDBG_ERROR("%s:%d] g_wd_lib.camera_denoising_get_alloc_prop (%d)",
        __func__, __LINE__, rc);
      return IMG_ERR_GENERAL;
  }

  /* Check and reallocate ION buffers */
  rc = wd_alloc_resize_buffers(p_comp, &allocProp);
  if (rc) {
    IDBG_ERROR("%s:%d] WAVELET DENOISING failed memory reallocation %d !!! ",
      __func__, __LINE__, rc);
  }

  return rc;
}

/**
 * Function: wd_comp_set_param
 *
 * Description: Set Wavelet parameters
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int wd_comp_set_param(void *handle, img_param_type param, void *p_data)
{
  wd_comp_t *p_comp = (wd_comp_t *)handle;
  int status = IMG_SUCCESS;

  status = p_comp->b.ops.set_parm(&p_comp->b, param, p_data);
  if (status < 0)
    return status;

  IDBG_MED("%s:%d] param 0x%x", __func__, __LINE__, param);
  switch (param) {
  case QWD_GAMMA_TABLE: {
    img_gamma_t *p_gamma = (img_gamma_t *)p_data;

    if (NULL == p_gamma) {
      IDBG_ERROR("%s:%d] invalid gamma table", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->current_gamma = *p_gamma;
    p_comp->gamma_set = TRUE;
  }
    break;
  case QWD_LOW_GAMMA_TABLE: {
    img_gamma_t *p_gamma = (img_gamma_t *)p_data;

    if (NULL == p_gamma) {
      IDBG_ERROR("%s:%d] invalid low gamma table", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->lowlight_gamma = *p_gamma;
    p_comp->lowlight_gamma_set = TRUE;
  }
    break;
  case QWD_CHROMATIX: {
    chromatix_parms_type *p_chromatix = (chromatix_parms_type *)p_data;
    if (NULL == p_chromatix) {
      IDBG_ERROR("%s:%d] invalid chromatix", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->p_chromatix = p_chromatix;
  }
    break;
 #if defined (CHROMATIX_304) || defined (CHROMATIX_306) || defined (CHROMATIX_307) || \
  defined (CHROMATIX_308) || defined (CHROMATIX_309) || defined(CHROMATIX_308E)|| \
  defined (CHROMATIX_310) || defined (CHROMATIX_310E)
  case QWD_CPP_CHROMATIX: {
    chromatix_cpp_type *p_cpp_chromatix = (chromatix_cpp_type *)p_data;
    if (NULL == p_cpp_chromatix) {
      IDBG_ERROR("%s:%d] invalid cpp_chromatix", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->p_cpp_chromatix = p_cpp_chromatix;
  }
    break;
#endif
  case QWD_3A_INFO: {
    wd_3a_info_t *p_3a_info = (wd_3a_info_t *)p_data;

    if (NULL == p_3a_info) {
      IDBG_ERROR("%s:%d] invalid gamma table", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->info_3a = *p_3a_info;
  }
    break;
  case QWD_MODE: {
    if (!p_data) {
      IDBG_ERROR("%s:%d] invalid denoise mode", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }

    p_comp->mode = *((wd_mode_t *)p_data);
  }
    break;
  case QWD_EARLY_CB: {
   if (!p_data) {
      IDBG_ERROR("%s:%d] invalid value for early cb", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }
    p_comp->early_cb_enabled = *((uint8_t *)p_data);
    IDBG_LOW("%s:%d] p_comp->early_cb_enabled %d", __func__, __LINE__, p_comp->early_cb_enabled);
  }
    break;
  case QIMG_CAMERA_DUMP:
    break;
  case QWD_BUFFERS_REALLOC:{
    if (!p_data) {
      IDBG_ERROR("%s:%d] invalid realloc data", __func__, __LINE__);
      return IMG_ERR_INVALID_INPUT;
    }

    status = wd_comp_buffers_realloc(p_comp, p_data);
  }
    break;
  default:
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  return status;
}

/**
 * Function: wd_comp_get_param
 *
 * Description: Gets Wavelet denoise parameters
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int wd_comp_get_param(void *handle, img_param_type param, void *p_data)
{
  wd_comp_t *p_comp = (wd_comp_t *)handle;
  int status = IMG_SUCCESS;

  status = p_comp->b.ops.get_parm(&p_comp->b, param, p_data);
  if (status < 0)
    return status;

  switch (param) {
  default:
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}


/**
 * Function: wd_map_mode
 *
 * Description: Maps from wd_mode_t to denois_process_plane_t
 *
 * Input parameters:
 *   mode - wavelet mode
 *
 * Return values:
 *     denois_process_plane_t type
 *
 * Notes: none
 **/
static denois_process_plane_t wd_map_mode(wd_mode_t mode)
{
  switch (mode) {
  case WD_MODE_CBCR_ONLY:
    return DENOISE_CBCR_ONLY;
  case WD_MODE_STREAMLINE_YCBCR:
    return DENOISE_STREAMLINE_YCBCR;
  case WD_MODE_STREAMLINED_CBCR:
    return DENOISE_STREAMLINED_CBCR;
  default:
  case WD_MODE_YCBCR_PLANE:
    return DENOISE_YCBCR_PLANE;
  }
}
/**
 * Function: wd_comp_lib_debug
 *
 * Description: Debug params for wavelet library
 *
 * Input parameters:
 *   p_denoiselib - library instance pointer
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static void wd_comp_lib_debug(denoise_lib_t *p_denoiselib, wd_comp_t *p_comp)
{
  IDBG_MED("%s:%d] WaveletLIB Y_width %d", __func__, __LINE__,
    p_denoiselib->Y_width);
  IDBG_MED("%s:%d] WaveletLIB Y_height %d", __func__, __LINE__,
    p_denoiselib->Y_height);
  IDBG_MED("%s:%d] WaveletLIB Chroma_width %d", __func__, __LINE__,
    p_denoiselib->Chroma_width);
  IDBG_MED("%s:%d] WaveletLIB Chroma_height %d", __func__, __LINE__,
    p_denoiselib->Chroma_height);
  IDBG_MED("%s:%d] WaveletLIB lines_to_pad %d", __func__, __LINE__,
    p_denoiselib->lines_to_pad);
  IDBG_MED("%s:%d] WaveletLIB process_planes %d", __func__, __LINE__,
    p_denoiselib->process_planes);
  IDBG_MED("%s:%d] WaveletLIB y_complexity %d", __func__, __LINE__,
    p_denoiselib->y_complexity);
  IDBG_MED("%s:%d] WaveletLIB cbcr_complexity %d", __func__, __LINE__,
    p_denoiselib->cbcr_complexity);
  IDBG_MED("%s:%d] WaveletLIB plumaplane %p", __func__, __LINE__,
    p_denoiselib->plumaplane);
  IDBG_MED("%s:%d] WaveletLIB pchromaplane1 %p", __func__, __LINE__,
    p_denoiselib->pchromaplane1);
  IDBG_MED("%s:%d] WaveletLIB pchromaplane2 %p", __func__, __LINE__,
    p_denoiselib->pchromaplane2);
  IDBG_MED("%s:%d] WaveletLIB denoiseplane %d", __func__, __LINE__,
    p_denoiselib->denoiseplane);
  IDBG_MED("%s:%d] WaveletLIB state %d", __func__, __LINE__,
    p_denoiselib->state);
  IDBG_MED("%s:%d] WaveletLIB segmentLineSize %d", __func__, __LINE__,
    p_denoiselib->segmentLineSize);

  if (p_comp->b.debug_info.camera_dump_enabled) {
    int i=0;
    char filename[FILENAME_MAX];
    char fbuf[1024];
    char *s = QCAMERA_DUMP_FRM_LOCATION"WNRInputParams";
    snprintf(filename, sizeof(filename), "%s_%s.txt", s,
      p_comp->b.debug_info.timestamp);
    size_t offset=0;
    offset += (size_t)snprintf(fbuf, sizeof(fbuf) - offset, "Y_width= %d\n",
      p_denoiselib->Y_width);
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "Y_height= %d\n",
      p_denoiselib->Y_height);
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "Chroma_width= %d\n",
      p_denoiselib->Chroma_width);
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset,
      "Chroma_height= %d\n",
      p_denoiselib->Chroma_height);
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "lines_to_pad= %d\n",
      p_denoiselib->lines_to_pad);
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "y_complexity= %d\n",
      p_denoiselib->y_complexity);
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset,
      "cbcr_complexity= %d\n",
      p_denoiselib->cbcr_complexity);
#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || defined (CHROMATIX_307) || \
  defined (CHROMATIX_308) || defined (CHROMATIX_309) || defined(CHROMATIX_308E)|| \
  defined (CHROMATIX_310) || defined (CHROMATIX_310E)
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset,
      "WNR trigger type(0-lux, 1-gain)= %d\n", p_comp->p_cpp_chromatix->
      chromatix_wavelet.wavelet_denoise_SW_420.control_denoise);
#else
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset,
      "WNR trigger type(0-lux, 1-gain)= %d\n", p_comp->p_chromatix->
      chromatix_VFE.chromatix_wavelet.wavelet_denoise_SW_420.control_denoise);
#endif
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "lux_idx= %f\n",
      p_comp->info_3a.lux_idx);
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "gain= %f\n",
      p_comp->info_3a.aec_real_gain);
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset,
      "referencePatchAverageValue= %d\n",
      p_denoiselib->noiseProfileData_Q20->referencePatchAverageValue);
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset,
      "fineTuningFlag= %d\n",
      p_denoiselib->noiseProfileData_Q20->fineTuningFlag);
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset,
      "lumaEpsilonThreshold= %d\n",
      p_denoiselib->noiseProfileData_Q20->lumaEpsilonThreshold);
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset,
      "chromaEpsilonThreshold= %d\n",
      p_denoiselib->noiseProfileData_Q20->chromaEpsilonThreshold);

    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset,
      "referenceNoiseProfileData= ");
    for (i = 0; i<NUM_NOISE_PROFILE_DATA; i++) {
      offset+=(size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "%d\n",
        p_denoiselib->noiseProfileData_Q20->referenceNoiseProfileData[i]);
    }
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "\n");
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "lumaEdgeWeight= ");

    for (i = 0; i < WAVELET_DENOISING_DATA_SIZE_BY_6; i++) {
      offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, " %d",
        p_denoiselib->noiseProfileData_Q20->lumaEdgeWeight[i]);
    }

    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "\n");
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset,
      "lumaEdgeLimitFactor= ");

    for (i = 0; i < WAVELET_DENOISING_DATA_SIZE_BY_6; i++) {
      offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "%d",
        p_denoiselib->noiseProfileData_Q20->lumaEdgeLimitFactor[i]);
    }

    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "\n");
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset,
      "chromaEdgeWeight= ");

    for (i = 0; i < WAVELET_DENOISING_DATA_SIZE_BY_6; i++) {
      offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "%d",
        p_denoiselib->noiseProfileData_Q20->chromaEdgeWeight[i]);
    }

    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "\n");
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset,
      "chromaEdgeLimitFactor= ");
    for (i = 0; i < WAVELET_DENOISING_DATA_SIZE_BY_6; i++) {
      offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "%d",
        p_denoiselib->noiseProfileData_Q20->chromaEdgeLimitFactor[i]);
    }
    offset += (size_t)snprintf(fbuf+offset, sizeof(fbuf) - offset, "\n");
    IDBG_INFO("%s:%d] offset= %zu", __func__, __LINE__, offset);
    IMG_DUMP_TO_FILE(filename, fbuf, offset);
 }
}

/**
 * Function: wd_comp_invalidate_buffer
 *
 * Description: Invalidate the buffer
 *
 * Input parameters:
 *   p_comp - The pointer to the component handle.
 *   p_frame - Frame which needs to be processed
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int wd_comp_invalidate_buffer(wd_comp_t *p_comp, img_frame_t *p_frame)
{
   int rc = IMG_SUCCESS;

   size_t buffer_size = 0;
   int ion_device_fd = -1;
   uint8_t *v_addr = NULL;
   int fd = -1;
   size_t luma_size = 0;
   uint32_t chroma_size = 0, chroma_height = 0;

  if ((NULL == p_comp) || (NULL == p_frame)) {
    IDBG_ERROR("%s: %d: Invalid input params", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  v_addr = p_frame->frame[0].plane[IY].addr;
  fd = p_frame->frame[0].plane[IY].fd;

  //Assuming semiplanar
  luma_size = (size_t)(p_frame->frame[0].plane[IY].stride *
    p_frame->frame[0].plane[IY].scanline);

  switch (p_frame->info.ss){
    case IMG_H2V2:
    case IMG_H1V2:
      chroma_height = (uint32_t)(p_frame->frame[0].plane[IY].height /2);
      break;
    case IMG_H2V1:
    case IMG_H1V1:
      chroma_height = (uint32_t)(p_frame->frame[0].plane[IY].height);
      break;
  }
  chroma_size = (uint32_t)p_frame->frame[0].plane[IY].stride * chroma_height;
  buffer_size = luma_size + chroma_size;

  IDBG_HIGH("%s: %d:] buffer_size %zu", __func__, __LINE__, buffer_size);

  if (p_comp->mapiondsp.ion_fd > 0) {
    ion_device_fd = p_comp->mapiondsp.ion_fd;
  } else if (p_comp->mapiongpu.ion_fd > 0) {
     ion_device_fd = p_comp->mapiongpu.ion_fd;
  }

  rc = img_cache_ops_external(v_addr, buffer_size, 0, fd,
    IMG_CACHE_CLEAN_INV, ion_device_fd);
  if (rc) {
    IDBG_ERROR("%s: %d:] Cache Invalidation Failed", __func__, __LINE__ );
  } else {
    IDBG_HIGH("%s: %d:] Cache Invalidation Success", __func__, __LINE__);
  }
  return IMG_SUCCESS;
}

/**
  Function: wd_comp_early_cb
 *
 * Description: If early callback is enabled, post event to
 * the client
 *
 * Input parameters:
 *   handle - Client handle
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int wd_comp_early_cb(void *handle)
{
  wd_comp_t *p_comp = (wd_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  IDBG_HIGH("%s:%d] p_comp %p", __func__, __LINE__, p_comp);
  IMG_SEND_EVENT(p_base, QIMG_EVT_EARLY_CB_DONE);
  return IMG_SUCCESS;
}

/**
 * Function: wd_comp_process_frame
 *
 * Description: Run the denoise algorithm on the given frame
 *
 * Input parameters:
 *   p_comp - The pointer to the component handle.
 *   p_frame - Frame which needs to be processed
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int wd_comp_process_frame(wd_comp_t *p_comp, img_frame_t *p_frame)
{
  int rc = IMG_SUCCESS;
  img_component_t *p_base = &p_comp->b;
#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || defined (CHROMATIX_307)  || \
  defined (CHROMATIX_308) || defined (CHROMATIX_309) || defined(CHROMATIX_308E)|| \
  defined (CHROMATIX_310) || defined (CHROMATIX_310E)
  chromatix_cpp_type *p_cpp_chromatix = p_comp->p_cpp_chromatix;
#else
  chromatix_parms_type *p_chromatix = p_comp->p_chromatix;
#endif
  wd_buffers_realloc_info_t info;
  img_perf_t img_perf = {
    img_perf_handle_create,
    img_perf_handle_destroy,
    img_perf_lock_start,
    img_perf_lock_end,
  };

  rc = img_image_stride_fill(p_frame);
  if (rc < 0) {
    IDBG_ERROR("%s:%d] image stride fill error %d",
      __func__, __LINE__, rc);
    return rc;
  }

  pthread_mutex_lock(&p_base->mutex);
#if defined (CHROMATIX_304) || defined (CHROMATIX_306) || defined (CHROMATIX_307) || \
  defined (CHROMATIX_308) || defined (CHROMATIX_309) || defined(CHROMATIX_308E)|| \
  defined (CHROMATIX_310) || defined (CHROMATIX_310E)
  if (p_comp->info_3a.lux_idx <
    p_cpp_chromatix->chromatix_wavelet.wavelet_denoise_SW_420.
      wavelet_enable_index) {
    IDBG_INFO("%s:%d] WNR not triggered as Lux Index %f is less than %d ",
      __func__, __LINE__, p_comp->info_3a.lux_idx,
      p_cpp_chromatix->chromatix_wavelet.wavelet_denoise_SW_420.
      wavelet_enable_index);
#else
    if (p_comp->info_3a.lux_idx <
      p_chromatix->chromatix_VFE.chromatix_wavelet.wavelet_denoise_SW_420.
        wavelet_enable_index) {
      IDBG_INFO("%s:%d] WNR not triggered as Lux Index %f is less than %d ",
        __func__, __LINE__, p_comp->info_3a.lux_idx,
        p_chromatix->chromatix_VFE.chromatix_wavelet.wavelet_denoise_SW_420.
        wavelet_enable_index);
#endif
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_SUCCESS;
  }

  IDBG_MED("%s:%d] calling denoising manager\n", __func__, __LINE__);

  info.height = (uint32_t)p_frame->info.height;
  info.width = (uint32_t)p_frame->info.width;
  info.mode = p_comp->mode;

  /*Added check for min width/height requirement
  Also width,height not multiple of 4 is not supported */
  if (p_frame->info.width <= 320 || p_frame->info.height <= 320) {
    IDBG_ERROR("%s:%d] resolution is too low does not need wavelet denoising",
      __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_SUCCESS;
  }

  if (((p_frame->info.width % 4) != 0) ||
    ((p_frame->info.height % 4) != 0)) {
    IDBG_ERROR("%s:%d] Resolution not multiple of 4 cannot support"
      " wavelet denoising", __func__, __LINE__);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_SUCCESS;
  }

  rc = wd_comp_buffers_realloc(p_comp, &info);
  if (rc) {
    IDBG_ERROR("%s:%d] WAVELET DENOISING failed memory reallocation %d !!!",
      __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_base->mutex);
    return rc;
  }

  p_comp->denoiseCtrl.Chroma_width = (uint32_t)p_frame->frame[0].plane[IC].stride;
  p_comp->denoiseCtrl.Chroma_height = (uint32_t)p_frame->frame[0].plane[IC].height;
  p_comp->denoiseCtrl.state = DENOISE;
  p_comp->denoiseCtrl.noiseProfileData_Q20 = p_comp->p_noiseprof;
  p_comp->denoiseCtrl.process_planes = wd_map_mode(p_comp->mode);

  p_comp->denoiseCtrl.plumaplane = (uint8_t *)p_frame->frame[0].plane[IY].addr;
  if (p_frame->frame[0].plane_cnt != 3) {
    p_comp->denoiseCtrl.pchromaplane1 = (uint8_t *)p_frame->frame[0].plane[IC].addr;
    p_comp->denoiseCtrl.denoiseplane = DENOISE_SEMI_PLANAR;
  } else {
    p_comp->denoiseCtrl.pchromaplane1 = (uint8_t *)p_frame->frame[0].plane[IC1].addr;
    p_comp->denoiseCtrl.pchromaplane2 = (uint8_t *)p_frame->frame[0].plane[IC2].addr;
    p_comp->denoiseCtrl.denoiseplane = DENOISE_PLANAR;
  }
  pthread_mutex_unlock(&p_base->mutex);

  /*debug params */
  wd_comp_lib_debug(&p_comp->denoiseCtrl, p_comp);

  p_comp->denoiseCtrl.ion.useDSP = p_comp->dspInitFlag;
  if (p_comp->denoiseCtrl.ion.useDSP) {
    p_comp->denoiseCtrl.ion.dspBuf.vAddr = (uint8_t *) p_comp->mapiondsp.virtual_addr;
    p_comp->denoiseCtrl.ion.dspBuf.size = p_comp->mapiondsp.bufsize;
    p_comp->denoiseCtrl.ion.dspheapBuf.vAddr =
      (uint8_t *) p_comp->mapiondspheap.virtual_addr;
    p_comp->denoiseCtrl.ion.dspheapBuf.size = p_comp->mapiondspheap.bufsize;
    p_comp->denoiseCtrl.ion.dspnoiseprofileBuf.vAddr =
      (uint8_t *) p_comp->mapiondspnoiseprofile.virtual_addr;
    p_comp->denoiseCtrl.ion.dspnoiseprofileBuf.size =
      p_comp->mapiondspnoiseprofile.bufsize;
  }
  p_comp->denoiseCtrl.ion.useGPU = p_comp->gpuInitFlag;
  if (p_comp->denoiseCtrl.ion.useGPU) {
    p_comp->denoiseCtrl.ion.gpuBuf.vAddr = (uint8_t *) p_comp->mapiongpu.virtual_addr;
    p_comp->denoiseCtrl.ion.gpuBuf.size = p_comp->mapiongpu.bufsize;
  }
  p_comp->denoiseCtrl.early_cb_enabled = p_comp->early_cb_enabled;
  p_comp->denoiseCtrl.notify_early_cb = wd_comp_early_cb;
  p_comp->denoiseCtrl.app_data = (void*)p_comp;
  p_comp->denoiseCtrl.p_img_perf = &img_perf;
  IDBG_MED("%s:%d] denoiseCtrl.early_cb_enabled %d", __func__, __LINE__,
    p_comp->denoiseCtrl.early_cb_enabled);
  if (!g_wd_lib.camera_denoising_manager(&p_comp->denoiseCtrl)) {
    IDBG_ERROR("%s:%d] WAVELET DENOISING failed !!! ", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }
  p_comp->denoiseCtrl.state = IDLE;
    //Invalidate Buffer
  wd_comp_invalidate_buffer(p_comp, p_frame);

  IDBG_HIGH("%s:%d] Wavelet Denoise Success", __func__, __LINE__);
  return rc;
}

/**
 * Function: wd_thread_loop
 *
 * Description: Main algorithm thread loop
 *
 * Input parameters:
 *   data - The pointer to the component object
 *
 * Return values:
 *     NULL
 *
 * Notes: none
 **/
void *wd_thread_loop(void *handle)
{
  wd_comp_t *p_comp = (wd_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_frame_t *p_frame = NULL;
  int i = 0, count;
  IDBG_MED("%s:%d] ", __func__, __LINE__);

  count = img_q_count(&p_base->inputQ);
  IDBG_MED("%s:%d] num buffers %d", __func__, __LINE__,
    count);

  for (i = 0; i < count; i++) {
    p_frame = img_q_dequeue(&p_base->inputQ);
    if (NULL == p_frame) {
      IDBG_ERROR("%s:%d] invalid buffer", __func__, __LINE__);
      goto error;
    }

    status = wd_comp_process_frame(p_comp, p_frame);
    if (status < 0) {
      IDBG_ERROR("%s:%d] process error %d", __func__, __LINE__, status);
      goto error;
    }
    IMG_CHK_ABORT_RET_LOCKED(p_base, &p_base->mutex);

    IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
  }

  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_STOPPED;
  pthread_mutex_unlock(&p_base->mutex);
  IMG_SEND_EVENT(p_base, QIMG_EVT_DONE);
  return IMG_SUCCESS;

error:
  /* flush rest of the buffers */
  count = img_q_count(&p_base->inputQ);
  IDBG_MED("%s:%d] Error buf count %d", __func__, __LINE__,
    count);

  for (i = 0; i < count; i++) {
    p_frame = img_q_dequeue(&p_base->inputQ);
    if (NULL == p_frame) {
      IDBG_ERROR("%s:%d] invalid buffer", __func__, __LINE__);
      continue;
    }
    status = img_q_enqueue(&p_base->outputQ, p_frame);
    if (status < 0) {
      IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
      continue;
    }
    IMG_SEND_EVENT(p_base, QIMG_EVT_BUF_DONE);
  }
  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_STOPPED;
  pthread_mutex_unlock(&p_base->mutex);
  IMG_SEND_EVENT_PYL(p_base, QIMG_EVT_ERROR, status, status);
  return NULL;
}

/**
 * Function: wd_comp_start
 *
 * Description: Start the execution of Wavelet denoise
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type will be defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int wd_comp_start(void *handle, void *p_data)
{
  wd_comp_t *p_comp = (wd_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_frame_t *p_frame = NULL;
  int count, i;

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT) ||
    (NULL == p_base->thread_loop)) {
    IDBG_ERROR("%s:%d] Error state %d", __func__, __LINE__,
      p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_NOT_SUPPORTED;
  }

  status = wd_comp_calibrate(p_comp);

  IMG_CHK_ABORT_UNLK_RET(p_base, &p_base->mutex);
  pthread_mutex_unlock(&p_base->mutex);
  if (status < 0) {
    IDBG_ERROR("%s:%d] calibration error %d", __func__, __LINE__, status);
    return status;
  }

  if (p_base->mode == IMG_SYNC_MODE) {

    count = img_q_count(&p_base->inputQ);
    IDBG_MED("%s:%d] num buffers %d", __func__, __LINE__,
      count);

    for (i = 0; i < count; i++) {
      p_frame = img_q_dequeue(&p_base->inputQ);
      if (NULL == p_frame) {
        IDBG_ERROR("%s:%d] invalid buffer", __func__, __LINE__);
        return IMG_GET_FRAME_FAILED;
      }

      status = wd_comp_process_frame(p_comp, p_frame);
      if (status < 0) {
        IDBG_ERROR("%s:%d] process error %d", __func__, __LINE__, status);
        return status;
      }
    }
  } else {
    status = p_comp->b.ops.start(&p_comp->b, p_data);
  }

  return status;
}

/**
 * Function: wd_comp_abort
 *
 * Description: Aborts the execution of Wavelet denoise
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type is defined in denoise.h
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int wd_comp_abort(void *handle, void *p_data)
{
  wd_comp_t *p_comp = (wd_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;

  IDBG_HIGH("%s:%d] p_base->mode = %d", __func__, __LINE__,p_base->mode);

  if (p_base->mode == IMG_ASYNC_MODE) {
    status = p_comp->b.ops.abort(&p_comp->b, p_data);
  }

  return (status < 0) ? status : 0;
}

/**
 * Function: wd_comp_process
 *
 * Description: This function is used to send any specific commands for the
 *              Wavelet denoise component
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   cmd - The command type which needs to be processed
 *   p_data - The pointer to the command payload
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *
 * Notes: none
 **/
int wd_comp_process (void *handle, img_cmd_type cmd, void *p_data)
{
  wd_comp_t *p_comp = (wd_comp_t *)handle;
  int status;

  status = p_comp->b.ops.process(&p_comp->b, cmd, p_data);
  if (status < 0)
    return status;

  return 0;
}

/**
 * Function: wd_comp_create
 *
 * Description: This function is used to create Qualcomm Wavelet
 *              denoise component
 *
 * Input parameters:
 *   @handle: library handle
 *   @p_ops - The pointer to img_component_t object. This object
 *            contains the handle and the function pointers for
 *            communicating with the imaging component.
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int wd_comp_create(void* handle, img_component_ops_t *p_ops)
{
  IMG_UNUSED(handle);

  wd_comp_t *p_comp = NULL;
  int status;

  if (NULL == g_wd_lib.ptr) {
    IDBG_ERROR("%s:%d] library not loaded", __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (NULL == p_ops) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  p_comp = (wd_comp_t *)malloc(sizeof(wd_comp_t));
  if (NULL == p_comp) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  memset(p_comp, 0x0, sizeof(wd_comp_t));
  status = img_comp_create(&p_comp->b);
  if (status < 0)
  {
    free(p_comp);
    return status;
  }

  /*set the main thread*/
  p_comp->b.thread_loop = wd_thread_loop;
  p_comp->b.p_core = p_comp;

  /* copy the ops table from the base component */
  *p_ops = p_comp->b.ops;
  p_ops->init            = wd_comp_init;
  p_ops->deinit          = wd_comp_deinit;
  p_ops->set_parm        = wd_comp_set_param;
  p_ops->get_parm        = wd_comp_get_param;
  p_ops->start           = wd_comp_start;
  p_ops->abort           = wd_comp_abort;
  p_ops->process         = wd_comp_process;

  p_comp->mode = WD_MODE_YCBCR_PLANE;

  p_ops->handle = (void *)p_comp;
  return IMG_SUCCESS;
}

/**
 * Function: wd_comp_load
 *
 * Description: This function is used to load Qualcomm wavelet library
 *
 * Input parameters:
 *   @name: library name
 *   @handle: library handle
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_NOT_FOUND
 *
 * Notes: none
 **/
int wd_comp_load(const char* name, void** handle)
{
  IMG_UNUSED(name);
  IMG_UNUSED(handle);

  int rc;
  if (g_wd_lib.ptr) {
    IDBG_ERROR("%s:%d] library already loaded", __func__, __LINE__);
    return IMG_SUCCESS;
  }

  g_wd_lib.ptr = dlopen("libmmcamera_wavelet_lib.so", RTLD_NOW);
  if (!g_wd_lib.ptr) {
    IDBG_ERROR("%s:%d] Error opening WD library", __func__, __LINE__);
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(g_wd_lib.camera_denoising_manager) =
    dlsym(g_wd_lib.ptr, "camera_denoising_manager");
  if (!g_wd_lib.camera_denoising_manager) {
    IDBG_ERROR("%s:%d] Error linking camera_denoising_manager",
      __func__, __LINE__);
    dlclose(g_wd_lib.ptr);
    g_wd_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(g_wd_lib.camera_denoising_get_alloc_prop) =
    dlsym(g_wd_lib.ptr, "camera_denoising_get_alloc_prop");
  if (!g_wd_lib.camera_denoising_get_alloc_prop) {
    IDBG_ERROR("%s:%d] Error linking camera_denoising_get_alloc_prop",
      __func__, __LINE__);
    dlclose(g_wd_lib.ptr);
    g_wd_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(g_wd_lib.camera_denoising_init) =
    dlsym(g_wd_lib.ptr, "camera_denoising_init");
  if (!g_wd_lib.camera_denoising_init) {
    IDBG_ERROR("%s:%d] Error linking camera_denoising_init",
      __func__, __LINE__);
    dlclose(g_wd_lib.ptr);
    g_wd_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  rc = g_wd_lib.camera_denoising_init();
  IDBG_HIGH("%s:%d] WD library loaded successfully dsp %d",
    __func__, __LINE__, rc);
  return IMG_SUCCESS;
}

/**
 * Function: wd_comp_unload
 *
 * Description: This function is used to unload Qualcomm wavelet library
 *
 * Input parameters:
 *   @handle: library handle
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void wd_comp_unload(void* handle)
{
  IMG_UNUSED(handle);

  int rc = 0;
  IDBG_HIGH("%s:%d] ptr %p", __func__, __LINE__, g_wd_lib.ptr);
  if (g_wd_lib.ptr) {
    rc = dlclose(g_wd_lib.ptr);
    if (rc < 0)
      IDBG_HIGH("%s:%d] error %s", __func__, __LINE__, dlerror());
    g_wd_lib.ptr = NULL;
  }
}
