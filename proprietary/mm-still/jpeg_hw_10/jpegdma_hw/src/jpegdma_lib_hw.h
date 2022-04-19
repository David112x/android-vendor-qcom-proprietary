/**************************************************************************
*                                                                         .
* Copyright (c) 2012-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential                      .
*                                                                         .
***************************************************************************/

#ifndef JPEGDMA_LIB_HW_H
#define JPEGDMA_LIB_HW_H

#include <unistd.h>
#include <media/msm_jpeg.h>
#include "jpegdma_lib_common.h"

#define JPEG_8974_V1 0x10000000
#define JPEG_8974_V2 0x10010000

#define IS_8974_V2(v) ((v) == JPEG_8974_V2)

#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
#define CEILING8(X)  (((X) + 0x0007) & 0xFFF8)
#define CEILING2(X)  (((X) + 0x0001) & 0xFFFE)
#define FLOOR16(X) ((X) & 0xFFF0)
#define FLOOR8(X)  ((X) & 0xFFF8)
#define FLOAT_TO_Q(exp, f) \
  ((int32_t)(((f)*(1<<(exp))) + (((f)<0) ? -0.5 : 0.5)))

void jpegdma_lib_hw_get_version(struct msm_jpeg_hw_cmd *p_hw_cmd);

void jpegdma_lib_hw_get_num_pipes(struct msm_jpeg_hw_cmd *p_hw_cmd);

struct msm_jpeg_hw_cmds *jpegdma_lib_hw_stop();

struct msm_jpeg_hw_cmds *jpegdma_lib_hw_start();

struct msm_jpeg_hw_cmds *jpegdma_lib_hw_fe_cfg(
    jpegdma_fe_input_cfg *p_input_cfg, uint8_t mcus_per_blk,
    uint32_t num_pipes);

struct msm_jpeg_hw_cmds *jpegdma_lib_hw_fe_buffer_cfg(
    jpegdma_fe_input_cfg *p_input_cfg, jpegdma_cmd_scale_cfg *scale_cfg,
    uint32_t num_pipes);

struct msm_jpeg_hw_cmds *jpegdma_lib_hw_transfer_cfg(
  jpegdma_cmd_input_cfg * pIn,
  jpegdma_cmd_scale_cfg *scale_cfg);

struct msm_jpeg_hw_cmds *jpegdma_lib_hw_we_cfg(jpegdma_we_cfg *we_cfg);


struct msm_jpeg_hw_cmds *jpegdma_lib_hw_we_bffr_cfg(
  jpegdma_we_output_cfg *pOut,
  jpegdma_cmd_scale_cfg *scale_cfg,
  uint32_t num_pipes);


struct msm_jpeg_hw_cmds *jpegdma_lib_hw_scale_cfg(
  jpegdma_scale_cfg *scale_cfg,
  jpegdma_cmd_input_cfg *pIn,
  uint32_t num_pipes);


struct msm_jpeg_hw_cmds *jpegdma_cmd_core_cfg(
    jpegdma_cmd_scale_cfg *p_scale_cfg,
    uint32_t num_pipes);



#endif /* JPEGDMA_LIB_HW_H */
