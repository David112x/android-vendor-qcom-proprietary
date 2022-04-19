/**************************************************************************
* Copyright (c) 2012-2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**************************************************************************/

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <media/msm_jpeg.h>
#include "jpegdma_lib_hw.h"
#include <jpeg_lib_hw_reg.h>
#include <stdlib.h>
#include <jpeg_hw_dbg.h>

#define HW_BUF_SIZE_TBL_ENTRIES 15

static struct msm_jpeg_hw_cmds *jpegdma_lib_hw_cmd_malloc(uint16_t size)
{
  struct msm_jpeg_hw_cmds *p_hw_cmds;

  p_hw_cmds = calloc(1, sizeof(struct msm_jpeg_hw_cmds) -
    sizeof(struct msm_jpeg_hw_cmd) + size);

  return p_hw_cmds;
}

static int jpegdma_lib_hw_cmd_copy(struct msm_jpeg_hw_cmds *p_hw_cmds,
    struct msm_jpeg_hw_cmd *p_hw_cmd, uint16_t size)
{
  int idx;

  if (!p_hw_cmds || !p_hw_cmd) {
    return -1;
  }
  idx = p_hw_cmds->m;

  if(idx > 0 ) {
    return -1;
  }

  memcpy(&p_hw_cmds->hw_cmd[idx], p_hw_cmd, size);
  p_hw_cmds->m += (size / sizeof (struct msm_jpeg_hw_cmd));

  return 0;
}

static struct msm_jpeg_hw_cmds *jpegdma_lib_hw_cmd_malloc_and_copy(uint16_t
  size, struct msm_jpeg_hw_cmd *p_hw_cmd)
{
  struct msm_jpeg_hw_cmds *p_hw_cmds;

  p_hw_cmds = jpegdma_lib_hw_cmd_malloc(size);
  if (p_hw_cmds) {
    jpegdma_lib_hw_cmd_copy(p_hw_cmds, p_hw_cmd, size);
  }
  return p_hw_cmds;
}

struct msm_jpeg_hw_cmd hw_cmd_get_version = {
  /*
   * type, repeat n times, offset, mask, data or pdata
   */
  MSM_JPEG_HW_CMD_TYPE_READ, 1, JPEG_HW_VERSION_ADDR,
    JPEG_HW_VERSION_BMSK, {0},
};

void jpegdma_lib_hw_get_version (struct msm_jpeg_hw_cmd *p_hw_cmd)
{
  memcpy (p_hw_cmd, &hw_cmd_get_version, sizeof (hw_cmd_get_version));
  return;
}

static struct msm_jpeg_hw_cmd hw_cmd_get_num_pipes = {
  /*
   * type, repeat n times, offset, mask, data or pdata
   */
  MSM_JPEG_HW_CMD_TYPE_READ, 1, JPEGDMA_HW_CAPABILITY_ADDR,
  JPEGDMA_HW_CAPABILITY__NUM_PROCESSING_PIPES_BMSK, {0},
};

void jpegdma_lib_hw_get_num_pipes(struct msm_jpeg_hw_cmd *p_hw_cmd)
{
  memcpy (p_hw_cmd, &hw_cmd_get_num_pipes, sizeof (hw_cmd_get_num_pipes));
  return;
}

static struct msm_jpeg_hw_cmd hw_cmd_stop_offline[] = {
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_IRQ_CLEAR_ADDR,
      JPEGDMA_IRQ_CLEAR_BMSK, {0x10}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_IRQ_CLEAR_ADDR,
      JPEGDMA_IRQ_CLEAR_BMSK, {0x20}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_IRQ_CLEAR_ADDR,
      JPEGDMA_IRQ_CLEAR_BMSK, {0x01}},
};

struct msm_jpeg_hw_cmds *jpegdma_lib_hw_stop_offline (void)
{
  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  p_hw_cmds =
    jpegdma_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_stop_offline),
               hw_cmd_stop_offline);
  if (!p_hw_cmds) {
    return NULL;
  }

  return p_hw_cmds;
}

struct msm_jpeg_hw_cmds *jpegdma_lib_hw_stop ()
{
  struct msm_jpeg_hw_cmds *p_hw_cmds;

  p_hw_cmds = jpegdma_lib_hw_stop_offline ();

  return p_hw_cmds;
}

static struct msm_jpeg_hw_cmd hw_cmd_start[] = {
  /*
   * type, repeat n times, offset, mask, data or pdata
   */
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_IRQ_MASK_ADDR,
      JPEGDMA_IRQ_MASK_BMSK, {JPEG_IRQ_ALLSOURCES_ENABLE}},
   //Update mask after updating the kernel
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_CMD_ADDR,
   0xFFFFFFFF, {JPEGDMA_START_TRANSFER}},
};

