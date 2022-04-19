/**********************************************************************
*  Copyright (c) 2013-2019 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "img_common.h"
#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "img_meta.h"
#include <cutils/properties.h>
#include <stdlib.h>
#include <arm_neon.h>
#include <gralloc_priv.h>
#ifdef __ANDROID__
#ifdef USE_PERF_API
#include "mp-ctl.h"
#include <dlfcn.h>
#endif
#endif

#ifdef __CAM_FCV__
#include <fastcv/fastcv.h>
#endif //__CAM_FCV__
#include "cam_types.h"

#ifdef TARGET_ION_ABI_VERSION
#include <linux/dma-buf.h>
#endif

#define Q12 4096
#define MN_DIVISION_Q_BITS 10
/**
 * Total number of token to be parsed per line
 */
#define MAX_PARSE_TOKEN 3

/** IMG_SCALE_RECT:
 *  @p_in_rect: input region
 *  @p_out_rect: output region
 *  @factor scale factor
 *
 *  Scale the region based on the scale factor
 **/
#define IMG_SCALE_RECT(p_in_rect, p_out_rect, factor) ({\
  p_out_rect->pos.x = (int)(p_in_rect->pos.x * factor); \
  p_out_rect->pos.y = (int)(p_in_rect->pos.y * factor); \
  p_out_rect->size.width = (int)(p_in_rect->size.width * factor); \
  p_out_rect->size.height = (int)(p_in_rect->size.height * factor); \
})

/** IMG_GET_ZOOM_IDX:
 *  @tbl: zoom table
 *  @size: size of the table
 *  @val: value in Q12 format
 *
 *  Find the zoom index on the zoom ratio table
 **/
#define IMG_GET_ZOOM_IDX(tbl, size, val) ({ \
  uint32_t i = 0; \
  for (i = 0; i < size; i++) { \
    if (tbl[i] >= val) \
      break; \
  } \
  if (i >= size) \
    i = size - 1; \
  i; \
})

extern void ds_2by2_asm(uint8_t *pSrc, const uint32_t srcWidth,
  const uint32_t srcHeight, const uint32_t srcStride, uint8_t *pDst,
  const uint32_t dstStride);

/**sigma_lut_in
  * Default sigma table for RNR under nornal lighting conditions
**/
 float sigma_lut_in[RNR_LUT_SIZE] = {
  1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f,
  1.0000f, 1.0000f, 1.0000f, 1.0000f,
  1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0000f, 1.0015f,
  1.0030f, 1.0045f, 1.0059f, 1.0074f,
  1.0089f, 1.0104f, 1.0119f, 1.0134f, 1.0149f, 1.0164f, 1.0178f, 1.0193f,
  1.0208f, 1.0223f, 1.0238f, 1.0253f,
  1.0268f, 1.0283f, 1.0297f, 1.0312f, 1.0327f, 1.0342f, 1.0357f, 1.0372f,
  1.0387f, 1.0402f, 1.0416f, 1.0430f,
  1.0444f, 1.0459f, 1.0473f, 1.0487f, 1.0502f, 1.0516f, 1.0530f, 1.0545f,
  1.0559f, 1.0573f, 1.0588f, 1.0602f,
  1.0616f, 1.0630f, 1.0645f, 1.0659f, 1.0673f, 1.0688f, 1.0702f, 1.0716f,
  1.0731f, 1.0745f, 1.0759f, 1.0774f,
  1.0788f, 1.0802f, 1.0816f, 1.0831f, 1.0845f, 1.0859f, 1.0874f, 1.0888f,
  1.0902f, 1.1036f, 1.1170f, 1.1304f,
  1.1438f, 1.1572f, 1.1706f, 1.1840f, 1.1974f, 1.2108f, 1.2242f, 1.2376f,
  1.2510f, 1.2644f, 1.2778f, 1.2870f,
  1.2962f, 1.3054f, 1.3145f, 1.3237f, 1.3329f, 1.3421f, 1.3513f, 1.3604f,
  1.3696f, 1.3788f, 1.3880f, 1.3971f,
  1.4063f, 1.4155f, 1.4247f, 1.4339f, 1.4430f, 1.4522f, 1.4614f, 1.4706f,
  1.4798f, 1.4889f, 1.4981f, 1.5073f,
  1.5165f, 1.5256f, 1.5348f, 1.5440f, 1.5532f, 1.5637f, 1.5743f, 1.5848f,
  1.5954f, 1.6060f, 1.6165f, 1.6271f,
  1.6376f, 1.6482f, 1.6587f, 1.6693f, 1.6798f, 1.6904f, 1.7009f, 1.7115f,
  1.7220f, 1.7326f, 1.7432f, 1.7537f,
  1.7662f, 1.7786f, 1.7911f, 1.8035f, 1.8160f, 1.8285f, 1.8409f, 1.8534f,
  1.8658f, 1.8783f, 1.8907f, 1.9032f,
  1.9157f, 1.9281f, 1.9406f, 1.9530f, 1.9655f, 1.9780f, 1.9780f, 1.9780f
};

/** img_perf_handle_t
 *   @instance: performance lib instance
 *   @perf_lock_acq: performance lib acquire function
 *   @perf_lock_rel: performance lib release function
 *
 *   Performance Lib Handle
 **/
typedef struct {
  void* instance;
  int (*perf_lock_acq)(int handle, int duration,
    int list[], int numArgs);
  int (*perf_lock_rel)(int handle);
} img_perf_handle_t;

/** img_perf_lock_handle_t
 *   @instance: performance lock instance
 *
 *   Performance Lock Handle
 **/
typedef struct {
  int instance;
} img_perf_lock_handle_t;

/**
 * Function: img_get_subsampling_factor
 *
 * Description: gets the height and width subsampling factors
 *
 * Input parameters:
 *   ss_type - subsampling type
 *   p_w_factor - width factor
 *   p_h_factor - height factor
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int img_get_subsampling_factor(img_subsampling_t ss_type, float *p_w_factor,
  float *p_h_factor)
{
  switch (ss_type) {
  case IMG_H2V2:
    *p_w_factor = .5;
    *p_h_factor = .5;
    break;
  case IMG_H2V1:
    *p_w_factor = .5;
    *p_h_factor = 1.0;
    break;
  case IMG_H1V2:
    *p_w_factor = 1.0;
    *p_h_factor = .5;
    break;
  case IMG_H1V1:
    *p_w_factor = 1.0;
    *p_h_factor = 1.0;
    break;
  default:
    return IMG_ERR_INVALID_INPUT;
  }
  return IMG_SUCCESS;
}

/**
 * Function: img_wait_for_completion
 *
 * Description: Static function to wait until the timer has expired.
 *
 * Input parameters:
 *   p_cond - pointer to the pthread condition
 *   p_mutex - pointer to the pthread mutex
 *   ms - time in milli seconds
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_TIMEOUT
 *
 * Notes: none
 **/
int img_wait_for_completion(pthread_cond_t *p_cond, pthread_mutex_t *p_mutex,
  int32_t ms)
{
  struct timespec ts;
  int rc = clock_gettime(CLOCK_REALTIME, &ts);
  if (rc < 0)
    return IMG_ERR_GENERAL;

  if (ms >= 1000) {
    ts.tv_sec += (ms / 1000);
    ts.tv_nsec += ((ms % 1000) * 1000000);
  } else {
    ts.tv_nsec += (ms * 1000000);
  }

  rc = pthread_cond_timedwait(p_cond, p_mutex, &ts);
  if (rc == ETIMEDOUT) {
    rc = IMG_ERR_TIMEOUT;
  }
  return rc;
}

/** img_image_copy:
 *  @out_buff: output buffer handler
 *  @in_buff: input buffer handler
 *
 * Function to copy image data from source to destination buffer
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_image_copy(img_frame_t *out_buff, img_frame_t *in_buff)
{
  int ret_val = IMG_ERR_INVALID_INPUT;
  uint8_t *in_ptr;
  uint8_t *out_ptr;
  uint32_t i,j;

  if (out_buff->info.width == in_buff->info.width
    && out_buff->info.height == in_buff->info.height
    && out_buff->frame[0].plane_cnt ==
    in_buff->frame[0].plane_cnt) {

    ret_val = IMG_SUCCESS;
    for (i = 0; i < in_buff->frame[0].plane_cnt; i++) {
      if (out_buff->frame[0].plane[i].height !=
          in_buff->frame[0].plane[i].height
        || out_buff->frame[0].plane[i].width !=
          in_buff->frame[0].plane[i].width
        || out_buff->frame[0].plane[i].plane_type
          != in_buff->frame[0].plane[i].plane_type) {
        ret_val = IMG_ERR_INVALID_INPUT;
        break;
      }
    }
  }

  if (IMG_SUCCESS == ret_val) {
    for (i = 0; i < in_buff->frame[0].plane_cnt; i++) {
      out_ptr = out_buff->frame[0].plane[i].addr
        + out_buff->frame[0].plane[i].offset;
      in_ptr = in_buff->frame[0].plane[i].addr
        + in_buff->frame[0].plane[i].offset;
      for (j=0; j<in_buff->frame[0].plane[i].height; j++) {
        memcpy(out_ptr, in_ptr, (size_t)out_buff->frame[0].plane[i].width);
        out_ptr += out_buff->frame[0].plane[i].stride;
        in_ptr += in_buff->frame[0].plane[i].stride;
      }
    }
  } else {
    IDBG_ERROR("%s:%d failed: Output and input buffers are not compatible",
      __func__, __LINE__);
  }

  return ret_val;
}

/**
 * Function: img_translate_cordinates
 *
 * Description: Translate the region from one window
 *             dimension to another
 *
 * Input parameters:
 *   dim1 - dimension of 1st window
 *   dim2 - dimension of 2nd window
 *   p_in_region - pointer to the input region
 *   p_out_region - pointer to the output region
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes:  none
 **/
