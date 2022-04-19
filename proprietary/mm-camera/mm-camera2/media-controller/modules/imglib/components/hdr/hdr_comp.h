/**********************************************************************
* Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __HDR_COMP_H__
#define __HDR_COMP_H__

#include "img_comp_priv.h"

#if HDR_LIB_GHOSTBUSTER
#include "hdr_gb_lib.h"
#else
#include "hdr_lib.h"
#endif

#include "hdr.h"
#include "hdr_chromatix.h"

#define GAMMA_TABLE_SIZE_HDR 64
#define MAX_GAMMA_INTERPOLATED 8192

/** MODULE_MASK:
 *
 * Mask to enable dynamic logging
 **/
#undef MODULE_MASK
#define MODULE_MASK IMGLIB_HDR_SHIFT

/** hdr_lib_gamma_parameters_t
 *   @hdr_gamma_R: gamma table struct R
 *   @hdr_gamma_G: gamma table struct G
 *   @hdr_gamma_B: gamma table struct B
 *   @red_gamma_table: Red gamma table 16 64 entry
 *   @blue_gamma_table: Blue gamma table 16 64 entry
 *   @green_gamma_table: Green gamma table 16 64 entry
 *
 *   HDR gamma parameter structure for algorithm
 **/

typedef struct {
  hdr_gamma_table_struct_t hdr_gamma_R;
  hdr_gamma_table_struct_t hdr_gamma_G;
  hdr_gamma_table_struct_t hdr_gamma_B;
  uint16_t red_gamma_table[GAMMA_TABLE_SIZE_HDR];
  uint16_t blue_gamma_table[GAMMA_TABLE_SIZE_HDR];
  uint16_t green_gamma_table[GAMMA_TABLE_SIZE_HDR];
} hdr_lib_gamma_parameters_t;

/** hdr_comp_t
 *   @b: base image component
 *   @gamma_info_count: counter for gamma frame info
 *   @gamma: gamma tables for the HDR for each input frame
 *   @param: hdr tables required for configuration
 *   @structHdrConfig: hdr configuration for the library
 *   @count: number of buffers send by the user
 *   @mode: hdr mode of operation
 *   @p_main_frame: pointer to array of main frame buffers
 *   @p_analysis_frame: pointer to array of analysis buffers
 *   @analyse_image: Flag to indicate if analysis frame is
 *                 present
 *   @main_count: number or main image buffers
 *   @out_index: Index of the buffer to be used as output buffer
 *   @out_crop: Output image crop
 *   @hdr_chromatix: hdr chromatix
 *   @aec_info_count: counter for aec frame info
 *   @aec_info: aec info for each input frame
 *
 *   HDR component structure
 **/
typedef struct {
  /*base component*/
  img_component_t b;
  int gamma_info_count;
  hdr_gamma_table_struct_t gamma[MAX_HDR_FRAMES];
  hdr_param_t param;
  hdr_config_t structHdrConfig;
  int count;
  hdr_mode_t mode;
  img_frame_t *p_main_frame[MAX_HDR_FRAMES];
  img_frame_t *p_analysis_frame[MAX_HDR_FRAMES];
  int analyse_image;
  int main_count;
  int out_index;
  hdr_crop_t out_crop;
  hdr_chromatix_t hdr_chromatix;
  int aec_info_count;
  img_aec_info_t aec_info[MAX_HDR_FRAMES];
  hdr_lib_gamma_parameters_t g_hdr_gamma;
  void *p_hdr_ctx;
} hdr_comp_t;

#endif //__HDR_COMP_H__

