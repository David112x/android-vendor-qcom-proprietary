/**********************************************************************
*  Copyright (c) 2013-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "img_test.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif
#include <stdbool.h>

/**
 *  dynamic loglevel
 **/
extern volatile uint32_t g_imgloglevel;
extern volatile uint32_t g_imgLogModuleMask;


#define UTIL_BUF_POOL_TEST 1

/**
 * chromaflash parameters
 */
#define CF_FLASH_WEIGHT "flash_weight"
#define CF_BR_COLOR "br_color"
#define CF_CONTRAST_ENH "contrast_enhancement"
#define CF_BR_INTENSITY "br_intensity"
#define CF_SHARPEN "sharpen_beta"
#define CF_DEGHOST_EN "deghost_enable"

/**
 * macro to compare string and apply parameters
 */
#define CMP_AND_APPLY_RET(s_param, key, param, value, is_f) ({ \
  if (!strcmp(s_param, key)) { \
    param = is_f ? atof(value) : atoi(value); \
    return; \
  } \
})

#define TEST_ROLE_CDS_C 224
#define TEST_ROLE_CDS_NEON 225
#define TEST_ROLE_CDS_FCV 226

/**
 * Function: img_test_init
 *
 * Description: initializes the image test object
 *
 * Input parameters:
 *   p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_NO_MEMORY
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int img_test_init(imglib_test_t *p_test)
{
  char filename[MAX_FILENAME_LEN];
  char path[MAX_FILENAME_LEN];
  uint32_t i = 0;
  char *ext = "%d.yuv";
  int rc = 0;

  IDBG_INFO("%s:%d] cnt %d", __func__, __LINE__, p_test->in_count);
  for (i = 0; i < p_test->in_count; i++) {
    strlcpy(filename, p_test->input_fn, strlen(p_test->input_fn) + 1);
    strncat(filename, ext, strlen(ext) + 1);
    snprintf(path, sizeof(path), filename, i);
    IDBG_INFO("%s:%d] filename %s", __func__, __LINE__, path);

    rc = img_test_fill_buffer(p_test, i, 0);
    if (rc < 0)
      return rc;

    rc = img_test_read(p_test, path, i);
    if (rc < 0)
      return rc;

  }

  return IMG_SUCCESS;
}

/**
 * Function: img_test_get_buf_type
 *
 * Description: Choose buffer type based on sub role
 *
 * Input parameters:
 *   p_test - test object
 *
 * Return values:
 *   img_buf_type_t
 *
 * Notes: none
 **/
img_buf_type_t img_test_get_buf_type(imglib_test_t *p_test)
{
  switch (p_test->sub_role) {
  case IMG_TEST_SW:
    return IMG_BUFFER_HEAP;
  case IMG_TEST_HW:
    return IMG_BUFFER_ION_IOMMU;
  case IMG_TEST_ADSP:
    return IMG_BUFFER_ION_ADSP;
  default:
    return IMG_BUFFER_HEAP;
  }
}

