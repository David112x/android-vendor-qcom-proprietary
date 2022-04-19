/**********************************************************************
*  Copyright (c) 2013-2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef _CAC3_H
#define _CAC3_H

#include <stdint.h>

#define CAC3_SUCCESS (0)
#define CAC3_ERROR   (1)
#define CAC3_TRUE    ((uint32_t)(1))
#define CAC3_FALSE   ((uint32_t)(0))
#define CAC3_MAX_NUM_FACES (20) // This should be dependent on FD setting.


/*** cac3_args_t
 *   @cac_enable_flag : Flag indicating id CAC is enabled
 *   @rnr_enable_flag: Flag indicating if RNR is enabled
 *   @skin_rnr_enable_flag: Flag indicating if Skin RNR is enabled
 *   @lds_enable_flag: Flag indicating id LDS is enabled
 *   @cds_enable_flag: Flag indicating id CDS is enabled
 *   @p_ion_vaddr: ION buffer's virtual address
 *   @p_y: y component
 *   @p_crcb: cbcr component
 *   @fd: FD of the input bufer
 *   @ion_heap_id: ION heap ID
 *   @is_cached: ION cached flag 1=cached ION buffer, 0=uncached ION buffer
 *   @image_format: 0 - cbcr 1 - crcb, both in h2v2
 *   @y_width: Y plane width
 *   @y_height: Y plane height
 *   @y_stride: Image y stride
 *   @cbcr_width : chroma plance width
 *   @cbcr_height: chroma plane height
 *   @cbcr_stride: chroma plane stride
 *   @detection_th1: Bright spot High
 *   @detection_th2: saturation threshold
 *   @verification_th1: Verification threshold1
 *   @verification_th2: Verification threshold2
 *   @sampling_factor: RNR downsample/upsample factor
 *   @sigma_lut: Pointer to RNR sigma (threshold) lookup table,162 length
 *   @lut_size: Valid entries in the sigma LUT
 *   @scale_factor: Size of the sigma_lut
 *   @center_noise_sigma: center ratio
 *   @center_noise_weight: default 1.0
 *   @weight_order: 2.0f if sampling factor=2, 1.5f if sampling
 *                factor=4, 1.0f if sampling factor=8
 *   @skin_sigma_lut: Pointer to skin RNR sigma lookup table (255 entries)
 *   @skin_sigma_lut_size: Size of the skin_sigma_lut
 *   @skin_strength: Skin strength
 *   @skin_cr: Skin Cr
 *   @skin_cb: Skin CB
 *   @skin_ymin: Skin Y minimum
 *   @skin_ymax: Skin Y maximim
 *   @skin_scaler: Skin scaler
 *   @skin_sigma_scale: Skin Sigma Scale
 *   @skin_threshold: Skin Threshold
 *   @ds_us_skin_detection: Skin detection downscale factor
 *   @num_faces: Number of faces detected by FD
 *   @face_coordinates: Co-ords of the faces detected
 *   @userdata: App data passed by the client
 *   @cac3_cb: CAC3 lib early callback function pointer
 *
 *   CAC3 Arguments
 **/
typedef struct
{
  uint32_t  cac3_enable_flag; // 0 - no cac, 1 - cac
  uint32_t  rnr_enable_flag;
  uint32_t  skin_rnr_enable_flag; // 0 -no Skin RNR but RNR, 1- both SkinRNR and RNR
  uint32_t  lds_enable_flag;
  uint32_t  cds_enable_flag;
  uint32_t  yuv420_to_yuv422;  //YUV420 to YUV422 flag - set if CDS on, LDS on
  uint32_t  yuyv422_to_yuv420; //YUYV422 to YUV420 flag - 0:disable, 1:enable

  uint8_t * p_ion_vaddr;       // ION buffer's virtual address
  uint8_t * p_y;               // Pointer to start of luma   data
  uint8_t * p_crcb;            // Pointer to start of chroma data
  int       fd;
  int       is_cached;         //1=cached ION buffer, 0=uncached ION buffer
  int       ion_heap_id;
  uint32_t  image_format;
  uint32_t  y_width;
  uint32_t  y_height;
  uint32_t  y_stride;
  uint32_t  cbcr_width;
  uint32_t  cbcr_height;
  uint32_t  cbcr_stride;
  uint32_t  y_out_stride;     //Used only when YUYV422 to YUV420 flag = 1 (on)
  uint32_t  cbcr_out_stride;  //Used only when YUYV422 to YUV420 flag = 1 (on)

  // parameters for CAC tunning
  int32_t   detection_th1;
  int32_t   detection_th2;
  int32_t   verification_th1;
  int32_t   verification_th2;

  //RNR arguments
  uint8_t  sampling_factor;
  float    *sigma_lut;
  int      lut_size;
  float    scale_factor;
  float    center_noise_sigma;
  float    center_noise_weight;
  float    weight_order;

  // Skin RNR arguments
  float    *skin_sigma_lut;
  int      skin_sigma_lut_size;
  float    skin_strength; // skin_slope = 1/skin_strength
  uint8_t  skin_cr;
  uint8_t  skin_cb;
  uint8_t  skin_ymin;
  uint8_t  skin_ymax;
  float    skin_scaler;  // originally float but we convert to Q10 and store as uint16_t
  float    skin_sigma_scale;
  float    skin_threshold;
  uint8_t  ds_us_skin_detection;
  //Face Detection arguments
  uint8_t    num_faces;

  //Userdata passed from the client
  void *userdata;
  int (*cac3_cb) (void *userdata);
} cac3_args_t;

/*Functions*/
uint32_t cac3_init(int ion_heap_id);

uint32_t cac3_process(cac3_args_t * args);

uint32_t cac3_deinit();

uint32_t cac3_pre_allocate_buffers(int ion_heap_id);

uint32_t cac3_destroy_buffers();

#endif
