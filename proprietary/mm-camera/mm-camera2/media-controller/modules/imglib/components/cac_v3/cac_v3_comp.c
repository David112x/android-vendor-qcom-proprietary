/**********************************************************************
*  Copyright (c) 2013-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include <linux/msm_ion.h>
#include "cac_v3_comp.h"
#include "chromatix_swpostproc.h"
#include <cutils/properties.h>

//#define USE_CHROMATIX
//#define USE_RNR_HYSTERISIS
//#define ENABLE_SKIN_RNR_CHROMATIX


#define RNR_LUT_REGIONS 2
//#define DBG_SIGMA_LUT

/**
 * CONSTANTS and MACROS
 **/

/* CAC3 Setprop config */
static uint32_t cac3_config;

/**skin_sigma_lut_in
  * Default skin sigma table for RNR under nornal lighting
  * conditions
**/
#ifndef USE_CHROMATIX
static const float skin_sigma_lut_in[SKINR_LUT_SIZE] = {
  1.0000f, 0.9992f, 0.9968f, 0.9928f, 0.9873f, 0.9802f, 0.9716f, 0.9616f,
  0.9501f, 0.9373f,
  0.9231f, 0.9077f, 0.8912f, 0.8735f, 0.8549f, 0.8353f, 0.8148f, 0.7936f,
  0.7717f, 0.7492f,
  0.7261f, 0.7027f, 0.6790f, 0.6549f, 0.6308f, 0.6065f, 0.5823f, 0.5581f,
  0.5341f, 0.5103f,
  0.4868f, 0.4636f, 0.4408f, 0.4184f, 0.3966f, 0.3753f, 0.3546f, 0.3345f,
  0.3150f, 0.2962f,
  0.2780f, 0.2606f, 0.2439f, 0.2278f, 0.2125f, 0.1979f, 0.1840f, 0.1708f,
  0.1583f, 0.1465f,
  0.1353f, 0.1248f, 0.1150f, 0.1057f, 0.0970f, 0.0889f, 0.0814f, 0.0743f,
  0.0678f, 0.0617f,
  0.0561f, 0.0510f, 0.0462f, 0.0418f, 0.0377f, 0.0340f, 0.0307f, 0.0276f,
  0.0247f, 0.0222f,
  0.0198f, 0.0177f, 0.0158f, 0.0141f, 0.0125f, 0.0111f, 0.0098f, 0.0087f,
  0.0077f, 0.0068f,
  0.0060f, 0.0053f, 0.0046f, 0.0040f, 0.0035f, 0.0031f, 0.0027f, 0.0023f,
  0.0020f, 0.0018f,
  0.0015f, 0.0013f, 0.0011f, 0.0010f, 0.0009f, 0.0007f, 0.0006f, 0.0005f,
  0.0005f, 0.0004f,
  0.0003f, 0.0003f, 0.0002f, 0.0002f, 0.0002f, 0.0001f, 0.0001f, 0.0001f,
  0.0001f, 0.0001f,
  0.0001f, 0.0001f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f,
  0.0000f, 0.0000f,
  0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f,
  0.0000f, 0.0000f,
  0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f,
  0.0000f, 0.0000f,
  0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f,
  0.0000f, 0.0000f,
  0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f,
  0.0000f, 0.0000f,
  0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f,
  0.0000f, 0.0000f,
  0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f,
  0.0000f, 0.0000f,
  0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f,
  0.0000f, 0.0000f,
  0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f,
  0.0000f, 0.0000f,
  0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f,
  0.0000f, 0.0000f,
  0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f,
  0.0000f, 0.0000f,
  0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f,
  0.0000f, 0.0000f,
  0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f,
  0.0000f, 0.0000f,
  0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f,
  0.0000f, 0.0000f,
  0.0000f, 0.0000f, 0.0000f, 0.0000f, 0.0000f
};
#endif //USE_CHROMATIX
/** cac_lib_info_t
 * @ptr: Pointer to CAC lib
 * @cac3_process: Function pointer for cac3_process
 * @cac3_init: Function pointer for cac3_init
 * @cac3_deinit: Function pointer for cac3_deinit
 * @cac3_pre_allocate_buffers: Function pointer for cac3_pre_allocate_buffers
 * @cac3_destroy_buffers: Function pointer for cac3_destroy_buffers
 **/
typedef struct {
  void *ptr;
  int (*cac3_process)(cac3_args_t  *args);
  int (*cac3_init)(int ion_heap_id);
  int (*cac3_deinit)();
  int (*cac3_pre_allocate_buffers)(int ion_heap_id);
  uint32_t (*cac3_destroy_buffers)();
} cac_lib_info_t;

static cac_lib_info_t g_cac_lib;

/**
 * Function: cac3_comp_configure_buffer1
 *
 * Description: Config input buffer when convert YUYV422 to YUV20 is needed
 *
 * Input parameters:
 *   p_frame - The pointer to the input frame
 *   p_addr - CbCr plane address
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int cac3_comp_configure_buffer1(img_frame_t *p_frame, uint8_t *p_addr)
{
  uint8_t *p_src_addr = NULL, *p_dest_addr = NULL;
  int width = 0, height = 0, stride = 0;
  int i = 0;

  if (NULL == p_frame) {
    IDBG_ERROR("%s %d: Frame is NULL",  __func__,  __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  width = p_frame[0].frame[0].plane[0].width / 2;
  height = p_frame[0].frame[0].plane[0].height / 2;
  stride = p_frame[0].frame[0].plane[0].stride;

  IDBG_LOW("%s %d: Cbcr Addr %p, wxh %dx%d", __func__, __LINE__,
    p_addr, width, height);
  //copy the chroma from next to last line to last line to cover a VPU
  //YUYV422 output issue
  p_dest_addr = p_addr + stride * (height - 1)  + 1;
  p_src_addr = p_addr + stride * (height - 2) + 1;

  IDBG_LOW("%s %d: Cbcr Addr %p, wxh %dx%d stride %d", __func__, __LINE__,
    p_addr, width, height, stride);
  IDBG_LOW("%s %d: src_addr %p dest addr %p", __func__, __LINE__,
    p_src_addr, p_dest_addr);

  for (i = 0; i < width; i++) {
    *p_dest_addr = *p_src_addr;
    p_src_addr = p_src_addr + 2;
    p_dest_addr = p_dest_addr + 2;
  }

  return IMG_SUCCESS;
}

/**
 * Function: cac3_comp_configure_buffer
 *
 * Description: Config input buffer
 *
 * Input parameters:
 *   p_frame - The pointer to the input frame
 *   pl_idx - Plane index
 *   padded_width - Width after padding
 *   padded_height -  Height after padding
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int cac3_comp_configure_buffer(img_frame_t *p_frame, uint16_t pl_idx,
  int padded_width, int padded_height)
{
  int rc = IMG_SUCCESS;
  uint8_t *p_addr = NULL, *p_src_addr = NULL, *p_dest_addr = NULL;
  int width = 0, vlines_to_pad = 0, hlines_to_pad = 0;
  int height = 0, stride = 0;
  int i = 0, j = 0;
  uint8_t *dst_ptr = NULL, *src_ptr = NULL;

  if (NULL == p_frame) {
    IDBG_ERROR("%s %d: Frame is NULL",  __func__,  __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  p_addr = p_frame[0].frame[0].plane[pl_idx].addr;
  width = p_frame[0].frame[0].plane[pl_idx].width/2;
  height = p_frame[0].frame[0].plane[pl_idx].height/2;
  stride = p_frame[0].frame[0].plane[pl_idx].stride;

  //Check if width needs padding
  vlines_to_pad = padded_width - width;

  IDBG_LOW("%s %d: Cbcr Addr %p, wxh %dx%d, padded wxh %dx%d", __func__,
    __LINE__,
    p_addr, width, height, padded_width, padded_height);
  //Mirror the last 128 columns to the right of the chroma buffer
  if (vlines_to_pad > 0) {
    p_dest_addr = p_addr + width + (vlines_to_pad - 1);
    p_src_addr = p_addr + (width - (vlines_to_pad + 1));

    IDBG_LOW("%s %d: Cbcr Addr %p, wxh %dx%d", __func__, __LINE__,
      p_addr, width, height);
    IDBG_LOW("%s %d: src_addr %p dest addr %p", __func__, __LINE__,
      p_src_addr, p_dest_addr);
    for (i = 0; i < height; i++) {
      src_ptr = p_src_addr;
      dst_ptr = p_dest_addr;
      for (j = 0; j < vlines_to_pad; j++) {
        *dst_ptr-- = *src_ptr++;
      }
      p_src_addr += stride;
      p_dest_addr += stride;
    }
  }

  //Check if width needs padding
  hlines_to_pad = padded_height - height;

  IDBG_LOW("%s %d:vlines2pad %d, hlines2pad %d", __func__, __LINE__,
    vlines_to_pad, hlines_to_pad);

  if (hlines_to_pad > 0) {
    //Mirror the last but one line
    p_dest_addr = p_addr + (stride * (height - 1 + hlines_to_pad));
    p_src_addr = p_addr + (stride * (height - 1 - hlines_to_pad));
    IDBG_INFO("%s %d: src_addr %p dest addr %p base addr %p", __func__,
      __LINE__,
      p_src_addr, p_dest_addr, p_addr);

    for (j= 0; j < hlines_to_pad; j++) {
      memcpy(p_dest_addr, p_src_addr, vlines_to_pad + width);
      p_src_addr += stride;
      p_dest_addr -= stride;
    }
  }
  return rc;
}

/**
 * Function: cac3_comp_init
 *
 * Description: Initializes the CAC component
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
int cac3_comp_init(void *handle, void* p_userdata, void *p_data)
{
  cac3_comp_t *p_comp = (cac3_comp_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] %p ", __func__, __LINE__, p_userdata);
  status = p_comp->b.ops.init(&p_comp->b, p_userdata, p_data);
  if (status < 0) {
    IDBG_ERROR("%s:%d] p_comp->b.ops.init returned %d",
      __func__, __LINE__, status);
    return status;
  }
  p_comp->userdata = p_userdata;
  p_comp->cac3_cb = p_data;
  return status;
}

/**
 * Function: cac3_comp_lib_debug
 *
 * Description: Debug params for cac library
 *
 * Input parameters:
 *   p_caclib - library instance pointer
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static void cac3_comp_lib_debug(cac3_args_t *p_caclib)
{
  IDBG_MED("%s:%d] CACLIB pIonBuf %p", __func__, __LINE__,
    p_caclib->p_ion_vaddr);
  IDBG_MED("%s:%d] CACLIB pInY %p", __func__, __LINE__,
    p_caclib->p_y);
  IDBG_MED("%s:%d] CACLIB pInC %p", __func__, __LINE__,
    p_caclib->p_crcb);
  IDBG_MED("%s:%d] CACLIB fdInY %d", __func__, __LINE__,
    p_caclib->fd);
  IDBG_MED("%s:%d] CACLIB Ion Heap ID %d", __func__, __LINE__,
    p_caclib->ion_heap_id);
  IDBG_MED("%s:%d] CACLIB y_width %d", __func__, __LINE__,
    p_caclib->y_width);
  IDBG_MED("%s:%d] CACLIB Y_height %d", __func__, __LINE__,
    p_caclib->y_height);
  IDBG_MED("%s:%d] CACLIB Y_stride %d", __func__, __LINE__,
    p_caclib->y_stride);
  IDBG_MED("%s:%d] CACLIB cbcr_width %d", __func__, __LINE__,
    p_caclib->cbcr_width);
  IDBG_MED("%s:%d] CACLIB cbcr_height %d", __func__, __LINE__,
    p_caclib->cbcr_height);
  IDBG_MED("%s:%d] CACLIB C_stride %d", __func__, __LINE__,
    p_caclib->cbcr_stride);
  IDBG_MED("%s:%d] CACLIB chromaorder %d", __func__, __LINE__,
    p_caclib->image_format);

  IDBG_MED("%s:%d] -------CAC PARAMS-----------", __func__, __LINE__);
  IDBG_MED("%s:%d] CACLIB Bright_Spot_HighTH %d", __func__, __LINE__,
    p_caclib->detection_th1);
  IDBG_MED("%s:%d] CACLIB Saturation_TH %d", __func__, __LINE__,
    p_caclib->detection_th2);
  IDBG_MED("%s:%d] CACLIB Verification_TH %d", __func__, __LINE__,
    p_caclib->verification_th1);
  IDBG_MED("%s:%d] CACLIB Verification_TH2 %d", __func__, __LINE__,
    p_caclib->verification_th2);

  IDBG_MED("%s:%d] -------RNR PARAMS-----------", __func__, __LINE__);
  IDBG_MED("%s:%d] CACLIB sampling_factor %d", __func__, __LINE__,
    p_caclib->sampling_factor);
   IDBG_MED("%s:%d] CACLIB lut_size %d", __func__, __LINE__,
    p_caclib->lut_size);
  IDBG_MED("%s:%d] CACLIB sigma_lut %p", __func__, __LINE__,
    p_caclib->sigma_lut);
#ifdef DBG_SIGMA_LUT
  for (i =0; i < p_caclib->lut_size; i++) {
    IDBG_MED("%s:%d] CACLIB sigma_lut[%d] = %f",  __func__, __LINE__,
      i, p_caclib->sigma_lut[i]);
  }
#endif
  IDBG_MED("%s:%d] CACLIB center_noise_sigma %f", __func__, __LINE__,
    p_caclib->center_noise_sigma);
  IDBG_MED("%s:%d] CACLIB center_noise_weight %f", __func__, __LINE__,
    p_caclib->center_noise_weight);
  IDBG_MED("%s:%d] CACLIB weight_order %f", __func__, __LINE__,
    p_caclib->weight_order);
  IDBG_MED("%s:%d] CACLIB scale_factor %f", __func__, __LINE__,
    p_caclib->scale_factor);
  IDBG_MED("%s:%d] CACLIB skin_cr %d", __func__, __LINE__,
    p_caclib->skin_cr);
  IDBG_MED("%s:%d] CACLIB skin_cb %d", __func__, __LINE__,
    p_caclib->skin_cb);
  IDBG_MED("%s:%d] CACLIB skin_theshold %f", __func__, __LINE__,
    p_caclib->skin_threshold);
  IDBG_MED("%s:%d] CACLIB skin_strength %f", __func__, __LINE__,
    p_caclib->skin_strength);
  IDBG_MED("%s:%d] CACLIB skin_ymin %d", __func__, __LINE__,
    p_caclib->skin_ymin);
  IDBG_MED("%s:%d] CACLIB skin_ymax %d", __func__, __LINE__,
    p_caclib->skin_ymax);
  IDBG_MED("%s:%d] CACLIB skin_scaler %f", __func__, __LINE__,
    p_caclib->skin_scaler);
  IDBG_MED("%s:%d] CACLIB skin_sigma_scale %f", __func__, __LINE__,
    p_caclib->skin_sigma_scale);
  IDBG_MED("%s:%d] CACLIB ds_us_skin_detection %d", __func__, __LINE__,
    p_caclib->ds_us_skin_detection);
  IDBG_MED("%s:%d] CACLIB skin_sigma_luzt_size %d", __func__, __LINE__,
    p_caclib->skin_sigma_lut_size);
#ifdef DBG_SIGMA_LUT
  for (i = 0; i < p_caclib->skin_sigma_lut_size; i++) {
    IDBG_MED("%s:%d] CACLIB skin_sigma_luzt %f", __func__, __LINE__,
    p_caclib->skin_sigma_lut[i]);
  }
#endif
  IDBG_MED("%s:%d] CACLIB cac2_enabled %d", __func__, __LINE__,
    p_caclib->cac3_enable_flag);
  IDBG_HIGH("%s:%d] CACLIB rnr_enabled %d", __func__, __LINE__,
    p_caclib->rnr_enable_flag);
  IDBG_HIGH("%s:%d] CACLIB skin RNR enabled %d", __func__, __LINE__,
    p_caclib->skin_rnr_enable_flag);
  IDBG_HIGH("%s:%d] CACLIB lds_enabled %d", __func__, __LINE__,
    p_caclib->lds_enable_flag);
  IDBG_HIGH("%s:%d] CACLIB cds_enabled %d", __func__, __LINE__,
    p_caclib->cds_enable_flag);
  IDBG_HIGH("%s:%d] CACLIB yuyv422_to_yuv420_enable_flag %d",
    __func__, __LINE__,  p_caclib->yuyv422_to_yuv420);
}

/**
 * Function: cac3_comp_fill_metadata
 *
 * Description: Fill in metadata is requested
 *
 * Input parameters:
 *   cac_rnr_params - Pointer to the cac3_args_t structure
 *   p_comp - Pointer to the component handle.
 *
 * Return values:
 *   standard img lib return codes
 *
 * Notes: memory allocated for the structures should be freed by the metadata
 * owner
 **/
