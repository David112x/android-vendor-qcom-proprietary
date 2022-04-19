/*************************************************************
* Copyright (c) 2016 Qualcomm Technologies, Inc.             *
* All Rights Reserved.                                       *
* Confidential and Proprietary - Qualcomm Technologies, Inc. *
*************************************************************/

#ifndef __JPEGDMA_COMP_H__
#define __JPEGDMA_COMP_H__

#include "img_comp_priv.h"
#include "cam_types.h"

/** jpegdma_buffer_type_t
 *   @JDMA_IN_BUFFER: buffer type input.
 *   @JDMA_OUT_BUFFER: buffer type output.
 *   @JDMA_META_BUFFER: buffer type meta
 *
 *   Buffer types.
 */
typedef enum {
  JDMA_IN_BUFFER,
  JDMA_OUT_BUFFER,
  JDMA_META_BUFFER,
} jpegdma_buffer_type_t;

/** jpegdma_comp_t
 *   @src_dim: source dimension
 *   @dest_dim: destination dimension
 *   @yuv_format: YUV format
 *   @crop: crop
 *   @framerate: framerate
 *   @src_buf_sz: source buffer size
 *   @dest_buf_sz: destination buffer size
 *   @max_ds_factor: max downscale factor
 *
 **/
typedef struct {
  img_dim_t src_dim;
  img_dim_t dest_dim;
  cam_format_t yuv_format;
  img_rect_t crop;
  uint32_t framerate;
  uint32_t src_buf_sz;
  uint32_t dest_buf_sz;
  float max_ds_factor;
} jpegdma_params_t;

/** jpegdma_comp_t
 *   @b: base component
 *   @p_drv_name: Pointer to jpeg dma device name
 *   @drv_fd: driver file descriptor
 *   @msg_pipefd: message pipe
 *   @ack_pipefd: message ack pipe
 *   @params: params
 *   @toProcessQ: Queue for img bundles to be processed
 *   @inProcessQ: Queue for img bundles being processed
 *   @userdata: user data
 *   @jpegdma_cb: callback function
 *
 **/
typedef struct {
  img_component_t b;
  char *p_drv_name;
  int32_t fd_drv;
  int msg_pipefd[2];
  int ack_pipefd[2];
  jpegdma_params_t params;
  img_queue_t toProcessQ;
  img_queue_t inProcessQ;
  void *userdata;
  int (*jpegdma_cb) (void *userdata, img_frame_bundle_t *p_frame_bundle,
    img_event_type cb_event);
} jpegdma_comp_t;

#endif