/**
 * Function: img_test_fill_buffer
 *
 * Description: fill the buffer structures
 *
 * Input parameters:
 *   p_test - test object
 *   index - index of the buffer
 *   analysis - set if its analysis frame
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_NO_MEMORY
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int img_test_fill_buffer(imglib_test_t *p_test, uint32_t index, int analysis)
{
  int lrc = 0;
  uint32_t i = 0;
  img_frame_t *pframe = NULL;
  int size = 0;
  float h_f, w_f;
  float chroma_f;
  int offset = 0;
  uint32_t length;

  if (index >= MAX_TEST_FRAMES) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  lrc = img_get_subsampling_factor(p_test->ss, &w_f, &h_f);
  if (lrc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  chroma_f = h_f * w_f * 2;

  pframe = &p_test->frame[index];
  pframe->frame_cnt = 1;
  pframe->idx = index;
  pframe->info.analysis = analysis;
  pframe->info.width = p_test->width;
  pframe->info.height = p_test->height;
  pframe->frame[0].plane[0].plane_type = p_test->input_format;

  IDBG_INFO("%s:%d] dim %dx%d, type %d", __func__, __LINE__, p_test->width,
    p_test->height, p_test->input_format);

  if (QIMG_SINGLE_PLN_INTLVD(pframe)) {
    length = p_test->stride * p_test->scanline * 2;
  } else {
    length = p_test->stride * p_test->scanline;
  }

  size = length * (1 + chroma_f);
  pframe->frame[0].plane_cnt = 2;

  lrc = img_buffer_get(img_test_get_buf_type(p_test), -1, 1, size,
    &p_test->img_test_mem_handle[p_test->mem_cnt]);
  if (lrc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Img buffer get failed", __func__, __LINE__);
    fprintf(stderr, "\nError: No Memory\n\n");
    return IMG_ERR_NO_MEMORY;
  }

  for (i = 0; i < pframe->frame[0].plane_cnt; i++) {
    pframe->frame[0].plane[i].fd =
      p_test->img_test_mem_handle[p_test->mem_cnt].fd;
    pframe->frame[0].plane[i].addr =
      ((uint8_t*) p_test->img_test_mem_handle[p_test->mem_cnt].vaddr)
      + offset;
    pframe->frame[0].plane[i].offset = 0;
    if (i == 0) { /* Y plane */
      pframe->frame[0].plane[i].width = pframe->info.width;
      pframe->frame[0].plane[i].height = pframe->info.height;
      pframe->frame[0].plane[i].stride = p_test->stride;
      pframe->frame[0].plane[i].scanline = p_test->scanline;
      pframe->frame[0].plane[i].length = length;
    } else { /* Chroma plane */
      pframe->frame[0].plane[i].width = pframe->info.width * w_f;
      pframe->frame[0].plane[i].height = pframe->info.height * h_f;
      pframe->frame[0].plane[i].stride = p_test->stride * w_f * 2;
      pframe->frame[0].plane[i].scanline = p_test->scanline * h_f;
      pframe->frame[0].plane[i].length = length * w_f * h_f * 2;
    }
    offset = pframe->frame[0].plane[i].length;
  }
  p_test->mem_cnt++;
  IDBG_INFO("%s:%d] ", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: img_test_read
 *
 * Description: read the data from the file and fill the buffers
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_NO_MEMORY
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int img_test_read(imglib_test_t *p_test, char *filename, uint32_t index)
{
  int lrc = 0;
  uint32_t i = 0;
  img_sub_frame_t *pframe = NULL;
  FILE *fp;

  if (index >= MAX_TEST_FRAMES) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  pframe = &p_test->frame[index].frame[0];

  IDBG_INFO("%s:%d] filename %s", __func__, __LINE__, filename);
  fp = fopen(filename, "rb+");
  if (fp) {
    for (i = 0; i < pframe->plane_cnt; i++) {
      lrc = fread(pframe->plane[i].addr, 1, pframe->plane[i].length, fp);
      IDBG_INFO("%s:%d] bytes_read %d idx %d", __func__, __LINE__, lrc, i);
    }
    fclose(fp);
  } else {
    IDBG_ERROR("%s:%d] failed to open %s", __func__, __LINE__, filename);
    return IMG_ERR_NO_MEMORY;
  }
  IDBG_INFO("%s:%d] ", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: img_test_write
 *
 * Description: write the data to the file
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_NO_MEMORY
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int img_test_write(imglib_test_t *p_test, char *filename, uint32_t index)
{
  int lrc = 0;
  uint32_t i = 0;
  img_sub_frame_t *pframe = NULL;
  FILE *fp;

  if (index >= MAX_TEST_FRAMES) {
    IDBG_ERROR("%s:%d] ", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  pframe = &p_test->frame[index].frame[0];

  IDBG_INFO("%s:%d] frame %p name %s", __func__, __LINE__, pframe, filename);

  fp = fopen(filename, "wb+");
  if (fp) {
    for (i = 0; i < pframe->plane_cnt; i++) {
      lrc = fwrite(pframe->plane[i].addr, 1, pframe->plane[i].length, fp);
      IDBG_INFO("%s:%d] bytes_written %d idx %d %p", __func__, __LINE__,
        lrc, i, pframe->plane[i].addr);
    }
    fclose(fp);
  } else {
    IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }
  IDBG_INFO("%s:%d] X", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: frameproc_test_event_handler
 *
 * Description: event handler for frameproc test case
 *
 * Input parameters:
 *   p_appdata - frameproc test object
 *   p_event - pointer to the event
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int frameproc_test_event_handler(void* p_appdata, img_event_t *p_event)
{
  frameproc_test_t *p_test = (frameproc_test_t *)p_appdata;

  if ((NULL == p_event) || (NULL == p_appdata)) {
    IDBG_ERROR("%s:%d] invalid event", __func__, __LINE__);
    return 0;
  }
  IDBG_INFO("%s:%d] type %d", __func__, __LINE__, p_event->type);
  switch (p_event->type) {
  case QIMG_EVT_IMG_OUT_BUF_DONE:
    pthread_cond_signal(&p_test->base->cond);
    break;
  case QIMG_EVT_IMG_BUF_DONE:
    if (p_test->caps.inplace_algo) {
      pthread_cond_signal(&p_test->base->cond);
    }
    break;
  case QIMG_EVT_META_BUF_DONE: {
    img_meta_t *p_meta = p_event->d.p_meta;
    IDBG_MED("%s:%d] QIMG_EVT_META_BUF_DONE p_meta %p",
      __func__, __LINE__, p_meta);

    if (p_test->stats_output) {
      img_paaf_result_t *p_paaf_result = NULL;
      p_paaf_result = (img_paaf_result_t *)img_get_meta(p_meta,
        IMG_META_PAAF_RESULT);
      fprintf(stderr, "PAAF output: FV = %f", p_paaf_result->fV);
    }
    break;
  }
  default:
    ;
  }
  return IMG_SUCCESS;
}

/**
 * Function: frameproc_test_cf_parse
 *
 * Description: Callback function for the chromaflash parser
 *
 * Input parameters:
 *   p_userdata - test object
 *   key - attribute
 *   value - value of the attribute
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static void frameproc_test_cf_parse(void *p_userdata, char *key, char *value)
{
  img_chromaflash_ctrl_t *p_cf_ctrl = (void *)p_userdata;

  IDBG_INFO("%s:%d] attr %s val %s ", __func__, __LINE__, key, value);
  CMP_AND_APPLY_RET(CF_FLASH_WEIGHT, key, p_cf_ctrl->flash_weight, value, 1);
  CMP_AND_APPLY_RET(CF_BR_COLOR, key, p_cf_ctrl->br_color, value, 1);
  CMP_AND_APPLY_RET(CF_CONTRAST_ENH, key, p_cf_ctrl->contrast_enhancement,
    value, 1);
  CMP_AND_APPLY_RET(CF_BR_INTENSITY, key, p_cf_ctrl->br_intensity, value, 1);
  CMP_AND_APPLY_RET(CF_SHARPEN, key, p_cf_ctrl->sharpen_beta, value, 1);
  CMP_AND_APPLY_RET(CF_DEGHOST_EN, key, p_cf_ctrl->deghost_enable, value, 0);
}

/*
 * Function: img_test_cds
 *
 * Description: execute cds test case
 *
 * Input parameters:
 *   p_test - test object
 *   @role - type of test
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int img_test_cds(imglib_test_t *p_test, int32_t role)
{
  img_frame_t *p_frame = &p_test->frame[0];
  bool inplace = true; /* for debugging */
  uint32_t idx = inplace ? 0 : 1;
  img_frame_t *p_out_frame = &p_test->frame[idx];
  img_ops_core_type type;
  int32_t rc = IMG_SUCCESS;
  IDBG_INFO("%s:%d] E", __func__, __LINE__);
  switch (role) {
  case TEST_ROLE_CDS_NEON:
    type = IMG_OPS_NEON;
    break;
  case TEST_ROLE_CDS_FCV:
    type = IMG_OPS_FCV;
    break;
  default:
  case TEST_ROLE_CDS_C:
    type = IMG_OPS_C;
    break;
  }
  if (!inplace) {
    img_test_fill_buffer(p_test, 1, 0);
    memcpy(IMG_ADDR(p_out_frame), IMG_ADDR(p_frame),
      IMG_Y_LEN(p_frame));
  }
  rc = img_sw_cds(p_frame->frame[0].plane[1].addr,
    IMG_WIDTH(p_frame),
    IMG_HEIGHT(p_frame)>>1,
    p_frame->frame[0].plane[1].stride,
    p_out_frame->frame[0].plane[1].addr,
    p_out_frame->frame[0].plane[1].stride,
    type);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = img_test_write(p_test, p_test->out_fn, idx);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
    return rc;
  }
  IDBG_INFO("%s:%d] X", __func__, __LINE__);
  return rc;
}

/*
 * Function: img_test_paaf
 *
 * Description: Fill in PAAF test params
 *
 * Input parameters:
 *   @paaf_cfg - PAAF cfg
 *   @p_test - test object
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
void img_test_fill_paaf_testparams(img_paaf_cfg_t *paaf_cfg,
  frameproc_test_t *p_test)
{
  float threshold = 304.0;

  paaf_cfg->roi.size.width = p_test->base->width/6;
  paaf_cfg->roi.size.height = p_test->base->width/6;
  paaf_cfg->roi.pos.x = (double)(2.5/6.0) *
    (double)paaf_cfg->roi.size.width;
  paaf_cfg->roi.pos.y = (double)(2.5/6.0) *
    (double)paaf_cfg->roi.size.height;
  paaf_cfg->cropped_roi.size.width = paaf_cfg->roi.size.width;
  paaf_cfg->cropped_roi.size.height = paaf_cfg->roi.size.height;
  paaf_cfg->cropped_roi.pos.x = paaf_cfg->roi.pos.x;
  paaf_cfg->cropped_roi.pos.y = paaf_cfg->roi.pos.y;

  if (paaf_cfg->filter_type == PAAF_ON_IIR) {
    double lcoeffa[] = {1.0, -1.758880, 0.930481, 1.0, -1.817633, 0.940604};
    double lcoeffb[] = {0.034788, 0.000000, -0.034788, 0.059808,
      0.000000, -0.059808};
    uint32_t i;

    for (i = 0; i < 6; i++) {
      paaf_cfg->coeffa[i] = lcoeffa[i];
      paaf_cfg->coeffb[i] = lcoeffb[i];
    }
    paaf_cfg->coeff_len = 6;
  }

  if (paaf_cfg->filter_type == PAAF_ON_FIR) {
    int coeff_fir[] = {-4, -4, 1, 2, 3, 4, 3, 2, 1, -4, -4};
    uint32_t k;
    for (k = 0; k < IMGLIB_ARRAY_SIZE(coeff_fir); k++) {
      paaf_cfg->coeff_fir[k] = coeff_fir[k];
    }
    paaf_cfg->coeff_len = sizeof(coeff_fir)/sizeof(coeff_fir[0]);
  }

  paaf_cfg->pixel_skip_cnt =
    (int32_t)(MAX(ceil((float)paaf_cfg->cropped_roi.size.width / threshold),
    ceil((float)paaf_cfg->roi.size.height / threshold)));

  return;
}

/**
 * Function: frameproc_test_execute
 *
 * Description: execute frameproc test case
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
int frameproc_test_execute(frameproc_test_t *p_test)
{
  int rc = IMG_SUCCESS;
  uint32_t i = 0;
  p_test->base->p_comp = &p_test->base->comp;
  p_test->base->p_core_ops = &p_test->base->core_ops;
  char *libname;
  int out_idx = p_test->base->in_count;
  uint32_t out_len = 0;
  uint8_t *out_addr = NULL;
  img_meta_t meta;
  img_chromaflash_ctrl_t cf_ctrl;
  uint32_t downscale_factor;
  img_dim_t dim;
  img_preload_param_t alloc_params;
  img_caps_t *p_caps = &p_test->caps;
  img_paaf_cfg_t paaf_cfg;

  memset(&meta, 0x0, sizeof(meta));
  p_caps->num_output = 1;
  switch (p_test->base->algo_index) {
  case 4: /* SW2D frame operations */
    p_caps->num_input = 1;
    p_caps->num_output = 1;
    p_caps->num_meta = 1;
    p_caps->use_internal_bufs = 1;
    libname = "libmmcamera_sw2d_lib.so";

    IDBG_HIGH("%s:%d] sw2d frame operation mask %d", __func__, __LINE__,
      p_test->base->sw2d_params.mask);
    rc = img_set_meta(&meta, IMG_META_SW2D_OPS,
      &p_test->base->sw2d_params.mask);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
      return rc;
    }
    break;
  case 3: /* chroma flash */
    p_caps->num_input = 3;
    p_caps->num_meta = 1;
    libname = "libmmcamera_chromaflash_lib.so";

    /* set default values */
    cf_ctrl.flash_weight = 15.0;
    cf_ctrl.deghost_enable = 1;
    cf_ctrl.br_color = .2;
    cf_ctrl.contrast_enhancement = 0.5;
    cf_ctrl.br_intensity = 1.0;
    cf_ctrl.sharpen_beta = 0.5;
    img_parse_main(p_test->base->tune_fn, &cf_ctrl, frameproc_test_cf_parse);

    rc = img_set_meta(&meta, IMG_META_CHROMAFLASH_CTRL, &cf_ctrl);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
      return rc;
    }
    break;
  case 2: /* opti zoom */
    p_caps->num_input = 8;
    p_caps->num_meta = 1;
    libname = "libmmcamera_optizoom_lib.so";
    break;
  case 1: /* ubi focus */
    p_caps->num_input = 5;
    p_caps->num_meta = 1;
    libname = "libmmcamera_ubifocus_lib.so";
    break;
  case 6: { /* Edge Alignment */
    p_caps->num_input = 1;
    p_caps->num_meta = 1;
    p_caps->inplace_algo = 1;
    dim.stride = 5000;
    dim.scanline = 4000;
    dim.width = 5000;
    dim.height = 4000;
    alloc_params.max_dim = dim;
    out_idx = 0;
    p_caps->num_output = 0;
    libname = "libmmcamera_edgesmooth_lib.so";
    break;
  }
  case 7:
    p_caps->num_input = 1;
    p_caps->num_meta = 1;
    p_caps->inplace_algo = 0;
    dim.stride = 5000;
    dim.scanline = 4000;
    dim.width = 5000;
    dim.height = 4000;
    alloc_params.max_dim = dim;
    p_caps->num_output = 0;
    libname = "libmmcamera_paaf_lib.so";
    p_test->stats_output = 1;
    paaf_cfg.enable = TRUE;
    paaf_cfg.filter_type = PAAF_ON_IIR;
    img_test_fill_paaf_testparams(&paaf_cfg, p_test);

    rc = img_set_meta(&meta, IMG_META_PAAF_CFG, &paaf_cfg);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
      return rc;
    }
    break;
  default:
  case 0: /* dummy */
    p_caps->num_input = 2;
    libname = "libmmcamera_dummyalgo.so";
    break;
  }

  rc = img_core_get_comp(IMG_COMP_GEN_FRAME_PROC, "qcom.gen_frameproc",
    p_test->base->p_core_ops);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_LOAD(p_test->base->p_core_ops, libname);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_CORE_PRELOAD(p_test->base->p_core_ops, &alloc_params);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
  }

  rc = IMG_COMP_CREATE(p_test->base->p_core_ops, p_test->base->p_comp);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_INIT(p_test->base->p_comp, (void *)p_test, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_CB(p_test->base->p_comp, frameproc_test_event_handler);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_test->base->p_comp, QIMG_PARAM_CAPS,
      (void *)p_caps);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  img_frame_ops_t l_frameops = {
    .dump_frame = img_dump_frame,
    .get_meta = img_get_meta,
    .set_meta = img_set_meta,
    .image_copy = img_image_copy,
    .image_scale = img_sw_downscale_2by2,
  };
  l_frameops.p_appdata = p_test;
  rc = IMG_COMP_SET_PARAM(p_test->base->p_comp, QIMG_PARAM_FRAME_OPS, &l_frameops);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_START(p_test->base->p_comp, p_caps);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  /* Queue meta to component */
  rc = IMG_COMP_Q_META_BUF(p_test->base->p_comp, &meta);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  /* Queue input buffer to component */
  for (i = 0; i < p_test->base->in_count; i++) {
    IDBG_INFO("%s:%d] dim %dx%d frame %p", __func__, __LINE__,
      p_test->base->frame[i].frame[0].plane[0].stride,
      p_test->base->frame[i].frame[0].plane[0].scanline,
      &p_test->base->frame[i]);

    rc = IMG_COMP_Q_BUF(p_test->base->p_comp, &p_test->base->frame[i],
        IMG_IN);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      return rc;
    }
  }

  /* Set output frame details */
  p_test->base->frame[out_idx] = p_test->base->frame[0];

  if (p_test->base->sw2d_params.mask & SW2D_OPS_DOWNSCALE) {
    downscale_factor = p_test->base->sw2d_params.downscale_factor;
    if (downscale_factor <= 0) {
      IDBG_ERROR("%s:%d] Error, invalid downscale factor. Setting to 1",
        __func__, __LINE__);
      downscale_factor = 1;
    }
    IDBG_HIGH("%s:%d] Scalar enabled, downscale factor %d",
      __func__, __LINE__, downscale_factor);
    p_test->base->frame[out_idx].frame[0].plane[0].width =
      p_test->base->frame[0].frame[0].plane[0].width / downscale_factor;
    p_test->base->frame[out_idx].frame[0].plane[0].height =
      p_test->base->frame[0].frame[0].plane[0].height / downscale_factor;
    p_test->base->frame[out_idx].frame[0].plane[0].stride =
      p_test->base->frame[0].frame[0].plane[0].stride / downscale_factor;
    p_test->base->frame[out_idx].frame[0].plane[0].scanline =
      p_test->base->frame[0].frame[0].plane[0].scanline / downscale_factor;
  }

  IDBG_HIGH("%s:%d] out dimension %d %d %d %d", __func__, __LINE__,
    p_test->base->frame[out_idx].frame[0].plane[0].width,
    p_test->base->frame[out_idx].frame[0].plane[0].height,
    p_test->base->frame[out_idx].frame[0].plane[0].stride,
    p_test->base->frame[out_idx].frame[0].plane[0].scanline);

  out_len = p_test->base->frame[out_idx].frame[0].plane[0].stride *
    p_test->base->frame[out_idx].frame[0].plane[0].scanline;
  p_test->base->frame[out_idx].frame[0].plane[0].length = out_len;
  p_test->base->frame[out_idx].frame[0].plane[1].length = out_len / 2;

  if (QIMG_SINGLE_PLN_INTLVD(&p_test->base->frame[0])) {
    out_addr = malloc(out_len * 2);
  } else {
    out_addr = malloc(out_len * 3/2);
  }
  if (!out_addr) {
    IDBG_ERROR("%s:%d] cannot alloc", __func__, __LINE__);
    return rc;
  }
  p_test->base->frame[out_idx].frame[0].plane[0].addr = out_addr;
  p_test->base->frame[out_idx].frame[0].plane[1].addr = out_addr + out_len;

  IDBG_INFO("%s:%d] out_idx %d, out_addr %p", __func__, __LINE__, out_idx,
    out_addr);

  /* Queue output buffer to component */
  if (!p_caps->inplace_algo) {
    rc = IMG_COMP_Q_BUF(p_test->base->p_comp, &p_test->base->frame[out_idx],
      IMG_OUT);
  }

  IDBG_INFO("%s:%d] before wait rc %d", __func__, __LINE__, rc);

  /* Wait for completion */
  pthread_mutex_lock(&p_test->base->mutex);
  rc = img_wait_for_completion(&p_test->base->cond, &p_test->base->mutex,
    10000);

  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_test->base->mutex);
    return rc;
  }
  IDBG_INFO("%s:%d] after wait rc %d", __func__, __LINE__, rc);
  pthread_mutex_unlock(&p_test->base->mutex);

  rc = img_test_write(p_test->base, p_test->base->out_fn, out_idx);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] Error, write failed %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_ABORT(p_test->base->p_comp, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_DEINIT(p_test->base->p_comp);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] Deinit failed %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_CORE_SHUTDOWN(p_test->base->p_core_ops);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("Shutdown failed %d", rc);
  }

  if (out_addr) {
    free(out_addr);
    out_addr = NULL;
  }
  IDBG_INFO("%s:%d] rc %d", __func__, __LINE__, rc);
  return 0;
}