static int cac3_comp_fill_metadata(cac3_args_t *cac_rnr_params,
  cac3_comp_t *p_comp)
{
  pproc_meta_data_t *p_meta = NULL;
  cac3_info_t *p_cac3 = NULL;
  rnr_info_t *p_rnr = NULL;
  lds_info_t *p_lds = NULL;
  fd_info_t *p_fd = NULL;
  uint32_t i = 0;
  int rc = IMG_SUCCESS;

  p_meta = (pproc_meta_data_t *)p_comp->b.debug_info.meta_data;

  IDBG_MED("%s %d] p_meta %p", __func__, __LINE__, p_meta);
  if (NULL == p_meta) {
    IDBG_ERROR("%s %d] metadata ptr is null", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  /*Allocate memory for the structures. Will be freed by pproc */
  p_cac3 = (cac3_info_t *) malloc(sizeof(cac3_info_t));
  if (!p_cac3) {
    IDBG_ERROR("%s %d]Error Allocating cac3 metadata memory", __func__,
      __LINE__);
    return IMG_ERR_GENERAL;
  }
  memset(p_cac3, 0, sizeof(cac3_info_t));
  p_cac3->enable = p_comp->cac_enable_flag;
  p_cac3->Detection_TH1 = p_comp->cac_chromatix_info.detection_th1;
  p_cac3->Detection_TH2 = p_comp->cac_chromatix_info.detection_th2;
  p_cac3->Verification_TH1 = p_comp->cac_chromatix_info.verification_th1;
  p_cac3->Verification_TH2 = p_comp->cac_chromatix_info.verification_th2;

  p_meta->header.tuning_size[PPROC_META_DATA_CAC3_IDX] = sizeof(cac3_info_t);
  p_meta->entry[PPROC_META_DATA_CAC3_IDX].dump_type = PPROC_META_DATA_CAC3;
  p_meta->entry[PPROC_META_DATA_CAC3_IDX].lux_idx = p_comp->img_3A_info.lux_idx;
  p_meta->entry[PPROC_META_DATA_CAC3_IDX].gain = p_comp->img_3A_info.gain;
  p_meta->entry[PPROC_META_DATA_CAC3_IDX].pproc_meta_dump = p_cac3;
  p_meta->entry[PPROC_META_DATA_CAC3_IDX].len = sizeof(cac3_info_t);
  p_rnr = (rnr_info_t *) malloc(sizeof(rnr_info_t));
  if (!p_rnr) {
    free(p_cac3);
    IDBG_ERROR("%s %d:] Error allocating rnr metadata memory", __func__,
      __LINE__);
    return IMG_ERR_GENERAL;
  }
  memset(p_rnr, 0, sizeof(rnr_info_t));
  p_rnr->enable = p_comp->rnr_enable_flag;
  p_rnr->y_width = cac_rnr_params->y_width;
  p_rnr->y_height = cac_rnr_params->y_height;
  p_rnr->y_stride = cac_rnr_params->y_stride;
  p_rnr->cbcr_width = cac_rnr_params->cbcr_width;
  p_rnr->cbcr_height = cac_rnr_params->cbcr_height;
  p_rnr->cbcr_stride = cac_rnr_params->cbcr_stride;
  p_rnr->center_noise_sigma = p_comp->rnr_chromatix_info.center_noise_sigma;
  p_rnr->center_noise_weight = p_comp->rnr_chromatix_info.center_noise_weight;
  p_rnr->sampling_factor = p_comp->rnr_chromatix_info.sampling_factor;
  p_rnr->hyst_sample_factor = p_comp->rnr_chromatix_info.sampling_factor;
  p_rnr->weight_order = p_comp->rnr_chromatix_info.weight_order;
  p_rnr->scale_factor = p_comp->rnr_chromatix_info.scale_factor;
  p_rnr->sigma_lut_size = p_comp->rnr_chromatix_info.lut_size;
  for (i = 0; i < RNR_LUT_SIZE; i++) {
    p_rnr->sigma_lut[i] = p_comp->rnr_chromatix_info.sigma_lut[i];
  }
  p_rnr->skin_cb = p_comp->skin_rnr_chromatix_info.skin_cb;
  p_rnr->skin_cr = p_comp->skin_rnr_chromatix_info.skin_cr;
  p_rnr->skin_ymin = p_comp->skin_rnr_chromatix_info.skin_ymin;
  p_rnr->skin_ymax = p_comp->skin_rnr_chromatix_info.skin_ymax;
  p_rnr->skin_scaler_cr = p_comp->skin_rnr_chromatix_info.skin_scaler;
  p_rnr->skin_strength =  p_comp->skin_rnr_chromatix_info.skin_strength;
  p_rnr->skin_threshold = p_comp->skin_rnr_chromatix_info.skin_threshold;
  p_rnr->skin_sigma_scale = p_comp->skin_rnr_chromatix_info.skin_sigma_scale;
  p_rnr->skin_chroma_ds_factor =
    p_comp->skin_rnr_chromatix_info.skin_chroma_ds_factor;
  p_rnr->ds_us_skin_detection =
    p_comp->skin_rnr_chromatix_info.ds_us_skin_detection;
  p_rnr->skin_lut_size = p_comp->skin_rnr_chromatix_info.skin_sigma_lut_size;
  p_rnr->reserve_data[0] = p_comp->skin_rnr_chromatix_info.s_rnr_enabled;
  for (i = 0; i < p_rnr->skin_lut_size; i++) {
    p_rnr->skin_lut[i] = p_comp->skin_rnr_chromatix_info.skin_sigma_lut[i];
  }
  p_meta->header.tuning_size[PPROC_META_DATA_RNR_IDX] = sizeof(rnr_info_t);
  p_meta->entry[PPROC_META_DATA_RNR_IDX].dump_type = PPROC_META_DATA_RNR;
  p_meta->entry[PPROC_META_DATA_RNR_IDX].lux_idx = p_comp->img_3A_info.lux_idx;
  p_meta->entry[PPROC_META_DATA_RNR_IDX].gain = p_comp->img_3A_info.gain;
  p_meta->entry[PPROC_META_DATA_RNR_IDX].pproc_meta_dump = p_rnr;
  p_meta->entry[PPROC_META_DATA_RNR_IDX].len = sizeof(rnr_info_t);

  p_lds = (lds_info_t *) malloc(sizeof(lds_info_t));
  if (!p_lds) {
    free(p_cac3);
    free(p_rnr);
    IDBG_ERROR("%s %d: Error allocating lds metadata memory", __func__,
      __LINE__);
    return IMG_ERR_GENERAL;
  }
  memset(p_lds, 0, sizeof(lds_info_t));
  p_lds->enable = p_comp->lds_enable_flag;
  p_lds->LDS_DS_ratio = 2;
  p_meta->header.tuning_size[PPROC_META_DATA_LDS_IDX] = sizeof(lds_info_t);
  p_meta->entry[PPROC_META_DATA_LDS_IDX].dump_type = PPROC_META_DATA_LDS;
  p_meta->entry[PPROC_META_DATA_LDS_IDX].lux_idx = p_comp->img_3A_info.lux_idx;
  p_meta->entry[PPROC_META_DATA_LDS_IDX].gain = p_comp->img_3A_info.gain;
  p_meta->entry[PPROC_META_DATA_LDS_IDX].pproc_meta_dump = p_lds;
  p_meta->entry[PPROC_META_DATA_LDS_IDX].len = sizeof(lds_info_t);

  p_fd = (fd_info_t *) malloc(sizeof(fd_info_t));
  if (!p_fd) {
    free(p_cac3);
    free(p_rnr);
    free(p_lds);
    IDBG_ERROR("%s %d: Error allocating FD metadata memory", __func__,
      __LINE__);
    return IMG_ERR_GENERAL;
  }
  memset(p_fd, 0, sizeof(fd_info_t));
  if (p_comp->fd_data.num_faces_detected > 0) {
    p_fd->face_detected = TRUE;
  } else {
    p_fd->face_detected = FALSE;
  }
  p_meta->header.tuning_size[PPROC_META_DATA_FD_IDX] = sizeof(fd_info_t);
  p_meta->entry[PPROC_META_DATA_FD_IDX].dump_type = PPROC_META_DATA_FD;
  p_meta->entry[PPROC_META_DATA_FD_IDX].pproc_meta_dump = p_fd;
  p_meta->entry[PPROC_META_DATA_FD_IDX].len = sizeof(fd_info_t);

  return rc;
}

/**
 * Function: cac3_comp_get_skin_ds_factor
 *
 * Description: Get the skin detection DS factor
 *
 * Input parameters:
 *   p_comp - The pointer to the component handle.
 *
 * Return values:
 *     skin detect ds ratio
 *
 * Notes: none
 **/
uint8_t cac3_comp_get_skin_ds_factor(cac3_comp_t *p_comp)
{
  int8_t cds_shift_factor = 0, lds_shift_factor = 0;
  uint8_t skin_detection_ds = 1;

  if (p_comp->cac_enable_flag) {
    skin_detection_ds = 1;
  } else {
    if (p_comp->cds_enable_flag) {
      cds_shift_factor = 1;
    }
    if (p_comp->lds_enable_flag) {
      lds_shift_factor = 1;
    }
    IDBG_MED("%s %d: skin_chroma_ds_factor %d", __func__, __LINE__,
      p_comp->skin_rnr_chromatix_info.skin_chroma_ds_factor);
    skin_detection_ds =
      p_comp->skin_rnr_chromatix_info.skin_chroma_ds_factor >>
      (lds_shift_factor + cds_shift_factor);
  }
  /*Need to fix in chromatix*/
  if (skin_detection_ds == 0) {
     skin_detection_ds = 1;
  }

  return skin_detection_ds;
}

/**
 * Function: cac3_comp_get_rnr_scaling_factor
 *
 * Description: This function calculates the scaling factor for
 * the current resolution wrt the max resolution (camif o/p)
 *
 * Arguments:
 *   @p_comp: cac component
 *   @p_frame: Input image frame
 *
 * Return values:
 *     new scaling factor
 *
 * Notes: none
 **/
static float cac3_comp_get_rnr_scaling_factor(cac3_comp_t *p_comp,
  img_frame_t *p_frame)
{
  float center_x, center_y, max_radius;
  float new_scale_factor = 0.0f;
  uint32_t current_width, current_height;

  max_radius = (float)p_comp->rnr_chromatix_info.lut_size - 1;

  // Current snapshot resolution
  current_width  = p_frame[0].info.width;
  current_height = p_frame[0].info.height;
  center_x = (float)(current_width - 1) * 0.5f;
  center_y = (float)(current_height - 1) * 0.5f;
  if (max_radius > 0.0f) {
    new_scale_factor =
      (float)(sqrt(center_x * center_x + center_y * center_y)) / max_radius;
  }

  return new_scale_factor;
}

#ifdef USE_RNR_HYSTERISIS
/* Function: cac3_comp_rnr_hysterisis
 *
 * Description: Helper function for rnr hysterisis
 *
 * Arguments:
 *   @trigger_pt_values: array of 4 tigger pt values
 *   @rnr_hysterisis_info hysterisis info for RNR
 *   @trigger: can be gain or lux value
 *   @trend: hysterisis trend
 *
 * Return values:
 *     Sampling factor
 *
 * Notes: none
 **/
static int cac3_comp_rnr_hysterisis(float *trigger_pt_values,
  hysterisis_info_t *rnr_hysterisis_info, float trigger, hysterisis_trend trend)
{
  int sampling_factor = 2;

  if (!trigger_pt_values) {
    IDBG_ERROR("%s %d: Null parameter for trigger_pt_values",
      __func__, __LINE__);
    return 0;
  }
  float trigger_ptA = trigger_pt_values[0];
  float trigger_ptB = trigger_pt_values[1];
  float trigger_ptC = trigger_pt_values[2];
  float trigger_ptD = trigger_pt_values[3];

  IDBG_MED("%s %d: trigger %f TpointA %f, TpointB %f, Tpointc %f, TpointD %f",
    __func__, __LINE__, trigger, trigger_ptA, trigger_ptB, trigger_ptC,
    trigger_ptD);

  if (trigger < trigger_ptA) {
    sampling_factor = 2;
    rnr_hysterisis_info->norml_hyst_enabled = FALSE;
    rnr_hysterisis_info->lowl_hyst_enabled = FALSE;
    IDBG_MED("%s %d: trigger < trigger_ptA, sampling Factor = %d",
      __func__,__LINE__, sampling_factor);
  } else if (trigger > trigger_ptD) {
    sampling_factor = 8;
    rnr_hysterisis_info->norml_hyst_enabled = FALSE;
    rnr_hysterisis_info->lowl_hyst_enabled = FALSE;
    IDBG_MED("%s %d: trigger > trigger_ptD, sampling Factor = %d",
      __func__, __LINE__, sampling_factor);
  } else if ((trigger > trigger_ptB) && (trigger < trigger_ptC)) {
    sampling_factor = 4;
    rnr_hysterisis_info->norml_hyst_enabled = FALSE;
    rnr_hysterisis_info->lowl_hyst_enabled = FALSE;
    IDBG_MED("%s %d: trigger > trigger_ptB && trigger < trigger_ptC,"
      "sampling Factor = %d", __func__, __LINE__, sampling_factor);
  } else {
    if (trigger >= trigger_ptA && trigger <= trigger_ptB) {
      if (rnr_hysterisis_info->norml_hyst_enabled) {
        if ((trend == HYSTERISIS_TREND_DOWNWARD) ||
          (trend == HYSTERISIS_TREND_UPWARD)) {
          sampling_factor = rnr_hysterisis_info->prev_sampling_factor;
          IDBG_MED("%s %d: Normal light Hysterisis, trend %d"
            "sampling Factor = %d", __func__, __LINE__, trend, sampling_factor);
        }
      } else {
        switch(trend) {
        case HYSTERISIS_TREND_UPWARD:
          sampling_factor = 2;
          break;
        case HYSTERISIS_TREND_DOWNWARD:
          sampling_factor = 4;
          break;
        case HYSTERISIS_TREND_NONE:
        default:
          IDBG_WARN("%s %d: trend NONE Invalid case,default sampling factor 2",
           __func__, __LINE__);
          break;
       }
       rnr_hysterisis_info->norml_hyst_enabled = TRUE;
       rnr_hysterisis_info->lowl_hyst_enabled = FALSE;
       IDBG_MED("%s %d: trend %d Normal Light Hysterisis enabled,"
         "sampling Factor = %d", __func__, __LINE__, trend, sampling_factor);
      }
    } else if (trigger >= trigger_ptC && trigger <= trigger_ptD) {
      if (rnr_hysterisis_info->lowl_hyst_enabled) {
        if ((trend == HYSTERISIS_TREND_DOWNWARD) ||
          (trend == HYSTERISIS_TREND_UPWARD)) {
          sampling_factor = rnr_hysterisis_info->prev_sampling_factor;
          IDBG_MED("%s %d: Low light Hysterisis, trend %d sampling Factor = %d",
            __func__, __LINE__, trend, sampling_factor);
        }
      } else {
        switch(trend) {
        case HYSTERISIS_TREND_UPWARD:
          sampling_factor = 4;
          break;
        case HYSTERISIS_TREND_DOWNWARD:
          sampling_factor = 8;
          break;
        case HYSTERISIS_TREND_NONE:
        default:
          IDBG_WARN("%s %d: trend None Invalid case,"
            "default sampling factor 2",
            __func__, __LINE__);
          break;
        }
        rnr_hysterisis_info->lowl_hyst_enabled = TRUE;
        rnr_hysterisis_info->norml_hyst_enabled = FALSE;

        IDBG_MED("%s %d: trend %d lowlHysterisis enabled,sampling Factor = %d",
          __func__, __LINE__, trend, sampling_factor);
      }
    }
  }
  return sampling_factor;
}

/**
 * Function: cac3_comp_get_sampling_factor
 *
 * Description: This function calculates the RNR sampling factor
 * using hysterisis
 *
 * Arguments:
 *   @p_client: CAC Component
 *   @chromatix_rnr: RNR structure from chromatix
 *   @trigger: current trigger -can be gain or lux value
 *
 * Return values:
 * None
 *
 * Notes: none
 **/
static void cac3_comp_get_sampling_factor(cac3_comp_t *p_comp,
  chromatix_RNR1_type *chromatix_rnr, float trigger)
{
  float hyst_trigger_pt_values[4];

  IDBG_MED("%s %d: chromatix_rnr->control_rnr %d,"
    "trigger %f, "
    "gain %lf, "
    "lux_idx %lf, "
    "prev_lux_value %lf, "
    "prev_gain_value %lf, "
    "prev_sampling_factor %d, "
    "s_rnr_enabled %d, ",
    __func__, __LINE__,
    chromatix_rnr->control_rnr,
    trigger,
    p_comp->rnr_hysterisis_info.gain,
    p_comp->rnr_hysterisis_info.lux_idx,
    p_comp->rnr_hysterisis_info.prev_lux_value,
    p_comp->rnr_hysterisis_info.prev_gain_value,
    p_comp->rnr_hysterisis_info.prev_sampling_factor,
    p_comp->skin_rnr_chromatix_info.s_rnr_enabled
    );

  if (chromatix_rnr->control_rnr == 0) {
    //If LUX did not change, use prev sampling factor
    if (trigger == p_comp->rnr_hysterisis_info.prev_lux_value &&
      p_comp->rnr_hysterisis_info.prev_sampling_factor) {
      p_comp->rnr_chromatix_info.sampling_factor =
        p_comp->rnr_hysterisis_info.prev_sampling_factor;
    } else {
      hyst_trigger_pt_values[0] =
        chromatix_rnr->hysteresis_point[0].lux_index_start;
      hyst_trigger_pt_values[1] =
        chromatix_rnr->hysteresis_point[0].lux_index_end;
      hyst_trigger_pt_values[2] =
        chromatix_rnr->hysteresis_point[1].lux_index_start;
      hyst_trigger_pt_values[3] =
        chromatix_rnr->hysteresis_point[1].lux_index_end;

      if (trigger > p_comp->rnr_hysterisis_info.prev_lux_value) {
        p_comp->rnr_hysterisis_info.lux_trend =
          HYSTERISIS_TREND_UPWARD;
      } else {
        p_comp->rnr_hysterisis_info.lux_trend =
          HYSTERISIS_TREND_DOWNWARD;
      }
      p_comp->rnr_chromatix_info.sampling_factor =
        cac3_comp_rnr_hysterisis(hyst_trigger_pt_values,
        &p_comp->rnr_hysterisis_info,trigger,
        p_comp->rnr_hysterisis_info.lux_trend);
    }
  } else {//gain based
    //If gain did not change, use prev sampling factor
    if (trigger == p_comp->rnr_hysterisis_info.prev_gain_value &&
      p_comp->rnr_hysterisis_info.prev_sampling_factor) {
      p_comp->rnr_chromatix_info.sampling_factor =
        p_comp->rnr_hysterisis_info.prev_sampling_factor;
    } else {
      hyst_trigger_pt_values[0] =
        chromatix_rnr->hysteresis_point[0].gain_start;
      hyst_trigger_pt_values[1] =
        chromatix_rnr->hysteresis_point[0].gain_end;
      hyst_trigger_pt_values[2] =
        chromatix_rnr->hysteresis_point[1].gain_start;
      hyst_trigger_pt_values[3] =
        chromatix_rnr->hysteresis_point[1].gain_end;

      if (trigger > p_comp->rnr_hysterisis_info.prev_gain_value) {
        p_comp->rnr_hysterisis_info.gain_trend =
          HYSTERISIS_TREND_UPWARD;
      } else {
        p_comp->rnr_hysterisis_info.gain_trend =
          HYSTERISIS_TREND_DOWNWARD;
      }
      p_comp->rnr_chromatix_info.sampling_factor =
        cac3_comp_rnr_hysterisis(hyst_trigger_pt_values,
        &p_comp->rnr_hysterisis_info, trigger,
        p_comp->rnr_hysterisis_info.gain_trend);
    }
  }
  return;
}
#endif

/**
 * Function: cac3_comp_get_interpolation_regions
 *
 * Description: Helper function is to get interpolation regions
 *
 * Arguments:
 *   @p_interp_info: Interpolation info pointer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
**/
int cac3_comp_get_interpolation_regions(
  interpolation_info_t *p_interp_info)
{
  int i = 0;
  if (NULL == p_interp_info) {
    IDBG_ERROR("%s %d: Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  for (i = 0; i < p_interp_info->num_of_regions; i++) {
    if ((p_interp_info->trigger_start[i] >= p_interp_info->trigger_end[i])) {
      IDBG_ERROR("%s %d: strigger start %f is <= to trigger end %f", __func__,
        __LINE__, p_interp_info->trigger_start[i],
        p_interp_info->trigger_end[i]);
      return IMG_ERR_INVALID_INPUT;
    }
    p_interp_info->regionStart = i;
    if (i == (p_interp_info->num_of_regions - 1) ||
      (p_interp_info->trigger <= p_interp_info->trigger_start[i])) {
      p_interp_info->regionEnd = i;
      p_interp_info->interp_ratio = 0.0;
      p_interp_info->enable_idx = i;
      break;
    } else if (p_interp_info->trigger < p_interp_info->trigger_end[i]) {
      p_interp_info->regionEnd = i + 1;
      p_interp_info->interp_ratio = (float) (p_interp_info->trigger -
        p_interp_info->trigger_start[i]) /
        (float)(p_interp_info->trigger_end[i] -
        p_interp_info->trigger_start[i]);
      p_interp_info->enable_idx = i;
      break;
    }
  }

  if (p_interp_info->interp_ratio < 1.0) {
    IDBG_MED("%s %d: Interpolation ratio %f < 1.0", __func__, __LINE__,
      p_interp_info->interp_ratio);
  }

  return IMG_SUCCESS;
}

/**
 * Function: cac3_comp_get_rnr_lut
 *
 * Description: Get RNR LUTs from chromatix
 *
 * Arguments:
 *   @p_comp: cac component
 *   @trigger: lux or gain
 *   @chromatix: chromatix pointer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
#ifdef USE_CHROMATIX
static int cac3_comp_get_rnr_lut(cac3_comp_t *p_comp,
  float trigger, chromatix_RNR1_type *chromatix_rnr)
{
  interpolation_info_t interp_info;
  float trigger_start[RNR_LUT_REGIONS];
  float trigger_end[RNR_LUT_REGIONS];
  int i = 0;

  if (!p_comp || !chromatix_rnr) {
    IDBG_ERROR("%s %d: Invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  interp_info.trigger = trigger;
  interp_info.trigger_start = trigger_start;
  interp_info.trigger_end = trigger_end;
  interp_info.interp_ratio = 0.0;

  /*Since there are 3 regions and 2 triggers, assigning the last region */
  interp_info.regionStart = RNR_LUT_REGIONS;
  interp_info.regionEnd = RNR_LUT_REGIONS;

  for (i = 0; i < RNR_LUT_REGIONS; i++) {
    if (chromatix_rnr->control_rnr == 0) {
      interp_info.trigger_start[i] =
        chromatix_rnr->sigma_lut_trigger[i].lux_index_start;
      interp_info.trigger_end[i] =
        chromatix_rnr->sigma_lut_trigger[i].lux_index_end;
    } else {
      interp_info.trigger_start[i] =
        chromatix_rnr->sigma_lut_trigger[i].gain_start;
      interp_info.trigger_end[i] =
        chromatix_rnr->sigma_lut_trigger[i].gain_end;
    }
  }

  for (i = 0; i < RNR_LUT_REGIONS; i++) {
    if ((interp_info.trigger_start[i] >= interp_info.trigger_end[i])) {
      IDBG_ERROR("%s %d: strigger start %f is <= to trigger end %f", __func__,
        __LINE__, interp_info.trigger_start[i],
        interp_info.trigger_end[i]);
      return IMG_ERR_INVALID_INPUT;
    }

    if (interp_info.trigger <= interp_info.trigger_start[i]) {
      interp_info.regionStart = i;
      interp_info.regionEnd = i;
      interp_info.interp_ratio = 0.0;
      break;
    } else if (interp_info.trigger < interp_info.trigger_end[i]) {
      interp_info.regionStart = i;
      interp_info.regionEnd = i+1;
      interp_info.interp_ratio = (float) (interp_info.trigger -
        interp_info.trigger_start[i]) /
        (float)(interp_info.trigger_end[i] -
        interp_info.trigger_start[i]);
      break;
    }
  }

  for (i = 0; i < p_comp->rnr_chromatix_info.lut_size; i++) {
    p_comp->rnr_chromatix_info.sigma_lut[i] =
      BILINEAR_INTERPOLATION(
        chromatix_rnr->sigma_lut[interp_info.regionStart][i],
        chromatix_rnr->sigma_lut[interp_info.regionEnd][i],
        interp_info.interp_ratio);
  }
#ifdef ENABLE_SKIN_RNR_CHROMATIX
  if (p_comp->skin_rnr_chromatix_info.s_rnr_enabled) {
    int skin_lut_size =
      p_comp->skin_rnr_chromatix_info.skin_sigma_lut_size;
    for (i = 0; i < skin_lut_size; i++) {
      p_comp->skin_rnr_chromatix_info.skin_sigma_lut[i] =
        BILINEAR_INTERPOLATION(
        chromatix_rnr->skin_lut[interp_info.regionStart][i],
        chromatix_rnr->skin_lut[interp_info.regionEnd][i],
        interp_info.interp_ratio);
    }
  }
#endif
  return IMG_SUCCESS;
}

/**
 * Function: cac3_comp_get_skin_rnr_params
 *
 * Description: This function updates the skin RNR parameters
 * for offline usecase for RNR from chromatix header
 *
 * Arguments:
 *   @p_comp: cac component
 *   @chromatix: chromatix ptr
 *   @p_interp_info: Interpolation info
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int cac3_comp_get_skin_rnr_params(cac3_comp_t *p_comp,
  chromatix_sw_postproc_type *chromatix, interpolation_info_t *p_interp_info)
{
#ifndef ENABLE_SKIN_RNR_CHROMATIX
  p_comp->skin_rnr_chromatix_info.s_rnr_enabled = FALSE;
  IDBG_ERROR("Disable skin RNR");
#else

  int regionStart = 0, regionEnd = 0;
  float skin_scaler = 0;

  chromatix_RNR1_type *chromatix_rnr =
    &(chromatix->chromatix_radial_noise1_reduction);

  if ((!chromatix_rnr) || (!p_interp_info)) {
    IDBG_ERROR("%s %d: Invalid params", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  regionStart = p_interp_info->regionStart;
  regionEnd = p_interp_info->regionEnd;

  p_comp->skin_rnr_chromatix_info.skin_cb =
    BILINEAR_INTERPOLATION (
    chromatix_rnr->rnr_data[regionStart].skin_cb,
    chromatix_rnr->rnr_data[regionEnd].skin_cb, p_interp_info->interp_ratio);
  p_comp->skin_rnr_chromatix_info.skin_cr =
    BILINEAR_INTERPOLATION (
    chromatix_rnr->rnr_data[regionStart].skin_cr,
    chromatix_rnr->rnr_data[regionEnd].skin_cr, p_interp_info->interp_ratio);
  p_comp->skin_rnr_chromatix_info.skin_ymax =
    BILINEAR_INTERPOLATION (
    chromatix_rnr->rnr_data[regionStart].skin_ymax,
    chromatix_rnr->rnr_data[regionEnd].skin_ymax, p_interp_info->interp_ratio);
  p_comp->skin_rnr_chromatix_info.skin_ymin =
    BILINEAR_INTERPOLATION (
    chromatix_rnr->rnr_data[regionStart].skin_ymin,
    chromatix_rnr->rnr_data[regionEnd].skin_ymin, p_interp_info->interp_ratio);
  p_comp->skin_rnr_chromatix_info.skin_strength =
    BILINEAR_INTERPOLATION (
    chromatix_rnr->rnr_data[regionStart].skin_strength,
    chromatix_rnr->rnr_data[regionEnd].skin_strength,
    p_interp_info->interp_ratio);
  p_comp->skin_rnr_chromatix_info.skin_threshold =
    BILINEAR_INTERPOLATION (
    chromatix_rnr->rnr_data[regionStart].skin_threshold,
    chromatix_rnr->rnr_data[regionEnd].skin_threshold,
    p_interp_info->interp_ratio);
  p_comp->skin_rnr_chromatix_info.skin_sigma_scale =
    BILINEAR_INTERPOLATION (
    chromatix_rnr->rnr_data[regionStart].skin_sigma_scale,
    chromatix_rnr->rnr_data[regionEnd].skin_sigma_scale,
    p_interp_info->interp_ratio);
  skin_scaler =
    BILINEAR_INTERPOLATION (
    chromatix_rnr->rnr_data[regionStart].skin_scaler_cr,
    chromatix_rnr->rnr_data[regionEnd].skin_scaler_cr,
    p_interp_info->interp_ratio);
  p_comp->skin_rnr_chromatix_info.skin_scaler = skin_scaler * skin_scaler;

  //Skin RNR downscale factor
  p_comp->skin_rnr_chromatix_info.skin_chroma_ds_factor =
    chromatix_rnr->rnr_data[regionStart].skin_chroma_ds_factor;
#endif //ENABLE_SKIN_RNR_CHROMATIX

  return IMG_SUCCESS;
}
#endif

/**
 * Function: cac3_comp_get_default_rnr_params
 *
 * Description: This function is to update the RNR parameters
 * for RNR from chromatix header
 *
 * Arguments:
 *   @p_client: cac component
 *   @p_frame: Input image frame
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
#ifndef USE_CHROMATIX
static int cac3_comp_get_default_rnr_params(cac3_comp_t *p_comp,
  img_frame_t *p_frame)
{
  uint8_t cb_min = 77, cb_max = 127;
  uint8_t cr_min = 133, cr_max = 173, sigma = 0;
  int i = 0;
  rnr_chromatix_info_t *rnr_chromatix_info =
    &p_comp->rnr_chromatix_info;
  skin_rnr_info_t *skin_rnr_chromatix_info =
    &p_comp->skin_rnr_chromatix_info;

  p_comp->rnr_enable_flag = TRUE;
  p_comp->skin_rnr_chromatix_info.s_rnr_enabled = TRUE;

  rnr_chromatix_info->lut_size = RNR_LUT_SIZE;
  for (i = 0; i < rnr_chromatix_info->lut_size; i++) {
    rnr_chromatix_info->sigma_lut[i] = sigma_lut_in[i];
  }
  rnr_chromatix_info->scale_factor =
    cac3_comp_get_rnr_scaling_factor(p_comp, p_frame);
  if (rnr_chromatix_info->scale_factor <= 0.0f) {
    IDBG_ERROR("%s %d: Invalid scale_factor %f", __func__, __LINE__,
      rnr_chromatix_info->scale_factor);
    return IMG_ERR_INVALID_INPUT;
  }

  rnr_chromatix_info->center_noise_sigma = 2.0f;
  rnr_chromatix_info->center_noise_weight = 1.0f;
  rnr_chromatix_info->sampling_factor = 8;

  //Weight order and skin strength depends on sampling factor
  rnr_chromatix_info->weight_order = 1.0f;
  skin_rnr_chromatix_info->skin_strength = 1.11f;

  skin_rnr_chromatix_info->skin_cb = (uint8_t)((cb_min + cb_max) / 2); // 102
  skin_rnr_chromatix_info->skin_cr = (uint8_t)((cr_min + cr_max) / 2); // 153;
  skin_rnr_chromatix_info->skin_ymin = 60;
  skin_rnr_chromatix_info->skin_ymax = 100;
  skin_rnr_chromatix_info->skin_threshold = 10.0f;
  skin_rnr_chromatix_info->skin_sigma_scale = 0.0f;

  sigma = skin_rnr_chromatix_info->skin_cb - cb_min;
  if (sigma <= 0.0f || skin_rnr_chromatix_info->skin_cr - cr_min <= 0.0f) {
    IDBG_ERROR("%s %d: Invalid sigma, cb, cr input, "
      "sigma=%f, skin_cr=%d, cr_min=%d", __func__, __LINE__, sigma,
      skin_rnr_chromatix_info->skin_cr, cr_min);
    return IMG_ERR_INVALID_INPUT;
  }
  skin_rnr_chromatix_info->skin_scaler  =
    (float)((sigma / (skin_rnr_chromatix_info->skin_cr - cr_min))*
    (sigma / (skin_rnr_chromatix_info->skin_cr - cr_min)));

  skin_rnr_chromatix_info->skin_sigma_lut_size = SKINR_LUT_SIZE;
  for (i = 0; i < SKINR_LUT_SIZE; i++) {
    skin_rnr_chromatix_info->skin_sigma_lut[i] = skin_sigma_lut_in[i];
  }

  return IMG_SUCCESS;
}
#endif //USE_CHROMATIX

/**
 * Function: cac3_comp_get_rnr_params
 *
 * Description: This function is to update the RNR parameters
 * for RNR from chromatix header
 *
 * Arguments:
 *   @p_client: cac component
 *   @p_frame: Input image frame
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int cac3_comp_get_rnr_params(cac3_comp_t *p_comp,
  img_frame_t *p_frame)
{
  int rc = IMG_SUCCESS;

#ifdef USE_CHROMATIX
  int i = 0;
  chromatix_sw_postproc_type *chromatix = NULL;
  chromatix_RNR1_type *chromatix_rnr = NULL;
  interpolation_info_t interp_info;
  int regionStart = 0, regionEnd = 0;
  float trigger, interp_ratio = 0;
  float trigger_start[MAX_LIGHT_TYPES_FOR_SPATIAL];
  float trigger_end[MAX_LIGHT_TYPES_FOR_SPATIAL];

  memset(&interp_info, 0, sizeof(interpolation_info_t));

  chromatix = (chromatix_sw_postproc_type *)p_comp->chromatix_ptr;
  if (NULL == chromatix) {
    IDBG_ERROR("%s %d: Chromatix is null", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  //Set RnR params
  chromatix_rnr =
    &(chromatix->chromatix_radial_noise1_reduction);

  //Check if RNR enabled or disabled
  p_comp->rnr_enable_flag = chromatix_rnr->rnr_enable;

  if (p_comp->rnr_enable_flag) {

    interp_info.trigger_start = trigger_start;
    interp_info.trigger_end = trigger_end;

    //Get the trigger
    trigger = (chromatix_rnr->control_rnr == 0) ?
      (float)p_comp->img_3A_info.lux_idx : (float)p_comp->img_3A_info.gain;

    IDBG_HIGH("%s: trigger = %f, rnr_enable_flag = %d\n", __func__, trigger,
      p_comp->rnr_enable_flag);

    //Get the start and end regions
    for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
      if (chromatix_rnr->control_rnr  == 0) { //lux index
        trigger_start[i] =
          chromatix_rnr->rnr_data[i].rnr_trigger.lux_index_start;
        trigger_end[i] =
          chromatix_rnr->rnr_data[i].rnr_trigger.lux_index_end;
      } else {
        trigger_start[i] = chromatix_rnr->rnr_data[i].rnr_trigger.gain_start;
        trigger_end[i] = chromatix_rnr->rnr_data[i].rnr_trigger.gain_end;
      }
    }

    interp_info.num_of_regions = MAX_LIGHT_TYPES_FOR_SPATIAL;
    interp_info.trigger = trigger;
    cac3_comp_get_interpolation_regions(&interp_info);

    regionStart = interp_info.regionStart;
    regionEnd = interp_info.regionEnd;
    interp_ratio = interp_info.interp_ratio;
    IDBG_MED("%s %d: regionStart %d, regionEnd %d", __func__, __LINE__,
      regionStart, regionEnd);

    //Get center noise weight
    p_comp->rnr_chromatix_info.center_noise_weight =
      chromatix_rnr->rnr_data[regionStart].center_noise_weight;
    //Weight Order
    p_comp->rnr_chromatix_info.weight_order =
      chromatix_rnr->rnr_data[regionStart].weight_order;

    //Get RNR LUT size -the number of valid entries in the RNR LUT table
    p_comp->rnr_chromatix_info.lut_size = chromatix_rnr->lut_size;
    p_comp->skin_rnr_chromatix_info.skin_sigma_lut_size =
      SKINR_LUT_SIZE;
    //Interpolate the LUT based on 3 trigger regions
    cac3_comp_get_rnr_lut(p_comp, trigger, chromatix_rnr);

    //Get Scale factor - depends on RNR LUT size
    p_comp->rnr_chromatix_info.scale_factor =
      cac3_comp_get_rnr_scaling_factor(p_comp, p_frame);
    IDBG_MED("%s :scaling factor %f", __func__,
      p_comp->rnr_chromatix_info.scale_factor);

    //Interpolate RNR params
    if (regionStart != regionEnd) {
      p_comp->rnr_chromatix_info.center_noise_sigma =
        BILINEAR_INTERPOLATION (
        chromatix_rnr->rnr_data[regionStart].center_noise_sigma,
        chromatix_rnr->rnr_data[regionEnd].center_noise_sigma, interp_ratio);
    } else {
      p_comp->rnr_chromatix_info.center_noise_sigma =
        chromatix_rnr->rnr_data[regionStart].center_noise_sigma;
    }
    IDBG_MED("%s: center_noise_sigma = %f\n", __func__,
      p_comp->rnr_chromatix_info.center_noise_sigma);

    //Interpolate Skin RNR params
    rc = cac3_comp_get_skin_rnr_params(p_comp, chromatix, &interp_info);


#ifdef USE_RNR_HYSTERISIS
    //Save current gain, lux and sampling factor  values as prev values
    p_comp->rnr_hysterisis_info.lux_idx = p_comp->img_3A_info.lux_idx;
    p_comp->rnr_hysterisis_info.gain = p_comp->img_3A_info.gain;
    p_comp->rnr_hysterisis_info.prev_lux_value =
      p_comp->img_3A_info.prev_lux_value;
    p_comp->rnr_hysterisis_info.prev_gain_value =
      p_comp->img_3A_info.prev_gain_value;
    p_comp->rnr_hysterisis_info.s_rnr_enabled =
      p_comp->img_3A_info.s_rnr_enabled;
    p_comp->skin_rnr_chromatix_info.s_rnr_enabled =
      p_comp->img_3A_info.s_rnr_enabled;
    IDBG_MED("%s %d: chromatix_rnr->control_rnr %d,"
      "trigger %f, "
      "p_comp->rnr_hysterisis_info.prev_lux_value %lf, "
      "p_comp->rnr_hysterisis_info.prev_gain_value %lf, "
      "IN p_comp->rnr_hysterisis_info.s_rnr_enabled %d "
      "p_comp->rnr_hysterisis_info.prev_sampling_factor %d, ",
      __func__, __LINE__,
      chromatix_rnr->control_rnr,
      trigger,
      p_comp->rnr_hysterisis_info.prev_lux_value,
      p_comp->rnr_hysterisis_info.prev_gain_value,
      p_comp->rnr_hysterisis_info.s_rnr_enabled,
      p_comp->rnr_hysterisis_info.prev_sampling_factor
      );
    if ((p_comp->rnr_hysterisis_info.prev_lux_value != 0) ||
      (p_comp->rnr_hysterisis_info.prev_gain_value != 0)) {
        cac3_comp_get_sampling_factor(p_comp, chromatix_rnr, trigger);
        //If hysterisis fails, get sampling factor from chromatix.
        if (p_comp->rnr_chromatix_info.sampling_factor == 0) {
          p_comp->rnr_chromatix_info.sampling_factor =
            chromatix_rnr->rnr_data[regionStart].sampling_factor;
        }
    } else {
      p_comp->rnr_chromatix_info.sampling_factor =
        chromatix_rnr->rnr_data[regionStart].sampling_factor;
    }
    p_comp->rnr_hysterisis_info.prev_sampling_factor =
      p_comp->rnr_chromatix_info.sampling_factor;
#else
    IDBG_MED("%s:%d: Hysterisis not enabled", __func__, __LINE__);
    p_comp->rnr_chromatix_info.sampling_factor =
      chromatix_rnr->rnr_data[regionStart].sampling_factor;
#endif
  } else {
    p_comp->skin_rnr_chromatix_info.s_rnr_enabled = FALSE;
  }
  IDBG_HIGH("%s %d: Final s_rnr_enabled %d,",
    __func__, __LINE__, p_comp->skin_rnr_chromatix_info.s_rnr_enabled);
#else
  //use default values if chromatix is not enabled
  rc = cac3_comp_get_default_rnr_params(p_comp, p_frame);
#endif
  return rc;
}

/**
 * Function: cac3_comp_get_cac_params
 *
 * Description: This function is to update the CAC parameters
 * for cac v2 from chromatix header
 *
 * Arguments:
 *   @p_comp: cac component
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int cac3_comp_get_cac_params(cac3_comp_t *p_comp)
{
#ifdef USE_CHROMATIX
  chromatix_sw_postproc_type *chromatix = NULL;
  chromatix_CAC2_type *chromatix_cac;
  interpolation_info_t interp_info;
  float trigger_start[MAX_LIGHT_TYPES_FOR_SPATIAL];
  float trigger_end[MAX_LIGHT_TYPES_FOR_SPATIAL];
  int regionStart = 0, regionEnd = 0, i = 0;
  float interp_ratio = 0;

  memset(&interp_info, 0, sizeof(interpolation_info_t));

  chromatix = (chromatix_sw_postproc_type *)p_comp->chromatix_ptr;

  //If CDS is on, CAC is off
  if (p_comp->cds_enable_flag) {
    p_comp->cac_enable_flag = FALSE;
    IDBG_HIGH("%s %d: CAC disabled. CDS = %d", __func__, __LINE__,
      p_comp->cds_enable_flag);
  } else if ((!p_comp->cds_enable_flag) &&
    (p_comp->skin_rnr_chromatix_info.skin_chroma_ds_factor > 1)) {
      IDBG_HIGH("%s %d: CAC disabled. CDS = %d, skin_chroma_ds_factor %d",
      __func__, __LINE__, p_comp->cds_enable_flag,
      p_comp->skin_rnr_chromatix_info.skin_chroma_ds_factor);
    p_comp->cac_enable_flag = FALSE;
  } else {
    p_comp->cac_enable_flag = TRUE;
  }

  chromatix_cac = &(chromatix->chromatix_CAC2_data);
  interp_info.trigger_start = trigger_start;
  interp_info.trigger_end = trigger_end;
  interp_info.trigger =
    (chromatix_cac->control_cac2 == 0) ? (float)p_comp->img_3A_info.lux_idx
    : (float)p_comp->img_3A_info.gain;

  IDBG_MED("%s:lux_idx %f, gain %f chromatix_cac3->control_CAC3 %d",
    __func__, p_comp->img_3A_info.lux_idx, p_comp->img_3A_info.gain,
    chromatix_cac->control_cac2);
  IDBG_MED("%s:Trigger %f", __func__, interp_info.trigger);

  for (i = 0; i < MAX_LIGHT_TYPES_FOR_SPATIAL; i++) {
    if (chromatix_cac->control_cac2 == 0) { //lux index
      interp_info.trigger_start[i] =
        chromatix_cac->cac2_data[i].cac2_trigger.lux_index_start;
      interp_info.trigger_end[i] =
        chromatix_cac->cac2_data[i].cac2_trigger.lux_index_end;
    } else { //gain
      interp_info.trigger_start[i] =
        chromatix_cac->cac2_data[i].cac2_trigger.gain_start;
      interp_info.trigger_end[i] =
        chromatix_cac->cac2_data[i].cac2_trigger.gain_end;
    }
  }
  interp_info.num_of_regions = MAX_LIGHT_TYPES_FOR_SPATIAL;
  cac3_comp_get_interpolation_regions(&interp_info);

  //Enable CAC per region enable flag
  p_comp->cac_enable_flag =
    (chromatix_cac->cac2_data[interp_info.enable_idx].cac2_enable == 1) ?
    TRUE : FALSE;

  regionStart = interp_info.regionStart;
  regionEnd = interp_info.regionEnd;
  interp_ratio = interp_info.interp_ratio;

  IDBG_HIGH("%s %d: region start %d regionend %d cac3_enable %d",
    __func__, __LINE__,
    regionStart, regionEnd, p_comp->cac_enable_flag);

  p_comp->cac_chromatix_info.detection_th1 =
    Round(BILINEAR_INTERPOLATION (
    chromatix_cac->cac2_data[regionStart].y_spot_thr_low,
    chromatix_cac->cac2_data[regionEnd].y_spot_thr_low, interp_ratio));
  p_comp->cac_chromatix_info.detection_th2 =
    Round(BILINEAR_INTERPOLATION (
    chromatix_cac->cac2_data[regionStart].y_saturation_thr,
    chromatix_cac->cac2_data[regionEnd].y_saturation_thr, interp_ratio));
  p_comp->cac_chromatix_info.verification_th1 =
    Round(BILINEAR_INTERPOLATION(
    chromatix_cac->cac2_data[regionStart].c_saturation_thr,
    chromatix_cac->cac2_data[regionEnd].c_saturation_thr, interp_ratio));
  p_comp->cac_chromatix_info.verification_th2 =
    Round(BILINEAR_INTERPOLATION(
    chromatix_cac->cac2_data[regionStart].c_spot_thr,
    chromatix_cac->cac2_data[regionEnd].c_spot_thr, interp_ratio));

  IDBG_HIGH("%s %d: detection_th1 %d detection_th2 %d",__func__, __LINE__,
    p_comp->cac_chromatix_info.detection_th1,
    p_comp->cac_chromatix_info.detection_th2);

  IDBG_HIGH("%s %d: verification_th1 %d verification_th2 %d",__func__, __LINE__,
    p_comp->cac_chromatix_info.verification_th1,
    p_comp->cac_chromatix_info.verification_th2);

#else
  p_comp->cac_enable_flag = TRUE;
  p_comp->cac_chromatix_info.detection_th1 = 10;
  p_comp->cac_chromatix_info.detection_th2 = 240;
  p_comp->cac_chromatix_info.verification_th1 = 15;
  p_comp->cac_chromatix_info.verification_th2 = 12;
#endif

  return IMG_SUCCESS;
}

/**
 * Function: cac3_comp_get_config
 *
 * Description: Get the CAC and RNR config
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
int cac3_comp_get_config(cac3_comp_t *p_comp, img_frame_t *p_frame)
{
  int rc = IMG_SUCCESS;

  if (!p_comp->cds_enable_flag) {
    p_comp->cac_enable_flag = TRUE;
  } else {
    p_comp->cac_enable_flag = FALSE;
  }

  if (p_comp->cac_enable_flag) {
    rc = cac3_comp_get_cac_params(p_comp);
    if (rc != IMG_SUCCESS){
      IDBG_ERROR("%s %d: Error configureing CAC params, disabling CAC rc=%d",
        __func__, __LINE__, rc);
      p_comp->cac_enable_flag = FALSE;
    }
  }

  rc = cac3_comp_get_rnr_params(p_comp, p_frame);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s %d: Error configureing RNR params, disabling RNR, rc=%d",
      __func__, __LINE__, rc);
    p_comp->rnr_enable_flag = FALSE;
  }

  // Overrirde CAC, RNR, SkinRNR flags and set as per setprop
  if (cac3_config != 0) {
    p_comp->cac_enable_flag &= ((cac3_config & (1 << CAC3_CAC_FLAG)) > 0);
    p_comp->rnr_enable_flag &= ((cac3_config & (1 << CAC3_RNR_FLAG)) > 0);
    p_comp->skin_rnr_chromatix_info.s_rnr_enabled &=
      ((cac3_config & (1 << CAC3_SRNR_FLAG)) > 0);
    IDBG_ERROR("%s %d: CAC config cac_flg = %d rnr_flg = %d skinrnr_flg = %d",
      __func__, __LINE__,
      p_comp->cac_enable_flag,
      p_comp->rnr_enable_flag,
      p_comp->skin_rnr_chromatix_info.s_rnr_enabled);
  }

  if ((!p_comp->cds_enable_flag) && (p_comp->lds_enable_flag)) {
  /*If cds is disabled and lds is enabled, there is a mismatch in metadeta.
      Disable LDS */
    p_comp->lds_enable_flag = FALSE;
  } else if (p_comp->cac_enable_flag && p_comp->lds_enable_flag) {
  /*If cac is enabled and lds is enabled, there is a mismatch in metadeta.
      Disable LDS */
    p_comp->lds_enable_flag = FALSE;
  }
  IMG_UNUSED(p_frame);
  return rc;
}

