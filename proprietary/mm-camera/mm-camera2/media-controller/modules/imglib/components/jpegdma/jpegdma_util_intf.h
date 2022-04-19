/*************************************************************
* Copyright (c) 2016 Qualcomm Technologies, Inc.             *
* All Rights Reserved.                                       *
* Confidential and Proprietary - Qualcomm Technologies, Inc. *
*************************************************************/

#ifndef __JPEGDMA_UTIL_INTERFACE_H__
#define __JPEGDMA_UTIL_INTERFACE_H__

#include "jpegdma_comp.h"
#include "cam_types.h"
#include "img_common.h"
#include "img_dbg.h"
#include <stdbool.h>
#include <poll.h>

/**
 * Function: jpegdma_util_get_dev_name
 *
 * Description: Go thru each "/dev/videoN" and search for v4l2
 *   jpeg dma device name.
 *
 * Input parameters:
 *   @p_drv_name: Char array where jpeg dma device name
 *   should be stored.
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t jpegdma_util_get_dev_name(char **p_drv_name);

/**
 * Function: jpegdma_util_get_max_scale
 *
 * Description: Get max downscale factor.
 *
 * Input parameters:
 *   @fh: JPEG DMA V4L2 driver file handle.
 *   @p_ds_factor: Max downscale factor.
 *
 * Return values:
 *   IMG_SUCCESS/IMG_ERR.
 *
 * Notes: none
 **/
int32_t jpegdma_util_get_max_scale(int fh, float *p_ds_factor);

/**
 * Function: jpegdma_util_start_thread
 *
 * Description: function to stop worker thread
 *
 * Input parameters:
 *   @p_comp: Pointer to jpegdma component.
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
void jpegdma_util_start_thread_loop(jpegdma_comp_t *p_comp);

/**
 * Function: jpegdma_util_stop_thread
 *
 * Description: function to stop worker thread
 *
 * Input parameters:
 *   @p_comp: Pointer to jpegdma component.
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t jpegdma_util_stop_thread_loop(jpegdma_comp_t *p_comp);

/**
 * Function: jpegdma_util_start
 *
 * Description: function to start util with input params
 *
 * Input parameters:
 *   @p_comp: Pointer to jpegdma component.
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: This function is blocking
 **/
int32_t jpegdma_util_start(jpegdma_comp_t *p_comp);

/**
 * Function: jpegdma_util_abort
 *
 * Description: function to start util with input params
 *
 * Input parameters:
 *   @p_comp: Pointer to jpegdma component.
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: This function is blocking
 **/
int32_t jpegdma_util_abort(jpegdma_comp_t *p_comp);

/**
 * Function: jpegdma_util_send_buffers
 *
 * Description: This function is used to send frame buffer for processing
 *
 * Input parameters:
 *   @handle - The pointer to the component handle.
 *   @p_buff - The buffer which needs to be processed
 *   @type: buffer type
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_OPERATION
 *
 *
 **/
int32_t jpegdma_util_send_buffers(jpegdma_comp_t *p_comp, void *p_buff,
  jpegdma_buffer_type_t buff_type);
#endif