struct msm_jpeg_hw_cmds *jpegdma_lib_hw_start()
{
  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  p_hw_cmds =
    jpegdma_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_start),
               hw_cmd_start);
  if (!p_hw_cmds) {
    return NULL;
  }

  return p_hw_cmds;
}

static struct msm_jpeg_hw_cmd hw_cmd_core_cfg[] ={

  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_CORE_CFG_ADDR,//core cfg
     JPEG_CORE_CFG_BMSK, {0x106005b}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_IRQ_MASK_ADDR,
     JPEG_IRQ_MASK_BMSK, {0xFFFFFFFF}},
};

struct msm_jpeg_hw_cmds *jpegdma_cmd_core_cfg (jpegdma_cmd_scale_cfg *
                                               p_scale_cfg,
                                               uint32_t num_pipes)
{
  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;
  uint32_t reg_val = 0;
  uint32_t scale_enabled = JPEGDMA_CORE_CFG__SCALE_0_ENABLE__SCALE_DISABLED;

  if (NULL == p_scale_cfg) {
    JPEG_HW_DBG("%s %d: Bad parameter", __func__, __LINE__);
    return NULL;
  }

  p_hw_cmds = jpegdma_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_core_cfg),
    hw_cmd_core_cfg);
  if (!p_hw_cmds) {
    return NULL;
  }
  p_hw_cmd = &p_hw_cmds->hw_cmd[0];

  if(p_scale_cfg->scale_enable) {
    JPEG_HW_DBG("%s %d:Scale Enabled", __func__, __LINE__);
    scale_enabled = JPEG_CORE_CFG__SCALE_ENABLE__SCALE_ENABLED;
  }

  reg_val = (JPEGDMA_CORE_CFG__TEST_BUS_ENABLE__TEST_BUS_ENABLED <<
    JPEGDMA_CORE_CFG__TEST_BUS_ENABLE_SHFT)|
    (JPEG_CORE_CFG__BRIDGE_ENABLE__BRIDGE_INTERFACE_ENABLED <<
    JPEGDMA_CORE_CFG__BRIDGE_ENABLE_SHFT);

  if (num_pipes > 0) {
    reg_val |= (scale_enabled << JPEGDMA_CORE_CFG__SCALE_0_ENABLE_SHFT) |
      (JPEGDMA_CORE_CFG__WE_0_ENABLE__WRITE_ENGINE_ENABLED <<
      JPEGDMA_CORE_CFG__WE_0_ENABLE_SHFT)|
      (JPEGDMA_CORE_CFG__FE_0_ENABLE__FETCH_ENGINE_ENABLED <<
      JPEGDMA_CORE_CFG__FE_0_ENABLE_SHFT);
  }

  if (num_pipes > 1) {
    reg_val |= (scale_enabled << JPEGDMA_CORE_CFG__SCALE_1_ENABLE_SHFT) |
      (JPEGDMA_CORE_CFG__WE_1_ENABLE__WRITE_ENGINE_ENABLED <<
      JPEGDMA_CORE_CFG__WE_1_ENABLE_SHFT)|
      (JPEGDMA_CORE_CFG__FE_1_ENABLE__FETCH_ENGINE_ENABLED <<
      JPEGDMA_CORE_CFG__FE_1_ENABLE_SHFT);
  }

  JPEG_HW_DBG("jpegdma_cmd_core_cfg: core cfg value = %x\n",reg_val);
  p_hw_cmd->data = reg_val;
  p_hw_cmd++;

  return p_hw_cmds;
}

static struct msm_jpeg_hw_cmd hw_cmd_fe_0_cfg[] = {
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_0_CFG_ADDR,
   JPEGDMA_FE_CFG_BMSK, {0x0180000}},//cbcr , enabled as default.
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_0_HINIT_ADDR,
   JPEGDMA_FE_RD_HINIT_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_0_HINIT_INT_ADDR,
   JPEGDMA_FE_RD_HINIT_INT_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_0_VINIT_ADDR,
   JPEGDMA_FE_RD_VINIT_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_0_VINIT_INT_ADDR,
   JPEGDMA_FE_RD_VINIT_INT_BMSK, {0}},
};

static struct msm_jpeg_hw_cmd hw_cmd_fe_1_cfg[] = {
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_1_CFG_ADDR,
   JPEGDMA_FE_CFG_BMSK, {0x0180000}},//cbcr , enabled as default.
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_1_HINIT_ADDR,
      JPEGDMA_FE_RD_HINIT_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_1_HINIT_INT_ADDR,
      JPEGDMA_FE_RD_HINIT_INT_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_1_VINIT_ADDR,
      JPEGDMA_FE_RD_VINIT_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_1_VINIT_INT_ADDR,
      JPEGDMA_FE_RD_VINIT_INT_BMSK, {0}},
};