/**
 * Function: cac3_comp_process_frame
 *
 * Description: Run the cac algorithm on the given frame
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
int cac3_comp_process_frame(cac3_comp_t *p_comp, img_frame_t *p_frame)
{
  int rc = IMG_SUCCESS;
  img_component_t *p_base = &p_comp->b;
  cac3_args_t cac_rnr_params;

  memset(&cac_rnr_params, 0x0, sizeof(cac3_args_t));

  pthread_mutex_lock(&p_base->mutex);

  cac3_comp_get_config(p_comp, p_frame);

  cac_rnr_params.userdata = p_comp->userdata;
  cac_rnr_params.cac3_cb = p_comp->cac3_cb;
  //Fill in the image dimensions
  cac_rnr_params.p_ion_vaddr = p_frame->frame[0].plane[IY].addr;
  cac_rnr_params.p_y = p_frame->frame[0].plane[IY].addr +
    p_frame->frame[0].plane[IY].offset;
  cac_rnr_params.fd = p_frame->frame[0].plane[IY].fd;
  cac_rnr_params.p_crcb = p_frame->frame[0].plane[IC].addr +
    p_frame->frame[0].plane[IC].offset;
  cac_rnr_params.y_width = p_frame->info.width;
  cac_rnr_params.y_height = p_frame->info.height;
  cac_rnr_params.y_stride = p_frame->frame[0].plane[IY].stride;

  if (p_comp->yuyv422_to_yuv420_enable_flag){
    cac_rnr_params.y_out_stride =
      PAD_TO_SIZE((p_frame->frame[0].plane[0].width), 16);
    cac_rnr_params.cbcr_out_stride =
      PAD_TO_SIZE((p_frame->frame[0].plane[0].width), 16);
    cac_rnr_params.cbcr_stride = p_frame->frame[0].plane[0].stride;
    cac_rnr_params.p_crcb = p_frame->frame[0].plane[0].addr +
      (cac_rnr_params.y_out_stride * p_frame->frame[0].plane[0].scanline);
    IDBG_MED("%s:%d] y_out_stride=%u, cbcr_out_stride=%u, p_crcb=%p",
      __func__, __LINE__,
      cac_rnr_params.y_out_stride,
      cac_rnr_params.cbcr_out_stride,
      cac_rnr_params.p_crcb);

    if (p_comp->cds_enable_flag) {
        cac_rnr_params.cbcr_width =
          CEILING8(p_frame->frame[0].plane[0].width / 2);
        cac_rnr_params.cbcr_height =
          CEILING8(p_frame->frame[0].plane[0].height / 2);
        cac3_comp_configure_buffer1(p_frame, cac_rnr_params.p_y);
    } else {
      cac_rnr_params.cbcr_width = p_frame->frame[0].plane[0].width;
      cac_rnr_params.cbcr_height = p_frame->frame[0].plane[0].height;
    }
  } else {
    if (p_comp->cds_enable_flag) {
      cac_rnr_params.cbcr_width =
        CEILING8(p_frame->frame[0].plane[IC].width / 2);
      cac_rnr_params.cbcr_height =
        CEILING8(p_frame->frame[0].plane[IC].height / 2);
      cac3_comp_configure_buffer(p_frame, IC, cac_rnr_params.cbcr_width,
        cac_rnr_params.cbcr_height);
    } else {
      cac_rnr_params.cbcr_width = p_frame->frame[0].plane[IC].width;
      cac_rnr_params.cbcr_height = p_frame->frame[0].plane[IC].height;
    }
    cac_rnr_params.cbcr_stride = p_frame->frame[0].plane[IC].stride;
  }

  cac_rnr_params.image_format = p_comp->chroma_order;
#if defined(USE_RESERVED_MEM)
  cac_rnr_params.ion_heap_id = ION_CAMERA_HEAP_ID;
#else
  cac_rnr_params.ion_heap_id = ION_SYSTEM_HEAP_ID;
#endif

  //Fill in the CAC parameters
  if (p_comp->cac_enable_flag) {
    cac_rnr_params.detection_th1 =
      p_comp->cac_chromatix_info.detection_th1;
    cac_rnr_params.detection_th2 =
      p_comp->cac_chromatix_info.detection_th2;
    cac_rnr_params.verification_th1 =
      p_comp->cac_chromatix_info.verification_th1;
    cac_rnr_params.verification_th2 =
      p_comp->cac_chromatix_info.verification_th2;
  }
  cac_rnr_params.cac3_enable_flag = p_comp->cac_enable_flag;

  //Fill in the RNR parameters
  if (p_comp->rnr_enable_flag) {
    cac_rnr_params.sampling_factor =
      p_comp->rnr_chromatix_info.sampling_factor;
    cac_rnr_params.sigma_lut = p_comp->rnr_chromatix_info.sigma_lut;
    cac_rnr_params.lut_size = p_comp->rnr_chromatix_info.lut_size;
    cac_rnr_params.scale_factor = p_comp->rnr_chromatix_info.scale_factor;
    cac_rnr_params.center_noise_sigma =
      p_comp->rnr_chromatix_info.center_noise_sigma;
    cac_rnr_params.center_noise_weight =
      p_comp->rnr_chromatix_info.center_noise_weight;
    cac_rnr_params.weight_order = p_comp->rnr_chromatix_info.weight_order;
  }
  cac_rnr_params.rnr_enable_flag = p_comp->rnr_enable_flag;

  //Fill in the skin RNR params
  if (p_comp->skin_rnr_chromatix_info.s_rnr_enabled) {
    cac_rnr_params.skin_sigma_lut =
      p_comp->skin_rnr_chromatix_info.skin_sigma_lut;
    cac_rnr_params.skin_sigma_lut_size =
      p_comp->skin_rnr_chromatix_info.skin_sigma_lut_size;
    cac_rnr_params.skin_strength =
      p_comp->skin_rnr_chromatix_info.skin_strength;
    cac_rnr_params.skin_cr =
      p_comp->skin_rnr_chromatix_info.skin_cr;
    cac_rnr_params.skin_cb =
      p_comp->skin_rnr_chromatix_info.skin_cb;
    cac_rnr_params.skin_ymin =
      p_comp->skin_rnr_chromatix_info.skin_ymin;
    cac_rnr_params.skin_ymax =
      p_comp->skin_rnr_chromatix_info.skin_ymax;
    cac_rnr_params.skin_sigma_scale =
      p_comp->skin_rnr_chromatix_info.skin_sigma_scale;
    cac_rnr_params.skin_scaler =
      p_comp->skin_rnr_chromatix_info.skin_scaler;
    cac_rnr_params.skin_threshold =
      p_comp->skin_rnr_chromatix_info.skin_threshold;
  }
  cac_rnr_params.skin_rnr_enable_flag =
    p_comp->skin_rnr_chromatix_info.s_rnr_enabled;

   p_comp->skin_rnr_chromatix_info.ds_us_skin_detection =
     cac3_comp_get_skin_ds_factor(p_comp);
   cac_rnr_params.ds_us_skin_detection =
     p_comp->skin_rnr_chromatix_info.ds_us_skin_detection;

  //Set LDS and CDS flag
  cac_rnr_params.lds_enable_flag = p_comp->lds_enable_flag;
  cac_rnr_params.cds_enable_flag = p_comp->cds_enable_flag;
  cac_rnr_params.yuv420_to_yuv422 = 0;
  cac_rnr_params.yuyv422_to_yuv420 = p_comp->yuyv422_to_yuv420_enable_flag;

  pthread_mutex_unlock(&p_base->mutex);

  //Print Params - To debug
  cac3_comp_lib_debug(&cac_rnr_params);

  //Fill in metadata is enabled
  if (p_comp->b.debug_info.camera_dump_enabled) {
    rc = cac3_comp_fill_metadata(&cac_rnr_params, p_comp);
    if (IMG_SUCCESS != rc) {
      IDBG_HIGH("%s:%d] Fill metadata error", __func__, __LINE__);
    }
  }

  IDBG_INFO("%s:%d] Start CAC/RNR/LDS", __func__, __LINE__);
  rc = g_cac_lib.cac3_process(&cac_rnr_params);

  pthread_mutex_lock(&p_base->mutex);
  p_comp->b.state = IMG_STATE_IDLE;
  pthread_mutex_unlock(&p_base->mutex);

  if (rc) {
    IDBG_ERROR("%s:%d] CAC failed", __func__, __LINE__);
  } else {
    IDBG_INFO("%s:%d] CAC/RNR/LDS Successful", __func__, __LINE__);
  }

  return rc;
}

/**
 * Function: cac3_thread_loop
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
void *cac3_thread_loop(void *handle)
{
  cac3_comp_t *p_comp = (cac3_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_frame_t *p_frame = NULL;
  int i = 0, count;
  IDBG_MED("%s:%d] ", __func__, __LINE__);

  count = img_q_count(&p_base->inputQ);
  IDBG_MED("%s:%d] num buffers %d", __func__, __LINE__, count);

  for (i = 0; i < count; i++) {
    p_frame = img_q_dequeue(&p_base->inputQ);
    if (NULL == p_frame) {
      IDBG_ERROR("%s:%d] invalid buffer", __func__, __LINE__);
      goto error;
    }
    /*process the frame*/
    status = cac3_comp_process_frame(p_comp, p_frame);
    if (status < 0) {
      IDBG_ERROR("%s:%d] process error %d", __func__, __LINE__, status);
      goto error;
    }

    /*enque the frame to the output queue*/
    status = img_q_enqueue(&p_base->outputQ, p_frame);
    if (status < 0) {
      IDBG_ERROR("%s:%d] enqueue error %d", __func__, __LINE__, status);
      goto error;
    }
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
  IDBG_MED("%s:%d] Error buf count %d", __func__, __LINE__, count);

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
 * Function: cac3_comp_abort
 *
 * Description: Aborts the execution of CAC
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type is defined in cac.h
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int cac3_comp_abort(void *handle, void *p_data)
{
  cac3_comp_t *p_comp = (cac3_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status;

  if (p_base->mode == IMG_ASYNC_MODE) {
    status = p_comp->b.ops.abort(&p_comp->b, p_data);
    if (status < 0) {
      return status;
    }
  }
  pthread_mutex_lock(&p_base->mutex);
  p_base->state = IMG_STATE_INIT;
  pthread_mutex_unlock(&p_base->mutex);

  return 0;
}

/**
 * Function: cac3_comp_process
 *
 * Description: This function is used to send any specific commands for the
 *              CAC component
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
int cac3_comp_process(void *handle, img_cmd_type cmd, void *p_data)
{
  cac3_comp_t *p_comp = (cac3_comp_t *)handle;
  int status;

  status = p_comp->b.ops.process(&p_comp->b, cmd, p_data);
  if (status < 0) {
    return status;
  }

  return 0;
}

/**
 * Function: cac3_comp_start
 *
 * Description: Start the execution of CAC
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   p_data - The pointer to the command structure. The structure
 *            for each command type will be defined in cac.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int cac3_comp_start(void *handle, void *p_data)
{
  cac3_comp_t *p_comp = (cac3_comp_t *)handle;
  img_component_t *p_base = &p_comp->b;
  int status = IMG_SUCCESS;
  img_frame_t *p_frame;

  pthread_mutex_lock(&p_base->mutex);
  if ((p_base->state != IMG_STATE_INIT) ||
    (NULL == p_base->thread_loop)) {
    IDBG_ERROR("%s:%d] Error state %d", __func__, __LINE__,
      p_base->state);
    pthread_mutex_unlock(&p_base->mutex);
    return IMG_ERR_NOT_SUPPORTED;
  }

  p_base->state = IMG_STATE_STARTED;
  pthread_mutex_unlock(&p_base->mutex);

  if (p_base->mode == IMG_SYNC_MODE) {
    p_frame = img_q_dequeue(&p_base->inputQ);
    if (NULL == p_frame) {
      IDBG_ERROR("%s:%d] invalid buffer", __func__, __LINE__);
      status = IMG_ERR_INVALID_INPUT;
      goto error;
    }
    status = cac3_comp_process_frame(p_comp, p_frame);
  } else {
    status = p_comp->b.ops.start(&p_comp->b, p_data);
  }

error:
  if (status != IMG_SUCCESS) {
    pthread_mutex_lock(&p_base->mutex);
    p_base->state = IMG_STATE_INIT;
    pthread_mutex_unlock(&p_base->mutex);
  }

  return status;
}

/**
 * Function: cac3_comp_get_param
 *
 * Description: Gets CAC parameters
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
int cac3_comp_get_param(void *handle, img_param_type param, void *p_data)
{
  cac3_comp_t *p_comp = (cac3_comp_t *)handle;
  int status = IMG_SUCCESS;

  status = p_comp->b.ops.get_parm(&p_comp->b, param, p_data);
  if (status < 0) {
    return status;
  }

  return IMG_SUCCESS;
}

/**
 * Function: cac3_comp_set_param
 *
 * Description: Set CAC parameters
 *
 * Input parameters:
 *   handle - The pointer to the component handle.
 *   param - The type of the parameter
 *   p_data - The pointer to the paramter structure. The structure
 *            for each paramter type will be defined in cac.h
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_INVALID_OPERATION
 *     IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int cac3_comp_set_param(void *handle, img_param_type param, void *p_data)
{
  cac3_comp_t *p_comp = (cac3_comp_t *)handle;
  int status = IMG_SUCCESS;

  status = p_comp->b.ops.set_parm(&p_comp->b, param, p_data);
  if (status < 0) {
    return status;
  }

  switch (param) {
    case QCAC_CHROMATIX_INFO : {
      void *l_chromatix = (void *)p_data;
      if (NULL == l_chromatix) {
        IDBG_ERROR("%s:%d] invalid chromatix info", __func__, __LINE__);
        return IMG_ERR_INVALID_INPUT;
      }
      p_comp->chromatix_ptr = l_chromatix;
      IDBG_LOW("%s:%d] chromatix info %p", __func__, __LINE__,
        p_comp->chromatix_ptr);
      break;
    }

    case QCAC_CHROMA_ORDER: {
      cac_chroma_order *l_chroma_order = (cac_chroma_order *)p_data;
      if (NULL == l_chroma_order) {
        IDBG_ERROR("%s:%d] invalid chroma order info", __func__, __LINE__);
        return IMG_ERR_INVALID_INPUT;
      }
      p_comp->chroma_order = *l_chroma_order;
      break;
    }

    case QLDS_ENABLED: {
      uint8_t *l_lds_enabled = (uint8_t *)p_data;
      if (NULL == l_lds_enabled) {
        IDBG_ERROR("%s:%d] invalid rnr enabled info", __func__, __LINE__);
        return IMG_ERR_INVALID_INPUT;
      }
      p_comp->lds_enable_flag = *l_lds_enabled;
      break;
    }

    case QCDS_ENABLED: {
      uint8_t *l_cds_enabled = (uint8_t *)p_data;
      if (NULL == l_cds_enabled) {
        IDBG_ERROR("%s:%d] invalid rnr enabled info", __func__, __LINE__);
        return IMG_ERR_INVALID_INPUT;
      }
      p_comp->cds_enable_flag = *l_cds_enabled;
      break;
    }

    case QYUYV422_TO_420_ENABLED: {
      uint8_t *l_yuyv422_to_420_enabled = (uint8_t *)p_data;
      if (NULL == l_yuyv422_to_420_enabled) {
        IDBG_ERROR("%s:%d] invalid rnr enabled info", __func__, __LINE__);
        return IMG_ERR_INVALID_INPUT;
      }
      p_comp->yuyv422_to_yuv420_enable_flag= *l_yuyv422_to_420_enabled;
      break;
    }

    case QCAC_3A_INFO: {
      img_3A_data_t *l_img_3A_data = (img_3A_data_t *)p_data;
      if (NULL == l_img_3A_data) {
        IDBG_ERROR("%s:%d] invalid 3A info", __func__, __LINE__);
        return IMG_ERR_INVALID_INPUT;
      }
      p_comp->img_3A_info = *l_img_3A_data;
      break;
    }

    case QFD_RESULT: {
      face_detection_data_t *l_fd_data = (face_detection_data_t *)p_data;
      if (NULL == l_fd_data) {
        IDBG_ERROR("%s:%d] invalid 3A info", __func__, __LINE__);
        return IMG_ERR_INVALID_INPUT;
      }
      p_comp->fd_data = *l_fd_data;
      break;
    }

    case QIMG_PARAM_MODE:
    case QIMG_CAMERA_DUMP:
    case QCAC_RGAMMA_TABLE:
    case QCAC_GGAMMA_TABLE:
    case QCAC_BGAMMA_TABLE:
      break;

    default: {
      IDBG_ERROR("%s:%d] invalid parameter %d", __func__, __LINE__, param);
      return IMG_ERR_INVALID_INPUT;
    }
  }
  return status;
}

/**
 * Function: cac3_comp_deinit
 *
 * Description: Deinitializes the CAC component
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
int cac3_comp_deinit(void *handle)
{
  cac3_comp_t *p_comp = (cac3_comp_t *)handle;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] \n", __func__, __LINE__);
  status = cac3_comp_abort(handle, NULL);
  if (status < 0) {
    return status;
  }

  if (p_comp->p_y_buffer) {
    free(p_comp->p_y_buffer);
    p_comp->p_y_buffer = NULL;
  }
  if (p_comp->p_c_buffer) {
    free(p_comp->p_c_buffer);
    p_comp->p_c_buffer = NULL;
  }
  status = p_comp->b.ops.deinit(&p_comp->b);
  if (status < 0) {
    return status;
  }

  free(p_comp);
  return IMG_SUCCESS;
}

/**
 * Function: cac3_comp_create
 *
 * Description: This function is used to create CAC component
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
int cac3_comp_create(void* handle, img_component_ops_t *p_ops)
{
  IMG_UNUSED(handle);

  cac3_comp_t *p_comp = NULL;
  int status;
  cac3_config = 0;

#ifdef __ANDROID__
  char prop[PROPERTY_VALUE_MAX];
  property_get("persist.vendor.camera.imglib.cac3", prop, "0");
  cac3_config = (uint32_t)atoi(prop);
#endif

  if (NULL == g_cac_lib.ptr) {
    IDBG_ERROR("%s:%d] library not loaded", __func__, __LINE__);
    return IMG_ERR_INVALID_OPERATION;
  }

  if (NULL == p_ops) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  p_comp = (cac3_comp_t *)malloc(sizeof(cac3_comp_t));
  if (NULL == p_comp) {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  memset(p_comp, 0x0, sizeof(cac3_comp_t));
  status = img_comp_create(&p_comp->b);
  if (status < 0) {
    free(p_comp);
    return status;
  }

  /*set the main thread*/
  p_comp->b.thread_loop = cac3_thread_loop;
  p_comp->b.p_core = p_comp;

  /* copy the ops table from the base component */
  *p_ops = p_comp->b.ops;
  p_ops->init            = cac3_comp_init;
  p_ops->deinit          = cac3_comp_deinit;
  p_ops->set_parm        = cac3_comp_set_param;
  p_ops->get_parm        = cac3_comp_get_param;
  p_ops->start           = cac3_comp_start;
  p_ops->abort           = cac3_comp_abort;
  p_ops->process         = cac3_comp_process;

  p_ops->handle = (void *)p_comp;
  return IMG_SUCCESS;
}

