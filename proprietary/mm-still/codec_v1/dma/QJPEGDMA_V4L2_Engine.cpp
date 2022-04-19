/*****************************************************************************
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *****************************************************************************/

#include "QJPEGDMA_V4L2_Engine.h"
#include <sched.h>

/*============================================================================
 MACROS and CONSTANTS
 ============================================================================*/

#define TRANSFER_TIMEOUT 10000

/* plane count for semiplanar and planar*/
#define MONO_CNT 1
#define SP_CNT 2
#define P_CNT 3

/* Default frame rate */
#define JPEGDMA_FRAME_RATE (30)

/* Macro to check if dimensions changed */
#define JPEGDMA_CMP_DIM(new_dim, curr_dim) ({ \
  bool ret = false; \
  if ((new_dim.width != curr_dim.width) || \
    (new_dim.height != curr_dim.height) || \
    (new_dim.stride != curr_dim.stride) || \
    (new_dim.scanline != curr_dim.scanline)) { \
    ret = true; \
  } \
  ret; \
})

/*===========================================================================
 * Function: QJPEGDMA_V4L2_Engine
 *
 * Description: QJPEGDMA_V4L2_Engine constructor
 *
 * Input parameters:
 *   @aPrivData_ptr : Client private data pointer
 *   @aCallback : Call back function
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QJPEGDMA_V4L2_Engine::QJPEGDMA_V4L2_Engine(void *aPrivData_ptr,
  DmaEngineCallback aCallback)
{
  QI_MUTEX_INIT(&mMutex);
  QI_COND_INIT(&mCond);
  mInputImage_ptr = NULL;
  mOutputImage_ptr = NULL;
  mState = ESTATE_IDLE;
  mOutputDone = false;
  mError = QI_SUCCESS;
  mCallback = aCallback;
  mClientData_ptr = aPrivData_ptr;
  mYuvFormat = CAM_FORMAT_MAX;
  mFramerate = JPEGDMA_FRAME_RATE;
  mFrameId = 0;
  mStreaming = false;
  mMaxDMADownScale = 0.0;
  QI_MUTEX_INIT(&mLibMutex);
  QI_COND_INIT(&mLibCond);
  memset(&mDmaComp, 0x0, sizeof(QDmaImgComp_t));
  memset(&mInputDim, 0x0, sizeof(img_dim_t));
  memset(&mOutputDim, 0x0, sizeof(img_dim_t));
  memset(&mCropRect, 0x0, sizeof(img_rect_t));
}

/*===========================================================================
 * Function: ReleaseSession
 *
 * Description: release the current encoding session
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QJPEGDMA_V4L2_Engine::ReleaseSession()
{
  img_component_ops_t *lComp_ptr = &mDmaComp.comp;
  int32_t lrc;

  QIDBG_HIGH("%s:%d: E", __func__, __LINE__);

  Stop();

  mTransferThread.StopThread(this);

  QI_LOCK(&mMutex);

  if (mStreaming == true) {
    lrc = IMG_COMP_ABORT(lComp_ptr, NULL);
    if (IMG_ERROR(lrc)) {
      QIDBG_ERROR("%s:%d: Failed to abort jpeg dma comp", __func__,
        __LINE__);
    }
    mStreaming = false;
    mFrameId = 0;
  }

  QI_UNLOCK(&mMutex);
  QIDBG_HIGH("%s:%d: X", __func__, __LINE__);
}

/*===========================================================================
 * Function: ~QJPEGDMA_V4L2_Engine
 *
 * Description: QJPEGDMA_V4L2_Engine destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QJPEGDMA_V4L2_Engine::~QJPEGDMA_V4L2_Engine()
{

  img_core_ops_t *lCore_ops_ptr = &mDmaComp.core_ops;
  img_component_ops_t *lComp_ptr = &mDmaComp.comp;
  int lrc = 0;

  QIDBG_HIGH("%s:%d: E", __func__, __LINE__);

  Stop();

  /*stop and join the dma engine thread*/
  mTransferThread.JoinThread();

  QI_LOCK(&mMutex);
  if (mStreaming == true) {
    lrc = IMG_COMP_ABORT(lComp_ptr, NULL);
    if (IMG_ERROR(lrc)) {
      QIDBG_ERROR("%s:%d: Failed to abort jpeg dma comp", __func__,
        __LINE__);
    }
  }
  QI_UNLOCK(&mMutex);

  lrc = IMG_COMP_DEINIT(lComp_ptr);
  if (IMG_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d: Failed to deinit jpeg dma comp", __func__,
      __LINE__);
  }

  lrc = IMG_COMP_UNLOAD(lCore_ops_ptr);
  if (IMG_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d: Failed to unload jpeg dma comp", __func__,
      __LINE__);
  }

  QI_MUTEX_DESTROY(&mMutex);
  QI_COND_DESTROY(&mCond);

  /*unload the library*/
  UnLoad();
  QIDBG_HIGH("%s:%d: X", __func__, __LINE__);
}

