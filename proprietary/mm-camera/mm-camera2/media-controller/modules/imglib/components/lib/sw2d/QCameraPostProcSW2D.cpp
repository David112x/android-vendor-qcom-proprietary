/**********************************************************************
*  Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#define LOG_TAG "QCameraSW2D"

#include <sys/types.h>
#include <stdbool.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <utils/Log.h>
#ifdef USE_FASTCV_OPT
#include <fastcv/fastcv.h>
#endif
#include "QCameraPostProcSW2D.h"

/**
 * Function: img_algo_init
 *
 * Description: algorithm initialization
 *
 * Arguments:
 *   @pp_context: return context [Output parameter]
 *   @p_params: init params
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
extern "C" int img_algo_init(void **p_context, img_init_params_t *p_params)
{
  IDBG_MED("%s:%d] E", __func__, __LINE__);
  IMG_UNUSED(p_params);
  int ret = IMG_SUCCESS;

  QCameraPostProcSW2D *sw2d = new QCameraPostProcSW2D;
  if (sw2d) {
    *p_context = sw2d;
  } else {
    ret  = IMG_ERR_GENERAL;
  }

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return ret;
}

/**
 * Function: img_algo_deinit
 *
 * Description: algorithm de-initialization
 *
 * Arguments:
 *   @p_context: mod prt
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
extern "C" int img_algo_deinit(void *p_context)
{
  IDBG_MED("%s:%d] E", __func__, __LINE__);
  if (!p_context) {
    IDBG_ERROR("%s:%d] Error, invalid input", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  QCameraPostProcSW2D* sw2d =
    (QCameraPostProcSW2D*) p_context;
  delete sw2d;

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return IMG_SUCCESS;
}

/**
 * Function: img_algo_process
 *
 * Description: algorithm main body
 *
 * Arguments:
 *   @arg1: p_context: class pointer
 *   @arg2: p_in_frame: input frame handler
 *   @arg3: in_frame_cnt: number of input frames
 *   @arg4: p_out_frame: output frame handler
 *   @arg5: out_frame_cnt: number of output frames
 *   @arg6: p_meta: frame meta data
 *   @arg7: meta_cnt: number of meta data array
 *
 * Return values:
 *   IMG_SUCCESS or IMG_ERR_GENERAL
 *
 * Notes: none
 **/