/**
 * Function: cac3_comp_load
 *
 * Description: This function is used to load CAC library
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
int cac3_comp_load(const char* name, void** handle)
{
  IMG_UNUSED(name);
  IMG_UNUSED(handle);

  int rc = IMG_SUCCESS;
  if (g_cac_lib.ptr) {
    IDBG_ERROR("%s:%d] library already loaded", __func__, __LINE__);
    return IMG_ERR_NOT_FOUND;
  }

  g_cac_lib.ptr = dlopen("libmmcamera_cac3_lib.so", RTLD_NOW);
  if (!g_cac_lib.ptr) {
    IDBG_ERROR("%s:%d] Error opening CAC library", __func__, __LINE__);
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(g_cac_lib.cac3_process) =
    dlsym(g_cac_lib.ptr, "cac3_process");
  if (!g_cac_lib.cac3_process) {
    IDBG_ERROR("%s:%d] Error linking camera CAC module start",
      __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(g_cac_lib.cac3_init) =
    dlsym(g_cac_lib.ptr, "cac3_init");
  if (!g_cac_lib.cac3_init) {
    IDBG_ERROR("%s:%d] Error linking cac_module_init",
      __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }
  *(void **)&(g_cac_lib.cac3_deinit) =
    dlsym(g_cac_lib.ptr, "cac3_deinit");
  if (!g_cac_lib.cac3_deinit) {
    IDBG_ERROR("%s:%d] Error linking cac_module_deinit",
      __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(g_cac_lib.cac3_pre_allocate_buffers) =
    dlsym(g_cac_lib.ptr, "cac3_pre_allocate_buffers");
  if (!g_cac_lib.cac3_pre_allocate_buffers) {
    IDBG_ERROR("%s:%d] Error linking pre_allocate_buffers",
      __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  *(void **)&(g_cac_lib.cac3_destroy_buffers) =
    dlsym(g_cac_lib.ptr, "cac3_destroy_buffers");
  if (!g_cac_lib.cac3_destroy_buffers) {
    IDBG_ERROR("%s:%d] Error linking pre_allocate_buffers",
      __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

#if defined(USE_RESERVED_MEM)
  rc = g_cac_lib.cac3_init(ION_CAMERA_HEAP_ID);
#else
  rc = g_cac_lib.cac3_init(ION_SYSTEM_HEAP_ID);
#endif
  if (rc) {
    IDBG_ERROR("%s:%d] CAC lib module init failed", __func__, __LINE__);
    dlclose(g_cac_lib.ptr);
    g_cac_lib.ptr = NULL;
    return IMG_ERR_NOT_FOUND;
  }

  IDBG_HIGH("%s:%d] CAC library loaded successfully", __func__, __LINE__);

  return rc;
}

 /**
* Function: cac3_comp_alloc
*
* Description: This function is used to Allocate buffers
* library
*
* Input parameters:
 *   @handle: pointer to the core ops structure
 *   @p_params: pointer to the alloc params
*
* Return values:
*     standard imglib ret codes
*
* Notes: none
**/
int cac3_comp_alloc(void *handle, void *p_params)
{
  int rc = IMG_SUCCESS;

  IMG_UNUSED(handle);
  IMG_UNUSED(p_params);
  IDBG_HIGH("%s:%d] ptr %p", __func__, __LINE__, g_cac_lib.ptr);

  if (g_cac_lib.ptr) {
#if defined(USE_RESERVED_MEM)
    rc = g_cac_lib.cac3_pre_allocate_buffers(ION_CAMERA_HEAP_ID);
#else
    rc = g_cac_lib.cac3_pre_allocate_buffers(ION_SYSTEM_HEAP_ID);
#endif
    if (rc) {
      IDBG_ERROR("%s:%d] CAC lib allocate buffers failed", __func__, __LINE__);
      dlclose(g_cac_lib.ptr);
      g_cac_lib.ptr = NULL;
      return IMG_ERR_NOT_FOUND;
    }
  }
  return rc;
}