struct msm_jpeg_hw_cmds *jpegdma_lib_hw_fe_cfg(
  jpegdma_fe_input_cfg *p_input_cfg,
  uint8_t mcus_per_blk __unused,
  uint32_t num_pipes)
{
  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;
  uint32_t reg_val = 0;
  uint8_t memory_format =0;
  uint8_t pln0_enable = 0, pln1_enable = 0, pln2_enable = 0;
  int ret;

  if (p_input_cfg->input_cbcr_order) {
    JPEG_HW_DBG("Input cbcr order = %d\n",
      p_input_cfg->input_cbcr_order);
    reg_val = ((uint32_t)p_input_cfg->input_cbcr_order <<
      JPEGDMA_FE_CFG__CBCR_ORDER_SHFT) | reg_val;
  }

  switch (p_input_cfg->plane_type) {
  case JPEGDMA_PLANE_TYPE_Y:
    memory_format = JPEGDMA_FE_CFG__PLN_BLOCK_TYPE_Y;
    break;
  case JPEGDMA_PLANE_TYPE_CB:
    memory_format = JPEGDMA_FE_CFG__PLN_BLOCK_TYPE_CB;
    break;
  case JPEGDMA_PLANE_TYPE_CR:
    memory_format = JPEGDMA_FE_CFG__PLN_BLOCK_TYPE_CR;
    break;
  case JPEGDMA_PLANE_TYPE_CBCR:
    memory_format = JPEGDMA_FE_CFG__PLN_BLOCK_TYPE_CBCR;
    break;
  }

  reg_val = ((uint32_t)memory_format <<
    JPEGDMA_FE_CFG__PLN_BLOCK_TYPE_SHFT) | reg_val;

  reg_val = (p_input_cfg->block_width << JPEGDMA_FE_CFG__BLOCK_WIDTH_SHFT) |
    reg_val;

  reg_val = (0x0F << JPEGDMA_FE_CFG__BURST_LENGTH_MAX_SHFT) | reg_val;

  reg_val = (JPEGDMA_FE_CFG__MAL_BOUNDARY__MAL_64_BYTES <<
    JPEGDMA_FE_CFG__MAL_BOUNDARY_SHFT) | reg_val;

  reg_val = (1 << JPEGDMA_FE_CFG__MAL_EN_SHFT) | reg_val;
  JPEG_HW_DBG("%s:%d] FE_CFG %x", __func__, __LINE__, reg_val);

  switch (num_pipes) {
  case 1:
    p_hw_cmds = jpegdma_lib_hw_cmd_malloc_and_copy(sizeof(hw_cmd_fe_0_cfg),
      hw_cmd_fe_0_cfg);
    if (!p_hw_cmds) {
      JPEG_HW_PR_ERR("%s:%d] Error malloc register config\n", __func__, __LINE__);
      return NULL;
    }
    p_hw_cmd = &p_hw_cmds->hw_cmd[0];

    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    p_hw_cmd->data = p_input_cfg->h_phase_0_int << JPEGDMA_FE_RD_HINIT_INT_SHFT;
    p_hw_cmd++;

    p_hw_cmd->data = p_input_cfg->h_phase_0_frac << JPEGDMA_FE_RD_HINIT_SHFT;
    p_hw_cmd++;

    p_hw_cmd->data = p_input_cfg->v_phase_0_int << JPEGDMA_FE_RD_VINIT_INT_SHFT;
    p_hw_cmd++;

    p_hw_cmd->data = p_input_cfg->v_phase_0_frac << JPEGDMA_FE_RD_VINIT_SHFT;
    p_hw_cmd++;
    break;
  case 2:
    p_hw_cmds = jpegdma_lib_hw_cmd_malloc(sizeof(hw_cmd_fe_0_cfg) +
      sizeof(hw_cmd_fe_1_cfg));
    if (!p_hw_cmds) {
      JPEG_HW_PR_ERR("%s:%d] Error malloc register config\n",
        __func__, __LINE__);
      return NULL;
    }
    p_hw_cmd = &p_hw_cmds->hw_cmd[0];

    ret = jpegdma_lib_hw_cmd_copy(p_hw_cmds, hw_cmd_fe_0_cfg,
      sizeof(hw_cmd_fe_0_cfg));
    if (ret < 0) {
      JPEG_HW_PR_ERR("%s:%d] Error copy register config\n", __func__, __LINE__);
      free(p_hw_cmds);
      return NULL;
    }

    ret = jpegdma_lib_hw_cmd_copy(p_hw_cmds, hw_cmd_fe_1_cfg,
      sizeof(hw_cmd_fe_1_cfg));
    if (ret < 0) {
      JPEG_HW_PR_ERR("%s:%d] Error copy register config\n", __func__, __LINE__);
      free(p_hw_cmds);
      return NULL;
    }

    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    p_hw_cmd->data = p_input_cfg->h_phase_0_int << JPEGDMA_FE_RD_HINIT_INT_SHFT;
    p_hw_cmd++;

    p_hw_cmd->data = p_input_cfg->h_phase_0_frac << JPEGDMA_FE_RD_HINIT_SHFT;
    p_hw_cmd++;

    p_hw_cmd->data = p_input_cfg->v_phase_0_int << JPEGDMA_FE_RD_VINIT_INT_SHFT;
    p_hw_cmd++;

    p_hw_cmd->data = p_input_cfg->v_phase_0_frac << JPEGDMA_FE_RD_VINIT_SHFT;
    p_hw_cmd++;

    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    p_hw_cmd->data = p_input_cfg->h_phase_1_int << JPEGDMA_FE_RD_HINIT_INT_SHFT;
    p_hw_cmd++;

    p_hw_cmd->data = p_input_cfg->h_phase_1_frac << JPEGDMA_FE_RD_HINIT_SHFT;
    p_hw_cmd++;

    p_hw_cmd->data = p_input_cfg->v_phase_1_int << JPEGDMA_FE_RD_VINIT_INT_SHFT;
    p_hw_cmd++;

    p_hw_cmd->data = p_input_cfg->v_phase_1_frac << JPEGDMA_FE_RD_VINIT_SHFT;
    p_hw_cmd++;
    break;
  default:
    JPEG_HW_PR_ERR("Invalid number of pipes %d\n", num_pipes);
    return NULL;
  }

  return p_hw_cmds;
}