/*===========================================================================
 * Function: GetMaxDownScale
 *
 * Description: Returns the max DMA downscale factor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   Max DMA downscale factor
 *
 * Notes: none
 *==========================================================================*/
float QJPEGDMA_V4L2_Engine::GetMaxDownScale()
{
  return mMaxDMADownScale;
}

/*===========================================================================
 * Function: EventHandler
 *
 * Description: Event handler for the dma engine
 *
 * Input parameters:
 *   @aFrameBundle_ptr - frame bundle
 *   @aCallbackEvent - call back event
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
void QJPEGDMA_V4L2_Engine::EventHandler(img_frame_bundle_t *aFrameBundle_ptr,
  img_event_type aCallbackEvent)
{
  QIDBG_HIGH("%s:%d: E", __func__, __LINE__);
  QIDBG_MED("%s:%d] Received callback ,event %d", __func__, __LINE__,
    aCallbackEvent);

  QI_LOCK(&mLibMutex);

  if (QIMG_EVT_BUF_DONE == aCallbackEvent) {
    QIDBG_HIGH("%s %d: JPEG_EVT_SESSION_DONE", __func__, __LINE__);
    mOutputDone = true;
  } else if (QIMG_EVT_ERROR == aCallbackEvent) {
    QIDBG_ERROR("%s %d: JPEG_EVT_ERROR", __func__, __LINE__);
    mError = QI_ERR_GENERAL;
  }

  if (aFrameBundle_ptr) {
    QIDBG_HIGH("%s:%d] Frame ID = %d", __func__, __LINE__, mFrameId);
    if ((aFrameBundle_ptr->p_input[0]->frame_id != mFrameId) ||
      (aFrameBundle_ptr->p_output[0]->frame_id != mFrameId) ||
      (aFrameBundle_ptr->p_meta[0]->frame_id != mFrameId)) {
      QIDBG_ERROR("%s %d: Error, Frame id mismatch mFrameId %d",
        __func__, __LINE__, mFrameId);
      QIDBG_ERROR("%s:%d] input Frame ID = %d", __func__, __LINE__,
        aFrameBundle_ptr->p_input[0]->frame_id);
      QIDBG_ERROR("%s:%d] Output Frame ID = %d", __func__, __LINE__,
        aFrameBundle_ptr->p_output[0]->frame_id);
      QIDBG_ERROR("%s:%d] Meta Frame ID = %d", __func__, __LINE__,
        aFrameBundle_ptr->p_meta[0]->frame_id);
      mError = QI_ERR_GENERAL;
    }
  }

  QI_SIGNAL(&mLibCond);
  QI_UNLOCK(&mLibMutex);
  QIDBG_HIGH("%s:%d: X", __func__, __LINE__);
}

/*===========================================================================
 * Function: jpegdma_cb_handler
 *
 * Description: Static function for event handler
 *
 * Input parameters:
 *   p_user - HW object pointer
 *   p_evt - pointer to the HW events
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_INVALID_INPUT
 *
 * Notes: none
 *==========================================================================*/