/**
 * Function: role_enum_to_str
 *
 * Description: translate role enum to string
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   role string
 *
 * Notes: none
 **/
char* role_enum_to_str(img_comp_role_t role)
{
  switch (role) {
  case IMG_COMP_DENOISE:
    return "DENOISE";
  case IMG_COMP_HDR:
    return "HDR";
  case IMG_COMP_FACE_PROC:
    return "FACE_PROC";
  case IMG_COMP_CAC:
    return "CAC";
  case IMG_COMP_GEN_FRAME_PROC:
    return "FRAME_PROC";
  case IMG_COMP_DUAL_FRAME_PROC:
    return "DUAL_FRAME_PROC";
  default:
    return NULL;
  }
}

/**
 * Function: sub_role_enum_to_str
 *
 * Description: translate role enum to string
 *
 * Input parameters:
 *   img_test_sub_role_t
 *
 * Return values:
 *   sub role string
 *
 * Notes: none
 **/
char* sub_role_enum_to_str(img_test_sub_role_t sub_role)
{
  switch (sub_role) {
  case IMG_TEST_SW:
    return "Use software component";
  case IMG_TEST_HW:
    return "Use hardware component";
  case IMG_TEST_ADSP:
    return "Use ADSP component";
  default:
    return NULL;
  }
}

