/**********************************************************************
*  Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __IMG_COMP_REGISTRY_H__
#define __IMG_COMP_REGISTRY_H__

#include "img_comp_factory_priv.h"

/** g_img_registry
 *
 *   Global Registry for the components available in the target
 **/
const img_comp_reg_t g_img_registry[] =
{
  {
    IMG_COMP_DENOISE,
    "qcom.wavelet",
    {
      .create = wd_comp_create,
      .load = wd_comp_load,
      .unload = wd_comp_unload,
    },
  },
  {
    IMG_COMP_HDR,
    "qcom.hdr",
    {
      .create = hdr_comp_create,
      .load = hdr_comp_load,
      .unload = hdr_comp_unload,
    },
  },
  {
    IMG_COMP_FACE_PROC,
    "qcom.faceproc",
    {
      .create = faceproc_comp_create,
      .load = faceproc_comp_load,
      .unload = faceproc_comp_unload,
    },
  },
  {
    IMG_COMP_FACE_PROC,
    "qcom.faceproc_dsp",
    {
      .create = faceproc_dsp_comp_create,
      .load = faceproc_dsp_comp_load,
      .unload = faceproc_dsp_comp_unload,
    },
  },
  {
    IMG_COMP_FACE_PROC,
    "qcom.faceproc_hw",
    {
      .create = faceproc_hw_comp_create,
      .load = faceproc_hw_comp_load,
      .unload = faceproc_hw_comp_unload,
    },
  },
  {
    IMG_COMP_CAC,
    "qcom.cac1",
    {
      .create = cac_comp_create,
      .load = cac_comp_load,
      .unload = cac_comp_unload,
    },
  },
  {
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    {
      .create = frameproc_comp_create,
      .load = frameproc_comp_load,
      .unload = frameproc_comp_unload,
      .alloc = frameproc_comp_alloc,
      .dealloc = frameproc_comp_dealloc,
      .preload_needed = frameproc_comp_preload_needed,
    },
  },
  {
    IMG_COMP_DUAL_FRAME_PROC,
    "qti.dual_frameproc",
    {
      .create = dual_frameproc_comp_create,
      .load = dual_frameproc_comp_load,
      .unload = dual_frameproc_comp_unload,
      .bind = dual_frameproc_comp_bind,
      .unbind = dual_frameproc_comp_unbind,
    },
  },
  {
    IMG_COMP_CAC,
    "qcom.cac2",
    {
      .create = cac2_comp_create,
      .load = cac2_comp_load,
      .unload = cac2_comp_unload,
      .alloc = cac2_comp_alloc,
      .dealloc = cac2_comp_dealloc,
    },
  },
  {
    IMG_COMP_CAC,
    "qcom.cac3",
    {
      .create = cac3_comp_create,
      .load = cac3_comp_load,
      .unload = cac3_comp_unload,
      .alloc = cac3_comp_alloc,
      .dealloc = cac3_comp_dealloc,
    },
  },
  {
    IMG_COMP_LIB2D,
    "qti.lib2d",
    {
      .create = lib2d_comp_create,
      .load = lib2d_comp_load,
      .unload = lib2d_comp_unload,
    },
  },
#ifdef JPEG_DMA_AVIL
  {
    IMG_COMP_GEN_2D,
    "qti.jpegdma",
    {
      .create = jpegdma_comp_create,
      .load = jpegdma_comp_load,
      .unload = jpegdma_comp_unload,
    },
  },
#endif
  {
    IMG_COMP_MULTI_FRAME_PROC,
    "qti.multi_frameproc",
    {
      .create = multi_frameproc_comp_create,
      .load = multi_frameproc_core_load,
      .unload = multi_frameproc_core_unload,
      .bind = multi_frameproc_mgr_bind,
      .unbind = multi_frameproc_mgr_unbind,
    },
  }
};

#define IMG_COMP_REG_SIZE sizeof(g_img_registry)/sizeof(g_img_registry[0])

#endif //__IMG_COMP_REGISTRY_H__