extern "C" int img_algo_process(void *p_context, img_frame_t *p_in_frame[],
  int in_frame_cnt,img_frame_t *p_out_frame[],
  int out_frame_cnt, img_meta_t *p_meta[], int meta_cnt)
{
  IDBG_MED("%s:%d] E", __func__, __LINE__);
  IMG_UNUSED(in_frame_cnt);
  IMG_UNUSED(p_in_frame);
  IMG_UNUSED(out_frame_cnt);
  int ret = IMG_SUCCESS;

  if (p_context != NULL) {
    QCameraPostProcSW2D* sw2d = (QCameraPostProcSW2D*) p_context;
    ret = sw2d->processData(p_in_frame, p_out_frame[0],
      ((p_meta && (meta_cnt > 0)) ? p_meta[0] : NULL));
  } else {
    ret = IMG_ERR_GENERAL;
  }

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return ret;
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
extern "C" int img_algo_set_frame_ops(void *p_context, img_frame_ops_t *p_ops)
{
  IDBG_MED("%s:%d] E", __func__, __LINE__);
  int ret = IMG_SUCCESS;

  if (!p_context || !p_ops) {
    IDBG_ERROR("%s:%d] Error, invalid input", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  QCameraPostProcSW2D* sw2d = (QCameraPostProcSW2D*) p_context;
  sw2d->setFrameOps(p_ops);
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return ret;
}

/**
 * Function: QCameraPostProcSW2D
 *
 * Description: constructor of class
 *
 * Arguments:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
QCameraPostProcSW2D::QCameraPostProcSW2D()
{
  IDBG_MED("%s:%d] E", __func__, __LINE__);

  mBufAllocDone = FALSE;
  memset(&mFrameOps, 0x0, sizeof(mFrameOps));

  IDBG_MED("%s:%d] X", __func__, __LINE__);
}

/**
 * Function: ~QCameraPostProcSW2D
 *
 * Description: destructor of class
 *
 * Arguments:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
QCameraPostProcSW2D::~QCameraPostProcSW2D()
{
  IDBG_MED("%s:%d] E", __func__, __LINE__);
  if (mBufAllocDone && mBufMemHandle.handle) {
    IDBG_MED("%s:%d] Release temp buf", __func__, __LINE__);
    img_buffer_release(&mBufMemHandle);
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);
}

/**
 * Function: deinterleaveFrame
 *
 * Description: Deinterleaves input frame to semi-planar if the
 *   input is a single plane YUV format
 *
 * Arguments:
 *   @inFrame: input frames
 *   @pOutFrame: output frame. if NULL, allocate internal buffer
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
int8_t QCameraPostProcSW2D::deinterleaveFrame(img_frame_t *inFrame,
  uint8_t *pOutFrame)
{
  int8_t ret = IMG_SUCCESS;
  int32_t outLength;
  img_frame_t tempFrame;

  if (!pOutFrame) {
    if (!mBufAllocDone) {
      IDBG_HIGH("%s:%d] allocate deinterleave buf %d x %d", __func__,
        __LINE__, QIMG_STRIDE(inFrame, 0), QIMG_SCANLINE(inFrame, 0));

      /* input is single plane YUV */
      outLength =  QIMG_STRIDE(inFrame, 0) * QIMG_SCANLINE(inFrame, 0) * 2;
      if (outLength <= 0) {
        IDBG_ERROR("%s:%d] Error, invalid buf length", __func__, __LINE__);
        goto error;
      }

      ret = img_buffer_get(IMG_BUFFER_ION_IOMMU, -1, TRUE,
        outLength, &mBufMemHandle);
      if (ret != IMG_SUCCESS) {
        IDBG_ERROR("%s:%d] Error, img buf get failed", __func__, __LINE__);
        goto error;
      }

      mBufAllocDone = TRUE;
      IDBG_HIGH("%s:%d] Allocate deinterleave buf successful, %p",
        __func__, __LINE__, mBufMemHandle.vaddr);
    } else {
      ret = img_buffer_cacheops(&mBufMemHandle, IMG_CACHE_CLEAN_INV,
        IMG_INTERNAL);
      if (IMG_ERROR(ret)) {
        IDBG_ERROR("%s:%d] Error, cache invalidate fail", __func__, __LINE__);
      }
    }
    pOutFrame = (uint8_t *)mBufMemHandle.vaddr;
  }

  if (!pOutFrame) {
    IDBG_ERROR("%s:%d] Error, op buffer invalid", __func__, __LINE__);
    goto error;
  }

  memset(&tempFrame, 0x00, sizeof(img_frame_t));

  QIMG_ADDR(&tempFrame, 0) = pOutFrame;
  QIMG_WIDTH(&tempFrame, 0) = QIMG_WIDTH(inFrame, 0);
  QIMG_HEIGHT(&tempFrame, 0) = QIMG_HEIGHT(inFrame, 0);
  QIMG_STRIDE(&tempFrame, 0) = QIMG_STRIDE(inFrame, 0);
  QIMG_SCANLINE(&tempFrame, 0) = QIMG_SCANLINE(inFrame, 0);
  QIMG_LEN(&tempFrame, 0) =
    (QIMG_STRIDE(inFrame, 0) * QIMG_SCANLINE(inFrame, 0));
  QIMG_ADDR(&tempFrame, 1) =
    QIMG_ADDR(&tempFrame, 0) + QIMG_LEN(&tempFrame, 0);

  ret = img_plane_deinterleave(QIMG_ADDR(inFrame, 0),
    QIMG_PL_TYPE(inFrame, 0), &tempFrame);
  if (IMG_ERROR(ret)) {
    IDBG_ERROR("%s:%d] Error deinterleave failed %d", __func__, __LINE__, ret);
    goto error;
  }

  IDBG_HIGH("%s:%d] Deinterleave successful %p", __func__, __LINE__, pOutFrame);
  return IMG_SUCCESS;

error:
   return ret;
}