/**
 * Function: wdmode_enum_to_str
 *
 * Description: translate role enum to string
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   role string
 *
 * Notes: none
 **/
char* wdmode_enum_to_str(wd_mode_t mode)
{
  switch (mode) {
  case WD_MODE_CBCR_ONLY:
    return "WD_MODE_CBCR_ONLY";
  case WD_MODE_STREAMLINE_YCBCR:
    return "WD_MODE_STREAMLINE_YCBCR";
  case WD_MODE_STREAMLINED_CBCR:
    return "WD_MODE_STREAMLINED_CBCR";
  case WD_MODE_YCBCR_PLANE:
    return "WD_MODE_YCBCR_PLANE";
  default:
    return NULL;
  }
}

/**
 * Function: print_usage
 *
 * Description: print the usage of the test application
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void print_usage()
{
  fprintf(stderr, "Usage: program_name [options] [-I <input file>]"
    " [-O <output file] [-W <width>] [-H <height>] [-R <role>]\n");
  fprintf(stderr, "Mandatory options:\n");
  fprintf(stderr, "  -I FILE\t\tPath to the input file.\n");
  fprintf(stderr, "  -i FILE\t\tPath to the input file2.\n");
  fprintf(stderr, "  -O FILE\t\tPath for the output file.\n");
  fprintf(stderr, "  -W WIDTH\t\tInput image width.\n");
  fprintf(stderr, "  -H HEIGHT\t\tInput image height.\n");
  fprintf(stderr, "  -w WIDTH\t\tInput image2 width.\n");
  fprintf(stderr, "  -h HEIGHT\t\tInput image2 height.\n");
  fprintf(stderr, "  -s STRIDE\t\tInput image stride.\n");
  fprintf(stderr, "  -S SCANLINE\t\tInput image scanline.\n");
  fprintf(stderr, "  -d STRIDE\t\tInput image2 stride.\n");
  fprintf(stderr, "  -D SCANLINE\t\tInput image2 scanline.\n");
  fprintf(stderr, "  -R ROLE\t\tImaging LIB role. 0.Denoise 1.HDR "
    "2.Face detect 3.CAC 4.FRAMEPROC 5.DUAL FRAMEPROC\n");
  fprintf(stderr, "  -r SUB ROLE\t\tImaging LIB sub role. 0.Software "
    "1.Hardware 3.ADSP\n");
  fprintf(stderr, "  -A ALGORITHM\t\tImaging algorithm 0.Dummy 1.Ubifocus "
    "2.Optizoom 3.Chromaflash 4.FrameOps 5.DCRF 6.EA 7.PAAF\n");
  fprintf(stderr, "  -N FILE\t\tNumber of input images.\n");
  fprintf(stderr, "  -T TUNE_TXT\t\tPath to the tuning file\n");
  fprintf(stderr, "  -M FILE\t\tWavelet denoise mode 0.YCbCr normal "
    "1.CbCr normal 2.YCbCr streamlined 3.CbCr streamlined\n");
  fprintf(stderr, "  -U IMAGING UTILITY\tImaging utility 1.BUF_POOL_TEST\n");
  fprintf(stderr, "  -C SW2D MASK\t\tSW2D algo: 1<<0 Downscale, "
    "1<<1 Deinterleave\n");
  fprintf(stderr, "  -x SW2D DOWNSCALE\tDownscalar factor \n");
  fprintf(stderr, "  -F IMAGE FORMAT \tInput Image format: 0.PLANE_Y, "
    "1.PLANE_CB_CR, 2.PLANE_CR_CB, 3.PLANE_CB, \n\t\t\t4.PLANE_CR, "
    "5.PLANE_Y_CB_Y_CR, 6.PLANE_Y_CR_Y_CB, "
    "7.PLANE_CB_Y_CR_Y, 8.PLANE_CR_Y_CB_Y\n");
  fprintf(stderr, "\nExample:\nadb shell /system/bin/mm-imglib-test -I "
    QCAMERA_DUMP_FRM_LOCATION"input -O /sytem/bin -W 3120 -H 4160 -s 3136 -S "
    "4160 -R 4 -A 3 -N 2 -T "QCAMERA_DUMP_FRM_LOCATION"tune.txt\n");
  fprintf(stderr, "\n");
}


/**
 * Function: main
 *
 * Description: main encoder test app routine
 *
 * Input parameters:
 *   argc - argument count
 *   argv - argument strings
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
int main(int argc, char* argv[])
{
  int rc, c, i, val = 0;
  imglib_test_t img_test, img_test1;
  hdr_test_t hdr_test;
  denoise_test_t denoise_test;
  faceproc_test_t faceproc_test;
  cac_test_t cac_test;
  frameproc_test_t frameproc_test;
  dual_frameproc_test_t dual_frameproc_test;
  uint32_t img_util = 0;

  /* Initialize the structures */
  memset(&img_test, 0x0, sizeof(img_test));
  memset(&img_test1, 0x0, sizeof(img_test));
  memset(&hdr_test, 0x0, sizeof(hdr_test));
  memset(&denoise_test, 0x0, sizeof(denoise_test));
  memset(&faceproc_test, 0x0, sizeof(faceproc_test));
  memset(&cac_test, 0x0, sizeof(cac_test));
  memset(&frameproc_test, 0x0, sizeof(frameproc_test));
  pthread_mutex_init(&img_test.mutex, NULL);
  pthread_cond_init(&img_test.cond, NULL);
  pthread_mutex_init(&img_test1.mutex, NULL);
  pthread_cond_init(&img_test1.cond, NULL);

  IMG_INIT_LOGGING();


  fprintf(stderr, "=======================================================\n");
  fprintf(stderr, " Qualcomm ImagingLIB test\n");
  fprintf(stderr, "=======================================================\n");
  opterr = 1;

  while ((c = getopt(argc, argv,
    "I:i:O:W:w:H:h:N:R:r:M:T:s:S:D:d:A:U:C:F:x:PZ")) != -1) {
    switch (c) {
    case 'O':
      img_test.out_fn = optarg;
      fprintf(stderr, "%-25s%s\n", "Output image path", img_test.out_fn);
      break;
    case 'I':
      img_test.input_fn = optarg;
      fprintf(stderr, "%-25s%s\n", "Input image path", img_test.input_fn);
      break;
    case 'W':
      img_test.width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Input image width", img_test.width);
      break;
    case 'H':
      img_test.height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Input image height", img_test.height);
      break;
    case 'i':
      img_test1.input_fn = optarg;
      fprintf(stderr, "%-25s%s\n", "Input image2 path", img_test1.input_fn);
      break;
    case 'w':
      img_test1.width = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Input image2 width", img_test1.width);
      break;
    case 'h':
      img_test1.height = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Input image2 height", img_test1.height);
      break;
    case 'N':
      img_test.in_count = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Input image count", img_test.in_count);
      break;
    case 'T':
      img_test.tune_fn = optarg;
      fprintf(stderr, "%-25s%s\n", "Tune path", img_test.tune_fn);
      break;
    case 'A':
      img_test.algo_index = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Algorithm", img_test.algo_index);
      break;
    case 'M':
      denoise_test.mode = atoi(optarg);
      if (wdmode_enum_to_str(denoise_test.mode) == NULL) {
        print_usage();
        val = -1;
        goto exit;
      }
      fprintf(stderr, "%-25s%s\n", "Wavelet mode role",
        wdmode_enum_to_str(denoise_test.mode));
      break;
    case 'R': {
      img_test.main_role = atoi(optarg);
      char *p_str_role = role_enum_to_str(img_test.main_role);
      if ((p_str_role == NULL) && (img_test.main_role >= IMG_COMP_ROLE_MAX)) {
        print_usage();
        val = -1;
        goto exit;
      }
      fprintf(stderr, "%-25s%s\n", "Imaging LIB role", p_str_role ?
        p_str_role : "custom role");
      break;
    }
    case 'r':
      img_test.sub_role = atoi(optarg);
      if (sub_role_enum_to_str(img_test.sub_role) == NULL) {
        fprintf(stderr, "%-25s%s\n", "Imaging LIB sub role",
          sub_role_enum_to_str(img_test.sub_role));
      }
      break;
    case 's':
      img_test.stride = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Input image stride", img_test.stride);
      break;
    case 'S':
      img_test.scanline = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Input image scanline", img_test.scanline);
      break;
    case 'd':
      img_test1.stride = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Input image2 stride", img_test1.stride);
      break;
    case 'D':
      img_test1.scanline = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Input image2 scanline", img_test1.scanline);
    case 'U':
      img_util = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Image utility", img_util);
      break;
    case 'F':
      img_test.input_format = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Input Image Format", img_test.input_format);
      break;
    case 'C':
      img_test.sw2d_params.mask = atoi(optarg);
      if (img_test.sw2d_params.mask & SW2D_OPS_DOWNSCALE) {
        fprintf(stderr, "%-25s\n", "SW2D Frame Ops\t\t DOWNSCALAR");
        break;
      }
      if (img_test.sw2d_params.mask & SW2D_OPS_DEINTERLEAVE) {
        fprintf(stderr, "%-25s\n", "SW2D Frame Ops\t\t DEINTERLEAVE");
        break;
      }
      break;
    case 'x':
      img_test.sw2d_params.downscale_factor = atoi(optarg);
      fprintf(stderr, "%-25s%d\n", "Downscale factor",
        img_test.sw2d_params.downscale_factor);
      break;
    default:
      print_usage();
      val = 0;
      goto exit;
    }
  }

  if ((img_test.width == 0) || (img_test.height == 0)
    || (img_test.out_fn == NULL) || (img_test.input_fn == NULL)
    || (img_test.in_count <= 0)) {
    fprintf(stderr, "%-25s\n", "Error in input");
    print_usage();
    val = 0;
    goto exit;
  }

  if (img_test.stride < img_test.width)
    img_test.stride = img_test.width;
  if (img_test.scanline < img_test.height)
    img_test.scanline = img_test.height;

  if (img_test1.stride < img_test1.width)
    img_test1.stride = img_test1.width;
  if (img_test1.scanline < img_test1.height)
    img_test1.scanline = img_test1.height;
  img_test1.in_count = img_test.in_count;

  rc = img_test_init(&img_test);
  if (rc != IMG_SUCCESS) {
    fprintf(stderr, "%-25s\n", "Error in test init");
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    val = rc;
    goto error;
  }

  /* Execute non-component testcases */
  switch (img_util) {
  case UTIL_BUF_POOL_TEST:
    rc = bufpool_test_execute(&img_test);
    goto exit;
  case 0:
    break; /* continue with comp test */
  default:
    fprintf(stderr, "Error invalid mode\n");
    goto error;
  }

  switch (img_test.main_role) {
  case IMG_COMP_HDR:
    hdr_test.base = &img_test;
    rc = hdr_test_execute(&hdr_test);
    break;
  case IMG_COMP_DENOISE:
    denoise_test.base = &img_test;
    rc = denoise_test_execute(&denoise_test);
    break;
  case IMG_COMP_FACE_PROC:
    faceproc_test.base = &img_test;
    rc = faceproc_test_execute(&faceproc_test);
    break;
  case IMG_COMP_CAC:
    cac_test.base = &img_test;
    rc = cac_test_execute(&cac_test);
    break;
  case IMG_COMP_GEN_FRAME_PROC:
    frameproc_test.base = &img_test;
    rc = frameproc_test_execute(&frameproc_test);
    break;
  case IMG_COMP_DUAL_FRAME_PROC:
    rc = img_test_init(&img_test1);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      val = rc;
      goto error;
    }
    dual_frameproc_test.main_base = &img_test;
    dual_frameproc_test.aux_base = &img_test1;
    rc = dual_frameproc_test_execute(&dual_frameproc_test);
    break;
  default: {
    fprintf(stderr, "%-25s\n", "Internal role");
    int32_t roleint = (int32_t)img_test.main_role;
    switch (roleint) {
    case TEST_ROLE_CDS_NEON:
    case TEST_ROLE_CDS_FCV:
    case TEST_ROLE_CDS_C:
      rc = img_test_cds(&img_test, roleint);
      break;
    default:
      fprintf(stderr, "Error invalid mode\n");
      goto error;
    }
  }
  }

  if (rc != IMG_SUCCESS) {
    fprintf(stderr, "Error rc %d", rc);
    goto error;
  }

  fprintf(stderr, "\nSUCCESSUL\n\n");

error:
  for (i = 0; i < img_test.mem_cnt; i++) {
    if (img_test.img_test_mem_handle[i].handle) {
      if (IMG_SUCCESS !=
        img_buffer_release(&img_test.img_test_mem_handle[i])) {
        IDBG_ERROR("%s:%d:] Img buffer release failed",
        __func__, __LINE__);
        return IMG_ERR_GENERAL;
      }
    }
  }
  for (i = 0; i < img_test1.mem_cnt; i++) {
    if (img_test1.img_test_mem_handle[i].handle) {
      if (IMG_SUCCESS !=
        img_buffer_release(&img_test1.img_test_mem_handle[i])) {
        IDBG_ERROR("%s:%d:] Img buffer release failed",
        __func__, __LINE__);
        return IMG_ERR_GENERAL;
      }
    }
  }

exit:
  pthread_mutex_destroy(&img_test.mutex);
  pthread_cond_destroy(&img_test.cond);
  pthread_mutex_destroy(&img_test1.mutex);
  pthread_cond_destroy(&img_test1.cond);

  return val;
}