static struct msm_jpeg_hw_cmd hw_cmd_fe_0_buffer_cfg[] = {
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_BUFFER_SIZE_0_ADDR,
     JPEG_PLN1_RD_BUFFER_SIZE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_0_STRIDE_ADDR, //PLN1_RD_STRIDE
      JPEGDMA_FE_RD_STRIDE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_0_HINIT_INT_ADDR, //PLN1 Start X
    JPEGDMA_FE_RD_VINIT_INT_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_0_VINIT_INT_ADDR, //PLN1 Start Y
    JPEGDMA_FE_RD_HINIT_INT_BMSK, {0}},
};

static struct msm_jpeg_hw_cmd hw_cmd_fe_1_buffer_cfg[] = {
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_BUFFER_SIZE_1_ADDR,
     JPEG_PLN1_RD_BUFFER_SIZE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_1_STRIDE_ADDR, //PLN1_RD_STRIDE
      JPEGDMA_FE_RD_STRIDE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_1_HINIT_INT_ADDR, //PLN1 Start X
    JPEGDMA_FE_RD_VINIT_INT_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_FE_RD_1_VINIT_INT_ADDR, //PLN1 Start Y
    JPEGDMA_FE_RD_HINIT_INT_BMSK, {0}},
};

struct msm_jpeg_hw_cmds *jpegdma_lib_hw_fe_buffer_cfg
  (jpegdma_fe_input_cfg * p_input_cfg,
  jpegdma_cmd_scale_cfg *p_scale_cfg,
  uint32_t num_pipes) {

  uint32_t reg_val = 0;
  uint32_t n_block_pattern = 0;
  uint32_t chroma_width =0;
  uint32_t chroma_height =0;
  int i = 0;
  uint32_t y_stride;
  uint32_t y_scanline;
  uint32_t height_0, height_1;
  uint32_t pln_hoffset = 0, pln_voffset = 0;
  int ret;

  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  JPEG_HW_DBG("%s:%d] w %d h_0 %d h_1 %d stride %d scanline %d pipes %d",
    __func__, __LINE__, p_input_cfg->image_width, p_input_cfg->image_height_0,
    p_input_cfg->image_height_0, p_input_cfg->stride, p_input_cfg->scanline,
    num_pipes);

  if (p_input_cfg->stride > p_input_cfg->image_width) {
    y_stride = p_input_cfg->stride;
  } else {
    y_stride = p_input_cfg->image_width;

  }
  p_input_cfg->image_width = p_input_cfg->stride;

  if (p_scale_cfg->crop_enable) {
    /* Set input input order only for cbcr plane type */
    pln_hoffset = p_scale_cfg->h_offset;
    pln_voffset = p_scale_cfg->v_offset;
    if (p_input_cfg->plane_type == JPEGDMA_PLANE_TYPE_CBCR) {
      /* Offset should be set per plane */
      pln_hoffset /= 2;
    }
  }

  switch (num_pipes) {
  case 1:
    p_hw_cmds =
      jpegdma_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_fe_0_buffer_cfg),
      hw_cmd_fe_0_buffer_cfg);
    if (!p_hw_cmds) {
      JPEG_HW_PR_ERR("%s:%d] Error malloc register config\n", __func__, __LINE__);
      return NULL;
    }
    p_hw_cmd = &p_hw_cmds->hw_cmd[0];

    //PLN0_RD_BUFFER_SIZE
    reg_val = (((p_input_cfg->image_height_0 - 1) <<
      JPEGDMA_FE_RD_BUFFER_SIZE___HEIGHT_SHFT)|
      (p_input_cfg->image_width - 1));
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    //PLN0_RD_STRIDE
    p_hw_cmd->data = y_stride;
    p_hw_cmd++;

    //PLN0_RD_X start
    p_hw_cmd->data = pln_hoffset;
    p_hw_cmd++;

    //PLN0_RD_Y start
    p_hw_cmd->data = pln_voffset;
    p_hw_cmd++;
    break;
  case 2:
    p_hw_cmds = jpegdma_lib_hw_cmd_malloc(sizeof(hw_cmd_fe_0_buffer_cfg) +
      sizeof(hw_cmd_fe_1_buffer_cfg));
    if (!p_hw_cmds) {
      JPEG_HW_PR_ERR("%s:%d] Error malloc register config\n", __func__, __LINE__);
      return NULL;
    }

    ret = jpegdma_lib_hw_cmd_copy(p_hw_cmds, hw_cmd_fe_0_buffer_cfg,
      sizeof(hw_cmd_fe_0_buffer_cfg));
    if (ret < 0) {
      JPEG_HW_PR_ERR("%s:%d] Error malloc register config\n", __func__, __LINE__);
      free(p_hw_cmds);
      return NULL;
    }

    ret = jpegdma_lib_hw_cmd_copy(p_hw_cmds, hw_cmd_fe_1_buffer_cfg,
      sizeof(hw_cmd_fe_1_buffer_cfg));
    if (ret < 0) {
      JPEG_HW_PR_ERR("%s:%d] Error malloc register config\n", __func__, __LINE__);
      free(p_hw_cmds);
      return NULL;
    }
    p_hw_cmd = &p_hw_cmds->hw_cmd[0];

    //PLN0_RD_BUFFER_SIZE
    reg_val = (((p_input_cfg->image_height_0 - 1) <<
      JPEGDMA_FE_RD_BUFFER_SIZE___HEIGHT_SHFT)|
      (p_input_cfg->image_width - 1));
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    //PLN0_RD_STRIDE
    p_hw_cmd->data = y_stride;
    p_hw_cmd++;

    //PLN0_RD_X start
    p_hw_cmd->data = pln_hoffset;
    p_hw_cmd++;

    //PLN0_RD_Y start
    p_hw_cmd->data = pln_voffset;
    p_hw_cmd++;

    //PLN1_RD_BUFFER_SIZE
    reg_val = (((p_input_cfg->image_height_1 - 1) <<
      JPEGDMA_FE_RD_BUFFER_SIZE___HEIGHT_SHFT)|
      (p_input_cfg->image_width - 1));
    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    //PLN1_RD_STRIDE
    p_hw_cmd->data = y_stride;
    p_hw_cmd++;

    //PLN1_RD_X start
    p_hw_cmd->data = pln_hoffset;
    p_hw_cmd++;

    //PLN1_RD_Y start
    p_hw_cmd->data = pln_voffset;
    p_hw_cmd++;
    break;
  default:
    JPEG_HW_PR_ERR("Invalid number of pipes %d\n", num_pipes);
    return NULL;
  }

  return p_hw_cmds;
}