int img_translate_cordinates(img_size_t dim1, img_size_t dim2,
  img_rect_t *p_in_region, img_rect_t *p_out_region)
{
  double min_scale;
  if (!dim1.width || !dim1.height || !dim2.width || !dim2.height) {
    IDBG_ERROR("%s:%d] Error invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  if ((dim1.width < dim2.width) || (dim1.height < dim2.height)) {
    IDBG_MED("%s:%d] input greater than output", __func__, __LINE__);
    return IMG_ERR_NOT_SUPPORTED;
  }

  min_scale = MIN((double)dim1.width/(double)dim2.width,
    (double)dim1.height/(double)dim2.height);
  img_pixel_t temp_coodinate;
  img_size_t temp_dim;
  temp_dim.width = (int32_t)(dim1.width/min_scale);
  temp_dim.height = (int32_t)(dim1.height/min_scale);
  temp_coodinate.x = (temp_dim.width - dim2.width)/2;
  temp_coodinate.y = (temp_dim.height - dim2.height)/2;
  IDBG_MED("%s:%d] int dim %dx%d pos (%d %d)",
    __func__, __LINE__,
    temp_dim.width,
    temp_dim.height,
    temp_coodinate.x,
    temp_coodinate.y);
  p_out_region->pos.x = (int32_t)((p_in_region->pos.x + temp_coodinate.x) * min_scale);
  p_out_region->pos.y = (int32_t)((p_in_region->pos.y + temp_coodinate.y) * min_scale);
  p_out_region->size.width = (int32_t)(p_in_region->size.width * min_scale);
  p_out_region->size.height = (int32_t)(p_in_region->size.height * min_scale);

  IDBG_MED("%s:%d] dim1 %dx%d dim2 %dx%d in_reg (%d %d %d %d)"
    "out_reg (%d %d %d %d)",
    __func__, __LINE__,
    dim1.width, dim1.height, dim2.width, dim2.height,
    p_in_region->pos.x, p_in_region->pos.y,
    p_in_region->size.width, p_in_region->size.height,
    p_out_region->pos.x, p_out_region->pos.y,
    p_out_region->size.width, p_out_region->size.height);
  return IMG_SUCCESS;
}

/**
 * Function: img_translate_cordinates
 *
 * Description: Translate the cordinates from one window
 *             dimension to another
 *
 * Input parameters:
 *   dim1 - dimension of 1st window
 *   dim2 - dimension of 2nd window
 *   p_in_region - pointer to the input region
 *   p_out_region - pointer to the output region
 *   zoom_factor - zoom factor
 *   p_zoom_tbl - zoom table
 *   num_entries - number of zoom table entries
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int img_translate_cordinates_zoom(img_size_t dim1, img_size_t dim2,
  img_rect_t *p_in_region, img_rect_t *p_out_region,
  double zoom_factor, const uint32_t *p_zoom_tbl,
  uint32_t num_entries)
{
  int status = IMG_SUCCESS;
  img_rect_t zoom_reg;
  img_size_t zoom_fov;
  img_rect_t *p_zoom_reg = &zoom_reg;
  if (!dim1.width || !dim1.height || !dim2.width || !dim2.height
    || (zoom_factor < 1.0) || (zoom_factor > 8.0)) {
    IDBG_ERROR("%s:%d] Error invalid input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  zoom_fov.width = (int32_t)(dim1.width/zoom_factor);
  zoom_fov.height = (int32_t)(dim1.height/zoom_factor);
  IDBG_MED("%s:%d] Zoom FOV %dx%d", __func__, __LINE__,
    zoom_fov.width, zoom_fov.height);

  if (!zoom_fov.width || !zoom_fov.height) {
    IDBG_ERROR("%s:%d] Error zoom value", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  status = img_translate_cordinates(zoom_fov, dim2, p_in_region, p_zoom_reg);

  if (zoom_factor == 1.0) {
    /* no zoom */
    *p_out_region = zoom_reg;
  } else if (IMG_SUCCESS == status) {
    IMG_SCALE_RECT(p_zoom_reg, p_out_region, zoom_factor);
  } else if (IMG_ERR_NOT_SUPPORTED == status) {
    /* update a zoom dimension */
    uint32_t scale_x = (uint32_t)(dim2.width * Q12 / zoom_fov.width);
    uint32_t scale_y = (uint32_t)(dim2.height * Q12 / zoom_fov.height);
    uint32_t scale = MAX(scale_x, scale_y);
    double ratio;
    uint32_t idx = 0;
    IDBG_MED("%s:%d] scale (%f %f) %u",
      __func__, __LINE__,
      (double)scale_x/(double)Q12, (double)scale_y/(double)Q12,
      scale);
    idx = IMG_GET_ZOOM_IDX(p_zoom_tbl, num_entries, scale);
    ratio = (double)p_zoom_tbl[idx] / (double)p_zoom_tbl[0];
    zoom_fov.width = (int32_t)(ratio * zoom_fov.width);
    zoom_fov.height = (int32_t)(ratio * zoom_fov.height);
    IDBG_MED("%s:%d] id %d val %u scale %f New Zoom FOV %dx%d",
      __func__, __LINE__, idx, p_zoom_tbl[idx], ratio,
      zoom_fov.width, zoom_fov.height);
    status = img_translate_cordinates(zoom_fov, dim2, p_in_region, p_zoom_reg);
    if (IMG_SUCCEEDED(status)) {
      zoom_factor = (double)dim1.width/(double)zoom_fov.width;
      IMG_SCALE_RECT(p_zoom_reg, p_out_region, zoom_factor);
    }
  }
  IDBG_MED("%s:%d] dim1 %dx%d dim2 %dx%d in_reg (%d %d %d %d)"
    "out_reg (%d %d %d %d)",
    __func__, __LINE__,
    dim1.width, dim1.height, dim2.width, dim2.height,
    p_in_region->pos.x, p_in_region->pos.y,
    p_in_region->size.width, p_in_region->size.height,
    p_out_region->pos.x, p_out_region->pos.y,
    p_out_region->size.width, p_out_region->size.height);

  return status;
}

/**
 * Function: img_sw_scale_init_mn
 *
 * Description: init downscaling
 *
 * Input parameters:
 *   vInfo - contains width/height info for scaling
 *   pSrc - pointer to original img buffer
 *   srcWidth - original image width
 *   srcHeight - original image height
 *   srcStride - original image stride
 *   pDst - pointer to scaled image buffer
 *   dstWidth - desired width of schaled image
 *   dstHeight - desired height of scaled image
 *   dstStride - scaled image stride
 *
 * Return values: none
 *
 * Notes:  none
 **/
void __unused img_sw_scale_init_mn(img_scale_mn_v_info_t*  vInfo,
  uint8_t* pSrc,
  uint32_t srcWidth,
  uint32_t srcHeight,
  uint32_t srcStride,
  uint8_t* pDst,
  uint32_t dstWidth,
  uint32_t dstHeight,
  uint32_t dstStride)
{
  IMG_UNUSED(pSrc);
  IMG_UNUSED(srcWidth);
  IMG_UNUSED(srcStride);
  IMG_UNUSED(pDst);
  IMG_UNUSED(dstWidth);

  vInfo->count = 0;
  vInfo->step  = 0;
  vInfo->height = srcHeight;
  vInfo->output_height = dstHeight;
  vInfo->p_v_accum_line = (uint16_t *)malloc( dstStride * sizeof(uint16_t) );

  if ( ! (vInfo->p_v_accum_line) ) {
    return; //error
  }
  memset(vInfo->p_v_accum_line,
    0,
    dstStride * sizeof(uint16_t)); // init to zero //TBD
}

/**
 * Function: img_sw_scale_mn_vscale_byte
 *
 * Description: init Vertical M/N scaling on an input lines,
 * which is one byte per pixel
 *
 * Input parameters:
 *   p_v_info - contains width/height info for scaling
 *   p_output_line
 *   output_width
 *   p_input_line
 *
 * Return values:
 *   0 - accumulating
 *   1 - outputting 1 line
 *
 * Notes:  none
 **/
int img_sw_scale_mn_vscale_byte (img_scale_mn_v_info_t *p_v_info,
  uint8_t *p_output_line,
  uint32_t output_width,
  uint8_t *p_input_line)
{
  uint32_t output_width_copy = output_width;
  uint16_t *p_v_accum_line = p_v_info->p_v_accum_line;
  uint32_t input_height = p_v_info->height;
  uint32_t output_height = p_v_info->output_height;
  uint32_t step = p_v_info->step;
  uint32_t count = p_v_info->count;

  // Accumulate one line
  while (output_width_copy--) {
    *p_v_accum_line = (uint16_t)(*p_v_accum_line + *p_input_line);
    p_v_accum_line ++;
    p_input_line ++;
  }
  step++;
  count += output_height;           // M
  if (count >= input_height) {        // N
    output_width_copy = output_width;
    p_v_accum_line    = p_v_info->p_v_accum_line;
    while (output_width_copy--) {
      // Scaled pixel is average of either ceiling(N/M) or flooring(N/M)
      // original pixels
      *p_output_line++ =
      (uint8_t)(((*p_v_accum_line++) * mn_division_table[step]) >> MN_DIVISION_Q_BITS);
    }
    // Update count and step
    p_v_info->count = (count - input_height);
    p_v_info->step  = 0;
    return 1;
  }
  // Update count and step
  p_v_info->count = count;
  p_v_info->step  = step;
  return 0;
}

/**
 * Function: face_proc_scale_mn_hscale_byte
 *
 * Description: init horizontal scaling
 *
 * Input parameters:
 *   p_output_line
 *   output_width - M value
 *   p_input_line
 *   input_width - N value
 *
 * Return values: None
 *
 * Notes:  none
 **/
void img_sw_scale_mn_hscale_byte (uint8_t *p_output_line,
  uint32_t output_width,
  uint8_t *p_input_line,
  uint32_t input_width)
{
  uint32_t input_width_copy = input_width;
  uint32_t step, count, accum;

  // Validate input pointer
  if (!p_output_line || !p_input_line)
    return;

  count = 0;
  step  = 0;
  accum = 0;
  while (input_width_copy--) {
    accum += *p_input_line++;
    count += output_width;
    step++;
    if (count >= input_width) {
      // Scaled pixel is average of either ceiling(N/M) or flooring(N/M)
      // original pixels
      *p_output_line++ = (uint8_t)((accum * mn_division_table[step]) >> MN_DIVISION_Q_BITS);
      count -= input_width;
      accum = 0;
      step = 0;
    }
  }
}

