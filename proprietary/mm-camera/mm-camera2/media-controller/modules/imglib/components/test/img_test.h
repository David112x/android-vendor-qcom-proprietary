/**********************************************************************
*  Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __IMGLIB_TEST_H__
#define __IMGLIB_TEST_H__

#include "img_common.h"
#include "img_comp.h"
#include "img_comp_factory.h"
#include "img_buffer.h"
#include "hdr.h"
#include "denoise.h"
#include "faceproc.h"
#include "cac.h"
#include "fd_chromatix.h"
#include "dual_frameproc_comp.h"

/**
 * CONSTANTS and MACROS
 **/
#define MAX_FILENAME_LEN 256
#define MAX_TEST_FRAMES 10

#define IMG_TEST_PARAM_FLASH_WEIGHT   (1<<0)
#define IMG_TEST_PARAM_DEGHOST_ENABLE (1<<1)
#define IMG_TEST_PARAM_ALL_VALID (IMG_TEST_PARAM_FLASH_WEIGHT|\
  IMG_TEST_PARAM_DEGHOST_ENABLE)

/* error macro*/
#define IMG_TEST_ERR(rc) ({ \
  if (IMG_ERROR(rc)) { \
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__); \
    goto error; \
  } \
})

/** img_test_sub_role_t
 *   @IMG_TEST_SW: Software component
 *   @IMG_TEST_HW: Hardware component
 *   @IMG_TEST_ADSP: ADSP component
 *
 *   Image test sub role component enum
 **/
typedef enum {
  IMG_TEST_SW,
  IMG_TEST_HW,
  IMG_TEST_ADSP,
} img_test_sub_role_t;

/** imglib_test_sw2d_t
 *   @mask: sw2d frame operations to apply on frame
 *   @downscale_factor: downscaling factor
 *
 *   Frameproc test SW2D module structure
 **/
typedef struct {
  img_sw2d_ops_t mask;
  uint32_t downscale_factor;
} imglib_test_sw2d_t;

/** imglib_test_t
 *   @frame: array of frames
 *   @meta_frame: array of meta data
 *   @img_test_mem_handle: handle for memory
 *   @mem_cnt: count of the buffers allocated
 *   @width: width of the frame
 *   @height: height of the frame
 *   @input_fn: input file name
 *   @out_fn: output file name
 *   @ss: subsampling type
 *   @in_count: input frame count
 *   @mutex: pointer to mutex variable
 *   @cond: pointer to condition variable
 *   @p_comp: pointer to the component ops
 *   @p_core_ops: pointer to the core ops
 *   @comp: component structure
 *   @core_ops: core ops structure
 *   @stride: stride of the image
 *   @scanline: scanline of the image
 *   @algo_index: algorithm index
 *   @main_role: main component
 *   @sub_role: sub component
 *   @tune_fn: tuning file name
 *   @input_format: input image format
 *   @sw2d_params: sw2d module params
 *
 *   Image test app structure
 **/
typedef struct {
  img_frame_t frame[MAX_TEST_FRAMES];
  img_meta_t meta_frame[MAX_TEST_FRAMES];
  img_mem_handle_t img_test_mem_handle[MAX_TEST_FRAMES];
  int mem_cnt;
  int width;
  int height;
  char *input_fn;
  char *out_fn;
  img_subsampling_t ss;
  uint32_t in_count;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  img_component_ops_t *p_comp;
  img_core_ops_t *p_core_ops;
  img_component_ops_t comp;
  img_core_ops_t core_ops;
  int stride;
  int scanline;
  int algo_index;
  img_comp_role_t main_role;
  img_test_sub_role_t sub_role;
  char *tune_fn;
  uint8_t deghost_enable;
  float flash_weight;
  uint32_t valid_test_params_mask;
  void *private;
  img_plane_type_t input_format;
  imglib_test_sw2d_t sw2d_params;
} imglib_test_t;

/** frameproc_test_t
 *   @base: base test app structure
 *   @caps: capabilities
 *   @stats_output: O/p from stats algo
 *
 *   Frameproc test app structure
 **/
typedef struct {
  imglib_test_t *base;
  img_caps_t caps;
  int stats_output;
} frameproc_test_t;

/** dual_frameproc_test_t
 *   @main_base: main base test app structure
 *   @aux_base: aux base test app structure
 *
 *   Frameproc test app structure
 **/
typedef struct {
  imglib_test_t *main_base;
  imglib_test_t *aux_base;
} dual_frameproc_test_t;