static struct msm_jpeg_hw_cmd hw_cmd_scale_0_cfg[] = {
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_PP_0_SCALE_CFG_ADDR,
       JPEGDMA_PP_SCALE_CFG_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_PP_0_SCALE_PHASEH_STEP_ADDR,
       JPEGDMA_PP_SCALE_CFG_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_PP_0_SCALE_PHASEV_STEP_ADDR,
       JPEGDMA_PP_SCALE_CFG_BMSK, {0}},
};

static struct msm_jpeg_hw_cmd hw_cmd_scale_1_cfg[] = {
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_PP_1_SCALE_CFG_ADDR,
       JPEGDMA_PP_SCALE_CFG_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_PP_1_SCALE_PHASEH_STEP_ADDR,
       JPEGDMA_PP_SCALE_CFG_BMSK, {0}},
   {MSM_JPEG_HW_CMD_TYPE_WRITE, 1, JPEGDMA_PP_1_SCALE_PHASEV_STEP_ADDR,
       JPEGDMA_PP_SCALE_CFG_BMSK, {0}},
};

struct msm_jpeg_hw_cmds *jpegdma_lib_hw_scale_cfg(
  jpegdma_scale_cfg *scale_cfg,
  jpegdma_cmd_input_cfg * pIn __unused,
  uint32_t num_pipes)
{
  uint32_t reg_val = 0;
  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;
  uint8_t hor_upscale =0,vert_upscale =0;
  uint8_t hor_downscale =0, vert_downscale =0;
  int ret;

  if (scale_cfg->scale_enable) {
    hor_downscale = 1;
    vert_downscale = 1;
  }

  if (hor_downscale) {
    reg_val = reg_val | (JPEGDMA_PP_SCALE_CFG___HSCALE_ENABLE__ENABLE <<
      JPEGDMA_PP_SCALE_CFG___HSCALE_ENABLE_SHFT);
  }
  if (vert_downscale) {
    reg_val = reg_val | (JPEGDMA_PP_SCALE_CFG___VSCALE_ENABLE__ENABLE <<
      JPEGDMA_PP_SCALE_CFG___VSCALE_ENABLE_SHFT);
  }

  switch (num_pipes) {
  case 1:
    p_hw_cmds = jpegdma_lib_hw_cmd_malloc_and_copy(sizeof (hw_cmd_scale_0_cfg),
        hw_cmd_scale_0_cfg);
    if (!p_hw_cmds) {
      JPEG_HW_PR_ERR("%s:%d] Error malloc register config\n", __func__, __LINE__);
      return NULL;
    }
    p_hw_cmd = &p_hw_cmds->hw_cmd[0];

    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    p_hw_cmd->data = scale_cfg->phase_h_step_0;
    p_hw_cmd++;

    p_hw_cmd->data = scale_cfg->phase_v_step_0;
    p_hw_cmd++;
    break;
  case 2:
    p_hw_cmds = jpegdma_lib_hw_cmd_malloc(sizeof(hw_cmd_scale_0_cfg) +
      sizeof(hw_cmd_scale_1_cfg));
    if (!p_hw_cmds) {
      JPEG_HW_PR_ERR("%s:%d] Error malloc register config\n", __func__, __LINE__);
      return NULL;
    }

    ret = jpegdma_lib_hw_cmd_copy(p_hw_cmds, hw_cmd_scale_0_cfg,
      sizeof(hw_cmd_scale_0_cfg));
    if (ret < 0) {
      JPEG_HW_PR_ERR("%s:%d] Error malloc register config\n", __func__, __LINE__);
      free(p_hw_cmds);
      return NULL;
    }

    ret = jpegdma_lib_hw_cmd_copy(p_hw_cmds, hw_cmd_scale_1_cfg,
      sizeof(hw_cmd_scale_1_cfg));
    if (ret < 0) {
      JPEG_HW_PR_ERR("%s:%d] Error malloc register config\n", __func__, __LINE__);
      free(p_hw_cmds);
      return NULL;
    }
    p_hw_cmd = &p_hw_cmds->hw_cmd[0];

    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    p_hw_cmd->data = scale_cfg->phase_h_step_0;;
    p_hw_cmd++;

    p_hw_cmd->data = scale_cfg->phase_v_step_0;;
    p_hw_cmd++;

    p_hw_cmd->data = reg_val;
    p_hw_cmd++;

    p_hw_cmd->data = scale_cfg->phase_h_step_1;
    p_hw_cmd++;

    p_hw_cmd->data = scale_cfg->phase_v_step_1;
    p_hw_cmd++;
    break;
  default:
    JPEG_HW_PR_ERR("Invalid number of pipes %d\n", num_pipes);
    return NULL;
  }

  return p_hw_cmds;
}