/**
 * Function: scalingInitMN
 *
 * Description: Image downscaling using MN method
 *
 * Input parameters:
 *   pSrc - pointer to original img buffer
 *   srcWidth - original image width
 *   srcHeight - original image height
 *   srcStride - original image stride
 *   pDst - pointer to scaled image buffer
 *   dstWidth - desired width of scaled image
 *   dstHeight - desired height of scaled image
 *   dstStride - desired stride of scaled image
 *
 * Return values: none
 *
 * Notes:  none
 **/
void __unused img_sw_downscale(uint8_t *src,uint32_t srcWidth,uint32_t srcHeight,
  uint32_t srcStride, uint8_t *dst, uint32_t dstWidth, uint32_t dstHeight,
  uint32_t dstStride)
{
  img_scale_mn_v_info_t  vInfo;

  uint32_t linesOutput;
  uint8_t  *pSourceLine, *pHScaledLine,  *pScaledLine;

  //init phase/filter coef/...
  img_sw_scale_init_mn(&vInfo, (uint8_t *)src, srcWidth, srcHeight, srcStride,
    dst, dstWidth, dstHeight, dstStride );

  linesOutput = 0;
  pSourceLine = src;
  pHScaledLine = (uint8_t *)malloc((dstWidth) << 1);
  if (!(pHScaledLine)) {
    free(vInfo.p_v_accum_line);
    return; //error
  }
  pScaledLine  = dst;

  while ( linesOutput < dstHeight ) {
    img_sw_scale_mn_hscale_byte( pHScaledLine, dstWidth,
      pSourceLine, srcWidth);

    pSourceLine += srcStride;

    if (img_sw_scale_mn_vscale_byte(&vInfo, pScaledLine,
      dstWidth, pHScaledLine)) {
      // Clear accumulation line
      (void)memset(vInfo.p_v_accum_line, 0,
        ((dstStride) * sizeof(uint16_t)));
      // Move to next destination line
      linesOutput++;
      pScaledLine += dstStride;
    }
  }
  free(pHScaledLine);
  free(vInfo.p_v_accum_line);
}

/**
 * Function: img_sw_downscale_2by2
 *
 * Description: Optimized version of downscale 2by2.
 *
 * Input parameters:
 *   @p_src - Pointer to source buffer.
 *   @src_width - Source buffer width.
 *   @src_height - Source buffer height.
 *   @src_stride - original image stride.
 *   @p_dst - Pointer to scaled destination buffer.
 *   @dst_stride - Destination stride.
 *
 * Return values: success
 *
 *  Notes: even if the assembly code doesnt return error, return
 *      type is added for future compatibility
 **/
int32_t img_sw_downscale_2by2(void *p_src, uint32_t src_width, uint32_t src_height,
  uint32_t src_stride, void *p_dst, uint32_t dst_stride)
{
  ds_2by2_asm(p_src, src_width, src_height, src_stride, p_dst, dst_stride);
  return IMG_SUCCESS;
}

/**
 * Function: img_sw_cds_c
 *
 * Description: Software CDS routine in C
 *
 * Input parameters:
 *   @p_src - Pointer to source buffer.
 *   @src_width - Source buffer width.
 *   @src_height - Source buffer height.
 *   @src_stride - original image stride.
 *   @p_dst - Pointer to scaled destination buffer.
 *   @dst_stride - Destination stride.
 *
 * Return values: Imaging errors
 **/
int32_t img_sw_cds_c(uint8_t *p_src, uint32_t src_width, uint32_t src_height,
  uint32_t src_stride, uint8_t *p_dst, uint32_t dst_stride)
{
  uint32_t row, col;
  uint8_t *pdst_tmp;
  uint8_t *psrc_tmp[2];
  uint32_t c_val;
  for (row = 0; row < (src_height - 1); row += 2) {
    /* Fetch 2 rows */
    psrc_tmp[0] = p_src + (row * src_stride);
    psrc_tmp[1] = p_src + ((row + 1) * src_stride);
    pdst_tmp = p_dst + ((row >> 1) * dst_stride);
    for (col = 0; col < src_width; col += 4) {
      /* Take 4 pixel per row */
      c_val = (uint32_t)(*psrc_tmp[0] + *(psrc_tmp[0] + 2) +
        *psrc_tmp[1] + *(psrc_tmp[1] + 2)) >> 2;
      *pdst_tmp = c_val;
      pdst_tmp++;
      /* Apply for the next component */
      c_val = (uint32_t)(*(psrc_tmp[0] + 1) + *(psrc_tmp[0] + 3) +
        *(psrc_tmp[1] + 1) + *(psrc_tmp[1] + 3)) >> 2;
      *pdst_tmp = c_val;
      pdst_tmp ++;
      /* advance 4 pixels to the first C component*/
      psrc_tmp[0] += 4;
      psrc_tmp[1] += 4;
    }
  }
  return IMG_SUCCESS;
}

/**
 * Function: img_sw_cds_neon
 *
 * Description: Software CDS routine in neon intrinsics
 *
 * Input parameters:
 *   @p_src - Pointer to source buffer.
 *   @src_width - Source buffer width.
 *   @src_height - Source buffer height.
 *   @src_stride - original image stride.
 *   @p_dst - Pointer to scaled destination buffer.
 *   @dst_stride - Destination stride.
 *
 * Return values: none
 **/
int32_t img_sw_cds_neon(uint8_t *p_src,
  uint32_t src_width,
  uint32_t src_height,
  uint32_t src_stride,
  uint8_t *p_dst,
  uint32_t dst_stride)
{
  uint32_t row, col;
  uint8_t *pdst_tmp = p_dst;
  uint8_t *psrc_tmp = p_src;
  uint32_t rem_bytes;
  uint32_t c_val, i;

  for (row = 0; row < src_height; row += 2) {
    pdst_tmp = p_dst + (row >> 1) * dst_stride;
    for (col = 0; col < src_width; col += 32) {
      psrc_tmp = p_src + row * src_stride + col;
      uint8x8x2_t out;
      uint8x8x4_t row[2];
      uint16x8_t temp[2];
      row[0] = vld4_u8(psrc_tmp);
      row[1] = vld4_u8(psrc_tmp + src_stride);
      /* unrolling the 2 vectors so as to prevent intrinsic compiler
         optimization */
      temp[0] = vaddl_u8(row[0].val[0], row[0].val[2]);
      temp[1] = vaddl_u8(row[1].val[0], row[1].val[2]);
      out.val[0] = vshrn_n_u16(vaddq_u16(temp[0], temp[1]), 2);
      temp[0] = vaddl_u8(row[0].val[1], row[0].val[3]);
      temp[1] = vaddl_u8(row[1].val[1], row[1].val[3]);
      out.val[1] = vshrn_n_u16(vaddq_u16(temp[0], temp[1]), 2);
      vst2_u8(pdst_tmp, out);
      pdst_tmp += 16;
    }
    /* handle the left over pixels using native C */
    if (col > src_width) {
      uint8_t *psrc_tmp[2];
      col -= 32; /* reset the column */
      rem_bytes = src_width - col;
      IDBG_LOW("%s:%d] Remaining bytes %d", __func__, __LINE__,
        rem_bytes);
      for (i = 0; i < 2; i++)
        psrc_tmp[i] = p_src + ((row + i) * src_stride) + col;
      for (i = 0; i < rem_bytes; i += 4) {
        /* Take 4 pixel per row */
        c_val = (uint32_t)(*psrc_tmp[0] + *(psrc_tmp[0] + 2) +
          *psrc_tmp[1] + *(psrc_tmp[1] + 2)) >> 2;
        *pdst_tmp = c_val;
        pdst_tmp++;
        /* Apply for the next component */
        c_val = (uint32_t)(*(psrc_tmp[0] + 1) + *(psrc_tmp[0] + 3) +
          *(psrc_tmp[1] + 1) + *(psrc_tmp[1] + 3)) >> 2;
        *pdst_tmp = c_val;
        pdst_tmp++;
        /* advance 3 pixels to the first C component*/
        psrc_tmp[0] += 4;
        psrc_tmp[1] += 4;
      }
    }
  }
  return IMG_SUCCESS;
} /*img_sw_cds_neon*/

/**
 * Function: img_sw_cds
 *
 * Description: Software CDS routine in neon intrinsics
 *
 * Input parameters:
 *   @p_src - Pointer to source buffer.
 *   @src_width - Source buffer width.
 *   @src_height - Source buffer height.
 *   @src_stride - original image stride.
 *   @p_dst - Pointer to scaled destination buffer.
 *   @dst_stride - Destination stride.
 *   @type: operation type
 *
 * Return values: Imaging errors
 **/