/**
 * Function: cac3_comp_dealloc
 *
 * Description: This function is used to DeAllocate buffers
 * library
 *
 * Input parameters:
 *   @handle: pointer to the handle
 *
 * Return values:
 *     standard imglib ret codes
 *
 * Notes: none
 **/
int cac3_comp_dealloc(void *handle)
{
  int rc = IMG_SUCCESS;
  IMG_UNUSED(handle);
  IDBG_HIGH("%s:%d] ptr %p", __func__, __LINE__, g_cac_lib.ptr);

  if (g_cac_lib.ptr) {
    rc = g_cac_lib.cac3_destroy_buffers();
    if (rc) {
      IDBG_ERROR("%s:%d] CAC lib deallocate buffers failed", __func__,
        __LINE__);
      dlclose(g_cac_lib.ptr);
      g_cac_lib.ptr = NULL;
      return IMG_ERR_NOT_FOUND;
    }
  }
  return rc;
}

/**
 * Function: cac3_comp_unload
 *
 * Description: This function is used to unload CAC library
 *
 * Input parameters:
 *   @handle: library handle
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void cac3_comp_unload(void* handle)
{
  IMG_UNUSED(handle);

  int rc = 0;
  IDBG_HIGH("%s:%d] ptr %p", __func__, __LINE__, g_cac_lib.ptr);

  if (g_cac_lib.ptr) {
    rc = g_cac_lib.cac3_deinit();
    if (rc) {
      IDBG_ERROR("%s:%d] CAC lib module deinit failed", __func__, __LINE__);
    }
    rc = dlclose(g_cac_lib.ptr);
    if (rc < 0) {
      IDBG_HIGH("%s:%d] error %s", __func__, __LINE__, dlerror());
      g_cac_lib.ptr = NULL;
    }
  }
}