static struct msm_jpeg_hw_cmd hw_cmd_we_0_buffer_cfg[] = {
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEGDMA_WE_PLN_WR_BUFFER_SIZE_0_ADDR,
      JPEGDMA_WE_PLN_WR_BUFFER_SIZE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEGDMA_WE_PLN_0_WR_STRIDE_ADDR,
      JPEGDMA_WE_PLN_WR_STRIDE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEGDMA_WE_PLN_0_WR_CFG_0_ADDR,
      JPEGDMA_WE_PLN_WR_CFG_1_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEGDMA_WE_PLN_0_WR_CFG_1_ADDR,
      JPEGDMA_WE_PLN_WR_CFG_1_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEGDMA_WE_PLN_0_WR_CFG_2_ADDR,
      JPEGDMA_WE_PLN_WR_CFG_2_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEGDMA_WE_PLN_0_WR_CFG_3_ADDR,
      JPEGDMA_WE_PLN_WR_CFG_3_BMSK, {0}},
};

static struct msm_jpeg_hw_cmd hw_cmd_we_1_buffer_cfg[] = {
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEGDMA_WE_PLN_WR_BUFFER_SIZE_1_ADDR,
      JPEGDMA_WE_PLN_WR_BUFFER_SIZE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEGDMA_WE_PLN_1_WR_STRIDE_ADDR,
      JPEGDMA_WE_PLN_WR_STRIDE_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEGDMA_WE_PLN_1_WR_CFG_0_ADDR,
      JPEGDMA_WE_PLN_WR_CFG_1_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEGDMA_WE_PLN_1_WR_CFG_1_ADDR,
      JPEGDMA_WE_PLN_WR_CFG_1_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEGDMA_WE_PLN_1_WR_CFG_2_ADDR,
      JPEGDMA_WE_PLN_WR_CFG_2_BMSK, {0}},
  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEGDMA_WE_PLN_1_WR_CFG_3_ADDR,
      JPEGDMA_WE_PLN_WR_CFG_3_BMSK, {0}},
};

