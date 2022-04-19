/*============================================================================

   Copyright (c) 2012, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef JPEGDMA_HW_COMMON_H
#define JPEGDMA_HW_COMMON_H

#include <unistd.h>
#include <media/msm_jpeg.h>

#define BOOL uint8_t

/*
enum GEMINI_FE_BURST_LENGTH_T
{
  GEMINI_FE_BURST_LENGTH4 = 1,
  GEMINI_FE_BURST_LENGTH8 = 2,
};

enum GEMINI_WE_BURST_LENGTH_T
{
  GEMINI_WE_BURST_LENGTH4 = 1,
  GEMINI_WE_BURST_LENGTH8 = 2,
};*/

typedef enum {
  JPEGDMA_SPEED_NORMAL = 1,
  JPEGDMA_SPEED_HIGH = 2
} JPEGDMA_SPEED_MODE_T;

/*******************************************************************/
/************************  JPEG Configuration **********************/
/*******************************************************************/

typedef struct
{


  JPEGDMA_SPEED_MODE_T speed_mode;
} jpegdma_cmd_jpeg_transfer_cfg;

/*******************************************************************/
/******************  Fetch Engine and Write Engine *****************/
/*******************************************************************/
typedef enum JPEGDMA_HW_INPUT_FORMAT
{
  JPEGDMA_INPUT_H1V1 = 0,
  JPEGDMA_INPUT_H1V2 = 1,         /**< only valid for offline transfer mode. */
  JPEGDMA_INPUT_H2V1 = 2,
  JPEGDMA_INPUT_H2V2 = 3,
  JPEGDMA_INPUT_MONOCHROME = 4,
  JPEGDMA_INPUT_FORMAT_MAX =5,
} JPEGDMA_HW_INPUT_FORMAT;

typedef enum JPEG_DMA_PLANE_TYPE
{
  JPEGDMA_PLANE_TYPE_Y,
  JPEGDMA_PLANE_TYPE_CB,
  JPEGDMA_PLANE_TYPE_CR,
  JPEGDMA_PLANE_TYPE_CBCR
}JPEGDMA_PLANE_TYPE;
typedef struct
{
  JPEGDMA_HW_INPUT_FORMAT inputFormat;
           /**< Use this information to derive others fields. */
  uint8_t  input_cbcr_order;
  /**< 0 = cb pixel in LSB of the word. 1 = cr pixel in LSB of the word.  */
  uint32_t image_width;
  uint32_t image_height;
  uint16_t plane_type;
  uint32_t stride;
  uint32_t scanline;
  uint32_t hw_buf_size;
  uint32_t block_width;
} jpegdma_cmd_input_cfg;

typedef struct
{
  JPEGDMA_HW_INPUT_FORMAT inputFormat;
           /**< Use this information to derive others fields. */
  uint8_t input_cbcr_order;
  /**< 0 = cb pixel in LSB of the word. 1 = cr pixel in LSB of the word.  */
  uint32_t image_width;
  uint32_t image_height_0;
  uint32_t image_height_1;

  uint32_t h_phase_0_int;
  uint32_t h_phase_0_frac;
  uint32_t v_phase_0_int;
  uint32_t v_phase_0_frac;
  uint32_t h_phase_1_int;
  uint32_t h_phase_1_frac;
  uint32_t v_phase_1_int;
  uint32_t v_phase_1_frac;

  uint16_t plane_type;
  uint32_t stride;
  uint32_t scanline;
  uint32_t hw_buf_size;
  uint32_t block_width;
} jpegdma_fe_input_cfg;

typedef struct
{
  JPEGDMA_HW_INPUT_FORMAT inputFormat;
           /**< Use this information to derive others fields. */
  /**< 0 = cb pixel in LSB of the word. 1 = cr pixel in LSB of the word.  */
  uint32_t output_width;
  uint32_t output_height_0;
  uint32_t output_height_1;
  uint8_t  mmu_prediction;
  uint16_t blocks_per_row_0;
  uint16_t blocks_per_col_0;
  uint16_t blocks_per_col_1;
  uint16_t blocks_per_row_1;
  uint32_t stride;
  uint32_t scanline;
  uint32_t hw_buf_size;
  uint32_t h_step;
  uint32_t last_h_step;
  uint32_t last_v_step_0;
  uint32_t last_v_step_1;

} jpegdma_we_output_cfg;

typedef struct
{
    uint8_t     scale_enable;
    uint32_t    scale_input_width;   // input width
    uint32_t    scale_input_height;  // input height
    uint32_t    h_offset;      // h_offset
    uint32_t    v_offset;      // v_offset
    uint32_t    output_width;  // output width
    uint32_t    output_height; // output height
    uint8_t     crop_enable;
    uint32_t    output_stride;
    uint32_t    output_scanline;
}jpegdma_cmd_scale_cfg;

typedef struct
{
    uint32_t    phase_h_step_0;
    uint32_t    phase_v_step_0;
    uint32_t    phase_h_step_1;
    uint32_t    phase_v_step_1;
    uint8_t     scale_enable;

}jpegdma_scale_cfg;

typedef struct {
  uint8_t cbcr_order;
} jpegdma_we_cfg;

/********************************************************************/
/************************* JPEG Messages ****************************/
/********************************************************************/

typedef enum
{
  JPEGDMA_EVT_SESSION_DONE = MSM_JPEG_EVT_SESSION_DONE,
  JPEGDMA_EVT_ERROR = MSM_JPEG_EVT_ERR,
  JPEGDMA_EVT_BUS_ERROR = MSM_JPEG_EVT_ERR,
  JPEGDMA_EVT_VIOLATION = MSM_JPEG_EVT_ERR,
  JPEGDMA_EVT_MAX
} JPEGDMA_EVT_ID;


#endif /*  JPEGDMA_HW_COMMON_H */