/**
 * Function: processData
 *
 * Description: main processing body
 *
 * Arguments:
 *   @arg1: inFrame: input frames
 *   @arg2: outFrame: output frame
 *   @arg3: p_meta: frame meta data
 *
 * Return values:
 *   imaging error values
 *
 * Notes: none
 **/
int8_t QCameraPostProcSW2D::processData(img_frame_t *inFrame[],
  img_frame_t *outFrame, img_meta_t *p_meta)
{
  int8_t ret = IMG_SUCCESS;
  uint8_t *pSrcBuf = NULL;
  uint32_t *p_sw2d_mask;

  /* retreive sw2d frame operation to apply */
  p_sw2d_mask =
    (uint32_t *)mFrameOps.get_meta(p_meta, IMG_META_SW2D_OPS);
  if (!p_sw2d_mask) {
    IDBG_ERROR("%s:%d] Error, invalid input", __func__, __LINE__);
    goto error;
  }

  IDBG_HIGH("%s:%d] I[%p, %d %d, %d %d, %d, type %d] "
    "O[%p, %d %d, %d %d, %d, type %d]", __func__, __LINE__,
    QIMG_ADDR(inFrame[0], 0), QIMG_WIDTH(inFrame[0], 0),
    QIMG_HEIGHT(inFrame[0], 0), QIMG_STRIDE(inFrame[0], 0),
    QIMG_SCANLINE(inFrame[0], 0), QIMG_LEN(inFrame[0], 0),
    QIMG_PL_TYPE(inFrame[0], 0), QIMG_ADDR(outFrame, 0),
    QIMG_WIDTH(outFrame, 0), QIMG_HEIGHT(outFrame, 0),
    QIMG_STRIDE(outFrame, 0), QIMG_SCANLINE(outFrame, 0),
    QIMG_LEN(outFrame, 0), QIMG_PL_TYPE(outFrame, 0));

  if (*p_sw2d_mask & SW2D_OPS_DOWNSCALE) {
    IDBG_HIGH("%s:%d] SW2D downscale", __func__, __LINE__);

    if (QIMG_SINGLE_PLN_INTLVD(inFrame[0])) {
      /* deinterleave single plane YUV to semi-planar */
      ret = deinterleaveFrame(inFrame[0], NULL);
      if (IMG_ERROR(ret)) {
        IDBG_ERROR("%s:%d] Error, deinterleave failed", __func__, __LINE__);
        goto error;
      }
      pSrcBuf = (uint8_t *)mBufMemHandle.vaddr;
    } else {
      pSrcBuf = QIMG_ADDR(inFrame[0], 0);
    }

    if (!pSrcBuf) {
      IDBG_ERROR("%s:%d] Error, src buffer null", __func__, __LINE__);
      goto error;
    }
    IDBG_MED("%s:%d] src buf %p", __func__, __LINE__, pSrcBuf);

    /* Downscale Y frame */
    fcvScaleDownMNu8(pSrcBuf,
      QIMG_WIDTH(inFrame[0], 0),
      QIMG_HEIGHT(inFrame[0], 0),
      QIMG_STRIDE(inFrame[0], 0),
      QIMG_ADDR(outFrame, 0),
      QIMG_WIDTH(outFrame, 0),
      QIMG_HEIGHT(outFrame, 0),
      QIMG_STRIDE(outFrame, 0));
    IDBG_HIGH("%s:%d] Downscale successful", __func__, __LINE__);
  }

  if (*p_sw2d_mask & SW2D_OPS_DEINTERLEAVE) {
    IDBG_HIGH("%s:%d] SW2D deinterleave", __func__, __LINE__);

    if (QIMG_SINGLE_PLN_INTLVD(inFrame[0])) {
      ret = deinterleaveFrame(inFrame[0], QIMG_ADDR(outFrame, 0));
      if (IMG_ERROR(ret)) {
        IDBG_ERROR("%s:%d] Error, deinterleaving failed", __func__, __LINE__);
        goto error;
      }
    } else {
      IDBG_HIGH("%s:%d] Deinterleaving not needed for this frame type",
        __func__, __LINE__);
    }
  }

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return IMG_SUCCESS;

error:
  IDBG_ERROR("%s:%d] Error, sw2d error", __func__, __LINE__);
  return IMG_ERR_GENERAL;
}