struct msm_jpeg_hw_cmds *jpegdma_lib_hw_we_bffr_cfg (
    jpegdma_we_output_cfg * pOut,
    jpegdma_cmd_scale_cfg *scale_cfg __unused,
    uint32_t num_pipes)
{
  uint32_t l_hw_buf_height = 0;
  uint32_t nRegVal = 0;
  int i = 0;
  int ret;

  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  switch (num_pipes) {
  case 1:
    p_hw_cmds = jpegdma_lib_hw_cmd_malloc_and_copy(sizeof(hw_cmd_we_0_buffer_cfg),
      hw_cmd_we_0_buffer_cfg);
    if (!p_hw_cmds) {
      JPEG_HW_PR_ERR("%s:%d] Error malloc register config\n", __func__, __LINE__);
      return NULL;
    }
    p_hw_cmd = &p_hw_cmds->hw_cmd[0];

    //PLN0_WR_BUFFER_SIZE
    nRegVal = ((pOut->output_height_0 <<
      JPEGDMA_WE_PLN_WR_BUFFER_SIZE___HEIGHT_SHFT) |
      pOut->output_width);
    p_hw_cmd->data = nRegVal;
    JPEG_HW_DBG("we_buffer_cfg PLN0_WR_BUFFER_SIZE = %d\n", p_hw_cmd->data);
    p_hw_cmd++;

    //PLN0_WR_STRIDE
    nRegVal = pOut->stride;
    p_hw_cmd->data = nRegVal;
    JPEG_HW_DBG("we_buffer_cfg PLN0_WR_STRIDE = %d\n", p_hw_cmd->data);
    p_hw_cmd++;

    nRegVal = (((uint32_t)(pOut->blocks_per_row_0 - 1) <<
      JPEGDMA_WE_PLN_WR_CFG_0___BLOCKS_PER_ROW_SHFT) |
      (uint32_t)(pOut->blocks_per_col_0-1));
    p_hw_cmd->data = nRegVal;
    JPEG_HW_DBG("we_buffer_cfg PLN0_WR_CFG_0 = %d\n", p_hw_cmd->data);
    p_hw_cmd++;

    nRegVal = ((pOut->last_h_step - 1) <<
      JPEGDMA_WE_PLN_WR_CFG_1___LAST_H_STEP_SHFT) |
      (pOut->h_step - 1);
    p_hw_cmd->data = nRegVal;
    p_hw_cmd++;

    nRegVal = ((pOut->last_v_step_0 - 1) <<
      JPEGDMA_WE_PLN_WR_CFG_2___LAST_V_STEP_SHFT);
    p_hw_cmd->data = nRegVal;
    p_hw_cmd++;
    break;
  case 2:
    p_hw_cmds = jpegdma_lib_hw_cmd_malloc(sizeof(hw_cmd_we_0_buffer_cfg) +
      sizeof(hw_cmd_we_1_buffer_cfg));
    if (!p_hw_cmds) {
      JPEG_HW_PR_ERR("%s:%d] Error malloc register config\n", __func__, __LINE__);
      return NULL;
    }

    ret = jpegdma_lib_hw_cmd_copy(p_hw_cmds, hw_cmd_we_0_buffer_cfg,
      sizeof(hw_cmd_we_0_buffer_cfg));
    if (ret < 0) {
      JPEG_HW_PR_ERR("%s:%d] Error malloc register config\n", __func__, __LINE__);
      free(p_hw_cmds);
      return NULL;
    }

    ret = jpegdma_lib_hw_cmd_copy(p_hw_cmds, hw_cmd_we_1_buffer_cfg,
      sizeof(hw_cmd_we_1_buffer_cfg));
    if (ret < 0) {
      JPEG_HW_PR_ERR("%s:%d] Error malloc register config\n", __func__, __LINE__);
      free(p_hw_cmds);
      return NULL;
    }
    p_hw_cmd = &p_hw_cmds->hw_cmd[0];

    //PLN0_WR_BUFFER_SIZE
    nRegVal = ((pOut->output_height_0 <<
      JPEGDMA_WE_PLN_WR_BUFFER_SIZE___HEIGHT_SHFT) |
      pOut->output_width);
    p_hw_cmd->data = nRegVal;
    JPEG_HW_DBG("we_buffer_cfg PLN0_WR_BUFFER_SIZE = %d\n", p_hw_cmd->data);
    p_hw_cmd++;

    //PLN0_WR_STRIDE
    nRegVal = pOut->stride;
    p_hw_cmd->data = nRegVal;
    JPEG_HW_DBG("we_buffer_cfg PLN0_WR_STRIDE = %d\n", p_hw_cmd->data);
    p_hw_cmd++;

    nRegVal = (((uint32_t)(pOut->blocks_per_row_0 - 1) <<
      JPEGDMA_WE_PLN_WR_CFG_0___BLOCKS_PER_ROW_SHFT) |
      (uint32_t)(pOut->blocks_per_col_0-1));
    p_hw_cmd->data = nRegVal;
    JPEG_HW_DBG("we_buffer_cfg PLN0_WR_CFG_0 = %d\n", p_hw_cmd->data);
    p_hw_cmd++;

    nRegVal = ((pOut->last_h_step - 1) <<
      JPEGDMA_WE_PLN_WR_CFG_1___LAST_H_STEP_SHFT) |
      (pOut->h_step - 1);
    p_hw_cmd->data = nRegVal;
    p_hw_cmd++;

    nRegVal = ((pOut->last_v_step_0 - 1) <<
      JPEGDMA_WE_PLN_WR_CFG_2___LAST_V_STEP_SHFT);
    p_hw_cmd->data = nRegVal;
    p_hw_cmd++;

    //Leave cfg 3 with default configuration
    p_hw_cmd++;

    //PLN1_WR_BUFFER_SIZE
    nRegVal = ((pOut->output_height_1 <<
      JPEGDMA_WE_PLN_WR_BUFFER_SIZE___HEIGHT_SHFT) |
      pOut->output_width);
    p_hw_cmd->data = nRegVal;
    JPEG_HW_DBG("we_buffer_cfg PLN1_WR_BUFFER_SIZE = %d\n", p_hw_cmd->data);
    p_hw_cmd++;

    //PLN1_WR_STRIDE
    p_hw_cmd->data = pOut->stride;
    JPEG_HW_DBG("we_buffer_cfg PLN1_WR_STRIDE = %d\n", p_hw_cmd->data);
    p_hw_cmd++;

    nRegVal = (((uint32_t)(pOut->blocks_per_row_1 - 1) <<
      JPEGDMA_WE_PLN_WR_CFG_0___BLOCKS_PER_ROW_SHFT) |
      (uint32_t)(pOut->blocks_per_col_1 - 1));
    p_hw_cmd->data = nRegVal;
    JPEG_HW_DBG("we_buffer_cfg PLN1_WR_STRIDE = %d\n", p_hw_cmd->data);
    p_hw_cmd++;

    nRegVal = ((pOut->last_h_step - 1) <<
      JPEGDMA_WE_PLN_WR_CFG_1___LAST_H_STEP_SHFT) |
      (pOut->h_step - 1);
    p_hw_cmd->data = nRegVal;
    p_hw_cmd++;

    nRegVal = ((pOut->last_v_step_1 - 1) <<
      JPEGDMA_WE_PLN_WR_CFG_2___LAST_V_STEP_SHFT);
    p_hw_cmd->data = nRegVal;
    p_hw_cmd++;
    break;
  default:
    JPEG_HW_PR_ERR("Invalid number of pipes %d\n", num_pipes);
    return NULL;
  }

  return p_hw_cmds;
}