/** hdr_test_t
 *   @base: base test app structure
 *   @mode: HDR mode
 *   @analyse: flag to indicate if the analysis is supported
 *   @gamma: gamma table
 *   @out_index: index of the output buffer
 *
 *   HDR test app structure
 **/
typedef struct {
  imglib_test_t *base;
  hdr_mode_t mode;
  int analyse;
  img_gamma_t gamma;
  int out_index;
} hdr_test_t;

/** denoise_test_t
 *   @base: base test app structure
 *   @mode: Wavelet denoise mode
 *   @gamma: gamma table
 *   @low_gamma: lowlight gamma table
 *   @info_3a: 3A information
 *
 *   Denoise test app structure
 **/
typedef struct {
  imglib_test_t *base;
  wd_mode_t mode;
  img_gamma_t gamma;
  img_gamma_t low_gamma;
  wd_3a_info_t info_3a;
} denoise_test_t;

/** cac_test_t
 *   @base: base test app structure
 *   @r_gamma: R - gamma table- 1024 entires
 *   @g_gamma: G - gamma table- 1024 entires
 *   @b_gamma: B - gamma table -1024 entries
 *   @chromatix_info: Chromatix Info
 *   @chroma_order - CBCR or CRCB
 *
 *   CAC test app structure
 **/
typedef struct {
  imglib_test_t *base;
  img_gamma_t r_gamma;
  img_gamma_t g_gamma;
  img_gamma_t b_gamma;
  cac_v1_chromatix_info_t chromatix_info;
  cac_3a_info_t info_3a;
  cac_chroma_order chroma_order;
} cac_test_t;

/** faceproc_test_t
 *   @base: base test app structure
 *   @mode: Faceproc denoise mode
 *   @config: faceproc configuration
 *   @result: faceproc result
 *
 *   Faceproc test app structure
 **/
typedef struct {
  imglib_test_t *base;
  faceproc_mode_t mode;
  faceproc_config_t config;
  faceproc_result_t result;
  fd_chromatix_t *test_chromatix;
} faceproc_test_t;

/** img_test_init
 *   @p_test: pointer to the imagelib testapp
 *
 *   Imagelib test app initialization
 **/
int img_test_init(imglib_test_t *p_test);

/** img_test_fill_buffer
 *   @p_test: pointer to the imagelib testapp
 *   @index: index of the buffer to be filled
 *   @analysis: indicates if the buffer is for analysis
 *
 *   Fill the buffers for imglib test application
 **/
int img_test_fill_buffer(imglib_test_t *p_test, uint32_t index, int analysis);

/** img_test_read
 *   @p_test: pointer to the imagelib testapp
 *   @filename: name of the file from which the buffer should be
 *            read
 *   @index: index of the buffer
 *
 *   Read the image and fill the buffer for imglib test
 *   application
 **/
int img_test_read(imglib_test_t *p_test, char *filename, uint32_t index);

/** img_test_write
 *   @p_test: pointer to the imagelib testapp
 *   @filename: name of the file from which the image should be
 *            written
 *   @index: index of the buffer
 *
 *   Read the image and fill the buffer for imglib test
 *   application
 **/
int img_test_write(imglib_test_t *p_test, char *filename, uint32_t index);

/** hdr_test_execute
 *   @p_test: pointer to the imagelib testapp
 *
 *   Executes the HDR test application
 **/
int hdr_test_execute(hdr_test_t *p_test);

/** denoise_test_execute
 *   @p_test: pointer to the imagelib testapp
 *
 *   Executes the denoise test application
 **/
int denoise_test_execute(denoise_test_t *p_test);

/** faceproc_test_execute
 *   @p_test: pointer to the imagelib testapp
 *
 *   Executes the faceproc test application
 **/
int faceproc_test_execute(faceproc_test_t *p_test);

/** cac_test_execute
 *   @p_test: pointer to the imagelib testapp
 *
 *   Executes the faceproc test application
 **/
int cac_test_execute(cac_test_t *p_test);

/** dual_frameproc_test_execute
 *   @p_test: pointer to the imagelib testapp
 *
 *   Executes the dual frameproc test application
 **/
int dual_frameproc_test_execute(dual_frameproc_test_t *p_test);

/**
 * Function: bufpool_test_execute
 *
 * Description: execute bufpool test case
 *
 * Input parameters:
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int bufpool_test_execute(imglib_test_t *p_test);

#endif //__IMGLIB_TEST_H__
