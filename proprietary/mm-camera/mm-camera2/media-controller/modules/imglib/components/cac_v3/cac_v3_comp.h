/**********************************************************************
* Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __CAC_COMP_H__
#define __CAC_COMP_H__

#include "cac.h"
#include "img_comp_priv.h"
#include "chromatix.h"
#include "cac3_interface.h"
#include "chromatix_metadata.h"
#define PAD_TO_SIZE(size, padding) (((size) + (padding) - 1) & ~((padding) - 1))

/** MODULE_MASK:
 *
 * Mask to enable dynamic logging
 **/
#undef MODULE_MASK
#define MODULE_MASK IMGLIB_CAC_SHIFT

/** interpolation_info_t
 * @trigger: Current trigger - can be Lux or gain
 * @num_of_regions: Number of interpolations regions
 * @trigger_start: array of trigger points of size
 *               num_of_regions
 * @trigger_end: array of trigger points of size
 *               num_of_regions
 * @regionStart: Start region for the current trigger
 * @gain_trend: End region for the current trigger
 * @interp_ratio: Interpolation ratio
 * @enable_idx: CAC enable region index
**/
typedef struct {
  float trigger;
  int num_of_regions;
  float *trigger_start;
  float *trigger_end;
  int regionStart;
  int regionEnd;
  float interp_ratio;
  int enable_idx;
} interpolation_info_t;

/** hysterisis_info_t
 * @norml_hyst_enabled: In the normal light hysterisis region
 * @lowl_hyst_enabled: In the low light hysterisis region
 * @prev_sampling_factor: Previous sampling factor
 * @lux_trend: Lux bases hysterisis trend
 * @gain_trend: Gain Based hysterisis trend
**/
typedef struct {
  uint8_t norml_hyst_enabled;
  uint8_t lowl_hyst_enabled;
  uint8_t prev_sampling_factor;
  hysterisis_trend lux_trend;
  hysterisis_trend gain_trend;
} rnr_hysterisis_info_t;

/** cac_comp_t
 *   @b: base component
 *   @cac_chromatix_info: CAC cfg data
 *   @rnr_chromatix_info: RNR cfg Data
 *   @skin_rnr_chromatix_info: Skin RNR cfg data
 *   @img_3A_info : 3A info
 *   @chroma_order : CBCR or CRCB
 *   @fd_data : Face detection result data
 *   @p_y_buffer: Y buffer ptr
 *   @p_c_buffer: Chroma buffer ptr
 *   @cac_enable_flag : Flag indicating id CAC is enabled
 *   @rnr_enable_flag: Flag indicating if RNR is enabled
 *   @lds_enable_flag: Flag indicating id LDS is enabled
 *   @cds_enable_flag: Flag indicating id CDS is enabled
 *   @yuyv422_to_yuv420_enable_flag: Flag indicating yuyv422 is used
 *   @userdata: user data
 *   @rnr_hysterisis_info: RNR hysterisis data
 *   @chromatix_ptr: Pointer to chromatix data
 *
 **/
typedef struct {
  /*base component*/
  img_component_t b;
  cac_v3_chromatix_info_t cac_chromatix_info;
  rnr_chromatix_info_t rnr_chromatix_info;
  skin_rnr_info_t skin_rnr_chromatix_info;
  img_3A_data_t img_3A_info;
  cac_chroma_order chroma_order;
  face_detection_data_t fd_data;
  uint8_t *p_y_buffer;
  uint8_t *p_c_buffer;
  uint8_t cac_enable_flag;
  uint8_t rnr_enable_flag;
  uint8_t lds_enable_flag;
  uint8_t cds_enable_flag;
  uint8_t yuyv422_to_yuv420_enable_flag;
  void *userdata;
  int (*cac3_cb) (void *userdata);
  hysterisis_info_t rnr_hysterisis_info;
  void *chromatix_ptr;
} cac3_comp_t;

#endif