struct msm_jpeg_hw_cmd hw_cmd_we_cfg[] = {

  {MSM_JPEG_HW_CMD_TYPE_WRITE, 1,JPEGDMA_WE_CFG_ADDR,
      JPEGDMA_WE_CFG_BMSK, {0}},
};

struct msm_jpeg_hw_cmds *jpegdma_lib_hw_we_cfg (jpegdma_we_cfg *we_cfg)
{
  uint32_t nRegVal = 0;

  struct msm_jpeg_hw_cmds *p_hw_cmds;
  struct msm_jpeg_hw_cmd *p_hw_cmd;

  p_hw_cmds = jpegdma_lib_hw_cmd_malloc_and_copy (sizeof (hw_cmd_we_cfg),
      hw_cmd_we_cfg);
  if (!p_hw_cmds) {
    return NULL;
  }
  p_hw_cmd = &p_hw_cmds->hw_cmd[0];

  nRegVal = ((uint32_t)we_cfg->cbcr_order) << JPEGDMA_WE_CFG__CBCR_ORDER_SHFT;
  nRegVal = (0x0F << JPEGDMA_WE_CFG__BURST_LENGTH_MAX_SHFT) | nRegVal;

  nRegVal = (JPEGDMA_WE_CFG__MAL_BOUNDARY__MAL_64_BYTES <<
    JPEGDMA_WE_CFG__MAL_BOUNDARY_SHFT) | nRegVal;

  nRegVal = (1 << JPEGDMA_WE_CFG__MAL_EN_SHFT) | nRegVal;

  p_hw_cmd->data = nRegVal;
  JPEG_HW_DBG("%s:%d] WE_CFG %x", __func__, __LINE__, nRegVal);

  return p_hw_cmds;
}