int jpegdma_cb_handler(void *p_user, img_frame_bundle_t *p_frame_bundle,
    img_event_type cb_event)
{
  QJPEGDMA_V4L2_Engine *lEngine = (QJPEGDMA_V4L2_Engine *) p_user;

  if (lEngine == NULL) {
    QIDBG_ERROR("%s %d: Error Callback sends NULL handle", __func__, __LINE__);
    return QI_ERR_INVALID_INPUT;
  }

  lEngine->EventHandler(p_frame_bundle, cb_event);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: StartDmaComp
 *
 * Description: Sets parameters and starts jpeg Dma component
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QJPEGDMA_V4L2_Engine::StartDmaComp()
{
  int lrc;
  img_component_ops_t *lComp_ptr = &mDmaComp.comp;

  QIDBG_HIGH("%s:%d: E", __func__, __LINE__);

  lrc = IMG_COMP_SET_PARAM(lComp_ptr, QIMG_PARAM_SRC_DIM, (void *)&mInputDim);
  if (IMG_ERROR(lrc)) {
    QIDBG_ERROR("%s %d: Error Set input dim", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = IMG_COMP_SET_PARAM(lComp_ptr, QIMG_PARAM_DST_DIM, (void *)&mOutputDim);
  if (IMG_ERROR(lrc)) {
    QIDBG_ERROR("%s %d: Error Set output dim", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = IMG_COMP_SET_PARAM(lComp_ptr, QIMG_PARAM_YUV_FMT, (void *)&mYuvFormat);
  if (IMG_ERROR(lrc)) {
    QIDBG_ERROR("%s %d: Error Set yuv format", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = IMG_COMP_SET_PARAM(lComp_ptr, QIMG_PARAM_FRAME_RATE,
    (void *)&mFramerate);
  if (IMG_ERROR(lrc)) {
    QIDBG_ERROR("%s %d: Error Set framerate", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = IMG_COMP_SET_PARAM(lComp_ptr, QIMG_PARAM_CROP_DIM, (void *)&mCropRect);
  if (IMG_ERROR(lrc)) {
    QIDBG_ERROR("%s %d: Error Set crop", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = IMG_COMP_START(lComp_ptr, NULL);
  if (IMG_ERROR(lrc)) {
    QIDBG_ERROR("%s %d: Error start jpeg dma component", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  QIDBG_HIGH("%s:%d: X", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: SendFramesToDmaComp
 *
 * Description: Sends input, output and meta frames to jpeg Dma component
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *
 * Notes: none
 *==========================================================================*/
int QJPEGDMA_V4L2_Engine::SendFramesToDmaComp()
{
  int lrc;
  img_component_ops_t *lComp_ptr = &mDmaComp.comp;

  QIDBG_HIGH("%s:%d: E", __func__, __LINE__);

  lrc = IMG_COMP_Q_BUF(lComp_ptr, &mInputFrame, IMG_IN);
  if (IMG_ERROR(lrc)) {
    QIDBG_ERROR("%s %d: Error send input frame", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = IMG_COMP_Q_BUF(lComp_ptr, &mOutputFrame, IMG_OUT);
  if (IMG_ERROR(lrc)) {
    QIDBG_ERROR("%s %d: Error send output frame", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  lrc = IMG_COMP_Q_META_BUF(lComp_ptr, &mMeta);
  if (IMG_ERROR(lrc)) {
    QIDBG_ERROR("%s %d: Error send meta frame", __func__, __LINE__);
    return QI_ERR_GENERAL;
  }

  QIDBG_HIGH("%s:%d: X", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: populateFrameInfo
 *
 * Description: Populate frame info
 *
 * Input parameters:
 *   @aFrame : Frame to populate
 *   @aImage_ptr : pointer to image
 *   @aDim : Current configurred dimensions, used to validate image dimesions.
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QJPEGDMA_V4L2_Engine::PopulateFrameInfo(img_frame_t &aFrame,
  QImage *aImage_ptr, img_dim_t &aDim)
{
  QIPlane *lPlane_ptr = NULL;
  int32_t lrc = QI_SUCCESS;

  QIDBG_HIGH("%s:%d: E", __func__, __LINE__);

  memset(&aFrame, 0x0, sizeof(img_frame_t));

  if ((aDim.width != aImage_ptr->ActualSize().Width()) ||
    (aDim.height != aImage_ptr->ActualSize().Height()) ||
    (aDim.stride != aImage_ptr->Size().Width()) ||
    (aDim.scanline != aImage_ptr->Size().Height())) {
    QIDBG_ERROR("%s:%d] Dimension mismatch config dim wxh %dx%d stxsl %dx%d,"
      "img dim wxh %dx%d stxsl %dx%d", __func__, __LINE__,
      aDim.width, aDim.height, aDim.stride, aDim.scanline,
      aImage_ptr->ActualSize().Width(), aImage_ptr->ActualSize().Height(),
      aImage_ptr->Size().Width(), aImage_ptr->Size().Height());
    return QI_ERR_INVALID_INPUT;
  }

  aFrame.frame_id = mFrameId;
  aFrame.idx = mFrameId;
  aFrame.frame_cnt = 1;
  aFrame.info.width = aImage_ptr->ActualSize().Width();
  aFrame.info.height = aImage_ptr->ActualSize().Height();
  aFrame.info.num_planes = aImage_ptr->PlaneCount();
  aFrame.frame[0].plane_cnt = aImage_ptr->PlaneCount();

  /*set the padded dimension*/
  if (MONO_CNT <= aImage_ptr->PlaneCount()) {
    lPlane_ptr = aImage_ptr->getPlane(QIPlane::PLANE_Y);
    if (NULL == lPlane_ptr) {
      QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }
    aFrame.frame[0].plane[0].fd = lPlane_ptr->Fd();
    aFrame.frame[0].plane[0].width = lPlane_ptr->ActualSize().Width();
    aFrame.frame[0].plane[0].height = lPlane_ptr->ActualSize().Height();
    aFrame.frame[0].plane[0].stride = lPlane_ptr->Size().Width();
    aFrame.frame[0].plane[0].scanline = lPlane_ptr->Size().Height();
    aFrame.frame[0].plane[0].offset = lPlane_ptr->Offset();
    aFrame.frame[0].plane[0].length = lPlane_ptr->Length();

    QIDBG_HIGH("%s:%d] y plane length %d , offset %d",
      __func__, __LINE__, aFrame.frame[0].plane[0].length,
      aFrame.frame[0].plane[0].offset);
  }

  if (SP_CNT == aImage_ptr->PlaneCount()) {
    lPlane_ptr = aImage_ptr->getPlane(QIPlane::PLANE_CB_CR);
    if (NULL == lPlane_ptr) {
      QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
      return QI_ERR_INVALID_INPUT;
    }

    aFrame.frame[0].plane[1].fd = lPlane_ptr->Fd();
    aFrame.frame[0].plane[1].width = lPlane_ptr->ActualSize().Width();
    aFrame.frame[0].plane[1].height = lPlane_ptr->ActualSize().Height();
    aFrame.frame[0].plane[1].stride = lPlane_ptr->Size().Width();
    aFrame.frame[0].plane[1].scanline = lPlane_ptr->Size().Height();
    aFrame.frame[0].plane[1].offset = lPlane_ptr->Offset();
    aFrame.frame[0].plane[1].length = lPlane_ptr->Length();

    QIDBG_HIGH("%s:%d] chroma plane length %d, offset %d",
      __func__, __LINE__, aFrame.frame[0].plane[1].length,
      aFrame.frame[0].plane[1].offset);

  } else if (P_CNT == aImage_ptr->PlaneCount()) {
    QIPlane::Type lType[2];

    if (QImage::IsCrCb(aImage_ptr->Format())) {
      lType[0] = QIPlane::PLANE_CR;
      lType[1] = QIPlane::PLANE_CB;
    } else {
      lType[0] = QIPlane::PLANE_CB;
      lType[1] = QIPlane::PLANE_CR;
    }

    for (int j = 0; j < 2; j++) {
      lPlane_ptr = aImage_ptr->getPlane(lType[j]);
      if (NULL == lPlane_ptr) {
        QIDBG_ERROR("%s:%d] Invalid input", __func__, __LINE__);
        return QI_ERR_INVALID_INPUT;
      }
      aFrame.frame[0].plane[j + 1].fd = lPlane_ptr->Fd();
      aFrame.frame[0].plane[j + 1].width = lPlane_ptr->ActualSize().Width();
      aFrame.frame[0].plane[j + 1].height = lPlane_ptr->ActualSize().Height();
      aFrame.frame[0].plane[j + 1].stride = lPlane_ptr->Size().Width();
      aFrame.frame[0].plane[j + 1].scanline = lPlane_ptr->Size().Height();
      aFrame.frame[0].plane[j + 1].offset = lPlane_ptr->Offset();
      aFrame.frame[0].plane[j + 1].length = lPlane_ptr->Length();

      QIDBG_HIGH("%s:%d] chroma plane %d length %d, offset %d",
        __func__, __LINE__, j + 1,aFrame.frame[0].plane[j + 1].length,
        aFrame.frame[0].plane[j + 1].offset);
    }
  }

  QIDBG_HIGH("%s:%d: X", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: getDmaYuvformat
 *
 * Description: translate IQ format to cam format
 *
 * Input parameters:
 *   fmt: IQ format
 *
 * Return values:
 *   Success:
 *   CAM_FORMAT_Y_ONLY
 *   CAM_FORMAT_YUV_420_NV12
 *   CAM_FORMAT_YUV_420_NV21
 *   CAM_FORMAT_YUV_420_YV12
 *   Failure:
 *   CAM_FORMAT_MAX
 *
 * Notes: none
 *==========================================================================*/
cam_format_t QJPEGDMA_V4L2_Engine::getDmaYuvformat(QIFormat fmt)
{
  switch(fmt){
  case QI_MONOCHROME:
    return CAM_FORMAT_Y_ONLY;
  case QI_YCBCR_SP:
    return CAM_FORMAT_YUV_420_NV12;
  case QI_YCRCB_SP:
    return CAM_FORMAT_YUV_420_NV21;
  case QI_IYUV:
    return CAM_FORMAT_YUV_420_YV12;
  default:
    return CAM_FORMAT_MAX;
  }
}

/*===========================================================================
 * Function: Configure
 *
 * Description: Configure the HW decoder component
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QJPEGDMA_V4L2_Engine::Configure()
{
  int lrc = QI_SUCCESS;
  int lImgLibrc;
  QIPlane *lPlane_ptr = NULL;
  img_dim_t lInputDim;
  img_dim_t lOutputDim;

  QIDBG_HIGH("%s:%d: E", __func__, __LINE__);

  lInputDim.width = mInputImage_ptr->ActualSize().Width();
  lInputDim.height = mInputImage_ptr->ActualSize().Height();
  lInputDim.stride = mInputImage_ptr->Size().Width();
  lInputDim.scanline = mInputImage_ptr->Size().Height();

  lOutputDim.width = mOutputImage_ptr->ActualSize().Width();
  lOutputDim.height = mOutputImage_ptr->ActualSize().Height();
  lOutputDim.stride = mOutputImage_ptr->Size().Width();
  lOutputDim.scanline = mOutputImage_ptr->Size().Height();

  if ((JPEGDMA_CMP_DIM(lInputDim, mInputDim) ||
    JPEGDMA_CMP_DIM(lOutputDim, mOutputDim)) && mStreaming == true) {
    QIDBG_HIGH("%s:%d: Dim changed re-configure", __func__, __LINE__);

    lImgLibrc = IMG_COMP_ABORT(&mDmaComp.comp, NULL);
    if (IMG_ERROR(lImgLibrc)) {
      QIDBG_ERROR("%s:%d: Failed to abort jpeg dma comp", __func__,
        __LINE__);
      return QI_ERR_GENERAL;
    }

    mStreaming = false;
    mFrameId = 0;
  }

  if (mStreaming == false) {
    mInputDim.width = lInputDim.width;
    mInputDim.height = lInputDim.height;
    mInputDim.stride = lInputDim.stride;
    mInputDim.scanline = lInputDim.scanline;

    mOutputDim.width = lOutputDim.width;
    mOutputDim.height = lOutputDim.height;
    mOutputDim.stride = lOutputDim.stride;
    mOutputDim.scanline = lOutputDim.scanline;

    mCropRect.pos.x = mInputCrop.Left();
    mCropRect.pos.y = mInputCrop.Top();
    mCropRect.size.width = mInputCrop.Width();
    mCropRect.size.height = mInputCrop.Height();

    mYuvFormat = getDmaYuvformat(mInputImage_ptr->Format());
    if (mYuvFormat == CAM_FORMAT_MAX) {
      QIDBG_ERROR("%s %d: Invalid yuv format %d", __func__, __LINE__,
        mInputImage_ptr->Format());
      return QI_ERR_INVALID_INPUT;
    }

    mFramerate = JPEGDMA_FRAME_RATE;
  }

  lrc = PopulateFrameInfo(mInputFrame, mInputImage_ptr, mInputDim);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error populating input frame", __func__, __LINE__);
    return lrc;
  }

  lrc = PopulateFrameInfo(mOutputFrame, mOutputImage_ptr, mOutputDim);
  if (lrc != QI_SUCCESS) {
    QIDBG_ERROR("%s %d: Error populating output frame", __func__, __LINE__);
    return lrc;
  }

  memset(&mMeta, 0x0, sizeof(img_meta_t));
  mMeta.frame_id = mFrameId;
  mMeta.output_crop.pos.x = mInputCrop.Left();
  mMeta.output_crop.pos.y = mInputCrop.Top();
  mMeta.output_crop.size.width = mInputCrop.Width();
  mMeta.output_crop.size.height = mInputCrop.Height();

  QIDBG_HIGH("%s:%d: E", __func__, __LINE__);
  return lrc;
}

/*===========================================================================
 * Function: Start
 *
 * Description: Started the hardware dma component with crop.
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QJPEGDMA_V4L2_Engine::Start(QImage &aInImage, QImage &aOutImage,
  QICrop &aInCrop, bool sync)
{
  int lrc = QI_SUCCESS;

  QIDBG_HIGH("%s:%d] E state %d", __func__, __LINE__, mState);

  QI_LOCK(&mMutex);

  if (mState != ESTATE_IDLE) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_INVALID_OPERATION;
  }

  mInputImage_ptr = &aInImage;
  mOutputImage_ptr = &aOutImage;

  if ((NULL == mInputImage_ptr) || (NULL == mOutputImage_ptr)) {
    QIDBG_ERROR("%s:%d] failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_INVALID_OPERATION;
  }
  mInputCrop = aInCrop;

  lrc = Configure();
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d] config failed", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return lrc;
  }

  mState = ESTATE_ACTIVE;

  if (sync == false) {
    lrc = mTransferThread.StartThread(this);
    if (QI_ERROR(lrc)) {
      QIDBG_ERROR("%s:%d] Error", __func__, __LINE__);
      mState = ESTATE_IDLE;
      QI_UNLOCK(&mMutex);
      return lrc;
    }
    mTransferThread.setThreadName("JPEG_DMA");
  } else {
    QI_UNLOCK(&mMutex);
    Transfer();
    return lrc;
  }

  QI_UNLOCK(&mMutex);

  QIDBG_HIGH("%s:%d: X", __func__, __LINE__);
  return lrc;
}

/*===========================================================================
 * Function: Start
 *
 * Description: Started the hardware dma component without crop.
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_GENERAL
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QJPEGDMA_V4L2_Engine::Start(QImage &aInImage, QImage &aOutImage, bool sync)
{
  int lrc = QI_SUCCESS;
  QICrop lCrop;

  lCrop.setCrop(0x0, 0x0, 0x0, 0x0);

  return Start(aInImage, aOutImage, lCrop, sync);;
}

/*===========================================================================
 * Function: Stop
 *
 * Description: Stops the hardware dma engine
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *
 * Notes: none
 *==========================================================================*/
int QJPEGDMA_V4L2_Engine::Stop()
{
  int lrc = QI_SUCCESS;

  QIDBG_MED("%s:%d] mState %d", __func__, __LINE__, mState);

  QI_LOCK(&mMutex);

  if (mState != ESTATE_ACTIVE) {
    QI_UNLOCK(&mMutex);
    return QI_SUCCESS;
  }

  mState = ESTATE_STOP_REQUESTED;

  QIDBG_MED("%s:%d] ", __func__, __LINE__);
  QI_WAIT(&mCond, &mMutex);
  QIDBG_MED("%s:%d] ", __func__, __LINE__);

  QI_UNLOCK(&mMutex);

  return QI_SUCCESS;
}

/*===========================================================================
 * Function: Execute
 *
 * Description: Executes the hardware dma engine
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QJPEGDMA_V4L2_Engine::Execute()
{
  if (mTransferThread.IsSelf()) {
    /* dma engine thread */
    QIDBG_HIGH("%s:%d] E", __func__, __LINE__);
    Transfer();
    QIDBG_HIGH("%s:%d] X", __func__, __LINE__);
  }
}

/*===========================================================================
 * Function: Transfer
 *
 * Description: Start Transfer
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/

void QJPEGDMA_V4L2_Engine::Transfer()
{
  int lrc = QI_SUCCESS;
  bool lsend_cb = true;
  img_component_ops_t *lComp_ptr = &mDmaComp.comp;

  QI_LOCK(&mMutex);

  if (mStreaming == false) {
    lrc = StartDmaComp();
    if (QI_ERROR(lrc)) {
      QIDBG_ERROR("%s:%d] error start dma component falied",
        __func__, __LINE__);
      goto error_startComp;
    }
    mStreaming = true;
  }

  mOutputDone = false;
  mError = QI_SUCCESS;

  lrc = SendFramesToDmaComp();
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d] error send frames falied", __func__, __LINE__);
    goto error_transfer;
  }
  QI_UNLOCK(&mMutex);

  QI_LOCK(&mLibMutex);
  if (!mOutputDone && QI_SUCCEEDED(mError)) {
    lrc = QIThread::WaitForCompletion(&mLibCond, &mLibMutex,
      TRANSFER_TIMEOUT);
  }
  QI_UNLOCK(&mLibMutex);

  QI_LOCK(&mMutex);
  QIDBG_MED("%s:%d] wait done %d", __func__, __LINE__, lrc);

  if (QI_ERROR(mError) || (QI_ERR_TIMEOUT == lrc)) {
    QIDBG_ERROR("%s:%d] error occured or timed out %d %d",
      __func__, __LINE__, mError, lrc);
    goto error_transfer;
  }

  if (mState == ESTATE_STOP_REQUESTED) {
    goto error_transfer;
  }

  mState = ESTATE_IDLE;
  //increment frame id
  mFrameId++;
  QIDBG_MED("%s:%d] Frame ID inc to = %d", __func__, __LINE__, mFrameId);
  QI_UNLOCK(&mMutex);

  if (mCallback) {
    mCallback(mClientData_ptr, mOutputImage_ptr , QI_SUCCESS);
  }

  return;

error_transfer:
  lrc = IMG_COMP_ABORT(lComp_ptr, NULL);
  if (IMG_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d: Failed to abort jpeg dma comp", __func__,
      __LINE__);
  }
  mStreaming = false;

error_startComp:
  if (mState == ESTATE_STOP_REQUESTED) {
    QI_SIGNAL(&mCond);
    lsend_cb = false;
  }

  mState = ESTATE_IDLE;
  mFrameId = 0;
  QIDBG_ERROR("%s:%d] Frame ID set to zero = %d", __func__, __LINE__, mFrameId);
  QI_UNLOCK(&mMutex);

  if (mCallback && lsend_cb)
    mCallback(mClientData_ptr, mOutputImage_ptr, QI_ERR_GENERAL);

  return;
}

/*===========================================================================
 * Function: Load
 *
 * Description: Loads the userspace jpeg dma driver
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   QI_SUCCESS
 *   QI_ERR_NOT_FOUND
 *   QI_ERR_INVALID_OPERATION
 *
 * Notes: none
 *==========================================================================*/
int QJPEGDMA_V4L2_Engine::Load()
{
  int32_t lrc;
  img_core_ops_t *lCore_ops_ptr = &mDmaComp.core_ops;
  char comp_name[25] = "qti.jpegdma";

  QIDBG_HIGH("%s:%d] E", __func__, __LINE__);

  if (mDmaComp.ptr) {
    QIDBG_ERROR("%s:%d: library already loaded", __func__, __LINE__);
    return QI_ERR_INVALID_OPERATION;
  }

  // Open libmmcamera_imglib
  mDmaComp.ptr = dlopen("libmmcamera_imglib.so", RTLD_NOW);
  if (!mDmaComp.ptr) {
    QIDBG_ERROR("%s:%d] Error opening imglib library", __func__, __LINE__);
    return QI_ERR_NOT_FOUND;
  }

  *(void **) &(mDmaComp.img_core_get_comp) = dlsym(mDmaComp.ptr,
    "img_core_get_comp");
  if (!mDmaComp.img_core_get_comp) {
    QIDBG_ERROR("%s:%d] Error linking img_core_get_comp", __func__, __LINE__);
    goto error_load;
  }

  lrc = mDmaComp.img_core_get_comp(IMG_COMP_GEN_2D, comp_name,
    lCore_ops_ptr);
  if (IMG_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d] Error getting jpegdma component", __func__, __LINE__);
    goto error_load;
  }

  lrc = IMG_COMP_LOAD(lCore_ops_ptr, NULL);
  if (IMG_ERROR(lrc) || !lCore_ops_ptr->handle) {
    QIDBG_ERROR("%s:%d] Error loading jpegdma component %p",
      __func__, __LINE__, lCore_ops_ptr->handle);
    goto error_load;
  }

  QIDBG_HIGH("%s:%d] %s loaded successfully X", __func__, __LINE__,
    (char *) lCore_ops_ptr->handle);
  return QI_SUCCESS;

error_load:
  dlclose(mDmaComp.ptr);
  mDmaComp.ptr = NULL;
  return QI_ERR_NOT_FOUND;
}

/*===========================================================================
 * Function: UnLoad
 *
 * Description: UnLoads the jpeg dma userspace driver
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QJPEGDMA_V4L2_Engine::UnLoad()
{
  int lrc = 0;
  QIDBG_HIGH("%s:%d: ptr %p", __func__, __LINE__, mDmaComp.ptr);
  if (mDmaComp.ptr) {
    lrc = dlclose(mDmaComp.ptr);
    if (lrc < 0)
      QIDBG_ERROR("%s:%d] error %s", __func__, __LINE__, dlerror());
    mDmaComp.ptr = NULL;
  }
}

/*===========================================================================
 * Function: Init
 *
 * Description: Function to load and initalize jpeg dma component.
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   true/false
 *
 * Notes: none
 *==========================================================================*/
bool QJPEGDMA_V4L2_Engine::Init()
{
  int32_t lrc = QI_SUCCESS;
  img_core_ops_t *lCore_ops_ptr = &mDmaComp.core_ops;
  img_component_ops_t *lComp_ptr = &mDmaComp.comp;

  QI_LOCK(&mMutex);

  QIDBG_HIGH("%s:%d] E", __func__, __LINE__);

  lrc = Load();
  if (QI_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d: cannot load jpeg dma component", __func__, __LINE__);
    goto error_load;
  }

  lrc = IMG_COMP_CREATE(lCore_ops_ptr, lComp_ptr);
  if (IMG_ERROR(lrc) || !lComp_ptr->handle) {
    QIDBG_ERROR("%s:%d: cannot create jpeg dma component err %d %p",
      __func__, __LINE__, lrc, lComp_ptr->handle);
    goto error_init;
  }

  lrc = IMG_COMP_INIT(lComp_ptr, (void *)this, (void *)jpegdma_cb_handler);
  if (IMG_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d: cannot init jpeg dma component", __func__, __LINE__);
    goto error_init;
  }

  lrc = IMG_COMP_GET_PARAM(lComp_ptr, QIMG_PARAM_MAX_DS,
    (void *)&mMaxDMADownScale);
  if (IMG_ERROR(lrc)) {
    QIDBG_ERROR("%s:%d: cannot init jpeg dma component", __func__, __LINE__);
    goto error_init;
  }
  QIDBG_HIGH("%s:%d: Max Downscale factor got is %f", __func__, __LINE__,
    mMaxDMADownScale);
  QI_UNLOCK(&mMutex);

  QIDBG_HIGH("%s:%d] X", __func__, __LINE__);
  return true;

error_init:
  UnLoad();

error_load:
  QI_UNLOCK(&mMutex);
  return false;
}

/*===========================================================================
 * Function: New
 *
 * Description: 2 phase constructor for QJPEGDMA_V4L2_Engine
 *
 * Input parameters:
 *   aPrivData - Pointer to client handle
 *   aCallback - client call back function
 *
 * Return values:
 *   jpeg dma engine pointer
 *
 * Notes: none
 *==========================================================================*/
QJPEGDMA_V4L2_Engine* QJPEGDMA_V4L2_Engine::New(void *aPrivData_ptr,
    DmaEngineCallback aCallback)
{

  QIDBG_HIGH("%s:%d: E", __func__, __LINE__);

  QJPEGDMA_V4L2_Engine* lDma_ptr = new QJPEGDMA_V4L2_Engine(aPrivData_ptr,
    aCallback);

  if (NULL == lDma_ptr) {
    QIDBG_ERROR("%s:%d] no memory", __func__, __LINE__);
    return NULL;
  }

  if (!lDma_ptr->Init()) {
    QIDBG_ERROR("%s:%d] validation failed", __func__, __LINE__);
    delete lDma_ptr;
    return NULL;
  }

  QIDBG_HIGH("%s:%d: X", __func__, __LINE__);
  return lDma_ptr;
}
