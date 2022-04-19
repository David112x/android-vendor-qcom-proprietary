/**********************************************************************
*  Copyright (c) 2013-2014,2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "img_meta.h"
#include "img_common.h"

/* for single frame algorithms */
#define __TEST_SINGLE_FRAME__

/* global variable to hold frame ops */
static img_frame_ops_t *g_frame_ops;

/**
 * Function: img_algo_process
 *
 * Description: Main algo process function
 *
 * Arguments:
 *   @p_in_frame: array of input frames
 *   @in_frame_cnt: input frame count
 *   @p_out_frame: array of output frames
 *   @in_frame_cnt: output frame count
 *   @p_out_frame: array of meta frames
 *   @in_frame_cnt: meta frame count
 *
 * Return values:
 *     imaging error values
 *
 * Notes: This symbol is Mandatory
 **/
int img_algo_process(void *p_context __unused,
  img_frame_t *p_in_frame[],
  int in_frame_cnt,
  img_frame_t *p_out_frame[],
  int out_frame_cnt,
  img_meta_t *p_meta[] __unused,
  int meta_cnt __unused)
{
  uint32_t i, j;
  IDBG_INFO("Frame count %d %d E", in_frame_cnt, out_frame_cnt);

#ifdef __TEST_SINGLE_FRAME__
  /*  Sample code for negative effect */
  /* input */
  uint8_t *p_in_y;
  uint8_t *p_in_c;
  /* output */
  uint8_t *p_out_y;
  uint8_t *p_out_c;
  /* dim */
  uint32_t stride;
  uint32_t height;

  /* local copy of all pointers */
  p_in_y = QIMG_ADDR(p_in_frame[0], 0);
  p_in_c = QIMG_ADDR(p_in_frame[0], 1);
  p_out_y = QIMG_ADDR(p_out_frame[0], 0);
  p_out_c = QIMG_ADDR(p_out_frame[0], 1);
  stride = QIMG_STRIDE(p_in_frame[0], 0);
  height = QIMG_HEIGHT(p_in_frame[0], 0);

  /* 0 - Y plane */
  for (i = 0; i < QIMG_HEIGHT(p_in_frame[0], 0); i++) {
    for (j = 0; j < QIMG_STRIDE(p_in_frame[0], 0); j++) {
      p_out_y[i * stride + j] = (255 - p_in_y[i * stride + j]);
    }
  }
  memcpy(p_out_c, p_in_c, QIMG_LEN(p_in_frame[0], 1));
  IDBG_INFO("Frame count  X");
#else
  for (i = 0; i < in_frame_cnt; i++) {
    g_frame_ops->dump_frame(p_in_frame[i], "test_algo_in", i, NULL);
  }
  for (i = 0; i < out_frame_cnt; i++) {
    g_frame_ops->dump_frame(p_out_frame[i], "test_algo_out", i, NULL);
  }
#endif
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_init
 *
 * Description: Create and initialize the algorithm wrapper
 *
 * Arguments:
 *   @pp_context: return context [Output parameter]
 *   @p_params: init params
 *
 * Return values:
 *     imaging error values
 *
 * Notes: This symbol is Mandatory
 **/
int img_algo_init(void **pp_context __unused,
  img_init_params_t *p_params __unused)
{
  IDBG_ERROR("");
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_deinit
 *
 * Description: deinitialize and destroy the algorithm wrapper
 *
 * Arguments:
 *   @p_context: context of the algorithm
 *
 * Return values:
 *     imaging error values
 *
 * Notes: This symbol is Mandatory
 **/
int img_algo_deinit(void *p_context __unused)
{
  IDBG_ERROR("");
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_set_frame_ops
 *
 * Description: Function to set frame operations
 *
 * Arguments:
 *   none
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
int img_algo_set_frame_ops(void *p_context __unused,
  img_frame_ops_t *p_ops)
{
  IDBG_ERROR("");
  int ret = IMG_SUCCESS;
  g_frame_ops = p_ops;
  return ret;
}