int32_t img_sw_cds(uint8_t *p_src, uint32_t src_width,
  uint32_t src_height,
  uint32_t src_stride,
  uint8_t *p_dst,
  uint32_t dst_stride,
  img_ops_core_type type)
{
  int32_t rc;

  if ((src_width % 4 != 0) || (src_height % 4 != 0) ||
    !p_src || !p_dst || (src_width > src_stride) ||
    ((src_width >> 1) > dst_stride)) {
    IDBG_ERROR("%s:%d] Invalid input %dx%d str %d %d ptr %p %p type %d",
      __func__, __LINE__, src_width, src_height,
      src_stride, dst_stride, p_src, p_dst, type);
    return IMG_ERR_NOT_SUPPORTED;
  }
  IDBG_HIGH("%s:%d] Input %dx%d str %d %d ptr %p %p type %d",
    __func__, __LINE__, src_width, src_height,
    src_stride, dst_stride, p_src, p_dst, type);
  switch (type) {
  case IMG_OPS_NEON:
    rc = img_sw_cds_neon(p_src, src_width, src_height,
      src_stride, p_dst, dst_stride);
    break;
  case IMG_OPS_FCV:
#ifdef __CAM_FCV__
    fcvScaleDownMNInterleaveu8(p_src, src_width>>1, src_height,
      src_stride, p_dst, src_width >> 2, src_height >> 1,
      dst_stride);
    rc = IMG_SUCCESS;
    break;
#endif
  /* fall through if fcv is not available */
  default:
    /* use native C */
    rc = img_sw_cds_c(p_src, src_width, src_height,
      src_stride, p_dst, dst_stride);
    break;
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return rc;
} /* img_sw_cds */

/** img_image_stride_fill:
 *  @out_buff: output buffer handler
 *
 * Function to fill image stride with image data
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_image_stride_fill(img_frame_t *out_buff)
{
  int ret_val = IMG_ERR_INVALID_INPUT;
  uint8_t *in_ptr8;
  uint8_t *out_ptr8;
  uint16_t *in_ptr16;
  uint16_t *out_ptr16;
  uint32_t *in_ptr32;
  uint32_t *out_ptr32;
  uint32_t i,j;
  register uint32_t k,padding;
  uint32_t step;

  if (out_buff->frame[0].plane_cnt) {
    ret_val = IMG_SUCCESS;
    for (i=0; i<out_buff->frame[0].plane_cnt; i++) {
      if (out_buff->frame[0].plane[i].height <= 0
        || out_buff->frame[0].plane[i].width <= 0
        || out_buff->frame[0].plane[i].stride <= 0
        || out_buff->frame[0].plane[i].width >
            out_buff->frame[0].plane[i].stride) {
        ret_val = IMG_ERR_INVALID_INPUT;
        break;
      }

      step = 1;
      if (PLANE_CB_CR == out_buff->frame[0].plane[i].plane_type)
        step = 2;
      else if (QIMG_SINGLE_PLN_INTLVD (out_buff))
        step = 4;

      // If stride and width are multiple by step,
      // then padding is also multiple by step
      if (out_buff->frame[0].plane[i].stride % step
        || out_buff->frame[0].plane[i].width % step) {
          ret_val = IMG_ERR_INVALID_INPUT;
          break;
      }
    }
  }

  if (IMG_SUCCESS == ret_val) {
    for (i=0; i<out_buff->frame[0].plane_cnt; i++) {

      padding = out_buff->frame[0].plane[i].stride
        - out_buff->frame[0].plane[i].width;

      if (!padding)
        continue;

      step = 1;
      out_ptr8 = out_buff->frame[0].plane[i].addr
        + out_buff->frame[0].plane[i].offset;

      if (PLANE_CB_CR == out_buff->frame[0].plane[i].plane_type) {
        step = 2;
        padding >>= 1;
        out_ptr16 = (uint16_t*)out_ptr8;
      } else if (QIMG_SINGLE_PLN_INTLVD(out_buff)) {
        step = 4;
        padding >>= 2;
        out_ptr32 = (uint32_t*)out_ptr8;
      }

      if (1 == step) {
        for (j=out_buff->frame[0].plane[i].height; j; j--) {

          out_ptr8 += out_buff->frame[0].plane[i].width;
          in_ptr8 = out_ptr8 - 1;
          for (k=padding; k; k--) {
            *out_ptr8++ = *in_ptr8--;
          }

        }
      } else if (2 == step) {
        for (j=out_buff->frame[0].plane[i].height; j; j--) {

          out_ptr16 += (out_buff->frame[0].plane[i].width >> 1);
          in_ptr16 = out_ptr16 - 1;
          for (k=padding; k; k--) {
            *out_ptr16++ = *in_ptr16--;
          }

        }
      } else if (4 == step) {
        for (j=out_buff->frame[0].plane[i].height; j; j--) {

          out_ptr32 += (out_buff->frame[0].plane[i].width >> 2);
          in_ptr32 = out_ptr32 - 1;
          for (k=padding; k; k--) {
            *out_ptr32++ = *in_ptr32--;
          }

        }
      }
    }
  } else {
    IDBG_ERROR("%s:%d failed: Output and input buffers are not compatible",
      __func__, __LINE__);
  }

  return ret_val;
}

/** img_alloc_ion:
 *  @mapion_list: Ion structure list to memory blocks to be allocated
 *  @num: number of buffers to be allocated
 *  @ionheapid: ION heap ID
 *  @cached:
 *    TRUE: mappings of this buffer should be cached, ion will do cache
            maintenance when the buffer is mapped for dma
 *    FALSE: mappings of this buffer should not be cached
 *
 * Function to allocate a physically contiguous memory
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_alloc_ion(img_mmap_info_ion *mapion_list, int num, uint32_t ionheapid,
  int cached)
{
  int ret_val = IMG_ERR_NO_MEMORY;
  struct ion_allocation_data alloc;
  int rc;
  int ion_fd;
  void *ret;
  int  p_pmem_fd;
  img_mmap_info_ion *mapion = mapion_list;
  int i;

  if (!num || !mapion_list) {
    IDBG_ERROR("%s:%d failed: Wrong input parameters, num=%d, mapion_list=%p",
      __func__, __LINE__, num, mapion_list);
    return IMG_ERR_INVALID_INPUT;
  }

  for (i=0, mapion = mapion_list; i<num; i++, mapion++) {
    if (mapion->virtual_addr
#ifndef TARGET_ION_ABI_VERSION
      || mapion->ion_info_fd.handle
#endif
      || mapion->ion_info_fd.fd
      || mapion->ion_fd
      || !mapion->bufsize) {
        IDBG_ERROR("%s:%d failed: Wrong input parameters",
          __func__, __LINE__);
        return IMG_ERR_INVALID_INPUT;
    }
  }

#ifndef TARGET_ION_ABI_VERSION
  ion_fd = open("/dev/ion", O_RDONLY);
#else
  ion_fd = ion_open();
#endif
  if (ion_fd < 0) {
    IDBG_ERROR("%s:%d Open ion device failed",
      __func__, __LINE__);
    return ret_val;
  }

  mapion_list->ion_fd = ion_fd;

  alloc.len = mapion_list->bufsize;
  alloc.align = 4096;
  alloc.heap_id_mask = ionheapid;
  alloc.flags = 0;

#ifndef TARGET_ION_ABI_VERSION
  if (ION_HEAP(ION_CP_MM_HEAP_ID) == ionheapid) {
    alloc.flags |= (uint32_t)ION_SECURE;
  }
#else
  if (ION_HEAP(ION_CP_MM_HEAP_ID) == ionheapid) {
    alloc.flags |= (uint32_t)ION_FLAG_SECURE;
  }
#endif


  if (TRUE == cached) {
    alloc.flags |= ION_FLAG_CACHED;
  }

  for (i=0, mapion = mapion_list; i<num; i++, mapion++) {
#ifndef TARGET_ION_ABI_VERSION
    rc = ioctl(ion_fd, ION_IOC_ALLOC, &alloc);

    if (rc < 0) {
      IDBG_ERROR("%s:%d ION alloc length %d %zu failed",
        __func__, __LINE__, rc, alloc.len);
      break;
    } else {
      mapion->ion_info_fd.handle = alloc.handle;
      rc = ioctl(ion_fd, ION_IOC_SHARE, &(mapion->ion_info_fd));

      if (rc < 0) {
        IDBG_ERROR("%s:%d ION map call failed %d",
          __func__, __LINE__, rc);
        break;
      } else {
        p_pmem_fd = mapion->ion_info_fd.fd;
        ret = mmap(NULL,
          alloc.len,
          PROT_READ  | PROT_WRITE,
          MAP_SHARED,
          p_pmem_fd,
          0);

        if (ret == MAP_FAILED) {
          IDBG_ERROR("%s:%d mmap call failed %d",
            __func__, __LINE__, rc);
          break;
        } else {
          IDBG_HIGH("%s:%d Ion allocation success virtaddr : %p fd %u",
            __func__, __LINE__,
            ret,
            (uint32_t)p_pmem_fd);
          mapion->virtual_addr = ret;
          mapion->ion_fd = ion_fd;
          ret_val = IMG_SUCCESS;
        }
      }
    }
#else
    rc = ion_alloc_fd(ion_fd, alloc.len, alloc.align, alloc.heap_id_mask,
      alloc.flags, &p_pmem_fd);
    if (rc < 0) {
      IDBG_ERROR("%s:%d ION map call failed %d",
        __func__, __LINE__, rc);
      break;
    } else {
      mapion->ion_info_fd.fd = p_pmem_fd;
      ret = mmap(NULL,
                alloc.len,
                PROT_READ  | PROT_WRITE,
                MAP_SHARED,
                p_pmem_fd,
                0);

      if (ret == MAP_FAILED) {
        IDBG_ERROR("%s:%d mmap call failed %d",
          __func__, __LINE__, rc);
        close(p_pmem_fd);
        break;
      } else {
        IDBG_HIGH("%s:%d Ion allocation success virtaddr : %p fd %u",
          __func__, __LINE__,
          ret,
          (uint32_t)p_pmem_fd);
        mapion->virtual_addr = ret;
        mapion->ion_fd = ion_fd;
        ret_val = IMG_SUCCESS;
      }
    }
#endif
  }

  if (IMG_SUCCESS != ret_val) {
    img_free_ion(mapion_list, num);
  }

  return ret_val;
}

/** img_free_ion:
 *  @mapion_list: Ion structure list to the allocated memory blocks
 *  @num: number of buffers to be freed
 *
 * Free ion memory
 *
 *
 * Returns IMG_SUCCESS in case of success
 **/
int img_free_ion(img_mmap_info_ion* mapion_list, int num)
{
#ifndef TARGET_ION_ABI_VERSION
  struct ion_handle_data handle_data;
#endif
  unsigned int bufsize;
  int i;
  img_mmap_info_ion* mapion;

  if (!num || !mapion_list) {
    IDBG_ERROR("%s:%d Wrong input parameters",
      __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  for (i=0, mapion = mapion_list; i<num; i++, mapion++) {

#ifndef TARGET_ION_ABI_VERSION
    if (mapion->ion_info_fd.handle) {
      if (mapion->ion_info_fd.fd) {
        if (mapion->virtual_addr) {
          bufsize = (mapion->bufsize + 4095) & (~4095U);
          munmap(mapion->virtual_addr, bufsize);
        }
        close(mapion->ion_info_fd.fd);
      }
      handle_data.handle = mapion->ion_info_fd.handle;
      ioctl(mapion->ion_fd, ION_IOC_FREE, &handle_data);
    }
#else
    if (mapion->ion_info_fd.fd) {
      if (mapion->virtual_addr) {
        bufsize = (mapion->bufsize + 4095) & (~4095U);
        munmap(mapion->virtual_addr, bufsize);
      }
      close(mapion->ion_info_fd.fd);
    }
#endif

  }

  if (mapion_list->ion_fd) {
    close(mapion_list->ion_fd);
  }

  return IMG_SUCCESS;
}

/** img_cache_ops_external:
 *  @p_buffer: vaddr of the buffer
 *  @size: Buffer size
 *  @offset: Buffer offset
 *  @fd: Fd of p_buffer
 *  @type: Type of cache operation- CACHE_INVALIDATE, CACHE_CLEAN
 *              or CACHE_CLEAN_INVALIDATE
 *  @ion_device_fd: Ion device FD
 *
 * Invalidate cache for memory allocated externally
 *
 *
 * Returns IMG_SUCCESS
 *         IMG_ERR_INVALID_INPUT
 *         IMG_ERR_GENERAL
 **/
int img_cache_ops_external (void *p_buffer, size_t size, uint32_t offset, int fd,
  img_cache_ops_t type, int ion_device_fd)
{

 int rc = IMG_SUCCESS;
/*TODO Need to implement cache using new ion api's.*/
#ifndef TARGET_ION_ABI_VERSION
  struct ion_custom_data custom_data;
  struct ion_flush_data cache_inv_data;
  struct ion_fd_data fd_data;
  struct ion_handle_data handle_data;
  uint32_t cmd = ION_IOC_CLEAN_INV_CACHES;

  memset(&custom_data, 0, sizeof(struct ion_custom_data));
  memset(&cache_inv_data, 0, sizeof(struct ion_flush_data));
  memset(&fd_data, 0, sizeof(struct ion_fd_data));
  memset(&handle_data, 0, sizeof(struct ion_handle_data));

  if (ion_device_fd < 0) {
    IDBG_ERROR("%s:%d: Invalid ION fd %d", __func__, __LINE__, ion_device_fd);
    return IMG_ERR_INVALID_INPUT;
  }

  if (NULL == p_buffer) {
    IDBG_ERROR("%s:%d: Buffer is null", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  fd_data.fd = fd;
  rc = ioctl (ion_device_fd, ION_IOC_IMPORT, &fd_data);
  if (rc) {
    IDBG_ERROR("%s:%d: ION_IOC_IMPORT failed", __func__, __LINE__);
    return rc;
  }

  switch (type) {
  case IMG_CACHE_INV:
    cmd = ION_IOC_INV_CACHES;
    break;
  case IMG_CACHE_CLEAN:
    cmd = ION_IOC_CLEAN_CACHES;
    break;
  default:
  case IMG_CACHE_CLEAN_INV:
    cmd = ION_IOC_CLEAN_INV_CACHES;
    break;
  }

  IDBG_HIGH("CAMCACHE : CacheOp : (vaddr=%p, fd=%d, size=%d, offset=%d), "
    "type=%s, ion_device_fd=%d",
    p_buffer, fd, size, offset,
    (type == IMG_CACHE_INV) ? "INV" :
    (type == IMG_CACHE_CLEAN) ? "CLEAN" : "CLEAN_INV",
    ion_device_fd);

  handle_data.handle = fd_data.handle;
  cache_inv_data.handle = fd_data.handle;
  cache_inv_data.vaddr = p_buffer;
  cache_inv_data.offset = offset;
  cache_inv_data.length = (uint32_t)size;
  custom_data.cmd = cmd;
  custom_data.arg = (unsigned long)&cache_inv_data;

  rc = ioctl(ion_device_fd, ION_IOC_CUSTOM, &custom_data);
  if (rc ) {
    IDBG_ERROR("%s:%d: Cache Invalidation failed %d", __func__, __LINE__, rc);
    ioctl(ion_device_fd, ION_IOC_FREE, &handle_data);
    return IMG_ERR_GENERAL;
  } else {
    IDBG_MED("Cache ops Success");
  }

  ioctl(ion_device_fd, ION_IOC_FREE, &handle_data);
#else
  struct dma_buf_sync buf_sync_start;
  struct dma_buf_sync buf_sync_end;

  /* ION_IOC_CLEAN_CACHES-->call the DMA_BUF_IOCTL_SYNC IOCTL with flags
     DMA_BUF_SYNC_START and DMA_BUF_SYNC_WRITE and then call the
     DMA_BUF_IOCTL_SYNC IOCTL with flags DMA_BUF_SYNC_END
     and DMA_BUF_SYNC_WRITE

     ION_IOC_INV_CACHES-->call the DMA_BUF_IOCTL_SYNC IOCT with flags
     DMA_BUF_SYNC_START and DMA_BUF_SYNC_WRITE and then call the
     DMA_BUF_IOCTL_SYNC IOCT with flags DMA_BUF_SYNC_END
     and DMA_BUF_SYNC_READ
  */
  switch (type) {
  case IMG_CACHE_INV:
    buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
    buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_READ;
    break;
  case IMG_CACHE_CLEAN:
    buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_WRITE;
    buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_WRITE;
    break;
  default:
  case IMG_CACHE_CLEAN_INV:
    buf_sync_start.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
    buf_sync_end.flags   = DMA_BUF_SYNC_END   | DMA_BUF_SYNC_RW;
    break;
  }

  rc = ioctl(fd, DMA_BUF_IOCTL_SYNC, &buf_sync_start.flags);
  if (rc) {
    IDBG_ERROR("Failed first DMA_BUF_IOCTL_SYNC start\n");
  }

  rc = ioctl(fd, DMA_BUF_IOCTL_SYNC, &buf_sync_end.flags);
  if (rc) {
    IDBG_ERROR("Failed first DMA_BUF_IOCTL_SYNC end\n");
  }

#endif
  return rc;
}

/** img_get_timestamp
 *  @timestamp: pointer to a char buffer. The buffer should be
 *    allocated by the caller
 *  @size: size of the char buffer
 *
 *  Get the current timestamp and convert it to a string
 *
 *  Return: None.
 **/
void img_get_timestamp(char *timestamp, uint32_t size) {
  time_t rawtime;
  struct tm *currenttime = NULL;
  rawtime = time(NULL);
  currenttime = localtime(&rawtime);
  if (currenttime) {
    snprintf(timestamp,
      size, "%04d%02d%02d%02d%02d%02d",
      currenttime->tm_year+1900, currenttime->tm_mon, currenttime->tm_mday,
      currenttime->tm_hour, currenttime->tm_min, currenttime->tm_sec);
  }
}

/** img_dump_frame
 *    @img_frame: frame handler
 *    @file_name: file name prefix
 *    @number: number to be appended at the end of the file name
 *    @p_meta: metadata handler
 *
 * Saves specified frame to folder /data/vendor/camera/
 *
 * Returns nothing
 **/
void img_dump_frame(img_frame_t *img_frame, const char* file_name,
  uint32_t number, void *p_meta)
{
  uint32_t i;
  size_t size;
  size_t written_size;
  int32_t out_file_fd;
  char name[FILENAME_MAX];
  char meta_file_name[FILENAME_MAX];
  char out_file_name[FILENAME_MAX];
  char value[PROPERTY_VALUE_MAX];
  char timestamp[25] = { 0 };
  uint32_t is_interleaved = 0;

  property_get("persist.vendor.camera.imglib.dump", value, "0");
  if (0 == atoi(value)) {
    return;
  }

  if (img_frame) {
    img_get_timestamp(timestamp, sizeof(timestamp));

    snprintf(name, sizeof(name), "%s%s_%s_%dx%d_%d_%d",
      QCAMERA_DUMP_FRM_LOCATION, file_name,
      timestamp, img_frame->frame[0].plane[0].stride,
      img_frame->frame[0].plane[0].scanline, img_frame->frame_id, number);
    snprintf(meta_file_name, sizeof(meta_file_name), "%s.txt", name);
    snprintf(out_file_name, sizeof(out_file_name), "%s.yuv", name);

    if (p_meta) {
      img_dump_meta(p_meta, meta_file_name);
    }
    is_interleaved = QIMG_SINGLE_PLN_INTLVD(img_frame);

    IDBG_LOW("%s:%d] framedump to %s\n", __func__, __LINE__, out_file_name);

    out_file_fd = open(out_file_name, O_RDWR | O_CREAT, 0777);

    if (out_file_fd >= 0) {
      for (i = 0; i < img_frame->frame[0].plane_cnt; i++) {
        if (is_interleaved) {
          size = (size_t)(img_frame->frame[0].plane[i].stride
            * img_frame->frame[0].plane[i].scanline * 2);
        } else {
          size = (size_t)(img_frame->frame[0].plane[i].stride
            * img_frame->frame[0].plane[i].scanline);
        }
        written_size = (size_t)write(out_file_fd,
          img_frame->frame[0].plane[i].addr + img_frame->frame[0].plane[i].offset,
          size);
        if (size != written_size)
          IDBG_ERROR("%s:%d failed: Cannot write data to file %s size %d %d\n",
            __func__, __LINE__, out_file_name, size, written_size);
      }

      close(out_file_fd);

      IDBG_HIGH("%s:%d: dim %dx%d pad_dim %dx%d",
        __func__, __LINE__,
        img_frame->frame[0].plane[0].width,
        img_frame->frame[0].plane[0].height,
        img_frame->frame[0].plane[0].stride,
        img_frame->frame[0].plane[0].scanline);
    } else {
      IDBG_ERROR("%s:%d failed: Cannot open file %s %s",
        __func__, __LINE__,
        out_file_name, strerror(errno));
    }
  } else {
    IDBG_ERROR("%s:%d failed: Null pointer detected\n", __func__, __LINE__);
  }
}

/** img_perf_lock_handle_create
 *
 * Creates new performance handle
 *
 * Returns new performance handle
 **/
void* img_perf_handle_create()
{
  img_perf_handle_t *perf_handle;
  char qcopt_lib_path[PATH_MAX] = {0};

  perf_handle = calloc(1, sizeof(img_perf_handle_t));
  if (!perf_handle) {
    IDBG_ERROR("%s:%d Not enough memory\n", __func__, __LINE__);
    return NULL;
  }

  if (!property_get("ro.vendor.extension_library", qcopt_lib_path, NULL)) {
    IDBG_ERROR("%s:%d Cannot get performance lib name\n", __func__, __LINE__);
    free(perf_handle);
    return NULL;
  }
  perf_handle->instance = dlopen(qcopt_lib_path, RTLD_NOW);

  if (!perf_handle->instance) {
    IDBG_ERROR("%s:%d Unable to open %s: %s\n", __func__, __LINE__,
      qcopt_lib_path, dlerror());
    free(perf_handle);
    return NULL;
  }
  perf_handle->perf_lock_acq = (int (*) (int, int, int[], int))dlsym(perf_handle->instance,
    "perf_lock_acq");

  if (!perf_handle->perf_lock_acq) {
    IDBG_ERROR("%s:%d Unable to get perf_lock_acq function handle\n", __func__,
      __LINE__);
    if (dlclose(perf_handle->instance)) {
      IDBG_ERROR("%s:%d Error occurred while closing qc-opt library\n",
        __func__, __LINE__);
    }
    free(perf_handle);
    return NULL;
  }
  perf_handle->perf_lock_rel = (int (*) (int))dlsym(perf_handle->instance, "perf_lock_rel");

  if (!perf_handle->perf_lock_rel) {
    IDBG_ERROR("%s:%d Unable to get perf_lock_rel function handle\n", __func__,
      __LINE__);
    if (dlclose(perf_handle->instance)) {
      IDBG_ERROR("%s:%d Error occurred while closing qc-opt library\n",
        __func__, __LINE__);
    }
    free(perf_handle);
    return NULL;
  }

  return perf_handle;
}

/** img_perf_handle_destroy
 *    @p_perf: performance handle
 *
 * Destoyes performance handle
 *
 * Returns None.
 **/
void img_perf_handle_destroy(void* p_perf)
{
  img_perf_handle_t *perf_handle = (img_perf_handle_t*)p_perf;

  IMG_RETURN_IF_NULL(return, perf_handle)
  IMG_RETURN_IF_NULL(return, perf_handle->instance)

  if (dlclose(perf_handle->instance)) {
    IDBG_ERROR("%s:%d Error occurred while closing qc-opt library\n",
      __func__, __LINE__);
  }

  free(perf_handle);
}

/** img_perf_lock_start
 *    @p_perf: performance handle
 *    @p_perf_lock_params: performance lock parameters
 *    @perf_lock_params_size: size of performance lock parameters
 *    @duration: duration
 *
 * Locks performance with specified parameters
 *
 * Returns new performance lock handle
 **/
void* img_perf_lock_start(void* p_perf, int* p_perf_lock_params,
  int perf_lock_params_size, int duration)
{
  img_perf_handle_t *perf_handle = (img_perf_handle_t*)p_perf;
  img_perf_lock_handle_t *lock_handle;
  int32_t rc = -1;

  IMG_RETURN_IF_NULL(return NULL, perf_handle)
  IMG_RETURN_IF_NULL(return NULL, p_perf_lock_params)
  IMG_RETURN_IF_NULL(return NULL, perf_handle->perf_lock_acq)

  lock_handle = calloc(1, sizeof(img_perf_lock_handle_t));
  if (!lock_handle) {
    IDBG_ERROR("%s:%d Not enough memory\n", __func__, __LINE__);
    return NULL;
  }

  rc = perf_handle->perf_lock_acq(lock_handle->instance,
    duration, p_perf_lock_params, perf_lock_params_size);
  if (rc < 0) {
    IDBG_HIGH("Failed to acquire lock\n");
  } else {
    lock_handle->instance = rc;
  }
  IDBG_MED("lock_handle->instance %d\n", lock_handle->instance);

  return lock_handle;
}

/** img_perf_lock_end
 *    @p_perf: performance handle
 *    @p_perf_lock: performance lock handle
 *
 * Locks performance with specified parameters
 *
 * Returns None.
 **/
void img_perf_lock_end(void* p_perf, void* p_perf_lock)
{
  img_perf_handle_t *perf_handle = (img_perf_handle_t*)p_perf;
  img_perf_lock_handle_t *lock_handle = (img_perf_lock_handle_t*)p_perf_lock;

  IMG_RETURN_IF_NULL(return, perf_handle)
  IMG_RETURN_IF_NULL(return, lock_handle)
  IMG_RETURN_IF_NULL(return, perf_handle->perf_lock_rel)

  if (lock_handle->instance > 0) {
    perf_handle->perf_lock_rel(lock_handle->instance);
  }
  free(lock_handle);
}

/** img_parse_extract_kv_pair
 *    @line: line to be parsed
 *    @p_userdata: userdata provided by the client
 *    @p_parse_cb: parse function provided by the client
 *
 *   Function to extrack key value pair from the line
 *
 * Returns None.
 **/
static void img_parse_extract_kv_pair(char* line,
  void *p_userdata,
  img_parse_cb_t p_parse_cb)
{
  char *val[MAX_PARSE_TOKEN];
  char *last;
  int index = 0;

  val[index] = strtok_r(line, "=", &last);
  if (val[index] == NULL) {
    val[index+1] = NULL;
  }
  while (val[index] != NULL) {
    if (index > 1) {
      IDBG_ERROR("%s:%d] line %s parse failed", __func__, __LINE__,
        val[index]);
      break;
    }
    ++index;
    val[index] = strtok_r(NULL, "=", &last);
  }

  IDBG_MED("%s:%d] attr %s val %s ", __func__, __LINE__,
    val[0], val[1]);
  p_parse_cb(p_userdata, val[0], val[1]);

}/*img_parse_extract_kv_pair*/

/** img_parse_main
 *    @datafile: file to be parsed
 *    @p_userdata: userdata provided by the client
 *    @p_parse_cb: parse function provided by the client
 *
 *   Main function for parsing
 *
 * Returns imglib error values.
 **/
int img_parse_main(const char* datafile, void *p_userdata,
  img_parse_cb_t p_parse_cb)
{
  int rc = IMG_SUCCESS;
  char *token, *last, *input_data = NULL;;
  uint32_t bytes_read;
  uint64_t input_size;
  bool input_size_overflow = false;
  FILE *fp;

  if (!p_parse_cb || !datafile || !p_userdata) {
    IDBG_ERROR("%s:%d] Invalid input p_parse_cb %p datafile %p",
      __func__, __LINE__, p_parse_cb, datafile);
    return IMG_ERR_INVALID_INPUT;
  }

  fp = fopen(datafile, "r");
  if (NULL == fp) {
    IDBG_ERROR("%s:%d] fp NULL", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  /*calculate size*/
  fseek(fp, 0L, SEEK_END);
  input_size = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  IDBG_HIGH("%s:%d] input_size %d", __func__, __LINE__, input_size);

  input_size_overflow = ((input_size + 1) < input_size);
  input_data = (char *)malloc((input_size + 1) * sizeof(char)); /* for '\0' */
  /*
   * Zero input size and non-null pointer is to be treated as if the pointer is
   * invalid and thus we error out and go through free() call.
   */
  if ((input_data == NULL) || input_size_overflow) {
    if (input_size_overflow) {
      IDBG_ERROR("%s:%d] input size overflow", __func__, __LINE__);
    } else {
      IDBG_ERROR("%s:%d] cannot allocate input", __func__, __LINE__);
    }
    rc = IMG_ERR_NO_MEMORY;
    goto error;
  }
  bytes_read = fread(input_data, 1, input_size, fp);
  IDBG_HIGH("%s:%d] bytes_read %d input_size %d", __func__, __LINE__,
    bytes_read, input_size);
  if (bytes_read < input_size) {
    IDBG_ERROR("%s:%d] cannot get the data from file %d %d", __func__,
      __LINE__, bytes_read, input_size);
    rc = IMG_ERR_GENERAL;
    goto error;
  }
  fclose(fp); /* close file handle */
  fp = NULL;

  /* null terminate, before string ops applied to input */
  input_data[input_size] = '\0';

  token = strtok_r(input_data, "\n", &last);
  while (token != NULL) {
    int len = strlen(token);
    if ((len > 0) && (token[len-1] == '\r'))
      token[len-1] = '\0';
    IDBG_HIGH ("%s:%d] token %s", __func__, __LINE__, token);
    img_parse_extract_kv_pair(token, p_userdata, p_parse_cb);
    token = strtok_r (NULL, "\n", &last);
  }
error:
  if (input_data) {
    free(input_data);
  }
  if (fp) {
    fclose(fp);
  }
  IDBG_HIGH("%s:%d] Status %d", __func__, __LINE__, rc);
  return rc;
}

/**
 * Function: img_plane_deinterleave.
 *
 * Description: Deinterleave single plane YUV format to
 *         semi-planar.
 *
 * Arguments:
 *   @p_src_buff - Pointer to src buffer.
 *   @type: format of the src buffer
 *   @p_frame - Pointer to face component frame where converted
 *     frame will be stored.
 *
 * Return values:
 *   IMG error codes.
 *
 * Notes: conversion to planar formats is not supported
 **/
int img_plane_deinterleave(uint8_t *p_src, img_plane_type_t type,
  img_frame_t *p_frame)
{
  uint8_t *in = p_src;
  int width = QIMG_WIDTH(p_frame, 0);
  int height = QIMG_HEIGHT(p_frame, 0);
  uint32_t size = width * height * 2;
  uint32_t i = 0;
  uint8_t *out;
  uint8_t *out1;

  switch (type) {
  case PLANE_Y_CB_Y_CR:
  case PLANE_Y_CR_Y_CB:
    out = QIMG_ADDR(p_frame, 0);
    out1 = QIMG_ADDR(p_frame, 0) + QIMG_LEN(p_frame, 0);
    break;
  case PLANE_CB_Y_CR_Y:
  case PLANE_CR_Y_CB_Y:
    out1 = QIMG_ADDR(p_frame, 0);
    out = QIMG_ADDR(p_frame, 0) + QIMG_LEN(p_frame, 0);
    break;
  default:
    IDBG_ERROR("%s:%d] Format not supported", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  for (i = 0; i < size>>5; i++) {
    __asm__ __volatile__ (
      "vld4.u8 {d0-d3}, [%2]! \n"
      "vst2.u8 {d0,d2}, [%0]! \n"
      "vst2.u8 {d1,d3}, [%1]! \n"
      :"+r"(out), "+r"(out1), "+r"(in)
      :
      :"memory","d0","d1","d2","d3"
    );
  }

  return IMG_SUCCESS;
}

/**
 * Function: img_boost_linear_k_c.
 *
 * Description: C Function to boost luma
 *
 * Arguments:
 *   @p_src_buff - Pointer to src buffer.
 *   @width: frame width
 *   @height: frame height
 *   @stride: frame stride
 *   @K: boost factor
 *
 * Return values:
 *   None
 *
 **/
void img_boost_linear_k_c(uint8_t *p_src, uint32_t width, uint32_t height,
  int32_t stride, float K)
{
  uint32_t i, j;
  uint8_t *p_temp = p_src;

  for (i = 0; i < height; i++) {
    p_temp = p_src + (stride * i);
    for (j = 0; j < width; j++) {
      p_temp[j] = MIN((K * p_temp[j]), 255);
    }
  }
}

extern void boost_line_asm(uint8_t *p_src,
  int32_t  length,
  uint8_t  K,
  uint8_t *p_dst);

/**
 * Function: img_boost_linear_k
 *
 * Description: API to boost luma
 *
 * Arguments:
 *   @p_src_buff - Pointer to src buffer.
 *   @width: frame width
 *   @height: frame height
 *   @stride: frame stride
 *   @K: boost factor
 *   @use_asm: indicates whether assembly of C routine needs to
 *           be used
 *
 * Return values:
 *   None
 *
 **/
void img_boost_linear_k(uint8_t *p_src, uint32_t width, uint32_t height,
  int32_t stride, float K, int8_t use_asm)
{
  if (use_asm) {
    uint32_t K_int = (uint32_t)K;
    boost_line_asm(p_src, stride * height, K_int, p_src);
  } else {
    img_boost_linear_k_c(p_src, width, height, stride, K);
  }
}

/**
 * Function: img_common_get_orientation_angle
 *
 * Description: This function returns the orientation angle
 *   based on gravity info
 *
 * Arguments:
 *   @gravity: gravity information
 *   @p_orientation_angle: pointer to fill orientation angle
 *
 * Return values:
 *    IMG_xx error codes
 *
 **/
int img_common_get_orientation_angle(float *gravity,
  int32_t *p_orientation_angle)
{
  if (!gravity || !p_orientation_angle) {
    return IMG_ERR_INVALID_INPUT;
  }

  IDBG_LOW("Gravity vector [0]=%f, [1]=%f, [2]=%f",
    gravity[0], gravity[1], gravity[2]);

  if (fabs(gravity[2]) > 6.0f) {
    // If the z-access value is more than threshold value,
    // we cannot determine/use the orientation angle.
    // set -1 to say invalid device orientation
    *p_orientation_angle = -1;
    return IMG_SUCCESS;
  }

  // gravity values range : [-9.806287, 9.806287]
  float gravity_max = 9.8f;
  float gravity_min = -9.8f;

  CLIP(gravity[0], gravity_min, gravity_max);
  CLIP(gravity[1], gravity_min, gravity_max);

  // Note : Find a better way to get the angle
  if ((gravity[0] >= 0.0f) && (gravity[1] >= 0.0f)) {
    // 0 -90 range
    // From 0 to 90 degress, the value move :
    // gravity[0] : from 0 to gravity_max
    // gravity[1] : from gravity_max to 0
    *p_orientation_angle = (int32_t)((gravity[0] / gravity_max) * 90);
  } else if ((gravity[0] >= 0.0f) && (gravity[1] <= 0.0f)) {
    // 90 - 180 range
    // From 90 to 180 degress, the value move :
    // gravity[0] : from gravity_max to 0
    // gravity[1] : from 0 to gravity_min
    *p_orientation_angle = 180 - (int32_t)((gravity[0] / gravity_max) * 90);
  } else if ((gravity[0] <= 0.0f) && (gravity[1] <= 0.0f)) {
    // 180 - 270 range
    // From 180 to 270 degress, the value move :
    // gravity[0] : from 0 to gravity_min
    // gravity[1] : from gravity_min to 0
    *p_orientation_angle =
      180 + (int32_t)((fabs(gravity[0]) / gravity_max) * 90);
  } else if ((gravity[0] <= 0.0f) && (gravity[1] >= 0.0f)) {
    // 270 - 360 range
    // From 270 to 360 degress, the value move :
    // gravity[0] : from gravity_min to 0
    // gravity[1] : from 0 to gravity_max
    *p_orientation_angle =
      360 - (int32_t)((fabs(gravity[0]) / gravity_max) * 90);
  }

  IDBG_LOW("angle = %d", *p_orientation_angle);

  return IMG_SUCCESS;
}

/** img_common_align_gravity_to_camera:
 *
 * Description: This function aligns the gravity data to match the
 *   camera coordinate system.
 *
 * Arguments:
 *    @gravity: gravity information
 *    @sensor_mount_angle: camera mount angle (0, 90, 180, 270 degrees)
 *    @camera_position: camera position (front or back)
 *
 * Return values:
 *    IMG_xx error codes
 **/
int img_common_align_gravity_to_camera(float *gravity,
  uint32_t sensor_mount_angle, int camera_pos)
{
  int ret = IMG_SUCCESS;
  float temp;
  cam_position_t camera_position = (cam_position_t)camera_pos;

  if (!gravity) {
    return IMG_ERR_INVALID_INPUT;
  }

  if ((camera_position == CAM_POSITION_BACK) ||
    (camera_position == CAM_POSITION_BACK_AUX)) {
    switch (sensor_mount_angle) {
    case 90:
      /* No Negation.  No axes swap */
      break;

    case 180:
      /* Negate x.  Swap x, y axes */
      temp = gravity[0];
      gravity[0] = gravity[1];
      gravity[1] = -temp;
      break;

    case 270:
      /* Negate x, y.  No axes swap */
      gravity[0] = -gravity[0];
      gravity[1] = -gravity[1];
      break;

    case 0:
      /* Negate y.  Swap x, y axes */
      temp = gravity[0];
      gravity[0] = -gravity[1];
      gravity[1] = temp;
      break;

    default:
      ret = IMG_ERR_INVALID_INPUT;
    }
  } else if ((camera_position == CAM_POSITION_FRONT) ||
  (camera_position == CAM_POSITION_FRONT_AUX)){
    switch (sensor_mount_angle) {
    case 90:
      /* Negate y, z.  No axes swap */
      gravity[1] = -gravity[1];
      gravity[2] = -gravity[2];
      break;

    case 180:
      /* Negate x, y, z.  Swap x, y axes */
      temp = gravity[0];
      gravity[0] = -gravity[1];
      gravity[1] = -temp;
      gravity[2] = -gravity[2];
      break;

    case 270:
      /* Negate x, z.  No axes swap */
      gravity[0] = -gravity[0];
      gravity[2] = -gravity[2];
      break;

    case 0:
      /* Negate z.  Swap x, y axes */
      temp = gravity[0];
      gravity[0] = gravity[1];
      gravity[1] = temp;
      gravity[2] = -gravity[2];
      break;

    default:
      ret = IMG_ERR_INVALID_INPUT;
    }
  } else {
    ret = IMG_ERR_INVALID_INPUT;
  }

  return ret;
}

/**
 * Function: img_common_handle_input_frame_cache_op
 *
 * Description: Handle input frame cache operations
 *   based on buffer_access flag
 *
 * Arguments:
 *   @exec_mode: Current module's execution mode
 *   @p_frame: img frame
 *   @ion_fd: ion fd handle
 *   @p_buffer_access: input buffer access flags. This function will update
 *     the same pointer with updated buffer access based on cache operation
 *     this function does.
 *
 * Return values:
 *    img error
 *
 * Notes: None
 **/
int32_t img_common_handle_input_frame_cache_op(
  img_comp_execution_mode_t exec_mode, img_frame_t* p_frame,
  int ion_fd, bool force_cache_op, uint32_t *p_buffer_access)
{
  if (!p_buffer_access)
    return IMG_ERR_GENERAL;

  uint32_t buffer_access = *p_buffer_access;
  int rc = IMG_SUCCESS;
  IDBG_HIGH("CAMCACHE : Input : force_cache_op=%d, exec_mode=%s, "
    "(vaddr=%p, fd=%d, len=%d) ion_device_fd=%d, buffer_access=%s(0x%x)",
    force_cache_op,
    (exec_mode == IMG_EXECUTION_HW) ? "HW" :
    (exec_mode == IMG_EXECUTION_SW) ? "SW" :
    (exec_mode == IMG_EXECUTION_SW_HW) ? "SW_HW" : "UNKNOWN",
    IMG_ADDR(p_frame), IMG_FD(p_frame), IMG_FRAME_LEN(p_frame),
    ion_fd,
    (buffer_access == 0) ? "NONE" :
    (buffer_access == CPU_HAS_READ) ? "READ" :
    (buffer_access == CPU_HAS_WRITTEN) ? "WRITTEN" :
    "READ_WRITTEN", buffer_access);

  // debug setting
  if (force_cache_op) {
    rc = img_cache_ops_external(IMG_ADDR(p_frame), IMG_FRAME_LEN(p_frame), 0,
      IMG_FD(p_frame), IMG_CACHE_INV, ion_fd);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Cache invalidate fail %d", rc);
    }
    *p_buffer_access = 0;

    if (exec_mode != IMG_EXECUTION_HW) {
      // If not HW module, we are going to read the input buffer
      // through cache, set READ flag
      *p_buffer_access = CPU_HAS_READ;
    }
    return IMG_SUCCESS;
  }

  // We are going to read this buffer while executing the algorithm.
  // If we are a HW module, we will be reading main memory directly,
  // so If a previous module writes the buffer through CPU, the contents
  // might have been cached. We need to make sure to Flush cache
  // before reading.
  if ((exec_mode != IMG_EXECUTION_SW) &&
    ((buffer_access & CPU_HAS_WRITTEN) == CPU_HAS_WRITTEN)) {
    // Flush the cache.
    // no need to invalidate, as this module wont write anything into
    // input buffer, so cache contents are still valid

    rc = img_cache_ops_external(IMG_ADDR(p_frame), IMG_FRAME_LEN(p_frame),
      0, IMG_FD(p_frame), IMG_CACHE_CLEAN, ion_fd);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Cache invalidate fail %d", rc);
    }
    buffer_access &= ~CPU_HAS_WRITTEN;
    // Inidcate that the buffer is still cached.
    buffer_access |= CPU_HAS_READ;
  } else if (exec_mode != IMG_EXECUTION_HW) {
    // We are going to read the input buffer through CPU.
    // set the buffer_access flag to READ so that when we do ACK, upstream
    // module knows that buffer is cached.
    buffer_access |= CPU_HAS_READ;
  }

  *p_buffer_access = buffer_access;

  return rc;

}

/**
 * Function: img_common_handle_output_frame_cache_op
 *
 * Description: Handle output frame cache operations
 *   based on buffer_access flag
 *
 * Arguments:
 *   @exec_mode: Current module's execution mode
 *   @p_frame: img frame
 *   @ion_fd: ion fd handle
 *   @p_buffer_access: input buffer access flags. This function will update
 *     the same pointer with updated buffer access based on cache operation
 *     this function does.
 *
 * Return values:
 *    img error
 *
 * Notes: None
 **/
int32_t img_common_handle_output_frame_cache_op(
  img_comp_execution_mode_t exec_mode, img_frame_t* p_frame,
  int ion_fd, bool force_cache_op, uint32_t *p_buffer_access)
{
  if (!p_buffer_access)
    return IMG_ERR_GENERAL;

  uint32_t buffer_access = *p_buffer_access;
  int rc = IMG_SUCCESS;

  IDBG_HIGH("CAMCACHE : Output : force_cache_op=%d, exec_mode=%s, "
    "(vaddr=%p, fd=%d, len=%d) ion_device_fd=%d, buffer_access=%s(0x%x)",
    force_cache_op,
    (exec_mode == IMG_EXECUTION_HW) ? "HW" :
    (exec_mode == IMG_EXECUTION_SW) ? "SW" :
    (exec_mode == IMG_EXECUTION_SW_HW) ? "SW_HW" : "UNKNOWN",
    IMG_ADDR(p_frame), IMG_FD(p_frame), IMG_FRAME_LEN(p_frame),
    ion_fd,
    (buffer_access == 0) ? "NONE" :
    (buffer_access == CPU_HAS_READ) ? "READ" :
    (buffer_access == CPU_HAS_WRITTEN) ? "WRITTEN" :
    "READ_WRITTEN", buffer_access);

  // debug setting
  if (force_cache_op) {
    rc = img_cache_ops_external(IMG_ADDR(p_frame), IMG_FRAME_LEN(p_frame), 0,
      IMG_FD(p_frame), IMG_CACHE_INV, ion_fd);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Cache invalidate fail %d", rc);
    }
    *p_buffer_access = 0;

    if (exec_mode != IMG_EXECUTION_HW) {
      // If not HW module, we are going to write the output buffer
      // through cache, set WRITE flag
      *p_buffer_access = CPU_HAS_WRITTEN;
    }
    return IMG_SUCCESS;
  }

  if ((exec_mode != IMG_EXECUTION_SW) && (buffer_access)) {
    // If I am a HW module and the memory is already cached, make sure to
    // invalidate the cache  if previous module has red something through CPU

    // No need to flush the current cache, as we start fresh
    rc = img_cache_ops_external(IMG_ADDR(p_frame), IMG_FRAME_LEN(p_frame), 0,
      IMG_FD(p_frame), IMG_CACHE_INV, ion_fd);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("Cache invalidate fail %d", rc);
    }
    buffer_access = 0;
  }

  if (exec_mode != IMG_EXECUTION_HW) {
    // If I am a SW module, make sure to set WRITE flag to indicate
    // downstream modules that the memory is cached.
    buffer_access |= CPU_HAS_WRITTEN;
  }

  *p_buffer_access = buffer_access;

  return rc;
}

/**
 * Function: img_common_handle_release_frame_cache_op
 *
 * Description: Handle cache operations while releasing the buf
 *
 * Arguments:
 *   @p_frame: img frame
 *   @ion_fd: ion fd handle
 *   @p_buffer_access: input buffer access flags. This function will update
 *     the same pointer with updated buffer access based on cache operation
 *     this function does.
 *
 * Return values:
 *    img error
 *
 * Notes: None
 **/
int32_t img_common_handle_release_frame_cache_op(
  img_frame_t* p_frame, int ion_fd, uint32_t *p_buffer_access)
{
  if (!p_buffer_access)
    return IMG_ERR_GENERAL;

  uint32_t buffer_access = *p_buffer_access;
  int rc = IMG_SUCCESS;

  IDBG_HIGH("CAMCACHE : Release : "
    "(vaddr=%p, fd=%d, len=%d) ion_device_fd=%d, buffer_access=%s(0x%x)",
    IMG_ADDR(p_frame), IMG_FD(p_frame), IMG_FRAME_LEN(p_frame),
    ion_fd,
    (buffer_access == 0) ? "NONE" :
    (buffer_access == CPU_HAS_READ) ? "READ" :
    (buffer_access == CPU_HAS_WRITTEN) ? "WRITTEN" :
    "READ_WRITTEN", buffer_access);

  if (buffer_access) {
    // We are releasing the buffer and we can not save current
    // cache state in internal buffer manager.
    // Make sure to invalidate buffers if buffer_access is not 0
    img_cache_ops_external(IMG_ADDR(p_frame), IMG_FRAME_LEN(p_frame), 0,
      IMG_FD(p_frame), IMG_CACHE_INV, ion_fd);
    buffer_access = 0;
  }

  *p_buffer_access = buffer_access;

  return rc;
}

/**
 * Function: img_common_handle_bufdone_frame_cache_op
 *
 * Description: Handle cache operations while doing buf done
 *
 * Arguments:
 *   @p_frame: img frame
 *   @ion_fd: ion fd handle
 *   @p_buffer_access: input buffer access flags. This function will update
 *     the same pointer with updated buffer access based on cache operation
 *     this function does.
 *
 * Return values:
 *    img error
 *
 * Notes: None
 **/
int32_t img_common_handle_bufdone_frame_cache_op(
  img_frame_t* p_frame, int ion_fd, uint32_t *p_buffer_access)
{
  if (!p_buffer_access)
    return IMG_ERR_GENERAL;

  uint32_t buffer_access = *p_buffer_access;
  int rc = IMG_SUCCESS;

  IDBG_HIGH("CAMCACHE : BufDone : "
    "(vaddr=%p, fd=%d, len=%d) ion_device_fd=%d, buffer_access=%s(0x%x)",
    IMG_ADDR(p_frame), IMG_FD(p_frame), IMG_FRAME_LEN(p_frame),
    ion_fd,
    (buffer_access == 0) ? "NONE" :
    (buffer_access == CPU_HAS_READ) ? "READ" :
    (buffer_access == CPU_HAS_WRITTEN) ? "WRITTEN" :
    "READ_WRITTEN", buffer_access);

  img_cache_ops_t cache_ops = IMG_CACHE_NO_OP;
  if ((buffer_access & CPU_HAS_WRITTEN) == CPU_HAS_WRITTEN) {
    cache_ops = IMG_CACHE_CLEAN_INV;
  } else if (buffer_access) {
    cache_ops = IMG_CACHE_INV;
  }

  if (cache_ops != IMG_CACHE_NO_OP) {
    // Before BUF_DONE, make sure to do the required cache operations
    img_cache_ops_external(IMG_ADDR(p_frame), IMG_FRAME_LEN(p_frame), 0,
      IMG_FD(p_frame), cache_ops, ion_fd);
  }

  *p_buffer_access = buffer_access;

  return rc;
}

/**
 * Function: img_common_get_prop
 *
 * Description: This function returns property value in 32-bit
 * integer
 *
 * Arguments:
 *   @prop_name: name of the property
 *   @def_val: default value of the property
 *
 * Return values:
 *    value of the property in 32-bit integer
 *
 * Notes: API will return 0 in case of error. The API wont
 *    check for validation of the inputs. The caller must ensure
 *    that the property name and default value is correct.
 **/
int32_t img_common_get_prop(const char* prop_name,
  const char* def_val)
{
  int32_t prop_val = 0;
#ifdef _ANDROID_
  char prop[PROPERTY_VALUE_MAX];

  property_get(prop_name, prop, def_val);
  /* If all angles search is used disable device usage of device orientation */
  prop_val = atoi(prop);
#endif
  return prop_val;
}

/**
 * Function: img_common_get_frame_fmt
 *
 * Description: This function updates format required for
 *    graphic buffer map.
 *
 * Arguments:
 *   @frame_fmt: image frame format
 *   @update_format: graphic frame format
 *
 * Return values:
 *    img error
 *
 * Notes: None.
 **/
int32_t img_common_get_frame_fmt
  (img_format_t frame_fmt, int32_t *update_format) {
  switch (frame_fmt) {
  case IMG_FMT_YUV_420_NV12:
    *update_format = HAL_PIXEL_FORMAT_YCbCr_420_SP;
    break;
  case IMG_FMT_YUV_420_NV21:
    *update_format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
    break;
  case IMG_FMT_YUV_420_NV12_VENUS:
    *update_format = HAL_PIXEL_FORMAT_YCbCr_420_SP_VENUS;
    break;
  case IMG_FMT_YUV_420_NV21_VENUS:
    *update_format = HAL_PIXEL_FORMAT_YCrCb_420_SP_VENUS;
    break;
  default:
    IDBG_ERROR("Cam Format not supported %d", frame_fmt);
    return IMG_ERR_INVALID_INPUT;
  }
  IDBG_LOW("Update Format imglib frame_fmt %d BufferMap frame_fmt 0x%x",
    frame_fmt, *update_format);
  return IMG_SUCCESS;
}
